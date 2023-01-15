//
//  ShaderChainer.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#include "ShaderChainer.hpp"
#include "AsciiShader.hpp"
#include "BlurShader.hpp"
#include "ConfigService.hpp"
#include "Console.hpp"
#include "FeedbackShader.hpp"
#include "FeedbackSourceService.hpp"
#include "GlitchShader.hpp"
#include "HSBShader.hpp"
#include "KaleidoscopeShader.hpp"
#include "MirrorShader.hpp"
#include "MixShader.hpp"
#include "PixelShader.hpp"
#include "ShaderChainerService.hpp"
#include "TileShader.hpp"
#include "TransformShader.hpp"
#include "VideoSourceService.hpp"
#include "ofMain.h"

void ShaderChainer::setup() {
  registerFeedbackDestination();
  ofFbo::Settings fboSettings;
  fboSettings.width = 640;
  fboSettings.height = 480;
  fboSettings.internalformat = GL_RGB;
  fboSettings.textureTarget = GL_TEXTURE_2D;
  fbo = ofFbo();
  fbo.allocate(640, 480);
  fbo.begin();
  ofClear(0, 0, 0, 255);
  fbo.end();

  frameTexture = std::make_shared<ofTexture>();
  frameTexture->allocate(640, 480, GL_RGB);
  frameBuffer.bind(GL_SAMPLER_2D_RECT);
  frameBuffer.allocate(640 * 480 * 4, GL_STATIC_COPY);
  frameTexture->setTextureWrap(GL_REPEAT, GL_REPEAT);

  previewTexture = std::make_shared<ofTexture>();
  previewTexture->allocate(320, 240, GL_RGB);
  previewBuffer.bind(GL_SAMPLER_2D_RECT);
  previewBuffer.allocate(320 * 240 * 4, GL_STATIC_COPY);

  for (auto &sh : shaders()) {
    sh->setup();
  }

  ping.allocate(640, 480);
  pong.allocate(640, 480);

  ping.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
  pong.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
}

void ShaderChainer::update() { fbo = processFrame(source->frameTexture); }

std::vector<std::shared_ptr<Shader>> ShaderChainer::shaders() {
  std::vector<std::shared_ptr<Shader>> shaders;
  auto shader = front;
  while (shader != nullptr) {
    shaders.push_back(shader);
    shader = shader->next;
  }
  return shaders;
}

ofFbo ShaderChainer::processFrame(std::shared_ptr<ofTexture> texture) {
  ping.begin();
  ofClear(255, 255, 255);
  ping.end();

  pong.begin();
  ofClear(255, 255, 255);
  pong.end();

  pong.begin();
  texture->draw(0, 0, 640, 480);
  pong.end();

  ofFbo *canv = &ping;
  ofFbo *tex = &pong;

  bool flip = false;
  auto currentShaders = shaders();

  // Return raw texture if no shaders
  if (currentShaders.empty()) {
    frameTexture = std::make_shared<ofTexture>(pong.getTexture());
    saveFeedbackFrame(texture);
    return pong;
  }

  for (auto sharedShader : currentShaders) {
    auto shader = sharedShader.get();
    
    ShaderChainerService::getService()->associateShaderWithChainer(shader->id()
                                                                   , shared_from_this());
    // Skip disabled shaders
    if (!shader->enabled()) {
      continue;
    }

    if (flip) {
      canv = &pong;
      tex = &ping;
    } else {
      canv = &ping;
      tex = &pong;
    }
    shader->shade(tex, canv);
    tex->begin();
    ofClear(255, 255, 255);
    tex->end();
    flip = !flip;
  }

  //  canv->getTexture().copyTo(frameBuffer);
  //  frameTexture->loadData(frameBuffer, GL_RGB, GL_UNSIGNED_BYTE);

  frameTexture = std::make_shared<ofTexture>(canv->getTexture());
  saveFeedbackFrame(frameTexture);

  return *canv;
}

void ShaderChainer::saveFrame() {
  // no-op
}

void ShaderChainer::pushShader(ShaderType shaderType) {
  auto shader = ShaderChainerService::getService()->shaderForType(
      shaderType, UUID::generateUUID(), 0);
  ShaderChainerService::getService()->addShader(shader);

  auto frontShader = front;

  if (frontShader == nullptr) {
    front = shader;
    shader->parent = nullptr;
    shader->next = nullptr;
  } else {
    while (frontShader->next != nullptr) {
      frontShader = frontShader->next;
    }
    frontShader->next = shader;
    shader->parent = frontShader;
    shader->next = nullptr;
  }
}

void ShaderChainer::saveFeedbackFrame(std::shared_ptr<ofTexture> frame) {
  if (feedbackDestination) {
    feedbackDestination->pushFrame(frame);
  }
}

void ShaderChainer::deleteShader(shared_ptr<Shader> shader) {
//  auto it = std::find(shaders.begin(), shaders.end(), shader);
//  if (it != shaders.end()) {
//    shaders.erase(it);
//  }
}

json ShaderChainer::serialize() {
  json j;
  j["shaders"] = json::array();
  j["chainerId"] = chainerId;
  j["name"] = name;
  j["sourceId"] = source->id;

  for (auto shader : shaders()) {
    j["shaders"].push_back(shader->serialize());
  }
  return j;
}

void ShaderChainer::load(json j) {
  std::cout << j.dump(4) << std::endl;

  if (VideoSourceService::getService()->videoSourceForId(j["sourceId"]) !=
      nullptr) {
    source = VideoSourceService::getService()->videoSourceForId(j["sourceId"]);
  }

  chainerId = j["chainerId"];

  std::shared_ptr<Shader> last = nullptr;
  for (auto shaderJson : j["shaders"]) {
    ShaderType shaderType = shaderJson["shaderType"];
    std::string shaderId = shaderJson["shaderId"];

    auto shader = ShaderChainerService::getService()->shaderForType(
        shaderType, shaderId, shaderJson);
    ShaderChainerService::getService()->addShader(shader);
    shader->parent = last;
    if (last != nullptr)
      last->next = shader;

    last = shader;
  }
}

void ShaderChainer::registerFeedbackDestination() {
  std::shared_ptr<FeedbackSource> source =
      std::make_shared<FeedbackSource>(chainerId, sourceName);
  FeedbackSourceService::getService()->registerFeedbackSource(source);
  feedbackDestination = source;
}

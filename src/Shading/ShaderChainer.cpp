//
//  ShaderChainer.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#include "ShaderChainer.hpp"
#include "ASCIIShader.hpp"
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

static const int EmptyId = -1;

void ShaderChainer::setup() {
  registerFeedbackDestination();
  fbo = ofFbo();
  fbo.allocate(settings.width->value, settings.height->value);
  fbo.begin();
  ofClear(0, 0, 0, 255);
  fbo.end();
  fbo.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
  
  resizeShader.load("shaders/Resize");
  
  frameTexture = std::make_shared<ofTexture>();
  frameTexture->allocate(source->settings.width->value, source->settings.height->value, GL_RGB);
  frameTexture->setTextureWrap(GL_REPEAT, GL_REPEAT);
  
  for (auto &sh : shaders()) {
    sh->setup();
  }

  ping.allocate(source->settings.width->value, source->settings.height->value);
  pong.allocate(source->settings.width->value, source->settings.height->value);

  ping.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
  pong.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
}

void ShaderChainer::update() {
  // If the width of our Source or the width of the Chainer itself (the output)
  // has changed, then setup again to recreate the textures.
  if (fbo.getWidth() != settings.width->value || source->settings.width->value != frameTexture->getWidth()) {
    setup();
  }
  processFrame(source->frameTexture);
  resizeFrame();
  saveFeedbackFrame();
}

std::vector<std::shared_ptr<Shader>> ShaderChainer::shaders() {
  std::vector<std::shared_ptr<Shader>> shaders;
  auto shader = front;
  while (shader != nullptr) {
    shaders.push_back(shader);
    shader = shader->next;
  }
  return shaders;
}

/// Draws our `frameTexture` into our `fbo` at the size specified by our `ShaderChainer`.
void ShaderChainer::resizeFrame() {
  fbo.begin();
  resizeShader.begin();
  resizeShader.setUniform2f("inDimensions", frameTexture->getWidth(), frameTexture->getHeight());
  resizeShader.setUniform2f("outDimensions", fbo.getWidth(), fbo.getHeight());
  resizeShader.setUniformTexture("tex", *frameTexture.get(), 4);
  fbo.draw(0, 0);
  resizeShader.end();
  fbo.end();
}

ofFbo ShaderChainer::processFrame(std::shared_ptr<ofTexture> texture) {
  ping.begin();
  ofClear(255, 255, 255, 0);
  ping.end();

  pong.begin();
  ofClear(255, 255, 255, 0);
  pong.end();

  pong.begin();
  texture->draw(0, 0);
  pong.end();

  ofFbo *canv = &ping;
  ofFbo *tex = &pong;
  
  ping.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
  pong.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);

  bool flip = false;
  auto currentShaders = shaders();

  // Return raw texture if no shaders
  if (currentShaders.empty()) {
    frameTexture = std::make_shared<ofTexture>(pong.getTexture());
    return pong;
  }

  for (auto sharedShader : currentShaders) {
    auto shader = sharedShader.get();

    ShaderChainerService::getService()->associateShaderWithChainer(
        shader->id(), shared_from_this());
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

  frameTexture = std::make_shared<ofTexture>(canv->getTexture());

  return *canv;
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

void ShaderChainer::saveFeedbackFrame() {
  if (feedbackDestination) {
    feedbackDestination->pushFrame(fbo);
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
  
  // We have a front shader
  if (front != nullptr)
    j["front"] = front->id();
  else
    j["front"] = EmptyId;

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
  
  if (j["front"] != EmptyId)
    front = ShaderChainerService::getService()->shaderForId(j["front"]);
}

void ShaderChainer::registerFeedbackDestination() {
  std::shared_ptr<FeedbackSource> source =
      std::make_shared<FeedbackSource>(chainerId, sourceName, std::make_shared<VideoSourceSettings>(settings));
  FeedbackSourceService::getService()->registerFeedbackSource(source);
  feedbackDestination = source;
}

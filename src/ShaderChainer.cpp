//
//  ShaderChainer.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#include "ShaderChainer.hpp"
#include "ofMain.h"
#include "MirrorShader.hpp"
#include "FeedbackSourceService.hpp"
#include "AsciiShader.hpp"
#include "ConfigService.hpp"
#include "KaleidoscopeShader.hpp"
#include "TransformShader.hpp"
#include "ShaderChainerService.hpp"
#include "Console.hpp"
#include "PixelShader.hpp"
#include "GlitchShader.hpp"
#include "HSBShader.hpp"
#include "FeedbackShader.hpp"
#include "BlurShader.hpp"

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
  frameBuffer.allocate(640*480*4, GL_STATIC_COPY);
  
  previewTexture = std::make_shared<ofTexture>();
  previewTexture->allocate(320, 240, GL_RGB);
  previewBuffer.bind(GL_SAMPLER_2D_RECT);
  previewBuffer.allocate(320*240*4, GL_STATIC_COPY);
    
  for (auto & sh : shaders) {
    sh->setup();
  }
  

  
  ping.allocate(640, 480);
  pong.allocate(640, 480);
}

void ShaderChainer::update() {
  fbo = processFrame(source->frameTexture);
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
  
  // Return raw texture if no shaders
  if (shaders.empty()) {
    frameTexture = std::make_shared<ofTexture>(pong.getTexture());
    saveFeedbackFrame(texture);
    return pong;
  }
  
  for (auto sharedShader : shaders) {
    auto shader = sharedShader.get();
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
  // Increment the ShaderId every time we push a new Shader
  static int ShaderId = -1;
  ShaderId++;
  
  std::string shaderIdStr = std::to_string(ShaderId);
  switch (shaderType) {
    case ShaderTypeNone:
      return;
    case ShaderTypeHSB: {
      auto settings = new HSBSettings(shaderIdStr, 0);
      auto shader = std::make_shared<HSBShader>(settings);
      shader.get()->setup();
      shaders.push_back(shader);
      return;
    }
    case ShaderTypeBlur: {
      auto settings = new BlurSettings(shaderIdStr, 0);
      auto shader = std::make_shared<BlurShader>(settings);
      shader.get()->setup();
      shaders.push_back(shader);
      return;
    }
    case ShaderTypePixelate: {
      auto settings = new PixelSettings(shaderIdStr, 0);
      auto shader = std::make_shared<PixelShader>(settings);
      shader.get()->setup();
      shaders.push_back(shader);
      return;
    }
    case ShaderTypeGlitch:
      return;
    case ShaderTypeMirror: {
      auto settings = new MirrorSettings(shaderIdStr, 0);
      auto shader = std::make_shared<MirrorShader>(settings);
      shader.get()->setup();
      shaders.push_back(shader);
      return;
    }
    case ShaderTypeTransform: {
      auto settings = new TransformSettings(shaderIdStr, 0);
      auto shader = std::make_shared<TransformShader>(settings);
      shader.get()->setup();
      shaders.push_back(shader);
      return;
    }
    case ShaderTypeFeedback: {
      auto settings = new FeedbackSettings(shaderIdStr, chainerId, 0);
      auto shader = std::make_shared<FeedbackShader>(settings);
      shader.get()->setup();
      feedbackShaders.push_back(shader);
      shaders.push_back(shader);
      return;
    }
    case ShaderTypeAscii: {
      auto settings = new AsciiSettings(shaderIdStr, 0);
      auto shader = std::make_shared<AsciiShader>(settings);
      shader.get()->setup();
      shaders.push_back(shader);
      return;
    }
    case ShaderTypeKaleidoscope: {
      auto settings = new KaleidoscopeSettings(shaderIdStr, 0);
      auto shader = std::make_shared<KaleidoscopeShader>(settings);
      shader.get()->setup();
      shaders.push_back(shader);
      return;
    }
  }
}

void ShaderChainer::saveFeedbackFrame(std::shared_ptr<ofTexture> frame) {
  if (feedbackDestination) {
    feedbackDestination->pushFrame(frame);
  }
}
 
void ShaderChainer::deleteShader(shared_ptr<Shader> shader) {
  auto it = std::find(shaders.begin(), shaders.end(), shader);
  if (it != shaders.end()) {
    shaders.erase(it);
  }
}

json ShaderChainer::serialize() {
  json j;
  j["shaders"] = json::array();
  j["chainerId"] = chainerId;
  
  for (auto shader : shaders) {
    j["shaders"].push_back(shader->serialize());
  }
  return j; 
}

void ShaderChainer::load(json j) {
  chainerId = j["chainerId"];
  for (auto shader : shaders) {
    shader.reset();
  }
  shaders.clear();
  for (auto shaderJson : j["shaders"]) {
    ShaderType shaderType = shaderJson["shaderType"];
    std::string shaderId = shaderJson["shaderId"];
    
    switch (shaderType) {
      case ShaderTypeNone:
        continue;
      case ShaderTypeHSB: {
        auto settings = new HSBSettings(shaderId, shaderJson);
        auto shader = std::make_shared<HSBShader>(settings);
        shader->setup();
        shaders.push_back(shader);
        continue;
      }
      case ShaderTypeBlur: {
        auto settings = new BlurSettings(shaderId, shaderJson);
        auto shader = std::make_shared<BlurShader>(settings);
        shader.get()->setup();
        shaders.push_back(shader);
        continue;
      }
      case ShaderTypePixelate: {
        auto settings = new PixelSettings(shaderId, shaderJson);
        auto shader = std::make_shared<PixelShader>(settings);
        shader.get()->setup();
        shaders.push_back(shader);
        continue;
      }
      case ShaderTypeGlitch:
        continue;
      case ShaderTypeMirror: {
        auto settings = new MirrorSettings(shaderId, shaderJson);
        auto shader = std::make_shared<MirrorShader>(settings);
        shader.get()->setup();
        shaders.push_back(shader);
        continue;
      }
      case ShaderTypeTransform: {
        auto settings = new TransformSettings(shaderId, shaderJson);
        auto shader = std::make_shared<TransformShader>(settings);
        shader.get()->setup();
        shaders.push_back(shader);
        continue;
      }
      case ShaderTypeFeedback: {
        auto settings = new FeedbackSettings(shaderId, chainerId, shaderJson);
        auto shader = std::make_shared<FeedbackShader>(settings);
        shader.get()->setup();
        feedbackShaders.push_back(shader);
        shaders.push_back(shader);
        continue;
      }
      case ShaderTypeAscii: {
        auto settings = new AsciiSettings(shaderId, shaderJson);
        auto shader = std::make_shared<AsciiShader>(settings);
        shader.get()->setup();
        shaders.push_back(shader);
        return;
      }
    }
  }
}

void ShaderChainer::registerFeedbackDestination() {
  std::shared_ptr<FeedbackSource> source = std::make_shared<FeedbackSource>(chainerId, sourceName);
  FeedbackSourceService::getService()->registerFeedbackSource(source);
  feedbackDestination = source;
}

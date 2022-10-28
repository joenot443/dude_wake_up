//
//  ShaderChainer.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#include "ShaderChainer.hpp"
#include "ofMain.h"
#include "Console.hpp"
#include "PixelShader.hpp"
#include "GlitchShader.hpp"
#include "HSBShader.hpp"
#include "FeedbackShader.hpp"
#include "BlurShader.hpp"

ofFbo ShaderChainer::fboChainingShaders(ofFbo texture) {
  auto ping = ofFbo();
  if (!ping.isAllocated()) {
    ping.allocate(texture.getWidth(), texture.getHeight());
  } else {
    ping.begin();
    ofClear(255, 255, 255);
    ping.end();
  }
  
  ofFbo pong = texture;
  
  ofFbo *canv = &ping;
  ofFbo *tex = &pong;
  
  bool flip = false;
  for (auto shader : shaders) {
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
  return *canv;
}

void ShaderChainer::pushShader(ShaderType shaderType) {
  switch (shaderType) {
    case ShaderTypeNone:
      return;
    case ShaderTypeHSB: {
      auto settings = new HSBSettings(settingsId);
      auto shader = new HSBShader(settings);
      shader->setup();
      shaders.push_back(shader);
      return;
    }
    case ShaderTypeBlur: {
      auto settings = new BlurSettings(settingsId);
      auto shader = new BlurShader(settings);
      shader->setup();
      shaders.push_back(shader);
      return;
    }
    case ShaderTypePixelate: {
      auto settings = new PixelSettings(settingsId);
      auto shader = new PixelShader(settings);
      shader->setup();
      shaders.push_back(shader);
      return;
    }
    case ShaderTypeGlitch:
      return;
    case ShaderTypeFeedback: {
      auto settings = new FeedbackSettings(settingsId, 0);
      auto shader = new FeedbackShader(settings, 0);
      shader->setup();
      feedbackShaders.push_back(shader);
      shaders.push_back(shader);
      return;
    }
  }
}

void ShaderChainer::deleteShader(Shader *shader) {
  auto it = std::find(shaders.begin(), shaders.end(), shader);
  if (it != shaders.end()) {
    shaders.erase(it);
  }
}


json ShaderChainer::serialize() {
  json j;
  j["shaders"] = json::array();
  j["settingsId"] = settingsId;
  
  for (auto shader : shaders) {
    j["shaders"].push_back(shader->serialize());
  }
  return j; 
}

void ShaderChainer::load(json j) {
  settingsId = j["settingsId"];
  for (auto shader : shaders) {
    delete shader;
  }
  shaders.clear();
  for (auto shaderJson : j["shaders"]) {
    ShaderType shaderType = shaderJson["shaderType"];
    pushShader(shaderType);
    shaders.back()->settings->load(shaderJson);
    shaders.back()->setup();
  }
}

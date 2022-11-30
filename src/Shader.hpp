//
//  Shader.h
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef Shader_h
#define Shader_h
#include "ofMain.h"
#include "VideoSettings.hpp"
#include "ofxImGui.h"
#include "Console.hpp"
#include "ShaderType.hpp"
#include "ShaderSettings.hpp"
#include "FeedbackSource.hpp"
#include <stdio.h>
#include "json.hpp"

using json = nlohmann::json;

struct Shader {
  Shader(ShaderSettings *settings) : settings(std::unique_ptr<ShaderSettings>(settings)) {};
  
  std::unique_ptr<ShaderSettings> settings;
  virtual void setup() {};
  virtual void shade(ofFbo *frame, ofFbo *canvas) {};
  virtual void clear() {};
  virtual std::string name() { return shaderTypeName(type()); };
  virtual bool enabled() { return true; };
  virtual bool hasFrameBuffer() { return false; };
  void saveFrame(ofFbo *frame) {};
  
  virtual ShaderType type() { return ShaderTypeNone; };
  
  virtual void drawSettings() {};
  
  json serialize() {
    json j = settings->serialize();
    j["shaderType"] = type();
    j["shaderId"] = settings->shaderId;
    return j;
  };
};

#endif /* Shader_h */

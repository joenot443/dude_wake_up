//
//  Shader.h
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef Shader_h
#define Shader_h
#include "ofMain.h"
#include "VideoSettings.h"
#include "ofxImGui.h"
#include "Console.hpp"
#include "ShaderType.h"
#include "ShaderSettings.h"
#include <stdio.h>

struct Shader {
//  Shader(ShaderSettings settings) : settings(settings) {};
  
  virtual void setup() {};
  virtual void shade(ofFbo *frame, ofFbo *canvas) {};
  virtual void clear() {};
  virtual std::string name() { return ""; };
  virtual bool enabled() { return false; };
  virtual ShaderType type() { return ShaderTypeNone; };
  
  virtual void drawSettings() {};
//  ShaderSettings settings {};
};

#endif /* Shader_h */

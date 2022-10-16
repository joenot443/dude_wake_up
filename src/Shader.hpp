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
#include <stdio.h>

struct Shader {
  
  virtual void setup() {};
  virtual void shade(ofFbo *frame, ofFbo *canvas) {};
  virtual void clear() {};
  virtual bool enabled() { return false; };
};

#endif /* Shader_h */

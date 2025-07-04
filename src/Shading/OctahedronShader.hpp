//
//  OctahedronShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef OctahedronShader_hpp
#define OctahedronShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct OctahedronSettings: public ShaderSettings {
  std::shared_ptr<Parameter> enableBackground;
  
	public:
  OctahedronSettings(std::string shaderId, json j, std::string name) :
  enableBackground(std::make_shared<Parameter>("Enable Background", ParameterType_Bool)),
  ShaderSettings(shaderId, j, name) {
    parameters = {enableBackground};
    registerParameters();
  };
};

class OctahedronShader: public Shader {
public:

  OctahedronSettings *settings;
  OctahedronShader(OctahedronSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
shader.load("shaders/Octahedron");
shader.load("shaders/Octahedron");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    
    
    canvas->begin();
    shader.begin();
    // Clear the frame
    ofClear(0,0,0,255);
    ofClear(0,0,0,0);
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform1i("enableBackground", settings->enableBackground->intValue);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }

    int inputCount() override {
    return 1;
  }
ShaderType type() override {
    return ShaderTypeOctahedron;
  }

  void drawSettings() override {
    
    CommonViews::ShaderCheckbox(settings->enableBackground);
  }
};

#endif

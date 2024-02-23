//
//  PaintShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef PaintShader_hpp
#define PaintShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct PaintSettings: public ShaderSettings {
  PaintSettings(std::string shaderId, json j) :

  ShaderSettings(shaderId, j) {
    parameters = { };
    oscillators = { };
    load(j);
    registerParameters();
  };
};

struct PaintShader: Shader {
  PaintSettings *settings;
  ofFbo lastFrame;
  bool shouldClear;
  PaintShader(PaintSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    shader.load("shaders/Paint");
    shouldClear = true;
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    if (!lastFrame.isAllocated() || lastFrame.getWidth() != frame->getWidth() || lastFrame.getHeight() != frame->getHeight()) {
      lastFrame.allocate(frame->getWidth(), frame->getHeight());
    }
    // Paint new frame onto lastFrame
    lastFrame.begin();
    if (shouldClear) {
      ofClear(0,0,0, 255);
      ofClear(0,0,0, 0);
    } else {
      lastFrame.draw(0, 0);
    }
    frame->draw(0, 0);
    lastFrame.end();
    
    // Paint the lastFrame onto canvas
    canvas->begin();
    if (shouldClear) {
      ofClear(0,0,0, 255);
      ofClear(0,0,0, 0);
      frame->draw(0, 0);
      shouldClear = false;
    } else {
      lastFrame.draw(0, 0);
    }
    canvas->end();
  }

  void clear() override {
    
  }

  ShaderType type() override {
    return ShaderTypePaint;
  }

  void drawSettings() override {
    CommonViews::H3Title("Paint");

    if (ImGui::Button("Clear Buffer")) {
      shouldClear = true;
    }
  }
};

#endif /* PaintShader_hpp */

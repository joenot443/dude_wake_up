//
//  ColorKeyMaskMakerShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef ColorKeyMaskMakerShader_hpp
#define ColorKeyMaskMakerShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct ColorKeyMaskMakerSettings: public ShaderSettings {
public:
  std::shared_ptr<Parameter> invert;
  std::shared_ptr<Parameter> drawInput;
  std::shared_ptr<Parameter> tolerance;
  std::shared_ptr<Parameter> color;
  std::shared_ptr<Parameter> drawBackground;
  std::shared_ptr<WaveformOscillator> toleranceOscillator;
  
  ColorKeyMaskMakerSettings(std::string shaderId, json j, std::string name) :
  tolerance(std::make_shared<Parameter>("tolerance", 0.1, 0.0, 1.0)),
  color(std::make_shared<Parameter>("color", 0.0, ParameterType_Color)),
  drawInput(std::make_shared<Parameter>("drawInput", 0.0, 0.0, 1.0, ParameterType_Bool)),
  drawBackground(std::make_shared<Parameter>("drawBackground", 0.0, 0.0, 1.0, ParameterType_Bool)),
  invert(std::make_shared<Parameter>("Invert", 0.0, 0.0, 1.0, ParameterType_Bool)),
  toleranceOscillator(std::make_shared<WaveformOscillator>(tolerance)),
  ShaderSettings(shaderId, j, name) {
    parameters = { tolerance, color, invert, drawInput };
    oscillators = { toleranceOscillator };
    load(j);
    registerParameters();
  };
};

class ColorKeyMaskMakerShader: public Shader {
public:
  ColorKeyMaskMakerSettings *settings;
  ColorKeyMaskMakerShader(ColorKeyMaskMakerSettings *settings) : settings(settings), Shader(settings) {};
  
  ofTexture auxTexture(std::shared_ptr<ofFbo> frame)
  {
    // Return typical feedback frame if auxillary isn't connected
    if (!hasOutputAtSlot(OutputSlotAux)) {
      return lastFrame->getTexture();
    }
    
    // First shader in the feedback aux chain
    std::shared_ptr<Shader> auxShader = std::dynamic_pointer_cast<Shader>(outputAtSlot(OutputSlotAux));
    
    // Final descendent of the feedback aux chain
    std::shared_ptr<Shader> terminalAuxShader = std::dynamic_pointer_cast<Shader>(auxShader->terminalDescendent());
    auxShader->traverseFrame(frame, 0);

    return terminalAuxShader->lastFrame->getTexture();
  }
  
  
  void setup() override {
    shader.load("shaders/ColorKeyMaskMaker");
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    if (canvas == nullptr) return;
    
    ofTexture auxText = hasOutputAtSlot(OutputSlotAux) ? auxTexture(frame) : frame->getTexture();
    
    canvas->begin();
    shader.begin();
//    // Clear the frame
    ofClear(0,0,0, 255);
    ofClear(0,0,0, 0);
    shader.setUniformTexture("tex", frame->getTexture(), 0);
    shader.setUniformTexture("auxTex", auxText, 4);
    shader.setUniform1f("tolerance", settings->tolerance->value);
    shader.setUniform1i("drawTex", settings->drawInput->intParamValue());
    shader.setUniform1i("drawBackground", settings->drawBackground->intParamValue());
    shader.setUniform1i("invert", settings->invert->intValue);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform4f("chromaKey", settings->color->color->data()[0], settings->color->color->data()[1], settings->color->color->data()[2], 1.0);
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
    return ShaderTypeColorKeyMaskMaker;
  }
  
  void drawSettings() override {
    CommonViews::H3Title("ColorKeyMaskMaker");
    CommonViews::ShaderParameter(settings->tolerance, settings->toleranceOscillator);
    CommonViews::ShaderCheckbox(settings->drawInput);
    CommonViews::ShaderCheckbox(settings->invert);
    CommonViews::ShaderCheckbox(settings->drawBackground);
    CommonViews::ShaderColor(settings->color);
  }
};

#endif

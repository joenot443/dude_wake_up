//
//  PsycurvesShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef PsycurvesShader_hpp
#define PsycurvesShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct PsycurvesSettings: public ShaderSettings {
	public:
		std::shared_ptr<Parameter> rotationAngle;
		std::shared_ptr<WaveformOscillator> rotationAngleOscillator;
		
		std::shared_ptr<Parameter> patternDensity;
		std::shared_ptr<WaveformOscillator> patternDensityOscillator;
		
		std::shared_ptr<Parameter> colorCycle;
		
		std::shared_ptr<Parameter> timeScale;
		std::shared_ptr<WaveformOscillator> timeScaleOscillator;
		
		PsycurvesSettings(std::string shaderId, json j, std::string name) :
			rotationAngle(std::make_shared<Parameter>("Rotation Angle", 45.0, 0.0, 360.0)),
			patternDensity(std::make_shared<Parameter>("Pattern Density", 15.0, 5.0, 30.0)),
			colorCycle(std::make_shared<Parameter>("Color Cycle", ParameterType_Color)),
			timeScale(std::make_shared<Parameter>("Time Scale", 1.0, 0.1, 5.0)),
			rotationAngleOscillator(std::make_shared<WaveformOscillator>(rotationAngle)),
			patternDensityOscillator(std::make_shared<WaveformOscillator>(patternDensity)),
			timeScaleOscillator(std::make_shared<WaveformOscillator>(timeScale)),
			ShaderSettings(shaderId, j, name) {
				// Default to a vibrant purple
				colorCycle->setColor({1.0, 1.0, 1.0, 1.0});
				parameters = { rotationAngle, patternDensity, timeScale };
				oscillators = { rotationAngleOscillator, patternDensityOscillator, timeScaleOscillator };
				registerParameters();
			};
};

class PsycurvesShader: public Shader {
public:

  PsycurvesSettings *settings;
  PsycurvesShader(PsycurvesSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    shader.load("shaders/Psycurves");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("time", TimeService::getService()->timeParam->value);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    
    // Pass new uniforms
    float rotationRadians = settings->rotationAngle->value * M_PI / 180.0;
    shader.setUniform1f("rotationAngle", rotationRadians);
    shader.setUniform1f("patternDensity", settings->patternDensity->value);
    shader.setUniform3f("colorCycle", 
      settings->colorCycle->color->data()[0],
      settings->colorCycle->color->data()[1],
      settings->colorCycle->color->data()[2]);
    shader.setUniform1f("timeScale", settings->timeScale->value);
    
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
    return ShaderTypePsycurves;
  }

  void drawSettings() override {
    CommonViews::ShaderParameter(settings->rotationAngle, settings->rotationAngleOscillator);
    CommonViews::ShaderParameter(settings->patternDensity, settings->patternDensityOscillator);
    CommonViews::ShaderColor(settings->colorCycle);
    CommonViews::ShaderParameter(settings->timeScale, settings->timeScaleOscillator);
  }
};

#endif

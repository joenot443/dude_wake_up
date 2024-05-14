//
//  FrequencyVisualizerShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef FrequencyVisualizerShader_hpp
#define FrequencyVisualizerShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct FrequencyVisualizerSettings : public ShaderSettings
{
  std::shared_ptr<Parameter> shaderValue;
  std::shared_ptr<WaveformOscillator> shaderWaveformOscillator;

  FrequencyVisualizerSettings(std::string shaderId, json j, std::string name) : shaderValue(std::make_shared<Parameter>("shaderValue", 1.0, -1.0, 2.0)),
                                                              shaderWaveformOscillator(std::make_shared<WaveformOscillator>(shaderValue)),
                                                              ShaderSettings(shaderId, j, name)
  {
    parameters = {shaderValue};
    oscillators = {shaderWaveformOscillator};
    load(j);
    registerParameters();
  };
};

struct FrequencyVisualizerShader : Shader
{
  FrequencyVisualizerSettings *settings;
  FrequencyVisualizerShader(FrequencyVisualizerSettings *settings) : settings(settings), Shader(settings){};
  ofShader shader;
  void setup() override
  {
    shader.load("shaders/FrequencyVisualizer");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override
  {
    auto source = AudioSourceService::getService()->selectedAudioSource;

    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("color", settings->shaderValue->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    if (source != nullptr && source->audioAnalysis.smoothSpectrum.size() > 0)
      shader.setUniform1fv("audio", &source->audioAnalysis.smoothSpectrum[0],
                           256);

    ofPushMatrix();
    ofScale(1, 1);
    frame->getTexture().drawSubsection(0, 0, frame->getWidth() / 2, frame->getHeight(), 0, 0);
    ofPopMatrix();

    ofPushMatrix();                     // push the current transformation matrix
    ofScale(-1, 1);                    // flip the x-axis
    ofTranslate(-frame->getWidth(), 0); // translate the drawing back to its original position
    frame->getTexture().drawSubsection(0, 0, frame->getWidth() / 2, frame->getHeight(), 0, 0);
    ofPopMatrix(); // reset the transformation matrix

    shader.end();
    canvas->end();
  }

  void clear() override
  {
  }

    int inputCount() override {
    return 1;
  }
ShaderType type() override { 
    return ShaderTypeFrequencyVisualizer;
  }

  void drawSettings() override
  {
    CommonViews::H3Title("FrequencyVisualizer");

    CommonViews::ShaderParameter(settings->shaderValue, settings->shaderWaveformOscillator);
  }
};

#endif /* FrequencyVisualizerShader_hpp */

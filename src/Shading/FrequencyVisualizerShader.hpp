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
  std::shared_ptr<Parameter> amount;
  std::shared_ptr<WaveformOscillator> amountOscillator;
  std::shared_ptr<Parameter> verticalStart;
  std::shared_ptr<WaveformOscillator> verticalStartOscillator;
  std::shared_ptr<Parameter> verticalBarCountParam;
  std::shared_ptr<WaveformOscillator> verticalBarCountOscillator;
  std::shared_ptr<Parameter> horizontalSpacingParam;
  std::shared_ptr<WaveformOscillator> horizontalSpacingOscillator;
  std::shared_ptr<Parameter> verticalSpacingParam;
  std::shared_ptr<WaveformOscillator> verticalSpacingOscillator;

  FrequencyVisualizerSettings(std::string shaderId, json j, std::string name) :
    amount(std::make_shared<Parameter>("Amount", 1.0, 0.0, 5.0)),
    amountOscillator(std::make_shared<WaveformOscillator>(amount)),
    verticalStart(std::make_shared<Parameter>("Vertical Start", 0.3, 0.0, 1.0)),
    verticalStartOscillator(std::make_shared<WaveformOscillator>(verticalStart)),
    verticalBarCountParam(std::make_shared<Parameter>("Vertical Bars", 256.0, 1.0, 256.0)),
    verticalBarCountOscillator(std::make_shared<WaveformOscillator>(verticalBarCountParam)),
    horizontalSpacingParam(std::make_shared<Parameter>("H Spacing", 1.0, 0.0, 2.0)),
    horizontalSpacingOscillator(std::make_shared<WaveformOscillator>(horizontalSpacingParam)),
    verticalSpacingParam(std::make_shared<Parameter>("V Spacing", 0.180, 0.0, 2.0)),
    verticalSpacingOscillator(std::make_shared<WaveformOscillator>(verticalSpacingParam)),
    ShaderSettings(shaderId, j, name)
  {
    parameters = {amount, verticalStart, verticalBarCountParam, horizontalSpacingParam, verticalSpacingParam};
    oscillators = {amountOscillator, verticalStartOscillator, verticalBarCountOscillator, horizontalSpacingOscillator, verticalSpacingOscillator};
    load(j);
    registerParameters();
  };
};

struct FrequencyVisualizerShader : Shader
{
  FrequencyVisualizerSettings *settings;
  FrequencyVisualizerShader(FrequencyVisualizerSettings *settings) : settings(settings), Shader(settings){};

  void setup() override
  {
    shader.load("shaders/Frequency Visualizer");
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override
  {
    auto source = AudioSourceService::getService()->selectedAudioSource;

    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1f("amount", settings->amount->value);
    shader.setUniform1f("time", ofGetElapsedTimef());
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    shader.setUniform1f("verticalStart", settings->verticalStart->value);
    shader.setUniform1f("verticalBarCount", settings->verticalBarCountParam->value);
    shader.setUniform1f("horizontalSpacing", settings->horizontalSpacingParam->value);
    shader.setUniform1f("verticalSpacing", settings->verticalSpacingParam->value);
    if (source != nullptr && source->audioAnalysis.smoothSpectrum.size() == 256)
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
    CommonViews::ShaderParameter(settings->amount, settings->amountOscillator);
    CommonViews::ShaderParameter(settings->verticalStart, settings->verticalStartOscillator);
    CommonViews::ShaderParameter(settings->verticalBarCountParam, settings->verticalBarCountOscillator);
    CommonViews::ShaderParameter(settings->horizontalSpacingParam, settings->horizontalSpacingOscillator);
    CommonViews::ShaderParameter(settings->verticalSpacingParam, settings->verticalSpacingOscillator);
  }
};

#endif

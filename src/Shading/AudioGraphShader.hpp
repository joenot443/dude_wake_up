//
//  AudioGraphShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef AudioGraphShader_hpp
#define AudioGraphShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include "AudioSourceService.hpp"
#include <stdio.h>

struct AudioGraphSettings: public ShaderSettings {
  std::shared_ptr<Parameter> thickness;
  std::shared_ptr<Parameter> amplitude;
  std::shared_ptr<Parameter> yTranslate;
  std::shared_ptr<Parameter> stroke;
  std::shared_ptr<Parameter> strokeThickness;
  std::shared_ptr<Parameter> color;
  std::shared_ptr<Parameter> strokeColor;

  std::shared_ptr<WaveformOscillator> thicknessOscillator;
  std::shared_ptr<WaveformOscillator> amplitudeOscillator;
  std::shared_ptr<WaveformOscillator> yTranslateOscillator;

  AudioGraphSettings(std::string shaderId, json j) :
  thickness(std::make_shared<Parameter>("Thickness", 2.0, 0.1, 20.0)),
  amplitude(std::make_shared<Parameter>("Amplitude", 1.0, 0.0, 5.0)),
  yTranslate(std::make_shared<Parameter>("Y Translate", 0.5, 0.0, 1.0)),
  stroke(std::make_shared<Parameter>("Stroke", 0.0, 0.0, 1.0, ParameterType_Bool)),
  strokeThickness(std::make_shared<Parameter>("Stroke Thickness", 1.0, 0.1, 10.0)),
  color(std::make_shared<Parameter>("Color", ParameterType_Color)),
  strokeColor(std::make_shared<Parameter>("Stroke Color", ParameterType_Color)),
  thicknessOscillator(std::make_shared<WaveformOscillator>(thickness)),
  amplitudeOscillator(std::make_shared<WaveformOscillator>(amplitude)),
  yTranslateOscillator(std::make_shared<WaveformOscillator>(yTranslate)),
  ShaderSettings(shaderId, j, "AudioGraph") {
    parameters = { thickness, amplitude, yTranslate, stroke, strokeThickness, color, strokeColor };
    oscillators = { thicknessOscillator, amplitudeOscillator, yTranslateOscillator };

    // Set default colors
    color->setColor({0.39, 0.78, 1.0, 1.0}); // Light blue
    strokeColor->setColor({1.0, 1.0, 1.0, 1.0}); // White

    load(j);
    registerParameters();
  };
};

struct AudioGraphShader: Shader {
  AudioGraphSettings *settings;
  AudioGraphShader(AudioGraphSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    // No shader needed - using direct draw calls
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    auto source = AudioSourceService::getService()->selectedAudioSource;

    canvas->begin();
    ofClear(0, 0, 0, 255);  // Clear to solid black

    // Draw waveform if audio source exists
    if (source != nullptr) {
      std::vector<float> waveform = source->audioAnalysis.getSafeWaveform();

      if (waveform.size() >= 256) {
        float width = canvas->getWidth();
        float height = canvas->getHeight();
        float centerY = height * (1. - settings->yTranslate->value);
        float amp = settings->amplitude->value * height * 0.5f;
        float lineThickness = settings->thickness->value;

        // Get color values from parameter (stored as std::array<float, 4>)
        auto fillColorArray = *settings->color->color;
        ofFloatColor fillColor(fillColorArray[0], fillColorArray[1], fillColorArray[2], fillColorArray[3]);

        auto outlineColorArray = *settings->strokeColor->color;
        ofFloatColor outlineColor(outlineColorArray[0], outlineColorArray[1], outlineColorArray[2], outlineColorArray[3]);

        bool drawStroke = settings->stroke->boolValue;
        float strokeWidth = settings->strokeThickness->value;

        // Create polyline for the waveform
        ofPolyline line;
        for (size_t i = 0; i < 256; i++) {
          float x = (float)i / 255.0f * width;
          // Waveform is bipolar (-1 to 1), so both positive and negative values
          float y = centerY - (waveform[i] * amp);
          line.addVertex(x, y);
        }

        // Get smoothed version using Bezier interpolation
        ofPolyline smoothedLine = line.getSmoothed(8);

        // Draw stroke/outline if enabled
        if (drawStroke && strokeWidth > 0) {
          ofSetColor(outlineColor);
          ofSetLineWidth(lineThickness + strokeWidth * 2.0f);
          smoothedLine.draw();
        }

        // Draw main waveform
        ofSetColor(fillColor);
        ofSetLineWidth(lineThickness);
        smoothedLine.draw();
      }
    }

    canvas->end();
  }

  void clear() override {

  }

  int inputCount() override {
    return 1;
  }

  ShaderType type() override {
    return ShaderTypeAudioGraph;
  }

  void drawSettings() override {
    CommonViews::H3Title("AudioGraph");
    CommonViews::ShaderParameter(settings->amplitude, settings->amplitudeOscillator);
    CommonViews::ShaderParameter(settings->thickness, settings->thicknessOscillator);
    CommonViews::ShaderParameter(settings->yTranslate, settings->yTranslateOscillator);
    CommonViews::ShaderCheckbox(settings->stroke);
    if (settings->stroke->boolValue) {
      CommonViews::Slider("Stroke Thickness", "strokeThickness", settings->strokeThickness);
      CommonViews::ShaderColor(settings->strokeColor);
    }
    CommonViews::ShaderColor(settings->color);
  }
};

#endif /* AudioGraphShader_hpp */

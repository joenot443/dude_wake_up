//
//  AudioMountainsShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef AudioMountainsShader_hpp
#define AudioMountainsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "AudioSourceService.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct AudioMountainsSettings: public ShaderSettings {
  AudioMountainsSettings(std::string shaderId, json j) :
  ShaderSettings(shaderId, j) {
    
  };
};

struct AudioMountainsShader: Shader {
  AudioMountainsSettings *settings;
  AudioMountainsShader(AudioMountainsSettings *settings) : settings(settings), Shader(settings) {};
  ofShader shader;
  void setup() override {
    #ifdef TESTING
shader.load("shaders/AudioMountains");
#endif
#ifdef RELEASE
shader.load("shaders/AudioMountains");
#endif
  }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    auto source = AudioSourceService::getService()->selectedAudioSource;
    canvas->begin();
    shader.begin();
    if (source != nullptr && source->audioAnalysis.smoothSpectrum.size() > 0)
      shader.setUniform1fv("audio", &source->audioAnalysis.smoothSpectrum[0], 256);
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void clear() override {
    
  }

  ShaderType type() override {
    return ShaderTypeAudioMountains;
  }

  void drawSettings() override {
  }
};

#endif /* AudioMountainsShader_hpp */

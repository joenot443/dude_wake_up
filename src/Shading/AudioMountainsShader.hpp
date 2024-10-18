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
	public:
  AudioMountainsSettings(std::string shaderId, json j, std::string name) :
  ShaderSettings(shaderId, j, name) {
    
  };
};

class AudioMountainsShader: public Shader {
public:

  AudioMountainsSettings *settings;
  AudioMountainsShader(AudioMountainsSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    #ifdef DEBUG
shader.load("shaders/AudioMountains");
#endif
#ifdef RELEASE
shader.load("shaders/AudioMountains");
#endif
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
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

    int inputCount() override {
    return 1;
  }
ShaderType type() override {
    return ShaderTypeAudioMountains;
  }

  void drawSettings() override {
  }
};

#endif /* AudioMountainsShader_hpp */

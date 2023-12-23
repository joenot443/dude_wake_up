//
//  AudioBumperShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef AudioBumperShader_hpp
#define AudioBumperShader_hpp

#include "AudioSourceService.hpp"
#include "CommonViews.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "Shader.hpp"
#include "ShaderSettings.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include <stdio.h>

struct AudioBumperSettings : public ShaderSettings {
	public:
  AudioBumperSettings(std::string shaderId, json j)
  : ShaderSettings(shaderId, j){
    
  };
};

class AudioBumperShader : public Shader {
public:

  AudioBumperSettings *settings;
  AudioBumperShader(AudioBumperSettings *settings)
  : settings(settings), Shader(settings){};
  ofShader shader;
  void setup() override {
#ifdef TESTING
    shader.load("shaders/AudioBumper");
#endif
#ifdef RELEASE
    shader.load("shaders/AudioBumper");
#endif
    
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    auto source = AudioSourceService::getService()->selectedAudioSource;
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    if (source != nullptr && source->audioAnalysis.smoothSpectrum.size() > 0)
      shader.setUniform1fv("audio", &source->audioAnalysis.smoothSpectrum[0],
                           256);
    // Flip the frame vertically
    ofPushMatrix();
    ofTranslate(0, frame->getHeight());
    ofScale(1, -1);
    frame->draw(0, 0);
    ofPopMatrix();
    shader.end();
    canvas->end();
  }
  
  void clear() override {}
  
  ShaderType type() override { return ShaderTypeAudioBumper; }
  
  void drawSettings() override {
    CommonViews::H3Title("AudioBumper");
  }
};

#endif /* AudioBumperShader_hpp */

//
//  MainStageView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/14/22.
//

#ifndef MainStageView_hpp
#define MainStageView_hpp

#include <CoreFoundation/CoreFoundation.h>
#include "AudioSourceBrowserView.hpp"
#include <ableton/Link.hpp>
#include "NodeLayoutView.hpp"
#include "OscillatorView.hpp"
#include "ShaderBrowserView.hpp"
#include "VideoSource.hpp"
#include "VideoSourceBrowserView.hpp"
#include "SubmitFeedbackView.hpp"
#include "WelcomeScreenView.hpp"
#include "StrandBrowserView.hpp"
#include "StageModeView.hpp"
#include "OscillatorPanelView.hpp"
#include "ofMain.h"
#include <stdio.h>

struct MainStageView {
private:
  std::shared_ptr<VideoSource> selectedVideoSource;
  ShaderType selectedShaderType;
  msa::BPMTapper bpmTapper;
  bool drawFPS = false;
  
  VideoSourceBrowserView videoSourceBrowserView = VideoSourceBrowserView();
  ShaderBrowserView shaderBrowserView = ShaderBrowserView();
  AudioSourceBrowserView audioSourceBrowserView = AudioSourceBrowserView();
  StrandBrowserView strandBrowserView = StrandBrowserView({});
  OscillatorView oscillatorView = OscillatorView();
  OscillatorPanelView oscillatorPanelView = OscillatorPanelView();
  SubmitFeedbackView submitFeedbackView = SubmitFeedbackView();
  StageModeView stageModeView = StageModeView();
  WelcomeScreenView welcomeScreenView = WelcomeScreenView();
  
  void drawMenu();
  void drawVideoSourceBrowser();
  void drawOutputBrowser();
  void drawShaderBrowser();

public:
  void setup();
  void update();
  void draw();
  void keyReleased(int key);
  void keyPressed(int key);
  
  void loadDirectory(std::string directory);
};

#endif /* MainStageView_hpp */

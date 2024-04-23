//
//  MainStageView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/14/22.
//

#ifndef MainStageView_hpp
#define MainStageView_hpp

#include "AudioSourceBrowserView.hpp"
#include "NodeLayoutView.hpp"
#include "OscillatorView.hpp"
#include "ShaderBrowserView.hpp"
#include "VideoSource.hpp"
#include "VideoSourceBrowserView.hpp"
#include "SubmitFeedbackView.hpp"
#include "StrandBrowserView.hpp"
#include "StageModeView.hpp"
#include "ofMain.h"
#include <stdio.h>

struct MainStageView {
private:
  std::shared_ptr<VideoSource> selectedVideoSource;
  ShaderType selectedShaderType;
  msa::BPMTapper bpmTapper;
  
  
  VideoSourceBrowserView videoSourceBrowserView = VideoSourceBrowserView();
  ShaderBrowserView shaderBrowserView = ShaderBrowserView();
  AudioSourceBrowserView audioSourceBrowserView = AudioSourceBrowserView();
  StrandBrowserView strandBrowserView = StrandBrowserView({});
  OscillatorView oscillatorView = OscillatorView();
  SubmitFeedbackView submitFeedbackView = SubmitFeedbackView();
  StageModeView stageModeView = StageModeView();
  
  void drawMenu();
  void drawMasks();
  void drawVideoSourceBrowser();
  void drawOutputBrowser();
  void drawShaderBrowser();

public:
  void setup();
  void update();
  void draw();
  void keyReleased(int key);
  
  NodeLayoutView nodeLayoutView;
};

#endif /* MainStageView_hpp */

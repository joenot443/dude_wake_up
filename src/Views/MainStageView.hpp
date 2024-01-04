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
  OscillatorView oscillatorView = OscillatorView();
  FileBrowserView fileBrowserView = FileBrowserView(FileBrowserType_JSON);
  SubmitFeedbackView submitFeedbackView = SubmitFeedbackView();
  
  void drawMenu();
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

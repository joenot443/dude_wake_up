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
#include "OutputBrowserView.hpp"
#include "ShaderBrowserView.hpp"
#include "ShaderChainerSettingsView.hpp"
#include "ShaderChainerView.hpp"
#include "ShaderChainersStageView.hpp"
#include "VideoSource.hpp"
#include "VideoSourceBrowserView.hpp"
#include "ShaderChainerPreviewView.hpp"
#include "ofMain.h"
#include <stdio.h>

struct MainStageView {
private:
  std::shared_ptr<VideoSource> selectedVideoSource;
  ShaderType selectedShaderType;

  std::vector<std::shared_ptr<ShaderChainerView>> shaderChainerViews;


  VideoSourceBrowserView videoSourceBrowserView = VideoSourceBrowserView();
  ShaderBrowserView shaderBrowserView = ShaderBrowserView();
  OutputBrowserView outputBrowserView = OutputBrowserView();
  ShaderChainerSettingsView shaderChainerSettingsView =
      ShaderChainerSettingsView();
  AudioSourceBrowserView audioSourceBrowserView = AudioSourceBrowserView();
  OscillatorView oscillatorView = OscillatorView();
  ShaderChainersStageView shaderChainerStageView = ShaderChainersStageView();
  FileBrowserView fileBrowserView = FileBrowserView(FileBrowserType_JSON);

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

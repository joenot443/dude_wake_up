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
#include "ShaderChainerSettingsView.hpp"
#include "ShaderBrowserView.hpp"
#include "ShaderChainerView.hpp"
#include "ShaderChainersStageView.hpp"
#include "VideoSource.hpp"
#include "VideoSourceBrowserView.hpp"
#include "VideoSourcePreviewView.hpp"
#include "ofMain.h"
#include <stdio.h>

struct MainStageView {
private:
  std::shared_ptr<VideoSource> selectedVideoSource;
  ShaderType selectedShaderType;

  std::vector<std::shared_ptr<ShaderChainerView>> shaderChainerViews;

  NodeLayoutView nodeLayoutView;

  VideoSourceBrowserView videoSourceBrowserView = VideoSourceBrowserView();
  VideoSourcePreviewView videoSourcePreviewView = VideoSourcePreviewView();
  ShaderBrowserView shaderBrowserView = ShaderBrowserView();
  OutputBrowserView outputBrowserView = OutputBrowserView();
  ShaderChainerSettingsView shaderChainerSettingsView =
      ShaderChainerSettingsView();
  AudioSourceBrowserView audioSourceBrowserView = AudioSourceBrowserView();
  OscillatorView oscillatorView = OscillatorView();
  ShaderChainersStageView shaderChainerStageView = ShaderChainersStageView();

  void styleWindow();
  void drawMenu();
  void drawNewShaderChainerButton();
  void drawNewShaderButton();
  void drawVideoSourceBrowser();
  void drawShaderChainerSettings();
  void drawSelectedShader();
  void drawOutputBrowser();
  void drawShaderBrowser();

  void pushShaderChainer(std::shared_ptr<ShaderChainer> shaderChainer);
  void populateShaderChainerViews();

public:
  void setup();
  void update();
  void draw();
  void keyReleased(int key);

};

#endif /* MainStageView_hpp */

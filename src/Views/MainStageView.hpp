//
//  MainStageView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/14/22.
//

#ifndef MainStageView_hpp
#define MainStageView_hpp

#include <stdio.h>
#include "ofMain.h"
#include "VideoSource.hpp"
#include "VideoSourceBrowserView.hpp"
#include "OutputBrowserView.hpp"
#include "OscillatorView.hpp"
#include "VideoSourcePreviewView.hpp"
#include "ShaderChainerSettingsView.hpp"
#include "ShaderChainerView.hpp"

struct MainStageView {
private:
  std::shared_ptr<VideoSource> selectedVideoSource;
  ShaderType selectedShaderType;
  
  std::vector<std::shared_ptr<ShaderChainerView>> shaderChainerViews;
    
  VideoSourceBrowserView videoSourceBrowserView = VideoSourceBrowserView();
  VideoSourcePreviewView videoSourcePreviewView = VideoSourcePreviewView();
  OutputBrowserView outputBrowserView = OutputBrowserView();
  ShaderChainerSettingsView shaderChainerSettingsView = ShaderChainerSettingsView();
  OscillatorView oscillatorView = OscillatorView();
  
  void drawNewShaderChainerButton();
  void drawNewShaderButton();
  void drawVideoSourceBrowser();
  void drawShaderChainerSettings();
  void drawShaderChainers();
  void drawSelectedShader();
  void drawOutputBrowser();
  void drawShaderSelection();
  
  void pushShaderChainer(std::shared_ptr<ShaderChainer> shaderChainer);
  
public:
  void setup();
  void update();
  void draw();
};

#endif /* MainStageView_hpp */

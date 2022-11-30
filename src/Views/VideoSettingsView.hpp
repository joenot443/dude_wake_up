//
//  VideoFeedSettingsView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-09.
//

#ifndef VideoFeedSettingsView_hpp
#define VideoFeedSettingsView_hpp

#include <stdio.h>
#include "VideoSettings.hpp"
#include "VideoStream.hpp"
#include "FileBrowserView.hpp"
#include "VideoSourceBrowserView.hpp"
#include "NewVideoSourceView.hpp"
#include "ShaderChainerView.hpp"

struct VideoSettingsView {
public:
  VideoStream *videoStream;
  void setup();
  void update();
  void draw();
  void teardown();
  
  VideoSettings *videoSettings;
  std::vector<std::shared_ptr<ShaderChainerView>> shaderChainerViews;
  std::vector<std::shared_ptr<ShaderChainer>> shaderChainers;
  std::vector<std::shared_ptr<FeedbackSource>> feedbackSources;
  
  std::function<void(int)> closeStream;
  std::vector<std::string> sourceNames;
  
  int selectedIndex = 0;
  ShaderChainer *selectedChainer;
  ShaderChainerView *selectedChainerView;
  VideoSourceBrowserView videoSourceBrowserView;
  NewVideoSourceView newVideoSourceView;
  
  
  VideoSettingsView(
                    VideoSettings *videoSettings,
                    std::shared_ptr<ShaderChainer> selectedShaderChain,
                    VideoStream *videoStream,
                    std::function<void(int)> closeStream)
  :
  videoSettings(videoSettings),
  closeStream(closeStream),
  videoStream(videoStream),
  shaderChainerViews({
    std::make_shared<ShaderChainerView>( selectedShaderChain )}),
  shaderChainers({selectedShaderChain}),
  selectedChainer(shaderChainers[0].get()),
  selectedChainerView(shaderChainerViews[0].get())
  {};
  
private:
  void styleWindow();
  void pushShaderChainer(std::shared_ptr<ShaderChainer> chainer);
  void selectShaderChainerAtIndex(int i);
  void drawMenuBar();
  void drawFileBrowserView();
  void drawShaderChainerTabs();
  void drawSelectedShader();
  void drawMenu();
  void drawVideoSourceBrowserView();

  bool hasDrawn = false;
};

#endif /* VideoFeedSettingsView_hpp */

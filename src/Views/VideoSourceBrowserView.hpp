//
//  VideoSourceBrowserView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/8/22.
//

#ifndef VideoSourceBrowserView_hpp
#define VideoSourceBrowserView_hpp

#include "TileBrowserView.hpp"
#include "FileBrowserView.hpp"
#include "VideoSource.hpp"
#include "ofMain.h"
#include <stdio.h>


struct VideoSourceBrowserView {
public:
  void setup();
  void refreshSources();
  void update();
  void draw();
  
  void loadDirectory(std::string directory);
  void drawLibraryHeader();
  void setCallback(std::function<void(std::shared_ptr<TileItem>)> callback);

  // New public method to set the current tab
  void setCurrentTab(int tabIndex);
  
  void drawSelectedBrowser();
  
  static int tabForSourceType(VideoSourceType type);
  ImVec2 size = ImVec2(0.0, 0.0);

private:
  ofFbo videoSourceFbo;
  TileBrowserView tileBrowserView = TileBrowserView({});
  FileBrowserView fileBrowserView = FileBrowserView(FileBrowserType_Source);
  std::vector<std::shared_ptr<TileItem>> webcamItems;
  std::vector<std::shared_ptr<TileItem>> shaderItems;
  std::vector<std::shared_ptr<LibraryTileItem>> libraryItems;

  std::string searchQuery;
  bool searchDirty = false;
  std::vector<std::shared_ptr<TileItem>> searchTileItems = {};
  TileBrowserView searchResultsTileBrowserView = TileBrowserView({});

  int currentTab = 0;
};

#endif /* VideoSourceBrowserView_hpp */

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
  
  void drawLibraryHeader();

private:
  ofFbo videoSourceFbo;
  TileBrowserView tileBrowserView = TileBrowserView({});
  FileBrowserView fileBrowserView = FileBrowserView(FileBrowserType_Source);
  std::vector<std::shared_ptr<TileItem>> webcamItems;
  std::vector<std::shared_ptr<TileItem>> shaderItems;
  std::vector<std::shared_ptr<LibraryTileItem>> libraryItems;

  // New member variables for search functionality
  std::string searchQuery;
  bool searchDirty = false;
  std::vector<std::shared_ptr<TileItem>> searchTileItems = {};
  TileBrowserView searchResultsTileBrowserView = TileBrowserView({});
};

#endif /* VideoSourceBrowserView_hpp */

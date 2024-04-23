//
//  ShaderBrowserView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/17/23.
//

#ifndef ShaderBrowserView_hpp
#define ShaderBrowserView_hpp

#include "ShaderType.hpp"
#include "TileBrowserView.hpp"
#include "ParameterService.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include <stdio.h>

struct ShaderBrowserView {
public:
  void setup();
  void update();
  void draw();
  void drawSearchView();
  
  std::string searchQuery = "";
  bool searchDirty = false;
  
  std::vector<std::shared_ptr<TileItem>> searchTileItems = {};

  TileBrowserView searchResultsTileBrowserView = TileBrowserView({});
  TileBrowserView favoritesTileBrowserView = TileBrowserView({});
  TileBrowserView filterTileBrowserView = TileBrowserView({});
  TileBrowserView mixTileBrowserView = TileBrowserView({});
  TileBrowserView basicTileBrowserView = TileBrowserView({});
  TileBrowserView transformTileBrowserView = TileBrowserView({});
  TileBrowserView maskTileBrowserView = TileBrowserView({});
  
  ShaderBrowserView() {
    ParameterService::getService()->subscribeToFavoritesUpdates(
        [this]()
        { setup(); });
  }
};

#endif /* ShaderBrowserView_hpp */

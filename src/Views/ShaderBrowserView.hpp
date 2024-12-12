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
#include "UUID.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include <stdio.h>

struct ShaderBrowserView {
public:
  void setup();
  void update();
  void draw();
  void drawSearchView();
  void setCallback(std::function<void(std::shared_ptr<TileItem>)> callback);
  ImVec2 size = ImVec2(0.0, 0.0);
  std::string searchQuery = "";
  std::string browserId;
  bool searchDirty = false;
  
  std::vector<std::shared_ptr<TileItem>> searchTileItems = {};

  std::unique_ptr<TileBrowserView> searchResultsTileBrowserView;
  std::unique_ptr<TileBrowserView> defaultFavoritesTileBrowserView;
  std::unique_ptr<TileBrowserView> favoritesTileBrowserView;
  std::unique_ptr<TileBrowserView> filterTileBrowserView;
  std::unique_ptr<TileBrowserView> glitchTileBrowserView;
  std::unique_ptr<TileBrowserView> mixTileBrowserView;
  std::unique_ptr<TileBrowserView> basicTileBrowserView;
  std::unique_ptr<TileBrowserView> transformTileBrowserView;
  std::unique_ptr<TileBrowserView> maskTileBrowserView;
  
  int currentTab = 0;
  void setCurrentTab(int tabIndex);
  void drawSelectedBrowser();
  
  ShaderBrowserView() : browserId(UUID::generateUUID()) {
    ParameterService::getService()->subscribeToFavoritesUpdates(
        [this]()
        { setup(); });
  }
};

#endif /* ShaderBrowserView_hpp */

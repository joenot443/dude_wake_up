//
//  TileBrowserView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/16/23.
//

#ifndef TileBrowserView_hpp
#define TileBrowserView_hpp

#include "ofMain.h"
#include "ShaderType.hpp"
#include "TileItem.hpp"
#include <stdio.h>
#include "UUID.hpp"
#include "ofxImGui.h"
#include "AbstractTileBrowserView.hpp"

class TileBrowserView : public AbstractTileBrowserView {
public:
  void setup() override;
  void draw() override;
  void update() override;
  void sortTileItems();
  void setTileItems(std::vector<std::shared_ptr<TileItem>> items) override;
  void setCallback(std::function<void(std::shared_ptr<TileItem>)> callback) override;
  
  float widthFraction = 0.2;
  std::string tileBrowserId;
  int tileCount = 0;
  
  template <typename T>
  TileBrowserView(const std::vector<std::shared_ptr<T>>& items) : tileItems(items.begin(), items.end()) {
    sortTileItems();
  }

  TileBrowserView(std::vector<std::shared_ptr<TileItem>> tileItems) : tileItems(tileItems),
  tileBrowserId(UUID::generateUUID())
  {};
  
private:
  std::vector<std::shared_ptr<TileItem>> tileItems;
  std::function<void(std::shared_ptr<TileItem>)> tileClickCallback;
};

#endif /* TileBrowserView_hpp */

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
#include <functional>
#include <vector>

struct TileContextMenuItem {
  std::string title;
  // Callback invoked when the menu item is selected. The clicked tile is provided.
  std::function<void(std::shared_ptr<TileItem>)> action;
  TileContextMenuItem(const std::string &title,
                      std::function<void(std::shared_ptr<TileItem>)> action)
      : title(title), action(action) {}
};

class TileBrowserView : public AbstractTileBrowserView {
public:
  void setup() override;
  void draw() override;
  void update() override;
  void setTileItems(std::vector<std::shared_ptr<TileItem>> items) override;
  void setCallback(std::function<void(std::shared_ptr<TileItem>)> callback) override;
  void drawTile(std::shared_ptr<TileItem> tile, const ImVec2& tileSize);
  void setContextMenuItems(const std::vector<TileContextMenuItem> &items);
  
  float widthFraction = 0.2;
  std::string tileBrowserId;
  int tileCount = 0;
  bool singleCategory;
  
  template <typename T>
  TileBrowserView(const std::vector<std::shared_ptr<T>>& items, bool singleCategory = false) : tileBrowserId(UUID::generateUUID()), singleCategory(singleCategory) {
    setTileItems(std::vector<std::shared_ptr<TileItem>>(items.begin(), items.end()));
  }

  TileBrowserView(std::vector<std::shared_ptr<TileItem>> items, bool singleCategory = false) : tileBrowserId(UUID::generateUUID()), singleCategory(singleCategory) {
    setTileItems(items);
  }
  
private:
  std::map<std::string, std::vector<std::shared_ptr<TileItem>>> categoryMap;
  std::vector<std::string> categories;  // To maintain category order
  std::string activeCategory = "";
  std::string emptyCategoryId = idString("emptyCategory");
  std::function<void(std::shared_ptr<TileItem>)> tileClickCallback;
  void drawTile(std::shared_ptr<TileItem> tile, const ImVec2& size, const ImVec2& tileSize);
  void applyHeaderStyles(bool isSelected);
  void popHeaderStyles(bool isSelected);
  void drawCategories(const ImVec2& tileSize);
  void drawSingleCategory(const std::string& category, const ImVec2& tileSize);
  std::vector<TileContextMenuItem> contextMenuItems;
};

#endif /* TileBrowserView_hpp */

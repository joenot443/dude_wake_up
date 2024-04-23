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
#include <stdio.h>
#include "UUID.hpp"
#include "ofxImGui.h"

class TileItem {
public:
  virtual ~TileItem() {} 
  std::string name;
  std::string category = "";
  ImTextureID textureID;
  int index;
  ShaderType shaderType;
  // Closure which will be called when the tile is clicked
  std::function<void()> dragCallback;

  TileItem(std::string name, ImTextureID textureID, int index,
           std::function<void()> dragCallback, std::string category = "", ShaderType type = ShaderTypeNone)
      : name(name), textureID(textureID), index(index), shaderType(type), category(category),
        dragCallback(dragCallback){};
};

class TileBrowserView {
public:
  void setup();
  void draw();
  void update();
  void sortTileItems();
  void setTileItems(std::vector<std::shared_ptr<TileItem>> items);
  
  float widthFraction = 0.2;
  std::string tileBrowserId;
  
  template <typename T>
  TileBrowserView(const std::vector<std::shared_ptr<T>>& items) : tileItems(items.begin(), items.end()) {
    sortTileItems();
  }

  TileBrowserView(std::vector<std::shared_ptr<TileItem>> tileItems) : tileItems(tileItems),
  tileBrowserId(UUID::generateUUID())
  {};
  
private:
  std::vector<std::shared_ptr<TileItem>> tileItems;
};

#endif /* TileBrowserView_hpp */

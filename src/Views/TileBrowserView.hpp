//
//  TileBrowserView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/16/23.
//

#ifndef TileBrowserView_hpp
#define TileBrowserView_hpp

#include "ofMain.h"
#include <stdio.h>
#include "UUID.hpp"
#include "ofxImGui.h"

struct TileItem {
  std::string name;
  ImTextureID textureID;
  int index;
  // Closure which will be called when the tile is clicked
  std::function<void()> dragCallback;

  TileItem(std::string name, ImTextureID textureID, int index,
           std::function<void()> dragCallback)
      : name(name), textureID(textureID), index(index),
        dragCallback(dragCallback){};
};

class TileBrowserView {
public:
  void setup();
  void draw();
  void update();

  std::vector<TileItem> tileItems;
  float widthFraction = 0.2;
  std::string tileBrowserId;

  TileBrowserView(std::vector<TileItem> tileItems) : tileItems(tileItems),
  tileBrowserId(UUID::generateUUID())
  {};
};

#endif /* TileBrowserView_hpp */

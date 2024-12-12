#ifndef AbstractTileBrowserView_hpp
#define AbstractTileBrowserView_hpp

#include "ofMain.h"
#include "TileItem.hpp"
#include "ofxImGui.h"

class AbstractTileBrowserView {
public:
  virtual ~AbstractTileBrowserView() = default;
  
  virtual void setup() = 0;
  virtual void draw() = 0;
  virtual void update() = 0;
  virtual void setTileItems(std::vector<std::shared_ptr<TileItem>> items) = 0;
  virtual void setCallback(std::function<void(std::shared_ptr<TileItem>)> callback) = 0;
  
  ImVec2 size = ImVec2(0, 0);
};

#endif /* AbstractTileBrowserView_hpp */ 
//
//  StrandBrowserView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/23/24.
//

#ifndef StrandBrowserView_hpp
#define StrandBrowserView_hpp

#include <stdio.h>
#include "ofMain.h"
#include "AvailableStrand.hpp"
#include "TileBrowserView.hpp"
#include "UUID.hpp"

class AvailableStrandTileItem: public TileItem {
public:
  virtual ~AvailableStrandTileItem() {}
  std::shared_ptr<AvailableStrand> availableStrand;
  ImTextureID textureID;
  int index;
  bool selected = false;

  AvailableStrandTileItem(std::shared_ptr<AvailableStrand> availableStrand, ImTextureID textureID, int index, std::function<void()> dragCallback)
      : availableStrand(availableStrand), textureID(textureID), index(index), selected(selected), TileItem(availableStrand->name, textureID, index, dragCallback){};
};

class StrandBrowserView {
public:
  void setup();
  void update();
  void draw();
  void keyReleased(int key);

  std::vector<std::shared_ptr<AvailableStrandTileItem>> tileItems;
  float widthFraction = 0.2;
  std::string tileBrowserId;

  TileBrowserView tileBrowserView;

  StrandBrowserView(std::vector<std::shared_ptr<AvailableStrandTileItem>> tileItems) :                                                                                       tileItems(tileItems),
                                                                                      tileBrowserId(UUID::generateUUID()),
                                                                                      tileBrowserView(tileItems){};
  private:
  std::vector<std::shared_ptr<AvailableStrandTileItem>> tileItemsFromAvailableStrands(std::vector<std::shared_ptr<AvailableStrand>> availableStrands);
};


#endif /* StrandBrowserView_hpp */
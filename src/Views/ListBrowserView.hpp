//
//  ListBrowserView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/6/23.
//

#ifndef ListBrowserView_hpp
#define ListBrowserView_hpp

#include <stdio.h>
#include "TileBrowserView.hpp"

class ListBrowserView {
public:
  void setup();
  void draw();
  void update();

  std::vector<TileItem> tileItems;
  float widthFraction = 0.2;
  std::string id;

  ListBrowserView(std::vector<TileItem> tileItems) : tileItems(tileItems),
  id(UUID::generateUUID())
  {};
};

#endif /* ListBrowserView_hpp */

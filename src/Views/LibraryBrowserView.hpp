//
//  LibraryBrowserView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/26/23.
//

#ifndef LibraryBrowserView_hpp
#define LibraryBrowserView_hpp

#include <stdio.h>
#include "TileBrowserView.hpp"

struct LibraryBrowserView
{
public:
  void setup();
  void update();
  void draw();

private:
  TileBrowserView tileBrowserView = TileBrowserView({});
  std::vector<TileItem> libraryItems;
};

#endif /* LibraryBrowserView_hpp */

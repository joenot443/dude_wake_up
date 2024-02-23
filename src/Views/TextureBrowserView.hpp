//
//  TextureBrowserView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/16/24.
//

#ifndef TextureBrowserView_hpp
#define TextureBrowserView_hpp

#include <stdio.h>
#include "Parameter.hpp"
#include "ParameterTileBrowserView.hpp"
#include "Texture.hpp"

class TextureBrowserView {
public:
  void setup();
  void draw();
  
  std::shared_ptr<Parameter> parameter;
  std::vector<std::shared_ptr<Texture>> textures;
  std::vector<std::shared_ptr<ParameterTileItem>> textureTileItems;
  ParameterTileBrowserView tileBrowserView;
  
  TextureBrowserView(std::shared_ptr<Parameter> parameter);
};

#endif /* TextureBrowserView_hpp */

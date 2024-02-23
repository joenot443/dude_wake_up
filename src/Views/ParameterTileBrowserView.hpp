//
//  ParameterTileBrowserView.h
//  dude_wake_up
//
//  Created by Joe Crozier on 2/16/24.
//

#ifndef ParameterTileBrowserView_h
#define ParameterTileBrowserView_h

#include <stdio.h>
#include "Parameter.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include "UUID.hpp"

class ParameterTileItem {
public:
  virtual ~ParameterTileItem() {}
  std::string name;
  std::string category = "";
  ImTextureID textureID;
  int index;
  bool selected = false;

  ParameterTileItem(std::string name, ImTextureID textureID, int index, std::string category = "")
      : name(name), textureID(textureID), index(index), category(category)
        {};
};

class ParameterTileBrowserView {
public:
  static bool draw(std::vector<std::shared_ptr<ParameterTileItem>> tileItems, std::shared_ptr<Parameter> parameter, bool drawNames = true);

  std::vector<std::shared_ptr<ParameterTileItem>> tileItems;
  std::shared_ptr<Parameter> parameter;
  float widthFraction = 0.2;
  std::string tileBrowserId;

  ParameterTileBrowserView(std::vector<std::shared_ptr<ParameterTileItem>> tileItems, std::shared_ptr<Parameter> parameter) : tileItems(tileItems), parameter(parameter),
  tileBrowserId(UUID::generateUUID())
  {};
};

#endif /* ParameterTileBrowserView_h */

//
//  ShaderBrowserView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/17/23.
//

#ifndef ShaderBrowserView_hpp
#define ShaderBrowserView_hpp

#include "ShaderType.hpp"
#include "TileBrowserView.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include <stdio.h>

struct ShaderBrowserView {
public:
  void setup();
  void update();
  void draw();

  TileBrowserView filterTileBrowserView = TileBrowserView({});
  TileBrowserView mixTileBrowserView = TileBrowserView({});
  TileBrowserView basicTileBrowserView = TileBrowserView({});
  TileBrowserView transformTileBrowserView = TileBrowserView({});
  TileBrowserView maskTileBrowserView = TileBrowserView({});
};

#endif /* ShaderBrowserView_hpp */

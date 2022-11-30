//
//  ShaderChainerSettingsView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/20/22.
//

#ifndef ShaderChainerSettingsView_hpp
#define ShaderChainerSettingsView_hpp

#include <stdio.h>
#include "ShaderChainer.hpp"
#include "ofxImGui.h"

struct ShaderChainerSettingsView {
public:
  void setup();
  void update();
  void draw();
  std::shared_ptr<ShaderChainer> shaderChainer;

private:
  void drawNameField();
  void drawVideoSourceSelector();
  void drawVideoSourceSettings();
};

#endif /* ShaderChainerSettingsView_hpp */

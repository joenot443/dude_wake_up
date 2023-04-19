//
//  ShaderSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/5/23.
//

#include <stdio.h>
#include "ShaderSource.hpp"
#include "NodeLayoutView.hpp"

json ShaderSource::serialize() {
  json j;
  j["videoSourceType"] = VideoSource_shader;
  j["shaderSourceType"] = shaderSourceType;
  j["id"] = id;
  j["x"] = NodeLayoutView::getInstance()->nodeForShaderSourceId(id)->position.x;
  j["y"] = NodeLayoutView::getInstance()->nodeForShaderSourceId(id)->position.y;
  return j;
}

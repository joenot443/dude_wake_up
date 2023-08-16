//
//  ShaderSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/5/23.
//

#include <stdio.h>
#include "ShaderSource.hpp"
#include "NodeLayoutView.hpp"

json ShaderSource::serialize()
{
  json j;
  j["videoSourceType"] = VideoSource_shader;
  j["shaderSourceType"] = shaderSourceType;
  j["id"] = id;
  auto node = NodeLayoutView::getInstance()->nodeForShaderSourceId(id);
  if (node)
  {
    j["x"] = node->position.x;
    j["y"] = node->position.y;
  }

  // Append the shader's json
  j["shader"] = shader->settings->serialize();

  return j;
}

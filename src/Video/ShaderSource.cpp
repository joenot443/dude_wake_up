//
//  ShaderSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/5/23.
//

#include <stdio.h>
#include "ShaderSource.hpp"
#include "NodeLayoutView.hpp"
#include "LayoutStateService.hpp"

json ShaderSource::serialize()
{
  json j;
  j["videoSourceType"] = VideoSource_shader;
  j["shaderSourceType"] = shaderSourceType;
  j["id"] = id;
  auto node = NodeLayoutView::getInstance()->nodeForShaderSourceId(id);
  if (node != nullptr)
  {
    j["x"] = node->position.x;
    j["y"] = node->position.y;
  }

  // Append the shader's json
  j["shader"] = shader->settings->serialize();

  return j;
}

void ShaderSource::setup() {
  shader->setup();
  fbo->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y, GL_RGBA);
  fbo->begin();
  ofSetColor(0, 0, 0, 255);
  ofDrawRectangle(0, 0, fbo->getWidth(), fbo->getHeight());
  fbo->end();

  canvas->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y, GL_RGBA);
  canvas->begin();
  ofSetColor(0, 0, 0, 255);
  ofDrawRectangle(0, 0, fbo->getWidth(), fbo->getHeight());
  canvas->end();
  
  maskShader.load("shaders/ColorKeyMaskMaker");

};

//
//  AvailableVideoSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/16/23.
//

#include "AvailableVideoSource.hpp"
#include "ShaderChainerService.hpp"
#include "UUID.hpp"
#include <stdio.h>

AvailableVideoSource::AvailableVideoSource(std::string sourceName,
                                           VideoSourceType type)
    : sourceName(sourceName), type(type),
      availableVideoSourceId(UUID::generateUUID()) {
  preview = std::make_shared<ofTexture>();
}

void AvailableVideoSourceShader::generatePreview()  {
  std::shared_ptr<ofFbo> fbo = std::make_shared<ofFbo>();
  std::shared_ptr<ofFbo> blankFbo = std::make_shared<ofFbo>();
  fbo->allocate(320, 240, GL_RGBA);
  blankFbo->allocate(320, 240, GL_RGBA);

  auto shader = ShaderChainerService::getService()->shaderForType(
      shaderTypeForShaderSourceType(shaderType), UUID::generateUUID(), 0);
  shader->setup();
  shader->shade(blankFbo, fbo);
  fbo->begin();
  ofSetColor(0, 0, 0, 128);
  ofDrawRectangle(0, 0, 320, 240);
  fbo->end();
  preview = std::make_shared<ofTexture>(fbo->getTexture());
  hasPreview = true;
}

//
//  AvailableShader.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/17/23.
//

#include "AvailableShader.hpp"
#include "VideoSourceService.hpp"
#include "ShaderChainerService.hpp"

AvailableShader::AvailableShader(ShaderType type, std::string name) : type(type), name(name) {
  preview = std::make_shared<ofTexture>();
  generatePreview();
}


void AvailableShader::generatePreview() {
  auto fbo = ofFbo();
  fbo.allocate(320, 240);
  
  auto previewCanvas = VideoSourceService::getService()->previewFbo();

  auto shader = ShaderChainerService::getService()->shaderForType(
      type, UUID::generateUUID(), 0);
  shader->setup();
  shader->shade(&previewCanvas, &fbo);
  preview = std::make_shared<ofTexture>(fbo.getTexture());
}

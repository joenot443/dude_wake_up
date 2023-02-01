//
//  AvailableShader.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/17/23.
//

#include "AvailableShader.hpp"
#include "ShaderChainerService.hpp"
#include "VideoSourceService.hpp"

AvailableShader::AvailableShader(ShaderType type, std::string name)
    : type(type), name(name) {
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
  fbo.begin();
  // Add a 70% black overlay to the preview
  ofSetColor(0, 0, 0, 70);
  ofDrawRectangle(0, 0, 320, 240);
  fbo.end();
  preview = std::make_shared<ofTexture>(fbo.getTexture());
}

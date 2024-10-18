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
  std::shared_ptr<ofFbo> canvas = std::make_shared<ofFbo>();
  canvas->allocate(426, 240);

  auto previewFbo = VideoSourceService::getService()->previewFbo();

  auto shader = ShaderChainerService::getService()->shaderForType(
      type, UUID::generateUUID(), 0);
  shader->allocateFrames();
  shader->setup();
  shader->shade(previewFbo, canvas);
  canvas->begin();
  // Add a 70% black overlay to the preview
  ofSetColor(0, 0, 0, 128);
  ofDrawRectangle(0, 0, 426, 240);
  canvas->end();
  preview = std::make_shared<ofTexture>(canvas->getTexture());
}

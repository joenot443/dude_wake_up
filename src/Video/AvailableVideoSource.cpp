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
                                           VideoSourceType type,
                                           ShaderSourceType shaderType,
                                           int webcamId,
                                           std::string path)
    : sourceName(sourceName), type(type),
      availableVideoSourceId(UUID::generateUUID()),
      shaderType(shaderType),
      webcamId(webcamId),
      path(path) {
  preview = std::make_shared<ofTexture>();
  generatePreview();
}

void AvailableVideoSource::generatePreview() {
  switch (type) {
  case VideoSource_shader: {
    auto fbo = ofFbo();
    auto blankFbo = ofFbo();
    fbo.allocate(320, 240);
    blankFbo.allocate(320, 240);

    auto shader = ShaderChainerService::getService()->shaderForType(
        shaderTypeForShaderSourceType(shaderType), UUID::generateUUID(), 0);
    shader->setup();
    shader->shade(&blankFbo, &fbo);
    fbo.begin();
    // Add a 70% black overlay to the preview
    ofSetColor(0, 0, 0, 70);
    ofDrawRectangle(0, 0, 320, 240);
    fbo.end();
    preview = std::make_shared<ofTexture>(fbo.getTexture());
  }
  case VideoSource_webcam: {
  }
  case VideoSource_file: {
  }
  }
}

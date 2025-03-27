//
//  AvailableVideoSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/16/23.
//

#include "BookmarkService.hpp"
#include "ofAVFoundationPlayer.h"
#include "AvailableVideoSource.hpp"
#include "ShaderChainerService.hpp"
#include "UUID.hpp"
#include <stdio.h>

AvailableVideoSource::AvailableVideoSource(std::string sourceName,
                                           std::string category,
                                           VideoSourceType type)
    : sourceName(sourceName), type(type), category(category),
      availableVideoSourceId(UUID::generateUUID()) {
  preview = std::make_shared<ofTexture>();
}

void AvailableVideoSourceShader::generatePreview()  {
  std::shared_ptr<ofFbo> fbo = std::make_shared<ofFbo>();
  std::shared_ptr<ofFbo> blankFbo = std::make_shared<ofFbo>();
  fbo->allocate(426, 240, GL_RGBA);
  blankFbo->allocate(426, 240, GL_RGBA);

  auto shader = ShaderChainerService::getService()->shaderForType(
      shaderTypeForShaderSourceType(shaderType), UUID::generateUUID(), 0);
  shader->setup();
  shader->shade(blankFbo, fbo);
  fbo->begin();
  ofSetColor(0, 0, 0, 128);
  ofDrawRectangle(0, 0, 426, 240);
  fbo->end();
  preview = std::make_shared<ofTexture>(fbo->getTexture());
  hasPreview = true;
}

void AvailableVideoSourceFile::generatePreview(ofVideoPlayer& videoPlayer) {
  // Update the video player to get the first frame
  videoPlayer.setPosition(0.1);
  // Play the video to ensure it's ready
  videoPlayer.play();
  videoPlayer.update();

  // Allocate an FBO to draw the frame
  ofFbo fbo;
  fbo.allocate(426, 240, GL_RGBA);
  
  // Begin drawing to the FBO
  fbo.begin();
  ofClear(0, 0, 0, 0); // Clear the FBO
  videoPlayer.draw(0, 0, 426, 240); // Draw the video frame
  fbo.end();
  
  // Set the preview texture
  preview = std::make_shared<ofTexture>(fbo.getTexture());
  
  hasPreview = true;
  videoPlayer.stop();
}

void AvailableVideoSourceImage::generatePreview() {
  // Create an image instance
  ofImage image;
  
  // Load the image file from the specified path
  if (!image.load(path)) {
    ofLogError("AvailableVideoSourceImage") << "Failed to load image file: " << path;
    return;
  }
  
  // Allocate an FBO to draw the image
  ofFbo fbo;
  fbo.allocate(426, 240, GL_RGBA);
  
  // Begin drawing to the FBO
  fbo.begin();
  ofClear(0, 0, 0, 0); // Clear the FBO
  image.draw(0, 0, 426, 240); // Draw the image
  fbo.end();
  
  // Set the preview texture
  preview = std::make_shared<ofTexture>(fbo.getTexture());
  hasPreview = true;
  BookmarkService::getService()->endAccessingBookmark(path);
}


bool AvailableVideoSourceFile::canGeneratePreview(ofVideoPlayer& videoPlayer) {
  return videoPlayer.getCurrentFrame() > 0;
}

bool AvailableVideoSourceFile::hasFailedToLoad(ofVideoPlayer& videoPlayer) {
  std::shared_ptr<ofAVFoundationPlayer> player = std::dynamic_pointer_cast<ofAVFoundationPlayer>(videoPlayer.getPlayer());
  return !(player->isPlaying() || player->isPaused());
}

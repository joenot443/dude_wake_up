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
#include "TimeService.hpp"
#include "UUID.hpp"
#include <algorithm>
#include <stdio.h>

AvailableVideoSource::AvailableVideoSource(std::string sourceName,
                                           std::string category,
                                           VideoSourceType type)
    : sourceName(sourceName), type(type), category(category),
      availableVideoSourceId(UUID::generateUUID()) {
  preview = std::make_shared<ofTexture>();
}

// Returns true if the FBO is essentially black/empty. Sampled sparsely so it's
// cheap to call at preview-generation time.
static bool previewLooksBlank(const std::shared_ptr<ofFbo> &fbo) {
  if (fbo == nullptr || !fbo->isAllocated()) return true;
  ofPixels pixels;
  fbo->readToPixels(pixels);
  if (!pixels.isAllocated()) return true;
  size_t numPixels = static_cast<size_t>(pixels.getWidth()) * pixels.getHeight();
  int ch = pixels.getNumChannels();
  if (numPixels == 0 || ch < 3) return true;
  const unsigned char *d = pixels.getData();
  size_t step = std::max<size_t>(1, numPixels / 2000); // sample ~2000 pixels
  for (size_t p = 0; p < numPixels; p += step) {
    const unsigned char *px = d + p * ch;
    if (px[0] > 10 || px[1] > 10 || px[2] > 10) return false;
  }
  return true;
}

void AvailableVideoSourceShader::generatePreview()  {
  previewAttempts++;

  // Maximum number of (lazy) retries before we give up and cache whatever we
  // get — keeps a genuinely-dark source from re-rendering every browser frame.
  static const int kMaxPreviewAttempts = 8;

  std::shared_ptr<ofFbo> fbo = std::make_shared<ofFbo>();
  std::shared_ptr<ofFbo> blankFbo = std::make_shared<ofFbo>();
  fbo->allocate(426, 240, GL_RGBA);
  blankFbo->allocate(426, 240, GL_RGBA);

  // Clear both FBOs so a source that samples its input doesn't read garbage.
  blankFbo->begin();
  ofClear(0, 0, 0, 255);
  blankFbo->end();
  fbo->begin();
  ofClear(0, 0, 0, 255);
  fbo->end();

  auto shader = ShaderChainerService::getService()->shaderForType(
      shaderTypeForShaderSourceType(shaderType), UUID::generateUUID(), 0);
  shader->setup();

  // Render at a fixed, representative time rather than the live value (which is
  // ~0 when previews are first built at startup), and step a few warm-up frames
  // so time-animated and phase-accumulating sources show a developed image.
  // The global time is restored immediately afterward; this runs synchronously.
  auto timeService = TimeService::getService();
  float savedTime = timeService->timeParam->value;
  const float previewBaseTime = 12.0f;
  const int warmupFrames = 4;
  for (int i = 0; i < warmupFrames; i++) {
    timeService->timeParam->value = previewBaseTime + i * (1.0f / 60.0f);
    shader->shade(blankFbo, fbo);
  }
  timeService->timeParam->value = savedTime;

  // If the render is still blank (e.g. a texture or GL resource wasn't ready),
  // don't cache it — leave hasPreview false so the browser retries on a later
  // frame once things have warmed up. Bounded by kMaxPreviewAttempts.
  if (previewLooksBlank(fbo) && previewAttempts < kMaxPreviewAttempts) {
    hasPreview = false;
    return;
  }

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


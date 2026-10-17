#pragma once

#include <memory>
#include <string>
#include <vector>

#include "ofTexture.h"

namespace nottawa {

struct FrameCacheResult {
  bool success = false;
  std::vector<std::shared_ptr<ofTexture>> frames;
  int  sourceFramesSeen = 0;
  int  sourceWidth      = 0;
  int  sourceHeight     = 0;
  std::string errorMessage;
};

// Synchronously decode a video file via AVAssetReader and return GPU
// textures for every Nth frame (stride). Must be called on a thread with
// a current OpenGL context.
FrameCacheResult buildFrameCacheFromAsset(const std::string& videoPath, int stride);

} // namespace nottawa

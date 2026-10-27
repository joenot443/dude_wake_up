// FileSourceCacheBuilder_win.cpp
//
// Non-macOS stub of buildFrameCacheFromAsset. The real implementation
// (.mm) uses AVAssetReader to decode a video into per-frame textures
// for the main-stage browser preview strip. Without a Windows video
// decode backend, this returns an empty cache; browsers show no
// video thumbnails until the backend lands (Phase 5+ libav route).

#ifndef TARGET_OSX

#include "FileSourceCacheBuilder.hpp"

namespace nottawa {

FrameCacheResult buildFrameCacheFromAsset(const std::string& /*videoPath*/, int /*stride*/) {
  FrameCacheResult r;
  r.success = false;
  r.errorMessage = "video frame-cache builder is not implemented on Windows yet";
  return r;
}

} // namespace nottawa

#endif // !TARGET_OSX

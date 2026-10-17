// IOSurfaceVideoPlayer_win.cpp
//
// Non-macOS stub of the AVFoundation-backed IOSurfaceVideoPlayer. The
// real implementation (IOSurfaceVideoPlayer.mm) uses Apple's zero-copy
// CoreVideo → GL texture path, which has no Windows equivalent. Until
// we wire a Media Foundation (or libav-based) video playback backend
// for Windows, FileSource is file-load-only — videos won't play but
// construction and tear-down are safe no-ops.
//
// Interface stays identical to IOSurfaceVideoPlayer.hpp so every caller
// in src/Video/, src/Services/, and nottawa_bridge compiles unchanged.

#ifndef TARGET_OSX

#include "IOSurfaceVideoPlayer.hpp"
#include "ofLog.h"
#include "ofTexture.h"

namespace nottawa {

struct IOSurfaceVideoPlayer::Impl {
  // Reserved for a future MediaFoundation/libav backend.
};

IOSurfaceVideoPlayer::IOSurfaceVideoPlayer() : impl_(nullptr) {}
IOSurfaceVideoPlayer::~IOSurfaceVideoPlayer() = default;

bool IOSurfaceVideoPlayer::load(const std::string& path) {
  static bool warned = false;
  if (!warned) {
    ofLogWarning("IOSurfaceVideoPlayer_win")
      << "video playback is not implemented on Windows yet (requested: "
      << path << ")";
    warned = true;
  }
  return false;
}

bool  IOSurfaceVideoPlayer::isLoaded()         const { return false; }
void  IOSurfaceVideoPlayer::close()                  {}
void  IOSurfaceVideoPlayer::setVolume(float)         {}

float IOSurfaceVideoPlayer::getDuration()      const { return 0.f; }
int   IOSurfaceVideoPlayer::getTotalNumFrames() const { return 0; }
float IOSurfaceVideoPlayer::getWidth()         const { return 0.f; }
float IOSurfaceVideoPlayer::getHeight()        const { return 0.f; }

void  IOSurfaceVideoPlayer::play()                   {}
void  IOSurfaceVideoPlayer::stop()                   {}
void  IOSurfaceVideoPlayer::setPaused(bool)          {}
bool  IOSurfaceVideoPlayer::isPlaying()        const { return false; }

void  IOSurfaceVideoPlayer::setPosition(float)       {}
float IOSurfaceVideoPlayer::getPosition()      const { return 0.f; }

void  IOSurfaceVideoPlayer::setSpeed(float)          {}
float IOSurfaceVideoPlayer::getSpeed()         const { return 1.f; }

void  IOSurfaceVideoPlayer::setLoopState(ofLoopType) {}

void  IOSurfaceVideoPlayer::update()                 {}
bool  IOSurfaceVideoPlayer::isFrameNew()       const { return false; }
bool  IOSurfaceVideoPlayer::getIsMovieDone()   const { return false; }

void  IOSurfaceVideoPlayer::draw(float, float, float, float) {}

} // namespace nottawa

#endif // !TARGET_OSX

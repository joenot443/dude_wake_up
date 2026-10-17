//
//  IOSurfaceVideoPlayer.mm
//
//  Zero-copy AVFoundation → GL_TEXTURE_2D player. See header for rationale.
//
//  Pipeline per frame:
//    AVPlayerItemVideoOutput -copyPixelBufferForItemTime:itemTimeForDisplay:
//      → CVPixelBufferRef (kCVPixelFormatType_32BGRA, IOSurface-backed)
//        → CVPixelBufferGetIOSurface()
//          → CGLTexImageIOSurface2D(GL_TEXTURE_2D, GL_RGBA, ..., GL_BGRA, ...)
//            → ofTexture wrapping that GL texture name
//

#include "IOSurfaceVideoPlayer.hpp"

#include "ofLog.h"
#include "ofGraphics.h"

#import <AVFoundation/AVFoundation.h>
#import <CoreVideo/CoreVideo.h>
#import <Foundation/Foundation.h>
#import <OpenGL/CGLIOSurface.h>
#import <OpenGL/CGLCurrent.h>
#import <OpenGL/gl3.h>

#include <atomic>

namespace nottawa {

// Notification handler (Obj-C trampoline) that flips an atomic bool when the
// AVPlayerItem reaches end-of-time. Engine thread reads the bool via
// getIsMovieDone() / inside update() to drive looping.
} // namespace nottawa

@interface NottawaIOSurfaceVideoPlayerObserver : NSObject
- (instancetype)initWithFlag:(std::atomic<bool>*)flag;
@end

@implementation NottawaIOSurfaceVideoPlayerObserver {
  std::atomic<bool>* _flag;
}
- (instancetype)initWithFlag:(std::atomic<bool>*)flag {
  if ((self = [super init])) {
    _flag = flag;
  }
  return self;
}
- (void)playerItemDidReachEnd:(NSNotification*)notification {
  if (_flag) _flag->store(true);
}
@end

namespace nottawa {

struct IOSurfaceVideoPlayer::Impl {
  // AVFoundation
  AVPlayer*                  player        = nil;
  AVPlayerItem*              item          = nil;
  AVPlayerItemVideoOutput*   output        = nil;
  NottawaIOSurfaceVideoPlayerObserver* endObserver = nil;

  // Most recent pixel buffer — held to keep its IOSurface alive for the
  // currently-bound rectangle texture. Released when a newer one is bound.
  CVPixelBufferRef           heldBuffer    = nullptr;

  // CGLTexImageIOSurface2D requires GL_TEXTURE_RECTANGLE. We then blit it
  // into a GL_TEXTURE_2D so the engine (which runs ArbTex-disabled and uses
  // GL_TEXTURE_2D throughout) can sample it like any other source.
  GLuint                     glTexRect     = 0;
  GLuint                     glTex2D       = 0;
  GLuint                     srcFbo        = 0; // RECT-attached FBO for blit READ
  GLuint                     dstFbo        = 0; // 2D-attached FBO for blit DRAW
  ofTexture                  videoTexture; // wraps glTex2D via setUseExternalTextureID
  int                        boundWidth    = 0;
  int                        boundHeight   = 0;

  // Cached metadata — pulled at load() time, used for sub-second math
  int                        width         = 0;
  int                        height        = 0;
  int                        totalFrames   = 0;
  float                      durationSec   = 0.0f;

  // Mirror state
  std::string                path;
  bool                       loaded        = false;
  ofLoopType                 loopState     = OF_LOOP_NORMAL;
  std::atomic<bool>          movieDone{false};
  bool                       frameNew      = false; // last update() pulled a new frame
  CMTime                     lastBoundTime = kCMTimeInvalid;

  // Volume mirror so getters can read without bouncing through the player.
  float                      volume        = 1.0f;
};

IOSurfaceVideoPlayer::IOSurfaceVideoPlayer() : impl_(std::make_unique<Impl>()) {}

IOSurfaceVideoPlayer::~IOSurfaceVideoPlayer() {
  close();
}

bool IOSurfaceVideoPlayer::load(const std::string& path) {
  // Idempotent: same path, already loaded → leave everything (AVPlayer, GL
  // textures, FBOs) in place. Without this, FileSource's resolution-update
  // subscription cascade tears down + rebuilds the player on every notify.
  if (impl_->loaded && impl_->path == path) {
    return true;
  }
  // Reset any prior state.
  close();

  @autoreleasepool {
    NSString* nsPath = [NSString stringWithUTF8String:path.c_str()];
    if (!nsPath) {
      ofLogError("IOSurfaceVideoPlayer") << "load: invalid path";
      return false;
    }
    NSURL* url = [NSURL fileURLWithPath:nsPath];
    AVURLAsset* asset = [AVURLAsset URLAssetWithURL:url options:nil];

    NSArray<AVAssetTrack*>* videoTracks = [asset tracksWithMediaType:AVMediaTypeVideo];
    if (videoTracks.count == 0) {
      ofLogError("IOSurfaceVideoPlayer") << "load: no video track";
      return false;
    }
    AVAssetTrack* track = videoTracks.firstObject;
    CGSize naturalSize = track.naturalSize;
    impl_->width  = (int)naturalSize.width;
    impl_->height = (int)naturalSize.height;
    impl_->durationSec = (float)CMTimeGetSeconds(asset.duration);

    // Estimate total frames from nominal rate × duration. Good enough for
    // FileSource's playback math (it doesn't require frame-accurate counts).
    float fps = track.nominalFrameRate > 0 ? track.nominalFrameRate : 30.0f;
    impl_->totalFrames = (int)std::round(fps * impl_->durationSec);
    if (impl_->totalFrames <= 0) impl_->totalFrames = 1;

    impl_->item = [AVPlayerItem playerItemWithAsset:asset];

    // BGRA + IOSurface backing — the two attributes that make zero-copy work.
    NSDictionary* pbAttrs = @{
      (id)kCVPixelBufferPixelFormatTypeKey      : @(kCVPixelFormatType_32BGRA),
      (id)kCVPixelBufferIOSurfacePropertiesKey  : @{},
    };
    impl_->output = [[AVPlayerItemVideoOutput alloc]
                       initWithPixelBufferAttributes:pbAttrs];
    impl_->output.suppressesPlayerRendering = YES; // we render via GL ourselves
    [impl_->item addOutput:impl_->output];

    impl_->player = [[AVPlayer alloc] initWithPlayerItem:impl_->item];
    impl_->player.volume = impl_->volume;

    // End-of-time notification → flip atomic flag.
    impl_->endObserver = [[NottawaIOSurfaceVideoPlayerObserver alloc]
                            initWithFlag:&impl_->movieDone];
    [[NSNotificationCenter defaultCenter]
       addObserver:impl_->endObserver
          selector:@selector(playerItemDidReachEnd:)
              name:AVPlayerItemDidPlayToEndTimeNotification
            object:impl_->item];
  }

  // Allocate the rectangle texture (target for CGLTexImageIOSurface2D — that
  // function rejects GL_TEXTURE_2D, so we go through RECT and blit).
  if (impl_->glTexRect == 0) {
    glGenTextures(1, &impl_->glTexRect);
    glBindTexture(GL_TEXTURE_RECTANGLE, impl_->glTexRect);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_RECTANGLE, 0);
  }

  // Allocate the GL_TEXTURE_2D blit target with empty storage. update() blits
  // the per-frame rectangle texture into this each tick. The engine's shader
  // chain samples from this texture like any other source.
  if (impl_->glTex2D == 0) {
    glGenTextures(1, &impl_->glTex2D);
    glBindTexture(GL_TEXTURE_2D, impl_->glTex2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, impl_->width, impl_->height,
                 0, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindTexture(GL_TEXTURE_2D, 0);
  }

  // FBOs for the per-frame blit (RECT → 2D).
  if (impl_->srcFbo == 0) glGenFramebuffers(1, &impl_->srcFbo);
  if (impl_->dstFbo == 0) {
    glGenFramebuffers(1, &impl_->dstFbo);
    glBindFramebuffer(GL_FRAMEBUFFER, impl_->dstFbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                           GL_TEXTURE_2D, impl_->glTex2D, 0);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
  }

  impl_->boundWidth  = impl_->width;
  impl_->boundHeight = impl_->height;

  // Set up the ofTexture wrapper. allocate() establishes texData (dimensions,
  // target) then setUseExternalTextureID swaps in our GL_TEXTURE_2D and
  // releases the auto-allocated one.
  impl_->videoTexture.allocate(impl_->width, impl_->height, GL_RGBA);
  impl_->videoTexture.setUseExternalTextureID(impl_->glTex2D);

  impl_->path = path;
  impl_->loaded = true;
  impl_->frameNew = false;
  impl_->movieDone = false;
  impl_->lastBoundTime = kCMTimeInvalid;
  return true;
}

bool IOSurfaceVideoPlayer::isLoaded() const {
  return impl_->loaded;
}

void IOSurfaceVideoPlayer::close() {
  @autoreleasepool {
    if (impl_->endObserver) {
      [[NSNotificationCenter defaultCenter] removeObserver:impl_->endObserver];
      impl_->endObserver = nil;
    }
    if (impl_->player) {
      [impl_->player pause];
    }
    if (impl_->output && impl_->item) {
      [impl_->item removeOutput:impl_->output];
    }
    impl_->output = nil;
    impl_->item   = nil;
    impl_->player = nil;
  }
  if (impl_->heldBuffer) {
    CVBufferRelease(impl_->heldBuffer);
    impl_->heldBuffer = nullptr;
  }
  if (impl_->srcFbo != 0) {
    glDeleteFramebuffers(1, &impl_->srcFbo);
    impl_->srcFbo = 0;
  }
  if (impl_->dstFbo != 0) {
    glDeleteFramebuffers(1, &impl_->dstFbo);
    impl_->dstFbo = 0;
  }
  if (impl_->glTexRect != 0) {
    glDeleteTextures(1, &impl_->glTexRect);
    impl_->glTexRect = 0;
  }
  if (impl_->glTex2D != 0) {
    // ofTexture wrapper points at this name via bUseExternalTextureID, so
    // its destructor will not try to delete it. Free it ourselves.
    glDeleteTextures(1, &impl_->glTex2D);
    impl_->glTex2D = 0;
  }
  impl_->videoTexture.clear();
  impl_->loaded   = false;
  impl_->frameNew = false;
  impl_->lastBoundTime = kCMTimeInvalid;
}

void IOSurfaceVideoPlayer::setVolume(float v) {
  impl_->volume = v;
  if (impl_->player) impl_->player.volume = v;
}

float IOSurfaceVideoPlayer::getDuration() const {
  return impl_->durationSec;
}

int IOSurfaceVideoPlayer::getTotalNumFrames() const {
  return impl_->totalFrames;
}

float IOSurfaceVideoPlayer::getWidth() const {
  return (float)impl_->width;
}

float IOSurfaceVideoPlayer::getHeight() const {
  return (float)impl_->height;
}

void IOSurfaceVideoPlayer::play() {
  if (impl_->player) [impl_->player play];
}

void IOSurfaceVideoPlayer::stop() {
  if (impl_->player) {
    [impl_->player pause];
    setPosition(0.0f);
  }
}

void IOSurfaceVideoPlayer::setPaused(bool paused) {
  if (!impl_->player) return;
  if (paused) [impl_->player pause];
  else        [impl_->player play];
}

bool IOSurfaceVideoPlayer::isPlaying() const {
  if (!impl_->player) return false;
  return impl_->player.rate != 0.0f;
}

void IOSurfaceVideoPlayer::setPosition(float pct) {
  if (!impl_->player) return;
  if (pct < 0.0f) pct = 0.0f;
  if (pct > 1.0f) pct = 1.0f;
  CMTime target = CMTimeMakeWithSeconds(pct * impl_->durationSec, 600);
  [impl_->player seekToTime:target
            toleranceBefore:kCMTimeZero
             toleranceAfter:kCMTimeZero];
}

float IOSurfaceVideoPlayer::getPosition() const {
  if (!impl_->player || impl_->durationSec <= 0.0f) return 0.0f;
  float t = (float)CMTimeGetSeconds(impl_->player.currentTime);
  return t / impl_->durationSec;
}

void IOSurfaceVideoPlayer::setSpeed(float speed) {
  if (!impl_->player) return;
  // AVPlayer supports negative rates for backwards playback (used by the
  // boomerang feature). 0 == paused.
  impl_->player.rate = speed;
}

float IOSurfaceVideoPlayer::getSpeed() const {
  if (!impl_->player) return 1.0f;
  return impl_->player.rate;
}

void IOSurfaceVideoPlayer::setLoopState(ofLoopType state) {
  impl_->loopState = state;
}

void IOSurfaceVideoPlayer::update() {
  impl_->frameNew = false;
  if (!impl_->loaded || !impl_->output ||
      impl_->glTexRect == 0 || impl_->glTex2D == 0) {
    return;
  }

  // Looping: if AVPlayer reached the end, either rewind+play or stay parked.
  if (impl_->movieDone.exchange(false)) {
    if (impl_->loopState == OF_LOOP_NORMAL) {
      setPosition(0.0f);
      play();
    }
    // OF_LOOP_NONE: leave isMovieDone() returning true until caller reacts.
  }

  CMTime now = impl_->item.currentTime;
  if (![impl_->output hasNewPixelBufferForItemTime:now]) return;

  CVPixelBufferRef pb = [impl_->output copyPixelBufferForItemTime:now
                                                itemTimeForDisplay:nullptr];
  if (!pb) return;

  IOSurfaceRef surface = CVPixelBufferGetIOSurface(pb);
  if (!surface) {
    // The output didn't give us an IOSurface-backed buffer — nothing we can
    // do without falling back to a CPU copy. Drop the frame.
    CVBufferRelease(pb);
    return;
  }

  GLsizei w = (GLsizei)CVPixelBufferGetWidth(pb);
  GLsizei h = (GLsizei)CVPixelBufferGetHeight(pb);

  CGLContextObj cgl = CGLGetCurrentContext();
  if (!cgl) {
    CVBufferRelease(pb);
    return;
  }

  // Step 1: bind the IOSurface into the rectangle texture (zero-copy view of
  // the same memory the AV decoder wrote into).
  glBindTexture(GL_TEXTURE_RECTANGLE, impl_->glTexRect);
  CGLError err = CGLTexImageIOSurface2D(
    cgl,
    GL_TEXTURE_RECTANGLE,
    GL_RGBA8,
    w, h,
    GL_BGRA,
    GL_UNSIGNED_INT_8_8_8_8_REV,
    surface,
    0
  );
  glBindTexture(GL_TEXTURE_RECTANGLE, 0);
  if (err != kCGLNoError) {
    ofLogError("IOSurfaceVideoPlayer") << "CGLTexImageIOSurface2D failed: " << err;
    CVBufferRelease(pb);
    return;
  }

  // Step 2: if dimensions changed (rare), resize the GL_TEXTURE_2D backing.
  if (w != impl_->boundWidth || h != impl_->boundHeight) {
    glBindTexture(GL_TEXTURE_2D, impl_->glTex2D);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, w, h, 0,
                 GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV, nullptr);
    glBindTexture(GL_TEXTURE_2D, 0);
    impl_->boundWidth  = w;
    impl_->boundHeight = h;
    impl_->width  = w;
    impl_->height = h;
    impl_->videoTexture.allocate((float)w, (float)h, GL_RGBA);
    impl_->videoTexture.setUseExternalTextureID(impl_->glTex2D);
  }

  // Step 3: blit RECT (read) → 2D (draw). Single GPU copy, no CPU touch.
  // Re-attach the rectangle texture to srcFbo each frame because the
  // rectangle texture's IOSurface backing was just rebound — rebinding the
  // attachment is the simplest way to make sure FBO state is in sync.
  glBindFramebuffer(GL_READ_FRAMEBUFFER, impl_->srcFbo);
  glFramebufferTexture2D(GL_READ_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_RECTANGLE, impl_->glTexRect, 0);

  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, impl_->dstFbo);
  glBlitFramebuffer(0, 0, w, h,
                    0, 0, w, h,
                    GL_COLOR_BUFFER_BIT, GL_LINEAR);

  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  // Hold the new pixel buffer so its IOSurface stays alive while sampled
  // (the rectangle texture's binding still references it until next rebind),
  // and release the previous one.
  if (impl_->heldBuffer) CVBufferRelease(impl_->heldBuffer);
  impl_->heldBuffer = pb;
  impl_->lastBoundTime = now;

  impl_->frameNew = true;
}

bool IOSurfaceVideoPlayer::isFrameNew() const {
  return impl_->frameNew;
}

bool IOSurfaceVideoPlayer::getIsMovieDone() const {
  return impl_->movieDone.load();
}

void IOSurfaceVideoPlayer::draw(float x, float y, float w, float h) {
  if (impl_->videoTexture.isAllocated()) {
    impl_->videoTexture.draw(x, y, w, h);
  }
}

} // namespace nottawa

//
//  ScratchFrame.hpp
//  dude_wake_up
//
//  A working frame a node allocates only when it's used (most nodes never
//  need one: it holds the optional effects' output, or Blend's ping-pong).
//  The only way to it is get(width, height), which allocates it at that size:
//  callers can't be handed an unallocated one. (Blend once checked for an
//  allocated frame itself and, after the frame became lazy, never drew.)
//

#ifndef ScratchFrame_hpp
#define ScratchFrame_hpp

#include "ofFbo.h"

#include <memory>

class ScratchFrame {
public:
  // The frame, allocated (or reallocated) at width x height. GL thread.
  std::shared_ptr<ofFbo> get(int width, int height) {
    if (!fbo_) fbo_ = std::make_shared<ofFbo>();
    if (!fbo_->isAllocated() || fbo_->getWidth() != width || fbo_->getHeight() != height)
      fbo_->allocate(width, height, GL_RGBA);
    return fbo_;
  }

  // Frees its GL storage (get() allocates it again). GL thread.
  void release() {
    if (fbo_) fbo_->clear();
  }

private:
  std::shared_ptr<ofFbo> fbo_;
};

#endif /* ScratchFrame_hpp */

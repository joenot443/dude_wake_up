#pragma once

#include <memory>
#include "ofFbo.h"

class FramePreview {
public:
  static FramePreview& getInstance() {
    static FramePreview instance;
    return instance;
  }

  void setFrame(std::shared_ptr<ofFbo> frame) {
    this->frame = frame;
  }

  void draw() {
    if (frame) {
      frame->draw(0, 0, 300, 200);
    }
  }

private:
  FramePreview() = default;
  std::shared_ptr<ofFbo> frame;
};

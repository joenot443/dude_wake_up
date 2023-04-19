//
//  EmptySource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/16/23.
//

#include "EmptySource.hpp"

void EmptySource::setup() {
  // Fill with black 100% alpha
  fbo.allocate(settings.width->value, settings.height->value);
  fbo.begin();
  ofSetColor(0, 0, 0, 255);
  ofDrawRectangle(0, 0, fbo.getWidth(), fbo.getHeight());
  fbo.end();
  frameTexture = std::shared_ptr<ofTexture>(&fbo.getTexture());
}

void EmptySource::saveFrame() {
  return;
}

json EmptySource::serialize() {
  return 0;
}

void EmptySource::load(json j) {
  return;
}

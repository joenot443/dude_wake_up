//
//  OutputWindow.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/30/22.
//

#include "OutputWindow.hpp"


void OutputWindow::setup() {}

void OutputWindow::update() {
  if (chainer->settings.width->intValue != ofGetWindowWidth()) {
    ofSetWindowShape(chainer->settings.width->intValue, chainer->settings.height->intValue);
  }
  std::stringstream strm;
  strm << "fps: " << ofGetFrameRate();
  ofSetWindowTitle(strm.str());
}

void OutputWindow::draw() {
  chainer->fbo.draw(0, 0);
}

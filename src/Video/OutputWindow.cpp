//
//  OutputWindow.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/30/22.
//

#include "OutputWindow.hpp"


void OutputWindow::setup() {
  ofFbo::Settings fboSettings;
  fboSettings.width = 640;
  fboSettings.height = 480;
  fboSettings.internalformat = GL_RGB;
  fboSettings.textureTarget = GL_TEXTURE_2D;
  outputFbo.allocate(fboSettings);
}

void OutputWindow::update() {

}

void OutputWindow::draw() {
  videoSource->frameTexture->draw(0, 0, ofGetWidth(), ofGetHeight());
}

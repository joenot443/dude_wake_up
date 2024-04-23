//
//  OutputWindow.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/30/22.
//

#include "OutputWindow.hpp"
#include "VideoSourceService.hpp"

void OutputWindow::setup() {}

void OutputWindow::update()
{
}

void OutputWindow::draw()
{
  fbo->draw(0, 0, ofGetWidth(), ofGetHeight());
}


void OutputWindow::keyReleased(int key) {
  // If the key released is ESC, close the output window
  if (key == OF_KEY_ESC) {
    VideoSourceService::getService()->closeOutputWindow(shared_from_this());
    ofGetCurrentWindow()->setWindowShouldClose();
  }
}

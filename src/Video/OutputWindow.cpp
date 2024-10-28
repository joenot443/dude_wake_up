//
//  OutputWindow.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/30/22.
//

#include "OutputWindow.hpp"
#include "VideoSourceService.hpp"
#include "LayoutStateService.hpp"

void OutputWindow::setup() {
  LayoutStateService::getService()->subscribeToResolutionUpdates([this]() {
    needsResolutionUpdate = true;
  });
}

void OutputWindow::update()
{
}

void OutputWindow::draw()
{
  if (needsResolutionUpdate) updateResolution();
  
  fbo->draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());
  if (drawFPS) ofSetWindowTitle(formatString("%.2f FPS", ofGetFrameRate()));
}

void OutputWindow::updateResolution()
{
  bool portrait = LayoutStateService::getService()->portrait;
  ImVec2 res = LayoutStateService::getService()->resolution;
  ofSetWindowShape(portrait ? res.y : res.x, portrait ? res.x : res.y);
  needsResolutionUpdate = false;
}

void OutputWindow::keyReleased(int key) {
  // If the key released is ESC, close the output window
  if (key == OF_KEY_ESC) {
    VideoSourceService::getService()->closeOutputWindow(shared_from_this());
    ofGetCurrentWindow()->setWindowShouldClose();
  }
  
  if (key == OF_KEY_SHIFT) {
    drawFPS = false;
    ofSetWindowTitle("");
  }
}

void OutputWindow::keyPressed(int key) {
  if (key == OF_KEY_SHIFT) {
    drawFPS = true;
  }
}

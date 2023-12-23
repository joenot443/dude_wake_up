//
//  OutputWindow.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/30/22.
//

#include "OutputWindow.hpp"

void OutputWindow::setup() {}

void OutputWindow::update()
{
}

void OutputWindow::draw()
{
  fbo->draw(0, 0);
}

void OutputWindow::setSource(std::shared_ptr<ofFbo> newSource) {
  fbo = newSource;
}

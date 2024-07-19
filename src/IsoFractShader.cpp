//
//  IsoFractShader.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 7/18/24.
//

#include <stdio.h>
#include "IsoFractShader.hpp"
#include "VideoSourceService.hpp"

void IsoFractShader::shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) {
  if (lastFrame->isAllocated()) {
//    lastFrame->begin();
//    ofSetColor(255, 255, 255);
//    ofDrawRectangle(0, 0, lastFrame->getWidth(), lastFrame->getHeight());
//    lastFrame->end();
  }
  
  canvas->begin();
  shader.begin();
  if (lastFrame->isAllocated()) {
    shader.setUniformTexture("tex", lastFrame->getTexture(), 4);
  }
  shader.setUniform1f("shaderValue", settings->shaderValue->value);
  shader.setUniform1f("time", ofGetElapsedTimef());
  shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
  shader.setUniform1f("alpha", settings->alpha->value);
  shader.setUniform1f("beta", settings->beta->value);
  shader.setUniform1f("gamma", settings->gamma->value);
  frame->draw(0, 0);
  shader.end();
  canvas->end();
  
  if (!lastFrame->isAllocated()) {
    lastFrame->allocate(frame->getWidth(), frame->getHeight());
  }
  lastFrame->begin();
  canvas->draw(0, 0);
  lastFrame->end();

}

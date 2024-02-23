//
//  FeedbackSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/12/23.
//

#include <stdio.h>
#include "FeedbackSource.hpp"
#include "FeedbackSourceService.hpp"
#include "Shader.hpp"

bool FeedbackSource::beingConsumed() {
  return FeedbackSourceService::getService()->isSourceBeingConsumed(id);
}

void FeedbackSource::shadeFrame(std::shared_ptr<Connectable> conn) {
  if (conn->connectableType() != ConnectableTypeShader) return;
  
  std::shared_ptr<Shader> shader = std::dynamic_pointer_cast<Shader>(conn);
  
  shader->traverseFrame(frameBuffer[startIndex]);
  
  // Get the final frame from that traversal chain
  
  std::shared_ptr<Shader> terminalShader = std::dynamic_pointer_cast<Shader>(shader->terminalDescendent());
  
  // Replace the relevant frame in our buffer
  auto canvas = frameBuffer[startIndex];
  canvas->begin();
  ofClear(0,0,0, 255);
  ofClear(0,0,0, 0);
  terminalShader->lastFrame->draw(0, 0, terminalShader->lastFrame->getWidth(), terminalShader->lastFrame->getHeight());
  canvas->end();
}



//
//  Shader.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/15/23.
//

#include <stdio.h>
#include "Shader.hpp"
#include "ShaderChainerService.hpp"

std::shared_ptr<FeedbackSource> Shader::feedbackDestination() {
  auto chainer = ShaderChainerService::getService()->shaderChainerForShaderId(id());
  if (chainer != nullptr) { return chainer->feedbackDestination; }
  
  return nullptr;
}


std::shared_ptr<ofTexture> Shader::auxTexture() {
  // Shader Aux input
  if (aux != nullptr && aux->feedbackDestination() != nullptr) {
    ofTexture feedbackTexture = *ShaderChainerService::getService()->shaderChainerForShaderId(aux->shaderId)->frameTexture.get();
    return std::shared_ptr<ofTexture>(&feedbackTexture);
  }
  
  // VideoSource Aux input
  if (sourceAux != nullptr) {
    return sourceAux->frameTexture;
  }
  
  return nullptr;
}

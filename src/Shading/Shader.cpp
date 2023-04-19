//
//  Shader.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/15/23.
//

#include <stdio.h>
#include "Shader.hpp"
#include "ShaderChainerService.hpp"
#include "FeedbackSourceService.hpp"
#include "NodeLayoutView.hpp"

std::shared_ptr<FeedbackSource> Shader::feedbackDestination() {
  auto chainer = ShaderChainerService::getService()->shaderChainerForShaderId(shaderId);
  if (chainer != nullptr) {
    // We're consuming this FeedbackSource, make sure it's reflected
    FeedbackSourceService::getService()->setConsumer(shaderId, chainer->feedbackDestination);
    return chainer->feedbackDestination;
  }
  
  return nullptr;
}


ofTexture Shader::auxTexture() {
  // Shader Aux input
  if (aux != nullptr && aux->feedbackDestination() != nullptr) {
    ofTexture feedbackTexture = *ShaderChainerService::getService()->shaderChainerForShaderId(aux->shaderId)->frameTexture.get();
    return feedbackTexture;
  }
  
  // VideoSource Aux input
  if (sourceAux != nullptr) {
    return *sourceAux->frameTexture;
  }
  
  return ofTexture();
}

void Shader::load(json j) {
  // We have a standard Shader Aux
  if (j["auxId"].is_string()) {
    aux = ShaderChainerService::getService()->shaderForId(j["auxId"]);
  }
  
  // We have a VideoSource Aux
  if (j["sourceAuxId"].is_string()) {
    
  }
}

json Shader::serialize() {
  settings->x->value = NodeLayoutView::getInstance()->nodeForShaderSourceId(shaderId)->position.x;
  settings->y->value = NodeLayoutView::getInstance()->nodeForShaderSourceId(shaderId)->position.y;
  
  json j = settings->serialize();
  j["shaderType"] = type();
  j["shaderId"] = settings->shaderId;
  j["x"] = settings->x->value;
  j["y"] = settings->y->value;
  
  if (auxConnected()) {
    if (sourceAux != nullptr) {
      j["sourceAuxId"] = sourceAux->id;
    }
    if (aux != nullptr) {
      j["auxId"] = aux->shaderId;
    }
  }
  
  return j;
};

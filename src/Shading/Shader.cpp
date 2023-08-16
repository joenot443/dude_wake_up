//
//  Shader.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/15/23.
//

#include <stdio.h>
#include "Shader.hpp"
#include "ShaderChainerService.hpp"
#include "VideoSourceService.hpp"
#include "FeedbackSourceService.hpp"
#include "NodeLayoutView.hpp"

// The "Final" selection for a FeedbackShader.
std::shared_ptr<FeedbackSource> Shader::chainerFeedbackDestination() {
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
  if (aux != nullptr) {
    return aux->lastFrame.getTexture();
  }
  
  // VideoSource Aux input
  if (auxSource != nullptr) {
    return *auxSource->frameTexture;
  }
  
  return ofTexture();
}

ofTexture Shader::maskTexture() {
  // Shader Mask input
  if (mask != nullptr) {
    auto chainer = ShaderChainerService::getService()->shaderChainerForShaderId(mask->shaderId);
    
    if (chainer != nullptr) {
      return *chainer->frameTexture;
    }
  }
  
  // VideoSource Mask input
  if (sourceMask != nullptr) {
    return *sourceMask->frameTexture;
  }
  
  return ofTexture();
}


void Shader::load(json j) {
  // We have a standard Shader Aux
  if (j["auxId"].is_string()) {
    aux = ShaderChainerService::getService()->shaderForId(j["auxId"]);
  }
  
  // We have a VideoSource Aux
  if (j["auxSourceId"].is_string()) {
    auxSource = VideoSourceService::getService()->videoSourceForId(j["auxSourceId"]);
  }
  
  // We have a standard Shader Mask
  if (j["maskId"].is_string()) {
    mask = ShaderChainerService::getService()->shaderForId(j["maskId"]);
  }
  
  // We have a VideoSource Mask
  if (j["sourceMaskId"].is_string()) {
    sourceMask = VideoSourceService::getService()->videoSourceForId(j["sourceMaskId"]);
  }
}

json Shader::serialize() {
  json j = settings->serialize();
  j["shaderType"] = type();
  j["shaderId"] = settings->shaderId;
  auto node = NodeLayoutView::getInstance()->nodeForShaderSourceId(shaderId);
  if (node != nullptr) {
    settings->x->value = NodeLayoutView::getInstance()->nodeForShaderSourceId(shaderId)->position.x;
    settings->y->value = NodeLayoutView::getInstance()->nodeForShaderSourceId(shaderId)->position.y;
    j["x"] = settings->x->value;
    j["y"] = settings->y->value;
  }
  
  if (auxConnected()) {
    if (auxSource != nullptr) {
      j["auxSourceId"] = auxSource->id;
    }
    if (aux != nullptr) {
      j["auxId"] = aux->shaderId;
    }
  }
  
  if (maskConnected()) {
    if (sourceMask != nullptr) {
      j["sourceMaskId"] = sourceMask->id;
    }
    if (mask != nullptr) {
      j["maskId"] = mask->shaderId;
    }
  }
  
  return j;
};

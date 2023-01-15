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
  return ShaderChainerService::getService()->shaderChainerForShaderId(id())->feedbackDestination;
}

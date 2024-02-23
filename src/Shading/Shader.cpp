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

void Shader::traverseFrame(std::shared_ptr<ofFbo> frame)
{
  clearLastFrame();
  shade(frame, lastFrame);
  for (std::shared_ptr<Connection> connection : outputs)
  {
    // We don't need to traverse Aux/Mask/Feedback connections
    if (connection->type == ConnectionTypeAux ||
        connection->type == ConnectionTypeFeedback ||
        connection->type == ConnectionTypeMask) continue;
    
    // Attempt to cast the connectable to a shader
    std::shared_ptr<Shader> shader = std::dynamic_pointer_cast<Shader>(connection->end);
    // If cast succeeded, traverse that shader's frame
    if (shader)
    {
      shader->traverseFrame(lastFrame);
    }
  }

  // If necessary, copy the texture to the Shader's FeedbackDestination
  if (feedbackDestination != nullptr && feedbackDestination->beingConsumed())
  {
    feedbackDestination->pushFrame(lastFrame);
  }
}

void Shader::clearLastFrame() {
  lastFrame->begin();
  ofSetColor(0, 0, 0, 255);
  ofDrawRectangle(0, 0, lastFrame->getWidth(), lastFrame->getHeight());
  ofClear(0,0,0, 255);
  ofClear(0,0,0, 0);
  lastFrame->end();
}

std::shared_ptr<VideoSourceSettings> Shader::sourceSettings() {
  if (hasParentOfType(ConnectableTypeSource)) {
    std::shared_ptr<Connectable> parentSource = parentOfType(ConnectableTypeSource);
    
    std::shared_ptr<VideoSource> castedSource = std::dynamic_pointer_cast<VideoSource>(parentSource);
    return castedSource->settings;
  } else {
    return VideoSourceService::getService()->defaultVideoSourceSettings();
  }
}

std::shared_ptr<Connectable> Shader::aux()
{
  // Shader Aux input
  if (hasInputForType(ConnectionTypeAux))
  {
    std::shared_ptr<Connectable> input = inputForType(ConnectionTypeAux);
    return input;
  }

  return nullptr;
}

std::shared_ptr<Connectable> Shader::mask()
{
  // Shader Mask input
  if (hasInputForType(ConnectionTypeMask))
  {
    std::shared_ptr<Connectable> input = inputForType(ConnectionTypeMask);
    return input;
  }

  return nullptr;
}

std::shared_ptr<Connectable> Shader::feedback()
{
  // Shader Mask input
  if (hasOutputForType(ConnectionTypeFeedback))
  {
    std::shared_ptr<Connectable> input = outputForType(ConnectionTypeFeedback);
    return input;
  }

  return nullptr;
}

void Shader::load(json j)
{
}

json Shader::serialize()
{
  json j = settings->serialize();
  j["shaderType"] = type();
  j["shaderId"] = settings->shaderId;
  auto node = NodeLayoutView::getInstance()->nodeForShaderSourceId(shaderId);
  if (node != nullptr)
  {
    settings->x->value = NodeLayoutView::getInstance()->nodeForShaderSourceId(shaderId)->position.x;
    settings->y->value = NodeLayoutView::getInstance()->nodeForShaderSourceId(shaderId)->position.y;
    j["x"] = settings->x->value;
    j["y"] = settings->y->value;
  }

  return j;
};

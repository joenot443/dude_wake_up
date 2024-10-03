//
//  Shader.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/15/23.
//

#include <stdio.h>
#include "Shader.hpp"
#include "Console.hpp"
#include "ShaderChainerService.hpp"
#include "VideoSourceService.hpp"
#include "FeedbackSourceService.hpp"
#include "NodeLayoutView.hpp"
#include "LayoutStateService.hpp"


void Shader::traverseFrame(std::shared_ptr<ofFbo> frame, int depth)
{
  clearLastFrame();
  checkForFileChanges();
  activateParameters();
  shade(frame, lastFrame);
  // On our terminal nodes, check if we need to update our defaultStageShader
  if (outputs.empty()) {
    if (depth > ParameterService::getService()->defaultStageShaderIdDepth.second) {
      ParameterService::getService()->defaultStageShaderIdDepth = std::pair<std::string, int>(shaderId, depth);
    }
  }
  
  for (auto [slot, connection] : outputs)
  {
    // Only traverse the Main slots.
    if (connection->inputSlot != InputSlotMain) { continue; }
    if (connection->outputSlot != OutputSlotMain) { continue; }
    
    // Attempt to cast the connectable to a shader
    std::shared_ptr<Shader> shader = std::dynamic_pointer_cast<Shader>(connection->end);
    // If cast succeeded, traverse that shader's frame
    if (shader)
    {
      shader->traverseFrame(lastFrame, depth + 1);
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
  } else {
    return 0;
  }

  return j;
};

void Shader::activateParameters() {
  for (std::shared_ptr<Parameter> param : settings->allParameters()) {
    param->active = true;
  }
}

void Shader::allocateLastFrame() {
  lastFrame->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y, GL_RGBA);
}

void Shader::enableAudioAutoReactivity(std::shared_ptr<Parameter> audioParam) {
  if (settings->audioReactiveParameter == nullptr) return;
  
  settings->audioReactiveParameter->addDriver(audioParam);
}

void Shader::disableAudioAutoReactivity() {
  settings->audioReactiveParameter->removeDriver();
}

void Shader::checkForFileChanges() {
  if (AllowShaderMonitoring != true) return;
  std::string shaderName = name();
  // Remove any spaces from the name
  shaderName.erase(std::remove(shaderName.begin(), shaderName.end(), ' '));
  
  auto path = "shaders/" + shaderName + ".frag";
  
  auto shaderFile = ofFile(path);
  
  if (!shaderFile.exists()) {
    log("Shader doesn't exist - " + name());
    return;
  }
  
  // Check when the file was last modified
  auto time = std::filesystem::last_write_time(shaderFile);
  
  if (time.time_since_epoch().count() > lastModified) {
    lastModified = time.time_since_epoch().count();
    shader.unload();
    // Print the contents of the shader file
    log("Shader file contents - " + shaderFile.readToBuffer().getText());
    
    shader.load("shaders/" + shaderName);
    log("Reloaded Shader for - " + shaderName);
  }
}

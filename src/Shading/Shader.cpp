//
//  Shader.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/15/23.
//

#include <stdio.h>
#include "Shader.hpp"
#include "NodeLayoutView.hpp"
#include "Colors.hpp"
#include "LayoutStateService.hpp"
#include "Console.hpp"
#include "ShaderChainerService.hpp"
#include "VideoSourceService.hpp"
#include "FeedbackSourceService.hpp"
#include "LayoutStateService.hpp"


void Shader::traverseFrame(std::shared_ptr<ofFbo> frame, int depth)
{
  if (!active) return;
  clearLastFrame();
  checkForFileChanges();
  activateParameters();
  shade(frame, lastFrame);
  applyOptionalShaders();
  
  // On our terminal nodes, check if we need to update our defaultStageShader
  if (outputs.empty()) {
    if (depth > ParameterService::getService()->defaultStageShaderIdDepth.second) {
      ParameterService::getService()->defaultStageShaderIdDepth = std::pair<std::string, int>(shaderId, depth);
    }
  }
  
  for (auto [slot, connections] : outputs) // Correctly iterate over connections
  {
    for (auto &connection : connections) {
      if (connection->inputSlot != InputSlotMain) { continue; }
      if (connection->outputSlot != OutputSlotMain) { continue; }
      
      std::shared_ptr<Shader> shader = std::dynamic_pointer_cast<Shader>(connection->end);
      if (shader)
      {
        shader->traverseFrame(lastFrame, depth + 1);
      }
    }
  }

  // If necessary, copy the texture to the Shader's FeedbackDestination
  if (feedbackDestination != nullptr && feedbackDestination->beingConsumed() && feedbackDestination->type == FeedbackType_full)
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
  for (std::shared_ptr<Parameter> param : settings->parameters) {
    param->active = true;
  }
}

void Shader::allocateFrames() {
  lastFrame->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y, GL_RGBA);
  
  optionalFrame->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y, GL_RGBA);
}

void Shader::enableAudioAutoReactivity(std::shared_ptr<Parameter> audioParam) {
  if (settings->audioReactiveParameter == nullptr) return;
  
  settings->audioReactiveParameter->addDriver(audioParam);
}

void Shader::disableAudioAutoReactivity() {
  settings->audioReactiveParameter->removeDriver();
}

std::shared_ptr<ofFbo> Shader::parentFrame() {
    for (auto const& [key, connection] : inputs) {
      return connection->start->frame();
    }
    return nullptr;
}

void Shader::checkForFileChanges() {
  if (!AllowShaderMonitoring) return;
  
  std::string shaderName = fileName();
  auto path = "shaders/" + shaderName + ".frag";
  
  auto shaderFile = ofFile(path);
  
  if (!shaderFile.exists()) {
//    log("Shader doesn't exist - " + name());
    return;
  }
  
  // Check when the file was last modified
  auto time = std::filesystem::last_write_time(shaderFile);
  auto timeSinceEpoch = std::chrono::duration_cast<std::chrono::seconds>(time.time_since_epoch()).count();
  
  if (timeSinceEpoch > lastModified) {
    lastModified = timeSinceEpoch;
    shader.unload();
    // Print the contents of the shader file
    log("Shader file contents - " + shaderFile.readToBuffer().getText());
    
    shader.load("shaders/" + shaderName);
    log("Reloaded Shader for - " + shaderName);
  }
}

void Shader::drawOptionalSettings() {
  if (ImGui::CollapsingHeader("Extra Settings", ImGuiTreeNodeFlags_CollapsingHeader))
  {
    for (auto shader : optionalShaders) {
      if (ImGui::CollapsingHeader(shader->name().c_str())) {
        shader->optionallyEnabled = true;
        shader->drawSettings();
      } else {
        shader->optionallyEnabled = false;
      }
    }
  }
}

void Shader::applyOptionalShaders() {
  for (auto shader: optionalShaders) {
    if (!shader->optionallyEnabled) continue;
    
    shader->shade(lastFrame, optionalFrame);
    populateLastFrame();
  }
}

void Shader::drawPreview(ImVec2 pos, float scale)
{
  ImTextureID texID = (ImTextureID)(uintptr_t)lastFrame->getTexture().getTextureData().textureID;
  ImGui::Image(texID, LayoutStateService::getService()->previewSize(scale));
}

void Shader::drawPreviewSized(ImVec2 size)
{
  ImTextureID texID = (ImTextureID)(uintptr_t)lastFrame->getTexture().getTextureData().textureID;
  ImVec2 cursorPos = ImGui::GetCursorPos();
  ImGui::GetWindowDrawList()->AddImageRounded(
    texID,
    cursorPos,                          // top-left corner
    cursorPos + size,                   // bottom-right corner
    ImVec2(0,0),                        // UV coordinates for top-left
    ImVec2(1,1),                        // UV coordinates for bottom-right
    IM_COL32_WHITE,
    8.0
  );
}

void Shader::generateOptionalShaders() {
  if (isOptional) return;
  if (!optionalShaders.empty()) return;
  
  std::vector<ShaderType> optionalShaderTypes = {ShaderTypeHSB, ShaderTypeBackground, ShaderTypeTransform, ShaderTypeRotate, ShaderTypeDoubleBlur, ShaderTypeMirror};
  
  for (ShaderType type : optionalShaderTypes) {
    std::shared_ptr<Shader> newShader = ShaderChainerService::getService()->shaderForType(type, UUID::generateUUID(), 0);
    newShader->isOptional = true;
    optionalShaders.push_back(newShader);
  }
}

void Shader::populateLastFrame() {
  lastFrame->begin();
  ofClear(0,0,0,0);
  optionalFrame->draw(0, 0, lastFrame->getWidth(), lastFrame->getHeight());
  lastFrame->end();
}

bool Shader::allowAuxOutputSlot() {
  return type() == ShaderTypeFeedback || type() == ShaderTypeColorKeyMaskMaker || type() == ShaderTypeLumaMaskMaker;
}

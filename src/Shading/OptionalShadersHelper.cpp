// src/Shading/OptionalShadersHelper.cpp

#include "OptionalShadersHelper.hpp"
#include "Shader.hpp"
#include "ShaderChainerService.hpp"

void OptionalShadersHelper::generateOptionalShaders() {
  std::vector<ShaderType> optionalShaderTypes = {ShaderTypeHSB, ShaderTypeTransform, ShaderTypeRotate, ShaderTypeDoubleBlur, ShaderTypeMirror};
  for (ShaderType type : optionalShaderTypes) {
    std::shared_ptr<Shader> newShader = ShaderChainerService::getService()->shaderForType(type, UUID::generateUUID(), 0);
    newShader->isOptional = true;
    optionalShaders.push_back(newShader);
  }
}

void OptionalShadersHelper::applyOptionalShaders(std::shared_ptr<ofFbo> lastFrame, std::shared_ptr<ofFbo> optionalFrame) {
  for (auto shader : optionalShaders) {
    if (!shader->optionallyEnabled) continue;
    shader->shade(lastFrame, optionalFrame);
    populateLastFrame(lastFrame, optionalFrame);
  }
}

void OptionalShadersHelper::drawOptionalSettings() {
  if (ImGui::CollapsingHeader("Extra Settings", ImGuiTreeNodeFlags_CollapsingHeader)) {
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

void OptionalShadersHelper::populateLastFrame(std::shared_ptr<ofFbo> lastFrame, std::shared_ptr<ofFbo> optionalFrame) {
  lastFrame->begin();
  ofClear(0, 0, 0, 0);
  optionalFrame->draw(0, 0, lastFrame->getWidth(), lastFrame->getHeight());
  lastFrame->end();
}

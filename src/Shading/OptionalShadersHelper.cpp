// src/Shading/OptionalShadersHelper.cpp

#include "OptionalShadersHelper.hpp"
#include "ColorKeyMaskMakerShader.hpp"
#include "LumaMaskMakerShader.hpp"
#include "Shader.hpp"
#include "ShaderChainerService.hpp"

void OptionalShadersHelper::generateOptionalShaders() {
  std::vector<ShaderType> optionalShaderTypes = {ShaderTypeHSB, ShaderTypeTransform, ShaderTypeRotate, ShaderTypeDoubleBlur, ShaderTypeMirror, ShaderTypeColorKeyMaskMaker, ShaderTypeLumaMaskMaker};
  for (ShaderType type : optionalShaderTypes) {
    std::shared_ptr<Shader> newShader = ShaderChainerService::getService()->shaderForType(type, UUID::generateUUID(), 0);
    newShader->isOptional = true;
    optionalShaders.push_back(newShader);
    if (newShader->type() == ShaderTypeColorKeyMaskMaker) {
      std::shared_ptr<ColorKeyMaskMakerShader> colorKeyShader = std::dynamic_pointer_cast<ColorKeyMaskMakerShader>(newShader);
      colorKeyShader->settings->drawInput->setBoolValue(true);
      colorKeyShader->settings->color->setColor({0.0, 0.0, 0.0, 1.0});
      colorKeyShader->settings->tolerance->setValue(0.1);
    }
    if (newShader->type() == ShaderTypeLumaMaskMaker) {
      std::shared_ptr<LumaMaskMakerShader> lumaMaskShader = std::dynamic_pointer_cast<LumaMaskMakerShader>(newShader);
      lumaMaskShader->settings->drawInput->setBoolValue(true);
    }
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
      if (ImGui::CollapsingHeader(idAppendedToString(shader->name().c_str(), shader->shaderId.c_str()).c_str())) {
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
  ofClear(0, 0, 0, 255);
  ofClear(0, 0, 0, 0);
  optionalFrame->draw(0, 0, lastFrame->getWidth(), lastFrame->getHeight());
  lastFrame->end();
}

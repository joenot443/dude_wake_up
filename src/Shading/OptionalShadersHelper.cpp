// src/Shading/OptionalShadersHelper.cpp

#include "OptionalShadersHelper.hpp"
#include "ColorKeyMaskMakerShader.hpp"
#include "LumaMaskMakerShader.hpp"
#include "Shader.hpp"
#include "ShaderChainerService.hpp"

std::vector<ShaderType> OptionalShadersHelper::shaderTypes() {
  return {ShaderTypeHSB, ShaderTypeBackground, ShaderTypeTransform, ShaderTypeRotate, ShaderTypeDoubleBlur, ShaderTypeMirror, ShaderTypeColorKeyMaskMaker, ShaderTypeLumaMaskMaker, ShaderTypeLighten, ShaderTypeDarken};
}

std::vector<ShaderType> OptionalShadersHelper::sourceTypes() {
  return {ShaderTypeHSB, ShaderTypeTransform, ShaderTypeRotate, ShaderTypeDoubleBlur, ShaderTypeMirror, ShaderTypeColorKeyMaskMaker, ShaderTypeLumaMaskMaker, ShaderTypeLighten, ShaderTypeDarken};
}

OptionalShadersHelper::OptionalShadersHelper(std::vector<ShaderType> types, bool maskDefaults)
    : types_(std::move(types)), maskDefaults_(maskDefaults), slots_(types_.size()), pending_(types_.size()) {}

std::string OptionalShadersHelper::name(size_t index) const {
  return index < types_.size() ? shaderTypeName(types_[index]) : std::string();
}

bool OptionalShadersHelper::enabled(size_t index) const {
  auto s = existing(index);
  return s && s->optionallyEnabled;
}

std::shared_ptr<Shader> OptionalShadersHelper::existing(size_t index) const {
  std::lock_guard<std::mutex> lock(mutex_);
  return index < slots_.size() ? slots_[index] : nullptr;
}

std::shared_ptr<Shader> OptionalShadersHelper::shader(size_t index) {
  if (index >= types_.size()) return nullptr;
  if (auto s = existing(index)) return s;
  return create(index);
}

std::shared_ptr<Shader> OptionalShadersHelper::create(size_t index) {
  // Built outside the lock (it compiles a program); only this thread creates.
  std::shared_ptr<Shader> newShader = ShaderChainerService::getService()->shaderForType(types_[index], NottawaUUID::generateUUID(), 0);
  if (!newShader) return nullptr;
  newShader->isOptional = true;
  if (maskDefaults_ && newShader->type() == ShaderTypeColorKeyMaskMaker) {
    std::shared_ptr<ColorKeyMaskMakerShader> colorKeyShader = std::dynamic_pointer_cast<ColorKeyMaskMakerShader>(newShader);
    colorKeyShader->settings->drawInput->setBoolValue(true);
    colorKeyShader->settings->color->setColor({0.0, 0.0, 0.0, 1.0});
    colorKeyShader->settings->tolerance->setValue(0.1);
  }
  if (maskDefaults_ && newShader->type() == ShaderTypeLumaMaskMaker) {
    std::shared_ptr<LumaMaskMakerShader> lumaMaskShader = std::dynamic_pointer_cast<LumaMaskMakerShader>(newShader);
    lumaMaskShader->settings->drawInput->setBoolValue(true);
  }

  nlohmann::json entry;
  {
    std::lock_guard<std::mutex> lock(mutex_);
    entry = std::move(pending_[index]);
    pending_[index] = nullptr;
  }
  if (entry.is_object()) {
    if (entry.contains("enabled") && entry["enabled"].is_boolean()) newShader->optionallyEnabled = entry["enabled"].get<bool>();
    if (entry.contains("settings")) newShader->settings->load(entry["settings"]);
  }

  std::lock_guard<std::mutex> lock(mutex_);
  slots_[index] = newShader;
  return newShader;
}

void OptionalShadersHelper::toggle(size_t index) {
  if (auto s = shader(index)) s->optionallyEnabled = !s->optionallyEnabled;
}

std::vector<std::shared_ptr<Shader>> OptionalShadersHelper::created() const {
  std::lock_guard<std::mutex> lock(mutex_);
  std::vector<std::shared_ptr<Shader>> out;
  for (auto& s : slots_) if (s) out.push_back(s);
  return out;
}

void OptionalShadersHelper::load(const nlohmann::json& entries) {
  if (!entries.is_array()) return;
  for (size_t i = 0; i < entries.size() && i < types_.size(); i++) {
    const auto& entry = entries[i];
    if (!entry.is_object()) continue;
    const bool on = entry.contains("enabled") && entry["enabled"].is_boolean() && entry["enabled"].get<bool>();
    if (auto s = existing(i)) {
      s->optionallyEnabled = on;
      if (entry.contains("settings")) s->settings->load(entry["settings"]);
      continue;
    }
    {
      std::lock_guard<std::mutex> lock(mutex_);
      pending_[i] = entry;
    }
    if (on) create(i);
  }
}

nlohmann::json OptionalShadersHelper::serialize() const {
  std::lock_guard<std::mutex> lock(mutex_);
  bool any = false;
  nlohmann::json out = nlohmann::json::array();
  for (size_t i = 0; i < types_.size(); i++) {
    if (slots_[i]) {
      nlohmann::json entry;
      entry["type"] = slots_[i]->type();
      entry["enabled"] = slots_[i]->optionallyEnabled;
      entry["settings"] = slots_[i]->settings->serialize();
      out.push_back(entry);
      any = true;
    } else if (pending_[i].is_object()) {
      out.push_back(pending_[i]);
      any = true;
    } else {
      out.push_back({{"type", types_[i]}, {"enabled", false}});
    }
  }
  return any ? out : nlohmann::json();
}

void OptionalShadersHelper::applyOptionalShaders(std::shared_ptr<ofFbo> lastFrame, std::shared_ptr<ofFbo> optionalFrame) {
  for (auto shader : created()) {
    if (!shader->optionallyEnabled) continue;

    // The scratch frame is only needed here, so it's allocated on first use.
    if (!optionalFrame->isAllocated() ||
        optionalFrame->getWidth() != lastFrame->getWidth() ||
        optionalFrame->getHeight() != lastFrame->getHeight()) {
      optionalFrame->allocate(lastFrame->getWidth(), lastFrame->getHeight(), GL_RGBA);
    }

    // Clear optionalFrame before applying the shader
    optionalFrame->begin();
    ofClear(0, 0, 0, 255);
    ofClear(0, 0, 0, 0);
    optionalFrame->end();

    shader->shade(lastFrame, optionalFrame);
    populateLastFrame(lastFrame, optionalFrame);
  }
}

void OptionalShadersHelper::drawOptionalSettings() {
  if (ImGui::CollapsingHeader("Extra Settings", ImGuiTreeNodeFlags_CollapsingHeader)) {
    for (size_t i = 0; i < types_.size(); i++) {
      if (ImGui::CollapsingHeader(idAppendedToString(name(i).c_str(), (std::to_string(reinterpret_cast<uintptr_t>(this)) + "-" + std::to_string(i)).c_str()).c_str())) {
        auto s = shader(i);
        if (!s) continue;
        s->optionallyEnabled = true;
        s->drawSettings();
      } else if (auto s = existing(i)) {
        s->optionallyEnabled = false;
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

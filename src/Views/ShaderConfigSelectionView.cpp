//
//  ShaderConfigSelectionView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/4/23.
//

#include "ShaderConfigSelectionView.hpp"
#include "ofxImGui.h"
#include "ConfigService.hpp"

void ShaderConfigSelectionView::draw(std::shared_ptr<Shader> shader) {
  if (ImGui::CollapsingHeader("Saved Configs", ImGuiTreeNodeFlags_CollapsingHeader)) {
    auto configs = ConfigService::getService()->availableConfigsForShaderType(shader->type());
    std::vector<std::string> names;
    
    // Populate names vector with configuration names
    for (const auto& config : configs) {
      names.push_back(config.name);
    }
    
    static int selection = -1;
    // Create a vector of const char* pointers pointing to the strings in names vector
    std::vector<const char*> cStrNames;
    for (const auto& name : names) {
      cStrNames.push_back(name.c_str());
    }
    ImGui::SetNextItemWidth(200.0);
    if (ImGui::Combo("Select Config", &selection, cStrNames.data(), cStrNames.size())) {
      shader->settings->load(ConfigService::getService()->shaderConfigForPath(configs[selection].path));
    }
    
    ImGui::SetNextItemWidth(200.0);
    static char inputText[256] = ""; // Reserve enough space for the input text
    ImGui::InputText("Name", inputText, sizeof(inputText));
    ImGui::SameLine();
    if (ImGui::Button("Save")) {
      ConfigService::getService()->saveShaderConfigFile(shader, std::string(inputText));
    }
  }
}

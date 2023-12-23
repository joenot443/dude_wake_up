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
  auto configs = ConfigService::getService()->availableConfigsForShaderType(shader->type());
  std::vector<std::string> names = {};
  //allocate space for pointers
  char** out = new char*[configs.size()];
  
  for(int i = 0; i < configs.size(); ++i){
    string s = configs[i].name;
    //allocate space for c string (note +1 for extra space)
    char* s_cstr = new char[s.size()+1];
    //copy data to prevent mutation of original vector
    strcpy(s_cstr, s.c_str());
    //put pointer in the output
    out[i] = s_cstr;
  }
  static int selection = -1;
  if (ImGui::Combo("Select Config", &selection, out, configs.size())) {
    shader->settings->load(ConfigService::getService()->shaderConfigForPath(configs[selection].path));
  }
  
  static char inputText[256] = ""; // Reserve enough space for the input text
  ImGui::InputText("Name", inputText, sizeof(inputText));
  ImGui::SameLine();
  if (ImGui::Button("Save")) {
    ConfigService::getService()->saveShaderConfigFile(shader, std::string(inputText));
  }
  
}

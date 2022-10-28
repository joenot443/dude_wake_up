//
//  ShaderChainerView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 10/18/22.
//

#include "ShaderChainerView.hpp"
#include "HSBShader.hpp"
#include "BlurShader.hpp"
#include "PixelShader.hpp"
#include "FeedbackShader.hpp"
#include "CommonViews.hpp"

static const float ButtonWidth = 120.0;
static const float ComboWidth = 160.0;

static const ShaderType AvailableShaderTypes[] = {ShaderTypeHSB, ShaderTypeBlur, ShaderTypePixelate, ShaderTypeFeedback};

void ShaderChainerView::setup() {
  
}

void ShaderChainerView::update() {
  
}

void ShaderChainerView::draw() {
  drawChainer();
  
  auto selectedShaderName = shaderTypeName(selectedShaderType);
  
  CommonViews::HorizontallyAligned(ButtonWidth + ComboWidth);
  ImGui::PushItemWidth(ButtonWidth);
  if (ImGui::BeginCombo("##shader_combo", selectedShaderName.c_str()))
  {
    for (auto shaderType : AvailableShaderTypes) {
      if (ImGui::Selectable(shaderTypeName(shaderType).c_str(), selectedShaderType == shaderType))
          selectedShaderType = shaderType;
    }
    
    ImGui::EndCombo();
  }
  
  ImGui::SameLine();
  
  if (ImGui::Button("Add New Shader") && selectedShaderType != ShaderTypeNone) {
    shaderChainer->pushShader(selectedShaderType);
  }
}

void ShaderChainerView::drawShaderButton(Shader *shader, bool selected) {
  if (selected) {
    pushSelectedButtonStyle();
  }
  
  CommonViews::HorizontallyAligned(ButtonWidth);
    
  if(ImGui::Button(shader->name().c_str(), ImVec2(ButtonWidth, 60))) {
    selectedShader = shader;
  }
  
  if (selected) {
    ImGui::PopStyleColor(3);
  }
}



void ShaderChainerView::drawChainer() {
  for (int n = 0; n < shaders->size(); n++) {
    auto shader = (*shaders)[n];
    bool selected = shader == selectedShader;
    ImGui::PushID(n);
    
    drawShaderButton(shader, selected);
    // Our buttons are both drag sources and drag targets here!
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
    {
      // Set payload to carry the index of our item (could be anything)
      ImGui::SetDragDropPayload("ShaderChainerDrop", &n, sizeof(int));
      
      ImGui::Text("%s", shader->name().c_str());
      ImGui::EndDragDropSource();
    }
    if (ImGui::BeginDragDropTarget())
    {
      if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("ShaderChainerDrop"))
      {
        int payload_n = *(const int*)payload->Data;
        auto tmp = (*shaders)[n];
        (*shaders)[n] = (*shaders)[payload_n];
        (*shaders)[payload_n] = tmp;
      }
      ImGui::EndDragDropTarget();
    }
    
    ImGui::SameLine();

    if (ImGui::Button("X", ImVec2(30, 30))) {
      shaderChainer->deleteShader(shader);
    }
    
    CommonViews::CenteredVerticalLine();

    ImGui::PopID();
  }
}

void ShaderChainerView::pushSelectedButtonStyle() {
  ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.7f, 0.6f, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.7f, 0.7f, 0.7f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.7f, 0.8f, 0.8f));
}
  

void ShaderChainerView::setShaderChainer(ShaderChainer *chainer) {
  shaderChainer = chainer;
  shaders = &shaderChainer->shaders;
  selectedShader = nullptr;  
}

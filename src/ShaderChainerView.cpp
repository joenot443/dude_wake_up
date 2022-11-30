//
//  ShaderChainerView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 10/18/22.
//

#include "ShaderChainerView.hpp"
#include "HSBShader.hpp"
#include "BlurShader.hpp"
#include "ShaderChainerService.hpp"
#include "PixelShader.hpp"
#include "FeedbackShader.hpp"
#include "CommonViews.hpp"

static const float ButtonWidth = 120.0;
static const float ComboWidth = 160.0;

void ShaderChainerView::setup() {
  
}

void ShaderChainerView::update() {
  
}

void ShaderChainerView::draw() {
  drawChainer();
}

void ShaderChainerView::drawChainer() {
  auto startX = ImGui::GetCursorPosX();
  
  for (int n = 0; n < shaderChainer->shaders.size(); n++) {
    auto shader = shaderChainer->shaders[n];
    bool selected = shader == ShaderChainerService::getService()->selectedShader;
    ImGui::PushID(n);
    
    drawShaderButton(shader, selected);
    
    // Our buttons are both drag sources and drag targets here!
    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceAllowNullID))
    {
      // Set payload to carry the index of our item (could be anything)
      ImGui::SetDragDropPayload("SwapShader", &n, sizeof(int));
      
      ImGui::Text("%s", shader->name().c_str());
      ImGui::EndDragDropSource();
    }
    
    

    if (ImGui::BeginDragDropTarget())
    {
      if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("SwapShader"))
      {
        int payload_n = *(const int*)payload->Data;
        auto tmp = shaderChainer->shaders[n];
        shaderChainer->shaders[n] = shaderChainer->shaders[payload_n];
        shaderChainer->shaders[payload_n] = tmp;
      }
      ImGui::EndDragDropTarget();
    }

    // if (ImGui::Button("X", ImVec2(30, 30))) {
    //   shaderChainer->deleteShader(shared);
    //   selectedShader = nullptr;
    // }

    ImGui::PopID();
    // Only draw on the same line if we're not the last item
    ImGui::SameLine();
  }
  
  drawShaderDropZone();

  // Draws a Selectable around the ShaderChainer
  ImGui::SameLine();
  auto buttonCount = shaderChainer->shaders.size() + 1;
  auto chainerWidth = buttonCount * (ButtonWidth + ImGui::GetStyle().ItemSpacing.x);
  auto columnSize = ImGui::GetWindowWidth() * 0.5;
  auto selectorSize = ImVec2(columnSize, 100);
  
  ImGui::SetCursorPosX(startX);
  ImGui::PushStyleColor(ImGuiCol_Header, (ImVec4)ImColor::HSV(0.2f, 0.4f, 0.4f, 0.2f));
  ImGui::PushStyleColor(ImGuiCol_HeaderHovered, (ImVec4)ImColor::HSV(0.2f, 0.4f, 0.4f, 0.5f));
  ImGui::PushStyleColor(ImGuiCol_HeaderActive, (ImVec4)ImColor::HSV(0.2f, 0.4f, 0.4f, 0.8f));

  auto name = formatString("%s##selectable", shaderChainer->name.c_str());
  if (ImGui::Selectable(name.c_str(), shaderChainer == ShaderChainerService::getService()->selectedShaderChainer, ImGuiSelectableFlags_None, selectorSize)) {
    // Select the shader chainer
    ShaderChainerService::getService()->selectShaderChainer(shaderChainer);
  }

  ImGui::PopStyleColor(3);
  
  ImGui::NewLine();
}

void ShaderChainerView::drawShaderButton(std::shared_ptr<Shader> shader, bool selected) {
  if (selected) {
    pushSelectedButtonStyle();
  }
    
  if(ImGui::Button(formatString("%s##%s", shader->name().c_str(), shaderChainer->id.c_str()).c_str(), ImVec2(ButtonWidth, 60))) {
    ShaderChainerService::getService()->selectedShader = shader;
  }

  if (selected) {
    ImGui::PopStyleColor(3);
  }
}

void ShaderChainerView::drawShaderDropZone() {
  ImGui::Button("Drop Shader", ImVec2(ButtonWidth, 60));
  
  if (ImGui::BeginDragDropTarget())
  {
    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("NewShader"))
    {
      int n = *(const int*)payload->Data;
      ShaderType shaderType = (ShaderType) n;
      shaderChainer->pushShader(shaderType);
    }
    ImGui::EndDragDropTarget();
  }
}


void ShaderChainerView::pushSelectedButtonStyle() {
  ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.7f, 0.6f, 0.6f));
  ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(0.7f, 0.7f, 0.7f));
  ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(0.7f, 0.8f, 0.8f));
}

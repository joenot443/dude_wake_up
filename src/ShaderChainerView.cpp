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

static const float ButtonWidth = 80.0;
static const float ComboWidth = 160.0;
static const float XButtonWidth = 20.0;

void ShaderChainerView::setup() {
  
}

void ShaderChainerView::update() {
  
}

void ShaderChainerView::draw() {
  drawChainer();
}

void ShaderChainerView::drawChainer() {
  splitter = ImDrawListSplitter();
  splitter.Split(ImGui::GetWindowDrawList(), 3);
  splitter.SetCurrentChannel(ImGui::GetWindowDrawList(), 1);
  
  drawSourceButton();
  
  ImGui::SameLine();
  
  for (int n = 0; n < shaderChainer->shaders.size(); n++) {
    auto shader = shaderChainer->shaders[n];
    bool selected = shader == ShaderChainerService::getService()->selectedShader;
    ImGui::PushID(n);
    
    drawShaderButton(shader, selected, n);
    
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


    ImGui::PopID();
    // Only draw on the same line if we're not the last item
    ImGui::SameLine();
  }
  
  drawShaderDropZone();

  // Draws a Selectable around the ShaderChainer
  ImGui::SameLine();

  splitter.Merge(ImGui::GetWindowDrawList());
  
  ImGui::NewLine();
}

void ShaderChainerView::drawSourceButton() {
  auto sourceName = shaderChainer->source->sourceName;
  if (ImGui::Button(formatString("Source \n %s ##%s", sourceName.c_str(), shaderChainer->id.c_str()).c_str(), ImVec2(ButtonWidth, 60))) {
    ShaderChainerService::getService()->selectShaderChainer(shaderChainer);
  }
}

void ShaderChainerView::drawShaderButton(std::shared_ptr<Shader> shader, bool selected, int idx) {
  if (selected) {
    pushSelectedButtonStyle();
  }
  
  ImGui::SetItemAllowOverlap();

  if (ImGui::Button(formatString("%s##%s", shader->name().c_str(), shaderChainer->id.c_str()).c_str(), ImVec2(ButtonWidth, 60))) {
    ShaderChainerService::getService()->selectedShader = shader;
  }
  
  if (selected) {
    ImGui::PopStyleColor(3);
  }
  
  
  splitter.SetCurrentChannel(ImGui::GetWindowDrawList(), 2);
  
  drawShaderCloseButton(shader);
  
  splitter.SetCurrentChannel(ImGui::GetWindowDrawList(), 1);
}

void ShaderChainerView::drawShaderCloseButton(std::shared_ptr<Shader> shader) {
  auto cursorX = ImGui::GetCursorPosX();
  ImGui::SameLine();
  ImGui::SetItemAllowOverlap();
  if (ImGui::Button("X")) {
    shaderChainer->deleteShader(shader);
    if (ShaderChainerService::getService()->selectedShader == shader) {
      ShaderChainerService::getService()->selectedShader = nullptr;
    }
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

//
//  TextEditorView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/17/23.
//

#include "ofMain.h"
#include "ofxImGui.h"
#include "DisplayText.hpp"
#include "LayoutStateService.hpp"
#include "ImGuiExtensions.hpp"
#include "CommonViews.hpp"
#include "TextEditorView.hpp"
#include "UUID.hpp"
#include "dear_widgets.h"
#include "Strings.hpp"
#include "ConfigService.hpp"

TextEditorView::TextEditorView(std::shared_ptr<DisplayText> displayText)
: displayText(displayText), selectedFontIndex(0), id(UUID::generateUUID()) {
  ofDirectory fontsDir = ofDirectory("fonts/editor");
  fontsDir.listDir();
  
  for (auto& file : fontsDir.getFiles()) {
    fonts.push_back(Font(removeFileExtension(file.getFileName())));
  }
  displayText->font = fonts[selectedFontIndex];
  font.load(displayText->font.path(), displayText->fontSize);
}

void TextEditorView::draw() {
  // Use ImGui::LabelText for left-aligned labels
  ImGui::Text("Text:");
  ImGui::SameLine();
  
  char buf[1024];
  strncpy(buf, displayText->text.c_str(), sizeof(buf));
  buf[sizeof(buf) - 1] = 0;
  if (ImGui::InputTextMultiline("##TextValue", buf, sizeof(buf), ImVec2(150., 100.))) {
    displayText->text = buf;
  }
  LayoutStateService::getService()->isEditingText = ImGui::IsItemActive();
  
  CommonViews::sSpacing();
  ImGui::Text("Color");
  CommonViews::ShaderColor(displayText->color);
  CommonViews::sSpacing();
  CommonViews::ShaderCheckbox(displayText->strokeEnabled);
  if (displayText->strokeEnabled->boolValue) {
  ImGui::Text("Stroke Color");
  CommonViews::ShaderColor(displayText->strokeColor);
  }
  int currentFontSize = displayText->fontSize;
  
  CommonViews::sSpacing();
  ImGui::Text("Font Size");
  ImGui::SameLine();
  if (ImGui::SliderInt("##FontSizeValue", &currentFontSize, 1, 300)) {
    displayText->fontSize = currentFontSize;
    font.load(displayText->font.path(), displayText->fontSize);
  }
  
  CommonViews::MultiSlider("Position", displayText->id, displayText->xPosition, displayText->yPosition, displayText->xPositionOscillator, displayText->yPositionOscillator);
  
  // Font selection
  CommonViews::sSpacing();
  ImGui::Text("Font");
  ImGui::SameLine();
  if (ImGui::BeginCombo("##FontsValue", fonts[selectedFontIndex].name.c_str())) {
    for (int i = 0; i < fonts.size(); ++i) {
      bool isSelected = (selectedFontIndex == i);
      if (ImGui::Selectable(fonts[i].name.c_str(), isSelected)) {
        selectedFontIndex = i;
        displayText->font = fonts[i];
      }
      if (isSelected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
}

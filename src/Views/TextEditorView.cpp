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
#include "Strings.hpp"
#include "ConfigService.hpp"

TextEditorView::TextEditorView(std::shared_ptr<DisplayText> displayText)
: displayText(displayText), id(UUID::generateUUID()) {
  ofDirectory fontsDir = ofDirectory("fonts/editor");
  fontsDir.listDir();
  
  // Collect font names
  for (auto& file : fontsDir.getFiles()) {
    fontNames.push_back(removeFileExtension(file.getFileName()));
  }
  
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
	  CommonViews::ShaderColor(displayText->strokeColor);
    CommonViews::ShaderParameter(displayText->strokeWeight, nullptr);
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
  CommonViews::ShaderOption(displayText->fontSelector, fontNames);
  
  // Update font if selection changed
  if (displayText->font.name != fontNames[displayText->fontSelector->intValue]) {
    displayText->font = Font(fontNames[displayText->fontSelector->intValue]);
    font.load(displayText->font.path(), displayText->fontSize);
  }
}

//
//  TextEditorView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/17/23.
//

#include "ofMain.h"
#include "ofxImGui.h"
#include "DisplayText.hpp"
#include "ImGuiExtensions.hpp"
#include "CommonViews.hpp"
#include "TextEditorView.hpp"
#include "dear_widgets.h"
#include "ConfigService.hpp"

TextEditorView::TextEditorView(std::shared_ptr<DisplayText> displayText)
: displayText(displayText), selectedFontIndex(0) {
  ofDirectory fontsDir = ofDirectory("fonts/editor");
  fontsDir.listDir();
  
  
  for (auto& file : fontsDir.getFiles()) {
    fonts.push_back(Font(file.getFileName(), file.getAbsolutePath()));
  }
  displayText->font = fonts[selectedFontIndex];
  font.load(displayText->font.path, displayText->fontSize);
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
  
  ImVec4 color(displayText->color.r / 255.0f, displayText->color.g / 255.0f, displayText->color.b / 255.0f, 1.0f);
  
  CommonViews::sSpacing();
  ImGui::Text("Color");
  ImGui::SameLine();
  if (ImGui::ColorEdit3("##ColorValue", (float*)&color)) {
    displayText->color = ofColor(color.x * 255.0f, color.y * 255.0f, color.z * 255.0f);
  }
  
  int currentFontSize = displayText->fontSize;
  
  CommonViews::sSpacing();
  ImGui::Text("Font Size");
  ImGui::SameLine();
  if (ImGui::SliderInt("##FontSizeValue", &currentFontSize, 1, 100)) {
    displayText->fontSize = currentFontSize;
    font.load(displayText->font.path, displayText->fontSize);
  }
  
  CommonViews::sSpacing();
  ImGui::Text("Position");
  ImGui::SameLine();
  drawPositionSlider();
  
  ImGui::Text("X Oscillator");
  ImGui::SameLine();
  CommonViews::OscillateButton("##xTextOsc", displayText->xPositionOscillator, displayText->xPosition);
  if (displayText->xPositionOscillator->enabled->boolValue) {
    ImGui::SameLine();
    CommonViews::HSpacing(5);
    CommonViews::OscillatorWindow(displayText->xPositionOscillator, displayText->xPosition);
    ImGui::NewLine();
  }
  
  ImGui::Text("Y Oscillator");
  ImGui::SameLine();
  CommonViews::OscillateButton("##yTextOsc", displayText->yPositionOscillator, displayText->yPosition);
  if (displayText->yPositionOscillator->enabled->boolValue) {
    ImGui::SameLine();
    CommonViews::HSpacing(5);
    CommonViews::OscillatorWindow(displayText->yPositionOscillator, displayText->yPosition);
  }
  
  // Font selection
  CommonViews::sSpacing();
  ImGui::Text("Fonts");
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


void TextEditorView::drawPositionSlider() {
  ImGuiExtensions::Slider2DFloat("", &displayText->xPosition->value, &displayText->yPosition->value, 0., 1., 0., 1., 0.5);
}
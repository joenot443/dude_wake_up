//
//  TextSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/17/23.
//

#include "TextSource.hpp"
#include "Console.hpp"
#include "CommonViews.hpp"
#include "NodeLayoutView.hpp"

void TextSource::setup() {
  fbo->allocate(settings->width->value, settings->height->value, GL_RGBA);
  font.load(displayText->font.path(), displayText->fontSize);
  fontPath = displayText->font.path();
}

void TextSource::saveFrame() {
  // Changed resolution
  if (static_cast<int>(fbo->getWidth()) != settings->width->intValue) {
    setup();
  }

  fbo->begin();
  
  bool shouldClear = false;
  
  // Clear if we've changed font size
  if (!font.isLoaded() ||
      font.getSize() != displayText->fontSize
      || fontPath != displayText->font.path()) {
    fontPath = displayText->font.path();
    font.load(displayText->font.path(), displayText->fontSize);
    shouldClear = true;
  }
  
  // Clear if our text has changed
  if (displayText->text != lastText) {
    lastText = displayText->text;
    shouldClear = true;
  }
  
  // Clear if we've moved
  if (
      xPos != settings->width->intValue * displayText->xPosition->value ||
      yPos != settings->height->intValue * (displayText->yPosition->value)) {
    xPos = settings->width->intValue * displayText->xPosition->value;
    yPos = settings->height->intValue * displayText->yPosition->value;
    shouldClear = true;
  }
  
  if (shouldClear) {
    ofClear(0, 0.);
  } else {
    ofSetColor(0, 0, 0, 0);
    ofDrawRectangle(0, 0, fbo->getWidth(), fbo->getHeight());
  }

  // Draw the text
  ofSetColor(displayText->color);
  font.drawString(displayText->text, xPos, settings->height->intValue - yPos);
  fbo->end();
}

json TextSource::serialize() {
  json j;
  j["videoSourceType"] = VideoSource_text;
  j["id"] = id;
  j["fontName"] = displayText->font.name;
  j["color"] = displayText->color.getHex();
  j["sourceName"] = sourceName;
  j["displayX"] = displayText->xPosition->value;
  j["displayY"] = displayText->yPosition->value;
  j["fontSize"] = displayText->fontSize;
  j["text"] = displayText->text;
  j["settings"] = settings->serialize();
  auto node = NodeLayoutView::getInstance()->nodeForShaderSourceId(id);
  if (node != nullptr) {
    j["x"] = node->position.x;
    j["y"] = node->position.y;
  }
  return j;
}

void TextSource::load(json j) {
  if (!j.is_object()) return;
  displayText->font.name = j["fontName"];
  font.load(displayText->font.path(), displayText->fontSize);
  displayText->color = ofColor::fromHex(j["color"]);

  displayText->xPosition->value = j["displayX"];
  displayText->yPosition->value = j["displayY"];

  displayText->fontSize = j["fontSize"];
  displayText->text = j["text"];
  
  settings->load(j["settings"]);
}

void TextSource::drawSettings() {
  CommonViews::H3Title("Text Editor");
  ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5, 5));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5., 5.));
  textEditorView.draw();
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
}

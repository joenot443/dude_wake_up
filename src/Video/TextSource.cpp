//
//  TextSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/17/23.
//

#include "TextSource.hpp"
#include "CommonViews.hpp"

void TextSource::setup() {
  fbo.allocate(settings.width->value, settings.height->value, GL_RGBA);
  font.load(displayText->font.path, displayText->fontSize);
  fontPath = displayText->font.path;
}

void TextSource::saveFrame() {
  // Changed resolution
  if (static_cast<int>(fbo.getWidth()) != settings.width->intValue) {
    setup();
  }

  fbo.begin();
  
  bool shouldClear = false;
  
  // Clear if we've changed font size
  if (font.getSize() != displayText->fontSize
      || fontPath != displayText->font.path) {
    fontPath = displayText->font.path;
    font.load(displayText->font.path, displayText->fontSize);
    shouldClear = true;
  }
  
  // Clear if our text has changed
  if (displayText->text != lastText) {
    lastText = displayText->text;
    shouldClear = true;
  }
  
  // Clear if we've moved
  if (
      xPos != settings.width->intValue * displayText->xPosition->value ||
      yPos != settings.height->intValue * (displayText->yPosition->value)) {
    xPos = settings.width->intValue * displayText->xPosition->value;
    yPos = settings.height->intValue * displayText->yPosition->value;
    shouldClear = true;
  }
  
  if (shouldClear) {
    ofClear(0, 255.);
  } else {
    ofSetColor(0, 0, 0, 0);
    ofDrawRectangle(0, 0, fbo.getWidth(), fbo.getHeight());
  }

  // Draw the text
  ofSetColor(displayText->color);
  font.drawString(displayText->text, xPos, settings.height->intValue - yPos);

  fbo.end();
  
  frameTexture = std::make_shared<ofTexture>(fbo.getTexture());
}

json TextSource::serialize() {
  json j;
  j["videoSourceType"] = VideoSource_text;
  j["id"] = id;
  j["fontPath"] = displayText->font.path;
  j["fontName"] = displayText->font.name;
  j["color"] = displayText->color.getHex();
  j["sourceName"] = sourceName;
  j["x"] = displayText->xPosition->value;
  j["y"] = displayText->yPosition->value;
  j["fontSize"] = displayText->fontSize;
  return j;
}

void TextSource::load(json j) {
  displayText->font.path = j["fontPath"];
  displayText->font.name = j["fontName"];

  displayText->color = ofColor::fromHex(j["color"]);

  displayText->xPosition->value = j["x"];
  displayText->yPosition->value = j["y"];

  displayText->fontSize = j["fontSize"];
}

void TextSource::drawSettings() {
  CommonViews::H3Title("Text Editor");
  ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5, 5));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5., 5.));
  textEditorView.draw();
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
}

//
//  TextSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/17/23.
//

#include "TextSource.hpp"
#include "Console.hpp"
#include "FontService.hpp"
#include "CommonViews.hpp"
#include "NodeLayoutView.hpp"
#include "LayoutStateService.hpp"

void TextSource::setup() {
  fbo->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y, GL_RGBA);
  optionalFbo->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y, GL_RGBA);
  
  font.load(displayText->font.path, displayText->fontSize);
  fontPath = displayText->font.path;
  strokeShader.load("shaders/Stroke");
  
  tempFbo.allocate(fbo->getWidth(), fbo->getHeight());
  if (displayText->font.name != FontService::getService()->fonts[displayText->fontSelector->intValue].name) {
    displayText->font = FontService::getService()->fonts[displayText->fontSelector->intValue];
    font.load(displayText->font.path, displayText->fontSize);
  }
}

void TextSource::saveFrame() {
  // Changed resolution
  if (static_cast<int>(fbo->getWidth()) != LayoutStateService::getService()->resolution.x) {
    setup();
  }

  tempFbo.begin();
  
  bool shouldClear = false;
  
  // Clear if we've changed font size
  if (!font.isLoaded() ||
      font.getSize() != displayText->fontSize
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
  ofSetColor(ofColor(255.0 * displayText->color->color->data()[0], 255.0 * displayText->color->color->data()[1], 255.0 * displayText->color->color->data()[2], 255.0 * displayText->color->color->data()[3]));
  
  font.drawString(displayText->text, xPos, settings->height->intValue - yPos);
  tempFbo.end();
  
  fbo->begin();
  ofClear(0, 0.);
  
  if (displayText->strokeEnabled->boolValue) {
    strokeShader.begin();
    strokeShader.setUniform4f("backgroundColor", displayText->color->color->data()[0], displayText->color->color->data()[1], displayText->color->color->data()[2], 0.0);
    strokeShader.setUniform4f("strokeColor", displayText->strokeColor->color->data()[0], displayText->strokeColor->color->data()[1], displayText->strokeColor->color->data()[2], displayText->strokeColor->color->data()[3]);
    strokeShader.setUniform1f("weight", displayText->strokeWeight->value);
    strokeShader.setUniform2f("dimensions", fbo->getWidth(), fbo->getHeight());
    strokeShader.setUniform1f("minThresh", 0.1);
    strokeShader.setUniform1f("maxThresh", 0.6);
    strokeShader.setUniformTexture("tex", tempFbo.getTexture(), 4);
    tempFbo.draw(0,0);
    strokeShader.end();
  } else {
    tempFbo.draw(0,0);
  }
  
  fbo->end();
}

json TextSource::serialize() {
  json j;
  j["videoSourceType"] = VideoSource_text;
  j["id"] = id;
  j["fontSize"] = displayText->fontSize;
  j["text"] = displayText->text;
  j["fontSelector"] = displayText->fontSelector->serialize();
  j["fontName"] = displayText->font.name;
  j["color"] = displayText->color->serialize();
  j["strokeColor"] = displayText->strokeColor->serialize();
  j["sourceName"] = sourceName;
  j["displayX"] = displayText->xPosition->value;
  j["displayY"] = displayText->yPosition->value;
  j["settings"] = settings->serialize();
  j["strokeEnabled"] = displayText->strokeEnabled->boolValue;
  auto node = NodeLayoutView::getInstance()->nodeForShaderSourceId(id);
  if (node != nullptr) {
    j["x"] = node->position.x;
    j["y"] = node->position.y;
  }
  return j;
}

void TextSource::load(json j) {
  if (!j.is_object()) return;
  
  if (j.contains("text")) {
    displayText->text = j["text"];
  }
  if (j.contains("fontSize")) {
    displayText->fontSize = j["fontSize"];
  }
  if (j.contains("fontSelector")) {
    displayText->fontSelector->load(j["fontSelector"]);
    displayText->font = FontService::getService()->fonts[displayText->fontSelector->intValue];
  }
  if (j.contains("fontName")) {
    displayText->font.name = j["fontName"];
  }
  if (j.contains("color")) {
    displayText->color->load(j["color"]);
  }
  if (j.contains("strokeColor")) {
    displayText->strokeColor->load(j["strokeColor"]);
  }
  if (j.contains("displayX")) {
    displayText->xPosition->value = j["displayX"];
  }
  if (j.contains("displayY")) {
    displayText->yPosition->value = j["displayY"];
  }
  if (j.contains("settings")) {
    settings->load(j["settings"]);
  }
  if (j.contains("strokeEnabled")) {
    displayText->strokeEnabled->setBoolValue(j["strokeEnabled"]);
  }
  if (j.contains("x")) {
    auto node = NodeLayoutView::getInstance()->nodeForShaderSourceId(id);
    if (node != nullptr) {
      node->position.x = j["x"];
    }
  }
  if (j.contains("y")) {
    auto node = NodeLayoutView::getInstance()->nodeForShaderSourceId(id);
    if (node != nullptr) {
      node->position.y = j["y"];
    }
  }
}

void TextSource::drawSettings() {
  CommonViews::H3Title("Text Editor");
  ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5, 5));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5., 5.));
  textEditorView.draw();
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
}

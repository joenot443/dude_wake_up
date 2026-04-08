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
  // Allocate FBOs with multisampling for better antialiasing
  ofFbo::Settings settings;
  settings.width = LayoutStateService::getService()->resolution.x;
  settings.height = LayoutStateService::getService()->resolution.y;
  settings.internalformat = GL_RGBA;
  settings.numSamples = max(4, static_cast<int>(displayText->textSmoothing->value)); // Use configurable MSAA, minimum 4
  settings.useDepth = false;
  settings.useStencil = false;
  settings.minFilter = GL_LINEAR;
  settings.maxFilter = GL_LINEAR;

  fbo->allocate(settings);
  optionalFbo->allocate(settings);
  tempFbo.allocate(settings);

  // Enable better font rendering
  font.setLetterSpacing(1.0);
  font.load(displayText->font.path, displayText->fontSize, true, true, true); // antialiased, full character set, make contours
  fontPath = displayText->font.path;
  strokeShader.load("shaders/Stroke");

  if (displayText->fontSelector->intValue >= 0 &&
      displayText->fontSelector->intValue < static_cast<int>(FontService::getService()->fonts.size()) &&
      displayText->font.name != FontService::getService()->fonts[displayText->fontSelector->intValue].name) {
    displayText->font = FontService::getService()->fonts[displayText->fontSelector->intValue];
    font.load(displayText->font.path, displayText->fontSize, true, true, true);
  }
}

void TextSource::saveFrame() {
  // Sync font from fontSelector (normally done by ImGui TextEditorView,
  // but NottawaApp doesn't run ImGui).
  if (displayText->fontSelector->intValue >= 0 &&
      displayText->fontSelector->intValue < static_cast<int>(FontService::getService()->fonts.size())) {
    auto& selectedFont = FontService::getService()->fonts[displayText->fontSelector->intValue];
    if (displayText->font.name != selectedFont.name) {
      displayText->font = selectedFont;
    }
  }

  // Reload font if size or path changed
  bool fontChanged = false;
  if (!font.isLoaded() ||
      font.getSize() != displayText->fontSize
      || fontPath != displayText->font.path) {
    fontPath = displayText->font.path;
    font.load(displayText->font.path, displayText->fontSize, true, true, true);
    fontChanged = true;
  }

  // Determine target FBO size
  int targetWidth = LayoutStateService::getService()->resolution.x;
  int targetHeight = LayoutStateService::getService()->resolution.y;

  if (displayText->cropToText->boolValue && font.isLoaded() && !displayText->text.empty()) {
    ofRectangle bounds = font.getStringBoundingBox(displayText->text, 0, 0);
    // Add padding for stroke weight and edge softness
    float padding = displayText->strokeEnabled->boolValue
      ? displayText->strokeWeight->value + displayText->edgeSoftness->value
      : displayText->edgeSoftness->value;
    int pad = static_cast<int>(ceil(padding)) + 2;
    targetWidth = std::max(1, static_cast<int>(ceil(bounds.width)) + pad * 2);
    targetHeight = std::max(1, static_cast<int>(ceil(bounds.height)) + pad * 2);
  }

  // Reallocate FBOs if size or antialiasing changed
  if (static_cast<int>(fbo->getWidth()) != targetWidth ||
      static_cast<int>(fbo->getHeight()) != targetHeight ||
      lastSampleCount != displayText->textSmoothing->value) {
    lastSampleCount = displayText->textSmoothing->value;

    ofFbo::Settings fboSettings;
    fboSettings.width = targetWidth;
    fboSettings.height = targetHeight;
    fboSettings.internalformat = GL_RGBA;
    fboSettings.numSamples = max(4, static_cast<int>(displayText->textSmoothing->value));
    fboSettings.useDepth = false;
    fboSettings.useStencil = false;
    fboSettings.minFilter = GL_LINEAR;
    fboSettings.maxFilter = GL_LINEAR;

    fbo->allocate(fboSettings);
    optionalFbo->allocate(fboSettings);
    tempFbo.allocate(fboSettings);

    font.setLetterSpacing(1.0);
    strokeShader.load("shaders/Stroke");
  }

  // Compute draw position
  float drawX, drawY;
  if (displayText->cropToText->boolValue && font.isLoaded() && !displayText->text.empty()) {
    ofRectangle bounds = font.getStringBoundingBox(displayText->text, 0, 0);
    float padding = displayText->strokeEnabled->boolValue
      ? displayText->strokeWeight->value + displayText->edgeSoftness->value
      : displayText->edgeSoftness->value;
    int pad = static_cast<int>(ceil(padding)) + 2;
    // Offset so text is centered in the tight FBO
    drawX = pad - bounds.x;
    drawY = pad - bounds.y;
  } else {
    drawX = fbo->getWidth() * displayText->xPosition->value;
    drawY = fbo->getHeight() * displayText->yPosition->value;
  }

  tempFbo.begin();

  // Enable smooth rendering
  ofEnableSmoothing();
  ofEnableAlphaBlending();
  ofEnableAntiAliasing();

  // Update cached position
  xPos = drawX;
  yPos = drawY;

  // Always clear — color, font, position, or text changes all require a fresh canvas
  if (displayText->backgroundEnabled->boolValue) {
    auto bg = displayText->backgroundColor->color->data();
    ofClear(bg[0] * 255.0, bg[1] * 255.0, bg[2] * 255.0, bg[3] * 255.0);
  } else {
    ofClear(0, 0.);
  }

  // Draw the text
  ofSetColor(ofColor(255.0 * displayText->color->color->data()[0], 255.0 * displayText->color->color->data()[1], 255.0 * displayText->color->color->data()[2], 255.0 * displayText->color->color->data()[3]));

  font.drawString(displayText->text, xPos, yPos);

  // Restore rendering settings
  ofDisableAntiAliasing();
  tempFbo.end();

  fbo->begin();
  if (displayText->backgroundEnabled->boolValue) {
    auto bg = displayText->backgroundColor->color->data();
    ofClear(bg[0] * 255.0, bg[1] * 255.0, bg[2] * 255.0, bg[3] * 255.0);
  } else {
    ofClear(0, 0.);
  }

  // Enable smooth rendering for FBO drawing
  ofEnableSmoothing();
  ofEnableAlphaBlending();

  if (displayText->strokeEnabled->boolValue) {
    strokeShader.begin();
    strokeShader.setUniform4f("backgroundColor", displayText->color->color->data()[0], displayText->color->color->data()[1], displayText->color->color->data()[2], 0.0);
    strokeShader.setUniform4f("strokeColor", displayText->strokeColor->color->data()[0], displayText->strokeColor->color->data()[1], displayText->strokeColor->color->data()[2], displayText->strokeColor->color->data()[3]);
    strokeShader.setUniform1f("weight", displayText->strokeWeight->value);
    strokeShader.setUniform1f("fineness", 64.0);
    strokeShader.setUniform1f("edgeSoftness", displayText->edgeSoftness->value);
    strokeShader.setUniform2f("dimensions", fbo->getWidth(), fbo->getHeight());
    strokeShader.setUniform1f("minThresh", 0.1);
    strokeShader.setUniform1f("maxThresh", 0.6);
    strokeShader.setUniformTexture("tex", tempFbo.getTexture(), 4);
    tempFbo.draw(0,0);
    strokeShader.end();
  } else {
    strokeShader.begin();
    strokeShader.setUniform4f("backgroundColor", displayText->color->color->data()[0], displayText->color->color->data()[1], displayText->color->color->data()[2], 0.0);
    strokeShader.setUniform4f("strokeColor", displayText->color->color->data()[0], displayText->color->color->data()[1], displayText->color->color->data()[2], 0.0);
    strokeShader.setUniform1f("weight", 0.5);
    strokeShader.setUniform1f("fineness", 64.0);
    strokeShader.setUniform1f("edgeSoftness", displayText->edgeSoftness->value);
    strokeShader.setUniform2f("dimensions", fbo->getWidth(), fbo->getHeight());
    strokeShader.setUniformTexture("tex", tempFbo.getTexture(), 4);
    tempFbo.draw(0,0);
    strokeShader.end();
  }

  ofDisableAlphaBlending();
  ofDisableSmoothing();
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
  j["backgroundColor"] = displayText->backgroundColor->serialize();
  j["backgroundEnabled"] = displayText->backgroundEnabled->boolValue;
  j["edgeSoftness"] = displayText->edgeSoftness->value;
  j["textSmoothing"] = displayText->textSmoothing->value;
  j["cropToText"] = displayText->cropToText->boolValue;
  auto node = NodeLayoutView::getInstance()->nodeForShaderSourceId(id);
  if (node != nullptr) {
    j["x"] = node->position.x;
    j["y"] = node->position.y;
  } else {
    j["x"] = origin.x;
    j["y"] = origin.y;
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
  if (j.contains("backgroundColor")) {
    displayText->backgroundColor->load(j["backgroundColor"]);
  }
  if (j.contains("backgroundEnabled")) {
    displayText->backgroundEnabled->setBoolValue(j["backgroundEnabled"]);
  }
  if (j.contains("edgeSoftness")) {
    displayText->edgeSoftness->value = j["edgeSoftness"];
  }
  if (j.contains("textSmoothing")) {
    displayText->textSmoothing->value = j["textSmoothing"];
  }
  if (j.contains("cropToText")) {
    displayText->cropToText->setBoolValue(j["cropToText"]);
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

void TextSource::centerText() {
  if (!font.isLoaded() || displayText->text.empty()) return;
  if (!fbo->isAllocated()) return;

  ofRectangle bounds = font.getStringBoundingBox(displayText->text, 0, 0);
  float fboW = fbo->getWidth();
  float fboH = fbo->getHeight();
  if (fboW <= 0 || fboH <= 0) return;

  displayText->xPosition->value = ((fboW - bounds.width) / 2.0f - bounds.x) / fboW;
  displayText->yPosition->value = ((fboH - bounds.height) / 2.0f - bounds.y) / fboH;
}

void TextSource::drawSettings() {
  CommonViews::H3Title("Text Editor");
  ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5, 5));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5., 5.));
  textEditorView.draw();
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
}

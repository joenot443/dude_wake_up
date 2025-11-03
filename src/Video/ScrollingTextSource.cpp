//
//  ScrollingTextSource.cpp
//  dude_wake_up
//
//  Created by Claude Code
//

#include "ScrollingTextSource.hpp"
#include "Console.hpp"
#include "FontService.hpp"
#include "CommonViews.hpp"
#include "NodeLayoutView.hpp"
#include "LayoutStateService.hpp"

void ScrollingTextSource::setup() {
  // Call parent setup
  TextSource::setup();

  // Reset scrolling state
  scrollPosition = 0.0;
  isWaiting = false;
  lastUpdateTime = ofGetElapsedTimef();
}

void ScrollingTextSource::saveFrame() {
  // Changed resolution
  if (static_cast<int>(fbo->getWidth()) != LayoutStateService::getService()->resolution.x) {
    setup();
  }

  tempFbo.begin();

  // Always clear
  ofClear(0, 255);
  ofClear(0, 0);

  // Reload font if changed
  if (!font.isLoaded() ||
      font.getSize() != displayText->fontSize
      || fontPath != displayText->font.path) {
    fontPath = displayText->font.path;
    font.load(displayText->font.path, displayText->fontSize);
  }

  // Scrolling logic
  double currentTime = ofGetElapsedTimef();
  double deltaTime = currentTime - lastUpdateTime;
  lastUpdateTime = currentTime;

  // Check if we're in delay period
  if (isWaiting) {
    if (currentTime - waitStartTime >= delay->value) {
      // Reset to start
      scrollPosition = 0.0;
      isWaiting = false;
    }
  } else {
    // Update scroll position based on speed and deltaTime
    float movement = speed->value * deltaTime;
    scrollPosition += movement;

    // Calculate text bounds
    ofRectangle textBounds = font.getStringBoundingBox(displayText->text, 0, 0);

    // Check if text has fully exited the frame
    bool hasExited = false;
    if (vertical->boolValue) {
      // Vertical scrolling
      float totalHeight = fbo->getHeight() + textBounds.height;
      if (scrollPosition >= totalHeight) {
        hasExited = true;
      }
    } else {
      // Horizontal scrolling
      float totalWidth = fbo->getWidth() + textBounds.width;
      if (scrollPosition >= totalWidth) {
        hasExited = true;
      }
    }

    if (hasExited) {
      isWaiting = true;
      waitStartTime = currentTime;
    }
  }

  // Clear if our full text has changed (user edited it)
  if (displayText->text != lastText) {
    lastText = displayText->text;
    scrollPosition = 0.0;
    isWaiting = false;
  }

  // Calculate draw position
  float drawX, drawY;
  ofRectangle textBounds = font.getStringBoundingBox(displayText->text, 0, 0);

  if (vertical->boolValue) {
    // Vertical scrolling
    drawX = fbo->getWidth() * displayText->xPosition->value;
    if (reverse->boolValue) {
      // Scroll down-to-up (start from bottom, move up)
      drawY = fbo->getHeight() + textBounds.height - scrollPosition;
    } else {
      // Scroll up-to-down (start from top, move down)
      drawY = -textBounds.height + scrollPosition;
    }
  } else {
    // Horizontal scrolling
    drawY = fbo->getHeight() * displayText->yPosition->value;
    if (reverse->boolValue) {
      // Scroll right-to-left (start from right, move left)
      drawX = fbo->getWidth() + textBounds.width - scrollPosition;
    } else {
      // Scroll left-to-right (start from left, move right)
      drawX = -textBounds.width + scrollPosition;
    }
  }

  // Draw the scrolling text
  ofSetColor(ofColor(255.0 * displayText->color->color->data()[0],
                     255.0 * displayText->color->color->data()[1],
                     255.0 * displayText->color->color->data()[2],
                     255.0 * displayText->color->color->data()[3]));

  font.drawString(displayText->text, drawX, drawY);
  tempFbo.end();

  fbo->begin();
  ofClear(0, 0.);

  if (displayText->strokeEnabled->boolValue) {
    strokeShader.begin();
    strokeShader.setUniform4f("backgroundColor", displayText->color->color->data()[0], displayText->color->color->data()[1], displayText->color->color->data()[2], 0.0);
    strokeShader.setUniform4f("strokeColor", displayText->strokeColor->color->data()[0], displayText->strokeColor->color->data()[1], displayText->strokeColor->color->data()[2], displayText->strokeColor->color->data()[3]);
    strokeShader.setUniform1f("weight", displayText->strokeWeight->value);
    strokeShader.setUniform1f("fineness", 64.0);
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

json ScrollingTextSource::serialize() {
  json j = TextSource::serialize();
  j["videoSourceType"] = VideoSource_scrollingText;
  j["speed"] = speed->value;
  j["vertical"] = vertical->boolValue;
  j["delay"] = delay->value;
  j["reverse"] = reverse->boolValue;
  return j;
}

void ScrollingTextSource::load(json j) {
  TextSource::load(j);

  if (j.contains("speed")) {
    speed->value = j["speed"];
  }
  if (j.contains("vertical")) {
    vertical->boolValue = j["vertical"];
  }
  if (j.contains("delay")) {
    delay->value = j["delay"];
  }
  if (j.contains("reverse")) {
    reverse->boolValue = j["reverse"];
  }
}

void ScrollingTextSource::drawSettings() {
  CommonViews::H3Title("Scrolling Text Editor");

  // Scrolling-specific settings
  CommonViews::ShaderParameter(speed, nullptr);
  CommonViews::ShaderCheckbox(vertical);
  CommonViews::ShaderCheckbox(reverse);
  CommonViews::ShaderParameter(delay, nullptr);

  ImGui::NewLine();

  // Draw the regular text editor
  ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5, 5));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5., 5.));
  textEditorView.draw();
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
}

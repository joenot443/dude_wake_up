//
//  TypewriterTextSource.cpp
//  dude_wake_up
//
//  Created by Claude Code
//

#include "TypewriterTextSource.hpp"
#include "Console.hpp"
#include "FontService.hpp"
#include "CommonViews.hpp"
#include "NodeLayoutView.hpp"
#include "LayoutStateService.hpp"

void TypewriterTextSource::setup() {
  // Call parent setup
  TextSource::setup();

  // Reset typewriter state
  currentCharIndex = 0;
  lastCharTime = ofGetElapsedTimef();
  isPaused = false;
  displayedText = "";
}

void TypewriterTextSource::saveFrame() {
  // Changed resolution
  if (static_cast<int>(fbo->getWidth()) != LayoutStateService::getService()->resolution.x) {
    setup();
  }

  tempFbo.begin();

  // Always clear for typewriter since we're redrawing each frame
  ofClear(0, 255);
  ofClear(0, 0);

  // Reload font if changed
  if (!font.isLoaded() ||
      font.getSize() != displayText->fontSize
      || fontPath != displayText->font.path) {
    fontPath = displayText->font.path;
    font.load(displayText->font.path, displayText->fontSize);
  }

  // Typewriter logic
  double currentTime = ofGetElapsedTimef();

  // Check if we're paused at the end
  if (isPaused) {
    if (currentTime - pauseStartTime >= hold->value) {
      // Reset to start
      currentCharIndex = 0;
      displayedText = "";
      isPaused = false;
      lastCharTime = currentTime;
    }
  } else {
    // Check if it's time to add the next character
    // Speed is characters per second, so delay is 1/speed
    float charDelay = 1.0f / speed->value;
    if (currentTime - lastCharTime >= charDelay) {
      if (currentCharIndex < displayText->text.length()) {
        displayedText += displayText->text[currentCharIndex];
        currentCharIndex++;
        lastCharTime = currentTime;
      } else {
        // Reached the end, start pause
        isPaused = true;
        pauseStartTime = currentTime;
      }
    }
  }

  // Clear if our full text has changed (user edited it)
  if (displayText->text != lastText) {
    lastText = displayText->text;
    currentCharIndex = 0;
    displayedText = "";
    isPaused = false;
    lastCharTime = currentTime;
  }

  // Update position
  xPos = fbo->getWidth() * displayText->xPosition->value;
  yPos = fbo->getHeight() * displayText->yPosition->value;

  // Draw the typewriter text (only what's been "typed" so far)
  ofSetColor(ofColor(255.0 * displayText->color->color->data()[0], 255.0 * displayText->color->color->data()[1], 255.0 * displayText->color->color->data()[2], 255.0 * displayText->color->color->data()[3]));

  // Flip y for OF style coordinates
  font.drawString(displayedText, xPos, yPos);
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

json TypewriterTextSource::serialize() {
  json j = TextSource::serialize();
  j["videoSourceType"] = VideoSource_typewriter;
  j["speed"] = speed->value;
  j["hold"] = hold->value;
  return j;
}

void TypewriterTextSource::load(json j) {
  TextSource::load(j);

  if (j.contains("speed")) {
    speed->value = j["speed"];
  }
  if (j.contains("hold")) {
    hold->value = j["hold"];
  }
}

void TypewriterTextSource::drawSettings() {
  CommonViews::H3Title("Typewriter Text Editor");

  // Typewriter-specific settings
  CommonViews::ShaderParameter(speed, nullptr);
  CommonViews::ShaderParameter(hold, nullptr);

  ImGui::NewLine();

  // Draw the regular text editor
  ImGui::PushStyleVar(ImGuiStyleVar_CellPadding, ImVec2(5, 5));
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(5., 5.));
  textEditorView.draw();
  ImGui::PopStyleVar();
  ImGui::PopStyleVar();
}

//
//  WindowsShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef WindowsShader_hpp
#define WindowsShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "TimeService.hpp"
#include "WaveformOscillator.hpp"
#include "FontService.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct WindowsSettings: public ShaderSettings {
  std::shared_ptr<Parameter> windowTitle;
  std::shared_ptr<Parameter> buttonText;
  std::shared_ptr<Parameter> buttonText2;
  std::shared_ptr<Parameter> buttonText3;
  std::shared_ptr<Parameter> floating;
  std::shared_ptr<Parameter> scale;
  std::shared_ptr<Parameter> xPosition;
  std::shared_ptr<Parameter> yPosition;
  std::shared_ptr<WaveformOscillator> scaleOscillator;
  std::shared_ptr<WaveformOscillator> xPositionOscillator;
  std::shared_ptr<WaveformOscillator> yPositionOscillator;

  WindowsSettings(std::string shaderId, json j) :
  windowTitle(std::make_shared<Parameter>("Window Title", ParameterType_Text)),
  buttonText(std::make_shared<Parameter>("Button Text", ParameterType_Text)),
  buttonText2(std::make_shared<Parameter>("Button Text 2", ParameterType_Text)),
  buttonText3(std::make_shared<Parameter>("Button Text 3", ParameterType_Text)),
  floating(std::make_shared<Parameter>("Floating", 0.0, ParameterType_Bool)),
  scale(std::make_shared<Parameter>("Scale", 1.0, 0.1, 2.0)),
  xPosition(std::make_shared<Parameter>("X Position", 0.5, 0.0, 1.0)),
  yPosition(std::make_shared<Parameter>("Y Position", 0.5, 0.0, 1.0)),
  scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
  xPositionOscillator(std::make_shared<WaveformOscillator>(xPosition)),
  yPositionOscillator(std::make_shared<WaveformOscillator>(yPosition)),
  ShaderSettings(shaderId, j, "Windows") {
    windowTitle->stringValue = "About Nottawa";
    buttonText->stringValue = "OK";
    buttonText2->stringValue = "";
    buttonText3->stringValue = "";
    parameters = { windowTitle, buttonText, buttonText2, buttonText3, floating, scale, xPosition, yPosition };
    oscillators = { scaleOscillator, xPositionOscillator, yPositionOscillator };
    load(j);
    registerParameters();
  };
};

struct WindowsShader: Shader {
  WindowsSettings *settings;
  ofTrueTypeFont titleFont;
  ofTrueTypeFont buttonFont;

  WindowsShader(WindowsSettings *settings) : settings(settings), Shader(settings) {};

  void setup() override {
    std::string fontPath = FontService::getService()->fontPathByName("windows");
    if (!fontPath.empty()) {
      titleFont.load(fontPath, 16, true, true);
      buttonFont.load(fontPath, 14, true, true);
    }
  }

  // Win2K 4-level raised bevel: white → light gray → dark gray → black
  void drawBeveledRect(float x, float y, float w, float h, bool raised) {
    // Face
    ofSetColor(192, 192, 192);
    ofDrawRectangle(x, y, w, h);

    if (raised) {
      // Outer highlight
      ofSetColor(255, 255, 255);
      ofDrawLine(x, y, x + w - 1, y);
      ofDrawLine(x, y, x, y + h - 1);
      // Inner highlight
      ofSetColor(223, 223, 223);
      ofDrawLine(x + 1, y + 1, x + w - 2, y + 1);
      ofDrawLine(x + 1, y + 1, x + 1, y + h - 2);
      // Inner shadow
      ofSetColor(128, 128, 128);
      ofDrawLine(x + 1, y + h - 2, x + w - 2, y + h - 2);
      ofDrawLine(x + w - 2, y + 1, x + w - 2, y + h - 2);
      // Outer shadow
      ofSetColor(0, 0, 0);
      ofDrawLine(x, y + h - 1, x + w, y + h - 1);
      ofDrawLine(x + w - 1, y, x + w - 1, y + h);
    } else {
      // Outer shadow (top-left)
      ofSetColor(128, 128, 128);
      ofDrawLine(x, y, x + w - 1, y);
      ofDrawLine(x, y, x, y + h - 1);
      // Inner shadow
      ofSetColor(0, 0, 0);
      ofDrawLine(x + 1, y + 1, x + w - 2, y + 1);
      ofDrawLine(x + 1, y + 1, x + 1, y + h - 2);
      // Inner highlight (bottom-right)
      ofSetColor(223, 223, 223);
      ofDrawLine(x + 1, y + h - 2, x + w - 2, y + h - 2);
      ofDrawLine(x + w - 2, y + 1, x + w - 2, y + h - 2);
      // Outer highlight
      ofSetColor(255, 255, 255);
      ofDrawLine(x, y + h - 1, x + w, y + h - 1);
      ofDrawLine(x + w - 1, y, x + w - 1, y + h);
    }
  }

  // Sunken inset border (Win2K content well)
  void drawSunkenRect(float x, float y, float w, float h) {
    // Dark top-left
    ofSetColor(128, 128, 128);
    ofDrawLine(x, y, x + w - 1, y);
    ofDrawLine(x, y, x, y + h - 1);
    // Darker inner top-left
    ofSetColor(0, 0, 0);
    ofDrawLine(x + 1, y + 1, x + w - 2, y + 1);
    ofDrawLine(x + 1, y + 1, x + 1, y + h - 2);
    // Light bottom-right
    ofSetColor(255, 255, 255);
    ofDrawLine(x, y + h - 1, x + w, y + h - 1);
    ofDrawLine(x + w - 1, y, x + w - 1, y + h);
    // Inner light bottom-right
    ofSetColor(223, 223, 223);
    ofDrawLine(x + 1, y + h - 2, x + w - 2, y + h - 2);
    ofDrawLine(x + w - 2, y + 1, x + w - 2, y + h - 2);
  }

  // Etched horizontal line separator
  void drawEtchedLine(float x1, float x2, float y) {
    ofSetColor(128, 128, 128);
    ofDrawLine(x1, y, x2, y);
    ofSetColor(255, 255, 255);
    ofDrawLine(x1, y + 1, x2, y + 1);
  }

  // Dotted focus rectangle
  void drawFocusRect(float x, float y, float w, float h) {
    ofSetColor(0, 0, 0);
    for (float px = x; px < x + w; px += 2) {
      ofDrawRectangle(px, y, 1, 1);
      ofDrawRectangle(px, y + h - 1, 1, 1);
    }
    for (float py = y; py < y + h; py += 2) {
      ofDrawRectangle(x, py, 1, 1);
      ofDrawRectangle(x + w - 1, py, 1, 1);
    }
  }

  // Small title bar button (minimize, maximize, close)
  void drawTitleButton(float x, float y, float size) {
    drawBeveledRect(x, y, size, size, true);
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    ofClear(0, 0, 0, 0);

    float canvasW = canvas->getWidth();
    float canvasH = canvas->getHeight();

    // Win2K chrome dimensions
    float titleBarHeight = 30;
    float borderSize = 4;
    float bottomBarHeight = 54;
    float titleBtnSize = 20;
    float titleBtnSpacing = 2;
    float sunkenInset = 2; // sunken border around content
    float chromeH = borderSize * 2 + titleBarHeight + bottomBarHeight + sunkenInset * 2;

    // Size the window to wrap the input frame's aspect ratio
    float frameAspect = frame->getWidth() / (float)frame->getHeight();
    float scale = settings->scale->value;

    float availH = canvasH - chromeH;
    float availW = canvasW - borderSize * 2 - sunkenInset * 2;
    float contentW, contentH;
    if (frameAspect > availW / availH) {
      contentW = availW * scale;
      contentH = contentW / frameAspect;
    } else {
      contentH = availH * scale;
      contentW = contentH * frameAspect;
    }

    float w = contentW + borderSize * 2 + sunkenInset * 2;
    float h = contentH + chromeH;

    float offsetX, offsetY;
    if (settings->floating->boolValue) {
      offsetX = (settings->xPosition->value * canvasW) - (w / 2);
      offsetY = (settings->yPosition->value * canvasH) - (h / 2);
    } else {
      offsetX = (canvasW - w) / 2;
      offsetY = (canvasH - h) / 2;
    }

    // ---- Window face ----
    ofSetColor(192, 192, 192);
    ofDrawRectangle(offsetX, offsetY, w, h);

    // ---- Outer 4-level border (Win2K style) ----
    // Outermost highlight
    ofSetColor(255, 255, 255);
    ofDrawLine(offsetX, offsetY, offsetX + w - 1, offsetY);
    ofDrawLine(offsetX, offsetY, offsetX, offsetY + h - 1);
    // Inner highlight
    ofSetColor(223, 223, 223);
    ofDrawLine(offsetX + 1, offsetY + 1, offsetX + w - 2, offsetY + 1);
    ofDrawLine(offsetX + 1, offsetY + 1, offsetX + 1, offsetY + h - 2);
    // Inner shadow
    ofSetColor(128, 128, 128);
    ofDrawLine(offsetX + 1, offsetY + h - 2, offsetX + w - 2, offsetY + h - 2);
    ofDrawLine(offsetX + w - 2, offsetY + 1, offsetX + w - 2, offsetY + h - 2);
    // Outermost shadow
    ofSetColor(0, 0, 0);
    ofDrawLine(offsetX, offsetY + h - 1, offsetX + w, offsetY + h - 1);
    ofDrawLine(offsetX + w - 1, offsetY, offsetX + w - 1, offsetY + h);

    // ---- Title bar gradient (Win2K: navy → medium blue) ----
    float tbX = offsetX + borderSize;
    float tbY = offsetY + borderSize;
    float tbW = w - borderSize * 2;

    ofMesh gradient;
    gradient.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
    gradient.addVertex(glm::vec3(tbX, tbY, 0));
    gradient.addColor(ofColor(0, 0, 128));
    gradient.addVertex(glm::vec3(tbX, tbY + titleBarHeight, 0));
    gradient.addColor(ofColor(0, 0, 128));
    gradient.addVertex(glm::vec3(tbX + tbW, tbY, 0));
    gradient.addColor(ofColor(16, 132, 208));
    gradient.addVertex(glm::vec3(tbX + tbW, tbY + titleBarHeight, 0));
    gradient.addColor(ofColor(16, 132, 208));
    gradient.draw();

    // ---- System menu icon (small placeholder square) ----
    float iconSize = 14;
    float iconX = tbX + 4;
    float iconY = tbY + (titleBarHeight - iconSize) / 2;
    ofSetColor(192, 192, 192);
    ofDrawRectangle(iconX, iconY, iconSize, iconSize);
    ofSetColor(0, 0, 128);
    ofDrawRectangle(iconX + 2, iconY + 2, iconSize - 4, iconSize - 4);

    // ---- Title text (bold, white) ----
    if (titleFont.isLoaded()) {
      ofSetColor(255, 255, 255);
      auto titleBbox = titleFont.getStringBoundingBox(settings->windowTitle->stringValue, 0, 0);
      float titleTextY = tbY + (titleBarHeight - titleBbox.height) / 2 - titleBbox.y;
      titleFont.drawString(settings->windowTitle->stringValue,
                           iconX + iconSize + 6,
                           titleTextY);
    }

    // ---- Title bar buttons: minimize, maximize, close ----
    float btnY = tbY + (titleBarHeight - titleBtnSize) / 2;
    float closeX = tbX + tbW - titleBtnSize - 2;
    float maxX = closeX - titleBtnSize - titleBtnSpacing;
    float minX = maxX - titleBtnSize - titleBtnSpacing;

    // Minimize button _
    drawTitleButton(minX, btnY, titleBtnSize);
    ofSetColor(0, 0, 0);
    ofSetLineWidth(1);
    ofDrawLine(minX + 4, btnY + titleBtnSize - 5, minX + titleBtnSize - 4, btnY + titleBtnSize - 5);

    // Maximize button □
    drawTitleButton(maxX, btnY, titleBtnSize);
    ofSetColor(0, 0, 0);
    ofNoFill();
    ofDrawRectangle(maxX + 3, btnY + 3, titleBtnSize - 7, titleBtnSize - 7);
    ofDrawLine(maxX + 3, btnY + 5, maxX + titleBtnSize - 4, btnY + 5); // thicker top bar
    ofFill();

    // Close button X
    drawTitleButton(closeX, btnY, titleBtnSize);
    ofSetColor(0, 0, 0);
    ofDrawLine(closeX + 4, btnY + 4, closeX + titleBtnSize - 5, btnY + titleBtnSize - 5);
    ofDrawLine(closeX + titleBtnSize - 5, btnY + 4, closeX + 4, btnY + titleBtnSize - 5);
    // Thicken the X
    ofDrawLine(closeX + 5, btnY + 4, closeX + titleBtnSize - 4, btnY + titleBtnSize - 5);
    ofDrawLine(closeX + titleBtnSize - 4, btnY + 4, closeX + 5, btnY + titleBtnSize - 5);

    // ---- Content area with sunken border ----
    float sunkenX = offsetX + borderSize;
    float sunkenY = offsetY + borderSize + titleBarHeight;
    float sunkenW = w - borderSize * 2;
    float sunkenH = contentH + sunkenInset * 2;

    // White content background (inside the sunken border)
    ofSetColor(255, 255, 255);
    ofDrawRectangle(sunkenX + sunkenInset, sunkenY + sunkenInset, contentW, contentH);

    // Draw the sunken inset border
    drawSunkenRect(sunkenX, sunkenY, sunkenW, sunkenH);

    // Draw input frame filling the content area
    ofSetColor(255, 255, 255);
    frame->draw(sunkenX + sunkenInset, sunkenY + sunkenInset, contentW, contentH);

    // ---- Etched separator line between content and button bar ----
    float sepY = sunkenY + sunkenH + 4;
    drawEtchedLine(offsetX + borderSize + 6, offsetX + w - borderSize - 6, sepY);

    // ---- Bottom button bar ----
    float bottomY = offsetY + h - borderSize - bottomBarHeight;

    // Buttons
    std::vector<std::string> buttonTexts;
    if (!settings->buttonText->stringValue.empty()) buttonTexts.push_back(settings->buttonText->stringValue);
    if (!settings->buttonText2->stringValue.empty()) buttonTexts.push_back(settings->buttonText2->stringValue);
    if (!settings->buttonText3->stringValue.empty()) buttonTexts.push_back(settings->buttonText3->stringValue);

    if (buttonTexts.size() > 0 && buttonFont.isLoaded()) {
      float buttonW = 95;
      float buttonH = 30;
      float buttonSpacing = 6;
      float totalButtonsWidth = buttonTexts.size() * buttonW + (buttonTexts.size() - 1) * buttonSpacing;
      float startX = offsetX + w - borderSize - 10 - totalButtonsWidth; // Right-aligned (Win2K style)

      for (int i = 0; i < (int)buttonTexts.size(); i++) {
        float bx = startX + i * (buttonW + buttonSpacing);
        float by = bottomY + (bottomBarHeight - buttonH) / 2;

        drawBeveledRect(bx, by, buttonW, buttonH, true);

        // Dotted focus rect on first button (default button)
        if (i == 0) {
          drawFocusRect(bx + 4, by + 4, buttonW - 8, buttonH - 8);
        }

        ofSetColor(0, 0, 0);
        auto bbox = buttonFont.getStringBoundingBox(buttonTexts[i], 0, 0);
        float textX = bx + (buttonW - bbox.width) / 2 - bbox.x;
        float textY = by + (buttonH - bbox.height) / 2 - bbox.y;
        buttonFont.drawString(buttonTexts[i], textX, textY);
      }
    }

    canvas->end();
  }

  void clear() override {

  }

  int inputCount() override {
    return 1;
  }

  ShaderType type() override {
    return ShaderTypeWindows;
  }

  void drawSettings() override {
    CommonViews::H3Title("Windows");

    // Editable window title
    char titleBuffer[256];
    strncpy(titleBuffer, settings->windowTitle->stringValue.c_str(), 255);
    titleBuffer[255] = '\0';
    if (ImGui::InputText("Window Title", titleBuffer, 256)) {
      settings->windowTitle->stringValue = std::string(titleBuffer);
    }

    // Editable button texts
    char buttonBuffer[256];
    strncpy(buttonBuffer, settings->buttonText->stringValue.c_str(), 255);
    buttonBuffer[255] = '\0';
    if (ImGui::InputText("Button Text", buttonBuffer, 256)) {
      settings->buttonText->stringValue = std::string(buttonBuffer);
    }

    char buttonBuffer2[256];
    strncpy(buttonBuffer2, settings->buttonText2->stringValue.c_str(), 255);
    buttonBuffer2[255] = '\0';
    if (ImGui::InputText("Button Text 2", buttonBuffer2, 256)) {
      settings->buttonText2->stringValue = std::string(buttonBuffer2);
    }

    char buttonBuffer3[256];
    strncpy(buttonBuffer3, settings->buttonText3->stringValue.c_str(), 255);
    buttonBuffer3[255] = '\0';
    if (ImGui::InputText("Button Text 3", buttonBuffer3, 256)) {
      settings->buttonText3->stringValue = std::string(buttonBuffer3);
    }

    // Floating mode toggle
    CommonViews::ShaderCheckbox(settings->floating);

    // Show position and scale controls when floating
    if (settings->floating->boolValue) {
      CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
      CommonViews::MultiSlider("Position", settings->shaderId, settings->xPosition, settings->yPosition,
                               settings->xPositionOscillator, settings->yPositionOscillator, 0.5625);
    }
  }
};

#endif /* WindowsShader_hpp */

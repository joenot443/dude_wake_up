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
  std::shared_ptr<Parameter> contentScale;
  std::shared_ptr<Parameter> floating;
  std::shared_ptr<Parameter> scale;
  std::shared_ptr<Parameter> xPosition;
  std::shared_ptr<Parameter> yPosition;
  std::shared_ptr<WaveformOscillator> scaleOscillator;
  std::shared_ptr<WaveformOscillator> xPositionOscillator;
  std::shared_ptr<WaveformOscillator> yPositionOscillator;

  WindowsSettings(std::string shaderId, json j) :
  windowTitle(std::make_shared<Parameter>("Window Title")),
  buttonText(std::make_shared<Parameter>("Button Text")),
  buttonText2(std::make_shared<Parameter>("Button Text 2")),
  buttonText3(std::make_shared<Parameter>("Button Text 3")),
  contentScale(std::make_shared<Parameter>("Content Scale", 0.8, 0.5, 1.0)),
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
    parameters = { windowTitle, buttonText, buttonText2, buttonText3, contentScale, floating, scale, xPosition, yPosition };
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
    std::string verdanaPath = FontService::getService()->fontPathByName("windows");
    if (!verdanaPath.empty()) {
      titleFont.load(verdanaPath, 11, true, true);
      buttonFont.load(verdanaPath, 11, true, true);
    }
  }

  void drawBeveledRect(float x, float y, float w, float h, bool raised) {
    // Main face
    ofSetColor(192, 192, 192);
    ofDrawRectangle(x, y, w, h);

    // 3D beveled edges
    if (raised) {
      // Light top and left edges
      ofSetColor(255, 255, 255);
      ofDrawLine(x, y, x + w, y); // top
      ofDrawLine(x, y, x, y + h); // left
      ofSetColor(223, 223, 223);
      ofDrawLine(x + 1, y + 1, x + w - 1, y + 1); // top inner
      ofDrawLine(x + 1, y + 1, x + 1, y + h - 1); // left inner

      // Dark bottom and right edges
      ofSetColor(64, 64, 64);
      ofDrawLine(x, y + h, x + w, y + h); // bottom
      ofDrawLine(x + w, y, x + w, y + h); // right
      ofSetColor(128, 128, 128);
      ofDrawLine(x + 1, y + h - 1, x + w - 1, y + h - 1); // bottom inner
      ofDrawLine(x + w - 1, y + 1, x + w - 1, y + h - 1); // right inner
    } else {
      // Inverted for pressed buttons
      ofSetColor(64, 64, 64);
      ofDrawLine(x, y, x + w, y);
      ofDrawLine(x, y, x, y + h);
      ofSetColor(128, 128, 128);
      ofDrawLine(x + 1, y + 1, x + w - 1, y + 1);
      ofDrawLine(x + 1, y + 1, x + 1, y + h - 1);

      ofSetColor(255, 255, 255);
      ofDrawLine(x, y + h, x + w, y + h);
      ofDrawLine(x + w, y, x + w, y + h);
      ofSetColor(223, 223, 223);
      ofDrawLine(x + 1, y + h - 1, x + w - 1, y + h - 1);
      ofDrawLine(x + w - 1, y + 1, x + w - 1, y + h - 1);
    }
  }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    canvas->begin();
    ofClear(0, 0, 0, 0);

    float canvasW = canvas->getWidth();
    float canvasH = canvas->getHeight();

    // Calculate window dimensions and position based on floating mode
    float w, h, offsetX, offsetY;

    if (settings->floating->boolValue) {
      // Floating mode: use scale and position parameters
      w = canvasW * settings->scale->value;
      h = canvasH * settings->scale->value;
      offsetX = (settings->xPosition->value * canvasW) - (w / 2);
      offsetY = (settings->yPosition->value * canvasH) - (h / 2);
    } else {
      // Fixed mode: fill entire canvas
      w = canvasW;
      h = canvasH;
      offsetX = 0;
      offsetY = 0;
    }

    // Window dimensions
    float titleBarHeight = 28;
    float borderSize = 4;
    float bottomBarHeight = 50;
    float closeButtonSize = 16;

    // Outer window border (3D frame)
    ofSetColor(192, 192, 192);
    ofDrawRectangle(offsetX, offsetY, w, h);

    // Title bar gradient (Windows blue)
    ofMesh gradient;
    gradient.setMode(OF_PRIMITIVE_TRIANGLE_STRIP);
    gradient.addVertex(glm::vec3(offsetX + borderSize, offsetY + borderSize, 0));
    gradient.addColor(ofColor(0, 84, 227));
    gradient.addVertex(glm::vec3(offsetX + borderSize, offsetY + borderSize + titleBarHeight, 0));
    gradient.addColor(ofColor(0, 84, 227));
    gradient.addVertex(glm::vec3(offsetX + w - borderSize, offsetY + borderSize, 0));
    gradient.addColor(ofColor(10, 130, 255));
    gradient.addVertex(glm::vec3(offsetX + w - borderSize, offsetY + borderSize + titleBarHeight, 0));
    gradient.addColor(ofColor(10, 130, 255));
    gradient.draw();

    // Title text
    if (titleFont.isLoaded()) {
      ofSetColor(255, 255, 255);
      titleFont.drawString(settings->windowTitle->stringValue, offsetX + borderSize + 8, offsetY + borderSize + 18);
    }

    // Close button (X)
    float closeX = offsetX + w - borderSize - closeButtonSize - 6;
    float closeY = offsetY + borderSize + 6;
    drawBeveledRect(closeX, closeY, closeButtonSize, closeButtonSize, true);
    ofSetColor(0, 0, 0);
    ofDrawLine(closeX + 4, closeY + 4, closeX + closeButtonSize - 4, closeY + closeButtonSize - 4);
    ofDrawLine(closeX + closeButtonSize - 4, closeY + 4, closeX + 4, closeY + closeButtonSize - 4);

    // Content area background
    float contentX = offsetX + borderSize;
    float contentY = offsetY + borderSize + titleBarHeight;
    float contentW = w - borderSize * 2;
    float contentH = h - borderSize * 2 - titleBarHeight - bottomBarHeight;

    ofSetColor(255, 255, 255);
    ofDrawRectangle(contentX, contentY, contentW, contentH);

    // Draw input frame scaled inside content area
    float scale = settings->contentScale->value;
    float scaledW = contentW * scale;
    float scaledH = contentH * scale;
    float frameX = contentX + (contentW - scaledW) / 2;
    float frameY = contentY + (contentH - scaledH) / 2;

    ofSetColor(255, 255, 255);
    frame->draw(frameX, frameY, scaledW, scaledH);

    // Bottom bar
    float bottomY = offsetY + h - borderSize - bottomBarHeight;
    ofSetColor(192, 192, 192);
    ofDrawRectangle(offsetX + borderSize, bottomY, w - borderSize * 2, bottomBarHeight);

    // Buttons - collect non-empty button texts
    std::vector<std::string> buttonTexts;
    if (!settings->buttonText->stringValue.empty()) buttonTexts.push_back(settings->buttonText->stringValue);
    if (!settings->buttonText2->stringValue.empty()) buttonTexts.push_back(settings->buttonText2->stringValue);
    if (!settings->buttonText3->stringValue.empty()) buttonTexts.push_back(settings->buttonText3->stringValue);

    if (buttonTexts.size() > 0 && buttonFont.isLoaded()) {
      float buttonW = 80;
      float buttonH = 24;
      float buttonSpacing = 10;
      float totalButtonsWidth = buttonTexts.size() * buttonW + (buttonTexts.size() - 1) * buttonSpacing;
      float startX = offsetX + (w - totalButtonsWidth) / 2;

      for (int i = 0; i < buttonTexts.size(); i++) {
        float buttonX = startX + i * (buttonW + buttonSpacing);
        float buttonY = bottomY + (bottomBarHeight - buttonH) / 2;

        drawBeveledRect(buttonX, buttonY, buttonW, buttonH, true);

        ofSetColor(0, 0, 0);
        auto bbox = buttonFont.getStringBoundingBox(buttonTexts[i], 0, 0);
        float textX = buttonX + (buttonW - bbox.width) / 2;
        float textY = buttonY + (buttonH + bbox.height) / 2 - 2;
        buttonFont.drawString(buttonTexts[i], textX, textY);
      }
    }

    // Outer 3D border
    // Outer highlight
    ofSetColor(223, 223, 223);
    ofDrawLine(offsetX, offsetY, offsetX + w, offsetY); // top
    ofDrawLine(offsetX, offsetY, offsetX, offsetY + h); // left

    // Outer shadow
    ofSetColor(64, 64, 64);
    ofDrawLine(offsetX, offsetY + h - 1, offsetX + w, offsetY + h - 1); // bottom
    ofDrawLine(offsetX + w - 1, offsetY, offsetX + w - 1, offsetY + h); // right

    // Inner shadow (just inside the window)
    ofSetColor(128, 128, 128);
    ofDrawLine(offsetX + borderSize - 1, offsetY + borderSize - 1, offsetX + w - borderSize + 1, offsetY + borderSize - 1); // top
    ofDrawLine(offsetX + borderSize - 1, offsetY + borderSize - 1, offsetX + borderSize - 1, offsetY + h - borderSize + 1); // left

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

    // Content scale slider
    CommonViews::ShaderParameter(settings->contentScale, nullptr);

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

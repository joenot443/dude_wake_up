#ifndef TransformShader_hpp
#define TransformShader_hpp

#include "ofMain.h"
#include "ShaderConfigSelectionView.hpp"
#include "ImGuiExtensions.hpp"
#include "Shader.hpp"
#include "ofxImGui.h"
#include <stdio.h>

// Transform

struct TransformSettings : public ShaderSettings {
public:
    std::shared_ptr<Parameter> minX;
    std::shared_ptr<Parameter> maxX;
    std::shared_ptr<Parameter> minY;
    std::shared_ptr<Parameter> maxY;
    std::shared_ptr<Parameter> rotate;
  
    std::shared_ptr<Parameter> scale;

    std::shared_ptr<Parameter> autoRotate;

    std::shared_ptr<Oscillator> minXOscillator;
    std::shared_ptr<Oscillator> maxXOscillator;
    std::shared_ptr<Oscillator> minYOscillator;
    std::shared_ptr<Oscillator> maxYOscillator;
    std::shared_ptr<Oscillator> rotateOscillator;
    std::shared_ptr<Oscillator> scaleOscillator;
    std::shared_ptr<Oscillator> autoRotateOscillator;

    TransformSettings(std::string shaderId, json j, std::string name)
        : minX(std::make_shared<Parameter>("minX", 0.1, 0.0, 1.0)),
          maxX(std::make_shared<Parameter>("maxX", 0.8, 0.0, 1.0)),
          minY(std::make_shared<Parameter>("minY", 0.1, 0.0, 1.0)),
          maxY(std::make_shared<Parameter>("maxY", 0.8, 0.0, 1.0)),
          rotate(std::make_shared<Parameter>("rotate", 0.0, 0.0, 360.0)),
          autoRotate(std::make_shared<Parameter>("autoRotate", 0.0, 0.0, 2.0)),
          scale(std::make_shared<Parameter>("scale", 1.0, 0.0, 5.0)),
          minXOscillator(std::make_shared<WaveformOscillator>(minX)),
          maxXOscillator(std::make_shared<WaveformOscillator>(maxX)),
          minYOscillator(std::make_shared<WaveformOscillator>(minY)),
          maxYOscillator(std::make_shared<WaveformOscillator>(maxY)),
          rotateOscillator(std::make_shared<WaveformOscillator>(rotate)),
          scaleOscillator(std::make_shared<WaveformOscillator>(scale)),
          autoRotateOscillator(std::make_shared<WaveformOscillator>(autoRotate)),
          ShaderSettings(shaderId, j, name) {
        parameters = {minX, maxX, minY, maxY, rotate, autoRotate, scale};
        oscillators = {minXOscillator, maxXOscillator, minYOscillator, maxYOscillator, rotateOscillator, autoRotateOscillator};
        load(j);
        registerParameters();
    }
};

struct TransformShader : public Shader {
public:
    ofShader shader;
    TransformSettings *settings;
    float autoRotateAmount;

    TransformShader(TransformSettings *settings)
        : settings(settings), Shader(settings){};

    void setup() override {}

    ShaderType type() override { return ShaderTypeTransform; }

  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
      canvas->begin();

      auto texture = frame->getTexture();
      float frameWidth = frame->getWidth();
      float frameHeight = frame->getHeight();

      // Define cropping region based on parameters
      float cropX = settings->minX->value * frameWidth;
      float cropY = settings->minY->value * frameHeight;
      float cropWidth = (settings->maxX->value - settings->minX->value) * frameWidth;
      float cropHeight = (settings->maxY->value - settings->minY->value) * frameHeight;
      float scaleX = cropWidth * settings->scale->value;
      float scaleY = cropHeight * settings->scale->value;

      ofClear(0, 0, 0, 0);
      ofPushMatrix();

      // Calculate the position to center the scaled texture
      float scaledWidth = scaleX;
      float scaledHeight = scaleY;
      float translateX = (canvas->getWidth() - scaledWidth) / 2;
      float translateY = (canvas->getHeight() - scaledHeight) / 2;

      // Translate to center the scaled image on the canvas
      ofTranslate(translateX, translateY);

      // Draw the cropped and scaled texture
      texture.drawSubsection(0, 0, scaledWidth, scaledHeight, cropX, cropY, cropWidth, cropHeight);

      ofPopMatrix();

      shader.end();
      canvas->end();
  }


    void drawSettings() override {
      CommonViews::H4Title("Area");
      auto frame = parentFrame();
      auto cursorPos = ImGui::GetCursorPos();
      if (frame != nullptr) {
        ofxImGui::AddImage(frame->getTexture(), ofVec2f(256.0, 144.0));
      }
      ImGui::SetCursorPos(cursorPos);
      
      CommonViews::AreaSlider(shaderId, settings->minX, settings->maxX, settings->minY, settings->maxY, settings->minXOscillator, settings->maxXOscillator, settings->minYOscillator, settings->maxYOscillator);

        CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
        CommonViews::ShaderParameter(settings->autoRotate, settings->autoRotateOscillator);
        CommonViews::ShaderParameter(settings->rotate, settings->rotateOscillator);
    }

    void clear() override {}
};

#endif

//
//  MirrorShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef MirrorShader_hpp
#define MirrorShader_hpp

#include "ofMain.h"

#include "Shader.hpp"
#include "WaveformOscillator.hpp"
#include "ofxImGui.h"
#include <stdio.h>

// Mirror

struct MirrorSettings : public ShaderSettings {
  std::string shaderId;

  std::shared_ptr<Parameter> mirrorXEnabled;
  std::shared_ptr<Parameter> mirrorYEnabled;

  std::shared_ptr<Parameter> xOffset;
  std::shared_ptr<Oscillator> xOffsetOscillator;

  std::shared_ptr<Parameter> yOffset;
  std::shared_ptr<Oscillator> yOffsetOscillator;

  std::shared_ptr<Parameter> lockXY;

  MirrorSettings(std::string shaderId, json j)
      : shaderId(shaderId),
        lockXY(std::make_shared<Parameter>("lockXY", shaderId, 0.0, 1.0, 1.0)),
        xOffset(
            std::make_shared<Parameter>("xOffset", shaderId, 0.15, 0.01, 1.0)),
        yOffset(
            std::make_shared<Parameter>("yOffset", shaderId, 0.15, 0.01, 1.0)),
        mirrorXEnabled(std::make_shared<Parameter>("mirrorXEnabled", shaderId,
                                                   1.0, 1.0, 0.0)),
        mirrorYEnabled(std::make_shared<Parameter>("mirrorYEnabled", shaderId,
                                                   1.0, 1.0, 0.0)),
        xOffsetOscillator(std::make_shared<WaveformOscillator>(xOffset)),
        yOffsetOscillator(std::make_shared<WaveformOscillator>(yOffset)),
        ShaderSettings(shaderId) {
    parameters = {lockXY, xOffset, yOffset, mirrorXEnabled, mirrorYEnabled};
    oscillators = {xOffsetOscillator, yOffsetOscillator};
    load(j);
  }
};

struct MirrorShader : Shader {
  ofShader shader;
  MirrorSettings *settings;
  MirrorShader(MirrorSettings *settings)
      : settings(settings), Shader(settings){};

  void setup() override { shader.load("shaders/mirror"); }

  ShaderType type() override { return ShaderTypeMirror; }

  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1i("mirrorXEnabled", settings->mirrorXEnabled->boolValue);
    shader.setUniform1i("mirrorYEnabled", settings->mirrorYEnabled->boolValue);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());

    if (settings->lockXY->boolValue) {
      shader.setUniform2f("offset", settings->xOffset->value,
                          settings->xOffset->value);
    } else {
      shader.setUniform2f("offset", settings->xOffset->value,
                          settings->yOffset->value);
    }

    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }

  void drawSettings() override {
    // Mirror X
    CommonViews::ShaderCheckbox(settings->mirrorXEnabled);

    // Mirror Y
    CommonViews::ShaderCheckbox(settings->mirrorYEnabled);

    // Lock XY
    CommonViews::ShaderCheckbox(settings->lockXY);

    if (settings->lockXY->boolValue) {
      // XY Offset
      CommonViews::ShaderParameter(settings->xOffset,
                                   settings->xOffsetOscillator);
    } else {
      // X Offset
      CommonViews::ShaderParameter(settings->xOffset,
                                   settings->xOffsetOscillator);

      // Y Offset
      CommonViews::ShaderParameter(settings->yOffset,
                                   settings->yOffsetOscillator);
    }
  }
};

#endif /* EmptyShader_hpp */

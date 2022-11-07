//
//  MirrorShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef EmptyShader_hpp
#define EmptyShader_hpp

#include "ofMain.h"
#include "VideoSettings.hpp"
#include "ofxImGui.h"
#include "Shader.hpp"
#include <stdio.h>

struct MirrorShader: Shader {
  ofShader shader;
  MirrorSettings *settings;
  MirrorShader(MirrorSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shadersGL2/new/mirror");
  }
  
  ShaderType type() override {
    return ShaderTypeMirror;
  }
  
  void shade(ofFbo *frame, ofFbo *canvas) override {
    canvas->begin();
    shader.begin();
    shader.setUniformTexture("tex", frame->getTexture(), 4);
    shader.setUniform1i("mirrorXEnabled", settings->mirrorXEnabled.boolValue);
    shader.setUniform1i("mirrorYEnabled", settings->mirrorYEnabled.boolValue);
    shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());
    
    if (settings->lockXY.boolValue) {
      shader.setUniform2f("offset", settings->xOffset.value, settings->xOffset.value);
    } else {
      shader.setUniform2f("offset", settings->xOffset.value, settings->yOffset.value);
    }
    
    frame->draw(0, 0);
    shader.end();
    canvas->end();
  }
  
  void drawSettings() override {
    CommonViews::H3Title("Mirror");
    
    // Mirror X
    ImGui::Text("Mirror X Enabled");
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
    ImGui::Checkbox("Enabled##mirror_x_enabled", &settings->mirrorXEnabled.boolValue);
    
    // Mirror Y
    ImGui::Text("Mirror Y Enabled");
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
    ImGui::Checkbox("Enabled##mirror_y_enabled", &settings->mirrorYEnabled.boolValue);
    
    // Lock XY
    ImGui::Text("Lock XY Enabled");
    ImGui::SetNextItemWidth(150.0);
    ImGui::SameLine(0, 20);
    ImGui::Checkbox("Enabled##lock_xy_enabled", &settings->lockXY.boolValue);
    
    
    if (settings->lockXY.boolValue) {
      // XY Offset
      CommonViews::SliderWithOscillator("X/Y Offset", "##xyoffset", &settings->xOffset, &settings->xOffsetOscillator);
      CommonViews::ModulationSelector(&settings->xOffset);
      CommonViews::MidiSelector(&settings->xOffset);
    } else {
      // X Offset
      CommonViews::SliderWithOscillator("X Offset", "##xoffset", &settings->xOffset, &settings->xOffsetOscillator);
      CommonViews::ModulationSelector(&settings->xOffset);
      CommonViews::MidiSelector(&settings->xOffset);
      
      // Y Offset
      CommonViews::SliderWithOscillator("Y Offset", "##yoffset", &settings->yOffset, &settings->yOffsetOscillator);
      CommonViews::ModulationSelector(&settings->yOffset);
      CommonViews::MidiSelector(&settings->yOffset);
    }
  }
};

#endif /* EmptyShader_hpp */

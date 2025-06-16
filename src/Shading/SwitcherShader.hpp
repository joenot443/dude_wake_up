//
//  SwitcherShader.hpp
//  dude_wake_up
//
//  Re-written 2025-05-21
//

#ifndef SwitcherShader_hpp
#define SwitcherShader_hpp

#include "ofMain.h"
#include "Colors.hpp"
#include "ShaderSettings.hpp"
#include "Fonts.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>

struct SwitcherSettings : public ShaderSettings
{
  std::shared_ptr<Parameter> selectedIndex;
  std::shared_ptr<Parameter> length;
  std::shared_ptr<Parameter> transitionDuration;
  std::shared_ptr<Parameter> blendMode;
  
  SwitcherSettings(std::string shaderId, json j)
  : selectedIndex      (std::make_shared<Parameter>("Output",              0.0, 0.0, 100.0)),
  length             (std::make_shared<Parameter>("Length",              4.0, 0.0, 10.0)),
  transitionDuration (std::make_shared<Parameter>("Transition Duration", 1.0, 0.0,  5.0)),
  blendMode          (std::make_shared<Parameter>("Blend Mode",          0.0, 0.0, 11.0)),
  ShaderSettings(shaderId, j, "Switcher")
  {
    parameters  = { selectedIndex, length, transitionDuration };
    oscillators = {};
    load(j);
    registerParameters();
  }
};

struct SwitcherShader : Shader
{
  // ------------------------------------------------------------------ state
  SwitcherSettings*                                settings;
  std::vector<std::shared_ptr<Connectable>>         activeInputs;
  
  bool   isPlaying      = true;
  bool   transitioning  = false;
  double lastSwitchTime = 0.0;
  float  transitionProg = 0.0f;
  
  std::shared_ptr<ofFbo> previousFbo;   // now just *points* to the
  // outgoing input’s FBO
  
  // ----------------------------------------------------------- life-cycle
  SwitcherShader(SwitcherSettings* s) : settings(s), Shader(s) {}
  
  void clear() override {}
  
  int inputCount() override
  { return std::max(2, std::min(10, static_cast<int>(activeInputs.size() + 1))); }
  
  ShaderType type() override { return ShaderTypeSwitcher; }
  
  void setup() override
  {
    shader.load("shaders/Mix");
    lastSwitchTime = ofGetElapsedTimef();
    isPlaying      = true;
    transitioning  = false;
  }
  
  // -------------------------------------------------------------- helpers
  void drawBlend(const std::shared_ptr<ofFbo>& a,
                 const std::shared_ptr<ofFbo>& b,
                 float mix)
  {
    shader.begin();
    shader.setUniformTexture("tex",  a->getTexture(), 4);
    shader.setUniformTexture("tex2", b->getTexture(), 8);
    shader.setUniform1f    ("tex2_mix", 1.0f - mix);
    b->draw(0, 0, b->getWidth(), b->getHeight());
    shader.end();
  }
  
  static void drawSingle(const std::shared_ptr<ofFbo>& fbo)
  { fbo->draw(0, 0); }
  
  // ---------------------------------------------------------------- shade
  void shade(std::shared_ptr<ofFbo> /*frame*/, std::shared_ptr<ofFbo> canvas) override
  {
    canvas->begin();
    
    // 1. build active input list
    activeInputs.clear();
    for (auto& [slot, conn] : inputs)
      if (conn) activeInputs.emplace_back(conn->start);
    
    if (activeInputs.empty()) {
      ofClear(0, 0, 0, 255);
      canvas->end();
      return;
    }
    
    // clamp selected index
    auto& sel = settings->selectedIndex->intValue;
    sel = (sel + activeInputs.size()) % activeInputs.size();
    
    const double now       = ofGetElapsedTimef();
    const float  playLen   = settings->length->value;
    const float  fadeLen   = std::max(0.f, settings->transitionDuration->value);
    const double phaseTime = now - lastSwitchTime;
    const double cycleLen  = playLen + fadeLen;
    
    // 2. state machine
    if (isPlaying && !transitioning && phaseTime >= playLen)
    {   // ---- BEGIN transition ----
      previousFbo  = activeInputs[sel]->frame();   // live reference
      sel          = (sel + 1) % activeInputs.size();
      transitioning = (fadeLen >= 0.01f);
      if (!transitioning) lastSwitchTime = now;    // hard cut
    }
    else if (transitioning && phaseTime >= cycleLen)
    {   // ---- END transition ----
      transitioning  = false;
      lastSwitchTime = now;
    }
    
    // 3. draw
    std::shared_ptr<ofFbo> currentFbo = activeInputs[sel]->frame();
    
    if (!transitioning || fadeLen < 0.01f) {
      drawSingle(currentFbo);
    } else {
      float prog = static_cast<float>((phaseTime - playLen) / fadeLen);
      prog       = ofClamp(prog, 0.f, 1.f);
      transitionProg = prog;
      drawBlend(previousFbo, currentFbo, prog);
    }
    
    canvas->end();
  }
  
  // -------------------------------------------------------------- UI bits
  void drawSettings() override
  {
    CommonViews::ShaderParameter(settings->length, nullptr);
    
    ImGui::SameLine();
    const char* icon = isPlaying ? ICON_MD_PAUSE : ICON_MD_PLAY_ARROW;
    if (CommonViews::IconButton(icon, shaderId)) isPlaying = !isPlaying;
    
    CommonViews::ShaderParameter(settings->transitionDuration, nullptr);
    drawSelector();
  }
  
  void drawSelector()
  {
    if (activeInputs.empty()) return;
    
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x);
    if (ImGui::BeginTable("split1",
                          static_cast<int>(activeInputs.size()),
                          ImGuiTableFlags_NoSavedSettings |
                          ImGuiTableFlags_Borders |
                          ImGuiTableFlags_SizingFixedFit))
    {
      for (int i = 0; i < activeInputs.size(); ++i)
      {
        ImGui::TableNextColumn();
        if (settings->selectedIndex->intValue == i) {
          ImGui::PushStyleColor(ImGuiCol_Tab, Colors::TabSelected.Value);
        }
        if (ImGui::Selectable(activeInputs[i]->name().c_str(),
                              settings->selectedIndex->intValue == i))
        {
          settings->selectedIndex->intValue = i;
          transitioning  = false;
          lastSwitchTime = ofGetElapsedTimef();
        }
        if (settings->selectedIndex->intValue == i) {
          ImGui::PopStyleColor();
        }
      }
      ImGui::EndTable();
    }
  }
};

#endif // SwitcherShader_hpp

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
#include "AudioSourceService.hpp"
#include "AudioSettings.hpp"
#include <stdio.h>

struct SwitcherSettings : public ShaderSettings
{
  std::shared_ptr<Parameter> selectedIndex;
  std::shared_ptr<Parameter> length;
  std::shared_ptr<Parameter> transitionDuration;
  std::shared_ptr<Parameter> blendMode;
  std::shared_ptr<Parameter> beatMode;
  std::shared_ptr<Parameter> beatsPerSwitch;
  
  SwitcherSettings(std::string shaderId, json j)
  : selectedIndex      (std::make_shared<Parameter>("Output",              0.0, 0.0, 100.0)),
  length             (std::make_shared<Parameter>("Length",              4.0, 0.0, 10.0)),
  transitionDuration (std::make_shared<Parameter>("Transition Duration", 1.0, 0.0,  5.0)),
  blendMode          (std::make_shared<Parameter>("Blend Mode",          0.0, 0.0, 11.0)),
  beatMode           (std::make_shared<Parameter>("Beat Mode",           0.0, 0.0,  1.0)),
  beatsPerSwitch     (std::make_shared<Parameter>("Beats Per Switch",   1.0, 1.0,  8.0)),
  ShaderSettings(shaderId, j, "Switcher")
  {
    parameters  = { selectedIndex, length, transitionDuration, beatMode, beatsPerSwitch };
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
  int    beatCount     = 0;
  float  lastBeatPct   = 0.0f;
  
  std::shared_ptr<ofFbo> previousFbo;   // now just *points* to the
  // outgoing input's FBO
  
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

  bool allowsDryTraversal() override { return true; }

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
  void updateSourceActivity(std::shared_ptr<Connectable> activeSource, std::shared_ptr<Connectable> transitionSource = nullptr) {
    for (auto& [slot, conn] : inputs) {
      // Don't deactivate the active/transition sources
      bool shouldBeActive = activeSource->connId() == conn->start->connId() || (transitionSource && transitionSource->connId() == conn->start->connId());

      if (conn->start->connectableType() == ConnectableTypeSource) {
        auto source = std::dynamic_pointer_cast<VideoSource>(conn->start);
        source->active = shouldBeActive;
      }

      else if (conn->start->hasParentOfType(ConnectableTypeSource)) {
        auto source = conn->start->parentOfType(ConnectableTypeSource);
        source->active = shouldBeActive;
      }
    }
  }

  void shade(std::shared_ptr<ofFbo> /*frame*/, std::shared_ptr<ofFbo> canvas) override
  {
    canvas->begin();
    
    // 1. build input list (we'll manage active state separately)
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
    bool isBeatMode = settings->beatMode->boolValue;

    const double now       = ofGetElapsedTimef();
    const float  playLen   = settings->length->value;
    const float  fadeLen   = std::max(0.f, settings->transitionDuration->value);
    const double phaseTime = now - lastSwitchTime;
    const double cycleLen  = playLen + fadeLen;

    // Get current audio source and beat info
    auto audioSource = AudioSourceService::getService()->selectedAudioSource;
    float currentBeatPct = audioSource->audioAnalysis.beatPulse->value;

    // 2. state machine
    if (isBeatMode) {
      // Check for beat transition (when we cross from high to low)
      if (currentBeatPct > lastBeatPct) {
        beatCount++;
        
        // Switch on every nth beat based on beatsPerSwitch
        if (beatCount >= settings->beatsPerSwitch->intValue) {
          beatCount = 0;
          previousFbo = activeInputs[sel]->frame();
          sel = (sel + 1) % activeInputs.size();
          updateSourceActivity(activeInputs[sel]);
        }
      }
      lastBeatPct = currentBeatPct;
    } else {

      // Original time-based switching logic
      if (isPlaying && !transitioning && phaseTime >= playLen)
      {   // ---- BEGIN transition ----
        previousFbo  = activeInputs[sel]->frame();   // live reference
        sel          = (sel + 1) % activeInputs.size();
        transitioning = (fadeLen >= 0.01f);
        if (!transitioning) {
          lastSwitchTime = now;    // hard cut
          // Update source activity for hard cut
          updateSourceActivity(activeInputs[sel]);
        }
      }
      else if (transitioning && phaseTime >= cycleLen)
      {   // ---- END transition ----
        transitioning  = false;
        lastSwitchTime = now;
        // Update source activity after transition ends
        updateSourceActivity(activeInputs[sel]);
      }
    }
    
    // 3. draw
    std::shared_ptr<ofFbo> currentFbo = activeInputs[sel]->frame();
    
    // Update source activity based on current state
    if (transitioning) {
      // During transition, both current and previous sources should be active
      auto prevSource = activeInputs[(sel - 1 + activeInputs.size()) % activeInputs.size()];
      updateSourceActivity(activeInputs[sel], prevSource);
    } else {
      // Outside transition, only current source should be active
      updateSourceActivity(activeInputs[sel]);
    }
    
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
    // Beat mode toggle
    CommonViews::ShaderCheckbox(settings->beatMode, false);
    
    if (settings->beatMode->boolValue) {
      // In beat mode, show beats per switch control
      CommonViews::IntSlider("Beats Per Switch", shaderId + "_bps", settings->beatsPerSwitch);
      
      ImGui::SameLine();
      const char* icon = isPlaying ? ICON_MD_PAUSE : ICON_MD_PLAY_ARROW;
      if (CommonViews::IconButton(icon, shaderId)) isPlaying = !isPlaying;
      
    } else {
      // In time mode, show original controls
      CommonViews::ShaderParameter(settings->length, nullptr);
      
      ImGui::SameLine();
      const char* icon = isPlaying ? ICON_MD_PAUSE : ICON_MD_PLAY_ARROW;
      if (CommonViews::IconButton(icon, shaderId)) isPlaying = !isPlaying;
      
      CommonViews::ShaderParameter(settings->transitionDuration, nullptr);
    }
    
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
      ImGui::PushStyleColor(ImGuiCol_Header, Colors::AccentColor.Value);
      for (int i = 0; i < activeInputs.size(); ++i)
      {
        ImGui::TableNextColumn();

        if (ImGui::Selectable(activeInputs[i]->name().c_str(),
                              settings->selectedIndex->intValue == i))
        {
          settings->selectedIndex->intValue = i;
          transitioning  = false;
          lastSwitchTime = ofGetElapsedTimef();
        }
      }
      ImGui::PopStyleColor();
      ImGui::EndTable();
    }
  }
};

#endif // SwitcherShader_hpp

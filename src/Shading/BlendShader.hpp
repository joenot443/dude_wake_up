//
//  BlendShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef BlendShader_hpp
#define BlendShader_hpp

#include "ofMain.h"
#include "ShaderSettings.hpp"

#include "LayoutStateService.hpp"
#include "ShaderChainerService.hpp"
#include "CommonViews.hpp"
#include "ofxImGui.h"
#include "ValueOscillator.hpp"
#include "Parameter.hpp"
#include "Shader.hpp"
#include <stdio.h>
#include <map>

static std::vector<std::string> BlendModeNames = {
  "Mix",
  "Multiply",
  "Screen",
  "Darken",
  "Lighten",
  "Difference",
  "Exclusion",
  "Overlay",
  "Hard Light",
  "Soft Light",
  "Color Dodge",
  "Linear Dodge",
  "Burn",
  "Linear Burn"
};

struct BlendSettings: public ShaderSettings {
  static const int MaxBlendInputs = 8;

  std::shared_ptr<Parameter> mode;
  std::shared_ptr<Parameter> swapInputs;
  std::shared_ptr<Parameter> mainStrength;
  std::shared_ptr<WaveformOscillator> mainStrengthOscillator;
  std::shared_ptr<Parameter> inputStrength[MaxBlendInputs];
  std::shared_ptr<WaveformOscillator> inputStrengthOscillator[MaxBlendInputs];

  BlendSettings(std::string shaderId, json j) :
  mode(std::make_shared<Parameter>("Mode", 4.0, 0.0, 13.0, ParameterType_Int)),
  swapInputs(std::make_shared<Parameter>("Swap Inputs", 0.0, 0.0, 1.0, ParameterType_Bool)),
  mainStrength(std::make_shared<Parameter>("Main Input Strength", 1.0, 0.0, 1.0)),
  mainStrengthOscillator(std::make_shared<WaveformOscillator>(mainStrength)),
  ShaderSettings(shaderId, j, "Blend") {
    mode->options = BlendModeNames;
    mode->buttonsVisible = true;

    parameters = { mode, swapInputs, mainStrength };
    oscillators = { mainStrengthOscillator };

    for (int i = 0; i < MaxBlendInputs; i++) {
      std::string name = "Input " + std::to_string(i + 2) + " Strength";
      inputStrength[i] = std::make_shared<Parameter>(name, 1.0, 0.0, 1.0);
      inputStrengthOscillator[i] = std::make_shared<WaveformOscillator>(inputStrength[i]);
      parameters.push_back(inputStrength[i]);
      oscillators.push_back(inputStrengthOscillator[i]);
    }

    audioReactiveParameter = inputStrength[0];
    load(j);
    registerParameters();
  };
};

struct BlendShader: Shader {
  BlendSettings *settings;
  // Map to store preview FBOs for each blend mode
  std::map<std::string, std::shared_ptr<ofFbo>> blendPreviews;
  // Intermediate FBO for preview ping-pong rendering
  std::shared_ptr<ofFbo> previewOptionalFrame;
  // Counter to trigger preview updates
  int frameCount = 0;
  // Flag to track if preview FBOs have been allocated
  bool previewsInitialized = false;
  
  
  BlendShader(BlendSettings *settings) : settings(settings), Shader(settings) {};
  
  void setup() override {
    shader.load("shaders/Blend");
  }
  
  // Helper function to render a preview for a specific blend mode using full input chain
  void renderPreview(std::shared_ptr<ofFbo> targetFbo, int blendModeIndex, std::shared_ptr<ofFbo> mainInput) {
    // Need target, main input, and the auxiliary buffer
    if (!targetFbo || !mainInput || !previewOptionalFrame || !previewOptionalFrame->isAllocated()) return;

    // Ensure helper FBO has correct dimensions (should be handled by allocation logic)
    if (previewOptionalFrame->getWidth() != targetFbo->getWidth() || previewOptionalFrame->getHeight() != targetFbo->getHeight()) {
        ofLogWarning("BlendShader") << "Preview optional frame size mismatch.";
        // Optionally reallocate here, but ideally it's allocated correctly initially
        return;
    }

    // --- Start Ping-Pong Blending for Preview ---

    // Clear target and auxiliary FBOs
    targetFbo->begin();
    ofClear(0, 0, 0, 0);
    targetFbo->end();

    previewOptionalFrame->begin();
    ofClear(0, 0, 0, 0);
    previewOptionalFrame->end();

    // Start with the main frame in the target FBO, scaled by mainStrength
    targetFbo->begin();
    ofSetColor(255, 255, 255, (unsigned char)(255.0f * settings->mainStrength->value));
    mainInput->draw(0, 0);
    ofSetColor(255);
    targetFbo->end();

    bool useOptionalAsSource = false;
    int previewInputIndex = 0;

    // For each possible input slot after the main one
    for (InputSlot slot : AllInputSlots) {
      if (slot == InputSlotMain) continue;
      if (!hasInputAtSlot(slot)) continue;

      std::shared_ptr<ofFbo> sourceFbo = useOptionalAsSource ? previewOptionalFrame : targetFbo;
      std::shared_ptr<ofFbo> destFbo = useOptionalAsSource ? targetFbo : previewOptionalFrame;

      std::shared_ptr<ofFbo> nextFrame = inputAtSlot(slot)->frame();
      if (!nextFrame || !nextFrame->isAllocated()) continue;

      float strength = (previewInputIndex < BlendSettings::MaxBlendInputs)
        ? settings->inputStrength[previewInputIndex]->value : 1.0f;

      destFbo->begin();
      shader.begin();

      shader.setUniform1i("mode", blendModeIndex);
      shader.setUniform1f("strength", strength);
      shader.setUniform2f("dimensions", destFbo->getWidth(), destFbo->getHeight());

      // Set textures based on swap setting
      ofTexture mainTexture = settings->swapInputs->boolValue ? nextFrame->getTexture() : sourceFbo->getTexture();
      ofTexture secondaryTexture = settings->swapInputs->boolValue ? sourceFbo->getTexture() : nextFrame->getTexture();

      shader.setUniformTexture("tex", mainTexture, 4);
      shader.setUniformTexture("tex2", secondaryTexture, 5);

      // Draw source into destination using the shader
      sourceFbo->draw(0, 0);

      shader.end();
      destFbo->end();

      useOptionalAsSource = !useOptionalAsSource;
      previewInputIndex++;
    }

    // Final copy to targetFbo if needed
    if (useOptionalAsSource) {
      targetFbo->begin();
      // Don't clear, just draw the result from the optional frame
      previewOptionalFrame->draw(0, 0);
      targetFbo->end();
    }
    // --- End Ping-Pong Blending for Preview ---
  }
  
  // Helper function to update all previews
  // No longer needs secondaryFrame, as renderPreview iterates through inputs
  void updatePreviews(std::shared_ptr<ofFbo> mainFrame) {
      // Find the index for each blend mode name and render its preview
      for (int i = 0; i < BlendModeNames.size(); ++i) {
          const std::string& modeName = BlendModeNames[i];
          if (blendPreviews.count(modeName)) {
              // Pass only the target FBO, mode index, and main input frame
              renderPreview(blendPreviews[modeName], i, mainFrame);
          }
      }
  }
  
  void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) override {
    // Update visibility of per-input strength parameters based on connections
    settings->mainStrength->hiddenFromNode = !hasInputAtSlot(InputSlotMain);
    int connectedCount = 0;
    for (InputSlot slot : AllInputSlots) {
      if (slot == InputSlotMain) continue;
      if (hasInputAtSlot(slot)) connectedCount++;
    }
    for (int i = 0; i < BlendSettings::MaxBlendInputs; i++) {
      settings->inputStrength[i]->hiddenFromNode = (i >= connectedCount);
    }

    // Guard: bail if FBOs aren't allocated yet (startup race)
    if (!frame || !canvas || !frame->isAllocated() || !canvas->isAllocated()) return;
    if (!optionalFrame || !optionalFrame->isAllocated()) return;
    // Validate GL textures are actually valid (isAllocated can be true with stale textures)
    if (frame->getTexture().getTextureData().textureID == 0) return;
    if (canvas->getTexture().getTextureData().textureID == 0) return;

    // Lazily initialize preview FBOs and the optional frame on the first run or if dimensions change
    if (!previewsInitialized ||
        (previewsInitialized && !blendPreviews.empty() && blendPreviews.begin()->second->getWidth() != frame->getWidth()) ||
        (previewsInitialized && (!previewOptionalFrame || previewOptionalFrame->getWidth() != frame->getWidth())))
    {
        ofFbo::Settings fboSettings;
        fboSettings.width = frame->getWidth();
        fboSettings.height = frame->getHeight();
        fboSettings.internalformat = GL_RGBA; // Or format matching your main FBOs
        fboSettings.textureTarget = GL_TEXTURE_2D;
        // Allocate ping-pong buffer for previews
        previewOptionalFrame = std::make_shared<ofFbo>();
        previewOptionalFrame->allocate(fboSettings);
        previewOptionalFrame->begin();
        ofClear(0,0,0,0);
        previewOptionalFrame->end();


        blendPreviews.clear(); // Clear any old FBOs
        for (const auto& name : BlendModeNames) {
          blendPreviews[name] = std::make_shared<ofFbo>();
          blendPreviews[name]->allocate(fboSettings);
          // Clear initially
          blendPreviews[name]->begin();
          ofClear(0,0,0,0);
          blendPreviews[name]->end();
        }
        previewsInitialized = true;
        frameCount = 0; // Reset frame count to update previews immediately after init
    }
    
    // Update previews every 60 frames (if relevant conditions met)
    frameCount++;
    if (frameCount % 60 == 0 && previewsInitialized && ShaderChainerService::getService()->selectedConnectable != nullptr && ShaderChainerService::getService()->selectedConnectable->connId() == shaderId) {
      // Update previews using only the main frame
      updatePreviews(frame);
    }
    
    // Clear both FBOs
    canvas->begin();
    ofClear(0, 0, 0, 0);
    canvas->end();
    
    optionalFrame->begin();
    ofClear(0, 0, 0, 0);
    optionalFrame->end();
    
    // Start with the main frame in the canvas, scaled by mainStrength
    canvas->begin();
    ofSetColor(255, 255, 255, (unsigned char)(255.0f * settings->mainStrength->value));
    frame->draw(0, 0);
    ofSetColor(255);
    canvas->end();

    bool useOptionalAsSource = false;
    int inputIndex = 0;

    // For each possible input slot after the main one
    for (InputSlot slot : AllInputSlots) {
      if (slot == InputSlotMain) continue;
      if (!hasInputAtSlot(slot)) continue;

      // Determine source and destination FBOs
      std::shared_ptr<ofFbo> sourceFbo = useOptionalAsSource ? optionalFrame : canvas;
      std::shared_ptr<ofFbo> destFbo = useOptionalAsSource ? canvas : optionalFrame;

      std::shared_ptr<ofFbo> nextFrame = inputAtSlot(slot)->frame();
      if (!nextFrame || !nextFrame->isAllocated()) continue;
      if (nextFrame->getTexture().getTextureData().textureID == 0) continue;

      float strength = (inputIndex < BlendSettings::MaxBlendInputs)
        ? settings->inputStrength[inputIndex]->value : 1.0f;

      destFbo->begin();
      shader.begin();
      shader.setUniform1i("mode", settings->mode->intValue);
      shader.setUniform1f("strength", strength);
      shader.setUniform2f("dimensions", frame->getWidth(), frame->getHeight());

      ofTexture mainTexture = settings->swapInputs->boolValue ? nextFrame->getTexture() : sourceFbo->getTexture();
      ofTexture secondaryTexture = settings->swapInputs->boolValue ? sourceFbo->getTexture() : nextFrame->getTexture();

      shader.setUniformTexture("tex", mainTexture, 4);
      shader.setUniformTexture("tex2", secondaryTexture, 5);

      sourceFbo->draw(0, 0);
      shader.end();
      destFbo->end();

      useOptionalAsSource = !useOptionalAsSource;
      inputIndex++;
    }
    
    // Final copy to canvas if needed
    if (useOptionalAsSource) {
      canvas->begin();
      optionalFrame->draw(0, 0);
      canvas->end();
    }
  }
  
  void clear() override {
    // Also clear preview FBOs if needed, though they are managed by shared_ptr
    blendPreviews.clear();
    previewOptionalFrame.reset(); // Reset the shared_ptr for the optional frame
    previewsInitialized = false;
  }
  
  int inputCount() override {
    return inputs.size() + 1;
  }
  
  ShaderType type() override {
    return ShaderTypeBlend;
  }
  
  void drawSettings() override {
    CommonViews::ShaderCheckbox(settings->swapInputs);

    // Main input strength (shown when main is connected)
    if (hasInputAtSlot(InputSlotMain)) {
      CommonViews::ShaderParameter(settings->mainStrength, settings->mainStrengthOscillator);
    }

    // Show per-input strength sliders for connected inputs
    int inputIndex = 0;
    for (InputSlot slot : AllInputSlots) {
      if (slot == InputSlotMain) continue;
      if (!hasInputAtSlot(slot)) continue;
      if (inputIndex < BlendSettings::MaxBlendInputs) {
        CommonViews::ShaderParameter(settings->inputStrength[inputIndex], settings->inputStrengthOscillator[inputIndex]);
      }
      inputIndex++;
    }
    ImGui::Dummy(ImVec2(1.0, 10.0));
    // Add a collapsible header for the previews
    if (ImGui::CollapsingHeader("Blend Previews")) {
      float availableWidth = ImGui::GetContentRegionAvail().x - 40.0;
      float previewWidth = (availableWidth / 3.0) - 10.0;
      int itemsPerRow = std::max(1, (int)(availableWidth / (previewWidth + ImGui::GetStyle().ItemSpacing.x)));
      int count = 0;
      
      // Iterate through all defined blend modes
      for (int i = 0; i < BlendModeNames.size(); ++i) {
        const std::string& modeName = BlendModeNames[i];
        
        // Check if a preview exists for this mode name
        if (blendPreviews.count(modeName)) {
          auto previewFbo = blendPreviews[modeName];
          
          // Ensure the FBO is allocated and valid before trying to draw
          if (previewFbo && previewFbo->isAllocated()) {
            float aspect = (previewFbo->getHeight() > 0) ? (float)previewFbo->getHeight() / (float)previewFbo->getWidth() : 1.0f;
            ImVec2 previewSize = ImVec2(previewWidth, previewWidth * aspect);
            ImVec2 childSize = ImVec2(previewWidth, previewWidth * aspect + 40.0);
            
            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(2.0, 2.0));
            if (ImGui::BeginChild(formatString("BlendPreview%s%d", modeName.c_str(), i).c_str(), childSize)) {
              ImTextureID texID = (ImTextureID)(uintptr_t)previewFbo->getTexture().getTextureData().textureID;
              
              std::string name = formatString("blendPreview%s%d", modeName.c_str(), i); // Use modeName + index for uniqueness
              if (ImGui::ImageButton(name.c_str(), texID, previewSize, ImVec2(0, 1), ImVec2(1, 0))) {
                // If clicked, update the main blend mode setting
                settings->mode->intValue = i;
                settings->mode->affirmIntValue(); // Notify that the int value has changed
              }
              ImGui::Text(modeName.c_str());
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();
            
            count++;
            // Arrange previews in a grid, moving to the next line if needed
            if (count % itemsPerRow != 0) {
              ImGui::SameLine();
            }
          } else {
            // Optional: Draw a placeholder if FBO isn't ready
            ImGui::Dummy(ImVec2(previewWidth, previewWidth)); // Placeholder size
            count++;
            if (count % itemsPerRow != 0) {
              ImGui::SameLine();
            }
          }
        }
      }
      ImGui::NewLine(); // Add some spacing after the preview grid
    }
  }
  
  // Getter for the previews
  const std::map<std::string, std::shared_ptr<ofFbo>>& getPreviews() const {
    return blendPreviews;
  }
};

#endif

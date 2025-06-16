//
//  TextEditorView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/17/23.
//

#include "ofMain.h"
#include "FontService.hpp"
#include "ofxImGui.h"
#include "DisplayText.hpp"
#include "LayoutStateService.hpp"
#include "ImGuiExtensions.hpp"
#include "CommonViews.hpp"
#include "TextEditorView.hpp"
#include "UUID.hpp"
#include "Strings.hpp"
#include "ConfigService.hpp"
#include "ofGraphics.h"
#include "ofRectangle.h"

TextEditorView::TextEditorView(std::shared_ptr<DisplayText> displayText)
: displayText(displayText), id(UUID::generateUUID()) {
  font.load(displayText->font.path, displayText->fontSize);
  previewStrokeShader.load("shaders/Stroke");
}

bool TextEditorView::checkAndReallocateIfNeeded() {
  // Check if any FBO needs reallocation
  bool needsReinitialization = false;
  
  // Check if preview FBOs exist and have correct dimensions
  if (previewsInitialized && !fontPreviews.empty()) {
    auto firstPreview = fontPreviews.begin()->second;
    if (!firstPreview || !firstPreview->isAllocated() || 
        firstPreview->getWidth() != PreviewWidth || 
        firstPreview->getHeight() != PreviewHeight) {
      needsReinitialization = true;
    }
  }
  
  // Check if temp FBO exists and has correct dimensions
  if (previewResourcesAllocated && previewTempFbo) {
    if (!previewTempFbo->isAllocated() || 
        previewTempFbo->getWidth() != PreviewWidth || 
        previewTempFbo->getHeight() != PreviewHeight) {
      needsReinitialization = true;
    }
  }
  
  // Reinitialize if needed
  if (needsReinitialization) {
    // Reset initialization flags
    previewsInitialized = false;
    previewResourcesAllocated = false;
    
    // Clear existing resources
    fontPreviews.clear();
    previewTempFbo.reset();
    
    return true;
  }
  
  return false;
}

void TextEditorView::allocatePreviewResources() {
  if (previewResourcesAllocated) return;

  ofFbo::Settings fboSettings;
  fboSettings.width = PreviewWidth;
  fboSettings.height = PreviewHeight;
  fboSettings.internalformat = GL_RGBA;
  fboSettings.textureTarget = GL_TEXTURE_2D;

  previewTempFbo = std::make_shared<ofFbo>();
  previewTempFbo->allocate(fboSettings);
  previewTempFbo->begin();
  ofClear(0,0,0,0);
  previewTempFbo->end();

  previewResourcesAllocated = true;
}

void TextEditorView::initializePreviews() {
  if (previewsInitialized) return;

  ofFbo::Settings fboSettings;
  fboSettings.width = PreviewWidth;
  fboSettings.height = PreviewHeight;
  fboSettings.internalformat = GL_RGBA;
  fboSettings.textureTarget = GL_TEXTURE_2D;

  fontPreviews.clear();
  auto& fonts = FontService::getService()->fonts;
  for (const auto& fontInfo : fonts) {
    fontPreviews[fontInfo.name] = std::make_shared<ofFbo>();
    fontPreviews[fontInfo.name]->allocate(fboSettings);
    fontPreviews[fontInfo.name]->begin();
    ofClear(0,0,0,0);
    fontPreviews[fontInfo.name]->end();
  }
  previewsInitialized = true;
}

void TextEditorView::renderPreview(std::shared_ptr<ofFbo> targetFbo, const Font& fontToPreview) {
  if (!targetFbo || !targetFbo->isAllocated() || !previewTempFbo || !previewTempFbo->isAllocated()) return;

  int previewFontSize = std::max(1, PreviewHeight / 5);
  if (!previewFont.isLoaded() || previewFont.settings.fontName != fontToPreview.name || previewFont.getSize() != previewFontSize) {
      previewFont.load(fontToPreview.path, previewFontSize);
      if (!previewFont.isLoaded()) {
          ofLogError("TextEditorView") << "Failed to load preview font: " << fontToPreview.path;
          return;
      }
  }

  targetFbo->begin();
  ofClear(0, 0, 0, 0);

  std::string textToDraw = displayText->text.empty() ? "Sample" : displayText->text;
  ofRectangle bounds = previewFont.getStringBoundingBox(textToDraw, 0, 0);
  float x = 10.0f;
  float y = PreviewHeight / 2.0f;

  ofSetColor(ofColor(255.0 * displayText->color->color->data()[0],
                     255.0 * displayText->color->color->data()[1],
                     255.0 * displayText->color->color->data()[2],
                     255.0 * displayText->color->color->data()[3]));

  previewFont.drawString(textToDraw, x, y);
  targetFbo->end();
}

void TextEditorView::updatePreviews() {
  if (!previewsInitialized || !previewResourcesAllocated) return;

  auto& fonts = FontService::getService()->fonts;
  for (const auto& fontInfo : fonts) {
    if (fontPreviews.count(fontInfo.name)) {
      renderPreview(fontPreviews[fontInfo.name], fontInfo);
    }
  }
  previewsNeedUpdate = false;
}

void TextEditorView::draw() {
  ImGui::Text("Text:");
  ImGui::SameLine();
  
  char buf[1024];
  strncpy(buf, displayText->text.c_str(), sizeof(buf));
  buf[sizeof(buf) - 1] = 0;
  bool closeHeader = false;
  if (ImGui::InputTextMultiline("##TextValue", buf, sizeof(buf), ImVec2(ImGui::GetContentRegionAvail().x - 8.0, 200.0))) {
    displayText->text = buf;
    previewsNeedUpdate = true;
    closeHeader = true;
  }
  
  LayoutStateService::getService()->isEditingText = ImGui::IsItemActive();
  
  CommonViews::ShaderOption(displayText->fontSelector, FontService::getService()->fontNames);
  if (closeHeader) ImGui::SetNextItemOpen(false);
  
  if (ImGui::CollapsingHeader("Font Previews")) {
    // Check if FBOs need to be reallocated (e.g., if dimensions changed)
    bool resourcesChanged = checkAndReallocateIfNeeded();
    
    if (!previewResourcesAllocated) {
        allocatePreviewResources();
    }
    if (!previewsInitialized) {
        initializePreviews();
    }
    if (previewsNeedUpdate || resourcesChanged) {
      updatePreviews();
    }
        
    float availableWidth = ImGui::GetContentRegionAvail().x - ImGui::GetStyle().WindowPadding.x * 2.0f;
    int itemsPerRow = std::max(1, static_cast<int>(availableWidth / (PreviewWidth + ImGui::GetStyle().ItemSpacing.x)));
    float previewWidth = PreviewWidth;
    float previewHeight = PreviewHeight;

    auto& fonts = FontService::getService()->fonts;
    for (int i = 0; i < fonts.size(); ++i) {
      const auto& fontInfo = fonts[i];
      if (fontPreviews.count(fontInfo.name)) {
        auto previewFbo = fontPreviews[fontInfo.name];
        if (previewFbo && previewFbo->isAllocated()) {
          ImVec2 previewSize = ImVec2(previewWidth, previewHeight);
          ImGui::BeginChild(formatString("FontPreviewChild%d", i).c_str(), ImVec2(0,0), ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_AutoResizeY, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

          ImTextureID texID = (ImTextureID)(uintptr_t)previewFbo->getTexture().getTextureData().textureID;
          std::string buttonName = formatString("##FontPreviewButton%d", i);

          if (ImGui::ImageButton(buttonName.c_str(), texID, previewSize, ImVec2(0, 0), ImVec2(1, 1))) {
             displayText->fontSelector->setValue(i);
             displayText->font = fontInfo;
             font.load(displayText->font.path, displayText->fontSize);
          }

          ImGui::Text("%s", fontInfo.name.c_str());

          ImGui::EndChild();
        }
      }
    }
    ImGui::NewLine();
  }

  CommonViews::ShaderColor(displayText->color);
  CommonViews::ShaderCheckbox(displayText->strokeEnabled);
  
  if (displayText->strokeEnabled->boolValue) {
      CommonViews::ShaderColor(displayText->strokeColor);
    CommonViews::ShaderParameter(displayText->strokeWeight, nullptr);
  }
  
  int currentFontSize = displayText->fontSize;
  ImGui::Text("Font Size");
  ImGui::SameLine();
  if (ImGui::SliderInt("##FontSizeValue", &currentFontSize, 1, 300)) {
    displayText->fontSize = currentFontSize;
    font.load(displayText->font.path, displayText->fontSize);
  }
  
  CommonViews::MultiSlider("Position", displayText->id, displayText->xPosition, displayText->yPosition, displayText->xPositionOscillator, displayText->yPositionOscillator, 0.5625);
  
  if (displayText->font.name != FontService::getService()->fonts[displayText->fontSelector->intValue].name) {
    displayText->font = FontService::getService()->fonts[displayText->fontSelector->intValue];
    font.load(displayText->font.path, displayText->fontSize);
  }
}

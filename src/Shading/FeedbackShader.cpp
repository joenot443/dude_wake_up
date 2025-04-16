//
//  FeedbackShader.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/5/22.
//

#include "FeedbackShader.hpp"
#include "FeedbackSource.hpp"
#include "CommonViews.hpp"
#include "ImGuiExtensions.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "ShaderChainerService.hpp"
#include "Console.hpp"
#include "FeedbackSource.hpp"
#include "FeedbackSourceService.hpp"

void FeedbackShader::setup()
{
  shader.load("shaders/Feedback");
  populateSource();
};

void FeedbackShader::populateSource()
{
  // Set to a safe default as a backup.
  feedbackSource = FeedbackSourceService::getService()->defaultFeedbackSource;
  
  // If our input slot is connected, we'll use that
  if (hasInputAtSlot(InputSlotTwo)) {
    feedbackSource = FeedbackSourceService::getService()->feedbackSourceForId(inputAtSlot(InputSlotTwo)->connId());
    FeedbackSourceService::getService()->setConsumer(shaderId, feedbackSource);
    return;
  }
  
  if (inputs.empty())
  {
    return;
  }
  
  switch (settings->sourceSelection->intValue)
  {
    case 0: // Origin
      if (hasParentOfType(ConnectableTypeSource))
      {
        std::shared_ptr<Connectable> videoSource = parentOfType(ConnectableTypeSource);
        feedbackSource = FeedbackSourceService::getService()->feedbackSourceForId(videoSource->connId());
        feedbackSource->type = FeedbackType_full;
      }
      else
      {
        feedbackSource = FeedbackSourceService::getService()->defaultFeedbackSource;
        feedbackSource->type = FeedbackType_full;
      }
      break;
    case 1: // Current
    {
      feedbackSource = FeedbackSourceService::getService()->feedbackSourceForId(shaderId);
      feedbackSource->type = FeedbackType_full;
      break;
    }
    case 2: {// Final
      std::shared_ptr<Shader> terminalShader = ShaderChainerService::getService()->terminalShader(std::dynamic_pointer_cast<Shader>(shared_from_this()));
      feedbackSource = FeedbackSourceService::getService()->feedbackSourceForId(terminalShader->shaderId);
      break;
    }
    case 3: // Self
      feedbackSource = FeedbackSourceService::getService()->feedbackSourceForId(shaderId);
      feedbackSource->type = FeedbackType_self;
      if (feedbackSource->hasBeenPrimed == false) feedbackSource->primeFrameBuffer(parentFrame());
      break;
  }
  FeedbackSourceService::getService()->setConsumer(shaderId, feedbackSource);
}

void FeedbackShader::drawPreview(ImVec2 pos, float scale)
{
  ImTextureID texID = (ImTextureID)(uintptr_t)lastFrame->getTexture().getTextureData().textureID;
  ImGui::Image(texID, ImVec2(160*4.0 / scale, 90*4.0 / scale));
}

int FeedbackShader::frameIndex()
{
  return static_cast<int>(settings->delayAmount->max -
                          settings->delayAmount->value);
}

ofTexture FeedbackShader::feedbackTexture()
{
  // Return typical feedback frame if auxillary isn't connected
  if (!hasOutputAtSlot(OutputSlotAux)) {
    return feedbackSource->getFrame(frameIndex());
  }
  
  // First shader in the feedback aux chain
  std::shared_ptr<Shader> auxShader = std::dynamic_pointer_cast<Shader>(outputAtSlot(OutputSlotAux));
  
  // Final descendent of the feedback aux chain
  std::shared_ptr<Shader> terminalAuxShader = std::dynamic_pointer_cast<Shader>(auxShader->terminalDescendent());
  std::shared_ptr<ofFbo> feedbackFbo = feedbackSource->getFbo(frameIndex());
  FramePreview::getInstance().setFrame(feedbackFbo);
  auxShader->traverseFrame(feedbackFbo, 0);
  
  // Push the resultant frame back into the feedbackSource
  if (feedbackSource->type == FeedbackType_self) {
    feedbackSource->pushFrame(terminalAuxShader->lastFrame);
  }
  
  return terminalAuxShader->lastFrame->getTexture();
}

void FeedbackShader::clearFrameIfNeeded() {
  if (settings->shouldClearFeedbackBuffer->boolValue) {
    clearFrameBuffer();
    settings->shouldClearFeedbackBuffer->toggleValue();
  }
}

void FeedbackShader::shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas)
{
  // Set the textures
  populateSource();
  
  clearFrameIfNeeded();
  // Shade the auxillary connection, if available.
  ofTexture fbTex = feedbackTexture();
  
  canvas->begin();
  shader.begin();
  // Clear the frame
  ofClear(0, 0, 0, 255);
  ofClear(0, 0, 0, 0);
  
  shader.setUniform1i("overlayEnabled", 0);
  
  shader.setUniformTexture("fbTexture", fbTex, 3);
  
  shader.setUniform1i("lumaEnabled",
                      settings->lumaKeyEnabled->boolValue);
  shader.setUniformTexture("mainTexture", frame->getTexture(), 4);
  shader.setUniform1f("mainAlpha", settings->mainAlpha->value);
  shader.setUniform1f("fbAlpha", 0.95 + 0.05 * settings->feedbackAlpha->value);
  shader.setUniform1i("priority", settings->priority->boolValue);
  shader.setUniform1f("fbMix", settings->feedbackMix->value);
  shader.setUniform1i("blendMode", settings->blendMode->intValue);
  shader.setUniform1f("scale", settings->scale->value);
  shader.setUniform1f("rotate", settings->rotation->value);
  shader.setUniform1f("lumaKey", settings->keyValue->value);
  shader.setUniform1f("lumaThresh", settings->keyThreshold->value);
  shader.setUniform2f("translate", settings->xPosition->value, -settings->yPosition->value);
  shader.setUniform1f("scale", 2.0 - settings->scale->value);
  
  
  frame->draw(0, 0);

  shader.end();
  canvas->end();
  
  clear();
}


void FeedbackShader::clearFrameBuffer()
{
  if (feedbackSource != nullptr)
  {
    feedbackSource->clearFrameBuffer();
  }
}

void FeedbackShader::drawSettings()
{
  
  
  ImGui::Checkbox(settings->lumaKeyEnabled->name.c_str(), &settings->lumaKeyEnabled->boolValue);
  ImGui::Checkbox(settings->priority->name.c_str(), &settings->priority->boolValue);
  
  if (!hasInputAtSlot(InputSlotTwo)) {
    ImGui::PushItemWidth(50.0);
    if (CommonViews::Selector(settings->sourceSelection, sourceNames))
    {
      populateSource();
    }
  }
  
  ImGui::SameLine();
  CommonViews::HSpacing(3);
  
  CommonViews::ShaderCheckbox(settings->shouldClearFeedbackBuffer);
  // Delay Amount
  CommonViews::ShaderParameter(settings->delayAmount,
                               settings->delayAmountOscillator);
  
  CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
  
  ImGui::Text("Position");
  ImGui::SameLine();
  if (CommonViews::ShaderDropdownButton(settings->positionDropdown)) {
    settings->positionDropdown->buttonsVisible = !settings->positionDropdown->buttonsVisible;
  }
  if (settings->positionDropdown->buttonsVisible) {
    CommonViews::MultiSlider("", formatString("##position%s", shaderId.c_str()), settings->xPosition, settings->yPosition, settings->xPositionOscillator, settings->yPositionOscillator);
  }
  ImGui::NewLine();
  
  CommonViews::ShaderParameter(settings->rotation, settings->rotationOscillator);
  // Blend
  CommonViews::ShaderParameter(settings->mainAlpha, settings->mainAlphaOscillator);
  CommonViews::ShaderParameter(settings->feedbackAlpha, settings->feedbackAlphaOscillator);
  CommonViews::ShaderParameter(settings->feedbackMix, settings->feedbackMixOscillator);
  
  if (settings->lumaKeyEnabled->boolValue)
  {
    // Key Value
    CommonViews::ShaderParameter(settings->keyValue,
                                 settings->keyValueOscillator);
    
    // Threshold
    CommonViews::ShaderParameter(settings->keyThreshold,
                                 settings->keyThresholdOscillator);
  }
  std::vector<const char *> blendModeNamesC;
  for (auto &name : blendModeNames) {
    blendModeNamesC.push_back(name.c_str());
  }
  ImGui::Combo("##BlendMode", &settings->blendMode->intValue, blendModeNamesC.data(), (int) blendModeNames.size());
}

int FeedbackShader::inputCount() { return 1; }

ShaderType FeedbackShader::type() { return ShaderTypeFeedback; }

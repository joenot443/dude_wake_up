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
      }
      else
      {
        feedbackSource = FeedbackSourceService::getService()->defaultFeedbackSource;
      }
      break;
    case 1: // Current
      feedbackSource = FeedbackSourceService::getService()->feedbackSourceForId(shaderId);
      break;
    case 2: // Final
      std::shared_ptr<Shader> terminalShader = ShaderChainerService::getService()->terminalShader(std::dynamic_pointer_cast<Shader>(shared_from_this()));
      feedbackSource = FeedbackSourceService::getService()->feedbackSourceForId(terminalShader->shaderId);
      break;
  }
  FeedbackSourceService::getService()->setConsumer(shaderId, feedbackSource);
}

void FeedbackShader::drawPreview(ImVec2 pos, float scale)
{
  ImTextureID texID = (ImTextureID)(uintptr_t)lastFrame->getTexture().getTextureData().textureID;
  ImGui::Image(texID, ImVec2(160 / scale, 90 / scale));
}

int FeedbackShader::frameIndex()
{
  return static_cast<int>(settings->delayAmount->max -
                          settings->delayAmount->value);
}

ofTexture FeedbackShader::feedbackTexture()
{
  // Return typical feedback frame if auxillary isn't connected
  if (!hasOutputAtSlot(OutputSlotAux)) return feedbackSource->getFrame(frameIndex());
  
  // First shader in the feedback aux chain
  std::shared_ptr<Shader> auxShader = std::dynamic_pointer_cast<Shader>(outputAtSlot(OutputSlotAux));
  
  // Final descendent of the feedback aux chain
  std::shared_ptr<Shader> terminalAuxShader = std::dynamic_pointer_cast<Shader>(auxShader->terminalDescendent());
  std::shared_ptr<ofFbo> feedbackFbo = feedbackSource->getFbo(frameIndex());
  
  auxShader->traverseFrame(feedbackFbo, 0);
  
  return terminalAuxShader->lastFrame->getTexture();
}

void FeedbackShader::shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas)
{
  // Set the textures
  populateSource();
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
  shader.setUniform1f("fbAlpha", settings->feedbackAlpha->value);
  shader.setUniform1i("priority", settings->priority->boolValue);
  shader.setUniform1f("fbMix", settings->feedbackMix->value);
  shader.setUniform1i("blendMode", settings->blendMode->intValue);
  shader.setUniform1f("scale", settings->scale->value);
  shader.setUniform1f("rotate", settings->rotation->value);
  shader.setUniform1f("lumaKey", settings->keyValue->value);
  shader.setUniform1f("lumaThresh", settings->keyThreshold->value);
  shader.setUniform2f("translate", settings->xPosition->value, settings->yPosition->value);
  shader.setUniform1f("scale", 2.0 - settings->scale->value);
  
  
  drawFbo(frame);
  
  //  // With feedback connected we need to flip our FBO to draw it
  //  if (feedbackConnected())
  //  {
  //    ofPushMatrix();
  //    ofScale(1, -1, 1);
  //    ofTranslate(0, -frame->getHeight());
  //    drawFbo(frame);
  //    ofPopMatrix();
  //  }
  //  else
  //  {
  //    drawFbo(frame);
  //  }
  
  shader.end();
  canvas->end();
  
  clear();
}

void FeedbackShader::drawFbo(std::shared_ptr<ofFbo> fbo)
{
  if (settings->rotation->value != 0.0)
  {
    fbo->draw(0, 0);
  }
  else
  {
    fbo->draw(0, 0);
  }
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
  CommonViews::H3Title("Feedback Settings");
  
  ImGui::Checkbox(settings->lumaKeyEnabled->name.c_str(), &settings->lumaKeyEnabled->boolValue);
  ImGui::Checkbox(settings->priority->name.c_str(), &settings->priority->boolValue);
  
  if (!hasInputAtSlot(InputSlotTwo)) {
    ImGui::PushItemWidth(50.0);
    if (ImGui::Combo("Source", &settings->sourceSelection->intValue, "Origin\0Current\0Final\0"))
    {
      populateSource();
    }
  }
  
  ImGui::SameLine();
  CommonViews::HSpacing(3);
  
  if (ImGui::Button(formatString("Clear Feedback Buffer##%s", settings->shaderId.c_str()).c_str()))
  {
    clearFrameBuffer();
  }
  // Delay Amount
  CommonViews::ShaderParameter(settings->delayAmount,
                               settings->delayAmountOscillator);
  
  CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
  CommonViews::MultiSlider("Position", formatString("##position%s", shaderId.c_str()), settings->xPosition, settings->yPosition, settings->xPositionOscillator, settings->yPositionOscillator);
  
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
  } else {
    // Convert the strings to C strings
    std::vector<const char *> blendModeNamesC;
    for (auto &name : blendModeNames) {
      blendModeNamesC.push_back(name.c_str());
    }
    ImGui::Combo("##BlendMode", &settings->blendMode->intValue, blendModeNamesC.data(), (int) blendModeNames.size());
  }
}

int FeedbackShader::inputCount() { return 2; }

ShaderType FeedbackShader::type() { return ShaderTypeFeedback; }

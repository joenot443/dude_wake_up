//
//  LumaLumaFeedbackShader.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/27/23.
//

#include "LumaFeedbackShader.hpp"
#include "FeedbackSource.hpp"
#include "CommonViews.hpp"
#include "ImGuiExtensions.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "ShaderChainerService.hpp"
#include "Console.hpp"
#include "FeedbackSource.hpp"
#include "FeedbackSourceService.hpp"

void LumaFeedbackShader::setup() {
  shader.load("shaders/LumaFeedback");
  populateSource();
};

void LumaFeedbackShader::populateSource() {
  // Set to a safe default as a backup.
  feedbackSource = FeedbackSourceService::getService()->defaultFeedbackSource;
  
  if (inputs.empty()) {
    return;
  }
  
  // If we have an Aux connection to Feedback from, use that by default.
  // Don't draw the source selector in this case
  if (auxConnected()) {
    feedbackSource = FeedbackSourceService::getService()->feedbackSourceForId(aux()->connId());
    FeedbackSourceService::getService()->setConsumer(shaderId, feedbackSource);
    return;
  }
  
  //
  switch (settings->sourceSelection->intValue) {
    case 0: // Origin
      if (hasParentOfType(ConnectableTypeSource)) {
        std::shared_ptr<Connectable> videoSource = parentOfType(ConnectableTypeSource);
        feedbackSource = FeedbackSourceService::getService()->feedbackSourceForId(videoSource->connId());
      } else {
        feedbackSource = FeedbackSourceService::getService()->defaultFeedbackSource;
      }
      break;
    case 1: // Current
      feedbackSource = FeedbackSourceService::getService()->feedbackSourceForId(shaderId);
      break;
    case 2: // Final
      feedbackSource = FeedbackSourceService::getService()->feedbackSourceForId(shaderId);
      break;
  }
  FeedbackSourceService::getService()->setConsumer(shaderId, feedbackSource);
}

void LumaFeedbackShader::shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas) {
  canvas->begin();
  shader.begin();
  // Set the textures
  populateSource();
  int frameIndex = static_cast<int>(settings->delayAmount->max -
                                    settings->delayAmount->value);
  
  ofTexture feedbackTexture = feedbackSource->getFrame(frameIndex);
  feedbackTexture.setTextureWrap(GL_REPEAT, GL_REPEAT);
  shader.setUniformTexture("fbTexture", feedbackTexture, 3);
  
  shader.setUniformTexture("mainTexture", frame->getTexture(), 4);
  shader.setUniform1f("blend", settings->blend->value);
  shader.setUniform1f("scale", settings->scale->value);
  shader.setUniform1f("lumaKey", settings->keyValue->value);
  shader.setUniform1f("lumaThresh", settings->keyThreshold->value);
  shader.setUniform2f("translate", settings->xPosition->value, (0.5 - settings->yPosition->value));
  shader.setUniform1f("scale", 2.0 - settings->scale->value);
  
  
  frame->draw(0, 0);
  shader.end();
  canvas->end();
  
  clear();
}

void LumaFeedbackShader::clearFrameBuffer() {
  if (feedbackSource != nullptr) {
    feedbackSource->clearFrameBuffer();
  }
}

void LumaFeedbackShader::drawSettings() {
  CommonViews::H3Title("LumaFeedback Settings");
  
  if (ImGui::Combo("Source", &settings->sourceSelection->intValue, "Origin\0Current\0Final\0")) {
    populateSource();
  }
  
  ImGui::SameLine();
  if (ImGui::Button(formatString("Clear LumaFeedback Buffer##%s", settings->shaderId.c_str()).c_str())) {
    clearFrameBuffer();
  }
  
  CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
  
  ImGui::Columns(2);
  ImGui::SetColumnWidth(0, 200);
  ImGuiExtensions::Slider2DFloat("", &settings->xPosition->value, &settings->yPosition->value, -1., 1.0, -1., 1.0, 1.0);
  //  ImGui::SetCursorPosX(ImGui::GetCursorPosX() - 50);
  ImGui::NextColumn();
  ImGui::Text("Oscillate X");
  ImGui::SameLine();
  CommonViews::OscillateButton("##yOscillator", settings->xPositionOscillator, settings->xPosition);
  ImGui::Text("Oscillate Y");
  ImGui::SameLine();
  CommonViews::OscillateButton("##yOscillator", settings->yPositionOscillator, settings->yPosition);
  ImGui::Columns(1);
  
  
  // Blend
  CommonViews::ShaderParameter(settings->blend, settings->blendOscillator);
  
  // Delay Amount
  CommonViews::ShaderParameter(settings->delayAmount,
                               settings->delayAmountOscillator);
  
  // Key Value
  CommonViews::ShaderParameter(settings->keyValue,
                               settings->keyValueOscillator);
  
  // Threshold
  CommonViews::ShaderParameter(settings->keyThreshold,
                               settings->keyThresholdOscillator);
}

ShaderType LumaFeedbackShader::type() { return ShaderTypeLumaFeedback; }

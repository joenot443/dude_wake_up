//
//  FeedbackShader.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/5/22.
//

#include "FeedbackShader.hpp"
#include "CommonViews.hpp"
#include "Console.hpp"
#include "FeedbackSource.hpp"
#include "FeedbackSourceService.hpp"

void FeedbackShader::setup() {
  shader.load("../../shaders/feedback");
  if (settings->feedbackSourceId.empty()) {
//    feedbackSource =
//        FeedbackSourceService::getService()->defaultFeedbackSource();
//    settings->feedbackSourceId = feedbackSource->id;
  } else {
    feedbackSource = FeedbackSourceService::getService()->feedbackSourceForId(
        settings->feedbackSourceId);
  }
};

void FeedbackShader::shade(ofFbo *frame, ofFbo *canvas) {
  canvas->begin();
  shader.begin();
  // Set the textures
  int frameIndex = static_cast<int>(settings->delayAmount->max -
                                    settings->delayAmount->value);
  
  ofTexture feedbackTexture;
  if (aux != nullptr && aux->feedbackDestination() != nullptr) {
    feedbackTexture = aux->feedbackDestination()->getFrame(frameIndex);
  } else if (feedbackDestination() != nullptr) {
    feedbackTexture = feedbackDestination()->getFrame(frameIndex);
  }
  shader.setUniformTexture("fbTexture", feedbackTexture, 3);
  
  shader.setUniform1i(settings->lumaKeyEnabled->shaderKey,
                      settings->lumaKeyEnabled->boolValue);
  shader.setUniformTexture("mainTexture", frame->getTexture(), 4);
  shader.setUniform1f(settings->blend->shaderKey, settings->blend->value);
  shader.setUniform1f(settings->keyValue->shaderKey, settings->keyValue->value);
  shader.setUniform1f(settings->keyThreshold->shaderKey,
                      settings->keyThreshold->value);

  frame->draw(0, 0);
  shader.end();
  canvas->end();

  clear();
}

void FeedbackShader::disableFeedback() {
  shader.setUniform1f(settings->keyValue->shaderKey, 0.0);
  shader.setUniform1f(settings->keyThreshold->shaderKey, 0.0);
  shader.setUniform1f(settings->blend->shaderKey, 0.0);
}

void FeedbackShader::clearFrameBuffer() { feedbackSource->clearFrameBuffer(); }

// Draw a selector for the feedback source.
// Use FeedbackSourceService to generate a list of available sources.
void FeedbackShader::drawFeedbackSourceSelector() {
  ImGui::Text("Feedback Source");
  ImGui::SetNextItemWidth(150.0);
  ImGui::SameLine(0, 20);
  // Get the list of available sources.
  std::vector<std::shared_ptr<FeedbackSource>> sources =
      FeedbackSourceService::getService()->feedbackSources();

  // Draw the selector.
  if (ImGui::BeginCombo("##feedbackSource", feedbackSource->name.c_str())) {
    for (int i = 0; i < sources.size(); i++) {
      // Get the id of the source.
      auto source = sources[i];
      bool isSelected = (settings->feedbackSourceId == source->id);
      if (ImGui::Selectable(source->name.c_str(), isSelected)) {
        settings->feedbackSourceId = source->id;
        feedbackSource = source;
      }

      if (isSelected) {
        ImGui::SetItemDefaultFocus();
      }
    }
    ImGui::EndCombo();
  }
}

void FeedbackShader::drawSettings() {
  CommonViews::H3Title("Feedback");

  CommonViews::H4Title("Feedback Parameters");

  CommonViews::ShaderCheckbox(settings->lumaKeyEnabled);

//  drawFeedbackSourceSelector();

  // Blend
  CommonViews::ShaderParameter(settings->blend, settings->blendOscillator);

  // Delay Amount
  CommonViews::ShaderParameter(settings->delayAmount,
                               settings->delayAmountOscillator);

  if (settings->lumaKeyEnabled->boolValue) {
    // Key Value
    CommonViews::ShaderParameter(settings->keyValue,
                                 settings->keyValueOscillator);

    // Threshold
    CommonViews::ShaderParameter(settings->keyThreshold,
                                 settings->keyThresholdOscillator);
  }
}

ShaderType FeedbackShader::type() { return ShaderTypeFeedback; }

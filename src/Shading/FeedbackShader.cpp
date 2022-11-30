//
//  FeedbackShader.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/5/22.
//

#include "FeedbackShader.hpp"
#include "CommonViews.hpp"
#include "FeedbackSourceService.hpp"
#include "FeedbackSource.hpp"
#include "Console.hpp"

void FeedbackShader::setup() {
  shader.load("shadersGL2/new/feedback");
  feedbackSource = FeedbackSourceService::getService()->feedbackSourceForId(settings->feedbackSourceId);
};

void FeedbackShader::shade(ofFbo *frame, ofFbo *canvas) {  
  canvas->begin();
  shader.begin();
  // Set the textures
  int frameIndex = settings->delayAmount->intValue;
  ofTexture feedbackTexture = feedbackSource->getFrame(frameIndex);

  shader.setUniform1i(settings->lumaKeyEnabled->shaderKey, settings->lumaKeyEnabled->boolValue);
  shader.setUniformTexture("mainTexture", frame->getTexture(), 4);
  shader.setUniformTexture("fbTexture", feedbackTexture, 3);
  shader.setUniform1f(settings->blend->shaderKey, settings->blend->value);
  shader.setUniform1f(settings->keyValue->shaderKey, settings->keyValue->value);
  shader.setUniform1f(settings->keyThreshold->shaderKey, settings->keyThreshold->value);
  
  frame->draw(0, 0);
  shader.end();
  canvas->end();
  
  clear();
}

void FeedbackShader::disableFeedback() {
  shader.setUniform1f(settings->keyValue->shaderKey,  0.0);
  shader.setUniform1f(settings->keyThreshold->shaderKey,
                      0.0);
  shader.setUniform1f(settings->mix->shaderKey,
                      0.0);
  shader.setUniform1f(settings->blend->shaderKey,
                      0.0);
}

void FeedbackShader::clearFrameBuffer() {
  feedbackSource->clearFrameBuffer();
}

// Draw a selector for the feedback source.
// Use FeedbackSourceService to generate a list of available sources.
void FeedbackShader::drawFeedbackSourceSelector() {
  ImGui::Text("Feedback Source");
  ImGui::SetNextItemWidth(150.0);
  ImGui::SameLine(0, 20);
  // Get the list of available sources.
  std::vector<std::shared_ptr<FeedbackSource>> sources = FeedbackSourceService::getService()->feedbackSources();

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
  
  
  ImGui::Checkbox("Luma Key Enabled##fb_luma_key", &settings->lumaKeyEnabled->boolValue);
  
  drawFeedbackSourceSelector();
  
  // Mix
  CommonViews::Slider("Blend", "##blend_amount", settings->blend);
  CommonViews::ModulationSelector(settings->blend);
  CommonViews::MidiSelector(settings->blend);
  
  // Delay Amount
  CommonViews::Slider("Delay", "##delay_amount", settings->delayAmount);
  CommonViews::ModulationSelector(settings->delayAmount);
  CommonViews::MidiSelector(settings->delayAmount);

  if (settings->lumaKeyEnabled->boolValue) {
    // Key Value
    CommonViews::Slider("Key Value", "##key_value", settings->keyValue);
    CommonViews::ModulationSelector(settings->keyValue);
    CommonViews::MidiSelector(settings->keyValue);
    
    // Threshold
    CommonViews::Slider("Key Threshold", "##key_threshold", settings->keyThreshold);
    CommonViews::ModulationSelector(settings->keyThreshold);
    CommonViews::MidiSelector(settings->keyThreshold);
  }
}

ShaderType FeedbackShader::type() {
  return ShaderTypeFeedback;
}

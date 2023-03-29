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
  shader.load("shaders/Feedback");
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
  // We have an Aux connection
  if (auxConnected()) {
    feedbackTexture = auxTexture();
  // Feedback from ourselves
  } else if (feedbackDestination() != nullptr) {
    feedbackTexture = feedbackDestination()->getFrame(frameIndex);
  // Default to a blank
  } else {
    feedbackTexture = ofTexture();
  }
  shader.setUniformTexture("fbTexture", feedbackTexture, 3);
  
  shader.setUniform1i("lumaEnabled",
                      settings->lumaKeyEnabled->boolValue);
  shader.setUniformTexture("mainTexture", frame->getTexture(), 4);
  shader.setUniform1f("blend", settings->blend->value);
  shader.setUniform1f("lumaKey", settings->keyValue->value);
  shader.setUniform1f("lumaThresh", settings->keyThreshold->value);

  frame->draw(0, 0);
  shader.end();
  canvas->end();

  clear();
}

void FeedbackShader::clearFrameBuffer() { feedbackSource->clearFrameBuffer(); }

void FeedbackShader::drawSettings() {
  CommonViews::ShaderCheckbox(settings->lumaKeyEnabled);

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

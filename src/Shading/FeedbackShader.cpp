//
//  FeedbackShader.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/5/22.
//

#include "FeedbackShader.hpp"
#include "CommonViews.hpp"
#include "ImGuiExtensions.hpp"
#include "ShaderConfigSelectionView.hpp"
#include "ShaderChainerService.hpp"
#include "Console.hpp"
#include "FeedbackSource.hpp"
#include "FeedbackSourceService.hpp"

void FeedbackShader::setup() {
  shader.load("shaders/Feedback");
  populateSource();
};

void FeedbackShader::populateSource() {
  // Set to a safe default as a backup.
  feedbackSource = FeedbackSourceService::getService()->defaultFeedbackSource;
  
  if (parent == nullptr && parentSource == nullptr) {
    return;
  }
  std::shared_ptr<ShaderChainer> chainer;
  
  // We have an Aux connection to Feedback from
  if (auxConnected()) {
    if (aux != nullptr) {
      chainer = ShaderChainerService::getService()->shaderChainerForShaderId(aux->shaderId);
    } else if (sourceAux != nullptr) {
      chainer = ShaderChainerService::getService()->shaderChainerForAuxShaderIdAndVideoSourceId(shaderId, sourceAux->id);
    }
  } else {
    chainer = ShaderChainerService::getService()->shaderChainerForShaderId(shaderId);
  }
  
  switch (settings->sourceSelection->intValue) {
    case 0: // Origin
      feedbackSource = chainer->source->feedbackDestination;
      break;
    case 1: //Final
      feedbackSource = chainer->feedbackDestination;
      break;
  }

  FeedbackSourceService::getService()->setConsumer(shaderId, feedbackSource);
}

void FeedbackShader::shade(ofFbo *frame, ofFbo *canvas) {
  canvas->begin();
  shader.begin();
  // Set the textures
  populateSource();
  int frameIndex = static_cast<int>(settings->delayAmount->max -
                                    settings->delayAmount->value);
  
  ofTexture feedbackTexture = feedbackSource->getFrame(frameIndex);
  feedbackTexture.setTextureWrap(GL_REPEAT, GL_REPEAT);
  shader.setUniformTexture("fbTexture", feedbackTexture, 3);
  
  shader.setUniform1i("lumaEnabled",
                      settings->lumaKeyEnabled->boolValue);
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

void FeedbackShader::clearFrameBuffer() {
  if (feedbackSource != nullptr) {
    feedbackSource->clearFrameBuffer();
  } else {
    feedbackDestination()->clearFrameBuffer();
  }
}

void FeedbackShader::drawSettings() {
  CommonViews::H3Title("Feedback Settings");
  ShaderConfigSelectionView::draw(this);
  ImGui::Checkbox(settings->lumaKeyEnabled->name.c_str(), &settings->lumaKeyEnabled->boolValue);
  if (ImGui::Combo("Source", &settings->sourceSelection->intValue, "Origin\0Final\0")) {
    populateSource();
  }

  ImGui::SameLine();
  if (ImGui::Button(formatString("Clear Feedback Buffer##%s", settings->shaderId.c_str()).c_str())) {
    clearFrameBuffer();
  }
  
  CommonViews::ShaderParameter(settings->scale, settings->scaleOscillator);
  
  ImGui::Columns(2);
  ImGui::SetColumnWidth(0, 200);
  ImGuiExtensions::Slider2DFloat("", &settings->xPosition->value, &settings->yPosition->value, 0., 0.5, 0., 0.5, 0.5);
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

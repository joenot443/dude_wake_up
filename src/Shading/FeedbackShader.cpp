//
//  FeedbackShader.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/5/22.
//

#include "FeedbackShader.hpp"
#include "CommonViews.hpp"
#include "Console.hpp"

void FeedbackShader::setup() {
  shader.load("shadersGL2/new/feedback");
};

void FeedbackShader::shade(ofFbo *frame, ofFbo *canvas) {
  // If using preprocessed frames for feedback, save it now.
  if (!settings->useProcessedFrame.boolValue) {
    saveFrame(frame);
  }
  
  canvas->begin();
  // Set the textures
  int frameIndex = settings->delayAmount.intValue;
  if (frameIndex < frameBuffer.size()) {
    auto texture = frameBuffer[frameIndex];
    shader.begin();
    shader.setUniformTexture("fbTexture", texture, 5);
  } else {
    log("Missing feedback texture");
    canvas->end();
    return;
  }
  shader.setUniform1i(settings->lumaKeyEnabled.shaderKey, settings->lumaKeyEnabled.intParamValue());
  shader.setUniformTexture("mainTexture", frame->getTexture(), 4);
  shader.setUniform1i(settings->feedbackType.shaderKey, settings->feedbackType.intValue);
  shader.setUniform1f(settings->blend.shaderKey, settings->blend.value);
  shader.setUniform1f(settings->keyValue.shaderKey, settings->keyValue.value);
  shader.setUniform1f(settings->keyThreshold.shaderKey, settings->keyThreshold.value);
  
  frame->draw(0, 0);
  shader.end();
  canvas->end();
  
  // If using processed frames for feedback, save it now.
  if (settings->useProcessedFrame.boolValue) {
    saveFrame(canvas);
  }
  
  clear();
}

void FeedbackShader::saveFrame(ofFbo *frame) {
  float scale = settings->scale.value;
  auto texture = frame->getTexture();
  float width = frame->getWidth();
  float height = frame->getHeight();
  fboFeedback.allocate(width, height);
  fboFeedback.begin();
  ofClear(0,0,0,255);
  float centerX = width / 2.0;
  float centerY = height / 2.0;
  float originX = centerX - centerX * scale;
  float originY = centerY - centerY * scale;
  float w = scale * width;
  float h = scale * height;
  texture.draw(originX, originY, w, h);
  fboFeedback.end();
  auto ret = fboFeedback.getTexture();
  frameBuffer.insert(frameBuffer.begin(), ret);
  
  if (frameBuffer.size() >= FrameBufferCount) {
    frameBuffer.pop_back();
  }
}

void FeedbackShader::disableFeedback() {
  shader.setUniform1f(settings->keyValue.shaderKey,  0.0);
  shader.setUniform1f(settings->keyThreshold.shaderKey,
                      0.0);
  shader.setUniform1f(settings->mix.shaderKey,
                      0.0);
  shader.setUniform1f(settings->blend.shaderKey,
                      0.0);
}

void FeedbackShader::clearFrameBuffer() {
  frameBuffer.clear();
  for(int i=0;i<FrameBufferCount;i++){
    ofFbo frame = ofFbo();
    frame.allocate(ofGetWidth(), ofGetHeight());
    frame.begin();
    ofClear(0,0,0,255);
    frame.end();
    frameBuffer.push_back(frame.getTexture());
  }
}

void FeedbackShader::drawSettings() {
  CommonViews::H3Title("Feedback");
  
  CommonViews::H4Title("Feedback Parameters");
  
  ImGui::Checkbox("Use Processed##fb_use_processed", &settings->useProcessedFrame.boolValue);
  
  ImGui::Checkbox("Luma Key Enabled##fb_luma_key", &settings->lumaKeyEnabled.boolValue);
  
  // Mix
  CommonViews::SliderWithOscillator("Blend", "##blend_amount", &settings->blend, &settings->blendOscillator);
  CommonViews::ModulationSelector(&settings->blend);
  CommonViews::MidiSelector(&settings->blend);
  
  // Delay Amount
  CommonViews::IntSliderWithOscillator("Delay", "##delay_amount", &settings->delayAmount, &settings->delayAmountOscillator);
  CommonViews::ModulationSelector(&settings->delayAmount);
  CommonViews::MidiSelector(&settings->delayAmount);

  if (settings->lumaKeyEnabled.boolValue) {
    // Key Value
    CommonViews::SliderWithOscillator("Key Value", "##key_value", &settings->keyValue, &settings->keyValueOscillator);
    CommonViews::ModulationSelector(&settings->keyValue);
    CommonViews::MidiSelector(&settings->keyValue);
    
    // Threshold
    CommonViews::SliderWithOscillator("Key Threshold", "##key_threshold", &settings->keyThreshold, &settings->keyThresholdOscillator);
    CommonViews::ModulationSelector(&settings->keyThreshold);
    CommonViews::MidiSelector(&settings->keyThreshold);
  }
}

void FeedbackShader::clear() {
  
};

bool FeedbackShader::enabled() {
  return true;
};

std::string FeedbackShader::name() {
  return "Feedback";
}

ShaderType FeedbackShader::type() {
  return ShaderTypeBlur;
}

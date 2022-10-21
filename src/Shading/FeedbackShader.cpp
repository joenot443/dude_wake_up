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
  if (!feedback->useProcessedFrame.boolValue) {
    saveFrame(frame);
  }
  
  canvas->begin();
  // Set the textures
  int frameIndex = feedback->mixSettings.delayAmount.intValue;
  if (frameIndex < frameBuffer.size()) {
    auto texture = frameBuffer[frameIndex];
    shader.begin();
    shader.setUniformTexture("fbTexture", texture, 5);
  } else {
    log("Missing feedback texture");
    canvas->end();
    return;
  }
  shader.setUniform1i(feedback->lumaKeyEnabled.shaderKey, feedback->lumaKeyEnabled.intParamValue());
  shader.setUniformTexture("mainTexture", frame->getTexture(), 4);
  shader.setUniform1i(feedback->mixSettings.feedbackType.shaderKey, feedback->mixSettings.feedbackType.intValue);
  shader.setUniform1f(feedback->mixSettings.blend.shaderKey, feedback->mixSettings.blend.value);
  shader.setUniform1f(feedback->mixSettings.keyValue.shaderKey, feedback->mixSettings.keyValue.value);
  shader.setUniform1f(feedback->mixSettings.keyThreshold.shaderKey, feedback->mixSettings.keyThreshold.value);
  
  frame->draw(0, 0);
  shader.end();
  canvas->end();
  
  // If using processed frames for feedback, save it now.
  if (feedback->useProcessedFrame.boolValue) {
    saveFrame(canvas);
  }
  
  clear();
}

void FeedbackShader::saveFrame(ofFbo *frame) {
  float scale = feedback->miscSettings.scale.value;
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
  shader.setUniform1f(feedback->mixSettings.keyValue.shaderKey,  0.0);
  shader.setUniform1f(feedback->mixSettings.keyThreshold.shaderKey,
                      0.0);
  shader.setUniform1f(feedback->mixSettings.mix.shaderKey,
                      0.0);
  shader.setUniform1f(feedback->mixSettings.blend.shaderKey,
                      0.0);
  shader.setUniform3f(formatString("fb%d_hsb_x", idx), 0.0, 0.0, 1.0);
  shader.setUniform3f(formatString("fb%c_hue_x", idx), 10.0, 0.0, 0.0);
  shader.setUniform3f(formatString("fb%c_rescale", idx),
                      0.0,
                      0.0,
                      1.0);
  
  shader.setUniform1f(formatString("fb%c_rotate", idx), 0.0);
  
  shader.setUniform3f(formatString("fb%c_invert", idx), feedback->hsbSettings.invertHue,
                      feedback->hsbSettings.invertSaturation,
                      feedback->hsbSettings.invertBrightness);
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
  
  ImGui::Checkbox("Use Processed##fb_use_processed", &feedback->useProcessedFrame.boolValue);
  
  ImGui::Checkbox("Luma Key Enabled##fb_luma_key", &feedback->lumaKeyEnabled.boolValue);
  
  // Mix
  CommonViews::SliderWithOscillator("Blend", "##blend_amount", &feedback->mixSettings.blend, &feedback->mixSettings.blendOscillator);
  CommonViews::ModulationSelector(&feedback->mixSettings.blend);
  CommonViews::MidiSelector(&feedback->mixSettings.blend);
  
  // Delay Amount
  CommonViews::IntSliderWithOscillator("Delay", "##delay_amount", &feedback->mixSettings.delayAmount, &feedback->mixSettings.delayAmountOscillator);
  CommonViews::ModulationSelector(&feedback->mixSettings.delayAmount);
  CommonViews::MidiSelector(&feedback->mixSettings.delayAmount);

  if (feedback->lumaKeyEnabled.boolValue) {
    // Key Value
    CommonViews::SliderWithOscillator("Key Value", "##key_value", &feedback->mixSettings.keyValue, &feedback->mixSettings.keyValueOscillator);
    CommonViews::ModulationSelector(&feedback->mixSettings.keyValue);
    CommonViews::MidiSelector(&feedback->mixSettings.keyValue);
    
    // Threshold
    CommonViews::SliderWithOscillator("Key Threshold", "##key_threshold", &feedback->mixSettings.keyThreshold, &feedback->mixSettings.keyThresholdOscillator);
    CommonViews::ModulationSelector(&feedback->mixSettings.keyThreshold);
    CommonViews::MidiSelector(&feedback->mixSettings.keyThreshold);
  }
  
  CommonViews::H4Title("HSB");
  // Hue
  CommonViews::SliderWithInvertOscillator("Hue", "##hue", &feedback->hsbSettings.hue, &feedback->hsbSettings.invertHue, &feedback->hsbSettings.hueOscillator);
  CommonViews::ModulationSelector(&feedback->hsbSettings.hue);
  CommonViews::MidiSelector(&feedback->hsbSettings.hue);
  
  // Saturation
  CommonViews::SliderWithInvertOscillator("Saturation", "##saturation", &feedback->hsbSettings.saturation, &feedback->hsbSettings.invertSaturation, &feedback->hsbSettings.saturationOscillator);
  CommonViews::ModulationSelector(&feedback->hsbSettings.saturation);
  CommonViews::MidiSelector(&feedback->hsbSettings.saturation);
  
  // Brightness
  CommonViews::SliderWithInvertOscillator("Brightness", "##brightness", &feedback->hsbSettings.brightness, &feedback->hsbSettings.invertBrightness, &feedback->hsbSettings.brightnessOscillator);
  CommonViews::ModulationSelector(&feedback->hsbSettings.brightness);
  CommonViews::MidiSelector(&feedback->hsbSettings.brightness);
}

void FeedbackShader::clear() {
  
};

bool FeedbackShader::enabled() {
  return true;
};

std::string FeedbackShader::name() {
  return "Feedback";
}

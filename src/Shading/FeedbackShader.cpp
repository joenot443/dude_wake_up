//
//  FeedbackShader.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/5/22.
//

#include "FeedbackShader.hpp"
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
  shader.setUniform1i(feedback->enabled.shaderKey, feedback->enabled.boolValue);
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

void FeedbackShader::clear() {
  
};

bool FeedbackShader::enabled() {
  return feedback->enabled.boolValue;
};

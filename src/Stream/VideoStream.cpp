//
//  VideoStream.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-07.
//

#include "VideoStream.h"
#include "FontService.hpp"
#include "MidiService.hpp"
#include "CommonViews.hpp"
#include "VideoSettings.h"
#include "ofxImGui.h"
#include "Video.h"

void VideoStream::setup() {
  gui.setup(nullptr, true, ImGuiConfigFlags_ViewportsEnable | ImGuiConfigFlags_DockingEnable);
  switch (config.source) {
    case VideoSource_file:
      player.load(config.path);
      player.play();
      player.setLoopState(OF_LOOP_NORMAL);
      break;
    case VideoSource_webcam:
      cam.listDevices();
      cam.setDeviceID(config.index);
      cam.setup(640, 480);
      break;
  }
  
  shaderMixer.load("shadersGL2/shader_mixer");
  shaderBlur.load("shadersGL2/shader_blur");
  shaderSharpen.load("shadersGL2/shader_sharpen");
  isSetup = true;
  ofSetWindowShape(640, 480);
  
  prepareFbos();
  clearFrameBuffer();
  ofAddListener(window->events().exit, this, &VideoStream::willExit);
}

void VideoStream::willExit(ofEventArgs &args) {
  closeStream(settings->streamId);
}

void VideoStream::update() {
  switch (config.source) {
    case VideoSource_file:
      return player.update();
    case VideoSource_webcam:
      return cam.update();
  }
}

void VideoStream::draw() {
  gui.begin();
  ImGui::PushFont(FontService::getService()->p);
  drawMainFbo();
  shadeHSB();
  shadeBlur();
  shadeSharpen();
  saveFeedbackFrame();
  completeFrame();
  drawDebug();
  if (!firstFrameDrawn) {
    firstDrawSetup();
  }
  ImGui::PopFont();
}

void VideoStream::firstDrawSetup() {
  firstFrameDrawn = true;
  MidiService::getService()->loadConfigFile();
}

void VideoStream::drawVideoPlayer() {
  player.draw(0, 0);
  float playerPosition = player.getPosition();
  float playerSpeed = player.getSpeed();
  
  if (position.value != playerPosition) {
    if (abs(position.value - playerPosition) > 0.01) {
      player.setPosition(position.value);
    } else {
      position.value = playerPosition;
    }
  }
  
  if (speed.value != playerSpeed) {
    if (abs(speed.value - playerSpeed) > 0.01) {
      player.setSpeed(speed.value);
    } else {
      speed.value = playerSpeed;
    }
  }
  
  
  if (ImGui::Begin(std::to_string(settings->streamId).c_str(), NULL, ImGuiWindowFlags_NoTitleBar || ImGuiWindowFlags_NoMove)) {
    ImGui::SliderFloat("Playback", &position.value, 0.0, 1.0);
    ImGui::SliderFloat("Speed", &speed.value, 0.0, 4.0);
    if (ImGui::Button("Clear Feedback")) {
      clearFrameBuffer();
    }
    CommonViews::MidiSelector(&position);
  }
  ImGui::End();
}


void VideoStream::drawMainFbo() {
  fbo.begin();
  drawVideo();
  fbo.end();
}

void VideoStream::shadeHSB() {
  fbo.begin();
  shaderMixer.begin();
  fbo.draw(0,0);
  
  FeedbackSettings feedback = settings->feedback1Settings;
  int index = settings->feedback1Settings.mixSettings.delayAmount.intValue();
  shaderMixer.setUniformTexture("fb0",frameBuffer[index].getTexture(),4);
  shaderMixer.setUniform1f("mix1blend1", 1.0);
  shaderMixer.setUniform1f("mix1keybright", 1.0);
  
  if (feedback.enabled) {
    shaderMixer.setUniform1f("fb0lumakeyvalue", feedback.mixSettings.keyValue.value);
    shaderMixer.setUniform1f("fb0lumakeythresh", feedback.mixSettings.keyThreshold.value);
    shaderMixer.setUniform1f("fb0mix", feedback.mixSettings.mix.value);
    shaderMixer.setUniform1i("fb0enabled", feedback.enabled);
    shaderMixer.setUniform3f("fb0_hsb_x", feedback.hsbSettings.hue.value, feedback.hsbSettings.saturation.value, feedback.hsbSettings.brightness.value);
    
    shaderMixer.setUniform3f("fb0_hue_x", 10.0, 0.0, 0.0);
    shaderMixer.setUniform3f("fb0_rescale",
                             feedback.miscSettings.xOffset.value,
                             feedback.miscSettings.yOffset.value,
                             1.0);
    
    shaderMixer.setUniform1f("fb0_rotate", feedback.miscSettings.rotate.value);
    
    shaderMixer.setUniform3f("fb0_modswitch", feedback.hsbSettings.invertHue, feedback.hsbSettings.invertSaturation, feedback.hsbSettings.invertBrightness);
  } else {
    resetFeedbackValues();
  }
  
  shaderMixer.setUniform1f("cam1_scale", 1);
  shaderMixer.setUniformTexture("cam", streamTexture(), 1);

  shaderMixer.setUniform2f("cam1dimensions", ofVec2f(streamWidth(), streamHeight()));
  shaderMixer.setUniform1f("width", 640);
  shaderMixer.setUniform1f("height", 480);
  shaderMixer.setUniform1i("channel1", 1);
  shaderMixer.setUniform1i("mix1", 2);
  
  shaderMixer.setUniform1f("channel1bright_powmap", 1.0);
  shaderMixer.setUniform1f("channel1hue_powmap", 1.0);
  shaderMixer.setUniform1f("channel1sat_powmap", 1.0);
  
  
  shaderMixer.setUniform1f("channel1hue_x", settings->basicSettings.hsbSettings.hue.value);
  shaderMixer.setUniform1f("channel1saturation_x", settings->basicSettings.hsbSettings.saturation.value);
  shaderMixer.setUniform1f("channel1bright_x", settings->basicSettings.hsbSettings.brightness.value);
  
  shaderMixer.setUniform1i("cam1_pixel_scale_x", settings->basicSettings.pixelSettings.scaleX.intValue());
  shaderMixer.setUniform1i("cam1_pixel_scale_y", settings->basicSettings.pixelSettings.scaleY.intValue());
  shaderMixer.setUniform1f("cam1_pixel_mix", settings->basicSettings.pixelSettings.mix.value);
  shaderMixer.setUniform1f("cam1_pixel_brightscale", 0.0f);
  
  shaderMixer.setUniform1i("cam1_pixel_switch", settings->basicSettings.pixelSettings.enabled);
  
  shaderMixer.end();
  fbo.end();
}

void VideoStream::resetFeedbackValues() {
  shaderMixer.setUniform1i("fb0enabled", false);
  shaderMixer.setUniform1f("fb0lumakeyvalue", 0.0);
  shaderMixer.setUniform1f("fb0lumakeythresh", 0.0);
  shaderMixer.setUniform1f("fb0mix", 0.0);
  shaderMixer.setUniform3f("fb0_hsb_x", 1.0, 1.0, 1.0);
  
  shaderMixer.setUniform3f("fb0_hue_x", 10.0, 0.0, 0.0);
  shaderMixer.setUniform3f("fb0_rescale", 0.0, 0.0, 1.0);
  shaderMixer.setUniform1f("fb0_rotate", 0.0);
}

void VideoStream::shadeBlur() {
  fbo.begin();
  shaderBlur.begin();
  fbo.draw(0,0);
  shaderBlur.setUniformTexture("texmod", fbo.getTexture(), 8);
  shaderBlur.setUniform1f("blur_amount", settings->basicSettings.blurSettings.amount.value);
  shaderBlur.setUniform1f("blur_radius", settings->basicSettings.blurSettings.radius.value);
  shaderBlur.end();
  fbo.end();
}

void VideoStream::shadeSharpen() {
  shaderSharpen.begin();
  fbo.draw(0,0, ofGetWidth(), ofGetHeight());
  shaderSharpen.setUniformTexture("texmod", fbo.getTexture(), 9);
  shaderSharpen.setUniform1f("sharpen_amount", settings->basicSettings.sharpenSettings.amount.value);
  shaderSharpen.setUniform1f("sharpen_radius", settings->basicSettings.sharpenSettings.radius.value);
  shaderSharpen.setUniform1f("sharpen_boost", settings->basicSettings.sharpenSettings.boost.value);
  shaderSharpen.setUniform1f("texmod_sharpen_amount", settings->basicSettings.sharpenSettings.amount.value);
  shaderSharpen.setUniform1f("texmod_sharpen_radius", settings->basicSettings.sharpenSettings.radius.value);
  shaderSharpen.setUniform1f("texmod_sharpen_boost", settings->basicSettings.sharpenSettings.boost.value);
  shaderBlur.end();
}

void VideoStream::shadeGlitch() {
  shaderGlitch.begin();
  fbo.draw(0,0, ofGetWidth(), ofGetHeight());
  shaderGlitch.setUniform2i("resolution", ofGetWidth(), ofGetHeight());
  shaderGlitch.setUniform1f("time", float(ofGetFrameNum() / 60.0));
  shaderGlitch.setUniform1i("glitch", 0.0);
  
}


void VideoStream::prepareFbos() {
  fbo.allocate(ofGetWidth(), ofGetHeight());
  fbo.begin();
  ofClear(0,0,0,255);
  fbo.end();
  
  fboSharpen.allocate(ofGetWidth(), ofGetHeight());
  fboSharpen.begin();
  ofClear(0,0,0,255);
  fboSharpen.end();
  
  fboBlur.allocate(ofGetWidth(), ofGetHeight());
  fboBlur.begin();
  ofClear(0,0,0,255);
  fboBlur.end();
}

void VideoStream::teardown() {
  cam.close();
  player.stop();
  player.close();
  
  window->setWindowShouldClose();
}

void VideoStream::saveFeedbackFrame() {
  ofFbo feedbackFrame = ofFbo();
  feedbackFrame.allocate(ofGetWidth(), ofGetHeight());
  feedbackFrame.begin();
  ofClear(0,0,0,255);
  fbo.draw(0, 0);
  
  feedbackFrame.end();
  frameBuffer.insert(frameBuffer.begin(), feedbackFrame);
  
  if (frameBuffer.size() >= FrameBufferCount) {
    frameBuffer.pop_back();
  }
}


void VideoStream::drawDebug() {
  //  frameBuffer[feedbackDrawIndex()].draw(ofGetWidth() - 200, ofGetHeight() - 100, 200, 100);
  
  //  cout<<"============"<<endl;
  //  cout << "fb0_mix:\t" << settings->feedback1Settings.mixSettings.mix << std::endl;
  //  cout << "fb0_hue:\t" << settings->feedback1Settings.hsbSettings.hue << std::endl;
  //  cout << "fb0_saturation:\t" << settings->feedback1Settings.hsbSettings.saturation << std::endl;
  //  cout << "fb0_bright:\t" << settings->feedback1Settings.hsbSettings.brightness << std::endl;
  //  cout << "fb0_delay_amount:\t" << settings->feedback1Settings.mixSettings.delayAmount << std::endl;
  //  cout<<"============"<<endl;
}


void VideoStream::clearFrameBuffer() {
  frameBuffer.clear();
  for(int i=0;i<FrameBufferCount;i++){
    ofFbo frame = ofFbo();
    frame.allocate(ofGetWidth(), ofGetHeight());
    frameBuffer.push_back(frame);
  }
}


void VideoStream::completeFrame() {
  frameCount += 1;
}

// Helpers

float VideoStream::streamHeight() {
  switch (config.source) {
    case VideoSource_file:
      return player.getHeight();
    case VideoSource_webcam:
      return cam.getHeight();
  }
}

float VideoStream::streamWidth() {
  switch (config.source) {
    case VideoSource_file:
      return player.getWidth();
    case VideoSource_webcam:
      return cam.getWidth();
  }
}

ofTexture VideoStream::streamTexture() {
  switch (config.source) {
    case VideoSource_file:
      return player.getTexture();
    case VideoSource_webcam:
      return cam.getTexture();
  }
}

void VideoStream::drawVideo() {
  switch (config.source) {
    case VideoSource_file:
      return drawVideoPlayer();
    case VideoSource_webcam:
      return cam.draw(0, 0);
  }
}


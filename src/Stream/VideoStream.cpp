//
//  VideoStream.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-07.
//

#include "VideoStream.hpp"
#include "ShaderChainer.hpp"
#include "EmptyShader.hpp"
#include "FontService.hpp"
#include "FeedbackShader.hpp"
#include "Console.hpp"
#include "HSBShader.hpp"
#include "PixelShader.hpp"
#include "BlurShader.hpp"
#include "MidiService.hpp"
#include "CommonViews.hpp"
#include "VideoSettings.hpp"
#include "ofxImGui.h"
#include "Video.hpp"

// MARK: - Lifecycle

void VideoStream::setup() {
  gui.setup(nullptr, true, ImGuiConfigFlags_ViewportsEnable);
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
  
  ofSetWindowShape(640, 480);
  
  for (auto & sh : shaderChainer->shaders) {
    sh->setup();
  }
  
  prepareFbos();
  clearFrameBuffer();
  ofAddListener(window->events().exit, this, &VideoStream::willExit);
}

void VideoStream::firstDrawSetup() {
  firstFrameDrawn = true;
}

void VideoStream::willExit(ofEventArgs &args) {
  closeStream(settings->streamId);
}

void VideoStream::teardown() {
  cam.close();
  player.stop();
  player.close();
  window->setWindowShouldClose();
}

void VideoStream::update() {
  switch (config.source) {
    case VideoSource_file:
      return player.update();
    case VideoSource_webcam:
      return cam.update();
  }
}

// MARK: - Drawing

void VideoStream::draw() {
  gui.begin();
    
  prepareMainFbo();

  fbo = shaderChainer->fboChainingShaders(fbo);
  fbo.draw(0, 0, ofGetWidth(), ofGetHeight());
  
  if (!firstFrameDrawn) {
    firstDrawSetup();
  }
  
  if (settings->videoFlags.resetFeedback.boolValue) {
    settings->videoFlags.resetFeedback.setValue(0.0);
    clearFrameBuffer();
  }
  
}

void VideoStream::prepareMainFbo() {
  fbo.begin();
//  float scale = settings->transformSettings.scale.value;
  drawVideo(1.0);
  fbo.end();
}

void VideoStream::drawVideo(float scale) {
  switch (config.source) {
    case VideoSource_file:
      return drawVideoPlayer();
    case VideoSource_webcam:
      ofClear(0,0,0,255);
      float centerX = streamWidth() / 2.0;
      float centerY = streamHeight() / 2.0;
      float originX = centerX - centerX * scale;
      float originY = centerY - centerY * scale;
      return cam.draw(originX, originY, scale * streamWidth(), scale * streamHeight());
  }
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
  
  drawVideoPlayerMenu();
}

void VideoStream::drawVideoPlayerMenu() {
  if (ImGui::Begin(std::to_string(settings->streamId).c_str(), NULL, ImGuiWindowFlags_NoTitleBar || ImGuiWindowFlags_NoMove)) {
    ImGui::SliderFloat("Playback", &position.value, 0.0, 1.0);
    ImGui::SliderFloat("Speed", &speed.value, 0.0, 4.0);
    if (ImGui::Button("Clear Feedback")) {
      shouldClearFrameBuffer = true;
      clearFrameBuffer();
    }
    CommonViews::MidiSelector(&position);
  }
  ImGui::End();
}


void VideoStream::drawMainFbo() {
  fbo.draw(0,0, ofGetWidth(), ofGetHeight());
}

// MARK: - Shading

void VideoStream::prepareFbos() {
  fbo.allocate(ofGetWidth(), ofGetHeight());
  fbo.begin();
  ofClear(0,0,0,255);
  fbo.end();
}

void VideoStream::drawDebug() {
  //  frameBuffer[feedbackDrawIndex()].draw(ofGetWidth() - 200, ofGetHeight() - 100, 200, 100);
  
  //  cout<<"============"<<endl;
  //  cout << "fb0_mix:\t" << settings->feedback0Settings.mixSettings.mix << std::endl;
  //  cout << "fb0_hue:\t" << settings->feedback0Settings.hsbSettings.hue << std::endl;
  //  cout << "fb0_saturation:\t" << settings->feedback0Settings.hsbSettings.saturation << std::endl;
  //  cout << "fb0_bright:\t" << settings->feedback0Settings.hsbSettings.brightness << std::endl;
  //  cout << "fb0_delay_amount:\t" << settings->feedback0Settings.mixSettings.delayAmount << std::endl;
  //  cout<<"============"<<endl;
}


void VideoStream::clearFrameBuffer() {
  for (auto & fb : shaderChainer->feedbackShaders) {
    fb->clearFrameBuffer();
  }
  shouldClearFrameBuffer = false;
}


// MARK: - Helpers

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

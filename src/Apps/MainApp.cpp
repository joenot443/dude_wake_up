#include "MainApp.h"
#include "ShaderChainer.hpp"
#include "TransformShader.hpp"
#include "ofxImGui.h"
#include "ofMain.h"
#include "AudioStream.hpp"
#include "MainSettings.hpp"
#include "UUID.hpp"
#include "FileSource.hpp"
#include "implot.h"
#include "FontService.hpp"
#include "AudioSettingsView.hpp"
#include "functional"
#include "ModulationService.hpp"
#include "OscillationService.hpp"
#include "MidiService.hpp"
#include "ShaderChainerService.hpp"
#include "WebcamSource.hpp"
#include "VideoSourceService.hpp"
#include <stdio.h>

const static ofVec2f windowSize = ofVec2f(1200, 800);

void MainApp::setup(){
  if (isSetup) return;
  
  ModulationService::getService();
  MidiService::getService();
  
  gui.setup(nullptr, true, ImGuiConfigFlags_ViewportsEnable);
  FontService::getService()->addFontToGui(&gui);
  
  std::function<void(StreamConfig)> videoFn = [this](StreamConfig config) {
    this->configToPush = std::make_shared<StreamConfig>(config);
  };
  
  std::function<void(AudioStreamConfig)> audioFn = [this](AudioStreamConfig config) {
    this->audioConfigToPush = std::make_shared<AudioStreamConfig>(config);
  };
  
  mainSettingsView = new MainSettingsView(new MainSettings(), videoFn, audioFn);
  mainSettingsView->setup();
  isSetup = true;
//  videoFn(StreamConfig(VideoSource_webcam, "", 1));
//  videoFn(StreamConfig(VideoSource_file, "/Users/jcrozier/Libra/ry/Mobile Documents/com~apple~CloudDocs/dude_wake_up/videos/dvd_logo.mp4", 0));
//  std::shared_ptr<AudioStreamConfig> config = make_shared<AudioStreamConfig>(AudioStreamConfig(AudioSource_microphone, "", 3));
  
//    pushAudioStream(config);
}

void MainApp::update(){
  OscillationService::getService()->tickOscillators();
  ModulationService::getService()->tickMappings();
  VideoSourceService::getService()->updateVideoSources();
  ShaderChainerService::getService()->updateShaderChainers();
  
  for (int i = 0; i < audioStreams.size(); i++) {
    if (audioStreams[i]->isSetup) {
      audioStreams[i]->update();
    }
  }
  
  for (int i = 0; i < videoSettingsViews.size(); i++) {
    videoSettingsViews[i]->update();
  }
}

void MainApp::draw(){ 
  gui.begin();
  ImGui::PushFont(FontService::getService()->p);
  ImGui::ShowDemoWindow();
  drawAudioSettings();
  drawMainSettings();
  drawVideoSettings();
  ImGui::PopFont();
  //  ImGui::EndFrame();
  gui.end();
  completeFrame();
  //  gui.draw();
}

void MainApp::completeFrame() {
  if (configToPush != NULL) {
    pushVideoStream(configToPush);
    configToPush = NULL;
  }
  
  if (audioConfigToPush != NULL) {
    pushAudioStream(audioConfigToPush);
    audioConfigToPush = NULL;
  }
}

void MainApp::drawAudioSettings() {
  ImGuiWindowFlags windowFlags = ImGuiWindowFlags_NoTitleBar;
  
  for (int i = 0; i < audioSettingsViews.size(); i++) {
    audioSettingsViews[i]->draw();
  }
}

void MainApp::pushAudioStream(std::shared_ptr<AudioStreamConfig> config) {
  auto audioStream = new AudioStream(*config);
  auto name = config->path.empty() ? std::to_string(config->index) : config->path;
  auto analysis = new AudioAnalysis(name);
  audioStream->analysis = analysis;
  audioStreams.push_back(audioStream);
  ModulationService::getService()->addAudioAnalysis(analysis);
  // Callback for closing stream, passed to VideoStream / VideoSettingsView
  std::function<void(int)> closeStream = [this](int streamId) {
    this->removeAudioStream(streamId);
  };
  
  auto audioSettingView = new AudioSettingsView(analysis, closeStream);
  audioSettingView->analysis = analysis;
  audioSettingView->setup();
  audioStream->setup();
  audioSettingsViews.push_back(audioSettingView);
}

void MainApp::removeAudioStream(int streamId) {
  for (int i = 0; i < videoStreams.size(); i++) {
    if (videoStreams[i]->settings->streamId == streamId) {
      auto stream = videoStreams[i];
      auto window = windows[i];
      stream->teardown();
      ofRemoveListener(window->events().draw, this, &MainApp::drawStream);
      
      windows.erase(windows.begin() + i);
      videoStreams.erase(videoStreams.begin() + i);
      videoSettingsViews.erase(videoSettingsViews.begin() + i);
      mainSettingsView->mainSettings->activeStreams -= 1;
    }
  }
}

void MainApp::pushVideoStream(std::shared_ptr<StreamConfig> config) {
  ofGLFWWindowSettings settings;
  settings.shareContextWith = window;
  settings.setGLVersion(2,1);
  auto streamWindow = ofCreateWindow(settings);
  
  // Callback for closing stream, passed to VideoStream / VideoSettingsView
  std::function<void(int)> closeStream = [this](int streamId) {
    this->removeVideoStream(streamId);
  };
  std::shared_ptr<VideoSource> videoSource = std::shared_ptr<VideoSource>(nullptr);
  
  switch (config->type) {
    case VideoSource_webcam: {
      videoSource =
      std::make_shared<WebcamSource>(UUID::generateUUID(),
                                     config->name,
                                     config->index);
      break;
    }
    case VideoSource_file: {
      videoSource = std::make_shared<FileSource>(UUID::generateUUID(),
                                                     config->name,
                                                     config->path);
      break;
    }
    default:
      break;
  }
  
  VideoSourceService::getService()->addVideoSource(videoSource);
  
  // Each stream created within the lifecycle will increment streamId by 1
  static int streamIdCounter = 0;
  auto streamId = streamIdCounter += 1;

  VideoSettings *videoSettings = new VideoSettings(streamId, std::to_string(streamId));
  
  auto shaderChainer = std::make_shared<ShaderChainer>(std::to_string(streamId), videoSource);
  shaderChainer->name = "Main Chainer";
  shaderChainer->setup();
  
  ShaderChainerService::getService()->addShaderChainer(shaderChainer);
    
  auto streamPointer = std::make_shared<VideoStream>(streamWindow, *config, videoSettings, shaderChainer.get(), closeStream);
  
  videoStreams.push_back(streamPointer.get());
  
  VideoSettingsView *settingsView = new VideoSettingsView(videoSettings, shaderChainer, streamPointer.get(), closeStream);
  videoSettingsViews.push_back(settingsView);
  
  settingsView->setup();
  
  windows.push_back(streamWindow);
  mainSettingsView->mainSettings->activeStreams += 1;

  ofRunApp(streamWindow, streamPointer);
}

void MainApp::removeVideoStream(int streamId) {
  for (int i = 0; i < videoStreams.size(); i++) {
    if (videoStreams[i]->settings->streamId == streamId) {
      auto stream = videoStreams[i];
      auto window = windows[i];
      stream->teardown();
      ofRemoveListener(window->events().draw, this, &MainApp::drawStream);
      
      windows.erase(windows.begin() + i);
      videoStreams.erase(videoStreams.begin() + i);
      videoSettingsViews.erase(videoSettingsViews.begin() + i);
      mainSettingsView->mainSettings->activeStreams -= 1;
    }
  }
}

void MainApp::drawMainSettings() {
  mainSettingsView->draw();
  
}

void MainApp::drawVideoSettings() {
  for (int i = 0; i < videoSettingsViews.size(); i++) {
    auto videoSettingsView = videoSettingsViews[i];
    videoSettingsView->draw();
  }
}

void MainApp::drawStream(ofEventArgs & args) {
  VideoStream *videoStream = NULL;
  VideoSettingsView *settingsView = NULL;
  
  // Find the first stream that hasn't been drawn yet
  for (int i = 0; i < videoStreams.size(); i++) {
    if (!videoStreams[i]->frameDrawn) {
      videoStream = videoStreams[i];
      settingsView = videoSettingsViews[i];
      break;
    }
  }
  
  // We've drawn all the streams
  if (videoStream == NULL || settingsView == NULL) {
    return;
  }
  
  videoStream->draw();
  videoStream->frameDrawn = true;
}

void MainApp::exitStream(ofEventArgs &args) {
  
}



void MainApp::resetState() {
  
}

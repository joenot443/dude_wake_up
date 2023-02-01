//
//  VideoSourceService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#include "VideoSourceService.hpp"
#include "UUID.hpp"
#include "WebcamSource.hpp"
#include "ShaderChainerService.hpp"
#include "FileSource.hpp"

void VideoSourceService::setup() {
  // Add an AvailableVideoSource for each Webcam and each ShaderType
  for (auto const& x : ofVideoGrabber().listDevices()) {
    auto webcamSource = std::make_shared<AvailableVideoSource>(x.deviceName, VideoSource_webcam, ShaderSource_empty, x.id, "");
    availableSourceMap[webcamSource->availableVideoSourceId] = webcamSource;
  }
  
  for (auto const& x : AvailableShaderSourceTypes) {
    auto shaderSource = std::make_shared<AvailableVideoSource>(shaderSourceTypeName(x), VideoSource_shader, x, 0, "");
    shaderSource->shaderType = x;
    availableSourceMap[shaderSource->availableVideoSourceId] = shaderSource;
  }
}

ofFbo VideoSourceService::previewFbo() {
  auto fbo = ofFbo();
  auto blankFbo = ofFbo();
  fbo.allocate(320, 240);
  blankFbo.allocate(320, 240);

  auto shader = ShaderChainerService::getService()->shaderForType(
      ShaderTypeOctahedron, UUID::generateUUID(), 0);
  shader->setup();
  shader->shade(&blankFbo, &fbo);
  return fbo;
}

// Create a FeedbackSource for the passed in VideoSource and register it in the FeedbackSourceService
void VideoSourceService::createFeedbackSource(std::shared_ptr<VideoSource> videoSource) {
  std::shared_ptr<FeedbackSource> feedbackSource = std::make_shared<FeedbackSource>(videoSource->id, videoSource->sourceName);
  FeedbackSourceService::getService()->registerFeedbackSource(feedbackSource);
  videoSource->feedbackDestination = feedbackSource;
}

// Iterate over every video source in the map and update them
void VideoSourceService::updateVideoSources() {
  for (auto const& x : videoSourceMap) {
      x.second->saveFrame();
      x.second->saveFeedbackFrame();
  }
}

std::shared_ptr<VideoSource> VideoSourceService::defaultVideoSource() {
  return videoSources().front();
}

// Add a video source to the map
void VideoSourceService::addVideoSource(std::shared_ptr<VideoSource> videoSource) {
  videoSourceMap[videoSource->id] = videoSource;
  createFeedbackSource(videoSource);
  videoSource->setup();
}

// Remove a video source from the map
void VideoSourceService::removeVideoSource(std::string id) {
  videoSourceMap.erase(id);
}

// Return a vector of all video sources
std::vector<std::shared_ptr<VideoSource>> VideoSourceService::videoSources() {
  std::vector<std::shared_ptr<VideoSource>> videoSources;
  for (auto const& x : videoSourceMap) {
    if (x.second != nullptr) {
      videoSources.push_back(x.second);
    }
  }
  return videoSources;
}

// Return a vector of all AvailableVideoSource
std::vector<std::shared_ptr<AvailableVideoSource>> VideoSourceService::availableVideoSources() {
  std::vector<std::shared_ptr<AvailableVideoSource>> videoSources;
  for (auto const& x : availableSourceMap) {
    if (x.second != nullptr) {
      videoSources.push_back(x.second);
    }
  }
  return videoSources;
}

// Return a vector of all video sources which should be used as inputs (non empty, non ShaderChainer)

std::vector<std::shared_ptr<VideoSource>> VideoSourceService::inputSources() {
  std::vector<std::shared_ptr<VideoSource>> videoSources;
  for (auto const& x : videoSourceMap) {
    if (x.second != nullptr && !(x.second->type == VideoSource_chainer)) {
      videoSources.push_back(x.second);
    }
  }
  return videoSources;
}

// Return a vector of the names of available webcams
std::vector<std::string> VideoSourceService::getWebcamNames() {
  std::vector<std::string> webcamNames;
  for (auto const& x : ofVideoGrabber().listDevices()) {
    webcamNames.push_back(x.deviceName);
  }
  return webcamNames;
}

// Adds a webcam video source to the map
std::shared_ptr<VideoSource> VideoSourceService::addWebcamVideoSource(std::string name, int deviceId, std::string id) {
  std::shared_ptr<VideoSource> videoSource = std::make_shared<WebcamSource>(id, name, deviceId);
  addVideoSource(videoSource);
  return videoSource;
}

// Adds a file video source to the map
std::shared_ptr<VideoSource> VideoSourceService::addFileVideoSource(std::string name, std::string path, std::string id) {
  std::shared_ptr<VideoSource> videoSource = std::make_shared<FileSource>(id, name, path);
  addVideoSource(videoSource);
  return videoSource;
}

// Adds a Shader video source to the map
std::shared_ptr<VideoSource> VideoSourceService::addShaderVideoSource(ShaderSourceType type, std::string id) {
  std::shared_ptr<VideoSource> videoSource = std::make_shared<ShaderSource>(id, type);
  addVideoSource(videoSource);
  return videoSource;
}

// Adds an OutputWindow for the passed in VideoSource
void VideoSourceService::addOutputWindowForVideoSource(std::shared_ptr<VideoSource> videoSource) {
  std::shared_ptr<OutputWindow> outputWindow = std::make_shared<OutputWindow>(videoSource);
  outputWindow->setup();
  ofGLFWWindowSettings settings;
  settings.shareContextWith = ofGetCurrentWindow();
  settings.setSize(1920, 1080);
  settings.setGLVersion(3, 2);
  auto streamWindow = ofCreateWindow(settings);
  ofRunApp(streamWindow, outputWindow);
  outputWindows.push_back(outputWindow);
}

// ConfigurableService

json VideoSourceService::config() {
  auto sources = videoSources();
  json container;
  
  for (auto source : sources) {
    // Don't serialize Chainer video sources, they'll be added in the ShaderChainerService
    if (source->type == VideoSource_chainer) { continue; }
    container[source->id] = source->serialize();
  }
  
  return container;
}

void VideoSourceService::loadConfig(json data) {
  videoSourceMap.clear();
  addShaderVideoSource(ShaderSource_empty);
  
  std::map<std::string, json> sourcesMap = data;
  
  for (auto pair : sourcesMap) {
    json j = pair.second;
    VideoSourceType type = j["videoSourceType"];
    std::string sourceId = j["id"];
    
    switch (type) {
      case VideoSource_file:
        addFileVideoSource(j["sourceName"], j["path"], sourceId);
        continue;
      case VideoSource_webcam:
        addWebcamVideoSource(j["sourceName"], j["deviceId"], sourceId);
        continue;
      case VideoSource_shader:
        addShaderVideoSource(j["shaderSourceType"], sourceId);
        continue;
      case VideoSource_chainer:
        continue;
    }
  }
}

std::shared_ptr<VideoSource> VideoSourceService::videoSourceForId(std::string id) {
  if (videoSourceMap.count(id)) {
    return videoSourceMap[id];
  }
  return nullptr;
}


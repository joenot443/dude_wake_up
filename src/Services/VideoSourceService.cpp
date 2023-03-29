//
//  VideoSourceService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#include "VideoSourceService.hpp"
#include "UUID.hpp"
#include "TextSource.hpp"
#include "WebcamSource.hpp"
#include "ImageSource.hpp"
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
  
  auto textSource = std::make_shared<AvailableVideoSource>("Basic Text", VideoSource_text, ShaderSource_empty, 0, "");
  availableSourceMap[textSource->availableVideoSourceId] = textSource;
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
  std::shared_ptr<FeedbackSource> feedbackSource = std::make_shared<FeedbackSource>(videoSource->id, videoSource->sourceName, std::make_shared<VideoSourceSettings>(videoSource->settings));
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
void VideoSourceService::addVideoSource(std::shared_ptr<VideoSource> videoSource, std::string id) {
  videoSourceMap[id] = videoSource;
  createFeedbackSource(videoSource);
  videoSource->setup();
}

// Remove a video source from the map
void VideoSourceService::removeVideoSource(std::string id) {
  videoSourceMap.erase(id);
  FeedbackSourceService::getService()->removeFeedbackSource(id);
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
  addVideoSource(videoSource, id);
  return videoSource;
}

// Adds a file video source to the map
std::shared_ptr<VideoSource> VideoSourceService::addFileVideoSource(std::string name, std::string path, std::string id) {
  std::shared_ptr<VideoSource> videoSource = std::make_shared<FileSource>(id, name, path);
  addVideoSource(videoSource, id);
  return videoSource;
}

// Adds a Shader video source to the map
std::shared_ptr<VideoSource> VideoSourceService::addShaderVideoSource(ShaderSourceType type, std::string id) {
  std::shared_ptr<VideoSource> videoSource = std::make_shared<ShaderSource>(id, type);
  addVideoSource(videoSource, id);
  return videoSource;
}

// Adds an Image video source to the map
std::shared_ptr<VideoSource> VideoSourceService::addImageVideoSource(std::string name, std::string path, std::string id) {
  std::shared_ptr<VideoSource> videoSource = std::make_shared<ImageSource>(id, name, path);
  addVideoSource(videoSource, id);
  return videoSource;
}

// Adds an Image video source to the map
std::shared_ptr<VideoSource> VideoSourceService::addTextVideoSource(std::string name, std::string id) {
  auto displayText = std::make_shared<DisplayText>();
  auto textSource = TextSource(id, name, displayText);
  std::shared_ptr<VideoSource> videoSource = std::make_shared<TextSource>(textSource);
  addVideoSource(videoSource, id);
  return videoSource;
}

// Adds an OutputWindow for the passed in VideoSource
void VideoSourceService::addOutputWindowForChainer(std::shared_ptr<ShaderChainer> chainer) {
  std::shared_ptr<OutputWindow> outputWindow = std::make_shared<OutputWindow>(chainer);
  outputWindow->setup();
  ofGLFWWindowSettings settings;
  settings.shareContextWith = ofGetCurrentWindow();
  settings.setSize(chainer->settings.width->value, chainer->settings.height->value);
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

void VideoSourceService::appendConfig(json j) {
  std::map<std::string, json> sourcesMap = j;
  
  VideoSourceType type = j["videoSourceType"];
  std::string sourceId = j["id"];
  
  switch (type) {
    case VideoSource_file:
      addFileVideoSource(j["sourceName"], j["path"], sourceId);
      return;
    case VideoSource_webcam:
      addWebcamVideoSource(j["sourceName"], j["deviceId"], sourceId);
      return;
    case VideoSource_shader:
      addShaderVideoSource(j["shaderSourceType"], sourceId);
      return;
    case VideoSource_chainer:
      return;
  }
}

void VideoSourceService::loadConfig(json data) {
  videoSourceMap.clear();
  std::map<std::string, json> sourceMap = data;
  
  for (auto pair : sourceMap) {
    std::map<std::string, json> source = pair.second;
    appendConfig(source);
  }
  addShaderVideoSource(ShaderSource_empty);
  appendConfig(data);
}

std::shared_ptr<VideoSource> VideoSourceService::videoSourceForId(std::string id) {
  if (videoSourceMap.count(id)) {
    return videoSourceMap[id];
  }
  return nullptr;
}


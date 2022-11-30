//
//  VideoSourceService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#include "VideoSourceService.hpp"
#include "UUID.hpp"
#include "WebcamSource.hpp"
#include "FileSource.hpp"

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
    videoSources.push_back(x.second);
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
void VideoSourceService::addWebcamVideoSource(std::string name, int deviceId) {
  std::shared_ptr<VideoSource> videoSource = std::make_shared<WebcamSource>(UUID::generateUUID(), name, deviceId);
  addVideoSource(videoSource);
}

// Adds a file video source to the map
void VideoSourceService::addFileVideoSource(std::string name, std::string path) {
  std::shared_ptr<VideoSource> videoSource = std::make_shared<FileSource>(UUID::generateUUID(), name, path);
  addVideoSource(videoSource);
}

// Adds a Shader video source to the map
void VideoSourceService::addShaderVideoSource(ShaderSourceType type) {
  std::shared_ptr<VideoSource> videoSource = std::make_shared<ShaderSource>(UUID::generateUUID(), type);
  addVideoSource(videoSource);
}

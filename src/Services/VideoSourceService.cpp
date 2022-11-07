//
//  VideoSourceService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#include "VideoSourceService.hpp"

// Iterate over every video source in the map and update them
void VideoSourceService::updateVideoSources() {
  for (auto const& x : videoSourceMap) {
    x.second->update();
  }
}

// Add a video source to the map
void VideoSourceService::addVideoSource(std::shared_ptr<VideoSource> videoSource) {
  videoSourceMap[videoSource->id] = videoSource;
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

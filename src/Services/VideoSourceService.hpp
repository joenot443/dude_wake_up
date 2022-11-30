//
//  VideoSourceService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#ifndef VideoSourceService_hpp
#define VideoSourceService_hpp

#include <stdio.h>
#include "VideoSource.hpp"
#include "ShaderSource.hpp"
#include "FeedbackSourceService.hpp"

// Singleton service which stores shared references to VideoSource and allows for their addition and deletion

class VideoSourceService {
private:
  std::map<std::string, std::shared_ptr<VideoSource>> videoSourceMap;
  void createFeedbackSource(std::shared_ptr<VideoSource> videoSource);

public:
  static VideoSourceService* service;
  VideoSourceService() {};
  
  static VideoSourceService* getService() {
    if (!service) {
      service = new VideoSourceService;
    }
    return service;
    
  }
  std::vector<std::shared_ptr<VideoSource>> videoSources();
  std::vector<std::string> videoSourceNames();
  std::vector<std::string> getWebcamNames();
  void updateVideoSources();
  void removeVideoSource(std::string id);
  void addVideoSource(std::shared_ptr<VideoSource> videoSource);
  std::shared_ptr<VideoSource> videoSourceForId(std::string id);

  void addWebcamVideoSource(std::string name, int index);
  void addShaderVideoSource(ShaderSourceType type);
  void addFileVideoSource(std::string name, std::string path);
};

#endif /* VideoSourceService_hpp */

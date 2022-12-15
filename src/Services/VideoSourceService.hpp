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
#include "OutputWindow.hpp"
#include "ConfigurableService.hpp"

using json = nlohmann::json;

// Singleton service which stores shared references to VideoSource and allows for their addition and deletion

class VideoSourceService: ConfigurableService {
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
  
  // VideoSources which should be used as inputs (non empty, non ShaderChainer)
  std::vector<std::shared_ptr<VideoSource>> inputSources();
  
  std::shared_ptr<VideoSource> defaultVideoSource();
  
  std::vector<std::string> videoSourceNames();
  std::vector<std::string> getWebcamNames();
  void updateVideoSources();
  void removeVideoSource(std::string id);
  void addVideoSource(std::shared_ptr<VideoSource> videoSource);
  std::shared_ptr<VideoSource> videoSourceForId(std::string id);

  void addWebcamVideoSource(std::string name, int index, std::string id = UUID::generateUUID());
  void addShaderVideoSource(ShaderSourceType type, std::string id = UUID::generateUUID());
  void addFileVideoSource(std::string name, std::string path, std::string id = UUID::generateUUID());

  // Output Windows

  std::vector<std::shared_ptr<OutputWindow>> outputWindows;
  void addOutputWindowForVideoSource(std::shared_ptr<VideoSource> videoSource);

  json config() override;
  void loadConfig(json j) override;
};

#endif /* VideoSourceService_hpp */

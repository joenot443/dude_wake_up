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
#include "TextSource.hpp"
#include "FeedbackSourceService.hpp"
#include "AvailableVideoSource.hpp"
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
      service->setup();
    }
    return service;
  }
  
  void setup();
  
  std::vector<std::shared_ptr<VideoSource>> videoSources();
  
  // ofFbo used to draw the Shader previews
  ofFbo previewFbo();
  
  // VideoSources which should be used as inputs (non empty, non ShaderChainer)
  std::vector<std::shared_ptr<VideoSource>> inputSources();
  
  // VideoSources which are available to be added to the Service
  std::vector<std::shared_ptr<AvailableVideoSource>> availableVideoSources();
  
  std::shared_ptr<VideoSource> defaultVideoSource();
  
  std::map<std::string, std::shared_ptr<AvailableVideoSource>> availableSourceMap;
  
  std::vector<std::string> videoSourceNames();
  std::vector<std::string> getWebcamNames();
  void updateVideoSources();
  void removeVideoSource(std::string id);
  void addVideoSource(std::shared_ptr<VideoSource> videoSource, std::string id);
  std::shared_ptr<VideoSource> videoSourceForId(std::string id);
  
  std::shared_ptr<VideoSource> addImageVideoSource(std::string name, std::string path, ImVec2 origin = ImVec2(0.,0.), std::string id = UUID::generateUUID());
  std::shared_ptr<VideoSource> addWebcamVideoSource(std::string name, int index, ImVec2 origin = ImVec2(0.,0.), std::string id = UUID::generateUUID());
  std::shared_ptr<VideoSource> addShaderVideoSource(ShaderSourceType type, ImVec2 origin = ImVec2(0.,0.), std::string id = UUID::generateUUID());
  std::shared_ptr<VideoSource> addFileVideoSource(std::string name, std::string path, ImVec2 origin = ImVec2(0.,0.), std::string id = UUID::generateUUID());
  std::shared_ptr<VideoSource> addTextVideoSource(std::string name, ImVec2 origin = ImVec2(0.,0.), std::string id = UUID::generateUUID());
  
  // Output Windows

  std::vector<std::shared_ptr<OutputWindow>> outputWindows;
  void addOutputWindowForChainer(std::shared_ptr<ShaderChainer> chainer);
  
  void clear();
  json config() override;
  void loadConfig(json j) override;
  void appendConfig(json j);
};

#endif /* VideoSourceService_hpp */

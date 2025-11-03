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
#include "LibrarySource.hpp"
#include "MultiSource.hpp"
#include "ShaderSource.hpp"
#include "TextSource.hpp"
#include "FeedbackSourceService.hpp"
#include "AvailableVideoSource.hpp"
#include "OutputWindow.hpp"
#include "ConfigurableService.hpp"
#include "observable.hpp"

using json = nlohmann::json;

// Singleton service which stores shared references to VideoSource and allows for their addition and deletion

class VideoSourceService : ConfigurableService
{
private:
  std::map<std::string, std::shared_ptr<VideoSource>> videoSourceMap;
  void createFeedbackSource(std::shared_ptr<VideoSource> videoSource);

public:
  static VideoSourceService *service;
  VideoSourceService() : emptyFbo(std::make_shared<ofFbo>()){};

  static VideoSourceService *getService()
  {
    if (!service)
    {
      service = new VideoSourceService;
      service->setup();
    }
    return service;
  }

  void setup();

  std::vector<std::shared_ptr<VideoSource>> videoSources();

  // ofFbo used to draw the Shader previews
  std::shared_ptr<ofFbo> previewFbo();

  // VideoSources which should be used as inputs (non empty, non ShaderChainer)
  std::vector<std::shared_ptr<VideoSource>> inputSources();

  // VideoSources which are available to be added to the Service
  std::vector<std::shared_ptr<AvailableVideoSource>> availableVideoSources();
  std::vector<std::shared_ptr<AvailableVideoSource>> availableVideoSourcesForType(VideoSourceType type);
  std::vector<std::shared_ptr<AvailableVideoSourceShader>> availableShaderVideoSources();

  int indexOfSourceType(ShaderSourceType type);

  
  std::shared_ptr<VideoSource> defaultVideoSource();
  std::shared_ptr<VideoSourceSettings> defaultVideoSourceSettings();
  
  void startAccessingBookmarkPath(std::string path);

  std::shared_ptr<VideoSource> replaceShaderVideoSource(std::shared_ptr<ShaderSource> shaderSource, ShaderSourceType type);
//  std::shared_ptr<VideoSource> replaceWebcamVideoSource(std::shared_ptr<WebcamSource> shaderSource, ShaderSourceType type);
  
  std::shared_ptr<ofFbo> emptyFbo;

  std::map<std::string, std::shared_ptr<AvailableVideoSource>> availableSourceMap;

  observable::subject<void()> availableVideoSourceUpdateSubject;

  void subscribeToAvailableVideoSourceUpdates(std::function<void()> callback);

  std::shared_ptr<AvailableVideoSource> availableVideoSourceForId(std::string id);

  std::vector<std::string> videoSourceNames();
  std::vector<std::string> getWebcamNames();
  void updateVideoSources();
  void removeVideoSource(std::string id);
  void addVideoSource(std::shared_ptr<VideoSource> videoSource, std::string id, json j = 0);
  void populateAvailableVideoSources();
  void addAvailableVideoSources(std::vector<std::shared_ptr<AvailableVideoSource>> sources);

  
  std::shared_ptr<VideoSource> videoSourceForId(std::string id);

  std::shared_ptr<VideoSource> makeImageVideoSource(std::string name, std::string path, ImVec2 origin = ImVec2(0., 0.), std::string id = UUID::generateUUID(), json j = 0);
  std::shared_ptr<MultiSource> makeMultiVideoSource(std::string name, ImVec2 origin = ImVec2(0., 0.), std::string id = UUID::generateUUID(), json j = 0);
  std::shared_ptr<VideoSource> makeWebcamVideoSource(std::string name, int deviceId, ImVec2 origin = ImVec2(0., 0.), std::string id = UUID::generateUUID(), json j = 0);
  std::shared_ptr<VideoSource> makeShaderVideoSource(ShaderSourceType type, ImVec2 origin = ImVec2(0., 0.), std::string id = UUID::generateUUID(), json j = 0);
  std::shared_ptr<VideoSource> makeFileVideoSource(std::string name, std::string path, ImVec2 origin = ImVec2(0., 0.), std::string id = UUID::generateUUID(), json j = 0);
  std::shared_ptr<VideoSource> makePlaylistVideoSource(std::string name, ImVec2 origin = ImVec2(0., 0.), std::string id = UUID::generateUUID(), json j = 0);
  std::shared_ptr<VideoSource> makeTextVideoSource(std::string name, ImVec2 origin = ImVec2(0., 0.), std::string id = UUID::generateUUID(), json j = 0);
  std::shared_ptr<VideoSource> makeTypewriterTextVideoSource(std::string name, ImVec2 origin = ImVec2(0., 0.), std::string id = UUID::generateUUID(), json j = 0);
  std::shared_ptr<VideoSource> makeScrollingTextVideoSource(std::string name, ImVec2 origin = ImVec2(0., 0.), std::string id = UUID::generateUUID(), json j = 0);
  std::shared_ptr<VideoSource> makeIconVideoSource(std::string name, ImVec2 origin = ImVec2(0., 0.), std::string id = UUID::generateUUID(), json j = 0);
  std::shared_ptr<VideoSource> makeLibraryVideoSource(std::shared_ptr<LibraryFile> libraryFile, ImVec2 origin = ImVec2(0., 0.), std::string id = UUID::generateUUID(), json j = 0);
  
  // Output Windows

  std::map<std::string, std::shared_ptr<OutputWindow>> outputWindows;
  std::shared_ptr<OutputWindow> lastOutputWindow;
  void closeOutputWindow(std::shared_ptr<OutputWindow> window);
  void addOutputWindow(std::shared_ptr<Connectable> connectable);
  void updateOutputWindow(std::shared_ptr<Connectable> oldConnectable, std::shared_ptr<Connectable> newConnectable);
  bool hasOutputWindowForConnectable(std::shared_ptr<Connectable> connectable);
  void captureOutputWindowScreenshot();
  
  void clear();
  json config() override;
  void loadConfig(json j) override;
  std::vector<std::string> idsFromLoadingConfig(json j);
  void appendConfig(json j);

  std::vector<std::shared_ptr<AvailableVideoSourceShader>> availableShaderSources;
  std::vector<std::shared_ptr<AvailableVideoSourceShader>> availableRandomShaderSources;
};

#endif /* VideoSourceService_hpp */

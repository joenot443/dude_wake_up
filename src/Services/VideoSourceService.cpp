//
//  VideoSourceService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#include "VideoSourceService.hpp"
#include "UUID.hpp"
#include "NodeLayoutView.hpp"
#include "ConfigService.hpp"
#include "LibraryService.hpp"
#include "TextSource.hpp"
#include "WebcamSource.hpp"
#include "IconSource.hpp"
#include "ImageSource.hpp"
#include "ShaderChainerService.hpp"
#include "FileSource.hpp"

void VideoSourceService::setup()
{
  populateAvailableVideoSources();
  emptyFbo->allocate(1280, 720, GL_RGBA);
}

void VideoSourceService::populateAvailableVideoSources()
{
#ifdef TESTING
  // Return early while testing. We don't test AvailableVideoSources for now.
  return;
#endif
  availableSourceMap.clear();

  // Add an AvailableVideoSource for each Webcam and each ShaderType
  for (auto const &x : ofVideoGrabber().listDevices())
  {
    auto webcamSource = std::make_shared<AvailableVideoSourceWebcam>(x.deviceName, x.id);
    availableSourceMap[webcamSource->availableVideoSourceId] = webcamSource;
  }

  for (auto const &x : AvailableShaderSourceTypes)
  {
    auto shaderSource = std::make_shared<AvailableVideoSourceShader>(shaderSourceTypeName(x), shaderSourceTypeCategory(x), x);
    shaderSource->generatePreview();
    availableSourceMap[shaderSource->availableVideoSourceId] = shaderSource;
  }

  auto textSource = std::make_shared<AvailableVideoSourceText>("Basic Text");
  availableSourceMap[textSource->availableVideoSourceId] = textSource;
  
  auto iconSource = std::make_shared<AvailableVideoSourceIcon>("Icon");
  availableSourceMap[iconSource->availableVideoSourceId] = iconSource;

  for (auto const &file : LibraryService::getService()->libraryFiles)
  {
    // Create an AvailableLibraryVideoSource for each LibraryFile
    auto fileSource = std::make_shared<AvailableVideoSourceLibrary>(file->name, file);
    fileSource->generatePreview();
    availableSourceMap[fileSource->availableVideoSourceId] = fileSource;
  }

  availableVideoSourceUpdateSubject.notify();
}

void VideoSourceService::addAvailableVideoSources(std::vector<std::shared_ptr<AvailableVideoSource>> sources)
{
  for (auto const &source : sources)
  {
    availableSourceMap[source->availableVideoSourceId] = source;
  }
}

void VideoSourceService::subscribeToAvailableVideoSourceUpdates(std::function<void()> callback)
{
  availableVideoSourceUpdateSubject.subscribe(callback);
}

std::shared_ptr<ofFbo> VideoSourceService::previewFbo()
{
  std::shared_ptr<ofFbo> fbo = std::make_shared<ofFbo>();
  std::shared_ptr<ofFbo> blankFbo = std::make_shared<ofFbo>();
  fbo->allocate(426, 240);
  blankFbo->allocate(426, 240);

  auto shader = ShaderChainerService::getService()->shaderForType(
      ShaderTypeOctahedron, UUID::generateUUID(), 0);
  shader->setup();
  shader->shade(blankFbo, fbo);
  return fbo;
}

// Create a FeedbackSource for the passed in VideoSource and register it in the FeedbackSourceService
void VideoSourceService::createFeedbackSource(std::shared_ptr<VideoSource> videoSource)
{
  std::shared_ptr<FeedbackSource> feedbackSource = std::make_shared<FeedbackSource>(videoSource->id, std::shared_ptr<VideoSourceSettings>(videoSource->settings));
  FeedbackSourceService::getService()->registerFeedbackSource(feedbackSource);
  videoSource->feedbackDestination = feedbackSource;
}

// Iterate over every video source in the map and update them
void VideoSourceService::updateVideoSources()
{
  for (auto const &x : videoSourceMap)
  {

    x.second->saveFrame();
    x.second->saveFeedbackFrame();
  }
}

std::shared_ptr<VideoSource> VideoSourceService::defaultVideoSource()
{
  return videoSources().front();
}

std::shared_ptr<VideoSourceSettings> VideoSourceService::defaultVideoSourceSettings()
{
  return std::make_shared<VideoSourceSettings>(UUID::generateUUID(), 0);
}

// Add a video source to the map
void VideoSourceService::addVideoSource(std::shared_ptr<VideoSource> videoSource, std::string id, json j)
{
  videoSourceMap[id] = videoSource;
  createFeedbackSource(videoSource);
  if (j.is_object())
  {
    videoSource->load(j);
  }
  videoSource->setup();
}

// Remove a video source from the map
void VideoSourceService::removeVideoSource(std::string id)
{
  if (videoSourceMap.find(id) != videoSourceMap.end())
  {
    auto source = videoSourceMap[id];
    source->teardown();
    FeedbackSourceService::getService()->removeFeedbackSource(id);
    ShaderChainerService::getService()->removeConnectable(source);
    videoSourceMap.erase(id);
  }
  
  ConfigService::getService()->saveDefaultConfigFile();
}

// Return a vector of all video sources
std::vector<std::shared_ptr<VideoSource>> VideoSourceService::videoSources()
{
  std::vector<std::shared_ptr<VideoSource>> videoSources;
  for (auto const &x : videoSourceMap)
  {
    if (x.second != nullptr)
    {
      videoSources.push_back(x.second);
    }
  }
  return videoSources;
}

// Returns a shared_ptr to the available video source with the passed in id
std::shared_ptr<AvailableVideoSource> VideoSourceService::availableVideoSourceForId(std::string id)
{
  if (availableSourceMap.find(id) != availableSourceMap.end())
  {
    return availableSourceMap[id];
  }
  return nullptr;
}

// Return a vector of all AvailableVideoSource
std::vector<std::shared_ptr<AvailableVideoSource>> VideoSourceService::availableVideoSources()
{
  std::vector<std::shared_ptr<AvailableVideoSource>> videoSources;
  for (auto const &x : availableSourceMap)
  {
    if (x.second != nullptr)
    {
      videoSources.push_back(x.second);
    }
  }
  return videoSources;
}

// Return a vector of all video sources which should be used as inputs (non empty, non ShaderChainer)

std::vector<std::shared_ptr<VideoSource>> VideoSourceService::inputSources()
{
  std::vector<std::shared_ptr<VideoSource>> videoSources;
  for (auto const &x : videoSourceMap)
  {
    if (x.second != nullptr && !(x.second->type == VideoSource_chainer))
    {
      videoSources.push_back(x.second);
    }
  }
  return videoSources;
}

// Return a vector of the names of available webcams
std::vector<std::string> VideoSourceService::getWebcamNames()
{
  std::vector<std::string> webcamNames;
  for (auto const &x : ofVideoGrabber().listDevices())
  {
    webcamNames.push_back(x.deviceName);
  }
  return webcamNames;
}

// Adds a webcam video source to the map
std::shared_ptr<VideoSource> VideoSourceService::addWebcamVideoSource(std::string name, int deviceId, ImVec2 origin, std::string id, json j)
{
  std::shared_ptr<WebcamSource> videoSource = std::make_shared<WebcamSource>(id, name);
  videoSource->origin = origin;
  videoSource->settings->deviceId->intValue = deviceId;
//  if (j["settings"].is_object()) {
//    videoSource->settings->load(j["settings"]);
//  }

  addVideoSource(videoSource, id, j);
  return videoSource;
}

// Adds a file video source to the map
std::shared_ptr<VideoSource> VideoSourceService::addFileVideoSource(std::string name, std::string path, ImVec2 origin, std::string id, json j)
{
  std::shared_ptr<VideoSource> videoSource = std::make_shared<FileSource>(id, name, path);
  videoSource->origin = origin;
  addVideoSource(videoSource, id, j);
  return videoSource;
}

// Adds an icon video source to the map
std::shared_ptr<VideoSource> VideoSourceService::addIconVideoSource(std::string name, ImVec2 origin, std::string id, json j)
{
  std::shared_ptr<VideoSource> videoSource = std::make_shared<IconSource>(id, "Icon");
  videoSource->origin = origin;
  addVideoSource(videoSource, id, j);
  return videoSource;
}


// Adds a Shader video source to the map
std::shared_ptr<VideoSource> VideoSourceService::addShaderVideoSource(ShaderSourceType type, ImVec2 origin, std::string id, json j)
{
  std::shared_ptr<ShaderSource> shaderSource = std::make_shared<ShaderSource>(id, type);
  // Check if "shader" exists in the json, if so, load it into the shaderSource
  if (j.count("shader"))
  {
    shaderSource->shader->settings->load(j["shader"]);
    shaderSource->shader->settings->registerParameters();
  }
  auto videoSource = std::dynamic_pointer_cast<VideoSource>(shaderSource);

  videoSource->origin = origin;
  addVideoSource(videoSource, id, j);
  return videoSource;
}

// Adds an Image video source to the map
std::shared_ptr<VideoSource> VideoSourceService::addImageVideoSource(std::string name, std::string path, ImVec2 origin, std::string id, json j)
{
  std::shared_ptr<VideoSource> videoSource = std::make_shared<ImageSource>(id, name, path);
  videoSource->load(j);
  videoSource->origin = origin;
  addVideoSource(videoSource, id, j);
  return videoSource;
}

// Adds a Text video source to the map
std::shared_ptr<VideoSource> VideoSourceService::addTextVideoSource(std::string name, ImVec2 origin, std::string id, json j)
{
  auto displayText = std::make_shared<DisplayText>();
  auto textSource = TextSource(id, name, displayText);
  textSource.load(j);
  std::shared_ptr<VideoSource> videoSource = std::make_shared<TextSource>(textSource);
  videoSource->origin = origin;
  addVideoSource(videoSource, id, j);
  return videoSource;
}

// Adds an OutputWindow for the passed in VideoSource
void VideoSourceService::addOutputWindow(std::shared_ptr<Connectable> connectable)
{
  std::shared_ptr<OutputWindow> outputWindow = std::make_shared<OutputWindow>(connectable->frame(), connectable);
  outputWindow->setup();
  ofGLFWWindowSettings settings;
  settings.shareContextWith = ofGetCurrentWindow();
  settings.setSize(connectable->frame()->getWidth(), connectable->frame()->getHeight());
  settings.setGLVersion(3, 2);
  auto streamWindow = ofCreateWindow(settings);
  ofRunApp(streamWindow, outputWindow);
  outputWindows[connectable->connId()] = outputWindow;
}

void VideoSourceService::updateOutputWindow(std::shared_ptr<Connectable> oldConnectable, std::shared_ptr<Connectable> newConnectable)
{
  if (outputWindows.count(oldConnectable->connId()) == 0)
    return;

  std::shared_ptr<OutputWindow> outputWindow = outputWindows[oldConnectable->connId()];
  outputWindow->fbo = newConnectable->frame();
  outputWindow->connectable = newConnectable;
  outputWindows.erase(oldConnectable->connId());

  outputWindows[newConnectable->connId()] = outputWindow;
}

bool VideoSourceService::hasOutputWindowForConnectable(std::shared_ptr<Connectable> connectable)
{
  return outputWindows.count(connectable->connId()) != 0;
}

void VideoSourceService::closeOutputWindow(std::shared_ptr<OutputWindow> window) {
  outputWindows.erase(window->connectable->connId());
}
// ConfigurableService

json VideoSourceService::config()
{
  auto sources = videoSources();
  json container;

  for (auto source : sources)
  {
    // Don't serialize Chainer video sources, they'll be added in the ShaderChainerService
    if (source->type == VideoSource_chainer)
    {
      continue;
    }
    // Don't serialize an Empty source
    if (source->type == VideoSource_shader && std::dynamic_pointer_cast<ShaderSource>(source)->shaderSourceType == ShaderSource_empty)
    {
      continue;
    }
    if (NodeLayoutView::getInstance()->nodeForShaderSourceId(source->id) == nullptr)
    {
      continue;
    }
    container[source->id] = source->serialize();
  }

  // Check if the container is an object and log its value count
  int size = container.size();
  if (container.is_object() && container.size() == 0)
  {
    ofLogNotice() << "VideoSourceService::config() container has " << container.size() << " values";
  }

  return container;
}

void VideoSourceService::appendConfig(json j)
{
  std::map<std::string, json> sourcesMap = j;

  VideoSourceType type = j["videoSourceType"];
  std::string sourceId = j["id"];
  ImVec2 position = ImVec2(j["x"], j["y"]);

  switch (type)
  {
  case VideoSource_file:
    addFileVideoSource(j["sourceName"], j["path"], position, sourceId, j);
    return;
  case VideoSource_webcam:
    addWebcamVideoSource(j["sourceName"], 0, position, sourceId, j);
    return;
  case VideoSource_shader:
    addShaderVideoSource(j["shaderSourceType"], position, sourceId, j);
    return;
  case VideoSource_chainer:
    return;
  case VideoSource_image:
    addImageVideoSource(j["sourceName"], j["path"], position, sourceId, j);
    return;
  case VideoSource_text:
    addTextVideoSource(j["sourceName"], position, sourceId, j);
      return;
    case VideoSource_icon:
    addIconVideoSource(j["sourceName"], position, sourceId, j);
      return;
  case VideoSource_empty:
    break;
  }
}

void VideoSourceService::clear()
{
  for (auto it = videoSourceMap.begin(); it != videoSourceMap.end();)
  {
    auto key = it->first;
    auto source = videoSourceMap[key];
    source->teardown();
    FeedbackSourceService::getService()->removeFeedbackSource(key);
    it = videoSourceMap.erase(it);
  }

  videoSourceMap.clear();
  outputWindows.clear();
}

void VideoSourceService::loadConfig(json data)
{
  idsFromLoadingConfig(data);
}


std::vector<std::string> VideoSourceService::idsFromLoadingConfig(json j)
{
  std::map<std::string, json> sourceMap = j;
  std::vector<std::string> ids;

  for (auto pair : sourceMap)
  {
    std::map<std::string, json> source = pair.second;
    appendConfig(source);
    ids.push_back(pair.first);
  }
  
  return ids;
}

std::shared_ptr<VideoSource> VideoSourceService::videoSourceForId(std::string id)
{
  if (videoSourceMap.count(id))
  {
    return videoSourceMap[id];
  }
  return nullptr;
}

//
//  VideoSourceService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#include "VideoSourceService.hpp"
#include "UUID.hpp"
#include "NodeLayoutView.hpp"
#include "LibraryService.hpp"
#include "TextSource.hpp"
#include "WebcamSource.hpp"
#include "ImageSource.hpp"
#include "ShaderChainerService.hpp"
#include "FileSource.hpp"

void VideoSourceService::setup()
{
  populateAvailableVideoSources();
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
    auto shaderSource = std::make_shared<AvailableVideoSourceShader>(shaderSourceTypeName(x), x);
    shaderSource->generatePreview();
    availableSourceMap[shaderSource->availableVideoSourceId] = shaderSource;
  }

  auto textSource = std::make_shared<AvailableVideoSourceText>("Basic Text");
  availableSourceMap[textSource->availableVideoSourceId] = textSource;

  for (auto const &file : LibraryService::getService()->libraryFiles)
  {
    // Create an AvailableLibraryVideoSource for each LibraryFile
    auto fileSource = std::make_shared<AvailableVideoSourceLibrary>(file->name, file);
    fileSource->generatePreview();
    availableSourceMap[fileSource->availableVideoSourceId] = fileSource;
  }

  availableVideoSourceUpdateSubject.notify();
}

void VideoSourceService::subscribeToAvailableVideoSourceUpdates(std::function<void()> callback)
{
  availableVideoSourceUpdateSubject.subscribe(callback);
}

ofFbo VideoSourceService::previewFbo()
{
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
void VideoSourceService::createFeedbackSource(std::shared_ptr<VideoSource> videoSource)
{
  std::shared_ptr<FeedbackSource> feedbackSource = std::make_shared<FeedbackSource>(videoSource->id, videoSource->sourceName, std::make_shared<VideoSourceSettings>(videoSource->settings));
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

// Add a video source to the map
void VideoSourceService::addVideoSource(std::shared_ptr<VideoSource> videoSource, std::string id)
{
  videoSourceMap[id] = videoSource;
  createFeedbackSource(videoSource);
  videoSource->setup();
}

// Remove a video source from the map
void VideoSourceService::removeVideoSource(std::string id)
{
  if (videoSourceMap.find(id) != videoSourceMap.end())
  {
    auto source = videoSourceMap[id];
    source->teardown();
    videoSourceMap.erase(id);
  }

  FeedbackSourceService::getService()->removeFeedbackSource(id);
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
std::shared_ptr<VideoSource> VideoSourceService::addWebcamVideoSource(std::string name, int deviceId, ImVec2 origin, std::string id)
{
  std::shared_ptr<VideoSource> videoSource = std::make_shared<WebcamSource>(id, name, deviceId);
  videoSource->origin = origin;
  addVideoSource(videoSource, id);
  return videoSource;
}

// Adds a file video source to the map
std::shared_ptr<VideoSource> VideoSourceService::addFileVideoSource(std::string name, std::string path, ImVec2 origin, std::string id)
{
  std::shared_ptr<VideoSource> videoSource = std::make_shared<FileSource>(id, name, path);
  videoSource->origin = origin;
  addVideoSource(videoSource, id);
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
  }
  auto videoSource = std::dynamic_pointer_cast<VideoSource>(shaderSource);

  videoSource->origin = origin;
  addVideoSource(videoSource, id);
  return videoSource;
}

// Adds an Image video source to the map
std::shared_ptr<VideoSource> VideoSourceService::addImageVideoSource(std::string name, std::string path, ImVec2 origin, std::string id, json j)
{
  std::shared_ptr<VideoSource> videoSource = std::make_shared<ImageSource>(id, name, path);
  videoSource->load(j);
  videoSource->origin = origin;
  addVideoSource(videoSource, id);
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
  addVideoSource(videoSource, id);
  return videoSource;
}

// Adds an OutputWindow for the passed in VideoSource
void VideoSourceService::addOutputWindowForChainer(std::shared_ptr<ShaderChainer> chainer)
{
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
    addFileVideoSource(j["sourceName"], j["path"], position, sourceId);
    return;
  case VideoSource_webcam:
    addWebcamVideoSource(j["sourceName"], j["deviceId"], position, sourceId);
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
    // TODO
    break;
  case VideoSource_empty:
    break;
  }

  // Create an implicit
}

void VideoSourceService::clear()
{
  for (auto it = videoSourceMap.begin(); it != videoSourceMap.end();)
  {
    auto key = it->first;
    removeVideoSource(key);
    it = videoSourceMap.erase(it);
  }

  videoSourceMap.clear();
  outputWindows.clear();
}

void VideoSourceService::loadConfig(json data)
{
  std::map<std::string, json> sourceMap = data;

  for (auto pair : sourceMap)
  {
    std::map<std::string, json> source = pair.second;
    appendConfig(source);
  }

  addShaderVideoSource(ShaderSource_empty);
}

std::shared_ptr<VideoSource> VideoSourceService::videoSourceForId(std::string id)
{
  if (videoSourceMap.count(id))
  {
    return videoSourceMap[id];
  }
  return nullptr;
}

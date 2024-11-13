//
//  VideoSourceService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#include <CoreFoundation/CoreFoundation.h>
#include <Security/SecItem.h>
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
#include "BookmarkService.hpp"
#include "LayoutStateService.hpp"
#include "FileSource.hpp"
#include <cstdlib>


void VideoSourceService::setup()
{
  populateAvailableVideoSources();
  emptyFbo->allocate(1280, 720, GL_RGBA);
}

void VideoSourceService::populateAvailableVideoSources()
{
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
    auto fileSource = std::make_shared<AvailableVideoSourceLibrary>(file.second->name, file.second);
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
    // If inactive, don't save the next frame
    if (!x.second->active) continue;
    
    x.second->saveFrame();
    x.second->applyOptionalShaders();
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
  videoSource->generateOptionalShaders();
  // Subscribe the Shader's setup() to resolution updates
  LayoutStateService::getService()->subscribeToResolutionUpdates([videoSource]() {
    videoSource->setup();
  });
  
}

// Remove a video source from the map
void VideoSourceService::removeVideoSource(std::string id)
{
  if (videoSourceMap.find(id) != videoSourceMap.end())
  {
    auto source = videoSourceMap[id];
    source->teardown();
    OscillationService::getService()->removeOscillatorsFor(source->settings->parameters);
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
std::shared_ptr<VideoSource> VideoSourceService::makeWebcamVideoSource(std::string name, int deviceId, ImVec2 origin, std::string id, json j)
{
  std::shared_ptr<WebcamSource> videoSource = std::make_shared<WebcamSource>(id, name);
  videoSource->origin = origin;
  videoSource->settings->deviceId->intValue = deviceId;
  
  return videoSource;
}

// Adds a file video source to the map
std::shared_ptr<VideoSource> VideoSourceService::makeFileVideoSource(std::string name, std::string path, ImVec2 origin, std::string id, json j)
{
  // Get the bookmark service instance
  BookmarkService* bookmarkService = BookmarkService::getService();
  std::shared_ptr<VideoSource> videoSource = nullptr;
  
  // Use the new bookmarkForPath method
  auto bookmarkString = bookmarkService->bookmarkForPath(path);
  if (!bookmarkString) {
    log("Failed to create or retrieve bookmark for path: %s", path.c_str());
    return nullptr;  // Return early on error
  }
  
  // Resolve the bookmark to get the file URL
  CFDataRef bookmarkData = CFDataCreate(kCFAllocatorDefault, reinterpret_cast<const UInt8*>(bookmarkString->c_str()), bookmarkString->length());
  Boolean isStale = false;
  CFErrorRef error = nullptr;
  CFURLRef fileURL = CFURLCreateByResolvingBookmarkData(
    kCFAllocatorDefault,        // Allocator
    bookmarkData,               // Bookmark data
    kCFURLBookmarkResolutionWithSecurityScope,  // Options (or 0 if no security scope needed)
    nullptr,                    // relativeToURL (can be nullptr)
    nullptr,                    // resourcePropertiesToInclude (can be nullptr)
    &isStale,                   // isStale flag
    &error                      // CFErrorRef for error reporting
  );
  
  // Check if fileURL is resolved successfully
  if (!fileURL) {
    log("Failed to resolve bookmark data for path: %s", path.c_str());
    if (error) {
      CFStringRef errorDesc = CFErrorCopyDescription(error);
      char buffer[256];
      CFStringGetCString(errorDesc, buffer, sizeof(buffer), kCFStringEncodingUTF8);
      log("Bookmark Resolution Error", buffer);
      CFRelease(errorDesc);
      CFRelease(error);  // Release error reference
    }
    CFRelease(bookmarkData);  // Release bookmarkData before returning
    return nullptr;
  }
  
  // Start accessing the security-scoped resource
  Boolean success = CFURLStartAccessingSecurityScopedResource(fileURL);
  if (!success) {
    log("Failed to access security-scoped resource for URL: %s", path.c_str());
    CFRelease(fileURL);     // Release fileURL
    CFRelease(bookmarkData); // Release bookmarkData
    return nullptr;  // Return early on error
  }
  
  // At this point, we have successfully resolved the bookmark and accessed the resource
  videoSource = std::make_shared<FileSource>(id, name, path);
  videoSource->origin = origin;
  
  // Release CF objects
  CFRelease(fileURL);
  CFRelease(bookmarkData);
  
  return videoSource;
}

// Adds an icon video source to the map
std::shared_ptr<VideoSource> VideoSourceService::makeIconVideoSource(std::string name, ImVec2 origin, std::string id, json j)
{
  std::shared_ptr<VideoSource> videoSource = std::make_shared<IconSource>(id, "Icon");
  videoSource->origin = origin;
  return videoSource;
}

// Adds a multi video source to the map
std::shared_ptr<MultiSource> VideoSourceService::makeMultiVideoSource(std::string name, ImVec2 origin, std::string id, json j)
{
  std::shared_ptr<MultiSource> videoSource = std::make_shared<MultiSource>(id, "Multi");
  videoSource->origin = origin;
  return videoSource;
}


// Adds a Shader video source to the map
std::shared_ptr<VideoSource> VideoSourceService::makeShaderVideoSource(ShaderSourceType type, ImVec2 origin, std::string id, json j)
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
  return videoSource;
}

// Adds an Image video source to the map
std::shared_ptr<VideoSource> VideoSourceService::makeImageVideoSource(std::string name, std::string path, ImVec2 origin, std::string id, json j)
{
  // Get the bookmark service instance
  BookmarkService* bookmarkService = BookmarkService::getService();
  std::shared_ptr<VideoSource> videoSource = nullptr;
  
  // Use the new bookmarkForPath method
  auto bookmarkString = bookmarkService->bookmarkForPath(path);
  if (!bookmarkString) {
    log("Failed to create or retrieve bookmark for path: %s", path.c_str());
    return nullptr;  // Return early on error
  }
  
  // Resolve the bookmark to get the file URL
  CFDataRef bookmarkData = CFDataCreate(kCFAllocatorDefault, reinterpret_cast<const UInt8*>(bookmarkString->c_str()), bookmarkString->length());
  Boolean isStale = false;
  CFErrorRef error = nullptr;
  CFURLRef fileURL = CFURLCreateByResolvingBookmarkData(
    kCFAllocatorDefault,        // Allocator
    bookmarkData,               // Bookmark data
    kCFURLBookmarkResolutionWithSecurityScope,  // Options (or 0 if no security scope needed)
    nullptr,                    // relativeToURL (can be nullptr)
    nullptr,                    // resourcePropertiesToInclude (can be nullptr)
    &isStale,                   // isStale flag
    &error                      // CFErrorRef for error reporting
  );
  
  // Check if fileURL is resolved successfully
  if (!fileURL) {
    log("Failed to resolve bookmark data for path: %s", path.c_str());
    if (error) {
      CFStringRef errorDesc = CFErrorCopyDescription(error);
      char buffer[256];
      CFStringGetCString(errorDesc, buffer, sizeof(buffer), kCFStringEncodingUTF8);
      log("Bookmark Resolution Error", buffer);
      CFRelease(errorDesc);
      CFRelease(error);  // Release error reference
    }
    CFRelease(bookmarkData);  // Release bookmarkData before returning
    return nullptr;
  }
  
  // Start accessing the security-scoped resource
  Boolean success = CFURLStartAccessingSecurityScopedResource(fileURL);
  if (!success) {
    log("Failed to access security-scoped resource for URL: %s", path.c_str());
    CFRelease(fileURL);     // Release fileURL
    CFRelease(bookmarkData); // Release bookmarkData
    return nullptr;  // Return early on error
  }
  
  // At this point, we have successfully resolved the bookmark and accessed the resource
  videoSource = std::make_shared<ImageSource>(id, name, path);
  videoSource->load(j);
  videoSource->origin = origin;
  
  // Release CF objects
  CFRelease(fileURL);
  CFRelease(bookmarkData);
  
  return videoSource;
}

// Adds a Text video source to the map
std::shared_ptr<VideoSource> VideoSourceService::makeTextVideoSource(std::string name, ImVec2 origin, std::string id, json j)
{
  auto displayText = std::make_shared<DisplayText>();
  auto textSource = TextSource(id, name, displayText);
  textSource.load(j);
  std::shared_ptr<VideoSource> videoSource = std::make_shared<TextSource>(textSource);
  videoSource->origin = origin;
  return videoSource;
}

// Adds a Library video source to the map
std::shared_ptr<VideoSource> VideoSourceService::makeLibraryVideoSource(std::shared_ptr<LibraryFile> libraryFile, ImVec2 origin, std::string id, json j)
{
  std::shared_ptr<VideoSource> videoSource = std::make_shared<LibrarySource>(id, libraryFile);
  videoSource->origin = origin;
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
  lastOutputWindow = outputWindow;
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
  lastOutputWindow = outputWindow;
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
    //    if (NodeLayoutView::getInstance()->nodeForShaderSourceId(source->id) == nullptr)
    //    {
    //      continue;
    //    }
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
  std::shared_ptr<VideoSource> source;
  switch (type)
  {
    case VideoSource_empty:
      break;
    case VideoSource_file:
      source = makeFileVideoSource(j["sourceName"], j["path"], position, sourceId, j);
      break;
    case VideoSource_webcam:
      source = makeWebcamVideoSource(j["sourceName"], 0, position, sourceId, j);
      break;
    case VideoSource_shader:
      source = makeShaderVideoSource(j["shaderSourceType"], position, sourceId, j);
      break;
    case VideoSource_chainer:
      break;
    case VideoSource_image:
      source = makeImageVideoSource(j["sourceName"], j["path"], position, sourceId, j);
      break;
    case VideoSource_text:
      source = makeTextVideoSource(j["sourceName"], position, sourceId, j);
      break;
    case VideoSource_icon:
      source = makeIconVideoSource(j["sourceName"], position, sourceId, j);
      break;
    case VideoSource_library:
      std::shared_ptr<LibraryFile> libraryFile = LibraryService::getService()->libraryFileForId(j["libraryFileId"]);
      source = makeLibraryVideoSource(libraryFile, position, sourceId, j);
  }
  if (source != nullptr)
  	addVideoSource(source, sourceId);
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

void VideoSourceService::captureOutputWindowScreenshot()
{
  if (lastOutputWindow == nullptr) return;
  
  std::shared_ptr<ofFbo> fbo = lastOutputWindow->fbo;
  
  // Create an ofImage to store the pixels
  ofImage image;
  
  // Allocate the image with the dimensions of the FBO
  image.allocate(fbo->getWidth(), fbo->getHeight(), OF_IMAGE_COLOR);
  
  // Bind the FBO and read its pixels
  fbo->bind();
  fbo->readToPixels(image.getPixels());
  fbo->unbind();
  
  // Construct the full file path for the screenshot
  std::string filePath = ConfigService::getService()->exportsFolderFilePath() + "/nottawa_" + ofGetTimestampString() + ".jpg";
  
  // Save the image to the specified file path
  if (ofSaveImage(image.getPixels(), filePath)) {
    ofLog() << "Screenshot saved to " << filePath;
    
    std::string command = "open " + ConfigService::getService()->exportsFolderFilePath();
    std::system(command.c_str());
  } else {
    ofLog() << "Failed";
  }
}

void VideoSourceService::startAccessingBookmarkPath(std::string path)
{
  // Get the bookmark service instance
  BookmarkService* bookmarkService = BookmarkService::getService();
  
  // Use the bookmarkForPath method to get the bookmark string
  auto bookmarkString = bookmarkService->bookmarkForPath(path);
  if (!bookmarkString) {
    log("Failed to create or retrieve bookmark for path: %s", path.c_str());
    return;  // Return early on error
  }
  
  // Convert the bookmark string to CFDataRef
  CFDataRef bookmarkData = CFDataCreate(kCFAllocatorDefault, reinterpret_cast<const UInt8*>(bookmarkString->c_str()), bookmarkString->length());
  Boolean isStale = false;
  CFErrorRef error = nullptr;
  
  // Resolve the bookmark to get the file URL
  CFURLRef fileURL = CFURLCreateByResolvingBookmarkData(
    kCFAllocatorDefault,        // Allocator
    bookmarkData,               // Bookmark data
    kCFURLBookmarkResolutionWithSecurityScope,  // Options (or 0 if no security scope needed)
    nullptr,                    // relativeToURL (can be nullptr)
    nullptr,                    // resourcePropertiesToInclude (can be nullptr)
    &isStale,                   // isStale flag
    &error                      // CFErrorRef for error reporting
  );
  
  // Check if fileURL is resolved successfully
  if (!fileURL) {
    log("Failed to resolve bookmark data for path: %s", path.c_str());
    if (error) {
      CFStringRef errorDesc = CFErrorCopyDescription(error);
      char buffer[256];
      CFStringGetCString(errorDesc, buffer, sizeof(buffer), kCFStringEncodingUTF8);
      log("Bookmark Resolution Error", buffer);
      CFRelease(errorDesc);
      CFRelease(error);  // Release error reference
    }
    CFRelease(bookmarkData);  // Release bookmarkData before returning
    return;
  }
  
  // Start accessing the security-scoped resource
  Boolean success = CFURLStartAccessingSecurityScopedResource(fileURL);
  if (!success) {
    log("Failed to access security-scoped resource for URL: %s", path.c_str());
  }
  
  // Release CF objects
  CFRelease(fileURL);
  CFRelease(bookmarkData);
}

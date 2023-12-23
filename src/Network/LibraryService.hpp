//
//  LibraryService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/20/23.
//

#ifndef LibraryService_hpp
#define LibraryService_hpp

#include <stdio.h>
#include "ofMain.h"
#include "Feedback.hpp"
#include "LibraryFile.hpp"
#include "ShaderChainer.hpp"
#include "observable.hpp"
#include <future>

enum DownloadState
{
  DownloadState_NotDownloaded,
  DownloadState_Downloading,
  DownloadState_Downloaded,
  DownloadState_Error
};

class LibraryService
{
public:
  static LibraryService *service;
  LibraryService(){};

  static LibraryService *getService()
  {
    if (!service)
    {
      service = new LibraryService;
      service->setup();
    }
    return service;
  }

  void setup();
  void fetchLibraryFiles();
  void backgroundFetchLibraryFiles();
  void didFetchLibraryFiles();
  
  void repopulateVideoSourcesFromMainThread();
  
  void uploadChainer(const std::string &name, const std::string &author, const std::shared_ptr<ShaderChainer> shaderChainer, std::function<void()> success_callback, std::function<void(const std::string &)> error_callback);

  void submitFeedback(Feedback feedback, std::function<void()> success_callback, std::function<void(const std::string &)> error_callback);

  void downloadThumbnail(std::shared_ptr<LibraryFile> file);
  void downloadAllThumbnails();
  void downloadFile(std::shared_ptr<LibraryFile> file);

  bool hasThumbnail(std::shared_ptr<LibraryFile> file);

  bool hasMedia(std::shared_ptr<LibraryFile> file);

  observable::subject<void()> libraryThumbnailUpdateSubject;

  std::vector<std::shared_ptr<LibraryFile>> libraryFiles;

  std::vector<std::future<void>> downloadFutures;

  ofEvent<LibraryFile> downloadNotification;
  ofEvent<void> thumbnailNotification;
};

#endif /* LibraryService_hpp */

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
#include "ShaderType.hpp"
#include "Credit.hpp"
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

  void submitFeedback(Feedback feedback, std::function<void()> success_callback, std::function<void(const std::string &)> error_callback);

  void downloadThumbnail(std::shared_ptr<LibraryFile> file);
  void downloadAllThumbnails();
  void downloadFile(std::shared_ptr<LibraryFile> file, std::function<void()>);
  
  std::shared_ptr<LibraryFile> libraryFileForId(std::string id);

  bool hasThumbnailOnDisk(std::shared_ptr<LibraryFile> file);

  bool hasMediaOnDisk(std::shared_ptr<LibraryFile> file);

  // Credits functionality
  void fetchShaderCredits();
  void backgroundFetchShaderCredits();
  Credit* getShaderCredit(ShaderType type);
  bool hasCredit(ShaderType type);

  observable::subject<void()> libraryThumbnailUpdateSubject;

  std::map<std::string, std::shared_ptr<LibraryFile>> libraryFiles;
  
  std::map<std::string, bool> libraryFileIdDownloadedMap;

  // Shader credits data
  std::map<ShaderType, Credit> shaderCredits;

  std::vector<std::future<void>> downloadFutures;

  ofEvent<LibraryFile> downloadNotification;
  ofEvent<void> thumbnailNotification;
};

#endif /* LibraryService_hpp */

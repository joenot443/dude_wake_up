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
#include "LibraryFile.hpp"
#include "ShaderType.hpp"
#include "Credit.hpp"
#include <memory>
#include "observable.hpp"
#include <algorithm>
#include <chrono>
#include <future>
#include <mutex>
#include <vector>

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

  void downloadThumbnail(std::shared_ptr<LibraryFile> file);
  void downloadAllThumbnails();
  void downloadFile(std::shared_ptr<LibraryFile> file, std::function<void()>);

  void pauseDownload(std::shared_ptr<LibraryFile> file);
  void resumeDownload(std::shared_ptr<LibraryFile> file, std::function<void()> callback);

  std::shared_ptr<LibraryFile> libraryFileForId(std::string id);
  // Thread-safe views of the library: fetches and downloads run on worker
  // threads while the UI reads.
  std::vector<std::shared_ptr<LibraryFile>> libraryFilesSnapshot();
  bool hasLibraryFiles();
  void setDownloaded(const std::string &id, bool downloaded);
  bool isDownloaded(const std::string &id);

  bool hasThumbnailOnDisk(std::shared_ptr<LibraryFile> file);

  bool hasMediaOnDisk(std::shared_ptr<LibraryFile> file);

  // Credits functionality
  void fetchShaderCredits();
  void backgroundFetchShaderCredits();
  std::shared_ptr<Credit> getShaderCredit(ShaderType type);
  bool hasCredit(ShaderType type);

  // Feedback
  void submitFeedback(const std::string &text,
                      const std::string &stateJson,
                      const std::string &screenshotData,
                      const std::string &author,
                      const std::string &email,
                      std::function<void(bool success, const std::string &error)> callback);

  // Strand Sharing
  // Shares a strand publicly and returns the URL via callback
  // previewData: PNG image data for the preview
  // videoData: MOV video data (optional, empty string to skip)
  void shareStrand(const std::string &title,
                   const std::string &description,
                   const std::string &strandJson,
                   const std::string &previewData,
                   const std::string &videoData,
                   const std::string &author,
                   std::function<void(bool success, const std::string &url, const std::string &error)> callback);

  // Fetches a shared strand by slug and returns the strand JSON via callback
  void fetchSharedStrand(const std::string &slug,
                         std::function<void(bool success, const std::string &strandJson, const std::string &title, const std::string &error)> callback);

  observable::subject<void()> libraryThumbnailUpdateSubject;

  // Guarded by filesMutex; use the accessors above from other threads.
  std::map<std::string, std::shared_ptr<LibraryFile>> libraryFiles;
  std::map<std::string, bool> libraryFileIdDownloadedMap;
  std::mutex filesMutex;

  // Shader credits data
  std::map<ShaderType, std::shared_ptr<Credit>> shaderCredits;

  // Runs a task on its own thread, keeping its future (guarded, since
  // tasks are started from worker threads as well as the main thread).
  template <class... Args>
  void launchAsync(Args &&...args)
  {
    std::lock_guard<std::mutex> lock(futuresMutex);
    // Drop finished tasks so the list doesn't grow without bound.
    downloadFutures.erase(std::remove_if(downloadFutures.begin(), downloadFutures.end(),
                                         [](std::future<void> &f) {
                                           return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
                                         }),
                          downloadFutures.end());
    downloadFutures.push_back(std::async(std::launch::async, std::forward<Args>(args)...));
  }
  std::vector<std::future<void>> downloadFutures;
  std::mutex futuresMutex;

  ofEvent<LibraryFile> downloadNotification;
  ofEvent<void> thumbnailNotification;
  std::mutex shaderCreditsMutex;
};

#endif /* LibraryService_hpp */

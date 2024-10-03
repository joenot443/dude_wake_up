//
//  LibraryService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/20/23.
//

#include "LibraryService.hpp"
#include "VideoSourceService.hpp"
#include "ConfigService.hpp"
#include "MainApp.h"
#include "Console.hpp"
#include "httplib.h"
#include "base64.h"
#include <sentry.h>
#include <cpr/cpr.h>

const static std::string API_URL = "http://165.227.44.1";
//const static std::string API_URL = "localhost:6000";

void LibraryService::setup()
{
}

//void LibraryService::uploadChainer(const std::string &name, const std::string &author, const std::shared_ptr<ShaderChainer> chainer, std::function<void()> success_callback, std::function<void(const std::string &)> error_callback)
//{
//  // Encode chainer data to base64
//  std::string chainer_base64 = httplib::detail::base64_encode(chainer->serialize().dump());
//
//  // Prepare the JSON body using nlohmann/json
//  nlohmann::json json_body;
//  json_body["name"] = name;
//  json_body["author"] = author;
//  json_body["chainer"] = chainer_base64;
//
//  // Create an httplib client instance
//  httplib::Client client(API_URL, 6000);
//
//  // Send POST request
//  auto res = client.Post("/chainers/new", json_body.dump(), "application/json");
//
//  if (res && res->status == 200)
//  {
//    std::cout << "Successful chainer upload." << std::endl;
//    success_callback();
//  }
//  else
//  {
//    std::string error_message = "Error: Failed to upload chainer";
//    error_callback(error_message);
//  }
//}

void LibraryService::backgroundFetchLibraryFiles()
{
  downloadFutures.push_back(std::async(std::launch::async, &LibraryService::fetchLibraryFiles, this));
}

void LibraryService::didFetchLibraryFiles() {
  downloadFutures.push_back(std::async(std::launch::async, &LibraryService::downloadAllThumbnails, this));
  repopulateVideoSourcesFromMainThread();
}

void LibraryService::repopulateVideoSourcesFromMainThread() {
  MainApp::getApp()->executeOnMainThread([]() {
    VideoSourceService::getService()->populateAvailableVideoSources();
  });
}

// Fetches the library files from the server at /api/media.
// This is a GET request that returns a JSON array of objects of the form:
// {
//   "name": "name",
//   "url": "url",
//   "thumbnailUrl": "thumbnailUrl"
// }
void LibraryService::fetchLibraryFiles()
{
  // Send GET request
  cpr::Response res = cpr::Get(cpr::Url{API_URL + ":6000/api/media"});

  if (res.status_code == 200)
  {
    // Parse the response body as JSON
    nlohmann::json json_body = nlohmann::json::parse(res.text);

    // Clear the library files vector
    libraryFiles.clear();

    // Iterate over the JSON array and add each object to the library files vector
    for (auto &library_file : json_body)
    {
      LibraryFile file;
      file.id = library_file["id"];
      file.name = library_file["name"];
      file.url = library_file["url"];
      file.thumbnailFilename = library_file["thumbnailFilename"];
      file.thumbnailUrl = library_file["thumbnailUrl"];
      file.filename = library_file["filename"];
      file.category = library_file["category"];
      
      auto shared = std::make_shared<LibraryFile>(file);

      
      libraryFileIdDownloadedMap[file.id] = hasMediaOnDisk(shared);
      libraryFiles[file.id] = shared;
      
      
    }
    didFetchLibraryFiles();
  }
  else
  {
    std::cout << "Error: Failed to fetch library files" << std::endl;
  }
}

bool LibraryService::hasThumbnailOnDisk(std::shared_ptr<LibraryFile> file)
{
  return ofFile::doesFileExist(file->thumbnailPath());
}

bool LibraryService::hasMediaOnDisk(std::shared_ptr<LibraryFile> file)
{
  return ofFile::doesFileExist(file->videoPath());
}

// Downloads the .jpg stored at file.thumbnailUrl, then stores it in
// ConfigService::libraryFolderFilePath
void LibraryService::downloadThumbnail(std::shared_ptr<LibraryFile> file)
{
  // Send GET request
  cpr::Response res = cpr::Get(cpr::Url{file->thumbnailUrl});

  if (res.status_code == 200)
  {
    std::string path = ConfigService::getService()->libraryFolderFilePath() + "/" + file->thumbnailFilename;
    std::ofstream file_stream(path, std::ios::binary);
    file_stream.write(res.text.data(), res.text.size());
    file_stream.close();
  }
  else
  {
    std::cout << "Error: Failed to download thumbnail" << std::endl;
  }
}

void LibraryService::downloadAllThumbnails()
{
  for (const auto &file : libraryFiles)
  {
    if (hasThumbnailOnDisk(file.second))
      continue;

    downloadFutures.push_back(std::async(std::launch::async, &LibraryService::downloadThumbnail, this, file.second));
  }
}

void LibraryService::submitFeedback(Feedback feedback, std::function<void()> success_callback, std::function<void(const std::string &)> error_callback)
{
  // First create a new sentry event
  sentry_uuid_t uuid = sentry_capture_event(sentry_value_new_message_event(
      SENTRY_LEVEL_INFO,
      "User Feedback",
      formatFeedback(feedback).c_str()));
}

void LibraryService::downloadFile(std::shared_ptr<LibraryFile> file, std::function<void()> callback)
{
  // Send GET request with a progress callback
  cpr::Response res =
      cpr::Get(
          cpr::Url{file->url},
          cpr::ProgressCallback{[file](cpr::cpr_off_t total, cpr::cpr_off_t current, cpr::cpr_off_t, cpr::cpr_off_t, intptr_t)
                                {
                                  log("Total: %ld", total);
                                  log("Current: %ld", current);
                                  file->isDownloading = true;

                                  float progress = (static_cast<double>(current) / static_cast<double>(total));

                                  if (progress > 0)
                                  {
                                    file->progress = progress;
                                  }

                                  return true;
                                }});

  if (res.status_code == 200)
  {
    std::string path = ConfigService::getService()->libraryFolderFilePath() + "/" + file->filename;
    std::ofstream file_stream(path, std::ios::binary);
    file_stream.write(res.text.data(), res.text.size());
    file_stream.close();
    libraryFileIdDownloadedMap[file->id] = true;
    log("Successfully downloaded file");
    file->isDownloading = false;
    if (callback) {
      callback();
    }
  }
  else
  {
    std::cout << "Error: Failed to download file" << std::endl;
  }
}

std::shared_ptr<LibraryFile> LibraryService::libraryFileForId(std::string id)
{
  if (libraryFiles.empty())
  {
    return nullptr;
  }
  if (libraryFiles.count(id) != 0) {
    return libraryFiles[id];
  }
  return nullptr;
}

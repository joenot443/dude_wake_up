//
//  LibraryService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/20/23.
//

#include "LibraryService.hpp"
#include "VideoSourceService.hpp"
#include "ConfigService.hpp"
#include "Credit.hpp"
#include "MainApp.h"
#include "Console.hpp"
#include "httplib.h"
#include "json.hpp"
#include <memory>
//#include <sentry.h>
#include <fstream>
#include <iostream>
#include <mutex>

using json = nlohmann::json;

// Helper function to parse HTTPS URL into host and path
// Returns false if parsing fails
inline bool parseHttpsUrl(const std::string& url_str, std::string& host, std::string& path) {
  const std::string scheme = "https://";
  if (url_str.rfind(scheme, 0) != 0) {
    log("Error: URL does not start with https:// : %s", url_str.c_str());
    return false; // Not an HTTPS URL
  }
  size_t scheme_end = scheme.length();
  size_t host_end = url_str.find('/', scheme_end);
  
  if (host_end != std::string::npos) {
    host = url_str.substr(scheme_end, host_end - scheme_end);
    path = url_str.substr(host_end);
  } else {
    // URL is just "https://hostname"
    host = url_str.substr(scheme_end);
    path = "/"; // Default path
  }
  
  if (host.empty()) {
    log("Error: Could not parse host from HTTPS URL: %s", url_str.c_str());
    return false;
  }
  return true;
}

const static std::string API_URL = "https://nottawa.app";
//const static std::string API_URL = "localhost";

void LibraryService::setup()
{
  // Automatically fetch shader credits on startup
  backgroundFetchShaderCredits();
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
  std::string host, path;
  
  // Parse the HTTPS URL
  if (!parseHttpsUrl(API_URL, host, path)) {
    log("Error: Could not parse API HTTPS URL: %s", API_URL.c_str());
    return;
  }
  
#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
  log("Error: HTTPS URL provided but httplib not compiled with SSL support for library files.");
  std::cerr << "Error: HTTPS URL provided but httplib not compiled with SSL support for library files." << std::endl;
  return;
#endif
  
  // Create SSL client directly
  httplib::SSLClient cli(host);
  
  // Disable SSL certificate verification (insecure but matches other methods)
  log("Warning: Disabling SSL server certificate verification! This is insecure.");
  cli.enable_server_certificate_verification(false);
  
  // Set reasonable timeouts
  cli.set_connection_timeout(10, 0);
  cli.set_read_timeout(30, 0);
  cli.set_follow_location(true);
  
  // Add a User-Agent header
  httplib::Headers headers = {
    { "User-Agent", "dude_wake_up_downloader/1.0" }
  };
  
  auto res = cli.Get("/api/media", headers);
  
  if (res && res->status == 200)
  {
    nlohmann::json json_body = nlohmann::json::parse(res->body);
    libraryFiles.clear();
    
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
    if (res) {
      log("Failed to fetch library files. Status: %d", res->status);
      log("Response body: %s", res->body.c_str());
    } else {
      auto err = res.error();
      log("Failed to fetch library files - connection error: %s (%d)", httplib::to_string(err).c_str(), static_cast<int>(err));
    }
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
// Assumes thumbnailUrl is always HTTPS.
void LibraryService::downloadThumbnail(std::shared_ptr<LibraryFile> file)
{
  std::string host, path;
  
  // Parse the HTTPS URL
  if (!parseHttpsUrl(file->thumbnailUrl, host, path)) {
    log("Error: Could not parse thumbnail HTTPS URL: %s", file->thumbnailUrl.c_str());
    std::cerr << "Error: Could not parse thumbnail HTTPS URL: " << file->thumbnailUrl << std::endl;
    return; // Stop if URL is invalid
  }
  
#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
  log("Error: HTTPS URL provided but httplib not compiled with SSL support for thumbnail download.");
  std::cerr << "Error: HTTPS URL provided but httplib not compiled with SSL support for thumbnail download." << std::endl;
  return; // Stop if SSL support is missing
#endif
  
  // Create SSL client directly
  httplib::SSLClient cli(host);
  
  // --- Option 2: Disable Verification (Insecure) ---
  log("Warning: Disabling SSL server certificate verification! This is insecure.");
  cli.enable_server_certificate_verification(false);
  // --- End Option 2 ---
  
  
  // Common settings
  cli.set_follow_location(true);
  cli.set_connection_timeout(10, 0);
  cli.set_read_timeout(30, 0); // Thumbnails should be smaller
  
  httplib::Headers headers = {
    { "User-Agent", "dude_wake_up_downloader/1.0" } // Add a user agent
  };
  
  // Send GET request
  auto res = cli.Get(path.c_str(), headers);
  
  if (res && res->status == 200)
  {
    std::string file_path = ConfigService::getService()->libraryFolderFilePath() + "/" + file->thumbnailFilename;
    std::ofstream file_stream(file_path, std::ios::binary);
    if (file_stream.is_open()) {
      file_stream.write(res->body.data(), res->body.size());
      file_stream.close();
      log("Successfully downloaded thumbnail %s", file->thumbnailFilename.c_str());
    } else {
      
      log("Error: Failed to open thumbnail file %s for writing.", file_path.c_str());
      std::cerr << "Error: Failed to open thumbnail file " << file_path << " for writing." << std::endl;
    }
  }
  else
  {
    if (res) {
      log("Error: Failed to download thumbnail %s. Status: %d", file->thumbnailFilename.c_str(), res->status);
      std::cerr << "Error: Failed to download thumbnail " << file->thumbnailFilename << ". Status: " << res->status << std::endl;
    } else {
      auto err = res.error();
      log("Error: Failed to download thumbnail %s. Error: %s (%d)", file->thumbnailFilename.c_str(), httplib::to_string(err).c_str(), static_cast<int>(err));
      std::cerr << "Error: Failed to download thumbnail " << file->thumbnailFilename << ". Connection/request error: " << httplib::to_string(err) << std::endl;
    }
  }
}

void LibraryService::downloadAllThumbnails()
{
  // Perform thumbnail downloads sequentially to avoid spawning a large number
  // of threads. This method is already executed on a background thread (see
  // didFetchLibraryFiles), so blocking here will not stall the main/UI
  // thread. Downloading one at a time also prevents excessive resource
  // consumption that previously led to crashes.
  for (const auto &file : libraryFiles)
  {
    if (hasThumbnailOnDisk(file.second)) {
      continue;  // Skip if thumbnail already exists on disk
    }

    // Download the thumbnail synchronously. The method handles its own error
    // reporting and will block until the download completes (or fails).
    downloadThumbnail(file.second);
  }
}

// Downloads the file specified by file->url with support for pause/resume.
// Assumes file->url is always HTTPS.
void LibraryService::downloadFile(std::shared_ptr<LibraryFile> file, std::function<void()> callback)
{
  std::string host, path;

  // Parse the HTTPS URL
  if (!parseHttpsUrl(file->url, host, path)) {
    log("Error: Could not parse file HTTPS URL: %s", file->url.c_str());
    std::cerr << "Error: Could not parse file HTTPS URL: " << file->url << std::endl;
    file->isDownloading = false;
    return;
  }

#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
  log("Error: HTTPS URL provided but httplib not compiled with SSL support.");
  std::cerr << "Error: HTTPS URL provided but httplib not compiled with SSL support." << std::endl;
  file->isDownloading = false;
  return;
#endif

  // Get file path for partial/complete download
  std::string file_path = ConfigService::getService()->libraryFolderFilePath() + "/" + file->filename;

  // Check if partial download exists
  uint64_t startByte = 0;
  if (ofFile::doesFileExist(file_path)) {
    ofFile partialFile(file_path);
    startByte = partialFile.getSize();
    file->downloadedBytes = startByte;
    log("Resuming download from byte %llu", startByte);
  }

  // Create SSL client
  httplib::SSLClient cli(host);
  log("Using SSLClient for host: %s", host.c_str());

  cli.enable_server_certificate_verification(false);
  cli.set_connection_timeout(15, 0);
  cli.set_read_timeout(120, 0);
  cli.set_follow_location(true);

  // Prepare headers with Range request if resuming
  httplib::Headers headers = {
    { "User-Agent", "dude_wake_up_downloader/1.0" }
  };

  if (startByte > 0) {
    headers.insert({ "Range", "bytes=" + std::to_string(startByte) + "-" });
  }

  // Send GET request with progress callback
  auto res = cli.Get(path.c_str(), headers,
                     [file, startByte](uint64_t current, uint64_t total) {
    if (!file->isDownloading) file->isDownloading = true;

    // Check if download is paused
    if (file->isPaused) {
      log("Download paused for file %s", file->filename.c_str());
      return false; // Abort download
    }

    // Update progress accounting for resumed bytes
    uint64_t totalDownloaded = startByte + current;
    uint64_t actualTotal = (total > 0) ? (startByte + total) : 0;

    file->downloadedBytes = totalDownloaded;
    file->totalBytes = actualTotal;

    if (actualTotal > 0) {
      file->progress = static_cast<float>(totalDownloaded) / static_cast<float>(actualTotal);
      log("Download Progress: %llu / %llu (%.1f%%)", totalDownloaded, actualTotal, file->progress * 100.0f);
    } else {
      log("Download Progress: %llu bytes / Unknown total", totalDownloaded);
      file->progress = 0.0f;
    }

    return true; // Continue download
  });

  // Check the result
  if (res && (res->status == 200 || res->status == 206)) // 206 = Partial Content (resume)
  {
    // Open file in append mode if resuming, otherwise truncate
    std::ios_base::openmode mode = std::ios::binary;
    if (startByte > 0 && res->status == 206) {
      mode |= std::ios::app; // Append mode for resume
    } else {
      mode |= std::ios::trunc; // Truncate for fresh download
    }

    std::ofstream file_stream(file_path, mode);
    if (file_stream.is_open()) {
      file_stream.write(res->body.data(), res->body.size());
      file_stream.close();

      // Only mark as complete if not paused
      if (!file->isPaused) {
        libraryFileIdDownloadedMap[file->id] = true;
        log("Successfully downloaded file %s", file->filename.c_str());
        file->isDownloading = false;
        file->progress = 1.0f;

        if (callback) {
          MainApp::getApp()->executeOnMainThread([callback](){
            callback();
          });
        }
      } else {
        // Download was paused - keep partial file
        file->isDownloading = false;
        log("Download paused, partial file saved: %s", file->filename.c_str());
      }
    } else {
      log("Error: Failed to open file %s for writing.", file_path.c_str());
      std::cerr << "Error: Failed to open file " << file_path << " for writing." << std::endl;
      file->isDownloading = false;
    }
  }
  else
  {
    if (res) {
      log("Error: Failed to download file %s. URL: %s. Status: %d", file->filename.c_str(), file->url.c_str(), res->status);
      std::cerr << "Error: Failed to download file " << file->filename << ". URL: " << file->url << ". Status: " << res->status << std::endl;
    } else {
      auto err = res.error();
      log("Error: Failed to download file %s. URL: %s. Error: %s (%d)", file->filename.c_str(), file->url.c_str(), httplib::to_string(err).c_str(), static_cast<int>(err));
      std::cerr << "Error: Failed to download file " << file->filename << ". URL: " << file->url << ". Connection/request error: " << httplib::to_string(err) << std::endl;
    }
    file->isDownloading = false;
  }
}

void LibraryService::pauseDownload(std::shared_ptr<LibraryFile> file)
{
  if (file && file->isDownloading) {
    file->isPaused = true;
    log("Pausing download for file %s", file->filename.c_str());
  }
}

void LibraryService::resumeDownload(std::shared_ptr<LibraryFile> file, std::function<void()> callback)
{
  if (file && file->isPaused) {
    file->isPaused = false;
    log("Resuming download for file %s", file->filename.c_str());

    // Launch new download thread that will resume from downloadedBytes
    downloadFutures.push_back(
      std::async(std::launch::async, &LibraryService::downloadFile, this, file, callback));
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

// Helper function to convert shader name string to ShaderType enum
ShaderType shaderTypeFromName(const std::string& name) {
  // Build a map of all possible shader names to their enum values
  static std::map<std::string, ShaderType> nameToTypeMap;
  if (nameToTypeMap.empty()) {
    // Populate the map by iterating through all shader types
    std::vector<ShaderType> allTypes = AllShaderTypes();
    for (ShaderType type : allTypes) {
      nameToTypeMap[shaderTypeName(type)] = type;
    }
    // Also add the additional shader types not in AllShaderTypes()
    for (int i = static_cast<int>(ShaderTypeNone); i <= static_cast<int>(ShaderTypeGodRay); i++) {
      ShaderType type = static_cast<ShaderType>(i);
      nameToTypeMap[shaderTypeName(type)] = type;
    }
  }
  
  auto it = nameToTypeMap.find(name);
  if (it != nameToTypeMap.end()) {
    return it->second;
  }
  return ShaderTypeNone; // Return None if name not found
}

// Fetches the shader credits from the server at /api/credits.
// This is a GET request that returns a JSON object of the form:
/*{[
 {
 "name": "Aerogel",
 "credit": "Joe Crozier",
 "description": "A shader that looks like aerogel"
 }
 ]
 }
 */
void LibraryService::fetchShaderCredits()
{
  std::string host, path;
  
  // Parse the HTTPS URL
  if (!parseHttpsUrl(API_URL, host, path)) {
    log("Error: Could not parse API HTTPS URL: %s", API_URL.c_str());
    return;
  }
  
#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
  log("Error: HTTPS URL provided but httplib not compiled with SSL support for shader credits.");
  std::cerr << "Error: HTTPS URL provided but httplib not compiled with SSL support for shader credits." << std::endl;
  return;
#endif
  
  // Create SSL client directly
  httplib::SSLClient cli(host);
  
  // Disable SSL certificate verification (insecure but matches other methods)
  log("Warning: Disabling SSL server certificate verification! This is insecure.");
  cli.enable_server_certificate_verification(false);
  
  // Set reasonable timeouts
  cli.set_connection_timeout(10, 0);
  cli.set_read_timeout(30, 0);
  cli.set_follow_location(true);
  
  // Add a User-Agent header
  httplib::Headers headers = {
    { "User-Agent", "dude_wake_up_downloader/1.0" }
  };
  
  auto res = cli.Get("/api/credits", headers);
  
  if (res && res->status == 200)
  {
    json json_body = json::parse(res->body);
    {
      std::lock_guard<std::mutex> lock(shaderCreditsMutex);
      shaderCredits.clear();
    }
    
    for (auto& shader : json_body) {
      std::string shaderName = shader["name"];
      std::string credit = shader["credit"];
      std::string description = shader["description"];
      ShaderType type = shaderTypeFromName(shaderName);
      if (type != ShaderTypeNone) {
        {
          std::lock_guard<std::mutex> lock(shaderCreditsMutex);
          shaderCredits.emplace(type, std::make_shared<Credit>(shaderName, credit, description));
        }
      } else {
        //        log("Warning: Unknown shader type name in credits: %s", shaderName.c_str());
      }
    }
    
    {
      std::lock_guard<std::mutex> lock(shaderCreditsMutex);
      log("Successfully fetched %zu shader credits", shaderCredits.size());
    }
  }
  else
  {
    if (res) {
      log("Failed to fetch shader credits. Status: %d", res->status);
      log("Response body: %s", res->body.c_str());
    } else {
      auto err = res.error();
      log("Failed to fetch shader credits - connection error: %s (%d)", httplib::to_string(err).c_str(), static_cast<int>(err));
    }
  }
}

void LibraryService::backgroundFetchShaderCredits()
{
  downloadFutures.push_back(std::async(std::launch::async, &LibraryService::fetchShaderCredits, this));
}

std::shared_ptr<Credit> LibraryService::getShaderCredit(ShaderType type)
{
  std::lock_guard<std::mutex> lock(shaderCreditsMutex);
  auto it = shaderCredits.find(type);
  if (it != shaderCredits.end()) {
    return it->second;
  }
  return nullptr;
}

bool LibraryService::hasCredit(ShaderType type)
{
  std::lock_guard<std::mutex> lock(shaderCreditsMutex);
  return shaderCredits.find(type) != shaderCredits.end();
}

// MARK: - Feedback Submission

void LibraryService::submitFeedback(const std::string &text,
                                    const std::string &stateJson,
                                    const std::string &screenshotData,
                                    const std::string &author,
                                    const std::string &email,
                                    std::function<void(bool success, const std::string &error)> callback)
{
  // Launch in background similar to other async operations
  downloadFutures.push_back(std::async(std::launch::async, [=]() {
    
    std::string host, dummy;
    if (!parseHttpsUrl(API_URL, host, dummy)) {
      if (callback) {
        MainApp::getApp()->executeOnMainThread([callback]() {
          callback(false, "Invalid API URL");
        });
      }
      return;
    }
    
#ifndef CPPHTTPLIB_OPENSSL_SUPPORT
    if (callback) {
      MainApp::getApp()->executeOnMainThread([callback]() {
        callback(false, "cpp-httplib built without SSL support");
      });
    }
    return;
#else
    httplib::SSLClient cli(host.c_str());
    cli.enable_server_certificate_verification(false);
    cli.set_connection_timeout(15, 0);
    cli.set_read_timeout(60, 0);
    cli.set_follow_location(true);
    
    httplib::Headers headers = {
      {"User-Agent", "dude_wake_up_downloader/1.0"}
    };
    
    httplib::MultipartFormDataItems items;
    items.push_back({"text", text, "", "text/plain"});
    items.push_back({"state", stateJson, "", "application/json"});
    if (!screenshotData.empty()) {
      items.push_back({"screenshot", screenshotData, "screenshot.png", "image/png"});
    }
    if (!author.empty()) {
      items.push_back({"author", author, "", "text/plain"});
    }
    if (!email.empty()) {
      items.push_back({"email", email, "", "text/plain"});
    }
    
    auto res = cli.Post("/api/feedback/new", headers, items);
#endif
    
    bool ok = false;
    std::string err;
    if (res && ((res->status == 201) || res->status == 200)) {
      ok = true;
    } else {
      ok = false;
      if (res) {
        err = "Server returned status " + std::to_string(res->status);
      } else {
        err = "Network error";
      }
    }
    
    // Invoke callback on main thread
    if (callback) {
      MainApp::getApp()->executeOnMainThread([callback, ok, err]() {
        callback(ok, err);
      });
    }
  }));
}

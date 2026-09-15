//
//  LibraryService_win.cpp
//
//  No-op stub of LibraryService for Windows builds. Every network path in
//  the real implementation goes through httplib::SSLClient, and until we
//  wire an OpenSSL (or Schannel) backend on Windows, none of those paths
//  can succeed. The macOS-side LibraryService.cpp is excluded on Windows
//  via patch-vcxproj.ps1's $ExcludeFiles.
//
//  Every method logs its first call and returns immediately. The library
//  UI in the app will show an empty library on Windows — that's the correct
//  behavior until Phase 5+ restores HTTPS.
//

#ifdef _WIN32

#include "LibraryService.hpp"
#include "ofLog.h"

LibraryService* LibraryService::service = nullptr;

namespace {
void warnOnce(const char* method) {
  static bool warned[24]{};
  static int  idx = 0;
  if (idx < 24) {
    ofLogWarning("LibraryService_win")
      << "stub: " << method << " — HTTPS backend disabled on Windows (Phase 5+)";
    warned[idx++] = true;
  }
}
} // namespace

void LibraryService::setup()                              { warnOnce("setup"); }
void LibraryService::fetchLibraryFiles()                  { warnOnce("fetchLibraryFiles"); }
void LibraryService::backgroundFetchLibraryFiles()        { warnOnce("backgroundFetchLibraryFiles"); }
void LibraryService::didFetchLibraryFiles()               { warnOnce("didFetchLibraryFiles"); }
void LibraryService::repopulateVideoSourcesFromMainThread() { warnOnce("repopulateVideoSourcesFromMainThread"); }

void LibraryService::downloadThumbnail(std::shared_ptr<LibraryFile>)                       { warnOnce("downloadThumbnail"); }
void LibraryService::downloadAllThumbnails()                                               { warnOnce("downloadAllThumbnails"); }
void LibraryService::downloadFile(std::shared_ptr<LibraryFile>, std::function<void()>)     { warnOnce("downloadFile"); }
void LibraryService::pauseDownload(std::shared_ptr<LibraryFile>)                           { warnOnce("pauseDownload"); }
void LibraryService::resumeDownload(std::shared_ptr<LibraryFile>, std::function<void()>)   { warnOnce("resumeDownload"); }

std::shared_ptr<LibraryFile> LibraryService::libraryFileForId(std::string) {
  warnOnce("libraryFileForId");
  return nullptr;
}

bool LibraryService::hasThumbnailOnDisk(std::shared_ptr<LibraryFile>) { warnOnce("hasThumbnailOnDisk"); return false; }
bool LibraryService::hasMediaOnDisk(std::shared_ptr<LibraryFile>)     { warnOnce("hasMediaOnDisk");     return false; }

void LibraryService::fetchShaderCredits()             { warnOnce("fetchShaderCredits"); }
void LibraryService::backgroundFetchShaderCredits()   { warnOnce("backgroundFetchShaderCredits"); }

std::shared_ptr<Credit> LibraryService::getShaderCredit(ShaderType) { warnOnce("getShaderCredit"); return nullptr; }
bool                    LibraryService::hasCredit(ShaderType)        { warnOnce("hasCredit"); return false; }

void LibraryService::submitFeedback(const std::string&, const std::string&, const std::string&,
                                    const std::string&, const std::string&,
                                    std::function<void(bool, const std::string&)> callback) {
  warnOnce("submitFeedback");
  if (callback) callback(false, "feedback submission unavailable on Windows (no HTTPS)");
}

void LibraryService::shareStrand(const std::string&, const std::string&, const std::string&,
                                 const std::string&, const std::string&, const std::string&,
                                 std::function<void(bool, const std::string&, const std::string&)> callback) {
  warnOnce("shareStrand");
  if (callback) callback(false, "", "strand sharing unavailable on Windows (no HTTPS)");
}

void LibraryService::fetchSharedStrand(const std::string&,
                                       std::function<void(bool, const std::string&, const std::string&, const std::string&)> callback) {
  warnOnce("fetchSharedStrand");
  if (callback) callback(false, "", "", "shared strand fetch unavailable on Windows (no HTTPS)");
}

#endif // _WIN32

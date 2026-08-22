// BookmarkService_win.cpp
// Non-macOS placeholder. BookmarkService is a macOS sandbox artifact (persistent
// file-access permission across launches). Windows apps aren't sandboxed by
// default, so the whole service is a permanent no-op on Windows. Full removal
// of the .hpp and its call sites is a Phase 5 task; this stub keeps the
// linker happy in the interim.

#ifndef TARGET_OSX

#include "BookmarkService.hpp"

BookmarkService* BookmarkService::service = nullptr;

void                          BookmarkService::saveBookmarkForPath(std::string)         {}
bool                          BookmarkService::hasBookmarkForPath (std::string)         { return false; }
std::shared_ptr<std::string>  BookmarkService::bookmarkForPath    (std::string)         { return nullptr; }
CFDataRef                     BookmarkService::loadBookmarkForPath(std::string)         { return nullptr; }

void                          BookmarkService::saveBookmarkForDirectory(std::string)    {}
bool                          BookmarkService::hasBookmarkForDirectory (std::string)    { return false; }
std::shared_ptr<std::string>  BookmarkService::bookmarkForDirectory    (std::string)    { return nullptr; }
CFDataRef                     BookmarkService::loadBookmarkForDirectory(std::string)    { return nullptr; }

void                          BookmarkService::beginAccessingBookmark(std::string)      {}
void                          BookmarkService::endAccessingBookmark  (std::string)      {}

#endif // !TARGET_OSX

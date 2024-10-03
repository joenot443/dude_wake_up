//
//  BookmarkService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 9/16/24.
//

#include "BookmarkService.hpp"
#include <CoreFoundation/CoreFoundation.h>
#include <string>

// Save the bookmark for the given path into UserDefaults
void BookmarkService::saveBookmarkForPath(std::string path) {
  // Create CFURLRef from path
  CFStringRef cfPath = CFStringCreateWithCString(kCFAllocatorDefault, path.c_str(), kCFStringEncodingUTF8);
  CFURLRef fileURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, cfPath, kCFURLPOSIXPathStyle, false);
  
  // Create a bookmark
  CFErrorRef error = nullptr;
  CFDataRef bookmarkData = CFURLCreateBookmarkData(kCFAllocatorDefault, fileURL, kCFURLBookmarkCreationWithSecurityScope, nullptr, nullptr, &error);
  
  if (bookmarkData != nullptr) {
    // Save the bookmark data into UserDefaults
    CFPreferencesSetAppValue(cfPath, bookmarkData, kCFPreferencesCurrentApplication);
    CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
    
    CFRelease(bookmarkData); // Release memory
  }
  
  // Clean up
  CFRelease(cfPath);
  CFRelease(fileURL);
}

// Check if a bookmark exists for the given path
bool BookmarkService::hasBookmarkForPath(std::string path) {

  CFStringRef cfPath = CFStringCreateWithCString(kCFAllocatorDefault, path.c_str(), kCFStringEncodingUTF8);
  
  // Check if a bookmark exists in UserDefaults
  CFDataRef bookmarkData = (CFDataRef)CFPreferencesCopyAppValue(cfPath, kCFPreferencesCurrentApplication);
  CFRelease(cfPath);
  
  if (bookmarkData != nullptr) {
    CFRelease(bookmarkData);
    return true; // Bookmark exists
  }
  
  return false;
}

// Load a bookmark for the given path from UserDefaults
CFDataRef BookmarkService::loadBookmarkForPath(std::string path) {
  CFStringRef cfPath = CFStringCreateWithCString(kCFAllocatorDefault, path.c_str(), kCFStringEncodingUTF8);
  
  // Retrieve bookmark data from UserDefaults
  CFDataRef bookmarkData = (CFDataRef)CFPreferencesCopyAppValue(cfPath, kCFPreferencesCurrentApplication);
  CFRelease(cfPath);
  
  return bookmarkData; // Caller is responsible for releasing this
}

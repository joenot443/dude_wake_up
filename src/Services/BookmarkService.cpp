//
//  BookmarkService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 9/16/24.
//

#include "BookmarkService.hpp"
#include "Strings.hpp"
#include "Console.hpp"
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

std::shared_ptr<std::string> BookmarkService::bookmarkForPath(std::string path) {
  CFStringRef cfPath = CFStringCreateWithCString(kCFAllocatorDefault, path.c_str(), kCFStringEncodingUTF8);
  
  // Retrieve bookmark data from UserDefaults
  CFDataRef bookmarkData = (CFDataRef)CFPreferencesCopyAppValue(cfPath, kCFPreferencesCurrentApplication);
  
  if (bookmarkData == nullptr) {
    // Bookmark doesn't exist, create and save it
    CFURLRef fileURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, cfPath, kCFURLPOSIXPathStyle, false);
    CFErrorRef error = nullptr;
    bookmarkData = CFURLCreateBookmarkData(kCFAllocatorDefault, fileURL, kCFURLBookmarkCreationWithSecurityScope, nullptr, nullptr, &error);
    
    if (bookmarkData != nullptr) {
      // Save the bookmark data into UserDefaults
      CFPreferencesSetAppValue(cfPath, bookmarkData, kCFPreferencesCurrentApplication);
      CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
    }
    
    CFRelease(fileURL);
  }
  
  CFRelease(cfPath);
  
  if (bookmarkData != nullptr) {
    // Convert CFDataRef to std::string
    const UInt8* dataPtr = CFDataGetBytePtr(bookmarkData);
    CFIndex length = CFDataGetLength(bookmarkData);
    auto bookmarkString = std::make_shared<std::string>(reinterpret_cast<const char*>(dataPtr), length);
    
    CFRelease(bookmarkData); // Release memory
    return bookmarkString;
  }
  
  return nullptr; // Return nullptr if bookmark creation failed
}

void BookmarkService::saveBookmarkForDirectory(std::string directoryPath) {
  CFStringRef cfPath = CFStringCreateWithCString(kCFAllocatorDefault, directoryPath.c_str(), kCFStringEncodingUTF8);
  CFURLRef dirURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, cfPath, kCFURLPOSIXPathStyle, true); // true for directory
  
  CFErrorRef error = nullptr;
  CFDataRef bookmarkData = CFURLCreateBookmarkData(kCFAllocatorDefault, dirURL, kCFURLBookmarkCreationWithSecurityScope, nullptr, nullptr, &error);
  
  if (bookmarkData != nullptr) {
    CFPreferencesSetAppValue(cfPath, bookmarkData, kCFPreferencesCurrentApplication);
    CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
    CFRelease(bookmarkData);
  }
  
  CFRelease(cfPath);
  CFRelease(dirURL);
}

bool BookmarkService::hasBookmarkForDirectory(std::string directoryPath) {
  CFStringRef cfPath = CFStringCreateWithCString(kCFAllocatorDefault, directoryPath.c_str(), kCFStringEncodingUTF8);
  CFDataRef bookmarkData = (CFDataRef)CFPreferencesCopyAppValue(cfPath, kCFPreferencesCurrentApplication);
  CFRelease(cfPath);
  
  if (bookmarkData != nullptr) {
    CFRelease(bookmarkData);
    return true;
  }
  
  return false;
}

CFDataRef BookmarkService::loadBookmarkForDirectory(std::string directoryPath) {
  CFStringRef cfPath = CFStringCreateWithCString(kCFAllocatorDefault, directoryPath.c_str(), kCFStringEncodingUTF8);
  CFDataRef bookmarkData = (CFDataRef)CFPreferencesCopyAppValue(cfPath, kCFPreferencesCurrentApplication);
  CFRelease(cfPath);
  
  return bookmarkData;
}

std::shared_ptr<std::string> BookmarkService::bookmarkForDirectory(std::string directoryPath) {
  CFStringRef cfPath = CFStringCreateWithCString(kCFAllocatorDefault, directoryPath.c_str(), kCFStringEncodingUTF8);
  CFDataRef bookmarkData = (CFDataRef)CFPreferencesCopyAppValue(cfPath, kCFPreferencesCurrentApplication);
  
  if (bookmarkData == nullptr) {
    CFURLRef dirURL = CFURLCreateWithFileSystemPath(kCFAllocatorDefault, cfPath, kCFURLPOSIXPathStyle, true);
    CFErrorRef error = nullptr;
    bookmarkData = CFURLCreateBookmarkData(kCFAllocatorDefault, dirURL, kCFURLBookmarkCreationWithSecurityScope, nullptr, nullptr, &error);
    
    if (bookmarkData != nullptr) {
      CFPreferencesSetAppValue(cfPath, bookmarkData, kCFPreferencesCurrentApplication);
      CFPreferencesAppSynchronize(kCFPreferencesCurrentApplication);
    }
    
    CFRelease(dirURL);
  }
  
  CFRelease(cfPath);
  
  if (bookmarkData != nullptr) {
    const UInt8* dataPtr = CFDataGetBytePtr(bookmarkData);
    CFIndex length = CFDataGetLength(bookmarkData);
    auto bookmarkString = std::make_shared<std::string>(reinterpret_cast<const char*>(dataPtr), length);
    
    CFRelease(bookmarkData);
    return bookmarkString;
  }
  
  return nullptr;
}

void BookmarkService::beginAccessingBookmark(std::string path) {
  // Get the bookmark string for the given path
  auto bookmarkString = bookmarkForPath(path);
  if (!bookmarkString) {
    log("Failed to create or retrieve bookmark for path");
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

void BookmarkService::endAccessingBookmark(std::string path) {
  // Get the bookmark string for the given path
  auto bookmarkString = bookmarkForPath(path);
  if (!bookmarkString) {
    log("Failed to retrieve bookmark for path");
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
    kCFURLBookmarkResolutionWithSecurityScope,  // Options
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
  
  // Stop accessing the security-scoped resource
  CFURLStopAccessingSecurityScopedResource(fileURL);
  
  // Release CF objects
  CFRelease(fileURL);
  CFRelease(bookmarkData);
}

//
//  BookmarkService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 9/16/24.
//

#ifndef BookmarkService_hpp
#define BookmarkService_hpp

#include <stdio.h>
#include "ofMain.h"

class BookmarkService
{
public:
  void saveBookmarkForPath(std::string path);
  bool hasBookmarkForPath(std::string path);
  std::shared_ptr<std::string> bookmarkForPath(std::string path);
  CFDataRef loadBookmarkForPath(std::string path);

  void setup() {
    
  }
  
  BookmarkService() {};
  static BookmarkService *service;

  static BookmarkService *getService() {
    if (!service) {
      service = new BookmarkService;
      service->setup();
    }
    return service;
  }
};


#endif /* BookmarkService_hpp */

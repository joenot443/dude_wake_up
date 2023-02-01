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

class LibraryService {
  static LibraryService* service;
  LibraryService() {};
  
  static LibraryService* getService() {
    if (!service) {
      service = new LibraryService;
      service->setup();
    }
    return service;
  }
  
  void setup();
  void fetchLibraryFiles();
  
  std::vector<LibraryFile> libraryFiles;
};

#endif /* LibraryService_hpp */

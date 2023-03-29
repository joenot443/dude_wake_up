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
#include "ShaderChainer.hpp"

class LibraryService {
public:
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
  void uploadChainer(const std::string &name, const std::string &author, const std::shared_ptr<ShaderChainer> shaderChainer, std::function<void()> success_callback, std::function<void(const std::string &)> error_callback);
  
  std::vector<LibraryFile> libraryFiles;
};

#endif /* LibraryService_hpp */

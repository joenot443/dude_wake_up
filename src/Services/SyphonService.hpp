//
//  SyphonService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 9/16/24.
//

#ifndef SyphonService_hpp
#define SyphonService_hpp

#include <stdio.h>
#include "ofMain.h"
#include "ofxSyphon.h"

class SyphonService
{
public:
  void setup() {
    // Initialize Syphon server setup
  }
  
  SyphonService() {};
  static SyphonService *service;

  static SyphonService *getService() {
    if (!service) {
      service = new SyphonService;
      service->setup();
    }
    return service;
  }
};

#endif /* SyphonService_hpp */

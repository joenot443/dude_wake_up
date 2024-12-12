//
//  SyphonService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 9/16/24.
//

#ifndef SyphonService_hpp
#define SyphonService_hpp

#include "ofMain.h"
#include "ofxSyphon.h"
#include <memory>

class SyphonService
{
public:
  void setup();
  
  SyphonService();
  static SyphonService *service;

  static SyphonService *getService();

  // Use a shared_ptr to manage the ofFbo for publishing
  void publishFbo(std::shared_ptr<ofFbo> fbo);

  // Update method to be called every frame
  void update();
  
private:
  ofxSyphonServer syphonServer;
  ofxSyphonClient syphonClient;
  std::shared_ptr<ofFbo> attachedFbo; // Shared pointer to the attached Fbo
};

#endif /* SyphonService_hpp */

//
//  VideoSource.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#ifndef VideoSource_hpp
#define VideoSource_hpp

#include <stdio.h>
#include "VideoSettings.hpp"

enum VideoSourceType { VideoSource_webcam, VideoSource_file };

class VideoSource {
  
public:
  std::string id;
  std::string name;

  VideoSourceType type;
  std::shared_ptr<ofTexture> frameTexture;
  ofBufferObject frameBuffer;

  VideoSource(std::string id, std::string name, VideoSourceType type) :
  id(id), name(name), type(type) {};

  virtual void setup() {};
  virtual void update() {};
  
private:
};

#endif /* VideoSourceSource_hpp */

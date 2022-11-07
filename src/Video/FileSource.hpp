//
//  FileSource.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#ifndef FileSource_hpp
#define FileSource_hpp

#include <stdio.h>
#include "VideoSource.hpp"

class FileSource : public VideoSource {
  std::string path;
  ofVideoPlayer player;
  
public:
  FileSource(std::string id, std::string name, std::string path) : VideoSource(id, name, VideoSource_file), path(path) {};
  void setup();
  void update();
};

#endif

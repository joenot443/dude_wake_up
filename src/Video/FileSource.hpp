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
  std::shared_ptr<Parameter> volume;
  
public:
  FileSource(std::string id, std::string name, std::string path) : VideoSource(id, name, VideoSource_file), path(path), volume(std::make_shared<Parameter>("volume", 0.5, 0.0, 1.0)) {};
  void setup();
  void saveFrame();
  json serialize() override;
  void load(json j) override;
  void drawSettings() override;
};

#endif

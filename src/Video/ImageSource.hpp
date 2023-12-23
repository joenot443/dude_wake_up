//
//  ImageSource.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/1/23.
//

#ifndef ImageSource_hpp
#define ImageSource_hpp

#include <stdio.h>
#include "ofMain.h"
#include "VideoSource.hpp"

using json = nlohmann::json;

class ImageSource : public VideoSource {
  
public:
  ImageSource(std::string id, std::string name, std::string path) : VideoSource(id, name, VideoSource_image), path(path) {};
  void setup() override;
  void saveFrame() override;
  json serialize() override;
  void load(json j) override;

private:
  std::string path;
};

#endif /* ImageSource_hpp */

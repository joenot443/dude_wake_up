//
//  MultiSource.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/1/23.
//

#ifndef MultiSource_hpp
#define MultiSource_hpp

#include <stdio.h>
#include "ofMain.h"
#include "VideoSource.hpp"

using json = nlohmann::json;

class MultiSource : public VideoSource {
  
public:
  MultiSource(std::string id, std::string name) : 
  VideoSource(id, name, VideoSource_multi), sources(sources), selectedSource(std::make_shared<Parameter>("Selected Source", 0.0, 0.0, 10.0)) {};
  void setup() override;
  void saveFrame() override;
  json serialize() override;
  void load(json j) override;
  void drawSettings() override;
  void addSource(std::shared_ptr<VideoSource> source);

private:
  std::vector<std::shared_ptr<VideoSource>> sources;
  std::shared_ptr<Parameter> selectedSource;
};

#endif /* MultiSource_hpp */

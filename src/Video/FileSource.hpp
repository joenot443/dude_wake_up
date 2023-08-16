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

class FileSource : public VideoSource
{
public:
  FileSource(std::string id, std::string name, std::string path)
      : VideoSource(id, name, VideoSource_file),
        path(path),
        volume(std::make_shared<Parameter>("volume", 0.5, 0.0, 1.0)),
        position(std::make_shared<Parameter>("position", 0.0, 0.0, 1.0)),
        mute(std::make_shared<Parameter>("mute", 0.0, 0.0, 1.0)){};

  std::string path;
  ofVideoPlayer player;
  ofFbo fbo;
  std::shared_ptr<Parameter> volume;
  std::shared_ptr<Parameter> position;
  std::shared_ptr<Parameter> mute;
  void setup() override;
  void saveFrame() override;
  json serialize() override;
  void load(json j) override;
  void drawSettings() override;
  void teardown() override;
  
  void updatePlaybackPosition();
};

#endif

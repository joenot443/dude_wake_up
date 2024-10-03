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
  FileSource(std::string id, std::string name, std::string path, VideoSourceType type = VideoSource_file)
  : VideoSource(id, name, type),
  path(path),
  volume(std::make_shared<Parameter>("Volume", 0.5, 0.0, 1.0)),
  sliderPosition(std::make_shared<Parameter>("Position", 0.0, 0.0, 1.0)),
  mute(std::make_shared<Parameter>("Mute", 0.0, 0.0, 1.0)),
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 2.0)),
  boomerang(std::make_shared<Parameter>("Boomerang", ParameterType_Bool)),
  position(0.0),
  start(0.0),
  end(1.0){};
  
  std::string path;
  ofVideoPlayer player;
  ofShader maskShader;
  std::shared_ptr<Parameter> volume;
  std::shared_ptr<Parameter> sliderPosition;
  std::shared_ptr<Parameter> boomerang;
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<Parameter> mute;
  void setup() override;
  void updateSettings();
  void saveFrame() override;
  json serialize() override;
  void load(json j) override;
  void drawSettings() override;
  void teardown() override;
  
  float position;
  float start;
  float end;
  
  bool playbackRangeDirty();
  void validateSettings();
  void applyRanges();
  void updatePlaybackPosition();
};

#endif

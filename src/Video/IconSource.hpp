//
//  IconSource.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/10/24.
//

#ifndef IconSource_hpp
#define IconSource_hpp

#include <stdio.h>
#include "ofMain.h"
#include "IconService.hpp"
#include "VideoSource.hpp"
#include "Texture.hpp"
#include "ImageTexture.hpp"
#include "ConfigService.hpp"

using json = nlohmann::json;

class IconSourceSettings : public VideoSourceSettings
{
public:
  std::shared_ptr<Parameter> icon;
  std::shared_ptr<Parameter> texture;
  std::shared_ptr<Parameter> enableTexture;
  std::vector<std::string> textureOptions;

  IconSourceSettings(std::string sourceId, json j) : VideoSourceSettings(sourceId, j),
                                                     icon(std::make_shared<Parameter>("icon", 0, 0, 1000)),
  enableTexture(std::make_shared<Parameter>("Enable Texture", 0, 0, 1)),
                                                     texture(std::make_shared<Parameter>("texture", 0, 0, 1000))
  {
    parameters = {icon, texture, enableTexture};
  }
};

class IconSource : public VideoSource
{

public:
  IconSource(std::string id, std::string name) : VideoSource(id, name, VideoSource_icon),
                                                 settings(std::make_shared<IconSourceSettings>(id, 0)),
                                                 imageTexture(std::make_shared<ImageTexture>(IconService::getService()->availableIcons()[0])){};

  std::shared_ptr<IconSourceSettings> settings;
  void setup() override;
  void drawSettings() override;
  json serialize() override;
  void load(json j) override;
  void updateSource();
  void drawSource();
  std::shared_ptr<ImageTexture> imageTexture;

private:
  std::string path;
};

#endif /* IconSource_hpp */

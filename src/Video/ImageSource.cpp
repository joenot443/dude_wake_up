//
//  ImageSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/1/23.
//

#include "ImageSource.hpp"
#include "NodeLayoutView.hpp"
#include "LayoutStateService.hpp"

void ImageSource::setup()
{
  ofImage image;
  image.load(path);
  fbo->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y);
  optionalFbo->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y);
  fbo->begin();
  image.draw(0, 0);
  fbo->end();
}

void ImageSource::saveFrame()
{
  return;
}

json ImageSource::serialize()
{
  json j;
  j["path"] = path;
  j["id"] = id;
  j["sourceName"] = sourceName;
  j["videoSourceType"] = VideoSource_image;
  j["settings"] = settings->serialize();
  auto node = NodeLayoutView::getInstance()->nodeForShaderSourceId(id);
  if (node != nullptr)
  {
    j["x"] = node->position.x;
    j["y"] = node->position.y;
  }
  return j;
}

void ImageSource::load(json j)
{
  if (!j.is_object())
  {
    log("Error hydrating ImageSource from json");
    return;
  }

  path = j["path"];
  id = j["id"];
  sourceName = j["sourceName"];
  settings->load(j["settings"]);
  auto node = NodeLayoutView::getInstance()->nodeForShaderSourceId(id);
  if (node != nullptr)
  {
    node->position.x = j["x"];
    node->position.y = j["y"];
  }
}

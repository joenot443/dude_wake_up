//
//  ImageSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/1/23.
//

#include "ImageSource.hpp"
#include "NodeLayoutView.hpp"

void ImageSource::setup()
{
  ofImage image;
  fbo->allocate(settings->width->value, settings->height->value);
  image.load(path);
  image.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
  fbo->getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
  fbo->begin();
  image.draw(0, 0, settings->width->value, settings->height->value);
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
  auto node = NodeLayoutView::getInstance()->nodeForShaderSourceId(id);
  if (node != nullptr)
  {
    node->position.x = j["x"];
    node->position.y = j["y"];
  }
}

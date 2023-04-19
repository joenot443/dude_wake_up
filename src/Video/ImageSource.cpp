//
//  ImageSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/1/23.
//

#include "ImageSource.hpp"
#include "NodeLayoutView.hpp"

void ImageSource::setup() {
  ofImage image;
  fbo.allocate(settings.width->value, settings.height->value);
  image.load(path);
  image.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
  fbo.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
  fbo.begin();
  image.draw(0,0, settings.width->value, settings.height->value);
  fbo.end();
  
  frameTexture = std::shared_ptr<ofTexture>(&fbo.getTexture());
}

void ImageSource::saveFrame() {
  return;
}

json ImageSource::serialize() {
  json j;
  j["path"] = path;
  j["id"] = id;
  j["sourceName"] = sourceName;
  j["videoSourceType"] = VideoSource_file;
  j["x"] = NodeLayoutView::getInstance()->nodeForShaderSourceId(id)->position.x;
  j["y"] = NodeLayoutView::getInstance()->nodeForShaderSourceId(id)->position.y;
  return j;
}

void ImageSource::load(json j) {
  
}

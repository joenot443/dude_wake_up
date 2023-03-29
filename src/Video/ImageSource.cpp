//
//  ImageSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/1/23.
//

#include "ImageSource.hpp"


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
  
}

void ImageSource::load(json j) {
  
}

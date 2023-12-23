//
//  ImageTexture.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/21/23.
//

#include "ImageTexture.hpp"
#include "ConfigService.hpp"


void ImageTexture::setup() {
  ofImage image;
  fbo->allocate(500, 500);
  image.load("shaders/" + name);
  image.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
  fbo->getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
  fbo->begin();
  image.draw(0, 0, 500, 500);
  fbo->end();
}


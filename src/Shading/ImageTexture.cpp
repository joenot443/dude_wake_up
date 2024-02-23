//
//  ImageTexture.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/21/23.
//

#include "ImageTexture.hpp"
#include "ConfigService.hpp"

void ImageTexture::prepareImage() {
  fbo->allocate(500, 500, GL_RGBA);
  image.load(icon->path);
  image.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
  fbo->getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
}

void ImageTexture::drawImage()
{
  fbo->begin();
  ofClear(0, 0, 0, 255);
  ofClear(0, 0, 0, 0 );
  image.draw(0, 0, 500, 500);
  fbo->end();
}

void ImageTexture::drawImageWithTexture() {
  fbo->begin();
  shader.begin();
  ofClear(0, 0, 0, 255);
  ofClear(0, 0, 0, 0 );
  shader.setUniformTexture("tex", texture->fbo.getTexture(), 4);
  shader.setUniformTexture("maskTex", image.getTexture(), 8);
  shader.setUniform1f("blend", 1.0);
  image.getTexture().draw(0, 0, 500, 500);
  shader.end();
  fbo->end();
}

void ImageTexture::updateImage(std::shared_ptr<Icon> icon)
{
  this->icon = icon;
  if (texture != nullptr) {
    prepareImage();
    drawImageWithTexture();
  } else {
    prepareImage();
    drawImage();
  }
}

void ImageTexture::updateTexture(std::shared_ptr<Texture> texture)
{
  this->texture = texture;
  if (texture != nullptr) {
    drawImageWithTexture();
  } else {
    drawImage();
  }
}

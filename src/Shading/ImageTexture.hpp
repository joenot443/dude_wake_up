//
//  ImageTexture.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/21/23.
//

#ifndef ImageTexture_hpp
#define ImageTexture_hpp

#include <stdio.h>
#include <ofMain.h>
#include "Texture.hpp"
#include "Icon.hpp"

class ImageTexture
{
public:
  std::shared_ptr<ofFbo> fbo;
  std::shared_ptr<Icon> icon;
  // Optional texture
  std::shared_ptr<Texture> texture;
  ofShader shader;
  ofImage image;

  void updateImage(std::shared_ptr<Icon> icon);
  void updateTexture(std::shared_ptr<Texture> texture);
  ImageTexture(std::shared_ptr<Icon> icon) :  icon(icon), fbo(std::make_shared<ofFbo>())
  {
    shader.load("shaders/Mask");
    fbo->allocate(500, 500);
  };

private:
  void prepareImage();
  void drawImage();
  void drawImageWithTexture();
};

#endif

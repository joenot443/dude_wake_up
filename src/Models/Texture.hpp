//
//  Texture.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/14/24.
//

#ifndef Texture_h
#define Texture_h

#include "ofMain.h"

struct Texture
{
  std::string name;
  std::string path;

  ofFbo fbo;
  Texture(std::string name, std::string path) : name(name), path(path)
  {
    setup();
  }

  void setup()
  {
    ofImage img;
    img.load(path);
    fbo.allocate(img.getWidth(), img.getHeight(), GL_RGBA);
    fbo.getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
    fbo.begin();
    img.draw(0, 0);
    fbo.end();
  }
};

#endif /* Texture_h */

#ifndef Image_hpp
#define Image_hpp

#include <string>
#include "ofMain.h"
#include "imgui.h"

struct Image {
  std::string name;
  std::string path;
  ofImage image;
  ofFbo fbo;
  ImTextureID textureID;

  Image(std::string name, std::string path) : name(name), path(path) {
    image.load(path);
    fbo.allocate(image.getWidth(), image.getHeight(), GL_RGBA);
    fbo.begin();
    image.draw(0, 0);
    fbo.end();
    textureID = (ImTextureID)(uintptr_t)fbo.getTexture().getTextureData().textureID;
  }
};

#endif /* Image_hpp */ 
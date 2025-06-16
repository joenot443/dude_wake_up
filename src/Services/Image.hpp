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
    image.getTexture().setTextureMinMagFilter(GL_LINEAR, GL_LINEAR);
    image.load(path);
    textureID = (ImTextureID)(uintptr_t)image.getTexture().getTextureData().textureID;
    // after you’ve uploaded/allocated the texture once
    glBindTexture(GL_TEXTURE_2D, textureID);
    glGenerateMipmap(GL_TEXTURE_2D);                       // build full pyramid
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER,
                    GL_LINEAR_MIPMAP_LINEAR);              // trilinear minification
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER,
                    GL_LINEAR);                            // keep bilinear for up-scales
    glBindTexture(GL_TEXTURE_2D, 0);
  }
};

#endif /* Image_hpp */ 

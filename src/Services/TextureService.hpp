//
//  TextureService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/14/24.
//

#ifndef TextureService_hpp
#define TextureService_hpp

#include <stdio.h>
#include "Texture.hpp"

class TextureService
{
private:
  std::vector<std::shared_ptr<Texture>> textures;

public:
  void setup();
  void populateTextures();
  bool populated;
  std::vector<std::shared_ptr<Texture>> availableTextures();
  std::vector<std::string> availableTextureNames();
  std::shared_ptr<Texture> defaultTexture();
  std::shared_ptr<Texture> textureWithName(std::string name);
  
  static TextureService *service;
  TextureService(){};
  
  
  static TextureService *getService() {
    if (!service) {
      service = new TextureService;
    }
    return service;
  }
};

#endif /* TextureService_hpp */

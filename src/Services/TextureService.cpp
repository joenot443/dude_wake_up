//
//  TextureService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/14/24.
//

#include "TextureService.hpp"
#include "ofMain.h"

void TextureService::setup()
{
  populateTextures();
}

void TextureService::populateTextures()
{
  std::vector<std::shared_ptr<Texture>> textures;

  ofDirectory dir("shaders/textures");
  dir.listDir();
  for (int i = 0; i < dir.size(); i++)
  {
    std::string path = dir.getPath(i);
    std::string name = dir.getName(i);
    textures.push_back(std::make_shared<Texture>(name, path));
  }
  // Sort by name
  std::sort(textures.begin(), textures.end(), [](std::shared_ptr<Texture> a, std::shared_ptr<Texture> b)
            { return a->name < b->name; });
  populated = true;
  this->textures = textures;
}

std::vector<std::shared_ptr<Texture>> TextureService::availableTextures()
{
  if (!populated) populateTextures();
  return textures;
}

std::vector<std::string> TextureService::availableTextureNames()
{
  if (!populated) populateTextures();

  std::vector<std::string> names;
  for (auto texture : textures)
  {
    names.push_back(texture->name);
  }
  return names;
}

std::shared_ptr<Texture> TextureService::defaultTexture()
{
  return textures[0];
}

std::shared_ptr<Texture> TextureService::textureWithName(std::string name)
{
  for (auto texture : textures)
  {
    if (texture->name == name)
    {
      return texture;
    }
  }
  return nullptr;
}

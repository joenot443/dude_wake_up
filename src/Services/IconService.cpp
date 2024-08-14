//
//  IconService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/14/24.
//

#include "IconService.hpp"
#include "ofMain.h"

void IconService::setup()
{
  solidColorShader->load("shaders/SolidColor");
  populateIcons();
}

void IconService::populateIcons()
{
  std::vector<std::shared_ptr<Icon>> icons;

  ofDirectory dir("images");
  dir.listDir();
  /*
   images will include several directories.
   Traverse through each directory and add its containing icons.
   Set its category to be the name of the directory.
   */

  for (int i = 0; i < dir.size(); i++)
  {
    std::string category = dir.getName(i);
    ofDirectory subDir(dir.getPath(i));
    if (!subDir.isDirectory()) continue;
    
    subDir.listDir();
    for (int j = 0; j < subDir.size(); j++)
    {
      if (!subDir.getFile(j).isFile()) continue;
      std::string path = subDir.getPath(j);
      std::string name = subDir.getName(j);
      std::string previewPath = subDir.path() + "previews/" + name;
      icons.push_back(std::make_shared<Icon>(name, path, previewPath, category));
    }
  }

  // Sort by name
  std::sort(icons.begin(), icons.end(), [](std::shared_ptr<Icon> a, std::shared_ptr<Icon> b)
            { return a->name < b->name; });

  this->icons = icons;
}

std::vector<std::shared_ptr<Icon>> IconService::availableIcons()
{
  return icons;
}

std::vector<std::string> IconService::availableIconNames()
{
  std::vector<std::string> names;
  for (auto icon : icons)
  {
    names.push_back(icon->name);
  }
  return names;
}

std::shared_ptr<Icon> IconService::defaultIcon()
{
  return icons[0];
}

std::shared_ptr<Icon> IconService::iconWithName(std::string name)
{
  for (auto icon : icons)
  {
    if (icon->name == name)
    {
      return icon;
    }
  }
  return nullptr;
}

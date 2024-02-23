//
//  Icon.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/16/24.
//

#ifndef Icon_h
#define Icon_h

#include "ofMain.h"

struct Icon
{
  std::string name;
  std::string path;
  std::string previewPath;
  std::string category;

  ofFbo fbo;
  Icon(std::string name, std::string path, std::string previewPath, std::string category) : name(name), path(path), category(category), previewPath(previewPath)
  {
    setup();
  }

  void setup()
  {
    ofImage img;
    img.load(previewPath);
    fbo.allocate(100, 100, GL_RGBA);
    fbo.begin();
    img.draw(0, 0, 100, 100);
    fbo.end();
  }
};

#endif /* Icon_h */

//
//  Icon.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/1/24.
//

#include <stdio.h>
#include "Icon.hpp"
#include "IconService.hpp"

void Icon::setup()
{
  ofImage img;
  img.load(previewPath);
  fbo.allocate(100, 100, GL_RGBA);
  fbo.begin();
  IconService::getService()->solidColorShader->begin();
  IconService::getService()->solidColorShader->setUniformTexture("tex", img.getTexture(), 4);
  IconService::getService()->solidColorShader->setUniform4f("color", 1.0, 1.0, 1.0, 1.0);
  IconService::getService()->solidColorShader->setUniform1f("colorMix", 1.0);
  img.draw(0, 0, 100, 100);
  IconService::getService()->solidColorShader->end();
  fbo.end();
}

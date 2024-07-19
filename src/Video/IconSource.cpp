//
//  IconSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/10/24.
//

#include "IconSource.hpp"
#include "IconService.hpp"
#include "TextureService.hpp"
#include "NodeLayoutView.hpp"
#include "LayoutStateService.hpp"
#include "CommonViews.hpp"

void IconSource::setup()
{
  settings->textureOptions = TextureService::getService()->availableTextureNames();
  // Insert "No Texture" as the first option
  drawSource();
  updateSource();
}

void IconSource::drawSource()
{
  fbo->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y);
  fbo->getTexture().setTextureWrap(GL_REPEAT, GL_REPEAT);
  fbo->begin();
  imageTexture->fbo->draw(0, 0, LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y);
  fbo->end();
}

void IconSource::drawSettings()
{
  if (CommonViews::IconFieldAndBrowser(settings->icon))
  {
    updateSource();
  }

  if (CommonViews::ShaderCheckbox(settings->enableTexture))
  {
    updateSource();
  }

  if (!settings->enableTexture->boolValue)
    return;

  if (CommonViews::TextureFieldAndBrowser(settings->texture))
  {
    updateSource();
  }
}

void IconSource::updateSource()
{
  imageTexture->updateImage(IconService::getService()->availableIcons()[settings->icon->intValue]);

  if (settings->enableTexture->boolValue)
  {
    imageTexture->updateTexture(TextureService::getService()->textureWithName(settings->textureOptions[settings->texture->intValue]));
  }
  else
  {
    imageTexture->updateTexture(nullptr);
  }
  drawSource();
}

json IconSource::serialize()
{
  json j;
  j["videoSourceType"] = VideoSource_icon;
  j["id"] = id;
  j["sourceName"] = sourceName;
  j["settings"] = settings->serialize();
  auto node = NodeLayoutView::getInstance()->nodeForShaderSourceId(id);
  if (node != nullptr)
  {
    j["x"] = node->position.x;
    j["y"] = node->position.y;
  }
  return j;
}

void IconSource::load(json j)
{
  if (!j.is_object())
    return;

  settings->load(j["settings"]);
}

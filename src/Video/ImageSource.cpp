//
//  ImageSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/1/23.
//

#include "ImageSource.hpp"
#include "NodeLayoutView.hpp"
#include "VideoSourceService.hpp"
#include "LayoutStateService.hpp"
#include "CommonViews.hpp"

void ImageSource::setup()
{
  image.load(path);
  fbo->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y);
  optionalFbo->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y);
  saveFrame();
}

void ImageSource::saveFrame()
{
  // If our width or height has changed, setup again
  if (fbo->getWidth() != LayoutStateService::getService()->resolution.x ||
      fbo->getHeight() != LayoutStateService::getService()->resolution.y) {
    setup();
    return;
  }

  fbo->begin();
  ofClear(0, 0, 0, 255);

  float imageWidth = image.getWidth();
  float imageHeight = image.getHeight();
  float canvasWidth = fbo->getWidth();
  float canvasHeight = fbo->getHeight();
  float drawWidth = imageWidth * scale->value;
  float drawHeight = imageHeight * scale->value;
  float drawX = 0;
  float drawY = 0;

  // Calculate scaling based on mode
  float canvasAspect = canvasWidth / canvasHeight;
  float imageAspect = imageWidth / imageHeight;

  switch(mode->intValue) {
    case 1: // Fill (stretch to fill)
      drawWidth = canvasWidth;
      drawHeight = canvasHeight;
      break;
    case 2: // Fit (fit within bounds)
      if (imageAspect > canvasAspect) {
        drawWidth = canvasWidth;
        drawHeight = canvasWidth / imageAspect;
      } else {
        drawHeight = canvasHeight;
        drawWidth = canvasHeight * imageAspect;
      }
      break;
    case 3: // Fill + Aspect (fill while maintaining aspect ratio)
      if (imageAspect > canvasAspect) {
        drawHeight = canvasHeight;
        drawWidth = canvasHeight * imageAspect;
      } else {
        drawWidth = canvasWidth;
        drawHeight = canvasWidth / imageAspect;
      }
      break;
    default: // Standard (manual scaling)
      {
        float posX = center->boolValue ? (canvasWidth - drawWidth) * 0.5f : translateX->value * canvasWidth;
        float posY = center->boolValue ? (canvasHeight - drawHeight) * 0.5f : translateY->value * canvasHeight;
        drawX = posX;
        drawY = posY;
      }
      break;
  }

  ofPushMatrix();

  // Apply flips if needed
  if (verticalFlip->boolValue) {
    ofTranslate(0, canvasHeight);
    ofScale(1, -1);
  }
  if (horizontalFlip->boolValue) {
    ofTranslate(canvasWidth, 0);
    ofScale(-1, 1);
  }

  // Draw the image with current transform settings
  image.draw(drawX, drawY, drawWidth, drawHeight);

  ofPopMatrix();
  fbo->end();
}

json ImageSource::serialize()
{
  json j;
  j["path"] = path;
  j["id"] = id;
  j["sourceName"] = sourceName;
  j["videoSourceType"] = VideoSource_image;
  j["settings"] = settings->serialize();

  // Save transform parameters
  j["mode"] = mode->intValue;
  j["scale"] = scale->value;
  j["translateX"] = translateX->value;
  j["translateY"] = translateY->value;
  j["center"] = center->boolValue;
  j["verticalFlip"] = verticalFlip->boolValue;
  j["horizontalFlip"] = horizontalFlip->boolValue;

  auto node = NodeLayoutView::getInstance()->nodeForShaderSourceId(id);
  if (node != nullptr)
  {
    j["x"] = node->position.x;
    j["y"] = node->position.y;
  }
  return j;
}

void ImageSource::drawSettings()
{
  if (fbo != nullptr && fbo->isAllocated()) {
    ImGui::Image((ImTextureID)fbo->getTexture().getTextureData().textureID, ImVec2(256.0, 144.0));
  }

  CommonViews::ShaderParameter(scale, nullptr);
  CommonViews::MultiSlider("Position", id, translateX, translateY, nullptr, nullptr);
  CommonViews::ShaderCheckbox(verticalFlip);
  CommonViews::ShaderCheckbox(horizontalFlip);
  
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 5.0);
  CommonViews::ShaderCheckbox(center);
  CommonViews::Selector(mode, mode->options);
}

void ImageSource::load(json j)
{
  if (!j.is_object())
  {
    log("Error hydrating ImageSource from json");
    return;
  }

  path = j["path"];
  VideoSourceService::getService()->startAccessingBookmarkPath(path);
  id = j["id"];
  sourceName = j["sourceName"];
  settings->load(j["settings"]);

  // Load transform parameters if they exist
  if (j.contains("mode")) mode->intValue = j["mode"];
  if (j.contains("scale")) scale->value = j["scale"];
  if (j.contains("translateX")) translateX->value = j["translateX"];
  if (j.contains("translateY")) translateY->value = j["translateY"];
  if (j.contains("center")) center->boolValue = j["center"];
  if (j.contains("verticalFlip")) verticalFlip->boolValue = j["verticalFlip"];
  if (j.contains("horizontalFlip")) horizontalFlip->boolValue = j["horizontalFlip"];

  auto node = NodeLayoutView::getInstance()->nodeForShaderSourceId(id);
  if (node != nullptr)
  {
    node->position.x = j["x"];
    node->position.y = j["y"];
  }
}

// 
//  FileSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#include "FileSource.hpp"
#include "NodeLayoutView.hpp"

void FileSource::setup() {
  player.setPixelFormat(OF_PIXELS_RGB);
  player.load(path);
  auto wd = player.getWidth();
  player.play();
  player.setVolume(0.5);
  frameTexture = std::make_shared<ofTexture>();
  frameTexture->allocate(1280, 720, GL_RGB);
}

void FileSource::saveFrame() {
  player.update();
  if (player.isFrameNew()) {
    frameTexture->loadData(player.getPixels());
  }
}

void FileSource::load(json j) {
  if (!j.is_object()) {
    log("Error hydrating WebcamSource from json");
    return;
  }
  
  path = j["path"];
  id = j["id"];
  sourceName = j["sourceName"];
}

json FileSource::serialize() {
  json j;
  j["path"] = path;
  j["id"] = id;
  j["sourceName"] = sourceName;
  j["videoSourceType"] = VideoSource_file;
  j["x"] = NodeLayoutView::getInstance()->nodeForShaderSourceId(id)->position.x;
  j["y"] = NodeLayoutView::getInstance()->nodeForShaderSourceId(id)->position.y;
  
  return j;
}

void FileSource::drawSettings() {
  player.setVolume(volume->value);
  CommonViews::Slider("Volume", "##volume", volume);
}

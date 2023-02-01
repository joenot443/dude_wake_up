// 
//  FileSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#include "FileSource.hpp"

void FileSource::setup() {
  player.setPixelFormat(OF_PIXELS_RGB);
  player.setVolume(0.0);
  player.load(path);
  auto wd = player.getWidth();
  player.play();
  player.setVolume(0.0);
  frameTexture = std::make_shared<ofTexture>();
  frameTexture->allocate(1280, 720, GL_RGB);
  frameBuffer.bind(GL_SAMPLER_2D_RECT);
  frameBuffer.allocate(1280*720*4, GL_STATIC_COPY);
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
  return j;
}

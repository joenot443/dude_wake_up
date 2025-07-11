//
//  FileSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#include "FileSource.hpp"
#include "VideoSourceService.hpp"
#include "NodeLayoutView.hpp"
#include "Fonts.hpp"
#include "CommonViews.hpp"
#include "LayoutStateService.hpp"

void FileSource::setup()
{
  applyRanges();
  player.setPixelFormat(OF_PIXELS_RGB);
  player.load(path);
  player.setPosition(start);
  player.play();
  player.setVolume(0.5);
  player.setLoopState(OF_LOOP_NORMAL);
  playing = true;
  fbo->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y);
  optionalFbo->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y);
  sliderPosition->value = settings->start->value;
  position = start;
  maskShader.load("shaders/ColorKeyMaskMaker");
}

void FileSource::updateSettings()
{
  settings->width->value = player.getWidth();
  settings->height->value = player.getHeight();
}

void FileSource::saveFrame()
{
  // If our width or height has changed, setup again
  if (fbo->getWidth() != LayoutStateService::getService()->resolution.x ||
      fbo->getHeight() != LayoutStateService::getService()->resolution.y) {
    setup();
  }
  
  if (player.getSpeed() != speed->value) {
    player.setSpeed(speed->value);
  }
  player.setLoopState(OF_LOOP_NONE);
  //  player.setLoopState(boomerang->boolValue ? OF_LOOP_PALINDROME : OF_LOOP_NORMAL);
  player.update();
  updatePlaybackPosition();
  
  if (!player.isFrameNew())
    return;
  
  fbo->begin();
  ofClear(0, 0, 0, 255);
  player.draw(0, 0, fbo->getWidth(), fbo->getHeight());
  fbo->end();
}

void FileSource::load(json j)
{
  if (!j.is_object())
  {
    log("Error hydrating WebcamSource from json");
    return;
  }

  path = j["path"];
  VideoSourceService::getService()->startAccessingBookmarkPath(path);
  id = j["id"];
  sourceName = j["sourceName"];
  mute->boolValue = j["mute"];
  volume->value = j["volume"];
  settings->load(j["settings"]);
  boomerang->boolValue = j["boomerang"];
  start = settings->start->value;
  end = settings->end->value;
  applyRanges();
}

json FileSource::serialize()
{
  json j;
  j["path"] = path;
  j["id"] = id;
  j["sourceName"] = sourceName;
  j["videoSourceType"] = VideoSource_file;
  j["mute"] = mute->boolValue;
  j["volume"] = volume->value;
  j["boomerang"] = boomerang->boolValue;
  j["settings"] = settings->serialize();
  auto node = NodeLayoutView::getInstance()->nodeForShaderSourceId(id);
  if (node != nullptr)
  {
    j["x"] = node->position.x;
    j["y"] = node->position.y;
  }
  return j;
}

void FileSource::drawSettings()
{
  if (mute->value < 0.5)
    player.setVolume(volume->value);

  CommonViews::Slider("Volume", "##volume", volume);
  ImGui::SameLine();
  
  // Play / Pause button
  if (CommonViews::PlayPauseButton("##filePlayPause", playing, ImVec2(25.0, 25.0))) {
    playing = !playing;
    // Toggle paused state and update the underlying video player
    player.setPaused(!playing);
    if (playing) {
      player.play();
    }
  }
  ImGui::SameLine();
  
  auto muteIcon = mute->value > 0.5 ? ICON_MD_VOLUME_MUTE : ICON_MD_VOLUME_UP;
  if (CommonViews::IconButton(muteIcon, "##mute"))
  {
    mute->value = mute->value > 0.5 ? 0.0 : 1.0;
    if (mute->value > 0.5)
    {
      player.setVolume(0.0);
    }
    else
    {
      player.setVolume(volume->value);
    }
  }

  // Draw a slider to control the video playback position
  if (CommonViews::PlaybackSlider(sliderPosition, player.getDuration())) {
    applyRanges();
  }
  
  CommonViews::PlaybackRangeSlider("Playback Range", "##playbackRange", settings->start, settings->end, player.getDuration(), playbackRangeDirty());
  
  if (playbackRangeDirty())
  {
    if (ImGui::Button("Apply Ranges")) {
      applyRanges();
    }
  }
  
//  CommonViews::ShaderCheckbox(boomerang);
}

void FileSource::updatePlaybackPosition()
{
  if (abs(player.getPosition() - end) < 0.01 || player.getIsMovieDone())
  {
    if (boomerang->boolValue && player.getSpeed() > 0.0 && direction) {
      log("Boomeranging");
      player.setPosition(end);
      player.setSpeed(-speed->value);
      player.play();
      direction = false;
    } else if (!boomerang->boolValue) {
      log("Moving to %f", start);
      sliderPosition->value = start;
      position = start;
      player.setPosition(start);
      player.play();
    }
    
    if (!player.isPlaying()) {
      player.play();
    }
    
    return;
  }
  
  // Restarting Boomerang
  if (abs(player.getPosition() - start) < 0.01 && !direction && boomerang->boolValue) {
    player.setPosition(start);
    player.setSpeed(1.0);
    player.play();
    direction = true;
    return;
  }
  
  if (!player.isPlaying() && playing) {
    player.play();
  }
  
  // Slider position has changed a significant amount
  if (((float) fabs(position - sliderPosition->value)) > 0.01) {
    log("Moving slider");
    position = sliderPosition->value;
    player.setPosition(position);
    player.play();
  } else {
    position = player.getPosition();
    sliderPosition->value = position;
  }
}

void FileSource::applyRanges()
{
  start = settings->start->value;
  end = settings->end->value;

  // If the player is currently within the new playback ranges, continue.
  // Otherwise, restart it at the start point
  if (player.getPosition() > start && player.getPosition() < end)
  {
    player.play();
  }
  else
  {
    player.stop();
    player.setPosition(start);
    sliderPosition->value = start;
    position = start;
    player.play();
  }
}

void FileSource::teardown()
{
  player.stop();
  player.close();
}

bool FileSource::playbackRangeDirty()
{
  return start != settings->start->value || end != settings->end->value;
}

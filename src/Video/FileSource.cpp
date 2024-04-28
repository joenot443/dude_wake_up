//
//  FileSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#include "FileSource.hpp"
#include "NodeLayoutView.hpp"
#include "Fonts.hpp"

void FileSource::setup()
{
  applyRanges();
  player.setPixelFormat(OF_PIXELS_RGB);
  player.load(path);
  player.setPosition(start);
  player.play();
  player.setVolume(0.5);
  player.setSpeed(1.0);
  player.setLoopState(OF_LOOP_NORMAL);
//	updateSettings();
  fbo->allocate(settings->width->value, settings->height->value);
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
  player.setLoopState(boomerang->boolValue ? OF_LOOP_PALINDROME : OF_LOOP_NORMAL);
  player.update();
  updatePlaybackPosition();

  if (!player.isFrameNew())
    return;

  if (settings->maskEnabled->boolValue == true)
  {
    fbo->begin();
    maskShader.begin();
    maskShader.setUniformTexture("tex", player.getTexture(), 0);
    maskShader.setUniform1f("time", ofGetElapsedTimef());
    maskShader.setUniform2f("dimensions", fbo->getWidth(), fbo->getHeight());
    maskShader.setUniform1i("drawTex", 1);
    maskShader.setUniform4f("chromaKey",
                            settings->maskColor->color->data()[0],
                            settings->maskColor->color->data()[1],
                            settings->maskColor->color->data()[2], 1.0);
    maskShader.setUniform1f("tolerance", settings->maskTolerance->value);
    maskShader.setUniform1i("invert", settings->invert->boolValue);
    ofClear(0, 0, 0, 255);
    ofClear(0, 0, 0, 0);

    player.draw(0, 0, fbo->getWidth(), fbo->getHeight());
    maskShader.end();
    fbo->end();
  }
  else
  {
    fbo->begin();
    ofClear(0, 0, 0, 255);
    player.draw(0, 0, fbo->getWidth(), fbo->getHeight());
    fbo->end();
  }
}

void FileSource::load(json j)
{
  if (!j.is_object())
  {
    log("Error hydrating WebcamSource from json");
    return;
  }

  path = j["path"];
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
  CommonViews::PlaybackSlider(sliderPosition, player.getDuration());
  CommonViews::PlaybackRangeSlider("Playback Range", "##playbackRange", settings->start, settings->end, player.getDuration(), playbackRangeDirty());
  
  if (playbackRangeDirty())
  {
    if (ImGui::Button("Apply Ranges")) {
      applyRanges();
    }
  }
  
  CommonViews::ShaderCheckbox(boomerang);
  drawMaskSettings();
}

void FileSource::updatePlaybackPosition()
{
  if (player.getPosition() > (end - 0.01) || player.getIsMovieDone())
  {
    if (boomerang->boolValue) {
      player.setSpeed(-1.0);
    } else {
      sliderPosition->value = start;
      position = start;
      player.setPosition(start);
    }
    return;
  }
  
  if (player.getPosition() < start ) {
    player.setSpeed(1.0);
    player.setPosition(start);
    position = start;
    return;
  }
  
  // Slider position has changed a significant amount
  if (((float) fabs(position - sliderPosition->value)) > 0.01) {
    position = sliderPosition->value;
    player.setPosition(position);
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

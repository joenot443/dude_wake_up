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
  player.setPixelFormat(OF_PIXELS_RGB);
  player.load(path);
  player.play();
  player.setVolume(0.5);
  frameTexture = std::make_shared<ofTexture>();
  frameTexture->allocate(1280, 720, GL_RGB);
  fbo.allocate(1280, 720, GL_RGB);
  position->value = 0.0;
}

void FileSource::saveFrame()
{
  player.update();
  if (player.isFrameNew())
  {
    fbo.begin();
    player.draw(0, 0, 1280, 720);
    fbo.end();
    frameTexture = std::make_shared<ofTexture>(fbo.getTexture());
  }
  updatePlaybackPosition();
}

void FileSource::updatePlaybackPosition()
{
  // Only update the video player if the position has changed by more than 0.5%
  if (abs(position->value - player.getPosition()) > 0.02)
  {
    player.setPosition(position->value);
    player.play();
  }
  else
  {
    position->value = player.getPosition();
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
  mute->value = j["mute"];
  volume->value = j["volume"];
}

json FileSource::serialize()
{
  json j;
  j["path"] = path;
  j["id"] = id;
  j["sourceName"] = sourceName;
  j["videoSourceType"] = VideoSource_file;
  j["mute"] = mute->value;
  j["volume"] = volume->value;
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
  CommonViews::Slider("Playback Position", "##playbackPosition", position);
}

void FileSource::teardown() {
  player.stop();
  player.close();
}

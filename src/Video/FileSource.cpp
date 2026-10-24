//
//  FileSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#include "FileSource.hpp"
#include "FileSourceCacheBuilder.hpp"
#include "VideoSourceService.hpp"
#include "NodeLayoutView.hpp"
#include "Fonts.hpp"
#include "CommonViews.hpp"
#include "LayoutStateService.hpp"
#include "Console.hpp"
#include "ofFileUtils.h"
#include <algorithm>
#include <cmath>

static constexpr uint64_t kCacheFileSizeThresholdBytes = 100ULL * 1024 * 1024;
static constexpr uint64_t kCacheMemoryBudgetBytes      = 4ULL * 1024ULL * 1024ULL * 1024ULL; // 4 GB
static constexpr int      kCacheStride                 = 1; // keep every Nth source frame

void FileSource::setup()
{
  if (path.length() == 0) {
    log("FileSource::setup() - path is empty, skipping");
    return;
  }

  teardownFrameCache();
  playbackMode = FilePlaybackMode_Streaming;

  applyRanges();
  // No close() first: load() keeps a file that's already open (setup() re-runs
  // on every resolution change) and closes the old one when the path differs.
  // RGBA: ofAVFoundationPlayer's getPixels() can use vImagePermuteChannels_ARGB8888
  // (SIMD channel shuffle, same memory layout) instead of vImageConvert_ARGB8888toRGB888
  // (drops alpha + repacks rows). 30-50% less CPU per frame on the decode side.
  player.setPixelFormat(OF_PIXELS_RGBA);
  bool loaded = player.load(path);
  log("FileSource::setup() - loaded %s from path: %s", loaded ? "successfully" : "FAILED", path.c_str());
  if (!loaded) return;
  // Match canvas to video native size — but only on the first setup() for
  // this source instance. Doing it every time means the resolution-update
  // subscription cascade reloads every source's player on each new add.
  if (!didApplyVideoResolution &&
      player.getWidth() > 0 && player.getHeight() > 0) {
    auto currentRes = LayoutStateService::getService()->resolution;
    if (currentRes.x != player.getWidth() || currentRes.y != player.getHeight()) {
      LayoutStateService::getService()->setCustomResolution(player.getWidth(), player.getHeight());
    }
    didApplyVideoResolution = true;
  }
  player.setVolume(0.0);
  mute->setValue(1.0);
  player.setPosition(start);
  player.play();
  player.setLoopState(OF_LOOP_NORMAL);
  playing = true;
  fbo->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y);
  optionalFbo->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y);
  sliderPosition->value = settings->start->value;
  position = start;
  maskShader.load("shaders/ColorKeyMaskMaker");

  // Capture native source metadata for tier selection.
  nativeWidth      = player.getWidth();
  nativeHeight     = player.getHeight();
  nativeFrameCount = player.getTotalNumFrames();
  nativeDuration   = player.getDuration();
  nativeFps        = (nativeDuration > 0.0f && nativeFrameCount > 0)
                       ? static_cast<float>(nativeFrameCount) / nativeDuration
                       : 30.0f;

  lastTickWallTime        = ofGetElapsedTimef();
  streamingLastUpdateTime = 0.0;
  cachedClockSeconds      = start * nativeDuration;

  uint64_t fsize = fileSizeBytes();
  uint64_t mem   = estimatedCacheBytes();
  bool eligibleForCache =
      nativeDuration > 0.0f &&
      nativeFrameCount > 0 &&
      nativeWidth > 0 && nativeHeight > 0 &&
      fsize > 0 && fsize < kCacheFileSizeThresholdBytes &&
      mem > 0 && mem < kCacheMemoryBudgetBytes;

#ifndef TARGET_OSX
  // The Windows player keeps its own frame cache (in RAM, built in the
  // background); see IOSurfaceVideoPlayer_win.cpp.
  eligibleForCache = false;
#endif
  if (eligibleForCache) {
    if (buildFrameCache()) {
      playbackMode = FilePlaybackMode_Cached;
      log("FileSource: cached %d frames (~%.1f MB est) — %s",
          (int)cachedFrames.size(), mem / 1048576.0, path.c_str());
    } else {
      teardownFrameCache();
      playbackMode = FilePlaybackMode_Streaming;
      log("FileSource: cache build failed, falling back to streaming — %s", path.c_str());
    }
  } else {
    log("FileSource: streaming mode (file %.1f MB, est cache %.1f MB) — %s",
        fsize / 1048576.0, mem / 1048576.0, path.c_str());
  }
}

void FileSource::updateSettings()
{
  settings->width->value = player.getWidth();
  settings->height->value = player.getHeight();
}

void FileSource::saveFrame()
{
  const bool canUseCache = useCachedMode &&
                           playbackMode == FilePlaybackMode_Cached &&
                           !cachedFrames.empty();
  if (canUseCache) {
    saveFrameCached();
  } else {
    saveFrameStreaming();
  }
}

void FileSource::saveFrameStreaming()
{
  // If resolution changed, just reallocate FBOs — don't reload the video player
  if (fbo->getWidth() != LayoutStateService::getService()->resolution.x ||
      fbo->getHeight() != LayoutStateService::getService()->resolution.y) {
    fbo->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y);
    optionalFbo->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y);
  }

  if (player.getSpeed() != speed->value) {
    player.setSpeed(speed->value);
  }
  player.setLoopState(OF_LOOP_NONE);

  // FPS-aware decode: only ask the player to advance when the video's native
  // frame interval has elapsed, scaled by playback speed.
  double now      = ofGetElapsedTimef();
  float  absSpeed = std::max(0.01f, std::fabs(speed->value));
  float  fps      = std::max(1.0f, nativeFps);
  float  interval = 1.0f / (fps * absSpeed);
  if (now - streamingLastUpdateTime >= interval) {
    player.update();
    streamingLastUpdateTime = now;
  }

  updatePlaybackPosition();

  if (!player.isFrameNew())
    return;

  fbo->begin();
  ofClear(0, 0, 0, 255);
  player.draw(0, 0, fbo->getWidth(), fbo->getHeight());
  fbo->end();
}

void FileSource::saveFrameCached()
{
  if (fbo->getWidth() != LayoutStateService::getService()->resolution.x ||
      fbo->getHeight() != LayoutStateService::getService()->resolution.y) {
    fbo->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y);
    optionalFbo->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y);
  }

  if (cachedFrames.empty() || nativeDuration <= 0.0f) {
    return;
  }

  double now = ofGetElapsedTimef();
  double dt  = now - lastTickWallTime;
  lastTickWallTime = now;
  // Guard against huge dt on first tick or after pauses in update.
  if (dt < 0.0 || dt > 1.0) dt = 0.0;

  if (playing) {
    float effectiveSpeed = speed->value * (direction ? 1.0f : -1.0f);
    cachedClockSeconds += static_cast<float>(dt) * effectiveSpeed;
  }

  float startSec = start * nativeDuration;
  float endSec   = end   * nativeDuration;
  if (endSec < startSec) std::swap(startSec, endSec);

  // Range / boomerang wrap.
  if (cachedClockSeconds >= endSec) {
    if (boomerang->boolValue && direction) {
      direction = false;
      cachedClockSeconds = endSec;
    } else {
      cachedClockSeconds = startSec;
    }
  } else if (cachedClockSeconds <= startSec) {
    if (boomerang->boolValue && !direction) {
      direction = true;
      cachedClockSeconds = startSec;
    } else if (!boomerang->boolValue && speed->value < 0.0f) {
      cachedClockSeconds = endSec;
    } else {
      cachedClockSeconds = startSec;
    }
  }

  // Slider scrub support.
  if (std::fabs(position - sliderPosition->value) > 0.01f) {
    cachedClockSeconds = sliderPosition->value * nativeDuration;
    position = sliderPosition->value;
  } else {
    position = nativeDuration > 0.0f ? cachedClockSeconds / nativeDuration : 0.0f;
    sliderPosition->value = position;
  }

  // Stride: cache holds every Nth source frame, so map source-frame index
  // through the stride to land on the correct cached slot.
  int origIdx = static_cast<int>(cachedClockSeconds * nativeFps);
  int idx = origIdx / std::max(1, kCacheStride);
  idx = std::clamp(idx, 0, static_cast<int>(cachedFrames.size()) - 1);

  fbo->begin();
  ofClear(0, 0, 0, 255);
  if (cachedFrames[idx]) {
    cachedFrames[idx]->draw(0, 0, fbo->getWidth(), fbo->getHeight());
  }
  fbo->end();
}

bool FileSource::buildFrameCache()
{
  if (nativeFrameCount <= 0 || nativeWidth <= 0 || nativeHeight <= 0) {
    log("buildFrameCache: invalid metadata (w=%d h=%d frames=%d)",
        nativeWidth, nativeHeight, nativeFrameCount);
    return false;
  }

  auto extracted = nottawa::buildFrameCacheFromAsset(path, kCacheStride);
  if (!extracted.success) {
    log("buildFrameCache: extractor failed — %s (saw %d source frames)",
        extracted.errorMessage.c_str(), extracted.sourceFramesSeen);
    cachedFrames.clear();
    return false;
  }

  cachedFrames = std::move(extracted.frames);

  const int expectedFrames = (nativeFrameCount + kCacheStride - 1) / kCacheStride;
  const int threshold = std::max(1, static_cast<int>(expectedFrames * 0.5f));
  const bool ok = static_cast<int>(cachedFrames.size()) >= threshold;

  log("buildFrameCache: %s — captured %d/%d frames (saw %d source frames)",
      ok ? "ACCEPTED" : "REJECTED",
      (int)cachedFrames.size(), expectedFrames, extracted.sourceFramesSeen);

  if (!ok) cachedFrames.clear();
  return ok;
}

void FileSource::teardownFrameCache()
{
  cachedFrames.clear();
}

uint64_t FileSource::estimatedCacheBytes() const
{
  if (nativeWidth <= 0 || nativeHeight <= 0 || nativeFrameCount <= 0) return 0;
  const int stride = std::max(1, kCacheStride);
  const uint64_t expectedFrames =
      (static_cast<uint64_t>(nativeFrameCount) + stride - 1) / stride;
  return static_cast<uint64_t>(nativeWidth) *
         static_cast<uint64_t>(nativeHeight) *
         4ULL *
         expectedFrames;
}

uint64_t FileSource::fileSizeBytes() const
{
  if (path.empty()) return 0;
  ofFile f(path);
  if (!f.exists()) return 0;
  return static_cast<uint64_t>(f.getSize());
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
  if (node != nullptr) {
    j["x"] = node->position.x;
    j["y"] = node->position.y;
  } else {
    j["x"] = origin.x;
    j["y"] = origin.y;
  }
  return j;
}

void FileSource::drawSettings()
{
  if (mute->value < 0.5)
    player.setVolume(volume->value);

  // Volume slider with mute button on the same line
  CommonViews::Slider("Volume", "##volume", volume, ImGui::GetContentRegionAvail().x - 100.0);
  ImGui::SameLine();

  std::string muteIcon = mute->boolValue ? "unmute.png" : "mute.png";
  if (CommonViews::ImageButton(mute->paramId, muteIcon, ImVec2(25.0, 25.0), ImVec2(2.0, 2.0)))
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

  // Playback slider (already has its own reset button with SameLine)
  if (CommonViews::PlaybackSlider(sliderPosition, player.getDuration(), ImGui::GetContentRegionAvail().x - 100.0)) {
    applyRanges();
  }

  ImGui::SameLine();
  // Play / Pause button on the same line as playback slider
  if (CommonViews::PlayPauseButton("##filePlayPause", playing, ImVec2(25.0, 25.0))) {
    playing = !playing;
    // Toggle paused state and update the underlying video player
    player.setPaused(!playing);
    if (playing) {
      player.play();
    }
  }
  ImGui::NewLine();
  ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 10.0);
  CommonViews::H3Title("Range");

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

  // Cached-mode timeline must follow the new range too.
  if (playbackMode == FilePlaybackMode_Cached && nativeDuration > 0.0f) {
    float startSec = start * nativeDuration;
    float endSec   = end   * nativeDuration;
    if (endSec < startSec) std::swap(startSec, endSec);
    cachedClockSeconds = std::clamp(cachedClockSeconds, startSec, endSec);
  }
}

void FileSource::teardown()
{
  // Snapshot use_counts so we can tell whether other holders are pinning
  // these textures (which would prevent glDeleteTextures from running).
  size_t pinned = 0;
  for (auto& t : cachedFrames) {
    if (t && t.use_count() > 1) ++pinned;
  }
  log("FileSource::teardown(%s) — releasing %d cached frames (%zu pinned by other holders)",
      sourceName.c_str(), (int)cachedFrames.size(), pinned);

  teardownFrameCache();
  player.stop();
  player.close();

  // Eagerly release the GL-side FBOs. The shared_ptr<ofFbo> wrappers themselves
  // stay alive (the source is held by the undo stack), but ofFbo::clear() drops
  // the underlying GL FBO + attached textures + depth/stencil buffers. Without
  // this, ~16 MB per source lingers for the lifetime of the undo entry.
  if (fbo) fbo->clear();
  if (optionalFbo) optionalFbo->clear();
  maskShader.unload();
}

bool FileSource::playbackRangeDirty()
{
  return start != settings->start->value || end != settings->end->value;
}

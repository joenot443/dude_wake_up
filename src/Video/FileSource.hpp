//
//  FileSource.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#ifndef FileSource_hpp
#define FileSource_hpp

#include <stdio.h>
#include <vector>
#include "VideoSource.hpp"
#include "IOSurfaceVideoPlayer.hpp"

enum FilePlaybackMode {
  FilePlaybackMode_Cached,
  FilePlaybackMode_Streaming
};

class FileSource : public VideoSource
{
public:
  FileSource(std::string id, std::string name, std::string path, VideoSourceType type = VideoSource_file)
  : VideoSource(id, name, type),
  path(path),
  volume(std::make_shared<Parameter>("Volume", 0.5, 0.0, 1.0)),
  sliderPosition(std::make_shared<Parameter>("Position", 0.0, 0.0, 1.0)),
  mute(std::make_shared<Parameter>("Mute", 0.0, 0.0, 1.0)),
  speed(std::make_shared<Parameter>("Speed", 1.0, 0.0, 2.0)),
  boomerang(std::make_shared<Parameter>("Boomerang", ParameterType_Bool)),
  position(0.0),
  start(0.0),
  end(1.0){};
  
  std::string path;
  // Custom AVFoundation player that delivers frames as zero-copy GL_TEXTURE_2D
  // textures via IOSurface — avoids the CPU pixel decode + upload that
  // ofVideoPlayer does. See IOSurfaceVideoPlayer.{hpp,mm}.
  IOSurfaceVideoPlayer player;
  ofShader maskShader;
  std::shared_ptr<Parameter> volume;
  std::shared_ptr<Parameter> sliderPosition;
  std::shared_ptr<Parameter> boomerang;
  std::shared_ptr<Parameter> speed;
  std::shared_ptr<Parameter> mute;
  bool playing = false;
  // True once we've called setCustomResolution() for this source. The
  // resolution-update subscription re-runs setup() on every notify; without
  // this gate, FileSource::setup → setCustomResolution → notify → setup
  // ping-pongs forever between two videos with different native sizes.
  bool didApplyVideoResolution = false;
  void setup() override;
  void updateSettings();
  void saveFrame() override;
  json serialize() override;
  void load(json j) override;
  void drawSettings() override;
  void teardown() override;
  
  float position;
  float start;
  float end;
  
  bool direction = true;
  bool playbackRangeDirty();
  void validateSettings();
  void applyRanges();
  void updatePlaybackPosition();

  // Two-tier playback (see plan: cached for small files, streaming fallback)
  FilePlaybackMode playbackMode = FilePlaybackMode_Streaming;

  // Runtime A/B toggle. When true, the cached path is used if a cache was
  // built; when false, force streaming regardless of what setup() decided.
  bool useCachedMode = true;

  int    nativeFrameCount = 0;
  float  nativeFps        = 30.0f;
  float  nativeDuration   = 0.0f;
  int    nativeWidth      = 0;
  int    nativeHeight     = 0;

  std::vector<std::shared_ptr<ofTexture>> cachedFrames;
  float  cachedClockSeconds = 0.0f;
  double lastTickWallTime   = 0.0;

  double streamingLastUpdateTime = 0.0;

  bool     buildFrameCache();
  void     teardownFrameCache();
  void     saveFrameCached();
  void     saveFrameStreaming();
  uint64_t estimatedCacheBytes() const;
  uint64_t fileSizeBytes() const;

  LIFECYCLE_TRACK("FileSource");
};

#endif

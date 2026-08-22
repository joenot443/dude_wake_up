// AVFRecorderWrapper_win.cpp
// Non-macOS placeholder. Real libav-based implementation lands in Phase 4
// (see WINDOWS_PORT_PLAN.md and scratchpad/LibavRecorder_win.cpp for the
// sketch). Exists so the empty-shell Windows build links; recording is a
// no-op until Phase 4.

#ifndef TARGET_OSX

#include "AVFRecorderWrapper.hpp"
#include "ofLog.h"

namespace {
  void warnOnce(const char* method) {
    static bool warned = false;
    if (!warned) {
      ofLogWarning("AVFRecorderWrapper_win") << "stub: " << method
        << " — libav implementation deferred to Phase 4";
      warned = true;
    }
  }
}

AVFRecorderWrapper::AVFRecorderWrapper()
  : recorderInstance(nullptr),
    cvPixelBufferPool(nullptr),
    sourceWidth(0), sourceHeight(0),
    videoWidth(0),  videoHeight(0),
    frameDurationSeconds(0.f),
    videoFps(0),
    frameCount(0),
    sourcePixelFormat(0) {}

AVFRecorderWrapper::~AVFRecorderWrapper() {}

bool AVFRecorderWrapper::setup(const std::string& path, int w, int h, float fps, const std::string& /*codec*/) {
  filePath = path; sourceWidth = videoWidth = w; sourceHeight = videoHeight = h;
  videoFps = static_cast<int32_t>(fps + 0.5f);
  warnOnce("setup");
  return false;
}

bool AVFRecorderWrapper::setupAudio(float sampleRate, int channels) {
  audioSampleRate = sampleRate; audioChannels = channels;
  warnOnce("setupAudio");
  return false;
}

void AVFRecorderWrapper::setMaxDuration(float seconds) { maxDurationSeconds = seconds; }

void AVFRecorderWrapper::setOnRecordingComplete(std::function<void(bool, const std::string&)> cb) {
  onRecordingComplete = std::move(cb);
}

bool AVFRecorderWrapper::start()                                            { warnOnce("start");           return false; }
bool AVFRecorderWrapper::addFrame(const ofPixels& /*pixels*/)               { warnOnce("addFrame");        return false; }
bool AVFRecorderWrapper::addAudioSamples(const float* /*s*/, int /*n*/)     { warnOnce("addAudioSamples"); return false; }
bool AVFRecorderWrapper::addAudioSamples(const std::vector<float>& /*s*/)   { warnOnce("addAudioSamples"); return false; }
void AVFRecorderWrapper::stop(std::function<void(bool)> cb)                 { warnOnce("stop"); if (cb) cb(false); }

bool  AVFRecorderWrapper::isRecording()   const { return false; }
float AVFRecorderWrapper::getElapsedTime() const { return 0.f; }

#endif // !TARGET_OSX

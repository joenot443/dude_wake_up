// AVFRecorderWrapper_win.cpp
//
// Windows thin delegate over LibavRecorder — keeps the public
// AVFRecorderWrapper interface bit-for-bit identical to the macOS AVFoundation
// path, so OutputWindow / ShareStrandView / any other caller never has to
// know which backend they're driving. The libav-specific fields live inside
// LibavRecorder itself; `recorderInstance` on the wrapper is re-used as
// LibavRecorder* on Windows.

#ifndef TARGET_OSX

#include "AVFRecorderWrapper.hpp"
#include "LibavRecorder.hpp"

static LibavRecorder* impl(void* p) { return static_cast<LibavRecorder*>(p); }

AVFRecorderWrapper::AVFRecorderWrapper()
  : recorderInstance(new LibavRecorder()),
    cvPixelBufferPool(nullptr),
    sourceWidth(0), sourceHeight(0),
    videoWidth(0),  videoHeight(0),
    frameDurationSeconds(0.f),
    videoFps(0),
    frameCount(0),
    sourcePixelFormat(0) {}

AVFRecorderWrapper::~AVFRecorderWrapper() {
  delete impl(recorderInstance);
  recorderInstance = nullptr;
}

bool AVFRecorderWrapper::setup(const std::string& path, int w, int h, float fps,
                               const std::string& codec) {
  filePath      = path;
  sourceWidth   = videoWidth  = w;
  sourceHeight  = videoHeight = h;
  frameDurationSeconds = (fps > 0.f) ? (1.f / fps) : 0.f;
  videoFps      = static_cast<int32_t>(fps + 0.5f);
  return impl(recorderInstance)->setup(path, w, h, fps, codec);
}

bool AVFRecorderWrapper::setupAudio(float sampleRate, int channels) {
  audioSampleRate = sampleRate;
  audioChannels   = channels;
  bool ok = impl(recorderInstance)->setupAudio(sampleRate, channels);
  audioEnabled = ok;
  return ok;
}

void AVFRecorderWrapper::setMaxDuration(float seconds) {
  maxDurationSeconds = seconds;
  impl(recorderInstance)->setMaxDuration(seconds);
}

void AVFRecorderWrapper::setOnRecordingComplete(
    std::function<void(bool, const std::string&)> cb) {
  onRecordingComplete = std::move(cb);
  // Forward a shim so we can piggy-back cleanup here if we ever need to.
  impl(recorderInstance)->setOnRecordingComplete(
    [wrapperCb = onRecordingComplete](bool ok, const std::string& p) {
      if (wrapperCb) wrapperCb(ok, p);
    });
}

bool AVFRecorderWrapper::start()                                 { return impl(recorderInstance)->start(); }
bool AVFRecorderWrapper::addFrame(const ofPixels& pixels)        { return impl(recorderInstance)->addFrame(pixels); }
bool AVFRecorderWrapper::addAudioSamples(const float* s, int n)  { return impl(recorderInstance)->addAudioSamples(s, n); }
bool AVFRecorderWrapper::addAudioSamples(const std::vector<float>& s) {
  if (audioChannels <= 0) return false;
  return impl(recorderInstance)->addAudioSamples(
      s.data(), static_cast<int>(s.size()) / audioChannels);
}
void AVFRecorderWrapper::stop(std::function<void(bool)> cb)      { impl(recorderInstance)->stop(std::move(cb)); }

bool  AVFRecorderWrapper::isRecording()    const { return impl(recorderInstance)->isRecording(); }
float AVFRecorderWrapper::getElapsedTime() const { return impl(recorderInstance)->getElapsedTime(); }

#endif // !TARGET_OSX

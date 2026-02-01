#pragma once

#include <string>
#include <functional>
#include <vector>
#include <chrono>
#include "ofPixels.h"

class AVFRecorderWrapper {
public:
  AVFRecorderWrapper();
  ~AVFRecorderWrapper();

  // Configure the recorder
  // Codec examples: "h264", "hevc", "prores" (needs constants like AVVideoCodecType*)
  // Returns false on setup error
  bool setup(const std::string& filePath, int width, int height, float fps, const std::string& codec = "h264");

  // Configure audio recording (call after setup, before start)
  // Returns false on setup error
  bool setupAudio(float sampleRate, int channels);

  // Set maximum recording duration (optional). When exceeded, recording auto-stops.
  // Set to 0 or negative to disable (default: unlimited)
  void setMaxDuration(float seconds);

  // Set callback for when recording completes (including auto-stop due to duration limit)
  void setOnRecordingComplete(std::function<void(bool success, const std::string& filePath)> callback);

  // Start recording
  // Returns false if starting fails
  bool start();

  // Add a frame from ofPixels
  // Returns false if adding frame fails or if duration limit reached
  bool addFrame(const ofPixels& pixels);

  // Add audio samples (interleaved float32)
  // samples: interleaved audio data (L0, R0, L1, R1, ...)
  // numFrames: number of sample frames (total samples / channels)
  bool addAudioSamples(const float* samples, int numFrames);
  bool addAudioSamples(const std::vector<float>& samples);

  // Stop recording and finalize the file
  void stop(std::function<void(bool success)> completionCallback = nullptr);

  bool isRecording() const;
  bool hasAudio() const { return audioEnabled; }
  float getElapsedTime() const;
  float getMaxDuration() const { return maxDurationSeconds; }

private:
  // Opaque pointer to the Objective-C AVFRecorder instance
  void* recorderInstance;
  
  // For CVPixelBuffer creation/pooling
  void* cvPixelBufferPool; // CVPixelBufferPoolRef
  int sourceWidth;   // Input frame dimensions
  int sourceHeight;
  int videoWidth;    // Output video dimensions (may be scaled down)
  int videoHeight;
  float frameDurationSeconds;
  int32_t videoFps;  // Store FPS directly for timestamp calculation
  long long frameCount;
  std::chrono::steady_clock::time_point recordingStartTime; // For accurate timestamps
  unsigned int sourcePixelFormat; // e.g., GL_RGB, GL_RGBA, GL_BGRA
  std::string filePath;
  ofPixels scaledPixels; // Buffer for scaled frames

  // Duration limit
  float maxDurationSeconds = 0.0f; // 0 = unlimited
  std::function<void(bool success, const std::string& filePath)> onRecordingComplete;

  // Audio recording
  bool audioEnabled = false;
  float audioSampleRate = 44100.0f;
  int audioChannels = 2;
  long long audioSampleCount = 0; // For timestamp calculation

  // Private helper to create pixel buffers
  // Returns CVPixelBufferRef (as void* to avoid ObjC types in header)
  void* createPixelBufferFromOfPixels(const ofPixels& pixels);
  void releasePixelBufferPool();
  
  // Prevent copying
  AVFRecorderWrapper(const AVFRecorderWrapper&) = delete;
  AVFRecorderWrapper& operator=(const AVFRecorderWrapper&) = delete;
};

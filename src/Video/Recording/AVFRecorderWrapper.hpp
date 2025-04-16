#pragma once

#include <string>
#include <functional>
#include "ofPixels.h" // Assuming we'll pass ofPixels
// #include "ofTexture.h" // Alternative: if reading directly from texture

class AVFRecorderWrapper {
public:
  AVFRecorderWrapper();
  ~AVFRecorderWrapper();
  
  // Configure the recorder
  // Codec examples: "h264", "hevc", "prores" (needs constants like AVVideoCodecType*)
  // Returns false on setup error
  bool setup(const std::string& filePath, int width, int height, float fps, const std::string& codec = "h264");
  
  // Start recording
  // Returns false if starting fails
  bool start();
  
  // Add a frame from ofPixels
  // Returns false if adding frame fails
  bool addFrame(const ofPixels& pixels);
  
  // Optional: Add frame directly from texture (more complex but potentially faster)
  // bool addFrame(const ofTexture& texture);
  
  // Stop recording and finalize the file
  void stop(std::function<void(bool success)> completionCallback = nullptr);
  
  bool isRecording() const;
  float getElapsedTime() const;
  
private:
  // Opaque pointer to the Objective-C AVFRecorder instance
  void* recorderInstance;
  
  // For CVPixelBuffer creation/pooling
  void* cvPixelBufferPool; // CVPixelBufferPoolRef
  int videoWidth;
  int videoHeight;
  float frameDurationSeconds;
  long long frameCount;
  unsigned int sourcePixelFormat; // e.g., GL_RGB, GL_RGBA, GL_BGRA
  std::string filePath;
  
  // Private helper to create pixel buffers
  // Returns CVPixelBufferRef (as void* to avoid ObjC types in header)
  void* createPixelBufferFromOfPixels(const ofPixels& pixels);
  void releasePixelBufferPool();
  
  // Prevent copying
  AVFRecorderWrapper(const AVFRecorderWrapper&) = delete;
  AVFRecorderWrapper& operator=(const AVFRecorderWrapper&) = delete;
};

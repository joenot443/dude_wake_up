#include "AVFRecorderWrapper.hpp"
#include "AVFRecorder.h"
#include "ofLog.h"
#include "ofUtils.h"

#import <AVFoundation/AVFoundation.h>
#import <CoreVideo/CoreVideo.h> // For CVPixelBuffer
#import <Foundation/Foundation.h>

// Helper function to convert ofPixels format to CVPixelBuffer format constant
// This needs careful verification based on your ofPixels source data!
OSType ofFormatToCVPixelFormat(ofPixelFormat format) {
  switch(format) {
    case OF_PIXELS_RGB:  return kCVPixelFormatType_24RGB; // Common but check AVFoundation needs
    case OF_PIXELS_RGBA: return kCVPixelFormatType_32RGBA;
    case OF_PIXELS_BGRA: return kCVPixelFormatType_32BGRA; // Often preferred by AVFoundation
      // Add other formats as needed
    default:
      ofLogWarning("AVFRecorderWrapper") << "Unsupported ofPixelFormat: " << format << ". Defaulting to BGRA.";
      return kCVPixelFormatType_32BGRA;
  }
}

// Helper to convert string codec to AVVideoCodecType
AVVideoCodecType stringToAVCodec(const std::string& codecStr) {
  if (codecStr == "h264") return AVVideoCodecTypeH264;
  if (codecStr == "hevc") return AVVideoCodecTypeHEVC;
  if (codecStr == "prores" || codecStr == "prores422") return AVVideoCodecTypeAppleProRes422;
  if (codecStr == "prores4444") return AVVideoCodecTypeAppleProRes4444;
  // Add more codecs as needed
  ofLogWarning("AVFRecorderWrapper") << "Unsupported codec string: '" << codecStr << "'. Defaulting to H.264.";
  return AVVideoCodecTypeH264;
}

AVFRecorderWrapper::AVFRecorderWrapper() :
recorderInstance(nullptr),
cvPixelBufferPool(nullptr),
videoWidth(0),
videoHeight(0),
frameDurationSeconds(0.0f),
videoFps(30),
frameCount(0),
sourcePixelFormat(OF_PIXELS_RGB)
{}

AVFRecorderWrapper::~AVFRecorderWrapper() {
  if (isRecording()) {
    ofLogWarning("AVFRecorderWrapper") << "Destructor called while recording. Stopping recording.";
    // Blocking stop might be okay here if necessary, but check for deadlocks.
    stop(nullptr); // No callback needed
  }
  
  if (recorderInstance) {
    AVFRecorder *recorder = (__bridge_transfer AVFRecorder *)recorderInstance;
    // ARC will release the recorder when 'recorder' goes out of scope
    recorderInstance = nullptr;
  }
  
  releasePixelBufferPool();
}

void AVFRecorderWrapper::releasePixelBufferPool() {
  if (cvPixelBufferPool) {
    CVPixelBufferPoolRelease((CVPixelBufferPoolRef)cvPixelBufferPool);
    cvPixelBufferPool = nullptr;
    ofLogVerbose("AVFRecorderWrapper") << "Released CVPixelBufferPool.";
  }
}

bool AVFRecorderWrapper::setup(const std::string& path, int width, int height, float fps, const std::string& codec) {
  if (isRecording()) {
    ofLogError("AVFRecorderWrapper") << "Cannot setup while recording.";
    return false;
  }

  // Clean up previous instance if any
  if (recorderInstance) {
    AVFRecorder *recorder = (__bridge_transfer AVFRecorder *)recorderInstance;
    recorderInstance = nullptr;
  }
  releasePixelBufferPool();

  // Cap resolution at 720p while maintaining aspect ratio
  const int MAX_HEIGHT = 720;
  int outputWidth = width;
  int outputHeight = height;

  if (height > MAX_HEIGHT) {
    float scale = (float)MAX_HEIGHT / (float)height;
    outputHeight = MAX_HEIGHT;
    outputWidth = (int)(width * scale);
    // Ensure even dimensions for H.264
    outputWidth = (outputWidth / 2) * 2;
    ofLogNotice("AVFRecorderWrapper") << "Scaling from " << width << "x" << height
                                       << " to " << outputWidth << "x" << outputHeight;
  }

  // Store source dimensions for scaling in addFrame
  sourceWidth = width;
  sourceHeight = height;
  videoWidth = outputWidth;
  videoHeight = outputHeight;
  frameDurationSeconds = 1.0f / fps;
  videoFps = static_cast<int32_t>(fps);  // Store FPS as integer for timestamp calculation
  frameCount = 0;
  filePath = path;
  // FBOs return RGBA, but AVFoundation needs BGRA - we'll convert when copying
  sourcePixelFormat = OF_PIXELS_RGBA;
  OSType cvPixelFormat = kCVPixelFormatType_32BGRA; // AVFoundation preferred format

  NSURL *outputURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:filePath.c_str()]];
  CGSize videoSize = CGSizeMake(outputWidth, outputHeight);
  int32_t timeScale = static_cast<int32_t>(fps);
  
  AVFRecorder *recorder = [[AVFRecorder alloc] initWithURL:outputURL size:videoSize timeScale:timeScale];
  if (!recorder) {
    ofLogError("AVFRecorderWrapper") << "Failed to initialize AVFRecorder.";
    return false;
  }
  
  // Store as opaque pointer, transferring ownership to ARC
  recorderInstance = (__bridge_retained void *)recorder;
  
  // --- Setup CVPixelBufferPool ---
  NSDictionary *pixelBufferAttributes = @{
    (NSString *)kCVPixelBufferPixelFormatTypeKey: @(cvPixelFormat),
    (NSString *)kCVPixelBufferWidthKey: @(width),
    (NSString *)kCVPixelBufferHeightKey: @(height),
    (NSString *)kCVPixelBufferIOSurfacePropertiesKey: @{},
    // Add other attributes if needed (e.g., kCVPixelBufferBytesPerRowAlignmentKey)
  };
  
  CVReturn status = CVPixelBufferPoolCreate(kCFAllocatorDefault,
                                            NULL, // pool attributes
                                            (__bridge CFDictionaryRef)pixelBufferAttributes,
                                            (CVPixelBufferPoolRef *)&cvPixelBufferPool);
  
  if (status != kCVReturnSuccess || !cvPixelBufferPool) {
    ofLogError("AVFRecorderWrapper") << "Failed to create CVPixelBufferPool: " << status;
    AVFRecorder *rec = (__bridge_transfer AVFRecorder *)recorderInstance;
    recorderInstance = nullptr;
    return false;
  }
  ofLogVerbose("AVFRecorderWrapper") << "Created CVPixelBufferPool.";
  
  // --- Setup Video Input Settings ---
  AVVideoCodecType avCodec = stringToAVCodec(codec);

  // Calculate bitrate based on resolution (roughly 2-3 Mbps for 720p)
  int bitrate = (int)(outputWidth * outputHeight * 4.0); // ~4 bits per pixel
  if (bitrate > 4000000) bitrate = 4000000; // Cap at 4 Mbps
  if (bitrate < 1000000) bitrate = 1000000; // Min 1 Mbps

  NSDictionary *videoSettings = @{
    AVVideoCodecKey: avCodec,
    AVVideoWidthKey: @(outputWidth),
    AVVideoHeightKey: @(outputHeight),
    AVVideoCompressionPropertiesKey: @{
      AVVideoAverageBitRateKey: @(bitrate),
      AVVideoProfileLevelKey: AVVideoProfileLevelH264High41,
      AVVideoMaxKeyFrameIntervalKey: @(30), // Keyframe every 30 frames (1 sec at 30fps)
      AVVideoExpectedSourceFrameRateKey: @(fps),
    }
  };
  
  NSError *error = nil;
  BOOL setupSuccess = [recorder setupVideoInputWithSettings:videoSettings error:&error];
  
  if (!setupSuccess) {
    ofLogError("AVFRecorderWrapper") << "AVFRecorder setupVideoInput failed: " << [[error localizedDescription] UTF8String];
    AVFRecorder *rec = (__bridge_transfer AVFRecorder *)recorderInstance;
    recorderInstance = nullptr;
    releasePixelBufferPool();
    return false;
  }
  
  ofLogNotice("AVFRecorderWrapper") << "Setup complete for file: " << filePath << " (" << width << "x" << height << " @ " << fps << "fps, codec: " << codec << ")";
  return true;
}

bool AVFRecorderWrapper::setupAudio(float sampleRate, int channels) {
  if (!recorderInstance) {
    ofLogError("AVFRecorderWrapper") << "Cannot setup audio, video not set up.";
    return false;
  }
  if (isRecording()) {
    ofLogError("AVFRecorderWrapper") << "Cannot setup audio while recording.";
    return false;
  }

  AVFRecorder *recorder = (__bridge AVFRecorder *)recorderInstance;
  NSError *error = nil;
  BOOL success = [recorder setupAudioInputWithSampleRate:(Float64)sampleRate
                                                channels:channels
                                                   error:&error];
  if (!success) {
    ofLogError("AVFRecorderWrapper") << "Failed to setup audio: " << [[error localizedDescription] UTF8String];
    return false;
  }

  audioEnabled = true;
  audioSampleRate = sampleRate;
  audioChannels = channels;
  audioSampleCount = 0;

  ofLogNotice("AVFRecorderWrapper") << "Audio setup complete: " << sampleRate << " Hz, " << channels << " channels";
  return true;
}

void AVFRecorderWrapper::setMaxDuration(float seconds) {
  maxDurationSeconds = seconds;
  if (seconds > 0) {
    ofLogNotice("AVFRecorderWrapper") << "Max duration set to " << seconds << " seconds.";
  } else {
    ofLogNotice("AVFRecorderWrapper") << "Max duration disabled (unlimited).";
  }
}

void AVFRecorderWrapper::setOnRecordingComplete(std::function<void(bool success, const std::string& filePath)> callback) {
  onRecordingComplete = callback;
}

bool AVFRecorderWrapper::start() {
  if (!recorderInstance) {
    ofLogError("AVFRecorderWrapper") << "Cannot start, not set up.";
    return false;
  }
  if (isRecording()) {
    ofLogWarning("AVFRecorderWrapper") << "Already recording.";
    return false;
  }
  
  AVFRecorder *recorder = (__bridge AVFRecorder *)recorderInstance;
  NSError *error = nil;
  BOOL startSuccess = [recorder startRecording:&error];
  
  if (!startSuccess) {
    ofLogError("AVFRecorderWrapper") << "AVFRecorder startRecording failed: " << [[error localizedDescription] UTF8String];
    // Should we clean up here? Maybe not, allow user to retry?
  }
  else {
    ofLogNotice("AVFRecorderWrapper") << "Recording started.";
    frameCount = 0;
    audioSampleCount = 0;
    recordingStartTime = std::chrono::steady_clock::now();
  }

  return startSuccess;
}

void* AVFRecorderWrapper::createPixelBufferFromOfPixels(const ofPixels& pixels) {
  if (!cvPixelBufferPool) {
    ofLogError("AVFRecorderWrapper") << "Pixel buffer pool is not initialized.";
    return nullptr;
  }

  // Verify pixel format matches the expected source format
  if (pixels.getPixelFormat() != sourcePixelFormat) {
    ofLogError("AVFRecorderWrapper") << "Critical Pixel format mismatch! Expected: " << sourcePixelFormat
    << ", Got: " << pixels.getPixelFormat() << ". Cannot copy.";
    return nullptr;
  }

  // Pixels should already be at video dimensions (scaled in addFrame if needed)
  if (pixels.getWidth() != videoWidth || pixels.getHeight() != videoHeight) {
    ofLogError("AVFRecorderWrapper") << "Frame dimensions mismatch. Expected: " << videoWidth << "x" << videoHeight
    << ", Got: " << pixels.getWidth() << "x" << pixels.getHeight();
    return nullptr;
  }

  CVPixelBufferRef pixelBuffer = NULL;
  CVReturn status = CVPixelBufferPoolCreatePixelBuffer(kCFAllocatorDefault,
                                                       (CVPixelBufferPoolRef)cvPixelBufferPool,
                                                       &pixelBuffer);

  if (status != kCVReturnSuccess || !pixelBuffer) {
    ofLogError("AVFRecorderWrapper") << "Failed to create pixel buffer from pool: " << status;
    return nullptr;
  }

  CVPixelBufferLockBaseAddress(pixelBuffer, 0);
  uint8_t *bufferData = (uint8_t *)CVPixelBufferGetBaseAddress(pixelBuffer);
  size_t destRowBytes = CVPixelBufferGetBytesPerRow(pixelBuffer);
  size_t srcRowBytes = pixels.getWidth() * 4;

  if (bufferData) {
    // Convert RGBA to BGRA by swapping R and B channels
    const uint8_t *srcData = pixels.getData();
    for (int y = 0; y < videoHeight; y++) {
      const uint8_t *srcRow = srcData + y * srcRowBytes;
      uint8_t *destRow = bufferData + y * destRowBytes;
      for (int x = 0; x < videoWidth; x++) {
        int srcOffset = x * 4;
        int destOffset = x * 4;
        destRow[destOffset + 0] = srcRow[srcOffset + 2]; // B <- R
        destRow[destOffset + 1] = srcRow[srcOffset + 1]; // G <- G
        destRow[destOffset + 2] = srcRow[srcOffset + 0]; // R <- B
        destRow[destOffset + 3] = srcRow[srcOffset + 3]; // A <- A
      }
    }
  } else {
    ofLogError("AVFRecorderWrapper") << "Failed to get pixel buffer base address.";
  }

  CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);

  return pixelBuffer;
}


bool AVFRecorderWrapper::addFrame(const ofPixels& pixels) {
  if (!isRecording()) {
    return false;
  }
  if (!recorderInstance) {
    ofLogError("AVFRecorderWrapper") << "Cannot add frame, recorder not initialized.";
    return false;
  }

  // Check duration limit before adding frame
  if (maxDurationSeconds > 0 && getElapsedTime() >= maxDurationSeconds) {
    ofLogNotice("AVFRecorderWrapper") << "Max duration (" << maxDurationSeconds << "s) reached. Auto-stopping.";
    auto callback = onRecordingComplete;
    std::string savedFilePath = filePath;
    stop([callback, savedFilePath](bool success) {
      if (callback) {
        callback(success, savedFilePath);
      }
    });
    return false;
  }

  // Scale pixels if needed (source != output dimensions)
  const ofPixels* pixelsToEncode = &pixels;
  if (pixels.getWidth() != videoWidth || pixels.getHeight() != videoHeight) {
    // Resize using bilinear interpolation
    scaledPixels.allocate(videoWidth, videoHeight, pixels.getPixelFormat());
    pixels.resizeTo(scaledPixels, OF_INTERPOLATE_BILINEAR);
    pixelsToEncode = &scaledPixels;
  }

  CVPixelBufferRef pixelBuffer = (CVPixelBufferRef)createPixelBufferFromOfPixels(*pixelsToEncode);
  if (!pixelBuffer) {
    return false;
  }

  // Calculate timestamp using actual elapsed time (handles variable frame rates)
  auto now = std::chrono::steady_clock::now();
  auto elapsedMicros = std::chrono::duration_cast<std::chrono::microseconds>(now - recordingStartTime).count();
  // Use microsecond precision for smooth timing (timescale of 1000000)
  CMTime timestamp = CMTimeMake(elapsedMicros, 1000000);

  AVFRecorder *recorder = (__bridge AVFRecorder *)recorderInstance;
  BOOL success = [recorder addPixelBuffer:pixelBuffer withTimestamp:timestamp];

  // Release the pixel buffer created by createPixelBufferFromOfPixels
  CVPixelBufferRelease(pixelBuffer);

  if (success) {
    frameCount++;
  } else {
    ofLogError("AVFRecorderWrapper") << "Failed to add frame " << frameCount;
    // Recorder might have failed internally, check isRecording status?
  }

  return success;
}

bool AVFRecorderWrapper::addAudioSamples(const float* samples, int numFrames) {
  if (!isRecording() || !audioEnabled) {
    return false;
  }
  if (!recorderInstance || !samples || numFrames <= 0) {
    return false;
  }

  // Calculate timestamp using actual elapsed time (matches video timestamps)
  auto now = std::chrono::steady_clock::now();
  auto elapsedMicros = std::chrono::duration_cast<std::chrono::microseconds>(now - recordingStartTime).count();
  CMTime timestamp = CMTimeMake(elapsedMicros, 1000000);

  AVFRecorder *recorder = (__bridge AVFRecorder *)recorderInstance;
  BOOL success = [recorder addAudioSamples:samples numFrames:numFrames timestamp:timestamp];

  if (success) {
    audioSampleCount += numFrames;
  }

  return success;
}

bool AVFRecorderWrapper::addAudioSamples(const std::vector<float>& samples) {
  if (samples.empty() || audioChannels <= 0) return false;
  int numFrames = static_cast<int>(samples.size()) / audioChannels;
  return addAudioSamples(samples.data(), numFrames);
}

void AVFRecorderWrapper::stop(std::function<void(bool success)> completionCallback) {
  if (!recorderInstance) {
    ofLogWarning("AVFRecorderWrapper") << "Cannot stop, not initialized.";
    if (completionCallback) completionCallback(false);
    return;
  }
  
  if (!isRecording()) {
    ofLogWarning("AVFRecorderWrapper") << "Cannot stop, not recording.";
    if (completionCallback) completionCallback(true); // Arguably already stopped?
    return;
  }
  
  ofLogNotice("AVFRecorderWrapper") << "Stopping recording...";
  
  AVFRecorder *recorder = (__bridge AVFRecorder *)recorderInstance;
  
  // Use autoreleasepool for the block to manage temporary ObjC objects
  @autoreleasepool {
    [recorder finishRecordingWithCompletionHandler:^(NSError * _Nullable error) {
      // This handler is called on an arbitrary thread!
      bool success = (error == nil);
      if (!success) {
        // Log error from the original thread if possible, or be careful here.
        NSString *errMsg = [error localizedDescription];
        ofLogError("AVFRecorderWrapper") << "Recording failed to finalize: " << (errMsg ? [errMsg UTF8String] : "Unknown error");
      } else {
        ofLogNotice("AVFRecorderWrapper") << "Recording finished successfully.";
      }
      
      // Clean up resources after stopping
      // Note: This is inside the completion handler. Consider if cleanup
      // should happen immediately in stop() or after completion.
      // Doing it here ensures the file writing is truly done.
      // However, the C++ object might be destroyed before this runs!
      // A safer pattern might involve signaling completion and cleaning up
      // in the destructor or a dedicated cleanup method managed by the C++ side.
      
      // For now, let's assume the wrapper object outlives this block.
      releasePixelBufferPool(); // Better done in destructor or explicit cleanup
      AVFRecorder *rec = (__bridge_transfer AVFRecorder *)recorderInstance;
      recorderInstance = nullptr; // Better done in destructor

      if (completionCallback) {
        // Use ofScheduleMainLoopUpdate() or similar if callback needs
        // to touch UI or OF main thread resources.
        completionCallback(success);
      }
    }];
  }
}
bool AVFRecorderWrapper::isRecording() const {
  if (!recorderInstance) {
    return false;
  }
  AVFRecorder *recorder = (__bridge AVFRecorder *)recorderInstance;
  return recorder.isRecording;
}

float AVFRecorderWrapper::getElapsedTime() const {
  if (!isRecording()) {
    return static_cast<float>(frameCount) * frameDurationSeconds;
  }
  // Use actual elapsed time for accuracy
  auto now = std::chrono::steady_clock::now();
  auto elapsedMicros = std::chrono::duration_cast<std::chrono::microseconds>(now - recordingStartTime).count();
  return static_cast<float>(elapsedMicros) / 1000000.0f;
}

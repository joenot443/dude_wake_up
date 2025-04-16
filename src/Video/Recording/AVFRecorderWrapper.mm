#include "AVFRecorderWrapper.hpp"
#include "AVFRecorder.h"
#include "ofLog.h"
#include "ofUtils.h"
#include "ConfigService.hpp"

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
  
  videoWidth = width;
  videoHeight = height;
  frameDurationSeconds = 1.0f / fps;
  frameCount = 0;
  filePath = path;
  // Set the expected source format based on known input
  sourcePixelFormat = OF_PIXELS_RGB;
  OSType cvPixelFormat = ofFormatToCVPixelFormat((ofPixelFormat)sourcePixelFormat);
  
  NSURL *outputURL = [NSURL fileURLWithPath:[NSString stringWithUTF8String:filePath.c_str()]];
  CGSize videoSize = CGSizeMake(width, height);
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
  NSDictionary *videoSettings = @{
    AVVideoCodecKey: avCodec,
    AVVideoWidthKey: @(width),
    AVVideoHeightKey: @(height),
    // Add compression properties if needed
    /* AVVideoCompressionPropertiesKey: @{
     AVVideoAverageBitRateKey: @(bitrate), // Example
     AVVideoProfileLevelKey: AVVideoProfileLevelH264HighAutoLevel, // Example
     } */
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
    frameCount = 0; // Reset frame count on start
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
    // Cannot proceed if format is wrong, data will be garbage.
    return nullptr;
  }
  
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
  void *bufferData = CVPixelBufferGetBaseAddress(pixelBuffer);
  
  if (bufferData) {
    memcpy(bufferData, pixels.getData(), pixels.getTotalBytes());
  } else {
    ofLogError("AVFRecorderWrapper") << "Failed to get pixel buffer base address.";
  }
  
  CVPixelBufferUnlockBaseAddress(pixelBuffer, 0);
  
  return pixelBuffer; // Caller is responsible for releasing this buffer (e.g., via CVPixelBufferRelease)
}


bool AVFRecorderWrapper::addFrame(const ofPixels& pixels) {
  if (!isRecording()) {
    //ofLogWarning("AVFRecorderWrapper") << "Cannot add frame, not recording.";
    return false; // Don't log every frame if recording stopped
  }
  if (!recorderInstance) {
    ofLogError("AVFRecorderWrapper") << "Cannot add frame, recorder not initialized.";
    return false;
  }
  
  CVPixelBufferRef pixelBuffer = (CVPixelBufferRef)createPixelBufferFromOfPixels(pixels);
  if (!pixelBuffer) {
    return false; // Error logged in createPixelBufferFromOfPixels
  }
  
  // Calculate timestamp
  // Ensure using floats for division before casting
  CMTime timestamp = CMTimeMake(frameCount, static_cast<int32_t>(1.0f / frameDurationSeconds));
  // Alternative if fps is integer:
  // CMTime timestamp = CMTimeMake(frameCount * timeScaleMultiplier, timeScale);
  
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
      
      std::string command = "open " + ConfigService::getService()->exportsFolderFilePath();
      std::system(command.c_str());
      
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
  // Calculate elapsed time based on frames added and duration per frame
  // This assumes frameCount is accurately updated in addFrame()
  return static_cast<float>(frameCount) * frameDurationSeconds;
}

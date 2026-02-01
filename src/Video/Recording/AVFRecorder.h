#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>
#import <AudioToolbox/AudioToolbox.h>

NS_ASSUME_NONNULL_BEGIN

@interface AVFRecorder : NSObject

@property (nonatomic, readonly) BOOL isRecording;
@property (nonatomic, readonly) BOOL hasAudio;

// Designated initializer
- (instancetype)initWithURL:(NSURL *)outputURL
                       size:(CGSize)videoSize
                 timeScale:(int32_t)timeScale; // Timescale typically matches FPS

// Setup video input with specific settings
// Settings keys could include AVVideoCodecKey, AVVideoCompressionPropertiesKey, etc.
- (BOOL)setupVideoInputWithSettings:(NSDictionary<NSString *, id> *)videoSettings error:(NSError **)error;

// Setup audio input for recording
// Call before startRecording if you want audio
- (BOOL)setupAudioInputWithSampleRate:(Float64)sampleRate
                             channels:(int)channels
                                error:(NSError **)error;

// Start the recording session
// Must be called after setupVideoInput (and optionally setupAudioInput)
- (BOOL)startRecording:(NSError **)error;

// Append a pixel buffer for a frame
// Timestamp should be calculated based on frame count and timescale
- (BOOL)addPixelBuffer:(CVPixelBufferRef)pixelBuffer withTimestamp:(CMTime)timestamp;

// Append audio samples (interleaved float32)
// numFrames is the number of sample frames (samples per channel)
- (BOOL)addAudioSamples:(const float *)samples
              numFrames:(int)numFrames
              timestamp:(CMTime)timestamp;

// Finish recording and write the file
- (void)finishRecordingWithCompletionHandler:(void (^)(NSError * _Nullable error))handler;

@end

NS_ASSUME_NONNULL_END

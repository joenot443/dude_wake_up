#import <Foundation/Foundation.h>
#import <AVFoundation/AVFoundation.h>
#import <CoreMedia/CoreMedia.h>
#import <CoreVideo/CoreVideo.h>

NS_ASSUME_NONNULL_BEGIN

@interface AVFRecorder : NSObject

@property (nonatomic, readonly) BOOL isRecording;

// Designated initializer
- (instancetype)initWithURL:(NSURL *)outputURL
                       size:(CGSize)videoSize
                 timeScale:(int32_t)timeScale; // Timescale typically matches FPS

// Setup video input with specific settings
// Settings keys could include AVVideoCodecKey, AVVideoCompressionPropertiesKey, etc.
- (BOOL)setupVideoInputWithSettings:(NSDictionary<NSString *, id> *)videoSettings error:(NSError **)error;

// Start the recording session
// Must be called after setupVideoInput
- (BOOL)startRecording:(NSError **)error;

// Append a pixel buffer for a frame
// Timestamp should be calculated based on frame count and timescale
- (BOOL)addPixelBuffer:(CVPixelBufferRef)pixelBuffer withTimestamp:(CMTime)timestamp;

// Finish recording and write the file
- (void)finishRecordingWithCompletionHandler:(void (^)(NSError * _Nullable error))handler;

@end

NS_ASSUME_NONNULL_END

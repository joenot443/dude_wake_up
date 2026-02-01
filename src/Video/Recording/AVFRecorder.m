#import "AVFRecorder.h"
#import <AppKit/AppKit.h> // For NSAlert if debugging needed

// Define a context for KVO - not strictly needed here but good practice
static void *AVFRecordingContext = &AVFRecordingContext;

@interface AVFRecorder ()

@property (nonatomic, strong) AVAssetWriter *assetWriter;
@property (nonatomic, strong) AVAssetWriterInput *videoInput;
@property (nonatomic, strong) AVAssetWriterInputPixelBufferAdaptor *pixelBufferAdaptor;
@property (nonatomic, strong) AVAssetWriterInput *audioInput;
@property (nonatomic, strong) dispatch_queue_t writingQueue;
@property (nonatomic, strong) NSURL *outputURL;
@property (nonatomic, assign) CGSize videoSize;
@property (nonatomic, assign) int32_t timeScale; // FPS
@property (nonatomic, assign) BOOL isRecording; // Internal mutable property
@property (nonatomic, assign) BOOL hasAudio;

// Audio format info
@property (nonatomic, assign) Float64 audioSampleRate;
@property (nonatomic, assign) int audioChannels;
@property (nonatomic, assign) CMAudioFormatDescriptionRef audioFormatDescription;

@end

@implementation AVFRecorder

- (instancetype)initWithURL:(NSURL *)outputURL
                       size:(CGSize)videoSize
                  timeScale:(int32_t)timeScale {
    self = [super init];
    if (self) {
        _outputURL = outputURL;
        _videoSize = videoSize;
        _timeScale = timeScale;
        _isRecording = NO;
        _hasAudio = NO;
        _audioFormatDescription = NULL;

        // Serial queue for all writing operations
        // Using DISPATCH_QUEUE_SERIAL for safety with AVAssetWriter
        _writingQueue = dispatch_queue_create("com.yourapp.avfrecorder.writingqueue", DISPATCH_QUEUE_SERIAL);
        
        // Ensure the directory exists
        NSError *dirError = nil;
        [[NSFileManager defaultManager] createDirectoryAtURL:[outputURL URLByDeletingLastPathComponent]
                                 withIntermediateDirectories:YES
                                                  attributes:nil
                                                       error:&dirError];
        if (dirError) {
          NSLog(@"[AVFRecorder] Error creating directory: %@", dirError);
          // Handle error appropriately, maybe return nil
        }
        
        // Delete existing file if overwrite desired (optional)
         if ([[NSFileManager defaultManager] fileExistsAtPath:[outputURL path]]) {
             NSError *removeError = nil;
             [[NSFileManager defaultManager] removeItemAtURL:outputURL error:&removeError];
             if (removeError) {
                 NSLog(@"[AVFRecorder] Error removing existing file: %@", removeError);
                 // Handle error
             }
         }
    }
    return self;
}

- (BOOL)setupVideoInputWithSettings:(NSDictionary<NSString *, id> *)videoSettings error:(NSError **)error {
    NSError *writerError = nil;
    self.assetWriter = [[AVAssetWriter alloc] initWithURL:self.outputURL
                                                 fileType:AVFileTypeMPEG4 // .mp4
                                                    error:&writerError];
    if (writerError || !self.assetWriter) {
        NSLog(@"[AVFRecorder] Error creating AVAssetWriter: %@", writerError);
        if (error) *error = writerError;
        return NO;
    }

    self.videoInput = [AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeVideo
                                                         outputSettings:videoSettings];
    self.videoInput.expectsMediaDataInRealTime = YES; // Important for live capture

    // Source Pixel Buffer Attributes: Match the format coming from ofPixels (RGB)
    NSDictionary *sourcePixelBufferAttributes = @{
        (NSString *)kCVPixelBufferPixelFormatTypeKey: @(kCVPixelFormatType_24RGB),
        (NSString *)kCVPixelBufferWidthKey: @(self.videoSize.width),
        (NSString *)kCVPixelBufferHeightKey: @(self.videoSize.height)
    };

    self.pixelBufferAdaptor = [AVAssetWriterInputPixelBufferAdaptor
                               assetWriterInputPixelBufferAdaptorWithAssetWriterInput:self.videoInput
                               sourcePixelBufferAttributes:sourcePixelBufferAttributes];

    if ([self.assetWriter canAddInput:self.videoInput]) {
        [self.assetWriter addInput:self.videoInput];
    } else {
        NSLog(@"[AVFRecorder] Cannot add video input to writer.");
        // Populate error object if needed
        if (error) *error = [NSError errorWithDomain:@"AVFRecorderErrorDomain" code:-1 userInfo:@{NSLocalizedDescriptionKey: @"Cannot add video input."}];
        return NO;
    }

    return YES;
}

- (BOOL)setupAudioInputWithSampleRate:(Float64)sampleRate
                             channels:(int)channels
                                error:(NSError **)error {
    if (!self.assetWriter) {
        NSLog(@"[AVFRecorder] Must call setupVideoInput before setupAudioInput");
        if (error) *error = [NSError errorWithDomain:@"AVFRecorderErrorDomain" code:-10
                                            userInfo:@{NSLocalizedDescriptionKey: @"Video must be set up first."}];
        return NO;
    }

    self.audioSampleRate = sampleRate;
    self.audioChannels = channels;

    // Audio output settings for AAC encoding
    AudioChannelLayout channelLayout = {0};
    channelLayout.mChannelLayoutTag = (channels == 1) ? kAudioChannelLayoutTag_Mono : kAudioChannelLayoutTag_Stereo;
    NSData *channelLayoutData = [NSData dataWithBytes:&channelLayout length:sizeof(channelLayout)];

    NSDictionary *audioSettings = @{
        AVFormatIDKey: @(kAudioFormatMPEG4AAC),
        AVSampleRateKey: @(sampleRate),
        AVNumberOfChannelsKey: @(channels),
        AVEncoderBitRateKey: @(128000), // 128 kbps
        AVChannelLayoutKey: channelLayoutData
    };

    self.audioInput = [AVAssetWriterInput assetWriterInputWithMediaType:AVMediaTypeAudio
                                                         outputSettings:audioSettings];
    self.audioInput.expectsMediaDataInRealTime = YES;

    if ([self.assetWriter canAddInput:self.audioInput]) {
        [self.assetWriter addInput:self.audioInput];
    } else {
        NSLog(@"[AVFRecorder] Cannot add audio input to writer.");
        if (error) *error = [NSError errorWithDomain:@"AVFRecorderErrorDomain" code:-11
                                            userInfo:@{NSLocalizedDescriptionKey: @"Cannot add audio input."}];
        return NO;
    }

    // Create audio format description for source (Float32 interleaved)
    AudioStreamBasicDescription asbd = {0};
    asbd.mSampleRate = sampleRate;
    asbd.mFormatID = kAudioFormatLinearPCM;
    asbd.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
    asbd.mBytesPerPacket = sizeof(float) * channels;
    asbd.mFramesPerPacket = 1;
    asbd.mBytesPerFrame = sizeof(float) * channels;
    asbd.mChannelsPerFrame = channels;
    asbd.mBitsPerChannel = 32;

    OSStatus status = CMAudioFormatDescriptionCreate(kCFAllocatorDefault,
                                                      &asbd,
                                                      sizeof(channelLayout),
                                                      &channelLayout,
                                                      0, NULL, NULL,
                                                      &_audioFormatDescription);
    if (status != noErr) {
        NSLog(@"[AVFRecorder] Failed to create audio format description: %d", (int)status);
        if (error) *error = [NSError errorWithDomain:@"AVFRecorderErrorDomain" code:status
                                            userInfo:@{NSLocalizedDescriptionKey: @"Failed to create audio format."}];
        return NO;
    }

    self.hasAudio = YES;
    NSLog(@"[AVFRecorder] Audio input set up: %.0f Hz, %d channels", sampleRate, channels);
    return YES;
}

- (BOOL)startRecording:(NSError **)error {
    if (self.isRecording) {
        NSLog(@"[AVFRecorder] Already recording.");
        // Maybe set an error?
        return NO;
    }
    
    __block BOOL success = NO;
    __block NSError *blockError = nil;

    // Use dispatch_sync to ensure setup is complete before returning
    dispatch_sync(self.writingQueue, ^{
        if (self.assetWriter.status == AVAssetWriterStatusUnknown) {
            success = [self.assetWriter startWriting];
            if (success) {
                [self.assetWriter startSessionAtSourceTime:kCMTimeZero];
                self.isRecording = YES;
                NSLog(@"[AVFRecorder] Started recording session.");
            } else {
                blockError = self.assetWriter.error;
                NSLog(@"[AVFRecorder] Error starting asset writer: %@", blockError);
            }
        } else {
             NSLog(@"[AVFRecorder] Writer status is not Unknown: %ld", (long)self.assetWriter.status);
             blockError = self.assetWriter.error ?: [NSError errorWithDomain:@"AVFRecorderErrorDomain" code:-2 userInfo:@{NSLocalizedDescriptionKey: @"Writer not in valid state to start."}];
        }
    });

    if (error && blockError) {
        *error = blockError;
    }

    return success;
}

- (BOOL)addPixelBuffer:(CVPixelBufferRef)pixelBuffer withTimestamp:(CMTime)timestamp {
    if (!self.isRecording || !pixelBuffer) {
        if (!self.isRecording) NSLog(@"[AVFRecorder] WARN: addPixelBuffer called while not recording.");
         if (!pixelBuffer) NSLog(@"[AVFRecorder] WARN: addPixelBuffer called with nil buffer.");
        return NO;
    }
    
    // Check if timestamp is valid
    if (!CMTIME_IS_VALID(timestamp)) {
         NSLog(@"[AVFRecorder] WARN: addPixelBuffer called with invalid timestamp.");
         return NO;
    }

    // Retain the buffer before passing to async block, release after.
    // The adaptor appends synchronously on the queue, so maybe not needed if called *from* the queue.
    // However, our C++ wrapper might call this from another thread, so dispatching is essential.
    // CVPixelBufferRetain(pixelBuffer); // Let's assume the C++ wrapper manages the retain/release

    __block BOOL appendSuccess = NO;
    dispatch_sync(self.writingQueue, ^{ // Using sync to know result immediately, could be async if perf needed
        if (self.videoInput.readyForMoreMediaData && self.isRecording) {
            appendSuccess = [self.pixelBufferAdaptor appendPixelBuffer:pixelBuffer
                                                  withPresentationTime:timestamp];
            if (!appendSuccess) {
                NSLog(@"[AVFRecorder] Error appending pixel buffer: %@", self.assetWriter.error);
                if (self.assetWriter.status == AVAssetWriterStatusFailed) {
                    // Recording has failed, stop it
                    [self finishRecordingWithCompletionHandler:nil]; // No handler needed here
                }
            }
        } else {
            NSLog(@"[AVFRecorder] WARN: Video input not ready or not recording when adding buffer.");
            // appendSuccess remains NO
        }
        // CVPixelBufferRelease(pixelBuffer);
    });

    return appendSuccess;
}

- (BOOL)addAudioSamples:(const float *)samples
              numFrames:(int)numFrames
              timestamp:(CMTime)timestamp {
    if (!self.isRecording || !self.hasAudio || !samples || numFrames <= 0) {
        return NO;
    }

    if (!CMTIME_IS_VALID(timestamp)) {
        NSLog(@"[AVFRecorder] WARN: addAudioSamples called with invalid timestamp.");
        return NO;
    }

    __block BOOL appendSuccess = NO;

    // Copy samples to ensure data remains valid during async processing
    size_t dataSize = numFrames * self.audioChannels * sizeof(float);
    float *samplesCopy = (float *)malloc(dataSize);
    if (!samplesCopy) {
        NSLog(@"[AVFRecorder] Failed to allocate audio buffer copy.");
        return NO;
    }
    memcpy(samplesCopy, samples, dataSize);

    dispatch_sync(self.writingQueue, ^{
        if (!self.audioInput.readyForMoreMediaData || !self.isRecording) {
            NSLog(@"[AVFRecorder] WARN: Audio input not ready or not recording.");
            free(samplesCopy);
            return;
        }

        // Create CMBlockBuffer from audio data
        CMBlockBufferRef blockBuffer = NULL;
        OSStatus status = CMBlockBufferCreateWithMemoryBlock(kCFAllocatorDefault,
                                                              samplesCopy,
                                                              dataSize,
                                                              kCFAllocatorMalloc, // Will free the memory
                                                              NULL,
                                                              0,
                                                              dataSize,
                                                              0,
                                                              &blockBuffer);
        if (status != kCMBlockBufferNoErr || !blockBuffer) {
            NSLog(@"[AVFRecorder] Failed to create block buffer: %d", (int)status);
            free(samplesCopy);
            return;
        }

        // Create CMSampleBuffer
        CMSampleBufferRef sampleBuffer = NULL;
        status = CMAudioSampleBufferCreateReadyWithPacketDescriptions(kCFAllocatorDefault,
                                                                       blockBuffer,
                                                                       self.audioFormatDescription,
                                                                       numFrames,
                                                                       timestamp,
                                                                       NULL, // No packet descriptions for PCM
                                                                       &sampleBuffer);
        CFRelease(blockBuffer);

        if (status != noErr || !sampleBuffer) {
            NSLog(@"[AVFRecorder] Failed to create audio sample buffer: %d", (int)status);
            return;
        }

        // Append to writer
        appendSuccess = [self.audioInput appendSampleBuffer:sampleBuffer];
        CFRelease(sampleBuffer);

        if (!appendSuccess) {
            NSLog(@"[AVFRecorder] Error appending audio samples: %@", self.assetWriter.error);
        }
    });

    return appendSuccess;
}

- (void)finishRecordingWithCompletionHandler:(void (^)(NSError * _Nullable error))handler {
    if (!self.isRecording && self.assetWriter.status != AVAssetWriterStatusWriting) {
         NSLog(@"[AVFRecorder] Not recording, cannot finish.");
         // Call handler immediately with an error or success=false if appropriate
         if (handler) {
             NSError *err = [NSError errorWithDomain:@"AVFRecorderErrorDomain" code:-3 userInfo:@{NSLocalizedDescriptionKey: @"Not recording."}];
             handler(err);
         }
         return;
    }
    
    dispatch_async(self.writingQueue, ^{
        self.isRecording = NO; // Set immediately on the queue

        if (self.assetWriter.status == AVAssetWriterStatusWriting) {
            [self.videoInput markAsFinished];
            if (self.audioInput) {
                [self.audioInput markAsFinished];
            }
            
            // Weak self for block safety
            __weak typeof(self) weakSelf = self;
            [self.assetWriter finishWritingWithCompletionHandler:^{
                // Called on an arbitrary queue, potentially not the writingQueue
                __strong typeof(weakSelf) strongSelf = weakSelf;
                if (!strongSelf) return;
                
                NSError *finishError = strongSelf.assetWriter.error;
                if (strongSelf.assetWriter.status == AVAssetWriterStatusCompleted) {
                    NSLog(@"[AVFRecorder] Finished recording successfully to %@", strongSelf.outputURL.path);
                    if (handler) handler(nil); // Success
                } else {
                    NSLog(@"[AVFRecorder] Failed to finish recording: %@", finishError);
                    if (handler) handler(finishError); // Failure
                }
                
                // Clean up writer
                 strongSelf.assetWriter = nil;
                 strongSelf.videoInput = nil;
                 strongSelf.pixelBufferAdaptor = nil;
                 strongSelf.audioInput = nil;
                 if (strongSelf.audioFormatDescription) {
                     CFRelease(strongSelf.audioFormatDescription);
                     strongSelf->_audioFormatDescription = NULL;
                 }
            }];
        } else {
             NSLog(@"[AVFRecorder] Writer was not in writing state (%ld), cannot finish properly.", (long)self.assetWriter.status);
             NSError *finishError = self.assetWriter.error ?: [NSError errorWithDomain:@"AVFRecorderErrorDomain" code:-4 userInfo:@{NSLocalizedDescriptionKey: @"Writer not in writing state."}];
             // Clean up resources
             self.assetWriter = nil;
             self.videoInput = nil;
             self.pixelBufferAdaptor = nil;
             self.audioInput = nil;
             if (self.audioFormatDescription) {
                 CFRelease(self.audioFormatDescription);
                 self->_audioFormatDescription = NULL;
             }
             if (handler) handler(finishError);
        }
    });
}

- (void)dealloc {
    // Check if still recording? Should be stopped via C++ wrapper destructor ideally.
    if (self.isRecording) {
        NSLog(@"[AVFRecorder] WARN: Dealloc called while still recording. Attempting to stop.");
         [self finishRecordingWithCompletionHandler:nil];
    }

    // Clean up audio format description if not already done
    if (_audioFormatDescription) {
        CFRelease(_audioFormatDescription);
        _audioFormatDescription = NULL;
    }

    NSLog(@"[AVFRecorder] Deallocated.");
}


@end 
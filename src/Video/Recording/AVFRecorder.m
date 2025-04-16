#import "AVFRecorder.h"
#import <AppKit/AppKit.h> // For NSAlert if debugging needed

// Define a context for KVO - not strictly needed here but good practice
static void *AVFRecordingContext = &AVFRecordingContext;

@interface AVFRecorder ()

@property (nonatomic, strong) AVAssetWriter *assetWriter;
@property (nonatomic, strong) AVAssetWriterInput *videoInput;
@property (nonatomic, strong) AVAssetWriterInputPixelBufferAdaptor *pixelBufferAdaptor;
@property (nonatomic, strong) dispatch_queue_t writingQueue;
@property (nonatomic, strong) NSURL *outputURL;
@property (nonatomic, assign) CGSize videoSize;
@property (nonatomic, assign) int32_t timeScale; // FPS
@property (nonatomic, assign) BOOL isRecording; // Internal mutable property

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
                                                 fileType:AVFileTypeQuickTimeMovie // .mov
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
            [self.videoInput markAsFinished]; // Mark input as finished first
            
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
                
                // Clean up writer? Maybe not here, let ARC handle it or manage in C++ wrapper destructor
                 strongSelf.assetWriter = nil;
                 strongSelf.videoInput = nil;
                 strongSelf.pixelBufferAdaptor = nil;
            }];
        } else {
             NSLog(@"[AVFRecorder] Writer was not in writing state (%ld), cannot finish properly.", (long)self.assetWriter.status);
             NSError *finishError = self.assetWriter.error ?: [NSError errorWithDomain:@"AVFRecorderErrorDomain" code:-4 userInfo:@{NSLocalizedDescriptionKey: @"Writer not in writing state."}];
             // Clean up resources even if not writing?
             self.assetWriter = nil;
             self.videoInput = nil;
             self.pixelBufferAdaptor = nil;
             if (handler) handler(finishError);
        }
    });
}

- (void)dealloc {
    // Check if still recording? Should be stopped via C++ wrapper destructor ideally.
    if (self.isRecording) {
        NSLog(@"[AVFRecorder] WARN: Dealloc called while still recording. Attempting to stop.");
        // Synchronous stop might be problematic in dealloc
        // This indicates a potential lifecycle issue in the calling code.
        // Consider asserting or logging heavily.
        // For safety, maybe just try dispatching async, but file might not finish.
         [self finishRecordingWithCompletionHandler:nil];
    }
    
    // Release queue if not using ARC for dispatch queues (modern SDKs use ARC)
    // if (_writingQueue) { dispatch_release(_writingQueue); _writingQueue = nil; } // Pre-ARC
    
    NSLog(@"[AVFRecorder] Deallocated.");
}


@end 
#pragma once

// C++ interface that can be used from anywhere in your C++ code
class SparkleUpdaterService {
public:
    static SparkleUpdaterService& getInstance();

    void initialize();
    void checkForUpdates();
    void checkForUpdatesInBackground();
    bool canCheckForUpdates();

private:
    SparkleUpdaterService() = default;
    ~SparkleUpdaterService() = default;
    SparkleUpdaterService(const SparkleUpdaterService&) = delete;
    SparkleUpdaterService& operator=(const SparkleUpdaterService&) = delete;

    void* updaterImpl = nullptr; // Will hold the Objective-C object
};

// Objective-C interface (only visible in .mm files)
#ifdef __OBJC__
#import <Cocoa/Cocoa.h>
#import <Sparkle/Sparkle.h>

@interface SparkleUpdater : NSObject <SPUUpdaterDelegate, SPUStandardUserDriverDelegate>

@property (strong) SPUStandardUpdaterController *updaterController;

+ (SparkleUpdater *)sharedUpdater;
- (void)startUpdater;
- (void)checkForUpdates;
- (void)checkForUpdatesInBackground;
- (BOOL)canCheckForUpdates;

@end
#endif
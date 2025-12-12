#import "SparkleUpdater.h"

// Objective-C Implementation
@implementation SparkleUpdater

+ (SparkleUpdater *)sharedUpdater {
    static SparkleUpdater *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[SparkleUpdater alloc] init];
    });
    return sharedInstance;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        // Initialize the updater controller with delegates
        // When initWithStartingUpdater is YES, the updater starts automatically
        _updaterController = [[SPUStandardUpdaterController alloc]
            initWithStartingUpdater:YES
            updaterDelegate:self
            userDriverDelegate:self];

        if (!_updaterController) {
            NSLog(@"Failed to initialize Sparkle updater controller");
        } else {
            NSLog(@"Sparkle updater initialized and started successfully");
        }
    }
    return self;
}

- (void)startUpdater {
    // The updater is already started in init when initWithStartingUpdater:YES is used
    // This method is kept for API compatibility but doesn't need to do anything
    if (self.updaterController) {
        NSLog(@"Sparkle updater is already running");
    }
}

- (void)checkForUpdates {
    [self.updaterController checkForUpdates:nil];
}

- (void)checkForUpdatesInBackground {
    // In Sparkle 2.x, background checks are handled automatically based on SUEnableAutomaticChecks
    // We can trigger a manual background check if needed
    if (self.updaterController.updater) {
        [self.updaterController.updater checkForUpdatesInBackground];
    }
}

- (BOOL)canCheckForUpdates {
    if (self.updaterController.updater) {
        return self.updaterController.updater.canCheckForUpdates;
    }
    return NO;
}

#pragma mark - SPUUpdaterDelegate

- (NSSet<NSString *> *)allowedChannelsForUpdater:(SPUUpdater *)updater {
    // Return nil to allow all channels, or specify channels like @[@"beta", @"stable"]
    return nil;
}

- (void)updater:(SPUUpdater *)updater didFinishLoadingAppcast:(SUAppcast *)appcast {
    NSLog(@"Sparkle: Finished loading appcast");
}

- (void)updaterDidNotFindUpdate:(SPUUpdater *)updater error:(NSError *)error {
    if (error) {
        NSLog(@"Sparkle: Error checking for updates: %@", error.localizedDescription);
    } else {
        NSLog(@"Sparkle: No updates available");
    }
}

- (void)updater:(SPUUpdater *)updater didFindValidUpdate:(SUAppcastItem *)item {
    NSLog(@"Sparkle: Found update version %@", item.displayVersionString);
}

- (void)updater:(SPUUpdater *)updater didAbortWithError:(NSError *)error {
    NSLog(@"Sparkle: Update aborted with error: %@", error.localizedDescription);
}

#pragma mark - SPUStandardUserDriverDelegate

- (BOOL)supportsGentleScheduledUpdateReminders {
    return YES;
}

- (void)standardUserDriverShouldHandleShowingScheduledUpdate:(SUAppcastItem *)update
                                                   andInImmediateFocus:(BOOL)immediateFocus {
    // This allows Sparkle to handle scheduled update notifications
    NSLog(@"Sparkle: Scheduled update available: %@", update.displayVersionString);
}

@end

// C++ Wrapper Implementation
SparkleUpdaterService& SparkleUpdaterService::getInstance() {
    static SparkleUpdaterService instance;
    return instance;
}

void SparkleUpdaterService::initialize() {
    @autoreleasepool {
        SparkleUpdater *updater = [SparkleUpdater sharedUpdater];
        updaterImpl = (__bridge_retained void*)updater;
        [updater startUpdater];
    }
}

void SparkleUpdaterService::checkForUpdates() {
    @autoreleasepool {
        if (updaterImpl) {
            SparkleUpdater *updater = (__bridge SparkleUpdater*)updaterImpl;
            [updater checkForUpdates];
        } else {
        }
    }
}

void SparkleUpdaterService::checkForUpdatesInBackground() {
    @autoreleasepool {
        if (updaterImpl) {
            SparkleUpdater *updater = (__bridge SparkleUpdater*)updaterImpl;
            [updater checkForUpdatesInBackground];
        } else {
        }
    }
}

bool SparkleUpdaterService::canCheckForUpdates() {
    @autoreleasepool {
        if (updaterImpl) {
            SparkleUpdater *updater = (__bridge SparkleUpdater*)updaterImpl;
            return [updater canCheckForUpdates];
        }
        return false;
    }
}

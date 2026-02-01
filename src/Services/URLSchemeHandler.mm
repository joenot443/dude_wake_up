// Include OpenFrameworks first to set up GLEW properly
#import "ofMain.h"

// Now it's safe to include Cocoa
#import <Cocoa/Cocoa.h>

#import "URLSchemeHandler.h"
#import "LibraryService.hpp"
#import "ConfigService.hpp"
#import "LayoutStateService.hpp"
#import "MainApp.h"

// Forward declare the Objective-C class
@interface URLSchemeHandlerObjC : NSObject

+ (URLSchemeHandlerObjC *)sharedHandler;
- (void)registerURLHandler;
- (void)handleGetURLEvent:(NSAppleEventDescriptor *)event withReplyEvent:(NSAppleEventDescriptor *)replyEvent;

@property (nonatomic, copy) NSString *pendingURL;

@end

// Objective-C Implementation
@implementation URLSchemeHandlerObjC

+ (URLSchemeHandlerObjC *)sharedHandler {
    static URLSchemeHandlerObjC *sharedInstance = nil;
    static dispatch_once_t onceToken;
    dispatch_once(&onceToken, ^{
        sharedInstance = [[URLSchemeHandlerObjC alloc] init];
    });
    return sharedInstance;
}

- (instancetype)init {
    self = [super init];
    if (self) {
        _pendingURL = nil;
    }
    return self;
}

- (void)registerURLHandler {
    NSAppleEventManager *appleEventManager = [NSAppleEventManager sharedAppleEventManager];
    [appleEventManager setEventHandler:self
                           andSelector:@selector(handleGetURLEvent:withReplyEvent:)
                         forEventClass:kInternetEventClass
                            andEventID:kAEGetURL];

    NSLog(@"URLSchemeHandler: Registered for nottawa:// URL scheme");
}

- (void)handleGetURLEvent:(NSAppleEventDescriptor *)event withReplyEvent:(NSAppleEventDescriptor *)replyEvent {
    NSString *urlString = [[event paramDescriptorForKeyword:keyDirectObject] stringValue];

    if (urlString) {
        NSLog(@"URLSchemeHandler: Received URL: %@", urlString);

        // Store as pending URL if app isn't fully initialized yet
        self.pendingURL = urlString;

        // Try to process immediately if app is ready
        URLSchemeHandler::getInstance().handleURL([urlString UTF8String]);
    }
}

@end

// C++ Wrapper Implementation
URLSchemeHandler& URLSchemeHandler::getInstance() {
    static URLSchemeHandler instance;
    return instance;
}

void URLSchemeHandler::initialize() {
    @autoreleasepool {
        URLSchemeHandlerObjC *handler = [URLSchemeHandlerObjC sharedHandler];
        handlerImpl = (__bridge_retained void*)handler;
        [handler registerURLHandler];
        initialized = true;

        NSLog(@"URLSchemeHandler: Initialized");
    }
}

void URLSchemeHandler::processPendingURL() {
    @autoreleasepool {
        if (handlerImpl) {
            URLSchemeHandlerObjC *handler = (__bridge URLSchemeHandlerObjC*)handlerImpl;
            if (handler.pendingURL) {
                NSString *url = handler.pendingURL;
                handler.pendingURL = nil;
                handleURL([url UTF8String]);
            }
        }
    }
}

void URLSchemeHandler::handleURL(const std::string& url) {
    // Parse the URL: nottawa://strands/{slug}
    // Expected format: nottawa://strands/abc123

    log("URLSchemeHandler: Processing URL: %s", url.c_str());

    // Check if URL starts with nottawa://
    const std::string prefix = "nottawa://";
    if (url.substr(0, prefix.length()) != prefix) {
        log("URLSchemeHandler: Invalid URL scheme, expected nottawa://");
        return;
    }

    std::string path = url.substr(prefix.length());

    // Check if it's a strand URL: strands/{slug}
    const std::string strandsPrefix = "strands/";
    if (path.substr(0, strandsPrefix.length()) != strandsPrefix) {
        log("URLSchemeHandler: Unknown URL path: %s", path.c_str());
        return;
    }

    std::string slug = path.substr(strandsPrefix.length());

    // Remove any trailing slashes or query parameters
    size_t queryPos = slug.find('?');
    if (queryPos != std::string::npos) {
        slug = slug.substr(0, queryPos);
    }
    size_t slashPos = slug.find('/');
    if (slashPos != std::string::npos) {
        slug = slug.substr(0, slashPos);
    }

    if (slug.empty()) {
        log("URLSchemeHandler: Empty slug in URL");
        return;
    }

    log("URLSchemeHandler: Fetching strand with slug: %s", slug.c_str());

    // Fetch the strand from the server
    LibraryService::getService()->fetchSharedStrand(slug,
        [this, slug](bool success, const std::string& strandJson, const std::string& title, const std::string& error) {
            if (success) {
                log("URLSchemeHandler: Successfully fetched strand '%s'", title.c_str());

                // Load the strand into the app
                std::vector<std::string> ids = ConfigService::getService()->loadStrandJson(strandJson);

                if (!ids.empty()) {
                    log("URLSchemeHandler: Loaded strand with %zu nodes", ids.size());

                    // Hide the welcome screen
                    LayoutStateService::getService()->showWelcomeScreen = false;
                }

                // Call the callback if set
                if (strandLoadedCallback) {
                    strandLoadedCallback(slug, true, "");
                }
            } else {
                log("URLSchemeHandler: Failed to fetch strand: %s", error.c_str());

                if (strandLoadedCallback) {
                    strandLoadedCallback(slug, false, error);
                }
            }
        });
}

void URLSchemeHandler::setStrandLoadedCallback(StrandLoadedCallback callback) {
    strandLoadedCallback = callback;
}

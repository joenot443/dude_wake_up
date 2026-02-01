#pragma once

//  URLSchemeHandler.h
//  dude_wake_up
//
//  Handles nottawa:// URL scheme for opening shared strands.
//  When the app is opened via a URL like nottawa://strands/abc123,
//  this handler fetches the strand data and loads it into the app.

#include <string>
#include <functional>

// C++ interface that can be used from anywhere in your C++ code
class URLSchemeHandler {
public:
    static URLSchemeHandler& getInstance();

    // Initialize the URL scheme handler - must be called during app startup
    void initialize();

    // Check if there's a pending URL to process (for cold launch)
    void processPendingURL();

    // Manually handle a URL (used by the Apple Event handler)
    void handleURL(const std::string& url);

    // Set callback for when a strand is successfully loaded
    using StrandLoadedCallback = std::function<void(const std::string& slug, bool success, const std::string& error)>;
    void setStrandLoadedCallback(StrandLoadedCallback callback);

private:
    URLSchemeHandler() = default;
    ~URLSchemeHandler() = default;
    URLSchemeHandler(const URLSchemeHandler&) = delete;
    URLSchemeHandler& operator=(const URLSchemeHandler&) = delete;

    void* handlerImpl = nullptr; // Will hold the Objective-C object
    StrandLoadedCallback strandLoadedCallback;
    std::string pendingURL;
    bool initialized = false;
};

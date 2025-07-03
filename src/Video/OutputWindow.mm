//
//  OutputWindow.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/30/22.
//

#include "OutputWindow.hpp"
#include "VideoSourceService.hpp"
#include "LayoutStateService.hpp"
#include "imgui.h"

#ifdef __OBJC__
#import <Cocoa/Cocoa.h>

@interface OutputWindowDelegate : NSObject
{
  OutputWindow* window;
}
- (id)initWithWindow:(OutputWindow*)w;
- (void)startRecording:(id)sender;
- (void)stopRecording:(id)sender;
@end

@implementation OutputWindowDelegate
- (id)initWithWindow:(OutputWindow*)w {
  self = [super init];
  if (self) {
    window = w;
  }
  return self;
}

- (void)startRecording:(id)sender {
  window->startRecording();
}

- (void)stopRecording:(id)sender {
  window->stopRecording();
}
@end
#endif

void OutputWindow::setup() {
//  LayoutStateService::getService()->subscribeToResolutionUpdates([this]() {
//    needsResolutionUpdate = true;
//  });

  // Initialize video recorder - AVFRecorderWrapper setup is called when starting
  // No explicit setup needed here unless defaults change
  setupNativeMenu();
}

void OutputWindow::setupNativeMenu() {
#ifdef __OBJC__
  // Guard against multiple menu creation
  if (recordMenu != nil) {
    return;
  }

  // Get the main menu
  NSMenu* mainMenu = [NSApp mainMenu];
  
  // Create Record menu
  recordMenu = [[NSMenu alloc] initWithTitle:@"Record"];
  NSMenuItem* recordMenuItem = [[NSMenuItem alloc] initWithTitle:@"Record" action:nil keyEquivalent:@""];
  [recordMenuItem setSubmenu:recordMenu];
  [mainMenu addItem:recordMenuItem];
  
  // Create delegate for menu actions
  menuDelegate = [[OutputWindowDelegate alloc] initWithWindow:this];
  
  // Add Start Recording item
  startRecordingItem = [[NSMenuItem alloc] initWithTitle:@"Start Recording" 
                                                action:@selector(startRecording:) 
                                         keyEquivalent:@"r"];
  [startRecordingItem setKeyEquivalentModifierMask:NSEventModifierFlagCommand];
  [startRecordingItem setTarget:menuDelegate];
  [recordMenu addItem:startRecordingItem];
  
  // Add Stop Recording item
  stopRecordingItem = [[NSMenuItem alloc] initWithTitle:@"Stop Recording" 
                                               action:@selector(stopRecording:) 
                                        keyEquivalent:@"r"];
  [stopRecordingItem setKeyEquivalentModifierMask:NSEventModifierFlagCommand];
  [stopRecordingItem setTarget:menuDelegate];
  [recordMenu addItem:stopRecordingItem];
  
  // Update menu state
  updateMenuState();
#endif
}

void OutputWindow::updateMenuState() {
#ifdef __OBJC__
  bool isRecording = vidRecorder.isRecording();
  [startRecordingItem setHidden:isRecording];
  [stopRecordingItem setHidden:!isRecording];
#endif
}

void OutputWindow::update()
{
}

void OutputWindow::draw()
{
  if (needsResolutionUpdate) updateResolution();
  
  fbo->draw(0, 0, ofGetWindowWidth(), ofGetWindowHeight());

  // Record frame if recording
  if (vidRecorder.isRecording()) {
    // Use ofxFastFboReader to speed up pixel reading
    // Important: Ensure FBO/reader is providing BGRA pixels for the wrapper
    if (reader.readToPixels(*fbo, recordPixels)) { 
       if (recordPixels.getWidth() > 0 && recordPixels.getHeight() > 0) {
          
          // Log the pixel format once
          if (!loggedPixelFormat) {
            ofLogNotice("OutputWindow") << "Recording using pixel format: " << recordPixels.getPixelFormat();
            // You can compare this to OF_PIXELS_RGB, OF_PIXELS_RGBA, OF_PIXELS_BGRA etc.
            loggedPixelFormat = true; 
          }

          // Add the frame to the AVFoundation recorder
          vidRecorder.addFrame(recordPixels);
       } else {
           ofLogWarning("OutputWindow") << "Read empty pixels from FBO.";
       }
    } else {
      ofLogWarning("OutputWindow") << "Failed to read pixels from FBO.";
    }
  }
  
  // Update window title based on recording status or FPS
  if (vidRecorder.isRecording()) {
    float elapsedTime = vidRecorder.getElapsedTime();
    int minutes = static_cast<int>(elapsedTime / 60.0f);
    int seconds = static_cast<int>(fmod(elapsedTime, 60.0f));
    ofSetWindowTitle(formatString("REC ● %02d:%02d", minutes, seconds));
  } else if (drawFPS) {
    ofSetWindowTitle(formatString("%.2f FPS", ofGetFrameRate()));
  } else {
    // Reset title if neither recording nor showing FPS (optional)
    // ofSetWindowTitle(""); 
  }
}

void OutputWindow::updateResolution()
{
//  bool portrait = LayoutStateService::getService()->portrait;
//  ImVec2 res = LayoutStateService::getService()->resolution;
//  ofSetWindowShape(portrait ? res.y : res.x, portrait ? res.x : res.y);
//  needsResolutionUpdate = false;
}

void OutputWindow::startRecording()
{
  // Present save dialog to choose recording destination
  std::string defaultName = "Recording_" + ofGetTimestampString() + ".mov";
  ofFileDialogResult result = ofSystemSaveDialog(defaultName, "Save Recording");
  if (!result.bSuccess) {
    return; // User cancelled
  }

  recordingPath = result.getPath();
  ofLogNotice("OutputWindow") << "Attempting to record to: " << recordingPath;

  bool setupOk = vidRecorder.setup(recordingPath, fbo->getWidth(), fbo->getHeight(), 60.0f, "h264"); 
  
  if (setupOk) {
    loggedPixelFormat = false; // Reset flag before starting
    vidRecorder.start(); // start() logs success/failure internally
    updateMenuState();
  } else {
    ofLogError("OutputWindow") << "Failed to setup recorder.";
  }
}

void OutputWindow::stopRecording()
{
  vidRecorder.stop([this](bool success){
    ofLogNotice("OutputWindow") << "Recording finished: " << (success ? "Success" : "Failed");
    this->loggedPixelFormat = false; // Reset flag
    // Reveal recording in Finder if path exists and recording succeeded
    if (success && !recordingPath.empty()) {
      std::string command = "open \"" + ofFilePath::getEnclosingDirectory(recordingPath) + "\"";
      std::system(command.c_str());
    }
    updateMenuState();
  });
  ofSetWindowTitle("");
}

void OutputWindow::keyReleased(int key) {
  // If the key released is ESC, close the output window
  if (key == OF_KEY_ESC) {
    // Stop recording if active before closing
    if (vidRecorder.isRecording()) {
      stopRecording();
    }
    VideoSourceService::getService()->closeOutputWindow(shared_from_this());
    ofGetCurrentWindow()->setWindowShouldClose();
  }
  
  if (key == OF_KEY_SHIFT) {
    drawFPS = false;
    ofSetWindowTitle("");
  }
  
  if (key == 'r' && ofGetKeyPressed(OF_KEY_COMMAND)) { // Require Cmd+R
    if (vidRecorder.isRecording()) {
      stopRecording();
    } else {
      startRecording();
    }
  }
}

void OutputWindow::keyPressed(int key) {
  if (key == OF_KEY_SHIFT) {
    drawFPS = true;
  }
}

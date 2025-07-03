//
//  OutputWindow.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/30/22.
//

#ifndef OutputWindow_hpp
#define OutputWindow_hpp

#include <stdio.h>
#include "ofMain.h"
#include "Connection.hpp"
#include "ofxFastFboReader.h"
#include "AVFRecorderWrapper.hpp"

#ifdef __OBJC__
@class NSMenu;
@class NSMenuItem;
@class OutputWindowDelegate;
#endif

struct OutputWindow : public ofBaseApp, std::enable_shared_from_this<OutputWindow>
{
  OutputWindow(std::shared_ptr<ofFbo> fbo, std::shared_ptr<Connectable> connectable) : fbo(fbo), connectable(connectable)
  { };

  void setup();
  void update();
  void draw();
  void teardown();
  void keyReleased(int key);
  void keyPressed(int key);
  void updateResolution();
  void startRecording();
  void stopRecording();
  void setupNativeMenu();
  void updateMenuState();
  bool drawFPS;
  bool needsResolutionUpdate = true;
  
  // Recording variables
  AVFRecorderWrapper vidRecorder;
  ofPixels recordPixels;
  ofxFastFboReader reader;
  bool loggedPixelFormat = false;

  // Path of the current recording (used to reveal in Finder when finished)
  std::string recordingPath;

#ifdef __OBJC__
  // Objective-C++ menu variables
  NSMenu* recordMenu;
  NSMenuItem* startRecordingItem;
  NSMenuItem* stopRecordingItem;
  OutputWindowDelegate* menuDelegate;
#else
  // Placeholder for non-Objective-C++ builds
  void* recordMenu;
  void* startRecordingItem;
  void* stopRecordingItem;
  void* menuDelegate;
#endif

public:
  std::shared_ptr<ofFbo> fbo;
  std::shared_ptr<Connectable> connectable;
};

#endif /* OutputWindow_hpp */



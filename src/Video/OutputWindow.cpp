//
//  OutputWindow.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/30/22.
//

#include "OutputWindow.hpp"
#include "VideoSourceService.hpp"
#include "LayoutStateService.hpp"

void OutputWindow::setup() {
//  LayoutStateService::getService()->subscribeToResolutionUpdates([this]() {
//    needsResolutionUpdate = true;
//  });

  // Initialize video recorder - AVFRecorderWrapper setup is called when starting
  // No explicit setup needed here unless defaults change
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

void OutputWindow::keyReleased(int key) {
  // If the key released is ESC, close the output window
  if (key == OF_KEY_ESC) {
    // Stop recording if active before closing
    if (vidRecorder.isRecording()) {
        vidRecorder.stop([](bool success){ 
          ofLogNotice("OutputWindow") << "Recording stopped on ESC: " << (success ? "Success" : "Failed");
        });
    }
    VideoSourceService::getService()->closeOutputWindow(shared_from_this());
    ofGetCurrentWindow()->setWindowShouldClose();
  }
  
  if (key == OF_KEY_SHIFT) {
    drawFPS = false;
    ofSetWindowTitle("");
  }
  
  if (key == 'r') { // Use lowercase 'r'
    if (vidRecorder.isRecording()) {
      // Stop recording
      vidRecorder.stop([this](bool success){
        ofLogNotice("OutputWindow") << "Recording finished: " << (success ? "Success" : "Failed");
        this->loggedPixelFormat = false; // Reset flag
      });
      ofSetWindowTitle("");
    }
    else {
      // Start recording
      ofFilePath f;
      // Save to Desktop for now, adjust path as needed
      string myPath = ConfigService::getService()->exportsFolderFilePath() + "/Recording_" + ofGetTimestampString() + ".mov";
      ofLogNotice("OutputWindow") << "Attempting to record to: " << myPath;

      // Setup recorder just before starting
      // Using H.264 codec by default. Change "h264" to "prores" or others if needed.
      bool setupOk = vidRecorder.setup(myPath, fbo->getWidth(), fbo->getHeight(), 60.0f, "h264"); 
      
      if (setupOk) {
           loggedPixelFormat = false; // Reset flag before starting
           vidRecorder.start(); // start() logs success/failure internally
      } else {
         ofLogError("OutputWindow") << "Failed to setup recorder.";
      }
    }
  }
}

void OutputWindow::keyPressed(int key) {
  if (key == OF_KEY_SHIFT) {
    drawFPS = true;
  }
}

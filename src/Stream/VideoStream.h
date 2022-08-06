//
//  VideoStream.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-07.
//

#ifndef VideoStream_hpp
#define VideoStream_hpp

#include "ofMain.h"
#include "VideoSettings.h"
#include "ofBaseApp.h"
#include "Oscillator.hpp"
#include "FeedbackShader.hpp"
#include "MainSettings.h"
#include "ofxImGui.h"

class VideoStream : public ofBaseApp {
public:
  VideoStream(std::shared_ptr<ofAppBaseWindow> window, StreamConfig config, VideoSettings *settings,   std::function<void(int)> closeStream) :
  position(Parameter("playerPosition", config.streamId, 0.0)),
  speed(Parameter("playerSpeed", config.streamId, 1.0, 0.0, 4.0)),
  feedbackShaders({FeedbackShader(&settings->feedback0Settings, &shaderMixer, 0),
    FeedbackShader(&settings->feedback1Settings, &shaderMixer, 1),
    FeedbackShader(&settings->feedback2Settings, &shaderMixer, 2)
  }),
  window(window),
  config(config),
  settings(settings),
  closeStream(closeStream) {};
  
  
  void setup();
  void update();
  void draw();
  void teardown();
  
  // Shading
  
  
  std::function<void(int)> closeStream;
  bool isSetup;
  bool shouldClearFrameBuffer;
  
  // Whether we've drawn the current frame for this stream.
  // Used in draw() event for ensuring all are drawn.
  bool frameDrawn;
  bool firstFrameDrawn;
  StreamConfig config;
  VideoSettings *settings;
private:
  // Drawing
  void prepareFbos();
  void completeFrame();
  void drawMainFbo();
  void drawDebug();
  void drawVideo();
  void drawVideoPlayer();
  
  // Shading
  void shadeBlur();
  void shadeHSB();
  void shadeFeedback();
  void shadeSharpen();
  void shadeGlitch();
  void saveFeedbackFrame();
  void resetFeedbackValues(FeedbackSettings *feedback);
  void clearFrameBuffer();
  
  // Events
  void firstDrawSetup();
  void willExit(ofEventArgs &args);
  
  // Helpers
  ofTexture streamTexture();
  float streamWidth();
  float streamHeight();
  
  
  ofxImGui::Gui gui;
  std::shared_ptr<ofAppBaseWindow> window;
  ofVideoPlayer player;
  ofVideoGrabber cam;
  ofShader shaderGlitch;
  ofShader shaderMixer;
  ofShader shaderBlur;
  ofShader shaderSharpen;
  ofFbo fbo;
  ofFbo fboBlur;
  ofFbo fboSharpen;
  ofFbo fboFeedback;
  Parameter position;
  Parameter speed;
  
  // Vector of vectors of feedback frames
  std::vector<ofFbo> frameBuffer;
  std::vector<FeedbackShader> feedbackShaders;
  long frameCount = 1;
  int frameDelayOffset;
};

#endif /* VideoStream_hpp */

//
//  VideoStream.hpppp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-07.
//

#ifndef VideoStream_hpp
#define VideoStream_hpp

#include "ofMain.h"
#include "ShaderChainer.hpp"
#include "VideoSettings.hpp"
#include "GlitchShader.hpp"
#include "HSBShader.hpp"
#include "PixelShader.hpp"
#include "BlurShader.hpp"
#include "ofBaseApp.h"
#include "Oscillator.hpp"
#include "FeedbackShader.hpp"
#include "VideoSource.hpp"
#include "MainSettings.hpp"
#include "ofxImGui.h"

class VideoStream : public ofBaseApp {
public:
  VideoStream(
              std::shared_ptr<ofAppBaseWindow> window,
              StreamConfig config,
              VideoSettings *settings,
              ShaderChainer *chainer,
              std::function<void(int)> closeStream) :
  position(Parameter("playerPosition", config.streamId, 0.0)),
  speed(Parameter("playerSpeed", config.streamId, 1.0, 0.0, 4.0)),
  outputChainer(chainer),
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
  ShaderChainer *outputChainer;
  
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
  void prepareMainFbo();
  void drawMainFbo();
  void drawDebug();
  void drawVideo();
  void drawVideoPlayer();
  void drawVideoPlayerMenu();
  
  // Shading
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
  std::shared_ptr<VideoSource> videoSource;
  ofVideoPlayer player;
  ofVideoGrabber cam;
  ofFbo fbo;
  ofTexture frameTexture;
  Parameter position;
  Parameter speed;
  
  long frameCount = 1;
  int frameDelayOffset;
};

#endif /* VideoStream_hpp */

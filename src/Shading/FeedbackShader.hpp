//
//  FeedbackShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/5/22.
//

#ifndef FeedbackShader_hpp
#define FeedbackShader_hpp

#include <stdio.h>
#include "ofMain.h"
#include "Shader.hpp"
#include "VideoSettings.hpp"
#include "ofMain.h"
#include "ofxImGui.h"

struct FeedbackShader: Shader  {

  FeedbackSettings *settings;
  ofShader shader;
  std::vector<ofTexture> frameBuffer;
  int idx = 0;
  ofFbo fboFeedback;

  FeedbackShader(FeedbackSettings *settings, int idx) : Shader(settings), settings(settings), idx(idx) {};
  
  void saveFrame(ofFbo *frame);
  void shade();
  void clearFrameBuffer();
  void disableFeedback();
  
  virtual void setup();
  virtual void shade(ofFbo *frame, ofFbo *canvas);
  virtual void clear();
  virtual bool enabled();
  virtual std::string name();
  virtual void drawSettings();
  virtual ShaderType type();
};

#endif /* FeedbackShader_hpp */

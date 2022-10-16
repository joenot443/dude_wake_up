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
#include "VideoSettings.h"
#include "ofMain.h"
#include "ofxImGui.h"

struct FeedbackShader: Shader  {
  FeedbackShader(FeedbackSettings *feedback, int idx) : feedback(feedback), idx(idx) {};
  FeedbackSettings *feedback;
  ofShader shader;
  std::vector<ofTexture> frameBuffer;
  int idx = 0;
  ofFbo fboFeedback;
  

  void saveFrame(ofFbo *frame);
  void shade();
  void clearFrameBuffer();
  void disableFeedback();
  
  virtual void setup();
  virtual void shade(ofFbo *frame, ofFbo *canvas);
  virtual void clear();
  virtual bool enabled();
};

#endif /* FeedbackShader_hpp */

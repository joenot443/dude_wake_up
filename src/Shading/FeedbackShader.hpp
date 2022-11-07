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
  ofFbo fboFeedback;
  std::shared_ptr<FeedbackSource> feedbackSource;
  
  FeedbackShader(FeedbackSettings *settings) : Shader(settings), settings(settings) {};
  
  void shade();
  void clearFrameBuffer();
  void disableFeedback();
  void drawFeedbackSourceSelector();
  
  virtual void setup();
  virtual void shade(ofFbo *frame, ofFbo *canvas);
  virtual void drawSettings();
  virtual ShaderType type();
};

#endif /* FeedbackShader_hpp */

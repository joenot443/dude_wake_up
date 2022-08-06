//
//  FeedbackShader.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/5/22.
//

#ifndef FeedbackShader_hpp
#define FeedbackShader_hpp

#include <stdio.h>
#include "VideoSettings.h"
#include "ofMain.h"

struct FeedbackShader {
  FeedbackShader(FeedbackSettings *feedback, ofShader *shaderMixer, int idx) : feedback(feedback), shaderMixer(shaderMixer), idx(idx) {};
  FeedbackSettings *feedback;
  ofShader *shaderMixer;
  int idx = 0;
  void shade();
  void disableFeedback();
};

#endif /* FeedbackShader_hpp */

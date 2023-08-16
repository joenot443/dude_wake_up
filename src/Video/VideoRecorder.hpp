//
//  VideoRecorder.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 7/26/23.
//

#ifndef VideoRecorder_hpp
#define VideoRecorder_hpp

#include <stdio.h>
#include "ShaderChainerService.hpp"
#include "ofxVideoRecorder.h"

struct VideoRecorder
{
public:
  void setup(std::shared_ptr<ShaderChainer> target);
  void startRecording();
  void stopRecording();
  void save();
  
  bool isRecording();

  std::shared_ptr<ShaderChainer> shaderChainer;
  
  ofxVideoRecorder recorder;
};

#endif /* VideoRecorder_hpp */

//
//  ShaderChainer.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef ShaderChainer_hpp
#define ShaderChainer_hpp

#include <stdio.h>
#include "ofMain.h"
#include "ofxImGui.h"
#include "FeedbackShader.hpp"
#include "Shader.hpp"

struct ShaderChainer {
  ofFbo fboChainingShaders(ofFbo texture);
  std::string settingsId;
  std::vector<FeedbackShader *> feedbackShaders;
  std::vector<Shader *> shaders;

  void pushShader(ShaderType shaderType);
  void deleteShader(Shader *);
  
  ShaderChainer(std::string settingsId) :
  settingsId(settingsId),
  feedbackShaders({}),
  shaders({}) {};
};


#endif /* ShaderChainer_hpp */

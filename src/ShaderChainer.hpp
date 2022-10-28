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
#include "HSBShader.hpp"
#include "json.hpp"
#include "macro_scope.hpp"

using json = nlohmann::json;

struct ShaderChainer {
  ofFbo fboChainingShaders(ofFbo texture);
  std::vector<FeedbackShader *> feedbackShaders;
  std::vector<Shader *> shaders;
  std::string settingsId;

  void pushShader(ShaderType shaderType);
  void deleteShader(Shader *);
  
  ShaderChainer(std::string settingsId) :
  settingsId(settingsId),
  feedbackShaders({}),
  shaders({}) {};
  
  json serialize();
  void load(json j);
};


#endif /* ShaderChainer_hpp */

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
#include "FeedbackSource.hpp"
#include "macro_scope.hpp"
#include "VideoSource.hpp"

using json = nlohmann::json;

struct ShaderChainer {
  void setup();
  void update();
  
  std::string name;
  
  ofFbo fbo;
  ofFbo processFrame(std::shared_ptr<ofTexture> frameTexture);
  
  std::vector<std::shared_ptr<FeedbackShader>> feedbackShaders;
  std::vector<std::shared_ptr<Shader>> shaders;
  std::shared_ptr<VideoSource> source;
  
  std::shared_ptr<FeedbackSource> feedbackDestination;
  
  std::string chainerId;

  
  void pushShader(ShaderType shaderType);
  void deleteShader(std::shared_ptr<Shader> shader);
  
  void saveFeedbackFrame(ofTexture frame);
  
  ShaderChainer(std::string chainerId,
                std::shared_ptr<VideoSource> source) :
  chainerId(chainerId),
  feedbackShaders({}),
  name(chainerId),
  source(source),
  shaders({}) {};
  
  json serialize();
  void load(json j);
  void registerFeedbackDestination();
};


#endif /* ShaderChainer_hpp */

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
#include "UUID.hpp"
#include "ofxImGui.h"
#include "FeedbackShader.hpp"
#include "Shader.hpp"
#include "HSBShader.hpp"
#include "json.hpp"
#include "FeedbackSource.hpp"
#include "FileBrowserView.hpp"
#include "macro_scope.hpp"
#include "VideoSource.hpp"

using json = nlohmann::json;

class ShaderChainer: public VideoSource {
public:
  void setup();
  void update();
  
  std::string name;

  ofFbo fbo;
  ofFbo ping;
  ofFbo pong;
  ofFbo processFrame(std::shared_ptr<ofTexture> frameTexture);
  
  std::vector<std::shared_ptr<Shader>> shaders;
  std::shared_ptr<VideoSource> source;
  
  std::shared_ptr<FeedbackSource> feedbackDestination;
  
  std::string chainerId;
  std::shared_ptr<FileBrowserSettings> fileBrowserSettings;
  
  void pushShader(ShaderType shaderType);
  void deleteShader(std::shared_ptr<Shader> shader);
  
  void saveFeedbackFrame(std::shared_ptr<ofTexture> frame);
  void saveFrame();
  
  ShaderChainer(std::string chainerId,
                std::string name,
                std::shared_ptr<VideoSource> source) :
  chainerId(chainerId),
  fileBrowserSettings(std::make_shared<FileBrowserSettings>()),
  name(name),
  source(source),
  shaders({}),
  VideoSource(UUID::generateUUID(), name, VideoSource_chainer) {};
  
  json serialize();
  void load(json j);
  void registerFeedbackDestination();
};


#endif /* ShaderChainer_hpp */

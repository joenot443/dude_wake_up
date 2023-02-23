//
//  ShaderChainer.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef ShaderChainer_hpp
#define ShaderChainer_hpp

#include "FeedbackShader.hpp"
#include "FeedbackSource.hpp"
#include "FileBrowserView.hpp"
#include "HSBShader.hpp"
#include "Shader.hpp"
#include "UUID.hpp"
#include "VideoSource.hpp"
#include "json.hpp"
#include "macro_scope.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include <stdio.h>

using json = nlohmann::json;

class ShaderChainer : public VideoSource, public std::enable_shared_from_this<ShaderChainer> {
public:
  void setup();
  void update();

  std::string name;

  ofFbo fbo;
  ofFbo ping;
  ofFbo pong;
  ofFbo processFrame(std::shared_ptr<ofTexture> frameTexture);
  unsigned int creationTime;
  std::vector<std::shared_ptr<Shader>> shaders();
  std::shared_ptr<Shader> front;
  std::shared_ptr<Shader> frontAux;
  
  std::shared_ptr<VideoSource> source;

  std::shared_ptr<FeedbackSource> feedbackDestination;

  std::string chainerId;

  void pushShader(ShaderType shaderType);
  void deleteShader(std::shared_ptr<Shader> shader);

  void saveFeedbackFrame(std::shared_ptr<ofTexture> frame);
  void saveFrame();

  ShaderChainer(std::string chainerId, std::string name,
                std::shared_ptr<VideoSource> source)
      : chainerId(chainerId),
        creationTime(ofGetUnixTime()),
        name(name), source(source),
        VideoSource(UUID::generateUUID(), name, VideoSource_chainer)
  {};

  json serialize();
  void load(json j);
  void registerFeedbackDestination();
};

#endif /* ShaderChainer_hpp */

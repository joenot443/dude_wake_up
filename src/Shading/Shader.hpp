//
//  Shader.h
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef Shader_h
#define Shader_h

#include "Console.hpp"
#include "FeedbackSource.hpp"
#include "ShaderSettings.hpp"
#include "ShaderType.hpp"
#include "VideoSource.hpp"
#include "WaveformOscillator.hpp"
#include "json.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include <stdio.h>

using json = nlohmann::json;

struct Shader {
  Shader(ShaderSettings *settings)
      : settings(std::unique_ptr<ShaderSettings>(settings)),
        shaderId(settings->shaderId), creationTime(ofGetUnixTime()) {};

  std::unique_ptr<ShaderSettings> settings;
  std::string shaderId;

  // Output
  std::shared_ptr<Shader> next;
  
  // Input
  std::shared_ptr<Shader> parent;
  std::shared_ptr<VideoSource> parentSource;
  std::shared_ptr<Shader> aux;
  std::shared_ptr<VideoSource> sourceAux;
  
  unsigned int creationTime;

  virtual void setup(){};
  virtual void shade(ofFbo *frame, ofFbo *canvas){};
  virtual void clear(){};
  virtual bool supportsAux() { return shaderTypeSupportsAux(type()); }
  virtual bool auxConnected() { return aux != nullptr || sourceAux != nullptr; }
  
  std::shared_ptr<FeedbackSource> feedbackDestination();
  virtual std::string id() { return settings->shaderId; };
  virtual std::string name() { return shaderTypeName(type()); };
  virtual bool enabled() { return true; };
  virtual bool hasFrameBuffer() { return false; };
  void saveFrame(ofFbo *frame){};
  
  ofTexture auxTexture();

  virtual ShaderType type() { return ShaderTypeNone; };

  virtual void drawSettings(){};

  virtual std::string idName() { return formatString("%s##%s", name().c_str(), id().c_str()); }

  json serialize() {
    json j = settings->serialize();
    j["shaderType"] = type();
    j["shaderId"] = settings->shaderId;
    return j;
  };
};

#endif /* Shader_h */

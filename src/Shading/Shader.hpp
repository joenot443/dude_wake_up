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
        shaderId(settings->shaderId), creationTime(ofGetUnixTime()) {
          lastFrame = ofFbo();
          lastFrame.allocate(1280, 720, GL_RGBA);
        };

  std::unique_ptr<ShaderSettings> settings;
  std::string shaderId;
  
  ofFbo lastFrame;

  // Output
  std::shared_ptr<Shader> next;
  
  // Input
  std::shared_ptr<Shader> parent;
  std::shared_ptr<VideoSource> parentSource;
  std::shared_ptr<Shader> aux;
  std::shared_ptr<VideoSource> auxSource;
  std::shared_ptr<Shader> mask;
  std::shared_ptr<VideoSource> sourceMask;
  
  unsigned int creationTime;

  virtual void setup(){};
  virtual void shade(ofFbo *frame, ofFbo *canvas){};
  virtual void clear(){};
  
  /// Aux
  ofTexture auxTexture();
  virtual bool supportsAux() { return shaderTypeSupportsAux(type()); }
  virtual bool auxConnected() {
    return aux != nullptr || auxSource != nullptr;
  }
  
  /// Mask
  ofTexture maskTexture();
  virtual bool supportsMask() { return shaderTypeSupportsMask(type()); }
  virtual bool maskConnected() {
    return mask != nullptr || sourceMask != nullptr;
  }
  
  /// Feedback
  
  // The FeedbackSource which this Shader will write to.
  // When this Shader's next is set to a FeedbackShader, this Shader
  // will act as a FeedbackSource.
  std::shared_ptr<FeedbackSource> shaderFeedbackDestination;
  
  // The FeedbackSource which this Chainer will write to.
  std::shared_ptr<FeedbackSource> chainerFeedbackDestination();
  
  virtual std::string name() { return shaderTypeName(type()); };
  virtual bool enabled() { return true; };
  virtual bool hasFrameBuffer() { return false; };
  void saveFrame(ofFbo *frame){};
  
  virtual ShaderType type() { return ShaderTypeNone; };

  virtual void drawSettings(){};
  
  void drawPreview(ImVec2 pos) {
    ImTextureID texID = (ImTextureID)(uintptr_t)lastFrame.getTexture().getTextureData().textureID;
    ImGui::Image(texID, ImVec2(160, 120));
  }

  virtual std::string idName() { return formatString("%s##%s", name().c_str(), shaderId.c_str()); }

  json serialize();
  
  // Called after all the Shaders have been created.
  // Used to populate aux.
  void load(json j);
};

#endif /* Shader_h */

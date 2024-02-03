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
#include "Connection.hpp"
#include <stdio.h>

using json = nlohmann::json;

class Shader: public Connectable
{
public:
  
  Shader(ShaderSettings *settings)
      : settings(std::unique_ptr<ShaderSettings>(settings)),
        shaderId(settings->shaderId), creationTime(ofGetUnixTime()), lastFrame(std::make_shared<ofFbo>())
  {
    lastFrame->allocate(1280, 720, GL_RGBA);
  };
  
  ofShader shader;
  std::unique_ptr<ShaderSettings> settings;
  
  std::string shaderId;

  std::shared_ptr<ofFbo> lastFrame;

  unsigned int creationTime;

  virtual void setup(){};
  virtual void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas){};
  virtual void clear(){};
  
  /// Feedback

  /// Aux
  std::shared_ptr<Connectable> aux();
  virtual bool supportsAux() { return shaderTypeSupportsAux(type()); }
  virtual bool auxConnected()
  {
    return hasInputForType(ConnectionTypeAux);
  }

  /// Mask
  std::shared_ptr<Connectable> mask();
  virtual bool supportsMask() { return shaderTypeSupportsMask(type()); }
  virtual bool maskConnected()
  {
    return hasInputForType(ConnectionTypeMask);
  }

  /// Feedback

  // The FeedbackSource which this Shader will write to.
  // When this Shader's next is set to a FeedbackShader, this Shader
  // will act as a FeedbackSource.
  std::shared_ptr<FeedbackSource> feedbackDestination;

  virtual std::string name() { return shaderTypeName(type()); };
  virtual bool enabled() { return true; };
  virtual bool hasFrameBuffer() { return false; };
  void saveFrame(ofFbo *frame){};

  virtual ShaderType type() { return ShaderTypeNone; };

  virtual void drawSettings(){};

  void drawPreview(ImVec2 pos, float scale)
  {
    ImTextureID texID = (ImTextureID)(uintptr_t)lastFrame->getTexture().getTextureData().textureID;
    ImGui::Image(texID, ImVec2(160/scale, 120/scale));
  }

  virtual std::string idName() { return formatString("%s##%s", name().c_str(), shaderId.c_str()); }
  
  // Connectable
  
  std::shared_ptr<ofFbo> frame() {
    return lastFrame;
  }
  
  std::string connId() {
    return shaderId;
  }
  
  ConnectableType connectableType() {
    return ConnectableTypeShader;
  }
  
  std::shared_ptr<VideoSourceSettings> sourceSettings();
  
  // Applies Shader to the current frame, writing to the lastFrame Fbo.
  // Calls traverseFrame() on Shaders in the next set.
  void traverseFrame(std::shared_ptr<ofFbo> frame);
  
  // Clears the lastFrame back to a 100% black
  void clearLastFrame();

  json serialize();

  // Called after all the Shaders have been created.
  // Used to populate aux.
  void load(json j);
};

#endif /* Shader_h */

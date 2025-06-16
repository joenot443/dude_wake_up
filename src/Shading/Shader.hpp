//
//  Shader.h
//  dude_wake_up
//
//  Created by Joe Crozier on 8/30/22.
//

#ifndef Shader_h
#define Shader_h

#include "Console.hpp"
#include "imgui.h"
#include "FeedbackSource.hpp"
#include "ShaderSettings.hpp"
#include "ShaderType.hpp"
#include "VideoSource.hpp"
#include "WaveformOscillator.hpp"
#include "json.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include "Connection.hpp"
#include "Colors.hpp"
#include <stdio.h>

using json = nlohmann::json;

static const bool AllowShaderMonitoring = true;

class Shader: public Connectable
{
public:
  
  Shader(ShaderSettings *settings)
  : settings(std::shared_ptr<ShaderSettings>(settings)),
  shaderId(settings->shaderId),
  creationTime(ofGetUnixTime()),
  lastModified(ofGetUnixTime()),
  lastFrame(std::make_shared<ofFbo>()),
  shader(ofShader()),
  optionalFrame(std::make_shared<ofFbo>())
  {
    color = colorFromName(settings->name);
  };
  
  ofShader shader;
  
  std::shared_ptr<ShaderSettings> settings;
  
  ImColor color;
  
  std::string shaderId;
  
  // Whether this Shader is an optional phase in another Shader
  bool isOptional;
  
  // If this Shader is optional, whether it is presently enabled
  bool optionallyEnabled;
  
  std::shared_ptr<ofFbo> lastFrame;
  
  // Used for applying the optional shaders
  std::shared_ptr<ofFbo> optionalFrame;
  
  // Shaders which can be optionally applied to an existing Shader.
  std::vector<std::shared_ptr<Shader>> optionalShaders;
  
  std::shared_ptr<ofFbo> parentFrame() override;
  
  // Timestamp for the last time the Shader file was modified
  unsigned int lastModified;
  
  unsigned int creationTime;
  
  virtual void setup(){};
  virtual void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas){};
  virtual void clear(){};
  
  // Feedback
  // The Feedback OUTPUT pin for shading feedback frames before injection
  std::shared_ptr<Connectable> feedback();
  
  // The FeedbackSource which this Shader will write to.
  // When this Shader's next is set to a FeedbackShader, this Shader
  // will act as a FeedbackSource.
  std::shared_ptr<FeedbackSource> feedbackDestination;
  
  void allocateFrames();
  
  virtual std::string name() { return shaderTypeName(type()); };
  virtual bool enabled() { return true; };
  virtual bool hasFrameBuffer() { return false; };
  virtual std::string fileName() { return name(); };
  void saveFrame(ofFbo *frame){};
  
  void checkForFileChanges();
  
  void enableAudioAutoReactivity(std::shared_ptr<Parameter> audioParam);
  void disableAudioAutoReactivity();

  virtual ShaderType type() { return ShaderTypeNone; };

  void drawSettings() override {};
  
  std::shared_ptr<Settings> settingsRef() override { 
    return std::dynamic_pointer_cast<Settings>(settings);
  }

  virtual void drawPreview(ImVec2 pos, float scale);
  
  virtual void drawPreviewSized(ImVec2 size);

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
  void traverseFrame(std::shared_ptr<ofFbo> frame, int depth);
  
  void applyOptionalSettings();
  
  // Clears the lastFrame back to a 100% black
  void clearLastFrame();

  json serialize();
  
  bool allowAuxOutputSlot();
  

  // Called after all the Shaders have been created.
  // Used to populate aux.
  void load(json j);
  
  // Sets the Shader's Parameters to be active.
  void activateParameters();
  
  // Optional Shaders
  
  void generateOptionalShaders();
  
  void drawOptionalSettings();
  void applyOptionalShaders();
  void populateLastFrame();
};

#endif

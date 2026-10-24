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
#include "CachedShader.hpp"
#include "json.hpp"
#include "ofMain.h"
#include "ofxImGui.h"
#include "Connection.hpp"
#include "Colors.hpp"
#include "LifecycleCounter.hpp"
#include <stdio.h>

using json = nlohmann::json;

static const bool AllowShaderMonitoring = false;

class Shader: public Connectable
{
public:
  
  Shader(ShaderSettings *settings)
  : settings(std::shared_ptr<ShaderSettings>(settings)),
  shaderId(settings->shaderId),
  creationTime(ofGetUnixTime()),
  lastModified(ofGetUnixTime()),
  lastFrame(std::make_shared<ofFbo>()),
  optionalFrame(std::make_shared<ofFbo>())
  {
    color = colorFromName(settings->name);
  };
  
  // Shared with other instances of the same effect (see CachedShader).
  CachedShader shader;
  
  std::shared_ptr<ShaderSettings> settings;
  
  ImColor color;
  
  std::string shaderId;
  
  // Whether this Shader is an optional phase in another Shader
  bool isOptional = false;
  
  // If this Shader is optional, whether it is presently enabled
  bool optionallyEnabled = false;

  // Whether this Shader is "dry" i.e. not rendering new frames but still allowing traversal
  bool isDry = false;

  std::shared_ptr<ofFbo> lastFrame;
  
  // Used for applying the optional shaders
  std::shared_ptr<ofFbo> optionalFrame;
  
  // Shaders which can be optionally applied to an existing Shader.
  OptionalShadersHelper optionalShaders{OptionalShadersHelper::shaderTypes(), false};
  
  std::shared_ptr<ofFbo> parentFrame() override;
  
  // Timestamp for the last time the Shader file was modified
  unsigned int lastModified;
  
  unsigned int creationTime;
  
  virtual void setup(){};
  virtual void shade(std::shared_ptr<ofFbo> frame, std::shared_ptr<ofFbo> canvas){};
  virtual void clear(){};

  // ---------------------------------------------------------------------
  // Session-only GLSL editing.
  //
  // When the user edits a shader's fragment source from the inspector,
  // we keep the edited string here and recompile in-place. Empty means
  // "no override; load straight from the disk .frag file as usual".
  //
  // Reverting just clears the override and re-loads from disk.
  // Nothing is persisted across sessions.
  // ---------------------------------------------------------------------
  std::string editedFragmentSource;

  // Populated by applyFragmentSourceOverride() when a compile or link
  // fails. The previously-valid program keeps running; the UI surfaces
  // these for the user.
  std::string lastCompileError;
  int lastCompileErrorLine = 0;

  bool hasSessionOverride() const { return !editedFragmentSource.empty(); }

  // Returns the source the user should see in the editor: their override
  // if one is active, otherwise the contents of shaders/<fileName>.frag
  // read from disk.
  std::string currentFragmentSource();

  // Stage-then-swap recompile. Builds a fresh ofShader from the given
  // fragment source (plus the disk vertex shader), and only replaces
  // `this->shader` if vertex + fragment compile AND the program links.
  // Returns true on success. On failure, lastCompileError /
  // lastCompileErrorLine are populated and the existing shader keeps
  // rendering. MUST run on the engine thread (owns the GL context).
  bool applyFragmentSourceOverride(const std::string& source);

  // Drop the session override and re-load from disk. MUST run on the
  // engine thread.
  void revertFragmentSource();
  
  // Feedback
  // The Feedback OUTPUT pin for shading feedback frames before injection
  std::shared_ptr<Connectable> feedback();
  
  // The FeedbackSource which this Shader will write to.
  // When this Shader's next is set to a FeedbackShader, this Shader
  // will act as a FeedbackSource.
  std::shared_ptr<FeedbackSource> feedbackDestination;
  
  void allocateFrames();

  // Release the GL resources held by this shader's FBOs and any optional
  // children. The shader object itself stays alive (the undo stack typically
  // holds it), but ~32 MB of full-resolution FBOs per shader + recursively
  // ~16 MB per optional child are released eagerly.
  void releaseFrames();

  virtual std::string name() { return shaderTypeName(type()); };
  virtual bool enabled() { return true; };
  virtual bool hasFrameBuffer() { return false; };
  virtual std::string fileName() { return name(); };
  virtual bool allowsDryTraversal() { return false; };
  
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

  virtual bool allowAuxOutputSlot();

  // Breaks any connections on the auxiliary output slot
  void breakAuxillary();

  // Called after all the Shaders have been created.
  // Used to populate aux.
  void load(json j);
  
  // Sets the Shader's Parameters to be active.
  void activateParameters();
  
  // Optional Shaders

  void drawOptionalSettings();
  void applyOptionalShaders();

  void setAudioUniform(std::vector<float> *audio);
  void setAudioUniform(const float *data, size_t count);

  // Subscription to LayoutStateService resolution updates. Stored here so
  // ShaderChainerService::removeShader can sever it explicitly — otherwise
  // a removed shader's setup()/allocateFrames() would still run on every
  // resolution change for as long as anything (e.g. undo) holds the shared_ptr.
  observable::unique_subscription resolutionUpdateSubscription;

  LIFECYCLE_TRACK("Shader");
};

#endif

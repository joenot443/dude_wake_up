// src/Shading/OptionalShadersHelper.hpp

#ifndef OptionalShadersHelper_hpp
#define OptionalShadersHelper_hpp

#include <functional>
#include <memory>
#include <mutex>
#include <string>
#include <vector>
#include "ofFbo.h"
#include "ShaderType.hpp"
#include "json.hpp"

class Shader;

// The extra effects (HSB, Transform, Blur, ...) every Shader and VideoSource
// can apply to its own output. Each slot's Shader is only created when it's
// first needed — enabled, or its settings opened — since every one compiles a
// program and registers parameters, and most nodes never use any. Settings
// loaded for a slot that isn't created are kept as JSON and written back on
// save, so they survive until the effect is used.
//
// Slots are created on the engine thread; the lookups that don't create
// (count, name, enabled, existing) are safe from any thread.
class OptionalShadersHelper {
public:
  static std::vector<ShaderType> shaderTypes();  // a Shader's extras
  static std::vector<ShaderType> sourceTypes();  // a VideoSource's extras

  // `maskDefaults`: colour/luma mask makers pass their input through.
  OptionalShadersHelper(std::vector<ShaderType> types, bool maskDefaults);

  size_t size() const { return types_.size(); }
  std::string name(size_t index) const;
  bool enabled(size_t index) const;
  // The slot's Shader, or null if it hasn't been created.
  std::shared_ptr<Shader> existing(size_t index) const;
  // The slot's Shader, created (with any loaded settings) if need be.
  std::shared_ptr<Shader> shader(size_t index);
  void toggle(size_t index);
  // Every Shader created so far.
  std::vector<std::shared_ptr<Shader>> created() const;

  // Restores the `optionalShaders` array written by serialize(): enabled
  // slots are created now, the others keep their JSON for later.
  void load(const nlohmann::json& entries);
  // An entry per slot, or null if no slot has been used or loaded.
  nlohmann::json serialize() const;

  // Runs the enabled extras over `lastFrame`, using `optionalFrame` (sized to
  // match on first use) as scratch.
  void applyOptionalShaders(std::shared_ptr<ofFbo> lastFrame, std::shared_ptr<ofFbo> optionalFrame);
  void drawOptionalSettings();

private:
  std::shared_ptr<Shader> create(size_t index);
  void populateLastFrame(std::shared_ptr<ofFbo> lastFrame, std::shared_ptr<ofFbo> optionalFrame);

  std::vector<ShaderType> types_;
  bool maskDefaults_;
  mutable std::mutex mutex_;  // guards slots_ and pending_
  std::vector<std::shared_ptr<Shader>> slots_;
  std::vector<nlohmann::json> pending_;  // loaded entries of uncreated slots
};


#endif

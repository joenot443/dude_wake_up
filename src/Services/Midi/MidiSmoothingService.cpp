//
//  MidiSmoothingService.cpp
//  dude_wake_up
//

#include "MidiSmoothingService.hpp"

#include "ofUtils.h"

namespace {
// Global upper bound on MIDI smoothing duration (4 seconds).
constexpr float kMaxDurationMs = 4000.0f;

inline float easeOutCubic(float t) {
  if (t <= 0.0f) return 0.0f;
  if (t >= 1.0f) return 1.0f;
  float u = 1.0f - t;
  return 1.0f - u * u * u;
}
}

void MidiSmoothingService::setEnabled(bool e) {
  enabled = e;
  if (!e) {
    std::lock_guard<std::mutex> lock(stateMutex);
    // Snap any in-flight smoothings to their target on the next tick.
    for (auto& entry : active) {
      entry.second.durationSec = 0.0f;
    }
  }
}

void MidiSmoothingService::setDurationMs(float ms) {
  if (ms < 0.0f) ms = 0.0f;
  if (ms > kMaxDurationMs) ms = kMaxDurationMs;
  durationMs = ms;
}

void MidiSmoothingService::setTarget(std::shared_ptr<Parameter> param, float normalized) {
  if (!param) return;

  float range = param->max - param->min;
  float target = normalized * range + param->min;
  double now = ofGetElapsedTimef();
  float dur = durationMs / 1000.0f;

  std::lock_guard<std::mutex> lock(stateMutex);
  auto it = active.find(param->paramId);
  if (it == active.end()) {
    SmoothingState s;
    s.param = param;
    s.startValue = param->value;
    s.targetValue = target;
    s.startTime = now;
    s.durationSec = dur;
    active[param->paramId] = s;
  } else {
    // Re-aim from the current (possibly mid-smoothing) value.
    it->second.param = param;
    it->second.startValue = param->value;
    it->second.targetValue = target;
    it->second.startTime = now;
    it->second.durationSec = dur;
  }
}

void MidiSmoothingService::tick() {
  if (active.empty()) return;

  double now = ofGetElapsedTimef();

  std::lock_guard<std::mutex> lock(stateMutex);
  for (auto it = active.begin(); it != active.end();) {
    auto param = it->second.param.lock();
    if (!param) {
      it = active.erase(it);
      continue;
    }

    float elapsed = static_cast<float>(now - it->second.startTime);
    if (it->second.durationSec <= 0.0f || elapsed >= it->second.durationSec) {
      param->setValue(it->second.targetValue);
      it = active.erase(it);
      continue;
    }

    float t = elapsed / it->second.durationSec;
    float eased = easeOutCubic(t);
    float v = it->second.startValue + (it->second.targetValue - it->second.startValue) * eased;
    param->setValue(v);
    ++it;
  }
}

void MidiSmoothingService::clearForParam(const std::string& paramId) {
  std::lock_guard<std::mutex> lock(stateMutex);
  active.erase(paramId);
}

json MidiSmoothingService::config() {
  json j;
  j["enabled"] = enabled;
  j["durationMs"] = durationMs;
  return j;
}

void MidiSmoothingService::loadConfig(json j) {
  if (j.is_null() || !j.is_object()) return;
  if (j.contains("enabled") && j["enabled"].is_boolean()) {
    enabled = j["enabled"].get<bool>();
  }
  if (j.contains("durationMs") && j["durationMs"].is_number()) {
    durationMs = j["durationMs"].get<float>();
    if (durationMs < 0.0f) durationMs = 0.0f;
    if (durationMs > kMaxDurationMs) durationMs = kMaxDurationMs;
  }
}

//
//  MidiSmoothingService.hpp
//  dude_wake_up
//
//  Eases MIDI-driven Parameter value changes over a configurable duration so
//  7-bit CC stepping doesn't snap visible properties between adjacent values.
//  MIDI events arrive on the MIDI listener thread; smoothings advance on the
//  engine thread via tick(). A mutex guards the in-flight map.
//

#ifndef MidiSmoothingService_hpp
#define MidiSmoothingService_hpp

#include <map>
#include <memory>
#include <mutex>
#include <string>

#include "ConfigurableService.hpp"
#include "Parameter.hpp"
#include "json.hpp"

using json = nlohmann::json;

class MidiSmoothingService : public ConfigurableService {
public:
  static MidiSmoothingService* service;
  static MidiSmoothingService* getService() {
    if (!service) service = new MidiSmoothingService;
    return service;
  }

  bool isEnabled() const { return enabled; }
  void setEnabled(bool e);

  float getDurationMs() const { return durationMs; }
  void setDurationMs(float ms);

  // Capture a new target for a parameter. Safe to call from the MIDI thread.
  void setTarget(std::shared_ptr<Parameter> param, float normalized);

  // Advance all in-flight smoothings. Engine thread only.
  void tick();

  // Drop any smoothing tracked for a paramId (used when a MIDI binding is removed).
  void clearForParam(const std::string& paramId);

  json config() override;
  void loadConfig(json j) override;

private:
  struct SmoothingState {
    std::weak_ptr<Parameter> param;
    float startValue = 0.0f;
    float targetValue = 0.0f;
    double startTime = 0.0;
    float durationSec = 0.08f;
  };

  bool enabled = false;
  float durationMs = 80.0f;

  std::map<std::string, SmoothingState> active;
  std::mutex stateMutex;
};

#endif /* MidiSmoothingService_hpp */

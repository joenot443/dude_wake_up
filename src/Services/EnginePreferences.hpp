//
//  EnginePreferences.hpp
//  dude_wake_up
//
//  The app preferences the engine itself uses (resolution, MIDI smoothing,
//  clearing on disconnect): loading them at startup, carrying them over from
//  the session on first run, and putting them into effect.
//

#ifndef EnginePreferences_hpp
#define EnginePreferences_hpp

#include "ConfigService.hpp"
#include "LayoutStateService.hpp"
#include "MidiSmoothingService.hpp"
#include "PreferencesService.hpp"
#include "ShaderChainerService.hpp"

#include <string>

namespace EnginePreferences {

// output.resolution's choices, in LayoutStateService's index order.
inline const char* const kResolutions[] = {"240p", "360p", "480p", "720p", "1080p", "1440p", "4k"};
constexpr int kResolutionCount = 7;

inline int resolutionIndex(const std::string& id) {
  for (int i = 0; i < kResolutionCount; ++i) if (id == kResolutions[i]) return i;
  return -1;
}

// Puts them into effect (also after a workspace or session loads, which
// used to set resolution and MIDI smoothing: they're preferences now).
inline void apply() {
  auto* prefs = PreferencesService::getService();
  if (!prefs->loaded()) return;  // the session at startup, before load(): don't impose defaults
  auto* layout = LayoutStateService::getService();
  const int resolution = resolutionIndex(prefs->string("output.resolution"));
  if (resolution >= 0 && layout->resolutionSetting != resolution) layout->updateResolutionSettings(resolution);
  auto* midi = MidiSmoothingService::getService();
  midi->setEnabled(prefs->boolean("midi.smoothing"));
  midi->setDurationMs(static_cast<float>(prefs->number("midi.smoothingMs")));
  ShaderChainerService::getService()->clearOnDisconnect = prefs->boolean("graph.clearOnDisconnect");
}

// First run: the values that used to be saved with the session.
inline void seedFromSession() {
  auto* prefs = PreferencesService::getService();
  const int resolution = LayoutStateService::getService()->resolutionSetting;
  if (resolution >= 0 && resolution < kResolutionCount) prefs->set("output.resolution", kResolutions[resolution]);
  auto* midi = MidiSmoothingService::getService();
  prefs->set("midi.smoothing", midi->isEnabled());
  prefs->set("midi.smoothingMs", midi->getDurationMs());
}

// At engine startup, after the session has loaded.
inline void setup() {
  auto* prefs = PreferencesService::getService();
  prefs->load(ConfigService::getService()->nottawaFolderFilePath() + "/preferences.json", seedFromSession);
  apply();
  prefs->subscribe([](const std::string& id) {
    if (id == "output.resolution" || id == "midi.smoothing" || id == "midi.smoothingMs" || id == "graph.clearOnDisconnect") apply();
  });
}

}  // namespace EnginePreferences

#endif /* EnginePreferences_hpp */

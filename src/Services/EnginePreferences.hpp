//
//  EnginePreferences.hpp
//  dude_wake_up
//
//  The app preferences the engine itself uses (resolution, frame rate, MIDI
//  smoothing, clearing on disconnect, audio): loading them at startup,
//  carrying them over from the session on first run, and putting them into
//  effect. Also the audio input, remembered app-wide.
//

#ifndef EnginePreferences_hpp
#define EnginePreferences_hpp

#include "AudioSourceService.hpp"
#include "ConfigService.hpp"
#include "LayoutStateService.hpp"
#include "MidiSmoothingService.hpp"
#include "PreferencesService.hpp"
#include "ShaderChainerService.hpp"

#include <atomic>
#include <cstdlib>
#include <string>

namespace EnginePreferences {

// output.resolution's choices, in LayoutStateService's index order.
inline const char* const kResolutions[] = {"240p", "360p", "480p", "720p", "1080p", "1440p", "4k"};
constexpr int kResolutionCount = 7;

inline int resolutionIndex(const std::string& id) {
  for (int i = 0; i < kResolutionCount; ++i) if (id == kResolutions[i]) return i;
  return -1;
}

// The engine loop's frame rate (output.frameRate), read every frame.
inline std::atomic<int> frameRate{60};

// Puts them into effect (also after a workspace or session loads, which
// used to set resolution and MIDI smoothing: they're preferences now).
inline void apply() {
  auto* prefs = PreferencesService::getService();
  if (!prefs->loaded()) return;  // the session at startup, before load(): don't impose defaults
  auto* layout = LayoutStateService::getService();
  const std::string res = prefs->string("output.resolution");
  const int resolution = resolutionIndex(res);
  if (resolution >= 0 && layout->resolutionSetting != resolution) layout->updateResolutionSettings(resolution);
  if (res == "custom") {
    const float w = static_cast<float>(prefs->number("output.customWidth"));
    const float h = static_cast<float>(prefs->number("output.customHeight"));
    layout->resolutionSetting = kResolutionCount;  // (no preset)
    if (layout->resolution.x != w || layout->resolution.y != h) layout->setCustomResolution(w, h);
  }
  frameRate = std::max(1, std::atoi(prefs->string("output.frameRate").c_str()));
  auto* audio = AudioSourceService::getService();
  const std::string range = prefs->string("audio.tempoRange");
  if (range == "auto") { audio->tempoMin = 0; audio->tempoMax = 0; }
  else {
    audio->tempoMin = std::atof(range.c_str());
    audio->tempoMax = 2.0 * audio->tempoMin;
  }
  audio->beatOffsetMs[AudioSourceType_File] = prefs->number("audio.beatOffsetFile");
  audio->beatOffsetMs[AudioSourceType_Microphone] = prefs->number("audio.beatOffsetInput");
  audio->beatOffsetMs[AudioSourceType_System] = prefs->number("audio.beatOffsetSystem");
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

// The input used last (app-wide): selected at startup, and started if
// audio.startOnLaunch.
inline void setupAudio() {
  auto* prefs = PreferencesService::getService();
  auto* audio = AudioSourceService::getService();
  const json last = prefs->state("audio.input");
  if (last.is_string()) {
    if (auto source = audio->audioSourceForId(last.get<std::string>()))
      audio->selectedAudioSource = source;  // selected, not started
  } else if (audio->selectedAudioSource) {
    prefs->setState("audio.input", audio->selectedAudioSource->id);  // first run: the session's
  }
  if (prefs->boolean("audio.startOnLaunch") && audio->selectedAudioSource && !audio->selectedAudioSource->active)
    audio->selectAudioSource(audio->selectedAudioSource);  // (sets it up, which starts it)
}

// At engine startup, after the session has loaded.
inline void setup() {
  auto* prefs = PreferencesService::getService();
  prefs->load(ConfigService::getService()->nottawaFolderFilePath() + "/preferences.json", seedFromSession);
  apply();
  setupAudio();
  prefs->subscribe([](const std::string& id) {
    if (id.rfind("output.", 0) == 0 || id.rfind("audio.", 0) == 0 || id.rfind("midi.", 0) == 0 || id == "graph.clearOnDisconnect")
      apply();
  });
}

}  // namespace EnginePreferences

#endif /* EnginePreferences_hpp */

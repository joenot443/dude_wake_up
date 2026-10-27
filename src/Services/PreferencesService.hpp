//
//  PreferencesService.hpp
//  dude_wake_up
//
//  App preferences shared by the macOS and Windows apps (docs/SETTINGS.md).
//
//  Each setting is declared once here (id, section, label, help, type,
//  range or choices, default, restart, platforms) and both apps build their
//  settings screens from that list through the bridge (ntw_get_settings).
//  Values persist in <nottawa folder>/preferences.json, apart from any
//  workspace: opening a workspace doesn't change them. Engine services that
//  use one follow it through subscribe().
//
//  Header-only (no project-file entries). Values are behind a mutex: any
//  thread may read them. set() and reset() from the engine thread (the
//  bridge hops there), where subscribers run.
//

#ifndef PreferencesService_hpp
#define PreferencesService_hpp

#include "json.hpp"

#include <algorithm>
#include <atomic>
#include <chrono>
#include <cmath>
#include <cstdio>
#include <filesystem>
#include <fstream>
#include <functional>
#include <map>
#include <mutex>
#include <string>
#include <utility>
#include <vector>

using json = nlohmann::json;

enum SettingType {
  SettingType_Bool = 0,
  SettingType_Int = 1,
  SettingType_Float = 2,
  SettingType_Choice = 3,
  SettingType_String = 4,
  SettingType_Path = 5,
  SettingType_Action = 6,  // a button (no value): the app performs it by id
};

enum SettingPlatform {
  SettingPlatform_Mac = 1,
  SettingPlatform_Windows = 2,
  SettingPlatform_Both = 3,
};

struct SettingDefinition {
  std::string id, section, label, help, unit;
  std::string group;      // optional subheading within the section
  std::string keywords;   // extra search terms, space-separated
  std::string dependsOn;  // "id" (a Bool: disabled while it's off) or "id=choice" (a Choice: disabled unless it's that)
  SettingType type = SettingType_Bool;
  double min = 0, max = 1, step = 0;
  int decimals = 0;       // numbers: digits shown after the point
  std::vector<std::pair<std::string, std::string>> choices;  // id, label
  json defaultValue;
  int restart = 0;        // SettingPlatform bits where a change needs a restart
  int platforms = SettingPlatform_Both;
};

class PreferencesService {
public:
  static PreferencesService* getService() {
    static PreferencesService service;
    return &service;
  }

  // The settings, in display order (sections in order of first appearance).
  const std::vector<SettingDefinition>& definitions() const { return definitions_; }

  const SettingDefinition* definition(const std::string& id) const {
    for (const auto& d : definitions_) if (d.id == id) return &d;
    return nullptr;
  }

  // Values (the default when unset or of the wrong type).
  json value(const std::string& id) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return valueLocked(id);
  }
  double number(const std::string& id) const {
    const json v = value(id);
    if (v.is_boolean()) return v.get<bool>() ? 1.0 : 0.0;
    return v.is_number() ? v.get<double>() : 0.0;
  }
  bool boolean(const std::string& id) const { return number(id) != 0.0; }
  std::string string(const std::string& id) const {
    const json v = value(id);
    return v.is_string() ? v.get<std::string>() : std::string();
  }

  // Sets a value, kept to the setting's type, range or choices. Returns
  // whether it changed. Subscribers are told; the file is written soon.
  // The default isn't stored, so a later version can change a default for
  // everyone who never chose otherwise.
  bool set(const std::string& id, json v) {
    const SettingDefinition* d = definition(id);
    if (!d) return false;
    v = conform(*d, v);
    if (v.is_null()) return false;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (valueLocked(id) == v) return false;
      if (v == d->defaultValue) values_.erase(id);
      else values_[id] = v;
      dirty_ = true;
      dirtySince_ = std::chrono::steady_clock::now();
    }
    notify(id);
    return true;
  }

  // Back to defaults: one section, or everything (empty section).
  void reset(const std::string& section = "") {
    std::vector<std::string> changed;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      for (const auto& d : definitions_) {
        if (!section.empty() && d.section != section) continue;
        if (values_.contains(d.id)) { values_.erase(d.id); changed.push_back(d.id); }
      }
      if (!changed.empty()) { dirty_ = true; dirtySince_ = std::chrono::steady_clock::now(); }
    }
    for (const auto& id : changed) notify(id);
  }

  // App state that isn't a setting (e.g. the audio input last used): kept
  // in preferences.json too, apart from any workspace, but not in the list.
  json state(const std::string& key) const {
    std::lock_guard<std::mutex> lock(mutex_);
    return state_.contains(key) ? state_.at(key) : json();
  }
  void setState(const std::string& key, const json& v) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (state_.contains(key) && state_[key] == v) return;
    state_[key] = v;
    dirty_ = true;
    dirtySince_ = std::chrono::steady_clock::now();
  }

  // Called with the id whenever a value changes (engine thread).
  using Observer = std::function<void(const std::string& id)>;
  void subscribe(Observer observer) { observers_.push_back(std::move(observer)); }

  // Reads the file; whether there was one. Without one (first run), values
  // stay defaults and loaded() false until seeded() (after the caller has
  // carried values over from where they used to live).
  bool load(const std::string& path) {
    path_ = path;
    bool found = false;
    {
      std::ifstream in(path);
      if (in) {
        try {
          json j = json::parse(in);
          if (j.is_object() && j.contains("values") && j["values"].is_object()) {
            std::lock_guard<std::mutex> lock(mutex_);
            for (auto& [id, v] : j["values"].items()) {
              const SettingDefinition* d = definition(id);
              if (!d) { values_[id] = v; continue; }  // kept for a newer version's settings
              const json c = conform(*d, v);
              if (!c.is_null() && c != d->defaultValue) values_[id] = c;
            }
            if (j.contains("state") && j["state"].is_object()) state_ = j["state"];
            found = true;
          }
        } catch (...) {}
      }
    }
    loaded_ = found;
    return found;
  }

  // First run: the carried-over values are set; writes the file.
  void seeded() {
    save();
    loaded_ = true;
  }

  // Whether the values are the user's: read from the file, or seeded.
  bool loaded() const { return loaded_.load(); }

  // Writes the file now.
  void save() {
    if (path_.empty()) return;
    json j;
    {
      std::lock_guard<std::mutex> lock(mutex_);
      j["values"] = values_;
      j["state"] = state_;
      dirty_ = false;
    }
    j["version"] = 1;
    std::error_code ec;
    std::filesystem::create_directories(std::filesystem::u8path(path_).parent_path(), ec);
    const std::string tmp = path_ + ".tmp";
    {
      std::ofstream out(tmp, std::ios::trunc);
      if (!out) return;
      out << j.dump(2);
    }
    std::remove(path_.c_str());
    std::rename(tmp.c_str(), path_.c_str());
  }

  // Engine loop, every frame: writes changes once they've settled (0.5 s).
  void flush() {
    {
      std::lock_guard<std::mutex> lock(mutex_);
      if (!dirty_ || std::chrono::steady_clock::now() - dirtySince_ < std::chrono::milliseconds(500)) return;
    }
    save();
  }

private:
  PreferencesService() { declare(); }

  void notify(const std::string& id) {
    for (const auto& o : observers_) o(id);
  }

  json valueLocked(const std::string& id) const {
    if (values_.contains(id)) return values_.at(id);
    const SettingDefinition* d = definition(id);
    return d ? d->defaultValue : json();
  }

  // A value made to fit the setting, or null if it can't be.
  static json conform(const SettingDefinition& d, const json& v) {
    switch (d.type) {
      case SettingType_Bool:
        if (v.is_boolean()) return v;
        if (v.is_number()) return v.get<double>() != 0.0;
        return json();
      case SettingType_Int:
      case SettingType_Float: {
        if (!v.is_number() && !v.is_boolean()) return json();
        double x = v.is_boolean() ? (v.get<bool>() ? 1.0 : 0.0) : v.get<double>();
        if (!std::isfinite(x)) return json();
        x = std::clamp(x, d.min, d.max);
        if (d.type == SettingType_Int) return static_cast<int64_t>(std::llround(x));
        return x;
      }
      case SettingType_Choice: {
        if (!v.is_string()) return json();
        for (const auto& c : d.choices) if (c.first == v.get<std::string>()) return v;
        return json();
      }
      case SettingType_String:
      case SettingType_Path:
        return v.is_string() ? v : json();
      case SettingType_Action:
        return json();
    }
    return json();
  }

  SettingDefinition& add(SettingDefinition d) {
    definitions_.push_back(std::move(d));
    return definitions_.back();
  }

  // Every setting, in display order.
  void declare() {
    using C = std::vector<std::pair<std::string, std::string>>;
    auto base = [](std::string id, std::string section, std::string label, std::string help, SettingType type) {
      SettingDefinition d;
      d.id = id; d.section = section; d.label = label; d.help = help; d.type = type;
      return d;
    };
    auto addBool = [&](std::string id, std::string section, std::string label, std::string help, bool def) -> SettingDefinition& {
      SettingDefinition d = base(id, section, label, help, SettingType_Bool);
      d.defaultValue = def;
      return add(d);
    };
    auto addNumber = [&](std::string id, std::string section, std::string label, std::string help, SettingType type,
                         double min, double max, double step, int decimals, std::string unit, double def) -> SettingDefinition& {
      SettingDefinition d = base(id, section, label, help, type);
      d.min = min; d.max = max; d.step = step; d.decimals = decimals; d.unit = unit;
      d.defaultValue = type == SettingType_Int ? json(static_cast<int64_t>(def)) : json(def);
      return add(d);
    };
    auto addChoice = [&](std::string id, std::string section, std::string label, std::string help, C choices,
                         std::string def) -> SettingDefinition& {
      SettingDefinition d = base(id, section, label, help, SettingType_Choice);
      d.choices = std::move(choices); d.defaultValue = def;
      return add(d);
    };
    auto addPath = [&](std::string id, std::string section, std::string label, std::string help) -> SettingDefinition& {
      SettingDefinition d = base(id, section, label, help, SettingType_Path);
      d.defaultValue = "";
      return add(d);
    };
    auto addAction = [&](std::string id, std::string section, std::string label, std::string help) -> SettingDefinition& {
      return add(base(id, section, label, help, SettingType_Action));
    };

    // Appearance
    addChoice("appearance.theme", "Appearance", "Theme", "Colours for the app. Default follows the system's light or dark mode.",
              {{"default", "Default"}, {"midnight", "Midnight"}, {"daylight", "Daylight"},
               {"ember", "Ember"}, {"arctic", "Arctic"}, {"forest", "Forest"}}, "default")
        .restart = SettingPlatform_Windows;
    addBool("display.nodeHints", "Appearance", "Show Node Hints", "Tips on the canvas, such as how to blend two nodes.", true)
        .keywords = "tips help canvas";

    // Output
    addChoice("output.resolution", "Output", "Resolution", "The size every source and effect renders at.",
              {{"240p", "240p (426 x 240)"}, {"360p", "360p (640 x 360)"}, {"480p", "480p (854 x 480)"},
               {"720p", "720p (1280 x 720)"}, {"1080p", "1080p (1920 x 1080)"}, {"1440p", "1440p (2560 x 1440)"},
               {"4k", "4K (3840 x 2160)"}, {"custom", "Custom"}}, "720p")
        .keywords = "size quality 4k hd custom";
    auto& width = addNumber("output.customWidth", "Output", "Width", "The custom resolution's width.",
                            SettingType_Int, 64, 7680, 1, 0, "px", 1920);
    width.dependsOn = "output.resolution=custom"; width.keywords = "custom resolution size";
    auto& height = addNumber("output.customHeight", "Output", "Height", "The custom resolution's height.",
                             SettingType_Int, 64, 4320, 1, 0, "px", 1080);
    height.dependsOn = "output.resolution=custom"; height.keywords = "custom resolution size";
    addChoice("output.frameRate", "Output", "Frame Rate",
              "How often a frame is rendered. Lower uses less power and GPU; higher is smoother on fast displays.",
              {{"24", "24 fps"}, {"30", "30 fps"}, {"60", "60 fps"}, {"120", "120 fps"}}, "60")
        .keywords = "fps performance smooth";
    auto& advance = addBool("output.autoAdvance", "Output", "Advance Output Window Automatically",
                            "When the node an output window shows is deleted, show the next one instead of closing.", true);
    advance.group = "Output Window";
    auto& fade = addNumber("output.advanceFade", "Output", "Advance Fade", "Crossfade when the output window moves to another node.",
                           SettingType_Float, 0.0, 2.0, 0.1, 1, "s", 0.3);
    fade.group = "Output Window"; fade.dependsOn = "output.autoAdvance";
    auto& stretch = addBool("output.stretchPreviews", "Output", "Stretch Preview Windows",
                            "Fill preview windows, ignoring the output's aspect ratio.", false);
    stretch.group = "Output Window"; stretch.keywords = "aspect fill";

    // Audio
    addBool("audio.startOnLaunch", "Audio", "Start Audio on Launch",
            "Turn audio analysis on when Nottawa starts, with the input used last.", false)
        .keywords = "input analysis microphone automatic";
    addChoice("audio.tempoRange", "Audio", "Tempo Range",
              "Tempos outside the range show doubled or halved. Choose 90-180 to see drum & bass as 174 rather than 87.",
              {{"auto", "Automatic (60-200)"}, {"60-120", "60-120 BPM"}, {"80-160", "80-160 BPM"},
               {"90-180", "90-180 BPM"}, {"100-200", "100-200 BPM"}}, "auto")
        .keywords = "bpm octave half double drum bass";
    auto offset = [&](std::string id, std::string label, std::string help) {
      auto& d = addNumber(id, "Audio", label, help, SettingType_Float, -250.0, 250.0, 1.0, 0, "ms", 0.0);
      d.group = "Beat Timing";
      d.keywords = "latency delay sync nudge offset";
    };
    offset("audio.beatOffsetFile", "Audio Files",
           "Moves the beat pulse earlier (+) or later (-) to line up with what you hear. Automatic tempo only.");
    offset("audio.beatOffsetInput", "Inputs", "Microphones and audio interfaces.");
    offset("audio.beatOffsetSystem", "System Audio", "Audio playing on this computer.");

    // Performance
    addBool("performance.frameCache", "Performance", "Cache Video Clips",
            "Decode clips into memory so looping, scrubbing and reverse play are instant. Applies to clips as they load.", true)
        .keywords = "video file ram memory scrub reverse frame";
    addChoice("performance.frameCacheBudget", "Performance", "Cache Memory",
              "The most memory clip caches use. Automatic suits most computers.",
              {{"auto", "Automatic"}, {"1024", "1 GB"}, {"2048", "2 GB"}, {"4096", "4 GB"},
               {"8192", "8 GB"}, {"16384", "16 GB"}}, "auto")
        .dependsOn = "performance.frameCache";

    // Graph
    addBool("graph.clearOnDisconnect", "Graph", "Clear Output on Disconnect",
            "When an effect loses its input, show nothing rather than its last frame.", true);

    // Browser
    addBool("browser.livePreview", "Browser", "Live Previews", "Animate the previews of sources and effects in the browser.", true)
        .keywords = "thumbnails animate";

    // MIDI
    addBool("midi.smoothing", "MIDI", "Smooth MIDI Controls", "Glide parameters to new MIDI values instead of jumping.", false)
        .keywords = "controller glide";
    addNumber("midi.smoothingMs", "MIDI", "Smoothing Time", "How long a glide to a new MIDI value takes.",
              SettingType_Float, 0.0, 4000.0, 10.0, 0, "ms", 80.0)
        .dependsOn = "midi.smoothing";

    // Screenshots
    addChoice("screenshots.format", "Screenshots", "Format", "The file type screenshots are saved as.",
              {{"png", "PNG"}, {"jpeg", "JPEG"}}, "png");
    // Each app keeps the folder itself (the Mac's is a sandbox bookmark) and supplies the chooser.
    addPath("screenshots.folder", "Screenshots", "Folder", "Where screenshots are saved.");

    // Updates (outside the Mac App Store and Microsoft Store builds)
    addBool("updates.check", "Updates", "Check for Updates", "Look for a newer version once a day.", true)
        .platforms = SettingPlatform_Windows;

    // Advanced
    addAction("advanced.openLogs", "Advanced", "Open Logs Folder", "The engine log and any crash or hang reports.")
        .keywords = "crash hang report debug";
    addAction("advanced.openData", "Advanced", "Open Data Folder", "Settings, the session, workspaces and the library.");
    addAction("advanced.resetAll", "Advanced", "Reset All Settings", "Every setting back to its default. Workspaces aren't affected.")
        .keywords = "defaults restore";
  }

  std::vector<SettingDefinition> definitions_;
  std::vector<Observer> observers_;
  mutable std::mutex mutex_;
  json values_ = json::object();
  json state_ = json::object();
  bool dirty_ = false;
  std::atomic<bool> loaded_{false};
  std::chrono::steady_clock::time_point dirtySince_;
  std::string path_;
};

#endif /* PreferencesService_hpp */

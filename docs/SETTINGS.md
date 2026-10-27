# Settings (spec, draft)

A richer settings screen on macOS (NottawaApp) and Windows (app/nottawa_app),
built in lockstep. Status: **draft for review** (Windows session wrote it; Mac
session reviews; Joe decides the open questions at the end).

## Where settings live

Three kinds, and today they're mixed up:

| Kind | Examples | Lives in |
|---|---|---|
| **App preferences, shared**: the same on both platforms, apply whatever workspace is open | node hints, output-window auto-advance, clear on disconnect, MIDI smoothing, screenshot format | **engine**, new `preferences.json` in the app data folder |
| **Workspace settings**: part of a session / saved workspace | the graph, layout, parameters (and today resolution and MIDI smoothing) | engine `config.json` / workspace files (as now) |
| **Platform UI state**: meaningless on the other platform or not a "setting" | window/sidebar layout, browser tab, stage strip order, onboarding flags, recents, voter id, share author, Mac screenshot-folder bookmark | UserDefaults / QSettings (as now) |

The engine's `config.json` is the session: anything in it changes when a
workspace is opened. Resolution and MIDI smoothing are stored there today, so
opening a workspace silently changes them (see open questions).

## Engine side (Windows session owns)

`PreferencesService` (src/Services): a registry of settings, each with

- `id` (stable, dotted: `display.nodeHints`), `section`, `label`, `help` (one line)
- `type`: bool, int, float, choice (string id from a list), string, path
- `min` / `max` / `step` (numbers), `choices` (id + label), `unit` ("ms", "s", "%")
- `default`, `restart` (needs a restart to take effect), `platforms` (both / mac / windows)

values persisted in `<app data>/preferences.json` (written debounced, like the
config), and engine code reading them by id. Engine services that already own a
setting (MIDI smoothing, clear on disconnect) read it from preferences.

Bridge (C, in nottawa_bridge.h):

```c
typedef struct {
  const char* id; const char* section; const char* label; const char* help;
  int type;                    // NTWSettingType
  double min, max, step;       // numbers
  const char* unit;
  const char** choiceIds; const char** choiceLabels; int choiceCount;
  int restart;                 // 1: takes effect after a restart
} NTWSettingInfo;

NTWSettingInfo* ntw_get_settings(int* outCount);   // in display order
void   ntw_free_settings(NTWSettingInfo*, int count);
double ntw_get_setting_number(const char* id);      // bool/int/float
void   ntw_set_setting_number(const char* id, double value);
char*  ntw_get_setting_string(const char* id);      // choice/string/path (free with ntw_free_string)
void   ntw_set_setting_string(const char* id, const char* value);
void   ntw_reset_settings(const char* section);     // NULL: all
typedef void (*NTWSettingsChangedCallback)(const char* id);
void   ntw_register_settings_changed_callback(NTWSettingsChangedCallback);
```

Platform-only entries (e.g. Windows "check for updates") are still declared
in the engine with `platforms`, so one list describes the whole screen; each
app skips what isn't for it and handles its own (path pickers, theme apply).

## UI (each session owns its app)

- Built from `ntw_get_settings()`: a section list (sidebar) and the section's
  rows. Row widgets by type: toggle, slider + value (with unit), stepper,
  picker, path chooser. Help text under each row.
- Changes apply live; `restart` rows say so. Per-section "Reset to Defaults".
- A search field filtering rows by label/help (cheap with a schema).
- Platform-only rows and actions added by each app where the schema says so
  (Mac: screenshot folder bookmark; Windows: update check, logs folder).
- First launch after this ships: each app copies its old UserDefaults /
  QSettings values for moved settings into the engine once (migration).

## Settings

Existing (both platforms unless noted), proposed home:

| id | type | default | today | proposed |
|---|---|---|---|---|
| display.nodeHints | bool | true | Mac UserDefaults, Win QSettings | shared pref |
| output.autoAdvance | bool | true | both, UI-side | shared pref |
| output.advanceFade | float s 0-2 | 0.3 | both, UI-side (Mac reads 0 when unset: bug) | shared pref |
| graph.clearOnDisconnect | bool | true | Mac UserDefaults + bridge, Win QSettings | shared pref (engine already uses it) |
| midi.smoothing | bool | false | engine, per workspace | shared pref? (open question) |
| midi.smoothingMs | float ms 0-4000 | 80 | engine, per workspace | shared pref? |
| output.resolution | choice | 720p | engine, per workspace | open question |
| screenshots.format | choice png/jpeg | png | both, UI-side | shared pref |
| screenshots.folder | path | (ask) | Mac bookmark, Win path | platform UI state |
| browser.livePreview | bool | true | both, UI-side (Mac: sidebar only) | shared pref |
| appearance.theme | choice | Default | both; Mac doesn't persist it (bug), Win needs restart | open question |
| updates.check | bool | true | Windows only | pref, platforms: windows |

New settings: **proposed, Joe to pick** (each is an engine change that then
appears in both apps):

- **Audio**: default input on launch (last used / microphone / system audio /
  sample track); start audio on launch; BPM range (60-200 default, or prefer
  "78-156" / "90-180" so drum & bass shows 174, see docs/BPM.md); beat pulse
  offset (latency compensation, ms, replacing ad-hoc nudge); tap-tempo taps
  needed in Auto (3).
- **Output**: frame-rate cap (30/60/display); output window display and
  fullscreen on launch; custom resolution (width x height, already in the
  engine).
- **Performance**: preview quality (thumbnail size / update rate); live
  previews in the browser; video frame cache on/off and RAM budget (Windows
  has it per file today); stage-mode GPU priority.
- **MIDI**: smoothing (above), device list and assignments manager (bridge
  queries exist; make it a proper page).
- **Advanced**: open logs folder, open data folder, reset all settings, crash
  and hang reports info.

## Plan

Branch `settings`. Each step: Windows pushes engine/bridge, messages the SHA;
both UIs build on it; both builds green before the next.

1. Engine: `PreferencesService`, schema, bridge API, persistence, change
   callback; existing shared prefs registered (no UI change yet). [Windows]
2. Both apps: schema-driven settings screen showing the step-1 settings,
   migration of old values, old UI-side reads switched to the engine. [each]
3. The chosen new settings, a few at a time: engine [Windows], then each UI
   only if it needs a platform hook. [both]

## Open questions (Joe)

1. **Resolution and MIDI smoothing**: app preferences (same whatever workspace
   is open), or stay saved with each workspace? Suggest: preferences. They
   describe the setup (projector, MIDI controller), not the piece.
2. **Theme**: shared (one choice for both) or per platform? Windows needs a
   restart to apply; Mac doesn't persist it at all today.
3. **Which new settings** from the list above (or others) for the first round.
4. **Layout**: a sidebar of sections (like macOS System Settings), or tabs?

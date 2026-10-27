# Settings

A richer settings screen on macOS (NottawaApp) and Windows (app/nottawa_app),
built in lockstep. Status: steps 1 (engine) and 2 (both settings screens)
done; step 3 (new settings) next.

## Where settings live

Three kinds, and today they're mixed up:

| Kind | Examples | Lives in |
|---|---|---|
| **App preferences, shared**: the same on both platforms, apply whatever workspace is open | node hints, output-window auto-advance, clear on disconnect, MIDI smoothing, screenshot format | **engine**, new `preferences.json` in the app data folder |
| **Workspace settings**: part of a session / saved workspace | the graph, layout, parameters (and today resolution and MIDI smoothing) | engine `config.json` / workspace files (as now) |
| **Platform UI state**: meaningless on the other platform or not a "setting" | window/sidebar layout, browser tab, stage strip order, onboarding flags, recents, voter id, share author, Mac screenshot-folder bookmark | UserDefaults / QSettings (as now) |

The engine's `config.json` is the session: anything in it changes when a
workspace is opened. Resolution and MIDI smoothing are stored there today, so
opening a workspace silently changed them. They're app preferences now
(decision 1): `loadConfigFile` re-applies the preferences after loading.

## Engine side (Windows session owns)

`PreferencesService` (src/Services/PreferencesService.hpp, header-only)
declares every setting once, in display order:

- `id` (stable, dotted: `display.nodeHints`), `section`, optional `group`
  (subheading), `label`, `help` (one line), `keywords` (extra search terms)
- `type`: bool, int, float, choice (string id from a list), string, path,
  action (a button with no value; the app performs it by id)
- numbers: `min` / `max` / `step`, `decimals`, `unit` ("ms", "s", "%")
- `default`, `dependsOn` (a bool's id: the row is greyed while it's off),
  `restart` (platform bits where a change needs a restart), `platforms`
  (1 macOS, 2 Windows, 3 both)

Values other than the default persist in `<nottawa folder>/preferences.json` (under
`appSupportFilePath()`; inside the sandbox container on macOS), written 0.5 s
after the last change, and on `ntw_save_default_config`. On first run (no
file) resolution and MIDI smoothing are seeded from the session.
`EnginePreferences.hpp` puts the engine's own (resolution, MIDI smoothing,
clear on disconnect) into effect. The legacy ImGui app never loads
preferences, so it neither applies nor writes them.

Values are behind a mutex: any thread may read. Sets go to the engine thread,
which runs the change callback for every change, including the app's own
sets (menus mirroring a setting can just follow it).

Bridge: the `MARK: - Settings` section of src/Bridge/nottawa_bridge.h is the
reference (`NTWSettingInfo`, `ntw_get_settings` / `ntw_free_settings`, get /
set number or string by id, `ntw_reset_settings(section)`,
`ntw_register_settings_changed_callback`). The old setters
(`ntw_set_resolution`, `ntw_set_midi_smoothing_*`,
`ntw_set_clear_on_disconnect`) set the preference.

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

Step 1's settings (all engine preferences now):

| id | section | type | default | notes |
|---|---|---|---|---|
| appearance.theme | Appearance | choice | default | default/midnight/daylight/ember/arctic/forest; restart on Windows |
| display.nodeHints | Appearance | bool | true | |
| output.resolution | Output | choice | 720p | 240p ... 4k; was per workspace |
| output.autoAdvance | Output | bool | true | group Output Window |
| output.advanceFade | Output | float s 0-2 | 0.3 | dependsOn autoAdvance |
| output.stretchPreviews | Output | bool | false | Mac didn't persist it |
| graph.clearOnDisconnect | Graph | bool | true | |
| browser.livePreview | Browser | bool | true | |
| midi.smoothing | MIDI | bool | false | was per workspace |
| midi.smoothingMs | MIDI | float ms 0-4000 | 80 | dependsOn smoothing |
| screenshots.format | Screenshots | choice | png | png/jpeg |
| screenshots.folder | Screenshots | path | "" | listed only: each app keeps its own (Mac bookmark) and chooser |
| updates.check | Updates | bool | true | Windows only |
| advanced.openLogs / openData / resetAll | Advanced | action | | the app performs them |

Step 3, batch 1 (Audio and Output):

| id | section | type | default | notes |
|---|---|---|---|---|
| output.resolution | Output | choice | 720p | gains `custom` |
| output.customWidth / customHeight | Output | int px | 1920 / 1080 | dependsOn `output.resolution=custom` |
| output.frameRate | Output | choice | 60 | 24/30/60/120: the engine loop's rate |
| output.reopenWindows | Output | bool | true | group Output Window: each app records its open output windows at quit (node, display, frame, full screen; platform state) and reopens them at launch, on the same display if connected |
| audio.startOnLaunch | Audio | bool | false | starts the input used last |
| audio.tempoRange | Audio | choice | auto | auto, 60-120, 80-160, 90-180, 100-200: Auto's tempo shown folded into it by octaves (beat count and phase with it) |
| audio.beatOffsetFile / Input / System | Audio | float ms -250..250 | 0 / 0 / +35 (Win), +45 (Mac) | group Beat Timing; + shows the pulse earlier; Auto only; taps are moved back by it before hinting |

The audio input is remembered app-wide (preferences.json `state`, not a
setting): selecting one records it, it's selected (not started) at launch,
and opening a workspace no longer switches it.

`dependsOn` is `id` (a Bool that's on) or `id=choice` (a Choice set to
that). Each app picks widgets by type; Windows uses a spin box for Int.

New settings, step 3 (Joe chose all four groups; each is an engine change
that then appears in both apps):

- **Audio**: default input on launch (last used / microphone / system audio /
  sample track); start audio on launch; BPM range (60-200 default, or prefer
  "78-156" / "90-180" so drum & bass shows 174, see docs/BPM.md); beat pulse
  offset (latency compensation, ms, replacing ad-hoc nudge); tap-tempo taps
  needed in Auto (3). Measured on the Mac: the shown beat is +15-20 ms late
  from a File source but +61-65 ms from System Audio (tap capture plus output
  buffering), so the offset should be per input kind (or File / System
  defaults differing by about 45 ms), not one number.
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

## Decisions (Joe)

1. Resolution and MIDI smoothing are **app preferences**: opening a
   workspace no longer changes them. (Existing users: seeded once from the
   current session.)
2. The theme is **shared**: `appearance.theme`, choices `default`,
   `midnight`, `daylight`, `ember`, `arctic`, `forest`. ("default" follows the
   system appearance: Midnight dark, Daylight light. macOS adds explicit
   Midnight and Daylight, which Windows already has.)
3. First round of new settings: **all four groups**: Audio, Output,
   Performance, MIDI & Advanced.
4. Layout: a **sidebar of sections** with search.

## Platform notes

- **Windows** (app/nottawa_app/Prefs.*): menus and the theme need values
  before the engine starts, so every value is copied to QSettings
  (`prefs/<id>`) on each change and read until then. Migration on first
  start copies the old QSettings keys that were ever written; an existing
  install keeps Clear on Disconnect off (its old default) unless it was on.
  The theme applies after a restart (banner in Settings).
- **macOS** (NottawaApp/Engine/SettingsStore.swift): migration flag
  `nottawa.settingsMigrated`; screenshots.folder stays a sandbox bookmark.
  Opening a welcome template strand sets the resolution it was made at,
  which is now a lasting preference change (with a toast).

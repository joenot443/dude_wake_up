# Nottawa Windows Port — Executable Implementation Plan

**Document version:** 1.0 (final synthesis after adversarial review)
**Target platform:** Windows 10 22H2 x64 and Windows 11 23H2 x64
**Effort baseline:** 62 working days likely (single engineer, critical path)
**Wall-clock with two engineers:** ~40 days
**Wall-clock with three engineers:** ~34 days

---

## 1. Executive Summary

### Problem

Nottawa is a macOS-only OpenFrameworks + Dear ImGui audio-reactive VJ application. It currently ships as a signed, notarized macOS `.dmg` via Sparkle. The goal is a shipping Windows `.exe` installer that (a) reaches feature parity with the macOS build for the VJ workflow (render pipeline, audio capture including desktop loopback, MP4 recording, deep-link URL scheme, auto-update, MIDI, webcam), and (b) can be maintained on the same release cadence as macOS from a shared codebase.

### Scope in one paragraph

Replace six macOS-specific subsystems (CATap system audio, AVFoundation recorder, IOSurface texture sharing to SwiftUI, Sparkle updater, security-scoped bookmarks, Apple-Event URL scheme) with Windows-native equivalents (WASAPI loopback, libav muxing, DXGI-shared-handle or omission entirely, WinSparkle, no-op passthrough, HKCU registry + named-pipe single-instance forwarding). Land a mechanical portability sweep first so the macOS Xcode build stays green through the port. Ship a signed installer via Inno Setup with WinSparkle wired into the existing appcast.

### Effort estimate (post-review)

| Metric | Days |
|---|---|
| Best case (single engineer, no gotchas) | 28 |
| Likely (single engineer) | **62** |
| Worst case (single engineer, cert delay + rehab overruns) | 90 |
| Likely (two engineers, one on lanes B+C) | 40 wall-clock |
| Likely (three engineers) | 34 wall-clock |

The 62-day baseline is the max of three adversarial reviews' revised estimates (44 / 46 / 62). It absorbs UTF-8 path plumbing, COM apartment discipline, sRGB pipeline audit, crash-reporter integration, and first-time-Windows-port learning-curve tax.

### Top 3 risks (from merged risk register)

1. **EV code-signing certificate issuance takes 3–10 business days** and gates the first signed installer. Order on day 1 of Phase 0.
2. **UTF-8 / UTF-16 path handling errors** silently break users with non-ASCII usernames, install paths, or strand slugs (affects registry, ffmpeg I/O, dr_libs, single-instance pipe, argv). Must be a Phase 0 concern via `src/Util/Utf8Path.hpp`.
3. **A/V sync and thread-safety in the libav recorder** — three defects already identified in the sketch (undeclared drainEncoder_, dropped audio tail, double onRecordingComplete), plus a global lock ordering problem between `recordingMutex` and `writeMutex` that will deadlock stop-recording unless designed carefully.

### Go/no-go decisions (see §10 for full list)

- **GATE-A** (Phase 1 exit): `bin/Nottawa.exe` launches to an empty ImGui window on a clean Win 11 VM. If not after 2 attempts → pivot to CMake build (adds 3–5d).
- **GATE-D** (Phase 4 mid): libav AAC + swr_convert achieves <30 ms A/V sync. If not → pivot to Media Foundation MFCreateSinkWriter (adds 4–6d).
- **GATE-H** (Phase 7 start): EV cert issued. If not after 10 business days → ship RC1 with OV cert and SmartScreen warning; re-sign with EV when it arrives.

### Non-negotiable outcomes at ship

- Signed Nottawa-Setup-1.0.0.exe installs on Win 10 22H2 and Win 11 23H2 without SmartScreen warning
- `nottawa://` URL scheme launches app cold and warm-forwards to running instance
- All three AudioSources (Mic, File, System Loopback) drive audio-reactive visuals
- Record → 15 s clip → MP4 plays in VLC and Windows Media Player with A/V drift < 30 ms
- WinSparkle upgrades installed app in place with user data preserved at `%APPDATA%\Nottawa`
- Crash reports flow to Sentry with symbolication from the PDB symbol server
- macOS Xcode build remains green from every commit in the port branch

---

## 2. Architecture Overview

### Platform seams

```
                        ┌──────────────────────────────────────┐
                        │   src/Apps/MainApp  (cross-platform) │
                        └────┬─────────────────┬───────────────┘
                             │                 │
   ┌─────────────────────────┼─────────────────┼─────────────────────────┐
   │  cross-platform core (src/Shading, src/NodeLayout, src/Views,      │
   │  src/Oscillation, src/Models, src/Services non-platform, ImGui,     │
   │  Gist, BTrack, KissFFT, dr_libs, stb_vorbis, nlohmann/json,         │
   │  ofxImGui, ofxMidi, ofxFastFboReader)                               │
   └─────────────────────────┬─────────────────┬─────────────────────────┘
                             │                 │
       ┌─────────────────────┴─────┐   ┌───────┴──────────────────────┐
       │  macOS platform surface   │   │  Windows platform surface    │
       ├───────────────────────────┤   ├──────────────────────────────┤
       │ SystemAudioSource_mac.mm  │   │ SystemAudioSource_win.cpp    │
       │   (CoreAudio CATap)       │   │   (WASAPI global loopback)   │
       │                           │   │                              │
       │ AVFRecorderWrapper.mm +   │   │ LibavRecorder.cpp            │
       │  AVFRecorder.h/.m         │   │   (ffmpeg 7.1 LGPL + NVENC/  │
       │  (AVAssetWriter, VT)      │   │    QSV/AMF/openh264 ladder)  │
       │                           │   │                              │
       │ IOSurfaceTextureManager   │   │ (excluded — no SwiftUI host  │
       │   (Metal/CGL interop      │   │  on Windows; header no-op    │
       │    for SwiftUI target)    │   │  stubs behind __APPLE__)     │
       │                           │   │                              │
       │ SparkleUpdater.mm         │   │ SparkleUpdater_win.cpp       │
       │   (Sparkle framework)     │   │   (WinSparkle 0.8+, shared   │
       │                           │   │    ed25519 key with macOS)   │
       │                           │   │                              │
       │ URLSchemeHandler.mm       │   │ URLSchemeHandler_win.cpp     │
       │   (Apple Events)          │   │   (HKCU registry + named     │
       │                           │   │    pipe + single-instance    │
       │                           │   │    mutex; UTF-16 native)     │
       │                           │   │                              │
       │ BookmarkService.cpp       │   │ BookmarkService_win passthru │
       │   (CF security-scoped)    │   │   (no sandbox on Windows)    │
       │                           │   │                              │
       │ OutputWindow.mm           │   │ OutputWindow.cpp +           │
       │  + OutputWindow_menu_mac  │   │   OutputWindow_menu_win.cpp  │
       │   (NSMenu Record)         │   │   (ofxImGui menubar; Ctrl+R) │
       │                           │   │                              │
       │ RevealInFinder (system)   │   │ RevealInFileManager          │
       │                           │   │   (ShellExecuteW)            │
       │                           │   │                              │
       │ (system Sentry .dylib)    │   │ sentry.dll + crashpad.exe    │
       └───────────────────────────┘   └──────────────────────────────┘

Foundation utilities added specifically for Windows correctness:
  src/Util/Utf8Path.hpp   — UTF-8 std::string <-> UTF-16 std::wstring
  src/Util/ComApartment.hpp — RAII COM init (MTA on capture threads)
  src/Util/PortabilityCheck.hpp — NOMINMAX canary TU
```

### Key design invariants

- **String policy:** All in-memory paths are UTF-8 `std::string`. All Win32 calls use the `W` variant (`RegSetValueExW`, `ShellExecuteW`, `CreateMutexW`, `CreateNamedPipeW`, `CreateFileW`, `_wfopen`). Conversion happens at the API boundary via `src/Util/Utf8Path.hpp`. Grep guard blocks new `*A(` calls in `_win.cpp` files.
- **COM apartment policy:** Main thread stays uninitialized (or STA if WinSparkle demands it — verified in Phase 1). WASAPI capture thread enters `COINIT_MULTITHREADED` at thread entry, exits at join. `ShellExecuteW` is called from a scoped STA on a helper thread. All apartment transitions RAII-wrapped in `src/Util/ComApartment.hpp`.
- **Lock ordering:** `LibavRecorder::writeMutex` MAY NOT be held while calling into `AudioSource`. `AudioSource::recordingMutex` MAY NOT be held while invoking the user callback (copy the `std::function` under lock, release, then invoke). `writeMutex` guards ALL `avformat_*` calls — audio and video packets cannot race in `av_interleaved_write_frame`.
- **sRGB / color pipeline:** Explicit policy chosen in Phase 2 M2.0 audit. `glEnable(GL_FRAMEBUFFER_SRGB)` state logged at startup. FBO internal formats explicit (`GL_RGBA8` — no compressed fallbacks).
- **Video PTS:** Wall-clock derived (`av_rescale_q(std::chrono::steady_clock::now() - recordingStartTime, ...)`), never a naive frame counter — shader-load stalls in the main draw loop make frame counting a source of A/V drift.

---

## 3. Sequenced Phases

Seven phases total. Effort figures reflect merged review findings; the wall-clock summary tracks the critical path only. Phase 0 milestones renumbered to absorb reviewer-mandated additions (M0.8, M0.9, M0.10, M0.11).

### Phase summary table

| Phase | Name | Duration (likely) | Critical path? | Parallel-lane eligible? |
|---|---|---:|:---:|:---:|
| 0 | Mechanical portability + foundations | 5d | yes | partial (M0.10 test-rehab spike parallelizable) |
| 1 | Empty shell + preview-install pipeline | 5d | yes | no |
| 2 | Rendering, sRGB, shader portability | 5d | yes | no |
| 3 | Audio capture (WASAPI) | 6d | yes | no |
| 4 | Recording (libav) | 8d | yes | no (audio tests pull forward) |
| 5 | Ancillary services | 8d | yes on single-eng path | yes with 2nd engineer |
| 6 | Testing & CI | 13d | no | yes (parallel lane C) |
| 6.5 | Production observability (Sentry, logs, diagnostics) | 3d | yes | no |
| 7 | Distribution & release | 9d | yes (blocked by EV cert wallclock) | no |

**Totals:** critical path (single engineer) = 62d likely. Two-engineer parallel plan detailed in §7.

---

### Phase 0 — Mechanical portability sweep + foundations

**Goal.** Land every code-only change that unblocks a Windows compiler while remaining a no-op on macOS. Introduce the UTF-8/UTF-16 policy, NOMINMAX canary, and test-suite health check. Every change verified against the macOS Xcode build before commit.

**Entry criteria.** None — this is day 1.

**Subsystems touched.** foundation, testing-ci (M0.10 only).

**Ordered milestones.**

| ID | Description |
|---|---|
| M0.1 | Fix two case-mismatched includes in `src/Services/ShaderChainerService.cpp:10` (`AsciiShader.hpp` → `ASCIIShader.hpp`) and `:122` (`16BitShader.hpp` → `16bitShader.hpp`). Land Python case-mismatch scanner as pre-push hook. |
| M0.2 | Rewrite `src/Util/MSATimer.h` to use `std::chrono::steady_clock` unconditionally. Delete `mach/mach_time.h` include and `machMultiplier` member. |
| M0.3 | Gate 7 unconditional CoreFoundation / IOSurface include sites behind `#ifdef __APPLE__`: `src/main.cpp:5`, `src/Views/MainStageView.hpp:11`, `src/Views/MainStageView.cpp:10`, `src/Services/VideoSourceService.cpp:8`, `src/Engine/EngineLoop.cpp:29-30`, `src/Engine/IOSurfaceTextureManager.hpp:18`, `src/Engine/IOSurfaceTextureManager.cpp:15-17`. Provide benign `#else` stubs where the TU has live CF-calling code. |
| M0.4 | Fix uintptr_t/long truncation. `src/NodeLayout/NodeTypes.hpp:20` (`NullId` type → `uintptr_t`, `%d` → `%llu` at line 51). `src/NodeLayout/NodeLayoutView.cpp:349` (`long pinKey` → `uintptr_t`). Make `pinIdConnectionMap` / `pinIdNodeMap` key types explicit. |
| M0.5 | Verify `src/Util/httplib.h` matches upstream cpp-httplib v0.12.1 (guards ARE intact — re-audit confirmed). Re-vendor `src/Audio/dr_wav.h`, `dr_mp3.h`, `dr_flac.h` from mackron/dr_libs current tips. Verify `src/Audio/stb_vorbis.h` guards. |
| M0.6 | Delete `src/AsioTest.cpp` (dead second `main()`) and `src/Audio/test_bpm_detector.cpp` (references non-existent `AutoBpmDetector.hpp`). |
| M0.7 | Author `src/Util/PortabilityCheck.hpp` — NOMINMAX canary TU compiled only when `_MSC_VER` defined. Add reminder comments to `src/Models/Parameter.hpp` and `src/Oscillation/WaveformOscillator.hpp` about the `min`/`max` member collision. |
| **M0.8** (new, from Review 2) | Author `src/Util/Utf8Path.hpp` + `.cpp` with `utf8_to_wide(const std::string&) -> std::wstring` and `wide_to_utf8(const std::wstring&) -> std::string` using `MultiByteToWideChar(CP_UTF8, ...)`. Add pre-push scanner that flags any `*A(` Win32 call in files matching `*_win.cpp`. |
| **M0.9** (new, from Review 2) | Full third-party license audit: enumerate every vendored `.h/.cpp/.dll/.lib`, record license, confirm GPLv3 compatibility. Include `imgui-node-editor` (Apache-2.0), `portmidi`, Sentry SDK, `libcpr`, openh264. Cross-signed by legal if available. |
| **M0.10** (new, from Review 1) | 2-hour spike: verify `dude_wake_up_tests` gtest suite still builds and links on macOS trunk. If broken (high likelihood — service singleton forward-decls at `ShaderChainerServiceTests.hpp:29-42` cover ~13 services vs current tree of ~20+), budget 2–4d Phase 6 rehab; land the fix in Phase 6 M6.1 as originally planned. |
| **M0.11** (new, from Reviews 2+3) | Order EV code-signing certificate today (SSL.com eSigner / EV YubiKey / DigiCert EV). 3–10 business day wallclock. Use self-signed cert for internal test installs until it arrives. See §10 GATE-H for fallback. |

**Exit gate / Definition of Done.**
- macOS Xcode build of `dude_wake_up` green after every commit
- `grep -rn 'mach_absolute_time' src/` returns zero project hits
- `grep -rn '#include <CoreFoundation' src/` returns hits only inside `#ifdef __APPLE__` blocks
- Node-editor pin-hover test on macOS still resolves connections after the uintptr_t map-key change
- `src/Util/Utf8Path.hpp` compiles and round-trips a fixture string containing `日本語`, `Юрий`, `José` with byte-exact equality
- Pre-push scanner rejects a test commit containing `RegSetValueExA(` inside a `_win.cpp` file
- License audit spreadsheet committed to `docs/THIRDPARTY-LICENSES.md`
- EV cert order confirmation email received

**Duration: 5 days likely** (bumped from 3 to absorb M0.8 + M0.9 + M0.10 spike).

---

### Phase 1 — Empty shell + preview-install pipeline

**Goal.** Produce a Windows `.sln`/`.vcxproj` that compiles, links, launches `nottawa.exe` to an empty ImGui window using stubs. **New (from Review 3):** produce an EV-signed (or self-signed if EV not yet arrived) preview installer of that empty shell so the entire distribution pipeline is proven before Phase 7 loads it up with real content.

**Entry criteria.** Phase 0 exit gate met. EV cert order placed (M0.11).

**Subsystems touched.** build-system, foundation, ancillary-services (stubs only), distribution (preview-install only).

**Ordered milestones.**

| ID | Description |
|---|---|
| M1.1 | Provision Windows build box: VS 2022 Community + Desktop C++ + v143 toolset + Windows 10 SDK 10.0.22621. Install OF 0.12.1 vs release. Verify `emptyExample` builds and runs (GATE A prerequisite). |
| M1.2 | Trim `addons.make` (drop `ofxSyphon`, `ofxFFmpegRecorder`). Confirm `ofxImGui`, `ofxMidi` are checked out at commits matching macOS. |
| M1.3 | Write seven stub `.cpp` files: `SystemAudioSource_win.cpp`, `AVFRecorderWrapper_win.cpp`, `IOSurfaceTextureManager_win.cpp`, `SparkleUpdater_win.cpp`, `URLSchemeHandler_win.cpp`, `BookmarkService_win.cpp`, `OutputWindow_win.cpp`. Each returns safe defaults and logs a one-shot warning on first call. |
| M1.4 | Wrap `.mm` file bodies in `#ifdef __APPLE__` (belt-and-braces beyond project exclusion). |
| M1.5 | Run OF `projectGenerator` to produce `apps/myApps/Nottawa/{Nottawa.sln, Nottawa.vcxproj, Nottawa.vcxproj.filters}`. Commit generated files. |
| M1.6 | Hand-edit `.vcxproj`: add every vendored include path under `src/**` and `lib/**`; add preprocessor defines (`NOMINMAX`, `WIN32_LEAN_AND_MEAN`, `_USE_MATH_DEFINES`, `__WINDOWS_WASAPI__`, `TARGET_WIN32`, `GLM_FORCE_CTOR_INIT`, `GLM_ENABLE_EXPERIMENTAL`, `OF_NO_FMOD=1`, `USE_KISS_FFT`, `ofxAddons_ENABLE_IMGUI`, `JSON_DIAGNOSTICS=1`, `IMGUI_DEFINE_MATH_OPERATORS=1`); set `/std:c++17`. Exclude every `.mm`/`.m` and `src/Bridge/`, `src/Engine/` folder. Include the seven `_win.cpp` stubs. |
| **M1.6b** (new, from Review 2) | Set project `SubSystem=Windows` (not Console). Keep OF's `WinMain` path OR set `/ENTRY:mainCRTStartup`. Test via "no console appears when double-clicking `nottawa.exe`." |
| **M1.6c** (new, from Review 2) | Settle COM apartment policy. Author `src/Util/ComApartment.hpp` with RAII `ComApartmentMTA` / `ComApartmentSTA`. Verify against WinSparkle 0.8 docs whether main thread needs STA. Document invariant in `docs/WINDOWS_ARCHITECTURE.md`. Add debug-only `CoGetApartmentType` assertion at every COM entry. |
| M1.7 | Bundle LGPL ffmpeg shared build DLLs + OF-shipped runtime DLLs into `apps/myApps/Nottawa/bin/`. Add `avformat`/`avcodec`/`avutil`/`swscale`/`swresample` `.lib` to linker inputs. |
| M1.8 | Wire runtime data path: `TARGET_WIN32` branch in `main.cpp` that sets `ofSetDataPathRoot` to `bin/data/`. Prefer post-build `robocopy` over junction (junction fails across drives). |
| M1.9 | First-build GATE-A: MSBuild returns 0. Running `bin/Nottawa.exe` opens a GLFW window with an ImGui dock layout, stays alive 5+ seconds, no missing-DLL popup, no crash. Every stub logs its first-call warning once. |
| **M1.10** (new, from Review 3) | Assemble minimum `packaging/windows/build_release.ps1` end-to-end with a self-signed cert. Produces `Nottawa-Setup-0.0.0-preview.exe`. Install on clean Win 11 VM, launch, uninstall. Proves the distribution pipeline BEFORE Phase 7 loads it with real content. If EV cert has arrived (unlikely by day 10), re-sign with it. |
| M1.11 | Send message to `windows-gotchas` confirming NOMINMAX canary compiles, and to `of-windows-tooling` confirming `projectGenerator` output is stable. |

**Exit gate / DoD.**
- Fresh clone on a second Windows machine: run `projectGenerator` once, drop ffmpeg DLLs, build `Nottawa.sln` in `Release|x64`, `bin/Nottawa.exe` launches to an idle ImGui dock layout
- macOS Xcode build still green from the same commit
- `dumpbin /dependents nottawa.exe` lists every ffmpeg DLL resolved from `bin/`
- No console window appears when double-clicking `nottawa.exe`
- `Nottawa-Setup-0.0.0-preview.exe` installs, launches, uninstalls without residue
- `docs/WINDOWS_ARCHITECTURE.md` skeleton exists with COM apartment policy, string policy sections filled in

**Duration: 5 days likely** (bumped from 4 to absorb M1.6b + M1.6c + M1.10).

---

### Phase 2 — Rendering, sRGB, shader portability

**Goal.** Replace the four `sampler2DRect` fragment shaders, add DPI-aware + long-path-aware app manifest, retire the dead `shader_glitch` pair, defensively negotiate webcam pixel formats, and **settle the sRGB / framebuffer color-space policy** (new from Review 2). First frame of real content rendered on Windows across NVIDIA/AMD/Intel drivers with parity to macOS output.

**Entry criteria.** Phase 1 GATE-A passed. `nottawa.exe` opens a window.

**Subsystems touched.** rendering.

**Ordered milestones.**

| ID | Description |
|---|---|
| **M2.0** (new, from Review 2) | **sRGB pipeline audit.** Determine whether macOS is currently rendering sRGB-corrected. Document policy. On Windows: `glfwWindowHint(GLFW_SRGB_CAPABLE, GLFW_TRUE)` before window creation AND `glEnable(GL_FRAMEBUFFER_SRGB)` after context current if macOS is sRGB. Set FBO internal formats to `GL_SRGB8_ALPHA8` where blend chain assumes perceptual space. Log `GL_FRAMEBUFFER_SRGB` state at startup. Publish policy in `docs/WINDOWS_ARCHITECTURE.md`. This MUST precede Phase 6 golden baselines. |
| M2.1 | Edit `shaders/{empty,fractal,fuji,plasma}.frag`: `sampler2DRect` → `sampler2D`. Confirm macOS parity for all four. |
| M2.2 | Delete `shaders/shader_glitch.{frag,vert}` (dead, no references). |
| M2.3 | Add `app.manifest` at repo root: `dpiAwareness = "PerMonitorV2, PerMonitor"` + `<ws2:longPathAware>true</ws2:longPathAware>` + Win10/11 supportedOS GUIDs. Reference from `.vcxproj` Linker > Manifest File Input. Validate on 4K 200%-scaled monitor. |
| M2.4 | Add `GL_VENDOR`/`GL_RENDERER`/`GL_VERSION` log line in `MainApp::setup()` before any shader loads. Add temporary `glValidateProgram` diagnostic wrapping `ofShader::load` so first-load failures surface with driver info. Log `GL_IMPLEMENTATION_COLOR_READ_FORMAT` too. |
| M2.5 | Refactor `src/Video/WebcamSource.cpp:18`: try `OF_PIXELS_RGB` → `OF_PIXELS_RGBA` in order (do NOT fall through to `NATIVE` — see review finding). Move `active=true` to after successful `stream.setup()`. Log negotiated format. Test with integrated cam, USB UVC, OBS Virtual Camera on both Win 10 and Win 11. |
| M2.6 | Full-graph parity smoke: load a saved macOS strand (WebcamSource + Plasma + FileSource + Blend + Warp + Feedback + Blur + Glitch) on Windows. Frame timing within 10% of macOS baseline. Force `ofFbo::Settings::internalformat = GL_RGBA8` at every allocation site (audit `src/Video/*.cpp` for `ofFbo::allocate`). |
| M2.7 | Verify `lib/ofxFastFboReader.cpp` readback path via `ShareStrandView` PNG export on Windows. If `GL_IMPLEMENTATION_COLOR_READ_FORMAT` disagrees with the requested format, adapt sws-scale input format in Phase 4. |
| **M2.8** (new, from Review 2) | GL context fallback: on `glfwCreateWindow` failure, retry with GL 3.0/3.1 and log warning. If still fails, show native `MessageBoxW` explaining driver requirement with link. Target: Sandy/Ivy Bridge iGPUs and machines with only Microsoft's OpenGL fallback (GL 1.1). |

**Exit gate / DoD.**
- Windows Nottawa renders a 5-shader strand at ~60 fps on at least two GPU classes (one discrete NVIDIA/AMD + one integrated Intel)
- `affirmScale()` responds correctly to inter-monitor DPI changes
- `ShareStrandView` exports a valid PNG with correct channel order
- sRGB decision documented; startup log confirms enabled/disabled state
- Full-graph parity smoke: SSIM ≥ 0.95 between macOS and Windows keyframes for the reference strand (loose because deferred to Phase 6 tight validation)
- No shader loads log a `glValidateProgram` warning on NVIDIA, AMD, or Intel drivers

**Duration: 5 days likely.**

---

### Phase 3 — Audio capture (WASAPI)

**Goal.** `SystemAudioSource` captures global desktop loopback on Windows via WASAPI. `MicrophoneAudioSource` and `FileAudioSource` work over OF's RtAudio WASAPI backend. Recording-callback path is wired thread-safely so Phase 4's recorder can consume it. **New (Review 1):** validate the audio path end-to-end against a synthetic sink BEFORE Phase 4 begins.

**Entry criteria.** Phase 1 exit met. `src/Util/ComApartment.hpp` exists (from M1.6c).

**Subsystems touched.** audio-capture.

**Ordered milestones.**

| ID | Description |
|---|---|
| M3.1 | Split `src/Audio/SystemAudioSource.hpp`: gate mac members with `__APPLE__`. Add sibling `_WIN32` block: `ComPtr<IAudioClient>`, `ComPtr<IAudioCaptureClient>`, `HANDLE captureEvent`, `std::thread captureThread`, `std::atomic<bool> running`, `WAVEFORMATEXTENSIBLE mixFormat`, portable `sampleRate_`/`channels_`. Change `setTargetProcess` signature to `uint32_t` (zero in-tree callers). |
| M3.2 | Rename `src/Audio/SystemAudioSource.mm` → `SystemAudioSource_mac.mm`. |
| M3.3 | Author `src/Audio/SystemAudioSource_win.cpp` wrapped in `#ifdef _WIN32`: MMDevice enumeration, `GetMixFormat`, `IAudioClient::Initialize` with `AUDCLNT_STREAMFLAGS_LOOPBACK + EVENTCALLBACK`, event-driven `captureLoop_`. **Enter `COINIT_MULTITHREADED` at thread entry** via `ComApartmentMTA` RAII. Push float32 through `ofSoundBuffer` → `AudioSource::audioIn()` (single-source-of-truth for `recordingCallback` fire). Call `gist.setSamplingFrequency(mixRate)` and `gist.setAudioFrameSize(512)` **before** starting capture thread (prevents Gist realloc race). Optionally set MMCSS "Pro Audio" priority via `AvSetMmThreadCharacteristicsW`. All Win32 calls use `W` variants. |
| M3.4 | `MicrophoneAudioSource`: confirm WASAPI shared-mode opens; enable `AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM` if OF 0.12.1 exposes it, else drop to 48000. Move `active=true` after `stream.setup()` success. |
| M3.5 | `FileAudioSource`: confirm WAV/MP3/FLAC/OGG load via `ofxAudioFile`. Verify `dr_wav`/`dr_mp3`/`dr_flac` I/O uses `_wfopen` on Windows via wide-string callback (patch upstream `_LARGE_FILES` I/O if needed for Unicode paths). |
| M3.6 | End-to-end smoke: switch `AudioSourceBrowser` to Microphone → reactive; to Sample Track → playback + BPM to 120±2; to System Audio → reactive while YouTube plays. |
| M3.7 | Document the `recordingCallback` thread-safety contract. `AudioSource::setRecordingCallback` copies the `std::function` under `recordingMutex`, then invokes outside. Global lock ordering: **NEVER hold `recordingMutex` while calling into recorder**. |
| **M3.8** (new, from Review 1) | Wire `recordingCallback` to a synthetic dev-only sink that writes raw f32 PCM to `.raw`. Record 5 s of WASAPI capture. `ffplay -f f32le -ar <rate> -ac <channels>` the .raw and verify no gaps, no drift, correct interleave order. This isolates the audio path from the recorder BEFORE Phase 4 begins. |

**Exit gate / DoD.**
- All three AudioSource types produce reactive visuals on Windows
- Audio-thread FFT peak sits within 20 Hz of a played 1 kHz sine
- No zero-callback masking (`active` flag reflects actual stream state)
- macOS `SystemAudioSource` still works
- Synthetic `.raw` sink capture proves the callback wire is glitch-free
- File load succeeds on a fixture path containing `日本語` (Unicode I/O validated)
- WASAPI capture thread is MTA-initialized (verified in debugger via `CoGetApartmentType`)

**Duration: 6 days likely** (bumped from 4 to absorb Unicode I/O in dr_libs and COM apartment discipline).

---

### Phase 4 — Recording (libav)

**Goal.** Replace `AVFRecorderWrapper`'s mac impl with a Windows `LibavRecorder` that keeps the `AVFRecorderWrapper.hpp` public API bit-for-bit stable. Produces playable MP4 with in-sync AAC audio, honors duration limit and `onRecordingComplete` callback. **New (Review 1):** unit-test the recorder in isolation BEFORE integrating with WASAPI capture.

**Entry criteria.** Phase 3 exit met. `SystemAudioSource_win` produces valid float32 buffers.

**Subsystems touched.** recording.

**Ordered milestones.**

| ID | Description |
|---|---|
| M4.1 | Vendor ffmpeg 7.1.x LGPL shared build via BtbN `--disable-gpl --disable-nonfree --enable-libopenh264 --enable-{nvenc,qsv,amf}`. Assert `strings avcodec-*.dll \| grep enable-gpl` empty. |
| M4.2 | Declare private helper `bool drainEncoder_(AVCodecContext*, AVStream*)` in `AVFRecorderWrapper.hpp` under `#ifdef _WIN32` (fixes sketch's undeclared-symbol link error). |
| M4.3 | Author `src/Video/Recording/LibavRecorder.cpp`: `LibavState` struct with `writeMutex`, `audioTail` ring buffer (fixes sketch's <1024-sample drop bug), reusable `AVFrame`s. Constructor/destructor with abnormal-exit finalize. **UTF-16 path plumbing** — `avio_open` needs `_wfopen`-fed AVIOContext for Unicode output paths. |
| **M4.3a** (new, from Review 1) | Author `dude_wake_up_tests/RecorderTests.hpp`: instantiate `AVFRecorderWrapper`, feed 60 procedural RGBA frames + synthetic 1 kHz sine at 48 kHz stereo, stop, `ffprobe` output. **Pull this test forward from Phase 6** — recorder is unit-tested BEFORE WASAPI integration. |
| M4.4 | Implement `setup()`: hardware encoder ladder try-open (nvenc → qsv → amf → libopenh264) with fallback on `avcodec_open2` failure. Bitrate = `clamp(w*h*4, 1e6, 4e6)` (macOS parity). Force `mp4` muxer regardless of extension (recommended per open question — file discussion with product). **Add `movflags=frag_keyframe+empty_moov+default_base_moof`** so files stay playable after abnormal exit (new from Review 1). Configure AAC encoder with actual audio source's `getSampleRate()` and channel count (queried via `AudioSourceService::getSelectedAudioSource()` at setup, not guessed). |
| M4.5 | Implement `addFrame()`: `sws_scale` RGBA → YUV420P. **Video PTS derived from wall-clock via `av_rescale_q(steady_clock::now() - recordingStartTime, ...)`** — NOT a naive frame counter (fixes drift under shader-load stalls). Single-invocation `onRecordingComplete` on duration-limit path (fixes sketch's double-fire). **`av_interleaved_write_frame` called under `writeMutex`** to serialize with audio path. |
| M4.6 | Implement `addAudioSamples()`: proper 1024-sample AAC chunking with `audioTail` carry-over under `writeMutex`. **Rebuild SwrContext** if `(sampleRate, channels)` differ from initial (handles mid-record source switch — new from Review 1). `swr_convert`; final zero-padded flush on `stop()`. |
| M4.7 | Implement `start()`/`stop()`/`isRecording()`/`getElapsedTime()`: `avformat_write_header` / `av_write_trailer`, drain both encoders, close IO, fire callbacks after moov finalized. |
| M4.8 | Update `.vcxproj` source list: add `LibavRecorder.cpp`, exclude `AVFRecorder.h/.m/.mm` on Windows, add ffmpeg include/lib paths, post-build DLL copy. |
| M4.9 | Five-scenario smoke matrix: (a) OutputWindow Ctrl+R no-audio, (b) ShareStrandView 15s no-audio, (c) ShareStrandView 15s mic mono→stereo, (d) ShareStrandView 15s WASAPI system audio, (e) hard-kill mid-record. ffprobe validates codec/dimensions/duration; VLC + Windows Media Player confirm A/V sync within 30 ms. **Scenario (e) verifies fragmented MP4 stays playable up to last flush.** Decode frame 30 from (a) at a known-color pixel and assert channel order (validates RGBA-vs-BGRA readback assumption). |
| **M4.10** (new, from Review 1) | Constrain UI: `AudioSourceBrowser` disables audio-source switch while `videoRecorder.isRecording()` is true (grey out with tooltip). Prevents mid-record source-switch races. |

**Exit gate / DoD.**
- All five smoke scenarios pass
- Recorded MP4s play in VLC AND Windows Media Player with in-sync audio
- `onRecordingComplete` fires exactly once per recording
- Hardware encoder ladder logs which encoder actually opened
- 30-second soak with an expensive shader chain produces audio track duration = video track duration ± 100 ms (validates wall-clock PTS)
- Recording completes on a fixture output path with `Nottawa 日本語.mp4` (Unicode I/O validated)
- Application Verifier run under a stress harness (video path + audio path concurrent) shows no deadlock, no data race

**Duration: 8 days likely** (bumped from 5 to absorb wall-clock PTS refactor, muxer thread-safety hardening, Unicode I/O, unit test pull-forward).

---

### Phase 5 — Ancillary services

**Goal.** Neutralize `BookmarkService` on Windows, split `OutputWindow.mm`, land `nottawa://` URL scheme handler with UTF-16-native registry + named-pipe single-instance forwarding, add cross-platform reveal-in-file-manager helper. SparkleUpdater stays stubbed (Option B) for MVP.

**Entry criteria.** Phase 1 exit met (can run parallel to 2/3/4 with a second engineer).

**Subsystems touched.** ancillary-services.

**Ordered milestones.**

| ID | Description |
|---|---|
| M5.1 | `BookmarkService` neutralization: gate `<CoreFoundation>` and `CFDataRef`-returning methods behind `__APPLE__` in `.hpp`. Wrap `.cpp` body in `__APPLE__/else`. Windows `else` block: `hasBookmark` returns true, `bookmarkForPath` = `make_shared<string>(path)`, `begin/endAccess` no-op. |
| M5.2 | `VideoSourceService.cpp` CF-block refactor: extract `CFURLCreateByResolvingBookmarkData` / `StartAccessingSecurityScopedResource` into `static bool resolveAndAccessBookmark(path)` helper with `__APPLE__` split; Windows branch returns true. Grep for all three uses (`makeFileVideoSource`, `makeImageVideoSource`, `startAccessingBookmarkPath`). |
| M5.3 | Add `src/Util/RevealInFileManager.{hpp,cpp}`: **`ShellExecuteW`** on Windows (COM-scoped STA via `ComApartmentSTA`), `open` shell-out on macOS. Replace `OutputWindow.mm:188` and `VideoSourceService.cpp:791`. |
| M5.4 | Split `OutputWindow`: rename `.mm` → `.cpp`. Move `OutputWindowDelegate` + `setupNativeMenu`/`updateMenuState` into new `OutputWindow_menu_mac.mm` (Apple only). Add `OutputWindow_menu_win.cpp` with ImGui-in-second-window Record submenu (**spike first** that ofxImGui supports per-window contexts with shared GL — GATE-E). Confirm `OF_KEY_COMMAND` maps to Ctrl on Windows; add `OF_KEY_CONTROL` fallback if not. **Second-window DPI:** query `glfwGetWindowContentScale` on move/resize; rebuild font atlas at new scale (new from Review 2). |
| M5.5 | URLSchemeHandler split: move `handleURL`/`setStrandLoadedCallback`/`getInstance`/pending-URL queue from `.mm` into `URLSchemeHandler_shared.cpp` (both platforms). |
| M5.6 | Author `URLSchemeHandler_win.cpp`: **`RegSetValueExW`** under `HKCU\Software\Classes\nottawa` (per-user, no admin). `CreateMutexW(nullptr, TRUE, L"Local\\NottawaSingleInstance_<user-hash>")` — **`Local\` namespace, not `Global\`** (Review 2: `Global\` requires `SeCreateGlobalPrivilege` unavailable in RDP/FUS sessions; also per-session-isolates via user hash). Named-pipe listener thread on `\\.\pipe\NottawaURLPipe_<user-hash>` with restricted DACL granting only current user SID. **Split init:** `initSingleInstanceAndPipe()` called from `main.cpp` BEFORE `ofRunApp` (mutex + pipe listener up before any second-instance can forward); rest of init called from `MainApp::setup()`. |
| M5.7 | `main.cpp`: switch to `int main(int argc, char** argv)` but **use `GetCommandLineW()` + `CommandLineToArgvW()`** on Windows for true UTF-16 command line (not ANSI argv). Scan for `nottawa://` prefix; if mutex already held, `CreateFileW` the pipe, `WriteFile` UTF-8-encoded URL, exit(0) with 200 ms retry-on-`ERROR_PIPE_BUSY`. Else push into pending queue before `ofRunApp`. Widen `URLSchemeHandler` init and `processPendingURL` gates to include `_WIN32`. |
| M5.8 | SparkleUpdater: ship Option B (stub, existing gates untouched) for MVP. Defer WinSparkle to Phase 7. |
| M5.9 | End-to-end: cold-launch `start nottawa://strands/testslug` opens app + loads strand. Warm-launch second `.exe` exits <200 ms and first instance loads. `nottawa://strand/日本語` (Unicode slug) survives argv → pipe → handler round-trip. Ctrl+R records in OutputWindow. Drop video file loads without BookmarkService errors. Explorer opens output folder on recording finish. URLSchemeHandler destructor cleanly joins pipe listener thread on app exit. |

**Exit gate / DoD.**
- All Windows ancillary flows work: URL scheme cold/warm/Unicode, OutputWindow recording, file drop, reveal-in-Explorer
- macOS regression pass confirms no change on Sparkle, native menu bar, Apple Event URL, security-scoped bookmarks
- Second-instance URL forwarding survives an RDP session (mutex creation does not fail)
- Registry entries appear only under `HKCU\Software\Classes\nottawa` (per-user install verification)
- No `*A(` Win32 calls in `_win.cpp` files (pre-push scanner passes)
- OutputWindow ImGui menu renders correctly at 200% DPI when dragged to a scaled monitor

**Duration: 8 days likely** (bumped from 5 to absorb Unicode plumbing, mutex/pipe security fixes, second-window DPI, ofxImGui-per-window spike risk).

---

### Phase 6 — Testing & CI

**Goal.** Rehabilitate the gtest suite for headless CI, add BTrack/Recorder/SystemAudio/parity/golden-image tests, stand up GitHub Actions windows-2022 + macos-14 jobs with cached OF and ffmpeg, and land the RC checklist template. This lane runs continuously so every phase's exit gate is verifiable in CI.

**Entry criteria.** Phase 1 exit met (can run parallel to 2/3/4/5 with a second engineer). Phase 0 M0.10 spike has already answered whether gtest suite builds.

**Subsystems touched.** testing-ci.

**Ordered milestones.**

| ID | Description |
|---|---|
| M6.1 | Rehabilitate `dude_wake_up_tests` on macOS trunk if broken (per M0.10 spike). Sync service singleton forward-decls in `ShaderChainerServiceTests.hpp:29-42`. Budget 2–4d rehab if needed. |
| M6.2 | Rename existing `test.sh` → `codesign_fragment.sh` (misnamed — currently a codesigning script). Grep repo for callers and update. |
| M6.3 | Add `--headless` mode to `dude_wake_up_tests/main.cpp`: `glfwWindowHint(GLFW_VISIBLE, false)`, skip `ofRunApp`, run gtest on main thread, `ofExit`. Add `NOMINMAX`/`WIN32_LEAN_AND_MEAN` when `_WIN32`. |
| M6.4 | Write new cross-platform `test.sh` + `test.ps1` dispatchers (`-Unit`/`-Golden`/`-Audio`/`-Recorder`/`-Parity`/`-All`/`-Filter`/`-Config`). |
| M6.5 | Delete dead `src/Audio/test_bpm_detector.cpp` (already in Phase 0 M0.6). Author `dude_wake_up_tests/BTrackDetectorTests.hpp` against `tests/fixtures/audio/click_120bpm.wav`. |
| M6.6 | Author `dude_wake_up_tests/RecorderTests.hpp` — already pulled forward to Phase 4 M4.3a. Here we integrate it into CI. |
| M6.7 | Author `dude_wake_up_tests/SystemAudioTests.hpp` — captures ffplay-played sine via WASAPI loopback, asserts FFT peak within 20 Hz of 1 kHz. `GTEST_SKIP` if no active endpoint. |
| M6.8 | Build `tools/shader_snapshot/main.cpp` headless GL harness. Author `tests/golden/run_golden.py` with per-vendor baselines + SSIM ≥ 0.98 + MAE ≤ 2/255. **Baselines must be captured AFTER Phase 2 M2.0 sRGB decision** (new from Review 2). |
| M6.9 | Author `tests/parity/run_parity.py` against a canonical Strand json with deterministic time+audio. **GATE-G:** requires TimeService/AudioSource injectable-clock — coordinate with foundation/audio-capture leads. If GATE-G fails, degrade to visual-similarity spot-check on 3 non-reactive shaders. |
| M6.10 | Write `.github/workflows/ci.yml` (win-build-test on windows-2022, mac-build-test on macos-14, cache OF 0.12.1 + ffmpeg-lgpl). Write `tools/ci/install_deps.ps1`. Include Unicode fixture path in the test suite. |
| M6.11 | Write `.github/workflows/gpu-matrix.yml` (self-hosted labels gpu-nvidia/amd/intel, WARP fallback for smoke) and `.github/ISSUE_TEMPLATE/rc-checklist.md`. |
| **M6.12** (new, from Review 3) | Add locale test: run a subprocess with `LANG=de_DE.UTF-8` (or `SetThreadUILanguage`) and verify a saved strand round-trips (catches `sprintf('%.2f', 3.14)` = `'3,14'` bugs). |
| **M6.13** (new, from Review 3) | Add appcast validator: CI job that curls `appcast.xml`, validates XML, checks both platform enclosures resolve to 200 OK, verifies ed25519 sig. Cron every 30 min post-launch. |
| **M6.14** (new, from Review 3) | Add release-cadence-parity gate: CI job refuses to publish appcast unless both platform enclosures exist for a stable release (or item is tagged as platform-specific hotfix). |

**Exit gate / DoD.**
- PR triggers both `windows-2022` and `macos-14` jobs; both green on a no-op change
- Intentionally perturbing a shader produces a failing golden run with diff PNG artifact
- RC checklist template renders correctly when tagging `rc-*`
- Locale test catches a deliberately-introduced comma-decimal `sprintf` bug
- Appcast validator alerts to email when a fixture bad-signature scenario is uploaded

**Duration: 13 days likely** (bumped from 11 to absorb locale + appcast validator + parity gate, and to reflect Review 1's "3 independent high-likelihood risks compound" observation).

---

### Phase 6.5 — Production observability (Sentry, logs, diagnostics)

**NEW PHASE inserted from Review 3.** Non-negotiable Phase 7 dependency.

**Goal.** Crash reports flow off-box to a symbol server for post-hoc symbolication. Users have a "Reveal Diagnostic Log" affordance. PDB retention is wired into the release script.

**Entry criteria.** Phase 4 exit met. Phase 5 exit met.

**Subsystems touched.** distribution, testing-ci.

**Ordered milestones.**

| ID | Description |
|---|---|
| M6.5.1 | Vendor `sentry-native` + `crashpad_handler.exe`. Wire `sentry_init` before `ofRunApp` with DSN behind a runtime opt-in (first-run dialog). Add breadcrumb hooks at shader-load, recorder-start, WASAPI-activate. |
| M6.5.2 | Configure `ofSetLogLevel` to write to `%LOCALAPPDATA%\Nottawa\logs\nottawa-YYYY-MM-DD.log` with 30-day rotation. Add `Help > Reveal Diagnostic Log` and `Help > Copy System Info to Clipboard` menu items (system info dumps GL vendor/renderer/version, Windows build, WASAPI default endpoint format, ffmpeg version, hardware encoder opened, camera device + negotiated format). |
| M6.5.3 | `build_release.ps1`: after link, upload `nottawa.pdb` + `WinSparkle.pdb` + ffmpeg `.pdb`s (when available from LGPL build) to Sentry symbol server (or Azure Blob + `symstore.exe`). Tag by AppVersion + Git SHA. Retain 24+ months. |
| M6.5.4 | Write `docs/WINDOWS_TROUBLESHOOTING.md` with diagnostic checklists for "no audio", "no camera", "blurry text", "recording is empty", "MIDI controller not detected", "app opens then closes". |
| M6.5.5 | First-run opt-in dialog for telemetry. Update privacy policy on `nottawa.app` BEFORE first Windows release. Ensure Sentry DSN is a runtime setting so it can be disabled. Scrub filenames/GPU strings. |
| M6.5.6 | `SetConsoleOutputCP(CP_UTF8)` + `SetConsoleCP(CP_UTF8)` early in WinMain. Use `FormatMessageW` everywhere for `GetLastError` formatting. |

**Exit gate / DoD.**
- Simulated crash produces a symbolicated stack in Sentry within 5 minutes
- `Help > Reveal Diagnostic Log` opens Explorer at a log file with today's date
- `Help > Copy System Info` clipboard contents include all documented fields
- Privacy policy update on `nottawa.app` is live
- `docs/WINDOWS_TROUBLESHOOTING.md` published and linked from README

**Duration: 3 days likely.**

---

### Phase 7 — Distribution & release

**Goal.** Ship signed `Nottawa-Setup-X.Y.Z.exe`. Register `nottawa://`. Integrate into existing single-appcast (mac + win with `<sparkle:os>` filters). WinSparkle auto-updates using shared ed25519 key.

**Entry criteria.** Phase 6.5 exit met. EV cert issued (or accepted OV fallback per GATE-H). Preview installer pipeline from M1.10 has been running successfully throughout the port.

**Subsystems touched.** distribution, ancillary-services.

**Ordered milestones.**

| ID | Description |
|---|---|
| M7.1 | EV cert already ordered on day 1 (M0.11). Confirm arrival by GATE-H. If OV fallback, proceed with SmartScreen warning acknowledgement to early testers. |
| M7.2 | Provision packaging box: Inno Setup 6.2, ImageMagick, WinSparkle 0.8+ SDK, BtbN ffmpeg LGPL zip, `VC_redist.x64.exe` (14.40+). Convert `icon.icns` → `packaging/windows/icon.ico`. |
| M7.3 | Write `packaging/windows/LICENSES/*.txt` for every bundled library (already partially done in Phase 0 M0.9). Assert LGPL (not GPL) ffmpeg via `strings avcodec-*.dll \| grep enable-gpl` empty. |
| M7.4 | Write `packaging/windows/nottawa.iss`: `[Setup]` modern wizard, `HKA` URL scheme, `[Files]` `recursesubdirs` for `data\`, `[Run]` `VC_redist` with `VCRedistNeedsInstall` check, `[Code]` `IsWindows10Build19041OrLater` soft warning (non-blocking). **Add firewall rule** via `netsh advfirewall firewall add rule name="Nottawa Ableton Link" dir=in action=allow program="{app}\nottawa.exe"` in `[Run]` (per-machine only); `[UninstallRun]` deletes rule (new from Review 2). |
| M7.5 | Write `packaging/windows/sign.ps1` (`signtool /a` with EV token OR `AzureSignTool` cloud HSM) and `packaging/windows/build_release.ps1` orchestrator (msbuild → stage → sign binaries → iscc → sign installer → SHA256). Extended from M1.10 preview version. |
| M7.6 | Fresh-VM install/uninstall matrix on Win 10 1809, Win 10 22H2, Win 11 23H2 (VC++ absent — must trigger silent redist). Confirm URL scheme click from browser opens app with argv. Add/Remove Programs entry correct with icon. Uninstall clean. **Include a VM with a non-ASCII username** (Unicode path validation). |
| M7.7 | WinSparkle wiring: vendor `lib/winsparkle/`, author `src/Services/SparkleUpdater_win.cpp` with `win_sparkle_set_appcast_url` + `init` + check APIs. Widen SparkleUpdater gates in `MainApp.cpp:70,75` and `MainStageView.cpp:330` to include `_WIN32`. Export ed25519 key from macOS Keychain, import to Windows box. Run `sign_update.exe` and confirm signature matches macOS-produced sig against `SUPublicEDKey` at `openFrameworks-Info.plist:18`. **Register shutdown-request handler** that postpones relaunch if `videoRecorder.isRecording()` (new from Review 1). |
| M7.8 | Refactor `deploy_update.sh` to accept `--platform windows\|macos\|both`. Emit two `<enclosure>` tags per `<item>` with `<sparkle:os>` filters. Extract appcast-merge logic from `release.sh` into `release-common.sh`. Pipe generated appcast through `xmllint` before upload. Publish to a staging appcast URL first; run a synthetic client against staging before promoting to production (new from Review 3). |
| M7.9 | End-to-end dry run: publish v1.0.0-rc1, install on clean Win 10 22H2 + Win 11 VMs. Bump to rc2. `build_release.ps1 -Publish`. Check for Updates in-app. WinSparkle downloads/verifies/silently installs/relaunches. `%APPDATA%\Nottawa` preserved. |
| M7.10 | Update `SPARKLE_SETUP.md` with Windows section. Add `packaging/README-release.md` as the two-platform release runbook. |
| **M7.11** (new, from Review 3) | Submit signed installer + `nottawa.exe` to Microsoft Defender submission portal 72h before public launch. Test under Defender with Cloud Protection ON. Include instructions in `docs/WINDOWS_TROUBLESHOOTING.md` for how users unquarantine if AV false-positives. |
| **M7.12** (new, from Review 3) | Signing recovery documentation. `docs/SIGNING_RECOVERY.md`: EV cert backup policy (Azure Key Vault preferred over hardware for exactly this reason). Second authorized signer on the vault. Ed25519 key stored in password manager with documented rotation plan. |
| **M7.13** (new, from Review 3) | CI-signed release path as exit gate. Tagged release CI job builds, signs (via cloud HSM), and publishes end-to-end without touching Joe's laptop. Drops bus factor. Requires committing to SSL.com eSigner or Azure Key Vault (not YubiKey). |
| **M7.14** (new, from Review 3) | Beta ring: 20–50 external testers 2–4 weeks before public launch to warm SmartScreen reputation and catch environment-specific issues (AV, camera hardware, corporate policies). |

**Exit gate / DoD.**
- `Nottawa-Setup-1.0.0.exe` is EV-signed, timestamped, installs cleanly on Win 10 1809 / Win 10 22H2 / Win 11 23H2
- WinSparkle upgrade from rc1 → rc2 succeeds silently and preserves user data
- Mac dev build and Windows dev build both see appropriate platform updates from the single appcast
- Fresh contributor can execute a Windows release from cold-start using `docs/WINDOWS_ARCHITECTURE.md` + `docs/SIGNING_RECOVERY.md` alone (verify by having someone unfamiliar attempt it)
- Microsoft Defender submission complete; no quarantine on test VMs with Cloud Protection enabled
- CI-signed path publishes a release without touching maintainer's laptop
- Beta ring feedback triaged; no P0 issues open

**Duration: 9 days likely** (bumped from 6 to absorb Defender submission, signing recovery docs, CI-signed path, beta ring coordination).

---

## 4. Per-Subsystem Detail

Distilled from the eight subsystem plans. Read this section when you're about to touch a specific area — file paths and line numbers are load-bearing.

### 4.1 Build system

Owns: `.sln`/`.vcxproj` generation, addon trim, include-path graph, preprocessor macros, third-party runtime bundling.

Key file touches:
- `addons.make` — drop `ofxSyphon` (line 3) and `ofxFFmpegRecorder` (line 5); keep `ofxImGui`, `ofxMidi`, `ofxFastFboReader`
- `compile_flags.txt` — Windows-agnostic replacement (drop `__MACOSX_CORE__`, add `__WINDOWS_WASAPI__`, `NOMINMAX`, `WIN32_LEAN_AND_MEAN`, `_USE_MATH_DEFINES`, `TARGET_WIN32=1`; purge stale absolute paths at lines 14-20)
- `src/Services/ShaderChainerService.cpp:10,122` — case-mismatch fixes
- `apps/myApps/Nottawa/Nottawa.{sln,vcxproj,vcxproj.filters}` — generated + hand-edited (commit them)
- `.gitignore` — add VS artifacts (`Debug/`, `Release/`, `x64/`, `bin/*.pdb`, `bin/*.ilk`, `*.user`, `.vs/`)

Preprocessor defines the Windows `.vcxproj` must set: `NOMINMAX`, `WIN32_LEAN_AND_MEAN`, `_USE_MATH_DEFINES`, `_CRT_SECURE_NO_WARNINGS`, `__WINDOWS_WASAPI__`, `TARGET_WIN32`, `GLM_FORCE_CTOR_INIT`, `GLM_ENABLE_EXPERIMENTAL`, `OF_NO_FMOD=1`, `USE_KISS_FFT`, `ofxAddons_ENABLE_IMGUI`, `JSON_DIAGNOSTICS=1`, `IMGUI_DEFINE_MATH_OPERATORS=1`, `LINK_PLATFORM_WINDOWS`.

Path corrections vs prior research: `IOSurfaceTextureManager` lives at `src/Engine/`, not `src/Video/`. `EngineLoop.cpp` is at `src/Engine/`, not `src/Apps/`.

### 4.2 Foundation

Owns: `mach_time` → `std::chrono` replacement, CoreFoundation include gating, filename-case fixes, vendored-header verification, uintptr_t/long truncation fix, UTF-8/UTF-16 helpers, COM apartment RAII.

Key file touches:
- `src/Util/MSATimer.h` — replace `mach_absolute_time` with `std::chrono::steady_clock` (single portable impl)
- `src/NodeLayout/NodeTypes.hpp:20,51` and `src/NodeLayout/NodeLayoutView.cpp:349` — uintptr_t truncation fix
- `src/Util/PortabilityCheck.hpp` (new) — NOMINMAX canary TU
- `src/Util/Utf8Path.hpp/.cpp` (new) — UTF-8 ↔ UTF-16 helpers
- `src/Util/ComApartment.hpp` (new) — RAII COM init wrappers
- `src/Audio/dr_wav.h`, `dr_mp3.h`, `dr_flac.h` — re-vendor from upstream mackron/dr_libs

Prior research corrections (verified in Phase 0 audit): `httplib.h` at `src/Util/httplib.h` IS cpp-httplib v0.12.1 with intact `_WIN32` / `NOMINMAX` / `winsock2.h` guards. The prior claim of "stripped Windows guards" was inaccurate — keep as-is and verify via diff. `dr_wav.h` v0.10.1, `dr_mp3.h` v0.4.7, `dr_flac.h` v0.12.1 also all have intact guards; re-vendoring is cleanliness, not necessity.

### 4.3 Rendering

Owns: `sampler2DRect` → `sampler2D`, DPI + long-path manifest, dead `shader_glitch` retirement, webcam pixel-format defense, sRGB pipeline policy, IOSurface exclusion.

Key file touches:
- `shaders/{empty,fractal,fuji,plasma}.frag` — line 3 or 4 sampler swap
- `shaders/shader_glitch.{frag,vert}` — delete
- `app.manifest` (new) — PerMonitorV2 DPI + `<ws2:longPathAware>true</ws2:longPathAware>`
- `src/Video/WebcamSource.cpp:18` — defensive pixel-format negotiation (RGB → RGBA, no `NATIVE` fallthrough)
- `src/Apps/MainApp.cpp:33,34,41,43,219` — Sparkle/URL includes gate, `ofDisableArbTex()`, `affirmScale()` DPI logic (already correct)
- `src/Engine/IOSurfaceTextureManager.{hpp,cpp}` — gate entire body under `#ifdef __APPLE__`
- `src/Engine/EngineLoop.cpp` and `src/Bridge/nottawa_bridge.{h,cpp}` — exclude from Windows `.vcxproj` (SwiftUI-target only)

Shader survey summary: 171 shaders in `./shaders/`; 170 use `#version 150` cleanly; only 4 live shaders + 1 dead file use `sampler2DRect`. No shader uses `#extension` or `gl_FragColor`.

### 4.4 Audio capture

Owns: `SystemAudioSource` cross-platform split, WASAPI global loopback impl, `MicrophoneAudioSource`/`FileAudioSource` WASAPI negotiation, `recordingCallback` thread-safety contract.

Key file touches:
- `src/Audio/SystemAudioSource.hpp` — gate mac members under `__APPLE__`, add `_WIN32` block with WASAPI members, portable `sampleRate_`/`channels_`
- `src/Audio/SystemAudioSource.mm` → `SystemAudioSource_mac.mm`
- `src/Audio/SystemAudioSource_win.cpp` (new) — WASAPI impl; COM MTA, `_wfopen`-capable I/O, wall-clock-monotonic buffer timing
- `src/Services/AudioSourceService.cpp` — verify OF WASAPI backend enumerates devices; no source change required
- `src/Audio/AudioSource.hpp/.cpp` — no changes (portable)
- `src/Audio/MicrophoneAudioSource.cpp:18-38` — move `active=true` after `stream.setup()` success
- `src/Audio/FileAudioSource.cpp` — no changes (portable via `ofxAudioFile` + dr_libs; dr_libs need Unicode I/O plumbing)
- `src/Audio/gist/Gist.h:41` — no changes (`USE_KISS_FFT` hardcoded, no Accelerate)

Verified: `setTargetProcess` and `setGlobalTap` have zero in-tree callers — per-process WASAPI loopback (Win 10 2004+) deferred to post-MVP.

### 4.5 Recording

Owns: `AVFRecorderWrapper` Windows impl via libav, hardware encoder ladder, AAC audio muxing, thread-safety, wall-clock PTS, Unicode output paths, abnormal-exit fragmented MP4.

Key file touches:
- `src/Video/Recording/AVFRecorderWrapper.hpp:53` — update comment; declare private `drainEncoder_` helper under `#ifdef _WIN32`
- `src/Video/Recording/AVFRecorderWrapper.mm` + `AVFRecorder.h/.m` — gate under `#ifdef __APPLE__`
- `src/Video/Recording/LibavRecorder.cpp` (new) — full Windows impl
- `libs/ffmpeg/include/**` + `libs/ffmpeg/lib/win64/*.lib` + `bin/*.dll` — vendored
- `src/Video/OutputWindow.hpp` and `src/Views/ShareStrandView.hpp` — no changes (public API preserved)

Sketch defects to fix during M4.3–M4.6:
1. `drainEncoder_` declared and defined (fixes link error)
2. AAC frame chunking uses tail-carry-over ring buffer (fixes dropped <1024-sample bug)
3. `onRecordingComplete` invoked exactly once from `stop()` — remove duplicate call from `addFrame()` duration-limit path
4. `movflags=frag_keyframe+empty_moov+default_base_moof` for hard-kill playability
5. Video PTS from `steady_clock`, not frame counter
6. `writeMutex` guards ALL `avformat_*` calls (audio + video) to prevent muxer race
7. SwrContext rebuilt if audio source format changes mid-record
8. `avio_open` via `_wfopen`-fed `AVIOContext` for Unicode output paths
9. libx264 (GPL) forbidden; openh264 (BSD-2 + Cisco patent grant) is the CPU fallback
10. Force `mp4` muxer regardless of extension for cross-platform parity

### 4.6 Ancillary services

Owns: `BookmarkService` Windows neutralization, `nottawa://` URL scheme (registry + single-instance + named pipe), `OutputWindow` file split, reveal-in-file-manager helper.

Key file touches:
- `src/Services/BookmarkService.hpp/.cpp` — Windows passthrough stubs (~30 LOC)
- `src/Services/VideoSourceService.cpp:282-350,396-460,795+,791` — extract CF block into helper; replace `system("open ...")` with `RevealInFileManager`
- `src/Services/URLSchemeHandler_shared.cpp` (new) — platform-neutral URL parse
- `src/Services/URLSchemeHandler_win.cpp` (new) — HKCU registry, `Local\` mutex, per-user pipe, restricted DACL, split init
- `src/main.cpp:81,103-105` — `GetCommandLineW()`, single-instance forwarding, widen URL scheme gate
- `src/Video/OutputWindow.mm` → `OutputWindow.cpp` + `OutputWindow_menu_mac.mm` + `OutputWindow_menu_win.cpp`
- `src/Util/RevealInFileManager.{hpp,cpp}` (new) — cross-platform helper

The Sparkle Option B (stub for MVP) requires zero source edits — existing `#ifdef TARGET_OSX` gates in `MainApp.cpp:70,75` and `MainStageView.cpp:330` already silence WinSparkle. Widen gates in Phase 7 M7.7 only.

### 4.7 Testing & CI

Owns: gtest suite rehab + headless mode, BTrack/System/Recorder tests, golden-image harness, parity harness, GitHub Actions CI, RC checklist, locale + appcast validators.

Key file touches (all under `dude_wake_up_tests/`, `tests/`, `tools/`, `.github/`):
- `dude_wake_up_tests/main.cpp` — add `--headless` mode
- `dude_wake_up_tests/BTrackDetectorTests.hpp` (new)
- `dude_wake_up_tests/RecorderTests.hpp` (new) — pulled forward to Phase 4 M4.3a
- `dude_wake_up_tests/SystemAudioTests.hpp` (new)
- `tools/shader_snapshot/main.cpp` (new)
- `tests/golden/run_golden.py` (new)
- `tests/parity/run_parity.py` (new)
- `tests/fixtures/audio/{click_120bpm.wav, sine_1khz_2s.wav}` (new)
- `.github/workflows/ci.yml`, `gpu-matrix.yml`, `.github/ISSUE_TEMPLATE/rc-checklist.md` (new)
- `tools/ci/install_deps.ps1` (new)

Ground-truth: current `test.sh` at repo root is a codesigning fragment, NOT a test runner — rename to `codesign_fragment.sh` in Phase 6 M6.2.

### 4.8 Distribution

Owns: Inno Setup installer, EV signing (signtool / AzureSignTool), WinSparkle integration, appcast dual-platform merge, VC++ redist bundling, LGPL license bundling, PDB retention.

Key file touches (all under `packaging/windows/` unless noted):
- `nottawa.iss` (new) — modern wizard, HKA URL scheme, firewall rule, VC redist trigger, Windows 10 pre-20H1 soft warning
- `build_release.ps1` (new) — orchestrator (msbuild → stage → sign → iscc → sign installer → SHA256 → PDB upload)
- `sign.ps1` (new) — signtool wrapper (EV token or AzureSignTool)
- `LICENSES/` (new) — bundled third-party license texts
- `icon.ico` (new) — converted from `icon.icns`
- `README.md` and `README-release.md` (new)
- `.gitignore` (new)
- `deploy_update.sh` (edit) — accept `--platform windows|macos|both`, emit two enclosures per item
- `release.sh` (edit) — extract appcast merge into `release-common.sh`
- `SPARKLE_SETUP.md` (edit) — append Windows/WinSparkle section

Critical constraint: ffmpeg build must be `--disable-gpl --disable-nonfree`. Assert via `strings avcodec-*.dll \| grep enable-gpl` empty in `build_release.ps1`.

---

## 5. Risk Register (merged)

Rank ordered by (likelihood × impact). Includes reviewer-added risks.

| # | Risk | Phase | Likelihood | Impact | Mitigation |
|---|---|---|---|---|---|
| 1 | UTF-8/UTF-16 path mishandling silently mangles paths for non-ASCII users (registry, ffmpeg I/O, dr_libs, single-instance pipe, argv) | 0 | high | high | Phase 0 M0.8 introduces `Utf8Path.hpp`; pre-push scanner blocks new `*A(` calls in `_win.cpp`; CI test with non-ASCII fixture path on both platforms |
| 2 | COM apartment RPC_E_CHANGED_MODE crash at first WASAPI or WinSparkle init | 1 | high | high | Phase 1 M1.6c settles apartment policy; RAII wrappers; debug-only apartment-type assertions at each COM entry |
| 3 | GPU driver GLSL rejection beyond the 4 known sampler2DRect files (integer promotion, uninit `out` fragments, const-array indexing) | 2 | high | medium | Persistent `glValidateProgram` diagnostic wrapping `ofShader::load`; log GL_VENDOR/RENDERER/VERSION at startup; fix per-shader as they surface |
| 4 | sRGB / framebuffer color-space mismatch breaks parity tests wholesale | 2 | high | high | Phase 2 M2.0 audit; enable `GL_FRAMEBUFFER_SRGB` consistently; capture golden baselines only after policy settled |
| 5 | Recorder A/V sync fails due to naive frame-counter PTS under shader-load stalls | 4 | high | high | Wall-clock PTS from `steady_clock`; unit-tested in M4.3a before WASAPI integration; 30s soak in M4.9 with expensive shader |
| 6 | Muxer thread-safety: `av_interleaved_write_frame` called concurrently from audio + video paths | 4 | high | high | Redefine `writeMutex` to guard ALL `avformat_*` calls; Application Verifier stress test in Phase 6 CI |
| 7 | Deadlock between `AudioSource::recordingMutex` and `LibavRecorder::writeMutex` on stop-recording | 4 | high | high | Global lock ordering documented; `setRecordingCallback` copies function pointer under lock, releases, then invokes |
| 8 | dude_wake_up_tests bit-rotted (service singleton forward-decls only cover ~13 services vs current ~20+) | 6 | high | high | Phase 0 M0.10 spike verifies on macOS trunk; budget 2–4d rehab in Phase 6 M6.1 |
| 9 | Deterministic parity mode requires TimeService/AudioSource injectable-clock changes | 6 | high | high | Raise ask early; GATE-G degrades parity test to visual spot-check if not delivered |
| 10 | GitHub Actions windows-latest has no physical GPU; shader golden tests unreliable | 6 | high | high | Two-tier CI: free runner for unit/audio/recorder; self-hosted `gpu-nvidia/amd/intel` runners for golden; WARP smoke on free tier |
| 11 | Windows Firewall prompt on first launch tanks UX; user declines, Ableton Link silently fails | 5,7 | high | medium | Installer adds firewall rule during per-machine install (M7.4); documented per-user fallback + Help menu action |
| 12 | Nottawa recorder recordingCallback thread races if audio source switched mid-record | 4 | medium | high | UI disables source switch while `isRecording()` (M4.10); SwrContext rebuilt if format changes (M4.6) |
| 13 | EV cert issuance takes 3–10 business days; blocks first signed installer | 7 | medium | high | Order day 1 (M0.11); self-signed cert for internal test during wait (M1.10); OV fallback per GATE-H |
| 14 | AV/EDR false-positive quarantines installer or WinSparkle downloaded delta post-release | 7 | medium | high | Pre-submit signed installer to Microsoft Defender submission portal 72h before public release (M7.11); test with Cloud Protection ON; sign delta with same EV cert |
| 15 | openh264 patent/licensing complexity if bundling Cisco DLL | 4,7 | medium | high | Legal review; ship first-run downloader from ciscobinary.openh264.org as safer alternative; hardware encoders (NVENC/QSV/AMF) sidestep on most rigs |
| 16 | ofDirectShowGrabber can't enumerate OBS Virtual Camera or MJPG-only UVC | 2 | medium | medium | Defensive pixel-format negotiation (M2.5); Media Foundation IMFSourceReader as documented follow-up if it bites |
| 17 | ffmpeg DLL ABI drift ships wrong GPL variant | 4,7 | medium | medium | Pin exact BtbN LGPL URL; assertion in `build_release.ps1` |
| 18 | projectGenerator overwrites hand-edited `.vcxproj` on future addon change | 1 | high | medium | Commit generated files; document "do not re-run without merging"; migrate Windows-specific settings to `.props` file (est. 4h) |
| 19 | OutputWindow ImGui-in-second-window may not work in OF 0.12.1 | 5 | medium | medium | Time-box 2h spike in M5.4 (GATE-E); Ctrl+R hotkey-only fallback documented |
| 20 | AAC encoder audio format (sample rate / channels) mismatched with actual source format | 4 | medium | medium | Configure encoder + SwrContext from `audioSource->getSampleRate()` at `setup()` time (M4.4); rebuild on mid-record change (M4.6) |
| 21 | Named-pipe race: two nottawa:// links opened before primary's listener thread spins up drops the second | 5 | medium | medium | Create mutex + start pipe listener BEFORE `ofRunApp` (M5.6); second-instance 200ms retry loop on `ERROR_PIPE_BUSY` (M5.7) |
| 22 | ofFbo internal format not RGBA8 on Intel iGPU; buffer overflow in sws_scale | 4 | low | high | Force `ofFbo::Settings::internalformat = GL_RGBA8`; assert `pixels.size() == w*h*4` at addFrame top |
| 23 | WinSparkle silent-relaunch during active recording loses the current MP4 | 4,7 | low | medium | Register shutdown-request handler that checks `isRecording()` and postpones or stop()s first (M7.7); fragmented-MP4 as secondary safety |
| 24 | Signing infrastructure single point of failure — YubiKey lost / laptop stolen | 7 | low | high | Prefer Azure Key Vault over YubiKey (M7.13); second authorized signer; `docs/SIGNING_RECOVERY.md` (M7.12); CI-signed path drops bus factor |
| 25 | No crash reporter shipped; customer crashes yield no signal | 6.5 | high | high | Phase 6.5 exists specifically for this: Sentry + PDB upload + diagnostic log affordances |
| 26 | Team skill: first-time-Windows-port dev underestimates COM/WASAPI/MSBuild learning tax | all | medium | high | Consider retaining Windows-dev consultant for 2–3d pairing; budget +30% on Phases 3/4/5/7 if maintainer is new to Win32 |
| 27 | Release-cadence parity drift: macOS ships, Windows lags | 6,7 | medium | high | CI gate refuses appcast publish for stable release unless both platform enclosures exist (M6.14); written policy in `packaging/README-release.md` |
| 28 | Bus factor: Windows shipping tied to single maintainer's box | 7 | medium | high | CI-signed release path (M7.13); `docs/WINDOWS_ARCHITECTURE.md` + `docs/WINDOWS_TROUBLESHOOTING.md` (M6.5.4); fresh-contributor test |
| 29 | GDPR/CCPA complaint from bundled telemetry without consent | 6.5 | low | high | First-run opt-in dialog (M6.5.5); runtime-configurable DSN; privacy policy update BEFORE first Windows release |
| 30 | mklink /J junction fails across NTFS volumes (dev-loop breakage on multi-drive setups) | 1 | low | low | Post-build `robocopy` as default (M1.8); junction only documented for single-volume case |
| 31 | File locking: Windows CreateFile default is share-none — Explorer refuses to move file open in Nottawa | 3 | medium | medium | Wrap CreateFileW with `FILE_SHARE_READ \| FILE_SHARE_WRITE \| FILE_SHARE_DELETE`; audit dr_libs I/O |
| 32 | Locale-dependent sprintf corrupts JSON, GLSL, appcast on non-English Windows | all | medium | medium | `setlocale(LC_ALL, "C")` + `std::locale::global(std::locale::classic())` early in WinMain; CI locale test (M6.12) |
| 33 | Second-window DPI: OutputWindow ImGui menu renders at wrong scale when dragged to differently-scaled monitor | 5 | medium | medium | Query `glfwGetWindowContentScale` on move/resize; rebuild ImGui font atlas at new scale (M5.4) |

---

## 6. Critical Path

Single-engineer sequence:

```
Phase 0 (5d) → Phase 1 (5d) → Phase 2 (5d) → Phase 3 (6d) → Phase 4 (8d)
             → Phase 5 (8d) → Phase 6.5 (3d) → Phase 7 (9d)
Cumulative: 49d critical work + 20% integration/debug buffer (10d) = ~59d
Phase 6 (13d) runs in parallel lane; not on critical path with a testing-focused engineer.
```

Wall-clock: **62 days** (baseline from max of reviewer revised estimates).

### Why this ordering

1. **Foundation first** (Phase 0). Every downstream phase assumes NOMINMAX-safe headers, no unconditional CoreFoundation, no `mach_time`, UTF-8 path helpers, COM apartment tools available. All Phase 0 changes verified against macOS Xcode build first — regressions surface immediately.
2. **Build the shell before filling it** (Phase 1). Once `nottawa.exe` launches to an empty ImGui window with seven stub implementations, every subsystem's work is validated against real linking and real launch — not conceptual mocks. Stubs are aggressively no-op (~1,500 LOC of mac code → <50 LOC per stub + one-shot warning) so integration failures show up as loud link errors.
3. **Risk-first within phases.** Phase 2 rendering before Phase 3 audio because GLSL portability failures are cross-driver and unbounded. Within Phase 3, WASAPI setup before recordingCallback wiring since broken capture invalidates every downstream audio test. Within Phase 4, encoder-ladder fallback before AAC ring buffer because open failure is the loudest failure mode.
4. **Terminal distribution.** Phase 7 requires all shipping subsystems complete AND EV cert issuance (external clock). Cert ordered day 1 (M0.11) so wallclock doesn't extend critical path.

## 7. Parallelization

### Two-engineer plan

Engineer 1 (critical path): Phase 0 → 1 → 2 → 3 → 4 → 6.5 → 7 = ~44 critical-path days + 20% buffer = ~53d wallclock.

Engineer 2 (parallel lane) idles during Phases 0+1 (Phase 0 is one person's mechanical sweep; Phase 1 is `.sln` generation + preview installer). Starts at day 10 (end of Phase 1) with Phase 5 (ancillary-services, 8d) followed by Phase 6 (testing-ci, 13d) = 21d wallclock, finishing at day 31. Engineer 2 then helps Engineer 1 shake out Phase 4 (day ~30) and Phase 7 (day ~44) integration bugs.

**Net wall-clock with two engineers: ~40 days** (vs 62d single-engineer). Savings ~35%.

### Three-engineer plan

Engineer 3 takes Phase 6 alone (13d) starting at day 10; Engineer 2 focuses on Phase 5 + supporting Engineer 1 on rendering/audio/recording debugging. **Net wall-clock: ~34 days.**

### Fine-grained parallelization within phases

- **Phase 0:** M0.5 (re-vendor dr_*), M0.7 (NOMINMAX canary), M0.8 (Utf8Path), M0.9 (license audit), M0.10 (test suite spike) all independent — can run same-day.
- **Phase 3 & 4:** Recorder can be developed against a synthetic 48 kHz sine source while Engineer 1 finishes WASAPI. Engineer 2 starts LibavRecorder in parallel with Phase 3 M3.3.
- **Phase 6:** Golden-image (M6.8), parity (M6.9), and CI wiring (M6.10) are independent; 2–3 sub-engineers can attack concurrently.
- **Phase 7:** EV cert ordering (M0.11) happens day 1 regardless of who owns Phase 7 — 3–10 business day external clock.

### Non-parallelizable

- Phase 0 → Phase 1 (Phase 1 depends on Phase 0's gated includes and Utf8Path)
- Phase 1 → Phase 2/3/4/5 (all need `.sln` to link into)
- Phase 4 → Phase 6.5 (Sentry breadcrumbs need recorder-start hooks)
- Phase 6.5 → Phase 7 M7.6 (installer smoke needs working crash reporter for real support-flow validation)
- Phase 6 M6.1 (test suite rehab) blocks the whole Phase 6 CI wiring — cannot fake around it

### Adopted sequencing changes vs original synthesis

| Reviewer | Objection | Adoption |
|---|---|---|
| Review 1 | Phase 3 lacks audio-only end-to-end test before Phase 4 | **Adopted** — M3.8 added |
| Review 1 | Phase 4 lacks unit test before scenario matrix | **Adopted** — M4.3a pulls RecorderTests forward |
| Review 1 | Phase 6 M6.1 spike must happen earlier | **Adopted** — spike moved to Phase 0 M0.10 |
| Review 2 | UTF-8/UTF-16 must be Phase 0 | **Adopted** — M0.8 added |
| Review 2 | COM apartment must be Phase 1 | **Adopted** — M1.6c added |
| Review 2 | sRGB audit must be first Phase 2 task | **Adopted** — M2.0 added |
| Review 2 | Firewall rule in Phase 7 installer | **Adopted** — M7.4 extended |
| Review 2 | SubSystem=Windows in Phase 1 | **Adopted** — M1.6b added |
| Review 3 | Preview installer at end of Phase 1 | **Adopted** — M1.10 added |
| Review 3 | Crash reporting as dedicated phase | **Adopted** — new Phase 6.5 |
| Review 3 | Parity release gate | **Adopted** — M6.14 added |
| Review 3 | Windows arch docs as Phase 7 gate | **Adopted** — M7.10 + fresh-contributor test |
| Review 3 | CI-signed release path | **Adopted** — M7.13 added |
| Review 3 | Beta ring for SmartScreen warming | **Adopted** — M7.14 added |

No reviewer objections were rejected. The plan absorbs all high-severity findings.

---

## 8. First-Week Action Items (Days 1–5)

Concrete, verifiable tasks. If you finish these on schedule, the plan is on track.

### Day 1 (Monday)

1. **Order EV code-signing certificate.** Choose SSL.com eSigner (cloud HSM, ~$300/yr) or SSL.com/DigiCert EV YubiKey (~$400–700/yr). Recommend SSL.com eSigner for future CI-signed path. Complete identity verification submission. Confirmation email is the day-1 deliverable.
2. **Fix `src/Services/ShaderChainerService.cpp:10`** — change `#include "AsciiShader.hpp"` → `#include "ASCIIShader.hpp"`. And `:122` — `16BitShader.hpp` → `16bitShader.hpp`. Land as one commit. Run macOS Xcode build to verify.
3. **Verify `dude_wake_up_tests` builds on macOS trunk (M0.10 spike).** 2-hour timebox. If broken, note in the PR description how many singleton forward-decls need adding.
4. **Rewrite `src/Util/MSATimer.h`** to use `std::chrono::steady_clock`. Delete `mach_time.h` and `machMultiplier`. Run macOS build; run a scratch test that verifies 1000 ms sleep gives 1.00 ± 0.02 seconds delta.

### Day 2 (Tuesday)

5. **Gate 7 CoreFoundation include sites** per Phase 0 M0.3. Full list: `src/main.cpp:5`, `src/Views/MainStageView.hpp:11`, `src/Views/MainStageView.cpp:10`, `src/Services/VideoSourceService.cpp:8`, `src/Engine/EngineLoop.cpp:29-30`, `src/Engine/IOSurfaceTextureManager.hpp:18`, `src/Engine/IOSurfaceTextureManager.cpp:15-17`. Grep to confirm zero unconditional CF includes remain.
6. **Fix uintptr_t/long truncation** in `src/NodeLayout/NodeTypes.hpp:20,51` and `src/NodeLayout/NodeLayoutView.cpp:349`. On macOS, click pins in the node editor to verify no regression.
7. **Delete `src/AsioTest.cpp` and `src/Audio/test_bpm_detector.cpp`** (both dead code).

### Day 3 (Wednesday)

8. **Author `src/Util/Utf8Path.hpp/.cpp`.** Provide `utf8_to_wide` / `wide_to_utf8`. Write a round-trip unit test with strings containing `日本語`, `Юрий`, `José`. Commit.
9. **Author `src/Util/PortabilityCheck.hpp`** NOMINMAX canary TU. Deliberately unset `/D NOMINMAX` in a scratch MSVC build and confirm the canary errors on `Parameter::min`.
10. **Add reminder comments** to `src/Models/Parameter.hpp` and `src/Oscillation/WaveformOscillator.hpp` about the `min`/`max` collision.

### Day 4 (Thursday)

11. **Re-vendor `src/Audio/dr_wav.h`, `dr_mp3.h`, `dr_flac.h`** from mackron/dr_libs upstream. Diff against upstream to confirm no local patches. On macOS: run `FileAudioSource` to load sample tracks and confirm no decode regression.
12. **Full third-party license audit** (M0.9). Populate `docs/THIRDPARTY-LICENSES.md` with every vendored `.h/.cpp/.dll/.lib` + license + GPLv3-compatibility check. Flag anything questionable (portmidi, Sentry, openh264).
13. **Provision Windows build box** (M1.1). Install VS 2022 Community + Desktop C++ + v143 + Win 10 SDK 10.0.22621. Install OF 0.12.1 vs release. Verify `emptyExample` builds and runs.

### Day 5 (Friday)

14. **Trim `addons.make`** (M1.2). Drop `ofxSyphon`, `ofxFFmpegRecorder`. Verify `ofxImGui`, `ofxMidi` commit-pinned to macOS matching versions.
15. **Write seven stub `.cpp` files** (M1.3). Each has full class body from shared `.hpp`, methods return safe defaults, first call logs a warning. Wrap each in `#ifndef TARGET_OSX` guard.
16. **Run OF projectGenerator** (M1.5) to produce initial `Nottawa.sln`/`.vcxproj`. Commit.
17. **Post daily standup update** to the port branch: what shipped in week 1, blockers, GATE-H (EV cert) status.

**Week 1 exit criteria:** Phase 0 milestones M0.1–M0.11 complete; Phase 1 M1.1–M1.5 complete. Macos Xcode build green from every commit. First `.sln` file exists.

---

## 9. Go/No-Go Decision Points

| Gate | Location | Trigger | If fail |
|---|---|---|---|
| **GATE-A** | End of Phase 1 (day 10) | Does `bin/Nottawa.exe` launch to an idle ImGui window on a fresh Win 11 VM with no missing-DLL popup and no crash in 5s? | After 2 iteration attempts, pivot to CMake-driven build (adds 3–5d) |
| **GATE-B** | Mid Phase 2 (day 13, after 3 shaders audited) | Are Windows GPU driver rejections limited to the 4 known sampler2DRect files, or is every driver rejecting >20% of shaders? | Escalate to `windows-gotchas` subagent for pattern catalog spike (adds 3–5d) |
| **GATE-C** | End of Phase 3 (day 20) | Does OF 0.12.1's Windows RtAudio backend expose `AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM` via `ofSoundStreamSettings`? | Windows-specific `ofSoundStream` setup for Mic + File sources (adds 2d) |
| **GATE-D** | Mid Phase 4 (day 24) | Does libav's AAC encoder + swr_convert achieve <30 ms A/V sync over 10 s recording with WASAPI audio? | Pivot to Media Foundation MFCreateSinkWriter (adds 4–6d); `recorder-research` subagent owns pivot investigation |
| **GATE-E** | Mid Phase 5 (day 35) | Does ofxImGui support a second per-window context with shared GL under OF 0.12.1? | Drop Windows OutputWindow menu entirely, rely on Ctrl+R hotkey (removes ~1d) |
| **GATE-F** | Start of Phase 6 (day 10, parallel lane) | Does existing `dude_wake_up_tests` still build on macOS trunk? (Already answered in M0.10 spike) | 2–4d for suite rehabilitation before writing new tests |
| **GATE-G** | Mid Phase 6 (day 15, parallel lane) | Will TimeService/AudioSource owners add deterministic-mode injectable clock? | Degrade `run_parity.py` to visual-similarity spot-check on 3 non-reactive shaders (removes 2d, leaves coverage gap) |
| **GATE-H** | Start of Phase 7 (day 50) | Has EV code-signing cert been issued? | Ship RC1 with OV cert accepting SmartScreen "Unknown publisher" warning; re-sign RC2+ with EV when it arrives |
| **GATE-I** | Mid Phase 7 (day 54) | Does WinSparkle's ed25519 signing produce a signature that verifies against existing `SUPublicEDKey` at `openFrameworks-Info.plist:18`? | Split into `appcast-mac.xml` + `appcast-win.xml` with separate keys (adds 1d; accepts operational split) |
| **GATE-J** | End of Phase 7 (day 62) | Do all RC-checklist items pass on both Win 10 22H2 and Win 11 23H2 VMs? | Block release; cycle back to owning subsystem for failing item |

---

## 10. Open Questions

Group by owning subsystem. Resolve before starting the corresponding phase.

**Foundation / Phase 0.**
- Does the port target x64-only or also x86? (Plan assumes x64-only.)
- Should the ffmpeg DLL set be committed to git (~40MB) or fetched by a bootstrap script? (Plan assumes fetch-via-script.)
- Is the SwiftUI-backed `Bridge/nottawa_bridge.cpp` intended to eventually build on Windows (feeding a hypothetical WinUI frontend), or strictly mac-only? (Plan assumes strictly mac-only.)

**Rendering / Phase 2.**
- Is any inter-process video sharing required on Windows (Spout for Resolume/VDMX, or NDI)? (Plan assumes no — Syphon has zero call sites on macOS anyway.)
- Which webcam devices must the Windows port support out of the box? (Integrated / USB UVC / OBS Virtual Camera / Snap Camera?) Determines whether ofDirectShowGrabber suffices or Media Foundation is required.
- Are shader files loaded from anywhere besides `./shaders/` at runtime? (User-installed shaders would mean we cannot assume every .frag has been audited.)
- Windows target minimum: Win 10 22H2? Affects `app.manifest` supportedOS GUIDs.

**Audio / Phase 3.**
- Does OF 0.12.1's Windows RtAudio backend expose `AUDCLNT_STREAMFLAGS_AUTOCONVERTPCM` via `ofSoundStreamSettings`?
- Is per-process system audio capture actually needed for any UI feature (grep says zero callers)?
- Should the WASAPI capture thread run at MMCSS "Pro Audio" priority via `AvSetMmThreadCharacteristicsW`?
- Should SystemAudioSource exclude Nottawa's own output from loopback to prevent feedback?

**Recording / Phase 4.**
- Container parity: honor `.mov` extension (real .mov, per current filename default) or force `.mp4`? Recommend forcing mp4.
- openh264 shipping: bundle Cisco's signed DLL (patent risk) vs first-run download? Needs legal call.
- Destructor's abnormal-exit `stop()` synchronous (blocking main thread up to 5s) or best-effort? Recommend synchronous.

**Ancillary / Phase 5.**
- Code-signed Windows installer planned for v1? (Confirmed yes — required for WinSparkle Option A.)
- Per-user or system-wide URL scheme registration? Recommend HKCU per-user.
- Portable ZIP variant alongside installer for LiveOps at venues without admin rights? Adds one line to `build_release.ps1`.
- `ofSystemSaveDialog` on OF 0.12.1 Windows uses native Win32 dialog or a mock? Blocks recorder start.

**Testing / Phase 6.**
- Is Catch2 (`Tests/catch_amalgamated.cpp`) used or dead? A grep of `CATCH_` in `src/` answers this.
- Should RC checklist gate release via GitHub Actions rule or stay advisory?
- Which physical GPUs will the team commit to owning for the matrix? Cloud fallback (Paperspace/Vast.ai) is workable at ~$0.50–1.50/hr.

**Distribution / Phase 7.**
- SSL.com eSigner (cloud HSM, CI-friendly) or physical YubiKey? Recommend eSigner for CI-signed path (M7.13).
- Single appcast with `<sparkle:os>` or split feeds? Recommend single-feed.
- Minimum Windows version — Win 10 1607 (LTSB) or Win 10 20H1 (when per-process audio arrived)? Recommend 1607 with graceful global-loopback fallback.
- Which ffmpeg version pin? Recorder picks it; installer bundles it. Lock exact BtbN release tag in `packaging/windows/README.md`.

---

## 11. Definition of Done (all phases)

The port is DONE when every phase's DoD is met AND:

- All 9 RC checklist items pass on both Win 10 22H2 and Win 11 23H2 VMs (including a VM with a non-ASCII username)
- Beta ring (20–50 external testers) reports no P0 issues
- Microsoft Defender submission accepted; no quarantine on test VMs with Cloud Protection enabled
- CI-signed release path publishes a release without touching maintainer's laptop
- A fresh contributor can execute a Windows release from `docs/WINDOWS_ARCHITECTURE.md` + `docs/SIGNING_RECOVERY.md` alone (verified by having someone unfamiliar attempt it)
- macOS Xcode build is green from the same commit as the shipping Windows installer
- WinSparkle upgrade from RC to GA succeeds on both Win 10 and Win 11 with user data preserved
- Simulated crash produces symbolicated Sentry stack within 5 minutes
- Appcast validator has been running for 7+ days without alert

---

## 12. Appendix: Skipped Concerns

Considered and deliberately deferred. A future maintainer reading this should understand why each is out of scope.

| Concern | Why skipped |
|---|---|
| **ofxSyphon on Windows** | No Windows equivalent that isn't a full DXGI-shared-handle port; zero in-tree callers on macOS per prior audit. Users needing inter-process video sharing get Spout as a follow-up. |
| **Per-process WASAPI loopback (Win 10 2004+)** | Zero in-tree callers of `setTargetProcess`. Global loopback is textbook and covers all current UI features. Add as follow-up sprint (~3d) only if a feature request lands. |
| **Media Foundation IMFSourceReader for webcam** | ofDirectShowGrabber is expected to cover integrated cams + USB UVC + OBS Virtual Camera. Media Foundation is documented pivot spike (~1 week) if smoke matrix fails. |
| **macOS libav port** | Bonus opportunity from the recorder work — would kill ~870 LOC of ObjC++ AVFRecorder — but out of scope for the Windows port itself. Follow-up commit. |
| **CMake unified build** | Faster path is VS-only for Windows + Xcode for macOS. CMake unification is a strategic investment worth 3–5 extra days; recommend as follow-up after v1 ships. |
| **arm64 Windows (Copilot+ PCs)** | OF 0.12.1 lacks a stock arm64-msvc target. Skip for MVP; revisit if user demand appears. |
| **MSIX / Microsoft Store distribution** | MSIX Packaging Tool can wrap the Inno-installed layout post-hoc into an `.msix` if Store submission is requested later. |
| **WinAppDriver UI test automation** | XCUITests on macOS are Swift-only and exercise the NottawaApp SwiftUI shell that has no Windows equivalent. The gtest suite + manual RC checklist covers Windows UI validation for MVP. |
| **Native Win32 menu bar for OutputWindow** | ofxImGui-in-second-window is the plan; if GATE-E fails, Ctrl+R hotkey-only is the fallback. Native `HMENU` would be more work with no user-visible benefit. |
| **WinSparkle delta updates** | WinSparkle 0.8 supports delta updates but requires additional signing ceremony. MVP ships full installer as update; deltas are a follow-up cost/benefit call. |
| **Auto-generated changelog from git history** | Nice-to-have; existing manual release-notes template is fine for MVP. |
| **In-app "Register URL Handler" repair action** | If browsers cache a stale handler association and re-registration is needed, cover via `Help > Register URL Handler` menu action in a follow-up if users report the issue. |
| **First-run downloader for openh264** | Bundling Cisco's signed DLL is Plan A. First-run downloader is contingency (~4h work) if legal blocks bundling. |
| **Deep audit of every OF shader for driver quirks beyond the 4 known** | Phase 2 M2.4 `glValidateProgram` diagnostic surfaces breakage at first-load; fix per-shader as reported. Preemptive sweep would balloon Phase 2 without evidence of need. |
| **Second-window (OutputWindow) MMCSS thread priority** | Recording thread uses `Avrt.lib` MMCSS "Pro Audio"; render thread is main OF loop and inherits OF's own scheduling. Nothing to add. |

---

**End of plan.** Address integration questions to `main`; specialized questions to `of-windows-tooling`, `recorder-research`, or `windows-gotchas` per subsystem.

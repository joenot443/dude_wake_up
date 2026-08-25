# Windows Architecture

Living document. As Windows-specific decisions get made, they land here so
future contributors can find the "why" without spelunking commit history.
See `WINDOWS_PORT_PLAN.md` for the roadmap; this file is the settled facts.

## COM apartment policy

**Main thread: uninitialized.** We do not call `CoInitializeEx` on the main
thread. OpenFrameworks does not require COM, and initializing it constrains
what other threads can do (once STA, always STA on that thread until
`CoUninitialize`; `RPC_E_CHANGED_MODE` on any conflicting later call).

**WASAPI capture thread: MTA.** `SystemAudioSource` and any other WASAPI
consumer instantiate `nottawa::ComApartmentMTA` at thread entry and hold it
for the thread's lifetime. Guard is RAII so early-return / exception paths
still balance.

**Shell / file-dialog helper threads: STA.** `ShellExecuteW`, `IFileDialog`,
and similar shell APIs are STA-affine. If we need them from a non-main
thread, wrap the call in a short-lived `nottawa::ComApartmentSTA` on a
dedicated helper thread — never on the WASAPI thread (would fight MTA).

**WinSparkle: manages its own thread.** WinSparkle 0.8+ documentation
confirms it initializes COM on its own worker thread. We do not need to
initialize COM on the main thread for it. If a future dependency ever
demands STA on main (e.g. drag-and-drop with legacy shell verbs), we must:
1. Confirm no other subsystem needs MTA on main.
2. Add `ComApartmentSTA` as the FIRST thing in `main()` before any OF init.
3. Update this doc.

**Guard implementation.** `src/Util/ComApartment.hpp`. The RAII base
distinguishes:
- `S_OK` / `S_FALSE` → we own the ref, `CoUninitialize` on destruction
- `RPC_E_CHANGED_MODE` → someone else beat us with conflicting mode; log
  and do NOT balance (would over-release their ref)

**Debug assertion.** `nottawa::assertApartment(APTTYPE_MTA, "site")` at
every COM entry point during dev catches policy drift. Compiled to a no-op
in release.

## String policy

**Internal representation: UTF-8, `std::string`.** All in-memory strings —
config keys, node names, file paths, JSON payloads — stay UTF-8 end to
end. Never store a `std::wstring` in a persistent structure.

**Boundary conversion at Win32 W-variant call sites.** Convert to/from
UTF-16 ONLY at the Win32 API boundary using
`nottawa::utf8_to_wide` / `nottawa::wide_to_utf8` from
`src/Util/Utf8Path.hpp`. Both use `MB_ERR_INVALID_CHARS` /
`WC_ERR_INVALID_CHARS` and return empty on invalid input rather than
lossy-transcoding.

**No A-variant Win32 calls.** `CreateFileA`, `MessageBoxA`, `SHFileOperationA`
etc. re-encode via the system ANSI code page and mangle non-ASCII paths.
Always use the `W` suffix and go through the Utf8Path helpers.

**MSVC source encoding.** Test fixtures must not depend on MSVC's assumed
source encoding — MSVC's default varies with system locale unless
`/utf-8` is passed. `dude_wake_up_tests/Utf8PathTests.hpp` uses
hex-escaped byte literals (`"\xE6\x97\xA5\xE6\x9C\xAC\xE8\xAA\x9E"`) so
the tests pass regardless. If the .vcxproj gains `/utf-8` we can relax
this, but until then: hex escapes only in test data.

**NOMINMAX invariant.** `src/Util/PortabilityCheck.hpp` fails the build
if `windows.h` leaks the `min`/`max` macros. The .vcxproj must define
`NOMINMAX` project-wide. If the canary fires: check
`AdditionalPreprocessorDefinitions` in the .vcxproj and any transitive
addon that might have `#undef NOMINMAX` (none as of Phase 0).

## Subsystem

**`SubSystem=Windows` (not Console).** Double-clicking `nottawa.exe` must
not spawn a console window. If OF's `main.cpp` uses standard `int main`
(not `WinMain`), set `/ENTRY:mainCRTStartup` in Linker > Advanced. Verify
by double-clicking the built exe from Explorer — no console flash.

**stdout/stderr.** With no console, `printf` and `std::cout` writes are
silently dropped. All logging goes through `ofLog*` which writes to
`ofSetLoggerChannel` — direct that to a rotating file on Windows (Phase
6.5 observability work).

## DPI awareness and long paths

**Manifest, not runtime API.** Per-monitor V2 DPI awareness is declared
in `app.manifest` at the repo root. Declaring it via
`SetProcessDpiAwarenessContext` at runtime is too late — top-level HWND
creation before the call happens at the wrong DPI. Manifest also
declares `PerMonitor` V1 as fallback for pre-1703 systems.

**Long path awareness.** Same manifest, `<longPathAware>true</longPathAware>`.
Users still need `HKLM\SYSTEM\CurrentControlSet\Control\FileSystem\LongPathsEnabled`
set (Win 10 1607+); the installer defaults to a short path under
`%LOCALAPPDATA%\Programs\Nottawa` to sidestep the requirement.

**Active code page = UTF-8.** Manifest also sets `<activeCodePage>UTF-8`
(Win 10 1903+). Makes any A-suffix Win32 call that leaks through — plus
`_mkdir`, `fopen`, etc. — treat their input as UTF-8 bytes. Older
Windows silently ignores the setting; the `Utf8Path.hpp` helpers cover
that gap by always going through the W-suffix APIs anyway.

**Manifest gets wired in via `scripts/patch-vcxproj.ps1`** into
`<Manifest><AdditionalManifestFiles>...\app.manifest</AdditionalManifestFiles></Manifest>`
on every ItemDefinitionGroup. Do NOT `#include` the manifest via
`.vcxproj`'s `<ClInclude>` — that's for headers.

## sRGB and framebuffer color space

TBD — decided in Phase 2 M2.0. Placeholder so this section exists.
Decision must precede Phase 6 golden-image baselines.

## ffmpeg licensing

**LGPL-only, no --enable-gpl.** GPLv3 project cannot ship GPL-licensed
ffmpeg (would GPL-taint ffmpeg's own components that were LGPL upstream —
and more importantly, obligate GPL source availability for the whole
distribution). Fetch script (`scripts/fetch_ffmpeg_lgpl.ps1`) pins to a
known LGPL BtbN build with SHA256 verification. Runtime belt-and-braces:
`LibavRecorder::verifyLgplOnce_` queries `avutil_configuration()` on
first setup and refuses to initialize if the string contains
`--enable-gpl`. See `src/Video/Recording/LibavRecorder.cpp`.

**Version pin.** BtbN dated releases embed the git hash in every asset
name, so the URL changes daily; only the `latest` rolling tag has
stable-named assets. Fetch script defaults to
`ffmpeg-n8.1-latest-win64-lgpl-shared-8.1.zip` from the `latest` tag
(shipping the current 8.1 release-branch build, currently 8.1.2). The
port plan originally called for 7.1.x; 8.1 carries the same modern APIs
we target (`AVChannelLayout`, `swr_alloc_set_opts2`, per-frame
`ch_layout`) so no code changes needed. The SHA pin — not the tag — is
what proves reproducibility.

## Recorder (LibavRecorder)

**Public surface stays `AVFRecorderWrapper`.** Windows re-uses
`recorderInstance` as a `LibavRecorder*`. macOS still delegates to the
Objective-C `AVFRecorder` class. Callers (OutputWindow, ShareStrandView)
never see libav headers.

**Thread model.**
- `setup` / `setupAudio` / `start` / `stop`: caller thread (main).
- `addFrame`: caller thread (main draw loop).
- `addAudioSamples`: audio thread (WASAPI / RtAudio callback).

**Concurrency.** `writeMutex_` serializes every `av_interleaved_write_frame`
call (per libav muxer thread-safety contract — the muxer is not
reentrant). `audioMutex_` guards `audioTail_` + `swrCtx_` so audio
callbacks can't race a finalize-time tail flush. Video-encoder state
(`videoCodecCtx_`, `swsCtx_`, `videoFrame_`) is main-thread-only and
needs no additional lock. Audio-encoder state (`audioCodecCtx_`,
`audioFrame_`) is audio-thread-only aside from the finalize flush,
which happens under `audioMutex_` when the audio thread has already
drained.

**Video PTS: wall-clock derived.** `steady_clock::now() - startTime_`
rescaled into the video stream's `time_base`. NOT a frame counter — a
shader-load hitch in the main draw loop would otherwise cause the
counter clock to drift behind real time, and audio would appear to
lead video in the final mux.

**Audio PTS: sample-count derived.** Cumulative encoded output samples,
starting from zero at `start()`. Deterministic and drift-free as long
as the source keeps producing at the declared sample rate. If WASAPI
drops buffers, audio track will be shorter than video track — an
acceptable failure mode (VLC plays through gaps).

**Single-fire completion.** `completed_` atomic `exchange(true)` gates
the finalize path. `stop()`, duration-limit expiry inside `addFrame()`,
and the destructor all funnel through `finalizeIfLive_` — only the
first caller runs the moov trailer + fires `onRecordingComplete`.

**Fragmented mp4.** `movflags=frag_keyframe+empty_moov+default_base_moof`
so files stay playable after abnormal exit (crash, kill, power loss).
Every keyframe seals a fragment; player picks up wherever the last
complete fragment lands.

**Encoder ladder.** `h264_nvenc → h264_qsv → h264_amf → libopenh264`.
First one that `avcodec_open2` accepts wins. Logs which encoder
opened on `setup()` success. libopenh264 is the portable LGPL software
fallback — no hardware acceleration but always works.

**Unicode output paths.** Same policy as `ofxAudioFile` (see below):
we rely on the manifest's `activeCodePage=UTF-8` for libavformat's
file protocol to transcode our UTF-8 `std::string` correctly into
`CreateFileW`. No custom AVIOContext needed under our
`MinVersion=10.0.19041` gate.

## Update mechanism

TBD — Phase 5. Options: WinSparkle (parallels macOS Sparkle appcast
format), or full stub for v1 with an in-app "check for updates" link to
the release page. Decision blocks EV cert use for signed installers.

## URL scheme handling

TBD — Phase 5. Requires:
- Registry entry under `HKCU\Software\Classes\nottawa\shell\open\command`
  (per-user, installer or first-run) with `"%1"` argv passthrough
- Argv parsing in `main()` before OF init so cold-launch URLs are routed
- Named-pipe forwarding for single-instance warm-launch (avoid spawning a
  second `nottawa.exe`; forward URL to the running instance)

## Audio thread and recordingCallback

**`AudioSource::audioIn()` is called on the audio capture thread.** Which
thread that is depends on the source:
- `MicrophoneAudioSource` — OF's RtAudio backend worker (WASAPI callback
  on Windows)
- `FileAudioSource` — OF's audio output callback (also RtAudio worker)
- `SystemAudioSource` — our own thread in `SystemAudioSource_win.cpp`
  (WASAPI loopback), MTA-initialized via `ComApartmentMTA`

**`AudioSource::setRecordingCallback`** takes the caller's `std::function`
under `recordingMutex`, then `audioIn()` invokes it on the audio thread.
`recordingMutex` is held ONLY across the copy of the callback pointer and
its invocation — no downstream work runs under it. This keeps the mutex
window microseconds long, which matters because the audio thread is
running at MMCSS "Pro Audio" priority and can't tolerate long stalls.

**Global lock ordering:** the recorder path (Phase 4 `LibavRecorder`)
must **NEVER acquire `recordingMutex`.** The audio thread already holds
it when it calls into the recorder; the recorder holds its own
`writeMutex` internally. If the recorder ever needs to reach back into
the AudioSource to query state, it must do so via a lock-free accessor
(atomic loads, thread-local caches) — not by acquiring `recordingMutex`.
Violating this order can deadlock (audio thread holds `recordingMutex`
waiting for `writeMutex`; recording UI thread holds `writeMutex` waiting
for `recordingMutex`).

**Silence gate does NOT gate the callback.** `AudioSource::audioIn`
runs the recording callback BEFORE the amplitude-based silence check,
so recordings include silent passages verbatim rather than skipping to
the next non-silent chunk. If we ever change that, update the recorder
too — its wall-clock PTS assumes samples arrive contiguously.

## Audio file I/O with Unicode paths

`ofxAudioFile` fans out to four decoders — dr_wav, dr_mp3, dr_flac,
stb_vorbis — and all four in the vendored copy under `lib/` open files
with `char*` paths and CRT `fopen`. Only dr_wav has `_w` (wide-char)
variants; the other three do not.

**Policy: rely on the manifest's `activeCodePage=UTF-8`** (Win 10 1903+).
With it, MSVC's `fopen("日本語.mp3", "rb")` calls `GetACP()`, gets
65001, and correctly transcodes UTF-8 bytes to UTF-16 for the actual
`CreateFileW` under the hood. Our Inno Setup `MinVersion=10.0.19041`
(Win 10 2004) already gates us above 1903 → this works.

**Fallback if a future user reports Unicode paths failing:** upgrade
dr_mp3 / dr_flac to a recent upstream (both added `_init_file_w` after
we vendored) and patch `ofxAudioFile.cpp` to prefer the `_w` variants
on Windows. stb_vorbis has no wide-char API — for that one, load into
memory via `_wfopen_s` + fread + `stb_vorbis_decode_memory`.

Do NOT go down that path pre-emptively — the manifest route works and
touches zero vendored library code.

## What we deliberately do NOT do on Windows

- **Sandbox / bookmark persistence.** `BookmarkService` is a permanent
  no-op on Windows (`src/Services/BookmarkService_win.cpp`). Windows apps
  aren't sandboxed by default. Full removal of the .hpp and its call
  sites is a Phase 5 cleanup task.
- **Syphon.** Video sharing between apps was macOS-only via `ofxSyphon`
  (dropped from `addons.make` in Phase 0). Windows equivalent is Spout;
  not in scope for v1.
- **AVFoundation recording.** `AVFRecorder` / `AVFRecorderWrapper` are
  macOS-only. Windows recording goes through libav in Phase 4.

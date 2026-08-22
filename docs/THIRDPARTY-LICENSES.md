# Third-Party License Audit — Nottawa

Nottawa itself is licensed **GPLv3** (per `README.md` — note the `LICENSE` file at the repo root is *not* the app license; it is the Sparkle framework's MIT license and needs to be renamed or supplemented, see **Follow-ups** below).

Every third-party component shipped or linked with the binary must be **GPLv3-compatible**. The FSF categorizes licenses at <https://www.gnu.org/licenses/license-list.html>. Compatibility summary applied throughout this document:

| License | Compatible with GPLv3? | Notes |
|---|---|---|
| Public Domain / Unlicense / MIT-0 | ✅ | No conditions to worry about |
| MIT / BSD-2 / BSD-3 / ISC / Zlib | ✅ | Attribution notice must be preserved |
| Apache 2.0 | ✅ (with GPLv3 only, not GPLv2) | Patent grant clause |
| LGPL 2.1+ / LGPL 3 | ✅ if **dynamically** linked | Static linking upgrades combined work to LGPL/GPL terms |
| MPL 2.0 | ✅ | File-level copyleft |
| GPLv2 only (no "or later") | ❌ | Not compatible with GPLv3 |
| GPLv2+ / GPLv3 / AGPL | ✅ | Copyleft transitive |
| APSL 2.0 | ⚠️ Free software per FSF, **but not GPL-compatible** | Would need to be avoided or removed |

## Currently vendored (source code in this repo)

| Library | Version | Location | Source | License | GPLv3-Compatible? | Notes |
|---|---|---|---|---|---|---|
| **BTrack** | 1.0.4 | `lib/BTrack/` | github.com/adamstark/BTrack | **GPLv3+** | ✅ | Same author as Gist; app is GPLv3 so no issue |
| **Gist** | vendored (undated) | `lib/gist/`, `src/Audio/gist/` | github.com/adamstark/Gist | **GPLv3+** | ✅ | Duplicated between `lib/` and `src/`; consolidate later |
| **kiss_fft** | 1.3.0 | `lib/kiss_fft130/` (also `lib/BTrack/libs/kiss_fft130/`) | github.com/mborgerding/kissfft | **BSD 3-Clause** | ✅ | Duplicated; harmless |
| **nlohmann/json** | 3.11.3 | `lib/json.hpp`, `src/Util/json.hpp` | github.com/nlohmann/json | **MIT** | ✅ | Duplicated between `lib/` and `src/` — pick one to keep |
| **cpp-httplib** | 0.12.1 | `src/Util/httplib.h` | github.com/yhirose/cpp-httplib | **MIT** | ✅ | **Vendored copy needs Windows-guards re-vendoring** (Phase 0 M0.2, day 4) |
| **stb_vorbis** | 1.17 | `lib/stb_vorbis.h`, `src/Audio/stb_vorbis.h` | github.com/nothings/stb | **Public Domain / MIT** (dual) | ✅ | Duplicated; harmless |
| **dr_wav** | 0.10.1 (2019-08-31) | `src/Audio/dr_wav.h` | github.com/mackron/dr_libs | **Public Domain / MIT-0** (dual) | ✅ | **STALE — upstream is v0.13+**; re-vendor per Phase 0 M0.2 |
| **dr_mp3** | 0.4.7 (2019-07-28) | `src/Audio/dr_mp3.h` | github.com/mackron/dr_libs | **Public Domain / MIT-0** (dual) | ✅ | **STALE**; re-vendor |
| **dr_flac** | 0.12.1 (2019-09-29) | `src/Audio/dr_flac.h` | github.com/mackron/dr_libs | **Public Domain / MIT-0** (dual) | ✅ | **STALE**; re-vendor |
| **ofxAudioFile** | vendored, no version | `src/Audio/ofxAudioFile.{h,cpp}` | github.com/npisanti/ofxAudioFile | **MIT** (per upstream README) | ✅ | License header missing from the vendored files — upstream is MIT |
| **imgui_node_editor** | 0.9.1 | `src/NodeLayout/node-editor/` | github.com/thedmd/imgui-node-editor | **Public Domain / permissive** (dual per file header) | ✅ | Plan v1 said Apache 2.0 — vendored version is actually public-domain-dual |
| **imgui_markdown** | vendored | `lib/imgui_markdown.h`, `src/Util/imgui_markdown.h` | github.com/juliettef/imgui_markdown | **zlib** | ✅ | Duplicated |
| **imgui_stdlib** | v0.10-based | `lib/imgui_stdlib.{cpp,h}`, `src/Util/imgui_stdlib.h` | Dear ImGui `misc/cpp/` | **MIT** (part of Dear ImGui) | ✅ | Local wrapper `.hpp` is project code, `.cpp` is upstream Dear ImGui |
| **implot** | vendored | `lib/implot/` | github.com/epezent/implot | **MIT** | ✅ | LICENSE file absent locally; MIT confirmed in `implot.h` header |
| **dear_widgets** | vendored | `lib/dear_widgets.{cpp,h,hpp}`, `src/Util/dear_widgets.{h,hpp}` | github.com/soufianekhiat/DearWidgets | **⚠️ MIT (per upstream repo)** — **LICENSE HEADER MISSING FROM VENDORED FILES** | ✅ | See **Follow-ups** |
| **ofxFastFboReader** | vendored | `lib/ofxFastFboReader.{h,cpp}` | github.com/satoruhiga/ofxFastFboReader | **MIT** (per upstream README) | ✅ | License notice missing from vendored files |
| **MSATimer / MSABPMTapper** | vendored | `src/Util/MSATimer.h`, `src/Util/MSABPMTapper.{h,cpp}` | github.com/memo/msalibs | **MIT** | ✅ | From Memo Akten's msalibs |
| **base64.h** | 2003 Apple version | `src/Util/base64.h` | Apple (via APSL 2.0) | **APSL 2.0** | ❌ **Not GPL-compatible per FSF** | **UNUSED** — grep found zero call sites in live code. **Recommend deletion** (see Follow-ups) |

### macOS-only vendored (won't ship on Windows, listed for completeness)

| Library | Location | License | Notes |
|---|---|---|---|
| **Sparkle.framework** | `lib/Sparkle.framework/` | MIT + component licenses (bsdiff, sais-lite, etc.) | macOS auto-updater. Windows replacement will be WinSparkle (see Planned) |
| **Syphon** | `lib/Syphon/` | BSD-2-Clause (per syphon.v002.info/license) | macOS-only inter-app GL sharing |
| **ofxSyphon** | `lib/ofxSyphon/` | Same as Syphon | Listed in `addons.make` but **not called from `src/`** (dead per prior audit). Also referenced in `src/buildRunMain.sh` / `buildRunTest.sh` include paths — remove those lines when trimming |
| **ofxVideoRecorder** | `lib/ofxVideoRecorder/` | MIT (per upstream github.com/timscaffidi/ofxVideoRecorder) | Not currently called from `src/` — appears dead; the AVFoundation recorder is the active path |
| **MusicBeatDetector** | `lib/MusicBeatDetector/` | **DIRECTORY IS EMPTY** — no files present | Ghost entry; safe to `rmdir` |

## OpenFrameworks addons (from `addons.make`)

| Addon | Live? | License | Notes |
|---|---|---|---|
| **ofxImGui** | ✅ | MIT (jvcleave fork of Dear ImGui) | Cross-platform |
| **ofxMidi** | ✅ | MIT (wraps RtMidi) | Cross-platform; on Windows uses WinMM backend |
| **ofxSyphon** | ❌ dead | BSD-2 | Remove from `addons.make` on Windows build |
| **ofxFastFboReader** | ✅ (vendored into `lib/`, not from addon path) | MIT | See above |
| **ofxFFmpegRecorder** | ❌ dead | MIT | Listed but no call sites in `src/`. Being replaced by direct libav wrapper on Windows anyway |

## External libraries linked at build time (not vendored in tree)

| Library | Where used | License | GPLv3-Compatible? | Notes |
|---|---|---|---|---|
| **OpenFrameworks 0.12.x** | app framework | MIT | ✅ | Ships as separate install; not in this repo |
| **Ableton Link** | `#include <ableton/Link.hpp>` from NodeLayout, Strand, MainStageView | **GPLv2+ OR commercial** | ✅ (via GPLv2+) | GPLv2 "or later" is upgradable to GPLv3; app can use the free-software variant |
| **Dear ImGui** | via ofxImGui addon | MIT | ✅ | Backing library for ofxImGui |

## Planned Windows-only additions (must be catalogued before ship)

| Library | Purpose | Version target | License | GPLv3-Compatible? | Notes |
|---|---|---|---|---|---|
| **ffmpeg (libavformat, libavcodec, libavutil, swscale, swresample)** | Video recorder replacement for AVFoundation | 7.x LTS | **LGPL 2.1+** *only* — build with `--disable-gpl` | ✅ if dynamically linked and no `--enable-gpl` components | Ships as DLLs alongside `Nottawa.exe`. **Do NOT `--enable-gpl`** — that would pull in libx264 as GPL and upgrade the combined work to GPL; irrelevant since Nottawa is already GPLv3, but stay clean-line to keep the recorder library LGPL-only |
| **libx264** (used only if enabled in ffmpeg) | H.264 software encoder | latest | **GPLv2+** | ✅ (via GPLv2+) | Only include if the LGPL-only AAC/H.264 path isn't sufficient. If used, add explicit GPLv2 credit |
| **openh264** (Cisco H.264) | H.264 fallback | 2.4+ | **BSD 2-Clause** + Cisco patent-royalty grant | ✅ | Alternative to libx264 if licensing simplicity wanted; Cisco pays MPEG-LA royalties for binary DLL |
| **WinSparkle** | Windows auto-updater | 0.9.x | **MIT** (with LGPL wxWidgets *removed* in current WinSparkle; verify at time of import) | ✅ | Replacement for macOS Sparkle. Confirm the vendored version does not pull in copyleft transitively |
| **DXGI / D3D11** | IOSurface replacement for texture sharing | Windows SDK | Windows SDK license (Microsoft EULA — royalty-free) | ✅ | Ships with the Win10/11 SDK; no redistribution issue |
| **WASAPI / audioclientactivationparams** | System audio capture | Windows SDK | Windows SDK license | ✅ | Same |
| **Visual C++ Runtime redist (vcredist_x64.exe)** | C++ runtime | 2015-2022 | Microsoft redistributable | ✅ | Bundled by installer |

## FLAGGED items requiring follow-up

Ordered by severity.

### 🔴 Critical (block release)

1. **`LICENSE` at repo root is Sparkle's license, not the app's.** The file lists Andy Matuschak, Elgato, Kornel Lesiński, Mayur Pawashe, etc. — this is `lib/Sparkle.framework/LICENSE` misplaced at root. `README.md` says the app is GPLv3 but there is no GPLv3 text at root. **Action:** move Sparkle's license to `docs/licenses/SPARKLE-LICENSE.txt` and put the actual GPLv3 text at `LICENSE`.
2. **`src/Util/base64.h` is APSL 2.0** — not GPLv3-compatible. **UNUSED** in current codebase (grep found no live call sites; only commented-out references in `src/Network/LibraryService.cpp`). **Action:** delete the file. `httplib.h` provides `httplib::detail::base64_encode` if base64 is ever needed.

### 🟠 High (fix before Windows RC)

3. **Missing license headers in vendored files:** `dear_widgets.{cpp,h,hpp}`, `ofxAudioFile.{h,cpp}`, `ofxFastFboReader.{h,cpp}`, `implot/*`. Upstream sources all confirm MIT-compatible licenses, but the vendored copies do not carry the notice text. **Action:** add SPDX-License-Identifier headers or a per-file NOTICE line pointing at `docs/THIRDPARTY-LICENSES.md`.
4. **Stale `dr_wav`/`dr_mp3`/`dr_flac`** (2019 versions) — day-4 re-vendor per plan. Licensing unchanged (still MIT-0/PD dual) but the stripped Windows guards need restoring by pulling from upstream.
5. **Duplicated vendored files** across `lib/` and `src/`: `json.hpp`, `stb_vorbis.h`, `gist/`, `dear_widgets.h`, `imgui_markdown.h`. Not a license issue but a maintenance hazard — someone will patch one copy and not the other. **Action:** pick one canonical location per file and delete the other; the day-4 `dr_libs` re-vendoring is a good time to enforce this.

### 🟡 Medium (verify before adding)

6. **WinSparkle**: confirm the specific tagged version has no LGPL/GPL transitive dependencies (older versions bundled wxWidgets; newer versions do not). Record the version and its dependency tree in this document when it lands.
7. **ffmpeg build config**: whoever produces the Windows ffmpeg DLLs must include a written build recipe here — the exact `./configure` line with `--disable-gpl --enable-shared --disable-static`. Ideally re-use a known-clean packaging (e.g., BtbN's LGPL builds).
8. **`ofxSyphon` references in `src/buildRunMain.sh` and `src/buildRunTest.sh`** — remove the `-I ../../../addons/ofxSyphon/{libs,src}` include lines when the addon is dropped for the Windows build. Not a license issue, but they'll cause include-path warnings on Windows.

### 🟢 Low (cleanup, not blocking)

9. **`lib/MusicBeatDetector/` is empty** — safe to `rmdir`.
10. **Sentry SDK** was flagged in the plan as potentially problematic — **not currently a dependency**. `src/main.cpp`'s `sentryPath` is just a filesystem path for crash-dump storage; there is no `#include <sentry.h>` anywhere. If Sentry is added in Phase 6.5 (observability), come back and populate this document. Sentry native SDK is MIT — fine either way.
11. **portmidi** flagged in plan — **not present** in this repo. Nothing to audit.

## "Verify before ship" checklist

- [ ] Repo root `LICENSE` file contains actual GPLv3 text (currently contains Sparkle's MIT)
- [ ] `src/Util/base64.h` deleted (APSL 2.0 collision)
- [ ] `dr_wav`/`dr_mp3`/`dr_flac` re-vendored from upstream mackron/dr_libs
- [ ] ffmpeg Windows DLLs are LGPL-only build (`--disable-gpl` confirmed, no libx264-GPL, no ffmpeg-full)
- [ ] Installer bundles a `licenses/` directory containing:
  - GPLv3 (app)
  - MIT (OpenFrameworks, Dear ImGui, ofxImGui, ofxMidi, imgui_node_editor, implot, dear_widgets, ofxAudioFile, ofxFastFboReader, MSA, cpp-httplib, nlohmann/json, WinSparkle, ofxVideoRecorder)
  - zlib (imgui_markdown)
  - BSD-3 (kiss_fft)
  - BSD-2 (Syphon, openh264)
  - Public domain / MIT-0 (dr_libs, stb_vorbis)
  - LGPL 2.1 (ffmpeg — required by LGPL if you ship the LGPL DLLs)
  - GPLv2+ (BTrack, Gist, Ableton Link)
- [ ] "Third-Party Notices" menu item or About-box link in the app pointing at the bundled `licenses/` directory
- [ ] Windows installer displays or references the app's GPLv3 license on first run
- [ ] WinSparkle version pinned and licensing confirmed
- [ ] No new dependency added without a corresponding row added to this document

---

**Audit produced:** Phase 0 M0.9, Day 4 of the Windows port plan. Update this file whenever adding, removing, upgrading, or reconfiguring a third-party component.

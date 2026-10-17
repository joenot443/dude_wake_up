# Nottawa 2.0 — App Store Release Notes

## "What's New" text (paste into App Store Connect)

> Version 2.0 — Complete Redesign
>
> Nottawa 2.0 is a top-to-bottom rebuild. New look, new editor, new performance tools — all backed by the same engine you know.
>
> BRAND-NEW INTERFACE
> • Redesigned node editor with cleaner pin connections and full keyboard navigation
> • Five built-in themes — Midnight, Daylight, Ember, Arctic, Forest
> • Resizable sidebar and inspector panels
> • Live preview windows you can pop out and place anywhere
>
> STAGE MODE
> Built for live performance. Solo any node, pin a permanent output, and monitor multiple stages at once without ever losing track of what's on screen.
>
> SHARPER PARAMETERS
> • Inline editing — start typing a value on any selected slider
> • Tab between parameters with the keyboard
> • Audio-reactive oscillators now run reliably across every shader (fixed a long-standing bug where toggling them on did nothing)
> • Copy and paste parameter groups between shaders
> • Mark favorites for instant access
>
> SCREENSHOT CAPTURE
> Press S in any preview window to save the frame instantly, ⇧S to pick a destination folder, or C to copy to clipboard. Configure your save folder in Settings → Screenshots.
>
> NEW & POLISHED SHADERS
> Library expansion with new effects and refined classics. Sliding Frame now ping-pongs smoothly between source and target. Triple shader has readable parameter names. Audio Glow Bars, Gameboy, Reflector, StrangeScreen and Stroke all touched up.
>
> WORKFLOW UPGRADES
> • Right-click context menu on every node (delete, break connections, blend, edit shader file)
> • Auto-positioning for new nodes — no more piles
> • Cmd+Click for multi-select
> • Drag a source straight onto an existing node to chain
> • Swap a node's shader in place without rewiring
> • Keyboard shortcuts for everything: Cmd+Z/Shift+Z, Space (play/pause), Delete, Escape
>
> UNDER THE HOOD
> Native Metal rendering, IOSurface-based texture sharing, redesigned engine thread for solid 60fps on Apple Silicon.
>
> Existing Nottawa configs from earlier versions are automatically migrated.
>
> Thanks for using Nottawa. Tap the ? button anytime for a quick tour.

---

## Internal change log (commit summary since v1.0.1 / build 30)

### Major
- Complete SwiftUI frontend rebuild (replaces ImGui frontend in 2.0)
- New design system: Theme.swift, ThemeManager, 5 presets (Midnight/Daylight/Ember/Arctic/Forest)
- Stage Mode for live performance (StageOutputView, StageModeView)
- Live preview windows (PreviewWindowManager, separate NSWindows)
- New shader editor canvas with dot-in-ring pins, multi-select, context menu
- Sidebar browser with shaders + sources, drag-to-canvas, drop-on-node chaining
- Inspector panel with parameter favorites, copy/paste, MIDI bindings

### New features this cycle
- Screenshot capture (`ScreenshotService.swift`) — `S` quick save, `⇧S` picker, `C` clipboard. Sandbox-friendly via security-scoped bookmark to user-picked folder.
- Help overlay (HelpOverlayView, HelpGuideViewModel)
- Audio panel with FrequencyColumnView
- Dynamic resolution
- Collapsible left panel
- BlendShader alpha swap
- Volume / playback controls for FileSource and LibrarySource

### Bug fixes
- **Oscillators not modulating across all shaders** — root cause was bridge bypassing `OscillationService::setEnabled()` so the `enabledOscillators_` hot list never picked up SwiftUI toggles or loaded saved configs. Fix in `nottawa_bridge.cpp:1155` + `OscillationService::addOscillator` now honors pre-enabled bool.
- **Sliding frame snap-reset** — now ping-pongs L→R with textures' roles swapping each cycle. Eliminates the visible reset moment.
- **Triple shader oscillators not visible** — same root cause as #1 above; renamed params to user-facing Capital Case (Scale, X Shift, Y Shift, Show Original/Left/Center/Right).
- **NottawaApp screenshot UX** — old flow always opened modal save panel; new flow saves silently after one-time folder pick.
- **Various** shader fixes: AudioGlowBars, Gameboy, Reflector, StrangeScreen, Stroke

### Sparkle removal
- v2.0 distributes exclusively through the Mac App Store. Sparkle.framework, SU* Info.plist keys, `SparkleUpdaterService` initialization, and the framework copy build script have all been removed. The `SparkleUpdater.{h,mm}` source files are retained on disk but no longer in any Sources or Frameworks build phase.

### Engine & infrastructure
- Phase 1-3 SwiftUI frontend rebuild (Engine extraction, C bridge, IOSurface texture sharing, node editor, parameter panels)
- Phases 1-9 feature-parity work (multi-select, context menu, blend creation, live preview, in-node shader swapping, preview windows, pin tooltips, save strand, sidebar browser, resizable panels)
- Native Metal rendering via IOSurface → MTLTexture
- Detached engine thread with std::promise/future dispatch
- GL context ownership transfer between main and engine threads
- 60fps update loop with time-based auto-save (replacing frame-counter-based)
- ConfigService sandbox container migration

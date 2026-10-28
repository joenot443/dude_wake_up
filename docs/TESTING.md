# Testing

Why this exists: a Blend that rendered nothing (an engine change made a
buffer lazy; Blend assumed it was there) went unnoticed for days on both
platforms, because no test looked at what the app draws. The tests below
check rendered pixels, and run before every push.

## Run them

```
powershell -File Tests\run_tests.ps1                 # build, then the suite (~10 min)
powershell -File Tests\run_tests.ps1 -SkipBuild      # the suite on the last test build
powershell -File Tests\run_tests.ps1 -Golden record  # record the golden images again
```

The build goes to `app\build\test`, so a running Nottawa isn't closed. The
suite runs in the built app with throwaway app data. It fails on any test
failure, a crash or a UI hang (> 2 s). Logs: `%TEMP%\nottawa-tests`.

**Before every push**: `.git/hooks/pre-push` runs it (install once per
clone: `Tests\hooks\install.ps1`). A failure stops the push.
`git push --no-verify` skips it; don't, unless the failure is understood.

## What's in it

All in `app/nottawa_app/StressTest.cpp`, driven through the engine bridge:

- **golden**: every effect (fed by Plasma) and every generated source,
  rendered deterministically and compared with `Tests/golden/windows/*.png`.
  Deterministic: the clock frozen (`ntw_debug_freeze_time`), step mode with
  exactly 8 frames (`ntw_debug_render_frames`, which also seeds ofRandom),
  256x144, read back from the engine (`ntw_debug_frame_rgba`), compared at
  128x72. A pixel "differs" when a channel is more than 24 apart; an image
  fails when over 1% of pixels differ or the mean difference is over 2.
  Entries that render differently twice in a row or between launches
  (effects reading the system clock, say) are in `unstable.txt` and are
  only checked for rendering something. A failure writes
  `golden-<name>.png` (expected | actual) to the scratch folder.
- **workflows**: what users do, checked by pixels: a chain renders at every
  stage; an effect changes its input; an optional effect on changes the
  output, off restores it; Mask keeps bright parts and a second input
  fills the rest; a workspace saved and reopened
  renders the same; 1080p frames are 1920x1080; undo removes an added
  effect and redo restores it.
- **blenddrag**: a Blend made through the engine and by dragging one output
  port onto another on the canvas (mouse events); its wiring; its frame
  isn't empty.
- **settings / settingsui**: the shared settings and the settings window.
- **churn, connect, undo, swap, copypaste, optional, strands, workspace,
  previews**: the stress scenarios (graph changes, crashes, leaks).

Not in the pre-push suite (they play audio aloud or need media files):
bpm, audiosettings, beattiming, inputs, cachesettings; run them with the
stress driver when touching those areas.

## Known issues

Listed in the scenario (`known` in `workflows()`), reported as KNOWN and
not failing the run; one that starts passing is reported as KNOWN-FIXED, so
it can come off the list.

- DirtyPlasma and Glitch RGB render differently on the first launch after a
  build (the same image every time), then match their golden images. Both
  sample TextureService textures. In `unstable.txt` until understood.
- Text inputs don't draw in the deterministic test mode (step mode): their
  frame stays uninitialised. Not yet checked in normal use.

## When a golden test fails

1. Look at `golden-<name>.png` in the scratch folder (expected | actual).
2. If the change was intended (an effect was changed on purpose), record
   again (`-Golden record`), look at the changed images in git, commit them.
3. If not, it's a regression: fix it.

## Rules

- A change to something other code relies on (allocation, order, defaults,
  lifetime) comes with a check of every user and a test.
- New effects and sources are covered by golden automatically (record to
  add their image).
- The macOS app has the same tests in its self-test (NottawaApp
  SelfTest.swift), with its own images in `Tests/golden/macos` (GPUs and
  drivers differ, so images aren't shared between platforms).

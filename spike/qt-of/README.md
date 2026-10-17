# Nottawa Qt UI spike

Go/no-go on Qt 6 as the Windows native UI framework. **Result: pass**: Qt is
the framework for the Windows UI rebuild (`app/nottawa_app/`).

## Layout

```
spike/qt-of/
├── hello/         Qt 6.8 + MSVC 2022 toolchain check, bare window
├── engine-fbo/    dual-context shared texture, 1080p plasma
└── build/         CMake build dir (gitignored)
```

## What the spike established

A background `QThread` owns a `QOpenGLContext` created with
`setShareContext(QOpenGLContext::globalShareContext())` and renders into an
FBO; the UI's `QOpenGLWidget` binds that FBO's texture directly. Texture ids
are valid across the share group, so no copy and no GL↔D3D interop is needed.

## Correction to the original results

The figures first recorded here (64.3 fps, p99 32 ms, "clean shutdown") were
not valid:

- The frame limiter measured the interval *including* the previous frame's
  sleep, so pacing alternated between sleeping and not sleeping; the fps
  numbers reflect that bug, not rendering throughput.
- `glFlush()` returns before the GPU finishes, so no GPU time was measured.
- Frames were handed over with no fence or buffering. `glFlush()` does
  **not** make one context's rendering visible to another; the GL spec
  requires a fence (`glFenceSync` + `glWaitSync`) or `glFinish()`.
- Every run ended with a force-kill, so shutdown was never tested.

`engine-fbo/` is kept as the record of the context-sharing experiment and
still has these flaws. The production path fixes them with a fenced triple
buffer (`src/Engine/FrameExchange`), now used by the Windows
`IOSurfaceTextureManager` to hand engine output to the Qt app.

## Measured results (2026-10-17)

Measured with the stand-in renderer that `app/nottawa_app` used before it
moved to the real engine via `nottawa_bridge.h`; it rendered single Nottawa
shaders through the same context sharing and handoff.

RTX 3070 (driver 581.57), Qt 6.8.0, MSVC 2022, 1920×1080, engine thread at
highest priority, 5-minute soak of `FractalAudio`, the heaviest shader in
the library (benchmark of all 193 below):

| Metric | Result |
|---|---|
| Engine rate | 60.00 fps average, 59.3 worst 1 s window |
| Frame intervals > 33 ms | 0 |
| Worst frame interval | ~31 ms (rare scheduler stalls; see note) |
| GPU time per frame | 7.09 ms average, 9.84 ms worst p99 |
| New frames shown by the UI | ~59/s (engine and display clocks are not phase-locked) |
| PCIe traffic | ~17 MB/s rendering vs 0–2 MB/s idle; a per-frame CPU copy would be ~500 MB/s each way |
| Memory / handles / threads | flat (166 MB working set, ~1700 handles, 61–62 threads) |
| Shutdown | normal window close, exit code 0 |

Note: at normal thread priority, 8% of one-second windows contained a frame
interval over 25 ms; at highest priority, 3%. The remainder is Windows sleep
jitter. A high-resolution waitable timer, or pacing off the display's vsync,
would tighten it further.

`NOTTAWA_BENCH_ALL=1` ran every shader for 1.5 s: 188 compiled and ran; the
heaviest averaged 7.1 ms of GPU time. Five fail to compile on this driver
because of bugs in the GLSL source (leftover Shadertoy uniforms, undefined
`outputColor`, `main` with parameters): `4DBeats`, `CubeGlow`, `Diffusion`,
`Hilbert`, `SpectrumClash`. The last three are registered in `ShaderType`.

## Fixes hit during the spike

1. `QOpenGLWidget::show()` can run `initializeGL` synchronously, so connect to
   `glReady` before calling `show()`.
2. `QOffscreenSurface` must be created on the GUI thread.

## Build / run

```powershell
pip install aqtinstall
python -m aqt install-qt windows desktop 6.8.0 win64_msvc2022_64 --outputdir C:\Qt

$cmake = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
& $cmake -S spike\qt-of\engine-fbo -B spike\qt-of\build\engine-fbo -G "Visual Studio 17 2022" -A x64
& $cmake --build spike\qt-of\build\engine-fbo --config Release
```

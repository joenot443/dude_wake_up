# Nottawa Qt UI spike

Three-day go/no-go on Qt 6 as the Windows native UI framework. **Result: PASS.** Qt is confirmed as the production framework for the Windows-native UI rebuild.

## Layout

```
spike/qt-of/
├── hello/         Day 1: Qt 6.8 + MSVC 2022 toolchain, bare window
├── engine-fbo/    Day 2+3: dual-context, shared texture, 1080p plasma, FPS overlay, 5-min soak
└── build/         CMake build dir (gitignored)
```

## What was validated

A background `QThread` runs its own `QOpenGLContext` created with
`setShareContext(QOpenGLContext::globalShareContext())`. It renders a
plasma shader into an FBO's color texture every ~16 ms. The UI thread's
`QOpenGLWidget` holds the Qt-side context (same share set) and binds the
engine's texture directly in `paintGL()`. The texture ID crosses threads
as a plain `uint32_t` — no copy, no translation, no vendor-specific GL↔D3D
interop extension.

## Day 3 soak results (RTX 3070, Qt 6.8.0, MSVC 2022)

- Sustained **64.3 fps** engine thread @ 1920×1080
- **p99 frame time 32 ms** (one monitor refresh — right at the vsync boundary)
- **19,377 frames in 5 minutes**, zero crashes
- Memory flat: **114 MB → 114.5 MB** across the full soak
- Thread/handle counts flat (24–28 / 560–564)
- Clean shutdown (exit 0)

## Build / run

```powershell
# One-time Qt install (if not already there)
pip install aqtinstall
python -m aqt install-qt windows desktop 6.8.0 win64_msvc2022_64 --outputdir C:\Qt

# Build either spike
$cmake  = "C:\Program Files\Microsoft Visual Studio\2022\Community\Common7\IDE\CommonExtensions\Microsoft\CMake\CMake\bin\cmake.exe"
$src    = "C:\Users\Joe\dev\dude_wake_up\spike\qt-of\engine-fbo"
$build  = "C:\Users\Joe\dev\dude_wake_up\spike\qt-of\build\engine-fbo"
& $cmake -S $src -B $build -G "Visual Studio 17 2022" -A x64
& $cmake --build $build --config Release
& "$build\Release\engine_fbo.exe"
```

## Three fixes we hit along the way

1. **`main: glReady connect installed` fired AFTER the signal** — Qt 6's
   `show()` can trigger `initializeGL` synchronously. Install the connect
   BEFORE `w.show()`, not after.
2. **`QOffscreenSurface` MUST be created on the GUI thread**, even though
   it's used from the engine thread. Allocate on main, pass pointer to
   the `QThread` subclass's constructor.
3. **`glFinish()` per frame tanks fps** to ~34 (CPU blocks until GPU
   drains). Replace with `glFlush()` — the driver auto-syncs shared
   textures across contexts.

## Next

Commit to Qt. Phase A of the UI port (shell: main window + toolbar +
stage output + empty panels) can start now. The engine FBO plumbing in
this spike is the pattern Phase A inherits for the real `StageOutput`
widget — only the shader source changes (ours → Nottawa's full
`ShaderChainerService::processFrame()` output).

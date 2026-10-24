# Windows release plan (Qt app)

Working checklist for shipping `app/nottawa_app` as Nottawa for Windows.
Supersedes the distribution parts of `WINDOWS_PORT_PLAN.md`, which targeted
the old ImGui `Nottawa.exe`. `[x]` done, `[ ]` to do, **(you)** needs a
decision or action from Joe.

Build a release: `powershell -File packaging\windows\build_release.ps1`
(add `-Certificate <thumbprint or .pfx>` to sign). Version: `VERSION`.

## 1. Package

- [x] Release script retargeted to the Qt app: builds into
      `app\build\release`, stages `dist\Nottawa-<version>\`, smoke-tests the
      staged copy (a short stress run with throwaway app data), signs when
      given a certificate, builds the installer, zips a portable copy and the
      symbols.
- [x] Staged layout: `Nottawa.exe`, Qt DLLs + plugins, ffmpeg LGPL DLLs, the
      Visual C++ runtime (app-local), `data\` (the Mac bundle's folders,
      without dev scripts), `licenses\`. No D3D/DXC compilers (unused).
- [x] `Nottawa.exe` with the app icon, version resource and a manifest
      (UTF-8 code page, long paths, Windows 10/11).
- [x] One version source, `VERSION`: exe resource, Help > About, installer.
- [x] Inno Setup script: per-user install by default (no admin prompt),
      Start menu entry, optional desktop shortcut, `nottawa://` handler,
      closes a running Nottawa on upgrade, clears the old `data\`, keeps the
      user's own data on uninstall.
- [ ] **(you)** Install Inno Setup on this machine (`winget install
      JRSoftware.InnoSetup`), then build and test the installer.
- [ ] Smaller ffmpeg: the downloaded LGPL build is 104 MB of the 132 MB of
      DLLs and bundles ~50 codec libraries (each a licence/source obligation,
      plus openh264 patent questions). A decode-only LGPL build of our own
      would be ~10-15 MB. **(you)** worth doing for the first release?

## 2. App readiness

- [x] Non-ASCII paths: the UTF-8 manifest; verified by running the staged
      app from `dist\Zoë-测试\` with app data in a `Zoë-测试` folder.
- [x] Nothing writes into the install folder: "Edit Effect File" only shows
      when running from the source tree; workspace loading no longer falls
      back to a temp file in the working directory.
- [x] Strands: templates can't be deleted or renamed; a rename can't leave
      the strands folder or overwrite another strand.
- [x] Deep links: `nottawa://strands/<slug>` opens a shared strand, from a
      cold start or handed to the running instance (which comes to the
      front). Malformed links are rejected.
- [x] Help: About shows the version; Show Logs Folder (crash and hang stacks
      are written to `engine.log`, as module+offset when the PDB isn't there).
- [ ] **(you)** Updates: none for the first release, a "new version
      available" check against a small JSON file on nottawa.app, or
      WinSparkle (full in-app updater).
- [ ] Minor: icon thumbnails log load errors (a `previews/` folder that
      doesn't exist); harmless.

## 3. Legal

- [x] `packaging\windows\licenses\`: `THIRD_PARTY_NOTICES.txt` (index,
      credit lines, LGPL source links) and the licence texts.
- [ ] **(you)** The repo's root `LICENSE` is Sparkle's licence (replaced in
      the "Auto updates" commit), not the GPLv3 the README states. Restore it?
- [ ] **(you)** GPL: is github.com/joenot443/dude_wake_up public? Users must
      be able to get the source of the version they have.
- [ ] **(you)** Shaders: 5 are CC BY-NC-SA (CloudyTunnel, Crosshatch,
      DiscoSun, Mist, VoronoiColumns), ~24 more are adapted from Shadertoy
      (default licence CC BY-NC-SA 3.0), 3 include snippets of unknown
      licence. Non-commercial terms conflict with the GPL. Same issue on Mac.
- [ ] **(you)** Fonts: Verdana can't be redistributed; DS-Digital, Lemon
      Milk, Seaside, Caviar Dreams, American Captain, Bebas, Black Jack,
      Aller, Windsong, Podarok, Veter, VCR OSD Mono, Minecraft, OpenFontIcons
      and windows.otf need their terms checked.
- [ ] **(you)** Confirm rights to the bundled audio, images and intro video.
- [ ] `lib/ofxFastFboReader` has no licence: ask its author or rewrite it.
- [ ] ffmpeg: host a copy of the exact source rather than only linking
      upstream (LGPL "corresponding source").

## 4. Signing and distribution

- [ ] **(you)** Code signing. Unsigned, SmartScreen shows "Windows protected
      your PC". Options: Azure Trusted Signing (cheapest, monthly, identity
      validation), an OV certificate (yearly; warnings fade as downloads build
      reputation), or the Microsoft Store (signs for you, handles updates).
- [ ] **(you)** Where the installer is hosted (nottawa.app, GitHub Releases).
- [ ] Release checklist: bump `VERSION`, notes, build, sign, upload, smoke
      test the download.

## 5. QA gates

- [ ] Clean-machine install, launch, upgrade, uninstall: Windows Sandbox
      **(you: enable it; needs admin and a reboot)** or a spare PC.
- [x] Stress suite and monkey seeds pass (dev build).
- [ ] One-hour stage-mode soak with audio: memory and handles flat.
- [ ] Other GPUs (Intel / AMD), high DPI, multi-monitor stage output.

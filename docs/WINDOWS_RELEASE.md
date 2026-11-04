# Windows release plan (Qt app)

Working checklist for shipping `app/nottawa_app` as Nottawa for Windows.
Supersedes the distribution parts of `WINDOWS_PORT_PLAN.md`, which targeted
the old ImGui `Nottawa.exe`. `[x]` done, `[ ]` to do, **(you)** needs a
decision or action from Joe.

Build a release: `powershell -File packaging\windows\build_release.ps1`
(installer, portable zip, symbols), then
`powershell -File packaging\windows\build_msix.ps1` (the Store package; Nottawa's
Partner Center identity is the script's default). Version: `VERSION`.

Decisions so far: distributed through the **Microsoft Store** (MSIX, signed
by Microsoft, updated by the Store); the Inno installer stays for direct
downloads and testers (unsigned). Non-Store copies check for new versions.
A minimal LGPL ffmpeg build ships in the first release.

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
- [x] Installer built (Inno Setup 6.7, per-user) and tested: silent install,
      files, Start menu, `nottawa://`, Add/Remove Programs entry, a smoke run
      of the installed copy, and an uninstall that removes all of it.
- [x] MSIX for the Store (`build_msix.ps1`): manifest, tile/taskbar images
      from the icon, resources.pri. Registered and run locally (Developer
      Mode) with test identity values.
- [x] Minimal ffmpeg: `scripts/build_ffmpeg_lgpl.sh` builds ffmpeg 8.1.2
      (decoders, demuxers, parsers for common formats; LGPL-2.1-or-later; no
      external libraries) with mingw-w64 in WSL (`sudo apt-get install -y gcc
      mingw-w64 nasm make pkg-config`; `FFMPEG_WORK=~/ffmpeg-build` builds
      faster than on /mnt/c). The DLLs went from 109 MB to 12 MB. Plays
      H.264, HEVC, VP9, ProRes, MPEG-4, Motion JPEG and raw video with AAC,
      Opus and PCM audio (stress scenario `videoformats`; the bundled intro
      is H.264). Not included: AV1 (needs dav1d), HAP (needs snappy). The
      exact source and build script land in `dist/ffmpeg-source/`.

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
- [x] Update check (non-Store copies): once a day reads
      `https://nottawa.app/windows/latest.json`; a newer version adds "Download
      Nottawa X…" to the Help menu and a toast (not during stage mode).
      Settings switch to turn it off. The website generates that file from
      `site.json` (see "Publishing on nottawa.app" below).
- [x] engine.log kept crash/hang reports intact (stdout used to overwrite
      them); the hang watchdog ignores startup.
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

- [x] Partner Center: registered, "Nottawa" reserved. Identity:
      `JoeCrozier.Nottawa`, `CN=30C50E17-1DED-455C-93DC-3B02982966B0`,
      "Joe Crozier" (build_msix.ps1 defaults).
- [x] Windows App Certification Kit: `packaging\windows\run_wack.ps1` (from an
      elevated PowerShell; signs a test copy with a throwaway certificate for
      the publisher, runs the kit, cleans up). 2.1.0 (2026-10-28): **PASS**,
      23 of 24. The one failure is optional, "Blocked executables": Qt can
      launch processes (opening URLs and folders) and Nottawa.exe holds a
      "cmd" string, which only matters for Windows in S mode.
- [ ] Store listing: description, screenshots, privacy policy URL (the app
      talks to nottawa.app for community strands), age rating.
- [x] Direct-download installer hosted at `nottawa.app/downloads/`; the
      download page has a Windows section (see "Publishing on nottawa.app").
- [ ] **(you)** Store listing live: put its URL in the website's `site.json`.
- [ ] Release checklist: bump `VERSION`, notes, build, sign, upload, smoke
      test the download.

## 5. QA gates

- [ ] Clean-machine install, launch, upgrade, uninstall: Windows Sandbox
      **(you: enable it; needs admin and a reboot)** or a spare PC.
- [x] Stress suite and monkey seeds pass (dev build).
- [x] One-hour stage-mode soak with audio (stress scenario `soak`,
      `NOTTAWA_SOAK_MINUTES`): 9 nodes with feedback and a Blend, 4
      parameters on a sample track's audio (volume 0), an output window,
      stage mode. 2.1.0 (2026-10-28): private memory 1414 -> 1415 MB and
      handles 1702 -> 1682 from minute 5 to 60; 59.9 fps average, 59.3
      lowest; BPM 130 throughout. No crash, hang or slow frame.
- [ ] Other GPUs (Intel / AMD), high DPI, multi-monitor stage output.

## Publishing on nottawa.app

The website (repo `joenot443/nottawa-server`, folder `nottawa-website/`)
has a Windows section on `/download.html#windows`. Its buttons and
`/windows/latest.json` (the update check) all come from one block in
`nottawa-website/src/_data/site.json`:

```json
"windows": {
  "version": "2.1.0",
  "storeUrl": "",
  "installer": "/downloads/Nottawa-Setup-2.1.0.exe",
  "portable": "/downloads/Nottawa-2.1.0-portable.zip",
  "notes": "Nottawa is now available for Windows."
}
```

The Microsoft Store button is hidden while `storeUrl` is empty. Once it's
set, the Store button is the main button and the installer is the second
one. The Mac section is separate and `release.sh` still maintains it.

For each release, from the Windows machine (after `build_release.ps1`):

1. Upload the installer and portable zip to the folder the server mounts
   at `/downloads` (Windows 10+ ships OpenSSH; needs your SSH key on the
   server for `root@165.227.44.1`):
   ```powershell
   scp dist\Nottawa-Setup-<version>.exe dist\Nottawa-<version>-portable.zip `
       root@165.227.44.1:/root/nottawa-server/downloads/
   ```
   Check it: `curl.exe -I https://nottawa.app/downloads/Nottawa-Setup-<version>.exe`
   should return 200 (it's served straight from that folder, no deploy needed).
2. In `nottawa-server`, edit `windows` in `nottawa-website/src/_data/site.json`:
   `version`, both paths, `notes`, and `storeUrl` once the listing is live
   (`https://apps.microsoft.com/detail/<product id>`, from Partner Center >
   Product identity).
3. Deploy the website: run `./deploy.sh` from the `nottawa-server` checkout
   (on Windows, from Git Bash). It commits everything in that repo, pushes,
   and rebuilds the site and server on the droplet.
4. Check `https://nottawa.app/windows/latest.json` shows the new version.
   Installed non-Store copies will offer the update within a day.

The 2.1.0 files have not been uploaded yet, so the Windows download links
404 until step 1 is done for 2.1.0.

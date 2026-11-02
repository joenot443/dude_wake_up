# The Nottawa promo videos

Every visual in the promo is rendered by Nottawa itself, frame by frame, reacting to the
track as if it were playing. A headless page composes the frames, and ffmpeg muxes them with
the track. Run it again and you get the same video, frame for frame.

```
 plan (promo.mjs plan) ──capture.sh──▶ NottawaApp "video" self-test ──▶ JPEG frames per shot ─┐
                                                                                             ├─ promo.mjs render: seek(t) ─▶ MP4
 the track ──────────────────────────────────────────────────────────────────────────────────┘
```

## Making it

```sh
xcodebuild -project dude_wake_up.xcodeproj -scheme NottawaApp -configuration Debug build
cd scripts/video && npm install && cd ../..          # Playwright; it drives the installed Chrome
F=/tmp/nottawa-video                                 # any scratch folder
for fmt in wide tall; do                             # wide 1920x1080 (YouTube, X); tall 1080x1920 (Shorts)
  node scripts/video/promo.mjs plan $fmt > $F/plan-$fmt.json
  scripts/video/capture.sh $F/plan-$fmt.json $F/footage-$fmt          # ~4 min
  node scripts/video/promo.mjs render $fmt $F/footage-$fmt $F/nottawa-promo-$fmt.mp4   # ~2 min
done
```

`STILLS=4.2,9,17 node scripts/video/promo.mjs render wide …` writes just those moments as PNGs.

The render is near-lossless, about 280 MB, because film grain barely compresses. For posting,
a two-pass 8 Mbps encode comes to about 37 MB:

```sh
ffmpeg -i in.mp4 -c:v libx264 -preset slow -b:v 8000k -pass 1 -an -f mp4 /dev/null
ffmpeg -i in.mp4 -c:v libx264 -preset slow -b:v 8000k -maxrate 12000k -bufsize 16000k \
       -pass 2 -pix_fmt yuv420p -c:a aac -b:a 192k -movflags +faststart out.mp4
```

## How the footage reacts to the music

`capture.sh` launches the app with the `video` self-test scenario (NottawaApp/Engine/SelfTest.swift),
in a throwaway session. For each shot it builds the chain (a source, then effects), then for every
frame it:

- feeds the track's samples up to that frame to the File audio source (`ntw_debug_feed_audio`),
  in 512-sample buffers as playback would. The audio clock that analysis and beat tracking read
  (`AudioClock`) is held, and it advances only as audio is fed. The spectrum's release and the
  Auto tempo tracker behave as in real time, however fast the frames render.
- holds the shader clock at the frame's track time (`ntw_debug_freeze_time`).
- renders exactly one engine frame (`ntw_debug_render_frames`) and reads it back.

Each shot starts with up to 8 s of earlier music, so the tracker locks on (it reports 120 BPM
for sample track 1), plus a second of rendered warm-up for feedback and trails.

## Changing it

- The shots and their shaders are at the top of `promo.mjs` (`HERO`, `MONTAGE`, `CHAIN`, …). A
  source is named as in the app, or `#<type>` when two share a name. Change a pick, then re-run
  `plan` and `capture.sh`.
- The video's beat `b` runs at 2 per second, and the track plays from `START` (36 s: four bars before
  the drop at 40). Every scene's timing is a `seg(b, from, to)` call in `seek`.
- Another track: pass it as the last argument to `capture.sh` and `render`, then set `START`, `STOP`
  and the beat rate to match it.

Fonts: Geist and Geist Mono (SIL Open Font License, `fonts/OFL.txt`).

# BPM detection

Nottawa's tempo and beat tracking is `src/Audio/BeatTracker.hpp`. Every audio
source feeds it every buffer (`AudioSource::audioIn`), and in Auto mode
`AudioSourceService::update` takes the tempo, the beat phase that drives the
beat pulse, and the beat count from it. It replaced `BTrackDetector` (BTrack
behind a wrapper), which is kept in `Tests/bpm/legacy/` as the baseline it's
measured against.

## Why it was rebuilt

Measured with `Tests/bpm` (below), the old detector was wrong in the way most
people use Nottawa:

- **System audio (Windows and macOS)** arrives as interleaved stereo at
  48 kHz; the detector took it as mono at 44.1 kHz, so it saw time run ~2.2x
  too fast. The shown tempo was right 3% of the time.
- **48 kHz sources** (the bundled sample tracks are 48 kHz, as are many
  files and microphones) read 8.8% slow: "A Nottawa Song" showed 110 BPM.
- Even in its best case (mono 44.1 kHz) the shown BPM jittered (beat-interval
  averaging), was rarely exact (BTrack's tempo steps are 2 BPM and whole
  11.6 ms frames), couldn't show tempos over 160, and quiet inputs failed
  (a silence gate dropped them).
- The beat pulse ran on a separate wall clock, only re-anchored to beats.
- Link mode showed double the Link tempo; a tap kept the old phase instead
  of putting the beat on the tap.

## How it works

1. **Input:** any channel count and rate, downmixed, levelled by a slow
   automatic gain (quiet inputs track like loud ones) and resampled
   (band-limited) to 22.05 kHz. Silence is fed too: the audio is the clock.
2. **Onsets:** spectral flux of square-root-compressed band energies, 24
   log-spaced bands 40 Hz-10 kHz, every 5.8 ms.
3. **Tempo, 4 times a second:** the autocorrelation of the last 8 s of
   onsets scored over each period and its multiples, refined between frames
   (precise to well under 0.1 BPM); then, among the strongest periods and
   the leader's octave and meter relatives, the one whose best-aligned
   pulse train lands on the strongest onsets, with a soft preference for
   tempos around 120 settling octave ties. Small changes are smoothed; a
   different tempo (a better octave, a new track) must win by 20% for ~2 s.
4. **Phase, ~29 times a second:** the pulse-train alignment at the tempo
   steers a phase-locked beat clock (half of each error corrected; a large
   error held for 0.75 s is jumped to). The clock runs on through
   breakdowns, gives the beat events, and its phase drives the beat pulse.

A track change (sample tracks) or a stream restart resets it; it re-locks in
about 4 s. Range 60-200 BPM.

## Results

`Tests/bpm/bpm_eval` feeds audio in callback-sized blocks exactly as a
source does and scores what the app would show against true beat times:

- **synthetic**: 43 generated pieces with exact beats: house, techno, drum &
  bass, half-time, hip-hop, rock, waltz, kick-only, and live situations
  (swing, a breakdown, a tempo ramp, track changes, crowd noise, reverb,
  -36 dB input, clipping, a drifting live band);
- **Ballroom**: 698 real recordings with annotated beats (the ISMIR 2004 /
  CPJKU set, used locally for testing only);
- **tracks**: the 10 bundled sample tracks (tempo only).

Acc1: share of the time the shown BPM is within 4% of the true tempo. Acc2:
the same allowing octave errors (x2, /2, x3, /3). Exact: within 0.5%.
Beat F: beat events within 70 ms of true beats.

| | Acc1 | Acc2 | exact | lock | jitter | beat F | pulse error |
|---|---|---|---|---|---|---|---|
| synthetic, old, mono 44.1 kHz (best case) | 70% | 90% | 64% | 4.3 s | 0.28 | 0.79 | +17 ms |
| synthetic, old, system audio (48 kHz stereo) | 3% | 4% | 0% | — | 0.76 | 0.27 | |
| synthetic, old, 48 kHz file | 2% | 2% | 0% | — | 0.35 | 0.80 | |
| **synthetic, new, any of the above** | **80%** | **99%** | **94%** | **2.9 s** | **0.01** | **0.93** | **+1 ms** |
| Ballroom, old, 44.1 kHz | 59% | 85% | 32% | 7.2 s | 0.39 | 0.68 | +18 ms |
| Ballroom, old, system audio | 3% | 5% | 0% | | 1.56 | 0.41 | |
| **Ballroom, new, 44.1 kHz or system audio** | **64%** | **88%** | **41%** | **5.8 s** | **0.04** | **0.72** | **+5 ms** |

On the 410 recordings where both get the tempo right, beats: F 0.82 (old
0.81), locked onto the off-beat 18 times (old 42), median pulse error 10 ms
(old 23 ms).

Bundled tracks: old 2% (every track ~8.8% slow); new 7 of 10. Two of the
"misses" by file name are right: "088 Solar Flare" and "110 Crisp Horizon"
are really 87.50 and 109.00 (checked with `--oracle`, a whole-track offline
estimate). The others: "065 Liquid Glow" has no clear beat; "080 L Train"
(79 BPM) has sections in 4/3 that read as 105; "133 Echoes of Tomorrow" is
as convincingly 88.7 (its triplet feel) as 133.

### Known limits

- **Octave convention.** Where a tempo and its half are equally plausible
  the shown tempo is the one nearer 120: drum & bass reads 87 rather than
  174, half-time dubstep 70 rather than 140, a kick on every beat at 180
  reads 90. The beat pulse then lands on every other beat. (Acc2 counts
  these right; DJ software differs too.)
- **Syncopated music** (some Latin) can still lock to the off-beat.
- **Latency.** The pulse is aligned to the audio as it's analysed; output,
  capture and display latency are for the BPM Nudge control.

## Running the evaluation

```
powershell -File Tests\bpm\build.ps1          # builds Tests\bpm\build\Release\bpm_eval.exe
cd Tests/bpm
./build/Release/bpm_eval.exe --algo new --synthetic
./build/Release/bpm_eval.exe --algo new --synthetic --rate 48000 --channels 2 --block 480   # as system audio
./build/Release/bpm_eval.exe --algo new --ballroom <dir with BallroomData and BallroomAnnotations-master>
./build/Release/bpm_eval.exe --algo new --tracks ../../audio --verbose
./build/Release/bpm_eval.exe --tracks ../../audio --oracle                # offline whole-track tempos
```

`--algo legacy` runs the old detector; `--verbose` prints each item;
`--only <text>` limits to matching items; `--dump` prints pulse offsets over
time; `--csv` writes per-item results. `build.ps1 -Trace` builds a version
that prints the tracker's candidates at every update. `sweep.sh` runs
Ballroom once per tuning setting, in parallel (`BT_MEM`, `BT_GAIN`,
`BT_EVERY`, `BT_SNAP`; see `BeatTracker::Tuning`).

Ballroom: audio from
`https://mtg.upf.edu/ismir2004/contest/tempoContest/data1.tar.gz` (the server
wants a browser User-Agent), beats from
`https://github.com/CPJKU/BallroomAnnotations`.

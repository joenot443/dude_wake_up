#!/usr/bin/env python3
"""Scores the audio samples NottawaApp's self-test recorded (see
NottawaApp/Engine/SelfTest.swift): what the app showed against each track.

    mac_selftest_bpm.py <selftest dir> <sample track dir>

Each CSV is one track: wall time, the track's own clock, and the shown BPM,
beat pulse and beat count. The pulse is exp(-5 * phase), so every sample
says when the last shown beat was: track time - phase * 60 / bpm.

Truth: the tempo in the track's file name, and the beat position found in
the audio itself, by folding a kick-weighted onset envelope at that tempo
(period fine-tuned within 1%). Folding contrast says how clear that is;
below ~1.5 the beat position isn't trustworthy.

Per track: median shown BPM; acc1 (within 4% of the label) and acc2 (also
allowing x2, /2, x3, /3) over the scored part; beat offset (median of shown
beats minus true beats, ms; + is late) and the share of shown beats within
70 ms of a true beat ("on") or of the off-beat ("off"). Scoring starts 10 s
in, after the tracker's warm-up.
"""
import csv
import math
import os
import re
import subprocess
import sys

import numpy as np

RATE = 22050
HOP = 128
WARM = 10.0
# The app decodes MP3s with the bundled (old) dr_mp3, which keeps the
# encoder delay and the info frame that ffmpeg and AVFoundation trim: its
# sample clock is this many samples ahead of theirs for the same audio
# (2257 on every sample track, measured by cross-correlating the decodes).
# Applies to the "files" clock only; "system" uses AVAudioPlayer's.
DR_MP3_LEAD_SAMPLES = 2257


def load_rows(path):
    with open(path) as f:
        r = csv.DictReader(f)
        rows = [{k: float(v) for k, v in row.items()} for row in r]
    return rows


def clock_fit(rows):
    """track_time = a * wall + b over the first pass through the track."""
    pts = [(r["wall"], r["track_time"]) for r in rows if r["track_time"] >= 0]
    if len(pts) < 50:
        return None
    w, t = np.array(pts).T
    # first pass only: stop where the clock wraps (looping)
    wrap = np.where(np.diff(t) < -1.0)[0]
    if len(wrap):
        w, t = w[: wrap[0] + 1], t[: wrap[0] + 1]
    a, b = np.polyfit(w, t, 1)
    for _ in range(2):
        keep = np.abs(t - (a * w + b)) < 0.03
        a, b = np.polyfit(w[keep], t[keep], 1)
    end = w[-1]
    return a, b, end


def onset_envelope(path, seconds):
    pcm = subprocess.run(
        ["ffmpeg", "-v", "quiet", "-t", str(seconds), "-i", path, "-ac", "1", "-ar", str(RATE), "-f", "f32le", "-"],
        capture_output=True, check=True).stdout
    x = np.frombuffer(pcm, dtype=np.float32)
    n = 1024
    win = np.hanning(n).astype(np.float32)
    frames = 1 + (len(x) - n) // HOP
    idx = np.arange(n)[None, :] + HOP * np.arange(frames)[:, None]
    spec = np.abs(np.fft.rfft(x[idx] * win, axis=1))
    logs = np.log1p(100 * spec)
    flux = np.maximum(0, np.diff(logs, axis=0))
    freqs = np.fft.rfftfreq(n, 1 / RATE)
    low = flux[:, freqs < 150].sum(axis=1)
    full = flux.sum(axis=1)
    env = low / (low.std() + 1e-9) + 0.5 * full / (full.std() + 1e-9)
    times = (np.arange(len(env)) + 1) * HOP / RATE + n / 2 / RATE
    return times, env


def fold(times, env, period, t0, bins=120):
    sel = times >= t0
    ph = ((times[sel] / period) % 1.0 * bins).astype(int)
    acc = np.bincount(ph, weights=env[sel], minlength=bins)
    cnt = np.bincount(ph, minlength=bins)
    curve = acc / np.maximum(cnt, 1)
    # smooth circularly (about 1/40 of a beat)
    k = np.array([1, 2, 3, 2, 1], float)
    curve = np.convolve(np.r_[curve[-2:], curve, curve[:2]], k / k.sum(), "valid")
    return curve


def truth_grid(path, label_bpm, seconds):
    times, env = onset_envelope(path, seconds)
    best = None
    for f in np.linspace(0.99, 1.01, 81):
        period = 60.0 / (label_bpm * f)
        curve = fold(times, env, period, WARM)
        contrast = curve.max() / (curve.mean() + 1e-9)
        if best is None or contrast > best[0]:
            best = (contrast, period, curve)
    contrast, period, curve = best
    bins = len(curve)
    i = int(curve.argmax())
    # parabolic peak
    y0, y1, y2 = curve[(i - 1) % bins], curve[i], curve[(i + 1) % bins]
    d = 0.5 * (y0 - y2) / (y0 - 2 * y1 + y2) if (y0 - 2 * y1 + y2) != 0 else 0
    phase = ((i + d) / bins) * period
    return period, phase, contrast


def track_rate(path):
    out = subprocess.run(["ffprobe", "-v", "quiet", "-show_entries", "stream=sample_rate", "-of", "csv=p=0", path],
                         capture_output=True, text=True, check=True).stdout
    return float(out.split()[0])


def within(bpm, truth, tol, octaves):
    factors = [1, 2, 0.5, 3, 1 / 3] if octaves else [1]
    return any(abs(bpm - truth * f) <= tol * truth * f for f in factors)


def score(csv_path, audio_dir):
    m = re.match(r"(\w+)-(\d{3})-(.+)\.csv$", os.path.basename(csv_path))
    label, bpm_label, name = m.group(1), int(m.group(2)), m.group(3)
    track = os.path.join(audio_dir, f"{bpm_label:03d}{name}.mp3")
    rows = load_rows(csv_path)
    fit = clock_fit(rows)
    if fit is None:
        return f"{label:6s} {name:20s} no track clock samples"
    a, b, end = fit
    if label == "files":
        b -= DR_MP3_LEAD_SAMPLES / track_rate(track)
    seconds = (end * a + b) + 2
    period, phase, contrast = truth_grid(track, bpm_label, seconds)
    true_bpm = 60 / period

    shown, offsets = [], []
    for r in rows:
        if r["wall"] > end:
            break
        t = a * r["wall"] + b
        if t < WARM or r["bpm"] <= 0:
            continue
        shown.append(r["bpm"])
        p = r["pulse"]
        if not (0.007 < p <= 1.0):
            continue
        beat = t - (-math.log(p) / 5.0) * 60.0 / r["bpm"]
        offsets.append(((beat - phase + period / 2) % period) - period / 2)
    if not shown:
        return f"{label:6s} {name:20s} nothing scored"
    shown = np.array(shown)
    off = np.array(offsets) * 1000
    pms = period * 1000
    acc1 = np.mean([within(x, bpm_label, 0.04, False) for x in shown]) * 100
    acc2 = np.mean([within(x, bpm_label, 0.04, True) for x in shown]) * 100
    on = np.mean(np.abs(off) < 70) * 100 if len(off) else 0
    offb = np.mean(np.abs(np.abs(off) - pms / 2) < 70) * 100 if len(off) else 0
    bias = np.median(off[np.abs(off) < 70]) if on > 0 else float("nan")
    flag = "" if contrast >= 1.5 else "  (weak beat truth)"
    return (f"{label:6s} {name:20s} label {bpm_label:3d} (audio {true_bpm:6.2f})  shown {np.median(shown):6.2f}  "
            f"acc1 {acc1:3.0f}%  acc2 {acc2:3.0f}%  beats on {on:3.0f}% off {offb:3.0f}%  bias {bias:+5.0f} ms  "
            f"contrast {contrast:.2f}{flag}")


def main():
    out_dir, audio_dir = sys.argv[1], sys.argv[2]
    for f in sorted(os.listdir(out_dir)):
        if f.endswith(".csv"):
            print(score(os.path.join(out_dir, f), audio_dir))


if __name__ == "__main__":
    main()

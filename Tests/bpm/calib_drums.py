# A drum track with exactly known beats: 128 BPM, first beat at 0.5 s.
# Kick on every beat, snare on 2 and 4, closed hats on eighths, a bass
# note on the off-beats. 48 kHz stereo 16-bit WAV.
import math, random, struct, sys, wave
SR, BPM, T0, SECONDS = 48000, 128.0, 0.5, 75.0
out = sys.argv[1]
n = int(SR * SECONDS)
buf = [0.0] * n
period = 60.0 / BPM
rnd = random.Random(1)
def add(start, samples):
    i0 = int(round(start * SR))
    for k, v in enumerate(samples):
        if 0 <= i0 + k < n: buf[i0 + k] += v
kick = []
phase = 0.0
for k in range(int(0.25 * SR)):
    t = k / SR
    f = 45 + 75 * math.exp(-t * 30)
    phase += 2 * math.pi * f / SR
    kick.append(0.9 * math.sin(phase) * math.exp(-t * 9))
snare = [0.35 * (rnd.random() * 2 - 1) * math.exp(-(k / SR) * 22) + 0.2 * math.sin(2 * math.pi * 190 * k / SR) * math.exp(-(k / SR) * 30) for k in range(int(0.2 * SR))]
hat, prev = [], 0.0
for k in range(int(0.05 * SR)):
    x = rnd.random() * 2 - 1
    hat.append(0.12 * (x - prev) * math.exp(-(k / SR) * 80)); prev = x
bass = [0.25 * math.sin(2 * math.pi * 55 * k / SR) * min(1, k / 200) * math.exp(-(k / SR) * 4) for k in range(int(period * 0.45 * SR))]
beat = 0
while T0 + beat * period < SECONDS:
    t = T0 + beat * period
    add(t, kick)
    if beat % 2 == 1: add(t, snare)
    add(t, hat); add(t + period / 2, hat)
    add(t + period / 2, bass)
    beat += 1
peak = max(abs(v) for v in buf) or 1
with wave.open(out, "wb") as w:
    w.setnchannels(2); w.setsampwidth(2); w.setframerate(SR)
    frames = bytearray()
    for v in buf:
        s = int(max(-1, min(1, v / peak * 0.8)) * 32767)
        frames += struct.pack("<hh", s, s)
    w.writeframes(bytes(frames))
print("beats", beat, "period", period)

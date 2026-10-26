// Synthetic music with exactly known beats, for evaluating beat tracking.
//
// Each case renders mono audio at a given sample rate from drum, bass and
// pad voices playing a style's pattern over a beat grid, which can change
// tempo (ramps, jumps), drift (live playing), swing, drop out (breakdowns),
// and be degraded (noise, reverb, low level, clipping). The beat times of
// that grid are the ground truth.
#pragma once

#include <algorithm>
#include <cmath>
#include <functional>
#include <random>
#include <string>
#include <vector>

namespace synth {

struct Case {
    std::string name;
    std::string style;        // see renderStyle()
    double bpm = 120;         // starting tempo (quarter-note beats)
    double endBpm = 0;        // >0: linear tempo ramp to this by the end
    double jumpAt = 0;        // >0: switch to jumpBpm at this time (with a phase reset)
    double jumpBpm = 0;
    double seconds = 60;
    double swing = 0;         // 0..1: 16th-note swing amount
    double humanize = 0;      // seconds of random timing jitter per hit
    double drift = 0;         // relative slow tempo wander (live band), e.g. 0.015
    int beatsPerBar = 4;
    double breakdownStart = 0, breakdownEnd = 0;  // seconds with no percussion
    double noiseDb = -200;    // crowd/pink noise level relative to full scale
    double reverb = 0;        // 0..1 reverb send
    double gainDb = 0;        // overall level (e.g. -30 for a quiet input)
    bool clip = false;        // hard-clip / brick-wall limit
    unsigned seed = 1;
};

struct Rendered {
    std::vector<float> mono;
    std::vector<double> beats;  // true beat times, seconds
    double sampleRate = 44100;
};

namespace detail {

constexpr double kPi = 3.14159265358979323846;

struct Mixer {
    std::vector<float>& out;
    double sr;
    void add(double t, const std::function<float(double)>& voice, double length) {
        const long start = long(t * sr);
        const long n = long(length * sr);
        for (long i = 0; i < n; ++i) {
            const long k = start + i;
            if (k < 0) continue;
            if (k >= long(out.size())) break;
            out[size_t(k)] += voice(double(i) / sr);
        }
    }
};

inline float kick(double t, double level) {
    const double f = 45 + 75 * std::exp(-t * 25);                // pitch sweep 120 -> 45 Hz
    const double phase = 2 * kPi * (45 * t + 75 * (1 - std::exp(-t * 25)) / 25);
    (void)f;
    return float(level * std::sin(phase) * std::exp(-t * 9) + level * 0.3 * std::exp(-t * 300));
}

struct Noise {
    std::mt19937 rng;
    std::uniform_real_distribution<float> u{-1.f, 1.f};
    float b0 = 0, b1 = 0, b2 = 0, hp = 0, last = 0;
    explicit Noise(unsigned seed) : rng(seed) {}
    float white() { return u(rng); }
    float pink() {  // Paul Kellet's economy filter
        const float w = white();
        b0 = 0.99765f * b0 + w * 0.0990460f;
        b1 = 0.96300f * b1 + w * 0.2965164f;
        b2 = 0.57000f * b2 + w * 1.0526913f;
        return (b0 + b1 + b2 + w * 0.1848f) * 0.2f;
    }
    float high() {  // crude high-pass of white noise
        const float w = white();
        const float y = w - last;
        last = w;
        return y * 0.5f;
    }
};

}  // namespace detail

// Beat times for a case: tempo (with ramp, jump and drift) integrated over time.
inline std::vector<double> beatGrid(const Case& c) {
    std::vector<double> beats;
    std::mt19937 rng(c.seed * 7919u + 13u);
    std::normal_distribution<double> walk(0.0, 1.0);
    double t = 0.0;
    double wander = 0.0;
    bool jumped = false;
    while (t < c.seconds) {
        beats.push_back(t);
        double bpm = c.bpm;
        if (c.endBpm > 0) bpm = c.bpm + (c.endBpm - c.bpm) * std::min(1.0, t / c.seconds);
        if (c.jumpAt > 0 && t >= c.jumpAt) bpm = c.jumpBpm;
        if (c.drift > 0) {  // a slow random walk, pulled back towards the base tempo
            wander = wander * 0.97 + walk(rng) * c.drift * 0.25;
            wander = std::clamp(wander, -2 * c.drift, 2 * c.drift);
            bpm *= 1.0 + wander;
        }
        double next = t + 60.0 / bpm;
        if (c.jumpAt > 0 && !jumped && next >= c.jumpAt) {
            // A new track starts at jumpAt: the beat grid restarts there.
            jumped = true;
            next = c.jumpAt;
            if (next - t < 0.05) next = t + 60.0 / bpm;
        }
        t = next;
    }
    return beats;
}

// Renders the case at `sampleRate`.
inline Rendered render(const Case& c, double sampleRate) {
    using namespace detail;
    Rendered r;
    r.sampleRate = sampleRate;
    r.beats = beatGrid(c);
    r.mono.assign(size_t(c.seconds * sampleRate), 0.f);
    Mixer mix{r.mono, sampleRate};
    Noise noise(c.seed);
    std::mt19937 rng(c.seed);
    std::normal_distribution<double> jitter(0.0, std::max(1e-9, c.humanize));

    auto inBreakdown = [&](double t) { return t >= c.breakdownStart && t < c.breakdownEnd; };
    auto hit = [&](double t) { return c.humanize > 0 ? t + jitter(rng) : t; };

    // Position of a 16th-note step within beat b (swing delays odd 16ths).
    auto stepTime = [&](size_t b, int step16) {
        const double t0 = r.beats[b];
        const double t1 = b + 1 < r.beats.size() ? r.beats[b + 1] : t0 + 60.0 / c.bpm;
        double frac = step16 / 4.0;
        if (step16 % 2 == 1) frac += c.swing * 0.125;
        return t0 + (t1 - t0) * frac;
    };

    const auto noiseVoice = [&](double decay, bool highOnly, double level) {
        return [&noise, decay, highOnly, level](double t) {
            const float n = highOnly ? noise.high() : noise.white();
            return float(level * n * std::exp(-t * decay));
        };
    };
    const auto tone = [](double freq, double decay, double level) {
        return [freq, decay, level](double t) { return float(level * std::sin(2 * kPi * freq * t) * std::exp(-t * decay)); };
    };
    const auto snare = [&](double t) {
        mix.add(hit(t), noiseVoice(28, false, 0.35), 0.25);
        mix.add(t, tone(185, 30, 0.25), 0.15);
    };
    const auto clap = [&](double t) { mix.add(hit(t), noiseVoice(35, true, 0.45), 0.2); };
    const auto hat = [&](double t, double level) { mix.add(hit(t), noiseVoice(90, true, level), 0.06); };
    const auto openHat = [&](double t) { mix.add(hit(t), noiseVoice(12, true, 0.18), 0.3); };
    const auto kickAt = [&](double t, double level = 0.9) {
        mix.add(hit(t), [level](double s) { return kick(s, level); }, 0.5);
    };
    const auto bass = [&](double t, double freq, double length) {
        mix.add(t, [freq, length](double s) {
            const double env = std::min(1.0, s * 200) * std::exp(-s * 3) * (s < length ? 1.0 : std::exp(-(s - length) * 60));
            const double saw = 2 * (freq * s - std::floor(freq * s + 0.5));
            return float(0.22 * env * (0.6 * saw + 0.4 * std::sin(2 * kPi * freq * s)));
        }, length + 0.1);
    };

    const double bassNotes[] = {55.0, 55.0, 65.4, 49.0};
    for (size_t b = 0; b < r.beats.size(); ++b) {
        const double t = r.beats[b];
        const int beatInBar = int(b % size_t(c.beatsPerBar));
        const int bar = int(b / size_t(c.beatsPerBar));
        const double beatLen = (b + 1 < r.beats.size() ? r.beats[b + 1] : t + 60.0 / c.bpm) - t;
        const double note = bassNotes[(bar / 2) % 4];
        if (inBreakdown(t)) continue;
        const std::string& s = c.style;
        if (s == "house" || s == "techno") {
            kickAt(t);
            if (beatInBar == 1 || beatInBar == 3) clap(t);
            openHat(stepTime(b, 2));
            for (int k = 0; k < 4; ++k) hat(stepTime(b, k), k % 2 ? 0.08 : 0.05);
            bass(stepTime(b, 2), note, beatLen * 0.4);
        } else if (s == "dnb") {  // two-step: kick 1 and the "and" of 2-ish, snare 2 and 4
            if (beatInBar == 0) kickAt(t);
            if (beatInBar == 2) kickAt(stepTime(b, 2), 0.8);
            if (beatInBar == 1 || beatInBar == 3) snare(t);
            for (int k = 0; k < 4; k += 2) hat(stepTime(b, k), 0.07);
            if (beatInBar % 2 == 0) bass(t, note, beatLen * 1.5);
        } else if (s == "halftime") {  // dubstep / trap feel: kick on 1, snare on 3
            if (beatInBar == 0) kickAt(t);
            if (beatInBar == 2) snare(t);
            if (beatInBar == 3 && bar % 2) kickAt(stepTime(b, 2), 0.7);
            for (int k = 0; k < 4; k += 2) hat(stepTime(b, k), 0.05);
            if (bar % 4 == 3 && beatInBar == 3) for (int k = 0; k < 8; ++k) hat(t + beatLen * k / 8.0, 0.05);  // hat roll
            if (beatInBar == 0) bass(t, note, beatLen * 3.5);
        } else if (s == "hiphop") {
            if (beatInBar == 0) kickAt(t);
            if (beatInBar == 1 && bar % 2) kickAt(stepTime(b, 2), 0.7);
            if (beatInBar == 2) kickAt(stepTime(b, 2), 0.8);
            if (beatInBar == 1 || beatInBar == 3) snare(t);
            for (int k = 0; k < 4; k += 2) hat(stepTime(b, k), 0.06);
            if (beatInBar == 0) bass(t, note, beatLen * 1.8);
        } else if (s == "rock") {
            if (beatInBar == 0 || beatInBar == 2) kickAt(t);
            if (beatInBar == 1 || beatInBar == 3) snare(t);
            for (int k = 0; k < 4; k += 2) hat(stepTime(b, k), 0.07);
            bass(t, note, beatLen * 0.8);
        } else if (s == "waltz") {  // 3/4: strong 1, light 2 and 3
            if (beatInBar == 0) { kickAt(t, 0.8); bass(t, note, beatLen * 0.9); }
            else { hat(t, 0.08); mix.add(t, tone(330, 12, 0.08), 0.3); }
        } else if (s == "minimal") {  // kick only
            kickAt(t);
        }
    }

    // A sustained pad (no transients) under everything, including breakdowns.
    {
        const double chord[] = {220.0, 261.6, 329.6};
        for (size_t i = 0; i < r.mono.size(); ++i) {
            const double t = double(i) / sampleRate;
            const double lfo = 0.5 + 0.5 * std::sin(2 * kPi * 0.13 * t);
            double v = 0;
            for (double f : chord) v += std::sin(2 * kPi * f * t + std::sin(2 * kPi * 0.31 * t));
            r.mono[i] += float(0.03 * lfo * v);
        }
    }

    if (c.reverb > 0) {  // feedback comb + allpass, Schroeder-style
        std::vector<float> wet(r.mono.size(), 0.f);
        const int delays[] = {1557, 1617, 1491, 1422};
        for (int d : delays) {
            const size_t D = size_t(d * sampleRate / 44100.0);
            std::vector<float> buf(D, 0.f);
            size_t idx = 0;
            for (size_t i = 0; i < r.mono.size(); ++i) {
                const float y = buf[idx];
                buf[idx] = r.mono[i] + y * 0.84f;
                wet[i] += y * 0.25f;
                idx = (idx + 1) % D;
            }
        }
        for (size_t i = 0; i < r.mono.size(); ++i) r.mono[i] = float(r.mono[i] * (1 - c.reverb * 0.5) + wet[i] * c.reverb);
    }
    if (c.noiseDb > -150) {
        const float level = float(std::pow(10.0, c.noiseDb / 20.0));
        for (float& s : r.mono) s += level * noise.pink() * 3.f;
    }
    // Normalise to -3 dBFS peak, then apply the case's level and clipping.
    float peak = 1e-9f;
    for (float s : r.mono) peak = std::max(peak, std::fabs(s));
    const float gain = float(0.707 / peak * std::pow(10.0, c.gainDb / 20.0)) * (c.clip ? 4.f : 1.f);
    for (float& s : r.mono) {
        s *= gain;
        if (c.clip) s = std::clamp(s, -0.9f, 0.9f);
    }
    return r;
}

// The standard synthetic suite: styles across their usual tempos plus
// the situations a live show throws at a tracker.
inline std::vector<Case> suite() {
    std::vector<Case> cases;
    auto add = [&](Case c) { cases.push_back(c); };
    unsigned seed = 1;
    for (double bpm : {118.0, 122.0, 124.0, 126.0, 128.0, 130.0, 132.0}) add({"house " + std::to_string(int(bpm)), "house", bpm, 0, 0, 0, 60, 0, 0, 0, 4, 0, 0, -200, 0, 0, false, seed++});
    for (double bpm : {135.0, 140.0, 145.0, 150.0}) add({"techno " + std::to_string(int(bpm)), "techno", bpm, 0, 0, 0, 60, 0, 0, 0, 4, 0, 0, -200, 0, 0, false, seed++});
    for (double bpm : {160.0, 170.0, 172.0, 174.0, 176.0}) add({"dnb " + std::to_string(int(bpm)), "dnb", bpm, 0, 0, 0, 60, 0, 0, 0, 4, 0, 0, -200, 0, 0, false, seed++});
    for (double bpm : {70.0, 75.0, 140.0, 150.0}) add({"halftime " + std::to_string(int(bpm)), "halftime", bpm, 0, 0, 0, 60, 0, 0, 0, 4, 0, 0, -200, 0, 0, false, seed++});
    for (double bpm : {85.0, 90.0, 95.0, 100.0}) add({"hiphop " + std::to_string(int(bpm)), "hiphop", bpm, 0, 0, 0, 60, 0, 0.004, 0, 4, 0, 0, -200, 0, 0, false, seed++});
    for (double bpm : {100.0, 110.0, 115.0, 120.0}) add({"rock " + std::to_string(int(bpm)), "rock", bpm, 0, 0, 0, 60, 0, 0.008, 0.012, 4, 0, 0, -200, 0, 0, false, seed++});
    for (double bpm : {84.0, 90.0}) add({"waltz " + std::to_string(int(bpm)), "waltz", bpm, 0, 0, 0, 60, 0, 0, 0, 3, 0, 0, -200, 0, 0, false, seed++});
    for (double bpm : {65.0, 180.0, 190.0}) add({"minimal " + std::to_string(int(bpm)), "minimal", bpm, 0, 0, 0, 60, 0, 0, 0, 4, 0, 0, -200, 0, 0, false, seed++});
    // Live situations
    add({"swing house 124", "house", 124, 0, 0, 0, 60, 0.6, 0, 0, 4, 0, 0, -200, 0, 0, false, seed++});
    add({"breakdown house 126", "house", 126, 0, 0, 0, 75, 0, 0, 0, 4, 25, 45, -200, 0, 0, false, seed++});
    add({"ramp 120-128", "house", 120, 128, 0, 0, 90, 0, 0, 0, 4, 0, 0, -200, 0, 0, false, seed++});
    add({"jump 124-140", "techno", 124, 0, 40, 140, 80, 0, 0, 0, 4, 0, 0, -200, 0, 0, false, seed++});
    add({"jump 128-87", "house", 128, 0, 40, 87, 80, 0, 0, 0, 4, 0, 0, -200, 0, 0, false, seed++});
    add({"crowd noise 128", "house", 128, 0, 0, 0, 60, 0, 0, 0, 4, 0, 0, -12, 0.3, 0, false, seed++});
    add({"reverb 122", "house", 122, 0, 0, 0, 60, 0, 0, 0, 4, 0, 0, -200, 0.8, 0, false, seed++});
    add({"quiet -36dB 128", "house", 128, 0, 0, 0, 60, 0, 0, 0, 4, 0, 0, -200, 0, -36, false, seed++});
    add({"clipped 130", "techno", 130, 0, 0, 0, 60, 0, 0, 0, 4, 0, 0, -200, 0, 0, true, seed++});
    add({"live band 112", "rock", 112, 0, 0, 0, 90, 0, 0.012, 0.02, 4, 0, 0, -200, 0.3, 0, false, seed++});
    return cases;
}

}  // namespace synth

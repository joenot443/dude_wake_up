// bpm_eval: measures Nottawa's BPM detection against ground truth.
//
//   bpm_eval [--algo legacy|new] [--rate 48000] [--channels 2] [--block 480]
//            [--synthetic] [--ballroom <dir>] [--tracks <dir>] [--limit N]
//            [--csv results.csv] [--verbose]
//
// Audio is fed to a tracker the way a live source does (callback-sized,
// interleaved blocks at the source's rate), and what the app would show is
// recorded: the BPM over time, beat events, and the visual beat pulse.
// These are scored against the true beat times:
//   tempo   Acc1: share of the time the BPM is within 4% of the true tempo;
//           Acc2: the same allowing octave errors (x2, x1/2, x3, x1/3);
//           exact: within 0.5% (what "128" vs "126.1" on screen is about);
//           lock: seconds until it holds within 4% (octave-tolerant) for 5 s;
//           jitter: spread of the shown BPM once settled.
//   beats   F-measure of beat events within +-70 ms of true beats.
//   pulse   F-measure of the visual pulse's peaks within +-70 ms, and their
//           mean offset (bias) and absolute error.
// Scoring starts after a warm-up (8 s for tempo, 5 s for beats).

#include "Synth.hpp"
#include "kiss_fft.h"
#include "Trackers.hpp"

#define DR_WAV_IMPLEMENTATION
#include "dr_wav.h"
#define DR_MP3_IMPLEMENTATION
#include "dr_mp3.h"

#include <cstdio>
#include <filesystem>
#include <fstream>
#include <map>
#include <memory>
#include <numeric>
#include <sstream>

namespace fs = std::filesystem;

struct Item {
    std::string dataset, category, name;
    std::vector<float> audio;  // interleaved
    int channels = 1;
    double sampleRate = 44100;
    std::vector<double> beats;  // truth; empty: tempo-only truth
    double tempo = 0;           // truth tempo when there are no beats
};

struct Result {
    std::string dataset, category, name;
    double truthTempo = 0, finalBpm = 0;
    double acc1 = 0, acc2 = 0, exact = 0, lock = -1, jitter = 0;
    bool finalAcc1 = false, finalAcc2 = false;
    double beatF = -1, pulseF = -1, pulseBiasMs = 0, pulseAbsMs = 0;
    double offbeat = 0;   // share of pulses near the off-beat (truth beat + half a period)
    double spreadMs = 0;  // spread (std dev) of pulse offsets near the beat
};

// ---- Truth helpers ---------------------------------------------------------

static double median(std::vector<double> v) {
    if (v.empty()) return 0;
    std::nth_element(v.begin(), v.begin() + v.size() / 2, v.end());
    return v[v.size() / 2];
}

static double localTempo(const std::vector<double>& beats, double t, double fallback) {
    if (beats.size() < 3) return fallback;
    auto it = std::upper_bound(beats.begin(), beats.end(), t);
    size_t i = size_t(std::clamp<long>(long(it - beats.begin()), 1, long(beats.size()) - 1));
    // Median of the (up to) 8 surrounding inter-beat intervals: hand-tapped
    // annotations jitter by several ms, so fewer would make a noisy truth.
    std::vector<double> ibi;
    for (size_t k = i >= 4 ? i - 4 : 0; k + 1 < beats.size() && k < i + 4; ++k) ibi.push_back(beats[k + 1] - beats[k]);
    return 60.0 / median(ibi);
}

static bool within(double bpm, double truth, double tol, bool octaves) {
    if (truth <= 0 || bpm <= 0) return false;
    const double factors[] = {1.0, 2.0, 0.5, 3.0, 1.0 / 3.0};
    for (int i = 0; i < (octaves ? 5 : 1); ++i)
        if (std::fabs(bpm - truth * factors[i]) <= tol * truth * factors[i]) return true;
    return false;
}

// F-measure of `detections` against `truth` within +-window (one-to-one matching).
static double fMeasure(const std::vector<double>& detections, const std::vector<double>& truth, double window,
                       double from, std::vector<double>* offsets = nullptr) {
    std::vector<double> d, g;
    for (double x : detections) if (x >= from) d.push_back(x);
    for (double x : truth) if (x >= from) g.push_back(x);
    if (g.empty()) return -1;
    if (d.empty()) return 0;
    std::vector<bool> used(g.size(), false);
    int hits = 0;
    for (double x : d) {
        auto it = std::lower_bound(g.begin(), g.end(), x - window);
        int best = -1;
        double bestDist = window + 1;
        for (; it != g.end() && *it <= x + window; ++it) {
            const size_t k = size_t(it - g.begin());
            if (!used[k] && std::fabs(*it - x) < bestDist) { best = int(k); bestDist = std::fabs(*it - x); }
        }
        if (best >= 0) {
            used[size_t(best)] = true;
            ++hits;
            if (offsets) offsets->push_back(x - g[size_t(best)]);
        }
    }
    const double p = double(hits) / d.size(), r = double(hits) / g.size();
    return p + r > 0 ? 2 * p * r / (p + r) : 0;
}

// ---- Running a tracker over an item ----------------------------------------

static bool g_dump = false;    // --dump: pulse offsets over time
static bool g_oracle = false;  // --oracle: offline whole-track tempo only

static Result evaluate(Tracker& tracker, const Item& item, int block) {
    Result res{item.dataset, item.category, item.name};
    const size_t frames = item.audio.size() / size_t(item.channels);
    const double duration = frames / item.sampleRate;
    const double globalTruth = item.beats.size() > 2 ? median([&] {
        std::vector<double> ibi;
        for (size_t i = 1; i < item.beats.size(); ++i) ibi.push_back(60.0 / (item.beats[i] - item.beats[i - 1]));
        return ibi;
    }()) : item.tempo;
    res.truthTempo = globalTruth;

    std::vector<double> sampleT, sampleBpm, beats, pulses;
    double nextSample = 0.0, nextPhaseT = 0.0, lastPhase = -1;
    const double phaseStep = 0.002;
    for (size_t f = 0; f < frames; f += size_t(block)) {
        const int n = int(std::min<size_t>(size_t(block), frames - f));
        const double t = f / item.sampleRate;
        const double tEnd = (f + n) / item.sampleRate;
        tracker.process(item.audio.data() + f * size_t(item.channels), n, item.channels, item.sampleRate, t);
        for (double b : tracker.takeBeats()) beats.push_back(b);
        // The state after this block is what the app shows until the next one.
        const double tNext = std::min(duration, (f + n + block) / item.sampleRate);
        for (; nextSample < tNext; nextSample += 0.1) {
            sampleT.push_back(nextSample);
            sampleBpm.push_back(tracker.displayedBpm());
        }
        for (nextPhaseT = std::max(nextPhaseT, tEnd); nextPhaseT < tNext; nextPhaseT += phaseStep) {
            const double ph = tracker.pulsePhase(nextPhaseT);
            if (lastPhase >= 0 && ph + 0.5 < lastPhase) pulses.push_back(nextPhaseT);  // wrapped: a pulse peak
            lastPhase = ph;
        }
    }

    // Tempo scores, after warm-up.
    const double warm = 8.0;
    int count = 0, a1 = 0, a2 = 0, ex = 0;
    std::vector<double> settled;
    for (size_t i = 0; i < sampleT.size(); ++i) {
        if (sampleT[i] < warm) continue;
        const double truth = item.beats.empty() ? globalTruth : localTempo(item.beats, sampleT[i], globalTruth);
        ++count;
        a1 += within(sampleBpm[i], truth, 0.04, false);
        a2 += within(sampleBpm[i], truth, 0.04, true);
        ex += within(sampleBpm[i], truth, 0.005, true);
        settled.push_back(sampleBpm[i]);
    }
    if (count) { res.acc1 = double(a1) / count; res.acc2 = double(a2) / count; res.exact = double(ex) / count; }
    // Final tempo: the median of the last 30%.
    std::vector<double> tail(sampleBpm.begin() + long(sampleBpm.size() * 0.7), sampleBpm.end());
    res.finalBpm = median(tail);
    const double truthEnd = item.beats.empty() ? globalTruth : localTempo(item.beats, duration - 1, globalTruth);
    res.finalAcc1 = within(res.finalBpm, truthEnd, 0.04, false);
    res.finalAcc2 = within(res.finalBpm, truthEnd, 0.04, true);
    // Lock: first time it then holds (octave-tolerant) for 5 s.
    double runStart = -1;
    for (size_t i = 0; i < sampleT.size(); ++i) {
        const double truth = item.beats.empty() ? globalTruth : localTempo(item.beats, sampleT[i], globalTruth);
        if (within(sampleBpm[i], truth, 0.04, true)) {
            if (runStart < 0) runStart = sampleT[i];
            if (sampleT[i] - runStart >= 5.0) { res.lock = runStart; break; }
        } else runStart = -1;
    }
    // Jitter: mean absolute deviation from a 4 s running median.
    if (settled.size() > 40) {
        double dev = 0;
        for (size_t i = 20; i + 20 < settled.size(); ++i) {
            std::vector<double> w(settled.begin() + long(i - 20), settled.begin() + long(i + 20));
            dev += std::fabs(settled[i] - median(w));
        }
        res.jitter = dev / double(settled.size() - 40);
    }
    if (!item.beats.empty()) {
        // Where pulses fall relative to the true beats (as a fraction of the
        // local beat period): on the beat (0), the off-beat (0.5), or between.
        int onCount = 0, offCount = 0, total = 0;
        double sum = 0, sumSq = 0;
        for (double p : pulses) {
            if (p < 5.0) continue;
            auto it = std::upper_bound(item.beats.begin(), item.beats.end(), p);
            if (it == item.beats.begin() || it == item.beats.end()) continue;
            const double b0 = *(it - 1), b1 = *it, period = b1 - b0;
            double frac = (p - b0) / period;
            if (frac > 0.5) frac -= 1;  // -0.5..0.5 around the nearer beat
            ++total;
            if (std::fabs(frac) > 0.35) ++offCount;
            if (std::fabs(frac) < 0.15) { ++onCount; sum += frac * period; sumSq += frac * period * frac * period; }
        }
        if (total) res.offbeat = double(offCount) / total;
        if (g_dump) {
            std::printf("dump %s: shown BPM / pulse offset from the nearest true beat (ms), per second\n", item.name.c_str());
            for (double sec = 5; sec < duration; sec += 1.0) {
                std::printf("  %4.0fs bpm %6.2f truth %6.2f |", sec, sampleBpm[std::min(sampleBpm.size() - 1, size_t(sec * 10))],
                            localTempo(item.beats, sec, globalTruth));
                for (double p : pulses) {
                    if (p < sec || p >= sec + 1) continue;
                    auto it = std::lower_bound(item.beats.begin(), item.beats.end(), p);
                    double d = 1e9;
                    if (it != item.beats.end()) d = *it - p;
                    if (it != item.beats.begin() && std::fabs(*(it - 1) - p) < std::fabs(d)) d = *(it - 1) - p;
                    std::printf(" %+5.0f", -1000 * d);
                }
                std::printf("\n");
            }
        }
        if (onCount > 1) res.spreadMs = 1000 * std::sqrt(std::max(0.0, sumSq / onCount - (sum / onCount) * (sum / onCount)));
        res.beatF = fMeasure(beats, item.beats, 0.07, 5.0);
        std::vector<double> offsets;
        res.pulseF = fMeasure(pulses, item.beats, 0.07, 5.0, &offsets);
        if (!offsets.empty()) {
            double s = 0, a = 0;
            for (double o : offsets) { s += o; a += std::fabs(o); }
            res.pulseBiasMs = 1000 * s / offsets.size();
            res.pulseAbsMs = 1000 * a / offsets.size();
        }
    }
    return res;
}

// ---- Oracle: offline tempo of a whole track --------------------------------
// Spectral flux over the whole file, then every tempo 60-200 BPM in 0.02
// steps scored by how strongly a pulse grid (at its best phase) lands on
// onsets across the whole track. Hindsight and no real-time limits: an
// independent check on a track's labelled tempo.
static void oracle(const Item& item) {
    const size_t frames = item.audio.size() / size_t(item.channels);
    std::vector<float> mono(frames);
    for (size_t i = 0; i < frames; ++i) {
        float s = 0;
        for (int c = 0; c < item.channels; ++c) s += item.audio[i * item.channels + c];
        mono[i] = s / item.channels;
    }
    const int N = 2048, hop = 256;
    const double fps = item.sampleRate / hop;
    kiss_fft_cfg cfg = kiss_fft_alloc(N, 0, nullptr, nullptr);
    std::vector<kiss_fft_cpx> in(N), out(N);
    std::vector<float> prev(N / 2 + 1, 0.f), env;
    for (size_t start = 0; start + N <= frames; start += hop) {
        for (int i = 0; i < N; ++i) { in[i].r = mono[start + i] * float(0.5 - 0.5 * std::cos(2 * 3.14159265358979 * i / N)); in[i].i = 0; }
        kiss_fft(cfg, in.data(), out.data());
        double flux = 0;
        for (int k = 1; k <= N / 2; ++k) {
            const float m = std::log1p(100.f * std::sqrt(out[k].r * out[k].r + out[k].i * out[k].i));
            flux += std::max(0.f, m - prev[size_t(k)]);
            prev[size_t(k)] = m;
        }
        env.push_back(float(flux));
    }
    kiss_fft_free(cfg);
    double mean = 0;
    for (float v : env) mean += v;
    mean /= env.size();
    for (float& v : env) v = std::max(0.f, float(v - mean));
    auto at = [&](double pos) {
        if (pos < 0 || pos + 1 >= env.size()) return 0.0;
        const size_t i = size_t(pos);
        const double f = pos - i;
        return env[i] * (1 - f) + env[i + 1] * f;
    };
    std::vector<std::pair<double, double>> scores;
    for (double bpm = 60; bpm <= 200; bpm += 0.02) {
        const double period = 60.0 * fps / bpm;
        double best = 0;
        for (double phi = 0; phi < period; phi += 1.0) {
            double s = 0;
            int n = 0;
            for (double pos = phi; pos < env.size(); pos += period) { s += at(pos); ++n; }
            best = std::max(best, s / n);
        }
        scores.push_back({bpm, best});
    }
    // Local maxima, best first.
    std::vector<std::pair<double, double>> peaks;
    for (size_t i = 1; i + 1 < scores.size(); ++i)
        if (scores[i].second > scores[i - 1].second && scores[i].second >= scores[i + 1].second) peaks.push_back(scores[i]);
    std::sort(peaks.begin(), peaks.end(), [](auto& a, auto& b) { return a.second > b.second; });
    std::printf("oracle %-28s label %6.1f |", item.name.substr(0, 28).c_str(), item.tempo);
    for (size_t i = 0; i < std::min<size_t>(5, peaks.size()); ++i) std::printf(" %7.2f (%.3f)", peaks[i].first, peaks[i].second);
    std::printf("\n");
}

// ---- Loading ---------------------------------------------------------------

static bool loadAudio(const fs::path& path, std::vector<float>& out, int& channels, double& rate) {
    const std::string ext = path.extension().string();
    if (ext == ".wav") {
        unsigned ch = 0, sr = 0;
        drwav_uint64 n = 0;
        float* data = drwav_open_file_and_read_pcm_frames_f32(path.string().c_str(), &ch, &sr, &n);
        if (!data) return false;
        out.assign(data, data + n * ch);
        drwav_free(data);
        channels = int(ch); rate = sr;
        return true;
    }
    if (ext == ".mp3") {
        drmp3_config cfg{};  // zero: the file's own channels and rate
        drmp3_uint64 n = 0;
        float* data = drmp3_open_file_and_read_f32(path.string().c_str(), &cfg, &n);
        if (!data) return false;
        out.assign(data, data + n * cfg.outputChannels);
        drmp3_free(data);
        channels = int(cfg.outputChannels); rate = cfg.outputSampleRate;
        return true;
    }
    return false;
}

// Re-shapes audio to what the simulated source delivers: `channels` channels at `rate`.
static void conform(Item& item, int channels, double rate) {
    const size_t frames = item.audio.size() / size_t(item.channels);
    std::vector<float> mono(frames);
    for (size_t i = 0; i < frames; ++i) {
        float s = 0;
        for (int c = 0; c < item.channels; ++c) s += item.audio[i * item.channels + c];
        mono[i] = s / item.channels;
    }
    if (rate > 0 && std::fabs(rate - item.sampleRate) > 1) {  // linear resampling (test-grade)
        const size_t outFrames = size_t(frames * rate / item.sampleRate);
        std::vector<float> res(outFrames);
        for (size_t i = 0; i < outFrames; ++i) {
            const double x = i * item.sampleRate / rate;
            const size_t k = size_t(x);
            const double fr = x - k;
            res[i] = float(mono[std::min(k, frames - 1)] * (1 - fr) + mono[std::min(k + 1, frames - 1)] * fr);
        }
        mono.swap(res);
        item.sampleRate = rate;
    }
    item.audio.assign(mono.size() * size_t(channels), 0.f);
    for (size_t i = 0; i < mono.size(); ++i)
        for (int c = 0; c < channels; ++c) item.audio[i * channels + c] = mono[i];
    item.channels = channels;
}

static std::string categoryOf(const std::string& synthName) {
    const auto space = synthName.find(' ');
    const std::string first = synthName.substr(0, space);
    static const std::map<std::string, std::string> live = {
        {"swing", "live"}, {"breakdown", "live"}, {"ramp", "live"}, {"jump", "live"}, {"crowd", "live"},
        {"reverb", "live"}, {"quiet", "live"}, {"clipped", "live"}, {"live", "live"}};
    auto it = live.find(first);
    return it != live.end() ? "live:" + first : first;
}

// ---- Main ------------------------------------------------------------------

int main(int argc, char** argv) {
    std::string algo = "legacy", csvPath, ballroom, tracks, only;
    int channels = 1, block = 512, limit = 0;
    double rate = 0;  // 0: each item's own rate
    bool synthetic = false, verbose = false;
    for (int i = 1; i < argc; ++i) {
        const std::string a = argv[i];
        auto next = [&] { return i + 1 < argc ? std::string(argv[++i]) : std::string(); };
        if (a == "--algo") algo = next();
        else if (a == "--rate") rate = std::stod(next());
        else if (a == "--channels") channels = std::stoi(next());
        else if (a == "--block") block = std::stoi(next());
        else if (a == "--limit") limit = std::stoi(next());
        else if (a == "--synthetic") synthetic = true;
        else if (a == "--ballroom") ballroom = next();
        else if (a == "--tracks") tracks = next();
        else if (a == "--csv") csvPath = next();
        else if (a == "--verbose") verbose = true;
        else if (a == "--only") only = next();  // items whose name contains this
        else if (a == "--dump") g_dump = true;
        else if (a == "--oracle") g_oracle = true;
    }

    std::vector<Result> results;
    auto run = [&](Item item) {
        if (!only.empty() && item.name.find(only) == std::string::npos) return;
        if (g_oracle) { oracle(item); return; }
        conform(item, channels, rate);
        auto tracker = makeTracker(algo);
        Result r = evaluate(*tracker, item, block);
        if (verbose)
            std::printf("%-10s %-34s truth %6.1f  final %6.1f  acc1 %3.0f%%  acc2 %3.0f%%  exact %3.0f%%  lock %5.1f  jit %5.2f  beatF %5.2f  pulseF %5.2f  bias %+5.0fms\n",
                        r.dataset.c_str(), r.name.substr(0, 34).c_str(), r.truthTempo, r.finalBpm, 100 * r.acc1, 100 * r.acc2,
                        100 * r.exact, r.lock, r.jitter, r.beatF, r.pulseF, r.pulseBiasMs);
        results.push_back(r);
    };

    if (synthetic) {
        for (const auto& c : synth::suite()) {
            const auto rendered = synth::render(c, rate > 0 ? rate : 44100);
            Item item{"synthetic", categoryOf(c.name), c.name, rendered.mono, 1, rendered.sampleRate, rendered.beats};
            run(std::move(item));
        }
    }
    if (!ballroom.empty()) {
        int n = 0;
        std::vector<fs::path> wavs;
        for (const auto& e : fs::recursive_directory_iterator(fs::path(ballroom) / "BallroomData"))
            if (e.path().extension() == ".wav") wavs.push_back(e.path());
        std::sort(wavs.begin(), wavs.end());
        // An even spread across the set when limited.
        const size_t stride = limit > 0 ? std::max<size_t>(1, wavs.size() / size_t(limit)) : 1;
        for (size_t k = 0; k < wavs.size(); k += stride) {
            const auto& wav = wavs[k];
            const fs::path ann = fs::path(ballroom) / "BallroomAnnotations-master" / (wav.stem().string() + ".beats");
            std::ifstream in(ann);
            if (!in) continue;
            Item item{"ballroom", wav.parent_path().filename().string(), wav.stem().string()};
            std::string line;
            while (std::getline(in, line)) {
                std::istringstream ls(line);
                double t;
                if (ls >> t) item.beats.push_back(t);
            }
            if (!loadAudio(wav, item.audio, item.channels, item.sampleRate)) continue;
            run(std::move(item));
            if (limit > 0 && ++n >= limit) break;
        }
    }
    if (!tracks.empty()) {
        for (const auto& e : fs::directory_iterator(tracks)) {
            if (e.path().extension() != ".mp3") continue;
            const std::string stem = e.path().stem().string();
            Item item{"tracks", "tracks", stem};
            item.tempo = std::stod(stem.substr(0, 3));
            if (!loadAudio(e.path(), item.audio, item.channels, item.sampleRate)) continue;
            run(std::move(item));
        }
    }

    // Summary per dataset and category.
    std::map<std::string, std::vector<const Result*>> groups;
    for (const auto& r : results) {
        groups[r.dataset].push_back(&r);
        groups[r.dataset + " / " + r.category].push_back(&r);
    }
    std::printf("\n%-34s %4s %6s %6s %6s %6s %6s %6s %6s %6s %6s %7s %7s\n", algo.c_str(), "n", "Acc1", "Acc2", "exact", "fin1", "fin2",
                "lock", "jitter", "beatF", "pulseF", "bias", "|err|");
    for (const auto& [key, rs] : groups) {
        double acc1 = 0, acc2 = 0, ex = 0, f1 = 0, f2 = 0, lock = 0, jit = 0, bf = 0, pf = 0, bias = 0, abs = 0;
        int locked = 0, withBeats = 0;
        for (const Result* r : rs) {
            acc1 += r->acc1; acc2 += r->acc2; ex += r->exact; f1 += r->finalAcc1; f2 += r->finalAcc2; jit += r->jitter;
            if (r->lock >= 0) { lock += r->lock; ++locked; }
            if (r->beatF >= 0) { bf += r->beatF; pf += r->pulseF; bias += r->pulseBiasMs; abs += r->pulseAbsMs; ++withBeats; }
        }
        const double n = double(rs.size());
        std::printf("%-34s %4zu %5.0f%% %5.0f%% %5.0f%% %5.0f%% %5.0f%% %5.1fs %6.2f %6.2f %6.2f %+6.0fms %6.0fms\n",
                    key.substr(0, 34).c_str(), rs.size(), 100 * acc1 / n, 100 * acc2 / n, 100 * ex / n, 100 * f1 / n, 100 * f2 / n,
                    locked ? lock / locked : -1.0, jit / n, withBeats ? bf / withBeats : -1.0, withBeats ? pf / withBeats : -1.0,
                    withBeats ? bias / withBeats : 0.0, withBeats ? abs / withBeats : 0.0);
    }
    if (!csvPath.empty()) {
        std::ofstream csv(csvPath);
        csv << "dataset,category,name,truth,final,acc1,acc2,exact,fin1,fin2,lock,jitter,beatF,pulseF,biasMs,absMs,offbeat,spreadMs\n";
        for (const auto& r : results)
            csv << r.dataset << ',' << r.category << ",\"" << r.name << "\"," << r.truthTempo << ',' << r.finalBpm << ',' << r.acc1 << ','
                << r.acc2 << ',' << r.exact << ',' << r.finalAcc1 << ',' << r.finalAcc2 << ',' << r.lock << ',' << r.jitter << ','
                << r.beatF << ',' << r.pulseF << ',' << r.pulseBiasMs << ',' << r.pulseAbsMs << ',' << r.offbeat << ',' << r.spreadMs << '\n';
    }
    return 0;
}

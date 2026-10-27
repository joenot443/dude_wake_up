// Tap tempo evaluation (bpm_eval --taps): simulated people tapping along,
// scored on the tempo and beat position each tapper reports after each tap.
#include "TapTempo.hpp"

#include <algorithm>
#include <cmath>
#include <cstdio>
#include <random>
#include <string>
#include <vector>

namespace {

// The tapper the app used before TapTempo (msa::BPMTapper): the mean interval
// since the first tap, starting over after 4 s; the beat on the last tap.
struct OldTapper {
    std::vector<double> taps;
    TapTempo::Result tap(double t) {
        if (!taps.empty() && t - taps.back() > 4) taps.clear();
        taps.push_back(t);
        TapTempo::Result r;
        r.taps = int(taps.size());
        if (taps.size() >= 2) {
            r.valid = true;
            r.bpm = 60.0 * (taps.size() - 1) / (taps.back() - taps.front());
            r.beatTime = taps.back();
        }
        return r;
    }
};

struct Scenario {
    std::string name;
    double bpm, jitter;  // jitter: std dev of tap timing, seconds
    int missAt = -1, doubleAt = -1, slipAt = -1;
    double changeTo = 0;  // a new tempo from tap 9
};

struct Tap { double time; double trueBeat; double trueBpm; };

std::vector<Tap> simulate(const Scenario& s, std::mt19937& rng) {
    std::normal_distribution<double> wobble(0.0, s.jitter);
    std::vector<Tap> taps;
    double beat = 1.0, bpm = s.bpm;
    for (int i = 0; i < 16; ++i) {
        if (s.changeTo > 0 && i == 8) bpm = s.changeTo;
        if (i == s.missAt) { beat += 60.0 / bpm; continue; }  // a beat with no tap
        double t = beat + wobble(rng);
        if (i == s.slipAt) t += 0.3 * 60.0 / bpm;               // a badly late tap
        taps.push_back({t, beat, bpm});
        if (i == s.doubleAt) taps.push_back({t + 0.06, beat, bpm});  // finger bounce
        beat += 60.0 / bpm;
    }
    return taps;
}

}  // namespace

void runTapEvaluation() {
    std::vector<Scenario> scenarios;
    for (double bpm : {70.0, 90.0, 120.0, 128.0, 140.0, 174.0})
        for (double j : {0.010, 0.020, 0.035}) scenarios.push_back({"clean", bpm, j});
    for (double bpm : {90.0, 128.0, 174.0}) {
        scenarios.push_back({"missed tap", bpm, 0.020, 5});
        scenarios.push_back({"double tap", bpm, 0.020, -1, 5});
        scenarios.push_back({"slip", bpm, 0.020, -1, -1, 5});
    }
    scenarios.push_back({"change 120->128", 120, 0.020, -1, -1, -1, 128});
    scenarios.push_back({"change 128->140", 128, 0.020, -1, -1, -1, 140});

    struct Stats { std::vector<double> bpmErr[17], phaseErr[17]; };
    auto summarise = [](std::vector<double> v) {
        if (v.empty()) return std::make_pair(0.0, 0.0);
        std::sort(v.begin(), v.end());
        return std::make_pair(v[v.size() / 2], v[size_t(v.size() * 0.9)]);
    };
    std::printf("\ntap tempo: |BPM error| %% and |beat error| ms after k taps (median / 90th percentile over 300 trials)\n");
    std::printf("%-26s %-6s %-17s %-17s %-17s %-17s %s\n", "scenario", "tapper", "k=4 bpm", "k=8 bpm", "k=16 bpm", "k=8 beat ms", "within 0.5 BPM at k=8");
    for (const auto& s : scenarios) {
        for (int which = 0; which < 2; ++which) {
            Stats st;
            int exact8 = 0, trials = 300;
            for (int trial = 0; trial < trials; ++trial) {
                std::mt19937 rng(unsigned(trial * 7919 + int(s.bpm) * 31 + int(s.jitter * 1000)));
                const auto taps = simulate(s, rng);
                OldTapper old;
                TapTempo fresh;
                int k = 0;
                for (const Tap& tap : taps) {
                    const TapTempo::Result r = which == 0 ? old.tap(tap.time) : fresh.tap(tap.time);
                    if (tap.time != taps.front().time && &tap != &taps.front() && tap.time - (&tap - 1)->time < 0.1) continue;  // the bounce itself
                    ++k;
                    if (!r.valid || k > 16) continue;
                    st.bpmErr[k].push_back(100.0 * std::fabs(r.bpm - tap.trueBpm) / tap.trueBpm);
                    st.phaseErr[k].push_back(1000.0 * std::fabs(r.beatTime - tap.trueBeat));
                    if (k == 8 && std::fabs(r.bpm - tap.trueBpm) <= 0.5) ++exact8;
                }
            }
            auto at = [&](int k) { auto m = summarise(st.bpmErr[k]); char b[32]; std::snprintf(b, sizeof b, "%5.2f / %5.2f", m.first, m.second); return std::string(b); };
            auto p8 = summarise(st.phaseErr[8]);
            char label[40];
            std::snprintf(label, sizeof label, "%s %.0f %.0fms", s.name.c_str(), s.bpm, s.jitter * 1000);
            std::printf("%-26s %-6s %-17s %-17s %-17s %5.0f / %5.0f      %3.0f%%\n", which == 0 ? label : "", which == 0 ? "old" : "new",
                        at(4).c_str(), at(8).c_str(), at(16).c_str(), p8.first, p8.second, 100.0 * exact8 / trials);
        }
    }
}

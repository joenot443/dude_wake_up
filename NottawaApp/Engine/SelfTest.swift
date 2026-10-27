//
//  SelfTest.swift
//  NottawaApp
//
//  Scripted checks run inside the real app, so the Mac build can be verified
//  without anyone at the keyboard (the Windows app's equivalent is
//  StressTest.cpp). Launch with NOTTAWA_SELFTEST set to a comma-separated
//  list of scenarios:
//
//    files          every sample track in Auto mode: BPM and beat phase
//    system         sample tracks played in-process, heard via System Audio
//    tap            Manual mode taps at 100 and 128 BPM
//    tapauto        Auto mode, Velvet Skies (87) tapped at 174
//    persist-set    change a parameter and quit at once
//    persist-check  (next launch) the change survived; then it's put back
//
//  NOTTAWA_SELFTEST_SECONDS sets the listening time per track (default 40).
//  Results go to <container tmp>/selftest/: report.txt, plus a CSV of
//  samples per audio scenario. scripts/mac_selftest.sh runs and scores it.
//  The app quits when done, with the audio settings it started with.
//

import AppKit
import AVFoundation
import Foundation

@MainActor
final class SelfTest {
    private static var running: SelfTest?

    static func startIfRequested() {
        guard let list = ProcessInfo.processInfo.environment["NOTTAWA_SELFTEST"], !list.isEmpty else { return }
        let test = SelfTest(scenarios: list.split(separator: ",").map { $0.trimmingCharacters(in: .whitespaces) })
        running = test
        Task { await test.run() }
    }

    private let scenarios: [String]
    private let engine = NottawaEngine.shared
    private let dir = FileManager.default.temporaryDirectory.appendingPathComponent("selftest")
    private let seconds = Double(ProcessInfo.processInfo.environment["NOTTAWA_SELFTEST_SECONDS"] ?? "") ?? 40
    private var report = ""
    private var sampler: Sampler?

    private init(scenarios: [String]) {
        self.scenarios = scenarios
    }

    // MARK: - Flow

    private func run() async {
        try? FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true)
        log("start \(scenarios.joined(separator: ",")) dir=\(dir.path)")
        await wait(3)  // let the session finish loading

        let saved = AudioState(engine)
        for scenario in scenarios {
            switch scenario {
            case "files": await files()
            case "system": await system()
            case "tap": await tap()
            case "tapauto": await tapAuto()
            case "persist-set": persistSet(); return  // quits right away, on purpose
            case "persist-check": persistCheck()
            default: log("FAIL unknown scenario \(scenario)")
            }
        }
        saved.restore(engine)
        await wait(0.5)
        log("done")
        NSApp.terminate(nil)
    }

    private func log(_ line: String) {
        NSLog("[selftest] %@", line)
        report += line + "\n"
        try? report.write(to: dir.appendingPathComponent("report.txt"), atomically: true, encoding: .utf8)
    }

    private func wait(_ s: Double) async {
        try? await Task.sleep(nanoseconds: UInt64(s * 1e9))
    }

    private func audioSource(type: Int) -> AudioSourceInfo? {
        engine.allAudioSources().first { $0.sourceType == type }
    }

    private func ensureAudioActive() {
        if !engine.isAudioActive { engine.toggleAudioSource() }
    }

    // MARK: - Audio scenarios

    /// Each sample track from the start in Auto mode, sampled from the File source.
    private func files() async {
        guard let file = audioSource(type: 1) else { return log("FAIL files: no File audio source") }
        engine.selectAudioSource(id: file.id)
        engine.setBpmEnabled(true)
        engine.setBpmMode(BpmMode.auto.rawValue)
        engine.setBpmLocked(false)
        for track in engine.sampleTracks() {
            engine.selectSampleTrack(track.index)
            engine.setFileAudioPosition(0)
            ensureAudioActive()
            if engine.fileAudioState().isPaused { engine.toggleFileAudioPause() }
            await listen(label: "files", track: track, clock: { [engine] in
                let s = engine.fileAudioState()
                return Double(s.playbackPosition) * Double(s.totalDuration)
            })
        }
    }

    /// Sample tracks played by this process through the output device and
    /// picked up by the System Audio tap, as music from another app would be.
    private func system() async {
        guard let sys = audioSource(type: 2) else { return log("FAIL system: no System audio source") }
        engine.selectAudioSource(id: sys.id)
        engine.setBpmEnabled(true)
        engine.setBpmMode(BpmMode.auto.rawValue)
        engine.setBpmLocked(false)
        ensureAudioActive()
        let picks = ProcessInfo.processInfo.environment["NOTTAWA_SELFTEST_SYSTEM_TRACKS"]?
            .split(separator: ",").map(String.init) ?? ["Nottawa Song", "Bass Cascade", "Eclipse", "Velvet Skies"]
        for track in engine.sampleTracks() where picks.contains(where: { track.name.contains($0) }) {
            guard let player = try? AVAudioPlayer(contentsOf: URL(fileURLWithPath: track.path)) else {
                log("FAIL system: can't play \(track.path)")
                continue
            }
            player.prepareToPlay()
            player.play()
            // currentTime is read on the sampler thread; AVAudioPlayer tolerates that for reads.
            nonisolated(unsafe) let p = player
            await listen(label: "system", track: track, clock: { p.isPlaying ? p.currentTime : -1 })
            player.stop()
            await wait(2)  // a gap, so the tracker's reset-after-silence starts each track fresh
        }
    }

    /// Records `seconds` of what the app shows, with the track's own clock
    /// alongside, then logs a one-line summary (scored properly offline).
    private func listen(label: String, track: AudioTrackInfo, clock: @escaping @Sendable () -> Double) async {
        let csv = dir.appendingPathComponent("\(label)-\(String(format: "%03d", track.bpm))-\(track.name).csv")
        let sampler = Sampler(engine: engine, clock: clock)
        self.sampler = sampler
        sampler.start()
        await wait(seconds)
        sampler.stop()
        self.sampler = nil
        let rows = sampler.rows
        try? sampler.csv().write(to: csv, atomically: true, encoding: .utf8)

        // Shown BPM over the last half, against the track's labeled tempo.
        let late = rows.suffix(rows.count / 2).map(\.bpm)
        let median = late.sorted()[safe: late.count / 2] ?? 0
        let truth = Float(track.bpm)
        let within4 = late.isEmpty ? 0 : Float(late.filter { abs($0 - truth) <= 0.04 * truth }.count) / Float(late.count)
        let loud = rows.map(\.rms).max() ?? 0
        log(String(format: "%@ %@ (%d): median bpm %.2f, within 4%% %.0f%% of 2nd half, beats %u, peak rms %.3f -> %@",
                   label, track.name, track.bpm, median, within4 * 100,
                   (rows.last?.beatCount ?? 0) &- (rows.first?.beatCount ?? 0), loud, csv.lastPathComponent))
        if loud < 0.001 { log("FAIL \(label) \(track.name): no audio reached the analyser") }
    }

    // MARK: - Tap

    /// Manual mode: taps through the same call as the Tap button, timed on
    /// a thread of their own.
    private func tap() async {
        engine.setBpmEnabled(true)
        engine.setBpmMode(BpmMode.manual.rawValue)
        for bpm in [100.0, 128.0] {
            await tapThread(bpm: bpm, count: 8)
            await wait(0.3)
            let shown = engine.extendedAudioAnalysis().bpm
            let ok = abs(Double(shown) - bpm) <= 0.01 * bpm
            log(String(format: "%@ tap manual %.0f: shows %.2f", ok ? "PASS" : "FAIL", bpm, shown))
            await wait(2.5)  // a pause, so the next run is a new tap sequence
        }
    }

    /// Auto mode on Velvet Skies (87), tapped at double tempo: the tracker
    /// should follow the taps to 174.
    private func tapAuto() async {
        guard let file = audioSource(type: 1),
              let track = engine.sampleTracks().first(where: { $0.name.contains("Velvet") }) else {
            return log("FAIL tapauto: no File source or Velvet Skies")
        }
        engine.selectAudioSource(id: file.id)
        engine.setBpmEnabled(true)
        engine.setBpmMode(BpmMode.auto.rawValue)
        engine.setBpmLocked(false)
        engine.selectSampleTrack(track.index)
        engine.setFileAudioPosition(0)
        ensureAudioActive()
        await wait(15)
        let before = engine.extendedAudioAnalysis().bpm
        await tapThread(bpm: 174, count: 8)
        var after: [Float] = []
        for _ in 0..<8 {
            await wait(1)
            after.append(engine.extendedAudioAnalysis().bpm)
        }
        let ok = after.suffix(4).allSatisfy { abs($0 - 174) <= 0.04 * 174 }
        log(String(format: "%@ tapauto: before %.2f, after taps at 174: %@", ok ? "PASS" : "FAIL", before,
                   after.map { String(format: "%.1f", $0) }.joined(separator: " ")))
    }

    private func tapThread(bpm: Double, count: Int) async {
        await withCheckedContinuation { (done: CheckedContinuation<Void, Never>) in
            Thread {
                let start = Date().addingTimeInterval(0.05)
                for i in 0..<count {
                    Thread.sleep(until: start.addingTimeInterval(Double(i) * 60 / bpm))
                    ntw_tap_bpm()
                }
                done.resume()
            }.start()
        }
    }

    // MARK: - Persistence

    private struct PersistState: Codable {
        var paramId: String
        var name: String
        var original: Float
        var value: Float
        var addedSourceId: String?
    }

    private var persistURL: URL { dir.appendingPathComponent("persist.json") }

    /// A float parameter on a node in the session, set to a new value, then
    /// quit straight away: only the save on quit can catch it.
    private func persistSet() {
        var added: String?
        var param = firstFloatParameter()
        if param == nil, let type = engine.availableShaderSources().first?.shaderTypeRaw {
            added = engine.addShaderVideoSource(type: type)
            param = firstFloatParameter()
        }
        guard let p = param else {
            log("FAIL persist-set: no float parameter found")
            NSApp.terminate(nil)
            return
        }
        var value = p.minValue + (p.maxValue - p.minValue) * 0.618
        if abs(value - p.value) < 0.05 * (p.maxValue - p.minValue) {
            value = p.minValue + (p.maxValue - p.minValue) * 0.382
        }
        engine.setParameterValue(paramId: p.id, value: value)
        let state = PersistState(paramId: p.id, name: p.name, original: p.value, value: value, addedSourceId: added)
        try? JSONEncoder().encode(state).write(to: persistURL)
        log(String(format: "persist-set %@ (%@): %.4f -> %.4f, quitting", p.name, p.id, p.value, value))
        NSApp.terminate(nil)
    }

    private func persistCheck() {
        guard let data = try? Data(contentsOf: persistURL),
              let state = try? JSONDecoder().decode(PersistState.self, from: data) else {
            return log("FAIL persist-check: no persist.json from persist-set")
        }
        guard let p = allParameters().first(where: { $0.id == state.paramId }) else {
            return log("FAIL persist-check: parameter \(state.paramId) is gone")
        }
        let ok = abs(p.value - state.value) <= 1e-4 * max(1, abs(state.value))
        log(String(format: "%@ persist: %@ is %.4f after relaunch (set %.4f, was %.4f)",
                   ok ? "PASS" : "FAIL", state.name, p.value, state.value, state.original))
        if let added = state.addedSourceId {
            engine.removeConnectable(id: added)
        } else {
            engine.setParameterValue(paramId: state.paramId, value: state.original)
        }
        try? FileManager.default.removeItem(at: persistURL)
    }

    private func allParameters() -> [ParameterInfo] {
        let ids = engine.allVideoSources().map(\.id) + engine.allShaders().map(\.id)
        return ids.flatMap { engine.parameters(for: $0) }
    }

    private func firstFloatParameter() -> ParameterInfo? {
        // One nothing else moves: an oscillator or audio driver would change it on its own.
        allParameters().first {
            $0.parameterType == .standard && $0.maxValue > $0.minValue && !$0.oscillatorEnabled && !$0.hasDriver
        }
    }
}

// MARK: - Audio state

/// The audio settings a run starts with, put back before it quits so the
/// session saved on quit is the user's, not the test's.
private struct AudioState {
    let sourceId: String?
    let active: Bool
    let bpmMode: BpmMode
    let bpmLocked: Bool
    let bpmEnabled: Bool
    let trackIndex: Int

    @MainActor init(_ engine: NottawaEngine) {
        let a = engine.extendedAudioAnalysis()
        sourceId = engine.selectedAudioSourceId
        active = engine.isAudioActive
        bpmMode = a.bpmMode
        bpmLocked = a.bpmLocked
        bpmEnabled = a.bpmEnabled
        trackIndex = engine.fileAudioState().selectedTrackIndex
    }

    @MainActor func restore(_ engine: NottawaEngine) {
        if let file = engine.allAudioSources().first(where: { $0.sourceType == 1 }) {
            engine.selectAudioSource(id: file.id)
            engine.selectSampleTrack(trackIndex)
        }
        if let id = sourceId { engine.selectAudioSource(id: id) }
        if engine.isAudioActive != active { engine.toggleAudioSource() }
        engine.setBpmMode(bpmMode.rawValue)
        engine.setBpmLocked(bpmLocked)
        engine.setBpmEnabled(bpmEnabled)
    }
}

// MARK: - Sampler

/// Reads what the app shows (BPM, beat pulse, beat count) every 4 ms on a
/// thread of its own, with the track's clock, for scoring offline.
private final class Sampler: @unchecked Sendable {
    struct Row {
        var wall: Double
        var trackTime: Double
        var bpm: Float
        var pulse: Float
        var beatCount: UInt32
        var rms: Float
    }

    private let clock: @Sendable () -> Double
    private let lock = NSLock()
    private var _rows: [Row] = []
    private var stopped = false

    init(engine: NottawaEngine, clock: @escaping @Sendable () -> Double) {
        self.clock = clock
    }

    var rows: [Row] { lock.withLock { _rows } }

    func start() {
        Thread { [self] in
            _rows.reserveCapacity(20000)
            while !lock.withLock({ stopped }) {
                let a = ntw_get_extended_audio_analysis()
                let row = Row(wall: CACurrentMediaTime(), trackTime: clock(), bpm: a.bpm, pulse: a.beatPulse,
                              beatCount: a.beatCount, rms: a.rms)
                lock.withLock { _rows.append(row) }
                usleep(4000)
            }
        }.start()
    }

    func stop() { lock.withLock { stopped = true } }

    func csv() -> String {
        var out = "wall,track_time,bpm,pulse,beat_count,rms\n"
        for r in rows {
            out += String(format: "%.6f,%.6f,%.3f,%.5f,%u,%.5f\n", r.wall, r.trackTime, r.bpm, r.pulse, r.beatCount, r.rms)
        }
        return out
    }
}

private extension Array {
    subscript(safe i: Int) -> Element? { indices.contains(i) ? self[i] : nil }
}

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
//    settings       every setting set and read back through the bridge, with
//                   clamping, the change callback, side effects, legacy
//                   setters, section reset and the UserDefaults migration
//    settings-set   change some settings and quit at once
//    settings-check (next launch) they survived; then they're put back
//    settingsui     the Settings window rendered per section, searching and
//                   with a dependent row off, plus the main window per theme
//    audiosettings  tempo ranges on Velvet Skies, and a File beat offset
//                   scored against the track's beat
//    audiolaunch-set / audiolaunch-check  start-on-launch and the remembered
//                   input across a relaunch; then put back
//    outputwindows  output windows recorded for reopening, and reopened by the
//                   rules (saved frame, centred on its display, display gone)
//    outputwindows-set / outputwindows-check  a window reopened at launch
//                   where it was at quit; then closed
//    diagnostics    the Copy Diagnostics text: its parts, and nothing naming the user
//    diagnostics-set / diagnostics-check  an abrupt exit, then the relaunch
//                   reports the previous session didn't quit normally
//    golden         every effect (fed by Plasma) and generated source, rendered
//                   deterministically and compared with NOTTAWA_GOLDEN_DIR
//                   (NOTTAWA_GOLDEN=record|stabilize); see docs/TESTING.md
//    workflows      a chain, an optional effect, Mask, a workspace round trip,
//                   1080p, undo/redo, checked by rendered pixels
//    cachesettings  a video clip (NOTTAWA_SELFTEST_CLIP, ~1.1 GB to cache)
//                   loaded under each Cache Video Clips / Cache Memory choice
//
//  NOTTAWA_SELFTEST_SECONDS sets the listening time per track (default 40).
//  Results go to <container tmp>/selftest/: report.txt, plus a CSV of
//  samples per audio scenario. scripts/mac_selftest.sh runs and scores it.
//  The app quits when done, with the audio settings it started with.
//

import AppKit
import AVFoundation
import CoreAudio
import Foundation
import ImageIO

@MainActor
final class SelfTest {
    private static var running: SelfTest?

    static func startIfRequested(viewModel: NodeEditorViewModel) {
        guard let list = ProcessInfo.processInfo.environment["NOTTAWA_SELFTEST"], !list.isEmpty else { return }
        let test = SelfTest(scenarios: list.split(separator: ",").map { $0.trimmingCharacters(in: .whitespaces) })
        test.viewModel = viewModel
        running = test
        Task { await test.run() }
    }

    private let scenarios: [String]
    private let engine = NottawaEngine.shared
    private var viewModel: NodeEditorViewModel!
    private let dir = FileManager.default.temporaryDirectory.appendingPathComponent("selftest")
    private let seconds = Double(ProcessInfo.processInfo.environment["NOTTAWA_SELFTEST_SECONDS"] ?? "") ?? 40
    private var report = ""
    /// Set by a scenario that has put the audio state back itself.
    fileprivate var keepAudioState = false
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
            case "settings": await settingsRoundTrip()
            case "settings-set": settingsSet(); return  // quits right away, on purpose
            case "settings-check": settingsCheck()
            case "settingsui": await settingsUI()
            case "audiosettings": await audioSettings()
            case "audiolaunch-set": audioLaunchSet(); return  // quits right away, on purpose
            case "audiolaunch-check": audioLaunchCheck()
            case "cachesettings": await cacheSettings()
            case "outputwindows": await outputWindows()
            case "diagnostics": diagnostics()
            case "diagnostics-set":
                log("diagnostics-set: exiting without quitting")
                exit(0)  // no willTerminate: as a crash would
            case "diagnostics-check": diagnosticsCheck()
            case "outputwindows-set": await outputWindowsSet(); return  // quits, on purpose
            case "outputwindows-check": outputWindowsCheck()
            case "texturelock": await textureLock()
            case "golden": golden()
            case "workflows": workflows()
            default: log("FAIL unknown scenario \(scenario)")
            }
        }
        if !keepAudioState { saved.restore(engine) }
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
        var tracks = engine.sampleTracks().filter { track in picks.contains(where: { track.name.contains($0) }) }
        // The calibration drum track (Tests/bpm/calib_drums.py: 128 BPM, exact
        // beats) instead, when given: scored against its known beats.
        if let calib = ProcessInfo.processInfo.environment["NOTTAWA_SELFTEST_CALIB"], !calib.isEmpty {
            tracks = [AudioTrackInfo(name: "Calibration", path: calib, bpm: 128, index: -1)]
        }
        // Measured with no offset (the capture path itself); the calibration
        // track again with the default offset, which should cancel it.
        let store = SettingsStore.shared
        let savedOffset = store.number("audio.beatOffsetSystem")
        let defaultOffset = store.info("audio.beatOffsetSystem")?.defaultNumber ?? 0
        var passes = tracks.map { ($0, 0.0, "system") }
        if tracks.first?.name == "Calibration" { passes.append((tracks[0], defaultOffset, "systemdefault")) }
        defer { store.set("audio.beatOffsetSystem", number: savedOffset) }
        for (track, offset, label) in passes {
            store.set("audio.beatOffsetSystem", number: offset)
            guard let player = try? AVAudioPlayer(contentsOf: URL(fileURLWithPath: track.path)) else {
                log("FAIL system: can't play \(track.path)")
                continue
            }
            player.prepareToPlay()
            player.play()
            // currentTime is read on the sampler thread; AVAudioPlayer tolerates that for reads.
            nonisolated(unsafe) let p = player
            await listen(label: label, track: track, clock: { p.isPlaying ? p.currentTime : -1 })
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

// MARK: - Settings

extension SelfTest {
    private var store: SettingsStore { .shared }

    /// Every value, to put back afterwards so the run leaves the user's settings as they were.
    private func snapshotSettings() -> [String: Any] {
        var out: [String: Any] = [:]
        for s in store.schema {
            switch s.kind {
            case .bool, .int, .float: out[s.id] = store.number(s.id)
            case .choice, .string: out[s.id] = store.string(s.id)
            case .path, .action: break
            }
        }
        return out
    }

    private func restoreSettings(_ saved: [String: Any]) {
        for (id, value) in saved {
            if let n = value as? Double, ntw_get_setting_number(id) != n { store.set(id, number: n) }
            if let v = value as? String, engineString(id) != v { store.set(id, string: v) }
        }
    }

    private func engineString(_ id: String) -> String {
        guard let c = ntw_get_setting_string(id) else { return "" }
        defer { ntw_free_string(c) }
        return String(cString: c)
    }

    private func expect(_ ok: Bool, _ what: String) -> Bool {
        if !ok { log("FAIL settings: \(what)") }
        return ok
    }

    func settingsRoundTrip() async {
        let rows = store.schema
        guard !rows.isEmpty else { return log("FAIL settings: no settings in the schema") }
        log("settings: \(rows.count) rows on macOS in \(store.sections.count) sections: \(store.sections.joined(separator: ", "))")
        let saved = snapshotSettings()
        var checks = 0, passed = 0
        func check(_ ok: Bool, _ what: String) { checks += 1; if expect(ok, what) { passed += 1 } }

        check(!rows.contains { $0.id == "updates.check" }, "Windows-only updates.check listed on the Mac")
        // System Audio's offset defaults to the Mac capture path (docs/BPM.md).
        check(store.info("audio.beatOffsetSystem")?.defaultNumber == 45, "System Audio offset default is \(store.info("audio.beatOffsetSystem")?.defaultNumber ?? .nan), want 45 on the Mac")
        check(UserDefaults.standard.bool(forKey: SettingsStore.migratedKey), "this launch didn't record the migration")

        for s in rows {
            let mark = store.changeLog.count
            func calledBack() -> Bool { store.changeLog[mark...].contains(s.id) }
            switch s.kind {
            case .bool:
                let was = store.bool(s.id)
                store.set(s.id, bool: !was)
                await wait(0.15)
                check(ntw_get_setting_number(s.id) == (was ? 0 : 1) && store.bool(s.id) == !was && calledBack(),
                      "\(s.id): set \(!was), engine \(ntw_get_setting_number(s.id)), store \(store.bool(s.id)), callback \(calledBack())")
                if s.id == "output.stretchPreviews" {
                    check(PreviewWindowManager.shared.stretchToFill == !was, "stretchPreviews not applied to preview windows")
                }
                store.set(s.id, bool: was)
            case .int, .float:
                let was = store.number(s.id)
                var mid = (s.min + s.max) / 2
                if mid == was { mid = (mid + s.max) / 2 }  // a change, so the callback fires
                if s.step > 0 { mid = (mid / s.step).rounded() * s.step }
                store.set(s.id, number: mid)
                await wait(0.15)
                check(abs(ntw_get_setting_number(s.id) - mid) < 1e-6 && abs(store.number(s.id) - mid) < 1e-6 && calledBack(),
                      "\(s.id): set \(mid), engine \(ntw_get_setting_number(s.id)), store \(store.number(s.id))")
                store.set(s.id, number: s.max + 1000)
                await wait(0.15)
                check(ntw_get_setting_number(s.id) == s.max && store.number(s.id) == s.max,
                      "\(s.id): over the max kept \(ntw_get_setting_number(s.id)), store \(store.number(s.id))")
                store.set(s.id, number: was)
            case .choice:
                let was = store.string(s.id)
                for (i, choice) in s.choices.enumerated() {
                    store.set(s.id, string: choice.id)
                    await wait(s.id == "output.resolution" ? 0.6 : 0.15)
                    check(engineString(s.id) == choice.id && store.string(s.id) == choice.id,
                          "\(s.id): set \(choice.id), engine \(engineString(s.id)), store \(store.string(s.id))")
                    switch s.id {
                    case "output.resolution":
                        check(Int(ntw_get_resolution_setting_index()) == i, "resolution \(choice.id): engine index \(ntw_get_resolution_setting_index())")
                        for dependent in rows where dependent.dependsOn?.hasPrefix("output.resolution=") == true {
                            check(store.isSatisfied(dependent.dependsOn!) == (choice.id == "custom"),
                                  "\(dependent.id) enabled \(store.isSatisfied(dependent.dependsOn!)) with resolution \(choice.id)")
                        }
                        if choice.id == "custom" {
                            let w0 = store.number("output.customWidth"), h0 = store.number("output.customHeight")
                            store.set("output.customWidth", number: 1000)
                            store.set("output.customHeight", number: 500)
                            await wait(0.6)
                            check(ntw_get_resolution_width() == 1000 && ntw_get_resolution_height() == 500,
                                  "custom 1000x500: engine renders \(ntw_get_resolution_width())x\(ntw_get_resolution_height())")
                            store.set("output.customWidth", number: 1200)
                            await wait(0.6)
                            check(ntw_get_resolution_width() == 1200, "custom width change: engine width \(ntw_get_resolution_width())")
                            store.set("output.customWidth", number: w0)
                            store.set("output.customHeight", number: h0)
                            await wait(0.3)
                        }
                    case "screenshots.format":
                        check(ScreenshotService.shared.format.rawValue == choice.id, "screenshot format \(choice.id) not used")
                    case "appearance.theme":
                        check(store.theme.settingId == choice.id, "theme \(choice.id) maps to \(store.theme)")
                    case "output.frameRate":
                        await wait(3)
                        let fps = Double(engine.engineFPS), target = Double(choice.id) ?? 0
                        log(String(format: "settings: frame rate %@: engine runs at %.1f fps", choice.id, fps))
                        check(abs(fps - target) <= 0.08 * target, "frame rate \(choice.id): engine at \(fps) fps")
                    default: break
                    }
                }
                store.set(s.id, string: "not-a-choice")
                await wait(0.15)
                check(s.choices.contains { $0.id == engineString(s.id) }, "\(s.id): accepted a value not in its choices (\(engineString(s.id)))")
                store.set(s.id, string: was)
                await wait(s.id == "output.resolution" ? 0.6 : 0.1)
            case .string:
                let was = store.string(s.id)
                store.set(s.id, string: "selftest")
                await wait(0.15)
                check(engineString(s.id) == "selftest", "\(s.id): string round trip")
                store.set(s.id, string: was)
            case .path, .action:
                break
            }
        }

        // The setters from before settings existed still work, and the store follows them.
        ntw_set_resolution(2)
        ntw_set_midi_smoothing_enabled(!store.bool("midi.smoothing"))
        let clear = store.bool("graph.clearOnDisconnect")
        ntw_set_clear_on_disconnect(clear ? 0 : 1)
        await wait(0.6)
        check(store.string("output.resolution") == "480p", "ntw_set_resolution(2): store shows \(store.string("output.resolution"))")
        check(store.bool("midi.smoothing") != (saved["midi.smoothing"] as? Double == 1), "ntw_set_midi_smoothing_enabled not reflected")
        check(store.bool("graph.clearOnDisconnect") == !clear, "ntw_set_clear_on_disconnect not reflected")
        restoreSettings(saved)
        await wait(0.6)

        // Resetting one section leaves the others.
        store.set("output.advanceFade", number: 1.7)
        store.set("display.nodeHints", bool: false)
        await wait(0.2)
        store.reset(section: "Output")
        await wait(0.3)
        check(store.number("output.advanceFade") == store.info("output.advanceFade")?.defaultNumber,
              "Output reset: fade is \(store.number("output.advanceFade"))")
        check(store.bool("display.nodeHints") == false, "Output reset changed Appearance's node hints")
        restoreSettings(saved)
        await wait(0.6)

        // Migration, on a defaults suite of its own.
        let suiteName = "com.joecrozier.nottawa.selftest-migration"
        if let d = UserDefaults(suiteName: suiteName) {
            d.removePersistentDomain(forName: suiteName)
            d.set(false, forKey: "showNodeHints")
            d.set(false, forKey: "autoAdvanceOutputWindow")
            d.set(1.5, forKey: "outputWindowFadeDuration")
            d.set(false, forKey: "clearOnDisconnect")
            d.set(false, forKey: "sidebarLivePreview")
            d.set("jpeg", forKey: "nottawa.screenshotFormat")
            let ids = store.migrateLegacyDefaults(d)
            await wait(0.3)
            check(ids.count == 6, "migration set \(ids.count) of 6: \(ids)")
            check(!store.bool("display.nodeHints") && !store.bool("output.autoAdvance") && store.number("output.advanceFade") == 1.5
                  && !store.bool("graph.clearOnDisconnect") && !store.bool("browser.livePreview") && store.string("screenshots.format") == "jpeg",
                  "migrated values not in the engine")
            check(store.migrateLegacyDefaults(d).isEmpty, "migration ran twice")
            d.removePersistentDomain(forName: suiteName)
            restoreSettings(saved)
            await wait(0.6)
        }
        let legacy = ["showNodeHints", "autoAdvanceOutputWindow", "outputWindowFadeDuration", "clearOnDisconnect",
                      "sidebarLivePreview", "nottawa.screenshotFormat"]
            .compactMap { k in UserDefaults.standard.object(forKey: k).map { "\(k)=\($0)" } }
        log("settings: this install's old UserDefaults values: \(legacy.isEmpty ? "none" : legacy.joined(separator: ", "))")

        let left = snapshotSettings().filter { id, v in "\(v)" != "\(saved[id] ?? "")" }
        check(left.isEmpty, "not put back: \(left)")
        log("\(passed == checks ? "PASS" : "FAIL") settings: \(passed)/\(checks) checks")
    }

    /// What the window server shows for one of our windows (SwiftUI's
    /// split views don't draw into cacheDisplay).
    private func writeWindowImage(_ window: NSWindow, to url: URL) -> Bool {
        guard let image = CGWindowListCreateImage(.null, .optionIncludingWindow, CGWindowID(window.windowNumber),
                                                  [.boundsIgnoreFraming, .bestResolution]) else { return false }
        let rep = NSBitmapImageRep(cgImage: image)
        guard let png = rep.representation(using: .png, properties: [:]) else { return false }
        return (try? png.write(to: url)) != nil
    }

    private struct SettingsPersistState: Codable {
        var saved: [String: String]  // id -> original, as text
        var expected: [String: String]
    }

    private var settingsPersistURL: URL { dir.appendingPathComponent("settings-persist.json") }

    /// Changes a few settings and quits at once: only the save on quit can catch them.
    func settingsSet() {
        let changes: [(String, String)] = [
            ("display.nodeHints", store.bool("display.nodeHints") ? "0" : "1"),
            ("output.advanceFade", "1.2"),
            ("appearance.theme", store.string("appearance.theme") == "ember" ? "arctic" : "ember"),
            ("midi.smoothingMs", "1250"),
        ]
        var saved: [String: String] = [:]
        for (id, value) in changes {
            if store.info(id)?.kind == .choice {
                saved[id] = store.string(id)
                store.set(id, string: value)
            } else {
                saved[id] = String(store.number(id))
                store.set(id, number: Double(value)!)
            }
        }
        let state = SettingsPersistState(saved: saved, expected: Dictionary(uniqueKeysWithValues: changes))
        try? JSONEncoder().encode(state).write(to: settingsPersistURL)
        log("settings-set \(changes.map { "\($0.0)=\($0.1)" }.joined(separator: " ")), quitting")
        NSApp.terminate(nil)
    }

    func settingsCheck() {
        guard let data = try? Data(contentsOf: settingsPersistURL),
              let state = try? JSONDecoder().decode(SettingsPersistState.self, from: data) else {
            return log("FAIL settings-check: no settings-persist.json from settings-set")
        }
        var bad: [String] = []
        for (id, value) in state.expected.sorted(by: { $0.key < $1.key }) {
            let now = store.info(id)?.kind == .choice ? engineString(id) : String(ntw_get_setting_number(id))
            let ok = store.info(id)?.kind == .choice ? now == value : abs((Double(now) ?? .nan) - Double(value)!) < 1e-6
            if !ok { bad.append("\(id) is \(now), set \(value)") }
        }
        log(bad.isEmpty ? "PASS settings persist: \(state.expected.count) settings survived quitting at once"
                        : "FAIL settings persist: \(bad.joined(separator: "; "))")
        for (id, value) in state.saved {
            if store.info(id)?.kind == .choice { store.set(id, string: value) } else { store.set(id, number: Double(value)!) }
        }
        try? FileManager.default.removeItem(at: settingsPersistURL)
    }

    /// Renders the Settings window per section, while searching, and with
    /// a dependent row off; and the main window in two themes.
    func settingsUI() async {
        let saved = snapshotSettings()
        let nav = SettingsNavigation.shared
        let before = Set(NSApp.windows.map { ObjectIdentifier($0) })
        // The app menu's Settings… item (⌘,), as a person would open it.
        if let appMenu = NSApp.mainMenu?.items.first?.submenu,
           let index = appMenu.items.firstIndex(where: { $0.keyEquivalent == "," }) {
            appMenu.performActionForItem(at: index)
        } else {
            NSApp.sendAction(Selector(("showSettingsWindow:")), to: nil, from: nil)
        }
        await wait(1.5)
        guard let window = NSApp.windows.first(where: { !before.contains(ObjectIdentifier($0)) && $0.contentView != nil })
                ?? NSApp.windows.first(where: { $0.identifier?.rawValue.contains("Settings") == true }) else {
            let list = NSApp.windows.map { "\($0.title)|\($0.identifier?.rawValue ?? "-")|\($0.isVisible)" }
            return log("FAIL settingsui: the Settings window didn't open; windows: \(list.joined(separator: ", "))")
        }
        var shots: [String] = []
        func shoot(_ name: String, _ w: NSWindow? = nil) {
            if writeWindowImage(w ?? window, to: dir.appendingPathComponent("ui-\(name).png")) { shots.append(name) }
        }
        for (i, section) in store.sections.enumerated() {
            nav.query = ""
            nav.section = section
            await wait(0.6)
            shoot(String(format: "%02d-%@", i + 1, section))
        }
        for query in ["fade", "midi", "zzz"] {
            nav.query = query
            await wait(0.6)
            shoot("search-\(query)")
        }
        nav.query = ""
        nav.section = "Output"
        store.set("output.autoAdvance", bool: false)
        await wait(0.6)
        shoot("output-autoadvance-off")
        restoreSettings(saved)
        for theme in ["ember", "daylight"] {
            store.set("appearance.theme", string: theme)
            await wait(0.8)
            if let main = NSApp.windows.first(where: { $0.identifier?.rawValue == "main" || ($0 !== window && $0.isVisible && $0.frame.width > 800) }) {
                shoot("main-\(theme)", main)
            }
        }
        restoreSettings(saved)
        await wait(0.5)
        window.performClose(nil)
        log("settingsui: \(shots.count) images (ui-*.png): \(shots.joined(separator: ", "))")
    }
}

// MARK: - Audio settings

extension SelfTest {
    /// Tempo ranges and a beat offset, on the File source with real tracks.
    func audioSettings() async {
        let store = SettingsStore.shared
        let saved = (range: store.string("audio.tempoRange"), offset: store.number("audio.beatOffsetFile"))
        guard let file = engine.allAudioSources().first(where: { $0.sourceType == 1 }),
              let velvet = engine.sampleTracks().first(where: { $0.name.contains("Velvet") }),
              let song = engine.sampleTracks().first(where: { $0.name.contains("Nottawa Song") }) else {
            return log("FAIL audiosettings: no File source or sample tracks")
        }
        logOutputDevice()
        engine.selectAudioSource(id: file.id)
        engine.setBpmEnabled(true)
        engine.setBpmMode(BpmMode.auto.rawValue)
        engine.setBpmLocked(false)
        store.set("audio.tempoRange", string: "auto")
        store.set("audio.beatOffsetFile", number: 0)

        // Velvet Skies is 87: shown doubled only where 87 is below the range.
        engine.selectSampleTrack(velvet.index)
        engine.setFileAudioPosition(0)
        if !engine.isAudioActive { engine.toggleAudioSource() }
        await wait(15)
        let expected: [(String, Double)] = [("auto", 87), ("60-120", 87), ("80-160", 87), ("90-180", 174), ("100-200", 174), ("auto", 87)]
        for (range, want) in expected {
            store.set("audio.tempoRange", string: range)
            await wait(1.5)
            var shown: [Float] = []
            for _ in 0..<12 { shown.append(engine.extendedAudioAnalysis().bpm); await wait(0.25) }
            let median = Double(shown.sorted()[shown.count / 2])
            let ok = abs(median - want) <= 0.03 * want
            log(String(format: "%@ tempo range %@: Velvet Skies shows %.2f (want %.0f)", ok ? "PASS" : "FAIL", range, median, want))
        }

        // The pulse moved by the File offset: Nottawa Song recorded without
        // and with +100 ms (scored offline: bias should drop by 100 ms).
        for offset in [0.0, 100.0] {
            store.set("audio.beatOffsetFile", number: offset)
            engine.selectSampleTrack(song.index)
            engine.setFileAudioPosition(0)
            await listen(label: offset == 0 ? "files" : "filesoffset\(Int(offset))", track: song, clock: { [engine] in
                let s = engine.fileAudioState()
                return Double(s.playbackPosition) * Double(s.totalDuration)
            })
        }
        store.set("audio.tempoRange", string: saved.range)
        store.set("audio.beatOffsetFile", number: saved.offset)
    }

    private struct LaunchState: Codable {
        var sourceId: String?
        var active: Bool
        var startOnLaunch: Bool
        var expectId: String
    }

    private var launchURL: URL { dir.appendingPathComponent("audiolaunch.json") }

    /// Start-on-launch on, a different input picked, then quit at once.
    func audioLaunchSet() {
        let store = SettingsStore.shared
        guard let file = engine.allAudioSources().first(where: { $0.sourceType == 1 }) else {
            log("FAIL audiolaunch-set: no File source")
            NSApp.terminate(nil)
            return
        }
        let state = LaunchState(sourceId: engine.selectedAudioSourceId, active: engine.isAudioActive,
                                startOnLaunch: store.bool("audio.startOnLaunch"), expectId: file.id)
        try? JSONEncoder().encode(state).write(to: launchURL)
        if engine.isAudioActive { engine.toggleAudioSource() }
        engine.selectAudioSource(id: file.id)
        if engine.isAudioActive { engine.toggleAudioSource() }  // remembered, but not running at quit
        store.set("audio.startOnLaunch", bool: true)
        log("audiolaunch-set: input \(file.name), start on launch on, quitting")
        NSApp.terminate(nil)
    }

    func audioLaunchCheck() {
        guard let data = try? Data(contentsOf: launchURL),
              let state = try? JSONDecoder().decode(LaunchState.self, from: data) else {
            return log("FAIL audiolaunch-check: no audiolaunch.json from audiolaunch-set")
        }
        let selected = engine.selectedAudioSourceId
        let ok = selected == state.expectId && engine.isAudioActive
        log("\(ok ? "PASS" : "FAIL") audiolaunch: after relaunch input \(selected ?? "none") (want \(state.expectId)), running \(engine.isAudioActive)")
        // Put back what the user had: their input, running or not, and the setting.
        SettingsStore.shared.set("audio.startOnLaunch", bool: state.startOnLaunch)
        if engine.isAudioActive { engine.toggleAudioSource() }
        if let id = state.sourceId { engine.selectAudioSource(id: id) }
        if engine.isAudioActive != state.active { engine.toggleAudioSource() }
        keepAudioState = true
        try? FileManager.default.removeItem(at: launchURL)
    }

    /// The default output device and the latency it adds after the app's
    /// clock (what a beat offset for heard audio has to cover).
    func logOutputDevice() {
        var id = AudioObjectID(kAudioObjectUnknown)
        var size = UInt32(MemoryLayout<AudioObjectID>.size)
        var addr = AudioObjectPropertyAddress(mSelector: kAudioHardwarePropertyDefaultOutputDevice,
                                              mScope: kAudioObjectPropertyScopeGlobal, mElement: kAudioObjectPropertyElementMain)
        guard AudioObjectGetPropertyData(AudioObjectID(kAudioObjectSystemObject), &addr, 0, nil, &size, &id) == noErr else {
            return log("output device: unknown")
        }
        func u32(_ selector: AudioObjectPropertySelector, _ object: AudioObjectID? = nil) -> UInt32 {
            var v: UInt32 = 0
            var s = UInt32(MemoryLayout<UInt32>.size)
            var a = AudioObjectPropertyAddress(mSelector: selector, mScope: kAudioDevicePropertyScopeOutput, mElement: kAudioObjectPropertyElementMain)
            _ = AudioObjectGetPropertyData(object ?? id, &a, 0, nil, &s, &v)
            return v
        }
        var rate: Float64 = 0
        var rs = UInt32(MemoryLayout<Float64>.size)
        var ra = AudioObjectPropertyAddress(mSelector: kAudioDevicePropertyNominalSampleRate,
                                            mScope: kAudioObjectPropertyScopeGlobal, mElement: kAudioObjectPropertyElementMain)
        _ = AudioObjectGetPropertyData(id, &ra, 0, nil, &rs, &rate)
        var name: Unmanaged<CFString>?
        var ns = UInt32(MemoryLayout<Unmanaged<CFString>?>.size)
        var na = AudioObjectPropertyAddress(mSelector: kAudioObjectPropertyName,
                                            mScope: kAudioObjectPropertyScopeGlobal, mElement: kAudioObjectPropertyElementMain)
        _ = AudioObjectGetPropertyData(id, &na, 0, nil, &ns, &name)
        // The first output stream's own latency.
        var streams = [AudioStreamID](repeating: 0, count: 8)
        var ss = UInt32(MemoryLayout<AudioStreamID>.size * streams.count)
        var sa = AudioObjectPropertyAddress(mSelector: kAudioDevicePropertyStreams,
                                            mScope: kAudioDevicePropertyScopeOutput, mElement: kAudioObjectPropertyElementMain)
        _ = AudioObjectGetPropertyData(id, &sa, 0, nil, &ss, &streams)
        let streamLatency = ss > 0 ? u32(kAudioStreamPropertyLatency, streams[0]) : 0
        let device = u32(kAudioDevicePropertyLatency), safety = u32(kAudioDevicePropertySafetyOffset)
        let buffer = u32(kAudioDevicePropertyBufferFrameSize)
        let ms = { (frames: UInt32) in rate > 0 ? Double(frames) / rate * 1000 : 0 }
        log(String(format: "output device %@ @ %.0f Hz: device latency %.1f ms + safety %.1f ms + stream %.1f ms + buffer %.1f ms = %.1f ms after render",
                   (name?.takeRetainedValue() as String?) ?? "?", rate, ms(device), ms(safety), ms(streamLatency), ms(buffer),
                   ms(device + safety + streamLatency + buffer)))
    }
}

// MARK: - Output windows

extension SelfTest {
    private var windowsDefaultsKey: String { "nottawa.outputWindows" }

    /// A node of the session to show, and its name.
    private func someNode() -> (id: String, name: String)? {
        if let v = engine.allVideoSources().first { return (v.id, v.name) }
        if let s = engine.allShaders().first { return (s.id, s.name) }
        return nil
    }

    private func close(after: Set<String>) {
        for id in PreviewWindowManager.shared.openNodeIds.subtracting(after) { PreviewWindowManager.shared.closePreview(for: id) }
    }

    func outputWindows() async {
        let manager = PreviewWindowManager.shared
        let savedDefaults = UserDefaults.standard.data(forKey: windowsDefaultsKey)
        defer { UserDefaults.standard.set(savedDefaults, forKey: windowsDefaultsKey) }
        guard let node = someNode(), let screen = NSScreen.main else { return log("FAIL outputwindows: no node or screen") }
        let before = manager.openNodeIds
        let place = NSRect(x: screen.frame.minX + 220, y: screen.frame.minY + 180, width: 640, height: 360)

        // Recorded at quit: node, display, frame, not full screen.
        manager.openPreview(for: node.id, nodeName: node.name)
        await wait(0.5)
        manager.reopen([.init(nodeId: node.id, screenName: screen.localizedName, frame: place, fullScreen: false)]) { _ in node.name }
        await wait(0.3)
        manager.saveOpenWindows()
        let entry = manager.savedWindows.first { $0.nodeId == node.id }
        let recorded = entry.map { $0.screenName == screen.localizedName && !$0.fullScreen && $0.frame.equalTo(place) } ?? false
        log("\(recorded ? "PASS" : "FAIL") outputwindows saved: \(entry.map { "\($0.screenName ?? "nil") \(NSStringFromRect($0.frame)) fullScreen \($0.fullScreen)" } ?? "nothing")")
        close(after: before)
        await wait(0.3)

        // Reopened at the saved frame on its display.
        var opened = manager.reopen([.init(nodeId: node.id, screenName: screen.localizedName, frame: place, fullScreen: false)]) { _ in node.name }
        await wait(0.3)
        let atFrame = manager.frame(for: node.id).map { $0.equalTo(place) } ?? false
        log("\(atFrame && opened == [node.id] ? "PASS" : "FAIL") outputwindows reopen: \(manager.frame(for: node.id).map(NSStringFromRect) ?? "none") (want \(NSStringFromRect(place)))")
        close(after: before)
        await wait(0.3)

        // A frame off its display: centred on the display instead.
        let lost = NSRect(x: -30000, y: -30000, width: 640, height: 360)
        manager.reopen([.init(nodeId: node.id, screenName: screen.localizedName, frame: lost, fullScreen: false)]) { _ in node.name }
        await wait(0.3)
        let centred = manager.frame(for: node.id).map { abs($0.midX - screen.visibleFrame.midX) < 2 && abs($0.midY - screen.visibleFrame.midY) < 2 } ?? false
        log("\(centred ? "PASS" : "FAIL") outputwindows off-display frame centred: \(manager.frame(for: node.id).map(NSStringFromRect) ?? "none")")
        close(after: before)
        await wait(0.3)

        // The display isn't connected: opens windowed where it lands, not at the saved frame.
        opened = manager.reopen([.init(nodeId: node.id, screenName: "No Such Display", frame: place, fullScreen: true)]) { _ in node.name }
        await wait(0.8)
        let frame = manager.frame(for: node.id)
        let windowed = opened == [node.id] && frame.map { !$0.equalTo(place) && $0.intersects(screen.frame) } ?? false
        log("\(windowed ? "PASS" : "FAIL") outputwindows display gone: opened windowed at \(frame.map(NSStringFromRect) ?? "none")")
        close(after: before)

        // A node that isn't in the session: skipped.
        opened = manager.reopen([.init(nodeId: "no-such-node", screenName: screen.localizedName, frame: place, fullScreen: false)]) { _ in nil }
        log("\(opened.isEmpty ? "PASS" : "FAIL") outputwindows missing node skipped")
    }

    private struct WindowsLaunchState: Codable {
        var nodeId: String
        var frame: CGRect
    }

    private var windowsLaunchURL: URL { dir.appendingPathComponent("outputwindows.json") }

    /// An output window placed, then quit (through the real quit-time save).
    func outputWindowsSet() async {
        let manager = PreviewWindowManager.shared
        guard let node = someNode(), let screen = NSScreen.main else {
            log("FAIL outputwindows-set: no node or screen")
            return NSApp.terminate(nil)
        }
        let place = NSRect(x: screen.frame.minX + 220, y: screen.frame.minY + 180, width: 640, height: 360)
        manager.reopen([.init(nodeId: node.id, screenName: screen.localizedName, frame: place, fullScreen: false)]) { _ in node.name }
        await wait(0.5)
        try? JSONEncoder().encode(WindowsLaunchState(nodeId: node.id, frame: place)).write(to: windowsLaunchURL)
        log("outputwindows-set: \(node.name) at \(NSStringFromRect(place)) on \(screen.localizedName), quitting")
        NSApp.terminate(nil)
    }

    func outputWindowsCheck() {
        guard let data = try? Data(contentsOf: windowsLaunchURL),
              let state = try? JSONDecoder().decode(WindowsLaunchState.self, from: data) else {
            return log("FAIL outputwindows-check: no outputwindows.json from outputwindows-set")
        }
        let frame = PreviewWindowManager.shared.frame(for: state.nodeId)
        let ok = frame.map { $0.equalTo(state.frame) } ?? false
        log("\(ok ? "PASS" : "FAIL") outputwindows launch: reopened at \(frame.map(NSStringFromRect) ?? "not reopened") (want \(NSStringFromRect(state.frame)))")
        PreviewWindowManager.shared.closePreview(for: state.nodeId)  // the quit-time save then records it closed
        try? FileManager.default.removeItem(at: windowsLaunchURL)
    }
}

// MARK: - Diagnostics

extension SelfTest {
    func diagnostics() {
        let text = Diagnostics.text()
        try? text.write(to: dir.appendingPathComponent("diagnostics.txt"), atomically: true, encoding: .utf8)
        let parts = ["Nottawa ", "System: macOS", "Graphics: ", "Display 1: ", "Engine: ", "Audio: ", "Settings changed: ", "Previous session: "]
        let missing = parts.filter { !text.contains($0) }
        let user = NSUserName(), home = "/Users/\(NSUserName())"
        let names = text.contains(user) || text.contains(home) || text.contains(NSHomeDirectory())
        log("\(missing.isEmpty && !names ? "PASS" : "FAIL") diagnostics: \(text.split(separator: "\n").count) lines\(missing.isEmpty ? "" : ", missing \(missing)")\(names ? ", NAMES THE USER" : "") (diagnostics.txt)")
        log("diagnostics: previous session \(Diagnostics.uncleanPreviousSession == nil ? "quit normally" : "didn't quit normally")")
        // The folder actions, from the sandbox (opt-in: they open Finder windows).
        if ProcessInfo.processInfo.environment["NOTTAWA_SELFTEST_OPENFOLDERS"] == "1" {
            // (Whether Finder showed them is checked from outside: activateFileViewerSelecting reports nothing.)
            SettingsStore.openLogsFolder()
            log("\(NSWorkspace.shared.open(SettingsStore.dataFolder) ? "PASS" : "FAIL") open data folder from the sandbox; logs folder asked of Finder")
        }
    }

    func diagnosticsCheck() {
        let unclean = Diagnostics.uncleanPreviousSession != nil
        let text = Diagnostics.text()
        log("\(unclean && text.contains("didn't quit normally") ? "PASS" : "FAIL") diagnostics after an abrupt exit: \(text.split(separator: "\n").last ?? "")")
    }
}

// MARK: - Frame cache

extension SelfTest {
    /// The clip loaded fresh under each setting: whether FileSource cached it.
    func cacheSettings() async {
        let store = SettingsStore.shared
        guard let path = ProcessInfo.processInfo.environment["NOTTAWA_SELFTEST_CLIP"],
              FileManager.default.fileExists(atPath: path) else {
            return log("FAIL cachesettings: no clip (NOTTAWA_SELFTEST_CLIP)")
        }
        let saved = (on: store.bool("performance.frameCache"), budget: store.string("performance.frameCacheBudget"))
        // (enabled, budget, cached?) for a clip whose cache is ~1106 MB.
        let cases: [(Bool, String, Bool)] = [(false, "auto", false), (true, "1024", false), (true, "2048", true), (true, "auto", true)]
        for (on, budget, want) in cases {
            store.set("performance.frameCache", bool: on)
            store.set("performance.frameCacheBudget", string: budget)
            await wait(0.3)
            let started = Date()
            guard let id = engine.addFileVideoSource(name: "selftest clip", path: path) else {
                log("FAIL cachesettings: couldn't add \(path)")
                continue
            }
            var state = ntw_get_file_source_state(id)
            for _ in 0..<40 where state.duration <= 0 { await wait(0.25); state = ntw_get_file_source_state(id) }
            let cached = state.cacheAvailable != 0
            log(String(format: "%@ cache %@ budget %@: cached %@ (%d frames, in use %d), loaded in %.1f s",
                       cached == want ? "PASS" : "FAIL", on ? "on" : "off", budget, cached ? "yes" : "no",
                       state.cachedFrameCount, state.useCachedMode, Date().timeIntervalSince(started)))
            if !on {
                // The clip's own switch, with the setting off: there's no cache to use.
                ntw_set_file_source_use_cached_mode(id, 1)
                await wait(0.5)
                let after = ntw_get_file_source_state(id)
                log("cachesettings: clip's own cache switched on with the setting off: available \(after.cacheAvailable), in use \(after.useCachedMode), frames \(after.cachedFrameCount)")
            }
            engine.removeConnectable(id: id)
            await wait(0.5)
        }
        store.set("performance.frameCache", bool: saved.on)
        store.set("performance.frameCacheBudget", string: saved.budget)
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

// MARK: - Rendered pixels (golden, workflows)

/// A node's frame shrunk to 128x72, top row first, straight RGBA: what golden
/// images hold. Same rules as the Windows suite (docs/TESTING.md).
struct GoldenImage {
    static let width = 128, height = 72
    var pixels: [UInt8]  // width * height * 4

    /// Reads a node's frame back from the engine (bottom-up rows) and box-
    /// filters it down, top row first.
    static func frame(_ id: String) -> GoldenImage? {
        var w: Int32 = 0, h: Int32 = 0
        ntw_debug_frame_rgba(id, &w, &h, nil, 0)
        guard w > 0, h > 0 else { return nil }
        var raw = [UInt8](repeating: 0, count: Int(w) * Int(h) * 4)
        guard ntw_debug_frame_rgba(id, &w, &h, &raw, Int32(raw.count)) != 0 else { return nil }
        let sw = Int(w), sh = Int(h)
        var out = [UInt8](repeating: 0, count: width * height * 4)
        for ty in 0..<height {
            let y0 = ty * sh / height, y1 = max(y0 + 1, (ty + 1) * sh / height)
            for tx in 0..<width {
                let x0 = tx * sw / width, x1 = max(x0 + 1, (tx + 1) * sw / width)
                var sum = [Int](repeating: 0, count: 4)
                for y in y0..<y1 {
                    let row = (sh - 1 - y) * sw  // bottom-up
                    for x in x0..<x1 { for c in 0..<4 { sum[c] += Int(raw[(row + x) * 4 + c]) } }
                }
                let n = (y1 - y0) * (x1 - x0)
                for c in 0..<4 { out[(ty * width + tx) * 4 + c] = UInt8((sum[c] + n / 2) / n) }
            }
        }
        return GoldenImage(pixels: out)
    }

    /// Nothing drawn: every channel 2 or less.
    var isEmpty: Bool { !pixels.contains { $0 > 2 } }

    /// The share of pixels with a channel more than 24 apart, and the mean
    /// channel difference; the same within 1% and a mean of 2.
    func diff(_ other: GoldenImage?) -> (changed: Double, mean: Double, same: Bool) {
        guard let other, other.pixels.count == pixels.count else { return (1, 255, false) }
        var changed = 0, sum = 0
        for i in stride(from: 0, to: pixels.count, by: 4) {
            var worst = 0
            for c in 0..<4 {
                let d = abs(Int(pixels[i + c]) - Int(other.pixels[i + c]))
                worst = max(worst, d)
                sum += d
            }
            if worst > 24 { changed += 1 }
        }
        let n = Double(pixels.count / 4)
        let c = Double(changed) / n, m = Double(sum) / (n * 4)
        return (c, m, c < 0.01 && m < 2)
    }

    private static func cgImage(_ pixels: [UInt8], width: Int) -> CGImage? {
        let provider = CGDataProvider(data: Data(pixels) as CFData)!
        return CGImage(width: width, height: pixels.count / 4 / width, bitsPerComponent: 8, bitsPerPixel: 32,
                       bytesPerRow: width * 4, space: CGColorSpace(name: CGColorSpace.sRGB)!,
                       bitmapInfo: CGBitmapInfo(rawValue: CGImageAlphaInfo.last.rawValue),
                       provider: provider, decode: nil, shouldInterpolate: false, intent: .defaultIntent)
    }

    private static func writePNG(_ image: CGImage, to url: URL) -> Bool {
        guard let dest = CGImageDestinationCreateWithURL(url as CFURL, "public.png" as CFString, 1, nil) else { return false }
        CGImageDestinationAddImage(dest, image, nil)
        return CGImageDestinationFinalize(dest)
    }

    func write(to url: URL) -> Bool {
        guard let image = Self.cgImage(pixels, width: Self.width) else { return false }
        return Self.writePNG(image, to: url)
    }

    /// Expected | actual side by side, for a failure.
    static func writePair(_ a: GoldenImage?, _ b: GoldenImage, to url: URL) {
        var out = [UInt8](repeating: 0, count: width * 2 * height * 4)
        for y in 0..<height {
            for x in 0..<width {
                for c in 0..<4 {
                    out[(y * width * 2 + x) * 4 + c] = a?.pixels[(y * width + x) * 4 + c] ?? 0
                    out[(y * width * 2 + width + x) * 4 + c] = b.pixels[(y * width + x) * 4 + c]
                }
            }
        }
        if let image = cgImage(out, width: width * 2) { _ = writePNG(image, to: url) }
    }

    /// A stored image, as straight RGBA (as written), or nil.
    init?(contentsOf url: URL) {
        guard let source = CGImageSourceCreateWithURL(url as CFURL, nil),
              let image = CGImageSourceCreateImageAtIndex(source, 0, nil),
              image.width == Self.width, image.height == Self.height,
              image.bitsPerPixel == 32, image.alphaInfo == .last,
              let data = image.dataProvider?.data as Data? else { return nil }
        var out = [UInt8](repeating: 0, count: Self.width * Self.height * 4)
        for y in 0..<Self.height {
            let row = data.subdata(in: (y * image.bytesPerRow)..<(y * image.bytesPerRow + Self.width * 4))
            out.replaceSubrange((y * Self.width * 4)..<((y + 1) * Self.width * 4), with: row)
        }
        pixels = out
    }

    init(pixels: [UInt8]) { self.pixels = pixels }
}

extension SelfTest {
    /// 256x144, the clock held, step mode: what golden images are rendered at.
    private func enterDeterministicMode() {
        engine.clearGraph()
        store.set("output.customWidth", number: 256)
        store.set("output.customHeight", number: 144)
        store.set("output.resolution", string: "custom")
        ntw_debug_freeze_time(12.5)
        ntw_debug_step_mode(1)
    }

    private func leaveDeterministicMode() {
        ntw_debug_step_mode(0)
        ntw_debug_freeze_time(-1)
        engine.clearGraph()
        ntw_reset_settings("Output")
        viewModel.refresh()
    }

    private func addSource(named name: String) -> String? {
        guard let s = engine.availableShaderSources().first(where: { $0.name == name }) else {
            log("FAIL no source called \(name)")
            return nil
        }
        return engine.addShaderVideoSource(type: s.shaderTypeRaw)
    }

    private func effectType(named name: String) -> Int? {
        if let e = engine.availableShaders().first(where: { $0.name == name }) { return e.shaderTypeRaw }
        log("FAIL no effect called \(name)")
        return nil
    }

    /// An effect fed from `input` (a source or an effect), output 0 to input 0.
    private func addEffect(type: Int, after input: String, inputIsSource: Bool) -> String? {
        engine.undoGroup {
            guard let id = engine.addShader(type: type) else { return nil }
            engine.makeConnection(startId: input, endId: id, connectionType: inputIsSource ? 1 : 0, outputSlot: 0, inputSlot: 0)
            return id
        }
    }

    /// Golden images (docs/TESTING.md): every effect fed by Plasma and every
    /// generated source, rendered deterministically and compared with
    /// NOTTAWA_GOLDEN_DIR. NOTTAWA_GOLDEN=record renders each twice and
    /// stores those that came out the same (the others go in unstable.txt);
    /// stabilize moves entries that differ from their image into unstable.txt.
    func golden() {
        let env = ProcessInfo.processInfo.environment
        let mode = env["NOTTAWA_GOLDEN"] ?? "check"
        guard let dirPath = env["NOTTAWA_GOLDEN_DIR"], !dirPath.isEmpty else { return log("FAIL golden: NOTTAWA_GOLDEN_DIR isn't set") }
        let dir = URL(fileURLWithPath: dirPath)
        try? FileManager.default.createDirectory(at: dir, withIntermediateDirectories: true)

        struct Entry { let name: String; let typeRaw: Int; let isSource: Bool }
        var entries: [Entry] = engine.availableShaderSources().map { Entry(name: "src-\($0.name)-\($0.shaderTypeRaw)", typeRaw: $0.shaderTypeRaw, isSource: true) }
        var effectTypes = Set<Int>()  // (an effect can be listed in two categories)
        for e in engine.availableShaders() where effectTypes.insert(e.shaderTypeRaw).inserted {
            entries.append(Entry(name: "fx-\(e.name)-\(e.shaderTypeRaw)", typeRaw: e.shaderTypeRaw, isSource: false))
        }
        func fileName(_ name: String) -> String {
            String(name.map { $0.isLetter || $0.isNumber || $0 == "-" ? $0 : "_" }) + ".png"
        }
        var seen = Set<String>()
        for e in entries where !seen.insert(fileName(e.name)).inserted { log("FAIL golden: two entries share \(e.name)") }

        let unstableURL = dir.appendingPathComponent("unstable.txt")
        let unstableText = (try? String(contentsOf: unstableURL, encoding: .utf8)) ?? ""
        let knownUnstable: Set<String> = mode == "record" ? [] : Set(unstableText.split(separator: "\n").map { $0.trimmingCharacters(in: .whitespaces) }.filter { !$0.isEmpty && !$0.hasPrefix("#") })

        enterDeterministicMode()
        guard let plasma = addSource(named: "Plasma") else { return leaveDeterministicMode() }
        var first: [String: GoldenImage] = [:]
        var failures: [String] = [], unstable: [String] = []
        var stored = 0, checked = 0, unstableChecked = 0

        for pass in 0..<(mode == "record" ? 2 : 1) {
            for entry in entries {
                ntw_debug_render_frames(0, 1)  // seeds ofRandom before the node's setup draws from it
                let id = entry.isSource ? engine.addShaderVideoSource(type: entry.typeRaw)
                                        : addEffect(type: entry.typeRaw, after: plasma, inputIsSource: true)
                guard let id else { failures.append("\(entry.name) (couldn't be added)"); continue }
                ntw_debug_render_frames(8, 1)
                let frame = GoldenImage.frame(id)
                engine.removeConnectable(id: id)
                guard let frame else { failures.append("\(entry.name) (no frame)"); continue }
                let url = dir.appendingPathComponent(fileName(entry.name))

                if mode == "record" {
                    if pass == 0 { first[entry.name] = frame; continue }
                    if frame.diff(first[entry.name]).same {
                        if frame.write(to: url), GoldenImage(contentsOf: url)?.pixels == frame.pixels { stored += 1 }
                        else { failures.append("\(entry.name) (didn't read back as written)") }
                    } else {
                        unstable.append(entry.name)
                        try? FileManager.default.removeItem(at: url)
                    }
                    continue
                }
                let expected = GoldenImage(contentsOf: url)
                if knownUnstable.contains(entry.name) {
                    unstableChecked += 1
                    // Only that it renders something (unless it didn't when recorded either).
                    if frame.isEmpty && !(expected?.isEmpty ?? false) { failures.append("\(entry.name) (renders nothing)") }
                    continue
                }
                guard let expected else { failures.append("\(entry.name) (no golden image: record them)"); continue }
                let d = frame.diff(expected)
                checked += 1
                if d.same { continue }
                if mode == "stabilize" { unstable.append(entry.name); continue }
                failures.append(String(format: "%@ (%.1f%% of pixels differ, mean %.1f)", entry.name, d.changed * 100, d.mean))
                GoldenImage.writePair(expected, frame, to: self.dir.appendingPathComponent("golden-" + fileName(entry.name)))
            }
        }
        leaveDeterministicMode()

        switch mode {
        case "record":
            // Kept: the block from "# Kept by hand" on (entries added by hand, with why).
            let kept = unstableText.range(of: "# Kept by hand").map { String(unstableText[$0.lowerBound...]) } ?? ""
            let text = "# Rendered differently twice in one launch (found when recording): only checked for rendering something.\n"
                + unstable.map { $0 + "\n" }.joined() + kept
            try? text.write(to: unstableURL, atomically: true, encoding: .utf8)
            log("golden: recorded \(stored) of \(entries.count); unstable (not the same twice): \(unstable.joined(separator: ", "))")
        case "stabilize":
            if !unstable.isEmpty {
                try? (unstableText + unstable.map { $0 + "\n" }.joined()).write(to: unstableURL, atomically: true, encoding: .utf8)
            }
            log("golden: stabilize: \(unstable.count) differed from their images and are now unstable: \(unstable.joined(separator: ", "))")
        default:
            log("golden: checked \(checked) against their images, \(unstableChecked) unstable for rendering something; \(failures.count) failed")
        }
        for f in failures { log("FAIL golden: \(f)") }
        if failures.isEmpty { log("PASS golden: \(entries.count) entries") }
    }

    /// What users do, checked by what it renders (deterministic, as golden).
    func workflows() {
        // Known bugs: reported, not failing the run; KNOWN-FIXED when one passes.
        let known: Set<String> = []
        func check(_ ok: Bool, _ what: String) {
            if known.contains(what) { log("\(ok ? "KNOWN-FIXED (remove from the known list)" : "KNOWN") workflows: \(what)") }
            else { log("\(ok ? "PASS" : "FAIL") workflows: \(what)") }
        }
        func render() { ntw_debug_render_frames(4, 1) }
        func nodeCount() -> Int { engine.allShaders().count + engine.allVideoSources().count }

        enterDeterministicMode()
        defer { leaveDeterministicMode() }
        guard let plasma = addSource(named: "Plasma"),
              let kaleidoType = effectType(named: "Kaleidoscope"), let hsbType = effectType(named: "HSB"),
              let maskType = effectType(named: "Mask"), let colorMaskType = effectType(named: "Color Mask"),
              let pixelateType = effectType(named: "Pixelate"),
              let kaleido = addEffect(type: kaleidoType, after: plasma, inputIsSource: true),
              let hsb = addEffect(type: hsbType, after: kaleido, inputIsSource: false) else {
            return log("FAIL workflows: couldn't build the chain")
        }

        // 1. A chain renders at every stage, and an effect changes its input.
        render()
        let p = GoldenImage.frame(plasma), k = GoldenImage.frame(kaleido), h = GoldenImage.frame(hsb)
        check(p.map { !$0.isEmpty } == true && k.map { !$0.isEmpty } == true && h.map { !$0.isEmpty } == true, "chain renders at every stage")
        check(k.map { !$0.diff(p).same } == true, "Kaleidoscope changes its input")

        // 2. An optional effect changes the output; off again, it's as before.
        // (On Plasma: Kaleidoscope's output is already mirror-symmetric.)
        if let mirror = engine.optionalShaders(for: plasma).first(where: { $0.name.contains("Mirror") }) {
            let before = GoldenImage.frame(plasma)
            engine.toggleOptionalShader(connectableId: plasma, index: mirror.index)
            render()
            let on = GoldenImage.frame(plasma)
            engine.toggleOptionalShader(connectableId: plasma, index: mirror.index)
            render()
            let off = GoldenImage.frame(plasma)
            check(on.map { !$0.isEmpty && !$0.diff(before).same } == true, "optional effect (Mirror) changes the output")
            check(off.map { $0.diff(before).same } == true, "optional effect off restores the output")
        } else {
            check(false, "optional effect: no Mirror on the node")
        }

        // 3. A two-input effect reads its second input. The catalog's Mask is
        // MaskEngine: by default it keys input A by its own brightness and
        // never reads B, so Matched Source is set to Input B. Without B
        // connected B is A, so the output is A where it's bright enough;
        // with B (Plasma through Color Mask, keyed to transparency) it changes.
        if let mask = addEffect(type: maskType, after: plasma, inputIsSource: true) {
            if let matched = engine.parameters(for: mask).first(where: { $0.name == "Matched Source" }) {
                engine.setParameterInt(paramId: matched.id, value: 1)
            }
            render()
            let alone = GoldenImage.frame(mask)
            check(alone.map { !$0.isEmpty } == true, "Mask renders with one input")
            if let colorMask = addEffect(type: colorMaskType, after: plasma, inputIsSource: true) {
                engine.makeConnection(startId: colorMask, endId: mask, connectionType: 0, outputSlot: 0, inputSlot: 1)
                render()
                check(GoldenImage.frame(mask).map { !$0.diff(alone).same } == true, "Mask (Matched Source: Input B) reads its second input")
                engine.removeConnectable(id: colorMask)
            }
            engine.removeConnectable(id: mask)
        }

        // 3b. A Text input draws its text: opaque text on a transparent
        // background, and new text draws differently. (Its FBO is
        // multisampled: the readback resolves it.)
        if let text = engine.addTextVideoSource(name: "Text") {
            ntw_set_text_source_text(text, "NOTTAWA")
            ntw_set_text_source_font_size(text, 120)
            render()
            let first = GoldenImage.frame(text)
            var opaque = 0, clear = 0
            if let first {
                for i in stride(from: 3, to: first.pixels.count, by: 4) { if first.pixels[i] > 200 { opaque += 1 } else { clear += 1 } }
                _ = first.write(to: dir.appendingPathComponent("workflows-text.png"))
            }
            ntw_set_text_source_text(text, "Hello")
            render()
            let second = GoldenImage.frame(text)
            check(opaque > 0 && clear > opaque, "a Text input draws text on a transparent background (\(opaque) opaque, \(clear) clear)")
            check(second.map { !$0.diff(first).same } == true, "new text draws differently")
            engine.removeConnectable(id: text)
        }

        // 4. A workspace saved and reopened renders the same.
        render()
        let hsbFrame = GoldenImage.frame(hsb)
        let nodes = nodeCount()
        let wsPath = dir.appendingPathComponent("workflows-ws.json").path
        engine.saveWorkspaceToPath(path: wsPath, name: "workflows")
        engine.clearGraph()
        render()
        let loaded = engine.loadWorkspace(path: wsPath)
        viewModel.refresh()
        render()
        check(loaded && nodeCount() == nodes, "workspace reopens with its nodes (\(nodes) -> \(nodeCount()))")
        check(GoldenImage.frame(hsb).map { $0.diff(hsbFrame).same } == true, "workspace reopens rendering the same")

        // 5. A resolution change reaches the frames.
        store.set("output.resolution", string: "1080p")
        render()
        var w: Int32 = 0, hh: Int32 = 0
        ntw_debug_frame_rgba(hsb, &w, &hh, nil, 0)
        check(w == 1920 && hh == 1080 && GoldenImage.frame(hsb).map { !$0.isEmpty } == true, "1080p renders at 1920x1080 (got \(w)x\(hh))")
        store.set("output.resolution", string: "custom")
        render()

        // 6. Undo removes an effect added after a node (dropped on it, the
        // app's add-and-connect path); redo brings it back, rendering.
        viewModel.refresh()
        let before = nodeCount()
        viewModel.addNodeFromDrop(payload: "shader:\(pixelateType)", at: .zero, targetNodeId: hsb)
        render()
        let added = nodeCount()
        engine.undo()
        render()
        let afterUndo = nodeCount()
        engine.redo()
        render()
        let back = engine.allShaders().first { $0.name == "Pixelate" }?.id
        check(added == before + 1 && afterUndo == before && back.flatMap { GoldenImage.frame($0) }.map { !$0.isEmpty } == true,
              "one undo removes a dropped-on effect, redo restores it rendering (\(before) -> \(added) -> \(afterUndo) nodes)")
    }
}

// MARK: - Texture lookups under load

extension SelfTest {
    /// The UI looks up each preview's surface every display frame. Under GPU
    /// load (run scripts/mac_gpu_hog.swift alongside) that lookup must not
    /// wait for the engine's GL work, nor previews wait for drawables. Renders 4K with every node shared and
    /// times the lookups from a thread, and the main thread's lateness.
    private func textureLock() async {
        let saved = snapshotSettings()
        ntw_set_custom_resolution(3840, 2160)
        let ids = engine.allVideoSources().map(\.id) + engine.allShaders().map(\.id)
        let started = ids.filter { ntw_get_connectable_texture($0) == nil }
        for id in started { engine.startTextureSharing(connectableId: id) }
        await wait(1.5)

        let duration = Double(ProcessInfo.processInfo.environment["NOTTAWA_SELFTEST_LOCK_SECONDS"] ?? "") ?? 15
        let end = CACurrentMediaTime() + duration
        let probe = LookupProbe(ids: ids, until: end)
        probe.start()
        var mainWorst = 0.0, mainOver50 = 0
        var engineWorst: Int64 = 0
        while CACurrentMediaTime() < end {
            let asked = CACurrentMediaTime()
            await wait(0.005)
            let late = CACurrentMediaTime() - asked - 0.005
            mainWorst = max(mainWorst, late)
            if late > 0.05 { mainOver50 += 1 }
            engineWorst = max(engineWorst, Int64(ntw_engine_frame_running_ms()))
        }
        while !probe.finished { await wait(0.05) }

        let (worst, over16, over50, count) = probe.summary()
        log(String(format: "texturelock: %d nodes at 4K, %d lookups: worst %.1f ms, %d over 16 ms, %d over 50 ms; main thread worst late %.1f ms (%d over 50 ms); engine frame seen running %lld ms; %.1f fps",
                   ids.count, count, worst * 1000, over16, over50, mainWorst * 1000, mainOver50, engineWorst, engine.engineFPS))
        log((ids.isEmpty ? "FAIL" : worst < 0.05 ? "PASS" : "FAIL") + " texturelock: lookups never wait on the engine's GL work")
        log((mainWorst < 0.1 ? "PASS" : "FAIL") + " texturelock: the main thread never waits 100 ms (previews skip frames instead)")

        for id in started { engine.stopTextureSharing(connectableId: id) }
        restoreSettings(saved)
        await wait(0.5)
    }
}

/// Times ntw_get_connectable_texture for every id, round robin, until `until`.
private final class LookupProbe: @unchecked Sendable {
    private let ids: [String]
    private let until: Double
    private let lock = NSLock()
    private var times: [Double] = []
    private var done = false

    init(ids: [String], until: Double) {
        self.ids = ids
        self.until = until
    }

    var finished: Bool { lock.withLock { done } }

    func start() {
        Thread { [self] in
            var local: [Double] = []
            var i = 0
            while CACurrentMediaTime() < until, !ids.isEmpty {
                let t0 = CACurrentMediaTime()
                _ = ntw_get_connectable_texture(ids[i % ids.count])
                local.append(CACurrentMediaTime() - t0)
                i += 1
                usleep(1000)
            }
            lock.withLock { times = local; done = true }
        }.start()
    }

    func summary() -> (worst: Double, over16: Int, over50: Int, count: Int) {
        let t = lock.withLock { times }
        return (t.max() ?? 0, t.filter { $0 > 0.016 }.count, t.filter { $0 > 0.05 }.count, t.count)
    }
}

private extension Array {
    subscript(safe i: Int) -> Element? { indices.contains(i) ? self[i] : nil }
}

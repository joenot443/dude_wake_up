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
            case "settings": await settingsRoundTrip()
            case "settings-set": settingsSet(); return  // quits right away, on purpose
            case "settings-check": settingsCheck()
            case "settingsui": await settingsUI()
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
                    case "screenshots.format":
                        check(ScreenshotService.shared.format.rawValue == choice.id, "screenshot format \(choice.id) not used")
                    case "appearance.theme":
                        check(store.theme.settingId == choice.id, "theme \(choice.id) maps to \(store.theme)")
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

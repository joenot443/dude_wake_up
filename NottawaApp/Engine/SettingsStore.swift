//
//  SettingsStore.swift
//  NottawaApp
//
//  The app preferences the engine owns (docs/SETTINGS.md): the schema, the
//  current values, and change notifications, for the Settings window and
//  everything that reads a setting. Values are cached here and refreshed by
//  the engine's change callback, so SwiftUI views observe them directly.
//

import AppKit
import Foundation

struct SettingChoice: Hashable {
    let id: String
    let label: String
}

struct SettingInfo: Identifiable, Hashable {
    enum Kind: Int {
        case bool = 0, int, float, choice, string, path, action
    }

    let id: String
    let section: String
    let label: String
    let help: String
    let unit: String
    let group: String
    let keywords: String
    let dependsOn: String?
    let kind: Kind
    let min: Double
    let max: Double
    let step: Double
    let decimals: Int
    let defaultNumber: Double
    let defaultString: String
    let choices: [SettingChoice]
    let needsRestart: Bool

    fileprivate init(from c: NTWSettingInfo) {
        func str(_ p: UnsafePointer<CChar>?) -> String { p.map { String(cString: $0) } ?? "" }
        id = str(c.id)
        section = str(c.section)
        label = str(c.label)
        help = str(c.help)
        unit = str(c.unit)
        group = str(c.group)
        keywords = str(c.keywords)
        let dep = str(c.dependsOn)
        dependsOn = dep.isEmpty ? nil : dep
        kind = Kind(rawValue: Int(c.type)) ?? .string
        min = c.min
        max = c.max
        step = c.step
        decimals = Int(c.decimals)
        defaultNumber = c.defaultNumber
        defaultString = str(c.defaultString)
        choices = (0..<Int(c.choiceCount)).map {
            SettingChoice(id: str(c.choiceIds[$0]), label: str(c.choiceLabels[$0]))
        }
        needsRestart = (c.restart & SettingsStore.macPlatform) != 0
    }

    /// Everything a search matches against.
    var searchText: String {
        [label, help, keywords, section, group].joined(separator: " ").lowercased()
    }
}

/// Used from the main thread (as the view models are); the engine's
/// callback hops there.
@Observable
final class SettingsStore {
    static let shared = SettingsStore()
    static let macPlatform: Int32 = 1

    private(set) var schema: [SettingInfo] = []
    private var numbers: [String: Double] = [:]
    private var strings: [String: String] = [:]

    private init() {}

    /// Sections in display order.
    var sections: [String] {
        var seen = Set<String>()
        return schema.map(\.section).filter { seen.insert($0).inserted }
    }

    func info(_ id: String) -> SettingInfo? { schema.first { $0.id == id } }

    /// After the engine has started: read the schema (this platform's rows)
    /// and every value, follow changes, and apply the ones the app acts on.
    func load() {
        var count: Int32 = 0
        if let ptr = ntw_get_settings(&count), count > 0 {
            schema = (0..<Int(count)).compactMap {
                (ptr[$0].platforms & Self.macPlatform) != 0 ? SettingInfo(from: ptr[$0]) : nil
            }
            ntw_free_settings(ptr, count)
        }
        for setting in schema { read(setting.id) }
        ntw_register_settings_changed_callback { cId in
            guard let cId else { return }
            let id = String(cString: cId)
            DispatchQueue.main.async { SettingsStore.shared.changed(id) }
        }
        for setting in schema { apply(setting.id) }
    }

    // MARK: - Values

    func bool(_ id: String) -> Bool { number(id) != 0 }
    func number(_ id: String) -> Double { numbers[id] ?? info(id)?.defaultNumber ?? 0 }
    func string(_ id: String) -> String { strings[id] ?? info(id)?.defaultString ?? "" }

    func set(_ id: String, bool value: Bool) { set(id, number: value ? 1 : 0) }

    func set(_ id: String, number value: Double) {
        numbers[id] = value  // at once, for the control; the callback brings the kept value
        ntw_set_setting_number(id, value)
    }

    func set(_ id: String, string value: String) {
        strings[id] = value
        ntw_set_setting_string(id, value)
    }

    /// One section back to its defaults, or every setting (nil).
    func reset(section: String?) {
        ntw_reset_settings(section)
    }

    private func read(_ id: String) {
        guard let kind = info(id)?.kind else { return }
        switch kind {
        case .bool, .int, .float:
            numbers[id] = ntw_get_setting_number(id)
        case .choice, .string, .path:
            if let c = ntw_get_setting_string(id) {
                strings[id] = String(cString: c)
                ntw_free_string(c)
            }
        case .action:
            break
        }
    }

    /// Ids changed since launch, oldest first (the self-test checks the callback with it).
    private(set) var changeLog: [String] = []

    private func changed(_ id: String) {
        read(id)
        changeLog.append(id)
        apply(id)
    }

    /// Settings the app carries out itself, rather than reading when needed.
    private func apply(_ id: String) {
        switch id {
        case "output.stretchPreviews":
            if PreviewWindowManager.shared.stretchToFill != bool(id) {
                PreviewWindowManager.shared.setStretchToFill(bool(id))
            }
        default:
            break
        }
    }

    /// A row's dependsOn: a Bool's id (on), or "id=choice" (that choice picked).
    func isSatisfied(_ dependsOn: String) -> Bool {
        let parts = dependsOn.split(separator: "=", maxSplits: 1).map(String.init)
        return parts.count == 2 ? string(parts[0]) == parts[1] : bool(dependsOn)
    }

    // MARK: - Theme

    var theme: ThemePreset { ThemePreset(settingId: string("appearance.theme")) }

    // MARK: - Migration

    /// Values this app kept in UserDefaults before the engine owned them.
    private static let legacyKeys: [(key: String, id: String)] = [
        ("showNodeHints", "display.nodeHints"),
        ("autoAdvanceOutputWindow", "output.autoAdvance"),
        ("outputWindowFadeDuration", "output.advanceFade"),
        ("clearOnDisconnect", "graph.clearOnDisconnect"),
        ("sidebarLivePreview", "browser.livePreview"),
        ("nottawa.screenshotFormat", "screenshots.format"),
    ]
    static let migratedKey = "nottawa.settingsMigrated"

    /// Once per install: copies the old UserDefaults values into the engine.
    /// Only keys that were ever written (a toggle changed from its default)
    /// are copied; the old keys are left in place for older builds.
    /// Returns the setting ids it set.
    @discardableResult
    func migrateLegacyDefaults(_ defaults: UserDefaults = .standard) -> [String] {
        guard !defaults.bool(forKey: Self.migratedKey) else { return [] }
        var migrated: [String] = []
        for (key, id) in Self.legacyKeys {
            guard let value = defaults.object(forKey: key), let kind = info(id)?.kind else { continue }
            switch (kind, value) {
            case (.bool, let v as NSNumber): set(id, bool: v.boolValue)
            case (.int, let v as NSNumber), (.float, let v as NSNumber): set(id, number: v.doubleValue)
            case (.choice, let v as String), (.string, let v as String): set(id, string: v)
            default: continue
            }
            migrated.append(id)
        }
        defaults.set(true, forKey: Self.migratedKey)
        return migrated
    }

    // MARK: - Folders (the Advanced actions)

    /// Settings, the session, workspaces and the library, inside the sandbox container.
    static var dataFolder: URL {
        // (NOTTAWA_APPDATA: the engine's throwaway data for scripted test runs.)
        let root = ProcessInfo.processInfo.environment["NOTTAWA_APPDATA"] ?? NSHomeDirectory()
        return URL(fileURLWithPath: root).appendingPathComponent("nottawa", isDirectory: true)
    }

    /// The Mac app keeps no log file: crash and hang reports are macOS's.
    static var logsFolder: URL {
        URL(fileURLWithPath: "/Users/\(NSUserName())/Library/Logs/DiagnosticReports", isDirectory: true)
    }

    /// Shows the crash reports folder in Finder. Outside the sandbox
    /// container, NSWorkspace.open(_:) is refused; revealing it in Finder isn't.
    static func openLogsFolder() {
        NSWorkspace.shared.activateFileViewerSelecting([logsFolder])
    }
}

extension ThemePreset {
    /// The engine's appearance.theme choice id for this preset, and back.
    var settingId: String {
        switch self {
        case .default_: return "default"
        case .midnight: return "midnight"
        case .daylight: return "daylight"
        case .ember: return "ember"
        case .arctic: return "arctic"
        case .forest: return "forest"
        }
    }

    init(settingId: String) {
        self = ThemePreset.allCases.first { $0.settingId == settingId } ?? .default_
    }
}

//
//  SettingsView.swift
//  NottawaApp
//
//  The Settings window, built from the engine's settings schema
//  (SettingsStore): a sidebar of sections with search, and each section's
//  rows. Mac-only parts (the screenshot folder bookmark, MIDI devices and
//  assignments, the folder actions) are added where the schema places them.
//

import SwiftUI

/// Which section is showing and the search text, shared so the self-test
/// can drive the window.
@Observable
final class SettingsNavigation {
    static let shared = SettingsNavigation()
    var section: String?
    var query = ""
    private init() {}
}

struct SettingsView: View {
    @State private var settings = SettingsStore.shared
    @State private var nav = SettingsNavigation.shared

    var body: some View {
        @Bindable var nav = nav

        NavigationSplitView {
            List(selection: $nav.section) {
                ForEach(settings.sections, id: \.self) { section in
                    Label(section, systemImage: Self.icon(for: section))
                        .tag(section)
                }
            }
            .navigationSplitViewColumnWidth(min: 170, ideal: 190, max: 240)
            .searchable(text: $nav.query, placement: .sidebar, prompt: "Search")
        } detail: {
            SettingsDetailView(section: nav.section ?? settings.sections.first, query: nav.query)
        }
        .frame(minWidth: 680, idealWidth: 720, minHeight: 460, idealHeight: 540)
        .onAppear {
            if nav.section == nil { nav.section = settings.sections.first }
        }
    }

    static func icon(for section: String) -> String {
        switch section {
        case "Appearance": return "paintpalette"
        case "Output": return "rectangle.on.rectangle"
        case "Graph": return "point.3.connected.trianglepath.dotted"
        case "Browser": return "square.grid.2x2"
        case "MIDI": return "pianokeys"
        case "Screenshots": return "camera"
        case "Advanced": return "gearshape.2"
        default: return "gearshape"
        }
    }
}

// MARK: - Detail

private struct SettingsDetailView: View {
    let section: String?
    let query: String
    private var settings: SettingsStore { .shared }

    private var terms: [String] {
        query.lowercased().split(separator: " ").map(String.init)
    }

    private func matches(_ text: String) -> Bool {
        terms.allSatisfy { text.contains($0) }
    }

    var body: some View {
        Form {
            if terms.isEmpty {
                if let section { sectionContent(section, rows: settings.schema.filter { $0.section == section }) }
            } else {
                searchResults
            }
        }
        .formStyle(.grouped)
        .navigationTitle(terms.isEmpty ? (section ?? "Settings") : "Search")
    }

    @ViewBuilder
    private var searchResults: some View {
        let hits = settings.sections.map { s in (s, settings.schema.filter { $0.section == s && matches($0.searchText) }) }
            .filter { !$0.1.isEmpty || ($0.0 == "MIDI" && matches(MidiExtrasView.searchText)) }
        if hits.isEmpty {
            Text("No settings match “\(query)”.")
                .foregroundStyle(.secondary)
        }
        ForEach(hits, id: \.0) { section, rows in
            ForEach(Self.chunks(rows), id: \.first!.id) { chunk in
                Section {
                    ForEach(chunk) { SettingRow(setting: $0) }
                } header: {
                    Text(chunk[0].group.isEmpty ? section : "\(section) › \(chunk[0].group)")
                }
            }
            if section == "MIDI" && matches(MidiExtrasView.searchText) { MidiExtrasView() }
        }
    }

    @ViewBuilder
    private func sectionContent(_ section: String, rows: [SettingInfo]) -> some View {
        ForEach(Self.chunks(rows), id: \.first!.id) { chunk in
            Section {
                ForEach(chunk) { SettingRow(setting: $0) }
            } header: {
                if !chunk[0].group.isEmpty { Text(chunk[0].group) }
            } footer: {
                if section == "Screenshots" && chunk.last?.id == rows.last?.id {
                    Text("In a preview window: S to save, ⇧S to choose the folder, C to copy.")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
            }
        }
        if section == "MIDI" { MidiExtrasView() }
        if rows.contains(where: { $0.kind != .action }) {
            Section {
                HStack {
                    Spacer()
                    Button("Reset \(section) to Defaults") { settings.reset(section: section) }
                }
            }
        }
    }

    /// Consecutive rows sharing a group become one form section.
    static func chunks(_ rows: [SettingInfo]) -> [[SettingInfo]] {
        var out: [[SettingInfo]] = []
        for row in rows {
            if let last = out.last?.last, last.group == row.group { out[out.count - 1].append(row) }
            else { out.append([row]) }
        }
        return out
    }
}

// MARK: - Rows

private struct SettingRow: View {
    let setting: SettingInfo
    private var settings: SettingsStore { .shared }
    @State private var confirmReset = false

    private var enabled: Bool {
        setting.dependsOn.map { settings.bool($0) } ?? true
    }

    var body: some View {
        control
            .disabled(!enabled)
            .opacity(enabled ? 1 : 0.45)  // labels included, not just the control
    }

    @ViewBuilder
    private var title: some View {
        Text(setting.label)
        if !help.isEmpty { Text(help) }
    }

    private var help: String {
        setting.needsRestart ? setting.help + " Takes effect after restarting Nottawa." : setting.help
    }

    @ViewBuilder
    private var control: some View {
        switch setting.kind {
        case .bool:
            Toggle(isOn: settings.binding(setting.id)) { title }
        case .int, .float:
            LabeledContent {
                HStack(spacing: 8) {
                    // Snapped to the step in the binding: a stepped Slider draws a tick per step.
                    Slider(value: settings.numberBinding(setting.id, step: setting.step), in: setting.min...setting.max)
                    Text(formatted(settings.number(setting.id)))
                        .monospacedDigit()
                        .foregroundStyle(.secondary)
                        .frame(width: 60, alignment: .trailing)
                }
                .frame(maxWidth: 260)
            } label: { title }
        case .choice:
            Picker(selection: settings.stringBinding(setting.id)) {
                ForEach(setting.choices, id: \.id) { Text($0.label).tag($0.id) }
            } label: { title }
            .pickerStyle(.menu)
        case .string:
            LabeledContent {
                TextField("", text: settings.stringBinding(setting.id))
                    .frame(maxWidth: 240)
            } label: { title }
        case .path:
            if setting.id == "screenshots.folder" {
                ScreenshotFolderRow(title: setting.label, help: help)
            } else {
                PathRow(setting: setting, help: help)
            }
        case .action:
            LabeledContent {
                Button(setting.label) { perform() }
                    .confirmationDialog("Reset every setting to its default?", isPresented: $confirmReset) {
                        Button("Reset All Settings", role: .destructive) { resetAll() }
                    } message: {
                        Text("Workspaces and your MIDI assignments aren't affected.")
                    }
            } label: { title }
        }
    }

    /// Number with the setting's decimals and unit; milliseconds from a
    /// second up read as seconds.
    private func formatted(_ value: Double) -> String {
        if setting.unit == "ms" && value >= 1000 { return String(format: "%.2f s", value / 1000) }
        let number = String(format: "%.\(setting.decimals)f", value)
        return setting.unit.isEmpty ? number : "\(number) \(setting.unit)"
    }

    private func perform() {
        switch setting.id {
        case "advanced.openLogs": NSWorkspace.shared.open(SettingsStore.logsFolder)
        case "advanced.openData": NSWorkspace.shared.open(SettingsStore.dataFolder)
        case "advanced.resetAll": confirmReset = true
        default: break
        }
    }

    private func resetAll() {
        settings.reset(section: nil)
        ScreenshotService.shared.clearFolder()
    }
}

/// The screenshot folder is a security-scoped bookmark the Mac app keeps
/// itself; the engine's value isn't used.
private struct ScreenshotFolderRow: View {
    let title: String
    let help: String
    @State private var path: String? = ScreenshotService.shared.currentFolderDisplayPath

    var body: some View {
        LabeledContent {
            HStack(spacing: 6) {
                Text(path ?? "Asked on first capture")
                    .foregroundStyle(path == nil ? .secondary : .primary)
                    .lineLimit(1)
                    .truncationMode(.head)
                Button("Change…") {
                    if let url = ScreenshotService.shared.chooseFolder() { path = url.path }
                }
            }
        } label: {
            Text(title)
            Text(help)
        }
    }
}

/// A folder kept as a plain path in the engine (none on the Mac yet).
private struct PathRow: View {
    let setting: SettingInfo
    let help: String
    private var settings: SettingsStore { .shared }

    var body: some View {
        LabeledContent {
            HStack(spacing: 6) {
                let value = settings.string(setting.id)
                Text(value.isEmpty ? "Not set" : value)
                    .foregroundStyle(value.isEmpty ? .secondary : .primary)
                    .lineLimit(1)
                    .truncationMode(.head)
                Button("Choose…") {
                    let panel = NSOpenPanel()
                    panel.canChooseDirectories = true
                    panel.canChooseFiles = false
                    if panel.runModal() == .OK, let url = panel.url { settings.set(setting.id, string: url.path) }
                }
            }
        } label: {
            Text(setting.label)
            Text(help)
        }
    }
}

// MARK: - MIDI devices and assignments (Mac-side, from bridge queries)

private struct MidiExtrasView: View {
    static let searchText = "midi devices assignments controller learn bindings"
    @State private var inputs: [MidiInputInfo] = []
    @State private var assignments: [MidiAssignmentInfo] = []

    var body: some View {
        Section("Devices") {
            if inputs.isEmpty {
                Text("No MIDI devices connected").foregroundStyle(.secondary)
            } else {
                ForEach(inputs) { input in
                    LabeledContent(input.name) {
                        Image(systemName: input.isOpen ? "circle.fill" : "circle")
                            .font(.caption2)
                            .foregroundStyle(input.isOpen ? .green : .secondary)
                    }
                }
            }
        }
        .onAppear(perform: refresh)

        Section("Assignments") {
            if assignments.isEmpty {
                Text("No MIDI assignments")
                    .foregroundStyle(.secondary)
            } else {
                ForEach(assignments) { assignment in
                    LabeledContent {
                        Button(role: .destructive) {
                            NottawaEngine.shared.removeMidiBinding(paramId: assignment.paramId)
                            refresh()
                        } label: {
                            Image(systemName: "trash")
                        }
                        .buttonStyle(.borderless)
                    } label: {
                        Text("\(assignment.ownerName) / \(assignment.paramName)")
                        Text(assignment.controlLabel)
                    }
                }
            }
        }
    }

    private func refresh() {
        inputs = NottawaEngine.shared.midiInputs()
        assignments = NottawaEngine.shared.midiAssignments()
    }
}

// MARK: - Bindings

extension SettingsStore {
    func binding(_ id: String) -> Binding<Bool> {
        Binding(get: { self.bool(id) }, set: { self.set(id, bool: $0) })
    }

    func numberBinding(_ id: String, step: Double = 0) -> Binding<Double> {
        Binding(get: { self.number(id) }, set: { value in
            let snapped = step > 0 ? (value / step).rounded() * step : value
            if snapped != self.number(id) { self.set(id, number: snapped) }
        })
    }

    func stringBinding(_ id: String) -> Binding<String> {
        Binding(get: { self.string(id) }, set: { self.set(id, string: $0) })
    }
}

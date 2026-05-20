//
//  SettingsView.swift
//  NottawaApp
//
//  Settings window with resolution, theme, and MIDI configuration.
//

import SwiftUI

struct SettingsView: View {
    @Environment(ThemeManager.self) private var theme
    @State private var resolutionIndex = 3
    @State private var midiInputs: [MidiInputInfo] = []
    @State private var midiAssignments: [MidiAssignmentInfo] = []
    @State private var midiSmoothingEnabled: Bool = false
    @State private var midiSmoothingDurationMs: Double = 80.0
    @AppStorage("showNodeHints") private var showNodeHints: Bool = true
    @AppStorage("autoAdvanceOutputWindow") private var autoAdvanceOutputWindow: Bool = true
    @AppStorage("outputWindowFadeDuration") private var outputWindowFadeDuration: Double = 0.3
    @State private var screenshotService = ScreenshotService.shared
    @State private var screenshotFolderPath: String?

    // Index → label table mirrors LayoutStateService.cpp::updateResolutionSettings.
    // Previously these labels were offset by two against the actual C++ mapping,
    // so picking "480p" silently set the engine to 240p, etc.
    private let resolutionOptions: [(name: String, index: Int)] = [
        ("240p",  0),
        ("360p",  1),
        ("480p",  2),
        ("720p",  3),
        ("1080p", 4),
        ("1440p", 5),
        ("4K",    6),
    ]

    var body: some View {
        @Bindable var theme = theme

        Form {
            // MARK: - Display

            Section("Display") {
                DSPicker(
                    selection: $resolutionIndex,
                    options: resolutionOptions.map { DSPickerOption(value: $0.index, label: $0.name) },
                    label: "Resolution", style: .menu
                )
                .onChange(of: resolutionIndex) { _, newValue in
                    NottawaEngine.shared.setResolution(settingIndex: newValue)
                }

                Picker("Theme", selection: $theme.preset) {
                    ForEach(ThemePreset.allCases) { preset in
                        Text(preset.rawValue).tag(preset)
                    }
                }

                Toggle("Show Node Hints", isOn: $showNodeHints)
                Toggle("Automatically advance output window", isOn: $autoAdvanceOutputWindow)

                HStack {
                    Text("Advance Fade")
                    Slider(value: $outputWindowFadeDuration, in: 0...2)
                        .disabled(!autoAdvanceOutputWindow)
                    Text(String(format: "%.1fs", outputWindowFadeDuration))
                        .font(.caption)
                        .monospacedDigit()
                        .foregroundStyle(.secondary)
                        .frame(width: 36, alignment: .trailing)
                }
            }

            // MARK: - Screenshots

            Section("Screenshots") {
                LabeledContent("Folder") {
                    HStack(spacing: 6) {
                        Text(screenshotFolderPath ?? "Not set — chosen on first capture")
                            .font(.caption)
                            .foregroundStyle(screenshotFolderPath == nil ? .secondary : .primary)
                            .lineLimit(1)
                            .truncationMode(.head)
                        Button("Change…") {
                            if let url = screenshotService.chooseFolder() {
                                screenshotFolderPath = url.path
                            }
                        }
                        .controlSize(.small)
                    }
                }

                Picker("Format", selection: Binding(
                    get: { screenshotService.format },
                    set: { screenshotService.format = $0 }
                )) {
                    Text("PNG").tag(ScreenshotFormat.png)
                    Text("JPEG").tag(ScreenshotFormat.jpeg)
                }

                Text("In a preview window: S to save, ⇧S to choose folder, C to copy.")
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }

            // MARK: - MIDI Smoothing

            Section("MIDI Smoothing") {
                Toggle("Enable Smoothing", isOn: $midiSmoothingEnabled)
                    .onChange(of: midiSmoothingEnabled) { _, newValue in
                        NottawaEngine.shared.midiSmoothingEnabled = newValue
                    }

                HStack {
                    Text("Duration")
                    Slider(value: $midiSmoothingDurationMs, in: 0...500)
                        .disabled(!midiSmoothingEnabled)
                        .onChange(of: midiSmoothingDurationMs) { _, newValue in
                            NottawaEngine.shared.midiSmoothingDurationMs = Float(newValue)
                        }
                    Text(String(format: "%.0f ms", midiSmoothingDurationMs))
                        .font(.caption)
                        .monospacedDigit()
                        .foregroundStyle(.secondary)
                        .frame(width: 56, alignment: .trailing)
                }

                Text("Eases MIDI knob/fader changes over the chosen duration. Helpful for 7-bit CC stepping; bypassed for toggles.")
                    .font(.caption2)
                    .foregroundStyle(.secondary)
            }

            // MARK: - MIDI Devices

            Section("MIDI Devices") {
                if midiInputs.isEmpty {
                    Text("No MIDI devices connected")
                        .foregroundStyle(.secondary)
                        .font(.caption)
                } else {
                    ForEach(midiInputs) { input in
                        LabeledContent(input.name) {
                            Image(systemName: input.isOpen ? "circle.fill" : "circle")
                                .font(.caption2)
                                .foregroundStyle(input.isOpen ? .green : .secondary)
                        }
                        .font(.caption)
                    }
                }
            }

            // MARK: - MIDI Assignments

            Section("MIDI Assignments") {
                if midiAssignments.isEmpty {
                    Text("No MIDI assignments")
                        .foregroundStyle(.secondary)
                        .font(.caption)
                } else {
                    ForEach(midiAssignments) { assignment in
                        HStack {
                            VStack(alignment: .leading, spacing: 2) {
                                Text("\(assignment.ownerName) / \(assignment.paramName)")
                                    .font(.caption)
                                Text(assignment.controlLabel)
                                    .font(.caption2)
                                    .foregroundStyle(.secondary)
                            }
                            Spacer()
                            Button(role: .destructive) {
                                NottawaEngine.shared.removeMidiBinding(paramId: assignment.paramId)
                                refreshMidi()
                            } label: {
                                Image(systemName: "trash")
                                    .font(.caption)
                            }
                            .buttonStyle(.borderless)
                        }
                    }
                }
            }
        }
        .formStyle(.grouped)
        .frame(width: 380)
        .onAppear {
            resolutionIndex = NottawaEngine.shared.resolutionSettingIndex
            midiSmoothingEnabled = NottawaEngine.shared.midiSmoothingEnabled
            midiSmoothingDurationMs = Double(NottawaEngine.shared.midiSmoothingDurationMs)
            refreshMidi()
            screenshotFolderPath = screenshotService.currentFolderDisplayPath
        }
    }

    private func refreshMidi() {
        midiInputs = NottawaEngine.shared.midiInputs()
        midiAssignments = NottawaEngine.shared.midiAssignments()
    }
}

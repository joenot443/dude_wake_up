//
//  SettingsView.swift
//  NottawaApp
//
//  Settings window with resolution, theme, and MIDI configuration.
//

import SwiftUI

struct SettingsView: View {
    @Environment(ThemeManager.self) private var theme
    @State private var resolutionIndex = 2
    @State private var midiInputs: [MidiInputInfo] = []
    @State private var midiAssignments: [MidiAssignmentInfo] = []
    @AppStorage("showNodeHints") private var showNodeHints: Bool = true

    private let resolutionOptions = [
        (name: "480p", index: 0),
        (name: "540p", index: 1),
        (name: "720p", index: 2),
        (name: "1080p", index: 3),
        (name: "1440p", index: 4),
        (name: "4K", index: 5),
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
            refreshMidi()
        }
    }

    private func refreshMidi() {
        midiInputs = NottawaEngine.shared.midiInputs()
        midiAssignments = NottawaEngine.shared.midiAssignments()
    }
}

//
//  MidiControlsView.swift
//  NottawaApp
//
//  Inline MIDI binding controls for a parameter: learn, display binding, remove.
//

import SwiftUI

struct MidiControlsView: View {
    @Environment(ThemeManager.self) private var theme
    @Environment(NodeEditorViewModel.self) private var viewModel
    let param: ParameterInfo

    @State private var hasMidi: Bool
    @State private var descriptor: String?
    @State private var isLearning = false

    init(param: ParameterInfo) {
        self.param = param
        self._hasMidi = State(initialValue: param.hasMidiBinding)
        self._descriptor = State(initialValue: param.midiDescriptor)
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            if isLearning {
                // Learning state — waiting for MIDI input
                HStack {
                    ProgressView()
                        .controlSize(.small)
                    Text("Listening for MIDI...")
                        .font(.caption)
                        .foregroundStyle(.purple)
                    Spacer()
                    Button {
                        NottawaEngine.shared.stopMidiLearning()
                        isLearning = false
                    } label: {
                        Image(systemName: "xmark.circle.fill")
                            .font(.caption)
                            .foregroundStyle(theme.colors.textSecondary)
                    }
                    .buttonStyle(.plainHitArea)
                }
            } else if hasMidi, let desc = descriptor {
                // Bound — show descriptor and remove button
                HStack {
                    Image(systemName: "pianokeys")
                        .font(.caption2)
                        .foregroundStyle(.purple)
                    Text(formatDescriptor(desc))
                        .font(.caption)
                        .fontWeight(.medium)
                    Spacer()
                    Button {
                        NottawaEngine.shared.removeMidiBinding(paramId: param.id)
                        hasMidi = false
                        descriptor = nil
                        viewModel.refreshInspector()
                    } label: {
                        Image(systemName: "xmark.circle.fill")
                            .font(.caption)
                            .foregroundStyle(theme.colors.textSecondary)
                    }
                    .buttonStyle(.plainHitArea)
                }
            } else {
                // No binding — show learn button
                HStack {
                    Text("MIDI")
                        .font(.caption)
                    Spacer()
                    Button {
                        NottawaEngine.shared.beginMidiLearning(paramId: param.id)
                        isLearning = true
                    } label: {
                        Label("Learn", systemImage: "pianokeys")
                            .font(.caption)
                    }
                    .buttonStyle(.plainHitArea)
                }
            }
        }
        .padding(8)
        .background(Color.purple.opacity(0.06))
        .clipShape(RoundedRectangle(cornerRadius: 6))
        .accessibilityIdentifier("midi-controls-\(param.id)")
        .onReceive(NotificationCenter.default.publisher(for: .parameterRefresh)) { _ in
            guard let fresh = viewModel.inspectorParameters.first(where: { $0.id == param.id }) else { return }
            if fresh.hasMidiBinding != hasMidi { hasMidi = fresh.hasMidiBinding }
            if fresh.midiDescriptor != descriptor { descriptor = fresh.midiDescriptor }
            // If we were learning and now have a binding, stop the learning indicator
            if isLearning && fresh.hasMidiBinding {
                isLearning = false
            }
        }
    }

    /// Format the raw MIDI descriptor string (portNum:portName:status:channel:control:$)
    /// into a human-readable label like "CC 7 · Ch 1".
    private func formatDescriptor(_ raw: String) -> String {
        let parts = raw.split(separator: ":")
        guard parts.count >= 5 else { return raw }
        let control = parts[4]
        let channel = parts[3]
        return "CC \(control) · Ch \(channel)"
    }
}

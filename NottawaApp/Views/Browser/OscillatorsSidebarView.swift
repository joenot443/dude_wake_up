//
//  OscillatorsSidebarView.swift
//  NottawaApp
//
//  Sidebar panel showing all active oscillators across all nodes.
//  Displays live waveform visualization and inline controls for each.
//

import SwiftUI

struct OscillatorsSidebarView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel

    @State private var oscillators: [ActiveOscillatorInfo] = []
    private let refreshTimer = Timer.publish(every: 0.5, on: .main, in: .common).autoconnect()

    var body: some View {
        Group {
            if oscillators.isEmpty {
                emptyState
            } else {
                oscillatorList
            }
        }
        .onAppear { refresh() }
        .onReceive(refreshTimer) { _ in refresh() }
    }

    // MARK: - Empty State

    private var emptyState: some View {
        VStack(spacing: 8) {
            Spacer()
            Image(systemName: "waveform.path")
                .font(.system(size: 32))
                .foregroundStyle(.secondary)
            Text("No active Oscillators")
                .font(.caption)
                .foregroundStyle(.secondary)
            Spacer()
        }
        .frame(maxWidth: .infinity)
    }

    // MARK: - Oscillator List

    private var oscillatorList: some View {
        ScrollView {
            LazyVStack(spacing: 10) {
                ForEach(oscillators) { osc in
                    OscillatorRowView(oscillator: osc) {
                        viewModel.selectNode(osc.connectableId)
                    }
                }
            }
            .padding(8)
        }
    }

    // MARK: - Refresh

    private func refresh() {
        oscillators = NottawaEngine.shared.activeOscillators()
    }
}

// MARK: - Oscillator Row

private struct OscillatorRowView: View {
    let oscillator: ActiveOscillatorInfo
    let onTapHeader: () -> Void

    @State private var enabled: Bool
    @State private var waveShape: WaveShape
    @State private var frequency: Float
    @State private var minOutput: Float
    @State private var maxOutput: Float

    init(oscillator: ActiveOscillatorInfo, onTapHeader: @escaping () -> Void) {
        self.oscillator = oscillator
        self.onTapHeader = onTapHeader
        self._enabled = State(initialValue: oscillator.enabled)
        self._waveShape = State(initialValue: oscillator.waveShape)
        self._frequency = State(initialValue: oscillator.frequency)
        self._minOutput = State(initialValue: oscillator.minOutput)
        self._maxOutput = State(initialValue: oscillator.maxOutput)
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            // Header
            Button(action: onTapHeader) {
                HStack {
                    Text("\(oscillator.paramName)")
                        .font(.caption)
                        .fontWeight(.semibold)
                    Text("—")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                    Text(oscillator.ownerName)
                        .font(.caption)
                        .foregroundStyle(.secondary)
                    Spacer()
                }
                .contentShape(Rectangle())
            }
            .buttonStyle(.plain)

            // Waveform preview
            WaveformPreviewView(
                waveShape: waveShape,
                frequency: frequency,
                minOutput: minOutput,
                maxOutput: maxOutput,
                paramMin: oscillator.paramMin,
                paramMax: oscillator.paramMax,
                enabled: enabled
            )

            // Enable toggle + wave shape picker
            HStack {
                Toggle("Osc", isOn: $enabled)
                    .toggleStyle(.switch)
                    .controlSize(.small)
                    .onChange(of: enabled) { _, newValue in
                        NottawaEngine.shared.setOscillatorEnabled(
                            paramId: oscillator.paramId, enabled: newValue)
                    }

                Spacer()

                if enabled {
                    Picker("", selection: $waveShape) {
                        ForEach(WaveShape.allCases) { shape in
                            Text(shape.displayName).tag(shape)
                        }
                    }
                    .pickerStyle(.menu)
                    .labelsHidden()
                    .frame(width: 90)
                    .onChange(of: waveShape) { _, newValue in
                        NottawaEngine.shared.setOscillatorWaveShape(
                            paramId: oscillator.paramId, waveShape: newValue.rawValue)
                    }
                }
            }

            if enabled {
                // Frequency
                VStack(alignment: .leading, spacing: 1) {
                    HStack {
                        Text("Frequency")
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                        Spacer()
                        Text(String(format: "%.2f", frequency))
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                            .monospacedDigit()
                    }
                    Slider(value: $frequency, in: 0...4)
                        .controlSize(.small)
                        .onChange(of: frequency) { _, newValue in
                            NottawaEngine.shared.setOscillatorFrequency(
                                paramId: oscillator.paramId, frequency: newValue)
                        }
                }

                // Min Output
                VStack(alignment: .leading, spacing: 1) {
                    HStack {
                        Text("Min")
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                        Spacer()
                        Text(String(format: "%.2f", minOutput))
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                            .monospacedDigit()
                    }
                    Slider(value: $minOutput, in: oscillator.paramMin...oscillator.paramMax)
                        .controlSize(.small)
                        .onChange(of: minOutput) { _, newValue in
                            NottawaEngine.shared.setOscillatorMinOutput(
                                paramId: oscillator.paramId, minOutput: newValue)
                        }
                }

                // Max Output
                VStack(alignment: .leading, spacing: 1) {
                    HStack {
                        Text("Max")
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                        Spacer()
                        Text(String(format: "%.2f", maxOutput))
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                            .monospacedDigit()
                    }
                    Slider(value: $maxOutput, in: oscillator.paramMin...oscillator.paramMax)
                        .controlSize(.small)
                        .onChange(of: maxOutput) { _, newValue in
                            NottawaEngine.shared.setOscillatorMaxOutput(
                                paramId: oscillator.paramId, maxOutput: newValue)
                        }
                }
            }
        }
        .font(.caption)
        .padding(8)
        .background(Color.secondary.opacity(0.08))
        .clipShape(RoundedRectangle(cornerRadius: 6))
        .onChange(of: oscillator.enabled) { _, v in enabled = v }
        .onChange(of: oscillator.frequency) { _, v in frequency = v }
        .onChange(of: oscillator.minOutput) { _, v in minOutput = v }
        .onChange(of: oscillator.maxOutput) { _, v in maxOutput = v }
        .onChange(of: oscillator.waveShape) { _, v in waveShape = v }
    }
}

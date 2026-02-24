//
//  OscillatorControlsView.swift
//  NottawaApp
//
//  Inline oscillator controls for a parameter: enable toggle, waveform shape,
//  frequency, min output, max output.
//

import SwiftUI

struct OscillatorControlsView: View {
    let param: ParameterInfo

    @State private var enabled: Bool
    @State private var waveShape: WaveShape
    @State private var frequency: Float
    @State private var minOutput: Float
    @State private var maxOutput: Float

    init(param: ParameterInfo) {
        self.param = param
        self._enabled = State(initialValue: param.oscillatorEnabled)
        self._waveShape = State(initialValue: param.oscillatorWaveShape)
        self._frequency = State(initialValue: param.oscillatorFrequency)
        self._minOutput = State(initialValue: param.oscillatorMinOutput)
        self._maxOutput = State(initialValue: param.oscillatorMaxOutput)
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            // Waveform visualization
            WaveformPreviewView(
                waveShape: waveShape,
                frequency: frequency,
                minOutput: minOutput,
                maxOutput: maxOutput,
                paramMin: param.minValue,
                paramMax: param.maxValue,
                enabled: enabled
            )

            // Enable toggle + waveform picker
            HStack {
                Toggle("Oscillator", isOn: $enabled)
                    .toggleStyle(.switch)
                    .controlSize(.small)
                    .accessibilityIdentifier("osc-enable-\(param.id)")
                    .onChange(of: enabled) { _, newValue in
                        NottawaEngine.shared.setOscillatorEnabled(paramId: param.id, enabled: newValue)
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
                    .frame(width: 100)
                    .accessibilityIdentifier("osc-waveshape-\(param.id)")
                    .onChange(of: waveShape) { _, newValue in
                        NottawaEngine.shared.setOscillatorWaveShape(paramId: param.id, waveShape: newValue.rawValue)
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
                        .accessibilityIdentifier("osc-frequency-\(param.id)")
                        .onChange(of: frequency) { _, newValue in
                            NottawaEngine.shared.setOscillatorFrequency(paramId: param.id, frequency: newValue)
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
                    Slider(value: $minOutput, in: param.minValue...param.maxValue)
                        .controlSize(.small)
                        .onChange(of: minOutput) { _, newValue in
                            NottawaEngine.shared.setOscillatorMinOutput(paramId: param.id, minOutput: newValue)
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
                    Slider(value: $maxOutput, in: param.minValue...param.maxValue)
                        .controlSize(.small)
                        .onChange(of: maxOutput) { _, newValue in
                            NottawaEngine.shared.setOscillatorMaxOutput(paramId: param.id, maxOutput: newValue)
                        }
                }
            }
        }
        .font(.caption)
        .padding(8)
        .background(Color.secondary.opacity(0.08))
        .clipShape(RoundedRectangle(cornerRadius: 6))
        .accessibilityIdentifier("osc-controls-\(param.id)")
        .onChange(of: param.oscillatorEnabled) { _, newValue in
            enabled = newValue
        }
        .onChange(of: param.oscillatorFrequency) { _, newValue in
            frequency = newValue
        }
        .onChange(of: param.oscillatorMinOutput) { _, newValue in
            minOutput = newValue
        }
        .onChange(of: param.oscillatorMaxOutput) { _, newValue in
            maxOutput = newValue
        }
        .onChange(of: param.oscillatorWaveShape) { _, newValue in
            waveShape = newValue
        }
    }
}

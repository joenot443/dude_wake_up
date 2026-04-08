//
//  OscillatorControlsView.swift
//  NottawaApp
//
//  Inline oscillator controls for a parameter: enable toggle, waveform shape,
//  frequency, min output, max output.
//

import SwiftUI

struct OscillatorControlsView: View {
    @Environment(ThemeManager.self) private var theme
    let param: ParameterInfo
    @Binding var enabled: Bool

    @State private var waveShape: WaveShape
    @State private var frequency: Float
    @State private var minOutput: Float
    @State private var maxOutput: Float

    init(param: ParameterInfo, enabled: Binding<Bool>) {
        self.param = param
        self._enabled = enabled
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
                DSToggle(isOn: $enabled, label: "Oscillator", style: .switch, size: .sm)
                    .accessibilityIdentifier("osc-enable-\(param.id)")
                    .onChange(of: enabled) { _, newValue in
                        NottawaEngine.shared.setOscillatorEnabled(paramId: param.id, enabled: newValue)
                    }

                Spacer()

                if enabled {
                    DSPicker(
                        selection: $waveShape,
                        options: WaveShape.allCases.map { DSPickerOption(value: $0, label: $0.displayName) },
                        style: .menu, size: .sm
                    )
                    .frame(width: 100)
                    .accessibilityIdentifier("osc-waveshape-\(param.id)")
                    .onChange(of: waveShape) { _, newValue in
                        NottawaEngine.shared.setOscillatorWaveShape(paramId: param.id, waveShape: newValue.rawValue)
                    }
                }
            }

            if enabled {
                // Frequency
                DSSlider(value: $frequency, range: 0...4, label: "Frequency", showValue: true)
                    .accessibilityIdentifier("osc-frequency-\(param.id)")
                    .onChange(of: frequency) { _, newValue in
                        NottawaEngine.shared.setOscillatorFrequency(paramId: param.id, frequency: newValue)
                    }

                // Min Output
                DSSlider(value: $minOutput, range: param.minValue...param.maxValue, label: "Min", showValue: true)
                    .onChange(of: minOutput) { _, newValue in
                        NottawaEngine.shared.setOscillatorMinOutput(paramId: param.id, minOutput: newValue)
                    }

                // Max Output
                DSSlider(value: $maxOutput, range: param.minValue...param.maxValue, label: "Max", showValue: true)
                    .onChange(of: maxOutput) { _, newValue in
                        NottawaEngine.shared.setOscillatorMaxOutput(paramId: param.id, maxOutput: newValue)
                    }
            }
        }
        .font(.caption)
        .padding(8)
        .background(theme.colors.surface)
        .clipShape(RoundedRectangle(cornerRadius: 6))
        .accessibilityIdentifier("osc-controls-\(param.id)")
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

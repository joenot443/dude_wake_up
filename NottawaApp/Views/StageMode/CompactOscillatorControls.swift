//
//  CompactOscillatorControls.swift
//  NottawaApp
//
//  Inline oscillator controls for Stage Mode. Shows a mini live waveform preview,
//  wave shape picker, and freq/min/max sliders. The enable icon lives in the
//  parent's title row; this view is only rendered when the oscillator is enabled.
//

import SwiftUI

struct CompactOscillatorControls: View {
    @Environment(ThemeManager.self) private var theme
    let param: ParameterInfo

    @State private var waveShape: WaveShape
    @State private var frequency: Float
    @State private var minOutput: Float
    @State private var maxOutput: Float

    init(param: ParameterInfo) {
        self.param = param
        self._waveShape = State(initialValue: param.oscillatorWaveShape)
        self._frequency = State(initialValue: param.oscillatorFrequency)
        self._minOutput = State(initialValue: param.oscillatorMinOutput)
        self._maxOutput = State(initialValue: param.oscillatorMaxOutput)
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            // Row 1: Mini waveform preview + Wave shape picker
            HStack(spacing: 6) {
                MiniWaveformView(
                    waveShape: waveShape,
                    frequency: frequency,
                    minOutput: minOutput,
                    maxOutput: maxOutput,
                    paramMin: param.minValue,
                    paramMax: param.maxValue
                )
                .frame(height: 24)

                Picker("", selection: $waveShape) {
                    ForEach(WaveShape.allCases) { shape in
                        Text(shape.displayName).tag(shape)
                    }
                }
                .pickerStyle(.menu)
                .labelsHidden()
                .controlSize(.mini)
                .fixedSize()
                .onChange(of: waveShape) { _, newValue in
                    NottawaEngine.shared.setOscillatorWaveShape(paramId: param.id, waveShape: newValue.rawValue)
                }
            }

            // Frequency slider
            DSSlider(value: $frequency, range: 0...4, label: "Freq", showValue: true, formatString: "%.1f")
                .onChange(of: frequency) { _, newValue in
                    NottawaEngine.shared.setOscillatorFrequency(paramId: param.id, frequency: newValue)
                }

            // Min output slider
            DSSlider(value: $minOutput, range: param.minValue...param.maxValue, label: "Min", showValue: true, formatString: "%.1f")
                .onChange(of: minOutput) { _, newValue in
                    NottawaEngine.shared.setOscillatorMinOutput(paramId: param.id, minOutput: newValue)
                }

            // Max output slider
            DSSlider(value: $maxOutput, range: param.minValue...param.maxValue, label: "Max", showValue: true, formatString: "%.1f")
                .onChange(of: maxOutput) { _, newValue in
                    NottawaEngine.shared.setOscillatorMaxOutput(paramId: param.id, maxOutput: newValue)
                }
        }
        .font(.caption2)
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

// MARK: - Mini Waveform Preview

/// Compact live waveform preview (~24pt tall) for Stage Mode.
/// Uses the same waveform math as WaveformPreviewView but at thumbnail scale.
private struct MiniWaveformView: View {
    let waveShape: WaveShape
    let frequency: Float
    let minOutput: Float
    let maxOutput: Float
    let paramMin: Float
    let paramMax: Float

    @State private var randOffset: Double = Double.random(in: 0 ..< .pi * 2)

    var body: some View {
        TimelineView(.animation(minimumInterval: 1.0 / 30.0)) { timeline in
            let phase = timeline.date.timeIntervalSinceReferenceDate
            Canvas { context, size in
                let w = size.width
                let h = size.height

                // Background
                context.fill(
                    Path(roundedRect: CGRect(origin: .zero, size: size), cornerRadius: 3),
                    with: .color(.black.opacity(0.25))
                )

                let actualMin = Double(Swift.min(minOutput, maxOutput))
                let actualMax = Double(Swift.max(minOutput, maxOutput))
                let pMin = Double(paramMin)
                let pMax = Double(paramMax)
                let yRange = pMax - pMin
                guard yRange > 0 else { return }

                func yPos(_ val: Double) -> CGFloat {
                    CGFloat(1.0 - (val - pMin) / yRange) * h
                }

                // Waveform line
                let sampleCount = Int(w)
                guard sampleCount > 1 else { return }

                let timeWindow = 4.0
                let freq = Double(frequency)
                var path = Path()

                for i in 0..<sampleCount {
                    let frac = Double(i) / Double(sampleCount - 1)
                    let t = phase - timeWindow + frac * timeWindow
                    let freqT = freq * t + randOffset

                    let v = waveformValue(shape: waveShape, freqT: freqT, t: t, freq: freq)
                    let normalized = (v + 1.0) / 2.0
                    let output = actualMin + normalized * (actualMax - actualMin)
                    let clamped = Swift.min(Swift.max(output, pMin), pMax)

                    let x = CGFloat(frac) * w
                    let y = yPos(clamped)

                    if i == 0 {
                        path.move(to: CGPoint(x: x, y: y))
                    } else {
                        path.addLine(to: CGPoint(x: x, y: y))
                    }
                }

                context.stroke(
                    path,
                    with: .color(.cyan),
                    style: StrokeStyle(lineWidth: 1.5, lineCap: .round, lineJoin: .round)
                )
            }
        }
        .clipShape(RoundedRectangle(cornerRadius: 3))
    }

    // MARK: - Waveform Math (mirrors WaveformOscillator::tick)

    private func waveformValue(shape: WaveShape, freqT: Double, t: Double, freq: Double) -> Double {
        switch shape {
        case .sine:
            return sin(freqT)
        case .square:
            return sin(freqT) >= 0 ? 1.0 : -1.0
        case .sawtooth:
            return 2.0 * posMod(freqT / (.pi * 2), 1.0) - 1.0
        case .triangle:
            let saw = posMod(freqT / (.pi * 2), 1.0)
            return 2.0 * abs(2.0 * saw - 1.0) - 1.0
        case .pulseTrain:
            return posMod(freqT / (.pi * 2), 1.0) < 0.1 ? 1.0 : -1.0
        case .exponentialSine:
            let s = sin(freqT)
            let raw = s * exp(s)
            return Swift.min(1.0, Swift.max(-1.0, (raw - 1.175) / 1.55))
        case .harmonicStack:
            let norm = 1.0 + 0.5 + 0.25
            return (sin(freqT) + 0.5 * sin(2.0 * freqT) + 0.25 * sin(3.0 * freqT)) / norm
        case .rectifiedSine:
            return 2.0 * abs(sin(freqT)) - 1.0
        case .noiseModulatedSine:
            let pseudoNoise = sin(t * 0.73) * 0.5 + sin(t * 1.31) * 0.3
            return sin(freqT + pseudoNoise * 5.0)
        case .bitcrushedSine:
            let steps = 8.0
            var v = floor(sin(freqT) * steps) / steps
            v *= steps / (steps - 1.0)
            return Swift.min(1.0, Swift.max(-1.0, v))
        case .moireOscillation:
            let freq2T = (freq * 1.05) * t + randOffset
            return 0.5 * sin(freqT) + 0.5 * sin(freq2T)
        }
    }

    private func posMod(_ a: Double, _ b: Double) -> Double {
        let r = a.truncatingRemainder(dividingBy: b)
        return r >= 0 ? r : r + b
    }
}

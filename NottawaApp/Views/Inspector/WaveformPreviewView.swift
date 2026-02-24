//
//  WaveformPreviewView.swift
//  NottawaApp
//
//  Live waveform visualization using SwiftUI Canvas + TimelineView.
//  Mathematically generates the waveform shape client-side, matching the C++
//  WaveformOscillator::tick() math. Displays a scrolling waveform with
//  min/max output horizontal lines.
//

import SwiftUI

struct WaveformPreviewView: View {
    let waveShape: WaveShape
    let frequency: Float
    let minOutput: Float
    let maxOutput: Float
    let paramMin: Float
    let paramMax: Float
    let enabled: Bool

    /// Random offset per instance (mirrors C++ randOffset).
    /// @State so it persists when the parent view rebuilds (e.g. sidebar refresh timer).
    @State private var randOffset: Double = Double.random(in: 0 ..< .pi * 2)

    private let height: CGFloat = 80

    var body: some View {
        if enabled {
            TimelineView(.animation(minimumInterval: 1.0 / 30.0)) { timeline in
                let phase = timeline.date.timeIntervalSinceReferenceDate
                WaveformCanvas(
                    waveShape: waveShape,
                    frequency: frequency,
                    minOutput: minOutput,
                    maxOutput: maxOutput,
                    paramMin: paramMin,
                    paramMax: paramMax,
                    phase: phase,
                    randOffset: randOffset
                )
            }
            .frame(height: height)
            .clipShape(RoundedRectangle(cornerRadius: 4))
        } else {
            RoundedRectangle(cornerRadius: 4)
                .fill(Color.black.opacity(0.15))
                .frame(height: height)
                .overlay(
                    Text("Oscillator Off")
                        .font(.caption2)
                        .foregroundStyle(.secondary)
                )
        }
    }
}

// MARK: - Canvas Drawing

/// Extracted Canvas for efficient redraw — TimelineView triggers redraws via changing `phase`.
private struct WaveformCanvas: View {
    let waveShape: WaveShape
    let frequency: Float
    let minOutput: Float
    let maxOutput: Float
    let paramMin: Float
    let paramMax: Float
    let phase: Double
    let randOffset: Double

    var body: some View {
        Canvas { context, size in
            let w = size.width
            let h = size.height

            // Background
            context.fill(
                Path(roundedRect: CGRect(origin: .zero, size: size), cornerRadius: 4),
                with: .color(.black.opacity(0.3))
            )

            let actualMin = Double(Swift.min(minOutput, maxOutput))
            let actualMax = Double(Swift.max(minOutput, maxOutput))
            let pMin = Double(paramMin)
            let pMax = Double(paramMax)

            let yRange = pMax - pMin
            guard yRange > 0 else { return }

            // Map value to Y pixel (top = max, bottom = min)
            func yPos(_ val: Double) -> CGFloat {
                CGFloat(1.0 - (val - pMin) / yRange) * h
            }

            // -- Min/Max horizontal lines --
            let isInBounds = actualMin >= pMin && actualMax <= pMax
            let lineColor: Color = isInBounds ? .green.opacity(0.5) : .red.opacity(0.5)

            let minY = yPos(actualMin)
            let maxY = yPos(actualMax)

            var minLine = Path()
            minLine.move(to: CGPoint(x: 0, y: minY))
            minLine.addLine(to: CGPoint(x: w, y: minY))
            context.stroke(minLine, with: .color(lineColor), style: StrokeStyle(lineWidth: 1, dash: [4, 3]))

            var maxLine = Path()
            maxLine.move(to: CGPoint(x: 0, y: maxY))
            maxLine.addLine(to: CGPoint(x: w, y: maxY))
            context.stroke(maxLine, with: .color(lineColor), style: StrokeStyle(lineWidth: 1, dash: [4, 3]))

            // Labels
            context.draw(
                Text(String(format: "%.1f", minOutput)).font(.system(size: 8)).foregroundStyle(lineColor),
                at: CGPoint(x: w - 18, y: minY + 7)
            )
            context.draw(
                Text(String(format: "%.1f", maxOutput)).font(.system(size: 8)).foregroundStyle(lineColor),
                at: CGPoint(x: w - 18, y: maxY - 7)
            )

            // -- Waveform line --
            let sampleCount = Int(w)
            guard sampleCount > 1 else { return }

            let timeWindow = 6.0
            let freq = Double(frequency)

            var waveformPath = Path()
            for i in 0 ..< sampleCount {
                let frac = Double(i) / Double(sampleCount - 1)
                let t = phase - timeWindow + frac * timeWindow
                let freqT = freq * t + randOffset

                let v = waveformValue(shape: waveShape, freqT: freqT, t: t, freq: freq)

                // Map [-1, 1] to [actualMin, actualMax]
                let normalized = (v + 1.0) / 2.0
                let output = actualMin + normalized * (actualMax - actualMin)
                let clamped = Swift.min(Swift.max(output, pMin), pMax)

                let x = CGFloat(frac) * w
                let y = yPos(clamped)

                if i == 0 {
                    waveformPath.move(to: CGPoint(x: x, y: y))
                } else {
                    waveformPath.addLine(to: CGPoint(x: x, y: y))
                }
            }

            context.stroke(
                waveformPath,
                with: .color(.cyan),
                style: StrokeStyle(lineWidth: 1.5, lineCap: .round, lineJoin: .round)
            )
        }
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
            // Normalize: actual range ~[-0.37, 2.72] → map center ~1.175, half-range ~1.55
            return Swift.min(1.0, Swift.max(-1.0, (raw - 1.175) / 1.55))

        case .harmonicStack:
            let norm = 1.0 + 0.5 + 0.25
            return (sin(freqT) + 0.5 * sin(2.0 * freqT) + 0.25 * sin(3.0 * freqT)) / norm

        case .rectifiedSine:
            return 2.0 * abs(sin(freqT)) - 1.0

        case .noiseModulatedSine:
            // Pseudo-noise (no Perlin in Swift) — sum of incommensurate sines
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

//
//  FrequencyColumnView.swift
//  NottawaApp
//
//  Frequency column: Bands/Bars/Waveform view mode with smoothing controls.
//  Graph sub-views read from AudioGraphData (15fps).
//  Control sub-views read from AudioControlData (3fps).
//

import SwiftUI

struct FrequencyColumnView: View {
    @Environment(ThemeManager.self) private var theme
    let graphs: AudioGraphData
    let controls: AudioControlData
    var graphHeight: CGFloat? = nil

    @State private var viewMode: FreqViewMode = .bands

    enum FreqViewMode: String, CaseIterable, Identifiable {
        case bands = "Bands"
        case bars = "Bars"
        case waveform = "Waveform"
        var id: String { rawValue }
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("Frequency")
                .font(.caption).fontWeight(.semibold).foregroundStyle(theme.colors.textSecondary)

            // Graph area — only reads from graphs (15fps)
            Group {
                switch viewMode {
                case .bands:
                    FreqBandsGraphView(data: graphs)
                case .bars:
                    FreqBarsGraphView(data: graphs)
                case .waveform:
                    FreqWaveformGraphView(data: graphs)
                }
            }
            .frame(height: graphHeight)
            .frame(maxHeight: graphHeight == nil ? .infinity : .none)

            // Controls — only reads from controls (3fps)
            DSPicker(
                selection: $viewMode,
                options: FreqViewMode.allCases.map { DSPickerOption(value: $0, label: $0.rawValue) },
                style: .segmented, size: .sm
            )

            FreqControlsView(data: controls)
        }
    }
}

// MARK: - Graph views (15fps)

struct FreqBandsGraphView: View {
    let data: AudioGraphData
    var body: some View {
        MultiLineScrollingGraphView(
            series: [
                (values: data.lowsValues, color: .purple),
                (values: data.midsValues, color: .yellow),
                (values: data.highsValues, color: .green),
            ]
        )
    }
}

struct FreqBarsGraphView: View {
    let data: AudioGraphData
    var body: some View {
        BarChartView(values: data.melSpectrum, barColor: .cyan)
    }
}

struct FreqWaveformGraphView: View {
    let data: AudioGraphData
    var body: some View {
        Canvas { context, size in
            let w = size.width
            let h = size.height
            let cornerRadius: CGFloat = 6

            // Dark background
            let bgRect = RoundedRectangle(cornerRadius: cornerRadius)
                .path(in: CGRect(origin: .zero, size: size))
            context.fill(bgRect, with: .color(Color(white: 0.08)))

            // Center line
            var centerPath = Path()
            centerPath.move(to: CGPoint(x: 0, y: h / 2))
            centerPath.addLine(to: CGPoint(x: w, y: h / 2))
            context.stroke(centerPath, with: .color(.white.opacity(0.12)), lineWidth: 0.5)

            let wave = data.waveform
            guard wave.count > 1 else { return }

            let stepX = w / CGFloat(wave.count - 1)

            // Build line path
            var linePath = Path()
            for i in 0..<wave.count {
                let x = CGFloat(i) * stepX
                let y = h / 2 - CGFloat(wave[i]) * (h / 2)
                let clamped = CGPoint(x: x, y: min(max(y, 0), h))
                if i == 0 {
                    linePath.move(to: clamped)
                } else {
                    linePath.addLine(to: clamped)
                }
            }

            // Gradient fill from line to center
            var fillAbove = Path()
            var fillBelow = Path()
            for i in 0..<wave.count {
                let x = CGFloat(i) * stepX
                let y = h / 2 - CGFloat(wave[i]) * (h / 2)
                let clampedY = min(max(y, 0), h)
                if i == 0 {
                    fillAbove.move(to: CGPoint(x: x, y: clampedY))
                    fillBelow.move(to: CGPoint(x: x, y: clampedY))
                } else {
                    fillAbove.addLine(to: CGPoint(x: x, y: clampedY))
                    fillBelow.addLine(to: CGPoint(x: x, y: clampedY))
                }
            }
            // Close fill paths to center line
            fillAbove.addLine(to: CGPoint(x: CGFloat(wave.count - 1) * stepX, y: h / 2))
            fillAbove.addLine(to: CGPoint(x: 0, y: h / 2))
            fillAbove.closeSubpath()

            context.fill(
                fillAbove,
                with: .linearGradient(
                    Gradient(colors: [Color.mint.opacity(0.25), Color.mint.opacity(0.0)]),
                    startPoint: CGPoint(x: w / 2, y: 0),
                    endPoint: CGPoint(x: w / 2, y: h / 2)
                )
            )

            fillBelow.addLine(to: CGPoint(x: CGFloat(wave.count - 1) * stepX, y: h / 2))
            fillBelow.addLine(to: CGPoint(x: 0, y: h / 2))
            fillBelow.closeSubpath()

            context.fill(
                fillBelow,
                with: .linearGradient(
                    Gradient(colors: [Color.mint.opacity(0.0), Color.mint.opacity(0.25)]),
                    startPoint: CGPoint(x: w / 2, y: h / 2),
                    endPoint: CGPoint(x: w / 2, y: h)
                )
            )

            // Line stroke
            context.stroke(linePath, with: .color(.mint), lineWidth: 2)

            // Subtle glow
            context.stroke(linePath, with: .color(.mint.opacity(0.3)), lineWidth: 5)
        }
        .drawingGroup()
        .clipShape(RoundedRectangle(cornerRadius: 6))
    }
}

// MARK: - Controls (3fps)

struct FreqControlsView: View {
    @Environment(ThemeManager.self) private var theme
    let data: AudioControlData
    private let engine = NottawaEngine.shared
    private var snapshot: ExtendedAudioAnalysisSnapshot { data.snapshot }

    var body: some View {
        HStack(spacing: 8) {
            DSPicker(
                selection: Binding(
                    get: { snapshot.smoothingMode },
                    set: { engine.setSmoothingMode($0.rawValue) }
                ),
                options: SmoothingMode.allCases.map { DSPickerOption(value: $0, label: $0.displayName) },
                style: .menu, size: .sm
            )
            .frame(width: 150)

            DSSlider(
                value: Binding(
                    get: { snapshot.frequencyRelease },
                    set: { engine.setFrequencyRelease($0) }
                ),
                range: 0.01...1.0,
                label: "Release",
                showValue: true
            )

            DSSlider(
                value: Binding(
                    get: { snapshot.frequencyScale },
                    set: { engine.setFrequencyScale($0) }
                ),
                range: 0.01...2.0,
                label: "Scale",
                showValue: true
            )
        }
    }
}

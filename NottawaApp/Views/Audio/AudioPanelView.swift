//
//  AudioPanelView.swift
//  NottawaApp
//
//  Container view for the audio analysis panel.
//
//  Performance architecture:
//  - AudioGraphData (@Observable, 15fps) — only Canvas views depend on it
//  - AudioControlData (@Observable, 3fps) — only sliders/pickers/buttons depend on it
//  - AudioPanelView passes both by reference without reading properties,
//    so it never re-evaluates on timer ticks.
//

import SwiftUI

// MARK: - Ring Buffer (fixed capacity, no allocation after init)

struct RingBuffer {
    private var storage: [Float]
    private(set) var writeIndex: Int = 0
    let capacity: Int

    init(capacity: Int) {
        self.capacity = capacity
        self.storage = Array(repeating: 0, count: capacity)
    }

    mutating func push(_ value: Float) {
        storage[writeIndex % capacity] = value
        writeIndex &+= 1
    }

    /// Ordered values (oldest → newest). Allocates once per call.
    func orderedValues() -> [Float] {
        if writeIndex < capacity {
            return Array(storage[0..<min(writeIndex, capacity)])
        }
        let start = writeIndex % capacity
        return Array(storage[start..<capacity]) + Array(storage[0..<start])
    }
}

// MARK: - Fast-updating graph data (15fps)

@Observable
final class AudioGraphData {
    private static let historySize = 120

    // Pre-computed arrays — computed once per tick, read by Canvas views
    var rmsValues: [Float] = []
    var beatFlash: CGFloat = 0  // 0..1, spikes to 1 on beat, smooth decay
    var bpm: Float = 120
    private var lastBeatCount: UInt32 = 0
    private var beatStartTime: TimeInterval = 0
    var lowsValues: [Float] = []
    var midsValues: [Float] = []
    var highsValues: [Float] = []
    var melSpectrum: [Float] = []
    var waveform: [Float] = []

    // Internal ring buffers (never read by views)
    private var rmsRing = RingBuffer(capacity: historySize)
    private var lowsRing = RingBuffer(capacity: historySize)
    private var midsRing = RingBuffer(capacity: historySize)
    private var highsRing = RingBuffer(capacity: historySize)

    private let engine = NottawaEngine.shared

    func poll() {
        let s = engine.extendedAudioAnalysis()

        rmsRing.push(s.rms)
        lowsRing.push(s.lows)
        midsRing.push(s.mids)
        highsRing.push(s.highs)

        // Beat flash: record wall-clock time on beat, compute smooth decay analytically
        if s.beatCount != lastBeatCount {
            lastBeatCount = s.beatCount
            beatStartTime = ProcessInfo.processInfo.systemUptime
        }
        let elapsed = ProcessInfo.processInfo.systemUptime - beatStartTime
        beatFlash = CGFloat(max(0, exp(-8.0 * elapsed))) // smooth ~350ms decay
        bpm = s.bpm

        // Compute once, read many
        rmsValues = rmsRing.orderedValues()
        lowsValues = lowsRing.orderedValues()
        midsValues = midsRing.orderedValues()
        highsValues = highsRing.orderedValues()
        melSpectrum = s.melSpectrum
        waveform = s.waveform
    }
}

// MARK: - Slow-updating control data (3fps)

@Observable
final class AudioControlData {
    var snapshot: ExtendedAudioAnalysisSnapshot = .zero
    var audioSources: [AudioSourceInfo] = []
    var selectedSourceId: String?
    var fileState = FileAudioState(
        volume: 1, isPaused: false, playbackPosition: 0,
        totalDuration: 0, isFileSource: false, selectedTrackIndex: -1
    )
    var tracks: [AudioTrackInfo] = []

    private let engine = NottawaEngine.shared

    func poll() {
        snapshot = engine.extendedAudioAnalysis()
        fileState = engine.fileAudioState()
        selectedSourceId = engine.selectedAudioSourceId
        audioSources = engine.allAudioSources()
        tracks = engine.sampleTracks()
    }
}

// MARK: - Container

struct AudioPanelView: View {
    // These are passed to children by reference — AudioPanelView's body
    // never reads their @Observable properties, so it never re-evaluates.
    @State private var graphs = AudioGraphData()
    @State private var controls = AudioControlData()

    private let fastTimer = Timer.publish(every: 1.0 / 15.0, on: .main, in: .common).autoconnect()
    private let slowTimer = Timer.publish(every: 1.0 / 3.0, on: .main, in: .common).autoconnect()

    var body: some View {
        VStack(spacing: 0) {
            Divider()

            AudioPanelTopBar(data: controls)

            Divider()

            HStack(spacing: 12) {
                // Loudness column
                VStack(alignment: .leading, spacing: 6) {
                    Text("Loudness")
                        .font(.caption).fontWeight(.semibold).foregroundStyle(.secondary)
                    LoudnessGraphView(data: graphs)
                        .frame(maxHeight: .infinity)
                    LoudnessControlsView(data: controls)
                }
                .frame(maxWidth: .infinity)

                Divider()

                // BPM column
                VStack(spacing: 6) {
                    ZStack {
                        BeatIndicatorView(data: graphs)
                        BpmOverlayView(data: controls)
                    }
                    .frame(maxHeight: .infinity)
                    BpmControlsView(data: controls)
                }
                .frame(maxWidth: .infinity)

                Divider()

                // Frequency column
                FrequencyColumnView(graphs: graphs, controls: controls)
                    .frame(minWidth: 300, maxWidth: .infinity)
            }
            .padding(.horizontal, 12)
            .padding(.vertical, 8)
            .frame(maxHeight: .infinity)
        }
        .background(.ultraThinMaterial)
        .onAppear { controls.poll() }
        .onReceive(fastTimer) { _ in graphs.poll() }
        .onReceive(slowTimer) { _ in controls.poll() }
    }
}

// MARK: - Graph-only views (re-render at 15fps)

struct LoudnessGraphView: View {
    let data: AudioGraphData
    var body: some View {
        ScrollingGraphView(values: data.rmsValues, color: .green)
    }
}

struct BeatIndicatorView: View {
    let data: AudioGraphData

    var body: some View {
        Canvas { context, size in
            let w = size.width
            let h = size.height
            let flash = data.beatFlash

            // Dark background
            let bgRect = RoundedRectangle(cornerRadius: 6)
                .path(in: CGRect(origin: .zero, size: size))
            context.fill(bgRect, with: .color(Color(white: 0.08)))

            // Subtle background pulse on beat
            if flash > 0.01 {
                context.fill(bgRect, with: .color(Color.orange.opacity(flash * 0.08)))
            }

            let center = CGPoint(x: w / 2, y: h / 2)

            // Outer glow (radiates from center on beat)
            if flash > 0.01 {
                let glowRadius = min(w, h) * 0.48 + flash * 20
                let glowRect = CGRect(
                    x: center.x - glowRadius,
                    y: center.y - glowRadius,
                    width: glowRadius * 2,
                    height: glowRadius * 2
                )
                context.fill(
                    Path(ellipseIn: glowRect),
                    with: .radialGradient(
                        Gradient(colors: [
                            Color.orange.opacity(flash * 0.5),
                            Color.orange.opacity(flash * 0.15),
                            Color.orange.opacity(0)
                        ]),
                        center: center,
                        startRadius: 0,
                        endRadius: glowRadius
                    )
                )
            }

            // Ring that pulses on beat
            let ringRadius = min(w, h) * 0.35
            let ringScale = 1.0 + flash * 0.08
            let r = ringRadius * ringScale
            let ringRect = CGRect(
                x: center.x - r, y: center.y - r,
                width: r * 2, height: r * 2
            )
            let ringPath = Path(ellipseIn: ringRect)
            context.stroke(
                ringPath,
                with: .color(Color.orange.opacity(0.25 + flash * 0.6)),
                lineWidth: 2.5 + flash * 2
            )

            // BPM number — large, centered, pulses with beat
            let bpmText = Text("\(Int(data.bpm))")
                .font(.system(size: min(w, h) * 0.3, weight: .bold, design: .rounded))
                .foregroundColor(.white.opacity(0.7 + flash * 0.3))
            context.draw(bpmText, at: CGPoint(x: center.x, y: center.y - 6))

            // "BPM" label below number
            let label = Text("BPM")
                .font(.system(size: 10, weight: .medium))
                .foregroundColor(.white.opacity(0.3 + flash * 0.2))
            context.draw(label, at: CGPoint(x: center.x, y: center.y + min(w, h) * 0.16))
        }
        .drawingGroup()
        .clipShape(RoundedRectangle(cornerRadius: 6))
    }
}

/// Lock/pause overlay on top of the beat indicator (3fps)
struct BpmOverlayView: View {
    let data: AudioControlData
    private let engine = NottawaEngine.shared
    private var snapshot: ExtendedAudioAnalysisSnapshot { data.snapshot }

    var body: some View {
        VStack {
            HStack {
                Button {
                    engine.setBpmLocked(!snapshot.bpmLocked)
                } label: {
                    Image(systemName: snapshot.bpmLocked ? "lock.fill" : "lock.open")
                        .font(.caption)
                        .foregroundStyle(.white.opacity(snapshot.bpmLocked ? 0.7 : 0.4))
                }
                .buttonStyle(.plain)
                .help(snapshot.bpmLocked ? "Unlock BPM" : "Lock BPM")
                Spacer()
                Button {
                    engine.setBpmEnabled(!snapshot.bpmEnabled)
                } label: {
                    Image(systemName: snapshot.bpmEnabled ? "pause.fill" : "play.fill")
                        .font(.caption)
                        .foregroundStyle(.white.opacity(0.4))
                }
                .buttonStyle(.plain)
                .help(snapshot.bpmEnabled ? "Pause BPM" : "Resume BPM")
            }
            .padding(8)
            Spacer()
        }
    }
}

// MARK: - Control-only views (re-render at 3fps)

struct LoudnessControlsView: View {
    let data: AudioControlData
    private let engine = NottawaEngine.shared

    var body: some View {
        HStack(spacing: 4) {
            Text("Release")
                .font(.caption2).foregroundStyle(.secondary)
            Slider(
                value: Binding(
                    get: { data.snapshot.loudnessRelease },
                    set: { engine.setLoudnessRelease($0) }
                ),
                in: 0.01...1.0
            )
        }
    }
}

struct BpmControlsView: View {
    let data: AudioControlData
    private let engine = NottawaEngine.shared
    private var snapshot: ExtendedAudioAnalysisSnapshot { data.snapshot }

    var body: some View {
        VStack(spacing: 4) {
            if !data.fileState.isFileSource {
                Picker("", selection: Binding(
                    get: { snapshot.bpmMode },
                    set: { engine.setBpmMode($0.rawValue) }
                )) {
                    ForEach(BpmMode.allCases) { mode in
                        Text(mode.displayName).tag(mode)
                    }
                }
                .labelsHidden()
                .pickerStyle(.segmented)
                .controlSize(.small)
            }

            if snapshot.bpmMode == .manual {
                HStack(spacing: 6) {
                    Button { engine.setBpmValue(snapshot.bpm - 1) } label: {
                        Image(systemName: "minus").frame(width: 12, height: 12)
                    }
                    .buttonStyle(.bordered).controlSize(.small)

                    Slider(
                        value: Binding(
                            get: { snapshot.bpm },
                            set: { engine.setBpmValue($0) }
                        ),
                        in: 1...300
                    )
                    .controlSize(.small)

                    Button { engine.setBpmValue(snapshot.bpm + 1) } label: {
                        Image(systemName: "plus").frame(width: 12, height: 12)
                    }
                    .buttonStyle(.bordered).controlSize(.small)

                    Button("Tap") { engine.tapBpm() }
                        .buttonStyle(.bordered).controlSize(.small)
                }
            }

            // Nudge
            HStack(spacing: 4) {
                Text("Nudge").font(.caption2).foregroundStyle(.secondary)
                Button { engine.nudgeBpm(-0.1) } label: {
                    Image(systemName: "minus").frame(width: 12, height: 12)
                }
                .buttonStyle(.bordered).controlSize(.small)

                Text(String(format: "%.2f", snapshot.bpmNudge))
                    .font(.caption2).monospacedDigit().frame(width: 36)

                Button { engine.nudgeBpm(0.1) } label: {
                    Image(systemName: "plus").frame(width: 12, height: 12)
                }
                .buttonStyle(.bordered).controlSize(.small)
                Spacer()
            }
        }
    }
}

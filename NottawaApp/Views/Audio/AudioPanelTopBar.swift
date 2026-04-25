//
//  AudioPanelTopBar.swift
//  NottawaApp
//
//  Top bar of the audio panel: source selector, start/stop, file controls.
//  Reads only from AudioControlData (3fps updates).
//

import SwiftUI

struct AudioPanelTopBar: View {
    @Environment(ThemeManager.self) private var theme
    let data: AudioControlData
    var onToggleAudio: (() -> Void)?

    private let engine = NottawaEngine.shared

    var body: some View {
        HStack(spacing: 12) {
            Button {
                engine.toggleAudioSource()
                onToggleAudio?()
            } label: {
                HStack(spacing: 4) {
                    Image(systemName: data.snapshot.audioActive ? "stop.fill" : "play.fill")
                    Text(data.snapshot.audioActive ? "Stop" : "Start")
                }
            }
            .buttonStyle(.plainHitArea)
            .padding(.horizontal, 8)
            .padding(.vertical, 4)
            .background(data.snapshot.audioActive ? Color.red.opacity(0.2) : Color.green.opacity(0.2))
            .foregroundStyle(data.snapshot.audioActive ? .red : .green)
            .clipShape(RoundedRectangle(cornerRadius: 6))

            DSPicker(
                selection: Binding(
                    get: { data.selectedSourceId ?? "" },
                    set: { engine.selectAudioSource(id: $0) }
                ),
                options: data.audioSources.map { DSPickerOption(value: $0.id, label: $0.name) },
                style: .menu, size: .sm
            )
            .frame(width: 180)

            if data.snapshot.audioActive {
                ListeningIndicator(sourceName: data.audioSources.first(where: { $0.id == data.selectedSourceId })?.name)
            } else {
                HStack(spacing: 6) {
                    Circle()
                        .fill(theme.colors.textTertiary)
                        .frame(width: 8, height: 8)
                    Text("Inactive")
                        .font(.caption2)
                        .fontWeight(.medium)
                        .foregroundStyle(theme.colors.textTertiary)
                }
                .padding(.horizontal, 8)
                .padding(.vertical, 4)
                .background(theme.colors.backgroundTertiary.opacity(0.5))
                .clipShape(Capsule())
            }

            if data.fileState.isFileSource {
                theme.colors.border.frame(width: 1, height: 20)

                if !data.tracks.isEmpty {
                    DSPicker(
                        selection: Binding(
                            get: { data.fileState.selectedTrackIndex },
                            set: { engine.selectSampleTrack($0) }
                        ),
                        options: data.tracks.map { DSPickerOption(value: $0.index, label: $0.name) },
                        style: .menu, size: .sm
                    )
                    .frame(width: 160)
                }

                Button {
                    engine.toggleFileAudioPause()
                } label: {
                    Image(systemName: data.fileState.isPaused ? "play.fill" : "pause.fill")
                }

                HStack(spacing: 4) {
                    Image(systemName: "speaker.fill")
                        .font(.caption).foregroundStyle(theme.colors.textSecondary)
                    DSSlider(
                        value: Binding(
                            get: { data.fileState.volume },
                            set: { engine.setFileAudioVolume($0) }
                        ),
                        range: 0...1
                    )
                    .frame(width: 80)
                }

                HStack(spacing: 4) {
                    Text(formatTime(data.fileState.playbackPosition * data.fileState.totalDuration))
                        .font(.caption).monospacedDigit().foregroundStyle(theme.colors.textSecondary)
                    DSSlider(
                        value: Binding(
                            get: { data.fileState.playbackPosition },
                            set: { engine.setFileAudioPosition($0) }
                        ),
                        range: 0...1
                    )
                    .frame(width: 120)
                    Text(formatTime(data.fileState.totalDuration))
                        .font(.caption).monospacedDigit().foregroundStyle(theme.colors.textSecondary)
                }
            }

            Spacer()
        }
        .padding(.horizontal, 12)
        .padding(.vertical, 6)
    }

    private func formatTime(_ seconds: Float) -> String {
        guard seconds.isFinite else { return "0:00" }
        let totalSec = Int(max(0, seconds))
        return String(format: "%d:%02d", totalSec / 60, totalSec % 60)
    }
}

// MARK: - Listening Indicator

private struct ListeningIndicator: View {
    @Environment(ThemeManager.self) private var theme
    let sourceName: String?
    @State private var pulse = false

    var body: some View {
        HStack(spacing: 6) {
            Circle()
                .fill(Color.red)
                .frame(width: 8, height: 8)
                .opacity(pulse ? 1.0 : 0.4)
                .animation(.easeInOut(duration: 0.8).repeatForever(autoreverses: true), value: pulse)
                .onAppear { pulse = true }

            Text("Listening")
                .font(.caption2)
                .fontWeight(.medium)
                .foregroundStyle(theme.colors.textSecondary)

            if let name = sourceName {
                Text("via \(name)")
                    .font(.caption2)
                    .foregroundStyle(theme.colors.textTertiary)
            }
        }
        .padding(.horizontal, 8)
        .padding(.vertical, 4)
        .background(Color.red.opacity(0.08))
        .clipShape(Capsule())
    }
}

//
//  FileSourceControlsView.swift
//  NottawaApp
//
//  Playback controls for FileSource and LibrarySource nodes:
//  play/pause, position scrubber, volume, speed.
//

import SwiftUI

struct FileSourceControlsView: View {
    @Environment(ThemeManager.self) private var theme
    let state: FileSourceState
    let sourceId: String

    private let engine = NottawaEngine.shared

    var body: some View {
        VStack(alignment: .leading, spacing: 10) {
            // Play/Pause + Position scrubber
            HStack(spacing: 8) {
                Button {
                    engine.setFileSourcePlaying(sourceId: sourceId, playing: !state.isPlaying)
                } label: {
                    Image(systemName: state.isPlaying ? "pause.fill" : "play.fill")
                        .frame(width: 20, height: 20)
                }
                .buttonStyle(.plainHitArea)

                VStack(alignment: .leading, spacing: 2) {
                    DSSlider(
                        value: Binding(
                            get: { state.position },
                            set: { engine.setFileSourcePosition(sourceId: sourceId, position: $0) }
                        ),
                        range: 0...1
                    )

                    HStack {
                        Text(formatTime(state.position * state.duration))
                            .font(.caption2)
                            .foregroundStyle(theme.colors.textSecondary)
                            .monospacedDigit()
                        Spacer()
                        Text(formatTime(state.duration))
                            .font(.caption2)
                            .foregroundStyle(theme.colors.textSecondary)
                            .monospacedDigit()
                    }
                }
            }

            // Volume
            HStack(spacing: 8) {
                Button {
                    engine.setFileSourceMute(sourceId: sourceId, mute: !state.isMuted)
                } label: {
                    Image(systemName: volumeIcon)
                        .frame(width: 20, height: 20)
                }
                .buttonStyle(.plainHitArea)

                DSSlider(
                    value: Binding(
                        get: { state.volume },
                        set: { engine.setFileSourceVolume(sourceId: sourceId, volume: $0) }
                    ),
                    range: 0...1
                )
                .disabled(state.isMuted)
                .opacity(state.isMuted ? 0.5 : 1.0)
            }

            // Speed
            HStack(spacing: 8) {
                Text(String(format: "%.1fx", state.speed))
                    .font(.caption)
                    .foregroundStyle(theme.colors.textSecondary)
                    .monospacedDigit()
                    .frame(width: 32, alignment: .center)

                DSSlider(
                    value: Binding(
                        get: { state.speed },
                        set: { engine.setFileSourceSpeed(sourceId: sourceId, speed: $0) }
                    ),
                    range: 0...2
                )
            }
        }
        .padding(.horizontal, 16)
        .padding(.vertical, 8)
    }

    private var volumeIcon: String {
        if state.isMuted { return "speaker.slash.fill" }
        if state.volume > 0.5 { return "speaker.wave.3.fill" }
        if state.volume > 0 { return "speaker.wave.1.fill" }
        return "speaker.fill"
    }

    private func formatTime(_ seconds: Float) -> String {
        guard seconds.isFinite else { return "0:00" }
        let totalSeconds = max(0, Int(seconds))
        let mins = totalSeconds / 60
        let secs = totalSeconds % 60
        return String(format: "%d:%02d", mins, secs)
    }
}

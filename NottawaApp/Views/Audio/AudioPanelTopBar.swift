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

    private let engine = NottawaEngine.shared

    var body: some View {
        HStack(spacing: 12) {
            Button {
                engine.toggleAudioSource()
            } label: {
                HStack(spacing: 4) {
                    Image(systemName: data.snapshot.audioActive ? "stop.fill" : "play.fill")
                    Text(data.snapshot.audioActive ? "Stop" : "Start")
                }
            }
            .buttonStyle(.plain)
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
        let totalSec = Int(max(0, seconds))
        return String(format: "%d:%02d", totalSec / 60, totalSec % 60)
    }
}

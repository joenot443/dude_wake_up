//
//  AudioPanelTopBar.swift
//  NottawaApp
//
//  Top bar of the audio panel: source selector, start/stop, file controls.
//  Reads only from AudioControlData (3fps updates).
//

import SwiftUI

struct AudioPanelTopBar: View {
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
            .buttonStyle(.bordered)
            .tint(data.snapshot.audioActive ? .red : .green)

            Picker("Source", selection: Binding(
                get: { data.selectedSourceId ?? "" },
                set: { engine.selectAudioSource(id: $0) }
            )) {
                ForEach(data.audioSources) { source in
                    Text(source.name).tag(source.id)
                }
            }
            .frame(width: 180)

            if data.fileState.isFileSource {
                Divider().frame(height: 20)

                if !data.tracks.isEmpty {
                    Picker("Track", selection: Binding(
                        get: { data.fileState.selectedTrackIndex },
                        set: { engine.selectSampleTrack($0) }
                    )) {
                        ForEach(data.tracks) { track in
                            Text(track.name).tag(track.index)
                        }
                    }
                    .frame(width: 160)
                }

                Button {
                    engine.toggleFileAudioPause()
                } label: {
                    Image(systemName: data.fileState.isPaused ? "play.fill" : "pause.fill")
                }

                HStack(spacing: 4) {
                    Image(systemName: "speaker.fill")
                        .font(.caption).foregroundStyle(.secondary)
                    Slider(
                        value: Binding(
                            get: { data.fileState.volume },
                            set: { engine.setFileAudioVolume($0) }
                        ),
                        in: 0...1
                    )
                    .frame(width: 80)
                }

                HStack(spacing: 4) {
                    Text(formatTime(data.fileState.playbackPosition * data.fileState.totalDuration))
                        .font(.caption).monospacedDigit().foregroundStyle(.secondary)
                    Slider(
                        value: Binding(
                            get: { data.fileState.playbackPosition },
                            set: { engine.setFileAudioPosition($0) }
                        ),
                        in: 0...1
                    )
                    .frame(width: 120)
                    Text(formatTime(data.fileState.totalDuration))
                        .font(.caption).monospacedDigit().foregroundStyle(.secondary)
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

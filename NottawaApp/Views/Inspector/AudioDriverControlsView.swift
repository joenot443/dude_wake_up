//
//  AudioDriverControlsView.swift
//  NottawaApp
//
//  Inline audio driver controls for a parameter: driver picker,
//  shift/scale sliders, remove button.
//

import SwiftUI

struct AudioDriverControlsView: View {
    let param: ParameterInfo

    @State private var shift: Float
    @State private var scale: Float
    @State private var availableDrivers: [AudioDriverOption] = []

    init(param: ParameterInfo) {
        self.param = param
        self._shift = State(initialValue: param.driverShift)
        self._scale = State(initialValue: param.driverScale)
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            if param.hasDriver, let driverName = param.driverName {
                // Active driver header
                HStack {
                    Image(systemName: "waveform.path")
                        .font(.caption2)
                        .foregroundStyle(.orange)
                    Text(driverName)
                        .font(.caption)
                        .fontWeight(.medium)
                    Spacer()
                    Button {
                        NottawaEngine.shared.removeParameterDriver(paramId: param.id)
                    } label: {
                        Image(systemName: "xmark.circle.fill")
                            .font(.caption)
                            .foregroundStyle(.secondary)
                    }
                    .buttonStyle(.plain)
                }

                // Shift slider
                VStack(alignment: .leading, spacing: 1) {
                    HStack {
                        Text("Shift")
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                        Spacer()
                        Text(String(format: "%.2f", shift))
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                            .monospacedDigit()
                    }
                    Slider(value: $shift, in: -1...1)
                        .controlSize(.small)
                        .onChange(of: shift) { _, newValue in
                            NottawaEngine.shared.setParameterDriverShift(paramId: param.id, shift: newValue)
                        }
                }

                // Scale slider
                VStack(alignment: .leading, spacing: 1) {
                    HStack {
                        Text("Scale")
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                        Spacer()
                        Text(String(format: "%.2f", scale))
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                            .monospacedDigit()
                    }
                    Slider(value: $scale, in: 0...2)
                        .controlSize(.small)
                        .onChange(of: scale) { _, newValue in
                            NottawaEngine.shared.setParameterDriverScale(paramId: param.id, scale: newValue)
                        }
                }
            } else {
                // No driver — show picker
                HStack {
                    Text("Audio Driver")
                        .font(.caption)
                    Spacer()
                    if availableDrivers.isEmpty {
                        Text("No audio source")
                            .font(.caption2)
                            .foregroundStyle(.secondary)
                    } else {
                        Menu {
                            ForEach(availableDrivers) { driver in
                                Button(driver.name) {
                                    NottawaEngine.shared.setParameterDriver(
                                        paramId: param.id,
                                        audioParamName: driver.name
                                    )
                                }
                            }
                        } label: {
                            Label("Assign", systemImage: "plus.circle")
                                .font(.caption)
                        }
                        .menuStyle(.borderlessButton)
                        .fixedSize()
                    }
                }
            }
        }
        .padding(8)
        .background(Color.orange.opacity(0.06))
        .clipShape(RoundedRectangle(cornerRadius: 6))
        .accessibilityIdentifier("driver-controls-\(param.id)")
        .onAppear {
            availableDrivers = NottawaEngine.shared.availableAudioDrivers()
        }
        .onChange(of: param.driverShift) { _, newValue in
            shift = newValue
        }
        .onChange(of: param.driverScale) { _, newValue in
            scale = newValue
        }
    }
}

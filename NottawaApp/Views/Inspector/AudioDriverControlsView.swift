//
//  AudioDriverControlsView.swift
//  NottawaApp
//
//  Inline audio driver controls for a parameter: driver picker,
//  shift/scale sliders, remove button.
//

import SwiftUI

struct AudioDriverControlsView: View {
    @Environment(ThemeManager.self) private var theme
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
                            .foregroundStyle(theme.colors.textSecondary)
                    }
                    .buttonStyle(.plain)
                }

                // Shift slider
                DSSlider(value: $shift, range: -1...1, label: "Shift", showValue: true)
                    .onChange(of: shift) { _, newValue in
                        NottawaEngine.shared.setParameterDriverShift(paramId: param.id, shift: newValue)
                    }

                // Scale slider
                DSSlider(value: $scale, range: 0...2, label: "Scale", showValue: true)
                    .onChange(of: scale) { _, newValue in
                        NottawaEngine.shared.setParameterDriverScale(paramId: param.id, scale: newValue)
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
                            .foregroundStyle(theme.colors.textSecondary)
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

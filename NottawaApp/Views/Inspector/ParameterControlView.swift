//
//  ParameterControlView.swift
//  NottawaApp
//
//  Renders a single parameter control based on its type.
//

import SwiftUI

struct ParameterControlView: View {
    let param: ParameterInfo

    @Environment(NodeEditorViewModel.self) private var viewModel

    @State private var sliderValue: Float
    @State private var intValue: Int
    @State private var boolValue: Bool
    @State private var colorValue: Color
    @State private var showOscillator = false
    @State private var showAudioDriver = false

    init(param: ParameterInfo) {
        self.param = param
        self._sliderValue = State(initialValue: param.value)
        self._intValue = State(initialValue: param.intValue)
        self._boolValue = State(initialValue: param.boolValue)
        self._colorValue = State(initialValue: Color(
            red: Double(param.color.r),
            green: Double(param.color.g),
            blue: Double(param.color.b),
            opacity: Double(param.color.a)
        ))
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            switch param.parameterType {
            case .hidden:
                EmptyView()

            case .bool_:
                HStack {
                    Text(param.name)
                        .font(.caption)
                    Spacer()
                    Toggle("", isOn: $boolValue)
                        .labelsHidden()
                        .onChange(of: boolValue) { _, newValue in
                            NottawaEngine.shared.setParameterBool(paramId: param.id, value: newValue)
                        }
                    favoriteButton
                }

            case .int_:
                if !param.options.isEmpty {
                    // Picker for int params with named options (e.g. font selector)
                    VStack(alignment: .leading, spacing: 2) {
                        HStack {
                            Text(param.name)
                                .font(.caption)
                            Spacer()
                            favoriteButton
                        }
                        Picker("", selection: $intValue) {
                            ForEach(0..<param.options.count, id: \.self) { idx in
                                Text(param.options[idx]).tag(idx)
                            }
                        }
                        .labelsHidden()
                        .onChange(of: intValue) { _, newValue in
                            NottawaEngine.shared.setParameterInt(paramId: param.id, value: newValue)
                        }
                    }
                } else {
                    VStack(alignment: .leading, spacing: 2) {
                        HStack {
                            Text(param.name)
                                .font(.caption)
                            Spacer()
                            Text("\(intValue)")
                                .font(.caption)
                                .foregroundStyle(.secondary)
                            oscillatorDriverButtons
                            favoriteButton
                        }
                        Slider(
                            value: Binding(
                                get: { Float(intValue) },
                                set: { intValue = Int($0) }
                            ),
                            in: param.minValue...param.maxValue,
                            step: 1
                        )
                        .disabled(param.oscillatorEnabled || param.hasDriver)
                        .opacity(param.oscillatorEnabled || param.hasDriver ? 0.4 : 1.0)
                        .onChange(of: intValue) { _, newValue in
                            NottawaEngine.shared.setParameterInt(paramId: param.id, value: newValue)
                        }
                    }
                    expandableSections
                }

            case .color:
                HStack {
                    Text(param.name)
                        .font(.caption)
                    Spacer()
                    ColorPicker("", selection: $colorValue, supportsOpacity: true)
                        .labelsHidden()
                        .onChange(of: colorValue) { _, newColor in
                            if let components = newColor.cgColor?.components {
                                let r = Float(components.count > 0 ? components[0] : 0)
                                let g = Float(components.count > 1 ? components[1] : 0)
                                let b = Float(components.count > 2 ? components[2] : 0)
                                let a = Float(components.count > 3 ? components[3] : 1)
                                NottawaEngine.shared.setParameterColor(paramId: param.id, r: r, g: g, b: b, a: a)
                            }
                        }
                    favoriteButton
                }

            case .standard:
                VStack(alignment: .leading, spacing: 2) {
                    HStack {
                        Text(param.name)
                            .font(.caption)
                        Spacer()
                        Text(String(format: "%.2f", sliderValue))
                            .font(.caption)
                            .foregroundStyle(.secondary)
                            .monospacedDigit()
                        oscillatorDriverButtons
                        favoriteButton
                    }
                    Slider(value: $sliderValue, in: param.minValue...param.maxValue)
                        .disabled(param.oscillatorEnabled || param.hasDriver)
                        .opacity(param.oscillatorEnabled || param.hasDriver ? 0.4 : 1.0)
                        .onChange(of: sliderValue) { _, newValue in
                            NottawaEngine.shared.setParameterValue(paramId: param.id, value: newValue)
                        }
                }
                expandableSections
            }
        }
        // Sync @State from param refresh (oscillator/driver drive value externally)
        .onChange(of: param.value) { _, newValue in
            if abs(sliderValue - newValue) > 0.001 {
                sliderValue = newValue
            }
        }
        .onChange(of: param.intValue) { _, newValue in
            if intValue != newValue {
                intValue = newValue
            }
        }
        .onChange(of: param.boolValue) { _, newValue in
            if boolValue != newValue {
                boolValue = newValue
            }
        }
    }

    // MARK: - Oscillator & Driver Toggle Buttons

    @ViewBuilder
    private var oscillatorDriverButtons: some View {
        if param.hasOscillator {
            Button {
                withAnimation(.easeInOut(duration: 0.2)) {
                    showOscillator.toggle()
                    if showOscillator { showAudioDriver = false }
                }
            } label: {
                Image(systemName: param.oscillatorEnabled ? "waveform.circle.fill" : "waveform.circle")
                    .font(.caption2)
                    .foregroundStyle(param.oscillatorEnabled ? .cyan : .secondary)
            }
            .buttonStyle(.plain)
            .accessibilityIdentifier("osc-toggle-\(param.id)")
        }

        Button {
            withAnimation(.easeInOut(duration: 0.2)) {
                showAudioDriver.toggle()
                if showAudioDriver { showOscillator = false }
            }
        } label: {
            Image(systemName: param.hasDriver ? "music.note.list" : "music.note")
                .font(.caption2)
                .foregroundStyle(param.hasDriver ? .orange : .secondary)
        }
        .buttonStyle(.plain)
        .accessibilityIdentifier("driver-toggle-\(param.id)")
    }

    // MARK: - Expandable Sections

    @ViewBuilder
    private var expandableSections: some View {
        if showOscillator && param.hasOscillator {
            OscillatorControlsView(param: param)
                .transition(.opacity.combined(with: .move(edge: .top)))
        }

        if showAudioDriver {
            AudioDriverControlsView(param: param)
                .transition(.opacity.combined(with: .move(edge: .top)))
        }
    }

    // MARK: - Favorite Button

    private var favoriteButton: some View {
        Button {
            NottawaEngine.shared.toggleParameterFavorite(paramId: param.id)
            viewModel.refreshInspector()
        } label: {
            Image(systemName: param.isFavorited ? "star.fill" : "star")
                .font(.caption2)
                .foregroundStyle(param.isFavorited ? .yellow : .secondary)
        }
        .buttonStyle(.plain)
    }
}

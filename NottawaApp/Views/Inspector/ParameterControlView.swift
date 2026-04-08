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
    @Environment(ThemeManager.self) private var theme

    @State private var sliderValue: Float
    @State private var intValue: Int
    @State private var boolValue: Bool
    @State private var colorValue: Color
    @State private var showOscillator = false
    @State private var showAudioDriver = false
    @State private var showMidi = false
    @State private var isExternallyDriven: Bool
    @State private var oscEnabled: Bool
    @State private var driverActive: Bool
    @State private var midiActive: Bool

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
        self._isExternallyDriven = State(initialValue: param.oscillatorEnabled || param.hasDriver)
        self._oscEnabled = State(initialValue: param.oscillatorEnabled)
        self._driverActive = State(initialValue: param.hasDriver)
        self._midiActive = State(initialValue: param.hasMidiBinding)
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            switch param.parameterType {
            case .hidden:
                EmptyView()

            case .bool_:
                HStack {
                    DSToggle(isOn: $boolValue, label: param.name, style: .switch, size: .sm)
                        .onChange(of: boolValue) { _, newValue in
                            NottawaEngine.shared.setParameterBool(paramId: param.id, value: newValue)
                            // Refresh graph state (e.g. aux output pin visibility)
                            viewModel.refresh()
                        }
                    Spacer()
                    resetButton
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
                            resetButton
                            favoriteButton
                        }
                        DSPicker(
                            selection: $intValue,
                            options: param.options.enumerated().map { DSPickerOption(value: $0.offset, label: $0.element) },
                            style: .menu, size: .sm
                        )
                        .onChange(of: intValue) { _, newValue in
                            NottawaEngine.shared.setParameterInt(paramId: param.id, value: newValue)
                        }
                    }
                } else {
                    IntSliderControl(
                        param: param,
                        intValue: $intValue,
                        isExternallyDriven: isExternallyDriven,
                        oscillatorDriverButtons: oscillatorDriverButtons,
                        resetButton: resetButton,
                        favoriteButton: favoriteButton
                    )
                    .onChange(of: intValue) { _, newValue in
                        NottawaEngine.shared.setParameterInt(paramId: param.id, value: newValue)
                    }
                    expandableSections
                }

            case .color:
                HStack {
                    Text(param.name)
                        .font(.caption)
                    Spacer()
                    DSColorPicker(color: $colorValue) { r, g, b, a in
                        NottawaEngine.shared.setParameterColor(
                            paramId: param.id, r: r, g: g, b: b, a: a
                        )
                    }
                    resetButton
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
                            .foregroundStyle(theme.colors.textSecondary)
                            .monospacedDigit()
                        oscillatorDriverButtons
                        resetButton
                        favoriteButton
                    }
                    DSSlider(value: $sliderValue, range: param.minValue...param.maxValue)
                        .disabled(isExternallyDriven)
                        .opacity(isExternallyDriven ? 0.4 : 1.0)
                        .onChange(of: sliderValue) { _, newValue in
                            NottawaEngine.shared.setParameterValue(paramId: param.id, value: newValue)
                        }
                }
                expandableSections
            }
        }
        .onChange(of: oscEnabled) { _, newValue in
            isExternallyDriven = newValue || driverActive
        }
        // Sync @State from fresh engine values (ParameterInfo.== is id-only,
        // so SwiftUI never detects value changes — we poll via notification).
        .onReceive(NotificationCenter.default.publisher(for: .parameterRefresh)) { _ in
            guard let fresh = viewModel.inspectorParameters.first(where: { $0.id == param.id }) else { return }
            let driven = fresh.oscillatorEnabled || fresh.hasDriver
            if driven != isExternallyDriven { isExternallyDriven = driven }
            if fresh.oscillatorEnabled != oscEnabled { oscEnabled = fresh.oscillatorEnabled }
            if fresh.hasDriver != driverActive { driverActive = fresh.hasDriver }
            if fresh.hasMidiBinding != midiActive { midiActive = fresh.hasMidiBinding }
            if abs(sliderValue - fresh.value) > 0.001 {
                sliderValue = fresh.value
            }
            if fresh.intValue != intValue {
                intValue = fresh.intValue
            }
            if fresh.boolValue != boolValue {
                boolValue = fresh.boolValue
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
                    if showOscillator {
                        showAudioDriver = false
                        if !oscEnabled {
                            oscEnabled = true
                            isExternallyDriven = true
                            NottawaEngine.shared.setOscillatorEnabled(paramId: param.id, enabled: true)
                        }
                    }
                }
            } label: {
                Image(systemName: oscEnabled ? "waveform.circle.fill" : "waveform.circle")
                    .font(.caption2)
                    .foregroundStyle(oscEnabled ? theme.colors.accent : theme.colors.textSecondary)
            }
            .buttonStyle(.plainHitArea)
            .accessibilityIdentifier("osc-toggle-\(param.id)")
        }

        Button {
            withAnimation(.easeInOut(duration: 0.2)) {
                showAudioDriver.toggle()
                if showAudioDriver { showOscillator = false; showMidi = false }
            }
        } label: {
            Image(systemName: driverActive ? "music.note.list" : "music.note")
                .font(.caption2)
                .foregroundStyle(driverActive ? theme.colors.warning : theme.colors.textSecondary)
        }
        .buttonStyle(.plainHitArea)
        .accessibilityIdentifier("driver-toggle-\(param.id)")

        Button {
            withAnimation(.easeInOut(duration: 0.2)) {
                showMidi.toggle()
                if showMidi { showOscillator = false; showAudioDriver = false }
            }
        } label: {
            Image(systemName: midiActive ? "pianokeys.inverse" : "pianokeys")
                .font(.caption2)
                .foregroundStyle(midiActive ? .purple : theme.colors.textSecondary)
        }
        .buttonStyle(.plainHitArea)
        .accessibilityIdentifier("midi-toggle-\(param.id)")
    }

    // MARK: - Expandable Sections

    @ViewBuilder
    private var expandableSections: some View {
        if showOscillator && param.hasOscillator {
            OscillatorControlsView(param: param, enabled: $oscEnabled)
                .transition(.opacity.combined(with: .move(edge: .top)))
        }

        if showAudioDriver || driverActive {
            AudioDriverControlsView(param: param)
                .transition(.opacity.combined(with: .move(edge: .top)))
        }

        if showMidi || midiActive {
            MidiControlsView(param: param)
                .transition(.opacity.combined(with: .move(edge: .top)))
        }
    }

    // MARK: - Reset Button

    private var resetButton: some View {
        Button {
            NottawaEngine.shared.resetParameter(paramId: param.id)
            viewModel.refreshInspector()
        } label: {
            Image(systemName: "arrow.counterclockwise")
                .font(.caption2)
                .foregroundStyle(theme.colors.textSecondary)
        }
        .buttonStyle(.plainHitArea)
        .help("Reset to default")
    }

    // MARK: - Favorite Button

    private var favoriteButton: some View {
        Button {
            NottawaEngine.shared.toggleParameterFavorite(paramId: param.id)
            viewModel.refreshInspector()
        } label: {
            Image(systemName: param.isFavorited ? "star.fill" : "star")
                .font(.caption2)
                .foregroundStyle(param.isFavorited ? theme.colors.warning : theme.colors.textSecondary)
        }
        .buttonStyle(.plainHitArea)
    }
}

// MARK: - Integer Slider Control

/// A stepped slider with -/+ buttons for integer parameters.
struct IntSliderControl<Buttons: View, Rst: View, Fav: View>: View {
    @Environment(ThemeManager.self) private var theme
    let param: ParameterInfo
    @Binding var intValue: Int
    let isExternallyDriven: Bool
    let oscillatorDriverButtons: Buttons
    let resetButton: Rst
    let favoriteButton: Fav

    private let minInt: Int
    private let maxInt: Int

    init(param: ParameterInfo, intValue: Binding<Int>, isExternallyDriven: Bool,
         oscillatorDriverButtons: Buttons, resetButton: Rst, favoriteButton: Fav) {
        self.param = param
        self._intValue = intValue
        self.isExternallyDriven = isExternallyDriven
        self.oscillatorDriverButtons = oscillatorDriverButtons
        self.resetButton = resetButton
        self.favoriteButton = favoriteButton
        self.minInt = Int(param.minValue)
        self.maxInt = Int(param.maxValue)
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 2) {
            HStack {
                Text(param.name)
                    .font(.caption)
                Spacer()
                oscillatorDriverButtons
                resetButton
                favoriteButton
            }
            HStack(spacing: 6) {
                Button {
                    if intValue > minInt { intValue -= 1 }
                } label: {
                    Image(systemName: "minus")
                        .font(.caption2.weight(.bold))
                        .frame(width: 20, height: 20)
                        .background(theme.colors.surface, in: RoundedRectangle(cornerRadius: 4))
                }
                .buttonStyle(.plainHitArea)
                .disabled(intValue <= minInt)

                DSSlider(
                    value: Binding(
                        get: { Float(intValue) },
                        set: { intValue = Int($0.rounded()) }
                    ),
                    range: Float(minInt)...Float(maxInt),
                    step: 1
                )
                .disabled(isExternallyDriven)
                .opacity(isExternallyDriven ? 0.4 : 1.0)

                Button {
                    if intValue < maxInt { intValue += 1 }
                } label: {
                    Image(systemName: "plus")
                        .font(.caption2.weight(.bold))
                        .frame(width: 20, height: 20)
                        .background(theme.colors.surface, in: RoundedRectangle(cornerRadius: 4))
                }
                .buttonStyle(.plainHitArea)
                .disabled(intValue >= maxInt)

                Text("\(intValue)")
                    .font(.caption.monospacedDigit())
                    .foregroundStyle(theme.colors.textSecondary)
                    .frame(minWidth: 28, alignment: .trailing)
            }
        }
    }
}

// MARK: - Parameter Refresh Notification

extension Notification.Name {
    static let parameterRefresh = Notification.Name("parameterRefresh")
}

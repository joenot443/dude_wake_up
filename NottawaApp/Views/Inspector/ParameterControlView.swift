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
    @Environment(HelpGuideViewModel.self) private var helpGuide

    @State private var sliderValue: Float
    @State private var intValue: Int
    @State private var boolValue: Bool
    @State private var colorValue: Color
    @State private var showOscillator = false
    @State private var showAudioDriver: Bool
    @State private var showMidi: Bool
    @State private var isExternallyDriven: Bool
    @State private var oscEnabled: Bool
    @State private var driverActive: Bool
    @State private var midiActive: Bool
    @State private var stringValue: String
    @State private var isEditingValue = false
    @State private var editText = ""
    @FocusState private var valueFieldFocused: Bool

    private var isSelected: Bool {
        viewModel.selectedParameterId == param.id
    }

    private func selectThisParameter() {
        viewModel.selectedParameterId = param.id
    }

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
        // Start the audio / MIDI panels open when the param is already
        // bound so the toggle button has something to actually toggle —
        // otherwise the panel was always forced visible by `|| midiActive`
        // and clicking the button did nothing visible.
        self._showAudioDriver = State(initialValue: param.hasDriver)
        self._showMidi = State(initialValue: param.hasMidiBinding)
        self._stringValue = State(initialValue: param.stringValue)
    }

    var body: some View {
        if param.parameterType == .hidden {
            EmptyView()
        } else {
            selectableContainer {
                contentStack
            }
            .background(blendModeAnchorReporter)
            .onChange(of: oscEnabled) { _, newValue in
                isExternallyDriven = newValue || driverActive
            }
            .onChange(of: isEditingValue) { _, newValue in
                if newValue {
                    valueFieldFocused = true
                }
            }
            .onReceive(NotificationCenter.default.publisher(for: .parameterBeginEdit)) { note in
                guard let info = note.userInfo,
                      let targetId = info["paramId"] as? String,
                      targetId == param.id,
                      param.parameterType == .standard else { return }
                let initial = (info["initialText"] as? String) ?? ""
                editText = initial
                isEditingValue = true
            }
            .onReceive(NotificationCenter.default.publisher(for: .parameterStepAnimated)) { note in
                guard let info = note.userInfo,
                      let targetId = info["paramId"] as? String,
                      targetId == param.id else { return }
                // UI-only smoothing: engine value has already jumped; we just
                // glide the thumb from the prior state to the new target.
                withAnimation(.easeOut(duration: 0.18)) {
                    if let v = info["value"] as? Float { sliderValue = v }
                    if let v = info["intValue"] as? Int { intValue = v }
                    if let v = info["boolValue"] as? Bool { boolValue = v }
                }
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
    }

    /// During inspector-attached help steps, report this row's global frame when
    /// it's the control the current step targets, so the help overlay can point
    /// its arrow directly at it.
    @ViewBuilder
    private var blendModeAnchorReporter: some View {
        if param.name == helpGuide.inspectorTargetParamName {
            GeometryReader { proxy in
                Color.clear.preference(
                    key: InspectorTargetFrameKey.self,
                    value: proxy.frame(in: .global)
                )
            }
        }
    }

    @ViewBuilder
    private func selectableContainer<Content: View>(@ViewBuilder _ content: () -> Content) -> some View {
        content()
            .padding(.horizontal, 6)
            .padding(.vertical, 4)
            .background(
                RoundedRectangle(cornerRadius: 6)
                    .fill(isSelected ? theme.colors.accent.opacity(0.08) : Color.clear)
            )
            .overlay(
                RoundedRectangle(cornerRadius: 6)
                    .strokeBorder(isSelected ? theme.colors.accent.opacity(0.6) : Color.clear, lineWidth: 1)
            )
            .contentShape(Rectangle())
            .simultaneousGesture(
                TapGesture().onEnded {
                    selectThisParameter()
                }
            )
    }

    @ViewBuilder
    private var contentStack: some View {
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

            case .text:
                VStack(alignment: .leading, spacing: 2) {
                    Text(param.name)
                        .font(.caption)
                    TextField(param.name, text: $stringValue)
                        .textFieldStyle(.roundedBorder)
                        .font(.caption)
                        .onSubmit {
                            NottawaEngine.shared.setParameterString(paramId: param.id, value: stringValue)
                        }
                        .onChange(of: stringValue) { _, newValue in
                            NottawaEngine.shared.setParameterString(paramId: param.id, value: newValue)
                        }
                }

            case .standard:
                VStack(alignment: .leading, spacing: 2) {
                    HStack {
                        Text(param.name)
                            .font(.caption)
                        Spacer()
                        editableValueLabel
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

        if showAudioDriver {
            AudioDriverControlsView(param: param)
                .transition(.opacity.combined(with: .move(edge: .top)))
        }

        if showMidi {
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

    // MARK: - Editable Value Label

    @ViewBuilder
    private var editableValueLabel: some View {
        if isEditingValue {
            TextField("", text: $editText)
                .focused($valueFieldFocused)
                .font(.system(size: 11, weight: .medium, design: .monospaced))
                .foregroundStyle(theme.colors.textPrimary)
                .multilineTextAlignment(.trailing)
                .frame(width: 54)
                .textFieldStyle(.plain)
                .padding(.horizontal, 4)
                .padding(.vertical, 1)
                .background(
                    RoundedRectangle(cornerRadius: 3)
                        .fill(theme.colors.backgroundTertiary)
                )
                .overlay(
                    RoundedRectangle(cornerRadius: 3)
                        .strokeBorder(theme.colors.accent.opacity(0.5), lineWidth: 1)
                )
                .onSubmit {
                    if let parsed = Float(editText) {
                        let clamped = min(max(parsed, param.minValue), param.maxValue)
                        sliderValue = clamped
                        NottawaEngine.shared.setParameterValue(paramId: param.id, value: clamped)
                    }
                    isEditingValue = false
                }
                .onExitCommand {
                    isEditingValue = false
                }
        } else {
            Text(String(format: "%.2f", sliderValue))
                .font(.caption)
                .foregroundStyle(theme.colors.textSecondary)
                .monospacedDigit()
                .padding(.horizontal, 4)
                .padding(.vertical, 1)
                .contentShape(Rectangle())
                .onTapGesture {
                    editText = String(format: "%.2f", sliderValue)
                    isEditingValue = true
                }
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
    static let parameterBeginEdit = Notification.Name("parameterBeginEdit")
    /// Keyboard-driven parameter step — ParameterControlView animates the thumb
    /// rather than snapping, so arrow-key nudges feel smooth.
    static let parameterStepAnimated = Notification.Name("parameterStepAnimated")
}

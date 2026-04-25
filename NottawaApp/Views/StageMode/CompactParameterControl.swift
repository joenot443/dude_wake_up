//
//  CompactParameterControl.swift
//  NottawaApp
//
//  Slim inline parameter control for Stage Mode channel strips.
//

import SwiftUI

struct CompactParameterControl: View {
    @Environment(ThemeManager.self) private var theme
    @Environment(NodeEditorViewModel.self) private var viewModel
    let param: ParameterInfo

    @State private var sliderValue: Float
    @State private var intValue: Int
    @State private var boolValue: Bool
    @State private var colorValue: Color

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

    private var isSelected: Bool {
        viewModel.selectedParameterId == param.id
    }

    var body: some View {
        if param.parameterType == .hidden {
            EmptyView()
        } else {
            content
                .padding(.horizontal, 4)
                .padding(.vertical, 2)
                .background(
                    RoundedRectangle(cornerRadius: 5)
                        .fill(isSelected ? theme.colors.accent.opacity(0.08) : Color.clear)
                )
                .overlay(
                    RoundedRectangle(cornerRadius: 5)
                        .strokeBorder(isSelected ? theme.colors.accent.opacity(0.6) : Color.clear, lineWidth: 1)
                )
                .contentShape(Rectangle())
                .id(param.id)
                .simultaneousGesture(
                    TapGesture().onEnded {
                        viewModel.selectedParameterId = param.id
                    }
                )
                .onReceive(NotificationCenter.default.publisher(for: .parameterStepAnimated)) { note in
                    guard let info = note.userInfo,
                          let targetId = info["paramId"] as? String,
                          targetId == param.id else { return }
                    withAnimation(.easeOut(duration: 0.18)) {
                        if let v = info["value"] as? Float { sliderValue = v }
                        if let v = info["intValue"] as? Int { intValue = v }
                        if let v = info["boolValue"] as? Bool { boolValue = v }
                    }
                }
        }
    }

    @ViewBuilder
    private var content: some View {
        switch param.parameterType {
        case .hidden:
            EmptyView()

        case .bool_:
            HStack {
                DSToggle(isOn: $boolValue, label: param.name, style: .switch, size: .sm)
                    .onChange(of: boolValue) { _, newValue in
                        NottawaEngine.shared.setParameterBool(paramId: param.id, value: newValue)
                    }
                Spacer()
                unfavoriteButton
            }

        case .int_:
            if !param.options.isEmpty {
                HStack {
                    Text(param.name)
                        .font(.caption2)
                        .lineLimit(1)
                    Spacer()
                    DSPicker(
                        selection: $intValue,
                        options: param.options.enumerated().map { DSPickerOption(value: $0.offset, label: $0.element) },
                        style: .menu, size: .sm
                    )
                    .onChange(of: intValue) { _, newValue in
                        NottawaEngine.shared.setParameterInt(paramId: param.id, value: newValue)
                    }
                    unfavoriteButton
                }
            } else {
                VStack(alignment: .leading, spacing: 1) {
                    HStack {
                        Text(param.name)
                            .font(.caption2)
                            .lineLimit(1)
                        Spacer()
                        oscillatorToggleIcon
                        unfavoriteButton
                    }
                    HStack(spacing: 4) {
                        Button {
                            if intValue > Int(param.minValue) { intValue -= 1 }
                        } label: {
                            Image(systemName: "minus")
                                .font(.system(size: 8, weight: .bold))
                                .frame(width: 16, height: 16)
                                .background(theme.colors.surface, in: RoundedRectangle(cornerRadius: 3))
                        }
                        .buttonStyle(.plainHitArea)
                        .disabled(intValue <= Int(param.minValue))

                        DSSlider(
                            value: Binding(
                                get: { Float(intValue) },
                                set: { intValue = Int($0.rounded()) }
                            ),
                            range: param.minValue...param.maxValue,
                            step: 1
                        )
                        .disabled(param.oscillatorEnabled || param.hasDriver)
                        .opacity(param.oscillatorEnabled || param.hasDriver ? 0.4 : 1.0)

                        Button {
                            if intValue < Int(param.maxValue) { intValue += 1 }
                        } label: {
                            Image(systemName: "plus")
                                .font(.system(size: 8, weight: .bold))
                                .frame(width: 16, height: 16)
                                .background(theme.colors.surface, in: RoundedRectangle(cornerRadius: 3))
                        }
                        .buttonStyle(.plainHitArea)
                        .disabled(intValue >= Int(param.maxValue))

                        Text("\(intValue)")
                            .font(.caption2.monospacedDigit())
                            .foregroundStyle(theme.colors.textSecondary)
                            .frame(minWidth: 22, alignment: .trailing)
                    }
                    .onChange(of: intValue) { _, newValue in
                        NottawaEngine.shared.setParameterInt(paramId: param.id, value: newValue)
                    }
                    if param.hasOscillator && param.oscillatorEnabled {
                        CompactOscillatorControls(param: param)
                    }
                }
            }

        case .color:
            HStack {
                Text(param.name)
                    .font(.caption2)
                    .lineLimit(1)
                Spacer()
                ColorPicker("", selection: $colorValue, supportsOpacity: false)
                    .labelsHidden()
                    .frame(width: 24, height: 24)
                    .onChange(of: colorValue) { _, newValue in
                        let resolved = newValue.resolve(in: EnvironmentValues())
                        NottawaEngine.shared.setParameterColor(
                            paramId: param.id,
                            r: resolved.red, g: resolved.green,
                            b: resolved.blue, a: resolved.opacity
                        )
                    }
                unfavoriteButton
            }

        case .text:
            VStack(alignment: .leading, spacing: 1) {
                Text(param.name)
                    .font(.caption2)
                    .lineLimit(1)
                TextField(param.name, text: Binding(
                    get: { param.stringValue },
                    set: { NottawaEngine.shared.setParameterString(paramId: param.id, value: $0) }
                ))
                .textFieldStyle(.roundedBorder)
                .font(.caption2)
            }

        case .standard:
            VStack(alignment: .leading, spacing: 1) {
                HStack {
                    Text(param.name)
                        .font(.caption2)
                        .lineLimit(1)
                    Spacer()
                    Text(String(format: "%.2f", sliderValue))
                        .font(.caption2)
                        .foregroundStyle(theme.colors.textSecondary)
                        .monospacedDigit()
                    oscillatorToggleIcon
                    unfavoriteButton
                }
                DSSlider(
                    value: $sliderValue,
                    range: param.minValue...param.maxValue
                )
                .disabled(param.oscillatorEnabled || param.hasDriver)
                .opacity(param.oscillatorEnabled || param.hasDriver ? 0.4 : 1.0)
                .onChange(of: sliderValue) { _, newValue in
                    NottawaEngine.shared.setParameterValue(paramId: param.id, value: newValue)
                }
                if param.hasOscillator && param.oscillatorEnabled {
                    CompactOscillatorControls(param: param)
                }
            }
        }
    }

    @ViewBuilder
    private var oscillatorToggleIcon: some View {
        if param.hasOscillator {
            Button {
                NottawaEngine.shared.setOscillatorEnabled(paramId: param.id, enabled: !param.oscillatorEnabled)
            } label: {
                Image(systemName: param.oscillatorEnabled ? "waveform.circle.fill" : "waveform.circle")
                    .font(.system(size: 9))
                    .foregroundStyle(param.oscillatorEnabled ? .cyan : .secondary)
            }
            .buttonStyle(.plainHitArea)
        }
    }

    private var unfavoriteButton: some View {
        Button {
            NottawaEngine.shared.toggleParameterFavorite(paramId: param.id)
        } label: {
            Image(systemName: "star.fill")
                .font(.system(size: 9))
                .foregroundStyle(theme.colors.warning)
        }
        .buttonStyle(.plainHitArea)
        .help("Remove from favorites")
    }
}

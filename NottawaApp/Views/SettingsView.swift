//
//  SettingsView.swift
//  NottawaApp
//
//  Settings window with resolution picker and theme selection.
//

import SwiftUI

struct SettingsView: View {
    @Environment(ThemeManager.self) private var theme
    @State private var resolutionIndex = 2

    private let resolutionOptions = [
        (name: "480p", index: 0),
        (name: "540p", index: 1),
        (name: "720p", index: 2),
        (name: "1080p", index: 3),
        (name: "1440p", index: 4),
        (name: "4K", index: 5),
    ]

    var body: some View {
        @Bindable var theme = theme

        Form {
            DSPicker(
                selection: $resolutionIndex,
                options: resolutionOptions.map { DSPickerOption(value: $0.index, label: $0.name) },
                label: "Resolution", style: .menu
            )
            .onChange(of: resolutionIndex) { _, newValue in
                NottawaEngine.shared.setResolution(settingIndex: newValue)
            }

            Picker("Theme", selection: $theme.preset) {
                ForEach(ThemePreset.allCases) { preset in
                    Text(preset.rawValue).tag(preset)
                }
            }
        }
        .formStyle(.grouped)
        .frame(width: 300)
        .onAppear {
            resolutionIndex = NottawaEngine.shared.resolutionSettingIndex
        }
    }
}

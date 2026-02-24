//
//  SettingsView.swift
//  NottawaApp
//
//  Settings window with resolution picker.
//

import SwiftUI

struct SettingsView: View {
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
        Form {
            Picker("Resolution", selection: $resolutionIndex) {
                ForEach(resolutionOptions, id: \.index) { option in
                    Text(option.name).tag(option.index)
                }
            }
            .onChange(of: resolutionIndex) { _, newValue in
                NottawaEngine.shared.setResolution(settingIndex: newValue)
            }
        }
        .formStyle(.grouped)
        .frame(width: 300)
        .onAppear {
            resolutionIndex = NottawaEngine.shared.resolutionSettingIndex
        }
    }
}

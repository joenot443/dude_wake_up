//
//  StageBottomBarView.swift
//  NottawaApp
//
//  Bottom bar for Stage Mode with BPM display and undo/redo.
//

import SwiftUI

struct StageBottomBarView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel
    @Environment(ThemeManager.self) private var theme
    @State private var bpm: Float = 120

    var body: some View {
        HStack {
            // BPM display
            Text("\(Int(bpm)) BPM")
                .font(.system(.body, design: .monospaced))
                .foregroundStyle(theme.colors.textSecondary)

            Spacer()

            // Undo / Redo
            HStack(spacing: 8) {
                Button {
                    viewModel.undo()
                } label: {
                    Image(systemName: "arrow.uturn.backward")
                }
                .disabled(!viewModel.canUndo)
                .help("Undo (Cmd+Z)")

                Button {
                    viewModel.redo()
                } label: {
                    Image(systemName: "arrow.uturn.forward")
                }
                .disabled(!viewModel.canRedo)
                .help("Redo (Cmd+Shift+Z)")
            }
        }
        .padding(.horizontal, 16)
        .padding(.vertical, 8)
        .background(theme.colors.backgroundSecondary)
        .onAppear { startBPMPolling() }
    }

    private func startBPMPolling() {
        Timer.scheduledTimer(withTimeInterval: 0.5, repeats: true) { _ in
            bpm = NottawaEngine.shared.currentBPM
        }
    }
}

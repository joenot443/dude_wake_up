//
//  ToolbarView.swift
//  NottawaApp
//
//  Top toolbar with undo/redo, add actions, and panel toggles.
//

import SwiftUI

struct ToolbarView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel
    @Environment(ThemeManager.self) private var theme

    var body: some View {
        HStack(spacing: 16) {
            // Undo / Redo
            HStack(spacing: 4) {
                Button {
                    viewModel.undo()
                } label: {
                    Image(systemName: "arrow.uturn.backward")
                }
                .disabled(!viewModel.canUndo)
                .help("Undo (Cmd+Z)")
                .accessibilityIdentifier("undo-button")

                Button {
                    viewModel.redo()
                } label: {
                    Image(systemName: "arrow.uturn.forward")
                }
                .disabled(!viewModel.canRedo)
                .help("Redo (Cmd+Shift+Z)")
                .accessibilityIdentifier("redo-button")
            }

            theme.colors.border.frame(width: 1, height: 20)

            // Sidebar toggle
            Button {
                viewModel.showSidebar.toggle()
            } label: {
                Image(systemName: "sidebar.leading")
            }
            .disabled(viewModel.stageModeEnabled)
            .help("Toggle Browser Sidebar")
            .accessibilityIdentifier("sidebar-toggle")

            // Audio panel toggle
            Button {
                viewModel.showAudioPanel.toggle()
            } label: {
                Image(systemName: "waveform")
            }
            .help("Toggle Audio Panel")
            .accessibilityIdentifier("audio-panel-toggle")

            // Stage mode toggle
            Button {
                withAnimation(.easeInOut(duration: 0.3)) {
                    viewModel.toggleStageMode()
                }
            } label: {
                Image(systemName: "slider.horizontal.2.rectangle.and.arrow.triangle.2.circlepath")
            }
            .foregroundStyle(viewModel.stageModeEnabled ? theme.colors.accent : theme.colors.textSecondary)
            .help("Stage Mode (Cmd+Shift+S)")
            .accessibilityIdentifier("stage-mode-toggle")

            theme.colors.border.frame(width: 1, height: 20)

            // Delete selected
            Button {
                viewModel.deleteSelected()
            } label: {
                Image(systemName: "trash")
            }
            .disabled(viewModel.selectedNodeIds.isEmpty && viewModel.selectedConnectionId == nil)
            .help("Delete selected (Delete)")
            .accessibilityIdentifier("delete-button")

            Spacer()

            // Workspace name indicator
            if let name = viewModel.currentWorkspaceName {
                Text(name)
                    .font(.system(size: 12, weight: .medium))
                    .foregroundStyle(theme.colors.textSecondary)
                    .accessibilityIdentifier("workspace-name")
            }
        }
        .buttonStyle(.plainHitArea)
        .font(.system(size: 14))
        .foregroundStyle(theme.colors.textSecondary)
        .padding(.horizontal, 16)
        .padding(.vertical, 8)
        .background(theme.colors.backgroundSecondary)
    }
}

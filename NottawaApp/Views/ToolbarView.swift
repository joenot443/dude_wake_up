//
//  ToolbarView.swift
//  NottawaApp
//
//  Top toolbar with undo/redo, add actions, and panel toggles.
//

import SwiftUI

struct ToolbarView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel

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

            Divider()
                .frame(height: 20)

            // Sidebar toggle
            Button {
                viewModel.showSidebar.toggle()
            } label: {
                Image(systemName: "sidebar.leading")
            }
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

            Divider()
                .frame(height: 20)

            // Delete selected
            Button {
                viewModel.deleteSelected()
            } label: {
                Image(systemName: "trash")
            }
            .disabled(viewModel.selectedNodeIds.isEmpty)
            .help("Delete selected (Delete)")
            .accessibilityIdentifier("delete-button")

            Spacer()
        }
        .padding(.horizontal, 16)
        .padding(.vertical, 8)
        .background(.ultraThinMaterial)
    }
}

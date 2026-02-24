//
//  ActionBarView.swift
//  NottawaApp
//
//  Floating action bar overlaid on the canvas, matching the ImGui version.
//

import SwiftUI

struct ActionBarView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel
    var canvasSize: CGSize

    var body: some View {
        Group {
            if viewModel.actionBarExpanded {
                expandedBar
                    .transition(.opacity.combined(with: .scale(scale: 0.8, anchor: .trailing)))
            } else {
                collapsedBar
                    .transition(.opacity.combined(with: .scale(scale: 0.8, anchor: .trailing)))
            }
        }
        .animation(.easeInOut(duration: 0.2), value: viewModel.actionBarExpanded)
    }

    // MARK: - Collapsed

    private var collapsedBar: some View {
        Button {
            viewModel.actionBarExpanded = true
        } label: {
            Image(systemName: "chevron.left")
                .font(.system(size: 14, weight: .semibold))
                .frame(width: 28, height: 28)
        }
        .buttonStyle(ActionBarButtonStyle())
        .accessibilityIdentifier("action-bar-expand")
        .padding(6)
        .background(Color(white: 0.13, opacity: 0.7))
        .clipShape(RoundedRectangle(cornerRadius: 12))
    }

    // MARK: - Expanded

    private var expandedBar: some View {
        HStack(spacing: 4) {
            // Collapse
            Button {
                viewModel.actionBarExpanded = false
            } label: {
                Image(systemName: "chevron.right")
                    .font(.system(size: 14, weight: .semibold))
                    .frame(width: 28, height: 28)
            }
            .buttonStyle(ActionBarButtonStyle())
            .accessibilityIdentifier("action-bar-collapse")
            .help("Collapse action bar")

            Divider()
                .frame(height: 20)
                .opacity(0.3)

            // Undo
            Button { viewModel.undo() } label: {
                Image(systemName: "arrow.uturn.backward")
                    .font(.system(size: 14, weight: .medium))
                    .frame(width: 28, height: 28)
            }
            .buttonStyle(ActionBarButtonStyle())
            .accessibilityIdentifier("action-bar-undo")
            .disabled(!viewModel.canUndo)
            .help("Undo (Cmd+Z)")

            // Redo
            Button { viewModel.redo() } label: {
                Image(systemName: "arrow.uturn.forward")
                    .font(.system(size: 14, weight: .medium))
                    .frame(width: 28, height: 28)
            }
            .buttonStyle(ActionBarButtonStyle())
            .accessibilityIdentifier("action-bar-redo")
            .disabled(!viewModel.canRedo)
            .help("Redo (Cmd+Shift+Z)")

            Divider()
                .frame(height: 20)
                .opacity(0.3)

            // Screenshot
            Button { viewModel.captureScreenshot() } label: {
                Image(systemName: "camera")
                    .font(.system(size: 14, weight: .medium))
                    .frame(width: 28, height: 28)
            }
            .buttonStyle(ActionBarButtonStyle())
            .accessibilityIdentifier("action-bar-screenshot")
            .help("Capture screenshot")

            // Help toggle
            Button { viewModel.helpEnabled.toggle() } label: {
                Image(systemName: viewModel.helpEnabled ? "questionmark.circle.fill" : "questionmark.circle")
                    .font(.system(size: 14, weight: .medium))
                    .frame(width: 28, height: 28)
            }
            .buttonStyle(ActionBarButtonStyle(isActive: viewModel.helpEnabled))
            .accessibilityIdentifier("action-bar-help")
            .help("Toggle help overlay")

            Divider()
                .frame(height: 20)
                .opacity(0.3)

            // Clear / Reset
            Button { viewModel.newWorkspace() } label: {
                Image(systemName: "trash.slash")
                    .font(.system(size: 14, weight: .medium))
                    .frame(width: 28, height: 28)
            }
            .buttonStyle(ActionBarButtonStyle())
            .accessibilityIdentifier("action-bar-clear")
            .help("Clear all nodes")

            // Zoom to fit
            Button {
                viewModel.zoomToFit(canvasSize: canvasSize)
            } label: {
                Image(systemName: "arrow.down.right.and.arrow.up.left")
                    .font(.system(size: 14, weight: .medium))
                    .frame(width: 28, height: 28)
            }
            .buttonStyle(ActionBarButtonStyle())
            .accessibilityIdentifier("action-bar-zoom-fit")
            .help("Zoom to fit all nodes")
        }
        .padding(.horizontal, 8)
        .padding(.vertical, 6)
        .background(Color(white: 0.13, opacity: 0.7))
        .clipShape(RoundedRectangle(cornerRadius: 12))
    }
}

// MARK: - Button Style

private struct ActionBarButtonStyle: ButtonStyle {
    var isActive: Bool = false

    func makeBody(configuration: Configuration) -> some View {
        configuration.label
            .foregroundStyle(
                configuration.isPressed ? .white.opacity(0.5) :
                isActive ? .cyan :
                .white.opacity(0.8)
            )
            .background(
                configuration.isPressed ?
                    Color.white.opacity(0.1) :
                    Color.clear
            )
            .clipShape(RoundedRectangle(cornerRadius: 6))
    }
}

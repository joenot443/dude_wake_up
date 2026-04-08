//
//  StageOutputView.swift
//  NottawaApp
//
//  Broadcast-monitor-style output panel for Stage Mode.
//  Output picker, solo badge, pop-out button, and window list.
//

import SwiftUI

struct StageOutputView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel
    @Environment(ThemeManager.self) private var theme

    private var isSoloed: Bool {
        viewModel.soloedNodeId != nil
    }

    private var currentOutputName: String {
        if let id = viewModel.stageOutputId,
           let node = viewModel.nodes.first(where: { $0.id == id }) {
            return node.name
        }
        return "No Output"
    }

    var body: some View {
        VStack(spacing: 0) {
            // MARK: - Header toolbar
            headerBar
                .padding(.horizontal, 12)
                .padding(.vertical, 8)
                .background(theme.colors.backgroundSecondary)

            theme.colors.border.frame(height: 1)

            // MARK: - Preview monitor
            previewMonitor
                .padding(12)

            // MARK: - Open windows list
            openWindowsList
                .padding(.horizontal, 12)
                .padding(.bottom, 12)
        }
    }

    // MARK: - Header Bar

    private var headerBar: some View {
        HStack(spacing: 8) {
            // Output picker capsule
            outputPicker

            // Solo badge (click to clear)
            if let soloId = viewModel.soloedNodeId,
               let node = viewModel.nodes.first(where: { $0.id == soloId }) {
                Button {
                    viewModel.toggleSolo(soloId)
                } label: {
                    HStack(spacing: 4) {
                        Circle()
                            .fill(.black.opacity(0.5))
                            .frame(width: 6, height: 6)
                        Text("SOLO: \(node.name)")
                            .font(.caption2.weight(.bold))
                            .lineLimit(1)
                    }
                    .padding(.horizontal, 8)
                    .padding(.vertical, 4)
                    .background(.yellow, in: Capsule())
                    .foregroundStyle(.black)
                }
                .buttonStyle(.plainHitArea)
                .help("Click to clear solo")
            }

            Spacer()

            // Pop-out button
            Button {
                if let outputId = viewModel.stageOutputId {
                    viewModel.openPreviewWindow(outputId)
                }
            } label: {
                HStack(spacing: 4) {
                    Image(systemName: "pip.enter")
                        .font(.caption)
                    Text("Pop Out")
                        .font(.caption2.weight(.medium))
                }
                .padding(.horizontal, 8)
                .padding(.vertical, 4)
                .background(theme.colors.surface, in: RoundedRectangle(cornerRadius: 6))
            }
            .buttonStyle(.plainHitArea)
            .help("Open in a separate window")
            .disabled(viewModel.stageOutputId == nil)
        }
    }

    // MARK: - Output Picker

    private var selectedOutputLabel: String {
        if let pinId = viewModel.pinnedOutputId,
           let node = viewModel.nodes.first(where: { $0.id == pinId }) {
            return node.name
        }
        return "Auto"
    }

    private var outputPicker: some View {
        Menu {
            Button {
                viewModel.pinnedOutputId = nil
            } label: {
                HStack {
                    Text("Auto (deepest output)")
                    if viewModel.pinnedOutputId == nil {
                        Image(systemName: "checkmark")
                    }
                }
            }

            Divider()

            ForEach(viewModel.orderedChainStrips, id: \.node.id) { strip in
                Button {
                    viewModel.pinnedOutputId = strip.node.id
                } label: {
                    HStack {
                        Text(strip.node.name)
                        if viewModel.pinnedOutputId == strip.node.id {
                            Image(systemName: "checkmark")
                        }
                    }
                }
            }
        } label: {
            HStack(spacing: 5) {
                Image(systemName: "tv")
                    .font(.caption)
                Text(selectedOutputLabel)
                    .font(.caption.weight(.semibold))
                    .lineLimit(1)
                Image(systemName: "chevron.down")
                    .font(.system(size: 8, weight: .bold))
            }
            .padding(.horizontal, 10)
            .padding(.vertical, 5)
            .background(theme.colors.surface, in: Capsule())
            .foregroundStyle(theme.colors.textPrimary)
        }
        .menuStyle(.borderlessButton)
        .fixedSize()
    }

    // MARK: - Preview Monitor

    private var previewMonitor: some View {
        ZStack(alignment: .bottomLeading) {
            if let outputId = viewModel.stageOutputId {
                TextureDisplayView(connectableId: outputId)
                    .aspectRatio(16.0 / 9.0, contentMode: .fit)
                    .clipShape(RoundedRectangle(cornerRadius: 6))
            } else {
                RoundedRectangle(cornerRadius: 6)
                    .fill(Color.black)
                    .aspectRatio(16.0 / 9.0, contentMode: .fit)
                    .overlay {
                        Text("No output")
                            .font(.callout)
                            .foregroundStyle(theme.colors.textTertiary)
                    }
            }

            // PGM-style label overlay
            if viewModel.stageOutputId != nil {
                Text(currentOutputName)
                    .font(.caption2.weight(.bold).monospaced())
                    .foregroundStyle(.white)
                    .padding(.horizontal, 6)
                    .padding(.vertical, 3)
                    .background(.black.opacity(0.6), in: RoundedRectangle(cornerRadius: 4))
                    .padding(8)
            }
        }
        .overlay(
            RoundedRectangle(cornerRadius: 6)
                .strokeBorder(
                    isSoloed ? theme.colors.warning.opacity(0.8) : Color.white.opacity(0.08),
                    lineWidth: isSoloed ? 2 : 1
                )
        )
        .shadow(color: isSoloed ? theme.colors.warning.opacity(0.3) : .clear, radius: 8)
    }

    // MARK: - Open Windows List

    @ViewBuilder
    private var openWindowsList: some View {
        let openIds = Array(viewModel.openPreviewWindowIds).sorted()
        if !openIds.isEmpty {
            VStack(alignment: .leading, spacing: 0) {
                Text("WINDOWS")
                    .font(.system(size: 9, weight: .bold).monospaced())
                    .foregroundStyle(theme.colors.textTertiary)
                    .padding(.bottom, 6)

                ForEach(openIds, id: \.self) { nodeId in
                    let name = viewModel.nodes.first(where: { $0.id == nodeId })?.name ?? nodeId
                    HStack(spacing: 6) {
                        Image(systemName: "rectangle.on.rectangle")
                            .font(.caption2)
                            .foregroundStyle(theme.colors.textSecondary)
                        Text(name)
                            .font(.caption)
                            .lineLimit(1)
                        Spacer()
                        Button {
                            viewModel.closePreviewWindow(nodeId)
                        } label: {
                            Image(systemName: "xmark.circle.fill")
                                .font(.caption)
                                .foregroundStyle(theme.colors.textSecondary)
                        }
                        .buttonStyle(.plainHitArea)
                    }
                    .padding(.vertical, 4)
                    .padding(.horizontal, 8)
                    .background(theme.colors.surface.opacity(0.5), in: RoundedRectangle(cornerRadius: 5))
                    .padding(.bottom, 2)
                }
            }
        }
    }
}

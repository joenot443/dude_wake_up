//
//  ChannelStripView.swift
//  NottawaApp
//
//  Per-shader/source row in Stage Mode showing preview, name, solo/bypass, and favorited params.
//

import SwiftUI

struct ChannelStripView: View {
    let strip: ChainStrip
    let index: Int
    let total: Int

    @Environment(NodeEditorViewModel.self) private var viewModel
    @Environment(ThemeManager.self) private var theme

    private var isSoloed: Bool {
        viewModel.soloedNodeId == strip.node.id
    }

    private var displayedParams: [ParameterInfo] {
        viewModel.stageShowAllParams ? strip.allParams : strip.favoriteParams
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            HStack(spacing: 10) {
                // Drag handle
                Image(systemName: "line.3.horizontal")
                    .font(.system(size: 14, weight: .medium))
                    .foregroundStyle(theme.colors.textTertiary)
                    .frame(width: 20)

                // Static snapshot thumbnail
                StripThumbnailView(connectableId: strip.node.id)
                    .frame(width: 64, height: 64)
                    .clipShape(RoundedRectangle(cornerRadius: 6))

                // Name and type
                VStack(alignment: .leading, spacing: 2) {
                    Text(strip.node.name)
                        .font(.headline)
                        .lineLimit(1)
                    Text(strip.node.isSource ? "Source" : "Effect")
                        .font(.caption2)
                        .foregroundStyle(theme.colors.textSecondary)
                }

                Spacer()

                // Solo button
                Button("S") {
                    viewModel.toggleSolo(strip.node.id)
                }
                .buttonStyle(StageButtonStyle(isActive: isSoloed, activeColor: .yellow))
                .help("Solo — preview this node's output")

                // Bypass button (shaders only)
                if strip.node.isShader {
                    Button("B") {
                        viewModel.toggleBypass(strip.node.id)
                    }
                    .buttonStyle(StageButtonStyle(isActive: strip.node.isBypassed, activeColor: .orange))
                    .help("Bypass — pass input through without processing")
                }
            }

            // Parameter sliders
            if !displayedParams.isEmpty {
                LazyVGrid(columns: [
                    GridItem(.flexible(), spacing: 8),
                    GridItem(.flexible(), spacing: 8)
                ], spacing: 4) {
                    ForEach(displayedParams) { param in
                        CompactParameterControl(param: param)
                    }
                }
            } else if !viewModel.stageShowAllParams {
                Text("No favorited parameters")
                    .font(.caption2)
                    .foregroundStyle(theme.colors.textTertiary)
                    .padding(.leading, 4)
            }
        }
        .padding(10)
        .background(strip.node.isBypassed ? theme.colors.surface.opacity(0.5) : theme.colors.surface)
        .clipShape(RoundedRectangle(cornerRadius: 8))
        .opacity(strip.node.isBypassed ? 0.5 : 1.0)
        .onDrag {
            NSItemProvider(object: ("strip:" + strip.node.id) as NSString)
        }
    }
}

// MARK: - Drop Delegate for Strip Reordering

struct StripDropDelegate: DropDelegate {
    let targetIndex: Int
    let viewModel: NodeEditorViewModel
    @Binding var dropTargetIndex: Int?

    func dropEntered(info: DropInfo) {
        withAnimation(.easeInOut(duration: 0.15)) {
            dropTargetIndex = targetIndex
        }
    }

    func dropUpdated(info: DropInfo) -> DropProposal? {
        DropProposal(operation: .move)
    }

    func dropExited(info: DropInfo) {
        withAnimation(.easeInOut(duration: 0.15)) {
            if dropTargetIndex == targetIndex {
                dropTargetIndex = nil
            }
        }
    }

    func performDrop(info: DropInfo) -> Bool {
        dropTargetIndex = nil
        guard let provider = info.itemProviders(for: [.text]).first else { return false }
        provider.loadItem(forTypeIdentifier: "public.text", options: nil) { data, _ in
            guard let data = data as? Data,
                  let str = String(data: data, encoding: .utf8),
                  str.hasPrefix("strip:") else { return }
            let draggedId = String(str.dropFirst("strip:".count))
            DispatchQueue.main.async {
                guard let fromIndex = viewModel.orderedChainStrips.firstIndex(where: { $0.node.id == draggedId }) else { return }
                guard fromIndex != targetIndex else { return }
                let dest = fromIndex < targetIndex ? targetIndex + 1 : targetIndex
                withAnimation(.easeInOut(duration: 0.2)) {
                    viewModel.moveStageStrip(from: IndexSet(integer: fromIndex), to: dest)
                }
            }
        }
        return true
    }

    func validateDrop(info: DropInfo) -> Bool {
        true
    }
}

// MARK: - Static Snapshot Thumbnail

/// Displays a static CGImage snapshot instead of a live MTKView.
/// The image updates whenever SwiftUI re-evaluates (driven by the stage refresh timer).
struct StripThumbnailView: View {
    @Environment(ThemeManager.self) private var theme
    let connectableId: String

    var body: some View {
        if let cgImage = TextureProvider.shared.snapshot(for: connectableId) {
            Image(decorative: cgImage, scale: 1.0)
                .resizable()
                .aspectRatio(contentMode: .fill)
        } else {
            Rectangle()
                .fill(theme.colors.surface)
        }
    }
}

// MARK: - Stage Button Style

struct StageButtonStyle: ButtonStyle {
    @Environment(ThemeManager.self) var theme
    let isActive: Bool
    let activeColor: Color

    func makeBody(configuration: Configuration) -> some View {
        configuration.label
            .font(.system(size: 12, weight: .bold, design: .monospaced))
            .frame(width: 28, height: 28)
            .background(isActive ? activeColor : theme.colors.surface)
            .foregroundStyle(isActive ? .black : theme.colors.textPrimary)
            .clipShape(RoundedRectangle(cornerRadius: 6))
            .overlay(
                RoundedRectangle(cornerRadius: 6)
                    .strokeBorder(isActive ? activeColor : theme.colors.border, lineWidth: 1)
            )
            .opacity(configuration.isPressed ? 0.7 : 1.0)
    }
}

//
//  NodeView.swift
//  NottawaApp
//
//  Visual representation of a single node (shader or source) on the canvas.
//

import SwiftUI

struct NodeView: View {
    let node: NodeModel
    let isSelected: Bool
    var scale: CGFloat = 1.0

    @Environment(NodeEditorViewModel.self) private var viewModel
    @Environment(ThemeManager.self) private var theme

    // Base dimensions (at scale 1.0)
    private let baseNodeWidth: CGFloat = 260
    private let baseNodeHeight: CGFloat = 146
    private let baseCornerRadius: CGFloat = 14
    private let basePinSize: CGFloat = 10

    private var nodeWidth: CGFloat { baseNodeWidth * scale }
    private var nodeHeight: CGFloat { baseNodeHeight * scale }
    private var cornerRadius: CGFloat { baseCornerRadius * scale }
    private var pinSize: CGFloat { basePinSize * scale }

    private var connectedInputSlots: Set<Int> {
        Set(viewModel.connections.filter { $0.endNodeId == node.id }.map(\.inputSlot))
    }

    private var hasOutputConnection: Bool {
        viewModel.connections.contains { $0.startNodeId == node.id && $0.outputSlot == 0 }
    }

    private var hasAuxOutputConnection: Bool {
        viewModel.connections.contains { $0.startNodeId == node.id && $0.outputSlot == 10 }
    }

    private var isSwapTarget: Bool {
        viewModel.swapTargetNodeId == node.id
    }

    // Scaled font helper
    private func scaledFont(_ size: CGFloat, weight: Font.Weight = .regular) -> Font {
        .system(size: size * scale, weight: weight)
    }

    var body: some View {
        VStack(spacing: 0) {
            // Main node: ZStack with preview filling entire area
            ZStack(alignment: .bottom) {
                // Full-bleed preview
                TextureDisplayView(connectableId: node.id)
                    .frame(width: nodeWidth, height: nodeHeight)
                    .clipped()
                    .allowsHitTesting(false)
                    .opacity(node.isActive ? 1.0 : 0.4)

                // Download overlay
                if let progress = node.downloadProgress {
                    NodeDownloadOverlay(
                        progress: progress,
                        isPaused: node.downloadPaused
                    )
                    .frame(width: nodeWidth, height: nodeHeight)
                    .allowsHitTesting(false)
                }

                // Bottom gradient overlay with title + actions
                VStack(spacing: 4 * scale) {
                    // Title row
                    HStack(spacing: 4 * scale) {
                        Circle()
                            .fill(node.isSource ? Color.green : theme.colors.accent)
                            .frame(width: 6 * scale, height: 6 * scale)
                        Text(node.name)
                            .font(scaledFont(11, weight: .bold))
                            .foregroundStyle(theme.colors.textOnAccent)
                            .shadow(color: .black.opacity(0.8), radius: 3, y: 1)
                            .lineLimit(1)
                        Spacer()
                    }

                    // Action buttons row
                    HStack(spacing: 6 * scale) {
                        Button {
                            viewModel.toggleActiveState(node.id)
                        } label: {
                            Image(systemName: node.isActive ? "pause.fill" : "play.fill")
                                .font(scaledFont(10, weight: .medium))
                        }
                        .buttonStyle(.plain)
                        .foregroundStyle(theme.colors.textOnAccent.opacity(0.7))

                        Button {
                            if isSwapTarget {
                                viewModel.swapTargetNodeId = nil
                            } else {
                                viewModel.swapTargetNodeId = node.id
                            }
                        } label: {
                            Image(systemName: "arrow.triangle.2.circlepath")
                                .font(scaledFont(10, weight: .medium))
                        }
                        .buttonStyle(.plain)
                        .foregroundStyle(isSwapTarget ? theme.colors.accent : theme.colors.textOnAccent.opacity(0.7))
                        .help(node.isShader ? "Swap shader type" : "Swap source type")

                        Spacer()

                        Button {
                            viewModel.deleteNode(node.id)
                        } label: {
                            Image(systemName: "trash")
                                .font(scaledFont(10, weight: .medium))
                        }
                        .buttonStyle(.plain)
                        .foregroundStyle(Color.red.opacity(0.7))
                    }
                }
                .padding(.horizontal, 10 * scale)
                .padding(.bottom, 6 * scale)
                .padding(.top, 20 * scale)
                .background(
                    LinearGradient(
                        stops: [
                            .init(color: .clear, location: 0.0),
                            .init(color: .black.opacity(0.6), location: 0.2),
                            .init(color: .black.opacity(0.85), location: 1.0),
                        ],
                        startPoint: .top,
                        endPoint: .bottom
                    )
                )
            }
            .frame(width: nodeWidth, height: nodeHeight)
            .clipShape(RoundedRectangle(cornerRadius: cornerRadius))
            // Subtle border for definition
            .overlay(
                RoundedRectangle(cornerRadius: cornerRadius)
                    .strokeBorder(theme.colors.border.opacity(0.4), lineWidth: 1)
            )
            // Selection glow border
            .overlay(
                Group {
                    if isSelected {
                        RoundedRectangle(cornerRadius: cornerRadius)
                            .strokeBorder(theme.colors.accent, lineWidth: 2.5 * scale)
                        RoundedRectangle(cornerRadius: cornerRadius + 2)
                            .strokeBorder(theme.colors.accent.opacity(0.3), lineWidth: 4 * scale)
                            .blur(radius: 6)
                            .padding(-2)
                    }
                }
            )
            // Edge-mounted input pins (left)
            .overlay(alignment: .leading) {
                if node.inputCount > 0 {
                    VStack(spacing: 4 * scale) {
                        ForEach(0..<node.inputCount, id: \.self) { slot in
                            PinView(
                                nodeId: node.id,
                                slotIndex: slot,
                                isOutput: false,
                                isConnected: connectedInputSlots.contains(slot),
                                scale: scale,
                                positionOffset: -pinSize / 2
                            )
                        }
                    }
                    .offset(x: -pinSize / 2)
                }
            }
            // Edge-mounted output pins (right)
            .overlay(alignment: .trailing) {
                VStack(spacing: 4 * scale) {
                    PinView(
                        nodeId: node.id,
                        slotIndex: 0,
                        isOutput: true,
                        isConnected: hasOutputConnection,
                        scale: scale,
                        positionOffset: pinSize / 2
                    )
                    if node.supportsAuxOutput {
                        PinView(
                            nodeId: node.id,
                            slotIndex: 10,
                            isOutput: true,
                            isConnected: hasAuxOutputConnection,
                            scale: scale,
                            positionOffset: pinSize / 2
                        )
                        .overlay(
                            Text("A")
                                .font(scaledFont(6, weight: .bold))
                                .foregroundStyle(theme.colors.textOnAccent)
                                .allowsHitTesting(false)
                        )
                    }
                }
                .offset(x: pinSize / 2)
            }
            // Shadow
            .shadow(color: .black.opacity(0.4), radius: 8 * scale, y: 4 * scale)

            // Inline swap browser — expands below the node
            if isSwapTarget {
                NodeInlineBrowserView(
                    isShaderBrowser: node.isShader,
                    onSelect: { shaderType in
                        if node.isShader {
                            viewModel.swapShader(nodeId: node.id, newType: shaderType)
                        } else {
                            viewModel.swapSource(nodeId: node.id, newType: shaderType)
                        }
                        viewModel.swapTargetNodeId = nil
                    },
                    onCancel: {
                        viewModel.swapTargetNodeId = nil
                    }
                )
            }
        }
        .frame(width: nodeWidth)
        .contentShape(RoundedRectangle(cornerRadius: cornerRadius))
        .accessibilityIdentifier("node-\(node.name)")
    }

}

// MARK: - Download Progress Overlay

/// Overlay shown on library source nodes while downloading.
private struct NodeDownloadOverlay: View {
    @Environment(ThemeManager.self) var theme
    let progress: Float
    let isPaused: Bool

    private var percentText: String {
        "\(Int(progress * 100))%"
    }

    var body: some View {
        ZStack {
            theme.colors.background.opacity(0.8)

            VStack(spacing: 8) {
                ZStack {
                    // Background ring
                    Circle()
                        .stroke(theme.colors.textTertiary.opacity(0.3), lineWidth: 4)
                        .frame(width: 50, height: 50)

                    // Progress ring
                    Circle()
                        .trim(from: 0, to: CGFloat(progress))
                        .stroke(
                            isPaused ? Color.orange : theme.colors.accent,
                            style: StrokeStyle(lineWidth: 4, lineCap: .round)
                        )
                        .frame(width: 50, height: 50)
                        .rotationEffect(.degrees(-90))

                    // Percentage
                    Text(percentText)
                        .font(.system(size: 14, weight: .semibold, design: .monospaced))
                        .foregroundStyle(theme.colors.textPrimary)
                }

                Text(isPaused ? "Paused" : "Downloading...")
                    .font(.system(size: 11))
                    .foregroundStyle(theme.colors.textSecondary)
            }
        }
    }
}

// MARK: - Inline Browser for Node Swap

/// Live-preview browser that expands directly from a node for shader/source swapping.
/// Shows a 2-column grid of animated preview tiles, matching the full browser experience.
struct NodeInlineBrowserView: View {
    let isShaderBrowser: Bool
    let onSelect: (Int) -> Void
    let onCancel: () -> Void

    @Environment(ThemeManager.self) private var theme
    @State private var searchText = ""
    @State private var selectedCategory = "All"
    private let previewManager = BrowserPreviewManager.shared

    private let columns = [
        GridItem(.flexible(), spacing: 8),
        GridItem(.flexible(), spacing: 8),
    ]

    private var allItems: [AvailableShaderInfo] {
        isShaderBrowser
            ? NottawaEngine.shared.availableShaders(category: shaderCategory)
            : NottawaEngine.shared.availableShaderSources()
    }

    /// Map the selected category string to ShaderCategory enum for the shader browser.
    private var shaderCategory: NottawaEngine.ShaderCategory {
        switch selectedCategory {
        case "Basic": return .basic
        case "Filter": return .filter
        case "Glitch": return .glitch
        case "Transform": return .transform
        case "Mix": return .mix
        case "Mask": return .mask
        default: return .all
        }
    }

    private var categories: [String] {
        if isShaderBrowser {
            return ["All", "Basic", "Filter", "Glitch", "Transform", "Mix", "Mask"]
        } else {
            let sources = NottawaEngine.shared.availableShaderSources()
            let cats = Set(sources.compactMap(\.category))
            return ["All"] + cats.sorted()
        }
    }

    private var items: [AvailableShaderInfo] {
        var result = allItems
        // Source category filtering (shader filtering is done via the enum)
        if !isShaderBrowser && selectedCategory != "All" {
            result = result.filter { $0.category == selectedCategory }
        }
        if !searchText.isEmpty {
            result = result.filter { $0.name.localizedCaseInsensitiveContains(searchText) }
        }
        return result
    }

    var body: some View {
        VStack(spacing: 0) {
            Divider()

            // Search
            HStack(spacing: 6) {
                Image(systemName: "magnifyingglass")
                    .font(.system(size: 10))
                    .foregroundStyle(.secondary)
                TextField("Search...", text: $searchText)
                    .textFieldStyle(.plain)
                    .font(.system(size: 11))
            }
            .padding(.horizontal, 8)
            .padding(.vertical, 6)
            .background(Color(.textBackgroundColor).opacity(0.5))

            // Category pills
            ScrollView(.horizontal, showsIndicators: false) {
                HStack(spacing: 4) {
                    ForEach(categories, id: \.self) { cat in
                        Button {
                            selectedCategory = cat
                        } label: {
                            Text(cat)
                                .font(.system(size: 9, weight: selectedCategory == cat ? .semibold : .regular))
                                .padding(.horizontal, 7)
                                .padding(.vertical, 3)
                                .background(selectedCategory == cat ? theme.colors.accent : Color(.controlBackgroundColor))
                                .foregroundStyle(selectedCategory == cat ? theme.colors.textOnAccent : .primary)
                                .clipShape(Capsule())
                        }
                        .buttonStyle(.plain)
                    }
                }
                .padding(.horizontal, 8)
                .padding(.vertical, 4)
            }

            Divider()

            // Live preview grid
            ScrollView {
                LazyVGrid(columns: columns, spacing: 8) {
                    ForEach(items) { item in
                        PreviewTileView(
                            name: item.name,
                            shaderTypeRaw: item.shaderTypeRaw,
                            isSource: !isShaderBrowser,
                            previewManager: previewManager
                        ) {
                            previewManager.destroyAll()
                            onSelect(item.shaderTypeRaw)
                        }
                    }
                }
                .padding(8)
            }
            .frame(height: 300)

            Divider()

            // Cancel
            Button {
                previewManager.destroyAll()
                onCancel()
            } label: {
                Text("Cancel")
                    .font(.system(size: 11))
                    .foregroundStyle(.secondary)
                    .frame(maxWidth: .infinity)
                    .padding(.vertical, 5)
                    .contentShape(Rectangle())
            }
            .buttonStyle(.plain)
        }
        .background(Color(.windowBackgroundColor))
        .onDisappear {
            previewManager.destroyAll()
        }
    }
}

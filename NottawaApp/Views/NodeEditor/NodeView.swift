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

    // Base dimensions (at scale 1.0)
    private let baseNodeWidth: CGFloat = 260
    private let basePreviewHeight: CGFloat = 146
    private let baseCornerRadius: CGFloat = 12

    private var nodeWidth: CGFloat { baseNodeWidth * scale }
    private var previewHeight: CGFloat { basePreviewHeight * scale }
    private var cornerRadius: CGFloat { baseCornerRadius * scale }

    private var borderColor: Color {
        if isSelected { return .white }
        return node.isSource ? .blue : .purple
    }

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
        viewModel.swapTargetNodeId == node.id &&
        viewModel.browserMode == .swap &&
        (viewModel.showShaderBrowser || viewModel.showSourceBrowser)
    }

    // Scaled font helper
    private func scaledFont(_ size: CGFloat, weight: Font.Weight = .regular) -> Font {
        .system(size: size * scale, weight: weight)
    }

    var body: some View {
        VStack(spacing: 0) {
            // Preview
            ZStack {
                TextureDisplayView(connectableId: node.id)
                    .frame(width: nodeWidth, height: previewHeight)
                    .clipped()
                    .allowsHitTesting(false)
                    .opacity(node.isActive ? 1.0 : 0.4)

                if let progress = node.downloadProgress {
                    NodeDownloadOverlay(
                        progress: progress,
                        isPaused: node.downloadPaused
                    )
                    .frame(width: nodeWidth, height: previewHeight)
                    .allowsHitTesting(false)
                }
            }

            // Pin bar
            HStack(spacing: 0) {
                // Input pins
                HStack(spacing: 4 * scale) {
                    if node.inputCount > 0 {
                        ForEach(0..<node.inputCount, id: \.self) { slot in
                            PinView(
                                nodeId: node.id,
                                slotIndex: slot,
                                isOutput: false,
                                isConnected: connectedInputSlots.contains(slot),
                                scale: scale
                            )
                        }
                    }
                }
                .frame(maxWidth: .infinity, alignment: .leading)

                // Node name
                Text(node.name)
                    .font(scaledFont(11, weight: .medium))
                    .lineLimit(1)
                    .foregroundStyle(.primary)
                    .frame(maxWidth: .infinity)

                // Output pins
                HStack(spacing: 4 * scale) {
                    PinView(
                        nodeId: node.id,
                        slotIndex: 0,
                        isOutput: true,
                        isConnected: hasOutputConnection,
                        scale: scale
                    )
                    if node.supportsAuxOutput {
                        PinView(
                            nodeId: node.id,
                            slotIndex: 10,
                            isOutput: true,
                            isConnected: hasAuxOutputConnection,
                            scale: scale
                        )
                        .overlay(
                            Text("A")
                                .font(scaledFont(7, weight: .bold))
                                .foregroundStyle(.white)
                                .allowsHitTesting(false)
                        )
                    }
                }
                .frame(maxWidth: .infinity, alignment: .trailing)
            }
            .padding(.horizontal, 8 * scale)
            .padding(.vertical, 6 * scale)
            .background(Color(.windowBackgroundColor).opacity(0.9))

            // Action buttons
            HStack(spacing: 8 * scale) {
                Button {
                    viewModel.toggleActiveState(node.id)
                } label: {
                    Image(systemName: node.isActive ? "pause.fill" : "play.fill")
                        .font(scaledFont(11))
                }
                .buttonStyle(.plain)
                .foregroundStyle(.secondary)

                Button {
                    if isSwapTarget {
                        closeBrowser()
                    } else {
                        viewModel.swapTargetNodeId = node.id
                        viewModel.browserMode = .swap
                        if node.isShader {
                            viewModel.showShaderBrowser = true
                            viewModel.showSourceBrowser = false
                        } else {
                            viewModel.showSourceBrowser = true
                            viewModel.showShaderBrowser = false
                        }
                    }
                } label: {
                    Image(systemName: "arrow.triangle.2.circlepath")
                        .font(scaledFont(11))
                }
                .buttonStyle(.plain)
                .foregroundStyle(isSwapTarget ? Color.accentColor : Color.secondary)
                .help(node.isShader ? "Swap shader type" : "Swap source type")

                Spacer()

                Button {
                    viewModel.deleteNode(node.id)
                } label: {
                    Image(systemName: "trash")
                        .font(scaledFont(11))
                }
                .buttonStyle(.plain)
                .foregroundStyle(.red.opacity(0.7))
            }
            .padding(.horizontal, 10 * scale)
            .padding(.vertical, 4 * scale)
            .background(Color(.windowBackgroundColor).opacity(0.8))

            // Inline swap browser — expands from the node itself
            if isSwapTarget {
                NodeInlineBrowserView(
                    isShaderBrowser: viewModel.showShaderBrowser,
                    onSelect: { shaderType in
                        if node.isShader {
                            viewModel.swapShader(nodeId: node.id, newType: shaderType)
                        } else {
                            viewModel.swapSource(nodeId: node.id, newType: shaderType)
                        }
                        closeBrowser()
                    },
                    onCancel: {
                        closeBrowser()
                    }
                )
            }
        }
        .frame(width: nodeWidth)
        .clipShape(RoundedRectangle(cornerRadius: cornerRadius))
        .overlay(
            RoundedRectangle(cornerRadius: cornerRadius)
                .strokeBorder(borderColor, lineWidth: (isSelected ? 2 : 1) * scale)
        )
        .shadow(color: .black.opacity(0.3), radius: (isSelected ? 8 : 4) * scale, y: 2 * scale)
        .contentShape(RoundedRectangle(cornerRadius: cornerRadius))
        .accessibilityIdentifier("node-\(node.name)")
    }

    private func closeBrowser() {
        viewModel.showShaderBrowser = false
        viewModel.showSourceBrowser = false
        viewModel.browserMode = .add
        viewModel.swapTargetNodeId = nil
    }
}

// MARK: - Download Progress Overlay

/// Overlay shown on library source nodes while downloading.
private struct NodeDownloadOverlay: View {
    let progress: Float
    let isPaused: Bool

    private var percentText: String {
        "\(Int(progress * 100))%"
    }

    var body: some View {
        ZStack {
            Color.black.opacity(0.7)

            VStack(spacing: 8) {
                ZStack {
                    // Background ring
                    Circle()
                        .stroke(Color.white.opacity(0.2), lineWidth: 4)
                        .frame(width: 50, height: 50)

                    // Progress ring
                    Circle()
                        .trim(from: 0, to: CGFloat(progress))
                        .stroke(
                            isPaused ? Color.orange : Color.accentColor,
                            style: StrokeStyle(lineWidth: 4, lineCap: .round)
                        )
                        .frame(width: 50, height: 50)
                        .rotationEffect(.degrees(-90))

                    // Percentage
                    Text(percentText)
                        .font(.system(size: 14, weight: .semibold, design: .monospaced))
                        .foregroundStyle(.white)
                }

                Text(isPaused ? "Paused" : "Downloading...")
                    .font(.system(size: 11))
                    .foregroundStyle(.white.opacity(0.8))
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
                                .background(selectedCategory == cat ? Color.accentColor : Color(.controlBackgroundColor))
                                .foregroundStyle(selectedCategory == cat ? .white : .primary)
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

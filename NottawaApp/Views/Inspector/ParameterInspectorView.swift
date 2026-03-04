//
//  ParameterInspectorView.swift
//  NottawaApp
//
//  Right-side panel showing parameters for the selected node.
//

import SwiftUI

struct ParameterInspectorView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel
    @Environment(ThemeManager.self) private var theme

    // Timer for periodic refresh (oscillators/audio drivers change values)
    @State private var refreshTimer: Timer?

    private var selectedNode: NodeModel? {
        guard let id = viewModel.selectedNodeId else { return nil }
        return viewModel.nodes.first(where: { $0.id == id })
    }

    private var selectedNodes: [NodeModel] {
        viewModel.nodes.filter { viewModel.selectedNodeIds.contains($0.id) }
    }

    private var isMultiSelect: Bool {
        viewModel.selectedNodeIds.count > 1
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 0) {
            if isMultiSelect {
                multiSelectionHeader
            } else if let node = selectedNode {
                singleSelectionHeader(node: node)
            }

            // Parameters list (single selection only)
            if !isMultiSelect {
                ScrollView {
                    LazyVStack(alignment: .leading, spacing: 8) {
                        ForEach(viewModel.inspectorParameters.filter { $0.parameterType != .hidden }) { param in
                            ParameterControlView(param: param)
                        }

                        // Optional Shaders section
                        if let node = selectedNode, node.isShader {
                            OptionalShadersSection(nodeId: node.id)
                        }
                    }
                    .padding(16)
                }
            }
        }
        .frame(width: viewModel.inspectorWidth)
        .background(theme.colors.backgroundSecondary)
        .accessibilityIdentifier("parameter-inspector")
        .onAppear {
            startRefreshTimer()
        }
        .onDisappear {
            stopRefreshTimer()
        }
    }

    // MARK: - Single Selection Header

    @ViewBuilder
    private func singleSelectionHeader(node: NodeModel) -> some View {
        HStack {
            Circle()
                .fill(theme.colors.accent)
                .frame(width: 8, height: 8)
            Text(node.name)
                .font(.headline)
            Spacer()
            Button {
                viewModel.deselectAll()
            } label: {
                Image(systemName: "xmark.circle.fill")
                    .foregroundStyle(theme.colors.textSecondary)
            }
            .buttonStyle(.plain)
        }
        .padding(.horizontal, 16)
        .padding(.vertical, 12)

        theme.colors.border.frame(height: 1)

        // Live preview of node output
        TextureDisplayView(connectableId: node.id)
            .aspectRatio(16.0 / 9.0, contentMode: .fit)
            .clipShape(RoundedRectangle(cornerRadius: 6))
            .allowsHitTesting(false)
            .padding(.horizontal, 16)
            .padding(.vertical, 8)

        theme.colors.border.frame(height: 1)

        // Text source controls (text field, font size)
        if let textState = viewModel.textSourceState {
            TextSourceControlsView(state: textState, sourceId: node.id)
            theme.colors.border.frame(height: 1)
        }

        // File source playback controls (video files, library sources)
        if let fileState = viewModel.fileSourceState {
            FileSourceControlsView(state: fileState, sourceId: node.id)
            theme.colors.border.frame(height: 1)
        }
    }

    // MARK: - Multi Selection Header

    private var multiSelectionHeader: some View {
        VStack(alignment: .leading, spacing: 0) {
            HStack {
                Image(systemName: "checkmark.circle.fill")
                    .foregroundStyle(theme.colors.accent)
                Text("\(viewModel.selectedNodeIds.count) nodes selected")
                    .font(.headline)
                Spacer()
                Button {
                    viewModel.deselectAll()
                } label: {
                    Image(systemName: "xmark.circle.fill")
                        .foregroundStyle(theme.colors.textSecondary)
                }
                .buttonStyle(.plain)
            }
            .padding(.horizontal, 16)
            .padding(.vertical, 12)

            theme.colors.border.frame(height: 1)

            ScrollView {
                VStack(alignment: .leading, spacing: 4) {
                    ForEach(selectedNodes) { node in
                        HStack(spacing: 8) {
                            Circle()
                                .fill(theme.colors.accent)
                                .frame(width: 6, height: 6)
                            Text(node.name)
                                .font(.caption)
                                .lineLimit(1)
                            Spacer()
                            Text(node.isSource ? "Source" : "Effect")
                                .font(.caption2)
                                .foregroundStyle(.tertiary)
                        }
                        .padding(.vertical, 4)
                        .padding(.horizontal, 12)
                        .background(
                            RoundedRectangle(cornerRadius: 5)
                                .fill(theme.colors.surface.opacity(0.5))
                        )
                        .onTapGesture {
                            viewModel.selectNode(node.id)
                        }
                    }
                }
                .padding(16)
            }
        }
    }

    private func startRefreshTimer() {
        refreshTimer = Timer.scheduledTimer(withTimeInterval: 0.5, repeats: true) { _ in
            viewModel.refreshInspector()
        }
    }

    private func stopRefreshTimer() {
        refreshTimer?.invalidate()
        refreshTimer = nil
    }
}

// MARK: - Optional Shaders Section

struct OptionalShadersSection: View {
    @Environment(ThemeManager.self) private var theme
    let nodeId: String
    @State private var optionalShaders: [NottawaEngine.OptionalShaderInfo] = []
    @State private var expandedIndex: Int?

    var body: some View {
        Group {
            if !optionalShaders.isEmpty {
                theme.colors.border.frame(height: 1)
                    .padding(.vertical, 8)

                Text("Optional Effects")
                    .font(.subheadline)
                    .fontWeight(.semibold)
                    .foregroundStyle(theme.colors.textSecondary)

                ForEach(optionalShaders, id: \.index) { shader in
                    VStack(alignment: .leading, spacing: 4) {
                        HStack {
                            DSToggle(isOn: Binding(
                                get: { shader.enabled },
                                set: { _ in
                                    NottawaEngine.shared.toggleOptionalShader(connectableId: nodeId, index: shader.index)
                                    refreshOptionalShaders()
                                }
                            ), label: shader.name, style: .switch, size: .sm)

                            Spacer()

                            if shader.enabled {
                                Button {
                                    withAnimation {
                                        expandedIndex = expandedIndex == shader.index ? nil : shader.index
                                    }
                                } label: {
                                    Image(systemName: expandedIndex == shader.index ? "chevron.up" : "chevron.down")
                                        .font(.caption)
                                }
                                .buttonStyle(.plain)
                                .foregroundStyle(theme.colors.textSecondary)
                            }
                        }

                        if shader.enabled && expandedIndex == shader.index {
                            OptionalShaderParametersView(nodeId: nodeId, optionalIndex: shader.index)
                                .padding(.leading, 8)
                        }
                    }
                }
            }
        }
        .onAppear {
            refreshOptionalShaders()
        }
    }

    private func refreshOptionalShaders() {
        optionalShaders = NottawaEngine.shared.optionalShaders(for: nodeId)
    }
}

// MARK: - Optional Shader Parameters

struct OptionalShaderParametersView: View {
    let nodeId: String
    let optionalIndex: Int
    @State private var parameters: [ParameterInfo] = []

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            ForEach(parameters.filter { $0.parameterType != .hidden }) { param in
                ParameterControlView(param: param)
            }
        }
        .onAppear {
            parameters = NottawaEngine.shared.optionalShaderParameters(
                connectableId: nodeId,
                optionalIndex: optionalIndex
            )
        }
    }
}

//
//  ParameterInspectorView.swift
//  NottawaApp
//
//  Right-side panel showing parameters for the selected node.
//

import SwiftUI

struct ParameterInspectorView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel

    // Timer for periodic refresh (oscillators/audio drivers change values)
    @State private var refreshTimer: Timer?

    private var selectedNode: NodeModel? {
        guard let id = viewModel.selectedNodeId else { return nil }
        return viewModel.nodes.first(where: { $0.id == id })
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 0) {
            // Header
            if let node = selectedNode {
                HStack {
                    Circle()
                        .fill(node.isSource ? .blue : .purple)
                        .frame(width: 8, height: 8)
                    Text(node.name)
                        .font(.headline)
                    Spacer()
                    Button {
                        viewModel.deselectAll()
                    } label: {
                        Image(systemName: "xmark.circle.fill")
                            .foregroundStyle(.secondary)
                    }
                    .buttonStyle(.plain)
                }
                .padding(.horizontal, 16)
                .padding(.vertical, 12)

                Divider()

                // Live preview of node output
                TextureDisplayView(connectableId: node.id)
                    .aspectRatio(16.0 / 9.0, contentMode: .fit)
                    .clipShape(RoundedRectangle(cornerRadius: 6))
                    .allowsHitTesting(false)
                    .padding(.horizontal, 16)
                    .padding(.vertical, 8)

                Divider()

                // File source playback controls (video files, library sources)
                if let fileState = viewModel.fileSourceState {
                    FileSourceControlsView(state: fileState, sourceId: node.id)
                    Divider()
                }
            }

            // Parameters list
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
        .frame(width: viewModel.inspectorWidth)
        .background(.ultraThinMaterial)
        .accessibilityIdentifier("parameter-inspector")
        .onAppear {
            startRefreshTimer()
        }
        .onDisappear {
            stopRefreshTimer()
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
    let nodeId: String
    @State private var optionalShaders: [NottawaEngine.OptionalShaderInfo] = []
    @State private var expandedIndex: Int?

    var body: some View {
        Group {
            if !optionalShaders.isEmpty {
                Divider()
                    .padding(.vertical, 8)

                Text("Optional Effects")
                    .font(.subheadline)
                    .fontWeight(.semibold)
                    .foregroundStyle(.secondary)

                ForEach(optionalShaders, id: \.index) { shader in
                    VStack(alignment: .leading, spacing: 4) {
                        HStack {
                            Toggle(shader.name, isOn: Binding(
                                get: { shader.enabled },
                                set: { _ in
                                    NottawaEngine.shared.toggleOptionalShader(connectableId: nodeId, index: shader.index)
                                    refreshOptionalShaders()
                                }
                            ))
                            .toggleStyle(.switch)
                            .controlSize(.small)

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
                                .foregroundStyle(.secondary)
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

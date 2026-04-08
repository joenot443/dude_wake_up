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
    @Environment(HelpGuideViewModel.self) private var helpGuide

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
                    VStack(alignment: .leading, spacing: 8) {
                        // Clear Feedback Buffer button (Feedback shader only)
                        if let node = selectedNode, isFeedbackShader(node) {
                            Button {
                                NottawaEngine.shared.clearFeedbackBuffer(shaderId: node.id)
                            } label: {
                                Label("Clear Feedback Buffer", systemImage: "trash")
                                    .font(.caption)
                                    .frame(maxWidth: .infinity)
                            }
                            .buttonStyle(.bordered)
                            .tint(theme.colors.accent)
                        }

                        ForEach(viewModel.inspectorParameters.filter { $0.parameterType != .hidden }) { param in
                            ParameterControlView(param: param)
                        }

                        // Optional Shaders section
                        if let node = selectedNode {
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

    private var isSelectedNodeBlend: Bool {
        guard let node = selectedNode,
              case .shader(let raw) = node.nodeType else { return false }
        return raw == 80
    }

    private func isFeedbackShader(_ node: NodeModel) -> Bool {
        if case .shader(let raw) = node.nodeType { return raw == 7 }
        return false
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
                NottawaEngine.shared.resetAllParameters(connectableId: node.id)
                viewModel.refreshInspector()
            } label: {
                Image(systemName: "arrow.counterclockwise")
                    .foregroundStyle(theme.colors.textSecondary)
            }
            .buttonStyle(.plainHitArea)
            .help("Reset all parameters")
            Button {
                viewModel.deselectAll()
            } label: {
                Image(systemName: "xmark.circle.fill")
                    .foregroundStyle(theme.colors.textSecondary)
            }
            .buttonStyle(.plainHitArea)
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

        // Icon source controls (icon picker grid)
        if let iconState = viewModel.iconSourceState {
            IconSourceControlsView(state: iconState, sourceId: node.id)
            theme.colors.border.frame(height: 1)
        }

        // File source playback controls (video files, library sources)
        if let fileState = viewModel.fileSourceState {
            FileSourceControlsView(state: fileState, sourceId: node.id)
            theme.colors.border.frame(height: 1)
        }

        // Position pad for shaders with X/Y parameters (e.g. Feedback)
        if let xParam = viewModel.inspectorParameters.first(where: { $0.name == "X" }),
           let yParam = viewModel.inspectorParameters.first(where: { $0.name == "Y" }),
           viewModel.textSourceState == nil {
            ParameterPositionPadView(
                xParam: xParam,
                yParam: yParam,
                connectableId: node.id
            )
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
                .buttonStyle(.plainHitArea)
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

    @State private var initialBlendMode: Int?
    @State private var initialAlpha: Float?

    private func startRefreshTimer() {
        refreshTimer = Timer.scheduledTimer(withTimeInterval: 0.1, repeats: true) { _ in
            viewModel.refreshInspector()
            NotificationCenter.default.post(name: .parameterRefresh, object: nil)
            checkHelpProgress()
        }
    }

    private func checkHelpProgress() {
        guard helpGuide.isActive, isSelectedNodeBlend else { return }
        let params = viewModel.inspectorParameters

        // Capture initial values on first check
        if initialBlendMode == nil, let mode = params.first(where: { $0.name.lowercased().contains("mode") }) {
            initialBlendMode = mode.intValue
        }
        if initialAlpha == nil, let alpha = params.first(where: { $0.name.lowercased().contains("alpha") }) {
            initialAlpha = alpha.value
        }

        // Detect blend mode changed from initial
        if let initial = initialBlendMode,
           let mode = params.first(where: { $0.name.lowercased().contains("mode") }),
           mode.intValue != initial {
            helpGuide.notifyBlendModeEdited()
        }

        // Detect alpha changed from initial
        if let initial = initialAlpha,
           let alpha = params.first(where: { $0.name.lowercased().contains("alpha") }),
           abs(alpha.value - initial) > 0.01 {
            helpGuide.notifyAlphaEdited()
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
    @Environment(NodeEditorViewModel.self) private var viewModel
    let nodeId: String
    @State private var optionalShaders: [NottawaEngine.OptionalShaderInfo] = []
    @State private var expandedIndex: Int?

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
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
                                    // Auto-expand when enabling, collapse when disabling
                                    withAnimation {
                                        if !shader.enabled {
                                            expandedIndex = shader.index
                                        } else if expandedIndex == shader.index {
                                            expandedIndex = nil
                                        }
                                    }
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
                                .buttonStyle(.plainHitArea)
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
        .onChange(of: viewModel.selectedNodeId) { _, _ in
            expandedIndex = nil
            refreshOptionalShaders()
        }
    }

    private func refreshOptionalShaders() {
        optionalShaders = NottawaEngine.shared.optionalShaders(for: nodeId)
    }
}

// MARK: - Parameter Position Pad

/// Position pad for shaders with X/Y parameters (e.g. Feedback).
/// Maps between the parameter's value range and the pad's 0–1 visual range.
struct ParameterPositionPadView: View {
    @Environment(ThemeManager.self) private var theme
    let xParam: ParameterInfo
    let yParam: ParameterInfo
    let connectableId: String

    private let engine = NottawaEngine.shared

    @State private var padX: Float
    @State private var padY: Float

    init(xParam: ParameterInfo, yParam: ParameterInfo, connectableId: String) {
        self.xParam = xParam
        self.yParam = yParam
        self.connectableId = connectableId
        self._padX = State(initialValue: Self.toNormalized(xParam.value, min: xParam.minValue, max: xParam.maxValue))
        self._padY = State(initialValue: Self.toNormalized(yParam.value, min: yParam.minValue, max: yParam.maxValue))
    }

    private static func toNormalized(_ value: Float, min: Float, max: Float) -> Float {
        (value - min) / (max - min)
    }

    private static func fromNormalized(_ norm: Float, min: Float, max: Float) -> Float {
        norm * (max - min) + min
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            HStack {
                Text("Position")
                    .font(.caption)
                    .foregroundStyle(theme.colors.textSecondary)
                Spacer()
                Text(String(format: "%.2f, %.2f",
                            Self.fromNormalized(padX, min: xParam.minValue, max: xParam.maxValue),
                            Self.fromNormalized(padY, min: yParam.minValue, max: yParam.maxValue)))
                    .font(.caption)
                    .foregroundStyle(theme.colors.textTertiary)
                    .monospacedDigit()
            }

            PositionPadView(x: $padX, y: $padY, connectableId: connectableId)
                .onChange(of: padX) { _, newValue in
                    engine.setParameterValue(paramId: xParam.id, value: Self.fromNormalized(newValue, min: xParam.minValue, max: xParam.maxValue))
                }
                .onChange(of: padY) { _, newValue in
                    engine.setParameterValue(paramId: yParam.id, value: Self.fromNormalized(newValue, min: yParam.minValue, max: yParam.maxValue))
                }
        }
        .padding(.horizontal, 16)
        .padding(.vertical, 8)
        .onChange(of: xParam.value) { _, newValue in
            let norm = Self.toNormalized(newValue, min: xParam.minValue, max: xParam.maxValue)
            if padX != norm { padX = norm }
        }
        .onChange(of: yParam.value) { _, newValue in
            let norm = Self.toNormalized(newValue, min: yParam.minValue, max: yParam.maxValue)
            if padY != norm { padY = norm }
        }
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

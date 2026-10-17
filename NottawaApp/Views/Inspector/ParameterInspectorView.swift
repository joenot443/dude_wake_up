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
                singleSelectionTitleBar(node: node)
            }

            // Parameters list (single selection only)
            if !isMultiSelect {
                ScrollView {
                    VStack(alignment: .leading, spacing: 0) {
                        if let node = selectedNode {
                            singleSelectionScrollableContent(node: node)
                        }

                        VStack(alignment: .leading, spacing: 8) {
                        // Edit shader code (shader nodes + shader-source generators).
                        if let node = selectedNode, node.hasEditableShader {
                            editShaderCodeButton(node: node)
                        }

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

                        // Reset Position button (Bounce shader only)
                        if let node = selectedNode, isBounceShader(node) {
                            Button {
                                NottawaEngine.shared.resetBouncePosition(shaderId: node.id)
                            } label: {
                                Label("Reset Position", systemImage: "arrow.counterclockwise")
                                    .font(.caption)
                                    .frame(maxWidth: .infinity)
                            }
                            .buttonStyle(.bordered)
                            .tint(theme.colors.accent)
                        }

                        // Crop / Place pads — for any node that carries the
                        // SourceTransform pipeline (TransformShader,
                        // WebcamSource, etc.).
                        if let node = selectedNode,
                           nodeHasTransformPipeline,
                           let minX = viewModel.inspectorParameters.first(where: { $0.name == "Min X" }),
                           let maxX = viewModel.inspectorParameters.first(where: { $0.name == "Max X" }),
                           let minY = viewModel.inspectorParameters.first(where: { $0.name == "Min Y" }),
                           let maxY = viewModel.inspectorParameters.first(where: { $0.name == "Max Y" }) {
                            // Resolve the input frame for the Crop pad
                            // backdrop. Shaders read it from their upstream
                            // connection; sources expose their own pre-
                            // transform feed under "sourceRaw:<id>".
                            let inputId: String? = node.isSource
                                ? "sourceRaw:\(node.id)"
                                : viewModel.connections
                                    .first(where: { $0.endNodeId == node.id && $0.inputSlot == 0 })?
                                    .startNodeId
                            TransformCropPadView(
                                nodeId: node.id,
                                inputConnectableId: inputId,
                                minXParam: minX,
                                maxXParam: maxX,
                                minYParam: minY,
                                maxYParam: maxY
                            )

                            // Place pad — only in Standard mode. Auto-fit
                            // modes (Fill / Fit / Fill+Aspect) compute
                            // placement from aspect ratios, so manual
                            // controls would be inert.
                            if let mode = viewModel.inspectorParameters.first(where: { $0.name == "Mode" }),
                               mode.intValue == 0,
                               let xParam = viewModel.inspectorParameters.first(where: { $0.name == "X" }),
                               let yParam = viewModel.inspectorParameters.first(where: { $0.name == "Y" }),
                               let scaleParam = viewModel.inspectorParameters.first(where: { $0.name == "Scale" }) {
                                TransformPlacePadView(
                                    nodeId: node.id,
                                    translateXParam: xParam,
                                    translateYParam: yParam,
                                    scaleParam: scaleParam,
                                    cropWidthFrac: max(0, maxX.value - minX.value),
                                    cropHeightFrac: max(0, maxY.value - minY.value)
                                )
                            }
                        }

                        ForEach(viewModel.inspectorParameters.filter { param in
                            guard param.parameterType != .hidden else { return false }
                            // Shader-controlled runtime hide (e.g. Blend hides unconnected input strengths).
                            if param.hiddenFromNode { return false }
                            // Text sources render their own position pad in TextSourceControlsView,
                            // so hide the duplicate X/Y params (and their pad) from the list.
                            if viewModel.textSourceState != nil,
                               isPositionXParam(param) || isPositionYParam(param) {
                                return false
                            }
                            // FeedbackAuxStrandView replaces the `Allow Auxillary`
                            // checkbox for shaders that surface an aux pin.
                            // Spelled with the C++ side's typo (FeedbackShader.hpp:100).
                            if let node = selectedNode,
                               node.supportsAuxOutput,
                               param.name == "Allow Auxillary" {
                                return false
                            }
                            // VideoSourceSettings params (width/height/start/end)
                            // are internal plumbing — never useful to expose.
                            if ["width", "height", "start", "end"].contains(param.name) {
                                return false
                            }
                            // Icon sources expose their picker via
                            // IconSourceControlsView — hide the underlying
                            // icon/texture parameters from the list.
                            if viewModel.iconSourceState != nil,
                               ["icon", "texture", "Enable Texture"].contains(param.name) {
                                return false
                            }
                            // Transform crop params are driven by the Crop pad.
                            if nodeHasTransformPipeline,
                               Self.transformCropParamNames.contains(param.name) {
                                return false
                            }
                            // Transform place / scale / center params are
                            // driven by TransformPlacePadView (Standard mode)
                            // or are inert (auto-fit modes).
                            if nodeHasTransformPipeline,
                               Self.transformPlaceParamNames.contains(param.name) {
                                return false
                            }
                            return !isPositionYParam(param)
                        }) { param in
                            if isPositionXParam(param), let yParam = matchingYParam(for: param) {
                                PositionPadControl(
                                    xParam: param,
                                    yParam: yParam,
                                    connectableId: selectedNode?.id
                                )
                            } else {
                                ParameterControlView(param: param)
                            }
                        }

                        // Auxiliary Strand section — for FeedbackShader and the
                        // mask-maker shaders that surface an aux output pin.
                        if let node = selectedNode, node.supportsAuxOutput {
                            FeedbackAuxStrandView(parentNodeId: node.id)
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
        }
        .frame(width: viewModel.inspectorWidth)
        .background(theme.colors.backgroundSecondary)
        .accessibilityIdentifier("parameter-inspector")
        .onPreferenceChange(InspectorTargetFrameKey.self) { frame in
            helpGuide.inspectorTargetFrame = frame
        }
        .onAppear {
            startRefreshTimer()
        }
        .onDisappear {
            stopRefreshTimer()
        }
    }

    // MARK: - Position Pad Detection

    private func isPositionXParam(_ param: ParameterInfo) -> Bool {
        if param.name == "Orb X" { return true }
        return (param.name == "X Position" || param.name == "X") && param.minValue >= 0 && param.maxValue <= 1.0
    }

    private func isPositionYParam(_ param: ParameterInfo) -> Bool {
        if param.name == "Orb Y" { return true }
        return (param.name == "Y Position" || param.name == "Y") && param.minValue >= 0 && param.maxValue <= 1.0
    }

    private func matchingYParam(for xParam: ParameterInfo) -> ParameterInfo? {
        let yName: String
        let requireUnitRange: Bool
        switch xParam.name {
        case "X": yName = "Y"; requireUnitRange = true
        case "X Position": yName = "Y Position"; requireUnitRange = true
        case "Orb X": yName = "Orb Y"; requireUnitRange = false
        default: return nil
        }
        return viewModel.inspectorParameters.first {
            guard $0.name == yName && $0.parameterType != .hidden else { return false }
            if requireUnitRange { return $0.minValue >= 0 && $0.maxValue <= 1.0 }
            return true
        }
    }

    private var isSelectedNodeBlend: Bool {
        guard let node = selectedNode,
              case .shader(let raw) = node.nodeType else { return false }
        return raw == 80
    }

    private var isSelectedNodeMaskEngine: Bool {
        guard let node = selectedNode,
              case .shader(let raw) = node.nodeType else { return false }
        return raw == 182 // ShaderTypeMaskEngine
    }

    private func isFeedbackShader(_ node: NodeModel) -> Bool {
        if case .shader(let raw) = node.nodeType { return raw == 7 }
        return false
    }

    private func isBounceShader(_ node: NodeModel) -> Bool {
        if case .shader(let raw) = node.nodeType { return raw == 58 }
        return false
    }

    /// Opens the live GLSL editor for the given shader. A small "modified"
    /// pill renders alongside the button when the shader currently has a
    /// session override active.
    @ViewBuilder
    private func editShaderCodeButton(node: NodeModel) -> some View {
        let hasOverride = NottawaEngine.shared.shaderHasSessionOverride(shaderId: node.id)
        HStack(spacing: 6) {
            Button {
                viewModel.shaderCodeEditorTarget = ShaderCodeEditorTarget(
                    id: node.id, name: node.name
                )
            } label: {
                Label("Edit Shader Code…", systemImage: "chevron.left.forwardslash.chevron.right")
                    .font(.caption)
                    .frame(maxWidth: .infinity)
            }
            .buttonStyle(.bordered)
            .tint(theme.colors.accent)
            .help("Edit this shader's fragment GLSL for the current session")

            if hasOverride {
                Text("MOD")
                    .font(.caption2.weight(.bold).monospaced())
                    .foregroundStyle(theme.colors.warning)
                    .padding(.horizontal, 5)
                    .padding(.vertical, 2)
                    .overlay(
                        RoundedRectangle(cornerRadius: 3)
                            .stroke(theme.colors.warning, lineWidth: 1)
                    )
                    .help("This shader has a session-only edit. Open the editor to revert.")
            }
        }
    }

    private static let transformCropParamNames: Set<String> = [
        "Min X", "Max X", "Min Y", "Max Y"
    ]

    /// Translate / center params are driven by TransformPlacePadView in
    /// Standard mode (and are no-ops in auto-fit modes) — always hide them.
    /// `Scale` is intentionally not in this list: it applies in *every* mode
    /// (as a uniform multiplier in auto-fit modes), and the param-list
    /// slider serves as the only way to edit it when the Place pad is
    /// hidden. In Standard mode the slider mirrors the pad's corner drag.
    private static let transformPlaceParamNames: Set<String> = [
        "X", "Y", "Center", "Scale From Center"
    ]

    /// True iff the currently selected node carries the SourceTransform
    /// pipeline — i.e. has all of Min X / Max X / Min Y / Max Y / X / Y /
    /// Scale / Mode. Used to drive the dedicated Crop / Place pads and to
    /// suppress the underlying parameter sliders that the pads replace.
    /// Works for any node (TransformShader, WebcamSource, future sources).
    private var nodeHasTransformPipeline: Bool {
        let required: Set<String> = [
            "Min X", "Max X", "Min Y", "Max Y", "X", "Y", "Scale", "Mode"
        ]
        let present = Set(viewModel.inspectorParameters.map(\.name))
        return required.isSubset(of: present)
    }

    // MARK: - Single Selection Header

    @ViewBuilder
    private func singleSelectionTitleBar(node: NodeModel) -> some View {
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
    }

    @ViewBuilder
    private func singleSelectionScrollableContent(node: NodeModel) -> some View {
        // Live preview of node output
        TextureDisplayView(connectableId: node.id)
            .aspectRatio(16.0 / 9.0, contentMode: .fit)
            .clipShape(RoundedRectangle(cornerRadius: 6))
            .allowsHitTesting(false)
            .padding(.horizontal, 16)
            .padding(.vertical, 8)

        theme.colors.border.frame(height: 1)

        // Mask shader: preset pane (Luma / Color Key / Match Blend) drives the
        // Source Mode; mode-specific params are shown/hidden via hiddenFromNode.
        if isSelectedNodeMaskEngine,
           let sourceMode = viewModel.inspectorParameters.first(where: { $0.name == "Source Mode" }) {
            MaskEnginePresetPaneView(
                sourceModeParam: sourceMode,
                outputModeParam: viewModel.inspectorParameters.first(where: { $0.name == "Output Mode" })
            )
            theme.colors.border.frame(height: 1)
        }

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

        // Position pad for shaders with X/Y parameters (e.g. Feedback).
        // Skip when the node uses the SourceTransform pipeline — that has
        // its own Place pad covering X/Y/Scale.
        if let xParam = viewModel.inspectorParameters.first(where: { $0.name == "X" }),
           let yParam = viewModel.inspectorParameters.first(where: { $0.name == "Y" }),
           viewModel.textSourceState == nil,
           !nodeHasTransformPipeline {
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
    @State private var initialMainStrength: Float?

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
        if initialMainStrength == nil, let strength = params.first(where: { $0.name == "Main Input Strength" }) {
            initialMainStrength = strength.value
        }

        // Detect blend mode changed from initial
        if let initial = initialBlendMode,
           let mode = params.first(where: { $0.name.lowercased().contains("mode") }),
           mode.intValue != initial {
            helpGuide.notifyBlendModeEdited()
        }

        // Detect the Main Input Strength slider changed from initial
        if let initial = initialMainStrength,
           let strength = params.first(where: { $0.name == "Main Input Strength" }),
           abs(strength.value - initial) > 0.01 {
            helpGuide.notifyAlphaEdited()
        }
    }

    private func stopRefreshTimer() {
        refreshTimer?.invalidate()
        refreshTimer = nil
    }
}

// MARK: - Mask Engine Preset Pane

/// Three-card preset selector for the unified Mask shader. Sets the Source
/// Mode (and a sensible default Output Mode) via the bridge; the C++ shader's
/// `hiddenFromNode` flags then reveal the params relevant to the chosen mode.
private struct MaskEnginePresetPaneView: View {
    @Environment(ThemeManager.self) private var theme
    @Environment(NodeEditorViewModel.self) private var viewModel

    let sourceModeParam: ParameterInfo
    let outputModeParam: ParameterInfo?

    private let presets = ["Luma", "Color Key", "Match Blend"]

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("Preset")
                .font(.caption)
                .foregroundStyle(theme.colors.textSecondary)
            HStack(spacing: 6) {
                ForEach(Array(presets.enumerated()), id: \.offset) { idx, label in
                    DSButton(
                        label,
                        variant: sourceModeParam.intValue == idx ? .primary : .secondary,
                        size: .sm,
                        fullWidth: true
                    ) {
                        applyPreset(idx)
                    }
                }
            }
        }
        .padding(.horizontal, 16)
        .padding(.vertical, 8)
    }

    private func applyPreset(_ idx: Int) {
        NottawaEngine.shared.setParameterInt(paramId: sourceModeParam.id, value: idx)
        // Default output: Blend (1) for Match Blend, Mask (0) otherwise.
        if let outputModeParam {
            NottawaEngine.shared.setParameterInt(paramId: outputModeParam.id, value: idx == 2 ? 1 : 0)
        }
        viewModel.refreshInspector()
    }
}

// MARK: - Optional Shaders Section

struct OptionalShadersSection: View {
    @Environment(ThemeManager.self) private var theme
    @Environment(NodeEditorViewModel.self) private var viewModel
    let nodeId: String
    @State private var optionalShaders: [NottawaEngine.OptionalShaderInfo] = []
    @State private var expandedIndex: Int?

    private var anyEnabled: Bool {
        optionalShaders.contains(where: { $0.enabled })
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            if !optionalShaders.isEmpty {
                theme.colors.border.frame(height: 1)
                    .padding(.vertical, 8)

                HStack(spacing: 6) {
                    Text("Optional Effects")
                        .font(.subheadline)
                        .fontWeight(.semibold)
                        .foregroundStyle(theme.colors.textSecondary)
                    Spacer()
                    if anyEnabled {
                        Button {
                            for shader in optionalShaders where shader.enabled {
                                NottawaEngine.shared.toggleOptionalShader(
                                    connectableId: nodeId, index: shader.index
                                )
                            }
                            refreshOptionalShaders()
                            expandedIndex = nil
                        } label: {
                            Image(systemName: "arrow.counterclockwise")
                                .font(.caption2)
                                .foregroundStyle(theme.colors.textTertiary)
                        }
                        .buttonStyle(.plainHitArea)
                        .help("Disable all optional effects")
                    }
                }

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
        // Invert Y: pad Y=0 is top (screen), but GL Y increases upward
        self._padY = State(initialValue: 1.0 - Self.toNormalized(yParam.value, min: yParam.minValue, max: yParam.maxValue))
    }

    private static func toNormalized(_ value: Float, min: Float, max: Float) -> Float {
        (value - min) / (max - min)
    }

    private static func fromNormalized(_ norm: Float, min: Float, max: Float) -> Float {
        norm * (max - min) + min
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("Position")
                .font(.caption)
                .foregroundStyle(theme.colors.textSecondary)

            PositionPadView(x: $padX, y: $padY, connectableId: connectableId)
                .onChange(of: padX) { _, newValue in
                    engine.setParameterValue(paramId: xParam.id, value: Self.fromNormalized(newValue, min: xParam.minValue, max: xParam.maxValue))
                }
                .onChange(of: padY) { _, newValue in
                    engine.setParameterValue(paramId: yParam.id, value: Self.fromNormalized(1.0 - newValue, min: yParam.minValue, max: yParam.maxValue))
                }

            PositionAxisReactivityRow(
                label: "X",
                value: Self.fromNormalized(padX, min: xParam.minValue, max: xParam.maxValue),
                param: xParam
            )
            PositionAxisReactivityRow(
                label: "Y",
                value: Self.fromNormalized(padY, min: yParam.minValue, max: yParam.maxValue),
                param: yParam
            )
        }
        .padding(.horizontal, 16)
        .padding(.vertical, 8)
        .onChange(of: xParam.value) { _, newValue in
            let norm = Self.toNormalized(newValue, min: xParam.minValue, max: xParam.maxValue)
            if padX != norm { padX = norm }
        }
        .onChange(of: yParam.value) { _, newValue in
            let norm = 1.0 - Self.toNormalized(newValue, min: yParam.minValue, max: yParam.maxValue)
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

// MARK: - Position Axis Reactivity Row

/// Per-axis readout + reactivity buttons (oscillator / audio driver / MIDI)
/// for the 2D position pads. Mirrors ParameterControlView's button row +
/// expandable panel pattern so the X and Y bindings of a position pad can
/// be modulated the same way any other parameter can be.
private struct PositionAxisReactivityRow: View {
    @Environment(NodeEditorViewModel.self) private var viewModel
    @Environment(ThemeManager.self) private var theme

    let label: String
    let value: Float
    let param: ParameterInfo

    @State private var showOscillator = false
    @State private var showAudioDriver: Bool
    @State private var showMidi: Bool
    @State private var oscEnabled: Bool
    @State private var driverActive: Bool
    @State private var midiActive: Bool

    init(label: String, value: Float, param: ParameterInfo) {
        self.label = label
        self.value = value
        self.param = param
        self._oscEnabled = State(initialValue: param.oscillatorEnabled)
        self._driverActive = State(initialValue: param.hasDriver)
        self._midiActive = State(initialValue: param.hasMidiBinding)
        // Start the audio / MIDI panels open if the param is already
        // bound — otherwise the toggle button looks broken (panel was
        // already forced visible by `|| midiActive`, so clicking did
        // nothing visible).
        self._showAudioDriver = State(initialValue: param.hasDriver)
        self._showMidi = State(initialValue: param.hasMidiBinding)
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 4) {
            HStack(spacing: 6) {
                Text("\(label): \(String(format: "%.2f", value))")
                    .font(.caption2)
                    .foregroundStyle(theme.colors.textTertiary)
                    .monospacedDigit()
                Spacer()
                if param.hasOscillator {
                    Button {
                        withAnimation(.easeInOut(duration: 0.2)) {
                            showOscillator.toggle()
                            if showOscillator {
                                showAudioDriver = false
                                showMidi = false
                                if !oscEnabled {
                                    oscEnabled = true
                                    NottawaEngine.shared.setOscillatorEnabled(paramId: param.id, enabled: true)
                                }
                            }
                        }
                    } label: {
                        Image(systemName: oscEnabled ? "waveform.circle.fill" : "waveform.circle")
                            .font(.caption2)
                            .foregroundStyle(oscEnabled ? theme.colors.accent : theme.colors.textSecondary)
                    }
                    .buttonStyle(.plainHitArea)
                    .accessibilityIdentifier("osc-toggle-\(param.id)")
                }
                Button {
                    withAnimation(.easeInOut(duration: 0.2)) {
                        showAudioDriver.toggle()
                        if showAudioDriver { showOscillator = false; showMidi = false }
                    }
                } label: {
                    Image(systemName: driverActive ? "music.note.list" : "music.note")
                        .font(.caption2)
                        .foregroundStyle(driverActive ? theme.colors.warning : theme.colors.textSecondary)
                }
                .buttonStyle(.plainHitArea)
                .accessibilityIdentifier("driver-toggle-\(param.id)")
                Button {
                    withAnimation(.easeInOut(duration: 0.2)) {
                        showMidi.toggle()
                        if showMidi { showOscillator = false; showAudioDriver = false }
                    }
                } label: {
                    Image(systemName: midiActive ? "pianokeys.inverse" : "pianokeys")
                        .font(.caption2)
                        .foregroundStyle(midiActive ? .purple : theme.colors.textSecondary)
                }
                .buttonStyle(.plainHitArea)
                .accessibilityIdentifier("midi-toggle-\(param.id)")
            }

            if showOscillator && param.hasOscillator {
                OscillatorControlsView(param: param, enabled: $oscEnabled)
                    .transition(.opacity.combined(with: .move(edge: .top)))
            }
            if showAudioDriver {
                AudioDriverControlsView(param: param)
                    .transition(.opacity.combined(with: .move(edge: .top)))
            }
            if showMidi {
                MidiControlsView(param: param)
                    .transition(.opacity.combined(with: .move(edge: .top)))
            }
        }
        .onReceive(NotificationCenter.default.publisher(for: .parameterRefresh)) { _ in
            guard let fresh = viewModel.inspectorParameters.first(where: { $0.id == param.id }) else { return }
            if fresh.oscillatorEnabled != oscEnabled { oscEnabled = fresh.oscillatorEnabled }
            if fresh.hasDriver != driverActive { driverActive = fresh.hasDriver }
            if fresh.hasMidiBinding != midiActive { midiActive = fresh.hasMidiBinding }
        }
    }
}

// MARK: - Position Pad Control

/// Wraps PositionPadView for use with ParameterInfo X/Y pairs in the inspector.
private struct PositionPadControl: View {
    @Environment(ThemeManager.self) private var theme
    let xParam: ParameterInfo
    let yParam: ParameterInfo
    let connectableId: String?

    @State private var xValue: Float
    @State private var yValue: Float

    init(xParam: ParameterInfo, yParam: ParameterInfo, connectableId: String?) {
        self.xParam = xParam
        self.yParam = yParam
        self.connectableId = connectableId
        self._xValue = State(initialValue: xParam.value)
        self._yValue = State(initialValue: yParam.value)
    }

    private func normalize(_ value: Float, _ param: ParameterInfo) -> Float {
        let range = param.maxValue - param.minValue
        guard range > 0 else { return 0 }
        return (value - param.minValue) / range
    }

    private func denormalize(_ normalized: Float, _ param: ParameterInfo) -> Float {
        return param.minValue + normalized * (param.maxValue - param.minValue)
    }

    /// Orb-style params (Electric Eels) feed straight into screen-space GLSL
    /// math, so the pad's Cartesian flip would reverse them. Skip the flip
    /// for those — drag up = orbY decreases = orb appears at top.
    private var invertY: Bool {
        yParam.name != "Orb Y"
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            Text("Position")
                .font(.caption)
                .foregroundStyle(theme.colors.textSecondary)

            PositionPadView(
                x: Binding(
                    get: { normalize(xValue, xParam) },
                    set: { xValue = denormalize($0, xParam) }
                ),
                y: Binding(
                    get: { invertY ? 1.0 - normalize(yValue, yParam) : normalize(yValue, yParam) },
                    set: { yValue = denormalize(invertY ? 1.0 - $0 : $0, yParam) }
                ),
                connectableId: connectableId
            )
                .onChange(of: xValue) { _, newValue in
                    NottawaEngine.shared.setParameterValue(paramId: xParam.id, value: newValue)
                }
                .onChange(of: yValue) { _, newValue in
                    NottawaEngine.shared.setParameterValue(paramId: yParam.id, value: newValue)
                }

            PositionAxisReactivityRow(label: "X", value: xValue, param: xParam)
            PositionAxisReactivityRow(label: "Y", value: yValue, param: yParam)
        }
    }
}

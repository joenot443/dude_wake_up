//
//  HelpGuideViewModel.swift
//  NottawaApp
//
//  Guided help tutorial state machine. Evaluates the current step
//  from NodeEditorViewModel graph state.
//

import Foundation
import SwiftUI

enum HelpStep: Int, CaseIterable {
    case placeSource
    case placeEffect
    case connectSourceToEffect
    case placeSecondSource
    case blendViaOutputs
    case openInspector
    case editBlendMode
    case editAlpha
    case actionButtonsTour
    case shareHelp
    case completion
}

@Observable
final class HelpGuideViewModel {
    // Reference to the node editor for reading graph state
    weak var viewModel: NodeEditorViewModel?

    // Sticky flags (persist across graph changes)
    var hasEditedBlendMode = false
    var hasEditedAlpha = false
    var hasSeenActionButtons = false
    var hasSeenShareHelp = false

    // Persisted completion
    var completed: Bool {
        didSet {
            UserDefaults.standard.set(completed, forKey: "nottawa_help_completed")
        }
    }

    init() {
        self.completed = UserDefaults.standard.bool(forKey: "nottawa_help_completed")
    }

    func reset() {
        hasEditedBlendMode = false
        hasEditedAlpha = false
        hasSeenActionButtons = false
        hasSeenShareHelp = false
        completed = false
    }

    /// Notify that a blend parameter was edited (called from inspector interactions).
    func notifyBlendModeEdited() { hasEditedBlendMode = true }
    func notifyAlphaEdited() { hasEditedAlpha = true }

    // MARK: - Computed Step

    var currentStep: HelpStep {
        guard let vm = viewModel else { return .placeSource }

        let sources = vm.nodes.filter { $0.isSource }
        let shaders = vm.nodes.filter { $0.isShader }
        let connections = vm.connections

        // Step 1: Place a source
        if sources.isEmpty { return .placeSource }

        // Step 2: Place an effect
        if shaders.isEmpty { return .placeEffect }

        // Step 3: Connect source to effect
        if connections.isEmpty { return .connectSourceToEffect }

        // Step 4: Place a second source
        if sources.count < 2 { return .placeSecondSource }

        // Step 5: Create a blend by connecting two output pins
        let blendNodes = shaders.filter { isBlendNode($0) }
        if blendNodes.isEmpty { return .blendViaOutputs }

        // Step 6-8: Inspect and edit the auto-created blend
        if let blend = blendNodes.first {
            if !vm.selectedNodeIds.contains(blend.id) { return .openInspector }
            if !hasEditedBlendMode { return .editBlendMode }
            if !hasEditedAlpha { return .editAlpha }
        }

        // Step 9: Action buttons tour
        if !hasSeenActionButtons { return .actionButtonsTour }

        // Step 10: Share help
        if !hasSeenShareHelp { return .shareHelp }

        // Step 11: Completion
        return .completion
    }

    var isActive: Bool {
        guard let vm = viewModel else { return false }
        return vm.helpEnabled && !completed
    }

    /// The sidebar tab the help guide requires for the current step, or nil if no preference.
    var requiredSidebarTab: NodeEditorViewModel.SidebarTab? {
        guard isActive else { return nil }
        switch currentStep {
        case .placeSource, .placeSecondSource:
            return .sources
        case .placeEffect:
            return .shaders
        default:
            return nil
        }
    }

    // MARK: - Per-Node Tooltip

    enum TooltipSide { case leading, trailing, below }

    struct NodeTooltip {
        let title: String
        let bodyText: String?
        let side: TooltipSide
    }

    /// Returns the help tooltip a given node should display, if any.
    func tooltip(for node: NodeModel) -> NodeTooltip? {
        guard isActive, let vm = viewModel else { return nil }
        let step = currentStep
        let isDragging = vm.dragConnection != nil

        switch step {
        case .connectSourceToEffect:
            if !isDragging, node.isSource, node.id == vm.nodes.first(where: { $0.isSource })?.id {
                // "Drag from here" — arrow points at output (trailing side)
                return NodeTooltip(title: HelpStrings.connectSourceEffectTitle, bodyText: HelpStrings.connectInstruction, side: .trailing)
            }
            if isDragging, node.isShader, !isBlendNode(node), node.id == vm.nodes.first(where: { $0.isShader && !isBlendNode($0) })?.id {
                // "Drop here" — arrow points at input (leading side)
                return NodeTooltip(title: HelpStrings.completeConnectionTitle, bodyText: HelpStrings.dragToEffectInstruction, side: .leading)
            }

        case .blendViaOutputs:
            if !isDragging, firstUnconnectedNodeId(vm) == node.id {
                return NodeTooltip(title: HelpStrings.blendViaOutputsTitle, bodyText: HelpStrings.blendViaOutputsInstruction, side: .trailing)
            }
            if isDragging, secondUnconnectedNodeId(vm) == node.id {
                return NodeTooltip(title: HelpStrings.blendViaOutputsDrop, bodyText: nil, side: .trailing)
            }

        case .openInspector:
            if isBlendNode(node) {
                return NodeTooltip(title: HelpStrings.openShaderInfoTitle, bodyText: HelpStrings.clickBlendInstruction, side: .below)
            }

        default:
            break
        }
        return nil
    }

    // MARK: - Helpers

    private func isBlendNode(_ node: NodeModel) -> Bool {
        if case .shader(let raw) = node.nodeType {
            return raw == 80 // ShaderTypeBlend
        }
        return false
    }

    /// First non-blend node (for blend-via-outputs tooltip).
    private func firstUnconnectedNodeId(_ vm: NodeEditorViewModel) -> String? {
        vm.nodes.first(where: { !isBlendNode($0) })?.id
    }

    /// Second non-blend node (for blend-via-outputs tooltip).
    private func secondUnconnectedNodeId(_ vm: NodeEditorViewModel) -> String? {
        let nonBlend = vm.nodes.filter { !isBlendNode($0) }
        return nonBlend.count >= 2 ? nonBlend[1].id : nil
    }
}

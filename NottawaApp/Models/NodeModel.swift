//
//  NodeModel.swift
//  NottawaApp
//
//  Unified model for both shaders and video sources on the canvas.
//

import Foundation

struct NodeModel: Identifiable, Hashable {
    let id: String
    let name: String
    let nodeType: NodeType
    let inputCount: Int
    var position: CGPoint
    var isActive: Bool
    var isBypassed: Bool = false
    var supportsAuxOutput: Bool
    var isAudioReactive: Bool = false
    var downloadProgress: Float? = nil   // nil = not downloading, 0.0-1.0 = downloading
    var downloadPaused: Bool = false
    var libraryFileId: String? = nil

    enum NodeType: Hashable {
        case shader(shaderTypeRaw: Int)
        case source(sourceType: Int)
    }

    var isSource: Bool {
        if case .source = nodeType { return true }
        return false
    }

    var isShader: Bool {
        if case .shader = nodeType { return true }
        return false
    }

    /// A "shader source" is a VideoSource of type VideoSource_shader — a
    /// procedural shader generator (Plasma, Galaxy, Aurora, etc.) wrapped
    /// as a source. It has an editable inner Shader just like effect nodes.
    /// Raw value 4 matches `VideoSource_shader` in src/Video/VideoSource.hpp.
    var isShaderSource: Bool {
        if case .source(let raw) = nodeType { return raw == 4 }
        return false
    }

    /// True if this node carries an editable Shader instance — either a
    /// chained effect or a shader-source generator.
    var hasEditableShader: Bool { isShader || isShaderSource }

    func hash(into hasher: inout Hasher) {
        hasher.combine(id)
    }

    static func == (lhs: NodeModel, rhs: NodeModel) -> Bool {
        lhs.id == rhs.id
        && lhs.position == rhs.position
        && lhs.isActive == rhs.isActive
        && lhs.isBypassed == rhs.isBypassed
        && lhs.inputCount == rhs.inputCount
        && lhs.supportsAuxOutput == rhs.supportsAuxOutput
        && lhs.downloadProgress == rhs.downloadProgress
        && lhs.downloadPaused == rhs.downloadPaused
    }

    /// Shader types whose auxiliary output is part of their semantics
    /// (mirrors `Shader::allowAuxOutputSlot()` in src/Shading/Shader.cpp).
    /// FeedbackShader overrides that on the C++ side to gate on the
    /// `Allow Auxillary` parameter, but the runtime feedback path itself only
    /// checks for an aux connection — so for the SwiftUI UX we surface the
    /// aux pin unconditionally for these types.
    static func shaderTypeSupportsAuxOutput(_ raw: Int) -> Bool {
        // 7  = ShaderTypeFeedback
        // 41 = ShaderTypeLumaMaskMaker
        // 45 = ShaderTypeColorKeyMaskMaker
        return raw == 7 || raw == 41 || raw == 45
    }

    // Factory from ConnectableInfo (shader)
    static func from(_ info: ConnectableInfo) -> NodeModel {
        let isShader = info.connectableType != .source
        return NodeModel(
            id: info.id,
            name: info.name,
            nodeType: info.connectableType == .source
                ? .source(sourceType: info.shaderTypeRaw)
                : .shader(shaderTypeRaw: info.shaderTypeRaw),
            inputCount: info.inputCount,
            position: CGPoint(x: CGFloat(info.posX), y: CGFloat(info.posY)),
            isActive: true,
            supportsAuxOutput: isShader ? NodeModel.shaderTypeSupportsAuxOutput(info.shaderTypeRaw) : false,
            isAudioReactive: NottawaEngine.shared.isAudioReactive(connectableId: info.id)
        )
    }

    // Factory from VideoSourceInfo
    static func from(_ info: VideoSourceInfo) -> NodeModel {
        NodeModel(
            id: info.id,
            name: info.name,
            nodeType: .source(sourceType: info.sourceType),
            inputCount: info.inputCount,
            position: CGPoint(x: CGFloat(info.posX), y: CGFloat(info.posY)),
            isActive: true,
            supportsAuxOutput: false,
            isAudioReactive: NottawaEngine.shared.isAudioReactive(connectableId: info.id)
        )
    }
}

// MARK: - Chain Strip (Stage Mode)

struct ChainStrip: Identifiable {
    let chainIndex: Int       // Which chain (for multi-chain graphs)
    let depth: Int            // 0 = source, 1 = first shader, etc.
    let node: NodeModel
    let favoriteParams: [ParameterInfo]
    let allParams: [ParameterInfo]
    let isTerminal: Bool      // True if no outgoing connections (leaf node)
    var id: String { node.id }
}

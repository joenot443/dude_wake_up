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

    func hash(into hasher: inout Hasher) {
        hasher.combine(id)
    }

    static func == (lhs: NodeModel, rhs: NodeModel) -> Bool {
        lhs.id == rhs.id
        && lhs.position == rhs.position
        && lhs.isActive == rhs.isActive
        && lhs.isBypassed == rhs.isBypassed
        && lhs.supportsAuxOutput == rhs.supportsAuxOutput
        && lhs.downloadProgress == rhs.downloadProgress
        && lhs.downloadPaused == rhs.downloadPaused
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
            supportsAuxOutput: isShader ? NottawaEngine.shared.supportsAuxOutput(shaderId: info.id) : false
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
            supportsAuxOutput: false
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

//
//  GraphModel.swift
//  NottawaApp
//
//  Observable model of the shader graph state. Refreshes automatically
//  when the C++ engine notifies of changes via the graph-changed callback.
//

import Foundation

@Observable
final class GraphModel {
    var shaders: [ConnectableInfo] = []
    var videoSources: [VideoSourceInfo] = []
    var connections: [ConnectionInfo] = []
    var selectedConnectableId: String?

    private let engine = NottawaEngine.shared

    init() {
        // Register for graph changes from the engine
        engine.registerGraphChangedCallback { [weak self] in
            self?.refresh()
        }
    }

    /// Pull the latest state from the engine.
    func refresh() {
        shaders = engine.allShaders()
        videoSources = engine.allVideoSources()
        connections = engine.allConnections()
        selectedConnectableId = engine.selectedConnectableId
    }

    /// Parameters for a specific connectable.
    func parameters(for connectableId: String) -> [ParameterInfo] {
        engine.parameters(for: connectableId)
    }

    // MARK: - Texture Sharing

    private(set) var sharingTextureIds: Set<String> = []

    func startTextureSharing(for connectableId: String) {
        engine.startTextureSharing(connectableId: connectableId)
        sharingTextureIds.insert(connectableId)
    }

    func stopTextureSharing(for connectableId: String) {
        engine.stopTextureSharing(connectableId: connectableId)
        sharingTextureIds.remove(connectableId)
        TextureProvider.shared.invalidate(connectableId: connectableId)
    }

    func stopAllTextureSharing() {
        for id in sharingTextureIds {
            engine.stopTextureSharing(connectableId: id)
        }
        sharingTextureIds.removeAll()
        TextureProvider.shared.invalidateAll()
    }
}

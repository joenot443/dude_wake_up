//
//  BrowserPreviewManager.swift
//  NottawaApp
//
//  Manages lifecycle of live shader preview instances for browser tiles.
//  Preloads static snapshots for all shaders/sources at startup so sidebar
//  browsing is instant. Also supports live animated preview mode.
//

import Foundation
import CoreGraphics

@Observable
final class BrowserPreviewManager {
    static let shared = BrowserPreviewManager()

    /// Map of shaderTypeRaw → preview connectable ID for texture sharing.
    private var activePreviews: [Int: String] = [:]
    /// Cached static snapshots keyed by the same key as activePreviews.
    private var snapshots: [Int: CGImage] = [:]
    private let engine = NottawaEngine.shared
    private var isPreloading = false

    private init() {}

    private func key(for shaderTypeRaw: Int, isSource: Bool) -> Int {
        isSource ? -shaderTypeRaw : shaderTypeRaw
    }

    // MARK: - Live Preview

    /// Get or create a preview ID for a shader type (live mode).
    func previewId(for shaderTypeRaw: Int, isSource: Bool) -> String? {
        let k = key(for: shaderTypeRaw, isSource: isSource)
        if let existing = activePreviews[k] {
            return existing
        }
        guard let id = engine.createPreview(shaderTypeRaw: shaderTypeRaw, isSource: isSource) else {
            return nil
        }
        activePreviews[k] = id
        return id
    }

    /// Destroy a single preview when its tile scrolls out of view.
    func destroyPreview(for shaderTypeRaw: Int, isSource: Bool) {
        let k = key(for: shaderTypeRaw, isSource: isSource)
        guard let id = activePreviews.removeValue(forKey: k) else { return }
        engine.destroyPreview(id: id)
    }

    /// Destroy all live previews (does not clear cached snapshots).
    func destroyAll() {
        engine.destroyAllPreviews()
        activePreviews.removeAll()
    }

    // MARK: - Static Snapshot

    /// Return a cached snapshot for a shader type, if available.
    func snapshot(for shaderTypeRaw: Int, isSource: Bool) -> CGImage? {
        snapshots[key(for: shaderTypeRaw, isSource: isSource)]
    }

    /// Capture a snapshot from a currently-active live preview.
    func captureSnapshotFromLive(for shaderTypeRaw: Int, isSource: Bool) {
        let k = key(for: shaderTypeRaw, isSource: isSource)
        guard let id = activePreviews[k] else { return }
        if let image = TextureProvider.shared.snapshot(for: id) {
            snapshots[k] = image
        }
    }

    /// Request a static snapshot (fallback for items not yet preloaded).
    func requestSnapshot(for shaderTypeRaw: Int, isSource: Bool) {
        let k = key(for: shaderTypeRaw, isSource: isSource)
        if snapshots[k] != nil { return }
        if activePreviews[k] != nil { return }

        guard let id = engine.createPreview(shaderTypeRaw: shaderTypeRaw, isSource: isSource) else { return }
        activePreviews[k] = id
        captureAfterDelay(key: k, id: id, retriesLeft: 5)
    }

    private func captureAfterDelay(key k: Int, id: String, retriesLeft: Int) {
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.2) { [weak self] in
            guard let self else { return }
            guard self.activePreviews[k] == id else { return }

            if let image = TextureProvider.shared.snapshot(for: id) {
                self.snapshots[k] = image
                self.engine.destroyPreview(id: id)
                self.activePreviews.removeValue(forKey: k)
            } else if retriesLeft > 0 {
                self.captureAfterDelay(key: k, id: id, retriesLeft: retriesLeft - 1)
            } else {
                self.engine.destroyPreview(id: id)
                self.activePreviews.removeValue(forKey: k)
            }
        }
    }

    // MARK: - Startup Preloading

    /// Clear all cached snapshots and re-capture them using the given source.
    /// Pass the selected video source node ID so effect previews use it as input.
    func refreshAllSnapshots(preferredSourceId: String? = nil) {
        engine.setPreferredPreviewSource(sourceId: preferredSourceId)
        snapshots.removeAll()
        destroyAll()
        isPreloading = false
        preloadAllSnapshots()
    }

    /// Preload snapshots for all available shaders and sources.
    /// Processes in small batches to avoid exhausting GPU resources.
    func preloadAllSnapshots() {
        guard !isPreloading else { return }
        isPreloading = true

        let shaders = engine.availableShaders()
        let sources = engine.availableShaderSources()

        var items: [(typeRaw: Int, isSource: Bool)] = []
        for s in shaders {
            let k = key(for: s.shaderTypeRaw, isSource: false)
            if snapshots[k] == nil { items.append((s.shaderTypeRaw, false)) }
        }
        for s in sources {
            let k = key(for: s.shaderTypeRaw, isSource: true)
            if snapshots[k] == nil { items.append((s.shaderTypeRaw, true)) }
        }

        preloadBatch(items: items, batchSize: 2, index: 0)
    }

    private func preloadBatch(items: [(typeRaw: Int, isSource: Bool)], batchSize: Int, index: Int) {
        guard index < items.count else {
            isPreloading = false
            return
        }

        let end = min(index + batchSize, items.count)
        var batchPreviews: [(key: Int, id: String)] = []

        for i in index..<end {
            let item = items[i]
            let k = key(for: item.typeRaw, isSource: item.isSource)
            if snapshots[k] != nil { continue }
            guard let id = engine.createPreview(shaderTypeRaw: item.typeRaw, isSource: item.isSource) else { continue }
            batchPreviews.append((k, id))
        }

        // Wait for the engine to render a frame, then capture.
        // Small batch + longer delay keeps main render loop smooth.
        DispatchQueue.main.asyncAfter(deadline: .now() + 0.5) { [weak self] in
            guard let self else { return }

            for (k, id) in batchPreviews {
                if let image = TextureProvider.shared.snapshot(for: id) {
                    self.snapshots[k] = image
                }
                self.engine.destroyPreview(id: id)
            }

            self.preloadBatch(items: items, batchSize: batchSize, index: end)
        }
    }
}

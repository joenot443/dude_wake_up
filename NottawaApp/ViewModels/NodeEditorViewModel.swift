//
//  NodeEditorViewModel.swift
//  NottawaApp
//
//  Central ViewModel for the node editor canvas and inspector.
//

import Foundation
import SwiftUI
import UniformTypeIdentifiers

/// Key for identifying a pin position in the pin positions dictionary.
struct PinKey: Hashable {
    let nodeId: String
    let slotIndex: Int
    let isOutput: Bool
}

/// State for an in-progress connection drag.
struct DragConnectionState {
    let fromNodeId: String
    let fromSlotIndex: Int
    let fromIsOutput: Bool
    let fromPosition: CGPoint   // Pin position at drag start (screen coords)
    var currentPosition: CGPoint
}

@Observable
final class NodeEditorViewModel {
    // MARK: - Graph State
    var nodes: [NodeModel] = []
    var connections: [ConnectionModel] = []

    // MARK: - Canvas State
    var canvasOffset: CGPoint = .zero
    var canvasScale: CGFloat = 1.0

    // MARK: - Selection
    var selectedNodeIds: Set<String> = []

    // MARK: - Drag Connection
    var dragConnection: DragConnectionState?
    /// Pin currently snapped to during drag (within magnetic radius).
    var dragSnapPinKey: PinKey?
    /// All valid target pins for the current drag (compatible type, not self).
    var dragValidPinKeys: Set<PinKey> = []

    // MARK: - Browser State
    var showShaderBrowser = false
    var showSourceBrowser = false
    var browserPosition: CGPoint = .zero
    var browserMode: BrowserMode = .add
    var swapTargetNodeId: String?

    enum BrowserMode {
        case add
        case swap
    }

    // MARK: - Action Bar State
    var actionBarExpanded = true
    var helpEnabled = false

    // MARK: - Sidebar State
    var showSidebar = true
    var sidebarActiveTab: SidebarTab = .shaders
    var sidebarWidth: CGFloat = 240
    var inspectorWidth: CGFloat = 350

    enum SidebarTab {
        case shaders
        case sources
        case oscillators
        case library
        case strands
    }

    // MARK: - Pin Positions (screen coords for cable drawing)
    var pinPositions: [PinKey: CGPoint] = [:]

    // MARK: - Strands State
    var strandsList: [StrandInfo] = []

    // MARK: - Library State
    var libraryFiles: [LibraryFileInfo] = []
    var libraryLoading = false
    private var downloadPollTimer: Timer?
    private var nodeDownloadPollTimer: Timer?

    // MARK: - Canvas Drop Highlight
    var isDropTargetActive = false

    // MARK: - Welcome Screen
    var showWelcomeScreen: Bool = true
    var welcomeScreenEnabled: Bool {
        didSet {
            UserDefaults.standard.set(welcomeScreenEnabled, forKey: "nottawa_welcome_enabled")
        }
    }
    var templateStrandsList: [StrandInfo] = []

    // MARK: - Stage Mode
    var stageModeEnabled: Bool = false
    var soloedNodeId: String? = nil
    var pinnedOutputId: String? = nil
    var stageShowAllParams: Bool = true
    var orderedChainStrips: [ChainStrip] = []
    var openPreviewWindowIds: Set<String> = []
    var stageStripOrder: [String]? = nil   // user-defined ordering (node IDs)
    private var stageRefreshTimer: Timer?
    private var savedSidebarState: Bool?

    // MARK: - Audio Panel
    var showAudioPanel = false
    var audioPanelHeight: CGFloat = 250

    // MARK: - Inspector
    var inspectorParameters: [ParameterInfo] = []
    var fileSourceState: FileSourceState?
    var textSourceState: TextSourceState?

    // MARK: - Community State
    var communityStrands: [SharedStrandInfo] = []
    var communityPage: Int = 1
    var communityTotalPages: Int = 1
    var communityLoading: Bool = false
    var communitySortOrder: CommunitySortOrder = .newest
    var strandViewMode: StrandViewMode = .local
    var shareInProgress: Bool = false
    var shareResult: ShareResult? = nil
    var showShareSheet: Bool = false
    var shareNodeId: String? = nil

    enum StrandViewMode {
        case local
        case community
    }

    enum CommunitySortOrder: String, CaseIterable {
        case newest = "Newest"
        case popular = "Popular"
        case mostViewed = "Most Viewed"
        case topRated = "Top Rated"
    }

    var sortedCommunityStrands: [SharedStrandInfo] {
        switch communitySortOrder {
        case .newest:
            return communityStrands.sorted { $0.createdAt > $1.createdAt }
        case .popular:
            return communityStrands.sorted { $0.opens > $1.opens }
        case .mostViewed:
            return communityStrands.sorted { $0.views > $1.views }
        case .topRated:
            return communityStrands.sorted { $0.score > $1.score }
        }
    }

    enum ShareResult {
        case success(slug: String)
        case error(message: String)
    }

    var voterId: String {
        if let id = UserDefaults.standard.string(forKey: "nottawa_voter_id") { return id }
        let id = UUID().uuidString
        UserDefaults.standard.set(id, forKey: "nottawa_voter_id")
        return id
    }

    private let engine = NottawaEngine.shared
    private var sharingTextureIds: Set<String> = []

    init() {
        self.welcomeScreenEnabled = UserDefaults.standard.object(forKey: "nottawa_welcome_enabled") as? Bool ?? true
        self.showWelcomeScreen = self.welcomeScreenEnabled
    }

    // MARK: - Initialization

    func setup() {
        engine.registerGraphChangedCallback { [weak self] in
            self?.refresh()
        }
        engine.registerLibraryUpdateCallback { [weak self] in
            self?.refreshLibrary()
        }
        engine.registerResolutionChangedCallback {
            PreviewWindowManager.shared.updateWindowSizes()
        }
        engine.registerStrandsUpdatedCallback { [weak self] in
            self?.refreshStrands()
        }
        refresh()
        fetchTemplateStrands()
    }

    // MARK: - Refresh from Engine

    func refresh() {
        let shaders = engine.allShaders()
        let sources = engine.allVideoSources()
        let conns = engine.allConnections()

        let shaderNodes = shaders.map { NodeModel.from($0) }
        let sourceNodes = sources.map { NodeModel.from($0) }

        // Update active state, bypass state, and download state from engine
        var allNodes = shaderNodes + sourceNodes
        for i in allNodes.indices {
            allNodes[i].isActive = engine.isConnectableActive(id: allNodes[i].id)
            if allNodes[i].isShader {
                allNodes[i].isBypassed = engine.isShaderBypassed(id: allNodes[i].id)
            }

            // Check download state for library sources (sourceType 8 = VideoSource_library)
            if case .source(let sourceType) = allNodes[i].nodeType, sourceType == 8 {
                if let dlState = engine.librarySourceState(sourceId: allNodes[i].id) {
                    if dlState.isDownloading {
                        allNodes[i].downloadProgress = dlState.progress
                        allNodes[i].downloadPaused = dlState.isPaused
                        allNodes[i].libraryFileId = dlState.libraryFileId
                    }
                }
            }
        }

        // Manage texture sharing: start for new nodes, stop for removed
        let newIds = Set(allNodes.map(\.id))
        let oldIds = sharingTextureIds

        for id in newIds.subtracting(oldIds) {
            engine.startTextureSharing(connectableId: id)
            sharingTextureIds.insert(id)
        }
        for id in oldIds.subtracting(newIds) {
            engine.stopTextureSharing(connectableId: id)
            sharingTextureIds.remove(id)
            TextureProvider.shared.invalidate(connectableId: id)
        }

        nodes = allNodes
        connections = conns.map { ConnectionModel.from($0) }

        // Clean up selection for removed nodes
        selectedNodeIds = selectedNodeIds.intersection(newIds)

        // Auto-poll while any on-canvas node is downloading
        let hasDownloading = allNodes.contains { $0.downloadProgress != nil }
        if hasDownloading && nodeDownloadPollTimer == nil {
            nodeDownloadPollTimer = Timer.scheduledTimer(withTimeInterval: 0.5, repeats: true) { [weak self] _ in
                self?.refresh()
            }
        } else if !hasDownloading && nodeDownloadPollTimer != nil {
            nodeDownloadPollTimer?.invalidate()
            nodeDownloadPollTimer = nil
        }

        // Refresh inspector if a selected node still exists
        refreshInspector()

        // Stage mode: refresh strips and auto-clear solo/pinned if node was deleted
        if stageModeEnabled {
            if let soloId = soloedNodeId, !newIds.contains(soloId) {
                soloedNodeId = nil
            }
            if let pinId = pinnedOutputId, !newIds.contains(pinId) {
                pinnedOutputId = nil
            }
            refreshStageStrips()
        }
    }

    // MARK: - Selection

    var selectedNodeId: String? {
        selectedNodeIds.count == 1 ? selectedNodeIds.first : nil
    }

    func selectNode(_ id: String, exclusive: Bool = true) {
        if exclusive {
            selectedNodeIds = [id]
        } else {
            selectedNodeIds.insert(id)
        }
        engine.selectConnectable(id: id)
        refreshInspector()
    }

    func deselectAll() {
        selectedNodeIds.removeAll()
        engine.deselectConnectable()
        inspectorParameters = []
        fileSourceState = nil
        textSourceState = nil
    }

    func toggleNodeSelection(_ id: String) {
        if selectedNodeIds.contains(id) {
            selectedNodeIds.remove(id)
            if selectedNodeIds.isEmpty {
                engine.deselectConnectable()
            }
        } else {
            selectedNodeIds.insert(id)
            engine.selectConnectable(id: id)
        }
        refreshInspector()
    }

    // MARK: - Inspector

    func refreshInspector() {
        guard let nodeId = selectedNodeId else {
            inspectorParameters = []
            fileSourceState = nil
            textSourceState = nil
            return
        }
        inspectorParameters = engine.parameters(for: nodeId)
        fileSourceState = engine.fileSourceState(sourceId: nodeId)
        textSourceState = engine.textSourceState(sourceId: nodeId)
    }

    // MARK: - Node Actions

    /// Fast local-only position update (no engine call) for smooth dragging.
    func setNodePositionLocal(_ id: String, to position: CGPoint) {
        if let idx = nodes.firstIndex(where: { $0.id == id }) {
            nodes[idx].position = position
        }
    }

    /// Update position locally and persist to engine.
    func moveNode(_ id: String, to position: CGPoint) {
        setNodePositionLocal(id, to: position)
        engine.setConnectablePosition(id: id, x: Float(position.x), y: Float(position.y))
    }

    func deleteSelected() {
        for id in selectedNodeIds {
            engine.stopTextureSharing(connectableId: id)
            sharingTextureIds.remove(id)
            TextureProvider.shared.invalidate(connectableId: id)
            engine.removeConnectable(id: id)
        }
        selectedNodeIds.removeAll()
        inspectorParameters = []
        refresh()
    }

    func toggleActiveState(_ id: String) {
        if let idx = nodes.firstIndex(where: { $0.id == id }) {
            nodes[idx].isActive.toggle()
            engine.setConnectableActive(id: id, active: nodes[idx].isActive)
        }
    }

    func deleteNode(_ id: String) {
        engine.stopTextureSharing(connectableId: id)
        sharingTextureIds.remove(id)
        TextureProvider.shared.invalidate(connectableId: id)
        engine.removeConnectable(id: id)
        selectedNodeIds.remove(id)
        refresh()
    }

    // MARK: - Connection Helpers

    /// Get all connections involving a specific node.
    func connectionsForNode(_ nodeId: String) -> [ConnectionModel] {
        connections.filter { $0.startNodeId == nodeId || $0.endNodeId == nodeId }
    }

    /// Get the display name for the other end of a connection relative to a node.
    func nodeNameForConnection(_ connection: ConnectionModel, relativeTo nodeId: String) -> String {
        let otherId = connection.startNodeId == nodeId ? connection.endNodeId : connection.startNodeId
        return nodes.first(where: { $0.id == otherId })?.name ?? "Unknown"
    }

    // MARK: - Connection Actions

    func makeConnection(startId: String, endId: String, outputSlot: Int, inputSlot: Int) {
        // Determine connection type based on node types
        let startNode = nodes.first(where: { $0.id == startId })
        let connectionType: Int = startNode?.isSource == true ? 1 : 0
        engine.makeConnection(
            startId: startId,
            endId: endId,
            connectionType: connectionType,
            outputSlot: outputSlot,
            inputSlot: inputSlot
        )
        refresh()
    }

    func removeConnection(_ id: String) {
        engine.removeConnection(id: id)
        refresh()
    }

    // MARK: - Multi-drag Support

    /// Move all selected nodes by a delta (local only, for smooth dragging).
    func moveSelectedNodesLocal(by delta: CGPoint) {
        for id in selectedNodeIds {
            if let idx = nodes.firstIndex(where: { $0.id == id }),
               let startPos = multiDragStartPositions[id] {
                nodes[idx].position = CGPoint(
                    x: startPos.x + delta.x,
                    y: startPos.y + delta.y
                )
            }
        }
    }

    /// Persist all selected node positions to the engine after drag ends.
    func commitSelectedNodePositions() {
        for id in selectedNodeIds {
            if let node = nodes.first(where: { $0.id == id }) {
                engine.setConnectablePosition(id: id, x: Float(node.position.x), y: Float(node.position.y))
            }
        }
        multiDragStartPositions.removeAll()
    }

    /// Snapshot current positions for multi-drag.
    var multiDragStartPositions: [String: CGPoint] = [:]

    func captureMultiDragStartPositions() {
        multiDragStartPositions.removeAll()
        for id in selectedNodeIds {
            if let node = nodes.first(where: { $0.id == id }) {
                multiDragStartPositions[id] = node.position
            }
        }
    }

    // MARK: - Blend

    func blendSelectedNodes() {
        guard selectedNodeIds.count == 2 else { return }
        let ids = Array(selectedNodeIds)
        createBlendBetween(id1: ids[0], id2: ids[1])
    }

    func createBlendBetween(id1: String, id2: String) {
        let node1 = nodes.first(where: { $0.id == id1 })
        let node2 = nodes.first(where: { $0.id == id2 })

        guard let blendId = engine.createBlendBetween(id1: id1, id2: id2) else { return }

        // Position blend to the right of both nodes, centered vertically
        if let n1 = node1, let n2 = node2 {
            let rightX = max(n1.position.x, n2.position.x) + 300
            let midY = (n1.position.y + n2.position.y) / 2
            engine.setConnectablePosition(id: blendId, x: Float(rightX), y: Float(midY))
        }

        refresh()
    }

    // MARK: - Shader Swap

    func swapShader(nodeId: String, newType: Int) {
        engine.stopTextureSharing(connectableId: nodeId)
        sharingTextureIds.remove(nodeId)
        TextureProvider.shared.invalidate(connectableId: nodeId)

        guard let newId = engine.replaceShader(shaderId: nodeId, newType: newType) else { return }

        // Start sharing for the new shader
        engine.startTextureSharing(connectableId: newId)
        sharingTextureIds.insert(newId)

        // Update selection
        selectedNodeIds.remove(nodeId)
        selectedNodeIds.insert(newId)

        refresh()
    }

    // MARK: - Source Swap

    func swapSource(nodeId: String, newType: Int) {
        engine.stopTextureSharing(connectableId: nodeId)
        sharingTextureIds.remove(nodeId)
        TextureProvider.shared.invalidate(connectableId: nodeId)

        guard let newId = engine.replaceVideoSource(sourceId: nodeId, newType: newType) else { return }

        // Start sharing for the new source
        engine.startTextureSharing(connectableId: newId)
        sharingTextureIds.insert(newId)

        // Update selection
        selectedNodeIds.remove(nodeId)
        selectedNodeIds.insert(newId)

        refresh()
    }

    // MARK: - Select All

    func selectAll() {
        selectedNodeIds = Set(nodes.map(\.id))
    }

    // MARK: - Copy / Paste

    func copySelected() {
        guard !selectedNodeIds.isEmpty else { return }
        engine.copyConnectables(ids: Array(selectedNodeIds))
    }

    func pasteNodes() {
        let newIds = engine.pasteConnectables()
        guard !newIds.isEmpty else { return }
        refresh()
        selectedNodeIds = Set(newIds)
        refreshInspector()
    }

    // MARK: - Add Nodes

    /// Tracks the last position where a node was added, for cascading.
    var lastAddedNodePosition: CGPoint?

    private func nextAutoPosition() -> CGPoint {
        if let last = lastAddedNodePosition {
            return CGPoint(x: last.x, y: last.y + 228)
        }
        return screenToCanvas(CGPoint(x: 400, y: 300))
    }

    @discardableResult
    func addShader(type: Int, at position: CGPoint? = nil) -> String? {
        guard let id = engine.addShader(type: type) else { return nil }
        let pos = position ?? nextAutoPosition()
        engine.setConnectablePosition(id: id, x: Float(pos.x), y: Float(pos.y))
        lastAddedNodePosition = pos
        refresh()
        return id
    }

    @discardableResult
    func addShaderVideoSource(type: Int, at position: CGPoint? = nil) -> String? {
        guard let id = engine.addShaderVideoSource(type: type) else { return nil }
        let pos = position ?? nextAutoPosition()
        engine.setConnectablePosition(id: id, x: Float(pos.x), y: Float(pos.y))
        lastAddedNodePosition = pos
        refresh()
        return id
    }

    @discardableResult
    func addTextVideoSource(at position: CGPoint? = nil) -> String? {
        guard let id = engine.addTextVideoSource(name: "Text") else { return nil }
        let pos = position ?? nextAutoPosition()
        engine.setConnectablePosition(id: id, x: Float(pos.x), y: Float(pos.y))
        lastAddedNodePosition = pos
        refresh()
        return id
    }

    @discardableResult
    func addIconVideoSource(at position: CGPoint? = nil) -> String? {
        guard let id = engine.addIconVideoSource(name: "Icon") else { return nil }
        let pos = position ?? nextAutoPosition()
        engine.setConnectablePosition(id: id, x: Float(pos.x), y: Float(pos.y))
        lastAddedNodePosition = pos
        refresh()
        return id
    }

    @discardableResult
    func addWebcamVideoSource(at position: CGPoint? = nil) -> String? {
        guard let id = engine.addWebcamVideoSource(name: "Webcam", deviceId: 0) else { return nil }
        let pos = position ?? nextAutoPosition()
        engine.setConnectablePosition(id: id, x: Float(pos.x), y: Float(pos.y))
        lastAddedNodePosition = pos
        refresh()
        return id
    }

    /// Add a non-shader video source by VideoSourceType enum value.
    @discardableResult
    func addNonShaderSource(type: Int, at position: CGPoint? = nil) -> String? {
        // VideoSourceType: 0=webcam, 3=icon, 5=text
        switch type {
        case 5: return addTextVideoSource(at: position)
        case 3: return addIconVideoSource(at: position)
        case 0: return addWebcamVideoSource(at: position)
        default: return nil
        }
    }

    /// Add a node at a specific canvas position from a drop payload.
    /// Payload format: "shader:<typeInt>" or "source:<typeInt>"
    /// If targetNodeId is provided and the payload is a shader (effect),
    /// the new shader is chained after the target (output 0 → input 0).
    func addNodeFromDrop(payload: String, at screenPoint: CGPoint, targetNodeId: String? = nil) {
        let canvasPos = screenToCanvas(screenPoint)
        let parts = payload.split(separator: ":", maxSplits: 1)
        guard parts.count == 2 else { return }

        let kind = parts[0]
        let value = String(parts[1])

        if kind == "strand" {
            loadStrand(id: value, at: canvasPos)
            return
        } else if kind == "shared_strand" {
            loadSharedStrand(slug: value)
            return
        } else if kind == "library" {
            addLibrarySource(libraryFileId: value, at: canvasPos)
        } else if kind == "nonsrc", let typeInt = Int(value) {
            addNonShaderSource(type: typeInt, at: canvasPos)
        } else if let typeInt = Int(value) {
            if kind == "shader" {
                if let targetId = targetNodeId,
                   let targetNode = nodes.first(where: { $0.id == targetId }) {
                    // Dropped onto an existing node — chain the new effect after it
                    let chainPos = CGPoint(x: targetNode.position.x + 300, y: targetNode.position.y)
                    guard let newId = addShader(type: typeInt, at: chainPos) else { return }
                    let connectionType: Int = targetNode.isSource ? 1 : 0
                    engine.makeConnection(
                        startId: targetId,
                        endId: newId,
                        connectionType: connectionType,
                        outputSlot: 0,
                        inputSlot: 0
                    )
                    refresh()
                } else {
                    addShader(type: typeInt, at: canvasPos)
                }
            } else if kind == "source" {
                addShaderVideoSource(type: typeInt, at: canvasPos)
            }
        }
    }

    // MARK: - Undo / Redo

    func undo() { engine.undo() }
    func redo() { engine.redo() }
    var canUndo: Bool { engine.canUndo }
    var canRedo: Bool { engine.canRedo }

    // MARK: - Canvas Coordinate Helpers

    /// Convert a screen point to canvas coordinates.
    func screenToCanvas(_ point: CGPoint) -> CGPoint {
        CGPoint(
            x: (point.x - canvasOffset.x) / canvasScale,
            y: (point.y - canvasOffset.y) / canvasScale
        )
    }

    /// Convert a canvas point to screen coordinates.
    func canvasToScreen(_ point: CGPoint) -> CGPoint {
        CGPoint(
            x: point.x * canvasScale + canvasOffset.x,
            y: point.y * canvasScale + canvasOffset.y
        )
    }

    // MARK: - Zoom to Fit

    /// Adjusts canvas offset and scale so all nodes are visible with padding.
    func zoomToFit(canvasSize: CGSize) {
        guard !nodes.isEmpty else {
            canvasScale = 1.0
            canvasOffset = .zero
            return
        }

        let nodeWidth: CGFloat = 200
        let nodeHeight: CGFloat = 150
        let padding: CGFloat = 60

        var minX = CGFloat.infinity
        var minY = CGFloat.infinity
        var maxX = -CGFloat.infinity
        var maxY = -CGFloat.infinity

        for node in nodes {
            minX = min(minX, node.position.x)
            minY = min(minY, node.position.y)
            maxX = max(maxX, node.position.x + nodeWidth)
            maxY = max(maxY, node.position.y + nodeHeight)
        }

        let contentWidth = maxX - minX + padding * 2
        let contentHeight = maxY - minY + padding * 2

        let scaleX = canvasSize.width / contentWidth
        let scaleY = canvasSize.height / contentHeight
        let newScale = min(scaleX, scaleY, 1.5) // Don't zoom in too far

        let centerX = (minX + maxX) / 2
        let centerY = (minY + maxY) / 2

        canvasScale = max(0.2, newScale)
        canvasOffset = CGPoint(
            x: canvasSize.width / 2 - centerX * canvasScale,
            y: canvasSize.height / 2 - centerY * canvasScale
        )
    }

    // MARK: - Pin Hit Testing

    /// Find the nearest pin within a distance threshold.
    func hitTestPin(at screenPoint: CGPoint, excluding nodeId: String? = nil, outputOnly: Bool = false, inputOnly: Bool = false) -> PinKey? {
        let threshold: CGFloat = 20
        var bestKey: PinKey?
        var bestDist = CGFloat.infinity

        for (key, pos) in pinPositions {
            if let excluding = nodeId, key.nodeId == excluding { continue }
            if outputOnly && !key.isOutput { continue }
            if inputOnly && key.isOutput { continue }

            let dist = hypot(pos.x - screenPoint.x, pos.y - screenPoint.y)
            if dist < threshold && dist < bestDist {
                bestDist = dist
                bestKey = key
            }
        }
        return bestKey
    }

    /// Compute valid target pins for the current drag and the snap target.
    func updateDragTargets(at screenPoint: CGPoint) {
        guard let drag = dragConnection else {
            dragSnapPinKey = nil
            dragValidPinKeys = []
            return
        }

        // Valid targets: opposite type, not on source node
        // Also include output pins when dragging from output (for blend)
        let targetIsInput = drag.fromIsOutput
        var validKeys = Set<PinKey>()
        for (key, _) in pinPositions {
            if key.nodeId == drag.fromNodeId { continue }
            if targetIsInput && !key.isOutput { validKeys.insert(key) }   // input pins
            if !targetIsInput && key.isOutput { validKeys.insert(key) }   // output pins
            if drag.fromIsOutput && key.isOutput && key.nodeId != drag.fromNodeId {
                validKeys.insert(key)  // output→output for blend
            }
        }
        dragValidPinKeys = validKeys

        // Magnetic snap — larger radius (30pt, scaled by zoom)
        let snapRadius: CGFloat = 30 / canvasScale
        var bestKey: PinKey?
        var bestDist = CGFloat.infinity
        for key in validKeys {
            guard let pos = pinPositions[key] else { continue }
            let dist = hypot(pos.x - screenPoint.x, pos.y - screenPoint.y)
            if dist < snapRadius && dist < bestDist {
                bestDist = dist
                bestKey = key
            }
        }
        dragSnapPinKey = bestKey
    }

    /// Clear all drag target state.
    func clearDragTargets() {
        dragSnapPinKey = nil
        dragValidPinKeys = []
    }

    // MARK: - Workspace Management

    var currentWorkspacePath: String?
    var currentWorkspaceName: String?

    var hasWorkspace: Bool { currentWorkspacePath != nil }

    func newWorkspace() {
        for node in nodes {
            engine.stopTextureSharing(connectableId: node.id)
        }
        sharingTextureIds.removeAll()
        TextureProvider.shared.invalidateAll()
        engine.clearGraph()
        currentWorkspacePath = nil
        currentWorkspaceName = nil
        selectedNodeIds.removeAll()
        inspectorParameters = []
        refresh()
    }

    func openWorkspace(url: URL) {
        for node in nodes {
            engine.stopTextureSharing(connectableId: node.id)
        }
        sharingTextureIds.removeAll()
        TextureProvider.shared.invalidateAll()
        engine.loadWorkspace(path: url.path)
        currentWorkspacePath = url.path
        currentWorkspaceName = url.deletingPathExtension().lastPathComponent
        selectedNodeIds.removeAll()
        inspectorParameters = []
        refresh()
    }

    func saveWorkspace() {
        if let path = currentWorkspacePath, let name = currentWorkspaceName {
            engine.saveWorkspaceToPath(path: path, name: name)
        }
    }

    func saveWorkspaceAs(url: URL) {
        let name = url.deletingPathExtension().lastPathComponent
        engine.saveWorkspaceToPath(path: url.path, name: name)
        currentWorkspacePath = url.path
        currentWorkspaceName = name
    }

    // MARK: - Screenshot

    func captureScreenshot() {
        // Find the best node to screenshot: selected node, or last shader, or last source
        let targetId: String? = selectedNodeId
            ?? nodes.last(where: { !$0.isSource })?.id
            ?? nodes.last?.id

        guard let id = targetId,
              let cgImage = TextureProvider.shared.snapshot(for: id) else {
            return
        }

        let panel = NSSavePanel()
        let timestamp = ISO8601DateFormatter().string(from: Date())
            .replacingOccurrences(of: ":", with: "-")
        panel.nameFieldStringValue = "nottawa_\(timestamp).jpg"
        panel.allowedContentTypes = [.jpeg, .png]

        guard panel.runModal() == .OK, let url = panel.url else { return }

        let nsImage = NSImage(cgImage: cgImage, size: NSSize(width: cgImage.width, height: cgImage.height))
        guard let tiffData = nsImage.tiffRepresentation,
              let bitmap = NSBitmapImageRep(data: tiffData) else { return }

        let isJPEG = url.pathExtension.lowercased() == "jpg" || url.pathExtension.lowercased() == "jpeg"
        let fileType: NSBitmapImageRep.FileType = isJPEG ? .jpeg : .png
        let properties: [NSBitmapImageRep.PropertyKey: Any] = isJPEG ? [.compressionFactor: 0.9] : [:]

        guard let data = bitmap.representation(using: fileType, properties: properties) else { return }

        do {
            try data.write(to: url)
            // Open containing folder in Finder
            NSWorkspace.shared.selectFile(url.path, inFileViewerRootedAtPath: "")
        } catch {
            print("Screenshot save failed: \(error)")
        }
    }

    // MARK: - Library

    func fetchLibrary() {
        libraryLoading = true
        engine.fetchLibraryFiles()
    }

    func refreshLibrary() {
        libraryFiles = engine.libraryFiles()
        libraryLoading = false
    }

    func addLibrarySource(libraryFileId: String, at position: CGPoint? = nil) {
        guard let id = engine.addLibraryVideoSource(libraryFileId: libraryFileId) else {
            print("NottawaApp: Failed to add library source for file ID: \(libraryFileId)")
            return
        }
        let pos = position ?? nextAutoPosition()
        engine.setConnectablePosition(id: id, x: Float(pos.x), y: Float(pos.y))
        lastAddedNodePosition = pos
        refresh()
    }

    func startDownloadPolling() {
        guard downloadPollTimer == nil else { return }
        downloadPollTimer = Timer.scheduledTimer(withTimeInterval: 0.5, repeats: true) { [weak self] _ in
            self?.refreshLibrary()
        }
    }

    func stopDownloadPolling() {
        downloadPollTimer?.invalidate()
        downloadPollTimer = nil
    }

    // MARK: - Strands

    func fetchStrands() {
        strandsList = engine.availableStrands()
    }

    func refreshStrands() {
        strandsList = engine.availableStrands()
    }

    func loadStrand(id: String, at position: CGPoint? = nil) {
        let newIds = engine.loadStrand(id: id)
        if let pos = position {
            // Position the first new node at the drop location
            if let firstId = newIds.first {
                engine.setConnectablePosition(id: firstId, x: Float(pos.x), y: Float(pos.y))
            }
        }
        refresh()
    }

    func deleteStrand(id: String) {
        engine.deleteStrand(id: id)
        refreshStrands()
    }

    func renameStrand(id: String, newName: String) {
        engine.renameStrand(id: id, newName: newName)
        refreshStrands()
    }

    // MARK: - Community Strands

    func fetchCommunityFeed(page: Int = 1) {
        communityLoading = true
        engine.fetchCommunityFeed(page: page, limit: 20, voterId: voterId) { [weak self] strands, totalPages in
            guard let self else { return }
            if page == 1 {
                self.communityStrands = strands
            } else {
                self.communityStrands.append(contentsOf: strands)
            }
            self.communityPage = page
            self.communityTotalPages = totalPages
            self.communityLoading = false
        }
    }

    func voteOnStrand(slug: String, vote: String) {
        // Optimistic update
        if let idx = communityStrands.firstIndex(where: { $0.slug == slug }) {
            let current = communityStrands[idx].userVote
            if vote == "up" {
                communityStrands[idx].userVote = current == "up" ? nil : "up"
                communityStrands[idx].score += current == "up" ? -1 : (current == "down" ? 2 : 1)
                communityStrands[idx].upvotes += current == "up" ? -1 : 1
                if current == "down" { communityStrands[idx].downvotes -= 1 }
            } else if vote == "down" {
                communityStrands[idx].userVote = current == "down" ? nil : "down"
                communityStrands[idx].score += current == "down" ? 1 : (current == "up" ? -2 : -1)
                communityStrands[idx].downvotes += current == "down" ? -1 : 1
                if current == "up" { communityStrands[idx].upvotes -= 1 }
            }
        }

        let voteAction = communityStrands.first(where: { $0.slug == slug })?.userVote == nil ? "none" : vote
        engine.voteOnStrand(slug: slug, voterId: voterId, vote: voteAction) { [weak self] success, upvotes, downvotes, score, userVote in
            guard let self, success else { return }
            if let idx = self.communityStrands.firstIndex(where: { $0.slug == slug }) {
                self.communityStrands[idx].upvotes = upvotes
                self.communityStrands[idx].downvotes = downvotes
                self.communityStrands[idx].score = score
                self.communityStrands[idx].userVote = userVote
            }
        }
    }

    func shareStrand(nodeId: String, title: String, description: String, author: String, previewPng: Data?) {
        shareInProgress = true
        shareResult = nil
        engine.shareStrand(nodeId: nodeId, title: title, description: description,
                          author: author, previewPng: previewPng) { [weak self] result in
            guard let self else { return }
            self.shareInProgress = false
            switch result {
            case .success(let slug):
                self.shareResult = .success(slug: slug)
                self.fetchCommunityFeed(page: 1)
                self.sidebarActiveTab = .strands
                self.strandViewMode = .community
                self.showShareSheet = false
            case .failure(let error):
                self.shareResult = .error(message: error.localizedDescription)
            }
        }
    }

    func loadSharedStrand(slug: String) {
        engine.fetchSharedStrand(slug: slug) { [weak self] result in
            guard let self else { return }
            switch result {
            case .success(let json):
                let newIds = self.engine.loadStrandFromJson(json)
                if !newIds.isEmpty {
                    self.refresh()
                }
            case .failure(let error):
                print("Failed to load shared strand: \(error.localizedDescription)")
            }
        }
    }

    // MARK: - Welcome Screen

    func fetchTemplateStrands() {
        templateStrandsList = engine.availableTemplateStrands()
    }

    func loadDemoStrand() {
        let _ = engine.loadDemoStrand()
        refresh()
        showWelcomeScreen = false
    }

    func dismissWelcomeScreen() {
        showWelcomeScreen = false
    }

    // MARK: - Stage Mode

    /// The connectable ID whose texture should fill the main output view.
    /// Priority: solo > pinned > deepest terminal shader.
    var stageOutputId: String? {
        // Solo overrides everything
        if let soloId = soloedNodeId,
           nodes.contains(where: { $0.id == soloId }) {
            return soloId
        }
        // Pinned output
        if let pinId = pinnedOutputId,
           nodes.contains(where: { $0.id == pinId }) {
            return pinId
        }
        // Default: terminal node (no outgoing connections) with the greatest depth
        let terminals = orderedChainStrips.filter(\.isTerminal)
        return terminals.max(by: { $0.depth < $1.depth })?.node.id
            ?? orderedChainStrips.last?.node.id
    }

    func toggleSolo(_ nodeId: String) {
        if soloedNodeId == nodeId {
            soloedNodeId = nil
        } else {
            soloedNodeId = nodeId
        }
    }

    func toggleBypass(_ nodeId: String) {
        if let idx = nodes.firstIndex(where: { $0.id == nodeId }) {
            nodes[idx].isBypassed.toggle()
            engine.setShaderBypassed(id: nodeId, bypassed: nodes[idx].isBypassed)
        }
    }

    func enterStageMode() {
        savedSidebarState = showSidebar
        showSidebar = false
        deselectAll()
        stageModeEnabled = true
        loadStageStripOrder()
        refreshStageStrips()
        startStageRefreshTimer()
    }

    func exitStageMode() {
        stageModeEnabled = false
        soloedNodeId = nil
        pinnedOutputId = nil
        stopStageRefreshTimer()
        if let saved = savedSidebarState {
            showSidebar = saved
            savedSidebarState = nil
        }
    }

    func moveStageStrip(from source: IndexSet, to destination: Int) {
        orderedChainStrips.move(fromOffsets: source, toOffset: destination)
        stageStripOrder = orderedChainStrips.map(\.node.id)
        saveStageStripOrder()
    }

    func resetStageStripOrder() {
        stageStripOrder = nil
        UserDefaults.standard.removeObject(forKey: "stageStripOrder")
        refreshStageStrips()
    }

    private func saveStageStripOrder() {
        UserDefaults.standard.set(stageStripOrder, forKey: "stageStripOrder")
    }

    private func loadStageStripOrder() {
        stageStripOrder = UserDefaults.standard.stringArray(forKey: "stageStripOrder")
    }

    func toggleStageMode() {
        if stageModeEnabled {
            exitStageMode()
        } else {
            enterStageMode()
        }
    }

    func refreshStageStrips() {
        // Build adjacency and compute topological depth via BFS
        var adjacency: [String: [String]] = [:]   // parent -> children
        var inDegree: [String: Int] = [:]

        let nodeMap = Dictionary(uniqueKeysWithValues: nodes.map { ($0.id, $0) })

        for node in nodes {
            inDegree[node.id] = 0
            adjacency[node.id] = []
        }

        for conn in connections {
            adjacency[conn.startNodeId, default: []].append(conn.endNodeId)
            inDegree[conn.endNodeId, default: 0] += 1
        }

        // BFS from roots (nodes with no incoming edges)
        var depth: [String: Int] = [:]
        var chainIndex: [String: Int] = [:]
        var queue: [String] = []
        var currentChain = 0

        let roots = nodes.filter { (inDegree[$0.id] ?? 0) == 0 }.map(\.id)

        for root in roots {
            if depth[root] != nil { continue }
            depth[root] = 0
            chainIndex[root] = currentChain
            queue.append(root)

            while !queue.isEmpty {
                let current = queue.removeFirst()
                let currentDepth = depth[current] ?? 0
                let chain = chainIndex[current] ?? currentChain

                for child in adjacency[current] ?? [] {
                    let newDepth = currentDepth + 1
                    if depth[child] == nil || newDepth > depth[child]! {
                        depth[child] = newDepth
                        chainIndex[child] = chain
                        queue.append(child)
                    }
                }
            }
            currentChain += 1
        }

        // Build strips, sorted by chain then depth
        var strips: [ChainStrip] = []
        for node in nodes {
            let d = depth[node.id] ?? 0
            let c = chainIndex[node.id] ?? 0
            let params = engine.parameters(for: node.id).filter { $0.parameterType != .hidden }
            let favoriteParams = params.filter(\.isFavorited)
            let terminal = (adjacency[node.id] ?? []).isEmpty
            strips.append(ChainStrip(
                chainIndex: c,
                depth: d,
                node: node,
                favoriteParams: favoriteParams,
                allParams: params,
                isTerminal: terminal
            ))
        }

        strips.sort {
            if $0.chainIndex != $1.chainIndex { return $0.chainIndex < $1.chainIndex }
            return $0.depth < $1.depth
        }

        // Apply user-defined custom order if set
        if let customOrder = stageStripOrder {
            let stripMap = Dictionary(uniqueKeysWithValues: strips.map { ($0.node.id, $0) })
            var reordered: [ChainStrip] = []
            for id in customOrder {
                if let strip = stripMap[id] {
                    reordered.append(strip)
                }
            }
            // Append any new strips not in the custom order
            for strip in strips where !customOrder.contains(strip.node.id) {
                reordered.append(strip)
            }
            strips = reordered
        }

        orderedChainStrips = strips
    }

    // MARK: - Preview Window Helpers

    func refreshOpenWindows() {
        openPreviewWindowIds = PreviewWindowManager.shared.openNodeIds
    }

    func openPreviewWindow(_ nodeId: String) {
        let name = nodes.first(where: { $0.id == nodeId })?.name ?? "Preview"
        PreviewWindowManager.shared.openPreview(for: nodeId, nodeName: name)
        refreshOpenWindows()
    }

    func closePreviewWindow(_ nodeId: String) {
        PreviewWindowManager.shared.closePreview(for: nodeId)
        refreshOpenWindows()
    }

    private func startStageRefreshTimer() {
        stopStageRefreshTimer()
        stageRefreshTimer = Timer.scheduledTimer(withTimeInterval: 0.5, repeats: true) { [weak self] _ in
            guard let self, self.stageModeEnabled else { return }
            self.refreshStageStrips()
            self.refreshOpenWindows()
        }
    }

    private func stopStageRefreshTimer() {
        stageRefreshTimer?.invalidate()
        stageRefreshTimer = nil
    }

    // MARK: - Cleanup

    func cleanup() {
        for id in sharingTextureIds {
            engine.stopTextureSharing(connectableId: id)
        }
        sharingTextureIds.removeAll()
        TextureProvider.shared.invalidateAll()
        nodeDownloadPollTimer?.invalidate()
        nodeDownloadPollTimer = nil
        stopStageRefreshTimer()
    }
}

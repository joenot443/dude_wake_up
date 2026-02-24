//
//  NodeEditorView.swift
//  NottawaApp
//
//  Main node editor canvas container with grid, cables, and node overlay.
//

import SwiftUI
import AppKit

// MARK: - Right-Click Context Menu + Pan (NSViewRepresentable)

/// Transparent overlay that captures right-mouse events for canvas panning
/// and context menus. Returns nil from hitTest for non-right-click events,
/// so left-clicks pass through to SwiftUI views underneath.
struct CanvasPanGestureView: NSViewRepresentable {
    var canvasOffset: CGPoint
    var viewModel: NodeEditorViewModel
    var onPan: (CGPoint) -> Void

    func makeCoordinator() -> Coordinator {
        Coordinator(viewModel: viewModel)
    }

    func makeNSView(context: Context) -> PanNSView {
        let view = PanNSView()
        view.coordinator = context.coordinator
        context.coordinator.canvasOffset = canvasOffset
        context.coordinator.onPan = onPan
        return view
    }

    func updateNSView(_ nsView: PanNSView, context: Context) {
        context.coordinator.canvasOffset = canvasOffset
        context.coordinator.onPan = onPan
    }

    class Coordinator {
        let viewModel: NodeEditorViewModel
        var canvasOffset: CGPoint = .zero
        var onPan: ((CGPoint) -> Void)?
        var startOffset: CGPoint?
        var startPoint: NSPoint?

        // Node layout constants (must match NodeView / NodeDragOverlayView)
        let nodeWidth: CGFloat = 260
        let previewHeight: CGFloat = 146
        let pinBarHeight: CGFloat = 28
        let buttonBarHeight: CGFloat = 24
        var totalNodeHeight: CGFloat { previewHeight + pinBarHeight + buttonBarHeight }

        init(viewModel: NodeEditorViewModel) {
            self.viewModel = viewModel
        }

        /// Hit-test for a node. Returns the node ID if hit.
        func hitTestNode(at viewPoint: CGPoint) -> String? {
            let vm = viewModel
            for node in vm.nodes.reversed() {
                let screenX = node.position.x * vm.canvasScale + vm.canvasOffset.x
                let screenY = node.position.y * vm.canvasScale + vm.canvasOffset.y
                let scaledW = nodeWidth * vm.canvasScale
                let scaledH = totalNodeHeight * vm.canvasScale

                let nodeRect = CGRect(
                    x: screenX - scaledW / 2,
                    y: screenY - scaledH / 2,
                    width: scaledW,
                    height: scaledH
                )

                if nodeRect.contains(viewPoint) {
                    return node.id
                }
            }
            return nil
        }

        /// Hit-test for a pin at the given screen point. Uses the shared pin positions dict.
        func hitTestPin(at viewPoint: CGPoint) -> PinKey? {
            let pinThreshold: CGFloat = 15
            var bestKey: PinKey?
            var bestDist = CGFloat.infinity
            for (key, pinPos) in viewModel.pinPositions {
                let dist = hypot(viewPoint.x - pinPos.x, viewPoint.y - pinPos.y)
                if dist < pinThreshold && dist < bestDist {
                    bestDist = dist
                    bestKey = key
                }
            }
            return bestKey
        }

        /// Find the connection for a specific pin.
        func connectionForPin(_ pin: PinKey) -> ConnectionModel? {
            if pin.isOutput {
                return viewModel.connections.first { $0.startNodeId == pin.nodeId && $0.outputSlot == pin.slotIndex }
            } else {
                return viewModel.connections.first { $0.endNodeId == pin.nodeId && $0.inputSlot == pin.slotIndex }
            }
        }

        func showContextMenu(for nodeId: String?, at point: NSPoint, in view: NSView, pinKey: PinKey? = nil) {
            let menu = NSMenu()

            // If right-clicked on a connected pin, show a focused "Break Connection" menu
            if let pinKey = pinKey, let conn = connectionForPin(pinKey) {
                let otherNodeId = pinKey.isOutput ? conn.endNodeId : conn.startNodeId
                let otherName = viewModel.nodes.first(where: { $0.id == otherNodeId })?.name ?? "Unknown"
                let direction = pinKey.isOutput ? "→" : "←"
                let breakItem = NSMenuItem(
                    title: "Break Connection \(direction) \(otherName)",
                    action: #selector(PanNSView.contextMenuBreakConnection(_:)),
                    keyEquivalent: ""
                )
                breakItem.representedObject = conn.id
                breakItem.target = view
                menu.addItem(breakItem)
                menu.popUp(positioning: nil, at: point, in: view)
                return
            }

            if let nodeId = nodeId {
                let node = viewModel.nodes.first(where: { $0.id == nodeId })
                let nodeName = node?.name ?? "Node"

                // Select the node if not already selected
                if !viewModel.selectedNodeIds.contains(nodeId) {
                    viewModel.selectNode(nodeId)
                }

                // Delete
                let deleteItem = NSMenuItem(title: "Delete \(nodeName)", action: #selector(PanNSView.contextMenuDelete(_:)), keyEquivalent: "")
                deleteItem.representedObject = nodeId
                deleteItem.target = view
                menu.addItem(deleteItem)

                // Break Connections submenu
                let nodeConnections = viewModel.connectionsForNode(nodeId)
                if !nodeConnections.isEmpty {
                    let breakMenu = NSMenu()
                    for conn in nodeConnections {
                        let otherName = viewModel.nodeNameForConnection(conn, relativeTo: nodeId)
                        let direction = conn.startNodeId == nodeId ? "→" : "←"
                        let breakItem = NSMenuItem(
                            title: "\(direction) \(otherName)",
                            action: #selector(PanNSView.contextMenuBreakConnection(_:)),
                            keyEquivalent: ""
                        )
                        breakItem.representedObject = conn.id
                        breakItem.target = view
                        breakMenu.addItem(breakItem)
                    }

                    // Break All
                    if nodeConnections.count > 1 {
                        breakMenu.addItem(.separator())
                        let breakAllItem = NSMenuItem(
                            title: "Break All",
                            action: #selector(PanNSView.contextMenuBreakAllConnections(_:)),
                            keyEquivalent: ""
                        )
                        breakAllItem.representedObject = nodeId
                        breakAllItem.target = view
                        breakMenu.addItem(breakAllItem)
                    }

                    let breakSubmenu = NSMenuItem(title: "Break Connections", action: nil, keyEquivalent: "")
                    breakSubmenu.submenu = breakMenu
                    menu.addItem(breakSubmenu)
                }

                menu.addItem(.separator())

                // Blend (if exactly 2 selected)
                if viewModel.selectedNodeIds.count == 2 {
                    let blendItem = NSMenuItem(
                        title: "Blend Selected",
                        action: #selector(PanNSView.contextMenuBlend(_:)),
                        keyEquivalent: ""
                    )
                    blendItem.target = view
                    menu.addItem(blendItem)
                }

                // Swap shader/source
                if node?.isShader == true {
                    let swapItem = NSMenuItem(
                        title: "Swap Shader...",
                        action: #selector(PanNSView.contextMenuSwapShader(_:)),
                        keyEquivalent: ""
                    )
                    swapItem.representedObject = nodeId
                    swapItem.target = view
                    menu.addItem(swapItem)

                    // Edit Shader File
                    let editItem = NSMenuItem(
                        title: "Edit Shader File",
                        action: #selector(PanNSView.contextMenuEditShaderFile(_:)),
                        keyEquivalent: ""
                    )
                    editItem.representedObject = nodeId
                    editItem.target = view
                    menu.addItem(editItem)
                } else if node?.isSource == true {
                    let swapItem = NSMenuItem(
                        title: "Swap Source...",
                        action: #selector(PanNSView.contextMenuSwapSource(_:)),
                        keyEquivalent: ""
                    )
                    swapItem.representedObject = nodeId
                    swapItem.target = view
                    menu.addItem(swapItem)
                }

                menu.addItem(.separator())

                // Preview Window
                let previewItem = NSMenuItem(
                    title: "Open Preview Window",
                    action: #selector(PanNSView.contextMenuPreviewWindow(_:)),
                    keyEquivalent: ""
                )
                previewItem.representedObject = nodeId
                previewItem.target = view
                menu.addItem(previewItem)

                // Save Strand
                let strandItem = NSMenuItem(
                    title: "Save Strand...",
                    action: #selector(PanNSView.contextMenuSaveStrand(_:)),
                    keyEquivalent: ""
                )
                strandItem.representedObject = nodeId
                strandItem.target = view
                menu.addItem(strandItem)

                // Share Strand
                let shareItem = NSMenuItem(
                    title: "Share Strand...",
                    action: #selector(PanNSView.contextMenuShareStrand(_:)),
                    keyEquivalent: ""
                )
                shareItem.representedObject = nodeId
                shareItem.target = view
                menu.addItem(shareItem)
            } else {
                // Canvas context menu (no node)
                let addShaderItem = NSMenuItem(
                    title: "Add Shader...",
                    action: #selector(PanNSView.contextMenuAddShader(_:)),
                    keyEquivalent: ""
                )
                addShaderItem.target = view
                menu.addItem(addShaderItem)

                let addSourceItem = NSMenuItem(
                    title: "Add Source...",
                    action: #selector(PanNSView.contextMenuAddSource(_:)),
                    keyEquivalent: ""
                )
                addSourceItem.target = view
                menu.addItem(addSourceItem)
            }

            menu.popUp(positioning: nil, at: point, in: view)
        }
    }

    class PanNSView: NSView {
        var coordinator: Coordinator?

        override func hitTest(_ point: NSPoint) -> NSView? {
            // Pass all events through when browser is open so SwiftUI overlay receives clicks
            if coordinator?.viewModel.showShaderBrowser == true ||
               coordinator?.viewModel.showSourceBrowser == true {
                return nil
            }
            // Intercept right-click, scroll, and magnify events; pass everything else through.
            guard let event = window?.currentEvent,
                  event.type == .rightMouseDown ||
                  event.type == .rightMouseDragged ||
                  event.type == .rightMouseUp ||
                  event.type == .scrollWheel ||
                  event.type == .magnify
            else { return nil }
            return super.hitTest(point)
        }

        override func scrollWheel(with event: NSEvent) {
            guard let coordinator else { return }
            let delta = event.hasPreciseScrollingDeltas ? event.scrollingDeltaY * 0.003 : event.scrollingDeltaY * 0.01
            let oldScale = coordinator.viewModel.canvasScale
            let newScale = max(0.2, min(3.0, oldScale * (1.0 + delta)))
            zoomTowardCursor(event: event, oldScale: oldScale, newScale: newScale)
        }

        override func magnify(with event: NSEvent) {
            guard let coordinator else { return }
            let oldScale = coordinator.viewModel.canvasScale
            let newScale = max(0.2, min(3.0, oldScale * (1.0 + event.magnification)))
            zoomTowardCursor(event: event, oldScale: oldScale, newScale: newScale)
        }

        /// Adjust scale and offset so the canvas point under the cursor stays fixed.
        private func zoomTowardCursor(event: NSEvent, oldScale: CGFloat, newScale: CGFloat) {
            guard let coordinator else { return }
            let cursorNS = convert(event.locationInWindow, from: nil)
            // NSView origin is bottom-left; SwiftUI is top-left
            let cursor = CGPoint(x: cursorNS.x, y: bounds.height - cursorNS.y)

            let offset = coordinator.viewModel.canvasOffset
            // Canvas coordinate under the cursor
            let canvasX = (cursor.x - offset.x) / oldScale
            let canvasY = (cursor.y - offset.y) / oldScale

            // New offset keeps that canvas point at the same screen position
            coordinator.viewModel.canvasOffset = CGPoint(
                x: cursor.x - canvasX * newScale,
                y: cursor.y - canvasY * newScale
            )
            coordinator.viewModel.canvasScale = newScale
        }

        override func rightMouseDown(with event: NSEvent) {
            guard let coordinator else { return }
            coordinator.startPoint = convert(event.locationInWindow, from: nil)
            coordinator.startOffset = coordinator.canvasOffset
        }

        override func rightMouseDragged(with event: NSEvent) {
            guard let coordinator,
                  let start = coordinator.startPoint,
                  let startOff = coordinator.startOffset else { return }
            let current = convert(event.locationInWindow, from: nil)
            let newOffset = CGPoint(
                x: startOff.x + (current.x - start.x),
                y: startOff.y - (current.y - start.y) // Flip Y (NSView → SwiftUI)
            )
            coordinator.onPan?(newOffset)
        }

        override func rightMouseUp(with event: NSEvent) {
            guard let coordinator else { return }

            if let start = coordinator.startPoint {
                let current = convert(event.locationInWindow, from: nil)
                let dist = hypot(current.x - start.x, current.y - start.y)

                if dist < 4 {
                    // Short right-click = context menu
                    // Convert to flipped coordinates for hit testing
                    let flippedY = bounds.height - current.y
                    let viewPoint = CGPoint(x: current.x, y: flippedY)
                    // Check if right-click is on a connected pin
                    let pinKey = coordinator.hitTestPin(at: viewPoint)
                    let nodeId = coordinator.hitTestNode(at: viewPoint)
                    coordinator.showContextMenu(for: nodeId, at: current, in: self, pinKey: pinKey)
                }
            }

            coordinator.startPoint = nil
            coordinator.startOffset = nil
        }

        // MARK: - Context Menu Actions

        @objc func contextMenuDelete(_ sender: NSMenuItem) {
            guard let nodeId = sender.representedObject as? String else { return }
            DispatchQueue.main.async { [weak self] in
                self?.coordinator?.viewModel.deleteNode(nodeId)
            }
        }

        @objc func contextMenuBreakConnection(_ sender: NSMenuItem) {
            guard let connId = sender.representedObject as? String else { return }
            DispatchQueue.main.async { [weak self] in
                self?.coordinator?.viewModel.removeConnection(connId)
            }
        }

        @objc func contextMenuBreakAllConnections(_ sender: NSMenuItem) {
            guard let nodeId = sender.representedObject as? String else { return }
            DispatchQueue.main.async { [weak self] in
                guard let vm = self?.coordinator?.viewModel else { return }
                let conns = vm.connectionsForNode(nodeId)
                for conn in conns {
                    vm.removeConnection(conn.id)
                }
            }
        }

        @objc func contextMenuBlend(_ sender: NSMenuItem) {
            DispatchQueue.main.async { [weak self] in
                self?.coordinator?.viewModel.blendSelectedNodes()
            }
        }

        @objc func contextMenuSwapShader(_ sender: NSMenuItem) {
            guard let nodeId = sender.representedObject as? String else { return }
            DispatchQueue.main.async { [weak self] in
                guard let vm = self?.coordinator?.viewModel else { return }
                vm.swapTargetNodeId = nodeId
                vm.browserMode = .swap
                vm.showShaderBrowser = true
                vm.showSourceBrowser = false
            }
        }

        @objc func contextMenuSwapSource(_ sender: NSMenuItem) {
            guard let nodeId = sender.representedObject as? String else { return }
            DispatchQueue.main.async { [weak self] in
                guard let vm = self?.coordinator?.viewModel else { return }
                vm.swapTargetNodeId = nodeId
                vm.browserMode = .swap
                vm.showSourceBrowser = true
                vm.showShaderBrowser = false
            }
        }

        @objc func contextMenuEditShaderFile(_ sender: NSMenuItem) {
            guard let nodeId = sender.representedObject as? String else { return }
            if let path = NottawaEngine.shared.shaderFilePath(shaderId: nodeId) {
                NSWorkspace.shared.open(URL(fileURLWithPath: path))
            }
        }

        @objc func contextMenuPreviewWindow(_ sender: NSMenuItem) {
            guard let nodeId = sender.representedObject as? String else { return }
            let nodeName = coordinator?.viewModel.nodes.first(where: { $0.id == nodeId })?.name ?? "Preview"
            DispatchQueue.main.async {
                PreviewWindowManager.shared.openPreview(for: nodeId, nodeName: nodeName)
            }
        }

        @objc func contextMenuSaveStrand(_ sender: NSMenuItem) {
            guard let nodeId = sender.representedObject as? String else { return }
            let nodeName = coordinator?.viewModel.nodes.first(where: { $0.id == nodeId })?.name ?? "Strand"
            DispatchQueue.main.async {
                let panel = NSSavePanel()
                panel.nameFieldStringValue = "\(nodeName).strand"
                panel.allowedContentTypes = [.json]
                panel.begin { response in
                    if response == .OK, let url = panel.url {
                        NottawaEngine.shared.saveStrand(nodeId: nodeId, path: url.path, name: nodeName)
                    }
                }
            }
        }

        @objc func contextMenuShareStrand(_ sender: NSMenuItem) {
            guard let nodeId = sender.representedObject as? String else { return }
            DispatchQueue.main.async { [weak self] in
                guard let vm = self?.coordinator?.viewModel else { return }
                vm.shareNodeId = nodeId
                vm.showShareSheet = true
            }
        }

        @objc func contextMenuAddShader(_ sender: NSMenuItem) {
            DispatchQueue.main.async { [weak self] in
                guard let vm = self?.coordinator?.viewModel else { return }
                vm.sidebarActiveTab = .shaders
                vm.showSidebar = true
            }
        }

        @objc func contextMenuAddSource(_ sender: NSMenuItem) {
            DispatchQueue.main.async { [weak self] in
                guard let vm = self?.coordinator?.viewModel else { return }
                vm.sidebarActiveTab = .sources
                vm.showSidebar = true
            }
        }
    }
}

// MARK: - Node Drag Overlay (NSViewRepresentable)

/// Transparent overlay that handles left-click drag for moving nodes.
/// Uses AppKit mouse events directly, bypassing SwiftUI gesture system
/// which has issues with MTKView event interception and gesture competition.
/// Passes through events for pins (connection drawing) and buttons.
struct NodeDragOverlayView: NSViewRepresentable {
    let viewModel: NodeEditorViewModel

    func makeCoordinator() -> Coordinator {
        Coordinator(viewModel: viewModel)
    }

    func makeNSView(context: Context) -> NodeDragNSView {
        let view = NodeDragNSView()
        view.coordinator = context.coordinator
        view.wantsLayer = true
        view.layerContentsRedrawPolicy = .onSetNeedsDisplay
        view.registerForDraggedTypes([.string])
        return view
    }

    func updateNSView(_ nsView: NodeDragNSView, context: Context) {}

    // MARK: - Coordinator

    class Coordinator {
        let viewModel: NodeEditorViewModel
        var draggedNodeId: String?
        var dragStartMousePoint: CGPoint?
        var dragStartNodePosition: CGPoint?
        var isDragging = false
        var isMultiDrag = false

        // Pin drag state
        var isDraggingPin = false

        // Node layout constants (must match NodeView)
        let nodeWidth: CGFloat = 260
        let previewHeight: CGFloat = 146
        let pinBarHeight: CGFloat = 28
        let buttonBarHeight: CGFloat = 24
        var totalNodeHeight: CGFloat { previewHeight + pinBarHeight + buttonBarHeight }

        init(viewModel: NodeEditorViewModel) {
            self.viewModel = viewModel
        }

        /// Returns the PinKey if the point is near a pin, nil otherwise.
        func hitTestPin(at viewPoint: CGPoint) -> PinKey? {
            let pinThreshold: CGFloat = 15
            var bestKey: PinKey?
            var bestDist = CGFloat.infinity
            for (key, pinPos) in viewModel.pinPositions {
                let dist = hypot(viewPoint.x - pinPos.x, viewPoint.y - pinPos.y)
                if dist < pinThreshold && dist < bestDist {
                    bestDist = dist
                    bestKey = key
                }
            }
            return bestKey
        }

        /// Returns true if the point is on a button bar that should pass through to SwiftUI.
        func hitTestButton(at viewPoint: CGPoint) -> Bool {
            let vm = viewModel
            for node in vm.nodes {
                let screenX = node.position.x * vm.canvasScale + vm.canvasOffset.x
                let screenY = node.position.y * vm.canvasScale + vm.canvasOffset.y
                let scaledW = nodeWidth * vm.canvasScale
                let scaledH = totalNodeHeight * vm.canvasScale

                let nodeRect = CGRect(
                    x: screenX - scaledW / 2,
                    y: screenY - scaledH / 2,
                    width: scaledW,
                    height: scaledH
                )

                guard nodeRect.contains(viewPoint) else { continue }

                let buttonAreaTop = nodeRect.maxY - buttonBarHeight * vm.canvasScale
                if viewPoint.y > buttonAreaTop {
                    return true
                }
            }
            return false
        }

        /// Hit-test for a node's draggable area (body, not pins or buttons).
        /// Returns the node ID if hit, nil if empty canvas. Checks topmost nodes first.
        func hitTestNode(at viewPoint: CGPoint) -> String? {
            let vm = viewModel
            for node in vm.nodes.reversed() {
                let screenX = node.position.x * vm.canvasScale + vm.canvasOffset.x
                let screenY = node.position.y * vm.canvasScale + vm.canvasOffset.y
                let scaledW = nodeWidth * vm.canvasScale
                let scaledH = totalNodeHeight * vm.canvasScale

                let nodeRect = CGRect(
                    x: screenX - scaledW / 2,
                    y: screenY - scaledH / 2,
                    width: scaledW,
                    height: scaledH
                )

                if nodeRect.contains(viewPoint) {
                    return node.id
                }
            }
            return nil
        }
    }

    // MARK: - NSView

    class NodeDragNSView: NSView {
        var coordinator: Coordinator?

        // Drag cable endpoints for Core Graphics rendering.
        // Stored directly on the NSView so draw() doesn't need SwiftUI.
        var cableFromPos: CGPoint?
        var cableToPos: CGPoint?
        var cableFromIsOutput: Bool = true
        var cableSnapped: Bool = false
        var cableIsBlendSnap: Bool = false

        override var isFlipped: Bool { true }

        override func draw(_ dirtyRect: NSRect) {
            guard let from = cableFromPos, let to = cableToPos,
                  let context = NSGraphicsContext.current?.cgContext else { return }

            let startPos = cableFromIsOutput ? from : to
            let endPos = cableFromIsOutput ? to : from
            let dx = abs(endPos.x - startPos.x)
            let dy = abs(endPos.y - startPos.y)
            let controlOffset = max(dx * 0.5, dy * 0.25, 60)

            // Cable color: green when snapped to valid target, cyan otherwise
            let cableColor: NSColor = cableSnapped
                ? NSColor.systemGreen.withAlphaComponent(0.9)
                : NSColor.systemCyan.withAlphaComponent(0.7)

            // Glow layer (wider, semi-transparent)
            context.saveGState()
            context.setStrokeColor(cableColor.withAlphaComponent(0.2).cgColor)
            context.setLineWidth(8)
            context.setLineDash(phase: 0, lengths: [])
            context.move(to: startPos)
            context.addCurve(
                to: endPos,
                control1: CGPoint(x: startPos.x + controlOffset, y: startPos.y),
                control2: CGPoint(x: endPos.x - controlOffset, y: endPos.y)
            )
            context.strokePath()
            context.restoreGState()

            // Main cable
            context.setStrokeColor(cableColor.cgColor)
            context.setLineWidth(3)
            context.setLineDash(phase: 0, lengths: cableSnapped ? [] : [6, 4])

            context.move(to: startPos)
            context.addCurve(
                to: endPos,
                control1: CGPoint(x: startPos.x + controlOffset, y: startPos.y),
                control2: CGPoint(x: endPos.x - controlOffset, y: endPos.y)
            )
            context.strokePath()

            // Snap indicator dot at endpoint when snapped
            if cableSnapped {
                let dotRadius: CGFloat = 5
                context.setFillColor(cableColor.cgColor)
                context.fillEllipse(in: CGRect(
                    x: endPos.x - dotRadius, y: endPos.y - dotRadius,
                    width: dotRadius * 2, height: dotRadius * 2
                ))

                // "Blend?" label for output→output connections
                if cableIsBlendSnap {
                    let midX = (startPos.x + endPos.x) / 2
                    let midY = (startPos.y + endPos.y) / 2
                    let label = "Blend?" as NSString
                    let font = NSFont.systemFont(ofSize: 13, weight: .semibold)
                    let attrs: [NSAttributedString.Key: Any] = [
                        .font: font,
                        .foregroundColor: NSColor.white,
                    ]
                    let size = label.size(withAttributes: attrs)
                    let padding: CGFloat = 6
                    let pillRect = CGRect(
                        x: midX - size.width / 2 - padding,
                        y: midY - size.height / 2 - padding - 16,
                        width: size.width + padding * 2,
                        height: size.height + padding * 2
                    )

                    // Pill background
                    context.saveGState()
                    let pillPath = CGPath(roundedRect: pillRect, cornerWidth: 8, cornerHeight: 8, transform: nil)
                    context.addPath(pillPath)
                    context.setFillColor(NSColor.systemPurple.withAlphaComponent(0.85).cgColor)
                    context.fillPath()
                    context.restoreGState()

                    // Text
                    NSGraphicsContext.saveGraphicsState()
                    NSGraphicsContext.current = NSGraphicsContext(cgContext: context, flipped: true)
                    label.draw(
                        at: CGPoint(x: pillRect.minX + padding, y: pillRect.minY + padding),
                        withAttributes: attrs
                    )
                    NSGraphicsContext.restoreGraphicsState()
                }
            }
        }

        override func hitTest(_ point: NSPoint) -> NSView? {
            guard let coordinator else { return nil }

            // Pass all events through when browser is open so SwiftUI overlay receives clicks
            if coordinator.viewModel.showShaderBrowser ||
               coordinator.viewModel.showSourceBrowser {
                return nil
            }

            // If we're already tracking a drag, keep receiving events
            if coordinator.isDragging {
                return self
            }

            // Only intercept left mouseDown
            guard let event = window?.currentEvent,
                  event.type == .leftMouseDown else {
                return nil
            }

            let viewPoint = convert(point, from: superview)

            // Buttons pass through to SwiftUI
            if coordinator.hitTestButton(at: viewPoint) {
                return nil
            }

            // Pins AND node bodies are handled at AppKit level
            return self
        }

        override func mouseDown(with event: NSEvent) {
            guard let coordinator else { return }
            let point = convert(event.locationInWindow, from: nil)
            let isCmd = event.modifierFlags.contains(.command)

            // Double-click → preview window
            if event.clickCount == 2 {
                if let nodeId = coordinator.hitTestNode(at: point) {
                    let nodeName = coordinator.viewModel.nodes.first(where: { $0.id == nodeId })?.name ?? "Preview"
                    DispatchQueue.main.async {
                        PreviewWindowManager.shared.openPreview(for: nodeId, nodeName: nodeName)
                    }
                }
                return
            }

            // Pin click → start connection drag
            if let pinKey = coordinator.hitTestPin(at: point),
               let pinPos = coordinator.viewModel.pinPositions[pinKey] {
                coordinator.isDragging = true
                coordinator.isDraggingPin = true
                coordinator.dragStartMousePoint = point
                coordinator.viewModel.dragConnection = DragConnectionState(
                    fromNodeId: pinKey.nodeId,
                    fromSlotIndex: pinKey.slotIndex,
                    fromIsOutput: pinKey.isOutput,
                    fromPosition: pinPos,
                    currentPosition: point
                )
                // Compute valid targets for highlighting
                coordinator.viewModel.updateDragTargets(at: point)
                // Set Core Graphics cable state and redraw
                cableFromPos = pinPos
                cableToPos = point
                cableFromIsOutput = pinKey.isOutput
                cableSnapped = false
                cableIsBlendSnap = false
                needsDisplay = true
                return
            }

            if let nodeId = coordinator.hitTestNode(at: point) {
                coordinator.draggedNodeId = nodeId
                coordinator.dragStartMousePoint = point
                coordinator.isDragging = true

                if isCmd {
                    // Cmd+Click: toggle selection
                    coordinator.viewModel.toggleNodeSelection(nodeId)
                } else if !coordinator.viewModel.selectedNodeIds.contains(nodeId) {
                    // Click on unselected node: exclusive select
                    coordinator.viewModel.selectNode(nodeId)
                }
                // If clicking on an already-selected node (no Cmd), keep selection for multi-drag

                // Set up for multi-drag if multiple are selected
                if coordinator.viewModel.selectedNodeIds.count > 1 &&
                   coordinator.viewModel.selectedNodeIds.contains(nodeId) {
                    coordinator.isMultiDrag = true
                    coordinator.viewModel.captureMultiDragStartPositions()
                } else {
                    coordinator.isMultiDrag = false
                    if let node = coordinator.viewModel.nodes.first(where: { $0.id == nodeId }) {
                        coordinator.dragStartNodePosition = node.position
                    }
                }
            } else {
                // Empty canvas — deselect and forward event for window management
                coordinator.viewModel.deselectAll()
                super.mouseDown(with: event)
            }
        }

        override func mouseDragged(with event: NSEvent) {
            guard let coordinator,
                  let startMouse = coordinator.dragStartMousePoint else { return }

            let point = convert(event.locationInWindow, from: nil)

            // Pin drag — update cable endpoint with magnetic snap
            if coordinator.isDraggingPin {
                coordinator.viewModel.dragConnection?.currentPosition = point
                coordinator.viewModel.updateDragTargets(at: point)

                // Snap cable endpoint to pin center if within magnetic radius
                if let snapKey = coordinator.viewModel.dragSnapPinKey,
                   let snapPos = coordinator.viewModel.pinPositions[snapKey] {
                    cableToPos = snapPos
                    cableSnapped = true
                    cableIsBlendSnap = cableFromIsOutput && snapKey.isOutput
                } else {
                    cableToPos = point
                    cableSnapped = false
                    cableIsBlendSnap = false
                }
                needsDisplay = true
                return
            }

            let scale = coordinator.viewModel.canvasScale
            let dx = (point.x - startMouse.x) / scale
            let dy = (point.y - startMouse.y) / scale

            if coordinator.isMultiDrag {
                coordinator.viewModel.moveSelectedNodesLocal(by: CGPoint(x: dx, y: dy))
            } else if let nodeId = coordinator.draggedNodeId,
                      let startNode = coordinator.dragStartNodePosition {
                let newPos = CGPoint(x: startNode.x + dx, y: startNode.y + dy)
                coordinator.viewModel.setNodePositionLocal(nodeId, to: newPos)
            }
        }

        override func mouseUp(with event: NSEvent) {
            guard let coordinator else { return }

            // Pin drag end — use snapped pin or hit-test target, then create connection
            if coordinator.isDraggingPin {
                let point = convert(event.locationInWindow, from: nil)
                if let drag = coordinator.viewModel.dragConnection {
                    // Prefer the magnetically snapped pin; fall back to hit-test
                    let snappedKey = coordinator.viewModel.dragSnapPinKey
                    let targetIsInput = drag.fromIsOutput

                    if let targetPin = snappedKey ?? coordinator.viewModel.hitTestPin(
                        at: point,
                        excluding: drag.fromNodeId,
                        outputOnly: !targetIsInput,
                        inputOnly: targetIsInput
                    ) {
                        if targetPin.isOutput == !drag.fromIsOutput {
                            // Normal connection: output→input
                            let startId = drag.fromIsOutput ? drag.fromNodeId : targetPin.nodeId
                            let endId = drag.fromIsOutput ? targetPin.nodeId : drag.fromNodeId
                            let outSlot = drag.fromIsOutput ? drag.fromSlotIndex : targetPin.slotIndex
                            let inSlot = drag.fromIsOutput ? targetPin.slotIndex : drag.fromSlotIndex
                            coordinator.viewModel.makeConnection(
                                startId: startId, endId: endId,
                                outputSlot: outSlot, inputSlot: inSlot
                            )
                        } else if drag.fromIsOutput && targetPin.isOutput {
                            // Output→Output → create Blend
                            coordinator.viewModel.createBlendBetween(
                                id1: drag.fromNodeId, id2: targetPin.nodeId
                            )
                        }
                    }
                }
                coordinator.viewModel.dragConnection = nil
                coordinator.viewModel.clearDragTargets()
                coordinator.isDraggingPin = false
                coordinator.isDragging = false
                coordinator.dragStartMousePoint = nil
                // Clear Core Graphics cable
                cableFromPos = nil
                cableToPos = nil
                cableSnapped = false
                cableIsBlendSnap = false
                needsDisplay = true
                return
            }

            if let startMouse = coordinator.dragStartMousePoint {
                let point = convert(event.locationInWindow, from: nil)
                let dist = hypot(point.x - startMouse.x, point.y - startMouse.y)

                if dist > 4 {
                    if coordinator.isMultiDrag {
                        coordinator.viewModel.commitSelectedNodePositions()
                    } else if let nodeId = coordinator.draggedNodeId,
                              let startNode = coordinator.dragStartNodePosition {
                        let scale = coordinator.viewModel.canvasScale
                        let dx = (point.x - startMouse.x) / scale
                        let dy = (point.y - startMouse.y) / scale
                        let newPos = CGPoint(x: startNode.x + dx, y: startNode.y + dy)
                        coordinator.viewModel.moveNode(nodeId, to: newPos)
                    }
                }
                // If dist <= 4, it's a tap — selection already happened in mouseDown
            }

            coordinator.draggedNodeId = nil
            coordinator.dragStartMousePoint = nil
            coordinator.dragStartNodePosition = nil
            coordinator.isDragging = false
            coordinator.isMultiDrag = false
        }

        // MARK: - NSDraggingDestination (drop from sidebar)

        override func draggingEntered(_ sender: NSDraggingInfo) -> NSDragOperation {
            DispatchQueue.main.async { [weak self] in
                self?.coordinator?.viewModel.isDropTargetActive = true
            }
            return .copy
        }

        override func draggingUpdated(_ sender: NSDraggingInfo) -> NSDragOperation {
            return .copy
        }

        override func draggingExited(_ sender: NSDraggingInfo?) {
            DispatchQueue.main.async { [weak self] in
                self?.coordinator?.viewModel.isDropTargetActive = false
            }
        }

        override func performDragOperation(_ sender: NSDraggingInfo) -> Bool {
            guard let coordinator,
                  let payload = sender.draggingPasteboard.string(forType: .string) else { return false }
            let dropPoint = convert(sender.draggingLocation, from: nil)
            let targetNodeId = coordinator.hitTestNode(at: dropPoint)
            DispatchQueue.main.async {
                coordinator.viewModel.isDropTargetActive = false
                coordinator.viewModel.addNodeFromDrop(payload: payload, at: CGPoint(x: dropPoint.x, y: dropPoint.y), targetNodeId: targetNodeId)
            }
            return true
        }
    }
}

// MARK: - Node Editor View

struct NodeEditorView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel

    var body: some View {
        ZStack {
            // Layer 1: Grid background
            CanvasGridView(offset: viewModel.canvasOffset, scale: viewModel.canvasScale)

            // Layer 2: Connection cables
            ConnectionCablesView()

            // Layer 3: Node views
            NodeOverlayView()

            // Layer 4: Node drag overlay (left-click drag on nodes + pins, AppKit-based)
            // Also draws the in-progress drag cable via Core Graphics (not SwiftUI)
            // because SwiftUI defers view updates during AppKit mouse tracking.
            NodeDragOverlayView(viewModel: viewModel)

            // Layer 5: Right-click pan + context menu + scroll zoom + pinch zoom overlay
            CanvasPanGestureView(canvasOffset: viewModel.canvasOffset, viewModel: viewModel) { newOffset in
                viewModel.canvasOffset = newOffset
            }
        }
        .coordinateSpace(name: "nodeEditor")
        .frame(maxWidth: .infinity, maxHeight: .infinity)
        .background(Color(.textBackgroundColor).opacity(0.3))
        .overlay {
            if viewModel.isDropTargetActive {
                RoundedRectangle(cornerRadius: 8)
                    .strokeBorder(Color.accentColor.opacity(0.6), lineWidth: 2, antialiased: true)
                    .background(Color.accentColor.opacity(0.05))
                    .allowsHitTesting(false)
                    .transition(.opacity)
            }
        }
        .animation(.easeInOut(duration: 0.15), value: viewModel.isDropTargetActive)
        .clipped()
        .onPreferenceChange(PinPositionPreferenceKey.self) { positions in
            viewModel.pinPositions = positions
        }
    }
}

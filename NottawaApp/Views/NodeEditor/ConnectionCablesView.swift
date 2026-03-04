//
//  ConnectionCablesView.swift
//  NottawaApp
//
//  Draws Bezier cables between connected pins.
//

import SwiftUI

struct ConnectionCablesView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel
    @Environment(ThemeManager.self) private var theme

    var body: some View {
        // Read observable properties outside Canvas closure so SwiftUI
        // tracks them and re-renders the Canvas when they change.
        let connections = viewModel.connections
        let pinPositions = viewModel.pinPositions
        let isDragging = viewModel.dragConnection != nil
        let dragValidPinKeys = viewModel.dragValidPinKeys
        // Capture theme color as a local — Canvas closure is @Sendable
        let accentColor = theme.colors.accent

        Canvas { context, size in
            for connection in connections {
                let outKey = PinKey(nodeId: connection.startNodeId, slotIndex: connection.outputSlot, isOutput: true)
                let inKey = PinKey(nodeId: connection.endNodeId, slotIndex: connection.inputSlot, isOutput: false)

                guard let startPos = pinPositions[outKey],
                      let endPos = pinPositions[inKey] else { continue }

                let path = cablePath(from: startPos, to: endPos)

                // Dim established cables during drag to make the drag cable stand out,
                // unless one of this cable's pins is a valid target.
                let isRelevant = dragValidPinKeys.contains(outKey) || dragValidPinKeys.contains(inKey)
                let opacity: Double = isDragging && !isRelevant ? 0.3 : 0.8
                let glowOpacity: Double = isDragging && !isRelevant ? 0.05 : 0.12

                // Glow layer
                context.stroke(path, with: .color(accentColor.opacity(glowOpacity)), lineWidth: 8)
                // Main cable
                context.stroke(path, with: .color(accentColor.opacity(opacity)), lineWidth: 2.5)
            }
        }
        .allowsHitTesting(false)
    }

    private func cablePath(from start: CGPoint, to end: CGPoint) -> Path {
        let dx = abs(end.x - start.x)
        let dy = abs(end.y - start.y)
        let controlOffset = max(dx * 0.5, dy * 0.25, 60)

        var path = Path()
        path.move(to: start)
        path.addCurve(
            to: end,
            control1: CGPoint(x: start.x + controlOffset, y: start.y),
            control2: CGPoint(x: end.x - controlOffset, y: end.y)
        )
        return path
    }
}

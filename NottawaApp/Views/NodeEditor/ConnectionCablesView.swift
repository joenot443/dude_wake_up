//
//  ConnectionCablesView.swift
//  NottawaApp
//
//  Draws Bezier cables between connected pins.
//

import SwiftUI

/// Shared Bezier cable path used by ConnectionCablesView and hit-testing.
func cablePath(from start: CGPoint, to end: CGPoint) -> Path {
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

/// Find the closest point on a cubic Bezier cable to a given point.
/// Returns the point on the curve nearest to `point`.
func closestPointOnCable(from start: CGPoint, to end: CGPoint, point: CGPoint) -> CGPoint {
    let dx = abs(end.x - start.x)
    let dy = abs(end.y - start.y)
    let controlOffset = max(dx * 0.5, dy * 0.25, 60)
    let cp1 = CGPoint(x: start.x + controlOffset, y: start.y)
    let cp2 = CGPoint(x: end.x - controlOffset, y: end.y)

    var bestPoint = start
    var bestDist = CGFloat.infinity
    let segments = 40
    for i in 0...segments {
        let t = CGFloat(i) / CGFloat(segments)
        let mt = 1 - t
        let x = mt*mt*mt*start.x + 3*mt*mt*t*cp1.x + 3*mt*t*t*cp2.x + t*t*t*end.x
        let y = mt*mt*mt*start.y + 3*mt*mt*t*cp1.y + 3*mt*t*t*cp2.y + t*t*t*end.y
        let d = hypot(x - point.x, y - point.y)
        if d < bestDist {
            bestDist = d
            bestPoint = CGPoint(x: x, y: y)
        }
    }
    return bestPoint
}

struct ConnectionCablesView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel
    @Environment(ThemeManager.self) private var theme

    var body: some View {
        let connections = viewModel.connections
        let pinPositions = viewModel.pinPositions
        let isDragging = viewModel.dragConnection != nil
        let dragValidPinKeys = viewModel.dragValidPinKeys
        let selectedConnId = viewModel.selectedConnectionId
        let hoverCableId = viewModel.dropHoverCableId
        let insertionPoint = viewModel.dropCableInsertionPoint
        let accentColor = theme.colors.accent
        let warningColor = theme.colors.warning

        ZStack {
            // All cables drawn via Canvas
            Canvas { context, size in
                for connection in connections {
                    let outKey = PinKey(nodeId: connection.startNodeId, slotIndex: connection.outputSlot, isOutput: true)
                    let inKey = PinKey(nodeId: connection.endNodeId, slotIndex: connection.inputSlot, isOutput: false)

                    guard let startPos = pinPositions[outKey],
                          let endPos = pinPositions[inKey] else { continue }

                    let path = cablePath(from: startPos, to: endPos)

                    // Cable being hovered for insertion gets warning color
                    if connection.id == hoverCableId {
                        context.stroke(path, with: .color(warningColor.opacity(0.25)), lineWidth: 10)
                        context.stroke(path, with: .color(warningColor.opacity(0.9)), lineWidth: 3)
                    } else {
                        let isRelevant = dragValidPinKeys.contains(outKey) || dragValidPinKeys.contains(inKey)
                        let opacity: Double = isDragging && !isRelevant ? 0.3 : 0.8
                        let glowOpacity: Double = isDragging && !isRelevant ? 0.05 : 0.12

                        context.stroke(path, with: .color(accentColor.opacity(glowOpacity)), lineWidth: 8)
                        context.stroke(path, with: .color(accentColor.opacity(opacity)), lineWidth: 2.5)
                    }
                }

                // Insertion indicator diamond at the closest point on the cable
                if hoverCableId != nil, let pt = insertionPoint {
                    let diamondSize: CGFloat = 10
                    var diamond = Path()
                    diamond.move(to: CGPoint(x: pt.x, y: pt.y - diamondSize))
                    diamond.addLine(to: CGPoint(x: pt.x + diamondSize, y: pt.y))
                    diamond.addLine(to: CGPoint(x: pt.x, y: pt.y + diamondSize))
                    diamond.addLine(to: CGPoint(x: pt.x - diamondSize, y: pt.y))
                    diamond.closeSubpath()

                    context.fill(diamond, with: .color(warningColor))
                    context.stroke(diamond, with: .color(.white.opacity(0.8)), lineWidth: 1.5)
                }
            }

            // Selected cable highlight overlay (SwiftUI Path — animates opacity)
            if let selectedId = selectedConnId,
               let conn = connections.first(where: { $0.id == selectedId }),
               let startPos = pinPositions[PinKey(nodeId: conn.startNodeId, slotIndex: conn.outputSlot, isOutput: true)],
               let endPos = pinPositions[PinKey(nodeId: conn.endNodeId, slotIndex: conn.inputSlot, isOutput: false)] {
                cablePath(from: startPos, to: endPos)
                    .stroke(accentColor.opacity(0.4), lineWidth: 12)
                cablePath(from: startPos, to: endPos)
                    .stroke(accentColor, lineWidth: 4)
            }
        }
        .animation(.easeOut(duration: 0.1), value: selectedConnId)
        .allowsHitTesting(false)
    }
}

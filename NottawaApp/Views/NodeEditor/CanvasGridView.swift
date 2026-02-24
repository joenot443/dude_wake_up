//
//  CanvasGridView.swift
//  NottawaApp
//
//  Subtle dot grid background for the node editor canvas.
//

import SwiftUI

struct CanvasGridView: View {
    let offset: CGPoint
    let scale: CGFloat

    private let gridSpacing: CGFloat = 40
    private let dotRadius: CGFloat = 1.5

    var body: some View {
        Canvas { context, size in
            let spacing = gridSpacing * scale
            guard spacing > 4 else { return } // Don't draw when zoomed too far out

            let offsetX = offset.x.truncatingRemainder(dividingBy: spacing)
            let offsetY = offset.y.truncatingRemainder(dividingBy: spacing)

            let cols = Int(size.width / spacing) + 2
            let rows = Int(size.height / spacing) + 2

            let dotSize = CGSize(width: dotRadius * 2, height: dotRadius * 2)
            let color = Color(.separatorColor)

            for col in 0..<cols {
                for row in 0..<rows {
                    let x = CGFloat(col) * spacing + offsetX
                    let y = CGFloat(row) * spacing + offsetY

                    let rect = CGRect(
                        x: x - dotRadius,
                        y: y - dotRadius,
                        width: dotSize.width,
                        height: dotSize.height
                    )
                    context.fill(Circle().path(in: rect), with: .color(color))
                }
            }
        }
        .drawingGroup()
    }
}

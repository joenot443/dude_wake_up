//
//  ScrollingGraphView.swift
//  NottawaApp
//
//  Reusable Canvas-based scrolling line graph with gradient fill.
//  Uses drawingGroup() to render via Metal instead of Core Animation.
//

import SwiftUI

struct ScrollingGraphView: View {
    let values: [Float]
    let color: Color
    let showCenterLine: Bool

    init(values: [Float], color: Color, showCenterLine: Bool = false) {
        self.values = values
        self.color = color
        self.showCenterLine = showCenterLine
    }

    var body: some View {
        Canvas { context, size in
            let w = size.width
            let h = size.height
            let cornerRadius: CGFloat = 6

            // Dark background
            let bgRect = RoundedRectangle(cornerRadius: cornerRadius)
                .path(in: CGRect(origin: .zero, size: size))
            context.fill(bgRect, with: .color(Color(white: 0.08)))

            // Subtle grid lines
            for i in 1..<4 {
                let y = h * CGFloat(i) / 4.0
                var gridLine = Path()
                gridLine.move(to: CGPoint(x: 0, y: y))
                gridLine.addLine(to: CGPoint(x: w, y: y))
                context.stroke(gridLine, with: .color(.white.opacity(0.06)), lineWidth: 0.5)
            }

            if showCenterLine {
                var centerPath = Path()
                centerPath.move(to: CGPoint(x: 0, y: h / 2))
                centerPath.addLine(to: CGPoint(x: w, y: h / 2))
                context.stroke(centerPath, with: .color(.white.opacity(0.15)), lineWidth: 0.5)
            }

            guard values.count > 1 else { return }

            let count = values.count
            let stepX = w / CGFloat(count - 1)

            // Build line path
            var linePath = Path()
            for i in 0..<count {
                let x = CGFloat(i) * stepX
                let y = h - CGFloat(values[i]) * h
                let clamped = CGPoint(x: x, y: min(max(y, 0), h))
                if i == 0 {
                    linePath.move(to: clamped)
                } else {
                    linePath.addLine(to: clamped)
                }
            }

            // Gradient fill under the line
            var fillPath = linePath
            fillPath.addLine(to: CGPoint(x: CGFloat(count - 1) * stepX, y: h))
            fillPath.addLine(to: CGPoint(x: 0, y: h))
            fillPath.closeSubpath()

            context.fill(
                fillPath,
                with: .linearGradient(
                    Gradient(colors: [color.opacity(0.35), color.opacity(0.0)]),
                    startPoint: CGPoint(x: w / 2, y: 0),
                    endPoint: CGPoint(x: w / 2, y: h)
                )
            )

            // Line stroke
            context.stroke(linePath, with: .color(color), lineWidth: 2)

            // Subtle glow
            context.stroke(linePath, with: .color(color.opacity(0.3)), lineWidth: 5)
        }
        .drawingGroup()
        .clipShape(RoundedRectangle(cornerRadius: 6))
    }
}

/// Multi-line variant for overlaying multiple data series.
struct MultiLineScrollingGraphView: View {
    let series: [(values: [Float], color: Color)]
    let showCenterLine: Bool

    init(series: [(values: [Float], color: Color)], showCenterLine: Bool = false) {
        self.series = series
        self.showCenterLine = showCenterLine
    }

    var body: some View {
        Canvas { context, size in
            let w = size.width
            let h = size.height
            let cornerRadius: CGFloat = 6

            // Dark background
            let bgRect = RoundedRectangle(cornerRadius: cornerRadius)
                .path(in: CGRect(origin: .zero, size: size))
            context.fill(bgRect, with: .color(Color(white: 0.08)))

            // Subtle grid lines
            for i in 1..<4 {
                let y = h * CGFloat(i) / 4.0
                var gridLine = Path()
                gridLine.move(to: CGPoint(x: 0, y: y))
                gridLine.addLine(to: CGPoint(x: w, y: y))
                context.stroke(gridLine, with: .color(.white.opacity(0.06)), lineWidth: 0.5)
            }

            if showCenterLine {
                var centerPath = Path()
                centerPath.move(to: CGPoint(x: 0, y: h / 2))
                centerPath.addLine(to: CGPoint(x: w, y: h / 2))
                context.stroke(centerPath, with: .color(.white.opacity(0.15)), lineWidth: 0.5)
            }

            for (values, color) in series {
                guard values.count > 1 else { continue }
                let count = values.count
                let stepX = w / CGFloat(count - 1)

                var linePath = Path()
                for i in 0..<count {
                    let x = CGFloat(i) * stepX
                    let y = h - CGFloat(values[i]) * h
                    let clamped = CGPoint(x: x, y: min(max(y, 0), h))
                    if i == 0 {
                        linePath.move(to: clamped)
                    } else {
                        linePath.addLine(to: clamped)
                    }
                }

                // Gradient fill per series (low opacity so they layer)
                var fillPath = linePath
                fillPath.addLine(to: CGPoint(x: CGFloat(count - 1) * stepX, y: h))
                fillPath.addLine(to: CGPoint(x: 0, y: h))
                fillPath.closeSubpath()

                context.fill(
                    fillPath,
                    with: .linearGradient(
                        Gradient(colors: [color.opacity(0.2), color.opacity(0.0)]),
                        startPoint: CGPoint(x: w / 2, y: 0),
                        endPoint: CGPoint(x: w / 2, y: h)
                    )
                )

                // Line stroke
                context.stroke(linePath, with: .color(color), lineWidth: 1.5)
            }
        }
        .drawingGroup()
        .clipShape(RoundedRectangle(cornerRadius: 6))
    }
}

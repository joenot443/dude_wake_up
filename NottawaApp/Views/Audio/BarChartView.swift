//
//  BarChartView.swift
//  NottawaApp
//
//  Reusable Canvas-based vertical bar chart with gradient fills.
//  Uses drawingGroup() for Metal-backed rendering.
//

import SwiftUI

struct BarChartView: View {
    let values: [Float]
    let barColor: Color

    init(values: [Float], barColor: Color = .cyan) {
        self.values = values
        self.barColor = barColor
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

            guard !values.isEmpty else { return }

            let count = values.count
            let totalGap = CGFloat(count + 1) * 3
            let barWidth = max(2, (w - totalGap) / CGFloat(count))
            let barRadius: CGFloat = 2

            for i in 0..<count {
                let val = CGFloat(min(max(values[i], 0), 1))
                let barH = val * (h - 4) // small inset from top
                let x = 3 + CGFloat(i) * (barWidth + 3)
                let y = h - barH

                // Rounded top bar
                let barRect = CGRect(x: x, y: y, width: barWidth, height: barH)
                let barPath = Path(roundedRect: barRect, cornerRadii: .init(
                    topLeading: barRadius, bottomLeading: 0,
                    bottomTrailing: 0, topTrailing: barRadius
                ))

                // Gradient from bright at top to darker at bottom
                context.fill(
                    barPath,
                    with: .linearGradient(
                        Gradient(colors: [barColor, barColor.opacity(0.3)]),
                        startPoint: CGPoint(x: x + barWidth / 2, y: y),
                        endPoint: CGPoint(x: x + barWidth / 2, y: h)
                    )
                )
            }
        }
        .drawingGroup()
        .clipShape(RoundedRectangle(cornerRadius: 6))
    }
}

//
//  TextSourceControlsView.swift
//  NottawaApp
//
//  Text editing controls for TextSource nodes:
//  multiline text field, font picker, font size slider, 2D position pad.
//

import SwiftUI

// MARK: - XY Position Pad

/// A 2D draggable pad for setting X/Y position (both 0–1 normalized).
/// Shows a crosshair indicator at the current position. Drag anywhere
/// in the pad to reposition. 16:9 aspect ratio to match the canvas.
struct PositionPadView: View {
    @Environment(ThemeManager.self) private var theme
    @Binding var x: Float
    @Binding var y: Float
    var connectableId: String? = nil

    private let padCornerRadius: CGFloat = 6
    private let handleRadius: CGFloat = 6

    var body: some View {
        GeometryReader { geo in
            let w = geo.size.width
            let h = geo.size.height
            let pointX = CGFloat(x) * w
            let pointY = CGFloat(y) * h

            ZStack {
                // Live preview background (or solid fallback)
                if let cid = connectableId {
                    TextureDisplayView(connectableId: cid)
                        .allowsHitTesting(false)
                        .clipShape(RoundedRectangle(cornerRadius: padCornerRadius))

                    // Dim overlay so crosshair/handle stay visible
                    RoundedRectangle(cornerRadius: padCornerRadius)
                        .fill(Color.black.opacity(0.3))
                } else {
                    RoundedRectangle(cornerRadius: padCornerRadius)
                        .fill(theme.colors.surface)
                }

                // Grid lines (subtle)
                Canvas { ctx, size in
                    let cols = 4
                    let rows = 3
                    for i in 1..<cols {
                        let xLine = size.width * CGFloat(i) / CGFloat(cols)
                        var path = Path()
                        path.move(to: CGPoint(x: xLine, y: 0))
                        path.addLine(to: CGPoint(x: xLine, y: size.height))
                        ctx.stroke(path, with: .color(.white.opacity(0.08)), lineWidth: 0.5)
                    }
                    for i in 1..<rows {
                        let yLine = size.height * CGFloat(i) / CGFloat(rows)
                        var path = Path()
                        path.move(to: CGPoint(x: 0, y: yLine))
                        path.addLine(to: CGPoint(x: size.width, y: yLine))
                        ctx.stroke(path, with: .color(.white.opacity(0.08)), lineWidth: 0.5)
                    }
                }
                .clipShape(RoundedRectangle(cornerRadius: padCornerRadius))

                // Crosshair lines
                Path { path in
                    path.move(to: CGPoint(x: pointX, y: 0))
                    path.addLine(to: CGPoint(x: pointX, y: h))
                }
                .stroke(Color.white.opacity(0.35), lineWidth: 0.5)

                Path { path in
                    path.move(to: CGPoint(x: 0, y: pointY))
                    path.addLine(to: CGPoint(x: w, y: pointY))
                }
                .stroke(Color.white.opacity(0.35), lineWidth: 0.5)

                // Handle dot
                Circle()
                    .fill(theme.colors.accent)
                    .frame(width: handleRadius * 2, height: handleRadius * 2)
                    .shadow(color: .black.opacity(0.5), radius: 3, x: 0, y: 1)
                    .position(x: pointX, y: pointY)

                // Outer ring
                Circle()
                    .strokeBorder(Color.white, lineWidth: 1.5)
                    .frame(width: handleRadius * 2 + 4, height: handleRadius * 2 + 4)
                    .position(x: pointX, y: pointY)

                // Border
                RoundedRectangle(cornerRadius: padCornerRadius)
                    .strokeBorder(theme.colors.border, lineWidth: 0.5)
            }
            .contentShape(Rectangle())
            .gesture(
                DragGesture(minimumDistance: 0, coordinateSpace: .local)
                    .onChanged { value in
                        x = Float(min(max(value.location.x / w, 0), 1))
                        y = Float(min(max(value.location.y / h, 0), 1))
                    }
            )
        }
        .aspectRatio(16.0 / 9.0, contentMode: .fit)
    }
}

// MARK: - Text Source Controls

struct TextSourceControlsView: View {
    @Environment(ThemeManager.self) private var theme
    let state: TextSourceState
    let sourceId: String

    private let engine = NottawaEngine.shared

    @State private var text: String
    @State private var fontSize: Int
    @State private var fontIndex: Int
    @State private var xPosition: Float
    @State private var yPosition: Float

    init(state: TextSourceState, sourceId: String) {
        self.state = state
        self.sourceId = sourceId
        self._text = State(initialValue: state.text)
        self._fontSize = State(initialValue: state.fontSize)
        self._fontIndex = State(initialValue: state.fontIndex)
        self._xPosition = State(initialValue: state.xPosition)
        self._yPosition = State(initialValue: state.yPosition)
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 10) {
            // Text field
            VStack(alignment: .leading, spacing: 4) {
                Text("Text")
                    .font(.caption)
                    .foregroundStyle(theme.colors.textSecondary)
                TextEditor(text: $text)
                    .font(.system(size: 13))
                    .frame(minHeight: 60, maxHeight: 120)
                    .scrollContentBackground(.hidden)
                    .background(theme.colors.surface)
                    .clipShape(RoundedRectangle(cornerRadius: 4))
                    .overlay(
                        RoundedRectangle(cornerRadius: 4)
                            .strokeBorder(theme.colors.border, lineWidth: 0.5)
                    )
                    .onChange(of: text) { _, newValue in
                        engine.setTextSourceText(sourceId: sourceId, text: newValue)
                    }
            }

            // Font picker
            if !state.fontNames.isEmpty {
                VStack(alignment: .leading, spacing: 2) {
                    Text("Font")
                        .font(.caption)
                        .foregroundStyle(theme.colors.textSecondary)
                    Picker("", selection: $fontIndex) {
                        ForEach(0..<state.fontNames.count, id: \.self) { idx in
                            Text(state.fontNames[idx]).tag(idx)
                        }
                    }
                    .labelsHidden()
                    .onChange(of: fontIndex) { _, newValue in
                        engine.setTextSourceFontIndex(sourceId: sourceId, fontIndex: newValue)
                    }
                }
            }

            // Font size
            DSSlider(
                value: Binding(
                    get: { Float(fontSize) },
                    set: { fontSize = Int($0) }
                ),
                range: 8...400,
                step: 1,
                label: "Size",
                showValue: true,
                formatString: "%.0f"
            )
            .onChange(of: fontSize) { _, newValue in
                engine.setTextSourceFontSize(sourceId: sourceId, fontSize: newValue)
            }

            // 2D Position pad
            VStack(alignment: .leading, spacing: 4) {
                HStack {
                    Text("Position")
                        .font(.caption)
                        .foregroundStyle(theme.colors.textSecondary)
                    Spacer()
                    Text(String(format: "%.2f, %.2f", xPosition, yPosition))
                        .font(.caption)
                        .foregroundStyle(theme.colors.textTertiary)
                        .monospacedDigit()
                }

                PositionPadView(x: $xPosition, y: $yPosition, connectableId: sourceId)
                    .onChange(of: xPosition) { _, _ in
                        engine.setTextSourcePosition(sourceId: sourceId, x: xPosition, y: yPosition)
                    }
                    .onChange(of: yPosition) { _, _ in
                        engine.setTextSourcePosition(sourceId: sourceId, x: xPosition, y: yPosition)
                    }
            }
        }
        .padding(.horizontal, 16)
        .padding(.vertical, 8)
        // Sync state from engine refresh
        .onChange(of: state.text) { _, newValue in
            if text != newValue { text = newValue }
        }
        .onChange(of: state.fontSize) { _, newValue in
            if fontSize != newValue { fontSize = newValue }
        }
        .onChange(of: state.fontIndex) { _, newValue in
            if fontIndex != newValue { fontIndex = newValue }
        }
        .onChange(of: state.xPosition) { _, newValue in
            if xPosition != newValue { xPosition = newValue }
        }
        .onChange(of: state.yPosition) { _, newValue in
            if yPosition != newValue { yPosition = newValue }
        }
    }
}

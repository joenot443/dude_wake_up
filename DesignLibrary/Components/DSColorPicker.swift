import SwiftUI

/// A web-style HSB color picker with a 2D saturation/brightness pad,
/// hue slider, and alpha slider.
struct DSColorPicker: View {
    @Binding var color: Color
    var onChange: ((Float, Float, Float, Float) -> Void)? = nil
    @Environment(ThemeManager.self) private var theme

    @State private var hue: Double = 0
    @State private var saturation: Double = 1
    @State private var brightness: Double = 1
    @State private var alpha: Double = 1
    @State private var isExpanded = false
    @State private var didInit = false

    private let padHeight: CGFloat = 150
    private let barHeight: CGFloat = 14
    private let thumbSize: CGFloat = 14

    var body: some View {
        // Swatch button — tap to show popover
        Button {
            isExpanded.toggle()
        } label: {
            RoundedRectangle(cornerRadius: 4)
                .fill(color)
                .frame(width: 28, height: 18)
                .overlay(
                    RoundedRectangle(cornerRadius: 4)
                        .strokeBorder(theme.colors.border, lineWidth: 1)
                )
        }
        .buttonStyle(.plain)
        .popover(isPresented: $isExpanded, arrowEdge: .bottom) {
            VStack(spacing: 8) {
                satBrightPad
                hueBar
                alphaBar
                hexField
            }
            .padding(8)
            .frame(width: 220)
        }
        .onAppear { syncFromColor() }
        .onChange(of: color) { _, _ in
            if !didInit { syncFromColor() }
        }
    }

    // MARK: - Saturation/Brightness Pad

    private var satBrightPad: some View {
        GeometryReader { geo in
            let w = geo.size.width
            let h = geo.size.height

            ZStack {
                // Base hue fill
                Rectangle().fill(Color(hue: hue, saturation: 1, brightness: 1))

                // White gradient (left to right = saturation)
                LinearGradient(
                    colors: [.white, .white.opacity(0)],
                    startPoint: .leading,
                    endPoint: .trailing
                )

                // Black gradient (top to bottom = brightness)
                LinearGradient(
                    colors: [.clear, .black],
                    startPoint: .top,
                    endPoint: .bottom
                )

                // Crosshair thumb
                Circle()
                    .strokeBorder(.white, lineWidth: 2)
                    .shadow(color: .black.opacity(0.4), radius: 2)
                    .frame(width: thumbSize, height: thumbSize)
                    .position(
                        x: saturation * w,
                        y: (1 - brightness) * h
                    )
            }
            .clipShape(RoundedRectangle(cornerRadius: 4))
            .contentShape(Rectangle())
            .gesture(
                DragGesture(minimumDistance: 0)
                    .onChanged { drag in
                        saturation = max(0, min(1, Double(drag.location.x / w)))
                        brightness = max(0, min(1, 1.0 - Double(drag.location.y / h)))
                        pushColor()
                    }
            )
        }
        .frame(height: padHeight)
    }

    // MARK: - Hue Bar

    private var hueBar: some View {
        GeometryReader { geo in
            let w = geo.size.width

            ZStack(alignment: .leading) {
                // Rainbow gradient
                LinearGradient(
                    colors: (0...6).map { Color(hue: Double($0) / 6.0, saturation: 1, brightness: 1) },
                    startPoint: .leading,
                    endPoint: .trailing
                )
                .clipShape(Capsule())

                // Thumb
                Circle()
                    .fill(Color(hue: hue, saturation: 1, brightness: 1))
                    .frame(width: thumbSize, height: thumbSize)
                    .overlay(Circle().strokeBorder(.white, lineWidth: 2))
                    .shadow(color: .black.opacity(0.3), radius: 2)
                    .offset(x: hue * (w - thumbSize))
            }
            .contentShape(Rectangle())
            .gesture(
                DragGesture(minimumDistance: 0)
                    .onChanged { drag in
                        hue = max(0, min(1, drag.location.x / w))
                        pushColor()
                    }
            )
        }
        .frame(height: barHeight)
    }

    // MARK: - Alpha Bar

    private var alphaBar: some View {
        GeometryReader { geo in
            let w = geo.size.width
            let opaqueColor = Color(hue: hue, saturation: saturation, brightness: brightness)

            ZStack(alignment: .leading) {
                // Checkerboard background for transparency visualization
                checkerboard
                    .clipShape(Capsule())

                // Alpha gradient
                LinearGradient(
                    colors: [opaqueColor.opacity(0), opaqueColor],
                    startPoint: .leading,
                    endPoint: .trailing
                )
                .clipShape(Capsule())

                // Thumb
                Circle()
                    .fill(opaqueColor.opacity(alpha))
                    .frame(width: thumbSize, height: thumbSize)
                    .overlay(Circle().strokeBorder(.white, lineWidth: 2))
                    .shadow(color: .black.opacity(0.3), radius: 2)
                    .offset(x: alpha * (w - thumbSize))
            }
            .contentShape(Rectangle())
            .gesture(
                DragGesture(minimumDistance: 0)
                    .onChanged { drag in
                        alpha = max(0, min(1, drag.location.x / w))
                        pushColor()
                    }
            )
        }
        .frame(height: barHeight)
    }

    // MARK: - Hex Field

    private var hexField: some View {
        HStack(spacing: 4) {
            Text(hexString)
                .font(.system(size: 10, weight: .medium, design: .monospaced))
                .foregroundStyle(theme.colors.textSecondary)
                .textSelection(.enabled)
            Spacer()
            Text("\(Int(alpha * 100))%")
                .font(.system(size: 10, weight: .medium, design: .monospaced))
                .foregroundStyle(theme.colors.textTertiary)
        }
    }

    // MARK: - Helpers

    private var checkerboard: some View {
        Canvas { context, size in
            let cellSize: CGFloat = 4
            let cols = Int(ceil(size.width / cellSize))
            let rows = Int(ceil(size.height / cellSize))
            for row in 0..<rows {
                for col in 0..<cols {
                    let isLight = (row + col) % 2 == 0
                    let rect = CGRect(x: CGFloat(col) * cellSize, y: CGFloat(row) * cellSize,
                                      width: cellSize, height: cellSize)
                    context.fill(Path(rect), with: .color(isLight ? .white : Color(white: 0.8)))
                }
            }
        }
    }

    private var hexString: String {
        let r = Int(round(hue2rgb(h: hue, s: saturation, b: brightness).r * 255))
        let g = Int(round(hue2rgb(h: hue, s: saturation, b: brightness).g * 255))
        let b = Int(round(hue2rgb(h: hue, s: saturation, b: brightness).b * 255))
        return String(format: "#%02X%02X%02X", r, g, b)
    }

    private func hue2rgb(h: Double, s: Double, b: Double) -> (r: Double, g: Double, b: Double) {
        let c = Color(hue: h, saturation: s, brightness: b)
        let resolved = c.resolve(in: EnvironmentValues())
        return (Double(resolved.red), Double(resolved.green), Double(resolved.blue))
    }

    private func syncFromColor() {
        let resolved = color.resolve(in: EnvironmentValues())
        let r = Double(resolved.red)
        let g = Double(resolved.green)
        let b = Double(resolved.blue)
        alpha = Double(resolved.opacity)

        // RGB to HSB
        let maxC = max(r, g, b)
        let minC = min(r, g, b)
        let delta = maxC - minC

        brightness = maxC

        if maxC > 0 {
            saturation = delta / maxC
        } else {
            saturation = 0
        }

        if delta > 0 {
            if maxC == r {
                hue = ((g - b) / delta).truncatingRemainder(dividingBy: 6)
                if hue < 0 { hue += 6 }
                hue /= 6
            } else if maxC == g {
                hue = ((b - r) / delta + 2) / 6
            } else {
                hue = ((r - g) / delta + 4) / 6
            }
        } else {
            hue = 0
        }

        didInit = true
    }

    private func pushColor() {
        let newColor = Color(hue: hue, saturation: saturation, brightness: brightness, opacity: alpha)
        color = newColor
        if let onChange {
            let resolved = newColor.resolve(in: EnvironmentValues())
            onChange(resolved.red, resolved.green, resolved.blue, resolved.opacity)
        }
    }
}

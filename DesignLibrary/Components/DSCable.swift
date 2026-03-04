import SwiftUI

// MARK: - Pin Position Preference Key

struct DSPinPositionKey: PreferenceKey {
    static var defaultValue: [String: CGPoint] = [:]

    static func reduce(value: inout [String: CGPoint], nextValue: () -> [String: CGPoint]) {
        value.merge(nextValue(), uniquingKeysWith: { $1 })
    }
}

// MARK: - Cable Shape

struct DSCable: Shape {
    var from: CGPoint
    var to: CGPoint

    var animatableData: AnimatablePair<CGPoint.AnimatableData, CGPoint.AnimatableData> {
        get { .init(from.animatableData, to.animatableData) }
        set {
            from.animatableData = newValue.first
            to.animatableData = newValue.second
        }
    }

    func path(in rect: CGRect) -> Path {
        let dx = abs(to.x - from.x)
        let dy = abs(to.y - from.y)
        let controlOffset = max(dx * 0.5, dy * 0.25, 60)

        var path = Path()
        path.move(to: from)
        path.addCurve(
            to: to,
            control1: CGPoint(x: from.x + controlOffset, y: from.y),
            control2: CGPoint(x: to.x - controlOffset, y: to.y)
        )
        return path
    }
}

enum DSCableStyle {
    case established  // Solid accent
    case dimmed       // Low opacity during drag
    case dragging     // Dashed while drawing
}

struct DSCableView: View {
    @Environment(ThemeManager.self) var theme

    let from: CGPoint
    let to: CGPoint
    let style: DSCableStyle

    init(from: CGPoint, to: CGPoint, style: DSCableStyle = .established) {
        self.from = from
        self.to = to
        self.style = style
    }

    var body: some View {
        ZStack {
            // Glow layer
            DSCable(from: from, to: to)
                .stroke(theme.colors.accent.opacity(glowOpacity), lineWidth: 8)

            // Main cable
            if style == .dragging {
                DSCable(from: from, to: to)
                    .stroke(theme.colors.accent.opacity(cableOpacity), style: StrokeStyle(lineWidth: 2.5, dash: [6, 4]))
            } else {
                DSCable(from: from, to: to)
                    .stroke(theme.colors.accent.opacity(cableOpacity), lineWidth: 2.5)
            }
        }
        .allowsHitTesting(false)
    }

    private var glowOpacity: Double {
        switch style {
        case .established: return 0.12
        case .dimmed: return 0.05
        case .dragging: return 0.15
        }
    }

    private var cableOpacity: Double {
        switch style {
        case .established: return 0.8
        case .dimmed: return 0.3
        case .dragging: return 0.7
        }
    }
}

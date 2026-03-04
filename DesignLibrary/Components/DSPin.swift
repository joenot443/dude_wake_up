import SwiftUI

enum DSPinState {
    case disconnected
    case connected
    case snapped
    case validTarget
    case dimmed
}

struct DSPin: View {
    @Environment(ThemeManager.self) var theme

    let state: DSPinState
    let label: String?
    let pinId: String?

    init(state: DSPinState = .disconnected, label: String? = nil, pinId: String? = nil) {
        self.state = state
        self.label = label
        self.pinId = pinId
    }

    private let pinSize: CGFloat = 10
    private var ringSize: CGFloat { pinSize + 4 }
    private var dotSize: CGFloat { pinSize * 0.5 }

    var body: some View {
        ZStack {
            // Snap glow background
            if state == .snapped {
                Circle()
                    .fill(theme.colors.success.opacity(0.2))
                    .frame(width: ringSize + 6, height: ringSize + 6)
            }

            // Outer ring (always visible)
            Circle()
                .strokeBorder(ringColor, lineWidth: ringLineWidth)
                .frame(width: ringSize, height: ringSize)

            // Inner dot (connected/snapped only)
            if state == .connected || state == .snapped {
                Circle()
                    .fill(dotColor)
                    .frame(width: dotSize, height: dotSize)
                    .shadow(color: dotColor.opacity(0.5), radius: 2)
            }

            // Label (for auxiliary pins)
            if let label {
                Text(label)
                    .font(.system(size: 6, weight: .bold))
                    .foregroundStyle(.white)
            }
        }
        .opacity(state == .dimmed ? 0.3 : 1.0)
        .animation(.easeInOut(duration: 0.15), value: state)
        .background(
            Group {
                if let pinId {
                    GeometryReader { geo in
                        Color.clear.preference(
                            key: DSPinPositionKey.self,
                            value: [pinId: CGPoint(
                                x: geo.frame(in: .named("dsChain")).midX,
                                y: geo.frame(in: .named("dsChain")).midY
                            )]
                        )
                    }
                }
            }
        )
    }

    private var ringColor: Color {
        switch state {
        case .snapped:
            return theme.colors.success.opacity(0.6)
        case .connected, .validTarget:
            return theme.colors.accent.opacity(0.5)
        case .disconnected:
            return theme.colors.textTertiary.opacity(0.7)
        case .dimmed:
            return theme.colors.textTertiary.opacity(0.7)
        }
    }

    private var ringLineWidth: CGFloat {
        switch state {
        case .snapped, .validTarget:
            return 1.5
        default:
            return 1
        }
    }

    private var dotColor: Color {
        switch state {
        case .snapped: return theme.colors.success
        case .connected: return theme.colors.accent
        default: return .clear
        }
    }
}

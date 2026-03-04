import SwiftUI

enum DSButtonVariant {
    case primary
    case secondary
    case ghost
    case destructive
}

enum DSButtonSize {
    case sm, md, lg
}

struct DSButton: View {
    @Environment(ThemeManager.self) var theme
    @Environment(\.isEnabled) var isEnabled

    let title: String
    let variant: DSButtonVariant
    let size: DSButtonSize
    let icon: String?
    let fullWidth: Bool
    let action: () -> Void

    @State private var isHovered = false

    init(
        _ title: String,
        variant: DSButtonVariant = .primary,
        size: DSButtonSize = .md,
        icon: String? = nil,
        fullWidth: Bool = false,
        action: @escaping () -> Void
    ) {
        self.title = title
        self.variant = variant
        self.size = size
        self.icon = icon
        self.fullWidth = fullWidth
        self.action = action
    }

    var body: some View {
        Button(action: action) {
            HStack(spacing: theme.spacing.sm) {
                if let icon {
                    Image(systemName: icon)
                        .font(.system(size: size == .sm ? 10 : 12, weight: .medium))
                }
                Text(title)
                    .font(buttonFont)
            }
            .padding(.horizontal, horizontalPadding)
            .frame(height: buttonHeight)
            .frame(maxWidth: fullWidth ? .infinity : nil)
            .foregroundStyle(foregroundColor)
            .background(
                RoundedRectangle(cornerRadius: theme.radii.md)
                    .fill(backgroundColor)
            )
            .overlay(
                RoundedRectangle(cornerRadius: theme.radii.md)
                    .strokeBorder(borderColor, lineWidth: variant == .secondary ? 1 : 0)
            )
        }
        .buttonStyle(DSPressStyle())
        .onHover { hovering in
            withAnimation(.easeOut(duration: 0.15)) {
                isHovered = hovering
            }
        }
    }

    private var buttonHeight: CGFloat {
        switch size {
        case .sm: return theme.sizing.buttonHeightSm
        case .md: return theme.sizing.buttonHeightMd
        case .lg: return theme.sizing.buttonHeightLg
        }
    }

    private var buttonFont: Font {
        switch size {
        case .sm: return theme.typography.caption
        case .md: return .system(size: theme.typography.bodySize, weight: .medium, design: theme.typography.fontDesign)
        case .lg: return .system(size: theme.typography.bodyLargeSize, weight: .medium, design: theme.typography.fontDesign)
        }
    }

    private var horizontalPadding: CGFloat {
        switch size {
        case .sm: return theme.spacing.md
        case .md: return theme.spacing.lg
        case .lg: return theme.spacing.xl
        }
    }

    private var backgroundColor: Color {
        guard isEnabled else { return theme.colors.backgroundTertiary }
        switch variant {
        case .primary:
            return isHovered ? theme.colors.accentHover : theme.colors.accent
        case .secondary:
            return isHovered ? theme.colors.surfaceHover : .clear
        case .ghost:
            return isHovered ? theme.colors.surfaceHover : .clear
        case .destructive:
            return isHovered ? theme.colors.destructive.opacity(0.85) : theme.colors.destructive
        }
    }

    private var foregroundColor: Color {
        guard isEnabled else { return theme.colors.textTertiary }
        switch variant {
        case .primary:
            return theme.colors.textOnAccent
        case .secondary:
            return isHovered ? theme.colors.textPrimary : theme.colors.textSecondary
        case .ghost:
            return isHovered ? theme.colors.textPrimary : theme.colors.textSecondary
        case .destructive:
            return theme.colors.textOnAccent
        }
    }

    private var borderColor: Color {
        guard isEnabled else { return theme.colors.border }
        switch variant {
        case .secondary: return theme.colors.border
        default: return .clear
        }
    }
}

// Minimal press feedback style
struct DSPressStyle: ButtonStyle {
    func makeBody(configuration: Configuration) -> some View {
        configuration.label
            .opacity(configuration.isPressed ? 0.75 : 1.0)
            .scaleEffect(configuration.isPressed ? 0.98 : 1.0)
            .animation(.easeOut(duration: 0.1), value: configuration.isPressed)
    }
}

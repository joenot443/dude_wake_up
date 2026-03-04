import SwiftUI

enum DSIconButtonVariant {
    case standard    // Gray, blends into background
    case accent      // Accent color
    case destructive // Red/destructive color
}

enum DSIconButtonSize {
    case sm  // Node action bars
    case md  // Toolbars
    case lg  // Prominent actions
}

struct DSIconButton: View {
    @Environment(ThemeManager.self) var theme
    @Environment(\.isEnabled) var isEnabled

    let icon: String
    let variant: DSIconButtonVariant
    let size: DSIconButtonSize
    let isActive: Bool
    let tooltip: String?
    let action: () -> Void

    @State private var isHovered = false

    init(
        _ icon: String,
        variant: DSIconButtonVariant = .standard,
        size: DSIconButtonSize = .md,
        isActive: Bool = false,
        tooltip: String? = nil,
        action: @escaping () -> Void
    ) {
        self.icon = icon
        self.variant = variant
        self.size = size
        self.isActive = isActive
        self.tooltip = tooltip
        self.action = action
    }

    var body: some View {
        Button(action: action) {
            Image(systemName: icon)
                .font(.system(size: iconSize, weight: .medium))
                .foregroundStyle(foregroundColor)
                .frame(width: frameSize, height: frameSize)
                .background(
                    RoundedRectangle(cornerRadius: backgroundRadius)
                        .fill(backgroundColor)
                )
        }
        .buttonStyle(DSPressStyle())
        .onHover { hovering in
            withAnimation(.easeOut(duration: 0.12)) {
                isHovered = hovering
            }
        }
        .help(tooltip ?? "")
    }

    private var iconSize: CGFloat {
        switch size {
        case .sm: return 11
        case .md: return 14
        case .lg: return 18
        }
    }

    private var frameSize: CGFloat {
        switch size {
        case .sm: return theme.sizing.buttonHeightSm - 4
        case .md: return theme.sizing.buttonHeightMd
        case .lg: return theme.sizing.buttonHeightLg
        }
    }

    private var backgroundRadius: CGFloat {
        switch size {
        case .sm: return theme.radii.sm
        case .md: return theme.radii.md
        case .lg: return theme.radii.lg
        }
    }

    private var foregroundColor: Color {
        guard isEnabled else { return theme.colors.textTertiary }

        if isActive {
            return theme.colors.accent
        }

        switch variant {
        case .standard:
            return isHovered ? theme.colors.textPrimary : theme.colors.textSecondary
        case .accent:
            return isHovered ? theme.colors.accentHover : theme.colors.accent
        case .destructive:
            return isHovered ? theme.colors.destructive : theme.colors.destructive.opacity(0.7)
        }
    }

    private var backgroundColor: Color {
        guard isEnabled else { return .clear }

        if isHovered {
            return theme.colors.surfaceHover
        }
        if isActive {
            return theme.colors.accentSubtle
        }
        return .clear
    }
}

import SwiftUI

enum DSToggleStyle {
    case `switch`
    case checkbox
}

enum DSToggleSize {
    case sm, md
}

struct DSToggle: View {
    @Environment(ThemeManager.self) var theme
    @Environment(\.isEnabled) var isEnabled

    @Binding var isOn: Bool
    let label: String?
    let style: DSToggleStyle
    let size: DSToggleSize

    @State private var isHovered = false

    init(
        isOn: Binding<Bool>,
        label: String? = nil,
        style: DSToggleStyle = .switch,
        size: DSToggleSize = .md
    ) {
        self._isOn = isOn
        self.label = label
        self.style = style
        self.size = size
    }

    private var scale: CGFloat {
        size == .sm ? 0.85 : 1.0
    }

    var body: some View {
        Button {
            guard isEnabled else { return }
            isOn.toggle()
        } label: {
            HStack(spacing: theme.spacing.sm) {
                toggleControl
                if let label {
                    Text(label)
                        .font(theme.typography.body)
                        .foregroundStyle(isEnabled ? theme.colors.textPrimary : theme.colors.textTertiary)
                }
            }
        }
        .buttonStyle(.plain)
        .onHover { hovering in
            withAnimation(.easeOut(duration: 0.12)) {
                isHovered = hovering
            }
        }
    }

    @ViewBuilder
    private var toggleControl: some View {
        switch style {
        case .switch:
            switchControl
        case .checkbox:
            checkboxControl
        }
    }

    // MARK: - Switch Style

    private var switchControl: some View {
        let trackWidth = theme.sizing.toggleTrackWidth * scale
        let trackHeight = theme.sizing.toggleTrackHeight * scale
        let thumbSize = theme.sizing.toggleThumbSize * scale
        let inset = (trackHeight - thumbSize) / 2
        let thumbOffset = isOn ? trackWidth - thumbSize - inset : inset

        return ZStack(alignment: .leading) {
            Capsule()
                .fill(switchTrackColor)
                .frame(width: trackWidth, height: trackHeight)

            Circle()
                .fill(Color.white)
                .frame(width: thumbSize, height: thumbSize)
                .shadow(color: .black.opacity(0.15), radius: 2, y: 1)
                .offset(x: thumbOffset)
        }
        .animation(.easeOut(duration: 0.15), value: isOn)
        .animation(.easeOut(duration: 0.12), value: isHovered)
    }

    private var switchTrackColor: Color {
        guard isEnabled else { return theme.colors.backgroundTertiary }
        if isOn {
            return isHovered ? theme.colors.accentHover : theme.colors.accent
        } else {
            return isHovered ? theme.colors.surfaceHover : theme.colors.backgroundTertiary
        }
    }

    // MARK: - Checkbox Style

    private var checkboxControl: some View {
        let boxSize = theme.sizing.checkboxSize * scale

        return ZStack {
            RoundedRectangle(cornerRadius: theme.radii.sm)
                .fill(checkboxFillColor)
                .frame(width: boxSize, height: boxSize)
                .overlay(
                    RoundedRectangle(cornerRadius: theme.radii.sm)
                        .strokeBorder(checkboxBorderColor, lineWidth: 1)
                )

            if isOn {
                Image(systemName: "checkmark")
                    .font(.system(size: boxSize * 0.6, weight: .bold))
                    .foregroundStyle(isEnabled ? theme.colors.textOnAccent : theme.colors.textTertiary)
            }
        }
        .animation(.easeOut(duration: 0.15), value: isOn)
        .animation(.easeOut(duration: 0.12), value: isHovered)
    }

    private var checkboxFillColor: Color {
        guard isEnabled else {
            return isOn ? theme.colors.backgroundTertiary : .clear
        }
        if isOn {
            return isHovered ? theme.colors.accentHover : theme.colors.accent
        } else {
            return isHovered ? theme.colors.surfaceHover : .clear
        }
    }

    private var checkboxBorderColor: Color {
        guard isEnabled else { return theme.colors.border }
        if isOn {
            return isHovered ? theme.colors.accentHover : theme.colors.accent
        } else {
            return isHovered ? theme.colors.textSecondary : theme.colors.border
        }
    }
}

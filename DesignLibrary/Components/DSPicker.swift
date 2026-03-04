import SwiftUI

enum DSPickerStyle {
    case menu
    case segmented
}

enum DSPickerSize {
    case sm, md
}

struct DSPickerOption<V: Hashable>: Identifiable {
    let value: V
    let label: String
    var id: String { label }
}

struct DSPicker<V: Hashable>: View {
    @Environment(ThemeManager.self) var theme
    @Environment(\.isEnabled) var isEnabled

    @Binding var selection: V
    let options: [DSPickerOption<V>]
    let label: String?
    let style: DSPickerStyle
    let size: DSPickerSize

    init(
        selection: Binding<V>,
        options: [DSPickerOption<V>],
        label: String? = nil,
        style: DSPickerStyle = .menu,
        size: DSPickerSize = .md
    ) {
        self._selection = selection
        self.options = options
        self.label = label
        self.style = style
        self.size = size
    }

    var body: some View {
        VStack(alignment: .leading, spacing: theme.spacing.xs) {
            if let label {
                Text(label)
                    .font(theme.typography.caption)
                    .foregroundStyle(isEnabled ? theme.colors.textSecondary : theme.colors.textTertiary)
            }

            switch style {
            case .menu:
                menuPicker
            case .segmented:
                segmentedPicker
            }
        }
    }

    private var buttonHeight: CGFloat {
        size == .sm ? theme.sizing.buttonHeightSm : theme.sizing.buttonHeightMd
    }

    // MARK: - Menu Style

    private var menuPicker: some View {
        Menu {
            ForEach(options) { option in
                Button {
                    selection = option.value
                } label: {
                    HStack {
                        Text(option.label)
                        if option.value == selection {
                            Image(systemName: "checkmark")
                        }
                    }
                }
            }
        } label: {
            MenuTrigger(
                selectedLabel: options.first(where: { $0.value == selection })?.label ?? "",
                height: buttonHeight,
                size: size,
                isEnabled: isEnabled
            )
        }
        .menuStyle(.borderlessButton)
        .menuIndicator(.hidden)
    }

    // MARK: - Segmented Style

    private var segmentedPicker: some View {
        GeometryReader { geo in
            let segmentWidth = geo.size.width / CGFloat(options.count)
            let selectedIndex = options.firstIndex(where: { $0.value == selection }) ?? 0

            ZStack(alignment: .leading) {
                // Track
                RoundedRectangle(cornerRadius: theme.radii.md)
                    .fill(theme.colors.backgroundTertiary)

                // Sliding indicator
                RoundedRectangle(cornerRadius: theme.radii.md - 1)
                    .fill(isEnabled ? theme.colors.accent : theme.colors.backgroundTertiary)
                    .frame(width: segmentWidth - 4)
                    .offset(x: CGFloat(selectedIndex) * segmentWidth + 2)
                    .animation(.easeInOut(duration: 0.2), value: selection)

                // Segments
                HStack(spacing: 0) {
                    ForEach(Array(options.enumerated()), id: \.element.id) { index, option in
                        let isSelected = option.value == selection

                        Text(option.label)
                            .font(.system(
                                size: size == .sm ? theme.typography.captionSize : theme.typography.bodySize,
                                weight: isSelected ? .medium : .regular,
                                design: theme.typography.fontDesign
                            ))
                            .foregroundStyle(segmentTextColor(isSelected: isSelected))
                            .frame(maxWidth: .infinity)
                            .frame(height: buttonHeight)
                            .contentShape(Rectangle())
                            .onTapGesture {
                                guard isEnabled else { return }
                                selection = option.value
                            }
                    }
                }
            }
        }
        .frame(height: buttonHeight)
    }

    private func segmentTextColor(isSelected: Bool) -> Color {
        guard isEnabled else { return theme.colors.textTertiary }
        return isSelected ? theme.colors.textOnAccent : theme.colors.textSecondary
    }
}

// MARK: - Menu Trigger (private subview for hover state)

private struct MenuTrigger: View {
    @Environment(ThemeManager.self) var theme

    let selectedLabel: String
    let height: CGFloat
    let size: DSPickerSize
    let isEnabled: Bool

    @State private var isHovered = false

    var body: some View {
        HStack(spacing: theme.spacing.sm) {
            Text(selectedLabel)
                .font(size == .sm ? theme.typography.caption : theme.typography.body)
                .foregroundStyle(isEnabled ? theme.colors.textPrimary : theme.colors.textTertiary)
            Spacer()
            Image(systemName: "chevron.up.chevron.down")
                .font(.system(size: 10, weight: .medium))
                .foregroundStyle(isEnabled ? theme.colors.textSecondary : theme.colors.textTertiary)
        }
        .padding(.horizontal, theme.spacing.md)
        .frame(height: height)
        .background(
            RoundedRectangle(cornerRadius: theme.radii.md)
                .fill(isHovered && isEnabled ? theme.colors.surfaceHover : theme.colors.surface)
        )
        .overlay(
            RoundedRectangle(cornerRadius: theme.radii.md)
                .strokeBorder(theme.colors.border, lineWidth: 1)
        )
        .onHover { hovering in
            withAnimation(.easeOut(duration: 0.12)) {
                isHovered = hovering
            }
        }
    }
}

// MARK: - CaseIterable Convenience

extension DSPicker where V: CaseIterable & CustomStringConvertible {
    init(
        selection: Binding<V>,
        label: String? = nil,
        style: DSPickerStyle = .menu,
        size: DSPickerSize = .md
    ) where V.AllCases: RandomAccessCollection {
        self._selection = selection
        self.options = V.allCases.map { DSPickerOption(value: $0, label: $0.description) }
        self.label = label
        self.style = style
        self.size = size
    }
}

import SwiftUI

enum ShowcaseSection: String, CaseIterable, Identifiable {
    case typography = "Typography"
    case colors = "Colors"
    case buttons = "Buttons"
    case iconButtons = "Icon Buttons"
    case toggles = "Toggles"
    case pickers = "Pickers"
    case sliders = "Sliders"
    case nodes = "Nodes"
    case spacing = "Spacing & Sizing"

    var id: String { rawValue }

    var icon: String {
        switch self {
        case .typography: return "textformat"
        case .colors: return "paintpalette"
        case .buttons: return "rectangle.and.hand.point.up.left"
        case .iconButtons: return "square.grid.2x2"
        case .toggles: return "switch.2"
        case .pickers: return "list.bullet"
        case .sliders: return "slider.horizontal.3"
        case .nodes: return "point.3.connected.trianglepath.dotted"
        case .spacing: return "ruler"
        }
    }
}

struct ShowcaseView: View {
    @Environment(ThemeManager.self) var theme
    @State private var selectedSection: ShowcaseSection = .typography

    var body: some View {
        HStack(spacing: 0) {
            sidebar

            Rectangle()
                .fill(theme.colors.border)
                .frame(width: 1)

            ScrollView {
                VStack(alignment: .leading, spacing: theme.spacing.xxl) {
                    sectionContent
                }
                .padding(theme.spacing.xxl)
                .frame(maxWidth: .infinity, alignment: .leading)
            }
            .frame(maxWidth: .infinity, maxHeight: .infinity)
            .background(theme.colors.background)
        }
        .animation(.easeInOut(duration: 0.3), value: theme.preset)
    }

    private var sidebar: some View {
        VStack(alignment: .leading, spacing: 0) {
            // Header
            VStack(alignment: .leading, spacing: theme.spacing.xs) {
                Text("Design Library")
                    .font(theme.typography.h3)
                    .foregroundStyle(theme.colors.textPrimary)

                Text(theme.tokens.description)
                    .font(theme.typography.caption)
                    .foregroundStyle(theme.colors.textTertiary)
            }
            .padding(theme.spacing.lg)

            Rectangle()
                .fill(theme.colors.border)
                .frame(height: 1)

            // Theme picker
            VStack(alignment: .leading, spacing: theme.spacing.sm) {
                Text("THEME")
                    .font(.system(size: 10, weight: .semibold, design: theme.typography.fontDesign))
                    .foregroundStyle(theme.colors.textTertiary)
                    .tracking(1)

                themePicker
            }
            .padding(theme.spacing.lg)

            Rectangle()
                .fill(theme.colors.border)
                .frame(height: 1)

            // Section navigation
            VStack(spacing: theme.spacing.xxs) {
                ForEach(ShowcaseSection.allCases) { section in
                    SidebarNavButton(
                        title: section.rawValue,
                        icon: section.icon,
                        isSelected: selectedSection == section
                    ) {
                        selectedSection = section
                    }
                }
            }
            .padding(theme.spacing.sm)

            Spacer()

            // Theme description footer
            VStack(alignment: .leading, spacing: theme.spacing.xs) {
                Rectangle()
                    .fill(theme.colors.border)
                    .frame(height: 1)

                Text("Font: \(designName)")
                    .font(theme.typography.caption)
                    .foregroundStyle(theme.colors.textTertiary)
                    .padding(.horizontal, theme.spacing.lg)
                    .padding(.vertical, theme.spacing.sm)
            }
        }
        .frame(width: 220)
        .background(theme.colors.backgroundSecondary)
    }

    private var themePicker: some View {
        @Bindable var tm = theme
        return Picker("Theme", selection: $tm.preset) {
            ForEach(ThemePreset.allCases) { preset in
                Text(preset.rawValue).tag(preset)
            }
        }
        .labelsHidden()
        .pickerStyle(.menu)
    }

    @ViewBuilder
    private var sectionContent: some View {
        switch selectedSection {
        case .typography: TypographyShowcase()
        case .colors: ColorShowcase()
        case .buttons: ButtonShowcase()
        case .iconButtons: IconButtonShowcase()
        case .toggles: ToggleShowcase()
        case .pickers: PickerShowcase()
        case .sliders: SliderShowcase()
        case .nodes: NodeShowcase()
        case .spacing: SpacingShowcase()
        }
    }

    private var designName: String {
        switch theme.typography.fontDesign {
        case .default: return "SF Pro"
        case .rounded: return "SF Pro Rounded"
        case .monospaced: return "SF Mono"
        case .serif: return "New York"
        @unknown default: return "System"
        }
    }
}

// MARK: - Sidebar Navigation Button

struct SidebarNavButton: View {
    @Environment(ThemeManager.self) var theme

    let title: String
    let icon: String
    let isSelected: Bool
    let action: () -> Void

    @State private var isHovered = false

    var body: some View {
        Button(action: action) {
            HStack(spacing: theme.spacing.sm) {
                Image(systemName: icon)
                    .font(.system(size: 13))
                    .frame(width: theme.sizing.iconMd)
                Text(title)
                    .font(theme.typography.body)
                Spacer()
            }
            .padding(.horizontal, theme.spacing.md)
            .padding(.vertical, theme.spacing.sm)
            .foregroundStyle(
                isSelected ? theme.colors.accent
                : isHovered ? theme.colors.textPrimary
                : theme.colors.textSecondary
            )
            .background(
                RoundedRectangle(cornerRadius: theme.radii.md)
                    .fill(
                        isSelected ? theme.colors.accentSubtle
                        : isHovered ? theme.colors.surfaceHover
                        : .clear
                    )
            )
        }
        .buttonStyle(.plain)
        .onHover { hovering in
            withAnimation(.easeOut(duration: 0.1)) {
                isHovered = hovering
            }
        }
    }
}

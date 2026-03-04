import SwiftUI

struct ColorShowcase: View {
    @Environment(ThemeManager.self) var theme

    var body: some View {
        VStack(alignment: .leading, spacing: theme.spacing.xl) {
            DSText("Colors", style: .h2)
            DSText("Full color palette with semantic meaning.", style: .body, color: theme.colors.textSecondary)

            sectionDivider

            colorGroup("Backgrounds", colors: [
                ("Background", theme.colors.background),
                ("Secondary", theme.colors.backgroundSecondary),
                ("Tertiary", theme.colors.backgroundTertiary),
            ])

            sectionDivider

            colorGroup("Surfaces", colors: [
                ("Surface", theme.colors.surface),
                ("Hover", theme.colors.surfaceHover),
                ("Pressed", theme.colors.surfacePressed),
            ])

            sectionDivider

            colorGroup("Borders", colors: [
                ("Border", theme.colors.border),
                ("Subtle", theme.colors.borderSubtle),
            ])

            sectionDivider

            colorGroup("Text", colors: [
                ("Primary", theme.colors.textPrimary),
                ("Secondary", theme.colors.textSecondary),
                ("Tertiary", theme.colors.textTertiary),
                ("On Accent", theme.colors.textOnAccent),
            ])

            sectionDivider

            colorGroup("Accent", colors: [
                ("Accent", theme.colors.accent),
                ("Hover", theme.colors.accentHover),
                ("Subtle", theme.colors.accentSubtle),
            ])

            sectionDivider

            colorGroup("Semantic", colors: [
                ("Destructive", theme.colors.destructive),
                ("Dest. Subtle", theme.colors.destructiveSubtle),
                ("Success", theme.colors.success),
                ("Succ. Subtle", theme.colors.successSubtle),
                ("Warning", theme.colors.warning),
                ("Warn. Subtle", theme.colors.warningSubtle),
            ])

            sectionDivider

            // Contrast demo
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Contrast Demo", style: .h4, color: theme.colors.textTertiary)

                DSCard {
                    VStack(alignment: .leading, spacing: theme.spacing.md) {
                        Text("Primary text on surface")
                            .font(theme.typography.body)
                            .foregroundStyle(theme.colors.textPrimary)
                        Text("Secondary text on surface")
                            .font(theme.typography.body)
                            .foregroundStyle(theme.colors.textSecondary)
                        Text("Tertiary text on surface")
                            .font(theme.typography.body)
                            .foregroundStyle(theme.colors.textTertiary)

                        HStack(spacing: theme.spacing.sm) {
                            accentChip("Accent", bg: theme.colors.accent, fg: theme.colors.textOnAccent)
                            accentChip("Success", bg: theme.colors.success, fg: theme.colors.textOnAccent)
                            accentChip("Warning", bg: theme.colors.warning, fg: theme.colors.textOnAccent)
                            accentChip("Destructive", bg: theme.colors.destructive, fg: theme.colors.textOnAccent)
                        }
                    }
                }
            }
        }
    }

    private var sectionDivider: some View {
        Rectangle()
            .fill(theme.colors.borderSubtle)
            .frame(height: 1)
    }

    private func colorGroup(_ title: String, colors: [(String, Color)]) -> some View {
        VStack(alignment: .leading, spacing: theme.spacing.md) {
            DSText(title, style: .h4, color: theme.colors.textTertiary)

            LazyVGrid(
                columns: [GridItem(.adaptive(minimum: 100, maximum: 140), spacing: theme.spacing.md)],
                spacing: theme.spacing.md
            ) {
                ForEach(colors, id: \.0) { name, color in
                    colorSwatch(name: name, color: color)
                }
            }
        }
    }

    private func colorSwatch(name: String, color: Color) -> some View {
        VStack(spacing: theme.spacing.xs) {
            RoundedRectangle(cornerRadius: theme.radii.md)
                .fill(color)
                .frame(height: 52)
                .overlay(
                    RoundedRectangle(cornerRadius: theme.radii.md)
                        .strokeBorder(theme.colors.border, lineWidth: 1)
                )

            Text(name)
                .font(theme.typography.caption)
                .foregroundStyle(theme.colors.textSecondary)
        }
    }

    private func accentChip(_ label: String, bg: Color, fg: Color) -> some View {
        Text(label)
            .font(theme.typography.caption)
            .foregroundStyle(fg)
            .padding(.horizontal, theme.spacing.md)
            .padding(.vertical, theme.spacing.xs)
            .background(
                RoundedRectangle(cornerRadius: theme.radii.sm)
                    .fill(bg)
            )
    }
}

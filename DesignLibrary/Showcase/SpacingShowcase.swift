import SwiftUI

struct SpacingShowcase: View {
    @Environment(ThemeManager.self) var theme

    var body: some View {
        VStack(alignment: .leading, spacing: theme.spacing.xl) {
            DSText("Spacing & Sizing", style: .h2)
            DSText("Spacing scale, border radii, and component dimensions.", style: .body, color: theme.colors.textSecondary)

            sectionDivider

            // Spacing scale
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Spacing Scale", style: .h4, color: theme.colors.textTertiary)

                VStack(spacing: theme.spacing.sm) {
                    spacingRow("xxs", value: theme.spacing.xxs)
                    spacingRow("xs", value: theme.spacing.xs)
                    spacingRow("sm", value: theme.spacing.sm)
                    spacingRow("md", value: theme.spacing.md)
                    spacingRow("lg", value: theme.spacing.lg)
                    spacingRow("xl", value: theme.spacing.xl)
                    spacingRow("xxl", value: theme.spacing.xxl)
                    spacingRow("xxxl", value: theme.spacing.xxxl)
                }
            }

            sectionDivider

            // Border radii
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Border Radii", style: .h4, color: theme.colors.textTertiary)

                HStack(spacing: theme.spacing.xl) {
                    radiusBox("sm", value: theme.radii.sm)
                    radiusBox("md", value: theme.radii.md)
                    radiusBox("lg", value: theme.radii.lg)
                    radiusBox("xl", value: theme.radii.xl)
                    radiusBox("full", value: theme.radii.full)
                }
            }

            sectionDivider

            // Component sizing
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Button Heights", style: .h4, color: theme.colors.textTertiary)

                VStack(spacing: theme.spacing.sm) {
                    sizingRow("Small", value: theme.sizing.buttonHeightSm)
                    sizingRow("Medium", value: theme.sizing.buttonHeightMd)
                    sizingRow("Large", value: theme.sizing.buttonHeightLg)
                }
            }

            sectionDivider

            // Slider sizing
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Slider Dimensions", style: .h4, color: theme.colors.textTertiary)

                HStack(spacing: theme.spacing.xxl) {
                    VStack(spacing: theme.spacing.xs) {
                        RoundedRectangle(cornerRadius: theme.sizing.sliderTrackHeight / 2)
                            .fill(theme.colors.accent)
                            .frame(width: 80, height: theme.sizing.sliderTrackHeight)

                        Text("Track: \(Int(theme.sizing.sliderTrackHeight))pt")
                            .font(.system(size: 10, design: .monospaced))
                            .foregroundStyle(theme.colors.textTertiary)
                    }

                    VStack(spacing: theme.spacing.xs) {
                        Circle()
                            .fill(theme.colors.accent)
                            .frame(width: theme.sizing.sliderThumbSize, height: theme.sizing.sliderThumbSize)
                            .shadow(color: .black.opacity(0.15), radius: 2, y: 1)

                        Text("Thumb: \(Int(theme.sizing.sliderThumbSize))pt")
                            .font(.system(size: 10, design: .monospaced))
                            .foregroundStyle(theme.colors.textTertiary)
                    }
                }
            }

            sectionDivider

            // Icon sizes
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Icon Sizes", style: .h4, color: theme.colors.textTertiary)

                HStack(spacing: theme.spacing.xxl) {
                    iconBox("sm", size: theme.sizing.iconSm)
                    iconBox("md", size: theme.sizing.iconMd)
                    iconBox("lg", size: theme.sizing.iconLg)
                }
            }

            sectionDivider

            // Spacing visualization
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Padding Demo", style: .h4, color: theme.colors.textTertiary)

                HStack(spacing: theme.spacing.lg) {
                    paddingDemo("sm", padding: theme.spacing.sm)
                    paddingDemo("md", padding: theme.spacing.md)
                    paddingDemo("lg", padding: theme.spacing.lg)
                    paddingDemo("xl", padding: theme.spacing.xl)
                }
            }
        }
    }

    private var sectionDivider: some View {
        Rectangle()
            .fill(theme.colors.borderSubtle)
            .frame(height: 1)
    }

    private func spacingRow(_ label: String, value: CGFloat) -> some View {
        HStack(spacing: theme.spacing.md) {
            Text(label)
                .font(.system(size: 11, weight: .medium, design: .monospaced))
                .foregroundStyle(theme.colors.textTertiary)
                .frame(width: 36, alignment: .trailing)

            RoundedRectangle(cornerRadius: 2)
                .fill(theme.colors.accent.opacity(0.8))
                .frame(width: max(2, value), height: 18)

            Text("\(Int(value))pt")
                .font(.system(size: 10, design: .monospaced))
                .foregroundStyle(theme.colors.textTertiary)
                .frame(width: 32, alignment: .leading)
        }
    }

    private func radiusBox(_ label: String, value: CGFloat) -> some View {
        VStack(spacing: theme.spacing.xs) {
            RoundedRectangle(cornerRadius: min(value, 24))
                .fill(theme.colors.accentSubtle)
                .overlay(
                    RoundedRectangle(cornerRadius: min(value, 24))
                        .strokeBorder(theme.colors.accent, lineWidth: 1.5)
                )
                .frame(width: 48, height: 48)

            Text(label)
                .font(.system(size: 10, weight: .medium, design: .monospaced))
                .foregroundStyle(theme.colors.textTertiary)

            Text("\(Int(value))pt")
                .font(.system(size: 9, design: .monospaced))
                .foregroundStyle(theme.colors.textTertiary)
        }
    }

    private func sizingRow(_ label: String, value: CGFloat) -> some View {
        HStack(spacing: theme.spacing.md) {
            Text(label)
                .font(.system(size: 11, weight: .medium, design: .monospaced))
                .foregroundStyle(theme.colors.textTertiary)
                .frame(width: 60, alignment: .trailing)

            RoundedRectangle(cornerRadius: theme.radii.md)
                .fill(theme.colors.surfaceHover)
                .overlay(
                    RoundedRectangle(cornerRadius: theme.radii.md)
                        .strokeBorder(theme.colors.borderSubtle, lineWidth: 1)
                )
                .frame(width: 120, height: value)

            Text("\(Int(value))pt")
                .font(.system(size: 10, design: .monospaced))
                .foregroundStyle(theme.colors.textTertiary)
        }
    }

    private func iconBox(_ label: String, size: CGFloat) -> some View {
        VStack(spacing: theme.spacing.xs) {
            Image(systemName: "star.fill")
                .font(.system(size: size))
                .foregroundStyle(theme.colors.accent)

            Text(label)
                .font(.system(size: 10, weight: .medium, design: .monospaced))
                .foregroundStyle(theme.colors.textTertiary)

            Text("\(Int(size))pt")
                .font(.system(size: 9, design: .monospaced))
                .foregroundStyle(theme.colors.textTertiary)
        }
    }

    private func paddingDemo(_ label: String, padding: CGFloat) -> some View {
        VStack(spacing: theme.spacing.xs) {
            ZStack {
                RoundedRectangle(cornerRadius: theme.radii.md)
                    .fill(theme.colors.accentSubtle)

                RoundedRectangle(cornerRadius: theme.radii.sm)
                    .fill(theme.colors.accent.opacity(0.3))
                    .padding(padding)
            }
            .frame(width: 72, height: 72)

            Text(label)
                .font(.system(size: 10, weight: .medium, design: .monospaced))
                .foregroundStyle(theme.colors.textTertiary)

            Text("\(Int(padding))pt")
                .font(.system(size: 9, design: .monospaced))
                .foregroundStyle(theme.colors.textTertiary)
        }
    }
}

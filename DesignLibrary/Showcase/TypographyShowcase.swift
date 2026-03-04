import SwiftUI

struct TypographyShowcase: View {
    @Environment(ThemeManager.self) var theme

    var body: some View {
        VStack(alignment: .leading, spacing: theme.spacing.xl) {
            DSText("Typography", style: .h2)
            DSText("Font styles, text hierarchy, and paragraph treatments.", style: .body, color: theme.colors.textSecondary)

            sectionDivider

            // Headings
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Headings", style: .h4, color: theme.colors.textTertiary)

                typographyRow("H1", font: theme.typography.h1, size: theme.typography.h1Size, weight: theme.typography.h1Weight)
                typographyRow("H2", font: theme.typography.h2, size: theme.typography.h2Size, weight: theme.typography.h2Weight)
                typographyRow("H3", font: theme.typography.h3, size: theme.typography.h3Size, weight: theme.typography.h3Weight)
                typographyRow("H4", font: theme.typography.h4, size: theme.typography.h4Size, weight: theme.typography.h4Weight)
            }

            sectionDivider

            // Body text
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Body Text", style: .h4, color: theme.colors.textTertiary)

                typographyRow("Body Large", font: theme.typography.bodyLarge, size: theme.typography.bodyLargeSize, weight: theme.typography.bodyLargeWeight)
                typographyRow("Body", font: theme.typography.body, size: theme.typography.bodySize, weight: theme.typography.bodyWeight)
                typographyRow("Caption", font: theme.typography.caption, size: theme.typography.captionSize, weight: theme.typography.captionWeight)
            }

            sectionDivider

            // Paragraph specimens
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Paragraph Specimens", style: .h4, color: theme.colors.textTertiary)

                DSCard {
                    VStack(alignment: .leading, spacing: theme.spacing.md) {
                        Text("Body")
                            .font(theme.typography.caption)
                            .foregroundStyle(theme.colors.textTertiary)

                        Text("Typography is the art and technique of arranging type to make written language legible, readable, and appealing when displayed. The arrangement of type involves selecting typefaces, point sizes, line lengths, line-spacing, and letter-spacing, as well as adjusting the space between pairs of letters.")
                            .font(theme.typography.body)
                            .foregroundStyle(theme.colors.textPrimary)
                            .lineSpacing(theme.typography.bodyLineSpacing)
                    }
                }

                DSCard {
                    VStack(alignment: .leading, spacing: theme.spacing.md) {
                        Text("Caption")
                            .font(theme.typography.caption)
                            .foregroundStyle(theme.colors.textTertiary)

                        Text("Typography is the art and technique of arranging type to make written language legible, readable, and appealing when displayed. The arrangement of type involves selecting typefaces, point sizes, line lengths, and letter-spacing.")
                            .font(theme.typography.caption)
                            .foregroundStyle(theme.colors.textSecondary)
                            .lineSpacing(theme.typography.captionLineSpacing)
                    }
                }
            }

            sectionDivider

            // Full hierarchy demo
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Hierarchy Demo", style: .h4, color: theme.colors.textTertiary)

                DSCard {
                    VStack(alignment: .leading, spacing: theme.spacing.md) {
                        Text("Audio Reactive Engine")
                            .font(theme.typography.h1)
                            .foregroundStyle(theme.colors.textPrimary)
                        Text("Real-time Visual Programming")
                            .font(theme.typography.h2)
                            .foregroundStyle(theme.colors.textSecondary)
                        Text("Connect sources to effects in a node-based graph to create dynamic visuals driven by audio input.")
                            .font(theme.typography.body)
                            .foregroundStyle(theme.colors.textSecondary)
                            .lineSpacing(theme.typography.bodyLineSpacing)
                        Text("Supports MIDI, BPM detection, and 155+ shader effects")
                            .font(theme.typography.caption)
                            .foregroundStyle(theme.colors.textTertiary)
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

    private func typographyRow(_ label: String, font: Font, size: CGFloat, weight: Font.Weight) -> some View {
        HStack(alignment: .firstTextBaseline) {
            Text(label)
                .font(.system(size: 11, weight: .medium, design: .monospaced))
                .foregroundStyle(theme.colors.textTertiary)
                .frame(width: 80, alignment: .leading)

            Text("The quick brown fox jumps over the lazy dog")
                .font(font)
                .foregroundStyle(theme.colors.textPrimary)

            Spacer()

            Text("\(Int(size))pt \(weightName(weight))")
                .font(.system(size: 10, weight: .regular, design: .monospaced))
                .foregroundStyle(theme.colors.textTertiary)
        }
    }

    private func weightName(_ weight: Font.Weight) -> String {
        switch weight {
        case .ultraLight: return "UltraLight"
        case .thin: return "Thin"
        case .light: return "Light"
        case .regular: return "Regular"
        case .medium: return "Medium"
        case .semibold: return "Semibold"
        case .bold: return "Bold"
        case .heavy: return "Heavy"
        case .black: return "Black"
        default: return "Regular"
        }
    }
}

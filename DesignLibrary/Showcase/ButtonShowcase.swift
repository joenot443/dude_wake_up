import SwiftUI

struct ButtonShowcase: View {
    @Environment(ThemeManager.self) var theme

    var body: some View {
        VStack(alignment: .leading, spacing: theme.spacing.xl) {
            DSText("Buttons", style: .h2)
            DSText("Button variants, sizes, and states.", style: .body, color: theme.colors.textSecondary)

            sectionDivider

            // Variants
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Variants", style: .h4, color: theme.colors.textTertiary)

                HStack(spacing: theme.spacing.md) {
                    DSButton("Primary", variant: .primary) {}
                    DSButton("Secondary", variant: .secondary) {}
                    DSButton("Ghost", variant: .ghost) {}
                    DSButton("Destructive", variant: .destructive) {}
                }
            }

            sectionDivider

            // Sizes
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Sizes", style: .h4, color: theme.colors.textTertiary)

                HStack(alignment: .center, spacing: theme.spacing.md) {
                    DSButton("Small", variant: .primary, size: .sm) {}
                    DSButton("Medium", variant: .primary, size: .md) {}
                    DSButton("Large", variant: .primary, size: .lg) {}
                }

                HStack(alignment: .center, spacing: theme.spacing.md) {
                    DSButton("Small", variant: .secondary, size: .sm) {}
                    DSButton("Medium", variant: .secondary, size: .md) {}
                    DSButton("Large", variant: .secondary, size: .lg) {}
                }
            }

            sectionDivider

            // With Icons
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("With Icons", style: .h4, color: theme.colors.textTertiary)

                HStack(spacing: theme.spacing.md) {
                    DSButton("Add Node", variant: .primary, icon: "plus") {}
                    DSButton("Settings", variant: .secondary, icon: "gear") {}
                    DSButton("Delete", variant: .destructive, icon: "trash") {}
                    DSButton("Share", variant: .ghost, icon: "square.and.arrow.up") {}
                }
            }

            sectionDivider

            // Disabled
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Disabled", style: .h4, color: theme.colors.textTertiary)

                HStack(spacing: theme.spacing.md) {
                    DSButton("Primary", variant: .primary) {}
                        .disabled(true)
                    DSButton("Secondary", variant: .secondary) {}
                        .disabled(true)
                    DSButton("Ghost", variant: .ghost) {}
                        .disabled(true)
                    DSButton("Destructive", variant: .destructive) {}
                        .disabled(true)
                }
            }

            sectionDivider

            // Full Width
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Full Width", style: .h4, color: theme.colors.textTertiary)

                DSButton("Save Workspace", variant: .primary, size: .lg, icon: "arrow.down.doc", fullWidth: true) {}
                DSButton("Cancel", variant: .secondary, fullWidth: true) {}
            }

            sectionDivider

            // Composition example
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Composition", style: .h4, color: theme.colors.textTertiary)

                DSCard {
                    VStack(alignment: .leading, spacing: theme.spacing.lg) {
                        Text("Delete Workspace?")
                            .font(theme.typography.h3)
                            .foregroundStyle(theme.colors.textPrimary)

                        Text("This action cannot be undone. All nodes and connections in this workspace will be permanently removed.")
                            .font(theme.typography.body)
                            .foregroundStyle(theme.colors.textSecondary)
                            .lineSpacing(theme.typography.bodyLineSpacing)

                        HStack(spacing: theme.spacing.md) {
                            Spacer()
                            DSButton("Cancel", variant: .ghost) {}
                            DSButton("Delete", variant: .destructive, icon: "trash") {}
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
}

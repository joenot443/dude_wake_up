import SwiftUI

struct IconButtonShowcase: View {
    @Environment(ThemeManager.self) var theme
    @State private var sidebarActive = false
    @State private var audioActive = true
    @State private var stageActive = false

    var body: some View {
        VStack(alignment: .leading, spacing: theme.spacing.xl) {
            DSText("Icon Buttons", style: .h2)
            DSText("Compact icon-only buttons for toolbars and action bars.", style: .body, color: theme.colors.textSecondary)

            sectionDivider

            // Variants
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Variants", style: .h4, color: theme.colors.textTertiary)

                HStack(spacing: theme.spacing.lg) {
                    labeledIcon("Standard", icon: "gear", variant: .standard)
                    labeledIcon("Accent", icon: "star.fill", variant: .accent)
                    labeledIcon("Destructive", icon: "trash", variant: .destructive)
                }
            }

            sectionDivider

            // Sizes
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Sizes", style: .h4, color: theme.colors.textTertiary)

                HStack(alignment: .center, spacing: theme.spacing.lg) {
                    VStack(spacing: theme.spacing.xs) {
                        DSIconButton("plus", size: .sm) {}
                        Text("sm").font(theme.typography.caption).foregroundStyle(theme.colors.textTertiary)
                    }
                    VStack(spacing: theme.spacing.xs) {
                        DSIconButton("plus", size: .md) {}
                        Text("md").font(theme.typography.caption).foregroundStyle(theme.colors.textTertiary)
                    }
                    VStack(spacing: theme.spacing.xs) {
                        DSIconButton("plus", size: .lg) {}
                        Text("lg").font(theme.typography.caption).foregroundStyle(theme.colors.textTertiary)
                    }
                }
            }

            sectionDivider

            // Active / toggle state
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Active State", style: .h4, color: theme.colors.textTertiary)
                DSText("Icon buttons can show a toggle/active state with accent highlight.", style: .caption)

                HStack(spacing: theme.spacing.md) {
                    DSIconButton("sidebar.leading", isActive: sidebarActive) { sidebarActive.toggle() }
                    DSIconButton("waveform", isActive: audioActive) { audioActive.toggle() }
                    DSIconButton("slider.horizontal.2.rectangle.and.arrow.triangle.2.circlepath", isActive: stageActive) { stageActive.toggle() }
                }
            }

            sectionDivider

            // Disabled
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Disabled", style: .h4, color: theme.colors.textTertiary)

                HStack(spacing: theme.spacing.md) {
                    DSIconButton("arrow.uturn.backward") {}.disabled(true)
                    DSIconButton("arrow.uturn.forward") {}.disabled(true)
                    DSIconButton("trash", variant: .destructive) {}.disabled(true)
                }
            }

            sectionDivider

            // Toolbar mockup
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Toolbar Mockup", style: .h4, color: theme.colors.textTertiary)

                DSCard {
                    HStack(spacing: theme.spacing.md) {
                        DSIconButton("arrow.uturn.backward", tooltip: "Undo") {}
                        DSIconButton("arrow.uturn.forward", tooltip: "Redo") {}.disabled(true)

                        toolbarDivider

                        DSIconButton("sidebar.leading", isActive: sidebarActive, tooltip: "Browser") {
                            sidebarActive.toggle()
                        }
                        DSIconButton("waveform", isActive: audioActive, tooltip: "Audio Panel") {
                            audioActive.toggle()
                        }
                        DSIconButton("slider.horizontal.2.rectangle.and.arrow.triangle.2.circlepath", isActive: stageActive, tooltip: "Stage Mode") {
                            stageActive.toggle()
                        }

                        toolbarDivider

                        DSIconButton("trash", variant: .destructive, tooltip: "Delete") {}

                        Spacer()
                    }
                    .padding(.vertical, theme.spacing.xs)
                }
            }

            sectionDivider

            // Node action bar mockup
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Node Action Bar", style: .h4, color: theme.colors.textTertiary)

                DSCard {
                    HStack(spacing: theme.spacing.sm) {
                        DSIconButton("pause.fill", size: .sm) {}
                        DSIconButton("arrow.triangle.2.circlepath", size: .sm) {}
                        Spacer()
                        DSIconButton("trash", variant: .destructive, size: .sm) {}
                    }
                    .padding(.vertical, theme.spacing.xxs)
                }
                .frame(width: 220)
            }

            sectionDivider

            // Common icons reference
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Common Icons", style: .h4, color: theme.colors.textTertiary)

                let icons: [(String, String)] = [
                    ("plus", "Add"),
                    ("trash", "Delete"),
                    ("gear", "Settings"),
                    ("arrow.uturn.backward", "Undo"),
                    ("arrow.uturn.forward", "Redo"),
                    ("play.fill", "Play"),
                    ("pause.fill", "Pause"),
                    ("sidebar.leading", "Sidebar"),
                    ("waveform", "Audio"),
                    ("square.and.arrow.up", "Share"),
                    ("doc.on.doc", "Duplicate"),
                    ("eye", "Preview"),
                    ("arrow.triangle.2.circlepath", "Swap"),
                    ("star.fill", "Favorite"),
                    ("xmark", "Close"),
                ]

                LazyVGrid(
                    columns: [GridItem(.adaptive(minimum: 80), spacing: theme.spacing.md)],
                    spacing: theme.spacing.md
                ) {
                    ForEach(icons, id: \.0) { icon, label in
                        VStack(spacing: theme.spacing.xs) {
                            DSIconButton(icon) {}
                            Text(label)
                                .font(theme.typography.caption)
                                .foregroundStyle(theme.colors.textTertiary)
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

    private var toolbarDivider: some View {
        Rectangle()
            .fill(theme.colors.border)
            .frame(width: 1, height: 20)
    }

    private func labeledIcon(_ label: String, icon: String, variant: DSIconButtonVariant) -> some View {
        VStack(spacing: theme.spacing.sm) {
            DSIconButton(icon, variant: variant) {}
            Text(label)
                .font(theme.typography.caption)
                .foregroundStyle(theme.colors.textTertiary)
        }
    }
}

import SwiftUI

// Demo enum for showcase
private enum ViewMode: String, CaseIterable, CustomStringConvertible, Hashable {
    case bands = "Bands"
    case bars = "Bars"
    case waveform = "Waveform"

    var description: String { rawValue }
}

private enum BlendMode: String, CaseIterable, CustomStringConvertible, Hashable {
    case normal = "Normal"
    case add = "Add"
    case multiply = "Multiply"
    case screen = "Screen"

    var description: String { rawValue }
}

struct PickerShowcase: View {
    @Environment(ThemeManager.self) var theme

    @State private var menuMode = ViewMode.bands
    @State private var segMode = ViewMode.bars
    @State private var blendMode = BlendMode.normal
    @State private var smMenu = ViewMode.waveform
    @State private var smSeg = ViewMode.bands
    @State private var labeledMenu = BlendMode.add
    @State private var labeledSeg = ViewMode.waveform
    @State private var disabledMenu = ViewMode.bands
    @State private var disabledSeg = ViewMode.bars
    @State private var compBlend = BlendMode.screen
    @State private var compView = ViewMode.bars

    var body: some View {
        VStack(alignment: .leading, spacing: theme.spacing.xl) {
            DSText("Pickers", style: .h2)
            DSText("Selection controls with menu and segmented styles.", style: .body, color: theme.colors.textSecondary)

            sectionDivider

            // Menu Style
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Menu Style", style: .h4, color: theme.colors.textTertiary)

                HStack(spacing: theme.spacing.lg) {
                    DSPicker(
                        selection: $menuMode,
                        options: ViewMode.allCases.map { DSPickerOption(value: $0, label: $0.rawValue) },
                        style: .menu
                    )
                    .frame(width: 160)

                    DSPicker(
                        selection: $blendMode,
                        options: BlendMode.allCases.map { DSPickerOption(value: $0, label: $0.rawValue) },
                        style: .menu
                    )
                    .frame(width: 160)
                }
            }

            sectionDivider

            // Segmented Style
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Segmented Style", style: .h4, color: theme.colors.textTertiary)

                DSPicker(
                    selection: $segMode,
                    options: ViewMode.allCases.map { DSPickerOption(value: $0, label: $0.rawValue) },
                    style: .segmented
                )
                .frame(width: 320)
            }

            sectionDivider

            // Sizes
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Sizes", style: .h4, color: theme.colors.textTertiary)

                HStack(spacing: theme.spacing.xl) {
                    VStack(alignment: .leading, spacing: theme.spacing.sm) {
                        DSText("Small", style: .caption)
                        DSPicker(
                            selection: $smMenu,
                            options: ViewMode.allCases.map { DSPickerOption(value: $0, label: $0.rawValue) },
                            style: .menu, size: .sm
                        )
                        .frame(width: 140)
                    }
                    VStack(alignment: .leading, spacing: theme.spacing.sm) {
                        DSText("Medium", style: .caption)
                        DSPicker(
                            selection: $menuMode,
                            options: ViewMode.allCases.map { DSPickerOption(value: $0, label: $0.rawValue) },
                            style: .menu, size: .md
                        )
                        .frame(width: 160)
                    }
                }

                HStack(spacing: theme.spacing.xl) {
                    VStack(alignment: .leading, spacing: theme.spacing.sm) {
                        DSText("Small", style: .caption)
                        DSPicker(
                            selection: $smSeg,
                            options: ViewMode.allCases.map { DSPickerOption(value: $0, label: $0.rawValue) },
                            style: .segmented, size: .sm
                        )
                        .frame(width: 260)
                    }
                    VStack(alignment: .leading, spacing: theme.spacing.sm) {
                        DSText("Medium", style: .caption)
                        DSPicker(
                            selection: $segMode,
                            options: ViewMode.allCases.map { DSPickerOption(value: $0, label: $0.rawValue) },
                            style: .segmented, size: .md
                        )
                        .frame(width: 320)
                    }
                }
            }

            sectionDivider

            // With Labels
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("With Labels", style: .h4, color: theme.colors.textTertiary)

                HStack(spacing: theme.spacing.xl) {
                    DSPicker(
                        selection: $labeledMenu,
                        options: BlendMode.allCases.map { DSPickerOption(value: $0, label: $0.rawValue) },
                        label: "Blend Mode", style: .menu
                    )
                    .frame(width: 180)

                    DSPicker(
                        selection: $labeledSeg,
                        options: ViewMode.allCases.map { DSPickerOption(value: $0, label: $0.rawValue) },
                        label: "View Mode", style: .segmented
                    )
                    .frame(width: 320)
                }
            }

            sectionDivider

            // Disabled
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Disabled", style: .h4, color: theme.colors.textTertiary)

                HStack(spacing: theme.spacing.xl) {
                    DSPicker(
                        selection: $disabledMenu,
                        options: ViewMode.allCases.map { DSPickerOption(value: $0, label: $0.rawValue) },
                        label: "Menu", style: .menu
                    )
                    .frame(width: 160)
                    .disabled(true)

                    DSPicker(
                        selection: $disabledSeg,
                        options: ViewMode.allCases.map { DSPickerOption(value: $0, label: $0.rawValue) },
                        label: "Segmented", style: .segmented
                    )
                    .frame(width: 280)
                    .disabled(true)
                }
            }

            sectionDivider

            // Composition
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Composition", style: .h4, color: theme.colors.textTertiary)

                DSCard {
                    VStack(alignment: .leading, spacing: theme.spacing.lg) {
                        Text("Effect Settings")
                            .font(theme.typography.h4)
                            .foregroundStyle(theme.colors.textPrimary)

                        DSPicker(
                            selection: $compView,
                            options: ViewMode.allCases.map { DSPickerOption(value: $0, label: $0.rawValue) },
                            label: "View Mode", style: .segmented
                        )

                        DSPicker(
                            selection: $compBlend,
                            options: BlendMode.allCases.map { DSPickerOption(value: $0, label: $0.rawValue) },
                            label: "Blend Mode", style: .menu
                        )

                        HStack(spacing: theme.spacing.md) {
                            Spacer()
                            DSButton("Reset", variant: .ghost, size: .sm) {}
                            DSButton("Apply", variant: .primary, size: .sm) {}
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

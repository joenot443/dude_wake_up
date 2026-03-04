import SwiftUI

struct ToggleShowcase: View {
    @Environment(ThemeManager.self) var theme

    @State private var switchOn = true
    @State private var switchOff = false
    @State private var checkOn = true
    @State private var checkOff = false
    @State private var labeled1 = true
    @State private var labeled2 = false
    @State private var labeled3 = true
    @State private var smSwitch = true
    @State private var smCheck = false
    @State private var disabledOn = true
    @State private var disabledOff = false
    @State private var oscEnabled = true
    @State private var audioReactive = false
    @State private var snapToBeat = true

    var body: some View {
        VStack(alignment: .leading, spacing: theme.spacing.xl) {
            DSText("Toggles", style: .h2)
            DSText("Binary on/off controls with switch and checkbox styles.", style: .body, color: theme.colors.textSecondary)

            sectionDivider

            // Switch Style
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Switch Style", style: .h4, color: theme.colors.textTertiary)

                HStack(spacing: theme.spacing.xl) {
                    DSToggle(isOn: $switchOn, label: "On")
                    DSToggle(isOn: $switchOff, label: "Off")
                }
            }

            sectionDivider

            // Checkbox Style
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Checkbox Style", style: .h4, color: theme.colors.textTertiary)

                HStack(spacing: theme.spacing.xl) {
                    DSToggle(isOn: $checkOn, label: "Checked", style: .checkbox)
                    DSToggle(isOn: $checkOff, label: "Unchecked", style: .checkbox)
                }
            }

            sectionDivider

            // With Labels
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("With Labels", style: .h4, color: theme.colors.textTertiary)

                VStack(alignment: .leading, spacing: theme.spacing.md) {
                    DSToggle(isOn: $labeled1, label: "Enable Oscillator", style: .switch)
                    DSToggle(isOn: $labeled2, label: "Audio Reactive", style: .switch)
                    DSToggle(isOn: $labeled3, label: "Snap to Beat", style: .checkbox)
                }
            }

            sectionDivider

            // Sizes
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Sizes", style: .h4, color: theme.colors.textTertiary)

                HStack(spacing: theme.spacing.xl) {
                    VStack(alignment: .leading, spacing: theme.spacing.sm) {
                        DSText("Small", style: .caption)
                        DSToggle(isOn: $smSwitch, style: .switch, size: .sm)
                        DSToggle(isOn: $smCheck, style: .checkbox, size: .sm)
                    }
                    VStack(alignment: .leading, spacing: theme.spacing.sm) {
                        DSText("Medium", style: .caption)
                        DSToggle(isOn: $switchOn, style: .switch, size: .md)
                        DSToggle(isOn: $checkOn, style: .checkbox, size: .md)
                    }
                }
            }

            sectionDivider

            // Disabled
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Disabled", style: .h4, color: theme.colors.textTertiary)

                HStack(spacing: theme.spacing.xl) {
                    DSToggle(isOn: $disabledOn, label: "Locked On")
                        .disabled(true)
                    DSToggle(isOn: $disabledOff, label: "Locked Off")
                        .disabled(true)
                    DSToggle(isOn: $disabledOn, label: "Checked", style: .checkbox)
                        .disabled(true)
                    DSToggle(isOn: $disabledOff, label: "Unchecked", style: .checkbox)
                        .disabled(true)
                }
            }

            sectionDivider

            // Composition
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Composition", style: .h4, color: theme.colors.textTertiary)

                DSCard {
                    VStack(alignment: .leading, spacing: theme.spacing.lg) {
                        Text("Oscillator Settings")
                            .font(theme.typography.h4)
                            .foregroundStyle(theme.colors.textPrimary)

                        VStack(alignment: .leading, spacing: theme.spacing.md) {
                            DSToggle(isOn: $oscEnabled, label: "Enable Oscillator", style: .switch)
                            DSToggle(isOn: $audioReactive, label: "Audio Reactive", style: .switch)
                            DSToggle(isOn: $snapToBeat, label: "Snap to Beat", style: .checkbox)
                        }

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

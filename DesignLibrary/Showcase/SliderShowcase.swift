import SwiftUI

struct SliderShowcase: View {
    @Environment(ThemeManager.self) var theme

    @State private var basicValue: Float = 0.5
    @State private var volumeValue: Float = 0.75
    @State private var brightnessValue: Float = 0.3
    @State private var frequencyValue: Float = 440.0
    @State private var mixValue: Float = 0.5
    @State private var decayValue: Float = 0.8
    @State private var rateValue: Float = 120.0

    var body: some View {
        VStack(alignment: .leading, spacing: theme.spacing.xl) {
            DSText("Sliders", style: .h2)
            DSText("Continuous value controls with custom styling.", style: .body, color: theme.colors.textSecondary)

            sectionDivider

            // Basic
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Basic", style: .h4, color: theme.colors.textTertiary)

                DSSlider(value: $basicValue, range: 0...1)
            }

            sectionDivider

            // With labels
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("With Labels", style: .h4, color: theme.colors.textTertiary)

                DSSlider(value: $volumeValue, range: 0...1, label: "Volume", showValue: true)
                DSSlider(value: $brightnessValue, range: 0...1, label: "Brightness", showValue: true)
                DSSlider(value: $frequencyValue, range: 20...20000, label: "Frequency (Hz)", showValue: true, formatString: "%.0f")
            }

            sectionDivider

            // In cards
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("In Cards", style: .h4, color: theme.colors.textTertiary)

                DSCard {
                    VStack(spacing: theme.spacing.lg) {
                        DSSlider(value: $volumeValue, range: 0...1, label: "Master Volume", showValue: true)
                        DSSlider(value: $brightnessValue, range: 0...1, label: "Intensity", showValue: true)
                        DSSlider(value: $mixValue, range: 0...1, label: "Dry / Wet", showValue: true)
                    }
                }
            }

            sectionDivider

            // Parameter panel mockup
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Parameter Panel", style: .h4, color: theme.colors.textTertiary)

                DSCard {
                    VStack(alignment: .leading, spacing: theme.spacing.lg) {
                        Text("Plasma Shader")
                            .font(theme.typography.h4)
                            .foregroundStyle(theme.colors.textPrimary)

                        VStack(spacing: theme.spacing.md) {
                            DSSlider(value: $frequencyValue, range: 20...20000, label: "Frequency", showValue: true, formatString: "%.0f")
                            DSSlider(value: $decayValue, range: 0...1, label: "Decay", showValue: true)
                            DSSlider(value: $rateValue, range: 30...240, label: "Rate (BPM)", showValue: true, formatString: "%.0f")
                            DSSlider(value: $mixValue, range: 0...1, label: "Mix", showValue: true)
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

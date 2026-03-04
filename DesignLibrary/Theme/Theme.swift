import SwiftUI

// MARK: - Color Hex Extension

extension Color {
    init(hex: UInt, opacity: Double = 1.0) {
        self.init(
            .sRGB,
            red: Double((hex >> 16) & 0xFF) / 255.0,
            green: Double((hex >> 8) & 0xFF) / 255.0,
            blue: Double(hex & 0xFF) / 255.0,
            opacity: opacity
        )
    }
}

// MARK: - Design Tokens

struct DesignTokens {
    let name: String
    let description: String
    let colors: ColorTokens
    let typography: TypographyTokens
    let spacing: SpacingTokens
    let radii: RadiiTokens
    let sizing: SizingTokens
}

// MARK: - Color Tokens

struct ColorTokens {
    // Backgrounds
    let background: Color
    let backgroundSecondary: Color
    let backgroundTertiary: Color

    // Surfaces (elevated elements like cards, popovers)
    let surface: Color
    let surfaceHover: Color
    let surfacePressed: Color

    // Borders
    let border: Color
    let borderSubtle: Color

    // Text
    let textPrimary: Color
    let textSecondary: Color
    let textTertiary: Color
    let textOnAccent: Color

    // Accent
    let accent: Color
    let accentHover: Color
    let accentSubtle: Color

    // Semantic
    let destructive: Color
    let destructiveSubtle: Color
    let success: Color
    let successSubtle: Color
    let warning: Color
    let warningSubtle: Color
}

// MARK: - Typography Tokens

struct TypographyTokens {
    let fontDesign: Font.Design

    let h1Size: CGFloat
    let h1Weight: Font.Weight
    let h2Size: CGFloat
    let h2Weight: Font.Weight
    let h3Size: CGFloat
    let h3Weight: Font.Weight
    let h4Size: CGFloat
    let h4Weight: Font.Weight
    let bodyLargeSize: CGFloat
    let bodyLargeWeight: Font.Weight
    let bodySize: CGFloat
    let bodyWeight: Font.Weight
    let captionSize: CGFloat
    let captionWeight: Font.Weight

    let bodyLineSpacing: CGFloat
    let captionLineSpacing: CGFloat

    var h1: Font { .system(size: h1Size, weight: h1Weight, design: fontDesign) }
    var h2: Font { .system(size: h2Size, weight: h2Weight, design: fontDesign) }
    var h3: Font { .system(size: h3Size, weight: h3Weight, design: fontDesign) }
    var h4: Font { .system(size: h4Size, weight: h4Weight, design: fontDesign) }
    var bodyLarge: Font { .system(size: bodyLargeSize, weight: bodyLargeWeight, design: fontDesign) }
    var body: Font { .system(size: bodySize, weight: bodyWeight, design: fontDesign) }
    var caption: Font { .system(size: captionSize, weight: captionWeight, design: fontDesign) }
}

// MARK: - Spacing Tokens

struct SpacingTokens {
    let xxs: CGFloat   // 2
    let xs: CGFloat    // 4
    let sm: CGFloat    // 8
    let md: CGFloat    // 12
    let lg: CGFloat    // 16
    let xl: CGFloat    // 24
    let xxl: CGFloat   // 32
    let xxxl: CGFloat  // 48
}

// MARK: - Radii Tokens

struct RadiiTokens {
    let sm: CGFloat
    let md: CGFloat
    let lg: CGFloat
    let xl: CGFloat
    let full: CGFloat  // 999 — pill shape
}

// MARK: - Sizing Tokens

struct SizingTokens {
    let buttonHeightSm: CGFloat
    let buttonHeightMd: CGFloat
    let buttonHeightLg: CGFloat
    let sliderTrackHeight: CGFloat
    let sliderThumbSize: CGFloat
    let iconSm: CGFloat
    let iconMd: CGFloat
    let iconLg: CGFloat
    let toggleTrackWidth: CGFloat
    let toggleTrackHeight: CGFloat
    let toggleThumbSize: CGFloat
    let checkboxSize: CGFloat
}

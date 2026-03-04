import SwiftUI

enum ThemePreset: String, CaseIterable, Identifiable {
    case default_ = "Default"
    case ember = "Ember"
    case arctic = "Arctic"
    case forest = "Forest"

    var id: String { rawValue }

    var tokens: DesignTokens {
        switch self {
        case .default_: return Self.midnightTokens
        case .ember: return Self.emberTokens
        case .arctic: return Self.arcticTokens
        case .forest: return Self.forestTokens
        }
    }

    /// Light-mode color overrides for adaptive themes. Returns `nil` for single-appearance themes.
    var lightColorOverrides: ColorTokens? {
        switch self {
        case .default_: return Self.daylightTokens.colors
        default: return nil
        }
    }

    // MARK: - Midnight
    // Deep navy with electric blue accents. Precise, focused, developer-tool feel.

    private static let midnightTokens = DesignTokens(
        name: "Midnight",
        description: "Deep, precise, and focused",
        colors: ColorTokens(
            background: Color(hex: 0x0B0E14),
            backgroundSecondary: Color(hex: 0x131720),
            backgroundTertiary: Color(hex: 0x1C2130),
            surface: Color(hex: 0x1E2433),
            surfaceHover: Color(hex: 0x262D3F),
            surfacePressed: Color(hex: 0x2E3650),
            border: Color(hex: 0x2D3548),
            borderSubtle: Color(hex: 0x1E2433),
            textPrimary: Color(hex: 0xE6EDF3),
            textSecondary: Color(hex: 0x8B949E),
            textTertiary: Color(hex: 0x484F58),
            textOnAccent: Color(hex: 0xFFFFFF),
            accent: Color(hex: 0x58A6FF),
            accentHover: Color(hex: 0x79B8FF),
            accentSubtle: Color(hex: 0x0D2240),
            destructive: Color(hex: 0xF85149),
            destructiveSubtle: Color(hex: 0x3D1214),
            success: Color(hex: 0x3FB950),
            successSubtle: Color(hex: 0x0D2F18),
            warning: Color(hex: 0xD29922),
            warningSubtle: Color(hex: 0x2D1F0A)
        ),
        typography: TypographyTokens(
            fontDesign: .default,
            h1Size: 30, h1Weight: .bold,
            h2Size: 22, h2Weight: .semibold,
            h3Size: 16, h3Weight: .semibold,
            h4Size: 13, h4Weight: .medium,
            bodyLargeSize: 15, bodyLargeWeight: .regular,
            bodySize: 13, bodyWeight: .regular,
            captionSize: 11, captionWeight: .regular,
            bodyLineSpacing: 4, captionLineSpacing: 2
        ),
        spacing: SpacingTokens(xxs: 2, xs: 4, sm: 8, md: 12, lg: 16, xl: 24, xxl: 32, xxxl: 48),
        radii: RadiiTokens(sm: 4, md: 6, lg: 8, xl: 12, full: 999),
        sizing: SizingTokens(
            buttonHeightSm: 28, buttonHeightMd: 34, buttonHeightLg: 42,
            sliderTrackHeight: 3, sliderThumbSize: 14,
            iconSm: 14, iconMd: 18, iconLg: 22,
            toggleTrackWidth: 36, toggleTrackHeight: 20, toggleThumbSize: 16,
            checkboxSize: 16
        )
    )

    // MARK: - Daylight
    // Clean white with indigo accents. Rounded, spacious, editorial feel.

    private static let daylightTokens = DesignTokens(
        name: "Daylight",
        description: "Clean, spacious, and approachable",
        colors: ColorTokens(
            background: Color(hex: 0xFFFFFF),
            backgroundSecondary: Color(hex: 0xF7F7F8),
            backgroundTertiary: Color(hex: 0xEDEDF0),
            surface: Color(hex: 0xFFFFFF),
            surfaceHover: Color(hex: 0xF3F3F6),
            surfacePressed: Color(hex: 0xE8E8EC),
            border: Color(hex: 0xE0E0E5),
            borderSubtle: Color(hex: 0xEDEDF0),
            textPrimary: Color(hex: 0x1A1A2E),
            textSecondary: Color(hex: 0x6B6B80),
            textTertiary: Color(hex: 0xA0A0B0),
            textOnAccent: Color(hex: 0xFFFFFF),
            accent: Color(hex: 0x5B4CE0),
            accentHover: Color(hex: 0x4A3BCF),
            accentSubtle: Color(hex: 0xEEECFB),
            destructive: Color(hex: 0xE5484D),
            destructiveSubtle: Color(hex: 0xFDECED),
            success: Color(hex: 0x1A7F37),
            successSubtle: Color(hex: 0xE6F6EB),
            warning: Color(hex: 0xBF8700),
            warningSubtle: Color(hex: 0xFFF8E1)
        ),
        typography: TypographyTokens(
            fontDesign: .rounded,
            h1Size: 32, h1Weight: .bold,
            h2Size: 24, h2Weight: .semibold,
            h3Size: 18, h3Weight: .semibold,
            h4Size: 14, h4Weight: .medium,
            bodyLargeSize: 16, bodyLargeWeight: .regular,
            bodySize: 14, bodyWeight: .regular,
            captionSize: 12, captionWeight: .regular,
            bodyLineSpacing: 5, captionLineSpacing: 3
        ),
        spacing: SpacingTokens(xxs: 2, xs: 4, sm: 8, md: 14, lg: 18, xl: 28, xxl: 36, xxxl: 52),
        radii: RadiiTokens(sm: 6, md: 10, lg: 14, xl: 20, full: 999),
        sizing: SizingTokens(
            buttonHeightSm: 30, buttonHeightMd: 38, buttonHeightLg: 46,
            sliderTrackHeight: 5, sliderThumbSize: 18,
            iconSm: 16, iconMd: 20, iconLg: 24,
            toggleTrackWidth: 40, toggleTrackHeight: 22, toggleThumbSize: 18,
            checkboxSize: 18
        )
    )

    // MARK: - Ember
    // Dark warm brown with orange accents. Bold, dense, industrial feel.

    private static let emberTokens = DesignTokens(
        name: "Ember",
        description: "Warm, bold, and expressive",
        colors: ColorTokens(
            background: Color(hex: 0x151010),
            backgroundSecondary: Color(hex: 0x1E1614),
            backgroundTertiary: Color(hex: 0x2A201C),
            surface: Color(hex: 0x2A201C),
            surfaceHover: Color(hex: 0x352A25),
            surfacePressed: Color(hex: 0x40342E),
            border: Color(hex: 0x3D302A),
            borderSubtle: Color(hex: 0x2A201C),
            textPrimary: Color(hex: 0xF5EDE8),
            textSecondary: Color(hex: 0xA89890),
            textTertiary: Color(hex: 0x6B5D55),
            textOnAccent: Color(hex: 0x151010),
            accent: Color(hex: 0xF08C00),
            accentHover: Color(hex: 0xFFA033),
            accentSubtle: Color(hex: 0x2D1E0A),
            destructive: Color(hex: 0xFF6B6B),
            destructiveSubtle: Color(hex: 0x3D1A1A),
            success: Color(hex: 0x69DB7C),
            successSubtle: Color(hex: 0x0D2F18),
            warning: Color(hex: 0xFFD43B),
            warningSubtle: Color(hex: 0x2D2A0A)
        ),
        typography: TypographyTokens(
            fontDesign: .default,
            h1Size: 28, h1Weight: .heavy,
            h2Size: 21, h2Weight: .bold,
            h3Size: 16, h3Weight: .bold,
            h4Size: 13, h4Weight: .semibold,
            bodyLargeSize: 15, bodyLargeWeight: .regular,
            bodySize: 13, bodyWeight: .regular,
            captionSize: 11, captionWeight: .medium,
            bodyLineSpacing: 4, captionLineSpacing: 2
        ),
        spacing: SpacingTokens(xxs: 2, xs: 4, sm: 6, md: 10, lg: 14, xl: 22, xxl: 30, xxxl: 44),
        radii: RadiiTokens(sm: 3, md: 6, lg: 8, xl: 10, full: 999),
        sizing: SizingTokens(
            buttonHeightSm: 28, buttonHeightMd: 34, buttonHeightLg: 42,
            sliderTrackHeight: 4, sliderThumbSize: 14,
            iconSm: 14, iconMd: 18, iconLg: 22,
            toggleTrackWidth: 36, toggleTrackHeight: 20, toggleThumbSize: 16,
            checkboxSize: 16
        )
    )

    // MARK: - Arctic
    // Cool gray with teal accents. Airy, generous spacing, frosted feel.

    private static let arcticTokens = DesignTokens(
        name: "Arctic",
        description: "Airy, modern, and serene",
        colors: ColorTokens(
            background: Color(hex: 0xF0F4F8),
            backgroundSecondary: Color(hex: 0xE2E8F0),
            backgroundTertiary: Color(hex: 0xCBD5E1),
            surface: Color(hex: 0xFFFFFF),
            surfaceHover: Color(hex: 0xF1F5F9),
            surfacePressed: Color(hex: 0xE2E8F0),
            border: Color(hex: 0xCBD5E1),
            borderSubtle: Color(hex: 0xE2E8F0),
            textPrimary: Color(hex: 0x0F172A),
            textSecondary: Color(hex: 0x475569),
            textTertiary: Color(hex: 0x94A3B8),
            textOnAccent: Color(hex: 0xFFFFFF),
            accent: Color(hex: 0x0EA5E9),
            accentHover: Color(hex: 0x0284C7),
            accentSubtle: Color(hex: 0xE0F2FE),
            destructive: Color(hex: 0xEF4444),
            destructiveSubtle: Color(hex: 0xFEE2E2),
            success: Color(hex: 0x22C55E),
            successSubtle: Color(hex: 0xDCFCE7),
            warning: Color(hex: 0xEAB308),
            warningSubtle: Color(hex: 0xFEF9C3)
        ),
        typography: TypographyTokens(
            fontDesign: .default,
            h1Size: 30, h1Weight: .light,
            h2Size: 22, h2Weight: .regular,
            h3Size: 17, h3Weight: .medium,
            h4Size: 14, h4Weight: .medium,
            bodyLargeSize: 15, bodyLargeWeight: .regular,
            bodySize: 14, bodyWeight: .regular,
            captionSize: 12, captionWeight: .regular,
            bodyLineSpacing: 6, captionLineSpacing: 3
        ),
        spacing: SpacingTokens(xxs: 2, xs: 6, sm: 10, md: 16, lg: 20, xl: 30, xxl: 40, xxxl: 56),
        radii: RadiiTokens(sm: 8, md: 12, lg: 16, xl: 24, full: 999),
        sizing: SizingTokens(
            buttonHeightSm: 32, buttonHeightMd: 40, buttonHeightLg: 48,
            sliderTrackHeight: 6, sliderThumbSize: 20,
            iconSm: 16, iconMd: 22, iconLg: 26,
            toggleTrackWidth: 42, toggleTrackHeight: 24, toggleThumbSize: 20,
            checkboxSize: 18
        )
    )

    // MARK: - Forest
    // Deep green with emerald accents. Organic, calm, grounded feel.

    private static let forestTokens = DesignTokens(
        name: "Forest",
        description: "Organic, calm, and grounded",
        colors: ColorTokens(
            background: Color(hex: 0x0C1510),
            backgroundSecondary: Color(hex: 0x121F18),
            backgroundTertiary: Color(hex: 0x1A2E22),
            surface: Color(hex: 0x1A2E22),
            surfaceHover: Color(hex: 0x223B2C),
            surfacePressed: Color(hex: 0x2A4836),
            border: Color(hex: 0x2D4A38),
            borderSubtle: Color(hex: 0x1A2E22),
            textPrimary: Color(hex: 0xD1E7DD),
            textSecondary: Color(hex: 0x8BAA9A),
            textTertiary: Color(hex: 0x5A7A6A),
            textOnAccent: Color(hex: 0x0C1510),
            accent: Color(hex: 0x34D399),
            accentHover: Color(hex: 0x6EE7B7),
            accentSubtle: Color(hex: 0x0A2618),
            destructive: Color(hex: 0xFB7185),
            destructiveSubtle: Color(hex: 0x3D1520),
            success: Color(hex: 0x4ADE80),
            successSubtle: Color(hex: 0x0A2618),
            warning: Color(hex: 0xFBBF24),
            warningSubtle: Color(hex: 0x2D2A0A)
        ),
        typography: TypographyTokens(
            fontDesign: .rounded,
            h1Size: 30, h1Weight: .medium,
            h2Size: 22, h2Weight: .medium,
            h3Size: 17, h3Weight: .medium,
            h4Size: 13, h4Weight: .medium,
            bodyLargeSize: 15, bodyLargeWeight: .regular,
            bodySize: 13, bodyWeight: .regular,
            captionSize: 11, captionWeight: .regular,
            bodyLineSpacing: 5, captionLineSpacing: 3
        ),
        spacing: SpacingTokens(xxs: 2, xs: 4, sm: 8, md: 12, lg: 16, xl: 24, xxl: 32, xxxl: 48),
        radii: RadiiTokens(sm: 6, md: 10, lg: 14, xl: 20, full: 999),
        sizing: SizingTokens(
            buttonHeightSm: 28, buttonHeightMd: 36, buttonHeightLg: 44,
            sliderTrackHeight: 4, sliderThumbSize: 16,
            iconSm: 14, iconMd: 20, iconLg: 24,
            toggleTrackWidth: 38, toggleTrackHeight: 22, toggleThumbSize: 18,
            checkboxSize: 16
        )
    )
}

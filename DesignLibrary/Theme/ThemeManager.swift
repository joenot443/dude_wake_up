import SwiftUI

@Observable
class ThemeManager {
    var preset: ThemePreset = .default_
    var colorScheme: ColorScheme = .dark

    var tokens: DesignTokens { preset.tokens }

    var colors: ColorTokens {
        if colorScheme == .light, let light = preset.lightColorOverrides {
            return light
        }
        return tokens.colors
    }

    var typography: TypographyTokens { tokens.typography }
    var spacing: SpacingTokens { tokens.spacing }
    var radii: RadiiTokens { tokens.radii }
    var sizing: SizingTokens { tokens.sizing }
}

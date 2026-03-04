import SwiftUI

enum DSTextStyle {
    case h1, h2, h3, h4, bodyLarge, body, caption
}

struct DSText: View {
    @Environment(ThemeManager.self) var theme

    let text: String
    let style: DSTextStyle
    let color: Color?

    init(_ text: String, style: DSTextStyle, color: Color? = nil) {
        self.text = text
        self.style = style
        self.color = color
    }

    var body: some View {
        Text(text)
            .font(font)
            .foregroundStyle(color ?? defaultColor)
    }

    private var font: Font {
        switch style {
        case .h1: return theme.typography.h1
        case .h2: return theme.typography.h2
        case .h3: return theme.typography.h3
        case .h4: return theme.typography.h4
        case .bodyLarge: return theme.typography.bodyLarge
        case .body: return theme.typography.body
        case .caption: return theme.typography.caption
        }
    }

    private var defaultColor: Color {
        switch style {
        case .caption:
            return theme.colors.textSecondary
        default:
            return theme.colors.textPrimary
        }
    }
}

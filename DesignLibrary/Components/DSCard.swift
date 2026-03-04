import SwiftUI

struct DSCard<Content: View>: View {
    @Environment(ThemeManager.self) var theme
    let content: () -> Content

    init(@ViewBuilder content: @escaping () -> Content) {
        self.content = content
    }

    var body: some View {
        content()
            .padding(theme.spacing.lg)
            .frame(maxWidth: .infinity, alignment: .leading)
            .background(
                RoundedRectangle(cornerRadius: theme.radii.lg)
                    .fill(theme.colors.surface)
            )
            .overlay(
                RoundedRectangle(cornerRadius: theme.radii.lg)
                    .strokeBorder(theme.colors.borderSubtle, lineWidth: 1)
            )
    }
}

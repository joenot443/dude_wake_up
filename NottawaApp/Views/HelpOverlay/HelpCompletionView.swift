//
//  HelpCompletionView.swift
//  NottawaApp
//
//  Completion popup shown at the end of the guided help tutorial.
//

import SwiftUI

struct HelpCompletionView: View {
    @Environment(ThemeManager.self) private var theme
    @Environment(HelpGuideViewModel.self) private var helpGuide

    var body: some View {
        VStack(spacing: 0) {
            // Title
            DSText(HelpStrings.completionTitle, style: .h1)
                .padding(.bottom, 16)

            // Main body paragraphs
            VStack(alignment: .leading, spacing: 12) {
                DSText(HelpStrings.completionDescription, style: .bodyLarge, color: theme.colors.textSecondary)
                DSText(HelpStrings.completionDescription2, style: .bodyLarge, color: theme.colors.textSecondary)
                DSText(HelpStrings.completionDescription3, style: .body, color: theme.colors.textTertiary)
            }
            .padding(.bottom, 20)

            // Sign-off with photo
            HStack(spacing: 12) {
                Image("JoePhoto")
                    .resizable()
                    .aspectRatio(contentMode: .fill)
                    .frame(width: 44, height: 44)
                    .clipShape(Circle())
                    .overlay(
                        Circle()
                            .strokeBorder(theme.colors.accent.opacity(0.4), lineWidth: 1.5)
                    )

                DSText(HelpStrings.completionDescription4, style: .h3)
            }
            .frame(maxWidth: .infinity, alignment: .leading)
            .padding(.bottom, 20)

            // Divider
            Rectangle()
                .fill(theme.colors.border.opacity(0.3))
                .frame(height: 1)
                .padding(.bottom, 16)

            // CTA button
            DSButton("Let me in!", variant: .primary, size: .lg) {
                helpGuide.completed = true
            }
        }
        .padding(28)
        .frame(maxWidth: 440)
        .background(
            RoundedRectangle(cornerRadius: theme.radii.lg)
                .fill(theme.colors.surface.opacity(0.95))
        )
        .overlay(
            RoundedRectangle(cornerRadius: theme.radii.lg)
                .strokeBorder(theme.colors.accent.opacity(0.3), lineWidth: 1)
        )
        .shadow(color: .black.opacity(0.4), radius: 20, y: 8)
    }
}

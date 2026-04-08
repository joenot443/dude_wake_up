//
//  HelpStepCardView.swift
//  NottawaApp
//
//  Reusable card for help tutorial steps.
//

import SwiftUI

struct HelpStepCardView: View {
    @Environment(ThemeManager.self) private var theme
    let title: String
    var bodyText: String? = nil
    var secondaryText: String? = nil
    var buttonTitle: String? = nil
    var onButton: (() -> Void)? = nil

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            DSText(title, style: .h3)

            if let bodyText {
                DSText(bodyText, style: .body, color: theme.colors.textSecondary)
            }

            if let secondaryText {
                DSText(secondaryText, style: .caption, color: theme.colors.textTertiary)
            }

            if let buttonTitle, let onButton {
                DSButton(buttonTitle, variant: .primary, size: .sm) {
                    onButton()
                }
                .padding(.top, 4)
            }
        }
        .padding(16)
        .frame(maxWidth: 320, alignment: .leading)
        .background(
            RoundedRectangle(cornerRadius: theme.radii.lg)
                .fill(theme.colors.surface.opacity(0.92))
        )
        .overlay(
            RoundedRectangle(cornerRadius: theme.radii.lg)
                .strokeBorder(theme.colors.accent.opacity(0.3), lineWidth: 1)
        )
        .shadow(color: .black.opacity(0.3), radius: 12, y: 4)
    }
}

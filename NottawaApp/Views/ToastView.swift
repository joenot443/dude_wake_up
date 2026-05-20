//
//  ToastView.swift
//  NottawaApp
//
//  Bottom-center transient notification observing `viewModel.toastMessage`.
//  Auto-dismiss is handled by the viewmodel; this view just animates in/out.
//

import SwiftUI

struct ToastView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel
    @Environment(ThemeManager.self) private var theme

    var body: some View {
        VStack {
            Spacer()
            if let message = viewModel.toastMessage {
                Text(message)
                    .font(.callout)
                    .foregroundStyle(theme.colors.textPrimary)
                    .padding(.horizontal, 16)
                    .padding(.vertical, 10)
                    .background(theme.colors.backgroundSecondary, in: Capsule())
                    .overlay(
                        Capsule().stroke(theme.colors.border, lineWidth: 1)
                    )
                    .shadow(color: .black.opacity(0.25), radius: 12, y: 4)
                    .padding(.bottom, 32)
                    .transition(.move(edge: .bottom).combined(with: .opacity))
                    .allowsHitTesting(false)
                    .accessibilityIdentifier("toast-message")
            }
        }
        .animation(.spring(response: 0.35, dampingFraction: 0.8), value: viewModel.toastMessage)
        .allowsHitTesting(false)
    }
}

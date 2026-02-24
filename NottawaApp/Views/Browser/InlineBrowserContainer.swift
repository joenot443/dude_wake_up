//
//  InlineBrowserContainer.swift
//  NottawaApp
//
//  Container that presents a browser view inline within the node editor canvas.
//

import SwiftUI

struct InlineBrowserContainer<Content: View>: View {
    @ViewBuilder let content: Content

    var body: some View {
        ZStack {
            // Dimmed backdrop
            Color.black.opacity(0.4)
                .ignoresSafeArea()

            // Browser content
            content
                .background(.ultraThickMaterial, in: RoundedRectangle(cornerRadius: 12))
                .clipShape(RoundedRectangle(cornerRadius: 12))
                .shadow(color: .black.opacity(0.5), radius: 20, y: 8)
                .padding(40)
        }
    }
}

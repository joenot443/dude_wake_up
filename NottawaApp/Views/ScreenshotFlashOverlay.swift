//
//  ScreenshotFlashOverlay.swift
//  NottawaApp
//
//  White-fade overlay shown for ~150ms after a successful capture.
//

import SwiftUI

struct ScreenshotFlashOverlay: View {
    @State private var screenshotService = ScreenshotService.shared
    @State private var opacity: Double = 0

    var body: some View {
        Rectangle()
            .fill(Color.white)
            .opacity(opacity)
            .allowsHitTesting(false)
            .onChange(of: screenshotService.flashTick) { _, _ in
                opacity = 0.6
                withAnimation(.easeOut(duration: 0.25)) {
                    opacity = 0
                }
            }
    }
}

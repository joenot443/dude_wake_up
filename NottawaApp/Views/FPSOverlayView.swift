//
//  FPSOverlayView.swift
//  NottawaApp
//
//  Shared FPS overlay showing engine and UI frame rates.
//

import SwiftUI

struct FPSOverlayView: View {
    let engineFPS: Int
    let uiFPS: Int

    var body: some View {
        VStack(alignment: .leading, spacing: 2) {
            Text("Engine \(engineFPS) FPS")
            Text("UI \(uiFPS) FPS")
        }
        .font(.system(.caption, design: .monospaced))
        .padding(.horizontal, 8)
        .padding(.vertical, 4)
        .background(.ultraThinMaterial, in: RoundedRectangle(cornerRadius: 6))
    }
}

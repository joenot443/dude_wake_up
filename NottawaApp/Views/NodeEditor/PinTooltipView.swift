//
//  PinTooltipView.swift
//  NottawaApp
//
//  Tooltip popover shown when hovering over a connected pin.
//

import SwiftUI

struct PinTooltipView: View {
    let connectedNodeName: String
    let connectionId: String
    let onBreak: () -> Void

    var body: some View {
        HStack(spacing: 8) {
            Text(connectedNodeName)
                .font(.caption)
                .fontWeight(.medium)

            Button(action: onBreak) {
                Image(systemName: "scissors")
                    .font(.caption)
                    .foregroundStyle(.red)
            }
            .buttonStyle(.plain)
            .help("Break connection")
        }
        .padding(.horizontal, 10)
        .padding(.vertical, 6)
    }
}

//
//  HelpArrowView.swift
//  NottawaApp
//
//  Animated directional arrow for help overlay.
//

import SwiftUI

enum HelpArrowDirection {
    case left, right, up, down
}

struct HelpArrowView: View {
    @Environment(ThemeManager.self) private var theme
    let direction: HelpArrowDirection

    var body: some View {
        TimelineView(.animation(minimumInterval: 1.0 / 30.0, paused: false)) { timeline in
            let time = timeline.date.timeIntervalSince1970
            let offset = sin(time * 5.0) * 5.0

            Image(systemName: symbolName)
                .font(.system(size: 28, weight: .bold))
                .foregroundStyle(theme.colors.accent)
                .offset(x: horizontalOffset(offset), y: verticalOffset(offset))
        }
    }

    private var symbolName: String {
        switch direction {
        case .left: return "arrow.left"
        case .right: return "arrow.right"
        case .up: return "arrow.up"
        case .down: return "arrow.down"
        }
    }

    private func horizontalOffset(_ offset: Double) -> CGFloat {
        switch direction {
        case .left: return -offset
        case .right: return offset
        case .up, .down: return 0
        }
    }

    private func verticalOffset(_ offset: Double) -> CGFloat {
        switch direction {
        case .up: return -offset
        case .down: return offset
        case .left, .right: return 0
        }
    }
}

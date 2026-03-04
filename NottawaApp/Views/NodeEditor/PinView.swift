//
//  PinView.swift
//  NottawaApp
//
//  A connection pin (input or output) on a node.
//

import SwiftUI

struct PinView: View {
    let nodeId: String
    let slotIndex: Int
    let isOutput: Bool
    let isConnected: Bool
    var scale: CGFloat = 1.0
    /// Offset applied by the parent (e.g. edge-mounting). Used to correct
    /// the GeometryReader-reported position since SwiftUI `.offset()` is visual-only.
    var positionOffset: CGFloat = 0
    @Environment(NodeEditorViewModel.self) private var viewModel
    @Environment(ThemeManager.self) private var theme

    private var pinSize: CGFloat { 12 * scale }
    private var ringSize: CGFloat { 16 * scale }
    private var dotSize: CGFloat { 6 * scale }

    @State private var isPulsing = false

    private var pinKey: PinKey {
        PinKey(nodeId: nodeId, slotIndex: slotIndex, isOutput: isOutput)
    }

    /// This pin is the magnetically snapped target.
    private var isSnapped: Bool {
        viewModel.dragSnapPinKey == pinKey
    }

    /// This pin is a valid drop target for the current drag.
    private var isValidTarget: Bool {
        viewModel.dragValidPinKeys.contains(pinKey)
    }

    /// A drag is in progress (some pin is being dragged).
    private var isDragging: Bool {
        viewModel.dragConnection != nil
    }

    private var shouldPulse: Bool {
        isValidTarget && isDragging && !isSnapped
    }

    private var pulseScale: CGFloat {
        isPulsing ? 1.12 : 1.0
    }

    var body: some View {
        ZStack {
            // Snap glow background
            if isSnapped {
                Circle()
                    .fill(theme.colors.success.opacity(0.2))
                    .frame(width: ringSize + 6 * scale, height: ringSize + 6 * scale)
            }

            // Outer ring (always visible)
            Circle()
                .strokeBorder(ringColor, lineWidth: ringLineWidth)
                .frame(width: ringSize, height: ringSize)
                .scaleEffect(pulseScale)

            // Inner dot (connected/snapped only)
            if isConnected || isSnapped {
                Circle()
                    .fill(dotColor)
                    .frame(width: dotSize, height: dotSize)
                    .shadow(color: dotColor.opacity(0.5), radius: 2 * scale)
                    .transition(.scale.combined(with: .opacity))
            }
        }
        // Dim pins that are not valid targets during a drag
        .opacity(isDragging && !isValidTarget && !isSnapped ? 0.3 : 1.0)
        .background(
            GeometryReader { geo in
                Color.clear
                    .preference(
                        key: PinPositionPreferenceKey.self,
                        value: [pinKey:
                                CGPoint(x: geo.frame(in: .named("nodeEditor")).midX + positionOffset,
                                        y: geo.frame(in: .named("nodeEditor")).midY)]
                    )
            }
        )
        .animation(.easeInOut(duration: 0.15), value: isSnapped)
        .animation(.easeInOut(duration: 0.15), value: isValidTarget)
        .animation(.easeInOut(duration: 0.15), value: isDragging)
        .animation(.easeInOut(duration: 0.15), value: isConnected)
        .onChange(of: shouldPulse) { _, newValue in
            if newValue {
                withAnimation(.easeInOut(duration: 0.8).repeatForever(autoreverses: true)) {
                    isPulsing = true
                }
            } else {
                withAnimation(.easeInOut(duration: 0.15)) {
                    isPulsing = false
                }
            }
        }
    }

    private var ringColor: Color {
        if isSnapped {
            return theme.colors.success.opacity(0.6)
        } else if isConnected || (isValidTarget && isDragging) {
            return theme.colors.accent.opacity(0.5)
        } else {
            return theme.colors.textTertiary.opacity(0.7)
        }
    }

    private var ringLineWidth: CGFloat {
        if isSnapped || (isValidTarget && isDragging) {
            return 1.5 * scale
        } else {
            return 1 * scale
        }
    }

    private var dotColor: Color {
        if isSnapped {
            return theme.colors.success
        } else {
            return theme.colors.accent
        }
    }
}

// MARK: - Preference Key for reporting pin positions

struct PinPositionPreferenceKey: PreferenceKey {
    static var defaultValue: [PinKey: CGPoint] = [:]

    static func reduce(value: inout [PinKey: CGPoint], nextValue: () -> [PinKey: CGPoint]) {
        value.merge(nextValue(), uniquingKeysWith: { $1 })
    }
}

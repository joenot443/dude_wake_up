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
    @Environment(NodeEditorViewModel.self) private var viewModel

    private var pinSize: CGFloat { 16 * scale }

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

    private var fillColor: Color {
        if isSnapped {
            return .green
        } else if isValidTarget {
            return .accentColor
        } else if isConnected {
            return .accentColor
        } else {
            return Color(.systemGray)
        }
    }

    var body: some View {
        ZStack {
            // Glow ring for snapped pin
            if isSnapped {
                Circle()
                    .fill(Color.green.opacity(0.3))
                    .frame(width: pinSize + 12 * scale, height: pinSize + 12 * scale)

                Circle()
                    .strokeBorder(Color.green.opacity(0.6), lineWidth: 2 * scale)
                    .frame(width: pinSize + 12 * scale, height: pinSize + 12 * scale)
            }

            // Pulse ring for valid targets (not snapped)
            if isValidTarget && !isSnapped && isDragging {
                Circle()
                    .strokeBorder(Color.accentColor.opacity(0.5), lineWidth: 1.5 * scale)
                    .frame(width: pinSize + 8 * scale, height: pinSize + 8 * scale)
            }

            // Main pin circle
            Circle()
                .fill(fillColor)
                .frame(width: pinSize, height: pinSize)
                .overlay(
                    Circle()
                        .strokeBorder(Color.white.opacity(0.3), lineWidth: 1)
                )
                // Dim pins that are not valid targets during a drag
                .opacity(isDragging && !isValidTarget && !isSnapped ? 0.3 : 1.0)
        }
        .background(
            GeometryReader { geo in
                Color.clear
                    .preference(
                        key: PinPositionPreferenceKey.self,
                        value: [pinKey:
                                CGPoint(x: geo.frame(in: .named("nodeEditor")).midX,
                                        y: geo.frame(in: .named("nodeEditor")).midY)]
                    )
            }
        )
        .animation(.easeInOut(duration: 0.15), value: isSnapped)
        .animation(.easeInOut(duration: 0.15), value: isValidTarget)
        .animation(.easeInOut(duration: 0.15), value: isDragging)
    }
}

// MARK: - Preference Key for reporting pin positions

struct PinPositionPreferenceKey: PreferenceKey {
    static var defaultValue: [PinKey: CGPoint] = [:]

    static func reduce(value: inout [PinKey: CGPoint], nextValue: () -> [PinKey: CGPoint]) {
        value.merge(nextValue(), uniquingKeysWith: { $1 })
    }
}

//
//  PinStripView.swift
//  NottawaApp
//
//  A single pill-shaped strip that contains all input or output pins for a node.
//  Replaces the per-slot PinView stack with one cohesive visual element.
//

import SwiftUI

struct PinStripView: View {
    let nodeId: String
    let isOutput: Bool
    let slots: [PinSlot]
    var scale: CGFloat = 1.0
    /// Horizontal offset applied to correct GeometryReader positions
    /// (since SwiftUI .offset() is visual-only).
    var positionOffset: CGFloat = 0

    @Environment(NodeEditorViewModel.self) private var viewModel
    @Environment(ThemeManager.self) private var theme

    // Dimensions
    private var stripWidth: CGFloat { 14 * scale }
    private var dotSize: CGFloat { 6 * scale }
    private var dotSpacing: CGFloat { 12 * scale }
    private var cornerRadius: CGFloat { 7 * scale }
    private var verticalPadding: CGFloat { 8 * scale }
    private var stripHeight: CGFloat {
        let count = CGFloat(slots.count)
        return max(stripWidth, count * dotSize + (count + 1) * dotSpacing * 0.5 + verticalPadding * 2)
    }

    private var isDragging: Bool {
        viewModel.dragConnection != nil
    }

    /// True if any slot in this strip is a valid drag target.
    private var hasValidTarget: Bool {
        slots.contains { viewModel.dragValidPinKeys.contains(pinKey(for: $0)) }
    }

    /// True if any slot in this strip is the snapped target.
    private var hasSnappedSlot: Bool {
        guard let snap = viewModel.dragSnapPinKey else { return false }
        return slots.contains { pinKey(for: $0) == snap }
    }

    var body: some View {
        ZStack {
            // Pill background
            Capsule()
                .fill(stripFill)
                .frame(width: stripWidth, height: stripHeight)

            // Pill border
            Capsule()
                .strokeBorder(stripBorder, lineWidth: borderWidth)
                .frame(width: stripWidth, height: stripHeight)

            // Slot dots inside the pill
            VStack(spacing: dotSpacing * 0.5) {
                Spacer().frame(height: verticalPadding)
                ForEach(slots) { slot in
                    PinDotView(
                        slot: slot,
                        nodeId: nodeId,
                        isOutput: isOutput,
                        scale: scale,
                        positionOffset: positionOffset
                    )
                }
                Spacer().frame(height: verticalPadding)
            }
            .frame(height: stripHeight)
        }
        .frame(width: stripWidth, height: stripHeight)
        // Dim the entire strip when dragging and no slot is valid
        .opacity(isDragging && !hasValidTarget && !hasSnappedSlot ? 0.3 : 1.0)
        .animation(.easeInOut(duration: 0.15), value: isDragging)
        .animation(.easeInOut(duration: 0.15), value: hasValidTarget)
    }

    // MARK: - Styling

    private var stripFill: Color {
        if hasSnappedSlot {
            return theme.colors.success.opacity(0.12)
        }
        return theme.colors.background.opacity(0.6)
    }

    private var stripBorder: Color {
        if hasSnappedSlot {
            return theme.colors.success.opacity(0.5)
        } else if hasValidTarget && isDragging {
            return theme.colors.accent.opacity(0.5)
        }
        return theme.colors.textTertiary.opacity(0.3)
    }

    private var borderWidth: CGFloat {
        (hasSnappedSlot || (hasValidTarget && isDragging)) ? 1.5 * scale : 1 * scale
    }

    private func pinKey(for slot: PinSlot) -> PinKey {
        PinKey(nodeId: nodeId, slotIndex: slot.slotIndex, isOutput: isOutput)
    }
}

// MARK: - Pin Slot Model

struct PinSlot: Identifiable {
    let slotIndex: Int
    let isConnected: Bool
    var label: String? = nil

    var id: Int { slotIndex }
}

// MARK: - Individual Dot Inside Strip

/// A single dot inside the pill strip. Reports its position for cable drawing
/// and shows connected/snap/valid-target visual states.
private struct PinDotView: View {
    let slot: PinSlot
    let nodeId: String
    let isOutput: Bool
    var scale: CGFloat = 1.0
    var positionOffset: CGFloat = 0

    @Environment(NodeEditorViewModel.self) private var viewModel
    @Environment(ThemeManager.self) private var theme

    @State private var isPulsing = false

    private var dotSize: CGFloat { 6 * scale }
    private var hitSize: CGFloat { 14 * scale }

    private var pinKey: PinKey {
        PinKey(nodeId: nodeId, slotIndex: slot.slotIndex, isOutput: isOutput)
    }

    private var isSnapped: Bool { viewModel.dragSnapPinKey == pinKey }
    private var isValidTarget: Bool { viewModel.dragValidPinKeys.contains(pinKey) }
    private var isDragging: Bool { viewModel.dragConnection != nil }
    private var shouldPulse: Bool { isValidTarget && isDragging && !isSnapped }

    var body: some View {
        ZStack {
            // Snap glow
            if isSnapped {
                Circle()
                    .fill(theme.colors.success.opacity(0.3))
                    .frame(width: dotSize + 6 * scale, height: dotSize + 6 * scale)
            }

            if slot.isConnected || isSnapped {
                // Filled dot
                Circle()
                    .fill(dotColor)
                    .frame(width: dotSize, height: dotSize)
                    .shadow(color: dotColor.opacity(0.5), radius: 2 * scale)
                    .scaleEffect(isPulsing ? 1.15 : 1.0)
            } else {
                // Hollow notch
                Circle()
                    .strokeBorder(notchColor, lineWidth: 1 * scale)
                    .frame(width: dotSize, height: dotSize)
                    .scaleEffect(isPulsing ? 1.15 : 1.0)
            }

            // Label (e.g. "A" for aux output)
            if let label = slot.label {
                Text(label)
                    .font(.system(size: 5 * scale, weight: .bold))
                    .foregroundStyle(theme.colors.textOnAccent)
                    .allowsHitTesting(false)
            }
        }
        .frame(width: hitSize, height: hitSize)
        // Report position for cable drawing and hit testing
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
        .animation(.easeInOut(duration: 0.15), value: slot.isConnected)
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

    private var dotColor: Color {
        isSnapped ? theme.colors.success : theme.colors.accent
    }

    private var notchColor: Color {
        if isValidTarget && isDragging {
            return theme.colors.accent.opacity(0.6)
        }
        return theme.colors.textTertiary.opacity(0.5)
    }
}

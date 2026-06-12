//
//  HelpOverlayView.swift
//  NottawaApp
//
//  Main help overlay for non-node-attached steps (sidebar, inspector,
//  centered cards, action buttons tour, share help, completion).
//  Node-attached tooltips are rendered by NodeView directly.
//

import SwiftUI

struct HelpOverlayView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel
    @Environment(HelpGuideViewModel.self) private var helpGuide
    @Environment(ThemeManager.self) private var theme

    // Measured size of the inspector-pointing help card, so it can be centred on
    // the target control while staying pinned to the inspector's leading edge.
    @State private var targetCardSize: CGSize = .zero

    var body: some View {
        GeometryReader { geo in
            let step = helpGuide.currentStep

            ZStack {
                switch step {
                // MARK: - Sidebar-pointing steps
                case .placeSource:
                    sidebarPointingCard(
                        in: geo,
                        title: HelpStrings.welcomeTitle,
                        bodyText: HelpStrings.welcomeSubtitle + "\n\n" + HelpStrings.startGuide + "\n\n" + HelpStrings.sourceDragInstruction,
                        arrow: .left
                    )

                case .placeEffect:
                    sidebarPointingCard(
                        in: geo,
                        title: HelpStrings.addEffectTitle,
                        bodyText: HelpStrings.effectDragInstruction,
                        arrow: .left
                    )

                case .placeSecondSource:
                    sidebarPointingCard(
                        in: geo,
                        title: HelpStrings.addSecondSourceTitle,
                        bodyText: HelpStrings.addSecondSourceInstruction + "\n\n" + HelpStrings.webcamVideoInstruction,
                        arrow: .left
                    )

                // MARK: - Inspector-pointing steps
                case .editBlendMode:
                    inspectorTargetPointingCard(
                        in: geo,
                        title: HelpStrings.changeParametersTitle,
                        bodyText: HelpStrings.blendModeInstruction
                    )

                case .editAlpha:
                    inspectorTargetPointingCard(
                        in: geo,
                        title: HelpStrings.changeParametersTitle,
                        bodyText: HelpStrings.alphaModeInstruction + "\n\n" + HelpStrings.otherNodesInstruction
                    )

                // MARK: - Button-advancement steps
                case .actionButtonsTour:
                    actionButtonsCard(in: geo)

                case .shareHelp:
                    shareHelpCard(in: geo)

                case .completion:
                    HelpCompletionView()
                        .position(x: geo.size.width / 2, y: geo.size.height / 2)

                // Node-attached steps are rendered by NodeView — nothing here
                case .connectSourceToEffect,
                     .blendViaOutputs,
                     .openInspector:
                    EmptyView()
                }
            }
        }
        .allowsHitTesting(allowsHitTesting)
        .animation(.easeInOut(duration: 0.3), value: helpGuide.currentStep)
    }

    // Only allow hit testing on steps with buttons
    private var allowsHitTesting: Bool {
        switch helpGuide.currentStep {
        case .actionButtonsTour, .shareHelp, .completion:
            return true
        default:
            return false
        }
    }

    // MARK: - Card Layouts

    /// Card with arrow pointing at the sidebar (left side)
    private func sidebarPointingCard(in geo: GeometryProxy, title: String, bodyText: String, arrow: HelpArrowDirection) -> some View {
        HStack(spacing: 8) {
            HelpArrowView(direction: arrow)
            HelpStepCardView(title: title, bodyText: bodyText)
        }
        .position(x: 200, y: geo.size.height / 2)
    }

    /// Card whose arrow points directly at the inspector control the current
    /// step targets (the Mode selector, the Main Input Strength slider, …). The
    /// control's global frame is captured by the inspector
    /// (`HelpGuideViewModel.inspectorTargetFrame`); here we convert it into the
    /// overlay's local space and vertically centre the card+arrow on it, while
    /// keeping the card pinned just left of the inspector panel. Falls back to a
    /// third of the way down until the frame is measured.
    private func inspectorTargetPointingCard(in geo: GeometryProxy, title: String, bodyText: String) -> some View {
        let overlayGlobal = geo.frame(in: .global)
        let target = helpGuide.inspectorTargetFrame

        let rawY = target == .zero
            ? geo.size.height / 3
            : target.midY - overlayGlobal.minY

        let halfH = targetCardSize.height / 2
        let y = min(max(rawY, halfH + 8), geo.size.height - halfH - 8)
        // Pin the card's trailing edge ~8pt from the inspector's left edge.
        let x = targetCardSize == .zero
            ? geo.size.width - 8
            : geo.size.width - targetCardSize.width / 2 - 8

        return HStack(spacing: 8) {
            HelpStepCardView(title: title, bodyText: bodyText)
            HelpArrowView(direction: .right)
        }
        .background(
            GeometryReader { proxy in
                Color.clear.preference(key: HelpCardSizeKey.self, value: proxy.size)
            }
        )
        .onPreferenceChange(HelpCardSizeKey.self) { targetCardSize = $0 }
        .position(x: x, y: y)
    }

    /// Action buttons tour card
    private func actionButtonsCard(in geo: GeometryProxy) -> some View {
        VStack(alignment: .leading, spacing: 12) {
            DSText(HelpStrings.actionButtonsTitle, style: .h3)

            actionRow(icon: "arrow.uturn.backward", text: HelpStrings.undoInstruction)
            actionRow(icon: "arrow.uturn.forward", text: HelpStrings.redoInstruction)
            actionRow(icon: "camera", text: HelpStrings.cameraViewInstruction)
            actionRow(icon: "questionmark.circle", text: HelpStrings.helpToggleInstruction)
            actionRow(icon: "trash.slash", text: HelpStrings.clearCanvasInstruction)
            actionRow(icon: "arrow.down.right.and.arrow.up.left", text: HelpStrings.scaleToFitInstruction)

            DSButton("Okay!", variant: .primary, size: .sm) {
                helpGuide.hasSeenActionButtons = true
            }
            .frame(maxWidth: .infinity, alignment: .center)
            .padding(.top, 4)
        }
        .padding(16)
        .frame(maxWidth: 360, alignment: .leading)
        .background(
            RoundedRectangle(cornerRadius: theme.radii.lg)
                .fill(theme.colors.surface.opacity(0.92))
        )
        .overlay(
            RoundedRectangle(cornerRadius: theme.radii.lg)
                .strokeBorder(theme.colors.accent.opacity(0.3), lineWidth: 1)
        )
        .shadow(color: .black.opacity(0.3), radius: 12, y: 4)
        // Pin just above the action bar, which lives in the canvas's
        // bottom-trailing corner with a 16pt inset and stands ~40pt tall.
        .padding(.trailing, 16)
        .padding(.bottom, 64)
        .frame(maxWidth: .infinity, maxHeight: .infinity, alignment: .bottomTrailing)
    }

    /// Share help card
    private func shareHelpCard(in geo: GeometryProxy) -> some View {
        VStack(alignment: .leading, spacing: 10) {
            DSText(HelpStrings.shareStrandTitle, style: .h3)
            DSText(HelpStrings.shareStrandDescription, style: .body, color: theme.colors.textSecondary)

            VStack(alignment: .leading, spacing: 6) {
                bulletRow(HelpStrings.shareStrandStep1)
                bulletRow(HelpStrings.shareStrandStep2)
                bulletRow(HelpStrings.shareStrandStep3)
                bulletRow(HelpStrings.shareStrandStep4)
            }
            .padding(.top, 4)

            DSText(HelpStrings.shareStrandNote, style: .caption, color: theme.colors.textTertiary)
                .padding(.top, 2)

            DSButton("Got it!", variant: .primary, size: .sm) {
                helpGuide.hasSeenShareHelp = true
            }
            .padding(.top, 4)
        }
        .padding(16)
        .frame(maxWidth: 400, alignment: .leading)
        .background(
            RoundedRectangle(cornerRadius: theme.radii.lg)
                .fill(theme.colors.surface.opacity(0.92))
        )
        .overlay(
            RoundedRectangle(cornerRadius: theme.radii.lg)
                .strokeBorder(theme.colors.accent.opacity(0.3), lineWidth: 1)
        )
        .shadow(color: .black.opacity(0.3), radius: 12, y: 4)
        .position(x: geo.size.width / 2, y: geo.size.height / 2)
    }

    // MARK: - Helpers

    private func actionRow(icon: String, text: String) -> some View {
        HStack(spacing: 8) {
            Image(systemName: icon)
                .font(.system(size: 14, weight: .medium))
                .foregroundStyle(theme.colors.textSecondary)
                .frame(width: 20)
            DSText(text, style: .body, color: theme.colors.textSecondary)
        }
    }

    private func bulletRow(_ text: String) -> some View {
        HStack(alignment: .top, spacing: 6) {
            Text("\u{2022}")
                .foregroundStyle(theme.colors.textSecondary)
            DSText(text, style: .caption, color: theme.colors.textSecondary)
        }
    }
}

// MARK: - Preference Keys

/// Carries the global-coordinate frame of the current step's target inspector
/// control up to the help overlay. Only non-empty frames win the reduce so a
/// stray default doesn't clobber a real measurement.
struct InspectorTargetFrameKey: PreferenceKey {
    static var defaultValue: CGRect = .zero
    static func reduce(value: inout CGRect, nextValue: () -> CGRect) {
        let next = nextValue()
        if next != .zero { value = next }
    }
}

/// Measured size of a help card, used to position it precisely.
struct HelpCardSizeKey: PreferenceKey {
    static var defaultValue: CGSize = .zero
    static func reduce(value: inout CGSize, nextValue: () -> CGSize) {
        let next = nextValue()
        if next != .zero { value = next }
    }
}

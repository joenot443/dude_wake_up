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
                    inspectorPointingCard(
                        in: geo,
                        title: HelpStrings.changeParametersTitle,
                        bodyText: HelpStrings.blendModeInstruction
                    )

                case .editAlpha:
                    inspectorPointingCard(
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

    /// Card pointing at the inspector panel (right side)
    private func inspectorPointingCard(in geo: GeometryProxy, title: String, bodyText: String) -> some View {
        HStack(spacing: 8) {
            HelpStepCardView(title: title, bodyText: bodyText)
            HelpArrowView(direction: .right)
        }
        .position(x: geo.size.width - 250, y: geo.size.height / 3)
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
        .position(x: geo.size.width / 2, y: geo.size.height / 2)
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

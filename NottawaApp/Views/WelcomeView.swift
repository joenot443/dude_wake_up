//
//  WelcomeView.swift
//  NottawaApp
//
//  Welcome screen shown on launch with a header, action buttons, and
//  two-column strand browsers (Templates + Recent).
//

import SwiftUI

struct WelcomeView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel
    @Environment(ThemeManager.self) private var theme

    @State private var titleOpacity: Double = 0
    @State private var titleOffset: CGFloat = 8
    @State private var subtitleOpacity: Double = 0
    @State private var subtitleOffset: CGFloat = 6

    var body: some View {
        ZStack {
            // Dark scrim behind panel
            Color.black.opacity(0.5)
                .ignoresSafeArea()
                .onTapGesture {
                    viewModel.dismissWelcomeScreen()
                }
                .accessibilityIdentifier("welcome-scrim")

            GeometryReader { geo in
                let panelWidth = geo.size.width * 0.60
                let panelHeight = geo.size.height * 0.80

                ZStack(alignment: .bottomTrailing) {
                    VStack(spacing: 0) {
                        welcomeHeader(width: panelWidth)

                        actionButtons()

                        Spacer().frame(height: 24)

                        strandBrowsers()
                            .frame(maxHeight: .infinity)
                    }

                    DSToggle(
                        isOn: Bindable(viewModel).welcomeScreenEnabled,
                        label: "Always show at launch",
                        style: .checkbox,
                        size: .sm
                    )
                    .foregroundStyle(theme.colors.textTertiary)
                    .opacity(0.6)
                    .padding(.horizontal, 20)
                    .padding(.bottom, 14)
                }
                .frame(width: panelWidth, height: panelHeight)
                .background(theme.colors.backgroundSecondary)
                .clipShape(RoundedRectangle(cornerRadius: 12))
                .shadow(color: .black.opacity(0.3), radius: 20)
                .position(x: geo.size.width / 2, y: geo.size.height / 2)
            }
        }
        .onAppear {
            viewModel.fetchTemplateStrands()
            viewModel.fetchStrands()
        }
        .onExitCommand {
            viewModel.dismissWelcomeScreen()
        }
    }

    // MARK: - Welcome Header

    @ViewBuilder
    private func welcomeHeader(width: CGFloat) -> some View {
        VStack(alignment: .leading, spacing: 8) {
            Text("Welcome to Nottawa.")
                .font(.system(size: 38, weight: .semibold, design: .serif))
                .foregroundStyle(theme.colors.textPrimary)
                .opacity(titleOpacity)
                .offset(y: titleOffset)

            Text("Make something cool.")
                .font(.system(size: 17, weight: .regular, design: .serif))
                .italic()
                .foregroundStyle(theme.colors.textSecondary)
                .opacity(subtitleOpacity)
                .offset(y: subtitleOffset)
        }
        .padding(.horizontal, 28)
        .padding(.vertical, 32)
        .frame(width: width, alignment: .leading)
        .frame(minHeight: 160, alignment: .leading)
        .onAppear(perform: startIntroAnimation)
    }

    // MARK: - Intro Animation

    private func startIntroAnimation() {
        // Reset (in case view re-appears, e.g. via menu).
        titleOpacity = 0
        titleOffset = 8
        subtitleOpacity = 0
        subtitleOffset = 6

        withAnimation(.easeOut(duration: 0.55)) {
            titleOpacity = 1
            titleOffset = 0
        }
        withAnimation(.easeOut(duration: 0.55).delay(0.2)) {
            subtitleOpacity = 1
            subtitleOffset = 0
        }
    }

    // MARK: - Action Buttons

    @ViewBuilder
    private func actionButtons() -> some View {
        // Primary = "what does the user most likely want to do right now?"
        // If the engine has restored a graph from the last session, Resume.
        // Otherwise (truly fresh / first launch), New.
        let hasResumableSession = !viewModel.nodes.isEmpty

        return HStack(spacing: 14) {
            WelcomeActionCard(
                icon: "arrow.uturn.backward",
                title: "Resume",
                subtitle: "Continue where you left off",
                isPrimary: hasResumableSession,
                isEnabled: hasResumableSession,
                action: { viewModel.resumePreviousWorkspace() }
            )

            WelcomeActionCard(
                icon: "plus.square.dashed",
                title: "New Workspace",
                subtitle: "Start with a blank canvas",
                isPrimary: !hasResumableSession,
                action: {
                    viewModel.newWorkspace()
                    viewModel.dismissWelcomeScreen()
                }
            )

            WelcomeActionCard(
                icon: "folder",
                title: "Open Workspace",
                subtitle: "Pick a saved file",
                action: openWorkspacePanel
            )
        }
        .padding(.horizontal, 28)
    }

    // MARK: - Strand Browsers

    @ViewBuilder
    private func strandBrowsers() -> some View {
        HStack(alignment: .top, spacing: 16) {
            // Templates column
            VStack(alignment: .leading, spacing: 8) {
                Text("Templates")
                    .font(.headline)
                    .padding(.horizontal, 8)

                ScrollView {
                    LazyVGrid(
                        columns: [GridItem(.adaptive(minimum: 120), spacing: 8)],
                        spacing: 8
                    ) {
                        ForEach(viewModel.templateStrandsList) { strand in
                            StrandTileView(strand: strand)
                                .onTapGesture {
                                    viewModel.loadStrand(id: strand.id)
                                    viewModel.dismissWelcomeScreen()
                                }
                        }
                    }
                    .padding(.horizontal, 8)
                }
            }
            .frame(maxWidth: .infinity, maxHeight: .infinity)

            theme.colors.border.frame(width: 1)

            // Recent column
            VStack(alignment: .leading, spacing: 8) {
                Text("Recent")
                    .font(.headline)
                    .padding(.horizontal, 8)

                ScrollView {
                    LazyVGrid(
                        columns: [GridItem(.adaptive(minimum: 120), spacing: 8)],
                        spacing: 8
                    ) {
                        ForEach(viewModel.strandsList) { strand in
                            StrandTileView(strand: strand)
                                .onTapGesture {
                                    viewModel.loadStrand(id: strand.id)
                                    viewModel.dismissWelcomeScreen()
                                }
                        }
                    }
                    .padding(.horizontal, 8)
                }

                if viewModel.strandsList.isEmpty {
                    ContentUnavailableView {
                        Label("No Recent Strands", systemImage: "rectangle.stack")
                    } description: {
                        Text("Saved strands will appear here.")
                    }
                    .frame(maxHeight: .infinity)
                }
            }
            .frame(maxWidth: .infinity, maxHeight: .infinity)
        }
        .padding(.horizontal, 16)
        .padding(.bottom, 16)
    }

    // MARK: - Helpers

    private func openWorkspacePanel() {
        let panel = NSOpenPanel()
        panel.allowedContentTypes = [.json]
        panel.allowsMultipleSelection = false
        panel.begin { response in
            if response == .OK, let url = panel.url {
                viewModel.openWorkspace(url: url)
                viewModel.dismissWelcomeScreen()
            }
        }
    }
}

// MARK: - Welcome Action Card

private struct WelcomeActionCard: View {
    let icon: String
    let title: String
    let subtitle: String
    var isPrimary: Bool = false
    var isEnabled: Bool = true
    let action: () -> Void

    @Environment(ThemeManager.self) private var theme
    @State private var hovering = false

    var body: some View {
        Button(action: action) {
            HStack(alignment: .center, spacing: 14) {
                Image(systemName: icon)
                    .font(.system(size: 20, weight: .regular))
                    .foregroundStyle(iconColor)
                    .frame(width: 28, height: 28)

                VStack(alignment: .leading, spacing: 3) {
                    Text(title)
                        .font(.system(size: 14, weight: .semibold))
                        .foregroundStyle(theme.colors.textPrimary)
                    Text(subtitle)
                        .font(.system(size: 11, weight: .regular))
                        .foregroundStyle(theme.colors.textTertiary)
                        .lineLimit(1)
                }

                Spacer(minLength: 0)
            }
            .padding(.horizontal, 16)
            .padding(.vertical, 14)
            .frame(maxWidth: .infinity, alignment: .leading)
            .background(
                RoundedRectangle(cornerRadius: 10)
                    .fill(backgroundFill)
            )
            .overlay(
                RoundedRectangle(cornerRadius: 10)
                    .stroke(borderColor, lineWidth: isPrimary ? 1 : 0.5)
            )
            .scaleEffect(hovering && isEnabled ? 1.015 : 1.0)
            .animation(.easeOut(duration: 0.12), value: hovering)
        }
        .buttonStyle(.plain)
        .disabled(!isEnabled)
        .opacity(isEnabled ? 1.0 : 0.45)
        .onHover { hovering = $0 }
    }

    private var iconColor: Color {
        isPrimary ? theme.colors.accent : theme.colors.textSecondary
    }

    private var backgroundFill: Color {
        if !isEnabled { return theme.colors.surface }
        if hovering { return theme.colors.surfaceHover }
        return theme.colors.surface
    }

    private var borderColor: Color {
        isPrimary ? theme.colors.accent.opacity(0.5) : theme.colors.borderSubtle
    }
}

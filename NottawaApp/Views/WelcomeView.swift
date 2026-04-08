//
//  WelcomeView.swift
//  NottawaApp
//
//  Welcome screen shown on launch with a live strand preview header,
//  action buttons, and two-column strand browsers (Templates + Recent).
//

import SwiftUI

struct WelcomeView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel
    @Environment(ThemeManager.self) private var theme

    @State private var currentWelcomeIndex: Int = 0

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

                VStack(spacing: 0) {
                    // MARK: - Live Strand Preview Header
                    livePreviewHeader(width: panelWidth)

                    Spacer().frame(height: 16)

                    // MARK: - Action Buttons
                    actionButtons()

                    Spacer().frame(height: 16)

                    // MARK: - Two-Column Browsers
                    strandBrowsers(height: panelHeight * 0.45)
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
            // Load the first template strand for live preview
            loadCurrentWelcomeStrand()
        }
        .onExitCommand {
            viewModel.dismissWelcomeScreen()
        }
    }

    // MARK: - Live Preview Header

    @ViewBuilder
    private func livePreviewHeader(width: CGFloat) -> some View {
        let aspectRatio = viewModel.welcomePreviewAspectRatio
        let headerHeight = width / aspectRatio

        ZStack(alignment: .bottom) {
            // Black background behind the live texture
            Color.black
                .frame(width: width, height: headerHeight)

            // Live texture from the engine
            if let outputId = viewModel.welcomePreviewOutputId {
                TextureDisplayView(connectableId: outputId)
                    .aspectRatio(aspectRatio, contentMode: .fit)
                    .frame(width: width, height: headerHeight)
                    .clipped()
            }

            // Strand name overlay at top-left
            if !viewModel.templateStrandsList.isEmpty {
                VStack {
                    HStack {
                        Text(currentStrandName)
                            .font(.caption.weight(.semibold))
                            .foregroundStyle(.white)
                            .padding(.horizontal, 10)
                            .padding(.vertical, 5)
                            .background(.black.opacity(0.5), in: Capsule())
                        Spacer()
                    }
                    .padding(.horizontal, 12)
                    .padding(.top, 10)
                    Spacer()
                }
            }

            // Bottom overlay bar
            HStack {
                // Always show at launch toggle
                DSToggle(isOn: Bindable(viewModel).welcomeScreenEnabled, label: "Always show at launch", style: .checkbox, size: .sm)
                    .padding(.horizontal, 10)
                    .padding(.vertical, 6)
                    .background(
                        Capsule().fill(Color.black.opacity(0.5))
                    )

                Spacer()

                // Navigation arrows
                if viewModel.templateStrandsList.count > 1 {
                    HStack(spacing: 4) {
                        Button {
                            navigateStrand(delta: -1)
                        } label: {
                            Image(systemName: "chevron.left")
                                .font(.caption.weight(.bold))
                                .foregroundStyle(.white)
                                .frame(width: 28, height: 28)
                                .background(.white.opacity(0.2), in: Circle())
                        }
                        .buttonStyle(.plainHitArea)

                        Text("\(currentWelcomeIndex + 1)/\(viewModel.templateStrandsList.count)")
                            .font(.caption2.weight(.medium).monospacedDigit())
                            .foregroundStyle(.white.opacity(0.8))

                        Button {
                            navigateStrand(delta: 1)
                        } label: {
                            Image(systemName: "chevron.right")
                                .font(.caption.weight(.bold))
                                .foregroundStyle(.white)
                                .frame(width: 28, height: 28)
                                .background(.white.opacity(0.2), in: Circle())
                        }
                        .buttonStyle(.plainHitArea)
                    }
                }

                Spacer()

                // Use this strand button
                Button("Use this") {
                    viewModel.dismissWelcomeScreen()
                }
                .buttonStyle(.plainHitArea)
                .padding(.horizontal, 10)
                .padding(.vertical, 4)
                .background(theme.colors.accent)
                .foregroundStyle(theme.colors.textOnAccent)
                .clipShape(Capsule())
                .controlSize(.small)
                .padding(.horizontal, 10)
                .padding(.vertical, 6)
            }
            .padding(.horizontal, 10)
            .padding(.bottom, 10)
        }
        .clipShape(
            UnevenRoundedRectangle(topLeadingRadius: 12, topTrailingRadius: 12)
        )
    }

    // MARK: - Action Buttons

    @ViewBuilder
    private func actionButtons() -> some View {
        HStack(spacing: 24) {
            Button {
                viewModel.resumePreviousWorkspace()
            } label: {
                Text("Resume Workspace")
                    .frame(width: 160, height: 36)
            }
            .buttonStyle(.plainHitArea)
            .background(theme.colors.surface)
            .foregroundStyle(theme.colors.textPrimary)
            .clipShape(RoundedRectangle(cornerRadius: 8))

            Button {
                viewModel.newWorkspace()
                viewModel.dismissWelcomeScreen()
            } label: {
                Text("New Workspace")
                    .frame(width: 160, height: 36)
            }
            .buttonStyle(.plainHitArea)
            .background(theme.colors.surface)
            .foregroundStyle(theme.colors.textPrimary)
            .clipShape(RoundedRectangle(cornerRadius: 8))

            Button {
                openWorkspacePanel()
            } label: {
                Text("Open Workspace")
                    .frame(width: 160, height: 36)
            }
            .buttonStyle(.plainHitArea)
            .background(theme.colors.surface)
            .foregroundStyle(theme.colors.textPrimary)
            .clipShape(RoundedRectangle(cornerRadius: 8))
        }
    }

    // MARK: - Strand Browsers

    @ViewBuilder
    private func strandBrowsers(height: CGFloat) -> some View {
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
            .frame(maxWidth: .infinity)
            .frame(height: height)

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
            .frame(maxWidth: .infinity)
            .frame(height: height)
        }
        .padding(.horizontal, 16)
        .padding(.bottom, 16)
    }

    // MARK: - Helpers

    private var currentStrandName: String {
        guard !viewModel.templateStrandsList.isEmpty,
              currentWelcomeIndex < viewModel.templateStrandsList.count else {
            return ""
        }
        return viewModel.templateStrandsList[currentWelcomeIndex].name
    }

    private func loadCurrentWelcomeStrand() {
        guard !viewModel.templateStrandsList.isEmpty else { return }
        let index = currentWelcomeIndex.clamped(to: 0..<viewModel.templateStrandsList.count)
        currentWelcomeIndex = index
        let strand = viewModel.templateStrandsList[index]
        viewModel.loadWelcomeStrand(id: strand.id)
    }

    private func navigateStrand(delta: Int) {
        guard !viewModel.templateStrandsList.isEmpty else { return }
        let count = viewModel.templateStrandsList.count
        currentWelcomeIndex = (currentWelcomeIndex + delta + count) % count
        loadCurrentWelcomeStrand()
    }

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

private extension Int {
    func clamped(to range: Range<Int>) -> Int {
        Swift.max(range.lowerBound, Swift.min(self, range.upperBound - 1))
    }
}

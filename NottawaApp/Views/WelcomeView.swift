//
//  WelcomeView.swift
//  NottawaApp
//
//  Welcome screen shown on launch with intro video, action buttons,
//  and two-column strand browsers (Templates + Recent).
//

import SwiftUI
import AVKit

struct WelcomeView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel
    @Environment(ThemeManager.self) private var theme

    @State private var player: AVPlayer?

    var body: some View {
        ZStack {
            // Dark scrim behind panel
            Color.black.opacity(0.5)
                .ignoresSafeArea()
                .onTapGesture {
                    viewModel.dismissWelcomeScreen()
                }

            GeometryReader { geo in
                let panelWidth = geo.size.width * 0.60
                let panelHeight = geo.size.height * 0.80

                VStack(spacing: 0) {
                    // MARK: - Header Video
                    headerVideo(width: panelWidth)

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
            setupPlayer()
            viewModel.fetchTemplateStrands()
            viewModel.fetchStrands()
        }
        .onDisappear {
            player?.pause()
        }
    }

    // MARK: - Header Video

    @ViewBuilder
    private func headerVideo(width: CGFloat) -> some View {
        let headerHeight = width * 0.30

        ZStack(alignment: .bottom) {
            if let player = player {
                LoopingVideoView(player: player)
                    .frame(width: width, height: headerHeight)
                    .clipped()
            } else {
                Rectangle()
                    .fill(Color.black)
                    .frame(width: width, height: headerHeight)
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

                // Try this out button
                Button("Try this out") {
                    viewModel.loadDemoStrand()
                }
                .buttonStyle(.plain)
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
                viewModel.dismissWelcomeScreen()
            } label: {
                Text("Resume Workspace")
                    .frame(width: 160, height: 36)
            }
            .buttonStyle(.plain)
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
            .buttonStyle(.plain)
            .background(theme.colors.surface)
            .foregroundStyle(theme.colors.textPrimary)
            .clipShape(RoundedRectangle(cornerRadius: 8))

            Button {
                openWorkspacePanel()
            } label: {
                Text("Open Workspace")
                    .frame(width: 160, height: 36)
            }
            .buttonStyle(.plain)
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

    private func setupPlayer() {
        // The intro video is copied to Resources/data/images/intro/ by copy_resources.sh
        let dataPath = Bundle.main.resourcePath.map { $0 + "/data/images/intro/intro.mov" }
        if let path = dataPath, FileManager.default.fileExists(atPath: path) {
            let url = URL(fileURLWithPath: path)
            let avPlayer = AVPlayer(url: url)
            avPlayer.isMuted = true
            avPlayer.actionAtItemEnd = .none

            // Loop the video
            NotificationCenter.default.addObserver(
                forName: .AVPlayerItemDidPlayToEndTime,
                object: avPlayer.currentItem,
                queue: .main
            ) { _ in
                avPlayer.seek(to: .zero)
                avPlayer.play()
            }

            avPlayer.play()
            self.player = avPlayer
        }
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

// MARK: - Looping Video (no controls)

private struct LoopingVideoView: NSViewRepresentable {
    let player: AVPlayer

    func makeNSView(context: Context) -> NSView {
        let view = NSView()
        let playerLayer = AVPlayerLayer(player: player)
        playerLayer.videoGravity = .resizeAspectFill
        playerLayer.autoresizingMask = [.layerWidthSizable, .layerHeightSizable]
        view.wantsLayer = true
        view.layer?.addSublayer(playerLayer)
        return view
    }

    func updateNSView(_ nsView: NSView, context: Context) {}
}

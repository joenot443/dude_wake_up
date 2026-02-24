//
//  VideoSourceBrowserView.swift
//  NottawaApp
//
//  Browser for adding video sources to the canvas.
//  Shows live animated previews using IOSurface texture sharing.
//

import SwiftUI

struct VideoSourceBrowserView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel

    @State private var searchText = ""
    @State private var selectedCategory = "All"
    private let previewManager = BrowserPreviewManager.shared

    private let columns = [
        GridItem(.flexible(), spacing: 12),
        GridItem(.flexible(), spacing: 12),
        GridItem(.flexible(), spacing: 12),
    ]

    private var allSources: [AvailableShaderInfo] {
        NottawaEngine.shared.availableShaderSources()
    }

    private var categories: [String] {
        var cats = Set(allSources.compactMap(\.category))
        return ["All"] + cats.sorted()
    }

    private var filteredSources: [AvailableShaderInfo] {
        var sources = allSources
        if selectedCategory != "All" {
            sources = sources.filter { $0.category == selectedCategory }
        }
        if !searchText.isEmpty {
            sources = sources.filter { $0.name.localizedCaseInsensitiveContains(searchText) }
        }
        return sources
    }

    private var isSwapMode: Bool {
        viewModel.browserMode == .swap
    }

    var body: some View {
        VStack(spacing: 0) {
            // Header
            HStack {
                Text(isSwapMode ? "Swap Source" : "Add Video Source")
                    .font(.title2)
                    .fontWeight(.semibold)
                Spacer()
                Button("Cancel") {
                    closeBrowser()
                }
                .keyboardShortcut(.cancelAction)
            }
            .padding()

            // Search
            TextField("Search sources...", text: $searchText)
                .textFieldStyle(.roundedBorder)
                .padding(.horizontal)

            // Category picker
            ScrollView(.horizontal, showsIndicators: false) {
                HStack(spacing: 6) {
                    ForEach(categories, id: \.self) { cat in
                        Button {
                            selectedCategory = cat
                        } label: {
                            Text(cat)
                                .font(.system(size: 12, weight: selectedCategory == cat ? .semibold : .regular))
                                .padding(.horizontal, 10)
                                .padding(.vertical, 5)
                                .background(selectedCategory == cat ? Color.accentColor : Color(.controlBackgroundColor))
                                .foregroundStyle(selectedCategory == cat ? .white : .primary)
                                .clipShape(Capsule())
                        }
                        .buttonStyle(.plain)
                    }
                }
                .padding(.horizontal)
                .padding(.vertical, 8)
            }

            // Grid
            ScrollView {
                LazyVGrid(columns: columns, spacing: 12) {
                    ForEach(filteredSources) { source in
                        PreviewTileView(
                            name: source.name,
                            shaderTypeRaw: source.shaderTypeRaw,
                            isSource: true,
                            previewManager: previewManager
                        ) {
                            if isSwapMode, let targetId = viewModel.swapTargetNodeId {
                                viewModel.swapSource(nodeId: targetId, newType: source.shaderTypeRaw)
                            } else {
                                viewModel.addShaderVideoSource(type: source.shaderTypeRaw)
                            }
                            closeBrowser()
                        }
                    }
                }
                .padding()
            }
        }
        .frame(maxWidth: 600, maxHeight: 500)
    }

    private func closeBrowser() {
        previewManager.destroyAll()
        viewModel.browserMode = .add
        viewModel.swapTargetNodeId = nil
        viewModel.showSourceBrowser = false
    }
}

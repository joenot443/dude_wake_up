//
//  ShaderBrowserView.swift
//  NottawaApp
//
//  Browser for adding or swapping shader effects on the canvas.
//  Shows live animated previews using IOSurface texture sharing.
//

import SwiftUI

struct ShaderBrowserView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel

    @State private var searchText = ""
    @State private var selectedCategory: NottawaEngine.ShaderCategory = .all
    private let previewManager = BrowserPreviewManager.shared

    private let columns = [
        GridItem(.flexible(), spacing: 12),
        GridItem(.flexible(), spacing: 12),
        GridItem(.flexible(), spacing: 12),
    ]

    private var isSwapMode: Bool {
        viewModel.browserMode == .swap
    }

    private var filteredShaders: [AvailableShaderInfo] {
        let shaders = NottawaEngine.shared.availableShaders(category: selectedCategory)
        if searchText.isEmpty { return shaders }
        return shaders.filter { $0.name.localizedCaseInsensitiveContains(searchText) }
    }

    var body: some View {
        VStack(spacing: 0) {
            // Header
            HStack {
                Text(isSwapMode ? "Swap Effect" : "Add Effect")
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
            TextField("Search effects...", text: $searchText)
                .textFieldStyle(.roundedBorder)
                .padding(.horizontal)

            // Category picker
            Picker("Category", selection: $selectedCategory) {
                Text("All").tag(NottawaEngine.ShaderCategory.all)
                Text("Basic").tag(NottawaEngine.ShaderCategory.basic)
                Text("Filter").tag(NottawaEngine.ShaderCategory.filter)
                Text("Glitch").tag(NottawaEngine.ShaderCategory.glitch)
                Text("Transform").tag(NottawaEngine.ShaderCategory.transform)
                Text("Mix").tag(NottawaEngine.ShaderCategory.mix)
                Text("Mask").tag(NottawaEngine.ShaderCategory.mask)
            }
            .pickerStyle(.segmented)
            .padding()

            // Grid
            ScrollView {
                LazyVGrid(columns: columns, spacing: 12) {
                    ForEach(filteredShaders) { shader in
                        PreviewTileView(
                            name: shader.name,
                            shaderTypeRaw: shader.shaderTypeRaw,
                            isSource: false,
                            previewManager: previewManager
                        ) {
                            if isSwapMode, let targetId = viewModel.swapTargetNodeId {
                                viewModel.swapShader(nodeId: targetId, newType: shader.shaderTypeRaw)
                            } else {
                                viewModel.addShader(type: shader.shaderTypeRaw)
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
        viewModel.showShaderBrowser = false
    }
}

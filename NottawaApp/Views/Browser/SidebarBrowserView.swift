//
//  SidebarBrowserView.swift
//  NottawaApp
//
//  Persistent left sidebar browser with Shaders/Sources tabs,
//  search, category filter, and a 2-column grid of draggable tiles.
//

import SwiftUI

// MARK: - Icon Tab Button

private struct SidebarTabButton: View {
    let icon: String
    let label: String
    let isActive: Bool
    let action: () -> Void

    var body: some View {
        Button(action: action) {
            VStack(spacing: 2) {
                Image(systemName: icon)
                    .font(.system(size: 16))
                Text(label)
                    .font(.system(size: 9))
            }
            .foregroundStyle(isActive ? Color.accentColor : .secondary)
            .frame(maxWidth: .infinity)
        }
        .buttonStyle(.plain)
    }
}

// MARK: - Section Header

private struct SectionHeaderView: View {
    let title: String
    let isCollapsed: Bool
    let onToggle: () -> Void

    var body: some View {
        Button(action: onToggle) {
            HStack(spacing: 4) {
                Image(systemName: isCollapsed ? "chevron.right" : "chevron.down")
                    .font(.system(size: 8, weight: .semibold))
                    .foregroundStyle(.tertiary)
                Text(title)
                    .font(.system(size: 11, weight: .semibold))
                    .foregroundStyle(.secondary)
                Spacer()
            }
            .padding(.horizontal, 6)
            .padding(.vertical, 4)
            .frame(maxWidth: .infinity)
            .background(.ultraThinMaterial)
            .contentShape(Rectangle())
        }
        .buttonStyle(.plain)
    }
}

// MARK: - Category Group

private struct CategoryGroup<Item: Identifiable>: Identifiable {
    let name: String
    let items: [Item]
    var id: String { name }
}

// MARK: - Sidebar Browser View

struct SidebarBrowserView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel

    @State private var searchText = ""
    @State private var searchVisible = false
    @State private var selectedShaderCategory: NottawaEngine.ShaderCategory = .all
    @State private var selectedSourceCategory = "All"
    @State private var collapsedSections: Set<String> = []
    private let previewManager = BrowserPreviewManager.shared
    @AppStorage("sidebarLivePreview") private var livePreview = true

    private let columns = [
        GridItem(.adaptive(minimum: 78), spacing: 4),
    ]

    // MARK: - Shader Categories

    private static let shaderCategories: [(String, NottawaEngine.ShaderCategory)] = [
        ("All", .all),
        ("Basic", .basic),
        ("Filter", .filter),
        ("Glitch", .glitch),
        ("Transform", .transform),
        ("Mix", .mix),
        ("Mask", .mask),
    ]

    // MARK: - Data

    private var filteredShaders: [AvailableShaderInfo] {
        let shaders = NottawaEngine.shared.availableShaders(category: selectedShaderCategory)
        if searchText.isEmpty { return shaders }
        return shaders.filter { $0.name.localizedCaseInsensitiveContains(searchText) }
    }

    private var shaderCategoryGroups: [CategoryGroup<AvailableShaderInfo>] {
        Self.shaderCategories
            .filter { $0.1 != .all }
            .compactMap { label, cat in
                var items = NottawaEngine.shared.availableShaders(category: cat)
                if !searchText.isEmpty {
                    items = items.filter { $0.name.localizedCaseInsensitiveContains(searchText) }
                }
                return items.isEmpty ? nil : CategoryGroup(name: label, items: items)
            }
    }

    private var allSources: [AvailableShaderInfo] {
        NottawaEngine.shared.availableShaderSources()
    }

    private var sourceCategories: [String] {
        let cats = Set(allSources.compactMap(\.category))
        return ["All", "Input"] + cats.sorted()
    }

    private var filteredSources: [AvailableShaderInfo] {
        var sources = allSources
        if selectedSourceCategory != "All" {
            sources = sources.filter { $0.category == selectedSourceCategory }
        }
        if !searchText.isEmpty {
            sources = sources.filter { $0.name.localizedCaseInsensitiveContains(searchText) }
        }
        return sources
    }

    private var sourceCategoryGroups: [CategoryGroup<AvailableShaderInfo>] {
        let cats = Set(allSources.compactMap(\.category)).sorted()
        return cats.compactMap { cat in
            var items = allSources.filter { $0.category == cat }
            if !searchText.isEmpty {
                items = items.filter { $0.name.localizedCaseInsensitiveContains(searchText) }
            }
            return items.isEmpty ? nil : CategoryGroup(name: cat, items: items)
        }
    }

    // MARK: - Body

    var body: some View {
        VStack(spacing: 0) {
            // Icon tab bar
            HStack(spacing: 0) {
                SidebarTabButton(icon: "sparkles", label: "Shaders",
                                 isActive: viewModel.sidebarActiveTab == .shaders) {
                    viewModel.sidebarActiveTab = .shaders
                }
                SidebarTabButton(icon: "video", label: "Sources",
                                 isActive: viewModel.sidebarActiveTab == .sources) {
                    viewModel.sidebarActiveTab = .sources
                }
                SidebarTabButton(icon: "waveform.path", label: "Osc",
                                 isActive: viewModel.sidebarActiveTab == .oscillators) {
                    viewModel.sidebarActiveTab = .oscillators
                }
                SidebarTabButton(icon: "books.vertical", label: "Library",
                                 isActive: viewModel.sidebarActiveTab == .library) {
                    viewModel.sidebarActiveTab = .library
                }
                SidebarTabButton(icon: "rectangle.stack", label: "Strands",
                                 isActive: viewModel.sidebarActiveTab == .strands) {
                    viewModel.sidebarActiveTab = .strands
                }
            }
            .padding(.horizontal, 8)
            .padding(.top, 8)
            .padding(.bottom, 6)
            .accessibilityIdentifier("sidebar-tab-picker")

            if viewModel.sidebarActiveTab == .oscillators {
                Divider()
                OscillatorsSidebarView()
            } else if viewModel.sidebarActiveTab == .library {
                Divider()
                LibrarySidebarView()
            } else if viewModel.sidebarActiveTab == .strands {
                Divider()
                StrandsSidebarView()
            } else {
                // Combined control row
                controlRow

                // Expandable search
                if searchVisible {
                    HStack(spacing: 4) {
                        TextField(
                            viewModel.sidebarActiveTab == .shaders ? "Search shaders..." : "Search sources...",
                            text: $searchText
                        )
                        .textFieldStyle(.roundedBorder)
                        .accessibilityIdentifier("sidebar-search")

                        Button {
                            searchText = ""
                            searchVisible = false
                        } label: {
                            Image(systemName: "xmark.circle.fill")
                                .font(.system(size: 12))
                                .foregroundStyle(.secondary)
                        }
                        .buttonStyle(.plain)
                    }
                    .padding(.horizontal, 10)
                    .padding(.bottom, 6)
                }

                Divider()

                // Grid
                gridContent
            }
        }
        .frame(width: viewModel.sidebarWidth)
        .background(.ultraThinMaterial)
        .onAppear {
            // Snapshots are loaded on-demand per tile via requestSnapshot()
            // in each tile's .onAppear. Eager preloading of all ~165 shaders
            // causes ~30s of GPU contention at startup.
        }
        .onChange(of: viewModel.sidebarActiveTab) { _, _ in
            searchText = ""
            searchVisible = false
        }
    }

    // MARK: - Control Row

    private var controlRow: some View {
        HStack(spacing: 6) {
            // Search toggle
            Button {
                searchVisible.toggle()
                if !searchVisible { searchText = "" }
            } label: {
                Image(systemName: "magnifyingglass")
                    .font(.system(size: 12))
                    .foregroundStyle(searchVisible ? Color.accentColor : .secondary)
            }
            .buttonStyle(.plain)

            // Category dropdown
            if viewModel.sidebarActiveTab == .shaders {
                shaderCategoryMenu
            } else {
                sourceCategoryMenu
            }

            Spacer()

            // Live preview toggle
            Toggle(isOn: $livePreview) {
                Image(systemName: livePreview ? "play.fill" : "pause.fill")
                    .font(.system(size: 10))
            }
            .toggleStyle(.checkbox)
            .help(livePreview ? "Live Preview On" : "Live Preview Off")
            .accessibilityIdentifier("live-preview-toggle")
        }
        .padding(.horizontal, 10)
        .padding(.vertical, 4)
    }

    // MARK: - Category Menus

    private var shaderCategoryMenu: some View {
        Menu {
            ForEach(Self.shaderCategories, id: \.1) { label, cat in
                Button {
                    selectedShaderCategory = cat
                } label: {
                    if selectedShaderCategory == cat {
                        Label(label, systemImage: "checkmark")
                    } else {
                        Text(label)
                    }
                }
            }
        } label: {
            HStack(spacing: 3) {
                Text(Self.shaderCategories.first { $0.1 == selectedShaderCategory }?.0 ?? "All")
                    .font(.system(size: 11))
                Image(systemName: "chevron.down")
                    .font(.system(size: 8))
            }
            .foregroundStyle(.primary)
        }
        .menuStyle(.borderlessButton)
        .fixedSize()
    }

    private var sourceCategoryMenu: some View {
        Menu {
            ForEach(sourceCategories, id: \.self) { cat in
                Button {
                    selectedSourceCategory = cat
                } label: {
                    if selectedSourceCategory == cat {
                        Label(cat, systemImage: "checkmark")
                    } else {
                        Text(cat)
                    }
                }
            }
        } label: {
            HStack(spacing: 3) {
                Text(selectedSourceCategory)
                    .font(.system(size: 11))
                Image(systemName: "chevron.down")
                    .font(.system(size: 8))
            }
            .foregroundStyle(.primary)
        }
        .menuStyle(.borderlessButton)
        .fixedSize()
    }

    // MARK: - Grid Content

    @ViewBuilder
    private var gridContent: some View {
        if viewModel.sidebarActiveTab == .shaders {
            shaderGrid
        } else {
            sourceGrid
        }
    }

    private var shaderGrid: some View {
        ScrollView {
            if selectedShaderCategory == .all {
                LazyVGrid(columns: columns, spacing: 4, pinnedViews: [.sectionHeaders]) {
                    ForEach(shaderCategoryGroups) { group in
                        Section {
                            if !collapsedSections.contains(group.name) {
                                ForEach(group.items) { shader in
                                    shaderTile(shader)
                                }
                            }
                        } header: {
                            SectionHeaderView(
                                title: group.name,
                                isCollapsed: collapsedSections.contains(group.name)
                            ) {
                                toggleSection(group.name)
                            }
                        }
                    }
                }
                .padding(6)
            } else {
                LazyVGrid(columns: columns, spacing: 4) {
                    ForEach(filteredShaders) { shader in
                        shaderTile(shader)
                    }
                }
                .padding(6)
            }
        }
    }

    private var nonShaderSources: [AvailableNonShaderSourceInfo] {
        NottawaEngine.shared.availableNonShaderSources()
    }

    private var sourceGrid: some View {
        ScrollView {
            // Non-shader sources section (Text, Icon, Webcam)
            if selectedSourceCategory == "All" || selectedSourceCategory == "Input" {
                if !nonShaderSources.isEmpty {
                    LazyVGrid(columns: columns, spacing: 4, pinnedViews: [.sectionHeaders]) {
                        Section {
                            if !collapsedSections.contains("Input") {
                                ForEach(nonShaderSources) { source in
                                    nonShaderSourceTile(source)
                                }
                            }
                        } header: {
                            SectionHeaderView(
                                title: "Input",
                                isCollapsed: collapsedSections.contains("Input")
                            ) {
                                toggleSection("Input")
                            }
                        }
                    }
                    .padding(.horizontal, 6)
                    .padding(.top, 6)
                }
            }

            // Shader sources
            if selectedSourceCategory == "All" {
                LazyVGrid(columns: columns, spacing: 4, pinnedViews: [.sectionHeaders]) {
                    ForEach(sourceCategoryGroups) { group in
                        Section {
                            if !collapsedSections.contains(group.name) {
                                ForEach(group.items) { source in
                                    sourceTile(source)
                                }
                            }
                        } header: {
                            SectionHeaderView(
                                title: group.name,
                                isCollapsed: collapsedSections.contains(group.name)
                            ) {
                                toggleSection(group.name)
                            }
                        }
                    }
                }
                .padding(6)
            } else if selectedSourceCategory != "Input" {
                LazyVGrid(columns: columns, spacing: 4) {
                    ForEach(filteredSources) { source in
                        sourceTile(source)
                    }
                }
                .padding(6)
            }
        }
    }

    private func toggleSection(_ name: String) {
        withAnimation(.easeInOut(duration: 0.2)) {
            if collapsedSections.contains(name) {
                collapsedSections.remove(name)
            } else {
                collapsedSections.insert(name)
            }
        }
    }

    // MARK: - Tile Helpers

    private func shaderTile(_ shader: AvailableShaderInfo) -> some View {
        SidebarTileView(
            name: shader.name,
            shaderTypeRaw: shader.shaderTypeRaw,
            isSource: false,
            livePreview: livePreview,
            previewManager: previewManager
        ) {
            viewModel.addShader(type: shader.shaderTypeRaw)
        }
    }

    private func sourceTile(_ source: AvailableShaderInfo) -> some View {
        SidebarTileView(
            name: source.name,
            shaderTypeRaw: source.shaderTypeRaw,
            isSource: true,
            livePreview: livePreview,
            previewManager: previewManager
        ) {
            viewModel.addShaderVideoSource(type: source.shaderTypeRaw)
        }
    }

    private func nonShaderSourceTile(_ source: AvailableNonShaderSourceInfo) -> some View {
        NonShaderSourceTileView(
            name: source.name,
            icon: source.icon,
            sourceType: source.sourceType
        ) {
            viewModel.addNonShaderSource(type: source.sourceType)
        }
    }
}

// MARK: - Sidebar Tile View

/// Compact tile for the sidebar with live or static preview and drag support.
struct SidebarTileView: View {
    let name: String
    let shaderTypeRaw: Int
    let isSource: Bool
    let livePreview: Bool
    let previewManager: BrowserPreviewManager
    let action: () -> Void

    @State private var previewId: String?

    private var dragPayload: String {
        isSource ? "source:\(shaderTypeRaw)" : "shader:\(shaderTypeRaw)"
    }

    var body: some View {
        Button(action: action) {
            VStack(spacing: 4) {
                ZStack {
                    RoundedRectangle(cornerRadius: 6)
                        .fill(isSource ? Color.blue.opacity(0.15) : Color.purple.opacity(0.15))
                        .aspectRatio(16.0 / 9.0, contentMode: .fit)

                    if livePreview, let pid = previewId {
                        TextureDisplayView(connectableId: pid)
                            .aspectRatio(16.0 / 9.0, contentMode: .fit)
                            .clipShape(RoundedRectangle(cornerRadius: 6))
                            .allowsHitTesting(false)
                    } else if let snapshot = previewManager.snapshot(for: shaderTypeRaw, isSource: isSource) {
                        Image(decorative: snapshot, scale: 1)
                            .resizable()
                            .aspectRatio(16.0 / 9.0, contentMode: .fit)
                            .clipShape(RoundedRectangle(cornerRadius: 6))
                    } else {
                        Image(systemName: isSource ? "video.fill" : "sparkles")
                            .font(.caption)
                            .foregroundStyle(isSource ? .blue : .purple)
                    }
                }

                Text(name)
                    .font(.system(size: 10))
                    .lineLimit(1)
                    .foregroundStyle(.primary)
            }
            .padding(6)
            .background(
                RoundedRectangle(cornerRadius: 8)
                    .fill(Color(.controlBackgroundColor))
            )
            .overlay(
                RoundedRectangle(cornerRadius: 8)
                    .strokeBorder(Color(.separatorColor), lineWidth: 0.5)
            )
        }
        .buttonStyle(.plain)
        .accessibilityIdentifier("sidebar-tile-\(name)")
        .onDrag {
            NSItemProvider(object: dragPayload as NSString)
        }
        .onAppear {
            if livePreview {
                previewId = previewManager.previewId(for: shaderTypeRaw, isSource: isSource)
            } else {
                previewManager.requestSnapshot(for: shaderTypeRaw, isSource: isSource)
            }
        }
        .onDisappear {
            previewId = nil
            previewManager.destroyPreview(for: shaderTypeRaw, isSource: isSource)
        }
        .onChange(of: livePreview) { _, newValue in
            if newValue {
                previewId = previewManager.previewId(for: shaderTypeRaw, isSource: isSource)
            } else {
                previewManager.captureSnapshotFromLive(for: shaderTypeRaw, isSource: isSource)
                previewId = nil
                previewManager.destroyPreview(for: shaderTypeRaw, isSource: isSource)
            }
        }
    }
}

// MARK: - Non-Shader Source Tile View

/// Compact tile for non-shader sources (Text, Icon, Webcam) using SF Symbols.
struct NonShaderSourceTileView: View {
    let name: String
    let icon: String
    let sourceType: Int
    let action: () -> Void

    private var dragPayload: String {
        "nonsrc:\(sourceType)"
    }

    var body: some View {
        Button(action: action) {
            VStack(spacing: 4) {
                ZStack {
                    RoundedRectangle(cornerRadius: 6)
                        .fill(Color.blue.opacity(0.15))
                        .aspectRatio(16.0 / 9.0, contentMode: .fit)

                    Image(systemName: icon)
                        .font(.title2)
                        .foregroundStyle(.blue)
                }

                Text(name)
                    .font(.system(size: 10))
                    .lineLimit(1)
                    .foregroundStyle(.primary)
            }
            .padding(6)
            .background(
                RoundedRectangle(cornerRadius: 8)
                    .fill(Color(.controlBackgroundColor))
            )
            .overlay(
                RoundedRectangle(cornerRadius: 8)
                    .strokeBorder(Color(.separatorColor), lineWidth: 0.5)
            )
        }
        .buttonStyle(.plain)
        .accessibilityIdentifier("sidebar-tile-\(name)")
        .onDrag {
            NSItemProvider(object: dragPayload as NSString)
        }
    }
}

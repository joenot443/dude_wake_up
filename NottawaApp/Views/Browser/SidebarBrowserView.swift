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
    @Environment(ThemeManager.self) private var theme
    let icon: String
    let label: String
    let isActive: Bool
    let action: () -> Void

    var body: some View {
        Button(action: action) {
            VStack(spacing: 3) {
                Image(systemName: icon)
                    .font(.system(size: 16))
                    .frame(height: 18)
                Text(label)
                    .font(.system(size: 9, weight: isActive ? .semibold : .regular))
                    .lineLimit(1)
                    .fixedSize()
            }
            .foregroundStyle(isActive ? theme.colors.accent : theme.colors.textSecondary)
            .frame(maxWidth: .infinity)
            .padding(.vertical, 4)
            .contentShape(Rectangle())
        }
        .buttonStyle(.plainHitArea)
    }
}

// MARK: - Section Header

private struct SectionHeaderView: View {
    @Environment(ThemeManager.self) private var theme
    let title: String
    let isCollapsed: Bool
    let onToggle: () -> Void

    var body: some View {
        Button(action: onToggle) {
            HStack(spacing: 4) {
                Image(systemName: isCollapsed ? "chevron.right" : "chevron.down")
                    .font(.system(size: 8, weight: .semibold))
                    .foregroundStyle(theme.colors.textTertiary)
                Text(title)
                    .font(.system(size: 11, weight: .semibold))
                    .foregroundStyle(theme.colors.textSecondary)
                Spacer()
            }
            .padding(.horizontal, 6)
            .padding(.vertical, 4)
            .frame(maxWidth: .infinity)
            .background(theme.colors.backgroundSecondary.opacity(0.95))
            .contentShape(Rectangle())
        }
        .buttonStyle(.plainHitArea)
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
    @Environment(ThemeManager.self) private var theme

    @State private var searchText = ""
    @State private var searchVisible = false
    @FocusState private var searchFieldFocused: Bool
    @State private var selectedShaderCategory: NottawaEngine.ShaderCategory = .all
    @State private var selectedSourceCategory = "All"
    @State private var collapsedSections: Set<String> = []
    @State private var showFavoriteShadersOnly = false
    @State private var showFavoriteSourcesOnly = false
    @State private var favoritesRefreshToken = UUID()
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
        _ = favoritesRefreshToken // trigger refresh
        var shaders = NottawaEngine.shared.availableShaders(category: selectedShaderCategory)
        if showFavoriteShadersOnly {
            shaders = shaders.filter(\.isFavorited)
        }
        if searchText.isEmpty { return shaders }
        return shaders.filter { $0.name.localizedCaseInsensitiveContains(searchText) }
    }

    private var shaderCategoryGroups: [CategoryGroup<AvailableShaderInfo>] {
        _ = favoritesRefreshToken
        return Self.shaderCategories
            .filter { $0.1 != .all }
            .compactMap { label, cat in
                var items = NottawaEngine.shared.availableShaders(category: cat)
                if showFavoriteShadersOnly {
                    items = items.filter(\.isFavorited)
                }
                if !searchText.isEmpty {
                    items = items.filter { $0.name.localizedCaseInsensitiveContains(searchText) }
                }
                return items.isEmpty ? nil : CategoryGroup(name: label, items: items)
            }
    }

    private var allSources: [AvailableShaderInfo] {
        _ = favoritesRefreshToken
        return NottawaEngine.shared.availableShaderSources()
    }

    private var sourceCategories: [String] {
        let cats = Set(allSources.compactMap(\.category))
        return ["All", "Input"] + cats.sorted()
    }

    private var filteredSources: [AvailableShaderInfo] {
        _ = favoritesRefreshToken
        var sources = allSources
        if showFavoriteSourcesOnly {
            sources = sources.filter(\.isFavorited)
        }
        if selectedSourceCategory != "All" {
            sources = sources.filter { $0.category == selectedSourceCategory }
        }
        if !searchText.isEmpty {
            sources = sources.filter { $0.name.localizedCaseInsensitiveContains(searchText) }
        }
        return sources
    }

    private var sourceCategoryGroups: [CategoryGroup<AvailableShaderInfo>] {
        _ = favoritesRefreshToken
        let cats = Set(allSources.compactMap(\.category)).sorted()
        return cats.compactMap { cat in
            var items = allSources.filter { $0.category == cat }
            if showFavoriteSourcesOnly {
                items = items.filter(\.isFavorited)
            }
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
                SidebarTabButton(icon: "sparkles", label: "Effects",
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
                theme.colors.border.frame(height: 1)
                OscillatorsSidebarView()
            } else if viewModel.sidebarActiveTab == .library {
                theme.colors.border.frame(height: 1)
                LibrarySidebarView()
            } else if viewModel.sidebarActiveTab == .strands {
                theme.colors.border.frame(height: 1)
                StrandsSidebarView()
            } else {
                // Combined control row
                controlRow

                // Expandable search
                if searchVisible {
                    HStack(spacing: 4) {
                        TextField(
                            viewModel.sidebarActiveTab == .shaders ? "Search effects..." : "Search sources...",
                            text: $searchText
                        )
                        .textFieldStyle(.roundedBorder)
                        .focused($searchFieldFocused)
                        .accessibilityIdentifier("sidebar-search")

                        Button {
                            searchText = ""
                            searchVisible = false
                        } label: {
                            Image(systemName: "xmark.circle.fill")
                                .font(.system(size: 12))
                                .foregroundStyle(theme.colors.textSecondary)
                        }
                        .buttonStyle(.plainHitArea)
                    }
                    .padding(.horizontal, 10)
                    .padding(.bottom, 6)
                }

                theme.colors.border.frame(height: 1)

                // Grid
                gridContent
            }
        }
        .frame(width: viewModel.sidebarWidth)
        .background(theme.colors.backgroundSecondary)
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

    private var isFavoritesActive: Bool {
        viewModel.sidebarActiveTab == .shaders ? showFavoriteShadersOnly : showFavoriteSourcesOnly
    }

    private var controlRow: some View {
        HStack(spacing: 6) {
            // Search toggle
            Button {
                searchVisible.toggle()
                if searchVisible {
                    searchFieldFocused = true
                } else {
                    searchText = ""
                }
            } label: {
                Image(systemName: "magnifyingglass")
                    .font(.system(size: 12))
                    .foregroundStyle(searchVisible ? theme.colors.accent : theme.colors.textSecondary)
            }
            .buttonStyle(.plainHitArea)

            // Favorites filter toggle
            Button {
                if viewModel.sidebarActiveTab == .shaders {
                    showFavoriteShadersOnly.toggle()
                } else {
                    showFavoriteSourcesOnly.toggle()
                }
            } label: {
                Image(systemName: isFavoritesActive ? "star.fill" : "star")
                    .font(.system(size: 12))
                    .foregroundStyle(isFavoritesActive ? theme.colors.warning : theme.colors.textSecondary)
            }
            .buttonStyle(.plainHitArea)
            .help(isFavoritesActive ? "Showing Favorites" : "Show Favorites")

            // Category dropdown
            if viewModel.sidebarActiveTab == .shaders {
                shaderCategoryMenu
            } else {
                sourceCategoryMenu
            }

            // Refresh previews using selected source node (if any) as input
            Button {
                let selectedSourceId = viewModel.selectedNodeId.flatMap { id in
                    viewModel.nodes.first(where: { $0.id == id && $0.isSource })?.id
                }
                BrowserPreviewManager.shared.refreshAllSnapshots(preferredSourceId: selectedSourceId)
            } label: {
                Image(systemName: "arrow.clockwise")
                    .font(.system(size: 12))
                    .foregroundStyle(theme.colors.textSecondary)
            }
            .buttonStyle(.plainHitArea)
            .help("Refresh Previews")

            Spacer()

            // Live preview toggle
            DSToggle(isOn: $livePreview, style: .checkbox, size: .sm)
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
            Text(Self.shaderCategories.first { $0.1 == selectedShaderCategory }?.0 ?? "All")
                .font(.system(size: 11))
                .foregroundStyle(theme.colors.textPrimary)
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
            Text(selectedSourceCategory)
                .font(.system(size: 11))
                .foregroundStyle(theme.colors.textPrimary)
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
        _ = favoritesRefreshToken
        var sources = NottawaEngine.shared.availableNonShaderSources()
        if showFavoriteSourcesOnly {
            sources = sources.filter(\.isFavorited)
        }
        return sources
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
            isFavorited: shader.isFavorited,
            livePreview: livePreview,
            previewManager: previewManager,
            onToggleFavorite: {
                NottawaEngine.shared.toggleFavoriteShaderType(shaderTypeRaw: shader.shaderTypeRaw)
                favoritesRefreshToken = UUID()
            }
        )
    }

    private func sourceTile(_ source: AvailableShaderInfo) -> some View {
        SidebarTileView(
            name: source.name,
            shaderTypeRaw: source.shaderTypeRaw,
            isSource: true,
            isFavorited: source.isFavorited,
            livePreview: livePreview,
            previewManager: previewManager,
            onToggleFavorite: {
                NottawaEngine.shared.toggleFavoriteSourceType(sourceType: source.shaderTypeRaw)
                favoritesRefreshToken = UUID()
            }
        )
    }

    private func nonShaderSourceTile(_ source: AvailableNonShaderSourceInfo) -> some View {
        NonShaderSourceTileView(
            name: source.name,
            icon: source.icon,
            sourceType: source.sourceType,
            isFavorited: source.isFavorited,
            onToggleFavorite: {
                // Non-shader sources use 1000 + sourceType offset
                NottawaEngine.shared.toggleFavoriteSourceType(sourceType: 1000 + source.sourceType)
                favoritesRefreshToken = UUID()
            }
        )
    }
}

// MARK: - Sidebar Tile View

/// Compact tile for the sidebar with live or static preview and drag support.
struct SidebarTileView: View {
    @Environment(ThemeManager.self) private var theme
    let name: String
    let shaderTypeRaw: Int
    let isSource: Bool
    let isFavorited: Bool
    let livePreview: Bool
    let previewManager: BrowserPreviewManager
    let onToggleFavorite: () -> Void

    @State private var previewId: String?

    private var dragPayload: String {
        isSource ? "source:\(shaderTypeRaw)" : "shader:\(shaderTypeRaw)"
    }

    var body: some View {
        VStack(spacing: 4) {
            ZStack(alignment: .topTrailing) {
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

                Button {
                    onToggleFavorite()
                } label: {
                    Image(systemName: isFavorited ? "star.fill" : "star")
                        .font(.system(size: 10))
                        .foregroundStyle(isFavorited ? theme.colors.warning : .white.opacity(0.7))
                        .shadow(color: .black.opacity(0.5), radius: 1, x: 0, y: 0.5)
                        .padding(3)
                }
                .buttonStyle(.plainHitArea)
            }

            Text(name)
                .font(.system(size: 10))
                .lineLimit(1)
                .foregroundStyle(theme.colors.textPrimary)
        }
        .padding(6)
        .background(
            RoundedRectangle(cornerRadius: 8)
                .fill(theme.colors.surface)
        )
        .overlay(
            RoundedRectangle(cornerRadius: 8)
                .strokeBorder(theme.colors.borderSubtle, lineWidth: 0.5)
        )
        .contextMenu {
            Button {
                onToggleFavorite()
            } label: {
                Label(isFavorited ? "Unfavorite" : "Favorite",
                      systemImage: isFavorited ? "star.slash" : "star.fill")
            }
        }
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
    @Environment(ThemeManager.self) private var theme
    let name: String
    let icon: String
    let sourceType: Int
    let isFavorited: Bool
    let onToggleFavorite: () -> Void

    private var dragPayload: String {
        "nonsrc:\(sourceType)"
    }

    var body: some View {
        VStack(spacing: 4) {
            ZStack(alignment: .topTrailing) {
                ZStack {
                    RoundedRectangle(cornerRadius: 6)
                        .fill(Color.blue.opacity(0.15))
                        .aspectRatio(16.0 / 9.0, contentMode: .fit)

                    Image(systemName: icon)
                        .font(.title2)
                        .foregroundStyle(.blue)
                }

                Button {
                    onToggleFavorite()
                } label: {
                    Image(systemName: isFavorited ? "star.fill" : "star")
                        .font(.system(size: 10))
                        .foregroundStyle(isFavorited ? theme.colors.warning : .white.opacity(0.7))
                        .shadow(color: .black.opacity(0.5), radius: 1, x: 0, y: 0.5)
                        .padding(3)
                }
                .buttonStyle(.plainHitArea)
            }

            Text(name)
                .font(.system(size: 10))
                .lineLimit(1)
                .foregroundStyle(theme.colors.textPrimary)
        }
        .padding(6)
        .background(
            RoundedRectangle(cornerRadius: 8)
                .fill(theme.colors.surface)
        )
        .overlay(
            RoundedRectangle(cornerRadius: 8)
                .strokeBorder(theme.colors.borderSubtle, lineWidth: 0.5)
        )
        .contextMenu {
            Button {
                onToggleFavorite()
            } label: {
                Label(isFavorited ? "Unfavorite" : "Favorite",
                      systemImage: isFavorited ? "star.slash" : "star.fill")
            }
        }
        .accessibilityIdentifier("sidebar-tile-\(name)")
        .onDrag {
            NSItemProvider(object: dragPayload as NSString)
        }
    }
}

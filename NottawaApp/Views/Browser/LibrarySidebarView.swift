//
//  LibrarySidebarView.swift
//  NottawaApp
//
//  Library browser sidebar showing downloadable video content
//  from the nottawa.app library with thumbnails, search, and categories.
//

import SwiftUI

struct LibrarySidebarView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel
    @State private var searchText = ""
    @State private var searchVisible = false
    @State private var selectedCategory = "All"
    @State private var collapsedSections: Set<String> = []

    private let columns = [GridItem(.adaptive(minimum: 78), spacing: 4)]

    private var categories: [String] {
        let cats = Set(viewModel.libraryFiles.map(\.category))
        return ["All"] + cats.sorted()
    }

    private var filteredFiles: [LibraryFileInfo] {
        var files = viewModel.libraryFiles
        if selectedCategory != "All" {
            files = files.filter { $0.category == selectedCategory }
        }
        if !searchText.isEmpty {
            files = files.filter { $0.name.localizedCaseInsensitiveContains(searchText) }
        }
        return files
    }

    private var categoryGroups: [LibraryCategoryGroup] {
        let cats = Set(viewModel.libraryFiles.map(\.category)).sorted()
        return cats.compactMap { cat in
            var items = viewModel.libraryFiles.filter { $0.category == cat }
            if !searchText.isEmpty {
                items = items.filter { $0.name.localizedCaseInsensitiveContains(searchText) }
            }
            return items.isEmpty ? nil : LibraryCategoryGroup(name: cat, items: items)
        }
    }

    private var hasActiveDownloads: Bool {
        viewModel.libraryFiles.contains { $0.isDownloading }
    }

    var body: some View {
        VStack(spacing: 0) {
            // Control row
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
                if categories.count > 1 {
                    libraryCategoryMenu
                }

                Spacer()
            }
            .padding(.horizontal, 10)
            .padding(.vertical, 4)

            // Expandable search
            if searchVisible {
                HStack(spacing: 4) {
                    TextField("Search library...", text: $searchText)
                        .textFieldStyle(.roundedBorder)
                        .accessibilityIdentifier("library-search")

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

            if viewModel.libraryLoading && viewModel.libraryFiles.isEmpty {
                VStack(spacing: 8) {
                    ProgressView()
                    Text("Loading library...")
                        .font(.caption)
                        .foregroundStyle(.secondary)
                }
                .frame(maxWidth: .infinity, maxHeight: .infinity)
            } else if viewModel.libraryFiles.isEmpty {
                Text("No library items available")
                    .font(.caption)
                    .foregroundStyle(.secondary)
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
            } else {
                ScrollView {
                    if selectedCategory == "All" {
                        LazyVGrid(columns: columns, spacing: 4, pinnedViews: [.sectionHeaders]) {
                            ForEach(categoryGroups) { group in
                                Section {
                                    if !collapsedSections.contains(group.name) {
                                        ForEach(group.items) { file in
                                            LibraryTileView(file: file) {
                                                viewModel.addLibrarySource(libraryFileId: file.id)
                                            }
                                        }
                                    }
                                } header: {
                                    LibrarySectionHeaderView(
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
                            ForEach(filteredFiles) { file in
                                LibraryTileView(file: file) {
                                    viewModel.addLibrarySource(libraryFileId: file.id)
                                }
                            }
                        }
                        .padding(6)
                    }
                }
            }
        }
        .onAppear {
            if viewModel.libraryFiles.isEmpty && !viewModel.libraryLoading {
                viewModel.fetchLibrary()
            }
            if hasActiveDownloads {
                viewModel.startDownloadPolling()
            }
        }
        .onDisappear {
            viewModel.stopDownloadPolling()
        }
        .onChange(of: hasActiveDownloads) { _, active in
            if active {
                viewModel.startDownloadPolling()
            } else {
                viewModel.stopDownloadPolling()
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

    // MARK: - Category Menu

    private var libraryCategoryMenu: some View {
        Menu {
            ForEach(categories, id: \.self) { cat in
                Button {
                    selectedCategory = cat
                } label: {
                    if selectedCategory == cat {
                        Label(cat, systemImage: "checkmark")
                    } else {
                        Text(cat)
                    }
                }
            }
        } label: {
            HStack(spacing: 3) {
                Text(selectedCategory)
                    .font(.system(size: 11))
                Image(systemName: "chevron.down")
                    .font(.system(size: 8))
            }
            .foregroundStyle(.primary)
        }
        .menuStyle(.borderlessButton)
        .fixedSize()
    }
}

// MARK: - Library Section Header

private struct LibrarySectionHeaderView: View {
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

// MARK: - Library Category Group

private struct LibraryCategoryGroup: Identifiable {
    let name: String
    let items: [LibraryFileInfo]
    var id: String { name }
}

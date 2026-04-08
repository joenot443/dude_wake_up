//
//  IconSourceControlsView.swift
//  NottawaApp
//
//  Icon picker grid for IconSource nodes.
//

import SwiftUI

/// Async thumbnail cache — loads and downscales icon images off the main thread.
@Observable
final class IconThumbnailCache {
    static let shared = IconThumbnailCache()

    private var cache: [String: NSImage] = [:]       // path → thumbnail
    private var loading: Set<String> = []             // paths currently being loaded
    private let thumbnailSize: CGFloat = 48

    func thumbnail(for path: String) -> NSImage? {
        cache[path]
    }

    func loadIfNeeded(_ path: String) {
        guard cache[path] == nil, !loading.contains(path) else { return }
        loading.insert(path)
        DispatchQueue.global(qos: .utility).async { [weak self] in
            guard let self else { return }
            let thumb = self.makeThumbnail(path: path)
            DispatchQueue.main.async {
                self.cache[path] = thumb
                self.loading.remove(path)
            }
        }
    }

    private func makeThumbnail(path: String) -> NSImage? {
        guard let source = NSImage(contentsOfFile: path) else { return nil }
        let size = NSSize(width: thumbnailSize, height: thumbnailSize)
        let thumb = NSImage(size: size)
        thumb.lockFocus()
        NSGraphicsContext.current?.imageInterpolation = .high
        let srcSize = source.size
        let scale = min(size.width / srcSize.width, size.height / srcSize.height)
        let drawSize = NSSize(width: srcSize.width * scale, height: srcSize.height * scale)
        let origin = NSPoint(x: (size.width - drawSize.width) / 2,
                             y: (size.height - drawSize.height) / 2)
        source.draw(in: NSRect(origin: origin, size: drawSize),
                    from: .zero, operation: .copy, fraction: 1.0)
        thumb.unlockFocus()
        return thumb
    }
}

/// A single icon tile that triggers async loading when it appears.
struct IconTileView: View {
    let icon: IconInfo
    let isSelected: Bool
    let onSelect: () -> Void

    @Environment(ThemeManager.self) private var theme
    @State private var cache = IconThumbnailCache.shared

    var body: some View {
        Button(action: onSelect) {
            ZStack {
                if let thumb = cache.thumbnail(for: icon.path) {
                    Image(nsImage: thumb)
                        .resizable()
                        .aspectRatio(contentMode: .fit)
                        .padding(4)
                } else {
                    ProgressView()
                        .scaleEffect(0.5)
                }
            }
            .frame(width: 48, height: 48)
            .background(theme.colors.surface)
            .clipShape(RoundedRectangle(cornerRadius: 6))
            .overlay(
                RoundedRectangle(cornerRadius: 6)
                    .stroke(isSelected ? theme.colors.accent : .clear, lineWidth: 2)
            )
        }
        .buttonStyle(.plain)
        .help(icon.name)
        .onAppear {
            cache.loadIfNeeded(icon.path)
        }
    }
}

struct IconSourceControlsView: View {
    @Environment(ThemeManager.self) private var theme
    let state: IconSourceState
    let sourceId: String

    @State private var searchText = ""
    @State private var selectedCategory: String? = nil

    private let engine = NottawaEngine.shared

    private var categories: [String] {
        Array(Set(state.icons.map(\.category))).sorted()
    }

    private var filteredIcons: [IconInfo] {
        state.icons.filter { icon in
            let matchesCategory = selectedCategory == nil || icon.category == selectedCategory
            let matchesSearch = searchText.isEmpty ||
                icon.name.localizedCaseInsensitiveContains(searchText)
            return matchesCategory && matchesSearch
        }
    }

    private let columns = [
        GridItem(.adaptive(minimum: 52, maximum: 60), spacing: 4)
    ]

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            // Category filter
            ScrollView(.horizontal, showsIndicators: false) {
                HStack(spacing: 4) {
                    categoryPill(nil, label: "All")
                    ForEach(categories, id: \.self) { cat in
                        categoryPill(cat, label: cat.capitalized)
                    }
                }
            }

            // Search
            TextField("Search icons…", text: $searchText)
                .textFieldStyle(.plain)
                .padding(6)
                .background(theme.colors.surface)
                .clipShape(RoundedRectangle(cornerRadius: 6))
                .font(.caption)

            // Icon grid
            ScrollView {
                LazyVGrid(columns: columns, spacing: 4) {
                    ForEach(filteredIcons) { icon in
                        IconTileView(
                            icon: icon,
                            isSelected: icon.id == state.selectedIndex,
                            onSelect: {
                                engine.setIconSourceIcon(sourceId: sourceId, iconIndex: icon.id)
                            }
                        )
                    }
                }
            }
            .frame(maxHeight: 200)
        }
        .padding(.horizontal, 16)
        .padding(.vertical, 8)
    }

    private func categoryPill(_ category: String?, label: String) -> some View {
        let isSelected = selectedCategory == category
        return Button {
            selectedCategory = category
        } label: {
            Text(label)
                .font(.caption2)
                .padding(.horizontal, 8)
                .padding(.vertical, 4)
                .background(isSelected ? theme.colors.accent : theme.colors.surface)
                .foregroundStyle(isSelected ? .white : theme.colors.textPrimary)
                .clipShape(Capsule())
        }
        .buttonStyle(.plain)
    }
}

//
//  RecentWorkspacesService.swift
//  NottawaApp
//
//  Tracks recently opened/saved workspaces for the File → Open Recent
//  submenu. Persists across app launches via security-scoped bookmarks
//  (sandbox-friendly, mirroring ScreenshotService's approach).
//

import Foundation
import SwiftUI

struct RecentWorkspace: Codable, Identifiable, Hashable {
    let id: UUID
    var name: String
    var path: String
    var bookmarkData: Data?
    var lastOpened: Date
}

@Observable
final class RecentWorkspacesService {
    static let shared = RecentWorkspacesService()

    private static let storageKey = "nottawa.recentWorkspaces"
    private static let maxEntries = 10

    private(set) var workspaces: [RecentWorkspace] = []

    private init() {
        load()
    }

    // MARK: - Public API

    /// Insert/update an entry for the given workspace URL. De-duplicates by
    /// absolute path and trims to the most recent `maxEntries`.
    func record(url: URL) {
        let path = url.path
        let name = url.deletingPathExtension().lastPathComponent
        let bookmark = makeBookmark(for: url)

        workspaces.removeAll { $0.path == path }
        workspaces.insert(
            RecentWorkspace(
                id: UUID(),
                name: name,
                path: path,
                bookmarkData: bookmark,
                lastOpened: Date()
            ),
            at: 0
        )
        if workspaces.count > Self.maxEntries {
            workspaces = Array(workspaces.prefix(Self.maxEntries))
        }
        persist()
    }

    /// Resolve an entry back to a usable URL. If the bookmark is stale it's
    /// regenerated and persisted. Callers must `startAccessingSecurityScopedResource()`
    /// on the returned URL before reading, and stop when done.
    func resolve(_ entry: RecentWorkspace) -> URL? {
        if let data = entry.bookmarkData {
            var stale = false
            do {
                let url = try URL(
                    resolvingBookmarkData: data,
                    options: [.withSecurityScope],
                    relativeTo: nil,
                    bookmarkDataIsStale: &stale
                )
                if stale, let fresh = makeBookmark(for: url) {
                    if let idx = workspaces.firstIndex(where: { $0.id == entry.id }) {
                        workspaces[idx].bookmarkData = fresh
                        persist()
                    }
                }
                return url
            } catch {
                NSLog("RecentWorkspacesService: failed to resolve bookmark: \(error)")
            }
        }
        // Fallback for entries without a bookmark, or when bookmark resolution
        // failed — try the raw path. The open will succeed iff the sandbox
        // still has access (e.g. same session).
        return URL(fileURLWithPath: entry.path)
    }

    func remove(id: UUID) {
        workspaces.removeAll { $0.id == id }
        persist()
    }

    func clear() {
        workspaces.removeAll()
        persist()
    }

    // MARK: - Persistence

    private func load() {
        guard let data = UserDefaults.standard.data(forKey: Self.storageKey) else { return }
        do {
            workspaces = try JSONDecoder().decode([RecentWorkspace].self, from: data)
        } catch {
            NSLog("RecentWorkspacesService: failed to decode stored list: \(error)")
            workspaces = []
        }
    }

    private func persist() {
        do {
            let data = try JSONEncoder().encode(workspaces)
            UserDefaults.standard.set(data, forKey: Self.storageKey)
        } catch {
            NSLog("RecentWorkspacesService: failed to encode list: \(error)")
        }
    }

    private func makeBookmark(for url: URL) -> Data? {
        do {
            return try url.bookmarkData(
                options: .withSecurityScope,
                includingResourceValuesForKeys: nil,
                relativeTo: nil
            )
        } catch {
            NSLog("RecentWorkspacesService: failed to create bookmark for \(url.path): \(error)")
            return nil
        }
    }
}

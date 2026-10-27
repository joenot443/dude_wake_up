//
//  ScreenshotService.swift
//  NottawaApp
//
//  Sandbox-friendly screenshot pipeline. First write triggers a one-shot
//  NSOpenPanel pre-pointed at ~/Desktop; subsequent writes reuse a stored
//  security-scoped bookmark and complete silently.
//

import AppKit
import CoreGraphics
import ImageIO
import SwiftUI
import UniformTypeIdentifiers

enum ScreenshotFormat: String, CaseIterable, Identifiable {
    case png
    case jpeg

    var id: String { rawValue }

    var fileExtension: String {
        switch self {
        case .png: return "png"
        case .jpeg: return "jpg"
        }
    }

    var utType: UTType {
        switch self {
        case .png: return .png
        case .jpeg: return .jpeg
        }
    }
}

enum ScreenshotMode {
    case quickSave        // Save to remembered folder; prompt if first run
    case forcePicker      // Always present picker (e.g. ⇧S)
    case clipboard        // Copy CGImage to NSPasteboard (no disk write)
}

enum ScreenshotResult {
    case saved(URL)
    case copied
    case cancelled
    case noTexture
    case writeFailed(String)
}

@Observable
final class ScreenshotService {
    static let shared = ScreenshotService()

    private static let bookmarkKey = "nottawa.screenshotFolderBookmark"

    /// Trigger value bumped whenever a capture succeeds — observed by
    /// `ScreenshotFlashOverlay` to fire its animation.
    var flashTick: Int = 0

    /// User-visible path to the current screenshot folder, or nil if not yet set.
    var currentFolderDisplayPath: String? {
        guard let url = resolveFolderURL(startAccessing: false) else { return nil }
        return url.path
    }

    /// The screenshots.format setting (its choice ids are these raw values),
    /// read from the engine directly since captures aren't on the main actor.
    var format: ScreenshotFormat {
        get {
            guard let c = ntw_get_setting_string("screenshots.format") else { return .png }
            defer { ntw_free_string(c) }
            return ScreenshotFormat(rawValue: String(cString: c)) ?? .png
        }
        set { ntw_set_setting_string("screenshots.format", newValue.rawValue) }
    }

    private init() {}

    // MARK: - Public capture

    @discardableResult
    func capture(connectableId: String, mode: ScreenshotMode) -> ScreenshotResult {
        guard let cgImage = TextureProvider.shared.snapshot(for: connectableId) else {
            return .noTexture
        }

        switch mode {
        case .clipboard:
            copyToClipboard(cgImage)
            bumpFlash()
            return .copied

        case .forcePicker:
            return saveWithPicker(cgImage)

        case .quickSave:
            if let folder = resolveFolderURL(startAccessing: true) {
                defer { folder.stopAccessingSecurityScopedResource() }
                return writeImage(cgImage, into: folder)
            }
            // First-run: prompt for folder
            return saveWithPicker(cgImage)
        }
    }

    /// Re-prompt the user for a folder (used by the Settings "Change…" button).
    @discardableResult
    func chooseFolder() -> URL? {
        guard let url = runFolderPicker() else { return nil }
        storeBookmark(for: url)
        return url
    }

    // MARK: - Folder picker

    /// Run picker, save bookmark, then write the image. Used for both first-run
    /// quickSave and explicit forcePicker requests.
    /// Forgets the folder; the next capture asks again.
    func clearFolder() {
        UserDefaults.standard.removeObject(forKey: Self.bookmarkKey)
    }

    private func saveWithPicker(_ image: CGImage) -> ScreenshotResult {
        guard let folder = runFolderPicker() else { return .cancelled }
        storeBookmark(for: folder)
        // Folder URL from NSOpenPanel is already authorized for this session,
        // no need to call startAccessingSecurityScopedResource here.
        return writeImage(image, into: folder)
    }

    private func runFolderPicker() -> URL? {
        let panel = NSOpenPanel()
        panel.canChooseFiles = false
        panel.canChooseDirectories = true
        panel.allowsMultipleSelection = false
        panel.canCreateDirectories = true
        panel.prompt = "Save Screenshots Here"
        panel.message = "Choose a folder where Nottawa should save your screenshots. You'll only need to do this once."
        panel.directoryURL = FileManager.default.urls(for: .desktopDirectory, in: .userDomainMask).first
        return panel.runModal() == .OK ? panel.url : nil
    }

    // MARK: - Bookmark persistence

    private func storeBookmark(for url: URL) {
        do {
            let data = try url.bookmarkData(
                options: .withSecurityScope,
                includingResourceValuesForKeys: nil,
                relativeTo: nil
            )
            UserDefaults.standard.set(data, forKey: Self.bookmarkKey)
        } catch {
            NSLog("ScreenshotService: failed to create bookmark: \(error)")
        }
    }

    /// Resolve the stored bookmark. If `startAccessing` is true, calls
    /// `startAccessingSecurityScopedResource()` — caller is responsible for
    /// `stopAccessingSecurityScopedResource()`.
    private func resolveFolderURL(startAccessing: Bool) -> URL? {
        guard let data = UserDefaults.standard.data(forKey: Self.bookmarkKey) else {
            return nil
        }
        do {
            var stale = false
            let url = try URL(
                resolvingBookmarkData: data,
                options: [.withSecurityScope],
                relativeTo: nil,
                bookmarkDataIsStale: &stale
            )
            if startAccessing {
                guard url.startAccessingSecurityScopedResource() else { return nil }
            }
            if stale {
                // Best-effort refresh; still return the URL we resolved.
                storeBookmark(for: url)
            }
            return url
        } catch {
            NSLog("ScreenshotService: failed to resolve bookmark: \(error)")
            UserDefaults.standard.removeObject(forKey: Self.bookmarkKey)
            return nil
        }
    }

    // MARK: - File write

    private func writeImage(_ image: CGImage, into folder: URL) -> ScreenshotResult {
        let format = self.format
        let filename = "nottawa_\(timestamp()).\(format.fileExtension)"
        let url = folder.appendingPathComponent(filename)

        guard let dest = CGImageDestinationCreateWithURL(
            url as CFURL,
            format.utType.identifier as CFString,
            1,
            nil
        ) else {
            return .writeFailed("Could not create image destination")
        }

        let properties: [CFString: Any] = format == .jpeg
            ? [kCGImageDestinationLossyCompressionQuality: 0.9]
            : [:]

        CGImageDestinationAddImage(dest, image, properties as CFDictionary)
        guard CGImageDestinationFinalize(dest) else {
            return .writeFailed("Failed to finalize image")
        }

        bumpFlash()
        return .saved(url)
    }

    private func timestamp() -> String {
        let f = DateFormatter()
        f.dateFormat = "yyyy-MM-dd_HH-mm-ss"
        return f.string(from: Date())
    }

    // MARK: - Clipboard

    private func copyToClipboard(_ image: CGImage) {
        let nsImage = NSImage(cgImage: image, size: NSSize(width: image.width, height: image.height))
        let pasteboard = NSPasteboard.general
        pasteboard.clearContents()
        pasteboard.writeObjects([nsImage])
    }

    // MARK: - Flash trigger

    private func bumpFlash() {
        DispatchQueue.main.async {
            self.flashTick &+= 1
        }
    }
}

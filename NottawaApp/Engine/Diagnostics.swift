//
//  Diagnostics.swift
//  NottawaApp
//
//  The Advanced › Copy Diagnostics text (advanced.copyDiagnostics): what a
//  bug report needs about this Mac, the engine and the settings, as plain
//  text. Nothing that names the user: no home paths, no user name.
//
//  The sandbox can't read ~/Library/Logs/DiagnosticReports, so whether the
//  previous session ended badly comes from a marker in the data folder,
//  written at launch and removed on a clean quit.
//

import AppKit
import Metal
import Security
import OpenGL.GL3

enum Diagnostics {
    // MARK: - Previous session

    private static var markerURL: URL { SettingsStore.dataFolder.appendingPathComponent("session-running") }

    /// When the previous session started, if it didn't quit cleanly (a crash,
    /// a hang that was force quit, or the Mac shutting down under it).
    private(set) static var uncleanPreviousSession: Date?

    /// At launch: notes how the last session ended, then marks this one running.
    static func noteLaunch() {
        let fm = FileManager.default
        if fm.fileExists(atPath: markerURL.path) {
            uncleanPreviousSession = (try? fm.attributesOfItem(atPath: markerURL.path)[.modificationDate]) as? Date ?? Date.distantPast
        }
        try? fm.createDirectory(at: SettingsStore.dataFolder, withIntermediateDirectories: true)
        fm.createFile(atPath: markerURL.path, contents: Data())
    }

    /// On a normal quit.
    static func noteCleanQuit() {
        try? FileManager.default.removeItem(at: markerURL)
    }

    // MARK: - Text

    static func text() -> String {
        let engine = NottawaEngine.shared
        let info = Bundle.main.infoDictionary ?? [:]
        let version = info["CFBundleShortVersionString"] as? String ?? "?"
        let build = info["CFBundleVersion"] as? String ?? "?"
        var lines: [String] = []

        lines.append("Nottawa \(version) (\(build)) (macOS, \(channel))")

        let os = ProcessInfo.processInfo.operatingSystemVersionString
        lines.append("System: macOS \(os), \(architecture), \(sysctlString("hw.model") ?? "unknown model"), \(ProcessInfo.processInfo.physicalMemory >> 30) GB")

        lines.append("Graphics: \(openGLDescription()); Metal: \(MTLCreateSystemDefaultDevice()?.name ?? "none")")

        for (i, screen) in NSScreen.screens.enumerated() {
            let size = screen.frame.size
            let scale = screen.backingScaleFactor
            var line = "Display \(i + 1): \(screen.localizedName), \(Int(size.width))x\(Int(size.height)) pt @\(Int(scale))x, \(screen.maximumFramesPerSecond) Hz"
            if screen == NSScreen.screens.first { line += " (main)" }
            lines.append(line)
        }

        let nodes = engine.allVideoSources().count + engine.allShaders().count
        lines.append(String(format: "Engine: %.0fx%.0f, %.1f fps, %d nodes", engine.resolutionWidth, engine.resolutionHeight, engine.engineFPS, nodes))
        let inputName = engine.allAudioSources().first { $0.id == engine.selectedAudioSourceId }?.name ?? "none"
        lines.append("Audio: \(inputName), \(engine.isAudioActive ? "on" : "off")")

        let store = SettingsStore.shared
        let changed = store.schema.compactMap { s -> String? in
            switch s.kind {
            case .bool, .int, .float:
                let v = store.number(s.id)
                guard v != s.defaultNumber else { return nil }
                return s.kind == .bool ? "\(s.id)=\(v != 0)" : "\(s.id)=\(v.formatted())"
            case .choice, .string:
                let v = store.string(s.id)
                return v == s.defaultString ? nil : "\(s.id)=\(v)"
            case .path, .action:
                return nil  // a folder can name the user
            }
        }
        lines.append("Settings changed: \(changed.isEmpty ? "none" : changed.joined(separator: ", "))")

        if let started = uncleanPreviousSession {
            let when = started == .distantPast ? "unknown time" : started.formatted(date: .abbreviated, time: .shortened)
            lines.append("Previous session: didn't quit normally (started \(when)); a crash report may be in Advanced › Open Logs Folder")
        } else {
            lines.append("Previous session: quit normally")
        }
        return lines.joined(separator: "\n")
    }

    static func copyToPasteboard() {
        NSPasteboard.general.clearContents()
        NSPasteboard.general.setString(text(), forType: .string)
    }

    // MARK: - Pieces

    /// From how the app is signed (only the certificate's kind is read: its
    /// full name names the developer).
    private static var channel: String {
        var code: SecCode?
        var staticCode: SecStaticCode?
        var signing: CFDictionary?
        guard SecCodeCopySelf([], &code) == errSecSuccess, let code,
              SecCodeCopyStaticCode(code, [], &staticCode) == errSecSuccess, let staticCode,
              SecCodeCopySigningInformation(staticCode, SecCSFlags(rawValue: kSecCSSigningInformation), &signing) == errSecSuccess,
              let certificates = (signing as? [String: Any])?[kSecCodeInfoCertificates as String] as? [SecCertificate],
              let leaf = certificates.first,
              let summary = SecCertificateCopySubjectSummary(leaf) as String? else {
            return "unsigned build"
        }
        if summary.hasPrefix("Developer ID Application") { return "direct download" }
        if summary.hasPrefix("Apple Distribution") || summary.hasPrefix("3rd Party Mac Developer") { return "Mac App Store" }
        return "development build"
    }

    private static var architecture: String {
        #if arch(arm64)
        return "arm64"
        #else
        return "x86_64"
        #endif
    }

    private static func sysctlString(_ name: String) -> String? {
        var size = 0
        guard sysctlbyname(name, nil, &size, nil, 0) == 0, size > 0 else { return nil }
        var buffer = [CChar](repeating: 0, count: size)
        guard sysctlbyname(name, &buffer, &size, nil, 0) == 0 else { return nil }
        return String(cString: buffer)
    }

    /// GL_RENDERER and GL_VERSION, from a throwaway 3.2 Core context like the
    /// engine's (the engine's own is current on its thread, not this one).
    private static func openGLDescription() -> String {
        let attributes: [CGLPixelFormatAttribute] = [
            kCGLPFAOpenGLProfile, CGLPixelFormatAttribute(UInt32(kCGLOGLPVersion_3_2_Core.rawValue)),
            kCGLPFAAccelerated, CGLPixelFormatAttribute(0),
        ]
        var format: CGLPixelFormatObj?
        var count: GLint = 0
        guard CGLChoosePixelFormat(attributes, &format, &count) == kCGLNoError, let format else { return "OpenGL unavailable" }
        defer { CGLDestroyPixelFormat(format) }
        var context: CGLContextObj?
        guard CGLCreateContext(format, nil, &context) == kCGLNoError, let context else { return "OpenGL unavailable" }
        let previous = CGLGetCurrentContext()
        CGLSetCurrentContext(context)
        defer {
            CGLSetCurrentContext(previous)
            CGLDestroyContext(context)
        }
        let renderer = glGetString(GLenum(GL_RENDERER)).map { String(cString: $0) } ?? "?"
        let vendor = glGetString(GLenum(GL_VENDOR)).map { String(cString: $0) } ?? "?"
        let version = glGetString(GLenum(GL_VERSION)).map { String(cString: $0) } ?? "?"
        return "\(renderer) (\(vendor)), OpenGL \(version)"
    }
}

//
//  PreviewWindowManager.swift
//  NottawaApp
//
//  Manages separate macOS windows for node previews.
//

import AppKit
import SwiftUI

final class PreviewWindowManager {
    static let shared = PreviewWindowManager()

    private var windows: [String: NSWindow] = [:]
    var stretchToFill: Bool = false

    /// IDs of nodes with open preview windows.
    var openNodeIds: Set<String> { Set(windows.keys) }

    private init() {}

    func openPreview(for nodeId: String, nodeName: String) {
        // If already open, bring to front
        if let existing = windows[nodeId] {
            existing.makeKeyAndOrderFront(nil)
            return
        }

        // Query texture dimensions for correct aspect ratio
        let engine = NottawaEngine.shared
        let texW = engine.textureWidth(connectableId: nodeId)
        let texH = engine.textureHeight(connectableId: nodeId)
        let aspect = texW > 0 && texH > 0 ? CGFloat(texW) / CGFloat(texH) : 16.0 / 9.0

        let windowWidth: CGFloat = 640
        let windowHeight = round(windowWidth / aspect)

        let previewView = PreviewWindowContentView(
            connectableId: nodeId, aspect: aspect, stretchToFill: stretchToFill
        )
        let hostingView = NSHostingView(rootView: previewView)

        let window = PreviewWindow(
            contentRect: NSRect(x: 0, y: 0, width: windowWidth, height: windowHeight),
            styleMask: [.titled, .closable, .miniaturizable, .resizable],
            backing: .buffered,
            defer: false
        )
        window.title = "\(nodeName) – Preview"
        if !stretchToFill {
            window.contentAspectRatio = NSSize(width: aspect, height: 1.0)
        }
        window.contentView = hostingView
        window.center()
        window.isReleasedWhenClosed = false
        window.makeKeyAndOrderFront(nil)

        windows[nodeId] = window

        NotificationCenter.default.addObserver(
            forName: NSWindow.willCloseNotification,
            object: window,
            queue: .main
        ) { [weak self] notification in
            guard let closedWindow = notification.object as? NSWindow else { return }
            self?.windows = self?.windows.filter { $0.value !== closedWindow } ?? [:]
        }
    }

    func closePreview(for nodeId: String) {
        windows[nodeId]?.close()
        windows.removeValue(forKey: nodeId)
    }

    func closeAll() {
        for (_, window) in windows {
            window.close()
        }
        windows.removeAll()
    }

    /// Toggle between stretch-to-fill and aspect-fit modes for all preview windows.
    func setStretchToFill(_ stretch: Bool) {
        stretchToFill = stretch
        let engine = NottawaEngine.shared
        let texW = CGFloat(engine.resolutionWidth)
        let texH = CGFloat(engine.resolutionHeight)
        let aspect = texW > 0 && texH > 0 ? texW / texH : 16.0 / 9.0

        for (nodeId, window) in windows {
            if stretch {
                // Remove aspect ratio constraint so the window can be freely resized
                window.contentAspectRatio = NSSize(width: 0, height: 0)
            } else {
                window.contentAspectRatio = NSSize(width: aspect, height: 1.0)
                // Snap window to correct aspect ratio
                let currentWidth = window.frame.width
                let titleBarHeight = window.frame.height - window.contentLayoutRect.height
                let newHeight = round(currentWidth / aspect) + titleBarHeight
                let newFrame = NSRect(
                    x: window.frame.origin.x,
                    y: window.frame.origin.y + window.frame.height - newHeight,
                    width: currentWidth,
                    height: newHeight
                )
                window.setFrame(newFrame, display: true, animate: true)
            }

            let previewView = PreviewWindowContentView(
                connectableId: nodeId, aspect: aspect, stretchToFill: stretch
            )
            window.contentView = NSHostingView(rootView: previewView)
        }
    }

    /// Update all open preview windows to match the current resolution aspect ratio.
    func updateWindowSizes() {
        let engine = NottawaEngine.shared
        let texW = CGFloat(engine.resolutionWidth)
        let texH = CGFloat(engine.resolutionHeight)
        guard texW > 0, texH > 0 else { return }
        let aspect = texW / texH

        for (nodeId, window) in windows {
            if !stretchToFill {
                window.contentAspectRatio = NSSize(width: aspect, height: 1.0)

                // Resize keeping the current window width
                let currentWidth = window.frame.width
                let titleBarHeight = window.frame.height - window.contentLayoutRect.height
                let newHeight = round(currentWidth / aspect) + titleBarHeight
                let newFrame = NSRect(
                    x: window.frame.origin.x,
                    y: window.frame.origin.y + window.frame.height - newHeight,
                    width: currentWidth,
                    height: newHeight
                )
                window.setFrame(newFrame, display: true, animate: true)
            }

            let previewView = PreviewWindowContentView(
                connectableId: nodeId, aspect: aspect, stretchToFill: stretchToFill
            )
            window.contentView = NSHostingView(rootView: previewView)
        }
    }
}

// MARK: - Preview Window (Esc to close)

final class PreviewWindow: NSWindow {
    override func cancelOperation(_ sender: Any?) {
        close()
    }

    override var canBecomeKey: Bool { true }
    override var canBecomeMain: Bool { true }
}

// MARK: - Preview Window Content

struct PreviewWindowContentView: View {
    let connectableId: String
    let aspect: CGFloat
    let stretchToFill: Bool

    @State private var shiftHeld = false
    @State private var fpsCounter = FPSCounter()

    var body: some View {
        Group {
            if stretchToFill {
                TextureDisplayView(connectableId: connectableId)
            } else {
                TextureDisplayView(connectableId: connectableId)
                    .aspectRatio(aspect, contentMode: .fit)
            }
        }
        .frame(minWidth: 240, maxWidth: .infinity, minHeight: 135, maxHeight: .infinity)
        .background(.black)
        .overlay(alignment: .topLeading) {
            if shiftHeld {
                TimelineView(.animation(minimumInterval: nil, paused: false)) { _ in
                    let uiFPS = fpsCounter.tick()
                    let engineFPS = NottawaEngine.shared.engineFPS
                    FPSOverlayView(engineFPS: Int(engineFPS), uiFPS: Int(uiFPS))
                        .padding(8)
                }
            }
        }
        .onAppear { installMonitors() }
    }

    private func installMonitors() {
        NSEvent.addLocalMonitorForEvents(matching: .flagsChanged) { event in
            shiftHeld = event.modifierFlags.contains(.shift)
            return event
        }
    }
}

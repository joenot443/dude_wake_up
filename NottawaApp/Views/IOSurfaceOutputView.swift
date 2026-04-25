//
//  IOSurfaceOutputView.swift
//  NottawaApp
//
//  Lightweight output view that displays an IOSurface directly via CALayer.
//  No Metal pipeline — the window server composites the IOSurface contents
//  directly, eliminating the GPU overhead of a full Metal render pass.
//

import AppKit
import SwiftUI
import IOSurface

// MARK: - NSView that displays an IOSurface via CALayer

final class IOSurfaceLayerView: NSView {
    var connectableId: String {
        didSet {
            currentSurface = nil
            updateSurface()
        }
    }

    private var displayLink: CVDisplayLink?
    private var currentSurface: IOSurfaceRef?

    init(connectableId: String) {
        self.connectableId = connectableId
        super.init(frame: .zero)
        wantsLayer = true
        layer?.contentsGravity = .resizeAspect
        layer?.backgroundColor = NSColor.black.cgColor
        startDisplayLink()
    }

    required init?(coder: NSCoder) {
        fatalError("init(coder:) not implemented")
    }

    override var wantsUpdateLayer: Bool { true }

    override func updateLayer() {
        updateSurface()
    }

    private func updateSurface() {
        guard let surface = NottawaEngine.shared.connectableIOSurface(connectableId: connectableId) else {
            return
        }
        currentSurface = surface
        // Force CALayer to re-read IOSurface pixels by toggling contents.
        // CALayer caches the pointer and skips recomposite if it hasn't changed.
        layer?.contents = nil
        layer?.contents = surface
    }

    // MARK: - Display Link

    private func startDisplayLink() {
        var dl: CVDisplayLink?
        CVDisplayLinkCreateWithActiveCGDisplays(&dl)
        guard let dl else { return }
        displayLink = dl

        CVDisplayLinkSetOutputHandler(dl) { [weak self] _, _, _, _, _ in
            DispatchQueue.main.async {
                self?.updateSurface()
            }
            return kCVReturnSuccess
        }
        CVDisplayLinkStart(dl)
    }

    private func stopDisplayLink() {
        if let dl = displayLink {
            CVDisplayLinkStop(dl)
            displayLink = nil
        }
    }

    deinit {
        stopDisplayLink()
    }
}

// MARK: - SwiftUI Wrapper

struct IOSurfaceOutputView: NSViewRepresentable {
    let connectableId: String

    func makeNSView(context: Context) -> IOSurfaceLayerView {
        IOSurfaceLayerView(connectableId: connectableId)
    }

    func updateNSView(_ nsView: IOSurfaceLayerView, context: Context) {
        nsView.connectableId = connectableId
    }
}

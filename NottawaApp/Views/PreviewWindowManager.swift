//
//  PreviewWindowManager.swift
//  NottawaApp
//
//  Manages separate macOS windows for node previews (a.k.a. "output windows"
//  — the floating windows sent to projectors during a live show).
//

import AppKit
import SwiftUI

/// Per-window state driving the content view. Stored as an `@Observable`
/// class so the NSHostingView's SwiftUI tree can react to advance/aspect/
/// stretch changes *without* the manager replacing the hosting view (which
/// would deallocate the IOSurfaceLayerView mid-render and ruin any
/// crossfade). The window keeps one hosting view for its entire lifetime;
/// only this state mutates.
@Observable
final class PreviewWindowState {
    /// Optional so the window can fade to black during a delete-retreat
    /// where the deleted node has no upstream to fall back to.
    var currentConnectableId: String?
    var currentNodeName: String
    var aspect: CGFloat
    var stretchToFill: Bool

    /// Set during an auto-advance crossfade: the previous connectable's id,
    /// rendered on top of the current one with a fading `outgoingOpacity`.
    /// Cleared to `nil` once the fade animation completes.
    var outgoingConnectableId: String? = nil
    var outgoingOpacity: Double = 0

    init(connectableId: String, nodeName: String, aspect: CGFloat, stretchToFill: Bool) {
        self.currentConnectableId = connectableId
        self.currentNodeName = nodeName
        self.aspect = aspect
        self.stretchToFill = stretchToFill
    }
}

final class PreviewWindowManager {
    static let shared = PreviewWindowManager()

    private var windows: [String: NSWindow] = [:]
    private var states: [String: PreviewWindowState] = [:]
    var stretchToFill: Bool = false

    /// IDs of nodes with open preview windows.
    var openNodeIds: Set<String> { Set(windows.keys) }

    /// Reverse lookup: which preview window owns this NSWindow (if any).
    func nodeId(for window: NSWindow) -> String? {
        windows.first(where: { $0.value === window })?.key
    }

    private init() {
        installKeyMonitor()
    }

    private var closeObservers: [String: Any] = [:]
    /// Each window's frame from before it went full screen, for reopening it.
    private var normalFrames: [ObjectIdentifier: NSRect] = [:]
    private var fullScreenObservers: [ObjectIdentifier: Any] = [:]

    /// App-wide key monitor that dispatches hotkeys when an output window
    /// is key. Installed once at init.
    private func installKeyMonitor() {
        NSEvent.addLocalMonitorForEvents(matching: .keyDown) { [weak self] event in
            guard let self,
                  let window = NSApp.keyWindow,
                  let connectableId = self.nodeId(for: window) else {
                return event
            }
            let flags = event.modifierFlags.intersection(.deviceIndependentFlagsMask)

            // Ctrl+Cmd+F → toggle fullscreen on the focused output window.
            // Returning nil consumes the event so no menu/system shortcut
            // double-fires it.
            if flags == [.control, .command],
               event.charactersIgnoringModifiers?.lowercased() == "f" {
                window.toggleFullScreen(nil)
                return nil
            }

            // Screenshot hotkeys only handle bare letters and shift —
            // leave anything with cmd/ctrl/option alone.
            let allowed: NSEvent.ModifierFlags = [.shift]
            guard flags.subtracting(allowed).isEmpty else { return event }

            switch event.charactersIgnoringModifiers?.lowercased() {
            case "s":
                let mode: ScreenshotMode = flags.contains(.shift) ? .forcePicker : .quickSave
                ScreenshotService.shared.capture(connectableId: connectableId, mode: mode)
                return nil
            case "c":
                // Plain c only — Cmd+C is system copy.
                guard !flags.contains(.shift) else { return event }
                ScreenshotService.shared.capture(connectableId: connectableId, mode: .clipboard)
                return nil
            default:
                return event
            }
        }
    }

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

        let state = PreviewWindowState(
            connectableId: nodeId,
            nodeName: nodeName,
            aspect: aspect,
            stretchToFill: stretchToFill
        )
        let previewView = PreviewWindowContentView(state: state)
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
        // Allow native fullscreen so Ctrl+Cmd+F (and the green window button)
        // can take the output to the whole display — useful for live shows
        // running the output to a projector / secondary screen.
        window.collectionBehavior.insert(.fullScreenPrimary)
        window.contentView = hostingView
        window.center()
        window.isReleasedWhenClosed = false
        window.makeKeyAndOrderFront(nil)

        windows[nodeId] = window
        states[nodeId] = state
        fullScreenObservers[ObjectIdentifier(window)] = NotificationCenter.default.addObserver(
            forName: NSWindow.willEnterFullScreenNotification, object: window, queue: .main
        ) { [weak self] note in
            guard let w = note.object as? NSWindow else { return }
            self?.normalFrames[ObjectIdentifier(w)] = w.frame
        }

        let observer = NotificationCenter.default.addObserver(
            forName: NSWindow.willCloseNotification,
            object: window,
            queue: .main
        ) { [weak self] notification in
            guard let self,
                  let closedWindow = notification.object as? NSWindow else { return }
            // Tear down the hosting view to stop display links
            closedWindow.contentView = nil
            self.normalFrames.removeValue(forKey: ObjectIdentifier(closedWindow))
            if let obs = self.fullScreenObservers.removeValue(forKey: ObjectIdentifier(closedWindow)) {
                NotificationCenter.default.removeObserver(obs)
            }
            // Remove from tracking
            if let closedId = self.windows.first(where: { $0.value === closedWindow })?.key {
                self.windows.removeValue(forKey: closedId)
                self.states.removeValue(forKey: closedId)
                if let obs = self.closeObservers.removeValue(forKey: closedId) {
                    NotificationCenter.default.removeObserver(obs)
                }
            }
        }
        closeObservers[nodeId] = observer
    }

    /// Move an existing preview window from one connectable to another by
    /// mutating its state — the NSHostingView and inner SwiftUI tree are
    /// preserved, so the two `IOSurfaceLayerView`s for the old and new feeds
    /// can co-exist briefly and crossfade. Used by the "Automatically advance
    /// output window" setting.
    @discardableResult
    func advanceWindow(from oldNodeId: String, to newNodeId: String, newNodeName: String, duration: Double) -> Bool {
        guard oldNodeId != newNodeId else { return false }
        guard let window = windows[oldNodeId], let state = states[oldNodeId] else { return false }
        // If the destination already has its own window, leave the original
        // alone — don't collapse two windows into one.
        guard windows[newNodeId] == nil else { return false }

        // Rekey: window + state + close observer all move from oldId → newId.
        windows.removeValue(forKey: oldNodeId)
        states.removeValue(forKey: oldNodeId)
        let observer = closeObservers.removeValue(forKey: oldNodeId)
        windows[newNodeId] = window
        states[newNodeId] = state
        if let observer { closeObservers[newNodeId] = observer }

        let engine = NottawaEngine.shared
        let texW = engine.textureWidth(connectableId: newNodeId)
        let texH = engine.textureHeight(connectableId: newNodeId)
        let newAspect = texW > 0 && texH > 0 ? CGFloat(texW) / CGFloat(texH) : 16.0 / 9.0

        window.title = "\(newNodeName) – Preview"
        if !stretchToFill {
            window.contentAspectRatio = NSSize(width: newAspect, height: 1.0)
        }

        let fadeDuration = max(0, duration)

        // Tick 1 (now, synchronous): introduce the outgoing view fully
        // opaque and swap the current feed to the new one. We need SwiftUI
        // to actually *render* once in this state — otherwise body sees the
        // final post-animation value and there's no "from" frame for the
        // fade to interpolate from.
        state.outgoingConnectableId = state.currentConnectableId
        state.outgoingOpacity = 1.0
        state.currentConnectableId = newNodeId
        state.currentNodeName = newNodeName
        state.aspect = newAspect

        // Tick 2 (next runloop iteration): SwiftUI has now drawn the
        // outgoing view at opacity 1.0. Start the animation that fades it
        // back to 0.
        DispatchQueue.main.async { [weak state] in
            guard let state else { return }
            withAnimation(.easeInOut(duration: fadeDuration)) {
                state.outgoingOpacity = 0.0
            }
            // Clear the outgoing connectable once the animation has had
            // time to finish so its IOSurfaceLayerView deallocates and
            // stops pulling frames. Guarded against a newer advance that
            // may have re-armed `outgoingOpacity` to 1.0 in the meantime.
            DispatchQueue.main.asyncAfter(deadline: .now() + fadeDuration + 0.05) { [weak state] in
                guard let state else { return }
                if state.outgoingOpacity <= 0.001 {
                    state.outgoingConnectableId = nil
                }
            }
        }

        return true
    }

    /// Inverse of `advanceWindow`: triggered when a windowed node is about
    /// to be deleted. The deleted node's last live frames fade out on top
    /// while the fallback (typically the upstream node) fades in
    /// underneath. If `fallbackId` is `nil` (e.g. deleting a source with
    /// no upstream), the window fades to black and closes once the
    /// animation completes.
    ///
    /// The caller (NodeEditorViewModel) is responsible for keeping the
    /// engine's connectable + its IOSurface alive for `duration + a small
    /// buffer` — otherwise the outgoing layer would lose its source mid-
    /// fade.
    @discardableResult
    func retreatWindow(
        deletedNodeId: String,
        fallbackId: String?,
        fallbackName: String?,
        duration: Double
    ) -> Bool {
        guard let window = windows[deletedNodeId],
              let state = states[deletedNodeId] else { return false }
        // If the fallback already has its own window, abort — we'd otherwise
        // collide two windows on the same connectable. The caller will fall
        // back to instant delete.
        if let fallback = fallbackId,
           fallback != deletedNodeId,
           windows[fallback] != nil {
            return false
        }

        let fadeDuration = max(0, duration)
        let aspect: CGFloat

        // Rekey the dicts. If there's a fallback we hand them over to it;
        // otherwise we keep them under `deletedNodeId` until we close the
        // window at the end of the fade.
        windows.removeValue(forKey: deletedNodeId)
        states.removeValue(forKey: deletedNodeId)
        let observer = closeObservers.removeValue(forKey: deletedNodeId)

        let newKey: String
        if let fallback = fallbackId {
            newKey = fallback
            windows[fallback] = window
            states[fallback] = state
            if let observer { closeObservers[fallback] = observer }

            let engine = NottawaEngine.shared
            let texW = engine.textureWidth(connectableId: fallback)
            let texH = engine.textureHeight(connectableId: fallback)
            aspect = texW > 0 && texH > 0 ? CGFloat(texW) / CGFloat(texH) : state.aspect

            window.title = "\(fallbackName ?? "Preview") – Preview"
            if !stretchToFill {
                window.contentAspectRatio = NSSize(width: aspect, height: 1.0)
            }
        } else {
            newKey = deletedNodeId
            windows[deletedNodeId] = window
            states[deletedNodeId] = state
            if let observer { closeObservers[deletedNodeId] = observer }
            aspect = state.aspect
        }

        // Tick 1: snapshot current → outgoing, swap current to fallback
        // (which may be nil → black background bleeds through).
        state.outgoingConnectableId = state.currentConnectableId
        state.outgoingOpacity = 1.0
        state.currentConnectableId = fallbackId
        if let fallbackName { state.currentNodeName = fallbackName }
        state.aspect = aspect

        let needsCloseAtEnd = (fallbackId == nil)

        // Tick 2: animate opacity 1 → 0 (same pattern as advanceWindow).
        DispatchQueue.main.async { [weak self, weak state] in
            guard let state else { return }
            withAnimation(.easeInOut(duration: fadeDuration)) {
                state.outgoingOpacity = 0.0
            }
            DispatchQueue.main.asyncAfter(deadline: .now() + fadeDuration + 0.05) { [weak self, weak state] in
                guard let state else { return }
                if state.outgoingOpacity <= 0.001 {
                    state.outgoingConnectableId = nil
                    if needsCloseAtEnd {
                        self?.closePreview(for: newKey)
                    }
                }
            }
        }

        return true
    }

    func closePreview(for nodeId: String) {
        windows[nodeId]?.contentView = nil
        windows[nodeId]?.close()
        windows.removeValue(forKey: nodeId)
        states.removeValue(forKey: nodeId)
        if let obs = closeObservers.removeValue(forKey: nodeId) {
            NotificationCenter.default.removeObserver(obs)
        }
    }

    func closeAll() {
        for (_, window) in windows {
            window.contentView = nil
            window.close()
        }
        windows.removeAll()
        states.removeAll()
        for (_, obs) in closeObservers {
            NotificationCenter.default.removeObserver(obs)
        }
        closeObservers.removeAll()
    }

    // MARK: - Reopening at launch (the output.reopenWindows setting)

    /// An output window as it was at quit.
    struct SavedWindow: Codable, Equatable {
        var nodeId: String
        var screenName: String?
        var frame: CGRect      // the normal (not full screen) frame
        var fullScreen: Bool
    }

    private static let savedKey = "nottawa.outputWindows"

    /// The open output windows, recorded at quit (whether or not the
    /// setting is on, so turning it on later has something to reopen).
    func saveOpenWindows() {
        let saved = windows.map { nodeId, window -> SavedWindow in
            let fullScreen = window.styleMask.contains(.fullScreen)
            return SavedWindow(nodeId: nodeId, screenName: window.screen?.localizedName,
                               frame: fullScreen ? (normalFrames[ObjectIdentifier(window)] ?? window.frame) : window.frame,
                               fullScreen: fullScreen)
        }
        UserDefaults.standard.set(try? JSONEncoder().encode(saved), forKey: Self.savedKey)
    }

    var savedWindows: [SavedWindow] {
        guard let data = UserDefaults.standard.data(forKey: Self.savedKey) else { return [] }
        return (try? JSONDecoder().decode([SavedWindow].self, from: data)) ?? []
    }

    /// Reopens saved windows whose nodes are in the session: on the same
    /// display where it's connected (full screen if they were), otherwise
    /// windowed where they open. Returns the node ids reopened.
    @discardableResult
    func reopen(_ saved: [SavedWindow], nodeName: (String) -> String?) -> [String] {
        var opened: [String] = []
        for entry in saved {
            guard let name = nodeName(entry.nodeId) else { continue }
            openPreview(for: entry.nodeId, nodeName: name)
            guard let window = windows[entry.nodeId] else { continue }
            opened.append(entry.nodeId)
            // A display that isn't connected (a projector left unplugged): stay where it opened.
            guard let screen = NSScreen.screens.first(where: { $0.localizedName == entry.screenName }) else { continue }
            if entry.frame.intersects(screen.frame) {
                window.setFrame(entry.frame, display: true)
            } else {
                let visible = screen.visibleFrame
                window.setFrame(NSRect(x: visible.midX - entry.frame.width / 2, y: visible.midY - entry.frame.height / 2,
                                       width: entry.frame.width, height: entry.frame.height), display: true)
            }
            if entry.fullScreen && !window.styleMask.contains(.fullScreen) {
                // After the frame has moved it to its display.
                DispatchQueue.main.async { window.toggleFullScreen(nil) }
            }
        }
        return opened
    }

    /// Frame of the window showing a node (for tests), nil if none.
    func frame(for nodeId: String) -> NSRect? { windows[nodeId]?.frame }

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

            // Mutate state instead of recreating the hosting view — keeps
            // any in-flight crossfade alive.
            if let state = states[nodeId] {
                state.stretchToFill = stretch
                state.aspect = aspect
            }
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

            states[nodeId]?.aspect = aspect
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
    let state: PreviewWindowState

    @State private var shiftHeld = false
    @State private var fpsCounter = FPSCounter()

    var body: some View {
        ZStack {
            // Current feed — always fully opaque at the bottom of the stack.
            // Nil during a delete-retreat with no upstream fallback; the
            // window's `.background(.black)` shows through in that case.
            if let current = state.currentConnectableId {
                surface(connectableId: current)
            }

            // Outgoing feed — only present during a crossfade. Rendered on
            // top of the current feed, opacity animates 1 → 0 so the new
            // feed (or black, if current is nil) bleeds through.
            // `allowsHitTesting(false)` keeps screenshot hotkeys / focus
            // directed at the current feed underneath.
            if let outgoing = state.outgoingConnectableId {
                surface(connectableId: outgoing)
                    .opacity(state.outgoingOpacity)
                    .allowsHitTesting(false)
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
        .overlay(ScreenshotFlashOverlay())
        .onAppear { installMonitors() }
    }

    @ViewBuilder
    private func surface(connectableId: String) -> some View {
        if state.stretchToFill {
            IOSurfaceOutputView(connectableId: connectableId)
        } else {
            IOSurfaceOutputView(connectableId: connectableId)
                .aspectRatio(state.aspect, contentMode: .fit)
        }
    }

    private func installMonitors() {
        NSEvent.addLocalMonitorForEvents(matching: .flagsChanged) { event in
            shiftHeld = event.modifierFlags.contains(.shift)
            return event
        }
    }
}

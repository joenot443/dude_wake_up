//
//  MainEditorView.swift
//  NottawaApp
//
//  Top-level editor layout: toolbar + canvas + inspector.
//

import SwiftUI

struct MainEditorView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel
    @Environment(ThemeManager.self) private var theme
    @State private var shiftHeld = false
    @State private var fpsCounter = FPSCounter()
    @State private var canvasSize: CGSize = CGSize(width: 800, height: 600)

    var body: some View {
        VStack(spacing: 0) {
            ToolbarView()

            theme.colors.border.frame(height: 1)

            HStack(spacing: 0) {
                // Sidebar (left)
                if viewModel.showSidebar {
                    SidebarBrowserView()
                        .transition(.move(edge: .leading))
                    PanelResizeHandle(
                        width: Binding(
                            get: { viewModel.sidebarWidth },
                            set: { viewModel.sidebarWidth = $0 }
                        ),
                        minWidth: 180,
                        maxWidth: 500,
                        invertDrag: false
                    )
                }

                // Canvas (center)
                if viewModel.stageModeEnabled {
                    StageModeView()
                        .frame(maxWidth: .infinity, maxHeight: .infinity)
                        .transition(.opacity)
                } else {
                    NodeEditorView()
                        .frame(maxWidth: .infinity, maxHeight: .infinity)
                        .background {
                            GeometryReader { geo in
                                Color.clear.onAppear { canvasSize = geo.size }
                                    .onChange(of: geo.size) { _, newSize in canvasSize = newSize }
                            }
                        }
                        .overlay(alignment: .bottomTrailing) {
                            ActionBarView(canvasSize: canvasSize)
                                .padding(16)
                        }
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
                        .overlay {
                            // Floating browser for swap mode only
                            if viewModel.browserMode == .swap &&
                               (viewModel.showShaderBrowser || viewModel.showSourceBrowser) {
                                GeometryReader { geo in
                                    Group {
                                        if viewModel.showShaderBrowser {
                                            ShaderBrowserView()
                                        } else {
                                            VideoSourceBrowserView()
                                        }
                                    }
                                    .frame(width: 500, height: 420)
                                    .background(theme.colors.backgroundSecondary, in: RoundedRectangle(cornerRadius: 12))
                                    .clipShape(RoundedRectangle(cornerRadius: 12))
                                    .shadow(color: .black.opacity(0.4), radius: 12, y: 4)
                                    .position(clampedBrowserPosition(in: geo.size))
                                }
                            }
                        }
                        .transition(.opacity)
                }

                // Inspector (right) — hidden in stage mode
                if !viewModel.selectedNodeIds.isEmpty && !viewModel.stageModeEnabled {
                    PanelResizeHandle(
                        width: Binding(
                            get: { viewModel.inspectorWidth },
                            set: { viewModel.inspectorWidth = $0 }
                        ),
                        minWidth: 250,
                        maxWidth: 500,
                        invertDrag: true
                    )
                    ParameterInspectorView()
                        .transition(.move(edge: .trailing))
                }
            }
            .animation(.easeInOut(duration: 0.2), value: viewModel.showSidebar)
            .animation(.easeInOut(duration: 0.2), value: viewModel.selectedNodeIds.isEmpty)
            .animation(.easeInOut(duration: 0.3), value: viewModel.stageModeEnabled)

            // Audio Panel (bottom)
            if viewModel.showAudioPanel {
                VerticalPanelResizeHandle(
                    height: Binding(
                        get: { viewModel.audioPanelHeight },
                        set: { viewModel.audioPanelHeight = $0 }
                    ),
                    minHeight: 150,
                    maxHeight: 400
                )
                AudioPanelView()
                    .frame(height: viewModel.audioPanelHeight)
                    .transition(.move(edge: .bottom))
            }
        }
        .background(theme.colors.background)
        .animation(.easeInOut(duration: 0.2), value: viewModel.showAudioPanel)
        .sheet(isPresented: Binding(
            get: { viewModel.showShareSheet },
            set: { viewModel.showShareSheet = $0 }
        )) {
            ShareStrandSheet()
        }
        .onAppear { installModifierMonitor() }
    }

    private func clampedBrowserPosition(in containerSize: CGSize) -> CGPoint {
        let browserW: CGFloat = 500
        let browserH: CGFloat = 420
        let pos = viewModel.browserPosition

        // Default to center if position is .zero
        let targetX = pos == .zero ? containerSize.width / 2 : pos.x
        let targetY = pos == .zero ? containerSize.height / 2 : pos.y

        let x = min(max(browserW / 2, targetX), containerSize.width - browserW / 2)
        let y = min(max(browserH / 2, targetY), containerSize.height - browserH / 2)

        return CGPoint(x: x, y: y)
    }

    /// Whether the current first responder is a text input (search field, etc.).
    private static func isTextFieldFocused() -> Bool {
        guard let firstResponder = NSApp.keyWindow?.firstResponder else { return false }
        return firstResponder is NSTextView || firstResponder is NSTextField
    }

    private func installModifierMonitor() {
        NSEvent.addLocalMonitorForEvents(matching: .flagsChanged) { event in
            shiftHeld = event.modifierFlags.contains(.shift)
            return event
        }
        let vm = viewModel
        NSEvent.addLocalMonitorForEvents(matching: .keyDown) { event in
            let flags = event.modifierFlags.intersection(.deviceIndependentFlagsMask)

            // Command-key shortcuts (work regardless of focus)
            if flags.contains(.command) {
                switch event.charactersIgnoringModifiers {
                case "z":
                    if flags.contains(.shift) {
                        vm.redo()
                    } else {
                        vm.undo()
                    }
                    return nil
                case "s":
                    if flags.contains(.shift) {
                        withAnimation(.easeInOut(duration: 0.3)) {
                            vm.toggleStageMode()
                        }
                        return nil
                    }
                    return event
                case "c":
                    if !flags.contains(.shift) && !MainEditorView.isTextFieldFocused() {
                        vm.copySelected()
                        return nil
                    }
                    return event
                case "v":
                    if !flags.contains(.shift) && !MainEditorView.isTextFieldFocused() {
                        vm.pasteNodes()
                        return nil
                    }
                    return event
                case "a":
                    if !flags.contains(.shift) {
                        vm.selectAll()
                        return nil
                    }
                    return event
                default:
                    return event
                }
            }

            // Non-command shortcuts — skip when a text field is focused
            guard !MainEditorView.isTextFieldFocused() else { return event }

            switch event.keyCode {
            case 51, 117: // 51 = Backspace (Delete), 117 = Forward Delete
                vm.deleteSelected()
                return nil
            case 49: // Space
                if let nodeId = vm.selectedNodeId {
                    vm.toggleActiveState(nodeId)
                }
                return nil
            case 53: // Escape
                // Let preview windows handle their own Escape
                if NSApp.keyWindow is PreviewWindow {
                    return event
                }
                if vm.showShaderBrowser || vm.showSourceBrowser {
                    vm.showShaderBrowser = false
                    vm.showSourceBrowser = false
                    vm.browserMode = .add
                    vm.swapTargetNodeId = nil
                } else {
                    vm.deselectAll()
                }
                return nil
            default:
                return event
            }
        }
    }
}

// MARK: - Panel Resize Handle

/// Thin draggable divider for resizing adjacent panels.
/// Uses `.global` coordinate space so the handle's own movement
/// doesn't shift the gesture reference frame, and computes width
/// from initial value + total translation to avoid jitter.
struct PanelResizeHandle: View {
    @Environment(ThemeManager.self) private var theme
    @Binding var width: CGFloat
    let minWidth: CGFloat
    let maxWidth: CGFloat
    let invertDrag: Bool

    @State private var startWidth: CGFloat?

    var body: some View {
        theme.colors.border
            .frame(width: 1)
            .padding(.horizontal, 2)
            .contentShape(Rectangle())
            .onHover { hovering in
                if hovering { NSCursor.resizeLeftRight.push() }
                else { NSCursor.pop() }
            }
            .gesture(
                DragGesture(minimumDistance: 1, coordinateSpace: .global)
                    .onChanged { value in
                        if startWidth == nil { startWidth = width }
                        let delta = invertDrag ? -value.translation.width : value.translation.width
                        width = max(minWidth, min(maxWidth, startWidth! + delta))
                    }
                    .onEnded { _ in
                        startWidth = nil
                    }
            )
    }
}

/// Thin draggable divider for resizing vertical (height) panels.
struct VerticalPanelResizeHandle: View {
    @Environment(ThemeManager.self) private var theme
    @Binding var height: CGFloat
    let minHeight: CGFloat
    let maxHeight: CGFloat

    @State private var startHeight: CGFloat?

    var body: some View {
        theme.colors.border
            .frame(height: 1)
            .padding(.vertical, 2)
            .contentShape(Rectangle())
            .onHover { hovering in
                if hovering { NSCursor.resizeUpDown.push() }
                else { NSCursor.pop() }
            }
            .gesture(
                DragGesture(minimumDistance: 1, coordinateSpace: .global)
                    .onChanged { value in
                        if startHeight == nil { startHeight = height }
                        // Dragging up (negative translation) increases height
                        let delta = -value.translation.height
                        height = max(minHeight, min(maxHeight, startHeight! + delta))
                    }
                    .onEnded { _ in
                        startHeight = nil
                    }
            )
    }
}

/// Counts display frames and reports FPS, updated once per second.
final class FPSCounter {
    private var lastTime = CACurrentMediaTime()
    private var frameCount = 0
    private var currentFPS: Double = 0

    func tick() -> Double {
        frameCount += 1
        let now = CACurrentMediaTime()
        if now - lastTime >= 1.0 {
            currentFPS = Double(frameCount) / (now - lastTime)
            frameCount = 0
            lastTime = now
        }
        return currentFPS
    }
}

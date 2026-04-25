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
    @Environment(HelpGuideViewModel.self) private var helpGuide
    @State private var shiftHeld = false
    @State private var fpsCounter = FPSCounter()
    @State private var canvasSize: CGSize = CGSize(width: 800, height: 600)
    @State private var monitorsInstalled = false

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
                                Color.clear.onAppear {
                                        canvasSize = geo.size
                                        if viewModel.needsInitialZoomToFit {
                                            viewModel.needsInitialZoomToFit = false
                                            viewModel.zoomToFit(canvasSize: geo.size)
                                        }
                                    }
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
                        .overlay {
                            if helpGuide.isActive {
                                HelpOverlayView()
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
                AudioPanelView(onToggleAudio: {
                    viewModel.isAudioActive = NottawaEngine.shared.isAudioActive
                })
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
        .onChange(of: helpGuide.requiredSidebarTab) { _, tab in
            guard let tab else { return }
            viewModel.sidebarActiveTab = tab
            viewModel.showSidebar = true
        }
        .onAppear {
            if !monitorsInstalled {
                monitorsInstalled = true
                installModifierMonitor()
            }
        }
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

            // Command-key shortcuts
            if flags.contains(.command) {
                let textFocused = MainEditorView.isTextFieldFocused()

                switch event.charactersIgnoringModifiers {
                case "z":
                    if textFocused { return event }
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
                    if !flags.contains(.shift) && !textFocused {
                        vm.copySelected()
                        return nil
                    }
                    return event
                case "v":
                    if !flags.contains(.shift) && !textFocused {
                        vm.pasteNodes()
                        return nil
                    }
                    return event
                case "a":
                    if textFocused { return event }
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

            // Ctrl-Tab / Ctrl-Shift-Tab: cycle node selection (left-to-right,
            // top-to-bottom by canvas position). Checked before plain Tab below.
            if event.keyCode == 48 && flags.contains(.control) {
                let ordered = vm.nodes.sorted {
                    if $0.position.y != $1.position.y { return $0.position.y < $1.position.y }
                    return $0.position.x < $1.position.x
                }
                if !ordered.isEmpty {
                    let shift = flags.contains(.shift)
                    let current = ordered.firstIndex(where: { $0.id == vm.selectedNodeId })
                    let next: Int
                    if let cur = current {
                        next = (cur + (shift ? 1 : -1) + ordered.count) % ordered.count
                    } else {
                        next = shift ? 0 : ordered.count - 1
                    }
                    vm.selectNode(ordered[next].id, exclusive: true)
                    return nil
                }
            }

            // Tab / Shift-Tab: navigate between parameters. Source depends on
            // whether the inspector (normal mode) or stage panel is active.
            if event.keyCode == 48 {
                let navigable = vm.stageModeEnabled
                    ? vm.stageNavigableParameters
                    : MainEditorView.navigableParameters(vm.inspectorParameters)
                if !navigable.isEmpty {
                    let shift = flags.contains(.shift)
                    let current = navigable.firstIndex(where: { $0.id == vm.selectedParameterId })
                    let next: Int
                    if let cur = current {
                        next = (cur + (shift ? -1 : 1) + navigable.count) % navigable.count
                    } else {
                        next = shift ? navigable.count - 1 : 0
                    }
                    vm.selectedParameterId = navigable[next].id
                    return nil
                }
            }

            // Parameter-selection keyboard shortcuts (arrows to step, digit/-/. to edit,
            // Escape to clear parameter selection before falling through to node deselect).
            let paramSource = vm.stageModeEnabled ? vm.stageNavigableParameters : vm.inspectorParameters
            if let paramId = vm.selectedParameterId,
               let param = paramSource.first(where: { $0.id == paramId }) {
                switch event.keyCode {
                case 126: // Up
                    MainEditorView.stepSelectedParameter(param: param, direction: 1, flags: flags, vm: vm)
                    return nil
                case 125: // Down
                    MainEditorView.stepSelectedParameter(param: param, direction: -1, flags: flags, vm: vm)
                    return nil
                case 53: // Escape
                    vm.selectedParameterId = nil
                    return nil
                default:
                    break
                }

                // Typing a digit / minus / period on a float param begins inline edit.
                // Only the inspector's ParameterControlView currently handles inline edit;
                // CompactParameterControl (stage mode) doesn't, so skip there.
                if !vm.stageModeEnabled,
                   param.parameterType == .standard,
                   flags.isDisjoint(with: [.command, .option, .control]),
                   let chars = event.charactersIgnoringModifiers,
                   let first = chars.first,
                   first.isNumber || first == "-" || first == "." {
                    NotificationCenter.default.post(
                        name: .parameterBeginEdit,
                        object: nil,
                        userInfo: ["paramId": param.id, "initialText": String(first)]
                    )
                    return nil
                }
            }

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

    /// Params eligible for Tab navigation (matches ParameterInspectorView's visible rows).
    /// Skips hidden params and skips Y-position halves that are rolled into a PositionPad.
    private static func navigableParameters(_ all: [ParameterInfo]) -> [ParameterInfo] {
        all.filter { p in
            p.parameterType != .hidden &&
            !((p.name == "Y Position" || p.name == "Y") && p.minValue >= 0 && p.maxValue <= 1.0)
        }
    }

    /// Adjust a selected parameter by one keyboard step.
    /// Modifier conventions: Shift = coarse (10%), Option = fine (0.1%), else normal (1%).
    private static func stepSelectedParameter(param: ParameterInfo,
                                              direction: Float,
                                              flags: NSEvent.ModifierFlags,
                                              vm: NodeEditorViewModel) {
        switch param.parameterType {
        case .standard:
            let range = param.maxValue - param.minValue
            let fraction: Float = flags.contains(.shift) ? 0.1
                                : flags.contains(.option) ? 0.001
                                : 0.01
            let step = max(range * fraction, .ulpOfOne)
            let newValue = min(max(param.value + direction * step, param.minValue), param.maxValue)
            NottawaEngine.shared.setParameterValue(paramId: param.id, value: newValue)
            NotificationCenter.default.post(
                name: .parameterStepAnimated,
                object: nil,
                userInfo: ["paramId": param.id, "value": newValue]
            )

        case .int_:
            let step: Int = flags.contains(.shift) ? 10 : 1
            let minInt = Int(param.minValue)
            let maxInt = Int(param.maxValue)
            let delta = Int(direction) * step
            let newValue = min(max(param.intValue + delta, minInt), maxInt)
            NottawaEngine.shared.setParameterInt(paramId: param.id, value: newValue)
            NotificationCenter.default.post(
                name: .parameterStepAnimated,
                object: nil,
                userInfo: ["paramId": param.id, "intValue": newValue]
            )

        case .bool_:
            let newValue = direction > 0
            NottawaEngine.shared.setParameterBool(paramId: param.id, value: newValue)
            vm.refresh()
            NotificationCenter.default.post(
                name: .parameterStepAnimated,
                object: nil,
                userInfo: ["paramId": param.id, "boolValue": newValue]
            )

        default:
            break
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

//
//  NottawaApp.swift
//  NottawaApp
//
//  Main entry point for the SwiftUI Nottawa application.
//

import SwiftUI
import UniformTypeIdentifiers

@main
struct NottawaApp: App {
    @State private var viewModel = NodeEditorViewModel()
    @State private var themeManager = ThemeManager()
    @State private var helpGuide = HelpGuideViewModel()
    @State private var engineStatus: EngineStatus = .idle
    @State private var stretchPreviewWindows = false

    var body: some Scene {
        Window("Nottawa", id: "main") {
            ColorSchemeObserver(themeManager: themeManager) {
                Group {
                    switch engineStatus {
                    case .ready:
                        MainEditorView()
                            .environment(viewModel)
                            .environment(themeManager)
                            .environment(helpGuide)
                            .navigationTitle(viewModel.currentWorkspaceName ?? "Nottawa")
                            .overlay {
                                if viewModel.showWelcomeScreen {
                                    WelcomeView()
                                        .environment(viewModel)
                                        .environment(themeManager)
                                }
                            }
                    case .error(let msg):
                        VStack(spacing: 12) {
                            Image(systemName: "exclamationmark.triangle")
                                .font(.largeTitle)
                                .foregroundStyle(.red)
                            Text("Engine Error")
                                .font(.title2)
                                .foregroundStyle(themeManager.colors.textPrimary)
                            Text(msg)
                                .foregroundStyle(themeManager.colors.textSecondary)
                        }
                        .frame(maxWidth: .infinity, maxHeight: .infinity)
                        .background(themeManager.colors.background)
                    default:
                        VStack(spacing: 12) {
                            ProgressView()
                            Text("Initializing Engine...")
                                .foregroundStyle(themeManager.colors.textSecondary)
                        }
                        .frame(maxWidth: .infinity, maxHeight: .infinity)
                        .background(themeManager.colors.background)
                    }
                }
            }
            .onAppear {
                initializeEngine()
            }
            .onOpenURL { url in
                guard url.scheme == "nottawa",
                      url.host == "strands",
                      let slug = url.pathComponents.dropFirst().first
                else { return }
                viewModel.loadSharedStrand(slug: String(slug))
            }
            .onReceive(NotificationCenter.default.publisher(for: NSApplication.willTerminateNotification)) { _ in
                viewModel.cleanup()
                NottawaEngine.shared.stop()
            }
            .handlesExternalEvents(preferring: Set(["*"]), allowing: Set(["*"]))
        }
        .handlesExternalEvents(matching: Set(["*"]))
        .windowStyle(.titleBar)
        .defaultSize(width: 1280, height: 800)
        .commands {
            CommandGroup(replacing: .newItem) {
                Button("New") {
                    viewModel.newWorkspace()
                }
                .keyboardShortcut("n", modifiers: .command)

                Button("Open...") {
                    let panel = NSOpenPanel()
                    panel.allowedContentTypes = [.json]
                    panel.allowsMultipleSelection = false
                    if let window = NSApp.keyWindow ?? NSApp.mainWindow {
                        panel.beginSheetModal(for: window) { response in
                            if response == .OK, let url = panel.url {
                                viewModel.openWorkspace(url: url)
                            }
                        }
                    } else {
                        panel.begin { response in
                            if response == .OK, let url = panel.url {
                                viewModel.openWorkspace(url: url)
                            }
                        }
                    }
                }
                .keyboardShortcut("o", modifiers: .command)
            }

            CommandGroup(after: .newItem) {
                Divider()

                Button("Save") {
                    if viewModel.hasWorkspace {
                        viewModel.saveWorkspace()
                    } else {
                        presentSavePanel()
                    }
                }
                .keyboardShortcut("s", modifiers: .command)

                Button("Save As...") {
                    presentSavePanel()
                }
                .keyboardShortcut("s", modifiers: [.command, .shift])
            }

            CommandGroup(after: .toolbar) {
                Toggle("Stretch Preview Windows", isOn: $stretchPreviewWindows)
                    .onChange(of: stretchPreviewWindows) { _, newValue in
                        PreviewWindowManager.shared.setStretchToFill(newValue)
                    }
            }

            CommandGroup(replacing: .undoRedo) {
                Button("Undo") {
                    viewModel.undo()
                }
                .keyboardShortcut("z", modifiers: .command)
                .disabled(!viewModel.canUndo)

                Button("Redo") {
                    viewModel.redo()
                }
                .keyboardShortcut("z", modifiers: [.command, .shift])
                .disabled(!viewModel.canRedo)
            }

        }

        Settings {
            SettingsView()
                .environment(themeManager)
        }
    }

    private func initializeEngine() {
        engineStatus = .initializing
        let engine = NottawaEngine.shared

        // GLFW's glfwInit() (called inside engine.initialize) installs its
        // own NSApplicationDelegate that returns NSTerminateCancel from
        // applicationShouldTerminate:, which silently blocks Cmd+Q / Quit.
        // Save SwiftUI's delegate and restore it after engine init.
        let savedDelegate = NSApp.delegate

        let success = engine.initialize()

        if NSApp.delegate !== savedDelegate {
            NSApp.delegate = savedDelegate
        }

        if success {
            engine.start()
            engineStatus = .ready
            viewModel.setup()
            helpGuide.viewModel = viewModel

            // GLFW creates a hidden window for the GL context that registers
            // raw accessibility elements, preventing XCUITest from seeing the
            // SwiftUI window. Strip accessibility from GLFW windows.
            DispatchQueue.main.async {
                for window in NSApp.windows where window.contentViewController == nil {
                    // Remove all accessibility children and hide the window
                    window.setAccessibilityChildren([])
                    window.setAccessibilityWindows([])
                    window.setAccessibilityElement(false)
                    window.setAccessibilityHidden(true)
                    window.orderOut(nil)
                }
            }
        } else {
            engineStatus = .error("Failed to initialize engine")
        }
    }

    private func presentSavePanel() {
        let panel = NSSavePanel()
        panel.allowedContentTypes = [.json]
        panel.nameFieldStringValue = "Workspace-\(dateString()).json"
        if let window = NSApp.keyWindow ?? NSApp.mainWindow {
            panel.beginSheetModal(for: window) { response in
                if response == .OK, let url = panel.url {
                    self.viewModel.saveWorkspaceAs(url: url)
                }
            }
        } else {
            panel.begin { response in
                if response == .OK, let url = panel.url {
                    self.viewModel.saveWorkspaceAs(url: url)
                }
            }
        }
    }

    private func dateString() -> String {
        let f = DateFormatter()
        f.dateFormat = "MM-dd-yyyy"
        return f.string(from: Date())
    }
}

// MARK: - Color Scheme Observer

/// Lightweight view that reads `@Environment(\.colorScheme)` from the view
/// hierarchy (not the App struct) and syncs it to ThemeManager. This ensures
/// the adaptive Default theme switches between Midnight/Daylight colors
/// whenever the system appearance changes.
struct ColorSchemeObserver<Content: View>: View {
    let themeManager: ThemeManager
    @ViewBuilder let content: () -> Content
    @Environment(\.colorScheme) private var colorScheme

    var body: some View {
        content()
            .onAppear {
                themeManager.colorScheme = colorScheme
            }
            .onChange(of: colorScheme) { _, newScheme in
                themeManager.colorScheme = newScheme
            }
    }
}

// MARK: - Engine Status

enum EngineStatus {
    case idle
    case initializing
    case ready
    case error(String)
}

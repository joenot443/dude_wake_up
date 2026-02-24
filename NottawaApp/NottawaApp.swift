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
    @State private var engineStatus: EngineStatus = .idle
    @State private var stretchPreviewWindows = false

    var body: some Scene {
        WindowGroup {
            Group {
                switch engineStatus {
                case .ready:
                    MainEditorView()
                        .environment(viewModel)
                        .navigationTitle(viewModel.currentWorkspaceName ?? "Nottawa")
                        .overlay {
                            if viewModel.showWelcomeScreen {
                                WelcomeView()
                                    .environment(viewModel)
                            }
                        }
                case .error(let msg):
                    VStack(spacing: 12) {
                        Image(systemName: "exclamationmark.triangle")
                            .font(.largeTitle)
                            .foregroundStyle(.red)
                        Text("Engine Error")
                            .font(.title2)
                        Text(msg)
                            .foregroundStyle(.secondary)
                    }
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
                default:
                    VStack(spacing: 12) {
                        ProgressView()
                        Text("Initializing Engine...")
                            .foregroundStyle(.secondary)
                    }
                    .frame(maxWidth: .infinity, maxHeight: .infinity)
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
                    panel.begin { response in
                        if response == .OK, let url = panel.url {
                            viewModel.openWorkspace(url: url)
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
        } else {
            engineStatus = .error("Failed to initialize engine")
        }
    }

    private func presentSavePanel() {
        let panel = NSSavePanel()
        panel.allowedContentTypes = [.json]
        panel.nameFieldStringValue = "Workspace-\(dateString()).json"
        panel.begin { response in
            if response == .OK, let url = panel.url {
                viewModel.saveWorkspaceAs(url: url)
            }
        }
    }

    private func dateString() -> String {
        let f = DateFormatter()
        f.dateFormat = "MM-dd-yyyy"
        return f.string(from: Date())
    }
}

// MARK: - Engine Status

enum EngineStatus {
    case idle
    case initializing
    case ready
    case error(String)
}

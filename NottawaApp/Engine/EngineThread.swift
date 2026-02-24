//
//  EngineThread.swift
//  NottawaApp
//
//  Manages the background thread that runs the C++ engine loop.
//  Handles initialization, start/stop, and provides a way to
//  dispatch work to the engine thread.
//

import Foundation

@Observable
final class EngineThread {
    static let shared = EngineThread()

    private(set) var state: State = .idle

    enum State {
        case idle
        case initializing
        case running
        case stopped
        case error(String)
    }

    private init() {}

    /// Initialize the engine on a background thread (needs OpenGL context).
    /// In Phase 1, this is called synchronously since we need the OF window
    /// to be created first.
    func initialize() {
        guard case .idle = state else { return }
        state = .initializing

        let engine = NottawaEngine.shared
        if engine.initialize() {
            state = .stopped
        } else {
            state = .error("Engine initialization failed")
        }
    }

    /// Start the engine tick loop on its background thread.
    func start() {
        guard case .stopped = state else { return }
        NottawaEngine.shared.start()
        state = .running
    }

    /// Stop the engine tick loop.
    func stop() {
        guard case .running = state else { return }
        NottawaEngine.shared.stop()
        state = .stopped
    }

    /// Execute a closure on the engine thread at the start of the next tick.
    func dispatch(_ work: @escaping () -> Void) {
        // In Phase 1, we call the bridge directly since the engine loop
        // handles its own task queue via EngineLoop::executeOnEngineThread.
        // For now, just run on the current thread since the bridge functions
        // are designed to be called from any thread.
        work()
    }
}

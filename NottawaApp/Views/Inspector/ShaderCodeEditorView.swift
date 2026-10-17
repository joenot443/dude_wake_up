//
//  ShaderCodeEditorView.swift
//  NottawaApp
//
//  Session-scoped GLSL editor sheet. Edits a single shader node's
//  fragment source, hot-reloads via the engine bridge, surfaces compile
//  errors with line numbers, and offers a one-click revert to disk.
//

import SwiftUI

struct ShaderCodeEditorView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel
    @Environment(ThemeManager.self) private var theme
    @Environment(\.dismiss) private var dismiss

    let shaderId: String
    let shaderName: String

    @State private var source: String = ""
    @State private var lastAppliedSource: String = ""
    @State private var compileResult: NottawaEngine.ShaderCompileResult?
    @State private var hasOverride: Bool = false
    @State private var debounceTask: Task<Void, Never>?
    @State private var isApplying = false

    private let engine = NottawaEngine.shared

    var body: some View {
        VStack(spacing: 0) {
            toolbar
                .padding(.horizontal, 12)
                .padding(.vertical, 8)
                .background(theme.colors.backgroundSecondary)

            theme.colors.border.frame(height: 1)

            editor

            if let result = compileResult, !result.ok {
                errorBanner(result)
            }
        }
        .frame(minWidth: 720, minHeight: 520)
        .background(theme.colors.background)
        .onAppear { loadFromEngine() }
        .onChange(of: source) { _, _ in scheduleDebouncedApply() }
        .onDisappear { debounceTask?.cancel() }
    }

    // MARK: - Toolbar

    private var toolbar: some View {
        HStack(spacing: 10) {
            Text(shaderName)
                .font(.headline)
                .foregroundStyle(theme.colors.textPrimary)

            if hasOverride {
                Text("MODIFIED")
                    .font(.caption2.weight(.bold).monospaced())
                    .foregroundStyle(theme.colors.warning)
                    .padding(.horizontal, 6)
                    .padding(.vertical, 2)
                    .overlay(
                        RoundedRectangle(cornerRadius: 4)
                            .stroke(theme.colors.warning, lineWidth: 1)
                    )
            }

            Spacer()

            if let result = compileResult {
                Image(systemName: result.ok ? "checkmark.circle.fill" : "exclamationmark.triangle.fill")
                    .foregroundStyle(result.ok ? theme.colors.success : theme.colors.warning)
                    .help(result.ok ? "Compiled OK" : "Compile error")
            }

            Button("Revert") { revert() }
                .disabled(!hasOverride || isApplying)
                .help("Drop the session override and reload the shader from disk")

            Button("Apply") { applyNow() }
                .disabled(source == lastAppliedSource || isApplying)
                .keyboardShortcut(.return, modifiers: .command)
                .help("Compile and apply now (⌘⏎)")

            Button("Close") { dismiss() }
                .keyboardShortcut(.escape, modifiers: [])
        }
    }

    // MARK: - Editor

    private var editor: some View {
        TextEditor(text: $source)
            .font(.system(size: 12, design: .monospaced))
            .foregroundStyle(theme.colors.textPrimary)
            .scrollContentBackground(.hidden)
            .background(theme.colors.background)
            .padding(8)
            .accessibilityIdentifier("shader-code-editor")
    }

    // MARK: - Error banner

    private func errorBanner(_ result: NottawaEngine.ShaderCompileResult) -> some View {
        VStack(alignment: .leading, spacing: 4) {
            HStack(spacing: 6) {
                Image(systemName: "exclamationmark.triangle.fill")
                    .foregroundStyle(theme.colors.warning)
                if result.errorLine > 0 {
                    Text("Compile error — line \(result.errorLine)")
                        .font(.callout.weight(.semibold))
                        .foregroundStyle(theme.colors.textPrimary)
                } else {
                    Text("Compile error")
                        .font(.callout.weight(.semibold))
                        .foregroundStyle(theme.colors.textPrimary)
                }
            }
            if let log = result.errorLog, !log.isEmpty {
                ScrollView {
                    Text(log)
                        .font(.system(size: 11, design: .monospaced))
                        .foregroundStyle(theme.colors.textSecondary)
                        .textSelection(.enabled)
                        .frame(maxWidth: .infinity, alignment: .leading)
                }
                .frame(maxHeight: 100)
            }
        }
        .padding(10)
        .frame(maxWidth: .infinity, alignment: .leading)
        .background(theme.colors.warning.opacity(0.12))
        .overlay(theme.colors.border.frame(height: 1), alignment: .top)
    }

    // MARK: - Actions

    private func loadFromEngine() {
        source = engine.shaderFragmentSource(shaderId: shaderId) ?? ""
        lastAppliedSource = source
        hasOverride = engine.shaderHasSessionOverride(shaderId: shaderId)
        compileResult = nil
    }

    /// Cancel any pending debounce and schedule a new apply for ~500ms in
    /// the future. Buys the user typing time before slamming the engine
    /// thread with a recompile on every keystroke.
    private func scheduleDebouncedApply() {
        debounceTask?.cancel()
        let snapshot = source
        debounceTask = Task { @MainActor in
            try? await Task.sleep(nanoseconds: 500_000_000)
            if Task.isCancelled { return }
            guard snapshot == source else { return } // user kept typing
            applyNow()
        }
    }

    private func applyNow() {
        guard source != lastAppliedSource, !isApplying else { return }
        isApplying = true
        let attempt = source
        let result = engine.setShaderFragmentSource(shaderId: shaderId, source: attempt)
        compileResult = result
        if result.ok {
            lastAppliedSource = attempt
            hasOverride = engine.shaderHasSessionOverride(shaderId: shaderId)
        }
        isApplying = false
    }

    private func revert() {
        engine.revertShaderSource(shaderId: shaderId)
        compileResult = nil
        // Re-read the disk source so the editor reflects what's actually live.
        source = engine.shaderFragmentSource(shaderId: shaderId) ?? ""
        lastAppliedSource = source
        hasOverride = false
    }
}

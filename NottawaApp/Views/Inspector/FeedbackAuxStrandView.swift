//
//  FeedbackAuxStrandView.swift
//  NottawaApp
//
//  Inspector section for managing the auxiliary strand connected to a
//  Feedback / ColorKeyMaskMaker / LumaMaskMaker shader's aux output (slot 10).
//  Each captured feedback frame is processed through this chain before being
//  mixed back in (see FeedbackShader::feedbackTexture in
//  src/Shading/FeedbackShader.cpp).
//

import SwiftUI

struct FeedbackAuxStrandView: View {
    @Environment(ThemeManager.self) private var theme
    @Environment(NodeEditorViewModel.self) private var viewModel

    let parentNodeId: String

    @State private var showBrowser = false

    private var chain: [NodeModel] {
        viewModel.auxChain(forNodeId: parentNodeId)
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 8) {
            theme.colors.border.frame(height: 1)
                .padding(.vertical, 8)

            HStack(spacing: 6) {
                Text("Auxiliary Strand")
                    .font(.subheadline)
                    .fontWeight(.semibold)
                    .foregroundStyle(theme.colors.textSecondary)
                Spacer()
            }

            Text("Each feedback frame is processed through this chain before mixing back in.")
                .font(.caption2)
                .foregroundStyle(theme.colors.textTertiary)
                .fixedSize(horizontal: false, vertical: true)

            ScrollView(.horizontal, showsIndicators: false) {
                HStack(spacing: 6) {
                    ForEach(Array(chain.enumerated()), id: \.element.id) { _, node in
                        chip(for: node)
                        Image(systemName: "arrow.right")
                            .font(.system(size: 9))
                            .foregroundStyle(theme.colors.textTertiary)
                    }
                    addButton
                }
                .padding(.vertical, 2)
            }
        }
        .popover(isPresented: $showBrowser, arrowEdge: .top) {
            NodeInlineBrowserView(
                isShaderBrowser: true,
                onSelect: { shaderType in
                    addFilter(shaderType: shaderType)
                    showBrowser = false
                },
                onCancel: {
                    showBrowser = false
                }
            )
            .frame(width: 320, height: 440)
        }
    }

    // MARK: - Chip

    @ViewBuilder
    private func chip(for node: NodeModel) -> some View {
        HStack(spacing: 4) {
            Button {
                viewModel.selectNode(node.id)
            } label: {
                Text(node.name)
                    .font(.caption)
                    .fontWeight(.medium)
                    .foregroundStyle(theme.colors.textPrimary)
                    .lineLimit(1)
            }
            .buttonStyle(.plainHitArea)

            Button {
                disconnect(node)
            } label: {
                Image(systemName: "xmark")
                    .font(.system(size: 8, weight: .bold))
                    .foregroundStyle(theme.colors.textTertiary)
            }
            .buttonStyle(.plainHitArea)
        }
        .padding(.horizontal, 8)
        .padding(.vertical, 4)
        .background(
            Capsule().fill(theme.colors.surface)
        )
        .overlay(
            Capsule().strokeBorder(theme.colors.warning.opacity(0.5), lineWidth: 1)
        )
    }

    private var addButton: some View {
        Button {
            showBrowser = true
        } label: {
            HStack(spacing: 4) {
                Image(systemName: "plus")
                    .font(.system(size: 9, weight: .bold))
                Text(chain.isEmpty ? "Add Filter" : "Add")
                    .font(.caption)
            }
            .padding(.horizontal, 8)
            .padding(.vertical, 4)
            .foregroundStyle(theme.colors.accent)
            .overlay(
                Capsule().strokeBorder(theme.colors.accent.opacity(0.6), lineWidth: 1)
            )
        }
        .buttonStyle(.plainHitArea)
    }

    // MARK: - Actions

    /// Disconnect the chain link feeding into `node`. Anything downstream of
    /// `node` keeps its internal wiring but is no longer part of the aux chain.
    private func disconnect(_ node: NodeModel) {
        guard let conn = viewModel.auxChainIncomingConnection(
            parentNodeId: parentNodeId,
            chainNodeId: node.id
        ) else { return }
        viewModel.removeConnection(conn.id)
    }

    /// Add a new shader to the end of the chain. If the chain is empty, wire
    /// it from the parent's aux pin (slot 10). Otherwise, wire it from the
    /// current tail's main output (slot 0).
    private func addFilter(shaderType: Int) {
        let chainSnapshot = chain
        guard let parentNode = viewModel.nodes.first(where: { $0.id == parentNodeId }) else {
            return
        }

        // Position the new shader to the right of whatever it's chaining off of.
        let anchor = chainSnapshot.last ?? parentNode
        let newPos = CGPoint(x: anchor.position.x + 320, y: anchor.position.y)

        // One undo removes the filter and its connection.
        ntw_begin_undo_group()
        defer { ntw_end_undo_group() }
        guard let newId = viewModel.addShader(type: shaderType, at: newPos) else { return }

        let startId: String
        let outputSlot: Int
        if let tail = chainSnapshot.last {
            startId = tail.id
            outputSlot = 0
        } else {
            startId = parentNodeId
            outputSlot = 10
        }
        viewModel.makeConnection(
            startId: startId,
            endId: newId,
            outputSlot: outputSlot,
            inputSlot: 0
        )
    }
}

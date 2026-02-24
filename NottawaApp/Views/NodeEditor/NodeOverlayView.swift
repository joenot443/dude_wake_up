//
//  NodeOverlayView.swift
//  NottawaApp
//
//  Positions all NodeView instances on the canvas with transform applied.
//

import SwiftUI

struct NodeOverlayView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel

    var body: some View {
        let scale = viewModel.canvasScale
        ForEach(viewModel.nodes) { node in
            NodeView(
                node: node,
                isSelected: viewModel.selectedNodeIds.contains(node.id),
                scale: scale
            )
            .position(
                x: node.position.x * scale + viewModel.canvasOffset.x,
                y: node.position.y * scale + viewModel.canvasOffset.y
            )
        }
    }
}

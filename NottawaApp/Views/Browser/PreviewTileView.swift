//
//  PreviewTileView.swift
//  NottawaApp
//
//  A browser tile that shows a live animated shader preview.
//

import SwiftUI

struct PreviewTileView: View {
    let name: String
    let shaderTypeRaw: Int
    let isSource: Bool
    let previewManager: BrowserPreviewManager
    let action: () -> Void

    @State private var previewId: String?

    var body: some View {
        Button(action: action) {
            VStack(spacing: 6) {
                ZStack {
                    RoundedRectangle(cornerRadius: 8)
                        .fill(Color(.darkGray).opacity(0.5))
                        .aspectRatio(16.0 / 9.0, contentMode: .fit)

                    if let pid = previewId {
                        TextureDisplayView(connectableId: pid)
                            .aspectRatio(16.0 / 9.0, contentMode: .fit)
                            .clipShape(RoundedRectangle(cornerRadius: 8))
                            .allowsHitTesting(false)
                    } else {
                        Image(systemName: isSource ? "video.fill" : "sparkles")
                            .font(.title2)
                            .foregroundStyle(.secondary)
                    }
                }

                Text(name)
                    .font(.caption)
                    .lineLimit(1)
                    .foregroundStyle(.primary)
            }
            .padding(8)
            .background(
                RoundedRectangle(cornerRadius: 10)
                    .fill(Color(.controlBackgroundColor))
            )
            .overlay(
                RoundedRectangle(cornerRadius: 10)
                    .strokeBorder(Color(.separatorColor), lineWidth: 0.5)
            )
        }
        .buttonStyle(.plain)
        .onAppear {
            previewId = previewManager.previewId(for: shaderTypeRaw, isSource: isSource)
        }
        .onDisappear {
            previewId = nil
            previewManager.destroyPreview(for: shaderTypeRaw, isSource: isSource)
        }
    }
}

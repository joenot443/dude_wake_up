//
//  StrandTileView.swift
//  NottawaApp
//
//  Tile view for a single strand in the sidebar browser.
//  Shows a preview thumbnail (loaded from disk) or a fallback icon,
//  with drag-to-canvas and click-to-load support.
//

import SwiftUI

struct StrandTileView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel
    let strand: StrandInfo

    @State private var thumbnail: CGImage?

    private var dragPayload: String {
        "strand:\(strand.id)"
    }

    var body: some View {
        Button {
            viewModel.loadStrand(id: strand.id)
        } label: {
            VStack(spacing: 4) {
                ZStack {
                    RoundedRectangle(cornerRadius: 6)
                        .fill(Color.orange.opacity(0.15))
                        .aspectRatio(16.0 / 9.0, contentMode: .fit)

                    if let thumbnail = thumbnail {
                        Image(decorative: thumbnail, scale: 1)
                            .resizable()
                            .aspectRatio(16.0 / 9.0, contentMode: .fit)
                            .clipShape(RoundedRectangle(cornerRadius: 6))
                    } else {
                        Image(systemName: "rectangle.stack")
                            .font(.caption)
                            .foregroundStyle(.orange)
                    }
                }

                Text(strand.name)
                    .font(.system(size: 10))
                    .lineLimit(1)
                    .foregroundStyle(.primary)
            }
            .padding(6)
            .background(
                RoundedRectangle(cornerRadius: 8)
                    .fill(Color(.controlBackgroundColor))
            )
            .overlay(
                RoundedRectangle(cornerRadius: 8)
                    .strokeBorder(Color(.separatorColor), lineWidth: 0.5)
            )
        }
        .buttonStyle(.plainHitArea)
        .onDrag {
            NSItemProvider(object: dragPayload as NSString)
        }
        .onAppear {
            loadThumbnail()
        }
    }

    private func loadThumbnail() {
        guard let path = strand.imagePath, !path.isEmpty else { return }
        DispatchQueue.global(qos: .userInitiated).async {
            guard let dataProvider = CGDataProvider(filename: path),
                  let image = CGImage(
                    pngDataProviderSource: dataProvider,
                    decode: nil,
                    shouldInterpolate: true,
                    intent: .defaultIntent
                  ) else {
                // Try JPEG if PNG fails
                guard let nsImage = NSImage(contentsOfFile: path),
                      let cgImage = nsImage.cgImage(forProposedRect: nil, context: nil, hints: nil) else {
                    return
                }
                DispatchQueue.main.async {
                    self.thumbnail = cgImage
                }
                return
            }
            DispatchQueue.main.async {
                self.thumbnail = image
            }
        }
    }
}

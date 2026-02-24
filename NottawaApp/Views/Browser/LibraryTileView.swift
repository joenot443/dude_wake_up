//
//  LibraryTileView.swift
//  NottawaApp
//
//  Compact tile for library items showing thumbnail, name,
//  download state, and supporting click-to-add and drag-to-canvas.
//

import SwiftUI

struct LibraryTileView: View {
    let file: LibraryFileInfo
    let action: () -> Void

    @State private var thumbnailImage: NSImage?

    private var dragPayload: String { "library:\(file.id)" }

    var body: some View {
        Button(action: action) {
            VStack(spacing: 4) {
                ZStack {
                    RoundedRectangle(cornerRadius: 6)
                        .fill(Color.green.opacity(0.15))
                        .aspectRatio(16.0 / 9.0, contentMode: .fit)

                    if let image = thumbnailImage {
                        Image(nsImage: image)
                            .resizable()
                            .aspectRatio(16.0 / 9.0, contentMode: .fill)
                            .clipShape(RoundedRectangle(cornerRadius: 6))
                    } else {
                        Image(systemName: "film")
                            .font(.caption)
                            .foregroundStyle(.green)
                    }

                    downloadOverlay
                }

                Text(file.name)
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
        .buttonStyle(.plain)
        .accessibilityIdentifier("library-tile-\(file.name)")
        .onDrag {
            NSItemProvider(object: dragPayload as NSString)
        }
        .onAppear { loadThumbnail() }
        .onChange(of: file.thumbnailPath) { _, _ in loadThumbnail() }
    }

    @ViewBuilder
    private var downloadOverlay: some View {
        if file.isDownloading {
            ZStack {
                Circle()
                    .fill(.ultraThinMaterial)
                    .frame(width: 28, height: 28)
                ProgressView(value: Double(file.downloadProgress))
                    .progressViewStyle(.circular)
                    .scaleEffect(0.7)
                    .frame(width: 24, height: 24)
            }
        } else if !file.isDownloaded {
            VStack {
                Spacer()
                HStack {
                    Spacer()
                    Image(systemName: "icloud.and.arrow.down")
                        .font(.system(size: 10))
                        .padding(4)
                        .background(.ultraThinMaterial, in: Circle())
                        .padding(4)
                }
            }
        }
    }

    private func loadThumbnail() {
        guard let path = file.thumbnailPath else { return }
        guard thumbnailImage == nil else { return }
        DispatchQueue.global(qos: .userInitiated).async {
            guard let image = NSImage(contentsOfFile: path) else { return }
            DispatchQueue.main.async {
                thumbnailImage = image
            }
        }
    }
}

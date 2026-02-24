//
//  ShareStrandSheet.swift
//  NottawaApp
//
//  Sheet for sharing a strand to the community.
//

import SwiftUI

struct ShareStrandSheet: View {
    @Environment(NodeEditorViewModel.self) private var viewModel
    @Environment(\.dismiss) private var dismiss

    @State private var title = ""
    @State private var description = ""
    @State private var author: String = UserDefaults.standard.string(forKey: "nottawa_share_author") ?? ""
    @State private var previewImage: CGImage?

    var body: some View {
        VStack(spacing: 16) {
            Text("Share Strand")
                .font(.headline)

            // Preview
            if let img = previewImage {
                Image(decorative: img, scale: 1.0)
                    .resizable()
                    .aspectRatio(contentMode: .fit)
                    .frame(maxHeight: 160)
                    .cornerRadius(8)
            } else {
                Rectangle()
                    .fill(Color(.controlBackgroundColor))
                    .frame(height: 120)
                    .overlay {
                        Image(systemName: "photo")
                            .font(.system(size: 32))
                            .foregroundStyle(.tertiary)
                    }
                    .cornerRadius(8)
            }

            // Form fields
            VStack(alignment: .leading, spacing: 8) {
                TextField("Title (required)", text: $title)
                    .textFieldStyle(.roundedBorder)

                TextField("Description (optional)", text: $description, axis: .vertical)
                    .textFieldStyle(.roundedBorder)
                    .lineLimit(3...5)

                TextField("Author (optional)", text: $author)
                    .textFieldStyle(.roundedBorder)
            }

            // Status
            if viewModel.shareInProgress {
                HStack(spacing: 8) {
                    ProgressView()
                        .controlSize(.small)
                    Text("Uploading...")
                        .font(.system(size: 12))
                        .foregroundStyle(.secondary)
                }
            }

            if let result = viewModel.shareResult {
                switch result {
                case .success(let slug):
                    VStack(spacing: 8) {
                        Label("Shared successfully!", systemImage: "checkmark.circle.fill")
                            .foregroundStyle(.green)
                            .font(.system(size: 13, weight: .medium))

                        let link = "https://nottawa.app/strands/\(slug)"
                        HStack {
                            Text(link)
                                .font(.system(size: 11))
                                .foregroundStyle(.secondary)
                                .lineLimit(1)
                                .truncationMode(.middle)

                            Button("Copy Link") {
                                NSPasteboard.general.clearContents()
                                NSPasteboard.general.setString(link, forType: .string)
                            }
                            .buttonStyle(.bordered)
                            .controlSize(.small)
                        }
                    }
                case .error(let message):
                    Label(message, systemImage: "exclamationmark.triangle.fill")
                        .foregroundStyle(.red)
                        .font(.system(size: 12))
                }
            }

            // Buttons
            HStack {
                Button("Cancel") {
                    dismiss()
                }
                .keyboardShortcut(.cancelAction)

                Spacer()

                if case .success = viewModel.shareResult {
                    Button("Done") {
                        dismiss()
                    }
                    .keyboardShortcut(.defaultAction)
                } else {
                    Button("Share") {
                        // Remember author for next time
                        if !author.isEmpty {
                            UserDefaults.standard.set(author, forKey: "nottawa_share_author")
                        }
                        let pngData = previewImage.flatMap { cgImage -> Data? in
                            let rep = NSBitmapImageRep(cgImage: cgImage)
                            return rep.representation(using: .png, properties: [:])
                        }
                        guard let nodeId = viewModel.shareNodeId else { return }
                        viewModel.shareStrand(nodeId: nodeId, title: title,
                                            description: description, author: author,
                                            previewPng: pngData)
                    }
                    .keyboardShortcut(.defaultAction)
                    .disabled(title.isEmpty || viewModel.shareInProgress)
                }
            }
        }
        .padding(20)
        .frame(width: 400)
        .onAppear {
            if let nodeId = viewModel.shareNodeId {
                previewImage = TextureProvider.shared.snapshot(for: nodeId)
                // Default title from node name
                if let node = viewModel.nodes.first(where: { $0.id == nodeId }) {
                    title = node.name
                }
            }
            viewModel.shareResult = nil
        }
    }
}

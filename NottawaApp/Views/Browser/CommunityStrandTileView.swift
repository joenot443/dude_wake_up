//
//  CommunityStrandTileView.swift
//  NottawaApp
//
//  Tile view for a single shared strand in the community feed.
//

import SwiftUI

struct CommunityStrandTileView: View {
    let slug: String
    @Environment(NodeEditorViewModel.self) private var viewModel

    private var strand: SharedStrandInfo? {
        viewModel.communityStrands.first { $0.slug == slug }
    }

    var body: some View {
        if let strand {
            VStack(alignment: .leading, spacing: 6) {
                // Preview image — tap to load strand
                Group {
                    if let urlStr = strand.previewUrl, let url = URL(string: urlStr) {
                        AsyncImage(url: url) { phase in
                            switch phase {
                            case .success(let image):
                                image
                                    .resizable()
                                    .aspectRatio(contentMode: .fill)
                                    .frame(height: 100)
                                    .clipped()
                            case .failure:
                                placeholderImage
                            case .empty:
                                ZStack {
                                    placeholderImage
                                    ProgressView()
                                        .controlSize(.small)
                                }
                            @unknown default:
                                placeholderImage
                            }
                        }
                        .frame(maxWidth: .infinity)
                        .frame(height: 100)
                        .cornerRadius(6)
                    } else {
                        placeholderImage
                            .frame(height: 100)
                            .cornerRadius(6)
                    }
                }
                .contentShape(Rectangle())
                .onTapGesture {
                    viewModel.loadSharedStrand(slug: slug)
                }

                // Title + author
                VStack(alignment: .leading, spacing: 2) {
                    Text(strand.title)
                        .font(.system(size: 12, weight: .semibold))
                        .lineLimit(1)

                    if let author = strand.author, !author.isEmpty {
                        Text("by \(author)")
                            .font(.system(size: 10))
                            .foregroundStyle(.secondary)
                            .lineLimit(1)
                    }
                }
                .contentShape(Rectangle())
                .onTapGesture {
                    viewModel.loadSharedStrand(slug: slug)
                }

                // Vote row
                HStack(spacing: 8) {
                    Button {
                        viewModel.voteOnStrand(slug: slug, vote: "up")
                    } label: {
                        HStack(spacing: 2) {
                            Image(systemName: strand.userVote == "up" ? "arrow.up.circle.fill" : "arrow.up.circle")
                                .font(.system(size: 12))
                                .foregroundStyle(strand.userVote == "up" ? .green : .secondary)
                            Text("\(strand.upvotes)")
                                .font(.system(size: 10))
                                .foregroundStyle(strand.userVote == "up" ? .green : .secondary)
                        }
                    }
                    .buttonStyle(.plain)

                    Text("\(strand.score)")
                        .font(.system(size: 11, weight: .medium))
                        .foregroundStyle(.primary)

                    Button {
                        viewModel.voteOnStrand(slug: slug, vote: "down")
                    } label: {
                        HStack(spacing: 2) {
                            Image(systemName: strand.userVote == "down" ? "arrow.down.circle.fill" : "arrow.down.circle")
                                .font(.system(size: 12))
                                .foregroundStyle(strand.userVote == "down" ? .red : .secondary)
                            Text("\(strand.downvotes)")
                                .font(.system(size: 10))
                                .foregroundStyle(strand.userVote == "down" ? .red : .secondary)
                        }
                    }
                    .buttonStyle(.plain)

                    Spacer()
                }
            }
            .padding(8)
            .background(Color(.controlBackgroundColor).opacity(0.5))
            .cornerRadius(8)
            .onDrag {
                NSItemProvider(object: "shared_strand:\(slug)" as NSString)
            }
            .contextMenu {
                Button("Copy Link") {
                    let link = "https://nottawa.app/strands/\(slug)"
                    NSPasteboard.general.clearContents()
                    NSPasteboard.general.setString(link, forType: .string)
                }
            }
        }
    }

    private var placeholderImage: some View {
        Rectangle()
            .fill(Color(.controlBackgroundColor))
            .frame(maxWidth: .infinity)
            .overlay {
                Image(systemName: "rectangle.stack")
                    .font(.system(size: 24))
                    .foregroundStyle(.tertiary)
            }
    }
}

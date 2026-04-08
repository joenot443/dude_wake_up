//
//  CommunityStrandListView.swift
//  NottawaApp
//
//  Scrollable list of shared strands from the community feed.
//

import SwiftUI

struct CommunityStrandListView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel

    var body: some View {
        VStack(spacing: 0) {
            // Header with sort + refresh
            HStack(spacing: 6) {
                Text("\(viewModel.communityStrands.count) strands")
                    .font(.system(size: 11))
                    .foregroundStyle(.secondary)
                Spacer()
                Menu {
                    ForEach(NodeEditorViewModel.CommunitySortOrder.allCases, id: \.self) { order in
                        Button {
                            viewModel.communitySortOrder = order
                        } label: {
                            HStack {
                                Text(order.rawValue)
                                if viewModel.communitySortOrder == order {
                                    Image(systemName: "checkmark")
                                }
                            }
                        }
                    }
                } label: {
                    HStack(spacing: 3) {
                        Image(systemName: "arrow.up.arrow.down")
                            .font(.system(size: 10))
                        Text(viewModel.communitySortOrder.rawValue)
                            .font(.system(size: 11))
                    }
                    .foregroundStyle(.secondary)
                }
                .menuStyle(.borderlessButton)
                .fixedSize()
                Button {
                    viewModel.fetchCommunityFeed(page: 1)
                } label: {
                    Image(systemName: "arrow.clockwise")
                        .font(.system(size: 12))
                }
                .buttonStyle(.plainHitArea)
                .foregroundStyle(.secondary)
                .disabled(viewModel.communityLoading)
            }
            .padding(.horizontal, 10)
            .padding(.vertical, 6)

            Divider()

            if viewModel.communityStrands.isEmpty && !viewModel.communityLoading {
                VStack(spacing: 8) {
                    Image(systemName: "globe")
                        .font(.system(size: 32))
                        .foregroundStyle(.tertiary)
                    Text("No shared strands yet")
                        .font(.system(size: 13, weight: .medium))
                        .foregroundStyle(.secondary)
                    Text("Share your creations with\nthe community!")
                        .font(.system(size: 11))
                        .foregroundStyle(.tertiary)
                        .multilineTextAlignment(.center)
                }
                .frame(maxWidth: .infinity, maxHeight: .infinity)
                .padding()
            } else {
                ScrollView {
                    LazyVStack(spacing: 8) {
                        ForEach(viewModel.sortedCommunityStrands) { strand in
                            CommunityStrandTileView(strand: strand)
                        }

                        if viewModel.communityPage < viewModel.communityTotalPages {
                            Button("Load More") {
                                viewModel.fetchCommunityFeed(page: viewModel.communityPage + 1)
                            }
                            .buttonStyle(.plainHitArea)
                            .font(.system(size: 12, weight: .medium))
                            .foregroundStyle(Color.accentColor)
                            .padding(.vertical, 8)
                        }
                    }
                    .padding(8)
                }
            }

            if viewModel.communityLoading {
                HStack(spacing: 6) {
                    ProgressView()
                        .controlSize(.small)
                    Text("Loading...")
                        .font(.system(size: 11))
                        .foregroundStyle(.secondary)
                }
                .padding(.vertical, 6)
            }
        }
        .onAppear {
            if viewModel.communityStrands.isEmpty {
                viewModel.fetchCommunityFeed()
            }
        }
    }
}

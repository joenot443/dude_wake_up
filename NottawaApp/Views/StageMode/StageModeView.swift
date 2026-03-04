//
//  StageModeView.swift
//  NottawaApp
//
//  Main container for Stage Mode — channel strips on left, output preview on right.
//

import SwiftUI

struct StageModeView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel
    @Environment(ThemeManager.self) private var theme
    @State private var dropTargetIndex: Int? = nil

    var body: some View {
        if viewModel.orderedChainStrips.isEmpty {
            VStack(spacing: 12) {
                Image(systemName: "slider.horizontal.3")
                    .font(.system(size: 40))
                    .foregroundStyle(theme.colors.textTertiary)
                Text("Add shaders in the node editor to use Stage Mode")
                    .font(.callout)
                    .foregroundStyle(theme.colors.textSecondary)
                Button("Exit Stage Mode") {
                    withAnimation(.easeInOut(duration: 0.3)) {
                        viewModel.toggleStageMode()
                    }
                }
                .buttonStyle(.plain)
                .padding(.horizontal, 12)
                .padding(.vertical, 6)
                .background(theme.colors.surface)
                .foregroundStyle(theme.colors.textPrimary)
                .clipShape(RoundedRectangle(cornerRadius: 6))
            }
            .frame(maxWidth: .infinity, maxHeight: .infinity)
        } else {
            HStack(spacing: 0) {
                // Left: channel strips with filter
                VStack(spacing: 0) {
                    // Favorites filter toggle + reset order
                    HStack(spacing: 6) {
                        Button {
                            viewModel.stageShowAllParams.toggle()
                        } label: {
                            Image(systemName: viewModel.stageShowAllParams ? "heart" : "heart.fill")
                                .font(.system(size: 13))
                                .foregroundStyle(viewModel.stageShowAllParams ? theme.colors.textSecondary : theme.colors.accent)
                        }
                        .buttonStyle(.plain)
                        .help(viewModel.stageShowAllParams ? "Show favorites only" : "Showing favorites only")

                        Text(viewModel.stageShowAllParams ? "All Parameters" : "Favorites Only")
                            .font(.caption)
                            .foregroundStyle(theme.colors.textSecondary)

                        Spacer()

                        if viewModel.stageStripOrder != nil {
                            Button {
                                viewModel.resetStageStripOrder()
                            } label: {
                                Image(systemName: "arrow.counterclockwise")
                                    .font(.system(size: 11))
                                    .foregroundStyle(theme.colors.textSecondary)
                            }
                            .buttonStyle(.plain)
                            .help("Reset to default order")
                        }
                    }
                    .padding(.horizontal, 12)
                    .padding(.vertical, 8)

                    theme.colors.border.frame(height: 1)

                    ScrollView {
                        LazyVStack(spacing: 0) {
                            ForEach(Array(viewModel.orderedChainStrips.enumerated()), id: \.element.id) { index, strip in
                                VStack(spacing: 0) {
                                    // Drop indicator above this strip
                                    if dropTargetIndex == index {
                                        dropIndicator
                                    }

                                    ChannelStripView(strip: strip, index: index, total: viewModel.orderedChainStrips.count)
                                        .padding(.vertical, 4)
                                        .onDrop(of: [.text], delegate: StripDropDelegate(
                                            targetIndex: index,
                                            viewModel: viewModel,
                                            dropTargetIndex: $dropTargetIndex
                                        ))
                                }
                            }

                            // Drop indicator after last strip
                            if dropTargetIndex == viewModel.orderedChainStrips.count {
                                dropIndicator
                            }
                        }
                        .padding(.horizontal, 12)
                        .padding(.vertical, 4)
                    }
                }
                .frame(minWidth: 350, idealWidth: 450)

                theme.colors.border.frame(width: 1)

                // Right: main output + bottom bar
                VStack(spacing: 0) {
                    StageOutputView()
                        .frame(maxWidth: .infinity, maxHeight: .infinity)

                    theme.colors.border.frame(height: 1)

                    StageBottomBarView()
                }
            }
        }
    }

    private var dropIndicator: some View {
        HStack(spacing: 4) {
            Circle()
                .fill(theme.colors.accent)
                .frame(width: 6, height: 6)
            Rectangle()
                .fill(theme.colors.accent)
                .frame(height: 2)
            Circle()
                .fill(theme.colors.accent)
                .frame(width: 6, height: 6)
        }
        .padding(.horizontal, 4)
        .padding(.vertical, 2)
        .transition(.opacity)
    }
}

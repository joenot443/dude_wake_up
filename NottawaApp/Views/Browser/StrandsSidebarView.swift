//
//  StrandsSidebarView.swift
//  NottawaApp
//
//  Sidebar view for browsing, loading, renaming, and deleting saved strands.
//

import SwiftUI

struct StrandsSidebarView: View {
    @Environment(NodeEditorViewModel.self) private var viewModel

    @State private var searchText = ""
    @State private var searchVisible = false
    @State private var renameStrandId: String?
    @State private var renameText = ""
    @State private var showDeleteConfirm = false
    @State private var deleteStrandId: String?

    private let columns = [
        GridItem(.adaptive(minimum: 78), spacing: 4),
    ]

    private var filteredStrands: [StrandInfo] {
        if searchText.isEmpty { return viewModel.strandsList }
        return viewModel.strandsList.filter {
            $0.name.localizedCaseInsensitiveContains(searchText)
        }
    }

    var body: some View {
        VStack(spacing: 0) {
            // Local / Community toggle
            Picker("", selection: Binding(
                get: { viewModel.strandViewMode },
                set: { viewModel.strandViewMode = $0 }
            )) {
                Text("Local").tag(NodeEditorViewModel.StrandViewMode.local)
                Text("Community").tag(NodeEditorViewModel.StrandViewMode.community)
            }
            .pickerStyle(.segmented)
            .padding(.horizontal, 10)
            .padding(.vertical, 6)

            if viewModel.strandViewMode == .community {
                CommunityStrandListView()
            } else {
                localStrandsContent
            }
        }
        .onAppear {
            viewModel.fetchStrands()
        }
        .alert("Rename Strand", isPresented: .init(
            get: { renameStrandId != nil },
            set: { if !$0 { renameStrandId = nil } }
        )) {
            TextField("Name", text: $renameText)
            Button("Cancel", role: .cancel) { renameStrandId = nil }
            Button("Rename") {
                if let id = renameStrandId, !renameText.isEmpty {
                    viewModel.renameStrand(id: id, newName: renameText)
                }
                renameStrandId = nil
            }
        } message: {
            Text("Enter a new name for this strand.")
        }
        .alert("Delete Strand?", isPresented: $showDeleteConfirm) {
            Button("Cancel", role: .cancel) { deleteStrandId = nil }
            Button("Delete", role: .destructive) {
                if let id = deleteStrandId {
                    viewModel.deleteStrand(id: id)
                }
                deleteStrandId = nil
            }
        } message: {
            Text("This strand will be permanently deleted.")
        }
    }

    // MARK: - Local Strands Content

    private var localStrandsContent: some View {
        VStack(spacing: 0) {
            // Search row
            HStack(spacing: 6) {
                Button {
                    searchVisible.toggle()
                    if !searchVisible { searchText = "" }
                } label: {
                    Image(systemName: "magnifyingglass")
                        .font(.system(size: 12))
                        .foregroundStyle(searchVisible ? Color.accentColor : .secondary)
                }
                .buttonStyle(.plain)

                Text("\(viewModel.strandsList.count) strands")
                    .font(.system(size: 11))
                    .foregroundStyle(.secondary)

                Spacer()
            }
            .padding(.horizontal, 10)
            .padding(.vertical, 4)

            if searchVisible {
                HStack(spacing: 4) {
                    TextField("Search strands...", text: $searchText)
                        .textFieldStyle(.roundedBorder)

                    Button {
                        searchText = ""
                        searchVisible = false
                    } label: {
                        Image(systemName: "xmark.circle.fill")
                            .font(.system(size: 12))
                            .foregroundStyle(.secondary)
                    }
                    .buttonStyle(.plain)
                }
                .padding(.horizontal, 10)
                .padding(.bottom, 6)
            }

            Divider()

            if filteredStrands.isEmpty {
                VStack(spacing: 8) {
                    Image(systemName: "rectangle.stack")
                        .font(.system(size: 32))
                        .foregroundStyle(.tertiary)
                    Text("No strands saved yet")
                        .font(.system(size: 13, weight: .medium))
                        .foregroundStyle(.secondary)
                    Text("Right-click a node and choose\n\"Save Strand\" to save one.")
                        .font(.system(size: 11))
                        .foregroundStyle(.tertiary)
                        .multilineTextAlignment(.center)
                }
                .frame(maxWidth: .infinity, maxHeight: .infinity)
                .padding()
            } else {
                ScrollView {
                    LazyVGrid(columns: columns, spacing: 4) {
                        ForEach(filteredStrands) { strand in
                            StrandTileView(strand: strand)
                                .contextMenu {
                                    Button("Rename...") {
                                        renameText = strand.name
                                        renameStrandId = strand.id
                                    }
                                    Divider()
                                    Button("Delete", role: .destructive) {
                                        deleteStrandId = strand.id
                                        showDeleteConfirm = true
                                    }
                                }
                        }
                    }
                    .padding(6)
                }
            }
        }
    }
}

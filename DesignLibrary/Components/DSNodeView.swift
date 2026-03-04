import SwiftUI

enum DSNodeType {
    case source
    case effect

    var emptyIcon: String {
        switch self {
        case .source: return "video.fill"
        case .effect: return "wand.and.stars"
        }
    }

    var emptyLabel: String {
        switch self {
        case .source: return "No signal"
        case .effect: return "No input"
        }
    }
}

struct DSNodeView: View {
    @Environment(ThemeManager.self) var theme

    let title: String
    let nodeId: String?
    let nodeType: DSNodeType
    let isSelected: Bool
    let isActive: Bool
    let isEmpty: Bool
    let inputPins: [DSPinState]
    let outputPins: [DSPinState]
    let auxiliaryLabel: String?
    let previewColors: [Color]

    init(
        title: String,
        nodeId: String? = nil,
        nodeType: DSNodeType = .effect,
        isSelected: Bool = false,
        isActive: Bool = true,
        isEmpty: Bool = false,
        inputPins: [DSPinState] = [.disconnected],
        outputPins: [DSPinState] = [.connected],
        auxiliaryLabel: String? = nil,
        previewColors: [Color] = [.purple, .blue]
    ) {
        self.title = title
        self.nodeId = nodeId
        self.nodeType = nodeType
        self.isSelected = isSelected
        self.isActive = isActive
        self.isEmpty = isEmpty
        self.inputPins = inputPins
        self.outputPins = outputPins
        self.auxiliaryLabel = auxiliaryLabel
        self.previewColors = previewColors
    }

    private let nodeWidth: CGFloat = 220
    private let nodeHeight: CGFloat = 160
    private let cornerRadius: CGFloat = 14
    private let pinSize: CGFloat = 10

    var body: some View {
        ZStack(alignment: .bottom) {
            if isEmpty {
                emptyState
            } else {
                previewState
            }

            // Bottom overlay: dark gradient + title + actions
            VStack(spacing: 0) {
                Spacer()
                HStack(alignment: .lastTextBaseline) {
                    Text(title)
                        .font(.system(size: 11, weight: .bold, design: theme.typography.fontDesign))
                        .foregroundStyle(.white)
                        .shadow(color: .black.opacity(0.7), radius: 2, y: 1)
                        .lineLimit(1)

                    Spacer()

                    HStack(spacing: 8) {
                        nodeAction("pause.fill")
                        nodeAction("arrow.triangle.2.circlepath")
                        nodeAction("trash", destructive: true)
                    }
                }
                .padding(.horizontal, 10)
                .padding(.bottom, 8)
                .padding(.top, 16)
                .background(
                    LinearGradient(
                        stops: [
                            .init(color: .clear, location: 0.0),
                            .init(color: .black.opacity(isEmpty ? 0.0 : 0.5), location: 0.35),
                            .init(color: .black.opacity(isEmpty ? 0.0 : 0.75), location: 1.0),
                        ],
                        startPoint: .top,
                        endPoint: .bottom
                    )
                )
            }
        }
        .frame(width: nodeWidth, height: nodeHeight)
        .clipShape(RoundedRectangle(cornerRadius: cornerRadius))
        // Border
        .overlay(
            RoundedRectangle(cornerRadius: cornerRadius)
                .strokeBorder(Color.white.opacity(isEmpty ? 0.06 : 0.1), lineWidth: 0.5)
        )
        // Selection glow border
        .overlay(selectionOverlay)
        // Input pins
        .overlay(alignment: .leading) {
            if !inputPins.isEmpty {
                VStack(spacing: 4) {
                    ForEach(Array(inputPins.enumerated()), id: \.offset) { idx, state in
                        DSPin(state: state, pinId: nodeId.map { "\($0)-in-\(idx)" })
                    }
                }
                .offset(x: -pinSize / 2)
            }
        }
        // Output pins
        .overlay(alignment: .trailing) {
            VStack(spacing: 4) {
                ForEach(Array(outputPins.enumerated()), id: \.offset) { idx, state in
                    DSPin(state: state, pinId: nodeId.map { "\($0)-out-\(idx)" })
                }
                if let auxiliaryLabel {
                    DSPin(state: .disconnected, label: auxiliaryLabel, pinId: nodeId.map { "\($0)-aux" })
                }
            }
            .offset(x: pinSize / 2)
        }
        // Shadow
        .shadow(color: .black.opacity(0.4), radius: 8, y: 4)
    }

    // MARK: - Preview State

    private var previewState: some View {
        ZStack {
            LinearGradient(
                colors: previewColors,
                startPoint: .topLeading,
                endPoint: .bottomTrailing
            )
            .opacity(isActive ? 1.0 : 0.3)

            // Inner vignette
            RoundedRectangle(cornerRadius: cornerRadius)
                .fill(
                    RadialGradient(
                        colors: [.clear, .black.opacity(0.2)],
                        center: .center,
                        startRadius: nodeWidth * 0.15,
                        endRadius: nodeWidth * 0.65
                    )
                )

            // Inactive overlay
            if !isActive {
                Image(systemName: "pause.fill")
                    .font(.system(size: 20, weight: .medium))
                    .foregroundStyle(.white.opacity(0.4))
            }
        }
    }

    // MARK: - Empty State

    private var emptyState: some View {
        ZStack {
            Color(white: 0.08)

            VStack(spacing: 6) {
                Image(systemName: nodeType.emptyIcon)
                    .font(.system(size: 22, weight: .light))
                    .foregroundStyle(.white.opacity(0.15))

                Text(nodeType.emptyLabel)
                    .font(.system(size: 9, weight: .medium, design: theme.typography.fontDesign))
                    .foregroundStyle(.white.opacity(0.2))
            }
            .offset(y: -10)
        }
    }

    // MARK: - Action Icon

    private func nodeAction(_ icon: String, destructive: Bool = false) -> some View {
        Image(systemName: icon)
            .font(.system(size: 9, weight: .medium))
            .foregroundStyle(
                destructive
                    ? Color.red.opacity(0.55)
                    : Color.white.opacity(isEmpty ? 0.25 : 0.45)
            )
    }

    // MARK: - Selection Overlay

    @ViewBuilder
    private var selectionOverlay: some View {
        if isSelected {
            RoundedRectangle(cornerRadius: cornerRadius)
                .strokeBorder(theme.colors.accent, lineWidth: 2.5)
                .shadow(color: theme.colors.accent.opacity(0.4), radius: 8)
                .animation(.easeOut(duration: 0.15), value: isSelected)
        }
    }
}

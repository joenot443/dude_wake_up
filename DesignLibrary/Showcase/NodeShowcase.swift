import SwiftUI

struct NodeShowcase: View {
    @Environment(ThemeManager.self) var theme
    @State private var selectedNodeIndex = 1

    var body: some View {
        VStack(alignment: .leading, spacing: theme.spacing.xl) {
            DSText("Nodes", style: .h2)
            DSText("Floating screen nodes — preview-dominant with edge-mounted pins and dark overlay chrome.", style: .body, color: theme.colors.textSecondary)

            sectionDivider

            // Basic node
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Basic Node", style: .h4, color: theme.colors.textTertiary)
                DSText("Preview fills the entire node. Title and actions overlaid at bottom with a dark gradient.", style: .caption)

                HStack(alignment: .top, spacing: theme.spacing.xxl) {
                    DSNodeView(
                        title: "Plasma",
                        inputPins: [.disconnected],
                        outputPins: [.connected],
                        previewColors: [Color(hex: 0x6A0DAD), Color(hex: 0x00BFFF)]
                    )

                    DSNodeView(
                        title: "Glitch RGB",
                        inputPins: [.connected],
                        outputPins: [.connected],
                        previewColors: [Color(hex: 0xFF3366), Color(hex: 0x33FF99)]
                    )
                }
            }

            sectionDivider

            // Selection state
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Selection", style: .h4, color: theme.colors.textTertiary)
                DSText("Click a node to toggle selection. Selected nodes get an accent glow border.", style: .caption)

                HStack(alignment: .top, spacing: theme.spacing.xl) {
                    ForEach(0..<3) { i in
                        let names = ["Webcam", "Chromatic", "Blur"]
                        let colors: [[Color]] = [
                            [Color(hex: 0x2196F3), Color(hex: 0x0D47A1)],
                            [Color(hex: 0xFF9800), Color(hex: 0xE65100)],
                            [Color(hex: 0x9C27B0), Color(hex: 0x4A148C)],
                        ]
                        DSNodeView(
                            title: names[i],
                            isSelected: selectedNodeIndex == i,
                            inputPins: i == 0 ? [] : [.connected],
                            outputPins: [.connected],
                            previewColors: colors[i]
                        )
                        .onTapGesture { selectedNodeIndex = i }
                    }
                }
            }

            sectionDivider

            // Active / inactive
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Active / Inactive", style: .h4, color: theme.colors.textTertiary)
                DSText("Inactive nodes dim to 30% with a centered pause indicator.", style: .caption)

                HStack(alignment: .top, spacing: theme.spacing.xxl) {
                    VStack(spacing: theme.spacing.xs) {
                        DSNodeView(
                            title: "Active",
                            isActive: true,
                            previewColors: [Color(hex: 0x00C853), Color(hex: 0x1B5E20)]
                        )
                        Text("Active")
                            .font(theme.typography.caption)
                            .foregroundStyle(theme.colors.textTertiary)
                    }
                    VStack(spacing: theme.spacing.xs) {
                        DSNodeView(
                            title: "Paused",
                            isActive: false,
                            previewColors: [Color(hex: 0x00C853), Color(hex: 0x1B5E20)]
                        )
                        Text("Inactive")
                            .font(theme.typography.caption)
                            .foregroundStyle(theme.colors.textTertiary)
                    }
                }
            }

            sectionDivider

            // Empty states
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Empty State", style: .h4, color: theme.colors.textTertiary)
                DSText("Shown when no preview is available yet.", style: .caption)

                HStack(alignment: .top, spacing: theme.spacing.xxl) {
                    VStack(spacing: theme.spacing.xs) {
                        DSNodeView(
                            title: "Webcam",
                            nodeType: .source,
                            isEmpty: true,
                            inputPins: [],
                            outputPins: [.disconnected]
                        )
                        Text("Source")
                            .font(theme.typography.caption)
                            .foregroundStyle(theme.colors.textTertiary)
                    }

                    VStack(spacing: theme.spacing.xs) {
                        DSNodeView(
                            title: "Blur",
                            nodeType: .effect,
                            isEmpty: true,
                            inputPins: [.disconnected],
                            outputPins: [.disconnected]
                        )
                        Text("Effect")
                            .font(theme.typography.caption)
                            .foregroundStyle(theme.colors.textTertiary)
                    }
                }
            }

            sectionDivider

            // Pin configurations
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Pin Configurations", style: .h4, color: theme.colors.textTertiary)
                DSText("Pins sit on the node edge — connected pins are filled, disconnected are hollow rings.", style: .caption)

                HStack(alignment: .top, spacing: theme.spacing.xl) {
                    VStack(spacing: theme.spacing.xs) {
                        DSNodeView(
                            title: "Source",
                            inputPins: [],
                            outputPins: [.connected],
                            previewColors: [Color(hex: 0x03A9F4), Color(hex: 0x01579B)]
                        )
                        Text("No inputs")
                            .font(theme.typography.caption)
                            .foregroundStyle(theme.colors.textTertiary)
                    }

                    VStack(spacing: theme.spacing.xs) {
                        DSNodeView(
                            title: "Blend",
                            inputPins: [.connected, .connected],
                            outputPins: [.connected],
                            previewColors: [Color(hex: 0xE91E63), Color(hex: 0x880E4F)]
                        )
                        Text("Two inputs")
                            .font(theme.typography.caption)
                            .foregroundStyle(theme.colors.textTertiary)
                    }

                    VStack(spacing: theme.spacing.xs) {
                        DSNodeView(
                            title: "Feedback",
                            inputPins: [.connected],
                            outputPins: [.connected],
                            auxiliaryLabel: "A",
                            previewColors: [Color(hex: 0xFFEB3B), Color(hex: 0xF57F17)]
                        )
                        Text("Auxiliary output")
                            .font(theme.typography.caption)
                            .foregroundStyle(theme.colors.textTertiary)
                    }
                }
            }

            sectionDivider

            // Pin states (enlarged for visibility)
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Pin States", style: .h4, color: theme.colors.textTertiary)
                DSText("Filled = connected. Hollow = disconnected. Green glow = snap target.", style: .caption)

                DSCard {
                    HStack(spacing: theme.spacing.xxl) {
                        pinStateDemo("Disconnected", state: .disconnected)
                        pinStateDemo("Connected", state: .connected)
                        pinStateDemo("Snapped", state: .snapped)
                        pinStateDemo("Dimmed", state: .dimmed)
                    }
                }
            }

            sectionDivider

            // Chain composition with real cables
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Chain Composition", style: .h4, color: theme.colors.textTertiary)
                DSText("Nodes connect via Bezier cables drawn between pin positions reported through PreferenceKey.", style: .caption)

                NodeChainDemo()
            }

            sectionDivider

            // Cable styles
            VStack(alignment: .leading, spacing: theme.spacing.lg) {
                DSText("Cable Styles", style: .h4, color: theme.colors.textTertiary)

                DSCard {
                    VStack(alignment: .leading, spacing: theme.spacing.lg) {
                        cableStyleRow("Established", style: .established)
                        cableStyleRow("Dimmed", style: .dimmed)
                        cableStyleRow("Dragging", style: .dragging)
                    }
                }
            }
        }
    }

    private var sectionDivider: some View {
        Rectangle()
            .fill(theme.colors.borderSubtle)
            .frame(height: 1)
    }

    private func pinStateDemo(_ label: String, state: DSPinState) -> some View {
        VStack(spacing: theme.spacing.sm) {
            DSPin(state: state)
                .scaleEffect(2.0)
                .frame(width: 24, height: 24)
            Text(label)
                .font(theme.typography.caption)
                .foregroundStyle(theme.colors.textTertiary)
        }
    }

    private func cableStyleRow(_ label: String, style: DSCableStyle) -> some View {
        HStack(spacing: theme.spacing.md) {
            Text(label)
                .font(.system(size: 11, weight: .medium, design: .monospaced))
                .foregroundStyle(theme.colors.textTertiary)
                .frame(width: 80, alignment: .trailing)

            DSCableView(
                from: CGPoint(x: 0, y: 12),
                to: CGPoint(x: 200, y: 12),
                style: style
            )
            .frame(width: 200, height: 24)
        }
    }
}

// MARK: - Chain Demo (real cables via PreferenceKey)

private struct NodeChainDemo: View {
    @Environment(ThemeManager.self) var theme
    @State private var pinPositions: [String: CGPoint] = [:]

    // Connections: output pin id -> input pin id
    private let connections: [(String, String)] = [
        ("cam-out-0", "glitch-in-0"),
        ("glitch-out-0", "blur-in-0"),
    ]

    var body: some View {
        ScrollView(.horizontal, showsIndicators: false) {
            HStack(spacing: 60) {
                DSNodeView(
                    title: "Webcam",
                    nodeId: "cam",
                    inputPins: [],
                    outputPins: [.connected],
                    previewColors: [Color(hex: 0x42A5F5), Color(hex: 0x1565C0)]
                )

                DSNodeView(
                    title: "Glitch",
                    nodeId: "glitch",
                    inputPins: [.connected],
                    outputPins: [.connected],
                    previewColors: [Color(hex: 0xEF5350), Color(hex: 0xB71C1C)]
                )

                DSNodeView(
                    title: "Blur",
                    nodeId: "blur",
                    isSelected: true,
                    inputPins: [.connected],
                    outputPins: [.connected],
                    previewColors: [Color(hex: 0xAB47BC), Color(hex: 0x6A1B9A)]
                )
            }
            .padding(.vertical, theme.spacing.md)
            .padding(.horizontal, theme.spacing.sm)
            .coordinateSpace(name: "dsChain")
            .onPreferenceChange(DSPinPositionKey.self) { positions in
                pinPositions = positions
            }
            .overlay {
                ForEach(connections, id: \.0) { outId, inId in
                    if let from = pinPositions[outId], let to = pinPositions[inId] {
                        DSCableView(from: from, to: to)
                    }
                }
            }
        }
    }
}

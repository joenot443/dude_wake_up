import SwiftUI

struct DSSlider: View {
    @Environment(ThemeManager.self) var theme

    @Binding var value: Float
    let range: ClosedRange<Float>
    let step: Float?
    let label: String?
    let showValue: Bool
    let formatString: String

    @State private var isDragging = false

    init(
        value: Binding<Float>,
        range: ClosedRange<Float> = 0...1,
        step: Float? = nil,
        label: String? = nil,
        showValue: Bool = false,
        formatString: String = "%.2f"
    ) {
        self._value = value
        self.range = range
        self.step = step
        self.label = label
        self.showValue = showValue
        self.formatString = formatString
    }

    private var fraction: CGFloat {
        let span = CGFloat(range.upperBound - range.lowerBound)
        guard span > 0 else { return 0 }
        return CGFloat(value - range.lowerBound) / span
    }

    var body: some View {
        VStack(alignment: .leading, spacing: theme.spacing.xs) {
            if label != nil || showValue {
                HStack {
                    if let label {
                        Text(label)
                            .font(theme.typography.caption)
                            .foregroundStyle(theme.colors.textSecondary)
                    }
                    Spacer()
                    if showValue {
                        Text(String(format: formatString, value))
                            .font(.system(size: theme.typography.captionSize, weight: .medium, design: .monospaced))
                            .foregroundStyle(theme.colors.textTertiary)
                    }
                }
            }

            GeometryReader { geo in
                let trackWidth = geo.size.width
                let thumbSize = theme.sizing.sliderThumbSize
                let trackHeight = theme.sizing.sliderTrackHeight
                let usableWidth = trackWidth - thumbSize
                let thumbOffset = fraction * usableWidth
                let fillWidth = thumbOffset + thumbSize / 2

                ZStack(alignment: .leading) {
                    // Track background
                    Capsule()
                        .fill(theme.colors.backgroundTertiary)
                        .frame(height: trackHeight)

                    // Track fill
                    Capsule()
                        .fill(isDragging ? theme.colors.accentHover : theme.colors.accent)
                        .frame(width: max(trackHeight, fillWidth), height: trackHeight)

                    // Thumb
                    Circle()
                        .fill(theme.colors.accent)
                        .frame(width: thumbSize, height: thumbSize)
                        .shadow(color: .black.opacity(isDragging ? 0.3 : 0.15), radius: isDragging ? 4 : 2, y: 1)
                        .scaleEffect(isDragging ? 1.15 : 1.0)
                        .offset(x: thumbOffset)
                }
                .frame(height: thumbSize)
                .contentShape(Rectangle())
                .gesture(
                    DragGesture(minimumDistance: 0, coordinateSpace: .local)
                        .onChanged { drag in
                            isDragging = true
                            let newFraction = max(0, min(1, drag.location.x / trackWidth))
                            var newValue = range.lowerBound + Float(newFraction) * (range.upperBound - range.lowerBound)
                            if let step, step > 0 {
                                newValue = (newValue / step).rounded() * step
                                newValue = min(max(newValue, range.lowerBound), range.upperBound)
                            }
                            value = newValue
                        }
                        .onEnded { _ in
                            withAnimation(.easeOut(duration: 0.2)) {
                                isDragging = false
                            }
                        }
                )
                .animation(.easeOut(duration: 0.15), value: isDragging)
            }
            .frame(height: theme.sizing.sliderThumbSize)
        }
    }
}

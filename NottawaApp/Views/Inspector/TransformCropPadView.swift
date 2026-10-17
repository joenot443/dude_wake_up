//
//  TransformCropPadView.swift
//  NottawaApp
//
//  Visual crop-rectangle editor for the Transform shader. Drives the four
//  Min X / Max X / Min Y / Max Y parameters via direct manipulation over a
//  live preview of the shader's input. Mirrors the PositionPadView pattern
//  in TextSourceControlsView.swift.
//

import SwiftUI

/// Section view shown in the parameter inspector for Transform shaders.
///
/// Owns local `@State` mirrors of the four crop parameters so drag updates
/// produce immediate SwiftUI invalidations (a custom `Binding(get:set:)`
/// pointing at the engine wouldn't — the setter has no observed storage to
/// trigger a body re-render). `.onChange(of: localValue)` pushes to the
/// engine; `.onChange(of: param.value)` pulls the engine's value back when
/// it changes externally (e.g. inspector refresh after another control
/// edits the same param, or an oscillator mutating it).
///
/// The pad backdrop renders the *input* frame (whatever's connected upstream
/// of the Transform), not the Transform's own output — so the crop rectangle
/// always sits over the pre-crop pixels you're selecting from. Falls back to
/// an empty placeholder if no upstream node is connected.
struct TransformCropPadView: View {
    @Environment(ThemeManager.self) private var theme

    let nodeId: String
    /// Connectable id whose output feeds this Transform's main input. Used as
    /// the live preview backdrop for the crop rect.
    let inputConnectableId: String?
    let minXParam: ParameterInfo
    let maxXParam: ParameterInfo
    let minYParam: ParameterInfo
    let maxYParam: ParameterInfo

    @State private var minX: Float
    @State private var maxX: Float
    @State private var minY: Float
    @State private var maxY: Float

    private let engine = NottawaEngine.shared

    init(nodeId: String,
         inputConnectableId: String?,
         minXParam: ParameterInfo,
         maxXParam: ParameterInfo,
         minYParam: ParameterInfo,
         maxYParam: ParameterInfo) {
        self.nodeId = nodeId
        self.inputConnectableId = inputConnectableId
        self.minXParam = minXParam
        self.maxXParam = maxXParam
        self.minYParam = minYParam
        self.maxYParam = maxYParam
        self._minX = State(initialValue: minXParam.value)
        self._maxX = State(initialValue: maxXParam.value)
        self._minY = State(initialValue: minYParam.value)
        self._maxY = State(initialValue: maxYParam.value)
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            HStack(spacing: 6) {
                Text("Crop")
                    .font(.subheadline)
                    .fontWeight(.semibold)
                    .foregroundStyle(theme.colors.textSecondary)
                Spacer()
                Button {
                    minX = 0; maxX = 1; minY = 0; maxY = 1
                } label: {
                    Image(systemName: "arrow.counterclockwise")
                        .font(.caption2)
                        .foregroundStyle(theme.colors.textTertiary)
                }
                .buttonStyle(.plainHitArea)
                .help("Reset crop to full frame")
            }

            CropRectPadView(
                minX: $minX,
                maxX: $maxX,
                minY: $minY,
                maxY: $maxY,
                connectableId: inputConnectableId
            )

            HStack {
                Text(String(format: "X %.2f – %.2f", minX, maxX))
                    .font(.caption2)
                    .foregroundStyle(theme.colors.textTertiary)
                    .monospacedDigit()
                Spacer()
                Text(String(format: "Y %.2f – %.2f", minY, maxY))
                    .font(.caption2)
                    .foregroundStyle(theme.colors.textTertiary)
                    .monospacedDigit()
            }
        }
        // Local → engine
        .onChange(of: minX) { _, new in
            engine.setParameterValue(paramId: minXParam.id, value: new)
        }
        .onChange(of: maxX) { _, new in
            engine.setParameterValue(paramId: maxXParam.id, value: new)
        }
        .onChange(of: minY) { _, new in
            engine.setParameterValue(paramId: minYParam.id, value: new)
        }
        .onChange(of: maxY) { _, new in
            engine.setParameterValue(paramId: maxYParam.id, value: new)
        }
        // Engine → local (skip if equal, otherwise the .onChange above fires
        // back into the engine and we get a redundant bridge call per tick).
        .onChange(of: minXParam.value) { _, new in if minX != new { minX = new } }
        .onChange(of: maxXParam.value) { _, new in if maxX != new { maxX = new } }
        .onChange(of: minYParam.value) { _, new in if minY != new { minY = new } }
        .onChange(of: maxYParam.value) { _, new in if maxY != new { maxY = new } }
        // Synthetic input ids (e.g. "sourceRaw:<id>" for sources that own
        // their pre-transform feed) aren't auto-shared by the ViewModel —
        // start sharing while the pad is on screen so the producer knows
        // to blit. Real upstream connectable ids manage themselves via the
        // ViewModel's auto-share, so skip those.
        .onAppear {
            if let id = inputConnectableId, id.contains(":") {
                engine.startTextureSharing(connectableId: id)
            }
        }
        .onDisappear {
            if let id = inputConnectableId, id.contains(":") {
                engine.stopTextureSharing(connectableId: id)
            }
        }
    }
}

// MARK: - Crop Rect Pad

/// Reusable rectangular selection pad over a live preview. Mirrors
/// PositionPadView's structure: 16:9 aspect, optional TextureDisplayView
/// backdrop, drag-to-edit. Edges and corners are draggable handles; the
/// rect body itself can be dragged to translate without resizing.
struct CropRectPadView: View {
    @Environment(ThemeManager.self) private var theme

    @Binding var minX: Float
    @Binding var maxX: Float
    @Binding var minY: Float
    @Binding var maxY: Float
    var connectableId: String? = nil

    private let padCornerRadius: CGFloat = 6
    private let handleSize: CGFloat = 10
    /// Pixel slack around handles/edges for hit testing.
    private let hitSlop: CGFloat = 8
    /// Don't allow the crop to collapse to nothing (in normalized units).
    private let minSize: Float = 0.02

    @State private var dragMode: DragMode = .none
    @State private var dragStart: CropSnapshot?

    private enum DragMode: Equatable {
        case none
        case body
        case edgeLeft, edgeRight, edgeTop, edgeBottom
        case cornerTL, cornerTR, cornerBL, cornerBR
    }

    private struct CropSnapshot {
        let minX: Float
        let maxX: Float
        let minY: Float
        let maxY: Float
        let pointerStart: CGPoint
    }

    var body: some View {
        GeometryReader { geo in
            let w = geo.size.width
            let h = geo.size.height
            let rect = CGRect(
                x: CGFloat(minX) * w,
                y: CGFloat(minY) * h,
                width: CGFloat(max(0.0, maxX - minX)) * w,
                height: CGFloat(max(0.0, maxY - minY)) * h
            )

            ZStack {
                // Live preview / fallback background
                if let cid = connectableId {
                    TextureDisplayView(connectableId: cid)
                        .allowsHitTesting(false)
                        .clipShape(RoundedRectangle(cornerRadius: padCornerRadius))
                } else {
                    RoundedRectangle(cornerRadius: padCornerRadius)
                        .fill(theme.colors.surface)
                }

                // Dimmed mask outside the crop rect, plus rule-of-thirds
                // guides inside it. One Canvas (single CGContext) — cheaper
                // than splitting across two during high-frequency redraws.
                Canvas { ctx, size in
                    var outer = Path(roundedRect: CGRect(origin: .zero, size: size),
                                     cornerRadius: padCornerRadius)
                    outer.addRect(rect)
                    ctx.fill(outer, with: .color(.black.opacity(0.55)), style: FillStyle(eoFill: true))

                    if rect.width > 4, rect.height > 4 {
                        for i in 1..<3 {
                            let x = rect.minX + rect.width * CGFloat(i) / 3.0
                            var p = Path()
                            p.move(to: CGPoint(x: x, y: rect.minY))
                            p.addLine(to: CGPoint(x: x, y: rect.maxY))
                            ctx.stroke(p, with: .color(.white.opacity(0.18)), lineWidth: 0.5)
                        }
                        for i in 1..<3 {
                            let y = rect.minY + rect.height * CGFloat(i) / 3.0
                            var p = Path()
                            p.move(to: CGPoint(x: rect.minX, y: y))
                            p.addLine(to: CGPoint(x: rect.maxX, y: y))
                            ctx.stroke(p, with: .color(.white.opacity(0.18)), lineWidth: 0.5)
                        }
                    }
                }
                .allowsHitTesting(false)

                // Crop rectangle border
                Rectangle()
                    .strokeBorder(theme.colors.accent, lineWidth: 1.5)
                    .frame(width: rect.width, height: rect.height)
                    .position(x: rect.midX, y: rect.midY)
                    .allowsHitTesting(false)

                // Corner handles
                ForEach(corners(rect: rect), id: \.0) { _, point in
                    handle()
                        .position(point)
                        .allowsHitTesting(false)
                }

                // Edge handles (midpoints)
                ForEach(edgeMidpoints(rect: rect), id: \.0) { _, point in
                    handle()
                        .position(point)
                        .allowsHitTesting(false)
                }

                // Outer pad border
                RoundedRectangle(cornerRadius: padCornerRadius)
                    .strokeBorder(theme.colors.border, lineWidth: 0.5)
                    .allowsHitTesting(false)
            }
            .contentShape(Rectangle())
            .gesture(
                DragGesture(minimumDistance: 0, coordinateSpace: .local)
                    .onChanged { value in
                        if dragStart == nil {
                            dragMode = hitTest(point: value.startLocation, rect: rect)
                            dragStart = CropSnapshot(
                                minX: minX, maxX: maxX,
                                minY: minY, maxY: maxY,
                                pointerStart: value.startLocation
                            )
                        }
                        applyDrag(value: value, w: w, h: h)
                    }
                    .onEnded { _ in
                        dragMode = .none
                        dragStart = nil
                    }
            )
        }
        .aspectRatio(16.0 / 9.0, contentMode: .fit)
    }

    // MARK: - Hit testing & drag application

    private func hitTest(point: CGPoint, rect: CGRect) -> DragMode {
        let slop = handleSize / 2 + hitSlop

        // Corners take priority
        let corners: [(DragMode, CGPoint)] = [
            (.cornerTL, CGPoint(x: rect.minX, y: rect.minY)),
            (.cornerTR, CGPoint(x: rect.maxX, y: rect.minY)),
            (.cornerBL, CGPoint(x: rect.minX, y: rect.maxY)),
            (.cornerBR, CGPoint(x: rect.maxX, y: rect.maxY)),
        ]
        for (mode, p) in corners where hypot(p.x - point.x, p.y - point.y) <= slop {
            return mode
        }

        // Edges next — within slop of the line, and inside the perpendicular extent
        if abs(point.x - rect.minX) <= slop, point.y > rect.minY - slop, point.y < rect.maxY + slop {
            return .edgeLeft
        }
        if abs(point.x - rect.maxX) <= slop, point.y > rect.minY - slop, point.y < rect.maxY + slop {
            return .edgeRight
        }
        if abs(point.y - rect.minY) <= slop, point.x > rect.minX - slop, point.x < rect.maxX + slop {
            return .edgeTop
        }
        if abs(point.y - rect.maxY) <= slop, point.x > rect.minX - slop, point.x < rect.maxX + slop {
            return .edgeBottom
        }

        // Inside body → translate. Outside → also treat as body so a click
        // in the dimmed region drags the whole rect along with the cursor.
        return .body
    }

    private func applyDrag(value: DragGesture.Value, w: CGFloat, h: CGFloat) {
        guard let start = dragStart, w > 0, h > 0 else { return }
        let dxNorm = Float((value.location.x - start.pointerStart.x) / w)
        let dyNorm = Float((value.location.y - start.pointerStart.y) / h)

        var newMinX = start.minX
        var newMaxX = start.maxX
        var newMinY = start.minY
        var newMaxY = start.maxY

        switch dragMode {
        case .none:
            return
        case .body:
            // Translate, clamping to [0, 1] without changing size
            let width = start.maxX - start.minX
            let height = start.maxY - start.minY
            newMinX = clamp(start.minX + dxNorm, 0, 1 - width)
            newMaxX = newMinX + width
            newMinY = clamp(start.minY + dyNorm, 0, 1 - height)
            newMaxY = newMinY + height
        case .edgeLeft:
            newMinX = clamp(start.minX + dxNorm, 0, start.maxX - minSize)
        case .edgeRight:
            newMaxX = clamp(start.maxX + dxNorm, start.minX + minSize, 1)
        case .edgeTop:
            newMinY = clamp(start.minY + dyNorm, 0, start.maxY - minSize)
        case .edgeBottom:
            newMaxY = clamp(start.maxY + dyNorm, start.minY + minSize, 1)
        case .cornerTL:
            newMinX = clamp(start.minX + dxNorm, 0, start.maxX - minSize)
            newMinY = clamp(start.minY + dyNorm, 0, start.maxY - minSize)
        case .cornerTR:
            newMaxX = clamp(start.maxX + dxNorm, start.minX + minSize, 1)
            newMinY = clamp(start.minY + dyNorm, 0, start.maxY - minSize)
        case .cornerBL:
            newMinX = clamp(start.minX + dxNorm, 0, start.maxX - minSize)
            newMaxY = clamp(start.maxY + dyNorm, start.minY + minSize, 1)
        case .cornerBR:
            newMaxX = clamp(start.maxX + dxNorm, start.minX + minSize, 1)
            newMaxY = clamp(start.maxY + dyNorm, start.minY + minSize, 1)
        }

        if newMinX != minX { minX = newMinX }
        if newMaxX != maxX { maxX = newMaxX }
        if newMinY != minY { minY = newMinY }
        if newMaxY != maxY { maxY = newMaxY }
    }

    private func clamp(_ v: Float, _ lo: Float, _ hi: Float) -> Float {
        max(lo, min(hi, v))
    }

    // MARK: - Handle geometry

    private func corners(rect: CGRect) -> [(String, CGPoint)] {
        [
            ("tl", CGPoint(x: rect.minX, y: rect.minY)),
            ("tr", CGPoint(x: rect.maxX, y: rect.minY)),
            ("bl", CGPoint(x: rect.minX, y: rect.maxY)),
            ("br", CGPoint(x: rect.maxX, y: rect.maxY)),
        ]
    }

    private func edgeMidpoints(rect: CGRect) -> [(String, CGPoint)] {
        [
            ("l", CGPoint(x: rect.minX, y: rect.midY)),
            ("r", CGPoint(x: rect.maxX, y: rect.midY)),
            ("t", CGPoint(x: rect.midX, y: rect.minY)),
            ("b", CGPoint(x: rect.midX, y: rect.maxY)),
        ]
    }

    @ViewBuilder
    private func handle() -> some View {
        ZStack {
            Rectangle()
                .fill(theme.colors.accent)
                .frame(width: handleSize, height: handleSize)
            Rectangle()
                .strokeBorder(Color.white, lineWidth: 1)
                .frame(width: handleSize, height: handleSize)
        }
        .shadow(color: .black.opacity(0.5), radius: 2, x: 0, y: 1)
    }
}

// MARK: - Transform Place Pad

/// Section view for positioning + scaling the cropped frame on the canvas.
/// Only shown when Transform's `Mode` is Standard — the other modes auto-fit
/// the cropped content, making manual placement irrelevant.
///
/// The pad represents the canvas; the inner rectangle represents where the
/// cropped frame lands. Drag the body to translate; drag a corner to scale
/// uniformly anchored at the opposite corner. The rect can extend beyond
/// the canvas bounds at large scales — that's intentional, mirroring the
/// shader's actual behavior (`TransformShader.hpp:122-126`).
struct TransformPlacePadView: View {
    @Environment(ThemeManager.self) private var theme

    let nodeId: String
    let translateXParam: ParameterInfo
    let translateYParam: ParameterInfo
    let scaleParam: ParameterInfo
    /// Crop dimensions as fractions of the input frame (used to compute the
    /// inner rect's aspect / size proportional to scale).
    let cropWidthFrac: Float
    let cropHeightFrac: Float

    @State private var translateX: Float
    @State private var translateY: Float
    @State private var scale: Float

    private let engine = NottawaEngine.shared
    private let padCornerRadius: CGFloat = 6
    private let handleSize: CGFloat = 10
    private let hitSlop: CGFloat = 8
    private let minScale: Float = 0.05

    /// Synthetic IOSurface id for TransformShader's intermediate cropped FBO.
    /// Must match the prefix used by `TransformShader::shade` on the C++ side
    /// (`"transformCrop:" + shaderId`).
    private var croppedSurfaceId: String { "transformCrop:\(nodeId)" }

    init(nodeId: String,
         translateXParam: ParameterInfo,
         translateYParam: ParameterInfo,
         scaleParam: ParameterInfo,
         cropWidthFrac: Float,
         cropHeightFrac: Float) {
        self.nodeId = nodeId
        self.translateXParam = translateXParam
        self.translateYParam = translateYParam
        self.scaleParam = scaleParam
        self.cropWidthFrac = cropWidthFrac
        self.cropHeightFrac = cropHeightFrac
        self._translateX = State(initialValue: translateXParam.value)
        self._translateY = State(initialValue: translateYParam.value)
        self._scale = State(initialValue: scaleParam.value)
    }

    @State private var dragMode: PlaceDragMode = .none
    @State private var dragStart: PlaceSnapshot?

    private enum PlaceDragMode: Equatable {
        case none, body
        case cornerTL, cornerTR, cornerBL, cornerBR
    }

    private struct PlaceSnapshot {
        let translateX: Float
        let translateY: Float
        let scale: Float
        let pointer: CGPoint
    }

    var body: some View {
        VStack(alignment: .leading, spacing: 6) {
            HStack(spacing: 6) {
                Text("Place")
                    .font(.subheadline)
                    .fontWeight(.semibold)
                    .foregroundStyle(theme.colors.textSecondary)
                Spacer()
                Button {
                    translateX = 0; translateY = 0; scale = 1
                } label: {
                    Image(systemName: "arrow.counterclockwise")
                        .font(.caption2)
                        .foregroundStyle(theme.colors.textTertiary)
                }
                .buttonStyle(.plainHitArea)
                .help("Reset position and scale")
            }

            GeometryReader { geo in
                let w = geo.size.width
                let h = geo.size.height
                let rectW = CGFloat(max(0, cropWidthFrac * scale)) * w
                let rectH = CGFloat(max(0, cropHeightFrac * scale)) * h
                let rectX = CGFloat(translateX) * w
                let rectY = CGFloat(translateY) * h
                let rect = CGRect(x: rectX, y: rectY, width: rectW, height: rectH)

                ZStack {
                    // Clean canvas background — represents the *empty* canvas
                    // that the cropped frame will be placed onto. Distinct
                    // from the cropped-content thumbnail rendered inside the
                    // placement rect below.
                    RoundedRectangle(cornerRadius: padCornerRadius)
                        .fill(theme.colors.surface)

                    // Cropped content thumbnail (F2) rendered exactly inside
                    // the placement rect. This is the cropped-but-not-placed
                    // intermediate frame from TransformShader, exposed via
                    // IOSurface under the synthetic id below.
                    TextureDisplayView(connectableId: croppedSurfaceId)
                        .allowsHitTesting(false)
                        .frame(width: max(0, rect.width), height: max(0, rect.height))
                        .position(x: rect.midX, y: rect.midY)
                        .clipped()

                    // Placement rectangle border (drawn on top of thumbnail)
                    Rectangle()
                        .strokeBorder(theme.colors.accent, lineWidth: 1.5)
                        .frame(width: rect.width, height: rect.height)
                        .position(x: rect.midX, y: rect.midY)
                        .allowsHitTesting(false)

                    // Corner handles
                    handle().position(x: rect.minX, y: rect.minY).allowsHitTesting(false)
                    handle().position(x: rect.maxX, y: rect.minY).allowsHitTesting(false)
                    handle().position(x: rect.minX, y: rect.maxY).allowsHitTesting(false)
                    handle().position(x: rect.maxX, y: rect.maxY).allowsHitTesting(false)

                    // Outer pad border
                    RoundedRectangle(cornerRadius: padCornerRadius)
                        .strokeBorder(theme.colors.border, lineWidth: 0.5)
                        .allowsHitTesting(false)
                }
                .clipShape(RoundedRectangle(cornerRadius: padCornerRadius))
                .contentShape(Rectangle())
                .gesture(
                    DragGesture(minimumDistance: 0, coordinateSpace: .local)
                        .onChanged { value in
                            if dragStart == nil {
                                dragMode = hitTest(point: value.startLocation, rect: rect)
                                dragStart = PlaceSnapshot(
                                    translateX: translateX,
                                    translateY: translateY,
                                    scale: scale,
                                    pointer: value.startLocation
                                )
                            }
                            applyDrag(value: value, w: w, h: h)
                        }
                        .onEnded { _ in
                            dragMode = .none
                            dragStart = nil
                        }
                )
            }
            .aspectRatio(16.0 / 9.0, contentMode: .fit)

            HStack {
                Text(String(format: "X %.2f  Y %.2f", translateX, translateY))
                    .font(.caption2)
                    .foregroundStyle(theme.colors.textTertiary)
                    .monospacedDigit()
                Spacer()
                Text(String(format: "Scale %.2f", scale))
                    .font(.caption2)
                    .foregroundStyle(theme.colors.textTertiary)
                    .monospacedDigit()
            }
        }
        // Local → engine
        .onChange(of: translateX) { _, n in
            engine.setParameterValue(paramId: translateXParam.id, value: n)
        }
        .onChange(of: translateY) { _, n in
            engine.setParameterValue(paramId: translateYParam.id, value: n)
        }
        .onChange(of: scale) { _, n in
            engine.setParameterValue(paramId: scaleParam.id, value: n)
        }
        // Engine → local (oscillators / external edits)
        .onChange(of: translateXParam.value) { _, n in if translateX != n { translateX = n } }
        .onChange(of: translateYParam.value) { _, n in if translateY != n { translateY = n } }
        .onChange(of: scaleParam.value) { _, n in if scale != n { scale = n } }
        // IOSurface for the cropped intermediate frame: subscribe while the
        // pad is on screen so TransformShader knows to blit each frame.
        .onAppear {
            engine.startTextureSharing(connectableId: croppedSurfaceId)
        }
        .onDisappear {
            engine.stopTextureSharing(connectableId: croppedSurfaceId)
        }
    }

    // MARK: - Hit testing

    private func hitTest(point: CGPoint, rect: CGRect) -> PlaceDragMode {
        let slop = handleSize / 2 + hitSlop
        let corners: [(PlaceDragMode, CGPoint)] = [
            (.cornerTL, CGPoint(x: rect.minX, y: rect.minY)),
            (.cornerTR, CGPoint(x: rect.maxX, y: rect.minY)),
            (.cornerBL, CGPoint(x: rect.minX, y: rect.maxY)),
            (.cornerBR, CGPoint(x: rect.maxX, y: rect.maxY)),
        ]
        for (mode, p) in corners where hypot(p.x - point.x, p.y - point.y) <= slop {
            return mode
        }
        return .body
    }

    private func applyDrag(value: DragGesture.Value, w: CGFloat, h: CGFloat) {
        guard let start = dragStart, w > 0, h > 0 else { return }
        let dx = Float((value.location.x - start.pointer.x) / w)
        let dy = Float((value.location.y - start.pointer.y) / h)

        // Original geometry (pre-drag) used to anchor scale operations.
        let origW = max(0.001, cropWidthFrac * start.scale)   // fraction of pad
        let origH = max(0.001, cropHeightFrac * start.scale)

        switch dragMode {
        case .none:
            return

        case .body:
            translateX = clamp(start.translateX + dx, -2, 2)
            translateY = clamp(start.translateY + dy, -2, 2)

        case .cornerBR:
            // Anchor TL. New scale derived from cursor position relative to TL.
            // Use whichever dimension grew/shrunk more, so the rect stays under
            // or close to the cursor.
            let newW = max(0.0, origW + dx)
            let newH = max(0.0, origH + dy)
            let sx = newW / max(0.001, cropWidthFrac)
            let sy = newH / max(0.001, cropHeightFrac)
            scale = clamp(max(sx, sy), minScale, 5.0)

        case .cornerTL:
            // Anchor BR. As cursor moves right/down (positive dx/dy) the rect
            // shrinks; as it moves left/up (negative dx/dy) the rect grows.
            let newW = max(0.0, origW - dx)
            let newH = max(0.0, origH - dy)
            let sx = newW / max(0.001, cropWidthFrac)
            let sy = newH / max(0.001, cropHeightFrac)
            let newScale = clamp(max(sx, sy), minScale, 5.0)
            // Keep BR fixed: shift translate so BR stays where it was.
            let actualNewW = cropWidthFrac * newScale
            let actualNewH = cropHeightFrac * newScale
            let brX = start.translateX + origW
            let brY = start.translateY + origH
            translateX = brX - actualNewW
            translateY = brY - actualNewH
            scale = newScale

        case .cornerTR:
            // Anchor BL.
            let newW = max(0.0, origW + dx)
            let newH = max(0.0, origH - dy)
            let sx = newW / max(0.001, cropWidthFrac)
            let sy = newH / max(0.001, cropHeightFrac)
            let newScale = clamp(max(sx, sy), minScale, 5.0)
            let actualNewH = cropHeightFrac * newScale
            let blY = start.translateY + origH
            translateY = blY - actualNewH
            scale = newScale

        case .cornerBL:
            // Anchor TR.
            let newW = max(0.0, origW - dx)
            let newH = max(0.0, origH + dy)
            let sx = newW / max(0.001, cropWidthFrac)
            let sy = newH / max(0.001, cropHeightFrac)
            let newScale = clamp(max(sx, sy), minScale, 5.0)
            let actualNewW = cropWidthFrac * newScale
            let trX = start.translateX + origW
            translateX = trX - actualNewW
            scale = newScale
        }
    }

    private func clamp(_ v: Float, _ lo: Float, _ hi: Float) -> Float {
        max(lo, min(hi, v))
    }

    @ViewBuilder
    private func handle() -> some View {
        ZStack {
            Rectangle()
                .fill(theme.colors.accent)
                .frame(width: handleSize, height: handleSize)
            Rectangle()
                .strokeBorder(Color.white, lineWidth: 1)
                .frame(width: handleSize, height: handleSize)
        }
        .shadow(color: .black.opacity(0.5), radius: 2, x: 0, y: 1)
    }
}

//
//  TextureDisplayView.swift
//  NottawaApp
//
//  Displays a live Metal texture from the C++ engine via IOSurface.
//  Uses MTKView for efficient 60fps rendering with display link.
//

import SwiftUI
import MetalKit

/// MTKView subclass that never accepts mouse events, letting SwiftUI
/// gestures on parent views handle all interaction.
final class PassthroughMTKView: MTKView {
    override func hitTest(_ point: NSPoint) -> NSView? { nil }
    override var acceptsFirstResponder: Bool { false }
}

struct TextureDisplayView: NSViewRepresentable {
    let connectableId: String

    func makeCoordinator() -> Coordinator {
        Coordinator(connectableId: connectableId)
    }

    func makeNSView(context: Context) -> PassthroughMTKView {
        let mtkView = PassthroughMTKView()
        mtkView.device = TextureProvider.shared.device
        mtkView.delegate = context.coordinator
        mtkView.colorPixelFormat = .bgra8Unorm
        mtkView.framebufferOnly = true
        mtkView.isPaused = false
        mtkView.enableSetNeedsDisplay = false
        mtkView.preferredFramesPerSecond = 60
        mtkView.layer?.isOpaque = false
        return mtkView
    }

    func updateNSView(_ nsView: PassthroughMTKView, context: Context) {
        context.coordinator.connectableId = connectableId
    }

    // MARK: - Coordinator (MTKViewDelegate)

    final class Coordinator: NSObject, MTKViewDelegate {
        var connectableId: String
        private var pipelineState: MTLRenderPipelineState?
        private var vertexBuffer: MTLBuffer?
        private let commandQueue: MTLCommandQueue

        init(connectableId: String) {
            self.connectableId = connectableId
            self.commandQueue = TextureProvider.shared.commandQueue
            super.init()
            setupPipeline()
        }

        private func setupPipeline() {
            let device = TextureProvider.shared.device

            // Compile shader source at runtime — no .metal file needed
            let shaderSource = """
            #include <metal_stdlib>
            using namespace metal;

            struct VertexOut {
                float4 position [[position]];
                float2 texCoord;
            };

            vertex VertexOut vertexShader(uint vertexID [[vertex_id]],
                                          constant float4 *vertices [[buffer(0)]]) {
                VertexOut out;
                out.position = float4(vertices[vertexID].xy, 0.0, 1.0);
                // Flip V coordinate for OpenGL -> Metal Y-axis difference
                out.texCoord = float2(vertices[vertexID].z, 1.0 - vertices[vertexID].w);
                return out;
            }

            fragment float4 fragmentShader(VertexOut in [[stage_in]],
                                           texture2d<float> tex [[texture(0)]]) {
                constexpr sampler s(mag_filter::linear, min_filter::linear);
                return tex.sample(s, in.texCoord);
            }
            """

            let options = MTLCompileOptions()
            guard let library = try? device.makeLibrary(source: shaderSource, options: options) else {
                print("TextureDisplayView: Failed to compile Metal shaders")
                return
            }

            let descriptor = MTLRenderPipelineDescriptor()
            descriptor.vertexFunction = library.makeFunction(name: "vertexShader")
            descriptor.fragmentFunction = library.makeFunction(name: "fragmentShader")
            descriptor.colorAttachments[0].pixelFormat = .bgra8Unorm

            pipelineState = try? device.makeRenderPipelineState(descriptor: descriptor)

            // Fullscreen quad: xy position + uv texcoord packed as float4
            let vertices: [SIMD4<Float>] = [
                SIMD4(-1,  1, 0, 1),  // top-left
                SIMD4(-1, -1, 0, 0),  // bottom-left
                SIMD4( 1,  1, 1, 1),  // top-right
                SIMD4(-1, -1, 0, 0),  // bottom-left
                SIMD4( 1, -1, 1, 0),  // bottom-right
                SIMD4( 1,  1, 1, 1),  // top-right
            ]
            vertexBuffer = device.makeBuffer(
                bytes: vertices,
                length: vertices.count * MemoryLayout<SIMD4<Float>>.stride,
                options: .storageModeShared
            )
        }

        func mtkView(_ view: MTKView, drawableSizeWillChange size: CGSize) {}

        func draw(in view: MTKView) {
            guard let pipelineState,
                  let vertexBuffer,
                  let texture = TextureProvider.shared.texture(for: connectableId),
                  let drawable = view.currentDrawable,
                  let passDescriptor = view.currentRenderPassDescriptor else { return }

            passDescriptor.colorAttachments[0].clearColor = MTLClearColor(red: 0, green: 0, blue: 0, alpha: 1)
            passDescriptor.colorAttachments[0].loadAction = .clear

            guard let commandBuffer = commandQueue.makeCommandBuffer(),
                  let encoder = commandBuffer.makeRenderCommandEncoder(descriptor: passDescriptor) else { return }

            encoder.setRenderPipelineState(pipelineState)
            encoder.setVertexBuffer(vertexBuffer, offset: 0, index: 0)
            encoder.setFragmentTexture(texture, index: 0)
            encoder.drawPrimitives(type: .triangle, vertexStart: 0, vertexCount: 6)
            encoder.endEncoding()

            commandBuffer.present(drawable)
            commandBuffer.commit()
        }
    }
}

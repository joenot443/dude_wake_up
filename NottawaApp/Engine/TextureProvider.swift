//
//  TextureProvider.swift
//  NottawaApp
//
//  Manages Metal textures backed by IOSurfaces from the C++ engine.
//  MTLTextures created from IOSurface auto-reflect OpenGL writes — no
//  per-frame recreation needed.
//

import Foundation
import Metal
import IOSurface
import CoreGraphics

final class TextureProvider {
    static let shared = TextureProvider()

    let device: MTLDevice
    let commandQueue: MTLCommandQueue

    /// Cached MTLTextures keyed by connectable ID.
    /// Invalidated when the IOSurfaceRef changes (e.g. resolution change).
    private var cache: [String: CachedTexture] = [:]

    private struct CachedTexture {
        let texture: MTLTexture
        let surface: IOSurfaceRef
    }

    private init() {
        guard let device = MTLCreateSystemDefaultDevice() else {
            fatalError("Metal is not supported on this device")
        }
        self.device = device
        self.commandQueue = device.makeCommandQueue()!
    }

    /// Get or create an MTLTexture for a connectable's shared IOSurface.
    /// Returns nil if the connectable is not being shared or not yet set up.
    func texture(for connectableId: String) -> MTLTexture? {
        let engine = NottawaEngine.shared

        guard let surface = engine.connectableIOSurface(connectableId: connectableId) else {
            // Not yet available; clear cache if stale
            cache.removeValue(forKey: connectableId)
            return nil
        }

        // Return cached texture if the IOSurface hasn't changed
        if let cached = cache[connectableId],
           cached.surface === surface {
            return cached.texture
        }

        // Create new MTLTexture from IOSurface
        let width = engine.textureWidth(connectableId: connectableId)
        let height = engine.textureHeight(connectableId: connectableId)
        guard width > 0, height > 0 else { return nil }

        let descriptor = MTLTextureDescriptor.texture2DDescriptor(
            pixelFormat: .bgra8Unorm,
            width: width,
            height: height,
            mipmapped: false
        )
        descriptor.usage = [.shaderRead]
        descriptor.storageMode = .managed

        guard let texture = device.makeTexture(
            descriptor: descriptor,
            iosurface: surface,
            plane: 0
        ) else { return nil }

        cache[connectableId] = CachedTexture(texture: texture, surface: surface)
        return texture
    }

    /// Capture a snapshot of the current texture as a CGImage.
    func snapshot(for connectableId: String) -> CGImage? {
        guard let texture = texture(for: connectableId) else { return nil }

        let width = texture.width
        let height = texture.height
        guard width > 0, height > 0 else { return nil }

        let bytesPerRow = width * 4
        var pixelData = [UInt8](repeating: 0, count: bytesPerRow * height)

        texture.getBytes(
            &pixelData,
            bytesPerRow: bytesPerRow,
            from: MTLRegionMake2D(0, 0, width, height),
            mipmapLevel: 0
        )

        let colorSpace = CGColorSpaceCreateDeviceRGB()
        // bgra8Unorm: byteOrder32Little + premultipliedFirst = BGRA in memory
        let bitmapInfo = CGBitmapInfo.byteOrder32Little.rawValue |
                         CGImageAlphaInfo.premultipliedFirst.rawValue

        guard let provider = CGDataProvider(data: Data(pixelData) as CFData),
              let image = CGImage(
                width: width,
                height: height,
                bitsPerComponent: 8,
                bitsPerPixel: 32,
                bytesPerRow: bytesPerRow,
                space: colorSpace,
                bitmapInfo: CGBitmapInfo(rawValue: bitmapInfo),
                provider: provider,
                decode: nil,
                shouldInterpolate: true,
                intent: .defaultIntent
              ) else { return nil }

        return image
    }

    /// Remove cached texture for a connectable.
    func invalidate(connectableId: String) {
        cache.removeValue(forKey: connectableId)
    }

    /// Clear all cached textures.
    func invalidateAll() {
        cache.removeAll()
    }
}

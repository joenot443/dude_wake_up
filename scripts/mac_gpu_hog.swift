// Keeps the GPU saturated for N seconds (default 30), as another busy app
// would, so the self-test can check that the UI doesn't wait on the GPU.
//   swiftc -O scripts/mac_gpu_hog.swift -o <out> && <out> 30
import Foundation
import Metal

let seconds = Double(CommandLine.arguments.dropFirst().first ?? "") ?? 30
let source = """
#include <metal_stdlib>
using namespace metal;
kernel void burn(device float* out [[buffer(0)]], uint id [[thread_position_in_grid]]) {
    float x = float(id) * 1e-6;
    for (int i = 0; i < 20000; i++) x = sin(x) * 1.0001 + cos(x * 0.5);
    out[id % 1024] = x;
}
"""
guard let device = MTLCreateSystemDefaultDevice(), let queue = device.makeCommandQueue() else { exit(1) }
let pipeline = try device.makeComputePipelineState(function: device.makeLibrary(source: source, options: nil).makeFunction(name: "burn")!)
let buffer = device.makeBuffer(length: 4096)!
let end = Date().addingTimeInterval(seconds)
let inFlight = DispatchSemaphore(value: 3)
while Date() < end {
    inFlight.wait()
    let cb = queue.makeCommandBuffer()!
    let enc = cb.makeComputeCommandEncoder()!
    enc.setComputePipelineState(pipeline)
    enc.setBuffer(buffer, offset: 0, index: 0)
    enc.dispatchThreads(MTLSize(width: 1 << 20, height: 1, depth: 1), threadsPerThreadgroup: MTLSize(width: 256, height: 1, depth: 1))
    enc.endEncoding()
    cb.addCompletedHandler { _ in inFlight.signal() }
    cb.commit()
}

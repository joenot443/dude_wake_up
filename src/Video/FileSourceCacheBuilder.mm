#include "FileSourceCacheBuilder.hpp"
#include "ofPixels.h"
#include "ofTexture.h"

#include <cstring>

#import <AVFoundation/AVFoundation.h>
#import <CoreVideo/CoreVideo.h>
#import <Foundation/Foundation.h>

namespace nottawa {

FrameCacheResult buildFrameCacheFromAsset(const std::string& videoPath, int stride)
{
  FrameCacheResult result;
  if (stride < 1) stride = 1;

  @autoreleasepool {
    NSString* nsPath = [NSString stringWithUTF8String:videoPath.c_str()];
    if (!nsPath) {
      result.errorMessage = "invalid path";
      return result;
    }
    NSURL* url = [NSURL fileURLWithPath:nsPath];
    AVURLAsset* asset = [AVURLAsset URLAssetWithURL:url options:nil];

    NSArray<AVAssetTrack*>* videoTracks = [asset tracksWithMediaType:AVMediaTypeVideo];
    if (videoTracks.count == 0) {
      result.errorMessage = "no video track";
      return result;
    }
    AVAssetTrack* track = videoTracks.firstObject;

    int width  = (int)track.naturalSize.width;
    int height = (int)track.naturalSize.height;
    if (width <= 0 || height <= 0) {
      result.errorMessage = "invalid dimensions";
      return result;
    }
    result.sourceWidth = width;
    result.sourceHeight = height;

    NSError* error = nil;
    AVAssetReader* reader = [AVAssetReader assetReaderWithAsset:asset error:&error];
    if (!reader) {
      result.errorMessage = std::string("assetReader: ") +
        (error ? [error.localizedDescription UTF8String] : "unknown");
      return result;
    }

    NSDictionary* outputSettings = @{
      (NSString*)kCVPixelBufferPixelFormatTypeKey : @(kCVPixelFormatType_32BGRA)
    };
    AVAssetReaderTrackOutput* output =
      [AVAssetReaderTrackOutput assetReaderTrackOutputWithTrack:track
                                                 outputSettings:outputSettings];
    output.alwaysCopiesSampleData = NO;
    if (![reader canAddOutput:output]) {
      result.errorMessage = "cannot add output";
      return result;
    }
    [reader addOutput:output];

    if (![reader startReading]) {
      result.errorMessage = std::string("startReading: ") +
        (reader.error ? [reader.error.localizedDescription UTF8String] : "unknown");
      return result;
    }

    int frameIdx = 0;
    while (reader.status == AVAssetReaderStatusReading) {
      CMSampleBufferRef sb = [output copyNextSampleBuffer];
      if (!sb) break;

      const bool keep = (frameIdx % stride == 0);
      if (keep) {
        CVImageBufferRef pix = CMSampleBufferGetImageBuffer(sb);
        if (pix) {
          CVPixelBufferLockBaseAddress(pix, kCVPixelBufferLock_ReadOnly);

          const int w = (int)CVPixelBufferGetWidth(pix);
          const int h = (int)CVPixelBufferGetHeight(pix);
          const size_t srcStride = CVPixelBufferGetBytesPerRow(pix);
          const unsigned char* srcBase = (const unsigned char*)CVPixelBufferGetBaseAddress(pix);

          ofPixels px;
          px.allocate(w, h, OF_PIXELS_BGRA);
          unsigned char* dst = px.getData();
          const size_t dstStride = (size_t)w * 4;
          if (srcStride == dstStride) {
            std::memcpy(dst, srcBase, dstStride * (size_t)h);
          } else {
            for (int row = 0; row < h; ++row) {
              std::memcpy(dst + (size_t)row * dstStride,
                          srcBase + (size_t)row * srcStride,
                          dstStride);
            }
          }

          CVPixelBufferUnlockBaseAddress(pix, kCVPixelBufferLock_ReadOnly);

          auto tex = std::make_shared<ofTexture>();
          tex->allocate(w, h, GL_RGBA);
          tex->loadData(px);
          result.frames.push_back(tex);
        }
      }

      CFRelease(sb);
      frameIdx++;
    }

    result.sourceFramesSeen = frameIdx;

    if (reader.status == AVAssetReaderStatusFailed) {
      result.errorMessage = std::string("reader failed: ") +
        (reader.error ? [reader.error.localizedDescription UTF8String] : "unknown");
      [reader cancelReading];
      return result;
    }
    [reader cancelReading];

    result.success = !result.frames.empty();
  }

  return result;
}

} // namespace nottawa

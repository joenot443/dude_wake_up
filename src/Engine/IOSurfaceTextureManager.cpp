//
//  IOSurfaceTextureManager.cpp
//  dude_wake_up
//
//  IOSurface-backed texture sharing: blits OpenGL FBOs to IOSurfaces
//  that can be read by Metal on the Swift side.
//

#include "IOSurfaceTextureManager.hpp"
#include "ShaderChainerService.hpp"
#include "VideoSourceService.hpp"
#include "Connection.hpp"
#include "ofMain.h"

#include <OpenGL/CGLCurrent.h>
#include <OpenGL/CGLIOSurface.h>
#include <CoreFoundation/CoreFoundation.h>

IOSurfaceTextureManager* IOSurfaceTextureManager::instance = nullptr;

IOSurfaceTextureManager* IOSurfaceTextureManager::getInstance() {
  if (!instance) {
    instance = new IOSurfaceTextureManager();
  }
  return instance;
}

IOSurfaceTextureManager::~IOSurfaceTextureManager() {
  std::lock_guard<std::mutex> lock(mutex);
  for (auto& [id, tex] : tracked) {
    cleanupSharedTexture(tex);
  }
  tracked.clear();
}

// ---------------------------------------------------------------------------
// MARK: - Public API (thread-safe)
// ---------------------------------------------------------------------------

void IOSurfaceTextureManager::startSharing(const std::string& connectableId) {
  std::lock_guard<std::mutex> lock(mutex);
  if (tracked.find(connectableId) != tracked.end()) return;

  SharedTexture tex;
  tex.needsSetup = true;
  tracked[connectableId] = tex;
}

void IOSurfaceTextureManager::stopSharing(const std::string& connectableId) {
  std::lock_guard<std::mutex> lock(mutex);
  auto it = tracked.find(connectableId);
  if (it == tracked.end()) return;

  cleanupSharedTexture(it->second);
  tracked.erase(it);
}

IOSurfaceRef IOSurfaceTextureManager::getIOSurface(const std::string& connectableId) {
  std::lock_guard<std::mutex> lock(mutex);
  auto it = tracked.find(connectableId);
  if (it == tracked.end()) return nullptr;
  return it->second.surface;
}

int IOSurfaceTextureManager::getWidth(const std::string& connectableId) {
  std::lock_guard<std::mutex> lock(mutex);
  auto it = tracked.find(connectableId);
  if (it == tracked.end()) return 0;
  return it->second.width;
}

int IOSurfaceTextureManager::getHeight(const std::string& connectableId) {
  std::lock_guard<std::mutex> lock(mutex);
  auto it = tracked.find(connectableId);
  if (it == tracked.end()) return 0;
  return it->second.height;
}

bool IOSurfaceTextureManager::isSharing(const std::string& connectableId) {
  std::lock_guard<std::mutex> lock(mutex);
  return tracked.find(connectableId) != tracked.end();
}

// ---------------------------------------------------------------------------
// MARK: - Per-frame blit (engine thread only)
// ---------------------------------------------------------------------------

void IOSurfaceTextureManager::setSecondaryLookup(ConnectableLookup lookup) {
  secondaryLookup = lookup;
}

// Defined in nottawa_bridge.cpp — direct lookup, no lambda indirection.
extern std::shared_ptr<Connectable> ntw_find_preview_connectable(const std::string& id);

static std::shared_ptr<Connectable> findConnectableForBlit(const std::string& id) {
  // Check preview instances first (avoids noisy error logs from shaderForId)
  auto preview = ntw_find_preview_connectable(id);
  if (preview) return preview;

  // Check video sources before shaders — videoSourceForId doesn't log on miss,
  // but shaderForId does, so this avoids noisy error spam for source IDs.
  auto source = VideoSourceService::getService()->videoSourceForId(id);
  if (source) return std::dynamic_pointer_cast<Connectable>(source);

  auto shader = ShaderChainerService::getService()->shaderForId(id);
  if (shader) return std::dynamic_pointer_cast<Connectable>(shader);

  return nullptr;
}

void IOSurfaceTextureManager::blitTrackedTextures() {
  std::lock_guard<std::mutex> lock(mutex);
  if (tracked.empty()) return;

  for (auto& [id, tex] : tracked) {
    auto connectable = findConnectableForBlit(id);
    if (!connectable) continue;

    auto fbo = connectable->frame();
    if (!fbo) continue;

    int srcW = fbo->getWidth();
    int srcH = fbo->getHeight();
    if (srcW <= 0 || srcH <= 0) continue;

    // Set up or resize if needed
    if (tex.needsSetup || tex.width != srcW || tex.height != srcH) {
      cleanupSharedTexture(tex);
      setupSharedTexture(tex, srcW, srcH);
      if (!tex.surface) continue;
    }

    // Blit from source FBO to IOSurface FBO
    GLuint srcFboId = fbo->getId();
    glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFboId);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tex.glFbo);
    glBlitFramebuffer(
      0, 0, srcW, srcH,
      0, 0, tex.width, tex.height,
      GL_COLOR_BUFFER_BIT, GL_NEAREST
    );
    glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
  }

  // Flush to ensure IOSurface contents are visible to Metal
  glFlush();
}

void IOSurfaceTextureManager::blitExternalFrame(const std::string& id,
                                                  unsigned int srcFboId,
                                                  int srcW, int srcH) {
  std::lock_guard<std::mutex> lock(mutex);
  auto it = tracked.find(id);
  if (it == tracked.end()) return;
  auto& tex = it->second;

  if (srcW <= 0 || srcH <= 0) return;

  // Set up IOSurface if needed
  if (tex.needsSetup || tex.width != srcW || tex.height != srcH) {
    cleanupSharedTexture(tex);
    setupSharedTexture(tex, srcW, srcH);
    if (!tex.surface) return;
  }

  // Blit
  glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFboId);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tex.glFbo);
  glBlitFramebuffer(
    0, 0, srcW, srcH,
    0, 0, tex.width, tex.height,
    GL_COLOR_BUFFER_BIT, GL_NEAREST
  );
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
}

// ---------------------------------------------------------------------------
// MARK: - GL resource setup/teardown (engine thread only)
// ---------------------------------------------------------------------------

void IOSurfaceTextureManager::setupSharedTexture(SharedTexture& tex, int width, int height) {
  // Create IOSurface with BGRA format
  const void* keys[] = {
    kIOSurfaceWidth,
    kIOSurfaceHeight,
    kIOSurfaceBytesPerElement,
    kIOSurfacePixelFormat
  };
  int w = width;
  int h = height;
  int bpe = 4;
  int pixelFormat = 'BGRA';

  CFNumberRef cfWidth = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &w);
  CFNumberRef cfHeight = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &h);
  CFNumberRef cfBpe = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &bpe);
  CFNumberRef cfPixelFormat = CFNumberCreate(kCFAllocatorDefault, kCFNumberIntType, &pixelFormat);

  const void* values[] = { cfWidth, cfHeight, cfBpe, cfPixelFormat };

  CFDictionaryRef props = CFDictionaryCreate(
    kCFAllocatorDefault, keys, values, 4,
    &kCFTypeDictionaryKeyCallBacks, &kCFTypeDictionaryValueCallBacks
  );

  tex.surface = IOSurfaceCreate(props);

  CFRelease(props);
  CFRelease(cfWidth);
  CFRelease(cfHeight);
  CFRelease(cfBpe);
  CFRelease(cfPixelFormat);

  if (!tex.surface) return;

  // Create GL texture backed by the IOSurface
  glGenTextures(1, &tex.glTexture);
  glBindTexture(GL_TEXTURE_RECTANGLE, tex.glTexture);

  CGLContextObj cglCtx = CGLGetCurrentContext();
  CGLError err = CGLTexImageIOSurface2D(
    cglCtx,
    GL_TEXTURE_RECTANGLE,
    GL_RGBA8,
    width, height,
    GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV,
    tex.surface, 0
  );

  if (err != kCGLNoError) {
    glDeleteTextures(1, &tex.glTexture);
    tex.glTexture = 0;
    CFRelease(tex.surface);
    tex.surface = nullptr;
    return;
  }

  glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_RECTANGLE, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glBindTexture(GL_TEXTURE_RECTANGLE, 0);

  // Create FBO with the IOSurface-backed texture as color attachment
  glGenFramebuffers(1, &tex.glFbo);
  glBindFramebuffer(GL_FRAMEBUFFER, tex.glFbo);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0,
                         GL_TEXTURE_RECTANGLE, tex.glTexture, 0);

  GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

  if (status != GL_FRAMEBUFFER_COMPLETE) {
    glDeleteFramebuffers(1, &tex.glFbo);
    tex.glFbo = 0;
    glDeleteTextures(1, &tex.glTexture);
    tex.glTexture = 0;
    CFRelease(tex.surface);
    tex.surface = nullptr;
    return;
  }

  tex.width = width;
  tex.height = height;
  tex.needsSetup = false;
}

void IOSurfaceTextureManager::cleanupSharedTexture(SharedTexture& tex) {
  if (tex.glFbo) {
    glDeleteFramebuffers(1, &tex.glFbo);
    tex.glFbo = 0;
  }
  if (tex.glTexture) {
    glDeleteTextures(1, &tex.glTexture);
    tex.glTexture = 0;
  }
  if (tex.surface) {
    CFRelease(tex.surface);
    tex.surface = nullptr;
  }
  tex.width = 0;
  tex.height = 0;
  tex.needsSetup = true;
}

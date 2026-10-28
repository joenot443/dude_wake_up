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

#ifdef __APPLE__
#include <OpenGL/CGLCurrent.h>
#include <OpenGL/CGLIOSurface.h>
#include <CoreFoundation/CoreFoundation.h>
#endif

IOSurfaceTextureManager* IOSurfaceTextureManager::instance = nullptr;

IOSurfaceTextureManager* IOSurfaceTextureManager::getInstance() {
  if (!instance) {
    instance = new IOSurfaceTextureManager();
  }
  return instance;
}

IOSurfaceTextureManager::~IOSurfaceTextureManager() {
  std::lock_guard<std::mutex> lock(mutex);
  for (auto& [id, entry] : entries) cleanupSharedTexture(entry->tex);
  for (auto& entry : pendingCleanup) cleanupSharedTexture(entry->tex);
  for (IOSurfaceRef surface : retiredSurfaces) CFRelease(surface);
}

// ---------------------------------------------------------------------------
// MARK: - Public API (thread-safe; never touches GL)
// ---------------------------------------------------------------------------

void IOSurfaceTextureManager::startSharing(const std::string& connectableId) {
  std::lock_guard<std::mutex> lock(mutex);
  if (entries.count(connectableId)) return;
  entries[connectableId] = std::make_shared<Entry>();
}

void IOSurfaceTextureManager::stopSharing(const std::string& connectableId) {
  std::lock_guard<std::mutex> lock(mutex);
  auto it = entries.find(connectableId);
  if (it == entries.end()) return;
  // GL cleanup happens on the engine thread, at the next blit.
  pendingCleanup.push_back(std::move(it->second));
  entries.erase(it);
}

IOSurfaceRef IOSurfaceTextureManager::getIOSurface(const std::string& connectableId) {
  std::lock_guard<std::mutex> lock(mutex);
  auto it = entries.find(connectableId);
  if (it == entries.end() || !it->second->ready) return nullptr;
  return it->second->tex.surface;
}

int IOSurfaceTextureManager::getWidth(const std::string& connectableId) {
  std::lock_guard<std::mutex> lock(mutex);
  auto it = entries.find(connectableId);
  return it == entries.end() ? 0 : it->second->width;
}

int IOSurfaceTextureManager::getHeight(const std::string& connectableId) {
  std::lock_guard<std::mutex> lock(mutex);
  auto it = entries.find(connectableId);
  return it == entries.end() ? 0 : it->second->height;
}

bool IOSurfaceTextureManager::isSharing(const std::string& connectableId) {
  std::lock_guard<std::mutex> lock(mutex);
  return entries.count(connectableId) > 0;
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
  // Synthetic ids (e.g. "transformCrop:<shaderId>") are owned by their
  // producing shader and blitted via blitExternalFrame — they aren't
  // connectables in any service, so short-circuit before shaderForId logs
  // a spurious "shader doesn't exist" warning every frame.
  if (id.find(':') != std::string::npos) return nullptr;

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

bool IOSurfaceTextureManager::prepare(Entry& entry, int width, int height) {
  SharedTexture& tex = entry.tex;
  if (!tex.needsSetup && tex.width == width && tex.height == height) return true;
  {
    std::lock_guard<std::mutex> lock(mutex);
    entry.ready = false;  // (its surface is about to change)
  }
  cleanupSharedTexture(tex);
  setupSharedTexture(tex, width, height);
  if (tex.needsSetup) return false;
  std::lock_guard<std::mutex> lock(mutex);
  entry.ready = true;
  entry.width = width;
  entry.height = height;
  return true;
}

static void blitInto(const SharedTexture& tex, GLuint srcFboId, int srcW, int srcH) {
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

void IOSurfaceTextureManager::blitTrackedTextures() {
  // Surfaces retired last blit: the UI has had a frame to retain them.
  for (IOSurfaceRef surface : retiredSurfaces) CFRelease(surface);
  retiredSurfaces.clear();

  // The entries to work on, taken under the lock; the GL work without it.
  std::vector<std::shared_ptr<Entry>> cleanup;
  std::vector<std::pair<std::string, std::shared_ptr<Entry>>> work;
  {
    std::lock_guard<std::mutex> lock(mutex);
    cleanup.swap(pendingCleanup);
    work.reserve(entries.size());
    for (auto& [id, entry] : entries) work.emplace_back(id, entry);
  }
  for (auto& entry : cleanup) cleanupSharedTexture(entry->tex);
  if (work.empty()) return;

  for (auto& [id, entry] : work) {
    auto connectable = findConnectableForBlit(id);
    if (!connectable) continue;

    auto fbo = connectable->frame();
    if (!fbo) continue;

    int srcW = fbo->getWidth();
    int srcH = fbo->getHeight();
    if (srcW <= 0 || srcH <= 0) continue;

    // Always blit right after a (re)allocation so the surface has valid
    // contents on first read; otherwise skip when the source FBO hasn't
    // been written since the last blit.
    SharedTexture& tex = entry->tex;
    bool didReallocate = tex.needsSetup || tex.width != srcW || tex.height != srcH;
    if (!prepare(*entry, srcW, srcH)) continue;
    if (!didReallocate && tex.lastBlittedTick == connectable->renderTick) continue;

    blitInto(tex, fbo->getId(), srcW, srcH);
    tex.lastBlittedTick = connectable->renderTick;
  }

  // Flush to ensure IOSurface contents are visible to Metal
  glFlush();
}

void IOSurfaceTextureManager::blitExternalFrame(const std::string& id,
                                                  unsigned int srcFboId,
                                                  int srcW, int srcH) {
  if (srcW <= 0 || srcH <= 0) return;
  std::shared_ptr<Entry> entry;
  {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = entries.find(id);
    if (it == entries.end()) return;
    entry = it->second;
  }
  if (!prepare(*entry, srcW, srcH)) return;
  blitInto(entry->tex, srcFboId, srcW, srcH);
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

  // Clear the IOSurface FBO to black so it doesn't show uninitialized pink
  glBindFramebuffer(GL_FRAMEBUFFER, tex.glFbo);
  glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
  glClear(GL_COLOR_BUFFER_BIT);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);

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
    retiredSurfaces.push_back(tex.surface);  // released next blit
    tex.surface = nullptr;
  }
  tex.width = 0;
  tex.height = 0;
  tex.needsSetup = true;
}

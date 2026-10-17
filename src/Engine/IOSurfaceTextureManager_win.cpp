// Windows implementation of IOSurfaceTextureManager. Instead of an IOSurface,
// each shared connectable gets a FrameExchange: three GL textures in the share
// group joined to the host UI (EngineLoop::setShareContext), handed over with
// fences. getIOSurface() returns the FrameExchange*.

#ifndef TARGET_OSX

#include "IOSurfaceTextureManager.hpp"
#include "FrameExchange.h"
#include "ShaderChainerService.hpp"
#include "VideoSourceService.hpp"
#include "Connection.hpp"
#include "ofMain.h"

IOSurfaceTextureManager* IOSurfaceTextureManager::instance = nullptr;

IOSurfaceTextureManager* IOSurfaceTextureManager::getInstance() {
  if (!instance) instance = new IOSurfaceTextureManager();
  return instance;
}

// Process-lifetime singleton; GL objects die with the context.
IOSurfaceTextureManager::~IOSurfaceTextureManager() {}

void IOSurfaceTextureManager::startSharing(const std::string& connectableId) {
  std::lock_guard<std::mutex> lock(mutex);
  if (tracked.count(connectableId)) return;
  SharedTexture tex;
  tex.exchange = std::make_shared<FrameExchange>();
  tex.surface = tex.exchange.get();
  tracked[connectableId] = std::move(tex);
}

void IOSurfaceTextureManager::stopSharing(const std::string& connectableId) {
  std::lock_guard<std::mutex> lock(mutex);
  auto it = tracked.find(connectableId);
  if (it == tracked.end()) return;
  retiredExchanges.push_back(it->second.exchange);
  pendingCleanup.push_back(std::move(it->second));
  tracked.erase(it);
}

IOSurfaceRef IOSurfaceTextureManager::getIOSurface(const std::string& connectableId) {
  std::lock_guard<std::mutex> lock(mutex);
  auto it = tracked.find(connectableId);
  if (it == tracked.end() || it->second.needsSetup) return nullptr;
  return it->second.surface;
}

int IOSurfaceTextureManager::getWidth(const std::string& connectableId) {
  std::lock_guard<std::mutex> lock(mutex);
  auto it = tracked.find(connectableId);
  return it == tracked.end() ? 0 : it->second.width;
}

int IOSurfaceTextureManager::getHeight(const std::string& connectableId) {
  std::lock_guard<std::mutex> lock(mutex);
  auto it = tracked.find(connectableId);
  return it == tracked.end() ? 0 : it->second.height;
}

bool IOSurfaceTextureManager::isSharing(const std::string& connectableId) {
  std::lock_guard<std::mutex> lock(mutex);
  return tracked.count(connectableId) > 0;
}

void IOSurfaceTextureManager::setSecondaryLookup(ConnectableLookup lookup) {
  secondaryLookup = std::move(lookup);
}

// Defined in nottawa_bridge.cpp.
extern std::shared_ptr<Connectable> ntw_find_preview_connectable(const std::string& id);

static std::shared_ptr<Connectable> findConnectableForBlit(const std::string& id) {
  // Synthetic ids ("transformCrop:<shaderId>") are fed via blitExternalFrame.
  if (id.find(':') != std::string::npos) return nullptr;
  if (auto preview = ntw_find_preview_connectable(id)) return preview;
  if (auto source = VideoSourceService::getService()->videoSourceForId(id))
    return std::dynamic_pointer_cast<Connectable>(source);
  if (auto shader = ShaderChainerService::getService()->shaderForId(id))
    return std::dynamic_pointer_cast<Connectable>(shader);
  return nullptr;
}

void IOSurfaceTextureManager::publish(SharedTexture& tex, unsigned int srcFboId, int srcW, int srcH) {
  std::vector<void*> readFences;
  void* oldWriteFence = nullptr;
  const int slot = tex.exchange->acquireWriteSlot(readFences, &oldWriteFence);
  if (slot < 0) return;
  for (void* fence : readFences) {
    glWaitSync(static_cast<GLsync>(fence), 0, GL_TIMEOUT_IGNORED);
    glDeleteSync(static_cast<GLsync>(fence));
  }
  if (oldWriteFence) glDeleteSync(static_cast<GLsync>(oldWriteFence));
  glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFboId);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tex.slotFbos[slot]);
  glBlitFramebuffer(0, 0, srcW, srcH, 0, 0, tex.width, tex.height,
                    GL_COLOR_BUFFER_BIT, GL_NEAREST);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  tex.exchange->publish(slot, glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
}

void IOSurfaceTextureManager::blitTrackedTextures() {
  std::lock_guard<std::mutex> lock(mutex);
  for (auto& tex : pendingCleanup) cleanupSharedTexture(tex);
  pendingCleanup.clear();
  if (tracked.empty()) return;

  for (auto& [id, tex] : tracked) {
    auto connectable = findConnectableForBlit(id);
    if (!connectable) continue;
    auto fbo = connectable->frame();
    if (!fbo) continue;
    const int srcW = fbo->getWidth();
    const int srcH = fbo->getHeight();
    if (srcW <= 0 || srcH <= 0) continue;

    bool reallocated = false;
    if (tex.needsSetup || tex.width != srcW || tex.height != srcH) {
      cleanupSharedTexture(tex);
      setupSharedTexture(tex, srcW, srcH);
      if (tex.needsSetup) continue;
      reallocated = true;
    }
    if (!reallocated && tex.lastBlittedTick == connectable->renderTick) continue;

    publish(tex, fbo->getId(), srcW, srcH);
    tex.lastBlittedTick = connectable->renderTick;
  }
  // Make the fences visible to the UI's context.
  glFlush();
}

void IOSurfaceTextureManager::blitExternalFrame(const std::string& id, unsigned int srcFboId,
                                                int srcW, int srcH) {
  std::lock_guard<std::mutex> lock(mutex);
  auto it = tracked.find(id);
  if (it == tracked.end() || srcW <= 0 || srcH <= 0) return;
  auto& tex = it->second;
  if (tex.needsSetup || tex.width != srcW || tex.height != srcH) {
    cleanupSharedTexture(tex);
    setupSharedTexture(tex, srcW, srcH);
    if (tex.needsSetup) return;
  }
  publish(tex, srcFboId, srcW, srcH);
  glFlush();
}

static_assert(sizeof(SharedTexture::slotTextures) / sizeof(unsigned int) == FrameExchange::kSlots,
              "SharedTexture slot arrays must match FrameExchange::kSlots");
constexpr int kSlots = FrameExchange::kSlots;

void IOSurfaceTextureManager::setupSharedTexture(SharedTexture& tex, int width, int height) {
  glGenTextures(kSlots, tex.slotTextures);
  glGenFramebuffers(kSlots, tex.slotFbos);
  bool ok = true;
  for (int i = 0; i < kSlots; ++i) {
    glBindTexture(GL_TEXTURE_2D, tex.slotTextures[i]);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glBindFramebuffer(GL_FRAMEBUFFER, tex.slotFbos[i]);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex.slotTextures[i], 0);
    ok = ok && glCheckFramebufferStatus(GL_FRAMEBUFFER) == GL_FRAMEBUFFER_COMPLETE;
    glClearColor(0.15f, 0.15f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
  }
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  if (!ok) {
    ofLogError("IOSurfaceTextureManager") << "shared texture FBO incomplete (" << width << "x" << height << ")";
    cleanupSharedTexture(tex);
    return;
  }
  uint32_t ids[kSlots];
  for (int i = 0; i < kSlots; ++i) ids[i] = tex.slotTextures[i];
  tex.exchange->setTextures(ids);
  tex.width = width;
  tex.height = height;
  tex.needsSetup = false;
}

void IOSurfaceTextureManager::cleanupSharedTexture(SharedTexture& tex) {
  if (tex.exchange)
    for (void* fence : tex.exchange->takeAllFences()) glDeleteSync(static_cast<GLsync>(fence));
  if (tex.slotFbos[0]) glDeleteFramebuffers(kSlots, tex.slotFbos);
  if (tex.slotTextures[0]) glDeleteTextures(kSlots, tex.slotTextures);
  for (int i = 0; i < kSlots; ++i) tex.slotFbos[i] = tex.slotTextures[i] = 0;
  tex.width = tex.height = 0;
  tex.needsSetup = true;
  tex.lastBlittedTick = UINT64_MAX;
}

#endif // !TARGET_OSX

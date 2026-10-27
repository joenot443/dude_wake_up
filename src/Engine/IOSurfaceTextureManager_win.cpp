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
  if (entries.count(connectableId)) return;
  auto entry = std::make_shared<Entry>();
  entry->tex.exchange = std::make_shared<FrameExchange>();
  entry->tex.surface = entry->tex.exchange.get();
  entries[connectableId] = std::move(entry);
}

void IOSurfaceTextureManager::stopSharing(const std::string& connectableId) {
  std::lock_guard<std::mutex> lock(mutex);
  auto it = entries.find(connectableId);
  if (it == entries.end()) return;
  retiredExchanges.push_back(it->second->tex.exchange);
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
  if (!ensureSlot(tex, slot)) return;
  glBindFramebuffer(GL_READ_FRAMEBUFFER, srcFboId);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, tex.slotFbos[slot]);
  glBlitFramebuffer(0, 0, srcW, srcH, 0, 0, tex.width, tex.height,
                    GL_COLOR_BUFFER_BIT, GL_NEAREST);
  glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);
  glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);

  tex.exchange->publish(slot, glFenceSync(GL_SYNC_GPU_COMMANDS_COMPLETE, 0));
}

bool IOSurfaceTextureManager::prepare(Entry& entry, int width, int height) {
  SharedTexture& tex = entry.tex;
  if (!tex.needsSetup && tex.width == width && tex.height == height) return true;
  {
    std::lock_guard<std::mutex> lock(mutex);
    entry.ready = false;  // (its textures are about to change)
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

void IOSurfaceTextureManager::blitTrackedTextures() {
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
    const int srcW = fbo->getWidth();
    const int srcH = fbo->getHeight();
    if (srcW <= 0 || srcH <= 0) continue;

    SharedTexture& tex = entry->tex;
    const bool reallocated = tex.needsSetup || tex.width != srcW || tex.height != srcH;
    if (!prepare(*entry, srcW, srcH)) continue;
    if (!reallocated && tex.lastBlittedTick == connectable->renderTick) continue;

    publish(tex, fbo->getId(), srcW, srcH);
    tex.lastBlittedTick = connectable->renderTick;
  }
  // Make the fences visible to the UI's context.
  glFlush();
}

void IOSurfaceTextureManager::blitExternalFrame(const std::string& id, unsigned int srcFboId,
                                                int srcW, int srcH) {
  std::shared_ptr<Entry> entry;
  {
    std::lock_guard<std::mutex> lock(mutex);
    auto it = entries.find(id);
    if (it == entries.end() || srcW <= 0 || srcH <= 0) return;
    entry = it->second;
  }
  if (!prepare(*entry, srcW, srcH)) return;
  publish(entry->tex, srcFboId, srcW, srcH);
  glFlush();
}

static_assert(sizeof(SharedTexture::slotTextures) / sizeof(unsigned int) == FrameExchange::kSlots,
              "SharedTexture slot arrays must match FrameExchange::kSlots");
constexpr int kSlots = FrameExchange::kSlots;

void IOSurfaceTextureManager::setupSharedTexture(SharedTexture& tex, int width, int height) {
  glGenTextures(kSlots, tex.slotTextures);
  glGenFramebuffers(kSlots, tex.slotFbos);
  for (bool& ready : tex.slotReady) ready = false;
  uint32_t ids[kSlots];
  for (int i = 0; i < kSlots; ++i) ids[i] = tex.slotTextures[i];
  tex.exchange->setTextures(ids);
  tex.width = width;
  tex.height = height;
  tex.needsSetup = false;
}

// Gives a slot storage and its blit FBO, the first time it's written.
bool IOSurfaceTextureManager::ensureSlot(SharedTexture& tex, int slot) {
  if (tex.slotReady[slot]) return true;
  glBindTexture(GL_TEXTURE_2D, tex.slotTextures[slot]);
  glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA8, tex.width, tex.height, 0, GL_RGBA, GL_UNSIGNED_BYTE, nullptr);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
  glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
  glBindTexture(GL_TEXTURE_2D, 0);
  glBindFramebuffer(GL_FRAMEBUFFER, tex.slotFbos[slot]);
  glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, tex.slotTextures[slot], 0);
  // No glCheckFramebufferStatus: one RGBA8 texture of non-zero size is
  // always complete, and the check makes the NVIDIA driver commit the new
  // storage on the spot. With the GPU busy that waited seconds while holding
  // the lock the UI's context needs (its wglMakeCurrent): the UI froze.
  glBindFramebuffer(GL_FRAMEBUFFER, 0);
  tex.slotReady[slot] = true;
  return true;
}

void IOSurfaceTextureManager::cleanupSharedTexture(SharedTexture& tex) {
  if (tex.exchange)
    for (void* fence : tex.exchange->takeAllFences()) glDeleteSync(static_cast<GLsync>(fence));
  if (tex.slotFbos[0]) glDeleteFramebuffers(kSlots, tex.slotFbos);
  if (tex.slotTextures[0]) glDeleteTextures(kSlots, tex.slotTextures);
  for (int i = 0; i < kSlots; ++i) { tex.slotFbos[i] = tex.slotTextures[i] = 0; tex.slotReady[i] = false; }
  tex.width = tex.height = 0;
  tex.needsSetup = true;
  tex.lastBlittedTick = UINT64_MAX;
}

#endif // !TARGET_OSX

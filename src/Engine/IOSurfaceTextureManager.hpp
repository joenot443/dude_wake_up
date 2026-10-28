//
//  IOSurfaceTextureManager.hpp
//  dude_wake_up
//
//  Manages IOSurface-backed FBOs for sharing OpenGL textures with Metal.
//  Each tracked connectable gets an IOSurface that is blitted to each frame.
//  The IOSurface can then be wrapped in an MTLTexture on the Swift side.
//

#ifndef IOSurfaceTextureManager_hpp
#define IOSurfaceTextureManager_hpp

#include <string>
#include <unordered_map>
#include <mutex>
#include <functional>
#include <memory>
#include <vector>
#ifdef __APPLE__
#include <IOSurface/IOSurface.h>
#else
// Windows: the "surface" handed to the UI is a FrameExchange* (a fenced
// ring of GL textures in the UI's share group, readable by several views).
typedef void* IOSurfaceRef;
class FrameExchange;
#endif

struct SharedTexture {
  IOSurfaceRef surface = nullptr;
  unsigned int glTexture = 0;  // GLuint
  unsigned int glFbo = 0;      // GLuint
  int width = 0;
  int height = 0;
  bool needsSetup = true;  // Defers GL resource creation to engine thread
  // Last-blitted source renderTick. blitTrackedTextures skips when the
  // source FBO hasn't been written since the previous blit. UINT64_MAX
  // until the first successful blit, so the initial frame always blits.
  uint64_t lastBlittedTick = UINT64_MAX;
#ifndef __APPLE__
  std::shared_ptr<FrameExchange> exchange;  // `surface` points at this
  unsigned int slotTextures[7] = {};  // FrameExchange::kSlots
  unsigned int slotFbos[7] = {};
  // Storage is allocated on a slot's first write: the engine reuses the
  // lowest free slot, so with few readers most slots never need any.
  bool slotReady[7] = {};
#endif
};

class IOSurfaceTextureManager {
public:
  static IOSurfaceTextureManager* getInstance();

  // Start sharing textures for a connectable (thread-safe, defers GL setup).
  void startSharing(const std::string& connectableId);

  // Stop sharing and release resources (thread-safe).
  void stopSharing(const std::string& connectableId);

  // Blit all tracked connectables' frames to their IOSurfaces.
  // Must be called on the engine (OpenGL) thread, after processFrame().
  void blitTrackedTextures();

  // Blit a specific FBO directly for a tracked ID (bypasses connectable lookup).
  // Use for preview instances where we already have the rendered frame.
  // Must be called on the engine (OpenGL) thread.
  void blitExternalFrame(const std::string& id, unsigned int srcFboId, int srcW, int srcH);

  // Get the IOSurface for a connectable (thread-safe).
  // Returns nullptr if not tracked or not yet set up.
  IOSurfaceRef getIOSurface(const std::string& connectableId);

  // Get texture dimensions for a connectable.
  int getWidth(const std::string& connectableId);
  int getHeight(const std::string& connectableId);

  // Check if a connectable is being tracked.
  bool isSharing(const std::string& connectableId);

  // Register a secondary connectable lookup for preview instances.
  // The callback is called when findConnectable fails to find an ID in the
  // main services. Returns a shared_ptr to a Connectable, or nullptr.
  using ConnectableLookup = std::function<std::shared_ptr<class Connectable>(const std::string&)>;
  void setSecondaryLookup(ConnectableLookup lookup);

private:
  IOSurfaceTextureManager() = default;
  ~IOSurfaceTextureManager();

  // Set up GL resources for a SharedTexture (must be on GL thread).
  void setupSharedTexture(SharedTexture& tex, int width, int height);

  // Tear down GL resources for a SharedTexture.
  void cleanupSharedTexture(SharedTexture& tex);

  std::mutex mutex;
  ConnectableLookup secondaryLookup;
  // `mutex` guards only the map and what the UI reads (ready, width,
  // height); the engine's GL work on an entry happens without it. Holding
  // it across GL calls froze the UI whenever the driver made the engine
  // wait for a busy GPU (the UI takes it to find a node's frames).
  struct Entry {
    SharedTexture tex;        // GL state: engine thread only
    bool ready = false;       // under mutex: what the UI sees
    int width = 0, height = 0;
  };
  std::unordered_map<std::string, std::shared_ptr<Entry>> entries;
  // stopSharing may run off the GL thread, so GL cleanup is deferred to the
  // next blitTrackedTextures().
  std::vector<std::shared_ptr<Entry>> pendingCleanup;
  // Sizes an entry's textures for the source (engine thread, without mutex),
  // then publishes ready/width/height under it.
  bool prepare(Entry& entry, int width, int height);
#ifdef __APPLE__
  // Surfaces replaced or stopped, released one blit later: getIOSurface
  // hands the UI an unretained pointer, which it retains just after.
  std::vector<IOSurfaceRef> retiredSurfaces;
#else
  // Exchanges outlive their entries because the UI may still hold the pointer.
  std::vector<std::shared_ptr<FrameExchange>> retiredExchanges;
  void publish(SharedTexture& tex, unsigned int srcFboId, int srcW, int srcH);
  bool ensureSlot(SharedTexture& tex, int slot);
#endif

  static IOSurfaceTextureManager* instance;
};

#endif /* IOSurfaceTextureManager_hpp */

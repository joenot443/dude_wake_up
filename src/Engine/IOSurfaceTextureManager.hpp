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
#include <IOSurface/IOSurface.h>

struct SharedTexture {
  IOSurfaceRef surface = nullptr;
  unsigned int glTexture = 0;  // GLuint
  unsigned int glFbo = 0;      // GLuint
  int width = 0;
  int height = 0;
  bool needsSetup = true;  // Defers GL resource creation to engine thread
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

  std::unordered_map<std::string, SharedTexture> tracked;
  std::mutex mutex;
  ConnectableLookup secondaryLookup;

  static IOSurfaceTextureManager* instance;
};

#endif /* IOSurfaceTextureManager_hpp */

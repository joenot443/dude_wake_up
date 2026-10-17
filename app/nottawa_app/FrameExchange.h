#pragma once

#include <cstdint>
#include <mutex>
#include <vector>

// Triple-buffered handoff of finished engine frames to the UI's GL context.
//
// Each slot is a texture in the shared GL namespace plus two fences:
//   writeFence — inserted by the engine after rendering into the slot; the UI
//                waits on it (glWaitSync) before sampling.
//   readFence  — inserted by the UI after sampling; the engine waits on it
//                before rendering into that slot again.
// The engine never writes the slot the UI is showing or the newest
// unconsumed frame, so with three slots one is always free.
//
// Fences are stored as void* (GLsync) so this header stays free of GL
// headers: the engine side uses GLEW, the UI side Qt's GL headers.
class FrameExchange {
public:
    static constexpr int kSlots = 3;

    void setTextures(const uint32_t (&ids)[kSlots]);
    uint32_t texture(int slot) const;

    // Engine side. acquireWriteSlot hands back the UI's read fence for the
    // slot (caller waits on and deletes it). publish returns the fence of a
    // frame the UI never took, which the caller must delete.
    int   acquireWriteSlot(void** readFenceOut);
    void* publish(int slot, void* writeFence);

    // UI side. acquireReadSlot returns the slot to display (-1 before the
    // first frame) and, when a new frame arrived, its write fence. After
    // drawing, releaseReadSlot stores the read fence and returns the old one
    // for deletion.
    int   acquireReadSlot(void** writeFenceOut);
    void* releaseReadSlot(int slot, void* readFence);

    // Removes and returns every fence still held, for deletion at shutdown.
    std::vector<void*> takeAllFences();

private:
    mutable std::mutex m_;
    uint32_t tex_[kSlots]        = {};
    void*    writeFence_[kSlots] = {};
    void*    readFence_[kSlots]  = {};
    int      latest_  = -1;
    int      showing_ = -1;
};

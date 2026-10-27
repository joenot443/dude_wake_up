#pragma once

#include <cstdint>
#include <mutex>
#include <vector>

// Hands finished engine frames to up to kReaders UI views (the stage, the
// node editor's live preview, a preview window, the inspector's preview and
// position pad) through a ring of GL textures in the shared namespace.
//
// Each slot has:
//   writeFence     — inserted by the engine after rendering into the slot;
//                    readers glWaitSync on it before sampling. Owned by the
//                    exchange; the engine deletes it when it next rewrites
//                    the slot.
//   readFence[r]   — inserted by reader r after sampling; the engine waits
//                    on (and deletes) these before rewriting the slot.
// The engine never writes the newest frame or any slot a reader is
// showing, so kReaders + 2 slots always leave one free.
//
// Fences are stored as void* (GLsync) so this header stays free of GL
// headers: the engine side uses GLEW, the UI side Qt's GL headers.
class FrameExchange {
public:
    // Stage, node editor, preview window, inspector, inspector pad.
    static constexpr int kReaders = 5;
    static constexpr int kSlots   = kReaders + 2;

    FrameExchange();

    void setTextures(const uint32_t (&ids)[kSlots]);
    uint32_t texture(int slot) const;

    // Engine side. Returns a slot to render into, the read fences to wait on
    // and delete, and the slot's previous write fence to delete.
    int  acquireWriteSlot(std::vector<void*>& readFencesOut, void** oldWriteFenceOut);
    void publish(int slot, void* writeFence);

    // Reader side. Returns the slot reader `reader` should display (-1 before
    // the first frame), its write fence (wait on it; don't delete), and
    // whether it is a frame this reader hasn't shown before. After drawing,
    // releaseReadSlot stores the reader's fence and returns the one it
    // replaces, for the caller to delete.
    int   acquireReadSlot(int reader, void** writeFenceOut, bool* isNew);
    void* releaseReadSlot(int reader, int slot, void* readFence);

    // Removes and returns every fence still held, for deletion at shutdown.
    std::vector<void*> takeAllFences();

private:
    mutable std::mutex m_;
    uint32_t tex_[kSlots]                  = {};
    void*    writeFence_[kSlots]           = {};
    void*    readFence_[kSlots][kReaders]  = {};
    uint64_t serial_[kSlots]               = {};
    uint64_t nextSerial_                   = 1;
    int      latest_                       = -1;
    int      showing_[kReaders];
    uint64_t shownSerial_[kReaders]        = {};
};

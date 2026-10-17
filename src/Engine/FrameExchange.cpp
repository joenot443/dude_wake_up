#include "FrameExchange.h"

void FrameExchange::setTextures(const uint32_t (&ids)[kSlots]) {
    std::lock_guard<std::mutex> lk(m_);
    for (int i = 0; i < kSlots; ++i) tex_[i] = ids[i];
}

uint32_t FrameExchange::texture(int slot) const {
    std::lock_guard<std::mutex> lk(m_);
    return (slot >= 0 && slot < kSlots) ? tex_[slot] : 0;
}

int FrameExchange::acquireWriteSlot(void** readFenceOut) {
    std::lock_guard<std::mutex> lk(m_);
    for (int s = 0; s < kSlots; ++s) {
        if (s == latest_ || s == showing_) continue;
        *readFenceOut = readFence_[s];
        readFence_[s] = nullptr;
        return s;
    }
    *readFenceOut = nullptr;
    return 0;  // unreachable with kSlots == 3
}

void* FrameExchange::publish(int slot, void* writeFence) {
    std::lock_guard<std::mutex> lk(m_);
    void* stale = nullptr;
    if (latest_ >= 0) {
        stale = writeFence_[latest_];
        writeFence_[latest_] = nullptr;
    }
    writeFence_[slot] = writeFence;
    latest_ = slot;
    return stale;
}

int FrameExchange::acquireReadSlot(void** writeFenceOut) {
    std::lock_guard<std::mutex> lk(m_);
    *writeFenceOut = nullptr;
    if (latest_ >= 0) {
        showing_ = latest_;
        *writeFenceOut = writeFence_[latest_];
        writeFence_[latest_] = nullptr;
        latest_ = -1;
    }
    return showing_;
}

void* FrameExchange::releaseReadSlot(int slot, void* readFence) {
    std::lock_guard<std::mutex> lk(m_);
    void* old = readFence_[slot];
    readFence_[slot] = readFence;
    return old;
}

std::vector<void*> FrameExchange::takeAllFences() {
    std::lock_guard<std::mutex> lk(m_);
    std::vector<void*> out;
    for (int s = 0; s < kSlots; ++s) {
        if (writeFence_[s]) out.push_back(writeFence_[s]);
        if (readFence_[s])  out.push_back(readFence_[s]);
        writeFence_[s] = readFence_[s] = nullptr;
    }
    latest_ = showing_ = -1;
    return out;
}

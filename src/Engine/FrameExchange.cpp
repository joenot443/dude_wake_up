#include "FrameExchange.h"

FrameExchange::FrameExchange() {
    for (int& s : showing_) s = -1;
}

void FrameExchange::setTextures(const uint32_t (&ids)[kSlots]) {
    std::lock_guard<std::mutex> lk(m_);
    for (int i = 0; i < kSlots; ++i) tex_[i] = ids[i];
}

uint32_t FrameExchange::texture(int slot) const {
    std::lock_guard<std::mutex> lk(m_);
    return (slot >= 0 && slot < kSlots) ? tex_[slot] : 0;
}

int FrameExchange::acquireWriteSlot(std::vector<void*>& readFencesOut, void** oldWriteFenceOut) {
    std::lock_guard<std::mutex> lk(m_);
    for (int s = 0; s < kSlots; ++s) {
        if (s == latest_) continue;
        bool shown = false;
        for (int r = 0; r < kReaders; ++r) shown |= (showing_[r] == s);
        if (shown) continue;
        for (int r = 0; r < kReaders; ++r) {
            if (readFence_[s][r]) readFencesOut.push_back(readFence_[s][r]);
            readFence_[s][r] = nullptr;
        }
        *oldWriteFenceOut = writeFence_[s];
        writeFence_[s] = nullptr;
        serial_[s] = 0;
        return s;
    }
    *oldWriteFenceOut = nullptr;
    return -1;  // unreachable: kSlots > kReaders + 1
}

void FrameExchange::publish(int slot, void* writeFence) {
    std::lock_guard<std::mutex> lk(m_);
    writeFence_[slot] = writeFence;
    serial_[slot] = nextSerial_++;
    latest_ = slot;
}

int FrameExchange::acquireReadSlot(int reader, void** writeFenceOut, bool* isNew) {
    std::lock_guard<std::mutex> lk(m_);
    *isNew = false;
    if (latest_ >= 0 && serial_[latest_] != shownSerial_[reader]) {
        showing_[reader] = latest_;
        shownSerial_[reader] = serial_[latest_];
        *isNew = true;
    }
    const int slot = showing_[reader];
    *writeFenceOut = slot >= 0 ? writeFence_[slot] : nullptr;
    return slot;
}

void* FrameExchange::releaseReadSlot(int reader, int slot, void* readFence) {
    std::lock_guard<std::mutex> lk(m_);
    void* old = readFence_[slot][reader];
    readFence_[slot][reader] = readFence;
    return old;
}

std::vector<void*> FrameExchange::takeAllFences() {
    std::lock_guard<std::mutex> lk(m_);
    std::vector<void*> out;
    for (int s = 0; s < kSlots; ++s) {
        if (writeFence_[s]) out.push_back(writeFence_[s]);
        writeFence_[s] = nullptr;
        for (int r = 0; r < kReaders; ++r) {
            if (readFence_[s][r]) out.push_back(readFence_[s][r]);
            readFence_[s][r] = nullptr;
        }
        serial_[s] = 0;
    }
    latest_ = -1;
    for (int r = 0; r < kReaders; ++r) { showing_[r] = -1; shownSerial_[r] = 0; }
    return out;
}

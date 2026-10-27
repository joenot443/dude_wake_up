//
//  SystemAudioSource.hpp
//  dude_wake_up
//
//  Created by Claude on 9/6/25.
//

#ifndef SystemAudioSource_hpp
#define SystemAudioSource_hpp

#include "AudioSource.hpp"
#include <atomic>
#include <thread>

#ifdef __APPLE__
#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioToolbox.h>
#include <sys/types.h>   // pid_t
#endif

#ifdef _WIN32
// Full inclusion of Windows COM headers in this header is deliberate — the
// members below are typed by their real Win32 identity so consumers get
// static checking. Cost is that every TU including SystemAudioSource.hpp on
// Windows pulls in windows.h + audioclient.h. Acceptable — the header is a
// leaf and consumers are few. If this ever grows onerous, refactor to pimpl.
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <mmdeviceapi.h>
#include <audioclient.h>
#include <wrl/client.h>   // Microsoft::WRL::ComPtr

// Portable process-ID matching the WASAPI process-loopback API surface.
// DWORD on Windows; typedef to uint32_t so headers don't need windows.h
// pulled in from other platforms.
using pid_t = uint32_t;
#endif

#if !defined(__APPLE__) && !defined(_WIN32)
#include <cstdint>
using pid_t = uint32_t;
#endif

#ifdef __APPLE__
#ifdef __OBJC__
@class CATapDescription;
#else
// Placeholder for non-Objective-C++ Apple builds
typedef struct objc_object CATapDescription;
#endif
#endif

class SystemAudioSource : public AudioSource {
private:
#ifdef __APPLE__
    AudioDeviceID aggregateDevice = kAudioObjectUnknown;
    AudioObjectID processTapID = kAudioObjectUnknown;
    AudioDeviceIOProcID ioProcID = nullptr;
    CATapDescription* tapDescription = nullptr;
    AudioObjectID processObjectID = kAudioObjectUnknown;

    // Audio format information (macOS Core Audio)
    AudioStreamBasicDescription streamFormat;
#endif

#ifdef _WIN32
    // WASAPI loopback capture. See SystemAudioSource_win.cpp for lifecycle.
    Microsoft::WRL::ComPtr<IMMDevice> device_;
    Microsoft::WRL::ComPtr<IAudioClient> audioClient_;
    Microsoft::WRL::ComPtr<IAudioCaptureClient> captureClient_;
    HANDLE captureEvent_ = nullptr;
    std::thread captureThread_;
    std::atomic<bool> running_{false};
    WAVEFORMATEXTENSIBLE mixFormat_{};   // populated from GetMixFormat
    uint32_t sampleRate_ = 48000;        // filled from mixFormat_
    uint16_t channels_ = 2;              // filled from mixFormat_
#endif

    bool isGlobalTap = true;
    pid_t targetPID = 0;
    uint32_t bufferFrameSize = 512;

    // Internal methods (macOS Core Audio; Windows uses WASAPI in Phase 3)
#ifdef __APPLE__
    bool createSystemAudioTap();
    bool setupAudioDevice();
    void cleanupTap();
    static OSStatus audioIOCallback(AudioObjectID inDevice,
                                   const AudioTimeStamp* inNow,
                                   const AudioBufferList* inInputData,
                                   const AudioTimeStamp* inInputTime,
                                   AudioBufferList* outOutputData,
                                   const AudioTimeStamp* inOutputTime,
                                   void* inClientData);

    AudioObjectID getSystemObject();
    AudioObjectID translatePIDToProcessObject(pid_t pid);
#endif

#ifdef _WIN32
    void captureLoop_();
    void stopCaptureThread_();
#endif

public:
    SystemAudioSource();
    virtual ~SystemAudioSource();

    void setup() override;
    void disable() override;
    AudioSourceType type() override { return AudioSourceType_System; }
    float getSampleRate() const override {
#ifdef __APPLE__
        return static_cast<float>(streamFormat.mSampleRate);
#elif defined(_WIN32)
        return static_cast<float>(sampleRate_);
#else
        return 44100.0f;
#endif
    }

    // Configuration methods.
    // pid=0 (or setGlobalTap(true)) means "all system audio". Non-zero pid
    // captures loopback from that process only (Windows: process-loopback via
    // ActivateAudioInterfaceAsync, Win 10 2004+). Zero in-tree callers pass
    // a specific pid today, so widening from pid_t across platforms is safe.
    void setTargetProcess(pid_t pid);
    void setGlobalTap(bool global);

    json serialize() override;
    void load(json j) override;
};

#endif /* SystemAudioSource_hpp */

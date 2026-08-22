//
//  SystemAudioSource.hpp
//  dude_wake_up
//
//  Created by Claude on 9/6/25.
//

#ifndef SystemAudioSource_hpp
#define SystemAudioSource_hpp

#include "AudioSource.hpp"

#ifdef __APPLE__
#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioToolbox.h>
#include <sys/types.h>   // pid_t
#else
#include <cstdint>
// Portable process-ID for the cross-platform API. On Windows, real WASAPI
// process-loopback IDs are DWORD, but int32_t covers them without pulling in
// Windows.h from a header. The Phase 3 Windows implementation converts.
using pid_t = int32_t;
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
    bool isGlobalTap = true;
    pid_t targetPID = -1;
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

public:
    SystemAudioSource();
    virtual ~SystemAudioSource();

    void setup() override;
    void disable() override;
    AudioSourceType type() override { return AudioSourceType_System; }
    float getSampleRate() const {
#ifdef __APPLE__
        return static_cast<float>(streamFormat.mSampleRate);
#else
        return 44100.0f;
#endif
    }

    // Configuration methods
    void setTargetProcess(pid_t pid);
    void setGlobalTap(bool global);

    json serialize() override;
    void load(json j) override;
};

#endif /* SystemAudioSource_hpp */

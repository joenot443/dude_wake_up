//
//  SystemAudioSource.hpp
//  dude_wake_up
//
//  Created by Claude on 9/6/25.
//

#ifndef SystemAudioSource_hpp
#define SystemAudioSource_hpp

#include "AudioSource.hpp"
#include <CoreAudio/CoreAudio.h>
#include <AudioToolbox/AudioToolbox.h>

#ifdef __OBJC__
@class CATapDescription;
#else
// Placeholder for non-Objective-C++ builds
typedef struct objc_object CATapDescription;
#endif

class SystemAudioSource : public AudioSource {
private:
    AudioDeviceID aggregateDevice = kAudioObjectUnknown;
    AudioObjectID processTapID = kAudioObjectUnknown;
    AudioDeviceIOProcID ioProcID = nullptr;
    CATapDescription* tapDescription = nullptr;
    AudioObjectID processObjectID = kAudioObjectUnknown;
    bool isGlobalTap = true;
    pid_t targetPID = -1;
    
    // Audio format information
    AudioStreamBasicDescription streamFormat;
    UInt32 bufferFrameSize = 512;
    
    // Internal methods
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
    
public:
    SystemAudioSource();
    virtual ~SystemAudioSource();
    
    void setup() override;
    void disable() override;
    AudioSourceType type() override { return AudioSourceType_System; }
    
    // Configuration methods
    void setTargetProcess(pid_t pid);
    void setGlobalTap(bool global);
    
    json serialize() override;
    void load(json j) override;
};

#endif /* SystemAudioSource_hpp */
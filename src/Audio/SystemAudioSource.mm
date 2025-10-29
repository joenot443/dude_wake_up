//
//  SystemAudioSource.mm
//  dude_wake_up
//
//  Created by Claude on 9/6/25.
//

#include "SystemAudioSource.hpp"
#include "Console.hpp"

#ifdef __OBJC__
#import <Foundation/Foundation.h>
#import <CoreAudio/CoreAudio.h>
#import <AudioToolbox/AudioToolbox.h>

// Forward declare CATap classes (available in macOS 14.2+)
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 140200
@interface CATapDescription : NSObject
- (instancetype)initMonoGlobalTapButExcludeProcesses:(NSArray<NSNumber*>*)processes;
- (instancetype)initProcessTap:(AudioObjectID)processObjectID;
@property (nonatomic, readonly) NSUUID* UUID;
@end

// Core Audio Tap functions
extern "C" {
    OSStatus AudioHardwareCreateProcessTap(CATapDescription* tapDescription, AudioObjectID* tapObjectID);
    OSStatus AudioHardwareDestroyProcessTap(AudioObjectID tapObjectID);
}
#endif
#endif

SystemAudioSource::SystemAudioSource() {
    name = "System Audio";
    id = "system_audio_tap";
    
    // Initialize stream format to match typical system audio
    memset(&streamFormat, 0, sizeof(streamFormat));
    streamFormat.mSampleRate = 48000.0;
    streamFormat.mFormatID = kAudioFormatLinearPCM;
    streamFormat.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
    streamFormat.mChannelsPerFrame = 2;
    streamFormat.mBitsPerChannel = 32;
    streamFormat.mFramesPerPacket = 1;
    streamFormat.mBytesPerFrame = streamFormat.mChannelsPerFrame * sizeof(Float32);
    streamFormat.mBytesPerPacket = streamFormat.mBytesPerFrame;
}

SystemAudioSource::~SystemAudioSource() {
    disable();
}

void SystemAudioSource::setup() {
#ifdef __OBJC__
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 140200
    if (active) {
        log("SystemAudioSource already active");
        return;
    }
    
    if (!createSystemAudioTap()) {
        log("Failed to create system audio tap");
        return;
    }
    
    if (!setupAudioDevice()) {
        log("Failed to setup audio device for tap");
        cleanupTap();
        return;
    }
    
    // Configure Gist for audio analysis
    gist.setAudioFrameSize(bufferFrameSize);
    gist.setSamplingFrequency(streamFormat.mSampleRate);
    
    // Start the audio device using the IO proc we created
    OSStatus status = AudioDeviceStart(aggregateDevice, ioProcID);
    if (status != noErr) {
        log("Failed to start audio device: %d", status);
        cleanupTap();
        return;
    }

    // Initialize bpmStartTime to current time
    auto currentTime = std::chrono::steady_clock::now();
    audioAnalysis.bpmStartTime = std::chrono::duration_cast<std::chrono::milliseconds>(currentTime.time_since_epoch()).count();

    active = true;
    log("SystemAudioSource activated successfully");
#else
    log("SystemAudioSource requires macOS 14.2 or later");
#endif
#else
    log("SystemAudioSource requires Objective-C++ compilation");
#endif
}

void SystemAudioSource::disable() {
    if (!active) return;
    
    if (aggregateDevice != kAudioObjectUnknown && ioProcID != nullptr) {
        AudioDeviceStop(aggregateDevice, ioProcID);
        AudioDeviceDestroyIOProcID(aggregateDevice, ioProcID);
        ioProcID = nullptr;
    }
    
    cleanupTap();
    active = false;
    log("SystemAudioSource disabled");
}

bool SystemAudioSource::createSystemAudioTap() {
#ifdef __OBJC__
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 140200
    @autoreleasepool {
        OSStatus status;
        
        // Create tap description with minimal setup
        tapDescription = [[CATapDescription alloc] initMonoGlobalTapButExcludeProcesses:@[]];
        
        // Create the process tap
        status = AudioHardwareCreateProcessTap(tapDescription, &processTapID);
        if (status != noErr) {
            log("AudioHardwareCreateProcessTap failed: %d", status);
            tapDescription = nil;
            return false;
        }
        
        return true;
    }
#else
    return false;
#endif
#else
    return false;
#endif
}

bool SystemAudioSource::setupAudioDevice() {
    if (processTapID == kAudioObjectUnknown || tapDescription == nullptr) {
        return false;
    }
    
    OSStatus status;
    
#ifdef __OBJC__
    @autoreleasepool {
        // Get default input and output devices as shown in the guide
        AudioObjectPropertyAddress propertyAddress = {
            kAudioHardwarePropertyDefaultInputDevice,
            kAudioObjectPropertyScopeGlobal,
            kAudioObjectPropertyElementMain
        };
        
        UInt32 propertySize = sizeof(AudioDeviceID);
        AudioDeviceID defaultInputDevice = kAudioObjectUnknown;
        status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, nullptr, &propertySize, &defaultInputDevice);
        
        propertyAddress.mSelector = kAudioHardwarePropertyDefaultOutputDevice;
        AudioDeviceID defaultOutputDevice = kAudioObjectUnknown;
        status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress, 0, nullptr, &propertySize, &defaultOutputDevice);
        
        // Get device UIDs
        propertyAddress.mSelector = kAudioDevicePropertyDeviceUID;
        CFStringRef inputUID = nullptr;
        CFStringRef outputUID = nullptr;
        
        propertySize = sizeof(CFStringRef);
        AudioObjectGetPropertyData(defaultInputDevice, &propertyAddress, 0, nullptr, &propertySize, &inputUID);
        AudioObjectGetPropertyData(defaultOutputDevice, &propertyAddress, 0, nullptr, &propertySize, &outputUID);
        
        // Get the tap's UUID
        NSString* tapUUID = [[tapDescription UUID] UUIDString];
        NSUUID* aggregateUID = [NSUUID UUID];
        
        NSDictionary* aggregateDescription = @{
            @(kAudioAggregateDeviceUIDKey): [aggregateUID UUIDString],
            @(kAudioAggregateDeviceIsPrivateKey): @(1),
            @(kAudioAggregateDeviceIsStackedKey): @(0),
            @(kAudioAggregateDeviceMasterSubDeviceKey): (__bridge NSString*)inputUID,
            @(kAudioAggregateDeviceSubDeviceListKey): @[
                @{
                    @(kAudioSubDeviceUIDKey): (__bridge NSString*)inputUID,
                    @(kAudioSubDeviceDriftCompensationKey): @(1),
                    @(kAudioSubDeviceDriftCompensationQualityKey): @(kAudioSubDeviceDriftCompensationMaxQuality),
                },
                @{
                    @(kAudioSubDeviceUIDKey): (__bridge NSString*)outputUID,
                    @(kAudioSubDeviceDriftCompensationKey): @(1),
                    @(kAudioSubDeviceDriftCompensationQualityKey): @(kAudioSubDeviceDriftCompensationMaxQuality),
                },
            ],
            @(kAudioAggregateDeviceTapListKey): @[
                @{
                    @(kAudioSubTapDriftCompensationKey): @(1),
                    @(kAudioSubTapUIDKey): tapUUID,
                },
            ],
        };
        
        AudioDeviceID aggregateDeviceID = kAudioObjectUnknown;
        status = AudioHardwareCreateAggregateDevice((__bridge CFDictionaryRef)aggregateDescription, &aggregateDeviceID);
        
        // Clean up CFStrings
        if (inputUID) CFRelease(inputUID);
        if (outputUID) CFRelease(outputUID);
        
        if (status != noErr) {
            log("Failed to create aggregate device: %d", status);
            return false;
        }
        
        aggregateDevice = aggregateDeviceID;
        
        // Use default format
        streamFormat.mSampleRate = 48000.0;
        streamFormat.mFormatID = kAudioFormatLinearPCM;
        streamFormat.mFormatFlags = kAudioFormatFlagIsFloat | kAudioFormatFlagIsPacked;
        streamFormat.mChannelsPerFrame = 2;
        streamFormat.mBitsPerChannel = 32;
        streamFormat.mFramesPerPacket = 1;
        streamFormat.mBytesPerFrame = streamFormat.mChannelsPerFrame * sizeof(Float32);
        streamFormat.mBytesPerPacket = streamFormat.mBytesPerFrame;
        
        // Create IO callback for the aggregate device
        status = AudioDeviceCreateIOProcID(aggregateDevice, audioIOCallback, this, &ioProcID);
        
        if (status != noErr) {
            log("Failed to create IO proc: %d", status);
            return false;
        }
        
        return true;
    }
#else
    return false;
#endif
}

void SystemAudioSource::cleanupTap() {
#ifdef __OBJC__
#if __MAC_OS_X_VERSION_MAX_ALLOWED >= 140200
    // Destroy aggregate device if it exists
    if (aggregateDevice != kAudioObjectUnknown) {
        AudioHardwareDestroyAggregateDevice(aggregateDevice);
        aggregateDevice = kAudioObjectUnknown;
    }
    
    // Clean up the process tap separately if we created one
    if (processTapID != kAudioObjectUnknown) {
        AudioHardwareDestroyProcessTap(processTapID);
        processTapID = kAudioObjectUnknown;
    }
    
    @autoreleasepool {
        tapDescription = nil;
    }
#endif
#endif
    
    processObjectID = kAudioObjectUnknown;
}

AudioObjectID SystemAudioSource::getSystemObject() {
    return kAudioObjectSystemObject;
}

AudioObjectID SystemAudioSource::translatePIDToProcessObject(pid_t pid) {
    AudioObjectPropertyAddress propertyAddress = {
        kAudioHardwarePropertyTranslatePIDToProcessObject,
        kAudioObjectPropertyScopeGlobal,
        kAudioObjectPropertyElementMain
    };
    
    AudioObjectID processObject = kAudioObjectUnknown;
    UInt32 propertySize = sizeof(processObject);
    
    OSStatus status = AudioObjectGetPropertyData(kAudioObjectSystemObject, &propertyAddress,
                                                sizeof(pid_t), &pid,
                                                &propertySize, &processObject);
    
    if (status != noErr) {
        log("Failed to translate PID %d to process object: %d", pid, status);
        return kAudioObjectUnknown;
    }
    
    return processObject;
}

OSStatus SystemAudioSource::audioIOCallback(AudioObjectID inDevice,
                                           const AudioTimeStamp* inNow,
                                           const AudioBufferList* inInputData,
                                           const AudioTimeStamp* inInputTime,
                                           AudioBufferList* outOutputData,
                                           const AudioTimeStamp* inOutputTime,
                                           void* inClientData) {
    
    SystemAudioSource* source = static_cast<SystemAudioSource*>(inClientData);
    if (!source || !source->active) {
        return noErr;
    }
    
    if (inInputData && inInputData->mNumberBuffers > 0) {
        // Check all buffers to find the one with audio data
        int bufferWithData = -1;
        float maxSampleOverall = 0.0f;
        
        for (UInt32 bufferIndex = 0; bufferIndex < inInputData->mNumberBuffers; bufferIndex++) {
            const AudioBuffer& buffer = inInputData->mBuffers[bufferIndex];
            const float* samples = static_cast<const float*>(buffer.mData);
            
            if (!samples || buffer.mDataByteSize == 0) continue;
            
            UInt32 numSamples = buffer.mDataByteSize / sizeof(float);
            float maxSampleInBuffer = 0.0f;
            
            for (UInt32 i = 0; i < numSamples; i++) {
                maxSampleInBuffer = MAX(maxSampleInBuffer, fabs(samples[i]));
            }
            
            if (maxSampleInBuffer > maxSampleOverall) {
                maxSampleOverall = maxSampleInBuffer;
                bufferWithData = bufferIndex;
            }
        }
        
        // Use the buffer with the most audio activity (or buffer 0 if all are silent)
        int bufferToUse = (bufferWithData >= 0) ? bufferWithData : 0;
        
        const AudioBuffer& buffer = inInputData->mBuffers[bufferToUse];
        const float* samples = static_cast<const float*>(buffer.mData);
        
        if (samples && buffer.mDataByteSize > 0) {
            UInt32 numFrames = buffer.mDataByteSize / sizeof(float) / source->streamFormat.mChannelsPerFrame;
            
            // Create ofSoundBuffer from the audio data
            ofSoundBuffer soundBuffer;
            soundBuffer.allocate(numFrames, source->streamFormat.mChannelsPerFrame);
            
            for (UInt32 frame = 0; frame < numFrames; frame++) {
                for (UInt32 channel = 0; channel < source->streamFormat.mChannelsPerFrame; channel++) {
                    UInt32 sampleIndex = frame * source->streamFormat.mChannelsPerFrame + channel;
                    if (sampleIndex < buffer.mDataByteSize / sizeof(float)) {
                        soundBuffer.getSample(frame, channel) = samples[sampleIndex];
                    }
                }
            }
            
            // Process through AudioSource pipeline
            source->audioIn(soundBuffer);
        }
    }
    
    return noErr;
}

void SystemAudioSource::setTargetProcess(pid_t pid) {
    targetPID = pid;
    isGlobalTap = false;
}

void SystemAudioSource::setGlobalTap(bool global) {
    isGlobalTap = global;
    if (global) {
        targetPID = -1;
    }
}

json SystemAudioSource::serialize() {
    json j = AudioSource::serialize();
    j["type"] = "system";
    j["globalTap"] = isGlobalTap;
    j["targetPID"] = targetPID;
    return j;
}

void SystemAudioSource::load(json j) {
    AudioSource::load(j);
    if (j.contains("globalTap")) {
        isGlobalTap = j["globalTap"];
    }
    if (j.contains("targetPID")) {
        targetPID = j["targetPID"];
    }
}

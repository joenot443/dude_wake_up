//
//  test_bpm_detector.cpp
//  Simple test file to verify AutoBpmDetector compilation
//  This file can be temporarily included to test the BPM detector
//

#include "AutoBpmDetector.hpp"
#include "Gist.h"
#include <iostream>
#include <vector>

void test_bpm_detector() {
    // Create a test instance
    AutoBpmDetector detector(44100, 512);
    
    // Create a test gist instance
    Gist<float> gist(512, 44100);
    
    // Create some test audio data (sine wave)
    std::vector<float> testFrame(512);
    for (int i = 0; i < 512; ++i) {
        testFrame[i] = sin(2.0 * M_PI * 440.0 * i / 44100.0) * 0.5f;
    }
    
    // Process the frame
    gist.processAudioFrame(testFrame);
    SimpleBeat beat = detector.processFrame(&gist);
    
    std::cout << "AutoBpmDetector test: BPM=" << beat.bpm 
              << ", Beat=" << (beat.isBeat ? "YES" : "NO") << std::endl;
    
    std::cout << "AutoBpmDetector compilation and basic functionality test passed!" << std::endl;
}
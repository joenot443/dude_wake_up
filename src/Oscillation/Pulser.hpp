//
//  Pulser.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/24/24.
//

#ifndef Pulser_hpp
#define Pulser_hpp

#include <stdio.h>
#include "Parameter.hpp"
#include "ofMain.h"

#include <cmath>

class Pulser {
public:
  std::shared_ptr<Parameter> threshold;
  std::shared_ptr<Parameter> duration;
  float startTime;
  bool isPulsing;

  // Default constructor
  Pulser() : threshold(nullptr), duration(nullptr), startTime(0), isPulsing(false) {}

  Pulser(std::shared_ptr<Parameter> threshold, std::shared_ptr<Parameter> duration)
  : threshold(threshold), duration(duration), startTime(0), isPulsing(false) {}
  
  float next(float value) {
    // Safety check: ensure parameters are initialized
    if (!threshold || !duration) {
      return 0.0f;
    }

    double currentTime = ofGetSystemTimeMillis() / 1000.0f;
    double delta = currentTime - startTime;
    double pulsePct = isPulsing ? delta / (double) duration->value : 0.0;
    
    // Allow the pulse to restart after 80% through
    if ((!isPulsing || pulsePct > 0.9) && value > threshold->value) {
      // Start a new pulse
      isPulsing = true;
      startTime = currentTime;
    }
    pulsePct = isPulsing ? delta / (double) duration->value : 0.0;

    if (delta < 0.0) {
      return 0.0f;
    }
    
    if (isPulsing) {
      if (delta < duration->value) {
        // First 50%
        if (pulsePct < 0.5) {
          double thirdPct = (0.3 - pulsePct) / 0.3;
          double y = 3.2457*pow(pulsePct, 4) + 3.2457*pulsePct;
          return y;
        }
        
        const float B = -5.0f; // Controls the steepness
        const float C = 0.0f; // Offset
        double y = -10*pow(pulsePct-0.5, 3) + 1;
        return y;
      } else {
        // End the pulse
        isPulsing = false;
        return 0.0f;
      }
    }
    
    return 0.0f; // Default return when not pulsing
  }
};

#endif /* Pulser_hpp */

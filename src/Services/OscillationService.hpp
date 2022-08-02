//
//  OscillationService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-05.
//

#ifndef OscillationService_hpp
#define OscillationService_hpp

#include <stdio.h>
#include <vector>
#include <deque>
#include "Oscillator.hpp"
#include "ValueOscillator.hpp"

class OscillationService {
private:
  std::deque<Oscillator *> oscillators;
  std::deque<ValueOscillator *> valueOscillators;

public:
  static OscillationService* service;
  OscillationService() {};
  static OscillationService* getService() {
    if (!service) {
      service = new OscillationService;
    }
    return service;
    
  }
  void addOscillator(Oscillator *);
  void addValueOscillator(ValueOscillator *);
  void tickOscillators();
};

#endif /* OscillationService_hpp */

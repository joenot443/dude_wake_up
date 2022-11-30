//
//  MidiPairing.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-21.
//

#ifndef MidiPairing_hpp
#define MidiPairing_hpp

#include <stdio.h>
#include "Parameter.hpp"

struct MidiPairing {
  std::string paramId;
  std::string descriptor;
  MidiPairing() {};
  MidiPairing(std::string paramId, std::string descriptor) : paramId(paramId), descriptor(descriptor) {}
};

#endif /* MidiPairing_hpp */

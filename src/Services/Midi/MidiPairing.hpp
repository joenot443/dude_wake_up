//
//  MidiPairing.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-21.
//

#ifndef MidiPairing_hpp
#define MidiPairing_hpp

#include <stdio.h>
#include "Parameter.h"

struct MidiPairing {
  std::string paramId;
  std::string descriptor;
  int channel;
  int portNum;
  int status;
  int control;
  MidiPairing() {};
  MidiPairing(std::string paramId, std::string descriptor, int channel, int portNum, int status, int control) : paramId(paramId), descriptor(descriptor), channel(channel), portNum(portNum), status(status), control(control) {}
};

#endif /* MidiPairing_hpp */

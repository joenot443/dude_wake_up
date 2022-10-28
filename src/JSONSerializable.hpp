//
//  JSONSerializable.h
//  dude_wake_up
//
//  Created by Joe Crozier on 10/24/22.
//

#ifndef JSONSerializable_h
#define JSONSerializable_h

using json = nlohmann::json;

class JSONSerializable {
public:
  std::vector<Parameter*> parameters;
  std::vector<Oscillator*> oscillators;
  
  json serialize() {
    json j;
    for (auto p : parameters) {
      j[p->paramId] = p->paramValue();
    }
    auto ops = Oscillator::parametersFromOscillators(oscillators);
    for (auto p : ops) {
      j[p->paramId] = p->paramValue();
    }
    return j;
  }
};


#endif /* JSONSerializable_h */

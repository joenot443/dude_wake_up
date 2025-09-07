//
//  MidiService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-20.
//

#ifndef MidiService_hpp
#define MidiService_hpp

#include <stdio.h>
#include "ofxMidi.h"
#include "ofMain.h"
#include "MidiPairing.hpp"
#include "Parameter.hpp"
#include "ConfigurableService.hpp"

class MidiService: public ofxMidiListener, public ofxMidiConnectionListener, public ConfigurableService {
private:
  std::map<std::string, MidiPairing> descriptorToPairing;
  std::map<std::string, MidiPairing> parameterIdToPairing;
  std::map<std::string, std::string> descriptorToParamId;
  
  void driveParameter(std::string paramId, ofxMidiMessage &msg);
  std::string descriptorFrom(ofxMidiMessage& msg);
  ofxMidiMessage messageFrom(std::string descriptor);
  
  // Boilerplate
  // add a message to the display queue
  std::vector<ofxMidiIn*> inputs;
  std::vector<ofxMidiOut*> outputs;
  std::deque<string> messages;
  int maxMessages;
  ofMutex messageMutex; // make sure we don't read from queue while writing
  int note, ctl;
  std::vector<unsigned char> bytes;
  bool midiInitialized;
  void addMessage(std::string msg);
  void newMidiMessage(ofxMidiMessage& msg) override;
  void midiInputAdded(std::string name, bool isNetwork) override;
  void midiInputRemoved(std::string name, bool isNetwork) override;
  void midiOutputAdded(std::string nam, bool isNetwork) override;
  void midiOutputRemoved(std::string name, bool isNetwork) override;
  void removePairing(std::shared_ptr<Parameter> param);
  
public:
  void setup();
  void update();
  void stopLearning();
  void beginLearning(std::shared_ptr<Parameter> parameter);
  void saveAssignment(std::shared_ptr<Parameter> param, std::string descriptor);
  bool isLearning();
  void initializeMidiPorts();

  bool hasPairingForDescriptor(std::string descriptor);
  bool hasPairingForParameterId(std::string paramId);
  
  
  MidiPairing* pairingForParameterId(std::string paramId);
  MidiPairing* pairingForDescriptor(std::string descriptor);
  
  /// Parameter currently being learned.
  std::shared_ptr<Parameter> learningParam;
  
  static MidiService* service;
  MidiService() {};
  static MidiService* getService() {
    if (!service) {
      service = new MidiService;
      service->setup();
    }
    return service;
  }
  
  json config() override;
  void loadConfig(json j) override;
};


#endif /* MidiService_hpp */

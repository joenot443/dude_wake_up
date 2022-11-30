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

class MidiService: public ofxMidiListener, public ofxMidiConnectionListener {
private:
  std::map<std::string, MidiPairing> descriptorToPairing;
  std::map<std::string, MidiPairing> parameterIdToPairing;
  
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
  void addMessage(std::string msg);
  void newMidiMessage(ofxMidiMessage& msg);
  void midiInputAdded(std::string name, bool isNetwork);
  void midiInputRemoved(std::string name, bool isNetwork);
  void midiOutputAdded(std::string nam, bool isNetwork);
  void midiOutputRemoved(std::string name, bool isNetwork);
  void removePairing(std::shared_ptr<Parameter> param);
  
public:
  void setup();
  void stopLearning();
  void beginLearning(std::shared_ptr<Parameter> parameter);
  void saveAssignment(std::shared_ptr<Parameter> param, std::string descriptor);
  bool isLearning();
  
  bool hasPairingForDescriptor(std::string descriptor);
  bool hasPairingForParameterId(std::string paramId);
  
  
  MidiPairing* pairingForParameterId(std::string paramId);
  MidiPairing* pairingForDescriptor(std::string descriptor);
  
  /// Parameter currently being learned.
  std::shared_ptr<Parameter> learningParam;
  
  void saveConfigFile();
  void loadConfigFile();
  
  static MidiService* service;
  MidiService() {};
  static MidiService* getService() {
    if (!service) {
      service = new MidiService;
      service->setup();
    }
    return service;
    
  }
};


#endif /* MidiService_hpp */

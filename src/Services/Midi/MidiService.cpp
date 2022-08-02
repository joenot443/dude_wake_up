//
//  MidiService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-20.
//

#include "MidiService.hpp"
#include "ParameterService.h"
#include "json.hpp"
#include "Console.h"
#include "Strings.h"
#include <ostream>

using json = nlohmann::json;

void MidiService::setup() {
  // list the number of available input & output ports
  ofxMidiIn input;
  ofxMidiOut output;
  input.listInPorts();
  output.listOutPorts();
  learningParam = NULL;
  
  // create and open input ports
  for(int i = 0; i < input.getNumInPorts(); ++i) {
    // new object
    inputs.push_back(new ofxMidiIn);
    
    // set this class to receive incoming midi events
    inputs[i]->addListener(this);
    
    // open input port via port number
    inputs[i]->openPort(i);
  }
  
  // create and open output ports
  for(int i = 0; i < output.getNumOutPorts(); ++i) {
    
    // new object
    outputs.push_back(new ofxMidiOut);
    
    // open input port via port number
    outputs[i]->openPort(i);
  }
  
  // set this class to receieve midi device (dis)connection events
  ofxMidi::setConnectionListener(this);
}
void MidiService::midiInputAdded(std::string name, bool isNetwork) {
  std::stringstream msg;
  msg << "ofxMidi: input added: " << name << " network: " << isNetwork;
  addMessage(msg.str());
  
  // create and open a new input port
  ofxMidiIn *newInput = new ofxMidiIn;
  newInput->openPort(name);
  newInput->addListener(this);
  inputs.push_back(newInput);
}

//--------------------------------------------------------------
void MidiService::midiInputRemoved(std::string name, bool isNetwork) {
  std::stringstream msg;
  msg << "ofxMidi: input removed: " << name << " network: " << isNetwork << endl;
  addMessage(msg.str());
  
  // close and remove input port
  std::vector<ofxMidiIn*>::iterator iter;
  for(iter = inputs.begin(); iter != inputs.end(); ++iter) {
    ofxMidiIn *input = (*iter);
    if(input->getName() == name) {
      input->closePort();
      input->removeListener(this);
      delete input;
      inputs.erase(iter);
      break;
    }
  }
}

//--------------------------------------------------------------
void MidiService::midiOutputAdded(string name, bool isNetwork) {
  stringstream msg;
  msg << "ofxMidi: output added: " << name << " network: " << isNetwork << endl;
  addMessage(msg.str());
  
  // create and open new output port
  ofxMidiOut *newOutput = new ofxMidiOut;
  newOutput->openPort(name);
  outputs.push_back(newOutput);
}

//--------------------------------------------------------------
void MidiService::midiOutputRemoved(std::string name, bool isNetwork) {
  std::stringstream msg;
  msg << "ofxMidi: output removed: " << name << " network: " << isNetwork << endl;
  addMessage(msg.str());
  
  // close and remove output port
  std::vector<ofxMidiOut*>::iterator iter;
  for(iter = outputs.begin(); iter != outputs.end(); ++iter) {
    ofxMidiOut *output = (*iter);
    if(output->getName() == name) {
      output->closePort();
      delete output;
      outputs.erase(iter);
      break;
    }
  }
}

//--------------------------------------------------------------
void MidiService::newMidiMessage(ofxMidiMessage& msg) {
  if (learningParam != NULL) {
    saveAssignment(learningParam, msg);
    saveConfigFile();
    return;
  }
  auto descriptor = descriptorFrom(msg);
  if (hasPairingForDescriptor(descriptor)) {
    auto pairing = descriptorToPairing[descriptor];
    driveParameter(pairing.paramId, msg);
  }
  
  addMessage(msg.toString());
}

void MidiService::driveParameter(std::string paramId, ofxMidiMessage &msg) {
  auto param = ParameterService::getService()->parameterForId(paramId);
  if (param == NULL) {
    log("Failed to get Parameter for MIDI message %s");
    return;
  }
  param->driveValue(msg.value / 127.0);
}

std::string MidiService::descriptorFrom(ofxMidiMessage &msg) {
  std::stringstream stream;
  stream << msg.portNum << ":" << msg.portName << ":" << msg.status << ":" << msg.channel << ":" << msg.control << ":" << "$";
  return stream.str();
}

ofxMidiMessage MidiService::messageFrom(std::string description) {
  std::vector<std::string> parts = split_string(description, ":");
  ofxMidiMessage msg;
  msg.portNum = std::stoi(parts[0]);
  msg.portName = parts[1];
  msg.status = MidiStatus(std::stoi(parts[2]));
  msg.channel = std::stoi(parts[3]);
  msg.control = std::stoi(parts[4]);
  return msg;
}

void MidiService::addMessage(std::string msg) {
  messageMutex.lock();
  messages.push_back(msg);
  while(messages.size() > maxMessages) {
    messages.pop_front();
  }
  messageMutex.unlock();
}

void MidiService::beginLearning(Parameter *parameter) {
  log("Starting to learn for %s", parameter->name.c_str());
  removePairing(parameter);
  learningParam = parameter;
}

void MidiService::stopLearning() {
  log("Stopping learning");
  learningParam = NULL;
}

void MidiService::saveAssignment(Parameter *param, ofxMidiMessage &msg) {
  std::string descriptor = descriptorFrom(msg);
  log("Saving %s for %s", param->name.c_str(), descriptor.c_str());
  auto pairing = MidiPairing(param->paramId, descriptor, msg.channel, msg.portNum, msg.status, msg.control);
  descriptorToPairing[descriptor] = pairing;
  parameterIdToPairing[param->paramId] = pairing;
  
  stopLearning();
}

void MidiService::removePairing(Parameter *param) {
  if (!hasPairingForParameterId(param->paramId)) {
    log("No need to remove %s", param->name.c_str());
    return;
  }
  
  auto pairing = pairingForParameterId(param->paramId);
  auto descriptor = pairing->descriptor;
  log("Removing %s for %d", param->name.c_str(), descriptor.c_str());
  descriptorToPairing.erase(descriptor);
  parameterIdToPairing.erase(param->paramId);
}

bool MidiService::hasPairingForDescriptor(std::string descriptor) {
  return descriptorToPairing.count(descriptor) > 0;
}

bool MidiService::hasPairingForParameterId(std::string paramId) {
  return parameterIdToPairing.count(paramId) > 0;
}

bool MidiService::isLearning() {
  return learningParam != NULL;
}

MidiPairing* MidiService::pairingForParameterId(std::string paramId) {
  return &parameterIdToPairing[paramId];
}
MidiPairing* MidiService::pairingForDescriptor(std::string descriptor) {
  return &descriptorToPairing[descriptor];
}

void MidiService::saveConfigFile() {
  std::map<std::string, std::string> descriptorToParamId;
  
  for (auto const& x : descriptorToPairing)
  {
    const std::string paramId = x.second.paramId;
    descriptorToParamId[paramId] = x.second.descriptor;
  }
  
  std::ofstream fileStream;
  const char* homeDir = getenv("HOME");
  fileStream.open(formatString("%s/midi_config.json", homeDir), ios::trunc);
  if (fileStream.is_open()) {
    auto jsonContents = json(descriptorToParamId);
    cout << jsonContents << endl;
    fileStream << jsonContents;
    fileStream.close();
  } else {
    log("Problem saving midi_config.");
  }
}

void MidiService::loadConfigFile() {
  std::fstream fileStream;
  const char* homeDir = getenv("HOME");
  fileStream.open(formatString("%s/midi_config.json", homeDir), ios::in);
  if (fileStream.is_open()) {
    
    json data;
    fileStream >> data;
    std::map<std::string,std::string> paramIdToDescriptor = data;

    for (auto const& x : paramIdToDescriptor) {
      auto paramId = x.first;
      Parameter *param = ParameterService::getService()->parameterForId(paramId);
      if (param) {
        ofxMidiMessage msg = messageFrom(x.second);
        saveAssignment(param, msg);
      }
    }

    log("Loaded MIDI config.");
  }
}
  



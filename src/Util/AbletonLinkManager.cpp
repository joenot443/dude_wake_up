#include "AbletonLinkManager.hpp"

AbletonLinkManager::AbletonLinkManager() : link(120.0) { // Default BPM, adjust as needed
  // Constructor logic, if any
}

AbletonLinkManager::~AbletonLinkManager() {
  // Destructor logic, if any
}

void AbletonLinkManager::setup() {
  // Initialization logic for Ableton Link, if any
}

void AbletonLinkManager::update() {
  // Logic to be called each frame to update Link state, if necessary
  // For example, you might want to capture session state here if it's not done on demand
}

double AbletonLinkManager::getTempo() {
  // Ensure link is updated if necessary before getting tempo
  // This might depend on how ableton::Link is designed to be used
  return link.captureAppSessionState().tempo();
}

double AbletonLinkManager::getBeat() {
  // Ensure link is updated if necessary
  // The 4.0 here is numBeats, adjust if your Link setup uses a different time signature
  double beat = link.captureAppSessionState().beatAtTime(link.clock().micros(), 4.0);
  // Return only the fractional component of the beat, or the full beat,
  // depending on what you need. The original code used the fractional part.
  return beat - floor(beat);
}

// Implement any other methods you added to the header
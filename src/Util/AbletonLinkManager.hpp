#ifndef AbletonLinkManager_hpp
#define AbletonLinkManager_hpp

#include "Link.hpp" // Assuming Link.hpp is the correct header for ableton::Link
#include <stdio.h>

class AbletonLinkManager {
public:
  AbletonLinkManager();
  ~AbletonLinkManager();

  void setup();
  void update();

  double getTempo();
  double getBeat();
  // Add any other methods you need to interact with Ableton Link

private:
  ableton::Link link;
  // Add any other private members or helper functions
};

#endif /* AbletonLinkManager_hpp */

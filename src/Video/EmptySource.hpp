//
//  EmptySource.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/16/23.
//

#ifndef EmptySource_hpp
#define EmptySource_hpp

#include <stdio.h>

#include <stdio.h>
#include "ofMain.h"
#include "UUID.hpp"
#include "VideoSource.hpp"

using json = nlohmann::json;

class EmptySource : public VideoSource {
  
public:
  EmptySource() : VideoSource(UUID::generateUUID(), "Empty", VideoSource_empty) {};
  void setup() override;
  void saveFrame() override;
  json serialize() override;
  void load(json j) override;

private:
  ofFbo fbo;
};


#endif /* EmptySource_hpp */

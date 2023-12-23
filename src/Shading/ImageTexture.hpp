//
//  ImageTexture.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 12/21/23.
//

#ifndef ImageTexture_hpp
#define ImageTexture_hpp

#include <stdio.h>
#include <ofMain.h>

class ImageTexture {
public:
  std::shared_ptr<ofFbo> fbo;
  ImageTexture(std::string name) : name(name), fbo(std::make_shared<ofFbo>()) {
    setup();
  };
  
private:
  std::string name;
  void setup();
};

#endif /* ImageTexture_hpp */

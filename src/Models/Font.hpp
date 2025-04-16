//
//  Font.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 4/14/25.
//

#ifndef Font_h
#define Font_h

#include <string>

struct Font {
  std::string name = "";
  
  std::string path = "";
  
  Font(std::string name, std::string path) : name(name), path(path) {};
};

#endif /* Font_h */

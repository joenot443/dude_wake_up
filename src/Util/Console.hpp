//
//  Console.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-22.
//

#ifndef Console_h
#define Console_h
#include <stdio.h>
#include <Strings.hpp>

template<typename ... Args>
void log(std::string string, Args ... args) {
  std::cout << formatString(string, args ...) << std::endl;
}

#endif /* Console_h */

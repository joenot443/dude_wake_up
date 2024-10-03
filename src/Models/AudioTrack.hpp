//
//  AudioTrack.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 9/23/24.
//

#ifndef AudioTrack_h
#define AudioTrack_h

struct AudioTrack
{
  std::string name;
  std::string path;
  
  AudioTrack(std::string name, std::string path) : name(name), path(path) {};
};

#endif /* AudioTrack_h */

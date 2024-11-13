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
  int bpm;
  
  AudioTrack(std::string name, std::string path, int bpm) : name(name), path(path), bpm(bpm) {};
};

#endif /* AudioTrack_h */

//
//  Video.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-07-22.
//

#ifndef Video_h
#define Video_h
#include <stdio.h>
#include "ofVideoPlayer.h"
#include "Strings.hpp"

//std::string videoPlaybackString(ofVideoPlayer *player) {
//  float duration = player->getDuration();
//  float position = player->getPosition();
//  auto positionDurationString = durationToString(position * duration);
//  auto endDurationString = durationToString(duration);
//  
//  return formatString("%s - %s", positionDurationString, endDurationString);
//}

static float frameTime() {
  static float t = 0.0;
  t += ImGui::GetIO().DeltaTime;
  return t;
}


#endif /* Video_h */

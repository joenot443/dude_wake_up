// 
//  FileSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#include "FileSource.hpp"

void FileSource::setup() {
  player.load(path);
  player.play();
}

void FileSource::update() {
  player.update();
  if (player.isFrameNew()) {
//    frame.begin();
    ofClear(0, 0, 0, 0);
    player.draw(0, 0);
//    frame.end();
  }
}



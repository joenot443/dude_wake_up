#ifndef PlaylistSettings_hpp
#define PlaylistSettings_hpp

#include "VideoSourceSettings.hpp"
#include "Parameter.hpp"
#include "ofMain.h"
#include <vector>

class PlaylistSettings : public VideoSourceSettings {
public:
  PlaylistSettings(std::string id, float width = 0)
    : VideoSourceSettings(id, width),
      folderPath("") {
  }
  
  std::string folderPath; 
  std::vector<ofFile> videoFiles;
  int selectedVideoIndex = 0;
};

#endif /* PlaylistSettings_hpp */

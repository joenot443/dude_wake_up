//
//  NewVideoSourceView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/9/22.
//

#ifndef NewVideoSourceView_hpp
#define NewVideoSourceView_hpp

#include <stdio.h>

#include "VideoSource.hpp"
#include "FileBrowserView.hpp"

struct NewVideoSourceViewSettings {
  VideoSourceType selectedType = VideoSource_webcam;
  int selectedWebcamIndex = 0;
  std::string selectedWebcamName;
};

struct NewVideoSourceView {
public:
  void draw();
  NewVideoSourceViewSettings settings;
  FileBrowserView fileBrowserView = FileBrowserView(std::make_shared<FileBrowserSettings>());
};



#endif /* NewVideoSourceView_hpp */

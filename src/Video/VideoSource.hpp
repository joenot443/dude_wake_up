//
//  VideoSource.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#ifndef VideoSource_hpp
#define VideoSource_hpp

#include <stdio.h>
#include "VideoSettings.hpp"
#include "FeedbackSource.hpp"

enum VideoSourceType { VideoSource_webcam, VideoSource_file, VideoSource_chainer };

class VideoSource {
  
public:
  std::string id;
  std::string sourceName;

  VideoSourceType type;
  std::shared_ptr<ofTexture> frameTexture;
  std::shared_ptr<ofTexture> previewTexture;
  ofBufferObject frameBuffer;
  ofBufferObject previewBuffer;
  std::shared_ptr<FeedbackSource> feedbackDestination;

  VideoSource(std::string id, std::string name, VideoSourceType type) :
  id(id), sourceName(name), type(type) {};

  virtual void setup() {};
  virtual void saveFrame() {};
  virtual void drawSettings() {}; 
  
  void saveFeedbackFrame() {
    if (frameTexture != nullptr) {
      feedbackDestination->pushFrame(frameTexture);
    }
  };
};

#endif /* VideoSourceSource_hpp */

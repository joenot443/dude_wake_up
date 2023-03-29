//
//  VideoSource.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#ifndef VideoSource_hpp
#define VideoSource_hpp

#include "FeedbackSource.hpp"
#include "Settings.hpp"
#include "VideoSourceSettings.hpp"
#include "json.hpp"
#include <stdio.h>

using json = nlohmann::json;

enum VideoSourceType {
  VideoSource_webcam,
  VideoSource_file,
  VideoSource_chainer,
  VideoSource_image,
  VideoSource_shader,
  VideoSource_text
};


class VideoSource {

public:
  std::string id;
  std::string sourceName;
  
  VideoSourceSettings settings;
  VideoSourceType type;
  std::shared_ptr<ofTexture> frameTexture;
  ofBufferObject frameBuffer;
  std::shared_ptr<FeedbackSource> feedbackDestination;

  VideoSource(std::string id, std::string name, VideoSourceType type)
      : id(id), sourceName(name), type(type), settings(id, 0) {};

  virtual void setup(){};
  void update() {
    if (frameTexture->getWidth() != settings.width->value) {
      setup();
    }
  }
  virtual void saveFrame(){};
  virtual void drawSettings(){};
  virtual json serialize(){};
  virtual void load(json j){};

  void saveFeedbackFrame() {
    if (frameTexture != nullptr) {
      feedbackDestination->pushFrame(frameTexture);
    }
  };
};

#endif /* VideoSourceSource_hpp */

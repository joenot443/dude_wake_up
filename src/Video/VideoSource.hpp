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
#include "UUID.hpp"
#include "VideoSourceSettings.hpp"
#include "json.hpp"
#include "Connection.hpp"
#include <stdio.h>

using json = nlohmann::json;

enum VideoSourceType {
  VideoSource_webcam,
  VideoSource_file,
  VideoSource_chainer,
  VideoSource_image,
  VideoSource_icon,
  VideoSource_shader,
  VideoSource_text,
  VideoSource_library,
  VideoSource_empty
};


class VideoSource: public Connectable {

public:
  std::string id;
  std::string sourceName;
  
  std::shared_ptr<VideoSourceSettings> settings;
  VideoSourceType type;
  std::shared_ptr<ofFbo> fbo;
  std::shared_ptr<FeedbackSource> feedbackDestination;
  
  // Layout
  ImVec2 origin;
  
  // Dummy Source
  VideoSource() : id(UUID::generateUUID()), sourceName(UUID::generateUUID()), settings(std::make_shared<VideoSourceSettings>(id, 0)), origin(ImVec2(0.,0.)) {};

  VideoSource(std::string id, std::string name, VideoSourceType type)
      : id(id), fbo(std::make_shared<ofFbo>()), sourceName(name), type(type), settings(std::make_shared<VideoSourceSettings>(id, 0)), origin(ImVec2(0.,0.)) {};

  virtual void setup(){};
  void update() {
    if (fbo->getTexture().getWidth() != settings->width->value) {
      setup();
    }
  }
  virtual void saveFrame(){};
  virtual void drawSettings(){};
  virtual json serialize(){ return 0; };
  virtual void load(json j){};
  
  virtual void teardown(){};
  
  std::string name() {
    return sourceName;
  }
  
  // Connectable
  
  std::shared_ptr<VideoSourceSettings> sourceSettings() {
    return settings;
  }
  
  std::shared_ptr<ofFbo> frame() {
    return fbo;
  }
  
  std::string connId() {
    return id;
  }
  
  ConnectableType connectableType() {
    return ConnectableTypeSource;
  }
  
  void drawPreview(ImVec2 pos, float scale) {
    if (!fbo->isAllocated()) return;
    
    ofTexture tex = fbo->getTexture();
    if (tex.isAllocated()) {
      ImTextureID texID = (ImTextureID)(uintptr_t)tex.getTextureData().textureID;
      ImGui::Image(texID, ImVec2(160/scale, 90/scale));
    }
  }

  void saveFeedbackFrame() {
    if (fbo != nullptr) {
      feedbackDestination->pushFrame(fbo);
    }
  };
  
  void drawMaskSettings();
};

#endif /* VideoSourceSource_hpp */

//
//  VideoSource.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#ifndef VideoSource_hpp
#define VideoSource_hpp

#include "FeedbackSource.hpp"
#include "OptionalShadersHelper.hpp"
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
  VideoSource_image,
  VideoSource_icon,
  VideoSource_shader,
  VideoSource_text,
  VideoSource_library,
  VideoSource_multi,
  VideoSource_empty
};


class VideoSource: public Connectable {

public:
  std::string id;
  std::string sourceName;
  
  std::shared_ptr<VideoSourceSettings> settings;
  VideoSourceType type;
  std::shared_ptr<ofFbo> fbo;
  std::shared_ptr<ofFbo> optionalFbo;
  std::shared_ptr<FeedbackSource> feedbackDestination;
  
  // Layout
  ImVec2 origin;
  
  // Dummy Source
  VideoSource() : id(UUID::generateUUID()), sourceName(UUID::generateUUID()), settings(std::make_shared<VideoSourceSettings>(id, 0)), origin(ImVec2(0.,0.)) {};

  VideoSource(std::string id, std::string name, VideoSourceType type)
      : id(id), fbo(std::make_shared<ofFbo>()), optionalFbo(std::make_shared<ofFbo>()), sourceName(name), type(type), settings(std::make_shared<VideoSourceSettings>(id, 0)), origin(ImVec2(0.,0.)) {};

  virtual void setup(){};
  void update() {
    if (fbo->getTexture().getWidth() != settings->width->value) {
      setup();
    }
  }
  virtual void saveFrame(){};
  void drawSettings() override {};
  
  void drawOptionalSettings() override;
  
  std::shared_ptr<Settings> settingsRef() override;
  
  void applyOptionalShaders();
  
  void generateOptionalShaders();
  
  virtual json serialize() { return 0; };
  int inputCount() override { return 0; };
  
  virtual void load(json j){};
  
  virtual void teardown(){};
  
  std::string name() override  {
    return sourceName;
  }
  
  // Connectable
  
  std::shared_ptr<VideoSourceSettings> sourceSettings() override {
    return settings;
  }
  
  std::shared_ptr<ofFbo> frame() override {
    return fbo;
  }
  
  std::string connId() override {
    return id;
  }
  
  ConnectableType connectableType() override {
    return ConnectableTypeSource;
  }
  
  std::shared_ptr<ofFbo> parentFrame() override {
    return frame();
  }
  
  void drawPreview(ImVec2 pos, float scale);
  
  virtual void drawPreviewSized(ImVec2 size);

  void saveFeedbackFrame() {
    if (fbo != nullptr) {
      feedbackDestination->pushFrame(fbo);
    }
  };

  OptionalShadersHelper optionalShadersHelper;
};

#endif /* VideoSourceSource_hpp */

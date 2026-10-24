//
//  WebcamSource.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#ifndef WebcamSource_hpp
#define WebcamSource_hpp

#include <stdio.h>
#include "ofMain.h"
#include "VideoSource.hpp"
#include "SourceTransform.hpp"

using json = nlohmann::json;

class WebcamVideoSourceSettings : public VideoSourceSettings {
public:
  std::shared_ptr<Parameter> deviceId;
  /// Crop + place pipeline shared with TransformShader. Adds Min X / Max X /
  /// Min Y / Max Y / X / Y / Scale / Mode / Center / Scale From Center /
  /// Vertical Flip / Horizontal Flip / Background Enabled / Background Color.
  SourceTransform transform;

  WebcamVideoSourceSettings(std::string sourceId, json j)
  : VideoSourceSettings(sourceId, j),
    deviceId(std::make_shared<Parameter>("Device", 0.0, 0.0, 100.0, ParameterType_Int)),
    transform(sourceId)
  {
    parameters.push_back(deviceId);
    transform.appendParameters(parameters);
    transform.appendOscillators(oscillators);
    registerParameters();
  }
};

class WebcamSource : public VideoSource {

public:
  WebcamSource(std::string id, std::string name)
  : VideoSource(id, name, VideoSource_webcam),
    settings(std::make_shared<WebcamVideoSourceSettings>(id, 0)) {};
  void setup() override;
  // Releases the camera (a deleted node lives on in the undo history; undo
  // re-adds it, which runs setup() again).
  void teardown() override;
  void saveFrame() override;
  json serialize() override;
  void load(json j) override;
  ofShader maskShader;
  void drawSettings() override;
  std::shared_ptr<Settings> settingsRef() override {
    return std::dynamic_pointer_cast<Settings>(settings);
  }
  std::shared_ptr<WebcamVideoSourceSettings> settings;
  std::vector<std::string> deviceNames;

  /// Raw camera capture, pre-transform. Exposed via IOSurface under
  /// "sourceRaw:<id>" so the SwiftUI Crop pad can render the original
  /// (uncropped, unscaled) feed as its backdrop.
  std::shared_ptr<ofFbo> rawFbo;

private:
  ofVideoGrabber grabber;
  bool grabberReady = false;
  int lastDeviceId = -1;
  LIFECYCLE_TRACK("WebcamSource");
};

#endif

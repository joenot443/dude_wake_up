//
//  AvailableStrand.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/5/23.
//

#ifndef AvailableStrand_h
#define AvailableStrand_h
#include "json.hpp"
#include "UUID.hpp"
#include "ofMain.h"
#include "JSONSerializable.hpp"
using json = nlohmann::json;

struct AvailableStrand : public JSONSerializable
{
  std::string name;
  std::string path;
  std::string imagePath;
  std::string id;

  // Preview
  std::shared_ptr<ofFbo> fbo;

  AvailableStrand(std::string name, std::string path, std::string imagePath,
                  std::string id = UUID::generateUUID())
      : name(name), imagePath(imagePath), path(path), id(id), fbo(std::make_shared<ofFbo>()){
        generatePreview();
      };

  json serialize()  { return json{{"name", name}, {"path", path}, {"id", id}, {"imagePath", imagePath}}; }
  void load(json j)  {
    path = j["path"];
    name = j["name"];
    id = j["id"];
  }

  void generatePreview() {
    // Load the image from the imagePath and draw it to the fbo
    ofImage image;
    image.load(imagePath);
    fbo->allocate(320, 240);
    fbo->begin();
    if (image.getWidth() > 0 && image.getHeight() > 0) {
      image.draw(0, 0, 320, 240);
    }
    ofSetColor(0, 0, 0, 128);
    ofDrawRectangle(0, 0, 320, 240);
    fbo->end();
  }
};

#endif /* AvailableStrand_h */

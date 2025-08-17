//
//  ImageSource.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/1/23.
//

#ifndef ImageSource_hpp
#define ImageSource_hpp

#include "VideoSource.hpp"

using json = nlohmann::json;

class ImageSource : public VideoSource {
  
public:
  ImageSource(std::string id, std::string name, std::string path) 
  : VideoSource(id, name, VideoSource_image), 
    path(path),
    mode(std::make_shared<Parameter>("Mode", 0, ParameterType_Int)),
    scale(std::make_shared<Parameter>("Scale", 1.0, 0.0, 5.0)),
    translateX(std::make_shared<Parameter>("X", 0.0, 0.0, 1.0)),
    translateY(std::make_shared<Parameter>("Y", 0.0, 0.0, 1.0)),
    center(std::make_shared<Parameter>("Center", ParameterType_Bool)),
    verticalFlip(std::make_shared<Parameter>("Vertical Flip", 0.0, 0.0, 1.0, ParameterType_Bool)),
    horizontalFlip(std::make_shared<Parameter>("Horizontal Flip", 0.0, 0.0, 1.0, ParameterType_Bool)) {
      mode->options = {"Standard", "Fill", "Fit", "Fill + Aspect"};
    };

  void setup() override;
  void saveFrame() override;
  void drawSettings() override;
  json serialize() override;
  void load(json j) override;
  
  std::string path;
  ofImage image;
  
  // Transform parameters
  std::shared_ptr<Parameter> mode;
  std::shared_ptr<Parameter> scale;
  std::shared_ptr<Parameter> translateX;
  std::shared_ptr<Parameter> translateY;
  std::shared_ptr<Parameter> center;
  std::shared_ptr<Parameter> verticalFlip;
  std::shared_ptr<Parameter> horizontalFlip;
};

#endif /* ImageSource_hpp */

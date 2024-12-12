//
//  ImageService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/14/24.
//

#ifndef ImageService_hpp
#define ImageService_hpp

#include <map>
#include <memory>
#include "Image.hpp"

class ImageService {
private:
  std::map<std::string, std::shared_ptr<Image>> images;
  bool populated = false;

public:
  void setup();
  void populateImages();
  std::vector<std::shared_ptr<Image>> availableImages();
  std::vector<std::string> availableImageNames();
  std::shared_ptr<Image> imageWithName(std::string name);

  static ImageService *service;
  ImageService() {};

  static ImageService *getService() {
    if (!service) {
      service = new ImageService;
    }
    return service;
  }
};

#endif /* ImageService_hpp */ 
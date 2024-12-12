#include "ImageService.hpp"
#include "ofMain.h"
#include "ofxImGui.h"

void ImageService::setup() {
  populateImages();
}

void ImageService::populateImages() {
  ofDirectory dir("images/icons");
  dir.listDir();
  for (int i = 0; i < dir.size(); i++) {
    std::string path = dir.getPath(i);
    std::string name = dir.getName(i);
    images[name] = std::make_shared<Image>(name, path);
  }
  populated = true;
}

std::vector<std::shared_ptr<Image>> ImageService::availableImages() {
  if (!populated) populateImages();

  std::vector<std::shared_ptr<Image>> imageList;
  for (const auto& pair : images) {
    imageList.push_back(pair.second);
  }
  return imageList;
}

std::vector<std::string> ImageService::availableImageNames() {
  if (!populated) populateImages();

  std::vector<std::string> names;
  for (const auto& pair : images) {
    names.push_back(pair.first);
  }
  return names;
}

std::shared_ptr<Image> ImageService::imageWithName(std::string name) {
  if (images.find(name) != images.end()) {
    return images[name];
  }
  return nullptr;
}

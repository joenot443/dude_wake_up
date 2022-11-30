//
//  FileBrowserView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/8/22.
//

#ifndef FileBrowserView_hpp
#define FileBrowserView_hpp
#include "Parameter.hpp"

#include <stdio.h>
#include <string>
#include <vector>

struct File {
  std::string name;
  std::string path;
  bool isDirectory;

  File(std::string path, bool isDirectory) {
    this->path = path;
    this->name = path.substr(path.find_last_of("/\\") + 1);
    this->isDirectory = isDirectory;
  };
};

struct FileBrowserSettings {
  std::vector<File> files = {};
  std::shared_ptr<File> selectedFile = nullptr;

  std::string currentDirectory = "/";
  
  void refresh();
};

struct FileBrowserView {
public:
  void setup();
  void update();
  void draw();
  void teardown();
  
  std::shared_ptr<FileBrowserSettings> settings;
  
  FileBrowserView(std::shared_ptr<FileBrowserSettings> settings) {
    this->settings = settings;
  }
};

#endif /* FileBrowserView_hpp */

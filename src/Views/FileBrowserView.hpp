//
//  FileBrowserView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/8/22.
//

#ifndef FileBrowserView_hpp
#define FileBrowserView_hpp
#include "Parameter.hpp"
#include "TileBrowserView.hpp"
#include "ListBrowserView.hpp"
#include "AvailableVideoSource.hpp"
#include "ofMain.h"

#include <stdio.h>
#include <string>
#include <vector>

enum FileBrowserType { FileBrowserType_JSON, FileBrowserType_Source };

struct File {
  std::string name;
  std::string path;
  bool isDirectory;
  ofTexture previewTexture;

  File(std::string path, bool isDirectory) {
    this->path = path;
    this->name = path.substr(path.find_last_of("/\\") + 1);
    this->isDirectory = isDirectory;
  };
};

struct FileBrowserView {
  std::vector<File> files = {};
  std::vector<std::shared_ptr<AvailableVideoSource>> sources;
  std::shared_ptr<File> selectedFile = nullptr;
  ofDirectory currentDirectory;
  FileBrowserType type = FileBrowserType_Source;
  
public:
  void setup();
  void update();
  void draw();
  void refresh();
  void teardown();
  
  FileBrowserView(FileBrowserType type) : type(type) {};

  ListBrowserView listBrowserView = ListBrowserView({});
};

#endif /* FileBrowserView_hpp */

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
#include <queue>

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
  std::shared_ptr<ofDirectory> currentDirectory;

  ofVideoPlayer videoPlayer;
  std::queue<std::pair<std::shared_ptr<AvailableVideoSourceFile>, std::shared_ptr<TileItem>>> previewQueue;
  
public:
  void setup();
  void update();
  void loadDirectory(std::string directory);
  void draw();
  void refresh();
  void teardown();
  
  FileBrowserView() {};

  ListBrowserView listBrowserView = ListBrowserView({});
  TileBrowserView tileBrowserView = TileBrowserView({});
};

#endif /* FileBrowserView_hpp */

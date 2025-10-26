//
//  LibraryFile.h
//  dude_wake_up
//
//  Created by Joe Crozier on 1/20/23.
//

#ifndef LibraryFile_h
#define LibraryFile_h
#include <stdio.h>
#include <string>
#include "json.hpp"
#include "ofMain.h"
#include "TileBrowserView.hpp"

using json = nlohmann::json;

struct LibraryFile
{
  std::string id;
  std::string name;
  std::string filename;
  std::string url;
  std::string thumbnailUrl;
  std::string thumbnailFilename;
  std::string category;
  bool isDownloading = false;
  bool isPaused = false;
  float progress = 0.0;
  uint64_t downloadedBytes = 0;
  uint64_t totalBytes = 0;
  
  std::string thumbnailPath();
  
  std::string videoPath();
  
  json serialize() {
    json j;
    j["id"] = id;
    j["name"] = name;
    j["filename"] = filename;
    j["url"] = url;
    j["thumbnailUrl"] = thumbnailUrl;
    j["thumbnailFilename"] = thumbnailFilename;
    j["category"] = category;
    return j;
  }
  
  void load(json j) {
    id = j["id"];
    name = j["name"];
    filename = j["filename"];
    url = j["url"];
    thumbnailUrl = j["thumbnailUrl"];
    thumbnailFilename = j["thumbnailFilename"];
    category = j["category"];
  }
};

class LibraryTileItem : public TileItem {
public:
  LibraryTileItem(std::string name, ImTextureID textureID, int index, std::shared_ptr<LibraryFile> libraryFile, std::function<void(std::string)> dragCallback, std::string category) :
  TileItem(name, textureID, index, dragCallback, category, TileType_Library), libraryFile(libraryFile) {};
  
  std::shared_ptr<LibraryFile> libraryFile;
};

#endif /* LibraryFile_h */

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
#include "ofMain.h"
#include "TileBrowserView.hpp"

struct LibraryFile
{
  std::string id;
  std::string name;
  std::string filename;
  std::string url;
  std::string thumbnailUrl;
  std::string thumbnailFilename;
  bool isDownloading = false;
  bool mediaDownloaded = false;
  bool thumbnailDownloaded = false;
  float progress = 0.0;
  
  std::string thumbnailPath();
  
  std::string videoPath();
  
  bool isMediaDownloaded() {
    return ofFile::doesFileExist(videoPath());
  }
  
  bool isThumbnailDownloaded() {
    return ofFile::doesFileExist(thumbnailPath());
  }
};

class LibraryTileItem : public TileItem {
public:
  LibraryTileItem(std::string name, ImTextureID textureID, int index, std::shared_ptr<LibraryFile> libraryFile, std::function<void()> dragCallback) :
  TileItem(name, textureID, index, dragCallback), libraryFile(libraryFile) {};
  
  std::shared_ptr<LibraryFile> libraryFile;
  
};

#endif /* LibraryFile_h */

//
//  LibraryFile.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 7/12/23.
//

#include <stdio.h>
#include "LibraryFile.hpp"
#include "ConfigService.hpp"
#include <string>

std::string LibraryFile::thumbnailPath() {
  return ofFilePath::join(ConfigService::getService()->libraryFolderFilePath(), thumbnailFilename);
}

std::string LibraryFile::videoPath() {
  return ofFilePath::join(ConfigService::getService()->libraryFolderFilePath(), filename);
}

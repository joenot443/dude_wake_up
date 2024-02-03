//
//  File.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/2/24.
//

#ifndef File_h
#define File_h

#include "ofMain.h"

static bool isImageFile(std::string path)
{
  std::string extension = ofFilePath::getFileExt(path);
  return extension == "png" || extension == "jpg" || extension == "jpeg" ||
         extension == "gif" || extension == "tiff" || extension == "bmp";
}

static bool isVideoFile(std::string path)
{
  std::string extension = ofFilePath::getFileExt(path);
  return extension == "mov" || extension == "mp4" || extension == "avi" ||
         extension == "mkv" || extension == "flv" || extension == "webm";
}

#endif /* File_h */

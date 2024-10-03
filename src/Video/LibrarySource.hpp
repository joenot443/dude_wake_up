//
//  LibrarySource.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 9/20/24.
//

#ifndef LibrarySource_hpp
#define LibrarySource_hpp

#include <stdio.h>
#include "LibraryFile.hpp"
#include "FileSource.hpp"

enum LibrarySourceState {
  LibrarySourceState_Waiting,
  LibrarySourceState_Downloading,
	LibrarySourceState_Completed,
	LibrarySourceState_Failed
};

class LibrarySource : public FileSource
{
public:
  
  std::shared_ptr<LibraryFile> libraryFile;
  LibrarySourceState state;
  LibrarySource(std::string id, std::shared_ptr<LibraryFile> libraryFile) :
	
  FileSource(id, libraryFile->name, libraryFile->videoPath(), VideoSource_library),
  state(LibrarySourceState_Waiting),
  libraryFile(libraryFile) {};
  
  void setup() override;
  void drawSettings() override;
  json serialize() override;
  void load(json j) override;
  
  void runSetupOnMainThread(ofEventArgs & args);
};


  

#endif /* LibrarySource_hpp */

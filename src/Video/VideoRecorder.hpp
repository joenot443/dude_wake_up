//
//  VideoRecorder.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 7/26/23.
//

#ifndef VideoRecorder_hpp
#define VideoRecorder_hpp

#include <string>

struct VideoRecordingSettings {
  std::string filePath;
  int outputWindow = 0;
  VideoRecordingSettings();
};

struct VideoRecorder
{
public:
  void setup();
  void startRecording();
  void stopRecording();
  void save();
  
  bool isRecording();

};

#endif /* VideoRecorder_hpp */

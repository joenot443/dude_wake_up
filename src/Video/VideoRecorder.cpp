//
//  VideoRecorder.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 7/26/23.
//

#include "VideoRecorder.hpp"
#include "ConfigService.hpp"

void VideoRecorder::setup() {
//  recorder.setVideoCodec("mpeg4");

//  recorder.setFfmpegLocation(ofFilePath::getAbsolutePath("ffmpeg"));
}

void VideoRecorder::startRecording() {
//  recorder.setup(<#string fname#>, <#int w#>, <#int h#>, <#float fps#>)
}

void VideoRecorder::stopRecording() {
  
}


bool VideoRecorder::isRecording() {
  return false;
}

VideoRecordingSettings::VideoRecordingSettings() : filePath(ConfigService::getService()->libraryFolderFilePath() + "output.mp4") {};
		

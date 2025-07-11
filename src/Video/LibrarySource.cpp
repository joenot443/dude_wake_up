//
//  LibrarySource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 9/20/24.
//

#include "LibrarySource.hpp"
#include "LibraryService.hpp"
#include "LayoutStateService.hpp"
#include "FontService.hpp"
#include "NodeLayoutView.hpp"
#include "ofMain.h" // For main thread utilities

void LibrarySource::load(json j)
{
  if (!j.is_object())
  {
    log("Error hydrating LibrarySource from json");
    return;
  }
  
  path = j["path"];
  id = j["id"];
  sourceName = j["sourceName"];
  mute->boolValue = j["mute"];
  volume->value = j["volume"];
  settings->load(j["settings"]);
  boomerang->boolValue = j["boomerang"];
  libraryFile->load(j["libraryFile"]);
  start = settings->start->value;
  end = settings->end->value;
  applyRanges();
}

json LibrarySource::serialize()
{
  json j;
  j["path"] = path;
  j["id"] = id;
  j["sourceName"] = sourceName;
  j["videoSourceType"] = VideoSource_library;
  j["mute"] = mute->boolValue;
  j["volume"] = volume->value;
  j["boomerang"] = boomerang->boolValue;
  j["settings"] = settings->serialize();
  j["libraryFile"] = libraryFile->serialize();
  j["libraryFileId"] = libraryFile->id;
  auto node = NodeLayoutView::getInstance()->nodeForShaderSourceId(id);
  if (node != nullptr)
  {
    j["x"] = node->position.x;
    j["y"] = node->position.y;
  }
  return j;
}

void LibrarySource::setup() {
  fbo->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y);
  
  
  // Check if the libraryFileId has been downloaded already
  if (LibraryService::getService()->libraryFileIdDownloadedMap[libraryFile->id]) {
    // Run FileSource setup normally
    FileSource::setup();
  }
  
  else {
    // Define the completion callback to be executed after download
    auto onComplete = [this]() {
      log("Download complete, setting up FileSource");
      this->state = LibrarySourceState_Completed;
      ofAddListener(ofEvents().update, this, &LibrarySource::runSetupOnMainThread);
    };
    state = LibrarySourceState_Downloading;
    
    // Pass the callback to downloadFile
    LibraryService::getService()->downloadFutures.push_back(
                                                            std::async(std::launch::async, &LibraryService::downloadFile, LibraryService::getService(), libraryFile, onComplete));
  }
}

void LibrarySource::drawSettings()
{
  if (LibraryService::getService()->libraryFileIdDownloadedMap[libraryFile->id]) {
    FileSource::drawSettings();
    return;
  } else {
    float progress = libraryFile->progress * 100.0f;  // Multiply by 100 for percentage
    ImGui::PushStyleColor(ImGuiCol_SliderGrab, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
    ImGui::PushStyleColor(ImGuiCol_SliderGrabActive, ImVec4(0.2f, 0.6f, 1.0f, 1.0f));
    ImGui::SliderFloat("Downloading...", &progress, 0.0f, 100.0f, "%.1f%%", ImGuiSliderFlags_NoInput);
    ImGui::PopStyleColor(2);
  }
}

// Method to be called on the main thread
void LibrarySource::runSetupOnMainThread(ofEventArgs & args)
{
  // Remove the listener to avoid repeated calls
  ofRemoveListener(ofEvents().update, this, &LibrarySource::runSetupOnMainThread);
  
  // Now it's safe to call FileSource::setup on the main thread
  FileSource::setup();
}

void LibrarySource::teardown() {
  FileSource::teardown();
}

void LibrarySource::drawPreviewSized(ImVec2 size) {
  if (libraryFile->isDownloading) {
    ImGui::PushFont(FontService::getService()->current->h1);
    ImGui::Dummy(ImVec2(1.0, size.y / 2.0 - 10.0f));
    ImGui::Dummy(ImVec2(size.x / 2.0 - 10.0f, 1.0)); ImGui::SameLine();
    ImGui::Text("%.1f%%", libraryFile->progress * 100.0f);
    ImGui::Dummy(ImVec2(1.0, size.y / 2.0 - 10.0f));
    ImGui::PopFont();
    return;
  }
  
  FileSource::drawPreviewSized(size);
}



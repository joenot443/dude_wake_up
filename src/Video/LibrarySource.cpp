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

  // Check if the file actually exists on disk (more reliable than checking the map,
  // which may not be populated when resuming a strand before library fetch completes)
  if (LibraryService::getService()->hasMediaOnDisk(libraryFile)) {
    // File exists on disk - run FileSource setup normally
    state = LibrarySourceState_Completed;
    // Update the map so other checks work correctly
    LibraryService::getService()->libraryFileIdDownloadedMap[libraryFile->id] = true;
    // Ensure path is set correctly (may differ from saved path if library folder changed)
    path = libraryFile->videoPath();
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
  // Only show FileSource settings if player is loaded
  if (player.isLoaded()) {
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

  // Ensure path is set correctly before calling FileSource::setup
  path = libraryFile->videoPath();

  // Now it's safe to call FileSource::setup on the main thread
  FileSource::setup();
}

void LibrarySource::teardown() {
  FileSource::teardown();
}

void LibrarySource::drawPreviewSized(ImVec2 size) {
  // Show download UI if player isn't ready yet
  if (!player.isLoaded()) {
    ImDrawList* draw_list = ImGui::GetWindowDrawList();
    ImVec2 cursor_pos = ImGui::GetCursorScreenPos();

    // Draw background rect for the entire preview area
    draw_list->AddRectFilled(cursor_pos, ImVec2(cursor_pos.x + size.x, cursor_pos.y + size.y),
                             IM_COL32(30, 30, 30, 255), 8.0f);

    // Calculate centered position for progress UI
    float progressBarWidth = size.x * 0.7f;
    float progressBarHeight = 8.0f;
    float marginX = (size.x - progressBarWidth) / 2.0f;
    float centerY = size.y / 2.0f;

    // Progress bar background (dark)
    ImVec2 bg_min = ImVec2(cursor_pos.x + marginX, cursor_pos.y + centerY - progressBarHeight / 2.0f);
    ImVec2 bg_max = ImVec2(cursor_pos.x + marginX + progressBarWidth, cursor_pos.y + centerY + progressBarHeight / 2.0f);
    draw_list->AddRectFilled(bg_min, bg_max, IM_COL32(40, 40, 40, 200), 4.0f);

    // Progress bar fill (blue if downloading, gray if paused)
    float progress = libraryFile->progress;
    float fillWidth = progressBarWidth * progress;
    if (fillWidth > 0.0f) {
      ImU32 fillColor = libraryFile->isPaused ? IM_COL32(120, 120, 120, 220) : IM_COL32(50, 150, 255, 220);
      ImVec2 fill_max = ImVec2(cursor_pos.x + marginX + fillWidth, cursor_pos.y + centerY + progressBarHeight / 2.0f);
      draw_list->AddRectFilled(bg_min, fill_max, fillColor, 4.0f);
    }

    // Progress percentage text
    ImGui::PushFont(FontService::getService()->current->h2);
    char progressText[32];
    snprintf(progressText, sizeof(progressText), "%.1f%%", progress * 100.0f);
    ImVec2 textSize = ImGui::CalcTextSize(progressText);
    ImVec2 textPos = ImVec2(
      cursor_pos.x + (size.x - textSize.x) / 2.0f,
      cursor_pos.y + centerY - progressBarHeight / 2.0f - textSize.y - 10.0f
    );
    draw_list->AddText(textPos, IM_COL32(255, 255, 255, 255), progressText);
    ImGui::PopFont();

    // Status label and pause/resume button
    ImGui::PushFont(FontService::getService()->current->h3);
    const char* label = libraryFile->isPaused ? "Paused" : "Downloading...";
    ImVec2 labelSize = ImGui::CalcTextSize(label);

    // Calculate button size and position
    float buttonSize = 20.0f;
    float totalWidth = labelSize.x + buttonSize + 5.0f; // 5px gap between label and button
    float startX = cursor_pos.x + (size.x - totalWidth) / 2.0f;
    float labelY = cursor_pos.y + centerY + progressBarHeight / 2.0f + 10.0f;

    // Draw label
    ImVec2 labelPos = ImVec2(startX, labelY);
    ImU32 labelColor = libraryFile->isPaused ? IM_COL32(200, 200, 200, 255) : IM_COL32(180, 180, 180, 255);
    draw_list->AddText(labelPos, labelColor, label);

    // Draw pause/resume button
    ImVec2 buttonPos = ImVec2(startX + labelSize.x + 5.0f, labelY + (labelSize.y - buttonSize) / 2.0f);
    ImVec2 buttonMin = buttonPos;
    ImVec2 buttonMax = ImVec2(buttonPos.x + buttonSize, buttonPos.y + buttonSize);

    // Check if mouse is hovering over button
    ImVec2 mousePos = ImGui::GetMousePos();
    bool isHovered = mousePos.x >= buttonMin.x && mousePos.x <= buttonMax.x &&
                     mousePos.y >= buttonMin.y && mousePos.y <= buttonMax.y;

    // Button background (circle)
    ImVec2 buttonCenter = ImVec2((buttonMin.x + buttonMax.x) / 2.0f, (buttonMin.y + buttonMax.y) / 2.0f);
    ImU32 buttonBgColor = isHovered ? IM_COL32(70, 70, 70, 255) : IM_COL32(50, 50, 50, 200);
    draw_list->AddCircleFilled(buttonCenter, buttonSize / 2.0f, buttonBgColor);

    // Draw icon (pause = two bars, resume = triangle)
    ImU32 iconColor = IM_COL32(255, 255, 255, 255);
    if (libraryFile->isPaused) {
      // Play triangle
      float iconSize = buttonSize * 0.5f;
      ImVec2 p1 = ImVec2(buttonCenter.x - iconSize * 0.3f, buttonCenter.y - iconSize * 0.4f);
      ImVec2 p2 = ImVec2(buttonCenter.x - iconSize * 0.3f, buttonCenter.y + iconSize * 0.4f);
      ImVec2 p3 = ImVec2(buttonCenter.x + iconSize * 0.4f, buttonCenter.y);
      draw_list->AddTriangleFilled(p1, p2, p3, iconColor);
    } else {
      // Pause bars
      float barWidth = 2.5f;
      float barHeight = buttonSize * 0.5f;
      float barGap = 3.0f;
      ImVec2 bar1Min = ImVec2(buttonCenter.x - barGap / 2.0f - barWidth, buttonCenter.y - barHeight / 2.0f);
      ImVec2 bar1Max = ImVec2(buttonCenter.x - barGap / 2.0f, buttonCenter.y + barHeight / 2.0f);
      ImVec2 bar2Min = ImVec2(buttonCenter.x + barGap / 2.0f, buttonCenter.y - barHeight / 2.0f);
      ImVec2 bar2Max = ImVec2(buttonCenter.x + barGap / 2.0f + barWidth, buttonCenter.y + barHeight / 2.0f);
      draw_list->AddRectFilled(bar1Min, bar1Max, iconColor);
      draw_list->AddRectFilled(bar2Min, bar2Max, iconColor);
    }

    ImGui::PopFont();

    // Handle button click with invisible button positioned absolutely at button location
    ImGui::SetCursorScreenPos(buttonMin);
    ImGui::InvisibleButton("pauseResumeButton", ImVec2(buttonSize, buttonSize));
    if (ImGui::IsItemClicked()) {
      if (libraryFile->isPaused) {
        // Resume download
        auto onComplete = [this]() {
          log("Download complete after resume");
          this->state = LibrarySourceState_Completed;
          ofAddListener(ofEvents().update, this, &LibrarySource::runSetupOnMainThread);
        };
        LibraryService::getService()->resumeDownload(libraryFile, onComplete);
      } else {
        // Pause download
        LibraryService::getService()->pauseDownload(libraryFile);
      }
    }
    ImGui::SetCursorPos(cursor_pos);
    return;
  }

  FileSource::drawPreviewSized(size);
}



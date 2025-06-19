#ifndef FeedbackView_hpp
#define FeedbackView_hpp

//  FeedbackView.hpp
//  dude_wake_up
//
//  Created by ChatGPT on behalf of Joe Crozier.
//
//  An ImGui powered view for collecting user feedback. This component provides
//  a simple UI allowing the user to enter a comment, optional author and email
//  fields, then captures a screenshot and sends everything – together with a
//  JSON-encoded representation of the current application state – to the
//  backend endpoint /api/feedback/new.
//
//  The networking layer uses the same cpp-httplib SSL client already utilised
//  by LibraryService so that we keep dependencies consistent.
//
//  NOTE: This class is purposely implemented inline in the header to avoid the
//  need for a dedicated compilation unit.
//

#include "ofMain.h"
#include "json.hpp"
#include "ConfigService.hpp"
#include <fstream>
#include "imgui.h"
#include <cstring>
#include "LibraryService.hpp"
#include <functional>
#include <future>
#include "CommonViews.hpp"

struct FeedbackView {
private:
  char textBuf[2048] = {0};
  char authorBuf[256] = {0};
  char emailBuf[256] = {0};

  std::string text() const { return std::string(textBuf); }
  std::string author() const { return std::string(authorBuf); }
  std::string email() const { return std::string(emailBuf); }

  bool sending = false;
  bool success = false;
  std::string error;
  bool attachScreenshot = true;

  // Captures screenshot and encodes it into PNG, returning the binary buffer.
  std::string captureScreenshotData()
  {
    int w = ofGetWidth();
    int h = ofGetHeight();
    ofPixels pixels;
    pixels.allocate(w, h, OF_IMAGE_COLOR);
    // Read from front buffer – ensures we capture what's on screen.
    glReadBuffer(GL_FRONT);
    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glReadPixels(0, 0, w, h, GL_RGB, GL_UNSIGNED_BYTE, pixels.getData());
    // Flip vertically because OpenGL origin is bottom left.
    ofPixels flipped;
    flipped.allocate(w, h, OF_IMAGE_COLOR);
    for (int y = 0; y < h; ++y) {
      memcpy(flipped.getPixels() + y * w * 3, pixels.getPixels() + (h - 1 - y) * w * 3, w * 3);
    }
    ofBuffer buffer;
    if (ofSaveImage(flipped, buffer, OF_IMAGE_FORMAT_PNG)) {
      return std::string(buffer.getData(), buffer.size());
    }
    return std::string();
  }

public:
  void draw(bool *open = nullptr)
  {
    if (open)
    {
      if (!*open)
      {
        return; // Early out if parent closed the window
      }
    }

    // Window sizing and style
    ImGui::SetNextWindowSize(ImVec2(460, 0), ImGuiCond_FirstUseEver);
    ImGui::Begin("Send Feedback", open, ImGuiWindowFlags_AlwaysAutoResize);

    CommonViews::H2Title("I love feedback", false);
    ImGui::TextWrapped("This is the best way for me to make Nottawa better.\nIf anything is broken, ugly, inconsistent, or unclear, I really want hear it. If you're confused about what to do or how the app works, let me know.\nIf your work was used in nottawa and wasn't properly attributed or you'd like it taken down, you can let me know here or reach out directly at joe@nottawa.app.");

    if (success)
    {
      ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(0, 200, 0, 255));
      ImGui::TextWrapped("Thank you! Feedback sent successfully.");
      ImGui::PopStyleColor();
      if (ImGui::Button("Close"))
      {
        success = false;
        textBuf[0] = '\0';
        if (open) *open = false;
      }
      ImGui::End();
      return;
    }

    if (!error.empty())
    {
      ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(255, 60, 60, 255));
      ImGui::TextWrapped("%s", error.c_str());
      ImGui::PopStyleColor();
  
    }

    ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0.18f, 0.18f, 0.18f, 1.0f));
    ImGui::SetNextItemWidth(400);
    ImGui::InputTextMultiline("Comment##feedback_text", textBuf, IM_ARRAYSIZE(textBuf), ImVec2(400, 120));
    CommonViews::sSpacing();

    ImGui::SetNextItemWidth(400);
    ImGui::InputText("Name (optional)", authorBuf, IM_ARRAYSIZE(authorBuf));
    ImGui::SetNextItemWidth(400);
    ImGui::InputText("Email (optional)", emailBuf, IM_ARRAYSIZE(emailBuf));
    ImGui::PopStyleColor();

    ImGui::Checkbox("Attach screenshot (optional)", &attachScreenshot);

    bool canSubmit = (strlen(textBuf) > 0) && !sending;

    if (sending)
    {
      ImGui::Text("Sending feedback…");
    }

    if (!sending)
    {
      if (!canSubmit)
      {
        ImGui::BeginDisabled();
      }
      if (ImGui::Button("Submit", ImVec2(120, 0)))
      {
        error.clear();
        sending = true;
        success = false;

        // Capture screenshot and load bytes
        std::string screenshotData;
        if (attachScreenshot) {
          screenshotData = captureScreenshotData();
        }

        nlohmann::json stateJson = ConfigService::getService()->currentConfig();
        std::string stateStr = stateJson.dump(4, ' ', false, nlohmann::detail::error_handler_t::replace);

        // Dispatch to LibraryService – it will run async internally
        LibraryService::getService()->submitFeedback(text(), stateStr, screenshotData, author(), email(), [this, open](bool ok, const std::string &err){
          success = ok;
          error = err;
          sending = false;
          if (success && open) *open = true; // keep open until user closes
        });
      }
      if (!canSubmit)
      {
        ImGui::EndDisabled();
      }
    }

    ImGui::End();
  }
};

#endif // FeedbackView_hpp 

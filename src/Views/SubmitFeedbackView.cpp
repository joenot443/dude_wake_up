#include "SubmitFeedbackView.hpp"
#include "ofxImGui.h"
#include "LibraryService.hpp"
#include "CommonViews.hpp"
#include "Console.hpp"

void SubmitFeedbackView::setup()
{
}

void SubmitFeedbackView::update()
{
}

void SubmitFeedbackView::draw()
{
  // Draw a form with a labeled box for name, email, and feedback
  ImGui::InputText("Name", feedback.name, IM_ARRAYSIZE(feedback.name));
  ImGui::InputText("Email", feedback.email, IM_ARRAYSIZE(feedback.email));
  ImGui::InputTextMultiline("Feedback", feedback.feedback, IM_ARRAYSIZE(feedback.feedback));
  CommonViews::mSpacing();
  if (ImGui::Button("Submit"))
  {
    submitFeedback();
    ImGui::CloseCurrentPopup();
  }
}

void SubmitFeedbackView::submitFeedback()
{
  LibraryService::getService()->submitFeedback(
      feedback, []()
      { log("Feedback submitted successfully"); },
      [](const std::string &error)
      {
        log(error);
      });
}

#include <stdio.h>
#include "ofMain.h"

#ifndef Feedback_hpp
#define Feedback_hpp

struct Feedback
{
  char name[32];
  char feedback[256];
  char email[64];
};

static std::string formatFeedback(Feedback feedback)
{
  std::stringstream ss;
  ss << "User Feedback" << "\n";
  ss << "Name: " << feedback.name << "\n";
  ss << "Email: " << feedback.email << "\n";
  ss << "Feedback: " << feedback.feedback << "\n";
  return ss.str();
}


#endif /* Feedback_hpp */

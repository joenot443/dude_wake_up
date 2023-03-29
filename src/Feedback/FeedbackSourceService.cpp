//
//  FeedbackSourceService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#include "FeedbackSourceService.hpp"

void FeedbackSourceService::registerFeedbackSource(std::shared_ptr<FeedbackSource> feedbackSource) {
  if (feedbackSourceMap.count(feedbackSource->id) != 0) {
    log("Reregistering FeedbackSource %s", feedbackSource->id.c_str());
  }
  feedbackSource->setup();
  feedbackSourceMap[feedbackSource->id] = feedbackSource;
}

std::vector<std::string> FeedbackSourceService::feedbackSourceNames() {
  std::vector<std::string> names;
  for (auto const& [key, val] : feedbackSourceMap) {
    names.push_back(val->name);
  }
  return names;
}

std::shared_ptr<FeedbackSource> FeedbackSourceService::feedbackSourceForId(std::string id) {
  if (feedbackSourceMap.count(id) == 0) {
    return NULL;
  }
  return feedbackSourceMap[id];
}

void FeedbackSourceService::removeFeedbackSource(std::string id) {
  if (feedbackSourceMap.count(id) == 0) {
    log("Tried to remove FeedbackSource %s, but it doesn't exist", id.c_str());
    return;
  }
  auto source = feedbackSourceMap[id];
  feedbackSourceMap.erase(id);
  source->teardown();
  source.reset();
}

std::vector<std::shared_ptr<FeedbackSource>> FeedbackSourceService::feedbackSources() {
  std::vector<std::shared_ptr<FeedbackSource>> feedbackSources;
  for (auto const& [key, val] : feedbackSourceMap) {
    feedbackSources.push_back(val);
  }
  return feedbackSources;
}

std::shared_ptr<FeedbackSource> FeedbackSourceService::defaultFeedbackSource() {
  return feedbackSources().front();
}

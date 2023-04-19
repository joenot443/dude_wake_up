//
//  FeedbackSourceService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#include "FeedbackSourceService.hpp"
#include "VideoSourceSettings.hpp"
#include "UUID.hpp"

static std::string DefaultSourceKey = "DefaultSource";

void FeedbackSourceService::setup() {
  std::shared_ptr<FeedbackSource> source = std::make_shared<FeedbackSource>(UUID::generateUUID(), "Empty", std::make_shared<VideoSourceSettings>(UUID::generateUUID(), 0));
  registerFeedbackSource(source);
  defaultFeedbackSource = source;
}

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

void FeedbackSourceService::setConsumer(std::string shaderId, std::shared_ptr<FeedbackSource> source) {
  consumerMap[shaderId] = source;
}

bool FeedbackSourceService::isSourceBeingConsumed(std::string feedbackSourceId) {
  for (auto const& [key, val] : consumerMap) {
    if (val->id == feedbackSourceId) return true;
  }
  return false;
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

void FeedbackSourceService::removeConsumerForShader(std::string shaderId) {
  consumerMap.erase(shaderId);
}

std::vector<std::shared_ptr<FeedbackSource>> FeedbackSourceService::feedbackSources() {
  std::vector<std::shared_ptr<FeedbackSource>> feedbackSources;
  for (auto const& [key, val] : feedbackSourceMap) {
    feedbackSources.push_back(val);
  }
  return feedbackSources;
}

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

void FeedbackSourceService::setup()
{
  std::shared_ptr<FeedbackSource> source = std::make_shared<FeedbackSource>(UUID::generateUUID(), std::make_shared<VideoSourceSettings>(UUID::generateUUID(), 0));
  registerFeedbackSource(source);
  defaultFeedbackSource = source;
  defaultFeedbackSource->setup();
}

void FeedbackSourceService::registerFeedbackSource(std::shared_ptr<FeedbackSource> feedbackSource)
{
  if (feedbackSourceMap.count(feedbackSource->id) != 0)
  {
    log("Reregistering FeedbackSource %s", feedbackSource->id.c_str());
  }
  feedbackSourceMap[feedbackSource->id] = feedbackSource;
}

void FeedbackSourceService::setConsumer(std::string shaderId, std::shared_ptr<FeedbackSource> source)
{
  bool found = false;
  for (auto &consumerPair : consumerMap)
  {
    if (consumerPair.second == source)
    {
      found = true;
      break;
    }
  }
  // Setup the feedbackSource if there are no consumers
  if (!found)
    source->setup();

  consumerMap[shaderId] = source;
}

void FeedbackSourceService::clearBuffers()
{
  for (auto const &[key, val] : feedbackSourceMap)
  {
    val->clearFrameBuffer();
  }
}

bool FeedbackSourceService::isSourceBeingConsumed(std::string feedbackSourceId)
{
  for (auto const &[key, val] : consumerMap)
  {
    if (val->id == feedbackSourceId)
      return true;
  }
  return false;
  }

std::shared_ptr<FeedbackSource> FeedbackSourceService::feedbackSourceForId(std::string id)
{
  if (feedbackSourceMap.count(id) == 0)
  {
    return NULL;
  }
  return feedbackSourceMap[id];
}

void FeedbackSourceService::removeFeedbackSource(std::string id)
{
  if (feedbackSourceMap.count(id) == 0)
  {
    log("Tried to remove FeedbackSource %s, but it doesn't exist", id.c_str());
    return;
  }
  auto source = feedbackSourceMap[id];
  feedbackSourceMap.erase(id);
  source->teardown();
  source.reset();
}

void FeedbackSourceService::removeConsumerForShader(std::string shaderId)
{
  consumerMap.erase(shaderId);
}

std::vector<std::shared_ptr<FeedbackSource>> FeedbackSourceService::feedbackSources()
{
  std::vector<std::shared_ptr<FeedbackSource>> feedbackSources;
  for (auto const &[key, val] : feedbackSourceMap)
  {
    feedbackSources.push_back(val);
  }
  return feedbackSources;
}

//
//  MultiSource.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 3/1/23.
//

#include "MultiSource.hpp"
#include "NodeLayoutView.hpp"
#include "LayoutStateService.hpp"

void MultiSource::setup()
{
  fbo->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y);
  optionalFbo->allocate(LayoutStateService::getService()->resolution.x, LayoutStateService::getService()->resolution.y);
  fbo->begin();
  fbo->end();
}

void MultiSource::saveFrame()
{
  frame()->begin();
  sources[selectedSource->intValue]->frame()->draw(0, 0);
  frame()->end();
}

json MultiSource::serialize()
{
  json j;
  j["id"] = id;
  j["sourceName"] = sourceName;
  j["videoSourceType"] = VideoSource_multi;
  j["settings"] = settings->serialize();
  auto node = NodeLayoutView::getInstance()->nodeForShaderSourceId(id);
  if (node != nullptr)
  {
    j["x"] = node->position.x;
    j["y"] = node->position.y;
  }
  return j;
}

void MultiSource::load(json j)
{
  if (!j.is_object())
  {
    log("Error hydrating MultiSource from json");
    return;
  }
  id = j["id"];
  sourceName = j["sourceName"];
  settings->load(j["settings"]);
  auto node = NodeLayoutView::getInstance()->nodeForShaderSourceId(id);
  if (node != nullptr)
  {
    node->position.x = j["x"];
    node->position.y = j["y"];
  }
}

void MultiSource::drawSettings() {
  // Create a vector of names
  std::vector<std::string> names;
  for (std::shared_ptr<VideoSource> source : sources) {
    names.push_back(source->sourceName);
  }
  CommonViews::Selector(selectedSource, names);
}

void MultiSource::addSource(std::shared_ptr<VideoSource> source) {
  sources.push_back(source);
}

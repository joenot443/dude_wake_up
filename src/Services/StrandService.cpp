#include "StrandService.hpp"
#include "ConfigService.hpp"

std::shared_ptr<AvailableStrand> StrandService::availableStrandForId(std::string id)
{
  if (strandMap.count(id) == 0)
  {
    return nullptr;
  }
  return strandMap[id];
}

std::vector<std::shared_ptr<AvailableStrand>> StrandService::availableStrands()
{
  std::vector<std::shared_ptr<AvailableStrand>> strands;
  for (auto const &[key, val] : strandMap)
  {
    strands.push_back(val);
  }
  return strands;
}

void StrandService::setup() {
  // Save default Config when a Strand is updated
  strandsUpdatedSubject.subscribe([this]()
  {
    ConfigService::getService()->saveDefaultConfigFile();
  });
}

void StrandService::notifyStrandsUpdated()
{
  strandsUpdatedSubject.notify();
}

void StrandService::subscribeToStrandsUpdated(std::function<void()> callback)
{
  strandsUpdatedSubject.subscribe(callback);
}


void StrandService::addStrand(std::shared_ptr<AvailableStrand> strand)
{
  if (strandMap.count(strand->id) != 0)
  {
    log("Reregistering Strand %s", strand->id.c_str());
  }
  strandMap[strand->id] = strand;
  notifyStrandsUpdated();
}

void StrandService::removeStrand(std::string id)
{
  if (strandMap.count(id) == 0)
  {
    log("Tried to remove Strand %s, but it doesn't exist",
        id.c_str());
    return;
  }
  auto strand = strandMap[id];
  strandMap.erase(id);
}



std::string StrandService::savePreview(std::string name, std::shared_ptr<Connectable> connectable) {
  // Get the terminal descendent from the Connectable and export its frame()
  // fbo to a .jpg file.
  auto terminal = connectable->terminalDescendent();
  if (terminal == nullptr) {
    return;
  }
  
  ofTexture tex = terminal->frame()->getTexture();
  ofPixels pixels;
  tex.readToPixels(pixels);
  std::string fileName = strandPreviewPath(name);
  ofSaveImage(pixels, fileName);
  return fileName;
}

std::string StrandService::strandPreviewPath(std::string name) {
  return ConfigService::getService()->relativeFilePathWithinNottawaFolder(formatString("%s.png", name.c_str()));
}

void StrandService::clear()
{
  for (auto it = strandMap.begin(); it != strandMap.end();)
  {
    auto key = it->first;
    auto strand = it->second;
    removeStrand(key);
    it = strandMap.erase(it);
  }
}

json StrandService::config()
{
  json j;
  for (auto const &[key, val] : strandMap)
  {
    j[val->name] = val->serialize();
  }
  return j;
}

void StrandService::loadConfig(json j)
{
  for (auto const &[key, val] : j.items())
  {
    std::string path = val["path"];
    std::string id = val["id"];
    std::string imagePath = val["imagePath"];
    // Check if a file exists at path
    if (!ofFile::doesFileExist(path)) {
      log("Strand file %s does not exist", path.c_str());
      continue;
    }
    
    auto strand = std::make_shared<AvailableStrand>(val["name"], path, imagePath, id);
    addStrand(strand);
  }
}

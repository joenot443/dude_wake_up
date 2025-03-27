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

std::vector<std::shared_ptr<AvailableStrand>> StrandService::availableTemplateStrands()
{
  std::vector<std::shared_ptr<AvailableStrand>> strands;
  for (auto const &[key, val] : templateMap)
  {
    strands.push_back(val);
  }
  return strands;
}

void StrandService::setup() {
  // Save default Config when a Strand is updated
  strandsUpdatedSubject.subscribe([this]()
  {
    if (running)
    	ConfigService::getService()->saveDefaultConfigFile();
  });
  
  populate();
  running = true;
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
    return "";
  }
  
  ofTexture tex = terminal->frame()->getTexture();
  ofPixels pixels;
  tex.readToPixels(pixels);
  std::filesystem::path fileName = strandPreviewPath(name);
  ofSaveImage(pixels, fileName);
  return fileName;
}

std::string StrandService::strandPreviewPath(std::string name) {
  return ConfigService::getService()->strandsFolderFilePath() + formatString("%s.png", name.c_str());
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

// Adds all strands in the nottawa folder to the strandMap
void StrandService::populate()
{
  std::string nottawaFolder = ConfigService::getService()->nottawaFolderFilePath();
  std::string templatesFolder = ConfigService::getService()->templatesFolderFilePath();
  
  populateMapFromFolder(&templateMap, templatesFolder);
  populateMapFromFolder(&strandMap, nottawaFolder);
}

void StrandService::populateMapFromFolder(std::map<std::string, std::shared_ptr<AvailableStrand>> *map, std::string folder) {
  
  ofDirectory directory;
  directory.open(folder);
  try {
    directory.listDir();
    directory.sort();
  } catch (std::exception &) {
    log("Couldn't read Strands directory");
  }
  
  for (int i = 0; i < directory.size(); i++)
  {
    auto file = directory.getFile(i);
    bool isDirectory = file.isDirectory();
    if (isDirectory)
      continue;
    bool isJson = ofIsStringInString(file.getFileName(), ".json");
    if (!isJson)
      continue;
    
    nlohmann::json json;
    std::fstream fileStream;
    std::string path = file.path();
    fileStream.open(path, std::ios::in);
    if (fileStream.is_open())
    {
      try
      {
        // Make sure the file isn't empty
        if (fileStream.peek() == std::ifstream::traits_type::eof())
        {
          log("JSON file for %s is empty.", path.c_str());
          return;
        }
        fileStream >> json;
      }
      catch (int code)
      {
        log("Could not load JSON file for %s.", path.c_str());
        return;
      }
    }
    
    
    if (json[ConfigTypeKey] != ConfigTypeStrand && json[ConfigTypeKey] != ConfigTypeFull)
    {
      continue;
    }
    std::string imagePath;
    if (json.contains("preview")) {
      imagePath = json["preview"];
    } else {
      imagePath = "";
    }
    
    auto strand = std::make_shared<AvailableStrand>(file.getFileName(), file.getAbsolutePath(), imagePath, UUID::generateUUID());
    if (strandNames.count(strand->name) != 0) {
      log("Adding Strand %s which already exists", strand->name.c_str());
      continue;
    }
    strandNames.insert(strand->name);
    (*map)[strand->id] = strand;
    notifyStrandsUpdated();
  }
}

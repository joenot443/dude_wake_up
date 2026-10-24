#include "StrandService.hpp"
#include <set>
#include "ConfigService.hpp"
#include <algorithm>
#include <cctype>
#include <filesystem>

namespace {
// Paths reach the maps in different spellings (separators, doubled
// slashes), so compare them normalised.
std::string normalizedPath(const std::string& path)
{
  if (path.empty()) return path;
  std::string out = std::filesystem::path(path).lexically_normal().generic_string();
#ifdef _WIN32
  // Windows paths are case-insensitive.
  std::transform(out.begin(), out.end(), out.begin(), [](unsigned char c) { return char(std::tolower(c)); });
#endif
  return out;
}
} // namespace

std::shared_ptr<AvailableStrand> StrandService::availableStrandForId(std::string id)
{
  if (strandMap.count(id) != 0)
  {
    return strandMap[id];
  }
  if (folderMap.count(id) != 0)
  {
    return folderMap[id];
  }
  if (templateMap.count(id) != 0)
  {
    return templateMap[id];
  }
  return nullptr;
}

std::vector<std::shared_ptr<AvailableStrand>> StrandService::availableStrands()
{
  // A strand saved this session is in strandMap and, once the folder is
  // rescanned, in folderMap too; list each file once.
  std::vector<std::shared_ptr<AvailableStrand>> strands;
  std::set<std::string> paths;
  for (auto const *map : {&strandMap, &folderMap})
  {
    for (auto const &[key, val] : *map)
    {
      if (!val->path.empty() && !paths.insert(normalizedPath(val->path)).second) continue;
      strands.push_back(val);
    }
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
    	ConfigService::getService()->requestSave();
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
  auto strand = availableStrandForId(id);
  if (strand == nullptr)
  {
    log("Tried to remove Strand %s, but it doesn't exist",
        id.c_str());
    return;
  }
  // Drop every entry for the strand's file, from either map.
  for (auto *map : {&strandMap, &folderMap})
  {
    for (auto it = map->begin(); it != map->end();)
    {
      if (it->first == id || (!strand->path.empty() && normalizedPath(it->second->path) == normalizedPath(strand->path)))
        it = map->erase(it);
      else ++it;
    }
  }
  strandNames.erase(strand->name);
  notifyStrandsUpdated();
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
  return fileName.string();
}

std::string StrandService::strandPreviewPath(std::string name) {
  return ConfigService::getService()->strandsFolderFilePath() + formatString("%s.png", name.c_str());
}

// New helper that allows callers to specify exactly where the preview image should be written
std::string StrandService::savePreviewToPath(std::string fullPath, std::shared_ptr<Connectable> connectable) {
  // Get the terminal descendant from the Connectable and export its frame() FBO to an image file.
  auto terminal = connectable == nullptr ? nullptr : connectable->terminalDescendent();
  if (terminal == nullptr) {
    return ""; // Nothing to save
  }

  ofTexture tex = terminal->frame()->getTexture();
  ofPixels pixels;
  tex.readToPixels(pixels);
  ofSaveImage(pixels, fullPath);
  return fullPath;
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

// Adds all strands in the nottawa/strands folder to the strandMap
void StrandService::populate()
{
  std::string nottawaFolder = ConfigService::getService()->strandsFolderFilePath();
  std::string templatesFolder = ConfigService::getService()->templatesFolderFilePath();
  
  populateMapFromFolder(&templateMap, templatesFolder);
  populateMapFromFolder(&folderMap, nottawaFolder);
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
    }
    // Without a recorded preview, use a same-named PNG beside the file.
    if (imagePath.empty()) {
      auto sibling = std::filesystem::path(file.getAbsolutePath()).replace_extension(".png");
      std::error_code ec;
      if (std::filesystem::exists(sibling, ec)) imagePath = sibling.string();
    }
    
    auto strand = std::make_shared<AvailableStrand>(removeFileExtension(file.getFileName()), file.getAbsolutePath(), imagePath, NottawaUUID::generateUUID());
    if (strandNames.count(strand->name) != 0) {
      log("Adding Strand %s which already exists", strand->name.c_str());
      continue;
    }
    if (strand->name == "nottawa_final") {
      demoStrand = strand;
    }
    strandNames.insert(strand->name);
    (*map)[strand->id] = strand;
    notifyStrandsUpdated();
  }
}

void StrandService::renameStrand(std::string id, std::string newName)
{
  auto strand = availableStrandForId(id);
  if (strand == nullptr)
  {
    log("Tried to rename Strand %s, but it doesn't exist", id.c_str());
    return;
  }

  // Determine new file path (same directory, new filename)
  std::filesystem::path oldPath(strand->path);
  std::filesystem::path newPath = oldPath.parent_path() / (newName + ".json");

  try
  {
    // Rename file on disk
    if (std::filesystem::exists(oldPath))
    {
      std::filesystem::rename(oldPath, newPath);
    }
  }
  catch (const std::exception &e)
  {
    log("Failed to rename Strand file: %s", e.what());
    return;
  }

  // Update every entry for the file (it can be in both maps).
  const std::string oldPathString = normalizedPath(oldPath.string());
  strandNames.erase(strand->name);
  strandNames.insert(newName);
  for (auto *map : {&strandMap, &folderMap})
  {
    for (auto &[key, entry] : *map)
    {
      if (entry != strand && normalizedPath(entry->path) != oldPathString) continue;
      entry->name = newName;
      entry->path = newPath.string();
    }
  }

  notifyStrandsUpdated();
}

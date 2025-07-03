//
//  StrandService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/4/22.
//

#ifndef StrandService_hpp
#define StrandService_hpp

#include <stdio.h>
#include "Connection.hpp"
#include "Models/Strand.hpp"
#include "observable.hpp"
#include "ConfigurableService.hpp"
#include "AvailableStrand.hpp"
#include "json.hpp"

using json = nlohmann::json;

class StrandService : public ConfigurableService
{
private:
  std::map<std::string, std::shared_ptr<AvailableStrand>> strandMap;
  std::map<std::string, std::shared_ptr<AvailableStrand>> templateMap;
  std::map<std::string, std::shared_ptr<AvailableStrand>> folderMap;
  
  std::set<std::string> strandNames;
  
  observable::subject<void()> strandsUpdatedSubject;

public:
  static StrandService *service;
  StrandService(){};

  static StrandService *getService()
  {
    if (!service)
    {
      service = new StrandService;
    }
    return service;
  }

  bool running = false;
  
  void setup();
  
  void populate();
  
  void populateMapFromFolder(std::map<std::string, std::shared_ptr<AvailableStrand>> *map, std::string folder);
  
  // Notifications
  void notifyStrandsUpdated();
  void subscribeToStrandsUpdated(std::function<void()> callback);

  std::vector<std::shared_ptr<AvailableStrand>> availableStrands();
  std::vector<std::shared_ptr<AvailableStrand>> availableTemplateStrands();
  std::shared_ptr<AvailableStrand> availableStrandForId(std::string id);
  
  void addStrand(std::shared_ptr<AvailableStrand> strand);
  void removeStrand(std::string id);
  std::string savePreview(std::string name, std::shared_ptr<Connectable> connectable);
  std::string savePreviewToPath(std::string fullPath, std::shared_ptr<Connectable> connectable);
  std::string strandPreviewPath(std::string name);
  void clear();

  void renameStrand(std::string id, std::string newName);
  
  std::shared_ptr<AvailableStrand> demoStrand;
  json config() override;
  void loadConfig(json j) override;
};

#endif /* StrandService_hpp */

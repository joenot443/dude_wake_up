//
//  StrandService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/4/22.
//

#ifndef StrandService_hpp
#define StrandService_hpp

#include <stdio.h>
#include "Strand.hpp"
#include "observable.hpp"
#include "ConfigurableService.hpp"
#include "AvailableStrand.hpp"
#include "json.hpp"

using json = nlohmann::json;

class StrandService : public ConfigurableService
{
private:
  std::map<std::string, std::shared_ptr<AvailableStrand>> strandMap;
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

  void setup();
  
  // Notifications
  void notifyStrandsUpdated();
  void subscribeToStrandsUpdated(std::function<void()> callback);

  std::vector<std::shared_ptr<AvailableStrand>> availableStrands();
  std::shared_ptr<AvailableStrand> availableStrandForId(std::string id);
  
  void addStrand(std::shared_ptr<AvailableStrand> strand);
  void removeStrand(std::string id);
  std::string savePreview(std::string name, std::shared_ptr<Connectable> connectable);
  std::string strandPreviewPath(std::string name);
  void clear();

  json config() override;
  void loadConfig(json j) override;
};

#endif /* StrandService_hpp */

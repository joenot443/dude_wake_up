//
//  FeedbackSourceService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/3/22.
//

#ifndef FeedbackSourceService_hpp
#define FeedbackSourceService_hpp

#include <stdio.h>
#include "FeedbackSource.hpp"

// Singleton class which stores shared references to FeedbackSource and allows for their addition and deletion

class FeedbackSourceService {
private:
  std::map<std::string, std::shared_ptr<FeedbackSource>> feedbackSourceMap;

public:
  static FeedbackSourceService* service;
  FeedbackSourceService() {};
  
  static FeedbackSourceService* getService() {
    if (!service) {
      service = new FeedbackSourceService;
    }
    return service;
    
  }
  std::vector<std::shared_ptr<FeedbackSource>> feedbackSources();
  std::shared_ptr<FeedbackSource> defaultFeedbackSource();

  std::vector<std::string> feedbackSourceNames();
  
  void removeFeedbackSource(std::string id);
  void registerFeedbackSource(std::shared_ptr<FeedbackSource> feedbackSource);
  std::shared_ptr<FeedbackSource> feedbackSourceForId(std::string id);
};


#endif /* FeedbackSourceService_hpp */

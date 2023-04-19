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
  
  // Maps a shaderId to a FeedbackSource to demonstrate that the Shader is consuming
  // that FeedbackSource.
  std::map<std::string, std::shared_ptr<FeedbackSource>> consumerMap;

public:
  static FeedbackSourceService* service;
  FeedbackSourceService() {};
  
  static FeedbackSourceService* getService() {
    if (!service) {
      service = new FeedbackSourceService;
      service->setup();
    }
    return service;
    
  }
  std::vector<std::shared_ptr<FeedbackSource>> feedbackSources();
  std::shared_ptr<FeedbackSource> defaultFeedbackSource;

  std::vector<std::string> feedbackSourceNames();
  
  void setup();
  void removeFeedbackSource(std::string id);
  void removeConsumerForShader(std::string shaderId);
  void setConsumer(std::string shaderId, std::shared_ptr<FeedbackSource> source);
  bool isSourceBeingConsumed(std::string feedbackSourceId);
  void registerFeedbackSource(std::shared_ptr<FeedbackSource> feedbackSource);
  std::shared_ptr<FeedbackSource> feedbackSourceForId(std::string id);
};


#endif /* FeedbackSourceService_hpp */

//
//  LayoutStateService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/30/23.
//

#ifndef LayoutStateService_hpp
#define LayoutStateService_hpp

#include <stdio.h>

class LayoutStateService {
public:
  LayoutStateService(){};

  bool showAudioSettings;
  static LayoutStateService *service;

  static LayoutStateService *getService() {
    if (!service) {
      service = new LayoutStateService;
    }
    return service;
  }
};

#endif /* LayoutStateService_hpp */

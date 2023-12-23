//
//  MarkdownService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/22/23.
//

#ifndef MarkdownService_hpp
#define MarkdownService_hpp

#include <stdio.h>
#include "Console.hpp"
#include "MarkdownItem.hpp"
#include <map>
#include <vector>
#include <deque>

class MarkdownService {
private:
  void setup();
  std::map<std::string, std::shared_ptr<MarkdownItem>> itemNameMap;
  
  void scanDirectory();
  std::shared_ptr<MarkdownItem> defaultItem;
  
public:
  static MarkdownService* service;
  std::shared_ptr<MarkdownItem> itemNamed(std::string name);
  
  bool hasItemForName(std::string name);

  MarkdownService() {};
  static MarkdownService* getService() {
    if (!service) {
      service = new MarkdownService;
      service->setup();
    }
    return service;
  }
};

#endif /* MarkdownService_hpp */

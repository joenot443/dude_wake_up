//
//  MarkdownView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/22/23.
//

#ifndef MarkdownView_hpp
#define MarkdownView_hpp

#include <stdio.h>
#include "Strings.hpp"
#include "MarkdownService.hpp"
#include "CommonViews.hpp"


class MarkdownView {
public:
  void setup();
  void draw();
  void update();
  
  std::string name;
  std::shared_ptr<MarkdownItem> item;
  
  MarkdownView(std::string name) : name(name) {
    setup();
  };
};

#endif /* MarkdownView_hpp */

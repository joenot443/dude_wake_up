//
//  MarkdownItem.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 10/31/23.
//

#ifndef MarkdownItem_h
#define MarkdownItem_h

struct MarkdownItem {
  std::string name;
  std::string contents;
  
  MarkdownItem(std::string name, std::string contents) : name(name), contents(contents) {};
};

#endif /* MarkdownItem_h */

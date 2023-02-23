//
//  MarkdownService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/22/23.
//

#include "MarkdownService.hpp"
#include "Console.hpp"
#include "ofMain.h"

static const std::string MarkdownDirectory = "markdown";

void MarkdownService::setup() {
  scanDirectory();
  defaultItem = std::make_shared<MarkdownItem>("default", "Missing .md file for that name.");
}

void MarkdownService::scanDirectory() {
  ofDirectory directory;
  directory.open(MarkdownDirectory);
  directory.listDir();
  directory.sort();

  for (int i = 0; i < directory.size(); i++) {
    auto file = directory.getFile(i);
    bool isDirectory = file.isDirectory();
    if (isDirectory) continue;
    
    bool isMarkdown = ofIsStringInString(file.getFileName(), ".md");
    if (!isMarkdown) continue;;
    
    auto name = ofSplitString(file.getFileName(), ".").front();
    ofBuffer buffer = ofBufferFromFile(file.getAbsolutePath());
    
    auto item = MarkdownItem(name, buffer.getText());
    
    itemNameMap[name] = std::make_shared<MarkdownItem>(item);
  }
  
  log(itemNameMap.size());
}

std::shared_ptr<MarkdownItem> MarkdownService::itemNamed(std::string name) {
  if (itemNameMap.count(name) == 0) return defaultItem;
  
  return itemNameMap[name];
}

bool MarkdownService::hasItemForName(std::string name) {
  return itemNameMap.count(name) != 0;
}

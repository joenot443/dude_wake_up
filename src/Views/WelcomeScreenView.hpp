//
//  WelcomeScreenView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 6/28/24.
//

#ifndef WelcomeScreenView_hpp
#define WelcomeScreenView_hpp

#include <stdio.h>
#include "StrandBrowserWindow.hpp"

class WelcomeScreenView {
public:
  StrandBrowserWindow recentBrowserWindow;
  StrandBrowserWindow templateBrowserWindow;
  
  WelcomeScreenView() : recentBrowserWindow(StrandBrowserWindow(StrandBrowserType_Recent)), templateBrowserWindow(StrandBrowserWindow(StrandBrowserType_Template)) {
    
  };
  
  void newWorkspace();
  void openWorkspace();
  
  void setup();
  void draw();
  void update();
};

#endif /* WelcomeScreenView_hpp */

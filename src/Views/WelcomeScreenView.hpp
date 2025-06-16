//
//  WelcomeScreenView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 6/28/24.
//

#ifndef WelcomeScreenView_hpp
#define WelcomeScreenView_hpp

#include <stdio.h>
#include "StrandBrowserView.hpp"
#include "StrandBrowserWindow.hpp"
#include "Models/Strand.hpp"

class ofVideoPlayer;

class WelcomeScreenView {
public:
  StrandBrowserWindow recentBrowserWindow;
  StrandBrowserWindow templateBrowserWindow;
  
  ofVideoPlayer player;
  
  WelcomeScreenView() : recentBrowserWindow(StrandBrowserWindow(StrandBrowserType_Recent)), templateBrowserWindow(StrandBrowserWindow(StrandBrowserType_Template)) {
    
  };
  
  void newWorkspace();
  void openWorkspace();
  
  void setup();
  void openDemo();
  void draw();
  void update();
};

#endif /* WelcomeScreenView_hpp */

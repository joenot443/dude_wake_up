//
//  HelpService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 6/17/24.
//

#ifndef HelpService_hpp
#define HelpService_hpp

#include <stdio.h>
#include "ConfigurableService.hpp"
#include "MarkdownView.hpp"

class HelpService {

public:
  bool completed;
  
  HelpService() {};
  
  bool placedSource();
  bool placedEffect();
  bool madeConnection();
  bool placedSecondSource();
  bool placedMixEffect();
  bool madeFirstMixConnection();
  bool madeSecondMixConnection();
  bool openedShaderInfo();
  bool editedMixParameter();
  bool openedStageMode();
  
  // Drop Source
  void drawSourceView();
  void drawSource2View();
  
  // Drop Effect
  void drawEffectView();
  void drawEffect2View();
  
  // Connect Source + Effect
  void drawConnectionView(ImVec2 sourceNodePosition, ImVec2 sourceNodeSize, float scale);
  void drawConnection2View(ImVec2 destNodePosition, ImVec2 destNodeSize, float scale);
  
  // Drop Second Source
  void drawSecondSourceView();
  void drawSecondSource2View();
  
  // Drop Mix Effect
  void drawMixEffectView();
  void drawMixEffect2View();
  void drawWrongEffectView();
  
  // Connect first Source to Mix
  void drawMixConnectionView(ImVec2 nodePosition, ImVec2 nodeSize, float scale);
  void drawMixConnection2View(ImVec2 nodePosition, ImVec2 nodeSize, float scale);
  
  // Connect second Source to Mix
  void drawSecondMixConnectionView(ImVec2 nodePosition, ImVec2 nodeSize, float scale);
  void drawSecondMixConnection2View(ImVec2 nodePosition, ImVec2 nodeSize, float scale);
  
  // Open the ShaderInfo pane
  void drawOpenShaderInfoView(ImVec2 nodePosition, ImVec2 nodeSize, float scale);
  
  // Change parameters on the ShaderInfo pane
  void drawEditParametersShaderInfoPane();
  
  // Open the Stage view
  void drawActionButtons();
  
  // Help in Stage Mode
  void drawStageModeHelp();
  
  void drawStageModeActionButtonsHelp();

  void drawRightArrowView();
  void drawLeftArrowView();
  
  void drawHelp();
  
  static HelpService *service;

  static HelpService *getService() {
    if (!service) {
      service = new HelpService;
    }
    return service;
  }
};

#endif /* HelpService_hpp */

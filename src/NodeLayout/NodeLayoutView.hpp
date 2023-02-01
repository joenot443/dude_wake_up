//
//  NodeLayoutView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/5/23.
//

#ifndef NodeLayoutView_hpp
#define NodeLayoutView_hpp

#include "NodeTypes.hpp"
#include "Shader.hpp"
#include "Shader.hpp"
#include "imgui_node_editor.h"
#include <stdio.h>

namespace ed = ax::NodeEditor;

struct NodeLayoutView {
public:
  void setup();
  void update();
  void draw();
  void queryNewLinks();
  void handleDropZone();
  void drawNodeWindows();
  void openSettingsWindow(std::shared_ptr<Shader> shader);
  void keyReleased(int key);

  ed::EditorContext *context = nullptr;
  bool firstFrame = true;
  bool shouldDelete = false;

  
  // Maps the id of the Shader or the VideoSource to the Node
  std::map<std::string, std::shared_ptr<Node>> idNodeMap;
  // Maps the id of the Pin to the Node
  std::map<long, std::shared_ptr<Node>> pinIdNodeMap;
  // Maps the id of a Pin to a shared_ptr to the Pin
  std::map<long, std::shared_ptr<Pin>> pinIdPinMap;
  // Maps the id of a Node to a shared_ptr of the Node
  std::map<long, std::shared_ptr<Node>> nodeIdNodeMap;

  // Maps the id of a Link to the ShaderLink
  std::map<long, std::shared_ptr<ShaderLink>> linksMap;
  
  std::set<std::shared_ptr<Shader>> shadersToOpen;

  std::unique_ptr<ImVec2> shaderDropLocation;
  std::unique_ptr<ImVec2> sourceDropLocation;
};

#endif /* NodeLayoutView_hpp */

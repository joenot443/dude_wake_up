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
#include "VideoSource.hpp"
#include "Shader.hpp"
#include "imgui_node_editor.h"
#include <stdio.h>

namespace ed = ax::NodeEditor;

class NodeLayoutView {
public:
  void setup();
  void update();
  void draw();

  void drawNodeWindows();
  void drawResolutionPopup();
  void drawUploadChainerWindow();
  
  void openSettingsWindow(std::shared_ptr<Shader> shader);
  void keyReleased(int key);
  
  // Returns the Node for the associated shaderSourceId, if it exists.
  // If it doesn't, create a new Node.
  std::shared_ptr<Node> nodeForShaderSourceId(std::string shaderSourceId, NodeType nodeType, std::string name, bool supportsAux);
  
  void debug();
  
  // Layout
  void queryNewLinks();
  void drawNode(std::shared_ptr<Node> node);
  int nodeIdTicker = 1;
  
  // Handlers
  void handleUnplacedNodes();
  void handleDropZone();
  void handleDoubleClick();
  void handleRightClick();
  void handleSaveNode(std::shared_ptr<Node> node);
  void handleDeleteNode(std::shared_ptr<Node> node);
  void handleDroppedSource(std::shared_ptr<VideoSource> source);
  void handleUploadChain(std::shared_ptr<Node> node);

  // Selection
  
  void selectChainer(std::shared_ptr<Node> node);

  ed::EditorContext *context = nullptr;
  std::vector<std::string> unplacedNodeIds = {};
  std::vector<std::shared_ptr<Node>> nodes;
  
  // Resolution Popup
  std::shared_ptr<Node> resolutionPopupNode;
  ImVec2 resolutionPopupLocation;
  bool popupLaunched;
  
  // Uploading Chainer
  std::shared_ptr<ShaderChainer> uploadChainer = nullptr;
  
  // Misc.
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
  
  std::set<std::shared_ptr<Node>> nodesToOpen;

  std::unique_ptr<ImVec2> nodeDropLocation;
  ed::NodeId contextMenuNodeId = 0;
  ImVec2 contextMenuLocation;
};

#endif /* NodeLayoutView_hpp */

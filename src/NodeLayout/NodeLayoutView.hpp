//
//  NodeLayoutView.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/5/23.
//

#ifndef NodeLayoutView_hpp
#define NodeLayoutView_hpp

#include "NodeTypes.hpp"
#include "imgui.h"
#include "VideoRecorder.hpp"
#include "Shader.hpp"
#include "VideoSource.hpp"
#include "Shader.hpp"
#include "imgui_node_editor.h"
#include <stdio.h>

namespace ed = ax::NodeEditor;

class NodeLayoutView {
public:
  static NodeLayoutView *instance;
  NodeLayoutView(){};
  
  static NodeLayoutView *getInstance() {
    if (!instance) {
      instance = new NodeLayoutView;
    }
    return instance;
  }
  
  void setup();
  void update();
  void draw();

  void drawNodeWindows();
  void drawPreviewWindows();
  void drawMetrics();
  void drawResolutionPopup();
  void drawUploadChainerWindow();
  void drawHelp();
  
  void openSettingsWindow(std::shared_ptr<Shader> shader);
  void keyReleased(int key);
  
  // Returns the Node for the associated shaderSourceId, if it exists.
  // If it doesn't, create a new Node.
  std::shared_ptr<Node> nodeForShaderSourceId(std::string shaderSourceId, NodeType nodeType, std::string name, std::shared_ptr<Connectable> connectable);
  
  // Returns the Node for the associated shaderSourceId, if it exists.
  // nullptr if not.
  std::shared_ptr<Node> nodeForShaderSourceId(std::string shaderSourceId);
  
  /// Returns the x,y coordinates of the Node for the passed ID. If not found, (0,0) is returned.
  ImVec2 coordinatesForNode(std::string id);
  
  
  
  void debug();
  
  // Layout
  void queryNewLinks();
  void populateNodePositions();
  void drawNode(std::shared_ptr<Node> node);
  void drawPreviewWindow(std::shared_ptr<Node> node);
  void drawActionButtons();
  void clear();
  void toggleCustomZoom();
  int nodeIdTicker = 1;
  
  // Handlers
  void handleUnplacedNodes();
  void handleDoubleClick();
  void handleRightClick();
  void handleSaveNode(std::shared_ptr<Node> node);
  void handleDeleteNode(std::shared_ptr<Node> node);
  void handleUploadChain(std::shared_ptr<Node> node);
  void handleUnplacedDownloadedLibraryFile();
  
  // Drag/Drop
  void handleDropZone();
  void handleDroppedSource(std::shared_ptr<VideoSource> source);
  std::shared_ptr<Node> nodeAtPosition(ImVec2 position);
  bool pointIsWithinNode(ImVec2 position, std::shared_ptr<Node> node);

  // Notifications
  void haveDownloadedAvailableLibraryFile(LibraryFile &file);
  
  // Layout
  
  // Selection
  void selectChainer(std::shared_ptr<Node> node);

  ed::EditorContext *context = nullptr;
  ed::Config *config = nullptr;
  std::vector<std::string> unplacedNodeIds = {};
  std::unique_ptr<ImVec2> nodeDropLocation;
  std::unique_ptr<LibraryFile> pendingFile;
  
  std::set<std::shared_ptr<Node>> nodes = {};
  std::set<std::shared_ptr<Node>> previewWindowNodes = {};
  std::set<std::shared_ptr<Node>> terminalNodes = {};
  
  // Resolution Popup
  std::shared_ptr<Node> resolutionPopupNode;
  ImVec2 resolutionPopupLocation;
  bool popupLaunched;
  
  // Misc.
  bool firstFrame = true;
  bool shouldDelete = false;
  bool makingLink = false;

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
  
  
  ed::NodeId contextMenuNodeId = 0;
  ImVec2 contextMenuLocation;
  VideoRecorder recorder;
};

#endif /* NodeLayoutView_hpp */

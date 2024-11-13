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
#include "LibraryFile.hpp"
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
  void drawSaveDialog();
  
  void closeSettingsWindow(std::shared_ptr<Connectable> connectable);
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
  void drawDebugWindow();
  
  // Layout
  void queryNewLinks();
  void populateNodePositions();
  void populateSelectedNodes();
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
  
  void handleDropZone();
  void handleDroppedSource(std::shared_ptr<VideoSource> source);
  std::shared_ptr<Node> nodeAtPosition(ImVec2 position);
  bool pointIsWithinNode(ImVec2 position, std::shared_ptr<Node> node);
  
  void loadStrand(std::shared_ptr<AvailableStrand> availableStrand);
  void saveStrand();

  void haveDownloadedAvailableLibraryFile(LibraryFile &file);
  
  bool canDelete();
  
  void selectChainer(std::shared_ptr<Node> node);

  void createBlendShaderForSelectedNodes(ed::NodeId firstNodeId, ed::NodeId secondNodeId);
  std::shared_ptr<Node> getNodeById(ed::NodeId nodeId);
  void addUnplacedConnectable(std::shared_ptr<Connectable> connectable);

  ed::EditorContext *context = nullptr;
  ed::Config *config = nullptr;
  std::vector<std::string> unplacedNodeIds = {};
  std::unique_ptr<ImVec2> nodeDropLocation;
  std::vector<std::shared_ptr<LibraryFile>> pendingFiles;
  
  std::set<std::shared_ptr<Node>> nodes = {};
  std::set<std::shared_ptr<Node>> previewWindowNodes = {};
  std::set<std::shared_ptr<Node>> terminalNodes = {};
  std::vector<std::shared_ptr<Connectable>> selectedConnectables = {};
  
  std::shared_ptr<Node> resolutionPopupNode;
  ImVec2 resolutionPopupLocation;
  bool popupLaunched;
  
  bool debugWindowOpen = false;
  bool firstFrame = true;
  bool shouldDelete = false;
  bool makingLink = false;
  bool fileDropInProgress = false;
  
  bool showSaveDialog = false;
  char saveFileName[256] = "";
  std::shared_ptr<Node> nodeToSave;
  
  std::map<std::string, std::shared_ptr<Node>> idNodeMap;
  std::map<long, std::shared_ptr<Node>> pinIdNodeMap;
  std::map<long, std::shared_ptr<Pin>> pinIdPinMap;
  std::map<long, std::shared_ptr<Node>> nodeIdNodeMap; // Use this map for node lookup
	
  std::map<long, std::shared_ptr<ShaderLink>> linksMap;
  
  std::set<std::shared_ptr<Node>> nodesToOpen;
  
  ed::NodeId contextMenuNodeId = 0;
  ImVec2 contextMenuLocation;
  VideoRecorder recorder;
};

#endif /* NodeLayoutView_hpp */

//
//  NodeLayoutView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/5/23.
//

#include "NodeLayoutView.hpp"
#include "FontService.hpp"
#include "Fonts.hpp"
#include "UploadChainerView.h"
#include "LayoutStateService.hpp"
#include "ShaderChainerService.hpp"
#include "Strings.hpp"
#include "ConfigService.hpp"
#include "VideoSourceService.hpp"
#include "Colors.hpp"
#include "ofMain.h"
#include <imgui.h>
#include <imgui_node_editor.h>

static const float ImGuiWindowTitleBarHeight = 70.0f;
static const float NodeWidth = 100.0f;

namespace ed = ax::NodeEditor;

void NodeLayoutView::setup() {
  ed::Config config;
  config.SettingsFile = "NodeLayout.json";
  
  context = ed::CreateEditor(&config);
}

void NodeLayoutView::update() {}

void ImGuiEx_BeginColumn() { ImGui::BeginGroup(); }

void ImGuiEx_NextColumn() {
  ImGui::EndGroup();
  ImGui::SameLine();
  ImGui::BeginGroup();
}

void ImGuiEx_EndColumn() { ImGui::EndGroup(); }

void NodeLayoutView::debug() {
  auto chainers = ShaderChainerService::getService()->shaderChainers();
  auto shaders = ShaderChainerService::getService()->shaders();
  
  for (auto c : chainers) {
    auto id = idNodeMap[c->source->id]->id;
    log("%s - %d", c->name.c_str(), id.Get());
  }
  
  for (auto s : shaders) {
    auto id = idNodeMap[s->id()]->id;
    log("%s - %d", s->name().c_str(), id.Get());
  }
}

void NodeLayoutView::draw() {
  auto &io = ImGui::GetIO();
  
  auto chainers = ShaderChainerService::getService()->shaderChainers();
  auto shaders = ShaderChainerService::getService()->shaders();
  
  nodes = {};
  
  ed::SetCurrentEditor(context);
  
  bool show = LayoutStateService::getService()->showAudioSettings;
  float height = show ? ofGetViewportHeight() * 3.0f / 5.0f
  : ofGetViewportHeight() - ImGuiWindowTitleBarHeight;
  
  // Start interaction with editor.
  ed::Begin("My Editor", ImVec2(ofGetWindowWidth() * (3.0f / 5.0f), height));
  
  ed::PushStyleVar(ed::StyleVar_ScrollDuration, 0.1f);
  int uniqueId = 1;
  int linkUniqueId = 1000;
  int sourceUniqueId = 10000;
  linksMap.clear();
  nodeIdNodeMap.clear();
  pinIdPinMap.clear();
  pinIdNodeMap.clear();
  
  // Loop over Shaders, creating a ShaderNode for each
  for (std::shared_ptr<Shader> shader : shaders) {
    auto shaderNode = nodeForShaderSourceId(shader->id(), NodeTypeShader, shader->name(), shader->supportsAux());
    shaderNode->shader = shader;
    drawNode(shaderNode);
    
    nodes.push_back(shaderNode);
  }
  
  // Create VideoSource Nodes
  for (std::shared_ptr<ShaderChainer> shaderChainer : chainers) {
    // Create a Link to the front shader, if it exists
    // Make source node
    
    auto sourceNode = nodeForShaderSourceId(shaderChainer->source->id, NodeTypeSource, shaderChainer->name, false);
    sourceNode->source = shaderChainer->source;
    sourceNode->chainer = shaderChainer;
    drawNode(sourceNode);
    
    // If our Source Node is connected, add a Link
    if (shaderChainer->front != nullptr) {
      ed::LinkId sourceNodeFrontShaderLink = linkUniqueId++;
      auto shaderNode = idNodeMap[shaderChainer->front->shaderId];
      ed::Link(sourceNodeFrontShaderLink, sourceNode->outputId, shaderNode->inputId);
      
      // Create the ShaderLink
      auto link = std::make_shared<ShaderLink>(sourceNodeFrontShaderLink,
                                               sourceNode, shaderNode);
      link->type = LinkTypeSource;
      linksMap[sourceNodeFrontShaderLink.Get()] = link;
    }
  }
  
  // Loop over ShaderNodes and draw links
  
  for (auto const &x : nodeIdNodeMap) {
    // Only create Links between ShaderNodes
    if (x.second->type != NodeTypeShader)
      continue;
    
    auto shaderNode = x.second;
    
    // Create a Link from Shader -> Shader
    if (shaderNode->shader->next != nullptr) {
      std::shared_ptr<Node> nextNode =
      idNodeMap[shaderNode->shader->next->id()];
      ed::LinkId nextShaderLinkId = uniqueId++;
      
      auto shaderLink =
      std::make_shared<ShaderLink>(nextShaderLinkId, nextNode, shaderNode);
      linksMap[nextShaderLinkId.Get()] = shaderLink;
      ed::Link(nextShaderLinkId, shaderLink->output->outputId,
               shaderLink->input->inputId);
    }
    
    // [Aux] Create a Link from Shader -> Shader
    if (shaderNode->shader->supportsAux() &&
        shaderNode->shader->aux != nullptr) {
      std::shared_ptr<Node> auxNode = idNodeMap[shaderNode->shader->aux->id()];
      ed::LinkId nextShaderLinkId = uniqueId++;
      shaderNode->auxNode = auxNode;
      auto shaderLink =
      std::make_shared<ShaderLink>(nextShaderLinkId, shaderNode, auxNode);
      shaderLink->type = LinkTypeAux;
      linksMap[nextShaderLinkId.Get()] = shaderLink;
      ed::Link(nextShaderLinkId, shaderLink->output->outputId,
               shaderLink->input->auxId);
    }
    
    // [Aux] Create a Link from Source -> Shader
    if (shaderNode->shader->supportsAux() &&
        shaderNode->shader->sourceAux != nullptr) {
      std::shared_ptr<Node> auxNode = idNodeMap[shaderNode->shader->sourceAux->id];
      ed::LinkId nextShaderLinkId = uniqueId++;
      shaderNode->auxNode = auxNode;
      auto shaderLink =
      std::make_shared<ShaderLink>(nextShaderLinkId, shaderNode, auxNode);
      shaderLink->type = LinkTypeAux;
      linksMap[nextShaderLinkId.Get()] = shaderLink;
      ed::Link(nextShaderLinkId, shaderLink->output->outputId,
               shaderLink->input->auxId);
    }
  }
  
  std::vector<ed::LinkId> selectedLinksIds;
  std::vector<ed::NodeId> selectedNodesIds;
  selectedLinksIds.resize(ed::GetSelectedObjectCount());
  selectedNodesIds.resize(ed::GetSelectedObjectCount());
  
  // Delete the selected Links
  ed::GetSelectedLinks(selectedLinksIds.data(), (int) selectedLinksIds.size());
  for (auto selectedLinkId : selectedLinksIds) {
    // Remove the Shader link
    std::shared_ptr<ShaderLink> link = linksMap[selectedLinkId.Get()];
    if (shouldDelete && link != nullptr) {
      switch (link->type) {
        case LinkTypeSource: {
          auto chainer =
          ShaderChainerService::getService()->shaderChainerForShaderId(
                                                                       link->output->shader->id());
          ShaderChainerService::getService()->breakShaderChainerFront(chainer);
          break;
        }
        case LinkTypeAux: {
          // [Source] -> [Aux]
          if (link->output->type == NodeTypeSource) {
            ShaderChainerService::getService()->breakSourceShaderAuxLink(
                                                                         link->output->source, link->input->shader);
          } else {
            ShaderChainerService::getService()->breakShaderAuxLink(
                                                                   link->output->shader, link->input->shader);
          }
          break;
        }
        case LinkTypeShader: {
          ShaderChainerService::getService()->breakShaderNextLink(
                                                                  link->output->shader);
          break;
        }
      }
    }
  }
  
  // Delete the selected Shader
  
  ed::GetSelectedNodes(selectedNodesIds.data(), (int) selectedNodesIds.size());
  for (auto selectedNodeId : selectedNodesIds) {
    std::shared_ptr<Node> node = nodeIdNodeMap[selectedNodeId.Get()];
    if (node == nullptr) {
      nodeIdNodeMap.erase(selectedNodeId.Get());
      continue;
    }
    
    if (node->id == selectedNodeId) {
      if (shouldDelete)
        handleDeleteNode(node);
      else
        ShaderChainerService::getService()->selectShader(node->shader);
    }
  }
  
  if (ed::BeginCreate()) {
    queryNewLinks();
  }
  
  if (firstFrame) {
    ed::NavigateToContent();
    firstFrame = false;
  }
  
  handleUnplacedNodes();
  handleRightClick();
  handleDoubleClick();
  
  
  ed::EndCreate();
  ed::End();
  
  drawNodeWindows();
  
  drawUploadChainerWindow();
  
  drawResolutionPopup();
  
  handleDropZone();
  
  ed::SetCurrentEditor(nullptr);
  
  shouldDelete = false;
}

void NodeLayoutView::drawNode(std::shared_ptr<Node> node) {
  ed::NodeId nodeId = node->id;
  ed::PushStyleColor(ax::NodeEditor::StyleColor_NodeBg, node->nodeColor());
  ed::BeginNode(node->id);
  
  // ----- FIRST COLUMN ------
  ImGuiEx_BeginColumn();
  
  // Draw delete button
  if (CommonViews::IconButton(ICON_MD_DELETE, node->idName())) {
    handleDeleteNode(node);
  }
  
  CommonViews::sSpacing();
  
  if (node->supportsInput()) {
    ed::PinId inputPinId = node->inputId;
    ed::BeginPin(inputPinId, ed::PinKind::Input);
    
    auto icon = node->hasInputLink() ? ICON_MD_RADIO_BUTTON_ON : ICON_MD_RADIO_BUTTON_OFF;
    CommonViews::IconTitle(icon);
    
    ed::EndPin();
    auto inPin = std::make_shared<Pin>(inputPinId, node, PinTypeInput);
    pinIdPinMap[inputPinId.Get()] = inPin;
    pinIdNodeMap[inputPinId.Get()] = node;
  }
  
  
  // Create an Aux pin if the Shader supports it
  if (node->supportsAux()) {
    ed::PinId auxPinId = node->auxId;
    
    ed::BeginPin(auxPinId, ed::PinKind::Input);
    auto icon = node->hasAuxLink() ? ICON_MD_RADIO_BUTTON_ON : ICON_MD_RADIO_BUTTON_OFF;
    CommonViews::sSpacing();
    CommonViews::IconTitle(icon);
    ImGui::PushFont(FontService::getService()->sm);
    ImGui::Text("(Aux)");
    ImGui::PopFont();
    ed::EndPin();
    
    auto auxPin = std::make_shared<Pin>(auxPinId, node, PinTypeAux);
    
    pinIdNodeMap[auxPinId.Get()] = node;
    pinIdPinMap[auxPinId.Get()] = auxPin;
  }
  ImGuiEx_NextColumn();
  
  // ----- SECOND COLUMN ------
  CommonViews::mSpacing();
  ImGui::PushFont(FontService::getService()->h3);
  ImGui::Text("%s", node->name.c_str());
  ImGui::PopFont();
  CommonViews::mSpacing();
  ImGuiEx_NextColumn();
  
  // ----- THIRD COLUMN ------
  auto name = node->idName();
  // Draw settings button
  if (CommonViews::IconButton(ICON_MD_SETTINGS, node->idName())) {
    bool inSet = nodesToOpen.find(node) != nodesToOpen.end();
    if (!inSet) {
      nodesToOpen.insert(node);
    } else {
      nodesToOpen.erase(node);
    }
  }
  
  CommonViews::sSpacing();
  
  ed::PinId outputPinId = node->outputId;
  
  // Output Pin
  ed::BeginPin(outputPinId, ed::PinKind::Output);
  auto icon = node->hasOutputLink() ? ICON_MD_PLAY_CIRCLE_FILLED : ICON_MD_PLAY_CIRCLE_OUTLINE;
  CommonViews::IconTitle(icon);
  ed::EndPin();
  auto outPin = std::make_shared<Pin>(outputPinId, node, PinTypeOutput);
  pinIdPinMap[outputPinId.Get()] = outPin;
  pinIdNodeMap[outputPinId.Get()] = node;
  
  // Resolution Selector (if Source)
  CommonViews::sSpacing();
  if (node->type == NodeTypeSource && CommonViews::IconButton(ICON_MD_MONITOR, node->idName())) {
    resolutionPopupNode = node;
    ed::Suspend();
    resolutionPopupLocation = ImGui::GetMousePos();
    ed::Resume();
  }
  
  ImGuiEx_EndColumn();
  ed::EndNode();
}

// Returns or creates a Node with the id of either the Shader or Source passed.
std::shared_ptr<Node> NodeLayoutView::nodeForShaderSourceId(std::string shaderSourceId, NodeType nodeType, std::string name, bool supportsAux) {
  std::shared_ptr<Node> node = idNodeMap[shaderSourceId];
  if (node != nullptr) {
    nodeIdNodeMap[node->id.Get()] = node;
    idNodeMap[shaderSourceId] = node;
    return node;
  }
  auto nodeId = nodeIdTicker++;
  auto outputId = nodeIdTicker++;
  auto inputId = nodeType == NodeTypeShader ? nodeIdTicker++ : NullId;
  auto auxId = supportsAux ? nodeIdTicker++ : NullId;
  node = std::make_shared<Node>(nodeId, outputId, inputId, name, nodeType);
  node->auxId = auxId;
  idNodeMap[shaderSourceId] = node;
  nodeIdNodeMap[nodeId] = node;
  return node;
}

void NodeLayoutView::keyReleased(int key) {
  if (key == OF_KEY_BACKSPACE) {
    shouldDelete = true;
  }
}

void NodeLayoutView::handleUnplacedNodes() {
  if (nodeDropLocation == nullptr) {
    auto canvasPos = ed::ScreenToCanvas(ImVec2(ofGetMouseX(), ofGetMouseY()));
    nodeDropLocation = std::make_unique<ImVec2>(canvasPos);
  }
  
  for (auto id : unplacedNodeIds) {
    auto node = idNodeMap[id];
    if (nodeDropLocation != nullptr) {
      ed::SetNodePosition(node.get()->id,
                          *nodeDropLocation.get());
      nodeDropLocation = std::make_unique<ImVec2>(nodeDropLocation->x - 200., nodeDropLocation->y);
    }
  }
  
  unplacedNodeIds = {};
}

void NodeLayoutView::handleDoubleClick() {
  auto edNode = ed::GetDoubleClickedNode();
  
  if (edNode) {
    long id = edNode.Get();
    // Check if the node is in our map
    if (nodeIdNodeMap.find(id) != nodeIdNodeMap.end()) {
      auto node = nodeIdNodeMap[id];
      selectChainer(node);
    }
  }
}

void NodeLayoutView::selectChainer(std::shared_ptr<Node> node) {
  if (node->type == NodeTypeShader) {
    //    ed::SelectNode(node->id, true);
    // Traverse parent direction
    auto child = node->shader;
    while (child->parent != nullptr) {
      child = child->parent;
      auto toSelect = idNodeMap[child->id()];
      ed::SelectNode(idNodeMap[child->id()]->id, true);
    }
    
    auto parent = node->shader;
    while (parent->next != nullptr) {
      parent = parent->next;
      ed::SelectNode(idNodeMap[parent->id()]->id, true);
    }
  }
  
  if (node->type == NodeTypeSource) {
    auto source = node->source;
    if (source == nullptr) return;
    
    auto chainer = ShaderChainerService::getService()->shaderChainerForVideoSourceId(source->id);
    if (chainer == nullptr) return;
    auto front = chainer->front;
    
    while (front != nullptr) {
      ed::SelectNode(idNodeMap[front->id()]->id, true);
      front = front->next;
    }
  }
}

void NodeLayoutView::handleRightClick() {
  ed::Suspend();
  if (ed::ShowNodeContextMenu(&contextMenuNodeId)) {
    
    ImGui::OpenPopup("Node");
    contextMenuLocation = ImGui::GetMousePos();
  }
  
  ImGui::SetNextWindowPos(contextMenuLocation);
  if (ImGui::BeginPopup("Node")) {
    auto node = nodeIdNodeMap[contextMenuNodeId.Get()];
    if (ImGui::MenuItem("Save Chain")) {
      handleSaveNode(node);
    }
    if (ImGui::MenuItem("Delete Node")) {
      handleDeleteNode(node);
    }
    if (ImGui::MenuItem("Debug")) {
      debug();
    }
    if (ImGui::MenuItem("Upload Chain")) {
      handleUploadChain(node);
    }
    ImGui::EndPopup();
  }
  ed::Resume();
}

void NodeLayoutView::handleUploadChain(std::shared_ptr<Node> node) {
  uploadChainer = node->chainer;
}

void NodeLayoutView::handleDeleteNode(std::shared_ptr<Node> node) {
  switch (node->type) {
    case NodeTypeShader:
      ShaderChainerService::getService()->removeShader(node->shader);
      break;
    case NodeTypeSource:
      VideoSourceService::getService()->removeVideoSource(node->source->id);
      ShaderChainerService::getService()->removeShaderChainer(node->chainer->chainerId);
      break;
  }
  nodesToOpen.erase(node);
}

void NodeLayoutView::handleSaveNode(std::shared_ptr<Node> node) {
  std::shared_ptr<ShaderChainer> shaderChainer;
  
  if (node->type == NodeTypeShader) {
    shaderChainer = ShaderChainerService::getService()->shaderChainerForShaderId(node->shader->id());
  } else if (node->type == NodeTypeSource) {
    shaderChainer = ShaderChainerService::getService()->shaderChainerForVideoSourceId(node->source->id);
  }
  
  if (shaderChainer == nullptr) return;
  
  
  // Present a file dialog to save the config file
  // Use a default name of "CURRENT_DATE_TIME.json"
  std::string defaultName =
  ofGetTimestampString("%Y-%m-%d_%H-%M-%S.json");
  defaultName = formatString("%s_%s", shaderChainer->name.c_str(), defaultName.c_str());
  
  ConfigService::getService()->saveShaderChainerConfigFile(shaderChainer, defaultName);
}

void NodeLayoutView::handleDropZone() {
  if (ImGui::BeginDragDropTarget()) {
    // Shader Drop Zone
    if (const ImGuiPayload *payload =
        ImGui::AcceptDragDropPayload("NewShader")) {
      int n = *(const int *)payload->Data;
      ShaderType shaderType = (ShaderType)n;
      auto shader = ShaderChainerService::getService()->makeShader(shaderType);
      auto canvasPos = ed::ScreenToCanvas(ImVec2(ofGetMouseX(), ofGetMouseY()));
      nodeDropLocation = std::make_unique<ImVec2>(canvasPos);
      unplacedNodeIds.push_back(shader->id());
    }
    
    // VideoSource Drop Zone
    if (const ImGuiPayload *payload =
        ImGui::AcceptDragDropPayload("VideoSource")) {
      AvailableVideoSource availableSource =
      *(const AvailableVideoSource *)payload->Data;
      VideoSourceType sourceType = availableSource.type;
      
      switch (sourceType) {
        case VideoSource_webcam: {
          auto source = VideoSourceService::getService()->addWebcamVideoSource(
                                                                               availableSource.sourceName, availableSource.webcamId);
          ShaderChainerService::getService()->addNewShaderChainer(source);
          unplacedNodeIds.push_back(source->id);
          break;
        }
        case VideoSource_file: {
          auto source = VideoSourceService::getService()->addFileVideoSource(
                                                                             availableSource.sourceName, availableSource.path);
          ShaderChainerService::getService()->addNewShaderChainer(source);
          unplacedNodeIds.push_back(source->id);
          break;
        }
        case VideoSource_chainer:
          // Skip
          break;
        case VideoSource_image:
          // Skip
          break;
        case VideoSource_text: {
          auto source = VideoSourceService::getService()->addTextVideoSource(availableSource.sourceName);
          ShaderChainerService::getService()->addNewShaderChainer(source);
          unplacedNodeIds.push_back(source->id);
          break;
        }
          
        case VideoSource_shader: {
          auto source = VideoSourceService::getService()->addShaderVideoSource(
                                                                               availableSource.shaderType);
          ShaderChainerService::getService()->addNewShaderChainer(source);
          unplacedNodeIds.push_back(source->id);
          break;
        }
      }
      
      auto canvasPos = ed::ScreenToCanvas(ImVec2(ofGetMouseX(), ofGetMouseY()));
      nodeDropLocation = std::make_unique<ImVec2>(canvasPos);
    }
    
    // Available Shader Chainer drop zone
    if (const ImGuiPayload *payload =
        ImGui::AcceptDragDropPayload("AvailableShaderChainer")) {
      AvailableShaderChainer availableShaderChainer =
      *(const AvailableShaderChainer *)payload->Data;
      
      
      auto newNodes = ShaderChainerService::getService()->loadAvailableShaderChainer(availableShaderChainer);
      
      unplacedNodeIds.insert(unplacedNodeIds.end(), newNodes.begin(), newNodes.end());
      
      auto canvasPos = ed::ScreenToCanvas(ImVec2(ofGetMouseX(), ofGetMouseY()));
      nodeDropLocation = std::make_unique<ImVec2>(canvasPos);
    }
    
    ImGui::EndDragDropTarget();
  }
}

void NodeLayoutView::handleDroppedSource(std::shared_ptr<VideoSource> source) {
  unplacedNodeIds.push_back(source->id);
}

void NodeLayoutView::queryNewLinks() {
  ed::PinId inputPinId, outputPinId;
  std::vector<ed::NodeId> selectedNodesIds;
  
  if (ed::QueryNewLink(&outputPinId, &inputPinId)) {
    if (inputPinId && outputPinId) // both are valid, let's accept link
    {
      std::shared_ptr<Node> sourceNode = pinIdNodeMap[outputPinId.Get()];
      std::shared_ptr<Node> destNode = pinIdNodeMap[inputPinId.Get()];
      
      if (sourceNode == nullptr || destNode == nullptr || sourceNode == destNode)
        return;
      
      // We have a Shader destination
      if (destNode->type == NodeTypeShader) {
        if (ed::AcceptNewItem()) {
          
          // VideoSource is our Source
          if (sourceNode->type == NodeTypeSource) {
            auto destPin = pinIdPinMap[inputPinId.Get()];
            // If our Dest Pin is an Aux Pin
            if (destPin->pinType == PinTypeAux) {
              ShaderChainerService::getService()->setAuxShader(sourceNode->source, destNode->shader);
            } else if (destPin->pinType == PinTypeInput) {
              ShaderChainerService::getService()->addShaderToFront(
                                                                   destNode->shader, sourceNode->chainer);
            }
          }
          
          // Shader is our Source
          if (sourceNode->type == NodeTypeShader) {
            auto destPin = pinIdPinMap[inputPinId.Get()];
            // If our Dest Pin is an Aux Pin
            if (destPin->pinType == PinTypeAux) {
              ShaderChainerService::getService()->setAuxShader(
                                                               sourceNode->shader, destNode->shader);
              // Our Dest Pin is a regular Input
            } else if (destPin->pinType == PinTypeInput) {
              ShaderChainerService::getService()->linkShaderToNext(
                                                                   destNode->shader, sourceNode->shader);
            }
          }
        }
      }
    }
  }
}

void NodeLayoutView::drawUploadChainerWindow() {
  if (uploadChainer != nullptr && uploadChainer->name.length()) {
    UploadChainerView::Draw(uploadChainer);
  }
}

void NodeLayoutView::drawNodeWindows() {
  if (nodesToOpen.size() == 0) return;
  
  for (auto const &node : nodesToOpen) {
    auto pos = ed::GetNodePosition(node->id);
    pos = ed::CanvasToScreen(pos);
    pos.x = pos.x + ed::GetNodeSize(node->id).x + 20;
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowCollapsed(false);
    
    
    auto style = ImGui::GetStyle();
    style.WindowMenuButtonPosition = ImGuiDir_None;
    auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10., 10.));
    if (ImGui::Begin(node->idName().c_str(), 0, flags)) {
      node->drawSettings();
    }
    ImGui::End();
    ImGui::PopStyleVar();
  }
}

void NodeLayoutView::drawResolutionPopup() {
  if (resolutionPopupNode != nullptr) {
    auto id = resolutionPopupNode->idName();
    
    // Check if we opened
    bool popupOpen = ImGui::IsPopupOpen(id.c_str(), ImGuiPopupFlags_AnyPopupId);
    if (!popupOpen && popupLaunched)
    {
      resolutionPopupNode = nullptr;
      popupLaunched = false;
      return;
    }
    
    ImGui::SetNextWindowPos(resolutionPopupLocation);
    ImGui::SetNextWindowCollapsed(false);
    
    ImGui::OpenPopup(id.c_str());
    if (ImGui::BeginPopup(id.c_str())) {
      CommonViews::ResolutionSelector(resolutionPopupNode->source);
      ImGui::EndPopup();
      popupLaunched = true;
    }
  }
}

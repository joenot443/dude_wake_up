//
//  NodeLayoutView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/5/23.
//

#include "NodeLayoutView.hpp"
#include "FontService.hpp"
#include "ImGuiExtensions.hpp"
#include "HelpService.hpp"
#include "LibraryService.hpp"
#include "MarkdownView.hpp"
#include "Fonts.hpp"
#include "VideoRecorder.hpp"
#include "UploadChainerView.h"
#include "LayoutStateService.hpp"
#include "ShaderChainerService.hpp"
#include "Strings.hpp"
#include "ConfigService.hpp"
#include "VideoSourceService.hpp"
#include "StrandService.hpp"
#include "Colors.hpp"
#include "ofMain.h"
#include <imgui.h>
#include <future>
#include <imgui_node_editor.h>
#include <imgui_node_editor_internal.h>

static const float ImGuiWindowTitleBarHeight = 70.0f;
static const float NodeWidth = 100.0f;
static const float NodeSettingsWidth = 350.0f;

namespace ed = ax::NodeEditor;

void NodeLayoutView::setup()
{
  nodes.clear();
  idNodeMap.clear();
  previewWindowNodes.clear();
  config = new ed::Config;
  config->SettingsFile = "NodeLayout.json";
  ImVector<float> zooms = ImVector<float>();
  //  zooms.push_back(1.0f);
  //  config->CustomZoomLevels = zooms;
  
  context = ed::CreateEditor(config);
  ofAddListener(LibraryService::getService()->downloadNotification, this, &NodeLayoutView::haveDownloadedAvailableLibraryFile);
}

void NodeLayoutView::update() {}
void ImGuiEx_BeginColumn() { ImGui::BeginGroup(); }

void ImGuiEx_NextColumn()
{
  ImGui::EndGroup();
  ImGui::SameLine();
  ImGui::BeginGroup();
}

void ImGuiEx_EndColumn() { ImGui::EndGroup(); }

void NodeLayoutView::debug()
{
  auto shaders = ShaderChainerService::getService()->shaders();
  auto connections = ShaderChainerService::getService()->connections();
  auto sources = VideoSourceService::getService()->videoSources();
  
  log("==========");
  for (auto s : shaders)
  {
    log("%s - %s. Ins: %d, Outs: %d", s->name().c_str(), s->connId().c_str(), s->inputs.size(), s->outputs.size());
  }
  
  for (auto s : sources)
  {
    log("%s - %s. Ins: %d, Outs: %d", s->sourceName.c_str(), s->connId().c_str(), s->inputs.size(), s->outputs.size());
  }
  
  for (auto c : connections)
  {
    log("%s - %s", c->start->connId().c_str(), c->end->connId().c_str());
  }
  
  log("==========");
}

void NodeLayoutView::draw()
{
  auto &io = ImGui::GetIO();
  
  auto sources = VideoSourceService::getService()->videoSources();
  auto shaders = ShaderChainerService::getService()->shaders();
  
  nodes = {};
  
  ed::SetCurrentEditor(context);
  ed::PushStyleColor(ax::NodeEditor::StyleColor_Bg, ImVec4(0, 0, 0, 0));
  ed::PushStyleColor(ax::NodeEditor::StyleColor_Grid, ImVec4(0, 0, 0, 0));
  
  bool showAudio = LayoutStateService::getService()->showAudioSettings;
  
  float width = (ImGui::GetWindowContentRegionMax().x * 4.0) / 5;
  float height = ImGui::GetWindowContentRegionMax().y - LayoutStateService::getService()->audioSettingsViewHeight() - 56.0f;
  
  ed::Begin("My Editor", ImVec2(width, height));
  ed::NavigateToContent();
  
  // Disable navigation if Tutorial is enabled
  //  if (LayoutStateService::getService()->helpEnabled) {
  //    ed::SuspendNavigation();
  //  } else {
  //    ed::ResumeNavigation();
  //  }
  
  ed::PushStyleVar(ed::StyleVar_ScrollDuration, 0.1f);
  ed::PushStyleVar(ax::NodeEditor::StyleVar_NodeRounding, 0.0);
  ed::PushStyleVar(ax::NodeEditor::StyleVar_NodeBorderWidth, 1.0);
  ed::PushStyleColor(ax::NodeEditor::StyleColor_NodeBorder, ImVec4(1.0, 1.0, 1.0, 1.0));
  int uniqueId = 1;
  int linkUniqueId = 1000;
  int sourceUniqueId = 10000;
  makingLink = false;
  linksMap.clear();
  nodeIdNodeMap.clear();
  pinIdPinMap.clear();
  pinIdNodeMap.clear();
  terminalNodes.clear();
  
  // Loop over Shaders, creating a ShaderNode for each
  for (std::shared_ptr<Shader> shader : shaders)
  {
    auto shaderNode = nodeForShaderSourceId(shader->shaderId, NodeTypeShader, shader->name(), shader);
    shaderNode->shader = shader;
    drawNode(shaderNode);
    
    nodes.insert(shaderNode);
    
    // Check if this is the terminal node, in which case, we'll add it to the preview nodes.
    if (ShaderChainerService::getService()->isTerminalShader(shaderNode->shader) &&
        ShaderChainerService::getService()->hasParents(shaderNode->shader))
    {
      terminalNodes.insert(shaderNode);
    }
  }
  
  // Create VideoSource Nodes
  for (std::shared_ptr<VideoSource> source : sources)
  {
    // Create a Link to the front shader, if it exists
    // Make source node
    std::shared_ptr<Connectable> conn = source;
    
    auto sourceNode = nodeForShaderSourceId(source->id, NodeTypeSource, source->sourceName, conn);
    sourceNode->source = source;
    drawNode(sourceNode);
    
    // Always draw a Preview for our Source
    terminalNodes.insert(sourceNode);
    
    nodes.insert(sourceNode);
  }
  
  // Loop over ShaderNodes and draw links
  
  for (auto const &x : nodeIdNodeMap)
  {
    auto shaderNode = x.second;
    
    // Create Links for every Output. [Can go to Mask, Aux, etc.]
    if (!shaderNode->connectable->outputs.empty())
    {
      for (auto pair : shaderNode->connectable->outputs)
      {
        auto slot = pair.first;
        auto connection = pair.second;
        std::shared_ptr<Node> nextNode = idNodeMap[connection->end->connId()];
        std::shared_ptr<Node> startNode = idNodeMap[connection->start->connId()];
        
        ed::LinkId nextShaderLinkId = uniqueId++;
        
        auto shaderLink =
        std::make_shared<ShaderLink>(nextShaderLinkId, nextNode, shaderNode, connection->outputSlot, connection->inputSlot, connection->id);
        linksMap[nextShaderLinkId.Get()] = shaderLink;
        ed::PinId inputPinId = 0;
        ed::PinId outputPinId = 0;
        outputPinId = startNode->outputIds[connection->outputSlot];
        inputPinId = nextNode->inputIds[connection->inputSlot];
        
        ed::Link(nextShaderLinkId,
                 outputPinId,
                 inputPinId);
      }
    }
  }
  
  std::vector<ed::LinkId> selectedLinksIds;
  std::vector<ed::NodeId> selectedNodesIds;
  selectedLinksIds.resize(ed::GetSelectedObjectCount());
  selectedNodesIds.resize(ed::GetSelectedObjectCount());
  
  // Delete the selected Links
  ed::GetSelectedLinks(selectedLinksIds.data(), (int)selectedLinksIds.size());
  for (auto selectedLinkId : selectedLinksIds)
  {
    // Remove the Shader link
    std::shared_ptr<ShaderLink> link = linksMap[selectedLinkId.Get()];
    if (shouldDelete && link != nullptr)
    {
      ShaderChainerService::getService()->breakConnectionForConnectionId(link->connectionId);
    }
  }
  
  // Selects or deletes shaders which are selected
  
  ed::GetSelectedNodes(selectedNodesIds.data(), (int)selectedNodesIds.size());
  for (auto selectedNodeId : selectedNodesIds)
  {
    std::shared_ptr<Node> node = nodeIdNodeMap[selectedNodeId.Get()];
    if (node == nullptr)
    {
      nodeIdNodeMap.erase(selectedNodeId.Get());
      continue;
    }
    
    if (node->id == selectedNodeId)
    {
      if (shouldDelete)
        handleDeleteNode(node);
      else
        ShaderChainerService::getService()->selectConnectable(node->connectable);
    }
  }
  
  if (selectedNodesIds.empty()) {
    ShaderChainerService::getService()->deselectConnectable();
  }
  
  if (ed::BeginCreate())
  {
    queryNewLinks();
  }
  
  if (firstFrame)
  {
    ed::NavigateToContent();
    firstFrame = false;
  }
  
  handleUnplacedNodes();
  handleRightClick();
  handleDoubleClick();
  handleUnplacedDownloadedLibraryFile();
  
  ed::EndCreate();
  
  populateNodePositions();
  
  ed::End();
  
  drawNodeWindows();
  
  drawUploadChainerWindow();
  
  drawResolutionPopup();
  
  handleDropZone();
  
  drawPreviewWindows();
  
  drawHelp();
  
  ed::SetCurrentEditor(nullptr);
  
  drawActionButtons();
  
  drawSaveDialog();
  
  //  drawMetrics();
  
  shouldDelete = false;
}

void NodeLayoutView::drawNode(std::shared_ptr<Node> node)
{
  ed::NodeId nodeId = node->id;
  ed::PushStyleColor(ax::NodeEditor::StyleColor_NodeBg, node->nodeColor());
  ed::BeginNode(node->id);
  
  // ----- FIRST COLUMN ------
  ImGuiEx_BeginColumn();
  
  // Draw delete button
  if (CommonViews::IconButton(ICON_MD_DELETE, node->idName()))
  {
    handleDeleteNode(node);
  }
  
  CommonViews::sSpacing();
  
  // Input Pin
  for (int i = 0; i < node->connectable->inputCount(); i++) {
    InputSlot slot = static_cast<InputSlot>(i);
    ed::PinId inputPinId = node->inputIds[slot];
    ed::BeginPin(inputPinId, ed::PinKind::Input);
    
    auto icon = node->hasInputLinkAt(slot) ? ICON_MD_RADIO_BUTTON_ON : ICON_MD_RADIO_BUTTON_OFF;
    
    CommonViews::IconTitle(icon);
    
    ed::EndPin();
    auto inPin = std::make_shared<Pin>(inputPinId, node, PinTypeInput);
    pinIdPinMap[inputPinId.Get()] = inPin;
    pinIdNodeMap[inputPinId.Get()] = node;
  }
  
  // Save Button
  if (node->type == NodeTypeSource)
  {
    CommonViews::Spacing(18);
    if (CommonViews::IconButton(ICON_MD_SAVE, node->idName()))
    {
      handleSaveNode(node);
    }
  }
  
  ImGuiEx_NextColumn();
  
  // ----- SECOND COLUMN ------
  
  // Draw preview button
  bool inSet = previewWindowNodes.find(node) != previewWindowNodes.end();
  auto visIcon = inSet ? ICON_MD_VISIBILITY_OFF : ICON_MD_VISIBILITY;
  
  // Draw a preview button (eye) if we are NOT the terminal node
  if (node->type == NodeTypeShader &&
      !ShaderChainerService::getService()->isTerminalShader(node->shader) &&
      ShaderChainerService::getService()->hasParents(node->shader) &&
      CommonViews::IconButton(visIcon, node->idName()))
  {
    bool inSet = previewWindowNodes.find(node) != previewWindowNodes.end();
    if (!inSet)
    {
      previewWindowNodes.insert(node);
    }
    else
    {
      previewWindowNodes.erase(node);
    }
    ImGui::SameLine();
  }
  
  // Draw launch window (rect) button, but only for the terminal shader
  else if ((node->type == NodeTypeShader && ShaderChainerService::getService()->isTerminalShader(node->shader))
           || node->type == NodeTypeSource)
  {
    if (CommonViews::IconButton(ICON_MD_FULLSCREEN, node->idName()))
    {
      VideoSourceService::getService()->addOutputWindow(node->connectable);
    }
  }
  
  CommonViews::sSpacing();
  
  // Node Title
  ImGui::PushFont(FontService::getService()->h3);
  ImGui::Text("%s", truncateString(formatString("%s", node->name.c_str()), 20).c_str());
  ImGui::PopFont();
  
  CommonViews::sSpacing();
  
  // Draw Aux pin if Feedback
  if (node->type == NodeTypeShader && node->shader->type() == ShaderTypeFeedback) {
    // Get secondary Output Pin
    ed::PinId outputPinId = node->outputIds[OutputSlotAux];
    
    // Output Pin
    ed::BeginPin(outputPinId, ed::PinKind::Output);
    auto icon = node->hasOutputLinkAt(OutputSlotAux) ? ICON_MD_PLAY_CIRCLE_FILLED : ICON_MD_PLAY_CIRCLE_OUTLINE;
    CommonViews::IconTitle(icon);
    ed::EndPin();
    auto outPin = std::make_shared<Pin>(outputPinId, node, PinTypeOutput);
    pinIdPinMap[outputPinId.Get()] = outPin;
    pinIdNodeMap[outputPinId.Get()] = node;
  }
  
  // Draw an Audio Reactivity button if we have one
  else if (node->type == NodeTypeShader && node->shader->settings->audioReactiveParameter != nullptr) {
    
    
    // Draw the disable button if we're already driving
    if (node->shader->settings->audioReactiveParameter->driver != nullptr) {
      
      if (CommonViews::IconButton(ICON_MD_MUSIC_OFF, formatString("##disableAudio%s", node->shader->shaderId.c_str()))) {
        node->shader->settings->audioReactiveParameter->removeDriver();
      }
    }
    // Otherwise drive the default audio driver button
    else if (CommonViews::IconButton(ICON_MD_MUSIC_NOTE, formatString("##enableAudio%s", node->shader->shaderId.c_str()))) {
      
      // Prefer to use the BPM param, use RMS is BPM isn't enabled
      auto analysisParameters = AudioSourceService::getService()->selectedAudioSource->audioAnalysis.parameters;
      auto bpmEnabled = AudioSourceService::getService()->selectedAudioSource->audioAnalysis.bpmEnabled;
      
      std::shared_ptr<Parameter> updateParameter = nullptr;
      
      for (auto param : analysisParameters) {
        if (bpmEnabled && param->name == "BPM") {
          updateParameter = param;
          break;
        } else if (!bpmEnabled && param->name == "Loudness") {
          updateParameter = param;
          break;
        }
      }
      
      if (updateParameter == nullptr) return;
      
      // Successfully assigned driver, begin Audio Analysis if it's not runnig
      if (AudioSourceService::getService()->selectedAudioSource != nullptr &&
          !AudioSourceService::getService()->selectedAudioSource->active) {
        AudioSourceService::getService()->selectedAudioSource->toggle();
      }
      node->shader->settings->audioReactiveParameter->addDriver(updateParameter);
    }
  }
  
  
  
  ImGuiEx_NextColumn();
  
  // ----- THIRD COLUMN ------
  
  auto name = node->idName();
  // Draw settings button
  if (CommonViews::IconButton(ICON_MD_SETTINGS, node->idName()))
  {
    bool inSet = nodesToOpen.find(node) != nodesToOpen.end();
    if (!inSet)
    {
      nodesToOpen.insert(node);
    }
    else
    {
      nodesToOpen.erase(node);
    }
  }
  
  CommonViews::sSpacing();
  
  // Draw an Output Pin for each OutputSlot which is populated
  ImVec2 cursorPos = ImGui::GetCursorPos();
  
  std::vector<OutputSlot> slotsToDraw = node->connectable->populatedOutputSlots();
  // On top, draw an Output Pin for the next available Slot
  OutputSlot nextSlot = node->connectable->nextAvailableOutputSlot();
  if (nextSlot != OutputSlotNone) {
    slotsToDraw.push_back(nextSlot);
  }
  
  for (OutputSlot slot : slotsToDraw) {
    ed::PinId outputPinId = node->outputIds[slot];
    ImGui::SetCursorPos(cursorPos);
    // Draw an Output Pin for each OutputSlot which is linked
    ed::BeginPin(outputPinId, ed::PinKind::Output);
    auto icon = node->hasOutputLinkAt(slot) ? ICON_MD_PLAY_CIRCLE_FILLED : ICON_MD_PLAY_CIRCLE_OUTLINE;
    CommonViews::IconTitle(icon);
    ed::EndPin();
    auto outPin = std::make_shared<Pin>(outputPinId, node, PinTypeOutput);
    pinIdPinMap[outputPinId.Get()] = outPin;
    pinIdNodeMap[outputPinId.Get()] = node;
  }
  
  CommonViews::sSpacing();
  
  // Draw Star for selecting Stage Shader
  if (node->type == NodeTypeShader && ShaderChainerService::getService()->isTerminalShader(node->shader))
  {
    if (ParameterService::getService()->isShaderIdStage(node->shader->shaderId)) {
      if (CommonViews::IconButton(ICON_MD_STAR, node->idName())) {
        ParameterService::getService()->toggleStageShaderId(node->shader->shaderId);
      }
    } else {
      if (CommonViews::IconButton(ICON_MD_STAR_OUTLINE, node->idName())) {
        ParameterService::getService()->toggleStageShaderId(node->shader->shaderId);
      }
    }
  }
  
  else if (node->type == NodeTypeSource)
  {
    auto icon = node->source->active ? ICON_MD_PAUSE : ICON_MD_PLAY_ARROW;
    if (CommonViews::IconButton(icon, formatString("##%splayPause", node->source->id.c_str()))) {
      node->source->active = !node->source->active;
    }
  }
  
  ImGuiEx_EndColumn();
  ed::EndNode();
}

std::shared_ptr<Node> NodeLayoutView::nodeForShaderSourceId(std::string shaderSourceId)
{
  if (idNodeMap.count(shaderSourceId))
    return idNodeMap[shaderSourceId];
  return nullptr;
}

// Returns or creates a Node with the id of either the Shader or Source passed.
std::shared_ptr<Node> NodeLayoutView::nodeForShaderSourceId(std::string shaderSourceId, NodeType nodeType, std::string name, std::shared_ptr<Connectable> connectable)
{
  
  bool supportsAux = nodeType == NodeTypeShader && ShaderChainerService::getService()->shaderForId(shaderSourceId)->inputCount() > 1;
  bool supportsMask = nodeType == NodeTypeShader && ShaderChainerService::getService()->shaderForId(shaderSourceId)->inputCount() > 2;
  
  
  std::shared_ptr<Node> node = idNodeMap[shaderSourceId];
  
  // We already have a node, return it
  if (node != nullptr)
  {
    nodeIdNodeMap[node->id.Get()] = node;
    idNodeMap[shaderSourceId] = node;
    return node;
  }
  
  // We don't have a node, create one
  auto nodeId = nodeIdTicker++;
  auto outputId = nodeIdTicker++;
  std::map<InputSlot, ed::PinId> inputIds;
  std::map<OutputSlot, ed::PinId> outputIds;
  
  // Create 10 outputIds for each Node, even though most of them won't be used.
  for (int i = 0; i < 11; i++) {
    OutputSlot slot = static_cast<OutputSlot>(i);
    outputIds[slot] = nodeIdTicker++;
  }
  
  // Create 10 inputIds for each Node, even though most of them won't be used.
  for (int i = 0; i < 11; i++) {
    InputSlot slot = static_cast<InputSlot>(i);
    inputIds[slot] = nodeIdTicker++;
  }
  
  auto origin = ImVec2(0, 0);
  if (nodeType == NodeTypeSource)
  {
    origin = VideoSourceService::getService()->videoSourceForId(shaderSourceId)->origin;
  }
  else if (nodeType == NodeTypeShader)
  {
    origin = ImVec2(ShaderChainerService::getService()->shaderForId(shaderSourceId)->settings->x->value, ShaderChainerService::getService()->shaderForId(shaderSourceId)->settings->y->value);
  }
  
  node = std::make_shared<Node>(nodeId, outputIds, inputIds, name, nodeType, connectable);
  
  // If we're placing the node from a JSON, we have an origin.
  if (origin.x != 0.)
  {
    ed::SetNodePosition(nodeId, origin);
  }
  idNodeMap[shaderSourceId] = node;
  nodeIdNodeMap[nodeId] = node;
  return node;
}

void NodeLayoutView::keyReleased(int key)
{
  if (key == OF_KEY_BACKSPACE
      && !LayoutStateService::getService()->isEditingText)
  {
    shouldDelete = true;
  }
}

void NodeLayoutView::populateNodePositions()
{
  for (auto node : nodes)
  {
    node->position = ed::GetNodePosition(node->id);
  }
}

bool NodeLayoutView::pointIsWithinNode(ImVec2 position, std::shared_ptr<Node> node) {
  return position.x > node->position.x && position.x < node->position.x + 150 && position.y > node->position.y && position.y < node->position.y + 75;
}

std::shared_ptr<Node> NodeLayoutView::nodeAtPosition(ImVec2 position) {
  for (auto node : nodes) {
    if (pointIsWithinNode(position, node)) {
      return node;
    }
  }
  
  return nullptr;
}

ImVec2 NodeLayoutView::coordinatesForNode(std::string id)
{
  if (idNodeMap.count(id))
  {
    return ed::GetNodePosition(idNodeMap[id]->id);
  }
  return ImVec2(0, 0);
}

void NodeLayoutView::handleUnplacedNodes()
{
  if (nodeDropLocation == nullptr)
  {
    auto canvasPos = ed::ScreenToCanvas(getScaledMouseLocation());
    nodeDropLocation = std::make_unique<ImVec2>(canvasPos);
  }
  
  if (unplacedNodeIds.size() == 0) return;
  
  auto node = idNodeMap[unplacedNodeIds[unplacedNodeIds.size() - 1]];
  ImVec2 offset = *nodeDropLocation.get() - node->savedPosition();
  
  for (auto id : unplacedNodeIds)
  {
    auto node = idNodeMap[id];
    
    if (nodeDropLocation != nullptr)
    {
      ImVec2 nodePosition = node->savedPosition();
      ed::SetNodePosition(node.get()->id, nodePosition + offset);
    }
  }
  
  unplacedNodeIds = {};
}

void NodeLayoutView::handleDoubleClick()
{
  auto edNode = ed::GetDoubleClickedNode();
  
  if (edNode)
  {
    long id = edNode.Get();
    // Check if the node is in our map
    if (nodeIdNodeMap.find(id) != nodeIdNodeMap.end())
    {
      auto node = nodeIdNodeMap[id];
      selectChainer(node);
    }
  }
}

void NodeLayoutView::selectChainer(std::shared_ptr<Node> node)
{
}

void NodeLayoutView::handleRightClick()
{
  ed::Suspend();
  if (ed::ShowNodeContextMenu(&contextMenuNodeId))
  {
    
    ImGui::OpenPopup("Node");
    contextMenuLocation = ImGui::GetMousePos();
  }
  
  ImGui::SetNextWindowPos(contextMenuLocation);
  if (ImGui::BeginPopup("Node"))
  {
    auto node = nodeIdNodeMap[contextMenuNodeId.Get()];
    if (ImGui::MenuItem("Save Strand"))
    {
      handleSaveNode(node);
    }
    if (ImGui::MenuItem("Delete Node"))
    {
      handleDeleteNode(node);
    }
    if (ImGui::MenuItem("Debug"))
    {
      debug();
    }
    if (ImGui::MenuItem("Upload Chain"))
    {
      handleUploadChain(node);
    }
    ImGui::EndPopup();
  }
  ed::Resume();
}

void NodeLayoutView::handleUploadChain(std::shared_ptr<Node> node)
{
  //  uploadChainer = node->chainer;
}

void NodeLayoutView::handleDeleteNode(std::shared_ptr<Node> node)
{
  switch (node->type)
  {
    case NodeTypeShader:
      ShaderChainerService::getService()->removeShader(node->shader);
      break;
    case NodeTypeSource:
      VideoSourceService::getService()->removeVideoSource(node->source->id);
      break;
  }
  previewWindowNodes.erase(node);
  nodesToOpen.erase(node);
}

void NodeLayoutView::handleSaveNode(std::shared_ptr<Node> node)
{
  Strand strand = ShaderChainerService::getService()->strandForConnectable(node->connectable);
  
  // Generate default filename
  std::string defaultName = ofGetTimestampString("%m-%d");
  defaultName = formatString("%s_%s", strand.name.c_str(), defaultName.c_str());
  std::string defaultJsonName = formatString("%s.json", defaultName.c_str());
  
  // Copy the default name into the char array
  std::strncpy(saveFileName, defaultJsonName.c_str(), sizeof(saveFileName) - 1);
  saveFileName[sizeof(saveFileName) - 1] = '\0'; // Ensure null termination
  
  // Show the ImGui save dialog popup
  showSaveDialog = true;
  nodeToSave = node;
}

void NodeLayoutView::handleDropZone()
{
  if (ImGui::BeginDragDropTarget())
  {
    // Shader Drop Zone
    if (const ImGuiPayload *payload =
        ImGui::AcceptDragDropPayload("NewShader"))
    {
      int n = *(const int *)payload->Data;
      ShaderType shaderType = (ShaderType)n;
      auto shader = ShaderChainerService::getService()->makeShader(shaderType);
      auto canvasPos = ed::ScreenToCanvas(getScaledMouseLocation());
      auto hovered = ed::GetHoveredNode();
      
      
      if (hovered.Get() == 0) {
        nodeDropLocation = std::make_unique<ImVec2>(canvasPos);
      } else {
        // Dropped on an existing Shader
        nodeDropLocation = std::make_unique<ImVec2>(ed::GetNodePosition(hovered) + ImVec2(150.0, 0.0));
        auto startNode = nodeIdNodeMap[hovered.Get()];
        // Check if the existing Shader was already connected to a next node
        std::shared_ptr<Connectable> next = nullptr;
        if (startNode->connectable->hasOutputAtSlot(OutputSlotMain)) {
          auto conn = startNode->connectable->connectionAtSlot(OutputSlotMain);
          ShaderChainerService::getService()->insert(startNode->connectable, shader, OutputSlotMain);
          ShaderChainerService::getService()->breakConnectionForConnectionId(conn->id);
        } else {
          ShaderChainerService::getService()->makeConnection(startNode->connectable, shader, startNode->type == NodeTypeShader ? ConnectionTypeShader : ConnectionTypeSource, OutputSlotMain, InputSlotMain);
        }
        
      }
      unplacedNodeIds.push_back(shader->shaderId);
    }
    
    // VideoSource Drop Zone
    if (const ImGuiPayload *payload =
        ImGui::AcceptDragDropPayload("VideoSource"))
    {
      std::string id = *(const std::string *)payload->Data;
      std::shared_ptr<AvailableVideoSource> availableSource = VideoSourceService::getService()->availableVideoSourceForId(id);
      
      VideoSourceType sourceType = availableSource->type;
      std::shared_ptr<VideoSource> source = nullptr;
      
      switch (sourceType)
      {
        case VideoSource_webcam:
        {
          auto availableWebcamSource = std::dynamic_pointer_cast<AvailableVideoSourceWebcam>(availableSource);
          source = VideoSourceService::getService()->addWebcamVideoSource(
                                                                          availableWebcamSource->sourceName, availableWebcamSource->webcamId);
          unplacedNodeIds.push_back(source->id);
          break;
        }
        case VideoSource_file:
        {
          auto availableFileSource = std::dynamic_pointer_cast<AvailableVideoSourceFile>(availableSource);
          source = VideoSourceService::getService()->addFileVideoSource(
                                                                        availableFileSource->sourceName, availableFileSource->path);
          unplacedNodeIds.push_back(source->id);
          break;
        }
        case VideoSource_chainer:
          // Skip
          break;
        case VideoSource_image:
        {
          auto availableFileSource = std::dynamic_pointer_cast<AvailableVideoSourceImage>(availableSource);
          source = VideoSourceService::getService()->addImageVideoSource(
                                                                         availableFileSource->sourceName, availableFileSource->path);
          unplacedNodeIds.push_back(source->id);
          break;
        }
        case VideoSource_text:
        {
          source = VideoSourceService::getService()->addTextVideoSource(availableSource->sourceName);
          unplacedNodeIds.push_back(source->id);
          break;
        }
          
        case VideoSource_icon:
        {
          source = VideoSourceService::getService()->addIconVideoSource(availableSource->sourceName);
          unplacedNodeIds.push_back(source->id);
          break;
        }
          
        case VideoSource_shader:
        {
          auto availableShaderSource = std::dynamic_pointer_cast<AvailableVideoSourceShader>(availableSource);
          source = VideoSourceService::getService()->addShaderVideoSource(availableShaderSource->shaderType);
          unplacedNodeIds.push_back(source->id);
          break;
        }
        case VideoSource_library:
        {
          auto availableLibrarySource = std::dynamic_pointer_cast<AvailableVideoSourceLibrary>(availableSource);
          auto file = availableLibrarySource->libraryFile;
          
          if (file->isMediaDownloaded())
          {
            auto source = VideoSourceService::getService()->addFileVideoSource(
                                                                               file->name, file->videoPath());
            unplacedNodeIds.push_back(source->id);
            break;
          }
          else
          {
            LibraryService::getService()->downloadFutures.push_back(std::async(std::launch::async, &LibraryService::downloadFile, LibraryService::getService(), availableLibrarySource->libraryFile));
          }
          break;
        }
        case VideoSource_multi:
          source = VideoSourceService::getService()->addMultiVideoSource(availableSource->sourceName);
          unplacedNodeIds.push_back(source->id);
          break;
        case VideoSource_empty:
          break;
      }
      
      auto within = nodeAtPosition(ed::ScreenToCanvas(getScaledMouseLocation()));
      if (within != nullptr && within->type == NodeTypeSource && source != nullptr) {
        // Remove the pending source from unplaced
        unplacedNodeIds.pop_back();
        
        std::shared_ptr<MultiSource> multiSource = VideoSourceService::getService()->addMultiVideoSource(within->name);
        multiSource->addSource(source);
        multiSource->addSource(within->source);
        unplacedNodeIds.push_back(multiSource->id);
        
        ShaderChainerService::getService()->copyConnections(within->source, multiSource);
        VideoSourceService::getService()->removeVideoSource(within->source->id);
      }
      
      auto canvasPos = ed::ScreenToCanvas(getScaledMouseLocation());
      nodeDropLocation = std::make_unique<ImVec2>(canvasPos);
    }
    
    // Available Shader Chainer drop zone
    if (const ImGuiPayload *payload =
        ImGui::AcceptDragDropPayload("AvailableStrand"))
    {
      std::string id = *(const std::string *)payload->Data;
      std::shared_ptr<AvailableStrand> availableStrand = StrandService::getService()->availableStrandForId(id);
      
      if (availableStrand != nullptr)
      {
        loadStrand(availableStrand);
      }
    }
    
    ImGui::EndDragDropTarget();
  }
}

void NodeLayoutView::loadStrand(std::shared_ptr<AvailableStrand> availableStrand)
{
  std::vector<std::string> ids = ConfigService::getService()->loadStrandFile(availableStrand->path);
  
  
  if (ids.size() != 0) {
    auto canvasPos = ed::ScreenToCanvas(getScaledMouseLocation());
    nodeDropLocation = std::make_unique<ImVec2>(canvasPos);
    // Add all the node IDs generated in loading the Strand
    unplacedNodeIds.insert(unplacedNodeIds.end(), ids.begin(), ids.end());
  }
}

void NodeLayoutView::handleDroppedSource(std::shared_ptr<VideoSource> source)
{
  unplacedNodeIds.push_back(source->id);
  nodeDropLocation = std::make_unique<ImVec2>(getScaledMouseLocation());
}

void NodeLayoutView::haveDownloadedAvailableLibraryFile(LibraryFile &file)
{
  pendingFile = std::make_unique<LibraryFile>(file);
}

void NodeLayoutView::handleUnplacedDownloadedLibraryFile()
{
  if (pendingFile == nullptr)
  {
    return;
  }
  
  auto source = VideoSourceService::getService()->addFileVideoSource(
                                                                     pendingFile->name, pendingFile->videoPath());
  unplacedNodeIds.push_back(source->id);
  pendingFile = nullptr;
}

void NodeLayoutView::queryNewLinks()
{
  ed::PinId inputPinId, outputPinId;
  std::vector<ed::NodeId> selectedNodesIds;
  // Attempt to create a link with the passed input/output
  if (ed::QueryNewLink(&outputPinId, &inputPinId))
  {
    if (inputPinId && outputPinId) // both are valid, let's accept link
    {
      std::shared_ptr<Node> sourceNode = pinIdNodeMap[outputPinId.Get()];
      std::shared_ptr<Node> destNode = pinIdNodeMap[inputPinId.Get()];
      InputSlot inputSlot;
      OutputSlot outputSlot;
      
      if (sourceNode == nullptr && destNode == nullptr) { return; }
      
      // Get the InputSlot for this connection
      for (auto [slot, inputId] : destNode->inputIds) {
        if (inputId == inputPinId) inputSlot = slot;
      }
      
      // Get the OutputSlot for this connection
      for (auto [slot, outputId] : sourceNode->outputIds) {
        if (outputId == outputPinId) outputSlot = slot;
      }
      
      
      // We have a Shader destination
      if (destNode->type == NodeTypeShader)
      {
        if (ed::AcceptNewItem())
        {
          // VideoSource is our Source
          if (sourceNode->type == NodeTypeSource)
          {
            ShaderChainerService::getService()->makeConnection(sourceNode->source, destNode->shader, ConnectionTypeSource, outputSlot, inputSlot, true);
          }
          
          // Shader is our Source
          if (sourceNode->type == NodeTypeShader)
          {
            ShaderChainerService::getService()->makeConnection(
                                                               sourceNode->shader, destNode->shader,
                                                               ConnectionTypeShader, outputSlot, inputSlot, true);
          }
        }
      }
    }
  }
}

void NodeLayoutView::drawUploadChainerWindow()
{
  //  if (uploadChainer != nullptr && uploadChainer->name.length())
  //  {
  //    UploadChainerView::draw(uploadChainer);
  //  }
}

void NodeLayoutView::drawNodeWindows()
{
  auto cursor = ImGui::GetCursorPos();
  if (nodesToOpen.size() == 0)
    return;
  
  for (auto const &node : nodesToOpen)
  {
    ImVec2 pos = ed::GetNodePosition(node->id);
    pos = ed::CanvasToScreen(pos);
    pos.x = pos.x + ed::GetNodeSize(node->id).x / ed::GetCurrentZoom() + 20;
    auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize;
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowSizeConstraints(ImVec2(NodeSettingsWidth, 0.0), ImVec2(NodeSettingsWidth, 9999999.0));
    ImGui::Begin(node->idName().c_str(), 0, flags);
    node->drawSettings();
    ImGui::End();
  }
  ImGui::SetCursorPos(cursor);
}

void NodeLayoutView::drawPreviewWindows()
{
  auto start = ImGui::GetCursorPos();
  if (previewWindowNodes.size() == 0 && terminalNodes.size() == 0)
    return;
  
  for (auto const &node : previewWindowNodes)
  {
    drawPreviewWindow(node);
  }
  for (auto const &node : terminalNodes)
  {
    drawPreviewWindow(node);
  }
  ImGui::SetCursorPos(start);
}

void NodeLayoutView::drawPreviewWindow(std::shared_ptr<Node> node)
{
  auto sizeScale = ed::GetCurrentZoom();
  
  auto windowSize = ImVec2(160 / sizeScale, 90 / sizeScale);
  
  auto pos = ed::GetNodePosition(node->id);
  pos = ed::CanvasToScreen(pos);
  
  pos.x = pos.x + (ed::GetNodeSize(node->id).x / ed::GetCurrentZoom()) / 2 - 80 / sizeScale;
  ofRectangle previewRect = ofRectangle(pos.x, pos.y, windowSize.x, windowSize.y);
  pos.y = pos.y - 90 / sizeScale - 10;
  
  auto style = ImGui::GetStyle();
  
  ImGui::SetCursorPos(pos);
  if (sizeScale > 5.0)
  {
    sizeScale = 5.0;
  }
  node->drawPreview(pos, sizeScale);
}

void NodeLayoutView::drawResolutionPopup()
{
  if (resolutionPopupNode != nullptr)
  {
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
    if (ImGui::BeginPopup(id.c_str()))
    {
      CommonViews::ResolutionSelector(resolutionPopupNode->source);
      ImGui::EndPopup();
      popupLaunched = true;
    }
  }
}

void NodeLayoutView::drawActionButtons()
{
  auto pos = ImGui::GetCursorPos();
  int buttonCount = 4;
  
  // Set the cursor to the bottom right of the window
  float shaderInfoPaneWidth = LayoutStateService::getService()->shouldDrawShaderInfo() ? LayoutStateService::getService()->browserSize().x : 0;
  
  ImGui::SetCursorPos(ImVec2(getScaledWindowWidth() - 50.0 * buttonCount - shaderInfoPaneWidth, getScaledWindowHeight() - LayoutStateService::getService()->audioSettingsViewHeight() - 100.0));
  
  auto helpIcon = LayoutStateService::getService()->helpEnabled ?  ICON_MD_HELP_OUTLINE : ICON_MD_HELP;
  // Draw the help button
  if (CommonViews::LargeIconButton(helpIcon, "help"))
  {
    LayoutStateService::getService()->helpEnabled = !LayoutStateService::getService()->helpEnabled;
  }
  
  ImGui::SameLine();
  CommonViews::HSpacing(2);
  ImGui::SameLine();
  
  // Draw the reset button
  if (CommonViews::LargeIconButton(ICON_MD_DELETE_FOREVER, "reset"))
  {
    clear();
  }
  ImGui::SameLine();
  CommonViews::HSpacing(2);
  ImGui::SameLine();
  
  // Draw the navigate to content button
  if (CommonViews::LargeIconButton(ICON_MD_CENTER_FOCUS_WEAK, "navigate"))
  {
    ed::SetCurrentEditor(context);
    ed::NavigateToContent();
    ed::SetCurrentEditor(nullptr);
  }
  ImGui::SameLine();
  CommonViews::HSpacing(2);
  //  ImGui::SameLine();
  //
  //  // Draw the zoom buttons
  //  if (CommonViews::LargeIconButton(ICON_MD_ZOOM_IN, "zoom_in"))
  //  {
  //    ed::SetCurrentEditor(context);
  //    ed::ZoomInc(true);
  //    ed::SetCurrentEditor(nullptr);
  //  }
  //  ImGui::SameLine();
  //  CommonViews::HSpacing(2);
  //  ImGui::SameLine();
  //  if (CommonViews::LargeIconButton(ICON_MD_ZOOM_OUT, "zoom_out"))
  //  {
  //    ed::SetCurrentEditor(context);
  //    ed::ZoomInc(false);
  //    ed::SetCurrentEditor(nullptr);
  //  }
  //  ImGui::SameLine();
  //  CommonViews::HSpacing(2);
  ImGui::SameLine();
  // Draw the stage mode button
  if (CommonViews::LargeIconButton(ICON_MD_GRID_VIEW, "stageMode"))
  {
    LayoutStateService::getService()->stageModeEnabled = true;
  }
  
  ImGui::SetCursorPos(pos);
}

void NodeLayoutView::clear() {
  idNodeMap.clear();
  previewWindowNodes.clear();
  terminalNodes.clear();
  nodesToOpen.clear();
  ed::SetCurrentEditor(context);
  ShaderChainerService::getService()->clear();
  VideoSourceService::getService()->clear();
  ParameterService::getService()->clear();
  ConfigService::getService()->saveDefaultConfigFile();
  OscillationService::getService()->clear();
  ed::SetCurrentEditor(nullptr);
}

void NodeLayoutView::drawMetrics()
{
  // Set the cursor to the bottom left of the node layout
  auto cursorPos = ImGui::GetCursorPos();
  ImGui::SetCursorPos(ImVec2(LayoutStateService::getService()->browserSize().x, getScaledWindowHeight() - 100));
  
  ed::SetCurrentEditor(context);
  ed::ShowMetrics();
}

void NodeLayoutView::drawSaveDialog() {
  if (showSaveDialog)
  {
    ImGui::OpenPopup("Save Strand");
    
    if (ImGui::BeginPopupModal("Save Strand", NULL, ImGuiWindowFlags_AlwaysAutoResize))
    {
      ImGui::Text("Enter the filename:");
      ImGui::InputText("##saveFileName", saveFileName, IM_ARRAYSIZE(saveFileName));
      
      if (ImGui::Button("Save", ImVec2(120, 0)))
      {
        // Close the popup
        ImGui::CloseCurrentPopup();
        showSaveDialog = false;
        
        // Proceed with saving the strand
        saveStrand();
      }
      
      ImGui::SameLine();
      if (ImGui::Button("Cancel", ImVec2(120, 0)))
      {
        ImGui::CloseCurrentPopup();
        showSaveDialog = false;
      }
      
      ImGui::EndPopup();
    }
  }
}

void NodeLayoutView::saveStrand()
{
	  Strand strand = ShaderChainerService::getService()->strandForConnectable(nodeToSave->connectable);

    // Trim extension from saveFileName if needed
    std::string fileName(saveFileName);
    auto dotPos = fileName.find_last_of('.');
    if (dotPos != std::string::npos)
    {
        fileName = fileName.substr(0, dotPos);
    }
    strand.name = fileName;
    
    // Use the fileName for saving operations
    std::string previewPath = StrandService::getService()->savePreview(fileName, nodeToSave->connectable);
    ConfigService::getService()->saveStrandFile(strand, ConfigService::getService()->strandsFolderFilePath() + fileName + ".json", previewPath);
}

void NodeLayoutView::drawHelp()
{
  if (!LayoutStateService::getService()->helpEnabled) return;
  
  ImVec2 cursorPos = ImGui::GetCursorPos();
  
  if (!HelpService::getService()->placedSource())
  {
    bool hasPayload = ImGui::GetDragDropPayload();
    ImGui::SetCursorPos(ImVec2(LayoutStateService::getService()->browserSize().x + 20.0, 150.0));
    if (hasPayload) {
      HelpService::getService()->drawSource2View();
    }
    else {
      HelpService::getService()->drawSourceView();
      ImGui::NewLine();
    }
    
    // Reset our cursor and return without drawing additional help
    ImGui::SetCursorPos(cursorPos);
    return;
  }
  
  if (!HelpService::getService()->placedEffect())
  {
    bool hasPayload = ImGui::GetDragDropPayload();
    ImGui::SetCursorPos(ImVec2(LayoutStateService::getService()->browserSize().x + 20.0, getScaledWindowHeight() / 3.0 + 30.0));
    if (hasPayload) {
      HelpService::getService()->drawEffect2View();
    } else {
      HelpService::getService()->drawEffectView();
      ImGui::NewLine();
    }
    // Reset our cursor and return without drawing additional help
    ImGui::SetCursorPos(cursorPos);
    return;
  }
  
  if (!HelpService::getService()->madeConnection())
  {
    if (!ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
      std::shared_ptr<VideoSource> source = VideoSourceService::getService()->videoSources().front();
      if (source == nullptr) {
        ImGui::SetCursorPos(cursorPos);
        return;
      };
      std::shared_ptr<Node> node = idNodeMap[source->id];
      if (node == nullptr) {
        ImGui::SetCursorPos(cursorPos);
        return;
      };
      HelpService::getService()->drawConnectionView(ed::CanvasToScreen(node->position), ed::GetNodeSize(node->id), ed::GetCurrentZoom());
    } else {
      std::shared_ptr<Shader> shader = ShaderChainerService::getService()->shaders().front();
      if (shader == nullptr) {
        ImGui::SetCursorPos(cursorPos);
        return;
      };
      
      std::shared_ptr<Node> node = idNodeMap[shader->shaderId];
      HelpService::getService()->drawConnection2View(ed::CanvasToScreen(node->position), ed::GetNodeSize(node->id), ed::GetCurrentZoom());
    }
    
    // Reset our cursor and return without drawing additional help
    ImGui::SetCursorPos(cursorPos);
    return;
  }
  
  if (!HelpService::getService()->placedSecondSource())
  {
    bool hasPayload = ImGui::GetDragDropPayload();
    ImGui::SetCursorPos(ImVec2(LayoutStateService::getService()->browserSize().x + 20.0, 150.0));
    if (hasPayload) {
      HelpService::getService()->drawSecondSource2View();
    }
    else {
      HelpService::getService()->drawSecondSourceView();
      ImGui::NewLine();
    }
    
    // Reset our cursor and return without drawing additional help
    ImGui::SetCursorPos(cursorPos);
    return;
  }
  
  if (!HelpService::getService()->placedMixEffect())
  {
    bool hasPayload = ImGui::GetDragDropPayload();
    ImGui::SetCursorPos(ImVec2(LayoutStateService::getService()->browserSize().x + 20.0, getScaledWindowHeight() / 3.0 + 30.0));
    
    // Check if our payload is a Mix shader
    auto payload = ImGui::GetDragDropPayload();
    bool isShader = payload && payload->IsDataType("NewShader");
    bool isMix = false;
    if (isShader) {
      int n = *(const int *)payload->Data;
      ShaderType shaderType = (ShaderType)n;
      isMix = shaderType == ShaderTypeMix;
    }
    
    if (hasPayload && isMix) {
      HelpService::getService()->drawMixEffect2View();
    } else if (hasPayload && !isMix) {
      HelpService::getService()->drawWrongEffectView();
    } else {
      HelpService::getService()->drawMixEffectView();
      ImGui::NewLine();
    }
    // Reset our cursor and return without drawing additional help
    ImGui::SetCursorPos(cursorPos);
    return;
  }
  
  // Making a connection to our Mix shader
  if (!HelpService::getService()->madeFirstMixConnection()) {
    if (!ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
      
      // Get a terminal node
      std::shared_ptr<Node> terminalNode;
      for (auto node : terminalNodes) {
        if (node->type == NodeTypeShader && !ShaderChainerService::getService()->isShaderType(node->connectable, ShaderTypeMix)) {
          terminalNode = node;
          break;
        }
      }
      if (terminalNode == nullptr) {
        // Reset our cursor and return without drawing additional help
        ImGui::SetCursorPos(cursorPos);
        return;
      }
      
      HelpService::getService()->drawMixConnectionView(ed::CanvasToScreen(terminalNode->position), ed::GetNodeSize(terminalNode->id), ed::GetCurrentZoom());
    } else {
      // Get a Mix node
      std::shared_ptr<Node> mixNode;
      for (auto node : nodes) {
        if (ShaderChainerService::getService()->isShaderType(node->connectable, ShaderTypeMix)) {
          mixNode = node;
        };
      }
      
      HelpService::getService()->drawMixConnection2View(ed::CanvasToScreen(mixNode->position), ed::GetNodeSize(mixNode->id), ed::GetCurrentZoom());
    }
    
    // Reset our cursor and return without drawing additional help
    ImGui::SetCursorPos(cursorPos);
    return;
  }
  
  // Making a second connection to our Mix shader
  if (!HelpService::getService()->madeSecondMixConnection()) {
    if (!ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
      
      // Get a terminal node
      std::shared_ptr<Node> terminalNode;
      for (auto node : terminalNodes) {
        if (!ShaderChainerService::getService()->isShaderType(node->connectable, ShaderTypeMix) && node->connectable->outputs.size() == 0) {
          terminalNode = node;
          break;
        }
      }
      
      if (terminalNode == nullptr) {
        ImGui::SetCursorPos(cursorPos);
        return;
      }
      
      HelpService::getService()->drawSecondMixConnectionView(ed::CanvasToScreen(terminalNode->position), ed::GetNodeSize(terminalNode->id), ed::GetCurrentZoom());
    } else {
      // Get a Mix node
      std::shared_ptr<Node> mixNode;
      for (auto node : nodes) {
        if (ShaderChainerService::getService()->isShaderType(node->connectable, ShaderTypeMix)) {
          mixNode = node;
        };
      }
      
      HelpService::getService()->drawSecondMixConnection2View(ed::CanvasToScreen(mixNode->position), ed::GetNodeSize(mixNode->id), ed::GetCurrentZoom());
    }
    
    // Reset our cursor and return without drawing additional help
    ImGui::SetCursorPos(cursorPos);
    return;
  }
  
  // Open the ShaderInfo pane
  if (!HelpService::getService()->openedShaderInfo()) {
    // Get a Mix node
    std::shared_ptr<Node> mixNode;
    for (auto node : nodes) {
      if (ShaderChainerService::getService()->isShaderType(node->connectable, ShaderTypeMix)) {
        mixNode = node;
      };
    }
    
    if (mixNode == nullptr) {
      ImGui::SetCursorPos(cursorPos);
      return;
    }
    
    HelpService::getService()->drawOpenShaderInfoView(ed::CanvasToScreen(mixNode->position), ed::GetNodeSize(mixNode->id), ed::GetCurrentZoom());
    
    // Reset our cursor and return without drawing additional help
    ImGui::SetCursorPos(cursorPos);
    return;
  }
  
  if (!HelpService::getService()->editedMixParameter()) {
    HelpService::getService()->drawEditParametersShaderInfoPane();
    
    // Reset our cursor and return without drawing additional help
    ImGui::SetCursorPos(cursorPos);
    return;
  }
  
  if (!HelpService::getService()->openedStageMode()) {
    HelpService::getService()->drawActionButtons();
    
    // Reset our cursor and return without drawing additional help
    ImGui::SetCursorPos(cursorPos);
    return;
  }
  
  if (!HelpService::getService()->openedStageMode()) {
    HelpService::getService()->drawActionButtons();
    
    // Reset our cursor and return without drawing additional help
    ImGui::SetCursorPos(cursorPos);
    return;
  }
}

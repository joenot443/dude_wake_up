//
//  NodeLayoutView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/5/23.
//

#include "NodeLayoutView.hpp"
#include "FontService.hpp"
#include "LibraryService.hpp"
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
#include <future>
#include <imgui_node_editor.h>
#include <imgui_node_editor_internal.h>

static const float ImGuiWindowTitleBarHeight = 70.0f;
static const float NodeWidth = 100.0f;

namespace ed = ax::NodeEditor;

void NodeLayoutView::setup()
{
  ed::Config config;
  config.SettingsFile = "NodeLayout.json";
  ImVector<float> zooms = ImVector<float>();
  zooms.push_back(1.0f);
//  config.CustomZoomLevels = zooms;

  context = ed::CreateEditor(&config);
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
  
  for (auto c : connections) {
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
  ed::PushStyleColor(ax::NodeEditor::StyleColor_Bg, ImVec4(0,0,0,0));
  ed::PushStyleColor(ax::NodeEditor::StyleColor_Grid, ImVec4(0,0,0,0));
  
  bool show = LayoutStateService::getService()->showAudioSettings;
  float height = show ? ofGetViewportHeight() * 3.0f / 5.0f
                      : ofGetViewportHeight() - ImGuiWindowTitleBarHeight;

  drawPreviewWindows();
  
  ed::Begin("My Editor", ImVec2(ofGetWindowWidth() * (4.0f / 5.0f), height));
  ed::NavigateToContent();
  ed::PushStyleVar(ed::StyleVar_ScrollDuration, 0.1f);
  int uniqueId = 1;
  int linkUniqueId = 1000;
  int sourceUniqueId = 10000;
  linksMap.clear();
  nodeIdNodeMap.clear();
  pinIdPinMap.clear();
  pinIdNodeMap.clear();
  terminalNodes.clear();

  // Loop over Shaders, creating a ShaderNode for each
  for (std::shared_ptr<Shader> shader : shaders)
  {
    auto shaderNode = nodeForShaderSourceId(shader->shaderId, NodeTypeShader, shader->name(), shader->supportsAux(), shader->supportsMask(), shader);
    shaderNode->shader = shader;
    drawNode(shaderNode);

    nodes.push_back(shaderNode);

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

    auto sourceNode = nodeForShaderSourceId(source->id, NodeTypeSource, source->sourceName, false, false, conn);
    sourceNode->source = source;
    drawNode(sourceNode);

    // Always draw a Preview for our Source
    terminalNodes.insert(sourceNode);

    nodes.push_back(sourceNode);
  }

  // Loop over ShaderNodes and draw links

  for (auto const &x : nodeIdNodeMap)
  {
    auto shaderNode = x.second;

    // Create Links for every Output [Can go to Mask, Aux, etc.]
    if (!shaderNode->connectable->outputs.empty())
    {
      for (auto connection : shaderNode->connectable->outputs) {
        std::shared_ptr<Node> nextNode = idNodeMap[connection->end->connId()];
        ed::LinkId nextShaderLinkId = uniqueId++;
        
        auto shaderLink =
        std::make_shared<ShaderLink>(nextShaderLinkId, nextNode, shaderNode, connection->id);
        shaderLink->type = connection->linkType();
        linksMap[nextShaderLinkId.Get()] = shaderLink;
        ed::PinId inputPinId = 0;
        
        switch (connection->type) {
          case ConnectionTypeShader:
            inputPinId = shaderLink->input->inputId;
            break;
          case ConnectionTypeSource:
            inputPinId = shaderLink->input->inputId;
            break;
          case ConnectionTypeAux:
            inputPinId = shaderLink->input->auxId;
            break;
          case ConnectionTypeMask:
            inputPinId = shaderLink->input->maskId;
            break;
        }
        
        ed::Link(nextShaderLinkId,
                 shaderLink->output->outputId,
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


  // Delete the selected Shader

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
        ShaderChainerService::getService()->selectShader(node->shader);
    }
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

  ed::SetCurrentEditor(nullptr);

  drawActionButtons();
  
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
  if (node->supportsInput())
  {
    ed::PinId inputPinId = node->inputId;
    ed::BeginPin(inputPinId, ed::PinKind::Input);

    auto icon = node->hasInputLink() ? ICON_MD_RADIO_BUTTON_ON : ICON_MD_RADIO_BUTTON_OFF;
    CommonViews::IconTitle(icon);

    ed::EndPin();
    auto inPin = std::make_shared<Pin>(inputPinId, node, PinTypeInput);
    pinIdPinMap[inputPinId.Get()] = inPin;
    pinIdNodeMap[inputPinId.Get()] = node;
  }

  // Aux Pin
  if (node->supportsAux())
  {
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

  // Mask Pin
  if (node->supportsMask())
  {
    ed::PinId maskPinId = node->maskId;
    ed::BeginPin(maskPinId, ed::PinKind::Input);
    auto icon = node->hasMaskLink() ? ICON_MD_RADIO_BUTTON_ON : ICON_MD_RADIO_BUTTON_OFF;
    CommonViews::sSpacing();
    CommonViews::IconTitle(icon);
    ImGui::PushFont(FontService::getService()->sm);
    ImGui::Text("(Mask)");
    ImGui::PopFont();
    ed::EndPin();

    auto maskPin = std::make_shared<Pin>(maskPinId, node, PinTypeMask);

    pinIdNodeMap[maskPinId.Get()] = node;
    pinIdPinMap[maskPinId.Get()] = maskPin;
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

  // Title
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
  else if (node->type == NodeTypeShader && ShaderChainerService::getService()->isTerminalShader(node->shader))
  {
    if (CommonViews::IconButton(ICON_MD_FULLSCREEN, node->idName()))
    {
      VideoSourceService::getService()->addOutputWindow(node->connectable);
    }

    // Draw a Record button, but only for the terminal shader

    if (CommonViews::IconButton(ICON_MD_CIRCLE, node->idName()))
    {
//      recorder.setup(ShaderChainerService::getService()->shaderChainerForShaderId(node->shader->shaderId));
    }
  }
  // Otherwise draw some spacing
  else if (node->type == NodeTypeSource)
  {
    CommonViews::Spacing(16);
  }
  else
  {
    CommonViews::sSpacing();
  }

  //  node->supportsMask() ? CommonViews::Spacing(16) : CommonViews::sSpacing();
  ImGui::PushFont(FontService::getService()->h3);
  ImGui::Text("%s", truncateString(formatString("%s", node->name.c_str()), 10).c_str());
  ImGui::PopFont();
  node->supportsMask() ? CommonViews::Spacing(28) : CommonViews::mSpacing();
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
  if (node->type == NodeTypeSource && CommonViews::IconButton(ICON_MD_MONITOR, node->idName()))
  {
    resolutionPopupNode = node;
    ed::Suspend();
    resolutionPopupLocation = ImGui::GetMousePos();
    ed::Resume();
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
std::shared_ptr<Node> NodeLayoutView::nodeForShaderSourceId(std::string shaderSourceId, NodeType nodeType, std::string name, bool supportsAux, bool supportsMask, std::shared_ptr<Connectable> connectable)
{
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
  auto inputId = nodeType == NodeTypeShader ? nodeIdTicker++ : NullId;
  auto auxId = supportsAux ? nodeIdTicker++ : NullId;
  auto maskId = supportsMask ? nodeIdTicker++ : NullId;
  auto origin = ImVec2(0, 0);
  if (nodeType == NodeTypeSource)
  {
    origin = VideoSourceService::getService()->videoSourceForId(shaderSourceId)->origin;
  }
  else if (nodeType == NodeTypeShader)
  {
    origin = ImVec2(ShaderChainerService::getService()->shaderForId(shaderSourceId)->settings->x->value, ShaderChainerService::getService()->shaderForId(shaderSourceId)->settings->y->value);
  }

  node = std::make_shared<Node>(nodeId, outputId, inputId, name, nodeType, connectable);
  node->auxId = auxId;
  node->maskId = maskId;

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
  if (key == OF_KEY_BACKSPACE)
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
    auto canvasPos = ed::ScreenToCanvas(ImVec2(ofGetMouseX(), ofGetMouseY()));
    nodeDropLocation = std::make_unique<ImVec2>(canvasPos);
  }

  for (auto id : unplacedNodeIds)
  {
    auto node = idNodeMap[id];
    if (nodeDropLocation != nullptr)
    {
      ed::SetNodePosition(node.get()->id,
                          *nodeDropLocation.get());
      nodeDropLocation = std::make_unique<ImVec2>(nodeDropLocation->x - 200., nodeDropLocation->y);
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
//  if (node->type == NodeTypeShader)
//  {
//    //    ed::SelectNode(node->id, true);
//    // Traverse parent direction
//    auto child = node->shader;
//    while (child->parent != nullptr)
//    {
//      child = child->parent;
//      auto toSelect = idNodeMap[child->shaderId];
//      ed::SelectNode(idNodeMap[child->shaderId]->id, true);
//    }
//
//    auto parent = node->shader;
////    while (parent->next != nullptr)
////    {
////      parent = parent->next;
////      ed::SelectNode(idNodeMap[parent->shaderId]->id, true);
////    }
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
    if (ImGui::MenuItem("Save Chain"))
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
  std::shared_ptr<ShaderChainer> shaderChainer;

//  if (node->type == NodeTypeShader)
//  {
//    shaderChainer = ShaderChainerService::getService()->shaderChainerForShaderId(node->shader->shaderId);
//  }
//
//  // Save from the first shaderchainer for the source
//  else if (node->type == NodeTypeSource)
//  {
//    shaderChainer = ShaderChainerService::getService()->shaderChainersForVideoSourceId(node->source->id).at(0);
//  }
//
//  if (shaderChainer == nullptr)
//    return;
//
//  // Present a file dialog to save the config file
//  // Use a default name of "CURRENT_DATE_TIME.json"
//
//  std::string defaultName =
//      ofGetTimestampString("%Y-%m-%d_%H-%M-%S.json");
//
//  defaultName = formatString("%s_%s", shaderChainer->name.c_str(), defaultName.c_str());
//
//  auto result = ofSystemSaveDialog(defaultName, "Save Effect Chain");
//  if (result.bSuccess)
//  {
//    ConfigService::getService()->saveShaderChainerConfigFile(shaderChainer, defaultName);
//  }
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
      auto canvasPos = ed::ScreenToCanvas(ImVec2(ofGetMouseX(), ofGetMouseY()));
      nodeDropLocation = std::make_unique<ImVec2>(canvasPos);
      unplacedNodeIds.push_back(shader->shaderId);
    }

    // VideoSource Drop Zone
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("VideoSource"))
    {
      std::string id = *(const std::string *)payload->Data;
      std::shared_ptr<AvailableVideoSource> availableSource = VideoSourceService::getService()->availableVideoSourceForId(id);

      VideoSourceType sourceType = availableSource->type;

      switch (sourceType)
      {
      case VideoSource_webcam:
      {
        auto availableWebcamSource = std::dynamic_pointer_cast<AvailableVideoSourceWebcam>(availableSource);
        auto source = VideoSourceService::getService()->addWebcamVideoSource(
            availableWebcamSource->sourceName, availableWebcamSource->webcamId);
        unplacedNodeIds.push_back(source->id);
        break;
      }
      case VideoSource_file:
      {
        auto availableFileSource = std::dynamic_pointer_cast<AvailableVideoSourceFile>(availableSource);
        auto source = VideoSourceService::getService()->addFileVideoSource(
            availableFileSource->sourceName, availableFileSource->path);
        unplacedNodeIds.push_back(source->id);
        break;
      }
      case VideoSource_chainer:
        // Skip
        break;
      case VideoSource_image:
        // Skip
        break;
      case VideoSource_text:
      {
        auto source = VideoSourceService::getService()->addTextVideoSource(availableSource->sourceName);
        unplacedNodeIds.push_back(source->id);
        break;
      }

      case VideoSource_shader:
      {
        auto availableShaderSource = std::dynamic_pointer_cast<AvailableVideoSourceShader>(availableSource);
        auto source = VideoSourceService::getService()->addShaderVideoSource(availableShaderSource->shaderType);
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
      case VideoSource_empty:
        break;
      }

      auto canvasPos = ed::ScreenToCanvas(ImVec2(ofGetMouseX(), ofGetMouseY()));
      nodeDropLocation = std::make_unique<ImVec2>(canvasPos);
    }

    // Available Shader Chainer drop zone
    if (const ImGuiPayload *payload =
            ImGui::AcceptDragDropPayload("AvailableShaderChainer"))
    {
      AvailableShaderChainer availableShaderChainer =
          *(const AvailableShaderChainer *)payload->Data;

      ConfigService::getService()->loadShaderChainerFile(*availableShaderChainer.path);

      auto canvasPos = ed::ScreenToCanvas(ImVec2(ofGetMouseX(), ofGetMouseY()));
      nodeDropLocation = std::make_unique<ImVec2>(canvasPos);
    }

    ImGui::EndDragDropTarget();
  }
}

void NodeLayoutView::handleDroppedSource(std::shared_ptr<VideoSource> source)
{
  unplacedNodeIds.push_back(source->id);
  auto canvasPos = ImVec2(ofGetMouseX(), ofGetMouseY());
  nodeDropLocation = std::make_unique<ImVec2>(canvasPos);
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

      // We have a Shader destination
      if (destNode->type == NodeTypeShader)
      {
        if (ed::AcceptNewItem())
        {
          // VideoSource is our Source
          if (sourceNode->type == NodeTypeSource)
          {
            auto destPin = pinIdPinMap[inputPinId.Get()];

            // If our Dest Pin is an Aux Pin
            if (destPin->pinType == PinTypeAux)
            {
              ShaderChainerService::getService()->makeConnection(sourceNode->source, destNode->shader, ConnectionTypeAux);
            }
            // If our Dest Pin is a Mask Pin
            else if (destPin->pinType == PinTypeMask)
            {
              ShaderChainerService::getService()->makeConnection(sourceNode->source, destNode->shader, ConnectionTypeMask);
            }
            // If our Dest Pin is a standard Input Pin
            else if (destPin->pinType == PinTypeInput)
            {
              ShaderChainerService::getService()->makeConnection(sourceNode->source, destNode->shader, ConnectionTypeSource);
            }
          }

          // Shader is our Source
          if (sourceNode->type == NodeTypeShader)
          {
            auto destPin = pinIdPinMap[inputPinId.Get()];
            // If our Dest Pin is an Aux Pin
            if (destPin->pinType == PinTypeAux)
            {
              ShaderChainerService::getService()->makeConnection(
                  sourceNode->shader, destNode->shader, ConnectionTypeAux);

              // If our Dest Pin is a Mask Pin
            }
            else if (destPin->pinType == PinTypeMask)
            {
              ShaderChainerService::getService()->makeConnection(
                  sourceNode->shader, destNode->shader, ConnectionTypeMask);

              // If our Dest Pin is standard Input Pin
            }
            else if (destPin->pinType == PinTypeInput)
            {
              ShaderChainerService::getService()->makeConnection(
                  sourceNode->shader, destNode->shader, ConnectionTypeShader);
            }
          }
        }
      }
    }
  }
}

void NodeLayoutView::drawUploadChainerWindow()
{
  if (uploadChainer != nullptr && uploadChainer->name.length())
  {
    UploadChainerView::draw(uploadChainer);
  }
}

void NodeLayoutView::drawNodeWindows()
{
  if (nodesToOpen.size() == 0)
    return;

  for (auto const &node : nodesToOpen)
  {
    auto pos = ed::GetNodePosition(node->id);
    pos = ed::CanvasToScreen(pos);
    pos.x = pos.x + ed::GetNodeSize(node->id).x / ed::GetCurrentZoom() + 20;
    ImGui::SetNextWindowPos(pos);
    ImGui::SetNextWindowCollapsed(false);

    auto style = ImGui::GetStyle();
    style.WindowMenuButtonPosition = ImGuiDir_None;
    auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_AlwaysAutoResize;
    ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(10., 10.));
    if (ImGui::Begin(node->idName().c_str(), 0, flags))
    {
      node->drawSettings();
    }
    ImGui::PopStyleVar();
    ImGui::End();
  }
}

void NodeLayoutView::drawPreviewWindows()
{
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
}

void NodeLayoutView::drawPreviewWindow(std::shared_ptr<Node> node)
{
  auto sizeScale = ed::GetCurrentZoom();
  
  auto pos = ed::GetNodePosition(node->id);
  pos = ed::CanvasToScreen(pos);
  pos.x = pos.x + (ed::GetNodeSize(node->id).x / ed::GetCurrentZoom()) / 2 - 80 / sizeScale;
  pos.y = pos.y - 140 / sizeScale;
  ImGui::SetNextWindowPos(pos);
  ImGui::SetNextWindowCollapsed(false);

  auto style = ImGui::GetStyle();
  style.WindowMenuButtonPosition = ImGuiDir_None;
  auto flags = ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_AlwaysAutoResize | ImGuiWindowFlags_NoBringToFrontOnFocus;
  ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0, 0.0));
  ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0,0,0,0));
  ImGui::PushStyleColor(ImGuiCol_FrameBg, ImVec4(0,0,0,0));
  if (ImGui::Begin(formatString("%s-preview", node->idName().c_str()).c_str(), 0, flags))
  {
    if (sizeScale > 5.0) {
      sizeScale = 5.0;
    }
    node->drawPreview(pos, sizeScale);
  }
  ImGui::End();
  ImGui::PopStyleColor();
  ImGui::PopStyleColor();
  ImGui::PopStyleVar();
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
  // Set the cursor to the bottom right of the window
  auto cursorPos = ImGui::GetCursorPos();
  ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x - 170, ImGui::GetWindowSize().y - 100));
  
  // Draw the reset button
  if (CommonViews::LargeIconButton(ICON_MD_RECYCLING, "reset"))
  {
    ed::SetCurrentEditor(context);
    ShaderChainerService::getService()->clear();
    VideoSourceService::getService()->clear();
    ed::SetCurrentEditor(nullptr);
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
  ImGui::SameLine();
  // Draw the zoom buttons
  if (CommonViews::LargeIconButton(ICON_MD_ZOOM_IN, "zoom_in"))
  {
    ed::SetCurrentEditor(context);
    ed::ZoomInc(true);
    ed::SetCurrentEditor(nullptr);
  }
  ImGui::SameLine();
  CommonViews::HSpacing(2);
  ImGui::SameLine();
  if (CommonViews::LargeIconButton(ICON_MD_ZOOM_OUT, "zoom_out"))
  {
    ed::SetCurrentEditor(context);
    ed::ZoomInc(false);
    ed::SetCurrentEditor(nullptr);
  }
  ImGui::SetCursorPos(cursorPos);
}

void NodeLayoutView::drawMetrics() {
  // Set the cursor to the bottom right of the window
  auto cursorPos = ImGui::GetCursorPos();
  ImGui::SetCursorPos(ImVec2(ImGui::GetWindowSize().x / 5, ImGui::GetWindowSize().y - 100));
  
  ed::SetCurrentEditor(context);
  ed::ShowMetrics();
}

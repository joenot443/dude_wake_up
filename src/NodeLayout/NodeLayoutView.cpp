//
//  NodeLayoutView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/5/23.
//

#include "NodeLayoutView.hpp"
#include "Models/Strand.hpp"
#include "FontService.hpp"
#include "ImGuiExtensions.hpp"
#include "ActionService.hpp"
#include "HelpService.hpp"
#include "LibraryService.hpp"
#include "MarkdownView.hpp"
#include "Fonts.hpp"
#include "UploadChainerView.h"
#include "LayoutStateService.hpp"
#include "ShaderChainerService.hpp"
#include "Strings.hpp"
#include "ConfigService.hpp"
#include "VideoSourceService.hpp"
#include "ShaderSource.hpp"
#include "StrandService.hpp"
#include "Colors.hpp"
#include "ofMain.h"
#include <imgui.h>
#include <future>
#include "imgui_node_editor.h"
#include "imgui_node_editor_internal.h"
#include "BookmarkService.hpp"

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
  config->EnableSmoothZoom = true;
  
  ImVector<float> zooms = ImVector<float>();
  zooms.push_back(1.0f);
  //    config->CustomZoomLevels = zooms;
  
  context = ed::CreateEditor(config);
  ofAddListener(LibraryService::getService()->downloadNotification, this, &NodeLayoutView::haveDownloadedAvailableLibraryFile);
  nodeShaderBrowserView = std::make_unique<NodeShaderBrowserView>();
  nodeShaderBrowserView->size = ImVec2(460.0, 500.0);
  nodeShaderBrowserView->leftPadding = 10.0f;
  nodeShaderBrowserView->setup();
  
  nodeVideoSourceBrowserView = std::make_unique<NodeVideoSourceBrowserView>();
  nodeVideoSourceBrowserView->size = ImVec2(460.0, 500.0);
  nodeVideoSourceBrowserView->leftPadding = 10.0f;
  nodeVideoSourceBrowserView->setup();
}

void NodeLayoutView::update() {
  nodeVideoSourceBrowserView->update();
}

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
  CommonViews::PushNodeRedesignStyle();
  
  bool showAudio = LayoutStateService::getService()->showAudioSettings;
  
  float width = (ImGui::GetWindowContentRegionMax().x * 4.0) / 5;
  float height = ImGui::GetWindowContentRegionMax().y - LayoutStateService::getService()->audioSettingsViewHeight() - 56.0f;
  
  ed::Begin("My Editor", ImVec2(width, height));
  ed::NavigateToContent();
  
  int uniqueId = 1;
  int linkUniqueId = 1000;
  int sourceUniqueId = 10000;
  makingLink = false;
  linksMap.clear();
  nodeIdNodeMap.clear();
  pinIdNodeMap.clear();
  pinIdConnectionMap.clear();
  terminalNodes.clear();
  
  int count = 0;
  // Loop over Shaders, creating a ShaderNode for each
  for (std::shared_ptr<Shader> shader : shaders)
  {
    //    log("Drawing node: %d - %s", count, shader->name().c_str());
    auto shaderNode = nodeForShaderSourceId(shader->shaderId, NodeTypeShader, shader->name(), shader);
    shaderNode->shader = shader;
    drawNodeNew(shaderNode);
//    drawNode(shaderNode);
    
    nodes.insert(shaderNode);
    
    // Check if this is the terminal node, in which case, we'll add it to the preview nodes.
    if (ShaderChainerService::getService()->isTerminalShader(shaderNode->shader) &&
        ShaderChainerService::getService()->hasParents(shaderNode->shader))
    {
      terminalNodes.insert(shaderNode);
    }
    count += 1;
  }
  
  // Create VideoSource Nodes
  for (std::shared_ptr<VideoSource> source : sources)
  {
    //    log("Drawing node: %d - %s", count, source->name().c_str());
    // Create a Link to the front shader, if it exists
    // Make source node
    std::shared_ptr<Connectable> conn = source;
    
    auto sourceNode = nodeForShaderSourceId(source->id, NodeTypeSource, source->sourceName, conn);
    sourceNode->source = source;
    drawNodeNew(sourceNode);
    
    // Always draw a Preview for our Source
    terminalNodes.insert(sourceNode);
    
    nodes.insert(sourceNode);
    count += 1;
  }
  
  // Loop over ShaderNodes and draw links
  
  for (auto const &x : nodeIdNodeMap)
  {
    auto shaderNode = x.second;
    
    if (!shaderNode->connectable->outputs.empty())
    {
      for (auto [slot, connections] : shaderNode->connectable->outputs)
      {
        for (auto &connection : connections) // Correctly iterate over connections
        {
          std::shared_ptr<Node> nextNode = idNodeMap[connection->end->connId()];
          std::shared_ptr<Node> startNode = idNodeMap[connection->start->connId()];

          // Skip drawing this link if either node has been deleted
          if (nextNode == nullptr || startNode == nullptr) {
            log("Warning: Skipping connection with deleted node(s) - start: %s, end: %s",
                connection->start->connId().c_str(), connection->end->connId().c_str());
            continue;
          }

          ed::LinkId nextShaderLinkId = uniqueId++;

          auto shaderLink =
          std::make_shared<ShaderLink>(nextShaderLinkId, nextNode, shaderNode, connection->outputSlot, connection->inputSlot, connection->id);
          linksMap[nextShaderLinkId.Get()] = shaderLink;
          ed::PinId inputPinId = 0;
          ed::PinId outputPinId = 0;
          outputPinId = startNode->outputIds[connection->outputSlot];
          inputPinId = nextNode->inputIds[connection->inputSlot];
          pinIdConnectionMap[inputPinId.Get()].push_back(shaderLink);
          pinIdConnectionMap[outputPinId.Get()].push_back(shaderLink);
          connection->linkId = nextShaderLinkId;
          ed::Link(nextShaderLinkId,
                   outputPinId,
                   inputPinId);
        }
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
    if (canDelete() && link != nullptr)
    {
      ShaderChainerService::getService()->breakConnectionForConnectionId(link->connectionId);
    }
  }
  
  // Selects or deletes shaders which are selected
  
  ed::GetSelectedNodes(selectedNodesIds.data(), (int)selectedNodesIds.size());
  std::vector<ed::NodeId> deletedNodeIds;
  
  for (auto selectedNodeId : selectedNodesIds)
  {
    std::shared_ptr<Node> node = nodeIdNodeMap[selectedNodeId.Get()];
    if (node == nullptr)
    {
      nodeIdNodeMap.erase(selectedNodeId.Get());
      deletedNodeIds.push_back(selectedNodeId);
      continue;
    }
    
    if (node->id == selectedNodeId)
    {
      if (canDelete())
        handleDeleteNode(node);
      else
        ShaderChainerService::getService()->selectConnectable(node->connectable);
    }
  }
  
  for (auto nodeId : deletedNodeIds) {
    ed::DeselectNode(nodeId);
  }
  
  if (selectedNodesIds.empty()) {
    ShaderChainerService::getService()->deselectConnectable();
  }
  
  // Flow through the active VideoSources
  if (LayoutStateService::getService()->flowEnabled) {
    for (auto source : sources) {
      drawFlow(source);
    }
  }
    
  handleUnplacedNodes();
  
  if (ed::BeginCreate())
  {
    queryNewLinks();
    ed::EndCreate();
  }
  
  if (firstFrame)
  {
    ed::NavigateToContent();
    firstFrame = false;
  }
  
  handleRightClick();
  handleDoubleClick();
  handleUnplacedDownloadedLibraryFile();
  populateNodePositions();
  populateSelectedNodes();
  drawPinHoverView();
  ed::End();
  
  handleDropZone();
  
  
  drawNodeWindows();
  
  drawUploadChainerWindow();
  
  drawHelp();
  drawActionButtons();
  CommonViews::PopNodeRedesignStyle();
  ed::SetCurrentEditor(nullptr);
  drawSaveDialog();
  
  shouldDelete = false;
}

void NodeLayoutView::drawPinHoverView() {
  // Layout constants
  const float buttonSize = 30.0f;
  const float padding = 12.0f;
  const float rowHeight = buttonSize + 4.0f;
  const float wiggle = 50.0f;
  const float buttonCornerRounding = 6.0f;
  const float tooltipCornerRounding = 12.0f;
  const float tooltipAlpha = 230.0f;
  const float tooltipBorderAlpha = 230.0f;
  const float tooltipYOffset = 10.0f;
  const float buttonTextPadding = 10.0f;
  const float symbolPadding = 8.0f;
  const float textSizeScale = 0.75f;

  static ed::PinId lastHoveredPin;
  static ImVec2 lastPinScreenPosition;
  static bool isHovering = false;
  static ImVec2 lastTooltipMin, lastTooltipMax;
  static int hoverFrameCount = 0;
  const int hoverFrameThreshold = 30; // ~1 second at 60fps

  ed::PinId hoveredPin = ed::GetHoveredPin();

  // If we're hovering over a pin, update our tracking variables
  if (hoveredPin.Get() != 0) {
    // If this is a new pin, reset the frame counter
    if (lastHoveredPin.Get() != hoveredPin.Get()) {
      hoverFrameCount = 0;
      lastHoveredPin = hoveredPin;
    }

    // Convert to screen space immediately to handle zoom/pan changes
    ImVec2 pinCanvasPos = ed::GetPinPosition(hoveredPin);
    lastPinScreenPosition = ed::CanvasToScreen(pinCanvasPos);
    isHovering = true;
    hoverFrameCount++;
  }

  // Get all ShaderLinks for this pin (check even if not currently hovering)
  long pinKey = lastHoveredPin.Get();
  if (pinKey == 0 || pinIdConnectionMap.find(pinKey) == pinIdConnectionMap.end()) {
    isHovering = false;
    return;
  }
  const auto& links = pinIdConnectionMap[pinKey];
  if (links.empty()) {
    isHovering = false;
    return;
  }

  // Suspend EARLY so all our mouse checks and drawing are in screen space
  ed::Suspend();

  // Get mouse position AFTER suspend to ensure it's in screen space
  ImVec2 mousePos = ImGui::GetMousePos();

  // Calculate the widest text width for all clipped node names
  float widestTextWidth = 0.0f;
  std::vector<std::string> clippedNames;
  for (size_t i = 0; i < links.size(); ++i) {
    std::shared_ptr<Node> thisNode = nullptr;
    std::shared_ptr<Node> otherNode = nullptr;
    if (links[i]->input->id == pinIdNodeMap[pinKey]->id) {
      thisNode = links[i]->input;
      otherNode = links[i]->output;
    } else {
      thisNode = links[i]->output;
      otherNode = links[i]->input;
    }
    const char* nodeName = otherNode ? otherNode->name.c_str() : "?";
    std::string clippedNodeName = nodeName;
    if (clippedNodeName.length() > 15) {
      clippedNodeName = clippedNodeName.substr(0, 15) + "…";
    }
    clippedNames.push_back(clippedNodeName);
    ImVec2 textSize = ImGui::CalcTextSize(clippedNodeName.c_str());
    if (textSize.x > widestTextWidth) widestTextWidth = textSize.x;
  }
  const float maxTooltipWidth = 320.0f;
  const float tooltipWidth = std::min(maxTooltipWidth, buttonSize + buttonTextPadding + 2 * padding + widestTextWidth);

  float tooltipHeight = links.size() * rowHeight + padding * 2;

  // Position tooltip so its bottom edge is tooltipYOffset px above the pin (already in screen space)
  ImVec2 tooltipCenter = lastPinScreenPosition - ImVec2(0, tooltipHeight + tooltipYOffset);
  ImVec2 tooltipMin = tooltipCenter - ImVec2(tooltipWidth/2, 0);
  ImVec2 tooltipMax = tooltipCenter + ImVec2(tooltipWidth/2, tooltipHeight);
  lastTooltipMin = tooltipMin;
  lastTooltipMax = tooltipMax;

  // Hover logic: keep open if mouse is within tooltip bounds plus wiggle room
  // Create a generous hover area that includes both the tooltip and the path to the pin
  // The tooltip is ABOVE the pin, so hoverMin.y should be at the tooltip top
  // and hoverMax.y should be at/below the pin
  float hoverMinX = std::min(tooltipMin.x, lastPinScreenPosition.x) - wiggle;
  float hoverMaxX = std::max(tooltipMax.x, lastPinScreenPosition.x) + wiggle;
  float hoverMinY = tooltipMin.y - wiggle;  // Top of tooltip with margin
  float hoverMaxY = lastPinScreenPosition.y + wiggle;  // Bottom at pin with margin

  bool mouseInHoverArea = mousePos.x >= hoverMinX && mousePos.x <= hoverMaxX &&
                          mousePos.y >= hoverMinY && mousePos.y <= hoverMaxY;

  // Keep the tooltip open if: we're hovering the pin OR mouse is in the hover area
  bool shouldShowTooltip = (hoveredPin.Get() != 0) || mouseInHoverArea;

  // Only show tooltip after hovering for the threshold duration
  if (!shouldShowTooltip || hoverFrameCount < hoverFrameThreshold) {
    if (!shouldShowTooltip) {
      isHovering = false;
      lastHoveredPin = ed::PinId();
      hoverFrameCount = 0;
    }
    ed::Resume();  // Must resume before returning since we suspended earlier
    return;
  }

  // Use the foreground draw list to ensure the popover is always on top
  ImDrawList* drawList = ImGui::GetForegroundDrawList();
  drawList->AddRectFilled(tooltipMin, tooltipMax, IM_COL32(40, 40, 40, tooltipAlpha), tooltipCornerRounding);
  drawList->AddRect(tooltipMin, tooltipMax, IM_COL32(60, 60, 60, tooltipBorderAlpha), tooltipCornerRounding);

  for (size_t i = 0; i < links.size(); ++i) {
    ImVec2 rowTopLeft = tooltipMin + ImVec2(padding, padding + i * rowHeight);
    float centerY = rowTopLeft.y + rowHeight / 2.0f;

    // X button: center vertically on centerY
    ImVec2 buttonMin = ImVec2(rowTopLeft.x, centerY - buttonSize / 2.0f);
    ImVec2 buttonMax = buttonMin + ImVec2(buttonSize, buttonSize);

    // Draw X button hover/click
    bool isOverButton = ImGui::IsMouseHoveringRect(buttonMin, buttonMax);
    ImU32 buttonColor = isOverButton ? IM_COL32(200, 50, 50, 255) : IM_COL32(150, 50, 50, 255);
    drawList->AddRectFilled(buttonMin, buttonMax, buttonColor, buttonCornerRounding);

    // Draw X symbol
    drawList->AddLine(
      buttonMin + ImVec2(symbolPadding, symbolPadding),
      buttonMax - ImVec2(symbolPadding, symbolPadding),
      IM_COL32(255, 255, 255, 255), 2.0f
    );
    drawList->AddLine(
      ImVec2(buttonMax.x - symbolPadding, buttonMin.y + symbolPadding),
      ImVec2(buttonMin.x + symbolPadding, buttonMax.y - symbolPadding),
      IM_COL32(255, 255, 255, 255), 2.0f
    );

    // Draw node name on the other side of the connection, vertically centered
    std::shared_ptr<Node> thisNode = nullptr;
    std::shared_ptr<Node> otherNode = nullptr;
    if (links[i]->input->id == pinIdNodeMap[pinKey]->id) {
      thisNode = links[i]->input;
      otherNode = links[i]->output;
    } else {
      thisNode = links[i]->output;
      otherNode = links[i]->input;
    }
    const char* nodeName = otherNode ? otherNode->name.c_str() : "?";
    std::string clippedNodeName = nodeName;
    if (clippedNodeName.length() > 15) {
      clippedNodeName = clippedNodeName.substr(0, 15) + "…";
    }
    float textMaxWidth = tooltipWidth - (buttonSize + buttonTextPadding + 2 * padding);
    float smallTextSize = ImGui::GetFontSize() * textSizeScale;
    // Calculate actual text height at the scaled size for proper centering
    float scaledTextHeight = smallTextSize;
    ImVec2 textPos = ImVec2(
      buttonMax.x + buttonTextPadding,
      centerY - scaledTextHeight / 2.0f
    );

    ImVec2 titleMin = textPos;
    ImVec2 titleMax = ImVec2(textPos.x + textMaxWidth, centerY + rowHeight / 2.0f);
    bool isOverTitle = ImGui::IsMouseHoveringRect(titleMin, titleMax);
    // Only draw a single selection indicator for the whole row
    ImVec2 selectionMin = buttonMin;
    ImVec2 selectionMax = ImVec2(textPos.x + textMaxWidth, buttonMax.y > (centerY + rowHeight / 2.0f) ? buttonMax.y : (centerY + rowHeight / 2.0f));
    bool isOverSelection = isOverButton || isOverTitle || ImGui::IsMouseHoveringRect(selectionMin, selectionMax);
    ImU32 selectionBgColor = isOverSelection ? IM_COL32(80, 80, 120, 180) : 0;
    if (isOverSelection) {
      drawList->AddRectFilled(selectionMin, selectionMax, selectionBgColor, buttonCornerRounding);
    }

    // Draw X button (on top of highlight)
    drawList->AddRectFilled(buttonMin, buttonMax, buttonColor, buttonCornerRounding);
    drawList->AddLine(
      buttonMin + ImVec2(symbolPadding, symbolPadding),
      buttonMax - ImVec2(symbolPadding, symbolPadding),
      IM_COL32(255, 255, 255, 255), 2.0f
    );
    drawList->AddLine(
      ImVec2(buttonMax.x - symbolPadding, buttonMin.y + symbolPadding),
      ImVec2(buttonMin.x + symbolPadding, buttonMax.y - symbolPadding),
      IM_COL32(255, 255, 255, 255), 2.0f
    );

    // Center the text vertically in the row using centerY
    drawList->AddText(NULL, smallTextSize, textPos, IM_COL32(255,255,255,255), clippedNodeName.c_str(), nullptr, textMaxWidth);
    
    // Handle row click (anywhere in the selection area)
    if (isOverSelection && ImGui::IsMouseClicked(ImGuiMouseButton_Left)) {
      ShaderChainerService::getService()->breakConnectionForConnectionId(links[i]->connectionId);
      isHovering = false;
      lastHoveredPin = ed::PinId();
      break;
    }
  }

  // Resume the node editor after drawing in screen space
  ed::Resume();
}

void NodeLayoutView::drawNodeNew(std::shared_ptr<Node> node) {
  ed::NodeId nodeId = node->id;
  ed::BeginNode(node->id);
  bool isSource = node->type == NodeTypeSource;
  bool isShader = node->type == NodeTypeShader;
  bool hasSelectorOpen = node->id == selectorNodeId;
  bool height = hasSelectorOpen ? 1000.0 : 500.0;
  ImVec2 previewPadding = ImVec2(0.9, 0.7);
  auto pos = ImGui::GetCursorPos();
  auto startPos = pos;
  
  // Preview:   500 x 280
  // Node Size: 500 x 220
  // Always show preview for sources and shaders (even when not connected)
  if (isSource || isShader) {
    ImGui::SetCursorPos(ImGui::GetCursorPos() + previewPadding);
    node->drawPreviewSized(ImVec2(500.0, 280.0) - 2*previewPadding);
    ImGui::Dummy(ImVec2(500.0, 280.0));
  }
  
  if (ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left) && ImGui::IsItemHovered()) {
    VideoSourceService::getService()->addOutputWindow(node->connectable);
  }
  
  /*
   20 - 50   -    360   - 50  - 20
   [Input] - Selector - [Output]
   */
  pos = ImGui::GetCursorPos();
  // Input Pin
  
  // Add padding
  ImGui::SetCursorPos(pos + ImVec2(20.0, 20.0));
  
  // Inner Rect background
  pos = ImGui::GetCursorPos();
  float innerRectWidth = 460.0;
  
  float selectorWidth = 400.0;
  
  ed::GetCurrentDrawList()->AddRectFilled(ImVec2(pos.x, pos.y), ImVec2(pos.x + innerRectWidth, pos.y + 46.0), Colors::NodeInnerRectBackgroundColor, 10.0);
  // Add padding
  ImGui::SetCursorPos(pos + ImVec2(10.0, 0.0));
  
  // Input Pin
  pos = ImGui::GetCursorPos();
  for (int i = 0; i < node->connectable->inputCount(); i++) {
    InputSlot slot = static_cast<InputSlot>(i);
    ed::PinId inputPinId = CommonViews::InputNodePin(node, slot);
    pinIdNodeMap[inputPinId.Get()] = node;
    ImGui::SameLine();
  }

  selectorWidth = selectorWidth - (ImGui::GetCursorPosX() - pos.x);
  
  // Selector
  if (CommonViews::SelectorTitleButton(node->connectable->name().substr(0, 30), node->connectable->connId(), selectorWidth)) {
    if (hasSelectorOpen) {
      selectorNodeId = 0;
    } else {
      selectorNodeId = node->id;
    }
    ed::SelectNode(node->id);
    
    if (!hasSelectorOpen) {
      if (isShader) {
        if (node->hasInputLinkAt(InputSlotMain)) {
          ShaderChainerService::getService()->hydrateAuxillaryShaders(node->shader->parentFrame());
          nodeShaderBrowserView->setup();
          nodeShaderBrowserView->setCallback([this, &node](std::shared_ptr<TileItem> tile) {
            handleUpdatedShaderNode(node, tile);
          });
        }
      } else {
        nodeVideoSourceBrowserView->setCallback([this, &node](std::shared_ptr<TileItem> tile) {
          handleUpdatedSourceNode(node, tile);
        });
      }
    }
  }
  
  // Right x padding
  ImGui::SetCursorPosX(ImGui::GetCursorPosX() + 10.0);
  
  ed::PinId outputPinId = CommonViews::OutputNodePin(node, OutputSlotMain);
  pinIdNodeMap[outputPinId.Get()] = node;
  
  
  // Draw the selector inside the node
  if (hasSelectorOpen) {
    ImGui::Dummy(ImVec2(0.0, 15.0));
    if (isShader) {
      nodeShaderBrowserView->draw();
    } else {
      nodeVideoSourceBrowserView->draw();
    }
  }
  
  // Calculate total available width and spacing
  float totalWidth = 460.0f;
  float buttonSpacing = 8.0f;
  
  // Calculate button count (accounting for conditional audio button)
  int buttonCount = 3; // Base buttons: play/pause, save, fullscreen
  if (node->hasAudioReactiveParameter()) {
    buttonCount++; // Add audio reactive button if available
  }
  
  // Calculate dynamic button size based on total width and spacing
  float buttonSize = (totalWidth - (buttonSpacing * (buttonCount - 1))) / buttonCount;
  float buttonHeight = 48.0;
  // Start position for first button
  ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(20.0, 10.0));
  
  if (node->hasAudioReactiveParameter()) {
    const char* buttonImage = node->isAudioReactiveParameterActive() ? "no-music.png" : "music.png";
    
    if (CommonViews::ImageButton(node->connectable->connId(), buttonImage, ImVec2(buttonSize,  buttonHeight))) {
      toggleAudioReactiveParameter(node);
    }
    ImGui::SameLine(0, buttonSpacing);
  }
  
  const char* buttonImage = node->connectable->active ? "pause.png" : "play.png";
  if (CommonViews::ImageButton(node->connectable->connId(), buttonImage, ImVec2(buttonSize,  buttonHeight))) {
    node->connectable->active = !node->connectable->active;
  }
  ImGui::SameLine(0, buttonSpacing);
  
  if (CommonViews::ImageButton(node->connectable->connId(), "save.png", ImVec2(buttonSize,  buttonHeight))) {
    handleSaveNode(node);
  }
  ImGui::SameLine(0, buttonSpacing);
  
  if (CommonViews::ImageButton(node->connectable->connId(), "fullscreen.png", ImVec2(buttonSize,  buttonHeight))) {
    VideoSourceService::getService()->addOutputWindow(node->connectable);
  }

  // Draw Aux output pin for shaders that support it (e.g., FeedbackShader)
  if (node->type == NodeTypeShader && node->shader->allowAuxOutputSlot()) {
    ImGui::Dummy(ImVec2(500.0, 10.0));

    // Center the aux output pin horizontally
    float auxPinWidth = 40.0f;
    float centerX = (500.0f - auxPinWidth) / 2.0f;
    ImGui::SetCursorPos(ImGui::GetCursorPos() + ImVec2(centerX, 0.0));

    ed::PinId auxOutputPinId = CommonViews::OutputNodePin(node, OutputSlotAux);
    pinIdNodeMap[auxOutputPinId.Get()] = node;

    ImGui::Dummy(ImVec2(500.0, 10.0));
  } else {
    ImGui::Dummy(ImVec2(500.0, 20.0));
  }

  ed::EndNode();
}

void NodeLayoutView::drawFlow(std::shared_ptr<Connectable> connectable) {
  for (std::shared_ptr<Connection> connection : connectable->outputConnections()) {
    ed::Flow(connection->linkId);
    drawFlow(connection->end);
  }
}

void NodeLayoutView::drawNode(std::shared_ptr<Node> node)
{
  ed::NodeId nodeId = node->id;
  ed::PushStyleColor(ax::NodeEditor::StyleColor_NodeBg, node->nodeColor());
  ed::BeginNode(node->id);
  
  // ----- FIRST COLUMN ------
  ImGuiEx_BeginColumn();
  
  // Draw delete button
  if (CommonViews::XLargeIconButton(ICON_MD_DELETE, node->idName()))
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
    
    CommonViews::XLargeIconTitle(icon);
    if (i < node->connectable->inputCount() - 1) {
      CommonViews::sSpacing();
    }
    
    ed::EndPin();
    auto inPin = std::make_shared<Pin>(inputPinId, node, PinTypeInput);
    pinIdNodeMap[inputPinId.Get()] = node;
  }
  
  // Save Button
  if (node->type == NodeTypeSource)
  {
    CommonViews::Spacing(72);
    if (CommonViews::XLargeIconButton(ICON_MD_SAVE, node->idName()))
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
      CommonViews::XLargeIconButton(visIcon, node->idName()))
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
    if (CommonViews::XLargeIconButton(ICON_MD_FULLSCREEN, node->idName()))
    {
      VideoSourceService::getService()->addOutputWindow(node->connectable);
    }
  }
  
  CommonViews::mSpacing();
  
  // Node Title
  ImGui::PushFont(FontService::getService()->current->h1);
  // Add some frame padding
  CommonViews::PaddedText(node->name, ImVec2(0.0, 0.0));
  ImGui::PopFont();
  
  CommonViews::mSpacing();
  
  // Draw Aux pin if Feedback
  if (node->type == NodeTypeShader && node->shader->allowAuxOutputSlot()) {
    // Get secondary Output Pin
    ed::PinId outputPinId = node->outputIds[OutputSlotAux];
    
    // Output Pin
    ed::BeginPin(outputPinId, ed::PinKind::Output);
    auto icon = node->hasOutputLinkAt(OutputSlotAux) ? ICON_MD_PLAY_CIRCLE_FILLED : ICON_MD_PLAY_CIRCLE_OUTLINE;
    CommonViews::XLargeIconTitle(icon);
    ed::EndPin();
    auto outPin = std::make_shared<Pin>(outputPinId, node, PinTypeOutput);
    pinIdNodeMap[outputPinId.Get()] = node;
  }
  
  // Draw an Audio Reactivity button if we have one
  else if (node->type == NodeTypeShader && node->shader->settings->audioReactiveParameter != nullptr) {
    
    // Draw the disable button if we're already driving
    if (node->shader->settings->audioReactiveParameter->driver != nullptr) {
      
      if (CommonViews::XLargeIconButton(ICON_MD_MUSIC_OFF, formatString("##disableAudio%s", node->shader->shaderId.c_str()))) {
        node->shader->settings->audioReactiveParameter->removeDriver();
      }
    }
    // Otherwise drive the default audio driver button
    else if (CommonViews::XLargeIconButton(ICON_MD_MUSIC_NOTE, formatString("##enableAudio%s", node->shader->shaderId.c_str()))) {
      toggleAudioReactiveParameter(node);
    }
  }
  
  
  
  ImGuiEx_NextColumn();
  
  // ----- THIRD COLUMN ------
  
  auto name = node->idName();
  // Draw settings button
  if (CommonViews::XLargeIconButton(ICON_MD_SETTINGS, node->idName()))
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
  
  std::vector<OutputSlot> slotsToDraw = node->connectable->populatedOutputSlots();
  // On top, draw an Output Pin for the next available Slot
  OutputSlot nextSlot = node->connectable->nextAvailableOutputSlot();
  if (nextSlot != OutputSlotNone) {
    slotsToDraw.push_back(nextSlot);
  }
  
  for (OutputSlot slot : slotsToDraw) {
    ed::PinId outputPinId = node->outputIds[slot];
    // Draw an Output Pin for each OutputSlot which is linked
    auto outPin = std::make_shared<Pin>(outputPinId, node, PinTypeOutput);
    pinIdNodeMap[outputPinId.Get()] = node;
    
    ed::BeginPin(outputPinId, ed::PinKind::Output);
    auto icon = node->hasOutputLinkAt(slot) ? ICON_MD_PLAY_CIRCLE_FILLED : ICON_MD_PLAY_CIRCLE_OUTLINE;
    CommonViews::XLargeIconTitle(icon);
    ed::EndPin();
  }
  
  CommonViews::mSpacing();
  
  // Draw Star for selecting Stage Shader
  if (node->type == NodeTypeShader && ShaderChainerService::getService()->isTerminalShader(node->shader))
  {
    if (ParameterService::getService()->isShaderIdStage(node->shader->shaderId)) {
      if (CommonViews::XLargeIconButton(ICON_MD_STAR, node->idName())) {
        ParameterService::getService()->toggleStageShaderId(node->shader->shaderId);
      }
    } else {
      if (CommonViews::XLargeIconButton(ICON_MD_STAR_OUTLINE, node->idName())) {
        ParameterService::getService()->toggleStageShaderId(node->shader->shaderId);
      }
    }
  }
  
  else if (node->type == NodeTypeSource)
  {
    auto icon = node->source->active ? ICON_MD_PAUSE : ICON_MD_PLAY_ARROW;
    if (CommonViews::XLargeIconButton(icon, formatString("##%splayPause", node->source->id.c_str()))) {
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
  
  if (idNodeMap.count(shaderSourceId) != 0) {
    std::shared_ptr<Node> node = idNodeMap[shaderSourceId];
    if (node != nullptr) {
      nodeIdNodeMap[node->id.Get()] = node;
      idNodeMap[shaderSourceId] = node;
      return node;
    }
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
  
  std::shared_ptr<Node> node = std::make_shared<Node>(nodeId, outputIds, inputIds, name, nodeType, connectable);
  
  // If we're placing the node from a JSON, we have an origin.
  // It's possible the Node hasn't actually be placed yet, in which case don't set its position
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

void NodeLayoutView::populateSelectedNodes()
{
  // Clear the current selection
  selectedConnectables.clear();
  
  // Get the count of selected nodes
  int selectedCount = ed::GetSelectedObjectCount();
  if (selectedCount == 0) return;
  
  // Create a vector to hold the selected node IDs
  std::vector<ed::NodeId> selectedNodeIds(selectedCount);
  
  // Retrieve the selected node IDs
  ed::GetSelectedNodes(selectedNodeIds.data(), selectedCount);
  
  // Iterate over the selected node IDs
  for (auto nodeId : selectedNodeIds)
  {
    // Find the node in the nodeIdNodeMap
    auto it = nodeIdNodeMap.find(nodeId.Get());
    if (it != nodeIdNodeMap.end())
    {
      // Add the node to the selectedNodes set
      selectedConnectables.push_back(it->second->connectable);
    }
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

void NodeLayoutView::handleUpdatedSourceNode(std::shared_ptr<Node> node, std::shared_ptr<TileItem> tile) {
  selectorNodeId = 0;
  std::shared_ptr<VideoSource> newSource = nullptr;
  if (node->source->type == VideoSource_shader) {
    newSource = VideoSourceService::getService()->replaceShaderVideoSource(std::dynamic_pointer_cast<ShaderSource>(node->source),  shaderSourceTypeForShaderType(tile->shaderType));
  } else {
    return;
  }
  idNodeMap.erase(node->source->id);
  auto newNode = nodeForShaderSourceId(node->source->id, NodeTypeSource, newSource->name(), newSource);
  ed::SetNodePosition(newNode->id, node->position);
  ed::SelectNode(newNode->id);
}

void NodeLayoutView::handleUpdatedShaderNode(std::shared_ptr<Node> node, std::shared_ptr<TileItem> tile) {
  selectorNodeId = 0;
  auto shader = node->shader;
  auto newShader = ShaderChainerService::getService()->replaceShader(shader, tile->shaderType);
  ShaderChainerService::getService()->copyConnections(shader, newShader);
  idNodeMap.erase(node->shader->shaderId);
  auto newNode = nodeForShaderSourceId(newShader->shaderId, NodeTypeShader, newShader->name(), newShader);
  ed::SetNodePosition(newNode->id, node->position);
  ShaderChainerService::getService()->selectedConnectable = nullptr;
  ed::ClearSelection();
  ed::SelectNode(newNode->id);
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
  
  if (node == nullptr) return;
  
  if (fileDropInProgress) {
    nodeDropLocation = std::make_unique<ImVec2>(ed::ScreenToCanvas(*nodeDropLocation.get()));
    fileDropInProgress = false;
  }
  
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
  
}

void NodeLayoutView::closeSettingsWindow(std::shared_ptr<Connectable> connectable)
{
  auto node = idNodeMap[connectable->connId()];
  bool inSet = nodesToOpen.find(node) != nodesToOpen.end();
  if (inSet && node != nullptr)
  {
    nodesToOpen.erase(node);
  }
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

    // Add "Edit" option for Shader and ShaderSource nodes
    bool isShader = node->type == NodeTypeShader;
    bool isShaderSource = node->type == NodeTypeSource && node->source->type == VideoSource_shader;

    if (isShader || isShaderSource) {
      if (ImGui::MenuItem("Edit")) {
        // Get the shader name
        std::string shaderName;
        if (isShader) {
          shaderName = node->shader->name();
        } else if (isShaderSource) {
          auto shaderSource = std::dynamic_pointer_cast<ShaderSource>(node->source);
          shaderName = shaderSource->shader->name();
        }

        // Construct the path to the .frag file
        std::string fragPath = ofToDataPath("shaders/" + shaderName + ".frag");

        // Open the file with the default editor on macOS
        std::string command = "open \"" + fragPath + "\"";
        system(command.c_str());
      }
    }

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
    int selectedNodeCount = ed::GetSelectedObjectCount();
    if (selectedNodeCount == 2) {
      ed::NodeId selectedNodes[2];
      ed::GetSelectedNodes(selectedNodes, 2);

      if (ImGui::MenuItem("Blend")) {
        createBlendShaderForSelectedNodes(selectedNodes[0], selectedNodes[1]);
      }
    }
    if (ImGui::BeginMenu("Break Connections")) {
      for (auto connection: node->connectable->connections()) {
        if (ImGui::MenuItem(formatString("%s → %s", connection->start->name().c_str(), connection->end->name().c_str()).c_str())) {
          ShaderChainerService::getService()->breakConnectionForConnectionId(connection->id);
        }
      }
      ImGui::EndMenu();
    }
    ImGui::EndPopup();
  }
  ed::Resume();
}

void NodeLayoutView::createBlendShaderForSelectedNodes(ed::NodeId firstNodeId, ed::NodeId secondNodeId) {
  auto firstNodeIt = nodeIdNodeMap.find(firstNodeId.Get());
  auto secondNodeIt = nodeIdNodeMap.find(secondNodeId.Get());
  
  if (firstNodeIt == nodeIdNodeMap.end() || secondNodeIt == nodeIdNodeMap.end()) {
    return; // Ensure nodes are valid
  }
  
  auto topNode = firstNodeIt->second->position.y < secondNodeIt->second->position.y ? firstNodeIt->second : secondNodeIt->second;
  auto bottomNode = firstNodeIt->second->position.y < secondNodeIt->second->position.y ? secondNodeIt->second : firstNodeIt->second;
  
  ImVec2 firstPos = topNode->position;
  ImVec2 secondPos = bottomNode->position;
  
  ImVec2 pos = ImVec2(max(firstPos.x, secondPos.x) + 800.0, (firstPos.y + secondPos.y) / 2.0);
  
  // Create a new BlendShader
  std::shared_ptr<Shader> blendShader = ActionService::getService()->addShader(ShaderTypeBlend);
  
  nodeDropLocation = std::make_unique<ImVec2>(pos);
  
  unplacedNodeIds.push_back(blendShader->shaderId);
  
  // Always use OutputSlotMain since we allow multiple connections from the same output
  ActionService::getService()->makeConnection(topNode->connectable, blendShader,
                                              topNode->type == NodeTypeShader ? ConnectionTypeShader : ConnectionTypeSource,
                                              OutputSlotMain, InputSlotMain, false);
  
  ActionService::getService()->makeConnection(bottomNode->connectable, blendShader,
                                              bottomNode->type == NodeTypeShader ? ConnectionTypeShader : ConnectionTypeSource,
                                              OutputSlotMain, InputSlotTwo, true);
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
      if (!hasDeletedBlend && node->shader->type() == ShaderTypeBlend) hasDeletedBlend = true;
      ActionService::getService()->removeShader(node->shader);
      break;
    case NodeTypeSource:
      ActionService::getService()->removeVideoSource(node->source);
      break;
  }
  // Deselect the node if it's selected
  if (ed::IsNodeSelected(node->id)) {
    ed::ClearSelection();
  }
  previewWindowNodes.erase(node);
  nodesToOpen.erase(node);
}

void NodeLayoutView::addUnplacedConnectable(std::shared_ptr<Connectable> connectable) {
  std::shared_ptr<Node> node = nodeForShaderSourceId(connectable->connId(), connectable->connectableType() == ConnectableTypeShader ? NodeTypeShader : NodeTypeSource, connectable->name(), connectable);
  
  unplacedNodeIds.push_back(connectable->connId());
  drawNodeNew(node);
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
  nodeToSave = node;
  saveStrand();
}

void NodeLayoutView::handleDropZone()
{
  if (ImGui::BeginDragDropTarget())
  {
    // Shader Drop Zone
    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("NewShader"))
    {
      int n = *(const int *)payload->Data;
      ShaderType shaderType = (ShaderType)n;
      
      auto shader = ActionService::getService()->addShader(shaderType);
      auto canvasPos = ed::ScreenToCanvas(getScaledMouseLocation());
      auto hovered = ed::GetHoveredNode();
      
      if (hovered.Get() == 0) {
        nodeDropLocation = std::make_unique<ImVec2>(canvasPos);
      } else {
        // Dropped on an existing Shader
        nodeDropLocation = std::make_unique<ImVec2>(ed::GetNodePosition(hovered) + ImVec2(ed::GetNodeSize(hovered).x, 0.0) + ImVec2(200.0, 0.0));
        auto startNode = nodeIdNodeMap[hovered.Get()];
        // Check if the existing Shader was already connected to a next node
        std::shared_ptr<Connectable> next = nullptr;
        if (startNode->connectable->hasOutputAtSlot(OutputSlotMain)) {
          auto conn = startNode->connectable->connectionAtSlot(OutputSlotMain);
          ShaderChainerService::getService()->insert(startNode->connectable, shader, OutputSlotMain);
          ShaderChainerService::getService()->breakConnectionForConnectionId(conn->id);
        } else {
          ActionService::getService()->makeConnection(startNode->connectable, shader, startNode->type == NodeTypeShader ? ConnectionTypeShader : ConnectionTypeSource, OutputSlotMain, InputSlotMain, true);
        }
      }
      unplacedNodeIds.push_back(shader->shaderId);
    }
    
    // VideoSource Drop Zone
    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("VideoSource"))
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
          source = ActionService::getService()->addWebcamVideoSource(availableWebcamSource->sourceName, availableWebcamSource->webcamId);
          unplacedNodeIds.push_back(source->id);
          break;
        }
        case VideoSource_file:
        {
          auto availableFileSource = std::dynamic_pointer_cast<AvailableVideoSourceFile>(availableSource);
          source = ActionService::getService()->addFileVideoSource(availableFileSource->sourceName, availableFileSource->path);
          unplacedNodeIds.push_back(source->id);
          break;
        }
        case VideoSource_image:
        {
          auto availableFileSource = std::dynamic_pointer_cast<AvailableVideoSourceImage>(availableSource);
          source = ActionService::getService()->addImageVideoSource(availableFileSource->sourceName, availableFileSource->path);
          unplacedNodeIds.push_back(source->id);
          break;
        }
        case VideoSource_text:
        {
          source = ActionService::getService()->addTextVideoSource(availableSource->sourceName);
          unplacedNodeIds.push_back(source->id);
          break;
        }
        case VideoSource_typewriter:
        {
          source = ActionService::getService()->addTypewriterTextVideoSource(availableSource->sourceName);
          unplacedNodeIds.push_back(source->id);
          break;
        }
        case VideoSource_scrollingText:
        {
          source = ActionService::getService()->addScrollingTextVideoSource(availableSource->sourceName);
          unplacedNodeIds.push_back(source->id);
          break;
        }
        case VideoSource_icon:
        {
          source = ActionService::getService()->addIconVideoSource(availableSource->sourceName);
          unplacedNodeIds.push_back(source->id);
          break;
        }
        case VideoSource_shader:
        {
          auto availableShaderSource = std::dynamic_pointer_cast<AvailableVideoSourceShader>(availableSource);
          source = ActionService::getService()->addShaderVideoSource(availableShaderSource->shaderType);
          unplacedNodeIds.push_back(source->id);
          break;
        }
        case VideoSource_library:
        {
          auto availableLibrarySource = std::dynamic_pointer_cast<AvailableVideoSourceLibrary>(availableSource);
          auto file = availableLibrarySource->libraryFile;
          source = ActionService::getService()->addLibraryVideoSource(file);
          unplacedNodeIds.push_back(source->id);
          break;
        }
        case VideoSource_multi:
        {
          //          source = VideoSourceService::getService()->addMultiVideoSource(availableSource->sourceName);
          //          unplacedNodeIds.push_back(source->id);
          break;
        }
        case VideoSource_playlist:
        {
          source = ActionService::getService()->addPlaylistVideoSource(availableSource->sourceName);
          unplacedNodeIds.push_back(source->id);
          break;
        }
        case VideoSource_empty:
          break;
      }
      
      
      // Multisource implementation, not needed for now
      
      //      auto within = nodeAtPosition(ed::ScreenToCanvas(getScaledMouseLocation()));
      //      if (within != nullptr && within->type == NodeTypeSource && source != nullptr) {
      //        // Remove the pending source from unplaced
      //        unplacedNodeIds.pop_back();
      //
      //        std::shared_ptr<MultiSource> multiSource = VideoSourceService::getService()->addMultiVideoSource(within->name);
      //        multiSource->addSource(source);
      //        multiSource->addSource(within->source);
      //        unplacedNodeIds.push_back(multiSource->id);
      //
      //        ShaderChainerService::getService()->copyConnections(within->source, multiSource);
      //        ActionService::getService()->removeVideoSource(within->source->id);
      //      }
      
      auto canvasPos = ed::ScreenToCanvas(getScaledMouseLocation());
      nodeDropLocation = std::make_unique<ImVec2>(canvasPos);
    }
    
    // Available Shader Chainer drop zone
    if (const ImGuiPayload *payload = ImGui::AcceptDragDropPayload("AvailableStrand"))
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
  pendingFiles.push_back(std::make_unique<LibraryFile>(file));
}

void NodeLayoutView::handleUnplacedDownloadedLibraryFile()
{
  if (pendingFiles.size() == 0)
  {
    return;
  }
  
  for (std::shared_ptr<LibraryFile> pendingFile : pendingFiles) {
    
    auto source = ActionService::getService()->addFileVideoSource(pendingFile->name, pendingFile->videoPath());
    unplacedNodeIds.push_back(source->id);
  }
  
  pendingFiles.clear();
}

void NodeLayoutView::queryNewLinks()
{
  ed::PinId inputPinId, outputPinId;
  std::vector<ed::NodeId> selectedNodesIds;
  // Attempt to create a link with the passed input/output
  if (ed::QueryNewLink(&outputPinId, &inputPinId))
  {
    if (inputPinId != outputPinId && inputPinId && outputPinId) // both are valid, let's accept link
    {
      
      std::shared_ptr<Node> sourceNode = pinIdNodeMap[outputPinId.Get()];
      std::shared_ptr<Node> destNode = pinIdNodeMap[inputPinId.Get()];
      
      InputSlot inputSlot = InputSlotNone;
      OutputSlot outputSlot = OutputSlotNone;
      
      if (sourceNode == nullptr && destNode == nullptr) { return; }
      
      // Get the InputSlot for this connection
      for (auto [slot, inputId] : destNode->inputIds) {
        if (inputId == inputPinId) inputSlot = slot;
      }
      
      // Get the OutputSlot for this connection
      for (auto [slot, outputId] : sourceNode->outputIds) {
        if (outputId == outputPinId) outputSlot = slot;
      }
      
      // If we're trying to connect Output -> Output and want to Blend
      if (inputSlot == InputSlotNone) {
        if (ed::AcceptNewItem()) {
          createBlendShaderForSelectedNodes(sourceNode->id, destNode->id);
        }
        return;
      }
      
      
      // We have a Shader destination
      if (destNode->type == NodeTypeShader)
      {
        if (ed::AcceptNewItem())
        {
          if (destNode->hasInputLinkAt(inputSlot)) {
            InputSlot slot = inputSlot;
            while (slot < destNode->connectable->inputCount()) {
              if (!destNode->connectable->hasInputAtSlot(slot)) { break; }
              slot = (InputSlot)((int) slot + 1);
            }
            inputSlot = slot;
          }
          
          // VideoSource is our Source
          if (sourceNode->type == NodeTypeSource)
          {
            ActionService::getService()->makeConnection(sourceNode->source, destNode->shader, ConnectionTypeSource, outputSlot, inputSlot, true);
          }
          
          // Shader is our Source
          if (sourceNode->type == NodeTypeShader)
          {
            ActionService::getService()->makeConnection(sourceNode->shader, destNode->shader, ConnectionTypeShader, outputSlot, inputSlot, true);
          }
        }
      }
    }
  }
}

bool NodeLayoutView::canDelete() {
  return shouldDelete && !ImGui::IsAnyItemActive();
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
  ed::SetCurrentEditor(context);
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
  ed::SetCurrentEditor(nullptr);
}

void NodeLayoutView::drawPreviewWindow(std::shared_ptr<Node> node)
{
  auto sizeScale = ed::GetCurrentZoom();
  
  auto windowSize = LayoutStateService::getService()->previewSize(sizeScale);
  
  auto pos = ed::GetNodePosition(node->id);
  pos = ed::CanvasToScreen(pos);
  
  // Adjust the position to account for the new size
  pos.x = pos.x + (ed::GetNodeSize(node->id).x / ed::GetCurrentZoom()) / 2 - (windowSize.x / 2);
  ofRectangle previewRect = ofRectangle(pos.x, pos.y, windowSize.x, windowSize.y);
  pos.y = pos.y - windowSize.y - 10;
  
  auto style = ImGui::GetStyle();
  
  ImGui::SetCursorPos(pos);
  if (sizeScale > 5.0)
  {
    sizeScale = 5.0;
  }
  node->drawPreview(pos, sizeScale);
}

void NodeLayoutView::drawActionButtons()
{
  ImVec2 pos = ImGui::GetCursorPos();
  int buttonCount = 6;
  float buttonWidth = 30.0;
  ImVec2 imageSize = ImVec2(buttonWidth, buttonWidth);
  ImVec2 imageRatio = ImVec2(1.5, 1.5);
  float shaderInfoPaneWidth = LayoutStateService::getService()->shouldDrawShaderInfo() ? LayoutStateService::getService()->browserSize().x : 0;
  
  // Position for either the collapsed or expanded action bar
  float yPos = getScaledWindowHeight() - LayoutStateService::getService()->audioSettingsViewHeight() - 64.0;
  
  if (!LayoutStateService::getService()->actionBarExpanded) {
    // Draw collapsed state with single button
    ImVec2 collapsedPos = ImVec2(getScaledWindowWidth() - 42 - shaderInfoPaneWidth, yPos);
    ImGui::SetCursorPos(collapsedPos);
    
    ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
    ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
    ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::ActionBarBackgroundColor.Value);
    ImGui::BeginChild("##CollapsedActionBar", ImVec2(buttonWidth, buttonWidth), ImGuiChildFlags_None, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoDecoration);
    
    if (CommonViews::ImageButton("expand", "back.png", imageSize, imageRatio, true)) {
      LayoutStateService::getService()->actionBarExpanded = true;
    }
    
    ImGui::EndChild();
    ImGui::PopStyleVar(2);
    ImGui::PopStyleColor();
    ImGui::SetCursorPos(pos);
    return;
  }
  
  // Draw expanded action bar
  float actionBarWidth = buttonWidth * buttonCount * imageRatio.x + shaderInfoPaneWidth + 30.0;
  ImGui::SetCursorPos(ImVec2(getScaledWindowWidth() - actionBarWidth - 12.0, yPos));
  
  ImGui::PushStyleColor(ImGuiCol_ChildBg, Colors::ActionBarBackgroundColor.Value);
  ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, 8.0f);
  ImGui::PushStyleVar(ImGuiStyleVar_ChildBorderSize, 1.0f);
  ImGui::SetNextItemAllowOverlap();
  
  
  ImGui::BeginChild("##ActionButtons", ImVec2(actionBarWidth, buttonWidth), ImGuiChildFlags_None, ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
  
  // Collapse button
  if (CommonViews::ImageButton("collapse", "forward.png", imageSize, imageRatio, true)) {
    LayoutStateService::getService()->actionBarExpanded = false;
  }
  ImGui::SameLine(0, 10);
  
  // Draw the undo button
  if (CommonViews::ImageButton("undo", "undo.png", imageSize, imageRatio, true)) {
    if (ActionService::getService()->canUndo()) {
      ActionService::getService()->undo();
    }
  }
  if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    ImGui::SetTooltip("Undo the last action (Cmd+Z)");
  ImGui::SameLine(0, 10);
  
  // Draw the redo button
  if (CommonViews::ImageButton("redo", "redo.png", imageSize, imageRatio, true)) {
    if (ActionService::getService()->canRedo()) {
      ActionService::getService()->redo();
    }
  }
  if (ImGui::IsItemHovered(ImGuiHoveredFlags_AllowWhenDisabled))
    ImGui::SetTooltip("Redo the last undone action (Cmd+Y)");
  ImGui::SameLine(0, 10);
  
  // Draw the capture screenshot button
  if (CommonViews::ImageButton("capture", "camera.png", imageSize, imageRatio, true)) {
    VideoSourceService::getService()->captureOutputWindowScreenshot();
  }
  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Capture a screenshot of the output window");
  ImGui::SameLine(0, 10);
  
  // Draw the help button
  std::string helpImage = LayoutStateService::getService()->helpEnabled ? "help-outline.png" : "help.png";
  if (CommonViews::ImageButton("help", helpImage, imageSize, imageRatio, true)) {
    LayoutStateService::getService()->helpEnabled = !LayoutStateService::getService()->helpEnabled;
  }
  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Toggle help overlay");
  ImGui::SameLine(0, 10);
  
  // Draw the reset button
  if (CommonViews::ImageButton("reset", "delete.png", imageSize, imageRatio, true)) {
    clear();
  }
  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Remove all nodes and connections (Cmd+N)");
  ImGui::SameLine(0, 10);
  
  // Draw the navigate to content button
  if (CommonViews::ImageButton("navigate", "focus.png", imageSize, imageRatio, true)) {
    ed::SetCurrentEditor(context);
    ed::NavigateToContent();
    ed::SetCurrentEditor(nullptr);
  }
  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Scale to fit content (100\%)");
  ImGui::SameLine(0, 10);
  
//  // Draw the stage mode button
//  if (CommonViews::ImageButton("stage", "stage.png", imageSize, imageRatio, true)) {
//    LayoutStateService::getService()->stageModeEnabled = true;
//  }
//  if (ImGui::IsItemHovered())
//    ImGui::SetTooltip("Enable stage mode (Cmd+B)");
  ImGui::SameLine(0, 10);
  
  // Draw the random strand button
  if (CommonViews::ImageButton("random", "dice.png", imageSize, imageRatio, true)) {
    clear();
    ShaderChainerService::getService()->randomStrand(
                                                     VideoSourceService::getService()->availableShaderSources,
                                                     ShaderChainerService::getService()->availableFilterShaders
                                                     );
  }
  if (ImGui::IsItemHovered())
    ImGui::SetTooltip("Create Random Filter Chain");
  
  ImGui::EndChild();
  ImGui::PopStyleVar(2);
  ImGui::PopStyleColor();
  ImGui::SetCursorPos(pos);
}

void NodeLayoutView::clear() {
  idNodeMap.clear();
  nodeIdNodeMap.clear();
  previewWindowNodes.clear();
  terminalNodes.clear();
  nodesToOpen.clear();
  selectorNodeId = ed::NodeId();
  ed::SetCurrentEditor(context);
  ShaderChainerService::getService()->clear();
  VideoSourceService::getService()->clear();
  ParameterService::getService()->clear();
  ActionService::getService()->clear();
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
  //  ed::ShowMetrics();
}

void NodeLayoutView::drawDebugWindow()
{
  auto cursorPos = ImGui::GetCursorPos();
  
  float shaderInfoPaneWidth = LayoutStateService::getService()->shouldDrawShaderInfo() ? LayoutStateService::getService()->browserSize().x : 0;
  ImGui::SetCursorPos(ImVec2(getScaledWindowWidth() - 400.0 - shaderInfoPaneWidth, getScaledWindowHeight() - LayoutStateService::getService()->audioSettingsViewHeight() - 600.0));
  
  ImGui::BeginChild("Debug Window", ImVec2(300.0, 300.0));
  // Draw live debug information about the state of the app
  auto shaders = ShaderChainerService::getService()->shaders();
  auto connections = ShaderChainerService::getService()->connections();
  auto sources = VideoSourceService::getService()->videoSources();
  
  ImGui::Text("Shaders:");
  for (const auto& shader : shaders) {
    ImGui::BulletText("Name: %s, ID: %s", shader->name().c_str(), shader->shaderId.c_str());
  }
  
  ImGui::Separator();
  ImGui::Text("%s", formatString("Connections: %d", connections.size()).c_str());
  for (const auto& connection : connections) {
    ImGui::BulletText("Start: %s, End: %s", connection->start->name().c_str(), connection->end->name().c_str());
  }
  
  ImGui::Separator();
  ImGui::Text("Video Sources:");
  for (const auto& source : sources) {
    ImGui::BulletText("Name: %s, ID: %s", source->sourceName.c_str(), source->id.c_str());
  }
  ImGui::EndChild();
  
  ImGui::SetCursorPos(cursorPos);
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
  
  // Ask the user for a destination (.json) path using a native save dialog
  std::string defaultJsonName = fileName + ".json";
  ofFileDialogResult result = ofSystemSaveDialog(defaultJsonName, "Save Strand");
  if (!result.bSuccess)
  {
    return; // User cancelled the dialog
  }
  
  std::string jsonPath = result.getPath();
  
  // Preview image is stored inside the application's strands folder (always writable inside sandbox)
  std::string previewPath = StrandService::getService()->strandPreviewPath(fileName);
  
  // Save the preview image to disk
  StrandService::getService()->savePreviewToPath(previewPath, nodeToSave->connectable);
  
  // Finally write the Strand JSON file and register it with the application
  ConfigService::getService()->saveStrandFile(strand, jsonPath, previewPath);

  // Persist security-scoped bookmark ONLY for the user-chosen json file (preview lives in container)
  BookmarkService::getService()->saveBookmarkForPath(jsonPath);
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
    ImGui::SetCursorPos(ImVec2(LayoutStateService::getService()->browserSize().x + 20.0, getScaledWindowHeight() / 3.0 + 200.0));
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
  
  if (!HelpService::getService()->placedBlendEffect() && !HelpService::getService()->deletedFirstBlendNode())
  {
    bool hasPayload = ImGui::GetDragDropPayload();
    ImGui::SetCursorPos(ImVec2(LayoutStateService::getService()->browserSize().x + 20.0, getScaledWindowHeight() / 3.0 + 100.0));
    
    // Check if our payload is a Blend shader
    auto payload = ImGui::GetDragDropPayload();
    bool isShader = payload && payload->IsDataType("NewShader");
    bool isBlend = false;
    if (isShader) {
      int n = *(const int *)payload->Data;
      ShaderType shaderType = (ShaderType)n;
      isBlend = shaderType == ShaderTypeBlend;
    }
    
    if (hasPayload && isBlend) {
      HelpService::getService()->drawBlendEffect2View();
    } else if (hasPayload && !isBlend) {
      HelpService::getService()->drawWrongEffectView();
    } else {
      HelpService::getService()->drawBlendEffectView();
      ImGui::NewLine();
    }
    // Reset our cursor and return without drawing additional help
    ImGui::SetCursorPos(cursorPos);
    return;
  }
  
  // Making a connection to our Blend shader
  if (!HelpService::getService()->madeFirstBlendConnection() && !HelpService::getService()->deletedFirstBlendNode()) {
    if (!ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
      
      // Get a terminal node
      std::shared_ptr<Node> terminalNode;
      for (auto node : terminalNodes) {
        if (node->type == NodeTypeShader && !ShaderChainerService::getService()->isShaderType(node->connectable, ShaderTypeBlend)) {
          terminalNode = node;
          break;
        }
      }
      if (terminalNode == nullptr) {
        // Reset our cursor and return without drawing additional help
        ImGui::SetCursorPos(cursorPos);
        return;
      }
      
      HelpService::getService()->drawBlendConnectionView(ed::CanvasToScreen(terminalNode->position), ed::GetNodeSize(terminalNode->id), ed::GetCurrentZoom());
    } else {
      // Get a Blend node
      std::shared_ptr<Node> blendNode;
      for (auto node : nodes) {
        if (ShaderChainerService::getService()->isShaderType(node->connectable, ShaderTypeBlend)) {
          blendNode = node;
        };
      }
      
      HelpService::getService()->drawBlendConnection2View(ed::CanvasToScreen(blendNode->position), ed::GetNodeSize(blendNode->id), ed::GetCurrentZoom());
    }
    
    // Reset our cursor and return without drawing additional help
    ImGui::SetCursorPos(cursorPos);
    return;
  }
  
  // Making a second connection to our Blend shader
  if (!HelpService::getService()->madeSecondBlendConnection() &&
      !HelpService::getService()->deletedFirstBlendNode()) {
    if (!ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
      
      // Get a terminal node
      std::shared_ptr<Node> terminalNode;
      for (auto node : terminalNodes) {
        if (!ShaderChainerService::getService()->isShaderType(node->connectable, ShaderTypeBlend) && node->connectable->outputs.size() == 0) {
          terminalNode = node;
          break;
        }
      }
      
      if (terminalNode == nullptr) {
        ImGui::SetCursorPos(cursorPos);
        return;
      }
      
      HelpService::getService()->drawSecondBlendConnectionView(ed::CanvasToScreen(terminalNode->position), ed::GetNodeSize(terminalNode->id), ed::GetCurrentZoom());
    } else {
      // Get a Blend node
      std::shared_ptr<Node> blendNode;
      for (auto node : nodes) {
        if (ShaderChainerService::getService()->isShaderType(node->connectable, ShaderTypeBlend)) {
          blendNode = node;
        };
      }
      
      HelpService::getService()->drawSecondBlendConnection2View(ed::CanvasToScreen(blendNode->position), ed::GetNodeSize(blendNode->id), ed::GetCurrentZoom());
    }
    
    // Reset our cursor and return without drawing additional help
    ImGui::SetCursorPos(cursorPos);
    return;
  }
  
  // Open the ShaderInfo pane
  if (!HelpService::getService()->openedShaderInfo() && !HelpService::getService()->deletedFirstBlendNode()) {
    // Get a Blend node
    std::shared_ptr<Node> blendNode;
    for (auto node : nodes) {
      if (ShaderChainerService::getService()->isShaderType(node->connectable, ShaderTypeBlend)) {
        blendNode = node;
      };
    }
    
    if (blendNode == nullptr) {
      ImGui::SetCursorPos(cursorPos);
      return;
    }
    
    HelpService::getService()->drawOpenShaderInfoView(ed::CanvasToScreen(blendNode->position), ed::GetNodeSize(blendNode->id), ed::GetCurrentZoom());
    
    // Reset our cursor and return without drawing additional help
    ImGui::SetCursorPos(cursorPos);
    return;
  }
  
  if (!HelpService::getService()->editedBlendModeParameter() || !HelpService::getService()->editedAlphaParameter()) {
    HelpService::getService()->drawEditParametersShaderInfoPane();
    
    // Reset our cursor and return without drawing additional help
    ImGui::SetCursorPos(cursorPos);
    return;
  }
  
  if (!HelpService::getService()->deletedFirstBlendNode()) {
    HelpService::getService()->drawDeleteBlendShader();
    
    // Reset our cursor and return without drawing additional help
    ImGui::SetCursorPos(cursorPos);
    return;
  }
  
  // Made a new Blend shader by connecting two outputs
  if (!HelpService::getService()->placedBlendEffect()) {
    std::shared_ptr<Node> terminalNode;

    for (auto node : terminalNodes) {
      if (node->type == NodeTypeShader && !ShaderChainerService::getService()->isShaderType(node->connectable, ShaderTypeBlend)) {
        terminalNode = node;
        break;
      }
    }
      if (terminalNode == nullptr) {
        // Reset our cursor and return without drawing additional help
        ImGui::SetCursorPos(cursorPos);
        return;
      }
      if (ImGui::IsMouseDragging(ImGuiMouseButton_Left)) {
        HelpService::getService()->drawBlendNodeShortcut2(ed::CanvasToScreen(terminalNode->position), ed::GetNodeSize(terminalNode->id), ed::GetCurrentZoom());
        
        ImGui::SetCursorPos(cursorPos);
        return;
      } else {
        std::shared_ptr<Node> secondNode;
        
        for (auto node : terminalNodes) {
          if (node->type == NodeTypeSource) { secondNode = node; }
        }
        HelpService::getService()->drawBlendNodeShortcut(ed::CanvasToScreen(secondNode->position), ed::GetNodeSize(secondNode->id), ed::GetCurrentZoom());
        
        ImGui::SetCursorPos(cursorPos);
        return;
    }
  }
  if (!HelpService::getService()->showPopup && !HelpService::getService()->completed) {
    HelpService::getService()->drawActionButtons();
    // Reset our cursor and return without drawing additional help
    ImGui::SetCursorPos(cursorPos);
    return;
  }
  
  if (HelpService::getService()->showPopup) {
    HelpService::getService()->drawCompletionPopup();
    // Reset our cursor and return without drawing additional help
    ImGui::SetCursorPos(cursorPos);
    return;
  }
}

void NodeLayoutView::toggleAudioReactiveParameter(std::shared_ptr<Node> node) {
  if (node->isAudioReactiveParameterActive()) {
    node->connectable->settingsRef()->audioReactiveParameter->removeDriver();
    return;
  }
  
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
  node->connectable->settingsRef()->audioReactiveParameter->addDriver(updateParameter);
}

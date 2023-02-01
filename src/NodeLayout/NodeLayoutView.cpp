//
//  NodeLayoutView.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/5/23.
//

#include "NodeLayoutView.hpp"
#include "ShaderChainerService.hpp"
#include "Strings.hpp"
#include "VideoSourceService.hpp"
#include "LayoutStateService.hpp"
#include "FontService.hpp"
#include "Fonts.hpp"
#include "ofMain.h"
#include <imgui.h>
#include <imgui_node_editor.h>

static const long NullId = -1000;
static const float ImGuiWindowTitleBarHeight = 70.0f;

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

void NodeLayoutView::draw() {
  auto &io = ImGui::GetIO();
  
  auto chainers = ShaderChainerService::getService()->shaderChainers();
  auto shaders = ShaderChainerService::getService()->shaders();
  
  ed::SetCurrentEditor(context);
  
  bool show = LayoutStateService::getService()->showAudioSettings;
  float height = show ?  ofGetViewportHeight() * 3.0f / 5.0f : ofGetViewportHeight() - ImGuiWindowTitleBarHeight;
  
  // Start interaction with editor.
  ed::Begin("My Editor",
            ImVec2(ofGetWindowWidth() * (3.0f / 5.0f), height));
  
  ed::PushStyleVar(ed::StyleVar_ScrollDuration, 0.1f);
  int uniqueId = 1;
  int sourceUniqueId = 10000;
  linksMap.clear();
  nodeIdNodeMap.clear();
  pinIdPinMap.clear();
  pinIdNodeMap.clear();
  
  // Loop over Shaders, creating a ShaderNode for each
  for (std::shared_ptr<Shader> shader : shaders) {
    ed::NodeId nodeId = uniqueId++;
    ed::PinId inputPinId = uniqueId++;
    ed::PinId outputPinId = uniqueId++;
    
    auto shaderNode =
    std::make_shared<Node>(nodeId, outputPinId, inputPinId, NodeTypeShader);
    
    ed::BeginNode(nodeId);
    ImGui::Text("%s", shader->name().c_str());
    
    ImGui::SameLine();
    // Open the ShaderSettings when tapped
    
    if (CommonViews::IconButton(ICON_FAD_XLRPLUG, shader->id())) {
      bool inSet = shadersToOpen.find(shader) != shadersToOpen.end();
      if (!inSet) {
        shadersToOpen.insert(shader);
      } else {
        shadersToOpen.erase(shader);
      }
      
    }
    // Make input/output pins for each ShaderNode
    ed::BeginPin(inputPinId, ed::PinKind::Input);
    ImGui::Text("In");
    ed::EndPin();
    auto inPin = std::make_shared<Pin>(inputPinId, shaderNode, PinTypeInput);
    
    ed::BeginPin(outputPinId, ed::PinKind::Output);
    ImGui::Text("Out");
    ed::EndPin();
    auto outPin = std::make_shared<Pin>(outputPinId, shaderNode, PinTypeOutput);
    
    // Create an Aux pin if the Shader supports it
    if (shader->supportsAux()) {
      ed::PinId auxPinId = uniqueId++;
      ed::BeginPin(auxPinId, ed::PinKind::Input);
      ImGui::Text("Aux (In)");
      ed::EndPin();
      auto auxPin = std::make_shared<Pin>(auxPinId, shaderNode, PinTypeAux);
      
      pinIdNodeMap[auxPinId.Get()] = shaderNode;
      pinIdPinMap[auxPinId.Get()] = auxPin;
      shaderNode->auxId = auxPinId;
    }
    
    ed::EndNode();
    
    shaderNode->shader = shader;
    idNodeMap[shader->id()] = shaderNode;
    nodeIdNodeMap[nodeId.Get()] = shaderNode;
    pinIdNodeMap[inputPinId.Get()] = shaderNode;
    pinIdNodeMap[outputPinId.Get()] = shaderNode;
    pinIdPinMap[inputPinId.Get()] = inPin;
    pinIdPinMap[outputPinId.Get()] = outPin;
  }
  
  if (shaderDropLocation != nullptr) {
    ed::SetNodePosition(idNodeMap[shaders.back()->id()].get()->id,
                        *shaderDropLocation.get());
    shaderDropLocation = nullptr;
  }
  
  // Create VideoSource Nodes
  for (std::shared_ptr<ShaderChainer> shaderChainer : chainers) {
    // Create a Link to the front shader, if it exists
    // Make source node
    ed::NodeId sourceNodeId = sourceUniqueId++;
    ed::PinId sourceNodeOutId = sourceUniqueId++;
    
    auto sourceNode = std::make_shared<Node>(sourceNodeId, sourceNodeOutId,
                                             NullId, NodeTypeSource);
    sourceNode->source = shaderChainer->source;
    sourceNode->chainer = shaderChainer;
    
    ed::BeginNode(sourceNodeId);
    ImGui::Text("%s", shaderChainer->source->sourceName.c_str());
    
    ed::BeginPin(sourceNodeOutId, ed::PinKind::Output);
    ImGui::Text("Output");
    ed::EndPin();
    
    // If our Source Node is connected, add a Link
    if (shaderChainer->front != nullptr) {
      ed::LinkId sourceNodeFrontShaderLink = sourceUniqueId++;
      auto shaderNode = idNodeMap[shaderChainer->front->shaderId];
      ed::Link(sourceNodeFrontShaderLink, sourceNodeOutId, shaderNode->inputId);
      
      // Create the ShaderLink
      auto link = std::make_shared<ShaderLink>(sourceNodeFrontShaderLink,
                                               sourceNode, shaderNode);
      link->type = LinkTypeSource;
      linksMap[sourceNodeFrontShaderLink.Get()] = link;
    }
    
    pinIdNodeMap[sourceNodeOutId.Get()] = sourceNode;
    ed::EndNode();
    
    idNodeMap[shaderChainer->source->id] = sourceNode;
    nodeIdNodeMap[sourceNode->id.Get()] = sourceNode;
  }
  
  if (sourceDropLocation != nullptr) {
    ed::SetNodePosition(idNodeMap[chainers.back()->source->id].get()->id,
                        *sourceDropLocation.get());
    sourceDropLocation = nullptr;
  }
  
  // Loop over ShaderNodes and draw links
  
  for (auto const& x : nodeIdNodeMap) {
    // Only create Links between ShaderNodes
    if (x.second->type != NodeTypeShader) continue;
    
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
    if (shaderNode->shader->supportsAux() && shaderNode->shader->aux != nullptr) {
      std::shared_ptr<Node> auxNode = idNodeMap[shaderNode->shader->aux->id()];
      ed::LinkId nextShaderLinkId = uniqueId++;
      shaderNode->auxNode = auxNode;
      auto shaderLink =
      std::make_shared<ShaderLink>(nextShaderLinkId, shaderNode, auxNode);
      shaderLink->type = LinkTypeAux;
      linksMap[nextShaderLinkId.Get()] = shaderLink;
      ed::Link(nextShaderLinkId, shaderLink->output->outputId, shaderLink->input->auxId);
    }
  }
  
  std::vector<ed::LinkId> selectedLinksIds;
  std::vector<ed::NodeId> selectedNodesIds;
  selectedLinksIds.resize(ed::GetSelectedObjectCount());
  selectedNodesIds.resize(ed::GetSelectedObjectCount());
  
  // Delete the selected Links
  ed::GetSelectedLinks(selectedLinksIds.data(), selectedLinksIds.size());
  for (auto selectedLinkId : selectedLinksIds) {
    // Remove the Shader link
    std::shared_ptr<ShaderLink> link = linksMap[selectedLinkId.Get()];
    if (shouldDelete && link != nullptr) {
      switch (link->type) {
        case LinkTypeSource: {
          auto chainer = ShaderChainerService::getService()->shaderChainerForShaderId(link->output->shader->id());
          ShaderChainerService::getService()->breakShaderChainerFront(chainer);
          break;
        }
        case LinkTypeAux: {
          ShaderChainerService::getService()->breakShaderAuxLink(link->output->shader, link->input->shader);
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
  
  ed::GetSelectedNodes(selectedNodesIds.data(), selectedNodesIds.size());
  for (auto selectedNodeId : selectedNodesIds) {
    std::shared_ptr<Node> node = nodeIdNodeMap[selectedNodeId.Get()];
    if (node == nullptr) {
      nodeIdNodeMap.erase(selectedNodeId.Get());
      continue;
    }
    
    if (node->id == selectedNodeId) {
      if (shouldDelete)
        switch (node->type) {
          case NodeTypeSource: {
            VideoSourceService::getService()->removeVideoSource(node->source->id);
            ShaderChainerService::getService()->removeShaderChainer(node->chainer->chainerId);
            break;
          }
          case NodeTypeShader: {
            ShaderChainerService::getService()->removeShader(node->shader);
            break;
          }
        }
      else
        ShaderChainerService::getService()->selectShader(node->shader);
    }
  }
  
  if (ed::BeginCreate()) {
    queryNewLinks();
  }
  
  ed::EndCreate();
  ed::End();
  
  drawNodeWindows();
  
  handleDropZone();
  
  if (firstFrame)
    ed::NavigateToContent(0.0f);
  
  ed::SetCurrentEditor(nullptr);
  
  firstFrame = false;
  shouldDelete = false;
//  ImGui::ShowDemoWindow();
  // ImGui::ShowMetricsWindow();
}

void NodeLayoutView::keyReleased(int key) {
  if (key == OF_KEY_BACKSPACE) {
    shouldDelete = true;
  }
}

void NodeLayoutView::handleDropZone() {
  if (ImGui::BeginDragDropTarget()) {
    // Shader Drop Zone
    if (const ImGuiPayload *payload =
        ImGui::AcceptDragDropPayload("NewShader")) {
      int n = *(const int *)payload->Data;
      ShaderType shaderType = (ShaderType)n;
      ShaderChainerService::getService()->makeShader(shaderType);
      auto canvasPos = ed::ScreenToCanvas(ImVec2(ofGetMouseX(), ofGetMouseY()));
      shaderDropLocation = std::make_unique<ImVec2>(canvasPos);
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
          break;
        }
        case VideoSource_file: {
          auto source = VideoSourceService::getService()->addFileVideoSource(availableSource.sourceName, availableSource.path);
          ShaderChainerService::getService()->addNewShaderChainer(source);
          // Skip
          break;
        }
        case VideoSource_chainer:
          // Skip
          break;
        case VideoSource_shader: {
          auto source = VideoSourceService::getService()->addShaderVideoSource(
                                                                               availableSource.shaderType);
          ShaderChainerService::getService()->addNewShaderChainer(source);
          break;
        }
      }
      
      auto canvasPos = ed::ScreenToCanvas(ImVec2(ofGetMouseX(), ofGetMouseY()));
      sourceDropLocation = std::make_unique<ImVec2>(canvasPos);
    }
    ImGui::EndDragDropTarget();
  }
}

void NodeLayoutView::queryNewLinks() {
  ed::PinId inputPinId, outputPinId;
  
  if (ed::QueryNewLink(&outputPinId, &inputPinId)) {
    if (inputPinId && outputPinId) // both are valid, let's accept link
    {
      std::shared_ptr<Node> sourceNode = pinIdNodeMap[outputPinId.Get()];
      std::shared_ptr<Node> destNode = pinIdNodeMap[inputPinId.Get()];
      if (sourceNode == nullptr || destNode == nullptr)
        return;
      
      // We have a Shader destination
      if (destNode->type == NodeTypeShader) {
        if (ed::AcceptNewItem()) {
          
          // VideoSource is our Source
          if (sourceNode->type == NodeTypeSource) {
            ShaderChainerService::getService()->addShaderToFront(
                                                                 destNode->shader, sourceNode->chainer);
          }
          
          if (sourceNode->type == NodeTypeShader) {
            auto destPin = pinIdPinMap[inputPinId.Get()];
            // If our Dest Pin is an Aux Pin
            if (destPin->pinType == PinTypeAux) {
              ShaderChainerService::getService()->setAuxShader(sourceNode->shader, destNode->shader);
              // Our Dest Pin is a regular Input
            } else if (destPin->pinType == PinTypeInput) {
              ShaderChainerService::getService()->linkShaderToNext(
                                                                   destNode->shader, sourceNode->shader);
            }
          }
          // Shader is our Source
        }
      }
    }
  }
}


void NodeLayoutView::drawNodeWindows() {
  for (auto const & shader: shadersToOpen) {
    auto node = idNodeMap[shader->id()];
    auto pos = ed::GetNodePosition(node->id);
    pos = ed::CanvasToScreen(pos);
    pos.x = pos.x + ed::GetNodeSize(node->id).x;
    ImGui::SetNextWindowPos(pos);
    auto flags = ImGuiWindowFlags_Tooltip;
    
    if (ImGui::Begin(shader->idName().c_str())) {
      shader->drawSettings();
    }
    ImGui::End();
  }
}

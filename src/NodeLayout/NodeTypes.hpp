//
//  NodeTypes.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/6/23.
//

#ifndef NodeTypes_h
#define NodeTypes_h

#include "Shader.hpp"
#include "ShaderChainer.hpp"
#include "Colors.hpp"
#include "VideoSource.hpp"
#include "imgui_node_editor.h"
#include <stdio.h>

static const long NullId = -1000;

namespace ed = ax::NodeEditor;

enum NodeType { NodeTypeSource, NodeTypeShader };
enum LinkType { LinkTypeShader, LinkTypeSource, LinkTypeAux, LinkTypeMask };
enum PinType {
  PinTypeInput,
  PinTypeOutput,
  PinTypeAux,
  PinTypeMask
};

struct Node {
  ed::NodeId id;
  ed::PinId outputId;
  ed::PinId inputId;
  ed::PinId auxId;
  ed::PinId maskId;
  std::string name;
  NodeType type;
  std::shared_ptr<Shader> shader;
  std::shared_ptr<VideoSource> source;
  std::shared_ptr<ShaderChainer> chainer;
  std::shared_ptr<Node> sourceNode;
  std::shared_ptr<Node> nextNode;
  std::shared_ptr<Node> auxNode;
  std::shared_ptr<Node> maskNode;
  
  ImVec2 position;
  
  std::string idName() {
    return formatString("%s##%d", name.c_str(), id.Get());
  }
  
  bool supportsInput() {
    return inputId.Get() != NullId;
  }
  
  bool hasAuxLink() {
    return supportsAux() &&
    (shader->aux != nullptr || shader->auxSource != nullptr);
  }
  
  bool hasMaskLink() {
    return supportsMask() &&
    (shader->mask != nullptr || shader->sourceMask != nullptr);
  }
  
  bool hasInputLink() {
    return supportsInput() &&
    (shader->parent != nullptr || shader->parentSource != nullptr);
  }
  
  bool hasOutputLink() {
    return type == NodeTypeShader ? shader->next != nullptr : chainer->front != nullptr;
  }
  
  bool supportsAux() {
    return auxId.Get() != NullId;
  }
  
  bool supportsMask() {
    return maskId.Get() != NullId;
  }
  
  // Draws the settings from the underlying Shader or VideoSource
  void drawSettings() {
    if (type == NodeTypeShader) {
      shader->drawSettings();
    } else {
      source->drawSettings();
      CommonViews::ResolutionSelector(source);
    }
  }
  
  // Draws the frame for the node
  void drawPreview(ImVec2 pos) {
    if (type == NodeTypeShader) {
      shader->drawPreview(pos);
    } else {
      source->drawPreview(pos);
    }
  }
  
  ImColor nodeColor() {
    if (type == NodeTypeSource) { return SourceNodeColor; }
    if (supportsAux()) return AuxNodeColor;
    if (supportsMask()) return MaskNodeColor;
    
    return supportsAux() ? AuxNodeColor : ShaderNodeColor;
  }

  Node(ed::NodeId id, ed::PinId outputId, ed::PinId inputId, std::string name, NodeType type)
      : id(id), outputId(outputId), inputId(inputId), type(type), name(name), position(ImVec2(0,0)) {}
};

struct Pin {
  ed::PinId id;
  std::shared_ptr<Node> node;
  PinType pinType;

  Pin(ed::PinId id, std::shared_ptr<Node> node, PinType pinType)
      : id(id), node(node), pinType(pinType) {}
};


struct ShaderLink {
  ed::LinkId id;
  std::shared_ptr<Node> input;
  std::shared_ptr<Node> output;
  LinkType type;

  ShaderLink(ed::LinkId id, std::shared_ptr<Node> inputNode,
             std::shared_ptr<Node> outputNode)
      : id(id), output(outputNode), input(inputNode), type(LinkTypeShader) {}
};

#endif /* NodeTypes_h */

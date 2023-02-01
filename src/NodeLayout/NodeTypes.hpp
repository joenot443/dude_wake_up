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
#include "VideoSource.hpp"
#include "imgui_node_editor.h"
#include <stdio.h>

namespace ed = ax::NodeEditor;

enum NodeType { NodeTypeSource, NodeTypeShader };
enum LinkType { LinkTypeShader, LinkTypeSource, LinkTypeAux };
enum PinType {
  PinTypeInput,
  PinTypeOutput,
  PinTypeAux
};

struct Node {
  ed::NodeId id;
  ed::PinId outputId;
  ed::PinId inputId;
  ed::PinId auxId;
  NodeType type;
  std::shared_ptr<Shader> shader;
  std::shared_ptr<VideoSource> source;
  std::shared_ptr<ShaderChainer> chainer;
  std::shared_ptr<Node> sourceNode;
  std::shared_ptr<Node> nextNode;
  std::shared_ptr<Node> auxNode;

  Node(ed::NodeId id, ed::PinId outputId, ed::PinId inputId, NodeType type)
      : id(id), outputId(outputId), inputId(inputId), type(type) {}
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

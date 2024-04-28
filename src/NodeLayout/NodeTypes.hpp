//
//  NodeTypes.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 1/6/23.
//

#ifndef NodeTypes_h
#define NodeTypes_h

#include "Shader.hpp"
#include "Connection.hpp"
#include "Colors.hpp"
#include "ShaderChainerService.hpp"
#include "VideoSource.hpp"
#include "imgui_node_editor.h"
#include <stdio.h>

static const long NullId = -1000;

namespace ed = ax::NodeEditor;

enum NodeType
{
  NodeTypeSource,
  NodeTypeShader
};

enum PinType
{
  PinTypeInput,
  PinTypeOutput
};

struct Node
{
  ed::NodeId id;
  ed::PinId outputId;
  std::map<InputSlot, ed::PinId> inputIds;
  std::string name;
  NodeType type;
  std::shared_ptr<Shader> shader;
  std::shared_ptr<VideoSource> source;
  std::shared_ptr<Connectable> connectable;

  ImVec2 position;

  std::string idName()
  {
    return formatString("%s##%d", name.c_str(), id.Get());
  }
  
  bool hasInputLinkAt(InputSlot slot)
  {
    return connectable->hasInputAtSlot(slot);
  }

  bool hasOutputLink()
  {
    return !connectable->outputs.empty();
  }
  // Draws the settings from the underlying Shader or VideoSource
  void drawSettings()
  {
    if (type == NodeTypeShader)
    {
      shader->drawSettings();
      ShaderConfigSelectionView::draw(shader);
    }
    else
    {
      source->drawSettings();
      CommonViews::ResolutionSelector(source);
    }
  }

  // Draws the frame for the node
  void drawPreview(ImVec2 pos, float scale)
  {
    if (type == NodeTypeShader)
    {
      shader->drawPreview(pos, scale);
    }
    else
    {
      source->drawPreview(pos, scale);
    }
  }

  ImColor nodeColor()
  {
    if (type == NodeTypeSource)
    {
      return SourceNodeColor;
    }
    static ImColor nodeColors[4] = {AuxNodeColor, MaskNodeColor, MaskNodeColor, MaskNodeColor};
    
    return nodeColors[connectable->inputCount()];
  }
  
  ImVec2 savedPosition() {
    if (type == NodeTypeShader)
    {
      return ImVec2(shader->settings->x->value, shader->settings->y->value);
    }
    else
    {
      return source->origin;
    }
  }

  Node(ed::NodeId id, ed::PinId outputId, std::map<InputSlot, ed::PinId> inputIds, std::string name, NodeType type, std::shared_ptr<Connectable> conn)
      : id(id), outputId(outputId), inputIds(inputIds), type(type), name(name), position(ImVec2(0, 0)), connectable(conn) {}
};

struct Pin
{
  ed::PinId id;
  std::shared_ptr<Node> node;
  PinType pinType;

  Pin(ed::PinId id, std::shared_ptr<Node> node, PinType pinType)
      : id(id), node(node), pinType(pinType) {}
};

struct ShaderLink
{
  ed::LinkId id;
  std::shared_ptr<Node> input;
  std::shared_ptr<Node> output;
  InputSlot inputSlot;
  std::string connectionId;

  ShaderLink(ed::LinkId id, std::shared_ptr<Node> inputNode,
             std::shared_ptr<Node> outputNode, InputSlot inputSlot, std::string connectionId)
      : id(id), output(outputNode), input(inputNode), inputSlot(inputSlot), connectionId(connectionId) {}
};

#endif /* NodeTypes_h */

//
//  Connnection.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/29/23.
//

#ifndef Connection_hpp
#define Connection_hpp

#include <stdio.h>
#include <memory>
#include <map>
#include <set>
#include <vector>
#include <string>
#include "JSONSerializable.hpp"
#include "imgui_node_editor.h"
#include "VideoSourceSettings.hpp"
#include "ShaderType.hpp"
#include "UUID.hpp"

using json = nlohmann::json;

class Connection;
class Connectable;

namespace ed = ax::NodeEditor;

enum ConnectableType
{
  ConnectableTypeSource,
  ConnectableTypeShader
};

enum ConnectionType
{
  // Shader to Shader
  ConnectionTypeShader,
  // Source to Shader
  ConnectionTypeSource
};

// 0-9
enum InputSlot
{
  InputSlotMain,
  InputSlotTwo,
  InputSlotThree,
  InputSlotFour,
  InputSlotFive,
  InputSlotSix,
  InputSlotSeven,
  InputSlotEight,
  InputSlotNine,
  InputSlotNone,
};

static const InputSlot AllInputSlots[] = {
  InputSlotMain,
  InputSlotTwo,
  InputSlotThree,
  InputSlotFour,
  InputSlotFive,
  InputSlotSix,
  InputSlotSeven,
  InputSlotEight,
  InputSlotNine
};

// 0-9
enum OutputSlot
{
  OutputSlotMain,
  OutputSlotTwo,
  OutputSlotThree,
  OutputSlotFour,
  OutputSlotFive,
  OutputSlotSix,
  OutputSlotSeven,
  OutputSlotEight,
  OutputSlotNine,
  OutputSlotNone,
  OutputSlotAux
};

static const OutputSlot StandardOutputSlots[] = {
  OutputSlotMain,
  OutputSlotTwo,
  OutputSlotThree,
  OutputSlotFour,
  OutputSlotFive,
  OutputSlotSix,
  OutputSlotSeven,
  OutputSlotEight,
  OutputSlotNine,
};

class Connection : public std::enable_shared_from_this<Connection>
{
public:
  std::string id;
  // Output
  std::shared_ptr<Connectable> start;
  // Input
  std::shared_ptr<Connectable> end;
  
  ConnectionType type;
  InputSlot inputSlot;
  OutputSlot outputSlot;
  ed::LinkId linkId;
  
  Connection(std::shared_ptr<Connectable> start,
             std::shared_ptr<Connectable> end,
             ConnectionType type,
             OutputSlot outputSlot,
             InputSlot inputSlot)
  : start(start), end(end), type(type), id(UUID::generateUUID()), inputSlot(inputSlot), outputSlot(outputSlot)
  {}
  
  json serialize();
};

class Connectable : public std::enable_shared_from_this<Connectable>
{
public:
  virtual ~Connectable() = default;
  
  std::map<InputSlot, std::shared_ptr<Connection>> inputs;
  std::map<OutputSlot, std::vector<std::shared_ptr<Connection>>> outputs; // Changed to vector
  
  // Either a Shader::shaderId or a VideoSource::id
  virtual std::string connId() = 0;
  
  // The Shader name or VideoSource name
  virtual std::string name() = 0;
  
  // The settings for the underlying Shader or VideoSource
  virtual std::shared_ptr<Settings> settingsRef() = 0;
  
  // Returns the first parent's current frame
  virtual std::shared_ptr<ofFbo> parentFrame() = 0;
  
  // The max number of inputs supported.
  virtual int inputCount() = 0;
  
  virtual void drawSettings() = 0;
  
  virtual void drawOptionalSettings() = 0;
  
  // The settings for the VideoSource itself, or the parent VideoSource,
  // or the defaultVideoSource if the Connectable has no VideoSource parent.
  virtual std::shared_ptr<VideoSourceSettings> sourceSettings() = 0;
  
  // The most recent frame rendered by this Connectable
  virtual std::shared_ptr<ofFbo> frame() = 0;
  
  virtual ConnectableType connectableType() = 0;
  
  // Whether the Connectable is processing new frames
  bool active = true;
  
  bool hasOutputs()
  {
    return !outputs.empty();
  }
  
  bool hasInputAtSlot(InputSlot slot)
  {
    return inputs.find(slot) != inputs.end();
  }
  
  bool hasOutputAtSlot(OutputSlot slot)
  {
    return outputs.find(slot) != outputs.end() && !outputs[slot].empty();
  }
  
  bool hasOutputForType(ConnectionType type)
  {
    for (auto const& [key, connections] : outputs)
    {
      for (auto const& connection : connections)
      {
        if (connection->type == type)
          return true;
      }
    }
    return false;
  }
  
  
  std::vector<OutputSlot> populatedOutputSlots()
  {
    std::vector<OutputSlot> slots;
    
    for (OutputSlot slot : StandardOutputSlots) {
      if (outputs.find(slot) != outputs.end() && !outputs[slot].empty()) {
        slots.push_back(slot);
      }
    }
    
    return slots;
  }
  
  std::vector<std::shared_ptr<Connectable> > outputConnectables() {
    std::vector<std::shared_ptr<Connectable> > connectables;
    for (auto const& [key, connections] : outputs) {
      for (auto const& connection : connections) {
        connectables.push_back(connection->end);
      }
    }
    return connectables;
  }
  
  std::vector<std::shared_ptr<Connection> > outputConnections() {
    std::vector<std::shared_ptr<Connection> > outputConnections;
    for (auto const& [key, connections] : outputs) {
      for (auto const& connection : connections) {
        outputConnections.push_back(connection);
      }
    }
    return outputConnections;
  }
  
  std::vector<std::shared_ptr<Connectable> > inputConnectables() {
    std::vector<std::shared_ptr<Connectable> > connectables;
    for (auto const& [key, connection] : inputs) {
      connectables.push_back(connection->start);
    }
    return connectables;
  }
  
  std::vector<std::shared_ptr<Connection> > inputConnections() {
    std::vector<std::shared_ptr<Connection> > inputConnections;
    for (auto const& [key, connection] : inputs) {
      inputConnections.push_back(connection);
    }
    return inputConnections;
  }
  
  OutputSlot nextAvailableOutputSlot()
  {
    for (OutputSlot slot : StandardOutputSlots) {
      if (outputs.find(slot) == outputs.end() || outputs[slot].empty()) {
        return slot;
      }
    }
    return OutputSlotNone;
  }
  
  std::shared_ptr<Connectable> inputAtSlot(InputSlot slot)
  {
    return inputs.at(slot)->start;
  }
  
  std::shared_ptr<Connectable> outputAtSlot(OutputSlot slot)
  {
    if (outputs.find(slot) != outputs.end() && !outputs[slot].empty()) {
      return outputs[slot].front()->end; // Return the first connection's end
    }
    return nullptr;
  }
  
  std::shared_ptr<Connection> connectionAtSlot(OutputSlot slot)
  {
    if (outputs.find(slot) != outputs.end() && !outputs[slot].empty()) {
      return outputs[slot].front(); // Return the first connection
    }
    return nullptr;
  }
  
  // Returns the furthest descendent of the Connectable
  std::shared_ptr<Connectable> terminalDescendent() {
    if (outputs.empty()) return shared_from_this();
    
    return outputs.begin()->second.front()->end->terminalDescendent();
  }
  
  std::shared_ptr<Connectable> parentOfType(ConnectableType type)
  {
    std::set<std::shared_ptr<Connectable>> visited;
    return parentOfType(type, visited);
  }
  
  // Private helper method that includes memoization to avoid infinite loops
  std::shared_ptr<Connectable> parentOfType(ConnectableType type, std::set<std::shared_ptr<Connectable>>& visited)
  {
    // Check if the current Connectable has already been visited to avoid loops
    if (visited.find(shared_from_this()) != visited.end())
      return nullptr;
    
    // Mark the current Connectable as visited
    visited.insert(shared_from_this());
    
    for (auto const& [key, input] : inputs)
    {
      // If the direct parent matches the type, return it
      if (input->start->connectableType() == type)
        return input->start;
      
      // Otherwise, recursively search the ancestors of the current parent
      auto ancestor = input->start->parentOfType(type, visited);
      if (ancestor != nullptr)
        return ancestor; // Found an ancestor of the specified type
    }
    return nullptr; // No ancestor of the specified type found
  }
  
  bool hasParentOfType(ConnectableType type)
  {
    std::set<std::shared_ptr<Connectable>> visited;
    return hasParentOfType(type, visited);
  }
  
  bool hasParentOfType(ConnectableType type, std::set<std::shared_ptr<Connectable>>& visited)
  {
    // Check if the current Connectable has already been visited to avoid loops
    if (visited.find(shared_from_this()) != visited.end())
      return false;
    
    // Mark the current Connectable as visited
    visited.insert(shared_from_this());
    
    for (auto const& [key, input] : inputs)
    {
      // Check if the current connectable's parent matches the type
      if (input->start->connectableType() == type)
        return true;
      
      // Recursively check if any of the ancestors match the type, passing the visited set along
      if (input->start->hasParentOfType(type, visited))
        return true;
    }
    return false; // Return false if no ancestor of the specified type is found
  }
  
  std::shared_ptr<Connectable> outputForType(ConnectionType type)
  {
    for (auto const& [key, connections] : outputs)
    {
      for (auto const& connection : connections)
      {
        if (connection->type == type)
          return connection->end;
      }
    }
    return nullptr;
  }
  
  std::shared_ptr<Connection> connectionFor(std::shared_ptr<Connectable> conn)
  {
    for (auto const& [key, connection] : inputs)
    {
      if (connection->start == conn)
        return connection;
    }
    
    for (auto const& [key, connections] : outputs)
    {
      for (auto const& connection : connections)
      {
        if (connection->end == conn)
          return connection;
      }
    }
    
    return nullptr;
  }
  
  std::vector<std::string> removeInputConnections(ConnectionType type)
  {
    std::vector<std::string> idsToRemove;
    for (auto const& [key, connection] : inputs)
    {
      if (connection->type == type)
      {
        idsToRemove.push_back(connection->id);
      }
    }
    return idsToRemove;
  }
  
  std::vector<std::string> removeOutputConnections(ConnectionType type)
  {
    std::vector<std::string> idsToRemove;
    for (auto const& [key, connections] : outputs)
    {
      for (auto const& connection : connections)
      {
        if (connection->type == type)
        {
          idsToRemove.push_back(connection->id);
        }
      }
    }
    return idsToRemove;
  }
  
  void removeConnection(std::shared_ptr<Connection> conn)
  {
    if (!conn)
      return; // Don't proceed if the connection is null

    // Remove the connection from the 'outputs' map of the starting Connectable
    if (conn->start)
    {
      // Use find() instead of operator[]
      auto output_iter = conn->start->outputs.find(conn->outputSlot);
      if (output_iter != conn->start->outputs.end()) // Check if the slot exists
      {
        auto& connections = output_iter->second; // Get reference to the vector
        connections.erase(std::remove(connections.begin(), connections.end(), conn), connections.end());

        // If it's the last Connection in that OutputSlot, remove the entry
        if (connections.empty()) {
          // Use the iterator to erase for efficiency
          conn->start->outputs.erase(output_iter);
        }
      }
    }

    // Remove the connection from the 'inputs' map of the ending Connectable
    // Use the inputSlot directly for more reliable removal
    if (conn->end)
    {
      auto input_iter = conn->end->inputs.find(conn->inputSlot);
      if (input_iter != conn->end->inputs.end()) {
        // Verify this is the same connection before erasing
        if (input_iter->second == conn || input_iter->second->id == conn->id) {
          conn->end->inputs.erase(input_iter);
        }
      }
    }
  }
  
  // Method to gather all connections
  std::vector<std::shared_ptr<Connection>> connections() {
    std::vector<std::shared_ptr<Connection>> allConnections;
    
    // Add all input connections
    for (const auto& [slot, connection] : inputs) {
      allConnections.push_back(connection);
    }
    
    // Add all output connections
    for (const auto& [slot, connectionList] : outputs) {
      allConnections.insert(allConnections.end(), connectionList.begin(), connectionList.end());
    }
    
    return allConnections;
  }
  
  std::shared_ptr<Connection> connectionAt(InputSlot slot)
  {
    auto it = inputs.find(slot);
    if (it != inputs.end()) {
      return it->second;
    }
    return nullptr;
  }
};

#endif /* Connection_hpp */

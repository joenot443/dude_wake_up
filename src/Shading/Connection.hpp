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
#include "VideoSourceSettings.hpp"
#include "ShaderType.hpp"
#include "UUID.hpp"

class Connection;
class Connectable;

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
  InputSlotNine
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
  std::map<OutputSlot, std::shared_ptr<Connection>> outputs;
  
  // Either a Shader::shaderId or a VideoSource::id
  virtual std::string connId() = 0;
  
  // The Shader name or VideoSource name
  virtual std::string name() = 0;
  
  // The max number of inputs supported.
  virtual int inputCount() = 0;
  
  virtual void drawSettings() = 0;
    
  // The settings for the VideoSource itself, or the parent VideoSource,
  // or the defaultVideoSource if the Connectable has no VideoSource parent.
  virtual std::shared_ptr<VideoSourceSettings> sourceSettings() = 0;
  
  // The most recent frame rendered by this Connectable
  virtual std::shared_ptr<ofFbo> frame() = 0;
  
  virtual ConnectableType connectableType() = 0;
  
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
    return outputs.find(slot) != outputs.end();
  }

  bool hasOutputForType(ConnectionType type)
  {
    for (auto const& [key, output] : outputs)
    {
      if (output->type == type)
        return true;
    }
    return false;
  }
  
  std::vector<OutputSlot> populatedOutputSlots()
  {
    std::vector<OutputSlot> slots;
    
    for (OutputSlot slot : StandardOutputSlots) {
      if (outputs.find(slot) != outputs.end()) {
        slots.push_back(slot);
      }
    }
    
    return slots;
  }
  
  OutputSlot nextAvailableOutputSlot()
  {
    for (OutputSlot slot : StandardOutputSlots) {
      if (outputs.find(slot) == outputs.end()) {
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
    return outputs.at(slot)->end;
  }
  
  std::shared_ptr<Connection> connectionAtSlot(OutputSlot slot)
  {
    return outputs.at(slot);
  }
  
  // Returns the furthest descendent of the Connectable
  std::shared_ptr<Connectable> terminalDescendent() {
    if (outputs.empty()) return shared_from_this();
    
    return outputs.begin()->second->end->terminalDescendent();
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
    for (auto const& [key, output] : outputs)
    {
      if (output->type == type)
        return output->end;
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
    
    for (auto const& [key, connection] : outputs)
    {
      if (connection->end == conn)
        return connection;
    }
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
    for (auto const& [key, connection] : outputs)
    {
      if (connection->type == type)
      {
        idsToRemove.push_back(connection->id);
      }
    }
    return idsToRemove;
  }
  
  // Returns the first parent's current frame
  std::shared_ptr<ofFbo> parentFrame() {
    for (auto const& [key, connection] : inputs) {
      return connection->start->frame();
    }
    return nullptr;
  }
  
  void removeConnection(std::shared_ptr<Connection> conn)
  {
    if (!conn)
      return; // Don't proceed if the connection is null
    
    // Remove the connection from the 'outputs' map of the starting Connectable
    if (conn->start)
    {
      auto it = conn->start->outputs.begin();
      while (it != conn->start->outputs.end()) {
        if (conn == it->second) {
          it = conn->start->outputs.erase(it);
        } else {
          ++it;
        }
      }
    }
    
    // Remove the connection from the 'inputs' map of the ending Connectable
    if (conn->end)
    {
      auto it = conn->end->inputs.begin();
      while (it != conn->end->inputs.end()) {
        if (conn == it->second) {
          it = conn->end->inputs.erase(it);
        } else {
          ++it;
        }
      }
    }
  }
};

#endif /* Connection_hpp */

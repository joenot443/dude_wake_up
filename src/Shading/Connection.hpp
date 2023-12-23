//
//  Connnection.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 8/29/23.
//

#ifndef Connection_hpp
#define Connection_hpp

#include <stdio.h>
#include "VideoSourceSettings.hpp"
#include "UUID.hpp"

class Connection;
class Connectable;

enum ConnectableType
{
  ConnectableTypeSource,
  ConnectableTypeShader
};

enum LinkType
{
  LinkTypeShader,
  LinkTypeSource,
  LinkTypeAux,
  LinkTypeMask
};

enum ConnectionType
{
  ConnectionTypeShader,
  ConnectionTypeSource,
  ConnectionTypeAux,
  ConnectionTypeMask
};

class Connection : std::enable_shared_from_this<Connectable>
{
public:
  std::string id;
  std::shared_ptr<Connectable> start;
  std::shared_ptr<Connectable> end;

  ConnectionType type;

  LinkType linkType()
  {
    switch (type)
    {
    case ConnectionTypeShader:
      return LinkTypeShader;
    case ConnectionTypeSource:
      return LinkTypeSource;
    case ConnectionTypeAux:
      return LinkTypeAux;
    case ConnectionTypeMask:
      return LinkTypeMask;
    }
  }
  Connection(std::shared_ptr<Connectable> start,
             std::shared_ptr<Connectable> end,
             ConnectionType type)
      : start(start), end(end), type(type), id(UUID::generateUUID())
  {
  }

  json serialize();
};

class Connectable : std::enable_shared_from_this<Connectable>
{
public:
  std::set<std::shared_ptr<Connection>> inputs;
  std::set<std::shared_ptr<Connection>> outputs;

  // Either a Shader::shaderId or a VideoSource::id
  virtual std::string connId() = 0;

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

  bool hasInputForType(ConnectionType type)
  {
    for (auto const input : inputs)
    {
      if (input->type == type)
        return true;
    }
    return false;
  }

  bool hasParentOfType(ConnectableType type)
  {
    if (connectableType() == type)
      return true;

    for (auto const input : inputs)
    {
      if (input->start->hasParentOfType(type))
        return true;
    }

    return false;
  }

  std::shared_ptr<Connectable> parentOfType(ConnectableType type)
  {

    for (auto const input : inputs)
    {
      if (input->start->connectableType() == type)
        return input->start;
    }

    for (auto const input : inputs)
    {
      if (input->start->hasParentOfType(type))
        return input->start->parentOfType(type);
    }

    return nullptr;
  }

  std::shared_ptr<Connectable> inputForType(ConnectionType type)
  {
    for (auto inputConnection : inputs)
    {
      if (inputConnection->type == type)
        return inputConnection->start;
    }
    return nullptr;
  }

  std::shared_ptr<Connection> connectionFor(std::shared_ptr<Connectable> conn)
  {
    for (auto const connection : inputs)
    {
      if (connection->start == conn)
        return connection;
    }

    for (auto const connection : outputs)
    {
      if (connection->end == conn)
        return connection;
    }
  }

  std::vector<std::string> removeInputConnections(ConnectionType type)
  {
    std::vector<std::string> idsToRemove;
    for (auto connection : inputs)
    {
      if (connection->type == type)
      {
        idsToRemove.push_back(connection->id);
      }
    }
  }

  void removeConnection(std::shared_ptr<Connection> conn)
  {
    if (!conn)
      return; // Don't proceed if the connection is null

    // Remove the connection from the 'outputs' set of the starting Connectable
    if (conn->start)
    {
      conn->start->outputs.erase(conn);
    }

    // Remove the connection from the 'inputs' set of the ending Connectable
    if (conn->end)
    {
      conn->end->inputs.erase(conn);
    }
  }
};

#endif /* Connection_hpp */

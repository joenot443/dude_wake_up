//
//  Strand.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/21/24.
//

#ifndef Strand_h
#define Strand_h

#include <ableton/Link.hpp>
#include <string>
#include "Connection.hpp"
#include "json.hpp"
#include "Shader.hpp"
#include "VideoSource.hpp"
#include "JSONSerializable.hpp"

using json = nlohmann::json;

// A collection of connections/connectables which are connected in a single graph.
struct Strand {
  std::vector<std::shared_ptr<Connection>> connections;
  std::vector<std::shared_ptr<Connectable>> connectables;
  std::string name;
  
  static std::string strandName(std::vector<std::shared_ptr<Connectable>> connectables) {
    std::string name;
    std::set<std::shared_ptr<Connectable>> visited;
    
    // Function to recursively visit connectables and concatenate their names
    std::function<void(std::shared_ptr<Connectable>)> visit = [&](std::shared_ptr<Connectable> current) {
      if (!current || visited.find(current) != visited.end()) {
        return; // Avoid revisiting connectables
      }
      
      visited.insert(current);
      // Append the current connectable's name with a separator if needed
      if (!name.empty()) {
        name += "->"; // Separator between names, adjust as necessary
      }
      name += current->name();
      // Return if the name is already greater than 20 characters
      if (name.length() > 20) return;
      
      // Visit all outputs (children) of the current connectable
      for (const auto& [slot, connections] : current->outputs) { // Correctly iterate over connections
        for (const auto& connection : connections) {
          std::shared_ptr<Connectable> end = connection->end;
          visit(end);
        }
      }
    };
    
    // Identify and start visiting from all oldest ancestors
    for (const auto& connectable : connectables) {
      if (connectable->inputs.empty()) { // This connectable has no parent, so it's an oldest ancestor
        visit(connectable);
      }
    }
    
    return name;
  }
  
  json serialize() {
    json j;
    j["name"] = name;
    j["connections"] = json::array();
    for (auto connection : connections) {
      j["connections"].push_back(connection->serialize());
    }
    j["shaders"] = json::array();
    for (auto connectable : connectables) {
      if (connectable->connectableType() == ConnectableTypeShader) {
        j["shaders"].push_back(std::dynamic_pointer_cast<Shader>(connectable)->serialize());
      }
    }
    
    j["sources"] = json::array();
    for (auto connectable : connectables) {
      if (connectable->connectableType() == ConnectableTypeSource) {
        j["sources"].push_back(std::dynamic_pointer_cast<VideoSource>(connectable)->serialize());
      }
    }
    return j;
  }
};



#endif /* Strand_h */

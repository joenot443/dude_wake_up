//
//  Strand.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2/21/24.
//

#ifndef Strand_h
#define Strand_h
#include "Connection.hpp"

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
            name += " -> "; // Separator between names, adjust as necessary
        }
        name += current->name();

        // Visit all outputs (children) of the current connectable
        for (const auto& output : current->outputs) {
            visit(output->end);
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
};



#endif /* Strand_h */

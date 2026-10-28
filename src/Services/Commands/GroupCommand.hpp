#ifndef GROUPCOMMAND_HPP
#define GROUPCOMMAND_HPP

#include "Command.hpp"

#include <memory>
#include <vector>

// Commands done as one user action (adding an effect after a node is an add
// and a connection): undone together, in reverse, and redone together.
// ActionService::beginGroup / endGroup build one.
class GroupCommand : public Command {
public:
  void add(std::shared_ptr<Command> command) { commands_.push_back(std::move(command)); }
  bool empty() const { return commands_.empty(); }

  // The children ran as they were added (in the group); a redo runs them again.
  void execute() override {
    for (auto& c : commands_) c->execute();
  }
  void undo() override {
    for (auto it = commands_.rbegin(); it != commands_.rend(); ++it) (*it)->undo();
  }

private:
  std::vector<std::shared_ptr<Command>> commands_;
};

#endif // GROUPCOMMAND_HPP

#ifndef ACTION_SERVICE_HPP
#define ACTION_SERVICE_HPP

#include "Commands/Command.hpp"
#include <stack>
#include <memory>

class ActionService {
public:
  // Executes a command and adds it to the undo stack
  void executeCommand(std::shared_ptr<Command> command);

  // Undoes the last executed command
  void undo();

  // Redoes the last undone command
  void redo();

private:
  // Stack to keep track of executed commands for undo
  std::stack<std::shared_ptr<Command>> undoStack;

  // Stack to keep track of undone commands for redo
  std::stack<std::shared_ptr<Command>> redoStack;
};

#endif // ACTION_SERVICE_HPP

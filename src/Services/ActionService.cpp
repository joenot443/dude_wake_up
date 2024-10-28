#include "ActionService.hpp"

void ActionService::executeCommand(std::shared_ptr<Command> command) {
  command->execute();
  undoStack.push(command);
  // Clear the redo stack whenever a new command is executed
  while (!redoStack.empty()) {
    redoStack.pop();
  }
}

void ActionService::undo() {
  if (!undoStack.empty()) {
    auto command = undoStack.top();
    undoStack.pop();
    command->undo();
    redoStack.push(command);
  }
}

void ActionService::redo() {
  if (!redoStack.empty()) {
    auto command = redoStack.top();
    redoStack.pop();
    command->execute();
    undoStack.push(command);
  }
}

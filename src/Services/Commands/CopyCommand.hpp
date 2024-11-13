#include "Command.hpp"
#include "ShaderChainerService.hpp"
#include "Connection.hpp"
#include "ActionService.hpp"

class CopyCommand : public Command {
public:
  CopyCommand(const std::vector<std::shared_ptr<Connectable>>& connectables)
    : connectables(connectables) {}

  void execute() override {
    // Store the connectables in the ActionService for later use by the PasteCommand
    ActionService::getService()->setCopiedConnectables(connectables);
  }

  void undo() override {
    // Copy operation doesn't need an undo
  }

private:
  std::vector<std::shared_ptr<Connectable>> connectables;
}; 
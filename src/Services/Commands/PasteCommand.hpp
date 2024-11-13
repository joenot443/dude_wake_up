#include "Command.hpp"
#include "ShaderChainerService.hpp"
#include "Connection.hpp"
#include "ActionService.hpp"
#include "NodeLayoutView.hpp"

class PasteCommand : public Command {
public:
  PasteCommand() {}

  void execute() override {
    // Retrieve the copied connectables from the ActionService
    auto copiedConnectables = ActionService::getService()->getCopiedConnectables();
    if (!copiedConnectables.empty()) {
      // Call a method on ShaderChainerService to handle pasting
      auto connectables = ShaderChainerService::getService()->pasteConnectables(copiedConnectables);
      for (auto connectable : connectables) {
        NodeLayoutView::getInstance()->addUnplacedConnectable(connectable);
      }
    }
  }

  void undo() override {
    // Implement undo logic for paste if necessary
  }
}; 

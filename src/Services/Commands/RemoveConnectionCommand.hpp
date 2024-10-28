#include "Command.hpp"
#include "../ShaderChainerService.hpp"

class RemoveConnectionCommand : public Command {
public:
  RemoveConnectionCommand(std::shared_ptr<Connection> connection) : connection(connection) {}

  void execute() override {
    ShaderChainerService::getService()->breakConnectionForConnectionId(connection->id);
  }

  void undo() override {
    ShaderChainerService::getService()->makeConnection(
      connection->start, connection->end, connection->type, 
      connection->outputSlot, connection->inputSlot
    );
  }

private:
  std::shared_ptr<Connection> connection;
};

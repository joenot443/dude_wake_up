#include "Command.hpp"
#include "../ShaderChainerService.hpp"

class MakeConnectionCommand : public Command {
public:
  MakeConnectionCommand(std::shared_ptr<Connection> connection) : connection(connection) {}

  void execute() override {
    ShaderChainerService::getService()->makeConnection(
      connection->start, connection->end, connection->type, 
      connection->outputSlot, connection->inputSlot
    );
  }

  void undo() override {
    ShaderChainerService::getService()->breakConnectionForConnectionId(connection->id);
  }

private:
  std::shared_ptr<Connection> connection;
};

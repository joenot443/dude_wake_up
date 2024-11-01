#include "Command.hpp"
#include "../ShaderChainerService.hpp"
#include "Connection.hpp"

class MakeConnectionCommand : public Command {
public:
  MakeConnectionCommand(std::shared_ptr<Connectable> start,
                        std::shared_ptr<Connectable> end,
                        ConnectionType type,
                        OutputSlot outputSlot,
                        InputSlot inputSlot,
                        bool shouldSaveConfig)
    : start(start), end(end), type(type), outputSlot(outputSlot), inputSlot(inputSlot), shouldSaveConfig(shouldSaveConfig) {}

  void execute() override {
    connection = ShaderChainerService::getService()->makeConnection(start, end, type, outputSlot, inputSlot, shouldSaveConfig);
  }

  void undo() override {
    if (connection) {
      ShaderChainerService::getService()->breakConnectionForConnectionId(connection->id);
    }
  }

  std::shared_ptr<Connection> getConnection() const {
    return connection;
  }

  bool shouldSaveConfig;
  std::shared_ptr<Connectable> start;
  std::shared_ptr<Connectable> end;
  ConnectionType type;
  OutputSlot outputSlot;
  InputSlot inputSlot;
  std::shared_ptr<Connection> connection;
};

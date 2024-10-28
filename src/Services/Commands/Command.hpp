#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <memory>

class Command {
public:
  virtual ~Command() {}
  virtual void execute() = 0;
  virtual void undo() = 0;
};

#endif // COMMAND_HPP


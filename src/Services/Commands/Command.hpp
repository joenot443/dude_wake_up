#ifndef COMMAND_HPP
#define COMMAND_HPP

#include <memory>
#include "LifecycleCounter.hpp"

class Command {
public:
  virtual ~Command() {}
  virtual void execute() = 0;
  virtual void undo() = 0;

private:
  ::lifecycle::Counter _lc_{"Command"};
};

#endif // COMMAND_HPP


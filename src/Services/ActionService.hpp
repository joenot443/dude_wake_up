#ifndef ACTION_SERVICE_HPP
#define ACTION_SERVICE_HPP

#include "Commands/Command.hpp"
#include "Commands/AddShaderCommand.hpp"
#include "Commands/RemoveShaderCommand.hpp"
#include "Commands/MakeConnectionCommand.hpp"
#include "Commands/RemoveConnectionCommand.hpp"
#include "Commands/AddImageVideoSourceCommand.hpp"
#include "Commands/AddWebcamVideoSourceCommand.hpp"
#include "Commands/AddFileVideoSourceCommand.hpp"
#include "Commands/AddTextVideoSourceCommand.hpp"
#include "Commands/AddLibraryVideoSourceCommand.hpp"
#include "Commands/AddIconVideoSourceCommand.hpp"
#include "Commands/AddShaderVideoSourceCommand.hpp"
#include "Commands/RemoveVideoSourceCommand.hpp"
#include "../Shading/ShaderType.hpp"
#include "Connection.hpp"
#include <stack>
#include <memory>

class ActionService {
public:
  // Singleton access method
  static ActionService* getService();
  
  // Executes a command and adds it to the undo stack
  void executeCommand(std::shared_ptr<Command> command);
  
  // Undoes the last executed command
  void undo();
  
  // Redoes the last undone command
  void redo();
  
  // Clears both undo and redo stacks
  void clear();
  
  // Check if undo is possible
  bool canUndo() const;
  
  // Check if redo is possible
  bool canRedo() const;
  
  // Entry points for specific commands
  std::shared_ptr<Shader> addShader(ShaderType shaderType);
  void removeShader(std::shared_ptr<Shader> shader);
  std::shared_ptr<Connection> makeConnection(std::shared_ptr<Connectable> start,
                                             std::shared_ptr<Connectable> end,
                                             ConnectionType type,
                                             OutputSlot outputSlot,
                                             InputSlot inputSlot,
                                             bool shouldSaveConfig);
  void removeConnection(std::shared_ptr<Connection> connection);
  
  // Video source commands
  std::shared_ptr<VideoSource> addImageVideoSource(const std::string& name, const std::string& path);
  std::shared_ptr<VideoSource> addWebcamVideoSource(const std::string& name, int deviceId);
  std::shared_ptr<VideoSource> addFileVideoSource(const std::string& name, const std::string& path);
  std::shared_ptr<VideoSource> addTextVideoSource(const std::string& name);
  std::shared_ptr<VideoSource> addLibraryVideoSource(std::shared_ptr<LibraryFile> libraryFile);
  std::shared_ptr<VideoSource> addIconVideoSource(const std::string& name);
  std::shared_ptr<VideoSource> addShaderVideoSource(ShaderSourceType type);
  void removeVideoSource(std::shared_ptr<VideoSource> videoSource);
  
private:
  // Private constructor to prevent instantiation
  ActionService() = default;
  
  // Stack to keep track of executed commands for undo
  std::stack<std::shared_ptr<Command>> undoStack;
  
  // Stack to keep track of undone commands for redo
  std::stack<std::shared_ptr<Command>> redoStack;
  
  // Static instance of the service
  static ActionService* service;
};

#endif // ACTION_SERVICE_HPP

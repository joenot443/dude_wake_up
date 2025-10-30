#include "ActionService.hpp"
#include "Commands/CopyCommand.hpp"
#include "Commands/PasteCommand.hpp"

// Initialize the static service instance
ActionService* ActionService::service = nullptr;

ActionService* ActionService::getService() {
  if (!service) {
    service = new ActionService();
  }
  return service;
}

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

void ActionService::clear() {
  while (!undoStack.empty()) {
    undoStack.pop();
  }
  while (!redoStack.empty()) {
    redoStack.pop();
  }
}

bool ActionService::canUndo() const {
  return !undoStack.empty();
}

bool ActionService::canRedo() const {
  return !redoStack.empty();
}

std::shared_ptr<Shader> ActionService::addShader(ShaderType shaderType) {
  auto command = std::make_shared<AddShaderCommand>(shaderType);
  executeCommand(command);
  return command->getShader();
}

void ActionService::removeShader(std::shared_ptr<Shader> shader) {
  auto command = std::make_shared<RemoveShaderCommand>(shader);
  executeCommand(command);
}

std::shared_ptr<Connection> ActionService::makeConnection(std::shared_ptr<Connectable> start,
                                                          std::shared_ptr<Connectable> end,
                                                          ConnectionType type,
                                                          OutputSlot outputSlot,
                                                          InputSlot inputSlot,
                                                          bool shouldSaveConfig) {
  auto command = std::make_shared<MakeConnectionCommand>(start, end, type, outputSlot, inputSlot, shouldSaveConfig);
  executeCommand(command);
  return command->getConnection();
}

void ActionService::removeConnection(std::shared_ptr<Connection> connection) {
  auto command = std::make_shared<RemoveConnectionCommand>(connection);
  executeCommand(command);
}

std::shared_ptr<VideoSource> ActionService::addImageVideoSource(const std::string& name, const std::string& path) {
  auto command = std::make_shared<AddImageVideoSourceCommand>(name, path);
  executeCommand(command);
  return command->getVideoSource();
}

std::shared_ptr<VideoSource> ActionService::addWebcamVideoSource(const std::string& name, int deviceId) {
  auto command = std::make_shared<AddWebcamVideoSourceCommand>(name, deviceId);
  executeCommand(command);
  return command->getVideoSource();
}

std::shared_ptr<VideoSource> ActionService::addFileVideoSource(const std::string& name, const std::string& path) {
  auto command = std::make_shared<AddFileVideoSourceCommand>(name, path);
  executeCommand(command);
  return command->getVideoSource();
}

std::shared_ptr<VideoSource> ActionService::addTextVideoSource(const std::string& name) {
  auto command = std::make_shared<AddTextVideoSourceCommand>(name);
  executeCommand(command);
  return command->getVideoSource();
}

std::shared_ptr<VideoSource> ActionService::addTypewriterTextVideoSource(const std::string& name) {
  auto command = std::make_shared<AddTypewriterTextVideoSourceCommand>(name);
  executeCommand(command);
  return command->getVideoSource();
}

std::shared_ptr<VideoSource> ActionService::addPlaylistVideoSource(const std::string& name) {
  auto command = std::make_shared<AddPlaylistVideoSourceCommand>(name, "");
  executeCommand(command);
  return command->getVideoSource();
}

std::shared_ptr<VideoSource> ActionService::addLibraryVideoSource(std::shared_ptr<LibraryFile> libraryFile) {
  auto command = std::make_shared<AddLibraryVideoSourceCommand>(libraryFile);
  executeCommand(command);
  return command->getVideoSource();
}

std::shared_ptr<VideoSource> ActionService::addIconVideoSource(const std::string& name) {
  auto command = std::make_shared<AddIconVideoSourceCommand>(name);
  executeCommand(command);
  return command->getVideoSource();
}

std::shared_ptr<VideoSource> ActionService::addShaderVideoSource(ShaderSourceType type) {
  auto command = std::make_shared<AddShaderVideoSourceCommand>(type);
  executeCommand(command);
  return command->getVideoSource();
}

void ActionService::removeVideoSource(std::shared_ptr<VideoSource> videoSource) {
  auto command = std::make_shared<RemoveVideoSourceCommand>(videoSource);
  executeCommand(command);
}

void ActionService::copy(const std::vector<std::shared_ptr<Connectable>>& connectables) {
  auto command = std::make_shared<CopyCommand>(connectables);
  executeCommand(command);
}

void ActionService::paste() {
  auto command = std::make_shared<PasteCommand>();
  executeCommand(command);
}

void ActionService::setCopiedConnectables(const std::vector<std::shared_ptr<Connectable>>& connectables) {
  copiedConnectables = connectables;
}

std::vector<std::shared_ptr<Connectable>> ActionService::getCopiedConnectables() const {
  return copiedConnectables;
}

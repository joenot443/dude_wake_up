//
//  ShaderChainerService.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/4/22.
//

#ifndef ShaderChainerService_hpp
#define ShaderChainerService_hpp

#include "AvailableShader.hpp"
#include "AvailableVideoSource.hpp"
#include "ConfigurableService.hpp"
#include "Strand.hpp"
#include "Shader.hpp"
#include "AvailableStrand.hpp"
#include "Connection.hpp"
#include "observable.hpp"
#include <stdio.h>

using json = nlohmann::json;

// Singleton service for managing ShaderChainer objects
class ShaderChainerService : public ConfigurableService
{

private:
  // Maps a shaderId to a Shader
  std::map<std::string, std::shared_ptr<Shader>> shadersMap;
  
  // Maps a connectionId to a Connection
  std::map<std::string, std::shared_ptr<Connection>> connectionMap;

  observable::subject<void()> shaderChainerUpdateSubject;

  
public:
  static ShaderChainerService *service;
  ShaderChainerService(){};
  void setup();

  static ShaderChainerService *getService()
  {
    if (!service)
    {
      service = new ShaderChainerService;
      service->setup();
    }
    return service;
  }

  static void recreateService()
  {
    service = new ShaderChainerService;
    service->setup();
  }

  void subscribeToShaderChainerUpdates(std::function<void()> callback);
  void updateShaders();
  void processFrame();
  
  std::shared_ptr<Connectable> selectedConnectable;

  // Removes all ShaderChainers and Shaders
  void reset();

  /// Accessing
  
  Strand strandForConnectable(std::shared_ptr<Connectable> connectable);
  
  bool hasParents(std::shared_ptr<Shader> shader);
  
  // Returns true if the passed Shader is the terminal node in a Chainer.
  bool isTerminalShader(std::shared_ptr<Shader> shader);
  
  bool isShaderType(std::shared_ptr<Connectable> connectable, ShaderType shaderType);
  
  // Traverses the shader's outputs to find the most terminal.
  std::shared_ptr<Shader> terminalShader(std::shared_ptr<Shader> shader);


  void selectConnectable(std::shared_ptr<Connectable> connectable);
  void deselectConnectable();
  std::shared_ptr<Shader> makeShader(ShaderType type);

  // Actions
  void addShader(std::shared_ptr<Shader> shader);
  void removeShader(std::shared_ptr<Shader> shader, bool fromMap = true);
  void removeConnectable(std::shared_ptr<Connectable> connectable);
  std::shared_ptr<Shader> replaceShader(std::shared_ptr<Shader> oldShader, ShaderType newType);
  std::vector<std::shared_ptr<Connectable>> pasteConnectables(const std::vector<std::shared_ptr<Connectable>>& connectables);
  std::shared_ptr<Connection>makeConnection(std::shared_ptr<Connectable> start,
           std::shared_ptr<Connectable> end,
           ConnectionType type,
           OutputSlot outputSlot,
           InputSlot inputSlot,
           bool shouldSaveConfig = false);

  // Break the connection from a Shader to the Input of another
  void breakConnectionForConnectionId(std::string connectionId);

  // Composite Actions
  void copyConnections(std::shared_ptr<Connectable> source, std::shared_ptr<Connectable> dest);
  
  void insert(std::shared_ptr<Connectable> start, std::shared_ptr<Connectable> connectable, OutputSlot slot);

  std::shared_ptr<Shader> shaderForId(std::string id);
  std::vector<std::shared_ptr<Shader>> shaders();
  std::vector<std::shared_ptr<Connection>> connections();

  // Available Shaders
  std::vector<std::shared_ptr<AvailableShader>> allAvailableShaders;
  std::vector<std::shared_ptr<AvailableShader>> availableBasicShaders;
  std::vector<std::shared_ptr<AvailableShader>> availableMixShaders;
  std::vector<std::shared_ptr<AvailableShader>> availableTransformShaders;
  std::vector<std::shared_ptr<AvailableShader>> availableFilterShaders;
  std::vector<std::shared_ptr<AvailableShader>> availableMaskShaders;
  std::vector<std::shared_ptr<AvailableShader>> availableDefaultFavoriteShaders;
  std::vector<std::shared_ptr<AvailableShader>> availableGlitchShaders;
  std::vector<std::shared_ptr<AvailableShader>> availableFavoriteShaders();
  std::vector<std::shared_ptr<AvailableShader>> availableRandomShaders;
  std::map<ShaderType, std::shared_ptr<AvailableShader>> availableShadersMap;

  // Auxiliary Available Shaders
  std::vector<std::shared_ptr<AvailableShader>> auxillaryAvailableBasicShaders;
  std::vector<std::shared_ptr<AvailableShader>> auxillaryAvailableMixShaders;
  std::vector<std::shared_ptr<AvailableShader>> auxillaryAvailableTransformShaders;
  std::vector<std::shared_ptr<AvailableShader>> auxillaryAvailableFilterShaders;
  std::vector<std::shared_ptr<AvailableShader>> auxillaryAvailableMaskShaders;
  std::vector<std::shared_ptr<AvailableShader>> auxillaryAvailableGlitchShaders;
  std::vector<std::shared_ptr<AvailableShader>> auxillaryAvailableFavoriteShaders;
  std::map<ShaderType, std::shared_ptr<AvailableShader>> availableAuxillaryShadersMap;
  std::vector<std::shared_ptr<AvailableShader>> allAuxillaryAvailableShaders;
  std::shared_ptr<ofFbo> basePreview;
  void hydrateAuxillaryShaders(std::shared_ptr<ofFbo> basePreview);

  // Constructor Ops
  std::shared_ptr<Shader> shaderForType(ShaderType type, std::string shaderId,
                                        json j);

  
  // Config / JSON
  void clear();
  json config() override;
  json connectionsConfig();
  void loadConfig(json j) override;
  std::vector<std::string> idsFromLoadingConfig(json j);
  void loadConnectionsConfig(json j);

  // Creates a random strand with two sources connected to two shaders, feeding into a blend shader
  void randomStrand(const std::vector<std::shared_ptr<AvailableVideoSourceShader>>& sources, 
                   const std::vector<std::shared_ptr<AvailableShader>>& shaders,
                   ImVec2 origin = ImVec2(50,0));

};

#endif /* ShaderChainerService_hpp */

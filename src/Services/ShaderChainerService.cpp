//
//  ShaderChainerService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/4/22.
//

#include "ShaderChainerService.hpp"
#include "AsciiShader.hpp"
#include "CubifyShader.hpp"
#include "SwirlingSoulShader.hpp"
#include "DoubleSwirlShader.hpp"
#include "SmokeRingShader.hpp"
#include "AudioCircleShader.hpp"
#include "LimboShader.hpp"
#include "TextureMaskShader.hpp"
#include "SnowfallShader.hpp"
#include "CannyShader.hpp"
#include "VHSShader.hpp"
#include "CirclePathShader.hpp"
#include "BounceShader.hpp"
#include "VertexShader.hpp"
#include "TripleShader.hpp"
#include "PaintShader.hpp"
#include "LumaFeedbackShader.hpp"
#include "GameboyShader.hpp"
#include "StaticFrameShader.hpp"
#include "SlidingFrameShader.hpp"
#include "MinMixerShader.hpp"
#include "SolidColorShader.hpp"
#include "16BitShader.hpp"
#include "HilbertShader.hpp"
#include "WarpShader.hpp"
#include "FrequencyVisualizerShader.hpp"
#include "ColorKeyMaskMakerShader.hpp"
#include "FeedbackSourceService.hpp"
#include "CurlySquaresShader.hpp"
#include "PlasmaTwoShader.hpp"
#include "DancingSquaresShader.hpp"
#include "LumaMaskMakerShader.hpp"
#include "MaskShader.hpp"
#include "CircleShader.hpp"
#include "CrosshatchShader.hpp"
#include "HalfToneShader.hpp"
#include "AudioBumperShader.hpp"
#include "AudioMountainsShader.hpp"
#include "ConfigService.hpp"
#include "AudioWaveformShader.hpp"
#include "AvailableShader.hpp"
#include "BlurShader.hpp"
#include "CloudShader.hpp"
#include "ColorPassShader.hpp"
#include "ConfigService.hpp"
#include "Console.hpp"
#include "DiscoShader.hpp"
#include "DitherShader.hpp"
#include "FeedbackShader.hpp"
#include "FishEyeShader.hpp"
#include "GalaxyShader.hpp"
#include "GlitchShader.hpp"
#include "HSBShader.hpp"
#include "KaleidoscopeShader.hpp"
#include "LiquidShader.hpp"
#include "MelterShader.hpp"
#include "MirrorShader.hpp"
#include "MixShader.hpp"
#include "MountainsShader.hpp"
#include "OctahedronShader.hpp"
#include "PixelShader.hpp"
#include "PsycurvesShader.hpp"
#include "RGBShiftShader.hpp"
#include "RainbowRotatorShader.hpp"
#include "RingsShader.hpp"
#include "RubiksShader.hpp"
#include "ShaderChainerService.hpp"
#include "SliderShader.hpp"
#include "SobelShader.hpp"
#include "TileShader.hpp"
#include "TissueShader.hpp"
#include "TransformShader.hpp"
#include "TriangleMapShader.hpp"
#include "VanGoghShader.hpp"
#include "VideoSourceService.hpp"
#include "WobbleShader.hpp"

void ShaderChainerService::setup()
{
#ifdef TESTING
  // Return early while testing. We don't test AvailableShaders for now.
  return;
#endif
  
  // Create an AvailableShader for each type
  for (auto const shaderType : AvailableBasicShaderTypes)
  {
    auto shader = std::make_shared<AvailableShader>(shaderType,
                                                    shaderTypeName(shaderType));
    availableBasicShaders.push_back(shader);
  }
  for (auto const shaderType : AvailableMixShaderTypes)
  {
    auto shader = std::make_shared<AvailableShader>(shaderType,
                                                    shaderTypeName(shaderType));
    availableMixShaders.push_back(shader);
  }
  for (auto const shaderType : AvailableFilterShaderTypes)
  {
    auto shader = std::make_shared<AvailableShader>(shaderType,
                                                    shaderTypeName(shaderType));
    availableFilterShaders.push_back(shader);
  }
  for (auto const shaderType : AvailableTransformShaderTypes)
  {
    auto shader = std::make_shared<AvailableShader>(shaderType,
                                                    shaderTypeName(shaderType));
    availableTransformShaders.push_back(shader);
  }
  for (auto const shaderType : AvailableMaskShaderTypes)
  {
    auto shader = std::make_shared<AvailableShader>(shaderType,
                                                    shaderTypeName(shaderType));
    availableMaskShaders.push_back(shader);
  }
}

void ShaderChainerService::subscribeToShaderChainerUpdates(
                                                           std::function<void()> callback)
{
  shaderChainerUpdateSubject.subscribe(callback);
}

void ShaderChainerService::processFrame()
{
  // Collect the `Connections` from connectionMap with a type of ConnectionTypeSource
  std::vector<std::shared_ptr<Connection>> sourceConnections;
  for (auto &connectionPair : connectionMap)
  {
    if (connectionPair.second->type == ConnectionTypeSource)
    {
      sourceConnections.push_back(connectionPair.second);
    }
  }
  
  for (auto connection : sourceConnections)
  {
    std::shared_ptr<VideoSource> videoSource = std::dynamic_pointer_cast<VideoSource>(connection->start);
    std::shared_ptr<Shader> shader = std::dynamic_pointer_cast<Shader>(connection->end);
    shader->traverseFrame(videoSource->frame());
  }
}

void ShaderChainerService::selectShader(std::shared_ptr<Shader> shader)
{
  selectedShader = shader;
}

std::shared_ptr<Shader> ShaderChainerService::makeShader(ShaderType type)
{
  auto shader = shaderForType(type, UUID::generateUUID(), 0);
  addShader(shader);
  return shader;
}

// Returns true if the passed Shader is the terminal node in a Chainer.
bool ShaderChainerService::isTerminalShader(std::shared_ptr<Shader> shader)
{
  return shader->outputs.empty();
}

std::shared_ptr<Shader> ShaderChainerService::terminalShader(std::shared_ptr<Shader> shader)
{
  if (isTerminalShader(shader))
  {
    return shader;
  }
  
  for (auto conn : shader->outputs)
  {
    if (conn->end->connectableType() == ConnectableTypeShader)
    {
      auto nextShader = std::dynamic_pointer_cast<Shader>(conn->end);
      return terminalShader(nextShader);
    }
  }
  return nullptr;
}

bool ShaderChainerService::hasParents(std::shared_ptr<Shader> shader)
{
  return !shader->inputs.empty();
}

// ConfigurableService

json ShaderChainerService::config()
{
  json j;
  
  for (auto const &[key, val] : shadersMap)
  {
    j[val->shaderId] = val->serialize();
  }
  
  return j;
}

json ShaderChainerService::connectionsConfig()
{
  json j;
  
  for (auto const &[key, val] : connectionMap)
  {
    j[val->id] = val->serialize();
  }
  
  return j;
}

void ShaderChainerService::clear()
{
  for (auto it = shadersMap.begin(); it != shadersMap.end();)
  {
    auto shader = it->second;
    removeShader(shader, false);
    it = shadersMap.erase(it);
  }
  shadersMap.clear();
  connectionMap.clear();
  // Clear the shaderChainerMap
  shaderIdShaderChainerMap.clear();
  videoSourceIdShaderChainerMap.clear();
}

void ShaderChainerService::loadConfig(json data)
{
  idsFromLoadingConfig(data);
}

std::vector<std::string> ShaderChainerService::idsFromLoadingConfig(json j) {
  // Load the shaders included in the data.
  /* The shaders come in as an object formatted as:
   {
   "shaderType": ShaderType,
   "shaderId": String,
   "x": Float
   "y": Float
   ...remaining ShaderSettings
   }
   */
  std::vector<std::string> ids;
  for (auto const &[key, val] : j.items())
  {
    ShaderType shaderType = val["shaderType"];
    std::string shaderId = key;
    
    auto shader = shaderForType(shaderType, shaderId, val);
    addShader(shader);
    // Set the position of the Shader
    
    if (val["x"].is_number() && val["y"].is_number())
    {
      float x = val["x"];
      float y = val["y"];
      shader->settings->x->value = x;
      shader->settings->y->value = y;
    }
    
    shader->settings->load(val);
    ids.push_back(shader->shaderId);
  }
  
  return ids;
}

void ShaderChainerService::loadConnectionsConfig(json j)
{
  for (auto const &[key, val] : j.items())
  {
    ConnectionType type = val["type"];
    std::string startId = val["start"];
    std::string endId = val["end"];
    
    std::shared_ptr<Connectable> start;
    std::shared_ptr<Connectable> end = shaderForId(endId);
    
    switch (type)
    {
      case ConnectionTypeSource:
        start = VideoSourceService::getService()->videoSourceForId(startId);
        break;
      case ConnectionTypeShader:
      case ConnectionTypeAux:
      case ConnectionTypeMask:
        start = shaderForId(startId);
        // We might be starting on a Source
        if (start == nullptr)
          start = VideoSourceService::getService()->videoSourceForId(startId);
    }
    
    if (start == nullptr || end == nullptr)
    {
      log("Couldn't load connection for %s -> %s", startId.c_str(), endId.c_str());
      continue;
    }
    
    makeConnection(start, end, type);
  }
}

void ShaderChainerService::addShader(std::shared_ptr<Shader> shader)
{
  if (shadersMap.count(shader->shaderId) != 0)
  {
    log("Reregistering Shader %s", shader->shaderId.c_str());
  }
  if (shader == nullptr)
  {
    log("Tried to add a null Shader");
    return;
  }
  
  std::shared_ptr<FeedbackSource> feedbackSource = std::make_shared<FeedbackSource>(shader->shaderId, std::make_shared<VideoSourceSettings>());
  
  FeedbackSourceService::getService()->registerFeedbackSource(feedbackSource);
  shader->feedbackDestination = feedbackSource;
  shadersMap[shader->shaderId] = shader;
  ConfigService::getService()->saveDefaultConfigFile();
}

void ShaderChainerService::removeShader(std::shared_ptr<Shader> shader, bool fromMap)
{
  if (shadersMap.count(shader->shaderId) == 0)
  {
    log("Tried to remove Shader %s, but it doesn't exist",
        shader->shaderId.c_str());
    return;
  }
  
  removeConnectable(shader);
  
  if (fromMap)
  {
    shadersMap.erase(shader->shaderId);
  }
  
  shader.reset();
  ConfigService::getService()->saveDefaultConfigFile();
}

void ShaderChainerService::removeConnectable(std::shared_ptr<Connectable> connectable)
{
  std::vector<std::string> connectionIdsToRemove;
  
  // Remove the input Connections
  if (!connectable->inputs.empty())
  {
    for (auto conn : connectable->inputs)
    {
      // Remove the Connection connecting [Parent] -> [Us]
      connectionIdsToRemove.push_back(conn->id);
    }
  }
  
  // Remove the output Connections
  if (!connectable->outputs.empty())
  {
    for (auto conn : connectable->outputs)
    {
      connectionIdsToRemove.push_back(conn->id);
    }
  }
  
  for (auto connId : connectionIdsToRemove)
  {
    breakConnectionForConnectionId(connId);
  }
  
  for (auto connId : connectionIdsToRemove)
  {
    connectionMap.erase(connId);
  }
  ConfigService::getService()->saveDefaultConfigFile();
}

std::shared_ptr<Shader> ShaderChainerService::shaderForId(std::string id)
{
  if (shadersMap.count(id) == 0)
  {
    log("Tried to get Shader %s, but it doesn't exist", id.c_str());
    return nullptr;
  }
  return shadersMap[id];
}

Strand ShaderChainerService::strandForConnectable(std::shared_ptr<Connectable> connectable) {
  Strand strand;
  std::set<std::shared_ptr<Connectable>> visitedConnectables;
  std::set<std::string> visitedConnectionIds;
  
  // Recursive function to explore and add connectables and connections to the strand
  std::function<void(std::shared_ptr<Connectable>)> explore = [&](std::shared_ptr<Connectable> current) {
    if (!current || visitedConnectables.find(current) != visitedConnectables.end()) {
      return; // Prevent processing the same connectable more than once
    }
    
    // Mark the current connectable as visited
    visitedConnectables.insert(current);
    strand.connectables.push_back(current);
    
    // Explore all inputs and outputs of the current connectable
    for (const auto& input : current->inputs) {
      if (visitedConnectionIds.find(input->id) == visitedConnectionIds.end()) {
        strand.connections.push_back(input);
        visitedConnectionIds.insert(input->id);
        explore(input->start);
      }
    }
    
    for (const auto& output : current->outputs) {
      if (visitedConnectionIds.find(output->id) == visitedConnectionIds.end()) {
        strand.connections.push_back(output);
        visitedConnectionIds.insert(output->id);
        explore(output->end);
      }
    }
  };
  
  // Start the exploration with the given connectable
  explore(connectable);
  
  strand.name = Strand::strandName(strand.connectables);
  
  return strand;
}

std::vector<std::shared_ptr<Shader>> ShaderChainerService::shaders()
{
  std::vector<std::shared_ptr<Shader>> shaders;
  for (auto const &[key, val] : shadersMap)
  {
    shaders.push_back(val);
  }
  // Sort the shaders by creationTime
  std::sort(shaders.begin(), shaders.end(),
            [](std::shared_ptr<Shader> a, std::shared_ptr<Shader> b)
            {
    return a->creationTime < b->creationTime;
  });
  return shaders;
}

std::vector<std::shared_ptr<Connection>> ShaderChainerService::connections()
{
  std::vector<std::shared_ptr<Connection>> connections;
  for (auto const &[key, val] : connectionMap)
  {
    connections.push_back(val);
  }
  return connections;
}

void ShaderChainerService::breakConnectionForConnectionId(std::string connectionId)
{
  std::shared_ptr<Connection> connection = connectionMap[connectionId];
  if (connection == nullptr)
  {
    log("Removing null Connection");
    return;
  }
  connection->start->removeConnection(connection);
  connectionMap.erase(connectionId);
  ConfigService::getService()->saveDefaultConfigFile();
}

std::shared_ptr<Connection> ShaderChainerService::makeConnection(std::shared_ptr<Connectable> start,
                                                                 std::shared_ptr<Connectable> end,
                                                                 ConnectionType type,
                                                                 bool shouldSaveConfig)
{
  // Only allow a single input for each type
  if (end->hasInputForType(type))
  {
    return nullptr;
  }
  
  auto connection = std::make_shared<Connection>(start, end, type);
  start->outputs.insert(connection);
  end->inputs.insert(connection);
  connectionMap[connection->id] = connection;
  
  // Update the Connectable's FeedbackSource
  
  if (end->connectableType() == ConnectableTypeShader)
  {
    std::shared_ptr<Shader> shader = std::dynamic_pointer_cast<Shader>(end);
    shader->feedbackDestination->updateSettings(start->sourceSettings());
  }
  
  // Update the OutputWindow, if necessary
  if (VideoSourceService::getService()->hasOutputWindowForConnectable(start))
  {
    VideoSourceService::getService()->updateOutputWindow(start, end);
  }
  
  if (shouldSaveConfig)
  {
    ConfigService::getService()->saveDefaultConfigFile();
  }
  
  return connection;
}

std::shared_ptr<Shader>
ShaderChainerService::shaderForType(ShaderType shaderType, std::string shaderId,
                                    json shaderJson)
{
  switch (shaderType)
  {
    // hygenSwitch
    case ShaderTypeCubify: {
      auto settings = new CubifySettings(shaderId, shaderJson);
      auto shader = std::make_shared<CubifyShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeSwirlingSoul: {
      auto settings = new SwirlingSoulSettings(shaderId, shaderJson);
      auto shader = std::make_shared<SwirlingSoulShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeDoubleSwirl: {
      auto settings = new DoubleSwirlSettings(shaderId, shaderJson);
      auto shader = std::make_shared<DoubleSwirlShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeSmokeRing: {
      auto settings = new SmokeRingSettings(shaderId, shaderJson);
      auto shader = std::make_shared<SmokeRingShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeAudioCircle: {
      auto settings = new AudioCircleSettings(shaderId, shaderJson);
      auto shader = std::make_shared<AudioCircleShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeLimbo:
    {
      auto settings = new LimboSettings(shaderId, shaderJson);
      auto shader = std::make_shared<LimboShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeTextureMask:
    {
      auto settings = new TextureMaskSettings(shaderId, shaderJson);
      auto shader = std::make_shared<TextureMaskShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeSnowfall:
    {
      auto settings = new SnowfallSettings(shaderId, shaderJson);
      auto shader = std::make_shared<SnowfallShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeCanny:
    {
      auto settings = new CannySettings(shaderId, shaderJson);
      auto shader = std::make_shared<CannyShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeVHS:
    {
      auto settings = new VHSSettings(shaderId, shaderJson);
      auto shader = std::make_shared<VHSShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeCirclePath:
    {
      auto settings = new CirclePathSettings(shaderId, shaderJson);
      auto shader = std::make_shared<CirclePathShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeBounce:
    {
      auto settings = new BounceSettings(shaderId, shaderJson);
      auto shader = std::make_shared<BounceShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeVertex:
    {
      auto settings = new VertexSettings(shaderId, shaderJson);
      auto shader = std::make_shared<VertexShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeTriple:
    {
      auto settings = new TripleSettings(shaderId, shaderJson);
      auto shader = std::make_shared<TripleShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypePaint:
    {
      auto settings = new PaintSettings(shaderId, shaderJson);
      auto shader = std::make_shared<PaintShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeLumaFeedback:
    {
      auto settings = new LumaFeedbackSettings(shaderId, shaderJson);
      auto shader = std::make_shared<LumaFeedbackShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeGameboy:
    {
      auto settings = new GameboySettings(shaderId, shaderJson);
      auto shader = std::make_shared<GameboyShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeStaticFrame:
    {
      auto settings = new StaticFrameSettings(shaderId, shaderJson);
      auto shader = std::make_shared<StaticFrameShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeSlidingFrame:
    {
      auto settings = new SlidingFrameSettings(shaderId, shaderJson);
      auto shader = std::make_shared<SlidingFrameShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeMinMixer:
    {
      auto settings = new MinMixerSettings(shaderId, shaderJson);
      auto shader = std::make_shared<MinMixerShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeSolidColor:
    {
      auto settings = new SolidColorSettings(shaderId, shaderJson);
      auto shader = std::make_shared<SolidColorShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderType16bit:
    {
      auto settings = new SixteenBitSettings(shaderId, shaderJson);
      auto shader = std::make_shared<SixteenBitShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeHilbert:
    {
      auto settings = new HilbertSettings(shaderId, shaderJson);
      auto shader = std::make_shared<HilbertShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeWarp:
    {
      auto settings = new WarpSettings(shaderId, shaderJson);
      auto shader = std::make_shared<WarpShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeFrequencyVisualizer:
    {
      auto settings = new FrequencyVisualizerSettings(shaderId, shaderJson);
      auto shader = std::make_shared<FrequencyVisualizerShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeColorKeyMaskMaker:
    {
      auto settings = new ColorKeyMaskMakerSettings(shaderId, shaderJson);
      auto shader = std::make_shared<ColorKeyMaskMakerShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeCurlySquares:
    {
      auto settings = new CurlySquaresSettings(shaderId, shaderJson);
      auto shader = std::make_shared<CurlySquaresShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypePlasmaTwo:
    {
      auto settings = new PlasmaTwoSettings(shaderId, shaderJson);
      auto shader = std::make_shared<PlasmaTwoShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeDancingSquares:
    {
      auto settings = new DancingSquaresSettings(shaderId, shaderJson);
      auto shader = std::make_shared<DancingSquaresShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeLumaMaskMaker:
    {
      auto settings = new LumaMaskMakerSettings(shaderId, shaderJson);
      auto shader = std::make_shared<LumaMaskMakerShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeMask:
    {
      auto settings = new MaskSettings(shaderId, shaderJson);
      auto shader = std::make_shared<MaskShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeCircle:
    {
      auto settings = new CircleSettings(shaderId, shaderJson);
      auto shader = std::make_shared<CircleShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeCrosshatch:
    {
      auto settings = new CrosshatchSettings(shaderId, shaderJson);
      auto shader = std::make_shared<CrosshatchShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeHalfTone:
    {
      auto settings = new HalfToneSettings(shaderId, shaderJson);
      auto shader = std::make_shared<HalfToneShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeTissue:
    {
      auto settings = new TissueSettings(shaderId, shaderJson);
      auto shader = std::make_shared<TissueShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypePsycurves:
    {
      auto settings = new PsycurvesSettings(shaderId, shaderJson);
      auto shader = std::make_shared<PsycurvesShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeFishEye:
    {
      auto settings = new FishEyeSettings(shaderId, shaderJson);
      auto shader = std::make_shared<FishEyeShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeColorPass:
    {
      auto settings = new ColorPassSettings(shaderId, shaderJson);
      auto shader = std::make_shared<ColorPassShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeSobel:
    {
      auto settings = new SobelSettings(shaderId, shaderJson);
      auto shader = std::make_shared<SobelShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeTriangleMap:
    {
      auto settings = new TriangleMapSettings(shaderId, shaderJson);
      auto shader = std::make_shared<TriangleMapShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeLiquid:
    {
      auto settings = new LiquidSettings(shaderId, shaderJson);
      auto shader = std::make_shared<LiquidShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeDisco:
    {
      auto settings = new DiscoSettings(shaderId, shaderJson);
      auto shader = std::make_shared<DiscoShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeOctahedron:
    {
      auto settings = new OctahedronSettings(shaderId, shaderJson);
      auto shader = std::make_shared<OctahedronShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeVanGogh:
    {
      auto settings = new VanGoghSettings(shaderId, shaderJson);
      auto shader = std::make_shared<VanGoghShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeRubiks:
    {
      auto settings = new RubiksSettings(shaderId, shaderJson);
      auto shader = std::make_shared<RubiksShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeRainbowRotator:
    {
      auto settings = new RainbowRotatorSettings(shaderId, shaderJson);
      auto shader = std::make_shared<RainbowRotatorShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeDither:
    {
      auto settings = new DitherSettings(shaderId, shaderJson);
      auto shader = std::make_shared<DitherShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeMountains:
    {
      auto settings = new MountainsSettings(shaderId, shaderJson);
      auto shader = std::make_shared<MountainsShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeGalaxy:
    {
      auto settings = new GalaxySettings(shaderId, shaderJson);
      auto shader = std::make_shared<GalaxyShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeAudioMountains:
    {
      auto settings = new AudioMountainsSettings(shaderId, shaderJson);
      auto shader = std::make_shared<AudioMountainsShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeAudioBumper:
    {
      auto settings = new AudioBumperSettings(shaderId, shaderJson);
      auto shader = std::make_shared<AudioBumperShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeAudioWaveform:
    {
      auto settings = new AudioWaveformSettings(shaderId, shaderJson);
      auto shader = std::make_shared<AudioWaveformShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeRings:
    {
      auto settings = new RingsSettings(shaderId, shaderJson);
      auto shader = std::make_shared<RingsShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeSlider:
    {
      auto settings = new SliderSettings(shaderId, shaderJson);
      auto shader = std::make_shared<SliderShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeWobble:
    {
      auto settings = new WobbleSettings(shaderId, shaderJson);
      auto shader = std::make_shared<WobbleShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeRGBShift:
    {
      auto settings = new RGBShiftSettings(shaderId, shaderJson);
      auto shader = std::make_shared<RGBShiftShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeNone:
      return 0;
    case ShaderTypeHSB:
    {
      auto settings = new HSBSettings(shaderId, shaderJson);
      auto shader = std::make_shared<HSBShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeBlur:
    {
      auto settings = new BlurSettings(shaderId, shaderJson);
      auto shader = std::make_shared<BlurShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypePixelate:
    {
      auto settings = new PixelSettings(shaderId, shaderJson);
      auto shader = std::make_shared<PixelShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeGlitch:
    {
      auto settings = new GlitchSettings(shaderId, shaderJson);
      auto shader = std::make_shared<GlitchShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeMirror:
    {
      auto settings = new MirrorSettings(shaderId, shaderJson);
      auto shader = std::make_shared<MirrorShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeTransform:
    {
      auto settings = new TransformSettings(shaderId, shaderJson);
      auto shader = std::make_shared<TransformShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeFeedback:
    {
      auto settings = new FeedbackSettings(shaderId, shaderJson);
      auto shader = std::make_shared<FeedbackShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeAscii:
    {
      auto settings = new AsciiSettings(shaderId, shaderJson);
      auto shader = std::make_shared<AsciiShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeKaleidoscope:
    {
      auto settings = new KaleidoscopeSettings(shaderId, shaderJson);
      auto shader = std::make_shared<KaleidoscopeShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeTile:
    {
      auto settings = new TileSettings(shaderId, shaderJson);
      auto shader = std::make_shared<TileShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeMix:
    {
      auto settings = new MixSettings(shaderId, shaderJson);
      auto shader = std::make_shared<MixShader>(settings);
      shader.get()->setup();
      return shader;
    }
    case ShaderTypePlasma:
    {
      auto settings = new PlasmaSettings(shaderId, shaderJson);
      auto shader = std::make_shared<PlasmaShader>(settings);
      shader.get()->setup();
      return shader;
    }
    case ShaderTypeFuji:
    {
      auto settings = new FujiSettings(shaderId, shaderJson);
      auto shader = std::make_shared<FujiShader>(settings);
      shader.get()->setup();
      return shader;
      break;
    }
    case ShaderTypeFractal:
    {
      auto settings = new FractalSettings(shaderId, shaderJson);
      auto shader = std::make_shared<FractalShader>(settings);
      shader.get()->setup();
      return shader;
      break;
    }
    case ShaderTypeClouds:
    {
      auto settings = new CloudSettings(shaderId, shaderJson);
      auto shader = std::make_shared<CloudShader>(settings);
      shader.get()->setup();
      return shader;
      break;
    }
    case ShaderTypeMelter:
    {
      auto settings = new MeltSettings(shaderId, shaderJson);
      auto shader = std::make_shared<MeltShader>(settings);
      shader.get()->setup();
      return shader;
      break;
    }
  }
}

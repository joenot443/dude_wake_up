//
//  ShaderChainerService.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 11/4/22.
//

#include "ShaderChainerService.hpp"
#include "AsciiShader.hpp"
#include "SimpleBarsShader.hpp"
#include "ComicbookShader.hpp"
#include "BackgroundShader.hpp"
#include "SimpleShapeShader.hpp"
#include "ColoredDropsShader.hpp"
#include "PlasmorShader.hpp"
#include "AutotangentShader.hpp"
#include "DiffractorShader.hpp"
#include "ScratchyShader.hpp"
#include "AerogelShader.hpp"
#include "BreatheShader.hpp"
#include "HeptagonsShader.hpp"
#include "OctagramsShader.hpp"
#include "PerplexionShader.hpp"
#include "ColorWheelShader.hpp"
#include "DiscoAudioShader.hpp"
#include "FlickerAudioShader.hpp"
#include "FractalAudioShader.hpp"
#include "StarryPlanesShader.hpp"
#include "UnknownPleasuresShader.hpp"
#include "SpaceRingsShader.hpp"
#include "CloudyShapesShader.hpp"
#include "AudioBlocksShader.hpp"
#include "FloatingSparksShader.hpp"
#include "CosmosShader.hpp"
#include "OutlineShader.hpp"
#include "DoubleBlurShader.hpp"
#include "GlitchAudioShader.hpp"
#include "ChromeGrillShader.hpp"
#include "FibersShader.hpp"
#include "MistShader.hpp"
#include "WelcomeRingsShader.hpp"
#include "GlitchRGBShader.hpp"
#include "GlitchDigitalShader.hpp"
#include "MotionBlurTextureShader.hpp"
#include "OnOffShader.hpp"
#include "IsoFractShader.hpp"
#include "SwirlShader.hpp"
#include "TraceAudioShader.hpp"
#include "PixelPlayShader.hpp"
#include "DirtyPlasmaShader.hpp"
#include "OneBitDitherShader.hpp"
#include "OldTVShader.hpp"
#include "TwistedTripShader.hpp"
#include "TwistedCubesShader.hpp"
#include "CoreShader.hpp"
#include "VoronoiColumnsShader.hpp"
#include "GodRayShader.hpp"
#include "WarpspeedShader.hpp"
#include "ReflectorShader.hpp"
#include "BlendShader.hpp"
#include "FullHouseShader.hpp"
#include "PieSplitShader.hpp"
#include "OverlayShader.hpp"
#include "SwitcherShader.hpp"
#include "RotateShader.hpp"
#include "MultiMixShader.hpp"
#include "ColorStepperShader.hpp"
#include "GridRunShader.hpp"
#include "ColorSwapShader.hpp"
#include "AlphaMixShader.hpp"
#include "GyroidsShader.hpp"
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
#include "WebcamSource.hpp"
#include "FileSource.hpp"
#include "ImageSource.hpp"
#include "TextSource.hpp"
#include "IconSource.hpp"
#include "LibrarySource.hpp"
#include "LayoutStateService.hpp"

void ShaderChainerService::setup()
{
  
  // Create an AvailableShader for each type
  for (auto const shaderType : AvailableBasicShaderTypes)
  {
    auto shader = std::make_shared<AvailableShader>(shaderType,
                                                    shaderTypeName(shaderType));
    availableBasicShaders.push_back(shader);
    availableShadersMap[shaderType] = shader;
    allAvailableShaders.push_back(shader);
  }
  for (auto const shaderType : AvailableMixShaderTypes)
  {
    auto shader = std::make_shared<AvailableShader>(shaderType,
                                                    shaderTypeName(shaderType));
    availableMixShaders.push_back(shader);
    availableShadersMap[shaderType] = shader;
    allAvailableShaders.push_back(shader);
  }
  for (auto const shaderType : AvailableFilterShaderTypes)
  {
    auto shader = std::make_shared<AvailableShader>(shaderType,
                                                    shaderTypeName(shaderType));
    availableFilterShaders.push_back(shader);
    availableShadersMap[shaderType] = shader;
    allAvailableShaders.push_back(shader);
  }
  for (auto const shaderType : AvailableTransformShaderTypes)
  {
    auto shader = std::make_shared<AvailableShader>(shaderType,
                                                    shaderTypeName(shaderType));
    availableTransformShaders.push_back(shader);
    availableShadersMap[shaderType] = shader;
    allAvailableShaders.push_back(shader);
  }
  for (auto const shaderType : AvailableMaskShaderTypes)
  {
    auto shader = std::make_shared<AvailableShader>(shaderType,
                                                    shaderTypeName(shaderType));
    availableMaskShaders.push_back(shader);
    availableShadersMap[shaderType] = shader;
    allAvailableShaders.push_back(shader);
  }
  
  for (auto const shaderType : AvailableGlitchShaderTypes)
  {
    auto shader = std::make_shared<AvailableShader>(shaderType,
                                                    shaderTypeName(shaderType));
    availableGlitchShaders.push_back(shader);
    availableShadersMap[shaderType] = shader;
    allAvailableShaders.push_back(shader);
  }
  
  for (auto const shaderType : { ShaderTypeBlend, ShaderTypeTransform, ShaderTypeRotate, ShaderTypeMirror, ShaderTypeHSB, ShaderTypeVHS, ShaderType16bit, ShaderTypePixelate, ShaderTypeAutotangent }) {
    auto availableShader = availableShadersMap[shaderType];
    availableDefaultFavoriteShaders.push_back(availableShader);
  }
}

std::vector<std::shared_ptr<AvailableShader>> ShaderChainerService::availableFavoriteShaders() {
  std::vector<std::shared_ptr<AvailableShader>> favorites = {};
  std::set<ShaderType> favoriteTypes = ParameterService::getService()->favoriteShaderTypes;
  
  for (auto type : favoriteTypes) {
    favorites.push_back(availableShadersMap[type]);
  }
  
  return favorites;
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
  std::vector<std::string> toErase = {};
  for (auto &connectionPair : connectionMap)
  {
    if (connectionPair.second == nullptr) {
      toErase.push_back(connectionPair.first);
      continue;
    }
    if (connectionPair.second->type == ConnectionTypeSource)
    {
      sourceConnections.push_back(connectionPair.second);
    }
  }
  
  for (auto id : toErase) {
    connectionMap.erase(id);
  }
  
  std::pair<int, std::shared_ptr<Shader>> maxPair = std::pair<int, std::shared_ptr<Shader>>(-1, nullptr);
  
  for (auto connection : sourceConnections)
  {
    // No need to traverse non-Main connections (Aux, Feedback, etc.)
    if (connection->inputSlot != InputSlotMain) continue;
    
    std::shared_ptr<VideoSource> videoSource = std::dynamic_pointer_cast<VideoSource>(connection->start);
    std::shared_ptr<Shader> shader = std::dynamic_pointer_cast<Shader>(connection->end);
    
    // No need to traverse if the video's not active
    if (videoSource && !videoSource->active) continue;
    
    shader->traverseFrame(videoSource->frame(), 0);
  }
}

void ShaderChainerService::selectConnectable(std::shared_ptr<Connectable> connectable)
{
  selectedConnectable = connectable;
}

void ShaderChainerService::deselectConnectable()
{
  selectedConnectable = nullptr;
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
  // True if we have no outputs or if the only output is an Aux
  
  return shader->outputs.empty() || (shader->outputs.size() == 1 && shader->outputs.count(OutputSlotAux) == 1);
}

bool ShaderChainerService::isShaderType(std::shared_ptr<Connectable> connectable, ShaderType shaderType) {
  if (connectable->connectableType() != ConnectableTypeShader) return false;
  
  std::shared_ptr<Shader> shader = std::dynamic_pointer_cast<Shader>(connectable);
  return shader->type() == shaderType;
}

std::shared_ptr<Shader> ShaderChainerService::terminalShader(std::shared_ptr<Shader> shader)
{
  if (!shader || shader->outputs.empty()) {
    return shader;
  }

  std::shared_ptr<Shader> terminal = shader;

  for (auto const& [slot, connections] : shader->outputs) {
    for (auto const& connection : connections) {
      auto candidate = std::dynamic_pointer_cast<Shader>(connection->end);
      if (candidate) {
        auto terminalCandidate = terminalShader(candidate);
        if (terminalCandidate) {
          terminal = terminalCandidate;
        }
      }
    }
  }

  return terminal;
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
    json res = val->serialize();
    if (res.is_object()) {
      j[val->shaderId] = res;
    }
  }
  
  return j;
}

json ShaderChainerService::connectionsConfig()
{
  json j;
  
  for (auto const &[key, val] : connectionMap)
  {
    if (val != nullptr) j[val->id] = val->serialize();
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
  selectedConnectable = nullptr;
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
    
    if (shader == nullptr) continue;
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
    InputSlot inputSlot = val["inputSlot"];
    OutputSlot outputSlot = val["outputSlot"];
    
    std::shared_ptr<Connectable> start;
    std::shared_ptr<Connectable> end = shaderForId(endId);
    
    switch (type)
    {
      case ConnectionTypeSource:
        start = VideoSourceService::getService()->videoSourceForId(startId);
        break;
      case ConnectionTypeShader:
        start = shaderForId(startId);
    }
    
    if (start == nullptr || end == nullptr)
    {
      log("Couldn't load connection for %s -> %s", startId.c_str(), endId.c_str());
      continue;
    }
    
    makeConnection(start, end, type, outputSlot, inputSlot);
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
  
  std::shared_ptr<FeedbackSource> feedbackSource = std::make_shared<FeedbackSource>(shader->shaderId, std::make_shared<VideoSourceSettings>(UUID::generateUUID(), 0));
  
  FeedbackSourceService::getService()->registerFeedbackSource(feedbackSource);
  shader->feedbackDestination = feedbackSource;
  shader->allocateFrames();
  shader->generateOptionalShaders();
  shadersMap[shader->shaderId] = shader;
  
  // Subscribe the Shader's setup() to resolution updates
  LayoutStateService::getService()->subscribeToResolutionUpdates([shader]() {
    shader->allocateFrames();
    shader->setup();
  });
  
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
  
  OscillationService::getService()->removeOscillatorsFor(shader->settings->parameters);
  
  removeConnectable(shader);
  
  for (auto param : shader->settings->parameters) {
    if (param->favorited) {
      ParameterService::getService()->removeFavoriteParameter(param);
    }
  }
  
  if (fromMap)
  {
    shadersMap.erase(shader->shaderId);
  }
  
  if (selectedConnectable == shader) {
    selectedConnectable = nullptr;
  }
  
  ParameterService::getService()->removeStageShaderId(shader->shaderId);
  
  shader.reset();
  ConfigService::getService()->saveDefaultConfigFile();
}

std::vector<std::shared_ptr<Connectable>> ShaderChainerService::pasteConnectables(const std::vector<std::shared_ptr<Connectable>>& connectables) {
  std::map<std::shared_ptr<Connectable>, std::shared_ptr<Connectable>> originalToNewMap;
  std::vector<std::shared_ptr<Connectable>> newConnectables;
  
  // Step 1: Create new instances of each connectable
  for (const auto& original : connectables) {
    std::shared_ptr<Connectable> newConnectable;

    if (original->connectableType() == ConnectableTypeShader) {
      auto originalShader = std::dynamic_pointer_cast<Shader>(original);
      auto newShader = makeShader(originalShader->type());
      newShader->settings->x->setValue(originalShader->settings->x->value);
      newShader->settings->y->setValue(originalShader->settings->y->value);
      newShader->settings->copyFrom(*originalShader->settings);
      addShader(newShader);
      newConnectable = newShader;
    } else if (original->connectableType() == ConnectableTypeSource) {
      auto originalSource = std::dynamic_pointer_cast<VideoSource>(original);
      std::shared_ptr<VideoSource> newSource;

      switch (originalSource->type) {
        case VideoSource_webcam: {
          auto webcamSource = std::dynamic_pointer_cast<WebcamSource>(originalSource);
          newSource = VideoSourceService::getService()->makeWebcamVideoSource(webcamSource->name(), webcamSource->settings->deviceId->intValue);
          break;
        }
        case VideoSource_file: {
          auto fileSource = std::dynamic_pointer_cast<FileSource>(originalSource);
          newSource = VideoSourceService::getService()->makeFileVideoSource(fileSource->name(), fileSource->path);
          break;
        }
        case VideoSource_image: {
          auto imageSource = std::dynamic_pointer_cast<ImageSource>(originalSource);
          newSource = VideoSourceService::getService()->makeImageVideoSource(imageSource->name(), imageSource->path);
          break;
        }
        case VideoSource_text: {
          auto textSource = std::dynamic_pointer_cast<TextSource>(originalSource);
          newSource = VideoSourceService::getService()->makeTextVideoSource(textSource->name());
          break;
        }
        case VideoSource_icon: {
          auto iconSource = std::dynamic_pointer_cast<IconSource>(originalSource);
          newSource = VideoSourceService::getService()->makeIconVideoSource("Icon");
          break;
        }
        case VideoSource_library: {
          auto librarySource = std::dynamic_pointer_cast<LibrarySource>(originalSource);
          newSource = VideoSourceService::getService()->makeLibraryVideoSource(librarySource->libraryFile);
          break;
        }
        // Add other VideoSourceTypes as needed
        default:
          break;
      }

      if (newSource) {
        newSource->settings->copyFrom(*originalSource->settings);
        VideoSourceService::getService()->addVideoSource(newSource, UUID::generateUUID());
        newConnectable = newSource;
      }
    }

    if (newConnectable) {
      originalToNewMap[original] = newConnectable;
      newConnectables.push_back(newConnectable);
    }
  }

  // Step 2: Create new connections
  for (const auto& original : connectables) {
    auto newConnectable = originalToNewMap[original];

    // Copy input connections
    for (const auto& [slot, conn] : original->inputs) {
      auto newStart = originalToNewMap.count(conn->start) ? originalToNewMap[conn->start] : conn->start;
      makeConnection(newStart, newConnectable, conn->type, conn->outputSlot, conn->inputSlot, true);
    }

    // Copy output connections
    for (const auto& [slot, connections] : original->outputs) {
      for (const auto& conn : connections) {
        auto newEnd = originalToNewMap.count(conn->end) ? originalToNewMap[conn->end] : conn->end;
        makeConnection(newConnectable, newEnd, conn->type, conn->outputSlot, conn->inputSlot, true);
      }
    }
  }
  
  return newConnectables;
}

void ShaderChainerService::removeConnectable(std::shared_ptr<Connectable> connectable)
{
  std::vector<std::string> connectionIdsToRemove;

  // Remove the input Connections
  for (auto [slot, conn] : connectable->inputs)
  {
    connectionIdsToRemove.push_back(conn->id);
  }

  // Remove the output Connections
  for (auto [slot, connections] : connectable->outputs)
  {
    for (auto &conn : connections)
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

void ShaderChainerService::insert(std::shared_ptr<Connectable> start, std::shared_ptr<Connectable> connectable, OutputSlot slot) {
  if (!start->hasOutputAtSlot(slot))
  {
      throw std::invalid_argument("No existing connection at the specified slot to insert into.");
  }
 
  auto originalConnections = start->outputs.at(slot); // Corrected to handle multiple connections
  start->outputs.erase(slot);
  for (auto &originalConnection : originalConnections) {
    connectionMap.erase(originalConnection->id);
  }
 
  for (auto &originalConnection : originalConnections) {
    auto newConnection = makeConnection(start, connectable, originalConnection->type, slot, InputSlotMain);
    connectable->inputs[InputSlotMain] = newConnection;
 
    auto continuationConnection = std::make_shared<Connection>(connectable, originalConnection->end, originalConnection->type, OutputSlotMain, originalConnection->inputSlot);
    connectable->outputs[OutputSlotMain].push_back(continuationConnection); // Add to vector
    originalConnection->end->inputs[originalConnection->inputSlot] = continuationConnection;
  }
}

void ShaderChainerService::copyConnections(std::shared_ptr<Connectable> source, std::shared_ptr<Connectable> dest) {
  // Copy input connections
  for (auto [slot, conn] : source->inputs) {
    makeConnection(conn->start, dest, conn->type, conn->outputSlot, conn->inputSlot, true);
  }
  
  // Copy output connections
  for (auto [slot, connections] : source->outputs) {
    for (auto &conn : connections) { // Iterate over all connections
      makeConnection(dest, conn->end, conn->type, conn->outputSlot, conn->inputSlot, true);
    }
  }
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
    for (const auto &[key, conn] : current->inputs) {
      if (visitedConnectionIds.find(conn->id) == visitedConnectionIds.end()) {
        strand.connections.push_back(conn);
        visitedConnectionIds.insert(conn->id);
        explore(conn->start);
      }
    }
    
    for (const auto &[key, connections] : current->outputs) {
      for (const auto &conn : connections) {
        if (visitedConnectionIds.find(conn->id) == visitedConnectionIds.end()) {
          strand.connections.push_back(conn);
          visitedConnectionIds.insert(conn->id);
          explore(conn->end);
        }
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
  auto it = connectionMap.find(connectionId);
  if (it == connectionMap.end())
  {
    log("Connection %s not found", connectionId.c_str());
    return;
  }

  std::shared_ptr<Connection> connection = it->second;
  if (connection == nullptr)
  {
    log("Removing null Connection");
    connectionMap.erase(it);
    return;
  }
  connection->start->removeConnection(connection);
  connection->end->removeConnection(connection);
  connectionMap.erase(it);
  ConfigService::getService()->saveDefaultConfigFile();
}

std::shared_ptr<Connection> ShaderChainerService::makeConnection(std::shared_ptr<Connectable> start,
                                                                 std::shared_ptr<Connectable> end,
                                                                 ConnectionType type,
                                                                 OutputSlot outputSlot,
                                                                 InputSlot inputSlot,
                                                                 bool shouldSaveConfig)
{
  // Only allow a single input for each type
  if (end->hasInputAtSlot(inputSlot))
  {
    log("Breaking Connection");
    breakConnectionForConnectionId(end->connectionAt(inputSlot)->id);
  }
  
  auto connection = std::make_shared<Connection>(start, end, type, outputSlot, inputSlot);
  start->outputs[outputSlot].push_back(connection); // Add to vector
  end->inputs[inputSlot] = connection;
  connectionMap[connection->id] = connection;
  
  // Update the Connectable's FeedbackSource
  
  if (end->connectableType() == ConnectableTypeShader)
  {
    std::shared_ptr<Shader> shader = std::dynamic_pointer_cast<Shader>(end);
    shader->feedbackDestination->updateSettings(start->sourceSettings());
  }
  
  // Update the OutputWindow, if necessary
  if (VideoSourceService::getService()->hasOutputWindowForConnectable(start) && LayoutStateService::getService()->outputWindowUpdatesAutomatically)
  {
    VideoSourceService::getService()->updateOutputWindow(start, end);
  }
  
  if (ParameterService::getService()->isShaderIdStage(start->connId())) {
    ParameterService::getService()->toggleStageShaderId(start->connId());
    ParameterService::getService()->toggleStageShaderId(end->connId());
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
    case ShaderTypeSimpleBars: {
      auto settings = new SimpleBarsSettings(shaderId, shaderJson);
      auto shader = std::make_shared<SimpleBarsShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeComicbook: {
      auto settings = new ComicbookSettings(shaderId, shaderJson);
      auto shader = std::make_shared<ComicbookShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeBackground: {
      auto settings = new BackgroundSettings(shaderId, shaderJson);
      auto shader = std::make_shared<BackgroundShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeSimpleShape: {
      auto settings = new SimpleShapeSettings(shaderId, shaderJson);
      auto shader = std::make_shared<SimpleShapeShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeColoredDrops: {
      auto settings = new ColoredDropsSettings(shaderId, shaderJson);
      auto shader = std::make_shared<ColoredDropsShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypePlasmor: {
      auto settings = new PlasmorSettings(shaderId, shaderJson);
      auto shader = std::make_shared<PlasmorShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeAutotangent: {
      auto settings = new AutotangentSettings(shaderId, shaderJson);
      auto shader = std::make_shared<AutotangentShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeDiffractor: {
      auto settings = new DiffractorSettings(shaderId, shaderJson);
      auto shader = std::make_shared<DiffractorShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeScratchy: {
      auto settings = new ScratchySettings(shaderId, shaderJson);
      auto shader = std::make_shared<ScratchyShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeAerogel: {
      auto settings = new AerogelSettings(shaderId, shaderJson);
      auto shader = std::make_shared<AerogelShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeBreathe: {
      auto settings = new BreatheSettings(shaderId, shaderJson);
      auto shader = std::make_shared<BreatheShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeHeptagons: {
      auto settings = new HeptagonsSettings(shaderId, shaderJson);
      auto shader = std::make_shared<HeptagonsShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeOctagrams: {
      auto settings = new OctagramsSettings(shaderId, shaderJson);
      auto shader = std::make_shared<OctagramsShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypePerplexion: {
      auto settings = new PerplexionSettings(shaderId, shaderJson);
      auto shader = std::make_shared<PerplexionShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeColorWheel: {
      auto settings = new ColorWheelSettings(shaderId, shaderJson);
      auto shader = std::make_shared<ColorWheelShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeDiscoAudio: {
      auto settings = new DiscoAudioSettings(shaderId, shaderJson);
      auto shader = std::make_shared<DiscoAudioShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeFlickerAudio: {
      auto settings = new FlickerAudioSettings(shaderId, shaderJson);
      auto shader = std::make_shared<FlickerAudioShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeFractalAudio: {
      auto settings = new FractalAudioSettings(shaderId, shaderJson);
      auto shader = std::make_shared<FractalAudioShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeStarryPlanes: {
      auto settings = new StarryPlanesSettings(shaderId, shaderJson);
      auto shader = std::make_shared<StarryPlanesShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeUnknownPleasures: {
      auto settings = new UnknownPleasuresSettings(shaderId, shaderJson);
      auto shader = std::make_shared<UnknownPleasuresShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeSpaceRings: {
      auto settings = new SpaceRingsSettings(shaderId, shaderJson);
      auto shader = std::make_shared<SpaceRingsShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeCloudyShapes: {
      auto settings = new CloudyShapesSettings(shaderId, shaderJson);
      auto shader = std::make_shared<CloudyShapesShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeAudioBlocks: {
      auto settings = new AudioBlocksSettings(shaderId, shaderJson);
      auto shader = std::make_shared<AudioBlocksShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeFloatingSparks: {
      auto settings = new FloatingSparksSettings(shaderId, shaderJson);
      auto shader = std::make_shared<FloatingSparksShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeCosmos: {
      auto settings = new CosmosSettings(shaderId, shaderJson);
      auto shader = std::make_shared<CosmosShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeOutline: {
      auto settings = new OutlineSettings(shaderId, shaderJson);
      auto shader = std::make_shared<OutlineShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeDoubleBlur: {
      auto settings = new DoubleBlurSettings(shaderId, shaderJson);
      auto shader = std::make_shared<DoubleBlurShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeGlitchAudio: {
      auto settings = new GlitchAudioSettings(shaderId, shaderJson);
      auto shader = std::make_shared<GlitchAudioShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeChromeGrill: {
      auto settings = new ChromeGrillSettings(shaderId, shaderJson);
      auto shader = std::make_shared<ChromeGrillShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeFibers: {
      auto settings = new FibersSettings(shaderId, shaderJson);
      auto shader = std::make_shared<FibersShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeMist: {
      auto settings = new MistSettings(shaderId, shaderJson);
      auto shader = std::make_shared<MistShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeWelcomeRings: {
      auto settings = new WelcomeRingsSettings(shaderId, shaderJson);
      auto shader = std::make_shared<WelcomeRingsShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeGlitchRGB: {
      auto settings = new GlitchRGBSettings(shaderId, shaderJson);
      auto shader = std::make_shared<GlitchRGBShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeGlitchDigital: {
      auto settings = new GlitchDigitalSettings(shaderId, shaderJson);
      auto shader = std::make_shared<GlitchDigitalShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeMotionBlurTexture: {
      auto settings = new MotionBlurTextureSettings(shaderId, shaderJson);
      auto shader = std::make_shared<MotionBlurTextureShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeOnOff: {
      auto settings = new OnOffSettings(shaderId, shaderJson);
      auto shader = std::make_shared<OnOffShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeIsoFract: {
      auto settings = new IsoFractSettings(shaderId, shaderJson);
      auto shader = std::make_shared<IsoFractShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeSwirl: {
      auto settings = new SwirlSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<SwirlShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeTraceAudio: {
      auto settings = new TraceAudioSettings(shaderId, shaderJson);
      auto shader = std::make_shared<TraceAudioShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypePixelPlay: {
      auto settings = new PixelPlaySettings(shaderId, shaderJson);
      auto shader = std::make_shared<PixelPlayShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeDirtyPlasma: {
      auto settings = new DirtyPlasmaSettings(shaderId, shaderJson);
      auto shader = std::make_shared<DirtyPlasmaShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeOneBitDither: {
      auto settings = new OneBitDitherSettings(shaderId, shaderJson);
      auto shader = std::make_shared<OneBitDitherShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeOldTV: {
      auto settings = new OldTVSettings(shaderId, shaderJson);
      auto shader = std::make_shared<OldTVShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeTwistedTrip: {
      auto settings = new TwistedTripSettings(shaderId, shaderJson);
      auto shader = std::make_shared<TwistedTripShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeTwistedCubes: {
      auto settings = new TwistedCubesSettings(shaderId, shaderJson);
      auto shader = std::make_shared<TwistedCubesShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeCore: {
      auto settings = new CoreSettings(shaderId, shaderJson);
      auto shader = std::make_shared<CoreShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeVoronoiColumns: {
      auto settings = new VoronoiColumnsSettings(shaderId, shaderJson);
      auto shader = std::make_shared<VoronoiColumnsShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeGodRay: {
      auto settings = new GodRaySettings(shaderId, shaderJson);
      auto shader = std::make_shared<GodRayShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeWarpspeed: {
      auto settings = new WarpspeedSettings(shaderId, shaderJson);
      auto shader = std::make_shared<WarpspeedShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeReflector: {
      auto settings = new ReflectorSettings(shaderId, shaderJson);
      auto shader = std::make_shared<ReflectorShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeBlend: {
      auto settings = new BlendSettings(shaderId, shaderJson);
      auto shader = std::make_shared<BlendShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeFullHouse: {
      auto settings = new FullHouseSettings(shaderId, shaderJson);
      auto shader = std::make_shared<FullHouseShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypePieSplit: {
      auto settings = new PieSplitSettings(shaderId, shaderJson);
      auto shader = std::make_shared<PieSplitShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeOverlay: {
      auto settings = new OverlaySettings(shaderId, shaderJson);
      auto shader = std::make_shared<OverlayShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeSwitcher: {
      auto settings = new SwitcherSettings(shaderId, shaderJson);
      auto shader = std::make_shared<SwitcherShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeRotate: {
      auto settings = new RotateSettings(shaderId, shaderJson);
      auto shader = std::make_shared<RotateShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeMultiMix: {
      auto settings = new MultiMixSettings(shaderId, shaderJson);
      auto shader = std::make_shared<MultiMixShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeColorStepper: {
      auto settings = new ColorStepperSettings(shaderId, shaderJson);
      auto shader = std::make_shared<ColorStepperShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeGridRun: {
      auto settings = new GridRunSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<GridRunShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeColorSwap: {
      auto settings = new ColorSwapSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<ColorSwapShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeAlphaMix: {
      auto settings = new AlphaMixSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<AlphaMixShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeGyroids: {
      auto settings = new GyroidsSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<GyroidsShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeCubify: {
      auto settings = new CubifySettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<CubifyShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeSwirlingSoul: {
      auto settings = new SwirlingSoulSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<SwirlingSoulShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeDoubleSwirl: {
      auto settings = new DoubleSwirlSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<DoubleSwirlShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeSmokeRing: {
      auto settings = new SmokeRingSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<SmokeRingShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeAudioCircle: {
      auto settings = new AudioCircleSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<AudioCircleShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeLimbo:
    {
      auto settings = new LimboSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<LimboShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeTextureMask:
    {
      auto settings = new TextureMaskSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<TextureMaskShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeSnowfall:
    {
      auto settings = new SnowfallSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<SnowfallShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeCanny:
    {
      auto settings = new CannySettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<CannyShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeVHS:
    {
      auto settings = new VHSSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<VHSShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeCirclePath:
    {
      auto settings = new CirclePathSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<CirclePathShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeBounce:
    {
      auto settings = new BounceSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<BounceShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeVertex:
    {
      auto settings = new VertexSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<VertexShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeTriple:
    {
      auto settings = new TripleSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<TripleShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypePaint:
    {
      auto settings = new PaintSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<PaintShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeGameboy:
    {
      auto settings = new GameboySettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<GameboyShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeStaticFrame:
    {
      auto settings = new StaticFrameSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<StaticFrameShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeSlidingFrame:
    {
      auto settings = new SlidingFrameSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<SlidingFrameShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeMinMixer:
    {
      auto settings = new MinMixerSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<MinMixerShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeSolidColor:
    {
      auto settings = new SolidColorSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<SolidColorShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderType16bit:
    {
      auto settings = new SixteenBitSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<SixteenBitShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeHilbert:
    {
      auto settings = new HilbertSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<HilbertShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeWarp:
    {
      auto settings = new WarpSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<WarpShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeFrequencyVisualizer:
    {
      auto settings = new FrequencyVisualizerSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<FrequencyVisualizerShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeColorKeyMaskMaker:
    {
      auto settings = new ColorKeyMaskMakerSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<ColorKeyMaskMakerShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeCurlySquares:
    {
      auto settings = new CurlySquaresSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<CurlySquaresShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypePlasmaTwo:
    {
      auto settings = new PlasmaTwoSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<PlasmaTwoShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeDancingSquares:
    {
      auto settings = new DancingSquaresSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<DancingSquaresShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeLumaMaskMaker:
    {
      auto settings = new LumaMaskMakerSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<LumaMaskMakerShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeMask:
    {
      auto settings = new MaskSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<MaskShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeCircle:
    {
      auto settings = new CircleSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<CircleShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeCrosshatch:
    {
      auto settings = new CrosshatchSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<CrosshatchShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeHalfTone:
    {
      auto settings = new HalfToneSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<HalfToneShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeTissue:
    {
      auto settings = new TissueSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<TissueShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypePsycurves:
    {
      auto settings = new PsycurvesSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<PsycurvesShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeFishEye:
    {
      auto settings = new FishEyeSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<FishEyeShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeColorPass:
    {
      auto settings = new ColorPassSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<ColorPassShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeSobel:
    {
      auto settings = new SobelSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<SobelShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeTriangleMap:
    {
      auto settings = new TriangleMapSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<TriangleMapShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeLiquid:
    {
      auto settings = new LiquidSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<LiquidShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeDisco:
    {
      auto settings = new DiscoSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<DiscoShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeOctahedron:
    {
      auto settings = new OctahedronSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<OctahedronShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeVanGogh:
    {
      auto settings = new VanGoghSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<VanGoghShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeRubiks:
    {
      auto settings = new RubiksSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<RubiksShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeRainbowRotator:
    {
      auto settings = new RainbowRotatorSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<RainbowRotatorShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeDither:
    {
      auto settings = new DitherSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<DitherShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeMountains:
    {
      auto settings = new MountainsSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<MountainsShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeGalaxy:
    {
      auto settings = new GalaxySettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<GalaxyShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeAudioMountains:
    {
      auto settings = new AudioMountainsSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<AudioMountainsShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeAudioBumper:
    {
      auto settings = new AudioBumperSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<AudioBumperShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeAudioWaveform:
    {
      auto settings = new AudioWaveformSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<AudioWaveformShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeRings:
    {
      auto settings = new RingsSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<RingsShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeSlider:
    {
      auto settings = new SliderSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<SliderShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeWobble:
    {
      auto settings = new WobbleSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<WobbleShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeRGBShift:
    {
      auto settings = new RGBShiftSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<RGBShiftShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeNone:
      return 0;
    case ShaderTypeHSB:
    {
      auto settings = new HSBSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<HSBShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeBlur:
    {
      auto settings = new BlurSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<BlurShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypePixelate:
    {
      auto settings = new PixelSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<PixelShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeGlitch:
    {
      auto settings = new GlitchSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<GlitchShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeMirror:
    {
      auto settings = new MirrorSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<MirrorShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeTransform:
    {
      auto settings = new TransformSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<TransformShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeFeedback:
    {
      auto settings = new FeedbackSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<FeedbackShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeAscii:
    {
      auto settings = new AsciiSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<AsciiShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeKaleidoscope:
    {
      auto settings = new KaleidoscopeSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<KaleidoscopeShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeTile:
    {
      auto settings = new TileSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<TileShader>(settings);
      shader->setup();
      return shader;
    }
    case ShaderTypeMix:
    {
      auto settings = new MixSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<MixShader>(settings);
      shader.get()->setup();
      return shader;
    }
    case ShaderTypePlasma:
    {
      auto settings = new PlasmaSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<PlasmaShader>(settings);
      shader.get()->setup();
      return shader;
    }
    case ShaderTypeFuji:
    {
      auto settings = new FujiSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<FujiShader>(settings);
      shader.get()->setup();
      return shader;
      break;
    }
    case ShaderTypeFractal:
    {
      auto settings = new FractalSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<FractalShader>(settings);
      shader.get()->setup();
      return shader;
      break;
    }
    case ShaderTypeClouds:
    {
      auto settings = new CloudSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<CloudShader>(settings);
      shader.get()->setup();
      return shader;
      break;
    }
    case ShaderTypeMelter:
    {
      auto settings = new MeltSettings(shaderId, shaderJson, shaderTypeName(shaderType));
      auto shader = std::make_shared<MeltShader>(settings);
      shader.get()->setup();
      return shader;
      break;
    }
  }
}

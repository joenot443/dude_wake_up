//
//  ShaderChainerServiceTests.cpp
//  dude_wake_up_tests
//
//  Created by Joe Crozier on 4/18/23.
//

#include <stdio.h>
#include <gtest/gtest.h>
#include <math.h>
#include "ofMain.h"
#include "MainApp.h"
#include "ShaderChainerService.hpp"
#include "FontService.hpp"
#include "ModulationService.hpp"
#include "OscillationService.hpp"
#include "AudioSourceService.hpp"
#include "MidiService.hpp"
#include "ConfigService.hpp"
#include "FeedbackSourceService.hpp"
#include "LibraryService.hpp"
#include "VideoSourceService.hpp"
#include "NodeLayoutView.hpp"
#include "MarkdownService.hpp"
#include "ShaderChainerService.hpp"
#include "LayoutStateService.hpp"
#include "ParameterService.hpp"

NodeLayoutView *NodeLayoutView::instance = 0;
FontService *FontService::service = 0;
ModulationService *ModulationService::service = 0;
OscillationService *OscillationService::service = 0;
MidiService *MidiService::service = 0;
ParameterService *ParameterService::service = 0;
ConfigService *ConfigService::service = 0;
FeedbackSourceService *FeedbackSourceService::service = 0;
VideoSourceService *VideoSourceService::service = 0;
ShaderChainerService *ShaderChainerService::service = 0;
AudioSourceService *AudioSourceService::service = 0;
LayoutStateService *LayoutStateService::service = 0;
MarkdownService *MarkdownService::service = 0;
LibraryService *LibraryService::service = 0;

// The fixture for testing ShaderChainerService.
//
// Test to add a Shader to the ShaderChainerService
TEST(ShaderChainerServiceTests, AddShaderTest) {
  auto service = ShaderChainerService::getService();

  auto shader = service->shaderForType(ShaderTypeHSB, UUID::generateUUID(), 0);
  ASSERT_TRUE(shader != nullptr);

  service->addShader(shader);
  ASSERT_TRUE(service->shaders().size() == 1);
}

TEST(ShaderChainerServiceTests, AddShaderChainerTest) {
  // Create ShaderChainerService instance
  auto service = ShaderChainerService::getService();

  // Create a VideoSource using the default constructor
  auto videoSource = std::make_shared<VideoSource>();

  // Create a ShaderChainer using its constructor
  std::string chainerId = UUID::generateUUID();
  std::string chainerName = "Test Chainer";
  auto shaderChainer = std::make_shared<ShaderChainer>(chainerId, chainerName, videoSource);

  // Add the ShaderChainer to the ShaderChainerService
  service->addShaderChainer(shaderChainer);

  // Check if the ShaderChainer has been added correctly
  auto addedChainer = service->shaderChainerForId(chainerId);
  ASSERT_TRUE(addedChainer != nullptr);
  ASSERT_EQ(addedChainer->chainerId, chainerId);
  ASSERT_EQ(addedChainer->name, chainerName);
  ASSERT_EQ(addedChainer->source, videoSource);
}

TEST(ShaderChainerTests, CreateShaderChainerWithVideoSource) {
  auto videoSource = std::make_shared<VideoSource>(UUID::generateUUID(), "TestVideoSource", VideoSource_chainer);

  std::string chainerId = UUID::generateUUID();
  auto shaderChainer = std::make_shared<ShaderChainer>(chainerId, "TestShaderChainer", videoSource);

  ASSERT_EQ(shaderChainer->chainerId, chainerId);
  ASSERT_EQ(shaderChainer->name, "TestShaderChainer");
  ASSERT_EQ(shaderChainer->source, videoSource);
}

TEST(ShaderChainerServiceTests, UpdateShaderChainers) {
  ShaderChainerService::recreateService();
  auto service = ShaderChainerService::getService();

  // Create VideoSource
  auto videoSource = std::make_shared<VideoSource>(UUID::generateUUID(), "TestVideoSource", VideoSource_chainer);

  // Create ShaderChainer and add it to the service
  auto shaderChainer = service->addNewShaderChainer(videoSource);
  ASSERT_EQ(service->shaderChainers().size(), 1);

  // Create Shaders
  auto shader1 = service->makeShader(ShaderTypeHSB);
  auto shader2 = service->makeShader(ShaderTypeBlur);
  auto shader3 = service->makeShader(ShaderTypePixelate);

  // Add Shaders to the service
  service->addShader(shader1);
  service->addShader(shader2);
  service->addShader(shader3);

  // Link Shaders
  service->linkShaderToNext(shader1, shader2);
  service->linkShaderToNext(shader2, shader3);

  // Add shader1 as the front of the ShaderChainer
  service->addShaderToFront(shader1, shaderChainer);
  
  // Check ShaderChainer front and next shaders
  ASSERT_EQ(shaderChainer->front, shader1);
  ASSERT_EQ(shaderChainer->front->next, shader2);
  ASSERT_EQ(shaderChainer->front->next->next, shader3);

  // Link VideoSource to shader3's Aux
  service->linkVideoSourceToShaderAux(videoSource, shader3);

  // Check if shader3 has videoSource as its sourceAux
  ASSERT_EQ(shader3->sourceAux, videoSource);

  // Break the ShaderChainer's front link
  service->breakShaderChainerFront(shaderChainer);

  // Check if the front of ShaderChainer is nullptr
  ASSERT_EQ(shaderChainer->front, nullptr);

  // Link VideoSource to shader1
  service->linkVideoSourceToShader(shaderChainer->source, shader1);

  // Check if the front of ShaderChainer is shader1
  ASSERT_EQ(shaderChainer->front, shader1);
}


TEST(ShaderChainerServiceTests, BreakLinksInShaderChainers) {
  ShaderChainerService::recreateService();
  auto service = ShaderChainerService::getService();

  // Create VideoSource
  auto videoSource = std::make_shared<VideoSource>(UUID::generateUUID(), "TestVideoSource", VideoSource_chainer);

  // Create ShaderChainer and add it to the service
  auto shaderChainer = service->addNewShaderChainer(videoSource);
  ASSERT_EQ(service->shaderChainers().size(), 1);

  // Create Shaders
  auto shader1 = service->makeShader(ShaderTypeHSB);
  auto shader2 = service->makeShader(ShaderTypeBlur);
  auto shader3 = service->makeShader(ShaderTypePixelate);

  // Add Shaders to the service
  service->addShader(shader1);
  service->addShader(shader2);
  service->addShader(shader3);

  // Link Shaders
  service->linkShaderToNext(shader1, shader2);
  service->linkShaderToNext(shader2, shader3);

  // Add shader1 as the front of the ShaderChainer
  service->addShaderToFront(shader1, shaderChainer);

  // Check ShaderChainer front and next shaders
  ASSERT_EQ(shaderChainer->front, shader1);
  ASSERT_EQ(shaderChainer->front->next, shader2);
  ASSERT_EQ(shaderChainer->front->next->next, shader3);

  // Link VideoSource to shader3's Aux
  service->linkVideoSourceToShaderAux(videoSource, shader3);

  // Check if shader3 has videoSource as its sourceAux
  ASSERT_EQ(shader3->sourceAux, videoSource);

  // Break the link between shader1 and shader2
  service->breakShaderNextLink(shader1);

  // Check if shader1's next is nullptr
  ASSERT_EQ(shader1->next, nullptr);

  // Break the Aux link of shader3
  service->breakSourceShaderAuxLink(videoSource, shader3);

  // Check if shader3's sourceAux and aux are nullptr
  ASSERT_EQ(shader3->sourceAux, nullptr);
  ASSERT_EQ(shader3->aux, nullptr);

  // Link VideoSource to shader3's Aux again
  service->linkVideoSourceToShaderAux(videoSource, shader3);

  // Break the sourceAux link of shader3
  service->breakSourceShaderAuxLink(videoSource, shader3);

  // Check if shader3's sourceAux is nullptr
  ASSERT_EQ(shader3->sourceAux, nullptr);
}

TEST(ShaderChainerServiceTests, LinkAndBreakVideoSourceToShaderAux) {
  ShaderChainerService::recreateService();
  auto service = ShaderChainerService::getService();

  // Create VideoSource
  auto videoSource = std::make_shared<VideoSource>(UUID::generateUUID(), "TestVideoSource", VideoSource_chainer);

  // Create ShaderChainer and add it to the service
  auto shaderChainer = service->addNewShaderChainer(videoSource);
  ASSERT_EQ(service->shaderChainers().size(), 1);

  // Create Shaders
  auto shader1 = service->makeShader(ShaderTypeHSB);
  auto shader2 = service->makeShader(ShaderTypeBlur);

  // Add Shaders to the service
  service->addShader(shader1);
  service->addShader(shader2);

  // Link shader1 as the front of the ShaderChainer
  service->addShaderToFront(shader1, shaderChainer);

  // Check ShaderChainer front
  ASSERT_EQ(shaderChainer->front, shader1);

  // Link VideoSource to shader2's Aux
  service->linkVideoSourceToShaderAux(videoSource, shader2);

  // Check if shader2 has videoSource as its sourceAux
  ASSERT_EQ(shader2->sourceAux, videoSource);

  // Break the sourceAux link of shader2
  service->breakSourceShaderAuxLink(videoSource, shader2);

  // Check if shader2's sourceAux is nullptr
  ASSERT_EQ(shader2->sourceAux, nullptr);
}

TEST(ShaderChainerServiceTests, FindShaderChainersForAuxShaderAndVideoSource) {
  ShaderChainerService::recreateService();
  auto service = ShaderChainerService::getService();

  // Create VideoSources
  auto videoSource1 = std::make_shared<VideoSource>(UUID::generateUUID(), "TestVideoSource1", VideoSource_chainer);
  auto videoSource2 = std::make_shared<VideoSource>(UUID::generateUUID(), "TestVideoSource2", VideoSource_chainer);

  // Create ShaderChainers and add them to the service
  auto shaderChainer1 = service->addNewShaderChainer(videoSource1);
  auto shaderChainer2 = service->addNewShaderChainer(videoSource2);
  ASSERT_EQ(service->shaderChainers().size(), 2);

  // Create Shaders
  auto shader1 = service->makeShader(ShaderTypeHSB);
  auto shader2 = service->makeShader(ShaderTypeBlur);

  // Add Shaders to the service
  service->addShader(shader1);
  service->addShader(shader2);

  // Link shader1 as the front of the ShaderChainer1 and shader2 as the front of the ShaderChainer2
  service->addShaderToFront(shader1, shaderChainer1);
  service->addShaderToFront(shader2, shaderChainer2);

  // Check ShaderChainer fronts
  ASSERT_EQ(shaderChainer1->front, shader1);
  ASSERT_EQ(shaderChainer2->front, shader2);

  // Link VideoSource2 to shader1's Aux
  auto auxChainer = service->linkVideoSourceToShaderAux(videoSource2, shader1);

  // Check if shader1 has videoSource2 as its sourceAux
  ASSERT_EQ(shader1->sourceAux, videoSource2);

  // Find ShaderChainers for videoSource1
  auto foundChainers = service->shaderChainersForVideoSourceId(videoSource1->id);
  ASSERT_EQ(foundChainers.size(), 1);
  ASSERT_EQ(foundChainers[0], shaderChainer1);

  // Find ShaderChainers for videoSource2
  foundChainers = service->shaderChainersForVideoSourceId(videoSource2->id);
  ASSERT_EQ(foundChainers.size(), 2);
  ASSERT_EQ(foundChainers[0], shaderChainer2);
}


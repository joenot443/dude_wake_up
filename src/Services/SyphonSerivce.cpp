#include "SyphonService.hpp"

SyphonService::SyphonService() {
  // Constructor implementation if needed
}

void SyphonService::setup() {
  // Initialize Syphon server and client setup
  syphonServer.setName("Syphon Server");
  syphonClient.setup();
}

SyphonService* SyphonService::getService() {
  if (!service) {
    service = new SyphonService;
    service->setup();
  }
  return service;
}

void SyphonService::publishFbo(std::shared_ptr<ofFbo> fbo) {
  attachedFbo = fbo;
}

void SyphonService::update() {
  if (attachedFbo) {
    syphonServer.publishTexture(&attachedFbo->getTexture());
  }
}

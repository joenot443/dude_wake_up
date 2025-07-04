#include "TimeService.hpp"
#include "OscillationService.hpp"
#include "ParameterService.hpp"

void TimeService::setup() {
  OscillationService::getService()->addOscillator(timeOscillator);
  OscillationService::getService()->addOscillator(timeSpeedOscillator);
  OscillationService::getService()->addOscillator(timeOffsetOscillator);
  ParameterService::getService()->registerParameter(timeParam);
  ParameterService::getService()->registerParameter(timeSpeedParam);
  ParameterService::getService()->registerParameter(timeOffsetParam);
}

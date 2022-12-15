//
//  Oscillator.cpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-26.
//

#include "Oscillator.hpp"
#include "OscillationService.hpp"
#include "Video.hpp"

const float OscillatorDampen = 0.3;

Oscillator::Oscillator(std::shared_ptr<Parameter> v) :
value(v),
name(v->name),
settingsId(value->paramId),
amplitude(std::make_shared<Parameter>("amp", v->paramId, 1.0, 0.0, v->max)),
shift(std::make_shared<Parameter>("shift", v->paramId, (v->max - v->min) / 2., -3.0, 3.0)),
frequency(std::make_shared<Parameter>("freq", v->paramId, 0.5, 0.0, 3.0)),
enabled(std::make_shared<Parameter>("enabled", v->paramId, 0.0, 0.0, 1.0))
{
  parameters = {amplitude, frequency, shift, enabled};
  data = ImVector<ImVec2>();
  data.reserve(100);
  xRange = {0.0, 10.0};
  span = 10.0;
  yRange = {value->min, value->max};
  OscillationService::getService()->addOscillator(std::shared_ptr<Oscillator>(this));
}

void Oscillator::tick() {
  if (!enabled->boolValue) return;
  
  float t = frameTime();  float xmod = fmodf(t, span);
  if (!data.empty() && xmod < data.back().x)
      data.shrink(0);
  
  float v = OscillatorDampen * amplitude->value * sin(frequency->value * t) + shift->value;
  value->setValue(fmin(fmax(v, value->min), value->max));
  data.push_back(ImVec2(xmod, value->value));
}

void Oscillator::load(json j) {
  
}

json Oscillator::serialize() {
  json j;
  
  for (auto p : parameters) {
    // Only serialize the parameters which are not default
    if (p->value != p->defaultValue) {
      j[p->name] = p->serialize();
    }
  }
  return j;
}

std::map<std::string, float> Oscillator::parameterValueMap() {
  std::map<std::string, float> m;
  for (auto p : parameters) {
    if (p->value != p->defaultValue) {
      m[p->name] = p->value;
    }
  }
  return m;
}

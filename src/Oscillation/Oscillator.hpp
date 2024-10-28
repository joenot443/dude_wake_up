//
//  Oscillator.hpp
//  dude_wake_up
//
//  Created by Joe Crozier on 2022-05-26.
//

#ifndef Oscillator_hpp
#define Oscillator_hpp

#include "Parameter.hpp"
#include <memory>
#include "imgui.h"
#include "json.hpp"
#include <stdio.h>

using json = nlohmann::json;

enum OscillatorType
{
  OscillatorType_value,
  OscillatorType_pulse,
  OscillatorType_waveform
};

struct Oscillator: public std::enable_shared_from_this<Oscillator>
{
public:
  virtual OscillatorType type() = 0;

  std::shared_ptr<Parameter> enabled;

  std::shared_ptr<Parameter> value;

  std::vector<std::shared_ptr<Parameter>> parameters;
  float span;

  std::string settingsId;
  std::string name;

  std::vector<float> xRange;
  std::vector<float> yRange;
  ImVector<ImVec2> data;

  virtual void tick() = 0;

  void load(json j)
  {
    for (auto p : parameters) {
      if (j[p->name].is_object()) {
        p->load(j[p->name]);
      }
    }
  }
  
  std::shared_ptr<Parameter> findParameter(std::string name) {
    for (auto p : parameters) {
      if (p->name == name) {
        return p;
      }
    }
    return nullptr;
  }

  // Returns the y values from data
  std::vector<float> values()
  {
    std::vector<float> v;
    for (auto d : data)
    {
      v.push_back(d.y);
    }
    return v;
  }

  json serialize()
  {
    json j = json::object();

    for (auto p : parameters)
    {
      // Only serialize the parameters which are not default
      if (p->value != p->defaultValue)
      {
        j[p->name] = p->serialize();
      }
    }
    return j;
  }

  Oscillator(std::shared_ptr<Parameter> v) : value(v),
                                             name(v->name),
                                             settingsId(value->paramId),
                                             enabled(std::make_shared<Parameter>("enabled", 0.0, 0.0, 1.0, ParameterType_Hidden))
  {
    parameters = {enabled};
    data = ImVector<ImVec2>();
    xRange = {0.0, 10.0};
    span = 10.0;
    yRange = {value->min, value->max};
  }

private:
  void pushValue(float value);
};

#endif /* Oscillator_hpp */

// Copyright (c) 2017 The ComPWA Team.
// This file is part of the ComPWA framework, check
// https://github.com/ComPWA/ComPWA/license.txt for details.

#ifndef SequentialPartialAmplitude_h
#define SequentialPartialAmplitude_h

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

#include "Core/FitParameter.hpp"
#include "Physics/Amplitude.hpp"
#include "Physics/HelicityFormalism/HelicityDecay.hpp"

namespace ComPWA {
namespace Physics {
namespace HelicityFormalism {

class SequentialPartialAmplitude : public Amplitude {

public:
  SequentialPartialAmplitude(std::shared_ptr<PartList> partL,
                             std::shared_ptr<Kinematics> kin,
                             const boost::property_tree::ptree &pt);

  virtual Amplitude *clone(std::string newName = "") const {
    auto tmp = (new SequentialPartialAmplitude(*this));
    tmp->setName(newName);
    return tmp;
  };

  void load(std::shared_ptr<PartList> partL, std::shared_ptr<Kinematics> kin,
          const boost::property_tree::ptree &pt);

  virtual boost::property_tree::ptree save() const;

  /// Check if parameters have changed.
  virtual bool isModified() const;

  /// Label as modified/unmodified
  virtual void setModified(bool b);

  virtual std::complex<double> evaluate(const DataPoint &point) const {
    std::complex<double> result = coefficient() * preFactor();
    for (auto i : PartialAmplitudes)
      result *= i->evaluate(point);

    assert(!std::isnan(result.real()) && !std::isnan(result.imag()));
    return result;
  };

  void
  addPartialAmplitude(std::shared_ptr<ComPWA::Physics::PartialAmplitude> d) {
    PartialAmplitudes.push_back(d);
  }

  std::shared_ptr<ComPWA::Physics::PartialAmplitude> partialAmplitude(int pos) {
    return PartialAmplitudes.at(pos);
  }

  std::vector<std::shared_ptr<ComPWA::Physics::PartialAmplitude>> &
  partialAmplitudes() {
    return PartialAmplitudes;
  }

  virtual void parameters(ParameterList &list);

  virtual void parametersFast(std::vector<double> &list) const {
    Amplitude::parametersFast(list);
    for (auto i : PartialAmplitudes)
      i->parametersFast(list);
  }

  /// Update parameters to the values given in \p par
  virtual void updateParameters(const ParameterList &par);

  /// Set phase space sample
  /// We use the phase space sample to calculate the normalization. The sample
  /// should be without efficiency applied.
  virtual void
  setPhspSample(std::shared_ptr<std::vector<ComPWA::DataPoint>> phspSample) {
    for (auto i : PartialAmplitudes)
      i->setPhspSample(phspSample);
  }

  virtual bool hasTree() const { return true; }

  virtual std::shared_ptr<FunctionTree> tree(std::shared_ptr<Kinematics> kin,
                                             const ParameterList &sample,
                                             const ParameterList &toySample,
                                             std::string suffix = "");

protected:
  std::vector<std::shared_ptr<ComPWA::Physics::PartialAmplitude>>
      PartialAmplitudes;
};

} // ns::HelicityFormalism
} // ns::Physics
} // ns::ComPWA

#endif

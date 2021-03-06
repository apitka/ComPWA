// Copyright (c) 2013, 2017 The ComPWA Team.
// This file is part of the ComPWA framework, check
// https://github.com/ComPWA/ComPWA/license.txt for details.

#ifndef PHYSICS_DECAYDYNAMICS_RELATIVISTICBREITWIGNER_HPP_
#define PHYSICS_DECAYDYNAMICS_RELATIVISTICBREITWIGNER_HPP_

#include <vector>
#include <memory>
#include <boost/property_tree/ptree.hpp>

#include "Core/Spin.hpp"
#include "Core/Functions.hpp"
#include "Core/Exceptions.hpp"
#include "Physics/DecayDynamics/AbstractDynamicalFunction.hpp"
#include "Physics/HelicityFormalism/AmpWignerD.hpp"

namespace ComPWA {
namespace Physics {
namespace DecayDynamics {

class HelicityDecay;

/// \class RelativisticBreitWigner
/// Relativistic Breit-Wigner model with barrier factors
/// The dynamical function implemented here is taken from PDG2014 (Eq.47-22) for
/// the one channel case.
/// due to the implementation, after create a RelBW with constructor,
/// one need to update the Orbital Angular Momentum intermediately before further processing
/// otherwise spin J will be used instead of Orbital Angular Momentum
class RelativisticBreitWigner
    : public ComPWA::Physics::DecayDynamics::AbstractDynamicalFunction {

public:
  //============ CONSTRUCTION ==================
  RelativisticBreitWigner(std::string name, std::pair<std::string,std::string> daughters,
               std::shared_ptr<ComPWA::PartList> partL);


  //======= INTEGRATION/NORMALIZATION ===========
  /// Check of parameters have changed and normalization has to be recalculatecd
  virtual bool isModified() const;

  /// Label as modified/unmodified
  virtual void setModified(bool b);
  
  //================ EVALUATION =================
  
  std::complex<double> evaluate(const ComPWA::DataPoint &point, int pos) const;

  /// Dynamical Breit-Wigner function.
  /// \param mSq Invariant mass squared
  /// \param mR Mass of the resonant state
  /// \param ma Mass of daughter particle
  /// \param mb Mass of daughter particle
  /// \param width Decay width
  /// \param L Orbital angular momentum between two daughters a and b
  /// \param mesonRadius Meson Radius
  /// \param ffType Form factor type
  /// \return Amplitude value
  static std::complex<double>
  dynamicalFunction(double mSq, double mR, double ma, double mb, double width,
                    unsigned int L, double mesonRadius, formFactorType ffType);

  //============ SET/GET =================

  void SetWidthParameter(std::shared_ptr<ComPWA::FitParameter> w) {
   Width = w;
  }

  std::shared_ptr<ComPWA::FitParameter> GetWidthParameter() {
    return Width;
  }

  void SetWidth(double w) {Width->setValue(w); }

  double GetWidth() const { return Width->value(); }

  void SetMesonRadiusParameter(std::shared_ptr<ComPWA::FitParameter> r) {
    MesonRadius = r;
  }

  std::shared_ptr<ComPWA::FitParameter> GetMesonRadiusParameter() {
    return MesonRadius;
  }

  /// \see GetMesonRadius() const { return MesonRadius->value(); }
  void SetMesonRadius(double w) { MesonRadius->setValue(w); }

  /// Get meson radius.
  /// The meson radius is a measure of the size of the resonant state. It is
  /// used to calculate the angular momentum barrier factors.
  double GetMesonRadius() const { return MesonRadius->value(); }

  /// \see GetFormFactorType()
  void SetFormFactorType(formFactorType t) { FormFactorType = t; }

  /// Get form factor type.
  /// The type of formfactor that is used to calculate the angular momentum
  /// barrier factors.
  formFactorType GetFormFactorType() { return FormFactorType; }

  virtual void parameters(ComPWA::ParameterList &list);

  virtual void parametersFast(std::vector<double> &list) const {
    AbstractDynamicalFunction::parametersFast(list);
    list.push_back(GetWidth());
    list.push_back(GetMesonRadius());
  }

  /// Update parameters to the values given in \p par
  virtual void updateParameters(const ComPWA::ParameterList &par);

  //=========== FUNCTIONTREE =================

  virtual bool hasTree() const { return true; }

  virtual std::shared_ptr<ComPWA::FunctionTree>
  tree(const ParameterList &sample, int pos, std::string suffix = "");

protected:
  /// Decay width of resonante state
  std::shared_ptr<ComPWA::FitParameter> Width;

  /// Meson radius of resonant state
  std::shared_ptr<ComPWA::FitParameter> MesonRadius;

  /// Form factor type
  formFactorType FormFactorType;

private:
  /// Temporary value (used to trigger recalculation of normalization)
  double CurrentMesonRadius;
  /// Temporary value (used to trigger recalculation of normalization)
  double CurrentWidth;
};

class BreitWignerStrategy : public ComPWA::Strategy {
public:
  BreitWignerStrategy(std::string namee = "")
      : ComPWA::Strategy(ParType::MCOMPLEX), name(namee) {}

  virtual const std::string to_str() const {
    return ("relativistic BreitWigner of " + name);
  }

  virtual void execute(ComPWA::ParameterList &paras,
                       std::shared_ptr<ComPWA::Parameter> &out);

protected:
  std::string name;
};

} // namespace DecayDynamics
} // namespace Physics
} // namespace ComPWA

#endif

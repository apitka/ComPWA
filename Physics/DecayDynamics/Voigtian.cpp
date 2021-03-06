// Copyright (c) 2013, 2017 The ComPWA Team.
// This file is part of the ComPWA framework, check
// https://github.com/ComPWA/ComPWA/license.txt for details.

#include <cmath>
#include <numeric>
#include <iterator>
#include <vector>

#include <boost/property_tree/ptree.hpp>

#include "Physics/HelicityFormalism/HelicityKinematics.hpp"
#include "Physics/DecayDynamics/Voigtian.hpp"
#include "Physics/DecayDynamics/Coupling.hpp"

using namespace ComPWA::Physics::DecayDynamics;

Voigtian::Voigtian(std::string name,
                   std::pair<std::string, std::string> daughters,
                   std::shared_ptr<ComPWA::PartList> partL) {

  LOG(TRACE) << "Voigtian::Factory() | Construction of " << name << ".";
  setName(name);
  auto partProp = partL->find(name)->second;
  SetMassParameter(std::make_shared<FitParameter>(partProp.GetMassPar()));

  auto decayTr = partProp.GetDecayInfo();
  if (partProp.GetDecayType() != "voigt")
    throw std::runtime_error(
        "Voigtian::Factory() | Decay type does not match! ");

  auto spin = partProp.GetSpinQuantumNumber("Spin");
  SetSpin(spin);
  // in default, using spin J as Orbital Angular Momentum
  // update by calling SetOrbitalAngularMomentum() before any further process
  // after RelBW is created by calling of constructor
  SetOrbitalAngularMomentum(spin);

  // auto ffType =
  // formFactorType(decayTr.get<int>("FormFactor.<xmlattr>.Type"));
  // SetFormFactorType(ffType);
  for (const auto &v : decayTr.get_child("")) {
    if (v.first != "Parameter")
      continue;
    std::string type = v.second.get<std::string>("<xmlattr>.Type");
    if (type == "Width") {
      SetWidthParameter(std::make_shared<FitParameter>(v.second));
    } else if (type == "MesonRadius") {
      ; // do nothing
      //      SetMesonRadiusParameter(
      //          std::make_shared<FitParameter>(v.second));
    } else {
      throw std::runtime_error("Voigtian::Factory() | Parameter of type " +
                               type + " is unknown.");
    }
  }
  double sigma = decayTr.get<double>("Resolution.<xmlattr>.Sigma");
  SetSigma(sigma);

  std::pair<double, double> daughterMasses(
      partL->find(daughters.first)->second.GetMass(),
      partL->find(daughters.second)->second.GetMass());
  SetDecayMasses(daughterMasses);
  SetDecayNames(daughters);

  LOG(TRACE) << "Voigtian::Factory() | Construction of the decay "
             << partProp.name() << " -> " << daughters.first << " + "
             << daughters.second;
}

std::complex<double> Voigtian::evaluate(const DataPoint &point, int pos) const {
  std::complex<double> result =
      dynamicalFunction(point.value(pos), Mass->value(), Width->value(), Sigma);
  assert(!std::isnan(result.real()) && !std::isnan(result.imag()));
  return result;
}

bool Voigtian::isModified() const {
  if (GetMass() != Current_mass || Width->value() != CurrentWidth)
    return true;
  return false;
}

void Voigtian::setModified(bool b) {
  if (b) {
    Current_mass = std::numeric_limits<double>::quiet_NaN();
    CurrentWidth = std::numeric_limits<double>::quiet_NaN();
  } else {
    Current_mass = Mass->value();
    CurrentWidth = Width->value();
  }
}

std::complex<double> Voigtian::dynamicalFunction(double mSq, double mR,
                                                 double wR, double sigma) {

  double sqrtS = sqrt(mSq);

  // the non-relativistic BreitWigner which is convoluted in Voigtian
  // has the exactly following expression:
  // BW(x, m, width) = 1/pi * width/2 * 1/((x - m)^2 + (width/2)^2)
  // i.e., the Lorentz formula with Gamma = width/2 and x' = x - m
  /// https://root.cern.ch/doc/master/RooVoigtianian_8cxx_source.html
  double argu = sqrtS - mR;
  double c = 1.0 / (sqrt(2.0) * sigma);
  double a = c * 0.5 * wR;
  double u = c * argu;
  std::complex<double> z(u, a);
  std::complex<double> v = Faddeeva::w(z, 1e-13);
  double val = c * 1.0 / sqrt(M_PI) * v.real();
  double sqrtVal = sqrt(val);

  /// keep the phi angle of the complex BW
  std::complex<double> invBW(argu, 0.5 * wR);
  std::complex<double> BW = 1.0 / invBW;
  double phi = std::arg(BW);
  std::complex<double> result(sqrtVal * cos(phi), sqrtVal * sin(phi));

  // transform width to coupling
  // Calculate coupling constant to final state
  // MesonRadius = 0.0, noFormFactor
  // std::complex<double> g_final = widthToCoupling(mSq, mR, wR, ma, mb, L, 0.0,
  // formFactorType::noFormFactor);
  // the BW to convolved in voigt is 1/PI * Gamma/2 * 1/((x-m)^2 + (Gamma/2)^2)
  // while I think the one common used in physics is Gamma/2 * 1/((x-m)^2 +
  // (Gamma/2)^2)
  // So we time the PI at last
  std::complex<double> g_final = sqrt(M_PI);
  double g_production = 1;
  result *= g_production;
  result *= g_final;

  assert((!std::isnan(result.real()) || !std::isinf(result.real())) &&
         "Voigtian::dynamicalFunction() | Result is NaN or Inf!");
  assert((!std::isnan(result.imag()) || !std::isinf(result.imag())) &&
         "Voigtian::dynamicalFunction() | Result is NaN or Inf!");

  return result;
}

std::shared_ptr<ComPWA::FunctionTree>
Voigtian::tree(const ParameterList &sample, int pos, std::string suffix) {

  size_t sampleSize = sample.mDoubleValue(pos)->values().size();

  auto tr = std::make_shared<FunctionTree>(
      "Voigtian" + suffix, MComplex("", sampleSize),
      std::make_shared<VoigtianStrategy>());

  tr->createLeaf("Mass", Mass, "Voigtian" + suffix);
  tr->createLeaf("Width", Width, "Voigtian" + suffix);
  tr->createLeaf("Sigma", Sigma, "Voigtian" + suffix);
  tr->createLeaf("Data_mSq[" + std::to_string(pos) + "]",
                 sample.mDoubleValue(pos), "Voigtian" + suffix);

  return tr;
};

void VoigtianStrategy::execute(ParameterList &paras,
                               std::shared_ptr<Parameter> &out) {
  if (out && checkType != out->type())
    throw BadParameter("VoigtianStrat::execute() | Parameter type mismatch!");

#ifndef NDEBUG
  // Check parameter type
  if (checkType != out->type())
    throw(WrongParType("VoigtianStrat::execute() | "
                       "Output parameter is of type " +
                       std::string(ParNames[out->type()]) +
                       " and conflicts with expected type " +
                       std::string(ParNames[checkType])));

  // How many parameters do we expect?
  size_t check_nInt = 0;
  size_t nInt = paras.intValues().size();
  size_t check_nDouble = 3;
  size_t nDouble = paras.doubleValues().size();
  nDouble += paras.doubleParameters().size();
  size_t check_nComplex = 0;
  size_t nComplex = paras.complexValues().size();
  size_t check_nMInteger = 0;
  size_t nMInteger = paras.mIntValues().size();
  size_t check_nMDouble = 1;
  size_t nMDouble = paras.mDoubleValues().size();
  size_t check_nMComplex = 0;
  size_t nMComplex = paras.mComplexValues().size();

  // Check size of parameter list
  if (nInt != check_nInt)
    throw(BadParameter("VoigtianStrat::execute() | "
                       "Number of IntParameters does not match: " +
                       std::to_string(nInt) + " given but " +
                       std::to_string(check_nInt) + " expected."));
  // I do not want to set sigma as fit parameter. I would like to set it as
  // fixed parameter/argument
  if (nDouble != check_nDouble)
    throw(BadParameter("VoigtianStrat::execute() | "
                       "Number of FitParameters does not match: " +
                       std::to_string(nDouble) + " given but " +
                       std::to_string(check_nDouble) + " expected."));
  if (nComplex != check_nComplex)
    throw(BadParameter("VoigtianStrat::execute() | "
                       "Number of ComplexParameters does not match: " +
                       std::to_string(nComplex) + " given but " +
                       std::to_string(check_nComplex) + " expected."));
  if (nMInteger != check_nMInteger)
    throw(BadParameter("VoigtianStrat::execute() | "
                       "Number of MultiInt does not match: " +
                       std::to_string(nMInteger) + " given but " +
                       std::to_string(check_nMInteger) + " expected."));
  if (nMDouble != check_nMDouble)
    throw(BadParameter("VoigtianStrat::execute() | "
                       "Number of MultiDoubles does not match: " +
                       std::to_string(nMDouble) + " given but " +
                       std::to_string(check_nMDouble) + " expected."));
  if (nMComplex != check_nMComplex)
    throw(BadParameter("VoigtianStrat::execute() | "
                       "Number of MultiComplexes does not match: " +
                       std::to_string(nMComplex) + " given but " +
                       std::to_string(check_nMComplex) + " expected."));
#endif

  size_t n = paras.mDoubleValue(0)->values().size();
  if (!out)
    out = MComplex("", n);
  auto par =
      std::static_pointer_cast<Value<std::vector<std::complex<double>>>>(out);
  auto &results = par->values(); // reference

  // Get parameters from ParameterList:
  // We use the same order of the parameters as was used during tree
  // construction.
  double m0 = paras.doubleParameter(0)->value();
  double Gamma0 = paras.doubleParameter(1)->value();
  double sigma = paras.doubleValue(0)->value();

  // calc function for each point
  for (unsigned int ele = 0; ele < n; ele++) {
    try {
      results.at(ele) = Voigtian::dynamicalFunction(
          paras.mDoubleValue(0)->values().at(ele), m0, Gamma0, sigma);
    } catch (std::exception &ex) {
      LOG(ERROR) << "VoigtianStrategy::execute() | " << ex.what();
      throw(std::runtime_error("VoigtianStrategy::execute() | "
                               "Evaluation of dynamic function failed!"));
    }
  }
}

void Voigtian::parameters(ParameterList &list) {
  AbstractDynamicalFunction::parameters(list);

  // We check of for each parameter if a parameter of the same name exists in
  // list. If so we check if both are equal and set the local parameter to the
  // parameter from the list. In this way we connect parameters that occur on
  // different positions in the amplitude.
  Width = list.addUniqueParameter(Width);
}

void Voigtian::updateParameters(const ParameterList &list) {

  // Try to update width
  std::shared_ptr<FitParameter> width;
  try {
    width = FindParameter(Width->name(), list);
  } catch (std::exception &ex) {
  }
  if (width)
    Width->updateParameter(width);

  return;
}

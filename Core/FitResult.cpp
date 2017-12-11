
// Copyright (c) 2015, 2017 The ComPWA Team.
// This file is part of the ComPWA framework, check
// https://github.com/ComPWA/ComPWA/license.txt for details.

#include "Core/FitResult.hpp"
#include "Core/Logging.hpp"

namespace ComPWA {

void FitResult::WriteText(std::string filename) {
  std::ofstream myfile;
  myfile.open(filename, std::ios::app);
  genOutput(myfile);
  myfile.close();
  return;
};

void FitResult::WriteSimpleText(std::string filename) {
  std::ofstream myfile;
  myfile.open(filename);
  genSimpleOutput(myfile);
  myfile.close();
  return;
};

double FitResult::shiftAngle(double v) {
  double originalVal = v;
  double val = originalVal;
  while (val > M_PI)
    val -= 2 * M_PI;
  while (val < (-1) * M_PI)
    val += 2 * M_PI;
  if (val != originalVal)
    LOG(info) << "shiftAngle(): shifting parameter from " << originalVal
              << " to " << val << "!";
  return val;
}

void FitResult::genSimpleOutput(std::ostream &out) {
  //  for (auto p : finalParameters.doubleValues())
  //    out << p->value() << " ";
  for (auto p : finalParameters.doubleParameters())
    out << p->value() << " " << p->error().first << " ";
  out << "\n";

  return;
}

void FitResult::SetFinalParameters(ParameterList &finPars) {
  finalParameters.DeepCopy(finPars);
}

void FitResult::SetTrueParameters(ParameterList &truePars) {
  trueParameters.DeepCopy(truePars);
}

void FitResult::SetFitFractions(ParameterList &list) {
  _fitFractions.DeepCopy(list);
}

void FitResult::Print(std::string opt) {
  std::stringstream s;
  genOutput(s, opt);
  std::string str = s.str();
  LOG(info) << str;
}

void FitResult::PrintFitParameters(TableFormater *tableResult) {
  bool printTrue = 0, printInitial = 0;
  if (trueParameters.numParameters())
    printTrue = 1;
  if (initialParameters.numParameters())
    printInitial = 1;

  // Column width for parameter with symmetric error
  size_t parErrorWidth = 22;

  // Do we have a parameter with assymetric errors?
  //  for (unsigned int o = 0; o < finalParameters.GetNDouble(); o++)
  for (auto p : finalParameters.doubleParameters()) {
    if (p->errorType() == ErrorType::ASYM)
      parErrorWidth = 33;
  }

  tableResult->addColumn("Nr");
  tableResult->addColumn("Name", 30);
  if (printInitial)
    tableResult->addColumn("Initial Value", parErrorWidth);
  tableResult->addColumn("Final Value", parErrorWidth);
  if (printTrue)
    tableResult->addColumn("True Value", 10);
  if (printTrue)
    tableResult->addColumn("Deviation", 9);
  tableResult->header();

  size_t parameterId = 0;
  for (auto p : finalParameters.doubleParameters()) {
    //    for (unsigned int o = 0; o < finalParameters.GetNDouble(); o++) {
    std::shared_ptr<DoubleParameter> iniPar, truePar;
    std::string name = p->name();

    if (printInitial) {
      try {
        iniPar = FindParameter(p->name(), initialParameters);
      } catch (BadParameter &bad) {
        iniPar.reset();
      }
    }
    if (printTrue) {
      try {
        truePar = FindParameter(p->name(), trueParameters);
      } catch (BadParameter &bad) {
        truePar.reset();
      }
    }

    ErrorType errorType = p->errorType();
    bool isFixed = p->isFixed();

    // Is parameter an angle?
    bool isAngle = 0;
    if (p->name().find("phase") != std::string::npos)
      isAngle = 1;
    // ... then shift the value to the domain [-pi;pi]
    if (isAngle && !isFixed) {
      p->setValue(shiftAngle(p->value()));
      if (printInitial)
        iniPar->setValue(shiftAngle(iniPar->value()));
      if (printTrue)
        truePar->setValue(shiftAngle(truePar->value()));
    }

    // Is parameter a magnitude?
    bool isMag = 0;
    if (p->name().find("mag") != std::string::npos)
      isMag = 1;
    // ... then make sure that it is positive
    if (isMag && !isFixed) {
      p->setValue(std::abs(p->value()));
      if (printInitial && iniPar)
        iniPar->setValue(std::abs(iniPar->value()));
      if (printTrue && truePar)
        truePar->setValue(std::abs(truePar->value()));
    }

    // Print parameter name
    *tableResult << parameterId << p->name();

    // Print initial values
    if (printInitial) {
      if (iniPar) {
        (*tableResult) << *iniPar; // |nr.| name| inital value|
      } else {
        (*tableResult) << " ";
      }
    }

    // Print final value
    if (!isFixed)
      *tableResult << *p; // final value
    else
      *tableResult << " ";

    // Print true values
    if (printTrue) {
      if (truePar) {
        *tableResult << *truePar;
        double pull = (truePar->value() - p->value());
        // Shift pull by 2*pi if that reduces the deviation
        if (isAngle && !isFixed) {
          while (pull < 0 && pull < -M_PI)
            pull += 2 * M_PI;
          while (pull > 0 && pull > M_PI)
            pull -= 2 * M_PI;
        }
        if (p->hasError()) {
          if (errorType == ErrorType::ASYM && pull < 0)
            pull /= p->error().first;
          else if (errorType == ErrorType::ASYM && pull > 0)
            pull /= p->error().second;
          else
            pull /= p->avgError();
        }
        if (!std::isnan(pull))
          (*tableResult) << pull;
        else
          (*tableResult) << " ";
      } else {
        (*tableResult) << " "
                       << " ";
      }
    }
  }
  tableResult->footer();
  parameterId++;
  return;
}

void FitResult::PrintFitFractions(TableFormater *fracTable) {
  LOG(info) << " FitResult::printFitFractions() | "
               "Calculating fit fractions!";

  double sum = 0, sumErrorSq = 0;

  fracTable->Reset();

  // print matrix
  fracTable->addColumn("Fit fractions [%]", 40); // add empty first column
  fracTable->addColumn("Fraction", 15);          // add empty first column
  fracTable->addColumn("Error", 15);             // add empty first column
  fracTable->header();
  for( auto f : _fitFractions.doubleParameters() ){
    std::string resName = f->name();
    *fracTable << resName << f->value();
    try {
      *fracTable << f->avgError(); // assume symmetric errors here
    } catch (std::exception &ex) {
      *fracTable << 0.0;
    }
    sum += f->value();
    sumErrorSq += f->error().first * f->error().first;
  }

  fracTable->delim();
  *fracTable << "Total" << sum << sqrt(sumErrorSq);
  fracTable->footer();
  sumFractions = sum;
  sumFractionsError = sqrt(sumErrorSq);

  return;
}

} // namespace

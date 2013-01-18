//! Simple \f$\chi^{2}\f$-Estimator.
/*! \class EIFChiOneD
 * @file EIFChiOneD.hpp
 * This class calculates a simple \f$\chi^{2}\f$ of a intensity and a dataset.
 * Data and Model are provided in the constructor using the Amplitude and Data
 * interfaces. The class itself fulfills the Estimator interface.
*/

#ifndef _EIFChiOneD_HPP
#define _EIFChiOneD_HPP

#include <vector>
#include <memory>
#include <string>

//PWA-Headers
#include "Estimator.hpp"
#include "Amplitude.hpp"
#include "Data.hpp"
#include "PWAEvent.hpp"
#include "PWAParameter.hpp"

class EIFChiOneD : public Estimator {

public:
  /// Default Constructor (0x0)
  EIFChiOneD(std::shared_ptr<Amplitude>, std::shared_ptr<Data>);

  virtual double controlParameter(std::vector<std::shared_ptr<PWAParameter> >& minPar);

  /** Destructor */
  virtual ~EIFChiOneD();

protected:

private:
  std::shared_ptr<Amplitude> pPIF_;
  std::shared_ptr<Data> pDIF_;

};

#endif /* _EIFChiOneD_HPP */

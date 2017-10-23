#include "Examples/PythonDalitzFit/PythonFit.hpp"
#include "Tools/RunManager.hpp"
#include "Tools/ParameterTools.hpp"
#include "Physics/ParticleList.hpp"
#include "Physics/HelicityFormalism/HelicityKinematics.hpp"
#include "Physics/HelicityFormalism/IncoherentIntensity.hpp"
#include "Tools/RootGenerator.hpp"
#include "Core/Generator.hpp"
#include "Core/Event.hpp"
#include "Core/Particle.hpp"
#include "Core/Kinematics.hpp"
#include "Estimator/MinLogLH/MinLogLH.hpp"
#include "Optimizer/Minuit2/MinuitIF.hpp"
//#include <boost/python.hpp>

#include <pybind11/iostream.h>
#include <pybind11/pybind11.h>
#include <pybind11/stl.h>
#include <pybind11/stl_bind.h>

namespace py = pybind11;
//using namespace ComPWA;
using ComPWA::Physics::HelicityFormalism::HelicityKinematics;
using ComPWA::Physics::HelicityFormalism::IncoherentIntensity;
using ComPWA::Optimizer::Minuit2::MinuitResult;

PYBIND11_MAKE_OPAQUE(ComPWA::PartList);

//BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(X_returnsame_overloads, X::returnsame, 0, 1)
//BOOST_PYTHON_FUNCTION_OVERLOADS(X_returnsum_overloads, X::returnsum, 1, 2)

std::shared_ptr<ComPWA::AmpIntensity> createIntens(
		std::string modelStr,
		std::shared_ptr<ComPWA::PartList> partL,
		std::shared_ptr<ComPWA::Kinematics> kin,
		std::shared_ptr<ComPWA::Data> phspSample){
  std::stringstream modelStream;
  modelStream << modelStr;
  boost::property_tree::ptree modelTree;
  boost::property_tree::xml_parser::read_xml(modelStream, modelTree);
  auto intens = IncoherentIntensity::Factory(
    partL, kin, modelTree.get_child("IncoherentIntensity"));
  auto phspPoints =
      std::make_shared<std::vector<ComPWA::dataPoint>>(phspSample->GetDataPoints(kin));
  intens->SetPhspSample(phspPoints, phspPoints);
  return intens;
}

std::vector<std::pair<std::string, std::string>> fitComponents(){
  std::vector<std::pair<std::string, std::string>> components;
  components.push_back(
    std::pair<std::string, std::string>("myAmp", "jpsiGammaPiPi"));
  components.push_back(
    std::pair<std::string, std::string>("f2(1270)", "jpsiGammaPiPi"));
  return components;
}

void saveResults(std::string file, std::shared_ptr<ComPWA::FitResult> result){
  std::ofstream ofs(file);
  boost::archive::xml_oarchive oa(ofs);
  std::shared_ptr<MinuitResult> resultM = std::dynamic_pointer_cast<MinuitResult>(result);
  oa << BOOST_SERIALIZATION_NVP(resultM);
}

void saveModel(std::string file, std::shared_ptr<ComPWA::PartList> partL, ComPWA::ParameterList& fitPar, std::shared_ptr<ComPWA::AmpIntensity> intens){
  ComPWA::UpdateParticleList(partL, fitPar);
  boost::property_tree::ptree ptout;
  ptout.add_child("ParticleList", ComPWA::SaveParticles(partL));
  std::shared_ptr<IncoherentIntensity> intensI = std::dynamic_pointer_cast<IncoherentIntensity>(intens);
  ptout.add_child("IncoherentIntensity", IncoherentIntensity::Save(intensI));
  boost::property_tree::xml_parser::write_xml(file, ptout, std::locale());
}
 
//BOOST_PYTHON_MODULE(Dalitz_ext)
PYBIND11_MODULE(Dalitz_ext, m)
{
//    using namespace boost::python;
	m.doc() = "pybind11 DalitzFit plugin"; // optional module docstring

    //class_<PythonFit>("PythonFit")
	py::class_<PythonFit>(m, "PythonFit")
      .def(py::init<>())
      .def("StartFit", &PythonFit::StartFit)
	  .def("setConfigFile", &PythonFit::setConfigFile)
	  //.def("testTree", &PythonFit::testTree, return_value_policy<manage_new_object>())
	  //.def("testTree", &PythonFit::testTree)
	  //.def("testVec", &PythonFit::testTVector3)
	  .def("useGeneva", &PythonFit::useGeneva)
      //.def("AddParameter", &ParameterList::AddParameter)
      //.def("to_str", &ParameterList::to_str)
      //.def("GetNDouble", &ParameterList::GetNDouble)
    ;

	// Global Functions
	m.def("ReadParticles", (void (*) (std::shared_ptr<ComPWA::PartList>, std::string)) &ComPWA::ReadParticles);
	m.def("GetDefaultParticles", []() { return defaultParticleList; } );
	m.def("GetInitialState", [](int id) { std::vector<ComPWA::pid> initialState = {id}; return initialState; } );
	m.def("GetFinalState", [](int idA, int idB, int idC) { std::vector<ComPWA::pid> finalState = {idA, idB, idC}; return finalState; } );
	m.def("GetIncoherentIntensity",  (std::shared_ptr<ComPWA::AmpIntensity>  (*)
			(std::string, std::shared_ptr<ComPWA::PartList>, std::shared_ptr<ComPWA::Kinematics>, std::shared_ptr<ComPWA::Data>))
			&createIntens);
	m.def("setErrorOnParameterList", (void (*) (ComPWA::ParameterList&, double, bool)) &setErrorOnParameterList);
	m.def("fitComponents",  (std::vector< std::pair<std::string, std::string> > (*) ()) &fitComponents);
	m.def("saveResults",  (void (*) (std::string, std::shared_ptr<ComPWA::FitResult>)) &saveResults);
	m.def("saveModel",  (void (*) (std::string, std::shared_ptr<ComPWA::PartList>, ComPWA::ParameterList&, std::shared_ptr<ComPWA::AmpIntensity>)) &saveModel);
	//m.def("CalculateFitFractions", (ComPWA::ParameterList (*)
	//        (std::shared_ptr<ComPWA::Kinematics>, std::shared_ptr<ComPWA::AmpIntensity>,
	//          std::shared_ptr<std::vector<ComPWA::dataPoint> >, std::vector<std::pair<std::string, std::string> > ))
	//		&ComPWA::Tools::CalculateFitFractions);
	//m.def("CalcFractionError", (void (*)
	//        (ParameterList&, std::vector<std::vector<double> >, ParameterList&, std::shared_ptr<AmpIntensity>,
	//          std::shared_ptr<std::vector<ComPWA::dataPoint> >, int, std::vector<std::pair<std::string, std::string> > ))
	//		&ComPWA::Tools::CalcFractionError);

	// ComPWA Interfaces
	py::class_<ComPWA::Kinematics, std::shared_ptr<ComPWA::Kinematics> >(m, "Kinematics");
	py::class_<ComPWA::Generator, std::shared_ptr<ComPWA::Generator> >(m, "Generator");
	py::class_<ComPWA::IEstimator, std::shared_ptr<ComPWA::IEstimator> >(m, "IEstimator");
	py::class_<ComPWA::Optimizer::Optimizer, std::shared_ptr<ComPWA::Optimizer::Optimizer> >(m, "Optimizer");
	py::class_<ComPWA::FitResult, std::shared_ptr<ComPWA::FitResult> >(m, "FitResult");

	// ComPWA Classes
	py::class_<ComPWA::AmpIntensity, std::shared_ptr<ComPWA::AmpIntensity> >(m, "AmpIntensity")
	  .def("GetParameters", &ComPWA::AmpIntensity::GetParameters)
	;

	py::class_<ComPWA::DataReader::Data, std::shared_ptr<ComPWA::DataReader::Data> >(m, "Data")
      .def(py::init<>())
	;

	py::class_<ComPWA::dataPoint>(m, "dataPoint")
      .def(py::init<>())
	;

	py::class_<ComPWA::PartList, std::shared_ptr<ComPWA::PartList> >(m, "PartList")
      .def(py::init<>())
	;

	py::class_<ComPWA::ParameterList>(m, "ParameterList")
      .def(py::init<>())
	;

	py::class_<HelicityKinematics, ComPWA::Kinematics, std::shared_ptr<HelicityKinematics> >(m, "HelicityKinematics")
      .def(py::init<std::shared_ptr<ComPWA::PartList>, std::vector<ComPWA::pid>, std::vector<ComPWA::pid> >())
	;

	py::class_<ComPWA::RunManager>(m, "RunManager")
      .def(py::init<>())
      .def("SetGenerator", &ComPWA::RunManager::SetGenerator)
	  .def("SetPhspSample", &ComPWA::RunManager::SetPhspSample, py::arg("phsp"),
			  py::arg("truePhsp") = std::shared_ptr<ComPWA::DataReader::Data>() )
	  .def("GeneratePhsp", &ComPWA::RunManager::GeneratePhsp)
	  .def("SetData", &ComPWA::RunManager::SetData)
	  .def("Generate", &ComPWA::RunManager::Generate)
	  .def("SetAmplitude", &ComPWA::RunManager::SetAmplitude)
	  .def("SetOptimizer", &ComPWA::RunManager::SetOptimizer)
	  .def("Fit", &ComPWA::RunManager::Fit)
    ;

	py::class_<ComPWA::Tools::RootGenerator, ComPWA::Generator, std::shared_ptr<ComPWA::Tools::RootGenerator> >(m, "RootGenerator")
      .def(py::init<std::shared_ptr<ComPWA::PartList>, std::shared_ptr<ComPWA::Kinematics> >())
	;

	py::class_<ComPWA::Estimator::MinLogLH, ComPWA::IEstimator, std::shared_ptr<ComPWA::Estimator::MinLogLH> >(m, "MinLogLH")
      .def(py::init<std::shared_ptr<ComPWA::Kinematics>, std::shared_ptr<ComPWA::AmpIntensity>,
    		  std::shared_ptr<ComPWA::DataReader::Data>, std::shared_ptr<ComPWA::DataReader::Data>,
			  std::shared_ptr<ComPWA::DataReader::Data>, unsigned int, unsigned int >())
	  .def("UseFunctionTree", &ComPWA::Estimator::MinLogLH::UseFunctionTree)
	;

	py::class_<ComPWA::Optimizer::Minuit2::MinuitIF, ComPWA::Optimizer::Optimizer, std::shared_ptr<ComPWA::Optimizer::Minuit2::MinuitIF> >(m, "MinuitIF")
      .def(py::init<std::shared_ptr<ComPWA::IEstimator>, ComPWA::ParameterList&>())
	  .def("SetHesse", &ComPWA::Optimizer::Minuit2::MinuitIF::SetHesse)
	;

	py::class_<ComPWA::Optimizer::Minuit2::MinuitResult, ComPWA::FitResult, std::shared_ptr<ComPWA::Optimizer::Minuit2::MinuitResult> >(m, "MinuitResult")
	  .def("Print", &ComPWA::Optimizer::Minuit2::MinuitResult::Print, py::arg("opt") = "")
	;

}


/*USAGE
 * export PYTHONPATH=$PYTHONPATH:YOUR_COMPWA_BUILD_DIR/lib
 * python3
 * >>> from Dalitz_ext import *
 */

//! Application to generate J/Psi -> g pi pi.
/*!
 * @file GenDalitzApp.cpp
 * This application uses the Breit-Wigner-Amplitude-Sum module and a
 * phase-space generator to generate a file with J/Psi -> g pi pi events.
*/

// Standard header files go here
#include <iostream>
#include <cmath>
#include <sstream>
#include <vector>
#include <string>
#include <memory>

// Boost header files go here
#include <boost/foreach.hpp>
#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/xml_parser.hpp>

// Root header files go here
#include "TMath.h"
#include "TLorentzVector.h"
#include "TParticle.h"
#include "TGenPhaseSpace.h"
#include "TFile.h"
#include "TROOT.h"
#include "TClonesArray.h"
#include "TTree.h"
#include "TRandom3.h"

// Physics Interface header files go here
#include "Physics/AmplitudeSum/AmpSumIntensity.hpp"
#include "Physics/AmplitudeSum/AmplitudeSetup.hpp"
#include "Core/Parameter.hpp"
#include "Core/ParameterList.hpp"

using namespace std;

const unsigned int MaxEvents = 100000;

//constants
const Double_t M = 3.096916; // GeV/c² (J/psi+)
const Double_t Br = 0.000093; // GeV/c² (width)
const Double_t m1 = 0.; // GeV/c² (gamma)
const Double_t m2 = 0.139570; // GeV/c² (pi)
const Double_t m3 = 0.139570; // GeV/c² (pi)
//const Double_t c = 299792458.; // m/s
const Double_t PI = 3.14159; // m/s

/************************************************************************************************/
/**
 * The main function.
 */
int main(int argc, char **argv){
  unsigned int i=0, mc=0;
  TRandom3 rando;

  //load resonances
  AmplitudeSetup ini("test/JPSI_ypipi.xml");
  cout << "loaded file " << ini.getFileName() << " with " << ini.getResonances().size() << " resonances:" << endl;
  for(std::vector<Resonance>::iterator reso=ini.getResonances().begin(); reso!=ini.getResonances().end(); reso++){
    cout << endl << "Resonance " << (*reso).m_name << endl;
    cout << "Mass =  " << (*reso).m_mass << " with range " << (*reso).m_mass_min << " to " << (*reso).m_mass_max << endl;
    cout << "Width = " << (*reso).m_width << " with range " << (*reso).m_width_min << " to " << (*reso).m_width_max << endl;
    cout << "Spin =  " << (*reso).m_spin << " m = " << (*reso).m_m << " n = " << (*reso).m_n << endl;
    cout << "Strength =  " << (*reso).m_strength << " Phase = " << (*reso).m_phase << endl;
    cout << "Breakupmomentum =  " << (*reso).m_breakup_mom << endl;
    cout << "DaughterA =  " << (*reso).m_daugtherA << " DaughterB = " << (*reso).m_daugtherB << endl;
  }
  cout << endl << endl;

  //Simple Breit-Wigner Physics-Module setup
  AmpSumIntensity testBW(M, Br, m1, m2, m3, ini);
  ParameterList minPar;
  testBW.fillStartParVec(minPar);
 // minPar.AddParameter(DoubleParameter(1.5,0.5,2.5,0.1));

  //Output File setup
  TFile output("test/3Part-4vecs.root","recreate");
  output.SetCompressionLevel(1); //try level 2 also

  TTree fTree ("data","Dalitz-Gen");
  TClonesArray *fEvt = new TClonesArray("TParticle");
  //TClonesArray &ar = *fEvt;
  fTree.Branch("Particles",&fEvt);

  TTree fTreePHSP ("mc","Dalitz-Gen-PHSP");
  TClonesArray *fEvtPHSP = new TClonesArray("TParticle");
  //TClonesArray &ar = *fEvt;
  fTreePHSP.Branch("Particles",&fEvtPHSP);

  //Generation
  TLorentzVector W(0.0, 0.0, 0.0, M);//= beam + target;

  //(Momentum, Energy units are Gev/C, GeV)
  Double_t masses[3] = { m1, m2, m2} ;

  TGenPhaseSpace event;
  event.SetDecay(W, 3, masses);

  TLorentzVector *pGamma,*pPip,*pPim,pPm23,pPm13;
  double weight, m23sq, m13sq, m12sq, maxTest=0;
  cout << "Start generation of y pi0 pi0 Dalitz" << endl;
  do{
      weight = event.Generate();

      pGamma = event.GetDecay(0);
      pPip    = event.GetDecay(1);
      pPim    = event.GetDecay(2);

      pPm23 = *pPim + *pPip;
      pPm13 = *pGamma + *pPim;

      m23sq=pPm23.M2(); m13sq=pPm13.M2();

      m12sq=M*M+m1*m1+m2*m2+m3*m3-m13sq-m23sq;
      if(m12sq<0){
        //cout << tmpm12_sq << "\t" << M*M << "\t" << m13_sq << "\t" << m23_sq << endl;
        //continue;
        m12sq=0.0001;
      }

      TParticle fparticleGam(22,1,0,0,0,0,*pGamma,W);
      TParticle fparticlePip(211,1,0,0,0,0,*pPip,W);
      TParticle fparticlePim(-211,1,0,0,0,0,*pPim,W);

      //call physics module
      vector<double> x;
      x.push_back(sqrt(m23sq));
      x.push_back(sqrt(m13sq));
      x.push_back(sqrt(m12sq));
      ParameterList intensL = testBW.intensity(x, minPar);
      double AMPpdf = intensL.GetDoubleParameter(0)->GetValue();
      //double AMPpdf = testBW.intensity(x, minPar);

      double test = rando.Uniform(0,5);

      //mb.setVal(m13);
      //double m13pdf = totAmp13.getVal();//fun_combi2->Eval(m13);
      if(maxTest<(weight*AMPpdf))
        maxTest=(weight*AMPpdf);
      if(i<MaxEvents && test<(weight*AMPpdf)){
        i++;
        new((*fEvt)[0]) TParticle(fparticleGam);
        new((*fEvt)[1]) TParticle(fparticlePip);
        new((*fEvt)[2]) TParticle(fparticlePim);

        fTree.Fill();
      }

      if(mc<MaxEvents && test<weight){
        mc++;
        new((*fEvtPHSP)[0]) TParticle(fparticleGam);
        new((*fEvtPHSP)[1]) TParticle(fparticlePip);
        new((*fEvtPHSP)[2]) TParticle(fparticlePim);

        fTreePHSP.Fill();
      }
  }while(i<MaxEvents || mc<MaxEvents);

  fTree.Print();
  fTree.Write();
  fTreePHSP.Write();
  output.Close();

  cout << "Done ... " << maxTest << endl << endl;

  return 0;
}

//-------------------------------------------------------------------------------
// Copyright (c) 2013 Mathias Michel.
// All rights reserved. This program and the accompanying materials
// are made available under the terms of the GNU Public License v3.0
// which accompanies this distribution, and is available at
// http://www.gnu.org/licenses/gpl.html
//
// Contributors:
//     Mathias Michel - initial API and implementation
//     Peter Weidenkaff -  assignment of final state particle masses
//-------------------------------------------------------------------------------
//****************************************************************************
// Class for defining the relativistic Breit-Wigner resonance model, which
// includes the use of Blatt-Weisskopf barrier factors.
//****************************************************************************

// --CLASS DESCRIPTION [MODEL] --
// Class for defining the relativistic Breit-Wigner resonance model, which
// includes the use of Blatt-Weisskopf barrier factors.

#include <cmath>
#include "Physics/AmplitudeSum/AmpWigner.hpp"

#include "qft++.h"

AmpWigner::AmpWigner()
{
	toEvaluate=false;
}
AmpWigner::AmpWigner(const char *name,
		unsigned int spin, unsigned int m,  unsigned int n,unsigned int subSys) :
												  _inSpin(spin),
												  _outSpin1(m),
												  _outSpin2(n),
												  _subSys(subSys)
{
	toEvaluate=true;
	initialise();
}

AmpWigner::AmpWigner(const AmpWigner& other, const char* newname) :
												  _inSpin(other._inSpin),
												  _outSpin1(other._outSpin1),
												  _outSpin2(other._outSpin2),
												  _subSys(other._subSys),
												  toEvaluate(other.toEvaluate)
{
	initialise();
}

AmpWigner::~AmpWigner() 
{
}

void AmpWigner::initialise() 
{
	DalitzKinematics* kin = dynamic_cast<DalitzKinematics*>(Kinematics::instance());
	_M=kin->M;
	//if(_subSys==5){
	_m1=kin->m1;
	_m2=kin->m2;
	_m3=kin->m3;//}
	//if(_subSys==4){
	//_m1=point->DPKin.m2;
	//_m2=point->DPKin.m3;
	//_m3=point->DPKin.m1;//}
	//if(_subSys==3){
	//_m1=point->DPKin.m3;
	//_m2=point->DPKin.m2;
	//_m3=point->DPKin.m1;//}
	//	cout<<"AmpWigner DEBUG set masses to m1="<<_m1<< " m2="<<_m2<<" m3=" <<_m3<<endl;

}    
void AmpWigner::setDecayMasses(double m1, double m2, double m3, double M){
	_M=M; _m1=m1; _m2=m2; _m3=m3;
	return;
}

double AmpWigner::evaluate(dataPoint& point) const {
	if(!toEvaluate) return 1.;
	DalitzKinematics* kin = dynamic_cast<DalitzKinematics*>(Kinematics::instance());

	double locmin_sq, locmax_sq, beta;

	double m23sq = point.getVal("m23sq");
	double m13sq = point.getVal("m13sq");
	double m12sq = kin->getThirdVariableSq(m23sq,m13sq);
	double invM1 = -999;
	double invM2 = -999;
	switch(_subSys){
	case 3: {
		invM1=sqrt(m12sq);
		invM2=sqrt(m23sq);
		locmin_sq = kin->invMassMin(5,_subSys,invM1*invM1);
		locmax_sq = kin->invMassMax(5,_subSys,invM1*invM1);
		break;
	}
	case 4: {
		invM1=sqrt(m13sq);
		invM2=sqrt(m12sq);
		locmin_sq = kin->invMassMin(5,_subSys,invM1*invM1);
		locmax_sq = kin->invMassMax(5,_subSys,invM1*invM1);
		break;
	}
	case 5: {
		invM1=sqrt(m23sq);
		invM2=sqrt(m13sq);
		locmin_sq = kin->invMassMin(4,_subSys,invM1*invM1);
		locmax_sq = kin->invMassMax(4,_subSys,invM1*invM1);
		break;
	}
	}

	//	switch(_subSys){
	//	case 5:{ //reso in m23
	//		locmin_sq = s2min(invM1*invM1,_M,_m1,_m2,_m3);
	//		locmax_sq = s2max(invM1*invM1,_M,_m1,_m2,_m3);
	//		break;
	//	}
	//	case 4:{ //reso in m13
	//		locmin_sq = s1min(invM1*invM1,_M,_m1,_m2,_m3);
	//		locmax_sq = s1max(invM1*invM1,_M,_m1,_m2,_m3);
	//		break;
	//	}
	//	case 3:{ //reso in m12
	//		//return 1;
	//		locmin_sq = s1min(invM1*invM1,_M,_m1,_m3,_m2);
	//		locmax_sq = s1max(invM1*invM1,_M,_m1,_m3,_m2);
	//		//if(beta!=beta) return 1.;
	//		break;
	//	}
	//	}
	double cosbeta = (2.*invM2*invM2-locmax_sq-locmin_sq)/(locmax_sq-locmin_sq);
	beta=acos(cosbeta);

	Spin j((double) _inSpin), m((double) _outSpin1), n((double)_outSpin2);
	double result = Wigner_d(j,m,n,beta);
	if( ( result!=result ) || (beta!=beta)) {
		std::cout<<"= AmpWigner: "<<std::endl;
		std::cout<< "NAN! J="<< _inSpin<<" M="<<_outSpin1<<" N="<<_outSpin2<<" beta="<<beta<<std::endl;
		std::cout<< "subSys: "<<_subSys<<" invM1sq="<<invM1*invM1 << " invM2sq=" <<invM2*invM2<< " cos(beta)="<<cosbeta<<std::endl;
		return 0;
	}
//	std::cout<<"===================================="<<std::endl;
	return result;
}


double AmpWigner::lambda(double x, double y, double z)const{
	return x*x+y*y+z*z-2.*x*y-2.*x*z-2.*y*z;
}

double AmpWigner::s2min(double s1, double m0, double m1, double m2, double m3)const
{
	double s      = m0*m0;
	double lamterm = sqrt( lambda(s1,s,m1*m1) ) * sqrt( lambda(s1, m2*m2, m3*m3) );

	double result  = m1*m1 + m3*m3 + ( (s-s1-m1*m1)*(s1-m2*m2+m3*m3) - lamterm )/(2.*s1);

	return result;
}

double AmpWigner::s2max(double s1, double m0, double m1, double m2, double m3)const
{
	double s      = m0*m0;
	double lamterm = sqrt( lambda(s1,s,m1*m1) ) * sqrt( lambda(s1, m2*m2, m3*m3) );

	double result  = m1*m1 + m3*m3 + ( (s-s1-m1*m1)*(s1-m2*m2+m3*m3) + lamterm )/(2.*s1);

	return result;
}

double AmpWigner::s3min(double s1, double m0, double m1, double m2, double m3)const
{
	double s      = m0*m0;
	double lamterm = sqrt( lambda(s1,s,m1*m1) ) * sqrt( lambda(s1, m3*m3, m1*m1) );

	double result  = m1*m1 + m2*m2 + ( (s-s1-m1*m1)*(s1-m1*m1+m2*m2) - lamterm )/(2.*s1);

	return result;
}

double AmpWigner::s3max(double s1, double m0, double m1, double m2, double m3)const
{
	double s      = m0*m0;
	double lamterm = sqrt( lambda(s1,s,m1*m1) ) * sqrt( lambda(s1, m3*m3, m1*m1) );

	double result  = m1*m1 + m2*m2 + ( (s-s1-m1*m1)*(s1-m1*m1+m3*m3) + lamterm )/(2.*s1);

	return result;
}

double AmpWigner::s1min(double s2, double m0, double m1, double m2, double m3)const
{
	double s      = m0*m0;
	double lamterm = sqrt( lambda(s2,s,m2*m2) ) * sqrt( lambda(s2, m3*m3, m1*m1) );

	double result  = m2*m2 + m3*m3 + ( (s-s2-m2*m2)*(s2-m1*m1+m3*m3) - lamterm )/(2.*s2);

	return result;
}

double AmpWigner::s1max(double s2, double m0, double m1, double m2, double m3)const
{
	double s      = m0*m0;
	double lamterm = sqrt( lambda(s2,s,m2*m2) ) * sqrt( lambda(s2, m1*m1, m3*m3) );

	double result  = m2*m2 + m3*m3 + ( (s-s2-m2*m2)*(s2-m1*m1+m3*m3) + lamterm )/(2.*s2);

	return result;
}
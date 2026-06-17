/**
BSD-3-Clause
Copyright 2019 Alliance for Sustainable Energy, LLC
Redistribution and use in source and binary forms, with or without modification, are permitted provided 
that the following conditions are met :
1.	Redistributions of source code must retain the above copyright notice, this list of conditions 
and the following disclaimer.
2.	Redistributions in binary form must reproduce the above copyright notice, this list of conditions 
and the following disclaimer in the documentation and/or other materials provided with the distribution.
3.	Neither the name of the copyright holder nor the names of its contributors may be used to endorse 
or promote products derived from this software without specific prior written permission.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, 
INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE 
ARE DISCLAIMED.IN NO EVENT SHALL THE COPYRIGHT HOLDER, CONTRIBUTORS, UNITED STATES GOVERNMENT OR UNITED STATES 
DEPARTMENT OF ENERGY, NOR ANY OF THEIR EMPLOYEES, BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, 
OR CONSEQUENTIAL DAMAGES(INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, 
WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT(INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT 
OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*/
#ifndef __lib_ondinv_h
#define __lib_ondinv_h

#include <string>
#include <vector>
//#include "mlm_spline.h" // spline interpolator for efficiency curves
#include "bspline.h"
using namespace std;
using namespace SPLINTER;

class ond_inverter
{
public:
	ond_inverter();
	virtual ~ond_inverter() {};


	double PNomConv; // [W]
	double PMaxOUT; // [W]
	double VOutConv; // [W]
	double VMppMin; // [V]
	double VMPPMax; // [V]
	double VAbsMax; // [V]
	double PSeuil; // [W]
	string ModeOper; // [-]
	string CompPMax; // [-]
	string CompVMax; // [-]
	string ModeAffEnum; // [-]
	double PNomDC; // [W]
	double PMaxDC; // [W]
	double IMaxDC; // [A]
	double INomDC; // [A]
	double INomAC; // [A]
	double IMaxAC; // [A]
	double TPNom; // [°C]
	double TPMax; // [°C]
	double TPLim1; // [°C]
	double TPLimAbs; // [°C]
	double PLim1; // [kW]
	double PLimAbs; // [kW]
	double VNomEff[3]; // [V]
	int NbInputs; // [-]
	int NbMPPT; // [-]
	double Aux_Loss; // [W]
	double Night_Loss; // [W]
	double lossRDc; // [V/A]
	double lossRAc; // [A]
	int effCurve_elements; // [-]
	double effCurve_Pdc[3][100]; // [W]
	double effCurve_Pac[3][100]; // [W]
	double effCurve_eta[3][100]; // [-]
	int doAllowOverpower; // [-] // ADDED TO CONSIDER MAX POWER USAGE [2018-06-23, TR]
	int doUseTemperatureLimit; // [-] // ADDED TO CONSIDER TEMPERATURE LIMIT USAGE [2018-06-23, TR]

	bool acpower(	
		/* inputs */
		double Pdc,			/* Input power to inverter (Wdc) */
		double Vdc,			/* Voltage input to inverter (Vdc) */
		double Tamb,		/* Ambient temperature (°C) */

		/* outputs */
		double *Pac,		/* AC output power (Wac) */
		double *Ppar,		/* AC parasitic power consumption (Wac) */
		double *Plr,		/* Part load ratio (Pdc_in/Pdc_rated, 0..1) */
		double *Eff,		/* Conversion efficiency (0..1) */
		double *Pcliploss,	/* Power loss due to clipping loss (Wac) */
		double *Psoloss,	/* Power loss due to operating power consumption (Wdc) */
		double *Pntloss,	/* Power loss due to night time tare loss (Wac) */
		double *dcloss,		/* DC power loss (Wdc) */
		double *acloss		/* AC power loss (Wac) */
	);
	double calcEfficiency(
		double Pdc,
		int index_eta
	);
	double tempDerateAC(
		double arrayT[],
		double arrayPAC[],
		double T
	);
	virtual void initializeManual();

private:
	bool ondIsInitialized;

	int noOfEfficiencyCurves;
//	tk::spline effSpline[2][3];
//	BSpline m_bspline3[2][3];
	BSpline m_bspline3[3];
	double x_max[3];
	double x_lim[3];
	double Pdc_threshold;
	double a[3];
	double b[3];

	double PNomDC_eff;
	double PMaxDC_eff;
	double INomDC_eff;
	double IMaxDC_eff;
	double T_array[6];
	double PAC_array[6];



};

#endif

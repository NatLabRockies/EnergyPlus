/*******************************************************************************************************
*  Copyright 2018 - pvyield GmbH / Timo Richert
*  Copyright 2017 Alliance for Sustainable Energy, LLC
*
*  NOTICE: This software was developed at least in part by Alliance for Sustainable Energy, LLC
*  (“Alliance”) under Contract No. DE-AC36-08GO28308 with the U.S. Department of Energy and the U.S.
*  The Government retains for itself and others acting on its behalf a nonexclusive, paid-up,
*  irrevocable worldwide license in the software to reproduce, prepare derivative works, distribute
*  copies to the public, perform publicly and display publicly, and to permit others to do so.
*
*  Redistribution and use in source and binary forms, with or without modification, are permitted
*  provided that the following conditions are met:
*
*  1. Redistributions of source code must retain the above copyright notice, the above government
*  rights notice, this list of conditions and the following disclaimer.
*
*  2. Redistributions in binary form must reproduce the above copyright notice, the above government
*  rights notice, this list of conditions and the following disclaimer in the documentation and/or
*  other materials provided with the distribution.
*
*  3. The entire corresponding source code of any redistribution, with or without modification, by a
*  research entity, including but not limited to any contracting manager/operator of a United States
*  National Laboratory, any institution of higher learning, and any non-profit organization, must be
*  made publicly available under this license for as long as the redistribution is made available by
*  the research entity.
*
*  4. Redistribution of this software, without modification, must refer to the software by the same
*  designation. Redistribution of a modified version of this software (i) may not refer to the modified
*  version by the same designation, or by any confusingly similar designation, and (ii) must refer to
*  the underlying software originally provided by Alliance as “System Advisor Model” or “SAM”. Except
*  to comply with the foregoing, the terms “System Advisor Model”, “SAM”, or any confusingly similar
*  designation may not be used to refer to any modified version of this software or any modified
*  version of the underlying software originally provided by Alliance without the prior written consent
*  of Alliance.
*
*  5. The name of the copyright holder, contributors, the United States Government, the United States
*  Department of Energy, or any of their employees may not be used to endorse or promote products
*  derived from this software without specific prior written permission.
*
*  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
*  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND
*  FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER,
*  CONTRIBUTORS, UNITED STATES GOVERNMENT OR UNITED STATES DEPARTMENT OF ENERGY, NOR ANY OF THEIR
*  EMPLOYEES, BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
*  DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
*  DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER
*  IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
*  THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
*******************************************************************************************************/

/*******************************************************************************************************
* Implementation of an inverter model based on OND files
*******************************************************************************************************/

#include <math.h>
#include <cmath>
#include <limits>
#include <vector>
#include <stdexcept>

#include "lib_ondinv.h"
#include "bsplinebuilder.h"
#include "datatable.h"


const int TEMP_DERATE_ARRAY_LENGTH = 6;
// test commit

ond_inverter::ond_inverter()
{
	PNomConv = PMaxOUT = VOutConv = VMppMin = VMPPMax = VAbsMax = PSeuil = PNomDC = PMaxDC =
		IMaxDC = INomDC = INomAC = IMaxAC = TPNom = TPMax = TPLim1 = TPLimAbs = PLim1 = PLimAbs = Aux_Loss =
		Night_Loss = lossRDc = lossRAc = std::numeric_limits<double>::quiet_NaN();
	ModeOper = CompPMax = CompVMax = ModeAffEnum = "";
	NbInputs = NbMPPT = 0;
	ondIsInitialized = false;
	doAllowOverpower = doUseTemperatureLimit = true;
}

// Initialize - Calculates values that only need calculation once
void ond_inverter::initializeManual()
{
	if (!ondIsInitialized)
	{
		// Check inverter configuration
		if (ModeOper != "MPPT") {
			throw std::invalid_argument("Invalid ModeOper, only 'MPPT' is supported.");
		}
		if (CompPMax != "Lim") {
			throw std::invalid_argument("Invalid CompPMax, only 'Lim' is supported.");
		}
		if (CompVMax != "Lim") {
			throw std::invalid_argument("Invalid CompVMax, only 'Lim' is supported.");
		}
		if (ModeAffEnum != "Efficiencyf_PIn") {
			throw std::invalid_argument("Invalid ModeAffEnum, only 'Efficiencyf_PIn' is supported.");
		}

		// Set up temperature limit array
		double PlimAbs_eff, PLim1_eff, PMaxOUT_eff;
		if (PLimAbs < 0.001 * PNomConv) {
			PlimAbs_eff = 0;
		}
		else {
			PlimAbs_eff = PLimAbs;
		}
		if (PLim1 < 0.001 * PNomConv) {
			PLim1_eff = 0;
		}
		else {
			PLim1_eff = PLim1;
		}
		if (PMaxOUT < 0.001 * PNomConv) {
			PMaxOUT_eff = PNomConv;
		}
		else {
			PMaxOUT_eff = PMaxOUT;
		}
		double T_array_init[] = { -300, TPMax, TPNom, TPLim1, TPLimAbs, TPLimAbs };
		double PAC_array_init[] = { PMaxOUT_eff, PMaxOUT_eff, PNomConv, PLim1_eff, PlimAbs_eff, 0 };
		for (int j = 0; j <= TEMP_DERATE_ARRAY_LENGTH - 1; j = j + 1) {
			T_array[j] = T_array_init[j];
			PAC_array[j] = PAC_array_init[j];
		}

		// Convert P_AC efficiency curve to P_DC

		// Set missing DC values if missing from OND file
		if (PNomDC < 0.0001 * PNomConv) {
			PNomDC_eff = PNomConv;
		}
		else {
			PNomDC_eff = PNomDC;
		}
		if (PMaxDC < 0.0001 * PMaxOUT) {
			PMaxDC_eff = PMaxOUT;
		}
		else {
			PMaxDC_eff = PMaxDC;
		}
		if (INomDC < 0.0001 * (PNomConv / VMPPMax)) {
			INomDC_eff = PNomDC_eff / VMppMin;
		}
		else {
			INomDC_eff = INomDC;
		}
		if (IMaxDC < 0.0001 * (PNomConv / VMPPMax)) {
			IMaxDC_eff = INomDC_eff * (PMaxDC_eff / PNomDC_eff);
		}
		else {
			IMaxDC_eff = IMaxDC;
		}

		// set up efficiency splines with linear interpolation for the first point
		//Pdc_threshold = 2;
		//std::vector<double> ondspl_X[2][3];
		//std::vector<double> ondspl_Y[2][3];
		//int splineIndex;
		//bool switchoverDone;

		//if (VNomEff[2] > 0) {
		//	noOfEfficiencyCurves = 3;
		//}
		//else {
		//	noOfEfficiencyCurves = 1;
		//}

		//for (int j = 0; j <= noOfEfficiencyCurves - 1; j = j + 1) {
		//	splineIndex = 0;
		//	switchoverDone = false;
		//	ondspl_X[0][j].clear();
		//	ondspl_Y[0][j].clear();
		//	ondspl_X[1][j].clear();
		//	ondspl_Y[1][j].clear();
		//	double atX[3];
		//	double atY[3];
		//	const int MAX_ELEMENTS = 100; // = effCurve_elements + 5;
		//	for (int i = 0; i <= MAX_ELEMENTS - 1; i = i + 1) {
		//		if (i <= 2) { // atan
		//			// atan
		//			atX[i] = effCurve_Pdc[j][i];
		//			atY[i] = effCurve_eta[j][i];

		//			if (i == 2) {
		//				x_lim[j] = effCurve_Pdc[j][i];

		//				double adder;
		//				double err;
		//				b[j] = 10;
		//				adder = 40;

		//				for (int k = 0; k <= 100; k = k + 1) {
		//					a[j] = atY[2] / atan(b[j] * atX[2] / PNomDC_eff);
		//					err = (a[j] * atan(b[j] * atX[1] / PNomDC_eff)) - atY[1];
		//					if (err > 0) {
		//						b[j] = b[j] - adder;
		//						adder = adder / 2;
		//					}
		//					else {
		//						b[j] = b[j] + adder;
		//					}
		//				}
		//			}
		//		}
		//		if (i >= 2 && i <= 99 && (effCurve_Pdc[j][i] > 0 || effCurve_eta[j][i] > 0)) { // spline
		//			ondspl_X[splineIndex][j].push_back(effCurve_Pdc[j][i]);
		//			ondspl_Y[splineIndex][j].push_back(effCurve_eta[j][i]);
		//		}
		//	}
		//	bool doCubicSpline[2];
		//	doCubicSpline[0] = true;
		//	doCubicSpline[1] = true;
		//	for (int i = 0; i <= 1; i = i + 1) {
		//		if (i == 0 || (i == 1 && Pdc_threshold < 0.8)) {
		//			effSpline[i][j].set_points(ondspl_X[i][j], ondspl_Y[i][j], doCubicSpline[i]);
		//		}
		//	}
		//}
		Pdc_threshold = 2;
		std::vector<double> ondspl_X;
		std::vector<double> ondspl_Y;
		DenseVector xSamples(1);
		DataTable samples;
//		int splineIndex;
//		bool switchoverDone;

		if (VNomEff[2] > 0) {
			noOfEfficiencyCurves = 3;
		}
		else {
			noOfEfficiencyCurves = 1;
		}

		for (int j = 0; j <= noOfEfficiencyCurves - 1; j = j + 1) {
//			splineIndex = 0;
//			switchoverDone = false;
			ondspl_X.clear();
			ondspl_Y.clear();
			double atX[3];
			double atY[3];
			const int MAX_ELEMENTS = 100; // = effCurve_elements + 5;
//			x_lim[j] = effCurve_Pdc[j][0];
			for (int i = 0; i <= MAX_ELEMENTS - 1; i++) 
			{
				
				if (i <= 2) { // atan
							  // atan
					atX[i] = effCurve_Pdc[j][i];
					atY[i] = effCurve_eta[j][i];

					if (i == 2) {
						x_lim[j] = effCurve_Pdc[j][i];

						double adder;
						double err;
						b[j] = 10;
						adder = 40;

						for (int k = 0; k <= 100; k = k + 1) {
							a[j] = atY[2] / atan(b[j] * atX[2] / PNomDC_eff);
							err = (a[j] * atan(b[j] * atX[1] / PNomDC_eff)) - atY[1];
							if (err > 0) {
								b[j] = b[j] - adder;
								adder = adder / 2;
							}
							else {
								b[j] = b[j] + adder;
							}
						}
					}
				}
				// include overlap at i=2
				if ((i >=2 && i < MAX_ELEMENTS) && (effCurve_Pdc[j][i] > 0))// && effCurve_eta[j][i] > 0)) 
				{ // spline
					ondspl_X.push_back(effCurve_Pdc[j][i]);
					ondspl_Y.push_back(effCurve_eta[j][i]);
				}
			}
			/* Spline
			bool doCubicSpline[2];
			doCubicSpline[0] = true;
			doCubicSpline[1] = true;
			for (int i = 0; i <= 1; i = i + 1) {
				if (i == 0 || (i == 1 && Pdc_threshold < 0.8)) {
					effSpline[i][j].set_points(ondspl_X[i], ondspl_Y[i], doCubicSpline[i]);
				}
			}
			*/
			// SPLINTER
			samples.clear();
			x_max[j] = ondspl_X.back();
			for (size_t k = 0; k < ondspl_X.size() && k < ondspl_Y.size(); k++)
			{
				xSamples(0) = ondspl_X[k];
				samples.addSample(xSamples, ondspl_Y[k]);
			}
			m_bspline3[j] = BSpline::Builder(samples).degree(3).build();

		}
		ondIsInitialized = true;
	}
}

double ond_inverter::calcEfficiency(double Pdc, int index_eta) {
	double eta;
//	int splineIndex;
	DenseVector x(1);
//	if (Pdc > (Pdc_threshold * PNomDC_eff)) {
//		splineIndex = 1;
//	}
//	else {
//		splineIndex = 0;
//	}
//	if (Pdc > PMaxDC_eff)
//	{
//		Pdc = PMaxDC_eff;
//	}
	if (Pdc > x_max[index_eta])
	{
		Pdc = x_max[index_eta];
	}
	if (Pdc <= 0) {
		eta = 0;
	}
	else if (Pdc >= x_lim[index_eta]) 
	{
//		eta = effSpline[splineIndex][index_eta](Pdc);
		x(0) = Pdc;
		eta = (m_bspline3[index_eta]).eval(x);
	}
	else 
	{
		eta = a[index_eta] * atan(b[index_eta] * Pdc / PNomDC_eff);
	}
	return eta;
}

// return maximum AC power based on inverter temperature [W]
// arrayPAC values must be in [kW] as in OND file
double ond_inverter::tempDerateAC(double arrayT[], double arrayPAC[], double T) {
	double PAC_max;
	double T_low;
	double T_high;
	double PAC_low;
	double PAC_high;
	const double PAC_MAX_INIT = -10 ^ 10;

	PAC_max = PAC_MAX_INIT;

	for (int i = 0; i <= TEMP_DERATE_ARRAY_LENGTH - 1; i = i + 1) {
		if (i == 0) {
			if (T <= arrayT[0]) {
				PAC_max = arrayPAC[0];
				break;
			}
			else if (T > arrayT[TEMP_DERATE_ARRAY_LENGTH - 1]) {
				PAC_max = arrayPAC[TEMP_DERATE_ARRAY_LENGTH - 1];
				break;
			}
		}
		else {
			if (T > arrayT[i - 1] && T <= arrayT[i]) {
				T_low = arrayT[i - 1];
				T_high = arrayT[i];
				PAC_low = arrayPAC[i - 1];
				PAC_high = arrayPAC[i];
				PAC_max = PAC_low + (PAC_high - PAC_low) * (T - T_low) / (T_high - T_low);
				break;
			}
		}
	}

	if (doAllowOverpower == 0 && doUseTemperatureLimit == 0) {
		PAC_max = PNomConv;
	}
	else if (doAllowOverpower == 1 && doUseTemperatureLimit == 0) {
		PAC_max = max(PAC_max, PNomConv);
	}
	else if (doAllowOverpower == 0 && doUseTemperatureLimit == 1) {
		PAC_max = min(PAC_max, PNomConv);
	}
	else if (doAllowOverpower && doUseTemperatureLimit) {
		// Do nothing, keep PAC_max
	}

	if (PAC_max == PAC_MAX_INIT) {
		throw std::invalid_argument("PAC_max has not been set.");
	}
	return (PAC_max);
}

bool ond_inverter::acpower(
	/* inputs */
	double Pdc,     /* Input power to inverter (Wdc) */
	double Vdc,     /* Voltage input to inverter (Vdc) */
	double Tamb,

	/* outputs */
	double *Pac,    /* AC output power (Wac) */
	double *Ppar,   /* AC parasitic power consumption (Wac) */
	double *Plr,    /* Part load ratio (Pdc_in/Pdc_rated, 0..1) */
	double *Eff,	    /* Conversion efficiency (0..1) */
	double *Pcliploss, /* Power loss due to clipping loss (Wac) */
	double *Psoloss, /* Power loss due to operating power consumption (Wdc) */
	double *Pntloss, /* Power loss due to night time tare loss (Wac) */
	double *dcloss,		/* DC power loss (Wdc) */
	double *acloss		/* AC power loss (Wac) */
)
{
	// Limit Pac to temperature limit
	double Pac_max_T;
	Pac_max_T = tempDerateAC(T_array, PAC_array, Tamb);

	// Limit Pac to current limit
	double Pac_max_I=0.0;

	// calculate voltage drop in DC cabling
	double dV_dcLoss;
	double Vdc_eff;
	double Pdc_eff;
	double Idc_eff;

	Pdc_eff = min(Pdc, Pac_max_T); // Limit Pdc to temperature limit
	Vdc_eff = Vdc;
	dV_dcLoss = 0;
	if (Vdc > 0 && Pdc > 0) {
		for (int i = 0; i <= 2; i = i + 1) {
			Idc_eff = Pdc_eff / Vdc_eff;
			dV_dcLoss = lossRDc * Idc_eff;
			*dcloss = dV_dcLoss * Idc_eff;
			Vdc_eff = Vdc - dV_dcLoss;
			Pac_max_I = Vdc_eff * IMaxDC_eff;
			if (Pdc > Pac_max_I) {
				Pdc = Pac_max_I; // Limit Pdc to current limit
			}
			Pdc_eff = Pdc - *dcloss;
		}
	}

	// determine efficiency from splines
	double V_eta_arr[2];
	double eta_arr[2];
	int index_shift;
	int index_eta;
	if (Pdc > 0) {
		if (noOfEfficiencyCurves == 3) {
			if (Vdc_eff < VNomEff[1]) {
				index_shift = 0;
			}
			else {
				index_shift = 1;
			}
			for (int i = 0; i <= 1; i = i + 1) {
				index_eta = index_shift + i;
				V_eta_arr[i] = VNomEff[index_eta];
				eta_arr[i] = calcEfficiency(Pdc_eff, index_eta); // effSpline[splineIndex][index_eta](Pdc_eff);
			}
			*Eff = eta_arr[0] + (eta_arr[1] - eta_arr[0]) * (Vdc_eff - V_eta_arr[0]) / (V_eta_arr[1] - V_eta_arr[0]);
		}
		else if (noOfEfficiencyCurves == 1) {
			*Eff = calcEfficiency(Pdc_eff, 0);
		}

		if (*Eff < 0.0) *Eff = 0.0;
		*Pac = *Eff * Pdc_eff;

		// Calculate clipping/limiting losses
		*Pcliploss = 0.0;
		double PacNoClip = *Pac;
		if (*Pac > Pac_max_T || *Pac > Pac_max_I)
		{
			*Pac = min(Pac_max_T, Pac_max_I);
			*Pcliploss = PacNoClip - *Pac;
		}
	}
	else {
		*Eff = 0;
		*Pac = 0;
	}

	// night time power loss Wac (note that if PacNoPso > Pso and Pac < Pso then the night time loss could be considered an operating power loss)
	// Pso: /* DC power require to start inversion process, or self-consumption by inverter (Wdc) */ = PSeuil
	*Psoloss = 0.0; // Self-consumption during operation
	*Ppar = 0.0;
	*Pntloss = 0.0;
	if (Pdc_eff <= PSeuil)
	{
		*Pac = -Night_Loss;
		*Ppar = Night_Loss;
		*Pntloss = Night_Loss;
	}
	else
	{
		// Power consumption during operation only occurs
		// when inverter is operating during the day
		// calculate by setting B to zero (ie. Pso = 0 );
		double PacNoPso = *Pac + Aux_Loss;
		*Psoloss = PacNoPso - *Pac;
	}

	// calculate voltage drop in AC cabling
	double Iac;

	// calculate AC loss, but do not subtract from Pac (will be done in pvsamv1)
	Iac = *Pac / VOutConv;
	*acloss = lossRAc * Iac * Iac;
	//*Pac = *Pac - *acloss;

	// Final calculations and returning true
	*Plr = Pdc_eff / PNomDC_eff;
	return true;
}

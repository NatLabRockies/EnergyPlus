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

#define _TCSTYPEINTERFACE_
#include "tcstype.h"
//#include <shared/lib_util.h>
#include "lib_util.h"
#include <algorithm>

#include "interpolation_routines.h"

using namespace std;

/* 
Clear sky POA estimates using Bird model


Determine clear sky GHI, DNI, and DHI from Bird/Holstrum model
This function was converted from the RREDC HTML Bird and Holstrum Model.
Don't try to compare this with the XLS version becuase Zenith angle and Air Mass
are calculated with simplified equations that results in up to 5deg
differences in zenith angle at higher zenith angles.  Also, the XLS
version uses 0.9662 for the direct component calculation when 0.9751 is
more accepted (see notes at the calculation)

Input Parameters:
  Time - a struct with the following elements, note that all elements
      can be column vectors, but they must all be the same length.
      pvl_maketimestruct may be used to generate the Time input.
  Time.year = The year in the gregorian calendar
  Time.month = the month of the year (January = 1 to December = 12)
  Time.day = the day of the month
  Time.hour = the hour of the day
  Time.minute = the minute of the hour
  Time.second = the second of the minute
  Time.UTCOffset = the UTC offset code, using the convention
     that a positive UTC offset is for time zones east of the prime meridian
     (e.g. EST = -5)

  Location - a struct with the following elements, note that all
     elements may scalars or column vectors, but they must all be the same 
     length (e.g. Time.hour(i) must correspond to Location.latitude(i)).
     pvl_makelocationstruct may be used to make scalar (stationary)
     location files.
  Location.latitude = vector or scalar latitude in decimal degrees (positive is
     northern hemisphere)
  Location.longitude = vector or scalar longitude in decimal degrees (positive is 
     east of prime meridian)
  Location.altitude = vector or scalar height above sea level in meters.
     While altitude is optional in many uses, it is required in this
     model implmentation.

  Param - required specific model inputs

  Output:   
  ClearSkyGHI - the modeled global horizonal irradiance in W/m^2 provided
     by the Bird clear-sky model.
  ClearSkyDNI - the modeled direct normal irradiance in W/m^2 provided
     by the Bird clear-sky model.
  ClearSkyDHI - the calculated diffuse horizonal irradiance in W/m^2 
     provided by the Bird clear-sky model.

  MoreInput - 
      Param.ground albedo
      Param.o3cm total ozone
      Param.h2ocm total precipitable water vapor
      Param.aod380 turbidity at .38um
      Param.aod500 turbidity at .70um
      Param.presspa;
      Values Recommeded by Bird 1991 unless good aerosol info is available defaults:
      Ba = 0.84 Aerosol forward scattering ratio for sky albedo 
      K1 = 0.1 Aerosol absorptance


*/


enum{	//Parameters
		P_AOD_INIT,
		P_H2O_INIT,
		P_CS_CUTOFF,

		//Inputs
		I_SUNEL,
		I_DNI_ACT,
		I_PRESS,
		I_TEMP,
		I_RH,
		I_ALBEDO,

		//Outputs
		O_AOD,
		O_H2OCM,
		O_ICS_REF,
		O_AIRMASS,

		//N_MAX
		N_MAX};

tcsvarinfo atmospheric_aod_variables[] = {
	//Parameters
	{TCS_PARAM, TCS_NUMBER, P_AOD_INIT, "AOD_init", "Initial aerosol optical depth", "cm", "", "", ".1"},
	{TCS_PARAM, TCS_NUMBER, P_H2O_INIT, "H2O_init", "Initial H2O optical depth", "cm", "", "", "2."},
	{TCS_PARAM, TCS_NUMBER, P_CS_CUTOFF, "cs_cutoff", "Cutoff factor for AOD calculation", "", "", "", "0.75"},

	//Inputs
	{TCS_INPUT, TCS_NUMBER, I_SUNEL, "SunEl", "Solar elevation angle", "deg", "", "", ""},
	{TCS_INPUT, TCS_NUMBER, I_DNI_ACT, "dni_act", "Measured DNI", "W/m2", "", "", ""},
	{TCS_INPUT, TCS_NUMBER, I_PRESS, "press", "Ambient pressure", "mbar", "", "", ""},
	{TCS_INPUT, TCS_NUMBER, I_TEMP, "temp", "Average station temperature", "C", "","", ""},
	{TCS_INPUT, TCS_NUMBER, I_RH, "rh", "Relative humidity", "%", "", "", ""},
	{TCS_INPUT, TCS_NUMBER, I_ALBEDO, "albedo", "Ground albedo", "-", "", "", ""},

	//Outputs
	{TCS_OUTPUT, TCS_NUMBER, O_AOD, "AOD", "Aerosol optical depth", "", "", "", ""},  
	{TCS_OUTPUT, TCS_NUMBER, O_H2OCM, "H2Ocm", "Total precipitable water vapor", "cm", "", "", ""},
	{ TCS_OUTPUT, TCS_NUMBER, O_ICS_REF, "DNI_clrsky_ref", "Reference clear sky DNI", "W/m2", "", "", "" },
	{ TCS_OUTPUT, TCS_NUMBER, O_AIRMASS, "airmass", "Air mass - pres. corrected", "cm", "", "", "" },

	//N_MAX
	{TCS_INVALID, TCS_INVALID, N_MAX,			0,					0, 0, 0, 0, 0	} } ;
	
	
class atmospheric_aod : public tcstypeinterface
{
private:

	//params
	double
		AOD_init,
		H2O_init,
		cs_cutoff;

	//inputs
	double
		SunEl,
		dni_act,
		press,
		temp,
		rh,
		albedo,
		O3cm,
		Ta3,
		Ta5;
	//Outputs
	double
		Id,
		Ics,
		AOD,	//AOD
		H2Ocm;	//water depth

	//Others
	double pi, d2r;
	double H2Ocm_last, AOD_last;


public:
	atmospheric_aod( tcscontext *cst, tcstypeinfo *ti)
		: tcstypeinterface( cst, ti)
	{
		AOD_init = std::numeric_limits<double>::quiet_NaN();
		H2O_init = std::numeric_limits<double>::quiet_NaN();
		cs_cutoff = std::numeric_limits<double>::quiet_NaN();
		
		SunEl = std::numeric_limits<double>::quiet_NaN();
		dni_act = std::numeric_limits<double>::quiet_NaN();
		press = std::numeric_limits<double>::quiet_NaN();
		temp = std::numeric_limits<double>::quiet_NaN();
		rh = std::numeric_limits<double>::quiet_NaN();
		albedo = std::numeric_limits<double>::quiet_NaN();
		
		H2Ocm_last = std::numeric_limits<double>::quiet_NaN();
		AOD_last = std::numeric_limits<double>::quiet_NaN();

		pi = acos(-1.);
		d2r = pi/180.;

	}

	virtual ~atmospheric_aod()
	{
	}

	virtual int init()
	{
		//Set parameters
		AOD_init = value(P_AOD_INIT);
		H2O_init = value(P_H2O_INIT);
		cs_cutoff = value(P_CS_CUTOFF);
		
		//Initialize stored variables with reasonable values
		AOD_last = AOD_init;
		H2Ocm_last = H2O_init;

		return 0;
	}

	virtual int call( double time, double step, int ncall )
	{						
		
		//Collect input values
		SunEl = 90-value(I_SUNEL);
		dni_act = value(I_DNI_ACT);
		press = value(I_PRESS);
		temp = value(I_TEMP);
		rh = value(I_RH);
		albedo = value(I_ALBEDO);
		
		if(albedo < 0.) albedo = 0.2;

		if(SunEl <= 1. ){	//The Bird model is valid up to 89deg zenith.
			Id = 0.;
			Ics = 0.;
			value(O_AOD, 0.);
			value(O_H2OCM, 0.);
			value(O_ICS_REF, 0.);
			value(O_AIRMASS, 0.);
			return 0;
		}

		
		/* 
		typical values for ozone optical depth (Dobson Units) are 300-500, but typically closer to 300 over non-extreme latitudes
		http://en.wikipedia.org/wiki/File:IM_ozavg_ept_200006.png
		http://www.temis.nl/protocols/o3col/data/omi/o3doas_yesterday.gif
		Conversion from DU to cm AOD at STP ==> .0001*DU
		http://en.wikipedia.org/wiki/Dobson_unit
		
		Aerosol optical depth at 380 and 500 nm wavelengths:
		http://icap.atmos.und.edu/ObservabilityMeeting/MeetingPDFs/Day-2/8_AERONET-Eck.pdf
		http://aaqr.org/VOL8_No4_December2008/8_AAQR-08-05-OA-0015_459-476.pdf
		http://aeronet.gsfc.nasa.gov/new_web/Documents/Aerosol_Optical_Depth.pdf

		The ratio of AOD at 380 and 500 appears to be about 0.75:1 (-> 500nm:380nm)

		Checked out surfrad (NOAA), desert environment seems to corroborate 0.75 number.

		*/

		O3cm = 0.3;	//[cm]	Confirmed with Pete


		//unit conversions
		press *= 100.;	//[mbar] -> [Pa]
		//...

		//Calculate precipitable water depth based on Gueymard estimate
		H2Ocm = w_Gueymard(temp, rh);


		// Recommeded values by Bird unless good aerosol info is available
		// Aerosol forward scattering ratio for sky albedo
		/*res = sum(cellfun(@(x) strcmp(x, 'Ba'), fieldnames(Param)));
		if ~res 
			Ba = 0.84 * ones(length(Time.year),1) ;
		else
			Ba = Param.Ba;
		end
		*/
		double Ba = 0.84;		//confirmed with Pete

		// Aerosol absorptance
		double K1 = 0.1;

		// Determine day of year and extraterrestrial normal radiation for each 
		// instant in Time.
		int DayOfYear = (int)floor(time/(3600.*24))+1;
		double Io = pvl_extraradiation(DayOfYear);

		// Calculate solar Zenith Angle
		// Assuming 
		// 1. Pressure is local standard pressure (per pvl_alt2pres)
		// 2. Temperature is 12C (default for pvl_ephemeris)
		//[~, SunEl, ~, ~]=pvl_ephemeris(Time, Location, press, temp);
		double ZA = 90.-SunEl;  

		// Determine Airmass.  Critical component of Bird model.
		// Kasten is used in original Bird paper
		double AM = pvl_relativeairmass(ZA,"kasten1966");
		// Pressure corrected airmass
		double AMp = pvl_absoluteairmass(AM, press);
		
		// Rayleigh Scattering Transmittance
		double Tr = exp((-0.0903 * pow(AMp, 0.84) ) * (1+AMp- pow(AMp, 1.01) ));
		// Ozone
		double Ozm = O3cm * AM;
		double Toz = 1 - 0.1611*Ozm * pow(1+139.48*Ozm, -0.3035) - 0.002715*Ozm / (1+0.044*Ozm+0.0003*pow(Ozm, 2));
		// Mixed gases
		double Tm = exp(-0.0127 * pow(AMp, 0.26));
		// Water Vapor
		double Wm = AM*H2Ocm;
		double Tw = 1 - 2.4959 * Wm / ( pow(1+79.034 * Wm, 0.6828) + 6.385 * Wm);

		Ics = Io * 0.9751 * Tr * Toz * Tm;	//DNI before accounting for water vapor or aerosols

		double Ta, TAA, TAS, Rs, Tau;

		if (dni_act > 1.){
			//begin loop here
			Ta3 = .5;
			double err = 999.;
			double tol = 0.001;
			double Ta3_0, Id0;
			int qi = 0;			//iteration
			while (fabs(err) > tol){
				Ta5 = Ta3 * 0.75;

				/*
				PG: Relationship is:
				ln(aod) = f ( ln(wavelength(um)) )
				[0..1] um
				close to linear
				*/

				// Daily turbidity
				Tau = 0.2758 * Ta3 + 0.35 * Ta5;
				Ta = exp(-pow(Tau, 0.873)*(1 + Tau - pow(Tau, 0.7088))*pow(AM, 0.9108));
				TAA = 1 - K1 * (1 - AM + pow(AM, 1.06)) * (1 - Ta);
				TAS = Ta / TAA;
				// Sky albedo
				Rs = 0.0685 + (1 - Ba)*(1 - TAS);

				// Direct component
				// Original multiplier of 0.9662 is factor based on Solar Constant of
				// Io=1353 and Thekakara spectral distribution  (1981) Maxwell and Iqbal
				// both recommend using 0.9751 instead, representing a Solar Constant
				// of Io=1367 and WMO-Wherli spectral distribution (1990)
				Id = Ics * Tw * Ta;

				//Calculate error between Bird DNI and reported DNI
				err = (Id - dni_act) / dni_act;
				double r = Id / dni_act;

				//Adjust the guess for Ta3
				if (qi == 0){  //First iteration or really big errors
					Ta3_0 = Ta3;
					Ta3 *= pow(r, 0.7);
				}
				else
				{
					//Linear projection to intersection point based on last step
					double dTa = (dni_act - Id0) / (Id - Id0)*(Ta3 - Ta3_0);
					if (dTa < -Ta3_0) dTa = -Ta3_0*0.5;
					double Ta_new = Ta3_0 + dTa;
					Ta3_0 = Ta3;
					Ta3 = Ta_new;

				}

				Id0 = Id;
				qi++;
			}
		// Direct on horizontal surface
		//double Idh=Id * cos(ZA*d2r);
		// Diffuse (scattered)
		//double Ias=Io * cos(ZA*d2r) * 0.79 * Toz * Tw * Tm * TAA * (0.5 * (1.0-Tr)+Ba * (1.0-TAS)) / (1.0-AM+ pow(AM, 1.02));
		// Total dif + dir on horizontal
		//double Itot=(Idh+Ias) / (1-albedo*Rs);
		//double Idif=Itot-Idh;

		// Return same size array 
		//ClearSkyDNI = Id;
		//ClearSkyGHI = Itot;
		//ClearSkyDHI = Idif;

		AOD = Tau;
		}
		else
		{
			AOD = 0.;
			AMp = 0.;
		}

		// Set output parameters
		if (Id / Ics > cs_cutoff && Ics > 0.){
			AOD_last = AOD;
			H2Ocm_last = H2Ocm;
			value(O_AOD, AOD);
			value(O_H2OCM, H2Ocm);
		}
		else
		{
			H2Ocm_last = H2Ocm;
			value(O_AOD, AOD_last);
			value(O_H2OCM, H2Ocm_last);
		}

		value(O_ICS_REF, Ics);
		value(O_AIRMASS, AMp);

		return 0;
	}

	virtual int converged( double time )
	{
		return 0;
	}

	/* 
	-----------------------------------------------------------------------------------------------
		Supplemental methods	
	-----------------------------------------------------------------------------------------------
	*/

	double pvl_extraradiation(int doy){
		/* 
		PVL_EXTRARADIATION Determine extraterrestrial radiation from day of year

		Syntax
		Ea = pvl_extraradiation(doy)

		Description
		Determine the amount of extraterrestrial solar radiation.

		Output Ea is the extraterrestrial radiation present in watts per square meter
		on a surface which is normal to the sun. Ea is of the same size as the
		input doy.

		Input doy is an array specifying the day of year. Valid values are >=1 and <367.


		Source
		http://solardat.uoregon.edu/SolarRadiationBasics.html, Eqs. SR1 and SR2
		SR1 	   	Partridge, G. W. and Platt, C. M. R. 1976. Radiative Processes in Meteorology and Climatology.
		SR2 	   	Duffie, J. A. and Beckman, W. A. 1991. Solar Engineering of Thermal Processes, 2nd edn. J. Wiley and Sons, New York.

		See also PVL_DAYOFYEAR PVL_DISC
		*/
		double B, Rfact2, Ea;
		
		B = 2*pi*doy/365;
		Rfact2 = 1.00011 + 0.034221 * cos(B)+ 0.00128*sin(B)+ 0.000719*cos(2*B)+0.000077*sin(2*B);
		Ea = 1367*Rfact2;

		return Ea;
	};

	double pvl_relativeairmass(double zenith, string varargin){
		/*
		 PVL_RELATIVEAIRMASS    Gives the relative (not pressure-corrected) airmass

		 Syntax
		   AM = RELATIVEAIRMASS(z)
		   AM = RELATIVEAIRMASS(z, model)

		 Description
		   Gives the airmass at sea-level when given a sun zenith angle, z (in 
		   degrees). 
		   The "model" variable allows selection of different airmass models
		   (described below). "model" must be a valid string. If "model" is not 
		   included or is not valid, the default model is 'kastenyoung1989'.

		 Inputs:
		   z - Zenith angle of the sun.  Note that some models use the apparent (refraction corrected)
			 zenith angle, and some models use the true (not refraction-corrected)
			 zenith angle. See model descriptions to determine which type of zenith
			 angle is required.
		   model - String variable indicating the airmass model to be used.  Avaiable models 
			 include the following:
			   'simple' - secant(apparent zenith angle) - Note that this gives -inf at zenith=90
			   'kasten1966' - See reference [1] - requires apparent sun zenith
			   'youngirvine1967' - See reference [2] - requires true sun zenith
			   'kastenyoung1989' - See reference [3] - requires apparent sun zenith
			   'gueymard1993' - See reference [4] - requires apparent sun zenith
			   'young1994' - See reference [5] - requries true sun zenith
			   'pickering2002' - See reference [6] - requires apparent sun zenith

		 Outputs:
		   AM - Relative airmass at sea level.  Will return NaN values for all zenith 
			 angles greater than 90 degrees.

		 References:
		   [1] Fritz Kasten. "A New Table and Approximation Formula for the
		   Relative Optical Air Mass". Technical Report 136, Hanover, N.H.: U.S.
		   Army Material Command, CRREL.
		   [2] A. T. Young and W. M. Irvine, "Multicolor Photoelectric Photometry
		   of the Brighter Planets," The Astronomical Journal, vol. 72, 
		   pp. 945-950, 1967.
		   [3] Fritz Kasten and Andrew Young. "Revised optical air mass tables and
		   approximation formula". Applied Optics 28:4735–4738
		   [4] C. Gueymard, "Critical analysis and performance assessment of 
		   clear sky solar irradiance models using theoretical and measured data,"
		   Solar Energy, vol. 51, pp. 121-138, 1993.
		   [5] A. T. Young, "AIR-MASS AND REFRACTION," Applied Optics, vol. 33, 
		   pp. 1108-1110, Feb 1994.
		   [6] Keith A. Pickering. "The Ancient Star Catalog". DIO 12:1, 20,

		 See also PVL_ABSOLUTEAIRMASS PVL_EPHEMERIS
		*/

		if(zenith > 90.) return std::numeric_limits<double>::quiet_NaN();

		double d2r = pi/180.;
		double coszen = cos(zenith*d2r);

		double AM;

		if(varargin == "kastenyoung1989")
			AM = 1./(coszen+0.50572*(pow(6.07995+(90-zenith), -1.6364) ));
		else if(varargin == "kasten1966")
			AM = 1./(coszen+0.15*pow(93.885-zenith, -1.253) );
		else if(varargin == "simple")
			AM = 1./coszen;
		else if(varargin == "pickering2002")
			AM = 1./(sin( d2r * (90-zenith+244./(165+47*pow(90-zenith, 1.1) ) )) );
		else if(varargin == "youngirvine1967")
			AM = 1./coszen*(1-0.0012*((pow(1./coszen, 2) )-1));
		else if(varargin == "young1994")
			AM = (1.002432*pow(coszen, 2) + 0.148386 * coszen + 0.0096467) / (pow(coszen, 3) + 0.149864 * pow(coszen, 2) + 0.0102963 * coszen + 0.000303978);
		else if(varargin == "gueymard1993")
			AM = 1./(coszen + 0.00176759 * zenith * pow(94.37515-zenith, -1.21563) );
		else{
			string msg = varargin + " is not a valid model type for relative airmass. The kastenyoung1989 model was used.";
			message(TCS_WARNING, msg.c_str());
			AM = 1./(coszen+0.50572*(pow(6.07995+(90-zenith), -1.6364)));
		}

		return AM;
	}; 

	double pvl_absoluteairmass(double AMrelative, double pressure){
		/*
		 PVL_ABSOLUTEAIRMASS Determine absolute (pressure corrected) airmass from relative airmass and pressure

		 Syntax
		 AMa = pvl_absoluteairmass(AMrelative, pressure)

		 Description
 
		   Gives the airmass for locations not at sea-level (i.e. not at standard
		   pressure). The input argument "AMrelative" is the relative airmass. The
		   input argument "pressure" is the pressure (in Pascals) at the location
		   of interest and must be greater than 0. The calculation for
		   absolute airmass is:
		   absolute airmass = (relative airmass)*pressure/101325

		 Inputs:   
		   AMrelative - The airmass at sea-level.  This can be calculated using the 
			 PV_LIB function pvl_relativeairmass. 
		   pressure - a scalar or vector of values providing the site pressure in
			 Pascal. If pressure is a vector it must be of the same size as all
			 other vector inputs. pressure must be >=0. Pressure may be measured
			 or an average pressure may be calculated from site altitude.

		 Output:   
		   AMa - Absolute (pressure corrected) airmass
   
		 References
		   [1] C. Gueymard, "Critical analysis and performance assessment of 
		   clear sky solar irradiance models using theoretical and measured data,"
		   Solar Energy, vol. 51, pp. 121-138, 1993.

		 See also PVL_RELATIVEAIRMASS
		*/

		return AMrelative*pressure/101325.;
	}

	double w_Gueymard(double T, double RH){
		/*
		Saturation vapor pressure from Gueymard (J. Appl. Meteorol., 1993)

		Temperature [C]
		Relative humidity [%]
		*/
		
		//limited error check
		if(T < -100.) T = 15.;
		if(RH < 0.) RH = 20.;
		
		double Tabs, T0, es, ev, rov, T1, Hv;

		Tabs=T+273.15;
		T0=Tabs/100.0;
		es=exp(22.329699-49.140396/T0 -10.921853/(T0*T0)-.39015156*T0);
		ev=0.01*RH*es;
		
		//Precipitable water estimate from Gueymard (Solar Energy, 1994)
		rov=216.7 * ev / Tabs;
		T1=Tabs / 273.15;
		Hv=0.4976 + 1.5265 * T1+exp(13.6897 * T1-14.9188 * pow(T1, 3) );
		
		return 0.1*Hv*rov;
	};

};

TCS_IMPLEMENT_TYPE( atmospheric_aod, "Aerosol optical depth calculator", "Mike Wagner", 1, atmospheric_aod_variables, NULL, 1 )


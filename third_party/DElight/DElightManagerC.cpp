/* Copyright 1992-2009	Regents of University of California
 *						Lawrence Berkeley National Laboratory
 *
 *  Authors: R.J. Hitchcock and W.L. Carroll
 *           Building Technologies Department
 *           Lawrence Berkeley National Laboratory
 */
/**************************************************************
 * C Language Implementation of DOE2.1d and Superlite 3.0
 * Daylighting Algorithms with new Complex Fenestration System
 * analysis algorithms.
 *
 * The original DOE2 daylighting algorithms and implementation
 * in FORTRAN were developed by F.C. Winkelmann at the
 * Lawrence Berkeley National Laboratory.
 *
 * The original Superlite algorithms and implementation in FORTRAN
 * were developed by Michael Modest and Jong-Jin Kim
 * under contract with Lawrence Berkeley National Laboratory.
 **************************************************************/

// This work was supported by the Assistant Secretary for Energy Efficiency
// and Renewable Energy, Office of Building Technologies,
// Building Systems and Materials Division of the
// U.S. Department of Energy under Contract No. DE-AC03-76SF00098.

/*
NOTICE: The Government is granted for itself and others acting on its behalf
a paid-up, nonexclusive, irrevocable worldwide license in this data to reproduce,
prepare derivative works, and perform publicly and display publicly.
Beginning five (5) years after (date permission to assert copyright was obtained),
subject to two possible five year renewals, the Government is granted for itself
and others acting on its behalf a paid-up, nonexclusive, irrevocable worldwide
license in this data to reproduce, prepare derivative works, distribute copies to
the public, perform publicly and display publicly, and to permit others to do so.
NEITHER THE UNITED STATES NOR THE UNITED STATES DEPARTMENT OF ENERGY, NOR ANY OF
THEIR EMPLOYEES, MAKES ANY WARRANTY, EXPRESS OR IMPLIED, OR ASSUMES ANY LEGAL
LIABILITY OR RESPONSIBILITY FOR THE ACCURACY, COMPLETENESS, OR USEFULNESS OF ANY
INFORMATION, APPARATUS, PRODUCT, OR PROCESS DISCLOSED, OR REPRESENTS THAT ITS USE
WOULD NOT INFRINGE PRIVATELY OWNED RIGHTS.
*/

#pragma warning(disable:4786)

// Standard includes
#include <cmath>
#include <vector>
#include <map>
#include <fstream>
#include <cstring>
#include <limits>
#include <algorithm> // for min/max
using namespace std;

// BGL includes
#include "BGL.h"
namespace BGL = BldgGeomLib;

// includes
#include "CONST.H"
#include "DBCONST.H"
#include "DEF.H"

// WLC includes
#include "NodeMesh2.h"
#include "WLCSurface.h"
#include "helpers.h"
#include "hemisphiral.h"
#include "btdf.h"
#include "CFSSystem.h"
#include "CFSSurface.h"

// includes
#include "DOE2DL.H"
#include "TOOLS.H"
#include "DElightManagerC.h"
#include "DElight2.h"

// Persistent data structures for storing DElight data
// required across EnergyPlus calls to the DLL
BLDG bldg;	// DElight bldg data structure
LIB lib;	// DElight library data structure

// Persistent Error/Warning handling vars
ofstream ofdmpfile;	// Error message dump file */
int iErrorOccurred = 0; // Error/Warning occurred flag

/******************************** subroutine writewndo *******************************/
/* Error/Warning handling routine for WLC code modules. */
/******************************** subroutine writewndo *******************************/
void writewndo(const std::string instring, string sfpflg)
{
	// Check for open Error message dump file.
	if(!ofdmpfile)
	{
        // Register that an Error file opening error has occurred
        iErrorOccurred = 1;

        // Throw an appropriate message to highest level calling routine
		throw "ERROR: DElight - No open Error Message file\n";
	}

    // Check Flag type (e=Error, w=Warning)
    if (sfpflg.size() == 0) return;
    if (sfpflg[0] == 'e') {
        // Write properly formulated Error message to Error file
		ofdmpfile << "ERROR: DElight - " << instring << "\n";

        // Register that an Error has occurred
        iErrorOccurred = 2;

        // Throw to highest level calling routine
		throw "";
    }
    else if (sfpflg[0] == 'w') {
        // Formulate Warning message and write it to Error file
		ofdmpfile << "WARNING: DElight - " << instring << "\n";

        // Register that a Warning has occurred
        iErrorOccurred = 3;

        // Return to calling location
        return;
    }
    else {
        // Do nothing and return to calling location
        return;
    }

    return;
}

/******************************** subroutine delightdaylightcoefficients *******************************/
/* Calls the DElight daylighting factors/coefficients routine from the DElight DLL. */
/* Exported subroutine for EnergyPlus preprocessing call to DElight. */
/* See corresponding Interface Subroutine in DElightManagerF.cc EnergyPlus module. */
/******************************** subroutine delightdaylightcoefficients *******************************/
extern "C" DllExport void delightdaylightcoefficients(double dBldgLat,
                                                      int* piErrorFlag)  // return Error Flag from DElight to EPlus
{
    // Open Error message dump file
    ofdmpfile.open("eplusout.delightdfdmp");

    // Check for successful opening of Error message dump file.
	if(!ofdmpfile)
	{
        // Set ErrorFlag return value for interpretation within EPlus
        *piErrorFlag = -1;
		return;
	}

// Wrap try block around the rest of this routine
try {

    char cFullInputFilename[250+1];
    char cFullOutputFilename[250+1];

    // Create filenames for DElight input and output files that will default to current working directory.
    strcpy( cFullInputFilename, "eplusout.delightin" );
    strcpy( cFullOutputFilename, "eplusout.delightout" );

    /* Set limits of sun position angles. */

    // Minimum altitude angle
    double dphsmin = 10.;
    if (fabs(dBldgLat) >= 48.0) dphsmin = 5.;

    // Minimum azimuth angle
    double dthsmin = -110.;
    /* Minimum solar azimuth for southern hemisphere */
    if (dBldgLat < 0.0) dthsmin = 70.;

    // Call DElight Daylight Factor calculation routine from the DElight2.dll
    // Init ErrorFlag return value
    int iErrorFlag = 0;
    iErrorFlag = DElightDaylightFactors4EPlus(cFullInputFilename,		/* input file name */
                                                    cFullOutputFilename,	/* output file name */
                                                    &bldg,		/* pointer to DElight bldg data structure */
                                                    &lib,		/* pointer to DElight library data structure */
                                                    5,			/* number of radiosity iterations */
                                                    0.0,		/* fraction of sky covered by clouds (0.0=clear 1.0=overcast) */
                                                    10,			/* Desired number of surface nodes */
                                                    10,			/* Desired number of window nodes */
                                                    NPHS,		/* Number of preprocessor sun altitude angles (0 => Full Set) */
                                                    dphsmin,	/* Minimum preprocessor sun altitude angle */
                                                    NTHS,		/* Number of preprocessor sun azimuth angles (0 => Full Set) */
                                                    dthsmin,	/* Minimum preprocessor sun azimuth angle */
                                                    &ofdmpfile);    // Error message dump file
    // Check returned ErrorFlag value
    if (iErrorFlag < 0) {
        // Set appropriate ErrorFlag return value for interpretation within EPlus
        *piErrorFlag = iErrorFlag;
    }

    // Check iErrorOccurred flag used in writewndo() for WLC code module
    if (iErrorOccurred == 3) {  // Warning(s) occurred
        // Set appropriate ErrorFlag return value for interpretation within EPlus
        *piErrorFlag = -10;
    }

    // Close Error message dump file
    ofdmpfile.close();

    // return to EnergyPlus normally from here
    return;

}   // end try
// Catch throws from writewndo() that handle errors/warnings in WLC code
catch(string sThrownMsg) {
    // Check Error/Warning occurred flag value
    if (iErrorOccurred == 1) {
        // No Error message file open, open one and write the thrown message to it
        ofdmpfile.open("eplusout.delightdfdmp");
        char msg[210];
        snprintf(msg, 210, "%s\n", sThrownMsg.c_str());
		ofdmpfile << msg;
        // Close Error message dump file
        ofdmpfile.close();
        // Set appropriate ErrorFlag return value for interpretation within EPlus
        *piErrorFlag = -2;
        // Return to EnergyPlus
        return;
    }
    else {
        // Close Error message dump file
        ofdmpfile.close();
        // Set appropriate ErrorFlag return value for interpretation within EPlus
        *piErrorFlag = -2;
        // Return to EnergyPlus
        return;
    }
}
catch(char* cThrownMsg) {
    // Check Error/Warning occurred flag value
    if (iErrorOccurred == 1) {
        // No Error message file open, open one and write the thrown message to it
        ofdmpfile.open("eplusout.delightdfdmp");
        char msg[210];
        snprintf(msg, 210, "%s\n", cThrownMsg);
		ofdmpfile << msg;
        // Close Error message dump file
        ofdmpfile.close();
        // Set appropriate ErrorFlag return value for interpretation within EPlus
        *piErrorFlag = -2;
        // Return to EnergyPlus
        return;
    }
    else {
        // Close Error message dump file
        ofdmpfile.close();
        // Set appropriate ErrorFlag return value for interpretation within EPlus
        *piErrorFlag = -2;
        // Return to EnergyPlus
        return;
    }
}
return;
}

/******************************** subroutine delightelecltgctrl *******************************/
/* Calls the DElight daylighting interior illuminance and electric lighting control routines from the DElight DLL. */
/* Exported subroutine for EnergyPlus timestep call to DElight. */
/* See corresponding Interface Subroutine in DElightManagerF.cc EnergyPlus module. */
/******************************** subroutine delightelecltgctrl *******************************/
extern "C" DllExport void delightelecltgctrl(int iNameLength,
                                    char* cZoneName,
                                    double dBldgLat,
                                    double dHISKF,
                                    double dHISUNF,
                                    double dCloudFraction,
                                    double dSOLCOSX,
                                    double dSOLCOSY,
                                    double dSOLCOSZ,
                                    double* pdPowerReducFac,	// return value for calculated Zone Elec Ltg Power Reduction Factor
                                    int* piErrorFlag)  // return Error Flag from DElight to EPlus
{
    // Open Error message dump file
    ofdmpfile.open("eplusout.delighteldmp", ios_base::out | ios_base::app);

    // Check for successful opening of debug dump file.
	if(!ofdmpfile)
	{
        // Set ErrorFlag return value for interpretation within EPlus
        *piErrorFlag = -1;
		return;
	}

// Wrap try block around remainder of this routine
try {

    /* Set limits of sun position angles. */

    // Minimum altitude angle
    double dphsmin = 10.;
    if (fabs(dBldgLat) >= 48.0) dphsmin = 5.;

    /* Maximum altitude and altitude angle increment for sun positions. */
    double dphsmax = min(90.0,113.5-fabs(dBldgLat));
    double dphsdel = (dphsmax - dphsmin) / ((double)(4-1));

    // Minimum azimuth angle
    double dthsmin = -110.;
    /* Minimum solar azimuth for southern hemisphere */
    if (dBldgLat < 0.0) dthsmin = 70.;

    /* Maximum azimuth and azimuth angle increment for sun positions. */
    double dthsdel = fabs(2.0 * dthsmin) / ((double)(5-1));
    double dthsmax = dthsmin + dthsdel * ((double)(5-1));

    // Transfer solar direction cosines to an array
    double dSOLCOS[3];
    dSOLCOS[0] = dSOLCOSX;
    dSOLCOS[1] = dSOLCOSY;
    dSOLCOS[2] = dSOLCOSZ;

    // Identify the current Zone index within the DElight Bldg structure
    // First modify cZoneName to match DElight names without blanks
    // Put a string terminator at the end of the Fortran-like string
    cZoneName[iNameLength] = '\0';
    // Replace all blank spaces in the Zone name with underscore characters for DElight input reading
    cZoneName = str_blnk2undr(cZoneName);
    // Now search for Zone name in DElight Bldg structure
	int iZone;
    for (iZone=0; iZone<bldg.nzones; iZone++) {
        if (strcmp(bldg.zone[iZone]->name, cZoneName) == 0) break;
    }

    // Init ErrorFlag return value
    int iErrorFlag = 0;

    // Call DElight Electric Lighting Control routine from the DElight2.dll.
    iErrorFlag = DElightElecLtgCtrl4EPlus(
        &bldg,					/* pointer to DElight Bldg data structure */
        bldg.zone[iZone],		/* pointer to DElight Zone data structure */
        dHISKF,			        /* Exterior horizontal illuminance from sky (lum/m^2) */
        dHISUNF,		        /* Exterior horizontal beam illuminance (lum/m^2) */
        dCloudFraction,	        /* fraction of sky covered by clouds (0.0=clear 1.0=overcast) */
        dSOLCOS,				/* Direction cosines of current sun position */
        dphsmin,			/* Minimum daylight factor sun altitude angle */
        dthsmin,			/* Minimum daylight factor sun azimuth angle */
        dphsmax,			/* Maximum daylight factor sun altitude angle */
        dthsmax,			/* Maximum daylight factor sun azimuth angle */
        dphsdel,			/* Increment of daylight factor sun altitude angles */
        dthsdel,			/* Increment of daylight factor sun azimuth angles */
        &ofdmpfile);        // Error message dump file

    // Check returned ErrorFlag value
    if (iErrorFlag < 0) {
        // Set appropriate ErrorFlag return value for interpretation within EPlus
        *piErrorFlag = iErrorFlag;
    }

    // Check iErrorOccurred flag used in writewndo() for WLC code module
    if (iErrorOccurred == 3) {  // Warning(s) occurred
        // Set appropriate ErrorFlag return value for interpretation within EPlus
        *piErrorFlag = -10;
    }

    // Set the return value for Power Reduction Factor (1.0 => Full Power On)
    *pdPowerReducFac = bldg.zone[iZone]->frac_power;

	/* Close Error message dump file. */
	ofdmpfile.close();

    return;
}   // end try
// Catch throws from writewndo() that handle errors/warnings in WLC code
catch(string sThrownMsg) {
    // Check Error/Warning occurred flag value
    if (iErrorOccurred == 1) {
        // No Error message file open, open one and write the thrown message to it
        ofdmpfile.open("eplusout.delighteldmp");
        char msg[210];
        snprintf(msg, 210, "%s\n", sThrownMsg.c_str());
		ofdmpfile << msg;
        // Close Error message dump file
        ofdmpfile.close();
        // Set appropriate ErrorFlag return value for interpretation within EPlus
        *piErrorFlag = -2;
        // Return to EnergyPlus
        return;
    }
    else {
        // Close Error message dump file
        ofdmpfile.close();
        // Set appropriate ErrorFlag return value for interpretation within EPlus
        *piErrorFlag = -2;
        // Return to EnergyPlus
        return;
    }
}
catch(char* cThrownMsg) {
    // Check Error/Warning occurred flag value
    if (iErrorOccurred == 1) {
        // No Error message file open, open one and write the thrown message to it
        ofdmpfile.open("eplusout.delighteldmp");
        char msg[210];
        snprintf(msg, 210, "%s\n", cThrownMsg);
		ofdmpfile << msg;
        // Close Error message dump file
        ofdmpfile.close();
        // Set appropriate ErrorFlag return value for interpretation within EPlus
        *piErrorFlag = -2;
        // Return to EnergyPlus
        return;
    }
    else {
        // Close Error message dump file
        ofdmpfile.close();
        // Set appropriate ErrorFlag return value for interpretation within EPlus
        *piErrorFlag = -2;
        // Return to EnergyPlus
        return;
    }
}
return;
}

/******************************** subroutine delightfreememory *******************************/
/* Calls the DElight routines to free memory allocated by DElight */
/* Not currently used by EnergyPlus. */
/******************************** subroutine delightfreememory *******************************/
extern "C" DllExport void delightfreememory()
{
    // Call DElight Memory Freeing routine from the DElight2.dll.
    DElightFreeMemory4EPlus(
        &bldg,		/* pointer to DElight bldg data structure */
        &lib);		/* pointer to DElight library data structure */

    return;
}

/****************************** subroutine delightoutputgenerator *****************************/
/* Calls the DElight routine to generate a DElight output file tailored by the output flag */
/* Not currently used by EnergyPlus. */
/* All EnergyPlus related output are passed back to EnergyPlus either through parameter list */
/* or via temporary ASCII file. */
/****************************** subroutine delightoutputgenerator *****************************/
extern "C" DllExport void delightoutputgenerator(int iOutputFlag)
{
    (void)iOutputFlag;
    // Call DElight Output Generation routine.
    // NOT YET IMPLEMENTED

    return;
}


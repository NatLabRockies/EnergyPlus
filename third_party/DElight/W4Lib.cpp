/* Copyright 1992-2009	Regents of University of California
 *						Lawrence Berkeley National Laboratory
 *
 *  Author: R.J. Hitchcock and W.L. Carroll
 *          Building Technologies Department
 *          Lawrence Berkeley National Laboratory
 */
/**************************************************************
 * C Language Implementation of DOE2 Daylighting Algorithms.
 *
 * The original DOE2 algorithms and implementation in FORTRAN
 * were developed by F.C. Winkelmann.
 * Simulation Research Group, Lawrence Berkeley Laboratory.
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
#include <vector>
#include <map>
#include <fstream>
#include <cstring>
#include <limits>
#include <cstdlib>

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
#include "W4Lib.h"
#include "struct.h"

/****************************** subroutine process_W4glazing_types *****************************/
// Reads the Window4 library file from disk,
// searches for library entries corresponding to IDs used in user building input file,
// processes matching entries,
// and populates new entries in DElight internal GLASS library.
/****************************************************************************/
/* C Language Implementation of DOE2 Daylighting Algorithms */
/* by Rob Hitchcock */
/* Building Technologies Program, Lawrence Berkeley Laboratory */
/****************************** subroutine process_W4glazing_types *****************************/
int process_W4glazing_types(
	BLDG *bldg_ptr,		/* building structure pointer */
	LIB *lib_ptr,		/* library structure pointer */
	FILE *W4libfile,	/* pointer to Window4 library data file */
	ofstream* pofdmpfile)	/* ptr to dump file */
{
	int iz, is, iw;	/* indexes */

	// Set up a list to contain unique glass_type codes as they are encountered in this building
	int iUniqueGlassIDs[200];
	int iUniqueIDCount = 0;

	// Cycle through building description identifying unique Window4 glass_type IDs.

	// Zone loop
	for (iz=0; iz<bldg_ptr->nzones; iz++) {

		// Surface loop
		for (is=0; is<bldg_ptr->zone[iz]->nsurfs; is++) {

			// Window loop
			for (iw=0; iw<bldg_ptr->zone[iz]->surf[is]->nwndos; iw++) {

				// Is this a Window4 glass type?
				int iGlass_Type_ID = atoi(bldg_ptr->zone[iz]->surf[is]->wndo[iw]->glass_type);
				if (iGlass_Type_ID > 11) {

					// Check to see if this is a unique glass_type ID (i.e., not yet encountered in this building)
					if (IsGlassIDUnique(iGlass_Type_ID, iUniqueGlassIDs, &iUniqueIDCount)) {

						// Read and process the Window4 data for this glass type and add it to the DElight GLASS library
						if (ProcessW4GlassType(iGlass_Type_ID, lib_ptr, W4libfile, pofdmpfile) < 0) {
							*pofdmpfile << "ERROR: DElight Cannot create new LIB GLASS entry for Window4 library entry ID = " << bldg_ptr->zone[iz]->surf[is]->wndo[iw]->glass_type << "\n";
							return (-1);
						}
					}
				}
			}
		}
	}

	return(0);
}

/****************************** subroutine IsGlassIDUnique *****************************/
/* Checks to see if the given glass_type ID is contained in the list of encountered IDs. */
/****************************************************************************/
/* C Language Implementation of DOE2 Daylighting Algorithms */
/* by Rob Hitchcock */
/* Building Technologies Program, Lawrence Berkeley Laboratory */
/****************************** subroutine IsGlassIDUnique *****************************/
int IsGlassIDUnique(
	int iGlass_Type,			// current glass_type ID
	int iUniqueGlassIDs[200],	// array of encountered glass_type IDs
	int* piUniqueIDCount)		// ptr to current count of unique glass_type ID
{
	for (int iGID = 0; iGID < *piUniqueIDCount; iGID++) {
		if (iUniqueGlassIDs[iGID] == iGlass_Type)
			return (0);
	}

	// Add the newly encountered unique ID to the list
	iUniqueGlassIDs[*piUniqueIDCount] = iGlass_Type;
	(*piUniqueIDCount)++;

	return (1);
}

/****************************** subroutine ProcessW4GlassType *****************************/
/* Locate and process the given Window4 glass_type ID. */
/****************************************************************************/
/* C Language Implementation of DOE2 Daylighting Algorithms */
/* by Rob Hitchcock */
/* Building Technologies Program, Lawrence Berkeley Laboratory */
/****************************** subroutine ProcessW4GlassType *****************************/
int ProcessW4GlassType(
	int iGlass_Type,	// W4 glass_type ID
	LIB *lib_ptr,		// library structure pointer
	FILE *W4libfile,	// pointer to Window4 library data file
	ofstream* pofdmpfile)	/* ptr to dump file */
{
	char cInputLine[MAX_CHAR_LINE+1];	// Input line
	char *token;						/* Input token pointer */
	int iW4ID;							// Window ID holder
	int iEntryFound = 0;				// matching ID found flag
	int iInLine;						// index
	double dTvis[10], dYdat[10], dTvisHemi;		// temp Tvis data holders
	double dTvFit1, dTvFit2;					// angular visible data curve fit coefs

	// Cycle through W4 library entries looking for a match.
	do {
		// Read the first six lines in a W4 library entry to reach the Window ID.
		for (iInLine = 0; iInLine < 6; iInLine++)
			if (fgets(cInputLine, MAX_CHAR_LINE, W4libfile) == NULL) return -1;;

		// Scan the sixth line to get the Window ID.
		sscanf(cInputLine,"%*s %*s %*s %d\n",&iW4ID);

		// Check to see if this is the matching W4 entry.
		if (iW4ID == iGlass_Type) {
			// Read and process the visible data for this entry.

			// Skip to the Tvis data line
			for (iInLine = 6; iInLine < 32; iInLine++)
				if (fgets(cInputLine, MAX_CHAR_LINE, W4libfile) == NULL) return -1;;

			// Scan the angular data and Tvis Hemispherical
			// Tokenize the line label
			token = strtok(cInputLine," ");
			// Tokenize the angular visible data
			int iAngle;
			for (iAngle=0; iAngle<10; iAngle++) {
				token = strtok(NULL," ");
				dTvis[iAngle] = atof(token);
			}
			// Tokenize the hemispherical visible data
			token = strtok(NULL," ");
			dTvisHemi = atof(token);

			// Qikfit the angular visible data.
			// Normalize the angular visible data by Tvis at normal incidence.
			for (iAngle=0; iAngle<10; iAngle++) {
				dYdat[iAngle] = dTvis[iAngle] / (dTvis[0] + 0.000001);
			}
			Qikfit4(10, dYdat, &dTvFit1, &dTvFit2);

			// Create a new DElight LIB GLASS entry
			lib_ptr->glass[lib_ptr->nglass] = new GLASS;
			if (lib_ptr->glass[lib_ptr->nglass] == NULL) {
//			if ((lib_ptr->glass[lib_ptr->nglass] = (GLASS *)malloc(sizeof(GLASS))) == NULL) {
				*pofdmpfile << "ERROR: DElight Insufficient memory for GLASS allocation\n";
				return(-1);
			}
			// Init the new GLASS entry
			struct_init("GLASS",(char *)lib_ptr->glass[lib_ptr->nglass]);

			// Populate the new DElight LIB GLASS entry with the Window4 data
			snprintf(lib_ptr->glass[lib_ptr->nglass]->name, 60, "%d", iGlass_Type);	/* glass type ID: 1 to 11 => DOE2 original, >11 => W4lib.dat, <0 => E10 library */
			lib_ptr->glass[lib_ptr->nglass]->vis_trans = dTvis[0];		/* visible transmittance at normal incidence */
			lib_ptr->glass[lib_ptr->nglass]->W4hemi_trans = dTvisHemi;	/* Window 4 hemispherical transmittance */
			lib_ptr->glass[lib_ptr->nglass]->W4vis_fit1 = dTvFit1;		/* Window 4 angular transmission curve fit coef #1 */
			lib_ptr->glass[lib_ptr->nglass]->W4vis_fit2 = dTvFit2;		/* Window 4 angular transmission curve fit coef #2 */

			// Skip to the Rbvis data line
			for (iInLine = 32; iInLine < 34; iInLine++)
				if (fgets(cInputLine, MAX_CHAR_LINE, W4libfile) == NULL) return -1;;

			// Scan the thirty-fourth line to get the inside hemispherical visible reflectance.
			sscanf(cInputLine,"%*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %*s %d\n",(int*)&lib_ptr->glass[lib_ptr->nglass]->inside_refl);

			// Increment LIB GLASS entry counter
			(lib_ptr->nglass)++;

			// Set flag
			iEntryFound = 1;
		}
		else {	// skip the remaining lines of this entry
			for (iInLine = 6; iInLine < 55; iInLine++)
				if (fgets(cInputLine, MAX_CHAR_LINE, W4libfile) == NULL) return -1;;
		}
    }
    while(!iEntryFound);

	return (0);
}

/****************************** subroutine Qikfit4 *****************************/
// Find the two undetermined coeffs in the constrained quartic form
// y = x*(2 - x + (1 - x)^2*(coef1 + coef2*(2+x)))
// which automatically satisfies the constraints y(0)=0, y(1)=1, y'(1)=0
/****************************************************************************/
/* C Language Implementation of DOE2 Daylighting Algorithms */
/* by Rob Hitchcock */
/* Building Technologies Program, Lawrence Berkeley Laboratory */
/****************************** subroutine Qikfit4 *****************************/
int Qikfit4(
	int iData,			// number of data points
	double dYdat[10],	// normalized angular visible data
	double* pdTvFit1,	// pointer to coef1
	double* pdTvFit2)	// pointer to coef2
{
	// data for curve fitting
	double dX0[10] = {1.0, 0.984808, 0.939693, 0.866025, 0.776044, 0.642788, 0.5, 0.342020, 0.173648, 0.0};
	double dp1=0.0, dp2=0.0, dp3=0.0, dp4=0.0, dp5=0.0, dp6=0.0;
	double d0, d1, d2;

	for (int ii = 0; ii < iData; ii++) {
		d0 = (2.0 - dX0[ii]) * dX0[ii];
		d1 = dX0[ii] * (1.0 - dX0[ii]) * (1.0 - dX0[ii]);
		d2 = 2.0 + dX0[ii];
		dp1 += (dYdat[ii] - d0) * d1;
		dp2 += d1 * d1;
		dp3 += d1 * d1 * d2;
		dp4 += (dYdat[ii] - d0) * d1 * d2;
		dp5 += d1 * d1 * d2;
		dp6 += d1 * d1 * d2 * d2;
	}

	*pdTvFit1 = (dp1 * dp6 - dp3 * dp4) / (dp2 * dp6 - dp3 * dp5);
	*pdTvFit2 = (dp1 * dp5 - dp2 * dp4) / (dp3 * dp5 - dp2 * dp6);

	return (0);
}

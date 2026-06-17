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
#include <iostream>
#include <fstream>
#include <cstdlib>
#include <cmath>
#include <vector>
#include <map>
#include <cstring>
#include <string>
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
#include "DFcalcs.h"
#include "SOL.H"
#include "geom.h"
#include "TOOLS.H"
#include "Radiosity.h"

/****************************** subroutine CalcDFs *****************************/
/* Calculates daylighting factors (interior illum / exterior horiz illum) */
/* for each ref_pt in a lighting zone, for overcast sky and */
/* a range of sun postions for clear skies, for open and closed window shades. */
/* Calculates coefficients for use in the hourly daylighting calculation. */
/* Converted from DOE2.1D FORTRAN code */
/* Modified from initial implementation using Radiosity algorithms */
/* from SuperLite 3.0 FORTRAN code */
/****************************************************************************/
/* Modifications to original DOE2.1D DCOF() subroutine include: */
/* 	Variable number of sun positions allowed by passing numbers and minimum angles. */
/*	Accumulates total daylight illuminances (fc) due to overcast sky, clear sky, */
/*	and clear sun components at each reference point and returns these totals */
/*	in bldg_ptr->zone[izone]->ref_pt[irp] structure. */
/* (Note: open shades == clear glazing; closed shades == diffuse glazing) */
/*	All other window shade values are ignored. */
/****************************************************************************/
/* Modifications to initial implementation (DCOF()) include: */
/* 	Rearrange loops in main daylight factor calc section to include CFS apertures. */
/* 	Modify direct contribution from window section to calc initial illuminance on */
/*  surface nodes and reference points. */
/* 	Replace split-flux interreflection calc with radiosity algos from SuperLite. */
/****************************************************************************/
/* C Language Implementation of DOE2 Daylighting Algorithms */
/* by Rob Hitchcock */
/* Building Technologies Program, Lawrence Berkeley Laboratory */
/****************************** subroutine CalcDFs *****************************/
int	CalcDFs(
	SUN_DATA *sun_ptr,	/* pointer to sun data structure */
	BLDG *bldg_ptr,		/* pointer to bldg structure */
	LIB *lib_ptr,		/* pointer to library structure */
	int iIterations,	/* number of radiosity iterations */
	ofstream* pofdmpfile)	/* ptr to LBLDLL error dump file */
{
	int iphs, iths;					/* sun position indexes */
	int izone, isurf, irp, icoord, iIntSurf, inode;	/* bldg component indexes */
	int iw, igt, ic;				/* indexes */
	double thsmin;		/* minimum sun postition azimuth (degrees: South=0.0, East=+90.0) */
	double phsmin;					/* minimum sun postition altitude (degrees) */
	double phsmax;					/* maximum sun position altitude (degrees) */
	double phsdel, thsdel;			/* sun position angle increments (degrees) */
	double phsun;				/* sun alt (radians) */
	double thsun;				/* sun azm in FredW solar coordinate system [S=0, E=90] (radians) */
	double phsun_deg;				/* sun alt (degrees) */
	/* Window 4 code modification begin */
	double cam1, cam2, cam3, cam4;	/* window coefs of trans holders */
	double E10coef1,E10coef2,E10coef3,E10coef4;	/* Energy-10 angular dependence equation coefs */
	double W4vis_fit1, W4vis_fit2;	// Window4 angular dependence equation coefs
	/* Window 4 code modification end */
	// EnergyPlus code modification
	double EPlusCoef[6];	/* EnergyPlus angular dependence equation coefs */
	double wnorm[NCOORDS];			/* window outward normal vector */
	double node[NCOORDS];			/* surface node coordinate holder */
	double nodesurfnormal[NCOORDS];		/* node surface INWARD normal unit vector */
	double ww, hw;		/* surface and window geom vars */
	double solic[MONTHS];	/* extraterrestrial irrad for 1st of each month (0 to 11) */
	double zenl;				/* clear sky zenith luminance (Kcd/m2) */
	double tfac;				/* turbidity factor */
	double tvisdf;			/* visible trans vars */
	double vis_trans;		/* visible transmittance of window for illum calcs */
	double domega;	/* solid angle subtended by window wrt ref_pt */
	HIT hit;				/* bldg-shade hit structure for dhitsh() test */
	double rwin[NCOORDS];	/* center of window element */
	double ray[NCOORDS];		/* ref_pt to center of window element vector */
	double phray, thray;		/* ray angles */
	double disq, ddis, dis;	/* ref_pt to window element distance vars */
	double cosWndoIncidence; 	/* cos of angle between ray and window outward normal */
	double tvisincidence;	/* tvis of glass for cosWndoIncidence angle */

	/* rjh 4/17/97 Move wsghit[][] outside dreflt() and CalcDiffuseWindowLuminance() to preserve values. */

    // Init Return Value
    int iReturnVal = 0;

	/* Set limits of sun position angles. */
	phsmin = sun_ptr->phsmin;
	thsmin = sun_ptr->thsmin;

	/* For non-standard number of sun altitudes minimum altitude is passed into CalcDFs(). */
	/* Reset minimum altitude for standard number of sun altitudes. */
	if (sun_ptr->nphs == NPHS) {
		phsmin = 10.;
		if (fabs(bldg_ptr->lat) >= 48.0) phsmin = 5.;
	}
	/* Maximum altitude and altitude angle increment for sun positions. */
	if (sun_ptr->nphs == 1) {
		phsmax = phsmin;
		phsdel = 0.;
	}
	else {
		phsmax = min(90.0,113.5-fabs(bldg_ptr->lat));
		phsdel = (phsmax - phsmin) / ((double)(sun_ptr->nphs-1));
	}

	/* For non-standard number of sun azimuths minimum azimuth is passed into CalcDFs(). */
	/* Reset minimum azimuth and azm angle increment for standard number of sun azimuths. */
	if (sun_ptr->nths == NTHS) {
		thsmin = -110.;
		/* minimum solar azimuth for southern hemisphere */
		if (bldg_ptr->lat < 0.0) thsmin = 70.;
	}
	/* Azimuth angle increment for sun positions. */
	if (sun_ptr->nths == 1) thsdel = 0.;
	else thsdel = fabs(2.0 * thsmin) / (sun_ptr->nths - 1);

	/* Calculate extraterrestrial direct normal solar illumination (lum/ft2) */
	/* for the first day of each month. */
	dsolic(solic);

	/* Find exterior illuminances (lum/ft2) on ground and bldg shade luminances */
	/* for different sun positions. */

	/* Sun position altitude (phsun) and azimuth (thsun) loops. */
	for (iphs=0; iphs<sun_ptr->nphs; iphs++) {
		phsun_deg = phsmin + (double)iphs * phsdel;
		phsun = (phsun_deg * DTOR);
		thsun = 0.;

// rjh debug
//*pofdmpfile << "Sun Altitude: iphs = " << iphs << " phsun = " << phsun << " phsun_deg = " << phsun_deg << "\n";

		/* Get clear sky zenith luminance, moisture, */
		/* and turbidity coef for reference month. */
		dzenlm(&zenl,&tfac,IMREF,bldg_ptr,phsun);

		/* Get exterior horiz illum from sky and sun for clear and overcast sky.  */
		if (dhill(&(bldg_ptr->hillumskyc[iphs]),&(bldg_ptr->hillumsunc[iphs]),&(bldg_ptr->hillumskyo[iphs]),bldg_ptr,IMREF,phsun,thsun,zenl,tfac,solic,pofdmpfile) < 0) {
			*pofdmpfile << "ERROR: DElight Bad return from dhill(), exit CalcDFs()\n";
			return(-1);
		}

		/* Sun position azm loop */
		for (iths=0; iths<sun_ptr->nths; iths++) {

			/* Solar azm wrt azm = 0 along bldg coord sys X-axis */
			/* converted from sun coord sys where azm=0 due South and azm>0 toward east. */
			thsun = ((thsmin + (double)iths * thsdel - 90.0) * DTOR + bldg_ptr->azm * DTOR);

// rjh debug
//if (iphs == 0) *pofdmpfile << "Sun Azimuth: iths = " << iths << " thsun = " << thsun << " thsun deg = " << thsun/DTOR << "\n";

			/* Calculate building shade luminances. */
			if (dshdlu(bldg_ptr,phsun,thsun,iphs,iths,solic,tfac,zenl,bldg_ptr->hillumskyc[iphs],bldg_ptr->hillumskyo[iphs],bldg_ptr->hillumsunc[iphs],pofdmpfile) < 0) {
				*pofdmpfile << "ERROR: DElight Bad return from dshdlu(), exit CalcDFs()\n";
				return(-1);
			}

			/* Calculate diffusing glazing window luminances. */
			// HERE foreach diffusing glazing window

		}
	}

	/* ------ Direct (or Initial) Illuminance at Nodal Surfaces Calculation ------ */

	/* Lighting Zone Loop */
	for (izone=0; izone<bldg_ptr->nzones; izone++) {

		/* Exterior Surface Loop */
		for (isurf=0; isurf<bldg_ptr->zone[izone]->nsurfs; isurf++) {

			/* Window Loop */
			for (iw=0; iw<bldg_ptr->zone[izone]->surf[isurf]->nwndos; iw++) {

				/* get library index of current window glass type */
				igt = lib_index(lib_ptr,"glass",bldg_ptr->zone[izone]->surf[isurf]->wndo[iw]->glass_type);
				if (igt < 0) continue;
				/* shorten often used bldg structure elements */
				ww = bldg_ptr->zone[izone]->surf[isurf]->wndo[iw]->width;
				hw = bldg_ptr->zone[izone]->surf[isurf]->wndo[iw]->height;

				// Window 4 implementation 4/2002
				// glass type ID: 1 to 11 => DOE2 original, >11 => W4lib.dat, <0 => E10 library
				int iGlass_Type_ID = atoi(bldg_ptr->zone[izone]->surf[isurf]->wndo[iw]->glass_type);
				if ((iGlass_Type_ID > 0) && (iGlass_Type_ID <= 11))  {	// DOE2 original

					// DOE2 original angular dependence equation coefs
					cam1 = lib_ptr->glass[igt]->cam1;
					cam2 = lib_ptr->glass[igt]->cam2;
					cam3 = lib_ptr->glass[igt]->cam3;
					cam4 = lib_ptr->glass[igt]->cam4;

					/* Diffuse and Normal transmittance for total solar spectrum. */
					double tsoldf = lib_ptr->glass[igt]->cam9;
					double tsolnm = cam1 + cam2 + cam3 + cam4;

					/* Diffuse transmittance (for normal vis_trans = 1.0) */
					tvisdf = (1.0 / tsolnm) * tsoldf;
				}
				else if ((iGlass_Type_ID > 11) && (iGlass_Type_ID <= 10000)) {	// Window4

					// Window4 angular dependence Fit4() function coefs.
					W4vis_fit1 = lib_ptr->glass[igt]->W4vis_fit1;
					W4vis_fit2 = lib_ptr->glass[igt]->W4vis_fit2;

					/* Diffuse transmittance (for normal vis_trans = 1.0) */
					tvisdf = lib_ptr->glass[igt]->W4hemi_trans / (lib_ptr->glass[igt]->vis_trans + 0.000001);
				}
				else if (iGlass_Type_ID > 10000) {	// EnergyPlus/Window5

					// EnergyPlus/Window5 angular dependence POLYF() function coefs.
					for (int icoef = 0; icoef < 6; icoef++)
					{
						EPlusCoef[icoef] = lib_ptr->glass[igt]->EPlusCoef[icoef];
					}

					/* Diffuse transmittance (for normal vis_trans = 1.0) */
					// NOTE: never actually used
				}
				else if (iGlass_Type_ID < 0) {	// Energy-10

					/* Energy-10 angular dependence equation coefs. */
					E10coef1 = lib_ptr->glass[igt]->E10coef[0];
					E10coef2 = lib_ptr->glass[igt]->E10coef[1];
					E10coef3 = lib_ptr->glass[igt]->E10coef[2];
					E10coef4 = lib_ptr->glass[igt]->E10coef[3];

					/* Diffuse transmittance (for normal vis_trans = 1.0) */
					tvisdf = lib_ptr->glass[igt]->E10hemi_trans;
				}
				else continue;

				/* Visible transmittance for this window for ref_pt illum calcs. */
				// NOTE: Not used for EnergyPlus/Window5 glass types
				vis_trans = lib_ptr->glass[igt]->vis_trans;

				/* unit vector normal to window (pointing away from room) */
				for (icoord=0; icoord<NCOORDS; icoord++) {
                    wnorm[icoord] = bldg_ptr->zone[izone]->surf[isurf]->outward_uvect[icoord];
                }

                // Switch from previous DOE2 window discretization method
                // to new WLC meshing method for polygons

                // Invoke WLC meshing method by reinvoking the WLCWNDOInit() method
				// for approx. 1/2 unit squares (i.e., 1/2ft x 1/2ft)
				// This means that ref pts beyond ~2ft from window will be accurate
                // wrt the subtended solid angle calculation.
                // Note that MaxGridNodeArea might get increased if number of nodes exceeds array limits.
                bldg_ptr->zone[izone]->surf[isurf]->wndo[iw]->WLCWNDOInit(0.25);

                // Loop through Wndo nodes
	            for (int iWndoElement=0; iWndoElement<bldg_ptr->zone[izone]->surf[isurf]->wndo[iw]->nnodes; iWndoElement++) {

                    // Tranfer the node position to the old rwin array.
                    for (ic=0; ic<NCOORDS; ic++) {
                        rwin[ic] = bldg_ptr->zone[izone]->surf[isurf]->wndo[iw]->node[iWndoElement][ic];
                    }

						/* Reference Point Loop */
						for (irp=0; irp<bldg_ptr->zone[izone]->nrefpts; irp++) {

							/* Calc ray from ref_pt to wndo element */
							/* distance between ref_pt and element */
							disq = 0.;
							for (ic=0; ic<NCOORDS; ic++) {
								ddis = rwin[ic] - bldg_ptr->zone[izone]->ref_pt[irp]->bs[ic];
								disq += ddis * ddis;
							}
							dis = sqrt(disq);

				            // Report distances that are too small for
				            // accurate window-element solid angle calculation.
				            if (dis < 2.0) {
                                // Set Return value to Warning
                                iReturnVal = -10;
					            *pofdmpfile << "WARNING: DElight Inaccurate daylight illuminance calculation may result for lighting zone " <<bldg_ptr->zone[izone]->name << "\n";
					            *pofdmpfile << "WARNING: for reference points closer than 2 feet from window " <<bldg_ptr->zone[izone]->surf[isurf]->wndo[iw]->name << "\n";
                                if (dis <= 0.0) {
					                *pofdmpfile << "WARNING: DElight Reference Point " << bldg_ptr->zone[izone]->ref_pt[irp]->name << " is positioned on the window surface and will be ignored.\n";
                                    continue;
                                }
				            }

							/* unit vector along ray from ref_pt to element */
							for (ic=0; ic<NCOORDS; ic++)
								ray[ic] = (rwin[ic] - bldg_ptr->zone[izone]->ref_pt[irp]->bs[ic]) / dis;

							/* Determine if ray intersects a zone-shade or bldg-shade. */
							/* NOTE: this includes all zone surfaces */
							/* contrary to DOE2 check of only "self-shade" */
							/* surfaces (in addition to zone and bldg shades). */
							/* dhitsh() resets HIT structure */
							dhitsh(&hit,bldg_ptr->zone[izone]->ref_pt[irp]->bs,ray,bldg_ptr,izone,isurf,isurf);

							/* Azm (-PI to PI) and alt (-PI/2 to PI/2) of ray (i.e., azm and alt of sky element) */
							/* Azm = 0 is along x-axis of bldg coord sys. */
							phray = asin(ray[2]);
							if ((ray[0]==0.0) && (ray[1]==0.0)) thray = 0.;
							else thray = atan2(ray[1],ray[0]);

// rjh debug
//if (irp == 0) {
//	*pofdmpfile << "Window Element ix = " << ix << " iy = " << iy << " icoords = " << rwin[0] << " " << rwin[1] << " " << rwin[2] << "\n";
//	*pofdmpfile << " phray = " << phray << " thray = " << thray << "\n";
//}

							/* Calc solid angle subtended by element wrt ref_pt */
							/* cos of angle between ray and window outward normal */
							cosWndoIncidence = ddot(wnorm,ray);


							/* Solid angle subtended by element wrt ref_pt */
//							domega = dwx * dwy * cosWndoIncidence / disq;
                            // Modified to use new node areas
							domega = bldg_ptr->zone[izone]->surf[isurf]->wndo[iw]->node_areas[iWndoElement] * cosWndoIncidence / disq;

							/* Calc tvis of glass for incidence angle */
							// Window 4 implementation 4/2002
							// glass type ID: 1 to 11 => DOE2 original, >11 => W4lib.dat, <0 => E10 library
							if ((iGlass_Type_ID > 0) && (iGlass_Type_ID <= 11))  {	// DOE2 original
								tvisincidence = max(0.0,(cam1+cosWndoIncidence*(cam2+cosWndoIncidence*(cam3+cosWndoIncidence*cam4))));
							}
							else if ((iGlass_Type_ID > 11) && (iGlass_Type_ID <= 10000)) {	// Window4
								tvisincidence = vis_trans * fit4(cosWndoIncidence, W4vis_fit1, W4vis_fit2);
							}
							else if (iGlass_Type_ID > 10000) {	// EnergyPlus/Window5
								tvisincidence = POLYF(cosWndoIncidence, EPlusCoef);
							}
							else if (iGlass_Type_ID < 0) {	// Energy-10
								tvisincidence = vis_trans * max(0.0,(cosWndoIncidence*(E10coef1+cosWndoIncidence*(E10coef2+cosWndoIncidence*(E10coef3+cosWndoIncidence*E10coef4)))));
							}

							/* Set ref_pt unit vector "surface" face normal (all ref_pts assumed horizontal facing upward). */
							nodesurfnormal[0] = 0.0;
							nodesurfnormal[1] = 0.0;
							nodesurfnormal[2] = 1.0;

							// Calculate Cos of angle between ray and inward normal unit vector for face of refpt plane.
							double cosPtSurfIncidence = ddot(nodesurfnormal,ray);

							/* Sun Position Altitude Loop */
							for (iphs=0; iphs<sun_ptr->nphs; iphs++) {

								/* Altitude of sun */
								phsun_deg = phsmin + (double)iphs * phsdel;
								phsun = phsun_deg * DTOR;

								/* Get clear sky zenith luminance, moisture, */
								/* and turbidity coef for reference month. */
								dzenlm(&zenl,&tfac,IMREF,bldg_ptr,phsun);

								/* Sun Position Azimuth Loop */
								for (iths=0; iths<sun_ptr->nths; iths++) {

									/* azm of sun in strange sun coord sys (0=East, counter-clockwise is positive) */
									thsun = (thsmin + (double)iths * thsdel - 90.0) * DTOR + bldg_ptr->azm * DTOR;

									/* Add contribution of current wndo element to */
									/* direct illum at current ref_pt, */
									/* for current sky condition. */
									int iWndoContribRetVal = wndo_element_refpt_illum_contrib(bldg_ptr,		/* pointer to bldg structure */
															lib_ptr,		/* pointer to lib structure */
															izone,			/* current zone index */
															isurf,			/* current surface index for Surface containing Window */
															isurf,			/* current node surface index NOT applicable */
															iw,				/* current window index */
															iWndoElement,	/* window element index */
															thsun,			/* sun azm (radians) */
															phsun,			/* sun alt (radians) */
															thray,			/* sky element azm angle */
															phray,			/* sky element alt angle */
															iphs,			/* sun alt index */
															iths,			/* sun azm index */
															solic,			/* extraterrestrial irrad for 1st of each month (0 to 11) */
															bldg_ptr->zone[izone]->ref_pt[irp]->bs,	/* coords of refpt */
															nodesurfnormal,	/* INWARD normal unit vector from face of refpt virtual surface */
															cosPtSurfIncidence,	/* cos(angle of incidence) of vector from refpt (on horiz plane) to wndo element */
															&hit,			/* hit structure pointer for ray from refpt to wndo element */
															domega,			/* solid angle subtended by window element wrt ref_pt */
															vis_trans,		/* tvis of glass for normal window incidence angle */
															tvisincidence,	/* tvis of glass for current window incidence angle */
															wnorm,			/* window outward normal vector */
															tfac,			/* turbidity factor */
															zenl,			/* zenith luminance */
															// return values stored in ref pt substructure
															&(bldg_ptr->zone[izone]->ref_pt[irp]->direct_skycillum[iphs][iths]),	/* ptr to direct illuminance from sky - clear */
															&(bldg_ptr->zone[izone]->ref_pt[irp]->direct_suncillum[iphs][iths]),	/* ptr to direct illuminance from sun - clear */
															&(bldg_ptr->zone[izone]->ref_pt[irp]->direct_skyoillum),				/* ptr to direct illuminance from sky - overcast */
															pofdmpfile);		/* ptr to LBLDLL error dump file */
                                    // Check return value for error/warning
                                    if (iWndoContribRetVal < 0) {
                                        // If errors were detected then return now, else register warnings and continue processing
                                        if (iWndoContribRetVal != -10) {
					                        *pofdmpfile << "ERROR: DElight Bad return from wndo_element_refpt_illum_contrib()\n";
					                        return(-1);
                                        }
                                        else {
                                            iReturnVal = -10;
                                        }
                                    }

								}	/* end of Sun Position Azimuth Loop */

							}	/* end of Sun Position Altitude Loop */

						}	/* end of Reference Point Loop */

						/* Interior Surface Loop */
						for (iIntSurf=0; iIntSurf<bldg_ptr->zone[izone]->nsurfs; iIntSurf++) {

							// Skip the current surface.
							if (iIntSurf == isurf) continue;

							// RJH 8/00
							// Get the visible reflectance of this surface.
							double dNodeSurfaceReflectance = bldg_ptr->zone[izone]->surf[iIntSurf]->vis_refl;

							/* Set node unit vector "surface" face normal (all nodes on a given surface have same normal). */
							// Note that this is the "inward facing" normal for the surface.
							for (ic=0; ic<NCOORDS; ic++)
								nodesurfnormal[ic] = bldg_ptr->zone[izone]->surf[iIntSurf]->inward_uvect[ic];

							/* Surface Nodal Patch Loop */
							for (inode=0; inode<bldg_ptr->zone[izone]->surf[iIntSurf]->nnodes; inode++) {

								// Transfer nodal patch coords to node[NCOORDS].
								for (ic=0; ic<NCOORDS; ic++)
									node[ic] = bldg_ptr->zone[izone]->surf[iIntSurf]->node[inode][ic];

								/* Calc ray from node to wndo element */
								/* distance between node and element */
								disq = 0.;
								for (ic=0; ic<NCOORDS; ic++) {
									ddis = rwin[ic] - node[ic];
									disq += ddis * ddis;
								}
								dis = sqrt(disq);

				                // Report distances that are too small for
				                // accurate window-element solid angle calculation.
								// RJH 2008-03-07: skip this warning and accept potential error
//				                if (dis < 2.0) {
                                    // Set return value for warning
//                                    iReturnVal = -10;
//					                *pofdmpfile << "WARNING: DElight Inaccurate daylight illuminance calculation may result for lighting zone " <<bldg_ptr->zone[izone]->name << "\n";
//					                *pofdmpfile << "WARNING: for surface nodes closer than 2 feet from window " <<bldg_ptr->zone[izone]->surf[isurf]->wndo[iw]->name << "\n";
//                                  if (dis <= 0.0) {
//					                    *pofdmpfile << "WARNING: DElight Surface Node " << inode << " on Surface " << bldg_ptr->zone[izone]->surf[iIntSurf]->name << " is positioned on the window surface and will be ignored.\n";
//                                      continue;
//                                  }
//				                }

								/* unit vector along ray from node to element */
								for (ic=0; ic<NCOORDS; ic++)
									ray[ic] = (rwin[ic] - node[ic]) / dis;

								/* Determine if ray intersects a zone-shade or bldg-shade. */
								/* NOTE: this includes all zone surfaces */
								/* contrary to DOE2 check of only "self-shade" */
								/* surfaces (in addition to zone and bldg shades). */
								/* dhitsh() sets HIT structure */
								dhitsh(&hit,node,ray,bldg_ptr,izone,isurf,iIntSurf);

								/* Azm (-PI to PI) and alt (-PI/2 to PI/2) of ray (i.e., azm and alt of sky element) */
								/* Azm = 0 is along x-axis of bldg coord sys. */
								phray = asin(ray[2]);
								if ((ray[0]==0.0) && (ray[1]==0.0)) thray = 0.;
								else thray = atan2(ray[1],ray[0]);

								/* Calc cos of angle between ray and window outward normal */
								cosWndoIncidence = ddot(wnorm,ray);

								/* Calc solid angle subtended by element wrt node */
//								domega = dwx * dwy * cosWndoIncidence / disq;
                                // Modified to use new node areas
							    domega = bldg_ptr->zone[izone]->surf[isurf]->wndo[iw]->node_areas[iWndoElement] * cosWndoIncidence / disq;

								/* Calc tvis of glass for incidence angle */
								// Window 4 implementation 4/2002
								// glass type ID: 1 to 11 => DOE2 original, >11 => W4lib.dat, <0 => E10 library
								if ((iGlass_Type_ID > 0) && (iGlass_Type_ID <= 11))  {	// DOE2 original
									tvisincidence = max(0.0,(cam1+cosWndoIncidence*(cam2+cosWndoIncidence*(cam3+cosWndoIncidence*cam4))));
								}
								else if ((iGlass_Type_ID > 11) && (iGlass_Type_ID <= 10000)) {	// Window4
									tvisincidence = vis_trans * fit4(cosWndoIncidence, W4vis_fit1, W4vis_fit2);
								}
								else if (iGlass_Type_ID > 10000) {	// EnergyPlus/Window5
									tvisincidence = POLYF(cosWndoIncidence, EPlusCoef);
								}
								else if (iGlass_Type_ID < 0) {	// Energy-10
									tvisincidence = vis_trans * max(0.0,(cosWndoIncidence*(E10coef1+cosWndoIncidence*(E10coef2+cosWndoIncidence*(E10coef3+cosWndoIncidence*E10coef4)))));
								}

								// Calculate Cos of angle between ray and inward normal unit vector for face of node surface plane.
								double cosPtSurfIncidence = ddot(nodesurfnormal,ray);

								/* Sun Position Altitude Loop */
								for (iphs=0; iphs<sun_ptr->nphs; iphs++) {

									/* Altitude of sun */
									phsun_deg = phsmin + (double)iphs * phsdel;
									phsun = phsun_deg * DTOR;

									/* Get clear sky zenith luminance, moisture, */
									/* and turbidity coef for reference month. */
									dzenlm(&zenl,&tfac,IMREF,bldg_ptr,phsun);

									/* Sun Position Azimuth Loop */
									for (iths=0; iths<sun_ptr->nths; iths++) {

										/* azm of sun in strange sun coord sys (0=East, counter-clockwise is positive) */
										thsun = (thsmin + (double)iths * thsdel - 90.0) * DTOR + bldg_ptr->azm * DTOR;

										/* Add contribution of current wndo element to */
										/* direct luminance at current surface node, */
										/* for current sky condition. */
										wndo_element_surfnode_lum_contrib(bldg_ptr,		/* pointer to bldg structure */
																lib_ptr,		/* pointer to lib structure */
																izone,			/* current zone index */
																isurf,			/* current surface index for Surface containing Window */
																iIntSurf,		/* current surface index for Surface containing Node */
																iw,				/* current window index */
															    iWndoElement,	/* window element index */
																thsun,			/* sun azm (radians) */
																phsun,			/* sun alt (radians) */
																thray,			/* sky element azm angle */
																phray,			/* sky element alt angle */
																iphs,			/* sun alt index */
																iths,			/* sun azm index */
																solic,			/* extraterrestrial irrad for 1st of each month (0 to 11) */
																node,			/* coords of surfnode */
																nodesurfnormal,	/* INWARD normal unit vector from face of surfnode */
																dNodeSurfaceReflectance, // visible reflectance of node surface
																cosPtSurfIncidence,	/* cos(angle of incidence) of vector from surfnode (on surface plane) to wndo element */
																&hit,			/* hit structure pointer for ray from surfnode to wndo element */
																domega,			/* solid angle subtended by window element wrt surfnode */
																vis_trans,		/* tvis of glass for normal incidence angle */
																tvisincidence,	/* tvis of glass for current incidence angle */
																wnorm,			/* window outward normal vector */
																tfac,			/* turbidity factor */
																zenl,			/* zenith luminance */
																// return values stored in surf node, or wndo node substructure
																&(bldg_ptr->zone[izone]->surf[iIntSurf]->direct_skyclum[inode][iphs][iths]),	/* ptr to direct luminance from sky - clear */
																&(bldg_ptr->zone[izone]->surf[iIntSurf]->direct_sunclum[inode][iphs][iths]),	/* ptr to direct luminance from sun - clear */
																&(bldg_ptr->zone[izone]->surf[iIntSurf]->direct_skyolum[inode]),				/* ptr to direct luminance from sky - overcast */
																pofdmpfile);	/* ptr to LBLDLL error dump file */

									}	/* end of Sun Position Azimuth Loop */

								}	/* end of Sun Position Altitude Loop */

							}	/* end of Surface Nodal Patch Loop */

						}	/* end of Interior Surface Loop */

//					}	/* end of y division Window Element Loop */

//				}	/* end of x division Window Element Loop */

				}	/* end of new Window Element Loop */

                // Now remesh this window using WLC meshing method
				// based on the user input max grid node area for interreflection calcs
                bldg_ptr->zone[izone]->surf[isurf]->wndo[iw]->WLCWNDOInit(bldg_ptr->zone[izone]->max_grid_node_area);
			}	/* end of Window Loop */

			/* CFS Surface Loop */
			for (int icfs = 0; icfs < bldg_ptr->zone[izone]->surf[isurf]->ncfs; icfs++) {

                // Get the CFSSystem for this CFSSurface
                CFSSystem* pCFSSystem4CFSSurf = NULL;
                string sCFSSystemType = bldg_ptr->zone[izone]->surf[isurf]->cfs[icfs]->TypeName();
                // Search for the CFSSystem of this Type associated with the current Surface
                for (int iCFSSys = 0; iCFSSys < (int)bldg_ptr->zone[izone]->surf[isurf]->vpCFSSystem.size(); iCFSSys++) {
                    if (bldg_ptr->zone[izone]->surf[isurf]->vpCFSSystem[iCFSSys]->TypeName() == sCFSSystemType) {
                        pCFSSystem4CFSSurf = bldg_ptr->zone[izone]->surf[isurf]->vpCFSSystem[iCFSSys];
                        break;
                    }
                }
                // Output failure to find CFSSystem for this CFSSurface
                if (!pCFSSystem4CFSSurf) {
				    *pofdmpfile << "ERROR: DElight No CFSSystem of Type " << sCFSSystemType << " found for Surface " << bldg_ptr->zone[izone]->surf[isurf]->name << "\n";
		            return -1;
                }

                // First distribute the Overcast Sky contribution to all interior surface nodes and to reference points

                // Create an Overcast Sky and CFSSystem Luminance Map for the current CFSSurface
                // HemiSphiral resolution
	            int sphiralM = 200;
	            int sphiralN = 1000;
                // Create the DNA string for CIE Overcast Sky
                // "SKY^GEN^CIEOVERCASTSKY^SunAlt^GrndRefl"
				// For 0th sun position altitude
				phsun_deg = phsmin;
                char cSkyStr[MAX_CHAR_LINE];
	            strcpy(cSkyStr,"");
	            snprintf(cSkyStr, 250, "SKY^GEN^CIEOVERCASTSKY^%6.2lf^%4.2lf", phsun_deg, bldg_ptr->zone[izone]->surf[isurf]->gnd_refl);
	            string	skyStr = cSkyStr;
                // Decode the DNA string
	            LumParam lpsky;
	            if (!SecretDecoderRing(lpsky,skyStr)) {
				    *pofdmpfile << "ERROR: DElight Incorrect Sky Generation Parameter - " << lpsky.BadName << "\n";
		            return -1;
	            }
                // Generate the Overcast Sky
//	            HemiSphiral	skyOvercast = GenSky(sphiralN, lpsky);
				// Set the hemisphiral resolutions
				lpsky.btdfHSResIn = sphiralM;
				lpsky.btdfHSResOut = sphiralN;
	            HemiSphiral	skyOvercast = GenSky(lpsky);
	            if (skyOvercast.size() == 0) {
				    *pofdmpfile << "ERROR: DElight HemiSphiral for Overcast Sky Size == ZERO\n";
		            return -1;
	            }

// Progress indicator
//    cout << ".";
//    cout << "Overcast Sky\n";
//	skyOvercast.plotview(75);
//	skyOvercast.plotview(75,DegToRad(0),DegToRad(0),DegToRad(0));

                // Create the CFSSystem Luminance Map for this sky type
//	            HemiSphiral	LumMapOvercast = pCFSSystem4CFSSurf->CFSLuminanceMap(sphiralM, sphiralN, skyOvercast, BGL::RHCoordSys3(bldg_ptr->zone[izone]->surf[isurf]->icsAxis(0),bldg_ptr->zone[izone]->surf[isurf]->icsAxis(1),bldg_ptr->zone[izone]->surf[isurf]->icsAxis(2)));
	            HemiSphiral	LumMapOvercast = pCFSSystem4CFSSurf->CFSLuminanceMap(skyOvercast, BGL::RHCoordSys3(bldg_ptr->zone[izone]->surf[isurf]->icsAxis(0),bldg_ptr->zone[izone]->surf[isurf]->icsAxis(1),bldg_ptr->zone[izone]->surf[isurf]->icsAxis(2)));
	            if (LumMapOvercast.size() == 0) {
				    *pofdmpfile << "ERROR: DElight HemiSphiral CFS Luminance Map Size == ZERO\n";
		            return -1;
	            }
                // Reset the current CFSSurface Luminance Map
                bldg_ptr->zone[izone]->surf[isurf]->cfs[icfs]->ResetLumMap(LumMapOvercast);

                // Now loop over all Surface Nodes adding Overcast Sky component
				/* Interior Surface Loop */
				for (iIntSurf=0; iIntSurf<bldg_ptr->zone[izone]->nsurfs; iIntSurf++) {

					// Skip the current surface.
					if (iIntSurf == isurf) continue;

					// Get the visible reflectance of this surface.
					double dNodeSurfaceReflectance = bldg_ptr->zone[izone]->surf[iIntSurf]->vis_refl;

					// Get surface inward normal unit vector and transfer to Vec3d.
					BGL::vector3 v3SurfNormal(bldg_ptr->zone[izone]->surf[iIntSurf]->inward_uvect[0],
						bldg_ptr->zone[izone]->surf[iIntSurf]->inward_uvect[1],
						bldg_ptr->zone[izone]->surf[iIntSurf]->inward_uvect[2]);

					/* Surface Nodal Patch Loop */
					for (inode=0; inode<bldg_ptr->zone[izone]->surf[iIntSurf]->nnodes; inode++) {

						// Get nodal patch coords and transfer to Point3d.
						BGL::point3 p3Node(bldg_ptr->zone[izone]->surf[iIntSurf]->node[inode][0],
							bldg_ptr->zone[izone]->surf[iIntSurf]->node[inode][1],
							bldg_ptr->zone[izone]->surf[iIntSurf]->node[inode][2]);

						// Get surface nodal patch area.
						double dSurfNodeArea = bldg_ptr->zone[izone]->surf[iIntSurf]->node_areas[inode];

						// Get the Overcast Sky illuminance on this node from the current CFS.
						double dCFSTotalIllum = bldg_ptr->zone[izone]->surf[isurf]->cfs[icfs]->TotRefPtIllum(v3SurfNormal, p3Node) * dSurfNodeArea;

                        // Add this Overcast Sky illuminance to the Surface Total Direct Illuminance from Overcast sky
			            bldg_ptr->zone[izone]->surf[iIntSurf]->TotDirectOvercastIllum += dCFSTotalIllum;

                        // Add the resulting Luminance from this CFS contribution to the Surface Node
                        // Note that Luminance is Illuminance * Surface Reflectance
			            bldg_ptr->zone[izone]->surf[iIntSurf]->direct_skyolum[inode] += dCFSTotalIllum * dNodeSurfaceReflectance;

					}	/* end of Surface Nodal Patch Loop */

				}	/* end of Interior Surface Loop */

                // Now loop over all Reference Points adding Overcast Sky component

				/* Set ref_pt "surface" normal unit vector (all ref_pts assumed horizontal facing upward). */
				BGL::vector3 v3RefPtNormal(0.0,0.0,1.0);

				/* Reference Point Loop */
				for (irp=0; irp<bldg_ptr->zone[izone]->nrefpts; irp++) {

					// Get refpt coords and transfer to Point3d.
					BGL::point3 p3RefPt(bldg_ptr->zone[izone]->ref_pt[irp]->bs[0],
						bldg_ptr->zone[izone]->ref_pt[irp]->bs[1],
						bldg_ptr->zone[izone]->ref_pt[irp]->bs[2]);

					// Get the Overcast Sky illuminance at this refpt from the current CFS.
					double dCFSTotalIllum = bldg_ptr->zone[izone]->surf[isurf]->cfs[icfs]->TotRefPtIllum(v3RefPtNormal, p3RefPt);
					bldg_ptr->zone[izone]->ref_pt[irp]->direct_skyoillum += dCFSTotalIllum;

				}	/* end of Reference Point Loop */

                // Next distribute the Clear Direct Sun contribution to all interior surface nodes and to reference points
                // for all Sun Positions

				/* Sun Position Altitude Loop */
				for (iphs=0; iphs<sun_ptr->nphs; iphs++) {

					/* Altitude of sun */
					phsun_deg = phsmin + (double)iphs * phsdel;
					phsun = phsun_deg * DTOR;

					/* Get clear sky zenith luminance, moisture, */
					/* and turbidity coef for reference month. */
					dzenlm(&zenl,&tfac,IMREF,bldg_ptr,phsun);

                    /* Sun Position Azimuth Loop */
					for (iths=0; iths<sun_ptr->nths; iths++) {

						/* azm of sun in strange sun coord sys (0=East, counter-clockwise is positive) */
						thsun = (thsmin + (double)iths * thsdel - 90.0) * DTOR + bldg_ptr->azm * DTOR;

                        // Create a Clear Sun sky and CFSSystem Luminance Map for the current Sun Position and CFSSurface
                        // HemiSphiral resolution
	                    sphiralM = 200;
	                    sphiralN = 2000; // BUT also see below for high altitudes
                        // Create the DNA string for CIE Clear Sun sky
                        // "SKY^GEN^CIECLEARSUN^SunAlt^SunAzm^Solic^TFac^AtmMoi^AtmTurb^BldgAlt^GrndRefl"
				        // For current sun position
                        // Convert Aziumth angle in radians to degrees
                        // and make sure it falls within -180 to 180
                        double thsun_deg = thsun/DTOR;
                        if (thsun_deg < -180.) thsun_deg += 360.;
                        if (thsun_deg > 180.) thsun_deg -= 360.;
                        if (phsun_deg > 60) {
                            sphiralN = 6000;
                            if (thsun_deg > 150.) {
                                sphiralN = 10000;
                            }
                        }
	                    strcpy(cSkyStr,"");
	                    snprintf(cSkyStr,250, "SKY^GEN^CIECLEARSUN^%6.2lf^%6.2lf^%10.4lf^%10.4lf^%6.2lf^%6.2lf^%8.2lf^%4.2lf", phsun_deg, thsun_deg, solic[IMREF], tfac, bldg_ptr->atmmoi[IMREF], bldg_ptr->atmtur[IMREF], bldg_ptr->alt, bldg_ptr->zone[izone]->surf[isurf]->gnd_refl);
	                    string	skyStr = cSkyStr;
//    cout << skyStr << "\n";
                        // Decode the DNA string
	                    if (!SecretDecoderRing(lpsky,skyStr)) {
				            *pofdmpfile << "ERROR: DElight Incorrect Sky Generation Parameter - " << lpsky.BadName << "\n";
		                    return -1;
	                    }
//    lpsky.Dump();
                        // Generate the sky
//	                    HemiSphiral	skyClearSun = GenSky(sphiralN, lpsky);
						// Set the hemisphiral resolutions
						lpsky.btdfHSResIn = sphiralM;
						lpsky.btdfHSResOut = sphiralN;
	                    HemiSphiral	skyClearSun = GenSky(lpsky);
	                    if (skyClearSun.size() == 0) {
				            *pofdmpfile << "ERROR: DElight HemiSphiral for Clear Sun Size == ZERO\n";
		                    return -1;
	                    }
// Progress indicator
//    cout << ".";
//    cout << "Clear Sun: Alt = " << phsun_deg << " Azm = " << RadToDeg(thsun) << "\n";
//	skyClearSun.plotview(75);
//	skyClearSun.plotview(75,DegToRad(0),DegToRad(90),DegToRad(0));

                        // Create the new CFSSystem Luminance Map for this sky type
//	                    HemiSphiral	LumMapClearSun = pCFSSystem4CFSSurf->CFSLuminanceMap(sphiralM, sphiralN, skyClearSun, BGL::RHCoordSys3(bldg_ptr->zone[izone]->surf[isurf]->icsAxis(0),bldg_ptr->zone[izone]->surf[isurf]->icsAxis(1),bldg_ptr->zone[izone]->surf[isurf]->icsAxis(2)));
	                    HemiSphiral	LumMapClearSun = pCFSSystem4CFSSurf->CFSLuminanceMap(skyClearSun, BGL::RHCoordSys3(bldg_ptr->zone[izone]->surf[isurf]->icsAxis(0),bldg_ptr->zone[izone]->surf[isurf]->icsAxis(1),bldg_ptr->zone[izone]->surf[isurf]->icsAxis(2)));
	                    if (LumMapClearSun.size() == 0) {
				            *pofdmpfile << "ERROR: DElight HemiSphiral for CFS Luminance Map Size == ZERO\n";
		                    return -1;
	                    }
                        // Reset the CFSSurface Luminance Map
                        bldg_ptr->zone[izone]->surf[isurf]->cfs[icfs]->ResetLumMap(LumMapClearSun);

                        // Now loop over all Surface Nodes adding Clear Sun component for current Sun Position

                        /* Interior Surface Loop */
						for (iIntSurf=0; iIntSurf<bldg_ptr->zone[izone]->nsurfs; iIntSurf++) {

							// Skip the current surface.
							if (iIntSurf == isurf) continue;

					        // Get the visible reflectance of this surface.
					        double dNodeSurfaceReflectance = bldg_ptr->zone[izone]->surf[iIntSurf]->vis_refl;

							// Get surface inward normal unit vector and transfer to Vec3d.
							BGL::vector3 v3SurfNormal(bldg_ptr->zone[izone]->surf[iIntSurf]->inward_uvect[0],
								bldg_ptr->zone[izone]->surf[iIntSurf]->inward_uvect[1],
								bldg_ptr->zone[izone]->surf[iIntSurf]->inward_uvect[2]);

							/* Surface Nodal Patch Loop */
							for (inode=0; inode<bldg_ptr->zone[izone]->surf[iIntSurf]->nnodes; inode++) {

								// Get nodal patch coords and transfer to Point3d.
								BGL::point3 p3Node(bldg_ptr->zone[izone]->surf[iIntSurf]->node[inode][0],
									bldg_ptr->zone[izone]->surf[iIntSurf]->node[inode][1],
									bldg_ptr->zone[izone]->surf[iIntSurf]->node[inode][2]);

							    // Get surface nodal patch area.
							    double dSurfNodeArea = bldg_ptr->zone[izone]->surf[iIntSurf]->node_areas[inode];

								// Get the illuminance on this node from the current CFS.
							    // Illuminance from sun - clear
								double dCFSTotalIllum = bldg_ptr->zone[izone]->surf[isurf]->cfs[icfs]->TotRefPtIllum(v3SurfNormal, p3Node) * dSurfNodeArea;

                                // Accumulate the Surface Total Direct Illuminance from Clear Sun for current sun position
								bldg_ptr->zone[izone]->surf[iIntSurf]->TotDirectSunCIllum[iphs][iths] += dCFSTotalIllum;

                                // Add the resulting Luminance from this CFS contribution to the Surface Node
                                // Note that Luminance is Illuminance * Surface Reflectance
								bldg_ptr->zone[izone]->surf[iIntSurf]->direct_sunclum[inode][iphs][iths] += dCFSTotalIllum * dNodeSurfaceReflectance;

                            }	/* end of Surface Nodal Patch Loop */

						}	/* end of Interior Surface Loop */

                        // Now loop over all Reference Points adding Clear Sun component for current sun position

						/* Reference Point Loop */

						/* Set ref_pt "surface" normal unit vector (all ref_pts assumed horizontal facing upward). */
						BGL::vector3 v3RefPtNormal(0.0,0.0,1.0);

						for (irp=0; irp<bldg_ptr->zone[izone]->nrefpts; irp++) {

							// Get refpt coords and transfer to Point3d.
							BGL::point3 p3RefPt(bldg_ptr->zone[izone]->ref_pt[irp]->bs[0],
								bldg_ptr->zone[izone]->ref_pt[irp]->bs[1],
								bldg_ptr->zone[izone]->ref_pt[irp]->bs[2]);

							// Get the illuminance at this refpt from the current CFS.
							// Illuminance from sun - clear
							double dCFSTotalIllum = bldg_ptr->zone[izone]->surf[isurf]->cfs[icfs]->TotRefPtIllum(v3RefPtNormal, p3RefPt);
							bldg_ptr->zone[izone]->ref_pt[irp]->direct_suncillum[iphs][iths] += dCFSTotalIllum;

						}	/* end of Reference Point Loop */

					}	/* end of Sun Position Azimuth Loop */

				}	/* end of Sun Position Altitude Loop */

                // Lastly, distribute the Clear Sky contribution to all interior surface nodes and to reference points
                // for all Sun Positions

				/* Sun Position Altitude Loop */
				for (iphs=0; iphs<sun_ptr->nphs; iphs++) {

					/* Altitude of sun */
					phsun_deg = phsmin + (double)iphs * phsdel;
					phsun = phsun_deg * DTOR;

					/* Get clear sky zenith luminance, moisture, */
					/* and turbidity coef for reference month. */
					dzenlm(&zenl,&tfac,IMREF,bldg_ptr,phsun);

                    /* Sun Position Azimuth Loop */
					for (iths=0; iths<sun_ptr->nths; iths++) {

						/* azm of sun in strange sun coord sys (0=East, counter-clockwise is positive) */
						thsun = (thsmin + (double)iths * thsdel - 90.0) * DTOR + bldg_ptr->azm * DTOR;

                        // Create a Clear Sky and CFSSystem Luminance Map for the current Sun Position and CFSSurface
                        // "SKY^GEN^CIECLEARSKY^SunAlt^SunAzm^ZenLum^GrndRefl"
                        // HemiSphiral resolution
	                    sphiralM = 200;
	                    sphiralN = 1000;
                        // Create the DNA string for CIE Clear Sky
				        // For current sun position
                        // Convert Aziumth angle in radians to degrees
                        // and make sure it falls within -180 to 180
                        double thsun_deg = thsun/DTOR;
                        if (thsun_deg < -180.) thsun_deg += 360.;
                        if (thsun_deg > 180.) thsun_deg -= 360.;
	                    strcpy(cSkyStr,"");
	                    snprintf(cSkyStr,250,"SKY^GEN^CIECLEARSKY^%6.2lf^%6.2lf^%10.6lf^%4.2lf", phsun_deg, thsun_deg, zenl, bldg_ptr->zone[izone]->surf[isurf]->gnd_refl);
	                    string	skyStr = cSkyStr;
                        // Decode the DNA string
	                    if (!SecretDecoderRing(lpsky,skyStr)) {
				            *pofdmpfile << "ERROR: DElight Incorrect Sky Generation Parameter - " << lpsky.BadName << "\n";
		                    return -1;
	                    }
                        // Generate the sky
//	                    HemiSphiral	skyClear = GenSky(sphiralN, lpsky);
						// Set the hemisphiral resolutions
						lpsky.btdfHSResIn = sphiralM;
						lpsky.btdfHSResOut = sphiralN;
	                    HemiSphiral	skyClear = GenSky(lpsky);
	                    if (skyClear.size() == 0) {
				            *pofdmpfile << "ERROR: DElight HemiSphiral for Clear Sky Size == ZERO\n";
		                    return -1;
	                    }
// Progress indicator
//    cout << ".";
//    cout << "Clear Sky: Alt = " << phsun_deg << " Azm = " << RadToDeg(thsun) << "\n";
//	skyClear.plotview(75);
//	skyClear.plotview(75,DegToRad(0),DegToRad(90),DegToRad(0));

                        // Create the new CFSSystem Luminance Map for this sky type
//	                    HemiSphiral	LumMapClear = pCFSSystem4CFSSurf->CFSLuminanceMap(sphiralM, sphiralN, skyClear, BGL::RHCoordSys3(bldg_ptr->zone[izone]->surf[isurf]->icsAxis(0),bldg_ptr->zone[izone]->surf[isurf]->icsAxis(1),bldg_ptr->zone[izone]->surf[isurf]->icsAxis(2)));
	                    HemiSphiral	LumMapClear = pCFSSystem4CFSSurf->CFSLuminanceMap(skyClear, BGL::RHCoordSys3(bldg_ptr->zone[izone]->surf[isurf]->icsAxis(0),bldg_ptr->zone[izone]->surf[isurf]->icsAxis(1),bldg_ptr->zone[izone]->surf[isurf]->icsAxis(2)));
	                    if (LumMapClear.size() == 0) {
				            *pofdmpfile << "ERROR: DElight HemiSphiral for CFS Luminance Map Size == ZERO\n";
		                    return -1;
	                    }
                        // Reset the CFSSurface Luminance Map
                        bldg_ptr->zone[izone]->surf[isurf]->cfs[icfs]->ResetLumMap(LumMapClear);

                        // Now loop over all Surface Nodes adding Clear Sky component for current Sun Position

                        /* Interior Surface Loop */
						for (iIntSurf=0; iIntSurf<bldg_ptr->zone[izone]->nsurfs; iIntSurf++) {

							// Skip the current surface.
							if (iIntSurf == isurf) continue;

					        // Get the visible reflectance of this surface.
					        double dNodeSurfaceReflectance = bldg_ptr->zone[izone]->surf[iIntSurf]->vis_refl;

							// Get surface inward normal unit vector and transfer to Vec3d.
							BGL::vector3 v3SurfNormal(bldg_ptr->zone[izone]->surf[iIntSurf]->inward_uvect[0],
								bldg_ptr->zone[izone]->surf[iIntSurf]->inward_uvect[1],
								bldg_ptr->zone[izone]->surf[iIntSurf]->inward_uvect[2]);

							/* Surface Nodal Patch Loop */
							for (inode=0; inode<bldg_ptr->zone[izone]->surf[iIntSurf]->nnodes; inode++) {

								// Get nodal patch coords and transfer to Point3d.
								BGL::point3 p3Node(bldg_ptr->zone[izone]->surf[iIntSurf]->node[inode][0],
									bldg_ptr->zone[izone]->surf[iIntSurf]->node[inode][1],
									bldg_ptr->zone[izone]->surf[iIntSurf]->node[inode][2]);

							    // Get surface nodal patch area.
							    double dSurfNodeArea = bldg_ptr->zone[izone]->surf[iIntSurf]->node_areas[inode];

								// Get the illuminance on this node from the current CFS.
								// Illuminance from sky - clear
								double dCFSTotalIllum = bldg_ptr->zone[izone]->surf[isurf]->cfs[icfs]->TotRefPtIllum(v3SurfNormal, p3Node) * dSurfNodeArea;

                                // Accumulate the Surface Total Direct Illuminance from Clear Sky for current sun position
								bldg_ptr->zone[izone]->surf[iIntSurf]->TotDirectSkyCIllum[iphs][iths] += dCFSTotalIllum;

                                // Add the resulting Luminance from this CFS contribution to the Surface Node
                                // Note that Luminance is Illuminance * Surface Reflectance
								bldg_ptr->zone[izone]->surf[iIntSurf]->direct_skyclum[inode][iphs][iths] += dCFSTotalIllum * dNodeSurfaceReflectance;

                            }	/* end of Surface Nodal Patch Loop */

						}	/* end of Interior Surface Loop */

                        // Now loop over all Reference Points adding Clear Sky component for current sun position

						/* Reference Point Loop */

						/* Set ref_pt "surface" normal unit vector (all ref_pts assumed horizontal facing upward). */
						BGL::vector3 v3RefPtNormal(0.0,0.0,1.0);

						for (irp=0; irp<bldg_ptr->zone[izone]->nrefpts; irp++) {

							// Get refpt coords and transfer to Point3d.
							BGL::point3 p3RefPt(bldg_ptr->zone[izone]->ref_pt[irp]->bs[0],
								bldg_ptr->zone[izone]->ref_pt[irp]->bs[1],
								bldg_ptr->zone[izone]->ref_pt[irp]->bs[2]);

							// Get the illuminance at this refpt from the current CFS.
							// Illuminance from sky - clear
							double dCFSTotalIllum = bldg_ptr->zone[izone]->surf[isurf]->cfs[icfs]->TotRefPtIllum(v3RefPtNormal, p3RefPt);
							bldg_ptr->zone[izone]->ref_pt[irp]->direct_skycillum[iphs][iths] += dCFSTotalIllum;

						}	/* end of Reference Point Loop */

					}	/* end of Sun Position Azimuth Loop */

				}	/* end of Sun Position Altitude Loop */

			}	/* end of CFS Surface Loop */

		}	/* end of Exterior Surface Loop */

    // Progress indicator termination
 //   cout << "\n";

		// Interreflection Calcs
        int iSliteInterRflRetVal;
		if ((iSliteInterRflRetVal = slite_interreflect(bldg_ptr, lib_ptr, sun_ptr, iIterations, pofdmpfile)) < 0) {
            // If errors were detected then return now, else register warnings and continue processing
            if (iSliteInterRflRetVal != -10) {
				*pofdmpfile << "ERROR: DElight Bad return from slite_interreflect()\n";
				return(-1);
            }
            else {
                iReturnVal = -10;
            }
        }

        // Daylight Factor Calcs
		// for each ref_pt in this zone
		for (irp=0; irp<bldg_ptr->zone[izone]->nrefpts; irp++) {
			// Calc daylight factor for overcast sky condition
			if (bldg_ptr->hillumskyo[0])
				bldg_ptr->zone[izone]->ref_pt[irp]->dfskyo = bldg_ptr->zone[izone]->ref_pt[irp]->skyoillum / bldg_ptr->hillumskyo[0];
			// for each Sun Position Altitude
			for (iphs=0; iphs<sun_ptr->nphs; iphs++) {
				// for each Sun Position Azimuth
				for (iths=0; iths<sun_ptr->nths; iths++) {
					// Calc component daylight factors for each clear sky sun position
					if (bldg_ptr->hillumskyc[iphs])
						bldg_ptr->zone[izone]->ref_pt[irp]->dfsky[iphs][iths] = bldg_ptr->zone[izone]->ref_pt[irp]->skycillum[iphs][iths] / bldg_ptr->hillumskyc[iphs];
					if (bldg_ptr->hillumsunc[iphs])
						bldg_ptr->zone[izone]->ref_pt[irp]->dfsun[iphs][iths] = bldg_ptr->zone[izone]->ref_pt[irp]->suncillum[iphs][iths] / bldg_ptr->hillumsunc[iphs];
				}
			}
		}
	}	/* end of Lighting Zone Loop */

	return(iReturnVal);
}

/************************** subroutine wndo_element_refpt_illum_contrib *************************/
/* Adds contribution of current window element to direct (initial) illuminance (lm/ft2) at */
/* current reference point. */

/* Modified from earlier version of wndo_element_contrib() */
/*   Key modifications are: */
/*       - remove glare related calculations */
/*       - replace references to ray[2] (i.e., cos(angle of incidence) for horiz surf) by cosSurfIncidence */
/*   8/2000 modifications: */
/*       - separate reference point illuminance calc from surface node luminance calculation */
/************************************************************************************************/
/* C Language Implementation based on modified DOE2 Daylighting Algorithms */
/* by Rob Hitchcock */
/* Building Technologies Program, Lawrence Berkeley Laboratory */
/************************** subroutine wndo_element_refpt_illum_contrib *************************/
int	wndo_element_refpt_illum_contrib(
	BLDG *bldg_ptr,		/* pointer to bldg structure */
	LIB *lib_ptr,		/* pointer to lib structure */
	int izone,			/* current zone index */
	int iWndoSurf,		/* current index for surface containing Window */
	int iNodeSurf,		/* current index for surface containing Node, Not Applicable (iWndoSurf==iNodeSurf */
	int iwndo,			/* current window index */
	int iWndoElement,	/* window element index */
	double thsun,		/* sun azm (radians) */
	double phsun,		/* sun alt (radians) */
	double thray,		/* sky element azm angle */
	double phray,		/* sky element alt angle */
	int iphs,			/* sun alt index */
	int iths,			/* sun azm index */
	double solic[MONTHS],	/* extraterrestrial irrad for 1st of each month (0 to 11) */
	double node[NCOORDS],	/* coords of refpt */
	double nodesurfnormal[NCOORDS],	/* INWARD normal unit vector from face of refpt */
	double cosPtSurfIncidence,	/* cos(angle of incidence) of vector from refpt (on horiz plane) to wndo element */
	HIT *hit_ptr,			/* hit structure pointer for ray from refpt to wndo element */
	double domega,			/* solid angle subtended by window element wrt ref_pt */
	double vis_trans,		/* tvis of glass for normal incidence angle */
	double tvisincidence,	/* tvis of glass for incidence angle */
	double wnorm[NCOORDS],	/* window outward normal vector */
	double tfac,			/* turbidity factor */
	double zenl,			/* zenith luminance */
	// return values stored in ref pt substructure
	double *pdirect_skycillum,	/* ptr to direct illuminance from sky (fc) - clear */
	double *pdirect_suncillum,	/* ptr to direct illuminance from sun (fc) - clear */
	double *pdirect_skyoillum,	/* ptr to direct illuminance from sky (fc) - overcast */
	ofstream* pofdmpfile)		/* ptr to LBLDLL error dump file */
{
	double elum, dedir;		/* luminance calc vars */
	double raycos[NCOORDS];	/* unit vector to sun from anywhere in the bldg */
	double cosi;			/* cos(incidence) of raycos onto wndo */
	double tviss;			/* vis trans for angle of incidence of raycos through wndo */
	double tvis1;			/* dummy vis trans for diffuse glazing calc using wndo luminance (==1.0) */
	double dnsoli;			/* dnsol() return value */
	HIT rchit;				/* bldg-shade hit structure for dhitsh() */

	/* CASE 1 - Window without shades (i.e., clear glazing) */
	if (bldg_ptr->zone[izone]->surf[iWndoSurf]->wndo[iwndo]->shade_flag == 0) {

		/* If ray hits front of global shading surface, */
		/* add contrib of shading surf luminance (cd/ft2) */
		/* to illum at ref_pt.  */
		if (hit_ptr->ihit == 4) {
			(*pdirect_skycillum) += bldg_ptr->bshade[hit_ptr->hitshade]->skylum[iphs][iths]*domega*tvisincidence*cosPtSurfIncidence;
			(*pdirect_suncillum) += bldg_ptr->bshade[hit_ptr->hitshade]->sunlum[iphs][iths]*domega*tvisincidence*cosPtSurfIncidence;
			if ((iphs ==0) && (iths ==0))
				(*pdirect_skyoillum) += bldg_ptr->bshade[hit_ptr->hitshade]->ovrlum*domega*tvisincidence*cosPtSurfIncidence;
		}

		/* If ray hits exterior of zone surface, */
		/* add contrib of zone surface luminance (cd/ft2) */
		/* to illum at ref_pt.  */
		if (hit_ptr->ihit == 2) {
			(*pdirect_skycillum) += bldg_ptr->zone[hit_ptr->hitzone]->surf[hit_ptr->hitshade]->skylum[iphs][iths]*domega*tvisincidence*cosPtSurfIncidence;
			(*pdirect_suncillum) += bldg_ptr->zone[hit_ptr->hitzone]->surf[hit_ptr->hitshade]->sunlum[iphs][iths]*domega*tvisincidence*cosPtSurfIncidence;
			if ((iphs ==0) && (iths ==0))
				(*pdirect_skyoillum) += bldg_ptr->zone[hit_ptr->hitzone]->surf[hit_ptr->hitshade]->ovrlum*domega*tvisincidence*cosPtSurfIncidence;
		}

		/* If shading surface not hit, add contrib of sky (cd/ft2) if it is visible from ref pt (phray > 0.0). */

		// WHAT ABOUT GROUND HITS (PHRAY <= 0)?
		// NOTE: PHRAY is z dimension of unit vector of ray from ref pt through window element
		// NOTE: Ray from ref pt (horiz looking up) can never see the ground, so ignore

		if (hit_ptr->ihit == 0) {

			/* for clear sky */
			if (phray > 0.0) {
				elum = dskylu(0,thray,phray,thsun,phsun,zenl);
				dedir = elum * domega * tvisincidence * cosPtSurfIncidence;
				(*pdirect_skycillum) += dedir;
			}

			/* for overcast sky */
			if ((iphs == 0) && (iths == 0)) {
				if (phray > 0.0) {
					elum = dskylu(1,thray,phray,thsun,phsun,zenl);
					dedir = elum * domega * tvisincidence * cosPtSurfIncidence;

// rjh 8/00
//	*pofdmpfile << "Overcast sky vars for iWndoSurf " << iWndoSurf << " iNodeSurf " << iNodeSurf << " iWndoElement(" << ix << ", " << iy << ") Node(" << node[0] << ", " << node[1] << ", " << node[2] << ")\n";
//	*pofdmpfile << "   elum = " << elum << " domega = " << domega << " tvisincidence = " << tvisincidence << " cosPtSurfIncidence = " << cosPtSurfIncidence << "\n";


					(*pdirect_skyoillum) += dedir;
				}
			}
		}

		/* Illuminance from (unreflected) direct sun. */
		/* (calculated only once per wndo for each ref pt) */
//		if ((ix == (nwx-1)) && (iy == (nwy-1))) {
		if (iWndoElement == 0) {
			/* unit vector to sun from anywhere in the bldg */
			raycos[0] = cos(phsun) * cos(thsun);
			raycos[1] = cos(phsun) * sin(thsun);
			raycos[2] = sin(phsun);
			/* is sun on front side of current window? */
			cosi = ddot(wnorm,raycos);
			if (cosi > 0.0) {
				/* does raycos from current ref_pt pass thru window? */
//				dpierc(&ip,w1,w2,w3,node,raycos);

                // Use WLC ray intersect surface polygon method.
                // Note that this tests ray intersecting surface face on the INSIDE of DOE2 convention surface
                // So this only works for rays from nodal points intersecting Window Surface polygons from inside zone.
                BGL::point3	pt3Node = BGL::point3(node[0], node[1], node[2]);	//	center of box
                BGL::vector3 vRayDir = BGL::vector3(raycos[0], raycos[1], raycos[2]);
                BGL::ray3	r3Ray(pt3Node,vRayDir);
                bool bIntersects = false;
                Double dParam;  // can be used to calculate intersection point
                bIntersects = bldg_ptr->zone[izone]->surf[iWndoSurf]->wndo[iwndo]->intersect(r3Ray,dParam);

				if (bIntersects) {
//				if (ip != 0) {
					/* does raycos from current ref_pt intercept shade? */
					/* NOTE: this includes all zone surfaces contrary to DOE2 check of */
					/* only "self-shade" surfaces (in addition to zone and bldg shades). */
					// rjh 5/14/97 correction - parameter ray replaced with raycos
	//				dhitsh(&rchit,rref,ray,bldg_ptr,izone,isurf);
					dhitsh(&rchit,node,raycos,bldg_ptr,izone,iWndoSurf,iNodeSurf);
					if (rchit.ihit == 0) {
						/* sun reaches ref_pt */
						/* increment illuminance */

						// tvis of glass for cosi incid angle
						// Window 4 implementation 4/2002
						/* set lib_index component type to glass */
						char sLibCompType[MAX_CHAR_UNAME+1];	/* temp str */
						/* get library index of current window glass type */
						strcpy(sLibCompType,"glass");
						int igt = lib_index(lib_ptr,sLibCompType,bldg_ptr->zone[izone]->surf[iWndoSurf]->wndo[iwndo]->glass_type);
						int iGlass_Type_ID = atoi(bldg_ptr->zone[izone]->surf[iWndoSurf]->wndo[iwndo]->glass_type);
						// glass type ID: 1 to 11 => DOE2 original, >11 => W4lib.dat, <0 => E10 library
						if ((iGlass_Type_ID > 0) && (iGlass_Type_ID <= 11))  {	// DOE2 original

							// DOE2 original angular dependence equation coefs
							double cam1 = lib_ptr->glass[igt]->cam1;
							double cam2 = lib_ptr->glass[igt]->cam2;
							double cam3 = lib_ptr->glass[igt]->cam3;
							double cam4 = lib_ptr->glass[igt]->cam4;

							// tvis of glass for cosi incid angle
							tviss = max(0.0,(cam1+cosi*(cam2+cosi*(cam3+cosi*cam4))));
						}
						else if ((iGlass_Type_ID > 11) && (iGlass_Type_ID <= 10000)) {	// Window4

							// Window4 angular dependence Fit4() function coefs.
							double W4vis_fit1 = lib_ptr->glass[igt]->W4vis_fit1;
							double W4vis_fit2 = lib_ptr->glass[igt]->W4vis_fit2;

							// tvis of glass for cosi incid angle
							tviss = vis_trans * fit4(cosi, W4vis_fit1, W4vis_fit2);
						}
						else if (iGlass_Type_ID > 10000) {	// EnergyPlus/Window5

							// tvis of glass for cosi incid angle
							tviss = POLYF(cosi, lib_ptr->glass[igt]->EPlusCoef);
						}
						else if (iGlass_Type_ID < 0) {	// Energy-10

							/* Energy-10 angular dependence equation coefs. */
							double E10coef1 = lib_ptr->glass[igt]->E10coef[0];
							double E10coef2 = lib_ptr->glass[igt]->E10coef[1];
							double E10coef3 = lib_ptr->glass[igt]->E10coef[2];
							double E10coef4 = lib_ptr->glass[igt]->E10coef[3];

							// tvis of glass for cosi incid angle
							tviss = vis_trans * max(0.0,(cosi*(E10coef1+cosi*(E10coef2+cosi*(E10coef3+cosi*E10coef4)))));
						}

						/* calc direct normal illuminance (lumens/ft2) from solar disk */
						dnsoli = dnsol(solic,bldg_ptr,IMREF,phsun,tfac,pofdmpfile);
						if (dnsoli < 0.0) {
							*pofdmpfile << "ERROR: DElight Bad return from dnsol() = " <<dnsoli << ", exit wndo_element_refpt_illum_contrib()\n";
							return(-1);
						}

						/* calc cos(incidence) for raycos on ref_pt surface */
						double cosinc;
						cosinc = ddot(nodesurfnormal,raycos);

						/* add illuminance from sun modified by wndo vis trans and angle of incidence on ref_pt surface */
						(*pdirect_suncillum) += dnsoli*tviss*cosinc;
					}
				}
			}
		}
	}

	/* CASE 2 - Window with closed shades (i.e., diffuse glazing) */
	else { // (bldg_ptr->zone[izone]->surf[iWndoSurf]->wndo[iwndo]->shade_flag != 0)

		/* Luminance of wndo element is shade luminance. */
		tvis1 = 1.0;

		/* for clear sky */
		(*pdirect_skycillum) += bldg_ptr->zone[izone]->surf[iWndoSurf]->wndo[iwndo]->wlumsky[iphs][iths]*domega*tvis1*cosPtSurfIncidence;
		(*pdirect_suncillum) += bldg_ptr->zone[izone]->surf[iWndoSurf]->wndo[iwndo]->wlumsun[iphs][iths]*domega*tvis1*cosPtSurfIncidence;

		/* for overcast sky */
		if ((iphs == 0) && (iths == 0)) {
			(*pdirect_skyoillum) += bldg_ptr->zone[izone]->surf[iWndoSurf]->wndo[iwndo]->wlumskyo*domega*tvis1*cosPtSurfIncidence;
		}
	}

	return(0);
}

/************************** subroutine wndo_element_surfnode_lum_contrib *************************/
/* Adds contribution of current window element to direct (initial) luminance (cd/ft2) at */
/* current surface node. */

/* Modified from earlier version of wndo_element_contrib() */
/*   Key modifications are: */
/*       - remove glare related calculations */
/*       - replace references to ray[2] (i.e., cos(angle of incidence) for horiz surf) by cosSurfIncidence */
/*   8/2000 modifications: */
/*       - include visible reflectance of node surface in node luminance calculation */
/*************************************************************************************************/
/* C Language Implementation based on modified DOE2 Daylighting Algorithms */
/* by Rob Hitchcock */
/* Building Technologies Program, Lawrence Berkeley Laboratory */
/************************** subroutine wndo_element_surfnode_lum_contrib *************************/
int	wndo_element_surfnode_lum_contrib(
	BLDG *bldg_ptr,		/* pointer to bldg structure */
	LIB *lib_ptr,		/* pointer to lib structure */
	int izone,			/* current zone index */
	int iWndoSurf,		/* current index for surface containing Window */
	int iNodeSurf,		/* current index for surface containing Node */
	int iwndo,			/* current window index */
	int iWndoElement,	/* window element index */
	double thsun,		/* sun azm (radians) */
	double phsun,		/* sun alt (radians) */
	double thray,		/* sky element azm angle */
	double phray,		/* sky element alt angle */
	int iphs,			/* sun alt index */
	int iths,			/* sun azm index */
	double solic[MONTHS],	/* extraterrestrial irrad for 1st of each month (0 to 11) */
	double node[NCOORDS],	/* coords of surfnode */
	double nodesurfnormal[NCOORDS],	/* INWARD normal unit vector from face of surfnode */
	double dNodeSurfaceReflectance, // visible reflectance of node surface
	double cosPtSurfIncidence,	/* cos(angle of incidence) of vector from surfnode (on surface plane) to wndo element */
	HIT *hit_ptr,			/* hit structure pointer for ray from surfnode to wndo element */
	double domega,			/* solid angle subtended by window element wrt surfnode */
	double vis_trans,		/* tvis of glass for normal incidence angle */
	double tvisincidence,	/* tvis of glass for incidence angle */
	double wnorm[NCOORDS],	/* window outward normal vector */
	double tfac,			/* turbidity factor */
	double zenl,			/* zenith luminance */
	// return values stored in surf node, or wndo node substructure
	double *pdirect_skyclum,	/* ptr to direct luminance from sky - clear */
	double *pdirect_sunclum,	/* ptr to direct luminance from sun - clear */
	double *pdirect_skyolum,	/* ptr to direct luminance from sky - overcast */
	ofstream* pofdmpfile)		/* ptr to LBLDLL error dump file */
{
	double elum, dedir;		/* luminance calc vars */
	double dGroundLumSkyC, dGroundLumSunC, dGroundLumSkyO;		/* luminance calc vars */
	double raycos[NCOORDS];	/* unit vector to sun from anywhere in the bldg */
	double cosi;			/* cos(incidence) of raycos onto wndo */
	double tviss;			/* vis trans for angle of incidence of raycos through wndo */
	double tvis1;			/* dummy vis trans for diffuse glazing calc using wndo luminance (==1.0) */
	double dnsoli;			/* dnsol() return value */
	HIT rchit;				/* bldg-shade hit structure for dhitsh() */

	/* CASE 1 - Window without shades (i.e., clear glazing) */
	if (bldg_ptr->zone[izone]->surf[iWndoSurf]->wndo[iwndo]->shade_flag == 0) {

		/* If ray hits front of global shading surface, */
		/* add contrib of shading surf luminance (cd/ft2) */
		/* to luminance at surf node.  */
		if (hit_ptr->ihit == 4) {
			(*pdirect_skyclum) += bldg_ptr->bshade[hit_ptr->hitshade]->skylum[iphs][iths]*domega*tvisincidence*cosPtSurfIncidence * dNodeSurfaceReflectance;
			(*pdirect_sunclum) += bldg_ptr->bshade[hit_ptr->hitshade]->sunlum[iphs][iths]*domega*tvisincidence*cosPtSurfIncidence * dNodeSurfaceReflectance;
			if ((iphs ==0) && (iths ==0))
				(*pdirect_skyolum) += bldg_ptr->bshade[hit_ptr->hitshade]->ovrlum*domega*tvisincidence*cosPtSurfIncidence * dNodeSurfaceReflectance;
		}

		/* If ray hits exterior of zone surface, */
		/* add contrib of zone surface luminance (cd/ft2) */
		/* to luminance at surf node.  */
		if (hit_ptr->ihit == 2) {
			(*pdirect_skyclum) += bldg_ptr->zone[hit_ptr->hitzone]->surf[hit_ptr->hitshade]->skylum[iphs][iths]*domega*tvisincidence*cosPtSurfIncidence * dNodeSurfaceReflectance;
			(*pdirect_sunclum) += bldg_ptr->zone[hit_ptr->hitzone]->surf[hit_ptr->hitshade]->sunlum[iphs][iths]*domega*tvisincidence*cosPtSurfIncidence * dNodeSurfaceReflectance;
			if ((iphs ==0) && (iths ==0))
				(*pdirect_skyolum) += bldg_ptr->zone[hit_ptr->hitzone]->surf[hit_ptr->hitshade]->ovrlum*domega*tvisincidence*cosPtSurfIncidence * dNodeSurfaceReflectance;
		}

		/* If shading surface not hit, add contrib of */
		/* sky or ground luminance (cd/ft2). */
		if (hit_ptr->ihit == 0) {

			/* for clear sky */
			// If ray points upward above or along the horizontal plane, then add contribution from sky
			if (phray >= 0.0) {
				elum = dskylu(0,thray,phray,thsun,phsun,zenl);
				dedir = elum * domega * tvisincidence * cosPtSurfIncidence * dNodeSurfaceReflectance;
				(*pdirect_skyclum) += dedir;
			}
			// If ray points downward below the horizontal plane, then add contribution from ground
			else {
				// Add luminance from ground modified by wndo vis trans and angle of incidence on node surface
				// Luminance of ground (cd/ft2) is
				// illuminance on ground (lum/ft2) * gnd_refl (which gives ft-lamberts) / PI
				dGroundLumSkyC = bldg_ptr->hillumskyc[iphs] * bldg_ptr->zone[izone]->surf[iWndoSurf]->gnd_refl / PI;
				dGroundLumSunC = bldg_ptr->hillumsunc[iphs] * bldg_ptr->zone[izone]->surf[iWndoSurf]->gnd_refl / PI;
				// Add separate sky and sun components of ground luminance
				(*pdirect_skyclum) += dGroundLumSkyC * domega * tvisincidence * cosPtSurfIncidence * dNodeSurfaceReflectance;
				(*pdirect_sunclum) += dGroundLumSunC * domega * tvisincidence * cosPtSurfIncidence * dNodeSurfaceReflectance;
			}

			/* for overcast sky */
			if ((iphs == 0) && (iths == 0)) {
				// If ray points upward above or along the horizontal plane, then add contribution from sky
				if (phray > 0.0) {
					elum = dskylu(1,thray,phray,thsun,phsun,zenl);
					dedir = elum * domega * tvisincidence * cosPtSurfIncidence * dNodeSurfaceReflectance;
					(*pdirect_skyolum) += dedir;
				}
				// If ray points downward below the horizontal plane, then add contribution from ground
				else {
					// Add luminance from ground modified by wndo vis trans and angle of incidence on node surface
					// Luminance of ground (cd/ft2) is
					// illuminance on ground (lum/ft2) * gnd_refl (which gives ft-lamberts) / PI
					dGroundLumSkyO = bldg_ptr->hillumskyo[iphs] * bldg_ptr->zone[izone]->surf[iWndoSurf]->gnd_refl / PI;
					(*pdirect_skyolum) += dGroundLumSkyO * domega * tvisincidence * cosPtSurfIncidence * dNodeSurfaceReflectance;
				}
			}
		}

		/* Illuminance from (unreflected) direct sun. */
		/* (calculated only once per wndo for each surf node) */
//		if ((ix == (nwx-1)) && (iy == (nwy-1))) {
		if (iWndoElement == 0) {
			/* unit vector to sun from anywhere in the bldg */
			raycos[0] = cos(phsun) * cos(thsun);
			raycos[1] = cos(phsun) * sin(thsun);
			raycos[2] = sin(phsun);

			/* is sun on front side of current window? */
			cosi = ddot(wnorm,raycos);
			if (cosi > 0.0) {
				/* does raycos from current surf node pass thru window? */
//				dpierc(&ip,w1,w2,w3,node,raycos);

                // Use WLC ray intersect surface polygon method.
                // Note that this tests ray intersecting surface face on the INSIDE of DOE2 convention surface
                // So this only works for rays from nodal points intersecting Window Surface polygons from inside zone.
                BGL::point3	pt3Node = BGL::point3(node[0], node[1], node[2]);	//	center of box
                BGL::vector3 vRayDir = BGL::vector3(raycos[0], raycos[1], raycos[2]);
                BGL::ray3	r3Ray(pt3Node,vRayDir);
                bool bIntersects = false;
                Double dParam;  // can be used to calculate intersection point
                bIntersects = bldg_ptr->zone[izone]->surf[iWndoSurf]->wndo[iwndo]->intersect(r3Ray,dParam);

				if (bIntersects) {
//				if (ip != 0) {
					/* does raycos from current surf node intercept shade? */
					/* NOTE: this includes all zone surfaces contrary to DOE2 check of */
					/* only "self-shade" surfaces (in addition to zone and bldg shades). */
					// rjh 5/14/97 correction - parameter ray replaced with raycos
	//				dhitsh(&rchit,rref,ray,bldg_ptr,izone,isurf);
					dhitsh(&rchit,node,raycos,bldg_ptr,izone,iWndoSurf,iNodeSurf);
					if (rchit.ihit == 0) {
						/* sun reaches surf node */
						/* increment luminance */

						// tvis of glass for cosi incid angle
						// Window 4 implementation 4/2002
						/* set lib_index component type to glass */
						char sLibCompType[MAX_CHAR_UNAME+1];	/* temp str */
						/* get library index of current window glass type */
						strcpy(sLibCompType,"glass");
						int igt = lib_index(lib_ptr,sLibCompType,bldg_ptr->zone[izone]->surf[iWndoSurf]->wndo[iwndo]->glass_type);
						int iGlass_Type_ID = atoi(bldg_ptr->zone[izone]->surf[iWndoSurf]->wndo[iwndo]->glass_type);
						// glass type ID: 1 to 11 => DOE2 original, >11 => W4lib.dat, <0 => E10 library
						if ((iGlass_Type_ID > 0) && (iGlass_Type_ID <= 11))  {	// DOE2 original

							// DOE2 original angular dependence equation coefs
							double cam1 = lib_ptr->glass[igt]->cam1;
							double cam2 = lib_ptr->glass[igt]->cam2;
							double cam3 = lib_ptr->glass[igt]->cam3;
							double cam4 = lib_ptr->glass[igt]->cam4;

							// tvis of glass for cosi incid angle
							tviss = max(0.0,(cam1+cosi*(cam2+cosi*(cam3+cosi*cam4))));
						}
						else if ((iGlass_Type_ID > 11) && (iGlass_Type_ID <= 10000)) {	// Window4

							// Window4 angular dependence Fit4() function coefs.
							double W4vis_fit1 = lib_ptr->glass[igt]->W4vis_fit1;
							double W4vis_fit2 = lib_ptr->glass[igt]->W4vis_fit2;

							// tvis of glass for cosi incid angle
							tviss = vis_trans * fit4(cosi, W4vis_fit1, W4vis_fit2);
						}
						else if (iGlass_Type_ID > 10000) {	// EnergyPlus/Window5

							// tvis of glass for cosi incid angle
							tviss = POLYF(cosi, lib_ptr->glass[igt]->EPlusCoef);
						}
						else if (iGlass_Type_ID < 0) {	// Energy-10

							/* Energy-10 angular dependence equation coefs. */
							double E10coef1 = lib_ptr->glass[igt]->E10coef[0];
							double E10coef2 = lib_ptr->glass[igt]->E10coef[1];
							double E10coef3 = lib_ptr->glass[igt]->E10coef[2];
							double E10coef4 = lib_ptr->glass[igt]->E10coef[3];

							// tvis of glass for cosi incid angle
							tviss = vis_trans * max(0.0,(cosi*(E10coef1+cosi*(E10coef2+cosi*(E10coef3+cosi*E10coef4)))));
						}

						/* calc direct normal illuminance (lumens/ft2) from solar disk */
						dnsoli = dnsol(solic,bldg_ptr,IMREF,phsun,tfac,pofdmpfile);
						if (dnsoli < 0.0) {
							*pofdmpfile << "ERROR: DElight Bad return from dnsol() = " <<dnsoli << ", exit wndo_element_lum_contrib()\n";
							return(-1);
						}

						/* calc cos(incidence) for raycos on node surface */
						double cosinc;
						cosinc = ddot(nodesurfnormal,raycos);

						/* add luminance from sun modified by wndo vis trans and angle of incidence on node surface */
						(*pdirect_sunclum) += dnsoli*tviss*cosinc * dNodeSurfaceReflectance;
					}
				}
			}
		}
	}

	/* CASE 2 - Window with closed shades (i.e., diffuse glazing) */
	else { // (bldg_ptr->zone[izone]->surf[iWndoSurf]->wndo[iwndo]->shade_flag != 0)

		/* Luminance of wndo element is shade luminance. */
		tvis1 = 1.0;

		/* for clear sky */
		(*pdirect_skyclum) += bldg_ptr->zone[izone]->surf[iWndoSurf]->wndo[iwndo]->wlumsky[iphs][iths]*domega*tvis1*cosPtSurfIncidence * dNodeSurfaceReflectance;
		(*pdirect_sunclum) += bldg_ptr->zone[izone]->surf[iWndoSurf]->wndo[iwndo]->wlumsun[iphs][iths]*domega*tvis1*cosPtSurfIncidence * dNodeSurfaceReflectance;

		/* for overcast sky */
		if ((iphs == 0) && (iths == 0)) {
			(*pdirect_skyolum) += bldg_ptr->zone[izone]->surf[iWndoSurf]->wndo[iwndo]->wlumskyo*domega*tvis1*cosPtSurfIncidence * dNodeSurfaceReflectance;
		}
	}

	return(0);
}

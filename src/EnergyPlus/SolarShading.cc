// EnergyPlus, Copyright (c) 1996-2025, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Sustainable Energy, LLC, and other
// contributors. All rights reserved.
//
// NOTICE: This Software was developed under funding from the U.S. Department of Energy and the
// U.S. Government consequently retains certain rights. As such, the U.S. Government has been
// granted for itself and others acting on its behalf a paid-up, nonexclusive, irrevocable,
// worldwide license in the Software to reproduce, distribute copies to the public, prepare
// derivative works, and perform publicly and display publicly, and to permit others to do so.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
//
// (1) Redistributions of source code must retain the above copyright notice, this list of
//     conditions and the following disclaimer.
//
// (2) Redistributions in binary form must reproduce the above copyright notice, this list of
//     conditions and the following disclaimer in the documentation and/or other materials
//     provided with the distribution.
//
// (3) Neither the name of the University of California, Lawrence Berkeley National Laboratory,
//     the University of Illinois, U.S. Dept. of Energy nor the names of its contributors may be
//     used to endorse or promote products derived from this software without specific prior
//     written permission.
//
// (4) Use of EnergyPlus(TM) Name. If Licensee (i) distributes the software in stand-alone form
//     without changes from the version obtained under this License, or (ii) Licensee makes a
//     reference solely to the software portion of its product, Licensee must refer to the
//     software as "EnergyPlus version X" software, where "X" is the version number Licensee
//     obtained under this License and may not use a different name for the software. Except as
//     specifically required in this Section (4), Licensee shall not use in a company name, a
//     product name, in advertising, publicity, or other promotional activities any name, trade
//     name, trademark, logo, or other designation of "EnergyPlus", "E+", "e+" or confusingly
//     similar designation, without the U.S. Department of Energy's prior written consent.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.

// C++ Headers
#include <algorithm>
#include <cassert>
#include <cmath>
#include <memory>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Vector3.hh>
#include <ObjexxFCL/member.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/CommandLineInterface.hh>
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataDaylightingDevices.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataErrorTracking.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataReportingFlags.hh>
#include <EnergyPlus/DataShadowingCombinations.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/DataViewFactorInformation.hh>
#include <EnergyPlus/DataWindowEquivalentLayer.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DaylightingDevices.hh>
#include <EnergyPlus/DaylightingManager.hh>
#include <EnergyPlus/DisplayRoutines.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/EnergyPlusLogger.hh>
// #include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/PluginManager.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SolarReflectionManager.hh>
#include <EnergyPlus/SolarShading.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/Vectors.hh>
#include <EnergyPlus/WindowComplexManager.hh>
#include <EnergyPlus/WindowEquivalentLayer.hh>
#include <EnergyPlus/WindowManager.hh>
#include <EnergyPlus/WindowManagerExteriorData.hh>
#include <EnergyPlus/WindowModel.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>
#include <WCEMultiLayerOptics.hpp>

namespace EnergyPlus::SolarShading {

// MODULE INFORMATION:
//       AUTHOR         Rick Strand
//       DATE WRITTEN   March 1997
//       MODIFIED       December 1998, FCW
//       MODIFIED       July 1999, Linda Lawrie, eliminate shadefl.scr,
//                      do shadowing calculations during simulation
//       MODIFIED       June 2001, FCW, handle window blinds
//       MODIFIED       May 2004, LKL, Polygons > 4 sides (not subsurfaces)
//       MODIFIED       January 2007, LKL, Taking parameters back to original integer (HC)
//       MODIFIED       August 2011, JHK, Including Complex Fenestration optical calculations
//       MODIFIED       November 2012, BG, Timestep solar and daylighting calculations
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// The purpose of this module is to encompass the routines and data
// which are need to perform the solar calculations in EnergyPlus.
// This also requires that shading and geometry routines and data
// which are used by the solar calculations be included in this module.

// METHODOLOGY EMPLOYED:
// Many of the methods used in this module have been carried over from the
// (I)BLAST program.  As such, there is not much documentation on the
// methodology used.  The original code was written mainly by George
// Walton and requires coordinate transformations.  It calculates
// shading using an overlapping polygon approach.

// REFERENCES:
// TARP Manual, NIST Publication.
// Passive Solar Extension of the BLAST Program, CERL/UIUC Publication.

using namespace DataEnvironment;
using namespace DataHeatBalance;
using namespace DataSurfaces;
using namespace DataShadowingCombinations;
using namespace SolarReflectionManager;
using namespace DataVectorTypes;
using namespace Window;
using namespace FenestrationCommon;
using namespace SingleLayerOptics;

int constexpr NPhi = 6;                                // Number of altitude angle steps for sky integration
int constexpr NTheta = 24;                             // Number of azimuth angle steps for sky integration
Real64 constexpr Eps = 1.e-10;                         // Small number
Real64 constexpr DPhi = Constant::PiOvr2 / NPhi;       // Altitude step size
Real64 constexpr DTheta = 2.0 * Constant::Pi / NTheta; // Azimuth step size
Real64 constexpr DThetaDPhi = DTheta * DPhi;           // Product of DTheta and DPhi
Real64 constexpr PhiMin = 0.5 * DPhi;                  // Minimum altitude

Real64 constexpr HCMULT = 100000.0; // Multiplier used to change meters to .01 millimeters for homogeneous coordinates.
                                    // Homogeneous Coordinates are represented in integers (64 bit). This changes the surface coordinates from meters
                                    // to .01 millimeters -- making that the resolution for shadowing, polygon clipping, etc.
Real64 constexpr sqHCMULT = (HCMULT * HCMULT);    // Square of HCMult used in Homogeneous coordinates
Real64 constexpr sqHCMULT_fac = (0.5 / sqHCMULT); // ( 0.5 / sqHCMULT ) factor

// Parameters for use with the variable OverlapStatus...
int constexpr NoOverlap = 1;
int constexpr FirstSurfWithinSecond = 2;
int constexpr SecondSurfWithinFirst = 3;
int constexpr PartialOverlap = 4;
int constexpr TooManyVertices = 5;
int constexpr TooManyFigures = 6;

void InitSolarCalculations(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         George Walton
    //       DATE WRITTEN   September 1977
    //       MODIFIED       na
    //       RE-ENGINEERED  Mar97, RKS, Initial EnergyPlus Version

    // PURPOSE OF THIS SUBROUTINE:
    // This routine controls the computation of the solar flux multipliers.

    // METHODOLOGY EMPLOYED:
    // All shadowing calculations have been grouped under this routine to
    // allow segmentation separating it from the hourly loads calculation.

    auto &s_surf = state.dataSurface;
#ifdef EP_Count_Calls
    ++state.dataTimingsData->NumInitSolar_Calls;
#endif
    if (state.dataGlobal->BeginSimFlag) {
        if (state.files.outputControl.shd) {
            state.dataSolarShading->shd_stream =
                std::make_unique<std::fstream>(state.dataStrGlobals->outputShdFilePath, std::ios_base::out | std::ios_base::trunc);
            if (!state.dataSolarShading->shd_stream) {
                ShowFatalError(
                    state, format("InitSolarCalculations: Could not open file \"{}\" for output (write).", state.dataStrGlobals->outputShdFilePath));
            }
        } else {
            state.dataSolarShading->shd_stream = std::make_unique<std::iostream>(nullptr);
        }

        if (state.dataSolarShading->GetInputFlag) {
            checkShadingSurfaceSchedules(state);
            processShadowingInput(state);
            state.dataSolarShading->GetInputFlag = false;
            state.dataSolarShading->MaxHCV =
                (((max(15, s_surf->MaxVerticesPerSurface) + 16) / 16) * 16) - 1; // Assure MaxHCV+1 is multiple of 16 for 128 B alignment
            assert((state.dataSolarShading->MaxHCV + 1) % 16 == 0);
        }

        if (state.dataSolarShading->firstTime) {
            DisplayString(state, "Allocate Solar Module Arrays");
        }
        AllocateModuleArrays(state);

        if (state.dataHeatBal->SolarDistribution != DataHeatBalance::Shadowing::FullInteriorExterior) {
            if (state.dataSolarShading->firstTime) {
                DisplayString(state, "Computing Interior Solar Absorption Factors");
            }
            ComputeIntSolarAbsorpFactors(state);
        }

        if (state.dataSolarShading->firstTime) {
            DisplayString(state, "Determining Shadowing Combinations");
        }
        DetermineShadowingCombinations(state);
        state.dataSolarShading->shd_stream.reset(); // Done writing to shd file

        if (state.dataSolarShading->firstTime) {
            DisplayString(state, "Computing Window Shade Absorption Factors");
        }
        ComputeWinShadeAbsorpFactors(state);

        if (s_surf->CalcSolRefl) {
            DisplayString(state, "Initializing Solar Reflection Factors");
            InitSolReflRecSurf(state);
        }

        if (state.dataSolarShading->firstTime) {
            DisplayString(state, "Proceeding with Initializing Solar Calculations");
        }
    }

    if (state.dataGlobal->BeginEnvrnFlag) {
        state.dataSolarShading->SurfSunCosTheta = 0.0;
        state.dataSolarShading->SurfSunlitArea = 0.0;
        s_surf->SurfSunlitArea = 0.0;
        s_surf->SurfSunlitFrac = 0.0;
        state.dataHeatBal->SurfSunlitFracHR = 0.0;
        state.dataHeatBal->SurfSunlitFrac = 0.0;
        state.dataHeatBal->SurfSunlitFracWithoutReveal = 0.0;
        state.dataHeatBal->SurfWinBackSurfaces = 0;
        state.dataHeatBal->SurfWinOverlapAreas = 0.0;
        state.dataHeatBal->SurfCosIncAngHR = 0.0;
        state.dataHeatBal->SurfCosIncAng = 0.0;
        state.dataSolarShading->SurfAnisoSkyMult = 1.0; // For isotropic sky; recalculated in AnisoSkyViewFactors if anisotropic radiance
        //    WithShdgIsoSky=0.0
        //    WoShdgIsoSky=0.0
        //    WithShdgHoriz=0.0
        //    WoShdgHoriz=0.0
        //    DifShdgRatioIsoSky=0.0
        //    DifShdgRatioHoriz=0.0
        state.dataSolarShading->SurfMultIsoSky = 0.0;
        state.dataSolarShading->SurfMultCircumSolar = 0.0;
        state.dataSolarShading->SurfMultHorizonZenith = 0.0;
        state.dataSolarShading->SurfWinRevealStatus = 0;

        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            state.dataHeatBal->ZoneWinHeatGain(zoneNum) = 0.0;
            state.dataHeatBal->ZoneWinHeatGainRep(zoneNum) = 0.0;
            state.dataHeatBal->ZoneWinHeatLossRep(zoneNum) = 0.0;
            state.dataHeatBal->ZoneWinHeatGainRepEnergy(zoneNum) = 0.0;
            state.dataHeatBal->ZoneWinHeatLossRepEnergy(zoneNum) = 0.0;
            state.dataHeatBal->ZoneOpaqSurfInsFaceCond(zoneNum) = 0.0;
            state.dataHeatBal->ZoneOpaqSurfInsFaceCondGainRep(zoneNum) = 0.0;
            state.dataHeatBal->ZoneOpaqSurfInsFaceCondLossRep(zoneNum) = 0.0;
            state.dataHeatBal->ZnOpqSurfInsFaceCondGnRepEnrg(zoneNum) = 0.0;
            state.dataHeatBal->ZnOpqSurfInsFaceCondLsRepEnrg(zoneNum) = 0.0;
        }
        for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
            state.dataHeatBal->ZoneTransSolar(enclNum) = 0.0;
            state.dataHeatBal->ZoneBmSolFrExtWinsRep(enclNum) = 0.0;
            state.dataHeatBal->ZoneBmSolFrIntWinsRep(enclNum) = 0.0;
            state.dataHeatBal->EnclSolInitialDifSolReflW(enclNum) = 0.0;
            state.dataHeatBal->ZoneDifSolFrExtWinsRep(enclNum) = 0.0;
            state.dataHeatBal->ZoneDifSolFrIntWinsRep(enclNum) = 0.0;
            state.dataHeatBal->ZoneTransSolarEnergy(enclNum) = 0.0;
            state.dataHeatBal->ZoneBmSolFrExtWinsRepEnergy(enclNum) = 0.0;
            state.dataHeatBal->ZoneBmSolFrIntWinsRepEnergy(enclNum) = 0.0;
            state.dataHeatBal->ZoneDifSolFrExtWinsRepEnergy(enclNum) = 0.0;
            state.dataHeatBal->ZoneDifSolFrIntWinsRepEnergy(enclNum) = 0.0;
        }
        for (int SurfNum = 1; SurfNum <= s_surf->TotSurfaces; ++SurfNum) {
            state.dataHeatBal->SurfQRadSWOutIncident(SurfNum) = 0.0;
            state.dataHeatBal->SurfQRadSWOutIncidentBeam(SurfNum) = 0.0;
            state.dataHeatBal->SurfBmIncInsSurfIntensRep(SurfNum) = 0.0;
            state.dataHeatBal->SurfBmIncInsSurfAmountRep(SurfNum) = 0.0;
            state.dataHeatBal->SurfIntBmIncInsSurfIntensRep(SurfNum) = 0.0;
            state.dataHeatBal->SurfIntBmIncInsSurfAmountRep(SurfNum) = 0.0;
            state.dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(SurfNum) = 0.0;
            state.dataHeatBal->SurfQRadSWOutIncidentGndDiffuse(SurfNum) = 0.0;
            state.dataHeatBal->SurfQRadSWOutIncBmToDiffReflGnd(SurfNum) = 0.0;
            state.dataHeatBal->SurfQRadSWOutIncSkyDiffReflGnd(SurfNum) = 0.0;
            state.dataHeatBal->SurfQRadSWOutIncBmToBmReflObs(SurfNum) = 0.0;
            state.dataHeatBal->SurfQRadSWOutIncBmToDiffReflObs(SurfNum) = 0.0;
            state.dataHeatBal->SurfQRadSWOutIncSkyDiffReflObs(SurfNum) = 0.0;
            state.dataHeatBal->SurfCosIncidenceAngle(SurfNum) = 0.0;
            state.dataHeatBal->SurfSWInAbsTotalReport(SurfNum) = 0.0;
            state.dataHeatBal->SurfBmIncInsSurfAmountRepEnergy(SurfNum) = 0.0;
            state.dataHeatBal->SurfIntBmIncInsSurfAmountRepEnergy(SurfNum) = 0.0;
            state.dataHeatBal->SurfInitialDifSolInAbsReport(SurfNum) = 0.0;
        }
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                auto &thisSpace = state.dataHeatBal->space(spaceNum);
                int const firstSurfWin = thisSpace.WindowSurfaceFirst;
                int const lastSurfWin = thisSpace.WindowSurfaceLast;
                for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {

                    s_surf->SurfWinTransSolar(SurfNum) = 0.0;
                    s_surf->SurfWinBmSolar(SurfNum) = 0.0;
                    s_surf->SurfWinBmBmSolar(SurfNum) = 0.0;
                    s_surf->SurfWinBmDifSolar(SurfNum) = 0.0;
                    s_surf->SurfWinDifSolar(SurfNum) = 0.0;

                    s_surf->SurfWinTransSolarEnergy(SurfNum) = 0.0;
                    s_surf->SurfWinBmSolarEnergy(SurfNum) = 0.0;
                    s_surf->SurfWinBmBmSolarEnergy(SurfNum) = 0.0;
                    s_surf->SurfWinBmDifSolarEnergy(SurfNum) = 0.0;

                    s_surf->SurfWinHeatGain(SurfNum) = 0.0;
                    s_surf->SurfWinHeatGainRep(SurfNum) = 0.0;
                    s_surf->SurfWinHeatLossRep(SurfNum) = 0.0;
                }
                for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {
                    s_surf->SurfWinGainConvGlazToZoneRep(SurfNum) = 0.0;
                    s_surf->SurfWinGainIRGlazToZoneRep(SurfNum) = 0.0;
                    s_surf->SurfWinLossSWZoneToOutWinRep(SurfNum) = 0.0;
                    s_surf->SurfWinGainFrameDividerToZoneRep(SurfNum) = 0.0;
                    s_surf->SurfWinGainConvShadeToZoneRep(SurfNum) = 0.0;
                    s_surf->SurfWinGainIRShadeToZoneRep(SurfNum) = 0.0;
                    s_surf->SurfWinGapConvHtFlowRep(SurfNum) = 0.0;
                    s_surf->SurfWinShadingAbsorbedSolar(SurfNum) = 0.0;

                    s_surf->SurfWinSysSolTransmittance(SurfNum) = 0.0;
                    s_surf->SurfWinSysSolReflectance(SurfNum) = 0.0;
                    s_surf->SurfWinSysSolAbsorptance(SurfNum) = 0.0;
                }
                for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {
                    s_surf->SurfWinDifSolarEnergy(SurfNum) = 0.0;
                    s_surf->SurfWinHeatGainRepEnergy(SurfNum) = 0.0;
                    s_surf->SurfWinHeatLossRepEnergy(SurfNum) = 0.0;
                    s_surf->SurfWinGapConvHtFlowRepEnergy(SurfNum) = 0.0;
                    s_surf->SurfWinShadingAbsorbedSolarEnergy(SurfNum) = 0.0;

                    state.dataHeatBal->SurfWinQRadSWwinAbsTot(SurfNum) = 0.0;
                    state.dataHeatBal->SurfWinQRadSWwinAbsTotEnergy(SurfNum) = 0.0;
                    state.dataHeatBal->SurfWinSWwinAbsTotalReport(SurfNum) = 0.0;
                    state.dataHeatBal->SurfWinInitialDifSolInTransReport(SurfNum) = 0.0;

                    s_surf->SurfWinInsideGlassCondensationFlag(SurfNum) = 0;
                    s_surf->SurfWinInsideFrameCondensationFlag(SurfNum) = 0;
                    s_surf->SurfWinInsideDividerCondensationFlag(SurfNum) = 0;
                }
            }
        }
    }

    // Initialize these once
    for (int IPhi = 1; IPhi <= NPhi; ++IPhi) {   // Loop over patch altitude values
        Real64 Phi = PhiMin + (IPhi - 1) * DPhi; // 7.5,22.5,37.5,52.5,67.5,82.5 for NPhi = 6
        state.dataSolarShading->sin_Phi.push_back(std::sin(Phi));
        state.dataSolarShading->cos_Phi.push_back(std::cos(Phi));
    }

    for (int ITheta = 1; ITheta <= NTheta; ++ITheta) { // Loop over patch azimuth values
        Real64 Theta = (ITheta - 1) * DTheta;          // 0,15,30,....,330,345 for NTheta = 24
        state.dataSolarShading->sin_Theta.push_back(std::sin(Theta));
        state.dataSolarShading->cos_Theta.push_back(std::cos(Theta));
    }

    state.dataSolarShading->firstTime = false;
}

void checkShadingSurfaceSchedules(EnergyPlusData &state)
{
    auto &s_surf = state.dataSurface;

    // Shading surfaces with a transmittance schedule that is always 1.0 are marked IsTransparent during shading surface input processing
    // Now that EMS (and other types) actuators are set up, check to see if the schedule has an actuator and reset if needed
    for (int surfNum = s_surf->ShadingSurfaceFirst; surfNum <= s_surf->ShadingSurfaceLast; ++surfNum) {
        auto &thisSurface = s_surf->Surface(surfNum);
        if (!thisSurface.IsTransparent) {
            continue;
        }
        // creating some dummy bools here on purpose -- we need to do some renaming and/or consolidate these into a meaningful new global sometime
        // for now I want the logic to be as readable as possible, so creating shorthand variables makes it very clear
        bool const anyPlugins = size(state.dataPluginManager->plugins) > 0;
        bool const runningByAPI = state.dataGlobal->eplusRunningViaAPI;
        bool const anyEMS = state.dataGlobal->AnyEnergyManagementSystemInModel;
        if ((anyEMS && EMSManager::isScheduleManaged(state, thisSurface.shadowSurfSched)) || runningByAPI || anyPlugins) {
            // Transmittance schedule definitely has an actuator or may have one via python plugin or API
            // Set not transparent so it won't be skipped during shading calcs
            thisSurface.IsTransparent = false;
            // Also set global flags
            state.dataSolarShading->anyScheduledShadingSurface = true;
            s_surf->ShadingTransmittanceVaries = true;
        } else if (!thisSurface.MirroredSurf) {
            // Warning moved here from shading surface input processing (skip warning for mirrored surfaces)
            ShowWarningError(state,
                             format(R"(Shading Surface="{}", Transmittance Schedule Name="{}", is always transparent.)",
                                    thisSurface.Name,
                                    thisSurface.shadowSurfSched->Name));
            ShowContinueError(state, "This shading surface will be ignored.");
        }
    }
}

void GetShadowingInput(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   July 1999
    //       MODIFIED       B. Griffith, Nov 2012, add calculation method

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine gets the Shadowing Calculation object.

    auto &s_surf = state.dataSurface;

    // Using/Aliasing
    using DataSystemVariables::ShadingMethod;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int NumItems;
    int NumNumbers;
    int NumAlphas;
    int IOStat;
    auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;
    state.dataIPShortCut->rNumericArgs({1, 4}) = 0.0; // so if nothing gotten, defaults will be maintained.
    state.dataIPShortCut->cAlphaArgs(1) = "";
    state.dataIPShortCut->cAlphaArgs(2) = "";
    cCurrentModuleObject = "ShadowCalculation";
    NumItems = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
    NumAlphas = 0;
    NumNumbers = 0;
    if (NumItems > 1) {
        ShowWarningError(state, format("{}: More than 1 occurrence of this object found, only first will be used.", cCurrentModuleObject));
    }

    if (NumItems != 0) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 cCurrentModuleObject,
                                                                 1,
                                                                 state.dataIPShortCut->cAlphaArgs,
                                                                 NumAlphas,
                                                                 state.dataIPShortCut->rNumericArgs,
                                                                 NumNumbers,
                                                                 IOStat,
                                                                 state.dataIPShortCut->lNumericFieldBlanks,
                                                                 state.dataIPShortCut->lAlphaFieldBlanks,
                                                                 state.dataIPShortCut->cAlphaFieldNames,
                                                                 state.dataIPShortCut->cNumericFieldNames);
        state.dataSolarShading->ShadowingCalcFrequency = state.dataIPShortCut->rNumericArgs(1);
    }

    if (state.dataSolarShading->ShadowingCalcFrequency <= 0) {
        //  Set to default value
        state.dataSolarShading->ShadowingCalcFrequency = 20;
    }
    if (state.dataSolarShading->ShadowingCalcFrequency > 31) {
        ShowWarningError(state, format("{}: suspect {}", cCurrentModuleObject, state.dataIPShortCut->cNumericFieldNames(1)));
        ShowContinueError(state, format("Value entered=[{:.0R}], Shadowing Calculations will be inaccurate.", state.dataIPShortCut->rNumericArgs(1)));
    }

    if (state.dataIPShortCut->rNumericArgs(2) > 199.0) {
        state.dataSolarShading->MaxHCS = state.dataIPShortCut->rNumericArgs(2);
    } else {
        state.dataSolarShading->MaxHCS = 15000;
    }

    int aNum = 1;
    unsigned pixelRes = 512u;
    if (NumAlphas >= aNum) {
        if (Util::SameString(state.dataIPShortCut->cAlphaArgs(aNum), "Scheduled")) {
            state.dataSysVars->shadingMethod = ShadingMethod::Scheduled;
            state.dataIPShortCut->cAlphaArgs(aNum) = "Scheduled";
            checkScheduledSurfacePresent(state);
        } else if (Util::SameString(state.dataIPShortCut->cAlphaArgs(aNum), "Imported")) {
            if (state.dataSched->ScheduleFileShadingProcessed) {
                state.dataSysVars->shadingMethod = ShadingMethod::Imported;
                state.dataIPShortCut->cAlphaArgs(aNum) = "Imported";
            } else {
                ShowWarningError(state, format("{}: invalid {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaFieldNames(aNum)));
                ShowContinueError(state,
                                  format("Value entered=\"{}\" while no Schedule:File:Shading object is defined, InternalCalculation will be used.",
                                         state.dataIPShortCut->cAlphaArgs(aNum)));
            }
        } else if (Util::SameString(state.dataIPShortCut->cAlphaArgs(aNum), "PolygonClipping")) {
            state.dataSysVars->shadingMethod = ShadingMethod::PolygonClipping;
            state.dataIPShortCut->cAlphaArgs(aNum) = "PolygonClipping";
        } else if (Util::SameString(state.dataIPShortCut->cAlphaArgs(aNum), "PixelCounting")) {
            state.dataSysVars->shadingMethod = ShadingMethod::PixelCounting;
            state.dataIPShortCut->cAlphaArgs(aNum) = "PixelCounting";
            if (NumNumbers >= 3) {
                pixelRes = (unsigned)state.dataIPShortCut->rNumericArgs(3);
            }
#ifdef EP_NO_OPENGL
            ShowWarningError(state, format("{}: invalid {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaFieldNames(aNum)));
            ShowContinueError(state, format("Value entered=\"{}\"", state.dataIPShortCut->cAlphaArgs(aNum)));
            ShowContinueError(state, "This version of EnergyPlus was not compiled to use OpenGL (required for PixelCounting)");
            ShowContinueError(state, "PolygonClipping will be used instead");
            state.dataSysVars->shadingMethod = ShadingMethod::PolygonClipping;
            state.dataIPShortCut->cAlphaArgs(aNum) = "PolygonClipping";
#else
            if (Penumbra::Penumbra::is_valid_context()) {
                std::shared_ptr<EnergyPlusLogger> penumbra_logger = std::make_shared<EnergyPlusLogger>(EnergyPlusLogger::Log_level::Info);
                state.dataSolarShading->LoggerContext = std::make_pair<EnergyPlusData *, std::string>(&state, "");
                penumbra_logger->set_message_context(&state.dataSolarShading->LoggerContext);
                state.dataSolarShading->penumbra = std::make_unique<Penumbra::Penumbra>(pixelRes, penumbra_logger);
            } else {
                ShowWarningError(state, "No GPU found (required for PixelCounting)");
                ShowContinueError(state, "PolygonClipping will be used instead");
                state.dataSysVars->shadingMethod = ShadingMethod::PolygonClipping;
                state.dataIPShortCut->cAlphaArgs(aNum) = "PolygonClipping";
            }
#endif
        } else {
            ShowWarningError(state, format("{}: invalid {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaFieldNames(aNum)));
            ShowContinueError(state, format("Value entered=\"{}\", PolygonClipping will be used.", state.dataIPShortCut->cAlphaArgs(aNum)));
        }
    } else {
        state.dataIPShortCut->cAlphaArgs(aNum) = "PolygonClipping";
        state.dataSysVars->shadingMethod = ShadingMethod::PolygonClipping;
    }

    aNum++;
    if (NumAlphas >= aNum) {
        if (Util::SameString(state.dataIPShortCut->cAlphaArgs(aNum), "Periodic")) {
            state.dataSysVars->DetailedSolarTimestepIntegration = false;
            state.dataIPShortCut->cAlphaArgs(aNum) = "Periodic";
        } else if (Util::SameString(state.dataIPShortCut->cAlphaArgs(aNum), "Timestep")) {
            state.dataSysVars->DetailedSolarTimestepIntegration = true;
            state.dataIPShortCut->cAlphaArgs(aNum) = "Timestep";
        } else {
            ShowWarningError(state, format("{}: invalid {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaFieldNames(aNum)));
            ShowContinueError(state, format("Value entered=\"{}\", Periodic will be used.", state.dataIPShortCut->cAlphaArgs(aNum)));
            state.dataSysVars->DetailedSolarTimestepIntegration = false;
            state.dataIPShortCut->cAlphaArgs(aNum) = "Periodic";
        }
    } else {
        state.dataSysVars->DetailedSolarTimestepIntegration = false;
        state.dataIPShortCut->cAlphaArgs(aNum) = "Periodic";
    }

    aNum++;
    if (NumAlphas >= aNum) {
        if (Util::SameString(state.dataIPShortCut->cAlphaArgs(aNum), "SutherlandHodgman")) {
            state.dataSysVars->SutherlandHodgman = true;
            state.dataIPShortCut->cAlphaArgs(aNum) = "SutherlandHodgman";
        } else if (Util::SameString(state.dataIPShortCut->cAlphaArgs(aNum), "ConvexWeilerAtherton")) {
            state.dataSysVars->SutherlandHodgman = false;
            state.dataIPShortCut->cAlphaArgs(aNum) = "ConvexWeilerAtherton";
        } else if (Util::SameString(state.dataIPShortCut->cAlphaArgs(aNum), "SlaterBarskyandSutherlandHodgman")) {
            state.dataSysVars->SutherlandHodgman = true;
            state.dataSysVars->SlaterBarsky = true;
            state.dataIPShortCut->cAlphaArgs(aNum) = "SlaterBarskyandSutherlandHodgman";
        } else if (state.dataIPShortCut->lAlphaFieldBlanks(aNum)) {
            if (!state.dataSysVars->SutherlandHodgman) { // if already set.
                state.dataIPShortCut->cAlphaArgs(aNum) = "ConvexWeilerAtherton";
            } else {
                if (!state.dataSysVars->SlaterBarsky) {
                    state.dataIPShortCut->cAlphaArgs(aNum) = "SutherlandHodgman";
                } else {
                    state.dataIPShortCut->cAlphaArgs(aNum) = "SlaterBarskyandSutherlandHodgman";
                }
            }
        } else {
            ShowWarningError(state, format("{}: invalid {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaFieldNames(aNum)));
            if (!state.dataSysVars->SutherlandHodgman) {
                ShowContinueError(state, format("Value entered=\"{}\", ConvexWeilerAtherton will be used.", state.dataIPShortCut->cAlphaArgs(aNum)));
            } else {
                if (!state.dataSysVars->SlaterBarsky) {
                    ShowContinueError(state, format("Value entered=\"{}\", SutherlandHodgman will be used.", state.dataIPShortCut->cAlphaArgs(aNum)));
                } else {
                    ShowContinueError(
                        state,
                        format("Value entered=\"{}\", SlaterBarskyandSutherlandHodgman will be used.", state.dataIPShortCut->cAlphaArgs(aNum)));
                }
            }
        }
    } else {
        if (!state.dataSysVars->SutherlandHodgman) {
            state.dataIPShortCut->cAlphaArgs(aNum) = "ConvexWeilerAtherton";
        } else {
            if (!state.dataSysVars->SlaterBarsky) {
                state.dataIPShortCut->cAlphaArgs(aNum) = "SutherlandHodgman";
            } else {
                state.dataIPShortCut->cAlphaArgs(aNum) = "SlaterBarskyandSutherlandHodgman";
            }
        }
    }

    aNum++;
    if (NumAlphas >= aNum) {
        if (Util::SameString(state.dataIPShortCut->cAlphaArgs(aNum), "SimpleSkyDiffuseModeling")) {
            state.dataSysVars->DetailedSkyDiffuseAlgorithm = false;
            state.dataIPShortCut->cAlphaArgs(aNum) = "SimpleSkyDiffuseModeling";
        } else if (Util::SameString(state.dataIPShortCut->cAlphaArgs(aNum), "DetailedSkyDiffuseModeling")) {
            state.dataSysVars->DetailedSkyDiffuseAlgorithm = true;
            state.dataIPShortCut->cAlphaArgs(aNum) = "DetailedSkyDiffuseModeling";
        } else if (state.dataIPShortCut->lAlphaFieldBlanks(3)) {
            state.dataSysVars->DetailedSkyDiffuseAlgorithm = false;
            state.dataIPShortCut->cAlphaArgs(aNum) = "SimpleSkyDiffuseModeling";
        } else {
            ShowWarningError(state, format("{}: invalid {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaFieldNames(aNum)));
            ShowContinueError(state, format("Value entered=\"{}\", SimpleSkyDiffuseModeling will be used.", state.dataIPShortCut->cAlphaArgs(aNum)));
        }
    } else {
        state.dataIPShortCut->cAlphaArgs(aNum) = "SimpleSkyDiffuseModeling";
        state.dataSysVars->DetailedSkyDiffuseAlgorithm = false;
    }

    aNum++;
    if (NumAlphas >= aNum) {
        if (Util::SameString(state.dataIPShortCut->cAlphaArgs(aNum), "Yes")) {
            state.dataSysVars->ReportExtShadingSunlitFrac = true;
            state.dataIPShortCut->cAlphaArgs(aNum) = "Yes";
        } else if (Util::SameString(state.dataIPShortCut->cAlphaArgs(aNum), "No")) {
            state.dataSysVars->ReportExtShadingSunlitFrac = false;
            state.dataIPShortCut->cAlphaArgs(aNum) = "No";
        } else {
            ShowWarningError(state, format("{}: invalid {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaFieldNames(aNum)));
            ShowContinueError(state, format("Value entered=\"{}\", InternalCalculation will be used.", state.dataIPShortCut->cAlphaArgs(aNum)));
        }
    } else {
        state.dataIPShortCut->cAlphaArgs(aNum) = "No";
        state.dataSysVars->ReportExtShadingSunlitFrac = false;
    }

    aNum++;
    if (NumAlphas >= aNum) {
        if (Util::SameString(state.dataIPShortCut->cAlphaArgs(aNum), "Yes")) {
            state.dataSysVars->DisableSelfShadingWithinGroup = true;
            state.dataIPShortCut->cAlphaArgs(aNum) = "Yes";
        } else if (Util::SameString(state.dataIPShortCut->cAlphaArgs(aNum), "No")) {
            state.dataIPShortCut->cAlphaArgs(aNum) = "No";
        } else {
            ShowWarningError(state, format("{}: invalid {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaFieldNames(aNum)));
            ShowContinueError(state,
                              format("Value entered=\"{}\", all shading effects would be considered.", state.dataIPShortCut->cAlphaArgs(aNum)));
        }
    } else {
        state.dataIPShortCut->cAlphaArgs(aNum) = "No";
    }

    aNum++;
    if (NumAlphas >= aNum) {
        if (Util::SameString(state.dataIPShortCut->cAlphaArgs(aNum), "Yes")) {
            state.dataSysVars->DisableSelfShadingBetweenGroup = true;
            state.dataIPShortCut->cAlphaArgs(aNum) = "Yes";
        } else if (Util::SameString(state.dataIPShortCut->cAlphaArgs(aNum), "No")) {
            state.dataIPShortCut->cAlphaArgs(aNum) = "No";
        } else {
            ShowWarningError(state, format("{}: invalid {}", cCurrentModuleObject, state.dataIPShortCut->cAlphaFieldNames(aNum)));
            ShowContinueError(state,
                              format("Value entered=\"{}\", all shading effects would be considered.", state.dataIPShortCut->cAlphaArgs(aNum)));
        }
    } else {
        state.dataIPShortCut->cAlphaArgs(aNum) = "No";
    }

    if (state.dataSysVars->DisableSelfShadingBetweenGroup && state.dataSysVars->DisableSelfShadingWithinGroup) {
        state.dataSysVars->DisableAllSelfShading = true;
    } else if (state.dataSysVars->DisableSelfShadingBetweenGroup || state.dataSysVars->DisableSelfShadingWithinGroup) {
        state.dataSysVars->DisableGroupSelfShading = true;
    }

    aNum++;
    state.dataSysVars->shadingGroupsNum = NumAlphas - (aNum - 1);
    state.dataSysVars->shadingGroupZoneListNames.allocate(state.dataSysVars->shadingGroupsNum);
    for (int numZone = 1; numZone <= state.dataSysVars->shadingGroupsNum; ++numZone) {
        state.dataSysVars->shadingGroupZoneListNames(numZone) = state.dataIPShortCut->cAlphaArgs(aNum - 1 + numZone);
    }

    if (!state.dataSysVars->DetailedSolarTimestepIntegration && s_surf->ShadingTransmittanceVaries &&
        state.dataHeatBal->SolarDistribution != DataHeatBalance::Shadowing::Minimal) {

        ShowWarningError(state, "GetShadowingInput: The shading transmittance for shading devices may change throughout the year.");
        ShowContinueError(state,
                          format("Choose Shading Calculation Update Frequency Method = Timestep in the {} object to capture all shading impacts.",
                                 cCurrentModuleObject));
    }
    if (!state.dataSysVars->DetailedSkyDiffuseAlgorithm && s_surf->ShadingTransmittanceVaries &&
        state.dataHeatBal->SolarDistribution != DataHeatBalance::Shadowing::Minimal) {

        ShowWarningError(state, "GetShadowingInput: The shading transmittance for shading devices may change throughout the year.");
        ShowContinueError(state, "Simulation has been reset to use DetailedSkyDiffuseModeling. Simulation continues.");
        ShowContinueError(state, format("Choose DetailedSkyDiffuseModeling in the {} object to remove this warning.", cCurrentModuleObject));
        state.dataSysVars->DetailedSkyDiffuseAlgorithm = true;
        state.dataIPShortCut->cAlphaArgs(2) = "DetailedSkyDiffuseModeling";
        if (!state.dataSysVars->DetailedSolarTimestepIntegration && state.dataSolarShading->ShadowingCalcFrequency > 1) {
            ShowContinueError(state,
                              format("Better accuracy may be gained by setting the {} to 1 in the {} object.",
                                     state.dataIPShortCut->cNumericFieldNames(1),
                                     cCurrentModuleObject));
        }
    } else if (state.dataSysVars->DetailedSkyDiffuseAlgorithm) {
        if (!s_surf->ShadingTransmittanceVaries || state.dataHeatBal->SolarDistribution == DataHeatBalance::Shadowing::Minimal) {
            ShowWarningError(state,
                             "GetShadowingInput: DetailedSkyDiffuseModeling is chosen but not needed as either the shading transmittance for "
                             "shading devices does not change throughout the year");
            ShowContinueError(state, " or MinimalShadowing has been chosen.");
            ShowContinueError(state, "Simulation should be set to use SimpleSkyDiffuseModeling, but is left at Detailed for simulation.");
            ShowContinueError(state, format("Choose SimpleSkyDiffuseModeling in the {} object to reduce computation time.", cCurrentModuleObject));
        }
    }

    print(state.files.eio,
          "{}",
          "! <Shadowing/Sun Position Calculations Annual Simulations>, Shading Calculation Method, "
          "Shading Calculation Update Frequency Method, Shading Calculation Update Frequency {days}, "
          "Maximum Figures in Shadow Overlap Calculations {}, Polygon Clipping Algorithm, Pixel Counting Resolution, Sky Diffuse Modeling "
          "Algorithm, Output External Shading Calculation Results, Disable "
          "Self-Shading Within Shading Zone Groups, Disable Self-Shading From Shading Zone Groups to Other Zones\n");
    print(state.files.eio,
          "Shadowing/Sun Position Calculations Annual Simulations,{},{},{},{},{},{},{},{},{},{}\n",
          state.dataIPShortCut->cAlphaArgs(1),
          state.dataIPShortCut->cAlphaArgs(2),
          state.dataSolarShading->ShadowingCalcFrequency,
          state.dataSolarShading->MaxHCS,
          state.dataIPShortCut->cAlphaArgs(3),
          pixelRes,
          state.dataIPShortCut->cAlphaArgs(4),
          state.dataIPShortCut->cAlphaArgs(5),
          state.dataIPShortCut->cAlphaArgs(6),
          state.dataIPShortCut->cAlphaArgs(7));
}

void processShadowingInput(EnergyPlusData &state)
{
    // all shadow input processing that needed zones and surfaces to already be read into data (part of fix for Defect #10299)

    if ((state.dataSysVars->shadingMethod == DataSystemVariables::ShadingMethod::PixelCounting) &&
        state.dataSolarShading->anyScheduledShadingSurface) {
        ShowSevereError(state, "The Shading Calculation Method of choice is \"PixelCounting\"; ");
        ShowContinueError(state, "and there is at least one shading surface of type ");
        ShowContinueError(state, "Shading:Site:Detailed, Shading:Building:Detailed, or Shading:Zone:Detailed, ");
        ShowContinueError(state, "that has an active transmittance schedule value greater than zero or may vary.");
        ShowContinueError(state, "With \"PixelCounting\" Shading Calculation Method, the shading surfaces will be treated as ");
        ShowContinueError(state, "completely opaque (transmittance = 0) during the shading calculation, ");
        ShowContinueError(state, "which may result in inaccurate or unexpected results.");
        ShowContinueError(state, "It is suggested switching to another Shading Calculation Method, such as \"PolygonClipping\".");
    }

    if (state.dataSysVars->shadingMethod == DataSystemVariables::ShadingMethod::Imported) {
        for (auto &surf : state.dataSurface->Surface) {
            if ((surf.surfExternalShadingSched = Sched::GetSchedule(state, surf.Name + "_shading")) != nullptr) {
                surf.SurfSchedExternalShadingFrac = true;
            } else {
                ShowWarningError(state,
                                 format("processShadowingInput: sunlit fraction schedule not found for {} when using ImportedShading.", surf.Name));
                ShowContinueError(state, "These values are set to 1.0.");
            }
        }
    }

    int SurfZoneGroup, CurZoneGroup;
    int Found = 0;
    if (state.dataSysVars->DisableGroupSelfShading) {
        Array1D_int DisableSelfShadingGroups;
        int NumOfShadingGroups = state.dataSysVars->shadingGroupsNum;
        if (NumOfShadingGroups > 0) {
            DisableSelfShadingGroups.allocate(NumOfShadingGroups);
            for (int i = 1; i <= NumOfShadingGroups; i++) {
                Found = Util::FindItemInList(
                    state.dataSysVars->shadingGroupZoneListNames(i), state.dataHeatBal->ZoneList, state.dataHeatBal->NumOfZoneLists);
                if (Found != 0) {
                    DisableSelfShadingGroups(i) = Found;
                }
            }

            for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; SurfNum++) {
                if (state.dataSurface->Surface(SurfNum).ExtBoundCond == 0) { // Loop through all exterior surfaces
                    SurfZoneGroup = 0;
                    // Check the shading zone group of each exterior surface
                    for (int ZoneGroupLoop = 1; ZoneGroupLoop <= NumOfShadingGroups; ZoneGroupLoop++) { // Loop through all defined shading groups
                        CurZoneGroup = DisableSelfShadingGroups(ZoneGroupLoop);
                        for (int ZoneNum = 1; ZoneNum <= state.dataHeatBal->ZoneList(CurZoneGroup).NumOfZones;
                             ZoneNum++) { // Loop through all zones in the zone list
                            if (state.dataSurface->Surface(SurfNum).Zone == state.dataHeatBal->ZoneList(CurZoneGroup).Zone(ZoneNum)) {
                                SurfZoneGroup = CurZoneGroup;
                                break;
                            }
                        }
                    }
                    // if a surface is not in any zone group, no self shading is disabled for this surface
                    if (SurfZoneGroup != 0) {
                        // if DisableSelfShadingWithinGroup, add all zones in the same zone group to the surface's disabled zone list
                        // if DisableSelfShadingBetweenGroups, add all zones in all other zone groups to the surface's disabled zone list
                        for (int ZoneGroupLoop = 1; ZoneGroupLoop <= NumOfShadingGroups; ZoneGroupLoop++) { // Loop through all defined shading groups
                            CurZoneGroup = DisableSelfShadingGroups(ZoneGroupLoop);
                            if (SurfZoneGroup == CurZoneGroup && state.dataSysVars->DisableSelfShadingWithinGroup) {
                                for (int ZoneNum = 1; ZoneNum <= state.dataHeatBal->ZoneList(CurZoneGroup).NumOfZones;
                                     ZoneNum++) { // Loop through all zones in the zone list
                                    state.dataSurface->SurfShadowDisabledZoneList(SurfNum).push_back(
                                        state.dataHeatBal->ZoneList(CurZoneGroup).Zone(ZoneNum));
                                }
                            } else if (SurfZoneGroup != CurZoneGroup && state.dataSysVars->DisableSelfShadingBetweenGroup) {
                                for (int ZoneNum = 1; ZoneNum <= state.dataHeatBal->ZoneList(CurZoneGroup).NumOfZones; ZoneNum++) {
                                    state.dataSurface->SurfShadowDisabledZoneList(SurfNum).push_back(
                                        state.dataHeatBal->ZoneList(CurZoneGroup).Zone(ZoneNum));
                                }
                            }
                        }
                    }
                }
            }
        } else {
            ShowFatalError(state, "No Shading groups are defined when disabling grouped self shading.");
        }
    }
}

void checkScheduledSurfacePresent(EnergyPlusData &state)
{
    // User has chosen "Scheduled" for sunlit fraction so check to see which surfaces don't have a schedule
    int numNotDef = 0;
    int constexpr maxErrMessages = 50;
    auto &surfData = state.dataSurface;
    for (int surfNum = 1; surfNum <= surfData->TotSurfaces; ++surfNum) {
        auto &thisSurf = surfData->Surface(surfNum);
        if ((thisSurf.Class == SurfaceClass::Shading || thisSurf.Class == SurfaceClass::Detached_F || thisSurf.Class == SurfaceClass::Detached_B ||
             thisSurf.Class == SurfaceClass::Overhang || thisSurf.Class == SurfaceClass::Fin)) {
            continue; // skip shading surfaces
        }
        if (!thisSurf.SurfSchedExternalShadingFrac) {
            numNotDef += 1;
            if (numNotDef == 1) {
                ShowWarningError(
                    state,
                    format("ShadowCalculation specified Schedule for the Shading Calculation Method but no schedule provided for {}", thisSurf.Name));
                ShowContinueError(
                    state, "When Schedule is selected for the Shading Calculation Method and no schedule is provided for a particular surface,");
                ShowContinueError(
                    state, "EnergyPlus will assume that the surface is not shaded.  Use SurfaceProperty:LocalEnvironment to specify a schedule");
                ShowContinueError(state, "for sunlit fraction if this was not desired.  Otherwise, this surface will not be shaded at all.");
            } else if (numNotDef <= maxErrMessages) {
                ShowWarningError(state, format("No schedule was provided for {} either.  See above error message for more details", thisSurf.Name));
            }
        }
    }
    if (numNotDef > maxErrMessages) {
        ShowContinueError(state, format("This message is only shown for the first {} occurrences of this issue.", maxErrMessages));
    }
}

void AllocateModuleArrays(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   February 1998
    //       MODIFIED       August 2005 JG - Added output variables for energy in J

    // PURPOSE OF THIS SUBROUTINE:
    // This routine allocates all of the arrays at the module level which
    // require allocation.

    // METHODOLOGY EMPLOYED:
    // Allocation is dependent on the user input file.

    int I;
    int NumOfLayers;

    auto &s_surf = state.dataSurface;

    state.dataSolarShading->SurfSunCosTheta.dimension(s_surf->TotSurfaces, 0.0);
    state.dataSolarShading->SurfSunlitArea.dimension(s_surf->TotSurfaces, 0.0);
    if (!state.dataWindowManager->inExtWindowModel->isExternalLibraryModel() || !state.dataWindowManager->winOpticalModel->isSimplifiedModel()) {
        state.dataSolarShading->SurfWinIntBeamAbsByShadFac.allocate(s_surf->TotSurfaces);
        state.dataSolarShading->SurfWinExtBeamAbsByShadFac.allocate(s_surf->TotSurfaces);
        state.dataSolarShading->SurfWinTransBmSolar.allocate(s_surf->TotSurfaces);
        state.dataSolarShading->SurfWinTransDifSolar.allocate(s_surf->TotSurfaces);
        state.dataSolarShading->SurfWinTransDifSolarGnd.allocate(s_surf->TotSurfaces);
        state.dataSolarShading->SurfWinTransDifSolarSky.allocate(s_surf->TotSurfaces);
        state.dataSolarShading->SurfWinTransBmBmSolar.allocate(s_surf->TotSurfaces);
        state.dataSolarShading->SurfWinTransBmDifSolar.allocate(s_surf->TotSurfaces);
    }
    state.dataSolarShading->SurfAnisoSkyMult.dimension(s_surf->TotSurfaces, 1.0);
    state.dataSolarShading->SurfIntAbsFac.dimension(s_surf->TotSurfaces, 0.0);
    // For isotropic sky: recalculated in AnisoSkyViewFactors if anisotropic radiance
    //  ALLOCATE(WithShdgIsoSky(TotSurfaces))
    //  WithShdgIsoSky=0.0
    //  ALLOCATE(WoShdgIsoSky(TotSurfaces))
    //  WoShdgIsoSky=0.0
    //  ALLOCATE(WithShdgHoriz(TotSurfaces))
    //  WithShdgHoriz=0.0
    //  ALLOCATE(WoShdgHoriz(TotSurfaces))
    //  WoShdgHoriz=0.0
    //  ALLOCATE(DifShdgRatioIsoSky(TotSurfaces))
    //  DifShdgRatioIsoSky=0.0
    //  ALLOCATE(DifShdgRatioHoriz(TotSurfaces))
    //  DifShdgRatioHoriz=0.0
    state.dataSolarShading->SurfMultIsoSky.dimension(s_surf->TotSurfaces, 0.0);
    state.dataSolarShading->SurfMultCircumSolar.dimension(s_surf->TotSurfaces, 0.0);
    state.dataSolarShading->SurfMultHorizonZenith.dimension(s_surf->TotSurfaces, 0.0);
    state.dataSolarShading->SurfWinRevealStatus.dimension(24, state.dataGlobal->TimeStepsInHour, s_surf->TotSurfaces, 0);

    // Weiler-Atherton
    state.dataSolarShading->MAXHCArrayBounds = 2 * (s_surf->MaxVerticesPerSurface + 1);
    state.dataSolarShading->MAXHCArrayIncrement = s_surf->MaxVerticesPerSurface + 1;
    state.dataSolarShading->XTEMP.dimension(2 * (s_surf->MaxVerticesPerSurface + 1), 0.0);
    state.dataSolarShading->YTEMP.dimension(2 * (s_surf->MaxVerticesPerSurface + 1), 0.0);
    state.dataSolarShading->XVC.dimension(s_surf->MaxVerticesPerSurface + 1, 0.0);
    state.dataSolarShading->XVS.dimension(s_surf->MaxVerticesPerSurface + 1, 0.0);
    state.dataSolarShading->YVC.dimension(s_surf->MaxVerticesPerSurface + 1, 0.0);
    state.dataSolarShading->YVS.dimension(s_surf->MaxVerticesPerSurface + 1, 0.0);
    state.dataSolarShading->ZVC.dimension(s_surf->MaxVerticesPerSurface + 1, 0.0);

    // Sutherland-Hodgman
    state.dataSolarShading->ATEMP.dimension(2 * (s_surf->MaxVerticesPerSurface + 1), 0.0);
    state.dataSolarShading->BTEMP.dimension(2 * (s_surf->MaxVerticesPerSurface + 1), 0.0);
    state.dataSolarShading->CTEMP.dimension(2 * (s_surf->MaxVerticesPerSurface + 1), 0.0);
    state.dataSolarShading->XTEMP1.dimension(2 * (s_surf->MaxVerticesPerSurface + 1), 0.0);
    state.dataSolarShading->YTEMP1.dimension(2 * (s_surf->MaxVerticesPerSurface + 1), 0.0);

    s_surf->SurfSunCosHourly.allocate(Constant::iHoursInDay);
    for (int hour = 1; hour <= Constant::iHoursInDay; hour++) {
        s_surf->SurfSunCosHourly(hour) = 0.0;
    }
    s_surf->SurfSunlitArea.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfSunlitFrac.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfSkySolarInc.dimension(s_surf->TotSurfaces, 0);
    s_surf->SurfGndSolarInc.dimension(s_surf->TotSurfaces, 0);
    s_surf->SurfBmToBmReflFacObs.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfBmToDiffReflFacObs.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfBmToDiffReflFacGnd.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfSkyDiffReflFacGnd.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfOpaqAI.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfOpaqAO.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinTransSolar.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinBmSolar.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinBmBmSolar.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinBmDifSolar.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinDifSolar.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinHeatGain.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinHeatGainRep.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinHeatLossRep.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinGainConvGlazToZoneRep.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinGainIRGlazToZoneRep.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinLossSWZoneToOutWinRep.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinGainFrameDividerToZoneRep.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinGainConvShadeToZoneRep.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinGainIRShadeToZoneRep.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinGapConvHtFlowRep.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinShadingAbsorbedSolar.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinSysSolTransmittance.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinSysSolReflectance.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinSysSolAbsorptance.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinInsideGlassCondensationFlag.dimension(s_surf->TotSurfaces, 0);
    s_surf->SurfWinInsideFrameCondensationFlag.dimension(s_surf->TotSurfaces, 0);
    s_surf->SurfWinInsideDividerCondensationFlag.dimension(s_surf->TotSurfaces, 0);

    state.dataHeatBal->SurfSunlitFracHR.dimension(Constant::iHoursInDay, s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfSunlitFrac.dimension(Constant::iHoursInDay, state.dataGlobal->TimeStepsInHour, s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfSunlitFracWithoutReveal.dimension(Constant::iHoursInDay, state.dataGlobal->TimeStepsInHour, s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfWinBackSurfaces.dimension(
        Constant::iHoursInDay, state.dataGlobal->TimeStepsInHour, state.dataBSDFWindow->MaxBkSurf, s_surf->TotSurfaces, 0);
    state.dataHeatBal->SurfWinOverlapAreas.dimension(
        Constant::iHoursInDay, state.dataGlobal->TimeStepsInHour, state.dataBSDFWindow->MaxBkSurf, s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfCosIncAngHR.dimension(Constant::iHoursInDay, s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfCosIncAng.dimension(Constant::iHoursInDay, state.dataGlobal->TimeStepsInHour, s_surf->TotSurfaces, 0.0);

    state.dataHeatBal->ZoneTransSolar.dimension(state.dataViewFactor->NumOfSolarEnclosures, 0.0);
    state.dataHeatBal->ZoneBmSolFrExtWinsRep.dimension(state.dataViewFactor->NumOfSolarEnclosures, 0.0);
    state.dataHeatBal->ZoneBmSolFrIntWinsRep.dimension(state.dataViewFactor->NumOfSolarEnclosures, 0.0);
    state.dataHeatBal->EnclSolInitialDifSolReflW.dimension(state.dataViewFactor->NumOfSolarEnclosures, 0.0);
    state.dataHeatBal->ZoneDifSolFrExtWinsRep.dimension(state.dataViewFactor->NumOfSolarEnclosures, 0.0);
    state.dataHeatBal->ZoneDifSolFrIntWinsRep.dimension(state.dataViewFactor->NumOfSolarEnclosures, 0.0);
    state.dataHeatBal->ZoneWinHeatGain.dimension(state.dataGlobal->NumOfZones, 0.0);
    state.dataHeatBal->ZoneWinHeatGainRep.dimension(state.dataGlobal->NumOfZones, 0.0);
    state.dataHeatBal->ZoneWinHeatLossRep.dimension(state.dataGlobal->NumOfZones, 0.0);
    state.dataHeatBal->ZoneOpaqSurfInsFaceCond.dimension(state.dataGlobal->NumOfZones, 0.0);
    state.dataHeatBal->ZoneOpaqSurfInsFaceCondGainRep.dimension(state.dataGlobal->NumOfZones, 0.0);
    state.dataHeatBal->ZoneOpaqSurfInsFaceCondLossRep.dimension(state.dataGlobal->NumOfZones, 0.0);
    state.dataHeatBal->ZoneOpaqSurfExtFaceCond.dimension(state.dataGlobal->NumOfZones, 0.0);
    state.dataHeatBal->ZoneOpaqSurfExtFaceCondGainRep.dimension(state.dataGlobal->NumOfZones, 0.0);
    state.dataHeatBal->ZoneOpaqSurfExtFaceCondLossRep.dimension(state.dataGlobal->NumOfZones, 0.0);

    state.dataHeatBal->SurfQRadSWOutIncident.dimension(s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfQRadSWOutIncidentBeam.dimension(s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfBmIncInsSurfIntensRep.dimension(s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfBmIncInsSurfAmountRep.dimension(s_surf->TotSurfaces, 0.0);
    //  ALLOCATE(DifIncInsSurfIntensRep(TotSurfaces))
    //  DifIncInsSurfIntensRep=0.0
    //  ALLOCATE(DifIncInsSurfAmountRep(TotSurfaces))
    //  DifIncInsSurfAmountRep=0.0
    state.dataHeatBal->SurfIntBmIncInsSurfIntensRep.dimension(s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfIntBmIncInsSurfAmountRep.dimension(s_surf->TotSurfaces, 0.0);
    //  ALLOCATE(IntDifIncInsSurfIntensRep(TotSurfaces))
    //  IntDifIncInsSurfIntensRep=0.0
    //  ALLOCATE(IntDifIncInsSurfAmountRep(TotSurfaces))
    //  IntDifIncInsSurfAmountRep=0.0
    state.dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse.dimension(s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfQRadSWOutIncidentGndDiffuse.dimension(s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfQRadSWOutIncBmToDiffReflGnd.dimension(s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfQRadSWOutIncSkyDiffReflGnd.dimension(s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfQRadSWOutIncBmToBmReflObs.dimension(s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfQRadSWOutIncBmToDiffReflObs.dimension(s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfQRadSWOutIncSkyDiffReflObs.dimension(s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfCosIncidenceAngle.dimension(s_surf->TotSurfaces, 0.0);

    state.dataHeatBal->SurfWinBSDFBeamDirectionRep.dimension(s_surf->TotSurfaces, 0);
    state.dataHeatBal->SurfWinBSDFBeamThetaRep.dimension(s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfWinBSDFBeamPhiRep.dimension(s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfWinQRadSWwinAbsTot.dimension(s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfWinQRadSWwinAbsLayer.dimension(s_surf->TotSurfaces, state.dataHeatBal->MaxSolidWinLayers, 0.0);
    state.dataHeatBal->SurfWinFenLaySurfTempFront.dimension(s_surf->TotSurfaces, state.dataHeatBal->MaxSolidWinLayers, 0.0);
    state.dataHeatBal->SurfWinFenLaySurfTempBack.dimension(s_surf->TotSurfaces, state.dataHeatBal->MaxSolidWinLayers, 0.0);

    state.dataHeatBal->SurfWinSWwinAbsTotalReport.dimension(s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfInitialDifSolInAbsReport.dimension(s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfWinInitialDifSolInTransReport.dimension(s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfSWInAbsTotalReport.dimension(s_surf->TotSurfaces, 0.0);

    // energy
    s_surf->SurfWinTransSolarEnergy.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinBmSolarEnergy.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinBmBmSolarEnergy.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinBmDifSolarEnergy.dimension(s_surf->TotSurfaces, 0.0);

    s_surf->SurfWinDifSolarEnergy.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinHeatGainRepEnergy.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinHeatLossRepEnergy.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinGapConvHtFlowRepEnergy.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinHeatTransferRepEnergy.dimension(s_surf->TotSurfaces, 0.0);
    s_surf->SurfWinShadingAbsorbedSolarEnergy.dimension(s_surf->TotSurfaces, 0.0);

    state.dataHeatBal->ZoneTransSolarEnergy.dimension(state.dataViewFactor->NumOfSolarEnclosures, 0.0);
    state.dataHeatBal->ZoneBmSolFrExtWinsRepEnergy.dimension(state.dataViewFactor->NumOfSolarEnclosures, 0.0);
    state.dataHeatBal->ZoneBmSolFrIntWinsRepEnergy.dimension(state.dataViewFactor->NumOfSolarEnclosures, 0.0);
    state.dataHeatBal->ZoneDifSolFrExtWinsRepEnergy.dimension(state.dataViewFactor->NumOfSolarEnclosures, 0.0);
    state.dataHeatBal->ZoneDifSolFrIntWinsRepEnergy.dimension(state.dataViewFactor->NumOfSolarEnclosures, 0.0);
    state.dataHeatBal->ZoneWinHeatGainRepEnergy.dimension(state.dataGlobal->NumOfZones, 0.0);
    state.dataHeatBal->ZoneWinHeatLossRepEnergy.dimension(state.dataGlobal->NumOfZones, 0.0);

    state.dataHeatBal->ZnOpqSurfInsFaceCondGnRepEnrg.dimension(state.dataGlobal->NumOfZones, 0.0);
    state.dataHeatBal->ZnOpqSurfInsFaceCondLsRepEnrg.dimension(state.dataGlobal->NumOfZones, 0.0);
    state.dataHeatBal->ZnOpqSurfExtFaceCondGnRepEnrg.dimension(state.dataGlobal->NumOfZones, 0.0);
    state.dataHeatBal->ZnOpqSurfExtFaceCondLsRepEnrg.dimension(state.dataGlobal->NumOfZones, 0.0);
    //  ALLOCATE(DifIncInsSurfAmountRepEnergy(TotSurfaces))
    //  DifIncInsSurfAmountRepEnergy=0.0
    state.dataHeatBal->SurfBmIncInsSurfAmountRepEnergy.dimension(s_surf->TotSurfaces, 0.0);
    state.dataHeatBal->SurfIntBmIncInsSurfAmountRepEnergy.dimension(s_surf->TotSurfaces, 0.0);
    //  ALLOCATE(IntDifIncInsSurfAmountRepEnergy(TotSurfaces))
    //  IntDifIncInsSurfAmountRepEnergy=0.0
    state.dataHeatBal->SurfWinQRadSWwinAbsTotEnergy.dimension(s_surf->TotSurfaces, 0.0);

    for (int SurfNum = 1; SurfNum <= s_surf->TotSurfaces; SurfNum++) {
        s_surf->SurfWinBmSolAbsdOutsReveal(SurfNum) = 0.0;
        s_surf->SurfWinBmSolRefldOutsRevealReport(SurfNum) = 0.0;
        s_surf->SurfWinBmSolAbsdInsReveal(SurfNum) = 0.0;
        s_surf->SurfWinBmSolRefldInsReveal(SurfNum) = 0.0;
        s_surf->SurfWinBmSolRefldInsRevealReport(SurfNum) = 0.0;
        s_surf->SurfWinOutsRevealDiffOntoGlazing(SurfNum) = 0.0;
        s_surf->SurfWinInsRevealDiffOntoGlazing(SurfNum) = 0.0;
        s_surf->SurfWinInsRevealDiffIntoZone(SurfNum) = 0.0;
        s_surf->SurfWinOutsRevealDiffOntoFrame(SurfNum) = 0.0;
        s_surf->SurfWinInsRevealDiffOntoFrame(SurfNum) = 0.0;
    }

    // Added report variables for inside reveal to debug CR 7596. TH 5/26/2009
    for (int SurfNum = 1; SurfNum <= s_surf->TotSurfaces; SurfNum++) {
        s_surf->SurfWinInsRevealDiffOntoGlazingReport(SurfNum) = 0.0;
        s_surf->SurfWinInsRevealDiffIntoZoneReport(SurfNum) = 0.0;
        s_surf->SurfWinInsRevealDiffOntoFrameReport(SurfNum) = 0.0;
        s_surf->SurfWinBmSolAbsdInsRevealReport(SurfNum) = 0.0;
    }

    DisplayString(state, "Initializing Zone and Enclosure Report Variables");
    for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclosureNum) {
        std::string &thisEnclosureName = state.dataViewFactor->EnclSolInfo(enclosureNum).Name;
        SetupOutputVariable(state,
                            "Enclosure Windows Total Transmitted Solar Radiation Rate",
                            Constant::Units::W,
                            state.dataHeatBal->ZoneTransSolar(enclosureNum),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            thisEnclosureName);
        SetupOutputVariable(state,
                            "Enclosure Exterior Windows Total Transmitted Beam Solar Radiation Rate",
                            Constant::Units::W,
                            state.dataHeatBal->ZoneBmSolFrExtWinsRep(enclosureNum),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            thisEnclosureName);
        SetupOutputVariable(state,
                            "Enclosure Interior Windows Total Transmitted Beam Solar Radiation Rate",
                            Constant::Units::W,
                            state.dataHeatBal->ZoneBmSolFrIntWinsRep(enclosureNum),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            thisEnclosureName);
        SetupOutputVariable(state,
                            "Enclosure Exterior Windows Total Transmitted Diffuse Solar Radiation Rate",
                            Constant::Units::W,
                            state.dataHeatBal->ZoneDifSolFrExtWinsRep(enclosureNum),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            thisEnclosureName);
        SetupOutputVariable(state,
                            "Enclosure Interior Windows Total Transmitted Diffuse Solar Radiation Rate",
                            Constant::Units::W,
                            state.dataHeatBal->ZoneDifSolFrIntWinsRep(enclosureNum),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            thisEnclosureName);
        // Energy variables
        SetupOutputVariable(state,
                            "Enclosure Windows Total Transmitted Solar Radiation Energy",
                            Constant::Units::J,
                            state.dataHeatBal->ZoneTransSolarEnergy(enclosureNum),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            thisEnclosureName);
        SetupOutputVariable(state,
                            "Enclosure Exterior Windows Total Transmitted Beam Solar Radiation Energy",
                            Constant::Units::J,
                            state.dataHeatBal->ZoneBmSolFrExtWinsRepEnergy(enclosureNum),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            thisEnclosureName);
        SetupOutputVariable(state,
                            "Enclosure Interior Windows Total Transmitted Beam Solar Radiation Energy",
                            Constant::Units::J,
                            state.dataHeatBal->ZoneBmSolFrIntWinsRepEnergy(enclosureNum),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            thisEnclosureName);
        SetupOutputVariable(state,
                            "Enclosure Exterior Windows Total Transmitted Diffuse Solar Radiation Energy",
                            Constant::Units::J,
                            state.dataHeatBal->ZoneDifSolFrExtWinsRepEnergy(enclosureNum),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            thisEnclosureName);
        SetupOutputVariable(state,
                            "Enclosure Interior Windows Total Transmitted Diffuse Solar Radiation Energy",
                            Constant::Units::J,
                            state.dataHeatBal->ZoneDifSolFrIntWinsRepEnergy(enclosureNum),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            thisEnclosureName);
    }
    for (int ZoneLoop = 1; ZoneLoop <= state.dataGlobal->NumOfZones; ++ZoneLoop) {
        SetupOutputVariable(state,
                            "Zone Windows Total Heat Gain Rate",
                            Constant::Units::W,
                            state.dataHeatBal->ZoneWinHeatGainRep(ZoneLoop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            state.dataHeatBal->Zone(ZoneLoop).Name);
        SetupOutputVariable(state,
                            "Zone Windows Total Heat Loss Rate",
                            Constant::Units::W,
                            state.dataHeatBal->ZoneWinHeatLossRep(ZoneLoop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            state.dataHeatBal->Zone(ZoneLoop).Name);
        SetupOutputVariable(state,
                            "Zone Windows Total Heat Gain Energy",
                            Constant::Units::J,
                            state.dataHeatBal->ZoneWinHeatGainRepEnergy(ZoneLoop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            state.dataHeatBal->Zone(ZoneLoop).Name);
        SetupOutputVariable(state,
                            "Zone Windows Total Heat Loss Energy",
                            Constant::Units::J,
                            state.dataHeatBal->ZoneWinHeatLossRepEnergy(ZoneLoop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            state.dataHeatBal->Zone(ZoneLoop).Name);
        if (state.dataGlobal->DisplayAdvancedReportVariables) {
            // CurrentModuleObject='Zone(Advanced)'
            SetupOutputVariable(state,
                                "Zone Opaque Surface Inside Faces Conduction Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneOpaqSurfInsFaceCond(ZoneLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(ZoneLoop).Name);
            SetupOutputVariable(state,
                                "Zone Opaque Surface Inside Faces Conduction Heat Gain Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneOpaqSurfInsFaceCondGainRep(ZoneLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(ZoneLoop).Name);
            SetupOutputVariable(state,
                                "Zone Opaque Surface Inside Faces Conduction Heat Loss Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneOpaqSurfInsFaceCondLossRep(ZoneLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(ZoneLoop).Name);
            SetupOutputVariable(state,
                                "Zone Opaque Surface Outside Faces Conduction Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneOpaqSurfExtFaceCond(ZoneLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(ZoneLoop).Name);
            SetupOutputVariable(state,
                                "Zone Opaque Surface Outside Faces Conduction Heat Gain Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneOpaqSurfExtFaceCondGainRep(ZoneLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(ZoneLoop).Name);
            SetupOutputVariable(state,
                                "Zone Opaque Surface Outside Faces Conduction Heat Loss Rate",
                                Constant::Units::W,
                                state.dataHeatBal->ZoneOpaqSurfExtFaceCondLossRep(ZoneLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(ZoneLoop).Name);
            // Energy variables
            SetupOutputVariable(state,
                                "Zone Opaque Surface Inside Faces Conduction Heat Gain Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZnOpqSurfInsFaceCondGnRepEnrg(ZoneLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(ZoneLoop).Name);
            SetupOutputVariable(state,
                                "Zone Opaque Surface Inside Faces Conduction Heat Loss Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZnOpqSurfInsFaceCondLsRepEnrg(ZoneLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(ZoneLoop).Name);
            SetupOutputVariable(state,
                                "Zone Opaque Surface Outside Faces Conduction Heat Gain Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZnOpqSurfExtFaceCondGnRepEnrg(ZoneLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(ZoneLoop).Name);
            SetupOutputVariable(state,
                                "Zone Opaque Surface Outside Faces Conduction Heat Loss Energy",
                                Constant::Units::J,
                                state.dataHeatBal->ZnOpqSurfExtFaceCondLsRepEnrg(ZoneLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                state.dataHeatBal->Zone(ZoneLoop).Name);
        }
    }

    DisplayString(state, "Initializing Surface (Shading) Report Variables");
    // CurrentModuleObject='Surfaces'
    for (int SurfLoop = 1; SurfLoop <= s_surf->TotSurfaces; ++SurfLoop) {
        auto &surf = s_surf->Surface(SurfLoop);
        SetupOutputVariable(state,
                            "Surface Outside Normal Azimuth Angle",
                            Constant::Units::deg,
                            surf.Azimuth,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surf.Name);
        if (surf.ExtSolar) {

            SetupOutputVariable(state,
                                "Surface Outside Face Sunlit Area",
                                Constant::Units::m2,
                                s_surf->SurfSunlitArea(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Sunlit Fraction",
                                Constant::Units::None,
                                s_surf->SurfSunlitFrac(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Incident Solar Radiation Rate per Area",
                                Constant::Units::W_m2,
                                state.dataHeatBal->SurfQRadSWOutIncident(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Incident Beam Solar Radiation Rate per Area",
                                Constant::Units::W_m2,
                                state.dataHeatBal->SurfQRadSWOutIncidentBeam(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Incident Sky Diffuse Solar Radiation Rate per Area",
                                Constant::Units::W_m2,
                                state.dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Incident Ground Diffuse Solar Radiation Rate per Area",
                                Constant::Units::W_m2,
                                state.dataHeatBal->SurfQRadSWOutIncidentGndDiffuse(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Beam Solar Incident Angle Cosine Value",
                                Constant::Units::None,
                                state.dataHeatBal->SurfCosIncidenceAngle(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Incident Sky Diffuse Ground Reflected Solar Radiation Rate per Area",
                                Constant::Units::W_m2,
                                state.dataHeatBal->SurfQRadSWOutIncSkyDiffReflGnd(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Incident Sky Diffuse Surface Reflected Solar Radiation Rate per Area",
                                Constant::Units::W_m2,
                                state.dataHeatBal->SurfQRadSWOutIncSkyDiffReflObs(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Incident Beam To Beam Surface Reflected Solar Radiation Rate per Area",
                                Constant::Units::W_m2,
                                state.dataHeatBal->SurfQRadSWOutIncBmToBmReflObs(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Incident Beam To Diffuse Surface Reflected Solar Radiation Rate per Area",
                                Constant::Units::W_m2,
                                state.dataHeatBal->SurfQRadSWOutIncBmToDiffReflObs(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Incident Beam To Diffuse Ground Reflected Solar Radiation Rate per Area",
                                Constant::Units::W_m2,
                                state.dataHeatBal->SurfQRadSWOutIncBmToDiffReflGnd(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
            SetupOutputVariable(state,
                                "Surface Anisotropic Sky Multiplier",
                                Constant::Units::None,
                                state.dataSolarShading->SurfAnisoSkyMult(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
            SetupOutputVariable(state,
                                "Surface Window BSDF Beam Direction Number",
                                Constant::Units::None,
                                state.dataHeatBal->SurfWinBSDFBeamDirectionRep(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
            SetupOutputVariable(state,
                                "Surface Window BSDF Beam Theta Angle",
                                Constant::Units::rad,
                                state.dataHeatBal->SurfWinBSDFBeamThetaRep(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
            SetupOutputVariable(state,
                                "Surface Window BSDF Beam Phi Angle",
                                Constant::Units::rad,
                                state.dataHeatBal->SurfWinBSDFBeamPhiRep(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
        }
        if (!surf.HeatTransSurf) {
            continue;
        }

        if (surf.Class == SurfaceClass::Window) {
            // CurrentModuleObject='Windows/GlassDoors'
            if (surf.ExtSolar) {
                SetupOutputVariable(state,
                                    "Surface Window Total Glazing Layers Absorbed Solar Radiation Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->SurfWinQRadSWwinAbsTot(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Total Glazing Layers Absorbed Shortwave Radiation Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->SurfWinSWwinAbsTotalReport(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);

                if (state.dataConstruction->Construct(surf.Construction).WindowTypeBSDF) {
                    NumOfLayers = state.dataConstruction->Construct(surf.Construction).TotSolidLayers;
                } else {
                    NumOfLayers = state.dataConstruction->Construct(surf.Construction).TotLayers;
                }
                for (I = 1; I <= NumOfLayers; ++I) {
                    if (state.dataConstruction->Construct(surf.Construction).WindowTypeBSDF) {
                        SetupOutputVariable(state,
                                            format("Surface Window Total Absorbed Shortwave Radiation Rate Layer {}", I),
                                            Constant::Units::W,
                                            state.dataHeatBal->SurfWinQRadSWwinAbsLayer(SurfLoop, I),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                    }
                    if (state.dataConstruction->Construct(surf.Construction).WindowTypeBSDF || (I == 1)) {
                        SetupOutputVariable(state,
                                            format("Surface Window Front Face Temperature Layer {}", I),
                                            Constant::Units::C,
                                            state.dataHeatBal->SurfWinFenLaySurfTempFront(SurfLoop, I),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                    }
                    if (state.dataConstruction->Construct(surf.Construction).WindowTypeBSDF || (I == NumOfLayers)) {
                        SetupOutputVariable(state,
                                            format("Surface Window Back Face Temperature Layer {}", I),
                                            Constant::Units::C,
                                            state.dataHeatBal->SurfWinFenLaySurfTempBack(SurfLoop, I),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                    }
                }

                SetupOutputVariable(state,
                                    "Surface Window Transmitted Solar Radiation Rate",
                                    Constant::Units::W,
                                    s_surf->SurfWinTransSolar(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Transmitted Beam Solar Radiation Rate",
                                    Constant::Units::W,
                                    s_surf->SurfWinBmSolar(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);

                // added TH 12/9/2009
                SetupOutputVariable(state,
                                    "Surface Window Transmitted Beam To Beam Solar Radiation Rate",
                                    Constant::Units::W,
                                    s_surf->SurfWinBmBmSolar(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Transmitted Beam To Diffuse Solar Radiation Rate",
                                    Constant::Units::W,
                                    s_surf->SurfWinBmDifSolar(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);

                SetupOutputVariable(state,
                                    "Surface Window Transmitted Diffuse Solar Radiation Rate",
                                    Constant::Units::W,
                                    s_surf->SurfWinDifSolar(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Heat Gain Rate",
                                    Constant::Units::W,
                                    s_surf->SurfWinHeatGainRep(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Heat Loss Rate",
                                    Constant::Units::W,
                                    s_surf->SurfWinHeatLossRep(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Gap Convective Heat Transfer Rate",
                                    Constant::Units::W,
                                    s_surf->SurfWinGapConvHtFlowRep(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Shading Device Absorbed Solar Radiation Rate",
                                    Constant::Units::W,
                                    s_surf->SurfWinShadingAbsorbedSolar(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Net Heat Transfer Rate",
                                    Constant::Units::W,
                                    s_surf->SurfWinHeatGain(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);

                if (state.dataGlobal->DisplayAdvancedReportVariables) {
                    // CurrentModuleObject='Windows/GlassDoors(Advanced)'
                    SetupOutputVariable(state,
                                        "Surface Window Inside Face Glazing Zone Convection Heat Gain Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinGainConvGlazToZoneRep(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Inside Face Glazing Net Infrared Heat Transfer Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinGainIRGlazToZoneRep(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Shortwave from Zone Back Out Window Heat Transfer Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinLossSWZoneToOutWinRep(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Inside Face Frame and Divider Zone Heat Gain Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinGainFrameDividerToZoneRep(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Inside Face Gap between Shade and Glazing Zone Convection Heat Gain Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinConvHeatFlowNatural(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Inside Face Shade Zone Convection Heat Gain Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinGainConvShadeToZoneRep(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Inside Face Shade Net Infrared Heat Transfer Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinGainIRShadeToZoneRep(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    if (state.dataConstruction->Construct(surf.Construction).WindowTypeEQL) {
                        SetupOutputVariable(state,
                                            "Surface Window Inside Face Other Convection Heat Gain Rate",
                                            Constant::Units::W,
                                            s_surf->SurfWinOtherConvHeatGain(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                    }
                }

                // Added TH 12/23/2008 for thermochromic windows
                // CurrentModuleObject='Thermochromic Windows'
                if (state.dataConstruction->Construct(surf.Construction).isTCWindow) {
                    SetupOutputVariable(state,
                                        "Surface Window Thermochromic Layer Temperature",
                                        Constant::Units::C,
                                        s_surf->SurfWinTCLayerTemp(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Thermochromic Layer Property Specification Temperature",
                                        Constant::Units::C,
                                        s_surf->SurfWinSpecTemp(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                }

                // Added TH 5/26/2009 for switchable windows to report switching factor (tinted level)
                // CurrentModuleObject='Switchable Windows'
                if (surf.HasShadeControl) {
                    if (s_surf->WindowShadingControl(surf.activeWindowShadingControl).ShadingType == WinShadingType::SwitchableGlazing) {
                        // IF (SurfaceWindow(SurfLoop)%ShadingFlag == WinShadingType::SwitchableGlazing) THEN  !ShadingFlag is not set to
                        // WinShadingType::SwitchableGlazing yet!
                        SetupOutputVariable(state,
                                            "Surface Window Switchable Glazing Switching Factor",
                                            Constant::Units::None,
                                            s_surf->SurfWinSwitchingFactor(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                        SetupOutputVariable(state,
                                            "Surface Window Switchable Glazing Visible Transmittance",
                                            Constant::Units::None,
                                            s_surf->SurfWinVisTransSelected(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                    }
                }

                if (s_surf->SurfWinFrameArea(SurfLoop) > 0.0) {
                    // CurrentModuleObject='Window Frames'
                    SetupOutputVariable(state,
                                        "Surface Window Frame Heat Gain Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinFrameHeatGain(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Frame Heat Loss Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinFrameHeatLoss(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Frame Inside Temperature",
                                        Constant::Units::C,
                                        s_surf->SurfWinFrameTempIn(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Frame Outside Temperature",
                                        Constant::Units::C,
                                        s_surf->SurfWinFrameTempSurfOut(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                }
                if (s_surf->SurfWinDividerArea(SurfLoop) > 0.0) {
                    // CurrentModuleObject='Window Dividers'
                    SetupOutputVariable(state,
                                        "Surface Window Divider Heat Gain Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinDividerHeatGain(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Divider Heat Loss Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinDividerHeatLoss(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Divider Inside Temperature",
                                        Constant::Units::C,
                                        s_surf->SurfWinDividerTempIn(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Divider Outside Temperature",
                                        Constant::Units::C,
                                        s_surf->SurfWinDividerTempSurfOut(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                }

                // CurrentModuleObject='Windows'
                // Energy
                SetupOutputVariable(state,
                                    "Surface Window Total Glazing Layers Absorbed Solar Radiation Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->SurfWinQRadSWwinAbsTotEnergy(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Transmitted Solar Radiation Energy",
                                    Constant::Units::J,
                                    s_surf->SurfWinTransSolarEnergy(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Transmitted Beam Solar Radiation Energy",
                                    Constant::Units::J,
                                    s_surf->SurfWinBmSolarEnergy(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    surf.Name);

                // added TH 12/9/2009
                SetupOutputVariable(state,
                                    "Surface Window Transmitted Beam To Beam Solar Radiation Energy",
                                    Constant::Units::J,
                                    s_surf->SurfWinBmBmSolarEnergy(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Transmitted Beam To Diffuse Solar Radiation Energy",
                                    Constant::Units::J,
                                    s_surf->SurfWinBmDifSolarEnergy(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    surf.Name);

                SetupOutputVariable(state,
                                    "Surface Window Transmitted Diffuse Solar Radiation Energy",
                                    Constant::Units::J,
                                    s_surf->SurfWinDifSolarEnergy(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Heat Gain Energy",
                                    Constant::Units::J,
                                    s_surf->SurfWinHeatGainRepEnergy(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Heat Loss Energy",
                                    Constant::Units::J,
                                    s_surf->SurfWinHeatLossRepEnergy(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Gap Convective Heat Transfer Energy",
                                    Constant::Units::J,
                                    s_surf->SurfWinGapConvHtFlowRepEnergy(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Shading Device Absorbed Solar Radiation Energy",
                                    Constant::Units::J,
                                    s_surf->SurfWinShadingAbsorbedSolarEnergy(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Net Heat Transfer Energy",
                                    Constant::Units::J,
                                    s_surf->SurfWinHeatTransferRepEnergy(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    surf.Name);

                SetupOutputVariable(state,
                                    "Surface Window System Solar Transmittance",
                                    Constant::Units::None,
                                    s_surf->SurfWinSysSolTransmittance(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window System Solar Reflectance",
                                    Constant::Units::None,
                                    s_surf->SurfWinSysSolReflectance(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window System Solar Absorptance",
                                    Constant::Units::None,
                                    s_surf->SurfWinSysSolAbsorptance(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Inside Face Glazing Condensation Status",
                                    Constant::Units::None,
                                    s_surf->SurfWinInsideGlassCondensationFlag(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Inside Face Frame Condensation Status",
                                    Constant::Units::None,
                                    s_surf->SurfWinInsideFrameCondensationFlag(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Inside Face Divider Condensation Status",
                                    Constant::Units::None,
                                    s_surf->SurfWinInsideDividerCondensationFlag(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);

                // Outside reveal report variables
                // IF (Surface(SurfLoop)%Reveal > 0.0) THEN
                SetupOutputVariable(state,
                                    "Surface Window Outside Reveal Reflected Beam Solar Radiation Rate",
                                    Constant::Units::W,
                                    s_surf->SurfWinBmSolRefldOutsRevealReport(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);
                // Energy
                SetupOutputVariable(state,
                                    "Surface Window Outside Reveal Reflected Beam Solar Radiation Energy",
                                    Constant::Units::J,
                                    s_surf->SurfWinBmSolRefldOutsRevealRepEnergy(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    surf.Name);
                // ENDIF

                // Inside reveal report variables
                if (s_surf->SurfWinInsideReveal(SurfLoop) > 0.0 || s_surf->SurfWinInsideSillDepth(SurfLoop) > 0.0) {
                    SetupOutputVariable(state,
                                        "Surface Window Inside Reveal Reflected Beam Solar Radiation Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinBmSolRefldInsRevealReport(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    // Energy
                    SetupOutputVariable(state,
                                        "Surface Window Inside Reveal Reflected Beam Solar Radiation Energy",
                                        Constant::Units::J,
                                        s_surf->SurfWinBmSolRefldInsRevealRepEnergy(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Sum,
                                        surf.Name);

                    // Added report variables for inside reveal to debug CR 7596. TH 5/26/2009
                    // All reflected solar by the inside reveal is turned into diffuse
                    SetupOutputVariable(state,
                                        "Surface Window Inside Reveal Absorbed Beam Solar Radiation Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinBmSolAbsdInsRevealReport(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Inside Reveal Reflected Diffuse Zone Solar Radiation Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinInsRevealDiffIntoZoneReport(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Inside Reveal Reflected Diffuse Frame Solar Radiation Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinInsRevealDiffOntoFrameReport(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Inside Reveal Reflected Diffuse Glazing Solar Radiation Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinInsRevealDiffOntoGlazingReport(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                }

                //     Output blind report variables only when blinds are used
                auto &surfShade = s_surf->surfShades(SurfLoop);
                if (surfShade.blind.matNum > 0) {
                    // CurrentModuleObject='Window Blinds'
                    SetupOutputVariable(state,
                                        "Surface Window Blind Beam to Beam Solar Transmittance",
                                        Constant::Units::None,
                                        s_surf->SurfWinBlTsolBmBm(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Blind Beam to Diffuse Solar Transmittance",
                                        Constant::Units::None,
                                        s_surf->SurfWinBlTsolBmDif(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Blind Diffuse to Diffuse Solar Transmittance",
                                        Constant::Units::None,
                                        s_surf->SurfWinBlTsolDifDif(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Blind and Glazing System Beam Solar Transmittance",
                                        Constant::Units::None,
                                        s_surf->SurfWinBlGlSysTsolBmBm(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Blind and Glazing System Diffuse Solar Transmittance",
                                        Constant::Units::None,
                                        s_surf->SurfWinBlGlSysTsolDifDif(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                }

                //     Output screen report variables only when screens are used
                if (s_surf->SurfaceWindow(SurfLoop).screenNum > 0) {
                    // CurrentModuleObject='Window Screens'
                    SetupOutputVariable(state,
                                        "Surface Window Screen Beam to Beam Solar Transmittance",
                                        Constant::Units::None,
                                        s_surf->SurfWinScTsolBmBm(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Screen Beam to Diffuse Solar Transmittance",
                                        Constant::Units::None,
                                        s_surf->SurfWinScTsolBmDif(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Screen Diffuse to Diffuse Solar Transmittance",
                                        Constant::Units::None,
                                        s_surf->SurfWinScTsolDifDif(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Screen and Glazing System Beam Solar Transmittance",
                                        Constant::Units::None,
                                        s_surf->SurfWinScGlSysTsolBmBm(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Screen and Glazing System Diffuse Solar Transmittance",
                                        Constant::Units::None,
                                        s_surf->SurfWinScGlSysTsolDifDif(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                }

                // CurrentModuleObject='Windows'
                SetupOutputVariable(state,
                                    "Surface Window Solar Horizontal Profile Angle",
                                    Constant::Units::deg,
                                    s_surf->SurfWinProfileAngHor(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Solar Vertical Profile Angle",
                                    Constant::Units::deg,
                                    s_surf->SurfWinProfileAngVert(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Glazing Beam to Beam Solar Transmittance",
                                    Constant::Units::None,
                                    s_surf->SurfWinGlTsolBmBm(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Glazing Beam to Diffuse Solar Transmittance",
                                    Constant::Units::None,
                                    s_surf->SurfWinGlTsolBmDif(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Glazing Diffuse to Diffuse Solar Transmittance",
                                    Constant::Units::None,
                                    s_surf->SurfWinGlTsolDifDif(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);
                SetupOutputVariable(state,
                                    "Surface Window Model Solver Iteration Count",
                                    Constant::Units::None,
                                    s_surf->SurfWinWindowCalcIterationsRep(SurfLoop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);
            } else { // Not ExtSolar
                if (state.dataGlobal->DisplayAdvancedReportVariables) {
                    // CurrentModuleObject='InteriorWindows(Advanced)'
                    if (surf.OriginalClass != SurfaceClass::TDD_Diffuser) {
                        SetupOutputVariable(state,
                                            "Surface Window Total Glazing Layers Absorbed Solar Radiation Rate",
                                            Constant::Units::W,
                                            state.dataHeatBal->SurfWinQRadSWwinAbsTot(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                    }
                    SetupOutputVariable(state,
                                        "Surface Window Total Glazing Layers Absorbed Shortwave Radiation Rate",
                                        Constant::Units::W,
                                        state.dataHeatBal->SurfWinSWwinAbsTotalReport(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);

                    if (surf.OriginalClass != SurfaceClass::TDD_Diffuser) {
                        SetupOutputVariable(state,
                                            "Surface Window Transmitted Solar Radiation Rate",
                                            Constant::Units::W,
                                            s_surf->SurfWinTransSolar(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                    }
                    SetupOutputVariable(state,
                                        "Surface Window Transmitted Beam Solar Radiation Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinBmSolar(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);

                    // added TH 12/9/2009
                    SetupOutputVariable(state,
                                        "Surface Window Transmitted Beam To Beam Solar Radiation Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinBmBmSolar(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Transmitted Beam To Diffuse Solar Radiation Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinBmDifSolar(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);

                    SetupOutputVariable(state,
                                        "Surface Window Transmitted Diffuse Solar Radiation Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinDifSolar(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Heat Gain Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinHeatGainRep(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Heat Loss Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinHeatLossRep(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Gap Convective Heat Transfer Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinGapConvHtFlowRep(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Shading Device Absorbed Solar Radiation Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinShadingAbsorbedSolar(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    if (s_surf->SurfWinFrameArea(SurfLoop) > 0.0) {
                        SetupOutputVariable(state,
                                            "Surface Window Frame Heat Gain Rate",
                                            Constant::Units::W,
                                            s_surf->SurfWinFrameHeatGain(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                        SetupOutputVariable(state,
                                            "Surface Window Frame Heat Loss Rate",
                                            Constant::Units::W,
                                            s_surf->SurfWinFrameHeatLoss(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                        SetupOutputVariable(state,
                                            "Surface Window Frame Inside Temperature",
                                            Constant::Units::C,
                                            s_surf->SurfWinFrameTempIn(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                        SetupOutputVariable(state,
                                            "Surface Window Frame Outside Temperature",
                                            Constant::Units::C,
                                            s_surf->SurfWinFrameTempSurfOut(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                    }
                    if (s_surf->SurfWinDividerArea(SurfLoop) > 0.0) {
                        SetupOutputVariable(state,
                                            "Surface Window Divider Heat Gain Rate",
                                            Constant::Units::W,
                                            s_surf->SurfWinDividerHeatGain(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                        SetupOutputVariable(state,
                                            "Surface Window Divider Heat Loss Rate",
                                            Constant::Units::W,
                                            s_surf->SurfWinDividerHeatLoss(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                        SetupOutputVariable(state,
                                            "Surface Window Divider Inside Temperature",
                                            Constant::Units::C,
                                            s_surf->SurfWinDividerTempIn(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                        SetupOutputVariable(state,
                                            "Surface Window Divider Outside Temperature",
                                            Constant::Units::C,
                                            s_surf->SurfWinDividerTempSurfOut(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                    }
                    // Energy

                    if (surf.OriginalClass != SurfaceClass::TDD_Diffuser) {
                        SetupOutputVariable(state,
                                            "Surface Window Total Glazing Layers Absorbed Solar Radiation Energy",
                                            Constant::Units::J,
                                            state.dataHeatBal->SurfWinQRadSWwinAbsTotEnergy(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Sum,
                                            surf.Name);
                    }

                    if (surf.OriginalClass != SurfaceClass::TDD_Diffuser) {
                        SetupOutputVariable(state,
                                            "Surface Window Transmitted Solar Radiation Energy",
                                            Constant::Units::J,
                                            s_surf->SurfWinTransSolarEnergy(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Sum,
                                            surf.Name);
                    }
                    SetupOutputVariable(state,
                                        "Surface Window Transmitted Beam Solar Radiation Energy",
                                        Constant::Units::J,
                                        s_surf->SurfWinBmSolarEnergy(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Sum,
                                        surf.Name);

                    SetupOutputVariable(state,
                                        "Surface Window Transmitted Beam To Beam Solar Radiation Energy",
                                        Constant::Units::J,
                                        s_surf->SurfWinBmBmSolarEnergy(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Sum,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Transmitted Beam To Diffuse Solar Radiation Energy",
                                        Constant::Units::J,
                                        s_surf->SurfWinBmDifSolarEnergy(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Sum,
                                        surf.Name);

                    SetupOutputVariable(state,
                                        "Surface Window Transmitted Diffuse Solar Radiation Energy",
                                        Constant::Units::J,
                                        s_surf->SurfWinDifSolarEnergy(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Sum,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Heat Gain Energy",
                                        Constant::Units::J,
                                        s_surf->SurfWinHeatGainRepEnergy(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Sum,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Heat Loss Energy",
                                        Constant::Units::J,
                                        s_surf->SurfWinHeatLossRepEnergy(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Sum,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Gap Convective Heat Transfer Energy",
                                        Constant::Units::J,
                                        s_surf->SurfWinGapConvHtFlowRepEnergy(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Sum,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Shading Device Absorbed Solar Radiation Energy",
                                        Constant::Units::J,
                                        s_surf->SurfWinShadingAbsorbedSolarEnergy(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Sum,
                                        surf.Name);

                    SetupOutputVariable(state,
                                        "Surface Window System Solar Transmittance",
                                        Constant::Units::None,
                                        s_surf->SurfWinSysSolTransmittance(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window System Solar Reflectance",
                                        Constant::Units::None,
                                        s_surf->SurfWinSysSolReflectance(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window System Solar Absorptance",
                                        Constant::Units::None,
                                        s_surf->SurfWinSysSolAbsorptance(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Inside Face Glazing Condensation Status",
                                        Constant::Units::None,
                                        s_surf->SurfWinInsideGlassCondensationFlag(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Inside Face Frame Condensation Status",
                                        Constant::Units::None,
                                        s_surf->SurfWinInsideFrameCondensationFlag(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Inside Face Divider Condensation Status",
                                        Constant::Units::None,
                                        s_surf->SurfWinInsideDividerCondensationFlag(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Outside Reveal Reflected Beam Solar Radiation Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinBmSolRefldOutsRevealReport(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Inside Reveal Reflected Beam Solar Radiation Rate",
                                        Constant::Units::W,
                                        s_surf->SurfWinBmSolRefldInsRevealReport(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    // Energy
                    SetupOutputVariable(state,
                                        "Surface Window Outside Reveal Reflected Beam Solar Radiation Energy",
                                        Constant::Units::J,
                                        s_surf->SurfWinBmSolRefldOutsRevealRepEnergy(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Sum,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Inside Reveal Reflected Beam Solar Radiation Energy",
                                        Constant::Units::J,
                                        s_surf->SurfWinBmSolRefldInsRevealRepEnergy(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Sum,
                                        surf.Name);

                    //     Output blind report variables only when blinds are used
                    auto &surfShade = s_surf->surfShades(SurfLoop);
                    if (surfShade.blind.matNum > 0) {
                        SetupOutputVariable(state,
                                            "Surface Window Blind Beam to Beam Solar Transmittance",
                                            Constant::Units::None,
                                            s_surf->SurfWinBlTsolBmBm(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                        SetupOutputVariable(state,
                                            "Surface Window Blind Beam to Diffuse Solar Transmittance",
                                            Constant::Units::None,
                                            s_surf->SurfWinBlTsolBmDif(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                        SetupOutputVariable(state,
                                            "Surface Window Blind Diffuse to Diffuse Solar Transmittance",
                                            Constant::Units::None,
                                            s_surf->SurfWinBlTsolDifDif(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                        SetupOutputVariable(state,
                                            "Surface Window Blind and Glazing System Beam Solar Transmittance",
                                            Constant::Units::None,
                                            s_surf->SurfWinBlGlSysTsolBmBm(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                        SetupOutputVariable(state,
                                            "Surface Window Blind and Glazing System Diffuse Solar Transmittance",
                                            Constant::Units::None,
                                            s_surf->SurfWinBlGlSysTsolDifDif(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                    }

                    //     Output screen report variables only when screens are used
                    if (s_surf->SurfaceWindow(SurfLoop).screenNum > 0) {
                        SetupOutputVariable(state,
                                            "Surface Window Screen Beam to Beam Solar Transmittance",
                                            Constant::Units::None,
                                            s_surf->SurfWinScTsolBmBm(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                        SetupOutputVariable(state,
                                            "Surface Window Screen Beam to Diffuse Solar Transmittance",
                                            Constant::Units::None,
                                            s_surf->SurfWinScTsolBmDif(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                        SetupOutputVariable(state,
                                            "Surface Window Screen Diffuse to Diffuse Solar Transmittance",
                                            Constant::Units::None,
                                            s_surf->SurfWinScTsolDifDif(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                        SetupOutputVariable(state,
                                            "Surface Window Screen and Glazing System Beam Solar Transmittance",
                                            Constant::Units::None,
                                            s_surf->SurfWinScGlSysTsolBmBm(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                        SetupOutputVariable(state,
                                            "Surface Window Screen and Glazing System Diffuse Solar Transmittance",
                                            Constant::Units::None,
                                            s_surf->SurfWinScGlSysTsolDifDif(SurfLoop),
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            surf.Name);
                    }

                    SetupOutputVariable(state,
                                        "Surface Window Solar Horizontal Profile Angle",
                                        Constant::Units::deg,
                                        s_surf->SurfWinProfileAngHor(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Solar Vertical Profile Angle",
                                        Constant::Units::deg,
                                        s_surf->SurfWinProfileAngVert(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Glazing Beam to Beam Solar Transmittance",
                                        Constant::Units::None,
                                        s_surf->SurfWinGlTsolBmBm(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Glazing Beam to Diffuse Solar Transmittance",
                                        Constant::Units::None,
                                        s_surf->SurfWinGlTsolBmDif(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Glazing Diffuse to Diffuse Solar Transmittance",
                                        Constant::Units::None,
                                        s_surf->SurfWinGlTsolDifDif(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                    SetupOutputVariable(state,
                                        "Surface Window Model Solver Iteration Count",
                                        Constant::Units::None,
                                        s_surf->SurfWinWindowCalcIterationsRep(SurfLoop),
                                        OutputProcessor::TimeStepType::Zone,
                                        OutputProcessor::StoreType::Average,
                                        surf.Name);
                }
            } // end non extsolar reporting as advanced variables
        } // Window Reporting
        if (surf.Class == SurfaceClass::Window && surf.ExtBoundCond > 0 && surf.ExtBoundCond != SurfLoop) { // Interzone window
                                                                                                            // CurrentModuleObject='InterzoneWindows'
            SetupOutputVariable(state,
                                "Surface Window Transmitted Beam Solar Radiation Rate",
                                Constant::Units::W,
                                s_surf->SurfWinBmSolTransThruIntWinRep(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
            // energy
            SetupOutputVariable(state,
                                "Surface Window Transmitted Beam Solar Radiation Energy",
                                Constant::Units::J,
                                s_surf->SurfWinBmSolTransThruIntWinRepEnergy(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                surf.Name);
        }
        if (surf.Class == SurfaceClass::TDD_Dome && surf.ExtSolar) {
            // CurrentModuleObject='TDD Domes'
            SetupOutputVariable(state,
                                "Surface Window Total Glazing Layers Absorbed Solar Radiation Rate",
                                Constant::Units::W,
                                state.dataHeatBal->SurfWinQRadSWwinAbsTot(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
            SetupOutputVariable(state,
                                "Surface Window Transmitted Solar Radiation Rate",
                                Constant::Units::W,
                                s_surf->SurfWinTransSolar(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
            // energy
            SetupOutputVariable(state,
                                "Surface Window Total Glazing Layers Absorbed Solar Radiation Energy",
                                Constant::Units::J,
                                state.dataHeatBal->SurfWinQRadSWwinAbsTotEnergy(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                surf.Name);
            SetupOutputVariable(state,
                                "Surface Window Transmitted Solar Radiation Energy",
                                Constant::Units::J,
                                s_surf->SurfWinTransSolarEnergy(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                surf.Name);
        }
        if (surf.OriginalClass == SurfaceClass::TDD_Diffuser) {
            // CurrentModuleObject='TDD Diffusers'
            SetupOutputVariable(state,
                                "Surface Outside Face Incident Solar Radiation Rate per Area",
                                Constant::Units::W_m2,
                                state.dataHeatBal->SurfQRadSWOutIncident(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
            SetupOutputVariable(state,
                                "Surface Window Total Glazing Layers Absorbed Solar Radiation Rate",
                                Constant::Units::W,
                                state.dataHeatBal->SurfWinQRadSWwinAbsTot(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
            SetupOutputVariable(state,
                                "Surface Window Transmitted Solar Radiation Rate",
                                Constant::Units::W,
                                s_surf->SurfWinTransSolar(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
            // energy
            SetupOutputVariable(state,
                                "Surface Window Total Glazing Layers Absorbed Solar Radiation Energy",
                                Constant::Units::J,
                                state.dataHeatBal->SurfWinQRadSWwinAbsTotEnergy(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                surf.Name);
            SetupOutputVariable(state,
                                "Surface Window Transmitted Solar Radiation Energy",
                                Constant::Units::J,
                                s_surf->SurfWinTransSolarEnergy(SurfLoop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                surf.Name);
        }
    }

    for (int SurfLoop = 1; SurfLoop <= s_surf->TotSurfaces; ++SurfLoop) {
        auto &surf = s_surf->Surface(SurfLoop);
        if (!surf.HeatTransSurf) {
            continue;
        }
        // CurrentModuleObject='Surfaces'
        SetupOutputVariable(state,
                            "Surface Inside Face Exterior Windows Incident Beam Solar Radiation Rate per Area",
                            Constant::Units::W_m2,
                            state.dataHeatBal->SurfBmIncInsSurfIntensRep(SurfLoop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surf.Name);
        SetupOutputVariable(state,
                            "Surface Inside Face Exterior Windows Incident Beam Solar Radiation Rate",
                            Constant::Units::W,
                            state.dataHeatBal->SurfBmIncInsSurfAmountRep(SurfLoop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surf.Name);
        SetupOutputVariable(state,
                            "Surface Inside Face Interior Windows Incident Beam Solar Radiation Rate per Area",
                            Constant::Units::W_m2,
                            state.dataHeatBal->SurfIntBmIncInsSurfIntensRep(SurfLoop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surf.Name);
        SetupOutputVariable(state,
                            "Surface Inside Face Interior Windows Incident Beam Solar Radiation Rate",
                            Constant::Units::W,
                            state.dataHeatBal->SurfIntBmIncInsSurfAmountRep(SurfLoop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surf.Name);
        SetupOutputVariable(state,
                            "Surface Inside Face Initial Transmitted Diffuse Absorbed Solar Radiation Rate",
                            Constant::Units::W,
                            state.dataHeatBal->SurfInitialDifSolInAbsReport(SurfLoop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surf.Name);
        SetupOutputVariable(state,
                            "Surface Inside Face Initial Transmitted Diffuse Transmitted Out Window Solar Radiation Rate",
                            Constant::Units::W,
                            state.dataHeatBal->SurfWinInitialDifSolInTransReport(SurfLoop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surf.Name);
        SetupOutputVariable(state,
                            "Surface Inside Face Absorbed Shortwave Radiation Rate",
                            Constant::Units::W,
                            state.dataHeatBal->SurfSWInAbsTotalReport(SurfLoop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surf.Name);
        // energy
        SetupOutputVariable(state,
                            "Surface Inside Face Exterior Windows Incident Beam Solar Radiation Energy",
                            Constant::Units::J,
                            state.dataHeatBal->SurfBmIncInsSurfAmountRepEnergy(SurfLoop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            surf.Name);
        SetupOutputVariable(state,
                            "Surface Inside Face Interior Windows Incident Beam Solar Radiation Energy",
                            Constant::Units::J,
                            state.dataHeatBal->SurfIntBmIncInsSurfAmountRepEnergy(SurfLoop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            surf.Name);
    }
}

void AnisoSkyViewFactors(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   April 1999
    //       MODIFIED       LKL; Dec 2002 -- Anisotropic is only sky radiance option
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates view factor multiplier, SurfAnisoSkyMult, for diffuse
    // sky irradiance on exterior surfaces taking into account
    // anisotropic radiance of the sky. Called by InitSurfaceHeatBalance
    // In this case the diffuse sky irradiance on a surface is given by
    //  SurfAnisoSkyMult(SurfNum) * DifSolarRad
    // SurfAnisoSkyMult accounts not only for the sky radiance distribution but
    // also for the effects of shading of sky diffuse radiation by
    // shadowing surfaces such as overhangs. It does not account for reflection
    // of sky diffuse radiation from shadowing surfaces.
    // Based on an empirical model described in
    // R. Perez, P. Ineichen, R. Seals, J. Michalsky and R. Stewart,
    // "Modeling Daylight Availability and Irradiance Components from Direct
    // and Global Irradiance," Solar Energy 44, 271-289, 1990.
    // In this model the radiance of the sky consists of three distributions
    // that are superimposed:

    // (1) An isotropic distribution that covers the entire sky dome;
    // (2) A circumsolar brightening centered around the position of the sun;
    // (3) A horizon brightening
    // The circumsolar brightening is assumed to be concentrated at a point
    // source at the center of the sun although this region actually begins at the
    // periphery of the solar disk and falls off in intensity with increasing
    // angular distance from the periphery.
    // The horizon brightening is assumed to be concentrated at the horizon and
    // to be independent of azimuth. In actuality, for clear skies, the horizon
    // brightening is highest at the horizon and decreases in intensity away from
    // the horizon. For overcast skies the horizon brightening has a negative value
    // since for such skies the sky radiance increases rather than decreases away
    // from the horizon.
    // The F11R, F12R, etc. values were provided by R. Perez, private communication,
    // 5/21/99. These values have higher precision than those listed in the above
    // paper.

    // Using/Aliasing

    // Locals
    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::array<Real64, 7> EpsilonLimit = {
        1.065, 1.23, 1.5, 1.95, 2.8, 4.5, 6.2}; // Upper limit of bins of the sky clearness parameter, Epsilon
    // Circumsolar brightening coefficients; index corresponds to range of Epsilon, the sky clearness parameter
    static constexpr std::array<Real64, 8> F11R = {-0.0083117, 0.1299457, 0.3296958, 0.5682053, 0.8730280, 1.1326077, 1.0601591, 0.6777470};
    static constexpr std::array<Real64, 8> F12R = {0.5877285, 0.6825954, 0.4868735, 0.1874525, -0.3920403, -1.2367284, -1.5999137, -0.3272588};
    static constexpr std::array<Real64, 8> F13R = {-0.0620636, -0.1513752, -0.2210958, -0.2951290, -0.3616149, -0.4118494, -0.3589221, -0.2504286};
    // Horizon/zenith brightening coefficient array; index corresponds to range of Epsilon, the sky clearness parameter
    static constexpr std::array<Real64, 8> F21R = {-0.0596012, -0.0189325, 0.0554140, 0.1088631, 0.2255647, 0.2877813, 0.2642124, 0.1561313};
    static constexpr std::array<Real64, 8> F22R = {0.0721249, 0.0659650, -0.0639588, -0.1519229, -0.4620442, -0.8230357, -1.1272340, -1.3765031};
    static constexpr std::array<Real64, 8> F23R = {-0.0220216, -0.0288748, -0.0260542, -0.0139754, 0.0012448, 0.0558651, 0.1310694, 0.2506212};

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    Real64 CosZenithAng;           // Cosine of solar zenith angle
    Real64 ZenithAng;              // Solar zenith angle (radians)
    Real64 ZenithAngDeg;           // Solar zenith angle (degrees)
    Real64 F1;                     // Circumsolar brightening coefficient
    Real64 F2;                     // Horizon/zenith brightening coefficient
    Real64 Epsilon;                // Sky clearness parameter
    Real64 Delta;                  // Sky brightness parameter
    Real64 CosIncAngBeamOnSurface; // Cosine of incidence angle of beam solar on surface
    int EpsilonBin;                // Sky clearness (Epsilon) bin index
    Real64 AirMass;                // Relative air mass
    Real64 AirMassH;               // Intermediate variable for relative air mass calculation
    Real64 CircumSolarFac;         // Ratio of cosine of incidence angle to cosine of zenith angle
    Real64 KappaZ3;                // Intermediate variable
    Real64 ViewFactorSkyGeom;      // Geometrical sky view factor
    Real64 constexpr cosine_tolerance(0.0001);

    auto &s_surf = state.dataSurface;

#ifdef EP_Count_Calls
    ++state.dataTimingsData->NumAnisoSky_Calls;
#endif

    CosZenithAng = state.dataEnvrn->SOLCOS(3);
    ZenithAng = std::acos(CosZenithAng);
    ZenithAngDeg = ZenithAng / Constant::DegToRad;

    state.dataSolarShading->SurfAnisoSkyMult = 0.0;

    //           Relative air mass
    AirMassH = (1.0 - 0.1 * state.dataEnvrn->Elevation / 1000.0);
    if (ZenithAngDeg <= 75.0) {
        AirMass = AirMassH / CosZenithAng;
    } else {
        AirMass = AirMassH / (CosZenithAng + 0.15 * std::pow(93.9 - ZenithAngDeg, -1.253));
    }
    KappaZ3 = 1.041 * pow_3(ZenithAng);
    Epsilon = ((state.dataEnvrn->BeamSolarRad + state.dataEnvrn->DifSolarRad) / state.dataEnvrn->DifSolarRad + KappaZ3) / (1.0 + KappaZ3);
    Delta = state.dataEnvrn->DifSolarRad * AirMass / 1353.0; // 1353 is average extraterrestrial irradiance (W/m2)
    //           Circumsolar (F1) and horizon/zenith (F2) brightening coefficients
    for (EpsilonBin = 0; EpsilonBin < 8; ++EpsilonBin) {
        if (EpsilonBin == 7) {
            break;
        }
        if (Epsilon < EpsilonLimit[EpsilonBin]) {
            break;
        }
    }
    F1 = max(0.0, F11R[EpsilonBin] + F12R[EpsilonBin] * Delta + F13R[EpsilonBin] * ZenithAng);
    F2 = F21R[EpsilonBin] + F22R[EpsilonBin] * Delta + F23R[EpsilonBin] * ZenithAng;

    for (int SurfNum : s_surf->AllExtSolarSurfaceList) {

        CosIncAngBeamOnSurface = state.dataEnvrn->SOLCOS(1) * s_surf->Surface(SurfNum).OutNormVec(1) +
                                 state.dataEnvrn->SOLCOS(2) * s_surf->Surface(SurfNum).OutNormVec(2) +
                                 state.dataEnvrn->SOLCOS(3) * s_surf->Surface(SurfNum).OutNormVec(3);

        // So I believe this should only be a diagnostic error...the calcs should always be within -1,+1; it's just round-off that we need to trap
        // for
        if (CosIncAngBeamOnSurface > 1.0) {
            if (CosIncAngBeamOnSurface > (1.0 + cosine_tolerance)) {
                ShowSevereError(state, "Cosine of incident angle of beam solar on surface out of range...too high");
                ShowContinueError(state, "This is a diagnostic error that should not be encountered under normal circumstances");
                ShowContinueError(state, format("Occurs on surface: {}", s_surf->Surface(SurfNum).Name));
                ShowContinueError(state, format("Current value = {} ... should be within [-1, +1]", CosIncAngBeamOnSurface));
                ShowFatalError(state, "Anisotropic solar calculation causes fatal error");
            }
            CosIncAngBeamOnSurface = 1.0;
        } else if (CosIncAngBeamOnSurface < -1.0) {
            if (CosIncAngBeamOnSurface < (-1.0 - cosine_tolerance)) {
                ShowSevereError(state, "Cosine of incident angle of beam solar on surface out of range...too low");
                ShowContinueError(state, "This is a diagnostic error that should not be encountered under normal circumstances");
                ShowContinueError(state, format("Occurs on surface: {}", s_surf->Surface(SurfNum).Name));
                ShowContinueError(state, format("Current value = {} ... should be within [-1, +1]", CosIncAngBeamOnSurface));
                ShowFatalError(state, "Anisotropic solar calculation causes fatal error");
            }
            CosIncAngBeamOnSurface = -1.0;
        }

        ViewFactorSkyGeom = s_surf->Surface(SurfNum).ViewFactorSky;
        state.dataSolarShading->SurfMultIsoSky(SurfNum) = ViewFactorSkyGeom * (1.0 - F1);
        //           0.0871557 below corresponds to a zenith angle of 85 deg
        CircumSolarFac = max(0.0, CosIncAngBeamOnSurface) / max(0.0871557, CosZenithAng);
        //           For near-horizontal roofs, model has an inconsistency that gives sky diffuse
        //           irradiance significantly different from DifSolarRad when zenith angle is
        //           above 85 deg. The following forces irradiance to be very close to DifSolarRad
        //           in this case.
        if (CircumSolarFac > 0.0 && CosZenithAng < 0.0871557 && s_surf->Surface(SurfNum).Tilt < 2.0) {
            CircumSolarFac = 1.0;
        }
        state.dataSolarShading->SurfMultCircumSolar(SurfNum) = F1 * CircumSolarFac;
        state.dataSolarShading->SurfMultHorizonZenith(SurfNum) = F2 * s_surf->Surface(SurfNum).SinTilt;

        if (!state.dataSysVars->DetailedSkyDiffuseAlgorithm || !s_surf->ShadingTransmittanceVaries ||
            state.dataHeatBal->SolarDistribution == DataHeatBalance::Shadowing::Minimal) {
            state.dataSolarShading->SurfAnisoSkyMult(SurfNum) =
                state.dataSolarShading->SurfMultIsoSky(SurfNum) * state.dataSolarShading->SurfDifShdgRatioIsoSky(SurfNum) +
                state.dataSolarShading->SurfMultCircumSolar(SurfNum) *
                    state.dataHeatBal->SurfSunlitFrac(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum) +
                state.dataSolarShading->SurfMultHorizonZenith(SurfNum) * state.dataSolarShading->SurfDifShdgRatioHoriz(SurfNum);
        } else {
            state.dataSolarShading->SurfAnisoSkyMult(SurfNum) =
                state.dataSolarShading->SurfMultIsoSky(SurfNum) *
                    state.dataSolarShading->SurfDifShdgRatioIsoSkyHRTS(state.dataGlobal->TimeStep, state.dataGlobal->HourOfDay, SurfNum) +
                state.dataSolarShading->SurfMultCircumSolar(SurfNum) *
                    state.dataHeatBal->SurfSunlitFrac(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum) +
                state.dataSolarShading->SurfMultHorizonZenith(SurfNum) *
                    state.dataSolarShading->SurfDifShdgRatioHorizHRTS(state.dataGlobal->TimeStep, state.dataGlobal->HourOfDay, SurfNum);
            state.dataSolarShading->SurfCurDifShdgRatioIsoSky(SurfNum) =
                state.dataSolarShading->SurfDifShdgRatioIsoSkyHRTS(state.dataGlobal->TimeStep, state.dataGlobal->HourOfDay, SurfNum);
        }
        state.dataSolarShading->SurfAnisoSkyMult(SurfNum) = max(0.0, state.dataSolarShading->SurfAnisoSkyMult(SurfNum)); // make sure not negative.
    }
}

void CHKBKS(EnergyPlusData &state,
            int const NBS, // Surface Number of the potential back surface
            int const NRS  // Surface Number of the potential shadow receiving surface
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy Code
    //       DATE WRITTEN
    //       MODIFIED       Nov 2001, FW: Reverse subroutine arguments NRS and NBS to
    //                                    correspond to how CHKBKS is called
    //                      Jan 2002, FW: change error message
    //       RE-ENGINEERED  Lawrie, Oct 2000
    //       Sep 2020: Revised the vector computation method to reliably produce CVec,
    //                 and simplified the warning messages.

    // PURPOSE OF THIS SUBROUTINE:
    // Determines whether a any vertices of the back surface are in front of the receiving surface;
    // if so, gives severe error.  Only base heat transfer surfaces are checked.

    // METHODOLOGY EMPLOYED:
    // na

    // REFERENCES:
    // BLAST/IBLAST code, original author George Walton

    // Using/Aliasing
    using namespace Vectors;

    int N;       // Loop Control (vertex counter)
    int NVRS;    // Number of vertices of the receiving surface
    int NVBS;    // Number of vertices of the back surface
    Real64 DOTP; // Dot product of C and D

    // Object Data
    Vector CVec(0.0); // Vector perpendicular to surface at vertex 1
    Vector DVec(0.0); // Vector from vertex 1 of first surface to vertex 'n' of second surface

    auto &s_surf = state.dataSurface;

    NVRS = s_surf->Surface(NRS).Sides;
    NVBS = s_surf->Surface(NBS).Sides;

    // SEE IF ANY VERTICES OF THE back surface ARE IN FRONT OF THE receiving surface

    for (N = 2; N < NVRS; N++) {
        CVec += cross(s_surf->Surface(NRS).Vertex(N) - s_surf->Surface(NRS).Vertex(1),
                      s_surf->Surface(NRS).Vertex((N + 1)) - s_surf->Surface(NRS).Vertex(1));
    }
    CVec /= (NVRS >= 3 ? NVRS : 3);

    for (N = 1; N <= NVBS; ++N) {
        DVec = s_surf->Surface(NBS).Vertex(N) - s_surf->Surface(NRS).Vertex(1);
        DOTP = dot(CVec, DVec);
        if (DOTP > 0.0009) {
            ShowSevereError(state, "Problem in interior solar distribution calculation (CHKBKS)");
            ShowContinueError(state, format("   Solar Distribution = FullInteriorExterior will not work in Zone={}", s_surf->Surface(NRS).ZoneName));
            ShowContinueError(state,
                              format("   because one or more of vertices, such as Vertex {} of back surface={}, is in front of receiving surface={}",
                                     N,
                                     s_surf->Surface(NBS).Name,
                                     s_surf->Surface(NRS).Name));
            ShowContinueError(state, format("   (Dot Product indicator={:20.4F})", DOTP));
            ShowContinueError(state,
                              "   Check surface geometry; if OK, use Solar Distribution = FullExterior instead. Use Output:Diagnostics, "
                              "DisplayExtraWarnings; for more details.");
            if (!state.dataGlobal->DisplayExtraWarnings) {
                break;
            }
        }
    }
}

void CHKGSS(EnergyPlusData &state,
            int const NRS,     // Surface number of the potential shadow receiving surface
            int const NSS,     // Surface number of the potential shadow casting surface
            Real64 const ZMIN, // Lowest point of the receiving surface
            bool &CannotShade  // TRUE if shadow casting surface cannot shade receiving surface.
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy Code
    //       DATE WRITTEN
    //       MODIFIED       na
    //       RE-ENGINEERED  Lawrie, Oct 2000

    // PURPOSE OF THIS SUBROUTINE:
    // Determines the possible shadowing combinations.  The
    // routine checks detached shadowing or base heat transfer surfaces
    // for the possibility that they cannot shade a given base heat transfer surface.

    // METHODOLOGY EMPLOYED:
    // Shadowing is not possible if:
    // 1.  The lowest point of the shadow receiving surface (receiving surface)
    //     Is higher than the highest point of the shadow casting surface (s.s.)
    // 2.  The shadow casting surface Faces up (e.g. A flat roof)
    // 3.  The shadow casting surface Is behind the receiving surface
    // 4.  The receiving surface is behind the shadow casting surface

    // REFERENCES:
    // BLAST/IBLAST code, original author George Walton

    // Using/Aliasing
    using namespace Vectors;

    // Object Data

    CannotShade = true;

    auto &s_surf = state.dataSurface;

    // see if no point of shadow casting surface is above low point of receiving surface

    auto const &surface_C = s_surf->Surface(NSS);
    if (surface_C.OutNormVec(3) > 0.9999) {
        return; // Shadow Casting Surface is horizontal and facing upward
    }
    auto const &vertex_C = surface_C.Vertex;
    Real64 ZMAX(vertex_C(1).z);
    for (int i = 2, e = surface_C.Sides; i <= e; ++i) {
        ZMAX = std::max(ZMAX, vertex_C(i).z);
    }
    if (ZMAX <= ZMIN) {
        return;
    }

    // SEE IF ANY VERTICES OF THE Shadow Casting Surface ARE ABOVE THE PLANE OF THE receiving surface

    auto const &surface_R = s_surf->Surface(NRS);
    auto const &vertex_R = surface_R.Vertex;
    Vector const &vertex_R_2 = vertex_R(2);
    Vector const AVec = (vertex_R(1) - vertex_R_2); // Vector from vertex 2 to vertex 1 of receiving surface
    Vector const BVec = (vertex_R(3) - vertex_R_2); // Vector from vertex 2 to vertex 3 of receiving surface

    Vector const CVec = cross(BVec, AVec); // Vector perpendicular to surface at vertex 2

    int const NVSS = surface_C.Sides; // Number of vertices of the shadow casting surface
    Real64 DOTP(0.0);                 // Dot Product
    for (int I = 1; I <= NVSS; ++I) {
        DOTP = dot(CVec, vertex_C(I) - vertex_R_2);
        if (DOTP > state.dataSolarShading->TolValue) {
            break; // DO loop
        }
    }

    // SEE IF ANY VERTICES OF THE receiving surface ARE ABOVE THE PLANE OF THE S.S.

    if (DOTP > state.dataSolarShading->TolValue) {

        Vector const &vertex_C_2 = vertex_C(2);
        Vector const AVec(vertex_C(1) - vertex_C_2);
        Vector const BVec(vertex_C(3) - vertex_C_2);

        Vector const CVec(cross(BVec, AVec));

        int const NVRS = surface_R.Sides; // Number of vertices of the receiving surface
        for (int I = 1; I <= NVRS; ++I) {
            DOTP = dot(CVec, vertex_R(I) - vertex_C_2);
            if (DOTP > state.dataSolarShading->TolValue) {
                CannotShade = false;
                break; // DO loop
            }
        }
    }
}

void CHKSBS(EnergyPlusData &state,
            int const HTS,   // Heat transfer surface number of the general receiving surf
            int const GRSNR, // Surface number of general receiving surface
            int const SBSNR  // Surface number of subsurface
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy Code
    //       DATE WRITTEN
    //       MODIFIED       na
    //       RE-ENGINEERED  Lawrie, Oct 2000

    // PURPOSE OF THIS SUBROUTINE:
    // Checks that a subsurface is completely
    // enclosed by its base surface.

    // REFERENCES:
    // BLAST/IBLAST code, original author George Walton

    // 3D Planar Polygons
    // In 3D applications, one sometimes wants to test a point and polygon that are in the same plane.
    // For example, one may have the intersection point of a ray with the plane of a polyhedron's face,
    // and want to test if it is inside the face.  Or one may want to know if the base of a 3D perpendicular
    // dropped from a point is inside a planar polygon.

    // 3D inclusion is easily determined by projecting the point and polygon into 2D.  To do this, one simply
    // ignores one of the 3D coordinates and uses the other two.  To optimally select the coordinate to ignore,
    // compute a normal vector to the plane, and select the coordinate with the largest absolute value [Snyder & Barr, 1987].
    // This gives the projection of the polygon with maximum area, and results in robust computations.
    // John M. Snyder & Alan H. Barr, "Ray Tracing Complex Models Containing Surface Tessellations",
    // Computer Graphics 21(4), 119-126 (1987) [also in the Proceedings of SIGGRAPH 1987]
    //--- using adapted routine from Triangulation code -- EnergyPlus.

    // MSG - for error message
    static Array1D_string const MSG(4, {"misses", "", "within", "overlaps"});

    int N;   // Loop Control
    int NVT; // Number of vertices
    int NS1; // Number of the figure being overlapped
    int NS2; // Number of the figure doing overlapping
    int NS3; // Location to place results of overlap

    bool inside;

    bool Out;
    Real64 X1; // ,SX,SY,SZ
    Real64 Y1;
    Real64 Z1;
    Real64 X2;
    Real64 Y2;
    Real64 Z2;
    Real64 BX;
    Real64 BY;
    Real64 BZ;
    Real64 BMAX;
    //  INTEGER M

    auto &s_surf = state.dataSurface;

    if (state.dataSolarShading->CHKSBSOneTimeFlag) {
        state.dataSolarShading->XVT.allocate(s_surf->MaxVerticesPerSurface + 1);
        state.dataSolarShading->YVT.allocate(s_surf->MaxVerticesPerSurface + 1);
        state.dataSolarShading->ZVT.allocate(s_surf->MaxVerticesPerSurface + 1);
        state.dataSolarShading->XVT = 0.0;
        state.dataSolarShading->YVT = 0.0;
        state.dataSolarShading->ZVT = 0.0;
        state.dataSolarShading->CHKSBSOneTimeFlag = false;
    }

    NS1 = 1;
    NS2 = 2;
    NS3 = 3;
    state.dataSolarShading->HCT(1) = 0.0;
    state.dataSolarShading->HCT(2) = 0.0;

    // Put coordinates of base surface into clockwise sequence on the x'-y' plane.

    state.dataSolarShading->XVT = 0.0;
    state.dataSolarShading->YVT = 0.0;
    state.dataSolarShading->ZVT = 0.0;
    state.dataSolarShading->XVS = 0.0;
    state.dataSolarShading->YVS = 0.0;
    CTRANS(state, GRSNR, HTS, NVT, state.dataSolarShading->XVT, state.dataSolarShading->YVT, state.dataSolarShading->ZVT);
    for (N = 1; N <= NVT; ++N) {
        state.dataSolarShading->XVS(N) = state.dataSolarShading->XVT(NVT + 1 - N);
        state.dataSolarShading->YVS(N) = state.dataSolarShading->YVT(NVT + 1 - N);
    }

    HTRANS1(state, NS2, NVT);

    // Put coordinates of the subsurface into clockwise sequence.

    state.dataSolarShading->NVS = s_surf->Surface(SBSNR).Sides;
    for (N = 1; N <= state.dataSolarShading->NVS; ++N) {
        state.dataSolarShading->XVS(N) = s_surf->ShadeV(SBSNR).XV(state.dataSolarShading->NVS + 1 - N);
        state.dataSolarShading->YVS(N) = s_surf->ShadeV(SBSNR).YV(state.dataSolarShading->NVS + 1 - N);
    }
    HTRANS1(state, NS1, state.dataSolarShading->NVS);

    // Determine the overlap condition.

    DeterminePolygonOverlap(state, NS1, NS2, NS3);

    // Print error condition if necessary.

    if (state.dataSolarShading->OverlapStatus != FirstSurfWithinSecond) {
        Out = false;
        // C                            COMPUTE COMPONENTS OF VECTOR
        // C                            NORMAL TO BASE SURFACE.
        X1 = s_surf->Surface(GRSNR).Vertex(1).x - s_surf->Surface(GRSNR).Vertex(2).x; // XV(1,GRSNR)-XV(2,GRSNR)
        Y1 = s_surf->Surface(GRSNR).Vertex(1).y - s_surf->Surface(GRSNR).Vertex(2).y; // YV(1,GRSNR)-YV(2,GRSNR)
        Z1 = s_surf->Surface(GRSNR).Vertex(1).z - s_surf->Surface(GRSNR).Vertex(2).z; // ZV(1,GRSNR)-ZV(2,GRSNR)
        X2 = s_surf->Surface(GRSNR).Vertex(3).x - s_surf->Surface(GRSNR).Vertex(2).x; // XV(3,GRSNR)-XV(2,GRSNR)
        Y2 = s_surf->Surface(GRSNR).Vertex(3).y - s_surf->Surface(GRSNR).Vertex(2).y; // YV(3,GRSNR)-YV(2,GRSNR)
        Z2 = s_surf->Surface(GRSNR).Vertex(3).z - s_surf->Surface(GRSNR).Vertex(2).z; // ZV(3,GRSNR)-ZV(2,GRSNR)
        BX = Y1 * Z2 - Y2 * Z1;
        BY = Z1 * X2 - Z2 * X1;
        BZ = X1 * Y2 - X2 * Y1;
        // C                            FIND LARGEST COMPONENT.
        BMAX = max(std::abs(BX), std::abs(BY), std::abs(BZ));
        // C
        if (std::abs(BX) == BMAX) {
            //        write(outputfiledebug,*) ' looking bx-bmax',bmax
            for (N = 1; N <= s_surf->Surface(SBSNR).Sides; ++N) { // NV(SBSNR)
                inside = polygon_contains_point(
                    s_surf->Surface(GRSNR).Sides, s_surf->Surface(GRSNR).Vertex, s_surf->Surface(SBSNR).Vertex(N), true, false, false);
                if (!inside) {
                    Out = true;
                    //            do m=1,surface(grsnr)%sides
                    //            write(outputfiledebug,*) 'grsnr,side=',m,surface(grsnr)%vertex
                    //            write(outputfiledebug,*) 'point outside=',surface(sbsnr)%vertex(n)
                    //            enddo
                    //            EXIT
                }
                //          Y1 = Surface(GRSNR)%Vertex(Surface(GRSNR)%Sides)%Y-Surface(SBSNR)%Vertex(N)%Y !YV(NV(GRSNR),GRSNR)-YV(N,SBSNR)
                //          Z1 = Surface(GRSNR)%Vertex(Surface(GRSNR)%Sides)%Z-Surface(SBSNR)%Vertex(N)%Z !ZV(NV(GRSNR),GRSNR)-ZV(N,SBSNR)
                //          DO M=1,Surface(GRSNR)%Sides !NV(GRSNR)
                //            Y2 = Y1
                //            Z2 = Z1
                //            Y1 = Surface(GRSNR)%Vertex(M)%Y-Surface(SBSNR)%Vertex(N)%Y !YV(M,GRSNR)-YV(N,SBSNR)
                //            Z1 = Surface(GRSNR)%Vertex(M)%Z-Surface(SBSNR)%Vertex(N)%Z !ZV(M,GRSNR)-ZV(N,SBSNR)
                //            SX = Y1*Z2-Y2*Z1
                //            IF(SX*BX.LT.-1.0d-6) THEN
                //              OUT=.TRUE.
                //              write(outputfiledebug,*) 'sx*bx=',sx*bx
                //              write(outputfiledebug,*) 'grsnr=',surface(grsnr)%vertex(m)
                //              write(outputfiledebug,*) 'sbsnr=',surface(sbsnr)%vertex(n)
                //            endif
                //          ENDDO
                //          IF (OUT) EXIT
            }
        } else if (std::abs(BY) == BMAX) {
            //        write(outputfiledebug,*) ' looking by-bmax',bmax
            for (N = 1; N <= s_surf->Surface(SBSNR).Sides; ++N) { // NV(SBSNR)
                inside = polygon_contains_point(
                    s_surf->Surface(GRSNR).Sides, s_surf->Surface(GRSNR).Vertex, s_surf->Surface(SBSNR).Vertex(N), false, true, false);
                if (!inside) {
                    Out = true;
                    //            do m=1,surface(grsnr)%sides
                    //            write(outputfiledebug,*) 'grsnr,side=',m,surface(grsnr)%vertex
                    //            write(outputfiledebug,*) 'point outside=',surface(sbsnr)%vertex(n)
                    //            enddo
                    //            EXIT
                }
                //          Z1 = Surface(GRSNR)%Vertex(Surface(GRSNR)%Sides)%Z-Surface(SBSNR)%Vertex(N)%Z !ZV(NV(GRSNR),GRSNR)-ZV(N,SBSNR)
                //          X1 = Surface(GRSNR)%Vertex(Surface(GRSNR)%Sides)%X-Surface(SBSNR)%Vertex(N)%X !XV(NV(GRSNR),GRSNR)-XV(N,SBSNR)
                //          DO M=1,Surface(GRSNR)%Sides !NV(GRSNR)
                //            Z2 = Z1
                //            X2 = X1
                //            Z1 = Surface(GRSNR)%Vertex(M)%Z-Surface(SBSNR)%Vertex(N)%Z !ZV(M,GRSNR)-ZV(N,SBSNR)
                //            X1 = Surface(GRSNR)%Vertex(M)%X-Surface(SBSNR)%Vertex(N)%X !XV(M,GRSNR)-XV(N,SBSNR)
                //            SY = Z1*X2-Z2*X1
                //            IF(SY*BY.LT.-1.0d-6) THEN
                //              OUT=.TRUE.
                //              write(outputfiledebug,*) 'sy*by=',sy*by
                //              write(outputfiledebug,*) 'grsnr=',surface(grsnr)%vertex(m)
                //              write(outputfiledebug,*) 'sbsnr=',surface(sbsnr)%vertex(n)
                //            ENDIF
                //          ENDDO
                //          IF (OUT) EXIT
            }
        } else {
            //        write(outputfiledebug,*) ' looking bz-bmax',bmax
            for (N = 1; N <= s_surf->Surface(SBSNR).Sides; ++N) { // NV(SBSNR)
                inside = polygon_contains_point(
                    s_surf->Surface(GRSNR).Sides, s_surf->Surface(GRSNR).Vertex, s_surf->Surface(SBSNR).Vertex(N), false, false, true);
                if (!inside) {
                    Out = true;
                    //            do m=1,surface(grsnr)%sides
                    //            write(outputfiledebug,*) 'grsnr,side=',m,surface(grsnr)%vertex
                    //            write(outputfiledebug,*) 'point outside=',surface(sbsnr)%vertex(n)
                    //            enddo
                    //            EXIT
                }
                //          X1 = Surface(GRSNR)%Vertex(Surface(GRSNR)%Sides)%X-Surface(SBSNR)%Vertex(N)%X !XV(NV(GRSNR),GRSNR)-XV(N,SBSNR)
                //          Y1 = Surface(GRSNR)%Vertex(Surface(GRSNR)%Sides)%Y-Surface(SBSNR)%Vertex(N)%Y !YV(NV(GRSNR),GRSNR)-YV(N,SBSNR)
                //          DO M=1,Surface(GRSNR)%Sides !NV(GRSNR)
                //            X2 = X1
                //            Y2 = Y1
                //            X1 = Surface(GRSNR)%Vertex(M)%X-Surface(SBSNR)%Vertex(N)%X !XV(M,GRSNR)-XV(N,SBSNR)
                //            Y1 = Surface(GRSNR)%Vertex(M)%Y-Surface(SBSNR)%Vertex(N)%Y !YV(M,GRSNR)-YV(N,SBSNR)
                //            SZ = X1*Y2-X2*Y1
                //            IF(SZ*BZ.LT.-1.0d-6) THEN
                //              OUT=.TRUE.
                //              write(outputfiledebug,*) 'sz*bz=',sz*bz
                //              write(outputfiledebug,*) 'grsnr=',surface(grsnr)%vertex(m)
                //              write(outputfiledebug,*) 'sbsnr=',surface(sbsnr)%vertex(n)
                //            ENDIF
                //          ENDDO
                //          IF (OUT) EXIT
            }
        }
        //    CALL ShowWarningError(state, 'Base surface does not surround subsurface (CHKSBS), Overlap Status='//  &
        //                           TRIM(cOverLapStatus(OverlapStatus)))
        //    CALL ShowContinueError(state, 'Surface "'//TRIM(Surface(GRSNR)%Name)//'" '//TRIM(MSG(OverlapStatus))//  &
        //                     ' SubSurface "'//TRIM(Surface(SBSNR)%Name)//'"')
        //    IF (FirstSurroundError) THEN
        //      CALL ShowWarningError(state, 'Base Surface does not surround subsurface errors occurring...'//  &
        //                     'Check that the SurfaceGeometry object is expressing the proper starting corner and '//  &
        //                     'direction [CounterClockwise/Clockwise]')
        //      FirstSurroundError=.FALSE.
        //    ENDIF
        if (Out) {
            state.dataSolarShading->TrackBaseSubSurround.redimension(++state.dataSolarShading->NumBaseSubSurround);
            state.dataSolarShading->TrackBaseSubSurround(state.dataSolarShading->NumBaseSubSurround).SurfIndex1 = GRSNR;
            state.dataSolarShading->TrackBaseSubSurround(state.dataSolarShading->NumBaseSubSurround).SurfIndex2 = SBSNR;
            state.dataSolarShading->TrackBaseSubSurround(state.dataSolarShading->NumBaseSubSurround).MiscIndex =
                state.dataSolarShading->OverlapStatus;
            //    CALL ShowRecurringWarningErrorAtEnd(state, 'Base surface does not surround subsurface (CHKSBS), Overlap Status='//  &
            //                       TRIM(cOverLapStatus(OverlapStatus)), &
            //                       TrackBaseSubSurround(GRSNR)%ErrIndex1)
            //    CALL ShowRecurringContinueErrorAtEnd(state, 'Surface "'//TRIM(Surface(GRSNR)%Name)//'" '//TRIM(MSG(OverlapStatus))//  &
            //                       ' SubSurface "'//TRIM(Surface(SBSNR)%Name)//'"',  &
            //                      TrackBaseSubSurround(SBSNR)%ErrIndex2)
            if (state.dataSolarShading->shd_stream) {
                *state.dataSolarShading->shd_stream << "==== Base does not Surround subsurface details ====\n";
                *state.dataSolarShading->shd_stream << "Surface=" << s_surf->Surface(GRSNR).Name << ' '
                                                    << state.dataSolarShading->cOverLapStatus(state.dataSolarShading->OverlapStatus) << '\n';
                *state.dataSolarShading->shd_stream << "Surface#=" << std::setw(5) << GRSNR << " NSides=" << std::setw(5)
                                                    << s_surf->Surface(GRSNR).Sides << '\n';
                *state.dataSolarShading->shd_stream << std::fixed << std::setprecision(2);
                for (N = 1; N <= s_surf->Surface(GRSNR).Sides; ++N) {
                    Vector const &v(s_surf->Surface(GRSNR).Vertex(N));
                    *state.dataSolarShading->shd_stream << "Vertex " << std::setw(5) << N << "=(" << std::setw(15) << v.x << ',' << std::setw(15)
                                                        << v.y << ',' << std::setw(15) << v.z << ")\n";
                }
                *state.dataSolarShading->shd_stream << "SubSurface=" << s_surf->Surface(SBSNR).Name << '\n';
                *state.dataSolarShading->shd_stream << "Surface#=" << std::setw(5) << SBSNR << " NSides=" << std::setw(5)
                                                    << s_surf->Surface(SBSNR).Sides << '\n';
                for (N = 1; N <= s_surf->Surface(SBSNR).Sides; ++N) {
                    Vector const &v(s_surf->Surface(SBSNR).Vertex(N));
                    *state.dataSolarShading->shd_stream << "Vertex " << std::setw(5) << N << "=(" << std::setw(15) << v.x << ',' << std::setw(15)
                                                        << v.y << ',' << std::setw(15) << v.z << ")\n";
                }
                *state.dataSolarShading->shd_stream << "================================\n";
            }
        }
    }
}

bool polygon_contains_point(int const nsides,            // number of sides (vertices)
                            Array1D<Vector> &polygon_3d, // points of polygon
                            Vector const &point_3d,      // point to be tested
                            bool const ignorex,
                            bool const ignorey,
                            bool const ignorez)
{

    // Function information:
    //       Author         Linda Lawrie
    //       Date written   October 2005
    //       Modified       na
    //       Re-engineered  na

    // Purpose of this function:
    // Determine if a point is inside a simple 2d polygon.  For a simple polygon (one whose
    // boundary never crosses itself).  The polygon does not need to be convex.

    // References:
    // M Shimrat, Position of Point Relative to Polygon, ACM Algorithm 112,
    // Communications of the ACM, Volume 5, Number 8, page 434, August 1962.

    // Use statements:
    // Using/Aliasing
    using namespace DataVectorTypes;

    // return value, true=inside, false = not inside

    EP_SIZE_CHECK(polygon_3d, nsides);

    int ip1;

    // Object Data
    Array1D<Vector_2d> polygon(nsides);
    Vector_2d point;

    bool inside = false;
    if (ignorex) {
        for (int i = 1; i <= nsides; ++i) {
            polygon(i).x = polygon_3d(i).y;
            polygon(i).y = polygon_3d(i).z;
        }
        point.x = point_3d.y;
        point.y = point_3d.z;
    } else if (ignorey) {
        for (int i = 1; i <= nsides; ++i) {
            polygon(i).x = polygon_3d(i).x;
            polygon(i).y = polygon_3d(i).z;
        }
        point.x = point_3d.x;
        point.y = point_3d.z;
    } else if (ignorez) {
        for (int i = 1; i <= nsides; ++i) {
            polygon(i).x = polygon_3d(i).x;
            polygon(i).y = polygon_3d(i).y;
        }
        point.x = point_3d.x;
        point.y = point_3d.y;
    } else { // Illegal
        assert(false);
        point.x = point.y = 0.0; // Elim possibly used uninitialized warnings
    }

    for (int i = 1; i <= nsides; ++i) {

        if (i < nsides) {
            ip1 = i + 1;
        } else {
            ip1 = 1;
        }

        if ((polygon(i).y < point.y && point.y <= polygon(ip1).y) || (point.y <= polygon(i).y && polygon(ip1).y < point.y)) {
            if ((point.x - polygon(i).x) - (point.y - polygon(i).y) * (polygon(ip1).x - polygon(i).x) / (polygon(ip1).y - polygon(i).y) < 0) {
                inside = !inside;
            }
        }
    }

    return inside;
}

void ComputeIntSolarAbsorpFactors(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy Code
    //       MODIFIED       B. Griffith, Oct 2010, deal with no floor case
    //                      L. Lawrie, Mar 2012, relax >154 tilt even further (>120 considered non-wall by ASHRAE)
    //       RE-ENGINEERED  Lawrie, Oct 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This routine computes the fractions of diffusely transmitted
    // solar energy absorbed by each zone surface.

    // METHODOLOGY EMPLOYED:
    // It is assumed that all transmitted solar energy is incident
    // on the floors of the zone (or enclosure).  The fraction directly absorbed in
    // the floor is given by 'ISABSF'.  It is proportional to the
    // area * solar absorptance.  The remaining solar energy is then
    // distributed uniformly around the room according to
    // area*absorptance product

    // REFERENCES:
    // BLAST/IBLAST code, original author George Walton

    using namespace DataWindowEquivalentLayer;

    Real64 AreaSum;       // Intermediate calculation value
    int Lay;              // Window glass layer number
    Real64 AbsDiffTotWin; // Sum of a window's glass layer solar absorptances
    Real64 TestFractSum;
    Real64 HorizAreaSum;

    auto &s_surf = state.dataSurface;

    for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclosureNum) {
        auto &thisEnclosure = state.dataViewFactor->EnclSolInfo(enclosureNum);

        AreaSum = 0.0;
        TestFractSum = 0.0;
        for (int const SurfNum : thisEnclosure.SurfacePtr) {
            if (state.dataHeatBal->Zone(s_surf->Surface(SurfNum).Zone).OfType == StandardZone && s_surf->Surface(SurfNum).CosTilt < -0.5) {
                AreaSum += s_surf->Surface(SurfNum).Area;
            }
        }

        HorizAreaSum = AreaSum;

        if ((thisEnclosure.FloorArea <= 0.0) && (HorizAreaSum > 0.0)) {
            // fill floor area even though surfs not called "Floor", they are roughly horizontal and face upwards.
            thisEnclosure.FloorArea = HorizAreaSum;
            ShowWarningError(state, "ComputeIntSolarAbsorpFactors: Solar distribution model is set to place solar gains on the zone floor,");
            ShowContinueError(state, format("...Enclosure=\"{}\" has no floor, but has approximate horizontal surfaces.", thisEnclosure.Name));
            ShowContinueError(state, format("...these Tilt > 120 degrees, (area=[{:.2R}] m2) will be used.", HorizAreaSum));
        }

        // Compute ISABSF

        for (int const SurfNum : thisEnclosure.SurfacePtr) {

            // only horizontal surfaces. !      !CR 8229, relaxed from -0.99 to -0.5  (Tilt > 154)
            // only horizontal surfaces. !      !CR8769 use ASHRAE std of >120, -0.9 to -0.5  (Tilt > 120)
            if ((state.dataHeatBal->Zone(s_surf->Surface(SurfNum).Zone).OfType != StandardZone || s_surf->Surface(SurfNum).CosTilt < -0.5) &&
                (state.dataHeatBal->Zone(s_surf->Surface(SurfNum).Zone).OfType == StandardZone || s_surf->Surface(SurfNum).ExtBoundCond > 0)) {

                int const ConstrNum = s_surf->SurfActiveConstruction(SurfNum);
                auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);
                // last minute V3.1
                if (thisConstruct.TransDiff <= 0.0) { // Opaque surface
                    if (AreaSum > 0.0) {
                        state.dataSolarShading->SurfIntAbsFac(SurfNum) = s_surf->Surface(SurfNum).Area * thisConstruct.InsideAbsorpSolar / AreaSum;
                    }
                } else { // Window (floor windows are assumed to have no shading device and no divider,
                    // and assumed to be non-switchable)
                    AbsDiffTotWin = 0.0;
                    if (!state.dataConstruction->Construct(s_surf->Surface(SurfNum).Construction).WindowTypeEQL) {
                        for (Lay = 1; Lay <= thisConstruct.TotGlassLayers; ++Lay) {
                            AbsDiffTotWin += thisConstruct.AbsDiffBack(Lay);
                        }
                    } else {
                        for (Lay = 1; Lay <= state.dataWindowEquivLayer->CFS(thisConstruct.EQLConsPtr).NL; ++Lay) {
                            AbsDiffTotWin += thisConstruct.AbsDiffBackEQL(Lay);
                        }
                    }
                    if (AreaSum > 0.0) {
                        state.dataSolarShading->SurfIntAbsFac(SurfNum) = s_surf->Surface(SurfNum).Area * AbsDiffTotWin / AreaSum;
                    }
                }
            }
            // CR 8229  test ISABSF for problems
            TestFractSum += state.dataSolarShading->SurfIntAbsFac(SurfNum);
        }

        if (TestFractSum <= 0.0) {
            if (thisEnclosure.ExtWindowArea > 0.0) { // we have a problem, the sun has no floor to go to
                if (thisEnclosure.FloorArea <= 0.0) {
                    ShowSevereError(state, "ComputeIntSolarAbsorpFactors: Solar distribution model is set to place solar gains on the zone floor,");
                    ShowContinueError(state, format("but Zone or Enclosure =\"{}\" does not appear to have any floor surfaces.", thisEnclosure.Name));
                    ShowContinueError(state, "Solar gains will be spread evenly on all surfaces in the zone, and the simulation continues...");
                } else { // Floor Area > 0 but still can't absorb
                    ShowSevereError(state, "ComputeIntSolarAbsorpFactors: Solar distribution model is set to place solar gains on the zone floor,");
                    ShowContinueError(state, format("but Zone or Enclosure =\"{}\" floor cannot absorb any solar gains. ", thisEnclosure.Name));
                    ShowContinueError(state, "Check the solar absorptance of the inside layer of the floor surface construction/material.");
                    ShowContinueError(state, "Solar gains will be spread evenly on all surfaces in the zone, and the simulation continues...");
                }

                // try again but use an even spread across all the surfaces in the zone, regardless of horizontal
                //  so as to not lose solar energy
                AreaSum = 0.0;
                for (int SurfNum : thisEnclosure.SurfacePtr) {
                    AreaSum += s_surf->Surface(SurfNum).Area;
                }

                for (int const SurfNum : thisEnclosure.SurfacePtr) {
                    int const ConstrNum = s_surf->SurfActiveConstruction(SurfNum);
                    auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);
                    if (thisConstruct.TransDiff <= 0.0) { // Opaque surface
                        if (AreaSum > 0.0) {
                            state.dataSolarShading->SurfIntAbsFac(SurfNum) =
                                s_surf->Surface(SurfNum).Area * thisConstruct.InsideAbsorpSolar / AreaSum;
                        }
                    } else { // Window (floor windows are assumed to have no shading device and no divider,
                        // and assumed to be non-switchable)
                        AbsDiffTotWin = 0.0;
                        if (!state.dataConstruction->Construct(s_surf->Surface(SurfNum).Construction).WindowTypeEQL) {
                            for (Lay = 1; Lay <= thisConstruct.TotGlassLayers; ++Lay) {
                                AbsDiffTotWin += thisConstruct.AbsDiffBack(Lay);
                            }
                        } else {
                            for (Lay = 1; Lay <= state.dataWindowEquivLayer->CFS(thisConstruct.EQLConsPtr).NL; ++Lay) {
                                AbsDiffTotWin += thisConstruct.AbsDiffBackEQL(Lay);
                            }
                        }

                        if (AreaSum > 0.0) {
                            state.dataSolarShading->SurfIntAbsFac(SurfNum) = s_surf->Surface(SurfNum).Area * AbsDiffTotWin / AreaSum;
                        }
                    }
                }
            }
        }

    } // enclosure loop
}

void CLIP(EnergyPlusData &state, int const NVT, Array1D<Real64> &XVT, Array1D<Real64> &YVT, Array1D<Real64> &ZVT)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy Code
    //       DATE WRITTEN
    //       MODIFIED       na
    //       RE-ENGINEERED  Lawrie, Oct 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine 'clips' the shadow casting surface polygon so that
    // none of it lies below the plane of the receiving surface polygon.  This
    // prevents the casting of 'false' shadows.

    // REFERENCES:
    // BLAST/IBLAST code, original author George Walton

    int NABOVE(0);    // Number of vertices of shadow casting surface. above the plane of receiving surface
    int NEXT(0);      // First vertex above plane of receiving surface
    int NON(0);       // Number of vertices of shadow casting surface. on plane of receiving surface
    Real64 XIN(0.0);  // X of entry point of shadow casting surface. into plane of receiving surface
    Real64 XOUT(0.0); // X of exit point of shadow casting surface. from plane of receiving surface
    Real64 YIN(0.0);  // Y of entry point of shadow casting surface. into plane of receiving surface
    Real64 YOUT(0.0); // Y of exit point of shadow casting surface. from plane of receiving surface
    //  INTEGER NVS      ! Number of vertices of the shadow/clipped surface

    // Determine if the shadow casting surface. is above, below, or intersects with the plane of the receiving surface

    state.dataSolarShading->NumVertInShadowOrClippedSurface = state.dataSolarShading->NVS;
    for (int N = 1; N <= NVT; ++N) {
        Real64 const ZVT_N(ZVT(N));
        if (ZVT_N > 0.0) {
            ++NABOVE;
        } else if (ZVT_N == 0.0) {
            ++NON;
        }
    }

    if (NABOVE + NON == NVT) { // Rename the unclipped shadow casting surface.

        state.dataSolarShading->NVS = NVT;
        state.dataSolarShading->NumVertInShadowOrClippedSurface = NVT;
        for (int N = 1; N <= NVT; ++N) {
            state.dataSolarShading->XVC(N) = XVT(N);
            state.dataSolarShading->YVC(N) = YVT(N);
            state.dataSolarShading->ZVC(N) = ZVT(N);
        }

    } else if (NABOVE == 0) { // Totally submerged shadow casting surface.

        state.dataSolarShading->NVS = 0;
        state.dataSolarShading->NumVertInShadowOrClippedSurface = 0;

    } else { // Remove (clip) that portion of the shadow casting surface. which is below the receiving surface

        state.dataSolarShading->NVS = NABOVE + 2;
        state.dataSolarShading->NumVertInShadowOrClippedSurface = NABOVE + 2;
        Real64 ZVT_N, ZVT_P(ZVT(1));
        XVT(NVT + 1) = XVT(1);
        YVT(NVT + 1) = YVT(1);
        ZVT(NVT + 1) = ZVT_P;
        for (int N = 1, P = 2; N <= NVT; ++N, ++P) {
            ZVT_N = ZVT_P;
            ZVT_P = ZVT(P);
            if (ZVT_N >= 0.0 && ZVT_P < 0.0) { // Line enters plane of receiving surface
                Real64 const ZVT_fac(1.0 / (ZVT_P - ZVT_N));
                XIN = (ZVT_P * XVT(N) - ZVT_N * XVT(P)) * ZVT_fac;
                YIN = (ZVT_P * YVT(N) - ZVT_N * YVT(P)) * ZVT_fac;
            } else if (ZVT_N <= 0.0 && ZVT_P > 0.0) { // Line exits plane of receiving surface
                NEXT = N + 1;
                Real64 const ZVT_fac(1.0 / (ZVT_P - ZVT_N));
                XOUT = (ZVT_P * XVT(N) - ZVT_N * XVT(P)) * ZVT_fac;
                YOUT = (ZVT_P * YVT(N) - ZVT_N * YVT(P)) * ZVT_fac;
            }
        }

        // Renumber the vertices of the clipped shadow casting surface. so they are still counter-clockwise sequential.

        state.dataSolarShading->XVC(1) = XOUT; //? Verify that the IN and OUT values were ever set?
        state.dataSolarShading->YVC(1) = YOUT;
        state.dataSolarShading->ZVC(1) = 0.0;
        state.dataSolarShading->XVC(state.dataSolarShading->NVS) = XIN;
        state.dataSolarShading->YVC(state.dataSolarShading->NVS) = YIN;
        state.dataSolarShading->ZVC(state.dataSolarShading->NVS) = 0.0;
        for (int N = 1; N <= NABOVE; ++N) {
            if (NEXT > NVT) {
                NEXT = 1;
            }
            state.dataSolarShading->XVC(N + 1) = XVT(NEXT);
            state.dataSolarShading->YVC(N + 1) = YVT(NEXT);
            state.dataSolarShading->ZVC(N + 1) = ZVT(NEXT);
            ++NEXT;
        }
    }
}

void CTRANS(EnergyPlusData &state,
            int const NS,         // Surface number whose vertex coordinates are being transformed
            int const NGRS,       // Base surface number for surface NS
            int &NVT,             // Number of vertices for surface NS
            Array1D<Real64> &XVT, // XYZ coordinates of vertices of NS in plane of NGRS
            Array1D<Real64> &YVT,
            Array1D<Real64> &ZVT)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy Code
    //       DATE WRITTEN
    //       MODIFIED       na
    //       RE-ENGINEERED  Lawrie, Oct 2000

    // PURPOSE OF THIS SUBROUTINE:
    // Transforms the general coordinates of the vertices
    // of surface NS to coordinates in the plane of the receiving surface NGRS.
    // See subroutine 'CalcCoordinateTransformation' SurfaceGeometry Module.

    // REFERENCES:
    // BLAST/IBLAST code, original author George Walton
    // NECAP subroutine 'SHADOW'

    Real64 Xdif; // Intermediate Result
    Real64 Ydif; // Intermediate Result
    Real64 Zdif; // Intermediate Result

    auto &s_surf = state.dataSurface;

    // Tuned
    auto const &surface = s_surf->Surface(NS);
    auto const &base_surface = s_surf->Surface(NGRS);
    auto const &base_lcsx = base_surface.lcsx;
    auto const &base_lcsy = base_surface.lcsy;
    auto const &base_lcsz = base_surface.lcsz;
    Real64 const base_X0 = s_surf->X0(NGRS);
    Real64 const base_Y0 = s_surf->Y0(NGRS);
    Real64 const base_Z0 = s_surf->Z0(NGRS);

    NVT = surface.Sides;

    // Perform transformation
    for (int N = 1; N <= NVT; ++N) {
        auto const &vertex = surface.Vertex(N);

        Xdif = vertex.x - base_X0;
        Ydif = vertex.y - base_Y0;
        Zdif = vertex.z - base_Z0;

        if (std::abs(Xdif) <= 1.E-15) {
            Xdif = 0.0;
        }
        if (std::abs(Ydif) <= 1.E-15) {
            Ydif = 0.0;
        }
        if (std::abs(Zdif) <= 1.E-15) {
            Zdif = 0.0;
        }

        XVT(N) = base_lcsx.x * Xdif + base_lcsx.y * Ydif + base_lcsx.z * Zdif;
        YVT(N) = base_lcsy.x * Xdif + base_lcsy.y * Ydif + base_lcsy.z * Zdif;
        ZVT(N) = base_lcsz.x * Xdif + base_lcsz.y * Ydif + base_lcsz.z * Zdif;
    }
}

void HTRANS(EnergyPlusData &state,
            int const I,          // Mode selector: 0 - Compute H.C. of sides
            int const NS,         // Figure Number
            int const NumVertices // Number of vertices
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy Code
    //       DATE WRITTEN
    //       MODIFIED       na
    //       RE-ENGINEERED  Lawrie, Oct 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine sets up the homogeneous coordinates.
    // This routine converts the cartesian coordinates of a surface
    // or shadow polygon to homogeneous coordinates.  It also
    // computes the area of the polygon.

    // METHODOLOGY EMPLOYED:
    // Note: Original legacy code used integer arithmetic (tests in subroutines
    // INCLOS and INTCPT are sensitive to round-off error).  However, porting to Fortran 77
    // (BLAST/IBLAST) required some variables to become REAL(r64) instead.

    // Notes on homogeneous coordinates:
    // A point (X,Y) is represented by a 3-element vector
    // (W*X,W*Y,W), where W may be any REAL(r64) number except 0.  a line
    // is also represented by a 3-element vector (A,B,C).  The
    // directed line (A,B,C) from point (W*X1,W*Y1,W) to point
    // (V*X2,V*Y2,V) is given by (A,B,C) = (W*X1,W*Y1,W) cross
    // (V*X2,V*Y2,V).  The sequence of the cross product is a
    // convention to determine sign.  The condition that a point lie
    // on a line is that (A,B,C) dot (W*X,W*Y,W) = 0.  'Normalize'
    // the representation of a point by setting W to 1.  Then if
    // (A,B,C) dot (X,Y,1) > 0.0, The point is to the left of the
    // line, and if it is less than zero, the point is to the right
    // of the line.  The intercept of two lines is given by
    // (W*X,W*Y,W) = (A1,B1,C1) cross (A2,B2,C3).

    // REFERENCES:
    // BLAST/IBLAST code, original author George Walton
    // W. M. Newman & R. F. Sproull, 'Principles of Interactive Computer Graphics', Appendix II, McGraw-Hill, 1973.
    // 'CRC Math Tables', 22 ED, 'Analytic Geometry', P.369

    // Using/Aliasing

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:
    //                1 - Compute H.C. of vertices & sides

    if (NS > 2 * state.dataSolarShading->MaxHCS) {
        ShowFatalError(state, format("Solar Shading: HTrans: Too many Figures (>{})", state.dataSolarShading->MaxHCS));
    }

    state.dataSolarShading->HCNV(NS) = NumVertices;

    // Tuned Linear indexing

    assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCY));
    assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCA));
    assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCB));
    assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCC));
    int const l1(state.dataSolarShading->HCX.index(NS, 1));
    if (I != 0) { // Transform vertices of figure ns.

        // See comment at top of module regarding HCMULT
        int l(l1);
        for (int N = 1; N <= NumVertices; ++N, ++l) { // [ l ] == ( NS, N )
            state.dataSolarShading->HCX[l] = nint64(state.dataSolarShading->XVS(N) * HCMULT);
            state.dataSolarShading->HCY[l] = nint64(state.dataSolarShading->YVS(N) * HCMULT);
        }
    }

    // Establish extra point for finding lines between points.

    int l = state.dataSolarShading->HCX.index(NS, NumVertices + 1);
    Int64 HCX_m = state.dataSolarShading->HCX[l] = state.dataSolarShading->HCX[l1]; // [ l ] == ( NS, NumVertices + 1 ), [ l1 ] == ( NS, 1 )
    Int64 HCY_m = state.dataSolarShading->HCY[l] = state.dataSolarShading->HCY[l1]; // [ l ] == ( NS, NumVertices + 1 ), [ l1 ] == ( NS, 1 )

    // Determine lines between points.
    l = l1;
    int m = l1 + 1u;
    Int64 HCX_l;
    Int64 HCY_l;
    Real64 SUM = 0.0;                                  // Sum variable
    for (int N = 1; N <= NumVertices; ++N, ++l, ++m) { // [ l ] == ( NS, N ), [ m ] == ( NS, N + 1 )
        HCX_l = HCX_m;
        HCY_l = HCY_m;
        HCX_m = state.dataSolarShading->HCX[m];
        HCY_m = state.dataSolarShading->HCY[m];
        state.dataSolarShading->HCA[l] = HCY_l - HCY_m;
        state.dataSolarShading->HCB[l] = HCX_m - HCX_l;
        SUM += state.dataSolarShading->HCC[l] = (HCY_m * HCX_l) - (HCX_m * HCY_l);
    }

    // Compute area of polygon.
    //  SUM=0.0D0
    //  DO N = 1, NumVertices
    //    SUM = SUM + HCX(N,NS)*HCY(N+1,NS) - HCY(N,NS)*HCX(N+1,NS) ! Since HCX and HCY integerized, value of SUM should be ok
    //  END DO
    state.dataSolarShading->HCAREA(NS) = SUM * sqHCMULT_fac;
    //  HCAREA(NS)=0.5d0*SUM*(kHCMULT)
}

void HTRANS0(EnergyPlusData &state,
             int const NS,         // Figure Number
             int const NumVertices // Number of vertices
)
{
    // Using/Aliasing

    // Locals

    if (NS > 2 * state.dataSolarShading->MaxHCS) {
        ShowFatalError(state, format("Solar Shading: HTrans0: Too many Figures (>{})", state.dataSolarShading->MaxHCS));
    }

    state.dataSolarShading->HCNV(NS) = NumVertices;

    // Tuned Linear indexing

    assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCY));
    assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCA));
    assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCB));
    assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCC));

    int const l1 = state.dataSolarShading->HCX.index(NS, 1);

    int l = state.dataSolarShading->HCX.index(NS, NumVertices + 1);
    Int64 HCX_m = state.dataSolarShading->HCX[l] = state.dataSolarShading->HCX[l1]; // [ l1 ] == ( NS, 1 )
    Int64 HCY_m = state.dataSolarShading->HCY[l] = state.dataSolarShading->HCY[l1]; // [ l1 ] == ( NS, 1 )

    l = l1;
    int m = l1 + 1u;
    Int64 HCX_l;
    Int64 HCY_l;
    Real64 SUM(0.0);
    for (int N = 1; N <= NumVertices; ++N, ++l, ++m) { // [ l ] == ( NS, N ), [ m ] == ( NS, N + 1 )
        HCX_l = HCX_m;
        HCY_l = HCY_m;
        HCX_m = state.dataSolarShading->HCX[m];
        HCY_m = state.dataSolarShading->HCY[m];
        state.dataSolarShading->HCA[l] = HCY_l - HCY_m;
        state.dataSolarShading->HCB[l] = HCX_m - HCX_l;
        SUM += state.dataSolarShading->HCC[l] = (HCY_m * HCX_l) - (HCX_m * HCY_l);
    }

    state.dataSolarShading->HCAREA(NS) = SUM * sqHCMULT_fac;
}

void HTRANS1(EnergyPlusData &state,
             int const NS,         // Figure Number
             int const NumVertices // Number of vertices
)
{
    // Using/Aliasing

    if (NS > 2 * state.dataSolarShading->MaxHCS) {
        ShowFatalError(state, format("Solar Shading: HTrans1: Too many Figures (>{})", state.dataSolarShading->MaxHCS));
    }

    state.dataSolarShading->HCNV(NS) = NumVertices;

    // Tuned Linear indexing

    assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCY));
    assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCA));
    assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCB));
    assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCC));

    int const l1 = state.dataSolarShading->HCX.index(NS, 1);

    // only in HTRANS1
    int l = l1;
    for (int N = 1; N <= NumVertices; ++N, ++l) { // [ l ] == ( NS, N )
        state.dataSolarShading->HCX[l] = nint64(state.dataSolarShading->XVS(N) * HCMULT);
        state.dataSolarShading->HCY[l] = nint64(state.dataSolarShading->YVS(N) * HCMULT);
    }

    l = state.dataSolarShading->HCX.index(NS, NumVertices + 1);
    Int64 HCX_m(state.dataSolarShading->HCX[l] = state.dataSolarShading->HCX[l1]); // [ l1 ] == ( NS, 1 )
    Int64 HCY_m(state.dataSolarShading->HCY[l] = state.dataSolarShading->HCY[l1]);

    l = l1;
    int m = l1 + 1u;
    Int64 HCX_l;
    Int64 HCY_l;
    Real64 SUM = 0.0;
    for (int N = 1; N <= NumVertices; ++N, ++l, ++m) { // [ l ] == ( NS, N ), [ m ] == ( NS, N + 1 )
        HCX_l = HCX_m;
        HCY_l = HCY_m;
        HCX_m = state.dataSolarShading->HCX[m];
        HCY_m = state.dataSolarShading->HCY[m];
        state.dataSolarShading->HCA[l] = HCY_l - HCY_m;
        state.dataSolarShading->HCB[l] = HCX_m - HCX_l;
        SUM += state.dataSolarShading->HCC[l] = (HCY_m * HCX_l) - (HCX_m * HCY_l);
    }

    state.dataSolarShading->HCAREA(NS) = SUM * sqHCMULT_fac;
}

void INCLOS(EnergyPlusData &state,
            int const N1,            // Figure number of figure 1
            int const N1NumVert,     // Number of vertices of figure 1
            int const N2,            // Figure number of figure 2
            int const N2NumVert,     // Number of vertices of figure 2
            int &NumVerticesOverlap, // Number of vertices which overlap
            int &NIN                 // Number of vertices of figure 1 within figure 2
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy Code
    //       DATE WRITTEN
    //       MODIFIED       na
    //       RE-ENGINEERED  Lawrie, Oct 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine determines which vertices of figure N1 lie within figure N2.

    // METHODOLOGY EMPLOYED:
    // For vertex N of figure N1 to lie within figure N2, it must be
    // on or to the right of all sides of figure N2, assuming
    // figure N2 is convex.

    // REFERENCES:
    // BLAST/IBLAST code, original author George Walton

    int K;              // Vertex number of the overlap
    int M;              // Side number of figure N2
    int N;              // Vertex number of figure N1
    bool CycleMainLoop; // Sets when to cycle main loop
    Real64 HFunct;

    NIN = 0;

    for (N = 1; N <= N1NumVert; ++N) {

        CycleMainLoop = false;

        // Eliminate cases where vertex N is to the left of side M.

        for (M = 1; M <= N2NumVert; ++M) {
            HFunct = state.dataSolarShading->HCX(N1, N) * state.dataSolarShading->HCA(N2, M) +
                     state.dataSolarShading->HCY(N1, N) * state.dataSolarShading->HCB(N2, M) + state.dataSolarShading->HCC(N2, M);
            if (HFunct > 0.0) {
                CycleMainLoop = true; // Set to cycle to the next value of N
                break;                // M DO loop
            }
        }

        if (CycleMainLoop) {
            continue;
        }
        ++NIN;

        // Check for duplication of previously determined points.

        if (NumVerticesOverlap != 0) {
            for (K = 1; K <= NumVerticesOverlap; ++K) {
                if ((state.dataSolarShading->XTEMP(K) == state.dataSolarShading->HCX(N1, N)) &&
                    (state.dataSolarShading->YTEMP(K) == state.dataSolarShading->HCY(N1, N))) {
                    CycleMainLoop = true; // Set to cycle to the next value of N
                    break;                // K DO loop
                }
            }
            if (CycleMainLoop) {
                continue;
            }
        }

        // Record enclosed vertices in temporary arrays.

        ++NumVerticesOverlap;
        state.dataSolarShading->XTEMP(NumVerticesOverlap) = state.dataSolarShading->HCX(N1, N);
        state.dataSolarShading->YTEMP(NumVerticesOverlap) = state.dataSolarShading->HCY(N1, N);
    }
}

void INTCPT(EnergyPlusData &state,
            int const NV1, // Number of vertices of figure NS1
            int const NV2, // Number of vertices of figure NS2
            int &NV3,      // Number of vertices of figure NS3
            int const NS1, // Number of the figure being overlapped
            int const NS2  // Number of the figure doing overlapping
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy Code
    //       DATE WRITTEN
    //       MODIFIED       na
    //       RE-ENGINEERED  Lawrie, Oct 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine determines all intercepts between the sides of figure NS1
    // and the sides of figure NS2.

    // METHODOLOGY EMPLOYED:
    // The requirements for intersection are that the end points of
    // line N lie on both sides of line M and vice versa.  Also
    // eliminate cases where the end point of one line lies exactly
    // on the other to reduce duplication with the enclosed points.

    // REFERENCES:
    // BLAST/IBLAST code, original author George Walton

    Real64 W;        // Normalization factor
    Real64 XUntrunc; // Untruncated X coordinate
    Real64 YUntrunc; // Untruncated Y coordinate
    Int64 I1;        // Intermediate result for testing intersection
    Int64 I2;        // Intermediate result for testing intersection
    int K;
    int KK;
    int M; // Side number of figure NS2
    int N; // Side number of figure NS1

    for (N = 1; N <= NV1; ++N) {
        for (M = 1; M <= NV2; ++M) {

            // Eliminate cases where sides N and M do not intersect.

            I1 = state.dataSolarShading->HCA(NS1, N) * state.dataSolarShading->HCX(NS2, M) +
                 state.dataSolarShading->HCB(NS1, N) * state.dataSolarShading->HCY(NS2, M) + state.dataSolarShading->HCC(NS1, N);
            I2 = state.dataSolarShading->HCA(NS1, N) * state.dataSolarShading->HCX(NS2, M + 1) +
                 state.dataSolarShading->HCB(NS1, N) * state.dataSolarShading->HCY(NS2, M + 1) + state.dataSolarShading->HCC(NS1, N);
            if (I1 >= 0 && I2 >= 0) {
                continue;
            }
            if (I1 <= 0 && I2 <= 0) {
                continue;
            }

            I1 = state.dataSolarShading->HCA(NS2, M) * state.dataSolarShading->HCX(NS1, N) +
                 state.dataSolarShading->HCB(NS2, M) * state.dataSolarShading->HCY(NS1, N) + state.dataSolarShading->HCC(NS2, M);
            I2 = state.dataSolarShading->HCA(NS2, M) * state.dataSolarShading->HCX(NS1, N + 1) +
                 state.dataSolarShading->HCB(NS2, M) * state.dataSolarShading->HCY(NS1, N + 1) + state.dataSolarShading->HCC(NS2, M);
            if (I1 >= 0 && I2 >= 0) {
                continue;
            }
            if (I1 <= 0 && I2 <= 0) {
                continue;
            }

            // Determine the point of intersection and record in the temporary array.

            KK = NV3;
            ++NV3;
            W = state.dataSolarShading->HCB(NS2, M) * state.dataSolarShading->HCA(NS1, N) -
                state.dataSolarShading->HCA(NS2, M) * state.dataSolarShading->HCB(NS1, N);
            XUntrunc = (state.dataSolarShading->HCC(NS2, M) * state.dataSolarShading->HCB(NS1, N) -
                        state.dataSolarShading->HCB(NS2, M) * state.dataSolarShading->HCC(NS1, N)) /
                       W;
            YUntrunc = (state.dataSolarShading->HCA(NS2, M) * state.dataSolarShading->HCC(NS1, N) -
                        state.dataSolarShading->HCC(NS2, M) * state.dataSolarShading->HCA(NS1, N)) /
                       W;
            if (NV3 > isize(state.dataSolarShading->XTEMP)) {
                state.dataSolarShading->XTEMP.redimension(isize(state.dataSolarShading->XTEMP) + 10, 0.0);
                state.dataSolarShading->YTEMP.redimension(isize(state.dataSolarShading->YTEMP) + 10, 0.0);
            }
            state.dataSolarShading->XTEMP(NV3) = nint64(XUntrunc);
            state.dataSolarShading->YTEMP(NV3) = nint64(YUntrunc);

            // Eliminate near-duplicate points.

            if (KK != 0) {
                Real64 const x(state.dataSolarShading->XTEMP(NV3));
                Real64 const y(state.dataSolarShading->YTEMP(NV3));
                for (K = 1; K <= KK; ++K) {
                    if (std::abs(x - state.dataSolarShading->XTEMP(K)) > 2.0) {
                        continue;
                    }
                    if (std::abs(y - state.dataSolarShading->YTEMP(K)) > 2.0) {
                        continue;
                    }
                    NV3 = KK;
                    break; // K DO loop
                }
            }
        }
    }
}

inline bool neq(Real64 a, Real64 b)
{
    return std::abs(a - b) > 2.0;
}

inline bool d_eq(Real64 a, Real64 b)
{
    return std::abs(a - b) < 2.0;
}

void CLIPLINE(Real64 &x0, Real64 &x1, Real64 &y0, Real64 &y1, Real64 maxX, Real64 minX, Real64 maxY, Real64 minY, bool &visible)
{

    // Line segment clipping
    // Reference:
    // Liang, Y.D., Barsky, B.A., Slater, M.
    // 2D line and polygon clipping based on space subdivision.
    // The Visual Computer 10, 407–422 (1994).

    // Tweaked via microbenchmarking to improve efficiency

    bool rev = false;
    if (x0 > x1) { // reverse for efficiency
        std::swap(x0, x1);
        std::swap(y0, y1);
        rev = true;
    }

    if (x0 > maxX || x1 < minX) {
        // Both points are outside the clip window, so they can't cross it
        return;
    }

    // defining variables
    Real64 const dx = x1 - x0; // >= 0
    Real64 const dy = y1 - y0;

    Real64 const q1 = x0 - minX;
    Real64 const q2 = maxX - x0;
    Real64 const q3 = y0 - minY;
    Real64 const q4 = maxY - y0;

    Real64 u1 = 0;
    Real64 u2 = 1;

    if ((dx == 0 && (q1 < 0 || q2 < 0)) || (dy == 0 && (q3 < 0 || q4 < 0))) {
        // Line is parallel to clipping window
        return;
    }
    if (dx != 0) {
        Real64 const r1 = q1 / -dx;
        if (r1 > u1) {
            u1 = r1;
        }
        Real64 const r2 = q2 / dx;
        if (r2 < u2) {
            u2 = r2;
        }
    }
    if (dy != 0) {
        Real64 const r3 = q3 / -dy;
        Real64 const r4 = q4 / dy;
        if (dy > 0) {
            if (r3 > u1) {
                u1 = r3;
            }
            if (r4 < u2) {
                u2 = r4;
            }
        } else {
            if (r4 > u1) {
                u1 = r4;
            }
            if (r3 < u2) {
                u2 = r3;
            }
        }
    }

    if (u1 > u2) { // reject
        // Line is outside the clipping window
        return;
    }

    visible = true;

    Real64 const xn0 = x0 + dx * u1;
    Real64 const yn0 = y0 + dy * u1;

    Real64 const xn1 = x0 + dx * u2;
    Real64 const yn1 = y0 + dy * u2;

    if (rev) {
        x0 = xn1;
        y0 = yn1;
        x1 = xn0;
        y1 = yn0;
    } else {
        x0 = xn0;
        y0 = yn0;
        x1 = xn1;
        y1 = yn1;
    }
}

void CLIPRECT(EnergyPlusData &state, int const NS2, int const NV1, int &NV3)
{
    // Polygon clipping by line segment clipping for rectangles
    // Reference:
    // Slater, M., Barsky, B.A.
    // 2D line and polygon clipping based on space subdivision.
    // The Visual Computer 10, 407–422 (1994).
    bool INTFLAG = false;
    int l = state.dataSolarShading->HCA.index(NS2, 1);
    Real64 maxX, minX, maxY, minY;
    if (state.dataSolarShading->HCX[l] > state.dataSolarShading->HCX[l + 2]) {
        maxX = state.dataSolarShading->HCX[l];
        minX = state.dataSolarShading->HCX[l + 2];
    } else {
        maxX = state.dataSolarShading->HCX[l + 2];
        minX = state.dataSolarShading->HCX[l];
    }
    if (state.dataSolarShading->HCY[l] > state.dataSolarShading->HCY[l + 2]) {
        maxY = state.dataSolarShading->HCY[l];
        minY = state.dataSolarShading->HCY[l + 2];
    } else {
        maxY = state.dataSolarShading->HCY[l + 2];
        minY = state.dataSolarShading->HCY[l];
    }

    Real64 arrx[20]; // Temp array for output X
    Real64 arry[20]; // Temp array for output Y
    int arrc = 0;    // Number of items in output

    for (int j = 0; j < NV1; ++j) {
        Real64 x_1 = state.dataSolarShading->XTEMP[j];
        Real64 y_1 = state.dataSolarShading->YTEMP[j];
        Real64 x_2 = state.dataSolarShading->XTEMP[(j + 1) % NV1];
        Real64 y_2 = state.dataSolarShading->YTEMP[(j + 1) % NV1];
        Real64 x1 = x_1, x2 = x_2, y1 = y_1, y2 = y_2;

        bool visible = false;
        CLIPLINE(x_1, x_2, y_1, y_2, maxX, minX, maxY, minY, visible);
        if (visible) {
            if ((x_1 != x1 || y_1 != y1) || (x_2 != x2 || y_2 != y2)) {
                INTFLAG = true;
            }
            // if line on edge, or inside, add both points
            if (arrc == 0 || ((neq(arrx[arrc - 1], x_1) || neq(arry[arrc - 1], y_1)) && (neq(arrx[0], x_1) || neq(arry[0], y_1)))) {
                arrx[arrc] = x_1;
                arry[arrc] = y_1;
                arrc += 1;
                if ((neq(x_1, x_2) || neq(y_1, y_2)) && (neq(arrx[0], x_2) || neq(arry[0], y_2))) {
                    arrx[arrc] = x_2;
                    arry[arrc] = y_2;
                    arrc += 1;
                }
            } else if ((neq(arrx[arrc - 1], x_2) || neq(arry[arrc - 1], y_2)) && (neq(arrx[0], x_2) || neq(arry[0], y_2))) {
                arrx[arrc] = x_2;
                arry[arrc] = y_2;
                arrc += 1;
            }
        }
    }
    NV3 = arrc;

    // Re-populate XTEMP/YTEMP
    if (NV3 > 1) {
        int LastEdgeIndex = -1, incr = 0;
        double cornerXs[4] = {minX, minX, maxX, maxX};
        double cornerYs[4] = {minY, maxY, maxY, minY};
        Real64 edges[4] = {minX, maxY, maxX, minY};
        Real64 LastEdgeX, LastEdgeY;
        for (int i = 0; i <= arrc; i++) {
            int k = i % arrc;

            Real64 currX = arrx[k], currY = arry[k];

            int edgeCount = 0, EdgeIndex = -1;
            for (int m = 0; m < 4; m++) {
                if (m % 2 == 0 && d_eq(currX, edges[m])) { // MinX or MaxX
                    edgeCount++;
                    EdgeIndex = m;
                } else if (m % 2 == 1 && d_eq(currY, edges[m])) {
                    edgeCount++;
                    EdgeIndex = m;
                }
            }
            if (edgeCount == 0) { // On inside
                if (i != arrc) {
                    state.dataSolarShading->XTEMP[incr] = currX;
                    state.dataSolarShading->YTEMP[incr] = currY;
                    incr++;
                }
                continue;
            } else if (edgeCount > 1) { // On corner
                if (d_eq(currX, minX)) {
                    if (d_eq(currY, minY)) {
                        EdgeIndex = 3;
                    } else {
                        EdgeIndex = 0;
                    }
                } else {
                    if (d_eq(currY, maxY)) {
                        EdgeIndex = 1;
                    } else {
                        EdgeIndex = 2;
                    }
                }
            }
            if ((LastEdgeIndex > -1 && EdgeIndex > -1) && LastEdgeIndex != EdgeIndex) {
                int jumpCount = 0;
                if ((EdgeIndex == 0 && LastEdgeIndex == 3) || (EdgeIndex - LastEdgeIndex == 1)) {
                    jumpCount = 1;
                } else if (EdgeIndex % 2 == LastEdgeIndex % 2) {
                    // Clockwise double jump
                    jumpCount = 2;
                } else if ((EdgeIndex == 3 && LastEdgeIndex == 0) || (LastEdgeIndex - EdgeIndex == 1)) {
                    // Clockwise triple jump
                    jumpCount = 3;
                }
                if (jumpCount > 0) {
                    Real64 cornerX;
                    Real64 cornerY;
                    int startIndex = (LastEdgeIndex + 1) % 4;
                    int added = 0;
                    for (int i1 = startIndex, j1 = 0; j1 < jumpCount; i1 = (i1 + 1) % 4, j1++) {
                        cornerX = cornerXs[i1];
                        cornerY = cornerYs[i1];
                        if (cornerX == LastEdgeX && cornerY == LastEdgeY) {
                            continue; // skip if jump started on corner
                        }

                        bool insideFlag = true;
                        for (int j = 0; j < NV1; ++j) {
                            if ((state.dataSolarShading->ATEMP[j] * cornerX) + (cornerY * state.dataSolarShading->BTEMP[j]) +
                                    state.dataSolarShading->CTEMP[j] >
                                0.0) {
                                insideFlag = false;
                                break;
                            }
                        }

                        if (insideFlag &&
                            (incr == 0 ||
                             ((neq(cornerX, state.dataSolarShading->XTEMP[incr - 1]) || neq(cornerY, state.dataSolarShading->YTEMP[incr - 1])) &&
                              (neq(cornerX, state.dataSolarShading->XTEMP[0]) || neq(cornerY, state.dataSolarShading->YTEMP[0]))))) {
                            state.dataSolarShading->XTEMP[incr] = cornerX;
                            state.dataSolarShading->YTEMP[incr] = cornerY;
                            incr++;
                            added++;
                        }
                    }
                    if (jumpCount > 2 && (added == jumpCount && edgeCount == 1)) {
                        if (i != arrc) {
                            state.dataSolarShading->XTEMP[incr] = currX;
                            state.dataSolarShading->YTEMP[incr] = currY;
                            incr++;
                        }
                        break;
                    }
                }
            }
            if (i != arrc) {
                state.dataSolarShading->XTEMP[incr] = currX;
                state.dataSolarShading->YTEMP[incr] = currY;
                incr++;
            }
            LastEdgeIndex = EdgeIndex;
            LastEdgeX = currX;
            LastEdgeY = currY;
        }
        NV3 = incr;

    } else {
        if (NV3 == 1) {
            state.dataSolarShading->XTEMP[0] = arrx[0];
            state.dataSolarShading->YTEMP[0] = arry[0];
        }
        if (NV3 == 0) {
            double cornerXs[4] = {minX, minX, maxX, maxX};
            double cornerYs[4] = {minY, maxY, maxY, minY};
            Real64 cornerX = cornerXs[0];
            Real64 cornerY = cornerYs[0];
            bool insideFlag = true;
            for (int j = 0; j < NV1; ++j) {
                if ((state.dataSolarShading->ATEMP[j] * cornerX) + (cornerY * state.dataSolarShading->BTEMP[j]) + state.dataSolarShading->CTEMP[j] >=
                    0.0) {
                    insideFlag = false;
                    break;
                }
            }
            if (insideFlag) {
                for (int i1 = 0; i1 < 4; i1++) {
                    state.dataSolarShading->XTEMP[i1] = cornerXs[i1];
                    state.dataSolarShading->YTEMP[i1] = cornerYs[i1];
                }
                NV3 = 4;
                INTFLAG = true;
            }
        }
    }

    // update homogeneous edges A,B,C
    if (NV3 > 0) {
        Real64 const X_0(state.dataSolarShading->XTEMP[0]);
        Real64 const Y_0(state.dataSolarShading->YTEMP[0]);
        Real64 XP_0 = X_0, XP_1;
        Real64 YP_0 = Y_0, YP_1;
        for (int P = 0; P < NV3 - 1; ++P) {
            XP_1 = state.dataSolarShading->XTEMP[P + 1];
            YP_1 = state.dataSolarShading->YTEMP[P + 1];

            state.dataSolarShading->ATEMP[P] = YP_0 - YP_1;
            state.dataSolarShading->BTEMP[P] = XP_1 - XP_0;
            state.dataSolarShading->CTEMP[P] = XP_0 * YP_1 - YP_0 * XP_1;
            XP_0 = XP_1;
            YP_0 = YP_1;
        }

        state.dataSolarShading->ATEMP[NV3 - 1] = YP_1 - Y_0;
        state.dataSolarShading->BTEMP[NV3 - 1] = X_0 - XP_1;
        state.dataSolarShading->CTEMP[NV3 - 1] = XP_1 * Y_0 - YP_1 * X_0;
    }

    // Determine overlap status
    if (NV3 < 3) { // Determine overlap status
        state.dataSolarShading->OverlapStatus = NoOverlap;
    } else if (!INTFLAG) {
        state.dataSolarShading->OverlapStatus = FirstSurfWithinSecond;
    }
}

void CLIPPOLY(EnergyPlusData &state,
              int const NS1, // Figure number of figure 1 (The subject polygon)
              int const NS2, // Figure number of figure 2 (The clipping polygon)
              int const NV1, // Number of vertices of figure 1
              int const NV2, // Number of vertices of figure 2
              int &NV3       // Number of vertices of figure 3
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Tyler Hoyt
    //       DATE WRITTEN   May 4, 2010
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Populate global arrays XTEMP and YTEMP with the vertices
    // of the overlap between NS1 and NS2, and determine relevant
    // overlap status.

    // METHODOLOGY EMPLOYED:
    // The Sutherland-Hodgman algorithm for polygon clipping is employed.

    using General::SafeDivide;

    typedef Array2D<Int64>::size_type size_type;
    bool INTFLAG; // For overlap status
    int S;        // Test vertex
    int KK;       // Duplicate test index
    int NVOUT;    // Current output length for loops
    int NVTEMP;

    Real64 W; // Normalization factor
    Real64 HFunct;

    auto &s_surf = state.dataSurface;

#ifdef EP_Count_Calls
    ++state.dataTimingsData->NumClipPoly_Calls;
#endif
    // Tuned Linear indexing

    assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCY));
    assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCA));
    assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCB));
    assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCC));

    // Populate the arrays with the original polygon
    for (size_type j = 0, l = state.dataSolarShading->HCX.index(NS1, 1), e = NV1; j < e; ++j, ++l) {
        state.dataSolarShading->XTEMP[j] = state.dataSolarShading->HCX[l]; // [ l ] == ( NS1, j+1 )
        state.dataSolarShading->YTEMP[j] = state.dataSolarShading->HCY[l];
        state.dataSolarShading->ATEMP[j] = state.dataSolarShading->HCA[l];
        state.dataSolarShading->BTEMP[j] = state.dataSolarShading->HCB[l];
        state.dataSolarShading->CTEMP[j] = state.dataSolarShading->HCC[l];
    }

    NVOUT = NV1; // First point-loop is the length of the subject polygon.
    INTFLAG = false;
    NVTEMP = 0;
    KK = 0;

    // Check if clipping polygon is rectangle
    if (state.dataSysVars->SlaterBarsky) {
        int l1 = state.dataSolarShading->HCA.index(NS2, 1);
        bool rectFlag = ((NV2 == 4) && (((((state.dataSolarShading->HCX[l1] == state.dataSolarShading->HCX[l1 + 1] &&
                                            state.dataSolarShading->HCY[l1] != state.dataSolarShading->HCY[l1 + 1]) &&
                                           ((state.dataSolarShading->HCY[l1 + 2] == state.dataSolarShading->HCY[l1 + 1] &&
                                             state.dataSolarShading->HCY[l1 + 3] == state.dataSolarShading->HCY[l1]))) &&
                                          state.dataSolarShading->HCX[l1 + 2] == state.dataSolarShading->HCX[l1 + 3]) ||
                                         ((((state.dataSolarShading->HCY[l1] == state.dataSolarShading->HCY[l1 + 1] &&
                                             state.dataSolarShading->HCX[l1] != state.dataSolarShading->HCX[l1 + 1]) &&
                                            (state.dataSolarShading->HCX[l1 + 2] == state.dataSolarShading->HCX[l1 + 1] &&
                                             state.dataSolarShading->HCX[l1 + 3] == state.dataSolarShading->HCX[l1])) &&
                                           (state.dataSolarShading->HCY[l1 + 2] == state.dataSolarShading->HCY[l1 + 3]))))));
        if (rectFlag) {
            CLIPRECT(state, NS2, NV1, NV3);
            return;
        }
    }

    int l = state.dataSolarShading->HCA.index(NS2, 1);
    for (int E = 1; E <= NV2; ++E, ++l) { // Loop over edges of the clipping polygon
        for (int P = 1; P <= NVOUT; ++P) {
            state.dataSolarShading->XTEMP1(P) = state.dataSolarShading->XTEMP(P);
            state.dataSolarShading->YTEMP1(P) = state.dataSolarShading->YTEMP(P);
        }
        S = NVOUT;
        Real64 const HCA_E(state.dataSolarShading->HCA[l]);
        Real64 const HCB_E(state.dataSolarShading->HCB[l]);
        Real64 const HCC_E(state.dataSolarShading->HCC[l]);
        Real64 XTEMP1_S(state.dataSolarShading->XTEMP1(S));
        Real64 YTEMP1_S(state.dataSolarShading->YTEMP1(S));
        for (int P = 1; P <= NVOUT; ++P) {
            Real64 const XTEMP1_P(state.dataSolarShading->XTEMP1(P));
            Real64 const YTEMP1_P(state.dataSolarShading->YTEMP1(P));
            HFunct = XTEMP1_P * HCA_E + YTEMP1_P * HCB_E + HCC_E;
            // S is constant within this block
            if (HFunct <= 0.0) { // Vertex is not in the clipping plane
                HFunct = XTEMP1_S * HCA_E + YTEMP1_S * HCB_E + HCC_E;
                if (HFunct > 0.0) { // Test vertex is in the clipping plane

                    // Find/store the intersection of the clip edge and the line connecting S and P
                    KK = NVTEMP;
                    ++NVTEMP;
                    Real64 const ATEMP_S(state.dataSolarShading->ATEMP(S));
                    Real64 const BTEMP_S(state.dataSolarShading->BTEMP(S));
                    Real64 const CTEMP_S(state.dataSolarShading->CTEMP(S));
                    W = HCB_E * ATEMP_S - HCA_E * BTEMP_S;
                    if (W != 0.0) {
                        Real64 const W_inv(1.0 / W);
                        state.dataSolarShading->XTEMP(NVTEMP) = nint64((HCC_E * BTEMP_S - HCB_E * CTEMP_S) * W_inv);
                        state.dataSolarShading->YTEMP(NVTEMP) = nint64((HCA_E * CTEMP_S - HCC_E * ATEMP_S) * W_inv);
                    } else {
                        state.dataSolarShading->XTEMP(NVTEMP) = SafeDivide(HCC_E * BTEMP_S - HCB_E * CTEMP_S, W);
                        state.dataSolarShading->YTEMP(NVTEMP) = SafeDivide(HCA_E * CTEMP_S - HCC_E * ATEMP_S, W);
                    }
                    INTFLAG = true;

                    if (E == NV2) { // Remove near-duplicates on last edge
                        if (KK != 0) {
                            Real64 const x(state.dataSolarShading->XTEMP(NVTEMP));
                            Real64 const y(state.dataSolarShading->YTEMP(NVTEMP));
                            for (int K = 1; K <= KK; ++K) {
                                if (std::abs(x - state.dataSolarShading->XTEMP(K)) > 2.0) {
                                    continue;
                                }
                                if (std::abs(y - state.dataSolarShading->YTEMP(K)) > 2.0) {
                                    continue;
                                }
                                NVTEMP = KK;
                                break; // K loop
                            }
                        }
                    }
                }

                KK = NVTEMP;
                ++NVTEMP;
                if (NVTEMP > state.dataSolarShading->MAXHCArrayBounds) {
                    int const NewArrayBounds(state.dataSolarShading->MAXHCArrayBounds + state.dataSolarShading->MAXHCArrayIncrement);
                    state.dataSolarShading->XTEMP.redimension(NewArrayBounds, 0.0);
                    state.dataSolarShading->YTEMP.redimension(NewArrayBounds, 0.0);
                    state.dataSolarShading->XTEMP1.redimension(NewArrayBounds, 0.0);
                    state.dataSolarShading->YTEMP1.redimension(NewArrayBounds, 0.0);
                    state.dataSolarShading->ATEMP.redimension(NewArrayBounds, 0.0);
                    state.dataSolarShading->BTEMP.redimension(NewArrayBounds, 0.0);
                    state.dataSolarShading->CTEMP.redimension(NewArrayBounds, 0.0);
                    state.dataSolarShading->MAXHCArrayBounds = NewArrayBounds;
                }

                state.dataSolarShading->XTEMP(NVTEMP) = XTEMP1_P;
                state.dataSolarShading->YTEMP(NVTEMP) = YTEMP1_P;

                if (E == NV2) { // Remove near-duplicates on last edge
                    if (KK != 0) {
                        Real64 const x(state.dataSolarShading->XTEMP(NVTEMP));
                        Real64 const y(state.dataSolarShading->YTEMP(NVTEMP));
                        for (int K = 1; K <= KK; ++K) {
                            if (std::abs(x - state.dataSolarShading->XTEMP(K)) > 2.0) {
                                continue;
                            }
                            if (std::abs(y - state.dataSolarShading->YTEMP(K)) > 2.0) {
                                continue;
                            }
                            NVTEMP = KK;
                            break; // K loop
                        }
                    }
                }

            } else {
                HFunct = XTEMP1_S * HCA_E + YTEMP1_S * HCB_E + HCC_E;
                if (HFunct <= 0.0) {                                        // Test vertex is not in the clipping plane
                    if (NVTEMP < 2 * (s_surf->MaxVerticesPerSurface + 1)) { // avoid assigning to element outside of XTEMP array size
                        KK = NVTEMP;
                        ++NVTEMP;
                        Real64 const ATEMP_S(state.dataSolarShading->ATEMP(S));
                        Real64 const BTEMP_S(state.dataSolarShading->BTEMP(S));
                        Real64 const CTEMP_S(state.dataSolarShading->CTEMP(S));
                        W = HCB_E * ATEMP_S - HCA_E * BTEMP_S;
                        if (W != 0.0) {
                            Real64 const W_inv(1.0 / W);
                            state.dataSolarShading->XTEMP(NVTEMP) = nint64((HCC_E * BTEMP_S - HCB_E * CTEMP_S) * W_inv);
                            state.dataSolarShading->YTEMP(NVTEMP) = nint64((HCA_E * CTEMP_S - HCC_E * ATEMP_S) * W_inv);
                        } else {
                            state.dataSolarShading->XTEMP(NVTEMP) = SafeDivide(HCC_E * BTEMP_S - HCB_E * CTEMP_S, W);
                            state.dataSolarShading->YTEMP(NVTEMP) = SafeDivide(HCA_E * CTEMP_S - HCC_E * ATEMP_S, W);
                        }
                        INTFLAG = true;

                        if (E == NV2) { // Remove near-duplicates on last edge
                            if (KK != 0) {
                                Real64 const x(state.dataSolarShading->XTEMP(NVTEMP));
                                Real64 const y(state.dataSolarShading->YTEMP(NVTEMP));
                                for (int K = 1; K <= KK; ++K) {
                                    if (std::abs(x - state.dataSolarShading->XTEMP(K)) > 2.0) {
                                        continue;
                                    }
                                    if (std::abs(y - state.dataSolarShading->YTEMP(K)) > 2.0) {
                                        continue;
                                    }
                                    NVTEMP = KK;
                                    break; // K loop
                                }
                            }
                        }
                    }
                }
            }
            S = P;
            XTEMP1_S = XTEMP1_P;
            YTEMP1_S = YTEMP1_P;
        } // end loop over points of subject polygon

        NVOUT = NVTEMP;
        if (NVOUT == 0) {
            break; // Added to avoid array bounds violation of XTEMP1 and YTEMP1 and wasted looping
        }
        NVTEMP = 0;

        if (E != NV2) {
            if (NVOUT > 2) { // Compute HC values for edges of output polygon
                Real64 const X_1(state.dataSolarShading->XTEMP(1));
                Real64 const Y_1(state.dataSolarShading->YTEMP(1));
                Real64 X_P(X_1), X_P1;
                Real64 Y_P(Y_1), Y_P1;
                for (int P = 1; P < NVOUT; ++P) {
                    X_P1 = state.dataSolarShading->XTEMP(P + 1);
                    Y_P1 = state.dataSolarShading->YTEMP(P + 1);
                    state.dataSolarShading->ATEMP(P) = Y_P - Y_P1;
                    state.dataSolarShading->BTEMP(P) = X_P1 - X_P;
                    state.dataSolarShading->CTEMP(P) = X_P * Y_P1 - Y_P * X_P1;
                    X_P = X_P1;
                    Y_P = Y_P1;
                }
                state.dataSolarShading->ATEMP(NVOUT) = Y_P1 - Y_1;
                state.dataSolarShading->BTEMP(NVOUT) = X_1 - X_P1;
                state.dataSolarShading->CTEMP(NVOUT) = X_P1 * Y_1 - Y_P1 * X_1;
            }
        }

    } // end loop over edges in NS2

    NV3 = NVOUT;

    if (NV3 < 3) { // Determine overlap status
        state.dataSolarShading->OverlapStatus = NoOverlap;
    } else if (!INTFLAG) {
        state.dataSolarShading->OverlapStatus = FirstSurfWithinSecond;
    }
}

void MULTOL(EnergyPlusData &state,
            int const NNN,   // argument
            int const LOC0,  // Location in the homogeneous coordinate array
            int const NRFIGS // Number of figures overlapped
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy Code
    //       DATE WRITTEN
    //       MODIFIED       na
    //       RE-ENGINEERED  Lawrie, Oct 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine determines the overlaps of figure 'NS2' with previous figures
    // 'LOC0+1' through 'LOC0+NRFIGS'.  For example, if NS2
    // is a shadow, overlap with previous shadows.

    // REFERENCES:
    // BLAST/IBLAST code, original author George Walton

    int I;   // Loop Control
    int NS1; // Number of the figure being overlapped
    int NS2; // Number of the figure doing overlapping
    int NS3; // Location to place results of overlap

    state.dataSolarShading->maxNumberOfFigures = max(state.dataSolarShading->maxNumberOfFigures, NRFIGS);

    NS2 = NNN;
    for (I = 1; I <= NRFIGS; ++I) {
        NS1 = LOC0 + I;
        NS3 = state.dataSolarShading->LOCHCA + 1;

        DeterminePolygonOverlap(state, NS1, NS2, NS3); // Find overlap of figure NS2 on figure NS1.

        // Process overlap cases:

        if (state.dataSolarShading->OverlapStatus == NoOverlap) {
            continue;
        }

        if ((state.dataSolarShading->OverlapStatus == TooManyVertices) || (state.dataSolarShading->OverlapStatus == TooManyFigures)) {
            break;
        }

        state.dataSolarShading->LOCHCA = NS3; // Increment h.c. arrays pointer.
    }
}

void ORDER(EnergyPlusData &state,
           int const NV3, // Number of vertices of figure NS3
           int const NS3  // Location to place results of overlap
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy Code
    //       DATE WRITTEN
    //       MODIFIED       na
    //       RE-ENGINEERED  Lawrie, Oct 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine sorts the vertices found by inclosure and
    // intercept in to clockwise order so that the overlap polygon
    // may be used in computing subsequent overlaps.

    // METHODOLOGY EMPLOYED:
    // The slopes of the lines from the left-most vertex to all
    // others are found.  The slopes are sorted into descending
    // sequence.  This sequence puts the vertices in clockwise order.

    // REFERENCES:
    // BLAST/IBLAST code, original author George Walton

    Real64 DELTAX; // Difference between X coordinates of two vertices
    Real64 DELTAY; // Difference between Y coordinates of two vertices
    Real64 SAVES;  // Temporary location for exchange of variables
    Real64 SAVEX;  // Temporary location for exchange of variables
    Real64 SAVEY;  // Temporary location for exchange of variables
    Real64 XMIN;   // X coordinate of left-most vertex
    Real64 YXMIN;
    int I;   // Sort index
    int IM1; // Sort control
    int J;   // Sort index
    int M;   // Number of slopes to be sorted
    int N;   // Vertex number
    int P;   // Location of first slope to be sorted

    auto &s_surf = state.dataSurface;

    if (state.dataSolarShading->ORDERFirstTimeFlag) {
        state.dataSolarShading->SLOPE.allocate(max(10, s_surf->MaxVerticesPerSurface + 1));
        state.dataSolarShading->ORDERFirstTimeFlag = false;
    }
    // Determine left-most vertex.

    XMIN = state.dataSolarShading->XTEMP(1);
    YXMIN = state.dataSolarShading->YTEMP(1);
    for (N = 2; N <= NV3; ++N) {
        if (state.dataSolarShading->XTEMP(N) >= XMIN) {
            continue;
        }
        XMIN = state.dataSolarShading->XTEMP(N);
        YXMIN = state.dataSolarShading->YTEMP(N);
    }

    // Determine slopes from left-most vertex to all others.  Identify
    // first and second or last points as they occur.

    P = 1;
    M = 0;
    for (N = 1; N <= NV3; ++N) {

        DELTAX = state.dataSolarShading->XTEMP(N) - XMIN;
        DELTAY = state.dataSolarShading->YTEMP(N) - YXMIN;

        if (std::abs(DELTAX) > 0.5) {

            ++M;
            state.dataSolarShading->SLOPE(M) = DELTAY / DELTAX;
            state.dataSolarShading->XTEMP(M) = state.dataSolarShading->XTEMP(N);
            state.dataSolarShading->YTEMP(M) = state.dataSolarShading->YTEMP(N);

        } else if (DELTAY > 0.5) {

            P = 2;
            state.dataSolarShading->HCX(NS3, 2) = nint64(state.dataSolarShading->XTEMP(N));
            state.dataSolarShading->HCY(NS3, 2) = nint64(state.dataSolarShading->YTEMP(N));

        } else if (DELTAY < -0.5) {

            state.dataSolarShading->HCX(NS3, NV3) = nint64(state.dataSolarShading->XTEMP(N));
            state.dataSolarShading->HCY(NS3, NV3) = nint64(state.dataSolarShading->YTEMP(N));

        } else {

            state.dataSolarShading->HCX(NS3, 1) = nint64(XMIN);
            state.dataSolarShading->HCY(NS3, 1) = nint64(YXMIN);
        }
    }

    // Sequence the temporary arrays in order of decreasing slopes.(bubble sort)

    if (M != 1) {

        for (I = 2; I <= M; ++I) {
            IM1 = I - 1;
            for (J = 1; J <= IM1; ++J) {
                if (state.dataSolarShading->SLOPE(I) <= state.dataSolarShading->SLOPE(J)) {
                    continue;
                }
                SAVEX = state.dataSolarShading->XTEMP(I);
                SAVEY = state.dataSolarShading->YTEMP(I);
                SAVES = state.dataSolarShading->SLOPE(I);
                state.dataSolarShading->XTEMP(I) = state.dataSolarShading->XTEMP(J);
                state.dataSolarShading->YTEMP(I) = state.dataSolarShading->YTEMP(J);
                state.dataSolarShading->SLOPE(I) = state.dataSolarShading->SLOPE(J);
                state.dataSolarShading->XTEMP(J) = SAVEX;
                state.dataSolarShading->YTEMP(J) = SAVEY;
                state.dataSolarShading->SLOPE(J) = SAVES;
            }
        }
    }

    // Place sequenced points in the homogeneous coordinate arrays.

    for (N = 1; N <= M; ++N) {
        state.dataSolarShading->HCX(NS3, N + P) = nint64(state.dataSolarShading->XTEMP(N));
        state.dataSolarShading->HCY(NS3, N + P) = nint64(state.dataSolarShading->YTEMP(N));
    }
}

void DeterminePolygonOverlap(EnergyPlusData &state,
                             int const NS1, // Number of the figure being overlapped
                             int const NS2, // Number of the figure doing overlapping
                             int const NS3  // Location to place results of overlap
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy Code
    //       RE-ENGINEERED  Lawrie, Oct 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine computes the possible overlap of two polygons.
    // It uses homogeneous coordinate techniques to determine the overlap area
    // between two convex polygons.  Results are stored in the homogeneous coordinate (HC) arrays.

    // METHODOLOGY EMPLOYED:
    // The vertices defining the overlap between fig.1 and fig.2
    // consist of: the vertices of fig.1 enclosed by fig.2 (A)
    // plus the vertices of fig.2 enclosed by fig.1 (B)
    // plus the intercepts of fig.1 and fig.2 (C & D)

    //                               +----------------------+
    //                               !                      !
    //                               !         FIG.2        !
    //                               !                      !
    //                +--------------C----------A           !
    //                !              !         /            !
    //                !              !        /             !
    //                !              B-------D--------------+
    //                !    FIG.1            /
    //                !                    /
    //                +-------------------+

    // REFERENCES:
    // BLAST/IBLAST code, original author George Walton

#ifdef EP_Count_Calls
    ++state.dataTimingsData->NumDetPolyOverlap_Calls;
#endif

    // Check for exceeding array limits.
    if (NS3 > state.dataSolarShading->MaxHCS) {

        state.dataSolarShading->OverlapStatus = TooManyFigures;

        if (!state.dataSolarShading->TooManyFiguresMessage && !state.dataGlobal->DisplayExtraWarnings) {
            ShowWarningError(state,
                             format("DeterminePolygonOverlap: Too many figures [>{}]  detected in an overlap calculation. Use "
                                    "Output:Diagnostics,DisplayExtraWarnings; for more details.",
                                    state.dataSolarShading->MaxHCS));
            state.dataSolarShading->TooManyFiguresMessage = true;
        }

        if (state.dataGlobal->DisplayExtraWarnings) {
            state.dataSolarShading->TrackTooManyFigures.redimension(++state.dataSolarShading->NumTooManyFigures);
            state.dataSolarShading->TrackTooManyFigures(state.dataSolarShading->NumTooManyFigures).SurfIndex1 =
                state.dataSolarShading->CurrentShadowingSurface;
            state.dataSolarShading->TrackTooManyFigures(state.dataSolarShading->NumTooManyFigures).SurfIndex2 =
                state.dataSolarShading->CurrentSurfaceBeingShadowed;
        }

        return;
    }

    state.dataSolarShading->OverlapStatus = PartialOverlap;
    int NV1 = state.dataSolarShading->HCNV(NS1); // Number of vertices of figure NS1
    int NV2 = state.dataSolarShading->HCNV(NS2); // Number of vertices of figure NS2
    int NV3 = 0;                                 // Number of vertices of figure NS3 (the overlap of NS1 and NS2)
    int NIN1 = 0;                                // Number of vertices of NS1 within NS2
    int NIN2 = 0;                                // Number of vertices of NS2 within NS1

    if (!state.dataSysVars->SutherlandHodgman) {
        INCLOS(state, NS1, NV1, NS2, NV2, NV3, NIN1); // Find vertices of NS1 within NS2.

        if (NIN1 >= NV1) {

            state.dataSolarShading->OverlapStatus = FirstSurfWithinSecond;

        } else {

            INCLOS(state, NS2, NV2, NS1, NV1, NV3, NIN2); // Find vertices of NS2 within NS1.

            if (NIN2 >= NV2) {

                state.dataSolarShading->OverlapStatus = SecondSurfWithinFirst;

            } else {

                INTCPT(state, NV1, NV2, NV3, NS1, NS2); // Find intercepts of NS1 & NS2.

                if (NV3 < 3) { // Overlap must have 3 or more vertices
                    state.dataSolarShading->OverlapStatus = NoOverlap;
                    return;
                }
            }
        }

    } else {
        // simple polygon clipping
        CLIPPOLY(state, NS1, NS2, NV1, NV2, NV3);
    }

    if (NV3 < state.dataSolarShading->MaxHCV) {

        if (!state.dataSysVars->SutherlandHodgman) {
            ORDER(state, NV3, NS3); // Put vertices in clockwise order.
        } else {
            assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCY));
            int l = state.dataSolarShading->HCX.index(NS3, 1);
            for (int N = 1; N <= NV3; ++N, ++l) {
                state.dataSolarShading->HCX[l] = nint64(state.dataSolarShading->XTEMP(N)); // [ l ] == ( N, NS3 )
                state.dataSolarShading->HCY[l] = nint64(state.dataSolarShading->YTEMP(N));
            }
        }

        HTRANS0(state, NS3, NV3); // Determine h.c. values of sides.
        // Skip overlaps of negligible area.

        if (std::abs(state.dataSolarShading->HCAREA(NS3)) * HCMULT < std::abs(state.dataSolarShading->HCAREA(NS1))) {
            state.dataSolarShading->OverlapStatus = NoOverlap;
        } else {
            if (state.dataSolarShading->HCAREA(NS1) * state.dataSolarShading->HCAREA(NS2) > 0.0) {
                state.dataSolarShading->HCAREA(NS3) = -state.dataSolarShading->HCAREA(NS3); // Determine sign of area of overlap
            }
            Real64 const HCT_1 = state.dataSolarShading->HCT(NS1);
            Real64 const HCT_2 = state.dataSolarShading->HCT(NS2);
            if (HCT_2 == 1.0 || HCT_1 == 1.0) {
                state.dataSolarShading->HCT(NS3) = HCT_1 * HCT_2;
            } else {
                // Determine transmission of overlap which corrects for prior shadows
                // The resulting transmission of overlapping shadows is HCT_1 * HCT_2
                // Shadows with HCT_1 and HCT_2 have already been applied in the overlapping area
                // so the correction is the difference between (HCT_1+HCT_2) and (HCT_1*HCT_2)
                state.dataSolarShading->HCT(NS3) = (HCT_1 + HCT_2) - HCT_1 * HCT_2;
            }
        }

    } else {

        state.dataSolarShading->OverlapStatus = TooManyVertices;

        if (!state.dataSolarShading->TooManyVerticesMessage && !state.dataGlobal->DisplayExtraWarnings) {
            ShowWarningError(state,
                             format("DeterminePolygonOverlap: Too many vertices [>{}] detected in an overlap calculation. Use "
                                    "Output:Diagnostics,DisplayExtraWarnings; for more details.",
                                    state.dataSolarShading->MaxHCV));
            state.dataSolarShading->TooManyVerticesMessage = true;
        }

        if (state.dataGlobal->DisplayExtraWarnings) {
            state.dataSolarShading->TrackTooManyVertices.redimension(++state.dataSolarShading->NumTooManyVertices);
            state.dataSolarShading->TrackTooManyVertices(state.dataSolarShading->NumTooManyVertices).SurfIndex1 =
                state.dataSolarShading->CurrentShadowingSurface;
            state.dataSolarShading->TrackTooManyVertices(state.dataSolarShading->NumTooManyVertices).SurfIndex2 =
                state.dataSolarShading->CurrentSurfaceBeingShadowed;
        }
    }
}

void CalcPerSolarBeam(EnergyPlusData &state,
                      Real64 const AvgEqOfTime,       // Average value of Equation of Time for period
                      Real64 const AvgSinSolarDeclin, // Average value of Sine of Solar Declination for period
                      Real64 const AvgCosSolarDeclin  // Average value of Cosine of Solar Declination for period
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy Code
    //       DATE WRITTEN
    //       MODIFIED       BG, Nov 2012 - Timestep solar.  DetailedSolarTimestepIntegration
    //       RE-ENGINEERED  Lawrie, Oct 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine manages computation of solar gain multipliers for beam radiation.  These
    // are calculated for a period of days depending on the input "Shadowing Calculations".

    // REFERENCES:
    // BLAST/IBLAST code, original author George Walton

    // Using/Aliasing

    using WindowComplexManager::InitComplexWindows;
    using WindowComplexManager::UpdateComplexWindows;

    int iHour; // Hour index number
    int TS;    // TimeStep Loop Countergit

    auto &s_surf = state.dataSurface;

    if (state.dataSolarShading->InitComplexOnce) {
        InitComplexWindows(state);
    }
    state.dataSolarShading->InitComplexOnce = false;

    if (state.dataGlobal->KickOffSizing || state.dataGlobal->KickOffSimulation) {
        return; // Skip solar calcs for these Initialization steps.
    }

#ifdef EP_Count_Calls
    ++state.dataTimingsData->NumCalcPerSolBeam_Calls;
#endif

    // Initialize some values for the appropriate period
    if (!state.dataSysVars->DetailedSolarTimestepIntegration) {

        // Array1D and 1D-ish
        for (int surfNum = 1; surfNum <= state.dataSurface->TotSurfaces; ++surfNum) {
            s_surf->SurfOpaqAO(surfNum) = 0.0;
            state.dataSolarShading->SurfSunCosTheta(surfNum) = 0.0;

            for (int hour = 1; hour <= 24; ++hour) {
                s_surf->SurfaceWindow(surfNum).OutProjSLFracMult[hour] = 1.0;
                s_surf->SurfaceWindow(surfNum).InOutProjSLFracMult[hour] = 1.0;
            }
        }

        // Array2D
        for (int hour = 1; hour <= 24; ++hour) {
            for (int surfNum = 1; surfNum <= state.dataSurface->TotSurfaces; ++surfNum) {
                state.dataHeatBal->SurfSunlitFracHR(hour, surfNum) = 0.0;
                state.dataHeatBal->SurfCosIncAngHR(hour, surfNum) = 0.0;
            }
        }

        // Array3D
        for (int hour = 1; hour <= 24; ++hour) {
            for (int timestep = 1; timestep <= state.dataGlobal->TimeStepsInHour; ++timestep) {
                for (int surfNum = 1; surfNum <= state.dataSurface->TotSurfaces; ++surfNum) {
                    state.dataHeatBal->SurfSunlitFrac(hour, timestep, surfNum) = 0.0;
                    state.dataHeatBal->SurfCosIncAng(hour, timestep, surfNum) = 0.0;
                    state.dataHeatBal->SurfSunlitFracWithoutReveal(hour, timestep, surfNum) = 0.0;
                }
            }
        }

        // Array4D
        for (int hour = 1; hour <= 24; ++hour) {
            for (int timestep = 1; timestep <= state.dataGlobal->TimeStepsInHour; ++timestep) {
                for (int backSurfNum = 1; backSurfNum <= state.dataBSDFWindow->MaxBkSurf; ++backSurfNum) {
                    for (int surfNum = 1; surfNum <= state.dataSurface->TotSurfaces; ++surfNum) {
                        state.dataHeatBal->SurfWinBackSurfaces(hour, timestep, backSurfNum, surfNum) = 0.0;
                        state.dataHeatBal->SurfWinOverlapAreas(hour, timestep, backSurfNum, surfNum) = 0.0;
                    }
                }
            }
        }

    } else {
        for (int surfNum = 1; surfNum <= state.dataSurface->TotSurfaces; ++surfNum) {
            state.dataSolarShading->SurfSunCosTheta(surfNum) = 0.0;
            s_surf->SurfOpaqAO(surfNum) = 0.0;

            s_surf->SurfaceWindow(surfNum).OutProjSLFracMult[state.dataGlobal->HourOfDay] = 1.0;
            s_surf->SurfaceWindow(surfNum).InOutProjSLFracMult[state.dataGlobal->HourOfDay] = 1.0;

            // Array2D
            state.dataHeatBal->SurfSunlitFracHR(state.dataGlobal->HourOfDay, surfNum) = 0.0;
            state.dataHeatBal->SurfCosIncAngHR(state.dataGlobal->HourOfDay, surfNum) = 0.0;

            // Array3D
            state.dataHeatBal->SurfSunlitFrac(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, surfNum) = 0.0;
            state.dataHeatBal->SurfSunlitFracWithoutReveal(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, surfNum) = 0.0;
            state.dataHeatBal->SurfCosIncAng(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, surfNum) = 0.0;
        }

        // Array4D
        for (int backSurfNum = 1; backSurfNum <= state.dataBSDFWindow->MaxBkSurf; ++backSurfNum) {
            for (int surfNum = 1; surfNum <= state.dataSurface->TotSurfaces; ++surfNum) {
                state.dataHeatBal->SurfWinBackSurfaces(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, backSurfNum, surfNum) = 0;
                state.dataHeatBal->SurfWinOverlapAreas(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, backSurfNum, surfNum) = 0.0;
            }
        }
    }

    if (!state.dataSysVars->DetailedSolarTimestepIntegration) {
        for (iHour = 1; iHour <= 24; ++iHour) { // Do for all hours
            for (TS = 1; TS <= state.dataGlobal->TimeStepsInHour; ++TS) {
                FigureSunCosines(state, iHour, TS, AvgEqOfTime, AvgSinSolarDeclin, AvgCosSolarDeclin);
            }
        }
    } else {
        FigureSunCosines(state, state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, AvgEqOfTime, AvgSinSolarDeclin, AvgCosSolarDeclin);
    }
    // Initialize/update the Complex Fenestration geometry and optical properties
    UpdateComplexWindows(state);
    if (!state.dataSysVars->DetailedSolarTimestepIntegration) {
        for (iHour = 1; iHour <= 24; ++iHour) { // Do for all hours.
            for (TS = 1; TS <= state.dataGlobal->TimeStepsInHour; ++TS) {
                FigureSolarBeamAtTimestep(state, iHour, TS);
            } // TimeStep Loop
        } // Hour Loop
    } else {
        FigureSolarBeamAtTimestep(state, state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep);
    }
}

void FigureSunCosines(EnergyPlusData &state,
                      int const iHour,
                      int const iTimeStep,
                      Real64 const EqOfTime,       // value of Equation of Time for period
                      Real64 const SinSolarDeclin, // value of Sine of Solar Declination for period
                      Real64 const CosSolarDeclin  // value of Cosine of Solar Declination for period
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   October 2012
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Determine solar position.  Default for sun below horizon.

    // METHODOLOGY EMPLOYED:
    // Given hour, timestep, equation of time, solar declination sine, and solar declination cosine,
    // determine sun directions for use elsewhere

    // Using/Aliasing

    Real64 CurrentTime; // Current Time for passing to Solar Position Routine

    auto &s_surf = state.dataSurface;

    if (state.dataGlobal->TimeStepsInHour != 1) {
        CurrentTime = double(iHour - 1) + double(iTimeStep) * (state.dataGlobal->TimeStepZone);
    } else {
        CurrentTime = double(iHour) + state.dataEnvrn->TS1TimeOffset;
    }
    SUN4(state, CurrentTime, EqOfTime, SinSolarDeclin, CosSolarDeclin);

    // Save hourly values for use in DaylightingManager
    if (!state.dataSysVars->DetailedSolarTimestepIntegration) {
        if (iTimeStep == state.dataGlobal->TimeStepsInHour) {
            s_surf->SurfSunCosHourly(iHour) = state.dataSolarShading->SUNCOS;
        }
    } else {
        s_surf->SurfSunCosHourly(iHour) = state.dataSolarShading->SUNCOS;
    }
    // Save timestep values for use in WindowComplexManager
    state.dataBSDFWindow->SUNCOSTS(iTimeStep, iHour) = state.dataSolarShading->SUNCOS;
}

void FigureSolarBeamAtTimestep(EnergyPlusData &state, int const iHour, int const iTimeStep)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B.Griffith, derived from CalcPerSolarBeam, Legacy and Lawrie.
    //       DATE WRITTEN   October 2012
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine computes solar gain multipliers for beam solar

    using DataSystemVariables::ShadingMethod;

    Real64 SurfArea;        // Surface area. For walls, includes all window frame areas.
    Real64 Fac1WoShdg;      // Intermediate calculation factor, without shading
    Real64 Fac1WithShdg;    // Intermediate calculation factor, with shading
    Real64 FracIlluminated; // Fraction of surface area illuminated by a sky patch

    auto &s_surf = state.dataSurface;

    // Recover the sun direction from the array stored in previous loop
    state.dataSolarShading->SUNCOS = state.dataBSDFWindow->SUNCOSTS(iTimeStep, iHour);

    state.dataSolarShading->SurfSunCosTheta = 0.0;

    if (state.dataSolarShading->SUNCOS(3) < DataEnvironment::SunIsUpValue) {
        return;
    }

    for (int SurfNum = 1; SurfNum <= s_surf->TotSurfaces; ++SurfNum) {
        state.dataSolarShading->SurfSunCosTheta(SurfNum) = state.dataSolarShading->SUNCOS(1) * s_surf->Surface(SurfNum).OutNormVec(1) +
                                                           state.dataSolarShading->SUNCOS(2) * s_surf->Surface(SurfNum).OutNormVec(2) +
                                                           state.dataSolarShading->SUNCOS(3) * s_surf->Surface(SurfNum).OutNormVec(3);
        if (!state.dataSysVars->DetailedSolarTimestepIntegration) {
            if (iTimeStep == state.dataGlobal->TimeStepsInHour) {
                state.dataHeatBal->SurfCosIncAngHR(iHour, SurfNum) = state.dataSolarShading->SurfSunCosTheta(SurfNum);
            }
        } else {
            state.dataHeatBal->SurfCosIncAngHR(iHour, SurfNum) = state.dataSolarShading->SurfSunCosTheta(SurfNum);
        }
        state.dataHeatBal->SurfCosIncAng(iHour, iTimeStep, SurfNum) = state.dataSolarShading->SurfSunCosTheta(SurfNum);
    }

    if ((state.dataSysVars->shadingMethod == ShadingMethod::Scheduled || state.dataSysVars->shadingMethod == ShadingMethod::Imported) &&
        !state.dataGlobal->DoingSizing && state.dataGlobal->KindOfSim == Constant::KindOfSim::RunPeriodWeather) {
        for (int SurfNum = 1; SurfNum <= s_surf->TotSurfaces; ++SurfNum) {
            auto &surf = s_surf->Surface(SurfNum);
            if (surf.SurfSchedExternalShadingFrac) {
                state.dataHeatBal->SurfSunlitFrac(iHour, iTimeStep, SurfNum) = surf.surfExternalShadingSched->getHrTsVal(state, iHour, iTimeStep);
            } else {
                state.dataHeatBal->SurfSunlitFrac(iHour, iTimeStep, SurfNum) = 1.0;
            }
        }
    } else {
        SHADOW(state, iHour, iTimeStep); // Determine sunlit areas and solar multipliers for all surfaces.
        for (int SurfNum = 1; SurfNum <= s_surf->TotSurfaces; ++SurfNum) {
            if (s_surf->Surface(SurfNum).Area >= 1.e-10) {
                SurfArea = s_surf->Surface(SurfNum).NetAreaShadowCalc;
                if (!state.dataSysVars->DetailedSolarTimestepIntegration) {
                    if (iTimeStep == state.dataGlobal->TimeStepsInHour) {
                        state.dataHeatBal->SurfSunlitFracHR(iHour, SurfNum) = state.dataSolarShading->SurfSunlitArea(SurfNum) / SurfArea;
                    }
                } else {
                    state.dataHeatBal->SurfSunlitFracHR(iHour, SurfNum) = state.dataSolarShading->SurfSunlitArea(SurfNum) / SurfArea;
                }
                state.dataHeatBal->SurfSunlitFrac(iHour, iTimeStep, SurfNum) = state.dataSolarShading->SurfSunlitArea(SurfNum) / SurfArea;
                if (state.dataHeatBal->SurfSunlitFrac(iHour, iTimeStep, SurfNum) < 1.e-5) {
                    state.dataHeatBal->SurfSunlitFrac(iHour, iTimeStep, SurfNum) = 0.0;
                }
            }
            // Added check
            if (state.dataHeatBal->SurfSunlitFrac(iHour, iTimeStep, SurfNum) > 1.0) {
                state.dataHeatBal->SurfSunlitFrac(iHour, iTimeStep, SurfNum) = 1.0;
            }
        }
    }
    //   Note -- if not the below, values are set in SkyDifSolarShading routine (constant for simulation)
    if (state.dataSysVars->DetailedSkyDiffuseAlgorithm && s_surf->ShadingTransmittanceVaries &&
        state.dataHeatBal->SolarDistribution != DataHeatBalance::Shadowing::Minimal) {
        for (int SurfNum = 1; SurfNum <= s_surf->TotSurfaces; ++SurfNum) {
            state.dataSolarShading->SurfWithShdgIsoSky(SurfNum) = 0.;
            state.dataSolarShading->SurfWoShdgIsoSky(SurfNum) = 0.;
            state.dataSolarShading->SurfWithShdgHoriz(SurfNum) = 0.;
            state.dataSolarShading->SurfWoShdgHoriz(SurfNum) = 0.;
        }

        for (int IPhi = 0; IPhi < NPhi; ++IPhi) { // Loop over patch altitude values
            state.dataSolarShading->SUNCOS(3) = state.dataSolarShading->sin_Phi[IPhi];

            for (int ITheta = 0; ITheta < NTheta; ++ITheta) { // Loop over patch azimuth values
                state.dataSolarShading->SUNCOS(1) = state.dataSolarShading->cos_Phi[IPhi] * state.dataSolarShading->cos_Theta[ITheta];
                state.dataSolarShading->SUNCOS(2) = state.dataSolarShading->cos_Phi[IPhi] * state.dataSolarShading->sin_Theta[ITheta];

                for (int SurfNum : s_surf->AllExtSolAndShadingSurfaceList) {
                    state.dataSolarShading->SurfSunCosTheta(SurfNum) = state.dataSolarShading->SUNCOS(1) * s_surf->Surface(SurfNum).OutNormVec(1) +
                                                                       state.dataSolarShading->SUNCOS(2) * s_surf->Surface(SurfNum).OutNormVec(2) +
                                                                       state.dataSolarShading->SUNCOS(3) * s_surf->Surface(SurfNum).OutNormVec(3);
                }

                SHADOW(state, iHour, iTimeStep); // Determine sunlit areas and solar multipliers for all surfaces.

                for (int SurfNum : s_surf->AllExtSolAndShadingSurfaceList) {

                    if (state.dataSolarShading->SurfSunCosTheta(SurfNum) < 0.0) {
                        continue;
                    }

                    Fac1WoShdg = state.dataSolarShading->cos_Phi[IPhi] * DThetaDPhi * state.dataSolarShading->SurfSunCosTheta(SurfNum);
                    SurfArea = s_surf->Surface(SurfNum).NetAreaShadowCalc;
                    if (SurfArea > Eps) {
                        FracIlluminated = state.dataSolarShading->SurfSunlitArea(SurfNum) / SurfArea;
                    } else {
                        FracIlluminated = state.dataSolarShading->SurfSunlitArea(SurfNum) / (SurfArea + Eps);
                    }
                    Fac1WithShdg = Fac1WoShdg * FracIlluminated;
                    state.dataSolarShading->SurfWithShdgIsoSky(SurfNum) += Fac1WithShdg;
                    state.dataSolarShading->SurfWoShdgIsoSky(SurfNum) += Fac1WoShdg;

                    // Horizon region
                    if (IPhi == 0) {
                        state.dataSolarShading->SurfWithShdgHoriz(SurfNum) += Fac1WithShdg;
                        state.dataSolarShading->SurfWoShdgHoriz(SurfNum) += Fac1WoShdg;
                    }
                } // End of surface loop
            } // End of Theta loop
        } // End of Phi loop

        for (int SurfNum : s_surf->AllExtSolAndShadingSurfaceList) {
            // Original conditions:
            // if (!s_surf->Surface(SurfNum).IsShadowing &&
            //    (!s_surf->Surface(SurfNum).HeatTransSurf || !s_surf->Surface(SurfNum).ExtSolar))
            //    continue;

            if (std::abs(state.dataSolarShading->SurfWoShdgIsoSky(SurfNum)) > Eps) {
                state.dataSolarShading->SurfDifShdgRatioIsoSkyHRTS(iTimeStep, iHour, SurfNum) =
                    (state.dataSolarShading->SurfWithShdgIsoSky(SurfNum)) / (state.dataSolarShading->SurfWoShdgIsoSky(SurfNum));
            } else {
                state.dataSolarShading->SurfDifShdgRatioIsoSkyHRTS(iTimeStep, iHour, SurfNum) =
                    (state.dataSolarShading->SurfWithShdgIsoSky(SurfNum)) / (state.dataSolarShading->SurfWoShdgIsoSky(SurfNum) + Eps);
            }
            if (std::abs(state.dataSolarShading->SurfWoShdgHoriz(SurfNum)) > Eps) {
                state.dataSolarShading->SurfDifShdgRatioHorizHRTS(iTimeStep, iHour, SurfNum) =
                    (state.dataSolarShading->SurfWithShdgHoriz(SurfNum)) / (state.dataSolarShading->SurfWoShdgHoriz(SurfNum));
            } else {
                state.dataSolarShading->SurfDifShdgRatioHorizHRTS(iTimeStep, iHour, SurfNum) =
                    (state.dataSolarShading->SurfWithShdgHoriz(SurfNum)) / (state.dataSolarShading->SurfWoShdgHoriz(SurfNum) + Eps);
            }
        }

        //  ! Get IR view factors. An exterior surface can receive IR radiation from
        //  ! sky, ground or shadowing surfaces. Assume shadowing surfaces have same
        //  ! temperature as outside air (and therefore same temperature as ground),
        //  ! so that the view factor to these shadowing surfaces can be included in
        //  ! the ground view factor. Sky IR is assumed to be isotropic and shadowing
        //  ! surfaces are assumed to be opaque to IR so they totally "shade" IR from
        //  ! sky or ground.

        //  DO SurfNum = 1,TotSurfaces
        //    Surface(SurfNum)%ViewFactorSkyIR = Surface(SurfNum)%ViewFactorSkyIR * DifShdgRatioIsoSky(SurfNum,IHOUR,TS)
        //    Surface(SurfNum)%ViewFactorGroundIR = 1.0 - Surface(SurfNum)%ViewFactorSkyIR
        //  END DO

    } // test for shading surfaces

    for (int SurfNum : s_surf->AllExtSolWinWithFrameSurfaceList) {
        // For exterior windows with frame/divider that are partially or fully sunlit,
        // correct SunlitFrac due to shadowing of frame and divider projections onto window glass.
        // Note: if SunlitFrac = 0.0 the window is either completely shaded or the sun is in back
        // of the window; in either case, frame/divider shadowing doesn't have to be done.

        if (state.dataHeatBal->SurfSunlitFrac(iHour, iTimeStep, SurfNum) > 0.0) {
            CalcFrameDividerShadow(state, SurfNum, s_surf->Surface(SurfNum).FrameDivider, iHour);
        }
    }
}

void DetermineShadowingCombinations(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         From Legacy Code
    //       DATE WRITTEN
    //       MODIFIED       LKL; March 2002 -- another missing translation from BLAST's routine
    //                      FCW; Jan 2003 -- removed line that prevented beam solar through interior windows
    //       RE-ENGINEERED  Rick Strand; 1998
    //                      Linda Lawrie; Oct 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This routine prepares a list of heat transfer surfaces and
    // their possible shadowers which is used to direct the hourly
    // calculation of shadows and sunlit areas.

    // METHODOLOGY EMPLOYED:
    // As appropriate surfaces are identified, they are placed into the
    // ShadowComb data structure (module level) with the accompanying lists
    // of other surface numbers.

    // REFERENCES:
    // BLAST/IBLAST code, original author George Walton

    // Using/Aliasing
    using namespace DataErrorTracking;

    Array1D_int GSS;             // List of shadowing surfaces numbers for a receiving surface
    Array1D_int BKS;             // List of back surface numbers for a receiving surface
    Array1D_int SBS;             // List of subsurfaces for a receiving surface
    bool CannotShade;            // TRUE if subsurface cannot shade receiving surface
    bool HasWindow;              // TRUE if a window is present on receiving surface
    Real64 ZMIN;                 // Lowest point on the receiving surface
    int BackSurfaceNumber;       // Back surface number
    int HTS;                     // Heat transfer surface number for a receiving surface
    int GRSNR;                   // Receiving surface number
    int GSSNR;                   // Shadowing surface number
    int SBSNR;                   // Subsurface number
    int NBKS;                    // Number of back surfaces for a receiving surface
    int NGSS;                    // Number of shadowing surfaces for a receiving surface
    int NSBS;                    // Number of subsurfaces for a receiving surface
    bool ShadowingSurf;          // True if a receiving surface is a shadowing surface
    Array1D_bool CastingSurface; // tracking during setup of ShadowComb

    auto &s_surf = state.dataSurface;

#ifdef EP_Count_Calls
    ++state.dataTimingsData->NumDetShadowCombs_Calls;
#endif

    state.dataShadowComb->ShadowComb.dimension(s_surf->TotSurfaces, ShadowingCombinations{}); // Set all elements to default constructed state

    CastingSurface.dimension(s_surf->TotSurfaces, false);

    state.dataSolarShading->HCA.dimension(2 * state.dataSolarShading->MaxHCS, state.dataSolarShading->MaxHCV + 1, 0);
    state.dataSolarShading->HCB.dimension(2 * state.dataSolarShading->MaxHCS, state.dataSolarShading->MaxHCV + 1, 0);
    state.dataSolarShading->HCC.dimension(2 * state.dataSolarShading->MaxHCS, state.dataSolarShading->MaxHCV + 1, 0);
    state.dataSolarShading->HCX.dimension(2 * state.dataSolarShading->MaxHCS, state.dataSolarShading->MaxHCV + 1, 0);
    state.dataSolarShading->HCY.dimension(2 * state.dataSolarShading->MaxHCS, state.dataSolarShading->MaxHCV + 1, 0);
    state.dataSolarShading->HCAREA.dimension(2 * state.dataSolarShading->MaxHCS, 0.0);
    state.dataSolarShading->HCNS.dimension(2 * state.dataSolarShading->MaxHCS, 0);
    state.dataSolarShading->HCNV.dimension(2 * state.dataSolarShading->MaxHCS, 0);
    state.dataSolarShading->HCT.dimension(2 * state.dataSolarShading->MaxHCS, 0.0);

    GSS.dimension(state.dataSolarShading->MaxGSS, 0);
    BKS.dimension(state.dataSolarShading->MaxGSS, 0);
    SBS.dimension(state.dataSolarShading->MaxGSS, 0);

    state.dataSolarShading->penumbraIDs.clear();

    HTS = 0;

    // Check every surface as a possible shadow receiving surface ("RS" = receiving surface).
    if (state.dataEnvrn->IgnoreSolarRadiation) {
        return;
    }

    for (GRSNR = 1; GRSNR <= s_surf->TotSurfaces; ++GRSNR) { // Loop through all surfaces (looking for potential receiving surfaces)...

        ShadowingSurf = s_surf->Surface(GRSNR).IsShadowing;
        NGSS = 0;
        NSBS = 0;
        NBKS = 0;

        if (!ShadowingSurf && !s_surf->Surface(GRSNR).HeatTransSurf) {
            continue;
        }
        HTS = GRSNR;

#ifndef EP_NO_OPENGL
        if (state.dataSolarShading->penumbra) {
            bool skipSurface = s_surf->Surface(GRSNR).MirroredSurf;
            // Penumbra doesn't need mirrored surfaces TODO: Don't bother creating them in the first place?

            // Skip interior surfaces if the other side has already been added to penumbra
            if (s_surf->Surface(GRSNR).ExtBoundCond > 0) {
                if (s_surf->SurfPenumbraID(s_surf->Surface(GRSNR).ExtBoundCond) >= 0) {
                    s_surf->SurfPenumbraID(GRSNR) = s_surf->SurfPenumbraID(s_surf->Surface(GRSNR).ExtBoundCond);
                    skipSurface = true;
                }
            }

            if (!skipSurface) {
                // Add surfaces to penumbra...
                Penumbra::Polygon poly;

                if (s_surf->Surface(GRSNR).Reveal > 0.0) {
                    Real64 R = s_surf->Surface(GRSNR).Reveal;
                    auto &norm = s_surf->Surface(GRSNR).NewellSurfaceNormalVector;
                    auto &v = s_surf->Surface(GRSNR).Vertex;
                    for (unsigned i = 0; i < v.size(); ++i) {
                        poly.push_back(v[i].x);
                        poly.push_back(v[i].y);
                        poly.push_back(v[i].z);

                        Vector vPrev;
                        if (i == 0) {
                            vPrev = v[v.size() - 1];
                        } else {
                            vPrev = v[i - 1];
                        }

                        Penumbra::Polygon rPoly; // Reveal surface
                        rPoly.push_back(v[i].x);
                        rPoly.push_back(v[i].y);
                        rPoly.push_back(v[i].z);

                        rPoly.push_back(v[i].x + norm.x * R);
                        rPoly.push_back(v[i].y + norm.y * R);
                        rPoly.push_back(v[i].z + norm.z * R);

                        rPoly.push_back(vPrev.x + norm.x * R);
                        rPoly.push_back(vPrev.y + norm.y * R);
                        rPoly.push_back(vPrev.z + norm.z * R);

                        rPoly.push_back(vPrev.x);
                        rPoly.push_back(vPrev.y);
                        rPoly.push_back(vPrev.z);

                        Penumbra::Surface rSurf(rPoly, fmt::format("{} reveal {}", s_surf->Surface(GRSNR).Name, i));
                        state.dataSolarShading->penumbra->add_surface(rSurf);
                    }
                } else {
                    for (auto const &v : s_surf->Surface(GRSNR).Vertex) {
                        poly.push_back(v.x);
                        poly.push_back(v.y);
                        poly.push_back(v.z);
                    }
                }
                Penumbra::Surface pSurf(poly, s_surf->Surface(GRSNR).Name);

                // Punch holes for subsurfaces
                if (s_surf->Surface(GRSNR).BaseSurf == GRSNR) { // Only look for subsurfaces on base surfaces
                    for (int subSurface = 1; subSurface <= s_surf->TotSurfaces; ++subSurface) {
                        if (s_surf->Surface(subSurface).BaseSurf != GRSNR) {
                            continue; // Ignore subsurfaces of other surfaces
                        }
                        if (!s_surf->Surface(subSurface).HeatTransSurf) {
                            continue; // Skip non heat transfer subsurfaces
                        }
                        if (subSurface == GRSNR) {
                            continue; // Surface itself cannot be its own subsurface
                        }

                        Penumbra::Polygon subPoly;
                        if (s_surf->Surface(subSurface).Reveal > 0.0) {
                            Real64 R = s_surf->Surface(subSurface).Reveal;
                            auto &norm = s_surf->Surface(subSurface).NewellSurfaceNormalVector;
                            for (auto const &v : s_surf->Surface(subSurface).Vertex) {
                                subPoly.push_back(v.x + norm.x * R);
                                subPoly.push_back(v.y + norm.y * R);
                                subPoly.push_back(v.z + norm.z * R);
                            }
                        } else {
                            for (auto const &v : s_surf->Surface(subSurface).Vertex) {
                                subPoly.push_back(v.x);
                                subPoly.push_back(v.y);
                                subPoly.push_back(v.z);
                            }
                        }

                        pSurf.add_hole(subPoly);
                    }
                }
                s_surf->SurfPenumbraID(GRSNR) = state.dataSolarShading->penumbra->add_surface(pSurf);
                state.dataSolarShading->penumbraIDs.push_back(s_surf->SurfPenumbraID(GRSNR));
            }
        }
#endif

        if (!ShadowingSurf && !s_surf->Surface(GRSNR).ExtSolar) {
            continue; // Skip surfaces with no external solar
        }

        if (!ShadowingSurf && s_surf->Surface(GRSNR).BaseSurf != GRSNR) { // Skip subsurfaces (SBS)
            continue;
        }

        // Get the lowest point of receiving surface
        ZMIN = minval(s_surf->Surface(GRSNR).Vertex, &Vector::z);

        // Check every surface as a possible shadow casting surface ("SS" = shadow sending)
        NGSS = 0;
        if (state.dataHeatBal->SolarDistribution != DataHeatBalance::Shadowing::Minimal) { // Except when doing simplified exterior shadowing.

            for (GSSNR = 1; GSSNR <= s_surf->TotSurfaces; ++GSSNR) { // Loop through all surfaces, looking for ones that could shade GRSNR

                if (GSSNR == GRSNR) {
                    continue; // Receiving surface cannot shade itself
                }
                if ((s_surf->Surface(GSSNR).HeatTransSurf) && (s_surf->Surface(GSSNR).BaseSurf == GRSNR)) {
                    continue; // A heat transfer subsurface of a receiving surface
                }
                // cannot shade the receiving surface
                if (ShadowingSurf) {
                    // If receiving surf is a shadowing surface exclude matching shadow surface as sending surface
                    // IF((GSSNR == GRSNR+1 .AND. Surface(GSSNR)%Name(1:3) == 'Mir').OR. &
                    //   (GSSNR == GRSNR-1 .AND. Surface(GRSNR)%Name(1:3) == 'Mir')) CYCLE
                    if (((GSSNR == GRSNR + 1) && s_surf->Surface(GSSNR).MirroredSurf) ||
                        ((GSSNR == GRSNR - 1) && s_surf->Surface(GRSNR).MirroredSurf)) {
                        continue;
                    }
                }

                if (s_surf->Surface(GSSNR).BaseSurf == GRSNR) { // Shadowing subsurface of receiving surface

                    ++NGSS;
                    if (NGSS > state.dataSolarShading->MaxGSS) {
                        GSS.redimension(state.dataSolarShading->MaxGSS *= 2, 0);
                    }
                    GSS(NGSS) = GSSNR;

                } else if ((s_surf->Surface(GSSNR).BaseSurf == 0) ||
                           ((s_surf->Surface(GSSNR).BaseSurf == GSSNR) &&
                            ((s_surf->Surface(GSSNR).ExtBoundCond == ExternalEnvironment) ||
                             s_surf->Surface(GSSNR).ExtBoundCond ==
                                 OtherSideCondModeledExt))) { // Detached shadowing surface or | any other base surface
                    // exposed to outside environment

                    CHKGSS(state, GRSNR, GSSNR, ZMIN, CannotShade); // Check to see if this can shade the receiving surface
                    if (!CannotShade) {                             // Update the shadowing surface data if shading is possible
                        ++NGSS;
                        if (NGSS > state.dataSolarShading->MaxGSS) {
                            GSS.redimension(state.dataSolarShading->MaxGSS *= 2, 0);
                        }
                        GSS(NGSS) = GSSNR;
                    }
                }

            } // ...end of surfaces DO loop (GSSNR)
        } else { // Simplified Distribution -- still check for Shading Subsurfaces

            for (GSSNR = 1; GSSNR <= s_surf->TotSurfaces; ++GSSNR) { // Loop through all surfaces (looking for surfaces which could shade GRSNR) ...

                if (GSSNR == GRSNR) {
                    continue; // Receiving surface cannot shade itself
                }
                if ((s_surf->Surface(GSSNR).HeatTransSurf) && (s_surf->Surface(GSSNR).BaseSurf == GRSNR)) {
                    continue; // Skip heat transfer subsurfaces of receiving surface
                }
                if (s_surf->Surface(GSSNR).BaseSurf == GRSNR) { // Shadowing subsurface of receiving surface
                    ++NGSS;
                    if (NGSS > state.dataSolarShading->MaxGSS) {
                        GSS.redimension(state.dataSolarShading->MaxGSS *= 2, 0);
                    }
                    GSS(NGSS) = GSSNR;
                }
            }

        } // ...end of check for simplified solar distribution

        // Check every surface as a receiving subsurface of the receiving surface
        NSBS = 0;
        HasWindow = false;
        // legacy: IF (OSENV(HTS) > 10) WINDOW=.TRUE. -->Note: WINDOW was set true for roof ponds, solar walls, or other zones
        for (SBSNR = 1; SBSNR <= s_surf->TotSurfaces; ++SBSNR) { // Loop through the surfaces yet again (looking for subsurfaces of GRSNR)...

            if (!s_surf->Surface(SBSNR).HeatTransSurf) {
                continue; // Skip non heat transfer subsurfaces
            }
            if (SBSNR == GRSNR) {
                continue; // Surface itself cannot be its own subsurface
            }
            if (s_surf->Surface(SBSNR).BaseSurf != GRSNR) {
                continue; // Ignore subsurfaces of other surfaces and other surfaces
            }

            if (state.dataConstruction->Construct(s_surf->Surface(SBSNR).Construction).TransDiff > 0.0) {
                HasWindow = true; // Check for window
            }
            CHKSBS(state, HTS, GRSNR, SBSNR); // Check that the receiving surface completely encloses the subsurface;
            // severe error if not
            ++NSBS;
            if (NSBS > state.dataSolarShading->MaxSBS) {
                SBS.redimension(state.dataSolarShading->MaxSBS *= 2, 0);
            }
            SBS(NSBS) = SBSNR;

        } // ...end of surfaces DO loop (SBSNR)

        // Check every surface as a back surface
        NBKS = 0;
        //                                        Except for simplified
        //                                        interior solar distribution,
        if ((state.dataHeatBal->SolarDistribution == DataHeatBalance::Shadowing::FullInteriorExterior) &&
            (HasWindow)) { // For full interior solar distribution | and a window present on base surface (GRSNR)

            for (BackSurfaceNumber = 1; BackSurfaceNumber <= s_surf->TotSurfaces;
                 ++BackSurfaceNumber) { // Loop through surfaces yet again, looking for back surfaces to GRSNR

                if (!s_surf->Surface(BackSurfaceNumber).HeatTransSurf) {
                    continue; // Skip non-heat transfer surfaces
                }
                if (s_surf->Surface(BackSurfaceNumber).BaseSurf == GRSNR) {
                    continue; // Skip subsurfaces of this GRSNR
                }
                if (BackSurfaceNumber == GRSNR) {
                    continue; // A back surface cannot be GRSNR itself
                }
                if (s_surf->Surface(BackSurfaceNumber).SolarEnclIndex != s_surf->Surface(GRSNR).SolarEnclIndex) {
                    continue; // Skip if back surface not in same solar enclosure
                }

                if (s_surf->Surface(BackSurfaceNumber).Class == SurfaceClass::IntMass) {
                    continue;
                }

                // Following line removed 1/27/03 by FCW. Was in original code that didn't do beam solar transmitted through
                // interior windows. Was removed to allow such beam solar but then somehow was put back in.
                // IF (Surface(BackSurfaceNumber)%BaseSurf /= BackSurfaceNumber) CYCLE ! Not for subsurfaces of Back Surface

                if (!state.dataSolarShading->penumbra) {
                    CHKBKS(state, BackSurfaceNumber, GRSNR); // CHECK FOR CONVEX ZONE; severe error if not
                }
                ++NBKS;
                if (NBKS > state.dataSolarShading->MaxBKS) {
                    BKS.redimension(state.dataSolarShading->MaxBKS *= 2, 0);
                }
                BKS(NBKS) = BackSurfaceNumber;

            } // ...end of surfaces DO loop (BackSurfaceNumber)
        }

        // Put this into the ShadowComb data structure
        state.dataShadowComb->ShadowComb(GRSNR).UseThisSurf = true;
        state.dataShadowComb->ShadowComb(GRSNR).NumGenSurf = NGSS;
        state.dataShadowComb->ShadowComb(GRSNR).NumBackSurf = NBKS;
        state.dataShadowComb->ShadowComb(GRSNR).NumSubSurf = NSBS;
        state.dataSolarShading->MaxDim = max(state.dataSolarShading->MaxDim, NGSS, NBKS, NSBS);

        state.dataShadowComb->ShadowComb(GRSNR).GenSurf.allocate({0, state.dataShadowComb->ShadowComb(GRSNR).NumGenSurf});
        state.dataShadowComb->ShadowComb(GRSNR).GenSurf(0) = 0;
        if (state.dataShadowComb->ShadowComb(GRSNR).NumGenSurf > 0) {
            state.dataShadowComb->ShadowComb(GRSNR).GenSurf({1, state.dataShadowComb->ShadowComb(GRSNR).NumGenSurf}) = GSS({1, NGSS});
        }

        state.dataShadowComb->ShadowComb(GRSNR).BackSurf.allocate({0, state.dataShadowComb->ShadowComb(GRSNR).NumBackSurf});
        state.dataShadowComb->ShadowComb(GRSNR).BackSurf(0) = 0;
        if (state.dataShadowComb->ShadowComb(GRSNR).NumBackSurf > 0) {
            state.dataShadowComb->ShadowComb(GRSNR).BackSurf({1, state.dataShadowComb->ShadowComb(GRSNR).NumBackSurf}) = BKS({1, NBKS});
        }

        state.dataShadowComb->ShadowComb(GRSNR).SubSurf.allocate({0, state.dataShadowComb->ShadowComb(GRSNR).NumSubSurf});
        state.dataShadowComb->ShadowComb(GRSNR).SubSurf(0) = 0;
        if (state.dataShadowComb->ShadowComb(GRSNR).NumSubSurf > 0) {
            state.dataShadowComb->ShadowComb(GRSNR).SubSurf({1, state.dataShadowComb->ShadowComb(GRSNR).NumSubSurf}) = SBS({1, NSBS});
        }

    } // ...end of surfaces (GRSNR) DO loop

    GSS.deallocate();
    SBS.deallocate();
    BKS.deallocate();

    if (!state.dataSolarShading->penumbra) {
        if (state.dataSolarShading->shd_stream) {
            *state.dataSolarShading->shd_stream << "Shadowing Combinations\n";
            if (state.dataHeatBal->SolarDistribution == DataHeatBalance::Shadowing::Minimal) {
                *state.dataSolarShading->shd_stream
                    << "..Solar Distribution=Minimal Shadowing, Detached Shading will not be used in shadowing calculations\n";
            } else if (state.dataHeatBal->SolarDistribution == DataHeatBalance::Shadowing::FullExterior) {
                if (s_surf->CalcSolRefl) {
                    *state.dataSolarShading->shd_stream << "..Solar Distribution=FullExteriorWithReflectionsFromExteriorSurfaces\n";
                } else {
                    *state.dataSolarShading->shd_stream << "..Solar Distribution=FullExterior\n";
                }
            } else if (state.dataHeatBal->SolarDistribution == DataHeatBalance::Shadowing::FullInteriorExterior) {
                if (s_surf->CalcSolRefl) {
                    *state.dataSolarShading->shd_stream << "..Solar Distribution=FullInteriorAndExteriorWithReflectionsFromExteriorSurfaces\n";
                } else {
                    *state.dataSolarShading->shd_stream << "..Solar Distribution=FullInteriorAndExterior\n";
                }
            } else {
            }

            *state.dataSolarShading->shd_stream << "..In the following, only the first 10 reference surfaces will be shown.\n";
            *state.dataSolarShading->shd_stream << "..But all surfaces are used in the calculations.\n";

            for (int HTSnum : s_surf->AllSurfaceListReportOrder) {
                *state.dataSolarShading->shd_stream << "==================================\n";
                if (state.dataShadowComb->ShadowComb(HTSnum).UseThisSurf) {
                    if (s_surf->Surface(HTSnum).IsConvex) {
                        *state.dataSolarShading->shd_stream << "Surface=" << s_surf->Surface(HTSnum).Name
                                                            << " is used as Receiving Surface in calculations and is convex.\n";
                    } else {
                        *state.dataSolarShading->shd_stream << "Surface=" << s_surf->Surface(HTSnum).Name
                                                            << " is used as Receiving Surface in calculations and is non-convex.\n";
                        if (state.dataShadowComb->ShadowComb(HTSnum).NumGenSurf > 0) {
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                ShowWarningError(state,
                                                 format("DetermineShadowingCombinations: Surface=\"{}\" is a receiving surface and is non-convex.",
                                                        s_surf->Surface(HTSnum).Name));
                                ShowContinueError(state,
                                                  "...Shadowing values may be inaccurate. Check .shd report file for more surface shading details");
                            } else {
                                ++state.dataErrTracking->TotalReceivingNonConvexSurfaces;
                            }
                        }
                    }
                } else {
                    *state.dataSolarShading->shd_stream << "Surface=" << s_surf->Surface(HTSnum).Name
                                                        << " is not used as Receiving Surface in calculations.\n";
                }
                *state.dataSolarShading->shd_stream << "Number of general casting surfaces=" << state.dataShadowComb->ShadowComb(HTSnum).NumGenSurf
                                                    << '\n';
                for (NGSS = 1; NGSS <= state.dataShadowComb->ShadowComb(HTSnum).NumGenSurf; ++NGSS) {
                    if (NGSS <= 10) {
                        *state.dataSolarShading->shd_stream
                            << "..Surface=" << s_surf->Surface(state.dataShadowComb->ShadowComb(HTSnum).GenSurf(NGSS)).Name << '\n';
                    }
                    CastingSurface(state.dataShadowComb->ShadowComb(HTSnum).GenSurf(NGSS)) = true;
                }
                *state.dataSolarShading->shd_stream << "Number of back surfaces=" << state.dataShadowComb->ShadowComb(HTSnum).NumBackSurf << '\n';
                for (NGSS = 1; NGSS <= min(10, state.dataShadowComb->ShadowComb(HTSnum).NumBackSurf); ++NGSS) {
                    *state.dataSolarShading->shd_stream
                        << "...Surface=" << s_surf->Surface(state.dataShadowComb->ShadowComb(HTSnum).BackSurf(NGSS)).Name << '\n';
                }
                *state.dataSolarShading->shd_stream << "Number of receiving sub surfaces=" << state.dataShadowComb->ShadowComb(HTSnum).NumSubSurf
                                                    << '\n';
                for (NGSS = 1; NGSS <= min(10, state.dataShadowComb->ShadowComb(HTSnum).NumSubSurf); ++NGSS) {
                    *state.dataSolarShading->shd_stream
                        << "....Surface=" << s_surf->Surface(state.dataShadowComb->ShadowComb(HTSnum).SubSurf(NGSS)).Name << '\n';
                }
            }
        }

        for (HTS = 1; HTS <= s_surf->TotSurfaces; ++HTS) {
            if (CastingSurface(HTS) && !s_surf->Surface(HTS).IsConvex) {
                if (state.dataGlobal->DisplayExtraWarnings) {
                    ShowSevereError(
                        state,
                        format("DetermineShadowingCombinations: Surface=\"{}\" is a casting surface and is non-convex.", s_surf->Surface(HTS).Name));
                    ShowContinueError(state, "...Shadowing values may be inaccurate. Check .shd report file for more surface shading details");
                } else {
                    ++state.dataErrTracking->TotalCastingNonConvexSurfaces;
                }
            }
        }

        if (state.dataErrTracking->TotalReceivingNonConvexSurfaces > 0) {
            ShowWarningMessage(state,
                               format("DetermineShadowingCombinations: There are {} surfaces which are receiving surfaces and are non-convex.",
                                      state.dataErrTracking->TotalReceivingNonConvexSurfaces));
            ShowContinueError(state, "...Shadowing values may be inaccurate. Check .shd report file for more surface shading details");
            ShowContinueError(state, "...Add Output:Diagnostics,DisplayExtraWarnings; to see individual warnings for each surface.");
            state.dataErrTracking->TotalWarningErrors += state.dataErrTracking->TotalReceivingNonConvexSurfaces;
        }

        if (state.dataErrTracking->TotalCastingNonConvexSurfaces > 0) {
            ShowSevereMessage(state,
                              format("DetermineShadowingCombinations: There are {} surfaces which are casting surfaces and are non-convex.",
                                     state.dataErrTracking->TotalCastingNonConvexSurfaces));
            ShowContinueError(state, "...Shadowing values may be inaccurate. Check .shd report file for more surface shading details");
            ShowContinueError(state, "...Add Output:Diagnostics,DisplayExtraWarnings; to see individual severes for each surface.");
            state.dataErrTracking->TotalSevereErrors += state.dataErrTracking->TotalCastingNonConvexSurfaces;
        }
    }

    CastingSurface.deallocate();

#ifndef EP_NO_OPENGL
    if (state.dataSolarShading->penumbra && state.dataSolarShading->penumbra->get_number_of_surfaces() > 0) {
        state.dataSolarShading->penumbra->set_model();
    }
#endif
}

void SHADOW(EnergyPlusData &state,
            int const iHour, // Hour index
            int const TS     // Time Step
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy Code
    //       DATE WRITTEN
    //       MODIFIED       Nov 2003, FCW: modify to do shadowing on shadowing surfaces
    //       RE-ENGINEERED  Lawrie, Oct 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is a driving routine for calculations of shadows
    // and sunlit areas used in computing the solar beam flux multipliers.

    // REFERENCES:
    // BLAST/IBLAST code, original author George Walton

    Real64 XS; // Intermediate result
    Real64 YS; // Intermediate result
    Real64 ZS; // Intermediate result
    int N;     // Vertex number
    int NGRS;  // Coordinate transformation index
    int NVT;
    int HTS;         // Heat transfer surface number of the general receiving surface
    int GRSNR;       // Surface number of general receiving surface
    int NBKS;        // Number of back surfaces
    int NGSS;        // Number of general shadowing surfaces
    int NSBS;        // Number of subsurfaces (windows and doors)
    Real64 SurfArea; // Surface area. For walls, includes all window frame areas.
    // For windows, includes divider area

    auto &s_surf = state.dataSurface;

    if (state.dataSolarShading->ShadowOneTimeFlag) {
        state.dataSolarShading->XVrt.allocate(s_surf->MaxVerticesPerSurface + 1);
        state.dataSolarShading->YVrt.allocate(s_surf->MaxVerticesPerSurface + 1);
        state.dataSolarShading->ZVrt.allocate(s_surf->MaxVerticesPerSurface + 1);
        state.dataSolarShading->XVrt = 0.0;
        state.dataSolarShading->YVrt = 0.0;
        state.dataSolarShading->ZVrt = 0.0;
        state.dataSolarShading->ShadowOneTimeFlag = false;
    }

#ifdef EP_Count_Calls
    if (iHour == 0) {
        ++state.dataTimingsData->NumShadow_Calls;
    } else {
        ++state.dataTimingsData->NumShadowAtTS_Calls;
    }
#endif

    state.dataSolarShading->SurfSunlitArea = 0.0;

#ifndef EP_NO_OPENGL
    if (state.dataSolarShading->penumbra) {
        Real64 ElevSun = Constant::PiOvr2 - std::acos(state.dataSolarShading->SUNCOS(3));
        Real64 AzimSun = std::atan2(state.dataSolarShading->SUNCOS(1), state.dataSolarShading->SUNCOS(2));
        state.dataSolarShading->penumbra->set_sun_position(AzimSun, ElevSun);
        state.dataSolarShading->penumbra->submit_pssa();
    }
#endif

    for (GRSNR = 1; GRSNR <= s_surf->TotSurfaces; ++GRSNR) {

        if (!state.dataShadowComb->ShadowComb(GRSNR).UseThisSurf) {
            continue;
        }

        state.dataSolarShading->SurfSunlitArea(GRSNR) = 0.0;

        NGSS = state.dataShadowComb->ShadowComb(GRSNR).NumGenSurf;
        state.dataSolarShading->NGSSHC = 0;
        NBKS = state.dataShadowComb->ShadowComb(GRSNR).NumBackSurf;
        state.dataSolarShading->NBKSHC = 0;
        NSBS = state.dataShadowComb->ShadowComb(GRSNR).NumSubSurf;
        state.dataSolarShading->NRVLHC = 0;
        state.dataSolarShading->NSBSHC = 0;
        state.dataSolarShading->LOCHCA = 1;
        // Temporarily determine the old heat transfer surface number (HTS)
        HTS = GRSNR;

        if (state.dataSolarShading->SurfSunCosTheta(GRSNR) < DataEnvironment::SunIsUpValue) { //.001) THEN ! Receiving surface is not in the sun

            state.dataSolarShading->SurfSunlitArea(HTS) = 0.0;
            SHDSBS(state, iHour, GRSNR, NBKS, NSBS, HTS, TS);

        } else if ((NGSS <= 0) && (NSBS <= 0)) { // Simple surface--no shaders or subsurfaces

            state.dataSolarShading->SurfSunlitArea(HTS) = s_surf->Surface(GRSNR).NetAreaShadowCalc;
        } else { // Surface in sun and either shading surfaces or subsurfaces present (or both)

#ifndef EP_NO_OPENGL
            int id = s_surf->SurfPenumbraID(HTS);
            if (state.dataSolarShading->penumbra && id >= 0) {
                // SurfSunlitArea(HTS) = buildingPSSF.at(id) / SurfSunCosTheta(HTS);
                state.dataSolarShading->SurfSunlitArea(HTS) =
                    state.dataSolarShading->penumbra->retrieve_pssa(id) / state.dataSolarShading->SurfSunCosTheta(HTS);
                // SurfSunlitArea(HTS) = penumbra->fetchPSSA(Surface(HTS).PenumbraID)/SurfSunCosTheta(HTS);
                for (int SS = 1; SS <= NSBS; ++SS) {
                    int HTSS = state.dataShadowComb->ShadowComb(HTS).SubSurf(SS);
                    id = s_surf->SurfPenumbraID(HTSS);
                    if (id >= 0) {
                        // SurfSunlitArea(HTSS) = buildingPSSF.at(id) / SurfSunCosTheta(HTSS);
                        state.dataSolarShading->SurfSunlitArea(HTSS) =
                            state.dataSolarShading->penumbra->retrieve_pssa(id) / state.dataSolarShading->SurfSunCosTheta(HTSS);
                        // SurfSunlitArea(HTSS) = penumbra->fetchPSSA(Surface(HTSS).PenumbraID)/SurfSunCosTheta(HTSS);
                        if (state.dataSolarShading->SurfSunlitArea(HTSS) > 0.0) {
                            if (iHour > 0 && TS > 0) {
                                state.dataHeatBal->SurfSunlitFracWithoutReveal(iHour, TS, HTSS) =
                                    state.dataSolarShading->SurfSunlitArea(HTSS) / s_surf->Surface(HTSS).Area;
                            }
                        }
                    }
                }
            } else if (!state.dataSolarShading->penumbra) {
#else
            {
#endif
                NGRS = s_surf->Surface(GRSNR).BaseSurf;
                if (s_surf->Surface(GRSNR).IsShadowing) {
                    NGRS = GRSNR;
                }

                // Compute the X and Y displacements of a shadow.
                XS = s_surf->Surface(NGRS).lcsx.x * state.dataSolarShading->SUNCOS(1) +
                     s_surf->Surface(NGRS).lcsx.y * state.dataSolarShading->SUNCOS(2) +
                     s_surf->Surface(NGRS).lcsx.z * state.dataSolarShading->SUNCOS(3);
                YS = s_surf->Surface(NGRS).lcsy.x * state.dataSolarShading->SUNCOS(1) +
                     s_surf->Surface(NGRS).lcsy.y * state.dataSolarShading->SUNCOS(2) +
                     s_surf->Surface(NGRS).lcsy.z * state.dataSolarShading->SUNCOS(3);
                ZS = s_surf->Surface(NGRS).lcsz.x * state.dataSolarShading->SUNCOS(1) +
                     s_surf->Surface(NGRS).lcsz.y * state.dataSolarShading->SUNCOS(2) +
                     s_surf->Surface(NGRS).lcsz.z * state.dataSolarShading->SUNCOS(3);

                if (std::abs(ZS) > Constant::SmallDistance) {
                    state.dataSolarShading->XShadowProjection = XS / ZS;
                    state.dataSolarShading->YShadowProjection = YS / ZS;
                    if (std::abs(state.dataSolarShading->XShadowProjection) < 1.e-8) {
                        state.dataSolarShading->XShadowProjection = 0.0;
                    }
                    if (std::abs(state.dataSolarShading->YShadowProjection) < 1.e-8) {
                        state.dataSolarShading->YShadowProjection = 0.0;
                    }
                } else {
                    state.dataSolarShading->XShadowProjection = 0.0;
                    state.dataSolarShading->YShadowProjection = 0.0;
                }

                CTRANS(state,
                       GRSNR,
                       NGRS,
                       NVT,
                       state.dataSolarShading->XVrt,
                       state.dataSolarShading->YVrt,
                       state.dataSolarShading->ZVrt); // Transform coordinates of the receiving surface to 2-D form

                // Re-order its vertices to clockwise sequential.
                for (N = 1; N <= NVT; ++N) {
                    state.dataSolarShading->XVS(N) = state.dataSolarShading->XVrt(NVT + 1 - N);
                    state.dataSolarShading->YVS(N) = state.dataSolarShading->YVrt(NVT + 1 - N);
                }

                HTRANS1(state, 1, NVT); // Transform to homogeneous coordinates.

                state.dataSolarShading->HCAREA(1) = -state.dataSolarShading->HCAREA(1); // Compute (+) gross surface area.
                state.dataSolarShading->HCT(1) = 1.0;

                SHDGSS(state, NGRS, iHour, TS, GRSNR, NGSS, HTS); // Determine shadowing on surface.

                if (!state.dataSolarShading->CalcSkyDifShading) {
                    SHDBKS(state, s_surf->Surface(GRSNR).BaseSurf, GRSNR, NBKS, HTS); // Determine possible back surfaces.
                }
            }

            SHDSBS(state, iHour, GRSNR, NBKS, NSBS, HTS, TS); // Subtract subsurf areas from total

            // Error checking:  require that 0 <= SurfSunlitArea <= AREA.  + or - .01*AREA added for round-off errors
            SurfArea = s_surf->Surface(GRSNR).NetAreaShadowCalc;
            state.dataSolarShading->SurfSunlitArea(HTS) = max(0.0, state.dataSolarShading->SurfSunlitArea(HTS));

            state.dataSolarShading->SurfSunlitArea(HTS) = min(state.dataSolarShading->SurfSunlitArea(HTS), SurfArea);
        } // ...end of surface in sun/surface with shaders and/or subsurfaces IF-THEN block

        // NOTE:
        // There used to be a call to legacy subroutine SHDCVR here when the
        // zone type was not a standard zone.
    }
}

void SHDBKS(EnergyPlusData &state,
            int const NGRS, // Number of the general receiving surface
            int const CurSurf,
            int const NBKS, // Number of back surfaces
            int const HTS   // Heat transfer surface number of the general receiving surf
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy Code
    //       DATE WRITTEN
    //       MODIFIED       na
    //       RE-ENGINEERED  Lawrie, Oct 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This is the driving subroutine for computing
    // the sunlit areas for back surfaces.

    // REFERENCES:
    // BLAST/IBLAST code, original author George Walton

    typedef Array2D<Int64>::size_type size_type;
    int I;
    int M;
    int N;
    int NVR;
    int NVT; // Number of vertices of back surface
    int BackSurfaceNumber;
    int NS1; // Number of the figure being overlapped
    int NS2; // Number of the figure doing overlapping
    int NS3; // Location to place results of overlap

    auto &s_surf = state.dataSurface;

    // Tuned Linear indexing

    assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCY));
    assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCA));

    if (state.dataSolarShading->SHDBKSOneTimeFlag) {
        state.dataSolarShading->XVrtx.allocate(s_surf->MaxVerticesPerSurface + 1);
        state.dataSolarShading->YVrtx.allocate(s_surf->MaxVerticesPerSurface + 1);
        state.dataSolarShading->ZVrtx.allocate(s_surf->MaxVerticesPerSurface + 1);
        state.dataSolarShading->XVrtx = 0.0;
        state.dataSolarShading->YVrtx = 0.0;
        state.dataSolarShading->ZVrtx = 0.0;
        state.dataSolarShading->SHDBKSOneTimeFlag = false;
    }

    if ((NBKS <= 0) || (state.dataSolarShading->SurfSunlitArea(HTS) <= 0.0) || (state.dataSolarShading->OverlapStatus == TooManyVertices) ||
        (state.dataSolarShading->OverlapStatus == TooManyFigures)) {
        return;
    }

    state.dataSolarShading->FBKSHC = state.dataSolarShading->LOCHCA + 1;

    for (I = 1; I <= NBKS; ++I) { // Loop through all back surfaces associated with the receiving surface

        BackSurfaceNumber = state.dataShadowComb->ShadowComb(CurSurf).BackSurf(I);

        if (state.dataSolarShading->SurfSunCosTheta(BackSurfaceNumber) > -DataEnvironment::SunIsUpValue) {
            continue; //-0.001) CYCLE ! go to next back surface since inside of this surface
        }
        // cannot be in sun if the outside can be

        // Transform coordinates of back surface from general system to the
        // plane of the receiving surface

        CTRANS(state, BackSurfaceNumber, NGRS, NVT, state.dataSolarShading->XVrtx, state.dataSolarShading->YVrtx, state.dataSolarShading->ZVrtx);

        // Project "shadow" from back surface along sun's rays to receiving surface.  Back surface vertices
        // become clockwise sequential.

        for (N = 1; N <= NVT; ++N) {
            state.dataSolarShading->XVS(N) =
                state.dataSolarShading->XVrtx(N) - state.dataSolarShading->XShadowProjection * state.dataSolarShading->ZVrtx(N);
            state.dataSolarShading->YVS(N) =
                state.dataSolarShading->YVrtx(N) - state.dataSolarShading->YShadowProjection * state.dataSolarShading->ZVrtx(N);
        }

        // Transform to the homogeneous coordinate system.

        NS3 = state.dataSolarShading->LOCHCA + 1;
        state.dataSolarShading->HCT(NS3) = 0.0;
        HTRANS1(state, NS3, NVT);

        // Adjust near-duplicate points.

        NVR = state.dataSolarShading->HCNV(1);
        int l3 = state.dataSolarShading->HCX.index(NS3, 1);
        for (N = 1; N <= NVT; ++N, ++l3) {
            Real64 const x3 = state.dataSolarShading->HCX[l3]; // [ l3 ] == ( NS3, N )
            Real64 const y3 = state.dataSolarShading->HCY[l3];
            size_type l1 = 0;
            for (M = 1; M <= NVR; ++M, ++l1) {
                if (std::abs(state.dataSolarShading->HCX[l1] - x3) > 6) {
                    continue; // [ l1 ] == ( 1, M )
                }
                if (std::abs(state.dataSolarShading->HCY[l1] - y3) > 6) {
                    continue;
                }
                state.dataSolarShading->HCX[l3] = state.dataSolarShading->HCX[l1];
                state.dataSolarShading->HCY[l3] = state.dataSolarShading->HCY[l1];
                break;
            }
        }

        HTRANS0(state, NS3, NVT);

        // Determine area of overlap of projected back surface and receiving surface.

        NS1 = 1;
        NS2 = NS3;
        state.dataSolarShading->HCT(NS3) = 1.0;
        DeterminePolygonOverlap(state, NS1, NS2, NS3);

        if (state.dataSolarShading->OverlapStatus == NoOverlap) {
            continue; // to next back surface
        }
        if ((state.dataSolarShading->OverlapStatus == TooManyVertices) || (state.dataSolarShading->OverlapStatus == TooManyFigures)) {
            break; // back surfaces DO loop
        }

        // Increment back surface count.

        state.dataSolarShading->LOCHCA = NS3;
        state.dataSolarShading->HCNS(state.dataSolarShading->LOCHCA) = BackSurfaceNumber;
        state.dataSolarShading->HCAREA(state.dataSolarShading->LOCHCA) = -state.dataSolarShading->HCAREA(state.dataSolarShading->LOCHCA);
        state.dataSolarShading->NBKSHC = state.dataSolarShading->LOCHCA - state.dataSolarShading->FBKSHC + 1;
    }
}

void SHDGSS(EnergyPlusData &state,
            int const NGRS,
            int const iHour,   // Hour Counter
            int const TS,      // TimeStep
            int const CurSurf, // Current Surface
            int const NGSS,    // Number of general shadowing surfaces
            int const HTS      // Heat transfer surface number of the general receiving surf
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy Code
    //       DATE WRITTEN
    //       MODIFIED       na
    //       RE-ENGINEERED  Lawrie, Oct 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine determines the shadows on a general receiving surface.

    // REFERENCES:
    // BLAST/IBLAST code, original author George Walton

    typedef Array2D<Int64>::size_type size_type;
    int GSSNR;             // General shadowing surface number
    int MainOverlapStatus; // Overlap status of the main overlap calculation not the check for
    // multiple overlaps (unless there was an error)
    int NS1;         // Number of the figure being overlapped
    int NS2;         // Number of the figure doing overlapping
    int NS3;         // Location to place results of overlap
    Real64 SchValue; // Value for Schedule of shading transmittance

    auto &s_surf = state.dataSurface;

    if (state.dataSolarShading->SHDGSSOneTimeFlag) {
        state.dataSolarShading->XVert.dimension(s_surf->MaxVerticesPerSurface + 1, 0.0);
        state.dataSolarShading->YVert.dimension(s_surf->MaxVerticesPerSurface + 1, 0.0);
        state.dataSolarShading->ZVert.dimension(s_surf->MaxVerticesPerSurface + 1, 0.0);
        state.dataSolarShading->SHDGSSOneTimeFlag = false;
    }

    state.dataSolarShading->FGSSHC = state.dataSolarShading->LOCHCA + 1;
    MainOverlapStatus = NoOverlap; // Set to ensure that the value from the last surface is not saved
    state.dataSolarShading->OverlapStatus = NoOverlap;

    if (NGSS <= 0) { // IF NO S.S., receiving surface FULLY SUNLIT.

        state.dataSolarShading->SurfSunlitArea(HTS) = state.dataSolarShading->HCAREA(1); // Surface fully sunlit

    } else {

        int ExitLoopStatus = -1;
        auto const &GenSurf = state.dataShadowComb->ShadowComb(CurSurf).GenSurf;
        int const sunIsUp = DataEnvironment::SunIsUpValue;
        for (int I = 1; I <= NGSS; ++I) { // Loop through all shadowing surfaces...

            GSSNR = GenSurf(I);

            if (state.dataSolarShading->SurfSunCosTheta(GSSNR) > sunIsUp) {
                continue; //.001) CYCLE ! NO SHADOW IF GSS IN SUNLIGHT.
            }

            auto const &surface = s_surf->Surface(GSSNR);
            bool const notHeatTransSurf = !surface.HeatTransSurf;

            //     This used to check to see if the shadowing surface was not opaque (within the scheduled dates of
            //            transmittance value.  Perhaps it ignored it if it were outside the range.  (if so, was an error)
            //     The proper action seems to be delete this statement all together, but there would also be no shading if
            //            the shading surface were transparent...
            //---former stmt      IF ((.NOT.Surface(GSSNR)%HeatTransSurf) .AND. &
            //---former stmt            GetCurrentScheduleValue(state, Surface(GSSNR)%SchedShadowSurfIndex,IHOUR) == 0.0) CYCLE

            if (notHeatTransSurf) {
                if (surface.IsTransparent) {
                    continue; // No shadow if shading surface is transparent
                }
                if (surface.shadowSurfSched != nullptr) {
                    if (surface.shadowSurfSched->getHrTsVal(state, iHour) == 1.0) {
                        continue;
                    }
                    if (!state.dataSolarShading->CalcSkyDifShading) {
                        if (surface.shadowSurfSched->getHrTsVal(state, iHour, TS) == 1.0) {
                            continue;
                        }
                    }
                }
            }
            // Eliminate shadowing surfaces that is supposed to be disabled.
            if (state.dataSysVars->DisableAllSelfShading) {
                if (surface.Zone != 0) {
                    continue; // Disable all shadowing surfaces in all zones. Attached shading surfaces are not part of a zone, zone value is 0.
                }
            } else if (state.dataSysVars->DisableGroupSelfShading) {
                std::vector<int> DisabledZones = s_surf->SurfShadowDisabledZoneList(CurSurf);
                bool isDisabledShadowSurf = false;
                for (int i : DisabledZones) {
                    if (surface.Zone == i) {
                        isDisabledShadowSurf = true;
                        break;
                    }
                }
                if (isDisabledShadowSurf) {
                    continue; // Disable all shadowing surfaces in all disabled zones.
                }
            }

            //      IF ((.NOT.Surface(GSSNR)%HeatTransSurf) .AND. &
            //            GetCurrentScheduleValue(state, Surface(GSSNR)%SchedShadowSurfIndex) == 1.0) CYCLE

            // Transform shadow casting surface from cartesian to homogeneous coordinates according to surface type.

            if ((notHeatTransSurf) && (surface.BaseSurf != 0)) {

                // For shadowing subsurface coordinates of shadow casting surface are relative to the receiving surface
                // project shadow to the receiving surface

                state.dataSolarShading->NVS = surface.Sides;
                auto const &XV = s_surf->ShadeV(GSSNR).XV;
                auto const &YV = s_surf->ShadeV(GSSNR).YV;
                auto const &ZV = s_surf->ShadeV(GSSNR).ZV;
                for (int N = 1; N <= state.dataSolarShading->NVS; ++N) {
                    state.dataSolarShading->XVS(N) = XV(N) - state.dataSolarShading->XShadowProjection * ZV(N);
                    state.dataSolarShading->YVS(N) = YV(N) - state.dataSolarShading->YShadowProjection * ZV(N);
                }

            } else {
                // Transform coordinates of shadow casting surface from general system to the system relative to the receiving surface
                int NVT;
                CTRANS(state, GSSNR, NGRS, NVT, state.dataSolarShading->XVert, state.dataSolarShading->YVert, state.dataSolarShading->ZVert);
                CLIP(state,
                     NVT,
                     state.dataSolarShading->XVert,
                     state.dataSolarShading->YVert,
                     state.dataSolarShading->ZVert); // Clip portions of the shadow casting surface which are behind the receiving surface

                if (state.dataSolarShading->NumVertInShadowOrClippedSurface <= 2) {
                    continue;
                }

                // Project shadow from shadow casting surface along sun's rays to receiving surface Shadow vertices
                // become clockwise sequential

                for (int N = 1; N <= state.dataSolarShading->NumVertInShadowOrClippedSurface; ++N) {
                    state.dataSolarShading->XVS(N) =
                        state.dataSolarShading->XVC(N) - state.dataSolarShading->XShadowProjection * state.dataSolarShading->ZVC(N);
                    state.dataSolarShading->YVS(N) =
                        state.dataSolarShading->YVC(N) - state.dataSolarShading->YShadowProjection * state.dataSolarShading->ZVC(N);
                }
            }

            // Transform to the homogeneous coordinate system.

            NS3 = state.dataSolarShading->LOCHCA + 1;
            HTRANS1(state, NS3, state.dataSolarShading->NVS);

            // Adjust near-duplicate points.

            assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCY));
            assert(state.dataSolarShading->HCX.index(1, 1) == 0u);
            size_type j(state.dataSolarShading->HCX.index(NS3, 1));
            size_type NVR(state.dataSolarShading->HCNV(1));
            for (int N = 1; N <= state.dataSolarShading->NumVertInShadowOrClippedSurface;
                 ++N, ++j) {                                       // Tuned Logic change: break after 1st "close" point found
                Int64 const HCX_N(state.dataSolarShading->HCX[j]); // [ j ] == ( NS3, N )
                Int64 const HCY_N(state.dataSolarShading->HCY[j]);
                for (size_type l = 0; l < NVR; ++l) { // [ l ] == ( 1, l+1 )
                    Real64 const delX = std::abs(state.dataSolarShading->HCX[l] - HCX_N);
                    if (delX > 6) {
                        continue;
                    }
                    Real64 const delY = std::abs(state.dataSolarShading->HCY[l] - HCY_N);
                    if (delY > 6) {
                        continue;
                    }
                    if (delX > 0) {
                        state.dataSolarShading->HCX[j] = state.dataSolarShading->HCX[l]; // [ j ] == ( NS3, N )
                    }
                    if (delY > 0) {
                        state.dataSolarShading->HCY[j] = state.dataSolarShading->HCY[l];
                    }
                    break;
                }
            }
            HTRANS0(state, NS3, state.dataSolarShading->NumVertInShadowOrClippedSurface);
            if (!state.dataSolarShading->CalcSkyDifShading && surface.shadowSurfSched != nullptr) {
                if (iHour != 0) {
                    SchValue = surface.shadowSurfSched->getHrTsVal(state, iHour, TS);
                } else {
                    SchValue = surface.SchedMinValue;
                }
            } else {
                SchValue = surface.SchedMinValue;
            }

            state.dataSolarShading->HCT(NS3) = SchValue;

            // Determine overlap of shadow with receiving surface

            state.dataSolarShading->CurrentShadowingSurface = I;
            state.dataSolarShading->CurrentSurfaceBeingShadowed = GSSNR;
            NS1 = 1;
            NS2 = NS3;
            DeterminePolygonOverlap(state, NS1, NS2, NS3);
            //  Next statement is special to deal with transmitting shading devices
            if (state.dataSolarShading->OverlapStatus == FirstSurfWithinSecond && SchValue > 0.0) {
                state.dataSolarShading->OverlapStatus = PartialOverlap;
            }
            MainOverlapStatus = state.dataSolarShading->OverlapStatus;
            ExitLoopStatus = MainOverlapStatus;

            if (MainOverlapStatus == NoOverlap) { // No overlap of general surface shadow and receiving surface
                                                  // Continue
            } else if ((MainOverlapStatus == FirstSurfWithinSecond) || (MainOverlapStatus == TooManyVertices) ||
                       (MainOverlapStatus == TooManyFigures)) {
                goto ShadowingSurfaces_exit;
            } else if ((MainOverlapStatus == SecondSurfWithinFirst) || (MainOverlapStatus == PartialOverlap)) {
                // Determine overlaps with previous shadows.
                state.dataSolarShading->LOCHCA = NS3;
                state.dataSolarShading->NGSSHC = state.dataSolarShading->LOCHCA - state.dataSolarShading->FGSSHC + 1;
                if (state.dataSolarShading->NGSSHC > 1) {
                    MULTOL(state,
                           state.dataSolarShading->LOCHCA,
                           state.dataSolarShading->FGSSHC - 1,
                           state.dataSolarShading->NGSSHC - 1); // HOYT - Remove this call
                }
            } else {
                goto ShadowingSurfaces_exit;
            }

            ExitLoopStatus = -1;
        }
    ShadowingSurfaces_exit:;

        // Compute sunlit area of surface (excluding effects of subsurfs).

        if (ExitLoopStatus == FirstSurfWithinSecond) { // Surface fully shaded
            state.dataSolarShading->SurfSunlitArea(HTS) = 0.0;
            state.dataSolarShading->LOCHCA = state.dataSolarShading->FGSSHC;

        } else if ((ExitLoopStatus == TooManyVertices) || (ExitLoopStatus == TooManyFigures)) { // Array limits exceeded, estimate
            state.dataSolarShading->SurfSunlitArea(HTS) = 0.25 * state.dataSolarShading->HCAREA(1);

        } else {

            // Compute the sunlit area here.
            // Call UnionShadow(FGSSHC,LOCHCA)

            state.dataSolarShading->NGSSHC = state.dataSolarShading->LOCHCA - state.dataSolarShading->FGSSHC + 1;
            if (state.dataSolarShading->NGSSHC <= 0) {
                state.dataSolarShading->SurfSunlitArea(HTS) = state.dataSolarShading->HCAREA(1); // Surface fully sunlit
            } else {
                Real64 A(state.dataSolarShading->HCAREA(1)); // Area
                for (int i = state.dataSolarShading->FGSSHC, e = state.dataSolarShading->FGSSHC + state.dataSolarShading->NGSSHC - 1; i <= e; ++i) {
                    A += state.dataSolarShading->HCAREA(i) * (1.0 - state.dataSolarShading->HCT(i));
                }
                state.dataSolarShading->SurfSunlitArea(HTS) = A;
                if (state.dataSolarShading->SurfSunlitArea(HTS) <= 0.0) { // Surface fully shaded
                    state.dataSolarShading->SurfSunlitArea(HTS) = 0.0;
                    state.dataSolarShading->LOCHCA = state.dataSolarShading->FGSSHC;
                }
            }
        }
    }
    state.dataSolarShading->NGSSHC = state.dataSolarShading->LOCHCA - state.dataSolarShading->FGSSHC + 1;
}

void CalcInteriorSolarOverlaps(EnergyPlusData &state,
                               int const iHour, // Hour Index
                               int const NBKS,  // Number of back surfaces associated with this GRSNR (in general, only
                               int const HTSS,  // Surface number of the subsurface (exterior window)
                               int const GRSNR, // General receiving surface number (base surface of the exterior window)
                               int const TS     // Time step Index
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   January 1999
    //       MODIFIED       Nov 2001, FW: include beam radiation overlaps with
    //                       back windows and doors; previously these subsurfaces ignored.
    //                      May 2002, FW: fix problem where reveal was not being considered
    //                       in calculating overlap areas if window is shaded only by reveal.
    //                      June 2002, FW: fix problem that gave incorrect calculation when
    //                       window is not shaded only by reveal
    //                      June 2002, FW: remove incorrect multiplication of overlap areas
    //                       by sunlit fraction when window is shaded only by reveal

    // PURPOSE OF THIS SUBROUTINE:
    // For an exterior window with surface number HTSS, determines (1) the surface numbers of back
    // surfaces receiving beam radiation from the window and (2) for each such back surface, the area
    // of the portion of the window sending beam radiation to the back surface; this is called the
    // "overlap area."

    // REFERENCES:
    // BLAST/IBLAST code, original author George Walton

    // SUBROUTINE ARGUMENT DEFINITIONS:
    //  some of these will receive beam radiation from HTSS this hour)

    // SUBROUTINE PARAMETER DEFINITIONS:
    int constexpr WindowShadedOnlyByReveal(2); // for use with RevealStatus

    typedef Array2D<Int64>::size_type size_type;
    int JBKS;        // Counter of back surfaces with non-zero overlap with HTSS
    int BackSurfNum; // Back surface number

    bool UseSimpleDistribution; // TRUE means simple interior solar distribution
    // (all incoming beam assumed to strike floor),
    // FALSE means exact interior solar distribution
    // (track which back surfaces beam illuminates)

    auto &s_surf = state.dataSurface;

    // Tuned Linear indexing

    assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCY));
    assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCA));
    assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCB));
    assert(equal_dimensions(state.dataSolarShading->HCX, state.dataSolarShading->HCC));

    if (state.dataSolarShading->SurfSunlitArea(HTSS) > 0.0) {

        UseSimpleDistribution = false;

        if ((NBKS <= 0) || (s_surf->Surface(GRSNR).ExtBoundCond > 0)) {

            UseSimpleDistribution = true;

        } else {
            // Using 'exact' distribution, replace subsurface HC entries with reveal HC entries
            // so that the reveal HC is used in calculating interior solar overlap areas

            // Adding the following line fixes a problem where, if the window was shaded only
            // by reveal, then the reveal was not considered in calculating interior solar
            // overlap areas (FCW 5/3/02).
            // IF(Surface(HTSS)%Reveal > 0.0) NRVLHC = 1
            // Changing the line to the following avoids incorrect calculation when window is not shaded
            // only by reveal (FCW 6/28/02).
            if (state.dataSolarShading->SurfWinRevealStatus(iHour, TS, HTSS) == WindowShadedOnlyByReveal) {
                state.dataSolarShading->NRVLHC = 1;
            }
            if (state.dataSolarShading->NRVLHC > 0) {
                for (int I = 1; I <= state.dataSolarShading->NRVLHC; ++I) {
                    int const iS(state.dataSolarShading->FSBSHC - 1 + I);
                    int const iR(state.dataSolarShading->FRVLHC - 1 + I);
                    state.dataSolarShading->HCT(iS) = state.dataSolarShading->HCT(iR);
                    state.dataSolarShading->HCNV(iS) = state.dataSolarShading->HCNV(iR);
                    state.dataSolarShading->HCAREA(iS) = state.dataSolarShading->HCAREA(iR);
                    size_type lS(state.dataSolarShading->HCX.index(iS, 1));
                    size_type lR(state.dataSolarShading->HCX.index(iR, 1));
                    for (int J = 1; J <= state.dataSolarShading->MaxHCV; ++J, ++lS, ++lR) { // [ lS ] == ( iS, J ), [ lR ] == ( iR, J )
                        state.dataSolarShading->HCX[lS] = state.dataSolarShading->HCX[lR];
                        state.dataSolarShading->HCY[lS] = state.dataSolarShading->HCY[lR];
                        state.dataSolarShading->HCA[lS] = state.dataSolarShading->HCA[lR];
                        state.dataSolarShading->HCB[lS] = state.dataSolarShading->HCB[lR];
                        state.dataSolarShading->HCC[lS] = state.dataSolarShading->HCC[lR];
                    }
                }
                state.dataSolarShading->NSBSHC = state.dataSolarShading->NRVLHC;
            }
        }

        // Check for array space.
        if (state.dataSolarShading->FSBSHC + state.dataSolarShading->NBKSHC > state.dataSolarShading->MaxHCS) {
            UseSimpleDistribution = true;
        }

        if (!UseSimpleDistribution) { // Compute overlaps

            std::unordered_map<unsigned, float> pssas;

#ifndef EP_NO_OPENGL
            if (state.dataSolarShading->penumbra) {
                // Add back surfaces to array
                std::vector<unsigned> pbBackSurfaces;
                for (int bkSurfNum : state.dataShadowComb->ShadowComb(GRSNR).BackSurf) {
                    if (bkSurfNum == 0) {
                        continue;
                    }
                    if (state.dataSolarShading->SurfSunCosTheta(bkSurfNum) < DataEnvironment::SunIsUpValue) {
                        pbBackSurfaces.push_back(s_surf->SurfPenumbraID(bkSurfNum));
                    }
                }
                pssas = state.dataSolarShading->penumbra->calculate_interior_pssas({(unsigned)s_surf->SurfPenumbraID(HTSS)}, pbBackSurfaces);
                // penumbra->render_interior_scene({(unsigned)Surface(HTSS).PenumbraID}, pbBackSurfaces);

                JBKS = 0;
                for (int bkSurfNum : state.dataShadowComb->ShadowComb(GRSNR).BackSurf) {
                    if (bkSurfNum == 0) {
                        continue;
                    }
                    if (pssas[s_surf->SurfPenumbraID(bkSurfNum)] > 0) {
                        ++JBKS;
                        state.dataHeatBal->SurfWinBackSurfaces(iHour, TS, JBKS, HTSS) = bkSurfNum;
                        Real64 OverlapArea = pssas[s_surf->SurfPenumbraID(bkSurfNum)] / state.dataSolarShading->SurfSunCosTheta(HTSS);
                        state.dataHeatBal->SurfWinOverlapAreas(iHour, TS, JBKS, HTSS) = OverlapArea * s_surf->SurfaceWindow(HTSS).glazedFrac;
                    }
                }
            }
#endif

            if (!state.dataSolarShading->penumbra) {

                state.dataSolarShading->FINSHC = state.dataSolarShading->FSBSHC + state.dataSolarShading->NSBSHC;

                JBKS = 0;

                for (int IBKS = 1; IBKS <= state.dataSolarShading->NBKSHC;
                     ++IBKS) { // Loop over back surfaces to GRSNR this hour. NBKSHC is the number of
                    // back surfaces that would receive beam radiation from the base surface, GRSNR,
                    // if the base surface was transparent. In general, some (at least one) or all of these
                    // will receive beam radiation from the exterior window subsurface, HTSS, of GRSNR,
                    // depending on the size of HTSS and its location on GRSNR

                    BackSurfNum = state.dataSolarShading->HCNS(state.dataSolarShading->FBKSHC - 1 + IBKS);

                    // Determine if this back surface number can receive beam radiation from the
                    // exterior window, HTSS, this hour, i.e., overlap area is positive

                    state.dataSolarShading->LOCHCA = state.dataSolarShading->FINSHC - 1;

                    MULTOL(state, state.dataSolarShading->FBKSHC - 1 + IBKS, state.dataSolarShading->FSBSHC - 1, state.dataSolarShading->NSBSHC);

                    // Compute overlap area for this back surface

                    state.dataSolarShading->NINSHC = state.dataSolarShading->LOCHCA - state.dataSolarShading->FINSHC + 1;
                    if (state.dataSolarShading->NINSHC <= 0) {
                        continue;
                    }
                    Real64 OverlapArea = state.dataSolarShading->HCAREA(state.dataSolarShading->FINSHC);
                    for (int J = 2; J <= state.dataSolarShading->NINSHC; ++J) {
                        OverlapArea += state.dataSolarShading->HCAREA(state.dataSolarShading->FINSHC - 1 + J) *
                                       (1.0 - state.dataSolarShading->HCT(state.dataSolarShading->FINSHC - 1 + J));
                    }

                    if (OverlapArea > 0.001) {
                        ++JBKS;
                        if (JBKS <= state.dataBSDFWindow->MaxBkSurf) {
                            state.dataHeatBal->SurfWinBackSurfaces(iHour, TS, JBKS, HTSS) = BackSurfNum;
                            int baseSurfaceNum = s_surf->Surface(BackSurfNum).BaseSurf;
                            state.dataHeatBal->SurfWinOverlapAreas(iHour, TS, JBKS, HTSS) = OverlapArea * s_surf->SurfaceWindow(HTSS).glazedFrac;
                            // If this is a subsurface, subtract its overlap area from its base surface
                            if (baseSurfaceNum != BackSurfNum) {
                                for (int iBaseBKS = 1; iBaseBKS <= JBKS; ++iBaseBKS) {
                                    if (baseSurfaceNum == state.dataHeatBal->SurfWinBackSurfaces(iHour, TS, iBaseBKS, HTSS)) {
                                        state.dataHeatBal->SurfWinOverlapAreas(iHour, TS, iBaseBKS, HTSS) =
                                            max(0.0,
                                                state.dataHeatBal->SurfWinOverlapAreas(iHour, TS, iBaseBKS, HTSS) -
                                                    state.dataHeatBal->SurfWinOverlapAreas(iHour, TS, JBKS, HTSS));
                                        break;
                                    }
                                }
                            }
                        }
                    }
                } // End of loop over back surfaces
            }
        }
    } // End of check that sunlit area > 0.
}

void CalcInteriorSolarDistribution(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   January 1999
    //       MODIFIED       Nov 1999, FW, for Window5 calculation method
    //                      Oct 2000, FW: add transmitted solar variables for reporting
    //                      Mar 2001, FW: add new calc of solar absorbed by window shades
    //                      May 2001, FW: add calc of solar transmitted and absorbed by window blinds
    //                      Oct 2001, LL: remove interpolation, solar now at time step
    //                      Oct 2001, FW: add solar transmitted through interior windows
    //                      Mar 24, 2001, FW: remove incorrect multiplication of Boverlap by sunlit fraction
    //                                        since effect of shadowing is already included in Aoverlap
    //                      Apr 2001, FW: add effects of beam solar reflection from outside and inside reveals
    //                      Jan 2003, FW: add between-glass shades and blinds
    //                      Dec 2003, FW: report beam incident on inside of surface
    //                      Jan 2004, FW: for blinds with horizontal slats, allow different diffuse/diffuse
    //                                    transmittance for ground and sky solar
    //                      Apr 2004, FW: allow diffusing glazing
    //                      May 2006, RR: allow external window screen
    //                      Jan 2010, TH: add calculating and reporting of WinBmBmSolar, WinBmDifSolar,
    //                                    WinBmBmSolarEnergy, and WinBmDifSolarEnergy
    //                      Jun 2013, SV: scheduled surface gains for walls and windows

    // PURPOSE OF THIS SUBROUTINE:
    // For a time step, calculates solar radiation absorbed by exterior
    // surfaces and interior solar radiation distribution

    using Dayltg::TransTDD;
    using namespace DataWindowEquivalentLayer;

    Array1D<Real64> CFBoverlap;    // Sum of boverlap for each back surface
    Array2D<Real64> CFDirBoverlap; // Directional boverlap (Direction, IBack)

    auto &s_mat = state.dataMaterial;
    auto &s_surf = state.dataSurface;

#ifdef EP_Count_Calls
    ++state.dataTimingsData->NumIntSolarDist_Calls;
#endif

    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurfWin = thisSpace.WindowSurfaceFirst;
            int const lastSurfWin = thisSpace.WindowSurfaceLast;
            for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {
                for (int lay = 1; lay <= CFSMAXNL + 1; ++lay) {
                    s_surf->SurfWinA(SurfNum, lay) = 0.0;
                }
                state.dataSolarShading->SurfWinIntBeamAbsByShadFac(SurfNum) = 0.0;
                state.dataSolarShading->SurfWinExtBeamAbsByShadFac(SurfNum) = 0.0;
            }
            int const firstSurfOpaque = thisSpace.OpaqOrIntMassSurfaceFirst;
            int const lastSurfOpaque = thisSpace.OpaqOrIntMassSurfaceLast;
            for (int SurfNum = firstSurfOpaque; SurfNum <= lastSurfOpaque; ++SurfNum) {
                s_surf->SurfOpaqAI(SurfNum) = 0.0;
                s_surf->SurfOpaqAO(SurfNum) = 0.0;
            }
        }
    }

    if ((int)state.dataDaylightingDevicesData->TDDPipe.size() > 0) {
        for (auto &e : state.dataDaylightingDevicesData->TDDPipe) {
            int SurfDome = e.Dome;
            for (int lay = 1; lay <= CFSMAXNL + 1; ++lay) {
                s_surf->SurfWinA(SurfDome, lay) = 0.0;
            }
            state.dataSolarShading->SurfWinIntBeamAbsByShadFac(SurfDome) = 0.0;
            state.dataSolarShading->SurfWinExtBeamAbsByShadFac(SurfDome) = 0.0;
        }
    }

    for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclosureNum) {
        // Solar entering a zone as beam or diffuse radiation, originating as beam solar incident on exterior windows)/(Beam normal solar) [W/(W/m2)]
        Real64 BTOTZone = 0.0;
        // Beam radiation from exterior windows absorbed in a zone or transmitted through
        Real64 BABSZone = 0.0;
        state.dataHeatBal->EnclSolDB(enclosureNum) = 0.0;
        state.dataHeatBal->EnclSolDBSSG(enclosureNum) = 0.0;
        state.dataHeatBal->EnclSolDBIntWin(enclosureNum) = 0.0;
        // Loop over exterior surfaces in this zone
        auto &thisEnclosure = state.dataViewFactor->EnclSolInfo(enclosureNum);
        // delete values from previous timestep
        if (state.dataHeatBal->AnyBSDF) {
            s_surf->SurfWinACFOverlap = 0.0;
        }

        //-------------------------------------------------------------------------
        // EXTERIOR BEAM SOLAR RADIATION ABSORBED ON THE OUTSIDE OF OPAQUE SURFACES
        //-------------------------------------------------------------------------
        // TODO: use opaq and window loop after airboundary is sorted
        // TODO: It may be useful to sort SurfacePtr to group windows and domes together to reduce if conditions
        for (int const SurfNum : thisEnclosure.SurfacePtr) {
            if (s_surf->Surface(SurfNum).Class != SurfaceClass::Window && s_surf->Surface(SurfNum).Class != SurfaceClass::TDD_Dome) {
                if (!s_surf->Surface(SurfNum).HeatTransSurf) {
                    continue;
                }
                if (!s_surf->Surface(SurfNum).ExtSolar) {
                    continue;
                }
                int const ConstrNum = s_surf->SurfActiveConstruction(SurfNum);
                Real64 CosInc = state.dataHeatBal->SurfCosIncAng(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum);
                Real64 SunLitFract = state.dataHeatBal->SurfSunlitFrac(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum);
                s_surf->SurfOpaqAO(SurfNum) = state.dataConstruction->Construct(ConstrNum).OutsideAbsorpSolar * CosInc * SunLitFract;
            }
        }

        //--------------------------------------------------------------------------------------------------------
        // EXTERIOR WINDOWS OR TDD DOMES
        //--------------------------------------------------------------------------------------------------------
        for (int const SurfNum : thisEnclosure.SurfacePtr) {
            auto &surf = s_surf->Surface(SurfNum);
            if (surf.Class != SurfaceClass::Window && surf.Class != SurfaceClass::TDD_Dome) {
                continue;
            }
            if (!surf.ExtSolar && surf.OriginalClass != SurfaceClass::TDD_Diffuser) {
                continue;
            }

            auto &surfWin = s_surf->SurfaceWindow(SurfNum);
            int const ConstrNum = s_surf->SurfActiveConstruction(SurfNum);
            int const ConstrNumSh = s_surf->SurfWinActiveShadedConstruction(SurfNum);
            auto &thisConstruct = state.dataConstruction->Construct(ConstrNum);
            int ScNum = surfWin.screenNum;
            WinShadingType ShadeFlag = s_surf->SurfWinShadingFlag(SurfNum); // Set in subr. WindowShadingManager

            auto &surfShade = s_surf->surfShades(SurfNum);

            Real64 ProfAng = 0.0; // Window solar profile angle (radians)

            Real64 SlatAng = surfShade.blind.slatAng;
            int PipeNum = s_surf->SurfWinTDDPipeNum(SurfNum);
            int SurfNum2 = SurfNum;
            if (surf.OriginalClass == SurfaceClass::TDD_Diffuser) {
                SurfNum2 = state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome;
            }
            Real64 CosInc = state.dataHeatBal->SurfCosIncAng(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum2);
            Real64 SunLitFract = state.dataHeatBal->SurfSunlitFrac(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum2);

            //-----------------------------------------
            // BLOCK 1
            // EXTERIOR BEAM AND DIFFUSE SOLAR RADIATION ABSORBED IN THE GLASS LAYERS OF (SurfWinA)
            // EXTERIOR BEAM ABSORBED BY SHADING DEVICE (SurfWinExtBeamAbsByShadFac)
            //-----------------------------------------
            // Somewhat of a kludge
            if (surf.Class == SurfaceClass::TDD_Dome || surf.OriginalClass == SurfaceClass::TDD_Diffuser) {
                state.dataHeatBal->SurfSunlitFracWithoutReveal(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum) =
                    SunLitFract; // Frames/dividers not allow
            }
            int FenSolAbsPtr = 0;
            if (s_surf->SurfWinWindowModelType(SurfNum) == WindowModel::BSDF) {
                FenSolAbsPtr = WindowScheduledSolarAbs(state, SurfNum, ConstrNum);
            }
            bool SunlitFracWithoutReveal =
                state.dataHeatBal->SurfSunlitFracWithoutReveal(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum) > 0;

            // Calculate interpolated blind properties
            Real64 FrontDiffDiffTrans; // Bare-blind front diffuse-diffuse solar transmittance
            Real64 FrontDiffDiffRefl;
            Real64 FrontDiffAbs;      // Bare-blind front diffuse solar reflectance
            Real64 BackDiffDiffTrans; // Bare-blind back diffuse-diffuse solar transmittance
            Real64 BackDiffDiffRefl;
            Real64 BackDiffAbs; // Bare-blind back diffuse solar reflectance

            Real64 FrontBeamDiffTrans; // Blind ProfileAnglesolar front beam-diffuse transmittance
            Real64 BackBeamDiffTrans;  // Blind solar back beam-diffuse transmittance
            Real64 FrontBeamDiffRefl;  // Blind solar front beam-diffuse reflectance
            Real64 BackBeamDiffRefl;   // Blind solar back beam-diffuse reflectance
            Real64 FrontBeamAbs;       // Blind solar front beam absorptance
            Real64 BackBeamAbs;        // Blind solar back beam absorptance

            if (s_surf->SurfWinWindowModelType(SurfNum) != WindowModel::EQL && ANY_BLIND(ShadeFlag)) {
                auto const &surfShade = s_surf->surfShades(SurfNum);
                [[maybe_unused]] auto const *matBlind = dynamic_cast<Material::MaterialBlind const *>(s_mat->materials(surfShade.blind.matNum));
                assert(matBlind != nullptr);

                int profIdxLo = surfShade.blind.profAngIdxLo;
                int profIdxHi = surfShade.blind.profAngIdxHi;
                Real64 profInterpFac = surfShade.blind.profAngInterpFac;

                FrontDiffDiffTrans = surfShade.blind.TAR.Sol.Ft.Df.Tra;

                if (SunLitFract > 0.0 || SunlitFracWithoutReveal) {
                    auto const &btar1 = surfShade.blind.TAR;
                    auto const &btarFront1Lo = btar1.Sol.Ft.Bm[profIdxLo];
                    auto const &btarFront1Hi = btar1.Sol.Ft.Bm[profIdxHi];

                    FrontBeamAbs = Interp(btarFront1Lo.Abs, btarFront1Hi.Abs, profInterpFac);
                    FrontBeamDiffTrans = Interp(btarFront1Lo.DfTra, btarFront1Hi.DfTra, profInterpFac);
                    if (ShadeFlag != WinShadingType::ExtBlind) { // FRONT: interior or bg blinds
                        FrontDiffDiffRefl = btar1.Sol.Ft.Df.Ref;
                        FrontDiffAbs = btar1.Sol.Ft.Df.Abs;
                        FrontBeamDiffRefl = Interp(btarFront1Lo.DfRef, btarFront1Hi.DfRef, profInterpFac);
                    }

                    if (ShadeFlag != WinShadingType::IntBlind) { // BACK: exterior or bg blinds{
                        BackDiffDiffTrans = btar1.Sol.Bk.Df.Tra;
                        BackDiffDiffRefl = btar1.Sol.Bk.Df.Ref;
                        BackDiffAbs = btar1.Sol.Bk.Df.Abs;

                        auto const &btarBack1Lo = btar1.Sol.Bk.Bm[profIdxLo];
                        auto const &btarBack1Hi = btar1.Sol.Bk.Bm[profIdxHi];

                        BackBeamDiffTrans = Interp(btarBack1Lo.DfTra, btarBack1Hi.DfTra, profInterpFac);
                        BackBeamDiffRefl = Interp(btarBack1Lo.DfRef, btarBack1Hi.DfRef, profInterpFac);
                        BackBeamAbs = Interp(btarBack1Lo.Abs, btarBack1Hi.Abs, profInterpFac);
                    }
                } // if (SunLitFrac > 0.0)
            } // if (ANY_BLIND)

            if (SunlitFracWithoutReveal) {

                if (s_surf->SurfWinWindowModelType(SurfNum) == WindowModel::Detailed) {

                    // For bare glazing or switchable glazing, the following includes the effects of
                    // (1) diffuse solar produced by beam solar incident on the outside and inside reveal
                    // surfaces, and (2) absorption of beam solar by outside and inside reveal surfaces.
                    // If there is an exterior shade/blind both of these effects are ignored. If there
                    // is an interior or between-glass shade/blind the effects of beam incident on
                    // inside reveal surfaces is ignored.
                    int NGlass = thisConstruct.TotGlassLayers;
                    Array1D<Real64> AbWin(NGlass); // Factor for front beam radiation absorbed in window glass layer
                    for (int Lay = 1; Lay <= NGlass; ++Lay) {
                        AbWin(Lay) = Window::POLYF(CosInc, thisConstruct.AbsBeamCoef(Lay)) * CosInc * SunLitFract *
                                     s_surf->SurfaceWindow(SurfNum).OutProjSLFracMult[state.dataGlobal->HourOfDay];
                    }
                    if (!IS_SHADED_NO_GLARE_CTRL(ShadeFlag)) {
                        // (ShadeFlag <= 0 || ShadeFlag >= 10) - Bare window (ShadeFlag = -1 or 0 or shading device of off)
                        for (int Lay = 1; Lay <= NGlass; ++Lay) {
                            // Add contribution of beam reflected from outside and inside reveal
                            s_surf->SurfWinA(SurfNum, Lay) = AbWin(Lay) +
                                                             s_surf->SurfWinOutsRevealDiffOntoGlazing(SurfNum) * thisConstruct.AbsDiff(Lay) +
                                                             s_surf->SurfWinInsRevealDiffOntoGlazing(SurfNum) * thisConstruct.AbsDiffBack(Lay);
                        }
                    } else {
                        // Shade, screen, blind or switchable glazing on (ShadeFlag > 0)
                        Real64 FracSunLit =
                            SunLitFract * s_surf->SurfaceWindow(SurfNum)
                                              .OutProjSLFracMult[state.dataGlobal->HourOfDay]; // Effective fraction of window that is sunlit;
                        Real64 InOutProjSLFracMult = s_surf->SurfaceWindow(SurfNum).InOutProjSLFracMult[state.dataGlobal->HourOfDay];
                        Array1D<Real64> AbWinSh(NGlass);    // Like AbWin, but for shaded window
                        Array1D<Real64> ADiffWinSh(NGlass); // Diffuse solar absorptance of glass layer, window with shading device
                        if (ANY_EXTERIOR_SHADE_BLIND_SCREEN(ShadeFlag)) {
                            FracSunLit = SunLitFract;
                        }

                        if (ANY_SHADE(ShadeFlag) || ShadeFlag == WinShadingType::SwitchableGlazing) {
                            auto const &thisConstructSh = state.dataConstruction->Construct(ConstrNumSh);
                            // Shade or switchable glazing on
                            for (int Lay = 1; Lay <= NGlass; ++Lay) {
                                AbWinSh(Lay) = Window::POLYF(CosInc, thisConstructSh.AbsBeamCoef(Lay)) * CosInc * FracSunLit;
                                ADiffWinSh(Lay) = thisConstructSh.AbsDiff(Lay);
                            }
                            if (ShadeFlag == WinShadingType::IntShade) { // Exterior beam absorbed by INTERIOR SHADE
                                // Note that AbsBeamShadeCoef includes effect of shade/glazing inter-reflection
                                Real64 AbsShade = Window::POLYF(CosInc,
                                                                thisConstructSh.AbsBeamShadeCoef); // Interior shade or blind beam solar absorptance
                                state.dataSolarShading->SurfWinExtBeamAbsByShadFac(SurfNum) =
                                    (AbsShade * CosInc * SunLitFract * InOutProjSLFracMult +
                                     s_surf->SurfWinOutsRevealDiffOntoGlazing(SurfNum) * thisConstructSh.AbsDiffShade) *
                                    surfWin.glazedFrac;
                                // In the above, GlazedFrac corrects for shadowing of divider onto interior shade
                            } else if (ShadeFlag == WinShadingType::ExtShade) { // Exterior beam absorbed by EXTERIOR SHADE
                                state.dataSolarShading->SurfWinExtBeamAbsByShadFac(SurfNum) = thisConstructSh.AbsDiffShade * CosInc * SunLitFract;
                            } else if (ShadeFlag == WinShadingType::BGShade) { // Exterior beam absorbed by BETWEEN-GLASS SHADE
                                Real64 AbsShade = Window::POLYF(CosInc, thisConstructSh.AbsBeamShadeCoef);
                                state.dataSolarShading->SurfWinExtBeamAbsByShadFac(SurfNum) =
                                    AbsShade * CosInc * SunLitFract +
                                    s_surf->SurfWinOutsRevealDiffOntoGlazing(SurfNum) * thisConstructSh.AbsDiffShade;
                            }

                        } else {
                            // Blind or screen on
                            ProfAng = surfShade.blind.profAng;
                            if (ShadeFlag == WinShadingType::IntBlind) {
                                // Interior blind on
                                Real64 TBmBm = Window::POLYF(CosInc, thisConstruct.TransSolBeamCoef);
                                Real64 RGlDiffBack = thisConstruct.ReflectSolDiffBack; // Glazing system back diffuse solar reflectance
                                Real64 RhoBlFront = FrontBeamDiffRefl;                 // Blind solar front beam reflectance
                                Real64 RhoBlDiffFront = FrontDiffDiffRefl;             // Blind solar front diffuse reflectance
                                for (int Lay = 1; Lay <= NGlass; ++Lay) {
                                    Real64 ADiffWin = thisConstruct.AbsDiff(Lay);        // Diffuse solar absorptance of glass layer, bare window
                                    Real64 AGlDiffBack = thisConstruct.AbsDiffBack(Lay); // Glass layer back diffuse solar absorptance
                                    AbWinSh(Lay) =
                                        AbWin(Lay) + (TBmBm * AGlDiffBack * RhoBlFront / (1.0 - RhoBlFront * RGlDiffBack)) * CosInc * FracSunLit;
                                    ADiffWinSh(Lay) =
                                        ADiffWin + thisConstruct.TransDiff * AGlDiffBack * RhoBlDiffFront / (1.0 - RhoBlDiffFront * RGlDiffBack);
                                }
                                // Exterior beam absorbed by INTERIOR BLIND

                                Real64 AbsBlFront = FrontBeamAbs;     // Blind solar front beam absorptance
                                Real64 AbsBlDiffFront = FrontDiffAbs; // Blind solar front diffuse absorptance
                                Real64 AbsShade =
                                    TBmBm * (AbsBlFront + RhoBlFront * RGlDiffBack * AbsBlDiffFront / (1.0 - RhoBlDiffFront * RGlDiffBack));
                                Real64 AbsShadeDiff =
                                    thisConstruct.TransDiff *
                                    (AbsBlDiffFront + RhoBlDiffFront * RGlDiffBack * AbsBlDiffFront /
                                                          (1.0 - RhoBlDiffFront * RGlDiffBack)); // Interior shade or blind diffuse solar absorptance

                                state.dataSolarShading->SurfWinExtBeamAbsByShadFac(SurfNum) =
                                    (AbsShade * CosInc * SunLitFract * InOutProjSLFracMult +
                                     s_surf->SurfWinOutsRevealDiffOntoGlazing(SurfNum) * AbsShadeDiff) *
                                    surfWin.glazedFrac;

                                // In the above, GlazedFrac corrects for shadowing of divider onto interior blind
                            } else if (ShadeFlag == WinShadingType::ExtBlind) {
                                // Exterior blind on
                                Real64 TBlBmBm = surfShade.blind.bmBmTrans; // Blind solar front beam-beam transmittance
                                Real64 TBlDifDif = FrontDiffDiffTrans;      // Diffuse-diffuse solar transmittance of blind
                                Real64 TBlBmDiff = FrontBeamDiffTrans;      // Blind solar front beam-diffuse transmittance
                                Real64 RhoBlBack = BackBeamDiffRefl;        // Blind solar back beam-diffuse reflectance
                                Real64 RhoBlDiffBack = BackDiffDiffRefl;    // Blind solar back diffuse reflectance
                                Real64 RGlFront =
                                    Window::POLYF(CosInc, thisConstruct.ReflSolBeamFrontCoef); // Glazing system solar front beam-beam reflectance
                                Real64 RGlDiffFront = thisConstruct.ReflectSolDiffFront;       // Glazing system front diffuse solar reflectance
                                for (int Lay = 1; Lay <= NGlass; ++Lay) {
                                    Real64 ADiffWin = thisConstruct.AbsDiff(Lay);     // Diffuse solar absorptance of glass layer, bare window
                                    Real64 AGlDiffFront = thisConstruct.AbsDiff(Lay); // Glass layer front diffuse solar absorptance
                                    AbWinSh(Lay) = TBlBmBm * AbWin(Lay) + ((TBlBmBm * RGlFront * RhoBlBack + TBlBmDiff) * AGlDiffFront /
                                                                           (1 - RGlDiffFront * RhoBlDiffBack)) *
                                                                              CosInc * FracSunLit;
                                    // ADiffWinSh = 0.0  ! Assumes no contribution from reveal reflection when exterior blind in place
                                    // Replaced above line with (FCW, 2/10/03):
                                    ADiffWinSh(Lay) = ADiffWin * TBlDifDif / (1.0 - RGlDiffFront * RhoBlDiffBack);
                                }
                                // Exterior beam absorbed by EXTERIOR BLIND
                                Real64 AbsBlFront = FrontBeamAbs;
                                Real64 AbsBlBack = BackBeamAbs;     // Blind solar back beam absorptance
                                Real64 AbsBlDiffBack = BackDiffAbs; // Blind solar back diffuse absorptance
                                Real64 AbsShade = AbsBlFront + AbsBlBack * RGlFront * TBlBmBm +
                                                  (AbsBlDiffBack * RGlDiffFront / (1.0 - RhoBlDiffBack * RGlDiffFront)) *
                                                      (RGlFront * TBlBmBm * RhoBlBack + TBlBmDiff);
                                state.dataSolarShading->SurfWinExtBeamAbsByShadFac(SurfNum) = AbsShade * CosInc * SunLitFract * InOutProjSLFracMult;
                            } else if (ShadeFlag == WinShadingType::ExtScreen) {
                                // Exterior screen on
                                auto const *screen = dynamic_cast<Material::MaterialScreen const *>(s_mat->materials(ScNum));
                                assert(screen != nullptr);

                                auto &surf = s_surf->Surface(SurfNum);

                                Real64 solPhi = std::acos(state.dataEnvrn->SOLCOS.z);
                                Real64 solTheta = std::atan2(state.dataEnvrn->SOLCOS.x, state.dataEnvrn->SOLCOS.y);
                                Real64 winPhi = surf.Tilt * Constant::DegToRad;
                                Real64 winTheta = surf.Azimuth * Constant::DegToRad;
                                Real64 phi = std::abs(solPhi - winPhi);
                                Real64 theta = std::abs(solTheta - winTheta);
                                Material::NormalizePhiTheta(phi, theta);
#ifdef PRECALC_INTERP_SCREEN
                                int ip1, ip2, it1, it2;
                                BilinearInterpCoeffs coeffs;
                                Material::GetPhiThetaIndices(phi, theta, screen->dPhi, screen->dTheta, ip1, ip2, it1, it2);
                                GetBilinearInterpCoeffs(
                                    phi, theta, ip1 * screen->dPhi, ip2 * screen->dPhi, it1 * screen->dTheta, it2 * screen->dTheta, coeffs);
                                auto const &b11 = screen->btars[ip1][it1];
                                auto const &b12 = screen->btars[ip1][it2];
                                auto const &b21 = screen->btars[ip2][it1];
                                auto const &b22 = screen->btars[ip2][it2];

                                Real64 TScDifDif = screen->DfTrans;
                                Real64 TScBmDif = BilinearInterp(b11.DfTrans, b12.DfTrans, b21.DfTrans, b22.DfTrans, coeffs);
                                Real64 TScBmBm = BilinearInterp(b11.BmTrans, b12.BmTrans, b21.BmTrans, b22.BmTrans, coeffs);
                                Real64 RScBack = BilinearInterp(b11.RefSolFront, b12.RefSolFront, b21.RefSolFront, b22.RefSolFront, coeffs);
                                Real64 RScDifBack = screen->DfRef;

                                Real64 RGlFront =
                                    Window::POLYF(CosInc, thisConstruct.ReflSolBeamFrontCoef); // Glazing system solar front beam-beam reflectance
                                Real64 RGlDiffFront = thisConstruct.ReflectSolDiffFront;       // Glazing system front diffuse solar reflectance
                                Real64 RGlDifFr = thisConstruct.ReflectSolDiffFront;           // Diffuse front reflectance of glass
                                // Reduce the bare window absorbed beam by the screen beam transmittance and then account for
                                // interreflections
                                for (int Lay = 1; Lay <= NGlass; ++Lay) {
                                    Real64 ADiffWin = thisConstruct.AbsDiff(Lay); // Diffuse solar absorptance of glass layer, bare window
                                    AbWinSh(Lay) = TScBmBm * AbWin(Lay) + (TScBmBm * RGlFront * RScBack + TScBmDif) * thisConstruct.AbsDiff(Lay) /
                                                                              (1.0 - RGlDiffFront * RScDifBack) * CosInc * FracSunLit;
                                    ADiffWinSh(Lay) = ADiffWin * TScDifDif / (1.0 - RGlDifFr * RScDifBack);
                                }
                                // Exterior beam absorbed by EXTERIOR SCREEN
                                Real64 AbsScBeam = BilinearInterp(b11.AbsSolFront, b12.AbsSolFront, b21.AbsSolFront, b22.AbsSolFront, coeffs);
                                Real64 AbsScDiffBack = screen->DfAbs;
                                Real64 AbsScreen = AbsScBeam * (1.0 + TScBmBm * RGlFront) +
                                                   (AbsScDiffBack * TScBmBm * RGlFront * RGlDiffFront * RScBack /
                                                    (1.0 - RScDifBack * RGlDiffFront)); // Exterior screen beam solar absorptance
                                state.dataSolarShading->SurfWinExtBeamAbsByShadFac(SurfNum) = AbsScreen * CosInc * SunLitFract * InOutProjSLFracMult;
#else  // !PRECALC_INTERP_SCREEN
                                Material::ScreenBmTransAbsRef btar;
                                CalcScreenTransmittance(state, screen, phi, theta, btar);

                                Real64 TScDifDif = screen->DfTrans;
                                Real64 TScBmDif = btar.DfTrans;
                                Real64 TScBmBm = btar.BmTrans;
                                Real64 RScBack = btar.RefSolFront;
                                Real64 RScDifBack = screen->DfRef;

                                Real64 RGlFront =
                                    Window::POLYF(CosInc, thisConstruct.ReflSolBeamFrontCoef); // Glazing system solar front beam-beam reflectance
                                Real64 RGlDiffFront = thisConstruct.ReflectSolDiffFront;       // Glazing system front diffuse solar reflectance
                                Real64 RGlDifFr = thisConstruct.ReflectSolDiffFront;           // Diffuse front reflectance of glass
                                // Reduce the bare window absorbed beam by the screen beam transmittance and then account for
                                // interreflections
                                for (int Lay = 1; Lay <= NGlass; ++Lay) {
                                    Real64 ADiffWin = thisConstruct.AbsDiff(Lay); // Diffuse solar absorptance of glass layer, bare window
                                    AbWinSh(Lay) = TScBmBm * AbWin(Lay) + (TScBmBm * RGlFront * RScBack + TScBmDif) * thisConstruct.AbsDiff(Lay) /
                                                                              (1.0 - RGlDiffFront * RScDifBack) * CosInc * FracSunLit;
                                    ADiffWinSh(Lay) = ADiffWin * TScDifDif / (1.0 - RGlDifFr * RScDifBack);
                                }
                                // Exterior beam absorbed by EXTERIOR SCREEN
                                Real64 AbsScBeam = btar.AbsSolFront;
                                Real64 AbsScDiffBack = screen->DfAbs;
                                Real64 AbsScreen = AbsScBeam * (1.0 + TScBmBm * RGlFront) +
                                                   (AbsScDiffBack * TScBmBm * RGlFront * RGlDiffFront * RScBack /
                                                    (1.0 - RScDifBack * RGlDiffFront)); // Exterior screen beam solar absorptance
                                state.dataSolarShading->SurfWinExtBeamAbsByShadFac(SurfNum) = AbsScreen * CosInc * SunLitFract * InOutProjSLFracMult;
#endif // PRECALC_INTERP_SCREEN
                            } else if (ShadeFlag == WinShadingType::BGBlind) {
                                // Between-glass blind o
                                // Isolated glass and blind properties at current incidence angle, profile angle and slat angle
                                Real64 t1 = Window::POLYF(CosInc,
                                                          thisConstruct.tBareSolCoef(1)); // Bare-glass beam solar trans for glass layers 1,2 and 3
                                Real64 t2 = Window::POLYF(CosInc, thisConstruct.tBareSolCoef(2));
                                Real64 af1 =
                                    Window::POLYF(CosInc,
                                                  thisConstruct.afBareSolCoef(1)); // Bare-glass beam solar front abs for glass layers 1,2 and 3
                                Real64 af2 = Window::POLYF(CosInc, thisConstruct.afBareSolCoef(2));
                                Real64 ab1 = Window::POLYF(
                                    CosInc,
                                    thisConstruct.abBareSolCoef(1)); // Bare-glass beam solar back absorptance for glass layers 1,2 and 3
                                Real64 ab2 = Window::POLYF(CosInc, thisConstruct.abBareSolCoef(2));
                                Real64 rf2 = Window::POLYF(CosInc, thisConstruct.rfBareSolCoef(2));
                                Real64 td1 = thisConstruct.tBareSolDiff(1); // Bare-glass diffuse solar transmittance for glass layers 1,2 and 3
                                Real64 td2 = thisConstruct.tBareSolDiff(2);
                                Real64 afd1 = thisConstruct.afBareSolDiff(1); // Bare-glass diffuse solar front absorptance for glass layers 1,2 and 3
                                Real64 afd2 = thisConstruct.afBareSolDiff(2);
                                Real64 abd1 = thisConstruct.abBareSolDiff(1); // Bare-glass diffuse solar back absorptance for glass layers 1,2 and 3
                                Real64 abd2 = thisConstruct.abBareSolDiff(2);
                                Real64 rfd2 = thisConstruct.rfBareSolDiff(2);
                                Real64 rbd1 = thisConstruct.rbBareSolDiff(1); // Bare-glass diffuse solar back reflectance for glass layers 1,2 and 3
                                Real64 rbd2 = thisConstruct.rbBareSolDiff(2);
                                Real64 tfshBB = surfShade.blind.bmBmTrans; // Bare-blind front and back beam-beam solar transmittance

                                auto const *matBlind = dynamic_cast<Material::MaterialBlind const *>(s_mat->materials(surfShade.blind.matNum));
                                assert(matBlind != nullptr);
                                Real64 tbshBB = matBlind->BeamBeamTrans(ProfAng, Constant::Pi - SlatAng);
                                Real64 tfshBd = FrontBeamDiffTrans; // Bare-blind front and back beam-diffuse solar transmittance
                                Real64 tbshBd = BackBeamDiffTrans;
                                Real64 rfshB = FrontBeamDiffRefl; // Bare-blind front and back beam solar reflectance
                                Real64 rbshB = BackBeamDiffRefl;
                                Real64 afshB = FrontBeamAbs;
                                Real64 abshB = BackBeamAbs;

                                Real64 tfshd = FrontDiffDiffTrans; // Bare-blind front and back diffuse-diffuse solar transmittance
                                Real64 tbshd = BackDiffDiffTrans;
                                Real64 rfshd = FrontDiffDiffRefl; // Bare-blind front and back diffuse solar reflectance
                                Real64 rbshd = BackDiffDiffRefl;
                                Real64 afshd = FrontDiffAbs;
                                Real64 abshd = BackDiffAbs;

                                Real64 AbsShade = 0.0;
                                Real64 AbsShadeDiff = 0.0;
                                if (NGlass == 2) {
                                    AbWinSh(1) = CosInc * FracSunLit *
                                                 (af1 + t1 * tfshBB * rf2 * tbshBB * ab1 +
                                                  t1 * (rfshB + rfshB * rbd1 * rfshd + tfshBB * rf2 * tbshBd + tfshBd * rfd2 * tbshd) * abd1);
                                    ADiffWinSh(1) = afd1 + td1 * (rfshd + rfshd * rbd1 * rfshd + tfshd * rfd2 * tbshd) * abd1;
                                    AbWinSh(2) =
                                        CosInc * FracSunLit *
                                        (t1 * rfshB * af2 + t1 * (rfshB * rf2 * rbshd + tfshBd * (1 + rfd2 * rbshd) + rfshB * rbd1 * tfshd) * afd2);
                                    ADiffWinSh(2) = td1 * (tfshd * (1 + rfd2 * rbshd) + rfshd * rbd1 * tfshd) * afd2;
                                    AbsShade = t1 * (afshB + tfshBB * rf2 * abshB + tfshBd * rfd2 * abshd + rfshB * rbd1 * afshd);
                                    AbsShadeDiff = td1 * (afshd * (1 + rfshd * rbd1) + tfshd * rfd2 * abshd);
                                } else if (NGlass == 3) {
                                    Real64 t1t2 = t1 * t2; // t1*t2
                                    Real64 td1td2 = td1 * td2;
                                    Real64 af3 = Window::POLYF(CosInc, thisConstruct.afBareSolCoef(3));
                                    Real64 rf3 = Window::POLYF(CosInc, thisConstruct.rfBareSolCoef(3));
                                    Real64 afd3 = thisConstruct.afBareSolDiff(3);
                                    Real64 rfd3 = thisConstruct.rfBareSolDiff(3);
                                    Real64 td2 = thisConstruct.tBareSolDiff(2);
                                    AbWinSh(1) = CosInc * FracSunLit *
                                                 (af1 + t1 * rf2 * ab1 + t1t2 * tfshBB * rf3 * tbshBB * t2 * ab1 +
                                                  t1t2 * (rfshB * td2 + rfshB * rbd2 * rfshd * td2 + tfshBd * rfd3 * tbshd * td2) * abd1);
                                    ADiffWinSh(1) = afd1 + td1 * rbd2 * abd1 +
                                                    td1td2 *
                                                        (rfshd * (1 + rbd2 * rfshd + td2 * rbd1 * td2 * rfshd) +
                                                         tfshd * (rfd3 * tbshd + rfd3 * rbshd * rfd3 * tbshd)) *
                                                        td2 * abd1;
                                    AbWinSh(2) = CosInc * FracSunLit *
                                                 (t1 * af2 + t1t2 * (tfshBB * rf3 * tbshBB * ab2 + rfshB * td2 * rbd1 * afd2) +
                                                  t1t2 * (rfshB * (1 + rbd2 * rfshd) + tfshBB * rf3 * tbshBd + tfshBd * rfd3 * tbshd) * abd2);
                                    ADiffWinSh(2) = td1 * afd2 + td1td2 * rfshd * td2 * rbd1 * afd2 +
                                                    td1td2 * (rfshd * (1 + rbd2 * rfshd) + tfshd * rfd3 * tbshd) * abd2;
                                    AbWinSh(3) = CosInc * FracSunLit *
                                                 (t1t2 * tfshBB * af3 + t1t2 *
                                                                            (tfshBB * rf3 * rbshB + tfshBd * (1 + rfd3 * rbshd) +
                                                                             rfshB * (rbd2 * tfshd + td2 * rbd1 * td2 * tfshd)) *
                                                                            afd3);
                                    ADiffWinSh(3) = td1td2 * (tfshd * (1 + rfd3 * rbshd) + rfshd * (rbd2 * tfshd + td2 * rbd1 * td2 * tfshd)) * afd3;
                                    AbsShade = t1t2 * (afshB * (1 + tfshBB * rf3) + afshd * (tfshBd * rfd3 + rfshB * (rbd2 + td2 * rbd1 * td2)));
                                    AbsShadeDiff = td1td2 * (afshd + tfshd * rfd3 * abshd + rfshd * (rfd2 + td2 * rbd2 * td2) * afshd);
                                } // End of check if NGlass
                                state.dataSolarShading->SurfWinExtBeamAbsByShadFac(SurfNum) =
                                    AbsShade * CosInc * SunLitFract * InOutProjSLFracMult +
                                    s_surf->SurfWinOutsRevealDiffOntoGlazing(SurfNum) * AbsShadeDiff;
                            } // End of check if blind is interior, exterior or between-glass
                        } // End of check if a blind is on

                        if (ShadeFlag != WinShadingType::SwitchableGlazing) {
                            // Interior or between glass shade or blind on
                            for (int Lay = 1; Lay <= NGlass; ++Lay) {
                                s_surf->SurfWinA(SurfNum, Lay) = AbWinSh(Lay);
                                // Add contribution of diffuse from beam on outside reveal
                                if (ANY_INTERIOR_SHADE_BLIND(ShadeFlag) || ANY_BETWEENGLASS_SHADE_BLIND(ShadeFlag)) {
                                    s_surf->SurfWinA(SurfNum, Lay) += ADiffWinSh(Lay) * s_surf->SurfWinOutsRevealDiffOntoGlazing(SurfNum);
                                }
                            }
                        } else {
                            auto const &thisConstructSh = state.dataConstruction->Construct(ConstrNumSh);
                            // Switchable glazing
                            for (int Lay = 1; Lay <= NGlass; ++Lay) {
                                Real64 SwitchFac = s_surf->SurfWinSwitchingFactor(SurfNum);
                                Real64 ADiffWin = thisConstruct.AbsDiff(Lay);
                                s_surf->SurfWinA(SurfNum, Lay) = Window::InterpSw(SwitchFac, AbWin(Lay), AbWinSh(Lay));
                                // Add contribution of diffuse from beam on outside and inside reveal
                                s_surf->SurfWinA(SurfNum, Lay) +=
                                    Window::InterpSw(SwitchFac, ADiffWin, ADiffWinSh(Lay)) * s_surf->SurfWinOutsRevealDiffOntoGlazing(SurfNum) +
                                    Window::InterpSw(SwitchFac, thisConstruct.AbsDiffBack(Lay), thisConstructSh.AbsDiffBack(Lay)) *
                                        s_surf->SurfWinInsRevealDiffOntoGlazing(SurfNum);
                            }
                        }
                    }

                } else if (s_surf->SurfWinWindowModelType(SurfNum) == WindowModel::BSDF) {
                    // Do not read from schedule file here since this will be called only if direct beam is hitting the window and schedule
                    // will not be loaded in that case even if diffuse part of solar radiation is entering through the window
                    if (FenSolAbsPtr == 0) {
                        // Put in the equivalent layer absorptions
                        // Simon: This should not be multiplied with CosInc since Abs coefficient already includes angular
                        // factor
                        for (int Lay = 1;
                             Lay <= s_surf->SurfaceWindow(SurfNum).ComplexFen.State(s_surf->SurfaceWindow(SurfNum).ComplexFen.CurrentState).NLayers;
                             ++Lay) {
                            Real64 absBeamWin = s_surf->SurfaceWindow(SurfNum)
                                                    .ComplexFen.State(s_surf->SurfaceWindow(SurfNum).ComplexFen.CurrentState)
                                                    .WinBmFtAbs(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, Lay);
                            Real64 AbWin =
                                absBeamWin * CosInc * SunLitFract * s_surf->SurfaceWindow(SurfNum).OutProjSLFracMult[state.dataGlobal->HourOfDay];

                            // Add contribution of beam reflected from outside and inside reveal
                            s_surf->SurfWinA(SurfNum, Lay) = AbWin +
                                                             s_surf->SurfWinOutsRevealDiffOntoGlazing(SurfNum) *
                                                                 s_surf->SurfaceWindow(SurfNum)
                                                                     .ComplexFen.State(s_surf->SurfaceWindow(SurfNum).ComplexFen.CurrentState)
                                                                     .WinFtHemAbs(Lay) +
                                                             s_surf->SurfWinInsRevealDiffOntoGlazing(SurfNum) *
                                                                 s_surf->SurfaceWindow(SurfNum)
                                                                     .ComplexFen.State(s_surf->SurfaceWindow(SurfNum).ComplexFen.CurrentState)
                                                                     .WinBkHemAbs(Lay);
                        }
                    }

                } else if (s_surf->SurfWinWindowModelType(SurfNum) == WindowModel::EQL) {
                    // call the ASHWAT fenestration model for optical properties
                    // determine the beam radiation absorptance and transmittance of the
                    // the equivalent layer window model
                    WindowEquivalentLayer::CalcEQLOpticalProperty(state, SurfNum, SolarArrays::BEAM, state.dataSolarShading->SurfWinAbsSolBeamEQL);
                    auto &CFS = state.dataWindowEquivLayer->CFS;
                    // recalculate the diffuse absorptance and transmittance of the
                    // the equivalent layer window model if there is shade control
                    int EQLNum = state.dataConstruction->Construct(surf.Construction).EQLConsPtr; // equivalent layer fenestration index
                    if (CFS(EQLNum).ISControlled) {
                        WindowEquivalentLayer::CalcEQLOpticalProperty(
                            state, SurfNum, SolarArrays::DIFF, state.dataSolarShading->SurfWinAbsSolDiffEQL);
                    } else {
                        state.dataSolarShading->SurfWinAbsSolDiffEQL(_, {1, CFS(EQLNum).NL + 1}) =
                            state.dataWindowEquivalentLayer->CFSDiffAbsTrans(_, {1, CFS(EQLNum).NL + 1}, EQLNum);
                    }
                    thisConstruct.TransDiff = state.dataSolarShading->SurfWinAbsSolDiffEQL(1, CFS(EQLNum).NL + 1);

                    for (int Lay = 1; Lay <= CFS(EQLNum).NL + 1; ++Lay) {
                        // Factor for front beam radiation absorbed for equivalent layer window model
                        Real64 AbWinEQL = state.dataSolarShading->SurfWinAbsSolBeamEQL(1, Lay) * CosInc * SunLitFract *
                                          s_surf->SurfaceWindow(SurfNum).InOutProjSLFracMult[state.dataGlobal->HourOfDay];
                        ;
                        if (CFS(EQLNum).L(1).LTYPE != LayerType::GLAZE) {
                            // if the first layer is not glazing (or it is a shade) do not
                            s_surf->SurfWinA(SurfNum, Lay) = AbWinEQL;
                        } else {
                            // the first layer is a glazing, include the outside reveal reflection
                            // and the inside reveal reflection until indoor shade layer is encountered.
                            if (CFS(EQLNum).L(Lay).LTYPE == LayerType::GLAZE) {
                                s_surf->SurfWinA(SurfNum, Lay) =
                                    AbWinEQL +
                                    s_surf->SurfWinOutsRevealDiffOntoGlazing(SurfNum) * state.dataSolarShading->SurfWinAbsSolBeamEQL(1, Lay) +
                                    s_surf->SurfWinInsRevealDiffOntoGlazing(SurfNum) * state.dataSolarShading->SurfWinAbsSolDiffEQL(2, Lay);
                            } else {
                                s_surf->SurfWinA(SurfNum, Lay) = AbWinEQL + s_surf->SurfWinOutsRevealDiffOntoGlazing(SurfNum) *
                                                                                state.dataSolarShading->SurfWinAbsSolBeamEQL(1, Lay);
                            }
                        }
                    }
                }
            } // End of SunlitFrac check

            //-----------------------------------------------------------------
            // BLOCK 2
            // SKY AND GROUND DIFFUSE SOLAR GAIN INTO ZONE FROM EXTERIOR WINDOW
            //-----------------------------------------------------------------

            Real64 SkySolarInc =
                s_surf->SurfSkySolarInc(SurfNum); // Incident solar radiation on a window: sky diffuse plus beam reflected from obstruction (W/m2)
            Real64 DiffTrans = 0.0;               // Glazing diffuse solar transmittance (including shade/blind/switching, if present)
            Real64 DiffTransGnd;                  // Ground diffuse solar transmittance for glazing with blind with horiz. slats or complex fen
            Real64 DiffTransBmGnd;                // Complex fen: diffuse solar transmittance for ground-reflected beam radiation
            Real64 DiffTransSky;                  // Sky diffuse solar transmittance for glazing with blind with horiz. slats or complex fen
            Real64 NomDiffTrans = 0.0;

            if (s_surf->SurfWinWindowModelType(SurfNum) == WindowModel::BSDF) { // complex fenestration
                if (FenSolAbsPtr == 0) {
                    // Sky diffuse solar transmittance for glazing with blind with horiz. slats or complex fen
                    DiffTransSky =
                        s_surf->SurfaceWindow(SurfNum).ComplexFen.State(s_surf->SurfaceWindow(SurfNum).ComplexFen.CurrentState).WinSkyTrans;
                    // Ground diffuse solar transmittance for glazing with blind with horiz. slats or complex fen
                    DiffTransGnd =
                        s_surf->SurfaceWindow(SurfNum).ComplexFen.State(s_surf->SurfaceWindow(SurfNum).ComplexFen.CurrentState).WinSkyGndTrans;
                    // Complex fen: diffuse solar transmittance for ground-reflected beam radiation
                    DiffTransBmGnd = s_surf->SurfaceWindow(SurfNum)
                                         .ComplexFen.State(s_surf->SurfaceWindow(SurfNum).ComplexFen.CurrentState)
                                         .WinBmGndTrans(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep);
                    // Define the effective transmittance for total sky and ground radiation
                    if ((SkySolarInc + s_surf->SurfWinBmGndSolarInc(SurfNum) + s_surf->SurfWinSkyGndSolarInc(SurfNum)) != 0.0) {
                        DiffTrans = (SkySolarInc * DiffTransSky + s_surf->SurfWinBmGndSolarInc(SurfNum) * DiffTransBmGnd +
                                     s_surf->SurfWinSkyGndSolarInc(SurfNum) * DiffTransGnd) /
                                    (SkySolarInc + s_surf->SurfWinBmGndSolarInc(SurfNum) + s_surf->SurfWinSkyGndSolarInc(SurfNum));
                    }
                    // Also update the nominal diffuse transmittance
                    NomDiffTrans =
                        s_surf->SurfaceWindow(SurfNum).ComplexFen.State(s_surf->SurfaceWindow(SurfNum).ComplexFen.CurrentState).WinDiffTrans;
                    // Do not store in TransDiff because it is not used by BSDF and rest of the code uses it as flag for opaque
                    // surface incorrectly assuming wall heat transfer routines for windows.
                    // Construct( Surface( SurfNum ).Construction ).TransDiff = NomDiffTrans;
                }
            } else if (surf.OriginalClass == SurfaceClass::TDD_Diffuser) {
                DiffTrans = TransTDD(state, PipeNum, CosInc, Dayltg::RadType::SolarAniso);
            } else {
                DiffTrans = thisConstruct.TransDiff;
            }

            if (s_surf->SurfWinWindowModelType(SurfNum) == WindowModel::Detailed) {
                if (IS_SHADED_NO_GLARE_CTRL(s_surf->SurfWinShadingFlag(SurfNum))) {
                    auto const &constrSh = state.dataConstruction->Construct(ConstrNumSh);
                    if (ShadeFlag != WinShadingType::SwitchableGlazing) {
                        // Shade or blind
                        if (ANY_SHADE_SCREEN(ShadeFlag)) {
                            // Shade or screen
                            DiffTrans = constrSh.TransDiff;
                        } else {
                            // Blind
                            int slatIdxLo = surfShade.blind.slatAngIdxLo;
                            int slatIdxHi = surfShade.blind.slatAngIdxHi;
                            Real64 slatInterpFac = surfShade.blind.slatAngInterpFac;

                            auto const &btarLo = constrSh.blindTARs[slatIdxLo];
                            auto const &btarHi = constrSh.blindTARs[slatIdxHi];

                            DiffTrans = Interp(btarLo.Sol.Ft.Df.Tra, btarHi.Sol.Ft.Df.Tra, slatInterpFac);

                            // For blinds with horizontal slats, allow different diffuse/diffuse transmittance for
                            // ground and sky solar
                            auto const *matBlind = dynamic_cast<Material::MaterialBlind const *>(s_mat->materials(surfShade.blind.matNum));
                            assert(matBlind != nullptr);
                            if (matBlind->SlatOrientation == DataWindowEquivalentLayer::Orientation::Horizontal) {
                                DiffTransGnd = Interp(btarLo.Sol.Ft.Df.TraGnd, btarHi.Sol.Ft.Df.TraGnd, slatInterpFac);
                                DiffTransSky = Interp(btarLo.Sol.Ft.Df.TraSky, btarHi.Sol.Ft.Df.TraSky, slatInterpFac);
                            }
                        }

                    } else { // Switchable glazing

                        Real64 SwitchFac = s_surf->SurfWinSwitchingFactor(SurfNum); // Switching factor for a window
                        DiffTrans = Window::InterpSw(SwitchFac, thisConstruct.TransDiff, constrSh.TransDiff);
                    }
                }
            }

            // Reporting variables
            if (s_surf->SurfWinWindowModelType(SurfNum) != WindowModel::EQL) {
                s_surf->SurfWinBlGlSysTsolDifDif(SurfNum) = DiffTrans;
                s_surf->SurfWinScGlSysTsolDifDif(SurfNum) = DiffTrans;
                if (ANY_BLIND(ShadeFlag) || ShadeFlag == WinShadingType::ExtScreen) {
                    s_surf->SurfWinBlGlSysTsolDifDif(SurfNum) = DiffTrans;
                    s_surf->SurfWinScGlSysTsolDifDif(SurfNum) = DiffTrans;
                    if (ShadeFlag == WinShadingType::ExtScreen) {
                        auto const *screen = dynamic_cast<Material::MaterialScreen const *>(s_mat->materials(ScNum));
                        assert(screen != nullptr);
                        s_surf->SurfWinScTsolDifDif(SurfNum) = screen->DfTrans;
                    } else {
                        s_surf->SurfWinBlTsolDifDif(SurfNum) = FrontDiffDiffTrans;
                    }
                }
            }

            //-----------------------------------------------------------------
            // BEAM SOLAR ON EXTERIOR WINDOW TRANSMITTED AS BEAM AND/OR DIFFUSE
            //-----------------------------------------------------------------
            Real64 TBmBm = 0.0;        // Beam-beam solar transmittance for bare window or window with switchable glazing
            Real64 TBmDif = 0.0;       // Beam-diffuse solar transmittance for bare window with diffusing glass
            Real64 TBmAllShBlSc = 0.0; // Beam-beam + beam-diffuse transmittance for window with shade, blind, screen, or switchable glazing
            Real64 TBmBmShBlSc = 0.0;  // Beam-beam transmittance for window with shade, blind, screen, or switchable glazing
            Real64 TBmDifShBlSc = 0.0; // Beam-diffuse transmittance for window with shade, blind, screen, or switchable glazing
            Real64 TBmBmBl;            // Beam-beam transmittance for window with blind
            Real64 TBmBmSc;            // Beam-beam transmittance for window with screen
            Real64 TDifBare;           // Bare diffuse transmittance of exterior window
            // Beam-beam transmittance for bare exterior window
            if (SunLitFract > 0.0) {
                if (surf.OriginalClass == SurfaceClass::TDD_Diffuser) {
                    TBmDif = TransTDD(state, PipeNum, CosInc, Dayltg::RadType::SolarBeam);
                    state.dataDaylightingDevicesData->TDDPipe(PipeNum).TransSolBeam = TBmDif;  // Report variable
                } else if (s_surf->SurfWinWindowModelType(SurfNum) == WindowModel::Detailed) { // Regular window
                    if (!s_surf->SurfWinSolarDiffusing(SurfNum)) {                             // Clear glazing
                        TBmBm = Window::POLYF(CosInc, thisConstruct.TransSolBeamCoef);         //[-]
                    } else {                                                                   // Diffusing glazing
                        TBmDif = Window::POLYF(CosInc, thisConstruct.TransSolBeamCoef);        //[-]
                    }
                } else if (s_surf->SurfWinWindowModelType(SurfNum) == WindowModel::BSDF) {
                    // Need to check what effect, if any, defining these here has
                    TBmBm = s_surf->SurfaceWindow(SurfNum)
                                .ComplexFen.State(s_surf->SurfaceWindow(SurfNum).ComplexFen.CurrentState)
                                .WinDirSpecTrans(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep);
                    TBmDif = s_surf->SurfaceWindow(SurfNum)
                                 .ComplexFen.State(s_surf->SurfaceWindow(SurfNum).ComplexFen.CurrentState)
                                 .WinDirHemiTrans(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep) -
                             TBmBm;
                } else if (s_surf->SurfWinWindowModelType(SurfNum) == WindowModel::EQL) {
                    // get ASHWAT fenestration model beam-beam and beam-diffuse properties
                    int EQLNum = state.dataConstruction->Construct(surf.Construction).EQLConsPtr; // equivalent layer fenestration index
                    Real64 TBmBmEQL = state.dataSolarShading->SurfWinAbsSolBeamEQL(1, state.dataWindowEquivLayer->CFS(EQLNum).NL + 1);
                    // Beam-diffuse transmittance
                    Real64 TBmDiffEQL = max(0.0, state.dataSolarShading->SurfWinAbsSolBeamEQL(2, state.dataWindowEquivLayer->CFS(EQLNum).NL + 1));
                    // Beam-beam transmittance: difference between beam-total and beam-diffuse transmittance
                    TBmBmEQL = max(0.0, (TBmBmEQL - TBmDiffEQL));
                    TBmBm = TBmBmEQL;
                    TBmDif = TBmDiffEQL;
                }
            }
            // Diffuse-diffuse transmittance for bare exterior window
            if (surf.OriginalClass == SurfaceClass::TDD_Diffuser) {
                TDifBare = TransTDD(state, PipeNum, CosInc, Dayltg::RadType::SolarAniso);
            } else {
                if (s_surf->SurfWinWindowModelType(SurfNum) == WindowModel::BSDF) {
                    // Complex Fenestration: use hemispherical ave of directional-hemispherical transmittance
                    // Note: this is not quite the same as the effective transmittance for total of sky and ground radiation
                    TDifBare = s_surf->SurfaceWindow(SurfNum).ComplexFen.State(s_surf->SurfaceWindow(SurfNum).ComplexFen.CurrentState).WinDiffTrans;
                } else { // Regular window
                    TDifBare = thisConstruct.TransDiff;
                }
            }

            //-----------------------------------------------------------------
            // BLOCK 3 - SCREEN, BLINDS AND GLAZING SYSTEM BEAM SOLAR TRANSMITTANCE
            //-----------------------------------------------------------------
            if (ConstrNumSh != 0 && SunLitFract > 0.0) {
                auto const &thisConstructSh = state.dataConstruction->Construct(ConstrNumSh);
                if (s_surf->SurfWinWindowModelType(SurfNum) != WindowModel::EQL) {
                    if (IS_SHADED_NO_GLARE_CTRL(ShadeFlag)) {
                        // Shade or screen or blind on, or switchable glazing
                        // (note in the following that diffusing glass is not oallowed in a window with shade, blind or switchable glazing)
                        if (ANY_SHADE(ShadeFlag) || ShadeFlag == WinShadingType::SwitchableGlazing) {
                            // Shade on or switchable glazing
                            TBmAllShBlSc = Window::POLYF(CosInc, thisConstructSh.TransSolBeamCoef);
                        } else {
                            // Blind or Screen on
                            Real64 TScBmDif;  // Beam-diffuse solar transmittance of screen
                            Real64 TBlBmDif;  // Beam-diffuse solar transmittance of blind
                            Real64 TBlDifDif; // Diffuse-diffuse solar transmittance of blind
                            Real64 TScBmBm;
                            Real64 TBlBmBm;
                            if (ShadeFlag == WinShadingType::ExtScreen) { // Exterior screen
                                auto const *screen = dynamic_cast<Material::MaterialScreen const *>(s_mat->materials(ScNum));
                                assert(screen != nullptr);

                                auto &surf = s_surf->Surface(SurfNum);
                                Real64 solPhi = std::acos(state.dataEnvrn->SOLCOS.z);
                                Real64 solTheta = std::atan2(state.dataEnvrn->SOLCOS.x, state.dataEnvrn->SOLCOS.y);
                                Real64 winPhi = surf.Tilt * Constant::DegToRad;
                                Real64 winTheta = surf.Azimuth * Constant::DegToRad;
                                Real64 phi = std::abs(solPhi - winPhi);
                                Real64 theta = std::abs(solTheta - winTheta);
                                Material::NormalizePhiTheta(phi, theta);
#ifdef PRECALC_INTERP_SCREEN
                                int ip1, ip2, it1, it2;
                                BilinearInterpCoeffs coeffs;
                                Material::GetPhiThetaIndices(phi, theta, screen->dPhi, screen->dTheta, ip1, ip2, it1, it2);
                                GetBilinearInterpCoeffs(
                                    phi, theta, ip1 * screen->dPhi, ip2 * screen->dPhi, it1 * screen->dTheta, it2 * screen->dTheta, coeffs);
                                auto const &b11 = screen->btars[ip1][it1];
                                auto const &b12 = screen->btars[ip1][it2];
                                auto const &b21 = screen->btars[ip2][it1];
                                auto const &b22 = screen->btars[ip2][it2];

                                Real64 RScBack = BilinearInterp(b11.RefSolFront, b12.RefSolFront, b21.RefSolFront, b22.RefSolFront, coeffs);
                                Real64 RScDifBack = screen->DfRef;

                                Real64 RGlBmFr = Window::POLYF(CosInc, thisConstruct.ReflSolBeamFrontCoef); // Beam front reflectance of glass
                                Real64 RGlDifFr = thisConstruct.ReflectSolDiffFront;                        // Diffuse front reflectance of glass
                                // beam transmittance (written in subroutine CalcScreenTransmittance each time step)
                                TScBmBm = BilinearInterp(b11.BmTrans, b12.BmTrans, b21.BmTrans, b22.BmTrans, coeffs);
                                TBmBmSc = TBmBm * TScBmBm;
                                TScBmDif = BilinearInterp(b11.DfTrans, b12.DfTrans, b21.DfTrans, b22.DfTrans, coeffs);
                                // beam-beam and diffuse transmittance of exterior beam
                                TBmAllShBlSc = TScBmBm * (TBmBm + RGlBmFr * RScBack * TDifBare / (1 - RGlDifFr * RScDifBack)) +
                                               TScBmDif * TDifBare / (1 - RGlDifFr * RScDifBack);
                                TBmBmShBlSc = TBmBmSc;
                                TBmDifShBlSc = TBmAllShBlSc - TBmBmShBlSc;
                                // Report variable for Beam-to-Diffuse transmittance (scattered transmittance)
                                s_surf->SurfWinScGlSysTsolBmBm(SurfNum) = TBmBmSc;
                                s_surf->SurfWinScTsolBmBm(SurfNum) = TScBmBm;
                                s_surf->SurfWinScTsolBmDif(SurfNum) = TScBmDif;
#else  // !PRECALC_INTERP_SCREEN
                                Material::ScreenBmTransAbsRef btar;
                                CalcScreenTransmittance(state, screen, phi, theta, btar);

                                Real64 RScBack = btar.RefSolFront;
                                Real64 RScDifBack = screen->DfRef;

                                Real64 RGlBmFr = Window::POLYF(CosInc, thisConstruct.ReflSolBeamFrontCoef); // Beam front reflectance of glass
                                Real64 RGlDifFr = thisConstruct.ReflectSolDiffFront;                        // Diffuse front reflectance of glass
                                // beam transmittance (written in subroutine CalcScreenTransmittance each time step)
                                TScBmBm = btar.BmTrans;
                                TBmBmSc = TBmBm * TScBmBm;
                                TScBmDif = btar.DfTrans;
                                // beam-beam and diffuse transmittance of exterior beam
                                TBmAllShBlSc = TScBmBm * (TBmBm + RGlBmFr * RScBack * TDifBare / (1 - RGlDifFr * RScDifBack)) +
                                               TScBmDif * TDifBare / (1 - RGlDifFr * RScDifBack);
                                TBmBmShBlSc = TBmBmSc;
                                TBmDifShBlSc = TBmAllShBlSc - TBmBmShBlSc;
                                // Report variable for Beam-to-Diffuse transmittance (scattered transmittance)
                                s_surf->SurfWinScGlSysTsolBmBm(SurfNum) = TBmBmSc;
                                s_surf->SurfWinScTsolBmBm(SurfNum) = TScBmBm;
                                s_surf->SurfWinScTsolBmDif(SurfNum) = TScBmDif;
#endif // PRECALC_INTERP_SCREEN
                            } else {
                                TBlBmBm = surfShade.blind.bmBmTrans;
                                TBlBmDif = FrontBeamDiffTrans;
                                if (ShadeFlag == WinShadingType::IntBlind) {
                                    Real64 RhoBlBmDifFr = FrontBeamDiffRefl;            // Beam-diffuse front reflectance of blind
                                    Real64 RGlDifBk = thisConstruct.ReflectSolDiffBack; // Diffuse front reflectance of glass
                                    Real64 RhoBlDifDifFr = FrontDiffDiffRefl;           // Diffuse-diffuse front reflectance of blind
                                    // beam-beam and diffuse transmittance of exterior beam
                                    TBmBmBl = TBmBm * TBlBmBm;
                                    TBlDifDif = FrontDiffDiffTrans;
                                    TBmAllShBlSc =
                                        TBmBm * (TBlBmBm + TBlBmDif + TBlDifDif * RhoBlBmDifFr * RGlDifBk / (1 - RhoBlDifDifFr * RGlDifBk));
                                    TBmBmShBlSc = TBmBmBl; // TBmBm * TBlBmBm
                                    TBmDifShBlSc = TBmAllShBlSc - TBmBmShBlSc;
                                    if (TBmDifShBlSc < 0.0) {
                                        TBmDifShBlSc = 0.0;
                                    }
                                } else if (ShadeFlag == WinShadingType::ExtBlind) {
                                    Real64 RhoBlBmDifBk = BackBeamDiffRefl;  // Beam-diffuse back reflectance of blind
                                    Real64 RhoBlDifDifBk = BackDiffDiffRefl; // Diffuse-diffuse back reflectance of blind
                                    Real64 RGlBmFr = Window::POLYF(CosInc, thisConstruct.ReflSolBeamFrontCoef);
                                    Real64 RGlDifFr = thisConstruct.ReflectSolDiffFront;
                                    // beam-beam and diffuse transmittance of exterior beam
                                    TBmBmBl = TBmBm * TBlBmBm;
                                    TBmAllShBlSc = TBlBmBm * (TBmBm + TDifBare * RGlBmFr * RhoBlBmDifBk / (1 - RGlDifFr * RhoBlDifDifBk)) +
                                                   TBlBmDif * TDifBare / (1 - RGlDifFr * RhoBlDifDifBk);
                                    TBmBmShBlSc = TBmBmBl; // TBmBm * TBlBmBm
                                    TBmDifShBlSc = TBmAllShBlSc - TBmBmShBlSc;
                                } else {
                                    // Between-glass blind on
                                    int NGlass = thisConstruct.TotGlassLayers;
                                    Real64 td2 = thisConstruct.tBareSolDiff(2);
                                    Real64 rbd1 = thisConstruct.rbBareSolDiff(1);
                                    Real64 rbshB = BackBeamDiffRefl;
                                    Real64 rfshd = FrontDiffDiffRefl;
                                    Real64 rbshd = BackDiffDiffRefl;
                                    Real64 tfshBd = FrontBeamDiffTrans;
                                    Real64 t1 = Window::POLYF(CosInc, thisConstruct.tBareSolCoef(1));
                                    Real64 t2 = Window::POLYF(CosInc, thisConstruct.tBareSolCoef(2));
                                    Real64 tfshBB = surfShade.blind.bmBmTrans;
                                    if (NGlass == 2) {
                                        Real64 rf2 = Window::POLYF(CosInc, thisConstruct.rfBareSolCoef(2));
                                        Real64 rfshB = FrontBeamDiffRefl;
                                        Real64 rfd2 = thisConstruct.rfBareSolDiff(2);
                                        TBmBmBl = t1 * tfshBB * t2;
                                        TBmAllShBlSc = t1 * tfshBB * t2 +
                                                       t1 * (tfshBB * rf2 * rbshB + tfshBd * (1.0 + rfd2 * rbshd) + rfshB * rbd1 * rfshd) * td2;
                                    } else { // NGlass = 3
                                        Real64 t1t2 = t1 * t2;
                                        Real64 t3 = Window::POLYF(CosInc, thisConstruct.tBareSolCoef(3));
                                        Real64 td3 = thisConstruct.tBareSolDiff(3);
                                        Real64 rf3 = Window::POLYF(CosInc, thisConstruct.rfBareSolCoef(3));
                                        Real64 rbd2 = thisConstruct.rbBareSolDiff(2);
                                        Real64 rfd3 = thisConstruct.rfBareSolDiff(3);
                                        Real64 tfshd = FrontDiffDiffTrans;
                                        TBmBmBl = t1 * t2 * tfshBB * t3;
                                        TBmAllShBlSc = t1t2 * tfshBB * t3 + t1t2 *
                                                                                (tfshBB * rf3 * rbshB + tfshBd * (1.0 + rfd3 * rbshd) +
                                                                                 rbshB * (rbd2 * tfshd + td2 * rbd1 * td2 * tfshd)) *
                                                                                td3;
                                    }
                                    // added TH 12/9/2009
                                    TBmBmShBlSc = TBmBmBl;
                                    TBmDifShBlSc = TBmAllShBlSc - TBmBmShBlSc;
                                }
                                s_surf->SurfWinBlTsolBmBm(SurfNum) = TBlBmBm;
                                s_surf->SurfWinBlTsolBmDif(SurfNum) = TBlBmDif;
                                s_surf->SurfWinBlGlSysTsolBmBm(SurfNum) = TBmBmBl;
                            }
                        }
                    } // End of check if ShadeFlag > 0 and ShadeFlag < 10
                } // end of checking if not eql window model
            } // end of checking if sunlitfract > 0

            if (ShadeFlag == WinShadingType::SwitchableGlazing) {
                // Switchable glazing
                Real64 SwitchFac = s_surf->SurfWinSwitchingFactor(SurfNum);
                if (!s_surf->SurfWinSolarDiffusing(SurfNum)) {
                    TBmBm = Window::InterpSw(SwitchFac, TBmBm, TBmAllShBlSc);
                } else {
                    TBmDif = Window::InterpSw(SwitchFac, TBmDif, TBmAllShBlSc);
                }
            }
            // Report variables
            s_surf->SurfWinGlTsolBmBm(SurfNum) = TBmBm;
            s_surf->SurfWinGlTsolBmDif(SurfNum) = TBmDif;
            s_surf->SurfWinGlTsolDifDif(SurfNum) = TDifBare;

            //-----------------------------------------------------------------
            // BLOCK 4 - REPORT WINDOW TRANSMITTANCE
            //-----------------------------------------------------------------

            // The following SurfWinTransBmSolar and SurfWinTransDifSolar will be combined later to give
            // WinTransSolar for reporting
            state.dataSolarShading->SurfWinTransBmSolar(SurfNum) = 0.0;
            state.dataSolarShading->SurfWinTransDifSolar(SurfNum) = 0.0;
            state.dataSolarShading->SurfWinTransDifSolarGnd(SurfNum) = 0.0;
            state.dataSolarShading->SurfWinTransDifSolarSky(SurfNum) = 0.0;
            state.dataSolarShading->SurfWinTransBmBmSolar(SurfNum) =
                0.0; // Factor for exterior beam to beam solar transmitted through window, or window plus shade, into zone at current time (m2)
            state.dataSolarShading->SurfWinTransBmDifSolar(SurfNum) =
                0.0; // Factor for exterior beam to diffuse solar transmitted through window, or window plus shade, into zone at current time (m2)

            Real64 InOutProjSLFracMult = s_surf->SurfaceWindow(SurfNum).InOutProjSLFracMult[state.dataGlobal->HourOfDay];
            if (s_surf->SurfWinWindowModelType(SurfNum) != WindowModel::EQL) {
                state.dataSolarShading->SurfWinTransDifSolar(SurfNum) = DiffTrans * surf.Area;
                if (ANY_BLIND(ShadeFlag)) {
                    auto const *matBlind = dynamic_cast<Material::MaterialBlind const *>(s_mat->materials(surfShade.blind.matNum));
                    assert(matBlind != nullptr);
                    if (matBlind->SlatOrientation == DataWindowEquivalentLayer::Orientation::Horizontal) {
                        state.dataSolarShading->SurfWinTransDifSolarGnd(SurfNum) = DiffTransGnd * surf.Area;
                        state.dataSolarShading->SurfWinTransDifSolarSky(SurfNum) = DiffTransSky * surf.Area;
                    }
                }
            } else {
                // In equivalent layer window model system diffuse transmittance is based on unit
                // diffuse radiation flux, and hence doesn't distinguish between sky and
                // ground reflected diffuse radiations
                state.dataSolarShading->SurfWinTransDifSolar(SurfNum) = DiffTrans * surf.Area;
                state.dataSolarShading->SurfWinTransDifSolarGnd(SurfNum) = DiffTrans * surf.Area;
                state.dataSolarShading->SurfWinTransDifSolarSky(SurfNum) = DiffTrans * surf.Area;
            }

            if (!IS_SHADED_NO_GLARE_CTRL(ShadeFlag) || ShadeFlag == WinShadingType::SwitchableGlazing) {
                // Unshaded or switchable glazing
                // Note: with previous defs of TBmBm & TBmDif, these come out right for Complex Fenestration
                // SurfWinTransBmSolar uses the directional-hemispherical transmittance
                state.dataSolarShading->SurfWinTransBmSolar(SurfNum) = (TBmBm + TBmDif) * SunLitFract * CosInc * surf.Area * InOutProjSLFracMult;
                state.dataSolarShading->SurfWinTransBmBmSolar(SurfNum) = TBmBm * SunLitFract * CosInc * surf.Area * InOutProjSLFracMult;   // m2
                state.dataSolarShading->SurfWinTransBmDifSolar(SurfNum) = TBmDif * SunLitFract * CosInc * surf.Area * InOutProjSLFracMult; // m2

            } else {
                state.dataSolarShading->SurfWinTransBmSolar(SurfNum) = TBmAllShBlSc * SunLitFract * CosInc * surf.Area * InOutProjSLFracMult;
                state.dataSolarShading->SurfWinTransBmBmSolar(SurfNum) = TBmBmShBlSc * SunLitFract * CosInc * surf.Area * InOutProjSLFracMult;
                state.dataSolarShading->SurfWinTransBmDifSolar(SurfNum) = TBmDifShBlSc * SunLitFract * CosInc * surf.Area * InOutProjSLFracMult;
            }

            // Add diffuse transmitted by window from beam reflected from outside reveal
            if (s_surf->SurfWinWindowModelType(SurfNum) == WindowModel::BSDF) { // Complex Fenestration
                if (FenSolAbsPtr == 0) {
                    state.dataSolarShading->SurfWinTransBmSolar(SurfNum) = (TBmBm + TBmDif) * SunLitFract * CosInc * surf.Area * InOutProjSLFracMult;
                    state.dataSolarShading->SurfWinTransBmBmSolar(SurfNum) = TBmBm * SunLitFract * CosInc * surf.Area * InOutProjSLFracMult;   // m2
                    state.dataSolarShading->SurfWinTransBmDifSolar(SurfNum) = TBmDif * SunLitFract * CosInc * surf.Area * InOutProjSLFracMult; // m2
                    state.dataSolarShading->SurfWinTransBmSolar(SurfNum) +=
                        s_surf->SurfWinOutsRevealDiffOntoGlazing(SurfNum) * NomDiffTrans * surf.Area;
                    state.dataSolarShading->SurfWinTransBmDifSolar(SurfNum) +=
                        s_surf->SurfWinOutsRevealDiffOntoGlazing(SurfNum) * NomDiffTrans * surf.Area;
                } else {
                    state.dataSolarShading->SurfWinTransBmSolar(SurfNum) = 0.0;
                    state.dataSolarShading->SurfWinTransBmDifSolar(SurfNum) = 0.0;
                }
            } else { // Regular window
                // this is also valid for equivalent layer window
                state.dataSolarShading->SurfWinTransBmSolar(SurfNum) += s_surf->SurfWinOutsRevealDiffOntoGlazing(SurfNum) * DiffTrans * surf.Area;
                state.dataSolarShading->SurfWinTransBmDifSolar(SurfNum) += s_surf->SurfWinOutsRevealDiffOntoGlazing(SurfNum) * DiffTrans * surf.Area;
            }

            //-----------------------------------------------------------------
            // BLOCK 5 - UPDATE SOLAR ENTERING A ZONE AS BEAM OR DIFFUSE RADIATION
            //-----------------------------------------------------------------
            // Increment factor for total exterior beam solar entering zone through window as beam or diffuse
            if (SunLitFract > 0.0 && surf.Class != SurfaceClass::TDD_Dome) {
                // Window is schedule surface gained. Do not make addition to what enters into zone since that information is not available
                if (FenSolAbsPtr == 0) {
                    Real64 TBmAll; // Window beam-to-(beam+diffuse) transmittance
                    if (s_surf->SurfWinWindowModelType(SurfNum) != WindowModel::BSDF && (ANY_BLIND(ShadeFlag) || ANY_SHADE_SCREEN(ShadeFlag))) {
                        TBmAll = TBmAllShBlSc;
                    } else {
                        TBmAll = TBmBm + TBmDif;
                    }
                    BTOTZone += surf.IncSolMultiplier * TBmAll * SunLitFract * CosInc * surf.Area * InOutProjSLFracMult; // [m2]
                }
            }

            // Correct for effect of (1) beam absorbed by inside reveal, (2) diffuse entering zone from beam
            // reflected by inside reveal and (3) diffuse transmitted by window from beam reflected from outside reveal.
            if (CosInc > 0.0) {
                // The BTOTZone is the solar into zone assuming no inside or outside reveals
                // The inside reveals receive solar (reflected part + absorbed part) from the window, this amount should be deducted from the
                // BTOTZone, then adds the InsRevealDiffIntoZone
                if (s_surf->SurfWinWindowModelType(SurfNum) == WindowModel::BSDF) { // Complex Fenestration
                    // Do not add total into zone from scheduled surface gains.  That will be added later
                    if (SurfaceScheduledSolarInc(state, SurfNum, ConstrNum) == 0) {
                        BTOTZone = BTOTZone - s_surf->SurfWinBmSolRefldInsReveal(SurfNum) - s_surf->SurfWinBmSolAbsdInsReveal(SurfNum) +
                                   s_surf->SurfWinInsRevealDiffIntoZone(SurfNum) +
                                   s_surf->SurfWinOutsRevealDiffOntoGlazing(SurfNum) * NomDiffTrans * surf.Area;
                    }
                } else { // Regular window
                    BTOTZone = BTOTZone - s_surf->SurfWinBmSolRefldInsReveal(SurfNum) - s_surf->SurfWinBmSolAbsdInsReveal(SurfNum) +
                               s_surf->SurfWinInsRevealDiffIntoZone(SurfNum) +
                               s_surf->SurfWinOutsRevealDiffOntoGlazing(SurfNum) * DiffTrans * surf.Area;
                }
                // Add beam solar absorbed by outside reveal to outside of window's base surface. Add beam solar absorbed by inside reveal to inside
                // of window's base surface. This ignores 2-D heat transfer effects.
                int BaseSurfNum = surf.BaseSurf;
                s_surf->SurfOpaqAI(BaseSurfNum) += s_surf->SurfWinBmSolAbsdInsReveal(SurfNum) / s_surf->Surface(BaseSurfNum).Area;
                s_surf->SurfOpaqAO(BaseSurfNum) += s_surf->SurfWinBmSolAbsdOutsReveal(SurfNum) / s_surf->Surface(BaseSurfNum).Area;
            }

            //-----------------------------------------------------------------
            // BLOCK 6 - INTERIOR BEAM FROM EXTERIOR WINDOW THAT IS ABSORBED/TRANSMITTED BY BACK SURFACES
            //-----------------------------------------------------------------

            // If shade is in place or there is a diffusing glass layer there is no interior beam
            // from this exterior window since the beam-beam transmittance of shades and diffusing glass
            // is assumed to be zero. The beam-beam transmittance of tubular daylighting devices is also
            // assumed to be zero.
            if (SunLitFract > 0.0) {
                if (s_surf->SurfWinWindowModelType(SurfNum) != WindowModel::BSDF) {
                    if (ANY_SHADE(ShadeFlag) || s_surf->SurfWinSolarDiffusing(SurfNum) || surf.OriginalClass == SurfaceClass::TDD_Diffuser ||
                        surf.Class == SurfaceClass::TDD_Dome) {
                        continue;
                    }
                }

                // Find interior beam radiation that is:
                // (1) absorbed by opaque back surfaces;
                // (2) absorbed by glass layers of back surfaces that are interior or exterior windows;
                // (3) absorbed by interior, exterior or between-glass shades or blinds of back surfaces
                //       that are exterior windows; and
                // (4) transmitted through back surfaces that are interior or exterior windows.
                // Beam-beam transmittance of exterior window
                Real64 TBm;      // Window beam-beam transmittance
                Real64 TBmDenom; // TBmDenominator
                Real64 TBmBmSc = s_surf->SurfWinScGlSysTsolBmBm(SurfNum);
                Real64 TBmBmBl = s_surf->SurfWinBlGlSysTsolBmBm(SurfNum);
                Real64 TBmBm = s_surf->SurfWinGlTsolBmBm(SurfNum);

                Real64 InOutProjSLFracMult = s_surf->SurfaceWindow(SurfNum).InOutProjSLFracMult[state.dataGlobal->HourOfDay];
                int InShelfSurf = 0; // Inside daylighting shelf surface number
                int ShelfNum = s_surf->SurfDaylightingShelfInd(SurfNum);
                if (ShelfNum > 0) { // Daylighting shelf
                    InShelfSurf = state.dataDaylightingDevicesData->Shelf(ShelfNum).InSurf;
                }
                if (ANY_BLIND(ShadeFlag)) {
                    TBm = TBmBmBl; // Interior, exterior or between-glass blind on
                } else if (ShadeFlag == WinShadingType::ExtScreen) {
                    TBm = TBmBmSc; // Exterior screen on
                } else {
                    TBm = TBmBm; // Bare glass or switchable glazing
                    // Correction for beam absorbed by inside reveal
                    TBmDenom = (SunLitFract * CosInc * surf.Area * InOutProjSLFracMult);
                    if (TBmDenom != 0.0) { // when =0.0, no correction
                        TBm -= s_surf->SurfWinBmSolAbsdInsReveal(SurfNum) / TBmDenom;
                    }
                    TBm = max(0.0, TBm);
                    // this multiplier doesn't work with other shading, so no need to apply in other branches
                    TBm *= surf.IncSolMultiplier;
                }

                if (TBm == 0.0) {
                    continue;
                }
                if (InShelfSurf > 0) { // Inside daylighting shelf
                    // Inside daylighting shelves assume that no beam will pass the end of the shelf.
                    // Since all beam is absorbed on the shelf, this might cause them to get unrealistically hot at times.
                    // BTOTWinZone - Transmitted beam solar factor for a window [m2]
                    Real64 BTOTWinZone = TBm * SunLitFract * surf.Area * CosInc * InOutProjSLFracMult;
                    // Shelf surface area is divided by 2 because only one side sees beam (Area was multiplied by 2 during init)
                    s_surf->SurfOpaqAI(InShelfSurf) += BTOTWinZone / (0.5 * s_surf->Surface(InShelfSurf).Area); //[-]
                    BABSZone += BTOTWinZone;                                                                    //[m2]
                    continue;
                }

                if (state.dataHeatBal->SolarDistribution == DataHeatBalance::Shadowing::FullInteriorExterior) { // Full interior solar distribution
                    if (s_surf->SurfWinWindowModelType(SurfNum) == WindowModel::Detailed) {
                        // Loop over back surfaces irradiated by beam from this exterior window
                        for (int IBack = 1; IBack <= state.dataBSDFWindow->MaxBkSurf; ++IBack) {
                            int BackSurfNum =
                                state.dataHeatBal->SurfWinBackSurfaces(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, IBack, SurfNum);
                            if (BackSurfNum == 0) {
                                break; // No more irradiated back surfaces for this exterior window
                            }

                            auto &surfShadeBack = s_surf->surfShades(BackSurfNum);
                            int ConstrNumBack = s_surf->SurfActiveConstruction(BackSurfNum);
                            auto const &constrBack = state.dataConstruction->Construct(ConstrNumBack);

                            int NBackGlass = constrBack.TotGlassLayers;
                            // Irradiated (overlap) area for this back surface, projected onto window plane
                            // (includes effect of shadowing on exterior window)
                            Real64 AOverlap =
                                state.dataHeatBal->SurfWinOverlapAreas(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, IBack, SurfNum);
                            // Back surface area irradiated by beam solar from an exterior window, projected onto window plane
                            Real64 BOverlap = TBm * AOverlap * CosInc; //[m2]
                            // AOverlap multiplied by exterior window beam transmittance and cosine of incidence angle
                            if (constrBack.TransDiff <= 0.0) {

                                // Back surface is opaque interior or exterior wall
                                // Interior solar absorptance of opaque surface
                                Real64 AbsIntSurf = state.dataHeatBalSurf->SurfAbsSolarInt(BackSurfNum);
                                s_surf->SurfOpaqAI(BackSurfNum) += BOverlap * AbsIntSurf / s_surf->Surface(BackSurfNum).Area; //[-]
                                BABSZone += BOverlap * AbsIntSurf;                                                            //[m2]

                            } else {

                                // Back surface is an interior or exterior window
                                // Note that exterior back windows can have a shading device but interior back windows
                                // are assumed to be bare, i.e., they have no shading device and are non-switchable.
                                // The layer order for interior windows is "outside" to "inside," where "outside" refers to
                                // the adjacent zone and "inside" refers to the current zone.
                                WinShadingType ShadeFlagBack = s_surf->SurfWinShadingFlag(BackSurfNum);
                                Real64 slatAngBack = surfShadeBack.blind.slatAng;
                                Real64 CosIncBack =
                                    std::abs(state.dataHeatBal->SurfCosIncAng(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, BackSurfNum));
                                if (s_surf->SurfWinWindowModelType(SurfNum) == WindowModel::BSDF) {
                                    // Transmitting window is complex fen, change the incident angle to one for ray joining
                                    // transmitting and back window centers
                                    CosIncBack = std::abs(state.dataBSDFWindow->ComplexWind(SurfNum).sdotN(IBack));
                                }
                                int ConstrNumBackShRaw = s_surf->SurfWinActiveShadedConstruction(BackSurfNum);
                                int const ConstrNumBackSh = ConstrNumBackShRaw == 0 ? ConstrNumBack : ConstrNumBackShRaw;
                                state.dataSolarShading->SurfWinAbsBeam.dimension(state.dataHeatBal->MaxSolidWinLayers, 0.0);
                                Real64 TransBeamWin = 0.0;           // Beam solar transmittance of a window
                                Real64 AbsBeamTotWin = 0.0;          // Sum of window glass layer beam solar absorptances
                                Real64 backSurfBeamSolInTrans = 0.0; // Fraction of BeamSolarRad transmitted out through window inside face [W]

                                // Interior beam absorptance of glass layers and beam transmittance of back exterior  &
                                // or interior window WITHOUT SHADING this timestep
                                if (NOT_SHADED(ShadeFlagBack)) {
                                    for (int Lay = 1; Lay <= NBackGlass; ++Lay) {
                                        state.dataSolarShading->SurfWinAbsBeam(Lay) = Window::POLYF(CosIncBack, constrBack.AbsBeamBackCoef(Lay));
                                    }
                                    TransBeamWin = Window::POLYF(CosIncBack, constrBack.TransSolBeamCoef);
                                }

                                // Interior beam absorptance of glass layers and beam transmittance
                                // of back exterior window with SHADE
                                if (ANY_SHADE(ShadeFlagBack)) {
                                    for (int Lay = 1; Lay <= state.dataConstruction->Construct(ConstrNumBackSh).TotGlassLayers; ++Lay) {
                                        state.dataSolarShading->SurfWinAbsBeam(Lay) =
                                            Window::POLYF(CosIncBack, state.dataConstruction->Construct(ConstrNumBackSh).AbsBeamBackCoef(Lay));
                                    }
                                    TransBeamWin = Window::POLYF(CosIncBack, state.dataConstruction->Construct(ConstrNumBackSh).TransSolBeamCoef);
                                }

                                // Interior beam absorbed by INTERIOR SHADE of back exterior window
                                if (ShadeFlagBack == WinShadingType::IntShade) {
                                    state.dataSolarShading->SurfWinIntBeamAbsByShadFac(BackSurfNum) =
                                        BOverlap * state.dataConstruction->Construct(ConstrNumBackSh).AbsDiffBackShade /
                                        (s_surf->Surface(BackSurfNum).Area + s_surf->SurfWinDividerArea(BackSurfNum));
                                    BABSZone += BOverlap * state.dataConstruction->Construct(ConstrNumBackSh).AbsDiffBackShade;
                                    backSurfBeamSolInTrans += BOverlap * state.dataConstruction->Construct(ConstrNumBackSh).AbsDiffBackShade;

                                    // Interior beam absorbed by EXTERIOR SHADE of back exterior window
                                } else if (ShadeFlagBack == WinShadingType::ExtShade) {
                                    Real64 RGlFront = constrBack.ReflectSolDiffFront;
                                    auto const *matSh = s_mat->materials(state.dataConstruction->Construct(ConstrNumBackSh).LayerPoint(1));
                                    auto const *matFenSh = dynamic_cast<Material::MaterialFen const *>(matSh);
                                    assert(matFenSh != nullptr);

                                    Real64 AbsSh = matFenSh->AbsorpSolar;
                                    Real64 RhoSh = 1.0 - AbsSh - matFenSh->Trans;
                                    Real64 AShBack = Window::POLYF(CosIncBack, constrBack.TransSolBeamCoef) * AbsSh / (1.0 - RGlFront * RhoSh);
                                    BABSZone += BOverlap * AShBack;
                                    backSurfBeamSolInTrans += BOverlap * AShBack;
                                    state.dataSolarShading->SurfWinIntBeamAbsByShadFac(BackSurfNum) =
                                        BOverlap * AShBack / (s_surf->Surface(BackSurfNum).Area + s_surf->SurfWinDividerArea(BackSurfNum));

                                    // Interior beam absorbed by BETWEEN-GLASS SHADE of back exterior window
                                } else if (ShadeFlagBack == WinShadingType::BGShade) {
                                    Real64 rbd1k = constrBack.rbBareSolDiff(1);
                                    Real64 rfd2k = constrBack.rfBareSolDiff(2);
                                    Real64 AShBack; // System shade absorptance for interior beam solar
                                    if (NBackGlass == 2) {
                                        Real64 t2k = Window::POLYF(CosIncBack, constrBack.tBareSolCoef(2));
                                        auto const *matShade = dynamic_cast<Material::MaterialShade const *>(
                                            s_mat->materials(state.dataConstruction->Construct(ConstrNumBackSh).LayerPoint(3)));
                                        assert(matShade != nullptr);
                                        Real64 TrSh = matShade->Trans;                       // Shade material solar transmittance
                                        Real64 RhoSh = matShade->ReflectShade;               // Shade material solar absorptance
                                        Real64 AbsSh = min(1.0, max(0.0, 1 - TrSh - RhoSh)); // Shade material solar absorptance
                                        AShBack = t2k * (1 + RhoSh * rfd2k + TrSh * rbd1k) * AbsSh;
                                    } else { // NBackGlass = 3
                                        Real64 t3k = Window::POLYF(CosIncBack, constrBack.tBareSolCoef(3));
                                        Real64 td2k = constrBack.tBareSolDiff(2);
                                        Real64 rbd2k = constrBack.rbBareSolDiff(2);
                                        Real64 rfd3k = constrBack.rfBareSolDiff(3);
                                        auto const *matShade = dynamic_cast<Material::MaterialShade const *>(
                                            s_mat->materials(state.dataConstruction->Construct(ConstrNumBackSh).LayerPoint(5)));
                                        assert(matShade != nullptr);
                                        Real64 TrSh = matShade->Trans;
                                        Real64 RhoSh = matShade->ReflectShade;
                                        Real64 AbsSh = min(1.0, max(0.0, 1 - TrSh - RhoSh));
                                        AShBack = t3k * (1 + RhoSh * rfd3k + TrSh * (rbd2k + td2k * rbd1k * td2k)) * AbsSh;
                                    }
                                    state.dataSolarShading->SurfWinIntBeamAbsByShadFac(BackSurfNum) =
                                        BOverlap * AShBack / s_surf->Surface(BackSurfNum).Area;
                                    BABSZone += BOverlap * AShBack;
                                    backSurfBeamSolInTrans += BOverlap * AShBack;
                                }

                                // Interior beam absorptance of glass layers and beam absorbed in blind
                                // of back exterior window with BLIND
                                if (ANY_BLIND(ShadeFlagBack)) {
                                    auto const &btarBack = surfShadeBack.blind.TAR;
                                    auto const *matBlindBack =
                                        dynamic_cast<Material::MaterialBlind const *>(s_mat->materials(surfShadeBack.blind.matNum));
                                    assert(matBlindBack != nullptr);

                                    auto const &btar = surfShade.blind.TAR;
                                    Real64 profAngBack = surfShadeBack.blind.profAng;

                                    int profIdxLoBack = surfShadeBack.blind.profAngIdxLo;
                                    int profIdxHiBack = surfShadeBack.blind.profAngIdxHi;
                                    Real64 profInterpFacBack = surfShadeBack.blind.profAngInterpFac;

                                    Real64 TGlBmBack = Window::POLYF(CosIncBack, constrBack.TransSolBeamCoef);
                                    Real64 TBlBmBmBack = matBlindBack->BeamBeamTrans(profAngBack, Constant::Pi - slatAngBack);

                                    // Blind solar back beam-diffuse transmittance
                                    // Is this supposed to be blindBack?
                                    Real64 TBlBmDiffBack =
                                        Interp(btar.Sol.Bk.Bm[profIdxLoBack].DfTra, btar.Sol.Bk.Bm[profIdxLoBack].DfTra, profInterpFacBack);

                                    if (ShadeFlagBack == WinShadingType::IntBlind) {
                                        // Interior beam absorptance of GLASS LAYERS of exterior back window with INTERIOR BLIND
                                        FrontDiffDiffRefl = btarBack.Sol.Ft.Df.Ref; // Blind solar front beam reflectance
                                        FrontDiffAbs = btarBack.Sol.Ft.Df.Abs;
                                        Real64 RhoBlFront =
                                            Interp(btar.Sol.Ft.Bm[profIdxLoBack].DfRef, btar.Sol.Ft.Bm[profIdxHiBack].DfRef, profInterpFacBack);
                                        Real64 AbsBlFront =
                                            Interp(btar.Sol.Ft.Bm[profIdxLoBack].Abs, btar.Sol.Ft.Bm[profIdxHiBack].Abs, profInterpFacBack);
                                        Real64 AbsBlBack =
                                            Interp(btar.Sol.Bk.Bm[profIdxLoBack].Abs, btar.Sol.Bk.Bm[profIdxHiBack].Abs, profInterpFacBack);

                                        Real64 RhoBlDiffFront = FrontDiffDiffRefl; // Glazing system solar back beam-beam reflectance
                                        Real64 RGlBack = Window::POLYF(
                                            CosIncBack, constrBack.ReflSolBeamBackCoef); // Glazing system back diffuse solar reflectance
                                        Real64 RGlDiffBack = constrBack.ReflectSolDiffBack;
                                        for (int Lay = 1; Lay <= NBackGlass; ++Lay) {
                                            Real64 AbWinBack = Window::POLYF(
                                                CosIncBack,
                                                constrBack.AbsBeamBackCoef(Lay)); // Factor for back beam radiation absorbed in window glass layer
                                            Real64 AGlDiffBack = constrBack.AbsDiffBack(Lay); // Glass layer back diffuse solar absorptance
                                            state.dataSolarShading->SurfWinAbsBeam(Lay) =
                                                TBlBmBmBack * AbWinBack + ((TBlBmBmBack * RGlBack * RhoBlFront + TBlBmDiffBack) * AGlDiffBack /
                                                                           (1.0 - RGlDiffBack * RhoBlDiffFront));
                                        }

                                        // Interior beam transmitted by exterior back window with INTERIOR BLIND
                                        Real64 TGlDif = constrBack.TransDiff; // Bare diffuse transmittance of back window
                                        TransBeamWin =
                                            TBlBmBmBack * (TGlBmBack + TGlDif * RGlBack * RhoBlFront / (1.0 - RGlDiffBack * RhoBlDiffFront)) +
                                            TBlBmDiffBack * TGlDif / (1.0 - RGlDiffBack * RhoBlDiffFront);

                                        // Interior beam absorbed by BLIND on exterior back window with INTERIOR BLIND

                                        Real64 AbsBlDiffFront = FrontDiffAbs; // Blind solar front diffuse absorptance
                                        Real64 ABlBack = AbsBlBack + TBlBmBmBack * RGlBack * AbsBlFront +
                                                         (AbsBlDiffFront * RGlDiffBack / (1 - RhoBlDiffFront * RGlDiffBack)) *
                                                             (RGlBack * TBlBmBmBack * RhoBlFront +
                                                              TBlBmDiffBack); // Blind solar back absorptance for interior solar
                                        state.dataSolarShading->SurfWinIntBeamAbsByShadFac(BackSurfNum) =
                                            BOverlap * ABlBack / (s_surf->Surface(BackSurfNum).Area + s_surf->SurfWinDividerArea(BackSurfNum));
                                        BABSZone += BOverlap * ABlBack;
                                        backSurfBeamSolInTrans += BOverlap * ABlBack;

                                    } else if (ShadeFlagBack == WinShadingType::ExtBlind) {

                                        // Interior beam absorptance of GLASS LAYERS of exterior back window with EXTERIOR BLIND

                                        Real64 RGlDiffFront = state.dataConstruction->Construct(ConstrNumBack)
                                                                  .ReflectSolDiffFront; // Glazing system front diffuse solar reflectance

                                        // Is this supposed to be beam-beam reflection?
                                        Real64 RhoBlBack =
                                            Interp(btar.Sol.Bk.Bm[profIdxLoBack].DfRef, btar.Sol.Bk.Bm[profIdxHiBack].DfRef, profInterpFacBack);
                                        Real64 RhoBlBmDifBk =
                                            Interp(btar.Sol.Bk.Bm[profIdxLoBack].DfRef, btar.Sol.Bk.Bm[profIdxHiBack].DfRef, profInterpFacBack);
                                        Real64 AbsBlBack =
                                            Interp(btar.Sol.Bk.Bm[profIdxLoBack].Abs, btar.Sol.Bk.Bm[profIdxHiBack].Abs, profInterpFacBack);

                                        for (int Lay = 1; Lay <= NBackGlass; ++Lay) {
                                            Real64 AbWinBack = Window::POLYF(CosIncBack, constrBack.AbsBeamBackCoef(Lay));
                                            Real64 AGlDiffFront = constrBack.AbsDiff(Lay);
                                            state.dataSolarShading->SurfWinAbsBeam(Lay) =
                                                AbWinBack + (TGlBmBack * AGlDiffFront * RhoBlBack / (1.0 - RhoBlBack * RGlDiffFront));
                                        }

                                        // Interior beam transmitted by exterior back window with EXTERIOR BLIND
                                        Real64 TBlDifDif = BackDiffDiffTrans;
                                        Real64 RhoBlDifDifBk = BackDiffDiffRefl;
                                        Real64 AbsBlDiffBack = BackDiffAbs;
                                        Real64 ABlBack =
                                            TGlBmBack * (AbsBlBack + RhoBlBack * RGlDiffFront * AbsBlDiffBack / (1 - RhoBlDifDifBk * RGlDiffFront));
                                        Real64 RGlDifFr = thisConstruct.ReflectSolDiffFront;
                                        TransBeamWin = TGlBmBack * (TBlBmBmBack + TBlBmDiffBack +
                                                                    TBlDifDif * RhoBlBmDifBk * RGlDifFr / (1.0 - RhoBlDifDifBk * RGlDifFr));
                                        // Interior beam absorbed by EXTERIOR BLIND on exterior back window
                                        BABSZone += BOverlap * ABlBack;
                                        backSurfBeamSolInTrans += BOverlap * ABlBack;
                                        state.dataSolarShading->SurfWinIntBeamAbsByShadFac(BackSurfNum) =
                                            BOverlap * ABlBack / (s_surf->Surface(BackSurfNum).Area + s_surf->SurfWinDividerArea(BackSurfNum));

                                    } else {
                                        // ShadeFlagBack == BGBlindOn
                                        Real64 t1k = Window::POLYF(CosIncBack, constrBack.tBareSolCoef(1));
                                        Real64 t2k = Window::POLYF(CosIncBack, constrBack.tBareSolCoef(2));
                                        Real64 af2k = Window::POLYF(CosIncBack, constrBack.afBareSolCoef(2));
                                        Real64 ab1k = Window::POLYF(CosIncBack, constrBack.abBareSolCoef(1));
                                        Real64 ab2k = Window::POLYF(CosIncBack, constrBack.abBareSolCoef(2));
                                        Real64 rb1k = Window::POLYF(CosIncBack, constrBack.rbBareSolCoef(1));
                                        Real64 rb2k = Window::POLYF(CosIncBack, constrBack.rbBareSolCoef(2));
                                        Real64 td1k = constrBack.tBareSolDiff(1);
                                        Real64 td2k = constrBack.tBareSolDiff(2);
                                        Real64 afd2k = constrBack.afBareSolDiff(2);
                                        Real64 abd1k = constrBack.abBareSolDiff(1);
                                        Real64 abd2k = constrBack.abBareSolDiff(2);
                                        Real64 rfd2k = constrBack.rfBareSolDiff(2);
                                        Real64 rbd1k = constrBack.rbBareSolDiff(1);
                                        Real64 rbd2k = constrBack.rbBareSolDiff(2);
                                        Real64 tfshBBk = matBlindBack->BeamBeamTrans(profAngBack, slatAngBack);
                                        Real64 tbshBBk = matBlindBack->BeamBeamTrans(profAngBack, Constant::Pi - slatAngBack);

                                        Real64 tfshBdk =
                                            Interp(btar.Sol.Ft.Bm[profIdxLoBack].DfTra, btar.Sol.Ft.Bm[profIdxHiBack].DfTra, profInterpFacBack);
                                        Real64 tbshBdk =
                                            Interp(btar.Sol.Bk.Bm[profIdxLoBack].DfTra, btar.Sol.Bk.Bm[profIdxHiBack].DfTra, profInterpFacBack);
                                        Real64 rfshBk =
                                            Interp(btar.Sol.Ft.Bm[profIdxLoBack].DfRef, btar.Sol.Ft.Bm[profIdxHiBack].DfRef, profInterpFacBack);
                                        Real64 rbshBk =
                                            Interp(btar.Sol.Bk.Bm[profIdxLoBack].DfRef, btar.Sol.Bk.Bm[profIdxHiBack].DfRef, profInterpFacBack);
                                        Real64 afshBk =
                                            Interp(btar.Sol.Ft.Bm[profIdxLoBack].Abs, btar.Sol.Ft.Bm[profIdxHiBack].Abs, profInterpFacBack);
                                        Real64 abshBk =
                                            Interp(btar.Sol.Bk.Bm[profIdxLoBack].Abs, btar.Sol.Bk.Bm[profIdxHiBack].Abs, profInterpFacBack);
                                        Real64 tfshdk = btarBack.Sol.Ft.Df.Tra;
                                        Real64 rfshdk = btarBack.Sol.Ft.Df.Ref;
                                        Real64 afshdk = btarBack.Sol.Ft.Df.Abs;
                                        Real64 tbshdk = btarBack.Sol.Bk.Df.Tra;
                                        Real64 rbshdk = btarBack.Sol.Bk.Df.Ref;
                                        Real64 abshdk = btarBack.Sol.Bk.Df.Abs;
                                        Real64 ABlBack;

                                        if (NBackGlass == 2) {
                                            // Interior beam absorptance of GLASS LAYERS of exterior back window with BETWEEN-GLASS BLIND
                                            state.dataSolarShading->SurfWinAbsBeam(2) =
                                                ab2k + t2k * tbshBBk * rb1k * tfshBBk * af2k +
                                                t2k * (tbshBBk * rb1k * tfshBdk + tbshBdk * rbd1k * tfshdk + rbshBk * (1.0 + rfd2k * rbshdk)) * afd2k;
                                            state.dataSolarShading->SurfWinAbsBeam(1) =
                                                t2k * tbshBBk * ab1k + t2k * (rbshBk * rfd2k * tbshdk + tbshBdk * (1.0 + rbd1k * rfshdk)) * abd1k;
                                            // Interior beam transmitted by exterior back window with BETWEEN-GLASS BLIND
                                            TransBeamWin =
                                                t2k * tbshBBk * t1k +
                                                t2k * (tbshBBk * rb1k * rfshBk + rbshBk * rfd2k * tbshdk + tbshBdk * (1.0 + rbd1k * rfshdk)) * td1k;
                                            // Interior beam absorbed by BLIND on exterior back window with BETWEEN-GLASS BLIND
                                            ABlBack = t2k * (abshBk + tbshBBk * rb1k * afshBk + rbshBk * rfd2k * abshdk + tbshBdk * rbd1k * afshdk);
                                        } else { // NBackGlass = 3
                                            Real64 t3k = Window::POLYF(CosIncBack, constrBack.tBareSolCoef(3));
                                            Real64 af3k = Window::POLYF(CosIncBack, constrBack.afBareSolCoef(3));
                                            Real64 ab3k = Window::POLYF(CosIncBack, constrBack.abBareSolCoef(3));
                                            Real64 afd3k = constrBack.afBareSolDiff(3);
                                            Real64 rfd3k = constrBack.rfBareSolDiff(3);
                                            state.dataSolarShading->SurfWinAbsBeam(3) =
                                                ab3k + t3k * tbshBBk * (rb2k + t2k * rb1k * t2k) * tfshBBk * af3k +
                                                t3k *
                                                    (tbshBdk * rbd2k * tfshdk + tbshBdk * td2k * rbd1k * td2k * tfshdk +
                                                     rbshBk * (1.0 + rfd3k * rbshdk)) *
                                                    afd3k;
                                            state.dataSolarShading->SurfWinAbsBeam(2) =
                                                t3k * tbshBBk * (ab2k + t2k * rb1k * (af2k + t2k * rfshBk * abd2k)) +
                                                t3k * (tbshBdk + tbshBdk * (rbd2k + td2k * rbd1k * td2k) * rfshdk + rbshBk * rfd3k * tbshdk) * abd2k +
                                                t3k * tbshBdk * td2k * rbd1k * afd2k;
                                            state.dataSolarShading->SurfWinAbsBeam(1) =
                                                t3k * tbshBBk * (t2k * ab1k + (rb2k + t2k * rb1k * t2k) * rfshBk * td2k * abd1k) +
                                                t3k * (rbshBk * rfd3k * tbshdk + tbshBdk * (1.0 + rbd2k * rfshdk + td2k * rbd2k * td2k * rfshdk)) *
                                                    td2k * abd1k;
                                            TransBeamWin = t3k * tbshBBk * t2k * t1k +
                                                           t3k *
                                                               (tbshBBk * (rb2k * rfshBk + t2k * rb1k * t2k * rfshBk) + rbshBk * rfd3k * tbshdk +
                                                                tbshBdk * (1.0 + rbd2k * rfshdk + td2k * rbd1k * td2k * rfshdk)) *
                                                               td2k * td1k;
                                            ABlBack = t3k * abshBk + t3k * tbshBBk * (rb2k + t2k * rb1k * t2k) * afshBk +
                                                      t3k * rbshBk * rfd3k * abshdk + t3k * tbshBdk * (rbd2k + td2k * rbd1k * td2k) * afshdk;
                                        }

                                        BABSZone += BOverlap * ABlBack;
                                        backSurfBeamSolInTrans += BOverlap * ABlBack;
                                        state.dataSolarShading->SurfWinIntBeamAbsByShadFac(BackSurfNum) =
                                            BOverlap * ABlBack / s_surf->Surface(BackSurfNum).Area;

                                    } // End of check if between-glass blind is on back window
                                } else if (ShadeFlagBack == WinShadingType::ExtScreen) {

                                    // Interior beam absorptance of GLASS LAYERS of exterior back window with EXTERIOR SCREEN
                                    Real64 TGlBmBack = Window::POLYF(CosIncBack, constrBack.TransSolBeamCoef);
                                    Real64 RGlDiffFront = constrBack.ReflectSolDiffFront;

                                    auto const *screen = dynamic_cast<Material::MaterialScreen const *>(s_mat->materials(ScNum));
                                    assert(screen != nullptr);

                                    auto &surf = s_surf->Surface(SurfNum);
                                    Real64 solPhi = std::acos(state.dataEnvrn->SOLCOS.z);
                                    Real64 solTheta = std::atan2(state.dataEnvrn->SOLCOS.x, state.dataEnvrn->SOLCOS.y);
                                    Real64 winPhi = surf.Tilt * Constant::DegToRad;
                                    Real64 winTheta = surf.Azimuth * Constant::DegToRad;
                                    Real64 phi = std::abs(solPhi - winPhi);
                                    Real64 theta = std::abs(solTheta - winTheta);
                                    Material::NormalizePhiTheta(phi, theta);
#ifdef PRECALC_INTERP_SCREEN
                                    int ip1, ip2, it1, it2;
                                    BilinearInterpCoeffs coeffs;
                                    Material::GetPhiThetaIndices(phi, theta, screen->dPhi, screen->dTheta, ip1, ip2, it1, it2);
                                    GetBilinearInterpCoeffs(
                                        phi, theta, ip1 * screen->dPhi, ip2 * screen->dPhi, it1 * screen->dTheta, it2 * screen->dTheta, coeffs);
                                    auto const &b11 = screen->btars[ip1][it1];
                                    auto const &b12 = screen->btars[ip1][it2];
                                    auto const &b21 = screen->btars[ip2][it1];
                                    auto const &b22 = screen->btars[ip2][it2];

                                    Real64 TScBmBmBack = BilinearInterp(b11.BmTransBack, b12.BmTransBack, b21.BmTransBack, b22.BmTransBack, coeffs);
                                    Real64 TScBmDiffBack = BilinearInterp(b11.DfTransBack, b12.DfTransBack, b21.DfTransBack, b22.DfTransBack, coeffs);
                                    Real64 RScBack = BilinearInterp(b11.RefSolFront, b12.RefSolFront, b21.RefSolFront, b22.RefSolFront, coeffs);
                                    Real64 RScDifBack = screen->DfRef;
                                    for (int Lay = 1; Lay <= NBackGlass; ++Lay) {
                                        Real64 AbWinBack = Window::POLYF(CosIncBack, constrBack.AbsBeamBackCoef(Lay));
                                        Real64 AGlDiffFront = constrBack.AbsDiff(Lay);
                                        state.dataSolarShading->SurfWinAbsBeam(Lay) =
                                            AbWinBack + (TGlBmBack * AGlDiffFront * RScBack / (1.0 - RScDifBack * RGlDiffFront));
                                    }

                                    // Interior beam transmitted by exterior back window with EXTERIOR SCREEN
                                    Real64 TScDifDif = screen->DfTrans;
                                    Real64 RScBmDifBk = BilinearInterp(b11.RefSolBack, b12.RefSolBack, b21.RefSolBack, b22.RefSolBack, coeffs);
                                    Real64 RGlDifFr = thisConstruct.ReflectSolDiffFront;
                                    Real64 RScDifDifBk = screen->DfRef;
                                    TransBeamWin = TGlBmBack *
                                                   (TScBmBmBack + TScBmDiffBack + TScDifDif * RScBmDifBk * RGlDifFr / (1.0 - RScDifDifBk * RGlDifFr));

                                    // Interior beam absorbed by EXTERIOR SCREEN on exterior back window
                                    Real64 AbsScBack = BilinearInterp(b11.AbsSolBack, b12.AbsSolBack, b21.AbsSolBack, b22.AbsSolBack, coeffs);
                                    Real64 AbsScDiffBack = screen->DfAbs;
                                    Real64 RScDiffBack = BilinearInterp(b11.RefSolFront, b12.RefSolFront, b21.RefSolFront, b22.RefSolFront, coeffs);
                                    // Screen solar back absorptance for interior solar
                                    Real64 AScBack =
                                        TGlBmBack * (AbsScBack + RScBack * RGlDiffFront * AbsScDiffBack / (1.0 - RScDiffBack * RGlDiffFront));

                                    BABSZone += BOverlap * AScBack;
                                    backSurfBeamSolInTrans += BOverlap * AScBack;
                                    state.dataSolarShading->SurfWinIntBeamAbsByShadFac(BackSurfNum) =
                                        BOverlap * AScBack / (s_surf->Surface(BackSurfNum).Area + s_surf->SurfWinDividerArea(BackSurfNum));
#else  // !PRECALC_INTERP_SCREEN
                                    Material::ScreenBmTransAbsRef btar;
                                    Material::CalcScreenTransmittance(state, screen, phi, theta, btar);

                                    Real64 TScBmBmBack = btar.BmTransBack;
                                    Real64 TScBmDiffBack = btar.DfTransBack;
                                    Real64 RScBack = btar.RefSolFront;
                                    Real64 RScDifBack = screen->DfRef;
                                    for (int Lay = 1; Lay <= NBackGlass; ++Lay) {
                                        Real64 AbWinBack = Window::POLYF(CosIncBack, constrBack.AbsBeamBackCoef(Lay));
                                        Real64 AGlDiffFront = constrBack.AbsDiff(Lay);
                                        state.dataSolarShading->SurfWinAbsBeam(Lay) =
                                            AbWinBack + (TGlBmBack * AGlDiffFront * RScBack / (1.0 - RScDifBack * RGlDiffFront));
                                    }

                                    // Interior beam transmitted by exterior back window with EXTERIOR SCREEN
                                    Real64 TScDifDif = screen->DfTrans;
                                    Real64 RScBmDifBk = btar.RefSolBack;
                                    Real64 RGlDifFr = thisConstruct.ReflectSolDiffFront;
                                    Real64 RScDifDifBk = screen->DfRef;
                                    TransBeamWin = TGlBmBack *
                                                   (TScBmBmBack + TScBmDiffBack + TScDifDif * RScBmDifBk * RGlDifFr / (1.0 - RScDifDifBk * RGlDifFr));

                                    // Interior beam absorbed by EXTERIOR SCREEN on exterior back window
                                    Real64 AbsScBack = btar.AbsSolBack;
                                    Real64 AbsScDiffBack = screen->DfAbs;
                                    Real64 RScDiffBack = btar.RefSolFront;
                                    // Screen solar back absorptance for interior solar
                                    Real64 AScBack =
                                        TGlBmBack * (AbsScBack + RScBack * RGlDiffFront * AbsScDiffBack / (1.0 - RScDiffBack * RGlDiffFront));

                                    BABSZone += BOverlap * AScBack;
                                    state.dataSolarShading->SurfWinIntBeamAbsByShadFac(BackSurfNum) =
                                        BOverlap * AScBack / (s_surf->Surface(BackSurfNum).Area + s_surf->SurfWinDividerArea(BackSurfNum));
#endif // PRECALC_INTERP_SCREEN

                                } // End of check if exterior screen on back window

                                // Interior beam absorptance of glass layers of back exterior window with SWITCHABLE GLAZING
                                if (ShadeFlagBack == WinShadingType::SwitchableGlazing && s_surf->Surface(BackSurfNum).ExtBoundCond == 0) {
                                    Real64 SwitchFac = s_surf->SurfWinSwitchingFactor(SurfNum); // Switching factor for a window
                                    Real64 AbsBeamWinSh; // Glass layer beam solar absorptance of a shaded window
                                    for (int Lay = 1; Lay <= NBackGlass; ++Lay) {
                                        AbsBeamWinSh =
                                            Window::POLYF(CosIncBack, state.dataConstruction->Construct(ConstrNumBackSh).AbsBeamBackCoef(Lay));
                                        state.dataSolarShading->SurfWinAbsBeam(Lay) =
                                            Window::InterpSw(SwitchFac, state.dataSolarShading->SurfWinAbsBeam(Lay), AbsBeamWinSh);
                                    }
                                    // Beam solar transmittance of a shaded window
                                    Real64 TransBeamWinSh =
                                        Window::POLYF(CosIncBack, state.dataConstruction->Construct(ConstrNumBackSh).TransSolBeamCoef);
                                    TransBeamWin = Window::InterpSw(SwitchFac, TransBeamWin, TransBeamWinSh);
                                }

                                // Sum of interior beam absorbed by all glass layers of back window
                                AbsBeamTotWin = 0.0;
                                for (int Lay = 1; Lay <= NBackGlass; ++Lay) {
                                    AbsBeamTotWin += state.dataSolarShading->SurfWinAbsBeam(Lay);
                                    s_surf->SurfWinA(BackSurfNum, Lay) +=
                                        BOverlap * state.dataSolarShading->SurfWinAbsBeam(Lay) /
                                        (s_surf->Surface(BackSurfNum).Area + s_surf->SurfWinDividerArea(BackSurfNum)); //[-]
                                }

                                // To BABSZon, add interior beam glass absorption and overall beam transmission for this back window
                                BABSZone += BOverlap * (AbsBeamTotWin + TransBeamWin);
                                backSurfBeamSolInTrans += BOverlap * (AbsBeamTotWin + TransBeamWin);
                                state.dataHeatBalSurf->SurfWinInitialBeamSolInTrans(BackSurfNum) +=
                                    backSurfBeamSolInTrans * state.dataEnvrn->BeamSolarRad; //[W]
                                // Interior beam transmitted to adjacent zone through an interior back window (assumed unshaded);
                                // this beam radiation is categorized as diffuse radiation in the adjacent zone.
                                int AdjSurfNum = s_surf->Surface(BackSurfNum).ExtBoundCond;
                                if (AdjSurfNum > 0) {
                                    int adjEnclosureNum = s_surf->Surface(AdjSurfNum).SolarEnclIndex;
                                    state.dataHeatBal->EnclSolDBIntWin(adjEnclosureNum) += BOverlap * TransBeamWin; //[m2]
                                    s_surf->SurfWinBmSolTransThruIntWinRep(BackSurfNum) +=
                                        BOverlap * TransBeamWin * state.dataEnvrn->BeamSolarRad; //[W]
                                    s_surf->SurfWinBmSolTransThruIntWinRepEnergy(BackSurfNum) =
                                        s_surf->SurfWinBmSolTransThruIntWinRep(BackSurfNum) * state.dataGlobal->TimeStepZoneSec;
                                }
                            } // End of check if back surface is opaque or window
                            state.dataHeatBal->SurfBmIncInsSurfAmountRep(BackSurfNum) += BOverlap;
                            state.dataHeatBal->SurfBmIncInsSurfAmountRepEnergy(BackSurfNum) =
                                state.dataHeatBal->SurfBmIncInsSurfAmountRep(BackSurfNum) * state.dataGlobal->TimeStepZoneSec;
                        } // End of loop over back surfaces
                    } else if (s_surf->SurfWinWindowModelType(SurfNum) == WindowModel::BSDF) {
                        // For complex window calculation goes over outgoing basis directions for current state
                        int CurCplxFenState = s_surf->SurfaceWindow(SurfNum).ComplexFen.CurrentState; // Current state for complex fenestration
                        // Get construction number which keeps transmittance properties
                        int IConst = s_surf->SurfaceWindow(SurfNum).ComplexFen.State(CurCplxFenState).Konst; // Current surface construction number
                                                                                                             // (it depends of state too)
                        // Solar radiation from this window will be calculated only in case when this window is not scheduled surface gained
                        if (WindowScheduledSolarAbs(state, SurfNum, IConst) == 0) {
                            // Current incoming direction number (Sun direction)
                            int IBm = state.dataBSDFWindow->ComplexWind(SurfNum)
                                          .Geom(CurCplxFenState)
                                          .SolBmIndex(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep);
                            // Report variables for complex fenestration here
                            state.dataHeatBal->SurfWinBSDFBeamDirectionRep(SurfNum) = IBm;
                            state.dataHeatBal->SurfWinBSDFBeamThetaRep(SurfNum) =
                                state.dataBSDFWindow->ComplexWind(SurfNum)
                                    .Geom(CurCplxFenState)
                                    .ThetaBm(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep);
                            state.dataHeatBal->SurfWinBSDFBeamPhiRep(SurfNum) = state.dataBSDFWindow->ComplexWind(SurfNum)
                                                                                    .Geom(CurCplxFenState)
                                                                                    .PhiBm(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep);

                            int BaseSurf = surf.BaseSurf; // Base surface number for current complex window
                            // Get total number of back surfaces for current window (surface)
                            // Note that it is organized by base surface
                            int NBkSurf = state.dataShadowComb->ShadowComb(BaseSurf).NumBackSurf;
                            if (!allocated(CFBoverlap)) {
                                CFBoverlap.allocate(NBkSurf);
                            }
                            if (!allocated(CFDirBoverlap)) {
                                CFDirBoverlap.allocate(NBkSurf, state.dataBSDFWindow->ComplexWind(SurfNum).Geom(CurCplxFenState).Trn.NBasis);
                            }

                            CFBoverlap = 0.0;
                            // Calculate effects on all back surfaces for each of basis directions.  Each of basis directions from the back of the
                            // window has to be considered as beam and therefore calculate CFBoverlap for each of them
                            for (int CurTrnDir = 1; CurTrnDir <= state.dataBSDFWindow->ComplexWind(SurfNum).Geom(CurCplxFenState).Trn.NBasis;
                                 ++CurTrnDir) {
                                Real64 CurLambda = state.dataBSDFWindow->ComplexWind(SurfNum)
                                                       .Geom(CurCplxFenState)
                                                       .Trn.Lamda(CurTrnDir); // Current lambda value in BSDF outgoing directions
                                Real64 DirTrans = state.dataConstruction->Construct(IConst).BSDFInput.SolFrtTrans(
                                    IBm, CurTrnDir); // Current BSDF directional transmittance
                                // Now calculate effect of this direction on all back surfaces
                                for (int IBack = 1; IBack <= NBkSurf; ++IBack) {
                                    CFDirBoverlap(IBack, CurTrnDir) =
                                        state.dataBSDFWindow->ComplexWind(SurfNum).Geom(CurCplxFenState).AOverlap(IBack, CurTrnDir) * DirTrans *
                                        CurLambda * CosInc;
                                    CFBoverlap(IBack) += CFDirBoverlap(IBack, CurTrnDir);
                                } // DO IBack = 1,MaxBkSurf
                            }

                            // Summarizing results
                            for (int IBack = 1; IBack <= NBkSurf; ++IBack) {
                                int BackSurfaceNumber = state.dataShadowComb->ShadowComb(BaseSurf).BackSurf(IBack);
                                int ConstrNumBack = s_surf->Surface(BackSurfaceNumber).Construction;
                                auto const &constrBack = state.dataConstruction->Construct(ConstrNumBack);
                                // Do not perform any calculation if surface is scheduled for incoming solar radiation
                                int SurfSolIncPtr = SurfaceScheduledSolarInc(state, BackSurfaceNumber, ConstrNumBack);

                                if (SurfSolIncPtr == 0) {
                                    // Surface hit is another complex fenestration
                                    if (s_surf->SurfWinWindowModelType(BackSurfaceNumber) == WindowModel::BSDF) {
                                        int CurBackState =
                                            s_surf->SurfaceWindow(BackSurfaceNumber)
                                                .ComplexFen.CurrentState; // Current state for back surface if that surface is complex fenestration
                                        // Do not take into account this window if it is scheduled for surface gains
                                        if (WindowScheduledSolarAbs(state, BackSurfaceNumber, ConstrNumBack) == 0) {
                                            // Calculate energy loss per each outgoing orientation
                                            for (int CurTrnDir = 1;
                                                 CurTrnDir <= state.dataBSDFWindow->ComplexWind(SurfNum).Geom(CurCplxFenState).Trn.NBasis;
                                                 ++CurTrnDir) {
                                                Real64 bestDot; // complex fenestration hits other complex fenestration, it is important to find
                                                // matching beam directions.  Beam leaving one window will have certain number for it's basis
                                                // while same beam reaching back surface will have different beam number.  This value is used
                                                // to keep best matching dot product for those directions
                                                Real64 curDot;   // temporary variable for current dot product
                                                int bestBackTrn; // Direction corresponding best dot product for back surface window
                                                for (int CurBackDir = 1;
                                                     CurBackDir <= state.dataBSDFWindow->ComplexWind(BackSurfaceNumber).Geom(CurBackState).Trn.NBasis;
                                                     ++CurBackDir) {
                                                    // Purpose of this part is to find best match for outgoing beam number of window back surface
                                                    // and incoming beam number of complex fenestration which this beam will hit on (back surface
                                                    // again)
                                                    curDot =
                                                        dot(state.dataBSDFWindow->ComplexWind(SurfNum).Geom(CurCplxFenState).sTrn(CurTrnDir),
                                                            state.dataBSDFWindow->ComplexWind(BackSurfaceNumber).Geom(CurBackState).sTrn(CurBackDir));
                                                    if (CurBackDir == 1) {
                                                        bestDot = curDot;
                                                        bestBackTrn = CurBackDir;
                                                    } else {
                                                        if (curDot < bestDot) {
                                                            bestDot = curDot;
                                                            bestBackTrn = CurBackDir;
                                                        }
                                                    }
                                                }
                                                // CurLambda = ComplexWind(BackSurfaceNumber)%Geom(CurBackState)%Trn%Lamda(CurTrnDir)
                                                // Add influence of this exact direction to what stays in the zone.  It is important to note that
                                                // this needs to be done for each outgoing direction
                                                Real64 babs = CFDirBoverlap(IBack, CurTrnDir) * (1 - s_surf->SurfaceWindow(BackSurfaceNumber)
                                                                                                         .ComplexFen.State(CurBackState)
                                                                                                         .IntegratedBkRefl(bestBackTrn));
                                                BABSZone += babs;
                                                state.dataHeatBalSurf->SurfWinInitialBeamSolInTrans(BackSurfaceNumber) +=
                                                    babs * state.dataEnvrn->BeamSolarRad; //[W]

                                                // Absorptance from current back direction
                                                int TotSolidLay = constrBack.TotSolidLayers;
                                                for (int Lay = 1; Lay <= TotSolidLay; ++Lay) {
                                                    // IF (ALLOCATED(Construct(ConstrNumBack)%BSDFInput)) THEN
                                                    // CFDirBoverlap is energy transmitted for current basis beam.  It is important to note that
                                                    // AWinOverlap array needs to contain flux and not absorbed energy because later in the code
                                                    // this will be multiplied with window area
                                                    s_surf->SurfWinACFOverlap(BackSurfaceNumber, Lay) +=
                                                        constrBack.BSDFInput.Layer(Lay).BkAbs(bestBackTrn, 1) * CFDirBoverlap(IBack, CurTrnDir) /
                                                        s_surf->Surface(BackSurfaceNumber).Area;
                                                    // END IF
                                                }

                                                // Interior beam transmitted to adjacent zone through an interior back window;
                                                // This beam radiation is categorized as diffuse radiation in the adjacent zone.
                                                // Note that this is done for each outgoing direction of exterior window
                                                int AdjSurfNum = s_surf->Surface(BackSurfaceNumber).ExtBoundCond;
                                                if (AdjSurfNum > 0) {
                                                    int adjEnclosureNum = s_surf->Surface(AdjSurfNum).SolarEnclIndex;
                                                    state.dataHeatBal->EnclSolDBIntWin(adjEnclosureNum) +=
                                                        CFDirBoverlap(IBack, CurTrnDir) * s_surf->SurfaceWindow(BackSurfaceNumber)
                                                                                              .ComplexFen.State(CurBackState)
                                                                                              .IntegratedBkTrans(bestBackTrn);
                                                    s_surf->SurfWinBmSolTransThruIntWinRep(BackSurfaceNumber) +=
                                                        CFDirBoverlap(IBack, CurTrnDir) *
                                                        s_surf->SurfaceWindow(BackSurfaceNumber)
                                                            .ComplexFen.State(CurBackState)
                                                            .IntegratedBkTrans(bestBackTrn) *
                                                        state.dataEnvrn->BeamSolarRad; //[W]
                                                    s_surf->SurfWinBmSolTransThruIntWinRepEnergy(BackSurfaceNumber) =
                                                        s_surf->SurfWinBmSolTransThruIntWinRep(BackSurfaceNumber) * state.dataGlobal->TimeStepZoneSec;
                                                }
                                            }
                                        }
                                    } else {
                                        if (constrBack.TransDiff <= 0.0) {
                                            // Do not take into account this window if it is scheduled for surface gains
                                            Real64 AbsIntSurf = constrBack.InsideAbsorpSolar;
                                            s_surf->SurfOpaqAI(BackSurfaceNumber) +=
                                                CFBoverlap(IBack) * AbsIntSurf / s_surf->Surface(BackSurfaceNumber).Area;
                                            BABSZone += CFBoverlap(IBack) * AbsIntSurf;
                                        } else {
                                            // Code for mixed windows goes here.  It is same as above code for "ordinary" windows.
                                            // Try to do something which will not produce duplicate code.
                                        }
                                    }
                                }
                            }

                            if (allocated(CFBoverlap)) {
                                CFBoverlap.deallocate();
                            }
                            if (allocated(CFDirBoverlap)) {
                                CFDirBoverlap.deallocate();
                            }
                        }

                    } else if (s_surf->SurfWinWindowModelType(SurfNum) == WindowModel::EQL) {

                        for (int IBack = 1; IBack <= state.dataBSDFWindow->MaxBkSurf; ++IBack) {
                            int BackSurfNum =
                                state.dataHeatBal->SurfWinBackSurfaces(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, IBack, SurfNum);
                            if (BackSurfNum == 0) {
                                break; // No more irradiated back surfaces for this exterior window
                            }
                            if (s_surf->SurfWinWindowModelType(IBack) != WindowModel::EQL) {
                                continue; // only EQL back window is allowed
                            }

                            int ConstrNumBack = s_surf->Surface(BackSurfNum).Construction;
                            auto const &constrBack = state.dataConstruction->Construct(ConstrNumBack);
                            int NBackGlass = constrBack.TotGlassLayers;
                            // Irradiated (overlap) area for this back surface, projected onto window plane
                            // (includes effect of shadowing on exterior window)

                            Real64 AOverlap =
                                state.dataHeatBal->SurfWinOverlapAreas(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, IBack, SurfNum);
                            Real64 BOverlap = TBm * AOverlap * CosInc; //[m2]

                            if (constrBack.TransDiff <= 0.0) {

                                // Back surface is opaque interior or exterior wall
                                Real64 AbsIntSurf = state.dataHeatBalSurf->SurfAbsSolarInt(BackSurfNum);
                                s_surf->SurfOpaqAI(BackSurfNum) += BOverlap * AbsIntSurf / s_surf->Surface(BackSurfNum).Area; //[-]
                                BABSZone += BOverlap * AbsIntSurf;                                                            //[m2]

                            } else {

                                // Back surface is an interior or exterior window
                                // Note that exterior back windows with and without shades are treated as defined.
                                // Equivalent Layer window model has no distinction when treating windows with and
                                // without shades (interior, inbetween and exterior shades)
                                //  Note in equivalent layer window model if storm window exists it is defined as part of
                                //  window construction, hence it does not require a separate treatment
                                state.dataSolarShading->SurfWinAbsBeamEQL = 0.0;
                                Real64 TransBeamWin = 0.0;           // Beam solar transmittance of a window
                                Real64 backSurfBeamSolInTrans = 0.0; // Fraction of BeamSolarRad transmitted out through window inside face [W]

                                // Interior beam absorptance of glass layers and beam transmittance of back exterior  &
                                // or interior window (treats windows with/without shades as defined) for this timestep

                                // call the ASHWAT fenestration model for beam radiation here
                                WindowEquivalentLayer::CalcEQLOpticalProperty(
                                    state, BackSurfNum, SolarArrays::BEAM, state.dataSolarShading->SurfWinAbsSolBeamBackEQL);
                                auto &CFS = state.dataWindowEquivLayer->CFS;
                                int EQLNum = constrBack.EQLConsPtr;
                                state.dataSolarShading->SurfWinAbsBeamEQL({1, CFS(EQLNum).NL}) =
                                    state.dataSolarShading->SurfWinAbsSolBeamBackEQL(1, {1, CFS(EQLNum).NL});
                                // get the interior beam transmitted through back exterior or interior EQL window
                                TransBeamWin = state.dataSolarShading->SurfWinAbsSolBeamBackEQL(1, CFS(EQLNum).NL + 1);
                                //   Absorbed by the interior shade layer of back exterior window
                                if (CFS(EQLNum).L(CFS(EQLNum).NL).LTYPE != LayerType::GLAZE) {
                                    state.dataSolarShading->SurfWinIntBeamAbsByShadFac(BackSurfNum) =
                                        BOverlap * state.dataSolarShading->SurfWinAbsSolBeamBackEQL(1, CFS(EQLNum).NL) /
                                        (s_surf->Surface(BackSurfNum).Area + s_surf->SurfWinDividerArea(BackSurfNum));
                                    BABSZone += BOverlap * state.dataSolarShading->SurfWinAbsSolBeamBackEQL(1, CFS(EQLNum).NL);
                                    backSurfBeamSolInTrans += BOverlap * state.dataSolarShading->SurfWinAbsSolBeamBackEQL(1, CFS(EQLNum).NL);
                                }
                                //   Absorbed by the exterior shade layer of back exterior window
                                if (CFS(EQLNum).L(1).LTYPE != LayerType::GLAZE) {
                                    state.dataSolarShading->SurfWinIntBeamAbsByShadFac(BackSurfNum) =
                                        BOverlap * state.dataSolarShading->SurfWinAbsSolBeamBackEQL(1, 1) /
                                        (s_surf->Surface(BackSurfNum).Area + s_surf->SurfWinDividerArea(BackSurfNum));
                                    BABSZone += BOverlap * state.dataSolarShading->SurfWinAbsSolBeamBackEQL(1, 1);
                                    backSurfBeamSolInTrans += BOverlap * state.dataSolarShading->SurfWinAbsSolBeamBackEQL(1, 1);
                                }

                                // determine the number of glass layers
                                NBackGlass = 0;
                                for (int Lay = 1; Lay <= CFS(EQLNum).NL; ++Lay) {
                                    if (CFS(EQLNum).L(Lay).LTYPE != LayerType::GLAZE) {
                                        continue;
                                    }
                                    ++NBackGlass;
                                }
                                if (NBackGlass >= 2) {
                                    // If the number of glass is greater than 2, in between glass shade can be present
                                    for (int Lay = 2; Lay <= CFS(EQLNum).NL - 1; ++Lay) {
                                        if (CFS(EQLNum).L(CFS(EQLNum).NL).LTYPE != LayerType::GLAZE) {
                                            // if there is in between shade glass determine the shade absorptance
                                            state.dataSolarShading->SurfWinIntBeamAbsByShadFac(BackSurfNum) +=
                                                BOverlap * state.dataSolarShading->SurfWinAbsSolBeamBackEQL(1, Lay) /
                                                s_surf->Surface(BackSurfNum).Area;
                                            BABSZone += BOverlap * state.dataSolarShading->SurfWinAbsSolBeamBackEQL(1, Lay);
                                            backSurfBeamSolInTrans += BOverlap * state.dataSolarShading->SurfWinAbsSolBeamBackEQL(1, Lay);
                                        }
                                    }
                                }
                                // Sum of interior beam absorbed by all glass layers of back window
                                Real64 AbsBeamTotWin = 0.0; // Glass layer beam solar absorptance of a shaded window
                                for (int Lay = 1; Lay <= CFS(EQLNum).NL; ++Lay) {
                                    AbsBeamTotWin += state.dataSolarShading->SurfWinAbsBeamEQL(Lay);
                                    s_surf->SurfWinA(BackSurfNum, Lay) +=
                                        BOverlap * state.dataSolarShading->SurfWinAbsBeamEQL(Lay) /
                                        (s_surf->Surface(BackSurfNum).Area + s_surf->SurfWinDividerArea(BackSurfNum)); //[-]
                                }

                                // To BABSZon, add interior beam glass absorption and overall beam transmission for this back window

                                BABSZone += BOverlap * (AbsBeamTotWin + TransBeamWin);
                                backSurfBeamSolInTrans += BOverlap * (AbsBeamTotWin + TransBeamWin);
                                state.dataHeatBalSurf->SurfWinInitialBeamSolInTrans(BackSurfNum) +=
                                    backSurfBeamSolInTrans * state.dataEnvrn->BeamSolarRad; //[W]

                                // Interior beam transmitted to adjacent zone through an interior back window (assumed unshaded);
                                // this beam radiation is categorized as diffuse radiation in the adjacent zone.

                                int AdjSurfNum = s_surf->Surface(BackSurfNum).ExtBoundCond;
                                if (AdjSurfNum > 0) {
                                    int adjEnclosureNum = s_surf->Surface(AdjSurfNum).SolarEnclIndex;
                                    state.dataHeatBal->EnclSolDBIntWin(adjEnclosureNum) += BOverlap * TransBeamWin; //[m2]
                                    s_surf->SurfWinBmSolTransThruIntWinRep(BackSurfNum) +=
                                        BOverlap * TransBeamWin * state.dataEnvrn->BeamSolarRad; //[W]
                                    s_surf->SurfWinBmSolTransThruIntWinRepEnergy(BackSurfNum) =
                                        s_surf->SurfWinBmSolTransThruIntWinRep(BackSurfNum) * state.dataGlobal->TimeStepZoneSec;
                                }
                            } // End of check if back surface is opaque or window
                            state.dataHeatBal->SurfBmIncInsSurfAmountRep(BackSurfNum) += BOverlap;
                            state.dataHeatBal->SurfBmIncInsSurfAmountRepEnergy(BackSurfNum) =
                                state.dataHeatBal->SurfBmIncInsSurfAmountRep(BackSurfNum) * state.dataGlobal->TimeStepZoneSec;
                        } // End of loop over back surfaces

                        //  *****************************

                    } // IF (SurfaceWindow(SurfNum)%WindowModelType /= WindowModel:: BSDF) THEN
                } else { // Simple interior solar distribution. All beam from exterior windows falls on floor;
                    // some of this is absorbed/transmitted, rest is reflected to other surfaces.

                    for (int const FloorNum : thisEnclosure.SurfacePtr) {
                        // In following, ISABSF is zero except for nominal floor surfaces
                        if (state.dataSolarShading->SurfIntAbsFac(FloorNum) <= 0.0 || FloorNum == SurfNum) {
                            continue; // Keep only floor surfaces
                        }
                        int const FlConstrNum = s_surf->SurfActiveConstruction(FloorNum);

                        Real64 BTOTWinZone = TBm * SunLitFract * surf.Area * CosInc * InOutProjSLFracMult; //[m2]
                        Real64 AbsBeamTotWin = 0.0;

                        if (state.dataConstruction->Construct(FlConstrNum).TransDiff <= 0.0) {
                            // Opaque surface
                            s_surf->SurfOpaqAI(FloorNum) +=
                                BTOTWinZone * state.dataSolarShading->SurfIntAbsFac(FloorNum) / s_surf->Surface(FloorNum).Area; //[-]
                        } else {
                            // Window

                            // Note that diffuse solar absorptance is used here for floor windows even though we're
                            // dealing with incident beam radiation. This is because, for this simple interior distribution,
                            // the beam radiation from exterior windows is assumed to be uniformly distributed over the
                            // floor and so it makes no sense to use directional absorptances. Note also that floor windows
                            // are assumed to not have blinds or shades in this calculation.
                            // For the case of the floor window a complex fenestration (strange situation) the correct back
                            // diffuse layer absorptions have already been put into the construction

                            for (int Lay = 1; Lay <= state.dataConstruction->Construct(FlConstrNum).TotGlassLayers; ++Lay) {
                                AbsBeamTotWin += state.dataConstruction->Construct(FlConstrNum).AbsDiffBack(Lay);
                            }
                            // In the following we have to multiply by the AbsDiffBack(Lay)/AbsBeamTotWin ratio to get the
                            // layer by layer absorbed beam since ISABSF(FloorNum) is proportional to AbsBeamTotWin
                            // (see ComputeIntSolarAbsorpFactors).

                            for (int Lay = 1; Lay <= state.dataConstruction->Construct(FlConstrNum).TotGlassLayers; ++Lay) {
                                s_surf->SurfWinA(FloorNum, Lay) += state.dataConstruction->Construct(FlConstrNum).AbsDiffBack(Lay) / AbsBeamTotWin *
                                                                   BTOTWinZone * state.dataSolarShading->SurfIntAbsFac(FloorNum) /
                                                                   s_surf->Surface(FloorNum).Area; //[-]
                            }
                        }

                        BABSZone += BTOTWinZone * state.dataSolarShading->SurfIntAbsFac(FloorNum); //[m2]

                        int AdjSurfNum = s_surf->Surface(FloorNum).ExtBoundCond;
                        if (state.dataConstruction->Construct(FlConstrNum).TransDiff > 0.0 && AdjSurfNum > 0) {

                            // Window in an interior floor

                            int adjEnclosureNum = s_surf->Surface(AdjSurfNum).SolarEnclIndex;

                            // Contribution (assumed diffuse) to adjacent zone of beam radiation passing
                            // through this window
                            state.dataHeatBal->EnclSolDBIntWin(adjEnclosureNum) += BTOTWinZone * state.dataSolarShading->SurfIntAbsFac(FloorNum) *
                                                                                   state.dataConstruction->Construct(FlConstrNum).TransDiff /
                                                                                   AbsBeamTotWin;

                            BABSZone += BTOTWinZone * state.dataSolarShading->SurfIntAbsFac(FloorNum) *
                                        state.dataConstruction->Construct(FlConstrNum).TransDiff / AbsBeamTotWin;
                        }

                    } // End of loop over floor sections
                } // End of check on complex vs. simple interior solar distribution

            } // End of sunlit fraction > 0 test
        } // End of first loop over surfaces in zone

        Real64 BABSZoneSSG = 0.0; // Beam radiation from exterior windows absorbed in a zone (only for scheduled surface gains)
        Real64 BTOTZoneSSG = 0.0; // Solar entering a zone in case of scheduled surface gains
        for (int iSSG = 1; iSSG <= s_surf->TotSurfIncSolSSG; ++iSSG) {
            int SurfNum = s_surf->SurfIncSolSSG(iSSG).SurfPtr;
            auto &surf = s_surf->Surface(SurfNum);
            // do calculation only if construction number match.
            if (s_surf->SurfIncSolSSG(iSSG).ConstrPtr == surf.Construction) {
                if (surf.SolarEnclIndex == enclosureNum) {
                    Real64 AbsIntSurf = state.dataConstruction->Construct(surf.Construction).InsideAbsorpSolar;
                    // SolarIntoZone = GetCurrentScheduleValue(SurfIncSolSSG(iSSG)%SchedPtr) * Surface(SurfNum)%Area
                    Real64 SolarIntoZone = s_surf->SurfIncSolSSG(iSSG).sched->getCurrentVal(); // Solar radiation into zone to current surface
                    s_surf->SurfOpaqAI(SurfNum) = SolarIntoZone * AbsIntSurf;
                    BABSZoneSSG += s_surf->SurfOpaqAI(SurfNum) * surf.Area;
                    BTOTZoneSSG += SolarIntoZone * surf.Area;
                }
            }
        }
        state.dataHeatBal->EnclSolDBSSG(enclosureNum) = BTOTZoneSSG - BABSZoneSSG;
        state.dataHeatBal->EnclSolDB(enclosureNum) = BTOTZone - BABSZone;

        if (state.dataHeatBal->EnclSolDB(enclosureNum) < 0.0) {
            state.dataHeatBal->EnclSolDB(enclosureNum) = 0.0;
        }

        // Variables for reporting
        for (int const SurfNum : thisEnclosure.SurfacePtr) {
            auto &surf = s_surf->Surface(SurfNum);

            Real64 SurfIncSolarMultiplier = surf.IncSolMultiplier;
            Real64 currBeamSolarRad = state.dataEnvrn->BeamSolarRad * SurfIncSolarMultiplier;
            Real64 currDifSolarRad = state.dataEnvrn->DifSolarRad * SurfIncSolarMultiplier;
            Real64 currGndSolarRad = state.dataEnvrn->GndSolarRad * SurfIncSolarMultiplier;
            if (state.dataHeatBal->SolarDistribution == DataHeatBalance::Shadowing::FullInteriorExterior) {
                state.dataHeatBal->SurfBmIncInsSurfAmountRep(SurfNum) *= currBeamSolarRad;
                state.dataHeatBal->SurfBmIncInsSurfAmountRepEnergy(SurfNum) =
                    state.dataHeatBal->SurfBmIncInsSurfAmountRep(SurfNum) * state.dataGlobal->TimeStepZoneSec;
                state.dataHeatBal->SurfBmIncInsSurfIntensRep(SurfNum) =
                    state.dataHeatBal->SurfBmIncInsSurfAmountRep(SurfNum) / (surf.Area + s_surf->SurfWinDividerArea(SurfNum));
            } else { // Simple interior solar distribution. All beam falls on floor.
                if (state.dataSolarShading->SurfIntAbsFac(SurfNum) > 0.0 && surf.HeatTransSurf) {
                    if (thisEnclosure.FloorArea > 0.0) {
                        // spread onto all floor surfaces, these may or may not be called "floor"
                        state.dataHeatBal->SurfBmIncInsSurfIntensRep(SurfNum) = currBeamSolarRad * BTOTZone / thisEnclosure.FloorArea;
                    } else if (thisEnclosure.TotalSurfArea > 0.0) {
                        // spread onto all interior surfaces
                        state.dataHeatBal->SurfBmIncInsSurfIntensRep(SurfNum) = currBeamSolarRad * BTOTZone / thisEnclosure.TotalSurfArea;
                    } else { // divide be zero otherwise
                        state.dataHeatBal->SurfBmIncInsSurfIntensRep(SurfNum) = 0.0;
                    }
                }
                state.dataHeatBal->SurfBmIncInsSurfAmountRep(SurfNum) = surf.Area * state.dataHeatBal->SurfBmIncInsSurfIntensRep(SurfNum);
                state.dataHeatBal->SurfBmIncInsSurfAmountRepEnergy(SurfNum) =
                    state.dataHeatBal->SurfBmIncInsSurfAmountRep(SurfNum) * state.dataGlobal->TimeStepZoneSec;
            }
            if (surf.Class == SurfaceClass::Window || surf.Class == SurfaceClass::TDD_Dome) {

                s_surf->SurfWinIntBeamAbsByShade(SurfNum) = state.dataSolarShading->SurfWinIntBeamAbsByShadFac(SurfNum);
                s_surf->SurfWinExtBeamAbsByShade(SurfNum) = currBeamSolarRad * state.dataSolarShading->SurfWinExtBeamAbsByShadFac(SurfNum);

                if ((surf.ExtBoundCond == ExternalEnvironment) || (surf.ExtBoundCond == OtherSideCondModeledExt)) {

                    WinShadingType ShadeFlag = s_surf->SurfWinShadingFlag(SurfNum);
                    int ShelfNum = s_surf->SurfDaylightingShelfInd(SurfNum);
                    int OutShelfSurf = 0;
                    if (ShelfNum > 0) { // Outside daylighting shelf
                        OutShelfSurf = state.dataDaylightingDevicesData->Shelf(ShelfNum).OutSurf;
                    }

                    // This lookup may be avoid if this 2nd surf loop can be combined with the 1st
                    if (surf.OriginalClass == SurfaceClass::TDD_Diffuser) {
                        int PipeNum = s_surf->SurfWinTDDPipeNum(SurfNum);
                        int SurfNum2 = state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome;
                        Real64 CosInc = state.dataHeatBal->SurfCosIncAng(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum2);
                        // Exterior diffuse solar incident on window (W/m2)
                        Real64 DifSolarInc = currDifSolarRad * state.dataSolarShading->SurfAnisoSkyMult(SurfNum2) +
                                             currGndSolarRad * s_surf->Surface(SurfNum2).ViewFactorGround;
                        // Exterior diffuse sky solar transmitted by TDD (W/m2)
                        Real64 SkySolarTrans = currDifSolarRad * TransTDD(state, PipeNum, CosInc, Dayltg::RadType::SolarAniso) *
                                               state.dataSolarShading->SurfAnisoSkyMult(SurfNum2);
                        // Exterior diffuse ground solar transmitted by TDD (W/m2)
                        Real64 GndSolarTrans = currGndSolarRad * state.dataDaylightingDevicesData->TDDPipe(PipeNum).TransSolIso *
                                               s_surf->Surface(SurfNum2).ViewFactorGround;

                        s_surf->SurfWinBmSolar(SurfNum) = currBeamSolarRad * state.dataSolarShading->SurfWinTransBmSolar(SurfNum);
                        s_surf->SurfWinDifSolar(SurfNum) = SkySolarTrans * surf.Area + GndSolarTrans * surf.Area;
                        s_surf->SurfWinBmSolarEnergy(SurfNum) = s_surf->SurfWinBmSolar(SurfNum) * state.dataGlobal->TimeStepZoneSec;
                        s_surf->SurfWinDifSolarEnergy(SurfNum) = s_surf->SurfWinDifSolar(SurfNum) * state.dataGlobal->TimeStepZoneSec;

                        s_surf->SurfWinTransSolar(SurfNum) = s_surf->SurfWinBmSolar(SurfNum) + s_surf->SurfWinDifSolar(SurfNum); //[W]
                        s_surf->SurfWinTransSolarEnergy(SurfNum) = s_surf->SurfWinTransSolar(SurfNum) * state.dataGlobal->TimeStepZoneSec;

                        state.dataDaylightingDevicesData->TDDPipe(PipeNum).TransmittedSolar = s_surf->SurfWinTransSolar(SurfNum);
                        // TDDPipe(PipeNum)%TransSolBeam = TBmBm ! Reported above
                        if (DifSolarInc > 0) {
                            state.dataDaylightingDevicesData->TDDPipe(PipeNum).TransSolDiff = (SkySolarTrans + GndSolarTrans) / DifSolarInc;
                        } else {
                            state.dataDaylightingDevicesData->TDDPipe(PipeNum).TransSolDiff = 0.0;
                        }

                    } else if (OutShelfSurf > 0) { // Outside daylighting shelf
                        Real64 ShelfSolarRad =
                            (currBeamSolarRad *
                                 state.dataHeatBal->SurfSunlitFrac(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, OutShelfSurf) *
                                 state.dataHeatBal->SurfCosIncAng(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, OutShelfSurf) +
                             currDifSolarRad * state.dataSolarShading->SurfAnisoSkyMult(OutShelfSurf)) *
                            state.dataDaylightingDevicesData->Shelf(ShelfNum).OutReflectSol;

                        Real64 DifSolarInc = currDifSolarRad * state.dataSolarShading->SurfAnisoSkyMult(SurfNum) +
                                             currGndSolarRad * surf.ViewFactorGround +
                                             ShelfSolarRad * state.dataDaylightingDevicesData->Shelf(ShelfNum).ViewFactor;

                        s_surf->SurfWinBmSolar(SurfNum) = currBeamSolarRad * state.dataSolarShading->SurfWinTransBmSolar(SurfNum);
                        s_surf->SurfWinDifSolar(SurfNum) = DifSolarInc * state.dataSolarShading->SurfWinTransDifSolar(SurfNum);
                        s_surf->SurfWinBmSolarEnergy(SurfNum) = s_surf->SurfWinBmSolar(SurfNum) * state.dataGlobal->TimeStepZoneSec;
                        s_surf->SurfWinDifSolarEnergy(SurfNum) = s_surf->SurfWinDifSolar(SurfNum) * state.dataGlobal->TimeStepZoneSec;

                        s_surf->SurfWinTransSolar(SurfNum) = s_surf->SurfWinBmSolar(SurfNum) + s_surf->SurfWinDifSolar(SurfNum); //[W]
                        s_surf->SurfWinTransSolarEnergy(SurfNum) = s_surf->SurfWinTransSolar(SurfNum) * state.dataGlobal->TimeStepZoneSec;

                    } else { // Regular window
                        Real64 SkySolarInc = s_surf->SurfSkySolarInc(SurfNum);
                        Real64 GndSolarInc = s_surf->SurfGndSolarInc(SurfNum);
                        Real64 DifSolarInc = SkySolarInc + GndSolarInc;
                        s_surf->SurfWinBmSolar(SurfNum) = currBeamSolarRad * state.dataSolarShading->SurfWinTransBmSolar(SurfNum);
                        // Note: for complex fenestration, SurfWinTransDifSolar has previously been defined using the effective
                        // transmittance for sky and ground diffuse radiation (including beam radiation reflected from the ground)
                        // so these calculations should be correct
                        s_surf->SurfWinDifSolar(SurfNum) = DifSolarInc * state.dataSolarShading->SurfWinTransDifSolar(SurfNum);
                        s_surf->SurfWinBmSolarEnergy(SurfNum) = s_surf->SurfWinBmSolar(SurfNum) * state.dataGlobal->TimeStepZoneSec;
                        s_surf->SurfWinDifSolarEnergy(SurfNum) = s_surf->SurfWinDifSolar(SurfNum) * state.dataGlobal->TimeStepZoneSec;
                        if (ANY_BLIND(ShadeFlag)) {
                            auto const &surfShade = s_surf->surfShades(SurfNum);
                            auto const *matBlind = dynamic_cast<Material::MaterialBlind const *>(s_mat->materials(surfShade.blind.matNum));
                            assert(matBlind != nullptr);
                            if (matBlind->SlatOrientation == DataWindowEquivalentLayer::Orientation::Horizontal) {
                                s_surf->SurfWinDifSolar(SurfNum) = SkySolarInc * state.dataSolarShading->SurfWinTransDifSolarSky(SurfNum) +
                                                                   GndSolarInc * state.dataSolarShading->SurfWinTransDifSolarGnd(SurfNum);
                                s_surf->SurfWinDifSolarEnergy(SurfNum) = s_surf->SurfWinDifSolar(SurfNum) * state.dataGlobal->TimeStepZoneSec;
                            }
                        }

                        s_surf->SurfWinTransSolar(SurfNum) = s_surf->SurfWinBmSolar(SurfNum) + s_surf->SurfWinDifSolar(SurfNum); //[W]
                        s_surf->SurfWinTransSolarEnergy(SurfNum) = s_surf->SurfWinTransSolar(SurfNum) * state.dataGlobal->TimeStepZoneSec;
                    }

                    // added TH 12/9/2009, CR 7907 & 7809
                    s_surf->SurfWinBmBmSolar(SurfNum) = currBeamSolarRad * state.dataSolarShading->SurfWinTransBmBmSolar(SurfNum);

                    s_surf->SurfWinBmDifSolar(SurfNum) = currBeamSolarRad * state.dataSolarShading->SurfWinTransBmDifSolar(SurfNum);
                    s_surf->SurfWinBmBmSolarEnergy(SurfNum) = s_surf->SurfWinBmBmSolar(SurfNum) * state.dataGlobal->TimeStepZoneSec;
                    s_surf->SurfWinBmDifSolarEnergy(SurfNum) = s_surf->SurfWinBmDifSolar(SurfNum) * state.dataGlobal->TimeStepZoneSec;

                    // Solar not added by TDD:DOME; added to zone via TDD:DIFFUSER
                    if (surf.Class != SurfaceClass::TDD_Dome) {
                        state.dataHeatBal->ZoneTransSolar(enclosureNum) += s_surf->SurfWinTransSolar(SurfNum); //[W]
                        state.dataHeatBal->ZoneTransSolarEnergy(enclosureNum) =
                            state.dataHeatBal->ZoneTransSolar(enclosureNum) * state.dataGlobal->TimeStepZoneSec; //[J]
                        state.dataHeatBal->ZoneBmSolFrExtWinsRep(enclosureNum) += s_surf->SurfWinBmSolar(SurfNum);
                        state.dataHeatBal->ZoneDifSolFrExtWinsRep(enclosureNum) += s_surf->SurfWinDifSolar(SurfNum);
                        state.dataHeatBal->ZoneBmSolFrExtWinsRepEnergy(enclosureNum) =
                            state.dataHeatBal->ZoneBmSolFrExtWinsRep(enclosureNum) * state.dataGlobal->TimeStepZoneSec; //[J]
                        state.dataHeatBal->ZoneDifSolFrExtWinsRepEnergy(enclosureNum) =
                            state.dataHeatBal->ZoneDifSolFrExtWinsRep(enclosureNum) * state.dataGlobal->TimeStepZoneSec; //[J]
                    }
                }
            }
        } // End of second loop over surfaces in zone

    } // End of first zone loop

    // Add interior window contribution to EnclSolDB

    for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclosureNum) {
        state.dataHeatBal->EnclSolDB(enclosureNum) += state.dataHeatBal->EnclSolDBIntWin(enclosureNum);
        state.dataHeatBal->ZoneBmSolFrIntWinsRep(enclosureNum) = state.dataHeatBal->EnclSolDBIntWin(enclosureNum) * state.dataEnvrn->BeamSolarRad;
        state.dataHeatBal->ZoneBmSolFrIntWinsRepEnergy(enclosureNum) =
            state.dataHeatBal->ZoneBmSolFrIntWinsRep(enclosureNum) * state.dataGlobal->TimeStepZoneSec; //[J]
    }

    // RJH - Calculate initial distribution of diffuse solar transmitted by exterior windows into each zone
    //       to all interior surfaces in the zone
    //       Includes subsequent transmittance of diffuse solar to adjacent zones through interior windows
    CalcWinTransDifSolInitialDistribution(state);
}
void CalcAbsorbedOnExteriorOpaqueSurfaces(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   May 2017
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates solar energy absorbed on exterior opaque surfaces

    auto &s_surf = state.dataSurface;

    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
            auto &thisSpace = state.dataHeatBal->space(spaceNum);
            for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {
                auto &surf = s_surf->Surface(SurfNum);
                // TH added 3/24/2010 while debugging CR 7872
                if (!surf.ExtSolar && surf.OriginalClass != SurfaceClass::TDD_Diffuser) {
                    continue;
                }
                int const ConstrNum = s_surf->SurfActiveConstruction(SurfNum);
                int SurfNum2 = SurfNum;
                if (surf.OriginalClass == SurfaceClass::TDD_Diffuser) {
                    int PipeNum = s_surf->SurfWinTDDPipeNum(SurfNum);
                    SurfNum2 = state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome;
                }
                Real64 CosInc = state.dataHeatBal->SurfCosIncAng(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum2);
                Real64 SunLitFract = state.dataHeatBal->SurfSunlitFrac(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum2);

                //-------------------------------------------------------------------------
                // EXTERIOR BEAM SOLAR RADIATION ABSORBED ON THE OUTSIDE OF OPAQUE SURFACES
                //-------------------------------------------------------------------------

                if (SunLitFract > 0.0 && state.dataConstruction->Construct(ConstrNum).TransDiff <= 0.0) {
                    s_surf->SurfOpaqAO(SurfNum) = state.dataConstruction->Construct(ConstrNum).OutsideAbsorpSolar * CosInc * SunLitFract;

                    // Note: movable insulation, if present, is accounted for in subr. InitIntSolarDistribution,
                    // where SurfQRadSWOutMvIns is calculated from SurfOpaqQRadSWOutAbs and insulation solar absorptance
                }
            }
        }
    }
}

void CalcInteriorSolarDistributionWCESimple(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   May 2017

    // PURPOSE OF THIS SUBROUTINE:
    // For a time step, calculates solar radiation absorbed by window layers, sky and diffuse solar
    // gain into zone from exterior window, beam solar on exterior window transmitted as beam and/or diffuse
    // and interior beam from exterior window that is absorbed/transmitted by back surfaces

    using namespace MultiLayerOptics;

    auto &s_surf = state.dataSurface;

    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurf = thisSpace.HTSurfaceFirst;
            int const lastSurf = thisSpace.HTSurfaceLast;
            for (int surfNum = firstSurf; surfNum <= lastSurf; ++surfNum) {
                s_surf->SurfOpaqAI(surfNum) = 0.0;
                s_surf->SurfOpaqAO(surfNum) = 0.0;
            }
        }
    }

    for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclosureNum) {

        Real64 BABSZone = 0;
        Real64 BTOTZone = 0;
        state.dataHeatBal->EnclSolDB(enclosureNum) = 0.0;
        state.dataHeatBal->EnclSolDBIntWin(enclosureNum) = 0.0;
        state.dataHeatBal->ZoneTransSolar(enclosureNum) = 0;
        state.dataHeatBal->ZoneTransSolarEnergy(enclosureNum) = 0;
        state.dataHeatBal->ZoneBmSolFrExtWinsRep(enclosureNum) = 0;
        state.dataHeatBal->ZoneDifSolFrExtWinsRep(enclosureNum) = 0;
        state.dataHeatBal->ZoneBmSolFrExtWinsRepEnergy(enclosureNum) = 0;
        state.dataHeatBal->ZoneDifSolFrExtWinsRepEnergy(enclosureNum) = 0;
        auto &thisEnclosure = state.dataViewFactor->EnclSolInfo(enclosureNum);

        for (int const SurfNum : thisEnclosure.SurfacePtr) {
            auto &surf = s_surf->Surface(SurfNum);
            if (surf.Class != SurfaceClass::Window) {
                continue;
            }
            int SurfNum2 = 0;
            if (surf.OriginalClass == SurfaceClass::TDD_Diffuser) {
                int PipeNum = s_surf->SurfWinTDDPipeNum(SurfNum);
                SurfNum2 = state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome;
            } else {
                SurfNum2 = SurfNum;
            }
            auto &window = s_surf->SurfaceWindow(SurfNum2);
            Real64 CosInc = state.dataHeatBal->SurfCosIncAng(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum2); // Note: surfnum 2
            Real64 SunLitFract = state.dataHeatBal->SurfSunlitFrac(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum2);

            std::pair<Real64, Real64> incomingAngle = getSunWCEAngles(state, SurfNum2, BSDFDirection::Incoming);
            Real64 Theta = incomingAngle.first;
            Real64 Phi = incomingAngle.second;

            int ConstrNum = s_surf->Surface(SurfNum2).Construction;
            if (s_surf->Surface(SurfNum2).activeShadedConstruction > 0) {
                ConstrNum = s_surf->Surface(SurfNum2).activeShadedConstruction;
            }
            auto aLayer = // (AUTO_OK_OBJ)
                CWindowConstructionsSimplified::instance(state).getEquivalentLayer(state, WavelengthRange::Solar, ConstrNum);

            ///////////////////////////////////////////////
            // Solar absorbed in window layers
            ///////////////////////////////////////////////
            if (state.dataHeatBal->SurfSunlitFracWithoutReveal(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum2) > 0.0) {
                size_t numOfLayers = aLayer->getNumOfLayers();
                if (s_surf->SurfWinWindowModelType(SurfNum) == WindowModel::BSDF) {
                    int CurrentState = s_surf->SurfaceWindow(SurfNum).ComplexFen.CurrentState;
                    auto &cplxState = s_surf->SurfaceWindow(SurfNum).ComplexFen.State(CurrentState);
                    for (size_t Lay = 1; Lay <= numOfLayers; ++Lay) {
                        // Simon: Important note about this equation is to use BeamSolarRad and not SurfQRadSWOutIncident
                        // is because BeamSolarRad is direct normal radiation (looking at the Sun) while SurfRadSWOutIncident
                        // is normal to window incidence. Since BSDF coefficients are taking into account angle of incidence,
                        // BeamSolarRad should be used in this case
                        state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, Lay) =
                            cplxState.WinSkyFtAbs(Lay) * s_surf->SurfSkySolarInc(SurfNum2) +
                            cplxState.WinSkyGndAbs(Lay) * s_surf->SurfGndSolarInc(SurfNum2) +
                            s_surf->SurfWinA(SurfNum, Lay) * state.dataEnvrn->BeamSolarRad +
                            s_surf->SurfWinACFOverlap(SurfNum, Lay) * state.dataEnvrn->BeamSolarRad;
                        state.dataHeatBal->SurfWinQRadSWwinAbsLayer(SurfNum, Lay) = state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, Lay) * surf.Area;
                        s_surf->SurfWinADiffFront(SurfNum, Lay) = cplxState.WinSkyGndAbs(Lay);
                    }
                } else {
                    for (size_t Lay = 1; Lay <= numOfLayers; ++Lay) {
                        Real64 AbWinBeam = aLayer->getAbsorptanceLayer(Lay, Side::Front, ScatteringSimple::Direct, Theta, Phi) *
                                           window.OutProjSLFracMult[state.dataGlobal->HourOfDay];
                        Real64 AbWinDiffFront = aLayer->getAbsorptanceLayer(Lay, Side::Front, ScatteringSimple::Diffuse, Theta, Phi);
                        //                        Real64 AbWinDiffBack = aLayer->getAbsorptanceLayer(Lay, Side::Back, ScatteringSimple::Diffuse,
                        //                        Theta, Phi);

                        // Simon: This should not be multiplied with cosine of incident angle. This however gives same
                        // results as BSDF and Winkelmann models.
                        s_surf->SurfWinA(SurfNum, Lay) =
                            AbWinBeam * CosInc * SunLitFract * s_surf->SurfaceWindow(SurfNum).OutProjSLFracMult[state.dataGlobal->HourOfDay];
                        s_surf->SurfWinADiffFront(SurfNum, Lay) = AbWinDiffFront;

                        // Simon: Same not as for BSDF. Normal solar radiation should be taken here because angle of
                        // incidence is already taken into account
                        Real64 absBeam = s_surf->SurfWinA(SurfNum, Lay) * state.dataEnvrn->BeamSolarRad;
                        Real64 absDiff =
                            s_surf->SurfWinADiffFront(SurfNum, Lay) * (s_surf->SurfSkySolarInc(SurfNum2) + s_surf->SurfGndSolarInc(SurfNum2));
                        state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, Lay) = (absBeam + absDiff);
                        state.dataHeatBal->SurfWinQRadSWwinAbsLayer(SurfNum, Lay) = state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, Lay) * surf.Area;
                    }
                }
            }

            ////////////////////////////////////////////////////////////////////
            // SKY AND GROUND DIFFUSE SOLAR GAIN INTO ZONE FROM EXTERIOR WINDOW
            ////////////////////////////////////////////////////////////////////
            constexpr Real64 minLambda{0.3};
            constexpr Real64 maxLambda{2.5};
            const Real64 Tdiff =
                aLayer->getPropertySimple(minLambda, maxLambda, PropertySimple::T, Side::Front, Scattering::DiffuseDiffuse, Theta, Phi);
            state.dataConstruction->Construct(ConstrNum).TransDiff = Tdiff;
            Real64 EnclSolDSWin = s_surf->SurfSkySolarInc(SurfNum2) * Tdiff * s_surf->Surface(SurfNum2).Area;
            if ((state.dataEnvrn->DifSolarRad != 0)) {
                EnclSolDSWin /= state.dataEnvrn->DifSolarRad;
            } else {
                EnclSolDSWin /= 1e-8;
            }

            Real64 EnclSolDGWin = s_surf->SurfGndSolarInc(SurfNum2) * Tdiff * s_surf->Surface(SurfNum2).Area;
            (state.dataEnvrn->GndSolarRad != 0) ? EnclSolDGWin /= state.dataEnvrn->GndSolarRad : EnclSolDGWin /= 1e-8;

            ////////////////////////////////////////////////////////////////////
            // BEAM SOLAR ON EXTERIOR WINDOW TRANSMITTED AS BEAM AND/OR DIFFUSE
            ////////////////////////////////////////////////////////////////////
            Real64 TBmBm = aLayer->getPropertySimple(minLambda, maxLambda, PropertySimple::T, Side::Front, Scattering::DirectDirect, Theta, Phi);
            Real64 TBmDif = aLayer->getPropertySimple(minLambda, maxLambda, PropertySimple::T, Side::Front, Scattering::DirectDiffuse, Theta, Phi);
            Real64 SurfWinTransBmBmSolar = TBmBm * SunLitFract * CosInc * surf.Area * window.InOutProjSLFracMult[state.dataGlobal->HourOfDay];
            Real64 SurfWinTransBmDifSolar = TBmDif * SunLitFract * CosInc * surf.Area * window.InOutProjSLFracMult[state.dataGlobal->HourOfDay];
            BTOTZone += SurfWinTransBmBmSolar + SurfWinTransBmDifSolar;

            Real64 DifSolarRadiation = s_surf->SurfSkySolarInc(SurfNum2) + s_surf->SurfGndSolarInc(SurfNum2);
            s_surf->SurfWinBmSolar(SurfNum) = state.dataEnvrn->BeamSolarRad * (TBmBm + TBmDif) * surf.Area * CosInc;
            s_surf->SurfWinDifSolar(SurfNum) = DifSolarRadiation * Tdiff * surf.Area;
            s_surf->SurfWinBmSolarEnergy(SurfNum) = s_surf->SurfWinBmSolar(SurfNum) * state.dataGlobal->TimeStepZoneSec;
            s_surf->SurfWinDifSolarEnergy(SurfNum) = s_surf->SurfWinDifSolar(SurfNum) * state.dataGlobal->TimeStepZoneSec;
            s_surf->SurfWinTransSolar(SurfNum) = s_surf->SurfWinBmSolar(SurfNum) + s_surf->SurfWinDifSolar(SurfNum);
            s_surf->SurfWinTransSolarEnergy(SurfNum) = s_surf->SurfWinTransSolar(SurfNum) * state.dataGlobal->TimeStepZoneSec;

            // Add beam solar absorbed by outside reveal to outside of window's base surface.
            // Add beam solar absorbed by inside reveal to inside of window's base surface.
            // This ignores 2-D heat transfer effects.
            int BaseSurfNum = surf.BaseSurf;
            s_surf->SurfOpaqAI(BaseSurfNum) = s_surf->SurfWinBmSolAbsdInsReveal(SurfNum2) / s_surf->Surface(BaseSurfNum).Area;
            s_surf->SurfOpaqAO(BaseSurfNum) = s_surf->SurfWinBmSolAbsdOutsReveal(SurfNum2) / s_surf->Surface(BaseSurfNum).Area;

            ////////////////////////////////////////////////////////////////////
            // BEAM SOLAR ON EXTERIOR WINDOW TRANSMITTED AS BEAM AND/OR DIFFUSE
            ////////////////////////////////////////////////////////////////////
            Real64 TBm = TBmBm;
            // Correction for beam absorbed by inside reveal
            Real64 TBmDenom = SunLitFract * CosInc * surf.Area * window.InOutProjSLFracMult[state.dataGlobal->HourOfDay];
            if (TBmDenom != 0.0) { // when =0.0, no correction
                TBm -= s_surf->SurfWinBmSolAbsdInsReveal(SurfNum) / TBmDenom;
            }

            TBm = max(0.0, TBm);
            TBm *= surf.IncSolMultiplier;

            int NumOfBackSurf = state.dataShadowComb->ShadowComb(BaseSurfNum).NumBackSurf;

            if (state.dataHeatBal->SolarDistribution == DataHeatBalance::Shadowing::FullInteriorExterior) {
                for (int IBack = 1; IBack <= NumOfBackSurf; ++IBack) {

                    int const BackSurfNum =
                        state.dataHeatBal->SurfWinBackSurfaces(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, IBack, SurfNum);

                    if (BackSurfNum == 0) {
                        break; // No more irradiated back surfaces for this exterior window
                    }
                    int ConstrNumBack = s_surf->Surface(BackSurfNum).Construction;
                    auto const &constrBack = state.dataConstruction->Construct(ConstrNumBack);
                    // NBackGlass = Construct( ConstrNumBack ).TotGlassLayers;
                    // Irradiated (overlap) area for this back surface, projected onto window plane
                    // (includes effect of shadowing on exterior window)
                    Real64 AOverlap = state.dataHeatBal->SurfWinOverlapAreas(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, IBack, SurfNum);
                    Real64 BOverlap = TBm * AOverlap * CosInc; //[m2]

                    if (constrBack.TransDiff <= 0.0) {
                        // Back surface is opaque interior or exterior wall

                        Real64 AbsIntSurf = state.dataHeatBalSurf->SurfAbsSolarInt(BackSurfNum);
                        s_surf->SurfOpaqAI(BackSurfNum) += BOverlap * AbsIntSurf / s_surf->Surface(BackSurfNum).Area; //[-]
                        BABSZone += BOverlap * AbsIntSurf;                                                            //[m2]
                    }
                }
            } else {
                for (int const FloorNum : thisEnclosure.SurfacePtr) {
                    // In following, ISABSF is zero except for nominal floor surfaces
                    if (!s_surf->Surface(FloorNum).HeatTransSurf) {
                        continue;
                    }
                    if (state.dataSolarShading->SurfIntAbsFac(FloorNum) <= 0.0 || FloorNum == SurfNum) {
                        continue; // Keep only floor surfaces
                    }

                    Real64 BTOTWinZone = TBm * SunLitFract * surf.Area * CosInc * window.InOutProjSLFracMult[state.dataGlobal->HourOfDay]; //[m2]

                    if (state.dataConstruction->Construct(s_surf->Surface(FloorNum).Construction).TransDiff <= 0.0) {
                        // Opaque surface
                        s_surf->SurfOpaqAI(FloorNum) +=
                            BTOTWinZone * state.dataSolarShading->SurfIntAbsFac(FloorNum) / s_surf->Surface(FloorNum).Area; //[-]
                    }
                }
            }
            state.dataHeatBal->ZoneTransSolar(enclosureNum) += s_surf->SurfWinTransSolar(SurfNum); //[W]
            state.dataHeatBal->ZoneTransSolarEnergy(enclosureNum) =
                state.dataHeatBal->ZoneTransSolar(enclosureNum) * state.dataGlobal->TimeStepZoneSec; //[J]
            state.dataHeatBal->ZoneBmSolFrExtWinsRep(enclosureNum) += s_surf->SurfWinBmSolar(SurfNum);
            state.dataHeatBal->ZoneDifSolFrExtWinsRep(enclosureNum) += s_surf->SurfWinDifSolar(SurfNum);
            state.dataHeatBal->ZoneBmSolFrExtWinsRepEnergy(enclosureNum) =
                state.dataHeatBal->ZoneBmSolFrExtWinsRep(enclosureNum) * state.dataGlobal->TimeStepZoneSec; //[J]
            state.dataHeatBal->ZoneDifSolFrExtWinsRepEnergy(enclosureNum) =
                state.dataHeatBal->ZoneDifSolFrExtWinsRep(enclosureNum) * state.dataGlobal->TimeStepZoneSec; //[J]
        }
        state.dataHeatBal->EnclSolDB(enclosureNum) = BTOTZone - BABSZone;
    }
}

int WindowScheduledSolarAbs(EnergyPlusData &state,
                            int const SurfNum, // Surface number
                            int const ConstNum // Construction number
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   June 2013

    // PURPOSE OF THIS SUBROUTINE:
    // Returns scheduled surface gain object for given surface-construction combination
    auto &s_surf = state.dataSurface;

    for (int i = 1; i <= s_surf->TotFenLayAbsSSG; ++i) {
        if ((s_surf->FenLayAbsSSG(i).SurfPtr == SurfNum) && (s_surf->FenLayAbsSSG(i).ConstrPtr == ConstNum)) {
            return i;
        }
    }

    return 0;
}

int SurfaceScheduledSolarInc(EnergyPlusData &state,
                             int const SurfNum, // Surface number
                             int const ConstNum // Construction number
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   June 2013

    // PURPOSE OF THIS SUBROUTINE:
    // Returns scheduled surface gain pointer for given surface-construction combination

    auto &s_surf = state.dataSurface;

    for (int i = 1; i <= s_surf->TotSurfIncSolSSG; ++i) {
        if ((s_surf->SurfIncSolSSG(i).SurfPtr == SurfNum) && (s_surf->SurfIncSolSSG(i).ConstrPtr == ConstNum)) {
            return i;
        }
    }

    return 0;
}

void PerformSolarCalculations(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   July 1999
    //       MODIFIED       Sept 2003, FCW: add calls to CalcBeamSolDiffuseReflFactors and
    //                       CalcBeamSolSpecularReflFactors
    //                      Jan 2004, FCW: call CalcDayltgCoefficients if storm window status on
    //                       any window has changed
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine determines if new solar/shading calculations need
    // to be performed and calls the proper routines to do the job.

    // METHODOLOGY EMPLOYED:
    // Users are allowed to enter a value for number of days in each period that
    // will be used for calculating solar.  (Later, this could be more complicated as
    // in allowing a number of days in a month or something).  Using this value or the
    // default (20 days) if nothing is entered by the user, the routine will use the
    // number of days left to determine if a new set of calculations should be done.
    // The calculations use the average of "equation of time" and "solar declination"
    // to perform the calculations.

    // REFERENCES:
    // na

    // Using/Aliasing
    using Dayltg::CalcDayltgCoefficients;
    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:
    // na

    // SUBROUTINE PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS
    // na

    // DERIVED TYPE DEFINITIONS
    // na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 SumDec;
    Real64 SumET;
    Real64 AvgEqOfTime;
    Real64 AvgSinSolarDeclin;
    Real64 AvgCosSolarDeclin;
    int PerDayOfYear;
    int Count;
    Real64 SinDec;
    Real64 EqTime;
    // not used INTEGER SurfNum

    auto &s_surf = state.dataSurface;
    // Calculate sky diffuse shading

    if (state.dataGlobal->BeginSimFlag) {
        state.dataSolarShading->CalcSkyDifShading = true;
        SkyDifSolarShading(state); // Calculate factors for shading of sky diffuse solar
        state.dataSolarShading->CalcSkyDifShading = false;
    }

    if (state.dataGlobal->BeginEnvrnFlag) {
        state.dataSolarShading->ShadowingDaysLeft = 0;
    }

    if (state.dataSolarShading->ShadowingDaysLeft <= 0 || state.dataSysVars->DetailedSolarTimestepIntegration) {

        if (!state.dataSysVars->DetailedSolarTimestepIntegration) {
            //  Perform calculations.
            state.dataSolarShading->ShadowingDaysLeft = state.dataSolarShading->ShadowingCalcFrequency;
            if (state.dataGlobal->DayOfSim + state.dataSolarShading->ShadowingDaysLeft > state.dataGlobal->NumOfDayInEnvrn) {
                state.dataSolarShading->ShadowingDaysLeft = state.dataGlobal->NumOfDayInEnvrn - state.dataGlobal->DayOfSim + 1;
            }

            //  Calculate average Equation of Time, Declination Angle for this period

            if (!state.dataGlobal->WarmupFlag) {
                if (state.dataGlobal->KindOfSim == Constant::KindOfSim::RunPeriodWeather) {
                    DisplayString(state, "Updating Shadowing Calculations, Start Date=" + state.dataEnvrn->CurMnDyYr);
                } else {
                    DisplayString(state, "Updating Shadowing Calculations, Start Date=" + state.dataEnvrn->CurMnDy);
                }
                state.dataReportFlag->DisplayPerfSimulationFlag = true;
            }

            PerDayOfYear = state.dataEnvrn->DayOfYear;
            SumDec = 0.0;
            SumET = 0.0;
            for (Count = 1; Count <= state.dataSolarShading->ShadowingDaysLeft; ++Count) {
                SUN3(PerDayOfYear, SinDec, EqTime);
                SumDec += SinDec;
                SumET += EqTime;
                ++PerDayOfYear;
            }

            //  Compute Period Values
            AvgSinSolarDeclin = SumDec / double(state.dataSolarShading->ShadowingDaysLeft);
            AvgCosSolarDeclin = std::sqrt(1.0 - pow_2(AvgSinSolarDeclin));
            AvgEqOfTime = SumET / double(state.dataSolarShading->ShadowingDaysLeft);
        } else {
            SUN3(state.dataEnvrn->DayOfYear, AvgSinSolarDeclin, AvgEqOfTime);
            AvgCosSolarDeclin = std::sqrt(1.0 - pow_2(AvgSinSolarDeclin));
            // trigger display of progress in the simulation every two weeks
            if (!state.dataGlobal->WarmupFlag && state.dataGlobal->BeginDayFlag && (state.dataGlobal->DayOfSim % 14 == 0)) {
                state.dataReportFlag->DisplayPerfSimulationFlag = true;
            }
        }

        CalcPerSolarBeam(state, AvgEqOfTime, AvgSinSolarDeclin, AvgCosSolarDeclin);

        // Calculate factors for solar reflection
        if (s_surf->CalcSolRefl) {
            CalcBeamSolDiffuseReflFactors(state);
            CalcBeamSolSpecularReflFactors(state);
            if (state.dataGlobal->BeginSimFlag) {
                CalcSkySolDiffuseReflFactors(state);
            }
        }
        //  Calculate daylighting coefficients
        CalcDayltgCoefficients(state);
    }

    if (!state.dataGlobal->WarmupFlag) {
        --state.dataSolarShading->ShadowingDaysLeft;
    }

    // Recalculate daylighting coefficients if storm window has been added
    // or removed from one or more windows at beginning of day
    if (state.dataDayltg->TotWindowsWithDayl > 0 && !state.dataGlobal->BeginSimFlag && !state.dataGlobal->BeginEnvrnFlag &&
        !state.dataGlobal->WarmupFlag && s_surf->TotStormWin > 0 && state.dataHeatBal->StormWinChangeThisDay) {
        CalcDayltgCoefficients(state);
    }
}

void SHDRVL(EnergyPlusData &state,
            int const HTSS,  // Heat transfer surface number of the subsurface
            int const SBSNR, // Subsurface number
            int const Hour,
            int const TS)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy Code
    //       DATE WRITTEN
    //       MODIFIED       May 2002 (FCW): allow triangular windows to have reveal.
    //       RE-ENGINEERED  Lawrie, Oct 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine computes the shadowing from a reveal onto a subsurface.

    // REFERENCES:
    // BLAST/IBLAST code, original author George Walton

    int NVS; // Number of vertices

    int constexpr None(0);                       // for use with RevealStatus
    int constexpr EntireWindowShadedByReveal(1); // for use with RevealStatus
    int constexpr WindowShadedOnlyByReveal(2);   // for use with RevealStatus

    Real64 A; // Area
    Real64 R; // Depth of the reveal (m)
    int I;    // Loop control
    int N;    // Vertex number
    int NS1;  // Locations in homogeneous coordinate array
    int NS2;
    // note, below dimensions not changed because subsurface still max 4
    Array1D<Real64> XVT(5); // Projected X coordinates of vertices
    Array1D<Real64> YVT(5); // Projected Y coordinates of vertices
    bool RevealStatusSet;   // Used to control flow through this subroutine.
    // Certain operations performed only if reveal status not yet set.
    int RevealStatus; // Status of the reveal, takes the parameter values above

    auto &s_surf = state.dataSurface;

    RevealStatus = None;
    RevealStatusSet = false;

    if (!state.dataSolarShading->CalcSkyDifShading) {
        state.dataSolarShading->SurfWinRevealStatus(Hour, TS, SBSNR) = None;
    }

    R = s_surf->Surface(SBSNR).Reveal;
    if (R <= 0.0) {
        RevealStatus = None;
        RevealStatusSet = true;
    }

    if (!RevealStatusSet) {

        state.dataSolarShading->FRVLHC = state.dataSolarShading->LOCHCA + 1;
        ++state.dataSolarShading->LOCHCA;
        NVS = s_surf->Surface(SBSNR).Sides;

        // Currently (06May02) windows are either rectangles (NVS=4) or triangles (NVS=3)

        if (NVS == 4) { // Rectangular subsurface

            // Determine vertices of reveal.
            // Project the subsurface up to the plane of the wall.

            XVT(1) = s_surf->ShadeV(SBSNR).XV(1) + R * max(state.dataSolarShading->XShadowProjection, 0.0);
            XVT(2) = s_surf->ShadeV(SBSNR).XV(2) + R * max(state.dataSolarShading->XShadowProjection, 0.0);
            XVT(3) = s_surf->ShadeV(SBSNR).XV(3) + R * min(state.dataSolarShading->XShadowProjection, 0.0);
            XVT(4) = s_surf->ShadeV(SBSNR).XV(4) + R * min(state.dataSolarShading->XShadowProjection, 0.0);
            YVT(1) = s_surf->ShadeV(SBSNR).YV(1) + R * min(state.dataSolarShading->YShadowProjection, 0.0);
            YVT(2) = s_surf->ShadeV(SBSNR).YV(2) + R * max(state.dataSolarShading->YShadowProjection, 0.0);
            YVT(3) = s_surf->ShadeV(SBSNR).YV(3) + R * max(state.dataSolarShading->YShadowProjection, 0.0);
            YVT(4) = s_surf->ShadeV(SBSNR).YV(4) + R * min(state.dataSolarShading->YShadowProjection, 0.0);

            // Check for complete shadowing.

            if ((XVT(2) >= XVT(3)) || (YVT(2) >= YVT(1))) {

                RevealStatus = EntireWindowShadedByReveal;
                RevealStatusSet = true;

            } else {
                // Re-order vertices to clockwise.

                for (N = 1; N <= NVS; ++N) {
                    state.dataSolarShading->XVS(N) = XVT(NVS + 1 - N);
                    state.dataSolarShading->YVS(N) = YVT(NVS + 1 - N);
                }

                // Transform to homogeneous coordinates

                HTRANS1(state, state.dataSolarShading->FRVLHC, NVS);
                state.dataSolarShading->HCAREA(state.dataSolarShading->FRVLHC) = -state.dataSolarShading->HCAREA(state.dataSolarShading->FRVLHC);
                state.dataSolarShading->HCT(state.dataSolarShading->FRVLHC) = 1.0;

                if (state.dataSolarShading->HCAREA(state.dataSolarShading->FRVLHC) <= 0.0) {
                    RevealStatus = EntireWindowShadedByReveal;
                    RevealStatusSet = true;
                }
            }

        } else if (NVS == 3) { // Triangular window

            // Project window to outside plane of parent surface

            for (N = 1; N <= 3; ++N) {
                XVT(N) = s_surf->ShadeV(SBSNR).XV(N) + R * state.dataSolarShading->XShadowProjection;
                YVT(N) = s_surf->ShadeV(SBSNR).YV(N) + R * state.dataSolarShading->YShadowProjection;
            }

            // Find the overlap between the original window and the projected window
            // Put XVT,YVT in clockwise order

            for (N = 1; N <= NVS; ++N) {
                state.dataSolarShading->XVS(N) = XVT(NVS + 1 - N);
                state.dataSolarShading->YVS(N) = YVT(NVS + 1 - N);
            }

            // Transform to homogeneous coordinates

            NS1 = state.dataSolarShading->LOCHCA + 1;
            state.dataSolarShading->LOCHCA = NS1;
            HTRANS1(state, NS1, NVS);

            // Put XV,YV in clockwise order

            for (N = 1; N <= NVS; ++N) {
                state.dataSolarShading->XVS(N) = s_surf->ShadeV(SBSNR).XV(NVS + 1 - N);
                state.dataSolarShading->YVS(N) = s_surf->ShadeV(SBSNR).YV(NVS + 1 - N);
            }

            // Transform to homogeneous coordinates

            NS2 = state.dataSolarShading->LOCHCA + 1;
            state.dataSolarShading->LOCHCA = NS2;
            HTRANS1(state, NS2, NVS);
            state.dataSolarShading->HCT(state.dataSolarShading->FRVLHC) = 1.0;

            // Find overlap

            DeterminePolygonOverlap(state, NS1, NS2, state.dataSolarShading->FRVLHC);
            if (state.dataSolarShading->OverlapStatus == NoOverlap) {
                RevealStatus = EntireWindowShadedByReveal;
                RevealStatusSet = true;
            }
        }
    }

    if (!RevealStatusSet) {

        // Check for no shadows on window.

        if (state.dataSolarShading->NSBSHC <= 1) {
            RevealStatus = WindowShadedOnlyByReveal;
            RevealStatusSet = true;
        } else {
            // Reduce all previous shadows to size of reveal opening.
            state.dataSolarShading->LOCHCA = state.dataSolarShading->FRVLHC;
            MULTOL(state, state.dataSolarShading->LOCHCA, state.dataSolarShading->FSBSHC, state.dataSolarShading->NSBSHC - 1);
            if ((state.dataSolarShading->OverlapStatus == TooManyVertices) || (state.dataSolarShading->OverlapStatus == TooManyFigures)) {
                RevealStatus = None;
                RevealStatusSet = true;
            } else {
                state.dataSolarShading->NRVLHC = state.dataSolarShading->LOCHCA - state.dataSolarShading->FRVLHC + 1;
                if (state.dataSolarShading->NRVLHC <= 1) {
                    RevealStatus = WindowShadedOnlyByReveal;
                    RevealStatusSet = true;
                }
            }
        }
    }

    if (!RevealStatusSet) {
        // Compute sunlit area.
        A = state.dataSolarShading->HCAREA(state.dataSolarShading->FRVLHC);
        for (I = 2; I <= state.dataSolarShading->NRVLHC; ++I) {
            A += state.dataSolarShading->HCAREA(state.dataSolarShading->FRVLHC - 1 + I) *
                 (1.0 - state.dataSolarShading->HCT(state.dataSolarShading->FRVLHC - 1 + I));
        }
        state.dataSolarShading->SurfSunlitArea(HTSS) = A;
    }

    if ((RevealStatus == EntireWindowShadedByReveal) || (state.dataSolarShading->SurfSunlitArea(HTSS) < 0.0)) {
        state.dataSolarShading->SurfSunlitArea(HTSS) = 0.0; // Window entirely shaded by reveal.
    } else if (RevealStatus == WindowShadedOnlyByReveal) {
        state.dataSolarShading->SurfSunlitArea(HTSS) =
            state.dataSolarShading->HCAREA(state.dataSolarShading->FRVLHC); // Window shaded only by reveal.
    }

    if (!state.dataSolarShading->CalcSkyDifShading) {
        state.dataSolarShading->SurfWinRevealStatus(Hour, TS, SBSNR) = RevealStatus;
    }
}

void SHDSBS(EnergyPlusData &state,
            int const iHour, // Hour Index
            int const CurSurf,
            int const NBKS, // Number of back surfaces
            int const NSBS, // Number of subsurfaces
            int const HTS,  // Heat transfer surface number of the general receiving surf
            int const TS    // Time step Index
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy Code
    //       DATE WRITTEN
    //       MODIFIED       FCW, Oct 2002: Surface%Area --> Surface%Area + SurfaceWindow%DividerArea
    //                       in calculation of SunlitFracWithoutReveal (i.e., use full window area, not
    //                       just glass area.
    //                      TH, May 2009: Bug fixed to address part of CR 7596 - inside reveals
    //                       causing high cooling loads
    //       RE-ENGINEERED  Lawrie, Oct 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine determines the shadowing on subsurfaces and
    // revises the base surface area accordingly.  It also computes
    // the effect of transparent subsurfaces.

    // REFERENCES:
    // BLAST/IBLAST code, original author George Walton

    Real64 A;        // Area
    int I;           // Loop control
    int J;           // Loop control
    int K;           // Window construction number
    int N;           // Vertex number
    Real64 SurfArea; // Surface area. For walls, includes all window frame areas.
    // For windows, includes divider area
    //  REAL(r64) FrameAreaAdd    ! Additional frame area sunlit
    //  REAL(r64) DividerAreaAdd  ! Additional frame area sunlit
    int HTSS;  // Heat transfer surface number of the subsurface
    int SBSNR; // Subsurface number

    auto &s_surf = state.dataSurface;

    if (NSBS > 0) { // Action taken only if subsurfaces present

        state.dataSolarShading->FSBSHC = state.dataSolarShading->LOCHCA + 1;

        for (I = 1; I <= NSBS; ++I) { // Do for all subsurfaces (sbs).

            SBSNR = state.dataShadowComb->ShadowComb(CurSurf).SubSurf(I);

            HTSS = SBSNR;

            K = s_surf->Surface(SBSNR).Construction;

            if (!state.dataSolarShading->penumbra) {
                if ((state.dataSolarShading->OverlapStatus != TooManyVertices) && (state.dataSolarShading->OverlapStatus != TooManyFigures) &&
                    (state.dataSolarShading->SurfSunlitArea(HTS) > 0.0)) {

                    // Re-order vertices to clockwise sequential; compute homogeneous coordinates.
                    state.dataSolarShading->NVS = s_surf->Surface(SBSNR).Sides;
                    for (N = 1; N <= state.dataSolarShading->NVS; ++N) {
                        state.dataSolarShading->XVS(N) = s_surf->ShadeV(SBSNR).XV(state.dataSolarShading->NVS + 1 - N);
                        state.dataSolarShading->YVS(N) = s_surf->ShadeV(SBSNR).YV(state.dataSolarShading->NVS + 1 - N);
                    }
                    state.dataSolarShading->LOCHCA = state.dataSolarShading->FSBSHC;
                    HTRANS1(state, state.dataSolarShading->LOCHCA, state.dataSolarShading->NVS);
                    state.dataSolarShading->HCAREA(state.dataSolarShading->LOCHCA) = -state.dataSolarShading->HCAREA(state.dataSolarShading->LOCHCA);
                    state.dataSolarShading->HCT(state.dataSolarShading->LOCHCA) = 1.0;
                    state.dataSolarShading->NSBSHC = state.dataSolarShading->LOCHCA - state.dataSolarShading->FSBSHC + 1;

                    // Determine sunlit area of subsurface due to shadows on general receiving surface.
                    if (state.dataSolarShading->NGSSHC > 0) {
                        MULTOL(state, state.dataSolarShading->LOCHCA, state.dataSolarShading->FGSSHC - 1, state.dataSolarShading->NGSSHC);
                        if ((state.dataSolarShading->OverlapStatus != TooManyVertices) && (state.dataSolarShading->OverlapStatus != TooManyFigures)) {
                            state.dataSolarShading->NSBSHC = state.dataSolarShading->LOCHCA - state.dataSolarShading->FSBSHC + 1;
                        }
                    }
                }

                if ((state.dataSolarShading->OverlapStatus == TooManyVertices) || (state.dataSolarShading->OverlapStatus == TooManyFigures) ||
                    (state.dataSolarShading->SurfSunlitArea(HTS) <= 0.0)) { // General receiving surface totally shaded.

                    state.dataSolarShading->SurfSunlitArea(HTSS) = 0.0;

                    if (iHour > 0 && TS > 0) {
                        state.dataHeatBal->SurfSunlitFracWithoutReveal(iHour, TS, HTSS) = 0.0;
                    }

                } else if ((state.dataSolarShading->NGSSHC <= 0) || (state.dataSolarShading->NSBSHC == 1)) { // No shadows.

                    state.dataSolarShading->SurfSunlitArea(HTSS) = state.dataSolarShading->HCAREA(state.dataSolarShading->FSBSHC);
                    state.dataSolarShading->SurfSunlitArea(HTS) -=
                        state.dataSolarShading->SurfSunlitArea(HTSS); // Revise sunlit area of general receiving surface.

                    // TH. This is a bug.  SunLitFracWithoutReveal should be a ratio of area
                    // IF(IHour > 0 .AND. TS > 0) SunLitFracWithoutReveal(HTSS,IHour,TS) = &
                    //      Surface(HTSS)%NetAreaShadowCalc

                    // new code fixed part of CR 7596. TH 5/29/2009
                    if (iHour > 0 && TS > 0) {
                        state.dataHeatBal->SurfSunlitFracWithoutReveal(iHour, TS, HTSS) =
                            state.dataSolarShading->SurfSunlitArea(HTSS) / s_surf->Surface(HTSS).NetAreaShadowCalc;
                    }

                    SHDRVL(state, HTSS, SBSNR, iHour, TS); // Determine shadowing from reveal.

                    if ((state.dataSolarShading->OverlapStatus == TooManyVertices) || (state.dataSolarShading->OverlapStatus == TooManyFigures)) {
                        state.dataSolarShading->SurfSunlitArea(HTSS) = 0.0;
                    }

                } else { // Compute area.

                    A = state.dataSolarShading->HCAREA(state.dataSolarShading->FSBSHC);
                    for (J = 2; J <= state.dataSolarShading->NSBSHC; ++J) {
                        A += state.dataSolarShading->HCAREA(state.dataSolarShading->FSBSHC - 1 + J) *
                             (1.0 - state.dataSolarShading->HCT(state.dataSolarShading->FSBSHC - 1 + J));
                    }
                    state.dataSolarShading->SurfSunlitArea(HTSS) = A;
                    if (state.dataSolarShading->SurfSunlitArea(HTSS) > 0.0) {

                        state.dataSolarShading->SurfSunlitArea(HTS) -=
                            state.dataSolarShading->SurfSunlitArea(HTSS); // Revise sunlit area of general receiving surface.

                        if (iHour > 0 && TS > 0) {
                            state.dataHeatBal->SurfSunlitFracWithoutReveal(iHour, TS, HTSS) =
                                state.dataSolarShading->SurfSunlitArea(HTSS) / s_surf->Surface(HTSS).Area;
                        }

                        SHDRVL(state, HTSS, SBSNR, iHour, TS); // Determine shadowing from reveal.

                        if ((state.dataSolarShading->OverlapStatus == TooManyVertices) || (state.dataSolarShading->OverlapStatus == TooManyFigures)) {
                            state.dataSolarShading->SurfSunlitArea(HTSS) = 0.0;
                        }

                    } else { // General receiving surface totally shaded.

                        state.dataSolarShading->SurfSunlitArea(HTSS) = 0.0;
                    }
                }
            }

            // Determine transmittance and absorptances of sunlit window.
            if (state.dataConstruction->Construct(K).TransDiff > 0.0) {

                if (!state.dataSolarShading->CalcSkyDifShading) { // Overlaps calculation is only done for beam solar
                    // shading, not for sky diffuse solar shading

                    CalcInteriorSolarOverlaps(state, iHour, NBKS, HTSS, CurSurf, TS);
                }
            }

            // Error checking.
            SurfArea = s_surf->Surface(SBSNR).NetAreaShadowCalc;
            state.dataSolarShading->SurfSunlitArea(HTSS) = max(0.0, state.dataSolarShading->SurfSunlitArea(HTSS));

            state.dataSolarShading->SurfSunlitArea(HTSS) = min(state.dataSolarShading->SurfSunlitArea(HTSS), SurfArea);

        } // End of subsurface loop
    }
}

void SUN3(int const JulianDayOfYear,      // Julian Day Of Year
          Real64 &SineOfSolarDeclination, // Sine of Solar Declination
          Real64 &EquationOfTime          // Equation of Time (Degrees)
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy Code
    //       DATE WRITTEN
    //       MODIFIED       na
    //       RE-ENGINEERED  Linda K. Lawrie

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine computes the coefficients for determining
    // the solar position.

    // METHODOLOGY EMPLOYED:
    // The expressions are based on least-squares fits of data on p.316 of 'Thermal
    // Environmental Engineering' by Threlkeld and on p.387 of the ASHRAE Handbook
    // of Fundamentals (need date of ASHRAE HOF).

    // REFERENCES:
    // BLAST/IBLAST code, original author George Walton

    // Fitted coefficients of Fourier series | Sine of declination coefficients
    static constexpr std::array<Real64, 9> SineSolDeclCoef = {
        0.00561800, 0.0657911, -0.392779, 0.00064440, -0.00618495, -0.00010101, -0.00007951, -0.00011691, 0.00002096};
    // Fitted coefficients of Fourier Series | Equation of Time coefficients
    static constexpr std::array<Real64, 9> EqOfTimeCoef = {
        0.00021971, -0.122649, 0.00762856, -0.156308, -0.0530028, -0.00388702, -0.00123978, -0.00270502, -0.00167992};

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 X;     // Day of Year in Radians (Computed from Input JulianDayOfYear)
    Real64 CosX;  // COS(X)
    Real64 SineX; // SIN(X)

    X = 0.017167 * JulianDayOfYear; // Convert julian date to angle X

    // Calculate sines and cosines of X
    SineX = std::sin(X);
    CosX = std::cos(X);

    SineOfSolarDeclination = SineSolDeclCoef[0] + SineSolDeclCoef[1] * SineX + SineSolDeclCoef[2] * CosX + SineSolDeclCoef[3] * (SineX * CosX * 2.0) +
                             SineSolDeclCoef[4] * (pow_2(CosX) - pow_2(SineX)) +
                             SineSolDeclCoef[5] * (SineX * (pow_2(CosX) - pow_2(SineX)) + CosX * (SineX * CosX * 2.0)) +
                             SineSolDeclCoef[6] * (CosX * (pow_2(CosX) - pow_2(SineX)) - SineX * (SineX * CosX * 2.0)) +
                             SineSolDeclCoef[7] * (2.0 * (SineX * CosX * 2.0) * (pow_2(CosX) - pow_2(SineX))) +
                             SineSolDeclCoef[8] * (pow_2(pow_2(CosX) - pow_2(SineX)) - pow_2(SineX * CosX * 2.0));

    EquationOfTime = EqOfTimeCoef[0] + EqOfTimeCoef[1] * SineX + EqOfTimeCoef[2] * CosX + EqOfTimeCoef[3] * (SineX * CosX * 2.0) +
                     EqOfTimeCoef[4] * (pow_2(CosX) - pow_2(SineX)) +
                     EqOfTimeCoef[5] * (SineX * (pow_2(CosX) - pow_2(SineX)) + CosX * (SineX * CosX * 2.0)) +
                     EqOfTimeCoef[6] * (CosX * (pow_2(CosX) - pow_2(SineX)) - SineX * (SineX * CosX * 2.0)) +
                     EqOfTimeCoef[7] * (2.0 * (SineX * CosX * 2.0) * (pow_2(CosX) - pow_2(SineX))) +
                     EqOfTimeCoef[8] * (pow_2(pow_2(CosX) - pow_2(SineX)) - pow_2(SineX * CosX * 2.0));
}

void SUN4(EnergyPlusData &state,
          Real64 const CurrentTime,    // Time to use in shadowing calculations
          Real64 const EqOfTime,       // Equation of time for current day
          Real64 const SinSolarDeclin, // Sine of the Solar declination (current day)
          Real64 const CosSolarDeclin  // Cosine of the Solar declination (current day)
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy Code
    //       DATE WRITTEN
    //       MODIFIED       na
    //       RE-ENGINEERED  Lawrie, Oct 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine computes solar direction cosines for a given hour.  These
    // cosines are used in the shadowing calculations.
    // REFERENCES:
    // BLAST/IBLAST code, original author George Walton

    Real64 H;       // Hour angle (before noon = +) (in radians)
    Real64 HrAngle; // Basic hour angle

    // Compute the hour angle
    HrAngle = (15.0 * (12.0 - (CurrentTime + EqOfTime)) + (state.dataEnvrn->TimeZoneMeridian - state.dataEnvrn->Longitude));
    H = HrAngle * Constant::DegToRad;

    // Compute the cosine of the solar zenith angle.
    state.dataSolarShading->SUNCOS(3) = SinSolarDeclin * state.dataEnvrn->SinLatitude + CosSolarDeclin * state.dataEnvrn->CosLatitude * std::cos(H);
    state.dataSolarShading->SUNCOS(2) = 0.0;
    state.dataSolarShading->SUNCOS(1) = 0.0;

    if (state.dataSolarShading->SUNCOS(3) < DataEnvironment::SunIsUpValue) {
        return; // Return if sun not above horizon.
    }

    // Compute other direction cosines.
    state.dataSolarShading->SUNCOS(2) = SinSolarDeclin * state.dataEnvrn->CosLatitude - CosSolarDeclin * state.dataEnvrn->SinLatitude * std::cos(H);
    state.dataSolarShading->SUNCOS(1) = CosSolarDeclin * std::sin(H);
}

void WindowShadingManager(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   December 1998
    //       MODIFIED       November 1999 (FW)
    //                      Aug 2001 (FW): change shading control names, change approach
    //                       to scheduling and glare control, add movable
    //                       insulation controls (mainly for heating reduction)
    //                      Dec 2001 (FW): add slat angle control for blinds
    //                      Aug 2002 (FW): add four new control types:
    //                        OnIfHighOutsideAirTempAndHighSolarOnWindow
    //                        OnIfHighOutsideAirTempAndHighHorizontalSolar
    //                        OnIfHighZoneAirTempAndHighSolarOnWindow
    //                        OnIfHighZoneAirTempAndHighHorizontalSolar
    //                      Dec 2002 (FW): add between-glass shade/blind
    //                      Mar 2003 (FW): allow GlareControlIsActive = .TRUE. only for daylit zones
    //                      Apr 2003 (FW): use SNLoadCoolRate or SNLoadHeatRate only if not first time step
    //                                     (fixes problem when used first time thru and not allocated)
    //                      May 2006 (RR): add exterior window screen
    //                      May 2009 (BG): add EMS actuator override for shade flag and slat angle
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // For windows with shading, selects the shaded construction
    // that is used in the heat balance calculation, and sets
    // the window shading flag, which is:
    //  -1: if window has no shading device
    //   0: if shading device is off
    //   1: if interior shade is on
    //   2: if glazing is switched to darker state
    //   3: if exterior shade is on
    //   6: if interior blind is on
    //   7: if exterior blind is on
    //   8: if between-glass shade is on
    //   9: if between-glass blind is on
    //  10: window has interior shade that is off but may be triggered on later
    //       to control daylight glare
    //  20: window has switchable glazing that is unswitched but may be switched later
    //       to control daylight glare or daylight illuminance
    //  30: window has exterior shade that is off but may be triggered on later
    //       to control daylight glare or daylight illuminance
    //  60: window has interior blind that is off but may be triggered on later
    //       to control daylight glare or daylight illuminance
    //  70: window has exterior blind that is off but may be triggered on later
    //       to control daylight glare or daylight illuminance
    //  80: window has between-glass shade that is off but may be triggered on later
    //       to control daylight glare or daylight illuminance
    //  90: window has between-glass blind that is off but may be triggered on later
    //       to control daylight glare or daylight illuminance
    // A "shading device" may be an exterior, interior or between-glass shade or blind,
    // or the lower-transmitting (dark) state of switchable glazing (e.g., electrochromic).
    // In all cases, the unshaded condition is represented
    // by the construction given by window's Surface()%Construction and
    // the shaded condition is represented by the construction given by
    // the window's Surface()%ShadedConstruction
    // REFERENCES:
    // na

    int IConst; // Construction

    auto &s_mat = state.dataMaterial;
    auto &s_surf = state.dataSurface;

    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurfWin = thisSpace.WindowSurfaceFirst;
            int const lastSurfWin = thisSpace.WindowSurfaceLast;
            for (int ISurf = firstSurfWin; ISurf <= lastSurfWin; ++ISurf) {
                auto &surfWin = s_surf->SurfaceWindow(ISurf);
                s_surf->SurfWinExtIntShadePrevTS(ISurf) = s_surf->SurfWinShadingFlag(ISurf);

                s_surf->SurfWinShadingFlag(ISurf) = WinShadingType::NoShade;
                s_surf->SurfWinFracTimeShadingDeviceOn(ISurf) = 0.0;
                if (s_surf->SurfWinWindowModelType(ISurf) == WindowModel::EQL) {
                    int EQLNum = state.dataConstruction->Construct(s_surf->Surface(ISurf).Construction).EQLConsPtr;
                    if (state.dataWindowEquivLayer->CFS(EQLNum).VBLayerPtr > 0) {
                        auto &surfShade = s_surf->surfShades(ISurf);
                        if (state.dataWindowEquivLayer->CFS(EQLNum).L(state.dataWindowEquivLayer->CFS(EQLNum).VBLayerPtr).CNTRL ==
                            state.dataWindowEquivalentLayer->lscNONE) {
                            surfShade.blind.slatAngDeg =
                                state.dataWindowEquivLayer->CFS(EQLNum).L(state.dataWindowEquivLayer->CFS(EQLNum).VBLayerPtr).PHI_DEG;
                        } else {
                            surfShade.blind.slatAngDeg = 0.0;
                        }
                    }
                }

                // Initialization of complex fenestration shading device
                if (s_surf->SurfWinWindowModelType(ISurf) == WindowModel::BSDF) {
                    auto &construction = state.dataConstruction->Construct(s_surf->Surface(ISurf).Construction);
                    int TotLayers = construction.TotLayers;
                    for (int Lay = 1; Lay <= TotLayers; ++Lay) {
                        const int LayPtr = construction.LayerPoint(Lay);
                        auto const *material = s_mat->materials(LayPtr);
                        const bool isShading = material->group == Material::Group::ComplexShade;
                        if (isShading && Lay == 1) {
                            s_surf->SurfWinShadingFlag(ISurf) = WinShadingType::ExtShade;
                        }
                        if (isShading && Lay == TotLayers) {
                            s_surf->SurfWinShadingFlag(ISurf) = WinShadingType::IntShade;
                        }
                    }

                    if (s_surf->SurfWinShadingFlag(ISurf) == WinShadingType::IntShade) {
                        auto &surfShade = s_surf->surfShades(ISurf);
                        auto &construction = state.dataConstruction->Construct(s_surf->Surface(ISurf).Construction);
                        const int TotLay = construction.TotLayers;
                        int ShadingLayerPtr = construction.LayerPoint(TotLay);

                        auto const *matComplexShade = dynamic_cast<Material::MaterialComplexShade *>(s_mat->materials(ShadingLayerPtr));
                        Real64 TauShadeIR = matComplexShade->TransThermal;
                        Real64 EpsShadeIR = matComplexShade->BackEmissivity;
                        Real64 RhoShadeIR = max(0.0, 1.0 - TauShadeIR - EpsShadeIR);
                        // Get properties of glass next to inside shading layer
                        int GlassLayPtr = construction.LayerPoint(TotLay - 2);
                        Real64 EpsGlassIR = s_mat->materials(GlassLayPtr)->AbsorpThermalBack;
                        Real64 RhoGlassIR = 1 - EpsGlassIR;

                        surfShade.effShadeEmi = EpsShadeIR * (1.0 + RhoGlassIR * TauShadeIR / (1.0 - RhoGlassIR * RhoShadeIR));
                        surfShade.effGlassEmi = EpsGlassIR * TauShadeIR / (1.0 - RhoGlassIR * RhoShadeIR);
                    }
                }

                if (s_surf->Surface(ISurf).ExtBoundCond != ExternalEnvironment) {
                    continue;
                }
                if (!s_surf->Surface(ISurf).HasShadeControl) {
                    continue;
                } else {
                    //
                }

                // Initialize switching factor (applicable only to switchable glazing) to unswitched
                s_surf->SurfWinSwitchingFactor(ISurf) = 0.0;

                IConst = s_surf->Surface(ISurf).Construction;
                // Vis trans at normal incidence of unswitched glass. Counting the GlazedFrac
                if (IConst > 0) {
                    s_surf->SurfWinVisTransSelected(ISurf) =
                        Window::POLYF(1.0, state.dataConstruction->Construct(IConst).TransVisBeamCoef) * surfWin.glazedFrac;
                }

                // Window has shading control
                // select the active window shading control and corresponding constructions
                size_t indexWindowShadingControl = selectActiveWindowShadingControlIndex(state, ISurf);
                if (!s_surf->Surface(ISurf).windowShadingControlList.empty() &&
                    indexWindowShadingControl <= s_surf->Surface(ISurf).windowShadingControlList.size() - 1) {
                    s_surf->Surface(ISurf).activeWindowShadingControl = s_surf->Surface(ISurf).windowShadingControlList[indexWindowShadingControl];
                }
                s_surf->Surface(ISurf).activeShadedConstructionPrev = s_surf->Surface(ISurf).activeShadedConstruction;
                if (!s_surf->Surface(ISurf).shadedConstructionList.empty() &&
                    indexWindowShadingControl <= s_surf->Surface(ISurf).shadedConstructionList.size() - 1) {
                    s_surf->Surface(ISurf).activeShadedConstruction = s_surf->Surface(ISurf).shadedConstructionList[indexWindowShadingControl];
                }
                s_surf->SurfWinActiveShadedConstruction(ISurf) = s_surf->Surface(ISurf).activeShadedConstruction;
                if (!s_surf->Surface(ISurf).shadedStormWinConstructionList.empty() &&
                    indexWindowShadingControl <= s_surf->Surface(ISurf).shadedStormWinConstructionList.size() - 1) {
                    if (s_surf->SurfWinStormWinFlag(ISurf) == 1) {
                        s_surf->SurfWinActiveShadedConstruction(ISurf) =
                            s_surf->Surface(ISurf).shadedStormWinConstructionList[indexWindowShadingControl];
                    }
                }

                int IShadingCtrl = s_surf->Surface(ISurf).activeWindowShadingControl;
                int IZone = s_surf->Surface(ISurf).Zone;
                // Setpoint for shading
                Real64 SetPoint = s_surf->WindowShadingControl(IShadingCtrl).SetPoint;   // Control setpoint
                Real64 SetPoint2 = s_surf->WindowShadingControl(IShadingCtrl).SetPoint2; // Second control setpoint

                bool SchedAllowsControl = true; // True if control schedule is not specified or is specified and schedule value = 1
                auto const *sched = s_surf->WindowShadingControl(IShadingCtrl).sched;
                if (sched != nullptr) {
                    if (s_surf->WindowShadingControl(IShadingCtrl).ShadingControlIsScheduled && sched->getCurrentVal() <= 0.0) {
                        SchedAllowsControl = false;
                    }
                }

                Real64 GlareControlIsActive = (state.dataDayltg->ZoneDaylight(IZone).totRefPts > 0 && state.dataEnvrn->SunIsUp &&
                                               s_surf->WindowShadingControl(IShadingCtrl).GlareControlIsActive); // True if glare control is active

                Real64 SolarOnWindow = 0.0;     // Direct plus diffuse solar intensity on window (W/m2)
                Real64 BeamSolarOnWindow = 0.0; // Direct solar intensity on window (W/m2)
                Real64 HorizSolar = 0.0;        // Horizontal direct plus diffuse solar intensity
                if (state.dataEnvrn->SunIsUp) {
                    Real64 SkySolarOnWindow = state.dataSolarShading->SurfAnisoSkyMult(ISurf) *
                                              state.dataEnvrn->DifSolarRad; // Sky diffuse solar intensity on window (W/m2)
                    BeamSolarOnWindow = state.dataEnvrn->BeamSolarRad *
                                        state.dataHeatBal->SurfCosIncAng(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, ISurf) *
                                        state.dataHeatBal->SurfSunlitFrac(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, ISurf);
                    SolarOnWindow = BeamSolarOnWindow + SkySolarOnWindow + state.dataEnvrn->GndSolarRad * s_surf->Surface(ISurf).ViewFactorGround;
                    HorizSolar = state.dataEnvrn->BeamSolarRad * state.dataEnvrn->SOLCOS(3) + state.dataEnvrn->DifSolarRad;
                }

                // Determine whether to deploy shading depending on type of control
                auto &thisIZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(IZone);

                bool shadingOn = false;
                bool shadingOffButGlareControlOn = false;
                switch (s_surf->WindowShadingControl(IShadingCtrl).shadingControlType) {
                case WindowShadingControlType::AlwaysOn: // 'ALWAYSON'
                    shadingOn = true;
                    break;
                case WindowShadingControlType::AlwaysOff: // 'ALWAYSOFF'
                    break;
                case WindowShadingControlType::OnIfScheduled: // 'ONIFSCHEDULEALLOWS'
                    if (SchedAllowsControl) {
                        shadingOn = true;
                    }
                    break;
                case WindowShadingControlType::HiSolar: // 'ONIFHIGHSOLARONWINDOW'
                    // ! Direct plus diffuse solar intensity on window
                    if (state.dataEnvrn->SunIsUp) {
                        if (SolarOnWindow > SetPoint && SchedAllowsControl) {
                            shadingOn = true;
                        } else if (GlareControlIsActive) {
                            shadingOffButGlareControlOn = true;
                        }
                    }
                    break;

                case WindowShadingControlType::HiHorzSolar: // 'ONIFHIGHHORIZONTALSOLAR'  ! Direct plus diffuse exterior horizontal solar intensity
                    if (state.dataEnvrn->SunIsUp) {
                        if (HorizSolar > SetPoint && SchedAllowsControl) {
                            shadingOn = true;
                        } else if (GlareControlIsActive) {
                            shadingOffButGlareControlOn = true;
                        }
                    }
                    break;

                case WindowShadingControlType::HiOutAirTemp: // 'OnIfHighOutdoorAirTemperature'
                    if (s_surf->SurfOutDryBulbTemp(ISurf) > SetPoint && SchedAllowsControl) {
                        shadingOn = true;
                    } else if (GlareControlIsActive) {
                        shadingOffButGlareControlOn = true;
                    }
                    break;

                case WindowShadingControlType::HiZoneAirTemp: // 'OnIfHighZoneAirTemperature'  ! Previous time step zone air temperature
                    if (thisIZoneHB.MAT > SetPoint && SchedAllowsControl) {
                        shadingOn = true;
                    } else if (GlareControlIsActive) {
                        shadingOffButGlareControlOn = true;
                    }
                    break;

                case WindowShadingControlType::OnHiOutTemp_HiSolarWindow: // 'OnIfHighOutdoorAirTempAndHighSolarOnWindow'  ! Outside air temp and
                                                                          // solar on window
                    if (state.dataEnvrn->SunIsUp) {
                        if (s_surf->SurfOutDryBulbTemp(ISurf) > SetPoint && SolarOnWindow > SetPoint2 && SchedAllowsControl) {
                            shadingOn = true;
                        } else if (GlareControlIsActive) {
                            shadingOffButGlareControlOn = true;
                        }
                    }
                    break;

                case WindowShadingControlType::OnHiOutTemp_HiHorzSolar: // 'OnIfHighOutdoorAirTempAndHighHorizontalSolar'  ! Outside air temp and
                                                                        // horizontal solar
                    if (state.dataEnvrn->SunIsUp) {
                        if (s_surf->SurfOutDryBulbTemp(ISurf) > SetPoint && HorizSolar > SetPoint2 && SchedAllowsControl) {
                            shadingOn = true;
                        } else if (GlareControlIsActive) {
                            shadingOffButGlareControlOn = true;
                        }
                    }
                    break;

                case WindowShadingControlType::OnHiZoneTemp_HiSolarWindow: // 'ONIFHIGHZONEAIRTEMPANDHIGHSOLARONWINDOW'  ! Zone air temp and solar on
                                                                           // window
                    if (state.dataEnvrn->SunIsUp) {
                        if (thisIZoneHB.MAT > SetPoint && SolarOnWindow > SetPoint2 && SchedAllowsControl) {
                            shadingOn = true;
                        } else if (GlareControlIsActive) {
                            shadingOffButGlareControlOn = true;
                        }
                    }
                    break;

                case WindowShadingControlType::OnHiZoneTemp_HiHorzSolar: // 'ONIFHIGHZONEAIRTEMPANDHIGHHORIZONTALSOLAR'  ! Zone air temp and
                                                                         // horizontal solar
                    if (state.dataEnvrn->SunIsUp) {
                        if (thisIZoneHB.MAT > SetPoint && HorizSolar > SetPoint2 && SchedAllowsControl) {
                            shadingOn = true;
                        } else if (GlareControlIsActive) {
                            shadingOffButGlareControlOn = true;
                        }
                    }
                    break;

                case WindowShadingControlType::HiZoneCooling:
                    // 'ONIFHIGHZONECOOLING'  ! Previous time step zone sensible cooling rate [W]
                    // In the following, the check on BeginSimFlag is needed since SNLoadCoolRate (and SNLoadHeatRate,
                    // used in other CASEs) are not allocated at this point for the first time step of the simulation.
                    if (!state.dataGlobal->BeginSimFlag) {
                        if (state.dataZoneEnergyDemand->ZoneSysEnergyDemand(IZone).airSysCoolRate > SetPoint && SchedAllowsControl) {
                            shadingOn = true;
                        } else if (GlareControlIsActive) {
                            shadingOffButGlareControlOn = true;
                        }
                    }
                    break;

                case WindowShadingControlType::HiGlare:
                    // 'ONIFHIGHGLARE'  ! Daylight glare index at first reference point in the zone.
                    // This type of shading control is done in DayltgInteriorIllum. Glare control is not affected
                    // by control schedule.
                    if (state.dataEnvrn->SunIsUp) {
                        shadingOffButGlareControlOn = true;
                    }
                    break;

                case WindowShadingControlType::MeetDaylIlumSetp:
                    // 'MEETDAYLIGHTILLUMINANCESETPOINT')  !  Daylight illuminance test is done in DayltgInteriorIllum
                    // Only switchable glazing does daylight illuminance control
                    if (state.dataEnvrn->SunIsUp && SchedAllowsControl) {
                        shadingOffButGlareControlOn = true;
                    }
                    break;

                case WindowShadingControlType::HiSolar_HiLumin_OffMidNight:
                    // 'OnIfHighSolarOrHighLuminanceTillMidnight'
                    // if shade is already on, then keep it on until midnight, otherwise check thresholds
                    if (SchedAllowsControl && IS_SHADED(s_surf->SurfWinExtIntShadePrevTS(ISurf))) {
                        shadingOn = true;
                    } else if (state.dataEnvrn->SunIsUp && SchedAllowsControl) {
                        if (SolarOnWindow > SetPoint) {
                            shadingOn = true;
                        } else {
                            // pass to DayltgInteriorIllum to check for luminance
                            shadingOn = false;
                            shadingOffButGlareControlOn = true;
                        }
                    }
                    // if it is the beginning of the day, then shades off
                    if (state.dataGlobal->BeginDayFlag) {
                        shadingOn = false;
                        shadingOffButGlareControlOn = false;
                    }
                    break;

                case WindowShadingControlType::HiSolar_HiLumin_OffSunset:
                    // 'OnIfHighSolarOrHighLuminanceTillSunset'
                    // if shade is already on, then keep it on until sunset, otherwise check thresholds
                    if (SchedAllowsControl && IS_SHADED(s_surf->SurfWinExtIntShadePrevTS(ISurf))) {
                        shadingOn = true;
                    } else if (state.dataEnvrn->SunIsUp && SchedAllowsControl) {
                        if (SolarOnWindow > SetPoint) {
                            shadingOn = true;
                        } else {
                            // pass to DayltgInteriorIllum to check for luminance
                            shadingOn = false;
                            shadingOffButGlareControlOn = true;
                        }
                    }
                    // if sunset, then shades off
                    if (!state.dataEnvrn->SunIsUp) {
                        shadingOn = false;
                        shadingOffButGlareControlOn = false;
                    }
                    break;

                case WindowShadingControlType::HiSolar_HiLumin_OffNextMorning:
                    // 'OnIfHighSolarOrHighLuminanceTillNextMorning'
                    // if shade is already on, then keep it on until next day when sun is up, otherwise check thresholds
                    if (SchedAllowsControl && IS_SHADED(s_surf->SurfWinExtIntShadePrevTS(ISurf))) {
                        shadingOn = true;
                    } else if (state.dataEnvrn->SunIsUp && SchedAllowsControl) {
                        if (SolarOnWindow > SetPoint) {
                            shadingOn = true;
                        } else {
                            // pass to DayltgInteriorIllum to check for luminance
                            shadingOn = false;
                            shadingOffButGlareControlOn = true;
                        }
                    }
                    // if next morning (identified by sun is not up in previous time step and is up now), then shades off
                    if (!state.dataEnvrn->SunIsUpPrevTS && state.dataEnvrn->SunIsUp) {
                        shadingOn = false;
                        shadingOffButGlareControlOn = false;
                    }
                    break;

                case WindowShadingControlType::OnNightLoOutTemp_OffDay: // 'OnNightIfLowOutdoorTempAndOffDay'
                    if (!state.dataEnvrn->SunIsUp && s_surf->SurfOutDryBulbTemp(ISurf) < SetPoint && SchedAllowsControl) {
                        shadingOn = true;
                    } else if (GlareControlIsActive) {
                        shadingOffButGlareControlOn = true;
                    }
                    break;

                case WindowShadingControlType::OnNightLoInTemp_OffDay: // 'OnNightIfLowInsideTempAndOffDay')
                    if (!state.dataEnvrn->SunIsUp && thisIZoneHB.MAT < SetPoint && SchedAllowsControl) {
                        shadingOn = true;
                    } else if (GlareControlIsActive) {
                        shadingOffButGlareControlOn = true;
                    }
                    break;

                case WindowShadingControlType::OnNightIfHeating_OffDay: // 'OnNightIfHeatingAndOffDay'
                    if (!state.dataGlobal->BeginSimFlag) {
                        if (!state.dataEnvrn->SunIsUp && state.dataZoneEnergyDemand->ZoneSysEnergyDemand(IZone).airSysHeatRate > SetPoint &&
                            SchedAllowsControl) {
                            shadingOn = true;
                        } else if (GlareControlIsActive) {
                            shadingOffButGlareControlOn = true;
                        }
                    }
                    break;

                case WindowShadingControlType::OnNightLoOutTemp_OnDayCooling: // 'OnNightIfLowOutdoorTempAndOnDayIfCooling'
                    if (!state.dataGlobal->BeginSimFlag) {
                        if (!state.dataEnvrn->SunIsUp) { // Night
                            if (s_surf->SurfOutDryBulbTemp(ISurf) < SetPoint && SchedAllowsControl) {
                                shadingOn = true;
                            }
                        } else { // Day
                            if (state.dataZoneEnergyDemand->ZoneSysEnergyDemand(IZone).airSysCoolRate > 0.0 && SchedAllowsControl) {
                                shadingOn = true;
                            } else if (GlareControlIsActive) {
                                shadingOffButGlareControlOn = true;
                            }
                        }
                    }
                    break;

                case WindowShadingControlType::OnNightIfHeating_OnDayCooling: // 'OnNightIfHeatingAndOnDayIfCooling'
                    if (!state.dataGlobal->BeginSimFlag) {
                        if (!state.dataEnvrn->SunIsUp) { // Night
                            if (state.dataZoneEnergyDemand->ZoneSysEnergyDemand(IZone).airSysHeatRate > SetPoint && SchedAllowsControl) {
                                shadingOn = true;
                            }
                        } else { // Day
                            if (state.dataZoneEnergyDemand->ZoneSysEnergyDemand(IZone).airSysCoolRate > 0.0 && SchedAllowsControl) {
                                shadingOn = true;
                            } else if (GlareControlIsActive) {
                                shadingOffButGlareControlOn = true;
                            }
                        }
                    }
                    break;

                case WindowShadingControlType::OffNight_OnDay_HiSolarWindow: // 'OffNightAndOnDayIfCoolingAndHighSolarOnWindow'
                    if (!state.dataGlobal->BeginSimFlag) {
                        if (state.dataEnvrn->SunIsUp && state.dataZoneEnergyDemand->ZoneSysEnergyDemand(IZone).airSysCoolRate > 0.0 &&
                            SchedAllowsControl) {
                            if (SolarOnWindow > SetPoint) {
                                shadingOn = true;
                            }
                        } else if (GlareControlIsActive) {
                            shadingOffButGlareControlOn = true;
                        }
                    }
                    break;

                case WindowShadingControlType::OnNight_OnDay_HiSolarWindow: // 'OnNightAndOnDayIfCoolingAndHighSolarOnWindow'
                    if (!state.dataGlobal->BeginSimFlag) {
                        if (state.dataEnvrn->SunIsUp && state.dataZoneEnergyDemand->ZoneSysEnergyDemand(IZone).airSysCoolRate > 0.0 &&
                            SchedAllowsControl) {
                            if (SolarOnWindow > SetPoint) {
                                shadingOn = true;
                            }
                        } else if (!state.dataEnvrn->SunIsUp && SchedAllowsControl) {
                            shadingOn = true;
                        } else if (GlareControlIsActive) {
                            shadingOffButGlareControlOn = true;
                        }
                    }
                    break;
                default:
                    ShowWarningError(state, format("Invalid Selection of Window Shading Control Type for Surface {}", s_surf->Surface(ISurf).Name));
                }

                WinShadingType ShType = s_surf->WindowShadingControl(IShadingCtrl).ShadingType;

                s_surf->SurfWinShadingFlag(ISurf) = WinShadingType::ShadeOff; // Initialize shading flag to off

                if (IS_SHADED(ShType)) {
                    if (shadingOn) {
                        s_surf->SurfWinShadingFlag(ISurf) = ShType;
                    } else if (shadingOffButGlareControlOn) {
                        if (ShType == WinShadingType::SwitchableGlazing) {
                            s_surf->SurfWinShadingFlag(ISurf) = WinShadingType::GlassConditionallyLightened;
                        } else if (ShType == WinShadingType::IntShade) {
                            s_surf->SurfWinShadingFlag(ISurf) = WinShadingType::IntShadeConditionallyOff;
                        } else if (ShType == WinShadingType::ExtShade) {
                            s_surf->SurfWinShadingFlag(ISurf) = WinShadingType::ExtShadeConditionallyOff;
                        } else if (ShType == WinShadingType::IntBlind) {
                            s_surf->SurfWinShadingFlag(ISurf) = WinShadingType::IntBlindConditionallyOff;
                        } else if (ShType == WinShadingType::ExtBlind) {
                            s_surf->SurfWinShadingFlag(ISurf) = WinShadingType::ExtBlindConditionallyOff;
                        } else if (ShType == WinShadingType::BGShade) {
                            s_surf->SurfWinShadingFlag(ISurf) = WinShadingType::BGShadeConditionallyOff;
                        } else if (ShType == WinShadingType::BGBlind) {
                            s_surf->SurfWinShadingFlag(ISurf) = WinShadingType::BGBlindConditionallyOff;
                        }
                    }
                }

                // Set switching factor to fully switched if ShadingFlag = 2
                if (s_surf->SurfWinShadingFlag(ISurf) == WinShadingType::SwitchableGlazing) {
                    s_surf->SurfWinSwitchingFactor(ISurf) = 1.0;

                    // Added TH 1/20/2010
                    // Vis trans at normal incidence of fully switched glass
                    IConst = s_surf->Surface(ISurf).activeShadedConstruction;
                    s_surf->SurfWinVisTransSelected(ISurf) =
                        Window::POLYF(1.0, state.dataConstruction->Construct(IConst).TransVisBeamCoef) * surfWin.glazedFrac;
                }

                // Slat angle control for blinds
                auto &surfShade = s_surf->surfShades(ISurf);
                surfShade.blind.slatAng = 0.0;
                surfShade.blind.slatAngDeg = 0.0;
                surfShade.blind.slatBlockBeam = false;
                if (ANY_BLIND(s_surf->SurfWinShadingFlag(ISurf)) || s_surf->SurfWinShadingFlag(ISurf) == WinShadingType::IntBlindConditionallyOff ||
                    s_surf->SurfWinShadingFlag(ISurf) == WinShadingType::ExtBlindConditionallyOff ||
                    s_surf->SurfWinShadingFlag(ISurf) == WinShadingType::BGBlindConditionallyOff) {
                    // Blind in place or may be in place due to glare control
                    if (surfShade.blind.matNum > 0) {
                        auto const *matBlind = dynamic_cast<Material::MaterialBlind const *>(s_mat->materials(surfShade.blind.matNum));
                        assert(matBlind != nullptr);
                        Real64 InputSlatAngle = matBlind->SlatAngle * Constant::DegToRad; // Slat angle of associated Material:WindowBlind (rad)
                        Real64 ThetaBase;                                                 // Intermediate slat angle variable (rad)
                        Real64 ThetaBlock1;                                               // Slat angles that just block beam solar (rad)
                        Real64 ThetaBlock2;

                        Real64 ProfAng = surfShade.blind.profAng =
                            Dayltg::ProfileAngle(state, ISurf, state.dataEnvrn->SOLCOS, matBlind->SlatOrientation);

                        if (ProfAng > Constant::PiOvr2 || ProfAng < -Constant::PiOvr2) {
                            ProfAng = min(max(ProfAng, -Constant::PiOvr2), Constant::PiOvr2);
                        }
                        surfShade.blind.profAngIdxLo = int((ProfAng + Constant::PiOvr2) / Material::dProfAng) + 1;
                        surfShade.blind.profAngIdxHi = std::min(Material::MaxProfAngs, surfShade.blind.profAngIdxLo + 1);

                        surfShade.blind.profAngInterpFac =
                            (ProfAng + Constant::PiOvr2 - (surfShade.blind.profAngIdxLo - 1) * Material::dProfAng) / Material::dProfAng;

                        if (matBlind->SlatWidth > matBlind->SlatSeparation && BeamSolarOnWindow > 0.0) {
                            ProfAng = surfShade.blind.profAng;
                            Real64 ThetaBase = std::acos(std::cos(ProfAng) * matBlind->SlatSeparation / matBlind->SlatWidth);
                            // There are two solutions for the slat angle that just blocks beam radiation
                            ThetaBlock1 = ProfAng + ThetaBase;
                            ThetaBlock2 = ProfAng + Constant::Pi - ThetaBase;
                            state.dataSolarShading->ThetaSmall = min(ThetaBlock1, ThetaBlock2);
                            state.dataSolarShading->ThetaBig = max(ThetaBlock1, ThetaBlock2);
                            state.dataSolarShading->ThetaMin = matBlind->MinSlatAngle * Constant::DegToRad;
                            state.dataSolarShading->ThetaMax = matBlind->MaxSlatAngle * Constant::DegToRad;
                        }

                        // TH 5/20/2010, CR 8064: Slat Width <= Slat Separation
                        if (matBlind->SlatWidth <= matBlind->SlatSeparation && BeamSolarOnWindow > 0.0) {
                            if (s_surf->WindowShadingControl(IShadingCtrl).slatAngleControl == SlatAngleControl::BlockBeamSolar) {
                                ProfAng = surfShade.blind.profAng;
                                if (std::abs(std::cos(ProfAng) * matBlind->SlatSeparation / matBlind->SlatWidth) <= 1.0) {
                                    // set to block 100% of beam solar, not necessarily to block maximum solar (beam + diffuse)
                                    ThetaBase = std::acos(std::cos(ProfAng) * matBlind->SlatSeparation / matBlind->SlatWidth);
                                    surfShade.blind.slatBlockBeam = true;
                                } else {
                                    // cannot block 100% of beam solar, turn slats to be perpendicular to sun beam to block maximal beam solar
                                    ThetaBase = 0.0;
                                }

                                // There are two solutions for the slat angle that just blocks beam radiation
                                ThetaBlock1 = ProfAng + ThetaBase;
                                ThetaBlock2 = ProfAng - ThetaBase + Constant::Pi;

                                state.dataSolarShading->ThetaSmall = min(ThetaBlock1, ThetaBlock2);
                                state.dataSolarShading->ThetaBig = max(ThetaBlock1, ThetaBlock2);
                                state.dataSolarShading->ThetaMin = matBlind->MinSlatAngle * Constant::DegToRad;
                                state.dataSolarShading->ThetaMax = matBlind->MaxSlatAngle * Constant::DegToRad;
                            }
                        }

                        //
                        Real64 slatAng = 0.0;
                        bool slatBlockBeam = false;
                        switch (s_surf->WindowShadingControl(IShadingCtrl).slatAngleControl) {

                        case SlatAngleControl::Fixed: { // 'FIXEDSLATANGLE'
                            slatAng = InputSlatAngle;

                            if ((slatAng <= state.dataSolarShading->ThetaSmall || slatAng >= state.dataSolarShading->ThetaBig) &&
                                (matBlind->SlatWidth > matBlind->SlatSeparation) && (BeamSolarOnWindow > 0.0)) {
                                slatBlockBeam = true;
                            }
                        } break;

                        case SlatAngleControl::Scheduled: { // 'SCHEDULEDSLATANGLE'
                            slatAng = s_surf->WindowShadingControl(IShadingCtrl).slatAngleSched->getCurrentVal();
                            slatAng = max(matBlind->MinSlatAngle, min(slatAng, matBlind->MaxSlatAngle)) * Constant::DegToRad;

                            if ((slatAng <= state.dataSolarShading->ThetaSmall || slatAng >= state.dataSolarShading->ThetaBig) &&
                                (matBlind->SlatWidth > matBlind->SlatSeparation) && (BeamSolarOnWindow > 0.0)) {
                                slatBlockBeam = true;
                            }
                        } break;

                        case SlatAngleControl::BlockBeamSolar: { // 'BLOCKBEAMSOLAR'
                            if (BeamSolarOnWindow > 0.0) {
                                if (matBlind->SlatSeparation >= matBlind->SlatWidth) {
                                    // TH 5/20/2010. CR 8064.
                                    // The following line of code assumes slats are always vertical/closed to minimize solar penetration
                                    // The slat angle can however change if the only goal is to block maximum amount of direct beam solar
                                    // SurfaceWindow(ISurf)%SlatAngThisTS = 0.0  ! Allows beam penetration but minimizes it

                                    if (state.dataSolarShading->ThetaSmall >= state.dataSolarShading->ThetaMin &&
                                        state.dataSolarShading->ThetaSmall <= state.dataSolarShading->ThetaMax) {
                                        slatAng = state.dataSolarShading->ThetaSmall;
                                    } else if (state.dataSolarShading->ThetaBig >= state.dataSolarShading->ThetaMin &&
                                               state.dataSolarShading->ThetaBig <= state.dataSolarShading->ThetaMax) {
                                        slatAng = state.dataSolarShading->ThetaBig;
                                    } else if (state.dataSolarShading->ThetaSmall < state.dataSolarShading->ThetaMin &&
                                               state.dataSolarShading->ThetaBig < state.dataSolarShading->ThetaMin) {
                                        slatAng = state.dataSolarShading->ThetaMin;
                                    } else if (state.dataSolarShading->ThetaSmall > state.dataSolarShading->ThetaMax &&
                                               state.dataSolarShading->ThetaBig > state.dataSolarShading->ThetaMax) {
                                        slatAng = state.dataSolarShading->ThetaMax;
                                    } else { // ThetaBig > ThetaMax and ThetaSmall < ThetaMin (no-block condition)
                                        slatAng = state.dataSolarShading->ThetaMin;
                                    }

                                } else { // Usual case -- slat width greater than slat separation
                                    if (state.dataSolarShading->ThetaSmall >= state.dataSolarShading->ThetaMin &&
                                        state.dataSolarShading->ThetaSmall <= state.dataSolarShading->ThetaMax) {
                                        slatAng = state.dataSolarShading->ThetaSmall;
                                        slatBlockBeam = true;
                                    } else if (state.dataSolarShading->ThetaBig >= state.dataSolarShading->ThetaMin &&
                                               state.dataSolarShading->ThetaBig <= state.dataSolarShading->ThetaMax) {
                                        slatAng = state.dataSolarShading->ThetaBig;
                                        slatBlockBeam = true;
                                    } else if (state.dataSolarShading->ThetaSmall < state.dataSolarShading->ThetaMin &&
                                               state.dataSolarShading->ThetaBig < state.dataSolarShading->ThetaMin) {
                                        slatAng = state.dataSolarShading->ThetaMin;
                                        slatBlockBeam = true;
                                    } else if (state.dataSolarShading->ThetaSmall > state.dataSolarShading->ThetaMax &&
                                               state.dataSolarShading->ThetaBig > state.dataSolarShading->ThetaMax) {
                                        slatAng = state.dataSolarShading->ThetaMax;
                                        slatBlockBeam = true;
                                    } else { // ThetaBig > ThetaMax and ThetaSmall < ThetaMin (no-block condition)
                                        slatAng = state.dataSolarShading->ThetaMin;
                                    }
                                }
                            } else {
                                slatAng = InputSlatAngle;
                            }
                        } break;
                        default:
                            break;
                        } // switch (slatAngControl)

                        if (surfShade.blind.slatAngDegEMSon) {
                            slatAng = Constant::DegToRad * surfShade.blind.slatAngDegEMSValue;
                        }

                        // Slat angle is changing, need to recalculate stored values
                        if (slatAng != surfShade.blind.slatAng) {
                            surfShade.blind.slatAng = slatAng;
                            surfShade.blind.slatAngDeg = surfShade.blind.slatAng * Constant::RadToDeg;

                            Material::GetSlatIndicesInterpFac(surfShade.blind.slatAng,
                                                              surfShade.blind.slatAngIdxLo,
                                                              surfShade.blind.slatAngIdxHi,
                                                              surfShade.blind.slatAngInterpFac);
                            surfShade.blind.TAR.interpSlatAng(matBlind->TARs[surfShade.blind.slatAngIdxLo],
                                                              matBlind->TARs[surfShade.blind.slatAngIdxHi],
                                                              surfShade.blind.slatAngInterpFac);

                            Real64 TauShIR = surfShade.blind.TAR.IR.Ft.Tra;
                            Real64 EpsShIR = surfShade.blind.TAR.IR.Bk.Emi;
                            Real64 RhoShIR = max(0.0, 1.0 - TauShIR - EpsShIR);

                            surfShade.effShadeEmi = EpsShIR * (1.0 + surfShade.glass.rhoIR * TauShIR / (1.0 - surfShade.glass.rhoIR * RhoShIR));
                            surfShade.effGlassEmi = surfShade.glass.epsIR * TauShIR / (1.0 - surfShade.glass.rhoIR * RhoShIR);
                        }

                        surfShade.blind.slatBlockBeam = slatBlockBeam;

                        // Air flow permeability for calculation of convective air flow between blind and glass
                        Real64 PermeabilityA = std::sin(surfShade.blind.slatAng) - matBlind->SlatThickness / matBlind->SlatSeparation;
                        Real64 PermeabilityB = 1.0 - (std::abs(matBlind->SlatWidth * std::cos(surfShade.blind.slatAng)) +
                                                      matBlind->SlatThickness * std::sin(surfShade.blind.slatAng)) /
                                                         matBlind->SlatSeparation;
                        surfShade.blind.airFlowPermeability = min(1.0, max(0.0, PermeabilityA, PermeabilityB));
                        surfShade.blind.bmBmTrans = matBlind->BeamBeamTrans(surfShade.blind.profAng, surfShade.blind.slatAng);
                    }
                } // End of check if interior or exterior or between glass blind in place

                // AR: don't need to do this anymore I don't think
                // CALL CalcScreenTransmittance to initialized all screens prior to HB calc's
                // if (s_surf->SurfWinShadingFlag(ISurf) == WinShadingType::ExtScreen && state.dataEnvrn->SunIsUp) {
                //    CalcScreenTransmittance(state, ISurf);
                // }

                // EMS Actuator Point: override setting if ems flag on
                if (s_surf->SurfWinShadingFlagEMSOn(ISurf)) {
                    WinShadingType SurfWinShadingFlagEMS = findValueInEnumeration(s_surf->SurfWinShadingFlagEMSValue(ISurf));
                    if (SurfWinShadingFlagEMS != WinShadingType::Invalid) {
                        s_surf->SurfWinShadingFlag(ISurf) = SurfWinShadingFlagEMS;
                    } else {
                        ShowWarningError(state,
                                         format("Invalid EMS value of Window Shading Control Type for Surface {}", s_surf->Surface(ISurf).Name));
                    }
                }
            } // End of surface loop
        }
    }
}

void CheckGlazingShadingStatusChange(EnergyPlusData &state)
{
    auto &s_surf = state.dataSurface;
    if (state.dataGlobal->BeginSimFlag) {
        if (state.dataWindowManager->inExtWindowModel->isExternalLibraryModel() && state.dataWindowManager->winOpticalModel->isSimplifiedModel()) {
            state.dataHeatBal->EnclRadAlwaysReCalc = true;
        } else {
            for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
                for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
                    auto &thisSpace = state.dataHeatBal->space(spaceNum);
                    for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {
                        auto &surf = s_surf->Surface(SurfNum);
                        if (state.dataConstruction->Construct(surf.Construction).isTCWindow ||
                            state.dataConstruction->Construct(surf.Construction).WindowTypeEQL) {
                            state.dataHeatBal->EnclRadAlwaysReCalc = true;
                            break;
                        }
                    }
                }
            }
        }
        if (state.dataHeatBal->EnclRadAlwaysReCalc) {
            for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclosureNum) {
                state.dataViewFactor->EnclSolInfo(enclosureNum).radReCalc = true;
            }
            for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfRadiantEnclosures; ++enclosureNum) {
                state.dataViewFactor->EnclRadInfo(enclosureNum).radReCalc = true;
            }
        }
    }
    if (state.dataHeatBal->EnclRadAlwaysReCalc) {
        return;
    }

    if (state.dataGlobal->BeginEnvrnFlag || state.dataGlobal->AnyConstrOverridesInModel || state.dataGlobal->AnySurfPropOverridesInModel) {
        for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclosureNum) {
            state.dataViewFactor->EnclSolInfo(enclosureNum).radReCalc = true;
        }
        for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfRadiantEnclosures; ++enclosureNum) {
            state.dataViewFactor->EnclRadInfo(enclosureNum).radReCalc = true;
        }
        return;
    }
    for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclosureNum) {
        state.dataViewFactor->EnclSolInfo(enclosureNum).radReCalc = false;
    }
    for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfRadiantEnclosures; ++enclosureNum) {
        state.dataViewFactor->EnclRadInfo(enclosureNum).radReCalc = false;
    }
    if (!state.dataGlobal->AndShadingControlInModel) {
        return;
    }
    for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclosureNum) {
        for (int const SurfNum : state.dataViewFactor->EnclRadInfo(enclosureNum).SurfacePtr) {
            auto &surf = s_surf->Surface(SurfNum);
            auto &surfShade = s_surf->surfShades(SurfNum);
            bool surfShadingStatusChange = s_surf->SurfWinExtIntShadePrevTS(SurfNum) != s_surf->SurfWinShadingFlag(SurfNum) ||
                                           surf.activeShadedConstruction != surf.activeShadedConstructionPrev || surfShade.blind.movableSlats;
            if (surfShadingStatusChange) {
                state.dataViewFactor->EnclSolInfo(enclosureNum).radReCalc = true;
                state.dataViewFactor->EnclRadInfo(enclosureNum).radReCalc = true;
                break;
            }
        }
    }
}

DataSurfaces::WinShadingType findValueInEnumeration(Real64 controlValue)
{
    // This is a workaround to translate EMS Shading control numerical values
    // EMS control of window shading devices involves setting the control values for shading control actuators with
    // one of these values.  The variable names can be used or replaced, it is the whole number values that trigger
    // changes in the modeling.
    // Shades and Blinds are either fully on or fully off, partial positions require multiple windows.
    // the window shading control flag values follow
    // -1: if window has no shading device
    // 0: if shading device is off
    // 1: if interior shade is on
    // 2: if glazing is switched to darker state
    // 3: if exterior shade is on
    // 4: if exterior screen is on
    // 6: if interior blind is on
    // 7: if exterior blind is on
    // 8: if between-glass shade is on
    // 9: if between-glass blind is on
    // 10: window has interior shade that is off but may be triggered on later to control daylight glare
    // 20: window has switchable glazing that is unswitched but may be switched later to control daylight glare or
    // daylight illuminance 30: window has exterior shade that is off but may be triggered on later to control
    // daylight glare or daylight illuminance 60: window has interior blind that is off but may be triggered on later
    // to control daylight glare or daylight illuminance 70: window has exterior blind that is off but may be
    // triggered on later to control daylight glare or daylight illuminance 80: window has between-glass shade that is
    // off but may be triggered on later to control daylight glare or daylight illuminance 90: window has
    // between-glass blind that is off but may be triggered on later to control daylight glare or daylight illuminance
    if (controlValue == -1.0) {
        return WinShadingType::NoShade;
    }
    if (controlValue == 0.0) {
        return WinShadingType::ShadeOff;
    }
    if (controlValue == 1.0) {
        return WinShadingType::IntShade;
    }
    if (controlValue == 2.0) {
        return WinShadingType::SwitchableGlazing;
    }
    if (controlValue == 3.0) {
        return WinShadingType::ExtShade;
    }
    if (controlValue == 4.0) {
        return WinShadingType::ExtScreen;
    }
    if (controlValue == 6.0) {
        return WinShadingType::IntBlind;
    }
    if (controlValue == 7.0) {
        return WinShadingType::ExtBlind;
    }
    if (controlValue == 8.0) {
        return WinShadingType::BGShade;
    }
    if (controlValue == 9.0) {
        return WinShadingType::BGBlind;
    }
    if (controlValue == 10.0) {
        return WinShadingType::IntShadeConditionallyOff;
    }
    if (controlValue == 20.0) {
        return WinShadingType::GlassConditionallyLightened;
    }
    if (controlValue == 30.0) {
        return WinShadingType::ExtShadeConditionallyOff;
    }
    if (controlValue == 60.0) {
        return WinShadingType::IntBlindConditionallyOff;
    }
    if (controlValue == 70.0) {
        return WinShadingType::ExtBlindConditionallyOff;
    }
    if (controlValue == 80.0) {
        return WinShadingType::BGShadeConditionallyOff;
    }
    if (controlValue == 90.0) {
        return WinShadingType::BGBlindConditionallyOff;
    }
    return WinShadingType::Invalid;
}

int selectActiveWindowShadingControlIndex(EnergyPlusData &state, int curSurface)
{
    auto &s_surf = state.dataSurface;
    // For a given surface, determine based on the schedules which index to the window shading control list vector
    // should be active
    // presume it is the first shading control - even if it is not active it needs to be some
    // shading control which is then turned off in the WindowShadingManager
    if (s_surf->Surface(curSurface).windowShadingControlList.size() > 1) {
        for (std::size_t listIndex = 0; listIndex < s_surf->Surface(curSurface).windowShadingControlList.size(); ++listIndex) {
            int wsc = s_surf->Surface(curSurface).windowShadingControlList[listIndex];
            // pick the first WindowShadingControl that has a non-zero schedule value
            if (s_surf->WindowShadingControl(wsc).sched->getCurrentVal() > 0.0) {
                return listIndex;
            }
        }
    }
    return 0;
}

void WindowGapAirflowControl(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   February 2003
    //       MODIFIED       June 2003, FCW: add fatal error for illegal schedule value
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // For airflow windows, determines the airflow in the gap of
    // double glazing and in the inner gap of triple glazing.

    // REFERENCES:
    // na

    auto &s_surf = state.dataSurface;
    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurfWin = thisSpace.WindowSurfaceFirst;
            int const lastSurfWin = thisSpace.WindowSurfaceLast;
            for (int ISurf = firstSurfWin; ISurf <= lastSurfWin; ++ISurf) {

                s_surf->SurfWinAirflowThisTS(ISurf) = 0.0;
                if (s_surf->SurfWinMaxAirflow(ISurf) == 0.0) {
                    continue;
                }
                if (s_surf->Surface(ISurf).ExtBoundCond != ExternalEnvironment) {
                    continue;
                }
                switch (s_surf->SurfWinAirflowControlType(ISurf)) {
                case WindowAirFlowControlType::MaxFlow: {
                    s_surf->SurfWinAirflowThisTS(ISurf) = s_surf->SurfWinMaxAirflow(ISurf);
                } break;
                case WindowAirFlowControlType::AlwaysOff: {
                    s_surf->SurfWinAirflowThisTS(ISurf) = 0.0;
                } break;
                case WindowAirFlowControlType::Schedule: {
                    if (s_surf->SurfWinAirflowHasSchedule(ISurf)) {
                        auto const *sched = s_surf->SurfWinAirflowScheds(ISurf);
                        Real64 ScheduleMult = sched->getCurrentVal(); // Multiplier value from schedule
                        if (ScheduleMult < 0.0 || ScheduleMult > 1.0) {
                            ShowFatalError(
                                state,
                                format("Airflow schedule has a value outside the range 0.0 to 1.0 for window={}", s_surf->Surface(ISurf).Name));
                        }
                        s_surf->SurfWinAirflowThisTS(ISurf) = ScheduleMult * s_surf->SurfWinMaxAirflow(ISurf);
                    }
                } break;
                default:
                    break;
                }
            } // End of surface loop
        } // End of space loop
    } // End of zone loop
}

void SkyDifSolarShading(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   May 1999
    //       MODIFIED       Sep 2000, FCW: add IR view factor calc
    //                      Sep 2002, FCW: correct error in expression for ground IR view factor.
    //                         Affects only non-vertical surfaces that are shadowed. For these surfaces
    //                         error caused underestimate of IR from ground and shadowing surfaces.
    //                      Dec 2002; LKL: Sky Radiance Distribution now only anisotropic
    //                      Nov 2003: FCW: modify to do sky solar shading of shadowing surfaces
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates factors that account for shading of sky diffuse
    // solar radiation by shadowing surfaces such as overhangs and detached
    // shades.
    // Called by PerformSolarCalculations
    // For each exterior heat transfer surface calculates the following
    // ratio (called DifShdgRatioIsoSky in this subroutine):
    //  R1 = (Diffuse solar from sky dome on surface, with shading)/
    //       (Diffuse solar from sky dome on surface, without shading)
    // To calculate the incident diffuse radiation on a surface the sky
    // hemisphere is divided into source elements ("patches"). Each patch
    // is assumed to have the same radiance, i.e. the sky radiance is isotropic.
    // The irradiance from each patch on a surface is calculated. Then these
    // irradiances are summed to get the net irradiance on a surface, which
    // the denominator of R1.
    // To get the numerator of R1 the same summation is done, but for each surface
    // and each patch the Shadow subroutine is called to determine how much
    // radiation from a patch is blocked by shading surfaces.
    // Also calculated is the following ratio (called DifShdgRatioHoriz in this routine):
    //  R2 = (Diffuse solar from sky horizon band on surface, with shading)/
    //       (Diffuse solar from sky horizon band on surface, without shading)
    // For this ratio only a band of sky just above the horizon is considered.
    // R1 and R2 are used in SUBROUTINE AnisoSkyViewFactors, which determines the
    // sky diffuse solar irradiance on each exterior heat transfer surface each
    // time step. In that routine the sky radiance distribution is a superposition
    // of an isotropic distribution,
    // a horizon brightening distribution and a circumsolar brightening distribution,
    // where the proportion of each distribution depends
    // on cloud cover, sun position and other factors. R1 multiplies the irradiance
    // due to the isotropic component and R2 multiplies the irradiance due to the
    // horizon brightening component.
    // Calculates sky and ground IR view factors assuming sky IR is isotropic and
    // shadowing surfaces are opaque to IR.

    // Using/Aliasing

    Real64 Fac1WoShdg;      // Intermediate calculation factor, without shading
    Real64 FracIlluminated; // Fraction of surface area illuminated by a sky patch
    Real64 Fac1WithShdg;    // Intermediate calculation factor, with shading
    Real64 SurfArea;        // Surface area (m2)
    // REAL(r64), ALLOCATABLE, DIMENSION(:) :: WithShdgIsoSky     ! Diffuse solar irradiance from isotropic
    //                                                          ! sky on surface, with shading
    // REAL(r64), ALLOCATABLE, DIMENSION(:) :: WoShdgIsoSky       ! Diffuse solar from isotropic
    //                                                           ! sky on surface, without shading
    // REAL(r64), ALLOCATABLE, DIMENSION(:) :: WithShdgHoriz      ! Diffuse solar irradiance from horizon portion
    // of
    //                                                           ! sky on surface, with shading
    // REAL(r64), ALLOCATABLE, DIMENSION(:) :: WoShdgHoriz        ! Diffuse solar irradiance from horizon portion
    // of
    //                                                           ! sky on surface, without shading
    // INTEGER iHour,iTS

    // Initialize Surfaces Arrays
    auto &s_surf = state.dataSurface;

    bool detailedShading = state.dataSysVars->DetailedSkyDiffuseAlgorithm && s_surf->ShadingTransmittanceVaries &&
                           state.dataHeatBal->SolarDistribution != DataHeatBalance::Shadowing::Minimal;
    state.dataSolarShading->SurfSunlitArea = 0.0;
    state.dataSolarShading->SurfWithShdgIsoSky.dimension(s_surf->TotSurfaces, 0.0);
    state.dataSolarShading->SurfWoShdgIsoSky.dimension(s_surf->TotSurfaces, 0.0);
    state.dataSolarShading->SurfWithShdgHoriz.dimension(s_surf->TotSurfaces, 0.0);
    state.dataSolarShading->SurfWoShdgHoriz.dimension(s_surf->TotSurfaces, 0.0);
    state.dataSolarShading->SurfDifShdgRatioIsoSky.allocate(s_surf->TotSurfaces);
    state.dataSolarShading->SurfDifShdgRatioHoriz.allocate(s_surf->TotSurfaces);
    // initialized as no shading
    state.dataSolarShading->SurfDifShdgRatioIsoSky = 1.0;
    state.dataSolarShading->SurfDifShdgRatioHoriz = 1.0;
    if (detailedShading) {
        state.dataSolarShading->SurfCurDifShdgRatioIsoSky.dimension(s_surf->TotSurfaces, 1.0);
        state.dataSolarShading->SurfDifShdgRatioIsoSkyHRTS.allocate(state.dataGlobal->TimeStepsInHour, 24, s_surf->TotSurfaces);
        state.dataSolarShading->SurfDifShdgRatioIsoSkyHRTS = 1.0;
        state.dataSolarShading->SurfDifShdgRatioHorizHRTS.allocate(state.dataGlobal->TimeStepsInHour, 24, s_surf->TotSurfaces);
        state.dataSolarShading->SurfDifShdgRatioHorizHRTS = 1.0;
    }

    for (int SurfNum : s_surf->AllExtSolarSurfaceList) {
        auto &surf = s_surf->Surface(SurfNum);

        // CurrentModuleObject='Surfaces'
        if (detailedShading) {
            SetupOutputVariable(state,
                                "Debug Surface Solar Shading Model DifShdgRatioIsoSky",
                                Constant::Units::None,
                                state.dataSolarShading->SurfCurDifShdgRatioIsoSky(SurfNum),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
        } else {
            SetupOutputVariable(state,
                                "Debug Surface Solar Shading Model DifShdgRatioIsoSky",
                                Constant::Units::None,
                                state.dataSolarShading->SurfDifShdgRatioIsoSky(SurfNum),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surf.Name);
        }
        SetupOutputVariable(state,
                            "Debug Surface Solar Shading Model DifShdgRatioHoriz",
                            Constant::Units::None,
                            state.dataSolarShading->SurfDifShdgRatioHoriz(SurfNum),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surf.Name);
        SetupOutputVariable(state,
                            "Debug Surface Solar Shading Model WithShdgIsoSky",
                            Constant::Units::None,
                            state.dataSolarShading->SurfWithShdgIsoSky(SurfNum),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surf.Name);
        SetupOutputVariable(state,
                            "Debug Surface Solar Shading Model WoShdgIsoSky",
                            Constant::Units::None,
                            state.dataSolarShading->SurfWoShdgIsoSky(SurfNum),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surf.Name);
    }

    for (int IPhi = 0; IPhi < NPhi; ++IPhi) { // Loop over patch altitude values
        state.dataSolarShading->SUNCOS(3) = state.dataSolarShading->sin_Phi[IPhi];

        for (int ITheta = 0; ITheta < NTheta; ++ITheta) { // Loop over patch azimuth values
            state.dataSolarShading->SUNCOS(1) = state.dataSolarShading->cos_Phi[IPhi] * state.dataSolarShading->cos_Theta[ITheta];
            state.dataSolarShading->SUNCOS(2) = state.dataSolarShading->cos_Phi[IPhi] * state.dataSolarShading->sin_Theta[ITheta];

            for (int SurfNum : s_surf->AllExtSolAndShadingSurfaceList) {
                auto &surf = s_surf->Surface(SurfNum);

                // Cosine of angle of incidence on surface of solar radiation from patch
                state.dataSolarShading->SurfSunCosTheta(SurfNum) = state.dataSolarShading->SUNCOS.x * surf.OutNormVec.x +
                                                                   state.dataSolarShading->SUNCOS.y * surf.OutNormVec.y +
                                                                   state.dataSolarShading->SUNCOS.z * surf.OutNormVec.z;
            }

            SHADOW(state, 24, 0);

            for (int SurfNum : s_surf->AllExtSolAndShadingSurfaceList) {
                auto &surf = s_surf->Surface(SurfNum);

                if (state.dataSolarShading->SurfSunCosTheta(SurfNum) < 0.0) {
                    continue;
                }

                Fac1WoShdg = state.dataSolarShading->cos_Phi[IPhi] * DThetaDPhi * state.dataSolarShading->SurfSunCosTheta(SurfNum);
                SurfArea = surf.NetAreaShadowCalc;
                if (SurfArea > Eps) {
                    FracIlluminated = state.dataSolarShading->SurfSunlitArea(SurfNum) / SurfArea;
                } else {
                    FracIlluminated = state.dataSolarShading->SurfSunlitArea(SurfNum) / (SurfArea + Eps);
                }
                Fac1WithShdg = Fac1WoShdg * FracIlluminated;
                state.dataSolarShading->SurfWithShdgIsoSky(SurfNum) += Fac1WithShdg;
                state.dataSolarShading->SurfWoShdgIsoSky(SurfNum) += Fac1WoShdg;

                // Horizon region
                if (IPhi == 0) {
                    state.dataSolarShading->SurfWithShdgHoriz(SurfNum) += Fac1WithShdg;
                    state.dataSolarShading->SurfWoShdgHoriz(SurfNum) += Fac1WoShdg;
                }
            } // End of surface loop
        } // End of Theta loop
    } // End of Phi loop

    for (int SurfNum : s_surf->AllExtSolAndShadingSurfaceList) {

        if (std::abs(state.dataSolarShading->SurfWoShdgIsoSky(SurfNum)) > Eps) {
            state.dataSolarShading->SurfDifShdgRatioIsoSky(SurfNum) =
                (state.dataSolarShading->SurfWithShdgIsoSky(SurfNum)) / (state.dataSolarShading->SurfWoShdgIsoSky(SurfNum));
        } else {
            state.dataSolarShading->SurfDifShdgRatioIsoSky(SurfNum) =
                (state.dataSolarShading->SurfWithShdgIsoSky(SurfNum)) / (state.dataSolarShading->SurfWoShdgIsoSky(SurfNum) + Eps);
        }
        if (std::abs(state.dataSolarShading->SurfWoShdgHoriz(SurfNum)) > Eps) {
            state.dataSolarShading->SurfDifShdgRatioHoriz(SurfNum) =
                (state.dataSolarShading->SurfWithShdgHoriz(SurfNum)) / (state.dataSolarShading->SurfWoShdgHoriz(SurfNum));
        } else {
            state.dataSolarShading->SurfDifShdgRatioHoriz(SurfNum) =
                (state.dataSolarShading->SurfWithShdgHoriz(SurfNum)) / (state.dataSolarShading->SurfWoShdgHoriz(SurfNum) + Eps);
        }
    }

    // Get IR view factors. An exterior surface can receive IR radiation from
    // sky, ground or shadowing surfaces. Assume shadowing surfaces have same
    // temperature as outside air (and therefore same temperature as ground),
    // so that the view factor to these shadowing surfaces can be included in
    // the ground view factor. Sky IR is assumed to be isotropic and shadowing
    // surfaces are assumed to be opaque to IR so they totally "shade" IR from
    // sky or ground.

    for (int SurfNum = 1; SurfNum <= s_surf->TotSurfaces; ++SurfNum) {
        auto &surface = s_surf->Surface(SurfNum);
        if (!state.dataSysVars->DetailedSkyDiffuseAlgorithm || !s_surf->ShadingTransmittanceVaries ||
            state.dataHeatBal->SolarDistribution == DataHeatBalance::Shadowing::Minimal) {
            surface.ViewFactorSkyIR *= state.dataSolarShading->SurfDifShdgRatioIsoSky(SurfNum);
        } else {
            surface.ViewFactorSkyIR *= state.dataSolarShading->SurfDifShdgRatioIsoSkyHRTS(1, 1, SurfNum);
        }
        surface.ViewFactorGroundIR = 1.0 - surface.ViewFactorSkyIR;

        if (surface.SurfHasSurroundingSurfProperty) {
            surface.ViewFactorGroundIR = 1.0 - surface.ViewFactorSkyIR - surface.ViewFactorSrdSurfs;
        }
    }

    //  DEALLOCATE(WithShdgIsoSky)
    //  DEALLOCATE(WoShdgIsoSky)
    //  DEALLOCATE(WithShdgHoriz)
    //  DEALLOCATE(WoShdgHoriz)

    if (state.dataSysVars->DetailedSkyDiffuseAlgorithm && s_surf->ShadingTransmittanceVaries &&
        state.dataHeatBal->SolarDistribution != DataHeatBalance::Shadowing::Minimal) {
        for (int SurfNum = 1; SurfNum <= s_surf->TotSurfaces; ++SurfNum) {
            state.dataSolarShading->SurfDifShdgRatioIsoSkyHRTS({1, state.dataGlobal->TimeStepsInHour}, {1, 24}, SurfNum) =
                state.dataSolarShading->SurfDifShdgRatioIsoSky(SurfNum);
            state.dataSolarShading->SurfDifShdgRatioHorizHRTS({1, state.dataGlobal->TimeStepsInHour}, {1, 24}, SurfNum) =
                state.dataSolarShading->SurfDifShdgRatioHoriz(SurfNum);
        }
    }
}

void CalcWindowProfileAngles(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   April 2002
    //       MODIFIED       na
    //       RE-ENGINEERED  na
    // PURPOSE OF THIS SUBROUTINE:
    // Called by CalcPerSolarBeam for wholly or partially sunlit exterior windows
    // Calculates horizontal and vertical beam solar profile angles

    Real64 ElevSun;       // Sun elevation; angle between sun and horizontal
    Real64 ElevWin;       // Window elevation: angle between window outward normal and horizontal
    Real64 AzimWin;       // Window azimuth (radians)
    Real64 AzimSun;       // Sun azimuth (radians)
    Real64 ProfileAngHor; // Solar profile angle (radians) for horizontally oriented window elements
    // such as the top and bottom of a frame.
    // This is the incidence angle in a plane that is normal to the window
    // and parallel to the Y-axis of the window (the axis along
    // which the height of the window is measured).
    Real64 ProfileAngVert; // Solar profile angle (radians) for vertically oriented elements
    // such as the sides of a frame.
    // This is the incidence angle in a plane that is normal to the window
    // and parallel to the X-axis of the window (the axis along
    // which the width of the window is measured).
    Vector3<Real64> WinNorm;                                  // Unit vector normal to window
    Vector3<Real64> WinNormCrossBase;                         // Cross product of WinNorm and vector along window baseline
    Vector3<Real64> SunPrime;                                 // Projection of sun vector onto plane (perpendicular to
    Vector3<Real64> const SolCosVec(state.dataEnvrn->SOLCOS); // Local Vector3 copy for speed (until SOLCOS mig to Vector3)
    //  window plane) determined by WinNorm and vector along
    //  baseline of window
    Real64 ThWin; // Azimuth angle of WinNorm (radians)
    Real64 dot1;
    Real64 dot2;
    Real64 dot3;

    auto &s_surf = state.dataSurface;

    ElevSun = Constant::PiOvr2 - std::acos(SolCosVec.z);
    AzimSun = std::atan2(SolCosVec.x, SolCosVec.y);

    Real64 const cos_ElevSun = std::cos(ElevSun);
    Real64 const sin_ElevSun = std::sin(ElevSun);

    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurfWin = thisSpace.WindowSurfaceFirst;
            int const lastSurfWin = thisSpace.WindowSurfaceLast;
            for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {
                auto &surf = s_surf->Surface(SurfNum);
                if (surf.ExtBoundCond != ExternalEnvironment && surf.ExtBoundCond != OtherSideCondModeledExt) {
                    continue;
                }

                s_surf->SurfWinProfileAngHor(SurfNum) = 0.0;
                s_surf->SurfWinProfileAngVert(SurfNum) = 0.0;
                if (state.dataHeatBal->SurfCosIncAng(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum) <= 0.0) {
                    continue;
                }

                ElevWin = Constant::PiOvr2 - surf.Tilt * Constant::DegToRad;
                AzimWin = surf.Azimuth * Constant::DegToRad;

                ProfileAngHor = std::atan(sin_ElevSun / std::abs(cos_ElevSun * std::cos(AzimWin - AzimSun))) - ElevWin;

                // CR9280 - were having negative profile angles on west sides.  commenting out previous code
                // (original code) for vertical windows
                //  IF(ABS(ElevWin) < 0.1d0) THEN  ! Near-vertical window
                //    ProfileAngVert = ABS(AzimWin-AzimSun)
                //  ELSE
                WinNorm = surf.OutNormVec;
                ThWin = AzimWin - Constant::PiOvr2;
                Real64 const sin_Elevwin(std::sin(ElevWin));
                WinNormCrossBase.x = -(sin_Elevwin * std::cos(ThWin));
                WinNormCrossBase.y = sin_Elevwin * std::sin(ThWin);
                WinNormCrossBase.z = std::cos(ElevWin);
                SunPrime = SolCosVec - WinNormCrossBase * dot(SolCosVec, WinNormCrossBase);
                dot1 = dot(WinNorm, SunPrime);
                dot2 = SunPrime.magnitude();
                dot3 = dot1 / dot2;
                if (dot3 > 1.0) {
                    dot3 = 1.0;
                } else if (dot3 < -1.0) {
                    dot3 = -1.0;
                }
                //    ProfileAngVert =
                //    ABS(ACOS(DOT_PRODUCT(WinNorm,SunPrime)/SQRT(DOT_PRODUCT(SunPrime,SunPrime))))
                ProfileAngVert = std::abs(std::acos(dot3));
                //  END IF
                // Constrain to 0 to pi
                if (ProfileAngVert > Constant::Pi) {
                    ProfileAngVert = Constant::TwoPi - ProfileAngVert;
                }

                s_surf->SurfWinProfileAngHor(SurfNum) = ProfileAngHor / Constant::DegToRad;
                s_surf->SurfWinProfileAngVert(SurfNum) = ProfileAngVert / Constant::DegToRad;
                s_surf->SurfWinTanProfileAngHor(SurfNum) = std::abs(std::tan(ProfileAngHor));
                s_surf->SurfWinTanProfileAngVert(SurfNum) = std::abs(std::tan(ProfileAngVert));
            }
        }
    }
}

void CalcFrameDividerShadow(EnergyPlusData &state,
                            int const SurfNum,  // Surface number
                            int const FrDivNum, // Frame/divider number
                            int const HourNum   // Hour number
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   June 2000
    //       MODIFIED       Aug 2000, FW: add effective shadowing by inside
    //                      projections
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Called by CalcPerSolarBeam for wholly or partially sunlit exterior windows
    // with a frame and/or divider. Using beam solar profile angles,
    // calculates fraction of glass shaded by exterior frame and divider projections,
    // The frame and divider profiles are assumed to be rectangular.
    // A similar shadowing approach is used to calculate the fraction of glass area
    // that produces beam solar illumination on interior frame and divider projections.
    // This fraction is used in CalcWinFrameAndDividerTemps to determine the
    // beam solar absorbed by inside projections. Beam solar reflected by inside projections
    // is assumed to stay in the zone (as beam solar) although in actuality roughly
    // half of this is reflected back onto the glass and the half that is reflected
    // into the zone is diffuse.
    // For multipane glazing the effect of solar absorbed by the exposed portion of
    // frame or divider between the panes is not calculated. Beam solar incident on
    // these portions is assumed to be transmitted into the zone unchanged.
    // The shadowing of diffuse solar radiation by projections is not considered.

    Real64 ElevSun;       // Sun elevation; angle between sun and horizontal
    Real64 ElevWin;       // Window elevation: angle between window outward normal and horizontal
    Real64 AzimWin;       // Window azimuth (radians)
    Real64 AzimSun;       // Sun azimuth (radians)
    Real64 ProfileAngHor; // Solar profile angle (radians) for horizontally oriented projections
    // such as the top and bottom of a frame or horizontal dividers.
    // This is the incidence angle in a plane that is normal to the window
    // and parallel to the Y-axis of the window (the axis along
    // which the height of the window is measured).
    Real64 ProfileAngVert; // Solar profile angle (radians) for vertically oriented projections
    // such as the top and bottom of a frame or horizontal dividers.
    // This is the incidence angle in a plane that is normal to the window
    // and parallel to the X-axis of the window (the axis along
    // which the width of the window is measured).
    Real64 TanProfileAngHor;  // Tangent of ProfileAngHor
    Real64 TanProfileAngVert; // Tangent of ProfileAngVert
    Real64 FrWidth;           // Frame width (m)
    Real64 DivWidth;          // Divider width (m)
    Real64 FrProjOut;         // Outside frame projection (m)
    Real64 DivProjOut;        // Outside divider projection (m)
    Real64 FrProjIn;          // Inside frame projection (m)
    Real64 DivProjIn;         // Inside divider projection (m)
    int NHorDiv;              // Number of horizontal dividers
    int NVertDiv;             // Number of vertical dividers
    Real64 GlArea;            // Glazed area (m2)
    Real64 Arealite;          // Area of a single lite of glass (m2); glazed area, GlArea,
    // if there is no divider (in which case there is only one lite).
    Real64 ArealiteCol; // Area of a vertical column of lites (m2)
    Real64 ArealiteRow; // Area of a horizontal row of lites (m2)
    Real64 AshVDout;    // Shaded area from all vertical divider outside projections (m2)
    Real64 AshVDin;     // Shaded area from all vertical divider inside projections (m2)
    Real64 AshHDout;    // Shaded area from all horizontal divider outside projections (m2)
    Real64 AshHDin;     // Shaded area from all horizontal divider inside projections (m2)
    Real64 AshVFout;    // Shaded area from outside projection of vertical sides of frame (m2)
    Real64 AshVFin;     // Shaded area from inside projection of vertical sides of frame (m2)
    Real64 AshHFout;    // Shaded area from outside projection of horizontal sides
    //   (top) of frame (m2)
    Real64 AshHFin; // Shaded area from inside projection of horizontal sides
    //   (top) of frame (m2)
    Real64 AshDDover;   // Divider/divider shadow overlap area (m2)
    Real64 AshFFover;   // Frame/frame shadow overlap area (m2)
    Real64 AshFVDover;  // Frame/vertical divider overlap area (m2)
    Real64 AshFHDover;  // Frame/horizontal divider overlap area (m2)
    Real64 AshFDtotOut; // Total outside projection shadow area (m2)
    Real64 AshFDtotIn;  // Total inside projection shadow area (m2)
    Real64 FracShFDOut; // Fraction of glazing shadowed by frame and divider
    //  outside projections
    Real64 FracShFDin; // Fraction of glazing that illuminates frame and divider
    //  inside projections with beam radiation

    Vector3<Real64> WinNorm(3);  // Window outward normal unit vector // Why the (3)?
    Real64 ThWin;                // Azimuth angle of WinNorm
    Vector3<Real64> SunPrime(3); // Projection of sun vector onto plane (perpendicular to // Why the (3)?
    //  window plane) determined by WinNorm and vector along
    //  baseline of window
    Vector3<Real64> WinNormCrossBase(3); // Cross product of WinNorm and vector along window baseline // Why the (3)?

    auto &s_surf = state.dataSurface;

    if (s_surf->FrameDivider(FrDivNum).FrameProjectionOut == 0.0 && s_surf->FrameDivider(FrDivNum).FrameProjectionIn == 0.0 &&
        s_surf->FrameDivider(FrDivNum).DividerProjectionOut == 0.0 && s_surf->FrameDivider(FrDivNum).DividerProjectionIn == 0.0) {
        return;
    }

    FrProjOut = s_surf->FrameDivider(FrDivNum).FrameProjectionOut;
    FrProjIn = s_surf->FrameDivider(FrDivNum).FrameProjectionIn;
    DivProjOut = s_surf->FrameDivider(FrDivNum).DividerProjectionOut;
    DivProjIn = s_surf->FrameDivider(FrDivNum).DividerProjectionIn;

    auto &surf = s_surf->Surface(SurfNum);
    GlArea = surf.Area;
    ElevWin = Constant::PiOvr2 - surf.Tilt * Constant::DegToRad;
    ElevSun = Constant::PiOvr2 - std::acos(state.dataSolarShading->SUNCOS(3));
    AzimWin = surf.Azimuth * Constant::DegToRad;
    AzimSun = std::atan2(state.dataSolarShading->SUNCOS(1), state.dataSolarShading->SUNCOS(2));

    ProfileAngHor = std::atan(std::sin(ElevSun) / std::abs(std::cos(ElevSun) * std::cos(AzimWin - AzimSun))) - ElevWin;
    if (std::abs(ElevWin) < 0.1) { // Near-vertical window
        ProfileAngVert = std::abs(AzimWin - AzimSun);
    } else {
        WinNorm = surf.OutNormVec;
        ThWin = AzimWin - Constant::PiOvr2;
        WinNormCrossBase(1) = -std::sin(ElevWin) * std::cos(ThWin);
        WinNormCrossBase(2) = std::sin(ElevWin) * std::sin(ThWin);
        WinNormCrossBase(3) = std::cos(ElevWin);
        SunPrime = state.dataSolarShading->SUNCOS - WinNormCrossBase * dot(state.dataSolarShading->SUNCOS, WinNormCrossBase);
        ProfileAngVert = std::abs(std::acos(dot(WinNorm, SunPrime) / magnitude(SunPrime)));
    }
    // Constrain to 0 to pi
    if (ProfileAngVert > Constant::Pi) {
        ProfileAngVert = 2 * Constant::Pi - ProfileAngVert;
    }
    TanProfileAngHor = std::abs(std::tan(ProfileAngHor));
    TanProfileAngVert = std::abs(std::tan(ProfileAngVert));

    NHorDiv = s_surf->FrameDivider(FrDivNum).HorDividers;
    NVertDiv = s_surf->FrameDivider(FrDivNum).VertDividers;
    FrWidth = s_surf->FrameDivider(FrDivNum).FrameWidth;
    DivWidth = s_surf->FrameDivider(FrDivNum).DividerWidth;

    Arealite = (surf.Height / (NHorDiv + 1.0) - DivWidth / 2.0) * (surf.Width / (NVertDiv + 1.0) - DivWidth / 2.0);
    if (DivProjOut > 0.0 || DivProjIn > 0.0) {
        ArealiteCol = (NHorDiv + 1) * Arealite;
        ArealiteRow = (NVertDiv + 1) * Arealite;
    } else {
        ArealiteCol = GlArea;
        ArealiteRow = GlArea;
    }
    AshVDout = 0.0;
    AshVDin = 0.0;
    AshHDout = 0.0;
    AshHDin = 0.0;
    AshVFout = 0.0;
    AshVFin = 0.0;
    AshHFout = 0.0;
    AshHFin = 0.0;
    AshDDover = 0.0;
    AshFFover = 0.0;
    AshFVDover = 0.0;
    AshFHDover = 0.0;

    if (DivProjOut > 0.0 || DivProjIn > 0.0) {

        // Shaded area from all vertical dividers
        AshVDout = NVertDiv * min((surf.Height - NHorDiv * DivWidth) * DivProjOut * TanProfileAngVert, ArealiteCol);
        AshVDin = NVertDiv * min((surf.Height - NHorDiv * DivWidth) * DivProjIn * TanProfileAngVert, ArealiteCol);

        // Shaded area from all horizontal dividers
        AshHDout = NHorDiv * min((surf.Width - NVertDiv * DivWidth) * DivProjOut * TanProfileAngHor, ArealiteRow);
        AshHDin = NHorDiv * min((surf.Width - NVertDiv * DivWidth) * DivProjIn * TanProfileAngHor, ArealiteRow);

        // Horizontal divider/vertical divider shadow overlap
        AshDDover = min(DivProjOut * TanProfileAngHor * DivProjOut * TanProfileAngVert, Arealite) * NHorDiv * NVertDiv;
    }

    if (FrProjOut > 0.0 || FrProjIn > 0.0) {

        // Shaded area from sides of frame; to avoid complications from possible overlaps between
        // shadow from side of frame and shadow from vertical divider the shaded area from side of
        // frame is restricted to the area of one column of lites.
        AshVFout = min((surf.Height - NHorDiv * DivWidth) * FrProjOut * TanProfileAngVert, ArealiteCol);
        AshVFin = min((surf.Height - NHorDiv * DivWidth) * FrProjIn * TanProfileAngVert, ArealiteCol);

        // Shaded area from top or bottom of frame; to avoid complications from possible overlaps
        // between shadow from top or bottom of frame and shadow from horizontal divider, the shaded
        // area from the top or bottom of frame is restricted to the area of one row of lites.
        AshHFout = min((surf.Width - NVertDiv * DivWidth) * FrProjOut * TanProfileAngHor, ArealiteRow);
        AshHFin = min((surf.Width - NVertDiv * DivWidth) * FrProjIn * TanProfileAngHor, ArealiteRow);

        // Top/bottom of frame/side of frame shadow overlap
        AshFFover = min(FrProjOut * TanProfileAngHor * FrProjOut * TanProfileAngVert, Arealite);
        if (DivProjOut > 0.0) {
            // Frame/vertical divider shadow overlap
            AshFVDover = min(FrProjOut * DivProjOut * TanProfileAngHor * TanProfileAngVert, Arealite) * NVertDiv;
            // Frame/horizontal divider shadow overlap
            AshFHDover = min(FrProjOut * DivProjOut * TanProfileAngHor * TanProfileAngVert, Arealite) * NHorDiv;
        }
    }

    AshFDtotOut = AshVDout + AshHDout + AshVFout + AshHFout - (AshDDover + AshFFover + AshFVDover + AshFHDover);
    AshFDtotIn =
        (AshVDin + AshHDin) * s_surf->FrameDivider(FrDivNum).DividerSolAbsorp + (AshVFin + AshHFin) * s_surf->FrameDivider(FrDivNum).FrameSolAbsorp;

    // Divide by the glazed area of the window
    FracShFDOut = AshFDtotOut / GlArea;
    FracShFDin = AshFDtotIn / GlArea;
    s_surf->SurfaceWindow(SurfNum).OutProjSLFracMult[HourNum] = 1.0 - FracShFDOut;
    s_surf->SurfaceWindow(SurfNum).InOutProjSLFracMult[HourNum] = 1.0 - (FracShFDin + FracShFDOut);
}

void CalcBeamSolarOnWinRevealSurface(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         F. Winkelmann
    //       DATE WRITTEN   April 2002
    //       MODIFIED:na
    //       RE-ENGINEERED:na

    // PURPOSE OF THIS SUBROUTINE
    // Called by InitHeatGains when the sun is up.
    // Calculates beam solar radiation absorbed and reflected by top, bottom,
    // right and left sides of outside and inside window reveal surfaces.
    // In doing this calculation, the shadowing on a reveal surface by other reveal surfaces
    // is determined using the orientation of the reveal surfaces and the sun position.
    // It is assumed that:
    // (1) The window is an exterior window and is rectangular.
    // (2) The reveal surfaces are perpendicular to the window plane.
    // (3) If an exterior shade or blind is in place, there is no beam solar on
    //     on exterior or interior reveal surfaces.
    // (3) If an interior shade or blind is in place, there is no beam solar on
    //     interior reveal surfaces.
    // (4) The effect of window divider, if present, is ignored, including shadowing
    //     of divider on inside reveal surfaces.

    // In the variable names, the "subscript" 1 = outside reveal, 2 = inside reveal
    // The outside reveal surfaces (top, bottom, left, right) are assumed to have the same depth
    // (given by Surface%Reveal and determined from vertices of window and vertices of parent
    // wall) and the same solar absorptance. The inside reveal surfaces are divided into
    // two categories: (1) the bottom reveal surface, called here the "inside sill;" and
    // the other reveal surfaces (left, right and top). The left, right and top inside reveal
    // surfaces are assumed to have the same depth and solar absorptance.
    // The depth of the outside reveal is measured from the outside surface of the glazing;
    // The depth of the inside sill and the other reveal surfaces is measured from the inside
    // surface of the glazing. The inside sill is
    // allowed to have depth and solar absorptance values that are different from the corresponding
    // values for the other inside reveal surfaces. The inside sill depth is required to be
    // greater than or equal to the depth of the other inside reveal surfaces. If the inside sill
    // depth is greater than zero the depth of the other inside reveal surfaces is required to
    // to be greater than zero.
    // The reflection of beam solar radiation from all reveal surfaces is assumed to be isotropic
    // diffuse; there is no specular component. Half of the beam solar reflected from outside
    // reveal surfaces is assumed to go towards the window; the other half is assumed to go back
    // to the exterior environment (i.e., reflection of this outward-going component from
    // other outside reveal surfaces is not considered). The half that goes towards the window
    // is added to the other radiation incident on the window.
    // Correspondingly, half of the beam solar reflected from inside reveal surfaces is assumed
    // to go towards the window, with the other half going into the zone (this half, and the portion
    // going towards the window that is reflected) is added in CalcInteriorSolarDistribution
    // to the variable BTOTzone, which is the total beam solar entering the zone as beam or diffuse.
    // The portion going towards the window that is not reflected is absorbed in the glazing or
    // transmitted back out into the exterior environment.
    // The beam solar that is absorbed by outside reveal surfaces is added to the solar absorbed
    // by the outside surface of the window's parent wall; similarly, the beam solar absorbed
    // by the inside reveal surfaces is added to the solar absorbed by the inside surface of the
    // parent wall (and is subtracted from BTOTzone).
    // The net effect of beam solar reflected from outside reveal surfaces is to INCREASE the
    // the heat gain to the zone, whereas the effect of beam solar reflected from interior reveal
    // surfaces is to DECREASE the heat gain to the zone since part of this reflected solar is
    // transmitted back out the window.
    // If the window has a frame, the absorption of reflected beam solar by the inside and outside
    // surfaces of the frame is considered. The shadowing of the frame onto interior reveal
    // surfaces is also considered.

    // The total glazing thickness is taken to be the sum of the thickness of the glass layers
    // and between-glass gas layers. If the window has an exterior, movable, storm window glass layer
    // the presence of this layer and its adjacent air gap is considered in calculating the glazing
    // properties (solar transmittance, etc.). But the storm window glass is assumed to be close
    // enough to the rest of the glazing that its effect on total glazing thickness and outside
    // reveal depth can be ignored.

    // METHODOLOGY EMPLOYED
    // na

    // REFERENCES
    // na

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS

    int ConstrNum;        // Construction number
    int ConstrNumSh;      // Shaded construction number
    Real64 CosBetaBottom; // Cosine of beam solar angle of incidence on bottom reveal
    Real64 CosBetaLeft;   // Cosine of beam solar angle of incidence on left reveal
    Real64 CosBeta;       // ABS of CosBetaBottom or CosBetaLeft
    Real64 d1;            // Depth of outside reveal + half of glazing thickness (m)
    Real64 d2;            // Depth of inside sill or of inside reveal plus half of glazing thickness (m)
    Real64 d2prime;       // Depth of shadow cast on a reveal surface by opposite reveal (m)
    Real64 d2prime2;      // Depth of shadow cast by frame onto inside reveal (m)
    Real64 d12;           // d12 = d1 + d2 - d2prime (m)
    Real64 TanAlpha;      // Tangent of horizontal or vertical profile angle
    Real64 TanGamma;      // Tangent of vertical or horizontal profile angle
    Real64 H;             // Window height, width (m)
    Real64 W;
    Real64 L;                    // Window height or width (m)
    Real64 A1sh;                 // Shadowed area of outside horizontal or vertical reveal (m2)
    Real64 A2sh;                 // Shadowed area of inside horizontal or vertical reveal (m2)
    Real64 A1ill;                // Illuminated area of outside horizontal or vertical reveal (m2)
    Real64 A2ill;                // Illuminated area of inside horizontal or vertical reveal (m2)
    Real64 SolTransGlass;        // Beam solar transmittance of glazing
    Real64 SolTransGlassSh;      // For switchable glazing, beam solar trans in switched state
    Real64 DiffReflGlass;        // Diffuse back reflectance of glazing
    Real64 DiffReflGlassSh;      // For switchable glazing, diffuse back refl in switched state
    int HorVertReveal;           // Index: 1 = horizontal reveal, 2 = vertical reveal
    Real64 OutsReveal;           // Depth of outside reveal (from outside glazing plane to outside wall plane) (m)
    Real64 InsReveal;            // Depth of inside reveal (from inside glazing plane to inside wall plane (m)
    Real64 InsSillDepth;         // Depth of inside sill, measured from innermost face of glazing (m)
    Real64 GlazingThickness;     // Thickness of glazing, measured from innermost face to outermost face (m)
    Real64 InsideRevealSolAbs;   // Solar absorptance of inside reveal or inside sill
    Real64 BmSolRefldOutsReveal; // Multiplied by beam solar gives beam solar reflected by horiz or vertical
    //  outside reveal surface (m2)
    Real64 BmSolRefldInsReveal; // Multiplied by beam solar gives beam solar reflected by horiz or vertical
    //  inside reveal surface (m2)
    WinShadingType ShadeFlag; // Shading flag
    int FrameDivNum;          // Frame/Divider number
    Real64 FrameWidth;        // Frame width (m)
    Real64 P1;                // Frame outside/inside projection plus half of glazing thickness (m)
    Real64 P2;
    Real64 f1; // f1=d1-P1, f2=d2-P2 (m)
    Real64 f2;
    Real64 L1; // Average distance of outside/inside illuminated area to frame;
    Real64 L2;
    // used in calculating view factor to frame (m)
    Real64 FracToGlassOuts;   // View factor from outside horizontal or vertical reveal to glass
    Real64 FracToGlassIns;    // View factor from inside horizontal or vertical reveal to glass
    Real64 TanProfileAngVert; // Tangent of vertical profile angle (the profile angle appropriate for
    // vertical reveal surfaces.
    Real64 TanProfileAngHor; // Tangent of horizontal profile angle (the profile angle appropriate for
    // horizontal reveal surfaces.

    Real64 tmp_SunlitFracWithoutReveal; // Temporary variable

    auto &s_surf = state.dataSurface;

    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurfWin = thisSpace.WindowSurfaceFirst;
            int const lastSurfWin = thisSpace.WindowSurfaceLast;
            for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {
                // Added TH for initialization. CR 7596 inside reveal causing high cooling loads
                // for outside reveals
                s_surf->SurfWinBmSolAbsdOutsReveal(SurfNum) = 0.0;
                s_surf->SurfWinBmSolRefldOutsRevealReport(SurfNum) = 0.0;
                s_surf->SurfWinBmSolRefldOutsRevealRepEnergy(SurfNum) = 0.0;
                s_surf->SurfWinOutsRevealDiffOntoGlazing(SurfNum) = 0.0;
                s_surf->SurfWinOutsRevealDiffOntoFrame(SurfNum) = 0.0;
                // for inside reveals
                s_surf->SurfWinBmSolAbsdInsReveal(SurfNum) = 0.0;
                s_surf->SurfWinBmSolAbsdInsRevealReport(SurfNum) = 0.0;
                s_surf->SurfWinBmSolRefldInsReveal(SurfNum) = 0.0;
                s_surf->SurfWinBmSolRefldInsRevealReport(SurfNum) = 0.0;
                s_surf->SurfWinBmSolRefldInsRevealRepEnergy(SurfNum) = 0.0;
                s_surf->SurfWinInsRevealDiffOntoGlazing(SurfNum) = 0.0;
                s_surf->SurfWinInsRevealDiffOntoGlazingReport(SurfNum) = 0.0;
                s_surf->SurfWinInsRevealDiffOntoFrame(SurfNum) = 0.0;
                s_surf->SurfWinInsRevealDiffOntoFrameReport(SurfNum) = 0.0;
                s_surf->SurfWinInsRevealDiffIntoZone(SurfNum) = 0.0;
                s_surf->SurfWinInsRevealDiffIntoZoneReport(SurfNum) = 0.0;

                auto &surf = s_surf->Surface(SurfNum);
                if ((surf.ExtBoundCond != ExternalEnvironment && surf.ExtBoundCond != OtherSideCondModeledExt)) {
                    continue;
                }
                if (surf.Reveal == 0.0 && s_surf->SurfWinInsideReveal(SurfNum) == 0.0 && s_surf->SurfWinInsideSillDepth(SurfNum) == 0.0) {
                    continue;
                }
                if (surf.Sides != 4) {
                    continue;
                }
                if (s_surf->SurfWinInsideSillDepth(SurfNum) < s_surf->SurfWinInsideReveal(SurfNum)) {
                    continue;
                }

                ShadeFlag = s_surf->SurfWinShadingFlag(SurfNum);
                if ((ShadeFlag == WinShadingType::ExtShade) || (ShadeFlag == WinShadingType::ExtBlind)) {
                    continue;
                }

                if (state.dataHeatBal->SurfCosIncAng(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum) <= 0.0) {
                    continue;
                }

                tmp_SunlitFracWithoutReveal =
                    state.dataHeatBal->SurfSunlitFracWithoutReveal(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum);

                // Calculate cosine of angle of incidence of beam solar on reveal surfaces,
                // assumed to be perpendicular to window plane

                CosBetaBottom = -state.dataEnvrn->SOLCOS(1) * surf.SinAzim * surf.CosTilt - state.dataEnvrn->SOLCOS(2) * surf.CosAzim * surf.CosTilt +
                                state.dataEnvrn->SOLCOS(3) * surf.SinTilt;

                CosBetaLeft = -state.dataEnvrn->SOLCOS(1) * surf.CosAzim - state.dataEnvrn->SOLCOS(2) * surf.SinAzim;

                // Note: CosBetaTop = -CosBetaBottom, CosBetaRight = -CosBetaLeft

                OutsReveal = surf.Reveal;
                InsReveal = s_surf->SurfWinInsideReveal(SurfNum);
                InsideRevealSolAbs = 0.0;
                GlazingThickness = s_surf->SurfWinTotGlazingThickness(SurfNum);
                H = surf.Height;
                W = surf.Width;
                d1 = OutsReveal + 0.5 * GlazingThickness;
                ConstrNum = s_surf->SurfActiveConstruction(SurfNum);
                ConstrNumSh = s_surf->SurfWinActiveShadedConstruction(SurfNum);
                auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);

                SolTransGlass = Window::POLYF(state.dataHeatBal->SurfCosIncAng(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum),
                                              thisConstruct.TransSolBeamCoef);
                TanProfileAngVert = s_surf->SurfWinTanProfileAngVert(SurfNum);
                TanProfileAngHor = s_surf->SurfWinTanProfileAngHor(SurfNum);
                FrameDivNum = surf.FrameDivider;
                FrameWidth = 0.0;
                if (FrameDivNum != 0) {
                    FrameWidth = s_surf->FrameDivider(FrameDivNum).FrameWidth;
                    if (FrameWidth > 0.0) {
                        P1 = s_surf->FrameDivider(FrameDivNum).FrameProjectionOut + 0.5 * GlazingThickness;
                        P2 = s_surf->FrameDivider(FrameDivNum).FrameProjectionIn + 0.5 * GlazingThickness;
                        if (OutsReveal + 0.5 * GlazingThickness <= P1) {
                            d1 = P1 + 0.001;
                        }
                    }
                }
                // Loop over vertical and horizontal reveal surfaces
                for (HorVertReveal = 1; HorVertReveal <= 2; ++HorVertReveal) {

                    FracToGlassOuts = 0.5;
                    FracToGlassIns = 0.5;
                    BmSolRefldOutsReveal = 0.0;
                    BmSolRefldInsReveal = 0.0;
                    A1ill = 0.0;
                    A2ill = 0.0;

                    // Added TH. 5/27/2009
                    A1sh = 0.0;
                    A2sh = 0.0;

                    if (HorVertReveal == 1) { // Vertical reveal
                        TanAlpha = TanProfileAngHor;
                        TanGamma = TanProfileAngVert;
                        CosBeta = std::abs(CosBetaLeft);
                        L = surf.Height;
                        d2 = InsReveal + 0.5 * GlazingThickness;
                        d2prime = d1 + d2 - W / TanGamma;
                        InsideRevealSolAbs = s_surf->SurfWinInsideRevealSolAbs(SurfNum);
                    } else { // Horizontal reveal
                        InsSillDepth = s_surf->SurfWinInsideSillDepth(SurfNum);
                        TanAlpha = TanProfileAngVert;
                        TanGamma = TanProfileAngHor;
                        CosBeta = std::abs(CosBetaBottom);
                        L = surf.Width;
                        if (CosBetaBottom > 0.0) { // Bottom reveal surfaces may be illuminated
                            d2 = InsSillDepth + 0.5 * GlazingThickness;
                            InsideRevealSolAbs = s_surf->SurfWinInsideSillSolAbs(SurfNum);
                        } else { // Top reveal surfaces may be illuminated
                            d2 = InsReveal + 0.5 * GlazingThickness;
                            InsideRevealSolAbs = s_surf->SurfWinInsideRevealSolAbs(SurfNum);
                        }
                        d2prime = d1 + d2 - H / TanGamma;
                    }
                    if (d2prime < 0.0) {
                        d2prime = 0.0; // No shadow from opposing reveal
                    }
                    d12 = d1 + d2 - d2prime;

                    if (FrameWidth <= 0.001) {
                        // Window without frame

                        // Find inside and outside shadowed area of vertical or horizontal reveal surfaces
                        // that can be illuminated by beam solar; shadowing is by other reveal surfaces.

                        if (d2prime <= d2) {
                            if (d12 * TanAlpha <= L) {
                                A1sh = 0.5 * TanAlpha * pow_2(d1);
                                A2sh = d2prime * L + 0.5 * TanAlpha * pow_2(d12) - A1sh;
                            } else { // d12*TanAlpha > L
                                if (d1 * TanAlpha <= L) {
                                    A1sh = 0.5 * TanAlpha * pow_2(d1);
                                    A2sh = d2 * L - 0.5 * TanAlpha * pow_2(L / TanAlpha - d1);
                                } else { // d1*TanAlpha > L
                                    A1sh = d1 * L - (0.5 / TanAlpha) * pow_2(L);
                                    A2sh = d2 * L;
                                }
                            }
                        } else { // d2prime > d2
                            A2sh = d2 * L;
                            if (d2prime < d1 + d2) {
                                if (d12 * TanAlpha <= L) {
                                    A1sh = L * (d2prime - d2) + 0.5 * TanAlpha * pow_2(d12);
                                } else { // d12*TanAlpha > L
                                    A1sh = d1 * L - 0.5 * pow_2(L) / TanAlpha;
                                }
                            } else { // d2prime >= d1+d2
                                A1sh = d1 * L;
                            }
                        }

                        // Added TH. 5/27/2009
                        if (A1sh < 0.0) {
                            A1sh = 0.0;
                        }
                        if (A2sh < 0.0) {
                            A2sh = 0.0;
                        }

                        if (OutsReveal >= 0.001) {
                            A1ill = d1 * L - A1sh; // A1ill = 0.0 if OutsReveal < 0.001
                        }
                        if (InsReveal >= 0.001) {
                            A2ill = d2 * L - A2sh; // A2ill = 0.0 if InsReveal < 0.001
                        }

                    } else { // Window with frame; take into account shadowing
                        // of inside reveal surfaces by frame
                        f1 = d1 - P1;
                        f2 = d2 - P2;
                        d2prime2 = FrameWidth / TanGamma;
                        if (HorVertReveal == 1) { // Vertical reveal
                            if (InsReveal + 0.5 * GlazingThickness <= P2) {
                                d2 = P2 + 0.001;
                            }
                        } else {                       // Horizontal
                            if (CosBetaBottom > 0.0) { // Bottom reveal surfaces may be illuminated
                                if (InsSillDepth + 0.5 * GlazingThickness <= P2) {
                                    d2 = P2 + 0.001;
                                }
                            } else { // Top reveal surfaces may be illuminated
                                if (InsReveal + 0.5 * GlazingThickness <= P2) {
                                    d2 = P2 + 0.001;
                                }
                            }
                        }

                        if (d2prime <= f2) { // Shadow from opposing reveal does not go beyond inside surface of frame

                            if (d12 * TanAlpha <= L) {
                                A1sh = 0.5 * TanAlpha * pow_2(f1);
                                L1 = f1 * (f1 * TanAlpha / (6.0 * L) + 0.5);
                                if (d2 - (d2prime + d2prime2 + P2) >= 0.0) {
                                    A2sh = (d2prime + d2prime2) * L + 0.5 * TanAlpha * (pow_2(d1 + d2 - d2prime) - pow_2(d1 + P2 + d2prime2));
                                    L2 = d2prime2 + 0.5 * (d2 - (d2prime + d2prime2 + P2));
                                } else { // d2-(d2prime+d2prime2+P2) < 0.  ! Inside reveal is fully shadowed by
                                         // frame and/or opposing reveal
                                    A2sh = f2 * L;
                                    L2 = f2;
                                }
                            } else { // d12*TanAlpha >= L
                                if ((d1 + P2) * TanAlpha <= L) {
                                    A1sh = 0.5 * TanAlpha * pow_2(f1);
                                    L1 = f1 * ((f1 * TanAlpha) / (6.0 * L) + 0.5);
                                    if ((d1 + P2 + d2prime2) * TanAlpha >= L) {
                                        A2sh = f2 * L;
                                        L2 = f2;
                                    } else { // (d1+P2+d2prime2)*TanAlpha < L
                                        A2sh = f2 * L - 0.5 * pow_2(L - (d1 + P2) * TanAlpha) / TanAlpha +
                                               d2prime2 * (L - (d1 + P2 + d2prime2 / 2.0) * TanAlpha);
                                        L2 = d2prime2 + (L / TanAlpha - (d1 + P2 + d2prime2)) / 3.0;
                                    }
                                } else { // (d1+P2)*TanAlpha > L
                                    L2 = f2;
                                    A2sh = f2 * L;
                                    if (f1 * TanAlpha <= L) {
                                        A1sh = 0.5 * TanAlpha * pow_2(f1);
                                        L1 = f1 * ((f1 * TanAlpha) / (6.0 * L) + 0.5);
                                    } else { // f1*TanAlpha > L
                                        A1sh = f1 * L - 0.5 * pow_2(L) / TanAlpha;
                                        L1 = f1 - (L / TanAlpha) / 3.0;
                                    }
                                }
                            }

                        } else { // d2prime > f2   ! Shadow from opposing reveal goes beyond inside of frame

                            A2sh = f2 * L;
                            L2 = f2;
                            if (d2prime >= d1 + d2) {
                                A1sh = 0.0;
                                L1 = f1;
                            } else { // d2prime < d1+d2
                                if (d2prime <= d2 + P1) {
                                    if (f1 * TanAlpha <= L) {
                                        A1sh = 0.5 * TanAlpha * pow_2(f1);
                                        L1 = f1 * ((f1 * TanAlpha) / (6.0 * L) + 0.5);
                                    } else { // f1*TanAlpha > L
                                        A1sh = f1 * L - 0.5 * pow_2(L) / TanAlpha;
                                        L1 = f1 - (L / TanAlpha) / 3.0;
                                    }
                                } else { // d2prime > d2+P1
                                    if (d12 * TanAlpha <= L) {
                                        A1sh = L * (d2prime - (d2 + P1)) + 0.5 * TanAlpha * pow_2(d12);
                                        L1 = (L * (f1 - d12 / 2.0) - d12 * TanAlpha * (f1 / 2 - d12 / 3.0)) / (L - d12 * TanAlpha / 2.0);
                                    } else { // d12*TanAlpha > L
                                        A1sh = f1 * L - 0.5 * pow_2(L) / TanAlpha;
                                        L1 = f1 - (L / TanAlpha) / 3.0;
                                    }
                                }
                            }
                        }

                        // Added TH. 5/27/2009
                        if (A1sh < 0.0) {
                            A1sh = 0.0;
                        }
                        if (A2sh < 0.0) {
                            A2sh = 0.0;
                        }

                        if (OutsReveal >= P1 + 0.5 * GlazingThickness + 0.001) {
                            A1ill = L * f1 - A1sh;
                        }
                        if (InsReveal >= P2 + 0.5 * GlazingThickness + 0.001) {
                            A2ill = L * f2 - A2sh;
                        }
                        if (L1 == 0.0) {
                            FracToGlassOuts = 0.0;
                        } else {
                            FracToGlassOuts = 0.5 * (1.0 - std::atan(FrameWidth / L1) / Constant::PiOvr2);
                        }
                        if (L2 == 0.0) {
                            FracToGlassIns = 0.0;
                        } else {
                            FracToGlassIns = 0.5 * (1.0 - std::atan(FrameWidth / L2) / Constant::PiOvr2);
                        }
                    } // End of check if window has frame

                    // Added TH. 5/27/2009
                    if (A1ill < 0.0) {
                        A1ill = 0.0;
                    }
                    if (A2ill < 0.0) {
                        A2ill = 0.0;
                    }

                    // Quantities related to outside reveal
                    if (A1ill > Constant::OneMillionth) {

                        s_surf->SurfWinBmSolAbsdOutsReveal(SurfNum) +=
                            A1ill * s_surf->SurfWinOutsideRevealSolAbs(SurfNum) * CosBeta * tmp_SunlitFracWithoutReveal;

                        BmSolRefldOutsReveal = A1ill * (1.0 - s_surf->SurfWinOutsideRevealSolAbs(SurfNum)) * CosBeta * tmp_SunlitFracWithoutReveal;

                        s_surf->SurfWinBmSolRefldOutsRevealReport(SurfNum) += state.dataEnvrn->BeamSolarRad * BmSolRefldOutsReveal;
                        s_surf->SurfWinBmSolRefldOutsRevealRepEnergy(SurfNum) =
                            s_surf->SurfWinBmSolRefldOutsRevealReport(SurfNum) * state.dataGlobal->TimeStepZoneSec;

                        // Reflected solar from outside horizontal and vertical reveal incident on glazing
                        s_surf->SurfWinOutsRevealDiffOntoGlazing(SurfNum) += FracToGlassOuts * BmSolRefldOutsReveal / surf.Area;

                        if (FrameWidth > 0.0) {
                            // Reflected solar from outside horizontal and vertical reveal incident on frame
                            s_surf->SurfWinOutsRevealDiffOntoFrame(SurfNum) +=
                                (0.5 - FracToGlassOuts) * BmSolRefldOutsReveal / s_surf->SurfWinFrameArea(SurfNum);
                        }

                    } // End of check if A1ill > 0.0 (actually 10^-6)

                    // Quantities related to inside reveal; inside reveal reflection/absorption is assumed
                    // to occur only if an interior shade or blind is not in place.

                    if (NOT_SHADED(ShadeFlag) || ShadeFlag == WinShadingType::SwitchableGlazing) {

                        if (A2ill > Constant::OneMillionth) {

                            DiffReflGlass = state.dataConstruction->Construct(ConstrNum).ReflectSolDiffBack;
                            if (ShadeFlag == WinShadingType::SwitchableGlazing) {
                                auto const &thisConstructSh = state.dataConstruction->Construct(ConstrNumSh);
                                SolTransGlassSh =
                                    Window::POLYF(state.dataHeatBal->SurfCosIncAng(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum),
                                                  thisConstructSh.TransSolBeamCoef);
                                SolTransGlass = Window::InterpSw(s_surf->SurfWinSwitchingFactor(SurfNum), SolTransGlass, SolTransGlassSh);
                                DiffReflGlassSh = thisConstructSh.ReflectSolDiffBack;
                                DiffReflGlass = Window::InterpSw(s_surf->SurfWinSwitchingFactor(SurfNum), DiffReflGlass, DiffReflGlassSh);
                            }

                            // Calc beam solar absorbed (m2)
                            s_surf->SurfWinBmSolAbsdInsReveal(SurfNum) +=
                                A2ill * SolTransGlass * InsideRevealSolAbs * CosBeta * tmp_SunlitFracWithoutReveal;

                            // Added TH 5/26/2009 for reporting purpose - Beam solar absorbed by the inside reveal
                            // (W)
                            s_surf->SurfWinBmSolAbsdInsRevealReport(SurfNum) +=
                                state.dataEnvrn->BeamSolarRad * A2ill * SolTransGlass * InsideRevealSolAbs * CosBeta * tmp_SunlitFracWithoutReveal;

                            // in m2 = Area * solar transmitted fraction * inside reveal reflection fraction
                            BmSolRefldInsReveal = A2ill * SolTransGlass * (1.0 - InsideRevealSolAbs) * CosBeta * tmp_SunlitFracWithoutReveal;

                            s_surf->SurfWinBmSolRefldInsReveal(SurfNum) += BmSolRefldInsReveal;

                            s_surf->SurfWinBmSolRefldInsRevealReport(SurfNum) +=
                                state.dataEnvrn->BeamSolarRad * BmSolRefldInsReveal; // W, BeamSolarRad in W/m2
                            s_surf->SurfWinBmSolRefldInsRevealRepEnergy(SurfNum) =
                                s_surf->SurfWinBmSolRefldInsRevealReport(SurfNum) * state.dataGlobal->TimeStepZoneSec;

                            // Reflected solar from inside horizontal and vertical reveal incident on glazing
                            s_surf->SurfWinInsRevealDiffOntoGlazing(SurfNum) += FracToGlassIns * BmSolRefldInsReveal / surf.Area;

                            // Added TH 5/26/2009 for reporting purpose - diffuse on window glass from inside
                            // reveal (W)
                            s_surf->SurfWinInsRevealDiffOntoGlazingReport(SurfNum) +=
                                state.dataEnvrn->BeamSolarRad * FracToGlassIns * BmSolRefldInsReveal;

                            // Reflected solar from inside horizontal and vertical reveal incident on frame
                            if (FrameWidth > 0.0) {
                                s_surf->SurfWinInsRevealDiffOntoFrame(SurfNum) +=
                                    (0.5 - FracToGlassIns) * BmSolRefldInsReveal / s_surf->SurfWinFrameArea(SurfNum);

                                // Added TH 5/26/2009 for reporting purpose - diffuse on window frame from inside
                                // reveal (W)
                                s_surf->SurfWinInsRevealDiffOntoFrameReport(SurfNum) +=
                                    state.dataEnvrn->BeamSolarRad * (0.5 - FracToGlassIns) * BmSolRefldInsReveal;
                            }

                            // Reflected solar from inside reveal going directly into zone and reflected from
                            // glass. Assumes half of solar reflected from inside reveal goes as diffuse radiation
                            // into the zone and half goes as diffuse radiation towards window.
                            s_surf->SurfWinInsRevealDiffIntoZone(SurfNum) += BmSolRefldInsReveal * (0.5 + DiffReflGlass * FracToGlassIns);

                            // Added TH 5/26/2009 for reporting purpose - diffuse into zone from inside reveal (W)
                            s_surf->SurfWinInsRevealDiffIntoZoneReport(SurfNum) +=
                                state.dataEnvrn->BeamSolarRad * BmSolRefldInsReveal * (0.5 + DiffReflGlass * FracToGlassIns);

                        } // End of check if A2ill > 0.0 (actually 10^-6)

                    } // End of check if interior shade or blind is in place

                } // End of loop over vertical and horizontal reveal

            } // End of surface loop
        }
    }
}

void ReportSurfaceShading(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   April 2000
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine uses the internal variables used in the Shading
    // calculations and prepares them for reporting (at timestep level).

    // METHODOLOGY EMPLOYED:
    // Because all of the calculations are done on a "daily" basis in this
    // module, it is difficult to formulate the values that might be useful
    // for reporting.  SunlitFrac was the first of these two arrays to be
    // made into "two dimensions".  It is not clear that both have to be
    // two dimensions.

    // REFERENCES:
    // na

    // Using/Aliasing
    using namespace OutputReportPredefined;

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:
    // na

    // SUBROUTINE PARAMETER DEFINITIONS:
    // na

    // INTERFACE BLOCK SPECIFICATIONS
    // na

    // DERIVED TYPE DEFINITIONS
    // na

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int SurfNum; // Loop Counter
    int RepCol;  // the column of the predefined report

    auto &s_surf = state.dataSurface;
    for (SurfNum = 1; SurfNum <= s_surf->TotSurfaces; ++SurfNum) {
        auto &surf = s_surf->Surface(SurfNum);

        s_surf->SurfSunlitFrac(SurfNum) = state.dataHeatBal->SurfSunlitFrac(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum);
        s_surf->SurfSunlitArea(SurfNum) =
            state.dataHeatBal->SurfSunlitFrac(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum) * surf.Area;
    }
    // added for predefined reporting
    RepCol = 0;
    if (state.dataEnvrn->Month == 3 && state.dataEnvrn->DayOfMonth == 21) {
        if ((state.dataGlobal->HourOfDay == 9) && (state.dataGlobal->TimeStep == 4)) {
            RepCol = state.dataOutRptPredefined->pdchSlfMar21_9;
        } else if ((state.dataGlobal->HourOfDay == 12) && (state.dataGlobal->TimeStep == 4)) {
            RepCol = state.dataOutRptPredefined->pdchSlfMar21_12;
        } else if ((state.dataGlobal->HourOfDay == 15) && (state.dataGlobal->TimeStep == 4)) {
            RepCol = state.dataOutRptPredefined->pdchSlfMar21_15;
        }
    } else if (state.dataEnvrn->Month == 6 && state.dataEnvrn->DayOfMonth == 21) {
        if ((state.dataGlobal->HourOfDay == 9) && (state.dataGlobal->TimeStep == 4)) {
            RepCol = state.dataOutRptPredefined->pdchSlfJun21_9;
        } else if ((state.dataGlobal->HourOfDay == 12) && (state.dataGlobal->TimeStep == 4)) {
            RepCol = state.dataOutRptPredefined->pdchSlfJun21_12;
        } else if ((state.dataGlobal->HourOfDay == 15) && (state.dataGlobal->TimeStep == 4)) {
            RepCol = state.dataOutRptPredefined->pdchSlfJun21_15;
        }
    } else if (state.dataEnvrn->Month == 12 && state.dataEnvrn->DayOfMonth == 21) {
        if ((state.dataGlobal->HourOfDay == 9) && (state.dataGlobal->TimeStep == 4)) {
            RepCol = state.dataOutRptPredefined->pdchSlfDec21_9;
        } else if ((state.dataGlobal->HourOfDay == 12) && (state.dataGlobal->TimeStep == 4)) {
            RepCol = state.dataOutRptPredefined->pdchSlfDec21_12;
        } else if ((state.dataGlobal->HourOfDay == 15) && (state.dataGlobal->TimeStep == 4)) {
            RepCol = state.dataOutRptPredefined->pdchSlfDec21_15;
        }
    }
    if (RepCol != 0) {
        for (SurfNum = 1; SurfNum <= s_surf->TotSurfaces; ++SurfNum) {
            auto &surf = s_surf->Surface(SurfNum);
            if (surf.Class == SurfaceClass::Window) {
                PreDefTableEntry(state, RepCol, surf.Name, s_surf->SurfSunlitFrac(SurfNum));
            }
        }
    }
}

void ReportSurfaceErrors(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   November 2004
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine reports some recurring type errors that can get mixed up with more important
    // errors in the error file.

    using namespace DataErrorTracking; // for error tracking

    static Array1D_string const MSG(4, {"misses", "", "within", "overlaps"});

    int Loop1;
    int Loop2;
    int Count;
    int TotCount;
    Array1D_bool SurfErrorReported;
    Array1D_bool SurfErrorReported2;

    auto &s_surf = state.dataSurface;

    if (state.dataSolarShading->NumTooManyFigures + state.dataSolarShading->NumTooManyVertices + state.dataSolarShading->NumBaseSubSurround > 0) {
        ShowMessage(state, "");
        ShowMessage(state, "===== Recurring Surface Error Summary =====");
        ShowMessage(state, "The following surface error messages occurred.");
        ShowMessage(state, "");

        if (state.dataSolarShading->NumBaseSubSurround > 0) {
            ShowMessage(state, "Base Surface does not surround subsurface errors occurring...");
            ShowMessage(state,
                        "Check that the GlobalGeometryRules object is expressing the proper starting corner and "
                        "direction [CounterClockwise/Clockwise]");
            ShowMessage(state, "");
        }

        SurfErrorReported.dimension(s_surf->TotSurfaces, false);
        TotCount = 0;
        for (Loop1 = 1; Loop1 <= state.dataSolarShading->NumBaseSubSurround; ++Loop1) {
            Count = 0;
            if (SurfErrorReported(state.dataSolarShading->TrackBaseSubSurround(Loop1).SurfIndex1)) {
                continue;
            }
            for (Loop2 = 1; Loop2 <= state.dataSolarShading->NumBaseSubSurround; ++Loop2) {
                if (state.dataSolarShading->TrackBaseSubSurround(Loop1).SurfIndex1 ==
                        state.dataSolarShading->TrackBaseSubSurround(Loop2).SurfIndex1 &&
                    state.dataSolarShading->TrackBaseSubSurround(Loop1).MiscIndex == state.dataSolarShading->TrackBaseSubSurround(Loop2).MiscIndex) {
                    ++Count;
                }
            }
            TotCount += Count;
            state.dataErrTracking->TotalWarningErrors += Count - 1;
            ShowWarningError(state,
                             format("Base surface does not surround subsurface (CHKSBS), Overlap Status={}",
                                    state.dataSolarShading->cOverLapStatus(state.dataSolarShading->TrackBaseSubSurround(Loop1).MiscIndex)));
            ShowContinueError(state, format("  The base surround errors occurred {} times.", Count));
            for (Loop2 = 1; Loop2 <= state.dataSolarShading->NumBaseSubSurround; ++Loop2) {
                if (state.dataSolarShading->TrackBaseSubSurround(Loop1).SurfIndex1 ==
                        state.dataSolarShading->TrackBaseSubSurround(Loop2).SurfIndex1 &&
                    state.dataSolarShading->TrackBaseSubSurround(Loop1).MiscIndex == state.dataSolarShading->TrackBaseSubSurround(Loop2).MiscIndex) {
                    ShowContinueError(state,
                                      format("Surface \"{}\" {} SubSurface \"{}\"",
                                             s_surf->Surface(state.dataSolarShading->TrackBaseSubSurround(Loop1).SurfIndex1).Name,
                                             MSG(state.dataSolarShading->TrackBaseSubSurround(Loop1).MiscIndex),
                                             s_surf->Surface(state.dataSolarShading->TrackBaseSubSurround(Loop2).SurfIndex2).Name));
                }
            }
            SurfErrorReported(state.dataSolarShading->TrackBaseSubSurround(Loop1).SurfIndex1) = true;
        }
        if (TotCount > 0) {
            ShowMessage(state, "");
            ShowContinueError(state, format("  The base surround errors occurred {} times (total).", TotCount));
            ShowMessage(state, "");
        }

        SurfErrorReported2.allocate(s_surf->TotSurfaces);
        SurfErrorReported = false;
        TotCount = 0;
        if (state.dataSolarShading->NumTooManyVertices > 0) {
            ShowMessage(state, format("Too many vertices [>={}] in shadow overlap errors occurring...", state.dataSolarShading->MaxHCV));
            ShowMessage(state,
                        "These occur throughout the year and may occur several times for the same surfaces. You "
                        "may be able to reduce them by "
                        "adding Output:Diagnostics,DoNotMirrorDetachedShading;");
        }
        for (Loop1 = 1; Loop1 <= state.dataSolarShading->NumTooManyVertices; ++Loop1) {
            Count = 0;
            SurfErrorReported2 = false;
            if (SurfErrorReported(state.dataSolarShading->TrackTooManyVertices(Loop1).SurfIndex1)) {
                continue;
            }
            for (Loop2 = 1; Loop2 <= state.dataSolarShading->NumTooManyVertices; ++Loop2) {
                if (state.dataSolarShading->TrackTooManyVertices(Loop1).SurfIndex1 ==
                    state.dataSolarShading->TrackTooManyVertices(Loop2).SurfIndex1) {
                    ++Count;
                }
            }
            TotCount += Count;
            state.dataErrTracking->TotalWarningErrors += Count - 1;
            ShowMessage(state, "");
            ShowWarningError(state, format("Too many vertices [>={}] in a shadow overlap", state.dataSolarShading->MaxHCV));
            ShowContinueError(state,
                              format("Overlapping figure={}, Surface Class=[{}]",
                                     s_surf->Surface(state.dataSolarShading->TrackTooManyVertices(Loop1).SurfIndex1).Name,
                                     cSurfaceClass(s_surf->Surface(state.dataSolarShading->TrackTooManyVertices(Loop1).SurfIndex1).Class)));
            ShowContinueError(state, format("  This error occurred {} times.", Count));
            for (Loop2 = 1; Loop2 <= state.dataSolarShading->NumTooManyVertices; ++Loop2) {
                if (state.dataSolarShading->TrackTooManyVertices(Loop1).SurfIndex1 ==
                    state.dataSolarShading->TrackTooManyVertices(Loop2).SurfIndex1) {
                    if (SurfErrorReported2(state.dataSolarShading->TrackTooManyVertices(Loop2).SurfIndex2)) {
                        continue;
                    }
                    ShowContinueError(state,
                                      format("Figure being Overlapped={}, Surface Class=[{}]",
                                             s_surf->Surface(state.dataSolarShading->TrackTooManyVertices(Loop2).SurfIndex2).Name,
                                             cSurfaceClass(s_surf->Surface(state.dataSolarShading->TrackTooManyVertices(Loop2).SurfIndex2).Class)));
                    SurfErrorReported2(state.dataSolarShading->TrackTooManyVertices(Loop2).SurfIndex2) = true;
                }
            }
            SurfErrorReported(state.dataSolarShading->TrackTooManyVertices(Loop1).SurfIndex1) = true;
        }
        if (TotCount > 0) {
            ShowMessage(state, "");
            ShowContinueError(state, format("  The too many vertices errors occurred {} times (total).", TotCount));
            ShowMessage(state, "");
        }

        SurfErrorReported = false;
        TotCount = 0;
        if (state.dataSolarShading->NumTooManyFigures > 0) {
            ShowMessage(state, format("Too many figures [>={}] in shadow overlap errors occurring...", state.dataSolarShading->MaxHCS));
            ShowMessage(state,
                        "These occur throughout the year and may occur several times for the same surfaces. You "
                        "may be able to reduce them by "
                        "adding OutputDiagnostics,DoNotMirrorDetachedShading;");
        }
        for (Loop1 = 1; Loop1 <= state.dataSolarShading->NumTooManyFigures; ++Loop1) {
            Count = 0;
            SurfErrorReported2 = false;
            if (SurfErrorReported(state.dataSolarShading->TrackTooManyFigures(Loop1).SurfIndex1)) {
                continue;
            }
            for (Loop2 = 1; Loop2 <= state.dataSolarShading->NumTooManyFigures; ++Loop2) {
                if (state.dataSolarShading->TrackTooManyFigures(Loop1).SurfIndex1 == state.dataSolarShading->TrackTooManyFigures(Loop2).SurfIndex1) {
                    ++Count;
                }
            }
            TotCount += Count;
            state.dataErrTracking->TotalWarningErrors += Count - 1;
            ShowMessage(state, "");
            ShowWarningError(state, format("Too many figures [>={}] in a shadow overlap", state.dataSolarShading->MaxHCS));
            ShowContinueError(state,
                              format("Overlapping figure={}, Surface Class=[{}]",
                                     s_surf->Surface(state.dataSolarShading->TrackTooManyFigures(Loop1).SurfIndex1).Name,
                                     cSurfaceClass(s_surf->Surface(state.dataSolarShading->TrackTooManyFigures(Loop1).SurfIndex1).Class)));
            ShowContinueError(state, format("  This error occurred {} times.", Count));
            for (Loop2 = 1; Loop2 <= state.dataSolarShading->NumTooManyFigures; ++Loop2) {
                if (state.dataSolarShading->TrackTooManyFigures(Loop1).SurfIndex1 == state.dataSolarShading->TrackTooManyFigures(Loop2).SurfIndex1) {
                    if (SurfErrorReported2(state.dataSolarShading->TrackTooManyFigures(Loop2).SurfIndex2)) {
                        continue;
                    }
                    ShowContinueError(state,
                                      format("Figure being Overlapped={}, Surface Class=[{}]",
                                             s_surf->Surface(state.dataSolarShading->TrackTooManyFigures(Loop2).SurfIndex2).Name,
                                             cSurfaceClass(s_surf->Surface(state.dataSolarShading->TrackTooManyFigures(Loop2).SurfIndex2).Class)));
                    SurfErrorReported2(state.dataSolarShading->TrackTooManyFigures(Loop2).SurfIndex2) = true;
                }
            }
            SurfErrorReported(state.dataSolarShading->TrackTooManyFigures(Loop1).SurfIndex1) = true;
        }
        if (TotCount > 0) {
            ShowMessage(state, "");
            ShowContinueError(state, format("  The too many figures errors occurred {} times (total).", TotCount));
            ShowMessage(state, "");
        }
        SurfErrorReported.deallocate();
        SurfErrorReported2.deallocate();
    }
}

void ComputeWinShadeAbsorpFactors(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   Mar 2001
    //       MODIFIED       Oct 2002,FCW: change ConstrNumSh =
    //       WindowShadingControl(WinShadeCtrlNum)%ShadedConstruction
    //                      to Surface(SurfNum)%ShadedConstruction
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Called by InitSolarCalculations. Finds fractions that apportion radiation absorbed by a
    // window shade to the two faces of the shade. For radiation incident from the left,
    // ShadeAbsFacFace(1) is the fraction of radiation absorbed in the left-hand half of the
    // of the shade and ShadeAbsFacFace(2) is the fraction absorbed in the right-hand half.
    // The shade is assumed to be homogeneous.

    // REFERENCES: See EnergyPlus engineering documentation
    // USE STATEMENTS: na
    auto &s_mat = state.dataMaterial;
    auto &s_surf = state.dataSurface;

    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurfWin = thisSpace.WindowSurfaceFirst;
            int const lastSurfWin = thisSpace.WindowSurfaceLast;
            for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {
                auto &surf = s_surf->Surface(SurfNum);
                if (surf.Class == SurfaceClass::Window && surf.HasShadeControl) {
                    int WinShadeCtrlNum = surf.activeWindowShadingControl; // Window shading control number

                    int MatNumSh = 0;       // Shade layer material number
                    Real64 AbsorpEff = 0.0; // Effective absorptance of isolated shade layer (fraction of
                    //  of incident radiation remaining after reflected portion is
                    //  removed that is absorbed
                    if (ANY_SHADE(s_surf->WindowShadingControl(WinShadeCtrlNum).ShadingType)) {
                        int const ConstrNumSh = surf.activeShadedConstruction;                 // Window construction number with shade
                        int TotLay = state.dataConstruction->Construct(ConstrNumSh).TotLayers; // Total layers in a construction

                        if (s_surf->WindowShadingControl(WinShadeCtrlNum).ShadingType == WinShadingType::IntShade) {
                            MatNumSh = state.dataConstruction->Construct(ConstrNumSh).LayerPoint(TotLay); // Interior shade
                        } else if (s_surf->WindowShadingControl(WinShadeCtrlNum).ShadingType == WinShadingType::ExtShade) {
                            MatNumSh = state.dataConstruction->Construct(ConstrNumSh).LayerPoint(1); // Exterior shade
                        } else if (s_surf->WindowShadingControl(WinShadeCtrlNum).ShadingType == WinShadingType::BGShade) {
                            if (state.dataConstruction->Construct(ConstrNumSh).TotGlassLayers == 2) {
                                // Double pane with between-glass shade
                                MatNumSh = state.dataConstruction->Construct(ConstrNumSh).LayerPoint(3);
                            } else {
                                // Triple pane with between-glass shade
                                MatNumSh = state.dataConstruction->Construct(ConstrNumSh).LayerPoint(5);
                            }
                        }

                        auto const *matSh = s_mat->materials(MatNumSh);
                        auto const *matFenSh = dynamic_cast<Material::MaterialFen const *>(matSh);
                        assert(matFenSh != nullptr);
                        AbsorpEff = matFenSh->AbsorpSolar / (matFenSh->AbsorpSolar + matFenSh->Trans + 0.0001);
                        AbsorpEff = min(max(AbsorpEff, 0.0001),
                                        0.999); // Constrain to avoid problems with following log eval
                        s_surf->SurfWinShadeAbsFacFace1(SurfNum) = (1.0 - std::exp(0.5 * std::log(1.0 - AbsorpEff))) / AbsorpEff;
                        s_surf->SurfWinShadeAbsFacFace2(SurfNum) = 1.0 - s_surf->SurfWinShadeAbsFacFace1(SurfNum);
                    }
                }
            }
        }
    }
}

void CalcWinTransDifSolInitialDistribution(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rob Hitchcock
    //       DATE WRITTEN   July 2007
    //       MODIFIED       N/A
    //       RE-ENGINEERED  N/A

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calculates the initial distribution
    // of diffuse solar transmitted through exterior windows
    // to individual heat transfer surfaces in each zone(or enclosure).

    // METHODOLOGY EMPLOYED:
    // Apportions diffuse solar transmitted through each exterior window
    // that is then absorbed, reflected, and/or transmitted
    // by other heat transfer surfaces in the zone.
    // Calculations use:
    // 1. WinDifSolar calculated in SUBROUTINE CalcInteriorSolarDistribution,
    // 2. view factors between each exterior window and
    // other heat transfer surfaces in a zone
    // calculated in SUBROUTINE CalcApproximateViewFactors, and
    // 3. surface absorptances, reflectances, and transmittances
    // determined here using revised code from SUBROUTINE InitIntSolarDistribution

    // Using/Aliasing
    using namespace DataViewFactorInformation;
    using namespace DataWindowEquivalentLayer;

    Real64 AbsInt;               // Tmp var for Inside surface short-wave absorptance
    Real64 InsideDifAbsorptance; // Inside diffuse solar absorptance of a surface
    Real64 InsideDifReflectance; // Inside diffuse solar reflectance of a surface
    int BlNum;                   // Blind number
    Real64 BlAbsDiffBk;          // Glass layer back diffuse solar absorptance when blind in place
    Real64 AbsDiffBkBl;          // Blind diffuse back solar absorptance as part of glazing system

    //  REAL(r64)    :: DividerSolAbs      ! Window divider solar absorptance
    //  REAL(r64)    :: DividerSolRefl     ! Window divider solar reflectance
    //  INTEGER :: MatNumGl           ! Glass layer material number
    //  INTEGER :: MatNumSh           ! Shade layer material number
    //  REAL(r64)    :: TransGl,ReflGl,AbsGl ! Glass layer solar transmittance, reflectance, absorptance

    Real64 ViewFactor = 0.0;       // temp var for view factor
                                   //        Real64 ViewFactorTotal;             // debug var for view factor total
    Real64 WinDifSolarTrans = 0.0; // debug var for WinDifSolar() [W]
                                   //        Real64 WinDifSolarDistTotl;         // debug var for window total
                                   //        distributed diffuse solar [W] Real64 WinDifSolarDistAbsorbedTotl; // debug
                                   //        var for individual exterior window total distributed
                                   //    diffuse solar absorbed [W]
                                   //        Real64 WinDifSolarDistReflectedTotl; // debug var for individual exterior window total distributed
                                   //    diffuse solar reflected [W]
                                   //        Real64 WinDifSolarDistTransmittedTotl; // debug var for individual exterior window total distributed
                                   //    diffuse solar transmitted [W]
    Real64 WinDifSolLayAbsW = 0.0; // temp var for diffuse solar absorbed by individual glass layer [W]
                                   //        Real64 ZoneDifSolarTrans;               // debug var for WinDifSolar() [W]
                                   //        Real64 ZoneDifSolarDistTotl;            // debug var for zone total
                                   //        distributed diffuse solar [W] Real64 ZoneDifSolarDistAbsorbedTotl;    //
                                   //        debug var for zone total distributed diffuse solar absorbed [W] Real64
                                   //        ZoneDifSolarDistReflectedTotl;   // debug var for zone total distributed
                                   //        diffuse solar reflected [W] Real64 ZoneDifSolarDistTransmittedTotl; //
                                   //        debug var for zone total distributed diffuse solar transmitted [W]

    Real64 DifSolarAbsW = 0.0;     // temp var for diffuse solar absorbed by surface [W]
    Real64 DifSolarAbs = 0.0;      // temp var for diffuse solar absorbed by surface [W/m2]
    Real64 DifSolarReflW = 0.0;    // temp var for diffuse solar reflected by surface [W]
    Real64 ShBlDifSolarAbsW = 0.0; // temp var for diffuse solar absorbed by shade/blind [W]

    Array2D<Real64> SurfWinAbsSolBeamEQL(2, CFSMAXNL + 1); // absorbed exterior beam radiation by layers fraction
    Array2D<Real64> SurfWinAbsSolDiffEQL(2,
                                         CFSMAXNL + 1);        // absorbed exterior diffuse radiation by layers fraction
    Array2D<Real64> SurfWinAbsSolBeamBackEQL(2, CFSMAXNL + 1); // absorbed interior beam radiation by layers fraction from back
    Array2D<Real64> AbsSolDiffBackEQL(2, CFSMAXNL + 1);        // absorbed exterior diffuse radiation by layers fraction from back
    int EQLNum;                                                // equivalent layer fenestration index
    int Lay;                                                   // equivalent layer fenestration layer index

    auto &s_surf = state.dataSurface;

    // Init accumulators for absorbed diffuse solar for all surfaces for later heat balance calcs
    state.dataHeatBalSurf->SurfOpaqInitialDifSolInAbs = 0.0;
    state.dataHeatBal->SurfWinInitialDifSolwinAbs = 0.0;

    // Init accumulator for total reflected diffuse solar within each zone for interreflection calcs
    state.dataHeatBal->EnclSolInitialDifSolReflW = 0.0;

    // Init accumulator for transmitted diffuse solar for all surfaces for reporting
    state.dataHeatBalSurf->SurfWinInitialDifSolInTrans = 0.0;

    // Loop over all zones doing initial distribution of diffuse solar to interior heat transfer surfaces
    for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfRadiantEnclosures; ++enclosureNum) {
        auto &thisEnclosure = state.dataViewFactor->EnclSolInfo(enclosureNum);
        // Init Zone accumulators for debugging
        //            ZoneDifSolarTrans = 0.0;
        //            ZoneDifSolarDistAbsorbedTotl = 0.0;
        //            ZoneDifSolarDistReflectedTotl = 0.0;
        //            ZoneDifSolarDistTransmittedTotl = 0.0;
        // Loop over all diffuse solar transmitting surfaces (i.e., exterior windows and TDDs) in the current zone
        for (int const DifTransSurfNum : thisEnclosure.SurfacePtr) {
            // Skip surfaces that are not exterior, except for TDD_Diffusers
            auto &difTransSurf = s_surf->Surface(DifTransSurfNum);
            if ((difTransSurf.ExtBoundCond != ExternalEnvironment) && (difTransSurf.ExtBoundCond != OtherSideCondModeledExt) &&
                (difTransSurf.OriginalClass != SurfaceClass::TDD_Diffuser)) {
                continue;
            }

            // Do I need to do anything special for TDDs?
            //                if ( SurfaceWindow( DifTransSurfNum ).OriginalClass == SurfaceClass::TDD_Diffuser )
            //                {
            //                }

            // Skip surfaces that are not exterior windows or TDD diffusers
            if (difTransSurf.Class != SurfaceClass::Window && difTransSurf.OriginalClass != SurfaceClass::TDD_Diffuser) {
                continue;
            }

            //----------------------------------------------------------------------------------------------------------
            // DISTRIBUTE TRANSMITTED DIFFUSE SOLAR THROUGH EXTERIOR WINDOWS AND TDDS TO INTERIOR HEAT TRANSFER
            // SURFACES
            //----------------------------------------------------------------------------------------------------------

            // Init transmitted solar debug vars
            //                ViewFactorTotal = 0.0;
            WinDifSolarTrans = s_surf->SurfWinDifSolar(DifTransSurfNum);
            //                ZoneDifSolarTrans += WinDifSolarTrans;

            // Init Exterior Window accumulators for debugging
            //                WinDifSolarDistAbsorbedTotl = 0.0;
            //                WinDifSolarDistReflectedTotl = 0.0;
            //                WinDifSolarDistTransmittedTotl = 0.0;

            // Loop over all heat transfer surfaces in the current zone that might receive diffuse solar
            for (int const HeatTransSurfNum : thisEnclosure.SurfacePtr) {
                // Skip surfaces that are not heat transfer surfaces
                // Skip tubular daylighting device domes
                auto &surf = s_surf->Surface(HeatTransSurfNum);

                if (surf.Class == SurfaceClass::TDD_Dome) {
                    continue;
                }

                // View factor from current (sending) window DifTransSurfNum to current (receiving) surface
                // HeatTransSurfNum
                int const HTenclosureSurfNum = surf.SolarEnclSurfIndex; // HT surface index for EnclSolInfo.SurfacePtr and F arrays
                int const enclosureNum = surf.SolarEnclIndex;           // index for EnclSolInfo
                int const DTenclSurfNum =
                    s_surf->Surface(DifTransSurfNum).SolarEnclSurfIndex; // Window surface index for EnclSolInfo.SurfacePtr and F arrays

                ViewFactor = state.dataViewFactor->EnclSolInfo(enclosureNum).F(HTenclosureSurfNum, DTenclSurfNum);
                // debug ViewFactorTotal
                //                    ViewFactorTotal += ViewFactor; // debug

                // Skip receiving surfaces with 0.0 view factor
                if (ViewFactor <= 0.0) {
                    continue;
                }

                Real64 const WinDifSolarTrans_Factor(WinDifSolarTrans * ViewFactor);
                Real64 const win_SwitchingFactor(s_surf->SurfWinSwitchingFactor(HeatTransSurfNum));
                Real64 const per_HTSurfaceArea(1.0 / surf.Area);

                // Calculate diffuse solar from current exterior window absorbed and reflected by current heat
                // transfer surface And calculate transmitted diffuse solar to adjacent zones through interior
                // windows
                int const ConstrNum = s_surf->SurfActiveConstruction(HeatTransSurfNum);
                if (state.dataConstruction->Construct(ConstrNum).TransDiff <= 0.0) { // Interior Opaque Surface

                    // Determine the inside (back) diffuse solar absorptance
                    // and reflectance of the current heat transfer surface
                    InsideDifAbsorptance = state.dataHeatBalSurf->SurfAbsSolarInt(HeatTransSurfNum);
                    // Inside (back) diffuse solar reflectance is assumed to be 1 - absorptance
                    InsideDifReflectance = 1.0 - InsideDifAbsorptance;

                    // Absorbed diffuse solar [W] = current window transmitted diffuse solar [W]
                    //    * view factor from current (sending) window DifTransSurfNum to current (receiving)
                    //    surface HeatTransSurfNum
                    //    * current surface inside solar absorptance
                    DifSolarAbsW = WinDifSolarTrans_Factor * InsideDifAbsorptance; // [W]

                    // Absorbed diffuse solar [W/m2] = Absorbed diffuse solar [W]
                    //                                 / current surface net area
                    DifSolarAbs = DifSolarAbsW * per_HTSurfaceArea;

                    // Accumulate absorbed diffuse solar [W/m2] on this surface for heat balance calcs
                    state.dataHeatBalSurf->SurfOpaqInitialDifSolInAbs(HeatTransSurfNum) += DifSolarAbs;

                    // Reflected diffuse solar [W] = current window transmitted diffuse solar
                    //    * view factor from current (sending) window DifTransSurfNum to current (receiving)
                    //    surface HeatTransSurfNum
                    //    * current window inside solar reflectance
                    DifSolarReflW = WinDifSolarTrans_Factor * InsideDifReflectance;

                    // Accumulate total reflected distributed diffuse solar for each zone for subsequent
                    // interreflection calcs
                    state.dataHeatBal->EnclSolInitialDifSolReflW(enclosureNum) += DifSolarReflW; // [W]

                    // Accumulate Window and Zone total distributed diffuse solar to check for conservation of
                    // energy For opaque surfaces all incident diffuse is either absorbed or reflected

                } else { // Exterior or Interior Window
                    int const ConstrNumSh = s_surf->SurfWinActiveShadedConstruction(HeatTransSurfNum);
                    int TotGlassLayers = state.dataConstruction->Construct(ConstrNum).TotGlassLayers;
                    WinShadingType ShadeFlag = s_surf->SurfWinShadingFlag(HeatTransSurfNum);

                    if (s_surf->SurfWinWindowModelType(HeatTransSurfNum) != WindowModel::EQL) {
                        if (NOT_SHADED(ShadeFlag)) { // No window shading
                            // Init accumulator for transmittance calc below
                            DifSolarAbsW = 0.0;

                            // Calc diffuse solar absorbed by all window glass layers
                            // Note: I am assuming here that individual glass layer absorptances have been
                            // corrected
                            //       to account for layer by layer transmittance and reflection effects.
                            for (int IGlass = 1; IGlass <= TotGlassLayers; ++IGlass) {
                                // Calc diffuse solar absorbed from the inside by each window glass layer [W]
                                AbsInt = state.dataConstruction->Construct(ConstrNum).AbsDiffBack(IGlass);
                                WinDifSolLayAbsW = WinDifSolarTrans_Factor * state.dataConstruction->Construct(ConstrNum).AbsDiffBack(IGlass);

                                // Accumulate distributed diffuse solar absorbed [W] by overall window for
                                // transmittance calc below
                                DifSolarAbsW += WinDifSolLayAbsW;

                                // Accumulate diffuse solar absorbed from the inside by each window glass layer
                                // [W/m2] for heat balance calcs
                                state.dataHeatBal->SurfWinInitialDifSolwinAbs(HeatTransSurfNum, IGlass) += WinDifSolLayAbsW * per_HTSurfaceArea;
                            }

                            // Calc diffuse solar reflected back to zone
                            // I don't really care if this is a window or opaque surface since I am just
                            // accumulating all reflected diffuse solar in a zone bucket for "interreflected"
                            // distribution Reflected diffuse solar [W] = current window transmitted diffuse solar
                            //    * view factor from current (sending) window DifTransSurfNum to current
                            //    (receiving) surface HeatTransSurfNum
                            //    * current window inside solar reflectance
                            InsideDifReflectance = state.dataConstruction->Construct(ConstrNum).ReflectSolDiffBack;
                            DifSolarReflW = WinDifSolarTrans_Factor * state.dataConstruction->Construct(ConstrNum).ReflectSolDiffBack;

                            // Accumulate total reflected distributed diffuse solar for each zone for subsequent
                            // interreflection calcs
                            state.dataHeatBal->EnclSolInitialDifSolReflW(enclosureNum) += DifSolarReflW; // [W]

                            //------------------------------------------------------------------------------
                            // DISTRIBUTE TRANSMITTED DIFFUSE SOLAR THROUGH INTERIOR WINDOW TO ADJACENT ZONE
                            //------------------------------------------------------------------------------

                            // If this receiving window surface (HeatTransSurfNum) is an interior window,
                            // calc distributed solar transmitted to adjacent zone [W]
                            // NOTE: This calc is here because interior windows are currently assumed to have no
                            // shading

                            // Get the adjacent surface number for this receiving window surface
                            int AdjSurfNum = surf.ExtBoundCond;
                            // If the adjacent surface number is > 0, this is an interior window
                            if (AdjSurfNum > 0) { // this is an interior window surface

                                // Calc diffuse solar from current exterior window
                                // transmitted through this interior window to adjacent zone [W]
                                // Transmitted diffuse solar [W] = current exterior window transmitted diffuse
                                // solar
                                //    * view factor from current (sending) window DifTransSurfNum to current
                                //    (receiving) surface HeatTransSurfNum
                                //    - diffuse absorbed by this interior window
                                //    - diffuse reflected by this interior window
                                Real64 DifSolarTransThroughW = WinDifSolarTrans_Factor - DifSolarAbsW - DifSolarReflW;
                                // HERE 8/15/07 Note Construct(AdjConstrNum)%TransDiff could be used here since
                                // the "front" transmittance for an interior window in the adjacent zone is the
                                // correct direction as long as I use the Construct() of the Surface in the
                                // adjacent zone. However, the above calculation better conserves energy, although
                                // possibly at the expense of less accurate transmittance calcs. Preliminary tests
                                // showed fairly good agreement between the two DifSolarTransW calculation
                                // methods, but for consistency I stuck with the above. int AdjConstrNum =
                                // Surface(AdjSurfNum).Construction;
                                //              DifSolarTransW = WinDifSolar(DifTransSurfNum) &
                                //                                * ViewFactor &
                                //                                * Construct(AdjConstrNum)%TransDiff

                                // Get the adjacent zone index
                                int const adjEnclosureNum = s_surf->Surface(AdjSurfNum).SolarEnclIndex;

                                // Call routine to distribute diffuse solar transmitted through this interior
                                // window into adjacent zone
                                CalcInteriorWinTransDifSolInitialDistribution(state, adjEnclosureNum, AdjSurfNum, DifSolarTransThroughW);
                            }

                            // Calc transmitted Window and Zone total distributed diffuse solar to check for
                            // conservation of energy This is not very effective since it assigns whatever
                            // distributed diffuse solar has not been absorbed or reflected to transmitted.
                            // Should be just total less reflected
                            Real64 DifSolarTransIntoW = WinDifSolarTrans_Factor - DifSolarReflW;

                            // Accumulate transmitted diffuse solar for reporting
                            state.dataHeatBalSurf->SurfWinInitialDifSolInTrans(HeatTransSurfNum) += DifSolarTransIntoW * per_HTSurfaceArea;

                        } else if (ShadeFlag == WinShadingType::SwitchableGlazing) { // Switchable glazing
                            // Init accumulator for transmittance calc below
                            DifSolarAbsW = 0.0;

                            auto const &construct = state.dataConstruction->Construct(ConstrNum);
                            auto const &construct_AbsDiffBack = construct.AbsDiffBack;
                            auto const &construct_sh = state.dataConstruction->Construct(ConstrNumSh);
                            auto const &construct_sh_AbsDiffBack = construct_sh.AbsDiffBack;
                            for (int IGlass = 1; IGlass <= TotGlassLayers; ++IGlass) {
                                // Calc diffuse solar absorbed in each window glass layer
                                WinDifSolLayAbsW =
                                    WinDifSolarTrans_Factor *
                                    Window::InterpSw(win_SwitchingFactor, construct_AbsDiffBack(IGlass), construct_sh_AbsDiffBack(IGlass));

                                // Accumulate distributed diffuse solar absorbed [W] by overall window for
                                // transmittance calc below
                                DifSolarAbsW += WinDifSolLayAbsW;

                                // Accumulate diffuse solar absorbed from the inside by each window glass layer
                                // [W/m2] for heat balance calcs
                                state.dataHeatBal->SurfWinInitialDifSolwinAbs(HeatTransSurfNum, IGlass) += WinDifSolLayAbsW * per_HTSurfaceArea;
                            }

                            // Calc diffuse solar reflected back to zone
                            DifSolarReflW = WinDifSolarTrans_Factor *
                                            Window::InterpSw(win_SwitchingFactor, construct.ReflectSolDiffBack, construct_sh.ReflectSolDiffBack);

                            // Accumulate total reflected distributed diffuse solar for each zone for subsequent
                            // interreflection calcs
                            state.dataHeatBal->EnclSolInitialDifSolReflW(enclosureNum) += DifSolarReflW; // [W]

                            // Accumulate transmitted Window and Zone total distributed diffuse solar to check for
                            // conservation of energy This is not very effective since it assigns whatever
                            // distributed diffuse solar has not been absorbed or reflected to transmitted.
                            // Should be just total less reflected
                            Real64 DifSolarTransIntoW = WinDifSolarTrans_Factor - DifSolarReflW;

                            // Accumulate transmitted diffuse solar for reporting
                            state.dataHeatBalSurf->SurfWinInitialDifSolInTrans(HeatTransSurfNum) += DifSolarTransIntoW * per_HTSurfaceArea;

                        } else if (ConstrNumSh != 0) {
                            // Interior, exterior or between-glass shade, screen or blind in place

                            // Init accumulator for transmittance calc below
                            DifSolarAbsW = 0.0;
                            WinDifSolLayAbsW = 0.0;

                            // First calc diffuse solar absorbed by each glass layer in this window with
                            // shade/blind in place
                            auto const &constrSh = state.dataConstruction->Construct(ConstrNumSh);
                            auto const &surfShade = s_surf->surfShades(HeatTransSurfNum);
                            Real64 slatInterpFac = surfShade.blind.slatAngInterpFac;

                            for (int IGlass = 1; IGlass <= constrSh.TotGlassLayers; ++IGlass) {
                                if (ANY_SHADE_SCREEN(ShadeFlag)) {
                                    // Calc diffuse solar absorbed in each window glass layer and shade
                                    WinDifSolLayAbsW = WinDifSolarTrans_Factor * constrSh.AbsDiffBack(IGlass);
                                } else if (ANY_BLIND(ShadeFlag)) {
                                    auto const &slatDfAbsLo = constrSh.layerSlatBlindDfAbs(IGlass)[surfShade.blind.slatAngIdxLo];
                                    auto const &slatDfAbsHi = constrSh.layerSlatBlindDfAbs(IGlass)[surfShade.blind.slatAngIdxHi];
                                    BlAbsDiffBk = Interp(slatDfAbsLo.Sol.Bk.Df.Abs, slatDfAbsHi.Sol.Bk.Df.Abs, slatInterpFac);
                                    // Calc diffuse solar absorbed in each window glass layer and shade
                                    WinDifSolLayAbsW = WinDifSolarTrans_Factor * BlAbsDiffBk;
                                }

                                // Accumulate distributed diffuse solar absorbed [W] by overall window for
                                // transmittance calc below
                                DifSolarAbsW += WinDifSolLayAbsW;

                                // Accumulate diffuse solar absorbed from the inside by each window glass layer
                                // [W/m2] for heat balance calcs
                                state.dataHeatBal->SurfWinInitialDifSolwinAbs(HeatTransSurfNum, IGlass) += WinDifSolLayAbsW * per_HTSurfaceArea;
                            }

                            // Next calc diffuse solar reflected back to zone from window with shade or blind on
                            // Diffuse back solar reflectance, bare glass or shade on
                            InsideDifReflectance = state.dataConstruction->Construct(ConstrNum).ReflectSolDiffBack;
                            if ((ShadeFlag == WinShadingType::IntBlind) || (ShadeFlag == WinShadingType::ExtBlind)) {
                                auto const &constr = state.dataConstruction->Construct(ConstrNum);
                                auto const &surfShade = state.dataSurface->surfShades(HeatTransSurfNum);
                                auto const &btarSlatLo = constr.blindTARs[surfShade.blind.slatAngIdxLo];
                                auto const &btarSlatHi = constr.blindTARs[surfShade.blind.slatAngIdxHi];
                                Real64 slatInterpFac = surfShade.blind.slatAngInterpFac;
                                // Diffuse back solar reflectance, blind present, vs. slat angle
                                InsideDifReflectance = Interp(btarSlatLo.Sol.Bk.Df.Ref, btarSlatHi.Sol.Bk.Df.Ref, slatInterpFac);
                            }
                            DifSolarReflW = WinDifSolarTrans_Factor * InsideDifReflectance;

                            // Accumulate total reflected distributed diffuse solar for each zone for subsequent
                            // interreflection calcs
                            state.dataHeatBal->EnclSolInitialDifSolReflW(enclosureNum) += DifSolarReflW; // [W]

                            // Now calc diffuse solar absorbed by shade/blind itself
                            BlNum = surfShade.blind.matNum;
                            if (ANY_SHADE_SCREEN(ShadeFlag)) {
                                // Calc diffuse solar absorbed by shade or screen [W]
                                ShBlDifSolarAbsW = WinDifSolarTrans_Factor * constrSh.AbsDiffBackShade;
                            } else if (ANY_BLIND(ShadeFlag)) {
                                auto const &surfShade = state.dataSurface->surfShades(HeatTransSurfNum);
                                auto const &btarSlatLo = constrSh.blindTARs[surfShade.blind.slatAngIdxLo];
                                auto const &btarSlatHi = constrSh.blindTARs[surfShade.blind.slatAngIdxHi];
                                Real64 slatInterpFac = surfShade.blind.slatAngInterpFac;
                                AbsDiffBkBl = Interp(btarSlatLo.Sol.Bk.Df.Abs, btarSlatHi.Sol.Bk.Df.Abs, slatInterpFac);
                                ShBlDifSolarAbsW = WinDifSolarTrans_Factor * AbsDiffBkBl;
                            }
                            // Correct for divider shadowing
                            if (ANY_EXTERIOR_SHADE_BLIND_SCREEN(ShadeFlag)) {
                                ShBlDifSolarAbsW *= s_surf->SurfaceWindow(HeatTransSurfNum).glazedFrac;
                            }

                            // Accumulate diffuse solar absorbed  by shade or screen [W/m2] for heat balance calcs
                            s_surf->SurfWinInitialDifSolAbsByShade(HeatTransSurfNum) += ShBlDifSolarAbsW * per_HTSurfaceArea;

                            // Accumulate distributed diffuse solar absorbed [W] by overall window for
                            // transmittance calc below
                            DifSolarAbsW += ShBlDifSolarAbsW;

                            // Accumulate transmitted Window and Zone total distributed diffuse solar to check for
                            // conservation of energy This is not very effective since it assigns whatever
                            // distributed diffuse solar has not been absorbed or reflected to transmitted.
                            // Should be just total less reflected
                            Real64 DifSolarTransIntoW = WinDifSolarTrans_Factor - DifSolarReflW;

                            // Accumulate transmitted diffuse solar for reporting
                            state.dataHeatBalSurf->SurfWinInitialDifSolInTrans(HeatTransSurfNum) += DifSolarTransIntoW * per_HTSurfaceArea;
                        } // End of shading flag check

                    } else {
                        // SurfaceWindow(HeatTransSurfNum)%WindowModelType == WindowModel:: EQL
                        // ConstrNum=Surface(HeatTransSurfNum)%Construction
                        // call the ASHWAT fenestration model for diffuse radiation here
                        WindowEquivalentLayer::CalcEQLOpticalProperty(state, HeatTransSurfNum, SolarArrays::DIFF, AbsSolDiffBackEQL);

                        EQLNum = state.dataConstruction->Construct(ConstrNum).EQLConsPtr;
                        for (Lay = 1; Lay <= state.dataWindowEquivLayer->CFS(EQLNum).NL; ++Lay) {

                            // Calc diffuse solar absorbed from the inside by each layer of EQL model [W]
                            // WinDifSolLayAbsW = WinDifSolar(DifTransSurfNum)* ViewFactor *
                            // Construct(ConstrNum)%AbsDiffBack(Lay)
                            WinDifSolLayAbsW = WinDifSolarTrans_Factor * AbsSolDiffBackEQL(2, Lay);

                            // Accumulate distributed diffuse solar absorbed [W] by overall window for
                            // transmittance calc below
                            DifSolarAbsW += WinDifSolLayAbsW;

                            // Accumulate diffuse solar absorbed from the inside by each window layer [W/m2] for
                            // heat balance calcs
                            state.dataHeatBal->SurfWinInitialDifSolwinAbs(HeatTransSurfNum, Lay) += WinDifSolLayAbsW * per_HTSurfaceArea;

                            // ASHWAT equivalent layer model may require not the individual layer absorption but
                            // the flux InitialDifSolwinEQL(HeatTransSurfNum) = WinDifSolar(DifTransSurfNum)*
                            // ViewFactor
                        }

                        // Calc diffuse solar reflected back to zone
                        // I don't really care if this is a window or opaque surface since I am just
                        // accumulating all reflected diffuse solar in a zone bucket for "interreflected"
                        // distribution Reflected diffuse solar [W] = current window transmitted diffuse solar
                        //    * view factor from current (sending) window DifTransSurfNum to current (receiving)
                        //    surface HeatTransSurfNum
                        //    * current window inside solar reflectance
                        InsideDifReflectance = state.dataConstruction->Construct(ConstrNum).ReflectSolDiffBack;
                        DifSolarReflW = WinDifSolarTrans_Factor * state.dataConstruction->Construct(ConstrNum).ReflectSolDiffBack;

                        // Accumulate total reflected distributed diffuse solar for each zone for subsequent
                        // interreflection calcs
                        state.dataHeatBal->EnclSolInitialDifSolReflW(enclosureNum) += DifSolarReflW; // [W]

                        //------------------------------------------------------------------------------
                        // DISTRIBUTE TRANSMITTED DIFFUSE SOLAR THROUGH INTERIOR WINDOW TO ADJACENT ZONE
                        //------------------------------------------------------------------------------

                        // If this receiving window surface (HeatTransSurfNum) is an interior window,
                        // calc distributed solar transmitted to adjacent zone [W]
                        // NOTE: This calc is here because interior windows are currently assumed to have no
                        // shading

                        // Get the adjacent surface number for this receiving window surface
                        int const AdjSurfNum = s_surf->Surface(HeatTransSurfNum).ExtBoundCond;
                        // If the adjacent surface number is > 0, this is an interior window
                        if (AdjSurfNum > 0) { // this is an interior window surface

                            // Calc diffuse solar from current exterior window
                            // transmitted through this interior window to adjacent zone [W]
                            // Transmitted diffuse solar [W] = current exterior window transmitted diffuse solar
                            //    * view factor from current (sending) window DifTransSurfNum to current
                            //    (receiving) surface HeatTransSurfNum
                            Real64 DifSolarTransW = AbsSolDiffBackEQL(2, state.dataWindowEquivLayer->CFS(EQLNum).NL + 1) * ViewFactor;
                            // int AdjConstrNum = Surface(AdjSurfNum).Construction;
                            // Get the adjacent zone index
                            int adjEnclosureNum = s_surf->Surface(AdjSurfNum).SolarEnclIndex;
                            // Call routine to distribute diffuse solar transmitted through this interior window
                            // into adjacent zone
                            CalcInteriorWinTransDifSolInitialDistribution(state, adjEnclosureNum, AdjSurfNum, DifSolarTransW);
                        }

                        // Calc transmitted Window and Zone total distributed diffuse solar to check for
                        // conservation of energy This is not very effective since it assigns whatever
                        // distributed diffuse solar has not been absorbed or reflected to transmitted.
                        // Should be just total less reflected
                        Real64 DifSolarTransIntoW = WinDifSolarTrans_Factor - DifSolarReflW;

                        // Accumulate transmitted diffuse solar for reporting
                        state.dataHeatBalSurf->SurfWinInitialDifSolInTrans(HeatTransSurfNum) += DifSolarTransIntoW * per_HTSurfaceArea;

                    } // IF (SurfaceWindow(HeatTransSurfNum)%WindowModelType /= WindowModel:: EQL) THEN

                    // HERE 8/14/07 Ignore absorptance and reflectance of Frames and Dividers for now.
                    // I would need revised view factors that included these surface types.
                    // By ignoring them here, the diffuse solar is accounted for on the other surfaces

                    //          IF(SurfaceWindow(HeatTransSurfNum)%FrameArea > 0.0) THEN  ! Window has a frame
                    // Note that FrameQRadInAbs is initially calculated in InitSolarHeatGains
                    //          END IF

                    //          IF(SurfaceWindow(HeatTransSurfNum)%DividerArea > 0.0) THEN  ! Window has dividers
                    //            DividerSolAbs = SurfaceWindow(HeatTransSurfNum)%DividerSolAbsorp
                    //            IF(SurfaceWindow(HeatTransSurfNum)%DividerType == Suspended) THEN ! Suspended
                    //            divider; account for inside glass
                    //              MatNumGl = Construct(ConstrNum)%LayerPoint(Construct(ConstrNum)%TotLayers)
                    //              TransGl = dataMaterial.Material(MatNumGl)%Trans
                    //              ReflGl = dataMaterial.Material(MatNumGl)%ReflectSolDiffBack
                    //              AbsGl = 1.0d0-TransGl-ReflGl
                    //              DividerSolRefl = 1.0d0-DividerSolAbs
                    //              DividerSolAbs = AbsGl + TransGl*(DividerSolAbs +
                    //              DividerSolRefl*AbsGl)/(1.0d0-DividerSolRefl*ReflGl)
                    //            END IF
                    // Correct for interior shade transmittance
                    //            IF(ShadeFlag == IntShadeOn) THEN
                    //              MatNumSh = Construct(ConstrNumSh)%LayerPoint(Construct(ConstrNumSh)%TotLayers)
                    //              DividerSolAbs = DividerSolAbs * dataMaterial.Material(MatNumSh)%Trans
                    //            ELSE IF(ShadeFlag == WinShadingType::IntBlind) THEN
                    //              DividerSolAbs = DividerSolAbs *
                    //              InterpSlatAng(SurfaceWindow(HeatTransSurfNum)%SlatAngThisTS, &
                    //                  SurfaceWindow(HeatTransSurfNum)%MovableSlats,Blind(BlNum)%SolBackDiffDiffTrans)
                    //            END IF
                    // Note that DividerQRadInAbs is initially calculated in InitSolarHeatGains

                    //          END IF  ! Window has dividers

                } // opaque or window heat transfer surface

            } // HeatTransSurfNum = Zone(ZoneNum)%SurfaceFirst, Zone(ZoneNum)%SurfaceLast

            // Check debug var for view factors here
            // ViewFactorTotal
            // Check debug vars for individual transmitting surfaces here
            //                WinDifSolarDistTotl = WinDifSolarDistAbsorbedTotl + WinDifSolarDistReflectedTotl +
            //                WinDifSolarDistTransmittedTotl;
            // WinDifSolarTrans

        } // DifTransSurfNum = Zone(ZoneNum)%SurfaceFirst, Zone(ZoneNum)%SurfaceLast

        // Check debug vars for zone totals here
        //            ZoneDifSolarDistTotl = ZoneDifSolarDistAbsorbedTotl + ZoneDifSolarDistReflectedTotl +
        //            ZoneDifSolarDistTransmittedTotl;
        // ZoneDifSolarTrans
        // ZoneDifSolarDistAbsorbedTotl
        // ZoneDifSolarDistReflectedTotl
        // ZoneDifSolarDistTransmittedTotl
        //    CALL DisplayString('Diffuse Solar Distribution Zone Totals')

    } // ZoneNum = 1, NumOfZones
}
void CalcInteriorWinTransDifSolInitialDistribution(EnergyPlusData &state,
                                                   int const IntWinEnclosureNum,     // Interior Window Enclosure index number
                                                   int const IntWinSurfNum,          // Interior Window Surface number
                                                   Real64 const IntWinDifSolarTransW // Diffuse Solar transmitted through Interior Window
                                                                                     // IntWinSurfNum from adjacent enclosure [W]
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rob Hitchcock
    //       DATE WRITTEN   August 2007
    //       MODIFIED       N/A
    //       RE-ENGINEERED  N/A

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calculates the initial distribution
    // of diffuse solar transmitted through the given interior window
    // to individual heat transfer surfaces in the given enclosure.
    // Diffuse solar transmitted through interior windows in this enclosure
    // to adjacent enclosures, is added to the EnclSolInitialDifSolReflW
    // of the adjacent enclosure for subsequent interreflection calcs

    // METHODOLOGY EMPLOYED:
    // Similar to method used in CalcWinTransDifSolInitialDistribution.
    // Apportions diffuse solar transmitted through an interior window
    // that is then absorbed, reflected, and/or transmitted
    // by other heat transfer surfaces in the given enclosure.
    // Calculations use:
    // 1. DifSolarTransW calculated in SUBROUTINE CalcWinTransDifSolInitialDistribution,
    // 2. view factors between the interior window and
    // other heat transfer surfaces in the given enclosure
    // calculated in SUBROUTINE CalcApproximateViewFactors, and
    // 3. surface absorptances, reflectances, and transmittances
    // determined here using revised code from SUBROUTINE InitIntSolarDistribution

    // Using/Aliasing
    using namespace DataViewFactorInformation;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int IGlass;                  // Glass layer counter
    int TotGlassLayers;          // Number of glass layers in a window construction
    WinShadingType ShadeFlag;    // Shading flag
    Real64 AbsInt;               // Tmp var for Inside surface short-wave absorptance
    Real64 InsideDifAbsorptance; // Inside diffuse solar absorptance of a surface
    Real64 InsideDifReflectance; // Inside diffuse solar reflectance of a surface
    int BlNum;                   // Blind number
    Real64 BlAbsDiffBk;          // Glass layer back diffuse solar absorptance when blind in place
    Real64 AbsDiffBkBl;          // Blind diffuse back solar absorptance as part of glazing system

    //  REAL(r64)    :: DividerSolAbs      ! Window divider solar absorptance
    //  REAL(r64)    :: DividerSolRefl     ! Window divider solar reflectance
    //  INTEGER :: MatNumGl           ! Glass layer material number
    //  INTEGER :: MatNumSh           ! Shade layer material number
    //  REAL(r64)    :: TransGl,ReflGl,AbsGl ! Glass layer solar transmittance, reflectance, absorptance

    Real64 ViewFactor;       // temp var for view factor
    Real64 ViewFactorTotal;  // debug var for view factor total
    Real64 WinDifSolarTrans; // debug var for WinDifSolar() [W]
                             //        Real64 WinDifSolarDistTotl; // debug var for window total distributed
                             //        diffuse solar [W] Real64 WinDifSolarDistAbsorbedTotl( 0.0 ); // debug var
                             //        for individual exterior window total
                             // distributed
    //           diffuse solar absorbed [W]
    //        Real64 WinDifSolarDistReflectedTotl( 0.0 ); // debug var for individual exterior window total
    //        distributed
    //           diffuse solar reflected [W]
    //        Real64 WinDifSolarDistTransmittedTotl( 0.0 ); // debug var for individual exterior window total
    //        distributed
    //           diffuse solar transmitted [W]
    Real64 WinDifSolLayAbsW; // temp var for diffuse solar absorbed by individual glass layer [W]
                             //        Real64 ZoneDifSolarTrans( 0.0 ); // debug var for WinDifSolar() [W]
    //  REAL(r64)    :: ZoneDifSolarDistTotl    ! debug var for zone total distributed diffuse solar [W]
    //        Real64 ZoneDifSolarDistAbsorbedTotl( 0.0 ); // debug var for zone total distributed diffuse solar
    //        absorbed [W] Real64 ZoneDifSolarDistReflectedTotl( 0.0 ); // debug var for zone total distributed
    //        diffuse solar reflected [W] Real64 ZoneDifSolarDistTransmittedTotl( 0.0 ); // debug var for zone
    //        total distributed diffuse solar transmitted [W]

    Real64 DifSolarAbsW = 0.0;     // temp var for diffuse solar absorbed by surface [W]
    Real64 DifSolarAbs = 0.0;      // temp var for diffuse solar absorbed by surface [W/m2]
    Real64 DifSolarReflW = 0.0;    // temp var for diffuse solar reflected by surface [W]
    Real64 DifSolarTransW = 0.0;   // temp var for diffuse solar transmitted through interior window surface [W]
    Real64 ShBlDifSolarAbsW = 0.0; // temp var for diffuse solar absorbed by shade/blind [W]

    //-------------------------------------------------------------------------------------------------
    // DISTRIBUTE TRANSMITTED DIFFUSE SOLAR THROUGH INTERIOR WINDOW TO INTERIOR HEAT TRANSFER SURFACES
    //-------------------------------------------------------------------------------------------------

    auto &s_surf = state.dataSurface;

    // Init debug vars
    ViewFactorTotal = 0.0;
    WinDifSolarTrans = IntWinDifSolarTransW;

    auto &thisEnclosure = state.dataViewFactor->EnclSolInfo(IntWinEnclosureNum);
    // Loop over all heat transfer surfaces in the current zone that might receive diffuse solar
    Real64 InitialZoneDifSolReflW_zone(0.0);
    for (int const HeatTransSurfNum : thisEnclosure.SurfacePtr) {

        auto &surf = s_surf->Surface(HeatTransSurfNum);

        // Skip surfaces that are not heat transfer surfaces
        if (!surf.HeatTransSurf) {
            continue;
        }
        // Skip tubular daylighting device domes
        if (surf.Class == SurfaceClass::TDD_Dome) {
            continue;
        }

        // View factor from current (sending) window IntWinSurfNum to current (receiving) surface HeatTransSurfNum
        int HTenclosureSurfNum = surf.SolarEnclSurfIndex;                          // HT surface index for EnclSolInfo.SurfacePtr and F arrays
        int enclosureNum = surf.SolarEnclIndex;                                    // index for EnclSolInfo
        int IntWinEnclSurfNum = s_surf->Surface(IntWinSurfNum).SolarEnclSurfIndex; // Window surface index for EnclSolInfo.SurfacePtr and F arrays

        ViewFactor = state.dataViewFactor->EnclSolInfo(enclosureNum).F(HTenclosureSurfNum, IntWinEnclSurfNum);
        // debug ViewFactorTotal
        ViewFactorTotal += ViewFactor; // debug

        // Skip receiving surfaces with 0.0 view factor
        if (ViewFactor <= 0.0) {
            continue;
        }
        Real64 const SolarTrans_ViewFactor(IntWinDifSolarTransW * ViewFactor);

        // Calculate diffuse solar from current interior window absorbed and reflected by current heat transfer
        // surface And calculate transmitted diffuse solar to adjacent zones through interior windows
        int const ConstrNum = s_surf->SurfActiveConstruction(HeatTransSurfNum);
        if (state.dataConstruction->Construct(ConstrNum).TransDiff <= 0.0) { // Interior Opaque Surface

            // Determine the inside (back) diffuse solar absorptance
            // and reflectance of the current heat transfer surface
            InsideDifAbsorptance = state.dataHeatBalSurf->SurfAbsSolarInt(HeatTransSurfNum);
            // Inside (back) diffuse solar reflectance is assumed to be 1 - absorptance
            InsideDifReflectance = 1.0 - InsideDifAbsorptance;

            // Absorbed diffuse solar [W] = current window transmitted diffuse solar [W]
            //    * view factor from current (sending) window IntWinSurfNum to current (receiving) surface
            //    HeatTransSurfNum
            //    * current surface inside solar absorptance
            DifSolarAbsW = SolarTrans_ViewFactor * InsideDifAbsorptance; // [W]

            // Absorbed diffuse solar [W/m2] = Absorbed diffuse solar [W]
            //                                 / current surface net area
            DifSolarAbs = DifSolarAbsW / surf.Area;

            // Accumulate absorbed diffuse solar [W/m2] on this surface for heat balance calcs
            state.dataHeatBalSurf->SurfOpaqInitialDifSolInAbs(HeatTransSurfNum) += DifSolarAbs;

            // Reflected diffuse solar [W] = current window transmitted diffuse solar
            //    * view factor from current (sending) window IntWinSurfNum to current (receiving) surface
            //    HeatTransSurfNum
            //    * current window inside solar reflectance
            DifSolarReflW = SolarTrans_ViewFactor * InsideDifReflectance;

            // Accumulate total reflected distributed diffuse solar for each zone for subsequent interreflection
            // calcs
            InitialZoneDifSolReflW_zone += DifSolarReflW; // [W]

            // Accumulate Window and Zone total distributed diffuse solar to check for conservation of energy
            // For opaque surfaces all incident diffuse is either absorbed or reflected
            //                WinDifSolarDistAbsorbedTotl += DifSolarAbsW; // debug [W]
            //                WinDifSolarDistReflectedTotl += DifSolarReflW; // debug [W]
            //                ZoneDifSolarDistAbsorbedTotl += DifSolarAbsW; // debug [W]
            //                ZoneDifSolarDistReflectedTotl += DifSolarReflW; // debug [W]

        } else { // Exterior or Interior Window

            int const ConstrNumSh = s_surf->SurfWinActiveShadedConstruction(HeatTransSurfNum);
            auto const &construction = state.dataConstruction->Construct(ConstrNum);

            TotGlassLayers = construction.TotGlassLayers;
            ShadeFlag = s_surf->SurfWinShadingFlag(HeatTransSurfNum);

            if (NOT_SHADED(ShadeFlag)) { // No window shading
                // Init accumulator for transmittance calc below
                DifSolarAbsW = 0.0;

                // Calc diffuse solar absorbed by all window glass layers
                // Note: I am assuming here that individual glass layer absorptances have been corrected
                //       to account for layer by layer transmittance and reflection effects.
                for (IGlass = 1; IGlass <= TotGlassLayers; ++IGlass) {
                    // Calc diffuse solar absorbed from the inside by each window glass layer [W]
                    AbsInt = construction.AbsDiffBack(IGlass);
                    WinDifSolLayAbsW = SolarTrans_ViewFactor * construction.AbsDiffBack(IGlass);

                    // Accumulate distributed diffuse solar absorbed [W] by overall window for transmittance calc
                    // below
                    DifSolarAbsW += WinDifSolLayAbsW;

                    // Accumulate diffuse solar absorbed from the inside by each window glass layer [W/m2] for
                    // heat balance calcs
                    state.dataHeatBal->SurfWinInitialDifSolwinAbs(HeatTransSurfNum, IGlass) += (WinDifSolLayAbsW / surf.Area);
                }
                // Accumulate Window and Zone total distributed diffuse solar to check for conservation of energy
                //                    WinDifSolarDistAbsorbedTotl += DifSolarAbsW; // debug
                //                    ZoneDifSolarDistAbsorbedTotl += DifSolarAbsW; // debug

                // Calc diffuse solar reflected back to zone
                // I don't really care if this is a window or opaque surface since I am just
                // accumulating all reflected diffuse solar in a zone bucket for "interreflected" distribution
                // Reflected diffuse solar [W] = current window transmitted diffuse solar
                //    * view factor from current (sending) window IntWinSurfNum to current (receiving) surface
                //    HeatTransSurfNum
                //    * current window inside solar reflectance
                DifSolarReflW = SolarTrans_ViewFactor * construction.ReflectSolDiffBack;

                // Accumulate total reflected distributed diffuse solar for each zone for subsequent
                // interreflection calcs
                InitialZoneDifSolReflW_zone += DifSolarReflW; // [W]

                // Accumulate Window and Zone total distributed diffuse solar to check for conservation of energy

                //                    WinDifSolarDistReflectedTotl += DifSolarReflW; // debug
                //                    ZoneDifSolarDistReflectedTotl += DifSolarReflW; // debug

                // Calc transmitted Window and Zone total distributed diffuse solar to check for conservation of
                // energy This is not very effective since it assigns whatever distributed diffuse solar has not
                // been absorbed or reflected to transmitted.
                DifSolarTransW = SolarTrans_ViewFactor - DifSolarAbsW - DifSolarReflW;

                // Accumulate transmitted Window and Zone total distributed diffuse solar to check for
                // conservation of energy
                //                    WinDifSolarDistTransmittedTotl += DifSolarTransW; // debug [W]
                //                    ZoneDifSolarDistTransmittedTotl += DifSolarTransW; // debug [W]

                // Accumulate transmitted diffuse solar for reporting
                state.dataHeatBalSurf->SurfWinInitialDifSolInTrans(HeatTransSurfNum) += (SolarTrans_ViewFactor - DifSolarReflW) / surf.Area;

                //-----------------------------------------------------------------------------------
                // ADD TRANSMITTED DIFFUSE SOLAR THROUGH INTERIOR WINDOW TO ADJACENT ZONE
                // TOTAL REFLECTED DIFFUSE SOLAR FOR SUBSEQUENT INTERREFLECTION CALCS
                //-----------------------------------------------------------------------------------

                // If this receiving window surface (HeatTransSurfNum) is an interior window,
                // add transmitted diffuse solar to adjacent zone total reflected distributed
                // diffuse solar for subsequent interreflection calcs
                // NOTE: This calc is here because interior windows are currently assumed to have no shading

                // Get the adjacent surface number for this receiving window surface
                int const AdjSurfNum = surf.ExtBoundCond;
                // If the adjacent surface number is > 0, this is an interior window
                if (AdjSurfNum > 0) { // this is an interior window surface

                    // Get the adjacent zone/enclosure index
                    // Add transmitted diffuse solar to total reflected distributed diffuse solar for each zone
                    // for subsequent interreflection calcs
                    state.dataHeatBal->EnclSolInitialDifSolReflW(s_surf->Surface(AdjSurfNum).SolarEnclIndex) += DifSolarTransW; // [W]
                }

            } else if (ShadeFlag == WinShadingType::SwitchableGlazing) { // Switchable glazing
                // Init accumulator for transmittance calc below
                auto const &constructionSh = state.dataConstruction->Construct(ConstrNumSh);
                DifSolarAbsW = 0.0;

                for (IGlass = 1; IGlass <= TotGlassLayers; ++IGlass) {
                    // Calc diffuse solar absorbed in each window glass layer
                    WinDifSolLayAbsW = SolarTrans_ViewFactor * InterpSw(s_surf->SurfWinSwitchingFactor(HeatTransSurfNum),
                                                                        construction.AbsDiffBack(IGlass),
                                                                        constructionSh.AbsDiffBack(IGlass));

                    // Accumulate distributed diffuse solar absorbed [W] by overall window for transmittance calc
                    // below
                    DifSolarAbsW += WinDifSolLayAbsW;

                    // Accumulate diffuse solar absorbed from the inside by each window glass layer [W/m2] for
                    // heat balance calcs
                    state.dataHeatBal->SurfWinInitialDifSolwinAbs(HeatTransSurfNum, IGlass) += (WinDifSolLayAbsW / surf.Area);
                }
                // Accumulate Window and Zone total distributed diffuse solar to check for conservation of energy
                //					WinDifSolarDistAbsorbedTotl += DifSolarAbsW; // debug
                //					ZoneDifSolarDistAbsorbedTotl += DifSolarAbsW; // debug

                // Calc diffuse solar reflected back to zone
                DifSolarReflW =
                    SolarTrans_ViewFactor *
                    InterpSw(s_surf->SurfWinSwitchingFactor(HeatTransSurfNum), construction.ReflectSolDiffBack, constructionSh.ReflectSolDiffBack);

                // Accumulate total reflected distributed diffuse solar for each zone for subsequent
                // interreflection calcs
                InitialZoneDifSolReflW_zone += DifSolarReflW; // [W]

                // Accumulate Window and Zone total distributed diffuse solar to check for conservation of energy
                //					WinDifSolarDistReflectedTotl += DifSolarReflW; // debug
                //					ZoneDifSolarDistReflectedTotl += DifSolarReflW; // debug

                // Accumulate transmitted Window and Zone total distributed diffuse solar to check for
                // conservation of energy This is not very effective since it assigns whatever distributed diffuse
                // solar has not been absorbed or reflected to transmitted.
                DifSolarTransW = SolarTrans_ViewFactor - DifSolarAbsW - DifSolarReflW;
                //					WinDifSolarDistTransmittedTotl += DifSolarTransW; // debug [W]
                //					ZoneDifSolarDistTransmittedTotl += DifSolarTransW; // debug
                //[W]

                // Accumulate transmitted diffuse solar for reporting
                state.dataHeatBalSurf->SurfWinInitialDifSolInTrans(HeatTransSurfNum) += (SolarTrans_ViewFactor - DifSolarReflW) / surf.Area;

            } else {
                auto const &surfShade = s_surf->surfShades(HeatTransSurfNum);
                // Interior, exterior or between-glass shade, screen or blind in place

                // Init accumulator for transmittance calc below
                DifSolarAbsW = 0.0;
                WinDifSolLayAbsW = 0.0;

                // First calc diffuse solar absorbed by each glass layer in this window with shade/blind in place
                auto const &constrSh = state.dataConstruction->Construct(ConstrNumSh);
                for (IGlass = 1; IGlass <= constrSh.TotGlassLayers; ++IGlass) {
                    if (ANY_SHADE_SCREEN(ShadeFlag)) {
                        // Calc diffuse solar absorbed in each window glass layer and shade
                        WinDifSolLayAbsW = SolarTrans_ViewFactor * constrSh.AbsDiffBack(IGlass);
                    } else if (ANY_BLIND(ShadeFlag)) {
                        auto const &surfShade = state.dataSurface->surfShades(HeatTransSurfNum);
                        auto const &dfAbsSlatLo = constrSh.layerSlatBlindDfAbs(IGlass)[surfShade.blind.slatAngIdxLo];
                        auto const &dfAbsSlatHi = constrSh.layerSlatBlindDfAbs(IGlass)[surfShade.blind.slatAngIdxHi];
                        Real64 slatInterpFac = surfShade.blind.slatAngInterpFac;
                        BlAbsDiffBk = Interp(dfAbsSlatLo.Sol.Bk.Df.Abs, dfAbsSlatHi.Sol.Bk.Df.Abs, slatInterpFac);
                        // Calc diffuse solar absorbed in each window glass layer and shade
                        WinDifSolLayAbsW = SolarTrans_ViewFactor * BlAbsDiffBk;
                    }

                    // Accumulate distributed diffuse solar absorbed [W] by overall window for transmittance calc
                    // below
                    DifSolarAbsW += WinDifSolLayAbsW;

                    // Accumulate diffuse solar absorbed from the inside by each window glass layer [W/m2] for
                    // heat balance calcs
                    state.dataHeatBal->SurfWinInitialDifSolwinAbs(HeatTransSurfNum, IGlass) += (WinDifSolLayAbsW / surf.Area);
                }
                // Accumulate Window and Zone total distributed diffuse solar to check for conservation of energy
                //                    WinDifSolarDistAbsorbedTotl += DifSolarAbsW; // debug
                //                    ZoneDifSolarDistAbsorbedTotl += DifSolarAbsW; // debug

                // Next calc diffuse solar reflected back to zone from window with shade or blind on
                // Diffuse back solar reflectance, bare glass or shade on
                InsideDifReflectance = construction.ReflectSolDiffBack;
                if ((ShadeFlag == WinShadingType::IntBlind) || (ShadeFlag == WinShadingType::ExtBlind)) {
                    auto const &surfShade = state.dataSurface->surfShades(HeatTransSurfNum);
                    auto const &btarSlatLo = constrSh.blindTARs[surfShade.blind.slatAngIdxLo];
                    auto const &btarSlatHi = constrSh.blindTARs[surfShade.blind.slatAngIdxHi];
                    Real64 slatInterpFac = surfShade.blind.slatAngInterpFac;
                    // Diffuse back solar reflectance, blind present, vs. slat angle
                    InsideDifReflectance = Interp(btarSlatLo.Sol.Bk.Df.Ref, btarSlatHi.Sol.Bk.Df.Ref, slatInterpFac);
                }
                DifSolarReflW = SolarTrans_ViewFactor * InsideDifReflectance;

                // Accumulate total reflected distributed diffuse solar for each zone for subsequent
                // interreflection calcs
                InitialZoneDifSolReflW_zone += DifSolarReflW; // [W]

                // Accumulate Window and Zone total distributed diffuse solar to check for conservation of energy
                //                    WinDifSolarDistReflectedTotl += DifSolarReflW; // debug
                //                    ZoneDifSolarDistReflectedTotl += DifSolarReflW; // debug

                // Now calc diffuse solar absorbed by shade/blind itself
                BlNum = surfShade.blind.matNum;
                if (ANY_SHADE_SCREEN(ShadeFlag)) {
                    // Calc diffuse solar absorbed by shade or screen [W]
                    ShBlDifSolarAbsW = SolarTrans_ViewFactor * constrSh.AbsDiffBackShade;
                } else if (ANY_BLIND(ShadeFlag)) {
                    auto const &surfShade = state.dataSurface->surfShades(HeatTransSurfNum);
                    auto const &btarSlatLo = constrSh.blindTARs[surfShade.blind.slatAngIdxLo];
                    auto const &btarSlatHi = constrSh.blindTARs[surfShade.blind.slatAngIdxHi];
                    Real64 slatInterpFac = surfShade.blind.slatAngInterpFac;
                    // Calc diffuse solar absorbed by blind [W]
                    AbsDiffBkBl = Interp(btarSlatLo.Sol.Bk.Df.Abs, btarSlatHi.Sol.Bk.Df.Abs, slatInterpFac);
                    ShBlDifSolarAbsW = SolarTrans_ViewFactor * AbsDiffBkBl;
                }
                // Correct for divider shadowing
                if (ANY_EXTERIOR_SHADE_BLIND_SCREEN(ShadeFlag)) {
                    ShBlDifSolarAbsW *= s_surf->SurfaceWindow(HeatTransSurfNum).glazedFrac;
                }

                // Accumulate diffuse solar absorbed  by shade or screen [W/m2] for heat balance calcs
                s_surf->SurfWinInitialDifSolAbsByShade(HeatTransSurfNum) += (ShBlDifSolarAbsW / surf.Area);

                // Accumulate distributed diffuse solar absorbed [W] by overall window for transmittance calc
                // below
                DifSolarAbsW += ShBlDifSolarAbsW;

                // Accumulate Window and Zone total distributed diffuse solar to check for conservation of energy
                //                    WinDifSolarDistAbsorbedTotl += ShBlDifSolarAbsW; // debug
                //                    ZoneDifSolarDistAbsorbedTotl += ShBlDifSolarAbsW; // debug

                // Accumulate transmitted Window and Zone total distributed diffuse solar to check for
                // conservation of energy This is not very effective since it assigns whatever distributed diffuse
                // solar has not been absorbed or reflected to transmitted.
                DifSolarTransW = SolarTrans_ViewFactor - DifSolarAbsW - DifSolarReflW;
                //                    WinDifSolarDistTransmittedTotl += DifSolarTransW; // debug [W]
                //                    ZoneDifSolarDistTransmittedTotl += DifSolarTransW; // debug [W]

                // Accumulate transmitted diffuse solar for reporting
                state.dataHeatBalSurf->SurfWinInitialDifSolInTrans(HeatTransSurfNum) += (SolarTrans_ViewFactor - DifSolarReflW) / surf.Area;

            } // End of shading flag check

            // HERE 8/14/07 Ignore absorptance and reflectance of Frames and Dividers for now.
            // I would need revised view factors that included these surface types.
            // By ignoring them here, the diffuse solar is accounted for on the other surfaces

            //          IF(SurfaceWindow(HeatTransSurfNum)%FrameArea > 0.0) THEN  ! Window has a frame
            // Note that FrameQRadInAbs is initially calculated in InitSolarHeatGains
            //          END IF

            //          IF(SurfaceWindow(HeatTransSurfNum)%DividerArea > 0.0) THEN  ! Window has dividers
            //            DividerSolAbs = SurfaceWindow(HeatTransSurfNum)%DividerSolAbsorp
            //            IF(SurfaceWindow(HeatTransSurfNum)%DividerType == Suspended) THEN ! Suspended divider;
            //            account for inside glass
            //              MatNumGl = Construct(ConstrNum)%LayerPoint(Construct(ConstrNum)%TotLayers)
            //              TransGl = dataMaterial.Material(MatNumGl)%Trans
            //              ReflGl = dataMaterial.Material(MatNumGl)%ReflectSolDiffBack
            //              AbsGl = 1.0d0-TransGl-ReflGl
            //              DividerSolRefl = 1.0d0-DividerSolAbs
            //              DividerSolAbs = AbsGl + TransGl*(DividerSolAbs +
            //              DividerSolRefl*AbsGl)/(1.0d0-DividerSolRefl*ReflGl)
            //            END IF
            // Correct for interior shade transmittance
            //            IF(ShadeFlag == IntShadeOn) THEN
            //              MatNumSh = Construct(ConstrNumSh)%LayerPoint(Construct(ConstrNumSh)%TotLayers)
            //              DividerSolAbs = DividerSolAbs * dataMaterial.Material(MatNumSh)%Trans
            //            ELSE IF(ShadeFlag == WinShadingType::IntBlind) THEN
            //              DividerSolAbs = DividerSolAbs *
            //              InterpSlatAng(SurfaceWindow(HeatTransSurfNum)%SlatAngThisTS, &
            //                  SurfaceWindow(HeatTransSurfNum)%MovableSlats,Blind(BlNum)%SolBackDiffDiffTrans)
            //            END IF
            // Note that DividerQRadInAbs is initially calculated in InitSolarHeatGains

            //          END IF  ! Window has dividers
        } // opaque or window heat transfer surface

    } // HeatTransSurfNum = Zone(ZoneNum)%SurfaceFirst, Zone(ZoneNum)%SurfaceLast
    state.dataHeatBal->EnclSolInitialDifSolReflW(IntWinEnclosureNum) += InitialZoneDifSolReflW_zone;

    // Check debug var for view factors here
    // ViewFactorTotal
    // Check debug vars for individual transmitting surfaces here
    //        WinDifSolarDistTotl = WinDifSolarDistAbsorbedTotl + WinDifSolarDistReflectedTotl +
    //        WinDifSolarDistTransmittedTotl; //Debug
    // WinDifSolarTrans
}

void CalcComplexWindowOverlap(EnergyPlusData &state,
                              BSDFGeomDescr &Geom,               // State Geometry
                              BSDFWindowGeomDescr const &Window, // Window Geometry
                              int const ISurf                    // Surface number of the complex fenestration
)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   May 2012
    //       MODIFIED       Simon Vidanovic (May 2013) - added overlaps calculations for daylighting
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // For each of basis directions on back surface of the window calculates
    // overlap areas. It also calculates overlap areas and reflectances for daylighting calculations

    using namespace Vectors;

    Real64 XShadowProjection; // temporary buffer
    Real64 YShadowProjection; // temporary buffer

    Real64 XSp;            // for calc BSDF projection direction
    Real64 YSp;            // for calc BSDF projection direction
    Real64 ZSp;            // for calc BSDF projection direction
    Real64 SdotX;          // temporary variable for manipulating .dot. product
    Real64 SdotY;          // temporary variable for manipulating .dot. product
    Real64 SdotZ;          // temporary variable for manipulating .dot. product
    int BackSurfaceNumber; // current back surface number
    int NVT;               // Number of vertices of back surface
    int NS1;               // Number of the figure being overlapped
    int NS2;               // Number of the figure doing overlapping
    int NS3;               // Location to place results of overlap
    int IRay;              // Current ray of BSDF direction
    int KBkSurf;           // Current back surface
    int N;

    // Daylighting
    int IConst;                // Construction number of back surface
    int InsideConLay;          // Construction's inside material layer number
    Real64 VisibleReflectance; // Visible reflectance for inside surface material
    Real64 TotAOverlap;        // Total overlap area for given outgoing direction
    Real64 TotARhoVisOverlap;  // Total overlap area time reflectance for given outgoing direction

    auto &s_surf = state.dataSurface;

    state.dataSolarShading->XVertex.dimension(s_surf->MaxVerticesPerSurface + 1, 0.0);
    state.dataSolarShading->YVertex.dimension(s_surf->MaxVerticesPerSurface + 1, 0.0);
    state.dataSolarShading->ZVertex.dimension(s_surf->MaxVerticesPerSurface + 1, 0.0);

    Geom.AOverlap.dimension(Window.NBkSurf, Geom.Trn.NBasis, 0.0);
    Geom.ARhoVisOverlap.dimension(Window.NBkSurf, Geom.Trn.NBasis, 0.0);
    Geom.AveRhoVisOverlap.dimension(Geom.Trn.NBasis, 0.0);

    // First to calculate and store coordinates of the window surface
    state.dataSolarShading->LOCHCA = 1;
    int BaseSurf = s_surf->Surface(ISurf).BaseSurf; // Base surface number

    // Base surface contains current window surface (ISurf).
    // Since that is case, below transformation should always return ZVT = 0.0
    // for every possible transformation
    CTRANS(state, ISurf, BaseSurf, NVT, state.dataSolarShading->XVertex, state.dataSolarShading->YVertex, state.dataSolarShading->ZVertex);

    // HTRANS routine is using coordinates stored in XVS and YVS in order to calculate
    // surface area.  Since both projections are equal to zero, then simply
    // copy these values into XVS and YVS arrays
    for (N = 1; N <= NVT; ++N) {
        state.dataSolarShading->XVS(N) = state.dataSolarShading->XVertex(N);
        state.dataSolarShading->YVS(N) = state.dataSolarShading->YVertex(N);
    }

    // This calculates the area stored in XVS and YVS
    HTRANS1(state, state.dataSolarShading->LOCHCA, NVT);

    // Calculation of overlap areas for each outgoing basis direction
    for (IRay = 1; IRay <= Geom.Trn.NBasis; ++IRay) { // basis directions loop (on back surface)
        // For current basis direction calculate dot product between window surface
        // and basis direction.  This will be used to calculate projection of each
        // of the back surfaces to window surface for given basis direction
        SdotX = dot(s_surf->Surface(ISurf).lcsx, Geom.sTrn(IRay));
        SdotY = dot(s_surf->Surface(ISurf).lcsy, Geom.sTrn(IRay));
        SdotZ = dot(s_surf->Surface(ISurf).lcsz, Geom.sTrn(IRay));
        XSp = -SdotX;
        YSp = -SdotY;
        ZSp = -SdotZ;

        // Projection of shadows for current basis direction
        if (std::abs(ZSp) > Constant::SmallDistance) {
            XShadowProjection = XSp / ZSp;
            YShadowProjection = YSp / ZSp;
            if (std::abs(XShadowProjection) < 1.e-8) {
                XShadowProjection = 0.0;
            }
            if (std::abs(YShadowProjection) < 1.e-8) {
                YShadowProjection = 0.0;
            }
        } else {
            XShadowProjection = 0.0;
            YShadowProjection = 0.0;
        }

        for (KBkSurf = 1; KBkSurf <= Window.NBkSurf; ++KBkSurf) { // back surf loop
            // BaseSurf = Surface(ISurf).BaseSurf
            BackSurfaceNumber = state.dataShadowComb->ShadowComb(BaseSurf).BackSurf(KBkSurf);

            // Transform coordinates of back surface from general system to the
            // plane of the receiving surface
            CTRANS(state,
                   BackSurfaceNumber,
                   BaseSurf,
                   NVT,
                   state.dataSolarShading->XVertex,
                   state.dataSolarShading->YVertex,
                   state.dataSolarShading->ZVertex);

            // Project "shadow" from back surface along sun's rays to receiving surface.  Back surface vertices
            // become clockwise sequential.

            for (N = 1; N <= NVT; ++N) {
                state.dataSolarShading->YVS(N) = state.dataSolarShading->YVertex(N) - YShadowProjection * state.dataSolarShading->ZVertex(N);
                state.dataSolarShading->XVS(N) = state.dataSolarShading->XVertex(N) - XShadowProjection * state.dataSolarShading->ZVertex(N);
            }

            // Transform to the homogeneous coordinate system.

            NS3 = state.dataSolarShading->LOCHCA + 1;
            state.dataSolarShading->HCT(NS3) = 0.0;
            HTRANS1(state, NS3, NVT);

            // Determine area of overlap of projected back surface and receiving surface.

            NS1 = 1;
            NS2 = NS3;
            state.dataSolarShading->HCT(NS3) = 1.0;
            DeterminePolygonOverlap(state, NS1, NS2, NS3);

            if (state.dataSolarShading->OverlapStatus == NoOverlap) {
                continue; // to next back surface
            }
            if ((state.dataSolarShading->OverlapStatus == TooManyVertices) || (state.dataSolarShading->OverlapStatus == TooManyFigures)) {
                break; // back surfaces DO loop
            }

            state.dataSolarShading->LOCHCA = NS3;
            state.dataSolarShading->HCNS(state.dataSolarShading->LOCHCA) = BackSurfaceNumber;
            state.dataSolarShading->HCAREA(state.dataSolarShading->LOCHCA) = -state.dataSolarShading->HCAREA(state.dataSolarShading->LOCHCA);

            Geom.AOverlap(KBkSurf, IRay) = state.dataSolarShading->HCAREA(state.dataSolarShading->LOCHCA);
        } // DO KBkSurf  = 1 , NBkSurf

        // If some of back surfaces is contained in base surface, then need to subtract shadow of subsurface
        // from shadow on base surface.  Reason is that above shadowing algorithm is calculating shadow without
        // influence of subsurfaces
        for (KBkSurf = 1; KBkSurf <= Window.NBkSurf; ++KBkSurf) { // back surf loop
            BackSurfaceNumber = state.dataShadowComb->ShadowComb(BaseSurf).BackSurf(KBkSurf);
            // CurBaseSurf is Current base surface number for shadow overlap calculations
            int CurBaseSurf = s_surf->Surface(BackSurfaceNumber).BaseSurf;
            if (CurBaseSurf != BackSurfaceNumber) {
                // Search if that base surface in list of back surfaces for current window
                // CurBackSurface is Current back surface number for base surface
                int CurBackSurface = 0;
                for (N = 1; N <= Window.NBkSurf; ++N) {
                    if (state.dataShadowComb->ShadowComb(BaseSurf).BackSurf(N) == CurBaseSurf) {
                        CurBackSurface = N;
                        break;
                    }
                }
                if (CurBackSurface != 0) {
                    Geom.AOverlap(CurBackSurface, IRay) -= Geom.AOverlap(KBkSurf, IRay);
                }
            }
        }

        auto &s_mat = state.dataMaterial;

        // Calculate overlap area times reflectance.  This is necessary for complex fenestration daylighting
        // calculations
        TotAOverlap = 0.0;
        TotARhoVisOverlap = 0.0;
        for (KBkSurf = 1; KBkSurf <= Window.NBkSurf; ++KBkSurf) { // back surf loop
            BackSurfaceNumber = state.dataShadowComb->ShadowComb(BaseSurf).BackSurf(KBkSurf);
            IConst = s_surf->Surface(BackSurfaceNumber).Construction;
            InsideConLay = state.dataConstruction->Construct(IConst).TotLayers;
            if (s_surf->SurfWinWindowModelType(BackSurfaceNumber) == WindowModel::BSDF) {
                VisibleReflectance = state.dataConstruction->Construct(IConst).ReflectVisDiffBack;
            } else {
                VisibleReflectance = (1.0 - s_mat->materials(InsideConLay)->AbsorpVisible);
            }
            Geom.ARhoVisOverlap(KBkSurf, IRay) = Geom.AOverlap(KBkSurf, IRay) * VisibleReflectance;
            TotAOverlap += Geom.AOverlap(KBkSurf, IRay);
            TotARhoVisOverlap += Geom.ARhoVisOverlap(KBkSurf, IRay);
        }

        if (TotAOverlap != 0.0) {
            Geom.AveRhoVisOverlap(IRay) = TotARhoVisOverlap / TotAOverlap;
        }

    } // DO IRay = 1, Geom%Trn%NBasis

    // Reset back shadowing counter since complex windows do not need it anymore
    state.dataSolarShading->LOCHCA = 1;
}

void TimestepInitComplexFenestration(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   May 2012
    //       MODIFIED       May 2012 (Initialize complex fenestration in case of EMS)
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Performs initialization of complex fenestration. It also performs check if current surface containing
    // complex fenestration have construction changed (by EMS) in which case performs addition of current states
    // into complex fenestration array

    using WindowComplexManager::CheckCFSStates;

    // Locals
    int iSurf;       // Current surface number
    int iState;      // current state number
    int NumOfStates; // number of states for current window

    auto &s_surf = state.dataSurface;

    for (iSurf = 1; iSurf <= s_surf->TotSurfaces; ++iSurf) {
        if (s_surf->SurfWinWindowModelType(iSurf) == WindowModel::BSDF) {
            // This will check complex fenestrations state and add new one if necessary (EMS case)
            CheckCFSStates(state, iSurf);

            NumOfStates = state.dataBSDFWindow->ComplexWind(iSurf).NumStates;

            // Check for overlap areas and initialize if necessary
            for (iState = 1; iState <= NumOfStates; ++iState) {
                // do initialization only once
                if (state.dataBSDFWindow->ComplexWind(iSurf).Geom(iState).InitState) {
                    CalcComplexWindowOverlap(
                        state, state.dataBSDFWindow->ComplexWind(iSurf).Geom(iState), state.dataBSDFWindow->ComplexWind(iSurf), iSurf);
                    state.dataBSDFWindow->ComplexWind(iSurf).Geom(iState).InitState = false;
                }
            }
        }
    }
}

} // namespace EnergyPlus::SolarShading

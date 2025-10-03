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
#include <string>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>
#include <ObjexxFCL/Vector3.hh>
// #include <ObjexxFCL/Vector4.hh>
#include <ObjexxFCL/member.functions.hh>
#include <ObjexxFCL/random.hh>
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/DElightManagerF.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBSDFWindow.hh>
#include <EnergyPlus/DataDElight.hh>
#include <EnergyPlus/DataDaylighting.hh>
#include <EnergyPlus/DataDaylightingDevices.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataErrorTracking.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/DataStringGlobals.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/DataViewFactorInformation.hh>
#include <EnergyPlus/DaylightingDevices.hh>
#include <EnergyPlus/DaylightingManager.hh>
#include <EnergyPlus/DisplayRoutines.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatBalanceManager.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/PierceSurface.hh>
#include <EnergyPlus/SQLiteProcedures.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SolarReflectionManager.hh>
#include <EnergyPlus/SolarShading.hh>
#include <EnergyPlus/SurfaceOctree.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WindowComplexManager.hh>
#include <EnergyPlus/WindowManager.hh>

namespace EnergyPlus::Dayltg {

// MODULE INFORMATION
//       AUTHOR         Fred Winkelmann
//       DATE WRITTEN   July 1997, December 1998
//       MODIFIED       Oct 2004; LKL -- Efficiencies and code restructure
//                      Aug 2012: BG -- Added availability schedule

// PURPOSE OF THIS MODULE:
// Manages the daylighting calculations for each thermal zone that has an associated
// Daylighting:Controls object.

// Includes calculation of interior daylight illuminance and glare
// from each of the windows in a zone, control of window shading devices
// to reduce glare, and control of overhead electric lighting in response
// to interior daylight illuminance level at one or two user-specified
// reference points at which sensors are located.

// METHODOLOGY EMPLOYED:
// REFERENCES:
// "Daylighting Calculation in DOE-2," F.C.Winkelmann, LBL-11353, May 1983
// "Daylighting Simulation in the DOE-2 Building Energy Analysis Program,"
// F.C. Winkelmann and S. Selkowitz, Energy and Buildings 8(1985)271-286

// OTHER NOTES:
// This module was created from DOE-2.1E subroutines.

// Correspondence between DOE-2.1E and EnergyPlus subroutine names:

// DOE-2.1E    EnergyPlus                      In Module           Called from Module
// DAVREF      DayltgAveInteriorReflectance    DaylightingManager DaylightingManager
// DCOF        CalcDayltgCoefficients          DaylightingManager DaylightingManager
// DCROSS      DayltgCrossProduct              DaylightingManager DaylightingManager
// DEXTIL      DayltgCurrentExtHorizIllum      WeatherManager     WeatherManager
// DGLARE      DayltgGlare                     DaylightingManager DaylightingManager
// DHILL       DayltgExtHorizIllum             DaylightingManager DaylightingManager
// DHITSH      DayltgHitObstruction            DaylightingManager DaylightingManager
// DINTIL      DayltgInteriorIllum             DaylightingManager HeatBalanceSurfaceManager
// DLTSYS      DayltgElecLightingControl       DaylightingManager HeatBalanceSurfaceManager
// DNSOL       not used
// DPFAC       DayltgPositionFactor            DaylightingManager DaylightingManager
// DPIERC      PierceSurface                   PierceSurface      DaylightingManager
// DREFLT      DayltgInterReflectedIllum       DaylightingManager DaylightingManager
// DSKYLU      DayltgSkyLuminance              DaylightingManager DaylightingManager
// DTHLIM      DayltgAzimuthLimits             DaylightingManager DaylightingManager
// DLUMEF      DayltgLuminousEfficacy          WeatherManager     WeatherManager

// Using/Aliasing
using namespace DataSurfaces;

void DayltgAveInteriorReflectance(EnergyPlusData &state, int const enclNum) // Enclosure number
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   July 1997
    //       MODIFIED       Mar 2004, FCW: add calculation of following SurfaceWindow variables:
    //                        EnclAreaMinusThisSurf, EnclAreaReflProdMinusThisSurf, RhoCeilingWall,
    //                        RhoFloorWall, FractionUpgoing. Add calculation of ZoneDaylight%floorVisRefl.

    // PURPOSE OF THIS SUBROUTINE:
    // Called by CalcDayltgCoefficients for each daylit zone. Determines total
    // area and area-weighted average visible reflectance of
    // all inside faces of the surfaces of a zone.  In addition, finds
    // area and average reflectance of interzone, underground and exterior
    // heat-transfer surfaces in the following categories: floor (tilt > 170 deg),
    // ceiling (tilt < 10 deg), and wall (10 < tilt < 170 deg).
    // The window reflectance values used here assume the windows have no shading
    // devices. This information is used in the calculation of the
    // internally-reflected daylighting component.

    // Finds total number of exterior windows in the space.

    // REFERENCES:
    // Based on DOE-2.1E subroutine DAVREF
    auto &dl = state.dataDayltg;
    auto &s_surf = state.dataSurface;

    // Total inside surface area, including windows
    Real64 AInsTot = 0.0;
    // Sum of products of inside surface area * vis reflectance
    Real64 ARHTOT = 0.0;

    // Area sum and area * reflectance sum for different orientations
    std::array<Real64, (int)FWC::Num> AR = {0.0, 0.0, 0.0};
    std::array<Real64, (int)FWC::Num> ARH = {0.0, 0.0, 0.0};
    // Loop over surfaces in the zone's enclosure

    auto &thisEnclosure = state.dataViewFactor->EnclSolInfo(enclNum);
    for (int ISurf : thisEnclosure.SurfacePtr) {
        auto const &surf = s_surf->Surface(ISurf);

        SurfaceClass IType = surf.Class;
        // Error if window has multiplier > 1 since this causes incorrect illuminance calc
        if (IType == SurfaceClass::Window && surf.Multiplier > 1.0) {
            if (thisEnclosure.TotalEnclosureDaylRefPoints > 0) {
                ShowSevereError(state, format("DayltgAveInteriorReflectance: Multiplier > 1.0 for window {} in Zone={}", surf.Name, surf.ZoneName));
                ShowContinueError(state, "...not allowed since it is in a zone or enclosure with daylighting.");
                ShowFatalError(state, "Program terminates due to preceding conditions.");
            } else {
                ShowSevereError(state, format("DayltgAveInteriorReflectance: Multiplier > 1.0 for window {} in Zone={}", surf.Name, surf.ZoneName));
                ShowContinueError(state, "...an adjacent Zone has daylighting. Simulation cannot proceed.");
                ShowFatalError(state, "Program terminates due to preceding conditions.");
            }
        }
        if (IType == SurfaceClass::Wall || IType == SurfaceClass::Floor || IType == SurfaceClass::Roof || IType == SurfaceClass::Window ||
            IType == SurfaceClass::Door) {
            Real64 AREA = surf.Area;
            // In following, FrameArea and DividerArea can be non-zero only for exterior windows
            AInsTot += AREA + s_surf->SurfWinFrameArea(ISurf) * (1.0 + 0.5 * s_surf->SurfWinProjCorrFrIn(ISurf)) +
                       s_surf->SurfWinDividerArea(ISurf) * (1.0 + s_surf->SurfWinProjCorrDivIn(ISurf));
            ARHTOT +=
                AREA * state.dataConstruction->Construct(surf.Construction).ReflectVisDiffBack +
                s_surf->SurfWinFrameArea(ISurf) * (1.0 + 0.5 * s_surf->SurfWinProjCorrFrIn(ISurf)) * (1.0 - s_surf->SurfWinFrameSolAbsorp(ISurf)) +
                s_surf->SurfWinDividerArea(ISurf) * (1.0 + s_surf->SurfWinProjCorrDivIn(ISurf)) * (1.0 - s_surf->SurfWinDividerSolAbsorp(ISurf));

            FWC fwc = FWC::Ceiling; // Ceiling
            if (surf.Tilt > 10.0 && surf.Tilt < 170.0) {
                fwc = FWC::Wall; // Wall
            }
            if (surf.Tilt >= 170.0) {
                fwc = FWC::Floor; // Floor
            }
            AR[(int)fwc] += AREA + s_surf->SurfWinFrameArea(ISurf) * (1.0 + 0.5 * s_surf->SurfWinProjCorrFrIn(ISurf)) +
                            s_surf->SurfWinDividerArea(ISurf) * (1.0 + s_surf->SurfWinProjCorrDivIn(ISurf));
            ARH[(int)fwc] +=
                AREA * state.dataConstruction->Construct(surf.Construction).ReflectVisDiffBack +
                s_surf->SurfWinFrameArea(ISurf) * (1.0 + 0.5 * s_surf->SurfWinProjCorrFrIn(ISurf)) * (1.0 - s_surf->SurfWinFrameSolAbsorp(ISurf)) +
                s_surf->SurfWinDividerArea(ISurf) * (1.0 + s_surf->SurfWinProjCorrDivIn(ISurf)) * (1.0 - s_surf->SurfWinDividerSolAbsorp(ISurf));
        }
    }

    // Average inside surface reflectance of enclosure
    if (AInsTot <= 0.0) {
        ShowSevereError(state, format("DayltgAveInteriorReflectance: Total opaque surface area is <=0.0 in solar enclosure={}", thisEnclosure.Name));
        ShowFatalError(state, "Program terminates due to preceding conditions.");
    }
    dl->enclDaylight(enclNum).aveVisDiffReflect = ARHTOT / AInsTot;
    // Total inside surface area of enclosure
    dl->enclDaylight(enclNum).totInsSurfArea = AInsTot;
    // Average floor visible reflectance
    dl->enclDaylight(enclNum).floorVisRefl = ARH[iFWC_Ceiling] / (AR[iFWC_Ceiling] + 1.e-6);

    for (int ISurf : thisEnclosure.SurfacePtr) {
        auto const &surf = s_surf->Surface(ISurf);
        if (surf.Class != SurfaceClass::Wall && surf.Class != SurfaceClass::Floor && surf.Class != SurfaceClass::Roof) {
            continue;
        }

        // Remove this surface from the space inside surface area and area*reflectivity
        // The resulting areas are AP(ITILT). The resulting area*reflectivity is ARHP(ITILT).
        // Initialize gross area of surface (including subsurfaces)
        Real64 ATWL = surf.Area; // This is the surface area less subsurfaces
        // Area * reflectance for this surface, excluding attached windows and doors
        Real64 ARHTWL = surf.Area * state.dataConstruction->Construct(surf.Construction).ReflectVisDiffBack;

        FWC fwc = (surf.Tilt > 45.0 && surf.Tilt < 135.0) ? FWC::Wall : ((surf.Tilt >= 135.0) ? FWC::Floor : FWC::Ceiling);

        // Loop over windows and doors on this wall
        for (int IWinDr : thisEnclosure.SurfacePtr) {
            auto const &surfWinDr = s_surf->Surface(IWinDr);
            if ((surfWinDr.Class != SurfaceClass::Window && surfWinDr.Class != SurfaceClass::Door) || surfWinDr.BaseSurf != ISurf) {
                continue;
            }

            ATWL += surfWinDr.Area + s_surf->SurfWinFrameArea(IWinDr) * (1.0 + 0.5 * s_surf->SurfWinProjCorrFrIn(IWinDr)) +
                    s_surf->SurfWinDividerArea(IWinDr) * (1.0 + s_surf->SurfWinProjCorrDivIn(IWinDr));
            ARHTWL +=
                surfWinDr.Area * state.dataConstruction->Construct(surfWinDr.Construction).ReflectVisDiffBack +
                s_surf->SurfWinFrameArea(IWinDr) * (1.0 + 0.5 * s_surf->SurfWinProjCorrFrIn(IWinDr)) * (1.0 - s_surf->SurfWinFrameSolAbsorp(IWinDr)) +
                s_surf->SurfWinDividerArea(IWinDr) * (1.0 + s_surf->SurfWinProjCorrDivIn(IWinDr)) * (1.0 - s_surf->SurfWinDividerSolAbsorp(IWinDr));
        }

        std::array<Real64, (int)FWC::Num> AP;
        std::array<Real64, (int)FWC::Num> ARHP;
        // Inside surface area of floor, walls and ceilings, minus surface ISurf and its subsurfaces
        for (int iFWC = iFWC_Floor; iFWC < (int)FWC::Num; ++iFWC) {
            if (iFWC == (int)fwc) {
                AP[iFWC] = AR[iFWC] - ATWL;
                ARHP[iFWC] = ARH[iFWC] - ARHTWL;
            } else {
                AP[iFWC] = AR[iFWC];
                ARHP[iFWC] = ARH[iFWC];
            }
        }
        s_surf->SurfaceWindow(ISurf).EnclAreaMinusThisSurf = AP;
        s_surf->SurfaceWindow(ISurf).EnclAreaReflProdMinusThisSurf = ARHP;
    } // for (ISurf)

    for (int IWin : thisEnclosure.SurfacePtr) {
        auto const &surf = s_surf->Surface(IWin);
        if (surf.Class != SurfaceClass::Window) {
            continue;
        }

        auto &surfWin = s_surf->SurfaceWindow(IWin);
        auto const &zone = state.dataHeatBal->Zone(surf.Zone);
        int ISurf = surf.BaseSurf;

        // Ratio of floor-to-window-center height and average floor-to-ceiling height
        Real64 ETA = max(0.0, min(1.0, (surfWin.WinCenter.z - zone.OriginZ) * zone.FloorArea / zone.Volume));

        std::array<Real64, (int)FWC::Num> AP = s_surf->SurfaceWindow(ISurf).EnclAreaMinusThisSurf;
        std::array<Real64, (int)FWC::Num> ARHP = s_surf->SurfaceWindow(ISurf).EnclAreaReflProdMinusThisSurf;
        // Average reflectance seen by light moving up (RhoCeilingWall) and down (RhoFloorWall)
        // across horizontal plane through center of window
        surfWin.rhoCeilingWall = (ARHP[iFWC_Wall] * (1.0 - ETA) + ARHP[iFWC_Ceiling]) / (AP[iFWC_Wall] * (1.0 - ETA) + AP[iFWC_Ceiling] + 1.0e-5);
        surfWin.rhoFloorWall = (ARHP[iFWC_Wall] * ETA + ARHP[iFWC_Floor]) / (AP[iFWC_Wall] * ETA + AP[iFWC_Floor] + 1.e-9);

        // Angle factor for windows with diffusing shades. SurfaceWindow(IWin)%FractionUpgoing is
        // fraction of light from the shade that goes up toward ceiling and upper part of walls.
        // 1 - SurfaceWindow(IWin)%FractionUpgoing is fraction that goes down toward floor and lower part of walls.
        surfWin.fractionUpgoing = surf.Tilt / 180.0;

        // Daylighting shelf simplification:  All light goes up to the ceiling regardless of orientation of shelf
        if (s_surf->SurfDaylightingShelfInd(IWin) > 0) {
            if (state.dataDaylightingDevicesData->Shelf(s_surf->SurfDaylightingShelfInd(IWin)).InSurf > 0) {
                surfWin.fractionUpgoing = 1.0;
            }
        }
    } // for (IWin)
} // DayltgAveInteriorReflectance()

void CalcDayltgCoefficients(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   July 1997
    //       MODIFIED       FW, Jan 2002: add variable slat angle blinds
    //                      FW, Mar 2002: add triangular windows
    //                      FW, Oct 2002: remove warning on window discretization relative to
    //                                    reference point distance to window plane
    //                      FW, Jan 2003: add between-glass shades and blinds
    //                      FW, Apr 2003: initialize shading type to 'NOSHADE' in window loop
    //                      PE, May 2003: add light pipes (tubular daylighting devices)
    //                      FW, Jul 2003: account for possible non-zero transmittance of
    //                                    shading surfaces (previously all shading surfaces were
    //                                    assumed to be opaque)
    //                      PE, Aug 2003: add daylighting shelves
    //                      FW, Sep 2003: write the bare-window overcast sky daylight factors to the eio file
    //                      FW, Nov 2003: add exterior beam and sky solar diffuse reflection from obstructions;
    //                                    add beam solar and sky solar reflection from ground with obstructions.
    //                      FW, Nov 2003: change expression for NDIVX, NDIVY (no. of window elements in X,Y) to
    //                                    round up to nearest integer rather than down
    //                      FW, Nov 2003: add specular reflection of beam solar from obstructions
    //                      RJH, Jan 2004: add alternative daylighting analysis using DElight
    //                                     All modifications demarked with RJH (Rob Hitchcock)
    //                      FW, Feb 2004: add daylighting through interior windows
    //                      FW, Apr 2004: add light well efficiency that multiplies glazing transmittance
    //                      FW, Apr 2004: add diffusing glazing
    //                      RJH, Jul 2004: add error handling for warnings/errors returned from DElight
    //                      LKL, Oct 2004: Separate "map" and "ref" point calculations -- move some input routines to
    //                                     separate routines.

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates daylighting factors for later use in the time-step loop.

    // METHODOLOGY EMPLOYED:

    // For each combination of exterior window and reference point in a zone,
    // calculates daylighting factors (interior illuminance / exterior illuminance)
    // and glare factors for clear and overcast skies and for windows with and
    // without shading devices. These factors are calculated for each hourly
    // sun position for design days and for selected days throughout the year.

    // If a target zone has one or more interior windows, also calculates daylighting
    // factors for the target zone that are associated with exterior windows in adjacent
    // zones that share interior windows with the target zone.

    // The daylight illuminance at a reference point from a window is determined
    // by dividing the window into rectangular elements and calculating the illuminance
    // reaching the reference point directly from each element. The illumination
    // from an element can come from the sky or ground if the window is unshaded, or from
    // a shading device illuminated by solar radiation. Also considered are the
    // illuminance contribution from interreflection among the zone's interior surfaces
    // and sunlight striking the reference point.

    // In calculating sky-related interior illuminance and luminance quantities,
    // the sky luminance for the different sky types are determined from distributions
    // in which the zenith luminance is normalized to 1.0 cd/m2. Similarly, sun-related
    // illuminance and luminance quantities are based on beam normal solar illuminance
    // normalized to 1.0 lux.
    // The daylight and glare factors calculated in this subroutine are used in DayltgInteriorIllum
    // to get the daylight illuminance and glare at each time step.
    // Based on this information and user-input lighting setpoint and type of lighting
    // control system, DayltgElecLightingControl then determines how much the overhead electric lighting
    // can be reduced.

    // REFERENCES:
    // Based on DOE-2.1E subroutine DCOF.

    auto &dl = state.dataDayltg;
    auto &s_surf = state.dataSurface;

    if (dl->CalcDayltghCoefficients_firstTime) {
        GetDaylightingParametersInput(state);
        CheckTDDsAndLightShelvesInDaylitZones(state);
        AssociateWindowShadingControlWithDaylighting(state);
        dl->CalcDayltghCoefficients_firstTime = false;
    } // End of check if firstTime

    // Find the total number of exterior windows associated with all Daylighting:Detailed enclosures.
    // An exterior window is associated with such a enclosure if (1) it is an exterior window in the enclosure, or
    // (2) it is an exterior window in an adjacent enclosure that shares an interior window with the enclosure.
    // Note that exterior windows in category (2) may be counted more than once if an adjacent enclosure
    // is adjacent to more than one daylit enclosure with which the adjacent enclosure shares interior windows.
    // If there are no interior windows in a building, than TotWindowsWithDayl is just the total number of
    // exterior windows in Daylighting:Detailed enclosures. Note that it is possible for a
    // Daylighting:Detailed enclosure to have zero exterior windows of its own, but it may have an interior
    // through which daylight passes from adjacent enclosures with exterior windows.
    if ((int)dl->DaylRefPt.size() == 0) {
        return;
    }
    if (state.dataGlobal->BeginSimFlag) {
        dl->TotWindowsWithDayl = 0;
        for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
            dl->TotWindowsWithDayl += dl->enclDaylight(enclNum).NumOfDayltgExtWins;
        }
    }

    if (dl->TotWindowsWithDayl == 0) {
        return;
    }

    //-----------------------------------------!
    // Detailed daylighting factor calculation !
    //-----------------------------------------!
    if (!state.dataSysVars->DetailedSolarTimestepIntegration && !state.dataGlobal->KickOffSizing && !state.dataGlobal->KickOffSimulation) {
        if (state.dataGlobal->WarmupFlag) {
            DisplayString(state, "Calculating Detailed Daylighting Factors, Start Date=" + state.dataEnvrn->CurMnDy);
        } else {
            DisplayString(state, "Updating Detailed Daylighting Factors, Start Date=" + state.dataEnvrn->CurMnDy);
        }
    }

    if (state.dataGlobal->BeginSimFlag) {

        // Find minimum solid angle subtended by an interior window in Daylighting:Detailed zones.
        // Used in calculating daylighting through interior windows.
        CalcMinIntWinSolidAngs(state);

        // Warning if detailed daylighting has been requested for a zone with no associated exterior windows.
        for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
            auto const &thisEnclDaylight = dl->enclDaylight(enclNum);
            if (thisEnclDaylight.NumOfDayltgExtWins == 0 && thisEnclDaylight.TotalExtWindows == 0) {
                for (int daylightCtrlNum : thisEnclDaylight.daylightControlIndexes) {
                    if (dl->daylightControl(daylightCtrlNum).TotalDaylRefPoints > 0) {
                        ShowWarningError(
                            state,
                            format("Detailed daylighting will not be done for Daylighting:Controls={}", dl->daylightControl(daylightCtrlNum).Name));
                        ShowContinueError(state, "because it has no associated exterior windows.");
                    }
                }
            }

            // Find area and reflectance quantities used in calculating inter-reflected illuminance.
            // TH 9/10/2009. Need to calculate for zones without daylighting controls (TotalDaylRefPoints = 0)
            // but with adjacent zones having daylighting controls.
            if ((thisEnclDaylight.NumOfDayltgExtWins > 0) || thisEnclDaylight.adjEnclHasDayltgCtrl) {
                DayltgAveInteriorReflectance(state, enclNum);
            }
        }
    }

    int numTDD = (int)state.dataDaylightingDevicesData->TDDPipe.size();
    // Zero daylighting factor arrays
    if (numTDD > 0) {
        int iHrBeg = state.dataSysVars->DetailedSolarTimestepIntegration ? state.dataGlobal->HourOfDay : 1;
        int iHrEnd = state.dataSysVars->DetailedSolarTimestepIntegration ? state.dataGlobal->HourOfDay : Constant::iHoursInDay;
        for (int iHr = iHrBeg; iHr <= iHrEnd; ++iHr) {
            for (int iTDD = 1; iTDD <= numTDD; ++iTDD) {
                dl->TDDTransVisBeam(iHr, iTDD) = 0.0;
                dl->TDDFluxInc(iHr, iTDD) = Illums();
                dl->TDDFluxTrans(iHr, iTDD) = Illums();
            }
        }
    }

    if (!state.dataSysVars->DetailedSolarTimestepIntegration) {
        if (state.dataGlobal->BeginDayFlag) {
            // Calculate hourly sun angles, clear sky zenith luminance, and exterior horizontal illuminance
            dl->sunAngles = SunAngles();
            dl->sunAnglesHr = {SunAngles()};
            dl->horIllum = {Illums()};
            for (int IHR = 1; IHR <= Constant::iHoursInDay; ++IHR) {
                auto const &surfSunCosHr = s_surf->SurfSunCosHourly(IHR);
                if (surfSunCosHr.z < DataEnvironment::SunIsUpValue) {
                    continue; // Skip if sun is below horizon //Autodesk SurfSunCosHourly was uninitialized here
                }

                Real64 phi = Constant::PiOvr2 - std::acos(surfSunCosHr.z);
                Real64 theta = std::atan2(surfSunCosHr.y, surfSunCosHr.x);
                dl->sunAngles = dl->sunAnglesHr[IHR] = {phi, std::sin(phi), std::cos(phi), theta};

                DayltgExtHorizIllum(state, dl->horIllum[IHR]);
            }
        }
    } else { // timestep integrated calculations
        dl->sunAngles = dl->sunAnglesHr[state.dataGlobal->HourOfDay] = {SunAngles()};
        dl->horIllum[state.dataGlobal->HourOfDay] = Illums();
        auto const &surfSunCosHr = s_surf->SurfSunCosHourly(state.dataGlobal->HourOfDay);
        if (!(surfSunCosHr.z < DataEnvironment::SunIsUpValue)) { // Skip if sun is below horizon
            Real64 phi = Constant::PiOvr2 - std::acos(surfSunCosHr.z);
            Real64 theta = std::atan2(surfSunCosHr.y, surfSunCosHr.x);
            dl->sunAngles = dl->sunAnglesHr[state.dataGlobal->HourOfDay] = {phi, std::sin(phi), std::cos(phi), theta};
            DayltgExtHorizIllum(state, dl->horIllum[state.dataGlobal->HourOfDay]);
        }
    }

    CalcDayltgCoeffsRefMapPoints(state);

    if (dl->doSkyReporting && (!state.dataGlobal->KickOffSizing && !state.dataGlobal->KickOffSimulation) &&
        (dl->FirstTimeDaylFacCalc && dl->TotWindowsWithDayl > 0 &&
         (!state.dataSysVars->DetailedSolarTimestepIntegration || state.dataGlobal->HourOfDay == 12))) {
        // Write the bare-window four sky daylight factors at noon time to the eio file; this is done only
        // for first time that daylight factors are calculated and so is insensitive to possible variation
        // due to change in ground reflectance from month to month, or change in storm window status.
        static constexpr std::string_view Format_700("! <Sky Daylight Factors>, Sky Type, MonthAndDay, Daylighting Control Name, Enclosure Name, "
                                                     "Window Name, Reference Point, Daylight Factor\n");
        print(state.files.eio, Format_700);
        for (int controlNum = 1; controlNum <= (int)dl->daylightControl.size(); ++controlNum) {
            auto &thisDayltgCtrl = dl->daylightControl(controlNum);
            int enclNum = thisDayltgCtrl.enclIndex;
            auto &thisEnclDaylight = dl->enclDaylight(enclNum);

            if (thisEnclDaylight.NumOfDayltgExtWins == 0 || !thisEnclDaylight.hasSplitFluxDaylighting) {
                continue;
            }
            for (int windowCounter = 1; windowCounter <= thisEnclDaylight.NumOfDayltgExtWins; ++windowCounter) {
                int windowSurfNum = thisEnclDaylight.DayltgExtWinSurfNums(windowCounter);
                // For this report, do not include ext wins in zone adjacent to ZoneNum since the inter-reflected
                // component will not be calculated for these windows until the time-step loop.
                if (s_surf->Surface(windowSurfNum).SolarEnclIndex != enclNum) {
                    continue;
                }
                // Output for each reference point, for each sky. Group by sky type first

                static constexpr std::array<std::string_view, (int)SkyType::Num> skyTypeStrings = {
                    "Clear Sky", "Clear Turbid Sky", "Intermediate Sky", "Overcast Sky"};

                for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
                    for (int refPtNum = 1; refPtNum <= thisDayltgCtrl.TotalDaylRefPoints; ++refPtNum) {
                        Real64 DaylFac = thisDayltgCtrl.daylFac[12](windowCounter, refPtNum)[iWinCover_Bare][iLum_Illum].sky[iSky];
                        print(state.files.eio,
                              " Sky Daylight Factors,{},{},{},{},{},{},{:.4R}\n",
                              skyTypeStrings[iSky],
                              state.dataEnvrn->CurMnDy,
                              thisDayltgCtrl.Name,
                              state.dataViewFactor->EnclSolInfo(thisDayltgCtrl.enclIndex).Name,
                              s_surf->Surface(windowSurfNum).Name,
                              dl->DaylRefPt(thisDayltgCtrl.refPts(refPtNum).num).Name,
                              DaylFac);
                    } // for (refPtNum)
                } // for (iSky)
            } // for (windowCounter)
        } // for (controlNum)
        dl->FirstTimeDaylFacCalc = false;
        dl->doSkyReporting = false;
    } // if (detailedIntegration etc.)

    // Skip if no daylight windows
    if (dl->TotWindowsWithDayl == 0) {
        return;
    }

    // Skip if no request of reporting
    if ((!dl->DFSReportSizingDays) && (!dl->DFSReportAllShadowCalculationDays)) {
        return;
    }

    // Skip duplicate calls
    if (state.dataGlobal->KickOffSizing) {
        return;
    }
    if (state.dataGlobal->DoingSizing) {
        return;
    }
    if (state.dataGlobal->KickOffSimulation) {
        return;
    }

    if (dl->DFSReportSizingDays) {
        if (state.dataGlobal->DoWeathSim && state.dataGlobal->DoDesDaySim) {
            if (state.dataGlobal->KindOfSim == Constant::KindOfSim::RunPeriodWeather) {
                return;
            }
        }
    }

    if (dl->DFSReportAllShadowCalculationDays) {
        if (state.dataGlobal->KindOfSim != Constant::KindOfSim::RunPeriodWeather) {
            return;
        }
    }

    // open a new file eplusout.dfs for saving the daylight factors
    if (dl->CreateDFSReportFile) {
        InputOutputFile &dfs = state.files.dfs.ensure_open(state, "CalcDayltgCoefficients", state.files.outputControl.dfs);
        print(dfs, "{}\n", "This file contains daylight factors for all exterior windows of daylight enclosures.");
        print(dfs, "{}\n", "MonthAndDay,Enclosure Name,Zone Name,Window Name,Window State");
        print(dfs,
              "{}\n",
              "Hour,Reference Point,Daylight Factor for Clear Sky,Daylight Factor for Clear Turbid Sky,"
              "Daylight Factor for Intermediate Sky,Daylight Factor for Overcast Sky");
        dl->CreateDFSReportFile = false;
    }

    for (int controlNum = 1; controlNum <= (int)dl->daylightControl.size(); ++controlNum) {
        auto &thisDayltgCtrl = dl->daylightControl(controlNum);
        int enclNum = thisDayltgCtrl.enclIndex;
        auto &thisEnclDaylight = dl->enclDaylight(enclNum);
        if (thisEnclDaylight.NumOfDayltgExtWins == 0) {
            continue;
        }

        for (int windowCounter = 1; windowCounter <= thisEnclDaylight.NumOfDayltgExtWins; ++windowCounter) {
            int windowSurfNum = thisEnclDaylight.DayltgExtWinSurfNums(windowCounter);
            auto &surf = s_surf->Surface(windowSurfNum);
            // For this report, do not include ext wins in zone/enclosure adjacent to ZoneNum since the inter-reflected
            // component will not be calculated for these windows until the time-step loop.
            if (surf.SolarEnclIndex == enclNum) {

                int numWinCover = surf.HasShadeControl ? (int)WinCover::Num : 1;

                // loop over each slat angle
                for (int iWinCover = 0; iWinCover < numWinCover; ++iWinCover) {
                    if (iWinCover == iWinCover_Bare) {
                        // base window without shades, screens, or blinds
                        print(state.files.dfs,
                              "{},{},{},{},Base Window\n",
                              state.dataEnvrn->CurMnDy,
                              state.dataViewFactor->EnclSolInfo(enclNum).Name,
                              state.dataHeatBal->Zone(thisDayltgCtrl.zoneIndex).Name,
                              s_surf->Surface(windowSurfNum).Name);
                    } else if (iWinCover == iWinCover_Shaded) {
                        // window shade or blind with fixed slat angle
                        print(state.files.dfs,
                              "{},{},{},{},Blind or Slat Applied\n",
                              state.dataEnvrn->CurMnDy,
                              state.dataViewFactor->EnclSolInfo(enclNum).Name,
                              state.dataHeatBal->Zone(thisDayltgCtrl.zoneIndex).Name,
                              s_surf->Surface(windowSurfNum).Name);
                    }

                    for (int IHR = 1; IHR <= Constant::iHoursInDay; ++IHR) {
                        // For each Daylight Reference Point
                        auto &daylFacHr = thisDayltgCtrl.daylFac[IHR];
                        for (int refPtNum = 1; refPtNum <= thisDayltgCtrl.TotalDaylRefPoints; ++refPtNum) {
                            auto &illums = daylFacHr(windowCounter, refPtNum)[iWinCover][iLum_Illum];

                            // write daylight factors - 4 sky types for each daylight ref point
                            print(state.files.dfs,
                                  "{},{},{:.5R},{:.5R},{:.5R},{:.5R}\n",
                                  IHR,
                                  dl->DaylRefPt(thisDayltgCtrl.refPts(refPtNum).num).Name,
                                  illums.sky[(int)SkyType::Clear],
                                  illums.sky[(int)SkyType::ClearTurbid],
                                  illums.sky[(int)SkyType::Intermediate],
                                  illums.sky[(int)SkyType::Overcast]);

                        } // for (refPtNum) Reference Point
                    } // for (IHR) hour
                } // for (ISlatAngle) slat angle
            } // if (SolarEnclIndex == enclNum)
        } // for (windowCounter) exterior windows in enclosure
    } // for (controlNum) daylighting control
} // CalcDayltgCoefficients()

void CalcDayltgCoeffsRefMapPoints(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   October 2004
    //       MODIFIED       May 2006 (RR): added exterior window screens
    //                      April 2012 (LKL); change to allow multiple maps per zone

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine does the daylighting coefficient calculation for the
    // daylighting and illuminance map reference points.
    auto &dl = state.dataDayltg;
    auto const &s_surf = state.dataSurface;

    if (dl->VeryFirstTime) {
        // make sure all necessary surfaces match to pipes
        bool ErrorsFound = false;
        for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
            for (int loopwin = 1; loopwin <= dl->enclDaylight(enclNum).NumOfDayltgExtWins; ++loopwin) {
                int IWin = dl->enclDaylight(enclNum).DayltgExtWinSurfNums(loopwin);
                if (s_surf->Surface(IWin).OriginalClass != SurfaceClass::TDD_Diffuser) {
                    continue;
                }
                // Look up the TDD:DOME object
                int PipeNum = s_surf->SurfWinTDDPipeNum(IWin);
                if (PipeNum == 0) {
                    ShowSevereError(
                        state,
                        format("GetTDDInput: Surface={}, TDD:Dome object does not reference a valid Diffuser object.", s_surf->Surface(IWin).Name));
                    ShowContinueError(state, "...needs DaylightingDevice:Tubular of same name as Surface.");
                    ErrorsFound = true;
                }
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, "Not all TubularDaylightDome objects have corresponding DaylightingDevice:Tubular objects. Program terminates.");
        }
        dl->VeryFirstTime = false;
    }

    // Calc for daylighting reference points for daylighting controls that use SplitFlux method
    for (int daylightCtrlNum = 1; daylightCtrlNum <= (int)dl->daylightControl.size(); ++daylightCtrlNum) {
        if (dl->daylightControl(daylightCtrlNum).DaylightMethod != DaylightingMethod::SplitFlux) {
            continue;
        }
        // Skip enclosures with no exterior windows or in adjacent enclosure(s) with which an interior window is shared
        if (dl->enclDaylight(dl->daylightControl(daylightCtrlNum).enclIndex).NumOfDayltgExtWins == 0) {
            continue;
        }
        CalcDayltgCoeffsRefPoints(state, daylightCtrlNum);
    }
    if (!state.dataGlobal->DoingSizing && !state.dataGlobal->KickOffSimulation) {
        // Calc for illuminance maps
        if ((int)dl->illumMaps.size() > 0) {
            for (int MapNum = 1; MapNum <= (int)dl->illumMaps.size(); ++MapNum) {
                int mapZoneNum = dl->illumMaps(MapNum).zoneIndex;
                std::string name = format("Zone={}", state.dataHeatBal->Zone(mapZoneNum).Name);
                int mapSpaceNum = dl->illumMaps(MapNum).spaceIndex;
                if (mapSpaceNum > 0) {
                    name = format("Space={}", state.dataHeatBal->space(mapSpaceNum).Name);
                }
                if (state.dataGlobal->WarmupFlag) {
                    DisplayString(state, format("Calculating Daylighting Coefficients (Map Points), {}", name));
                } else {
                    DisplayString(state, format("Updating Daylighting Coefficients (Map Points), {}", name));
                }
                CalcDayltgCoeffsMapPoints(state, MapNum);
            }
        }
    }
} // CalcDayltgCoeffsRefMapPoints()

void CalcDayltgCoeffsRefPoints(EnergyPlusData &state, int const daylightCtrlNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   April 2012
    //       MODIFIED       November 2012 (B. Griffith), refactor for detailed timestep integration and remove duplicate code

    // PURPOSE OF THIS SUBROUTINE:
    // Provides calculations for Daylighting Coefficients for daylighting reference points
    auto &dl = state.dataDayltg;
    auto const &s_surf = state.dataSurface;

    //  glare calculation (radians)
    int IConst;            // Construction counter
    int ICtrl;             // Window control counter
    int IWin;              // Window counter
    int IWin2;             // Secondary window counter (for TDD:DOME object, if exists)
    int InShelfSurf;       // Inside daylighting shelf surface number
    WinShadingType ShType; // Window shading type
    int BlNum;             // Window Blind Number
    int LSHCAL;            // Interior shade calculation flag: 0=not yet
    //  calculated, 1=already calculated
    int NWX;     // Number of window elements in x direction for dayltg calc
    int NWY;     // Number of window elements in y direction for dayltg calc
    int NWYlim;  // For triangle, largest NWY for a given IX
    Real64 COSB; // Cosine of angle between window outward normal and ray from
    //  reference point to window element
    Real64 PHRAY;  // Altitude of ray from reference point to window element (radians)
    Real64 THRAY;  // Azimuth of ray from reference point to window element (radians)
    Real64 DOMEGA; // Solid angle subtended by window element wrt reference point (steradians)
    Real64 TVISB;  // Visible transmittance of window for COSB angle of incidence (times light well
    //   efficiency, if appropriate)
    int ISunPos; // Sun position counter; used to avoid calculating various
    //  quantities that do not depend on sun position.
    Real64 ObTrans; // Product of solar transmittances of exterior obstructions hit by ray
    // from reference point through a window element
    bool is_Rectangle;         // True if window is rectangular
    bool is_Triangle;          // True if window is triangular
    Real64 DWX;                // Horizontal dimension of window element (m)
    Real64 DWY;                // Vertical dimension of window element (m)
    Real64 DAXY;               // Area of window element
    Real64 SkyObstructionMult; // Ratio of obstructed to unobstructed sky diffuse at a ground point
    ExtWinType extWinType;     // Exterior window type (InZoneExtWin, AdjZoneExtWin, NotInOrAdjZoneExtWin)
    int BRef;
    int ILB;
    bool hitIntObs;        // True iff interior obstruction hit
    bool hitExtObs;        // True iff ray from ref pt to ext win hits an exterior obstruction
    Real64 TVISIntWin;     // Visible transmittance of int win at COSBIntWin for light from ext win
    Real64 TVISIntWinDisk; // Visible transmittance of int win at COSBIntWin for sun

    Vector3<Real64> W2;
    Vector3<Real64> W3 = {0.0, 0.0, 0.0};
    Vector3<Real64> W21;
    Vector3<Real64> W23;
    Vector3<Real64> RREF2;
    Vector3<Real64> RWIN;
    Vector3<Real64> RWIN2;
    Vector3<Real64> Ray;
    Vector3<Real64> WNORM2;
    Vector3<Real64> VIEWVC;
    Vector3<Real64> U2;
    Vector3<Real64> U21;
    Vector3<Real64> U23;
    Vector3<Real64> VIEWVC2;

    int WinEl; // Current window element

    if (dl->refFirstTime && (dl->maxControlRefPoints > 0)) {
        dl->RefErrIndex.allocate(dl->maxControlRefPoints, s_surf->TotSurfaces);
        dl->RefErrIndex = 0;
        dl->refFirstTime = false;
    }

    auto &thisDayltgCtrl = dl->daylightControl(daylightCtrlNum);
    auto const &thisEnclDaylight = dl->enclDaylight(thisDayltgCtrl.enclIndex);
    int zoneNum = thisDayltgCtrl.zoneIndex;
    // Azimuth of view vector in absolute coord sys
    Real64 AZVIEW = (thisDayltgCtrl.ViewAzimuthForGlare + state.dataHeatBal->Zone(zoneNum).RelNorth + state.dataHeatBal->BuildingAzimuth +
                     state.dataHeatBal->BuildingRotationAppendixG) *
                    Constant::DegToRad;
    // View vector components in absolute coord sys
    VIEWVC = {std::sin(AZVIEW), std::cos(AZVIEW), 0.0};

    for (auto &refPt : thisDayltgCtrl.refPts) {
        refPt.lums[iLum_Illum] = 0.0; // Daylight illuminance at reference points (lux)
        refPt.glareIndex = 0.0;       // Glare index at reference points
        for (auto &extWin : refPt.extWins) {
            extWin.solidAng = extWin.solidAngWtd = 0.0;
            extWin.lums[iLum_Illum] = extWin.lums[iLum_Back] = extWin.lums[iLum_Source] = {0.0, 0.0};
        }
    }

    int iHrBeg = state.dataSysVars->DetailedSolarTimestepIntegration ? state.dataGlobal->HourOfDay : 1;
    int iHrEnd = state.dataSysVars->DetailedSolarTimestepIntegration ? state.dataGlobal->HourOfDay : Constant::iHoursInDay;
    int numExtWins = thisEnclDaylight.NumOfDayltgExtWins;
    int numRefPts = thisDayltgCtrl.TotalDaylRefPoints;

    for (int iHr = iHrBeg; iHr <= iHrEnd; ++iHr) {
        auto &daylFacHr = thisDayltgCtrl.daylFac[iHr];
        for (int iWin = 1; iWin <= numExtWins; ++iWin) {
            for (int iRefPt = 1; iRefPt <= numRefPts; ++iRefPt) {
                for (int iWinCover = 0; iWinCover < (int)WinCover::Num; ++iWinCover) {
                    auto &daylFac = daylFacHr(iWin, iRefPt)[iWinCover];
                    daylFac[iLum_Illum] = Illums();
                    daylFac[iLum_Source] = Illums();
                    daylFac[iLum_Back] = Illums();
                } // for (iSlatAng)
            } // for (iRefPt)
        } // for (iWin)
    } // for (iHr)

    BRef = 0;

    for (int IL = 1; IL <= thisDayltgCtrl.TotalDaylRefPoints; ++IL) {
        auto const &refPt = thisDayltgCtrl.refPts(IL);
        // Reference point in absolute coordinate system
        Vector3<Real64> RREF = refPt.absCoords;

        //           -------------
        // ---------- WINDOW LOOP ----------
        //           -------------
        for (int loopwin = 1; loopwin <= thisEnclDaylight.NumOfDayltgExtWins; ++loopwin) {

            FigureDayltgCoeffsAtPointsSetupForWindow(state,
                                                     daylightCtrlNum,
                                                     IL,
                                                     loopwin,
                                                     CalledFor::RefPoint,
                                                     RREF,
                                                     VIEWVC,
                                                     IWin,
                                                     IWin2,
                                                     NWX,
                                                     NWY,
                                                     W2,
                                                     W3,
                                                     W21,
                                                     W23,
                                                     LSHCAL,
                                                     InShelfSurf,
                                                     ICtrl,
                                                     ShType,
                                                     BlNum,
                                                     WNORM2,
                                                     extWinType,
                                                     IConst,
                                                     RREF2,
                                                     DWX,
                                                     DWY,
                                                     DAXY,
                                                     U2,
                                                     U23,
                                                     U21,
                                                     VIEWVC2,
                                                     is_Rectangle,
                                                     is_Triangle);
            //           ---------------------
            // ---------- WINDOW ELEMENT LOOP ----------
            //           ---------------------

            WinEl = 0;

            for (int IX = 1; IX <= NWX; ++IX) {
                if (is_Rectangle) {
                    NWYlim = NWY;
                } else if (is_Triangle) {
                    NWYlim = NWY - IX + 1;
                }

                for (int IY = 1; IY <= NWYlim; ++IY) {

                    ++WinEl;

                    FigureDayltgCoeffsAtPointsForWindowElements(state,
                                                                daylightCtrlNum,
                                                                IL,
                                                                loopwin,
                                                                CalledFor::RefPoint,
                                                                WinEl,
                                                                IWin,
                                                                IWin2,
                                                                IX,
                                                                IY,
                                                                SkyObstructionMult,
                                                                W2,
                                                                W21,
                                                                W23,
                                                                RREF,
                                                                NWYlim,
                                                                VIEWVC2,
                                                                DWX,
                                                                DWY,
                                                                DAXY,
                                                                U2,
                                                                U23,
                                                                U21,
                                                                RWIN,
                                                                RWIN2,
                                                                Ray,
                                                                PHRAY,
                                                                LSHCAL,
                                                                COSB,
                                                                ObTrans,
                                                                TVISB,
                                                                DOMEGA,
                                                                THRAY,
                                                                hitIntObs,
                                                                hitExtObs,
                                                                WNORM2,
                                                                extWinType,
                                                                IConst,
                                                                RREF2,
                                                                is_Triangle,
                                                                TVISIntWin,
                                                                TVISIntWinDisk);

                    //           -------------------
                    // ---------- SUN POSITION LOOP ----------
                    //           -------------------

                    // Sun position counter. Used to avoid calculating various quantities
                    // that do not depend on sun position.

                    if (!state.dataSysVars->DetailedSolarTimestepIntegration) {
                        ISunPos = 0;
                        for (int IHR = 1; IHR <= Constant::iHoursInDay; ++IHR) {

                            FigureDayltgCoeffsAtPointsForSunPosition(state,
                                                                     daylightCtrlNum,
                                                                     IL,
                                                                     IX,
                                                                     NWX,
                                                                     IY,
                                                                     NWYlim,
                                                                     WinEl,
                                                                     IWin,
                                                                     IWin2,
                                                                     IHR,
                                                                     ISunPos,
                                                                     SkyObstructionMult,
                                                                     RWIN2,
                                                                     Ray,
                                                                     PHRAY,
                                                                     LSHCAL,
                                                                     InShelfSurf,
                                                                     COSB,
                                                                     ObTrans,
                                                                     TVISB,
                                                                     DOMEGA,
                                                                     ICtrl,
                                                                     ShType,
                                                                     BlNum,
                                                                     THRAY,
                                                                     WNORM2,
                                                                     extWinType,
                                                                     IConst,
                                                                     AZVIEW,
                                                                     RREF2,
                                                                     hitIntObs,
                                                                     hitExtObs,
                                                                     CalledFor::RefPoint,
                                                                     TVISIntWin,
                                                                     TVISIntWinDisk);

                        } // End of hourly sun position loop, IHR
                    } else { // timestep integrated
                        if (state.dataEnvrn->SunIsUp && !dl->MySunIsUpFlag) {
                            ISunPos = 0;
                            dl->MySunIsUpFlag = true;
                        } else if (state.dataEnvrn->SunIsUp && dl->MySunIsUpFlag) {
                            ISunPos = 1;
                        } else if (!state.dataEnvrn->SunIsUp && dl->MySunIsUpFlag) {
                            dl->MySunIsUpFlag = false;
                            ISunPos = -1;
                        } else if (!state.dataEnvrn->SunIsUp && !dl->MySunIsUpFlag) {
                            ISunPos = -1;
                        }

                        FigureDayltgCoeffsAtPointsForSunPosition(state,
                                                                 daylightCtrlNum,
                                                                 IL,
                                                                 IX,
                                                                 NWX,
                                                                 IY,
                                                                 NWYlim,
                                                                 WinEl,
                                                                 IWin,
                                                                 IWin2,
                                                                 state.dataGlobal->HourOfDay,
                                                                 ISunPos,
                                                                 SkyObstructionMult,
                                                                 RWIN2,
                                                                 Ray,
                                                                 PHRAY,
                                                                 LSHCAL,
                                                                 InShelfSurf,
                                                                 COSB,
                                                                 ObTrans,
                                                                 TVISB,
                                                                 DOMEGA,
                                                                 ICtrl,
                                                                 ShType,
                                                                 BlNum,
                                                                 THRAY,
                                                                 WNORM2,
                                                                 extWinType,
                                                                 IConst,
                                                                 AZVIEW,
                                                                 RREF2,
                                                                 hitIntObs,
                                                                 hitExtObs,
                                                                 CalledFor::RefPoint,
                                                                 TVISIntWin,
                                                                 TVISIntWinDisk);
                    }

                } // End of window Y-element loop, IY
            } // End of window X-element loop, IX

            // Loop again over hourly sun positions and calculate daylight factors by adding
            // direct and inter-reflected illum components, then dividing by exterior horiz illum.
            // Also calculate corresponding glare factors.

            ILB = BRef + IL;

            if (!state.dataSysVars->DetailedSolarTimestepIntegration) {
                ISunPos = 0;
                for (int IHR = 1; IHR <= Constant::iHoursInDay; ++IHR) {
                    FigureRefPointDayltgFactorsToAddIllums(state, daylightCtrlNum, ILB, IHR, ISunPos, IWin, loopwin, NWX, NWY, ICtrl);

                } // End of sun position loop, IHR
            } else {
                if (state.dataEnvrn->SunIsUp && !dl->MySunIsUpFlag) {
                    ISunPos = 0;
                    dl->MySunIsUpFlag = true;
                } else if (state.dataEnvrn->SunIsUp && dl->MySunIsUpFlag) {
                    ISunPos = 1;
                } else if (!state.dataEnvrn->SunIsUp && dl->MySunIsUpFlag) {
                    dl->MySunIsUpFlag = false;
                    ISunPos = -1;
                } else if (!state.dataEnvrn->SunIsUp && !dl->MySunIsUpFlag) {
                    ISunPos = -1;
                }
                FigureRefPointDayltgFactorsToAddIllums(
                    state, daylightCtrlNum, ILB, state.dataGlobal->HourOfDay, ISunPos, IWin, loopwin, NWX, NWY, ICtrl);
            }
        } // End of window loop, loopwin - IWin

    } // End of reference point loop, IL
}

void CalcDayltgCoeffsMapPoints(EnergyPlusData &state, int const mapNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   April 2012
    //       MODIFIED      November 2012 (B. Griffith), refactor for detailed timestep integration and remove duplicate code

    // PURPOSE OF THIS SUBROUTINE:
    // Provides calculations for Daylighting Coefficients for map illuminance points

    // METHODOLOGY EMPLOYED:
    // Was previously part of CalcDayltgCoeffsRefMapPoints -- broken out to all multiple
    // maps per zone
    auto &dl = state.dataDayltg;
    auto const &s_surf = state.dataSurface;

    //  In the following four variables, I=1 for clear sky, 2 for overcast.
    int numRefPts; // Number of daylighting reference points in a zone
    //  glare calculation (radians)
    int IConst;            // Construction counter
    int ICtrl;             // Window control counter
    int IWin;              // Window counter
    int IWin2;             // Secondary window counter (for TDD:DOME object, if exists)
    int InShelfSurf;       // Inside daylighting shelf surface number
    WinShadingType ShType; // Window shading type
    int BlNum;             // Window Blind Number
    int LSHCAL;            // Interior shade calculation flag: 0=not yet
    //  calculated, 1=already calculated
    int NWX;     // Number of window elements in x direction for dayltg calc
    int NWY;     // Number of window elements in y direction for dayltg calc
    int NWYlim;  // For triangle, largest NWY for a given IX
    Real64 DWX;  // Horizontal dimension of window element (m)
    Real64 DWY;  // Vertical dimension of window element (m)
    Real64 COSB; // Cosine of angle between window outward normal and ray from
    //  reference point to window element
    Real64 PHRAY;  // Altitude of ray from reference point to window element (radians)
    Real64 THRAY;  // Azimuth of ray from reference point to window element (radians)
    Real64 DOMEGA; // Solid angle subtended by window element wrt reference point (steradians)
    Real64 TVISB;  // Visible transmittance of window for COSB angle of incidence (times light well
    //   efficiency, if appropriate)
    int ISunPos; // Sun position counter; used to avoid calculating various
    //  quantities that do not depend on sun position.
    Real64 ObTrans; // Product of solar transmittances of exterior obstructions hit by ray
    // from reference point through a window element
    bool is_Rectangle;         // True if window is rectangular
    bool is_Triangle;          // True if window is triangular
    Real64 DAXY;               // Area of window element
    Real64 SkyObstructionMult; // Ratio of obstructed to unobstructed sky diffuse at a ground point
    ExtWinType extWinType;     // Exterior window type (InZoneExtWin, AdjZoneExtWin, NotInOrAdjZoneExtWin)
    int ILB;
    bool hitIntObs;        // True iff interior obstruction hit
    bool hitExtObs;        // True iff ray from ref pt to ext win hits an exterior obstruction
    Real64 TVISIntWin;     // Visible transmittance of int win at COSBIntWin for light from ext win
    Real64 TVISIntWinDisk; // Visible transmittance of int win at COSBIntWin for sun
    int WinEl;             // window elements counter

    Vector3<Real64> W2;
    Vector3<Real64> W3;
    Vector3<Real64> W21;
    Vector3<Real64> W23;
    Vector3<Real64> RREF2;
    Vector3<Real64> RWIN;
    Vector3<Real64> RWIN2;
    Vector3<Real64> Ray;
    Vector3<Real64> WNORM2;
    Vector3<Real64> VIEWVC;
    Vector3<Real64> U2;
    Vector3<Real64> U21;
    Vector3<Real64> U23;
    Vector3<Real64> VIEWVC2;

    if (dl->mapFirstTime && (int)dl->illumMaps.size() > 0) {
        int IL = -999;
        for (int MapNum = 1; MapNum <= (int)dl->illumMaps.size(); ++MapNum) {
            IL = max(IL, dl->illumMaps(MapNum).TotalMapRefPoints);
        }
        dl->MapErrIndex.dimension(IL, s_surf->TotSurfaces, 0);
        dl->mapFirstTime = false;
    }

    auto &illumMap = dl->illumMaps(mapNum);
    int enclNum = illumMap.enclIndex;
    auto const &thisEnclDaylight = dl->enclDaylight(enclNum);

    // Azimuth of view vector in absolute coord sys - set to zero here, because glare isn't calculated for map points
    // but these are arguments to some of the functions that are shared with regular reference points, so initalize here.
    Real64 AZVIEW = 0.0;
    // View vector components in absolute coord sys
    VIEWVC = {0.0, 0.0, 0.0};

    numRefPts = illumMap.TotalMapRefPoints;
    int numExtWins = thisEnclDaylight.NumOfDayltgExtWins;

    for (auto &refPt : illumMap.refPts) {
        refPt.lums[iLum_Illum] = 0.0; // Daylight illuminance at reference points (lux)
        for (int iExtWin = 1; iExtWin <= numExtWins; ++iExtWin) {
            refPt.winLums(iExtWin) = {0.0, 0.0};
        }
    }

    int iHrBeg = state.dataSysVars->DetailedSolarTimestepIntegration ? state.dataGlobal->HourOfDay : 1;
    int iHrEnd = state.dataSysVars->DetailedSolarTimestepIntegration ? state.dataGlobal->HourOfDay : Constant::iHoursInDay;

    for (int iHr = iHrBeg; iHr <= iHrEnd; ++iHr) {
        auto &daylFacHr = illumMap.daylFac[iHr];
        for (int iWin = 1; iWin <= numExtWins; ++iWin) {
            for (int iRefPt = 1; iRefPt <= numRefPts; ++iRefPt) {
                for (int iWinCover = 0; iWinCover < (int)WinCover::Num; ++iWinCover) {
                    daylFacHr(iWin, iRefPt)[iWinCover] = Illums();
                }
            }
        }
    }

    for (int IL = 1; IL <= numRefPts; ++IL) {
        auto const &refPt = illumMap.refPts(IL);
        Vector3<Real64> RREF = refPt.absCoords;

        //           -------------
        // ---------- WINDOW LOOP ----------
        //           -------------

        for (int loopwin = 1; loopwin <= numExtWins; ++loopwin) {

            // daylightingCtrlNum parameter is unused for map points
            FigureDayltgCoeffsAtPointsSetupForWindow(state,
                                                     0,
                                                     IL,
                                                     loopwin,
                                                     CalledFor::MapPoint,
                                                     RREF,
                                                     VIEWVC,
                                                     IWin,
                                                     IWin2,
                                                     NWX,
                                                     NWY,
                                                     W2,
                                                     W3,
                                                     W21,
                                                     W23,
                                                     LSHCAL,
                                                     InShelfSurf,
                                                     ICtrl,
                                                     ShType,
                                                     BlNum,
                                                     WNORM2,
                                                     extWinType,
                                                     IConst,
                                                     RREF2,
                                                     DWX,
                                                     DWY,
                                                     DAXY,
                                                     U2,
                                                     U23,
                                                     U21,
                                                     VIEWVC2,
                                                     is_Rectangle,
                                                     is_Triangle,
                                                     mapNum);
            //           ---------------------
            // ---------- WINDOW ELEMENT LOOP ----------
            //           ---------------------
            WinEl = 0;

            for (int IX = 1; IX <= NWX; ++IX) {
                if (is_Rectangle) {
                    NWYlim = NWY;
                } else if (is_Triangle) {
                    NWYlim = NWY - IX + 1;
                }

                for (int IY = 1; IY <= NWYlim; ++IY) {

                    ++WinEl;

                    // daylightingCtrlNum parameter is unused for map points
                    FigureDayltgCoeffsAtPointsForWindowElements(state,
                                                                0,
                                                                IL,
                                                                loopwin,
                                                                CalledFor::MapPoint,
                                                                WinEl,
                                                                IWin,
                                                                IWin2,
                                                                IX,
                                                                IY,
                                                                SkyObstructionMult,
                                                                W2,
                                                                W21,
                                                                W23,
                                                                RREF,
                                                                NWYlim,
                                                                VIEWVC2,
                                                                DWX,
                                                                DWY,
                                                                DAXY,
                                                                U2,
                                                                U23,
                                                                U21,
                                                                RWIN,
                                                                RWIN2,
                                                                Ray,
                                                                PHRAY,
                                                                LSHCAL,
                                                                COSB,
                                                                ObTrans,
                                                                TVISB,
                                                                DOMEGA,
                                                                THRAY,
                                                                hitIntObs,
                                                                hitExtObs,
                                                                WNORM2,
                                                                extWinType,
                                                                IConst,
                                                                RREF2,
                                                                is_Triangle,
                                                                TVISIntWin,
                                                                TVISIntWinDisk,
                                                                mapNum);
                    //           -------------------
                    // ---------- SUN POSITION LOOP ----------
                    //           -------------------

                    // Sun position counter. Used to avoid calculating various quantities
                    // that do not depend on sun position.
                    if (!state.dataSysVars->DetailedSolarTimestepIntegration) {
                        ISunPos = 0;
                        for (int IHR = 1; IHR <= Constant::iHoursInDay; ++IHR) {
                            // daylightingCtrlNum parameter is unused for map points
                            FigureDayltgCoeffsAtPointsForSunPosition(state,
                                                                     0,
                                                                     IL,
                                                                     IX,
                                                                     NWX,
                                                                     IY,
                                                                     NWYlim,
                                                                     WinEl,
                                                                     IWin,
                                                                     IWin2,
                                                                     IHR,
                                                                     ISunPos,
                                                                     SkyObstructionMult,
                                                                     RWIN2,
                                                                     Ray,
                                                                     PHRAY,
                                                                     LSHCAL,
                                                                     InShelfSurf,
                                                                     COSB,
                                                                     ObTrans,
                                                                     TVISB,
                                                                     DOMEGA,
                                                                     ICtrl,
                                                                     ShType,
                                                                     BlNum,
                                                                     THRAY,
                                                                     WNORM2,
                                                                     extWinType,
                                                                     IConst,
                                                                     AZVIEW,
                                                                     RREF2,
                                                                     hitIntObs,
                                                                     hitExtObs,
                                                                     CalledFor::MapPoint,
                                                                     TVISIntWin,
                                                                     TVISIntWinDisk,
                                                                     mapNum);
                        } // End of hourly sun position loop, IHR
                    } else {
                        if (state.dataEnvrn->SunIsUp && !dl->CalcDayltgCoeffsMapPointsMySunIsUpFlag) {
                            ISunPos = 0;
                            dl->CalcDayltgCoeffsMapPointsMySunIsUpFlag = true;
                        } else if (state.dataEnvrn->SunIsUp && dl->CalcDayltgCoeffsMapPointsMySunIsUpFlag) {
                            ISunPos = 1;
                        } else if (!state.dataEnvrn->SunIsUp && dl->CalcDayltgCoeffsMapPointsMySunIsUpFlag) {
                            dl->CalcDayltgCoeffsMapPointsMySunIsUpFlag = false;
                            ISunPos = -1;
                        } else if (!state.dataEnvrn->SunIsUp && !dl->CalcDayltgCoeffsMapPointsMySunIsUpFlag) {
                            ISunPos = -1;
                        }
                        // daylightingCtrlNum parameter is unused for map points
                        FigureDayltgCoeffsAtPointsForSunPosition(state,
                                                                 0,
                                                                 IL,
                                                                 IX,
                                                                 NWX,
                                                                 IY,
                                                                 NWYlim,
                                                                 WinEl,
                                                                 IWin,
                                                                 IWin2,
                                                                 state.dataGlobal->HourOfDay,
                                                                 ISunPos,
                                                                 SkyObstructionMult,
                                                                 RWIN2,
                                                                 Ray,
                                                                 PHRAY,
                                                                 LSHCAL,
                                                                 InShelfSurf,
                                                                 COSB,
                                                                 ObTrans,
                                                                 TVISB,
                                                                 DOMEGA,
                                                                 ICtrl,
                                                                 ShType,
                                                                 BlNum,
                                                                 THRAY,
                                                                 WNORM2,
                                                                 extWinType,
                                                                 IConst,
                                                                 AZVIEW,
                                                                 RREF2,
                                                                 hitIntObs,
                                                                 hitExtObs,
                                                                 CalledFor::MapPoint,
                                                                 TVISIntWin,
                                                                 TVISIntWinDisk,
                                                                 mapNum);
                    }
                } // End of window Y-element loop, IY
            } // End of window X-element loop, IX

            if (!state.dataSysVars->DetailedSolarTimestepIntegration) {
                // Loop again over hourly sun positions and calculate daylight factors by adding
                // direct and inter-reflected illum components, then dividing by exterior horiz illum.
                // Also calculate corresponding glare factors.
                ILB = IL;
                for (int IHR = 1; IHR <= Constant::iHoursInDay; ++IHR) {
                    FigureMapPointDayltgFactorsToAddIllums(state, mapNum, ILB, IHR, IWin, loopwin, ICtrl);
                } // End of sun position loop, IHR
            } else {
                ILB = IL;
                FigureMapPointDayltgFactorsToAddIllums(state, mapNum, ILB, state.dataGlobal->HourOfDay, IWin, loopwin, ICtrl);
            }

        } // End of window loop, loopwin - IWin

    } // End of reference point loop, IL
}

void FigureDayltgCoeffsAtPointsSetupForWindow(EnergyPlusData &state,
                                              int const daylightCtrlNum, // zero if called for map points
                                              int const iRefPoint,
                                              int const loopwin,
                                              CalledFor const CalledFrom,    // indicate  which type of routine called this routine
                                              Vector3<Real64> const &RREF,   // Location of a reference point in absolute coordinate system
                                              Vector3<Real64> const &VIEWVC, // View vector in absolute coordinate system
                                              int &IWin,
                                              int &IWin2,
                                              int &NWX,
                                              int &NWY,
                                              Vector3<Real64> &W2,     // Second vertex of window
                                              Vector3<Real64> &W3,     // Third vertex of window
                                              Vector3<Real64> &W21,    // Vector from window vertex 2 to window vertex 1
                                              Vector3<Real64> &W23,    // Vector from window vertex 2 to window vertex 3
                                              int &LSHCAL,             // Interior shade calculation flag:  0=not yet calculated, 1=already calculated
                                              int &InShelfSurf,        // Inside daylighting shelf surface number
                                              int &ICtrl,              // Window control counter
                                              WinShadingType &ShType,  // Window shading type
                                              int &BlNum,              // Window blind number
                                              Vector3<Real64> &WNORM2, // Unit vector normal to window
                                              ExtWinType &extWinType,  // Exterior window type (InZoneExtWin, AdjZoneExtWin, NotInOrAdjZoneExtWin)
                                              int &IConst,             // Construction counter
                                              Vector3<Real64> &RREF2,  // Location of virtual reference point in absolute coordinate system
                                              Real64 &DWX,             // Horizontal dimension of window element (m)
                                              Real64 &DWY,             // Vertical dimension of window element (m)
                                              Real64 &DAXY,            // Area of window element
                                              Vector3<Real64> &U2,     // Second vertex of window for TDD:DOME (if exists)
                                              Vector3<Real64> &U23,    // Vector from window vertex 2 to window vertex 3 for TDD:DOME (if exists)
                                              Vector3<Real64> &U21,    // Vector from window vertex 2 to window vertex 1 for TDD:DOME (if exists)
                                              Vector3<Real64> &VIEWVC2, // Virtual view vector in absolute coordinate system
                                              bool &is_Rectangle,       // True if window is rectangular
                                              bool &is_Triangle,        // True if window is triangular
                                              int const MapNum)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   November 2012, refactor from legacy code by Fred Winklemann

    // PURPOSE OF THIS SUBROUTINE:
    // collect code to setup calculations for each window for daylighting coefficients

    // METHODOLOGY EMPLOYED:
    // switch as need to serve both reference points and map points based on calledFrom
    auto &dl = state.dataDayltg;
    auto &s_surf = state.dataSurface;

    int ShelfNum; // Daylighting shelf object number
    int NDIVX;    // Number of window x divisions for daylighting calc
    int NDIVY;    // Number of window y divisions for daylighting calc
    Real64 ALF;   // Distance from reference point to window plane (m)
    Real64 D1a;   // Projection of vector from window origin to reference
    //  on window X  axis (m)
    Real64 D1b; // Projection of vector from window origin to reference
    //  on window Y axis (m)
    Real64 SolidAngExtWin;    // Approx. solid angle subtended by an ext. window wrt ref pt
    Real64 SolidAngMinIntWin; // Approx. smallest solid angle subtended by an int. window wrt ref pt
    Real64 SolidAngRatio;     // Ratio of SolidAngExtWin and SolidAngMinIntWin
    Real64 SinCornerAng;      // For triangle, sine of corner angle of window element

    int zoneNum = 0; // zone number
    int enclNum = 0; // enclosure number

    Vector3<Real64> W1 = {0.0, 0.0, 0.0};
    Vector3<Real64> WC = {0.0, 0.0, 0.0};

    if (CalledFrom == CalledFor::RefPoint) {
        auto &daylCtrl = dl->daylightControl(daylightCtrlNum);
        daylCtrl.refPts(iRefPoint).extWins(loopwin).solidAng = 0.0;
        daylCtrl.refPts(iRefPoint).extWins(loopwin).solidAngWtd = 0.0;
        zoneNum = daylCtrl.zoneIndex;
        enclNum = daylCtrl.enclIndex;
    } else if (CalledFrom == CalledFor::MapPoint) {
        assert(MapNum > 0);
        auto const &illumMap = dl->illumMaps(MapNum);
        zoneNum = illumMap.zoneIndex;
        enclNum = illumMap.enclIndex;
    }
    IWin = dl->enclDaylight(enclNum).DayltgExtWinSurfNums(loopwin);

    auto &surf = s_surf->Surface(IWin);
    auto &surfWin = s_surf->SurfaceWindow(IWin);

    if (s_surf->Surface(surf.BaseSurf).SolarEnclIndex == enclNum) {
        extWinType = ExtWinType::InZone;
    } else {
        extWinType = ExtWinType::AdjZone;
    }

    IConst = s_surf->SurfActiveConstruction(IWin);

    // For thermochromic windows, the daylight and glare factors are calculated for a base window cosntruction
    //  at base TC layer temperature. During each time step calculations at DayltgInteriorIllum,
    //  DayltgInteriorMapIllum, and DayltgGlare, the daylight and glare factors are adjusted by the visible
    //  transmittance ratio = VT of actual TC window based on last hour TC layer temperature / VT of the base TC window
    if (state.dataConstruction->Construct(IConst).isTCWindow) {
        // For thermochromic windows, use the base window construction at base temperature of the TC layer
        IConst = state.dataConstruction->Construct(IConst).TCMasterConstrNum;
    }

    ICtrl = surf.activeWindowShadingControl;
    ShType = WinShadingType::NoShade; // 'NOSHADE'
    BlNum = 0;
    // ScNum = 0; //Unused Set but never used
    if (surf.HasShadeControl) {
        ShType = s_surf->WindowShadingControl(ICtrl).ShadingType;
    }
    if (ANY_BLIND(ShType)) {
        BlNum = s_surf->surfShades(IWin).blind.matNum;
    }
    // ScNum = SurfaceWindow( IWin ).ScreenNumber; //Unused Set but never used

    ShelfNum = s_surf->SurfDaylightingShelfInd(IWin);
    if (ShelfNum > 0) {
        InShelfSurf =
            state.dataDaylightingDevicesData->Shelf(s_surf->SurfDaylightingShelfInd(IWin)).InSurf; // Inside daylighting shelf present if > 0
    } else {
        InShelfSurf = 0;
    }

    is_Rectangle = false;
    is_Triangle = false;
    if (surf.Sides == 3) {
        is_Triangle = true;
    }
    if (surf.Sides == 4) {
        is_Rectangle = true;
    }

    if (is_Rectangle) {
        // Vertices of window (numbered counter-clockwise starting at upper left as viewed
        // from inside of room). Assumes original vertices are numbered counter-clockwise from
        // upper left as viewed from outside.
        W3 = surf.Vertex(2);
        W2 = surf.Vertex(3);
        W1 = surf.Vertex(4);
    } else if (is_Triangle) {
        W3 = surf.Vertex(2);
        W2 = surf.Vertex(3);
        W1 = surf.Vertex(1);
    }

    // Shade/blind calculation flag
    LSHCAL = 0;

    // Visible transmittance at normal incidence
    s_surf->SurfWinVisTransSelected(IWin) = Window::POLYF(1.0, state.dataConstruction->Construct(IConst).TransVisBeamCoef) * surfWin.glazedFrac;
    // For windows with switchable glazing, ratio of visible transmittance at normal
    // incidence for fully switched (dark) state to that of unswitched state
    s_surf->SurfWinVisTransRatio(IWin) = 1.0;
    if (ICtrl > 0) {
        if (ShType == WinShadingType::SwitchableGlazing) {
            int IConstShaded = surf.activeShadedConstruction; // Shaded construction counter
            s_surf->SurfWinVisTransRatio(IWin) =
                General::SafeDivide(Window::POLYF(1.0, state.dataConstruction->Construct(IConstShaded).TransVisBeamCoef),
                                    Window::POLYF(1.0, state.dataConstruction->Construct(IConst).TransVisBeamCoef));
        }
    }

    // Unit vectors from window vertex 2 to 1 and 2 to 3,
    // center point of window, and vector from ref pt to center of window
    W21 = W1 - W2;
    W23 = W3 - W2;
    Real64 HW = W21.magnitude();
    Real64 WW = W23.magnitude();
    if (is_Rectangle) {
        WC = W2 + (W23 + W21) / 2.0;
    } else if (is_Triangle) {
        WC = W2 + (W23 + W21) / 3.0;
    }
    s_surf->SurfaceWindow(IWin).WinCenter = WC;
    Vector3<Real64> REFWC = WC - RREF;
    // Unit vectors
    W21 /= HW;
    W23 /= WW;

    // Unit vector normal to window (pointing away from room)
    Vector3<Real64> WNORM = surf.lcsz;

    // Initialize number of window elements
    NDIVX = 40; // Does this mean that windows are split into 1,600 points for daylighting? WHYYYYYY?
    NDIVY = 40;

    // Distance from ref point to window plane
    ALF = std::abs(dot(WNORM, REFWC));
    if (CalledFrom == CalledFor::RefPoint) {
        // Check if ref point to close to window due to input error (0.1524 m below is 0.5 ft)
        if (ALF < 0.1524 && extWinType == ExtWinType::InZone) {
            // Ref pt is close to window plane. Get vector from window
            // origin to projection of ref pt on window plane.
            Vector3<Real64> W2REF = RREF + ALF * WNORM - W2;

            D1a = dot(W2REF, W23);
            D1b = dot(W2REF, W21);

            //            ! Error message if ref pt is too close to window.
            if (D1a > 0.0 && D1b > 0.0 && D1b <= HW && D1a <= WW) {
                ShowSevereError(
                    state,
                    format("CalcDaylightCoeffRefPoints: Daylighting calculation cannot be done for Daylighting:Controls={} because reference point "
                           "#{} is less than 0.15m (6\") from window plane {}",
                           dl->daylightControl(daylightCtrlNum).Name,
                           iRefPoint,
                           surf.Name));
                ShowContinueError(state, format("Distance=[{:.5R}]. This is too close; check position of reference point.", ALF));
                ShowFatalError(state, "Program terminates due to preceding condition.");
            }
        } else if (ALF < 0.1524 && extWinType == ExtWinType::AdjZone) {
            if (dl->RefErrIndex(iRefPoint, IWin) == 0) { // only show error message once
                ShowWarningError(state,
                                 format("CalcDaylightCoeffRefPoints: For Daylghting:Controls=\"{}\" External Window=\"{}\"in Zone=\"{}\" reference "
                                        "point is less than 0.15m (6\") from window plane ",
                                        dl->daylightControl(daylightCtrlNum).Name,
                                        surf.Name,
                                        state.dataHeatBal->Zone(surf.Zone).Name));
                ShowContinueError(state,
                                  format("Distance=[{:.1R} m] to ref point=[{:.1R},{:.1R},{:.1R}], Inaccuracy in Daylighting Calcs may result.",
                                         ALF,
                                         RREF.x,
                                         RREF.y,
                                         RREF.z));
                dl->RefErrIndex(iRefPoint, IWin) = 1;
            }
        }
    } else if (CalledFrom == CalledFor::MapPoint) {
        if (ALF < 0.1524 && extWinType == ExtWinType::AdjZone) {
            if (dl->MapErrIndex(iRefPoint, IWin) == 0) { // only show error message once
                ShowWarningError(state,
                                 format("CalcDaylightCoeffMapPoints: For Zone=\"{}\" External Window=\"{}\"in Zone=\"{}\" map point is less than "
                                        "0.15m (6\") from window plane ",
                                        state.dataHeatBal->Zone(zoneNum).Name,
                                        surf.Name,
                                        state.dataHeatBal->Zone(surf.Zone).Name));
                ShowContinueError(
                    state,
                    format("Distance=[{:.1R} m] map point=[{:.1R},{:.1R},{:.1R}], Inaccuracy in Map Calcs may result.", ALF, RREF.x, RREF.y, RREF.z));
                dl->MapErrIndex(iRefPoint, IWin) = 1;
            }
        }
    }
    // Number of window elements in X and Y for daylighting calculation
    if (ALF > 0.1524) {
        NDIVX = 1 + int(4.0 * WW / ALF);
        NDIVY = 1 + int(4.0 * HW / ALF);
    }

    if (extWinType == ExtWinType::AdjZone) {
        // Adjust number of exterior window elements to give acceptable number of rays through
        // interior windows in the zone (for accuracy of interior window daylighting calculation)
        SolidAngExtWin = General::SafeDivide(((surf.Area + s_surf->SurfWinDividerArea(IWin)) / surf.Multiplier), pow_2(ALF));
        SolidAngMinIntWin = dl->enclDaylight(enclNum).MinIntWinSolidAng;
        SolidAngRatio = max(1.0, SolidAngExtWin / SolidAngMinIntWin);
        NDIVX *= std::sqrt(SolidAngRatio);
        NDIVY *= std::sqrt(SolidAngRatio);
    }

    NWX = min(40, NDIVX);
    NWY = min(40, NDIVY);

    // Discretization of triangle is simpler if NWX = NWY
    if (is_Triangle) {
        NWX = max(NWX, NWY);
        NWY = NWX;
    }

    // Edge lengths of window elements
    DWX = WW / NWX;
    DWY = HW / NWY;

    // Azimuth and altitude of window normal
    surfWin.phi = std::asin(WNORM.z);
    surfWin.theta = (std::abs(WNORM.x) > 1.0e-5 || std::abs(WNORM.y) > 1.0e-5) ? std::atan2(WNORM.y, WNORM.x) : 0.0;

    // Recalculation of values for TDD:DOME
    if (surf.OriginalClass == SurfaceClass::TDD_Diffuser) {

        // Look up the TDD:DOME object
        int PipeNum = s_surf->SurfWinTDDPipeNum(IWin);
        IWin2 = state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome;

        auto &surf2 = s_surf->Surface(IWin2);
        auto &surfWin2 = s_surf->SurfaceWindow(IWin2);

        // Calculate reference point coords relative to the diffuser coordinate system
        // W21, W23, and WNORM are the unit vectors
        Vector3<Real64> REFD = {dot(REFWC, W21), dot(REFWC, W23), dot(REFWC, WNORM)};

        // Calculate view vector coords relative to the diffuser coordinate system
        Vector3<Real64> VIEWVD = {dot(VIEWVC, W21), dot(VIEWVC, W23), dot(VIEWVC, WNORM)};

        Vector3<Real64> U3 = surf2.Vertex(2);
        U2 = surf2.Vertex(3);
        Vector3<Real64> U1 = {0.0, 0.0, 0.0};

        if (surf2.Sides == 4) {
            // Vertices of window (numbered counter-clockwise starting
            // at upper left as viewed from inside of room)
            // Assumes original vertices are numbered counter-clockwise from
            // upper left as viewed from outside.
            U3 = surf2.Vertex(2);
            U2 = surf2.Vertex(3);
            U1 = surf2.Vertex(4);
        } else if (surf2.Sides == 3) {
            U3 = surf2.Vertex(2);
            U2 = surf2.Vertex(3);
            U1 = surf2.Vertex(1);
        }

        // Unit vectors from window vertex 2 to 1 and 2 to 3,
        // center point of window, and vector from ref pt to center of window
        U21 = U1 - U2;
        U23 = U3 - U2;
        HW = U21.magnitude();
        WW = U23.magnitude();
        if (surf2.Sides == 4) {
            WC = U2 + (U23 + U21) / 2.0;
        } else if (surf2.Sides == 3) {
            WC = U2 + (U23 + U21) / 3.0;
        }
        s_surf->SurfaceWindow(IWin2).WinCenter = WC;
        // Unit vectors
        U21 /= HW;
        U23 /= WW;

        // Unit vector normal to dome (pointing away from TDD)
        // These are specific to the exterior.
        // NOTE:  Preserve WNORM for later in the code.
        WNORM2 = cross(U21, U23).normalize();

        // Azimuth and altitude of dome normal
        // These are specific to the exterior.
        surfWin2.phi = std::asin(WNORM2.z);
        surfWin2.theta = (std::abs(WNORM2.x) > 1.0e-5 || std::abs(WNORM2.y) > 1.0e-5) ? std::atan2(WNORM2.y, WNORM2.x) : 0.0;

        // Calculate new virtual reference point coords relative to dome coord system
        // W21, W23, and WNORM2 are now the unit vectors for the dome coord system
        REFWC = REFD.x * U21 + REFD.y * U23 + REFD.z * WNORM2;
        RREF2 = WC - REFWC;

        // Calculate new virtual view vector coords relative to dome coord system
        VIEWVC2 = VIEWVD.x * U21 + VIEWVD.y * U23 + VIEWVD.z * WNORM2;

        // Copy several values from the diffuser so that DayltgInterReflectedIllum works correctly
        // These are specific to the interior.
        surfWin2.rhoCeilingWall = surfWin.rhoCeilingWall;
        surfWin2.rhoFloorWall = surfWin.rhoFloorWall;
        surfWin2.fractionUpgoing = surfWin.fractionUpgoing;
        surfWin2.glazedFrac = surfWin.glazedFrac;

    } else {
        // This is not a TDD:DIFFUSER.  Make sure nothing is messed up for a regular window.
        IWin2 = IWin;
        WNORM2 = WNORM;
        RREF2 = RREF;
        VIEWVC2 = VIEWVC;

        U2 = W2;
        U21 = W21;
        U23 = W23;
    }

    // Initialize bsdf daylighting coefficients here.  Only one time initialization
    if (s_surf->SurfWinWindowModelType(IWin) == WindowModel::BSDF) {
        if (!state.dataBSDFWindow->ComplexWind(IWin).DaylightingInitialized) {
            int NRefPts = 0;
            if (CalledFrom == CalledFor::MapPoint) {
                NRefPts = dl->illumMaps(MapNum).TotalMapRefPoints;
            } else if (CalledFrom == CalledFor::RefPoint) {
                NRefPts = dl->daylightControl(daylightCtrlNum).TotalDaylRefPoints;
            }
            InitializeCFSDaylighting(state, daylightCtrlNum, IWin, NWX, NWY, RREF, NRefPts, iRefPoint, CalledFrom, MapNum);
            // if ((WinEl == (NWX * NWY)).and.(CalledFrom == CalledForMapPoint).and.(NRefPts == iRefPoint)) then
            if ((CalledFrom == CalledFor::MapPoint) && (NRefPts == iRefPoint)) {
                state.dataBSDFWindow->ComplexWind(IWin).DaylightingInitialized = true;
            }
        }
    }

    int iHrBeg = state.dataSysVars->DetailedSolarTimestepIntegration ? state.dataGlobal->HourOfDay : 1;
    int iHrEnd = state.dataSysVars->DetailedSolarTimestepIntegration ? state.dataGlobal->HourOfDay : Constant::iHoursInDay;

    for (int iHr = iHrBeg; iHr <= iHrEnd; ++iHr) {
        // Initialize sky and sun components of direct illuminance (arrays EDIRSK, EDIRSU, EDIRSUdisk)
        // and average window luminance (arrays AVWLSK, AVWLSU, AVWLSUdisk), at ref pt.
        dl->dirIllum(iHr)[iWinCover_Bare] = dl->dirIllum(iHr)[iWinCover_Shaded] = Illums();
        dl->avgWinLum(iHr)[iWinCover_Bare] = dl->avgWinLum(iHr)[iWinCover_Shaded] = Illums();
    }

    if (CalledFrom == CalledFor::RefPoint) {
        // Initialize solid angle subtended by window wrt ref pt
        // and solid angle weighted by glare position factor
        s_surf->SurfaceWindow(IWin).refPts(iRefPoint).solidAng = 0.0;
        s_surf->SurfaceWindow(IWin).refPts(iRefPoint).solidAngWtd = 0.0;
    }
    // Area of window element
    if (is_Rectangle) {
        DAXY = DWX * DWY;
    } else if (is_Triangle) {
        SinCornerAng = std::sqrt(1.0 - pow_2(dot(W21, W23)));
        DAXY = DWX * DWY * SinCornerAng;
    }
}

void FigureDayltgCoeffsAtPointsForWindowElements(
    EnergyPlusData &state,
    int const daylightCtrlNum, // Current daylighting control number (only used when called from RefPoint)
    int const iRefPoint,
    int const loopwin,
    CalledFor const CalledFrom, // indicate  which type of routine called this routine
    int const WinEl,            // Current window element number
    int const IWin,
    int const IWin2,
    int const iXelement,
    int const iYelement,
    Real64 &SkyObstructionMult,
    Vector3<Real64> const &W2,      // Second vertex of window
    Vector3<Real64> const &W21,     // Vector from window vertex 2 to window vertex 1
    Vector3<Real64> const &W23,     // Vector from window vertex 2 to window vertex 3
    Vector3<Real64> const &RREF,    // Location of a reference point in absolute coordinate system
    int const NWYlim,               // For triangle, largest NWY for a given IX
    Vector3<Real64> const &VIEWVC2, // Virtual view vector in absolute coordinate system
    Real64 const DWX,               // Horizontal dimension of window element (m)
    Real64 const DWY,               // Vertical dimension of window element (m)
    Real64 const DAXY,              // Area of window element
    Vector3<Real64> const &U2,      // Second vertex of window for TDD:DOME (if exists)
    Vector3<Real64> const &U23,     // Vector from window vertex 2 to window vertex 3 for TDD:DOME (if exists)
    Vector3<Real64> const &U21,     // Vector from window vertex 2 to window vertex 1 for TDD:DOME (if exists)
    Vector3<Real64> &RWIN,          // Center of a window element for TDD:DOME (if exists) in abs coord sys
    Vector3<Real64> &RWIN2,         // Center of a window element for TDD:DOME (if exists) in abs coord sys
    Vector3<Real64> &Ray,           // Unit vector along ray from reference point to window element
    Real64 &PHRAY,                  // Altitude of ray from reference point to window element (radians)
    int &LSHCAL,                    // Interior shade calculation flag:  0=not yet calculated, 1=already calculated
    Real64 &COSB,                   // Cosine of angle between window outward normal and ray from reference point to window element
    Real64 &ObTrans,                // Product of solar transmittances of exterior obstructions hit by ray
    Real64 &TVISB,                  // Visible transmittance of window for COSB angle of incidence (times light well
    Real64 &DOMEGA,                 // Solid angle subtended by window element wrt reference point (steradians)
    Real64 &THRAY,                  // Azimuth of ray from reference point to window element (radians)
    bool &hitIntObs,                // True iff interior obstruction hit
    bool &hitExtObs,                // True iff ray from ref pt to ext win hits an exterior obstruction
    Vector3<Real64> const &WNORM2,  // Unit vector normal to window
    ExtWinType const extWinType,    // Exterior window type (InZoneExtWin, AdjZoneExtWin, NotInOrAdjZoneExtWin)
    int const IConst,               // Construction counter
    Vector3<Real64> const &RREF2,   // Location of virtual reference point in absolute coordinate system
    bool const is_Triangle,
    Real64 &TVISIntWin,     // Visible transmittance of int win at COSBIntWin for light from ext win
    Real64 &TVISIntWinDisk, // Visible transmittance of int win at COSBIntWin for sun
    int const MapNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   November 2012, refactor from legacy code by Fred Winklemann

    // PURPOSE OF THIS SUBROUTINE:
    // collect code to do calculations for each window element for daylighting coefficients

    // REFERENCES:
    // switch as need to serve both reference points and map points based on calledFrom
    auto &dl = state.dataDayltg;
    auto &s_surf = state.dataSurface;

    Real64 RR; // Distance from ref point to intersection of view vector
    //  and plane normal to view vector and window element (m)
    Real64 ASQ; // Square of distance from above intersection to window element (m2)
    Real64 YD;  // Vertical displacement of window element wrt ref point

    Real64 COSBIntWin; // Cos of angle between int win outward normal and ray betw ref pt and
    //  exterior window element or between ref pt and sun

    // Local complex fenestration variables
    Real64 TransBeam; // Obstructions transmittance for incoming BSDF rays (temporary variable)

    auto &surfWin = s_surf->SurfaceWindow(IWin);

    ++LSHCAL;
    SkyObstructionMult = 1.0;

    // Center of win element in absolute coord sys
    RWIN = W2 + (double(iXelement) - 0.5) * W23 * DWX + (double(iYelement) - 0.5) * W21 * DWY;

    // Center of win element on TDD:DOME in absolute coord sys
    // If no TDD, RWIN2 = RWIN
    RWIN2 = U2 + (double(iXelement) - 0.5) * U23 * DWX + (double(iYelement) - 0.5) * U21 * DWY;

    // Distance between ref pt and window element
    Real64 DIS = distance(RWIN, RREF);

    // Unit vector along ray from ref pt to element
    Ray = (RWIN - RREF) / DIS;

    // Cosine of angle between ray and window outward normal
    COSB = dot(WNORM2, Ray);

    // If COSB > 0, direct light from window can reach ref pt. Otherwise go to loop
    // over sun position and calculate inter-reflected component of illuminance
    if (COSB <= 0.0) {
        return;
    }

    // Azimuth (-pi to pi) and altitude (-pi/2 to pi/2) of ray. Azimuth = 0 is along east.
    PHRAY = std::asin(Ray.z);
    if (std::abs(Ray.x) > 1.0e-5 || std::abs(Ray.y) > 1.0e-5) {
        THRAY = std::atan2(Ray.y, Ray.x);
    } else {
        THRAY = 0.0;
    }

    // Solid angle subtended by element wrt ref pt.
    Real64 DAXY1 = DAXY; // For triangle, area of window element at end of column
    // For triangle, at end of Y column only one half of parallelopiped's area contributes
    if (is_Triangle && iYelement == NWYlim) {
        DAXY1 = 0.5 * DAXY;
    }
    DOMEGA = DAXY1 * COSB / (DIS * DIS);

    // Calculate position factor (used in glare calculation) for this
    // win element / ref pt / view-vector combination
    Real64 POSFAC = 0.0;

    // Distance from ref pt to intersection of view vector and plane
    // normal to view vector containing the window element

    if (CalledFrom == CalledFor::RefPoint) {
        RR = DIS * dot(Ray, VIEWVC2);
        if (RR > 0.0) {
            // Square of distance from above intersection point to win element
            ASQ = DIS * DIS - RR * RR;
            // Vertical displacement of win element wrt ref pt
            YD = RWIN2.z - RREF2.z;
            // Horizontal and vertical displacement ratio and position factor
            Real64 XR = std::sqrt(std::abs(ASQ - YD * YD)) / RR;
            Real64 YR = std::abs(YD / RR);
            POSFAC = DayltgGlarePositionFactor(XR, YR);
        }
    }

    hitIntObs = false;
    int IntWinHitNum = 0;   // Surface number of interior window that is intersected
    bool hitIntWin = false; // Ray from ref pt passes through interior window
    TVISIntWinDisk = 0.0;   // Init Value
    TVISIntWin = 0.0;

    Vector3<Real64> HitPtIntWin = {0.0, 0.0, 0.0};
    auto const &surf = s_surf->Surface(IWin);
    if (surf.OriginalClass == SurfaceClass::TDD_Diffuser) {
        // Look up the TDD:DOME object
        int PipeNum = s_surf->SurfWinTDDPipeNum(IWin);
        // Unshaded visible transmittance of TDD for a single ray from sky/ground element
        TVISB = TransTDD(state, PipeNum, COSB, RadType::VisibleBeam) * surfWin.glazedFrac;

    } else { // Regular window
        if (s_surf->SurfWinWindowModelType(IWin) != WindowModel::BSDF) {
            // Vis trans of glass for COSB incidence angle
            TVISB = Window::POLYF(COSB, state.dataConstruction->Construct(IConst).TransVisBeamCoef) * surfWin.glazedFrac * surfWin.lightWellEff;
        } else {
            // Complex fenestration needs to use different equation for visible transmittance.  That will be calculated later
            // in the code since it depends on different incoming directions.  For now, just put zero to differentiate from
            // regular windows
            TVISB = 0.0;
        }
        if (extWinType == ExtWinType::AdjZone) {
            int zoneNum = 0;
            if (CalledFrom == CalledFor::RefPoint) {
                zoneNum = dl->daylightControl(daylightCtrlNum).zoneIndex;
            } else if (CalledFrom == CalledFor::MapPoint) {
                assert(MapNum > 0);
                zoneNum = dl->illumMaps(MapNum).zoneIndex;
            }
            // Does ray pass through an interior window in zone (ZoneNum) containing the ref point?
            for (int const spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                for (int IntWin = thisSpace.WindowSurfaceFirst; IntWin <= thisSpace.WindowSurfaceLast; ++IntWin) {
                    auto const &surfIntWin = s_surf->Surface(IntWin);
                    // in develop this was Surface(IntWin).Class == SurfaceClass::Window && Surface(IntWin).ExtBoundCond >= 1
                    if (surfIntWin.ExtBoundCond < 1) {
                        continue;
                    }

                    if (s_surf->Surface(surfIntWin.ExtBoundCond).Zone != surf.Zone) {
                        continue;
                    }

                    hitIntWin = PierceSurface(state, IntWin, RREF, Ray, HitPtIntWin);
                    if (hitIntWin) {
                        IntWinHitNum = IntWin;
                        COSBIntWin = dot(surfIntWin.OutNormVec, Ray);
                        if (COSBIntWin <= 0.0) {
                            hitIntWin = false;
                            IntWinHitNum = 0;
                            continue;
                        }
                        TVISIntWin = Window::POLYF(COSBIntWin, state.dataConstruction->Construct(surfIntWin.Construction).TransVisBeamCoef);
                        TVISB *= TVISIntWin;
                        break; // Ray passes thru interior window; exit from DO loop
                    }
                }
            } // End of loop over surfaces in zone ZoneNum

            if (!hitIntWin) {
                // Ray does not pass through an int win in ZoneNum. Therefore, it hits the opaque part
                // of a surface between ref point in ZoneNum and ext win element in adjacent zone.
                hitIntObs = true;
            }
        } // End of check if this is an ext win in an adjacent zone
    } // End of check if TDD:Diffuser or regular exterior window or complex fenestration

    // Check for interior obstructions
    if (extWinType == ExtWinType::InZone && !hitIntObs) {
        // Check for obstruction between reference point and window element
        // Returns hitIntObs = true iff obstruction is hit
        // (Example of interior obstruction is a wall in an L-shaped room that lies
        // between reference point and window.)
        hitIntObs = DayltgHitInteriorObstruction(state, IWin, RREF, RWIN);
    }

    if (extWinType == ExtWinType::AdjZone && IntWinHitNum > 0 && !hitIntObs) {
        // Check for obstruction between ref point and interior window through which ray passes
        hitIntObs = DayltgHitInteriorObstruction(state, IntWinHitNum, RREF, HitPtIntWin);
        if (!hitIntObs) {
            // Check for obstruction between intersection point on int window and ext win element
            hitIntObs = DayltgHitBetWinObstruction(state, IntWinHitNum, IWin, HitPtIntWin, RWIN);
        }
    }
    if (CalledFrom == CalledFor::RefPoint) {
        // Glare calculations only done for regular reference points, not for maps
        if (!hitIntObs) {
            if (extWinType == ExtWinType::InZone || (extWinType == ExtWinType::AdjZone && hitIntWin)) {
                // Increment solid angle subtended by portion of window above ref pt
                surfWin.refPts(iRefPoint).solidAng += DOMEGA;
                dl->daylightControl(daylightCtrlNum).refPts(iRefPoint).extWins(loopwin).solidAng += DOMEGA;
                // Increment position-factor-modified solid angle
                surfWin.refPts(iRefPoint).solidAngWtd += DOMEGA * POSFAC;
                dl->daylightControl(daylightCtrlNum).refPts(iRefPoint).extWins(loopwin).solidAngWtd += DOMEGA * POSFAC;
            }
        }
    }
    if (hitIntObs) {
        ObTrans = 0.0;
    }

    hitExtObs = false;
    if (!hitIntObs) {
        // No interior obstruction was hit.
        // Check for exterior obstructions between window element and sky/ground.
        // Get product of transmittances of obstructions hit by ray.
        // ObTrans = 1.0 will be returned if no exterior obstructions are hit.

        if (s_surf->SurfWinWindowModelType(IWin) != WindowModel::BSDF) {
            // the IHR (now HourOfDay) here is/was not correct, this is outside of hour loop
            // the hour is used to query schedule for transmission , not sure what to do
            // it will work for detailed and never did work correctly before.
            ObTrans = DayltgHitObstruction(state, state.dataGlobal->HourOfDay, IWin2, RWIN2, Ray);
            if (ObTrans < 1.0) {
                hitExtObs = true;
            }
        } else {
            // Transmittance from exterior obstruction surfaces is calculated here. This needs to be done for each timestep
            // in order to account for changes in exterior surface transmittances
            int CplxFenState = surfWin.ComplexFen.CurrentState;
            auto &complexWinDayltgGeom = state.dataBSDFWindow->ComplexWind(IWin).DaylghtGeom(CplxFenState);
            int NReflSurf = 0; // Number of blocked beams for complex fenestration
            if (CalledFrom == CalledFor::RefPoint) {
                NReflSurf = complexWinDayltgGeom.RefPoint(iRefPoint).NReflSurf(WinEl);
            } else if (CalledFrom == CalledFor::MapPoint) {
                NReflSurf = complexWinDayltgGeom.IlluminanceMap(iRefPoint, MapNum).NReflSurf(WinEl);
            }
            int RayIndex;
            for (int ICplxFen = 1; ICplxFen <= NReflSurf; ++ICplxFen) {
                if (CalledFrom == CalledFor::RefPoint) {
                    RayIndex = complexWinDayltgGeom.RefPoint(iRefPoint).RefSurfIndex(ICplxFen, WinEl);
                } else if (CalledFrom == CalledFor::MapPoint) {
                    RayIndex = complexWinDayltgGeom.IlluminanceMap(iRefPoint, MapNum).RefSurfIndex(ICplxFen, WinEl);
                }
                Vector3<Real64> RayVector = state.dataBSDFWindow->ComplexWind(IWin).Geom(CplxFenState).sInc(RayIndex);
                // It will get product of all transmittances
                TransBeam = DayltgHitObstruction(state, state.dataGlobal->HourOfDay, IWin, RWIN, RayVector);
                // IF (TransBeam > 0.0d0) ObTrans = TransBeam
                if (CalledFrom == CalledFor::RefPoint) {
                    complexWinDayltgGeom.RefPoint(iRefPoint).TransOutSurf(ICplxFen, WinEl) = TransBeam;
                } else if (CalledFrom == CalledFor::MapPoint) {
                    complexWinDayltgGeom.IlluminanceMap(iRefPoint, MapNum).TransOutSurf(ICplxFen, WinEl) = TransBeam;
                }
            }
            // This will avoid obstruction multiplier calculations for non-CFS window
            ObTrans = 0.0;
        }
    }

    if (s_surf->CalcSolRefl && PHRAY < 0.0 && ObTrans > 1.0e-6) {
        // Calculate effect of obstructions on shading of sky diffuse reaching the ground point hit
        // by the ray. This effect is given by the ratio SkyObstructionMult =
        // (obstructed sky diffuse at ground point)/(unobstructed sky diffuse at ground point).
        // This ratio is calculated for an isotropic sky.
        // Ground point hit by the ray:
        Real64 Alfa = std::acos(-Ray.z);
        Real64 Beta = std::atan2(Ray.y, Ray.x);
        // Distance between ground hit point and proj'n of center of window element onto ground (m)
        Real64 HorDis = (RWIN2.z - s_surf->GroundLevelZ) * std::tan(Alfa);
        Vector3<Real64> GroundHitPt = {RWIN2.x + HorDis * std::cos(Beta), RWIN2.y + HorDis * std::sin(Beta), s_surf->GroundLevelZ};

        SkyObstructionMult =
            CalcObstrMultiplier(state, GroundHitPt, DataSurfaces::AltAngStepsForSolReflCalc, DataSurfaces::AzimAngStepsForSolReflCalc);
    } // End of check if solar reflection calculation is in effect
} // FigureDayltgCoeffsAtPointsForWindowElements()

void InitializeCFSDaylighting(EnergyPlusData &state,
                              int const daylightCtrlNum,       // Current daylighting control number
                              int const IWin,                  // Complex fenestration number
                              int const NWX,                   // Number of horizontal divisions
                              int const NWY,                   // Number of vertical divisions
                              Vector3<Real64> const &RefPoint, // reference point coordinates
                              int const NRefPts,               // Number of reference points
                              int const iRefPoint,             // Reference points counter
                              CalledFor const CalledFrom,
                              int const MapNum)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   April 2013

    // PURPOSE OF THIS SUBROUTINE:
    // For incoming BSDF window direction calculates whether bin is coming from sky, ground or reflected surface.
    // Routine also calculates intersection points with ground and exterior reflection surfaces.
    auto &dl = state.dataDayltg;
    auto &s_surf = state.dataSurface;

    // Object Data
    DataBSDFWindow::BSDFDaylghtPosition elPos; // altitude and azimuth of intersection element
    Vector Vec;                                // temporary vector variable

    int NumOfWinEl = NWX * NWY; // Number of window elements

    auto &surf = s_surf->Surface(IWin);
    Real64 DWX = surf.Width / NWX;  // Window element width
    Real64 DWY = surf.Height / NWY; // Window element height

    int zoneNum = dl->daylightControl(daylightCtrlNum).zoneIndex;
    Real64 AZVIEW = (dl->daylightControl(daylightCtrlNum).ViewAzimuthForGlare + state.dataHeatBal->Zone(zoneNum).RelNorth +
                     state.dataHeatBal->BuildingAzimuth + state.dataHeatBal->BuildingRotationAppendixG) *
                    Constant::DegToRad;

    // Perform necessary calculations for window coordinates and vectors.  This will be used to calculate centroids for
    // each window element
    Vector3<Real64> W1 = {0.0, 0.0, 0.0};
    Vector3<Real64> W2 = {0.0, 0.0, 0.0};
    Vector3<Real64> W3 = {0.0, 0.0, 0.0};

    if (surf.Sides == 4) {
        W3 = surf.Vertex(2);
        W2 = surf.Vertex(3);
        W1 = surf.Vertex(4);
    } else if (surf.Sides == 3) {
        W3 = surf.Vertex(2);
        W2 = surf.Vertex(3);
        W1 = surf.Vertex(1);
    }

    Vector3<Real64> W21 = W1 - W2;
    W21 /= surf.Height;
    Vector3<Real64> W23 = W3 - W2;
    W23 /= surf.Width;
    Vector3<Real64> WNorm = surf.lcsz;

    Real64 WinElArea = DWX * DWY;
    if (surf.Sides == 3) {
        WinElArea *= std::sqrt(1.0 - pow_2(dot(W21, W23)));
    }

    auto &complexWin = state.dataBSDFWindow->ComplexWind(IWin);

    if (CalledFrom == CalledFor::MapPoint) {

        if (!allocated(complexWin.IlluminanceMap)) {
            complexWin.IlluminanceMap.allocate(NRefPts, (int)dl->illumMaps.size());
        }

        AllocateForCFSRefPointsGeometry(complexWin.IlluminanceMap(iRefPoint, MapNum), NumOfWinEl);

    } else if (CalledFrom == CalledFor::RefPoint) {
        if (!allocated(complexWin.RefPoint)) {
            complexWin.RefPoint.allocate(NRefPts);
        }

        AllocateForCFSRefPointsGeometry(complexWin.RefPoint(iRefPoint), NumOfWinEl);
    }

    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    //! Allocation for each complex fenestration state reference points
    //!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    if (!allocated(complexWin.DaylghtGeom)) {
        complexWin.DaylghtGeom.allocate(state.dataBSDFWindow->ComplexWind(IWin).NumStates);
    }

    // Calculation needs to be performed for each state
    for (int CurFenState = 1; CurFenState <= complexWin.NumStates; ++CurFenState) {
        // number of incident basis directions for current state
        int NBasis = complexWin.Geom(CurFenState).Inc.NBasis;
        // number of outgoing basis directions for current state
        int NTrnBasis = complexWin.Geom(CurFenState).Trn.NBasis;

        if (CalledFrom == CalledFor::MapPoint) {
            if ((int)dl->illumMaps.size() > 0) {
                // illuminance map for each state
                if (!allocated(complexWin.DaylghtGeom(CurFenState).IlluminanceMap)) {
                    complexWin.DaylghtGeom(CurFenState).IlluminanceMap.allocate(NRefPts, (int)dl->illumMaps.size());
                }

                AllocateForCFSRefPointsState(
                    state, complexWin.DaylghtGeom(CurFenState).IlluminanceMap(iRefPoint, MapNum), NumOfWinEl, NBasis, NTrnBasis);

                InitializeCFSStateData(state,
                                       complexWin.DaylghtGeom(CurFenState).IlluminanceMap(iRefPoint, MapNum),
                                       complexWin.IlluminanceMap(iRefPoint, MapNum),
                                       daylightCtrlNum,
                                       IWin,
                                       RefPoint,
                                       CurFenState,
                                       NBasis,
                                       NTrnBasis,
                                       AZVIEW,
                                       NWX,
                                       NWY,
                                       W2,
                                       W21,
                                       W23,
                                       DWX,
                                       DWY,
                                       WNorm,
                                       WinElArea);
            }

        } else if (CalledFrom == CalledFor::RefPoint) {
            if (!allocated(complexWin.DaylghtGeom(CurFenState).RefPoint)) {
                complexWin.DaylghtGeom(CurFenState).RefPoint.allocate(NRefPts);
            }

            AllocateForCFSRefPointsState(state, complexWin.DaylghtGeom(CurFenState).RefPoint(iRefPoint), NumOfWinEl, NBasis, NTrnBasis);

            InitializeCFSStateData(state,
                                   complexWin.DaylghtGeom(CurFenState).RefPoint(iRefPoint),
                                   complexWin.RefPoint(iRefPoint),
                                   daylightCtrlNum,
                                   IWin,
                                   RefPoint,
                                   CurFenState,
                                   NBasis,
                                   NTrnBasis,
                                   AZVIEW,
                                   NWX,
                                   NWY,
                                   W2,
                                   W21,
                                   W23,
                                   DWX,
                                   DWY,
                                   WNorm,
                                   WinElArea);
        }
    }
} // InitializeCFSDaylighting()

void InitializeCFSStateData(EnergyPlusData &state,
                            DataBSDFWindow::BSDFRefPoints &StateRefPoint,
                            DataBSDFWindow::BSDFRefPointsGeomDescr &DaylghtGeomDescr,
                            [[maybe_unused]] int const daylightCtrlNum, // Current daylighting control number
                            int const iWin,
                            Vector3<Real64> const &RefPoint, // reference point
                            int const CurFenState,
                            int const NBasis,
                            int const NTrnBasis,
                            Real64 const AZVIEW,
                            int const NWX,
                            int const NWY,
                            Vector3<Real64> const &W2,
                            Vector3<Real64> const &W21,
                            Vector3<Real64> const &W23,
                            Real64 const DWX,
                            Real64 const DWY,
                            Vector3<Real64> const &WNorm, // unit vector from window (point towards outside)
                            Real64 const WinElArea)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   June 2013

    // PURPOSE OF THIS SUBROUTINE:
    // Initialize daylight state data for current
    auto &s_surf = state.dataSurface;

    // SUBROUTINE LOCAL VARIABLES
    int curWinEl;
    bool hit;
    int TotHits;
    Real64 DotProd; // Temporary variable for manipulating dot product .dot.
    int NSky;
    int NGnd;
    int NReflSurf;
    int MaxTotHits;
    Real64 LeastHitDsq; // dist^2 from window element center to hit point
    Real64 HitDsq;
    Real64 TransRSurf;
    int J;

    Vector3<Real64> RWin;
    Vector3<Real64> V;
    Vector3<Real64> GroundHitPt;

    // temporary arrays for surfaces
    // Each complex fenestration state can have different number of basis elements
    // This is the reason for making these temporary arrays local
    Array1D_int TmpSkyInd(NBasis, 0);                              // Temporary sky index list
    Array1D_int TmpGndInd(NBasis, 0);                              // Temporary gnd index list
    Array1D<Real64> TmpGndMultiplier(NBasis, 0.0);                 // Temporary ground obstruction multiplier
    Array1D_int TmpRfSfInd(NBasis, 0);                             // Temporary RefSurfIndex
    Array1D_int TmpRfRyNH(NBasis, 0);                              // Temporary RefRayNHits
    Array2D_int TmpHSurfNo(s_surf->TotSurfaces, NBasis, 0);        // Temporary HitSurfNo
    Array2D<Real64> TmpHSurfDSq(s_surf->TotSurfaces, NBasis, 0.0); // Temporary HitSurfDSq

    // Object Data
    Vector3<Real64> Centroid;                                                                       // current window element centroid
    Vector3<Real64> HitPt;                                                                          // surface hit point
    Array1D<Vector3<Real64>> TmpGndPt(NBasis, Vector3<Real64>(0.0, 0.0, 0.0));                      // Temporary ground intersection list
    Array2D<Vector3<Real64>> TmpHitPt(s_surf->TotSurfaces, NBasis, Vector3<Real64>(0.0, 0.0, 0.0)); // Temporary HitPt

    CFSRefPointPosFactor(state, RefPoint, StateRefPoint, iWin, CurFenState, NTrnBasis, AZVIEW);

    auto const &surf = s_surf->Surface(iWin);

    curWinEl = 0;
    // loop through window elements. This will calculate sky, ground and reflection bins for each window element
    for (int IX = 1; IX <= NWX; ++IX) {
        for (int IY = 1; IY <= NWY; ++IY) {

            ++curWinEl;

            // centroid coordinates for current window element
            Centroid = W2 + (double(IX) - 0.5) * W23 * DWX + (double(IY) - 0.5) * W21 * DWY;
            RWin = Centroid;

            CFSRefPointSolidAngle(state, RefPoint, RWin, WNorm, StateRefPoint, DaylghtGeomDescr, iWin, CurFenState, NTrnBasis, curWinEl, WinElArea);

            NSky = 0;
            NGnd = 0;
            NReflSurf = 0;
            MaxTotHits = 0;
            // Calculation of potential surface obstruction for each incoming direction
            for (int IRay = 1; IRay <= NBasis; ++IRay) {

                hit = false;
                TotHits = 0;
                for (int JSurf = 1; JSurf <= s_surf->TotSurfaces; ++JSurf) {
                    auto &surf2 = s_surf->Surface(JSurf);

                    // the following test will cycle on anything except exterior surfaces and shading surfaces
                    if (surf2.HeatTransSurf && surf2.ExtBoundCond != ExternalEnvironment) {
                        continue;
                    }
                    //  skip the base surface containing the window and any other subsurfaces of that surface
                    if (JSurf == surf.BaseSurf || surf2.BaseSurf == surf.BaseSurf) {
                        continue;
                    }
                    //  skip surfaces that face away from the window
                    DotProd = dot(state.dataBSDFWindow->ComplexWind(iWin).Geom(CurFenState).sInc(IRay), surf2.NewellSurfaceNormalVector);
                    if (DotProd >= 0) {
                        continue;
                    }
                    hit = PierceSurface(state, JSurf, Centroid, state.dataBSDFWindow->ComplexWind(iWin).Geom(CurFenState).sInc(IRay), HitPt);
                    if (!hit) {
                        continue; // Miss: Try next surface
                    }
                    if (TotHits == 0) {
                        // First hit for this ray
                        TotHits = 1;
                        ++NReflSurf;
                        TmpRfSfInd(NReflSurf) = IRay;
                        TmpRfRyNH(NReflSurf) = 1;
                        TmpHSurfNo(1, NReflSurf) = JSurf;
                        TmpHitPt(1, NReflSurf) = HitPt;
                        V = HitPt - Centroid;                // vector array from window ctr to hit pt
                        LeastHitDsq = V.magnitude_squared(); // dist^2 window ctr to hit pt
                        TmpHSurfDSq(1, NReflSurf) = LeastHitDsq;
                        if (!surf2.HeatTransSurf && surf2.shadowSurfSched != nullptr) {
                            TransRSurf = 1.0; // If a shadowing surface may have a scheduled transmittance, treat it here as completely transparent
                        } else {
                            TransRSurf = 0.0;
                        }
                    } else {
                        V = HitPt - Centroid;
                        HitDsq = V.magnitude_squared();
                        if (HitDsq >= LeastHitDsq) {
                            if (TransRSurf > 0.0) { // forget the new hit if the closer hit is opaque
                                J = TotHits + 1;
                                if (TotHits > 1) {
                                    for (int I = 2; I <= TotHits; ++I) {
                                        if (HitDsq < TmpHSurfDSq(I, NReflSurf)) {
                                            J = I;
                                            break;
                                        }
                                    }
                                    if (!surf2.HeatTransSurf && surf2.shadowSurfSched == nullptr) {
                                        //  The new hit is opaque, so we can drop all the hits further away
                                        TmpHSurfNo(J, NReflSurf) = JSurf;
                                        TmpHitPt(J, NReflSurf) = HitPt;
                                        TmpHSurfDSq(J, NReflSurf) = HitDsq;
                                        TotHits = J;
                                    } else {
                                        //  The new hit is scheduled (presumed transparent), so keep the more distant hits
                                        //     Note that all the hists in the list will be transparent except the last,
                                        //       which may be either transparent or opaque
                                        if (TotHits >= J) {
                                            for (int I = TotHits; I >= J; --I) {
                                                TmpHSurfNo(I + 1, NReflSurf) = TmpHSurfNo(I, NReflSurf);
                                                TmpHitPt(I + 1, NReflSurf) = TmpHitPt(I, NReflSurf);
                                                TmpHSurfDSq(I + 1, NReflSurf) = TmpHSurfDSq(I, NReflSurf);
                                            }
                                            TmpHSurfNo(J, NReflSurf) = JSurf;
                                            TmpHitPt(J, NReflSurf) = HitPt;
                                            TmpHSurfDSq(J, NReflSurf) = HitDsq;
                                            ++TotHits;
                                        }
                                    } // if (.NOT.Surface(JSurf)%HeatTransSurf .AND. Surface(JSurf)%SchedShadowSurfIndex == 0)  then
                                } // if (TotHits > 1) then
                            } // if (TransRSurf  > 0.0d0) then
                        } else { // if (HitDsq >= LeastHitDsq) then
                            //  A new closest hit.  If it is opaque, drop the current hit list,
                            //    otherwise add it at the front
                            LeastHitDsq = HitDsq;
                            if (!surf2.HeatTransSurf && surf2.shadowSurfSched != nullptr) {
                                TransRSurf = 1.0; // New closest hit is transparent, keep the existing hit list
                                for (int I = TotHits; I >= 1; --I) {
                                    TmpHSurfNo(I + 1, NReflSurf) = TmpHSurfNo(I, NReflSurf);
                                    TmpHitPt(I + 1, NReflSurf) = TmpHitPt(I, NReflSurf);
                                    TmpHSurfDSq(I + 1, NReflSurf) = TmpHSurfDSq(I, NReflSurf);
                                    ++TotHits;
                                }
                            } else {
                                TransRSurf = 0.0; // New closest hit is opaque, drop the existing hit list
                                TotHits = 1;
                            }
                            TmpHSurfNo(1, NReflSurf) = JSurf; // In either case the new hit is put in position 1
                            TmpHitPt(1, NReflSurf) = HitPt;
                            TmpHSurfDSq(1, NReflSurf) = LeastHitDsq;
                        }
                    }
                } // do JSurf = 1, TotSurfaces
                if (TotHits <= 0) {
                    auto const &sIncRay = state.dataBSDFWindow->ComplexWind(iWin).Geom(CurFenState).sInc(IRay);
                    // This ray reached the sky or ground unobstructed
                    if (sIncRay.z < 0.0) {
                        // A ground ray
                        ++NGnd;
                        TmpGndInd(NGnd) = IRay;
                        TmpGndPt(NGnd).x = Centroid.x - (sIncRay.x / sIncRay.z) * Centroid.z;
                        TmpGndPt(NGnd).y = Centroid.y - (sIncRay.y / sIncRay.z) * Centroid.z;
                        TmpGndPt(NGnd).z = 0.0;

                        // for solar reflectance calculations, need to precalculate obstruction multipliers
                        if (s_surf->CalcSolRefl) {
                            GroundHitPt = TmpGndPt(NGnd);
                            TmpGndMultiplier(NGnd) =
                                CalcObstrMultiplier(state, GroundHitPt, AltAngStepsForSolReflCalc, DataSurfaces::AzimAngStepsForSolReflCalc);
                        }
                    } else {
                        // A sky ray
                        ++NSky;
                        TmpSkyInd(NSky) = IRay;
                    }
                } else {
                    // Save the number of hits for this ray
                    TmpRfRyNH(NReflSurf) = TotHits;
                }
                MaxTotHits = max(MaxTotHits, TotHits);
            } // do IRay = 1, ComplexWind(IWin)%Geom(CurFenState)%Inc%NBasis

            // Fill up state data for current window element data
            StateRefPoint.NSky(curWinEl) = NSky;
            StateRefPoint.SkyIndex({1, NSky}, curWinEl) = TmpSkyInd({1, NSky});

            StateRefPoint.NGnd(curWinEl) = NGnd;
            StateRefPoint.GndIndex({1, NGnd}, curWinEl) = TmpGndInd({1, NGnd});
            StateRefPoint.GndPt({1, NGnd}, curWinEl) = TmpGndPt({1, NGnd});
            StateRefPoint.GndObstrMultiplier({1, NGnd}, curWinEl) = TmpGndMultiplier({1, NGnd});

            StateRefPoint.NReflSurf(curWinEl) = NReflSurf;
            StateRefPoint.RefSurfIndex({1, NReflSurf}, curWinEl) = TmpRfSfInd({1, NReflSurf});
            StateRefPoint.RefRayNHits({1, NReflSurf}, curWinEl) = TmpRfRyNH({1, NReflSurf});
            StateRefPoint.HitSurfNo({1, MaxTotHits}, {1, NReflSurf}, curWinEl) = TmpHSurfNo({1, MaxTotHits}, {1, NReflSurf});
            StateRefPoint.HitSurfDSq({1, MaxTotHits}, {1, NReflSurf}, curWinEl) = TmpHSurfDSq({1, MaxTotHits}, {1, NReflSurf});
            StateRefPoint.HitPt({1, MaxTotHits}, {1, NReflSurf}, curWinEl) = TmpHitPt({1, MaxTotHits}, {1, NReflSurf});
        } // do IY = 1, NWY
    } // do IX = 1, NWX
}

void AllocateForCFSRefPointsState(
    [[maybe_unused]] EnergyPlusData &state, DataBSDFWindow::BSDFRefPoints &StateRefPoint, int const NumOfWinEl, int const NBasis, int const NTrnBasis)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   June 2013

    // PURPOSE OF THIS SUBROUTINE:
    // Memory allocation for complex fenestration systems reference points geometry
    auto &s_surf = state.dataSurface;

    if (!allocated(StateRefPoint.NSky)) {
        StateRefPoint.NSky.allocate(NumOfWinEl);
        StateRefPoint.NSky = 0;
    }

    if (!allocated(StateRefPoint.SkyIndex)) {
        StateRefPoint.SkyIndex.allocate(NBasis, NumOfWinEl);
        StateRefPoint.SkyIndex = 0;
    }

    if (!allocated(StateRefPoint.NGnd)) {
        StateRefPoint.NGnd.allocate(NumOfWinEl);
        StateRefPoint.NGnd = 0;
    }

    if (!allocated(StateRefPoint.GndIndex)) {
        StateRefPoint.GndIndex.allocate(NBasis, NumOfWinEl);
        StateRefPoint.GndIndex = 0;
    }

    if (!allocated(StateRefPoint.GndPt)) {
        StateRefPoint.GndPt.allocate(NBasis, NumOfWinEl);
        StateRefPoint.GndPt = Vector(0.0, 0.0, 0.0);
    }

    if (!allocated(StateRefPoint.GndObstrMultiplier)) {
        StateRefPoint.GndObstrMultiplier.allocate(NBasis, NumOfWinEl);
        StateRefPoint.GndObstrMultiplier = 0.0;
    }

    if (!allocated(StateRefPoint.NReflSurf)) {
        StateRefPoint.NReflSurf.allocate(NumOfWinEl);
        StateRefPoint.NReflSurf = 0;
    }

    if (!allocated(StateRefPoint.RefSurfIndex)) {
        StateRefPoint.RefSurfIndex.allocate(NBasis, NumOfWinEl);
        StateRefPoint.RefSurfIndex = 0;
    }

    if (!allocated(StateRefPoint.TransOutSurf)) {
        StateRefPoint.TransOutSurf.allocate(NBasis, NumOfWinEl);
        StateRefPoint.TransOutSurf = 1.0;
    }

    if (!allocated(StateRefPoint.RefRayNHits)) {
        StateRefPoint.RefRayNHits.allocate(NBasis, NumOfWinEl);
        StateRefPoint.RefRayNHits = 0;
    }

    if (!allocated(StateRefPoint.HitSurfNo)) {
        StateRefPoint.HitSurfNo.allocate(s_surf->TotSurfaces, NBasis, NumOfWinEl);
        StateRefPoint.HitSurfNo = 0;
    }

    if (!allocated(StateRefPoint.HitSurfDSq)) {
        StateRefPoint.HitSurfDSq.allocate(s_surf->TotSurfaces, NBasis, NumOfWinEl);
        StateRefPoint.HitSurfDSq = 0.0;
    }

    if (!allocated(StateRefPoint.HitPt)) {
        StateRefPoint.HitPt.allocate(s_surf->TotSurfaces, NBasis, NumOfWinEl);
        StateRefPoint.HitPt = Vector(0.0, 0.0, 0.0);
    }

    if (!allocated(StateRefPoint.RefPointIndex)) {
        StateRefPoint.RefPointIndex.allocate(NumOfWinEl);
        StateRefPoint.RefPointIndex = 0;
    }

    if (!allocated(StateRefPoint.RefPointIntersection)) {
        StateRefPoint.RefPointIntersection.allocate(NTrnBasis);
        StateRefPoint.RefPointIntersection = false;
    }

    if (!allocated(StateRefPoint.RefPtIntPosFac)) {
        StateRefPoint.RefPtIntPosFac.allocate(NTrnBasis);
        StateRefPoint.RefPtIntPosFac = 0.0;
    }
}

void AllocateForCFSRefPointsGeometry(DataBSDFWindow::BSDFRefPointsGeomDescr &RefPointsGeomDescr, int const NumOfWinEl)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   June 2013

    // PURPOSE OF THIS SUBROUTINE:
    // Memory allocation for complex fenestration systems reference points geometry

    // SUBROUTINE LOCAL VARIABLES

    if (!allocated(RefPointsGeomDescr.SolidAngle)) {
        RefPointsGeomDescr.SolidAngle.allocate(NumOfWinEl);
        RefPointsGeomDescr.SolidAngle = 0.0;
    }

    if (!allocated(RefPointsGeomDescr.SolidAngleVec)) {
        RefPointsGeomDescr.SolidAngleVec.allocate(NumOfWinEl);
        RefPointsGeomDescr.SolidAngleVec = Vector(0.0, 0.0, 0.0);
    }
}

void CFSRefPointSolidAngle(EnergyPlusData &state,
                           Vector3<Real64> const &RefPoint,
                           Vector3<Real64> const &RWin,
                           Vector3<Real64> const &WNorm,
                           DataBSDFWindow::BSDFRefPoints &RefPointMap,
                           DataBSDFWindow::BSDFRefPointsGeomDescr &RefPointGeomMap,
                           int const iWin,
                           int const CurFenState,
                           int const NTrnBasis,
                           int const curWinEl,
                           Real64 const WinElArea)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   June 2013

    // PURPOSE OF THIS SUBROUTINE:
    // Calculate position factor for given reference point.

    // calculate vector from center of window element to the current reference point
    Vector3<Real64> Ray = RefPoint - RWin;

    // figure out outgoing beam direction from current reference point
    Real64 BestMatch = 0.0;
    for (int iTrnRay = 1; iTrnRay <= NTrnBasis; ++iTrnRay) {
        Vector3<Real64> const &V = state.dataBSDFWindow->ComplexWind(iWin).Geom(CurFenState).sTrn(iTrnRay);
        Real64 temp = dot(Ray, V);
        if (temp > BestMatch) {
            BestMatch = temp;
            RefPointMap.RefPointIndex(curWinEl) = iTrnRay;
        }
    }

    // calculate solid view angle
    Real64 Dist = Ray.magnitude();
    Vector3<Real64> RayNorm = Ray / (-Dist);
    RefPointGeomMap.SolidAngleVec(curWinEl) = RayNorm;
    Real64 CosB = dot(WNorm, RayNorm);
    RefPointGeomMap.SolidAngle(curWinEl) = WinElArea * CosB / (Dist * Dist);
}

void CFSRefPointPosFactor(EnergyPlusData &state,
                          Vector3<Real64> const &RefPoint,
                          DataBSDFWindow::BSDFRefPoints &RefPointMap,
                          int const iWin,
                          int const CurFenState,
                          int const NTrnBasis,
                          Real64 const AZVIEW)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   June 2013

    // PURPOSE OF THIS SUBROUTINE:
    // Calculate position factor for given reference point.

    auto const &sTrn = state.dataBSDFWindow->ComplexWind(iWin).Geom(CurFenState).sTrn;
    for (int iTrnRay = 1; iTrnRay <= NTrnBasis; ++iTrnRay) {
        Vector3<Real64> V = sTrn(iTrnRay);
        V.negate();

        Vector3<Real64> InterPoint;

        bool hit = PierceSurface(state, iWin, RefPoint, V, InterPoint);
        if (hit) {
            RefPointMap.RefPointIntersection(iTrnRay) = true;

            DataBSDFWindow::BSDFDaylghtPosition elPos = WindowComplexManager::DaylghtAltAndAzimuth(V);

            Real64 XR = std::tan(std::abs(Constant::PiOvr2 - AZVIEW - elPos.Azimuth) + 0.001);
            Real64 YR = std::tan(elPos.Altitude + 0.001);
            RefPointMap.RefPtIntPosFac(iTrnRay) = DayltgGlarePositionFactor(XR, YR);
        }
    }
} // CFSRefPointPosFactor()

Real64 CalcObstrMultiplier(EnergyPlusData &state,
                           Vector3<Real64> const &GroundHitPt, // Coordinates of point that ray hits ground (m)
                           int const AltSteps,                 // Number of steps in altitude angle for solar reflection calc
                           int const AzimSteps                 // Number of steps in azimuth angle of solar reflection calc
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   April 2013, refactor from legacy code by Fred Winklemann

    // PURPOSE OF THIS SUBROUTINE:
    // collect code to do obstruction multiplier from ground point

    // METHODOLOGY EMPLOYED:
    // Send rays upward from hit point and see which ones are unobstructed and so go to sky.
    // Divide hemisphere centered at ground hit point into elements of altitude Phi and
    // azimuth Theta and create upward-going ground ray unit vector at each Phi,Theta pair.
    // Phi = 0 at the horizon; Phi = Pi/2 at the zenith.

    // Locals
    auto const &dl = state.dataDayltg;
    auto const &s_surf = state.dataSurface;

    bool hitObs; // True iff obstruction is hit

    Vector3<Real64> URay;     // Unit vector in (Phi,Theta) direction
    Vector3<Real64> ObsHitPt; // Unit vector in (Phi,Theta) direction

    assert(AzimSteps <= DataSurfaces::AzimAngStepsForSolReflCalc);

    Real64 DPhi = Constant::PiOvr2 / (AltSteps / 2.0); // Phi increment (radians)
    Real64 DTheta = Constant::Pi / AzimSteps;          // Theta increment (radians)

    // Tuned Precompute Phi trig table
    if (AltSteps != dl->AltSteps_last) {
        for (int IPhi = 1, IPhi_end = (AltSteps / 2); IPhi <= IPhi_end; ++IPhi) {
            Real64 Phi = (IPhi - 0.5) * DPhi;
            dl->cos_Phi[IPhi] = std::cos(Phi);
            dl->sin_Phi[IPhi] = std::sin(Phi);
        }
        dl->AltSteps_last = AltSteps;
    }
    // Tuned Precompute Theta trig table
    if (AzimSteps != dl->AzimSteps_last) {
        for (int ITheta = 1; ITheta <= 2 * AzimSteps; ++ITheta) {
            Real64 Theta = (ITheta - 0.5) * DTheta;
            dl->cos_Theta[ITheta] = std::cos(Theta);
            dl->sin_Theta[ITheta] = std::sin(Theta);
        }
        dl->AzimSteps_last = AzimSteps;
    }

    Real64 SkyGndObs = 0.0;   // Obstructed sky irradiance at a ground point
    Real64 SkyGndUnObs = 0.0; // Unobstructed sky irradiance at a ground point

    // Altitude loop
    for (int IPhi = 1, IPhi_end = (AltSteps / 2); IPhi <= IPhi_end; ++IPhi) {
        Real64 sinPhi = dl->sin_Phi[IPhi]; // sinPhi
        Real64 cosPhi = dl->cos_Phi[IPhi]; // cosPhi

        // Third component of ground ray unit vector in (Theta,Phi) direction
        URay.z = sinPhi;
        Real64 dOmegaGnd = cosPhi * DTheta * DPhi; // Solid angle element of ray from ground point (steradians)
        // Cosine of angle of incidence of ground ray on ground plane
        Real64 CosIncAngURay = sinPhi;
        Real64 IncAngSolidAngFac = CosIncAngURay * dOmegaGnd / Constant::Pi; // CosIncAngURay*dOmegaGnd/Pi
        // Azimuth loop
        for (int ITheta = 1; ITheta <= 2 * AzimSteps; ++ITheta) {
            URay.x = cosPhi * dl->cos_Theta[ITheta];
            URay.y = cosPhi * dl->sin_Theta[ITheta];
            SkyGndUnObs += IncAngSolidAngFac;
            // Does this ground ray hit an obstruction?
            hitObs = false;
            if (s_surf->TotSurfaces < octreeCrossover) { // Linear search through surfaces

                for (int ObsSurfNum : s_surf->AllShadowPossObstrSurfaceList) {
                    hitObs = PierceSurface(state, ObsSurfNum, GroundHitPt, URay, ObsHitPt); // Check if ray pierces surface
                    if (hitObs) {
                        break;
                    }
                }

            } else { // Surface octree search

                // Lambda function for the octree to test for surface hit
                auto surfaceHit = [&GroundHitPt, &hitObs, &URay, &ObsHitPt](SurfaceData const &surface) -> bool {
                    if (surface.IsShadowPossibleObstruction) {
                        hitObs = PierceSurface(surface, GroundHitPt, URay, ObsHitPt); // Check if ray pierces surface
                        return hitObs;
                    } else {
                        return false;
                    }
                };

                // Check octree surface candidates until a hit is found, if any
                Vector3<Real64> const URay_inv(SurfaceOctreeCube::safe_inverse(URay));
                state.dataHeatBalMgr->surfaceOctree.hasSurfaceRayIntersectsCube(GroundHitPt, URay, URay_inv, surfaceHit);
            }

            if (hitObs) {
                continue; // Obstruction hit
            }
            // Sky is hit
            SkyGndObs += IncAngSolidAngFac;
        } // End of azimuth loop
    } // End of altitude loop

    // in case ground point is surrounded by obstructions (SkyGndUnObs == 0), then multiplier will be equal to zero
    // This should not happen anyway because in that case ray would not be able to reach ground point
    return (SkyGndUnObs != 0.0) ? (SkyGndObs / SkyGndUnObs) : 0.0;
} // CalcObstrMultiplier()

void FigureDayltgCoeffsAtPointsForSunPosition(
    EnergyPlusData &state,
    int const daylightCtrlNum, // Daylighting control index
    int const iRefPoint,
    int const iXelement,
    int const NWX, // Number of window elements in x direction for dayltg calc
    int const iYelement,
    int const NWY,   // Number of window elements in y direction for dayltg calc
    int const WinEl, // Current window element counter
    int const IWin,
    int const IWin2,
    int const iHour,
    int &ISunPos,
    Real64 const SkyObstructionMult,
    Vector3<Real64> const &RWIN2, // Center of a window element for TDD:DOME (if exists) in abs coord sys
    Vector3<Real64> const &Ray,   // Unit vector along ray from reference point to window element
    Real64 const PHRAY,           // Altitude of ray from reference point to window element (radians)
    int const LSHCAL,             // Interior shade calculation flag:  0=not yet calculated, 1=already calculated
    int const InShelfSurf,        // Inside daylighting shelf surface number
    Real64 const COSB,            // Cosine of angle between window outward normal and ray from reference point to window element
    Real64 const ObTrans,         // Product of solar transmittances of exterior obstructions hit by ray from reference point through a window element
    Real64 const TVISB,           // Visible transmittance of window for COSB angle of incidence (times light well efficiency, if appropriate)
    Real64 const DOMEGA,          // Solid angle subtended by window element wrt reference point (steradians)
    int const ICtrl,              // Window control counter
    WinShadingType const ShType,  // Window shading type
    [[maybe_unused]] int const BlNum, // Window blind number
    Real64 const THRAY,               // Azimuth of ray from reference point to window element (radians)
    Vector3<Real64> const &WNORM2,    // Unit vector normal to window
    ExtWinType const extWinType,      // Exterior window type (InZoneExtWin, AdjZoneExtWin, NotInOrAdjZoneExtWin)
    int const IConst,                 // Construction counter
    Real64 const AZVIEW,              // Azimuth of view vector in absolute coord system for glare calculation (radians)
    Vector3<Real64> const &RREF2,     // Location of virtual reference point in absolute coordinate system
    bool const hitIntObs,             // True iff interior obstruction hit
    bool const hitExtObs,             // True iff ray from ref pt to ext win hits an exterior obstruction
    CalledFor const CalledFrom,       // indicate  which type of routine called this routine
    Real64 TVISIntWin,                // Visible transmittance of int win at COSBIntWin for light from ext win
    Real64 &TVISIntWinDisk,           // Visible transmittance of int win at COSBIntWin for sun
    int const MapNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   November 2012, refactor from legacy code by Fred Winklemann

    // PURPOSE OF THIS SUBROUTINE:
    // collect code for calculations sun position aspects for daylighting coefficients

    // METHODOLOGY EMPLOYED:
    // switch as need to serve both reference points and map points based on calledFrom
    auto &s_surf = state.dataSurface;
    if (s_surf->SurfSunCosHourly(iHour).z < DataEnvironment::SunIsUpValue) {
        return;
    }

    auto &dl = state.dataDayltg;
    auto &s_mat = state.dataMaterial;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Vector3<Real64> RREF{0.0, 0.0, 0.0}; // Location of a reference point in absolute coordinate system //Autodesk Was used uninitialized:

    Real64 ObTransDisk;     // Product of solar transmittances of exterior obstructions hit by ray from reference point to sun
    Real64 LumAtHitPtFrSun; // Luminance at hit point of obstruction by reflection of direct light from sun (cd/m2)

    Real64 TVISS; // Direct solar visible transmittance of window at given angle of incidence
    //  (times light well efficiency, if appropriate)
    Real64 XAVWL; // XAVWL*TVISS is contribution of window luminance from solar disk (cd/m2)

    bool hitObs;          // True iff obstruction is hit
    Real64 ObsVisRefl;    // Visible reflectance of obstruction
    Real64 SkyReflVisLum; // Reflected sky luminance at hit point divided by

    Real64 SpecReflectance; // Specular reflectance of a reflecting surface
    Real64 TVisRefl;        // Bare window vis trans for reflected beam
    //  (times light well efficiency, if appropriate)
    Real64 PHSUNrefl; // Altitude angle of reflected sun (radians)
    Real64 THSUNrefl; // Azimuth anggle of reflected sun (radians)

    Real64 COSBIntWin; // Cos of angle between int win outward normal and ray betw ref pt and
    //  exterior window element or between ref pt and sun
    Real64 TVisIntWinMult;     // Interior window vis trans multiplier for ext win in adjacent zone
    Real64 TVisIntWinDiskMult; // Interior window vis trans solar disk multiplier for ext win in adj zone
    Real64 WindowSolidAngleDaylightPoint;

    ++ISunPos;

    // Altitude of sun (degrees)
    dl->sunAngles = dl->sunAnglesHr[iHour];

    // First time through, call routine to calculate inter-reflected illuminance
    // at reference point and luminance of window with shade, screen or blind.

    // Rob/TH - Not sure whether this call is necessary for interior zones with interior windows only.
    //  new code would be -
    // IF (LSHCAL == 1 .AND. ExtWinType /= AdjZoneExtWin) CALL DayltgInterReflectedIllum(ISunPos,IHR,ZoneNum,IWin2)
    int enclNum = 0; // enclosure index
    int zoneNum = 0; // zone index
    if (CalledFrom == CalledFor::RefPoint) {
        zoneNum = dl->daylightControl(daylightCtrlNum).zoneIndex;
        enclNum = dl->daylightControl(daylightCtrlNum).enclIndex;
    } else if (CalledFrom == CalledFor::MapPoint) {
        assert(MapNum > 0);
        zoneNum = dl->illumMaps(MapNum).zoneIndex;
        enclNum = dl->illumMaps(MapNum).enclIndex;
    }
    if (s_surf->SurfWinWindowModelType(IWin) != WindowModel::BSDF) {
        if (LSHCAL == 1) {
            DayltgInterReflectedIllum(state, ISunPos, iHour, enclNum, IWin2);
        }
    } else {
        if (LSHCAL == 1) {
            DayltgInterReflectedIllumComplexFenestration(state, IWin2, WinEl, iHour, daylightCtrlNum, iRefPoint, CalledFrom, MapNum);
        }
        if (COSB <= 0.0) {
            return;
        }
        DayltgDirectIllumComplexFenestration(state, IWin, WinEl, iHour, iRefPoint, CalledFrom, MapNum);
        // Call direct sun component only once since calculation is done for entire window
        if (WinEl == (NWX * NWY)) {
            DayltgDirectSunDiskComplexFenestration(state, IWin2, iHour, iRefPoint, WinEl, AZVIEW, CalledFrom, MapNum);
        }
        return;
    }

    // Daylighting shelf simplification:  The shelf completely blocks all view of the window,
    // only interrelflected illumination is allowed (see DayltgInterReflectedIllum above).
    // Everything else in this loop has to do with direct luminance from the window.
    if (InShelfSurf > 0) {
        return;
    }

    if (COSB <= 0.0) {
        return;
    }

    auto &surfWin = s_surf->SurfaceWindow(IWin);

    Illums XDirIllum;
    Illums XAvgWinLum;
    Real64 const Ray_3 = Ray.z;
    Real64 const DOMEGA_Ray_3 = DOMEGA * Ray_3;

    // Add contribution of this window element to glare and to
    // direct illuminance at reference point

    // The I,J,K indices for sky and sun components of direct illuminance
    // (EDIRSK, EDIRSU) and average window luminance (AVWLSK, AVWLSU) are:
    // I=1 for clear sky, =2 Clear turbid, =3 Intermediate, =4 Overcast;
    // J=1 for bare window, =2 for window with shade or fixed slat-angle blind;
    // K = sun position index.

    // ----- CASE I -- BARE WINDOW (no shading device)

    // Beam solar and sky solar reflected from nearest obstruction.
    // In the following hitIntObs == false  ==> no interior obstructions hit, and
    //                  hitExtObs == true  ==> one or more exterior obstructions hit.
    if (s_surf->CalcSolRefl && !hitIntObs && hitExtObs) {
        int NearestHitSurfNum;        // Surface number of nearest obstruction
        Vector3<Real64> NearestHitPt; // Hit point of ray on nearest obstruction
        // One or more exterior obstructions was hit; get contribution of reflection
        // from nearest obstruction.
        // Find obstruction whose hit point is closest to this ray's window element
        DayltgClosestObstruction(state, RWIN2, Ray, NearestHitSurfNum, NearestHitPt);
        if (NearestHitSurfNum > 0) {

            // Beam solar reflected from nearest obstruction

            LumAtHitPtFrSun = DayltgSurfaceLumFromSun(state, iHour, Ray, NearestHitSurfNum, NearestHitPt);
            dl->avgWinLum(iHour)[iWinCover_Bare].sun += LumAtHitPtFrSun * TVISB;
            if (PHRAY >= 0.0) {
                dl->dirIllum(iHour)[iWinCover_Bare].sun += LumAtHitPtFrSun * DOMEGA_Ray_3 * TVISB;
            }

            // Sky solar reflected from nearest obstruction

            int const ObsConstrNum = s_surf->SurfActiveConstruction(NearestHitSurfNum);
            if (ObsConstrNum > 0) {
                // Exterior building surface is nearest hit
                if (!state.dataConstruction->Construct(ObsConstrNum).TypeIsWindow) {
                    // Obstruction is not a window, i.e., is an opaque surface
                    ObsVisRefl = 1.0 - s_mat->materials(state.dataConstruction->Construct(ObsConstrNum).LayerPoint(1))->AbsorpVisible;
                } else {
                    // Obstruction is a window; assume it is bare
                    ObsVisRefl = state.dataConstruction->Construct(ObsConstrNum).ReflectVisDiffFront;
                }
            } else {
                // Shadowing surface is nearest hit
                if (s_surf->SurfDaylightingShelfInd(NearestHitSurfNum) > 0) {
                    // This is a daylighting shelf, for which reflection is separately calculated
                    ObsVisRefl = 0.0;
                } else {
                    ObsVisRefl = s_surf->SurfShadowDiffuseVisRefl(NearestHitSurfNum);
                    if (s_surf->SurfShadowGlazingConstruct(NearestHitSurfNum) > 0) {
                        ObsVisRefl += s_surf->SurfShadowGlazingFrac(NearestHitSurfNum) *
                                      state.dataConstruction->Construct(s_surf->SurfShadowGlazingConstruct(NearestHitSurfNum)).ReflectVisDiffFront;
                    }
                }
            }
            // Surface number to use when obstruction is a shadowing surface
            int NearestHitSurfNumX = NearestHitSurfNum;
            // Each shadowing surface has a "mirror" duplicate surface facing in the opposite direction.
            // The following gets the correct side of a shadowing surface for reflection.
            if (s_surf->Surface(NearestHitSurfNum).IsShadowing) {
                if (dot(Ray, s_surf->Surface(NearestHitSurfNum).OutNormVec) > 0.0) {
                    NearestHitSurfNumX = NearestHitSurfNum + 1;
                }
            }
            if (!state.dataSysVars->DetailedSkyDiffuseAlgorithm || !s_surf->ShadingTransmittanceVaries ||
                state.dataHeatBal->SolarDistribution == DataHeatBalance::Shadowing::Minimal) {
                SkyReflVisLum = ObsVisRefl * s_surf->Surface(NearestHitSurfNumX).ViewFactorSky *
                                state.dataSolarShading->SurfDifShdgRatioIsoSky(NearestHitSurfNumX) / Constant::Pi;
            } else {
                SkyReflVisLum = ObsVisRefl * s_surf->Surface(NearestHitSurfNumX).ViewFactorSky *
                                state.dataSolarShading->SurfDifShdgRatioIsoSkyHRTS(1, iHour, NearestHitSurfNumX) / Constant::Pi;
            }
            assert(equal_dimensions(dl->avgWinLum, dl->dirIllum));
            auto &gilsk = dl->horIllum[iHour];
            auto &avwlsk = dl->avgWinLum(iHour)[iWinCover_Bare];
            auto &edirsk = dl->dirIllum(iHour)[iWinCover_Bare];

            for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
                XAvgWinLum.sky[iSky] = gilsk.sky[iSky] * SkyReflVisLum;
                avwlsk.sky[iSky] += XAvgWinLum.sky[iSky] * TVISB;
                if (PHRAY >= 0.0) {
                    XDirIllum.sky[iSky] = gilsk.sky[iSky] * SkyReflVisLum * DOMEGA_Ray_3;
                    edirsk.sky[iSky] += XDirIllum.sky[iSky] * TVISB;
                }
            }
        }
    } // End of check if solar reflection calculation is in effect

    if (ObTrans > 1.e-6) {
        // Ray did not hit an obstruction or the transmittance product of hit obstructions is non-zero.
        // Contribution of sky or ground luminance in cd/m2
        if (s_surf->Surface(IWin).OriginalClass == SurfaceClass::TDD_Diffuser) {
            // Make all transmitted light diffuse for a TDD with a bare diffuser
            assert(equal_dimensions(dl->avgWinLum, dl->winLum));
            assert(equal_dimensions(dl->avgWinLum, dl->dirIllum));
            auto &avwlsk = dl->avgWinLum(iHour)[iWinCover_Bare];
            auto &edirsk = dl->dirIllum(iHour)[iWinCover_Bare];
            auto &wlumsk = dl->winLum(iHour)[iWinCover_Bare];
            for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
                avwlsk.sky[iSky] += wlumsk.sky[iSky];
                if (PHRAY > 0.0) {
                    edirsk.sky[iSky] += wlumsk.sky[iSky] * DOMEGA_Ray_3;
                }
            }

            dl->avgWinLum(iHour)[iWinCover_Bare].sun += dl->winLum(iHour)[iWinCover_Bare].sun;
            dl->avgWinLum(iHour)[iWinCover_Bare].sunDisk += dl->winLum(iHour)[iWinCover_Bare].sunDisk;

            if (PHRAY > 0.0) {
                dl->dirIllum(iHour)[iWinCover_Bare].sun += dl->winLum(iHour)[iWinCover_Bare].sun * DOMEGA_Ray_3;
            }
        } else {                         // Bare window
            Vector3<Real64> GroundHitPt; // Coordinates of point that ray hits ground (m)
            // Tuned Hoisted operations out of loop and linear indexing
            if (s_surf->CalcSolRefl) { // Coordinates of ground point hit by the ray
                Real64 Alfa = std::acos(-Ray_3);
                Real64 const Ray_1(Ray.x);
                Real64 const Ray_2(Ray.y);
                //                    Beta = std::atan2( Ray_2, Ray_1 ); //Unused Tuning below eliminated use
                // Distance between ground hit point and proj'n of center of window element onto ground (m)
                Real64 HorDis = (RWIN2.z - s_surf->GroundLevelZ) * std::tan(Alfa);
                GroundHitPt.z = s_surf->GroundLevelZ;
                // Tuned Replaced by below: sqrt is faster than sincos
                //                    GroundHitPt( 1 ) = RWIN2( 1 ) + HorDis * std::cos( Beta );
                //                    GroundHitPt( 2 ) = RWIN2( 2 ) + HorDis * std::sin( Beta );
                Real64 const Ray_r(std::sqrt(square(Ray_1) + square(Ray_2)));
                if (Ray_r > 0.0) {
                    HorDis /= Ray_r;
                    GroundHitPt.x = RWIN2.x + HorDis * Ray_1;
                    GroundHitPt.y = RWIN2.y + HorDis * Ray_2;
                } else { // Treat as angle==0
                    GroundHitPt.x = RWIN2.x + HorDis;
                    GroundHitPt.y = RWIN2.y;
                }
            }
            Real64 const GILSK_mult((state.dataEnvrn->GndReflectanceForDayltg / Constant::Pi) * ObTrans * SkyObstructionMult);
            Real64 const TVISB_ObTrans(TVISB * ObTrans);
            Real64 const AVWLSU_add(TVISB_ObTrans * dl->horIllum[iHour].sun * (state.dataEnvrn->GndReflectanceForDayltg / Constant::Pi));
            Vector3<Real64> const SUNCOS_iHour(s_surf->SurfSunCosHourly(iHour));
            assert(equal_dimensions(dl->dirIllum, dl->avgWinLum));
            auto &edirsk = dl->dirIllum(iHour)[iWinCover_Bare];
            auto &avwlsk = dl->avgWinLum(iHour)[iWinCover_Bare];

            for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
                if (PHRAY > 0.0) {                                                                     // Ray heads upward to sky
                    Real64 ELUM = DayltgSkyLuminance(state, static_cast<SkyType>(iSky), THRAY, PHRAY); // Sky or ground luminance (cd/m2)
                    XDirIllum.sky[iSky] = ELUM * DOMEGA_Ray_3;
                    Real64 DEDIR = XDirIllum.sky[iSky] * TVISB; // Illuminance contribution at reference point from window element (lux)
                    edirsk.sky[iSky] += DEDIR * ObTrans;
                    avwlsk.sky[iSky] += ELUM * TVISB_ObTrans;
                    XAvgWinLum.sky[iSky] = ELUM * ObTrans;
                } else { // PHRAY <= 0.
                    // Ray heads downward to ground.
                    // Contribution from sky diffuse reflected from ground
                    XAvgWinLum.sky[iSky] = dl->horIllum[iHour].sky[iSky] * GILSK_mult;
                    avwlsk.sky[iSky] += TVISB * XAvgWinLum.sky[iSky];
                    // Contribution from beam solar reflected from ground (beam reaching ground point
                    // can be obstructed [SunObstructionMult < 1.0] if CalcSolRefl = .TRUE.)
                } // End of check if ray is going up or down
            } // for (iSky)

            if (PHRAY <= 0.0) {
                // SunObstructionMult = 1.0; //Tuned
                if (s_surf->CalcSolRefl) { // Coordinates of ground point hit by the ray
                    // Sun reaches ground point if vector from this point to the sun is unobstructed
                    hitObs = false;
                    Vector3<Real64> ObsHitPt; // Coordinates of hit point on an obstruction (m)
                    for (int ObsSurfNum : s_surf->AllShadowPossObstrSurfaceList) {
                        hitObs = PierceSurface(state, ObsSurfNum, GroundHitPt, SUNCOS_iHour, ObsHitPt);
                        if (hitObs) {
                            break;
                        }
                    }
                    // if ( hitObs ) SunObstructionMult = 0.0;
                    if (!hitObs) {
                        dl->avgWinLum(iHour)[iWinCover_Bare].sun += AVWLSU_add;
                    }
                } else {
                    dl->avgWinLum(iHour)[iWinCover_Bare].sun += AVWLSU_add;
                }
            } // (PHRAY <= 0.0)
        }
    } // End of check if bare window or TDD:DIFFUSER

    // Illuminance from beam solar (without interior reflection)
    // Just run this once on the last pass
    if (iXelement == NWX && iYelement == NWY) { // Last pass

        // Beam solar reaching reference point directly without exterior reflection

        // Unit vector from ref. pt. to sun
        Vector3<Real64> RAYCOS;
        RAYCOS.x = dl->sunAngles.cosPhi * std::cos(dl->sunAngles.theta);
        RAYCOS.y = dl->sunAngles.cosPhi * std::sin(dl->sunAngles.theta);
        RAYCOS.z = dl->sunAngles.sinPhi;

        // Is sun on front side of exterior window?
        Real64 COSI = dot(WNORM2, RAYCOS); // Cosine of angle between direct sun and window outward normal
        bool hit;                          // True if ray from ref point thru window element hits an obstruction
        bool hitWin;                       // True if ray passes thru window
        Vector3<Real64> HP;
        if (COSI > 0.0) {

            // Does RAYCOS pass thru exterior window? HP is point that RAYCOS intersects window plane.
            hitWin = PierceSurface(state, IWin2, RREF2, RAYCOS, HP);
            // True if ray from ref pt to sun hits an interior obstruction
            if (hitWin) {
                bool hitIntObsDisk = false;
                if (extWinType == ExtWinType::InZone) {
                    // Check for interior obstructions between reference point and HP.
                    hitIntObsDisk = DayltgHitInteriorObstruction(state, IWin2, RREF2, HP);
                }
                ObTransDisk = 0.0; // Init value
                // Init flag for vector from RP to sun passing through interior window
                bool hitIntWinDisk = false;
                if (extWinType == ExtWinType::AdjZone) { // This block is for RPs in zones with interior windows
                    // adjacent to zones with exterior windows
                    // Does RAYCOS pass through interior window in zone containing RP?
                    // Loop over zone surfaces looking for interior windows between reference point and sun
                    // Surface number of int window intersected by ray betw ref pt and sun
                    int IntWinDiskHitNum;
                    // Intersection point on an interior window for ray from ref pt to sun (m)
                    Vector3<Real64> HitPtIntWinDisk;
                    auto const &thisZone = state.dataHeatBal->Zone(zoneNum);
                    for (int const spaceNum : thisZone.spaceIndexes) {
                        auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                        for (int IntWinDisk = thisSpace.WindowSurfaceFirst, IntWinDisk_end = thisSpace.WindowSurfaceLast;
                             IntWinDisk <= IntWinDisk_end;
                             ++IntWinDisk) {
                            auto const &surfIntWinDisk = s_surf->Surface(IntWinDisk);
                            if (surfIntWinDisk.ExtBoundCond < 1) {
                                continue;
                            }

                            if (s_surf->Surface(surfIntWinDisk.ExtBoundCond).Zone != s_surf->Surface(IWin2).Zone) {
                                continue;
                            }

                            hitIntWinDisk = PierceSurface(state, IntWinDisk, RREF, RAYCOS, HitPtIntWinDisk);
                            if (!hitIntWinDisk) {
                                continue;
                            }

                            IntWinDiskHitNum = IntWinDisk;
                            COSBIntWin = dot(surfIntWinDisk.OutNormVec, RAYCOS);
                            if (COSBIntWin <= 0.0) {
                                hitIntWinDisk = false;
                                IntWinDiskHitNum = 0;
                                continue;
                            }
                            TVISIntWinDisk =
                                Window::POLYF(COSBIntWin, state.dataConstruction->Construct(surfIntWinDisk.Construction).TransVisBeamCoef);
                            break;
                        } // for (IntWinDisk)
                    } // for (spaceNum)

                    if (!hitIntWinDisk) { // Vector from RP to sun does not pass through interior window
                        ObTransDisk = 0.0;
                        hit = true; //! fcw Is this needed?
                    }

                    // Check for interior obstructions between ref point and interior window
                    hitIntObsDisk = false;
                    if (hitIntWinDisk) {
                        hitIntObsDisk = DayltgHitInteriorObstruction(state, IntWinDiskHitNum, RREF, HitPtIntWinDisk);
                        // If no obstruction between RP and hit int win, check for obstruction
                        // between int win and ext win
                        if (!hitIntObsDisk) {
                            hitIntObsDisk = DayltgHitBetWinObstruction(state, IntWinDiskHitNum, IWin2, HitPtIntWinDisk, HP);
                        }
                    }
                    if (hitIntObsDisk) {
                        ObTransDisk = 0.0;
                    }
                } // case where RP is in zone with interior window adjacent to zone with exterior window

                //                    hitExtObsDisk = false; //Unused Set but never used
                // RJH 08-25-07 hitIntWinDisk should not be reset to false here, and should be tested below.
                // This is to correct logic flaw causing direct solar to reach adjacent zone refpt
                // when vector to sun does not pass through interior window
                // hitIntWinDisk = false
                if (!hitIntObsDisk) { // No interior obstruction was hit
                    // Net transmittance of exterior obstructions encountered by RAYCOS
                    // ObTransDisk = 1.0 will be returned if no exterior obstructions are hit.
                    ObTransDisk = DayltgHitObstruction(state, iHour, IWin2, RREF2, RAYCOS);
                    //                        if ( ObTransDisk < 1.0 ) hitExtObsDisk = true; //Unused Set but never used
                    // RJH 08-26-07 However, if this is a case of interior window
                    // and vector to sun does not pass through interior window
                    // then reset ObTransDisk to 0.0 since it is the key test for adding
                    // contribution of sun to RP below.
                    if ((extWinType == ExtWinType::AdjZone) && (!hitIntWinDisk)) {
                        ObTransDisk = 0.0;
                    }
                }

                // PETER: need side wall mounted TDD to test this
                // PETER: probably need to replace RREF2 with RWIN2
                // PETER: need to check for interior obstructions too.

                if (ObTransDisk > 1.e-6) {

                    // Sun reaches reference point;  increment illuminance.
                    // Direct normal illuminance is normalized to 1.0

                    if (s_surf->Surface(IWin).OriginalClass == SurfaceClass::TDD_Diffuser) {
                        // No beam is transmitted.  Takes care of TDD with a bare diffuser and all types of blinds.
                        TVISS = 0.0;
                    } else {
                        // Beam transmittance for bare window and all types of blinds
                        TVISS = Window::POLYF(COSI, state.dataConstruction->Construct(IConst).TransVisBeamCoef) * surfWin.glazedFrac *
                                surfWin.lightWellEff;
                        if (extWinType == ExtWinType::AdjZone && hitIntWinDisk) {
                            TVISS *= TVISIntWinDisk;
                        }
                    }

                    dl->dirIllum(iHour)[iWinCover_Bare].sunDisk = RAYCOS.z * TVISS * ObTransDisk; // Bare window

                    Real64 transBmBmMult = 0.0;

                    if (ANY_BLIND(ShType)) {
                        auto const &surfShade = s_surf->surfShades(IWin);
                        auto const *matBlind = dynamic_cast<Material::MaterialBlind const *>(s_mat->materials(surfShade.blind.matNum));
                        assert(matBlind != nullptr);

                        Real64 ProfAng = ProfileAngle(state, IWin, RAYCOS, matBlind->SlatOrientation);
                        transBmBmMult = matBlind->BeamBeamTrans(ProfAng, surfShade.blind.slatAng);
                        dl->dirIllum(iHour)[iWinCover_Shaded].sunDisk = RAYCOS.z * TVISS * transBmBmMult * ObTransDisk;

                    } else if (ShType == WinShadingType::ExtScreen) {
                        //                          pass angle from sun to window normal here using PHSUN and THSUN from above and surface angles
                        //                          SunAltitudeToWindowNormalAngle = PHSUN - SurfaceWindow(IWin)%Phi
                        //                          SunAzimuthToWindowNormalAngle = THSUN - SurfaceWindow(IWin)%Theta
                        auto const *screen = dynamic_cast<Material::MaterialScreen *>(s_mat->materials(surfWin.screenNum));
                        assert(screen != nullptr);

                        Real64 phi = std::abs(dl->sunAngles.phi - surfWin.phi);
                        Real64 theta = std::abs(dl->sunAngles.theta - surfWin.theta);
                        int ip1, ip2, it1, it2;
                        BilinearInterpCoeffs coeffs;
                        Material::NormalizePhiTheta(phi, theta);
                        Material::GetPhiThetaIndices(phi, theta, screen->dPhi, screen->dTheta, ip1, ip2, it1, it2);
                        GetBilinearInterpCoeffs(
                            phi, theta, ip1 * screen->dPhi, ip2 * screen->dPhi, it1 * screen->dTheta, it2 * screen->dTheta, coeffs);
                        transBmBmMult = BilinearInterp(screen->btars[ip1][it1].BmTrans,
                                                       screen->btars[ip1][it2].BmTrans,
                                                       screen->btars[ip2][it1].BmTrans,
                                                       screen->btars[ip2][it2].BmTrans,
                                                       coeffs);

                        dl->dirIllum(iHour)[iWinCover_Shaded].sunDisk = RAYCOS.z * TVISS * transBmBmMult * ObTransDisk;
                    }

                    if (CalledFrom == CalledFor::RefPoint) {
                        // Glare from solar disk

                        // Position factor for sun (note that AZVIEW is wrt y-axis and THSUN is wrt
                        // x-axis of absolute coordinate system.
                        Real64 XR = std::tan(std::abs(Constant::PiOvr2 - AZVIEW - dl->sunAngles.theta) + 0.001);
                        Real64 YR = std::tan(dl->sunAngles.phi + 0.001);
                        Real64 POSFAC =
                            DayltgGlarePositionFactor(XR, YR); // Position factor for a window element / ref point / view vector combination

                        WindowSolidAngleDaylightPoint = s_surf->SurfaceWindow(IWin).refPts(iRefPoint).solidAngWtd;

                        if (POSFAC != 0.0 && WindowSolidAngleDaylightPoint > 0.000001) {
                            // Increment window luminance.  Luminance of solar disk (cd/m2)
                            // is 1.47*10^4*(direct normal solar illuminance) for direct normal solar
                            // illuminance in lux (lumens/m2). For purposes of calculating daylight factors
                            // direct normal solar illuminance = 1.0.
                            // Solid angle subtended by sun is 0.000068 steradians

                            XAVWL = 14700.0 * std::sqrt(0.000068 * POSFAC) * double(NWX * NWY) / std::pow(WindowSolidAngleDaylightPoint, 0.8);
                            dl->avgWinLum(iHour)[iWinCover_Bare].sunDisk = XAVWL * TVISS * ObTransDisk; // Bare window

                            if (ANY_BLIND(ShType)) {
                                dl->avgWinLum(iHour)[iWinCover_Shaded].sunDisk = XAVWL * TVISS * transBmBmMult * ObTransDisk;
                            } else if (ShType == WinShadingType::ExtScreen) {
                                dl->avgWinLum(iHour)[iWinCover_Shaded].sunDisk = XAVWL * TVISS * transBmBmMult * ObTransDisk;
                            }
                        } // Position Factor
                    } // if (calledFrom == RefPt)
                } // if (ObTransDisk > 1e-6) // Beam avoids all obstructions
            } // if (hitWin)
        } // if (COSI > 0.0) // Sun on front side

        // Beam solar reaching reference point after beam-beam (specular) reflection from
        // an exterior surface

        if (s_surf->CalcSolRefl) {
            // Receiving surface number corresponding this window
            int RecSurfNum = s_surf->SurfShadowRecSurfNum(IWin2);
            if (RecSurfNum > 0) { // interior windows do not apply
                if (state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NumPossibleObs > 0) {
                    bool hitRefl;              // True iff ray hits reflecting surface
                    Vector3<Real64> HitPtRefl; // Point that ray hits reflecting surface
                    Vector3<Real64> SunVecMir; // Sun ray mirrored in reflecting surface
                    Vector3<Real64> ReflNorm;  // Normal vector to reflecting surface
                    // This window has associated obstructions that could reflect beam onto the window
                    for (int loop = 1, loop_end = state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NumPossibleObs; loop <= loop_end;
                         ++loop) {
                        int ReflSurfNum = state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).PossibleObsSurfNums(loop);
                        int ReflSurfNumX = ReflSurfNum;
                        // Each shadowing surface has a "mirror" duplicate surface facing in the opposite direction.
                        // The following gets the correct side of a shadowing surface for reflection.
                        if (s_surf->Surface(ReflSurfNum).IsShadowing) {
                            if (dot(RAYCOS, s_surf->Surface(ReflSurfNum).OutNormVec) < 0.0) {
                                ReflSurfNumX = ReflSurfNum + 1;
                            }
                        }
                        // Require that the surface can have specular reflection
                        if (s_surf->Surface(ReflSurfNum).Class == SurfaceClass::Window || s_surf->SurfShadowGlazingFrac(ReflSurfNum) > 0.0) {
                            ReflNorm = s_surf->Surface(ReflSurfNumX).OutNormVec;
                            // Vector to sun that is mirrored in obstruction
                            SunVecMir = RAYCOS - 2.0 * dot(RAYCOS, ReflNorm) * ReflNorm;
                            // Skip if reflecting surface is not sunlit
                            if (state.dataHeatBal->SurfSunlitFrac(iHour, 1, ReflSurfNumX) < 0.01) {
                                continue;
                            }
                            // Skip if altitude angle of mirrored sun is negative since reflected sun cannot
                            // reach reference point in this case
                            if (SunVecMir.z <= 0.0) {
                                continue;
                            }
                            // Cosine of incidence angle of reflected beam on window
                            Real64 CosIncAngRec = dot(s_surf->Surface(IWin2).OutNormVec, SunVecMir);
                            if (CosIncAngRec <= 0.0) {
                                continue;
                            }
                            // Does ray from ref. pt. along SunVecMir pass through window?
                            hitWin = PierceSurface(state, IWin2, RREF2, SunVecMir, HP);
                            if (!hitWin) {
                                continue; // Ray did not pass through window
                            }
                            // Check if this ray hits interior obstructions
                            hit = DayltgHitInteriorObstruction(state, IWin2, RREF2, HP);
                            if (hit) {
                                continue; // Interior obstruction was hit
                            }
                            // Does ray hit this reflecting surface?
                            hitRefl = PierceSurface(state, ReflSurfNum, RREF2, SunVecMir, HitPtRefl);
                            if (!hitRefl) {
                                continue; // Ray did not hit this reflecting surface
                            }
                            Real64 ReflDistanceSq = distance_squared(HitPtRefl, RREF2);
                            Real64 ReflDistance = std::sqrt(ReflDistanceSq);
                            // Is ray from ref. pt. to reflection point (HitPtRefl) obstructed?
                            bool hitObsRefl = false;
                            Vector3<Real64> HitPtObs; // Hit point on obstruction
                            for (int loop2 = 1, loop2_end = state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).NumPossibleObs;
                                 loop2 <= loop2_end;
                                 ++loop2) {
                                int const ObsSurfNum = state.dataSolarReflectionManager->SolReflRecSurf(RecSurfNum).PossibleObsSurfNums(loop2);
                                if (ObsSurfNum == ReflSurfNum || ObsSurfNum == s_surf->Surface(ReflSurfNum).BaseSurf) {
                                    continue;
                                }
                                hitObs = PierceSurface(state, ObsSurfNum, RREF2, SunVecMir, ReflDistance, HitPtObs); // ReflDistance cutoff added
                                if (hitObs) { // => Could skip distance check (unless < vs <= ReflDistance really matters)
                                    if (distance_squared(HitPtObs, RREF2) < ReflDistanceSq) { // Distance squared from ref pt to reflection point
                                        hitObsRefl = true;
                                        break;
                                    }
                                }
                            }
                            if (hitObsRefl) {
                                continue; // Obstruction closer than reflection pt. was hit; go to next obstruction
                            }
                            // There is no obstruction for this ray between ref pt and hit pt on reflecting surface.
                            // See if ray from hit pt on reflecting surface to original (unmirrored) sun position is obstructed
                            hitObs = false;
                            if (s_surf->Surface(ReflSurfNum).Class == SurfaceClass::Window) {
                                // Reflecting surface is a window.
                                // Receiving surface number for this reflecting window.
                                int ReflSurfRecNum = s_surf->SurfShadowRecSurfNum(ReflSurfNum);
                                if (ReflSurfRecNum > 0) {
                                    // Loop over possible obstructions for this reflecting window
                                    for (int loop2 = 1, loop2_end = state.dataSolarReflectionManager->SolReflRecSurf(ReflSurfRecNum).NumPossibleObs;
                                         loop2 <= loop2_end;
                                         ++loop2) {
                                        int const ObsSurfNum =
                                            state.dataSolarReflectionManager->SolReflRecSurf(ReflSurfRecNum).PossibleObsSurfNums(loop2);
                                        hitObs = PierceSurface(state, ObsSurfNum, HitPtRefl, RAYCOS, HitPtObs);
                                        if (hitObs) {
                                            break;
                                        }
                                    }
                                }
                            } else {
                                // Reflecting surface is a building shade
                                for (int ObsSurfNum : s_surf->AllShadowPossObstrSurfaceList) {
                                    if (ObsSurfNum == ReflSurfNum) {
                                        continue;
                                    }
                                    hitObs = PierceSurface(state, ObsSurfNum, HitPtRefl, RAYCOS, HitPtObs);
                                    if (hitObs) {
                                        break;
                                    }
                                }
                            } // End of check if reflector is a window or shadowing surface

                            if (hitObs) {
                                continue; // Obstruction hit between reflection hit point and sun; go to next obstruction
                            }

                            // No obstructions. Calculate reflected beam illuminance at ref. pt. from this reflecting surface.
                            SpecReflectance = 0.0;
                            Real64 CosIncAngRefl = std::abs(dot(RAYCOS, ReflNorm)); // Cos of angle of incidence of beam on reflecting surface
                            if (s_surf->Surface(ReflSurfNum).Class == SurfaceClass::Window) {
                                int const ConstrNumRefl = s_surf->SurfActiveConstruction(ReflSurfNum);
                                SpecReflectance =
                                    Window::POLYF(std::abs(CosIncAngRefl), state.dataConstruction->Construct(ConstrNumRefl).ReflSolBeamFrontCoef);
                            }
                            if (s_surf->Surface(ReflSurfNum).IsShadowing && s_surf->SurfShadowGlazingConstruct(ReflSurfNum) > 0) {
                                SpecReflectance =
                                    s_surf->SurfShadowGlazingFrac(ReflSurfNum) *
                                    Window::POLYF(
                                        std::abs(CosIncAngRefl),
                                        state.dataConstruction->Construct(s_surf->SurfShadowGlazingConstruct(ReflSurfNum)).ReflSolBeamFrontCoef);
                            }
                            TVisRefl = Window::POLYF(CosIncAngRec, state.dataConstruction->Construct(IConst).TransVisBeamCoef) * surfWin.glazedFrac *
                                       surfWin.lightWellEff;
                            dl->dirIllum(iHour)[iWinCover_Bare].sunDisk += SunVecMir.z * SpecReflectance * TVisRefl; // Bare window

                            Real64 TransBmBmMultRefl = 0.0;
                            if (ANY_BLIND(ShType)) {
                                auto const &surfShade = s_surf->surfShades(IWin);
                                auto const *matBlind = dynamic_cast<Material::MaterialBlind const *>(s_mat->materials(surfShade.blind.matNum));

                                Real64 ProfAng = ProfileAngle(state, IWin, SunVecMir, matBlind->SlatOrientation);
                                TransBmBmMultRefl = matBlind->BeamBeamTrans(ProfAng, surfShade.blind.slatAng);
                                dl->dirIllum(iHour)[iWinCover_Shaded].sunDisk += SunVecMir.z * SpecReflectance * TVisRefl * TransBmBmMultRefl;
                            } else if (ShType == WinShadingType::ExtScreen) {
                                // pass angle from sun to window normal here using PHSUN and THSUN from above and
                                // surface angles SunAltitudeToWindowNormalAngle = PHSUN - SurfaceWindow(IWin)%Phi
                                // SunAzimuthToWindowNormalAngle = THSUN - SurfaceWindow(IWin)%Theta
                                auto const *screen = dynamic_cast<Material::MaterialScreen const *>(s_mat->materials(surfWin.screenNum));
                                assert(screen != nullptr);

                                Real64 phi = std::abs(dl->sunAngles.phi - surfWin.phi);
                                Real64 theta = std::abs(dl->sunAngles.theta - surfWin.theta);
                                int ip1, ip2, it1, it2; // lo/hi phi/theta interpolation map indices
                                BilinearInterpCoeffs coeffs;
                                Material::NormalizePhiTheta(phi, theta);
                                Material::GetPhiThetaIndices(phi, theta, screen->dPhi, screen->dTheta, ip1, ip2, it1, it2);
                                GetBilinearInterpCoeffs(
                                    phi, theta, ip1 * screen->dPhi, ip2 * screen->dPhi, it1 * screen->dTheta, it2 * screen->dTheta, coeffs);

                                TransBmBmMultRefl = BilinearInterp(screen->btars[ip1][it1].BmTrans,
                                                                   screen->btars[ip1][it2].BmTrans,
                                                                   screen->btars[ip2][it1].BmTrans,
                                                                   screen->btars[ip2][it2].BmTrans,
                                                                   coeffs);
                                dl->dirIllum(iHour)[iWinCover_Shaded].sunDisk += SunVecMir.z * SpecReflectance * TVisRefl * TransBmBmMultRefl;
                            } // End of check if window has a blind or screen

                            // Glare from reflected solar disk

                            PHSUNrefl = SunVecMir.z;
                            THSUNrefl = std::atan2(SunVecMir.y, SunVecMir.x);
                            Real64 XR = std::tan(std::abs(Constant::PiOvr2 - AZVIEW - THSUNrefl) + 0.001);
                            Real64 YR = std::tan(PHSUNrefl + 0.001);
                            Real64 POSFAC = DayltgGlarePositionFactor(XR, YR);
                            if (POSFAC != 0.0 && s_surf->SurfaceWindow(IWin).refPts(iRefPoint).solidAngWtd > 0.000001) {
                                XAVWL = 14700.0 * std::sqrt(0.000068 * POSFAC) * double(NWX * NWY) /
                                        std::pow(s_surf->SurfaceWindow(IWin).refPts(iRefPoint).solidAngWtd, 0.8);
                                dl->avgWinLum(iHour)[iWinCover_Bare].sunDisk += XAVWL * TVisRefl * SpecReflectance; // Bare window
                                if (ANY_BLIND(ShType)) {
                                    dl->avgWinLum(iHour)[iWinCover_Shaded].sunDisk += XAVWL * TVisRefl * SpecReflectance * TransBmBmMultRefl;
                                } else if (ShType == WinShadingType::ExtScreen) {
                                    dl->avgWinLum(iHour)[iWinCover_Shaded].sunDisk += XAVWL * TVisRefl * SpecReflectance * TransBmBmMultRefl;
                                }
                            }
                        } // End of check that obstruction can specularly reflect
                    } // End of loop over obstructions associated with this window

                } // End of check if this window has associated obstructions
            } // End of check to see if this is exterior type window
        } // End of check if exterior reflection calculation is in effect

    } // Last pass

    if ((ICtrl > 0 && (ANY_BLIND(ShType) || ANY_SHADE_SCREEN(ShType))) || s_surf->SurfWinSolarDiffusing(IWin)) {

        // ----- CASE II -- WINDOW WITH SCREEN, SHADE, BLIND, OR DIFFUSING WINDOW
        // Interior window visible transmittance multiplier for exterior window in adjacent zone
        TVisIntWinMult = 1.0;
        TVisIntWinDiskMult = 1.0;
        if (s_surf->Surface(IWin).SolarEnclIndex != dl->daylightControl(daylightCtrlNum).enclIndex) {
            TVisIntWinMult = TVISIntWin;
            TVisIntWinDiskMult = TVISIntWinDisk;
        }

        Real64 const DOMEGA_Ray_3_TVisIntWinMult(DOMEGA_Ray_3 * TVisIntWinMult);

        auto &wlumsk = dl->winLum(iHour)[iWinCover_Shaded];
        auto &edirsk = dl->dirIllum(iHour)[iWinCover_Shaded];
        auto &avwlsk = dl->avgWinLum(iHour)[iWinCover_Shaded];
        for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
            // IF (.NOT.SurfaceWindow(IWin)%MovableSlats .AND. JB > 1) EXIT
            avwlsk.sky[iSky] += wlumsk.sky[iSky] * TVisIntWinMult;
            if (PHRAY > 0.0) {
                edirsk.sky[iSky] += wlumsk.sky[iSky] * DOMEGA_Ray_3_TVisIntWinMult;
            }
        } // for (iSky)

        dl->avgWinLum(iHour)[iWinCover_Shaded].sun += dl->winLum(iHour)[iWinCover_Shaded].sun * TVisIntWinMult;
        dl->avgWinLum(iHour)[iWinCover_Shaded].sunDisk += dl->winLum(iHour)[iWinCover_Shaded].sunDisk * TVisIntWinDiskMult;

        if (PHRAY > 0.0) {
            dl->dirIllum(iHour)[iWinCover_Shaded].sun += dl->winLum(iHour)[iWinCover_Shaded].sun * DOMEGA_Ray_3_TVisIntWinMult;
        }
    }
} // FigureDayltgCoeffsAtPointsForSunPosition()

void FigureRefPointDayltgFactorsToAddIllums(EnergyPlusData &state,
                                            int const daylightCtrlNum, // Current daylighting control number
                                            int const iRefPoint,
                                            int const iHour,
                                            int &ISunPos,
                                            int const IWin,
                                            int const loopwin,
                                            int const NWX,  // Number of window elements in x direction for dayltg calc
                                            int const NWY,  // Number of window elements in y direction for dayltg calc
                                            int const ICtrl // Window control counter
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith, Oct 2012, derived from legacy code by Fred Winkelmann
    //       DATE WRITTEN   Oct. 2012

    // PURPOSE OF THIS SUBROUTINE:
    // calculation worker routine to fill daylighting coefficients

    // METHODOLOGY EMPLOYED:
    // this version is just for reference points.

    // SUBROUTINE PARAMETER DEFINITIONS:
    Real64 constexpr tmpDFCalc(0.05); // cut off illuminance (lux) for exterior horizontal in calculating the daylighting and glare factors

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    auto &dl = state.dataDayltg;
    auto &s_surf = state.dataSurface;

    if (s_surf->SurfSunCosHourly(iHour).z < DataEnvironment::SunIsUpValue) {
        return;
    }

    ++ISunPos;

    // Altitude of sun (degrees)
    dl->sunAngles = dl->sunAnglesHr[iHour];

    auto &thisDayltgCtrl = dl->daylightControl(daylightCtrlNum);

    auto &surf = s_surf->Surface(IWin);

    int const enclNum = surf.SolarEnclIndex;

    // Loop over shading index (1=bare window; 2=diffusing glazing, shade, screen or fixed slat-angle blind;

    // TH. 9/22/2009. CR 7625 - daylight illuminance spikes during some sunset hours due to the calculated sky and sun
    //  related daylight factors > 1, which theoretically can occur when sun is perpendicular to the window
    //  and interior surfaces with high visible reflectance.
    // Added tmpDFCalc (default to 0.05 lux) as the cap for GILSK and GILSU in calculating the daylight factors
    //  the assumption behind it is if exterior horizontal surface does not get daylight, spaces do not get daylight.

    auto &daylFacHr = thisDayltgCtrl.daylFac[iHour];

    for (int iWinCover = 0; iWinCover < (int)WinCover::Num; ++iWinCover) {

        auto const &gilsk = dl->horIllum[iHour];
        auto const &edirsk = dl->dirIllum(iHour)[iWinCover];
        auto const &eintsk = dl->reflIllum(iHour)[iWinCover];
        auto const &avwlsk = dl->avgWinLum(iHour)[iWinCover];

        auto &daylFac = daylFacHr(loopwin, iRefPoint)[iWinCover];
        auto &illFac = daylFac[iLum_Illum];
        auto &sourceFac = daylFac[iLum_Source];
        auto &backFac = daylFac[iLum_Back];

        for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) { // Loop over sky types

            if (gilsk.sky[iSky] > tmpDFCalc) {
                illFac.sky[iSky] = (edirsk.sky[iSky] + eintsk.sky[iSky]) / gilsk.sky[iSky];
                sourceFac.sky[iSky] = avwlsk.sky[iSky] / (NWX * NWY * gilsk.sky[iSky]);
                backFac.sky[iSky] = eintsk.sky[iSky] * dl->enclDaylight(enclNum).aveVisDiffReflect / (Constant::Pi * gilsk.sky[iSky]);
            } else {
                illFac.sky[iSky] = 0.0;
                sourceFac.sky[iSky] = 0.0;
                backFac.sky[iSky] = 0.0;
            }

        } // for (iSky)

        if (dl->horIllum[iHour].sun > tmpDFCalc) {
            daylFac[iLum_Illum].sun = (dl->dirIllum(iHour)[iWinCover].sun + dl->reflIllum(iHour)[iWinCover].sun) / (dl->horIllum[iHour].sun + 0.0001);
            daylFac[iLum_Illum].sunDisk =
                (dl->dirIllum(iHour)[iWinCover].sunDisk + dl->reflIllum(iHour)[iWinCover].sunDisk) / (dl->horIllum[iHour].sun + 0.0001);
            daylFac[iLum_Source].sun = dl->avgWinLum(iHour)[iWinCover].sun / (NWX * NWY * (dl->horIllum[iHour].sun + 0.0001));
            daylFac[iLum_Source].sunDisk = dl->avgWinLum(iHour)[iWinCover].sunDisk / (NWX * NWY * (dl->horIllum[iHour].sun + 0.0001));
            daylFac[iLum_Back].sun = dl->reflIllum(iHour)[iWinCover].sun * dl->enclDaylight(enclNum).aveVisDiffReflect /
                                     (Constant::Pi * (dl->horIllum[iHour].sun + 0.0001));
            daylFac[iLum_Back].sunDisk = dl->reflIllum(iHour)[iWinCover].sunDisk * dl->enclDaylight(enclNum).aveVisDiffReflect /
                                         (Constant::Pi * (dl->horIllum[iHour].sun + 0.0001));
        } else {
            daylFac[iLum_Illum].sun = 0.0;
            daylFac[iLum_Illum].sunDisk = 0.0;

            daylFac[iLum_Source].sun = 0.0;
            daylFac[iLum_Source].sunDisk = 0.0;

            daylFac[iLum_Back].sun = 0.0;
            daylFac[iLum_Back].sunDisk = 0.0;
        }
    } // for (jSH)

    // For switchable glazing put daylighting factors for switched (dark) state in IS=2 location
    if (ICtrl > 0 && s_surf->WindowShadingControl(ICtrl).ShadingType == WinShadingType::SwitchableGlazing) {

        Real64 VTR = s_surf->SurfWinVisTransRatio(IWin); // Ratio of Tvis of fully-switched state to that of the unswitched state
        auto &daylFac2 = daylFacHr(loopwin, iRefPoint)[iWinCover_Shaded];
        auto const &daylFac1 = daylFacHr(loopwin, iRefPoint)[iWinCover_Bare];

        for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
            daylFac2[iLum_Illum].sky[iSky] = daylFac1[iLum_Illum].sky[iSky] * VTR;
            daylFac2[iLum_Source].sky[iSky] = daylFac1[iLum_Source].sky[iSky] * VTR;
            daylFac2[iLum_Back].sky[iSky] = daylFac1[iLum_Back].sky[iSky] * VTR;
        } // for (iSky)

        daylFac2[iLum_Illum].sun = daylFac1[iLum_Illum].sun * VTR;
        daylFac2[iLum_Source].sun = daylFac1[iLum_Source].sun * VTR;
        daylFac2[iLum_Back].sun = daylFac1[iLum_Back].sun * VTR;
        daylFac2[iLum_Illum].sunDisk = daylFac1[iLum_Illum].sunDisk * VTR;
        daylFac2[iLum_Source].sunDisk = daylFac1[iLum_Source].sunDisk * VTR;
        daylFac2[iLum_Back].sunDisk = daylFac1[iLum_Back].sunDisk * VTR;
    } // ICtrl > 0
} // FigureRefPointDayltgFactorsToAddIllums()

void FigureMapPointDayltgFactorsToAddIllums(EnergyPlusData &state,
                                            int const MapNum,
                                            int const iMapPoint,
                                            int const iHour,
                                            int const IWin,
                                            int const loopwin,
                                            int const ICtrl // Window control counter
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith, Oct 2012, derived from legacy code by Fred Winkelmann, Peter Ellis, Linda Lawrie
    //       DATE WRITTEN   Nov. 2012

    // PURPOSE OF THIS SUBROUTINE:
    // calculation worker routine to fill daylighting coefficients

    // METHODOLOGY EMPLOYED:
    // this version is just for map points.

    // SUBROUTINE PARAMETER DEFINITIONS:
    Real64 constexpr tmpDFCalc(0.05); // cut off illuminance (lux) for exterior horizontal in calculating
    // the daylighting and glare factors

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    auto &dl = state.dataDayltg;
    auto &s_surf = state.dataSurface;

    if (s_surf->SurfSunCosHourly(iHour).z < DataEnvironment::SunIsUpValue) {
        return;
    }

    // Loop over shading index (1=bare window; 2=diffusing glazing, shade, screen or blind;

    // TH. 9/22/2009. CR 7625 - daylight illuminance spikes during some sunset hours due to the calculated sky and sun
    //  related daylight factors > 1, which theoretically can occur when sun is perpendicular to the window
    //  and interior surfaces with high visible reflectance.
    // Added tmpDFCalc (default to 0.05 lux) as the cap for GILSK and GILSU in calculating the daylight factors
    //  the assumption behind it is if exterior horizontal surface does not get daylight, spaces do not get daylight.

    auto &illumMap = dl->illumMaps(MapNum);
    auto &daylFacHr = illumMap.daylFac[iHour];
    for (int iWinCover = 0; iWinCover < (int)WinCover::Num; ++iWinCover) {

        auto const &gilsk = dl->horIllum[iHour];
        auto const &edirsk = dl->dirIllum(iHour)[iWinCover];
        auto const &eintsk = dl->reflIllum(iHour)[iWinCover];
        auto &illSky = daylFacHr(loopwin, iMapPoint)[iWinCover];

        for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) { // Loop over sky types
            illSky.sky[iSky] = (gilsk.sky[iSky] > tmpDFCalc) ? ((edirsk.sky[iSky] + eintsk.sky[iSky]) / gilsk.sky[iSky]) : 0.0;
        } // for (iSky)

        if (dl->horIllum[iHour].sun > tmpDFCalc) {
            daylFacHr(loopwin, iMapPoint)[iWinCover].sun =
                (dl->dirIllum(iHour)[iWinCover].sun + dl->reflIllum(iHour)[iWinCover].sun) / (dl->horIllum[iHour].sun + 0.0001);
            daylFacHr(loopwin, iMapPoint)[iWinCover].sunDisk =
                (dl->dirIllum(iHour)[iWinCover].sunDisk + dl->reflIllum(iHour)[iWinCover].sunDisk) / (dl->horIllum[iHour].sun + 0.0001);
        } else {
            daylFacHr(loopwin, iMapPoint)[iWinCover].sun = 0.0;
            daylFacHr(loopwin, iMapPoint)[iWinCover].sunDisk = 0.0;
        }
    } // for (iWinCover)

    // For switchable glazing put daylighting factors for switched (dark) state in IS=2 location
    if (ICtrl > 0 && s_surf->WindowShadingControl(ICtrl).ShadingType == WinShadingType::SwitchableGlazing) {
        Real64 VTR = s_surf->SurfWinVisTransRatio(IWin); // ratio of Tvis of switched to unswitched state
        auto &illSky2 = daylFacHr(loopwin, iMapPoint)[iWinCover_Shaded];
        auto const &illSky1 = daylFacHr(loopwin, iMapPoint)[iWinCover_Bare];
        for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
            illSky2.sky[iSky] = illSky1.sky[iSky] * VTR;
        }

        daylFacHr(loopwin, iMapPoint)[iWinCover_Shaded].sun = daylFacHr(loopwin, iMapPoint)[iWinCover_Bare].sun * VTR;
        daylFacHr(loopwin, iMapPoint)[iWinCover_Shaded].sunDisk = daylFacHr(loopwin, iMapPoint)[iWinCover_Bare].sunDisk * VTR;
    } // ICtrl > 0
}

void GetDaylightingParametersInput(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   Oct 2004

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine provides a simple structure to get all daylighting
    // parameters.
    auto &dl = state.dataDayltg;
    auto &s_surf = state.dataSurface;

    if (!dl->getDaylightingParametersInputFlag) {
        return;
    }
    dl->getDaylightingParametersInputFlag = false;

    auto const &s_ipsc = state.dataIPShortCut;
    s_ipsc->cCurrentModuleObject = "Daylighting:Controls";
    bool ErrorsFound = false;
    int TotDaylightingControls = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
    if (TotDaylightingControls > 0) {
        dl->enclDaylight.allocate(state.dataViewFactor->NumOfSolarEnclosures);
        GetInputDayliteRefPt(state, ErrorsFound);
        GetDaylightingControls(state, ErrorsFound);
        GeometryTransformForDaylighting(state);
        GetInputIlluminanceMap(state, ErrorsFound);
        GetLightWellData(state, ErrorsFound);
        if (ErrorsFound) {
            ShowFatalError(state, "Program terminated for above reasons, related to DAYLIGHTING");
        }
        DayltgSetupAdjZoneListsAndPointers(state);
    }

    dl->maxNumRefPtInAnyDaylCtrl = 0;
    dl->maxNumRefPtInAnyEncl = 0;
    // Loop through all daylighting controls to find total reference points in each enclosure
    for (int daylightCtrlNum = 1; daylightCtrlNum <= (int)dl->daylightControl.size(); ++daylightCtrlNum) {
        int numRefPoints = dl->daylightControl(daylightCtrlNum).TotalDaylRefPoints;
        dl->maxNumRefPtInAnyDaylCtrl = max(numRefPoints, dl->maxNumRefPtInAnyDaylCtrl);
    }
    for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
        dl->maxNumRefPtInAnyEncl = max(state.dataViewFactor->EnclSolInfo(enclNum).TotalEnclosureDaylRefPoints, dl->maxNumRefPtInAnyEncl);
    }

    dl->maxEnclSubSurfaces = max(maxval(state.dataHeatBal->Zone, &DataHeatBalance::ZoneData::NumSubSurfaces),
                                 maxval(dl->enclDaylight, &EnclDaylightCalc::NumOfDayltgExtWins));

    for (int SurfNum : s_surf->AllHTWindowSurfaceList) {
        auto const &surf = s_surf->Surface(SurfNum);
        int const surfEnclNum = surf.SolarEnclIndex;
        int const numEnclRefPoints = state.dataViewFactor->EnclSolInfo(surfEnclNum).TotalEnclosureDaylRefPoints;
        auto &surfWin = s_surf->SurfaceWindow(SurfNum);
        if (numEnclRefPoints > 0) {
            if (!s_surf->SurfWinSurfDayLightInit(SurfNum)) {
                surfWin.refPts.allocate(numEnclRefPoints);
                for (auto &refPt : surfWin.refPts) {
                    new (&refPt) SurfaceWindowRefPt();
                }

                s_surf->SurfWinSurfDayLightInit(SurfNum) = true;
            }
        } else {
            int SurfNumAdj = surf.ExtBoundCond;
            if (SurfNumAdj > 0) {
                int const adjSurfEnclNum = s_surf->Surface(SurfNumAdj).SolarEnclIndex;
                int const numAdjEnclRefPoints = state.dataViewFactor->EnclSolInfo(adjSurfEnclNum).TotalEnclosureDaylRefPoints;
                if (numAdjEnclRefPoints > 0) {
                    if (!s_surf->SurfWinSurfDayLightInit(SurfNum)) {
                        surfWin.refPts.allocate(numAdjEnclRefPoints);
                        for (auto &refPt : surfWin.refPts) {
                            new (&refPt) SurfaceWindowRefPt();
                        }
                        s_surf->SurfWinSurfDayLightInit(SurfNum) = true;
                    }
                }
            }
        }

        if (surf.ExtBoundCond != ExternalEnvironment) {
            continue;
        }

        if (!surf.HasShadeControl) {
            continue;
        }

        auto &thisSurfEnclosure(state.dataViewFactor->EnclSolInfo(surf.SolarEnclIndex));
        if (s_surf->WindowShadingControl(surf.activeWindowShadingControl).GlareControlIsActive) {
            // Error if GlareControlIsActive but window is not in a Daylighting:Detailed zone
            if (thisSurfEnclosure.TotalEnclosureDaylRefPoints == 0) {
                ShowSevereError(state, format("Window={} has Window Shading Control with", surf.Name));
                ShowContinueError(state, "GlareControlIsActive = Yes but it is not in a Daylighting zone or enclosure.");
                ShowContinueError(state, format("Zone or enclosure indicated={}", state.dataViewFactor->EnclSolInfo(surf.SolarEnclIndex).Name));
                ErrorsFound = true;
            }
            // Error if GlareControlIsActive and window is in a Daylighting:Detailed zone/enclosure with
            // an interior window adjacent to another Daylighting:Detailed zone/enclosure
            if (thisSurfEnclosure.TotalEnclosureDaylRefPoints > 0) {
                for (int const intWin : thisSurfEnclosure.SurfacePtr) {
                    int const SurfNumAdj = s_surf->Surface(intWin).ExtBoundCond;
                    if (s_surf->Surface(intWin).Class == SurfaceClass::Window && SurfNumAdj > 0) {
                        auto &adjSurfEnclosure(state.dataViewFactor->EnclSolInfo(s_surf->Surface(SurfNumAdj).SolarEnclIndex));
                        if (adjSurfEnclosure.TotalEnclosureDaylRefPoints > 0) {
                            ShowSevereError(state, format("Window={} has Window Shading Control with", surf.Name));
                            ShowContinueError(state, "GlareControlIsActive = Yes and is in a Daylighting zone or enclosure");
                            ShowContinueError(state, "that shares an interior window with another Daylighting zone or enclosure");
                            ShowContinueError(state, format("Adjacent Zone or Enclosure indicated={}", adjSurfEnclosure.Name));
                            ErrorsFound = true;
                        }
                    }
                }
            }
        }

        if (s_surf->WindowShadingControl(surf.activeWindowShadingControl).shadingControlType != WindowShadingControlType::MeetDaylIlumSetp) {
            continue;
        }

        // Error if window has shadingControlType = MeetDaylightingIlluminanceSetpoint &
        // but is not in a Daylighting:Detailed zone
        if (thisSurfEnclosure.TotalEnclosureDaylRefPoints == 0) {
            ShowSevereError(state, format("Window={} has Window Shading Control with", surf.Name));
            ShowContinueError(state, "MeetDaylightingIlluminanceSetpoint but it is not in a Daylighting zone or enclosure.");
            ShowContinueError(state, format("Zone or enclosure indicated={}", thisSurfEnclosure.Name));
            ErrorsFound = true;
            continue;
        }

        // Error if window has shadingControlType = MeetDaylightIlluminanceSetpoint and is in a &
        // Daylighting:Detailed zone with an interior window adjacent to another Daylighting:Detailed zone
        for (int const intWin : thisSurfEnclosure.SurfacePtr) {
            int const SurfNumAdj = s_surf->Surface(intWin).ExtBoundCond;
            if (s_surf->Surface(intWin).Class == SurfaceClass::Window && SurfNumAdj > 0) {
                auto &adjSurfEnclosure(state.dataViewFactor->EnclSolInfo(s_surf->Surface(SurfNumAdj).SolarEnclIndex));
                if (adjSurfEnclosure.TotalEnclosureDaylRefPoints > 0) {
                    ShowSevereError(state, format("Window={} has Window Shading Control with", surf.Name));
                    ShowContinueError(state, "MeetDaylightIlluminanceSetpoint and is in a Daylighting zone or enclosure");
                    ShowContinueError(state, "that shares an interior window with another Daylighting zone or enclosure");
                    ShowContinueError(state, format("Adjacent Zone or enclosure indicated={}", adjSurfEnclosure.Name));
                    ErrorsFound = true;
                }
            }
        }
    } // for (SurfNum)

    if (!state.dataHeatBal->AnyAirBoundary) {
        for (int SurfLoop = 1; SurfLoop <= s_surf->TotSurfaces; ++SurfLoop) {
            auto const &surf = s_surf->Surface(SurfLoop);
            if (surf.Class != SurfaceClass::Window || !surf.ExtSolar) {
                continue;
            }

            int const enclOfSurf = surf.SolarEnclIndex;
            auto const &enclSol = state.dataViewFactor->EnclSolInfo(enclOfSurf);
            if (enclSol.TotalEnclosureDaylRefPoints == 0 || enclSol.HasInterZoneWindow || !dl->enclDaylight(enclOfSurf).hasSplitFluxDaylighting) {
                continue;
            }

            auto &surfWin = s_surf->SurfaceWindow(SurfLoop);
            for (int refPtNum = 1; refPtNum <= enclSol.TotalEnclosureDaylRefPoints; ++refPtNum) {
                auto &refPt = surfWin.refPts(refPtNum);
                SetupOutputVariable(state,
                                    format("Daylighting Window Reference Point {} Illuminance", refPtNum),
                                    Constant::Units::lux,
                                    refPt.illumFromWinRep,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);
                SetupOutputVariable(state,
                                    format("Daylighting Window Reference Point {} View Luminance", refPtNum),
                                    Constant::Units::cd_m2,
                                    refPt.lumWinRep,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surf.Name);
            }
        }
    } else {
        for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
            auto const &enclSol = state.dataViewFactor->EnclSolInfo(enclNum);
            for (int const enclSurfNum : enclSol.SurfacePtr) {
                auto const &surf = s_surf->Surface(enclSurfNum);
                auto &surfWindow = s_surf->SurfaceWindow(enclSurfNum);

                if (surf.Class != SurfaceClass::Window || !surf.ExtSolar) {
                    continue;
                }

                if (enclSol.TotalEnclosureDaylRefPoints == 0 || enclSol.HasInterZoneWindow) {
                    continue;
                }

                auto const &enclDayltg = dl->enclDaylight(enclNum);
                if (!enclDayltg.hasSplitFluxDaylighting) {
                    continue;
                }

                int refPtCount = 0;
                for (int controlNum : enclDayltg.daylightControlIndexes) {
                    auto const &control = dl->daylightControl(controlNum);
                    for (int refPtNum = 1; refPtNum <= control.TotalDaylRefPoints; ++refPtNum) {
                        ++refPtCount; // Count reference points across each daylighting control in the same enclosure
                        auto &refPt = surfWindow.refPts(refPtCount);
                        std::string varKey = format("{} to {}", surf.Name, state.dataDayltg->DaylRefPt(control.refPts(refPtNum).num).Name);
                        SetupOutputVariable(state,
                                            "Daylighting Window Reference Point Illuminance",
                                            Constant::Units::lux,
                                            refPt.illumFromWinRep,
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            varKey);
                        SetupOutputVariable(state,
                                            "Daylighting Window Reference Point View Luminance",
                                            Constant::Units::cd_m2,
                                            refPt.lumWinRep,
                                            OutputProcessor::TimeStepType::Zone,
                                            OutputProcessor::StoreType::Average,
                                            varKey);
                    }
                } // for (controlNum)
            } // for (enclSurfNum)
        } // for (enclNum)
    }

    // RJH DElight Modification Begin - Calls to DElight preprocessing subroutines
    if (doesDayLightingUseDElight(state)) {
        Real64 dLatitude = state.dataEnvrn->Latitude;
        DisplayString(state, "Calculating DElight Daylighting Factors");
        DElightManagerF::DElightInputGenerator(state);
        // Init Error Flag to 0 (no Warnings or Errors)
        DisplayString(state, "ReturnFrom DElightInputGenerator");
        int iErrorFlag = 0;
        DisplayString(state, "Calculating DElight DaylightCoefficients");
        DElightManagerF::GenerateDElightDaylightCoefficients(dLatitude, iErrorFlag);
        // Check Error Flag for Warnings or Errors returning from DElight
        // RJH 2008-03-07: open file for READWRITE and DELETE file after processing
        DisplayString(state, "ReturnFrom DElight DaylightCoefficients Calc");
        if (iErrorFlag != 0) {
            // Open DElight Daylight Factors Error File for reading
            auto iDElightErrorFile = state.files.outputDelightDfdmpFilePath.try_open(state.files.outputControl.delightdfdmp); // (THIS_AUTO_OK)

            // Sequentially read lines in DElight Daylight Factors Error File
            // and process them using standard EPlus warning/error handling calls
            // Process all error/warning messages first
            // Then, if any error has occurred, ShowFatalError to terminate processing
            bool bEndofErrFile = !iDElightErrorFile.good();
            std::string cErrorMsg; // Each DElight Error Message can be up to 200 characters long
            while (!bEndofErrFile) {
                auto cErrorLine = iDElightErrorFile.readLine(); // (THIS_AUTO_OK)
                if (cErrorLine.eof) {
                    bEndofErrFile = true;
                    continue;
                }
                // Is the current line a Warning message?
                if (has_prefix(cErrorLine.data, "WARNING: ")) {
                    cErrorMsg = cErrorLine.data.substr(9);
                    ShowWarningError(state, cErrorMsg);
                }
                // Is the current line an Error message?
                if (has_prefix(cErrorLine.data, "ERROR: ")) {
                    cErrorMsg = cErrorLine.data.substr(7);
                    ShowSevereError(state, cErrorMsg);
                    iErrorFlag = 1;
                }
            }

            // Close and Delete DElight Error File
            if (iDElightErrorFile.is_open()) {
                iDElightErrorFile.close();
                FileSystem::removeFile(iDElightErrorFile.filePath);
            }

            // If any DElight Error occurred then ShowFatalError to terminate
            if (iErrorFlag > 0) {
                ErrorsFound = true;
            }
        } else {
            if (FileSystem::fileExists(state.files.outputDelightDfdmpFilePath.filePath)) {
                FileSystem::removeFile(state.files.outputDelightDfdmpFilePath.filePath);
            }
        }
    }
    // RJH DElight Modification End - Calls to DElight preprocessing subroutines

    // TH 6/3/2010, added to report daylight factors
    s_ipsc->cCurrentModuleObject = "Output:DaylightFactors";
    int NumReports = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
    if (NumReports > 0) {
        int NumNames;
        int NumNumbers;
        int IOStat;
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 s_ipsc->cCurrentModuleObject,
                                                                 1,
                                                                 s_ipsc->cAlphaArgs,
                                                                 NumNames,
                                                                 s_ipsc->rNumericArgs,
                                                                 NumNumbers,
                                                                 IOStat,
                                                                 s_ipsc->lNumericFieldBlanks,
                                                                 s_ipsc->lAlphaFieldBlanks,
                                                                 s_ipsc->cAlphaFieldNames,
                                                                 s_ipsc->cNumericFieldNames);
        if (has_prefix(s_ipsc->cAlphaArgs(1), "SIZINGDAYS")) {
            dl->DFSReportSizingDays = true;
        } else if (has_prefix(s_ipsc->cAlphaArgs(1), "ALLSHADOWCALCULATIONDAYS")) {
            dl->DFSReportAllShadowCalculationDays = true;
        }
    }

    if (ErrorsFound) {
        ShowFatalError(state, "Program terminated for above reasons");
    }
} // FigureMapPointDayltgFactorsToAddIllums()

void GetInputIlluminanceMap(EnergyPlusData &state, bool &ErrorsFound)
{
    // Perform the GetInput function for the Output:IlluminanceMap
    // Glazer - June 2016 (moved from GetDaylightingControls)
    auto &dl = state.dataDayltg;
    auto const &s_surf = state.dataSurface;

    Array1D_bool ZoneMsgDone;

    Real64 CosBldgRelNorth = std::cos(-(state.dataHeatBal->BuildingAzimuth + state.dataHeatBal->BuildingRotationAppendixG) * Constant::DegToRad);
    Real64 SinBldgRelNorth = std::sin(-(state.dataHeatBal->BuildingAzimuth + state.dataHeatBal->BuildingRotationAppendixG) * Constant::DegToRad);
    // these are only for Building Rotation for Appendix G when using world coordinate system
    Real64 CosBldgRotAppGonly = std::cos(-state.dataHeatBal->BuildingRotationAppendixG * Constant::DegToRad);
    Real64 SinBldgRotAppGonly = std::sin(-state.dataHeatBal->BuildingRotationAppendixG * Constant::DegToRad);

    bool doTransform = false;
    Real64 OldAspectRatio = 1.0;
    Real64 NewAspectRatio = 1.0;

    CheckForGeometricTransform(state, doTransform, OldAspectRatio, NewAspectRatio);

    auto const &s_ipsc = state.dataIPShortCut;
    s_ipsc->cCurrentModuleObject = "Output:IlluminanceMap";
    int TotIllumMaps = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

    dl->illumMaps.allocate(TotIllumMaps);

    if (TotIllumMaps > 0) {
        int IOStat;
        int NumAlpha;
        int NumNumber;
        auto &ip = state.dataInputProcessing->inputProcessor;
        for (int MapNum = 1; MapNum <= TotIllumMaps; ++MapNum) {
            ip->getObjectItem(state,
                              s_ipsc->cCurrentModuleObject,
                              MapNum,
                              s_ipsc->cAlphaArgs,
                              NumAlpha,
                              s_ipsc->rNumericArgs,
                              NumNumber,
                              IOStat,
                              s_ipsc->lNumericFieldBlanks,
                              s_ipsc->lAlphaFieldBlanks,
                              s_ipsc->cAlphaFieldNames,
                              s_ipsc->cNumericFieldNames);

            auto &illumMap = dl->illumMaps(MapNum);
            illumMap.Name = s_ipsc->cAlphaArgs(1);
            int const zoneNum = Util::FindItemInList(s_ipsc->cAlphaArgs(2), state.dataHeatBal->Zone);
            if (zoneNum > 0) {
                illumMap.zoneIndex = zoneNum;
                // set enclosure index for first space in zone
                int enclNum = state.dataHeatBal->space(state.dataHeatBal->Zone(zoneNum).spaceIndexes(1)).solarEnclosureNum;
                illumMap.enclIndex = enclNum;
                // check that all spaces in the zone are in the same enclosure
                for (int spaceCounter = 2; spaceCounter <= state.dataHeatBal->Zone(zoneNum).numSpaces; ++spaceCounter) {
                    int spaceNum = state.dataHeatBal->Zone(zoneNum).spaceIndexes(spaceCounter);
                    if (enclNum != state.dataHeatBal->space(spaceNum).solarEnclosureNum) {
                        ShowSevereError(state,
                                        format("{}=\"{}\" All spaces in the zone must be in the same enclosure for daylighting illuminance maps.",
                                               s_ipsc->cCurrentModuleObject,
                                               s_ipsc->cAlphaArgs(1)));
                        ShowContinueError(
                            state, format("Zone=\"{}\" spans multiple enclosures. Use a Space Name instead.", state.dataHeatBal->Zone(zoneNum).Name));
                        ErrorsFound = true;
                        break;
                    }
                }
            } else {
                int const spaceNum = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(2), state.dataHeatBal->space);
                if (spaceNum == 0) {
                    ShowSevereError(state,
                                    format("{}=\"{}\", invalid {}=\"{}\".",
                                           s_ipsc->cCurrentModuleObject,
                                           s_ipsc->cAlphaArgs(1),
                                           s_ipsc->cAlphaFieldNames(2),
                                           s_ipsc->cAlphaArgs(2)));
                    ErrorsFound = true;
                } else {
                    illumMap.spaceIndex = spaceNum;
                    illumMap.zoneIndex = state.dataHeatBal->space(spaceNum).zoneNum;
                    illumMap.enclIndex = state.dataHeatBal->space(spaceNum).solarEnclosureNum;
                    assert(illumMap.enclIndex > 0);
                }
            }

            illumMap.Z = s_ipsc->rNumericArgs(1);
            illumMap.Xmin = s_ipsc->rNumericArgs(2);
            illumMap.Xmax = s_ipsc->rNumericArgs(3);
            if (s_ipsc->rNumericArgs(2) > s_ipsc->rNumericArgs(3)) {
                ShowSevereError(state, format("{}=\"{}\", invalid entry.", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state,
                                  format("...{} {:.2R} must be <= {} {:.2R}.",
                                         s_ipsc->cNumericFieldNames(2),
                                         s_ipsc->rNumericArgs(2),
                                         s_ipsc->cNumericFieldNames(3),
                                         s_ipsc->rNumericArgs(3)));
                ErrorsFound = true;
            }
            illumMap.Xnum = s_ipsc->rNumericArgs(4);
            illumMap.Xinc = (illumMap.Xnum != 1) ? ((illumMap.Xmax - illumMap.Xmin) / (illumMap.Xnum - 1)) : 0.0;

            illumMap.Ymin = s_ipsc->rNumericArgs(5);
            illumMap.Ymax = s_ipsc->rNumericArgs(6);
            if (s_ipsc->rNumericArgs(5) > s_ipsc->rNumericArgs(6)) {
                ShowSevereError(state, format("{}=\"{}\", invalid entry.", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state,
                                  format("...{} {:.2R} must be <= {} {:.2R}.",
                                         s_ipsc->cNumericFieldNames(5),
                                         s_ipsc->rNumericArgs(5),
                                         s_ipsc->cNumericFieldNames(6),
                                         s_ipsc->rNumericArgs(6)));
                ErrorsFound = true;
            }
            illumMap.Ynum = s_ipsc->rNumericArgs(7);
            illumMap.Yinc = (illumMap.Ynum != 1) ? ((illumMap.Ymax - illumMap.Ymin) / (illumMap.Ynum - 1)) : 0.0;

            if (illumMap.Xnum * illumMap.Ynum > MaxMapRefPoints) {
                ShowSevereError(state, format("{}=\"{}\", too many map points specified.", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state,
                                  format("...{}[{}] * {}[{}].= [{}] must be <= [{}].",
                                         s_ipsc->cNumericFieldNames(4),
                                         illumMap.Xnum,
                                         s_ipsc->cNumericFieldNames(7),
                                         illumMap.Ynum,
                                         illumMap.Xnum * illumMap.Ynum,
                                         MaxMapRefPoints));
                ErrorsFound = true;
            }
        } // MapNum
        s_ipsc->cCurrentModuleObject = "OutputControl:IlluminanceMap:Style";
        int MapStyleIn = ip->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

        if (MapStyleIn == 0) {
            s_ipsc->cAlphaArgs(1) = "COMMA";
            dl->MapColSep = DataStringGlobals::CharComma; // comma
        } else if (MapStyleIn == 1) {
            ip->getObjectItem(state,
                              s_ipsc->cCurrentModuleObject,
                              1,
                              s_ipsc->cAlphaArgs,
                              NumAlpha,
                              s_ipsc->rNumericArgs,
                              NumNumber,
                              IOStat,
                              s_ipsc->lNumericFieldBlanks,
                              s_ipsc->lAlphaFieldBlanks,
                              s_ipsc->cAlphaFieldNames,
                              s_ipsc->cNumericFieldNames);
            if (s_ipsc->cAlphaArgs(1) == "COMMA") {
                dl->MapColSep = DataStringGlobals::CharComma; // comma
            } else if (s_ipsc->cAlphaArgs(1) == "TAB") {
                dl->MapColSep = DataStringGlobals::CharTab; // tab
            } else if (s_ipsc->cAlphaArgs(1) == "FIXED" || s_ipsc->cAlphaArgs(1) == "SPACE") {
                dl->MapColSep = DataStringGlobals::CharSpace; // space
            } else {
                dl->MapColSep = DataStringGlobals::CharComma; // comma
                ShowWarningError(state,
                                 format("{}: invalid {}=\"{}\", Commas will be used to separate fields.",
                                        s_ipsc->cCurrentModuleObject,
                                        s_ipsc->cAlphaFieldNames(1),
                                        s_ipsc->cAlphaArgs(1)));
                s_ipsc->cAlphaArgs(1) = "COMMA";
            }
        }
        print(state.files.eio, "! <Daylighting:Illuminance Maps>,#Maps,Style\n");
        ConvertCaseToLower(s_ipsc->cAlphaArgs(1), s_ipsc->cAlphaArgs(2));
        s_ipsc->cAlphaArgs(1).erase(1);
        s_ipsc->cAlphaArgs(1) += s_ipsc->cAlphaArgs(2).substr(1);
        print(state.files.eio, "Daylighting:Illuminance Maps,{},{}\n", TotIllumMaps, s_ipsc->cAlphaArgs(1));
    }

    // Check for illuminance maps associated with this zone
    for (auto &illumMap : dl->illumMaps) {

        if (illumMap.zoneIndex == 0) {
            continue;
        }

        auto &zone = state.dataHeatBal->Zone(illumMap.zoneIndex);
        // Calc cos and sin of Zone Relative North values for later use in transforming Reference Point coordinates
        Real64 CosZoneRelNorth = std::cos(-zone.RelNorth * Constant::DegToRad);
        Real64 SinZoneRelNorth = std::sin(-zone.RelNorth * Constant::DegToRad);

        if (illumMap.Xnum * illumMap.Ynum == 0) {
            continue;
        }

        // Add additional daylighting reference points for map
        illumMap.TotalMapRefPoints = illumMap.Xnum * illumMap.Ynum;
        illumMap.refPts.allocate(illumMap.TotalMapRefPoints);
        for (auto &refPt : illumMap.refPts) {
            new (&refPt) DaylMapPt();
        }

        if (illumMap.TotalMapRefPoints > MaxMapRefPoints) {
            ShowSevereError(state, "GetDaylighting Parameters: Total Map Reference points entered is greater than maximum allowed.");
            ShowContinueError(state, format("Occurs in Zone={}", zone.Name));
            ShowContinueError(state,
                              format("Maximum reference points allowed={}, entered amount ( when error first occurred )={}",
                                     MaxMapRefPoints,
                                     illumMap.TotalMapRefPoints));
            ErrorsFound = true;
            break;
        }

        // Calc cos and sin of Zone Relative North values for later use in transforming Map Point coordinates
        // CosZoneRelNorth = std::cos( -zone.RelNorth * DegToRadians ); //Tuned These should not be changing
        // SinZoneRelNorth = std::sin( -zone.RelNorth * DegToRadians );
        illumMap.Xinc = (illumMap.Xnum != 1) ? ((illumMap.Xmax - illumMap.Xmin) / (illumMap.Xnum - 1)) : 0.0;
        illumMap.Yinc = (illumMap.Ynum != 1) ? ((illumMap.Ymax - illumMap.Ymin) / (illumMap.Ynum - 1)) : 0.0;

        // Map points and increments are stored in AbsCoord and then that is operated on if relative coords entered.
        for (int Y = 1; Y <= illumMap.Ynum; ++Y) {
            for (int X = 1; X <= illumMap.Xnum; ++X) {
                int iRefPt = (Y - 1) * illumMap.Xnum + X;
                auto &refPt = illumMap.refPts(iRefPt);
                refPt.absCoords = {illumMap.Xmin + (X - 1) * illumMap.Xinc, illumMap.Ymin + (Y - 1) * illumMap.Yinc, illumMap.Z};
            }
        }

        for (int Y = 1; Y <= illumMap.Ynum; ++Y) {
            for (int X = 1; X <= illumMap.Xnum; ++X) {
                int iRefPt = (Y - 1) * illumMap.Xnum + X;
                auto &refPt = illumMap.refPts(iRefPt);

                if (!s_surf->DaylRefWorldCoordSystem) {
                    Real64 Xb = refPt.absCoords.x * CosZoneRelNorth - refPt.absCoords.y * SinZoneRelNorth + zone.OriginX;
                    Real64 Yb = refPt.absCoords.x * SinZoneRelNorth + refPt.absCoords.y * CosZoneRelNorth + zone.OriginY;
                    refPt.absCoords.x = Xb * CosBldgRelNorth - Yb * SinBldgRelNorth;
                    refPt.absCoords.y = Xb * SinBldgRelNorth + Yb * CosBldgRelNorth;
                    refPt.absCoords.z += zone.OriginZ;
                    if (doTransform) {
                        Real64 Xo = refPt.absCoords.x; // world coordinates.... shifted by relative north angle...
                        Real64 Yo = refPt.absCoords.y;
                        // next derotate the building
                        Real64 XnoRot = Xo * CosBldgRelNorth + Yo * SinBldgRelNorth;
                        Real64 YnoRot = Yo * CosBldgRelNorth - Xo * SinBldgRelNorth;
                        // translate
                        Real64 Xtrans = XnoRot * std::sqrt(NewAspectRatio / OldAspectRatio);
                        Real64 Ytrans = YnoRot * std::sqrt(OldAspectRatio / NewAspectRatio);
                        // rerotate
                        refPt.absCoords.x = Xtrans * CosBldgRelNorth - Ytrans * SinBldgRelNorth;

                        refPt.absCoords.y = Xtrans * SinBldgRelNorth + Ytrans * CosBldgRelNorth;
                    }
                } else {
                    Real64 Xb = refPt.absCoords.x;
                    Real64 Yb = refPt.absCoords.y;
                    refPt.absCoords.x = Xb * CosBldgRotAppGonly - Yb * SinBldgRotAppGonly;
                    refPt.absCoords.y = Xb * SinBldgRotAppGonly + Yb * CosBldgRotAppGonly;
                }
                if (iRefPt == 1) {
                    illumMap.Xmin = refPt.absCoords.x;
                    illumMap.Ymin = refPt.absCoords.y;
                    illumMap.Xmax = refPt.absCoords.x;
                    illumMap.Ymax = refPt.absCoords.y;
                    illumMap.Z = refPt.absCoords.z;
                }
                illumMap.Xmin = min(illumMap.Xmin, refPt.absCoords.x);
                illumMap.Ymin = min(illumMap.Ymin, refPt.absCoords.y);
                illumMap.Xmax = max(illumMap.Xmax, refPt.absCoords.x);
                illumMap.Ymax = max(illumMap.Ymax, refPt.absCoords.y);
                if ((refPt.absCoords.x < zone.MinimumX && (zone.MinimumX - refPt.absCoords.x) > 0.001) ||
                    (refPt.absCoords.x > zone.MaximumX && (refPt.absCoords.x - zone.MaximumX) > 0.001) ||
                    (refPt.absCoords.y < zone.MinimumY && (zone.MinimumY - refPt.absCoords.y) > 0.001) ||
                    (refPt.absCoords.y > zone.MaximumY && (refPt.absCoords.y - zone.MaximumY) > 0.001) ||
                    (refPt.absCoords.z < zone.MinimumZ && (zone.MinimumZ - refPt.absCoords.z) > 0.001) ||
                    (refPt.absCoords.z > zone.MaximumZ && (refPt.absCoords.z - zone.MaximumZ) > 0.001)) {
                    refPt.inBounds = false;
                }

                // Test extremes of Map Points against Zone Min/Max
                if (iRefPt != 1 && iRefPt != illumMap.TotalMapRefPoints) {
                    continue;
                }

                if (refPt.inBounds) {
                    continue;
                }

                if (refPt.absCoords.x < zone.MinimumX || refPt.absCoords.x > zone.MaximumX) {
                    ShowWarningError(
                        state,
                        format("GetInputIlluminanceMap: Reference Map point #[{}], X Value outside Zone Min/Max X, Zone={}", iRefPt, zone.Name));
                    ShowContinueError(state,
                                      format("...X Reference Point= {:.2R}, Zone Minimum X= {:.2R}, Zone Maximum X= {:.2R}",
                                             refPt.absCoords.x,
                                             zone.MinimumX,
                                             zone.MaximumX));
                    ShowContinueError(
                        state,
                        format("...X Reference Distance Outside MinimumX= {:.4R} m.",
                               (refPt.absCoords.x < zone.MinimumX) ? (zone.MinimumX - refPt.absCoords.x) : (refPt.absCoords.x - zone.MaximumX)));
                }
                if (refPt.absCoords.y < zone.MinimumY || refPt.absCoords.y > zone.MaximumY) {
                    ShowWarningError(
                        state,
                        format("GetInputIlluminanceMap: Reference Map point #[{}], Y Value outside Zone Min/Max Y, Zone={}", iRefPt, zone.Name));
                    ShowContinueError(state,
                                      format("...Y Reference Point= {:.2R}, Zone Minimum Y= {:.2R}, Zone Maximum Y= {:.2R}",
                                             refPt.absCoords.y,
                                             zone.MinimumY,
                                             zone.MaximumY));
                    ShowContinueError(
                        state,
                        format("...Y Reference Distance Outside MinimumY= {:.4R} m.",
                               (refPt.absCoords.y < zone.MinimumY) ? (zone.MinimumY - refPt.absCoords.y) : (refPt.absCoords.y - zone.MaximumY)));
                }
                if (refPt.absCoords.z < zone.MinimumZ || refPt.absCoords.z > zone.MaximumZ) {
                    ShowWarningError(
                        state,
                        format("GetInputIlluminanceMap: Reference Map point #[{}], Z Value outside Zone Min/Max Z, Zone={}", iRefPt, zone.Name));
                    ShowContinueError(state,
                                      format("...Z Reference Point= {:.2R}, Zone Minimum Z= {:.2R}, Zone Maximum Z= {:.2R}",
                                             refPt.absCoords.z,
                                             zone.MinimumZ,
                                             zone.MaximumZ));
                    ShowContinueError(
                        state,
                        format("...Z Reference Distance Outside MinimumZ= {:.4R} m.",
                               (refPt.absCoords.z < zone.MinimumZ) ? (zone.MinimumZ - refPt.absCoords.z) : (refPt.absCoords.z - zone.MaximumZ)));
                }
            } // for (X)
        } // for (Y)
    } // for (MapNum)

    ZoneMsgDone.dimension(state.dataGlobal->NumOfZones, false);
    for (auto const &illumMap : dl->illumMaps) {
        if (illumMap.zoneIndex == 0) {
            continue;
        }
        int enclNum = illumMap.enclIndex;
        if (!dl->enclDaylight(enclNum).hasSplitFluxDaylighting && !ZoneMsgDone(illumMap.zoneIndex)) {
            ShowSevereError(state,
                            format("Zone Name in Output:IlluminanceMap is not used for Daylighting:Controls={}",
                                   state.dataHeatBal->Zone(illumMap.zoneIndex).Name));
            ErrorsFound = true;
        }
    }
    ZoneMsgDone.deallocate();
    if (ErrorsFound) {
        return;
    }

    if (TotIllumMaps > 0) {
        print(state.files.eio,
              "! <Daylighting:Illuminance Maps:Detail>,Name,Zone,XMin {{m}},XMax {{m}},Xinc {{m}},#X Points,YMin "
              "{{m}},YMax {{m}},Yinc {{m}},#Y Points,Z {{m}}\n");
    }
    for (auto const &illumMap : dl->illumMaps) {
        print(state.files.eio,
              "Daylighting:Illuminance Maps:Detail,{},{},{:.2R},{:.2R},{:.2R},{},{:.2R},{:.2R},{:.2R},{},{:.2R}\n",
              illumMap.Name,
              state.dataHeatBal->Zone(illumMap.zoneIndex).Name,
              illumMap.Xmin,
              illumMap.Xmax,
              illumMap.Xinc,
              illumMap.Xnum,
              illumMap.Ymin,
              illumMap.Ymax,
              illumMap.Yinc,
              illumMap.Ynum,
              illumMap.Z);
    }

} // GetInputIlluminanceMap()

void GetDaylightingControls(EnergyPlusData &state, bool &ErrorsFound)
{
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   March 2002
    //       MODIFIED       Glazer - July 2016 - Move geometry transformation portion, rearrange input, allow more than three reference points
    // Obtain the user input data for Daylighting:Controls object in the input file.

    static constexpr std::string_view routineName = "GetDaylightingControls";

    auto &dl = state.dataDayltg;

    int IOStat;
    int NumAlpha;
    int NumNumber;

    // Smallest deviation from unity for the sum of all fractions
    // Accept approx 4 to 8 ULP error (technically abs(1.0 + sumFracs) should be close to 2)
    //   constexpr Real64 FractionTolerance(4 * std::numeric_limits<Real64>::epsilon());
    // Instead, we use a 0.001 = 0.1% tolerance
    constexpr Real64 FractionTolerance(0.001);

    auto &ip = state.dataInputProcessing->inputProcessor;
    auto const &s_ipsc = state.dataIPShortCut;
    s_ipsc->cCurrentModuleObject = "Daylighting:Controls";
    int totDaylightingControls = ip->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
    dl->daylightControl.allocate(totDaylightingControls);
    Array1D<bool> spaceHasDaylightingControl;
    spaceHasDaylightingControl.dimension(state.dataGlobal->numSpaces, false);
    // Reset to zero in case this is called more than once in unit tests
    for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
        state.dataViewFactor->EnclSolInfo(enclNum).TotalEnclosureDaylRefPoints = 0;
    }
    for (int controlNum = 1; controlNum <= totDaylightingControls; ++controlNum) {
        s_ipsc->cAlphaArgs = "";
        s_ipsc->rNumericArgs = 0.0;
        ip->getObjectItem(state,
                          s_ipsc->cCurrentModuleObject,
                          controlNum,
                          s_ipsc->cAlphaArgs,
                          NumAlpha,
                          s_ipsc->rNumericArgs,
                          NumNumber,
                          IOStat,
                          s_ipsc->lNumericFieldBlanks,
                          s_ipsc->lAlphaFieldBlanks,
                          s_ipsc->cAlphaFieldNames,
                          s_ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{routineName, s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(1)};

        auto &daylightControl = dl->daylightControl(controlNum);
        daylightControl.Name = s_ipsc->cAlphaArgs(1);

        // Is it a zone or space name?
        int const zoneNum = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(2), state.dataHeatBal->Zone);
        if (zoneNum > 0) {
            daylightControl.zoneIndex = zoneNum;
            // set enclosure index for first space in zone
            int enclNum = state.dataHeatBal->space(state.dataHeatBal->Zone(zoneNum).spaceIndexes(1)).solarEnclosureNum;
            daylightControl.enclIndex = enclNum;
            // check that all spaces in the zone are in the same enclosure
            for (int spaceCounter = 2; spaceCounter <= state.dataHeatBal->Zone(zoneNum).numSpaces; ++spaceCounter) {
                int zoneSpaceNum = state.dataHeatBal->Zone(zoneNum).spaceIndexes(spaceCounter);
                if (daylightControl.enclIndex != state.dataHeatBal->space(zoneSpaceNum).solarEnclosureNum) {
                    ShowSevereError(state,
                                    format("{}: invalid {}=\"{}\" All spaces in the zone must be in the same enclosure for daylighting.",
                                           s_ipsc->cCurrentModuleObject,
                                           s_ipsc->cAlphaFieldNames(2),
                                           s_ipsc->cAlphaArgs(2)));
                    ErrorsFound = true;
                    break;
                }
            }
            for (int zoneSpaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                // Check if this is a duplicate
                if (spaceHasDaylightingControl(zoneSpaceNum)) {
                    ShowWarningError(state,
                                     format("{}=\"{}\" Space=\"{}\" already has a {} object assigned to it.",
                                            s_ipsc->cCurrentModuleObject,
                                            daylightControl.Name,
                                            state.dataHeatBal->space(zoneSpaceNum).Name,
                                            s_ipsc->cCurrentModuleObject));
                    ShowContinueError(state, "This control will override the lighting power factor for this space.");
                }
                spaceHasDaylightingControl(zoneSpaceNum) = true;
            }
        } else {
            int const spaceNum = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(2), state.dataHeatBal->space);
            if (spaceNum == 0) {
                ShowSevereError(state,
                                format("{}: invalid {}=\"{}\".", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaFieldNames(2), s_ipsc->cAlphaArgs(2)));
                ErrorsFound = true;
                continue;
            } else {
                daylightControl.spaceIndex = spaceNum;
                daylightControl.zoneIndex = state.dataHeatBal->space(spaceNum).zoneNum;
                daylightControl.enclIndex = state.dataHeatBal->space(spaceNum).solarEnclosureNum;
                // Check if this is a duplicate
                if (spaceHasDaylightingControl(spaceNum)) {
                    ShowWarningError(state,
                                     format("{}=\"{}\" Space=\"{}\" already has a {} object assigned to it.",
                                            s_ipsc->cCurrentModuleObject,
                                            daylightControl.Name,
                                            state.dataHeatBal->space(spaceNum).Name,
                                            s_ipsc->cCurrentModuleObject));
                    ShowContinueError(state, "This control will override the lighting power factor for this space.");
                }
                spaceHasDaylightingControl(spaceNum) = true;
            }
        }

        dl->enclDaylight(daylightControl.enclIndex).daylightControlIndexes.emplace_back(controlNum);
        daylightControl.ZoneName = state.dataHeatBal->Zone(daylightControl.zoneIndex).Name;

        if (s_ipsc->lAlphaFieldBlanks(3)) {
            daylightControl.DaylightMethod = DaylightingMethod::SplitFlux;
        } else {
            daylightControl.DaylightMethod =
                static_cast<DaylightingMethod>(getEnumValue(DaylightingMethodNamesUC, Util::makeUPPER(s_ipsc->cAlphaArgs(3))));

            if (daylightControl.DaylightMethod == DaylightingMethod::Invalid) {
                daylightControl.DaylightMethod = DaylightingMethod::SplitFlux;
                ShowWarningError(state,
                                 format("Invalid {} = {}, occurs in {}object for {}=\"{}",
                                        s_ipsc->cAlphaFieldNames(3),
                                        s_ipsc->cAlphaArgs(3),
                                        s_ipsc->cCurrentModuleObject,
                                        s_ipsc->cCurrentModuleObject,
                                        s_ipsc->cAlphaArgs(1)));
                ShowContinueError(state, "SplitFlux assumed, and the simulation continues.");
            }
        }
        dl->enclDaylight(daylightControl.enclIndex).hasSplitFluxDaylighting |= (daylightControl.DaylightMethod == DaylightingMethod::SplitFlux);

        if (s_ipsc->lAlphaFieldBlanks(4)) { // Field: Availability Schedule Name
            daylightControl.availSched = Sched::GetScheduleAlwaysOn(state);
        } else if ((daylightControl.availSched = Sched::GetSchedule(state, s_ipsc->cAlphaArgs(4))) == nullptr) {
            ShowWarningItemNotFound(state,
                                    eoh,
                                    s_ipsc->cAlphaFieldNames(4),
                                    s_ipsc->cAlphaArgs(4),
                                    "Schedule was not found so controls will always be available, and the simulation continues.");
            daylightControl.availSched = Sched::GetScheduleAlwaysOn(state);
        }

        daylightControl.LightControlType = static_cast<LtgCtrlType>(getEnumValue(LtgCtrlTypeNamesUC, s_ipsc->cAlphaArgs(5)));
        if (daylightControl.LightControlType == LtgCtrlType::Invalid) {
            ShowWarningInvalidKey(
                state, eoh, s_ipsc->cAlphaFieldNames(5), s_ipsc->cAlphaArgs(5), "Continuous assumed, and the simulation continues.");
        }

        daylightControl.MinPowerFraction = s_ipsc->rNumericArgs(1);  // Field: Minimum Input Power Fraction for Continuous Dimming Control
        daylightControl.MinLightFraction = s_ipsc->rNumericArgs(2);  // Field: Minimum Light Output Fraction for Continuous Dimming Control
        daylightControl.LightControlSteps = s_ipsc->rNumericArgs(3); // Field: Number of Stepped Control Steps
        daylightControl.LightControlProbability =
            s_ipsc->rNumericArgs(4); // Field: Probability Lighting will be Reset When Needed in Manual Stepped Control

        if (!s_ipsc->lAlphaFieldBlanks(6)) { // Field: Glare Calculation Daylighting Reference Point Name
            daylightControl.glareRefPtNumber = Util::FindItemInList(s_ipsc->cAlphaArgs(6),
                                                                    dl->DaylRefPt,
                                                                    &RefPointData::Name); // Field: Glare Calculation Daylighting Reference Point Name
            if (daylightControl.glareRefPtNumber == 0) {
                ShowSevereError(state,
                                format("{}: invalid {}=\"{}\" for object named: {}",
                                       s_ipsc->cCurrentModuleObject,
                                       s_ipsc->cAlphaFieldNames(6),
                                       s_ipsc->cAlphaArgs(6),
                                       s_ipsc->cAlphaArgs(1)));
                ErrorsFound = true;
                continue;
            }
        } else if (daylightControl.DaylightMethod == DaylightingMethod::SplitFlux) {
            ShowWarningError(state, format("No {} provided for object named: {}", s_ipsc->cAlphaFieldNames(6), s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, "No glare calculation performed, and the simulation continues.");
        }

        // Field: Glare Calculation Azimuth Angle of View Direction Clockwise from Zone y-Axis
        daylightControl.ViewAzimuthForGlare = !s_ipsc->lNumericFieldBlanks(5) ? s_ipsc->rNumericArgs(5) : 0.0;

        daylightControl.MaxGlareallowed = s_ipsc->rNumericArgs(6);           // Field: Maximum Allowable Discomfort Glare Index
        daylightControl.DElightGriddingResolution = s_ipsc->rNumericArgs(7); // Field: DElight Gridding Resolution

        int curTotalDaylRefPts = NumAlpha - 6; // first six alpha fields are not part of extensible group
        daylightControl.TotalDaylRefPoints = curTotalDaylRefPts;
        state.dataViewFactor->EnclSolInfo(daylightControl.enclIndex).TotalEnclosureDaylRefPoints += curTotalDaylRefPts;
        dl->ZoneDaylight(daylightControl.zoneIndex).totRefPts += curTotalDaylRefPts;
        dl->maxControlRefPoints = max(dl->maxControlRefPoints, curTotalDaylRefPts);
        if ((NumNumber - 7) / 2 != daylightControl.TotalDaylRefPoints) {
            ShowSevereError(state,
                            format("{}The number of extensible numeric fields and alpha fields is inconsistent for: {}",
                                   s_ipsc->cCurrentModuleObject,
                                   s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state,
                              format("For each field: {} there needs to be the following fields: Fraction Controlled by Reference Point and "
                                     "Illuminance Setpoint at Reference Point",
                                     s_ipsc->cAlphaFieldNames(NumAlpha)));
            ErrorsFound = true;
        }

        daylightControl.refPts.allocate(curTotalDaylRefPts);

        for (auto &refPt : daylightControl.refPts) {
            refPt = DaylRefPt();
        }

        int countRefPts = 0;
        for (int refPtNum = 1; refPtNum <= curTotalDaylRefPts; ++refPtNum) {
            auto &refPt = daylightControl.refPts(refPtNum);
            refPt.num =
                Util::FindItemInList(s_ipsc->cAlphaArgs(6 + refPtNum), dl->DaylRefPt, &RefPointData::Name); // Field: Daylighting Reference Point Name
            if (refPt.num == 0) {
                ShowSevereError(state,
                                format("{}: invalid {}=\"{}\" for object named: {}",
                                       s_ipsc->cCurrentModuleObject,
                                       s_ipsc->cAlphaFieldNames(6 + refPtNum),
                                       s_ipsc->cAlphaArgs(6 + refPtNum),
                                       s_ipsc->cAlphaArgs(1)));
                ErrorsFound = true;
                continue;
            } else {
                ++countRefPts;
            }
            refPt.fracZoneDaylit = s_ipsc->rNumericArgs(6 + refPtNum * 2); // Field: Fraction Controlled by Reference Point
            refPt.illumSetPoint = s_ipsc->rNumericArgs(7 + refPtNum * 2);  // Field: Illuminance Setpoint at Reference Point

            if (daylightControl.DaylightMethod == DaylightingMethod::SplitFlux) {
                SetupOutputVariable(state,
                                    format("Daylighting Reference Point {} Illuminance", refPtNum),
                                    Constant::Units::lux,
                                    refPt.lums[iLum_Illum],
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    daylightControl.Name);
                SetupOutputVariable(state,
                                    format("Daylighting Reference Point {} Daylight Illuminance Setpoint Exceeded Time", refPtNum),
                                    Constant::Units::hr,
                                    refPt.timeExceedingDaylightIlluminanceSetPoint,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    daylightControl.Name);
                SetupOutputVariable(state,
                                    format("Daylighting Reference Point {} Glare Index", refPtNum),
                                    Constant::Units::None,
                                    refPt.glareIndex,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    daylightControl.Name);
                SetupOutputVariable(state,
                                    format("Daylighting Reference Point {} Glare Index Setpoint Exceeded Time", refPtNum),
                                    Constant::Units::hr,
                                    refPt.timeExceedingGlareIndexSetPoint,
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    daylightControl.Name);
            } // if (DaylightMethod == SplitFlux)
        } // for (RefPtNum)

        // Register Error if 0 DElight RefPts have been input for valid DElight object
        if (countRefPts < 1) {
            ShowSevereError(state, format("No Reference Points input for {} zone ={}", s_ipsc->cCurrentModuleObject, daylightControl.ZoneName));
            ErrorsFound = true;
        }

        Real64 sumFracs = 0.0;
        for (auto const &refPt : daylightControl.refPts) {
            sumFracs += refPt.fracZoneDaylit;
        }

        daylightControl.sumFracLights = sumFracs;
        if ((1.0 - sumFracs) > FractionTolerance) {
            ShowWarningError(state, "GetDaylightingControls: Fraction of zone or space controlled by the Daylighting reference points is < 1.0.");
            ShowContinueError(state,
                              format("..discovered in {}=\"{}\", only {:.3R} of the zone or space is controlled.",
                                     s_ipsc->cCurrentModuleObject,
                                     daylightControl.Name,
                                     sumFracs));
        } else if ((sumFracs - 1.0) > FractionTolerance) {
            ShowSevereError(state, "GetDaylightingControls: Fraction of zone or space controlled by the Daylighting reference points is > 1.0.");
            ShowContinueError(state,
                              format("..discovered in {}=\"{}\", trying to control {:.3R} of the zone or space.",
                                     s_ipsc->cCurrentModuleObject,
                                     daylightControl.Name,
                                     sumFracs));
            ErrorsFound = true;
        }

        if (daylightControl.LightControlType == LtgCtrlType::Stepped && daylightControl.LightControlSteps <= 0) {
            ShowWarningError(state, "GetDaylightingControls: For Stepped Control, the number of steps must be > 0");
            ShowContinueError(state,
                              format("..discovered in \"{}\" for Zone=\"{}\", will use 1", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaArgs(2)));
            daylightControl.LightControlSteps = 1;
        }
        SetupOutputVariable(state,
                            "Daylighting Lighting Power Multiplier",
                            Constant::Units::None,
                            daylightControl.PowerReductionFactor,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            daylightControl.Name);
    } // for (controlNum)
} // GetDaylightingControls()

void GeometryTransformForDaylighting(EnergyPlusData &state)
{
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   March 2002
    //       MODIFIED       Glazer - July 2016 - separated this from GetInput function
    // For splitflux daylighting, transform the geometry
    auto &dl = state.dataDayltg;
    auto const &s_surf = state.dataSurface;

    // Calc cos and sin of Building Relative North values for later use in transforming Reference Point coordinates
    Real64 CosBldgRelNorth = std::cos(-(state.dataHeatBal->BuildingAzimuth + state.dataHeatBal->BuildingRotationAppendixG) * Constant::DegToRad);
    Real64 SinBldgRelNorth = std::sin(-(state.dataHeatBal->BuildingAzimuth + state.dataHeatBal->BuildingRotationAppendixG) * Constant::DegToRad);
    // these are only for Building Rotation for Appendix G when using world coordinate system
    Real64 CosBldgRotAppGonly = std::cos(-state.dataHeatBal->BuildingRotationAppendixG * Constant::DegToRad);
    Real64 SinBldgRotAppGonly = std::sin(-state.dataHeatBal->BuildingRotationAppendixG * Constant::DegToRad);

    bool doTransform = false;
    Real64 OldAspectRatio = 1.0;
    Real64 NewAspectRatio = 1.0;

    CheckForGeometricTransform(state, doTransform, OldAspectRatio, NewAspectRatio);
    for (auto &daylCntrl : dl->daylightControl) {
        auto &zone = state.dataHeatBal->Zone(daylCntrl.zoneIndex);

        // Calc cos and sin of Zone Relative North values for later use in transforming Reference Point coordinates
        Real64 CosZoneRelNorth = std::cos(-zone.RelNorth * Constant::DegToRad);
        Real64 SinZoneRelNorth = std::sin(-zone.RelNorth * Constant::DegToRad);

        Real64 rLightLevel = InternalHeatGains::GetDesignLightingLevelForZone(state, daylCntrl.zoneIndex);
        InternalHeatGains::CheckLightsReplaceableMinMaxForZone(state, daylCntrl.zoneIndex);

        for (int refPtNum = 1; refPtNum <= daylCntrl.TotalDaylRefPoints; ++refPtNum) {
            auto &refPt = daylCntrl.refPts(refPtNum);
            auto &curRefPt = dl->DaylRefPt(refPt.num); // get the active daylighting:referencepoint
            curRefPt.indexToFracAndIllum = refPtNum;   // back reference to the index to the ZoneDaylight structure arrays related to reference points
            if (s_surf->DaylRefWorldCoordSystem) {
                // transform only by appendix G rotation
                refPt.absCoords.x = curRefPt.coords.x * CosBldgRotAppGonly - curRefPt.coords.y * SinBldgRotAppGonly;
                refPt.absCoords.y = curRefPt.coords.x * SinBldgRotAppGonly + curRefPt.coords.y * CosBldgRotAppGonly;
                refPt.absCoords.z = curRefPt.coords.z;
            } else {
                // Transform reference point coordinates into building coordinate system
                Real64 Xb = curRefPt.coords.x * CosZoneRelNorth - curRefPt.coords.y * SinZoneRelNorth + zone.OriginX;
                Real64 Yb = curRefPt.coords.x * SinZoneRelNorth + curRefPt.coords.y * CosZoneRelNorth + zone.OriginY;
                // Transform into World Coordinate System
                refPt.absCoords.x = Xb * CosBldgRelNorth - Yb * SinBldgRelNorth;
                refPt.absCoords.y = Xb * SinBldgRelNorth + Yb * CosBldgRelNorth;
                refPt.absCoords.z = curRefPt.coords.z + zone.OriginZ;
                if (doTransform) {
                    Real64 Xo = refPt.absCoords.x; // world coordinates.... shifted by relative north angle...
                    Real64 Yo = refPt.absCoords.y;
                    // next derotate the building
                    Real64 XnoRot = Xo * CosBldgRelNorth + Yo * SinBldgRelNorth;
                    Real64 YnoRot = Yo * CosBldgRelNorth - Xo * SinBldgRelNorth;
                    // translate
                    Real64 Xtrans = XnoRot * std::sqrt(NewAspectRatio / OldAspectRatio);
                    Real64 Ytrans = YnoRot * std::sqrt(OldAspectRatio / NewAspectRatio);
                    // rerotate
                    refPt.absCoords.x = Xtrans * CosBldgRelNorth - Ytrans * SinBldgRelNorth;
                    refPt.absCoords.y = Xtrans * SinBldgRelNorth + Ytrans * CosBldgRelNorth;
                }
            }

            auto &orp = state.dataOutRptPredefined;
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchDyLtZone, curRefPt.Name, daylCntrl.ZoneName);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchDyLtCtrlName, curRefPt.Name, daylCntrl.Name);
            if (daylCntrl.DaylightMethod == DaylightingMethod::SplitFlux) {
                OutputReportPredefined::PreDefTableEntry(state, orp->pdchDyLtKind, curRefPt.Name, "SplitFlux");
            } else {
                OutputReportPredefined::PreDefTableEntry(state, orp->pdchDyLtKind, curRefPt.Name, "DElight");
            }
            // ( 1=continuous, 2=stepped, 3=continuous/off )
            if (daylCntrl.LightControlType == LtgCtrlType::Continuous) {
                OutputReportPredefined::PreDefTableEntry(state, orp->pdchDyLtCtrlType, curRefPt.Name, "Continuous");
            } else if (daylCntrl.LightControlType == LtgCtrlType::Stepped) {
                OutputReportPredefined::PreDefTableEntry(state, orp->pdchDyLtCtrlType, curRefPt.Name, "Stepped");
            } else if (daylCntrl.LightControlType == LtgCtrlType::ContinuousOff) {
                OutputReportPredefined::PreDefTableEntry(state, orp->pdchDyLtCtrlType, curRefPt.Name, "Continuous/Off");
            }
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchDyLtFrac, curRefPt.Name, refPt.fracZoneDaylit);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchDyLtWInst, curRefPt.Name, rLightLevel);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchDyLtWCtrl, curRefPt.Name, rLightLevel * refPt.fracZoneDaylit);

            if (refPt.absCoords.x < zone.MinimumX || refPt.absCoords.x > zone.MaximumX) {
                refPt.inBounds = false;
                ShowWarningError(state,
                                 format("GeometryTransformForDaylighting: Reference point X Value outside Zone Min/Max X, Zone={}", zone.Name));
                ShowContinueError(state,
                                  format("...X Reference Point= {:.2R}, Zone Minimum X= {:.2R}, Zone Maximum X= {:.2R}",
                                         refPt.absCoords.x,
                                         zone.MinimumX,
                                         zone.MaximumX));
                ShowContinueError(
                    state,
                    format("...X Reference Distance Outside MinimumX= {:.4R} m.",
                           (refPt.absCoords.x < zone.MinimumX) ? (zone.MinimumX - refPt.absCoords.x) : (refPt.absCoords.x - zone.MaximumX)));
            }
            if (refPt.absCoords.y < zone.MinimumY || refPt.absCoords.y > zone.MaximumY) {
                refPt.inBounds = false;
                ShowWarningError(state,
                                 format("GeometryTransformForDaylighting: Reference point Y Value outside Zone Min/Max Y, Zone={}", zone.Name));
                ShowContinueError(state,
                                  format("...Y Reference Point= {:.2R}, Zone Minimum Y= {:.2R}, Zone Maximum Y= {:.2R}",
                                         refPt.absCoords.x,
                                         zone.MinimumY,
                                         zone.MaximumY));
                ShowContinueError(
                    state,
                    format("...Y Reference Distance Outside MinimumY= {:.4R} m.",
                           (refPt.absCoords.y < zone.MinimumY) ? (zone.MinimumY - refPt.absCoords.y) : (refPt.absCoords.y - zone.MaximumY)));
            }
            if (refPt.absCoords.z < zone.MinimumZ || refPt.absCoords.z > zone.MaximumZ) {
                refPt.inBounds = false;
                ShowWarningError(state,
                                 format("GeometryTransformForDaylighting: Reference point Z Value outside Zone Min/Max Z, Zone={}", zone.Name));
                ShowContinueError(state,
                                  format("...Z Reference Point= {:.2R}, Zone Minimum Z= {:.2R}, Zone Maximum Z= {:.2R}",
                                         refPt.absCoords.z,
                                         zone.MinimumZ,
                                         zone.MaximumZ));
                ShowContinueError(
                    state,
                    format("...Z Reference Distance Outside MinimumZ= {:.4R} m.",
                           (refPt.absCoords.z < zone.MinimumZ) ? (zone.MinimumZ - refPt.absCoords.z) : (refPt.absCoords.z - zone.MaximumZ)));
            }
        } // for (refPt)
    } // for (daylightCtrl)
} // GeometryTransformForDaylighting()

void GetInputDayliteRefPt(EnergyPlusData &state, bool &ErrorsFound)
{
    // Perform GetInput function for the Daylighting:ReferencePoint object
    // Glazer - July 2016
    auto const &dl = state.dataDayltg;
    auto &ip = state.dataInputProcessing->inputProcessor;
    auto const &s_ipsc = state.dataIPShortCut;
    s_ipsc->cCurrentModuleObject = "Daylighting:ReferencePoint";

    int RefPtNum = 0;
    int IOStat;
    int NumAlpha;
    int NumNumber;

    int TotRefPoints = ip->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);

    dl->DaylRefPt.allocate(TotRefPoints);
    for (auto &pt : dl->DaylRefPt) {
        ip->getObjectItem(state,
                          s_ipsc->cCurrentModuleObject,
                          ++RefPtNum,
                          s_ipsc->cAlphaArgs,
                          NumAlpha,
                          s_ipsc->rNumericArgs,
                          NumNumber,
                          IOStat,
                          s_ipsc->lNumericFieldBlanks,
                          s_ipsc->lAlphaFieldBlanks,
                          s_ipsc->cAlphaFieldNames,
                          s_ipsc->cNumericFieldNames);
        pt.Name = s_ipsc->cAlphaArgs(1);
        pt.ZoneNum = Util::FindItemInList(s_ipsc->cAlphaArgs(2), state.dataHeatBal->Zone);
        if (pt.ZoneNum == 0) {
            int spaceNum = Util::FindItemInList(s_ipsc->cAlphaArgs(2), state.dataHeatBal->space);
            if (spaceNum == 0) {
                ShowSevereError(state,
                                format("{}=\"{}\", invalid {}=\"{}\".",
                                       s_ipsc->cCurrentModuleObject,
                                       s_ipsc->cAlphaArgs(1),
                                       s_ipsc->cAlphaFieldNames(2),
                                       s_ipsc->cAlphaArgs(2)));
                ErrorsFound = true;
            } else {
                pt.ZoneNum = state.dataHeatBal->space(spaceNum).zoneNum;
            }
        }
        pt.coords = {s_ipsc->rNumericArgs(1), s_ipsc->rNumericArgs(2), s_ipsc->rNumericArgs(3)};
    }
}

bool doesDayLightingUseDElight(EnergyPlusData const &state)
{
    auto const &dl = state.dataDayltg;
    for (auto const &znDayl : dl->daylightControl) {
        if (znDayl.DaylightMethod == DaylightingMethod::DElight) {
            return true;
        }
    }
    return false;
}

void CheckTDDsAndLightShelvesInDaylitZones(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Dec 2007

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine checks daylighting input for TDDs and light shelfs
    //  which need to be checked after daylighting input has been read in (CR 7145)
    //  (eventually this should be changed once/if implementations change to decouple from daylighting calcs so that
    //  these devices can be used in models without daylighting controls
    // CR 7145 was for TDDs, but also implenting check for light shelves, the other "daylighting device"

    // METHODOLOGY EMPLOYED:
    // loop thru daylighting devices and check that their zones have daylight controls

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    auto &s_surf = state.dataSurface;

    bool ErrorsFound = false;

    for (auto const &pipe : state.dataDaylightingDevicesData->TDDPipe) {
        int SurfNum = pipe.Diffuser;
        if (SurfNum > 0) {
            int const pipeEnclNum = s_surf->Surface(SurfNum).SolarEnclIndex;
            if (state.dataViewFactor->EnclSolInfo(pipeEnclNum).TotalEnclosureDaylRefPoints == 0) {
                ShowWarningError(state,
                                 format("DaylightingDevice:Tubular = {}:  is not connected to a Zone that has Daylighting, no visible transmittance "
                                        "will be modeled through the daylighting device.",
                                        pipe.Name));
            }
        } else { // SurfNum == 0
            // should not come here (would have already been caught in TDD get input), but is an error
            ShowSevereError(state, format("DaylightingDevice:Tubular = {}:  Diffuser surface not found ", pipe.Name));
            ErrorsFound = true;
        }
    } // for (pipe)

    for (auto const &shelf : state.dataDaylightingDevicesData->Shelf) {
        if (shelf.Window == 0) {
            // should not come here (would have already been caught in shelf get input), but is an error
            ShowSevereError(state, format("DaylightingDevice:Shelf = {}:  window not found ", shelf.Name));
            ErrorsFound = true;
        }
    } // for (shelf)

    if (ErrorsFound) {
        ShowFatalError(state, "CheckTDDsAndLightShelvesInDaylitZones: Errors in DAYLIGHTING input.");
    }
}

void AssociateWindowShadingControlWithDaylighting(EnergyPlusData &state)
{
    auto &dl = state.dataDayltg;
    auto &s_surf = state.dataSurface;

    for (auto &winShadeControl : s_surf->WindowShadingControl) {
        if (winShadeControl.DaylightingControlName.empty()) {
            continue;
        }
        int found = -1;
        for (int daylightCtrlNum = 1; daylightCtrlNum <= (int)dl->daylightControl.size(); ++daylightCtrlNum) {
            if (Util::SameString(winShadeControl.DaylightingControlName, dl->daylightControl(daylightCtrlNum).Name)) {
                found = daylightCtrlNum;
                break;
            }
        }
        if (found > 0) {
            winShadeControl.DaylightControlIndex = found;
        } else {
            ShowWarningError(state, "AssociateWindowShadingControlWithDaylighting: Daylighting object name used in WindowShadingControl not found.");
            ShowContinueError(state,
                              format("..The WindowShadingControl object=\"{}\" and referenes an object named: \"{}\"",
                                     winShadeControl.Name,
                                     winShadeControl.DaylightingControlName));
        }
    }
} // AssociateWindowShadingControlWithDaylighting()

void GetLightWellData(EnergyPlusData &state, bool &ErrorsFound) // If errors found in input
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   Apr 2004

    // PURPOSE OF THIS SUBROUTINE:
    // Gets data for a light well associated with a rectangular exterior window.
    // Calculates light well efficiency, defined as the ratio of the amount of visible
    // solar radiation leaving a well to the amount entering the well.

    // METHODOLOGY EMPLOYED:
    // Based on fit to Fig. 8-21, "Efficiency factors for various depths of light wells
    // based on well-interreflectance values," Lighting Handbook, 8th Edition, Illuminating
    // Engineering Society of North America, 1993.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    int IOStat;        // IO Status when calling get input subroutine
    int NumAlpha;      // Number of alpha names being passed
    int NumProp;       // Number of properties being passed
    int TotLightWells; // Total Light Well objects

    auto &ip = state.dataInputProcessing->inputProcessor;
    auto &s_surf = state.dataSurface;
    auto const &s_ipsc = state.dataIPShortCut;

    // Get the total number of Light Well objects
    s_ipsc->cCurrentModuleObject = "DaylightingDevice:LightWell";
    TotLightWells = ip->getNumObjectsFound(state, s_ipsc->cCurrentModuleObject);
    if (TotLightWells == 0) {
        return;
    }

    for (int loop = 1; loop <= TotLightWells; ++loop) {

        ip->getObjectItem(state,
                          s_ipsc->cCurrentModuleObject,
                          loop,
                          s_ipsc->cAlphaArgs,
                          NumAlpha,
                          s_ipsc->rNumericArgs,
                          NumProp,
                          IOStat,
                          s_ipsc->lNumericFieldBlanks,
                          s_ipsc->lAlphaFieldBlanks,
                          s_ipsc->cAlphaFieldNames,
                          s_ipsc->cNumericFieldNames);

        int SurfNum = Util::FindItemInList(s_ipsc->cAlphaArgs(1), s_surf->Surface);
        if (SurfNum == 0) {
            ShowSevereError(
                state, format("{}: invalid {}=\"{}\" not found.", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaFieldNames(1), s_ipsc->cAlphaArgs(1)));
            ErrorsFound = true;
            continue;
        }

        auto const &surf = s_surf->Surface(SurfNum);
        auto &surfWin = s_surf->SurfaceWindow(SurfNum);
        // Check that associated surface is an exterior window
        // True if associated surface is not an exterior window
        if (surf.Class != SurfaceClass::Window && surf.ExtBoundCond != ExternalEnvironment) {
            ShowSevereError(state,
                            format("{}: invalid {}=\"{}\" - not an exterior window.",
                                   s_ipsc->cCurrentModuleObject,
                                   s_ipsc->cAlphaFieldNames(1),
                                   s_ipsc->cAlphaArgs(1)));
            ErrorsFound = true;
            continue;
        }

        // Associated surface is an exterior window; calculate light well efficiency.
        surfWin.lightWellEff = 1.0;
        Real64 HeightWell = s_ipsc->rNumericArgs(1);  // Well height (from window to bottom of well) (m)
        Real64 PerimWell = s_ipsc->rNumericArgs(2);   // Well perimeter (at bottom of well) (m)
        Real64 AreaWell = s_ipsc->rNumericArgs(3);    // Well area (at bottom of well) (m2)
        Real64 VisReflWell = s_ipsc->rNumericArgs(4); // Area-weighted visible reflectance of well walls

        // Warning if light well area is less than window area
        if (AreaWell < (surf.Area + s_surf->SurfWinDividerArea(SurfNum) - 0.1)) {
            ShowSevereError(
                state, format("{}: invalid {}=\"{}\" - Areas.", s_ipsc->cCurrentModuleObject, s_ipsc->cAlphaFieldNames(1), s_ipsc->cAlphaArgs(1)));
            ShowContinueError(state, format("has Area of Bottom of Well={:.1R} that is less than window area={:.1R}", surf.Area, AreaWell));
        }

        if (HeightWell >= 0.0 && PerimWell > 0.0 && AreaWell > 0.0) {
            Real64 WellCavRatio = 2.5 * HeightWell * PerimWell / AreaWell;
            surfWin.lightWellEff = std::exp(-WellCavRatio * (0.16368 - 0.14467 * VisReflWell));
        }
    } // End of loop over light well objects
} // GetLightWellData()

inline WinCover findWinShadingStatus(EnergyPlusData &state, int const IWin)
{
    // Return the window shading status, 1=unshaded, 2=shaded

    auto &s_surf = state.dataSurface;
    bool WinShadedNoGlareControl = IS_SHADED_NO_GLARE_CTRL(s_surf->SurfWinShadingFlag(IWin));

    return ((s_surf->SurfWinWindowModelType(IWin) != WindowModel::BSDF) && (WinShadedNoGlareControl || s_surf->SurfWinSolarDiffusing(IWin)))
               ? WinCover::Shaded
               : WinCover::Bare;
}

Real64 DayltgGlare(EnergyPlusData &state,
                   int IL,                   // Reference point index: 1=first ref pt, 2=second ref pt
                   Real64 BLUM,              // Window background (surround) luminance (cd/m2)
                   int const daylightCtrlNum // Current daylighting control number
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   July 1997

    // PURPOSE OF THIS SUBROUTINE:
    // CALCULATE GLARE INDEX.

    // METHODOLOGY EMPLOYED:
    // Called from DayltgInteriorIllum.  Finds glare index at reference
    // point no. IL in a space using the Cornell/BRS large source
    // glare formula. BLUM is the background luminance (cd/m**2).
    // TH comment 1/21/2010: The SurfaceWindow(IWin)%ShadingFlag has to be set
    //  before calling this subroutine. For switchable glazings this is tricky
    //  because the ZoneDaylight(ZoneNum)%SourceLumFromWinAtRefPt(IL,2,loop)
    //  may change every time step to represent intermediate switched state.

    // REFERENCES:
    // Based on DOE-2.1E subroutine DGLARE.

    Real64 GTOT = 0.0; // Glare constant

    auto &dl = state.dataDayltg;

    // Loop over exterior windows associated with zone
    auto &thisDayltgCtrl = dl->daylightControl(daylightCtrlNum);
    auto &thisEnclDaylight = dl->enclDaylight(thisDayltgCtrl.enclIndex);
    for (int loop = 1; loop <= thisEnclDaylight.NumOfDayltgExtWins; ++loop) {
        int IWin = thisEnclDaylight.DayltgExtWinSurfNums(loop);
        WinCover winCover = findWinShadingStatus(state, IWin);
        // Conversion from ft-L to cd/m2, with cd/m2 = 0.2936 ft-L, gives the 0.4794 factor
        // below, which is (0.2936)**0.6
        auto const &extWin = thisDayltgCtrl.refPts(IL).extWins(loop);
        Real64 GTOT1 = 0.4794 * std::pow(extWin.lums[iLum_Source][(int)winCover], 1.6) //
                       * std::pow(extWin.solidAngWtd, 0.8);
        Real64 GTOT2 = BLUM + 0.07 * std::sqrt(extWin.solidAng) * extWin.lums[iLum_Source][(int)winCover];
        GTOT += GTOT1 / (GTOT2 + 0.000001);
    }

    // Glare index (adding 0.000001 prevents LOG10 (0))
    return max(0.0, 10.0 * std::log10(GTOT + 0.000001));
}

void DayltgGlareWithIntWins(EnergyPlusData &state,
                            int const daylightCtrlNum // Current daylighting control number
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   March 2004

    // PURPOSE OF THIS SUBROUTINE:
    // Calculate daylighting glare index for zones with interior windows.

    // METHODOLOGY EMPLOYED:
    // Finds glare index at reference point IL in a daylit zone using the Cornell/BRS large source
    // glare formula. Takes into account inter-reflected illuminance from light entering
    // the zone through interior windows

    // REFERENCES:
    // Based on subroutine DayltgGlare.

    Real64 GTOT = 0.0; // Glare constant(?) // TODO: does this need to be reset for every refPt?

    // Calculate background luminance including effect of inter-reflected illuminance from light
    // entering zone through its interior windows
    auto &dl = state.dataDayltg;
    auto &thisDayltgCtrl = dl->daylightControl(daylightCtrlNum);
    auto &thisEnclDaylight = dl->enclDaylight(thisDayltgCtrl.enclIndex);
    int RefPoints = thisDayltgCtrl.TotalDaylRefPoints; // Number of daylighting reference points in zone
    for (int IL = 1; IL <= RefPoints; ++IL) {
        auto &refPt = thisDayltgCtrl.refPts(IL);

        Real64 BackgroundLum = refPt.lums[iLum_Back] + thisEnclDaylight.InterReflIllFrIntWins * thisEnclDaylight.aveVisDiffReflect / Constant::Pi;
        BackgroundLum = max(refPt.illumSetPoint * thisEnclDaylight.aveVisDiffReflect / Constant::Pi, BackgroundLum);

        // Loop over exterior windows associated with zone
        for (int loop = 1; loop <= thisEnclDaylight.NumOfDayltgExtWins; ++loop) {
            int IWin = thisEnclDaylight.DayltgExtWinSurfNums(loop);
            WinCover winCover = findWinShadingStatus(state, IWin);
            // Conversion from ft-L to cd/m2, with cd/m2 = 0.2936 ft-L, gives the 0.4794 factor
            // below, which is (0.2936)**0.6
            auto const &extWin = thisDayltgCtrl.refPts(IL).extWins(loop);
            Real64 GTOT1 = 0.4794 * (std::pow(extWin.lums[iLum_Source][(int)winCover], 1.6)) //
                           * std::pow(extWin.solidAngWtd, 0.8);
            Real64 GTOT2 = BackgroundLum + 0.07 * std::sqrt(extWin.solidAng) * extWin.lums[iLum_Source][(int)winCover];
            GTOT += GTOT1 / (GTOT2 + 0.000001);
        }

        // Glare index
        refPt.glareIndex = max(0.0, 10.0 * std::log10(GTOT + 0.000001));
    } // for (IL)
} // DaylGlareWithIntWins()

void DayltgExtHorizIllum(EnergyPlusData &state,
                         Illums &HI // Horizontal illuminance from sky for different sky types
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   July 1997

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates exterior daylight illuminance.

    // METHODOLOGY EMPLOYED:
    // Called by CalcDayltgCoefficients. Calculates illuminance
    // on unobstructed horizontal surface by integrating
    // over the luminance distribution of standard CIE skies.
    // Calculates horizontal beam illuminance.
    // REFERENCES:
    // Based on DOE-2.1E subroutine DHILL.

    // Argument array dimensioning

    // SUBROUTINE PARAMETER DEFINITIONS:
    Real64 constexpr DTH = (2.0 * Constant::Pi) / double(NTH); // Sky integration azimuth stepsize (radians)
    Real64 constexpr DPH = Constant::PiOvr2 / double(NPH);     // Sky integration altitude stepsize (radians)

    // Integrate to obtain illuminance from sky.
    // The contribution in lumens/m2 from a patch of sky at altitude PH and azimuth TH
    // is L(TH,PH)*SIN(PH)*COS(PH)*DTH*DPH, where L(TH,PH) is the luminance
    // of the patch in cd/m2.
    auto &dl = state.dataDayltg;

    //  Init
    if (dl->DayltgExtHorizIllum_firstTime) {
        for (int IPH = 1; IPH <= NPH; ++IPH) {
            dl->PH[IPH] = (IPH - 0.5) * DPH;
            dl->SPHCPH[IPH] = std::sin(dl->PH[IPH]) * std::cos(dl->PH[IPH]); // DA = COS(PH)*DTH*DPH
        }
        for (int ITH = 1; ITH <= NTH; ++ITH) {
            dl->TH[ITH] = (ITH - 0.5) * DTH;
        }
        dl->DayltgExtHorizIllum_firstTime = false;
    }

    HI = Illums();

    // Sky integration
    for (int IPH = 1; IPH <= NPH; ++IPH) {
        Real64 const PH_IPH = dl->PH[IPH];
        Real64 const SPHCPH_IPH = dl->SPHCPH[IPH];
        for (int ITH = 1; ITH <= NTH; ++ITH) {
            Real64 const TH_ITH = dl->TH[ITH];
            for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
                HI.sky[iSky] += DayltgSkyLuminance(state, static_cast<SkyType>(iSky), TH_ITH, PH_IPH) * SPHCPH_IPH;
            }
        }
    }

    for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
        HI.sky[iSky] *= DTH * DPH;
    }

    // Direct solar horizontal illum (for unit direct normal illuminance)
    HI.sun = dl->sunAngles.sinPhi * 1.0;
} // DayltgExtHorizIllum()

// Product of solar transmittances of exterior obstructions
Real64 DayltgHitObstruction(EnergyPlusData &state,
                            int const IHOUR,           // Hour number
                            int const IWin,            // Window index
                            Vector3<Real64> const &R1, // Origin of ray (m)
                            Vector3<Real64> const &RN  // Unit vector along ray
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   July 1997
    //       MODIFIED       FCW, May 2003: update list of surface classes that qualify as obstructions;
    //                        add interior surfaces as possible obstructors;
    //                        return from DO loop over surfaces as soon as any obstruction is hit;
    //                      FCW, July 2003: change from returning whether an obstruction is hit or not
    //                        to product of solar transmittances of hit obstructions.
    //                      FCW, Nov 2003: remove interior surfaces as possible obstructors since there
    //                        is now a separate check for interior obstructions; exclude windows and
    //                        doors as obstructors since if they are obstructors their base surfaces will
    //                        also be obstructors
    //       RE-ENGINEERED  Sept 2015. Stuart Mentzer. Octree for performance.

    // PURPOSE OF THIS SUBROUTINE:
    // Determines the product of the solar transmittances of the obstructions hit by a ray
    // from R1 in the direction of vector RN.

    // REFERENCES:
    // Based on DOE-2.1E subroutine DHITSH.

    auto &s_surf = state.dataSurface;
    // Local declarations
    bool hit; // True iff a particular obstruction is hit

    Real64 ObTrans = 1.0;

    auto const &window = s_surf->Surface(IWin);
    int const window_iBaseSurf = window.BaseSurf;

    Vector3<Real64> DayltgHitObstructionHP;
    // Loop over potentially obstructing surfaces, which can be building elements, like walls, or shadowing surfaces, like overhangs
    // Building elements are assumed to be opaque
    // A shadowing surface is opaque unless its transmittance schedule value is non-zero
    if (s_surf->TotSurfaces < octreeCrossover) { // Linear search through surfaces

        for (int ISurf : s_surf->AllShadowPossObstrSurfaceList) {
            auto const &surface = s_surf->Surface(ISurf);
            SurfaceClass IType = surface.Class;
            if ((IType == SurfaceClass::Wall || IType == SurfaceClass::Roof || IType == SurfaceClass::Floor) && (ISurf != window_iBaseSurf)) {
                hit = PierceSurface(state, ISurf, R1, RN, DayltgHitObstructionHP);
                if (hit) { // Building element is hit (assumed opaque)
                    ObTrans = 0.0;
                    break;
                }
            } else if (surface.IsShadowing) {
                hit = PierceSurface(state, ISurf, R1, RN, DayltgHitObstructionHP);
                if (hit) { // Shading surface is hit
                    // Get solar transmittance of the shading surface
                    Real64 const Trans = (surface.shadowSurfSched != nullptr) ? surface.shadowSurfSched->getHrTsVal(state, IHOUR, 1) : 0.0;
                    if (Trans < 1.e-6) {
                        ObTrans = 0.0;
                        break;
                    } else {
                        ObTrans *= Trans;
                    }
                }
            }
        }

    } else { // Surface octree search

        auto const &window_base(window_iBaseSurf > 0 ? s_surf->Surface(window_iBaseSurf) : window);
        auto const *window_base_p(&window_base);

        // Lambda function for the octree to test for surface hit and update transmittance if hit
        auto solarTransmittance = [=, &state, &R1, &RN, &hit, &ObTrans](SurfaceData const &surface) -> bool {
            if (!surface.IsShadowPossibleObstruction) {
                return false; // Do Consider separate octree without filtered surfaces
            }
            DataSurfaces::SurfaceClass const sClass(surface.Class);
            Vector3<Real64> HP;
            if ((sClass == SurfaceClass::Wall || sClass == SurfaceClass::Roof || sClass == SurfaceClass::Floor) && (&surface != window_base_p)) {
                hit = PierceSurface(surface, R1, RN, HP);
                if (hit) { // Building element is hit (assumed opaque)
                    ObTrans = 0.0;
                    return true;
                }
            } else if (surface.IsShadowing) {
                hit = PierceSurface(surface, R1, RN, HP);
                if (hit) { // Shading surface is hit
                    // Get solar transmittance of the shading surface
                    Real64 const Trans = (surface.shadowSurfSched != nullptr) ? surface.shadowSurfSched->getHrTsVal(state, IHOUR, 1) : 0.0;
                    if (Trans < 1.e-6) {
                        ObTrans = 0.0;
                        return true;
                    } else {
                        ObTrans *= Trans;
                        return ObTrans == 0.0;
                    }
                }
            }
            return false;
        };

        // Check octree surface candidates for hits: short circuits if zero transmittance reached
        Vector3<Real64> const RN_inv(SurfaceOctreeCube::safe_inverse(RN));
        state.dataHeatBalMgr->surfaceOctree.processSomeSurfaceRayIntersectsCube(state, R1, RN, RN_inv, solarTransmittance);
    }

    return ObTrans;
} // DayltgHitObstruction()

bool DayltgHitInteriorObstruction(EnergyPlusData &state,
                                  int const IWin,            // Window index
                                  Vector3<Real64> const &R1, // Origin of ray (m)
                                  Vector3<Real64> const &R2  // Destination of ray (m)
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   July 1997
    //       RE-ENGINEERED  Sept 2015. Stuart Mentzer. Octree for performance.

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine checks for interior obstructions between reference point and window element.

    auto &s_surf = state.dataSurface;

    // Preconditions
    assert(magnitude(R2 - R1) > 0.0); // Protect normalize() from divide by zero

    bool hit = false;
    Vector3<Real64> RN = (R2 - R1).normalize(); // Make unit vector
    Real64 const d12 = distance(R1, R2);        // Distance between R1 and R2

    auto const &window = s_surf->Surface(IWin);
    int const window_Enclosure = window.SolarEnclIndex;
    int const window_iBaseSurf = window.BaseSurf;
    auto const &window_base = window_iBaseSurf > 0 ? s_surf->Surface(window_iBaseSurf) : window;
    int const window_base_iExtBoundCond = window_base.ExtBoundCond;

    // Loop over potentially obstructing surfaces, which can be building elements, like walls, or shadowing surfaces, like overhangs
    if (s_surf->TotSurfaces < octreeCrossover) { // Linear search through surfaces
        // Hit coordinates, if ray hits an obstruction
        Vector3<Real64> DayltgHitInteriorObstructionHP;

        for (int ISurf = 1; ISurf <= s_surf->TotSurfaces; ++ISurf) {
            auto const &surface = s_surf->Surface(ISurf);
            SurfaceClass IType = surface.Class;
            if ((surface.IsShadowing) ||                         // Shadowing surface
                ((surface.SolarEnclIndex == window_Enclosure) && // Wall/ceiling/floor is in same zone as window
                 (IType == SurfaceClass::Wall || IType == SurfaceClass::Roof || IType == SurfaceClass::Floor) && (ISurf != window_iBaseSurf) &&
                 (ISurf != window_base_iExtBoundCond))) // Exclude window's base or base-adjacent surfaces
            {
                hit = PierceSurface(state, ISurf, R1, RN, d12, DayltgHitInteriorObstructionHP); // Check if R2-R1 segment pierces surface
                if (hit) {
                    break; // Segment pierces surface: Don't check the rest
                }
            }
        }

    } else { // Surface octree search

        auto const *window_base_p = &window_base;
        auto const &window_base_adjacent = window_base_iExtBoundCond > 0 ? s_surf->Surface(window_base_iExtBoundCond) : window_base;
        auto const *window_base_adjacent_p = &window_base_adjacent;

        // Lambda function for the octree to test for surface hit
        auto surfaceHit = [=, &R1, &hit](SurfaceData const &surface) -> bool {
            DataSurfaces::SurfaceClass const sClass = surface.Class;
            Vector3<Real64> HP;                                  // Hit point
            if ((surface.IsShadowing) ||                         // Shadowing surface
                ((surface.SolarEnclIndex == window_Enclosure) && // Surface is in same zone as window
                 (sClass == SurfaceClass::Wall || sClass == SurfaceClass::Roof || sClass == SurfaceClass::Floor) && // Wall, ceiling/roof, or floor
                 (&surface != window_base_p) && (&surface != window_base_adjacent_p))) // Exclude window's base or base-adjacent surfaces
            {
                hit = PierceSurface(surface, R1, RN, d12, HP); // Check if R2-R1 segment pierces surface
                return hit;
            } else {
                return false;
            }
        };

        // Check octree surface candidates until a hit is found, if any
        state.dataHeatBalMgr->surfaceOctree.hasSurfaceSegmentIntersectsCube(R1, R2, surfaceHit);
    }

    return hit;
} // DayltgHitInteriorObstruction()

bool DayltgHitBetWinObstruction(EnergyPlusData &state,
                                int const IWin1,           // Surface number of origin window
                                int const IWin2,           // Surface number of destination window
                                Vector3<Real64> const &R1, // Origin of ray (on IWin1) (m)
                                Vector3<Real64> const &R2  // Destination of ray (on IWin2) (m)
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   Feb 2004
    //       RE-ENGINEERED  Sept 2015. Stuart Mentzer. Octree for performance.

    // PURPOSE OF THIS SUBROUTINE:
    // Determines if a ray from point R1 on window IWin1 to point R2
    // on window IWin2 hits an obstruction

    auto &s_surf = state.dataSurface;

    // Preconditions
    assert(magnitude(R2 - R1) > 0.0); // Protect normalize() from divide by zero

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    SurfaceClass IType; // Surface type/class

    bool hit = false;
    Vector3<Real64> RN = (R2 - R1).normalize(); // Unit vector

    Real64 const d12 = distance(R1, R2); // Distance between R1 and R2 (m)

    auto const &window1 = s_surf->Surface(IWin1);
    int const window1_iBaseSurf = window1.BaseSurf;
    auto const &window1_base = window1_iBaseSurf > 0 ? s_surf->Surface(window1_iBaseSurf) : window1;
    int const window1_base_iExtBoundCond = window1_base.ExtBoundCond;

    auto const &window2 = s_surf->Surface(IWin2);
    int const window2_Enclosure = window2.SolarEnclIndex;
    int const window2_iBaseSurf = window2.BaseSurf;
    auto const &window2_base = window2_iBaseSurf > 0 ? s_surf->Surface(window2_iBaseSurf) : window2;
    int const window2_base_iExtBoundCond = window2_base.ExtBoundCond;

    // Preconditions
    //        assert( window1.Zone == window2_Zone ); //? This is violated in PurchAirWithDoubleFacadeDaylighting so then why the asymmetry
    // of  only checking for wall/roof/floor for window2 zone below?

    // Loop over potentially obstructing surfaces, which can be building elements, like walls, or shadowing surfaces, like overhangs
    if (s_surf->TotSurfaces < octreeCrossover) { // Linear search through surfaces

        for (int ISurf = 1; ISurf <= s_surf->TotSurfaces; ++ISurf) {
            auto const &surface = s_surf->Surface(ISurf);
            IType = surface.Class;
            if ((surface.IsShadowing) ||                          // Shadowing surface
                ((surface.SolarEnclIndex == window2_Enclosure) && // Wall/ceiling/floor is in same zone as windows
                 (IType == SurfaceClass::Wall || IType == SurfaceClass::Roof || IType == SurfaceClass::Floor) && // Wall, ceiling/roof, or floor
                 (ISurf != window1_iBaseSurf) && (ISurf != window2_iBaseSurf) &&                                 // Exclude windows' base surfaces
                 (ISurf != window1_base_iExtBoundCond) && (ISurf != window2_base_iExtBoundCond))) // Exclude windows' base-adjacent surfaces
            {
                Vector3<Real64> HP;
                hit = PierceSurface(state, ISurf, R1, RN, d12, HP); // Check if R2-R1 segment pierces surface
                if (hit) {
                    break; // Segment pierces surface: Don't check the rest
                }
            }
        }

    } else { // Surface octree search

        auto const *window1_base_p = &window1_base;
        auto const &window1_base_adjacent = window1_base_iExtBoundCond > 0 ? s_surf->Surface(window1_base_iExtBoundCond) : window1_base;
        auto const *window1_base_adjacent_p = &window1_base_adjacent;

        auto const *window2_base_p = &window2_base;
        auto const &window2_base_adjacent = (window2_base_iExtBoundCond > 0) ? s_surf->Surface(window2_base_iExtBoundCond) : window2_base;
        auto const *window2_base_adjacent_p = &window2_base_adjacent;

        // Lambda function for the octree to test for surface hit
        auto surfaceHit = [=, &R1, &RN, &hit](SurfaceData const &surface) -> bool {
            DataSurfaces::SurfaceClass const sClass = surface.Class;
            Vector3<Real64> HP;
            if ((surface.IsShadowing) ||                          // Shadowing surface
                ((surface.SolarEnclIndex == window2_Enclosure) && // Surface is in same zone as window
                 (sClass == SurfaceClass::Wall || sClass == SurfaceClass::Roof || sClass == SurfaceClass::Floor) && // Wall, ceiling/roof, or floor
                 (&surface != window1_base_p) && (&surface != window2_base_p) &&                                    // Exclude windows' base surfaces
                 (&surface != window1_base_adjacent_p) && (&surface != window2_base_adjacent_p))) // Exclude windows' base-adjacent surfaces
            {
                hit = PierceSurface(surface, R1, RN, d12, HP); // Check if R2-R1 segment pierces surface
                return hit;
            } else {
                return false;
            }
        };

        // Check octree surface candidates until a hit is found, if any
        state.dataHeatBalMgr->surfaceOctree.hasSurfaceSegmentIntersectsCube(R1, R2, surfaceHit);
    }

    return hit;
} // DayltingHitBetWinObstruction()

void initDaylighting(EnergyPlusData &state, bool const initSurfaceHeatBalancefirstTime)
{
    // For daylit zones, calculate interior daylight illuminance at reference points and
    // simulate lighting control system to get overhead electric lighting reduction
    // factor due to daylighting.
    auto &dl = state.dataDayltg;
    auto &s_surf = state.dataSurface;

    for (int SurfNum : s_surf->AllExtSolWindowSurfaceList) {
        for (auto &refPt : s_surf->SurfaceWindow(SurfNum).refPts) {
            refPt.illumFromWinRep = refPt.lumWinRep = 0.0;
        }
    }

    // Reset space power reduction factors
    for (int spaceNum = 1; spaceNum <= state.dataGlobal->numSpaces; ++spaceNum) {
        dl->spacePowerReductionFactor(spaceNum) = 1.0;
    }
    for (int daylightCtrlNum = 1; daylightCtrlNum <= (int)dl->daylightControl.size(); ++daylightCtrlNum) {
        auto &thisDayltgCtrl = dl->daylightControl(daylightCtrlNum);
        thisDayltgCtrl.PowerReductionFactor = 1.0;
        if (state.dataEnvrn->PreviousSolRadPositive) {
            // Reset to zero only if there was solar in the previous timestep, otherwise these are already zero
            dl->enclDaylight(thisDayltgCtrl.enclIndex).InterReflIllFrIntWins = 0.0; // inter-reflected illuminance from interior windows
            for (int refPtNum = 1; refPtNum <= thisDayltgCtrl.TotalDaylRefPoints; ++refPtNum) {
                auto &refPt = thisDayltgCtrl.refPts(refPtNum);
                refPt.lums[iLum_Illum] = 0.0;
                refPt.glareIndex = 0.0;
                refPt.timeExceedingGlareIndexSetPoint = 0.0;
                refPt.timeExceedingDaylightIlluminanceSetPoint = 0.0;
            }
        }

        if (state.dataEnvrn->SunIsUp && thisDayltgCtrl.TotalDaylRefPoints != 0) {
            if (initSurfaceHeatBalancefirstTime) {
                DisplayString(state, "Computing Interior Daylighting Illumination");
            }
            DayltgInteriorIllum(state, daylightCtrlNum);
        }
    }

    // The following report variables are valid only for daylit zones/enclosures without interior windows
    if (state.dataEnvrn->SunIsUp) {
        for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
            if ((state.dataViewFactor->EnclSolInfo(enclNum).TotalEnclosureDaylRefPoints == 0) ||
                (state.dataViewFactor->EnclSolInfo(enclNum).HasInterZoneWindow)) {
                continue;
            }

            auto &thisEnclDaylight = dl->enclDaylight(enclNum);
            for (int extWinNum = 1; extWinNum <= thisEnclDaylight.NumOfDayltgExtWins; ++extWinNum) {
                int IWin = thisEnclDaylight.DayltgExtWinSurfNums(extWinNum);
                WinCover winCover = WinCover::Bare;
                if (s_surf->SurfWinWindowModelType(IWin) != WindowModel::BSDF &&
                    (IS_SHADED(s_surf->SurfWinShadingFlag(IWin)) || s_surf->SurfWinSolarDiffusing(IWin))) {
                    winCover = WinCover::Shaded;
                }
                int refPtCount = 0;
                for (int controlNum : dl->enclDaylight(enclNum).daylightControlIndexes) {
                    auto &daylCtrl = dl->daylightControl(controlNum);
                    if (daylCtrl.DaylightMethod != DaylightingMethod::SplitFlux) {
                        continue;
                    }

                    for (int refPtNum = 1; refPtNum <= daylCtrl.TotalDaylRefPoints; ++refPtNum) {
                        ++refPtCount; // Count reference points across each daylighting control in the same enclosure
                        auto &refPt = s_surf->SurfaceWindow(IWin).refPts(refPtCount);
                        auto const &daylCtrlRefPt = daylCtrl.refPts(refPtNum);
                        refPt.illumFromWinRep = daylCtrlRefPt.extWins(extWinNum).lums[iLum_Illum][(int)winCover];
                        refPt.lumWinRep = daylCtrlRefPt.extWins(extWinNum).lums[iLum_Source][(int)winCover];
                    }
                } // for (controlNum)
            } // for (extWinNum)
        } // for (enclNum)
    } // if (SunIsUp)

    if (state.dataEnvrn->SunIsUp && (int)state.dataDaylightingDevicesData->TDDPipe.size() > 0) {
        if (initSurfaceHeatBalancefirstTime) {
            DisplayString(state, "Computing Interior Daylighting Illumination for TDD pipes");
        }
        DayltgInteriorTDDIllum(state);
    }

    for (int daylightCtrlNum = 1; daylightCtrlNum <= (int)dl->daylightControl.size(); ++daylightCtrlNum) {
        auto &thisDayltgCtrl = dl->daylightControl(daylightCtrlNum);

        // RJH DElight Modification Begin - Call to DElight electric lighting control subroutine
        // Check if the sun is up and the current Thermal Zone hosts a Daylighting:DElight object
        if (state.dataEnvrn->SunIsUp && thisDayltgCtrl.TotalDaylRefPoints != 0 && (thisDayltgCtrl.DaylightMethod == DaylightingMethod::DElight)) {
            int zoneNum = thisDayltgCtrl.zoneIndex;
            // Call DElight interior illuminance and electric lighting control subroutine
            Real64 dPowerReducFac = 1.0; // Return value Electric Lighting Power Reduction Factor for current Zone and Timestep
            Real64 dHISKFFC = state.dataEnvrn->HISKF * DataDElight::LUX2FC;
            Real64 dHISUNFFC = state.dataEnvrn->HISUNF * DataDElight::LUX2FC;
            Real64 dSOLCOS1 = state.dataEnvrn->SOLCOS.x;
            Real64 dSOLCOS2 = state.dataEnvrn->SOLCOS.y;
            Real64 dSOLCOS3 = state.dataEnvrn->SOLCOS.z;
            Real64 dLatitude = state.dataEnvrn->Latitude;
            Real64 dCloudFraction = state.dataEnvrn->CloudFraction;
            // Init Error Flag to 0 (no Warnings or Errors) (returned from DElight)
            int iErrorFlag = 0;

            DElightManagerF::DElightElecLtgCtrl(len(state.dataHeatBal->Zone(zoneNum).Name),
                                                state.dataHeatBal->Zone(zoneNum).Name,
                                                dLatitude,
                                                dHISKFFC,
                                                dHISUNFFC,
                                                dCloudFraction,
                                                dSOLCOS1,
                                                dSOLCOS2,
                                                dSOLCOS3,
                                                dPowerReducFac,
                                                iErrorFlag);
            // Check Error Flag for Warnings or Errors returning from DElight
            // RJH 2008-03-07: If no warnings/errors then read refpt illuminances for standard output reporting
            if (iErrorFlag != 0) {
                std::string cErrorMsg; // Each DElight Error Message can be up to 200 characters long
                // Open DElight Electric Lighting Error File for reading
                auto iDElightErrorFile = state.files.outputDelightDfdmpFilePath.try_open(state.files.outputControl.delightdfdmp); // (THIS_AUTO_OK)
                bool elOpened = iDElightErrorFile.good();

                // Sequentially read lines in DElight Electric Lighting Error File
                // and process them using standard EPlus warning/error handling calls
                bool bEndofErrFile = false;
                while (!bEndofErrFile && elOpened) {
                    auto cErrorLine = iDElightErrorFile.readLine(); // (THIS_AUTO_OK)
                    if (cErrorLine.eof) {
                        bEndofErrFile = true;
                        continue;
                    }

                    // Is the current line a Warning message?
                    if (has_prefix(cErrorLine.data, "WARNING: ")) {
                        cErrorMsg = cErrorLine.data.substr(9);
                        ShowWarningError(state, cErrorMsg);
                    }
                    // Is the current line an Error message?
                    if (has_prefix(cErrorLine.data, "ERROR: ")) {
                        cErrorMsg = cErrorLine.data.substr(7);
                        ShowSevereError(state, cErrorMsg);
                        iErrorFlag = 1;
                    }
                }

                // Close DElight Error File and delete

                if (elOpened) {
                    iDElightErrorFile.close();
                    FileSystem::removeFile(iDElightErrorFile.filePath);
                }
                // If any DElight Error occurred then ShowFatalError to terminate
                if (iErrorFlag > 0) {
                    ShowFatalError(state, "End of DElight Error Messages");
                }
            } else { // RJH 2008-03-07: No errors
                     // extract reference point illuminance values from DElight Electric Lighting dump file for reporting
                     // Open DElight Electric Lighting Dump File for reading
                auto iDElightErrorFile = state.files.outputDelightEldmpFilePath.try_open(state.files.outputControl.delighteldmp); // (THIS_AUTO_OK)
                bool elOpened = iDElightErrorFile.is_open();

                // Sequentially read lines in DElight Electric Lighting Dump File
                // and extract refpt illuminances for standard EPlus output handling
                bool bEndofErrFile = false;
                int iDElightRefPt = 0; // Reference Point number for reading DElight Dump File (eplusout.delighteldmp)
                while (!bEndofErrFile && elOpened) {
                    auto line = iDElightErrorFile.read<Real64>(); // (THIS_AUTO_OK)
                    Real64 dRefPtIllum = line.data;               // tmp var for reading RefPt illuminance
                    if (line.eof) {
                        bEndofErrFile = true;
                        continue;
                    }
                    // Increment refpt counter
                    ++iDElightRefPt;
                    // Assure refpt index does not exceed number of refpts in this zone
                    if (iDElightRefPt <= thisDayltgCtrl.TotalDaylRefPoints) {
                        thisDayltgCtrl.refPts(iDElightRefPt).lums[iLum_Illum] = dRefPtIllum;
                    }
                }

                // Close DElight Electric Lighting Dump File and delete
                if (elOpened) {
                    iDElightErrorFile.close();
                    FileSystem::removeFile(iDElightErrorFile.filePath);
                };
            }
            // Store the calculated total zone Power Reduction Factor due to DElight daylighting
            // in the ZoneDaylight structure for later use
            thisDayltgCtrl.PowerReductionFactor = dPowerReducFac;
        }
        // RJH DElight Modification End - Call to DElight electric lighting control subroutine
    }

    if (state.dataEnvrn->SunIsUp && !state.dataGlobal->DoingSizing) {
        DayltgInteriorMapIllum(state);
    }
    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        for (int const spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            for (int SurfNum = thisSpace.WindowSurfaceFirst; SurfNum <= thisSpace.WindowSurfaceLast; ++SurfNum) {
                s_surf->SurfWinFracTimeShadingDeviceOn(SurfNum) = 0.0;
                if (IS_SHADED(s_surf->SurfWinShadingFlag(SurfNum))) {
                    s_surf->SurfWinFracTimeShadingDeviceOn(SurfNum) = 1.0;
                } else {
                    s_surf->SurfWinFracTimeShadingDeviceOn(SurfNum) = 0.0;
                }
            }
        }
    }
}

void manageDaylighting(EnergyPlusData &state)
{
    auto &dl = state.dataDayltg;

    if (state.dataEnvrn->SunIsUp && (state.dataEnvrn->BeamSolarRad + state.dataEnvrn->GndSolarRad + state.dataEnvrn->DifSolarRad > 0.0)) {
        for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
            auto const &enclSol = state.dataViewFactor->EnclSolInfo(enclNum);
            if (enclSol.TotalEnclosureDaylRefPoints == 0 || !enclSol.HasInterZoneWindow) {
                continue;
            }

            DayltgInterReflIllFrIntWins(state, enclNum);
            for (int daylightCtrlNum : dl->enclDaylight(enclNum).daylightControlIndexes) {
                DayltgGlareWithIntWins(state, daylightCtrlNum);
            }
        }
        DayltgElecLightingControl(state);
    } else if (dl->mapResultsToReport && state.dataGlobal->TimeStep == state.dataGlobal->TimeStepsInHour) {
        for (int MapNum = 1; MapNum <= (int)dl->illumMaps.size(); ++MapNum) {
            ReportIllumMap(state, MapNum);
        }
        dl->mapResultsToReport = false;
    }
} // manageDaylighting()

void DayltgInteriorIllum(EnergyPlusData &state,
                         int const daylightCtrlNum) // Daylighting:Controls number
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   July 1997
    //       MODIFIED       March 2000, FCW: interpolate clear-sky daylight factors using
    //                      HourOfDay/WeightNow and NextHour/WeightNextHour. Previously
    //                      only HourOfDay was used
    //                      Jan 2001, FCW: interpolate in slat angle for windows with blinds
    //                      that have movable slats
    //                      Oct 2002, LKL: changed interpolation steps to HourOfDay/WeightNow
    //                      LastHour/WeightPreviousHour
    //                      Aug 2003, FCW: fix bug that prevented shadingControlType =
    //                      MEETDAYLIGHTILLUMINANCESETPOINT from working
    //                      Mar 2004, FCW: fix bug in calc of illuminance setpoint contribution
    //                      to background luminance: now it is divided by pi to give cd/m2
    //                      Mar 2004, FCW: modify to handle daylighting through interior windows
    //                      June 2009, TH: modified for thermochromic windows
    //                      Jan 2010, TH (CR 7984): added iterations for switchable windows with shading
    //                       control of MeetDaylightIlluminanceSetpoint and glare control is active
    //                       Also corrected bugs (CR 7988) for switchable glazings not related to CR 7984

    // PURPOSE OF THIS SUBROUTINE:
    // Using daylighting factors and exterior illuminance, determine
    // the current-hour interior daylight illuminance and glare index
    // at each reference point in a space. Deploy window shading window by window
    // if glare control is active for window and if the acceptable glare index
    // is exceeded at both reference points.

    // Called by InitSurfaceHeatBalance.

    // REFERENCES:
    // Based on DOE-2.1E subroutine DINTIL.
    auto &dl = state.dataDayltg;
    auto &s_surf = state.dataSurface;

    Real64 constexpr tmpSWIterStep(0.05); // step of switching factor, assuming maximum of 20 switching states

    int NREFPT; // Number of daylighting reference points
    int iSky1;  // Sky type index values for averaging two sky types
    int iSky2;
    Array1D<Real64> SetPnt; // Illuminance setpoint at reference points (lux)
    Array1D<Real64> GLRNEW; // New glare index at reference point

    auto &thisDayltgCtrl = dl->daylightControl(daylightCtrlNum);
    int enclNum = thisDayltgCtrl.enclIndex;
    auto &thisEnclDaylight = dl->enclDaylight(enclNum);
    int ISWFLG; // Switchable glazing flag: =1 if one or more windows in a zone
    //  has switchable glazing that adjusts visible transmittance to just meet
    //  daylighting setpoint; =0 otherwise.
    Real64 VTRAT;        // Ratio between switched and unswitched visible transmittance at normal incidence
    Real64 BACL;         // Window background (surround) luminance for glare calc (cd/m2)
    Real64 SkyWeight;    // Weighting factor used to average two different sky types
    Real64 HorIllSkyFac; // Ratio between horizontal illuminance from sky horizontal irradiance and
    //   luminous efficacy and horizontal illuminance from averaged sky
    bool GlareFlag; // True if maximum glare is exceeded

    Real64 VTRatio;  // VT (visible transmittance) ratio = VTNow / VTMaster
    Real64 VTNow;    // VT of the time step actual TC window
    Real64 VTMaster; // VT of the base/master TC window

    Array2D<std::array<std::array<Real64, (int)DataSurfaces::WinCover::Num>, (int)Lum::Num>> tmpDaylFromWinAtRefPt;

    bool breakOuterLoop(false);
    bool continueOuterLoop(false);

    struct ShadeGroupLums
    {
        Array1D<std::array<std::array<Real64, (int)DataSurfaces::WinCover::Num>, (int)Lum::Num>> WDAYIL; // Illuminance from window at ref-point
        Array1D<std::array<Real64, (int)Lum::Num>> RDAYIL; // Illuminance from window at ref-point after closing shade
        Real64 switchedWinLum;
        Real64 unswitchedWinLum;
        Real64 switchedTvis;
        Real64 unswitchedTvis;
        Real64 lumRatio;
    };

    Array1D<ShadeGroupLums> shadeGroupsLums;

    // Array2D<std::array<std::array<Real64, (int)DataSurfaces::WinCover::Num>, iLum_Num>> WDAYIL; // Illuminance from window at reference point
    // (second index)
    //   the number of shade deployment groups (third index)
    // Array2D<std::array<Real64, (int)DataSurfaces::WinCover::Num>> WBACLU; // Background illuminance from window at reference point (second index)
    //   the number of shade deployment groups (third index)
    // Array2D<std::array<Real64, iLum_Num>> RDAYIL; // Illuminance from window at reference point after closing shade
    // Array2D<Real64> RBACLU; // Background illuminance from window at reference point after closing shade
    // Array1D<Real64> DILLSW;         // Illuminance a ref point from a group of windows that can be switched together,
    // Array1D<Real64> DILLUN;         //  and from those that aren't (lux)
    // Array1D<Real64> TVIS1;  // Visible transmittance at normal incidence of unswitched glazing
    // Array1D<Real64> TVIS2;  // Visible transmittance at normal incidence of fully-switched glazing
    // Array1D<Real64> ASETIL; // Illuminance ratio (lux)

    if (thisDayltgCtrl.DaylightMethod != DaylightingMethod::SplitFlux) {
        return;
    }

    NREFPT = thisDayltgCtrl.TotalDaylRefPoints;

    if (dl->DayltgInteriorIllum_firstTime) {
        dl->DaylIllum.allocate(dl->maxNumRefPtInAnyDaylCtrl);
        dl->DayltgInteriorIllum_firstTime = false;
    }

    // size these for the maximum of the shade deployment order
    shadeGroupsLums.allocate(dl->maxShadeDeployOrderExtWins);
    for (auto &shadeGroupLums : shadeGroupsLums) {
        shadeGroupLums.WDAYIL.allocate(dl->maxControlRefPoints);
        shadeGroupLums.RDAYIL.allocate(dl->maxControlRefPoints);
    }

    // Three arrays to save original clear and dark (fully switched) states'
    //  zone/window daylighting properties.
    tmpDaylFromWinAtRefPt.allocate(dl->maxNumRefPtInAnyDaylCtrl, dl->maxEnclSubSurfaces);

    SetPnt.allocate(dl->maxNumRefPtInAnyDaylCtrl);
    dl->DaylIllum.allocate(dl->maxNumRefPtInAnyDaylCtrl);
    GLRNEW.allocate(dl->maxNumRefPtInAnyDaylCtrl);

    for (int iRefPt = 1; iRefPt <= (int)dl->maxNumRefPtInAnyDaylCtrl; ++iRefPt) {
        for (int iExtWin = 1; iExtWin <= (int)dl->maxEnclSubSurfaces; ++iExtWin) {
            auto &tmpDayl = tmpDaylFromWinAtRefPt(iRefPt, iExtWin);
            tmpDayl[iLum_Illum] = tmpDayl[iLum_Back] = tmpDayl[iLum_Source] = {0.0, 0.0};
        }
    }

    // Initialize reference point illuminance and window background luminance
    for (int IL = 1; IL <= NREFPT; ++IL) {
        auto &refPt = thisDayltgCtrl.refPts(IL);
        SetPnt(IL) = refPt.illumSetPoint;
        dl->DaylIllum(IL) = 0.0;
        refPt.lums[iLum_Back] = 0.0;
    }

    if (state.dataEnvrn->SkyClearness > 3.0) { // Sky is average of clear and clear turbid
        SkyWeight = min(1.0, (state.dataEnvrn->SkyClearness - 3.0) / 3.0);
        iSky1 = (int)SkyType::Clear;
        iSky2 = (int)SkyType::ClearTurbid;
    } else if (state.dataEnvrn->SkyClearness > 1.2) { // Sky is average of clear turbid and intermediate
        SkyWeight = (state.dataEnvrn->SkyClearness - 1.2) / 1.8;
        iSky1 = (int)SkyType::ClearTurbid;
        iSky2 = (int)SkyType::Intermediate;
    } else { // Sky is average of intermediate and overcast
        SkyWeight = min(1.0, max(0.0, (state.dataEnvrn->SkyClearness - 1.0) / 0.2, (state.dataEnvrn->SkyBrightness - 0.05) / 0.4));
        iSky1 = (int)SkyType::Intermediate;
        iSky2 = (int)SkyType::Overcast;
    }

    // First loop over exterior windows associated with this zone. The window may be an exterior window in
    // the zone or an exterior window in an adjacent zone that shares an interior window with the zone.
    // Find contribution of each window to the daylight illum and to the glare numerator at each reference point.
    // Use shading flags set in WindowShadingManager.
    for (int loop = 1; loop <= thisEnclDaylight.NumOfDayltgExtWins; ++loop) {
        int IWin = thisEnclDaylight.DayltgExtWinSurfNums(loop);

        // Added TH 6/29/2009 for thermochromic windows
        VTRatio = 1.0;
        if (NREFPT > 0) {
            int const IConst = s_surf->Surface(IWin).Construction;
            auto const &construction = state.dataConstruction->Construct(IConst);
            if (construction.isTCWindow) {
                // For thermochromic windows, daylight and glare factors are always calculated
                //  based on the master construction. They need to be adjusted by the VTRatio, including:
                //  ZoneDaylight()%DaylIllFacSky, DaylIllFacSun, DaylIllFacSunDisk; DaylBackFacSky,
                //  DaylBackFacSun, DaylBackFacSunDisk, DaylSourceFacSky, DaylSourceFacSun, DaylSourceFacSunDisk
                VTNow = Window::POLYF(1.0, construction.TransVisBeamCoef);
                VTMaster = Window::POLYF(1.0, state.dataConstruction->Construct(construction.TCMasterConstrNum).TransVisBeamCoef);
                VTRatio = VTNow / VTMaster;
            }
        }

        bool ShadedOrDiffusingGlassWin = s_surf->SurfWinWindowModelType(IWin) != WindowModel::BSDF &&
                                         (IS_SHADED(s_surf->SurfWinShadingFlag(IWin)) || s_surf->SurfWinSolarDiffusing(IWin));

        Real64 wgtCurrHr = state.dataGlobal->WeightNow;
        Real64 wgtPrevHr = state.dataGlobal->WeightPreviousHour;

        std::array<Illums, (int)DataSurfaces::WinCover::Num> SFHR; // Sky source luminance factor for sky type, bare/shaded window
        std::array<Illums, (int)DataSurfaces::WinCover::Num> DFHR; // Sky daylight factor for sky type, bare/shaded window
        std::array<Illums, (int)DataSurfaces::WinCover::Num> BFHR; // Sky background luminance factor for sky type, bare/shaded window

        // Loop over reference points
        for (int IL = 1; IL <= NREFPT; ++IL) {

            auto const &daylFacCurr = thisDayltgCtrl.daylFac[state.dataGlobal->HourOfDay](loop, IL)[iWinCover_Bare];
            auto const &daylFacPrev = thisDayltgCtrl.daylFac[state.dataGlobal->PreviousHour](loop, IL)[iWinCover_Bare];
            // Daylight factors for current sun position
            auto const &illFacCurr = daylFacCurr[iLum_Illum];
            auto const &illFacPrev = daylFacPrev[iLum_Illum];
            auto &dfhr = DFHR[iWinCover_Bare];
            auto const &backFacCurr = daylFacCurr[iLum_Back];
            auto const &backFacPrev = daylFacPrev[iLum_Back];
            auto &bfhr = BFHR[iWinCover_Bare];
            auto const &sourceFacCurr = daylFacCurr[iLum_Source];
            auto const &sourceFacPrev = daylFacPrev[iLum_Source];
            auto &sfhr = SFHR[iWinCover_Bare];

            auto const &daylFac2Curr = thisDayltgCtrl.daylFac[state.dataGlobal->HourOfDay](loop, IL)[iWinCover_Shaded];
            auto const &daylFac2Prev = thisDayltgCtrl.daylFac[state.dataGlobal->PreviousHour](loop, IL)[iWinCover_Shaded];

            auto const &illFac2Curr = daylFac2Curr[iLum_Illum];
            auto const &illFac2Prev = daylFac2Prev[iLum_Illum];
            auto &dfhr2 = DFHR[iWinCover_Shaded];
            auto const &backFac2Curr = daylFac2Curr[iLum_Back];
            auto const &backFac2Prev = daylFac2Prev[iLum_Back];
            auto &bfhr2 = BFHR[iWinCover_Shaded];
            auto const &sourceFac2Curr = daylFac2Curr[iLum_Source];
            auto const &sourceFac2Prev = daylFac2Prev[iLum_Source];
            auto &sfhr2 = SFHR[iWinCover_Shaded];

#ifdef GET_OUT
            auto const &daylFacShCurr = thisDayltgCtrl.daylFac[state.dataGlobal->HourOfDay](loop, IL)[iWinCover_Shaded];
            auto const &daylFacShPrev = thisDayltgCtrl.daylFac[state.dataGlobal->PreviousHour](loop, IL)[iWinCover_Shaded];

            auto const &illFacShCurr = daylFacShCurr[iLum_Illum];
            auto const &illFacShPrev = daylFacShPrev[iLum_Illum];

            auto const &backFacShCurr = daylFacShCurr[iLum_Back];
            auto const &backFacShPrev = daylFacShPrev[iLum_Back];

            auto const &sourceFacShCurr = daylFacShCurr[iLum_Source];
            auto const &sourceFacShPrev = daylFacShPrev[iLum_Source];
#endif // GET_OUT
            for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {

                // ===Bare window===
                // Sky daylight factor for sky type (second index), bare/shaded window (first index)
                dfhr.sky[iSky] = VTRatio * (wgtCurrHr * illFacCurr.sky[iSky] + wgtPrevHr * illFacPrev.sky[iSky]);
                bfhr.sky[iSky] = VTRatio * (wgtCurrHr * backFacCurr.sky[iSky] + wgtPrevHr * backFacPrev.sky[iSky]);
                sfhr.sky[iSky] = VTRatio * (wgtCurrHr * sourceFacCurr.sky[iSky] + wgtPrevHr * sourceFacPrev.sky[iSky]);

                if (ShadedOrDiffusingGlassWin) {

                    // ===Shaded window or window with diffusing glass===
                    // Shade, screen, blind with fixed slats, or diffusing glass
                    dfhr2.sky[iSky] = VTRatio * (wgtCurrHr * illFac2Curr.sky[iSky] + wgtPrevHr * illFac2Prev.sky[iSky]);
                    bfhr2.sky[iSky] = VTRatio * (wgtCurrHr * backFac2Curr.sky[iSky] + wgtPrevHr * backFac2Prev.sky[iSky]);
                    sfhr2.sky[iSky] = VTRatio * (wgtCurrHr * sourceFac2Curr.sky[iSky] + wgtPrevHr * sourceFac2Prev.sky[iSky]);
                } // End of check if window is shaded or has diffusing glass
            } // for (iSky)

            // Sun daylight factor for bare/shaded window
            DFHR[iWinCover_Bare].sun =
                VTRatio * (wgtCurrHr * (illFacCurr.sun + illFacCurr.sunDisk) + wgtPrevHr * (illFacPrev.sun + illFacPrev.sunDisk));

            // Sun background luminance factor for bare/shaded window
            BFHR[iWinCover_Bare].sun =
                VTRatio * (wgtCurrHr * (backFacCurr.sun + backFacCurr.sunDisk) + wgtPrevHr * (backFacPrev.sun + backFacPrev.sunDisk));

            // Sun source luminance factor for bare/shaded window
            SFHR[iWinCover_Bare].sun =
                VTRatio * (wgtCurrHr * (sourceFacCurr.sun + sourceFacCurr.sunDisk) + wgtPrevHr * (sourceFacPrev.sun + sourceFacPrev.sunDisk));

            if (ShadedOrDiffusingGlassWin) {

                // ===Shaded window or window with diffusing glass===
                // Shade, screen, blind with fixed slats, or diffusing glass
                DFHR[iWinCover_Shaded].sun = VTRatio * (wgtCurrHr * illFac2Curr.sun + wgtPrevHr * illFac2Prev.sun);
                BFHR[iWinCover_Shaded].sun = VTRatio * (wgtCurrHr * backFac2Curr.sun + wgtPrevHr * backFac2Prev.sun);
                SFHR[iWinCover_Shaded].sun = VTRatio * (wgtCurrHr * sourceFac2Curr.sun + wgtPrevHr * sourceFac2Prev.sun);

                auto const &surfShade = s_surf->surfShades(IWin);
                if (!surfShade.blind.slatBlockBeam) {
                    DFHR[iWinCover_Shaded].sun += VTRatio * (wgtCurrHr * illFac2Curr.sunDisk + wgtPrevHr * illFac2Prev.sunDisk);
                    BFHR[iWinCover_Shaded].sun += VTRatio * (wgtCurrHr * backFac2Curr.sunDisk + wgtPrevHr * backFac2Prev.sunDisk);
                    SFHR[iWinCover_Shaded].sun += VTRatio * (wgtCurrHr * sourceFac2Curr.sunDisk + wgtPrevHr * sourceFac2Prev.sunDisk);
                }
            } // End of check if window is shaded or has diffusing glass

            // Get illuminance at ref point from bare and shaded window by
            // multiplying daylight factors by exterior horizontal illuminance

            // Adding 0.001 in the following prevents zero HorIllSky in early morning or late evening when sun
            // is up in the present time step but GILSK(ISky,HourOfDay) and GILSK(ISky,NextHour) are both zero.
            auto const &gilskCurr = dl->horIllum[state.dataGlobal->HourOfDay];
            auto const &gilskPrev = dl->horIllum[state.dataGlobal->PreviousHour];

            // HISKF is current time step horizontal illuminance from sky, calculated in DayltgLuminousEfficacy,
            // which is called in WeatherManager. HISUNF is current time step horizontal illuminance from sun,
            // also calculated in DayltgLuminousEfficacy.
            Real64 horIllSky1 =
                state.dataGlobal->WeightNow * gilskCurr.sky[iSky1] + state.dataGlobal->WeightPreviousHour * gilskPrev.sky[iSky1] + 0.001;
            Real64 horIllSky2 =
                state.dataGlobal->WeightNow * gilskCurr.sky[iSky2] + state.dataGlobal->WeightPreviousHour * gilskPrev.sky[iSky2] + 0.001;

            HorIllSkyFac = state.dataEnvrn->HISKF / ((1 - SkyWeight) * horIllSky2 + SkyWeight * horIllSky1);

            auto &daylFromWinAtRefPt = thisDayltgCtrl.refPts(IL).extWins(loop).lums;
            auto &tmpDayl = tmpDaylFromWinAtRefPt(IL, loop);
            for (int iWinCover = 0; iWinCover < (int)WinCover::Num; ++iWinCover) {
                auto const &dfhr3 = DFHR[iWinCover];
                auto const &bfhr3 = BFHR[iWinCover];
                auto const &sfhr3 = SFHR[iWinCover];

                // What is this?
                if (iWinCover == iWinCover_Shaded && !ShadedOrDiffusingGlassWin) {
                    break;
                }

                daylFromWinAtRefPt[iLum_Illum][iWinCover] =
                    dfhr3.sun * state.dataEnvrn->HISUNF +
                    HorIllSkyFac * (dfhr3.sky[iSky1] * SkyWeight * horIllSky1 + dfhr3.sky[iSky2] * (1.0 - SkyWeight) * horIllSky2);
                daylFromWinAtRefPt[iLum_Back][iWinCover] =
                    bfhr3.sun * state.dataEnvrn->HISUNF +
                    HorIllSkyFac * (bfhr3.sky[iSky1] * SkyWeight * horIllSky1 + bfhr3.sky[iSky2] * (1.0 - SkyWeight) * horIllSky2);
                daylFromWinAtRefPt[iLum_Source][iWinCover] =
                    sfhr3.sun * state.dataEnvrn->HISUNF +
                    HorIllSkyFac * (sfhr3.sky[iSky1] * SkyWeight * horIllSky1 + sfhr3.sky[iSky2] * (1.0 - SkyWeight) * horIllSky2);

                daylFromWinAtRefPt[iLum_Source][iWinCover] = max(daylFromWinAtRefPt[iLum_Source][iWinCover], 0.0);

                // Added TH 1/21/2010 - save the original clear and dark (fully switched) states'
                //  zone daylighting values, needed for switachable glazings
                tmpDayl[iLum_Illum][iWinCover] = daylFromWinAtRefPt[iLum_Illum][iWinCover];
                tmpDayl[iLum_Back][iWinCover] = daylFromWinAtRefPt[iLum_Back][iWinCover];
                tmpDayl[iLum_Source][iWinCover] = daylFromWinAtRefPt[iLum_Source][iWinCover];
            } // for for (iWinCover)

        } // End of reference point loop, IL
    } // End of first loop over exterior windows associated with this zone

    // Initialize flag that one or more windows has switchable glazing
    // control that adjusts visible transmittance to just meet dayltg setpoint
    // (and the window has not already been switched)
    ISWFLG = 0;

    // Second loop over windows. Find total daylight illuminance and background luminance
    // for each ref pt from all exterior windows associated with the zone.  Use shading flags.
    // This illuminance excludes contribution of inter-reflected illuminance produced by solar
    // entering the zone through interior windows (which is calculated in DayltgInterReflIllFrIntWins.

    for (int loop = 1; loop <= thisEnclDaylight.NumOfDayltgExtWins; ++loop) {
        int IWin = thisEnclDaylight.DayltgExtWinSurfNums(loop);
        int ICtrl = s_surf->Surface(IWin).activeWindowShadingControl;
        if (s_surf->Surface(IWin).HasShadeControl && ISWFLG == 0) {
            if (s_surf->WindowShadingControl(ICtrl).shadingControlType == WindowShadingControlType::MeetDaylIlumSetp &&
                s_surf->SurfWinShadingFlag(IWin) == WinShadingType::GlassConditionallyLightened) {
                ISWFLG = 1;
            }
        }

        // Determine if illuminance contribution is from bare or shaded window
        //  For switchable glazings with shading control type of WSCT_MeetDaylIlumSetp,
        //   the shading flag is initialized at GlassConditionallyLightened (20), and
        //   the window is initialized at clear state: IS = 1
        //  For other windows with glare control, the shading flag is initialized at >10, to be determined
        WinCover winCover = findWinShadingStatus(state, IWin);

        for (int IL = 1; IL <= NREFPT; ++IL) {
            auto &refPt = thisDayltgCtrl.refPts(IL);
            dl->DaylIllum(IL) += refPt.extWins(loop).lums[iLum_Illum][(int)winCover];
            refPt.lums[iLum_Back] += refPt.extWins(loop).lums[iLum_Back][(int)winCover];
        }
    } // End of second window loop over exterior windows associated with this zone

    // Optical switching control (e.g. electrochromic glass) to adjust
    // window's vis trans downward so daylight level equals or is as
    // close as possible to the illuminance setpoint at first reference point.
    // Assumes vis trans in the fully switched state is less than that in the
    // unswitched state. Assumes some windows in a space may have this control and
    // others not.

    int count = 0;

    // If daylight illuminance is above setpoint, allow switching
    if (ISWFLG != 0 && dl->DaylIllum(1) > SetPnt(1)) {

        // array of flags to indicate that previously groups would have already shaded this window
        Array1D_bool previously_shaded;
        previously_shaded.dimension(dl->maxDayltgExtWins, false);

        // Third loop over windows.  Get illuminance at ref pt 1 from
        // windows that can be switched (DILLSW) with a group and those that can't (DILLUN).
        // Windows that can be switched are initially in the unswitched state. For subsequent
        // groups the windows in previous groups are fully switched.
        for (auto &shadeGroupLum : shadeGroupsLums) {
            shadeGroupLum.switchedWinLum = shadeGroupLum.unswitchedWinLum = 0.0;
        }

        for (std::size_t igroup = 1; igroup <= thisDayltgCtrl.ShadeDeployOrderExtWins.size(); igroup++) {
            auto &shadeGroupLums = shadeGroupsLums(igroup);
            std::vector<int> const &listOfExtWin = thisDayltgCtrl.ShadeDeployOrderExtWins[igroup - 1];
            for (const int IWin : listOfExtWin) {
                ++count;
                // need to map back to the original order of the "loop" to not change all the other data structures
                int loop = thisDayltgCtrl.MapShdOrdToLoopNum(count);
                if (loop == 0) {
                    continue;
                }

                if (!s_surf->Surface(IWin).HasShadeControl) {
                    continue;
                }

                int ICtrl = s_surf->Surface(IWin).activeWindowShadingControl;
                WinCover winCover = findWinShadingStatus(state, IWin);

                auto const &daylFromWinAtRefPt = thisDayltgCtrl.refPts(1).extWins(loop).lums[iLum_Illum];
                if (s_surf->SurfWinShadingFlag(IWin) == WinShadingType::GlassConditionallyLightened &&
                    s_surf->WindowShadingControl(ICtrl).shadingControlType == WindowShadingControlType::MeetDaylIlumSetp &&
                    !previously_shaded(loop)) {
                    shadeGroupLums.switchedWinLum += daylFromWinAtRefPt[(int)winCover];
                    previously_shaded(loop) = true;
                } else {
                    shadeGroupLums.unswitchedWinLum +=
                        !previously_shaded(loop) ? daylFromWinAtRefPt[(int)winCover] : daylFromWinAtRefPt[iWinCover_Shaded];
                }
            } // for (IWin)
        } // for (igroup)

        // Transmittance multiplier
        for (auto &shadeGroupLums : shadeGroupsLums) {
            shadeGroupLums.lumRatio = (SetPnt(1) - shadeGroupLums.unswitchedWinLum) / (shadeGroupLums.switchedWinLum + 0.00001);
        }

        // ASETIL < 1 means there's enough light, so check for switching

        // Fourth loop over windows to determine which to switch
        // iterate in the order that the shades are specified in WindowShadeControl
        count = 0;
        breakOuterLoop = false;
        continueOuterLoop = false;
        for (std::size_t igroup = 1; igroup <= thisDayltgCtrl.ShadeDeployOrderExtWins.size(); igroup++) {
            auto &shadeGroupLums = shadeGroupsLums(igroup);
            std::vector<int> const &listOfExtWin = thisDayltgCtrl.ShadeDeployOrderExtWins[igroup - 1];

            for (const int IWin : listOfExtWin) {
                ++count;
                auto const &surfWin = s_surf->SurfaceWindow(IWin);
                // need to map back to the original order of the "loop" to not change all the other data structures
                int loop = thisDayltgCtrl.MapShdOrdToLoopNum(count);
                if (loop > 0 && shadeGroupLums.lumRatio < 1.0) {

                    int ICtrl = s_surf->Surface(IWin).activeWindowShadingControl;
                    if (!s_surf->Surface(IWin).HasShadeControl) {
                        continueOuterLoop = true;
                        continue;
                    }
                    if (s_surf->SurfWinShadingFlag(IWin) != WinShadingType::GlassConditionallyLightened ||
                        s_surf->WindowShadingControl(ICtrl).shadingControlType != WindowShadingControlType::MeetDaylIlumSetp) {
                        continueOuterLoop = true;
                        continue;
                    }

                    int const IConst = s_surf->SurfActiveConstruction(IWin);
                    // Vis trans at normal incidence of unswitched glass
                    shadeGroupLums.unswitchedTvis =
                        Window::POLYF(1.0, state.dataConstruction->Construct(IConst).TransVisBeamCoef) * surfWin.glazedFrac;

                    // Vis trans at normal incidence of fully switched glass
                    int const IConstShaded = s_surf->Surface(IWin).activeShadedConstruction;
                    shadeGroupLums.switchedTvis =
                        Window::POLYF(1.0, state.dataConstruction->Construct(IConstShaded).TransVisBeamCoef) * surfWin.glazedFrac;

                    // Reset shading flag to indicate that window is shaded by being partially or fully switched
                    s_surf->SurfWinShadingFlag(IWin) = WinShadingType::SwitchableGlazing;

                    // ASETIL < 0 means illuminance from non-daylight-switchable windows exceeds setpoint,
                    // so completely switch all daylight-switchable windows to minimize solar gain
                    if (shadeGroupLums.lumRatio <= 0.0) {
                        s_surf->SurfWinSwitchingFactor(IWin) = 1.0;
                        s_surf->SurfWinVisTransSelected(IWin) = shadeGroupLums.switchedTvis;
                    } else {
                        // Case where 0 < ASETIL < 1: darken glass in all
                        // daylight-switchable windows to just meet illuminance setpoint
                        // From this equation: SETPNT(1) = DILLUN + DILLSW/TVIS1 * VisTransSelected
                        s_surf->SurfWinVisTransSelected(IWin) =
                            max(shadeGroupLums.switchedTvis, shadeGroupLums.lumRatio * shadeGroupLums.unswitchedTvis) + 0.000001;
                        s_surf->SurfWinSwitchingFactor(IWin) = (shadeGroupLums.unswitchedTvis - s_surf->SurfWinVisTransSelected(IWin)) /
                                                               (shadeGroupLums.unswitchedTvis - shadeGroupLums.switchedTvis + 0.000001);
                        // bound switching factor between 0 and 1
                        s_surf->SurfWinSwitchingFactor(IWin) = min(1.0, s_surf->SurfWinSwitchingFactor(IWin));
                        s_surf->SurfWinSwitchingFactor(IWin) = max(0.0, s_surf->SurfWinSwitchingFactor(IWin));
                    }

                    // Adjust daylight quantities based on ratio between switched and unswitched visible transmittance
                    for (int IL = 1; IL <= NREFPT; ++IL) {
                        // DaylIllum(IL) and BacLum(IL) were calculated at the clear state:
                        //  and need to adjusted for intermediate switched state at VisTransSelected:
                        auto &daylFromWinAtRefPt = thisDayltgCtrl.refPts(IL).extWins(loop).lums;
                        auto const &tmpDayl = tmpDaylFromWinAtRefPt(IL, loop);

                        VTRAT = s_surf->SurfWinVisTransSelected(IWin) / (shadeGroupLums.unswitchedTvis + 0.000001);
                        dl->DaylIllum(IL) += (VTRAT - 1.0) * daylFromWinAtRefPt[iLum_Illum][iWinCover_Bare];
                        thisDayltgCtrl.refPts(IL).lums[iLum_Back] += (VTRAT - 1.0) * daylFromWinAtRefPt[iLum_Back][iWinCover_Bare];

                        // Adjust illum, background illum and source luminance for this window in intermediate switched state
                        //  for later use in the DayltgGlare calc because SurfaceWindow(IWin)%ShadingFlag = WinShadingType::SwitchableGlazing = 2
                        VTRAT = s_surf->SurfWinVisTransSelected(IWin) / (shadeGroupLums.switchedTvis + 0.000001);
                        daylFromWinAtRefPt[iLum_Illum][iWinCover_Shaded] = VTRAT * tmpDayl[iLum_Illum][iWinCover_Shaded];
                        daylFromWinAtRefPt[iLum_Back][iWinCover_Shaded] = VTRAT * tmpDayl[iLum_Back][iWinCover_Shaded];
                        daylFromWinAtRefPt[iLum_Source][iWinCover_Shaded] = VTRAT * tmpDayl[iLum_Source][iWinCover_Shaded];
                    } // for (IL)
                } // if (loop > 0 && ASETIL < 1)
                // If new daylight does not exceed the illuminance setpoint, done, no more checking other groups of switchable glazings
                if (dl->DaylIllum(1) <= SetPnt(1)) {
                    breakOuterLoop = true;
                    break;
                }
            } // for (Win)
            if (breakOuterLoop) {
                break;
            }
            if (continueOuterLoop) {
                continue;
            }
        } // for (igroup)

    } // ISWFLG /= 0 .AND. DaylIllum(1) > SETPNT(1)

    // loop over windows to do luminance based control
    count = 0;
    for (int igroup = 1; igroup <= (int)thisDayltgCtrl.ShadeDeployOrderExtWins.size(); igroup++) {
        for (int const IWin : thisDayltgCtrl.ShadeDeployOrderExtWins[igroup - 1]) {
            ++count;
            int ICtrl = s_surf->Surface(IWin).activeWindowShadingControl;
            WindowShadingControlType shCtrlType = s_surf->WindowShadingControl(ICtrl).shadingControlType;
            if (!((shCtrlType == WindowShadingControlType::HiSolar_HiLumin_OffMidNight) ||
                  (shCtrlType == WindowShadingControlType::HiSolar_HiLumin_OffSunset) ||
                  (shCtrlType == WindowShadingControlType::HiSolar_HiLumin_OffNextMorning))) {
                continue;
            }
            // need to map back to the original order of the "loop" to not change all the other data structures
            int loop = thisDayltgCtrl.MapShdOrdToLoopNum(count);
            if (loop == 0) {
                continue;
            }

            WinShadingType currentFlag = s_surf->SurfWinShadingFlag(IWin);
            WinShadingType ShType = s_surf->WindowShadingControl(ICtrl).ShadingType;
            if ((currentFlag != WinShadingType::IntShadeConditionallyOff) && (currentFlag != WinShadingType::GlassConditionallyLightened) &&
                (currentFlag != WinShadingType::ExtShadeConditionallyOff) && (currentFlag != WinShadingType::IntBlindConditionallyOff) &&
                (currentFlag != WinShadingType::ExtBlindConditionallyOff) && (currentFlag != WinShadingType::BGShadeConditionallyOff) &&
                (currentFlag != WinShadingType::BGBlindConditionallyOff)) {
                continue;
            }

            auto const &daylFromWinAtRefPt = thisDayltgCtrl.refPts(1).extWins(loop).lums;
            if (daylFromWinAtRefPt[iLum_Source][iWinCover_Bare] > s_surf->WindowShadingControl(ICtrl).SetPoint2) {
                // shade on if luminance of this window is above setpoint
                s_surf->SurfWinShadingFlag(IWin) = ShType;
                // update total illuminance and background luminance
                for (int IL = 1; IL <= NREFPT; ++IL) {
                    dl->DaylIllum(IL) += daylFromWinAtRefPt[iLum_Illum][iWinCover_Shaded] - daylFromWinAtRefPt[iLum_Illum][iWinCover_Bare];
                    thisDayltgCtrl.refPts(IL).lums[iLum_Back] +=
                        daylFromWinAtRefPt[iLum_Back][iWinCover_Shaded] - daylFromWinAtRefPt[iLum_Back][iWinCover_Bare];
                }
            } else {
                // shade off if luminance is below setpoint
                s_surf->SurfWinShadingFlag(IWin) = WinShadingType::ShadeOff;
            }
        } // for (IWin)
    } // for (igroup)

    // Calculate glare index at each reference point assuming the daylight illuminance setpoint is
    //  met at both reference points, either by daylight or electric lights
    for (int IL = 1; IL <= NREFPT; ++IL) {
        auto &refPt = thisDayltgCtrl.refPts(IL);
        BACL = max(SetPnt(IL) * dl->enclDaylight(enclNum).aveVisDiffReflect / Constant::Pi, refPt.lums[iLum_Back]);
        // DayltgGlare uses ZoneDaylight(ZoneNum)%SourceLumFromWinAtRefPt(IL,1,loop) for unshaded windows, and
        //  ZoneDaylight(ZoneNum)%SourceLumFromWinAtRefPt(IL,2,loop) for shaded windows
        refPt.glareIndex = DayltgGlare(state, IL, BACL, daylightCtrlNum);
    }

    // Check if glare level is less than maximum allowed at each ref pt.  If maximum
    // is exceeded at either ref pt, attempt to reduce glare to acceptable level by closing
    // shading device on windows that have shades that have not already been closed.
    GlareFlag = false;
    for (auto const &refPt : thisDayltgCtrl.refPts) {
        if (refPt.glareIndex > thisDayltgCtrl.MaxGlareallowed) {
            GlareFlag = true;
            break;
        }
    }

    if (GlareFlag) {
        bool blnCycle;
        bool GlareOK;
        Real64 tmpMult;
        // Glare is too high at a ref pt.  Loop through windows.
        count = 0;

        continueOuterLoop = false;
        for (std::size_t igroup = 1; igroup <= thisDayltgCtrl.ShadeDeployOrderExtWins.size(); igroup++) {
            auto &shadeGroupLums = shadeGroupsLums(igroup);
            std::vector<int> const &listOfExtWin = thisDayltgCtrl.ShadeDeployOrderExtWins[igroup - 1];

            int countBeforeListOfExtWinLoop = count;
            bool atLeastOneGlareControlIsActive = false;

            for (const int IWin : listOfExtWin) {
                ++count;
                // need to map back to the original order of the "loop" to not change all the other data structures
                int loop = thisDayltgCtrl.MapShdOrdToLoopNum(count);
                if (loop == 0) {
                    continue;
                }

                auto const &surfWin = s_surf->SurfaceWindow(IWin);
                // Check if window is eligible for glare control
                // TH 1/21/2010. Switchable glazings already in partially switched state
                //  should be allowed to further dim to control glare
                // if (SurfWinShadingFlag(IWin) <= BGBlind && SurfWinShadingFlag(IWin) != SwitchableGlazing) {
                if (NOT_SHADED(s_surf->SurfWinShadingFlag(IWin)) || ANY_SHADE_SCREEN(s_surf->SurfWinShadingFlag(IWin)) ||
                    ANY_BLIND(s_surf->SurfWinShadingFlag(IWin))) {
                    continueOuterLoop = false;
                    continue;
                }
                int ICtrl = s_surf->Surface(IWin).activeWindowShadingControl;
                if (!s_surf->Surface(IWin).HasShadeControl) {
                    continueOuterLoop = false;
                    continue;
                }
                if (s_surf->WindowShadingControl(ICtrl).GlareControlIsActive) {
                    atLeastOneGlareControlIsActive = true;

                    // Illuminance (WDAYIL) and background luminance (WBACLU) contribution from this
                    // window without shading (IS=1) and with shading (IS=2) for each ref pt
                    //  For switchable windows, this may be partially switched rather than fully dark
                    for (int IL = 1; IL <= NREFPT; ++IL) {
                        auto const &daylFromWinAtRefPt = thisDayltgCtrl.refPts(IL).extWins(loop).lums;
                        for (int iWinCover = 0; iWinCover < (int)WinCover::Num; ++iWinCover) {
                            shadeGroupLums.WDAYIL(IL)[iLum_Illum][iWinCover] = daylFromWinAtRefPt[iLum_Illum][iWinCover];
                            shadeGroupLums.WDAYIL(IL)[iLum_Back][iWinCover] = daylFromWinAtRefPt[iLum_Back][iWinCover];
                        }
                    }

                    // Recalculate illuminance and glare with shading on this window.
                    //  For switchable glazings, this is the fully switched (dark) state
                    for (int IL = 1; IL <= NREFPT; ++IL) {
                        auto &rdayil = shadeGroupLums.RDAYIL(IL);
                        auto const &wdayil = shadeGroupLums.WDAYIL(IL);
                        auto const &refPt = thisDayltgCtrl.refPts(IL);

                        if (s_surf->SurfWinShadingFlag(IWin) != WinShadingType::SwitchableGlazing) {
                            // for non switchable glazings or switchable glazings not switched yet (still in clear state)
                            //  SurfaceWindow(IWin)%ShadingFlag = WinShadingFlag::GlassConditionallyLightened
                            rdayil[iLum_Illum] = dl->DaylIllum(IL) - wdayil[iLum_Illum][iWinCover_Bare] + wdayil[iLum_Illum][iWinCover_Shaded];
                            rdayil[iLum_Back] = refPt.lums[iLum_Back] - wdayil[iLum_Back][iWinCover_Bare] + wdayil[iLum_Back][iWinCover_Shaded];
                        } else {
                            // switchable glazings already in partially switched state when calc the RDAYIL(IL) & RBACLU(IL)
                            auto const &tmpDayl = tmpDaylFromWinAtRefPt(loop, IL);
                            rdayil[iLum_Illum] = dl->DaylIllum(IL) - wdayil[iLum_Illum][iWinCover_Shaded] + tmpDayl[iLum_Illum][iWinCover_Shaded];
                            rdayil[iLum_Back] = refPt.lums[iLum_Back] - wdayil[iLum_Back][iWinCover_Shaded] + tmpDayl[iLum_Back][iWinCover_Shaded];
                        }
                    } // for (IL)

                    if (s_surf->SurfWinShadingFlag(IWin) == WinShadingType::GlassConditionallyLightened) {
                        s_surf->SurfWinShadingFlag(IWin) = WinShadingType::SwitchableGlazing;
                    } else if (s_surf->SurfWinShadingFlag(IWin) == WinShadingType::IntShadeConditionallyOff) {
                        s_surf->SurfWinShadingFlag(IWin) = WinShadingType::IntShade;
                    } else if (s_surf->SurfWinShadingFlag(IWin) == WinShadingType::ExtShadeConditionallyOff) {
                        s_surf->SurfWinShadingFlag(IWin) = WinShadingType::ExtShade;
                    } else if (s_surf->SurfWinShadingFlag(IWin) == WinShadingType::IntBlindConditionallyOff) {
                        s_surf->SurfWinShadingFlag(IWin) = WinShadingType::IntBlind;
                    } else if (s_surf->SurfWinShadingFlag(IWin) == WinShadingType::ExtBlindConditionallyOff) {
                        s_surf->SurfWinShadingFlag(IWin) = WinShadingType::ExtBlind;
                    } else if (s_surf->SurfWinShadingFlag(IWin) == WinShadingType::BGShadeConditionallyOff) {
                        s_surf->SurfWinShadingFlag(IWin) = WinShadingType::BGShade;
                    } else if (s_surf->SurfWinShadingFlag(IWin) == WinShadingType::BGBlindConditionallyOff) {
                        s_surf->SurfWinShadingFlag(IWin) = WinShadingType::BGBlind;
                    }

                    // For switchable glazings, it is switched to fully dark state,
                    // update ZoneDaylight(ZoneNum)%SourceLumFromWinAtRefPt(IL,2,loop) for use in DayltgGlare
                    if (s_surf->SurfWinShadingFlag(IWin) == WinShadingType::SwitchableGlazing) {
                        for (int IL = 1; IL <= NREFPT; ++IL) {
                            auto &daylFromWinAtRefPt = thisDayltgCtrl.refPts(IL).extWins(loop).lums;
                            auto const &tmpDayl = tmpDaylFromWinAtRefPt(IL, loop);

                            daylFromWinAtRefPt[iLum_Source][iWinCover_Shaded] = tmpDayl[iLum_Source][iWinCover_Shaded];
                            daylFromWinAtRefPt[iLum_Illum][iWinCover_Shaded] = tmpDayl[iLum_Illum][iWinCover_Shaded];
                            daylFromWinAtRefPt[iLum_Back][iWinCover_Shaded] = tmpDayl[iLum_Back][iWinCover_Shaded];
                        }

                        int const IConst = s_surf->SurfActiveConstruction(IWin);
                        // Vis trans at normal incidence of unswitched glass
                        shadeGroupLums.unswitchedTvis =
                            Window::POLYF(1.0, state.dataConstruction->Construct(IConst).TransVisBeamCoef) * surfWin.glazedFrac;

                        // Vis trans at normal incidence of fully switched glass
                        int const IConstShaded = s_surf->Surface(IWin).activeShadedConstruction;
                        shadeGroupLums.switchedTvis =
                            Window::POLYF(1.0, state.dataConstruction->Construct(IConstShaded).TransVisBeamCoef) * surfWin.glazedFrac;
                    } // if (switchableGlazing)
                } // if (GlareControlIsActive)
            } // for (IWin)
            if (continueOuterLoop) {
                continue;
            }

            if (atLeastOneGlareControlIsActive) {

                // Re-calc daylight and glare at shaded state. For switchable glazings, it is the fully dark state.
                for (int IL = 1; IL <= NREFPT; ++IL) {
                    BACL = max(SetPnt(IL) * dl->enclDaylight(enclNum).aveVisDiffReflect / Constant::Pi, shadeGroupLums.RDAYIL(IL)[iLum_Back]);
                    // DayltgGlare uses ZoneDaylight(ZoneNum)%SourceLumFromWinAtRefPt(IL,2,loop) for shaded state
                    GLRNEW(IL) = DayltgGlare(state, IL, BACL, daylightCtrlNum);
                }

                // Check if the shading did not improve the glare conditions
                //
                // blnCycle when true resets the specific window to its non-shaded condition. A later comment says
                //      Shading this window has not improved the glare situation.
                //      Reset shading flag to no shading condition, go to next window.
                //
                // If the original glare was too high at all reference points and the new glare is lower at all reference points it is good, don't
                // reset it. For each reference point, if the original glare was too high but ok at other reference points and the glare gets
                // lower at the reference and stays ok at the other reference points it is good, don't reset it.
                //
                // The old comments when there were only two reference points were:
                //     One ref pt;  go to next window if glare has increased.
                //     Two ref pts.  There are three cases depending on glare values.
                //         (1) Initial glare too high at both ref pts.  Deploy shading on
                //             this window if this decreases glare at both ref pts.
                //         (2) Initial glare too high only at first ref pt.  Deploy shading
                //             on this window if glare at first ref pt decreases and
                //             glare at second ref pt stays below max.
                //         (3) Initial glare too high at second ref pt.  Deploy shading if glare
                //             at second ref pt decreases and glare at first ref pt stays below max.
                //
                // The approach taken is just to count the number of reference points that fulfill the individual requirements and see if it
                // covers all the reference points.
                int numRefPtOldAboveMaxNewBelowOld = 0;
                int numRefPtOldBelowMaxNewBelowMax = 0;
                for (int IL = 1; IL <= NREFPT; ++IL) {
                    auto const &refPt = thisDayltgCtrl.refPts(IL);

                    if (refPt.glareIndex > thisDayltgCtrl.MaxGlareallowed && GLRNEW(IL) <= refPt.glareIndex) {
                        ++numRefPtOldAboveMaxNewBelowOld;
                    } else if (refPt.glareIndex <= thisDayltgCtrl.MaxGlareallowed && GLRNEW(IL) <= thisDayltgCtrl.MaxGlareallowed) {
                        ++numRefPtOldBelowMaxNewBelowMax;
                    }
                }
                blnCycle = true;
                if ((numRefPtOldAboveMaxNewBelowOld + numRefPtOldBelowMaxNewBelowMax) == NREFPT) {
                    blnCycle = false;
                }
            }

            // restore the count to the value prior to the last loop through the group of exterior windows
            count = countBeforeListOfExtWinLoop;
            breakOuterLoop = false;

            for (const int IWin : listOfExtWin) {
                ++count;
                // need to map back to the original order of the "loop" to not change all the other data structures
                int loop = thisDayltgCtrl.MapShdOrdToLoopNum(count);
                if (loop == 0) {
                    continue;
                }

                // if (SurfWinShadingFlag(IWin) <= BGBlind && SurfWinShadingFlag(IWin) != SwitchableGlazing) {
                if (NOT_SHADED(s_surf->SurfWinShadingFlag(IWin)) || ANY_SHADE_SCREEN(s_surf->SurfWinShadingFlag(IWin)) ||
                    ANY_BLIND(s_surf->SurfWinShadingFlag(IWin))) {
                    continue;
                }

                int ICtrl = s_surf->Surface(IWin).activeWindowShadingControl;
                if (!s_surf->Surface(IWin).HasShadeControl) {
                    continue;
                }
                if (s_surf->WindowShadingControl(ICtrl).GlareControlIsActive) {

                    // Shading this window has not improved the glare situation.
                    // Reset shading flag to no shading condition, go to next window.
                    if (blnCycle) {
                        //  for switchable glazings, reset properties to clear state or partial switched state?
                        if (s_surf->SurfWinShadingFlag(IWin) == WinShadingType::SwitchableGlazing) {
                            s_surf->SurfWinSwitchingFactor(IWin) = 0.0;
                            s_surf->SurfWinVisTransSelected(IWin) = shadeGroupLums.unswitchedTvis;

                            // RESET properties for fully dark state
                            for (int IL = 1; IL <= NREFPT; ++IL) {
                                auto &daylFromWinAtRefPt = thisDayltgCtrl.refPts(IL).extWins(loop).lums;
                                auto const &tmpDayl = tmpDaylFromWinAtRefPt(IL, loop);
                                daylFromWinAtRefPt[iLum_Illum][iWinCover_Shaded] = tmpDayl[iLum_Illum][iWinCover_Shaded];
                                daylFromWinAtRefPt[iLum_Source][iWinCover_Shaded] = tmpDayl[iLum_Source][iWinCover_Shaded];
                                daylFromWinAtRefPt[iLum_Back][iWinCover_Shaded] = tmpDayl[iLum_Back][iWinCover_Shaded];
                            }
                        }

                        s_surf->SurfWinShadingFlag(IWin) = WinShadingType::ShadeOff;
                        continue;
                    }

                    // Shading this window has improved the glare situation.
                    // Reset background luminance, glare index, and daylight illuminance at each ref pt.
                    // For switchable glazings, this is fully switched, dark state
                    for (int IL = 1; IL <= NREFPT; ++IL) {
                        auto &refPt = thisDayltgCtrl.refPts(IL);
                        refPt.lums[iLum_Back] = shadeGroupLums.RDAYIL(IL)[iLum_Back];
                        refPt.glareIndex = GLRNEW(IL);
                        dl->DaylIllum(IL) = shadeGroupLums.RDAYIL(IL)[iLum_Illum];
                    }

                    // TH comments (5/22/2009): seems for EC windows, if the calculated glare exceeds the max setpoint,
                    //  the EC windows will be reset to fully dark state which significantly reduces the available daylight.
                    //  A better way is to dim the EC windows as necessary just to meet the glare index, which will still
                    //  provide more daylight while not exceeding the max glare! The question is then how to set the
                    //  SwitchingFactor to just meet the glare index.
                    //  This was addressed in CR 7984 for E+ 5.0. 1/19/2010

                    // If switchable glazing, set switching factor to 1: fully switched.
                    if (s_surf->SurfWinShadingFlag(IWin) == WinShadingType::SwitchableGlazing) {
                        // tmpSWFactor0 = SurfaceWindow( IWin ).SwitchingFactor; // save original
                        // switching  factor
                        ////Unused Set but never used
                        s_surf->SurfWinSwitchingFactor(IWin) = 1.0;
                        s_surf->SurfWinVisTransSelected(IWin) = shadeGroupLums.switchedTvis;

                        // restore fully dark values
                        for (int IL = 1; IL <= NREFPT; ++IL) {
                            auto &daylFromWinAtRefPt = thisDayltgCtrl.refPts(IL).extWins(loop).lums;
                            auto const &tmpDayl = tmpDaylFromWinAtRefPt(IL, loop);
                            auto &wdayil = shadeGroupLums.WDAYIL(IL);
                            wdayil[iLum_Illum][iWinCover_Shaded] = tmpDayl[iLum_Illum][iWinCover_Shaded];
                            wdayil[iLum_Back][iWinCover_Shaded] = tmpDayl[iLum_Back][iWinCover_Shaded];
                            daylFromWinAtRefPt[iLum_Illum][iWinCover_Shaded] = tmpDayl[iLum_Illum][iWinCover_Shaded];
                            daylFromWinAtRefPt[iLum_Back][iWinCover_Shaded] = tmpDayl[iLum_Back][iWinCover_Shaded];
                            daylFromWinAtRefPt[iLum_Source][iWinCover_Shaded] = tmpDayl[iLum_Source][iWinCover_Shaded];
                        }
                    }

                    // Check if glare now acceptable at each ref pt.
                    GlareOK = false;
                    if (NREFPT == 1) {
                        if (thisDayltgCtrl.refPts(1).glareIndex <= thisDayltgCtrl.MaxGlareallowed) {
                            GlareOK = true;
                        }
                    } else if (NREFPT > 1) {
                        if (thisDayltgCtrl.refPts(1).glareIndex <= thisDayltgCtrl.MaxGlareallowed &&
                            thisDayltgCtrl.refPts(2).glareIndex <= thisDayltgCtrl.MaxGlareallowed) {
                            GlareOK = true;
                        }
                    }

                    if (GlareOK) {
                        if (s_surf->SurfWinShadingFlag(IWin) == WinShadingType::SwitchableGlazing &&
                            s_surf->WindowShadingControl(ICtrl).shadingControlType == WindowShadingControlType::MeetDaylIlumSetp) {
                            // Added TH 1/14/2010
                            // Only for switchable glazings with MeetDaylightIlluminanceSetpoint control
                            // The glazing is in fully dark state, it might lighten a bit to provide more daylight
                            //  while meeting maximum discomfort glare index
                            // Iteration to find the right switching factor meeting the glare index

                            // get fully dark state values
                            Real64 tmpSWSL1 = tmpDaylFromWinAtRefPt(1, loop)[iLum_Source][iWinCover_Shaded];
                            Real64 tmpSWSL2 = (NREFPT > 1) ? tmpDaylFromWinAtRefPt(2, loop)[iLum_Source][iWinCover_Shaded] : 0.0;

                            // use simple fixed step search in iteraction, can be improved in future
                            Real64 tmpSWFactor = 1.0 - tmpSWIterStep;
                            while (tmpSWFactor > 0) {
                                // calc new glare at new switching state
                                for (int IL = 1; IL <= NREFPT; ++IL) {
                                    auto &rdayil = shadeGroupLums.RDAYIL(IL);
                                    auto const &wdayil = shadeGroupLums.WDAYIL(IL);
                                    auto &refPt = thisDayltgCtrl.refPts(IL);
                                    rdayil[iLum_Illum] =
                                        dl->DaylIllum(IL) +
                                        (wdayil[iLum_Illum][iWinCover_Bare] - wdayil[iLum_Illum][iWinCover_Shaded]) * (1.0 - tmpSWFactor);
                                    rdayil[iLum_Back] =
                                        refPt.lums[iLum_Back] +
                                        (wdayil[iLum_Back][iWinCover_Bare] - wdayil[iLum_Back][iWinCover_Shaded]) * (1.0 - tmpSWFactor);
                                    BACL = max(SetPnt(IL) * dl->enclDaylight(enclNum).aveVisDiffReflect / Constant::Pi, rdayil[iLum_Back]);
                                    // needs to update SourceLumFromWinAtRefPt(IL,2,loop) before re-calc DayltgGlare
                                    tmpMult = (shadeGroupLums.unswitchedTvis -
                                               (shadeGroupLums.unswitchedTvis - shadeGroupLums.switchedTvis) * tmpSWFactor) /
                                              shadeGroupLums.switchedTvis;
                                    refPt.extWins(loop).lums[iLum_Source][iWinCover_Shaded] = ((IL == 1) ? tmpSWSL1 : tmpSWSL2) * tmpMult;
                                    // Calc new glare
                                    GLRNEW(IL) = DayltgGlare(state, IL, BACL, daylightCtrlNum);
                                } // for (IL)

                                // Check whether new glare is OK
                                GlareOK = false;
                                if (NREFPT == 1) {
                                    if (GLRNEW(1) <= thisDayltgCtrl.MaxGlareallowed) {
                                        GlareOK = true;
                                    }
                                } else if (NREFPT > 1) {
                                    if (GLRNEW(1) <= thisDayltgCtrl.MaxGlareallowed && GLRNEW(2) <= thisDayltgCtrl.MaxGlareallowed) {
                                        GlareOK = true;
                                    }
                                }

                                if (GlareOK) {
                                    if (tmpSWFactor >= tmpSWIterStep) {
                                        // Continue to lighten the glazing
                                        tmpSWFactor -= tmpSWIterStep;
                                        continue;
                                    } else {
                                        // Glare still OK but glazing already in clear state, no more lighten
                                        breakOuterLoop = true;
                                        break;
                                    }
                                } else {
                                    // Glare too high, exit and use previous switching state
                                    tmpSWFactor += tmpSWIterStep;
                                    breakOuterLoop = true;
                                    break;
                                }
                            } // if (tmpSWFactor > 0)

                            // Final re-calculation if needed
                            if (!GlareOK) {
                                // Glare too high, use previous state and re-calc
                                for (int IL = 1; IL <= NREFPT; ++IL) {
                                    auto &rdayil = shadeGroupLums.RDAYIL(IL);
                                    auto const &wdayil = shadeGroupLums.WDAYIL(IL);
                                    rdayil[iLum_Illum] =
                                        dl->DaylIllum(IL) +
                                        (wdayil[iLum_Illum][iWinCover_Bare] - wdayil[iLum_Illum][iWinCover_Shaded]) * (1.0 - tmpSWFactor);
                                    rdayil[iLum_Back] =
                                        thisDayltgCtrl.refPts(IL).lums[iLum_Back] +
                                        (wdayil[iLum_Back][iWinCover_Bare] - wdayil[iLum_Back][iWinCover_Shaded]) * (1.0 - tmpSWFactor);
                                    BACL = max(SetPnt(IL) * dl->enclDaylight(enclNum).aveVisDiffReflect / Constant::Pi, rdayil[iLum_Back]);

                                    // needs to update SourceLumFromWinAtRefPt(IL,2,IWin) before re-calc DayltgGlare
                                    tmpMult = (shadeGroupLums.unswitchedTvis -
                                               (shadeGroupLums.unswitchedTvis - shadeGroupLums.switchedTvis) * tmpSWFactor) /
                                              shadeGroupLums.switchedTvis;
                                    thisDayltgCtrl.refPts(1).extWins(loop).lums[iLum_Source][iWinCover_Shaded] =
                                        ((IL == 1) ? tmpSWSL1 : tmpSWSL2) * tmpMult;
                                    GLRNEW(IL) = DayltgGlare(state, IL, BACL, daylightCtrlNum);
                                }
                            }

                            // Update final results
                            for (int IL = 1; IL <= NREFPT; ++IL) {
                                auto &refPt = thisDayltgCtrl.refPts(IL);
                                auto const &rdayil = shadeGroupLums.RDAYIL(IL);
                                refPt.lums[iLum_Back] = rdayil[iLum_Back];
                                refPt.glareIndex = GLRNEW(IL);
                                dl->DaylIllum(IL) = rdayil[iLum_Illum];

                                tmpMult =
                                    (shadeGroupLums.unswitchedTvis - (shadeGroupLums.unswitchedTvis - shadeGroupLums.switchedTvis) * tmpSWFactor) /
                                    shadeGroupLums.switchedTvis;
                                // update report variables
                                auto &daylFromWinAtRefPt = refPt.extWins(loop).lums;
                                auto const &tmpDayl = tmpDaylFromWinAtRefPt(IL, loop);
                                daylFromWinAtRefPt[iLum_Illum][iWinCover_Shaded] = tmpDayl[iLum_Illum][iWinCover_Shaded] * tmpMult;
                                daylFromWinAtRefPt[iLum_Back][iWinCover_Shaded] = tmpDayl[iLum_Back][iWinCover_Shaded] * tmpMult;
                            }
                            s_surf->SurfWinSwitchingFactor(IWin) = tmpSWFactor;
                            s_surf->SurfWinVisTransSelected(IWin) =
                                shadeGroupLums.unswitchedTvis - (shadeGroupLums.unswitchedTvis - shadeGroupLums.switchedTvis) * tmpSWFactor;

                        } else {
                            // For un-switchable glazing or switchable glazing but not MeetDaylightIlluminaceSetpoint control,
                            // it is in shaded state and glare is ok - job is done, exit the window loop - IWin
                            breakOuterLoop = true;
                            break;
                        }
                    } // if (glareOK)
                } // if (glareControlIsActive)
            } // for (IWin)
            if (breakOuterLoop) {
                break;
            }
        } // for (igroup)
    } // if (GlareFlag)

    // Loop again over windows and reset remaining shading flags that
    // are 10 or higher (i.e., conditionally off) to off
    for (int spaceNum : state.dataHeatBal->Zone(thisDayltgCtrl.zoneIndex).spaceIndexes) {
        auto const &thisSpace = state.dataHeatBal->space(spaceNum);
        for (int IWin = thisSpace.WindowSurfaceFirst; IWin <= thisSpace.WindowSurfaceLast; ++IWin) {
            if (s_surf->Surface(IWin).ExtBoundCond != ExternalEnvironment) {
                continue;
            }
            bool anyGlareControl = (s_surf->SurfWinShadingFlag(IWin) == WinShadingType::IntShadeConditionallyOff) ||
                                   (s_surf->SurfWinShadingFlag(IWin) == WinShadingType::GlassConditionallyLightened) ||
                                   (s_surf->SurfWinShadingFlag(IWin) == WinShadingType::ExtShadeConditionallyOff) ||
                                   (s_surf->SurfWinShadingFlag(IWin) == WinShadingType::IntBlindConditionallyOff) ||
                                   (s_surf->SurfWinShadingFlag(IWin) == WinShadingType::ExtBlindConditionallyOff);
            if (anyGlareControl) {
                s_surf->SurfWinShadingFlag(IWin) = WinShadingType::ShadeOff;
            }
        }
    }

    // Variables for reporting
    for (int IL = 1; IL <= NREFPT; ++IL) {
        auto &refPt = thisDayltgCtrl.refPts(IL);
        refPt.lums[iLum_Illum] = dl->DaylIllum(IL);

        // added TH 12/2/2008
        refPt.timeExceedingGlareIndexSetPoint = (refPt.glareIndex > thisDayltgCtrl.MaxGlareallowed) ? state.dataGlobal->TimeStepZone : 0.0;
        // added TH 7/6/2009
        refPt.timeExceedingDaylightIlluminanceSetPoint = (dl->DaylIllum(IL) > refPt.illumSetPoint) ? state.dataGlobal->TimeStepZone : 0.0;
    }
} // DayltgInteriorIllum()

void DayltgInteriorTDDIllum(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   October 2006

    // PURPOSE OF THIS SUBROUTINE:
    // Calculate the TDD Pipe illuminance values
    auto &dl = state.dataDayltg;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int iSky1; // Sky type index values for averaging two sky types
    int iSky2;
    Real64 SkyWeight; // Weighting factor used to average two different sky types

    if (state.dataEnvrn->SkyClearness > 3.0) { // Sky is average of clear and clear turbid
        SkyWeight = min(1.0, (state.dataEnvrn->SkyClearness - 3.0) / 3.0);
        iSky1 = (int)SkyType::Clear;
        iSky2 = (int)SkyType::ClearTurbid;
    } else if (state.dataEnvrn->SkyClearness > 1.2) { // Sky is average of clear turbid and intermediate
        SkyWeight = (state.dataEnvrn->SkyClearness - 1.2) / 1.8;
        iSky1 = (int)SkyType::ClearTurbid;
        iSky2 = (int)SkyType::Intermediate;
    } else { // Sky is average of intermediate and overcast
        SkyWeight = min(1.0, max(0.0, (state.dataEnvrn->SkyClearness - 1.0) / 0.2, (state.dataEnvrn->SkyBrightness - 0.05) / 0.4));
        iSky1 = (int)SkyType::Intermediate;
        iSky2 = (int)SkyType::Overcast;
    }

    // Calculate and report TDD visible transmittances
    for (int PipeNum = 1; PipeNum <= (int)state.dataDaylightingDevicesData->TDDPipe.size(); ++PipeNum) {

        state.dataDaylightingDevicesData->TDDPipe(PipeNum).TransVisBeam =
            state.dataGlobal->WeightNow * dl->TDDTransVisBeam(state.dataGlobal->HourOfDay, PipeNum) +
            state.dataGlobal->WeightPreviousHour * dl->TDDTransVisBeam(state.dataGlobal->PreviousHour, PipeNum);

        auto const &tddFluxIncCurr = dl->TDDFluxInc(state.dataGlobal->HourOfDay, PipeNum);
        auto const &tddFluxIncPrev = dl->TDDFluxInc(state.dataGlobal->PreviousHour, PipeNum);

        auto const &tddFluxTransCurr = dl->TDDFluxTrans(state.dataGlobal->HourOfDay, PipeNum);
        auto const &tddFluxTransPrev = dl->TDDFluxTrans(state.dataGlobal->PreviousHour, PipeNum);

        Illums TDDTransVisDiff;
        for (int iSky = iSky1; iSky <= iSky2; ++iSky) {
            Real64 tddTransVisDiffCurr = (tddFluxIncCurr.sky[iSky] > 0.0) ? (tddFluxTransCurr.sky[iSky] / tddFluxIncCurr.sky[iSky]) : 0.0;
            Real64 tddTransVisDiffPrev = (tddFluxIncPrev.sky[iSky] > 0.0) ? (tddFluxTransPrev.sky[iSky] / tddFluxIncPrev.sky[iSky]) : 0.0;

            TDDTransVisDiff.sky[iSky] =
                state.dataGlobal->WeightNow * tddTransVisDiffCurr + state.dataGlobal->WeightPreviousHour * tddTransVisDiffPrev;
        } // for (iSky)

        state.dataDaylightingDevicesData->TDDPipe(PipeNum).TransVisDiff =
            SkyWeight * TDDTransVisDiff.sky[iSky1] + (1.0 - SkyWeight) * TDDTransVisDiff.sky[iSky2];
    } // for (PipeNum)
} // DayltgInteriorTDDIllum()

void DayltgElecLightingControl(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   July 1997
    //       MODIFIED       Mar 2004, FCW: add inter-reflected illuminance from interior windows to DaylIllum
    //                      Apr 2004, FCW: move CALL ReportIllumMap from DayltgInteriorIllum2 (DayltgInteriorMapIllum)
    //                      Apr 2010, BG NREL: remove inter-reflected illuminance to stop double counting
    //                      Aug 2012, BG NREL: added availability schedule logic

    // PURPOSE OF THIS SUBROUTINE:
    // For a daylit space, determines lighting power reduction factor due to
    // daylighting for different lighting control systems.

    // Called by InitSurfaceHeatBalance.

    // REFERENCES:
    // Based on DOE-2.1E subroutine DLTSYS.
    auto &dl = state.dataDayltg;

    if (dl->daylightControl.empty()) {
        return;
    }
    // Reset space power reduction factors
    for (int spaceNum = 1; spaceNum <= state.dataGlobal->numSpaces; ++spaceNum) {
        dl->spacePowerReductionFactor(spaceNum) = 1.0;
    }

    for (auto &thisDayltgCtrl : dl->daylightControl) {

        if (thisDayltgCtrl.DaylightMethod != DaylightingMethod::SplitFlux) {
            // Set space power reduction factors
            if (thisDayltgCtrl.PowerReductionFactor < 1.0) {
                if (thisDayltgCtrl.spaceIndex > 0) {
                    // This is a space-level daylighting control
                    dl->spacePowerReductionFactor(thisDayltgCtrl.spaceIndex) = thisDayltgCtrl.PowerReductionFactor;
                } else {
                    // This is a zone-level daylighting control
                    for (int spaceNum : state.dataHeatBal->Zone(thisDayltgCtrl.zoneIndex).spaceIndexes) {
                        dl->spacePowerReductionFactor(spaceNum) = thisDayltgCtrl.PowerReductionFactor;
                    }
                }
            }
            continue;
        }

        // Electric lighting power reduction factor for a given daylighting control
        Real64 &TotReduction = thisDayltgCtrl.PowerReductionFactor;
        TotReduction = 0.0;
        Real64 ZFTOT = 0.0;

        // check if scheduled to be available
        if (thisDayltgCtrl.availSched->getCurrentVal() > 0.0) {

            // Loop over reference points
            for (int IL = 1; IL <= thisDayltgCtrl.TotalDaylRefPoints; ++IL) {
                auto &refPt = thisDayltgCtrl.refPts(IL);
                // Total fraction of zone that is daylit
                ZFTOT += refPt.fracZoneDaylit;

                dl->DaylIllum(IL) = refPt.lums[iLum_Illum];
                Real64 FL = 0.0;
                if (dl->DaylIllum(IL) < refPt.illumSetPoint) {
                    FL = (refPt.illumSetPoint - dl->DaylIllum(IL)) / refPt.illumSetPoint;
                }

                // BRANCH ON LIGHTING SYSTEM TYPE
                LtgCtrlType LSYSTP = thisDayltgCtrl.LightControlType;
                Real64 FP = 0.0;
                if (LSYSTP != LtgCtrlType::Stepped) {
                    // Continuously dimmable system with linear power curve
                    // Fractional output power required to meet setpoint
                    FP = 1.0;
                    // LIGHT-CTRL-TYPE = CONTINUOUS (LSYSTP = 1)
                    if (FL <= thisDayltgCtrl.MinLightFraction) {
                        FP = thisDayltgCtrl.MinPowerFraction;
                    }
                    // LIGHT-CTRL-TYPE = CONTINUOUS/OFF (LSYSTP = 3)
                    if (FL <= thisDayltgCtrl.MinLightFraction && LSYSTP == LtgCtrlType::ContinuousOff) {
                        FP = 0.0;
                    }
                    if (FL > thisDayltgCtrl.MinLightFraction && FL < 1.0) {
                        FP = (FL + (1.0 - FL) * thisDayltgCtrl.MinPowerFraction - thisDayltgCtrl.MinLightFraction) /
                             (1.0 - thisDayltgCtrl.MinLightFraction);
                    }

                } else { // LSYSTP = 2
                    // Stepped system
                    FP = 0.0;
                    // #9060: Use a tolerance, otherwise at very low (< 1e-12) daylighting conditions, you can get a multiplier > 1.0
                    if (dl->DaylIllum(IL) < 0.1) {
                        FP = 1.0;
                    } else if (dl->DaylIllum(IL) < refPt.illumSetPoint) {
                        FP = double(int(thisDayltgCtrl.LightControlSteps * FL) + 1) / double(thisDayltgCtrl.LightControlSteps);
                    }

                    if (thisDayltgCtrl.LightControlProbability < 1.0) {
                        // Manual operation.  Occupant sets lights one level too high a fraction of the time equal to
                        // 1. - ZoneDaylight(ZoneNum)%LightControlProbability.  RANDOM_NUMBER returns a random number
                        // between 0 and 1.
                        Real64 XRAN;
                        RANDOM_NUMBER(XRAN);
                        if (XRAN >= thisDayltgCtrl.LightControlProbability) {
                            // Set level one higher
                            if (FP < 1.0) {
                                FP += (1.0 / double(thisDayltgCtrl.LightControlSteps));
                            }
                        } // XRAN
                    } // Light Control Probability < 1
                } // Lighting System Type

                refPt.powerReductionFactor = FP;

                // Accumulate net ltg power reduction factor for entire zone
                TotReduction += refPt.powerReductionFactor * refPt.fracZoneDaylit;

            } // End of loop over reference points, IL

            // Correct for fraction of zone (1-ZFTOT) not controlled by
            // the reference points.  For this fraction (which is usually zero),
            // the electric lighting is unaffected and the power reduction
            // factor is therefore 1.0.
            TotReduction += (1.0 - ZFTOT);
        } else { // controls not currently available
            TotReduction = 1.0;
        }

        // Set space power reduction factors
        if (thisDayltgCtrl.spaceIndex > 0) {
            // This is a space-level daylighting control
            dl->spacePowerReductionFactor(thisDayltgCtrl.spaceIndex) = TotReduction;
        } else {
            // This is a zone-level daylighting control
            for (int spaceNum : state.dataHeatBal->Zone(thisDayltgCtrl.zoneIndex).spaceIndexes) {
                dl->spacePowerReductionFactor(spaceNum) = TotReduction;
            }
        }
    } // end daylighting control loop

    //  IF(TotIllumMaps > 0 .and. .not. DoingSizing .and. .not. WarmupFlag .and. .not. KickoffSimulation) THEN
    if ((int)dl->illumMaps.size() > 0 && !state.dataGlobal->DoingSizing && !state.dataGlobal->WarmupFlag) {
        for (int mapNum = 1; mapNum <= (int)dl->illumMaps.size(); ++mapNum) {
            auto &illumMap = dl->illumMaps(mapNum);
            if (state.dataGlobal->TimeStep == 1) {
                dl->mapResultsToReport = false;
            }
            for (auto &refPt : illumMap.refPts) {
                refPt.lumsHr[iLum_Illum] += refPt.lums[iLum_Illum] / double(state.dataGlobal->TimeStepsInHour);
                if (refPt.lumsHr[iLum_Illum] > 0.0) {
                    dl->mapResultsToReport = true;
                    dl->mapResultsReported = true;
                }
            }
            ReportIllumMap(state, mapNum);
            if (state.dataGlobal->TimeStep == state.dataGlobal->TimeStepsInHour) {
                for (auto &refPt : illumMap.refPts) {
                    refPt.lumsHr[iLum_Illum] = refPt.lums[iLum_Illum] = 0.0;
                }
            }
        } // for (mapNum)
    } // if (MapSize > 0)
} // DayltgElecLightingControl()

Real64 DayltgGlarePositionFactor(Real64 X, // Lateral and vertical distance of luminous window element from
                                 Real64 Y)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   July 1997

    // PURPOSE OF THIS SUBROUTINE:
    // by table interpolation, evaluates the
    // Hopkinson position factor used in glare calculation
    // (Hopkinson, Petherbridge, AND Longmore -- Daylighting,
    // London, 1966, PP 307, 323).  X (Y) is the lateral
    // (vertical) distance of luminous window element from
    // horizontal line of vision, divided by horizontal distance
    // from eye of observer. The array PF contains values of
    // the position factor for X = 0, 0.5, 1.0, 1.5, 2.0, 2.5,
    // and 3.0 and Y = 0, 0.5, 1.0, 1.5, 2.0. Called by CalcDayltgCoefficients.

    // REFERENCES:
    // Based on DOE-2.1E subroutine DPFAC.

    // Position factor array
    static constexpr std::array<std::array<Real64, 7>, 5> PF = {{
        {1.00, 0.492, 0.226, 0.128, 0.081, 0.061, 0.057},
        {0.123, 0.119, 0.065, 0.043, 0.029, 0.026, 0.023},
        {0.019, 0.026, 0.019, 0.016, 0.014, 0.011, 0.011},
        {0.008, 0.008, 0.008, 0.008, 0.008, 0.006, 0.006},
        {0.0, 0.0, 0.003, 0.003, 0.003, 0.003, 0.003},
    }};

    if (X < 0.0 || X >= 3.0) {
        return 0.0;
    }
    if (Y < 0.0 || Y >= 2.0) {
        return 0.0;
    }

    int IX = 1 + int(2.0 * X);
    int IY = 1 + int(2.0 * Y);
    Real64 X1 = 0.5 * double(IX - 1);
    Real64 Y1 = 0.5 * double(IY - 1);
    Real64 FA = PF[IY - 1][IX - 1] + 2.0 * (X - X1) * (PF[IY - 1][IX] - PF[IY - 1][IX - 1]);
    Real64 FB = PF[IY][IX - 1] + 2.0 * (X - X1) * (PF[IY][IX] - PF[IY][IX - 1]);
    return FA + 2.0 * (Y - Y1) * (FB - FA);
} // DayltgGlarePositionFactor()

void DayltgInterReflectedIllum(EnergyPlusData &state,
                               int const ISunPos, // Sun position counter; used to avoid calculating various
                               int const IHR,     // Hour of day
                               int const enclNum, // Daylighting enclosure index
                               int const IWin     // Window index
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   July 1997
    //       MODIFIED       FCW December 1998
    //                      FCW June 2001: Add blind calculations
    //                      FCW Jan 2001: Add blinds with movable slats
    //                      FCW Jan 2003: Add between-glass blinds
    //                      FCW Jul 2003: account for transmittance of shading surfaces
    //                       (previously these were assumed opaque even if transmittance schedule
    //                        value was non-zero)
    //                      FCW Aug 2003: modify initialization of WinLum from WinLum = 0. TO
    //                        WinLum(:,:,IHR) = 0. Otherwise values calculated in previous
    //                        call are incorrectly zeroed. Result was that window luminance with
    //                        shade or blind included only contribution from first window element
    //                        in window element loop in CalcDayltgCoefficients, thus seriously
    //                        undercalculating window luminance for windows with more than one
    //                        window element. Similarly, modified initialization of WLUMSU from
    //                        WLUMSU = 0. to WLUMSU(:,IHR) = 0., and of WLUMSUdisk from
    //                        WLUMSUdisk = 0. to WLUMSUdisk(:,IHR) = 0.
    //                      PGE Aug 2003: Add daylighting shelves.
    //                      FCW Nov 2003: Add beam solar and sky solar reflected from obstructions;
    //                                    add beam solar reflected from ground accounting for obstructions.
    //                      FCW Nov 2003: increase NPHMAX from 9 to 10 to avoid rays with altitude angle = 0
    //                                    for vertical surfaces.
    //                      FCW Nov 2003: fix the expression for min and max limits of azimuth; old expression
    //                                    broke down for window normals with negative altitude angle
    //                      FCW Nov 2003: add specular reflection from exterior obstructions
    //                      FCW Apr 2004: add light well efficiency multiplying window transmittance
    //                      FCW Apr 2004: add diffusing glazing
    //                      RAR (FSEC)  May 2006: add exterior window screen
    //                      B. Griffith NREL April 2010: CR7869 add adjacent zone area if window is not on this zone
    //                                    apply interior window transmission and blocking to beam transmission from ext win

    // PURPOSE OF THIS SUBROUTINE:
    // Called from CalcDayltgCoefficients for each window and reference point in a daylit
    // space, for each sun position. Calculates illuminance (EINTSK and EINTSU) at reference point due
    // to internally reflected light by integrating to determine the amount of flux from
    // sky and ground (and beam reflected from obstructions) transmitted through
    // the center of the window and then reflecting this
    // light from the inside surfaces of the space.  The "split-flux" method is used
    // (Lynes, Principles of Natural Lighting, 1968).  EINT is determined for
    // different sky types and for window with and without shades, screens or blinds.
    // Also finds luminance (WinLum and WLUMSU) of window with shade or blind, &
    // or with diffusing glass, for different sky types.

    // REFERENCES:
    // Based on DOE-2.1E subroutine DREFLT.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    // In the following I,J arrays:
    // I = sky type;
    // J = 1 for bare window, 2 and above for window with shade or blind.
    Illums ZSK;                   // Sky-related and sun-related illuminance on window from sky/ground
    Vector3<Real64> U;            // Unit vector in (PH,TH) direction
    Vector3<Real64> nearestHitPt; // Hit point of ray on nearest obstruction (m)
    Vector3<Real64> obsHitPt;     // Coordinates of hit point on an obstruction (m)
    Vector3<Real64> groundHitPt;  // Coordinates of point that ray from window center hits the ground (m)
    std::array<Dayltg::Illums, (int)DataSurfaces::WinCover::Num> FLCW = {Illums()}; // Sky-related upgoing luminous flux
    std::array<Dayltg::Illums, (int)DataSurfaces::WinCover::Num> FLFW = {Illums()}; // Sky-related downgoing luminous flux

    //  3=intermediate, 4=overcast
    Real64 DPH; // Sky/ground element altitude and azimuth increments (radians)
    Real64 DTH;
    Real64 PH; // Sky/ground element altitude and azimuth (radians)
    Real64 TH;
    Real64 SPH; // Sine and cosine of PH
    Real64 CPH;
    Real64 PHMIN; // Limits of altitude integration (radians)
    Real64 PHMAX;
    Real64 ThMin; // Limits of azimuth integration (radians)
    Real64 ThMax;
    Real64 PhWin; // Altitude, azimuth angle of window normal (radians)
    Real64 ThWin;
    Real64 ACosTanTan; // ACOS(-TAN(Ph)*TAN(PhWin))
    Real64 DA;         // CPH*DTH*DPH
    Real64 COSB;       // Cosine of angle of incidence of light from sky or ground
    Real64 TVISBR;     // Transmittance of window without shading at COSB
    //  (times light well efficiency, if appropriate)
    Real64 ZSU;
    //  element for clear and overcast sky
    Real64 ObTrans; // Product of solar transmittances of obstructions seen by a light ray

    // unused  REAL(r64)         :: HitPointLumFrClearSky     ! Luminance of obstruction from clear sky (cd/m2)
    // unused  REAL(r64)         :: HitPointLumFrOvercSky     ! Luminance of obstruction from overcast sky (cd/m2)
    // unused  REAL(r64)         :: HitPointLumFrSun          ! Luminance of obstruction from sun (cd/m2)
    int ICtrl;       // Window control pointer
    Real64 COSBSun;  // Cosine of angle of incidence of direct sun on window
    Real64 TVISBSun; // Window's visible transmittance at COSBSun
    //  (times light well efficiency, if appropriate)
    Real64 ZSU1; // Transmitted direct normal illuminance (lux)
    //  CHARACTER(len=32) :: ShType                    ! Window shading device type
    bool ShadeOn;                // True if exterior or interior window shade present
    bool BlindOn;                // True if exterior or interior window blind present
    bool ScreenOn;               // True if exterior window screen present
                                 //        int ScNum; // Screen number //Unused Set but never used
    int PipeNum;                 // TDD pipe object number
    int ShelfNum;                // Daylighting shelf object number
    int InShelfSurf;             // Inside daylighting shelf surface number
    int OutShelfSurf;            // Outside daylighting shelf surface number
    Real64 TransBlBmDiffFront;   // Isolated blind vis beam-diffuse front transmittance
    Real64 TransScBmDiffFront;   // Isolated screen vis beam-diffuse front transmittance
    Real64 ReflGlDiffDiffBack;   // Bare glazing system vis diffuse back reflectance
    Real64 ReflGlDiffDiffFront;  // Bare glazing system vis diffuse front reflectance
    Real64 ReflBlBmDiffFront;    // Isolated blind vis beam-diffuse front reflectance
    Real64 TransBlDiffDiffFront; // Isolated blind vis diffuse-diffuse front transmittance
    Real64 ReflBlDiffDiffFront;  // Isolated blind vis diffuse-diffuse front reflectance
    Real64 ReflBlDiffDiffBack;   // Isolated blind vis diffuse-diffuse back reflectance
    Real64 ReflScDiffDiffBack;   // Isolated screen vis diffuse-diffuse back reflectance

    Real64 td2; // Diffuse-diffuse vis trans of bare glass layers 2 and 3
    Real64 td3;
    Real64 rbd1; // Beam-diffuse back vis reflectance of bare glass layers 1 and 2
    Real64 rbd2;
    Real64 rfd2; // Beam-diffuse front vis reflectance of bare glass layers 2 and 3
    Real64 rfd3;
    Real64 tfshd;      // Diffuse-diffuse front vis trans of bare blind
    Real64 rbshd;      // Diffuse-diffuse back vis reflectance of bare blind
    Real64 ZSUObsRefl; // Illuminance on window from beam solar reflected by an
    //  obstruction (for unit beam normal illuminance)
    int NearestHitSurfNum;  // Surface number of nearest obstruction
    int NearestHitSurfNumX; // Surface number to use when obstruction is a shadowing surface
    Real64 LumAtHitPtFrSun; // Luminance at hit point on obstruction from solar reflection
    //  for unit beam normal illuminance (cd/m2)
    Real64 SunObstructionMult; // = 1 if sun hits a ground point; otherwise = 0
    bool hitObs;               // True iff obstruction is hit
    Real64 ObsVisRefl;         // Visible reflectance of obstruction
    Real64 SkyReflVisLum;      // Reflected sky luminance at hit point divided by unobstructed sky
    //  diffuse horizontal illuminance [(cd/m2)/lux]
    Real64 dReflObsSky; // Contribution to sky-related illuminance on window due to sky diffuse
    //  reflection from an obstruction
    Real64 TVisSunRefl; // Diffuse vis trans of bare window for beam reflection calc
    //  (times light well efficiency, if appropriate)
    Real64 ZSU1refl; // Beam normal illuminance times ZSU1refl = illuminance on window
    //  due to specular reflection from exterior surfaces

    ExtWinType extWinType;      // Exterior window type (InZoneExtWin, AdjZoneExtWin, NotInOrAdjZoneExtWin)
    Real64 EnclInsideSurfArea;  // temporary for calculations, total surface area of enclosure surfaces m2
    int IntWinAdjZoneExtWinNum; // the index of the exterior window in IntWinAdjZoneExtWin nested struct
    int IntWinNum;              // window index for interior windows associated with exterior windows
    Real64 COSBintWin;

    WinShadingType ShType;

    auto &s_mat = state.dataMaterial;
    auto &dl = state.dataDayltg;
    auto &s_surf = state.dataSurface;

    auto &thisEnclDaylight = dl->enclDaylight(enclNum);
    auto const &surf = s_surf->Surface(IWin);
    auto const &surfWin = s_surf->SurfaceWindow(IWin);
    int const enclNumThisWin = s_surf->Surface(surf.BaseSurf).SolarEnclIndex;
    // The inside surface area, ZoneDaylight(ZoneNum)%totInsSurfArea was calculated in subr DayltgAveInteriorReflectance

    if (enclNumThisWin == enclNum) {
        extWinType = ExtWinType::InZone;
        EnclInsideSurfArea = dl->enclDaylight(enclNumThisWin).totInsSurfArea;
        IntWinAdjZoneExtWinNum = 0;
    } else {
        extWinType = ExtWinType::AdjZone;
        // If window is exterior window in adjacent zone, then use areas of both enclosures
        EnclInsideSurfArea = dl->enclDaylight(enclNum).totInsSurfArea + dl->enclDaylight(enclNumThisWin).totInsSurfArea;
        // find index in IntWinAdjZoneExtWin
        for (int AdjExtWinLoop = 1; AdjExtWinLoop <= thisEnclDaylight.NumOfIntWinAdjEnclExtWins; ++AdjExtWinLoop) {
            if (IWin == thisEnclDaylight.IntWinAdjEnclExtWin(AdjExtWinLoop).SurfNum) { // found it
                IntWinAdjZoneExtWinNum = AdjExtWinLoop;
                break; // added TH 4/13/2010
            }
        }
    }

    // Initialize window luminance and fluxes for split-flux calculation
    dl->winLum(IHR)[(int)iWinCover_Bare] = dl->winLum(IHR)[(int)iWinCover_Shaded] = Illums();
    // dl->WLUMSU(IHR, _) = 0.0;
    // dl->WLUMSUdisk(IHR, _) = 0.0;

    int const IConst = s_surf->SurfActiveConstruction(IWin);
    auto const &construct = state.dataConstruction->Construct(IConst);

    BlindOn = false;
    ShadeOn = false;
    ScreenOn = false;

    if (s_surf->Surface(IWin).OriginalClass == SurfaceClass::TDD_Dome) {
        PipeNum = s_surf->SurfWinTDDPipeNum(IWin);
    }

    ShelfNum = s_surf->SurfDaylightingShelfInd(IWin);
    if (ShelfNum > 0) {
        InShelfSurf = state.dataDaylightingDevicesData->Shelf(ShelfNum).InSurf;   // Inside daylighting shelf present if > 0
        OutShelfSurf = state.dataDaylightingDevicesData->Shelf(ShelfNum).OutSurf; // Outside daylighting shelf present if > 0
    } else {
        InShelfSurf = 0;
        OutShelfSurf = 0;
    }

    // Divide sky and ground into elements of altitude PH and
    // azimuth TH, and add the contribution of light coming from each
    // element to the transmitted flux at the center of the window
    // Azimuth ranges over a maximum of 2 Pi radians.
    // Altitude ranges over a maximum of Pi/2 radians between -Pi/2 < PH < +Pi/2, so that elements are not counted twice
    // PH = 0 at the horizon; PH = Pi/2 at the zenith
    PHMIN = max(-Constant::PiOvr2, surfWin.phi - Constant::PiOvr2);
    PHMAX = min(Constant::PiOvr2, surfWin.phi + Constant::PiOvr2);
    DPH = (PHMAX - PHMIN) / double(NPHMAX);

    // Sky/ground element altitude integration
    Vector3<Real64> const SUNCOS_IHR(s_surf->SurfSunCosHourly(IHR));
    for (int IPH = 1; IPH <= NPHMAX; ++IPH) {
        PH = PHMIN + (double(IPH) - 0.5) * DPH;

        SPH = std::sin(PH);
        CPH = std::cos(PH);
        // Third component of unit vector in (TH,PH) direction
        U.z = SPH;

        // Limits of azimuth integration
        PhWin = surfWin.phi;
        ThWin = surfWin.theta;
        if (PhWin >= 0.0) {
            if (PH >= Constant::PiOvr2 - PhWin) {
                ThMin = -Constant::Pi;
                ThMax = Constant::Pi;
            } else {
                ACosTanTan = std::acos(-std::tan(PH) * std::tan(PhWin));
                ThMin = ThWin - std::abs(ACosTanTan);
                ThMax = ThWin + std::abs(ACosTanTan);
            }

        } else { // PhiSurf < 0.0
            if (PH <= -PhWin - Constant::PiOvr2) {
                ThMin = -Constant::Pi;
                ThMax = Constant::Pi;
            } else {
                ACosTanTan = std::acos(-std::tan(PH) * std::tan(PhWin));
                ThMin = ThWin - std::abs(ACosTanTan);
                ThMax = ThWin + std::abs(ACosTanTan);
            }
        }

        DTH = (ThMax - ThMin) / double(NTHMAX);
        DA = CPH * DTH * DPH;

        // Sky/ground element azimuth integration
        Real64 const sin_window_phi(std::sin(surfWin.phi));
        Real64 const cos_window_phi(std::cos(surfWin.phi));
        for (int ITH = 1; ITH <= NTHMAX; ++ITH) {
            TH = ThMin + (double(ITH) - 0.5) * DTH;
            U.x = CPH * std::cos(TH);
            U.y = CPH * std::sin(TH);
            // Cosine of angle of incidence of light from sky or ground element
            COSB = SPH * sin_window_phi + CPH * cos_window_phi * std::cos(TH - surfWin.theta);
            if (COSB < 0.0) {
                continue; // Sky/ground elements behind window (although there shouldn't be any)
            }

            // Initialize illuminance on window for this sky/ground element
            ZSK = Illums();
            ZSU = 0.0;
            // Initialize illuminance on window from beam solar reflection if ray hits an obstruction
            ZSUObsRefl = 0.0;

            if (ISunPos == 1) { // Intersection calculation has to be done only for first sun position
                // Determine net transmittance of obstructions that the ray hits. ObTrans will be 1.0
                // if no obstructions are hit.
                ObTrans = DayltgHitObstruction(state, IHR, IWin, s_surf->SurfaceWindow(IWin).WinCenter, U);
                dl->ObTransM[IPH][ITH] = ObTrans;
                dl->SkyObstructionMult[IPH][ITH] = 1.0;
            }

            // SKY AND GROUND RADIATION ON WINDOW

            // Contribution is from sky if PH > 0 (ray goes upward), and from ground if PH < 0 (ray goes downward)
            // (There may also be contributions from reflection from obstructions; see 'BEAM SOLAR AND SKY SOLAR
            // REFLECTED FROM NEAREST OBSTRUCTION,' below.)

            if (PH > 0.0) { // Contribution is from sky
                for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
                    ZSK.sky[iSky] = DayltgSkyLuminance(state, static_cast<SkyType>(iSky), TH, PH) * COSB * DA * dl->ObTransM[IPH][ITH];
                }
            } else { // PH <= 0.0; contribution is from ground
                if (s_surf->CalcSolRefl && dl->ObTransM[IPH][ITH] > 1.e-6 && ISunPos == 1) {
                    // Calculate effect of obstructions on shading of sky diffuse reaching the ground point hit
                    // by the ray. This effect is given by the ratio SkyObstructionMult =
                    // (obstructed sky diffuse at ground point)/(unobstructed sky diffuse at ground point).
                    // This ratio is calculated for an isotropic sky.
                    // Ground point hit by the ray:
                    Real64 Alfa = std::acos(-U.z);
                    Real64 Beta = std::atan2(U.y, U.x);
                    Real64 HorDis = (s_surf->SurfaceWindow(IWin).WinCenter.z - s_surf->GroundLevelZ) * std::tan(Alfa);
                    groundHitPt.z = s_surf->GroundLevelZ;
                    groundHitPt.x = s_surf->SurfaceWindow(IWin).WinCenter.x + HorDis * std::cos(Beta);
                    groundHitPt.y = s_surf->SurfaceWindow(IWin).WinCenter.y + HorDis * std::sin(Beta);

                    dl->SkyObstructionMult[IPH][ITH] =
                        CalcObstrMultiplier(state, groundHitPt, AltAngStepsForSolReflCalc, DataSurfaces::AzimAngStepsForSolReflCalc);
                } // End of check if solar reflection calc is in effect

                auto const &gilsk = dl->horIllum[IHR];
                for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
                    // Below, luminance of ground in cd/m2 is illuminance on ground in lumens/m2
                    // times ground reflectance, divided by pi, times obstruction multiplier.
                    ZSK.sky[iSky] = (gilsk.sky[iSky] * state.dataEnvrn->GndReflectanceForDayltg / Constant::Pi) * COSB * DA * dl->ObTransM[IPH][ITH] *
                                    dl->SkyObstructionMult[IPH][ITH];
                }
                // Determine if sun illuminates the point that ray hits the ground. If the solar reflection
                // calculation has been requested (CalcSolRefl = .TRUE.) shading by obstructions, including
                // the building itself, is considered in determining whether sun hits the ground point.
                // Otherwise this shading is ignored and the sun always hits the ground point.
                SunObstructionMult = 1.0;
                if (s_surf->CalcSolRefl && dl->ObTransM[IPH][ITH] > 1.e-6 && ISunPos == 1) {
                    // Sun reaches ground point if vector from this point to the sun is unobstructed
                    hitObs = false;
                    for (int ObsSurfNum : s_surf->AllShadowPossObstrSurfaceList) {
                        hitObs = PierceSurface(state, ObsSurfNum, groundHitPt, SUNCOS_IHR, obsHitPt);
                        if (hitObs) {
                            break;
                        }
                    }
                    if (hitObs) {
                        SunObstructionMult = 0.0;
                    }
                }
                ZSU = (dl->horIllum[IHR].sun * state.dataEnvrn->GndReflectanceForDayltg / Constant::Pi) * COSB * DA * dl->ObTransM[IPH][ITH] *
                      SunObstructionMult;
            }
            // BEAM SOLAR AND SKY SOLAR REFLECTED FROM NEAREST OBSTRUCTION

            if (s_surf->CalcSolRefl && dl->ObTransM[IPH][ITH] < 1.0) {
                // Find obstruction whose hit point is closest to the center of the window
                DayltgClosestObstruction(state, s_surf->SurfaceWindow(IWin).WinCenter, U, NearestHitSurfNum, nearestHitPt);
                if (NearestHitSurfNum > 0) {

                    // Beam solar reflected from nearest obstruction.
                    LumAtHitPtFrSun = DayltgSurfaceLumFromSun(state, IHR, U, NearestHitSurfNum, nearestHitPt);
                    ZSUObsRefl = LumAtHitPtFrSun * COSB * DA;
                    ZSU += ZSUObsRefl;

                    // Sky solar reflected from nearest obstruction.
                    int const ObsConstrNum = s_surf->Surface(NearestHitSurfNum).Construction;
                    if (ObsConstrNum > 0) {
                        // Exterior building surface is nearest hit
                        if (!state.dataConstruction->Construct(ObsConstrNum).TypeIsWindow) {
                            // Obstruction is not a window, i.e., is an opaque surface
                            ObsVisRefl = 1.0 - s_mat->materials(state.dataConstruction->Construct(ObsConstrNum).LayerPoint(1))->AbsorpVisible;
                        } else {
                            // Obstruction is a window; assume it is bare
                            ObsVisRefl = state.dataConstruction->Construct(ObsConstrNum).ReflectVisDiffFront;
                        }
                    } else {
                        // Shadowing surface is nearest hit
                        if (s_surf->SurfDaylightingShelfInd(NearestHitSurfNum) > 0) {
                            // Skip daylighting shelves, whose reflection is separately calculated
                            ObsVisRefl = 0.0;
                        } else {
                            ObsVisRefl = s_surf->SurfShadowDiffuseVisRefl(NearestHitSurfNum);
                            if (s_surf->SurfShadowGlazingConstruct(NearestHitSurfNum) > 0) {
                                ObsVisRefl +=
                                    s_surf->SurfShadowGlazingFrac(NearestHitSurfNum) *
                                    state.dataConstruction->Construct(s_surf->SurfShadowGlazingConstruct(NearestHitSurfNum)).ReflectVisDiffFront;
                            }
                            // Note in the above that ShadowSurfDiffuseVisRefl is the reflectance of opaque part of
                            // shadowing surface times (1 - ShadowSurfGlazingFrac)
                        }
                    }
                    NearestHitSurfNumX = NearestHitSurfNum;
                    // Each shadowing surface has a "mirror" duplicate surface facing in the opposite direction.
                    // The following gets the correct side of a shadowing surface for reflection.
                    if (s_surf->Surface(NearestHitSurfNum).IsShadowing) {
                        if (dot(U, s_surf->Surface(NearestHitSurfNum).OutNormVec) > 0.0) {
                            NearestHitSurfNumX = NearestHitSurfNum + 1;
                        }
                    }
                    if (!state.dataSysVars->DetailedSkyDiffuseAlgorithm || !s_surf->ShadingTransmittanceVaries ||
                        state.dataHeatBal->SolarDistribution == DataHeatBalance::Shadowing::Minimal) {
                        SkyReflVisLum = ObsVisRefl * s_surf->Surface(NearestHitSurfNumX).ViewFactorSky *
                                        state.dataSolarShading->SurfDifShdgRatioIsoSky(NearestHitSurfNumX) / Constant::Pi;
                    } else {
                        SkyReflVisLum = ObsVisRefl * s_surf->Surface(NearestHitSurfNumX).ViewFactorSky *
                                        state.dataSolarShading->SurfDifShdgRatioIsoSkyHRTS(1, IHR, NearestHitSurfNumX) / Constant::Pi;
                    }
                    dReflObsSky = SkyReflVisLum * COSB * DA;

                    auto const &gilsk = dl->horIllum[IHR];
                    for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
                        ZSK.sky[iSky] += gilsk.sky[iSky] * dReflObsSky;
                    }
                }
            } // End of check if exterior solar reflection calculation is active

            //  ===Bare window (no shade or blind; non-diffusing glass)===

            // Increment flux entering space and window luminance (cd/m2).
            // FLCW--(I,J) = part of incoming flux (in lumens) that goes up to ceiling and upper part of walls.
            // FLFW--(I,J) = part that goes down to floor and lower part of walls

            if (s_surf->Surface(IWin).OriginalClass == SurfaceClass::TDD_Dome) {
                // Unshaded visible transmittance of TDD for a single ray from sky/ground element
                TVISBR = TransTDD(state, PipeNum, COSB, RadType::VisibleBeam) * surfWin.glazedFrac;

                // Make all transmitted light diffuse for a TDD with a bare diffuser
                auto &wlumsk = dl->winLum(IHR)[iWinCover_Bare];
                auto &flfwsk = FLFW[iWinCover_Bare];
                auto &flcwsk = FLCW[iWinCover_Bare];

                auto &tddFluxInc = dl->TDDFluxInc(IHR, PipeNum);
                auto &tddFluxTrans = dl->TDDFluxTrans(IHR, PipeNum);
                for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
                    wlumsk.sky[iSky] += ZSK.sky[iSky] * TVISBR / Constant::Pi;
                    flfwsk.sky[iSky] += ZSK.sky[iSky] * TVISBR * (1.0 - surfWin.fractionUpgoing);
                    flcwsk.sky[iSky] += ZSK.sky[iSky] * TVISBR * surfWin.fractionUpgoing;

                    // For later calculation of diffuse visible transmittance
                    tddFluxInc.sky[iSky] += ZSK.sky[iSky];
                    tddFluxTrans.sky[iSky] += ZSK.sky[iSky] * TVISBR;

                } // for (iSky)

                tddFluxInc.sky[(int)SkyType::Clear] += ZSU;
                tddFluxTrans.sky[(int)SkyType::Clear] += ZSU * TVISBR;

                dl->winLum(IHR)[iWinCover_Bare].sun += ZSU * TVISBR / Constant::Pi;
                flfwsk.sun += ZSU * TVISBR * (1.0 - surfWin.fractionUpgoing);
                flcwsk.sun += ZSU * TVISBR * surfWin.fractionUpgoing;

            } else { // Bare window
                // Transmittance of bare window for this sky/ground element
                TVISBR = Window::POLYF(COSB, construct.TransVisBeamCoef) * surfWin.glazedFrac * surfWin.lightWellEff;

                if (InShelfSurf > 0) { // Inside daylighting shelf
                    // Daylighting shelf simplification:  All light is diffuse
                    // SurfaceWindow(IWin)%FractionUpgoing is already set to 1.0 earlier
                    auto &flcwsk = FLCW[iWinCover_Bare];
                    for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
                        flcwsk.sky[iSky] += ZSK.sky[iSky] * TVISBR * surfWin.fractionUpgoing;
                    }
                    flcwsk.sun += ZSU * TVISBR * surfWin.fractionUpgoing;

                } else { // Normal window

                    // CR 7869  correct TVISBR if disk beam passes thru interior window
                    if (extWinType == ExtWinType::AdjZone) {
                        // modify TVISBR by second window transmission
                        // first determine if ray from point passes thru any interior window
                        hitObs = false;
                        for (int IntWinLoop = 1; IntWinLoop <= thisEnclDaylight.IntWinAdjEnclExtWin(IntWinAdjZoneExtWinNum).NumOfIntWindows;
                             ++IntWinLoop) {
                            IntWinNum = thisEnclDaylight.IntWinAdjEnclExtWin(IntWinAdjZoneExtWinNum).IntWinNum(IntWinLoop);
                            auto const &surfIntWin = s_surf->SurfaceWindow(IntWinNum);
                            hitObs = PierceSurface(state, IntWinNum, surfIntWin.WinCenter, SUNCOS_IHR, obsHitPt);
                            if (hitObs) { // disk passes thru
                                // cosine of incidence angle of light from sky or ground element for
                                COSBintWin = SPH * std::sin(surfIntWin.phi) + CPH * std::cos(surfIntWin.phi) * std::cos(TH - surfIntWin.theta);
                                TVISBR *= Window::POLYF(COSBintWin,
                                                        state.dataConstruction->Construct(s_surf->Surface(IntWinNum).Construction).TransVisBeamCoef);
                                break;
                            }
                        }
                        if (!hitObs) { // blocked by opaque parts, beam does not actually pass thru interior window to reach zone
                            TVISBR = 0.0;
                        }
                    }

                    auto &flfwsk = FLFW[iWinCover_Bare];
                    auto &flcwsk = FLCW[iWinCover_Bare];
                    for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
                        // IF (PH < 0.0d0) THEN
                        // Fixed by FCW, Nov. 2003:
                        if (PH > 0.0) {
                            flfwsk.sky[iSky] += ZSK.sky[iSky] * TVISBR;
                        } else {
                            flcwsk.sky[iSky] += ZSK.sky[iSky] * TVISBR;
                        }
                    } // for (iSky)

                    if (PH > 0.0) {
                        flfwsk.sun += ZSU * TVISBR;
                    } else {
                        flcwsk.sun += ZSU * TVISBR;
                    }

                } // End of check if window with daylighting shelf or normal window
            } // End of check if TDD:DOME or bare window

            // Check if window has shade or blind
            ICtrl = s_surf->Surface(IWin).activeWindowShadingControl;
            if (s_surf->Surface(IWin).HasShadeControl) {
                ShType = s_surf->WindowShadingControl(ICtrl).ShadingType;
                ShadeOn = ANY_SHADE(ShType);
                BlindOn = ANY_BLIND(ShType);
                ScreenOn = (ShType == WinShadingType::ExtScreen);
            }

            if (ShadeOn || BlindOn || ScreenOn || s_surf->SurfWinSolarDiffusing(IWin)) {

                // ===Window with interior or exterior shade or blind, exterior screen, or with diffusing glass===

                // Increment flux entering space and window luminance. Shades and diffusing glass are
                // assumed to be perfect diffusers, i.e., the transmittance is independent of angle of
                // incidence and the transmitted light is isotropic. The transmittance of a blind is
                // assumed to depend on profile angle and slat angle; the diffuse light entering the room from
                // the slats of the blind is assumed to be isotropic. With blinds, light can also enter
                // the room by passing between the slats without reflection. The beam transmittance of a screen
                // is assumed to depend on sun azimuth and azimuth angle.

                // For light from a shade, or from diffusing glass, or from the slats of a blind, a flux fraction,
                // SurfaceWindow(IWin)%FractionUpgoing (determined by window tilt), goes up toward
                // ceiling and upper part of walls, and 1-Surfacewindow(iwin)%FractionUpgoing
                // goes down toward floor and lower part of walls. For a blind, the light passing
                // between the slats goes either up or down depending on the altitude angle of the
                // element from which the light came. For a screen, the light passing
                // between the screen's cylinders goes either up or down depending on the altitude angle of the
                // element from which the light came.

                int IConstShaded = s_surf->SurfWinActiveShadedConstruction(IWin);
                if (s_surf->SurfWinSolarDiffusing(IWin)) {
                    IConstShaded = s_surf->Surface(IWin).Construction;
                }

                // Transmittance of window including shade, screen or blind
                Real64 transBmBmMult = 0.0;
                Real64 transMult = 0.0;

                if (ShadeOn) { // Shade
                    if (s_surf->Surface(IWin).OriginalClass == SurfaceClass::TDD_Dome) {
                        // Shaded visible transmittance of TDD for a single ray from sky/ground element
                        transMult = TransTDD(state, PipeNum, COSB, RadType::VisibleBeam) * surfWin.glazedFrac;
                    } else { // Shade only, no TDD
                        // Calculate transmittance of the combined window and shading device for this sky/ground element
                        transMult = Window::POLYF(COSB, state.dataConstruction->Construct(IConstShaded).TransVisBeamCoef) * surfWin.glazedFrac *
                                    surfWin.lightWellEff;
                    }

                } else if (ScreenOn) { // Screen: get beam-beam, beam-diffuse and diffuse-diffuse vis trans/ref of screen and glazing system
                    auto const *screen = dynamic_cast<Material::MaterialScreen *>(s_mat->materials(surfWin.screenNum));
                    assert(screen != nullptr);

                    Real64 phi = std::abs(PH - surfWin.phi);
                    Real64 theta = std::abs(TH - surfWin.theta);
                    int ip1, ip2, it1, it2; // lo/hi phi/theta interpolation map indices
                    BilinearInterpCoeffs coeffs;

                    Material::NormalizePhiTheta(phi, theta);
                    Material::GetPhiThetaIndices(phi, theta, screen->dPhi, screen->dTheta, ip1, ip2, it1, it2);
                    GetBilinearInterpCoeffs(phi, theta, ip1 * screen->dPhi, ip2 * screen->dPhi, it1 * screen->dTheta, it2 * screen->dTheta, coeffs);

                    ReflGlDiffDiffFront = state.dataConstruction->Construct(IConst).ReflectVisDiffFront;
                    ReflScDiffDiffBack = screen->DfRefVis;

                    auto const &b11 = screen->btars[ip1][it1];
                    auto const &b12 = screen->btars[ip1][it2];
                    auto const &b21 = screen->btars[ip2][it1];
                    auto const &b22 = screen->btars[ip2][it2];

                    TransScBmDiffFront = BilinearInterp(b11.DfTransVis, b12.DfTransVis, b21.DfTransVis, b22.DfTransVis, coeffs);

                    transMult = TransScBmDiffFront * surfWin.glazedFrac * state.dataConstruction->Construct(IConst).TransDiffVis /
                                (1 - ReflGlDiffDiffFront * ReflScDiffDiffBack) * surfWin.lightWellEff;

                    transBmBmMult = BilinearInterp(b11.BmTransVis, b12.BmTransVis, b21.BmTransVis, b22.BmTransVis, coeffs);

                } else if (BlindOn) { // Blind: get beam-diffuse and beam-beam vis trans of blind+glazing system
                    // PETER:  As long as only interior blinds are allowed for TDDs, no need to change TransMult calculation
                    //         for TDDs because it is based on TVISBR which is correctly calculated for TDDs above.
                    auto const &surfShade = s_surf->surfShades(IWin);
                    auto const *matBlind = dynamic_cast<Material::MaterialBlind const *>(s_mat->materials(surfShade.blind.matNum));
                    assert(matBlind != nullptr);
                    Real64 ProfAng = ProfileAngle(state, IWin, U, matBlind->SlatOrientation);

                    auto &btar = surfShade.blind.TAR;
                    int idxLo = surfShade.blind.profAngIdxLo;
                    int idxHi = std::min(Material::MaxProfAngs, idxLo + 1);
                    Real64 interpFac = surfShade.blind.profAngInterpFac;
                    TransBlBmDiffFront = Interp(btar.Vis.Ft.Bm[idxLo].DfTra, btar.Vis.Ft.Bm[idxHi].DfTra, interpFac);

                    if (ShType == WinShadingType::IntBlind) { // Interior blind
                        ReflGlDiffDiffBack = construct.ReflectVisDiffBack;
                        ReflBlBmDiffFront = Interp(btar.Vis.Ft.Bm[idxLo].DfRef, btar.Vis.Ft.Bm[idxHi].DfRef, interpFac);
                        ReflBlDiffDiffFront = btar.Vis.Ft.Df.Ref;
                        TransBlDiffDiffFront = btar.Vis.Ft.Df.Tra;
                        transMult = TVISBR * (TransBlBmDiffFront + ReflBlBmDiffFront * ReflGlDiffDiffBack * TransBlDiffDiffFront /
                                                                       (1.0 - ReflBlDiffDiffFront * ReflGlDiffDiffBack));

                    } else if (ShType == WinShadingType::ExtBlind) { // Exterior blind
                        ReflGlDiffDiffFront = construct.ReflectVisDiffFront;
                        ReflBlDiffDiffBack = btar.Vis.Bk.Df.Ref;
                        transMult = TransBlBmDiffFront * surfWin.glazedFrac * construct.TransDiffVis /
                                    (1.0 - ReflGlDiffDiffFront * ReflBlDiffDiffBack) * surfWin.lightWellEff;

                    } else { // Between-glass blind
                        Real64 t1 = Window::POLYF(COSB, construct.tBareVisCoef(1));
                        td2 = construct.tBareVisDiff(2);
                        rbd1 = construct.rbBareVisDiff(1);
                        rfd2 = construct.rfBareVisDiff(2);
                        Real64 tfshBd = Interp(btar.Vis.Ft.Bm[idxLo].DfTra, btar.Vis.Ft.Bm[idxHi].DfTra, interpFac);
                        tfshd = btar.Vis.Ft.Df.Tra;
                        Real64 rfshB = Interp(btar.Vis.Ft.Bm[idxLo].DfRef, btar.Vis.Ft.Bm[idxHi].DfRef, interpFac);
                        rbshd = btar.Vis.Ft.Df.Ref;
                        if (construct.TotGlassLayers == 2) { // 2 glass layers
                            transMult = t1 * (tfshBd * (1.0 + rfd2 * rbshd) + rfshB * rbd1 * tfshd) * td2 * surfWin.lightWellEff;
                        } else { // 3 glass layers; blind between layers 2 and 3
                            Real64 t2 = Window::POLYF(COSB, construct.tBareVisCoef(2));
                            td3 = construct.tBareVisDiff(3);
                            rfd3 = construct.rfBareVisDiff(3);
                            rbd2 = construct.rbBareVisDiff(2);
                            transMult = t1 * t2 * (tfshBd * (1.0 + rfd3 * rbshd) + rfshB * (rbd2 * tfshd + td2 * rbd1 * td2 * tfshd)) * td3 *
                                        surfWin.lightWellEff;
                        }
                    }

                    transBmBmMult = TVISBR * matBlind->BeamBeamTrans(ProfAng, surfShade.blind.slatAng);
                } else { // Diffusing glass
                    transMult = Window::POLYF(COSB, state.dataConstruction->Construct(IConstShaded).TransVisBeamCoef) * surfWin.glazedFrac *
                                surfWin.lightWellEff;
                } // End of check if shade, blind or diffusing glass

                if (s_surf->Surface(IWin).OriginalClass == SurfaceClass::TDD_Dome) {
                    // No beam is transmitted.  This takes care of all types of screens and blinds.
                    transBmBmMult = 0.0;
                }

                // Daylighting shelf simplification:  No beam makes it past end of shelf, all light is diffuse
                if (InShelfSurf > 0) { // Inside daylighting shelf
                    transBmBmMult = 0.0;
                }

                // DayltgInterReflectedIllumTransBmBmMult is used in the following for windows with blinds or screens to get contribution from light
                // passing directly between slats or between screen material without reflection.

                auto &wlumsk = dl->winLum(IHR)[iWinCover_Shaded];
                auto &flfwsk = FLFW[iWinCover_Shaded];
                auto &flcwsk = FLCW[iWinCover_Shaded];

                for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
                    // Should these be bare or shaded?
                    wlumsk.sky[iSky] += ZSK.sky[iSky] * transMult / Constant::Pi;
                    flfwsk.sky[iSky] += ZSK.sky[iSky] * transMult * (1.0 - surfWin.fractionUpgoing);
                    flcwsk.sky[iSky] += ZSK.sky[iSky] * transMult * surfWin.fractionUpgoing;

                    if (BlindOn || ScreenOn) {
                        if (PH > 0.0) {
                            flfwsk.sky[iSky] += ZSK.sky[iSky] * transBmBmMult;
                        } else {
                            flcwsk.sky[iSky] += ZSK.sky[iSky] * transBmBmMult;
                        }
                    }
                }

                dl->winLum(IHR)[iWinCover_Shaded].sun += ZSU * transMult / Constant::Pi;
                flfwsk.sun += ZSU * transMult * (1.0 - surfWin.fractionUpgoing);
                flcwsk.sun += ZSU * transMult * surfWin.fractionUpgoing;
                if (BlindOn || ScreenOn) {
                    if (PH > 0.0) {
                        flfwsk.sun += ZSU * transBmBmMult;
                    } else {
                        flcwsk.sun += ZSU * transBmBmMult;
                    }
                }
            } // End of window with shade, screen, blind or diffusing glass

        } // End of azimuth integration loop, ITH
    } // End of altitude integration loop, IPH

    if (OutShelfSurf > 0) { // Outside daylighting shelf
        // Add exterior diffuse illuminance due to outside shelf
        // Since all of the illuminance is added to the zone as upgoing diffuse, it can be added as a lump sum here

        TVISBR = construct.TransDiffVis; // Assume diffuse transmittance for shelf illuminance

        auto const &gilsk = dl->horIllum[IHR];
        auto &flcwsk = FLCW[iWinCover_Bare];
        for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
            // This is only an estimate because the anisotropic sky view of the shelf is not yet taken into account.
            // SurfAnisoSkyMult would be great to use but it is not available until the heat balance starts up.
            ZSK.sky[iSky] = gilsk.sky[iSky] * 1.0 * state.dataDaylightingDevicesData->Shelf(ShelfNum).OutReflectVis *
                            state.dataDaylightingDevicesData->Shelf(ShelfNum).ViewFactor;

            // SurfaceWindow(IWin)%FractionUpgoing is already set to 1.0 earlier
            flcwsk.sky[iSky] += ZSK.sky[iSky] * TVISBR * surfWin.fractionUpgoing;
        } // ISKY

        ZSU = dl->horIllum[IHR].sun * state.dataHeatBal->SurfSunlitFracHR(IHR, OutShelfSurf) *
              state.dataDaylightingDevicesData->Shelf(ShelfNum).OutReflectVis * state.dataDaylightingDevicesData->Shelf(ShelfNum).ViewFactor;
        flcwsk.sun += ZSU * TVISBR * surfWin.fractionUpgoing;
    }

    // Sky-related portion of internally reflected illuminance.
    // The inside surface area, ZoneDaylight(ZoneNum)%totInsSurfArea, and ZoneDaylight(ZoneNum)%aveVisDiffReflect,
    // were calculated in subr DayltgAveInteriorReflectance.

    for (int iWinCover = 0; iWinCover < (int)WinCover::Num; ++iWinCover) {
        auto &eintsk = dl->reflIllum(IHR)[iWinCover];
        auto const &flfwsk = FLFW[iWinCover];
        auto const &flcwsk = FLCW[iWinCover];

        for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
            // Full area of window is used in following since effect of dividers on reducing
            // effective window transmittance has already been accounted for in calc of FLFWSK and FLCWSK.
            eintsk.sky[iSky] = (flfwsk.sky[iSky] * surfWin.rhoFloorWall + flcwsk.sky[iSky] * surfWin.rhoCeilingWall) *
                               (surf.Area / surfWin.glazedFrac) / (EnclInsideSurfArea * (1.0 - dl->enclDaylight(enclNum).aveVisDiffReflect));
        } // for (iSky)
    } // for (iWinCover)

    // BEAM SOLAR RADIATION ON WINDOW

    // Beam reaching window directly (without specular reflection from exterior obstructions)

    if (state.dataHeatBal->SurfSunlitFracHR(IHR, IWin) > 0.0) {
        // Cos of angle of incidence
        COSBSun = dl->sunAngles.sinPhi * std::sin(surfWin.phi) +
                  dl->sunAngles.cosPhi * std::cos(surfWin.phi) * std::cos(dl->sunAngles.theta - surfWin.theta);

        if (COSBSun > 0.0) {
            // Multiply direct normal illuminance (normalized to 1.0 lux)
            // by incident angle factor and by fraction of window that is sunlit.
            // Note that in the following SurfSunlitFracHR accounts for possibly non-zero transmittance of
            // shading surfaces.

            ZSU1 = COSBSun * state.dataHeatBal->SurfSunlitFracHR(IHR, IWin);

            // Contribution to window luminance and downgoing flux

            // -- Bare window

            if (s_surf->Surface(IWin).OriginalClass == SurfaceClass::TDD_Dome) {
                // Unshaded visible transmittance of TDD for collimated beam from the sun
                TVISBSun = TransTDD(state, PipeNum, COSBSun, RadType::VisibleBeam) * surfWin.glazedFrac;
                dl->TDDTransVisBeam(IHR, PipeNum) = TVISBSun;

                FLFW[iWinCover_Bare].sunDisk = 0.0; // Diffuse light only

                dl->winLum(IHR)[iWinCover_Bare].sun += ZSU1 * TVISBSun / Constant::Pi;
                FLFW[iWinCover_Bare].sun += ZSU1 * TVISBSun * (1.0 - surfWin.fractionUpgoing);
                FLCW[iWinCover_Bare].sun += ZSU1 * TVISBSun * surfWin.fractionUpgoing;

            } else { // Bare window
                TVISBSun = Window::POLYF(COSBSun, construct.TransVisBeamCoef) * surfWin.glazedFrac * surfWin.lightWellEff;

                // Daylighting shelf simplification:  No beam makes it past end of shelf, all light is diffuse
                if (InShelfSurf > 0) {                  // Inside daylighting shelf
                    FLFW[iWinCover_Bare].sunDisk = 0.0; // Diffuse light only

                    // SurfaceWindow(IWin)%FractionUpgoing is already set to 1.0 earlier
                    // WLUMSU(1,IHR) = WLUMSU(1,IHR) + ZSU1 * TVISBSun / PI
                    // FLFWSU(1) = FLFWSU(1) + ZSU1 * TVISBSun * (1.0 - SurfaceWindow(IWin)%FractionUpgoing)
                    FLCW[iWinCover_Bare].sun += ZSU1 * TVISBSun * surfWin.fractionUpgoing;
                } else { // Normal window
                    FLFW[iWinCover_Bare].sunDisk = ZSU1 * TVISBSun;
                }
            }

            // -- Window with shade, screen, blind or diffusing glass
            if (ShadeOn || BlindOn || ScreenOn || s_surf->SurfWinSolarDiffusing(IWin)) {
                Real64 transBmBmMult = 0.0;
                Real64 transMult = 0.0;

                if (ShadeOn || ScreenOn || s_surf->SurfWinSolarDiffusing(IWin)) { // Shade or screen on or diffusing glass
                    if (s_surf->Surface(IWin).OriginalClass == SurfaceClass::TDD_Dome) {
                        // Shaded visible transmittance of TDD for collimated beam from the sun
                        transMult = TransTDD(state, PipeNum, COSBSun, RadType::VisibleBeam) * surfWin.glazedFrac;

                    } else if (ScreenOn) {
                        auto const *screen = dynamic_cast<Material::MaterialScreen const *>(s_mat->materials(surfWin.screenNum));
                        assert(screen != nullptr);
                        Real64 phi = std::abs(dl->sunAngles.phi - surfWin.phi);
                        Real64 theta = std::abs(dl->sunAngles.theta - surfWin.theta);
                        int ip1, ip2, it1, it2;
                        BilinearInterpCoeffs coeffs;
                        Material::NormalizePhiTheta(phi, theta);
                        Material::GetPhiThetaIndices(phi, theta, screen->dPhi, screen->dTheta, ip1, ip2, it1, it2);
                        GetBilinearInterpCoeffs(
                            phi, theta, ip1 * screen->dPhi, ip2 * screen->dPhi, it1 * screen->dTheta, it2 * screen->dTheta, coeffs);
                        Real64 BmBmTransVis = BilinearInterp(screen->btars[ip1][it1].BmTransVis,
                                                             screen->btars[ip1][it2].BmTransVis,
                                                             screen->btars[ip2][it1].BmTransVis,
                                                             screen->btars[ip2][it2].BmTransVis,
                                                             coeffs);

                        transMult = BmBmTransVis * surfWin.glazedFrac * surfWin.lightWellEff;
                    } else {
                        int IConstShaded = s_surf->SurfWinActiveShadedConstruction(IWin);
                        if (s_surf->SurfWinSolarDiffusing(IWin)) {
                            IConstShaded = surf.Construction;
                        }
                        transMult = Window::POLYF(COSBSun, state.dataConstruction->Construct(IConstShaded).TransVisBeamCoef) * surfWin.glazedFrac *
                                    surfWin.lightWellEff;
                    }

                } else { // Blind on
                    auto const &surfShade = s_surf->surfShades(IWin);
                    // As long as only interior blinds are allowed for TDDs, no need to change TransMult calculation
                    // for TDDs because it is based on TVISBSun which is correctly calculated for TDDs above.
                    auto const *matBlind = dynamic_cast<Material::MaterialBlind const *>(s_mat->materials(surfShade.blind.matNum));
                    assert(matBlind != nullptr);

                    // These are "cached" in the surfShade struct
                    auto &btar = surfShade.blind.TAR;
                    int idxLo = surfShade.blind.profAngIdxLo;
                    int idxHi = surfShade.blind.profAngIdxHi;
                    int interpFac = surfShade.blind.profAngInterpFac;
                    TransBlBmDiffFront = Interp(btar.Vis.Ft.Bm[idxLo].DfTra, btar.Vis.Ft.Bm[idxHi].DfTra, interpFac);

                    if (ShType == WinShadingType::IntBlind) { // Interior blind
                        // TH CR 8121, 7/7/2010
                        // ReflBlBmDiffFront = WindowManager::InterpProfAng(ProfAng,Blind(BlNum)%VisFrontBeamDiffRefl)
                        ReflBlBmDiffFront = Interp(btar.Vis.Ft.Bm[idxLo].DfRef, btar.Vis.Ft.Bm[idxHi].DfRef, interpFac);

                        // TH added 7/12/2010 for CR 8121
                        ReflBlDiffDiffFront = btar.Vis.Ft.Df.Ref;
                        TransBlDiffDiffFront = btar.Vis.Ft.Df.Tra;

                        transMult = TVISBSun * (TransBlBmDiffFront + ReflBlBmDiffFront * ReflGlDiffDiffBack * TransBlDiffDiffFront /
                                                                         (1.0 - ReflBlDiffDiffFront * ReflGlDiffDiffBack));

                    } else if (ShType == WinShadingType::ExtBlind) { // Exterior blind
                        transMult = TransBlBmDiffFront * (construct.TransDiffVis / (1.0 - ReflGlDiffDiffFront * btar.Vis.Bk.Df.Ref)) *
                                    surfWin.glazedFrac * surfWin.lightWellEff;

                    } else { // Between-glass blind
                        Real64 t1 = Window::POLYF(COSBSun, construct.tBareVisCoef(1));
                        Real64 tfshBd = Interp(btar.Vis.Ft.Bm[idxLo].DfTra, btar.Vis.Ft.Bm[idxHi].DfTra, interpFac);
                        Real64 rfshB = Interp(btar.Vis.Ft.Bm[idxLo].DfRef, btar.Vis.Ft.Bm[idxHi].DfRef, interpFac);
                        if (construct.TotGlassLayers == 2) { // 2 glass layers
                            transMult = t1 * (tfshBd * (1.0 + rfd2 * rbshd) + rfshB * rbd1 * tfshd) * td2 * surfWin.lightWellEff;
                        } else { // 3 glass layers; blind between layers 2 and 3
                            Real64 t2 = Window::POLYF(COSBSun, construct.tBareVisCoef(2));
                            transMult = t1 * t2 * (tfshBd * (1.0 + rfd3 * rbshd) + rfshB * (rbd2 * tfshd + td2 * rbd1 * td2 * tfshd)) * td3 *
                                        surfWin.lightWellEff;
                        }
                    }

                    transBmBmMult = TVISBSun * matBlind->BeamBeamTrans(surfShade.blind.profAng, surfShade.blind.slatAng);
                } // ShadeOn/ScreenOn/BlindOn/Diffusing glass

                if (s_surf->Surface(IWin).OriginalClass == SurfaceClass::TDD_Dome) {
                    transBmBmMult = 0.0; // No beam, diffuse only
                }

                // Daylighting shelf simplification:  No beam makes it past end of shelf, all light is diffuse
                if (InShelfSurf > 0) {   // Inside daylighting shelf
                    transBmBmMult = 0.0; // No beam, diffuse only
                    // SurfaceWindow(IWin)%FractionUpgoing is already set to 1.0 earlier
                }

                dl->winLum(IHR)[iWinCover_Shaded].sun += ZSU1 * transMult / Constant::Pi;
                dl->winLum(IHR)[iWinCover_Shaded].sunDisk = ZSU1 * transBmBmMult / Constant::Pi;
                FLFW[iWinCover_Shaded].sun += ZSU1 * transMult * (1.0 - surfWin.fractionUpgoing);
                FLFW[iWinCover_Shaded].sunDisk = ZSU1 * transBmBmMult;
                FLCW[iWinCover_Shaded].sun += ZSU1 * transMult * surfWin.fractionUpgoing;
            } // if (BlindOn || ShadeOn)
        } // if (COSBSun > 0)
    } // if (SurfSunlitFracHR > 0)

    // Beam reaching window after specular reflection from exterior obstruction

    // In the following, Beam normal illuminance times ZSU1refl = illuminance on window due to
    // specular reflection from exterior surfaces

    if (s_surf->CalcSolRefl && s_surf->Surface(IWin).OriginalClass != SurfaceClass::TDD_Dome) {

        ZSU1refl = s_surf->SurfReflFacBmToBmSolObs(IHR, IWin);

        if (ZSU1refl > 0.0) {
            // Contribution to window luminance and downgoing flux

            // -- Bare window. We use diffuse-diffuse transmittance here rather than beam-beam to avoid
            //    complications due to specular reflection from multiple exterior surfaces

            TVisSunRefl = construct.TransDiffVis * surfWin.glazedFrac * surfWin.lightWellEff;
            // In the following it is assumed that all reflected beam is going downward, as it would be in the
            // important case of reflection from a highly glazed facade of a neighboring building. However, in
            // rare cases (such as upward specular reflection from a flat horizontal skylight) it may
            // actually be going upward.
            FLFW[iWinCover_Bare].sunDisk += ZSU1refl * TVisSunRefl;

            // -- Window with shade, blind or diffusing glass

            if (ShadeOn || BlindOn || ScreenOn || s_surf->SurfWinSolarDiffusing(IWin)) {
                Real64 transMult = 0.0;

                if (ShadeOn || s_surf->SurfWinSolarDiffusing(IWin)) { // Shade on or diffusing glass
                    int IConstShaded = s_surf->SurfWinActiveShadedConstruction(IWin);
                    if (s_surf->SurfWinSolarDiffusing(IWin)) {
                        IConstShaded = s_surf->Surface(IWin).Construction;
                    }
                    transMult = state.dataConstruction->Construct(IConstShaded).TransDiffVis * surfWin.glazedFrac * surfWin.lightWellEff;

                } else if (ScreenOn) { // Exterior screen on
                    auto const *screen = dynamic_cast<Material::MaterialScreen const *>(s_mat->materials(surfWin.screenNum));
                    assert(screen != nullptr);
                    Real64 TransScDiffDiffFront = screen->DfTransVis;

                    transMult = TransScDiffDiffFront *
                                (state.dataConstruction->Construct(IConst).TransDiffVis / (1.0 - ReflGlDiffDiffFront * ReflScDiffDiffBack)) *
                                surfWin.glazedFrac * surfWin.lightWellEff;

                } else { // Blind on

                    auto const &surfShade = s_surf->surfShades(IWin);
                    auto const &btar = surfShade.blind.TAR;
                    [[maybe_unused]] auto const *matBlind = dynamic_cast<Material::MaterialBlind const *>(s_mat->materials(surfShade.blind.matNum));

                    assert(matBlind != nullptr);

                    TransBlDiffDiffFront = btar.Vis.Ft.Df.Tra;
                    if (ShType == WinShadingType::IntBlind) { // Interior blind
                        ReflBlDiffDiffFront = btar.Vis.Ft.Df.Ref;
                        transMult = TVisSunRefl * (TransBlDiffDiffFront + ReflBlDiffDiffFront * ReflGlDiffDiffBack * TransBlDiffDiffFront /
                                                                              (1.0 - ReflBlDiffDiffFront * ReflGlDiffDiffBack));

                    } else if (ShType == WinShadingType::ExtBlind) { // Exterior blind
                        transMult = TransBlDiffDiffFront * (construct.TransDiffVis / (1.0 - ReflGlDiffDiffFront * btar.Vis.Bk.Df.Ref)) *
                                    surfWin.glazedFrac * surfWin.lightWellEff;

                    } else { // Between-glass blind
                        Real64 t1 = construct.tBareVisDiff(1);
                        Real64 tfshBd = btar.Vis.Ft.Df.Tra;
                        Real64 rfshB = btar.Vis.Ft.Df.Ref;
                        if (construct.TotGlassLayers == 2) { // 2 glass layers
                            transMult = t1 * (tfshBd * (1.0 + rfd2 * rbshd) + rfshB * rbd1 * tfshd) * td2 * surfWin.lightWellEff;
                        } else { // 3 glass layers; blind between layers 2 and 3
                            Real64 t2 = construct.tBareVisDiff(2);
                            transMult = t1 * t2 * (tfshBd * (1.0 + rfd3 * rbshd) + rfshB * (rbd2 * tfshd + td2 * rbd1 * td2 * tfshd)) * td3 *
                                        surfWin.lightWellEff;
                        }
                    } // End of check of interior/exterior/between-glass blind
                } // if (Blind)

                dl->winLum(IHR)[iWinCover_Shaded].sun += ZSU1refl * transMult / Constant::Pi;
                FLFW[iWinCover_Shaded].sun += ZSU1refl * transMult * (1.0 - surfWin.fractionUpgoing);
                FLCW[iWinCover_Shaded].sun += ZSU1refl * transMult * surfWin.fractionUpgoing;
            } // End of check if window has shade, blind or diffusing glass
        } // End of check if ZSU1refl > 0.0
    } // End of check if solar reflections are in effect

    // Sun-related portion of internally reflected illuminance

    // Full area of window is used in following since effect of dividers on reducing
    // effective window transmittance already accounted for in calc of FLFWSU and FLCWSU
    // CR 7869 added effect of intervening interior windows on transmittance and
    // added inside surface area of adjacent zone
    for (int iWinCover = 0; iWinCover < (int)WinCover::Num; ++iWinCover) {
        dl->reflIllum(IHR)[iWinCover].sun = (FLFW[iWinCover].sun * surfWin.rhoFloorWall + FLCW[iWinCover].sun * surfWin.rhoCeilingWall) *
                                            (surf.Area / surfWin.glazedFrac) / (EnclInsideSurfArea * (1.0 - thisEnclDaylight.aveVisDiffReflect));

        dl->reflIllum(IHR)[iWinCover].sunDisk = FLFW[iWinCover].sunDisk * surfWin.rhoFloorWall * (surf.Area / surfWin.glazedFrac) /
                                                (EnclInsideSurfArea * (1.0 - thisEnclDaylight.aveVisDiffReflect));
    }
} // DayltgInterReflectedIllum()

void ComplexFenestrationLuminances(EnergyPlusData &state,
                                   int const IWin,
                                   int const WinEl,
                                   int const NBasis,
                                   int const IHR,
                                   int const iRefPoint,
                                   Array1D<Illums> &ElementLuminance, // luminance at window element (exterior side)
                                   CalledFor const CalledFrom,
                                   int const MapNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   June 2013

    Vector3<Real64> obsHitPt;    // Coordinates of hit point on an obstruction (m)
    Vector3<Real64> groundHitPt; // Coordinates of point that ray from window center hits the ground (m)

    auto &dl = state.dataDayltg;
    auto &s_surf = state.dataSurface;

    int CurCplxFenState = s_surf->SurfaceWindow(IWin).ComplexFen.CurrentState;
    auto &complexWinGeom = state.dataBSDFWindow->ComplexWind(IWin).Geom(CurCplxFenState);
    // Calculate luminance from sky and sun excluding exterior obstruction transmittances and obstruction multipliers
    int SolBmIndex = complexWinGeom.SolBmIndex(IHR, state.dataGlobal->TimeStep);
    for (int iIncElem = 1; iIncElem <= NBasis; ++iIncElem) {
        Real64 LambdaInc = complexWinGeom.Inc.Lamda(iIncElem);
        // COSB = ComplexWind(IWin)%Geom(CurCplxFenState)%CosInc(iIncElem)
        // DA = ComplexWind(IWin)%Geom(CurCplxFenState)%DAInc(iIncElem)
        Real64 Altitude = complexWinGeom.pInc(iIncElem).Altitude;
        Real64 Azimuth = complexWinGeom.pInc(iIncElem).Azimuth;
        auto &elemLum = ElementLuminance(iIncElem);
        auto const &gilsk = dl->horIllum[IHR];

        if (Altitude > 0.0) {
            // Ray from sky element
            for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
                elemLum.sky[iSky] = DayltgSkyLuminance(state, static_cast<SkyType>(iSky), Azimuth, Altitude) * LambdaInc;
            }
        } else if (Altitude < 0.0) {
            // Ray from ground element
            // BeamObstrMultiplier = ComplexWind(IWin)%DaylghtGeom(CurCplxFenState)%GndObstrMultiplier(WinEl, iIncElem)
            for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
                elemLum.sky[iSky] = gilsk.sky[iSky] * state.dataEnvrn->GndReflectanceForDayltg / Constant::Pi * LambdaInc;
            }
            elemLum.sun = dl->horIllum[IHR].sun * state.dataEnvrn->GndReflectanceForDayltg / Constant::Pi * LambdaInc;
        } else {
            // Ray from the element which is half sky and half ground
            for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
                // in this case half of the pach is coming from the sky and half from the ground
                elemLum.sky[iSky] = 0.5 * DayltgSkyLuminance(state, static_cast<SkyType>(iSky), Azimuth, Altitude) * LambdaInc +
                                    0.5 * gilsk.sky[iSky] * state.dataEnvrn->GndReflectanceForDayltg / Constant::Pi * LambdaInc;
            }
            elemLum.sun = 0.5 * dl->horIllum[IHR].sun * state.dataEnvrn->GndReflectanceForDayltg / Constant::Pi * LambdaInc;
        }
        // Sun beam calculations
        if ((SolBmIndex == iIncElem) && (state.dataHeatBal->SurfSunlitFracHR(IHR, IWin) > 0.0)) {
            elemLum.sunDisk = 1.0;
        }
    }

    auto const &complexWinDaylightGeom = state.dataBSDFWindow->ComplexWind(IWin).DaylghtGeom(CurCplxFenState);

    if (CalledFrom == CalledFor::RefPoint) {
        auto const &complexWinRefPoint = complexWinDaylightGeom.RefPoint(iRefPoint);
        // add exterior obstructions transmittances to calculated luminances
        for (int iReflElem = 1; iReflElem <= complexWinRefPoint.NReflSurf(WinEl); ++iReflElem) {
            Real64 ObstrTrans = complexWinRefPoint.TransOutSurf(iReflElem, WinEl);
            int iReflElemIndex = complexWinRefPoint.RefSurfIndex(iReflElem, WinEl);

            auto &elemLum = ElementLuminance(iReflElemIndex);
            for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
                elemLum.sky[iSky] *= ObstrTrans;
            }
            elemLum.sun *= ObstrTrans;
            elemLum.sunDisk *= ObstrTrans;
        }

        // add exterior ground element obstruction multipliers to calculated luminances. For sun reflection, calculate if
        // sun reaches the ground for that point
        Vector3<Real64> const SUNCOS_IHR = s_surf->SurfSunCosHourly(IHR);
        for (int iGndElem = 1; iGndElem <= complexWinRefPoint.NGnd(WinEl); ++iGndElem) {
            // case for sky elements. Integration is done over upper ground hemisphere to determine how many obstructions
            // were hit in the process

            Real64 BeamObstrMultiplier = complexWinRefPoint.GndObstrMultiplier(iGndElem, WinEl);
            int iGndElemIndex = complexWinRefPoint.GndIndex(iGndElem, WinEl);

            auto &elemLum = ElementLuminance(iGndElemIndex);
            for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
                elemLum.sky[iSky] *= BeamObstrMultiplier;
            }

            // direct sun disk reflect off the ground
            Real64 SunObstrMultiplier = 1.0;
            if (s_surf->CalcSolRefl) {
                // Sun reaches ground point if vector from this point to the sun is unobstructed
                for (int ObsSurfNum : s_surf->AllShadowPossObstrSurfaceList) {
                    groundHitPt = complexWinRefPoint.GndPt(iGndElem, WinEl);
                    bool hitObs = PierceSurface(state, ObsSurfNum, groundHitPt, SUNCOS_IHR, obsHitPt);
                    if (hitObs) {
                        SunObstrMultiplier = 0.0;
                        break;
                    }
                }
            }
            elemLum.sun *= SunObstrMultiplier;
        }

    } else { // if (CalledFrom != RefPoint)

        auto const &complexWinIllumMap = complexWinDaylightGeom.IlluminanceMap(iRefPoint, MapNum);
        // add exterior obstructions transmittances to calculated luminances
        for (int iReflElem = 1; iReflElem <= complexWinIllumMap.NReflSurf(WinEl); ++iReflElem) {
            Real64 ObstrTrans = complexWinIllumMap.TransOutSurf(iReflElem, WinEl);
            int iReflElemIndex = complexWinIllumMap.RefSurfIndex(iReflElem, WinEl);
            auto &elemLum = ElementLuminance(iReflElemIndex);

            for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
                elemLum.sky[iSky] *= ObstrTrans;
            }
            elemLum.sun *= ObstrTrans;
            elemLum.sunDisk *= ObstrTrans;
        }

        // add exterior ground element obstruction multipliers to calculated luminances. For sun reflection, calculate if
        // sun reaches the ground for that point
        Vector3<Real64> const SUNCOS_IHR = s_surf->SurfSunCosHourly(IHR);
        for (int iGndElem = 1; iGndElem <= complexWinIllumMap.NGnd(WinEl); ++iGndElem) {
            // case for sky elements. Integration is done over upper ground hemisphere to determine how many obstructions
            // were hit in the process
            Real64 BeamObstrMultiplier = complexWinIllumMap.GndObstrMultiplier(iGndElem, WinEl);
            int iGndElemIndex = complexWinIllumMap.GndIndex(iGndElem, WinEl);

            auto &elemLum = ElementLuminance(iGndElemIndex);
            for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
                elemLum.sky[iSky] *= BeamObstrMultiplier;
            }

            // direct sun disk reflect off the ground
            Real64 SunObstrMultiplier = 1.0;
            if (s_surf->CalcSolRefl) {
                // Sun reaches ground point if vector from this point to the sun is unobstructed
                for (int ObsSurfNum : s_surf->AllShadowPossObstrSurfaceList) {
                    groundHitPt = complexWinIllumMap.GndPt(iGndElem, WinEl);

                    bool hitObs = PierceSurface(state, ObsSurfNum, groundHitPt, SUNCOS_IHR, obsHitPt);
                    if (hitObs) {
                        SunObstrMultiplier = 0.0;
                        break;
                    }
                }
            }
            elemLum.sun *= SunObstrMultiplier;
        }
    } // if (CalledFrom == RefPoint)
} // ComplexFenestrationLuminances()

void DayltgInterReflectedIllumComplexFenestration(EnergyPlusData &state,
                                                  int const IWin,            // Window index
                                                  int const WinEl,           // Current window element counter
                                                  int const IHR,             // Hour of day
                                                  int const daylightCtrlNum, // Daylighting control number
                                                  int const iRefPoint,       // reference point counter
                                                  CalledFor const CalledFrom,
                                                  int const MapNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   April 2013

    // PURPOSE OF THIS SUBROUTINE:
    // Called from CalcDayltgCoefficients for each complex (bsdf) fenestration and reference point in a daylit
    // space, for each sun position. Calculates illuminance (EINTSK and EINTSU) at reference point due
    // to internally reflected light by integrating to determine the amount of flux from
    // sky and ground (and beam reflected from obstructions) transmitted through
    // the center of the window and then reflecting this
    // light from the inside surfaces of the space.

    auto &dl = state.dataDayltg;
    auto &s_surf = state.dataSurface;

    Array1D<Illums> FL; // Sky related luminous flux
    // Array1D<Real64> FLSU;     // Sun related luminous flux, excluding entering beam
    // Array1D<Real64> FLSUdisk; // Sun related luminous flux, due to entering beam

    Array1D<Illums> FirstFlux; // Sky related first reflected flux
    // Array1D<Real64> FirstFluxSU;     // Sun related first reflected flux, excluding entering beam
    // Array1D<Real64> FirstFluxSUdisk; // Sun related first reflected flux, due to entering beam

    Array1D<Illums> ElementLuminance; // sky related luminance at window element (exterior side)
    // Array1D<Real64> ElementLuminanceSun;     // sun related luminance at window element (exterior side), exluding beam
    // Array1D<Real64> ElementLuminanceSunDisk; // sun related luminance at window element (exterior side), due to sun beam
    Illums FLTot;
    // Real64 FLSUTot;
    // Real64 FLSUdiskTot;

    // Total for first relflected fluxes
    Illums FFTot = Illums();
    // Real64 FFSUTot;
    // Real64 FFSUdiskTot;

    int NIncBasis;
    int SolBmIndex; // index of current sun position

    Real64 LambdaInc; // current lambda value for incoming direction
    // REAL(r64) :: LambdaTrn  ! current lambda value for incoming direction
    Real64 dirTrans; // directional bsdf transmittance

    auto const &surf = s_surf->Surface(IWin);
    auto const &surfWin = s_surf->SurfaceWindow(IWin);

    int CurCplxFenState = surfWin.ComplexFen.CurrentState;
    auto &complexWinGeom = state.dataBSDFWindow->ComplexWind(IWin).Geom(CurCplxFenState);
    int iConst = surfWin.ComplexFen.State(CurCplxFenState).Konst;
    int NTrnBasis = complexWinGeom.Trn.NBasis;

    if (!allocated(FL)) {
        FL.allocate(NTrnBasis);
    }
    FL = Illums();
    // if (!allocated(FLSU)) FLSU.dimension(NTrnBasis, 0.0);
    // if (!allocated(FLSUdisk)) FLSUdisk.dimension(NTrnBasis, 0.0);

    if (!allocated(FirstFlux)) {
        FirstFlux.allocate(NTrnBasis);
    }
    FirstFlux = Illums();
    // if (!allocated(FirstFluxSU)) FirstFluxSU.dimension(NTrnBasis, 0.0);
    // if (!allocated(FirstFluxSUdisk)) FirstFluxSUdisk.dimension(NTrnBasis, 0.0);

    NIncBasis = complexWinGeom.Inc.NBasis;
    if (!allocated(ElementLuminance)) {
        ElementLuminance.allocate(NIncBasis);
    }
    ElementLuminance = Illums();
    // if (!allocated(ElementLuminanceSun)) ElementLuminanceSun.dimension(NIncBasis, 0.0);
    // if (!allocated(ElementLuminanceSunDisk)) ElementLuminanceSunDisk.dimension(NIncBasis, 0.0);

    // Integration over sky/ground/sun elements is done over window incoming basis element and flux is calculated for each
    // outgoing direction. This is used to calculate first reflected flux

    ComplexFenestrationLuminances(state, IWin, WinEl, NIncBasis, IHR, iRefPoint, ElementLuminance, CalledFrom, MapNum);

    // luminance from sun disk needs to include fraction of sunlit area
    SolBmIndex = complexWinGeom.SolBmIndex(IHR, state.dataGlobal->TimeStep);
    Real64 COSIncSun = (SolBmIndex > 0) ? complexWinGeom.CosInc(SolBmIndex) : 0.0;

    for (int i = 1; i <= (int)ElementLuminance.size(); ++i) {
        ElementLuminance(i).sunDisk *= state.dataHeatBal->SurfSunlitFracHR(IHR, IWin) * COSIncSun;
    }

    //        FLSKTot = 0.0;
    FLTot.sun = 0.0;
    FLTot.sunDisk = 0.0;
    FFTot.sun = 0.0;
    FFTot.sunDisk = 0.0;
    // now calculate flux into each outgoing direction by integrating over all incoming directions
    for (int iBackElem = 1; iBackElem <= NTrnBasis; ++iBackElem) {
        for (int iIncElem = 1; iIncElem <= NIncBasis; ++iIncElem) {
            LambdaInc = complexWinGeom.Inc.Lamda(iIncElem);
            dirTrans = state.dataConstruction->Construct(iConst).BSDFInput.VisFrtTrans(iBackElem, iIncElem);

            auto &fl = FL(iBackElem);
            auto const &elemLum = ElementLuminance(iIncElem);
            for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
                fl.sky[iSky] += dirTrans * LambdaInc * elemLum.sky[iSky];
            }

            fl.sun += dirTrans * LambdaInc * elemLum.sun;
            fl.sunDisk += dirTrans * LambdaInc * elemLum.sunDisk;
        }

        auto &firstFlux = FirstFlux(iBackElem);
        auto const &fl = FL(iBackElem);
        for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
            firstFlux.sky[iSky] = fl.sky[iSky] * complexWinGeom.AveRhoVisOverlap(iBackElem);
            FFTot.sky[iSky] += firstFlux.sky[iSky];
            //                FLSKTot( iSky ) += FLSK( iSky, iBackElem );
        }
        firstFlux.sun = fl.sun * complexWinGeom.AveRhoVisOverlap(iBackElem);
        FFTot.sun += firstFlux.sun;
        FLTot.sun += fl.sun;

        firstFlux.sunDisk = fl.sunDisk * complexWinGeom.AveRhoVisOverlap(iBackElem);
        FFTot.sunDisk += firstFlux.sunDisk;
        FLTot.sunDisk += fl.sunDisk;
    }

    auto const &thisEnclDaylight = dl->enclDaylight(dl->daylightControl(daylightCtrlNum).enclIndex);
    Real64 EnclInsideSurfArea = thisEnclDaylight.totInsSurfArea;

    auto &eintsk = dl->reflIllum(IHR)[iWinCover_Bare];
    for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
        eintsk.sky[iSky] = FFTot.sky[iSky] * (surf.Area / surfWin.glazedFrac) / (EnclInsideSurfArea * (1.0 - thisEnclDaylight.aveVisDiffReflect));
    } // for (iSky)

    dl->reflIllum(IHR)[iWinCover_Bare].sun =
        FFTot.sun * (surf.Area / surfWin.glazedFrac) / (EnclInsideSurfArea * (1.0 - thisEnclDaylight.aveVisDiffReflect));
    dl->reflIllum(IHR)[iWinCover_Bare].sunDisk =
        FFTot.sunDisk * (surf.Area / surfWin.glazedFrac) / (EnclInsideSurfArea * (1.0 - thisEnclDaylight.aveVisDiffReflect));

    if (allocated(FL)) {
        FL.deallocate();
    }
    // if (allocated(FLSU)) FLSU.deallocate();
    // if (allocated(FLSUdisk)) FLSUdisk.deallocate();

    if (allocated(FirstFlux)) {
        FirstFlux.deallocate();
    }
    // if (allocated(FirstFluxSU)) FirstFluxSU.deallocate();
    // if (allocated(FirstFluxSUdisk)) FirstFluxSUdisk.deallocate();

    if (allocated(ElementLuminance)) {
        ElementLuminance.deallocate();
    }
    // if (allocated(ElementLuminanceSun)) ElementLuminanceSun.deallocate();
    // if (allocated(ElementLuminanceSunDisk)) ElementLuminanceSunDisk.deallocate();
}

void DayltgDirectIllumComplexFenestration(EnergyPlusData &state,
                                          int const IWin,      // Window index
                                          int const WinEl,     // Current window element counter
                                          int const IHR,       // Hour of day
                                          int const iRefPoint, // reference point index
                                          CalledFor const CalledFrom,
                                          int const MapNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   June 2013

    auto &dl = state.dataDayltg;
    auto &s_surf = state.dataSurface;

    // Luminances from different sources to the window
    Array1D<Illums> ElementLuminance; // sky related luminance at window element (exterior side)
    // Array1D<Real64> ElementLuminanceSun; // sun related luminance at window element (exterior side),
    // exluding beam
    // Array1D<Real64> ElementLuminanceSunDisk; // sun related luminance at window element (exterior side),
    // due to sun beam

    int RefPointIndex; // reference point patch number

    Real64 dirTrans;    // directional BSDF transmittance
    Real64 dOmega;      // solid view angle of current element
    Real64 zProjection; // z-axe projection of solid view angle (used to calculate amount of light at horizontal surface
    // laying at reference point)

    int CurCplxFenState = s_surf->SurfaceWindow(IWin).ComplexFen.CurrentState;
    auto &complexWin = state.dataBSDFWindow->ComplexWind(IWin);
    int iConst = s_surf->SurfaceWindow(IWin).ComplexFen.State(CurCplxFenState).Konst;
    int NIncBasis = complexWin.Geom(CurCplxFenState).Inc.NBasis;

    if (!allocated(ElementLuminance)) {
        ElementLuminance.allocate(NIncBasis);
    }
    ElementLuminance = Illums();
    // if (!allocated(ElementLuminanceSun)) ElementLuminanceSun.dimension(NIncBasis, 0.0);
    // if (!allocated(ElementLuminanceSunDisk)) ElementLuminanceSunDisk.dimension(NIncBasis, 0.0);

    ComplexFenestrationLuminances(state, IWin, WinEl, NIncBasis, IHR, iRefPoint, ElementLuminance, CalledFrom, MapNum);

    // find number of outgoing basis towards current reference point
    if (CalledFrom == CalledFor::RefPoint) {
        RefPointIndex = complexWin.DaylghtGeom(CurCplxFenState).RefPoint(iRefPoint).RefPointIndex(WinEl);
        dOmega = complexWin.RefPoint(iRefPoint).SolidAngle(WinEl);
        zProjection = complexWin.RefPoint(iRefPoint).SolidAngleVec(WinEl).z;
    } else if (CalledFrom == CalledFor::MapPoint) {
        assert(MapNum > 0);
        RefPointIndex = complexWin.DaylghtGeom(CurCplxFenState).IlluminanceMap(iRefPoint, MapNum).RefPointIndex(WinEl);
        dOmega = complexWin.IlluminanceMap(iRefPoint, MapNum).SolidAngle(WinEl);
        zProjection = complexWin.IlluminanceMap(iRefPoint, MapNum).SolidAngleVec(WinEl).z;
    }

    Illums WinLum = Illums();
    Illums EDir = Illums();

    for (int iIncElem = 1; iIncElem <= NIncBasis; ++iIncElem) {
        // LambdaInc = ComplexWind(IWin)%Geom(CurCplxFenState)%Inc%Lamda(iIncElem)
        dirTrans = state.dataConstruction->Construct(iConst).BSDFInput.VisFrtTrans(RefPointIndex, iIncElem);

        auto const &elemLum = ElementLuminance(iIncElem);
        for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
            WinLum.sky[iSky] += dirTrans * elemLum.sky[iSky];
        }

        WinLum.sun += dirTrans * elemLum.sun;

        // For sun disk need to go throug outgoing directions and see which directions actually contain reference point
    }

    if (zProjection > 0.0) {
        for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
            EDir.sky[iSky] = WinLum.sky[iSky] * dOmega * zProjection;
        }
        EDir.sun = WinLum.sun * dOmega * zProjection;
    }

    // Store solution in global variables
    auto &avwlsk = dl->avgWinLum(IHR)[iWinCover_Bare];
    auto &edirsk = dl->dirIllum(IHR)[iWinCover_Bare];

    for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
        avwlsk.sky[iSky] += WinLum.sky[iSky];
        edirsk.sky[iSky] += EDir.sky[iSky];
    }

    dl->avgWinLum(IHR)[iWinCover_Bare].sun += WinLum.sun;
    dl->dirIllum(IHR)[iWinCover_Bare].sun += EDir.sun;
    // AVWLSUdisk(1,IHR) = AVWLSUdisk(1,IHR) + WinLumSUdisk
} // DayltgDirectIllumComplexFenestration()

void DayltgDirectSunDiskComplexFenestration(EnergyPlusData &state,
                                            int const iWin,  // Window index
                                            int const iHour, // Hour of day
                                            int const iRefPoint,
                                            int const NumEl,            // Total number of window elements
                                            Real64 const AZVIEW,        // Azimuth of view vector in absolute coord system for
                                            CalledFor const CalledFrom, // indicate  which type of routine called this routine
                                            int const MapNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Simon Vidanovic
    //       DATE WRITTEN   June 2013

    // PURPOSE OF THIS SUBROUTINE:
    // Calculate illuminance from sun disk for complex fenestration systems

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    auto &dl = state.dataDayltg;
    auto &s_surf = state.dataSurface;

    assert(CalledFrom != CalledFor::MapPoint || MapNum > 0);

    auto const &window = s_surf->SurfaceWindow(iWin);
    int CurCplxFenState = window.ComplexFen.CurrentState;
    int iConst = window.ComplexFen.State(CurCplxFenState).Konst;

    auto const &complexWindow = state.dataBSDFWindow->ComplexWind(iWin);
    auto const &complexWindowGeom = complexWindow.Geom(CurCplxFenState);
    auto const &complexWindowDayltgGeom = complexWindow.DaylghtGeom(CurCplxFenState);
    int SolBmIndex = complexWindowGeom.SolBmIndex(iHour, state.dataGlobal->TimeStep);

    Real64 WindowSolidAngleDaylightPoint = (CalledFrom == CalledFor::RefPoint) ? window.refPts(iRefPoint).solidAngWtd : 0.0;
    if (WindowSolidAngleDaylightPoint < 1e-6) {
        return;
    }

    Illums WinLum;
    Illums ElemLum;

    int NTrnBasis = complexWindowGeom.Trn.NBasis;
    for (int iTrnElem = 1; iTrnElem <= NTrnBasis; ++iTrnElem) {
        // if ray from any part of the window can reach reference point
        int refPointIntersect = (CalledFrom == CalledFor::RefPoint)
                                    ? complexWindowDayltgGeom.RefPoint(iRefPoint).RefPointIntersection(iTrnElem)
                                    : complexWindowDayltgGeom.IlluminanceMap(iRefPoint, MapNum).RefPointIntersection(iTrnElem);

        if (refPointIntersect == 0) {
            continue;
        }

        Real64 PosFac = (CalledFrom == CalledFor::RefPoint) ? complexWindowDayltgGeom.RefPoint(iRefPoint).RefPtIntPosFac(iTrnElem)
                                                            : complexWindowDayltgGeom.IlluminanceMap(iRefPoint, MapNum).RefPtIntPosFac(iTrnElem);

        Real64 RayZ = -complexWindowGeom.sTrn(iTrnElem).z;

        // Need to recalculate position factor for dominant direction in case of specular bsdf.  Otherwise this will produce
        // very inaccurate results because of position factor of the sun and bsdf pach can vary by lot
        if (iTrnElem == SolBmIndex) {
            Real64 XR = std::tan(std::abs(Constant::PiOvr2 - AZVIEW - dl->sunAngles.theta) + 0.001);
            Real64 YR = std::tan(dl->sunAngles.phi + 0.001);
            PosFac = DayltgGlarePositionFactor(XR, YR);
            RayZ = dl->sunAngles.sinPhi;
        }

        if (PosFac == 0.0) {
            continue;
        }

        Real64 dirTrans = (SolBmIndex > 0) ? state.dataConstruction->Construct(iConst).BSDFInput.VisFrtTrans(iTrnElem, SolBmIndex) : 0.0;
        Real64 LambdaTrn = complexWindowGeom.Trn.Lamda(iTrnElem);
        Vector3<Real64> V = -complexWindowGeom.sTrn(iTrnElem);
        Vector3<Real64> RWin = s_surf->Surface(iWin).Centroid;
        Real64 TransBeam = DayltgHitObstruction(state, iHour, iWin, RWin, V);

        WinLum.sunDisk += (14700.0 * std::sqrt(0.000068 * PosFac) * double(NumEl) / std::pow(WindowSolidAngleDaylightPoint, 0.8)) * dirTrans *
                          LambdaTrn * TransBeam;

        ElemLum.sunDisk += RayZ * dirTrans * LambdaTrn * TransBeam;
    } // for (iTrnElem)

    dl->avgWinLum(iHour)[iWinCover_Bare].sunDisk = WinLum.sunDisk;
    dl->dirIllum(iHour)[iWinCover_Bare].sunDisk = ElemLum.sunDisk;
}

Real64 DayltgSkyLuminance(EnergyPlusData const &state,
                          SkyType sky,        // Sky type: 1=clear, 2=clear turbid, 3=intermediate, 4=overcast
                          Real64 const THSKY, // Azimuth and altitude of sky element (radians)
                          Real64 const PHSKY)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   July 1997

    // PURPOSE OF THIS SUBROUTINE:
    // Called by CalcDayltgCoefficients, DayltgExtHorizIllum AND DayltgInterReflectedIllum.  gives
    // luminance in cd/m2 for four different sky types, as described in R.Perez, P.Ineichen,
    // R.Seals, J.Michalsky and R.Stewart, "Modeling daylight availability and irradiance
    // components from direct and global irradiance," Solar Energy 44, 1990, 271-289.
    // The luminance distributions in this routine are normalized such that
    // the zenith luminance is 1.0, i.e., DayltgSkyLuminance =
    // (sky luminance at THSKY, PHSKY)/(zenith luminance), which is dimensionless.
    // The sky types are:
    // 1. Standard CIE clear sky
    // 2. Standard CIE high-turbidity clear sky
    // 3. CIE intermediate sky
    // 4. CIE overcast sky

    // METHODOLOGY EMPLOYED:

    // REFERENCES:
    // Based on DOE-2.1E subroutine DSKYLU, which did only clear and overcast skies.

    // OTHER NOTES:
    // THSKY ranges from 0 to 2Pi starting with 0 directly East and rotating clockwise.
    // PHSKY ranges from 0 to Pi starting with 0 at the horizon and Pi/2 at the zenith.

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    auto const &dl = state.dataDayltg;

    Real64 G = 0.0;    // Angle between sun and element of sky (radians)
    Real64 COSG = 0.0; // Cosine of G

    Real64 SPHSKY = max(std::sin(PHSKY), 0.01); // Prevent floating point underflows
    Real64 Z = Constant::PiOvr2 - dl->sunAngles.phi;
    if (sky != SkyType::Overcast) { // Following not needed for overcast sky
        COSG = SPHSKY * dl->sunAngles.sinPhi + std::cos(PHSKY) * dl->sunAngles.cosPhi * std::cos(THSKY - dl->sunAngles.theta);
        COSG = max(DataPrecisionGlobals::constant_minusone, min(COSG, 1.0)); // Prevent out of range due to roundoff
        G = std::acos(COSG);
    }

    switch (sky) {
    case SkyType::Clear: {
        Real64 Z1 = 0.910 + 10.0 * std::exp(-3.0 * G) + 0.45 * COSG * COSG;
        Real64 Z2 = 1.0 - std::exp(-0.32 / SPHSKY);
        Real64 Z3 = 0.27385 * (0.91 + 10.0 * std::exp(-3.0 * Z) + 0.45 * dl->sunAngles.sinPhi * dl->sunAngles.sinPhi);
        return Z1 * Z2 / Z3;

    } break;
    case SkyType::ClearTurbid: {
        Real64 Z1 = 0.856 + 16.0 * std::exp(-3.0 * G) + 0.3 * COSG * COSG;
        Real64 Z2 = 1.0 - std::exp(-0.32 / SPHSKY);
        Real64 Z3 = 0.27385 * (0.856 + 16.0 * std::exp(-3.0 * Z) + 0.3 * dl->sunAngles.sinPhi * dl->sunAngles.sinPhi);
        return Z1 * Z2 / Z3;

    } break;

    case SkyType::Intermediate: {
        Real64 Z1 = (1.35 * (std::sin(3.59 * PHSKY - 0.009) + 2.31) * std::sin(2.6 * dl->sunAngles.phi + 0.316) + PHSKY + 4.799) / 2.326;
        Real64 Z2 = std::exp(-G * 0.563 * ((dl->sunAngles.phi - 0.008) * (PHSKY + 1.059) + 0.812));
        Real64 Z3 = 0.99224 * std::sin(2.6 * dl->sunAngles.phi + 0.316) + 2.73852;
        Real64 Z4 = std::exp(-Z * 0.563 * ((dl->sunAngles.phi - 0.008) * 2.6298 + 0.812));
        return Z1 * Z2 / (Z3 * Z4);
    } break;
    case SkyType::Overcast: {
        return (1.0 + 2.0 * SPHSKY) / 3.0;
    } break;
    default:
        assert(false);
        return 0.0;
    }
}

Real64 ProfileAngle(EnergyPlusData &state,
                    int const SurfNum,                                     // Surface number
                    Vector3<Real64> const &CosDirSun,                      // Solar direction cosines
                    DataWindowEquivalentLayer::Orientation const HorOrVert // If HORIZONTAL, calculates ProfileAngHor
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   May 2001

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates profile angle for a surface.

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:
    // For HorOrVert = HORIZONTAL,
    //  this is the incidence angle in a plane that is normal to the window
    //  and parallel to the Y-axis of the window (the axis along
    //  which the height of the window is measured).
    //  For HorOrVert = VERTICAL,
    //  this is the incidence angle in a plane that is normal to the window
    //  and parallel to the X-axis of the window (the axis along
    //  which the width of the window is measured).
    // If VERTICAL, calculates ProfileAngVert
    auto &s_surf = state.dataSurface;

    auto const &surf = s_surf->Surface(SurfNum);
    if (HorOrVert == DataWindowEquivalentLayer::Orientation::Horizontal) {  // Profile angle for horizontal structures
        Real64 ElevWin = Constant::PiOvr2 - surf.Tilt * Constant::DegToRad; // Window elevation: angle between outward normal and horizontal (radians)
        Real64 AzimWin = (90.0 - surf.Azimuth) * Constant::DegToRad;        // Window azimuth (radians)
        Real64 ElevSun = std::asin(CosDirSun.z);                            // Sun elevation; angle between sun and horizontal (radians)
        Real64 AzimSun = std::atan2(CosDirSun.y, CosDirSun.x);              // Sun azimuth (radians)
        return std::atan(std::sin(ElevSun) / std::abs(std::cos(ElevSun) * std::cos(AzimWin - AzimSun))) - ElevWin;
    } else { // Profile angle for vertical structures
        Real64 ElevWin = Constant::PiOvr2 - surf.Tilt * Constant::DegToRad;
        Real64 AzimWin = surf.Azimuth * Constant::DegToRad;    // 7952
        Real64 AzimSun = std::atan2(CosDirSun.x, CosDirSun.y); // 7952

        Real64 ProfileAng;
        if (std::abs(ElevWin) < 0.1) {      // Near-vertical window
            ProfileAng = AzimWin - AzimSun; // CR7952 allow sign changes.
        } else {
            Vector3<Real64> WinNorm = surf.OutNormVec; // Window outward normal unit vector
            Real64 ThWin = AzimWin - Constant::PiOvr2;
            Real64 const sin_ElevWin = std::sin(ElevWin);
            // Cross product of WinNorm and vector along window baseline
            Vector3<Real64> WinNormCrossBase = {-sin_ElevWin * std::cos(ThWin), sin_ElevWin * std::sin(ThWin), std::cos(ElevWin)};
            // Projection of sun vector onto plane (perpendicular to window plane) determined
            // by WinNorm and vector along baseline of window
            Vector3<Real64> SunPrime = CosDirSun - WinNormCrossBase * dot(CosDirSun, WinNormCrossBase);
            ProfileAng = std::abs(std::acos(dot(WinNorm, SunPrime) / SunPrime.magnitude()));
            // CR7952 correct sign of result for vertical slats
            if ((AzimWin - AzimSun) < 0.0) {
                ProfileAng = -1.0 * ProfileAng;
            }
        }
        // Constrain to 0 to pi
        if (ProfileAng > Constant::Pi) {
            ProfileAng = 2.0 * Constant::Pi - ProfileAng;
        }
        return ProfileAng;
    }
}

void DayltgClosestObstruction(EnergyPlusData &state,
                              Vector3<Real64> const &RecPt,  // Point on window from which ray emanates (m)
                              Vector3<Real64> const &RayVec, // Unit vector along ray pointing away from window (m)
                              int &NearestHitSurfNum,        // Surface number of nearest obstruction that is hit by ray;
                              Vector3<Real64> &NearestHitPt  // Ray's hit point on nearest obstruction (m)
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   November 2003

    // PURPOSE OF THIS SUBROUTINE:
    // Determines surface number and hit point of closest exterior obstruction hit
    // by a ray from a window. If no obstruction is hit, NearestHitSurfNum = 0.

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:
    //  = 0 if no obstruction is hit.
    auto &s_surf = state.dataSurface;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Vector3<Real64> HitPt; // Hit point on an obstruction (m)
    bool hit;              // True iff obstruction is hit

    NearestHitSurfNum = 0;
    Real64 NearestHitDistance_sq(std::numeric_limits<Real64>::max()); // Distance squared from receiving point to nearest hit point for a ray (m^2)
    NearestHitPt = 0.0;
    if (s_surf->TotSurfaces < octreeCrossover) { // Linear search through surfaces

        for (int ObsSurfNum : s_surf->AllShadowPossObstrSurfaceList) {
            // Determine if this ray hits the surface and, if so, get the distance from the receiving point to the hit
            hit = PierceSurface(state, ObsSurfNum, RecPt, RayVec, HitPt);
            if (!hit) { // Ray pierces surface
                continue;
            }

            // If obstruction is a window and its base surface is the nearest obstruction hit so far set nearestHitSurface to this window
            // Note that in this case NearestHitDistance_sq has already been calculated, so does not have to be recalculated
            if ((s_surf->Surface(ObsSurfNum).Class == SurfaceClass::Window) && (s_surf->Surface(ObsSurfNum).BaseSurf == NearestHitSurfNum)) {
                NearestHitSurfNum = ObsSurfNum;
            } else {
                // Distance squared from receiving point to hit point
                Real64 const HitDistance_sq(distance_squared(HitPt, RecPt));
                // Reset NearestHitSurfNum and NearestHitDistance_sq if this hit point is closer than previous closest
                if (HitDistance_sq < NearestHitDistance_sq) {
                    NearestHitDistance_sq = HitDistance_sq;
                    NearestHitSurfNum = ObsSurfNum;
                    NearestHitPt = HitPt;
                }
            } // End of check if obstruction was hit
        } // for (ObsSurfNum)

    } else { // Surface octree search

        SurfaceData const *nearestHitSurface(nullptr);

        // Lambda function for the octree to test for surface hit
        auto surfaceHit = [&s_surf, &RecPt, &RayVec, &hit, &NearestHitDistance_sq, &nearestHitSurface, &NearestHitPt](SurfaceData const &surface) {
            if (surface.IsShadowPossibleObstruction) {
                Vector3<Real64> HitPt;
                // Determine if this ray hits the surface and, if so, get the distance from the receiving point to the hit
                hit = PierceSurface(surface, RecPt, RayVec, HitPt); // Check if ray pierces surface
                if (!hit) {
                    return;
                }

                // If obstruction is a window and its base surface is the nearest obstruction hit so far set nearestHitSurface to this window
                // Note that in this case NearestHitDistance_sq has already been calculated, so does not have to be recalculated
                if ((surface.Class == SurfaceClass::Window) && (surface.BaseSurf > 0) && (&s_surf->Surface(surface.BaseSurf) == nearestHitSurface)) {
                    nearestHitSurface = &surface;
                } else {
                    // Distance squared from receiving point to hit point
                    Real64 const HitDistance_sq(distance_squared(HitPt, RecPt));
                    // Reset nearestHitSurface and NearestHitDistance_sq if this hit point is closer than previous closest
                    if (HitDistance_sq < NearestHitDistance_sq) {
                        NearestHitDistance_sq = HitDistance_sq;
                        nearestHitSurface = &surface;
                        NearestHitPt = HitPt;
                    }
                } // End of check if obstruction was hit
            }
        };

        // Process octree surface candidates
        Vector3<Real64> const RayVec_inv(SurfaceOctreeCube::safe_inverse(RayVec));
        state.dataHeatBalMgr->surfaceOctree.processSurfaceRayIntersectsCube(RecPt, RayVec, RayVec_inv, surfaceHit);
        if (nearestHitSurface != nullptr) { // Find surface number: This is inefficient: Improve when surfaces know their own number
            for (int i = 1; i <= s_surf->TotSurfaces; ++i) {
                if (&s_surf->Surface(i) == nearestHitSurface) {
                    NearestHitSurfNum = i;
                    break;
                }
            }
            assert(NearestHitSurfNum != 0);
        }
    }
} // DayltgClosestObstruction()

Real64 DayltgSurfaceLumFromSun(EnergyPlusData &state,
                               int const IHR,                   // Hour number
                               Vector3<Real64> const &Ray,      // Ray from window to reflecting surface (m)
                               int const ReflSurfNum,           // Number of surface for which luminance is being calculated
                               Vector3<Real64> const &ReflHitPt // Point on ReflSurfNum for luminance calculation (m)
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   November 2003

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates exterior surface luminance due to beam solar diffuse reflection.

    // Locals
    // SUBROUTINE ARGUMENT DEFINITIONS:
    //  beam normal illuminance (cd/m2)

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Vector3<Real64> SurfaceLumFromSunReflNorm; // Unit normal to reflecting surface (m)
    Vector3<Real64> SurfaceLumFromSunObsHitPt; // Hit point on obstruction (m)
    bool hitObs;                               // True iff obstruction is hit
    Real64 DiffVisRefl;                        // Diffuse visible reflectance of ReflSurfNum

    auto &s_surf = state.dataSurface;
    // Skip daylighting shelves since reflection from these is separately calculated
    if (s_surf->SurfDaylightingShelfInd(ReflSurfNum) > 0) {
        return 0.0;
    }

    auto const &reflSurf = s_surf->Surface(ReflSurfNum);

    // Normal to reflecting surface in hemisphere containing window element
    SurfaceLumFromSunReflNorm = reflSurf.OutNormVec;
    if (reflSurf.IsShadowing) {
        if (dot(SurfaceLumFromSunReflNorm, Ray) > 0.0) {
            SurfaceLumFromSunReflNorm *= -1.0;
        }
    }
    // Cosine of angle of incidence of sun at HitPt if sun were to reach HitPt
    Vector3<Real64> const SUNCOS_IHR = s_surf->SurfSunCosHourly(IHR);
    Real64 CosIncAngAtHitPt = dot(SurfaceLumFromSunReflNorm, SUNCOS_IHR);
    // Require that the sun be in front of this surface relative to window element
    if (CosIncAngAtHitPt <= 0.0) {
        return 0.0; // Sun is in back of reflecting surface
    }
    // Sun reaches ReflHitPt if vector from ReflHitPt to sun is unobstructed
    hitObs = false;
    for (int ObsSurfNum : s_surf->AllShadowPossObstrSurfaceList) {
        // Exclude as a possible obstructor ReflSurfNum and its base surface (if it has one)
        if (ObsSurfNum == ReflSurfNum || ObsSurfNum == reflSurf.BaseSurf) {
            continue;
        }
        hitObs = PierceSurface(state, ObsSurfNum, ReflHitPt, SUNCOS_IHR, SurfaceLumFromSunObsHitPt);
        if (hitObs) {
            break;
        }
    }

    if (hitObs) {
        return 0.0; // Obstruction was hit, blocking s auto surfaceHit = [&state, &GroundHitPtun
    }
    // Obstruction was not hit; sun reaches ReflHitPt.
    // Calculate luminance at ReflHitPt due to beam solar reflection (for unit beam normal illuminance)
    if (reflSurf.IsShadowing) {
        DiffVisRefl = s_surf->SurfShadowDiffuseVisRefl(ReflSurfNum);
        // Note that if the shadowing surface has a non-zero glazing fraction (e.g., neighboring bldg) that the above is
        // (1 - glazing fraction) * (vis refl of opaque part of shadowing surface); specular reflection is
        // excluded in this value of DiffVisRefl.
    } else { // Exterior building surface
        if (!state.dataConstruction->Construct(reflSurf.Construction).TypeIsWindow) {
            DiffVisRefl = 1.0 - state.dataConstruction->Construct(reflSurf.Construction).OutsideAbsorpSolar;
        } else {
            // Window; assume bare so no beam-to-diffuse reflection
            DiffVisRefl = 0.0;
        }
    }
    return CosIncAngAtHitPt * DiffVisRefl / Constant::Pi;
}

void DayltgInteriorMapIllum(EnergyPlusData &state)
{

    // *****super modified version of DayltgInteriorIllum by Peter Graham Ellis
    // *****removes all control code, just calculates illum with previously determined control settings
    // *****this should be packaged into a subroutine called from 2 places

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   July 1997
    //       MODIFIED       March 2000, FW: interpolate clear-sky daylight factors using
    //                      HourOfDay/WeightNow and NextHour/WeightNextHour. Previously
    //                      only HourOfDay was used
    //                      Jan 2001, FW: interpolate in slat angle for windows with blinds
    //                      that have movable slats
    //                      Dec 2003, FW: fix bug--even though between-glass shade/blind is on
    //                        daylight illum at ref pt was calculated as though it was off
    //                      June 2009, TH: modified for thermochromic windows
    //                      March 2010, TH: fix bug (CR 8057) for electrochromic windows
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Using daylighting factors and exterior illuminance, determine
    // the current-hour interior daylight illuminance and glare index
    // at each reference point in a space.

    // Called by InitSurfaceHeatBalance.

    // REFERENCES:
    // Based on DOE-2.1E subroutine DINTIL.
    auto &dl = state.dataDayltg;

    // Locals
    Array1D<Real64> daylight_illum;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int iSky1; // Sky type index values for averaging two sky types
    int iSky2;
    Real64 SkyWeight;    // Weighting factor used to average two different sky types
    Real64 HorIllSkyFac; // Ratio between horizontal illuminance from sky horizontal irradiance and
    //   luminous efficacy and horizontal illuminance from averaged sky

    if (state.dataGlobal->WarmupFlag) {
        return;
    }

    auto &s_surf = state.dataSurface;

    daylight_illum.allocate(MaxMapRefPoints);

    //              Initialize reference point illuminance and window background luminance

    for (auto &thisMap : dl->illumMaps) {
        int enclNum = thisMap.enclIndex;
        auto &thisEnclDaylight = dl->enclDaylight(enclNum);

        int NREFPT = thisMap.TotalMapRefPoints; // Number of daylighting map reference points

        daylight_illum = 0.0;

        if (state.dataEnvrn->SkyClearness > 3.0) { // Sky is average of clear and clear turbid
            SkyWeight = min(1.0, (state.dataEnvrn->SkyClearness - 3.0) / 3.0);
            iSky1 = (int)SkyType::Clear;
            iSky2 = (int)SkyType::ClearTurbid;
        } else if (state.dataEnvrn->SkyClearness > 1.2) { // Sky is average of clear turbid and intermediate
            SkyWeight = (state.dataEnvrn->SkyClearness - 1.2) / 1.8;
            iSky1 = (int)SkyType::ClearTurbid;
            iSky2 = (int)SkyType::Intermediate;
        } else { // Sky is average of intermediate and overcast
            SkyWeight = min(1.0, max(0.0, (state.dataEnvrn->SkyClearness - 1.0) / 0.2, (state.dataEnvrn->SkyBrightness - 0.05) / 0.4));
            iSky1 = (int)SkyType::Intermediate;
            iSky2 = (int)SkyType::Overcast;
        }

        //              First loop over windows in this space.
        //              Find contribution of each window to the daylight illum
        //              and to the glare numerator at each reference point.
        //              Use shading flags set in WindowShadingManager.

        auto &daylFacHrCurr = thisMap.daylFac[state.dataGlobal->HourOfDay];
        auto &daylFacHrPrev = thisMap.daylFac[state.dataGlobal->PreviousHour];

        for (int loop = 1; loop <= thisEnclDaylight.NumOfDayltgExtWins; ++loop) {
            int IWin = thisEnclDaylight.DayltgExtWinSurfNums(loop);

            // Added TH 6/29/2009 for thermochromic windows
            Real64 VTRatio = 1.0;
            if (NREFPT > 0) {
                int IConst = s_surf->Surface(IWin).Construction;
                auto const &construction = state.dataConstruction->Construct(IConst);
                if (construction.isTCWindow) {
                    // For thermochromic windows, daylight and glare factors are always calculated
                    //  based on the master construction. They need to be adjusted by the VTRatio, including:
                    //  ZoneDaylight()%DaylIllFacSky, DaylIllFacSun, DaylIllFacSunDisk; DaylBackFacSky,
                    //  DaylBackFacSun, DaylBackFacSunDisk, DaylSourceFacSky, DaylSourceFacSun, DaylSourceFacSunDisk
                    Real64 VTNow = Window::POLYF(1.0, construction.TransVisBeamCoef);
                    Real64 VTMaster = Window::POLYF(1.0, state.dataConstruction->Construct(construction.TCMasterConstrNum).TransVisBeamCoef);
                    VTRatio = VTNow / VTMaster;
                }
            }

            Real64 wgtThisHr = state.dataGlobal->WeightNow;
            Real64 wgtPrevHr = state.dataGlobal->WeightPreviousHour;

            std::array<Dayltg::Illums, (int)DataSurfaces::WinCover::Num> DFHR; // Sky daylight factor for sky type, bare/shaded window

            auto &dfhr = DFHR[iWinCover_Bare];
            auto &dfhrSh = DFHR[iWinCover_Shaded];

            auto &surfShade = s_surf->surfShades(IWin);
            //              Loop over reference points
            for (int ILB = 1; ILB <= NREFPT; ++ILB) {
                // if (ILB != 5) continue;
                auto const &illSkyCurr = daylFacHrCurr(loop, ILB)[iWinCover_Bare];
                auto const &illSkyPrev = daylFacHrPrev(loop, ILB)[iWinCover_Bare];
                auto const &illShSkyCurr = daylFacHrCurr(loop, ILB)[iWinCover_Shaded];
                auto const &illShSkyPrev = daylFacHrPrev(loop, ILB)[iWinCover_Shaded];

                //          Daylight factors for current sun position
                for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
                    //                                ===Bare window===
                    dfhr.sky[iSky] = VTRatio * (wgtThisHr * illSkyCurr.sky[iSky] + wgtPrevHr * illSkyPrev.sky[iSky]);

                    if ((s_surf->SurfWinWindowModelType(IWin) != WindowModel::BSDF) &&
                        (IS_SHADED(s_surf->SurfWinShadingFlag(IWin)) || s_surf->SurfWinSolarDiffusing(IWin))) {

                        //                                 ===Shaded window===
                        // Shade, screen, blind with fixed slats, or diffusing glass
                        dfhrSh.sky[iSky] = VTRatio * (wgtThisHr * illShSkyCurr.sky[iSky] + wgtPrevHr * illShSkyPrev.sky[iSky]);
                    } // End of check if window is shaded or has diffusing glass
                } // for (iSky)

                // Sun daylight factor for bare/shaded window
                std::array<Illums, (int)DataSurfaces::WinCover::Num> tmpDFHR;
                tmpDFHR[iWinCover_Bare].sun =
                    VTRatio * (wgtThisHr * (illSkyCurr.sun + illSkyCurr.sunDisk) + wgtPrevHr * (illSkyPrev.sun + illSkyPrev.sunDisk));

                if ((s_surf->SurfWinWindowModelType(IWin) != WindowModel::BSDF) &&
                    (IS_SHADED(s_surf->SurfWinShadingFlag(IWin)) || s_surf->SurfWinSolarDiffusing(IWin))) {

                    //                                 ===Shaded window===
                    // Shade, screen, blind with fixed slats, or diffusing glass
                    tmpDFHR[iWinCover_Shaded].sun = VTRatio * (wgtThisHr * illShSkyCurr.sun + wgtPrevHr * illShSkyPrev.sun);

                    if (!surfShade.blind.slatBlockBeam) {
                        tmpDFHR[iWinCover_Shaded].sun += VTRatio * (wgtThisHr * illShSkyCurr.sunDisk + wgtPrevHr * illShSkyPrev.sunDisk);
                    }
                } // End of check if window is shaded or has diffusing glass

                //              Get illuminance at ref point from bare and shaded window by
                //              multiplying daylight factors by exterior horizontal illuminance

                // Adding 0.001 in the following prevents zero DayltgInteriorMapIllumHorIllSky in early morning or late evening when sun
                // is up in the present time step but GILSK(ISky,HourOfDay) and GILSK(ISky,NextHour) are both zero.
                Illums tmpHorIll; // Horizontal illuminance for different sky types
                auto const &gilCurr = dl->horIllum[state.dataGlobal->HourOfDay];
                auto const &gilPrev = dl->horIllum[state.dataGlobal->PreviousHour];
                for (int iSky = (int)SkyType::Clear; iSky < (int)SkyType::Num; ++iSky) {
                    tmpHorIll.sky[iSky] = wgtThisHr * gilCurr.sky[iSky] + wgtPrevHr * gilPrev.sky[iSky] + 0.001;
                }

                // HISKF is current time step horizontal illuminance from sky, calculated in DayltgLuminousEfficacy,
                // which is called in WeatherManager. HISUNF is current time step horizontal illuminance from sun,
                // also calculated in DayltgLuminousEfficacy.
                HorIllSkyFac = state.dataEnvrn->HISKF / ((1.0 - SkyWeight) * tmpHorIll.sky[iSky2] + SkyWeight * tmpHorIll.sky[iSky1]);

                for (int iWinCover = 0; iWinCover < (int)WinCover::Num; ++iWinCover) {
                    if (iWinCover == iWinCover_Shaded) {
                        if (s_surf->SurfWinWindowModelType(IWin) == WindowModel::BSDF) {
                            break;
                        }
                        if (NOT_SHADED(s_surf->SurfWinShadingFlag(IWin)) && !s_surf->SurfWinSolarDiffusing(IWin)) {
                            break;
                        }
                    }
                    auto const &dfhr3 = DFHR[iWinCover];

                    thisMap.refPts(ILB).winLums(loop)[iWinCover] = tmpDFHR[iWinCover].sun * state.dataEnvrn->HISUNF +
                                                                   HorIllSkyFac * (dfhr3.sky[iSky1] * SkyWeight * tmpHorIll.sky[iSky1] +
                                                                                   dfhr3.sky[iSky2] * (1.0 - SkyWeight) * tmpHorIll.sky[iSky2]);
                }

            } // End of reference point loop
        } // End of first loop over windows

        //              Second loop over windows. Find total daylight illuminance
        //              and background luminance for each ref pt from all windows in
        //              the space.  Use shading flags.

        for (int loop = 1; loop <= thisEnclDaylight.NumOfDayltgExtWins; ++loop) {
            int IWin = thisEnclDaylight.DayltgExtWinSurfNums(loop);
            auto const &surfWin = s_surf->SurfaceWindow(IWin);

            WinCover winCover = findWinShadingStatus(state, IWin);

            // CR 8057. 3/17/2010.
            // Switchable windows may be in partially switched state rather than fully dark state
            Real64 VTMULT = 1.0;

            int ICtrl = s_surf->Surface(IWin).activeWindowShadingControl;
            if (s_surf->Surface(IWin).HasShadeControl) {
                if (s_surf->WindowShadingControl(ICtrl).shadingControlType == WindowShadingControlType::MeetDaylIlumSetp &&
                    s_surf->SurfWinShadingFlag(IWin) == WinShadingType::SwitchableGlazing) {
                    // switchable windows in partial or fully switched state,
                    //  get its intermediate VT calculated in DayltgInteriorIllum
                    int IConstShaded = s_surf->Surface(IWin).activeShadedConstruction;
                    if (IConstShaded > 0) {
                        // Visible transmittance (VT) of electrochromic (EC) windows in fully dark state
                        Real64 VTDark = Window::POLYF(1.0, state.dataConstruction->Construct(IConstShaded).TransVisBeamCoef) * surfWin.glazedFrac;
                        if (VTDark > 0) {
                            VTMULT = s_surf->SurfWinVisTransSelected(IWin) / VTDark;
                        }
                    }
                }
            }

            for (int IL = 1; IL <= NREFPT; ++IL) {
                //              Determine if illuminance contribution is from bare or shaded window
                daylight_illum(IL) += VTMULT * thisMap.refPts(IL).winLums(loop)[(int)winCover];
            }
        } // End of second window loop

        //              Variables for reporting
        for (int IL = 1; IL <= NREFPT; ++IL) {
            thisMap.refPts(IL).lums[iLum_Illum] = max(daylight_illum(IL), 0.0);
        }
    } // End loop over maps
} // DayltgInteriorMapIllum()

void ReportIllumMap(EnergyPlusData &state, int const MapNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Ellis
    //       DATE WRITTEN   May 2003

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine produces the Daylighting Illuminance Map output.  Each separate map (by zone)
    // is placed on a temporary file and later (see CloseReportIllumMaps) coallesced into a single
    // output file.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    std::string MapNoString;
    auto &dl = state.dataDayltg;

    if (dl->ReportIllumMap_firstTime) {
        dl->ReportIllumMap_firstTime = false;
        dl->FirstTimeMaps.dimension((int)dl->illumMaps.size(), true);
        dl->EnvrnPrint.dimension((int)dl->illumMaps.size(), true);
        dl->SavedMnDy.allocate((int)dl->illumMaps.size());
    }

    auto &illumMap = dl->illumMaps(MapNum);

    if (dl->FirstTimeMaps(MapNum)) {

        dl->FirstTimeMaps(MapNum) = false;

        auto openMapFile = [&](const fs::path &filePath) -> InputOutputFile & {
            auto &outputFile = *illumMap.mapFile;
            outputFile.filePath = FileSystem::appendSuffixToPath(filePath, fmt::to_string(MapNum));
            outputFile.ensure_open(state, "ReportIllumMap");
            return outputFile;
        };
        if (dl->MapColSep == DataStringGlobals::CharTab) {
            if (!openMapFile(state.files.outputMapTabFilePath).good()) {
                return;
            }
            //                CommaDelimited = false; //Unused Set but never used
        } else if (dl->MapColSep == DataStringGlobals::CharComma) {
            if (!openMapFile(state.files.outputMapCsvFilePath).good()) {
                return;
            }
            //                CommaDelimited = true; //Unused Set but never used
        } else {
            if (!openMapFile(state.files.outputMapTxtFilePath).good()) {
                return;
            }
            //                CommaDelimited = false; //Unused Set but never used
        }

        dl->SavedMnDy(MapNum) = state.dataEnvrn->CurMnDyHr.substr(0, 5);

        illumMap.Name = format("{} at {:.2R}m", illumMap.Name, illumMap.Z);
    }
    if (dl->SavedMnDy(MapNum) != state.dataEnvrn->CurMnDyHr.substr(0, 5)) {
        dl->EnvrnPrint(MapNum) = true;
        dl->SavedMnDy(MapNum) = state.dataEnvrn->CurMnDyHr.substr(0, 5);
    }

    illumMap.pointsHeader = "";
    int rCount = 0;
    for (auto &thisDayltgCtrl : dl->daylightControl) {
        if (thisDayltgCtrl.zoneIndex != illumMap.zoneIndex) {
            continue;
        }

        for (int R = 1; R <= thisDayltgCtrl.TotalDaylRefPoints; ++R) {
            ++rCount;
            auto const &refPt = thisDayltgCtrl.refPts(R);
            illumMap.pointsHeader += format(" RefPt{}=({:.2R}:{:.2R}:{:.2R}),", rCount, refPt.absCoords.x, refPt.absCoords.y, refPt.absCoords.z);
        }
    }

    if (rCount > 0) {
        // Remove trailing comma
        illumMap.pointsHeader.pop_back();
    }
    if (dl->EnvrnPrint(MapNum)) {
        WriteDaylightMapTitle(
            state, MapNum, *illumMap.mapFile, illumMap.Name, state.dataEnvrn->EnvironmentName, illumMap.zoneIndex, illumMap.pointsHeader, illumMap.Z);
        dl->EnvrnPrint(MapNum) = false;
    }

    if (!state.dataGlobal->WarmupFlag) {
        if (state.dataGlobal->TimeStep == state.dataGlobal->TimeStepsInHour) { // Report only hourly

            int linelen = 0;
            // Write X scale column header
            std::string mapLine = format(" {} {:02}:00", dl->SavedMnDy(MapNum), state.dataGlobal->HourOfDay);
            if (illumMap.HeaderXLineLengthNeeded) {
                linelen = int(len(mapLine));
            }
            int RefPt = 1;
            for (int X = 1; X <= illumMap.Xnum; ++X) {
                const std::string AddXorYString =
                    format("{}({:.2R};{:.2R})=", dl->MapColSep, illumMap.refPts(RefPt).absCoords.x, illumMap.refPts(RefPt).absCoords.y);
                if (illumMap.HeaderXLineLengthNeeded) {
                    linelen += int(len(AddXorYString));
                }
                mapLine += AddXorYString;
                ++RefPt;
            } // X

            if (illumMap.HeaderXLineLengthNeeded) {
                illumMap.HeaderXLineLength = linelen;
                if (static_cast<std::string::size_type>(illumMap.HeaderXLineLength) > len(mapLine)) {
                    ShowWarningError(state,
                                     format("ReportIllumMap: Map=\"{}\" -- the X Header overflows buffer -- will be truncated at {} characters.",
                                            illumMap.Name,
                                            int(len(mapLine))));
                    ShowContinueError(state, format("...needed {} characters. Please contact EnergyPlus support.", illumMap.HeaderXLineLength));
                }
                illumMap.HeaderXLineLengthNeeded = false;
            }

            print(*illumMap.mapFile, "{}\n", mapLine);

            // Write Y scale prefix and illuminance values
            RefPt = 1;
            for (int Y = 1; Y <= illumMap.Ynum; ++Y) {
                mapLine = format("({:.2R};{:.2R})=", illumMap.refPts(RefPt).absCoords.x, illumMap.refPts(RefPt).absCoords.y);
                for (int R = RefPt; R <= RefPt + illumMap.Xnum - 1; ++R) {
                    int IllumOut = nint(illumMap.refPts(R).lumsHr[iLum_Illum]);
                    std::string String = fmt::to_string(IllumOut);
                    ;
                    if (!illumMap.refPts(R).inBounds) {
                        String = "*" + String;
                    }
                    mapLine += dl->MapColSep + String;
                }

                print(*illumMap.mapFile, "{}\n", mapLine);

                RefPt += illumMap.Xnum;
            } // X

            if (state.dataSQLiteProcedures->sqlite) {
                if (dl->SQFirstTime) {
                    int const nX(maxval(dl->illumMaps, &IllumMap::Xnum));
                    int const nY(maxval(dl->illumMaps, &IllumMap::Ynum));
                    dl->XValue.allocate(nX);
                    dl->YValue.allocate(nY);
                    dl->IllumValue.allocate(nX, nY);
                    dl->SQFirstTime = false;
                }

                for (int Y = 1; Y <= illumMap.Ynum; ++Y) {
                    dl->YValue(Y) = illumMap.Ymin + (Y - 1) * illumMap.Yinc;
                    for (int X = 1; X <= illumMap.Xnum; ++X) {
                        dl->XValue(X) = illumMap.Xmin + (X - 1) * illumMap.Xinc;
                        int IllumIndex = X + (Y - 1) * illumMap.Xnum;
                        dl->IllumValue(X, Y) = nint(illumMap.refPts(IllumIndex).lumsHr[iLum_Illum]);
                        if (!illumMap.refPts(IllumIndex).inBounds) {
                            dl->IllumValue(X, Y) = -dl->IllumValue(X, Y);
                        }
                    } // X Loop
                } // Y Loop

                // We need DataGlobals::CalendarYear, and not DataEnvironment::Year because
                // otherwise if you run a TMY file, you'll get for eg 1977, 1981, etc
                state.dataSQLiteProcedures->sqlite->createSQLiteDaylightMap(MapNum,
                                                                            state.dataGlobal->CalendarYear,
                                                                            state.dataEnvrn->Month,
                                                                            state.dataEnvrn->DayOfMonth,
                                                                            state.dataGlobal->HourOfDay,
                                                                            illumMap.Xnum,
                                                                            dl->XValue,
                                                                            illumMap.Ynum,
                                                                            dl->YValue,
                                                                            dl->IllumValue);

            } // WriteOutputToSQLite
        } // end time step
    } // not Warmup
}

void CloseReportIllumMaps(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   June 2003

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine "closes" out the created daylight illuminance maps by merging them
    // into the "eplusout.map" file.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    auto &dl = state.dataDayltg;

    if ((int)dl->illumMaps.size() > 0) {
        // Write map header
        if (dl->MapColSep == DataStringGlobals::CharTab) {
            state.files.map.filePath = state.files.outputMapTabFilePath;
        } else if (dl->MapColSep == DataStringGlobals::CharComma) {
            state.files.map.filePath = state.files.outputMapCsvFilePath;
        } else {
            state.files.map.filePath = state.files.outputMapTxtFilePath;
        }

        state.files.map.ensure_open(state, "CloseReportIllumMaps");

        for (int MapNum = 1; MapNum <= (int)dl->illumMaps.size(); ++MapNum) {
            auto &illumMap = dl->illumMaps(MapNum);
            if (!illumMap.mapFile->good()) {
                continue; // fatal error processing
            }

            const std::vector<std::string> mapLines = illumMap.mapFile->getLines();
            if (mapLines.empty()) {
                ShowSevereError(state, format("CloseReportIllumMaps: IllumMap=\"{}\" is empty.", illumMap.Name));
                break;
            }
            for (const std::string &mapLine : mapLines) {
                print(state.files.map, "{}\n", mapLine);
            }
            illumMap.mapFile->del();
        }

        if (!dl->mapResultsReported && !state.dataErrTracking->AbortProcessing) {
            const std::string message = "CloseReportIllumMaps: Illuminance maps requested but no data ever reported. Likely cause is no solar.";
            ShowSevereError(state, message);
            print(state.files.map, "{}\n", message);
        }
    }
}

void CloseDFSFile(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   August 2010

    // PURPOSE OF THIS SUBROUTINE:
    // Make sure DFSFile is closed at exit time.  Do not rely on operating system to
    // take care of it.

    state.files.dfs.close();
}

void DayltgSetupAdjZoneListsAndPointers(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   Feb. 2004
    //       MODIFIED:      June 2010;LKL - Merged two routines.

    // PURPOSE OF THIS SUBROUTINE:
    // For each Daylighting:Detailed enclosure, creates a list of adjacent enclosures,
    // that have one or more exterior windows and that share one or more interior
    // windows with Z. Used in calculation of daylighting through interior windows.

    // Sets the daylighting factor pointers for each Daylighting:Detailed control. The pointer
    // may be associated with an exterior window in a daylit target zone's enclosure or an exterior window in
    // an adjacent enclosure, daylit or not, that shares interior windows with the target zone's enclosure.

    // Count number of exterior Windows (use to allocate arrays)
    auto &dl = state.dataDayltg;
    auto &s_surf = state.dataSurface;

    for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
        auto &thisEnclDaylight = dl->enclDaylight(enclNum);
        thisEnclDaylight.TotalExtWindows = 0;

        // Count exterior windows in this solar enclosure
        for (int const surfNum : state.dataViewFactor->EnclSolInfo(enclNum).SurfacePtr) {
            auto const &surf = s_surf->Surface(surfNum);
            if ((surf.Class == SurfaceClass::Window && surf.ExtBoundCond == ExternalEnvironment) ||
                surf.OriginalClass == SurfaceClass::TDD_Diffuser) {
                ++thisEnclDaylight.TotalExtWindows;
            }
        }
    } // End of primary enclosure loop

    for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
        int NumList = 0;
        if (state.dataViewFactor->EnclSolInfo(enclNum).TotalEnclosureDaylRefPoints == 0) {
            continue;
        }
        auto &thisEnclDaylight = dl->enclDaylight(enclNum);
        if (!thisEnclDaylight.hasSplitFluxDaylighting) {
            continue;
        }
        // This is a Daylighting:Detailed enclosure
        // Find adjacent zones/enclosures
        for (int adjEnclNum = 1; adjEnclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++adjEnclNum) {
            if (adjEnclNum == enclNum) {
                continue;
            }
            // Require that adjEnclNum have a least one exterior window
            bool AdjEnclHasExtWins = false;
            for (int SurfNumAdj : state.dataViewFactor->EnclSolInfo(adjEnclNum).SurfacePtr) {
                if ((s_surf->Surface(SurfNumAdj).Class == SurfaceClass::Window) &&
                    (s_surf->Surface(SurfNumAdj).ExtBoundCond == ExternalEnvironment)) {
                    AdjEnclHasExtWins = true;
                    break;
                }
            }
            if (!AdjEnclHasExtWins) {
                continue;
            }
            // Loop again through surfaces in ZoneNumAdj and see if any are interior windows adjacent to ZoneNum
            for (int SurfNumAdj : state.dataViewFactor->EnclSolInfo(adjEnclNum).SurfacePtr) {
                auto const &surfAdj = s_surf->Surface(SurfNumAdj);
                if ((surfAdj.Class == SurfaceClass::Window) && (surfAdj.ExtBoundCond >= 1)) {
                    // This is an interior window in ZoneNumAdj
                    if (s_surf->Surface(surfAdj.ExtBoundCond).SolarEnclIndex == enclNum) {
                        // This interior window is adjacent to ZoneNum
                        ++NumList;
                        break;
                    }
                }
            }
        }
        thisEnclDaylight.AdjIntWinEnclNums.allocate(NumList);
        thisEnclDaylight.AdjIntWinEnclNums = 0;
    } // End of primary enclosure loop

    for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
        int NumList = 0;
        if (state.dataViewFactor->EnclSolInfo(enclNum).TotalEnclosureDaylRefPoints == 0) {
            continue;
        }
        auto &thisEnclDaylight = dl->enclDaylight(enclNum);
        if (!thisEnclDaylight.hasSplitFluxDaylighting) {
            continue;
        }
        // This is a Daylighting:Detailed enclosure
        // Find adjacent zones/enclosures
        for (int adjEnclNum = 1; adjEnclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++adjEnclNum) {
            if (adjEnclNum == enclNum) {
                continue;
            }
            // Require that adjEnclNum have a least one exterior window
            bool AdjEnclHasExtWins = false;
            for (int SurfNumAdj : state.dataViewFactor->EnclSolInfo(adjEnclNum).SurfacePtr) {
                auto const &surfAdj = s_surf->Surface(SurfNumAdj);
                if (surfAdj.Class == SurfaceClass::Window && surfAdj.ExtBoundCond == ExternalEnvironment) {
                    AdjEnclHasExtWins = true;
                    break;
                }
            }
            if (!AdjEnclHasExtWins) {
                continue;
            }
            // Loop again through surfaces in ZoneNumAdj and see if any are interior windows adjacent to enclNum
            for (int SurfNumAdj : state.dataViewFactor->EnclSolInfo(adjEnclNum).SurfacePtr) {
                auto const &surfAdj = s_surf->Surface(SurfNumAdj);
                if (surfAdj.Class != SurfaceClass::Window || surfAdj.ExtBoundCond < 1) {
                    continue;
                }

                // This is an interior window in adjEnclNum
                if (s_surf->Surface(surfAdj.ExtBoundCond).SolarEnclIndex != enclNum) {
                    continue;
                }

                // This interior window is adjacent to ZoneNum
                ++NumList;
                int enclNumAdj = surfAdj.SolarEnclIndex;
                thisEnclDaylight.AdjIntWinEnclNums(NumList) = enclNumAdj;
                dl->enclDaylight(enclNumAdj).adjEnclHasDayltgCtrl = true;
                break;
            }
        }
        thisEnclDaylight.NumOfIntWinAdjEncls = NumList;
    } // End of primary enclosure loop

    // now fill out information on relationship between adjacent exterior windows and associated interior windows
    for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
        auto &enclDayl = dl->enclDaylight(enclNum);
        // first find count of exterior windows
        if (enclDayl.NumOfIntWinAdjEncls <= 0) {
            enclDayl.NumOfIntWinAdjEnclExtWins = 0;
            continue;
        }
        for (int adjEnclNum : enclDayl.AdjIntWinEnclNums) {
            for (int SurfNumAdj : state.dataViewFactor->EnclSolInfo(adjEnclNum).SurfacePtr) {
                if ((s_surf->Surface(SurfNumAdj).Class == SurfaceClass::Window) &&
                    (s_surf->Surface(SurfNumAdj).ExtBoundCond == ExternalEnvironment)) {
                    ++enclDayl.NumOfIntWinAdjEnclExtWins;
                }
            }
        }
        // now allocate nested struct based on exterior window count
        enclDayl.IntWinAdjEnclExtWin.allocate(enclDayl.NumOfIntWinAdjEnclExtWins);

        // now fill nested structure
        int ExtWinIndex = 0;
        for (int adjEnclNum : enclDayl.AdjIntWinEnclNums) {
            for (int SurfNumAdj : state.dataViewFactor->EnclSolInfo(adjEnclNum).SurfacePtr) {
                auto const &surfAdj = s_surf->Surface(SurfNumAdj);
                if (surfAdj.Class != SurfaceClass::Window || surfAdj.ExtBoundCond != ExternalEnvironment) {
                    continue;
                }

                ++ExtWinIndex;
                auto &intWinAdjEnclExtWin = enclDayl.IntWinAdjEnclExtWin(ExtWinIndex);
                intWinAdjEnclExtWin.SurfNum = SurfNumAdj;

                // now count interior windows shared by both zones
                int NumOfIntWindowsCount = 0;
                for (int SurfNumAdj2 : state.dataViewFactor->EnclSolInfo(adjEnclNum).SurfacePtr) {
                    auto const &surfAdj2 = s_surf->Surface(SurfNumAdj2);
                    if ((surfAdj2.Class == SurfaceClass::Window) && (surfAdj2.ExtBoundCond >= 1)) {
                        // This is an interior window in ZoneNumAdj
                        if (s_surf->Surface(surfAdj2.ExtBoundCond).SolarEnclIndex == enclNum) {
                            // This interior window is adjacent to ZoneNum and associated with this
                            ++NumOfIntWindowsCount;
                        }
                    }
                } // for (SurfNumAdj2)

                // allocate nested array
                intWinAdjEnclExtWin.IntWinNum.allocate(NumOfIntWindowsCount);
                intWinAdjEnclExtWin.IntWinNum = 0;
                int IntWinIndex = 0;
                for (int SurfNumAdj2 : state.dataViewFactor->EnclSolInfo(adjEnclNum).SurfacePtr) {
                    auto const &surfAdj2 = s_surf->Surface(SurfNumAdj2);
                    if (surfAdj2.Class != SurfaceClass::Window || surfAdj2.ExtBoundCond < 1) {
                        continue;
                    }

                    // This is an interior window in ZoneNumAdj
                    if (s_surf->Surface(surfAdj2.ExtBoundCond).SolarEnclIndex == enclNum) {
                        // This interior window is adjacent to ZoneNum and associated with this
                        intWinAdjEnclExtWin.IntWinNum(++IntWinIndex) = SurfNumAdj2;
                    }
                } // for (SurfNumAdj2)
            } // for (SurfNumAdj)
        } // for (adjEnclNum)
    } // End of primary enclosure loop

    Array1D_int enclExtWin;
    enclExtWin.dimension(state.dataViewFactor->NumOfSolarEnclosures, 0);

    for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
        enclExtWin(enclNum) = 0;
        if (state.dataViewFactor->EnclSolInfo(enclNum).TotalEnclosureDaylRefPoints == 0) {
            continue;
        }
        auto const &thisEnclDaylight = dl->enclDaylight(enclNum);
        if (!thisEnclDaylight.hasSplitFluxDaylighting) {
            continue;
        }
        // This is a Daylighting:Detailed zone

        // Get exterior windows in this solar enclosure
        for (int const surfNum : state.dataViewFactor->EnclSolInfo(enclNum).SurfacePtr) {
            auto const &surf = s_surf->Surface(surfNum);
            if ((surf.Class == SurfaceClass::Window && surf.ExtBoundCond == ExternalEnvironment) ||
                surf.OriginalClass == SurfaceClass::TDD_Diffuser) {
                ++enclExtWin(enclNum);
            }
        }

        // Get exterior windows in adjacent enclosures that share interior windows with enclNum
        if (thisEnclDaylight.NumOfIntWinAdjEncls == 0) {
            continue;
        }

        for (int adjEnclNum : thisEnclDaylight.AdjIntWinEnclNums) {
            // Get exterior windows in EnclNumAdj -- there must be at least one, otherwise
            // it would not be an "AdjIntWinEncl"
            for (int SurfNumAdj : state.dataViewFactor->EnclSolInfo(adjEnclNum).SurfacePtr) {
                auto const &surfAdj = s_surf->Surface(SurfNumAdj);
                if ((surfAdj.Class == SurfaceClass::Window && surfAdj.ExtBoundCond == ExternalEnvironment) ||
                    surfAdj.OriginalClass == SurfaceClass::TDD_Diffuser) {
                    ++enclExtWin(enclNum);
                }
            }
        } // for (adjEnclNum)
    } // for (enclNum)

    dl->maxShadeDeployOrderExtWins = 0;
    for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
        auto &thisEnclDaylight = dl->enclDaylight(enclNum);
        if (!thisEnclDaylight.hasSplitFluxDaylighting) {
            continue;
        }
        thisEnclDaylight.NumOfDayltgExtWins = 0;
        int thisEnclNumRefPoints = state.dataViewFactor->EnclSolInfo(enclNum).TotalEnclosureDaylRefPoints;
        if (thisEnclNumRefPoints > 0) {
            // This is a Daylighting:Detailed enclosure

            // Get exterior windows in this enclosure
            if (enclExtWin(enclNum) == 0) {
                continue;
            }
            thisEnclDaylight.DayltgExtWinSurfNums.allocate(enclExtWin(enclNum));
            thisEnclDaylight.DayltgExtWinSurfNums = 0;
            for (int controlNum : thisEnclDaylight.daylightControlIndexes) {
                auto &thisDayltgCtrl = dl->daylightControl(controlNum);
                thisDayltgCtrl.MapShdOrdToLoopNum.allocate(enclExtWin(enclNum));
                thisDayltgCtrl.MapShdOrdToLoopNum = 0;

                assert((int)thisDayltgCtrl.refPts.size() == thisDayltgCtrl.TotalDaylRefPoints);
                for (auto &refPt : thisDayltgCtrl.refPts) {
                    refPt.extWins.allocate(enclExtWin(enclNum));
                    for (auto &extWin : refPt.extWins) {
                        new (&extWin) DaylRefPtExtWin();
                    }
                }
            }

            int enclExtWinCtr = 0;

            for (int const surfNum : state.dataViewFactor->EnclSolInfo(enclNum).SurfacePtr) {
                auto const &surf = s_surf->Surface(surfNum);
                if ((surf.Class == SurfaceClass::Window && surf.ExtBoundCond == ExternalEnvironment) ||
                    surf.OriginalClass == SurfaceClass::TDD_Diffuser) {
                    ++enclExtWinCtr;
                    thisEnclDaylight.DayltgExtWinSurfNums(enclExtWinCtr) = surfNum;
                }
            }

            // Get exterior windows in adjacent enclosures that share interior windows with enclNum
            if (thisEnclDaylight.NumOfIntWinAdjEncls > 0) {
                for (int adjEnclNum : thisEnclDaylight.AdjIntWinEnclNums) {
                    // Get exterior windows in EnclNumAdj -- there must be at least one, otherwise
                    // it would not be an "AdjIntWinEncl"
                    for (int SurfNumAdj : state.dataViewFactor->EnclSolInfo(adjEnclNum).SurfacePtr) {
                        auto const &surfAdj = s_surf->Surface(SurfNumAdj);
                        if ((surfAdj.Class == SurfaceClass::Window && surfAdj.ExtBoundCond == ExternalEnvironment) ||
                            surfAdj.OriginalClass == SurfaceClass::TDD_Diffuser) {
                            ++enclExtWinCtr;
                            thisEnclDaylight.DayltgExtWinSurfNums(enclExtWinCtr) = SurfNumAdj;

                            auto &surfWinAdj = s_surf->SurfaceWindow(SurfNumAdj);
                            // If no daylighting in the adjacent enclosure, set up variables anyway:
                            if (state.dataViewFactor->EnclSolInfo(adjEnclNum).TotalEnclosureDaylRefPoints == 0 &&
                                !s_surf->SurfWinSurfDayLightInit(SurfNumAdj)) {
                                surfWinAdj.refPts.allocate(thisEnclNumRefPoints);
                                for (auto &refPt : surfWinAdj.refPts) {
                                    new (&refPt) SurfaceWindowRefPt();
                                }
                                s_surf->SurfWinSurfDayLightInit(SurfNumAdj) = true;
                            }
                        }
                    } // for (SurfNumAdj)
                } // for (adjEnclNum)
            } // if (thisEnclDaylight.NumOfIntWinAdjEncls > 0)

            thisEnclDaylight.NumOfDayltgExtWins = enclExtWin(enclNum);
            int winSize = enclExtWin(enclNum);
            for (int controlNum : thisEnclDaylight.daylightControlIndexes) {
                auto &thisDayltgCtrl = dl->daylightControl(controlNum);
                int refSize = thisDayltgCtrl.TotalDaylRefPoints;
                for (int iHr = 1; iHr <= Constant::iHoursInDay; ++iHr) {
                    thisDayltgCtrl.daylFac[iHr].allocate(winSize, refSize);
                }
            }
        } // if (thisEncl.NumOfRefPoints > 0)

        if (s_surf->TotWinShadingControl > 0) {
            CreateShadeDeploymentOrder(state, enclNum);
        }
    } // for (enclNum)

    for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
        auto const &thisEnclDaylight = dl->enclDaylight(enclNum);
        if (!thisEnclDaylight.hasSplitFluxDaylighting) {
            continue;
        }
        int thisEnclNumRefPoints = state.dataViewFactor->EnclSolInfo(enclNum).TotalEnclosureDaylRefPoints;
        if (thisEnclNumRefPoints > 0) {
            if (s_surf->TotWinShadingControl > 0) {
                MapShadeDeploymentOrderToLoopNumber(state, enclNum);
            }
        }
    }

    for (auto &illumMap : dl->illumMaps) {
        assert((int)illumMap.refPts.size() == illumMap.TotalMapRefPoints);
        if (illumMap.TotalMapRefPoints == 0) {
            continue;
        }

        int numExtWin = enclExtWin(illumMap.enclIndex);
        if (numExtWin == 0) {
            continue;
        }

        for (auto &refPt : illumMap.refPts) {
            refPt.winLums.allocate(numExtWin);
            for (auto &winLums : refPt.winLums) {
                winLums = {0.0, 0.0};
            }
        }

        for (int iHr = 1; iHr <= Constant::iHoursInDay; ++iHr) {
            illumMap.daylFac[iHr].allocate(numExtWin, illumMap.TotalMapRefPoints);
        }

    } // End of map loop

    dl->dirIllum.allocate(Constant::iHoursInDay);
    dl->reflIllum.allocate(Constant::iHoursInDay);
    dl->winLum.allocate(Constant::iHoursInDay);
    dl->avgWinLum.allocate(Constant::iHoursInDay);

    static constexpr std::string_view Format_700("! <Enclosure/Window Adjacency Daylighting Counts>, Enclosure Name, Number of Exterior Windows, "
                                                 "Number of Exterior Windows in Adjacent Enclosures\n");
    print(state.files.eio, Format_700);
    for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
        auto &thisEnclDaylight = dl->enclDaylight(enclNum);
        if (!thisEnclDaylight.hasSplitFluxDaylighting) {
            continue;
        }
        if (state.dataViewFactor->EnclSolInfo(enclNum).TotalEnclosureDaylRefPoints == 0) {
            continue;
        }
        static constexpr std::string_view Format_701("Enclosure/Window Adjacency Daylighting Counts, {},{},{}\n");
        print(state.files.eio,
              Format_701,
              state.dataViewFactor->EnclSolInfo(enclNum).Name,
              thisEnclDaylight.TotalExtWindows,
              (thisEnclDaylight.NumOfDayltgExtWins - thisEnclDaylight.TotalExtWindows));
    }
    static constexpr std::string_view Format_702(
        "! <Enclosure/Window Adjacency Daylighting Matrix>, Enclosure Name, Number of Adjacent Enclosures with Windows,Adjacent "
        "Enclosure Names - 1st 100 (max)\n");
    print(state.files.eio, Format_702);
    for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
        auto &thisEnclDaylight = dl->enclDaylight(enclNum);
        if (!thisEnclDaylight.hasSplitFluxDaylighting) {
            continue;
        }
        if (state.dataViewFactor->EnclSolInfo(enclNum).TotalEnclosureDaylRefPoints == 0) {
            continue;
        }
        static constexpr std::string_view Format_703("Enclosure/Window Adjacency Daylighting Matrix, {},{}");
        print(state.files.eio, Format_703, state.dataViewFactor->EnclSolInfo(enclNum).Name, thisEnclDaylight.NumOfIntWinAdjEncls);
        for (int loop = 1, loop_end = min(thisEnclDaylight.NumOfIntWinAdjEncls, 100); loop <= loop_end; ++loop) {
            print(state.files.eio, ",{}", state.dataViewFactor->EnclSolInfo(thisEnclDaylight.AdjIntWinEnclNums(loop)).Name);
        }
        print(state.files.eio, "\n");
    }

    enclExtWin.deallocate();
}

void CreateShadeDeploymentOrder(EnergyPlusData &state, int const enclNum)
{
    // J. Glazer - 2018
    // create sorted list for shade deployment order
    // first step is to create a sortable list of WindowShadingControl objects by sequence
    auto &dl = state.dataDayltg;
    auto &s_surf = state.dataSurface;

    std::vector<std::pair<int, int>> shadeControlSequence; // sequence, WindowShadingControl
    for (int iShadeCtrl = 1; iShadeCtrl <= s_surf->TotWinShadingControl; ++iShadeCtrl) {
        auto &winShadeControl = s_surf->WindowShadingControl(iShadeCtrl);
        for (int spaceNum : state.dataHeatBal->Zone(winShadeControl.ZoneIndex).spaceIndexes) {
            int shadeCtrlEnclNum = state.dataHeatBal->space(spaceNum).solarEnclosureNum;
            if (shadeCtrlEnclNum == enclNum) {
                shadeControlSequence.push_back(std::make_pair(winShadeControl.SequenceNumber, iShadeCtrl));
                break;
            }
        }
    }
    // sort the WindowShadingControl objects based on sequence number
    sort(shadeControlSequence.begin(), shadeControlSequence.end());
    // now make the deployment list of lists.
    // each sublist is a group of surfaces that should be deployed together
    // often the sublist is just a single item.
    dl->maxShadeDeployOrderExtWins = 0;
    for (int controlNum : dl->enclDaylight(enclNum).daylightControlIndexes) {
        auto &thisDaylightCtrl = dl->daylightControl(controlNum);
        for (auto sequence : shadeControlSequence) { // This is an iterator (THIS_AUTO_OK)
            int curShadeControlNum = sequence.second;
            auto const &winShadeControl = s_surf->WindowShadingControl(curShadeControlNum);
            if (winShadeControl.multiSurfaceControl == MultiSurfaceControl::Group) {
                // add a group of surfaces since they should be deployed as a group
                std::vector<int> group;
                for (int i = 1; i <= winShadeControl.FenestrationCount; i++) {
                    group.push_back(winShadeControl.FenestrationIndex(i));
                }
                thisDaylightCtrl.ShadeDeployOrderExtWins.push_back(group);
            } else {
                // add each individual surface as a separate list so they are deployed individually
                for (int i = 1; i <= winShadeControl.FenestrationCount; i++) {
                    std::vector<int> singleMemberVector;
                    singleMemberVector.push_back(winShadeControl.FenestrationIndex(i));
                    thisDaylightCtrl.ShadeDeployOrderExtWins.push_back(singleMemberVector);
                }
            }
        }
        dl->maxShadeDeployOrderExtWins = max(dl->maxShadeDeployOrderExtWins, (int)thisDaylightCtrl.ShadeDeployOrderExtWins.size());
    }

    dl->maxDayltgExtWins = 0;
    for (auto const &enclDayl : dl->enclDaylight) {
        dl->maxDayltgExtWins = max(dl->maxDayltgExtWins, enclDayl.NumOfDayltgExtWins);
    }

} // CreateShadeDeploymentOrder()

void MapShadeDeploymentOrderToLoopNumber(EnergyPlusData &state, int const enclNum)
{
    // J. Glazer - 2018
    // Allow a way to map back to the original "loop" index that is used in many other places in the
    // ZoneDayLight data structure when traversing the list in the order of the window shaded deployment
    auto &dl = state.dataDayltg;
    auto &s_surf = state.dataSurface;

    auto const &thisEnclDaylight = dl->enclDaylight(enclNum);
    auto const &thisEnclSol = state.dataViewFactor->EnclSolInfo(enclNum);

    if (thisEnclSol.TotalEnclosureDaylRefPoints == 0 || thisEnclDaylight.NumOfDayltgExtWins == 0) {
        return;
    }

    for (int controlNum : thisEnclDaylight.daylightControlIndexes) {
        auto &thisDaylightCtrl = dl->daylightControl(controlNum);
        if (thisDaylightCtrl.ShadeDeployOrderExtWins.size() == 0) {
            continue;
        }

        int count = 0;
        bool showOnce = true;
        for (auto const &listOfExtWin : thisDaylightCtrl.ShadeDeployOrderExtWins) {
            for (int IWinShdOrd : listOfExtWin) {
                ++count;
                if (count > thisEnclDaylight.NumOfDayltgExtWins) {
                    if (showOnce) {
                        ShowWarningError(
                            state,
                            format("MapShadeDeploymentOrderToLoopNumber: too many controlled shaded windows in enclosure {}", thisEnclSol.Name));
                        ShowContinueError(state,
                                          "Check the Zone Name in the WindowShadingControl that references the following fenestration surfaces:");
                        showOnce = false;
                    }
                    ShowContinueError(state, format("  -  {}", s_surf->Surface(IWinShdOrd).Name));
                }
                for (int loop = 1; loop <= thisEnclDaylight.NumOfDayltgExtWins; ++loop) {
                    int IWinLoop = thisEnclDaylight.DayltgExtWinSurfNums(loop);
                    if (IWinShdOrd == IWinLoop) {
                        thisDaylightCtrl.MapShdOrdToLoopNum(count) = loop;
                        break;
                    }
                }
            }
        } // for (listOfExtWin)
    } // for (controlNum)
} // MapShadeDeploymentOrderToLoopNumber()

void DayltgInterReflIllFrIntWins(EnergyPlusData &state, int const enclNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   Mar. 2004

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates the inter-reflected illuminance in a daylit zone from beam
    // and diffuse daylight entering the zone through interior windows. This illuminance
    // is determined by the split-flux method and is assumed to be uniform, i.e., the same
    // at all reference points.

    auto &dl = state.dataDayltg;
    auto &s_surf = state.dataSurface;

    auto &enclDayl = dl->enclDaylight(enclNum);
    auto const &enclSol = state.dataViewFactor->EnclSolInfo(enclNum);

    enclDayl.InterReflIllFrIntWins = 0.0;

    for (int const IWin : enclSol.SurfacePtr) {
        auto &surf = s_surf->Surface(IWin);
        if (surf.Class != SurfaceClass::Window || surf.ExtBoundCond < 1) {
            continue;
        }
        auto const &surfWin = s_surf->SurfaceWindow(IWin);
        // This is an interior window in ZoneNum
        int const ConstrNum = surf.Construction;
        int const adjEnclNum = s_surf->Surface(surf.ExtBoundCond).SolarEnclIndex;
        // Luminous flux transmitted through an int win from adjacent zone's enclosure (lumens)
        Real64 QDifTrans = state.dataHeatBal->EnclSolQSDifSol(adjEnclNum) * state.dataConstruction->Construct(ConstrNum).TransDiffVis * surf.Area *
                           state.dataEnvrn->PDIFLW;
        Real64 QDifTransUp = QDifTrans * surfWin.fractionUpgoing;         // Upgoing part of QDifTrans (lumens)
        Real64 QDifTransDn = QDifTrans * (1.0 - surfWin.fractionUpgoing); // Downgoing part of QDifTrans (lumens)
        if (enclDayl.totInsSurfArea * (1.0 - enclDayl.aveVisDiffReflect) != 0.0) {
            enclDayl.InterReflIllFrIntWins += (QDifTransDn * surfWin.rhoFloorWall + QDifTransUp * surfWin.rhoCeilingWall) /
                                              (enclDayl.totInsSurfArea * (1.0 - enclDayl.aveVisDiffReflect));
        }
    } // for (iWin)

    // Add inter-reflected illuminance from beam solar entering enclosure through interior windows
    // TH, CR 7873, 9/17/2009
    if (dl->enclDaylight(enclNum).totInsSurfArea > 0) {
        enclDayl.InterReflIllFrIntWins +=
            (state.dataHeatBal->EnclSolDBIntWin(enclNum) * state.dataEnvrn->BeamSolarRad * state.dataEnvrn->PDIRLW * enclDayl.floorVisRefl) /
            (enclDayl.totInsSurfArea * (1.0 - enclDayl.aveVisDiffReflect));
    }
} // DayltgInterReflIllFrIntWins()

void CalcMinIntWinSolidAngs(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Fred Winkelmann
    //       DATE WRITTEN   Feb. 2004

    // PURPOSE OF THIS SUBROUTINE:
    // For each Daylighting:Detailed zone finds the minimum solid angle subtended
    // by interior windows through which daylight can pass from adjacent zones with
    // exterior windows.

    auto &dl = state.dataDayltg;
    auto &s_surf = state.dataSurface;

    for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
        auto &thisEnclDaylight = dl->enclDaylight(enclNum);
        thisEnclDaylight.MinIntWinSolidAng = 2.0 * Constant::Pi;
        if (state.dataViewFactor->EnclSolInfo(enclNum).TotalEnclosureDaylRefPoints == 0) {
            continue;
        }
        if (thisEnclDaylight.NumOfIntWinAdjEncls == 0) {
            continue;
        }

        for (int IWin : state.dataViewFactor->EnclSolInfo(enclNum).SurfacePtr) {
            auto const &surf = s_surf->Surface(IWin);

            if ((surf.Class != SurfaceClass::Window) || (surf.ExtBoundCond < 1)) {
                continue;
            }

            // This is an interior window in enclNum
            int const winAdjEnclNum = s_surf->Surface(surf.ExtBoundCond).SolarEnclIndex;
            bool IntWinNextToIntWinAdjZone = false; // True if an interior window is next to a zone with one or more exterior windows
            for (int adjEnclNum : thisEnclDaylight.AdjIntWinEnclNums) {
                if (winAdjEnclNum == adjEnclNum) {
                    IntWinNextToIntWinAdjZone = true;
                    break;
                }
            }

            if (!IntWinNextToIntWinAdjZone) {
                continue;
            }

            for (int controlNum : thisEnclDaylight.daylightControlIndexes) {
                auto &thisDayltgCtrl = dl->daylightControl(controlNum);
                for (int IL = 1; IL <= thisDayltgCtrl.TotalDaylRefPoints; ++IL) {
                    // Reference point in absolute coordinate system
                    Vector3<Real64> RREF = thisDayltgCtrl.refPts(IL).absCoords;
                    bool is_Triangle = (surf.Sides == 3);
                    bool is_Rectangle = (surf.Sides == 4);

                    Vector3<Real64> W1, W2, W3;
                    if (is_Rectangle) {
                        // Vertices of window numbered counter-clockwise starting at upper left as viewed
                        // from inside of room. Assumes original vertices are numbered counter-clockwise from
                        // upper left as viewed from outside.
                        W3 = surf.Vertex(2);
                        W2 = surf.Vertex(3);
                        W1 = surf.Vertex(4);
                    } else if (is_Triangle) {
                        W3 = surf.Vertex(2);
                        W2 = surf.Vertex(3);
                        W1 = surf.Vertex(1);
                    }
                    // Unit vectors from window vertex 2 to 1 and 2 to 3, center point of window,
                    // and vector from ref pt to center of window
                    Vector3<Real64> W21 = W1 - W2;
                    Vector3<Real64> W23 = W3 - W2;
                    Real64 HW = W21.magnitude();
                    Real64 WW = W23.magnitude();
                    Vector3<Real64> WC = (is_Rectangle) ? (W2 + (W23 + W21) / 2.0) : (W2 + (W23 + W21) / 3.0);

                    // Vector from ref point to center of window
                    Vector3<Real64> REFWC = WC - RREF;
                    W21 /= HW;
                    W23 /= WW;
                    // Unit vector normal to window (pointing away from room)
                    Vector3<Real64> WNORM = surf.OutNormVec;
                    // Distance from ref point to center of window
                    Real64 DIS = REFWC.magnitude();
                    // Unit vector from ref point to center of window
                    Vector3<Real64> Ray = REFWC / DIS;
                    // Cosine of angle between ray from ref pt to center of window and window outward normal
                    Real64 COSB = dot(WNORM, Ray);
                    if (COSB > 0.01765) { // 0 <= B < 89 deg
                        // Above test avoids case where ref point cannot receive daylight directly from the
                        // interior window
                        Real64 IntWinSolidAng = COSB * surf.Area / (pow_2(DIS) + 0.001);
                        thisEnclDaylight.MinIntWinSolidAng = min(thisEnclDaylight.MinIntWinSolidAng, IntWinSolidAng);
                    }
                } // for (IL)
            } // for (controlNum)
        } // for (IWin)
    } // for (enclNum)
}

void CheckForGeometricTransform(EnergyPlusData &state, bool &doTransform, Real64 &OldAspectRatio, Real64 &NewAspectRatio)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   February 2009

    // PURPOSE OF THIS SUBROUTINE:
    // check for geometrytransform in the daylighting access for reference and map points

    // METHODOLOGY EMPLOYED:
    // once reference points  have been converted to WCS,
    //  change them to reflect a different aspect
    // ratio for the entire building based on user input.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view CurrentModuleObject = "GeometryTransform";

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Array1D_string cAlphas(1);
    Array1D<Real64> rNumerics;

    // begin execution
    // get user input...
    doTransform = false;
    OldAspectRatio = 1.0;
    NewAspectRatio = 1.0;

    auto &ip = state.dataInputProcessing->inputProcessor;
    auto const &s_ipsc = state.dataIPShortCut;
    auto &s_surf = state.dataSurface;

    if (ip->getNumObjectsFound(state, CurrentModuleObject) == 1) {
        int NAlphas;
        int NNum;
        int IOStat;
        ip->getObjectItem(state,
                          CurrentModuleObject,
                          1,
                          cAlphas,
                          NAlphas,
                          rNumerics,
                          NNum,
                          IOStat,
                          s_ipsc->lNumericFieldBlanks,
                          s_ipsc->lAlphaFieldBlanks,
                          s_ipsc->cAlphaFieldNames,
                          s_ipsc->cNumericFieldNames);
        OldAspectRatio = rNumerics(1);
        NewAspectRatio = rNumerics(2);
        std::string transformPlane = cAlphas(1);
        if (transformPlane != "XY") {
            ShowWarningError(state, format("{}: invalid {}=\"{}...ignored.", CurrentModuleObject, s_ipsc->cAlphaFieldNames(1), cAlphas(1)));
        }
        doTransform = true;
        s_surf->AspectTransform = true;
    }
    if (s_surf->WorldCoordSystem) {
        doTransform = false;
        s_surf->AspectTransform = false;
    }
}

void WriteDaylightMapTitle(EnergyPlusData &state,
                           int const mapNum,
                           InputOutputFile &mapFile,
                           std::string const &mapName,
                           std::string const &environmentName,
                           int const ZoneNum,
                           std::string const &refPts,
                           Real64 const zcoord)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Greg Stark
    //       DATE WRITTEN   Sept 2008

    // PURPOSE OF THIS SUBROUTINE:
    // The purpose of the routine is to allow the daylighting map data to be written in various formats

    // must add correct number of commas at end
    auto &dl = state.dataDayltg;

    std::string fullmapName = fmt::format("{}:{}:{} Illuminance [lux] (Hourly)", state.dataHeatBal->Zone(ZoneNum).Name, environmentName, mapName);
    print(mapFile, "Date/Time{}{}{}{}{}{}\n", dl->MapColSep, fullmapName, dl->MapColSep, refPts, dl->MapColSep, dl->MapColSep);

    if (state.dataSQLiteProcedures->sqlite) {
        state.dataSQLiteProcedures->sqlite->createSQLiteDaylightMapTitle(mapNum, fullmapName, environmentName, ZoneNum, refPts, zcoord);
    }
} // WritDaylightMapTitle()

} // namespace EnergyPlus::Dayltg

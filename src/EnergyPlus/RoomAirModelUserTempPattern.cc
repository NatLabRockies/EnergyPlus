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

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/ArrayS.functions.hh>
// #include <ObjexxFCL/Fmath.hh>
#include <ObjexxFCL/member.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataErrorTracking.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/RoomAirModelUserTempPattern.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus::RoomAir {

// MODULE INFORMATION:
//       AUTHOR         Brent Griffith
//       DATE WRITTEN   August 2005 (started in January 2004)
//       RE-ENGINEERED

// PURPOSE OF THIS MODULE:
// This module is the main module for running the
// user-defined temperature pattern model.
// This "air model" doesn't predict anything about the room air
// but provides a method for users to model the
// impact of non-uniform air temps.  the distribution of air temperatures
// is defined by the user and referred to as a "pattern"

// METHODOLOGY EMPLOYED:
// This module contains all subroutines required by the
// user defined temperature pattern roomair modeling.
// See DataRoomAir.cc for variable declarations

// Functions

void ManageUserDefinedPatterns(EnergyPlusData &state, int const ZoneNum) // index number for the specified zone
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   January 2004/Aug 2005
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    //  manage the user-defined air temp. distribution model

    // transfer data from surface domain to air domain for the specified zone
    InitTempDistModel(state, ZoneNum);

    GetSurfHBDataForTempDistModel(state, ZoneNum);

    // perform TempDist model calculations
    CalcTempDistModel(state, ZoneNum);

    // transfer data from air domain back to surface domain for the specified zone
    SetSurfHBDataForTempDistModel(state, ZoneNum);
}

//****************************************************

void InitTempDistModel(EnergyPlusData &state, int const ZoneNum) // index number for the specified zone
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         <author>
    //       DATE WRITTEN   <date_written>

    if (state.dataRoomAirModelTempPattern->MyOneTimeFlag) {
        state.dataRoomAirModelTempPattern->MyEnvrnFlag.dimension(state.dataGlobal->NumOfZones, true);
        state.dataRoomAirModelTempPattern->MyOneTimeFlag = false;
    }

    auto &patternZoneInfo = state.dataRoomAir->AirPatternZoneInfo(ZoneNum);
    if (state.dataGlobal->BeginEnvrnFlag && state.dataRoomAirModelTempPattern->MyEnvrnFlag(ZoneNum)) {
        patternZoneInfo.TairMean = 23.0;
        patternZoneInfo.Tstat = 23.0;
        patternZoneInfo.Tleaving = 23.0;
        patternZoneInfo.Texhaust = 23.0;
        patternZoneInfo.Gradient = 0.0;
        for (int SurfNum = 1; SurfNum <= patternZoneInfo.totNumSurfs; ++SurfNum) {
            patternZoneInfo.Surf(SurfNum).TadjacentAir = 23.0;
        }
        state.dataRoomAirModelTempPattern->MyEnvrnFlag(ZoneNum) = false;
    }

    if (!state.dataGlobal->BeginEnvrnFlag) {
        state.dataRoomAirModelTempPattern->MyEnvrnFlag(ZoneNum) = true;
    }

    // init report variable
    patternZoneInfo.Gradient = 0.0;
}

void GetSurfHBDataForTempDistModel(EnergyPlusData &state, int const ZoneNum) // index number for the specified zone
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   August 2005

    // PURPOSE OF THIS SUBROUTINE:
    //  map data from Heat Balance domain to Room Air Modeling Domain
    //  for the current zone, (only need mean air temp)
    //  also acts as an init routine

    // METHODOLOGY EMPLOYED:
    // use ZT from DataHeatBalFanSys

    auto &patternZoneInfo = state.dataRoomAir->AirPatternZoneInfo(ZoneNum);
    auto const &zoneHeatBal = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);
    // initialize in preparation for calculations
    patternZoneInfo.Tstat = zoneHeatBal.MAT;
    patternZoneInfo.Tleaving = zoneHeatBal.MAT;
    patternZoneInfo.Texhaust = zoneHeatBal.MAT;
    for (auto &e : patternZoneInfo.Surf) {
        e.TadjacentAir = zoneHeatBal.MAT;
    }

    // the only input this method needs is the zone MAT or ZT or ZTAV  ?  (original was ZT)
    patternZoneInfo.TairMean = zoneHeatBal.MAT; // this is lagged from previous corrector result
}

//*****************************************************************************************

void CalcTempDistModel(EnergyPlusData &state, int const ZoneNum) // index number for the specified zone
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   August 2005
    //       MODIFIED
    //       RE-ENGINEERED

    // PURPOSE OF THIS SUBROUTINE:
    // figure out which pattern is scheduled and call
    // appropriate subroutine

    // Using/Aliasing
    using General::FindNumberInList;

    auto &patternZoneInfo = state.dataRoomAir->AirPatternZoneInfo(ZoneNum);
    // first determine availability
    Real64 AvailTest = patternZoneInfo.availSched->getCurrentVal();

    if ((AvailTest != 1.0) || (!patternZoneInfo.IsUsed)) {
        // model not to be used. Use complete mixing method

        patternZoneInfo.Tstat = patternZoneInfo.TairMean;
        patternZoneInfo.Tleaving = patternZoneInfo.TairMean;
        patternZoneInfo.Texhaust = patternZoneInfo.TairMean;
        for (auto &e : patternZoneInfo.Surf) {
            e.TadjacentAir = patternZoneInfo.TairMean;
        }

        return;

    } else { // choose pattern and call subroutine

        int CurntPatternKey = patternZoneInfo.patternSched->getCurrentVal();

        int CurPatrnID = FindNumberInList(CurntPatternKey, state.dataRoomAir->AirPattern, &TemperaturePattern::PatrnID);

        if (CurPatrnID == 0) {
            // throw error here ? way to test schedules before getting to this point?
            ShowFatalError(state, format("User defined room air pattern index not found: {}", CurntPatternKey));
            return;
        }

        switch (state.dataRoomAir->AirPattern(CurPatrnID).PatternMode) {
        case UserDefinedPatternType::ConstGradTemp: {
            FigureConstGradPattern(state, CurPatrnID, ZoneNum);
        } break;
        case UserDefinedPatternType::TwoGradInterp: {
            FigureTwoGradInterpPattern(state, CurPatrnID, ZoneNum);
        } break;
        case UserDefinedPatternType::NonDimenHeight: {
            FigureHeightPattern(state, CurPatrnID, ZoneNum);
        } break;
        case UserDefinedPatternType::SurfMapTemp: {
            FigureSurfMapPattern(state, CurPatrnID, ZoneNum);
        } break;
        default: {
            assert(false);
        } break;
        }
    } // availability control construct
}

void FigureSurfMapPattern(EnergyPlusData &state, int const PattrnID, int const ZoneNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B Griffith
    //       DATE WRITTEN   August 2005
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // main calculation routine for surface pattern

    // METHODOLOGY EMPLOYED:
    // simple polling and applying prescribed
    // delta Tai's to current mean air temp
    // on a surface by surface basis

    // Using/Aliasing
    using General::FindNumberInList;

    auto &patternZoneInfo = state.dataRoomAir->AirPatternZoneInfo(ZoneNum);
    auto &pattern = state.dataRoomAir->AirPattern(PattrnID);
    Real64 Tmean = patternZoneInfo.TairMean;

    for (int i = 1; i <= patternZoneInfo.totNumSurfs; ++i) {
        // cycle through zone surfaces and look for match
        int found = FindNumberInList(patternZoneInfo.Surf(i).SurfID, pattern.MapPatrn.SurfID, pattern.MapPatrn.NumSurfs);
        if (found != 0) { // if surf is in map then assign, else give it MAT
            patternZoneInfo.Surf(i).TadjacentAir = pattern.MapPatrn.DeltaTai(found) + Tmean;
        } else {
            patternZoneInfo.Surf(i).TadjacentAir = Tmean;
        }
    }

    patternZoneInfo.Tstat = pattern.DeltaTstat + Tmean;
    patternZoneInfo.Tleaving = pattern.DeltaTleaving + Tmean;
    patternZoneInfo.Texhaust = pattern.DeltaTexhaust + Tmean;
}

void FigureHeightPattern(EnergyPlusData &state, int const PattrnID, int const ZoneNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B Griffith
    //       DATE WRITTEN   August 2005

    // PURPOSE OF THIS SUBROUTINE:
    // calculate the pattern for non-dimensional vertical profile

    // METHODOLOGY EMPLOYED:
    // treat profile as lookup table and interpolate

    // Using/Aliasing
    using Fluid::FindArrayIndex;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

    auto &patternZoneInfo = state.dataRoomAir->AirPatternZoneInfo(ZoneNum);
    auto &pattern = state.dataRoomAir->AirPattern(PattrnID);
    Real64 tmpDeltaTai = 0.0;
    Real64 Tmean = patternZoneInfo.TairMean;

    for (int i = 1; i <= patternZoneInfo.totNumSurfs; ++i) {

        Real64 zeta = patternZoneInfo.Surf(i).Zeta;
        int lowSideID = FindArrayIndex(zeta, pattern.VertPatrn.ZetaPatrn);
        int highSideID = lowSideID + 1;
        if (lowSideID == 0) {
            lowSideID = 1; // protect against array bounds
        }

        Real64 lowSideZeta = pattern.VertPatrn.ZetaPatrn(lowSideID);
        Real64 hiSideZeta = (highSideID <= isize(pattern.VertPatrn.ZetaPatrn)) ? pattern.VertPatrn.ZetaPatrn(highSideID) : lowSideZeta;

        if ((hiSideZeta - lowSideZeta) != 0.0) {
            Real64 fractBtwn = (zeta - lowSideZeta) / (hiSideZeta - lowSideZeta);
            tmpDeltaTai = pattern.VertPatrn.DeltaTaiPatrn(lowSideID) +
                          fractBtwn * (pattern.VertPatrn.DeltaTaiPatrn(highSideID) - pattern.VertPatrn.DeltaTaiPatrn(lowSideID));

        } else { // would divide by zero, using low side value

            tmpDeltaTai = pattern.VertPatrn.DeltaTaiPatrn(lowSideID);
        }

        patternZoneInfo.Surf(i).TadjacentAir = tmpDeltaTai + Tmean;

    } // surfaces in this zone

    patternZoneInfo.Tstat = pattern.DeltaTstat + Tmean;
    patternZoneInfo.Tleaving = pattern.DeltaTleaving + Tmean;
    patternZoneInfo.Texhaust = pattern.DeltaTexhaust + Tmean;
}

void FigureTwoGradInterpPattern(EnergyPlusData &state, int const PattrnID, int const ZoneNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B Griffith
    //       DATE WRITTEN   Aug 2005

    // PURPOSE OF THIS SUBROUTINE:
    // calculate two gradient interpolation pattern

    // METHODOLOGY EMPLOYED:
    // Case statement controls how interpolations are done
    // based on user selected mode.
    // calculations vary by mode

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 Grad; // vertical temperature gradient C/m

    auto &patternZoneInfo = state.dataRoomAir->AirPatternZoneInfo(ZoneNum);
    auto const &pattern = state.dataRoomAir->AirPattern(PattrnID);

    if (state.dataRoomAirModelTempPattern->MyOneTimeFlag2) {
        state.dataRoomAirModelTempPattern->SetupOutputFlag.dimension(state.dataGlobal->NumOfZones, true); // init
        state.dataRoomAirModelTempPattern->MyOneTimeFlag2 = false;
    }

    if (state.dataRoomAirModelTempPattern->SetupOutputFlag(ZoneNum)) {
        SetupOutputVariable(state,
                            "Room Air Zone Vertical Temperature Gradient",
                            Constant::Units::K_m,
                            patternZoneInfo.Gradient,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            patternZoneInfo.ZoneName);

        state.dataRoomAirModelTempPattern->SetupOutputFlag(ZoneNum) = false;
    }

    Real64 Tmean = patternZoneInfo.TairMean;

    auto const &twoGrad = pattern.TwoGradPatrn;
    // determine gradient depending on mode
    switch (pattern.TwoGradPatrn.InterpolationMode) {
    case UserDefinedPatternMode::OutdoorDryBulb: {
        Grad = OutdoorDryBulbGrad(state.dataHeatBal->Zone(ZoneNum).OutDryBulbTemp,
                                  twoGrad.UpperBoundTempScale,
                                  twoGrad.HiGradient,
                                  twoGrad.LowerBoundTempScale,
                                  twoGrad.LowGradient);
    } break;
    case UserDefinedPatternMode::ZoneAirTemp: {
        if (Tmean >= twoGrad.UpperBoundTempScale) {
            Grad = twoGrad.HiGradient;
        } else if (Tmean <= twoGrad.LowerBoundTempScale) {
            Grad = twoGrad.LowGradient;
        } else if ((twoGrad.UpperBoundTempScale - twoGrad.LowerBoundTempScale) == 0.0) {
            // bad user input, trapped during get input
            Grad = twoGrad.LowGradient;
        } else {
            Grad = twoGrad.LowGradient + ((Tmean - twoGrad.LowerBoundTempScale) / (twoGrad.UpperBoundTempScale - twoGrad.LowerBoundTempScale)) *
                                             (twoGrad.HiGradient - twoGrad.LowGradient);
        }
    } break;
    case UserDefinedPatternMode::DeltaOutdoorZone: {
        Real64 DeltaT = state.dataHeatBal->Zone(ZoneNum).OutDryBulbTemp - Tmean;
        if (DeltaT >= twoGrad.UpperBoundTempScale) {
            Grad = twoGrad.HiGradient;
        } else if (DeltaT <= twoGrad.LowerBoundTempScale) {
            Grad = twoGrad.LowGradient;
        } else if ((twoGrad.UpperBoundTempScale - twoGrad.LowerBoundTempScale) == 0.0) {
            Grad = twoGrad.LowGradient;
        } else {
            Grad = twoGrad.LowGradient + ((DeltaT - twoGrad.LowerBoundTempScale) / (twoGrad.UpperBoundTempScale - twoGrad.LowerBoundTempScale)) *
                                             (twoGrad.HiGradient - twoGrad.LowGradient);
        }
    } break;
    case UserDefinedPatternMode::SensibleCooling: {
        Real64 CoolLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).airSysCoolRate;
        if (CoolLoad >= twoGrad.UpperBoundHeatRateScale) {
            Grad = twoGrad.HiGradient;

        } else if (CoolLoad <= twoGrad.LowerBoundHeatRateScale) {

            Grad = twoGrad.LowGradient;
        } else { // interpolate
            if ((twoGrad.UpperBoundHeatRateScale - twoGrad.LowerBoundHeatRateScale) == 0.0) {
                Grad = twoGrad.LowGradient;
            } else {

                Grad = twoGrad.LowGradient +
                       ((CoolLoad - twoGrad.LowerBoundHeatRateScale) / (twoGrad.UpperBoundHeatRateScale - twoGrad.LowerBoundHeatRateScale)) *
                           (twoGrad.HiGradient - twoGrad.LowGradient);
            }
        }
    } break;
    case UserDefinedPatternMode::SensibleHeating: {
        Real64 HeatLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).airSysHeatRate;
        if (HeatLoad >= twoGrad.UpperBoundHeatRateScale) {
            Grad = twoGrad.HiGradient;
        } else if (HeatLoad <= twoGrad.LowerBoundHeatRateScale) {
            Grad = twoGrad.LowGradient;
        } else if ((twoGrad.UpperBoundHeatRateScale - twoGrad.LowerBoundHeatRateScale) == 0.0) {
            Grad = twoGrad.LowGradient;
        } else {
            Grad = twoGrad.LowGradient +
                   ((HeatLoad - twoGrad.LowerBoundHeatRateScale) / (twoGrad.UpperBoundHeatRateScale - twoGrad.LowerBoundHeatRateScale)) *
                       (twoGrad.HiGradient - twoGrad.LowGradient);
        }
    } break;
    default:
        break;
    }

    Real64 ZetaTmean = 0.5; // by definition,

    for (int i = 1; i <= patternZoneInfo.totNumSurfs; ++i) {
        Real64 zeta = patternZoneInfo.Surf(i).Zeta;
        Real64 DeltaHeight = -1.0 * (ZetaTmean - zeta) * patternZoneInfo.ZoneHeight;
        patternZoneInfo.Surf(i).TadjacentAir = (DeltaHeight * Grad) + Tmean;
    }

    patternZoneInfo.Tstat = -1.0 * (0.5 * patternZoneInfo.ZoneHeight - twoGrad.TstatHeight) * Grad + Tmean;
    patternZoneInfo.Tleaving = -1.0 * (0.5 * patternZoneInfo.ZoneHeight - twoGrad.TleavingHeight) * Grad + Tmean;
    patternZoneInfo.Texhaust = -1.0 * (0.5 * patternZoneInfo.ZoneHeight - twoGrad.TexhaustHeight) * Grad + Tmean;
    patternZoneInfo.Gradient = Grad;
}

Real64 OutdoorDryBulbGrad(Real64 DryBulbTemp, // Zone(ZoneNum).OutDryBulbTemp
                          Real64 UpperBound,  // RoomAirPattern(PattrnID).TwoGradPatrn.UpperBoundTempScale
                          Real64 HiGradient,  // RoomAirPattern(PattrnID).TwoGradPatrn.HiGradient
                          Real64 LowerBound,  // RoomAirPattern(PattrnID).TwoGradPatrn.LowerBoundTempScale
                          Real64 LowGradient  // RoomAirPattern(PattrnID).TwoGradPatrn.LowGradient
)
{
    if (DryBulbTemp >= UpperBound) {
        return HiGradient;
    } else if (DryBulbTemp <= LowerBound) {
        return LowGradient;
    } else if ((UpperBound - LowerBound) == 0.0) {
        return LowGradient;
    } else {
        return LowGradient + ((DryBulbTemp - LowerBound) / (UpperBound - LowerBound)) * (HiGradient - LowGradient);
    }
}

void FigureConstGradPattern(EnergyPlusData &state, int const PattrnID, int const ZoneNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   August 2005

    auto &patternZoneInfo = state.dataRoomAir->AirPatternZoneInfo(ZoneNum);
    auto const &pattern = state.dataRoomAir->AirPattern(PattrnID);
    Real64 Tmean = patternZoneInfo.TairMean;  // MAT
    Real64 Grad = pattern.GradPatrn.Gradient; // Vertical temperature gradient

    Real64 ZetaTmean = 0.5; // non-dimensional height for MAT

    for (int i = 1; i <= patternZoneInfo.totNumSurfs; ++i) {
        Real64 zeta = patternZoneInfo.Surf(i).Zeta;
        Real64 DeltaHeight = -1.0 * (ZetaTmean - zeta) * patternZoneInfo.ZoneHeight;
        patternZoneInfo.Surf(i).TadjacentAir = DeltaHeight * Grad + Tmean;
    }

    patternZoneInfo.Tstat = pattern.DeltaTstat + Tmean;
    patternZoneInfo.Tleaving = pattern.DeltaTleaving + Tmean;
    patternZoneInfo.Texhaust = pattern.DeltaTexhaust + Tmean;
}

//*****************************************************************************************

Real64 FigureNDheightInZone(EnergyPlusData &state, int const thisHBsurf) // index in main Surface array
{
    // FUNCTION INFORMATION:
    //       AUTHOR         B.Griffith
    //       DATE WRITTEN   aug 2005, Jan2004

    // PURPOSE OF THIS FUNCTION:
    // return a non-dimensional height zeta

    // METHODOLOGY EMPLOYED:
    // figure average floor height (follows code in surfacegeometry.cc
    // use ceiling height from Zone structure
    // non dimensionalize surface's centroid's Z value

    // FUNCTION PARAMETER DEFINITIONS:
    Real64 constexpr TolValue(0.0001);

    // Get the centroid height for the surface
    Real64 Zcm = state.dataSurface->Surface(thisHBsurf).Centroid.z;
    auto &zone = state.dataHeatBal->Zone(state.dataSurface->Surface(thisHBsurf).Zone);

    // this next Do block is copied from SurfaceGeometry.cc with modification for just floor Z
    // used find floor z.
    int FloorCount = 0;
    Real64 ZFlrAvg = 0.0;
    Real64 ZMax = 0.0;
    Real64 ZMin = 0.0;
    int Count = 0;
    for (int spaceNum : zone.spaceIndexes) {
        auto const &thisSpace = state.dataHeatBal->space(spaceNum);
        for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {
            auto const &surf = state.dataSurface->Surface(SurfNum);
            if (surf.Class == DataSurfaces::SurfaceClass::Floor) {
                // Use Average Z for surface, more important for roofs than floors...
                ++FloorCount;
                Real64 Z1 = minval(surf.Vertex, &Vector3<Real64>::z);
                Real64 Z2 = maxval(surf.Vertex, &Vector3<Real64>::z);
                ZFlrAvg += (Z1 + Z2) / 2.0;
            } else if (surf.Class == DataSurfaces::SurfaceClass::Wall) {
                // Use Wall calculation in case no floor in zone
                ++Count;
                if (Count == 1) {
                    ZMax = surf.Vertex(1).z;
                    ZMin = ZMax;
                }
                ZMax = max(ZMax, maxval(surf.Vertex, &Vector3<Real64>::z));
                ZMin = min(ZMin, minval(surf.Vertex, &Vector3<Real64>::z));
            }
        }
    }

    ZFlrAvg = (FloorCount > 0.0) ? (ZFlrAvg / FloorCount) : ZMin;

    Real64 ZoneZorig = ZFlrAvg; // Z floor  [M]
    Real64 ZoneCeilHeight = zone.CeilingHeight;

    // first check if some basic things are reasonable

    Real64 SurfMinZ = minval(state.dataSurface->Surface(thisHBsurf).Vertex, &Vector3<Real64>::z);
    Real64 SurfMaxZ = maxval(state.dataSurface->Surface(thisHBsurf).Vertex, &Vector3<Real64>::z);

    if (SurfMinZ < (ZoneZorig - TolValue)) {
        if (state.dataGlobal->DisplayExtraWarnings) {
            ShowWarningError(state, "RoomAirModelUserTempPattern: Problem in non-dimensional height calculation");
            ShowContinueError(state, format("too low surface: {} in zone: {}", state.dataSurface->Surface(thisHBsurf).Name, zone.Name));
            ShowContinueError(state, format("**** Average floor height of zone is: {:.3R}", ZoneZorig));
            ShowContinueError(state, format("**** Surface minimum height is: {:.3R}", SurfMinZ));
        } else {
            ++state.dataErrTracking->TotalRoomAirPatternTooLow;
        }
    }

    if (SurfMaxZ > (ZoneZorig + ZoneCeilHeight + TolValue)) {
        if (state.dataGlobal->DisplayExtraWarnings) {
            ShowWarningError(state, "RoomAirModelUserTempPattern: Problem in non-dimensional height calculation");
            ShowContinueError(state, format(" too high surface: {} in zone: {}", state.dataSurface->Surface(thisHBsurf).Name, zone.Name));
            ShowContinueError(state, format("**** Average Ceiling height of zone is: {:.3R}", (ZoneZorig + ZoneCeilHeight)));
            ShowContinueError(state, format("**** Surface Maximum height is: {:.3R}", SurfMaxZ));
        } else {
            ++state.dataErrTracking->TotalRoomAirPatternTooHigh;
        }
    }

    // non dimensionalize.
    Real64 Zeta = (Zcm - ZoneZorig) / ZoneCeilHeight;
    if (Zeta > 0.99) {
        Zeta = 0.99;
    } else if (Zeta < 0.01) {
        Zeta = 0.01;
    }

    return Zeta;
}

//***************************************************

void SetSurfHBDataForTempDistModel(EnergyPlusData &state, int const ZoneNum) // index number for the specified zone
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   August 2005,Feb. 2004

    // PURPOSE OF THIS SUBROUTINE:
    //  map data from air domain back to surface domain for each zone
    //  collects code couples to remote data structures

    // METHODOLOGY EMPLOYED:
    // sets values in Heat balance variables

    // Using/Aliasing
    using HVAC::RetTempMax;
    using HVAC::RetTempMin;
    using InternalHeatGains::SumAllReturnAirLatentGains;
    using Psychrometrics::PsyCpAirFnW;
    using Psychrometrics::PsyHFnTdbW;
    using Psychrometrics::PsyHgAirFnWTdb;
    using Psychrometrics::PsyRhoAirFnPbTdbW;

    // set air system leaving node conditions
    // this is not so easy.  THis task is normally done in CalcZoneLeavingConditions
    //  but efforts to do this update there were not successful.
    //  Need to revisit how to best implement this. Ended up taking code from CalcZoneLeavingConditions
    //  ZoneNum is already equal to ActualZoneNum , changed block of source

    auto &patternZoneInfo = state.dataRoomAir->AirPatternZoneInfo(ZoneNum);

    if (patternZoneInfo.ZoneNodeID != 0) {
        // the zone system node should get the conditions leaving the zone (but before return air heat gains are added).
        state.dataLoopNodes->Node(patternZoneInfo.ZoneNodeID).Temp = patternZoneInfo.Tleaving;
    }

    // What if ZoneNodeID is 0?

    auto &zoneNode = state.dataLoopNodes->Node(patternZoneInfo.ZoneNodeID);
    auto const &zone = state.dataHeatBal->Zone(ZoneNum);
    auto &zoneHeatBal = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);

    int ZoneMult = zone.Multiplier * zone.ListMultiplier;

    for (int returnNodeNum : state.dataZoneEquip->ZoneEquipConfig(ZoneNum).ReturnNode) {
        // BEGIN BLOCK of code from CalcZoneLeavingConditions*********************************
        auto &returnNode = state.dataLoopNodes->Node(returnNodeNum);

        // RETURN AIR HEAT GAIN from the Lights statement; this heat gain is stored in
        // Add sensible heat gain from refrigerated cases with under case returns
        Real64 QRetAir = InternalHeatGains::zoneSumAllReturnAirConvectionGains(state, ZoneNum, returnNodeNum);

        Real64 CpAir = PsyCpAirFnW(zoneNode.HumRat);

        // Need to add the energy to the return air from lights and from airflow windows. Where the heat
        // is added depends on if there is system flow or not.  If there is system flow the heat is added
        // to the Zone Return Node.  If there is no system flow then the heat is added back to the zone in the
        // Correct step through the SysDepZoneLoads variable.

        Real64 MassFlowRA = returnNode.MassFlowRate / ZoneMult;
        Real64 TempZoneAir = patternZoneInfo.Tleaving; // key difference from
        Real64 TempRetAir = TempZoneAir;
        Real64 WinGapFlowToRA = 0.0;
        Real64 WinGapTtoRA = 0.0;
        Real64 WinGapFlowTtoRA = 0.0;

        if (zone.HasAirFlowWindowReturn) {
            for (int spaceNum : zone.spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {
                    if (state.dataSurface->SurfWinAirflowThisTS(SurfNum) > 0.0 &&
                        state.dataSurface->SurfWinAirflowDestination(SurfNum) == DataSurfaces::WindowAirFlowDestination::Return) {
                        Real64 FlowThisTS =
                            PsyRhoAirFnPbTdbW(
                                state, state.dataEnvrn->OutBaroPress, state.dataSurface->SurfWinTAirflowGapOutlet(SurfNum), zoneNode.HumRat) *
                            state.dataSurface->SurfWinAirflowThisTS(SurfNum) * state.dataSurface->Surface(SurfNum).Width;
                        WinGapFlowToRA += FlowThisTS;
                        WinGapFlowTtoRA += FlowThisTS * state.dataSurface->SurfWinTAirflowGapOutlet(SurfNum);
                    }
                }
            }
        }
        if (WinGapFlowToRA > 0.0) {
            WinGapTtoRA = WinGapFlowTtoRA / WinGapFlowToRA;
        }

        if (!zone.NoHeatToReturnAir) {
            if (MassFlowRA > 0.0) {
                if (WinGapFlowToRA > 0.0) {
                    // Add heat-to-return from window gap airflow
                    if (MassFlowRA >= WinGapFlowToRA) {
                        TempRetAir = (WinGapFlowTtoRA + (MassFlowRA - WinGapFlowToRA) * TempZoneAir) / MassFlowRA;
                    } else {
                        // All of return air comes from flow through airflow windows
                        TempRetAir = WinGapTtoRA;
                        // Put heat from window airflow that exceeds return air flow into zone air
                        zoneHeatBal.SysDepZoneLoads += (WinGapFlowToRA - MassFlowRA) * CpAir * (WinGapTtoRA - TempZoneAir);
                    }
                }
                // Add heat-to-return from lights
                TempRetAir += QRetAir / (MassFlowRA * CpAir);
                if (TempRetAir > RetTempMax) {
                    returnNode.Temp = RetTempMax;
                    if (!state.dataGlobal->ZoneSizingCalc) {
                        zoneHeatBal.SysDepZoneLoads += CpAir * MassFlowRA * (TempRetAir - RetTempMax);
                    }
                } else if (TempRetAir < RetTempMin) {
                    returnNode.Temp = RetTempMin;
                    if (!state.dataGlobal->ZoneSizingCalc) {
                        zoneHeatBal.SysDepZoneLoads += CpAir * MassFlowRA * (TempRetAir - RetTempMin);
                    }
                } else {
                    returnNode.Temp = TempRetAir;
                }
            } else { // No return air flow
                // Assign all heat-to-return from window gap airflow to zone air
                if (WinGapFlowToRA > 0.0) {
                    zoneHeatBal.SysDepZoneLoads += WinGapFlowToRA * CpAir * (WinGapTtoRA - TempZoneAir);
                }
                // Assign all heat-to-return from lights to zone air
                if (QRetAir > 0.0) {
                    zoneHeatBal.SysDepZoneLoads += QRetAir;
                }
                returnNode.Temp = zoneNode.Temp;
            }
        } else {
            returnNode.Temp = zoneNode.Temp;
        }

        // Update the rest of the Return Air Node conditions, if the return air system exists!
        returnNode.Press = zoneNode.Press;

        Real64 H2OHtOfVap = PsyHgAirFnWTdb(zoneNode.HumRat, returnNode.Temp);

        // Include impact of under case returns for refrigerated display cases when updating return node
        // humidity ratio
        if (!zone.NoHeatToReturnAir) {
            if (MassFlowRA > 0) {
                Real64 SumRetAirLatentGainRate = SumAllReturnAirLatentGains(state, ZoneNum, returnNodeNum);
                returnNode.HumRat = zoneNode.HumRat + (SumRetAirLatentGainRate / (H2OHtOfVap * MassFlowRA));
            } else {
                // If no mass flow rate exists, include the latent HVAC case credit with the latent Zone case credit
                returnNode.HumRat = zoneNode.HumRat;
                state.dataHeatBal->RefrigCaseCredit(ZoneNum).LatCaseCreditToZone += state.dataHeatBal->RefrigCaseCredit(ZoneNum).LatCaseCreditToHVAC;
                // shouldn't the HVAC term be zeroed out then?
                Real64 SumRetAirLatentGainRate = SumAllReturnAirLatentGains(state, ZoneNum, 0);
                zoneHeatBal.latentGain += SumRetAirLatentGainRate;
            }
        } else {
            returnNode.HumRat = zoneNode.HumRat;
            state.dataHeatBal->RefrigCaseCredit(ZoneNum).LatCaseCreditToZone += state.dataHeatBal->RefrigCaseCredit(ZoneNum).LatCaseCreditToHVAC;
            // shouldn't the HVAC term be zeroed out then?

            zoneHeatBal.latentGain += SumAllReturnAirLatentGains(state, ZoneNum, returnNodeNum);
        }

        returnNode.Enthalpy = PsyHFnTdbW(returnNode.Temp, returnNode.HumRat);

        // END BLOCK of code from CalcZoneLeavingConditions*********************************
    }

    // set exhaust node leaving temp if present
    if (allocated(patternZoneInfo.ExhaustAirNodeID)) {
        for (int exhaustAirNodeID : patternZoneInfo.ExhaustAirNodeID) {
            state.dataLoopNodes->Node(exhaustAirNodeID).Temp = patternZoneInfo.Texhaust;
        }
    }

    // set thermostat reading for air system .
    state.dataHeatBalFanSys->TempTstatAir(ZoneNum) = patternZoneInfo.Tstat;

    // set results for all surface
    for (int spaceNum : zone.spaceIndexes) {
        auto const &thisSpace = state.dataHeatBal->space(spaceNum);
        for (int i = thisSpace.HTSurfaceFirst, j = 0; i <= thisSpace.HTSurfaceLast; ++i) {
            state.dataHeatBal->SurfTempEffBulkAir(i) = patternZoneInfo.Surf(++j).TadjacentAir;
        }
    }

    // set flag for reference air temperature mode
    for (int spaceNum : zone.spaceIndexes) {
        auto const &thisSpace = state.dataHeatBal->space(spaceNum);
        for (int i = thisSpace.HTSurfaceFirst; i <= thisSpace.HTSurfaceLast; ++i) {
            state.dataSurface->SurfTAirRef(i) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
            state.dataSurface->SurfTAirRefRpt(i) = DataSurfaces::SurfTAirRefReportVals[state.dataSurface->SurfTAirRef(i)];
        }
    }
}

//*****************************************************************************************

} // namespace EnergyPlus::RoomAir

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
#include <numeric>
#include <string>
#include <unordered_map>
#include <unordered_set>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>
#include <ObjexxFCL/member.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/ConvectionCoefficients.hh>
#include <EnergyPlus/ConvectionConstants.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataErrorTracking.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/Vectors.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus::Convect {

// Module containing the routines dealing with the convection coefficients

// MODULE INFORMATION:
//       AUTHOR         Rick Strand
//       DATE WRITTEN   August 2000
//       MODIFIED       Brent Griffith, August 2010 expanded model choices
//       RE-ENGINEERED  na

// PURPOSE OF THIS MODULE:
// This module contains the routines dealing with convection coefficients.
// This module collects correlations/calculations for both the interior and exterior
// Manages a portion of the input and calculations for Hc values for use in surface heat balances.

// METHODOLOGY EMPLOYED:
// Subroutines are called to fill the variable HConvIn with the convection coefficient at
// the inside face.  or outside face for the current surface.

using namespace DataLoopNode;
using namespace DataHeatBalance;
using namespace DataSurfaces;
using namespace DataVectorTypes;

// Coefficients that modify the convection coeff based on surface roughness
std::array<Real64, 6> const RoughnessMultiplier{2.17, 1.67, 1.52, 1.13, 1.11, 1.0};
constexpr std::array<std::string_view, (int)RefTemp::Num> RefTempNamesUC{"MEANAIRTEMPERATURE", "ADJACENTAIRTEMPERATURE", "SUPPLYAIRTEMPERATURE"};
constexpr std::array<std::string_view, (int)RefWind::Num> RefWindNamesUC{
    "WEATHERFILE", "HEIGHTADJUST", "PARALLELCOMPONENT", "PARALLELCOMPONENTHEIGHTADJUST"};

enum class ConvSurfDeltaT
{
    Invalid = -1,
    Positive,
    Zero,
    Negative,
    Num
};

// parameters, by zone, for flow regimes for adaptive convection on inside face
enum class InConvFlowRegime
{
    Invalid = -1,
    A1, // In-floor heating or in-ceiling cooling
    A2, // In-wall heating
    A3, // no HVAC system, all buoyancy
    B,  // Convective heater in zone
    C,  // central mechanical air
    D,  // zone mechanical air
    E,  // mixed. mechanical air and buoyancy
    Num
};

void InitIntConvCoeff(EnergyPlusData &state,
                      const Array1D<Real64> &SurfaceTemperatures,    // Temperature of surfaces for evaluation of HcIn
                      ObjexxFCL::Optional_int_const ZoneToResimulate // if passed in, then only calculate surfaces that have this zone
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   March 1998
    //       MODIFIED       Dan Fisher, Nov 2000
    //                      Sep 2011 LKL/BG - resimulate only zones needing it for Radiant systems
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine initializes the arrays associated with interior
    // surface convection.  The main parameter which is initialized
    // in this routine is HConvIn, the convection coefficient on the
    // inside surface.

    // METHODOLOGY EMPLOYED:
    // Determine the temperature difference between the surface and the
    // zone air for the last time step and then base the calculation
    // of the convection coefficient on that value and the surface tilt.

    // REFERENCES:
    // (I)BLAST legacy routine VARTMP
    // 1.  Passive Solar Extension of the BLAST Program
    //       Appendix E. p. 17,18
    // 2.  ASHRAE
    //       Simple Algorithm:    ASHRAE Handbook of Fundamentals 1985, p. 23.2, Table 1
    //       Detailed Algorithm:  ASHRAE Handbook of Fundamentals 2001, p. 3.12, Table 5
    // 3.  Walton, G. N. 1983. Thermal Analysis Research Program (TARP) Reference Manual,
    //     NBSSIR 83-2655, National Bureau of Standards, "Surface Inside Heat Balances", pp 79-80
    // 4.  Fisher, D.E. and C.O. Pedersen, Convective Heat Transfer in Building Energy and
    //       Thermal Load Calculations, ASHRAE Transactions, vol. 103, Pt. 2, 1997, p.137
    // 5.  ISO Standard 15099:2003e

    auto const &Zone = state.dataHeatBal->Zone;
    auto const &Surface = state.dataSurface->Surface;

    if (state.dataConvect->GetUserSuppliedConvectionCoeffs) {
        GetUserConvCoeffs(state);
        state.dataConvect->GetUserSuppliedConvectionCoeffs = false;
    }

    if (state.dataConvect->NodeCheck) { // done once when conditions are ready...
        if (!state.dataGlobal->SysSizingCalc && !state.dataGlobal->ZoneSizingCalc && state.dataZoneEquip->ZoneEquipInputsFilled &&
            allocated(state.dataLoopNodes->Node)) {
            state.dataConvect->NodeCheck = false;
            for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
                auto &zone = state.dataHeatBal->Zone(ZoneNum);
                if (zone.IntConvAlgo != HcInt::CeilingDiffuser) {
                    continue;
                }
                if (zone.SystemZoneNodeNumber != 0) {
                    continue;
                }
                ShowSevereError(
                    state,
                    format("InitInteriorConvectionCoeffs: Inside Convection=CeilingDiffuser, but no system inlet node defined, Zone={}", zone.Name));
                ShowContinueError(state, format("Defaulting inside convection to TARP. Check ZoneHVAC:EquipmentConnections for Zone={}", zone.Name));
                zone.IntConvAlgo = HcInt::ASHRAETARP;
            }
            // insert one-time setup for adaptive inside face
        }
    }

    if (state.dataConvect->ActiveSurfaceCheck && !state.dataGlobal->SysSizingCalc && !state.dataGlobal->ZoneSizingCalc &&
        state.dataZoneEquip->ZoneEquipSimulatedOnce) {
        SetupAdaptiveConvRadiantSurfaceData(state);
        state.dataConvect->ActiveSurfaceCheck = false;
    }

    if (state.dataGlobal->BeginEnvrnFlag && state.dataConvect->MyEnvirnFlag) {
        bool anyAdaptiveConvectionAlgorithm = false;
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            if (state.dataSurface->surfIntConv(SurfNum).model == HcInt::AdaptiveConvectionAlgorithm) {
                anyAdaptiveConvectionAlgorithm = true;
                break;
            }
        }
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            if (state.dataHeatBal->Zone(ZoneNum).IntConvAlgo == HcInt::AdaptiveConvectionAlgorithm) {
                anyAdaptiveConvectionAlgorithm = true;
                break;
            }
        }
        if (anyAdaptiveConvectionAlgorithm) {
            // need to clear out node conditions because dynamic assignments will be affected
            if (state.dataLoopNodes->NumOfNodes > 0 && allocated(state.dataLoopNodes->Node)) {
                for (auto &e : state.dataLoopNodes->Node) {
                    e.Temp = state.dataLoopNodes->DefaultNodeValues.Temp;
                    e.TempMin = state.dataLoopNodes->DefaultNodeValues.TempMin;
                    e.TempMax = state.dataLoopNodes->DefaultNodeValues.TempMax;
                    e.TempSetPoint = state.dataLoopNodes->DefaultNodeValues.TempSetPoint;
                    e.MassFlowRate = state.dataLoopNodes->DefaultNodeValues.MassFlowRate;
                    e.MassFlowRateMin = state.dataLoopNodes->DefaultNodeValues.MassFlowRateMin;
                    e.MassFlowRateMax = state.dataLoopNodes->DefaultNodeValues.MassFlowRateMax;
                    e.MassFlowRateMinAvail = state.dataLoopNodes->DefaultNodeValues.MassFlowRateMinAvail;
                    e.MassFlowRateMaxAvail = state.dataLoopNodes->DefaultNodeValues.MassFlowRateMaxAvail;
                    e.MassFlowRateSetPoint = state.dataLoopNodes->DefaultNodeValues.MassFlowRateSetPoint;
                    e.Quality = state.dataLoopNodes->DefaultNodeValues.Quality;
                    e.Press = state.dataLoopNodes->DefaultNodeValues.Press;
                    e.Enthalpy = state.dataLoopNodes->DefaultNodeValues.Enthalpy;
                    e.HumRat = state.dataLoopNodes->DefaultNodeValues.HumRat;
                    e.HumRatMin = state.dataLoopNodes->DefaultNodeValues.HumRatMin;
                    e.HumRatMax = state.dataLoopNodes->DefaultNodeValues.HumRatMax;
                    e.HumRatSetPoint = state.dataLoopNodes->DefaultNodeValues.HumRatSetPoint;
                    e.TempSetPointHi = state.dataLoopNodes->DefaultNodeValues.TempSetPointHi;
                    e.TempSetPointLo = state.dataLoopNodes->DefaultNodeValues.TempSetPointLo;
                }
                if (allocated(state.dataLoopNodes->MoreNodeInfo)) {
                    for (auto &e : state.dataLoopNodes->MoreNodeInfo) {
                        e.WetBulbTemp = state.dataLoopNodes->DefaultNodeValues.Temp;
                        e.RelHumidity = 0.0;
                        e.ReportEnthalpy = state.dataLoopNodes->DefaultNodeValues.Enthalpy;
                        e.VolFlowRateStdRho = 0.0;
                        e.VolFlowRateCrntRho = 0.0;
                        e.Density = 0.0;
                    }
                }
            }
        }
        state.dataConvect->MyEnvirnFlag = false;
    }

    if (!state.dataGlobal->BeginEnvrnFlag) {
        state.dataConvect->MyEnvirnFlag = true;
    }

    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        switch (Zone(ZoneNum).IntConvAlgo) {
        case HcInt::CeilingDiffuser:
            CalcCeilingDiffuserIntConvCoeff(state, ZoneNum, SurfaceTemperatures);
            break;
        case HcInt::TrombeWall:
            CalcTrombeWallIntConvCoeff(state, ZoneNum, SurfaceTemperatures);
            break;
        default:;
            // nothing
        }
    }

    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        auto const &zone = state.dataHeatBal->Zone(ZoneNum);
        for (int spaceNum : zone.spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            Real64 spaceMAT = state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum).MAT;
            for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {

                if (present(ZoneToResimulate)) {
                    if ((ZoneNum != ZoneToResimulate) && (state.dataSurface->SurfAdjacentZone(SurfNum) != ZoneToResimulate)) {
                        continue; // skip surfaces that are not associated with this zone
                    }
                }
                auto const &surface = Surface(SurfNum);
                if (state.dataSurface->UseRepresentativeSurfaceCalculations) {
                    int repSurfNum = surface.RepresentativeCalcSurfNum;
                    if (SurfNum != repSurfNum) {
                        continue;
                    }
                }

                HcInt intConvAlgo = state.dataSurface->surfIntConv(SurfNum).model;
                if (intConvAlgo == HcInt::SetByZone) {
                    intConvAlgo = zone.IntConvAlgo;
                }

                switch (intConvAlgo) {
                case HcInt::Value:
                case HcInt::Schedule:
                case HcInt::UserCurve: {
                    state.dataHeatBalSurf->SurfHConvInt(SurfNum) = SetIntConvCoeff(state, SurfNum);
                    // Establish some lower limit to avoid a zero convection coefficient (and potential divide by zero problems)
                    if (state.dataHeatBalSurf->SurfHConvInt(SurfNum) < state.dataHeatBal->LowHConvLimit) {
                        state.dataHeatBalSurf->SurfHConvInt(SurfNum) = state.dataHeatBal->LowHConvLimit;
                    }
                } break;

                case HcInt::ASHRAESimple: {
                    CalcASHRAESimpleIntConvCoeff(state, SurfNum, SurfaceTemperatures(SurfNum), spaceMAT);
                    // Establish some lower limit to avoid a zero convection coefficient (and potential divide by zero problems)
                    if (state.dataHeatBalSurf->SurfHConvInt(SurfNum) < state.dataHeatBal->LowHConvLimit) {
                        state.dataHeatBalSurf->SurfHConvInt(SurfNum) = state.dataHeatBal->LowHConvLimit;
                    }
                } break;

                case HcInt::ASHRAETARP: {
                    if (!state.dataConstruction->Construct(Surface(SurfNum).Construction).TypeIsWindow) {
                        CalcASHRAEDetailedIntConvCoeff(state, SurfNum, SurfaceTemperatures(SurfNum), spaceMAT);
                    } else {
                        CalcISO15099WindowIntConvCoeff(state, SurfNum, SurfaceTemperatures(SurfNum), spaceMAT);
                    }

                    // Establish some lower limit to avoid a zero convection coefficient (and potential divide by zero problems)
                    if (state.dataHeatBalSurf->SurfHConvInt(SurfNum) < state.dataHeatBal->LowHConvLimit) {
                        state.dataHeatBalSurf->SurfHConvInt(SurfNum) = state.dataHeatBal->LowHConvLimit;
                    }
                } break;

                case HcInt::AdaptiveConvectionAlgorithm: {
                    ManageIntAdaptiveConvAlgo(state, SurfNum);
                } break;

                case HcInt::CeilingDiffuser:
                case HcInt::TrombeWall: {
                    // Already done above and can't be at individual surface
                } break;

                case HcInt::ASTMC1340: {
                    CalcASTMC1340ConvCoeff(state, SurfNum, SurfaceTemperatures(SurfNum), spaceMAT);
                } break;

                default: {
                    ShowFatalError(state, "Unhandled convection coefficient algorithm."); // assert?
                } break;
                }

                if (state.dataSurface->SurfEMSOverrideIntConvCoef(SurfNum)) {
                    state.dataHeatBalSurf->SurfHConvInt(SurfNum) = state.dataSurface->SurfEMSValueForIntConvCoef(SurfNum);
                    if (Surface(SurfNum).ExtBoundCond == DataSurfaces::KivaFoundation) {
                        Real64 hConst = state.dataSurface->SurfEMSValueForIntConvCoef(SurfNum);
                        state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].in = KIVA_CONST_CONV(hConst);
                    }
                }

            } // for (surface)
        } // for (space)
    } // for (zone)

    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            for (int SurfNum = thisSpace.WindowSurfaceFirst; SurfNum <= thisSpace.WindowSurfaceLast; ++SurfNum) {
                auto const &surface(Surface(SurfNum));
                if (state.dataSurface->UseRepresentativeSurfaceCalculations) {
                    int repSurfNum = surface.RepresentativeCalcSurfNum;
                    if (SurfNum != repSurfNum) {
                        continue;
                    }
                }
                if (Surface(SurfNum).ExtBoundCond == ExternalEnvironment) {
                    state.dataHeatBalSurf->SurfHConvInt(SurfNum) =
                        state.dataHeatBalSurf->SurfHConvInt(SurfNum) * state.dataHeatBalSurf->SurfWinCoeffAdjRatio(SurfNum);
                }
            }
        }
    }
}

void InitExtConvCoeff(EnergyPlusData &state,
                      int const SurfNum,                          // Surface number (in Surface derived type)
                      Real64 const HMovInsul,                     // Equivalent convection coefficient of movable insulation
                      Material::SurfaceRoughness const Roughness, // Roughness index (1-6), see DataHeatBalance parameters
                      Real64 const AbsExt,                        // Exterior thermal absorptance
                      Real64 const TempExt,                       // Exterior surface temperature (C)
                      Real64 &HExt,                               // Convection coefficient to exterior air
                      Real64 &HSky,                               // "Convection" coefficient to sky temperature
                      Real64 &HGround,                            // "Convection" coefficient to ground temperature
                      Real64 &HAir,                               // Radiation to Air Component
                      Real64 &HSrdSurf                            // Radiation to surrounding surfaces
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         George Walton
    //       DATE WRITTEN   January 1990
    //       RE-ENGINEERED  Mar98 (RKS); Sep03 (LKL): Add additional flavors of Ext Convection Coeff.
    //                      Dec03 (PGE): Re-eng'd ASHRAEDetailed to match BLAST and TARP.
    //                      Aug04 (PGE): Corrected error for calculating local wind speeds for different terrains.
    //                      Aug 2010 B. Griffith.  for outside air convection, added new adaptive convection algorithm etc.

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine determines the outside convection coefficient for
    // a particular surface.

    // METHODOLOGY EMPLOYED:
    // Based on the properties of a particular surface, determine what the
    // outside convection coefficients are for outside air, the sky, and
    // the ground.  Convection coefficients for the sky and ground are
    // actually linearized radiation coefficients.  The ground surface is
    // assumed to be the same temperature as the outside air.

    // REFERENCES:
    // (I)BLAST legacy routine OCNVCO
    // TARP Reference Manual, "Surface Outside Heat Balances", pp 71ff

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 SurfWindSpeed;  // Local wind speed at height of the heat transfer surface (m/s)
    Real64 Hn;             // Natural part of exterior convection
    Real64 Hf;             // Forced part of exterior convection
    Real64 rCalcPerimeter; // approximation for Perimeter

    auto const &surface = state.dataSurface->Surface(SurfNum);

    if (state.dataConvect->GetUserSuppliedConvectionCoeffs) {
        GetUserConvCoeffs(state);
        state.dataConvect->GetUserSuppliedConvectionCoeffs = false;
    }

    Real64 TAir = state.dataSurface->SurfOutDryBulbTemp(SurfNum) + Constant::Kelvin;
    Real64 TSurf = TempExt + Constant::Kelvin;
    Real64 TSky = state.dataEnvrn->SkyTempKelvin;
    Real64 TGround = TAir;
    HSrdSurf = 0.0;

    if (surface.SurfHasSurroundingSurfProperty) {
        int SrdSurfsNum = surface.SurfSurroundingSurfacesNum;
        if (state.dataSurface->SurroundingSurfsProperty(SrdSurfsNum).skyTempSched != nullptr) {
            TSky = state.dataSurface->SurroundingSurfsProperty(SrdSurfsNum).skyTempSched->getCurrentVal() + Constant::Kelvin;
        }
        HSrdSurf = SurroundingSurfacesRadCoeffAverage(state, SurfNum, TSurf, AbsExt);
    }
    if (surface.UseSurfPropertyGndSurfTemp) {
        int gndSurfsNum = surface.SurfPropertyGndSurfIndex;
        TGround = state.dataSurface->GroundSurfsProperty(gndSurfsNum).SurfsTempAvg + Constant::Kelvin;
    }

    int BaseSurf = surface.BaseSurf; // If this is a base surface, BaseSurf = SurfNum

    Real64 SurfWindDir = state.dataSurface->SurfOutWindDir(SurfNum);

    if (!surface.ExtWind) {
        SurfWindSpeed = 0.0; // No wind exposure
    } else if (surface.Class == SurfaceClass::Window && state.dataSurface->SurfWinShadingFlag(SurfNum) == WinShadingType::ExtShade) {
        SurfWindSpeed = 0.0; // Assume zero wind speed at outside glass surface of window with exterior shade
    } else {
        SurfWindSpeed = state.dataSurface->SurfOutWindSpeed(SurfNum);
    }

    // Check if exterior is to be set by user
    HcExt extConvAlgo = state.dataSurface->surfExtConv(SurfNum).model;
    if (extConvAlgo == HcExt::SetByZone) {
        extConvAlgo = state.dataHeatBal->Zone(surface.Zone).ExtConvAlgo;
    }

    switch (extConvAlgo) {
    case HcExt::Value:
    case HcExt::Schedule:
    case HcExt::UserCurve: {
        HExt = SetExtConvCoeff(state, SurfNum);
    } break;

    case HcExt::ASHRAESimple: {
        if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].f = [](double, double, double, double windSpeed) -> double {
                return windSpeed;
            };
            state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].out = [=](double, double, double hfTerm, double, double) -> double {
                return CalcASHRAESimpExtConvCoeff(Roughness, hfTerm);
            };
        } else {
            HExt = CalcASHRAESimpExtConvCoeff(Roughness, SurfWindSpeed); // includes radiation to sky, ground, and air
        }
    } break;

    case HcExt::ASHRAETARP:
    case HcExt::BLASTHcOutside:
    case HcExt::TarpHcOutside: {
        //   Convection is split into forced and natural components. The total
        //   convective heat transfer coefficient is the sum of these components.
        //   Coefficients for subsurfaces are handled in a special way.  The values for perimeter and gross area
        //   are actually referencing the base surface because a subsurface does not initiate a completely new
        //   thermal boundary layer (although it may add some additional complexity that cannot be accounted for
        //   here).  The values for height (Z) and roughness do, however, come from the subsurface.
        //   BLAST algorithm has been replaced by this one since it was identical except for the standard wind
        //   speed measurement height which was only different because of unit conversions:  10 m vs. 30 ft (= 9.14 m).
        //   ASHRAE/BLAST REFERENCES:
        //   ?
        //   TARP REFERENCES:
        //   Walton, G. N.  1983.  Thermal Analysis Research Program Reference Manual.
        //   National Bureau of Standards.  NBSSIR 83-2655.

        // due to outlying calculations when perimeter is very small compared to area, use Perimeter
        // approximation calculation

        if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            if (surface.Class == SurfaceClass::Wall) {
                auto const &fnd = state.dataSurfaceGeometry->kivaManager.surfaceMap[SurfNum].get_instance(0).first->foundation;
                const double length = fnd.netPerimeter;
                const double height = fnd.wall.heightAboveGrade;
                const double area = length * height;
                const double perim = 2.0 * (length + height);
                state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].f = [=](double, double, double, double windSpeed) -> double {
                    // Average windward and leeward since all walls use same algorithm
                    double windwardHf = CalcSparrowWindward(Roughness, perim, area, windSpeed);
                    double leewardHf = CalcSparrowLeeward(Roughness, perim, area, windSpeed);
                    return (windwardHf + leewardHf) / 2.0;
                };
            } else { // Slab (used for exterior grade convection)
                // Assume very large area for grade (relative to perimeter).
                constexpr double area = 9999999.;
                constexpr double perim = 1.;
                state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].f = [=](double, double, double, double windSpeed) -> double {
                    return CalcSparrowWindward(Roughness, perim, area, windSpeed);
                };
            }
            state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].out =
                [=](double Tsurf, double Tamb, double hfTerm, double, double cosTilt) -> double {
                Real64 Ts = Tsurf;
                if (HMovInsul > 0.0) {
                    Ts = (HMovInsul * Tsurf + hfTerm * Tamb) / (HMovInsul + hfTerm);
                }
                return CalcASHRAETARPNatural(Ts, Tamb, cosTilt) + hfTerm;
            };
        } else {
            if (state.dataSurface->Surface(BaseSurf).GrossArea != 0.0 && state.dataSurface->Surface(BaseSurf).Height != 0.0) {
                rCalcPerimeter = 2.0 * (state.dataSurface->Surface(BaseSurf).GrossArea / state.dataSurface->Surface(BaseSurf).Height +
                                        state.dataSurface->Surface(BaseSurf).Height);
                Hf = CalcHfExteriorSparrow(SurfWindSpeed,
                                           state.dataSurface->Surface(BaseSurf).GrossArea,
                                           rCalcPerimeter,
                                           surface.CosTilt,
                                           surface.Azimuth,
                                           Roughness,
                                           SurfWindDir);
            } else {
                Hf = 0.0;
            }
            if (HMovInsul > 0.0) {
                TSurf = (HMovInsul * TSurf + Hf * TAir) / (HMovInsul + Hf);
            }
            Hn = CalcASHRAETARPNatural(TSurf, TAir, surface.CosTilt);
            HExt = Hn + Hf;
        }
    } break;

    case HcExt::MoWiTTHcOutside: {
        if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            if (surface.Class == SurfaceClass::Wall) {
                state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].f = [=](double, double, double, double windSpeed) -> double {
                    // Average windward and leeward since all walls use same algorithm
                    double windwardHf = CalcMoWITTForcedWindward(windSpeed);
                    double leewardHf = CalcMoWITTForcedLeeward(windSpeed);
                    return (windwardHf + leewardHf) / 2.0;
                };
            } else {
                state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].f = [=](double, double, double, double windSpeed) -> double {
                    return CalcMoWITTForcedWindward(windSpeed);
                };
            }
            state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].out =
                [=](double Tsurf, double Tamb, double hfTerm, double, double) -> double {
                Real64 Hn = CalcMoWITTNatural(Tsurf - Tamb);
                return std::sqrt(pow_2(Hn) + pow_2(hfTerm));
            };
        } else {
            // NOTE: Movable insulation is not taken into account here
            if (Windward(surface.CosTilt, surface.Azimuth, SurfWindDir)) {
                HExt = CalcMoWITTWindward(TAir - TSurf, SurfWindSpeed);
            } else { // leeward
                HExt = CalcMoWITTLeeward(TAir - TSurf, SurfWindSpeed);
            }
        }
    } break;

    case HcExt::DOE2HcOutside: {
        if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            if (surface.Class == SurfaceClass::Wall) {
                state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].f = [=](double, double, double, double windSpeed) -> double {
                    // Average windward and leeward since all walls use same algorithm
                    double windwardHf = CalcMoWITTForcedWindward(windSpeed);
                    double leewardHf = CalcMoWITTForcedLeeward(windSpeed);
                    return (windwardHf + leewardHf) / 2.0;
                };
            } else {
                state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].f = [=](double, double, double, double windSpeed) -> double {
                    return CalcMoWITTForcedWindward(windSpeed);
                };
            }
            state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].out =
                [=](double Tsurf, double Tamb, double hfTerm, double, double cosTilt) -> double {
                Real64 Hf = CalcDOE2Forced(Tsurf, Tamb, cosTilt, hfTerm, Roughness);
                Real64 Ts = Tsurf;
                if (HMovInsul > 0.0) {
                    Ts = (HMovInsul * TSurf + Hf * Tamb) / (HMovInsul + Hf);
                }

                Real64 Hn = CalcASHRAETARPNatural(Ts, Tamb, cosTilt);
                return Hn + Hf;
            };
        } else {
            if (Windward(surface.CosTilt, surface.Azimuth, SurfWindDir)) {
                Hf = CalcDOE2Windward(TSurf, TAir, surface.CosTilt, SurfWindSpeed, Roughness);
            } else { // leeward
                Hf = CalcDOE2Leeward(TSurf, TAir, surface.CosTilt, SurfWindSpeed, Roughness);
            }
            if (HMovInsul > 0.0) {
                TSurf = (HMovInsul * TSurf + Hf * TAir) / (HMovInsul + Hf);
            }

            Hn = CalcASHRAETARPNatural(TSurf, TAir, surface.CosTilt);
            // Better if there was iteration for movable insulation?
            HExt = Hn + Hf;
        }
    } break;

    case HcExt::AdaptiveConvectionAlgorithm: {
        HExt = ManageExtAdaptiveConvAlgo(state, SurfNum);
    } break;

    default: {
        ShowFatalError(state, format("InitExtConvection Coefficients: invalid parameter -- outside convection type, Surface={}", surface.Name));
    } break;
    }

    if (state.dataSurface->SurfEMSOverrideExtConvCoef(SurfNum)) {
        HExt = state.dataSurface->SurfEMSValueForExtConvCoef(SurfNum);
        if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].f = KIVA_HF_ZERO;
            Real64 hConst = state.dataSurface->SurfEMSValueForExtConvCoef(SurfNum);
            state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].out = KIVA_CONST_CONV(hConst);
        }
    }

    HExt = HExt * state.dataHeatBalSurf->SurfWinCoeffAdjRatio(SurfNum);

    if (TSurf == TSky || extConvAlgo == HcExt::ASHRAESimple) {
        HSky = 0.0;
    } else {
        // Compute sky radiation coefficient
        HSky = Constant::StefanBoltzmann * AbsExt * surface.ViewFactorSkyIR * state.dataSurface->SurfAirSkyRadSplit(SurfNum) *
               (pow_4(TSurf) - pow_4(TSky)) / (TSurf - TSky);
    }

    if (TSurf == TAir || extConvAlgo == HcExt::ASHRAESimple) {
        HGround = 0.0;
        HAir = 0.0;
    } else {
        // Compute ground radiation coefficient
        HGround = Constant::StefanBoltzmann * AbsExt * surface.ViewFactorGroundIR * (pow_4(TSurf) - pow_4(TGround)) / (TSurf - TGround);

        // Compute air radiation coefficient
        HAir = Constant::StefanBoltzmann * AbsExt * surface.ViewFactorSkyIR * (1.0 - state.dataSurface->SurfAirSkyRadSplit(SurfNum)) *
               (pow_4(TSurf) - pow_4(TAir)) / (TSurf - TAir);
    }
}

Real64 CalcHfExteriorSparrow(Real64 const SurfWindSpeed,                 // Local wind speed at height of the heat transfer surface (m/s)
                             Real64 const GrossArea,                     // Gross surface area {m2}
                             Real64 const Perimeter,                     // Surface perimeter length {m}
                             Real64 const CosTilt,                       // Cosine of the Surface Tilt Angle
                             Real64 const Azimuth,                       // Facing angle (degrees) of the surface outward normal
                             Material::SurfaceRoughness const Roughness, // Surface roughness index (6=very smooth, 5=smooth, 4=medium smooth,
                             Real64 const WindDirection                  // Wind (compass) direction (degrees)
)
{
    if (Windward(CosTilt, Azimuth, WindDirection)) {
        return CalcSparrowWindward(Roughness, Perimeter, GrossArea, SurfWindSpeed);
    } else {
        return CalcSparrowLeeward(Roughness, Perimeter, GrossArea, SurfWindSpeed);
    }
}

bool Windward(Real64 const CosTilt,      // Cosine of the surface tilt angle
              Real64 const Azimuth,      // or Facing, Direction the surface outward normal faces (degrees)
              Real64 const WindDirection // Wind direction measured clockwise from geographic North
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   September 2003

    // PURPOSE OF THIS FUNCTION:
    // This function determines if a surface is "windward" or "leeward" (that is,
    // into / against the wind (true) or in shelter from wind (false).

    // METHODOLOGY EMPLOYED:
    // Leeward is defined as greater than 100 degrees from normal incidence.
    // Note that a sufficiently horizontal surface is always considered windward.

    // REFERENCES:
    //   Walton, G. N.  1981.  Passive solar extension of the Building Loads
    //   Analysis and System Thermodynamics (BLAST) program.  Technical Report,
    //   United States Army Construction Engineering Research Laboratory,
    //   Champaign, IL.

    // Surface is horizontal
    if (std::abs(CosTilt) >= 0.98) {
        return true;
    }

    Real64 Diff = std::abs(WindDirection - Azimuth); // Difference between the wind direction and the surface azimuth
    if ((Diff - 180.0) > 0.001) {
        Diff -= 360.0;
    }
    return ((std::abs(Diff) - 90.0) <= 0.001);
}

void GetUserConvCoeffs(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   February 2003
    //       MODIFIED       November 2004; add more "user supplied convection coefficients"

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine gets the input for the object "Convection Coefficients" which
    // can be specified by a user to override the "normally" calculated convection coefficients.  The
    // change (November 2004) allows the user to specify down to the "surface level" the
    // exterior or interior algorithm to be used.

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("GetUserConvectionCoefficients");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Array1D_string Alphas(9);
    Array1D<Real64> Numbers(2);
    int NumAlphas;
    int NumNumbers;
    int Status;
    bool ErrorsFound(false);
    std::string CurrentModuleObject;

    auto &Zone = state.dataHeatBal->Zone;
    auto &Surface = state.dataSurface->Surface;

    auto &ipsc = state.dataIPShortCut;

    // first get user-defined H models so they can be processed for later objects
    CurrentModuleObject = "SurfaceConvectionAlgorithm:Inside:UserCurve";
    int TotHcIntUserCurves = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
    state.dataConvect->hcIntUserCurve.allocate(TotHcIntUserCurves);
    for (int Loop = 1; Loop <= TotHcIntUserCurves; ++Loop) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 Loop,
                                                                 ipsc->cAlphaArgs,
                                                                 NumAlphas,
                                                                 ipsc->rNumericArgs,
                                                                 NumNumbers,
                                                                 Status,
                                                                 ipsc->lNumericFieldBlanks,
                                                                 ipsc->lAlphaFieldBlanks,
                                                                 ipsc->cAlphaFieldNames,
                                                                 ipsc->cNumericFieldNames);
        auto &intConvUserCurve = state.dataConvect->hcIntUserCurve(Loop);
        intConvUserCurve.Name = ipsc->cAlphaArgs(1);

        ErrorObjectHeader eoh{RoutineName, CurrentModuleObject, intConvUserCurve.Name};
        intConvUserCurve.refTempType = static_cast<RefTemp>(getEnumValue(RefTempNamesUC, ipsc->cAlphaArgs(2)));
        if (intConvUserCurve.refTempType == RefTemp::Invalid) {
            ShowSevereInvalidKey(state, eoh, ipsc->cAlphaFieldNames(2), ipsc->cAlphaArgs(2));
            ErrorsFound = true;
        }

        if (!ipsc->lAlphaFieldBlanks(3)) {
            intConvUserCurve.hcFnTempDiffCurveNum = Curve::GetCurveIndex(state, ipsc->cAlphaArgs(3));
            if (intConvUserCurve.hcFnTempDiffCurveNum == 0) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(3), ipsc->cAlphaArgs(3));
                ErrorsFound = true;
            } else { // check type
                auto const *curve = state.dataCurveManager->curves(intConvUserCurve.hcFnTempDiffCurveNum);
                if (curve->numDims != 1) {
                    ErrorsFound = true;
                    Curve::ShowSevereCurveDims(state, eoh, ipsc->cAlphaFieldNames(3), curve->Name, "1", curve->numDims);
                }
            }
        } else {
            intConvUserCurve.hcFnTempDiffCurveNum = 0;
        }

        if (!ipsc->lAlphaFieldBlanks(4)) {
            intConvUserCurve.hcFnTempDiffDivHeightCurveNum = Curve::GetCurveIndex(state, ipsc->cAlphaArgs(4));
            if (intConvUserCurve.hcFnTempDiffDivHeightCurveNum == 0) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(4), ipsc->cAlphaArgs(4));
                ErrorsFound = true;
            } else { // check type
                auto const *curve = state.dataCurveManager->curves(intConvUserCurve.hcFnTempDiffDivHeightCurveNum);
                if (curve->numDims != 1) {
                    ErrorsFound = true;
                    Curve::ShowSevereCurveDims(state, eoh, ipsc->cAlphaFieldNames(4), curve->Name, "1", curve->numDims);
                }
            }
        } else {
            intConvUserCurve.hcFnTempDiffDivHeightCurveNum = 0;
        }

        if (!ipsc->lAlphaFieldBlanks(5)) {
            intConvUserCurve.hcFnACHCurveNum = Curve::GetCurveIndex(state, ipsc->cAlphaArgs(5));
            if (intConvUserCurve.hcFnACHCurveNum == 0) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(5), ipsc->cAlphaArgs(5));
                ErrorsFound = true;
            } else { // check type
                auto const *curve = state.dataCurveManager->curves(intConvUserCurve.hcFnACHCurveNum);
                if (curve->numDims != 1) {
                    ErrorsFound = true;
                    Curve::ShowSevereCurveDims(state, eoh, ipsc->cAlphaFieldNames(5), curve->Name, "1", curve->numDims);
                }
            }
        } else {
            intConvUserCurve.hcFnACHCurveNum = 0;
        }

        if (!ipsc->lAlphaFieldBlanks(6)) {
            intConvUserCurve.hcFnACHDivPerimLengthCurveNum = Curve::GetCurveIndex(state, ipsc->cAlphaArgs(6));
            if (intConvUserCurve.hcFnACHDivPerimLengthCurveNum == 0) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(6), ipsc->cAlphaArgs(6));
                ErrorsFound = true;
            } else { // check type
                auto const *curve = state.dataCurveManager->curves(intConvUserCurve.hcFnACHDivPerimLengthCurveNum);
                if (curve->numDims != 1) {
                    ErrorsFound = true;
                    Curve::ShowSevereCurveDims(state, eoh, ipsc->cAlphaFieldNames(6), curve->Name, "1", curve->numDims);
                }
            }
        } else {
            intConvUserCurve.hcFnACHDivPerimLengthCurveNum = 0;
        }

    } // end of 'SurfaceConvectionAlgorithm:Inside:UserCurve'

    CurrentModuleObject = "SurfaceConvectionAlgorithm:Outside:UserCurve";
    int TotOutsideHcUserCurves = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
    state.dataConvect->hcExtUserCurve.allocate(TotOutsideHcUserCurves);
    for (int Loop = 1; Loop <= TotOutsideHcUserCurves; ++Loop) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 Loop,
                                                                 ipsc->cAlphaArgs,
                                                                 NumAlphas,
                                                                 ipsc->rNumericArgs,
                                                                 NumNumbers,
                                                                 Status,
                                                                 ipsc->lNumericFieldBlanks,
                                                                 ipsc->lAlphaFieldBlanks,
                                                                 ipsc->cAlphaFieldNames,
                                                                 ipsc->cNumericFieldNames);

        auto &extConvUserCurve = state.dataConvect->hcExtUserCurve(Loop);

        extConvUserCurve.Name = ipsc->cAlphaArgs(1);

        ErrorObjectHeader eoh{RoutineName, CurrentModuleObject, extConvUserCurve.Name};
        extConvUserCurve.windSpeedType = static_cast<RefWind>(getEnumValue(RefWindNamesUC, Util::makeUPPER(ipsc->cAlphaArgs(2))));
        if (extConvUserCurve.windSpeedType == RefWind::Invalid) {
            ShowSevereInvalidKey(state, eoh, ipsc->cAlphaFieldNames(2), ipsc->cAlphaArgs(2));
            ErrorsFound = true;
        }

        // A3 , \field Hf Function of Wind Speed Curve Name
        if (!ipsc->lAlphaFieldBlanks(3)) {
            extConvUserCurve.hfFnWindSpeedCurveNum = Curve::GetCurveIndex(state, ipsc->cAlphaArgs(3));
            if (extConvUserCurve.hfFnWindSpeedCurveNum == 0) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(3), ipsc->cAlphaArgs(3));
                ErrorsFound = true;
            } else { // check type
                auto const *curve = state.dataCurveManager->curves(extConvUserCurve.hfFnWindSpeedCurveNum);
                if (curve->numDims != 1) {
                    ErrorsFound = true;
                    Curve::ShowSevereCurveDims(state, eoh, ipsc->cAlphaFieldNames(3), curve->Name, "1", curve->numDims);
                }
            }
        } else {
            extConvUserCurve.hfFnWindSpeedCurveNum = 0;
        }

        //  A4 , \field Hn Function of Temperature Difference Curve Name
        if (!ipsc->lAlphaFieldBlanks(4)) {
            extConvUserCurve.hnFnTempDiffCurveNum = Curve::GetCurveIndex(state, ipsc->cAlphaArgs(4));
            if (extConvUserCurve.hnFnTempDiffCurveNum == 0) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(4), ipsc->cAlphaArgs(4));
                ErrorsFound = true;
            } else { // check type
                auto const *curve = state.dataCurveManager->curves(extConvUserCurve.hnFnTempDiffCurveNum);
                if (curve->numDims != 1) {
                    ErrorsFound = true;
                    Curve::ShowSevereCurveDims(state, eoh, ipsc->cAlphaFieldNames(4), curve->Name, "1", curve->numDims);
                }
            }
        } else {
            extConvUserCurve.hnFnTempDiffCurveNum = 0;
        }

        //  A5 , \field Hn Function of Temperature Difference Divided by Height Curve Name
        if (!ipsc->lAlphaFieldBlanks(5)) {
            extConvUserCurve.hnFnTempDiffDivHeightCurveNum = Curve::GetCurveIndex(state, ipsc->cAlphaArgs(5));
            if (extConvUserCurve.hnFnTempDiffDivHeightCurveNum == 0) {
                ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(5), ipsc->cAlphaArgs(5));
                ErrorsFound = true;
            } else { // check type
                auto const *curve = state.dataCurveManager->curves(extConvUserCurve.hnFnTempDiffDivHeightCurveNum);
                if (curve->numDims != 1) {
                    ErrorsFound = true;
                    Curve::ShowSevereCurveDims(state, eoh, ipsc->cAlphaFieldNames(5), curve->Name, "1", curve->numDims);
                }
            }
        } else {
            extConvUserCurve.hnFnTempDiffDivHeightCurveNum = 0;
        }

    } // 'SurfaceConvectionAlgorithm:Outside:UserCurve'

    // now get user directed overrides at the surface level.
    state.dataSurface->TotUserIntConvModels = 0;
    state.dataSurface->TotUserExtConvModels = 0;
    CurrentModuleObject = "SurfaceProperty:ConvectionCoefficients:MultipleSurface";
    int Count = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
    for (int Loop = 1; Loop <= Count; ++Loop) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 Loop,
                                                                 Alphas,
                                                                 NumAlphas,
                                                                 Numbers,
                                                                 NumNumbers,
                                                                 Status,
                                                                 ipsc->lNumericFieldBlanks,
                                                                 ipsc->lAlphaFieldBlanks,
                                                                 ipsc->cAlphaFieldNames,
                                                                 ipsc->cNumericFieldNames);
        if (Alphas(2) == "INSIDE") {
            ++state.dataSurface->TotUserIntConvModels;
        } else if (Alphas(2) == "OUTSIDE") {
            ++state.dataSurface->TotUserExtConvModels;
        }

        if (Alphas(6) == "INSIDE") {
            ++state.dataSurface->TotUserIntConvModels;
        } else if (Alphas(6) == "OUTSIDE") {
            ++state.dataSurface->TotUserExtConvModels;
        }
        if (NumAlphas >= 2 && ipsc->lAlphaFieldBlanks(2)) {
            ShowWarningError(state,
                             format("GetUserConvectionCoefficients: {}, for {}={}", CurrentModuleObject, ipsc->cAlphaFieldNames(1), Alphas(1)));
            ShowContinueError(state, format("{} is blank and rest of fields will not be processed.", ipsc->cAlphaFieldNames(2)));
        }
        if (NumAlphas >= 6 && ipsc->lAlphaFieldBlanks(6)) {
            ShowWarningError(state,
                             format("GetUserConvectionCoefficients: {}, for {}={}", CurrentModuleObject, ipsc->cAlphaFieldNames(1), Alphas(1)));
            ShowContinueError(state, format("{} is blank and rest of fields will not be processed.", ipsc->cAlphaFieldNames(6)));
        }
    }
    CurrentModuleObject = "SurfaceProperty:ConvectionCoefficients";
    Count = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
    for (int Loop = 1; Loop <= Count; ++Loop) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 Loop,
                                                                 Alphas,
                                                                 NumAlphas,
                                                                 Numbers,
                                                                 NumNumbers,
                                                                 Status,
                                                                 ipsc->lNumericFieldBlanks,
                                                                 ipsc->lAlphaFieldBlanks,
                                                                 ipsc->cAlphaFieldNames,
                                                                 ipsc->cNumericFieldNames);
        if (Alphas(2) == "INSIDE") {
            ++state.dataSurface->TotUserIntConvModels;
        } else if (Alphas(2) == "OUTSIDE") {
            ++state.dataSurface->TotUserExtConvModels;
        }

        if (Alphas(6) == "INSIDE") {
            ++state.dataSurface->TotUserIntConvModels;
        } else if (Alphas(6) == "OUTSIDE") {
            ++state.dataSurface->TotUserExtConvModels;
        }
        if (NumAlphas >= 2 && ipsc->lAlphaFieldBlanks(2)) {
            ShowWarningError(state,
                             format("GetUserConvectionCoefficients: {}, for {}={}", CurrentModuleObject, ipsc->cAlphaFieldNames(1), Alphas(1)));
            ShowContinueError(state, format("{} is blank and rest of fields will not be processed.", ipsc->cAlphaFieldNames(2)));
        }
        if (NumAlphas >= 6 && ipsc->lAlphaFieldBlanks(6)) {
            ShowWarningError(state,
                             format("GetUserConvectionCoefficients: {}, for {}={}", CurrentModuleObject, ipsc->cAlphaFieldNames(1), Alphas(1)));
            ShowContinueError(state, format("{} is blank and rest of fields will not be processed.", ipsc->cAlphaFieldNames(6)));
        }
    }

    state.dataSurface->userIntConvModels.allocate(state.dataSurface->TotUserIntConvModels);
    state.dataSurface->userExtConvModels.allocate(state.dataSurface->TotUserExtConvModels);

    state.dataSurface->TotUserIntConvModels = 0;
    state.dataSurface->TotUserExtConvModels = 0;

    //   Now, get for real and check for consistency
    CurrentModuleObject = "SurfaceProperty:ConvectionCoefficients";
    Count = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
    for (int Loop = 1; Loop <= Count; ++Loop) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 Loop,
                                                                 Alphas,
                                                                 NumAlphas,
                                                                 Numbers,
                                                                 NumNumbers,
                                                                 Status,
                                                                 ipsc->lNumericFieldBlanks,
                                                                 ipsc->lAlphaFieldBlanks,
                                                                 ipsc->cAlphaFieldNames,
                                                                 ipsc->cNumericFieldNames);

        ErrorObjectHeader eoh{RoutineName, CurrentModuleObject, ""};
        int surfNum = Util::FindItemInList(Alphas(1), Surface);

        if (surfNum == 0) {
            ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(1), Alphas(1));
            ErrorsFound = true;
            continue;
        }

        for (int Pass = 1, Ptr = 2, FieldNo = 2, NumField = 1; Pass <= 2; ++Pass, Ptr += 4, FieldNo += 4, ++NumField) {

            if (Alphas(Ptr).empty()) {
                continue;
            }

            if (Alphas(Ptr) == "OUTSIDE") {
                if (Surface(surfNum).OSCPtr > 0) {
                    ShowSevereError(
                        state,
                        format("GetUserSuppliedConvectionCoefficients: {}, OUTSIDE {} cannot be specified for OtherSideCoefficient Surface={}",
                               CurrentModuleObject,
                               CurrentModuleObject,
                               Alphas(1)));
                    ErrorsFound = true;
                }

                HcExt hcExt = static_cast<HcExt>(getEnumValue(HcExtNamesUC, Alphas(Ptr + 1)));

                switch (hcExt) {

                case HcExt::ASHRAESimpleCombined:
                case HcExt::TarpHcOutside:
                case HcExt::MoWiTTHcOutside:
                case HcExt::DOE2HcOutside:
                case HcExt::AdaptiveConvectionAlgorithm: {
                    ApplyExtConvValue(state, surfNum, hcExt, 0);
                } break;

                case HcExt::Value: {
                    ++state.dataSurface->TotUserExtConvModels;
                    auto &userExtConvModel = state.dataSurface->userExtConvModels(state.dataSurface->TotUserExtConvModels);
                    userExtConvModel.SurfaceName = Alphas(1);
                    userExtConvModel.WhichSurface = surfNum;
                    if (Numbers(NumField) < state.dataHeatBal->LowHConvLimit || Numbers(NumField) > state.dataHeatBal->HighHConvLimit) {
                        ShowSevereError(state, format("{}{}=\"{}, out of range value", RoutineName, CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state,
                                          format("{}={}, {}=[{:.5R}].",
                                                 ipsc->cAlphaFieldNames(Ptr),
                                                 Alphas(Ptr),
                                                 ipsc->cNumericFieldNames(NumField),
                                                 Numbers(NumField)));
                        ShowContinueError(state,
                                          format("Out-of-range from low/high limits=[>={:.9R}, <={:.1R}].",
                                                 state.dataHeatBal->LowHConvLimit,
                                                 state.dataHeatBal->HighHConvLimit));
                        ShowContinueError(state, "Limits are set (or default) in HeatBalanceAlgorithm object.");
                        ErrorsFound = true;
                    }
                    userExtConvModel.overrideType = OverrideType::Value;
                    userExtConvModel.OverrideValue = Numbers(NumField);
                    if (!ipsc->lAlphaFieldBlanks(Ptr + 2)) {
                        ShowWarningError(state, format("{}{}=\"{}, duplicate value", RoutineName, CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state,
                                          format("Since VALUE is used for \"{}\", {}={} is ignored.",
                                                 ipsc->cAlphaFieldNames(FieldNo + 2),
                                                 ipsc->cAlphaFieldNames(Ptr + 2),
                                                 Alphas(Ptr + 2)));
                    }
                    ApplyExtConvValue(state, surfNum, hcExt, state.dataSurface->TotUserExtConvModels);
                } break;

                case HcExt::Schedule: { // Schedule
                    ++state.dataSurface->TotUserExtConvModels;
                    auto &userExtConvModel = state.dataSurface->userExtConvModels(state.dataSurface->TotUserExtConvModels);
                    userExtConvModel.SurfaceName = Alphas(1);
                    userExtConvModel.WhichSurface = surfNum;
                    userExtConvModel.overrideType = OverrideType::Schedule;
                    if ((userExtConvModel.sched = Sched::GetSchedule(state, Alphas(Ptr + 2))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(Ptr + 2), Alphas(Ptr + 2));
                        ErrorsFound = true;
                    } else if (!userExtConvModel.sched->checkMinMaxVals(
                                   state, Clusive::In, state.dataHeatBal->LowHConvLimit, Clusive::In, state.dataHeatBal->HighHConvLimit)) {
                        Sched::ShowSevereBadMinMax(state,
                                                   eoh,
                                                   ipsc->cAlphaFieldNames(Ptr + 2),
                                                   Alphas(Ptr + 2),
                                                   Clusive::In,
                                                   state.dataHeatBal->LowHConvLimit,
                                                   Clusive::In,
                                                   state.dataHeatBal->HighHConvLimit,
                                                   "Limits are set (or default) in HeatBalanceAlgorithm object.");
                        ErrorsFound = true;
                    }
                    ApplyExtConvValue(state, surfNum, hcExt, state.dataSurface->TotUserExtConvModels);
                } break;

                case HcExt::UserCurve: { // User curve
                    ++state.dataSurface->TotUserExtConvModels;
                    auto &userExtConvModel = state.dataSurface->userExtConvModels(state.dataSurface->TotUserExtConvModels);
                    userExtConvModel.SurfaceName = Alphas(1);
                    userExtConvModel.WhichSurface = surfNum;
                    userExtConvModel.overrideType = OverrideType::UserCurve;
                    userExtConvModel.UserCurveIndex = Util::FindItemInList(Alphas(Ptr + 3), state.dataConvect->hcExtUserCurve);
                    if (userExtConvModel.UserCurveIndex == 0) {
                        ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(Ptr + 3), Alphas(Ptr + 3));
                        ErrorsFound = true;
                    }
                    ApplyExtConvValue(state, surfNum, hcExt, state.dataSurface->TotUserExtConvModels);
                } break;

                case HcExt::UserValue: // Unhandled cases < HcExt::UserCurve
                case HcExt::UserSchedule:
                case HcExt::SetByZone:
                case HcExt::ASHRAETARP:
                case HcExt::BLASTHcOutside:
                case HcExt::None: {
                    ShowSevereError(state, format("{}{}=\"{}, check input", RoutineName, CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("Check Input Entered :{}", Alphas(Ptr + 1)));
                    ErrorsFound = true;
                } break;

                default: { // ExtValue > HcExt::UserCurve
                    // specificmodel
                    ++state.dataSurface->TotUserExtConvModels;
                    auto &userExtConvModel = state.dataSurface->userExtConvModels(state.dataSurface->TotUserExtConvModels);
                    userExtConvModel.SurfaceName = Alphas(1);
                    userExtConvModel.WhichSurface = surfNum;
                    userExtConvModel.overrideType = OverrideType::SpecifiedModel;
                    userExtConvModel.HcExtModelEq = hcExt;
                    ApplyExtConvValue(state, surfNum, hcExt, state.dataSurface->TotUserExtConvModels);
                } break;
                } // switch (ExtValue)

            } else if (Alphas(Ptr) == "INSIDE") {

                if (state.dataSurface->surfIntConv(surfNum).userModelNum != 0) {
                    ShowSevereError(state, format("{}{}=\"{}, duplicate (inside)", RoutineName, CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, "Duplicate (Inside) assignment attempt.");
                    ErrorsFound = true;
                    continue;
                }

                HcInt hcInt = static_cast<HcInt>(getEnumValue(HcIntNamesUC, Alphas(Ptr + 1)));

                switch (hcInt) {
                // Are these not used anymore? They can be deleted then
                case HcInt::UserValue:
                case HcInt::UserSchedule:
                case HcInt::SetByZone: {
                    ShowSevereError(state, format("{}{}=\"{}, invalid value", RoutineName, CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("Invalid Value Entered, for {}={}", ipsc->cAlphaFieldNames(Ptr), Alphas(Ptr)));
                    ShowContinueError(state, format("invalid value in {}={}", ipsc->cAlphaFieldNames(Ptr + 1), Alphas(Ptr + 1)));
                    ErrorsFound = true;
                } break;

                case HcInt::CeilingDiffuser:
                case HcInt::TrombeWall: {
                    ShowSevereError(state, format("{}{}=\"{}, invalid value", RoutineName, CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("Invalid Value Entered, for {}={}", ipsc->cAlphaFieldNames(Ptr), Alphas(Ptr)));
                    ShowContinueError(state,
                                      format("invalid value in {}={}\". This type is only applicable at a Zone level.",
                                             ipsc->cAlphaFieldNames(Ptr + 1),
                                             Alphas(Ptr + 1)));
                    ErrorsFound = true;
                } break;

                case HcInt::ASHRAESimple:
                case HcInt::ASHRAETARP:
                case HcInt::AdaptiveConvectionAlgorithm:
                case HcInt::ASTMC1340: {
                    ApplyIntConvValue(state, surfNum, hcInt, 0);
                } break;

                case HcInt::Value: {
                    ++state.dataSurface->TotUserIntConvModels;
                    auto &userIntConvModel = state.dataSurface->userIntConvModels(state.dataSurface->TotUserIntConvModels);
                    userIntConvModel.SurfaceName = Alphas(1);
                    userIntConvModel.WhichSurface = surfNum;
                    if (Numbers(NumField) < state.dataHeatBal->LowHConvLimit || Numbers(NumField) > state.dataHeatBal->HighHConvLimit) {
                        ShowSevereBadMinMax(state,
                                            eoh,
                                            ipsc->cNumericFieldNames(NumField),
                                            Numbers(NumField),
                                            Clusive::In,
                                            state.dataHeatBal->LowHConvLimit,
                                            Clusive::In,
                                            state.dataHeatBal->HighHConvLimit,
                                            "Limits are set (or default) in HeatBalanceAlgorithm object.");
                        ErrorsFound = true;
                    }
                    userIntConvModel.overrideType = OverrideType::Value;
                    userIntConvModel.OverrideValue = Numbers(NumField);
                    if (!ipsc->lAlphaFieldBlanks(Ptr + 2)) {
                        ShowWarningError(state, format("{}{}=\"{}, duplicate value", RoutineName, CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state,
                                          format("Since VALUE is used for \"{}\", {}={} is ignored.",
                                                 ipsc->cAlphaFieldNames(FieldNo + 1),
                                                 ipsc->cAlphaFieldNames(Ptr + 2),
                                                 Alphas(Ptr + 2)));
                    }
                    ApplyIntConvValue(state, surfNum, hcInt, state.dataSurface->TotUserIntConvModels);
                } break;

                case HcInt::Schedule: {
                    ++state.dataSurface->TotUserIntConvModels;
                    auto &userIntConvModel = state.dataSurface->userIntConvModels(state.dataSurface->TotUserIntConvModels);
                    userIntConvModel.SurfaceName = Alphas(1);
                    userIntConvModel.WhichSurface = surfNum;
                    userIntConvModel.overrideType = OverrideType::Schedule;
                    if ((userIntConvModel.sched = Sched::GetSchedule(state, Alphas(Ptr + 2))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(Ptr + 2), Alphas(Ptr + 2));
                        ErrorsFound = true;
                    } else if (!userIntConvModel.sched->checkMinMaxVals(
                                   state, Clusive::In, state.dataHeatBal->LowHConvLimit, Clusive::In, state.dataHeatBal->HighHConvLimit)) {
                        Sched::ShowSevereBadMinMax(state,
                                                   eoh,
                                                   ipsc->cAlphaFieldNames(Ptr + 2),
                                                   Alphas(Ptr + 2),
                                                   Clusive::In,
                                                   state.dataHeatBal->LowHConvLimit,
                                                   Clusive::In,
                                                   state.dataHeatBal->HighHConvLimit,
                                                   "Limits are set (or default) in HeatBalanceAlgorithm object.");
                        ErrorsFound = true;
                    }
                    ApplyIntConvValue(state, surfNum, hcInt, state.dataSurface->TotUserIntConvModels);
                } break;

                case HcInt::UserCurve: {
                    ++state.dataSurface->TotUserIntConvModels;
                    auto &userIntConvModel = state.dataSurface->userIntConvModels(state.dataSurface->TotUserIntConvModels);
                    userIntConvModel.SurfaceName = Alphas(1);
                    userIntConvModel.WhichSurface = surfNum;
                    userIntConvModel.overrideType = OverrideType::UserCurve;
                    userIntConvModel.UserCurveIndex = Util::FindItemInList(Alphas(Ptr + 3), state.dataConvect->hcIntUserCurve);
                    if (userIntConvModel.UserCurveIndex == 0) {
                        ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(Ptr + 3), Alphas(Ptr + 3));
                        ErrorsFound = true;
                    }
                    ApplyIntConvValue(state, surfNum, hcInt, state.dataSurface->TotUserIntConvModels);
                } break;

                default: { // > HcInt::UserCurve
                    // specificmodel
                    ++state.dataSurface->TotUserIntConvModels;
                    auto &userIntConvModel = state.dataSurface->userIntConvModels(state.dataSurface->TotUserIntConvModels);
                    userIntConvModel.SurfaceName = Alphas(1);
                    userIntConvModel.WhichSurface = surfNum;
                    userIntConvModel.overrideType = OverrideType::SpecifiedModel;
                    userIntConvModel.HcIntModelEq = hcInt;
                    ApplyIntConvValue(state, surfNum, hcInt, state.dataSurface->TotUserIntConvModels);
                } break;
                } // switch(HcInt)
            } // if ("INSIDE")
        } // for (pass)
    } // for (Loop)

    CurrentModuleObject = "SurfaceProperty:ConvectionCoefficients:MultipleSurface";
    Count = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
    for (int Loop = 1; Loop <= Count; ++Loop) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 Loop,
                                                                 Alphas,
                                                                 NumAlphas,
                                                                 Numbers,
                                                                 NumNumbers,
                                                                 Status,
                                                                 ipsc->lNumericFieldBlanks,
                                                                 ipsc->lAlphaFieldBlanks,
                                                                 ipsc->cAlphaFieldNames,
                                                                 ipsc->cNumericFieldNames);
        // Check Field 1 for validity
        ErrorObjectHeader eoh{RoutineName, CurrentModuleObject, ""};
        SurfaceFilter surfaceFilter = static_cast<SurfaceFilter>(getEnumValue(SurfaceFilterNamesUC, Alphas(1)));

        for (int Pass = 1, Ptr = 2, FieldNo = 2, NumField = 1; Pass <= 2; ++Pass, Ptr += 4, FieldNo += 4, ++NumField) {

            if (Alphas(Ptr).empty()) {
                continue;
            }

            if (Alphas(Ptr) == "OUTSIDE") {

                HcExt hcExt = static_cast<HcExt>(getEnumValue(HcExtNamesUC, Alphas(Ptr + 1)));

                switch (hcExt) {

                // Are these not used anymore? Can just get rid of them and let these inputs become HcExt::Invalid;
                case HcExt::SetByZone:
                case HcExt::BLASTHcOutside:
                case HcExt::UserValue:
                case HcExt::UserSchedule: {
                    ShowSevereError(state, format("{}{}=\"{}, check input", RoutineName, CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("Check Input Entered :{}", Alphas(Ptr + 1)));
                    ErrorsFound = true;
                } break;

                case HcExt::ASHRAESimple:
                case HcExt::ASHRAETARP:
                case HcExt::MoWiTTHcOutside:
                case HcExt::DOE2HcOutside:
                case HcExt::AdaptiveConvectionAlgorithm: {
                    ApplyExtConvValueMulti(state, surfaceFilter, hcExt, 0);
                } break;

                case HcExt::Value: {
                    // SimpleValueAssignment via userExtConvModels array
                    ++state.dataSurface->TotUserExtConvModels;
                    auto &userExtConvModel = state.dataSurface->userExtConvModels(state.dataSurface->TotUserExtConvModels);
                    userExtConvModel.SurfaceName = Alphas(Ptr);
                    userExtConvModel.WhichSurface = -999;
                    if (Numbers(NumField) < state.dataHeatBal->LowHConvLimit || Numbers(NumField) > state.dataHeatBal->HighHConvLimit) {
                        ShowSevereBadMinMax(state,
                                            eoh,
                                            ipsc->cNumericFieldNames(NumField),
                                            Numbers(NumField),
                                            Clusive::In,
                                            state.dataHeatBal->LowHConvLimit,
                                            Clusive::In,
                                            state.dataHeatBal->HighHConvLimit,
                                            "Limits are set (or default) in HeatBalanceAlgorithm object.");
                        ErrorsFound = true;
                    }
                    userExtConvModel.overrideType = OverrideType::Value;
                    userExtConvModel.OverrideValue = Numbers(NumField);
                    if (!ipsc->lAlphaFieldBlanks(Ptr + 2)) {
                        ShowWarningError(state, format("{}{}=\"{}, duplicate value", RoutineName, CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state,
                                          format("Since VALUE is used for \"{}\", {}={} is ignored.",
                                                 ipsc->cAlphaFieldNames(FieldNo + 2),
                                                 ipsc->cAlphaFieldNames(Ptr + 2),
                                                 Alphas(Ptr + 2)));
                    }
                    ApplyExtConvValueMulti(state, surfaceFilter, hcExt, state.dataSurface->TotUserExtConvModels);
                } break;

                case HcExt::Schedule: {
                    ++state.dataSurface->TotUserExtConvModels;
                    auto &userExtConvModel = state.dataSurface->userExtConvModels(state.dataSurface->TotUserExtConvModels);
                    userExtConvModel.SurfaceName = Alphas(Ptr);
                    userExtConvModel.WhichSurface = -999;
                    userExtConvModel.overrideType = OverrideType::Schedule;
                    if ((userExtConvModel.sched = Sched::GetSchedule(state, Alphas(Ptr + 2))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(Ptr + 2), Alphas(Ptr + 2));
                        ErrorsFound = true;
                    } else if (!userExtConvModel.sched->checkMinMaxVals(
                                   state, Clusive::In, state.dataHeatBal->LowHConvLimit, Clusive::In, state.dataHeatBal->HighHConvLimit)) {
                        Sched::ShowSevereBadMinMax(state,
                                                   eoh,
                                                   ipsc->cAlphaFieldNames(Ptr + 2),
                                                   Alphas(Ptr + 2),
                                                   Clusive::In,
                                                   state.dataHeatBal->LowHConvLimit,
                                                   Clusive::In,
                                                   state.dataHeatBal->HighHConvLimit,
                                                   "Limits are set (or default) in HeatBalanceAlgorithm object.");
                        ErrorsFound = true;
                    }
                    ApplyExtConvValueMulti(state, surfaceFilter, hcExt, state.dataSurface->TotUserExtConvModels);
                } break;

                case HcExt::UserCurve: { // User curve
                    ++state.dataSurface->TotUserExtConvModels;
                    auto &userExtConvModel = state.dataSurface->userExtConvModels(state.dataSurface->TotUserExtConvModels);
                    userExtConvModel.SurfaceName = Alphas(Ptr);
                    userExtConvModel.WhichSurface = -999;
                    userExtConvModel.overrideType = OverrideType::UserCurve;
                    userExtConvModel.UserCurveIndex = Util::FindItemInList(Alphas(Ptr + 3), state.dataConvect->hcExtUserCurve);
                    if (userExtConvModel.UserCurveIndex == 0) {
                        ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(Ptr + 3), Alphas(Ptr + 3));
                        ErrorsFound = true;
                    }
                    ApplyExtConvValueMulti(state, surfaceFilter, hcExt, state.dataSurface->TotUserExtConvModels);
                } break;

                default: { // > HcExt::UserCurve
                    // specificmodel
                    ++state.dataSurface->TotUserExtConvModels;
                    auto &userExtConvModel = state.dataSurface->userExtConvModels(state.dataSurface->TotUserExtConvModels);
                    userExtConvModel.SurfaceName = Alphas(Ptr);
                    userExtConvModel.WhichSurface = -999;
                    userExtConvModel.overrideType = OverrideType::SpecifiedModel;
                    userExtConvModel.HcExtModelEq = hcExt;
                    ApplyExtConvValueMulti(state, surfaceFilter, hcExt, state.dataSurface->TotUserExtConvModels);
                } break;
                } // switch (hcExt)

            } else if (Alphas(Ptr) == "INSIDE") {
                HcInt hcInt = static_cast<HcInt>(getEnumValue(HcIntNamesUC, Alphas(Ptr + 1)));

                switch (hcInt) {

                // Are these not used anymore? We can delete them and let them become HcInt::Invalid
                case HcInt::SetByZone:
                case HcInt::UserValue:
                case HcInt::UserSchedule: {
                    ShowSevereError(state, format("{}{}=\"{}, invalid value", RoutineName, CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format(" Invalid {} entered={}", ipsc->cAlphaFieldNames(Ptr + 1), Alphas(Ptr + 1)));
                    ErrorsFound = true;
                } break;

                case HcInt::CeilingDiffuser:
                case HcInt::TrombeWall: {
                    ShowSevereError(state, format("{}{}=\"{}, invalid value", RoutineName, CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format(" Invalid {} entered={}", ipsc->cAlphaFieldNames(Ptr), Alphas(Ptr)));
                    ShowContinueError(state,
                                      format("invalid value in {}={}\". This type is only applicable at a Zone level.",
                                             ipsc->cAlphaFieldNames(Ptr + 1),
                                             Alphas(Ptr + 1)));
                    ErrorsFound = true;
                } break;

                case HcInt::ASHRAESimple:
                case HcInt::ASHRAETARP:
                case HcInt::AdaptiveConvectionAlgorithm:
                case HcInt::ASTMC1340: {
                    ApplyIntConvValueMulti(state, surfaceFilter, hcInt, 0);
                } break;

                case HcInt::Value: {
                    // SimpleValueAssignment via userExtConvModels array
                    ++state.dataSurface->TotUserIntConvModels;
                    auto &userIntConvModel = state.dataSurface->userIntConvModels(state.dataSurface->TotUserIntConvModels);
                    userIntConvModel.SurfaceName = Alphas(Ptr);
                    userIntConvModel.WhichSurface = -999;
                    if (Numbers(NumField) < state.dataHeatBal->LowHConvLimit || Numbers(NumField) > state.dataHeatBal->HighHConvLimit) {
                        ShowSevereBadMinMax(state,
                                            eoh,
                                            ipsc->cNumericFieldNames(NumField),
                                            Numbers(NumField),
                                            Clusive::In,
                                            state.dataHeatBal->LowHConvLimit,
                                            Clusive::In,
                                            state.dataHeatBal->HighHConvLimit,
                                            "Limits are set (or default) in HeatBalanceAlgorithm object.");
                        ErrorsFound = true;
                    }
                    userIntConvModel.overrideType = OverrideType::Value;
                    userIntConvModel.OverrideValue = Numbers(NumField);
                    if (!ipsc->lAlphaFieldBlanks(Ptr + 2)) {
                        ShowWarningError(state, format("{}{}=\"{}, duplicate value", RoutineName, CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state,
                                          format("Since VALUE is used for \"{}\", {}={} is ignored.",
                                                 ipsc->cAlphaFieldNames(FieldNo + 2),
                                                 ipsc->cAlphaFieldNames(Ptr + 2),
                                                 Alphas(Ptr + 2)));
                    }
                    ApplyIntConvValueMulti(state, surfaceFilter, hcInt, state.dataSurface->TotUserIntConvModels);
                } break;

                case HcInt::Schedule: {
                    ++state.dataSurface->TotUserIntConvModels;
                    auto &userIntConvModel = state.dataSurface->userIntConvModels(state.dataSurface->TotUserIntConvModels);
                    userIntConvModel.SurfaceName = Alphas(Ptr);
                    userIntConvModel.WhichSurface = -999;
                    userIntConvModel.overrideType = OverrideType::Schedule;
                    if ((userIntConvModel.sched = Sched::GetSchedule(state, Alphas(Ptr + 2))) == nullptr) {
                        ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(Ptr + 2), Alphas(Ptr + 2));
                        ErrorsFound = true;
                    } else if (!userIntConvModel.sched->checkMinMaxVals(state,
                                                                        Clusive::In,
                                                                        state.dataHeatBal->LowHConvLimit, // >=
                                                                        Clusive::In,
                                                                        state.dataHeatBal->HighHConvLimit)) { // <=
                        Sched::ShowSevereBadMinMax(state,
                                                   eoh,
                                                   ipsc->cAlphaFieldNames(Ptr + 2),
                                                   Alphas(Ptr + 2),
                                                   Clusive::In,
                                                   state.dataHeatBal->LowHConvLimit,
                                                   Clusive::In,
                                                   state.dataHeatBal->HighHConvLimit,
                                                   "Limits are set (or default) in HeatBalanceAlgorithm object.");
                        ErrorsFound = true;
                    }
                    ApplyIntConvValueMulti(state, surfaceFilter, hcInt, state.dataSurface->TotUserIntConvModels);
                } break;

                case HcInt::UserCurve: {
                    ++state.dataSurface->TotUserIntConvModels;
                    auto &userIntConvModel = state.dataSurface->userIntConvModels(state.dataSurface->TotUserIntConvModels);
                    userIntConvModel.SurfaceName = Alphas(Ptr);
                    userIntConvModel.WhichSurface = -999;
                    userIntConvModel.overrideType = OverrideType::UserCurve;
                    userIntConvModel.UserCurveIndex = Util::FindItemInList(Alphas(Ptr + 3), state.dataConvect->hcIntUserCurve);
                    if (userIntConvModel.UserCurveIndex == 0) {
                        ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(Ptr + 3), Alphas(Ptr + 3));
                        ErrorsFound = true;
                    }
                    ApplyIntConvValueMulti(state, surfaceFilter, hcInt, state.dataSurface->TotUserIntConvModels);
                } break;

                default: { // > HcInt::UserCurve
                    // specificmodel
                    ++state.dataSurface->TotUserIntConvModels;
                    auto &userIntConvModel = state.dataSurface->userIntConvModels(state.dataSurface->TotUserIntConvModels);
                    userIntConvModel.SurfaceName = Alphas(Ptr);
                    userIntConvModel.WhichSurface = -999;
                    userIntConvModel.overrideType = OverrideType::SpecifiedModel;
                    userIntConvModel.HcIntModelEq = hcInt;
                    ApplyIntConvValueMulti(state, surfaceFilter, hcInt, state.dataSurface->TotUserIntConvModels);
                } break;
                } // switch (hcIn)

            } else { // Error Case
                ShowSevereError(state, format("{}{}=\"{}, invalid value", RoutineName, CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format(" Invalid {} entered={}", ipsc->cAlphaFieldNames(Ptr), Alphas(Ptr)));
                ErrorsFound = true;
            }
        } // for (Pass)
    } // for (Loop)

    if (state.dataHeatBal->DefaultExtConvAlgo == HcExt::ASHRAESimple ||
        std::any_of(Zone.begin(), Zone.end(), [](DataHeatBalance::ZoneData const &e) { return e.ExtConvAlgo == HcExt::ASHRAESimple; })) {
        Count = 0;
        for (int Loop = 1; Loop <= state.dataSurface->TotUserExtConvModels; ++Loop) {
            auto const &userExtConvModel = state.dataSurface->userExtConvModels(Loop);
            int SurfNum = userExtConvModel.WhichSurface;
            // Tests show that Zone will override the simple convection specification of global.
            if (SurfNum <= 0) {
                continue; // ignore this error condition
            }
            if (Surface(SurfNum).Zone == 0) {
                continue; // ignore this error condition
            }
            if (Zone(Surface(SurfNum).Zone).ExtConvAlgo == HcExt::ASHRAESimple &&
                ((userExtConvModel.overrideType == OverrideType::SpecifiedModel && userExtConvModel.HcExtModelEq != HcExt::ASHRAESimple) ||
                 userExtConvModel.overrideType != OverrideType::SpecifiedModel)) {
                ++Count;
                if (state.dataGlobal->DisplayExtraWarnings) {
                    ShowSevereError(state, format("{}Surface=\"{}\", mixed algorithms.", RoutineName, userExtConvModel.SurfaceName));
                    ShowContinueError(
                        state, "Zone Outside Convection Algorithm specifies \"SimpleCombined\". SimpleCombined will be used for this surface.");
                }
            }
        }
        if (Count > 0) {
            ShowSevereMessage(state,
                              format("{}{}", RoutineName, format("{} surfaces had different outside convection algorithms specified when", Count)));
            ShowContinueError(state,
                              "the Zone Outside Convection Algorithm specifies \"SimpleCombined\". SimpleCombined will be used for these surfaces.");
            if (!state.dataGlobal->DisplayExtraWarnings) {
                ShowContinueError(state, "Use OutputDiagnostics,DisplayExtraWarnings; to see specific instances.");
                state.dataErrTracking->TotalSevereErrors += Count;
            }
        }
    }

    // get SurfaceConvectionAlgorithm:Inside:AdaptiveModelSelections

    CurrentModuleObject = "SurfaceConvectionAlgorithm:Inside:AdaptiveModelSelections";
    Count = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
    if (Count == 1) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 1,
                                                                 ipsc->cAlphaArgs,
                                                                 NumAlphas,
                                                                 ipsc->rNumericArgs,
                                                                 NumNumbers,
                                                                 Status,
                                                                 ipsc->lNumericFieldBlanks,
                                                                 ipsc->lAlphaFieldBlanks,
                                                                 ipsc->cAlphaFieldNames,
                                                                 ipsc->cNumericFieldNames);
        // state.dataConvect->intAdaptiveConvAlgo.Name = ipsc->cAlphaArgs(1); // not used by E+, unique object
        ErrorObjectHeader eoh{RoutineName, CurrentModuleObject, ipsc->cAlphaArgs(1)};

        auto &intAlgo = state.dataConvect->intAdaptiveConvAlgo;
        for (int iInConvClass = 0, i = 2; iInConvClass < (int)IntConvClass::Num && i <= NumAlphas; ++iInConvClass, i += 2) {

            intAlgo.intConvClassEqNums[iInConvClass] = static_cast<HcInt>(getEnumValue(HcIntNamesUC, ipsc->cAlphaArgs(i)));

            if (intAlgo.intConvClassEqNums[iInConvClass] == HcInt::Invalid) {
                ShowSevereInvalidKey(state, eoh, ipsc->cAlphaFieldNames(i), ipsc->cAlphaArgs(i));
                ErrorsFound = true;
            } else if (intAlgo.intConvClassEqNums[iInConvClass] == HcInt::UserCurve) {
                intAlgo.intConvClassUserCurveNums[iInConvClass] = Util::FindItemInList(ipsc->cAlphaArgs(i + 1), state.dataConvect->hcIntUserCurve);
                if (intAlgo.intConvClassUserCurveNums[iInConvClass] == 0) {
                    ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(i + 1), ipsc->cAlphaArgs(i + 1));
                    ErrorsFound = true;
                }
            }
        } // for (iInConvClass)
    }

    CurrentModuleObject = "SurfaceConvectionAlgorithm:Outside:AdaptiveModelSelections";
    Count = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
    if (Count == 1) {
        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 1,
                                                                 ipsc->cAlphaArgs,
                                                                 NumAlphas,
                                                                 ipsc->rNumericArgs,
                                                                 NumNumbers,
                                                                 Status,
                                                                 ipsc->lNumericFieldBlanks,
                                                                 ipsc->lAlphaFieldBlanks,
                                                                 ipsc->cAlphaFieldNames,
                                                                 ipsc->cNumericFieldNames);

        // state.dataConvect->ExtAdaptiveConvAlgo.Name = ipsc->cAlphaArgs(1); // not used by E+, unique object
        ErrorObjectHeader eoh{RoutineName, CurrentModuleObject, ipsc->cAlphaArgs(1)};
        auto &extAlgo = state.dataConvect->extAdaptiveConvAlgo;

        for (int iOutConvClass = 0, i = 2; i < (int)ExtConvClass::Num && i <= NumAlphas; ++iOutConvClass, i += 2) {

            extAlgo.extConvClass2EqNums[iOutConvClass] = static_cast<HcExt>(getEnumValue(HcExtNamesUC, ipsc->cAlphaArgs(i)));

            if (extAlgo.extConvClass2EqNums[iOutConvClass] == HcExt::Invalid) {
                ShowSevereInvalidKey(state, eoh, ipsc->cAlphaFieldNames(i), ipsc->cAlphaArgs(i));
                ErrorsFound = true;

            } else if (extAlgo.extConvClass2EqNums[iOutConvClass] == HcExt::UserCurve) {
                extAlgo.extConvClass2UserCurveNums[iOutConvClass] = Util::FindItemInList(ipsc->cAlphaArgs(i + 1), state.dataConvect->hcExtUserCurve);
                if (extAlgo.extConvClass2UserCurveNums[iOutConvClass] == 0) {
                    ShowSevereItemNotFound(state, eoh, ipsc->cAlphaFieldNames(i + 1), ipsc->cAlphaArgs(i + 1));
                    ErrorsFound = true;
                }
            }
        } // for (iOutConvClass)
    } // if (Count == 1)

    if (ErrorsFound) {
        ShowFatalError(state, format("{}Errors found getting input.  Program termination.", RoutineName));
    }

    SetupAdaptiveConvStaticMetaData(state);
}

void ApplyIntConvValue(EnergyPlusData &state, int surfNum, HcInt model, int convUserCoeffNum)
{
    auto &surfIntConv = state.dataSurface->surfIntConv(surfNum);
    if (convUserCoeffNum == 0) {
        surfIntConv.model = model;
    } else if (surfIntConv.userModelNum == 0) {
        surfIntConv.model = model;
        surfIntConv.userModelNum = convUserCoeffNum;
    } else {
        ShowWarningError(state,
                         format("User Supplied Convection Coefficients not overwriting already assigned value for (Inside) in Surface={}",
                                state.dataSurface->Surface(surfNum).Name));
    }
}

void ApplyIntConvValueMulti(EnergyPlusData &state, SurfaceFilter surfaceFilter, HcInt model, int userModelNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   November 2004

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine applies a convection type to a set of surfaces.

    if (state.dataSurface->SurfaceFilterLists[(int)surfaceFilter].size() == 0) {
        ShowWarningError(state,
                         format("User Supplied Convection Coefficients, Multiple Surface Assignments=\"{}\", there were no surfaces of that type "
                                "found for Inside assignment.",
                                SurfaceFilterNamesUC[(int)surfaceFilter]));
        return;
    }

    int numWarnings = 0;
    for (int surfNum : state.dataSurface->SurfaceFilterLists[(int)surfaceFilter]) {
        auto &surfIntConv = state.dataSurface->surfIntConv(surfNum);
        if (userModelNum == 0) {
            surfIntConv.model = model;
        } else if (surfIntConv.userModelNum == 0) {
            surfIntConv.model = model;
            surfIntConv.userModelNum = userModelNum;
        } else if (state.dataGlobal->DisplayExtraWarnings) {
            ShowWarningError(state,
                             format("User Supplied Convection Coefficients, Multiple Surface Assignments=\"{}\", not overwriting already "
                                    "assigned value for (Inside) in Surface={}",
                                    SurfaceFilterNamesUC[(int)surfaceFilter],
                                    state.dataSurface->Surface(surfNum).Name));
        } else {
            ++numWarnings;
        }
    } // for (surfNum)

    if (!state.dataGlobal->DisplayExtraWarnings && numWarnings > 0) {
        ShowWarningError(state,
                         format("User Supplied Convection Coefficients, Multiple Surface Assignments=\"{}\", not overwriting already assigned "
                                "values for {} Inside assignments.",
                                SurfaceFilterNamesUC[(int)surfaceFilter],
                                numWarnings));
    }
}

void ApplyExtConvValue(EnergyPlusData &state, int surfNum, HcExt model, int userModelNum)
{
    if (state.dataSurface->Surface(surfNum).OSCPtr > 0) {
        return;
    }

    auto &surfExtConv = state.dataSurface->surfExtConv(surfNum);
    if (userModelNum == 0) {
        surfExtConv.model = model;
    } else if (surfExtConv.userModelNum == 0) {
        surfExtConv.model = model;
        surfExtConv.userModelNum = userModelNum;
    } else {
        ShowWarningError(state,
                         format("User Supplied Convection Coefficients not overwriting already assigned value for (Outside) in Surface={}",
                                state.dataSurface->Surface(surfNum).Name));
    }
}

void ApplyExtConvValueMulti(EnergyPlusData &state, SurfaceFilter surfaceFilter, HcExt model, int convUserCoeffNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   November 2004

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine applies a convection type to a set of surfaces.

    if (state.dataSurface->SurfaceFilterLists[(int)surfaceFilter].size() == 0) {
        return;
    }

    int numWarnings = 0;
    for (int surfNum : state.dataSurface->SurfaceFilterLists[(int)surfaceFilter]) {
        if (state.dataSurface->Surface(surfNum).OSCPtr > 0) {
            continue;
        }
        auto &surfExtConv = state.dataSurface->surfExtConv(surfNum);
        if (convUserCoeffNum == 0) {
            surfExtConv.model = model;
        } else if (surfExtConv.userModelNum == 0) {
            surfExtConv.model = model;
            surfExtConv.userModelNum = convUserCoeffNum;
        } else if (state.dataGlobal->DisplayExtraWarnings) {
            ShowWarningError(state,
                             format("User Supplied Convection Coefficients, Multiple Surface Assignments=\"{}\", not overwriting already "
                                    "assigned value for (Outside) in Surface={}",
                                    SurfaceFilterNamesUC[(int)surfaceFilter],
                                    state.dataSurface->Surface(surfNum).Name));
        } else {
            ++numWarnings;
        }
    } // for (surfNum)

    if (!state.dataGlobal->DisplayExtraWarnings && numWarnings > 0) {
        ShowWarningError(state,
                         format("User Supplied Convection Coefficients, Multiple Surface Assignments=\"{}\", not overwriting already assigned "
                                "values for {} Outside assignments.",
                                SurfaceFilterNamesUC[(int)surfaceFilter],
                                numWarnings));
    }
}

Real64 CalcASHRAESimpExtConvCoeff(Material::SurfaceRoughness const Roughness, // Integer index for roughness, relates to parameter array indices
                                  Real64 const SurfWindSpeed                  // Current wind speed, m/s
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   August 2000

    // PURPOSE OF THIS FUNCTION:
    // This subroutine calculates the exterior convection coefficient
    // using the ASHRAE Simple Method from a correlation from Figure 1
    // on p. 22.4 of the 1989 ASHRAE Handbook of Fundamentals.
    // This is a combined coefficient that includes radiation to sky, ground, and air.

    // REFERENCES:
    // ASHRAE Handbook of Fundamentals 1989, p.22.4

    // FUNCTION PARAMETER DEFINITIONS:
    constexpr static std::array<Real64, 6> D = {11.58, 12.49, 10.79, 8.23, 10.22, 8.23};
    constexpr static std::array<Real64, 6> E = {5.894, 4.065, 4.192, 4.00, 3.100, 3.33};
    constexpr static std::array<Real64, 6> F = {0.0, 0.028, 0.0, -0.057, 0.0, -0.036};

    return D[(int)Roughness] + E[(int)Roughness] * SurfWindSpeed + F[(int)Roughness] * pow_2(SurfWindSpeed);
}

Real64 CalcASHRAESimpleIntConvCoeff(Real64 const Tsurf, Real64 const Tamb, Real64 const cosTilt)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   August 2000

    // PURPOSE OF THIS FUNCTION:
    // This subroutine calculates the interior convection coefficient for a surface.

    // METHODOLOGY EMPLOYED:
    // The convection coefficients are taken directly from the TARP Reference Manual.  TARP calculated
    // its coefficients using the surface conductances for e=0.9 found in ASHRAE Handbook of Fundamentals
    // 1985 in Table 1 on p. 23.2, but subtracted off the radiative component which was estimated at
    // 1.02 * 0.9 = 0.918 BTU/h-ft2-F.  Coefficients were then converted to SI units to yield the values
    // in this subroutine.

    // REFERENCES:
    // 1.  Walton, G. N. 1983. Thermal Analysis Research Program (TARP) Reference Manual,
    //     NBSSIR 83-2655, National Bureau of Standards, "Surface Inside Heat Balances", pp 79.
    // 2.  ASHRAE Handbook of Fundamentals 1985, p. 23.2, Table 1.

    //      +---------------------+-----------+---------------------------------------------+------------------+-----------------+-------------+
    //      |      Situation      | DeltaTemp |                   CosTilt                   | cos(tilt)*deltaT | Convection Type | Coefficient |
    //      +---------------------+-----------+---------------------------------------------+------------------+-----------------+-------------+
    //      | Vertical Surface    | N/A       | -0.3827 to 0.3827 (67.5 to 112.5 degrees)   | N/A              | Normal          |       3.076 |
    //      | Horizontal Surface  | Positive  | 0.9238 to 1.0 (0 to 22.5 degrees)           | Positive         | Enhanced        |       4.043 |
    //      | Horizontal Surface  | Positive  | -0.9238 to -1.0 (157.5 to 180 degrees)      | Negative         | Reduced         |       0.948 |
    //      | Horizontal Surface  | Negative  | 0.9239 to 1.0 (0 to 22.5 degrees)           | Negative         | Reduced         |       0.948 |
    //      | Horizontal Surface  | Negative  | -0.9239 to -1.0 (157.5 to 180 degrees)      | Positive         | Enhanced        |       4.040 |
    //      | Tilted Surface      | Positive  | 0.3827 to 0.9239 (22.5 to 67.5 degrees)     | Positive         | Enhanced        |       3.870 |
    //      | Tilted Surface      | Negative  | -0.3827 to -0.9239 (157.5 to 157.5 degrees) | Positive         | Enhanced        |       3.870 |
    //      | Tilted Surface      | Negative  | 0.3827 to 0.9239 (22.5 to 67.5 degrees)     | Negative         | Reduced         |       2.281 |
    //      | Tilted Surface      | Positive  | -0.3827 to -0.9239 (157.5 to 157.5 degrees) | Negative         | Reduced         |       2.281 |
    //      +---------------------+-----------+---------------------------------------------+------------------+-----------------+-------------+

    // Set HConvIn using the proper correlation based on DeltaTemp and Cosine of the Tilt of the Surface
    if (std::abs(cosTilt) < 0.3827) { // Vertical Surface
        return 3.076;
    } else {
        Real64 DeltaTempCosTilt = (Tamb - Tsurf) * cosTilt;
        if (std::abs(cosTilt) >= 0.9239) { // Horizontal Surface
            if (DeltaTempCosTilt > 0.0) {  // Enhanced Convection
                return 4.040;
            } else if (DeltaTempCosTilt < 0.0) { // Reduced Convection
                return 0.948;
            } else { // Zero DeltaTemp
                return 3.076;
            }
        } else {                          // tilted surface
            if (DeltaTempCosTilt > 0.0) { // Enhanced Convection
                return 3.870;
            } else if (DeltaTempCosTilt < 0.0) { // Reduced Convection
                return 2.281;
            } else { // Zero DeltaTemp
                return 3.076;
            }
        }
    }
}

void CalcASHRAESimpleIntConvCoeff(EnergyPlusData &state,
                                  int const SurfNum,                  // surface number for which coefficients are being calculated
                                  Real64 const SurfaceTemperature,    // Temperature of surface for evaluation of HcIn
                                  Real64 const ZoneMeanAirTemperature // Mean Air Temperature of Zone
)
{
    auto const &surface = state.dataSurface->Surface(SurfNum);
    if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
        state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].in = [](double Tsurf, double Tamb, double, double, double cosTilt) -> double {
            return CalcASHRAESimpleIntConvCoeff(Tsurf, Tamb, cosTilt);
        };
    } else {
        state.dataHeatBalSurf->SurfHConvInt(SurfNum) = CalcASHRAESimpleIntConvCoeff(SurfaceTemperature, ZoneMeanAirTemperature, surface.CosTilt);
    }

    // Establish some lower limit to avoid a zero convection coefficient (and potential divide by zero problems)
    state.dataHeatBalSurf->SurfHConvInt(SurfNum) = max(state.dataHeatBalSurf->SurfHConvInt(SurfNum), state.dataHeatBal->LowHConvLimit);
}

Real64 CalcASHRAETARPNatural(Real64 const Tsurf, Real64 const Tamb, Real64 const cosTilt)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   August 2000

    // PURPOSE OF THIS FUNCTION:
    // This subroutine calculates the convection coefficient for a surface.

    // NOTE:
    // Because surface tilts are given with respect to the outward normal, applications for interior
    // surfaces should use a negative cos(Tilt).

    // METHODOLOGY EMPLOYED:
    // The algorithm for convection coefficients is taken directly from the TARP Reference Manual.
    // ASHRAE Handbook of Fundamentals 2001, p. 3.12, Table 5 gives equations for natural convection
    // heat transfer coefficients in the turbulent range for large, vertical plates and for large,
    // horizontal plates facing upward when heated (or downward when cooled).  A note in the text
    // also gives an approximation for large, horizontal places facing downward when heated (or
    // upward when cooled) recommending that it should be half of the facing upward value.
    // TARP then adds a curve fit as a function of the cosine of the tilt angle to provide intermediate
    // values between vertical and horizontal.  The curve fit values at the extremes match the ASHRAE
    // values very well.

    // REFERENCES:
    // 1.  Walton, G. N. 1983. Thermal Analysis Research Program (TARP) Reference Manual,
    //     NBSSIR 83-2655, National Bureau of Standards, "Surface Inside Heat Balances", pp 79-80.
    // 2.  ASHRAE Handbook of Fundamentals 2001, p. 3.12, Table 5.

    Real64 DeltaTemp = Tsurf - Tamb;

    // Set HConvIn using the proper correlation based on DeltaTemp and Surface (Cosine Tilt)

    if ((DeltaTemp == 0.0) || (cosTilt == 0.0)) { // Vertical Surface

        return CalcASHRAEVerticalWall(DeltaTemp);

    } else if (((DeltaTemp < 0.0) && (cosTilt < 0.0)) || ((DeltaTemp > 0.0) && (cosTilt > 0.0))) { // Enhanced Convection

        return CalcWaltonUnstableHorizontalOrTilt(DeltaTemp, cosTilt);

    } else { // (((DeltaTemp > 0.0) && (cosTilt < 0.0)) || ((DeltaTemp < 0.0) && (cosTilt > 0.0))) // Reduced Convection

        return CalcWaltonStableHorizontalOrTilt(DeltaTemp, cosTilt);

    } // ...end of IF-THEN block to set HConvIn
}

void CalcASHRAEDetailedIntConvCoeff(EnergyPlusData &state,
                                    int const SurfNum,                  // surface number for which coefficients are being calculated
                                    Real64 const SurfaceTemperature,    // Temperature of surface for evaluation of HcIn
                                    Real64 const ZoneMeanAirTemperature // Mean Air Temperature of Zone
)
{
    auto const &surface = state.dataSurface->Surface(SurfNum);
    if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
        state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].in = [](double Tsurf, double Tamb, double, double, double cosTilt) -> double {
            return CalcASHRAETARPNatural(Tsurf, Tamb, cosTilt);
        };
    } else {
        state.dataHeatBalSurf->SurfHConvInt(SurfNum) = CalcASHRAETARPNatural(
            SurfaceTemperature, ZoneMeanAirTemperature, -surface.CosTilt); // negative CosTilt because CosTilt is relative to exterior
    }

    // Establish some lower limit to avoid a zero convection coefficient (and potential divide by zero problems)
    if (state.dataHeatBalSurf->SurfHConvInt(SurfNum) < state.dataHeatBal->LowHConvLimit) {
        state.dataHeatBalSurf->SurfHConvInt(SurfNum) = state.dataHeatBal->LowHConvLimit;
    }
}

void CalcDetailedHcInForDVModel(EnergyPlusData &state,
                                int const SurfNum,                             // surface number for which coefficients are being calculated
                                const Array1D<Real64> &SurfaceTemperatures,    // Temperature of surfaces for evaluation of HcIn
                                Array1D<Real64> &HcIn,                         // Interior Convection Coeff Array
                                ObjexxFCL::Optional<Array1S<Real64> const> Vhc // Velocity array for forced convection coeff calculation
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   August 2000
    //       MODIFIED       Used for DV model; Feb 2004, LKL

    // PURPOSE OF THIS FUNCTION:
    // This subroutine calculates the interior convection coefficient for a surface.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 TAirConv;
    Real64 Hf;
    auto const &surface = state.dataSurface->Surface(SurfNum);

    if (surface.HeatTransSurf) { // Only treat heat transfer surfaces

        // UCSD
        {
            if (state.dataSurface->SurfTAirRef(SurfNum) == DataSurfaces::RefAirTemp::AdjacentAirTemp) {
                TAirConv = state.dataHeatBal->SurfTempEffBulkAir(SurfNum);
            } else {
                // currently set to mean air temp but should add error warning here
                TAirConv = state.dataZoneTempPredictorCorrector->zoneHeatBalance(surface.Zone).MAT;
            }
        }

        assert(state.dataRoomAir->AirModel.allocated());
        if (state.dataRoomAir->AirModel(surface.Zone).AirModel == RoomAir::RoomAirModel::DispVent3Node ||
            state.dataRoomAir->AirModel(surface.Zone).AirModel == RoomAir::RoomAirModel::UFADInt ||
            state.dataRoomAir->AirModel(surface.Zone).AirModel == RoomAir::RoomAirModel::UFADExt) {

            // Set HConvIn using the proper correlation based on DeltaTemp and CosTiltSurf
            if (state.dataSurface->surfIntConv(SurfNum).userModelNum != 0) {

                HcIn(SurfNum) = SetIntConvCoeff(state, SurfNum);

            } else {
                HcIn(SurfNum) = CalcASHRAETARPNatural(SurfaceTemperatures(SurfNum),
                                                      TAirConv,
                                                      -surface.CosTilt); // negative CosTilt because CosTilt is relative to exterior
            }

        } else if (state.dataRoomAir->AirModel(surface.Zone).AirModel == RoomAir::RoomAirModel::CrossVent) {

            Hf = 4.3 * Vhc()(surface.Zone);

            // Set HConvIn using the proper correlation based on DeltaTemp and CosTiltSurf
            if (state.dataSurface->surfIntConv(SurfNum).userModelNum != 0) {

                HcIn(SurfNum) = SetIntConvCoeff(state, SurfNum);

            } else {
                HcIn(SurfNum) = CalcASHRAETARPNatural(SurfaceTemperatures(SurfNum),
                                                      TAirConv,
                                                      -surface.CosTilt); // negative CosTilt because CosTilt is relative to exterior
                HcIn(SurfNum) = std::pow(std::pow(HcIn(SurfNum), 3.2) + std::pow(Hf, 3.2), 1.0 / 3.2);
            }
        }
    }

    // Establish some lower limit to avoid a zero convection coefficient (and potential divide by zero problems)
    if (HcIn(SurfNum) < state.dataHeatBal->LowHConvLimit) {
        HcIn(SurfNum) = state.dataHeatBal->LowHConvLimit;
    }
}

Real64 CalcZoneSystemACH(EnergyPlusData &state, int const ZoneNum)
{

    if (!allocated(state.dataLoopNodes->Node)) {
        return 0.0;
    } else {
        // Set local variables
        Real64 ZoneVolume = state.dataHeatBal->Zone(ZoneNum).Volume;
        Real64 ZoneVolFlowRate = CalcZoneSystemVolFlowRate(state, ZoneNum);

        // Calculate ACH
        return ZoneVolFlowRate / ZoneVolume * Constant::rSecsInHour;
    }
}

Real64 CalcZoneSupplyAirTemp(EnergyPlusData &state, int const ZoneNum)
{

    int ZoneNode = state.dataHeatBal->Zone(ZoneNum).SystemZoneNodeNumber;
    if (ZoneNode <= 0) {
        return state.dataLoopNodes->Node(ZoneNode).Temp;
    }

    auto &zoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(ZoneNum);
    auto &zoneEquipList = state.dataZoneEquip->ZoneEquipList(zoneEquipConfig.EquipListIndex);

    int zoneInletNodeNum = 0;

    Real64 SumMdotTemp = 0.0;
    Real64 SumMdot = 0.0;

    for (int EquipNum = 1; EquipNum <= zoneEquipList.NumOfEquipTypes; ++EquipNum) {

        auto &equipData = zoneEquipList.EquipData(EquipNum);
        if (equipData.NumOutlets == 0) {
            continue;
        }

        zoneInletNodeNum = equipData.OutletNodeNums(1);
        if (zoneInletNodeNum == 0) {
            continue;
        }

        auto const &zoneInletNode = state.dataLoopNodes->Node(zoneInletNodeNum);
        if (zoneInletNode.MassFlowRate > 0.0) {
            SumMdotTemp += zoneInletNode.MassFlowRate * zoneInletNode.Temp;
            SumMdot += zoneInletNode.MassFlowRate;
        }
    }

    if (SumMdot > 0.0) {
        return SumMdotTemp / SumMdot; // mass flow weighted inlet temperature
    }

    if (zoneInletNodeNum > 0) {
        return state.dataLoopNodes->Node(zoneInletNodeNum).Temp;
    } else {
        return state.dataLoopNodes->Node(ZoneNode).Temp;
    }
}

Real64 CalcZoneSystemVolFlowRate(EnergyPlusData &state, int const ZoneNum)
{
    auto const &zone = state.dataHeatBal->Zone(ZoneNum);

    if (state.dataGlobal->BeginEnvrnFlag || zone.SystemZoneNodeNumber <= 0) {
        return 0.0;
    }

    auto const &zoneNode = state.dataLoopNodes->Node(zone.SystemZoneNodeNumber);
    int ZoneMult = zone.Multiplier * zone.ListMultiplier;
    Real64 AirDensity = Psychrometrics::PsyRhoAirFnPbTdbW(
        state, state.dataEnvrn->OutBaroPress, zoneNode.Temp, Psychrometrics::PsyWFnTdpPb(state, zoneNode.Temp, state.dataEnvrn->OutBaroPress));
    return zoneNode.MassFlowRate / (AirDensity * ZoneMult);
}

Real64 CalcCeilingDiffuserACH(EnergyPlusData &state, int const ZoneNum)
{
    constexpr Real64 MinFlow(0.01); // Minimum mass flow rate
    constexpr Real64 MaxACH(100.0); // Maximum ceiling diffuser correlation limit

    auto const &zone = state.dataHeatBal->Zone(ZoneNum);

    Real64 ACH = CalcZoneSystemACH(state, ZoneNum); // Air changes per hour

    Real64 ZoneMassFlowRate;
    Real64 ZoneMult = zone.Multiplier * zone.ListMultiplier;
    int ZoneNode = zone.SystemZoneNodeNumber; // Zone node as defined in system simulation
    if (!state.dataGlobal->BeginEnvrnFlag && ZoneNode > 0) {
        ZoneMassFlowRate = state.dataLoopNodes->Node(ZoneNode).MassFlowRate / ZoneMult;
    } else { // because these are not updated yet for new environment
        ZoneMassFlowRate = 0.0;
    }

    if (ZoneMassFlowRate < MinFlow) {
        ACH = 0.0;
    } else {
        // Calculate ACH
        ACH = min(ACH, MaxACH);
        ACH = max(ACH, 0.0);
    }

    return ACH;
}

Real64 CalcCeilingDiffuserIntConvCoeff(EnergyPlusData &state,
                                       Real64 const ACH, // [1/hr] air system air change rate
                                       Real64 const Tsurf,
                                       Real64 const Tair,
                                       Real64 const cosTilt,
                                       Real64 const humRat,
                                       Real64 const height,
                                       bool const isWindow)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   August 2000

    // PURPOSE OF THIS FUNCTION:
    // This subroutine calculates the interior convection coefficients
    // for ceiling diffusers correlated to the outlet air temperature.

    // REFERENCES:
    // Fisher, D.E. and C.O. Pedersen, Convective Heat Transfer in Building Energy and
    //       Thermal Load Calculations, ASHRAE Transactions, vol. 103, Pt. 2, 1997, p.137

    // OTHER NOTES:
    // The correlations shown below differ from (and are less accurate than) those shown
    // in the reference above (Fisher 1997).  They have been reformulated with an outlet
    // temperature reference in order to accommodate the structure of the EnergyPlus code.

    // If the Ceiling Diffuser option is selected the following correlations are used.
    // The development of the ceiling diffuser convection correlations is shown in reference 4.
    // The correlations shown below differ from (and are less accurate than) those shown in reference 4 because they have been
    // reformulated with an outlet temperature reference in order to accommodate the structure of the
    // EnergyPlus code.

    // Set HConvIn using the proper correlation based on Surface Tilt
    static const Real64 cos45(sqrt(2.) / 2.0);

    if (cosTilt < -cos45) {
        return CalcFisherPedersenCeilDiffuserFloor(state, ACH, Tsurf, Tair, cosTilt, humRat, height, isWindow); // Floor correlation
    } else if (cosTilt > cos45) {
        return CalcFisherPedersenCeilDiffuserCeiling(state, ACH, Tsurf, Tair, cosTilt, humRat, height, isWindow); // Ceiling correlation
    } else {
        return CalcFisherPedersenCeilDiffuserWalls(state, ACH, Tsurf, Tair, cosTilt, humRat, height, isWindow); // Wall correlation
    }
}

void CalcCeilingDiffuserIntConvCoeff(EnergyPlusData &state,
                                     int const ZoneNum,
                                     const Array1D<Real64> &SurfaceTemperatures) // zone number for which coefficients are being calculated
{

    Real64 ACH = CalcCeilingDiffuserACH(state, ZoneNum);

    Real64 AirHumRat = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).airHumRatAvg;

    for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
        auto const &thisSpace = state.dataHeatBal->space(spaceNum);
        for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {
            auto const &surface = state.dataSurface->Surface(SurfNum);
            if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
                Real64 height = surface.Height;
                bool isWindow = state.dataConstruction->Construct(surface.Construction).TypeIsWindow;
                state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].in =
                    [=, &state](double Tsurf, double Tamb, double, double, double cosTilt) -> double {
                    return CalcCeilingDiffuserIntConvCoeff(state, ACH, Tsurf, Tamb, cosTilt, AirHumRat, height, isWindow);
                };
            } else {
                state.dataHeatBalSurf->SurfHConvInt(SurfNum) =
                    CalcCeilingDiffuserIntConvCoeff(state,
                                                    ACH,
                                                    SurfaceTemperatures(SurfNum),
                                                    state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).MAT,
                                                    surface.CosTilt,
                                                    AirHumRat,
                                                    surface.Height,
                                                    state.dataConstruction->Construct(surface.Construction).TypeIsWindow);
                // Establish some lower limit to avoid a zero convection coefficient (and potential divide by zero problems)
                if (state.dataHeatBalSurf->SurfHConvInt(SurfNum) < state.dataHeatBal->LowHConvLimit) {
                    state.dataHeatBalSurf->SurfHConvInt(SurfNum) = state.dataHeatBal->LowHConvLimit;
                }
            }
        } // SurfNum
    }
}

// CalcCeilingDiffuserInletCorr should replace CalcCeilingDiffuser (above), if ZoneTempPredictorCorrector can
// ever be made to work correctly with the inlet air temperature.

void CalcCeilingDiffuserInletCorr(EnergyPlusData &state,
                                  int const ZoneNum,                         // Zone number
                                  const Array1S<Real64> &SurfaceTemperatures // For CalcASHRAEDetailed, if called
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   August 2000
    //       RE-ENGINEERED  July 2003 (Peter Graham Ellis)
    //       MODIFIED       July 2003, (CC) set a flag for reference temperature so that supply air temperature
    //                                      is used as the reference in the inside heat balance calculations

    // PURPOSE OF THIS FUNCTION:
    // This subroutine calculates the interior convection coefficients
    // for ceiling diffusers correlated to the inlet air temperature.

    // REFERENCES:
    // Fisher, D.E. and C.O. Pedersen, Convective Heat Transfer in Building Energy and
    //   Thermal Load Calculations, ASHRAE Transactions, vol. 103, Pt. 2, 1997, p.137

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 constexpr MinFlow(0.01); // Minimum mass flow rate
    Real64 constexpr MaxACH(100.0); // Maximum ceiling diffuser correlation limit
    Real64 ACH;                     // Air changes per hour

    auto const &zone = state.dataHeatBal->Zone(ZoneNum);

    if (state.dataGlobal->SysSizingCalc || state.dataGlobal->ZoneSizingCalc || !allocated(state.dataLoopNodes->Node)) {
        ACH = 0.0;
    } else {
        // Set local variables
        Real64 ZoneVolume = zone.Volume;
        Real64 ZoneMult = zone.Multiplier * zone.ListMultiplier;
        auto const &zoneNode = state.dataLoopNodes->Node(zone.SystemZoneNodeNumber);
        Real64 AirDensity = Psychrometrics::PsyRhoAirFnPbTdbW(
            state, state.dataEnvrn->OutBaroPress, zoneNode.Temp, Psychrometrics::PsyWFnTdpPb(state, zoneNode.Temp, state.dataEnvrn->OutBaroPress));
        Real64 ZoneMassFlowRate = zoneNode.MassFlowRate / ZoneMult;

        if (ZoneMassFlowRate < MinFlow) {
            ACH = 0.0;
        } else {
            // Calculate ACH (AR: can we please stop with these unparenthesized multiple divides? / / )
            ACH = ZoneMassFlowRate / AirDensity / ZoneVolume * Constant::rSecsInHour;
            // Limit ACH to range of correlation
            ACH = min(ACH, MaxACH);
            ACH = max(ACH, 0.0);
        }
    }

    for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
        auto const &thisSpace = state.dataHeatBal->space(spaceNum);
        for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {
            if (ACH <= 3.0) { // Use the other convection algorithm
                if (!state.dataConstruction->Construct(state.dataSurface->Surface(SurfNum).Construction).TypeIsWindow) {
                    CalcASHRAEDetailedIntConvCoeff(
                        state, SurfNum, SurfaceTemperatures(SurfNum), state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum).MAT);
                } else {
                    CalcISO15099WindowIntConvCoeff(
                        state, SurfNum, SurfaceTemperatures(SurfNum), state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum).MAT);
                }
            } else { // Use forced convection correlations
                Real64 Tilt = state.dataSurface->Surface(SurfNum).Tilt;

                // assume that reference air temp for user defined convection coefficient is the mean air temperature (=MAT)
                // Calculate the convection coefficient based on inlet (supply) air conditions
                if (Tilt < 45.0) {
                    state.dataHeatBalSurf->SurfHConvInt(SurfNum) = 0.49 * std::pow(ACH, 0.8); // Ceiling correlation
                } else if (Tilt > 135.0) {
                    state.dataHeatBalSurf->SurfHConvInt(SurfNum) = 0.13 * std::pow(ACH, 0.8); // Floor correlation
                } else {
                    state.dataHeatBalSurf->SurfHConvInt(SurfNum) = 0.19 * std::pow(ACH, 0.8); // Wall correlation
                }
                // set flag for reference air temperature
                state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneSupplyAirTemp;
                state.dataSurface->SurfTAirRefRpt(SurfNum) = DataSurfaces::SurfTAirRefReportVals[state.dataSurface->SurfTAirRef(SurfNum)];
            }

            // Establish some lower limit to avoid a zero convection coefficient (and potential divide by zero problems)
            if (state.dataHeatBalSurf->SurfHConvInt(SurfNum) < state.dataHeatBal->LowHConvLimit) {
                state.dataHeatBalSurf->SurfHConvInt(SurfNum) = state.dataHeatBal->LowHConvLimit;
            }

        } // SurfNum
    }
    if (ACH > 100.0) {
        ShowWarningError(state, "CeilingDiffuser convection correlation is out of range: ACH > 100");
    }
}

void CalcTrombeWallIntConvCoeff(EnergyPlusData &state,
                                int const ZoneNum,                         // Zone number for which coefficients are being calculated
                                const Array1D<Real64> &SurfaceTemperatures // Temperature of surfaces for evaluation of HcIn
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis

    // PURPOSE OF THIS FUNCTION:
    // This subroutine calculates the interior convection coefficient
    // using the Trombe Wall correlation ?????

    // SUBROUTINE PARAMETER DEFINITIONS:
    constexpr Real64 g(9.81);     // gravity constant (m/s**2)
    constexpr Real64 v(15.89e-6); // kinematic viscosity (m**2/s) for air at 300 K
    constexpr Real64 k(0.0263);   // thermal conductivity (W/m K) for air at 300 K
    constexpr Real64 Pr(0.71);    // Prandtl number for air at ?

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int Surf1 = 0; // first major wall surface
    int Surf2 = 0; // second major wall surface

    // If the Trombe Wall option is selected the following correlations
    // will be used based on references by .....
    // tall enclosed rectangular cavity

    // This routine assumes that the major Trombe wall surfaces are of the
    // "WALL" class and are vertical.  The important heat transfer surfaces
    // are assumed to have exactly equal widths AND must have a greater
    // width than the side surfaces.

    auto const &zone = state.dataHeatBal->Zone(ZoneNum);
    Real64 H = zone.CeilingHeight; // height of enclosure
    Real64 minorW = 100000.0;      // width of enclosure (narrow dimension) // An impossibly big width
    Real64 majorW = 0.0;           // width of major surface

    Real64 HConvNet = 0.0; // net heat transfer coefficient from wall to wall

    // determine major width and minor width
    for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
        auto const &thisSpace = state.dataHeatBal->space(spaceNum);
        for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {
            auto const &surface = state.dataSurface->Surface(SurfNum);
            if (surface.Class != SurfaceClass::Wall) {
                continue;
            }

            if (surface.Width > majorW) {
                majorW = surface.Width;
            }
            if (surface.Width < minorW) {
                minorW = surface.Width;
            }
        }

        // assign major surfaces
        for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {
            auto const &surface = state.dataSurface->Surface(SurfNum);
            if (surface.Class != SurfaceClass::Wall) {
                continue;
            }

            if (surface.Width == majorW) {
                if (Surf1 == 0) {
                    Surf1 = SurfNum;
                } else {
                    Surf2 = SurfNum;

                    break; // both major surfaces are now assigned
                }
            }
        }
    }

    // check to make sure major surfaces were found
    if (Surf1 > 0 && Surf2 > 0) {
        Real64 gapW = minorW;
        Real64 asp = H / gapW; // aspect ratio H/gapW // This calc should only be done once for the zone

        // make sure inside surface is hot, outside is cold
        // NOTE: this is not ideal.  could have circumstances that reverse this?
        Real64 Tso, Tsi;
        if (SurfaceTemperatures(Surf1) > SurfaceTemperatures(Surf2)) {
            Tsi = SurfaceTemperatures(Surf1) + Constant::Kelvin;
            Tso = SurfaceTemperatures(Surf2) + Constant::Kelvin;
        } else {
            Tso = SurfaceTemperatures(Surf1) + Constant::Kelvin;
            Tsi = SurfaceTemperatures(Surf2) + Constant::Kelvin;
        }

        Real64 beta = 2.0 / (Tso + Tsi);                                       // volumetric thermal expansion coefficient
        Real64 Gr = (g * beta * std::abs(Tsi - Tso) * pow_3(gapW)) / pow_2(v); // Grashof // curve fit for v = v(T)?
        Real64 Nu = CalcNusselt(state, Surf2, asp, Tso, Tsi, Gr, Pr);          // Nusselt // curve fit for Pr = Pr(T)?

        HConvNet = (k / gapW) * Nu; // curve fit for k = k(T)?

    } else {
        // fatal Error msg "heat transfer surfaces not found"
    }

    // Assign convection coefficients
    for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
        auto const &thisSpace = state.dataHeatBal->space(spaceNum);
        for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {
            auto const &surface = state.dataSurface->Surface(SurfNum);
            // Use ASHRAESimple correlation to give values for all the minor surfaces
            CalcASHRAESimpleIntConvCoeff(
                state, SurfNum, SurfaceTemperatures(SurfNum), state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum).MAT);

            // assign the convection coefficent to the major surfaces and any subsurfaces on them
            if ((surface.BaseSurf == Surf1) || (surface.BaseSurf == Surf2)) {
                if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
                    ShowFatalError(state, format("Trombe wall convection model not applicable for foundation surface ={}", surface.Name));
                }
                state.dataHeatBalSurf->SurfHConvInt(SurfNum) = 2.0 * HConvNet;
            }

            // Establish some lower limit to avoid a zero convection coefficient (and potential divide by zero problems)
            if (state.dataHeatBalSurf->SurfHConvInt(SurfNum) < state.dataHeatBal->LowHConvLimit) {
                state.dataHeatBalSurf->SurfHConvInt(SurfNum) = state.dataHeatBal->LowHConvLimit;
            }
        } // for (surfNum)
    } // for (spaceNum)
}

Real64 CalcNusselt(EnergyPlusData &state,
                   int const SurfNum, // Surface number
                   Real64 const asp,  // Aspect ratio: window height to gap width
                   Real64 const tso,  // Temperature of gap surface closest to outside (K)
                   Real64 const tsi,  // Temperature of gap surface closest to zone (K)
                   Real64 const gr,   // Gap gas Grashof number
                   Real64 const pr)   // Gap gas Prandtl number
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Peter Graham Ellis, based on code adapted by Fred Winkelmann
    //                      from Window5 subroutine NusseltNumber
    //       DATE WRITTEN   September 2001

    // PURPOSE OF THIS SUBROUTINE:
    // Finds the Nusselt number for gas-filled gaps between isothermal solid layers.
    // The gap may be filled with a single gas or a gas mixture.

    // METHODOLOGY EMPLOYED:
    // Based on methodology in Chapter 5 of the July 18, 2001 draft of ISO 15099,
    // "Thermal Performance of Windows, Doors and Shading Devices--Detailed Calculations."
    // The equation numbers below correspond to those in the standard.

    // REFERENCES:
    // Window5 source code; ISO 15099

    auto const &surface = state.dataSurface->Surface(SurfNum);

    Real64 tilt = surface.Tilt;
    Real64 tiltr = tilt * Constant::DegToRad;
    Real64 costilt = surface.CosTilt;
    Real64 sintilt = surface.SinTilt;
    Real64 ra = gr * pr; // Rayleigh number
    //! fw if (ra > 2.0e6): error that outside range of Rayleigh number?

    Real64 gnu901; // Nusselt number temporary variables for
    if (ra <= 1.0e4) {
        gnu901 = 1.0 + 1.7596678e-10 * std::pow(ra, 2.2984755); // eq. 51
    } else if (ra > 1.0e4 && ra <= 5.0e4) {
        gnu901 = 0.028154 * std::pow(ra, 0.4134); // eq. 50
    } else {
        gnu901 = 0.0673838 * std::pow(ra, 1.0 / 3.0); // eq. 49
    }

    Real64 gnu902 = 0.242 * std::pow(ra / asp, 0.272); // eq. 52
    Real64 gnu90 = max(gnu901, gnu902);

    if (tso > tsi) {                          // window heated from above
        return 1.0 + (gnu90 - 1.0) * sintilt; // eq. 53
    }

    // window heated from below
    if (tilt >= 60.0) {
        Real64 g = 0.5 * std::pow(1.0 + std::pow(ra / 3160.0, 20.6), -0.1);     // eq. 47
        Real64 gnu601a = 1.0 + pow_7(0.0936 * std::pow(ra, 0.314) / (1.0 + g)); // eq. 45
        Real64 gnu601 = std::pow(gnu601a, 0.142857);
        // For any aspect ratio
        Real64 gnu602 = (0.104 + 0.175 / asp) * std::pow(ra, 0.283); // eq. 46
        Real64 gnu60 = max(gnu601, gnu602);

        // linear interpolation for layers inclined at angles between 60 and 90 deg
        return ((90.0 - tilt) * gnu60 + (tilt - 60.0) * gnu90) / 30.0;

    } else { // eq. 42
        Real64 cra = ra * costilt;
        Real64 a = 1.0 - 1708.0 / cra;
        Real64 b = std::pow(cra / 5830.0, 0.33333) - 1.0; // LKL- replace .333 with OneThird?
        Real64 gnua = (std::abs(a) + a) / 2.0;
        Real64 gnub = (std::abs(b) + b) / 2.0;
        Real64 ang = 1708.0 * std::pow(std::sin(1.8 * tiltr), 1.6);
        return 1.0 + 1.44 * gnua * (1.0 - ang / cra) + gnub;
    }
}

Real64 SetExtConvCoeff(EnergyPlusData &state, int const SurfNum) // Surface Number
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   May 1998

    // PURPOSE OF THIS FUNCTION:
    // This function accesses the data structure for the User
    // Supplied Exterior Convection Coefficients and returns that
    // as the result of this function.  The surface has already
    // been verified to have user supplied exterior convection values.

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    Real64 HExt = 0.0; // Will become the returned value

    auto const &surface = state.dataSurface->Surface(SurfNum);
    auto &surfExtConv = state.dataSurface->surfExtConv(SurfNum);
    auto const &userExtConvModel = state.dataSurface->userExtConvModels(surfExtConv.userModelNum);

    switch (userExtConvModel.overrideType) {
    case OverrideType::Value: {
        HExt = userExtConvModel.OverrideValue;
        if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].f = KIVA_HF_ZERO;
            state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].out = KIVA_CONST_CONV(HExt);
        }
        surfExtConv.hfModelEq = HcExt::UserValue; // reporting
        surfExtConv.hnModelEq = HcExt::None;      // reporting
    } break;

    case OverrideType::Schedule: {
        HExt = userExtConvModel.sched->getCurrentVal();
        // Need to check for validity
        if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].f = KIVA_HF_ZERO;
            state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].out = KIVA_CONST_CONV(HExt);
        }
        surfExtConv.hfModelEq = HcExt::UserSchedule; // reporting
        surfExtConv.hnModelEq = HcExt::None;         // reporting
    } break;

    case OverrideType::UserCurve: {
        HExt = CalcUserDefinedExtHcModel(state, SurfNum, userExtConvModel.UserCurveIndex);
        // Kiva convection handled in function above
        surfExtConv.hfModelEq = HcExt::UserCurve; // reporting
        surfExtConv.hnModelEq = HcExt::None;      // reporting
    } break;

    case OverrideType::SpecifiedModel: {
        HExt = EvaluateExtHcModels(state, SurfNum, userExtConvModel.HcExtModelEq, userExtConvModel.HcExtModelEq);
        // Kiva convection handled in function above
        surfExtConv.hfModelEq = userExtConvModel.HcExtModelEq; // reporting
        surfExtConv.hnModelEq = userExtConvModel.HcExtModelEq; // reporting
    } break;

    default:
        assert(false);
    }

    surfExtConv.hfModelEqRpt = HcExtReportVals[(int)surfExtConv.hfModelEq];
    surfExtConv.hnModelEqRpt = HcExtReportVals[(int)surfExtConv.hnModelEq];

    return HExt;
}

Real64 SetIntConvCoeff(EnergyPlusData &state, int const SurfNum) // Surface Number
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Linda K. Lawrie
    //       DATE WRITTEN   May 1998

    // PURPOSE OF THIS FUNCTION:
    // This function accesses the data structure for the User
    // Supplied Interior Convection Coefficients and returns that
    // as the result of this function.  The surface has already
    // been verified to have user supplied interior convection values.

    Real64 HInt = 0.0; // Will become the returned value

    auto const &surface = state.dataSurface->Surface(SurfNum);
    auto &surfIntConv = state.dataSurface->surfIntConv(SurfNum);
    auto const &userIntConvModel = state.dataSurface->userIntConvModels(surfIntConv.userModelNum);

    switch (userIntConvModel.overrideType) {
    case OverrideType::Value: {
        HInt = userIntConvModel.OverrideValue;
        if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].in = KIVA_CONST_CONV(HInt);
        }
        surfIntConv.hcModelEq = HcInt::UserValue; // reporting
        surfIntConv.hcModelEqRpt = HcIntReportVals[(int)surfIntConv.hcModelEq];
    } break;

    case OverrideType::Schedule: {
        HInt = userIntConvModel.sched->getCurrentVal();
        // Need to check for validity
        if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].in = KIVA_CONST_CONV(HInt);
        }
        surfIntConv.hcModelEq = HcInt::UserSchedule; // reporting
        surfIntConv.hcModelEqRpt = HcIntReportVals[(int)surfIntConv.hcModelEq];
    } break;

    case OverrideType::UserCurve: {
        HInt = CalcUserDefinedIntHcModel(state, SurfNum, userIntConvModel.UserCurveIndex);
        // Kiva convection handled in function above
        surfIntConv.hcModelEq = HcInt::UserCurve; // reporting
        surfIntConv.hcModelEqRpt = HcIntReportVals[(int)surfIntConv.hcModelEq];
    } break;
    case OverrideType::SpecifiedModel: {
        HInt = EvaluateIntHcModels(state, SurfNum, userIntConvModel.HcIntModelEq);
        // Kiva convection handled in function above
        surfIntConv.hcModelEq = userIntConvModel.HcIntModelEq;
        surfIntConv.hcModelEqRpt = HcIntReportVals[(int)surfIntConv.hcModelEq];
    } break;
    default: {
        assert(false);
    }
    }

    return HInt;
}

Real64 CalcISO15099WindowIntConvCoeff(EnergyPlusData &state,
                                      Real64 const SurfaceTemperature, // Temperature of surface for evaluation of HcIn
                                      Real64 const AirTemperature,     // Mean Air Temperature of Zone (or adjacent air temperature)
                                      Real64 const AirHumRat,          // air humidity ratio
                                      Real64 const Height,             // window cavity height [m]
                                      Real64 TiltDeg,                  // glazing tilt in degrees
                                      Real64 const sineTilt            // sine of glazing tilt
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   January 2009
    //       MODIFIED       BG May 2009, added EMS override for window coeffs.

    // PURPOSE OF THIS SUBROUTINE:
    // Calculate interior surface convection coefficients for windows

    // METHODOLOGY EMPLOYED:
    // correlation documented in ISO 15099, Section 8.3.2.2

    // REFERENCES:
    // Internation Standard ISO 15099. Thermal performance of windows, doors and shading devices -- Detailed Calculations
    // First Edition 2003-11-15. ISO 15099:2003(E)

    // Locals
    static constexpr Real64 OneThird((1.0 / 3.0)); // 1/3 in highest precision
    static Real64 const pow_5_25(0.56 * root_4(1.0E+5));
    static Real64 const pow_11_25(0.56 * root_4(1.0E+11));
    static Real64 const pow_11_2(0.58 * std::pow(1.0E+11, 0.2));
    static constexpr std::string_view RoutineName("WindowTempsForNominalCond");

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 constexpr g(9.81); // acceleration due to gravity [m/s2]
    Real64 Nuint(0.0);        // Nusselt number for interior surface convection

    Real64 SurfTempKelvin = SurfaceTemperature + Constant::Kelvin;
    Real64 AirTempKelvin = AirTemperature + Constant::Kelvin;
    Real64 DeltaTemp = SurfaceTemperature - AirTemperature;

    // protect against wildly out of range temperatures
    if ((AirTempKelvin < 200.0) || (AirTempKelvin > 400.0)) { // out of range
        return state.dataHeatBal->LowHConvLimit;
    }
    if ((SurfTempKelvin < 180.0) || (SurfTempKelvin > 450.0)) { // out of range
        return state.dataHeatBal->LowHConvLimit;
    }

    // mean film temperature
    Real64 TmeanFilmKelvin = AirTempKelvin + 0.25 * (DeltaTemp); // eq. 133 in ISO 15099
    Real64 TmeanFilm = TmeanFilmKelvin - 273.15;

    // density of air [kg/m3]
    Real64 rho = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, TmeanFilm, AirHumRat, RoutineName);

    // the following properties are probably for dry air, should maybe be remade for moist-air
    Real64 lambda = 2.873E-3 + 7.76E-5 * TmeanFilmKelvin; // thermal conductivity of air [W/m-K] // Table B.1 in ISO 15099,
    Real64 mu = 3.723E-6 + 4.94E-8 * TmeanFilmKelvin;     // dynamic viscosity of air [kg/m-s] // Table B.2 in ISO 15099
    Real64 Cp = Psychrometrics::PsyCpAirFnW(AirHumRat);   // specific heat of air [J/kg-K]

    // four cases depending on tilt and DeltaTemp (heat flow direction )
    if (DeltaTemp > 0.0) {
        TiltDeg = 180.0 - TiltDeg; // complement angle if cooling situation
    }

    // Rayleigh number for cavity height [ Non dim]
    Real64 RaH = (pow_2(rho) * pow_3(Height) * g * Cp * (std::abs(DeltaTemp))) / (TmeanFilmKelvin * mu * lambda); // eq 132 in ISO 15099

    // case a)
    if ((0.0 <= TiltDeg) && (TiltDeg < 15.0)) {

        Nuint = 0.13 * std::pow(RaH, OneThird);

        // case b)
    } else if ((15.0 <= TiltDeg) && (TiltDeg <= 90.0)) {

        // Rayleigh number for slanted cavity
        Real64 RaCV = 2.5E+5 * std::pow(std::exp(0.72 * TiltDeg) / sineTilt, 0.2); // eq. 137

        if (RaH <= RaCV) {
            Nuint = 0.56 * root_4(RaH * sineTilt); // eq. 135 in ISO 15099
        } else {
            Nuint = 0.13 * (std::pow(RaH, OneThird) - std::pow(RaCV, OneThird)) + 0.56 * root_4(RaCV * sineTilt); // eq. 136 in ISO 15099
        }

        // case c)
    } else if ((90.0 < TiltDeg) && (TiltDeg <= 179.0)) {
        // bound by applicability
        if (RaH * sineTilt < 1.0E+5) {
            Nuint = pow_5_25; // bounded
        } else if (RaH * sineTilt >= 1.0E+11) {
            Nuint = pow_11_25; // bounded
        } else {
            Nuint = 0.56 * root_4(RaH * sineTilt); // eq.. 138
        }

        // case d)
    } else if ((179.0 < TiltDeg) && (TiltDeg <= 180.0)) {

        if (RaH > 1.0E+11) {
            Nuint = pow_11_2; // bounded
        } else {
            Nuint = 0.58 * std::pow(RaH, 0.2);
        }

    } else {
        assert(false);
    }

    return Nuint * lambda / Height;
}

void CalcISO15099WindowIntConvCoeff(EnergyPlusData &state,
                                    int const SurfNum,               // surface number for which coefficients are being calculated
                                    Real64 const SurfaceTemperature, // Temperature of surface for evaluation of HcIn
                                    Real64 const AirTemperature      // Mean Air Temperature of Zone (or adjacent air temperature)
)
{
    auto const &surface = state.dataSurface->Surface(SurfNum);

    // Get humidity ratio
    Real64 AirHumRat =
        (surface.spaceNum > 0) ? state.dataZoneTempPredictorCorrector->spaceHeatBalance(surface.spaceNum).airHumRatAvg : state.dataEnvrn->OutHumRat;

    Real64 Height = surface.Height;
    Real64 TiltDeg = surface.Tilt;
    Real64 sineTilt = surface.SinTilt;

    if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
        ShowFatalError(state, format("ISO15099 convection model not applicable for foundation surface ={}", surface.Name));
    }

    state.dataHeatBalSurf->SurfHConvInt(SurfNum) =
        CalcISO15099WindowIntConvCoeff(state, SurfaceTemperature, AirTemperature, AirHumRat, Height, TiltDeg, sineTilt);

    // EMS override point (Violates Standard 15099?  throw warning? scary.
    if (state.dataSurface->SurfEMSOverrideIntConvCoef(SurfNum)) {
        state.dataHeatBalSurf->SurfHConvInt(SurfNum) = state.dataSurface->SurfEMSValueForIntConvCoef(SurfNum);
    } else {
        state.dataHeatBalSurf->SurfHConvInt(SurfNum) *= state.dataHeatBalSurf->SurfWinCoeffAdjRatio(SurfNum);
    }

    // Establish some lower limit to avoid a zero convection coefficient (and potential divide by zero problems)
    if (state.dataHeatBalSurf->SurfHConvInt(SurfNum) < state.dataHeatBal->LowHConvLimit) {
        state.dataHeatBalSurf->SurfHConvInt(SurfNum) = state.dataHeatBal->LowHConvLimit;
    }
}

void SetupAdaptiveConvStaticMetaData(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS SUBROUTINE:
    // do one-time setup needed to store static data for adaptive convection algorithm

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 thisZoneHorizHydralicDiameter;
    bool DoReport;

    Real64 BldgVolumeSum = 0.0;
    for (int ZoneLoop = 1; ZoneLoop <= state.dataGlobal->NumOfZones; ++ZoneLoop) {
        auto const &zone = state.dataHeatBal->Zone(ZoneLoop);
        BldgVolumeSum += zone.Volume * zone.Multiplier * zone.ListMultiplier;
        Real64 PerimExtLengthSum = 0.0; // init
        int ExtWallCount = 0;           // init
        int ExtWindowCount = 0;         // init
        // model perimeter of bounding horizontal rectangle from max and min x and y values
        Real64 thisZoneSimplePerim = 2.0 * (zone.MaximumY - zone.MinimumY) + 2.0 * (zone.MaximumX - zone.MinimumX);
        if (thisZoneSimplePerim > 0.0) {
            thisZoneHorizHydralicDiameter = 4.0 * zone.FloorArea / thisZoneSimplePerim;
        } else if (zone.FloorArea > 0.0) {
            thisZoneHorizHydralicDiameter = std::sqrt(zone.FloorArea);
        }

        Real64 thisWWR = (zone.ExtGrossWallArea > 0.0) ? (zone.ExtWindowArea / zone.ExtGrossWallArea) : -999.0; // throw error?

        for (int spaceNum : zone.spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            // first pass thru this zones surfaces to gather data
            for (int SurfLoop = thisSpace.HTSurfaceFirst; SurfLoop <= thisSpace.HTSurfaceLast; ++SurfLoop) {
                auto const &surf = state.dataSurface->Surface(SurfLoop);
                // first catch exterior walls and do summations
                if (surf.ExtBoundCond != ExternalEnvironment) {
                    continue;
                }

                if (surf.Class == SurfaceClass::Wall) {
                    PerimExtLengthSum += surf.Width;
                    ++ExtWallCount;
                } else if (surf.Class == SurfaceClass::Window || surf.Class == SurfaceClass::GlassDoor) {
                    ++ExtWindowCount;
                }
            }
        }
        for (int spaceNum : state.dataHeatBal->Zone(ZoneLoop).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            // second pass thru zone surfs to fill data
            for (int SurfLoop = thisSpace.HTSurfaceFirst; SurfLoop <= thisSpace.HTSurfaceLast; ++SurfLoop) {
                auto &surfIntConv = state.dataSurface->surfIntConv(SurfLoop);
                surfIntConv.zoneWallHeight = zone.CeilingHeight;
                surfIntConv.zonePerimLength = PerimExtLengthSum;
                surfIntConv.zoneHorizHydrDiam = thisZoneHorizHydralicDiameter;
                surfIntConv.windowWallRatio = thisWWR;
            } // 2nd pass over surfaces.
        }

        // third pass for window locations
        if ((ExtWindowCount > 0) && (ExtWallCount > 0)) {
            for (int spaceNum : state.dataHeatBal->Zone(ZoneLoop).spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);

                for (int SurfLoop = thisSpace.HTSurfaceFirst; SurfLoop <= thisSpace.HTSurfaceLast; ++SurfLoop) {
                    auto const &surf = state.dataSurface->Surface(SurfLoop);
                    auto &surfIntConv = state.dataSurface->surfIntConv(SurfLoop);
                    if (surf.ExtBoundCond != ExternalEnvironment) {
                        continue;
                    }

                    if (surf.Class == SurfaceClass::Window || surf.Class == SurfaceClass::GlassDoor) {

                        if (surfIntConv.windowWallRatio < 0.5) {
                            surfIntConv.windowLocation =
                                (surf.Centroid.z < zone.Centroid.z) ? IntConvWinLoc::LowerPartOfExteriorWall : IntConvWinLoc::UpperPartOfExteriorWall;
                        } else {
                            surfIntConv.windowLocation = IntConvWinLoc::LargePartOfExteriorWall;
                        }

                        auto const &baseSurf = state.dataSurface->Surface(surf.BaseSurf);
                        auto &baseSurfIntConv = state.dataSurface->surfIntConv(surf.BaseSurf);
                        if (baseSurf.ExtBoundCond != ExternalEnvironment) {
                            continue;
                        }
                        if (baseSurf.Class != SurfaceClass::Wall) {
                            continue;
                        }

                        baseSurfIntConv.windowLocation =
                            (baseSurf.Centroid.z < surf.Centroid.z) ? IntConvWinLoc::WindowAboveThis : IntConvWinLoc::WindowBelowThis;

                    } else if (surf.Class == SurfaceClass::Wall && surfIntConv.windowLocation == IntConvWinLoc::NotSet) {
                        surfIntConv.windowLocation =
                            (surf.Centroid.z < zone.Centroid.z) ? IntConvWinLoc::WindowAboveThis : IntConvWinLoc::WindowBelowThis;
                    }
                }
            } // third pass over surfaces
        }
    } // loop over zones for inside face parameters

    state.dataConvect->CubeRootOfOverallBuildingVolume = std::pow(BldgVolumeSum, 1.0 / 3.0);

    SurfaceGeometry::GeoSummary geoSummaryRoof;
    SurfaceGeometry::GetGeoSummaryRoof(state, geoSummaryRoof);

    state.dataConvect->RoofLongAxisOutwardAzimuth = geoSummaryRoof.Azimuth;

    // Calculate facade areas, perimeters, and heights.
    // Why are these calculations so quick and dirty while the roof calcluation is much more detailed?
    std::array<SurfaceGeometry::GeoSummary, (int)DataSurfaces::Compass8::Num> geoSummaryFacades;

    // first pass over surfaces for outside face params
    for (auto const &surf : state.dataSurface->Surface) {
        if (surf.ExtBoundCond != ExternalEnvironment) {
            continue;
        }
        if (!surf.HeatTransSurf) {
            continue;
        }
        if ((surf.Tilt < 45.0) || (surf.Tilt >= 135.0)) {
            continue; // not a vertical wall
        }

        DataSurfaces::Compass8 compass8 = AzimuthToCompass8(surf.Azimuth);

        Real64 x_min = Constant::BigNumber;
        Real64 x_max = -Constant::BigNumber;
        Real64 y_min = Constant::BigNumber;
        Real64 y_max = -Constant::BigNumber;
        Real64 z_min = Constant::BigNumber;
        Real64 z_max = -Constant::BigNumber;

        for (auto const &v : surf.Vertex) {
            if (v.x < x_min) {
                x_min = v.x;
            } else if (v.x > x_max) {
                x_max = v.x;
            }
            if (v.y < y_min) {
                y_min = v.y;
            } else if (v.y > y_max) {
                y_max = v.y;
            }
            if (v.z < z_min) {
                z_min = v.z;
            } else if (v.z > z_max) {
                z_max = v.z;
            }
        }

        auto &facade = geoSummaryFacades[(int)compass8];
        facade.Area += surf.Area;
        facade.Zmax = max(z_max, facade.Zmax);
        facade.Zmin = min(z_min, facade.Zmin);
        facade.Ymax = max(y_max, facade.Ymax);
        facade.Ymin = min(y_min, facade.Ymin);
        facade.Xmax = max(x_max, facade.Xmax);
        facade.Xmin = min(x_min, facade.Xmin);
    } // fist loop over surfaces for outside face params

    for (auto &facade : geoSummaryFacades) {
        facade.Perimeter = 2.0 * std::sqrt(pow_2(facade.Xmax - facade.Xmin) + pow_2(facade.Ymax - facade.Ymin)) + 2.0 * (facade.Zmax - facade.Zmin);
        facade.Height = facade.Zmax - facade.Zmin;
    }
    for (int surfNum = 1; surfNum <= state.dataSurface->TotSurfaces; ++surfNum) {
        auto const &surf = state.dataSurface->Surface(surfNum);
        if (surf.ExtBoundCond != ExternalEnvironment) {
            continue;
        }
        if (!surf.HeatTransSurf) {
            continue;
        }

        Real64 z_min = Constant::BigNumber;
        Real64 z_max = -Constant::BigNumber;
        for (auto const &v : surf.Vertex) {
            if (v.z < z_min) {
                z_min = v.z;
            } else if (v.z > z_max) {
                z_max = v.z;
            }
        }
        Real64 z_del = z_max - z_min;

        auto &surfExtConv = state.dataSurface->surfExtConv(surfNum);
        if ((surf.Tilt >= 45.0) && (surf.Tilt < 135.0)) { // treat as vertical wall
            DataSurfaces::Compass8 compass8 = AzimuthToCompass8(surf.Azimuth);
            auto const &facade = geoSummaryFacades[(int)compass8];

            surfExtConv.faceArea = max(facade.Area, surf.GrossArea);
            surfExtConv.facePerimeter = max(facade.Perimeter, surf.Perimeter);
            surfExtConv.faceHeight = max(facade.Height, z_del);
        } else if (surf.Tilt < 45.0) { // assume part of roof
            surfExtConv.faceArea = max(geoSummaryRoof.Area, surf.GrossArea);
            surfExtConv.facePerimeter = max(geoSummaryRoof.Perimeter, surf.Perimeter);
            surfExtConv.faceHeight = max(geoSummaryRoof.Height, z_del);
        } else if (surf.Tilt >= 135.0) { // assume floor over exterior, just use surface's geometry
            surfExtConv.faceArea = surf.GrossArea;
            surfExtConv.facePerimeter = surf.Perimeter;
            surfExtConv.faceHeight = z_del;
        }
    } // second pass thru surfs for outside face convection params.

    // now send to EIO if surface reporting selected
    General::ScanForReports(state, "Surfaces", DoReport, "Details");
    if (DoReport) { // echo out static geometry data related to convection models
        static constexpr std::string_view Format_900("! <Surface Convection Parameters>, Surface Name, Outside Model Assignment, Outside "
                                                     "Area [m2], Outside Perimeter [m], Outside Height "
                                                     "[m], Inside Model Assignment, Inside Height [m], Inside Perimeter Envelope [m], Inside "
                                                     "Hydraulic Diameter [m], Window Wall Ratio, "
                                                     "Window Location, Near Radiant {{Yes/No}}, Has Active HVAC {{Yes/No}}\n");
        print(state.files.eio, Format_900); // header
        for (int SurfLoop : state.dataSurface->AllSurfaceListReportOrder) {
            auto const &surf = state.dataSurface->Surface(SurfLoop);
            auto const &surfExtConv = state.dataSurface->surfExtConv(SurfLoop);
            auto const &surfIntConv = state.dataSurface->surfIntConv(SurfLoop);

            if (!surf.HeatTransSurf) {
                continue;
            }

            static constexpr std::string_view Format_901(
                "Surface Convection Parameters,{},{},{:.2R},{:.2R},{:.2R},{},{:.2R},{:.2R},{:.2R},{:.2R},{},{},{}\n");

            // This reporting rubric (using numbers instead of strings, using negative numbers for "built-in" coefficients) is stupid,
            // but we are maintaining compatiblity here
            int hcExtRptNum = surfExtConv.userModelNum;
            if (hcExtRptNum == 0) {
                hcExtRptNum = -Convect::HcExtReportVals[(int)surfExtConv.model];
            }

            int hcIntRptNum = surfIntConv.userModelNum;
            if (hcIntRptNum == 0) {
                hcIntRptNum = -Convect::HcIntReportVals[(int)surfIntConv.model];
            }

            print(state.files.eio,
                  Format_901,
                  surf.Name,
                  hcExtRptNum,
                  surfExtConv.faceArea,
                  surfExtConv.facePerimeter,
                  surfExtConv.faceHeight,
                  hcIntRptNum,
                  surfIntConv.zoneWallHeight,
                  surfIntConv.zonePerimLength,
                  surfIntConv.zoneHorizHydrDiam,
                  surfIntConv.windowWallRatio,
                  surfIntConv.windowLocation,
                  surfIntConv.getsRadiantHeat ? "Yes" : "No",
                  surfIntConv.hasActiveInIt ? "Yes" : "No");

            // [m] length of perimeter zone's exterior wall | [m] hydraulic diameter, usually 4 times the zone floor area div by
            // perimeter | [-] area of windows over area of exterior wall for zone | relative location of window in zone for
            // interior Hc models
        }

        // if display advanced reports also dump meta group data used for convection geometry
        if (state.dataGlobal->DisplayAdvancedReportVariables) {
            static constexpr std::string_view Format_8000 =
                "! <Building Convection Parameters:{} Facade>, Perimeter, Height, Xmin, Xmax, Ymin, Ymax, Zmin, Zmax \n";
            static constexpr std::string_view Format_8001 =
                "Building Convection Parameters:{} Facade, {:.2R},{:.2R},{:.2R},{:.2R},{:.2R},{:.2R},{:.2R},{:.2R}\n";

            for (int c8 = 0; c8 < (int)DataSurfaces::Compass8::Num; ++c8) {

                // header for north facade
                print(state.files.eio, Format_8000, DataSurfaces::compass8Names[c8]);

                auto const &facade = geoSummaryFacades[c8];
                print(state.files.eio,
                      Format_8001,
                      DataSurfaces::compass8Names[c8],
                      facade.Perimeter,
                      facade.Height,
                      facade.Xmin,
                      facade.Xmax,
                      facade.Ymin,
                      facade.Ymax,
                      facade.Zmin,
                      facade.Zmax);
            }

            static constexpr std::string_view Format_8800(
                "! <Building Convection Parameters:Roof>, Area [m2], Perimeter [m], Height [m], Tilt [deg], Azimuth [deg]\n");
            print(state.files.eio, Format_8800); // header for roof
            static constexpr std::string_view Format_8801("Building Convection Parameters:Roof,{:.2R},{:.2R},{:.2R},{:.2R},{:.2R}\n");
            print(state.files.eio,
                  Format_8801,
                  geoSummaryRoof.Area,
                  geoSummaryRoof.Perimeter,
                  geoSummaryRoof.Height,
                  geoSummaryRoof.Tilt,
                  geoSummaryRoof.Azimuth);
        } // Display
    } // Do Report
}

void SetupAdaptiveConvRadiantSurfaceData(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Sept 2011

    // PURPOSE OF THIS SUBROUTINE:
    // identify Zones that have active radiant elements for convection model classifications

    // METHODOLOGY EMPLOYED:
    // Need to fill in values for ZoneEquipConfig%InWallActiveElement, ZoneEquipConfig%InCeilingActiveElement
    // and ZoneEquipConfig(ZoneNum)%InFloorActiveElement.

    for (int ZoneLoop = 1; ZoneLoop <= state.dataGlobal->NumOfZones; ++ZoneLoop) {
        int activeWallCount = 0;
        Real64 activeWallArea = 0.0;
        int activeCeilingCount = 0;
        Real64 activeCeilingArea = 0.0;
        int activeFloorCount = 0;
        Real64 activeFloorArea = 0.0;

        auto const &zone = state.dataHeatBal->Zone(ZoneLoop);
        for (int spaceNum : zone.spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            for (int SurfLoop = thisSpace.HTSurfaceFirst; SurfLoop <= thisSpace.HTSurfaceLast; ++SurfLoop) {
                auto const &surface = state.dataSurface->Surface(SurfLoop);
                if (!state.dataSurface->surfIntConv(SurfLoop).hasActiveInIt) {
                    continue;
                }
                if (surface.Class == SurfaceClass::Wall || surface.Class == SurfaceClass::Door) {
                    ++activeWallCount;
                    activeWallArea += surface.Area;
                } else if (surface.Class == SurfaceClass::Roof) {
                    ++activeCeilingCount;
                    activeCeilingArea += surface.Area;
                } else if (surface.Class == SurfaceClass::Floor) {
                    ++activeFloorCount;
                    activeFloorArea += surface.Area;
                }
            }
        } // surface loop

        auto &zoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(ZoneLoop);
        zoneEquipConfig.InWallActiveElement = (activeWallCount > 0 && activeWallArea > 0.0);
        zoneEquipConfig.InCeilingActiveElement = (activeCeilingCount > 0 && activeCeilingArea > 0.0);
        zoneEquipConfig.InFloorActiveElement = (activeFloorCount > 0 && activeFloorArea > 0);
    } // zone loop
}

void ManageIntAdaptiveConvAlgo(EnergyPlusData &state,
                               int const SurfNum) // surface number for which coefficients are being calculated
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine manages the calculation of interior convection coefficient for a surface.

    // METHODOLOGY EMPLOYED:
    // This routine implements the Adaptive Convection Algorithm developed by IB-M 2000 and IB-M 2002
    //  - first calls a large routine, DynamicIntConvSurfaceClassification, that has most of the complex logic
    //  - then calls a straightforward routine that maps the classification to model equation
    //  - then calls a routine with a large case statement that calls model equations.

    // USE STATEMENTS:

    // this next call sets up the flow regime and assigns a classification to surface
    //  TODO: candidate for rework to do zone level calcs once rather than for each surface
    DynamicIntConvSurfaceClassification(state, SurfNum);

    // simple worker routine takes surface classification and fills in model to use (IntConvHcModelEq) for that surface
    MapIntConvClassToHcModels(state, SurfNum);

    state.dataHeatBalSurf->SurfHConvInt(SurfNum) = EvaluateIntHcModels(state, SurfNum, state.dataSurface->surfIntConv(SurfNum).hcModelEq);
}

Real64 ManageExtAdaptiveConvAlgo(EnergyPlusData &state,
                                 int const SurfNum) // surface number for which coefficients are being calculated
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine calculates the convection coefficient for the outside face of a surface

    // METHODOLOGY EMPLOYED:
    // This routine implements an adaptive structure and classification system for outdoor
    //   It calls a series of separable worker routines

    DynamicExtConvSurfaceClassification(state, SurfNum);

    MapExtConvClassToHcModels(state, SurfNum);

    auto const &surfExtConv = state.dataSurface->surfExtConv(SurfNum);
    return EvaluateExtHcModels(state, SurfNum, surfExtConv.hnModelEq, surfExtConv.hfModelEq);
}

Real64 EvaluateIntHcModels(EnergyPlusData &state, int const SurfNum, HcInt const ConvModelEquationNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS SUBROUTINE:
    // central case statement for calling inside convection models

    // METHODOLOGY EMPLOYED:
    //  - fills value for Hc by calling the appropriate convection model, usually as a function.
    //     preparation of argument values for the function calls is contained in each Case block (repeats)
    //  - also updates the reference air temperature type for use in the surface heat balance calcs

    Real64 tmpHc = 0.0;

    auto const &thisSurface = state.dataSurface->Surface(SurfNum);
    auto const &surfIntConv = state.dataSurface->surfIntConv(SurfNum);

    int const ZoneNum = thisSurface.Zone;
    int const spaceNum = thisSurface.spaceNum;
    Real64 const Tsurface = state.dataHeatBalSurf->SurfInsideTempHist(1)(SurfNum);
    Real64 const Tzone = state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum).MAT;

    auto &HnFn = state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].in;
    // now call appropriate function to calculate Hc
    switch (ConvModelEquationNum) {

    case HcInt::UserCurve: {
        tmpHc = CalcUserDefinedIntHcModel(state, SurfNum, surfIntConv.hcUserCurveNum);
    } break;

    case HcInt::ASHRAEVerticalWall: {
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            HnFn = [](double Tsurf, double Tamb, double, double, double) -> double { return CalcASHRAEVerticalWall(Tsurf - Tamb); };
        } else {
            tmpHc = CalcASHRAEVerticalWall((Tsurface - Tzone));
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;

    } break;

    case HcInt::WaltonUnstableHorizontalOrTilt: {
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            HnFn = [](double Tsurf, double Tamb, double, double, double cosTilt) -> double {
                return CalcWaltonUnstableHorizontalOrTilt(Tsurf - Tamb, cosTilt);
            };
        } else {
            tmpHc = CalcWaltonUnstableHorizontalOrTilt((Tsurface - Tzone), thisSurface.CosTilt); // TODO verify CosTilt in vs out
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::WaltonStableHorizontalOrTilt: {
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            HnFn = [](double Tsurf, double Tamb, double, double, double cosTilt) -> double {
                return CalcWaltonStableHorizontalOrTilt(Tsurf - Tamb, cosTilt);
            };
        } else {
            tmpHc = CalcWaltonStableHorizontalOrTilt((Tsurface - Tzone), thisSurface.CosTilt); // TODO verify CosTilt in vs out
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::FisherPedersenCeilDiffuserFloor: {
        Real64 AirChangeRate = CalcCeilingDiffuserACH(state, ZoneNum);
        Real64 AirHumRat = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).airHumRatAvg;
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {

            HnFn = [=, &state](double Tsurf, double Tamb, double, double, double cosTilt) -> double {
                return CalcFisherPedersenCeilDiffuserFloor(state, AirChangeRate, Tsurf, Tamb, cosTilt, AirHumRat, thisSurface.Height);
            };
        } else {
            tmpHc = CalcFisherPedersenCeilDiffuserFloor(state,
                                                        AirChangeRate,
                                                        Tsurface,
                                                        Tzone,
                                                        thisSurface.CosTilt,
                                                        AirHumRat,
                                                        thisSurface.Height,
                                                        state.dataConstruction->Construct(thisSurface.Construction).TypeIsWindow);
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::FisherPedersenCeilDiffuserCeiling: {
        Real64 AirChangeRate = CalcCeilingDiffuserACH(state, ZoneNum);
        Real64 AirHumRat = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).airHumRatAvg;
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {

            HnFn = [=, &state](double Tsurf, double Tamb, double, double, double cosTilt) -> double {
                return CalcFisherPedersenCeilDiffuserCeiling(state, AirChangeRate, Tsurf, Tamb, cosTilt, AirHumRat, thisSurface.Height);
            };
        } else {
            tmpHc = CalcFisherPedersenCeilDiffuserCeiling(state,
                                                          AirChangeRate,
                                                          Tsurface,
                                                          Tzone,
                                                          thisSurface.CosTilt,
                                                          AirHumRat,
                                                          thisSurface.Height,
                                                          state.dataConstruction->Construct(thisSurface.Construction).TypeIsWindow);
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::FisherPedersenCeilDiffuserWalls: {
        Real64 AirChangeRate = CalcCeilingDiffuserACH(state, ZoneNum);
        Real64 AirHumRat = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).airHumRatAvg;
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {

            HnFn = [=, &state](double Tsurf, double Tamb, double, double, double cosTilt) -> double {
                return CalcFisherPedersenCeilDiffuserWalls(state, AirChangeRate, Tsurf, Tamb, cosTilt, AirHumRat, thisSurface.Height);
            };
        } else {
            tmpHc = CalcFisherPedersenCeilDiffuserWalls(state,
                                                        AirChangeRate,
                                                        Tsurface,
                                                        Tzone,
                                                        thisSurface.CosTilt,
                                                        AirHumRat,
                                                        thisSurface.Height,
                                                        state.dataConstruction->Construct(thisSurface.Construction).TypeIsWindow);
        }
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            HnFn = [=](double, double, double, double, double) -> double { return tmpHc; };
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::AlamdariHammondStableHorizontal: {
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            Real64 HorizHydrDiam = surfIntConv.zoneHorizHydrDiam;
            HnFn = [=](double Tsurf, double Tamb, double, double, double) -> double {
                return CalcAlamdariHammondStableHorizontal(Tsurf - Tamb, HorizHydrDiam);
            };
        } else {
            tmpHc = CalcAlamdariHammondStableHorizontal(state, (Tsurface - Tzone), surfIntConv.zoneHorizHydrDiam, SurfNum);
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::AlamdariHammondVerticalWall: {
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            Real64 WallHeight = surfIntConv.zoneWallHeight;
            HnFn = [=](double Tsurf, double Tamb, double, double, double) -> double {
                return CalcAlamdariHammondVerticalWall(Tsurf - Tamb, WallHeight);
            };
        } else {
            tmpHc = CalcAlamdariHammondVerticalWall(state, (Tsurface - Tzone), surfIntConv.zoneWallHeight, SurfNum);
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::AlamdariHammondUnstableHorizontal: {
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            Real64 HorizHydrDiam = surfIntConv.zoneHorizHydrDiam;
            HnFn = [=](double Tsurf, double Tamb, double, double, double) -> double {
                return CalcAlamdariHammondStableHorizontal(Tsurf - Tamb, HorizHydrDiam);
            };
        } else {
            tmpHc = CalcAlamdariHammondUnstableHorizontal(state, (Tsurface - Tzone), surfIntConv.zoneHorizHydrDiam, SurfNum);
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::KhalifaEq3WallAwayFromHeat: {
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            HnFn = [=](double Tsurf, double Tamb, double, double, double) -> double { return CalcKhalifaEq3WallAwayFromHeat(Tsurf - Tamb); };
        } else {
            tmpHc = CalcKhalifaEq3WallAwayFromHeat((Tsurface - Tzone));
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::KhalifaEq4CeilingAwayFromHeat: {
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            HnFn = [=](double Tsurf, double Tamb, double, double, double) -> double { return CalcKhalifaEq4CeilingAwayFromHeat(Tsurf - Tamb); };
        } else {
            tmpHc = CalcKhalifaEq4CeilingAwayFromHeat((Tsurface - Tzone));
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::KhalifaEq5WallNearHeat: {
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            HnFn = [=](double Tsurf, double Tamb, double, double, double) -> double { return CalcKhalifaEq5WallsNearHeat(Tsurf - Tamb); };
        } else {
            tmpHc = CalcKhalifaEq5WallsNearHeat((Tsurface - Tzone));
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::KhalifaEq6NonHeatedWalls: {
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            HnFn = [=](double Tsurf, double Tamb, double, double, double) -> double { return CalcKhalifaEq6NonHeatedWalls(Tsurf - Tamb); };
        } else {
            tmpHc = CalcKhalifaEq6NonHeatedWalls((Tsurface - Tzone));
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::KhalifaEq7Ceiling: {
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            HnFn = [=](double Tsurf, double Tamb, double, double, double) -> double { return CalcKhalifaEq7Ceiling(Tsurf - Tamb); };
        } else {
            tmpHc = CalcKhalifaEq7Ceiling((Tsurface - Tzone));
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::AwbiHattonHeatedFloor: {
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            Real64 HorizHydrDiam = surfIntConv.zoneHorizHydrDiam;
            HnFn = [=](double Tsurf, double Tamb, double, double, double) -> double {
                return CalcAwbiHattonHeatedFloor(Tsurf - Tamb, HorizHydrDiam);
            };
        } else {
            tmpHc = CalcAwbiHattonHeatedFloor((Tsurface - Tzone), surfIntConv.zoneHorizHydrDiam);
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::AwbiHattonHeatedWall: {
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            Real64 HorizHydrDiam = surfIntConv.zoneHorizHydrDiam;
            HnFn = [=](double Tsurf, double Tamb, double, double, double) -> double { return CalcAwbiHattonHeatedWall(Tsurf - Tamb, HorizHydrDiam); };
        } else {
            tmpHc = CalcAwbiHattonHeatedWall((Tsurface - Tzone), surfIntConv.zoneHorizHydrDiam);
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::BeausoleilMorrisonMixedAssistingWall: {
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            HnFn = [=, &state](double Tsurf, double Tamb, double, double, double) -> double {
                return CalcBeausoleilMorrisonMixedAssistedWall(
                    Tsurf - Tamb, surfIntConv.zoneWallHeight, Tsurf, CalcZoneSupplyAirTemp(state, ZoneNum), CalcZoneSystemACH(state, ZoneNum));
            };
        } else {
            tmpHc = CalcBeausoleilMorrisonMixedAssistedWall(state, (Tsurface - Tzone), surfIntConv.zoneWallHeight, Tsurface, ZoneNum);
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::BeausoleilMorrisonMixedOppossingWall: {
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            HnFn = [=, &state](double Tsurf, double Tamb, double, double, double) -> double {
                return CalcBeausoleilMorrisonMixedOpposingWall(
                    Tsurf - Tamb, surfIntConv.zoneWallHeight, Tsurf, CalcZoneSupplyAirTemp(state, ZoneNum), CalcZoneSystemACH(state, ZoneNum));
            };
        } else {
            tmpHc = CalcBeausoleilMorrisonMixedOpposingWall(state, (Tsurface - Tzone), surfIntConv.zoneWallHeight, Tsurface, ZoneNum);
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::BeausoleilMorrisonMixedStableCeiling: {
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            HnFn = [=, &state](double Tsurf, double Tamb, double, double, double) -> double {
                return CalcBeausoleilMorrisonMixedStableCeiling(
                    Tsurf - Tamb, surfIntConv.zoneHorizHydrDiam, Tsurf, CalcZoneSupplyAirTemp(state, ZoneNum), CalcZoneSystemACH(state, ZoneNum));
            };
        } else {
            tmpHc = CalcBeausoleilMorrisonMixedStableCeiling(state, (Tsurface - Tzone), surfIntConv.zoneHorizHydrDiam, Tsurface, ZoneNum);
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::BeausoleilMorrisonMixedUnstableCeiling: {
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            HnFn = [=, &state](double Tsurf, double Tamb, double, double, double) -> double {
                return CalcBeausoleilMorrisonMixedUnstableCeiling(
                    Tsurf - Tamb, surfIntConv.zoneHorizHydrDiam, Tsurf, CalcZoneSupplyAirTemp(state, ZoneNum), CalcZoneSystemACH(state, ZoneNum));
            };
        } else {
            tmpHc = CalcBeausoleilMorrisonMixedUnstableCeiling(state, (Tsurface - Tzone), surfIntConv.zoneHorizHydrDiam, Tsurface, ZoneNum);
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::BeausoleilMorrisonMixedStableFloor: {
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            HnFn = [=, &state](double Tsurf, double Tamb, double, double, double) -> double {
                return CalcBeausoleilMorrisonMixedStableFloor(
                    Tsurf - Tamb, surfIntConv.zoneHorizHydrDiam, Tsurf, CalcZoneSupplyAirTemp(state, ZoneNum), CalcZoneSystemACH(state, ZoneNum));
            };
        } else {
            tmpHc = CalcBeausoleilMorrisonMixedStableFloor(state, (Tsurface - Tzone), surfIntConv.zoneHorizHydrDiam, Tsurface, ZoneNum);
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::BeausoleilMorrisonMixedUnstableFloor: {
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            HnFn = [=, &state](double Tsurf, double Tamb, double, double, double) -> double {
                return CalcBeausoleilMorrisonMixedUnstableFloor(
                    Tsurf - Tamb, surfIntConv.zoneHorizHydrDiam, Tsurf, CalcZoneSupplyAirTemp(state, ZoneNum), CalcZoneSystemACH(state, ZoneNum));
            };
        } else {
            tmpHc = CalcBeausoleilMorrisonMixedUnstableFloor(state, (Tsurface - Tzone), surfIntConv.zoneHorizHydrDiam, Tsurface, ZoneNum);
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::FohannoPolidoriVerticalWall: {
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            Real64 QdotConvection = -state.dataHeatBalSurf->SurfQdotConvInPerArea(SurfNum);
            Real64 WallHeight = surfIntConv.zoneWallHeight;
            HnFn = [=](double Tsurf, double Tamb, double, double, double) -> double {
                return CalcFohannoPolidoriVerticalWall(Tsurf - Tamb,
                                                       WallHeight,
                                                       Tsurf - Constant::Kelvin, // Kiva already uses Kelvin, but algorithm expects C
                                                       QdotConvection);
            };
        } else {
            tmpHc = CallCalcFohannoPolidoriVerticalWall(
                state, (Tsurface - Tzone), surfIntConv.zoneWallHeight, Tsurface, -state.dataHeatBalSurf->SurfQdotConvInPerArea(SurfNum), SurfNum);
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::KaradagChilledCeiling: {
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            HnFn = [=](double Tsurf, double Tamb, double, double, double) -> double { return CalcKaradagChilledCeiling(Tsurf - Tamb); };
        } else {
            tmpHc = CalcKaradagChilledCeiling((Tsurface - Tzone));
        }
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::ISO15099Windows: {
        CalcISO15099WindowIntConvCoeff(state, SurfNum, Tsurface, Tzone);
        tmpHc = state.dataHeatBalSurf->SurfHConvInt(SurfNum);
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
    } break;

    case HcInt::GoldsteinNovoselacCeilingDiffuserWindow: {
        tmpHc = CalcGoldsteinNovoselacCeilingDiffuserWindow(
            state, surfIntConv.zonePerimLength, surfIntConv.windowWallRatio, surfIntConv.windowLocation, ZoneNum);
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            HnFn = [=](double, double, double, double, double) -> double { return tmpHc; };
        }
        if (state.dataHeatBal->Zone(ZoneNum).SystemZoneNodeNumber > 0) {
            state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneSupplyAirTemp;
        } else {
            state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
        }
    } break;

    case HcInt::GoldsteinNovoselacCeilingDiffuserWalls: {
        tmpHc = CalcGoldsteinNovoselacCeilingDiffuserWall(state, surfIntConv.zonePerimLength, surfIntConv.windowLocation, ZoneNum);
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            HnFn = [=](double, double, double, double, double) -> double { return tmpHc; };
        }
        if (state.dataHeatBal->Zone(ZoneNum).SystemZoneNodeNumber > 0) {
            state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneSupplyAirTemp;
        } else {
            state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
        }
    } break;

    case HcInt::GoldsteinNovoselacCeilingDiffuserFloor: {
        tmpHc = CalcGoldsteinNovoselacCeilingDiffuserFloor(surfIntConv.zonePerimLength, ZoneNum);
        if (thisSurface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            HnFn = [=](double, double, double, double, double) -> double { return tmpHc; };
        }
        if (state.dataHeatBal->Zone(ZoneNum).SystemZoneNodeNumber > 0) {
            state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneSupplyAirTemp;
        } else {
            state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
        }
    } break;
    default: {
        assert(false);
    } break;
    } // switch (ConvModelEqNum)

    state.dataSurface->SurfTAirRefRpt(SurfNum) = DataSurfaces::SurfTAirRefReportVals[state.dataSurface->SurfTAirRef(SurfNum)];

    if (tmpHc < AdaptiveHcIntLowLimit) {
        tmpHc = AdaptiveHcIntLowLimit;
    }

    return tmpHc;
}

Real64 EvaluateExtHcModels(EnergyPlusData &state, int const SurfNum, HcExt const NaturalConvModelEqNum, HcExt const ForcedConvModelEqNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS SUBROUTINE:
    // central case statement for evaluating exterior specific convection models

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 Hf(0.0); // the forced, or wind driven portion of film coefficient
    Real64 Hn(0.0); // the natural, or buoyancy driven portion of film coefficient
    Real64 SurfWindSpeed;
    Real64 SurfWindDir;
    Real64 HydraulicDiameter;

    // Kiva callback functions
    Kiva::ForcedConvectionTerm HfTermFn;
    Kiva::ConvectionAlgorithm HfFn(KIVA_CONST_CONV(0.0));
    Kiva::ConvectionAlgorithm HnFn(KIVA_CONST_CONV(0.0));

    auto const &surface = state.dataSurface->Surface(SurfNum);
    auto const &surfExtConv = state.dataSurface->surfExtConv(SurfNum);
    auto const &SurfQdotConvOutRepPerArea = state.dataHeatBalSurf->SurfQdotConvOutPerArea;
    Real64 SurfOutTemp = state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum);

    // first call Hn models
    switch (NaturalConvModelEqNum) {
    case HcExt::None: {
        Hn = 0.0;
        HnFn = KIVA_CONST_CONV(0.0);
    } break;

    case HcExt::UserCurve: {
        Hn = CalcUserDefinedExtHcModel(state, SurfNum, surfExtConv.hnUserCurveNum);
        if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            HnFn = [=, &state](double Tsurf, double Tamb, double HfTerm, double Roughness, double CosTilt) -> double {
                // Remove Hfterm since this is only used for the natural convection portion
                return state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].out(Tsurf, Tamb, HfTerm, Roughness, CosTilt) - HfTerm;
            };
        }
    } break;

    case HcExt::NaturalASHRAEVerticalWall: {
        Hn = CalcASHRAEVerticalWall((SurfOutTemp - state.dataSurface->SurfOutDryBulbTemp(SurfNum)));
        HnFn = [=](double Tsurf, double Tamb, double, double, double) -> double { return CalcASHRAEVerticalWall(Tsurf - Tamb); };
    } break;

    case HcExt::NaturalWaltonUnstableHorizontalOrTilt: {
        Hn = CalcWaltonUnstableHorizontalOrTilt((SurfOutTemp - state.dataSurface->SurfOutDryBulbTemp(SurfNum)),
                                                surface.CosTilt); // TODO verify CosTilt in vs out
        HnFn = [=](double Tsurf, double Tamb, double, double, double cosTilt) -> double {
            return CalcWaltonUnstableHorizontalOrTilt(Tsurf - Tamb, cosTilt);
        };
    } break;

    case HcExt::NaturalWaltonStableHorizontalOrTilt: {
        Hn = CalcWaltonStableHorizontalOrTilt((SurfOutTemp - state.dataSurface->SurfOutDryBulbTemp(SurfNum)),
                                              surface.CosTilt); // TODO verify CosTilt in vs out
        HnFn = [=](double Tsurf, double Tamb, double, double, double cosTilt) -> double {
            return CalcWaltonStableHorizontalOrTilt(Tsurf - Tamb, cosTilt);
        };
    } break;

    case HcExt::AlamdariHammondVerticalWall: {
        Real64 FaceHeight = surfExtConv.faceHeight;
        Hn = CalcAlamdariHammondVerticalWall(state, (SurfOutTemp - state.dataSurface->SurfOutDryBulbTemp(SurfNum)), FaceHeight, SurfNum);
        HnFn = [=](double Tsurf, double Tamb, double, double, double) -> double { return CalcAlamdariHammondVerticalWall(Tsurf - Tamb, FaceHeight); };
    } break;

    case HcExt::FohannoPolidoriVerticalWall: {
        if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            // Not compatible with Kiva (Exterior surfaces in Kiva are not currently reported. Also need to add cell-level convection.)
            ShowFatalError(state, format("Fohanno Polidori convection model not applicable for foundation surface ={}", surface.Name));
        }
        Hn = CallCalcFohannoPolidoriVerticalWall(state,
                                                 (SurfOutTemp - state.dataSurface->SurfOutDryBulbTemp(SurfNum)),
                                                 surfExtConv.faceHeight,
                                                 SurfOutTemp,
                                                 -SurfQdotConvOutRepPerArea(SurfNum),
                                                 SurfNum);
    } break;

    case HcExt::AlamdariHammondStableHorizontal: {
        if (surfExtConv.facePerimeter > 0.0) {
            HydraulicDiameter = 4.0 * surfExtConv.faceArea / surfExtConv.facePerimeter;
        } else {
            HydraulicDiameter = std::sqrt(surfExtConv.faceArea);
        }
        Hn = CalcAlamdariHammondStableHorizontal(state, (SurfOutTemp - state.dataSurface->SurfOutDryBulbTemp(SurfNum)), HydraulicDiameter, SurfNum);
    } break;

    case HcExt::AlamdariHammondUnstableHorizontal: {
        if (surfExtConv.facePerimeter > 0.0) {
            HydraulicDiameter = 4.0 * surfExtConv.faceArea / surfExtConv.facePerimeter;
        } else {
            HydraulicDiameter = std::sqrt(surfExtConv.faceArea);
        }
        Hn = CalcAlamdariHammondUnstableHorizontal(state, (SurfOutTemp - state.dataSurface->SurfOutDryBulbTemp(SurfNum)), HydraulicDiameter, SurfNum);
    } break;
    default: {
        assert(false);
    }
    } // switch (NaturalConvModelEqNum)

    if (!surface.ExtWind) {
        SurfWindSpeed = 0.0; // No wind exposure
    } else if (surface.Class == SurfaceClass::Window && state.dataSurface->SurfWinShadingFlag(SurfNum) == WinShadingType::ExtShade) {
        SurfWindSpeed = 0.0; // Assume zero wind speed at outside glass surface of window with exterior shade
    } else {
        SurfWindSpeed = state.dataSurface->SurfOutWindSpeed(SurfNum);
    }

    Material::SurfaceRoughness Roughness =
        state.dataMaterial->materials(state.dataConstruction->Construct(surface.Construction).LayerPoint(1))->Roughness;

    switch (ForcedConvModelEqNum) {
    case HcExt::None: {
        Hf = 0.0;
        HfTermFn = KIVA_HF_DEF;
        HfFn = KIVA_CONST_CONV(0.0);
    } break;

    case HcExt::UserCurve: {
        Hf = CalcUserDefinedExtHcModel(state, SurfNum, surfExtConv.hfUserCurveNum);
        if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            HfTermFn = state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].f;
            HnFn = state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].out;
        }
    } break;

    case HcExt::SparrowWindward: {
        Hf = CalcSparrowWindward(state, Roughness, surfExtConv.facePerimeter, surfExtConv.faceArea, SurfWindSpeed, SurfNum);

        if (surface.Class == SurfaceClass::Floor) { // used for exterior grade
            // Assume very large area for grade (relative to perimeter).
            constexpr double area = 9999999.;
            constexpr double perim = 1.;
            HfTermFn = [=](double, double, double, double windSpeed) -> double { return CalcSparrowWindward(Roughness, perim, area, windSpeed); };
        } else {
            if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
                auto const &fnd = state.dataSurfaceGeometry->kivaManager.surfaceMap[SurfNum].get_instance(0).first->foundation;
                const double length = fnd.netPerimeter;
                const double height = fnd.wall.heightAboveGrade;
                const double area = length * height;
                const double perim = 2.0 * (length + height);
                HfTermFn = [=](double, double, double, double windSpeed) -> double {
                    // Average windward and leeward since all walls use same algorithm
                    double windwardHf = CalcSparrowWindward(Roughness, perim, area, windSpeed);
                    double leewardHf = CalcSparrowLeeward(Roughness, perim, area, windSpeed);
                    return (windwardHf + leewardHf) / 2.0;
                };
            }
        }
        HfFn = [](double, double, double HfTerm, double, double) -> double { return HfTerm; };
    } break;

    case HcExt::SparrowLeeward: {
        Hf = CalcSparrowLeeward(state, Roughness, surfExtConv.facePerimeter, surfExtConv.faceArea, SurfWindSpeed, SurfNum);
        if (surface.Class == SurfaceClass::Floor) { // used for exterior grade
            // Assume very large area for grade (relative to perimeter).
            constexpr double area = 9999999.;
            constexpr double perim = 1.;
            HfTermFn = [=](double, double, double, double windSpeed) -> double { return CalcSparrowLeeward(Roughness, perim, area, windSpeed); };
        } else {
            if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
                auto const &fnd = state.dataSurfaceGeometry->kivaManager.surfaceMap[SurfNum].get_instance(0).first->foundation;
                const double length = fnd.netPerimeter;
                const double height = fnd.wall.heightAboveGrade;
                const double area = length * height;
                const double perim = 2.0 * (length + height);
                HfTermFn = [=](double, double, double, double windSpeed) -> double {
                    // Average windward and leeward since all walls use same algorithm
                    double windwardHf = CalcSparrowWindward(Roughness, perim, area, windSpeed);
                    double leewardHf = CalcSparrowLeeward(Roughness, perim, area, windSpeed);
                    return (windwardHf + leewardHf) / 2.0;
                };
            }
        }
        HfFn = [](double, double, double HfTerm, double, double) -> double { return HfTerm; };
    } break;

    case HcExt::MoWiTTWindward: {
        Hf = CalcMoWITTWindward(SurfOutTemp - state.dataSurface->SurfOutDryBulbTemp(SurfNum), SurfWindSpeed);
        if (surface.Class == SurfaceClass::Floor) { // used for exterior grade
            HfTermFn = [=](double, double, double, double windSpeed) -> double { return CalcMoWITTForcedWindward(windSpeed); };
        } else {
            HfTermFn = [=](double, double, double, double windSpeed) -> double {
                // Average windward and leeward since all walls use same algorithm
                double windwardHf = CalcMoWITTForcedWindward(windSpeed);
                double leewardHf = CalcMoWITTForcedLeeward(windSpeed);
                return (windwardHf + leewardHf) / 2.0;
            };
        }
        HfFn = [](double, double, double HfTerm, double, double) -> double { return HfTerm; };
    } break;

    case HcExt::MoWiTTLeeward: {
        Hf = CalcMoWITTLeeward((SurfOutTemp - state.dataSurface->SurfOutDryBulbTemp(SurfNum)), SurfWindSpeed);
        if (surface.Class == SurfaceClass::Floor) { // used for exterior grade
            HfTermFn = [=](double, double, double, double windSpeed) -> double { return CalcMoWITTForcedLeeward(windSpeed); };
        } else {
            HfTermFn = [=](double, double, double, double windSpeed) -> double {
                // Average windward and leeward since all walls use same algorithm
                double windwardHf = CalcMoWITTForcedWindward(windSpeed);
                double leewardHf = CalcMoWITTForcedLeeward(windSpeed);
                return (windwardHf + leewardHf) / 2.0;
            };
        }
        HfFn = [](double, double, double HfTerm, double, double) -> double { return HfTerm; };
    } break;

    case HcExt::DOE2Windward: {
        Hf = CalcDOE2Windward(SurfOutTemp, state.dataSurface->SurfOutDryBulbTemp(SurfNum), surface.CosTilt, SurfWindSpeed, Roughness);
        if (surface.Class == SurfaceClass::Floor) { // used for exterior grade
            HfTermFn = [=](double, double, double, double windSpeed) -> double { return CalcMoWITTForcedWindward(windSpeed); };
        } else {
            HfTermFn = [=](double, double, double, double windSpeed) -> double {
                // Average windward and leeward since all walls use same algorithm
                double windwardHf = CalcMoWITTForcedWindward(windSpeed);
                double leewardHf = CalcMoWITTForcedLeeward(windSpeed);
                return (windwardHf + leewardHf) / 2.0;
            };
        }
        HfFn = [](double, double, double HfTerm, double, double) -> double { return HfTerm; };
    } break;

    case HcExt::DOE2Leeward: {
        Hf = CalcDOE2Leeward(SurfOutTemp, state.dataSurface->SurfOutDryBulbTemp(SurfNum), surface.CosTilt, SurfWindSpeed, Roughness);
        if (surface.Class == SurfaceClass::Floor) { // used for exterior grade
            HfTermFn = [=](double, double, double, double windSpeed) -> double { return CalcMoWITTForcedWindward(windSpeed); };
        } else {
            HfTermFn = [=](double, double, double, double windSpeed) -> double {
                // Average windward and leeward since all walls use same algorithm
                double windwardHf = CalcMoWITTForcedWindward(windSpeed);
                double leewardHf = CalcMoWITTForcedLeeward(windSpeed);
                return (windwardHf + leewardHf) / 2.0;
            };
        }
        HfFn = [=](double Tsurf, double Tamb, double hfTerm, double, double cosTilt) -> double {
            return CalcDOE2Forced(Tsurf, Tamb, cosTilt, hfTerm, Roughness);
        };
    } break;

    case HcExt::NusseltJurges: {
        Hf = CalcNusseltJurges(SurfWindSpeed);
        HfTermFn = [=](double, double, double, double windSpeed) -> double { return CalcNusseltJurges(windSpeed); };
        HfFn = [](double, double, double HfTerm, double, double) -> double { return HfTerm; };
    } break;

    case HcExt::McAdams: {
        Hf = CalcMcAdams(SurfWindSpeed);
        HfTermFn = [=](double, double, double, double windSpeed) -> double { return CalcMcAdams(windSpeed); };
        HfFn = [](double, double, double HfTerm, double, double) -> double { return HfTerm; };
    } break;

    case HcExt::Mitchell: {
        Hf = CalcMitchell(state, SurfWindSpeed, state.dataConvect->CubeRootOfOverallBuildingVolume, SurfNum);
        HfTermFn = [&](double, double, double, double windSpeed) -> double {
            return CalcMitchell(windSpeed, state.dataConvect->CubeRootOfOverallBuildingVolume);
        };
        HfFn = [](double, double, double HfTerm, double, double) -> double { return HfTerm; };
    } break;

    case HcExt::ClearRoof: {
        SurfWindDir = state.dataSurface->SurfOutWindDir(SurfNum);
        Hf = CalcClearRoof(state,
                           SurfNum,
                           SurfOutTemp,
                           state.dataSurface->SurfOutDryBulbTemp(SurfNum),
                           SurfWindSpeed,
                           SurfWindDir,
                           surfExtConv.faceArea,
                           surfExtConv.facePerimeter);
        HfTermFn = [=](double, double, double, double windSpeed) -> double { return windSpeed; };
        if (surface.Class == SurfaceClass::Floor) { // used for exterior grade
            // Assume very large area for grade (relative to perimeter).
            constexpr double area = 9999999.;
            constexpr double perim = 1.;
            HfFn = [=, &state](double Tsurf, double Tamb, double hfTerm, double, double) -> double {
                return CalcClearRoof(state, Tsurf, Tamb, hfTerm, area, perim, Roughness);
            };
        } else {
            if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
                auto const &fnd = state.dataSurfaceGeometry->kivaManager.surfaceMap[SurfNum].get_instance(0).first->foundation;
                const double length = fnd.netPerimeter;
                const double height = fnd.wall.heightAboveGrade;
                const double area = length * height;
                const double perim = 2.0 * (length + height);
                HfFn = [=, &state](double Tsurf, double Tamb, double hfTerm, double, double) -> double {
                    return CalcClearRoof(state, Tsurf, Tamb, hfTerm, area, perim, Roughness);
                };
            }
        }
    } break;

    case HcExt::BlockenWindward: {
        Hf = CalcBlockenWindward(state, state.dataEnvrn->WindSpeed, state.dataEnvrn->WindDir, surface.Azimuth, SurfNum);
        // Not compatible with Kiva (doesn't use weather station windspeed)
        if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            ShowFatalError(state, format("Blocken Windward convection model not applicable for foundation surface ={}", surface.Name));
        }
    } break;

    case HcExt::EmmelVertical: {
        Hf = CalcEmmelVertical(state.dataEnvrn->WindSpeed, state.dataEnvrn->WindDir, surface.Azimuth);
        // Not compatible with Kiva (doesn't use weather station windspeed)
        if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            ShowFatalError(state, format("Emmel Vertical convection model not applicable for foundation surface ={}", surface.Name));
        }
    } break;

    case HcExt::EmmelRoof: {
        Hf = CalcEmmelRoof(state.dataEnvrn->WindSpeed, state.dataEnvrn->WindDir, state.dataConvect->RoofLongAxisOutwardAzimuth);
        // Not compatible with Kiva (doesn't use weather station windspeed)
        if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
            ShowFatalError(state, format("Emmel Roof convection model not applicable for foundation surface ={}", surface.Name));
        }
        break;
    } break;

    default: {
        assert(false);
    }
    } // swtich (ForcedConvModelEqNum)

    Real64 Hc = Hf + Hn;

    if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
        state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].f = HfTermFn;
        state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].out =
            [=](double Tsurf, double Tamb, double HfTerm, double Roughness, double cosTilt) -> double {
            Real64 hcExt = HfFn(Tsurf, Tamb, HfTerm, Roughness, cosTilt) + HnFn(Tsurf, Tamb, HfTerm, Roughness, cosTilt);
            if (hcExt < AdaptiveHcExtLowLimit) {
                hcExt = AdaptiveHcExtLowLimit;
            }
            return hcExt;
        };
        Hc = 0.0; // Not used in Kiva
    }

    if (Hc < AdaptiveHcExtLowLimit) {
        Hc = AdaptiveHcExtLowLimit;
    }
    return Hc;
}

void DynamicExtConvSurfaceClassification(EnergyPlusData &state, int const SurfNum) // surface number
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   August 2010

    // METHODOLOGY EMPLOYED:
    // Decide surface classification based on wind and buoyancy, class, orientation

    auto const &surface = state.dataSurface->Surface(SurfNum);
    auto &surfExtConv = state.dataSurface->surfExtConv(SurfNum);

    Real64 surfWindDir = state.dataSurface->SurfOutWindDir(SurfNum);

    if (surface.Class == SurfaceClass::Roof ||
        (surface.Class == SurfaceClass::Floor && surface.ExtBoundCond == DataSurfaces::KivaFoundation) // Applies to exterior grade
    ) {
        Real64 DeltaTemp =
            (surface.ExtBoundCond == DataSurfaces::KivaFoundation)
                ? (state.dataSurfaceGeometry->kivaManager.surfaceMap[SurfNum].results.Tconv - state.dataSurface->SurfOutDryBulbTemp(SurfNum))
                : (state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum) - state.dataSurface->SurfOutDryBulbTemp(SurfNum));

        surfExtConv.convClass = (DeltaTemp < 0.0) ? ExtConvClass::RoofStable : ExtConvClass::RoofUnstable;

    } else {
        surfExtConv.convClass =
            Windward(surface.CosTilt, surface.Azimuth, surfWindDir) ? ExtConvClass::WindwardVertWall : ExtConvClass::LeewardVertWall;
    }
}

void MapExtConvClassToHcModels(EnergyPlusData &state, int const SurfNum) // surface number
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // Use these arrays to convert general surface classifications to
    // specific classifications for both wind-driven and natural
    // convection
    static constexpr std::array<ExtConvClass2, (int)ExtConvClass::Num> WindConvectionExtConvClass2s = {
        ExtConvClass2::WindConvection_WallWindward, // WindwardWall
        ExtConvClass2::WindConvection_WallLeeward,  // LeewardWall
        ExtConvClass2::WindConvection_HorizRoof,    // RoofStable
        ExtConvClass2::WindConvection_HorizRoof     // RoofUnstable
    };

    static constexpr std::array<ExtConvClass2, (int)ExtConvClass::Num> NaturalConvectionExtConvClass2s = {
        ExtConvClass2::NaturalConvection_VertWall,     // WindwardWall
        ExtConvClass2::NaturalConvection_VertWall,     // LeewardWall
        ExtConvClass2::NaturalConvection_StableHoriz,  // RoofStable
        ExtConvClass2::NaturalConvection_UnstableHoriz // RoofUnstable
    };

    auto &surfExtConv = state.dataSurface->surfExtConv(SurfNum);

    ExtConvClass outConvClass = surfExtConv.convClass;
    surfExtConv.convClassRpt = ExtConvClassReportVals[(int)outConvClass];

    ExtConvClass2 outConvClass2Wind = WindConvectionExtConvClass2s[(int)outConvClass];
    ExtConvClass2 outConvClass2Natural = NaturalConvectionExtConvClass2s[(int)outConvClass];

    surfExtConv.hfModelEq = state.dataConvect->extAdaptiveConvAlgo.extConvClass2EqNums[(int)outConvClass2Wind];
    surfExtConv.hfModelEqRpt = HcExtReportVals[(int)surfExtConv.hfModelEq];

    if (surfExtConv.hfModelEq == HcExt::UserCurve) {
        surfExtConv.hfUserCurveNum = state.dataConvect->extAdaptiveConvAlgo.extConvClass2UserCurveNums[(int)outConvClass2Wind];
    }

    surfExtConv.hnModelEq = state.dataConvect->extAdaptiveConvAlgo.extConvClass2EqNums[(int)outConvClass2Natural];
    surfExtConv.hnModelEqRpt = HcExtReportVals[(int)surfExtConv.hnModelEq];
    if (surfExtConv.hnModelEq == HcExt::UserCurve) {
        surfExtConv.hnUserCurveNum = state.dataConvect->extAdaptiveConvAlgo.extConvClass2UserCurveNums[(int)outConvClass2Natural];
    }
}

void DynamicIntConvSurfaceClassification(EnergyPlusData &state, int const SurfNum) // surface number
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR        Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS SUBROUTINE:
    // collects dynamic updates needed for adaptive convection algorithm

    // METHODOLOGY EMPLOYED:
    // Decide flow regime to set IntConvClassification done by zone using the following rules

    // Using zone flow regime, and surface's characteristics assign IntConvHcModelEq

    // SUBROUTINE PARAMETER DEFINITIONS:
    Real64 constexpr g(9.81);                     // gravity constant (m/s**2)
    Real64 constexpr v(15.89e-6);                 // kinematic viscosity (m**2/s) for air at 300 K
    Real64 constexpr ActiveDelTempThreshold(1.5); // deg C, temperature difference for surfaces to be considered "active"

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int PriorityEquipOn(0);
    constexpr int MaxZoneEquipmentOn{11};
    constexpr int MaxZoneEquipmentIdx{MaxZoneEquipmentOn - 1};
    std::array<int, MaxZoneEquipmentOn> HeatingPriorityStack{};
    std::array<int, MaxZoneEquipmentOn> CoolingPriorityStack{};
    std::array<InConvFlowRegime, MaxZoneEquipmentOn> FlowRegimeStack{};
    FlowRegimeStack.fill(InConvFlowRegime::Invalid);
    int EquipOnCount(0);
    InConvFlowRegime FinalFlowRegime(InConvFlowRegime::Invalid);
    Real64 Tmin(std::numeric_limits<float>::max()); // temporary min surf temp
    Real64 Tmax(std::numeric_limits<float>::min()); // temporary max surf temp
    Real64 GrH(0.0);                                // Grashof number for zone height H
    Real64 Re(0.0);                                 // Reynolds number for zone air system flow
    Real64 Ri(0.0);                                 // Richardson Number, Gr/Re**2 for determining mixed regime
    Real64 AirDensity(0.0);                         // temporary zone air density
    Real64 DeltaTemp(0.0);                          // temporary temperature difference (Tsurf - Tair)

    auto &surface = state.dataSurface->Surface(SurfNum);
    int zoneNum = surface.Zone;
    int spaceNum = surface.spaceNum;
    auto &zone = state.dataHeatBal->Zone(zoneNum);

    EquipOnCount = 0;

    // HVAC connections
    if (!zone.IsControlled) { // no HVAC control
        FlowRegimeStack[0] = InConvFlowRegime::A3;
    } else { // is controlled, lets see by how and if that means is currently active

        auto &zoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(surface.Zone);
        auto const &zoneNode = state.dataLoopNodes->Node(zone.SystemZoneNodeNumber);

        if (!(zoneEquipConfig.EquipListIndex > 0) || state.dataGlobal->SysSizingCalc || state.dataGlobal->ZoneSizingCalc ||
            !state.dataZoneEquip->ZoneEquipSimulatedOnce) {
            FlowRegimeStack[0] = InConvFlowRegime::A3;
        } else {

            auto &zoneEquipList = state.dataZoneEquip->ZoneEquipList(zoneEquipConfig.EquipListIndex);
            for (int EquipNum = 1; EquipNum <= zoneEquipList.NumOfEquipTypes; ++EquipNum) {

                switch (zoneEquipList.EquipType(EquipNum)) {
                case DataZoneEquipment::ZoneEquipType::AirDistributionUnit:
                case DataZoneEquipment::ZoneEquipType::PurchasedAir: {
                    if (!allocated(zoneEquipList.EquipData(EquipNum).OutletNodeNums)) {
                        continue;
                    }

                    // get inlet node, not zone node if possible
                    int zoneInletNodeNum = zoneEquipList.EquipData(EquipNum).OutletNodeNums(1);
                    if ((zoneInletNodeNum > 0 && state.dataLoopNodes->Node(zoneInletNodeNum).MassFlowRate > 0.0) ||
                        (zoneInletNodeNum <= 0 && zoneNode.MassFlowRate > 0.0)) {
                        EquipOnCount = min(EquipOnCount + 1, MaxZoneEquipmentIdx);
                        FlowRegimeStack[EquipOnCount] = InConvFlowRegime::C;
                        HeatingPriorityStack[EquipOnCount] = zoneEquipList.HeatingPriority(EquipNum);
                        CoolingPriorityStack[EquipOnCount] = zoneEquipList.CoolingPriority(EquipNum);
                    }
                } break;
                case DataZoneEquipment::ZoneEquipType::WindowAirConditioner:
                case DataZoneEquipment::ZoneEquipType::PackagedTerminalHeatPump:
                case DataZoneEquipment::ZoneEquipType::PackagedTerminalAirConditioner:
                case DataZoneEquipment::ZoneEquipType::DehumidifierDX:
                case DataZoneEquipment::ZoneEquipType::PackagedTerminalHeatPumpWaterToAir:
                case DataZoneEquipment::ZoneEquipType::FourPipeFanCoil:
                case DataZoneEquipment::ZoneEquipType::UnitVentilator:
                case DataZoneEquipment::ZoneEquipType::UnitHeater:
                case DataZoneEquipment::ZoneEquipType::OutdoorAirUnit: {
                    if (!allocated(zoneEquipList.EquipData(EquipNum).OutletNodeNums)) {
                        continue;
                    }

                    int zoneInletNodeNum = zoneEquipList.EquipData(EquipNum).OutletNodeNums(1);
                    if ((zoneInletNodeNum > 0 && state.dataLoopNodes->Node(zoneInletNodeNum).MassFlowRate > 0.0) ||
                        (zoneInletNodeNum <= 0 && zoneNode.MassFlowRate > 0.0)) {

                        EquipOnCount = min(EquipOnCount + 1, MaxZoneEquipmentIdx);
                        FlowRegimeStack[EquipOnCount] = InConvFlowRegime::D;
                        HeatingPriorityStack[EquipOnCount] = zoneEquipList.HeatingPriority(EquipNum);
                        CoolingPriorityStack[EquipOnCount] = zoneEquipList.CoolingPriority(EquipNum);
                    }
                } break;
                case DataZoneEquipment::ZoneEquipType::CoolingPanel:
                case DataZoneEquipment::ZoneEquipType::BaseboardSteam:
                case DataZoneEquipment::ZoneEquipType::BaseboardConvectiveWater:
                case DataZoneEquipment::ZoneEquipType::BaseboardConvectiveElectric:
                case DataZoneEquipment::ZoneEquipType::BaseboardWater: {
                    if (zoneEquipList.EquipData(EquipNum).ON) {
                        EquipOnCount = min(EquipOnCount + 1, MaxZoneEquipmentIdx);
                        FlowRegimeStack[EquipOnCount] = InConvFlowRegime::B;
                        HeatingPriorityStack[EquipOnCount] = zoneEquipList.HeatingPriority(EquipNum);
                        CoolingPriorityStack[EquipOnCount] = zoneEquipList.CoolingPriority(EquipNum);
                    }
                } break;
                    // Is this the same case as above?
                case DataZoneEquipment::ZoneEquipType::BaseboardElectric:
                case DataZoneEquipment::ZoneEquipType::HighTemperatureRadiant: {
                    if (zoneEquipList.EquipData(EquipNum).ON) {
                        EquipOnCount = min(EquipOnCount + 1, MaxZoneEquipmentIdx);
                        FlowRegimeStack[EquipOnCount] = InConvFlowRegime::B;
                        HeatingPriorityStack[EquipOnCount] = zoneEquipList.HeatingPriority(EquipNum);
                        CoolingPriorityStack[EquipOnCount] = zoneEquipList.CoolingPriority(EquipNum);
                    }
                } break;
                case DataZoneEquipment::ZoneEquipType::VentilatedSlab:
                case DataZoneEquipment::ZoneEquipType::LowTemperatureRadiantConstFlow:
                case DataZoneEquipment::ZoneEquipType::LowTemperatureRadiantVarFlow:
                case DataZoneEquipment::ZoneEquipType::LowTemperatureRadiantElectric: {
                    if (zoneEquipConfig.InFloorActiveElement) {
                        for (int spaceNumLoop : zone.spaceIndexes) {
                            auto const &thisSpace = state.dataHeatBal->space(spaceNumLoop);

                            for (int SurfLoop = thisSpace.HTSurfaceFirst; SurfLoop <= thisSpace.HTSurfaceLast; ++SurfLoop) {

                                if (!state.dataSurface->surfIntConv(SurfLoop).hasActiveInIt) {
                                    continue;
                                }
                                auto const &surfaceLoop = state.dataSurface->Surface(SurfLoop);
                                if (surfaceLoop.Class != SurfaceClass::Floor) {
                                    continue;
                                }

                                Real64 DeltaTempLoop = state.dataHeatBalSurf->SurfInsideTempHist(1)(SurfLoop) -
                                                       state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNumLoop).MAT;
                                if (DeltaTempLoop > ActiveDelTempThreshold) { // assume heating with floor
                                    // system ON is not enough because floor surfaces can continue to heat because of thermal capacity
                                    EquipOnCount = min(EquipOnCount + 1, MaxZoneEquipmentIdx);
                                    FlowRegimeStack[EquipOnCount] = InConvFlowRegime::A1;
                                    HeatingPriorityStack[EquipOnCount] = zoneEquipList.HeatingPriority(EquipNum);
                                    CoolingPriorityStack[EquipOnCount] = zoneEquipList.CoolingPriority(EquipNum);
                                    break;
                                } // if (DeltaTemp)
                            } // for (SurfLoop)
                        } // for (spaceNumLoop)
                    } // if (InFloorActiveElement)

                    if (zoneEquipConfig.InCeilingActiveElement) {
                        for (int spaceNumLoop : zone.spaceIndexes) {
                            auto const &thisSpace = state.dataHeatBal->space(spaceNumLoop);

                            for (int SurfLoop = thisSpace.HTSurfaceFirst; SurfLoop <= thisSpace.HTSurfaceLast; ++SurfLoop) {
                                if (!state.dataSurface->surfIntConv(SurfLoop).hasActiveInIt) {
                                    continue;
                                }
                                auto const &surfaceLoop = state.dataSurface->Surface(SurfLoop);
                                if (surfaceLoop.Class != SurfaceClass::Roof) {
                                    continue;
                                }

                                Real64 DeltaTempLoop = state.dataHeatBalSurf->SurfInsideTempHist(1)(SurfLoop) -
                                                       state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNumLoop).MAT;
                                if (DeltaTempLoop < ActiveDelTempThreshold) { // assume cooling with ceiling
                                    // system ON is not enough because  surfaces can continue to cool because of thermal capacity
                                    EquipOnCount = min(EquipOnCount + 1, MaxZoneEquipmentIdx);
                                    FlowRegimeStack[EquipOnCount] = InConvFlowRegime::A1;
                                    HeatingPriorityStack[EquipOnCount] = zoneEquipList.HeatingPriority(EquipNum);
                                    CoolingPriorityStack[EquipOnCount] = zoneEquipList.CoolingPriority(EquipNum);
                                    break;
                                } // if (DeltaTempLoop)
                            } // for (SurfLoop)
                        } // for (spaceNumLoop)
                    } // if (InCeilingActiveElement)

                    if (zoneEquipConfig.InWallActiveElement) {
                        for (int spaceNumLoop : zone.spaceIndexes) {
                            auto const &thisSpace = state.dataHeatBal->space(spaceNumLoop);

                            for (int SurfLoop = thisSpace.HTSurfaceFirst; SurfLoop <= thisSpace.HTSurfaceLast; ++SurfLoop) {
                                if (!state.dataSurface->surfIntConv(SurfLoop).hasActiveInIt) {
                                    continue;
                                }
                                auto const &surface_test = state.dataSurface->Surface(SurfLoop);
                                if (surface_test.Class != SurfaceClass::Wall && surface_test.Class != SurfaceClass::Door) {
                                    continue;
                                }

                                DeltaTemp = state.dataHeatBalSurf->SurfInsideTempHist(1)(SurfLoop) -
                                            state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNumLoop).MAT;
                                if (DeltaTemp > ActiveDelTempThreshold) { // assume heating with wall panel
                                    // system ON is not enough because  surfaces can continue to heat because of thermal capacity
                                    EquipOnCount = min(EquipOnCount + 1, MaxZoneEquipmentIdx);
                                    FlowRegimeStack[EquipOnCount] = InConvFlowRegime::A2;
                                    HeatingPriorityStack[EquipOnCount] = zoneEquipList.HeatingPriority(EquipNum);
                                    CoolingPriorityStack[EquipOnCount] = zoneEquipList.CoolingPriority(EquipNum);
                                } else { // not heating, no special models wall cooling so use simple buoyancy
                                    EquipOnCount = min(EquipOnCount + 1, MaxZoneEquipmentIdx);
                                    FlowRegimeStack[EquipOnCount] = InConvFlowRegime::A3;
                                    HeatingPriorityStack[EquipOnCount] = zoneEquipList.HeatingPriority(EquipNum);
                                    CoolingPriorityStack[EquipOnCount] = zoneEquipList.CoolingPriority(EquipNum);
                                } // else (DeltaTemp)
                            } // for (SurfLoop)
                        } // for (spaceNumLoop)
                    } // if (InWallActiveElement)
                } break;
                default:; // nothing
                }

            } // for (EquipNum)
        }
    }

    // now select which equipment type is dominant compared to all those that are ON
    if (EquipOnCount > 0) {
        if (state.dataZoneEnergyDemand->ZoneSysEnergyDemand(zoneNum).predictedRate >= 0.0) { // heating load
            PriorityEquipOn = 1;
            for (int EquipOnLoop = 1; EquipOnLoop <= EquipOnCount; ++EquipOnLoop) {
                // assume highest priority/first sim order is dominant for flow regime
                if (HeatingPriorityStack[EquipOnLoop] < HeatingPriorityStack[PriorityEquipOn]) {
                    PriorityEquipOn = EquipOnLoop;
                }
            }
        } else if (state.dataZoneEnergyDemand->ZoneSysEnergyDemand(zoneNum).predictedRate < 0.0) { // cooling load
            PriorityEquipOn = 1;
            for (int EquipOnLoop = 1; EquipOnLoop <= EquipOnCount; ++EquipOnLoop) {
                // assume highest priority/first sim order is dominant for flow regime
                if (CoolingPriorityStack[EquipOnLoop] < CoolingPriorityStack[PriorityEquipOn]) {
                    PriorityEquipOn = EquipOnLoop;
                }
            }
        }
        FinalFlowRegime = FlowRegimeStack[PriorityEquipOn];
    } else {
        // no equipment on, so simple buoyancy flow regime
        FinalFlowRegime = InConvFlowRegime::A3;
    }

    // now if flow regimes C or D, then check for Mixed regime or very low flow rates
    if ((FinalFlowRegime == InConvFlowRegime::C) || (FinalFlowRegime == InConvFlowRegime::D)) {

        auto const &zoneNode = state.dataLoopNodes->Node(zone.SystemZoneNodeNumber);
        // Calculate Grashof, Reynolds, and Richardson numbers for the zone
        // Grashof for zone air based on largest delta T between surfaces and zone height
        for (int spaceNumLoop : zone.spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNumLoop);
            for (int surfNum = thisSpace.HTSurfaceFirst; surfNum <= thisSpace.HTSurfaceLast; ++surfNum) {
                Real64 SurfTemp = state.dataHeatBalSurf->SurfInsideTempHist(1)(surfNum);
                if (SurfTemp < Tmin) {
                    Tmin = SurfTemp;
                } else if (SurfTemp > Tmax) {
                    Tmax = SurfTemp;
                }
            }
        }
        GrH = (g * (Tmax - Tmin) * pow_3(zone.CeilingHeight)) /
              ((state.dataZoneTempPredictorCorrector->zoneHeatBalance(zoneNum).MAT + Constant::Kelvin) * pow_2(v));

        // Reynolds number = Vdot supply / v * cube root of zone volume (Goldstein and Noveselac 2010)
        if (zoneNode.MassFlowRate > 0.0) {
            AirDensity = Psychrometrics::PsyRhoAirFnPbTdbW(state,
                                                           state.dataEnvrn->OutBaroPress,
                                                           zoneNode.Temp,
                                                           Psychrometrics::PsyWFnTdpPb(state, zoneNode.Temp, state.dataEnvrn->OutBaroPress));
            Re = zoneNode.MassFlowRate / (v * AirDensity * std::pow(zone.Volume, 1.0 / 3.0));
        } else {
            Re = 0.0;
        }

        if (Re > 0.0) {
            Ri = GrH / pow_2(Re); // Richardson Number
            if (Ri > 10.0) {      // natural convection expected
                FinalFlowRegime = InConvFlowRegime::A3;
            } else if (Ri < 0.1) { // forced
                // no change, already a forced regime
            } else { // mixed
                FinalFlowRegime = InConvFlowRegime::E;
            }
        } else { // natural convection expected
            FinalFlowRegime = InConvFlowRegime::A3;
        }
    }

    static constexpr std::array<std::array<IntConvClass, (int)ConvSurfDeltaT::Num>,
                                (int)SurfOrientation::Num>
        A1{{
            {IntConvClass::A1_FloorHeatCeilCool_StableHoriz,     // HorizontalDown, Positive
             IntConvClass::A1_FloorHeatCeilCool_StableHoriz,     // HorizontalDown, Zero
             IntConvClass::A1_FloorHeatCeilCool_UnstableHoriz},  // HorizontalDown, Negative
            {IntConvClass::A1_FloorHeatCeilCool_StableTilted,    // TiltedDownward, Positive
             IntConvClass::A1_FloorHeatCeilCool_StableTilted,    // TiltedDownward, Zero
             IntConvClass::A1_FloorHeatCeilCool_UnstableTilted}, // TiltedDownward, Negative
            {IntConvClass::A1_FloorHeatCeilCool_VertWalls,       // Vertical, Positive
             IntConvClass::A1_FloorHeatCeilCool_VertWalls,       // Vertical, Zero
             IntConvClass::A1_FloorHeatCeilCool_VertWalls},      // Vertical, Negative
            {IntConvClass::A1_FloorHeatCeilCool_UnstableTilted,  // TiltedUpward, Positive
             IntConvClass::A1_FloorHeatCeilCool_StableTilted,    // TiltedUpward, Zero
             IntConvClass::A1_FloorHeatCeilCool_StableTilted},   // TiltedUpward, Negative
            {IntConvClass::A1_FloorHeatCeilCool_UnstableHoriz,   // HorizontalUp, Positive
             IntConvClass::A1_FloorHeatCeilCool_StableHoriz,     // HorizontalUp, Zero
             IntConvClass::A1_FloorHeatCeilCool_StableHoriz}     // HorizontalUp, Negative
        }};

    static constexpr std::array<std::array<IntConvClass, (int)ConvSurfDeltaT::Num>,
                                (int)SurfOrientation::Num>
        A2{{
            {IntConvClass::A2_WallPanelHeat_StableHoriz,         // HorizontalDown, Positive
             IntConvClass::A2_WallPanelHeat_StableHoriz,         // HorizontalDown, Zero
             IntConvClass::A2_WallPanelHeat_UnstableHoriz},      // HorizontalDown, Negative
            {IntConvClass::A2_WallPanelHeat_StableTilted,        // TiltedDownward, Positive
             IntConvClass::A2_WallPanelHeat_StableTilted,        // TiltedDownward, Zero
             IntConvClass::A2_WallPanelHeat_UnstableTilted},     // TiltedDownward, Negative
            {IntConvClass::A2_WallPanelHeat_VertWallsNonHeated,  // Vertical, Positive
             IntConvClass::A2_WallPanelHeat_VertWallsNonHeated,  // Vertical, Zero
             IntConvClass::A2_WallPanelHeat_VertWallsNonHeated}, // Vertical, Negative
            {IntConvClass::A2_WallPanelHeat_UnstableTilted,      // TiltedUpward, Positive
             IntConvClass::A2_WallPanelHeat_StableTilted,        // TiltedUpward, Zero
             IntConvClass::A2_WallPanelHeat_StableTilted},       // TiltedUpward, Negative
            {IntConvClass::A2_WallPanelHeat_UnstableHoriz,       // HorizontalUp, Positive
             IntConvClass::A2_WallPanelHeat_StableHoriz,         // HorizontalUp, Zero
             IntConvClass::A2_WallPanelHeat_StableHoriz}         // HorizontalUp, Negative
        }};

    static constexpr std::array<std::array<IntConvClass, (int)ConvSurfDeltaT::Num>,
                                (int)SurfOrientation::Num>
        A3{{
            {IntConvClass::A3_SimpleBuoy_StableHoriz,     // HorizontalDown, Positive
             IntConvClass::A3_SimpleBuoy_StableHoriz,     // HorizontalDown, Zero
             IntConvClass::A3_SimpleBuoy_UnstableHoriz},  // HorizontalDown, Negative
            {IntConvClass::A3_SimpleBuoy_StableTilted,    // TiltedDownward, Positive
             IntConvClass::A3_SimpleBuoy_StableTilted,    // TiltedDownward, Zero
             IntConvClass::A3_SimpleBuoy_UnstableTilted}, // TiltedDownward, Negative
            {IntConvClass::A3_SimpleBuoy_VertWalls,       // Vertical, Positive
             IntConvClass::A3_SimpleBuoy_VertWalls,       // Vertical, Zero
             IntConvClass::A3_SimpleBuoy_VertWalls},      // Vertical, Negative
            {IntConvClass::A3_SimpleBuoy_UnstableTilted,  // TiltedUpward, Positive
             IntConvClass::A3_SimpleBuoy_StableTilted,    // TiltedUpward, Zero
             IntConvClass::A3_SimpleBuoy_StableTilted},   // TiltedUpward, Negative
            {IntConvClass::A3_SimpleBuoy_UnstableHoriz,   // HorizontalUp, Positive
             IntConvClass::A3_SimpleBuoy_StableHoriz,     // HorizontalUp, Zero
             IntConvClass::A3_SimpleBuoy_StableHoriz}     // HorizontalUp, Negative
        }};

    static constexpr std::array<std::array<IntConvClass, (int)ConvSurfDeltaT::Num>,
                                (int)SurfOrientation::Num>
        B{{
            {IntConvClass::B_ConvectiveHeat_StableHoriz,     // HorizontalDown, Positive
             IntConvClass::B_ConvectiveHeat_StableHoriz,     // HorizontalDown, Zero
             IntConvClass::B_ConvectiveHeat_UnstableHoriz},  // HorizontalDown, Negative
            {IntConvClass::B_ConvectiveHeat_StableTilted,    // TiltedDownward, Positive
             IntConvClass::B_ConvectiveHeat_StableTilted,    // TiltedDownward, Zero
             IntConvClass::B_ConvectiveHeat_UnstableTilted}, // TiltedDownward, Negative
            {IntConvClass::B_ConvectiveHeat_VertWalls,       // Vertical, Positive
             IntConvClass::B_ConvectiveHeat_VertWalls,       // Vertical, Zero
             IntConvClass::B_ConvectiveHeat_VertWalls},      // Vertical, Negative
            {IntConvClass::B_ConvectiveHeat_UnstableTilted,  // TiltedUpward, Positive
             IntConvClass::B_ConvectiveHeat_StableTilted,    // TiltedUpward, Zero
             IntConvClass::B_ConvectiveHeat_StableTilted},   // TiltedUpward, Negative
            {IntConvClass::B_ConvectiveHeat_UnstableHoriz,   // HorizontalUp, Positive
             IntConvClass::B_ConvectiveHeat_StableHoriz,     // HorizontalUp, Zero
             IntConvClass::B_ConvectiveHeat_StableHoriz}     // HorizontalUp, Negative
        }};

    static constexpr std::array<std::array<IntConvClass, (int)ConvSurfDeltaT::Num>,
                                (int)SurfOrientation::Num>
        D{{
            {IntConvClass::D_ZoneFanCirc_StableHoriz,     // HorizontalDown, Positive
             IntConvClass::D_ZoneFanCirc_StableHoriz,     // HorizontalDown, Zero
             IntConvClass::D_ZoneFanCirc_UnstableHoriz},  // HorizontalDown, Negative
            {IntConvClass::D_ZoneFanCirc_StableTilted,    // TiltedDownward, Positive
             IntConvClass::D_ZoneFanCirc_StableTilted,    // TiltedDownward, Zero
             IntConvClass::D_ZoneFanCirc_UnstableTilted}, // TiltedDownward, Negative
            {IntConvClass::D_ZoneFanCirc_Walls,           // Vertical, Positive
             IntConvClass::D_ZoneFanCirc_Walls,           // Vertical, Zero
             IntConvClass::D_ZoneFanCirc_Walls},          // Vertical, Negative
            {IntConvClass::D_ZoneFanCirc_UnstableTilted,  // TiltedUpward, Positive
             IntConvClass::D_ZoneFanCirc_StableTilted,    // TiltedUpward, Zero
             IntConvClass::D_ZoneFanCirc_StableTilted},   // TiltedUpward, Negative
            {IntConvClass::D_ZoneFanCirc_UnstableHoriz,   // HorizontalUp, Positive
             IntConvClass::D_ZoneFanCirc_StableHoriz,     // HorizontalUp, Zero
             IntConvClass::D_ZoneFanCirc_StableHoriz}     // HorizontalUp, Negative
        }};

    auto DeltaTempLambda = [](Real64 surfTemp, Real64 airTemp) {
        Real64 deltaT = surfTemp - airTemp;
        if (deltaT > 0.0) {
            return (int)ConvSurfDeltaT::Positive;
        } else if (deltaT < 0.0) {
            return (int)ConvSurfDeltaT::Negative;
        } else {
            return (int)ConvSurfDeltaT::Zero;
        }
    };

    // now finish out specific model eq for this surface

    int iDeltaTemp =
        DeltaTempLambda(state.dataHeatBalSurf->SurfInsideTempHist(1)(SurfNum), state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum).MAT);
    int iConvOrient = int(surface.convOrientation);

    auto &surfIntConv = state.dataSurface->surfIntConv(SurfNum);
    switch (FinalFlowRegime) {
    case InConvFlowRegime::A1: {

        switch (surface.Class) {
        case SurfaceClass::Wall:
        case SurfaceClass::Door:
        case SurfaceClass::IntMass: {
            surfIntConv.convClass = A1[iConvOrient][iDeltaTemp];
        } break;
        case SurfaceClass::Roof: {
            surfIntConv.convClass = (surfIntConv.hasActiveInIt) ? IntConvClass::A1_FloorHeatCeilCool_ChilledCeil : A1[iConvOrient][iDeltaTemp];
        } break;
        case SurfaceClass::Floor: {
            surfIntConv.convClass = (surfIntConv.hasActiveInIt) ? IntConvClass::A1_FloorHeatCeilCool_HeatedFloor : A1[iConvOrient][iDeltaTemp];
        } break;
        case SurfaceClass::Window:
        case SurfaceClass::GlassDoor:
        case SurfaceClass::TDD_Diffuser: {
            surfIntConv.convClass = IntConvClass::A1_FloorHeatCeilCool_Windows;
        } break;
        default:
            assert(false);
        }

        if (surfIntConv.convClass == IntConvClass::Invalid) {
            ShowSevereError(state, format("DynamicIntConvSurfaceClassification: failed to resolve Hc model for A1 surface named{}", surface.Name));
        }

    } break; // A1

    case InConvFlowRegime::A2: {

        switch (surface.Class) {
        case SurfaceClass::Roof:
        case SurfaceClass::Floor:
        case SurfaceClass::IntMass: {
            surfIntConv.convClass = A2[iConvOrient][iDeltaTemp];
        } break;
        case SurfaceClass::Wall:
        case SurfaceClass::Door: {
            surfIntConv.convClass = (surfIntConv.hasActiveInIt) ? IntConvClass::A2_WallPanelHeat_HeatedVerticalWall : A2[iConvOrient][iDeltaTemp];
        } break;
        case SurfaceClass::Window:
        case SurfaceClass::GlassDoor:
        case SurfaceClass::TDD_Diffuser: {
            surfIntConv.convClass = IntConvClass::A2_WallPanelHeat_Windows;
        } break;
        default:
            assert(false);
        }

        if (surfIntConv.convClass == IntConvClass::Invalid) {
            ShowSevereError(state, format("DynamicIntConvSurfaceClassification: failed to resolve Hc model for A2 surface named{}", surface.Name));
        }

    } break; // A2

    case InConvFlowRegime::A3: {

        switch (surface.Class) {
        case SurfaceClass::Wall:
        case SurfaceClass::Door:
        case SurfaceClass::Roof:
        case SurfaceClass::Floor: {
            surfIntConv.convClass = A3[iConvOrient][iDeltaTemp];
        } break;
        case SurfaceClass::IntMass: {
            // assume horizontal upwards
            surfIntConv.convClass = A3[int(SurfOrientation::HorizontalUp)][iDeltaTemp];
        } break;
        case SurfaceClass::Window:
        case SurfaceClass::GlassDoor:
        case SurfaceClass::TDD_Diffuser: {
            surfIntConv.convClass = IntConvClass::A3_SimpleBuoy_Windows;
        } break;
        default:
            assert(false);
        }

        if (surfIntConv.convClass == IntConvClass::Invalid) {
            ShowSevereError(state, format("DynamicIntConvSurfaceClassification: failed to resolve Hc model for A3 surface named{}", surface.Name));
        }

    } break; // A3

    case InConvFlowRegime::B: {

        switch (surface.Class) {
        case SurfaceClass::Wall:
        case SurfaceClass::Door: {
            surfIntConv.convClass = (surfIntConv.getsRadiantHeat) ? IntConvClass::B_ConvectiveHeat_VertWallsNearHeat : B[iConvOrient][iDeltaTemp];
        } break;
        case SurfaceClass::Roof:
        case SurfaceClass::Floor: {
            surfIntConv.convClass = B[iConvOrient][iDeltaTemp];
        } break;
        case SurfaceClass::Window:
        case SurfaceClass::GlassDoor:
        case SurfaceClass::TDD_Diffuser: {
            surfIntConv.convClass = IntConvClass::B_ConvectiveHeat_Windows;
        } break;
        case SurfaceClass::IntMass: {
            // assume horizontal upwards
            surfIntConv.convClass = B[int(SurfOrientation::HorizontalUp)][iDeltaTemp];
        } break;
        default:
            assert(false);
        }

        if (surfIntConv.convClass == IntConvClass::Invalid) {
            ShowSevereError(state, format("DynamicIntConvSurfaceClassification: failed to resolve Hc model for B surface named{}", surface.Name));
        }
    } break; // B

    case InConvFlowRegime::C: {

        switch (surface.Class) {
        case SurfaceClass::Wall:
        case SurfaceClass::Door: {
            surfIntConv.convClass = IntConvClass::C_CentralAirHeat_Walls;
        } break;
        case SurfaceClass::Roof: {
            surfIntConv.convClass = IntConvClass::C_CentralAirHeat_Ceiling;
        } break;
        case SurfaceClass::Floor: {
            surfIntConv.convClass = IntConvClass::C_CentralAirHeat_Floor;
        } break;
        case SurfaceClass::Window:
        case SurfaceClass::GlassDoor:
        case SurfaceClass::TDD_Diffuser: {
            surfIntConv.convClass = IntConvClass::C_CentralAirHeat_Windows;
        } break;
        case SurfaceClass::IntMass: {
            surfIntConv.convClass = IntConvClass::C_CentralAirHeat_Floor;
        } break;
        default:
            assert(false);
        }

        if (surfIntConv.convClass == IntConvClass::Invalid) {
            ShowSevereError(state, format("DynamicIntConvSurfaceClassification: failed to resolve Hc model for C surface named{}", surface.Name));
        }

    } break; // C

    case InConvFlowRegime::D: {

        switch (surface.Class) {
        case SurfaceClass::Wall:
        case SurfaceClass::Door:
        case SurfaceClass::Roof:
        case SurfaceClass::Floor: {
            surfIntConv.convClass = D[iConvOrient][iDeltaTemp];
        } break;
        case SurfaceClass::Window:
        case SurfaceClass::GlassDoor:
        case SurfaceClass::TDD_Diffuser: {
            surfIntConv.convClass = IntConvClass::D_ZoneFanCirc_Windows;
        } break;
        case SurfaceClass::IntMass: {
            // assume horizontal upwards.
            surfIntConv.convClass = D[int(SurfOrientation::HorizontalUp)][iDeltaTemp];
        } break;
        default:
            assert(false);
        }

        if (surfIntConv.convClass == IntConvClass::Invalid) {
            ShowSevereError(state, format("DynamicIntConvSurfaceClassification: failed to resolve Hc model for D surface named{}", surface.Name));
        }

    } break; // D

    case InConvFlowRegime::E: {
        Real64 deltaTemp =
            state.dataHeatBalSurf->SurfInsideTempHist(1)(SurfNum) - state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum).MAT;

        switch (surface.Class) {
        case SurfaceClass::Wall:
        case SurfaceClass::Door: {
            switch (FlowRegimeStack[PriorityEquipOn]) {
            case InConvFlowRegime::C: {
                // assume forced flow is down along wall (ceiling diffuser)
                surfIntConv.convClass = (deltaTemp > 0.0) ? IntConvClass::E_MixedBuoy_OpposFlowWalls
                                                          :                            // surface is hotter so plume upwards and forces oppose
                                            IntConvClass::E_MixedBuoy_AssistFlowWalls; // surface is cooler so plume down and forces assist
            } break;
            case InConvFlowRegime::D: {
                // assume forced flow is upward along wall (perimeter zone HVAC with fan)
                surfIntConv.convClass = (deltaTemp > 0.0) ? IntConvClass::E_MixedBuoy_AssistFlowWalls
                                                          :                           // surface is hotter so plume up and forces assist
                                            IntConvClass::E_MixedBuoy_OpposFlowWalls; // surface is cooler so plume downward and forces oppose
            } break;
            default:
                assert(false);
            }

        } break;

        case SurfaceClass::Roof: {
            surfIntConv.convClass = (deltaTemp > 0.0) ? // surface is hotter so stable
                                        IntConvClass::E_MixedBuoy_StableCeiling
                                                      : IntConvClass::E_MixedBuoy_UnstableCeiling;
        } break;
        case SurfaceClass::Floor: {
            surfIntConv.convClass = (deltaTemp > 0.0) ? // surface is hotter so unstable
                                        IntConvClass::E_MixedBuoy_UnstableFloor
                                                      : IntConvClass::E_MixedBuoy_StableFloor;
        } break;
        case SurfaceClass::Window: {
        case SurfaceClass::GlassDoor:
        case SurfaceClass::TDD_Diffuser: {
            surfIntConv.convClass = IntConvClass::E_MixedBuoy_Windows;
        } break;
        case SurfaceClass::IntMass: {
            surfIntConv.convClass = (deltaTemp > 0.0) ? IntConvClass::E_MixedBuoy_UnstableFloor : IntConvClass::E_MixedBuoy_StableFloor;
        } break;
        default:
            assert(false);
        }

            if (surfIntConv.convClass == IntConvClass::Invalid) {
                ShowSevereError(state,
                                format("DynamicIntConvSurfaceClassification: failed to resolve Hc model for E surface named {}", surface.Name));
            }
        }
    } break; // E

    default:
        ShowSevereError(state,
                        format("DynamicIntConvSurfaceClassification: failed to determine zone flow regime for surface named {}", surface.Name));
    }

    // Set report var after surface has been classified
    surfIntConv.convClassRpt = IntConvClassReportVals[(int)surfIntConv.convClass];
}

void MapIntConvClassToHcModels(EnergyPlusData &state, int const SurfNum) // surface pointer index
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS SUBROUTINE:
    // Map Hc model equation data from central structure to surface structure

    // METHODOLOGY EMPLOYED:
    // Long case statement depends on surface classification determined in DynamicIntConvSurfaceClassification
    // then simply map data stored in InsideFaceAdaptiveConvectionAlgo into the surface's structure
    // if model type is user-defined, also store the index to the user curve to be used.

    auto &surfIntConv = state.dataSurface->surfIntConv(SurfNum);
    IntConvClass intConvClass = surfIntConv.convClass;
    assert(intConvClass != IntConvClass::Invalid);

    switch (intConvClass) {
    // A few cases require special handling
    case IntConvClass::C_CentralAirHeat_Walls: {
        if ((surfIntConv.zonePerimLength == 0.0) &&
            (state.dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)intConvClass] == HcInt::GoldsteinNovoselacCeilingDiffuserWalls)) {
            // no perimeter, Goldstein Novolselac model not good so revert to fisher pedersen model
            surfIntConv.hcModelEq = HcInt::FisherPedersenCeilDiffuserWalls;
            surfIntConv.hcModelEqRpt = HcIntReportVals[(int)surfIntConv.hcModelEq];
        } else {
            surfIntConv.hcModelEq = state.dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)intConvClass];
            surfIntConv.hcModelEqRpt = HcIntReportVals[(int)surfIntConv.hcModelEq];
        }
        if (surfIntConv.hcModelEq == HcInt::UserCurve) {
            surfIntConv.hcUserCurveNum = state.dataConvect->intAdaptiveConvAlgo.intConvClassUserCurveNums[(int)intConvClass];
        }
    } break;

    case IntConvClass::C_CentralAirHeat_Floor: {
        if ((surfIntConv.zonePerimLength == 0.0) &&
            (state.dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)intConvClass] == HcInt::GoldsteinNovoselacCeilingDiffuserFloor)) {
            // no perimeter, Goldstein Novolselac model not good so revert to fisher pedersen model
            surfIntConv.hcModelEq = HcInt::FisherPedersenCeilDiffuserFloor;
            surfIntConv.hcModelEqRpt = HcIntReportVals[(int)surfIntConv.hcModelEq];
        } else {
            surfIntConv.hcModelEq = state.dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)intConvClass];
            surfIntConv.hcModelEqRpt = HcIntReportVals[(int)surfIntConv.hcModelEq];
        }
        if (surfIntConv.hcModelEq == HcInt::UserCurve) {
            surfIntConv.hcUserCurveNum = state.dataConvect->intAdaptiveConvAlgo.intConvClassUserCurveNums[(int)intConvClass];
        }
    } break;

    case IntConvClass::C_CentralAirHeat_Windows: {
        if ((surfIntConv.zonePerimLength == 0.0) &&
            (state.dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)intConvClass] == HcInt::GoldsteinNovoselacCeilingDiffuserWindow)) {
            // no perimeter, Goldstein Novolselac model not good so revert to ISO15099
            surfIntConv.hcModelEq = HcInt::ISO15099Windows;
            surfIntConv.hcModelEqRpt = HcIntReportVals[(int)surfIntConv.hcModelEq];
        } else {
            surfIntConv.hcModelEq = state.dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)intConvClass];
            surfIntConv.hcModelEqRpt = HcIntReportVals[(int)surfIntConv.hcModelEq];
        }
        if (surfIntConv.hcModelEq == HcInt::UserCurve) {
            surfIntConv.hcUserCurveNum = state.dataConvect->intAdaptiveConvAlgo.intConvClassUserCurveNums[(int)intConvClass];
        }
    } break;

    default: { // Invalid has been asserted above so we can use default here
        surfIntConv.hcModelEq = state.dataConvect->intAdaptiveConvAlgo.intConvClassEqNums[(int)intConvClass];
        surfIntConv.hcModelEqRpt = HcIntReportVals[(int)surfIntConv.hcModelEq];
        if (surfIntConv.hcModelEq == HcInt::UserCurve) {
            surfIntConv.hcUserCurveNum = state.dataConvect->intAdaptiveConvAlgo.intConvClassUserCurveNums[(int)intConvClass];
        }
    }
    } // switch (intConvClass)
}

Real64 CalcUserDefinedIntHcModel(EnergyPlusData &state, int const SurfNum, int const UserCurveNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS SUBROUTINE:
    // calculate user-defined convection correlations for inside face

    // METHODOLOGY EMPLOYED:
    // call curve objects to evaluate user's model equation
    // prepare independent parameters for x values

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 tmpAirTemp;
    Real64 AirChangeRate;

    auto const &surface = state.dataSurface->Surface(SurfNum);
    int zoneNum = state.dataSurface->Surface(SurfNum).Zone;
    int spaceNum = state.dataSurface->Surface(SurfNum).spaceNum;
    auto const &zone = state.dataHeatBal->Zone(zoneNum);

    Real64 SumMdotTemp = 0.0;
    Real64 SumMdot = 0.0;
    Real64 SupplyAirTemp = state.dataZoneTempPredictorCorrector->zoneHeatBalance(zoneNum).MAT;
    if (zone.IsControlled) {
        auto const &zoneNode = state.dataLoopNodes->Node(zone.SystemZoneNodeNumber);
        Real64 AirDensity = Psychrometrics::PsyRhoAirFnPbTdbW(
            state, state.dataEnvrn->OutBaroPress, zoneNode.Temp, Psychrometrics::PsyWFnTdpPb(state, zoneNode.Temp, state.dataEnvrn->OutBaroPress));
        AirChangeRate = (zoneNode.MassFlowRate * Constant::rSecsInHour) / (AirDensity * zone.Volume);

        auto const &zoneEquipConfig = state.dataZoneEquip->ZoneEquipConfig(surface.Zone);
        if (zoneEquipConfig.EquipListIndex > 0) {
            auto const &zoneEquipList = state.dataZoneEquip->ZoneEquipList(zoneEquipConfig.EquipListIndex);
            for (int EquipNum = 1; EquipNum <= zoneEquipList.NumOfEquipTypes; ++EquipNum) {
                if (!allocated(zoneEquipList.EquipData(EquipNum).OutletNodeNums)) {
                    continue;
                }

                int zoneInletNodeNum = zoneEquipList.EquipData(EquipNum).OutletNodeNums(1);
                if (zoneInletNodeNum <= 0) {
                    continue;
                }
                auto const &zoneInletNode = state.dataLoopNodes->Node(zoneInletNodeNum);
                if (zoneInletNode.MassFlowRate > 0.0) { // Technically speaking, this check is not necessary since x += 0.0 is x.
                    SumMdotTemp += zoneInletNode.MassFlowRate * zoneInletNode.Temp;
                }
            } // for (EquipNum)
        }
        if (SumMdot > 0.0) {
            SupplyAirTemp = SumMdotTemp / SumMdot; // mass flow weighted inlet temperature
        }
    }

    auto &userCurve = state.dataConvect->hcIntUserCurve(UserCurveNum);
    auto const &surfIntConv = state.dataSurface->surfIntConv(SurfNum);

    switch (userCurve.refTempType) {
    case RefTemp::MeanAirTemp:
        tmpAirTemp = state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum).MAT;
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneMeanAirTemp;
        break;
    case RefTemp::AdjacentAirTemp:
        tmpAirTemp = state.dataHeatBal->SurfTempEffBulkAir(SurfNum);
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::AdjacentAirTemp;
        break;
    case RefTemp::SupplyAirTemp:
        tmpAirTemp = SupplyAirTemp;
        state.dataSurface->SurfTAirRef(SurfNum) = DataSurfaces::RefAirTemp::ZoneSupplyAirTemp;
        break;
    default:
        assert(false);
    }

    state.dataSurface->SurfTAirRefRpt(SurfNum) = DataSurfaces::SurfTAirRefReportVals[state.dataSurface->SurfTAirRef(SurfNum)];

    Real64 HcFnTempDiff(0.0), HcFnTempDiffDivHeight(0.0), HcFnACH(0.0), HcFnACHDivPerimLength(0.0);
    Kiva::ConvectionAlgorithm HcFnTempDiffFn(KIVA_CONST_CONV(0.0)), HcFnTempDiffDivHeightFn(KIVA_CONST_CONV(0.0));
    if (userCurve.hcFnTempDiffCurveNum > 0) {
        HcFnTempDiff =
            Curve::CurveValue(state, userCurve.hcFnTempDiffCurveNum, std::abs(state.dataHeatBalSurf->SurfInsideTempHist(1)(SurfNum) - tmpAirTemp));
        HcFnTempDiffFn = [&](double Tsurf, double Tamb, double, double, double) -> double {
            return Curve::CurveValue(state, userCurve.hcFnTempDiffCurveNum, std::abs(Tsurf - Tamb));
        };
    }

    if (userCurve.hcFnTempDiffDivHeightCurveNum > 0) {
        HcFnTempDiffDivHeight =
            Curve::CurveValue(state,
                              userCurve.hcFnTempDiffDivHeightCurveNum,
                              (std::abs(state.dataHeatBalSurf->SurfInsideTempHist(1)(SurfNum) - tmpAirTemp) / surfIntConv.zoneWallHeight));
        HcFnTempDiffDivHeightFn = [=, &state](double Tsurf, double Tamb, double, double, double) -> double {
            return Curve::CurveValue(state, userCurve.hcFnTempDiffDivHeightCurveNum, std::abs(Tsurf - Tamb) / surfIntConv.zoneWallHeight);
        };
    }

    if (userCurve.hcFnACHCurveNum > 0) {
        HcFnACH = Curve::CurveValue(state, userCurve.hcFnACHCurveNum, AirChangeRate);
    }

    if (userCurve.hcFnACHDivPerimLengthCurveNum > 0) {
        HcFnACHDivPerimLength = Curve::CurveValue(state, userCurve.hcFnACHDivPerimLengthCurveNum, (AirChangeRate / surfIntConv.zonePerimLength));
    }

    if (state.dataSurface->Surface(SurfNum).ExtBoundCond == DataSurfaces::KivaFoundation) {
        state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].in =
            [=](double Tsurf, double Tamb, double HfTerm, double Roughness, double CosTilt) -> double {
            return HcFnTempDiffFn(Tsurf, Tamb, HfTerm, Roughness, CosTilt) + HcFnTempDiffDivHeightFn(Tsurf, Tamb, HfTerm, Roughness, CosTilt) +
                   HcFnACH + HcFnACHDivPerimLength;
        };
        return 0.0;
    } else {
        return HcFnTempDiff + HcFnTempDiffDivHeight + HcFnACH + HcFnACHDivPerimLength;
    }
}

Real64 CalcUserDefinedExtHcModel(EnergyPlusData &state, int const SurfNum, int const UserCurveNum)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS SUBROUTINE:
    // calculate user-defined convection correlations for outside face

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    Real64 windVel;
    Real64 Theta;
    Real64 ThetaRad;

    auto &userCurve = state.dataConvect->hcExtUserCurve(UserCurveNum);
    auto const &surface = state.dataSurface->Surface(SurfNum);

    switch (userCurve.windSpeedType) {
    case RefWind::WeatherFile:
        windVel = state.dataEnvrn->WindSpeed;
        break;
    case RefWind::AtZ:
        windVel = state.dataSurface->SurfOutWindSpeed(SurfNum);
        break;
    case RefWind::ParallelComp:
        // WindSpeed , WindDir, surface Azimuth
        Theta = CalcWindSurfaceTheta(state.dataEnvrn->WindDir, surface.Azimuth);
        ThetaRad = Theta * Constant::DegToRad;
        break;
    case RefWind::ParallelCompAtZ:
        // Surface WindSpeed , Surface WindDir, surface Azimuth
        Theta = CalcWindSurfaceTheta(state.dataSurface->SurfOutWindDir(SurfNum), surface.Azimuth);
        ThetaRad = Theta * Constant::DegToRad;
        windVel = std::cos(ThetaRad) * state.dataSurface->SurfOutWindSpeed(SurfNum);
        break;
    default:
        assert(false);
    }

    Kiva::ForcedConvectionTerm HfFnWindSpeedFn(KIVA_HF_DEF);
    Kiva::ConvectionAlgorithm HnFnTempDiffFn(KIVA_CONST_CONV(0.0)), HnFnTempDiffDivHeightFn(KIVA_CONST_CONV(0.0));

    Real64 HfFnWindSpeed(0.0), HnFnTempDiff(0.0), HnFnTempDiffDivHeight(0.0);

    if (userCurve.hfFnWindSpeedCurveNum > 0) {
        HfFnWindSpeed = Curve::CurveValue(state, userCurve.hfFnWindSpeedCurveNum, windVel);
        HfFnWindSpeedFn = [&](double, double, double, double windSpeed) -> double {
            return Curve::CurveValue(state, userCurve.hfFnWindSpeedCurveNum, windSpeed);
        };
    }

    auto const &surfExtConv = state.dataSurface->surfExtConv(SurfNum);

    Real64 surfDeltaTemp = std::abs(state.dataHeatBalSurf->SurfInsideTempHist(1)(SurfNum) - state.dataSurface->SurfOutDryBulbTemp(SurfNum));
    if (userCurve.hnFnTempDiffCurveNum > 0) {
        HnFnTempDiff = Curve::CurveValue(state, userCurve.hnFnTempDiffCurveNum, surfDeltaTemp);
        HnFnTempDiffFn = [&](double Tsurf, double Tamb, double, double, double) -> double {
            return Curve::CurveValue(state, userCurve.hnFnTempDiffCurveNum, std::abs(Tsurf - Tamb));
        };
    }

    if (userCurve.hnFnTempDiffDivHeightCurveNum > 0) {
        if (surfExtConv.faceHeight > 0.0) {
            HnFnTempDiffDivHeight = Curve::CurveValue(state, userCurve.hnFnTempDiffDivHeightCurveNum, surfDeltaTemp / surfExtConv.faceHeight);
            HnFnTempDiffDivHeightFn = [=, &state](double Tsurf, double Tamb, double, double, double) -> double {
                return Curve::CurveValue(state, userCurve.hnFnTempDiffDivHeightCurveNum, (std::abs(Tsurf - Tamb) / surfExtConv.faceHeight));
            };
        }
    }

    if (surface.ExtBoundCond == DataSurfaces::KivaFoundation) {
        state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].f = HfFnWindSpeedFn;
        state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].out =
            [=](double Tsurf, double Tamb, double HfTerm, double Roughness, double CosTilt) -> double {
            return HnFnTempDiffFn(Tsurf, Tamb, HfTerm, Roughness, CosTilt) + HnFnTempDiffDivHeightFn(Tsurf, Tamb, HfTerm, Roughness, CosTilt) +
                   HfTerm;
        };
    }
    return HfFnWindSpeed + HnFnTempDiff + HnFnTempDiffDivHeight;
}

//** Begin catalog of Hc equation functions. **** !*************************************************

Real64 CalcFisherPedersenCeilDiffuserFloor(EnergyPlusData &state,
                                           Real64 const ACH, // [1/hr] air system air change rate
                                           Real64 const Tsurf,
                                           Real64 const Tair,
                                           Real64 const cosTilt,
                                           Real64 const humRat,
                                           Real64 const height,
                                           bool const isWindow)
{

    // AUTHOR: Brent Griffith (Aug 2010)
    // PURPOSE OF THIS FUNCTION: Calculate the model equation by Fisher and Pedersen for floors with ceiling diffusers
    // REFERENCE: Fisher, D.E. and C.O. Pedersen, Convective Heat Transfer in Building Energy and Thermal Load Calculations,
    //            ASHRAE Transactions, vol. 103, Pt. 2, 1997, p.13

    if (ACH >= 3.0) {
        return 3.873 + 0.082 * std::pow(ACH, 0.98);
    } else {                               // Revert to purely natural convection
        Real64 Hforced = 4.11365377688938; // Value of Hforced when ACH=3
        return CalcFisherPedersenCeilDiffuserNatConv(state, Hforced, ACH, Tsurf, Tair, cosTilt, humRat, height, isWindow);
    }
}

Real64 CalcFisherPedersenCeilDiffuserCeiling(EnergyPlusData &state,
                                             Real64 const ACH, // [1/hr] air system air change rate
                                             Real64 const Tsurf,
                                             Real64 const Tair,
                                             Real64 const cosTilt,
                                             Real64 const humRat,
                                             Real64 const height,
                                             bool const isWindow)
{

    // AUTHOR: Brent Griffith (Aug 2010)
    // PURPOSE OF THIS FUNCTION: Calculate the model equation by Fisher and Pedersen for floors with ceiling diffusers
    // REFERENCE: Fisher, D.E. and C.O. Pedersen, Convective Heat Transfer in Building Energy and Thermal Load Calculations,
    //            ASHRAE Transactions, vol. 103, Pt. 2, 1997, p.13

    if (ACH >= 3.0) {
        return 2.234 + 4.099 * std::pow(ACH, 0.503);
    } else {                               // Revert to purely natural convection
        Real64 Hforced = 9.35711423763866; // Value of Hforced when ACH=3
        return CalcFisherPedersenCeilDiffuserNatConv(state, Hforced, ACH, Tsurf, Tair, cosTilt, humRat, height, isWindow);
    }
}

Real64 CalcFisherPedersenCeilDiffuserWalls(EnergyPlusData &state,
                                           Real64 const ACH, // [1/hr] air system air change rate
                                           Real64 const Tsurf,
                                           Real64 const Tair,
                                           Real64 const cosTilt,
                                           Real64 const humRat,
                                           Real64 const height,
                                           bool const isWindow)
{

    // AUTHOR: Brent Griffith (Aug 2010)
    // PURPOSE OF THIS FUNCTION: Calculate the model equation by Fisher and Pedersen for floors with ceiling diffusers
    // REFERENCE: Fisher, D.E. and C.O. Pedersen, Convective Heat Transfer in Building Energy and Thermal Load Calculations,
    //            ASHRAE Transactions, vol. 103, Pt. 2, 1997, p.13

    if (ACH >= 3.0) {
        return 1.208 + 1.012 * std::pow(ACH, 0.604);
    } else {                               // Revert to purely natural convection
        Real64 Hforced = 3.17299636062606; // Value of Hforced when ACH=3
        return CalcFisherPedersenCeilDiffuserNatConv(state, Hforced, ACH, Tsurf, Tair, cosTilt, humRat, height, isWindow);
    }
}

Real64 CalcFisherPedersenCeilDiffuserNatConv(EnergyPlusData &state,
                                             Real64 const Hforced,
                                             Real64 const ACH,
                                             Real64 const Tsurf,
                                             Real64 const Tair,
                                             Real64 const cosTilt,
                                             Real64 const humRat,
                                             Real64 const height,
                                             bool const isWindow)
{

    Real64 Hnatural;

    if (isWindow) {                        // Unlikely for a floor, but okay...
        Real64 const tilt = acos(cosTilt); // outward facing tilt
        Real64 const sinTilt = sin(tilt);
        Hnatural = CalcISO15099WindowIntConvCoeff(state, Tsurf, Tair, humRat, height, tilt, sinTilt);
    } else {
        Hnatural = CalcASHRAETARPNatural(Tsurf, Tair, -cosTilt); // negative cosTilt because interior of surface
    }
    if (ACH <= 0.5) {
        return Hnatural;
    } else {
        return Hnatural + ((Hforced - Hnatural) * ((ACH - 0.5) / 2.5)); // range for interpolation goes from ACH=0.5 to ACH=3.0 or a range of 2.5
    }
}

Real64 CalcAlamdariHammondUnstableHorizontal(Real64 const DeltaTemp,        // [C] temperature difference between surface and air
                                             Real64 const HydraulicDiameter // [m] characteristic size, = (4 * area) / perimeter
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Jul 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate model equation for Alamdari and Hammond
    // This function only for the Unstable heat flow direction for horizontal surfaces

    // REFERENCES:
    // Alamdari, F. and G.P. Hammond. 1983. Improved data correlations
    // for buoyancy-driven convection in rooms.  Building Services Engineering
    // Research & Technology. Vol. 4, No. 3.

    return std::pow(pow_6(1.4 * std::pow(std::abs(DeltaTemp) / HydraulicDiameter, 0.25)) + (1.63 * pow_2(DeltaTemp)),
                    1.0 / 6.0); // Tuned pow_6( std::pow( std::abs( DeltaTemp ), 1.0/3.0 ) ) changed to pow_2( DeltaTemp )
}

Real64 CalcAlamdariHammondUnstableHorizontal(EnergyPlusData &state,
                                             Real64 const DeltaTemp,         // [C] temperature difference between surface and air
                                             Real64 const HydraulicDiameter, // [m] characteristic size, = (4 * area) / perimeter
                                             int const SurfNum               // for messages
)
{
    std::string_view constexpr routineName = "CalcAlamdariHammondUnstableHorizontal";
    if (HydraulicDiameter > 0.0) {
        return CalcAlamdariHammondUnstableHorizontal(DeltaTemp, HydraulicDiameter);
    } else {
        ErrorObjectHeader eoh{routineName, "Surface", state.dataSurface->Surface(SurfNum).Name};
        ShowWarningHydraulicDiameterZero(state, state.dataConvect->AHUnstableHorizontalErrorIDX, eoh);
        return 9.999;
    }
}

Real64 CalcAlamdariHammondStableHorizontal(Real64 const DeltaTemp,        // [C] temperature difference between surface and air
                                           Real64 const HydraulicDiameter // [m] characteristic size, = (4 * area) / perimeter
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Jul 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate model equation for Alamdari and Hammond
    // This function only for the Stable heat flow direction for horizontal surfaces

    // REFERENCES:
    // Alamdari, F. and G.P. Hammond. 1983. Improved data correlations
    // for buoyancy-driven convection in rooms.  Building Services Engineering
    // Research & Technology. Vol. 4, No. 3.

    return 0.6 * std::pow(std::abs(DeltaTemp) / pow_2(HydraulicDiameter), 0.2);
}

Real64 CalcAlamdariHammondStableHorizontal(EnergyPlusData &state,
                                           Real64 const DeltaTemp,         // [C] temperature difference between surface and air
                                           Real64 const HydraulicDiameter, // [m] characteristic size, = (4 * area) / perimeter
                                           int const SurfNum               // for messages
)
{
    std::string_view constexpr routineName = "CalcAlamdariHammondStableHorizontal";
    if (HydraulicDiameter > 0.0) {
        return CalcAlamdariHammondStableHorizontal(DeltaTemp, HydraulicDiameter);
    } else {
        ErrorObjectHeader eoh{routineName, "Surface", state.dataSurface->Surface(SurfNum).Name};
        ShowWarningHydraulicDiameterZero(state, state.dataConvect->AHStableHorizontalErrorIDX, eoh);
        if (DeltaTemp == 0.0 && !state.dataGlobal->WarmupFlag) {
            ShowWarningDeltaTempZero(state, state.dataConvect->BMMixedAssistedWallErrorIDX1, eoh);
        }
        return 9.999;
    }
}

Real64 CalcAlamdariHammondVerticalWall(Real64 const DeltaTemp, // [C] temperature difference between surface and air
                                       Real64 const Height)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Jul 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate model equation for Alamdari and Hammond
    // This function only for the vertical wall surfaces

    // METHODOLOGY EMPLOYED:
    // isolate function for equation.

    // REFERENCES:
    // Alamdari, F. and G.P. Hammond. 1983. Improved data correlations
    // for buoyancy-driven convection in rooms.  Building Services Engineering
    // Research & Technology. Vol. 4, No. 3.

    return std::pow(pow_6(1.5 * std::pow(std::abs(DeltaTemp) / Height, 0.25)) + (1.23 * pow_2(DeltaTemp)),
                    1.0 / 6.0); // Tuned pow_6( std::pow( std::abs( DeltaTemp ), 1.0/3.0 ) ) changed to pow_2( DeltaTemp )
}

Real64 CalcAlamdariHammondVerticalWall(EnergyPlusData &state,
                                       Real64 const DeltaTemp, // [C] temperature difference between surface and air
                                       Real64 const Height,    // [m] characteristic size, = zone height
                                       int const SurfNum       // for messages
)
{
    std::string_view constexpr routineName = "CalcAlamdariHammondVerticalWall";

    if (Height > 0.0) {
        return CalcAlamdariHammondVerticalWall(DeltaTemp, Height);
    } else {
        ErrorObjectHeader eoh{routineName, "Surface", state.dataSurface->Surface(SurfNum).Name};
        ShowWarningHydraulicDiameterZero(state, state.dataConvect->AHVerticalWallErrorIDX, eoh);
        return 9.999;
    }
}

Real64 CalcKhalifaEq3WallAwayFromHeat(Real64 const DeltaTemp) // [C] temperature difference between surface and air
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Jul 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate model equation for Khalifa's Eq 3 for Walls Away From Heat

    // METHODOLOGY EMPLOYED:
    // isolate function for equation.

    // REFERENCES:
    // Khalifa AJN. 1989 Heat transfer processes in buildings. Ph.D. Thesis,
    //   University of Wales College of Cardiff, Cardiff, UK.
    // Equations actually from Beausoleil-Morrison 2000 who referenced Khalifa
    // Beausoleil-Morrison, I. 2000. The adaptive coupling of heat and
    //  air flow modeling within dynamic whole-building simulations.
    //  PhD. Thesis. University of Strathclyde, Glasgow, UK.

    return 2.07 * std::pow(std::abs(DeltaTemp), 0.23);
}

Real64 CalcKhalifaEq4CeilingAwayFromHeat(Real64 const DeltaTemp) // [C] temperature difference between surface and air
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Jul 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate model equation for Khalifa's Eq 4 for Ceilings Away From Heat

    // REFERENCES:
    // Khalifa AJN. 1989 Heat transfer processes in buildings. Ph.D. Thesis,
    //   University of Wales College of Cardiff, Cardiff, UK.
    // Equations actually from Beausoleil-Morrison 2000 who referenced Khalifa
    // Beausoleil-Morrison, I. 2000. The adaptive coupling of heat and
    //  air flow modeling within dynamic whole-building simulations.
    //  PhD. Thesis. University of Strathclyde, Glasgow, UK.

    return 2.72 * std::pow(std::abs(DeltaTemp), 0.13);
}

Real64 CalcKhalifaEq5WallsNearHeat(Real64 const DeltaTemp) // [C] temperature difference between surface and air
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Jul 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate model equation for Khalifa's Eq 5 for Walls near the heater

    // REFERENCES:
    // Khalifa AJN. 1989 Heat transfer processes in buildings. Ph.D. Thesis,
    //   University of Wales College of Cardiff, Cardiff, UK.
    // Equations actually from Beausoleil-Morrison 2000 who referenced Khalifa
    // Beausoleil-Morrison, I. 2000. The adaptive coupling of heat and
    //  air flow modeling within dynamic whole-building simulations.
    //  PhD. Thesis. University of Strathclyde, Glasgow, UK.

    return 1.98 * std::pow(std::abs(DeltaTemp), 0.32);
}

Real64 CalcKhalifaEq6NonHeatedWalls(Real64 const DeltaTemp) // [C] temperature difference between surface and air
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Jul 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate model equation for Khalifa's Eq 6 for non-heated walls

    // REFERENCES:
    // Khalifa AJN. 1989 Heat transfer processes in buildings. Ph.D. Thesis,
    //   University of Wales College of Cardiff, Cardiff, UK.
    // Equations actually from Beausoleil-Morrison 2000 who referenced Khalifa
    // Beausoleil-Morrison, I. 2000. The adaptive coupling of heat and
    //  air flow modeling within dynamic whole-building simulations.
    //  PhD. Thesis. University of Strathclyde, Glasgow, UK.

    return 2.30 * std::pow(std::abs(DeltaTemp), 0.24);
}

Real64 CalcKhalifaEq7Ceiling(Real64 const DeltaTemp) // [C] temperature difference between surface and air
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Jul 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate model equation for Khalifa's Eq 7 for ceilings

    // REFERENCES:
    // Khalifa AJN. 1989 Heat transfer processes in buildings. Ph.D. Thesis,
    //   University of Wales College of Cardiff, Cardiff, UK.
    // Equations actually from Beausoleil-Morrison 2000 who referenced Khalifa
    // Beausoleil-Morrison, I. 2000. The adaptive coupling of heat and
    //  air flow modeling within dynamic whole-building simulations.
    //  PhD. Thesis. University of Strathclyde, Glasgow, UK.

    return 3.10 * std::pow(std::abs(DeltaTemp), 0.17);
}

Real64 CalcAwbiHattonHeatedFloor(Real64 const DeltaTemp,        // [C] temperature difference between surface and air
                                 Real64 const HydraulicDiameter // [m] characteristic size, = (4 * area) / perimeter
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Jul 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate model equation for Awbi and Hatton for heated floors

    // METHODOLOGY EMPLOYED:
    // apply numerical protection for low values of hydraulic diameter

    // REFERENCES:
    // Awbi, H.B. and A. Hatton. 1999. Natural convection from heated room surfaces.
    //   Energy and Buildings 30 (1999) 233-244.
    //   This function is for equation 15 in the reference

    if (HydraulicDiameter > 1.0) {
        return 2.175 * std::pow(std::abs(DeltaTemp), 0.308) / std::pow(HydraulicDiameter, 0.076);
    } else {
        Real64 const pow_fac(2.175 / std::pow(1.0, 0.076));
        return pow_fac * std::pow(std::abs(DeltaTemp), 0.308);
    }
}

Real64 CalcAwbiHattonHeatedWall(Real64 const DeltaTemp,        // [C] temperature difference between surface and air
                                Real64 const HydraulicDiameter // [m] characteristic size, = (4 * area) / perimeter
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Jul 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate model equation for Awbi and Hatton for heated walls

    // REFERENCES:
    // Awbi, H.B. and A. Hatton. 1999. Natural convection from heated room surfaces.
    //   Energy and Buildings 30 (1999) 233-244.
    //   This function is for equation 12 in the reference

    return 1.823 * std::pow(std::abs(DeltaTemp), 0.293) / std::pow(max(HydraulicDiameter, 1.0), 0.121);
}

Real64 CalcBeausoleilMorrisonMixedAssistedWall(Real64 const DeltaTemp,     // [C] temperature difference between surface and air
                                               Real64 const Height,        // [m] characteristic size
                                               Real64 const SurfTemp,      // [C] surface temperature
                                               Real64 const SupplyAirTemp, // [C] temperature of supply air into zone
                                               Real64 const AirChangeRate  // [ACH] [1/hour] supply air ACH for zone
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Jul 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate model equation Beausoleil-Morrison's mixed flow regime
    // with mechanical and buoyancy forces assisting each other along a Wall

    // REFERENCES:
    // Beausoleil-Morrison, I. 2000. The adaptive coupling of heat and
    //  air flow modeling within dynamic whole-building simulations.
    //  PhD. Thesis. University of Strathclyde, Glasgow, UK.

    Real64 cofpow =
        std::sqrt(pow_6(1.5 * std::pow(std::abs(DeltaTemp) / Height, 0.25)) + std::pow(1.23 * pow_2(DeltaTemp), 1.0 / 6.0)) +
        pow_3(((SurfTemp - SupplyAirTemp) / std::abs(DeltaTemp)) *
              (-0.199 + 0.190 * std::pow(AirChangeRate,
                                         0.8))); // Tuned pow_6( std::pow( std::abs( DeltaTemp ), 1.0/3.0 ) ) changed to pow_2( DeltaTemp )
    Real64 Hc = std::pow(std::abs(cofpow),
                         1.0 / 3.0); // Tuned pow_6( std::pow( std::abs( DeltaTemp ), 1.0/3.0 ) ) changed to pow_2( DeltaTemp )
    if (cofpow < 0.0) {
        Hc = -Hc;
    }
    return Hc;
}

Real64 CalcBeausoleilMorrisonMixedAssistedWall(EnergyPlusData &state,
                                               Real64 const DeltaTemp, // [C] temperature difference between surface and air
                                               Real64 const Height,    // [m] characteristic size
                                               Real64 const SurfTemp,  // [C] surface temperature
                                               int const ZoneNum       // index of zone for messaging
)
{
    std::string_view constexpr routineName = "CalcBeausoleilMorrisonMixedAssistedWall";

    if ((std::abs(DeltaTemp) > HVAC::SmallTempDiff) && (Height != 0.0)) {
        Real64 SupplyAirTemp = CalcZoneSupplyAirTemp(state, ZoneNum);
        Real64 AirChangeRate = CalcZoneSystemACH(state, ZoneNum);
        return CalcBeausoleilMorrisonMixedAssistedWall(DeltaTemp, Height, SurfTemp, SupplyAirTemp, AirChangeRate);
    } else {
        ErrorObjectHeader eoh{routineName, "Zone", state.dataHeatBal->Zone(ZoneNum).Name};
        if (Height == 0.0) {
            ShowWarningHydraulicDiameterZero(state, state.dataConvect->BMMixedAssistedWallErrorIDX2, eoh);
        }
        if (DeltaTemp == 0.0 && !state.dataGlobal->WarmupFlag) {
            ShowWarningDeltaTempZero(state, state.dataConvect->BMMixedAssistedWallErrorIDX1, eoh);
        }
        return 9.999;
    }
}

Real64 CalcBeausoleilMorrisonMixedOpposingWall(Real64 const DeltaTemp,     // [C] temperature difference between surface and air
                                               Real64 const Height,        // [m] characteristic size
                                               Real64 const SurfTemp,      // [C] surface temperature
                                               Real64 const SupplyAirTemp, // [C] temperature of supply air into zone
                                               Real64 const AirChangeRate  // [ACH] [1/hour] supply air ACH for zone
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Jul 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate model equation Beausoleil-Morrison's mixed flow regime
    // with mechanical and buoyancy forces opposing each other along a Wall

    // REFERENCES:
    // Beausoleil-Morrison, I. 2000. The adaptive coupling of heat and
    //  air flow modeling within dynamic whole-building simulations.
    //  PhD. Thesis. University of Strathclyde, Glasgow, UK.

    Real64 HcTmp1 = 9.999;
    Real64 HcTmp2 = 9.999;

    if (Height != 0.0) {
        Real64 cofpow =
            std::sqrt(pow_6(1.5 * std::pow(std::abs(DeltaTemp) / Height, 0.25)) + std::pow(1.23 * pow_2(DeltaTemp), 1.0 / 6.0)) -
            pow_3(((SurfTemp - SupplyAirTemp) / std::abs(DeltaTemp)) *
                  (-0.199 + 0.190 * std::pow(AirChangeRate,
                                             0.8))); // Tuned pow_6( std::pow( std::abs( DeltaTemp ), 1.0/3.0 ) ) changed to pow_2( DeltaTemp )
        HcTmp1 = std::pow(std::abs(cofpow),
                          1.0 / 3.0); // Tuned pow_6( std::pow( std::abs( DeltaTemp ), 1.0/3.0 ) ) changed to pow_2( DeltaTemp )
        if (cofpow < 0.0) {
            HcTmp1 = -HcTmp1;
        }

        HcTmp2 = 0.8 * std::pow(pow_6(1.5 * std::pow(std::abs(DeltaTemp) / Height, 0.25)) + (1.23 * pow_2(DeltaTemp)),
                                1.0 / 6.0); // Tuned pow_6( std::pow( std::abs( DeltaTemp ), 1.0/3.0 ) ) changed to pow_2( DeltaTemp )
    }

    Real64 HcTmp3 = 0.8 * ((SurfTemp - SupplyAirTemp) / std::abs(DeltaTemp)) * (-0.199 + 0.190 * std::pow(AirChangeRate, 0.8));

    return max(max(HcTmp1, HcTmp2), HcTmp3);
}

Real64 CalcBeausoleilMorrisonMixedOpposingWall(EnergyPlusData &state,
                                               Real64 const DeltaTemp, // [C] temperature difference between surface and air
                                               Real64 const Height,    // [m] characteristic size
                                               Real64 const SurfTemp,  // [C] surface temperature
                                               int const ZoneNum       // index of zone for messaging
)
{
    std::string_view constexpr routineName = "CalcBeausoleilMorrisonMixedOpposingWall";
    ErrorObjectHeader eoh{routineName, "Zone", state.dataHeatBal->Zone(ZoneNum).Name};
    if (std::abs(DeltaTemp) > HVAC::SmallTempDiff) { // protect divide by zero

        if (Height == 0.0) {
            ShowWarningHydraulicDiameterZero(state, state.dataConvect->BMMixedOpposingWallErrorIDX2, eoh);
            return 9.999;
        }
        Real64 SupplyAirTemp = CalcZoneSupplyAirTemp(state, ZoneNum);
        Real64 AirChangeRate = CalcZoneSystemACH(state, ZoneNum);
        return CalcBeausoleilMorrisonMixedOpposingWall(DeltaTemp, Height, SurfTemp, SupplyAirTemp, AirChangeRate);

    } else {
        if (!state.dataGlobal->WarmupFlag) {
            ShowWarningDeltaTempZero(state, state.dataConvect->BMMixedOpposingWallErrorIDX1, eoh);
        }
        return 9.999;
    }
}

Real64 CalcBeausoleilMorrisonMixedStableFloor(Real64 const DeltaTemp,         // [C] temperature difference between surface and air
                                              Real64 const HydraulicDiameter, // [m] characteristic size, = (4 * area) / perimeter
                                              Real64 const SurfTemp,          // [C] surface temperature
                                              Real64 const SupplyAirTemp,     // [C] temperature of supply air into zone
                                              Real64 const AirChangeRate      // [ACH] [1/hour] supply air ACH for zone
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Jul 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate model equation Beausoleil-Morrison's mixed flow regime
    // with mechanical and buoyancy forces acting on an thermally stable floor

    // REFERENCES:
    // Beausoleil-Morrison, I. 2000. The adaptive coupling of heat and
    //  air flow modeling within dynamic whole-building simulations.
    //  PhD. Thesis. University of Strathclyde, Glasgow, UK.

    Real64 cofpow = pow_3(0.6 * std::pow(std::abs(DeltaTemp) / HydraulicDiameter, 0.2)) +
                    pow_3(((SurfTemp - SupplyAirTemp) / std::abs(DeltaTemp)) * (0.159 + 0.116 * std::pow(AirChangeRate, 0.8)));
    Real64 Hc = std::pow(std::abs(cofpow), 1.0 / 3.0);
    if (cofpow < 0.0) {
        Hc = -Hc;
    }
    return Hc;
}

void ShowWarningHydraulicDiameterZero(EnergyPlusData &state, int &errorIdx, ErrorObjectHeader const &eoh)
{
    if (errorIdx == 0) {
        ShowWarningMessage(state, format("{}: Convection model not evaluated (would divide by zero)", eoh.routineName));
        ShowContinueError(
            state, format("Effective hydraulic diameter is zero, convection model not applicable for {} named {}", eoh.objectType, eoh.objectName));
        ShowContinueError(state, "Convection heat transfer coefficient set to 9.999 [W/m2-K] and the simulation continues");
    }
    ShowRecurringWarningErrorAtEnd(state,
                                   format("{}: Convection model not evaluated because effective hydraulic diameter is zero "
                                          "and set to 9.999 [W/m2-K]",
                                          eoh.routineName),
                                   errorIdx);
}

void ShowWarningDeltaTempZero(EnergyPlusData &state, int &errorIdx, ErrorObjectHeader const &eoh)
{
    if (errorIdx == 0) {
        ShowWarningMessage(state, format("{}: Convection model not evaluated (would divide by zero)", eoh.routineName));
        ShowContinueError(state, "The temperature difference between surface and air is zero");
        ShowContinueError(state, format("Occurs for {} named {}", eoh.objectType, eoh.objectName));
        ShowContinueError(state, "Convection surface heat transfer coefficient set to 9.999 [W/m2-K] and the simulation continues");
    }

    ShowRecurringWarningErrorAtEnd(state,
                                   format("{}: Convection model not evaluated because of zero temperature "
                                          "difference and set to 9.999 [W/m2-K]",
                                          eoh.routineName),
                                   errorIdx);
}

void ShowWarningWindowLocation(EnergyPlusData &state, int &errorIdx, ErrorObjectHeader const &eoh, IntConvWinLoc winLoc)
{
    if (errorIdx == 0) {
        ShowSevereMessage(state, format("{}: Convection model not evaluated (bad relative window location)", eoh.routineName));
        ShowContinueError(state, format("Value for window location = {}", winLoc));
        ShowContinueError(state, format("Occurs for {} named {}", eoh.objectType, eoh.objectName));
        ShowContinueError(state, "Convection surface heat transfer coefficient set to 9.999 [W/m2-K] and the simulation continues");
    }
    ShowRecurringSevereErrorAtEnd(state,
                                  format("{}: Convection model not evaluated because bad window "
                                         "location and set to 9.999 [W/m2-K]",
                                         eoh.routineName),
                                  errorIdx);
}

void ShowWarningPerimeterLengthZero(EnergyPlusData &state, int &errorIdx, ErrorObjectHeader const &eoh)
{
    if (errorIdx == 0) {
        ShowWarningError(state, format("{}: Convection model not evaluated (zero zone exterior perimeter length)", eoh.routineName));
        ShowContinueError(state, "Value for zone exterior perimeter length = 0.0");
        ShowContinueError(state, format("Occurs for {} named {}", eoh.objectType, eoh.objectName));
        ShowContinueError(state, "Convection surface heat transfer coefficient set to 9.999 [W/m2-K] and the simulation continues");
    }
    ShowRecurringSevereErrorAtEnd(state,
                                  format("{}: Convection model not evaluated because bad perimeter "
                                         "length and set to 9.999 [W/m2-K]",
                                         eoh.routineName),
                                  errorIdx);
}

void ShowWarningFaceAreaZero(EnergyPlusData &state, int &errorIdx, ErrorObjectHeader const &eoh)
{
    if (errorIdx == 0) {
        ShowSevereMessage(state, format("{}: Convection model not evaluated (bad face area)", eoh.routineName));
        ShowContinueError(state, "Value for effective face area = 0.0");
        ShowContinueError(state, format("Occurs for {} named {}", eoh.objectType, eoh.objectName));
        ShowContinueError(state, "Convection surface heat transfer coefficient set to 9.999 [W/m2-K] and the simulation continues");
    }
    ShowRecurringSevereErrorAtEnd(
        state, format("{}: Convection model not evaluated because bad face area and set to 9.999 [W/m2-k]", eoh.routineName), errorIdx);
}

Real64 CalcBeausoleilMorrisonMixedStableFloor(EnergyPlusData &state,
                                              Real64 const DeltaTemp,         // [C] temperature difference between surface and air
                                              Real64 const HydraulicDiameter, // [m] characteristic size, = (4 * area) / perimeter
                                              Real64 const SurfTemp,          // [C] surface temperature
                                              int const ZoneNum               // index of zone for messaging
)
{
    std::string_view constexpr routineName = "CalcBeausoleilMorrisonMixedStableFloor";

    if ((HydraulicDiameter != 0.0) && (std::abs(DeltaTemp) > HVAC::SmallTempDiff)) {
        Real64 SupplyAirTemp = CalcZoneSupplyAirTemp(state, ZoneNum);
        Real64 AirChangeRate = CalcZoneSystemACH(state, ZoneNum);
        return CalcBeausoleilMorrisonMixedStableFloor(DeltaTemp, HydraulicDiameter, SurfTemp, SupplyAirTemp, AirChangeRate);
    } else {
        ErrorObjectHeader eoh{routineName, "Zone", state.dataHeatBal->Zone(ZoneNum).Name};
        if (HydraulicDiameter == 0.0) {
            ShowWarningHydraulicDiameterZero(state, state.dataConvect->BMMixedStableFloorErrorIDX1, eoh);
        }
        if (DeltaTemp == 0.0 && !state.dataGlobal->WarmupFlag) {
            ShowWarningDeltaTempZero(state, state.dataConvect->BMMixedStableFloorErrorIDX2, eoh);
        }
        return 9.999;
    }
}

Real64 CalcBeausoleilMorrisonMixedUnstableFloor(Real64 const DeltaTemp,         // [C] temperature difference between surface and air
                                                Real64 const HydraulicDiameter, // [m] characteristic size, = (4 * area) / perimeter
                                                Real64 const SurfTemp,          // [C] surface temperature
                                                Real64 const SupplyAirTemp,     // [C] temperature of supply air into zone
                                                Real64 const AirChangeRate      // [ACH] [1/hour] supply air ACH for zone
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Jul 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate model equation Beausoleil-Morrison's mixed flow regime
    // with mechanical and buoyancy forces acting on an thermally unstable floor

    // REFERENCES:
    // Beausoleil-Morrison, I. 2000. The adaptive coupling of heat and
    //  air flow modeling within dynamic whole-building simulations.
    //  PhD. Thesis. University of Strathclyde, Glasgow, UK.

    Real64 cofpow =
        std::sqrt(pow_6(1.4 * std::pow(std::abs(DeltaTemp) / HydraulicDiameter, 0.25)) + pow_6(1.63 * std::pow(std::abs(DeltaTemp), 1.0 / 3.0))) +
        pow_3(((SurfTemp - SupplyAirTemp) / std::abs(DeltaTemp)) * (0.159 + 0.116 * std::pow(AirChangeRate, 0.8)));
    Real64 Hc = std::pow(std::abs(cofpow), 1.0 / 3.0);
    if (cofpow < 0.0) {
        Hc = -Hc;
    }

    return Hc;
}

Real64 CalcBeausoleilMorrisonMixedUnstableFloor(EnergyPlusData &state,
                                                Real64 const DeltaTemp,         // [C] temperature difference between surface and air
                                                Real64 const HydraulicDiameter, // [m] characteristic size, = (4 * area) / perimeter
                                                Real64 const SurfTemp,          // [C] surface temperature
                                                int const ZoneNum               // index of zone for messaging
)
{
    std::string_view constexpr routineName = "CalcBeausoleilMorrisonMixedUnstableFloor";
    if ((HydraulicDiameter != 0.0) && (std::abs(DeltaTemp) > HVAC::SmallTempDiff)) {
        Real64 SupplyAirTemp = CalcZoneSupplyAirTemp(state, ZoneNum);
        Real64 AirChangeRate = CalcZoneSystemACH(state, ZoneNum);
        return CalcBeausoleilMorrisonMixedUnstableFloor(DeltaTemp, HydraulicDiameter, SurfTemp, SupplyAirTemp, AirChangeRate);
    } else {
        ErrorObjectHeader eoh{routineName, "Zone", state.dataHeatBal->Zone(ZoneNum).Name};
        if (HydraulicDiameter == 0.0) {
            ShowWarningHydraulicDiameterZero(state, state.dataConvect->BMMixedUnstableFloorErrorIDX1, eoh);
        }

        if (DeltaTemp == 0.0 && !state.dataGlobal->WarmupFlag) {
            ShowWarningDeltaTempZero(state, state.dataConvect->BMMixedUnstableFloorErrorIDX2, eoh);
        }
        return 9.999;
    }
}

Real64 CalcBeausoleilMorrisonMixedStableCeiling(Real64 const DeltaTemp,         // [C] temperature difference between surface and air
                                                Real64 const HydraulicDiameter, // [m] characteristic size, = (4 * area) / perimeter
                                                Real64 const SurfTemp,          // [C] surface temperature
                                                Real64 const SupplyAirTemp,     // [C] temperature of supply air into zone
                                                Real64 const AirChangeRate      // [ACH] [1/hour] supply air ACH for zone
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Jul 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate model equation Beausoleil-Morrison's mixed flow regime
    // with mechanical and buoyancy forces acting on a thermally stable ceiling

    // REFERENCES:
    // Beausoleil-Morrison, I. 2000. The adaptive coupling of heat and
    //  air flow modeling within dynamic whole-building simulations.
    //  PhD. Thesis. University of Strathclyde, Glasgow, UK.

    Real64 cofpow = pow_3(0.6 * std::pow(std::abs(DeltaTemp) / HydraulicDiameter, 0.2)) +
                    pow_3(((SurfTemp - SupplyAirTemp) / std::abs(DeltaTemp)) * (-0.166 + 0.484 * std::pow(AirChangeRate, 0.8)));
    Real64 Hc = std::pow(std::abs(cofpow), 1.0 / 3.0);
    if (cofpow < 0.0) {
        Hc = -Hc;
    }
    return Hc;
}

Real64 CalcBeausoleilMorrisonMixedStableCeiling(EnergyPlusData &state,
                                                Real64 const DeltaTemp,         // [C] temperature difference between surface and air
                                                Real64 const HydraulicDiameter, // [m] characteristic size, = (4 * area) / perimeter
                                                Real64 const SurfTemp,          // [C] surface temperature
                                                int const ZoneNum               // index of zone for messaging
)
{
    std::string_view constexpr routineName = "CalcBeausoleilMorrisonMixedStableCeiling";
    if ((HydraulicDiameter != 0.0) && (std::abs(DeltaTemp) > HVAC::SmallTempDiff)) {
        Real64 SupplyAirTemp = CalcZoneSupplyAirTemp(state, ZoneNum);
        Real64 AirChangeRate = CalcZoneSystemACH(state, ZoneNum);
        return CalcBeausoleilMorrisonMixedStableCeiling(DeltaTemp, HydraulicDiameter, SurfTemp, SupplyAirTemp, AirChangeRate);
    } else {
        ErrorObjectHeader eoh{routineName, "Zone", state.dataHeatBal->Zone(ZoneNum).Name};

        if (HydraulicDiameter == 0.0) {
            ShowWarningHydraulicDiameterZero(state, state.dataConvect->BMMixedStableCeilingErrorIDX1, eoh);
        }
        if (DeltaTemp == 0.0 && !state.dataGlobal->WarmupFlag) {
            ShowWarningDeltaTempZero(state, state.dataConvect->BMMixedStableCeilingErrorIDX2, eoh);
        }
        return 9.999;
    }
}

Real64 CalcBeausoleilMorrisonMixedUnstableCeiling(Real64 const DeltaTemp,         // [C] temperature difference between surface and air
                                                  Real64 const HydraulicDiameter, // [m] characteristic size, = (4 * area) / perimeter
                                                  Real64 const SurfTemp,          // [C] surface temperature
                                                  Real64 const SupplyAirTemp,     // [C] temperature of supply air into zone
                                                  Real64 const AirChangeRate      // [ACH] [1/hour] supply air ACH for zone
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Jul 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate model equation Beausoleil-Morrison's mixed flow regime
    // with mechanical and buoyancy forces acting on a thermally unstable ceiling

    // REFERENCES:
    // Beausoleil-Morrison, I. 2000. The adaptive coupling of heat and
    //  air flow modeling within dynamic whole-building simulations.
    //  PhD. Thesis. University of Strathclyde, Glasgow, UK.

    Real64 cofpow =
        std::sqrt(pow_6(1.4 * std::pow(std::abs(DeltaTemp) / HydraulicDiameter, 0.25)) + pow_6(1.63 * std::pow(std::abs(DeltaTemp), 1.0 / 3.0))) +
        pow_3(((SurfTemp - SupplyAirTemp) / std::abs(DeltaTemp)) * (-0.166 + 0.484 * std::pow(AirChangeRate, 0.8)));
    Real64 Hc = std::pow(std::abs(cofpow), 1.0 / 3.0);
    if (cofpow < 0.0) {
        Hc = -Hc;
    }
    return Hc;
}

Real64 CalcBeausoleilMorrisonMixedUnstableCeiling(EnergyPlusData &state,
                                                  Real64 const DeltaTemp,         // [C] temperature difference between surface and air
                                                  Real64 const HydraulicDiameter, // [m] characteristic size, = (4 * area) / perimeter
                                                  Real64 const SurfTemp,          // [C] surface temperature
                                                  int const ZoneNum               // index of zone for messaging
)
{
    std::string_view constexpr routineName = "CalcBeausoleilMorrisonMixedUnstableCeiling";

    if ((HydraulicDiameter != 0.0) && (std::abs(DeltaTemp) > HVAC::SmallTempDiff)) {
        Real64 SupplyAirTemp = CalcZoneSupplyAirTemp(state, ZoneNum);
        Real64 AirChangeRate = CalcZoneSystemACH(state, ZoneNum);
        return CalcBeausoleilMorrisonMixedUnstableCeiling(DeltaTemp, HydraulicDiameter, SurfTemp, SupplyAirTemp, AirChangeRate);
    } else {
        ErrorObjectHeader eoh{routineName, "Zone", state.dataHeatBal->Zone(ZoneNum).Name};
        if (HydraulicDiameter == 0.0) {
            ShowWarningHydraulicDiameterZero(state, state.dataConvect->BMMixedUnstableCeilingErrorIDX1, eoh);
        }
        if (DeltaTemp == 0.0 && !state.dataGlobal->WarmupFlag) {
            ShowWarningDeltaTempZero(state, state.dataConvect->BMMixedUnstableCeilingErrorIDX2, eoh);
        }
        return 9.999;
    }
}

Real64 CalcFohannoPolidoriVerticalWall(Real64 const DeltaTemp, // [C] temperature difference between surface and air
                                       Real64 const Height,    // [m] characteristic size, height of zone
                                       Real64 const SurfTemp,  // [C] surface temperature
                                       Real64 const QdotConv   // [W/m2] heat flux rate for rayleigh #
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Jul 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate model equation for natural convection

    // REFERENCES:
    // Fohanno, S., and G. Polidori. 2006. Modelling of natural convective heat transfer
    // at an internal surface. Energy and Buildings 38 (2006) 548 - 553

    // FUNCTION PARAMETER DEFINITIONS:
    Real64 constexpr g = 9.81;     // gravity constant (m/s**2)  // Constant::Gravity is 9.807
    Real64 constexpr v = 15.89e-6; // kinematic viscosity (m**2/s) for air at 300 K
    Real64 constexpr k = 0.0263;   // thermal conductivity (W/m K) for air at 300 K
    Real64 constexpr Pr = 0.71;    // Prandtl number for air at ?

    Real64 BetaFilm = 1.0 / (Constant::Kelvin + SurfTemp + 0.5 * DeltaTemp); // TODO check sign on DeltaTemp
    Real64 RaH = (g * BetaFilm * QdotConv * pow_4(Height) * Pr) / (k * pow_2(v));

    if (RaH <= 6.3e09) {
        return 1.332 * std::pow(std::abs(DeltaTemp) / Height, 0.25);
    } else {
        return 1.235 * std::exp(0.0467 * Height) * std::pow(std::abs(DeltaTemp), 0.316);
    }
}

Real64 CallCalcFohannoPolidoriVerticalWall(EnergyPlusData &state,
                                           Real64 const DeltaTemp, // [C] temperature difference between surface and air
                                           Real64 const Height,    // [m] characteristic size, height of zone
                                           Real64 const SurfTemp,  // [C] surface temperature
                                           Real64 const QdotConv,  // [W/m2] heat flux rate for rayleigh #
                                           int const SurfNum       // for messages
)
{
    std::string_view constexpr routineName = "CalcFohannoPolidoriVerticalWall";
    if (Height > 0.0) {
        return CalcFohannoPolidoriVerticalWall(DeltaTemp, Height, SurfTemp, QdotConv);
    } else {
        ErrorObjectHeader eoh{routineName, "Surface", state.dataSurface->Surface(SurfNum).Name};
        // bad value for Height, but we have little info to identify calling culprit
        ShowWarningHydraulicDiameterZero(state, state.dataConvect->CalcFohannoPolidoriVerticalWallErrorIDX, eoh);
        return 9.999;
    }
}

Real64 CalcKaradagChilledCeiling(Real64 const DeltaTemp) // [C] temperature difference between surface and air
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Jul 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate model equation for natural convection developed by Karadag for chilled ceilings

    // REFERENCES:
    // Karadag, R. 2009. New approach relevant to total heat transfer coefficient
    //   including the effect of radiation and convection at the ceiling in a cooled
    //   ceiling room.  Applied Thermal Engineering 29 (2009) 1561-1565
    //    This function is for equation 8 in the reference

    return 3.1 * std::pow(std::abs(DeltaTemp), 0.22);
}

Real64 CalcGoldsteinNovoselacCeilingDiffuserWindow(Real64 const AirSystemFlowRate,        // [m3/s] air system flow rate
                                                   Real64 const ZoneExtPerimLength,       // [m] length of zone perimeter with exterior walls
                                                   Real64 const WindWallRatio,            // [ ] fraction of window area to wall area for zone
                                                   IntConvWinLoc const WindowLocationType // index for location types
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate model equation for windows in zones with slot diffusers on them
    //  developed by Novoselac for RP-1416

    // REFERENCES:
    // Goldstien, K. and A. Novoselac. 2010. Convective Heat Transfer in Rooms
    //  With Ceiling Slot Diffusers (RP-1416). HVAC&R Research Journal TBD

    if (ZoneExtPerimLength > 0.0) {
        if (WindWallRatio <= 0.5) {

            switch (WindowLocationType) {
            case IntConvWinLoc::UpperPartOfExteriorWall:
            case IntConvWinLoc::LargePartOfExteriorWall:
            case IntConvWinLoc::NotSet:
                return 0.117 * std::pow(AirSystemFlowRate / ZoneExtPerimLength, 0.8);
            case IntConvWinLoc::LowerPartOfExteriorWall:
                return 0.093 * std::pow(AirSystemFlowRate / ZoneExtPerimLength, 0.8);
            default:
                assert(false);
                return 9.999;
            }

        } else {
            return 0.103 * std::pow(AirSystemFlowRate / ZoneExtPerimLength, 0.8);
        }
    } else {
        return 9.999;
    }
}

Real64 CalcGoldsteinNovoselacCeilingDiffuserWindow(EnergyPlusData &state,
                                                   Real64 const zoneExtPerimLength, // [m] length of zone perimeter with exterior walls
                                                   Real64 const WindWallRatio,      // [ ] fraction of window area to wall area for zone
                                                   IntConvWinLoc const winLoc,      // index for location types
                                                   int const ZoneNum                // for messages
)
{
    std::string_view constexpr routineName = "CalcGoldsteinNovoselacCeilingDiffuserWindow";
    ErrorObjectHeader eoh{routineName, "Zone", state.dataHeatBal->Zone(ZoneNum).Name};

    Real64 AirSystemFlowRate = CalcZoneSystemVolFlowRate(state, ZoneNum);

    if (zoneExtPerimLength > 0.0) {
        if (WindWallRatio <= 0.5) {
            if (winLoc != IntConvWinLoc::UpperPartOfExteriorWall && winLoc != IntConvWinLoc::LowerPartOfExteriorWall &&
                winLoc != IntConvWinLoc::LargePartOfExteriorWall && winLoc != IntConvWinLoc::NotSet) {
                // Should we not be returning 9.999 here as we do everywhere else?
                ShowWarningWindowLocation(state, state.dataConvect->CalcGoldsteinNovoselacCeilingDiffuserWindowErrorIDX1, eoh, winLoc);
            }
        }
    } else {
        ShowWarningPerimeterLengthZero(state, state.dataConvect->CalcGoldsteinNovoselacCeilingDiffuserWindowErrorIDX2, eoh);
    }
    return CalcGoldsteinNovoselacCeilingDiffuserWindow(AirSystemFlowRate, zoneExtPerimLength, WindWallRatio, winLoc);
}

Real64 CalcGoldsteinNovoselacCeilingDiffuserWall(Real64 const AirSystemFlowRate,        // [m3/s] air system flow rate
                                                 Real64 const ZoneExtPerimLength,       // [m] length of zone perimeter with exterior walls
                                                 IntConvWinLoc const WindowLocationType // index for location types
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate model equation for exterior walls in zones with slot diffusers on them
    //  developed by Novoselac for RP-1416

    // REFERENCES:
    // Goldstien, K. and A. Novoselac. 2010. Convective Heat Transfer in Rooms
    //  With Ceiling Slot Diffusers (RP-1416). HVAC&R Research Journal TBD

    if (ZoneExtPerimLength > 0.0) {

        switch (WindowLocationType) {
        case IntConvWinLoc::WindowAboveThis:
        case IntConvWinLoc::NotSet:
            return 0.063 * std::pow(AirSystemFlowRate / ZoneExtPerimLength, 0.8);
        case IntConvWinLoc::WindowBelowThis:
            return 0.093 * std::pow(AirSystemFlowRate / ZoneExtPerimLength, 0.8);
        default:
            assert(false);
            return 9.999;
        }
    } else {
        return 9.999;
    }
}

Real64 CalcGoldsteinNovoselacCeilingDiffuserWall(EnergyPlusData &state,
                                                 Real64 const zoneExtPerimLength, // [m] length of zone perimeter with exterior walls
                                                 IntConvWinLoc const winLoc,      // index for location types
                                                 int const ZoneNum                // for messages
)
{
    std::string_view constexpr routineName = "CalcGoldsteinNovoselacCeilingDiffuserWall";
    ErrorObjectHeader eoh{routineName, "Zone", state.dataHeatBal->Zone(ZoneNum).Name};
    Real64 AirSystemFlowRate = CalcZoneSystemVolFlowRate(state, ZoneNum);

    // Should we not be returning 9.999 under these conditions?
    if (zoneExtPerimLength > 0.0) {
        if (winLoc != IntConvWinLoc::WindowAboveThis && winLoc != IntConvWinLoc::WindowBelowThis && winLoc != IntConvWinLoc::NotSet) {
            ShowWarningWindowLocation(state, state.dataConvect->CalcGoldsteinNovoselacCeilingDiffuserWallErrorIDX1, eoh, winLoc);
        }
    } else {
        ShowWarningPerimeterLengthZero(state, state.dataConvect->CalcGoldsteinNovoselacCeilingDiffuserWallErrorIDX2, eoh);
    }
    return CalcGoldsteinNovoselacCeilingDiffuserWall(AirSystemFlowRate, zoneExtPerimLength, winLoc);
}

Real64 CalcGoldsteinNovoselacCeilingDiffuserFloor(Real64 const AirSystemFlowRate, // [m3/s] air system flow rate
                                                  Real64 const ZoneExtPerimLength // [m] length of zone perimeter with exterior walls
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate model equation for floors in zones with slot diffusers on them
    //  developed by Novoselac for RP-1416

    // REFERENCES:
    // Goldstien, K. and A. Novoselac. 2010. Convective Heat Transfer in Rooms
    //  With Ceiling Slot Diffusers (RP-1416). HVAC&R Research Journal TBD

    if (ZoneExtPerimLength > 0.0) {
        return 0.048 * std::pow(AirSystemFlowRate / ZoneExtPerimLength, 0.8);
    } else {
        return 9.999; // safe but noticeable
    }
}

Real64 CalcGoldsteinNovoselacCeilingDiffuserFloor(EnergyPlusData &state,
                                                  Real64 const ZoneExtPerimLength, // [m] length of zone perimeter with exterior walls
                                                  int const ZoneNum                // for messages
)
{
    std::string_view constexpr routineName = "CalcGoldsteinNovoselacCeilingDiffuserFloor";
    Real64 AirSystemFlowRate = CalcZoneSystemVolFlowRate(state, ZoneNum);

    if (ZoneExtPerimLength <= 0.0) {
        ErrorObjectHeader eoh{routineName, "Zone", state.dataHeatBal->Zone(ZoneNum).Name};
        ShowWarningPerimeterLengthZero(state, state.dataConvect->CalcGoldsteinNovoselacCeilingDiffuserFloorErrorIDX, eoh);
        // return 9.999?
    }
    return CalcGoldsteinNovoselacCeilingDiffuserFloor(AirSystemFlowRate, ZoneExtPerimLength);
}

Real64 CalcSparrowWindward(Material::SurfaceRoughness const RoughnessIndex, Real64 const FacePerimeter, Real64 const FaceArea, Real64 const WindAtZ)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate Sparrow Hf for windward surfaces

    // REFERENCES:

    //   1. TARP Reference Manual, "Surface Outside Heat Balances", pp 71ff
    //   2. Sparrow, E. M., J. W. Ramsey, and E. A. Mass.  1979.  Effect of finite
    //   width on heat transfer and fluid flow about an inclined rectangular plate.
    //   Journal of Heat Transfer 101:  204.
    //   3. McClellan, T.M.  1996.  Investigation of a heat balance cooling load
    //   procedure with a detailed study of outside heat transfer parameters.
    //   M.S. Thesis, Department of Mechanical and Industrial Engineering,
    //   University of Illinois at Urbana-Champaign.

    return 2.537 * RoughnessMultiplier[(int)RoughnessIndex] * std::sqrt(FacePerimeter * WindAtZ / FaceArea);
}

Real64 CalcSparrowLeeward(Material::SurfaceRoughness const RoughnessIndex, Real64 const FacePerimeter, Real64 const FaceArea, Real64 const WindAtZ)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS FUNCTION:
    // Calculate Sparrow Hf for leeward surfaces

    // REFERENCES:

    //   1. TARP Reference Manual, "Surface Outside Heat Balances", pp 71ff
    //   2. Sparrow, E. M., J. W. Ramsey, and E. A. Mass.  1979.  Effect of finite
    //   width on heat transfer and fluid flow about an inclined rectangular plate.
    //   Journal of Heat Transfer 101:  204.
    //   3. McClellan, T.M.  1996.  Investigation of a heat balance cooling load
    //   procedure with a detailed study of outside heat transfer parameters.
    //   M.S. Thesis, Department of Mechanical and Industrial Engineering,
    //   University of Illinois at Urbana-Champaign.

    return 0.5 * CalcSparrowWindward(RoughnessIndex, FacePerimeter, FaceArea, WindAtZ);
}

Real64 CalcSparrowWindward(EnergyPlusData &state,
                           Material::SurfaceRoughness const RoughnessIndex,
                           Real64 const FacePerimeter,
                           Real64 const FaceArea,
                           Real64 const WindAtZ,
                           int const SurfNum)
{
    std::string_view constexpr routineName = "CalcSparrowWindward";

    if (FaceArea > 0.0) {
        return CalcSparrowWindward(RoughnessIndex, FacePerimeter, FaceArea, WindAtZ);

    } else {
        ErrorObjectHeader eoh{routineName, "Surface", state.dataSurface->Surface(SurfNum).Name};
        ShowWarningFaceAreaZero(state, state.dataConvect->CalcSparrowWindwardErrorIDX, eoh);
        return 9.999; // safe but noticeable
    }
}

Real64 CalcSparrowLeeward(EnergyPlusData &state,
                          Material::SurfaceRoughness const RoughnessIndex,
                          Real64 const FacePerimeter,
                          Real64 const FaceArea,
                          Real64 const WindAtZ,
                          int const SurfNum)
{
    std::string_view constexpr routineName = "CalcSparrowLeeward";

    if (FaceArea > 0.0) {
        return CalcSparrowLeeward(RoughnessIndex, FacePerimeter, FaceArea, WindAtZ);
    } else {
        ErrorObjectHeader eoh{routineName, "Surface", state.dataSurface->Surface(SurfNum).Name};
        ShowWarningFaceAreaZero(state, state.dataConvect->CalcSparrowLeewardErrorIDX, eoh);
        return 9.999; // safe but noticeable
    }
}

Real64 CalcMoWITTNatural(Real64 DeltaTemp)
{
    Real64 constexpr temp_fac(0.84);
    return temp_fac * std::pow(std::abs(DeltaTemp), 1.0 / 3.0);
}

Real64 CalcMoWITTForcedWindward(Real64 const WindAtZ)
{
    Real64 constexpr wind_fac(3.26); // = a, Constant, W/(m2K(m/s)^b)
    Real64 constexpr wind_exp(0.89); // = b
    return wind_fac * std::pow(WindAtZ, wind_exp);
}

Real64 CalcMoWITTForcedLeeward(Real64 const WindAtZ)
{
    Real64 constexpr wind_fac(3.55);  // = a, Constant, W/(m2K(m/s)^b)
    Real64 constexpr wind_exp(0.617); // = b
    return wind_fac * std::pow(WindAtZ, wind_exp);
}

Real64 CalcMoWITTWindward(Real64 const DeltaTemp, Real64 const WindAtZ)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS FUNCTION:
    // calculate MoWITT Hc equation for windward surfaces

    // REFERENCES:
    //   Yazdanian, M. and J.H. Klems.  1994.  Measurement of the exterior convective
    //   film coefficient for windows in low-rise buildings.
    //   ASHRAE Transactions 100(1):  1087.

    Real64 Hn = CalcMoWITTNatural(DeltaTemp);
    Real64 Hf = CalcMoWITTForcedWindward(WindAtZ);
    return std::sqrt(pow_2(Hn) + pow_2(Hf));
}

Real64 CalcMoWITTLeeward(Real64 const DeltaTemp, Real64 const WindAtZ)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS FUNCTION:
    // calculate MoWITT Hc equation for leeward surfaces

    // REFERENCES:
    //   Yazdanian, M. and J.H. Klems.  1994.  Measurement of the exterior convective
    //   film coefficient for windows in low-rise buildings.
    //   ASHRAE Transactions 100(1):  1087.

    Real64 Hn = CalcMoWITTNatural(DeltaTemp);
    Real64 Hf = CalcMoWITTForcedLeeward(WindAtZ);
    return std::sqrt(pow_2(Hn) + pow_2(Hf));
}

Real64 CalcDOE2Forced(
    Real64 const SurfaceTemp, Real64 const AirTemp, Real64 const CosineTilt, Real64 const HfSmooth, Material::SurfaceRoughness const RoughnessIndex)
{
    // This allows costly HfSmooth to be calculated independently (avoids excessive use of std::pow() in Kiva)
    Real64 Hn = CalcASHRAETARPNatural(SurfaceTemp, AirTemp, CosineTilt);
    Real64 HcSmooth = std::sqrt(pow_2(Hn) + pow_2(HfSmooth));
    return RoughnessMultiplier[(int)RoughnessIndex] * (HcSmooth - Hn);
}

Real64 CalcDOE2Windward(
    Real64 const SurfaceTemp, Real64 const AirTemp, Real64 const CosineTilt, Real64 const WindAtZ, Material::SurfaceRoughness const RoughnessIndex)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // calculate DOE-2 Hf equation for windward surfaces

    // REFERENCES:
    //   Lawrence Berkeley Laboratory.  1994.  DOE2.1E-053 source code.
    //   Yazdanian, M. and J.H. Klems.  1994.  Measurement of the exterior convective
    //   film coefficient for windows in low-rise buildings.
    //   ASHRAE Transactions 100(1):  1087.
    Real64 HfSmooth = CalcMoWITTForcedWindward(WindAtZ);

    return CalcDOE2Forced(SurfaceTemp, AirTemp, CosineTilt, HfSmooth, RoughnessIndex);
}

Real64 CalcDOE2Leeward(
    Real64 const SurfaceTemp, Real64 const AirTemp, Real64 const CosineTilt, Real64 const WindAtZ, Material::SurfaceRoughness const RoughnessIndex)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS FUNCTION:
    // calculate DOE-2 Hf equation for leeward surfaces

    // REFERENCES:
    //   Lawrence Berkeley Laboratory.  1994.  DOE2.1E-053 source code.
    //   Yazdanian, M. and J.H. Klems.  1994.  Measurement of the exterior convective
    //   film coefficient for windows in low-rise buildings.
    //   ASHRAE Transactions 100(1):  1087.

    Real64 HfSmooth = CalcMoWITTForcedLeeward(WindAtZ);

    return CalcDOE2Forced(SurfaceTemp, AirTemp, CosineTilt, HfSmooth, RoughnessIndex);
}

Real64 CalcNusseltJurges(Real64 const WindAtZ)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS FUNCTION:
    // calculate model equation for forced convection using Nusselt Jurges correlation
    // model is attributed to Nusselt and Jurges but the equation is recast in current units
    // by Palyvos

    // REFERENCES:
    // 1. Nusselt, W., W. Jurges. 1922. Die Kuhlung einer ebenen Wand durch einen Luftstrom
    //     (The cooling of a plane wall by an air flow). Gesundheits Ingenieur 52, Heft, 45, Jargang.
    // 2. Palyvos, J.A., 2008. A survey of wind convection coefficient correlations for building
    //     envelope energy systems' modeling. Applied Thermal Engineering 28 (2008) 801-808. Elsevier.

    return 5.8 + 3.94 * WindAtZ;
}

Real64 CalcMcAdams(Real64 const WindAtZ)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS FUNCTION:
    // calculate model equation for forced convection using McAdams correlation
    // model is attributed to McAdams but the equation is as recast in current units
    // by Palyvos

    // REFERENCES:
    // 1. McAdams, W.H., 1954. Heat Transmission, third ed., McGraw-Hill, New York.
    // 2. Palyvos, J.A., 2008. A survey of wind convection coefficient correlations for building
    //     envelope energy systems' modeling. Applied Thermal Engineering 28 (2008) 801-808. Elsevier.

    return 5.8 + 3.8 * WindAtZ;
}

Real64 CalcMitchell(Real64 const WindAtZ, Real64 const LengthScale)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS FUNCTION:
    // calculate model equation for forced convection using Mitchell correlation
    // model is attributed to Mitchell but the equation is as recast in current units
    // by Palyvos

    // REFERENCES:
    // 1. Mitchell, J.W., 1976. Heat transfer from spheres and other animal forms. Biophy. J. 16 (1976) 561
    // 2. Palyvos, J.A., 2008. A survey of wind convection coefficient correlations for building
    //     envelope energy systems' modeling. Applied Thermal Engineering 28 (2008) 801-808. Elsevier.

    return 8.6 * std::pow(WindAtZ, 0.6) / std::pow(LengthScale, 0.4);
}

Real64 CalcMitchell(EnergyPlusData &state, Real64 const WindAtZ, Real64 const LengthScale, int const SurfNum)
{
    if (LengthScale > 0.0) {
        return CalcMitchell(WindAtZ, LengthScale);
    } else {
        if (state.dataConvect->CalcMitchellErrorIDX == 0) {
            ShowSevereMessage(state, "CalcMitchell: Convection model not evaluated (bad length scale)");
            ShowContinueError(state, format("Value for effective length scale = {:.5R}", LengthScale));
            ShowContinueError(state, format("Occurs for surface named = {}", state.dataSurface->Surface(SurfNum).Name));
            ShowContinueError(state, "Convection surface heat transfer coefficient set to 9.999 [W/m2-K] and the simulation continues");
        }
        ShowRecurringSevereErrorAtEnd(state,
                                      "CalcMitchell: Convection model not evaluated because bad length scale and set to 9.999 [W/m2-k]",
                                      state.dataConvect->CalcMitchellErrorIDX);
        return 9.999; // safe but noticeable
    }
}

Real64 CalcWindSurfaceTheta(Real64 const WindDir, Real64 const SurfAzimuth)
{
    // Computes the angle theta between the wind direction and the surface azimuth
    // Should always be a value between 0-180 deg

    Real64 windDir = std::fmod(WindDir, 360);
    Real64 surfAzi = std::fmod(SurfAzimuth, 360);
    Real64 theta = std::abs(windDir - surfAzi);
    if (theta > 180) {
        return abs(theta - 360);
    } else {
        return theta;
    }
}

Real64 CalcBlockenWindward(EnergyPlusData &state,
                           Real64 const WindAt10m,
                           Real64 const WindDir,     // Wind direction measured clockwise from geographic North
                           Real64 const SurfAzimuth, // or Facing, Direction the surface outward normal faces (degrees)
                           int const SurfNum)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS FUNCTION:
    // calculate model equation for forced convection using Blocken correlation

    // REFERENCES:
    // Blocken, B., T. Defraeye, D. Derome, J. Carmeliet. 2009.
    //  High-Resolution CFD Simulations for Forced Convection
    //   Heat Transfer Coefficients at the Facade of a Low-Rise Building.
    //   Building and Environment 44 (2009) 2396 - 2412.

    Real64 Theta = CalcWindSurfaceTheta(WindDir, SurfAzimuth); // angle between wind and surface azimuth

    if (Theta <= 11.25) {
        return 4.6 * std::pow(WindAt10m, 0.89);
    } else if (Theta <= 33.75) {
        return 5.0 * std::pow(WindAt10m, 0.8);
    } else if (Theta <= 56.25) {
        return 4.6 * std::pow(WindAt10m, 0.84);
    } else if (Theta <= 100.0) {
        return 4.5 * std::pow(WindAt10m, 0.81);
    } else {
        if (state.dataConvect->CalcBlockenWindwardErrorIDX == 0) {
            ShowSevereMessage(state, "CalcBlockenWindward: Convection model wind angle calculation suspect (developer issue)");
            ShowContinueError(state, format("Value for theta angle = {:.5R}", Theta));
            ShowContinueError(state, format("Occurs for surface named = {}", state.dataSurface->Surface(SurfNum).Name));
            ShowContinueError(state, "Convection model uses EmmelVertical correlation and the simulation continues");
        }
        ShowRecurringSevereErrorAtEnd(
            state, "CalcBlockenWindward: Convection model wind angle calculation suspect.", state.dataConvect->CalcBlockenWindwardErrorIDX);
        return CalcEmmelVertical(WindAt10m, WindDir, SurfAzimuth);
    }
}

Real64 CalcEmmelVertical(Real64 const WindAt10m,
                         Real64 const WindDir,     // Wind direction measured clockwise from geographic North
                         Real64 const SurfAzimuth) // or Facing, Direction the surface outward normal faces (degrees)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS FUNCTION:
    // calculate model equation for forced convection using Emmel correlation
    // for vertical walls

    // REFERENCES:
    // Emmel, M.G., M.O. Abadie, N. Mendes. 2007. New external convective
    //   heat transfer coefficient correlations for isolated low-rise buildings.
    //   Energy and Buildings 39 (2007) 335- 342

    Real64 Theta = CalcWindSurfaceTheta(WindDir, SurfAzimuth); // angle between wind and surface azimuth

    if (Theta <= 22.5) {
        return 5.15 * std::pow(WindAt10m, 0.81);
    } else if (Theta <= 67.5) {
        return 3.34 * std::pow(WindAt10m, 0.84);
    } else if (Theta <= 112.5) {
        return 4.78 * std::pow(WindAt10m, 0.71);
    } else if (Theta <= 157.5) {
        return 4.05 * std::pow(WindAt10m, 0.77);
    } else {
        return 3.54 * std::pow(WindAt10m, 0.76);
    }
}

Real64 CalcEmmelRoof(Real64 const WindAt10m,
                     Real64 const WindDir,                // Wind direction measured clockwise from geographic North
                     Real64 const LongAxisOutwardAzimuth) // or Facing, Direction the surface outward normal faces (degrees)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Brent Griffith
    //       DATE WRITTEN   Aug 2010

    // PURPOSE OF THIS FUNCTION:
    // calculate model equation for forced convection using Emmel correlation
    // for horizontal roofs

    // REFERENCES:
    // Emmel, M.G., M.O. Abadie, N. Mendes. 2007. New external convective
    //   heat transfer coefficient correlations for isolated low-rise buildings.
    //    Energy and Buildings 39 (2007) 335- 342

    Real64 Theta = CalcWindSurfaceTheta(WindDir, LongAxisOutwardAzimuth); // angle between wind and surface azimuth

    if (Theta <= 22.5) {
        return 5.11 * std::pow(WindAt10m, 0.78);
    } else if (Theta <= 67.5) {
        return 4.60 * std::pow(WindAt10m, 0.79);
    } else if (Theta <= 112.5) {
        return 3.67 * std::pow(WindAt10m, 0.85);
    } else if (Theta <= 157.5) {
        return 4.60 * std::pow(WindAt10m, 0.79);
    } else {
        return 5.11 * std::pow(WindAt10m, 0.78);
    }
}

Real64 CalcClearRoof(EnergyPlusData &state,
                     Real64 const SurfTemp,
                     Real64 const AirTemp,
                     Real64 const WindAtZ,
                     Real64 const RoofArea,
                     Real64 const RoofPerimeter,
                     Material::SurfaceRoughness const RoughnessIndex)
{

    // FUNCTION PARAMETER DEFINITIONS:
    Real64 constexpr g = 9.81;     // gravity constant (m/s**2)
    Real64 constexpr v = 15.89e-6; // kinematic viscosity (m**2/s) for air at 300 K
    Real64 constexpr k = 0.0263;   // thermal conductivity (W/m K) for air at 300 K
    Real64 constexpr Pr = 0.71;    // Prandtl number for air at ?

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    Real64 eta;

    // find x, don't know x. avoid time consuming geometry algorithm
    Real64 x = std::sqrt(RoofArea) / 2.0; // quick simplification, geometry routines to develop

    Real64 Ln = (RoofPerimeter > 0.0) ? (RoofArea / RoofPerimeter) : std::sqrt(RoofArea);
    Real64 DeltaTemp = SurfTemp - AirTemp;
    Real64 BetaFilm = 1.0 / (Constant::Kelvin + SurfTemp + 0.5 * DeltaTemp);
    Real64 AirDensity = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, AirTemp, state.dataEnvrn->OutHumRat);

    Real64 GrLn = g * pow_2(AirDensity) * pow_3(Ln) * std::abs(DeltaTemp) * BetaFilm / pow_2(v);
    Real64 RaLn = GrLn * Pr;

    Real64 Rex = WindAtZ * AirDensity * x / v;

    Real64 Rf = RoughnessMultiplier[(int)RoughnessIndex];
    if (Rex > 0.1) { // avoid zero and crazy small denominators
        Real64 tmp = std::log1p(GrLn / pow_2(Rex));
        eta = tmp / (1.0 + tmp);
    } else {
        eta = 1.0; // forced convection gone because no wind
    }

    return eta * (k / Ln) * 0.15 * std::pow(RaLn, 1.0 / 3.0) + (k / x) * Rf * 0.0296 * std::pow(Rex, 0.8) * std::pow(Pr, 1.0 / 3.0);
}

Real64 CalcClearRoof(EnergyPlusData &state,
                     int const SurfNum,
                     Real64 const SurfTemp,
                     Real64 const AirTemp,
                     Real64 const WindAtZ,
                     [[maybe_unused]] Real64 const WindDirect, // Wind direction measured clockwise from geographic North
                     Real64 const RoofArea,
                     Real64 const RoofPerimeter)
{
    Material::SurfaceRoughness const RoughnessIndex =
        state.dataMaterial->materials(state.dataConstruction->Construct(state.dataSurface->Surface(SurfNum).Construction).LayerPoint(1))->Roughness;
    // find x, don't know x. avoid time consuming geometry algorithm
    Real64 x = std::sqrt(RoofArea) / 2.0; // quick simplification, geometry routines to develop

    if (x > 0.0) {
        return CalcClearRoof(state, SurfTemp, AirTemp, WindAtZ, RoofArea, RoofPerimeter, RoughnessIndex);
    } else {
        if (state.dataSurface->Surface(SurfNum).ExtBoundCond != DataSurfaces::OtherSideCondModeledExt) {
            if (state.dataConvect->CalcClearRoofErrorIDX == 0) {
                ShowSevereMessage(state, "CalcClearRoof: Convection model not evaluated (bad value for distance to roof edge)");
                ShowContinueError(state, format("Value for distance to roof edge ={:.3R}", x));
                ShowContinueError(state, format("Occurs for surface named = {}", state.dataSurface->Surface(SurfNum).Name));
                ShowContinueError(state, "Convection surface heat transfer coefficient set to 9.999 [W/m2-K] and the simulation continues");
            }
            ShowRecurringSevereErrorAtEnd(
                state,
                "CalcClearRoof: Convection model not evaluated because bad value for distance to roof edge and set to 9.999 [W/m2-k]",
                state.dataConvect->CalcClearRoofErrorIDX);
        }
        return 9.9999; // safe but noticeable
    }
}

void CalcASTMC1340ConvCoeff(EnergyPlusData &state,
                            int const SurfNum,                  // surface number for which coefficients are being calculated
                            Real64 const SurfaceTemperature,    // Temperature of surface for evaluation of HcIn
                            Real64 const ZoneMeanAirTemperature // Mean Air Temperature of Zone
)
{
    auto const &surface = state.dataSurface->Surface(SurfNum);

    int ZoneNum = surface.Zone;
    Real64 Volume = state.dataHeatBal->Zone(ZoneNum).Volume; // Volume of the zone in m3
    Real64 Vair = std::pow(Volume, 1.0 / 3.0) * CalcZoneSystemACH(state, ZoneNum) / 3600;

    state.dataHeatBalSurf->SurfHConvInt(SurfNum) =
        CalcASTMC1340ConvCoeff(state, SurfNum, SurfaceTemperature, ZoneMeanAirTemperature, Vair, surface.Tilt);

    // Establish some lower limit to avoid a zero convection coefficient (and potential divide by zero problems)
    if (state.dataHeatBalSurf->SurfHConvInt(SurfNum) < state.dataHeatBal->LowHConvLimit) {
        state.dataHeatBalSurf->SurfHConvInt(SurfNum) = state.dataHeatBal->LowHConvLimit;
    }
}

Real64 CalcASTMC1340ConvCoeff(EnergyPlusData &state, int const SurfNum, Real64 const Tsurf, Real64 const Tair, Real64 const Vair, Real64 const Tilt)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Dareum Nam
    //       DATE WRITTEN   Feb 2021

    // PURPOSE OF THIS FUNCTION:
    // Calculate the inside convection coefficient for attic zones containing radiant barriers

    // REFERENCES:
    // 1. ASTM C1340: Standard Practice for Estimation of Heat Gain or Loss Through Ceilings Under Attics
    // Containing Radiant Barriers by Use of a Computer Program
    // 2. Fontanini, A. D., Aguilar, J. L. C., Mitchell, M. S., Kosny, J., Merket, N., DeGraw, J. W., & Lee, E. (2018).
    // Predicting the performance of radiant technologies in attics: Reducing the discrepancies between attic specific
    // and whole-building energy models. Energy and Buildings, 169, 69-83.

    Real64 Nun; // Nusselt number for natural convection
    Real64 Nuf; // Nusselt number for forced convection
    Real64 Grc; // Critical Grashof number

    constexpr Real64 g = Constant::Gravity; // Acceleration of gravity, m/s2

    auto const &surface = state.dataSurface->Surface(SurfNum);

    // Characteristic length: the length along the heat flow direction
    // (the square root of surface area for floors and ceilings,
    // average height for gables and walls, and length of pitched roof
    // from soffit to ridge) Surface Outside Face Outdoor Air Wind
    // Speed (for exterior surfaces)
    Real64 L = (Tilt == 0 || Tilt == 180) ? std::sqrt(surface.Area) : surface.Height;
    // The velocity of the air stream in m/s, (for interior surfaces)
    Real64 v = (surface.ExtBoundCond == 0) ? state.dataSurface->SurfOutWindSpeed(SurfNum) : Vair;
    // Prandtl number
    Real64 Pr = 0.7880 - (2.631 * std::pow(10, -4) * (Tair + 273.15));
    // Volume coefficient of expansion of air, 1/K
    Real64 beta_SI = 1 / (Tair + 273.15);
    // Density of air, kg/m3
    Real64 rho_SI = (22.0493 / (Tair + 273.15)) * 16;
    // Specific heat of air, J/kg.k
    Real64 cp_SI = 0.068559 * (3.4763 + (1.066 * std::pow(10, -4) * (Tair + 273.15))) * 4186.8;
    Real64 dv = (241.9 * std::pow(10, -7)) * (145.8 * (Tair + 273.15) * std::pow((Tair + 273.15), 0.5)) / ((Tair + 273.15) + 110.4);
    // Kinematic viscosity of air, m2/s
    Real64 visc = dv * (0.45359237 / (0.3048 * 3600)) / rho_SI;
    Real64 k_SI_n = (0.6325 * std::pow(10, -5) * std::pow((Tair + 273.15), 0.5) * 241.77);
    Real64 k_SI_d = (1.0 + (245.4 * std::pow(10, (-12 / (Tair + 273.15)))) / (Tair + 273.15));
    // Thermal conductivity of air, W/m.K
    Real64 k_SI = 1.730735 * (k_SI_n / k_SI_d);

    // Temperature difference between TSurf and Tair
    Real64 DeltaTemp = Tsurf - Tair;

    // Rayleigh number
    Real64 Ra = std::abs(g * beta_SI * rho_SI * cp_SI * DeltaTemp * (L * L * L)) / (visc * k_SI);
    // Reynolds number
    Real64 Re = (v * L) / visc;

    // Natural convection (Nun)
    if (Tilt == 0) {         // Horizontal surface: Roof
        if (DeltaTemp > 0) { // heat flow down
            Nun = 0.58 * std::pow(Ra, 0.2);
        } else if (Ra < 8000000) { // heat flow up
            Nun = 0.54 * std::pow(Ra, 0.25);
        } else {
            Nun = 0.15 * std::pow(Ra, 1.0 / 3.0);
        }
    } else if (Tilt > 0 && Tilt < 90) { // Tilted roof
        if (DeltaTemp > 0) {            // heat flow down
            if (Tilt < 2) {
                Nun = 0.58 * std::pow(Ra, 0.2);
            } else {
                Nun = 0.56 * std::pow(Ra * (std::sin(Tilt * Constant::DegToRad)), 0.25);
            }
        } else { // heat flow up
            if (Tilt < 15) {
                Grc = 1000000;
            } else if (Tilt <= 75.0) {
                Grc = std::pow(10, Tilt / (1.1870 + (0.0870 * Tilt)));
            } else {
                Grc = 5000000000;
            }
            if ((Ra / Pr) <= Grc) {
                Nun = 0.56 * std::pow(Ra * (std::sin(Tilt * 3.14159 / 180)), 0.25);
            } else {
                Nun = 0.14 * (std::pow(Ra, Constant::OneThird) - std::pow(Grc * Pr, Constant::OneThird)) +
                      0.56 * std::pow(Grc * Pr * (std::sin(Tilt * Constant::DegToRad)), 0.25);
            }
        }
    } else if (Tilt == 180) { // Horizontal surface: Floor
        if (DeltaTemp <= 0) { // heat flow down
            Nun = 0.58 * std::pow(Ra, 0.2);
        } else if (Ra < 8000000) { // heat flow up
            Nun = 0.54 * std::pow(Ra, 0.25);
        } else {
            Nun = 0.15 * std::pow(Ra, 1.0 / 3.0);
        }
    } else if (Tilt > 90 && Tilt < 180) { // Tilted Floor
        if (DeltaTemp <= 0) {             // heat flow down
            if (Tilt > 178) {
                Nun = 0.58 * std::pow(Ra, 0.2);
            } else {
                Nun = 0.56 * std::pow(Ra * (std::sin(Tilt * Constant::DegToRad)), 0.25);
            }
        } else { // heat flow up
            if (Tilt > 165) {
                Grc = 1000000;
            } else if (Tilt <= 105.0) {
                Grc = std::pow(10, Tilt / (1.1870 + (0.0870 * Tilt)));
            } else {
                Grc = 5000000000;
            }
            if ((Ra / Pr) <= Grc) {
                Nun = 0.56 * std::pow(Ra * (std::sin(Tilt * Constant::DegToRad)), 0.25);
            } else {
                Nun = 0.14 * (std::pow(Ra, Constant::OneThird) - std::pow(Grc * Pr, Constant::OneThird)) +
                      0.56 * std::pow(Grc * Pr * (std::sin(Tilt * Constant::DegToRad)), 0.25);
            }
        }
    } else { // Vertical wall (Tilt = 90)
        if (Ra < 1000000000) {
            Nun = 0.59 * std::pow(Ra, 0.25);
        } else {
            Nun = 0.10 * std::pow(Ra, 1.0 / 3.0);
        }
    }

    // Forced convection (Nuf)
    if (Re < 500000) {
        Nuf = 0.664 * std::pow(Pr, 1.0 / 3.0) * std::pow(Re, 0.5);
    } else {
        Nuf = std::pow(Pr, 1.0 / 3.0) * ((0.037 * std::pow(Re, 0.8)) - 850);
    }

    // Combined convection coefficient
    Real64 hf = Nuf * k_SI / L;
    Real64 hn = Nun * k_SI / L;
    return std::pow((std::pow(hf, 3) + std::pow(hn, 3)), 1.0 / 3.0);
}

SurfOrientation GetSurfConvOrientation(Real64 const Tilt)
{
    if (Tilt < 5.0) {
        return SurfOrientation::HorizontalDown;
    } else if ((Tilt >= 5.0) && (Tilt < 85.0)) {
        return SurfOrientation::TiltedDownward;
    } else if ((Tilt >= 85.0) && (Tilt < 95.0)) {
        return SurfOrientation::Vertical;
    } else if ((Tilt >= 95.0) && (Tilt < 175.0)) {
        return SurfOrientation::TiltedUpward;
    } else if (Tilt >= 175.0) {
        return SurfOrientation::HorizontalUp;
    } else {
        return SurfOrientation::Invalid;
    }
}

Real64 SurroundingSurfacesRadCoeffAverage(EnergyPlusData &state, int const SurfNum, Real64 const TSurfK, Real64 const AbsExt)
{
    // compute exterior surfaces LW radiation transfer coefficient to surrounding surfaces
    // the surface.SrdSurfTemp is weighed by surrounding surfaces view factor
    Real64 HSrdSurf = 0.0;
    auto const &surface = state.dataSurface->Surface(SurfNum);
    Real64 SrdSurfsTK = surface.SrdSurfTemp + Constant::Kelvin;
    if (TSurfK != SrdSurfsTK) {
        HSrdSurf = Constant::StefanBoltzmann * AbsExt * surface.ViewFactorSrdSurfs * (pow_4(TSurfK) - pow_4(SrdSurfsTK)) / (TSurfK - SrdSurfsTK);
    }
    return HSrdSurf;
}

} // namespace EnergyPlus::Convect

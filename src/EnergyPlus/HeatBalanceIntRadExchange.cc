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
#include <cassert>
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/ArrayS.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/DataViewFactorInformation.hh>
#include <EnergyPlus/DisplayRoutines.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/HeatBalanceIntRadExchange.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/Material.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WindowEquivalentLayer.hh>

namespace EnergyPlus {

namespace HeatBalanceIntRadExchange {
    // Module containing the routines dealing with the interior radiant exchange
    // between surfaces.

    // MODULE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   September 2000
    //       MODIFIED       Aug 2001, FW: recalculate ScriptF for a zone if window interior
    //                       shade/blind status is different from previous time step. This is
    //                       because ScriptF, which is used to calculate interior LW
    //                       exchange between surfaces, depends on inside surface emissivities,
    //                       which, for a window, depends on whether or not an interior
    //                       shade or blind is in place.

    // PURPOSE OF THIS MODULE:
    // Part of the heat balance modularization/re-engineering.  Purpose of this
    // module is to replace the MRT with RBAL method of modeling radiant exchange
    // between interior surfaces.

    // METHODOLOGY EMPLOYED:
    // Standard EnergyPlus methodology

    // REFERENCES:
    // ASHRAE Loads Toolkit "Script F" routines by Curt Pedersen
    // Hottel, H.C., and A.F. Sarofim. "Radiative Transfer" (mainly chapter 3),
    //  McGraw-Hill, Inc., New York, 1967.

    void CalcInteriorRadExchange(EnergyPlusData &state,
                                 Array1S<Real64> const SurfaceTemp,              // Current surface temperatures
                                 int const SurfIterations,                       // Number of iterations in calling subroutine
                                 Array1D<Real64> &NetLWRadToSurf,                // Net long wavelength radiant exchange from other surfaces
                                 ObjexxFCL::Optional_int_const ZoneToResimulate, // if passed in, then only calculate for this zone
#ifdef EP_Count_Calls
                                 std::string_view const CalledFrom)
#else
                                 [[maybe_unused]] std::string_view const CalledFrom)
#endif
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   September 2000
        //       MODIFIED       6/18/01, FCW: calculate IR on windows
        //                      Jan 2002, FCW: add blinds with movable slats
        //                      Sep 2011 LKL/BG - resimulate only zones needing it for Radiant systems

        // PURPOSE OF THIS SUBROUTINE:
        // Determines the interior radiant exchange between surfaces using
        // Hottel's ScriptF method for the grey interchange between surfaces
        // in an enclosure.

        // REFERENCES:
        // Hottel, H. C. and A. F. Sarofim, Radiative Transfer, Ch 3, McGraw Hill, 1967.

        // Types
        typedef Array1D<Real64>::size_type size_type;

        bool IntShadeOrBlindStatusChanged; // True if status of interior shade or blind on at least
        // one window in a zone has changed from previous time step

        auto &SurfaceTempRad = state.dataHeatBalIntRadExchg->SurfaceTempRad;
        auto &SurfaceTempInKto4th = state.dataHeatBalIntRadExchg->SurfaceTempInKto4th;
        auto &SurfaceEmiss = state.dataHeatBalIntRadExchg->SurfaceEmiss;

        if (state.dataHeatBalIntRadExchg->CalcInteriorRadExchangefirstTime) {
            SurfaceTempRad.allocate(state.dataHeatBalIntRadExchg->MaxNumOfRadEnclosureSurfs);
            SurfaceTempInKto4th.allocate(state.dataHeatBalIntRadExchg->MaxNumOfRadEnclosureSurfs);
            SurfaceEmiss.allocate(state.dataHeatBalIntRadExchg->MaxNumOfRadEnclosureSurfs);
            state.dataHeatBalIntRadExchg->CalcInteriorRadExchangefirstTime = false;
            if (state.dataSysVars->DeveloperFlag) {
                DisplayString(state, " OMP turned off, HBIRE loop executed in serial");
            }
        }

        if (state.dataGlobal->KickOffSimulation || state.dataGlobal->KickOffSizing) {
            return;
        }

        bool const PartialResimulate(present(ZoneToResimulate));

#ifdef EP_Count_Calls
        if (!PartialResimulate) {
            ++state.dataTimingsData->NumIntRadExchange_Calls;
        } else {
            ++state.dataTimingsData->NumIntRadExchangeZ_Calls;
        }
        if (CalledFrom.empty()) {
            // do nothing
        } else if (CalledFrom == "Main") {
            ++state.dataTimingsData->NumIntRadExchangeMain_Calls;
        } else if (CalledFrom == "Outside") {
            ++state.dataTimingsData->NumIntRadExchangeOSurf_Calls;
        } else if (CalledFrom == "Inside") {
            ++state.dataTimingsData->NumIntRadExchangeISurf_Calls;
        }
#endif

        int startEnclosure = 1;
        int endEnclosure = state.dataViewFactor->NumOfRadiantEnclosures;
        if (PartialResimulate) {
            // ToDo: For now, use min and max enclosure numbers associated with this zone, this could include unrelated enclosures
            startEnclosure = state.dataHeatBal->Zone(ZoneToResimulate).zoneRadEnclosureFirst;
            endEnclosure = state.dataHeatBal->Zone(ZoneToResimulate).zoneRadEnclosureLast;
            for (int enclosureNum = startEnclosure; enclosureNum <= endEnclosure; ++enclosureNum) {
                auto const &enclosure = state.dataViewFactor->EnclRadInfo(enclosureNum);
                for (int i : enclosure.SurfacePtr) {
                    NetLWRadToSurf(i) = 0.0;
                    state.dataSurface->SurfWinIRfromParentZone(i) = 0.0;
                }
            }
        } else {
            NetLWRadToSurf = 0.0;
            for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; SurfNum++) {
                state.dataSurface->SurfWinIRfromParentZone(SurfNum) = 0.0;
            }
        }

        for (int enclosureNum = startEnclosure; enclosureNum <= endEnclosure; ++enclosureNum) {

            auto &zone_info = state.dataViewFactor->EnclRadInfo(enclosureNum);
            auto &zone_ScriptF = zone_info.ScriptF; // Tuned Transposed
            int const n_zone_Surfaces = zone_info.NumOfSurfaces;
            size_type const s_zone_Surfaces = n_zone_Surfaces;

            // Calculate ScriptF if first time step in environment and surface heat-balance iterations not yet started;
            // recalculate ScriptF if status of window interior shades or blinds has changed from
            // previous time step. This recalculation is required since ScriptF depends on the inside
            // emissivity of the inside surfaces, which, for windows, is (1) the emissivity of the
            // inside face of the inside glass layer if there is no interior shade/blind, or (2) the effective
            // emissivity of the shade/blind if the shade/blind is in place. (The "effective emissivity"
            // in this case is (1) the shade/blind emissivity if the shade/blind IR transmittance is zero,
            // or (2) a weighted average of the shade/blind emissivity and inside glass emissivity if the
            // shade/blind IR transmittance is not zero (which is sometimes the case for a "shade" and
            // usually the case for a blind). It assumed for switchable glazing that the inside surface
            // emissivity does not change if the glazing is switched on or off.

            // Determine if status of interior shade/blind on one or more windows in the zone has changed
            // from previous time step.  Also make a check for any changes in interior movable insulation.

            if (SurfIterations == 0) {

                bool IntMovInsulChanged; // True if the status of interior movable insulation has changed

                IntShadeOrBlindStatusChanged = false;
                IntMovInsulChanged = false;

                if (!state.dataGlobal->BeginEnvrnFlag) { // Check for change in shade/blind status
                    for (int const SurfNum : zone_info.SurfacePtr) {
                        if (IntShadeOrBlindStatusChanged || IntMovInsulChanged) {
                            break; // Need only check if one window's status or one movable insulation status has changed
                        }
                        if (state.dataConstruction->Construct(state.dataSurface->Surface(SurfNum).Construction).TypeIsWindow) {
                            DataSurfaces::WinShadingType ShadeFlag = state.dataSurface->SurfWinShadingFlag(SurfNum);
                            DataSurfaces::WinShadingType ShadeFlagPrev = state.dataSurface->SurfWinExtIntShadePrevTS(SurfNum);
                            if (ShadeFlagPrev != ShadeFlag && (ANY_INTERIOR_SHADE_BLIND(ShadeFlagPrev) || ANY_INTERIOR_SHADE_BLIND(ShadeFlag))) {
                                IntShadeOrBlindStatusChanged = true;
                            }
                            if (state.dataSurface->SurfWinWindowModelType(SurfNum) == DataSurfaces::WindowModel::EQL &&
                                state.dataWindowEquivLayer
                                    ->CFS(state.dataConstruction->Construct(state.dataSurface->Surface(SurfNum).Construction).EQLConsPtr)
                                    .ISControlled) {
                                IntShadeOrBlindStatusChanged = true;
                            }
                        } else {
                            if (state.dataSurface->AnyMovableInsulation) {
                                UpdateMovableInsulationFlag(state, IntMovInsulChanged, SurfNum);
                            }
                        }
                    }
                }

                if (IntShadeOrBlindStatusChanged || IntMovInsulChanged ||
                    state.dataGlobal->BeginEnvrnFlag) { // Calc inside surface emissivities for this time step
                    for (int ZoneSurfNum = 1; ZoneSurfNum <= n_zone_Surfaces; ++ZoneSurfNum) {
                        int const SurfNum = zone_info.SurfacePtr(ZoneSurfNum);
                        int const ConstrNum = state.dataSurface->Surface(SurfNum).Construction;
                        zone_info.Emissivity(ZoneSurfNum) = state.dataHeatBalSurf->SurfAbsThermalInt(SurfNum);
                        if (state.dataConstruction->Construct(ConstrNum).TypeIsWindow &&
                            ANY_INTERIOR_SHADE_BLIND(state.dataSurface->SurfWinShadingFlag(SurfNum))) {
                            zone_info.Emissivity(ZoneSurfNum) = state.dataHeatBalSurf->SurfAbsThermalInt(SurfNum);
                        }
                        if (state.dataSurface->SurfWinWindowModelType(SurfNum) == DataSurfaces::WindowModel::EQL &&
                            state.dataWindowEquivLayer->CFS(state.dataConstruction->Construct(ConstrNum).EQLConsPtr).ISControlled) {
                            zone_info.Emissivity(ZoneSurfNum) = WindowEquivalentLayer::EQLWindowInsideEffectiveEmiss(state, ConstrNum);
                        }
                    }

                    if (state.dataHeatBalIntRadExchg->CarrollMethod) {
                        CalcFp(n_zone_Surfaces, zone_info.Emissivity, zone_info.FMRT, zone_info.Fp);
                    } else {
                        CalcScriptF(state, n_zone_Surfaces, zone_info.Area, zone_info.F, zone_info.Emissivity, zone_ScriptF);
                        // precalc - multiply by StefanBoltzmannConstant
                        zone_ScriptF *= Constant::StefanBoltzmann;
                    }
                }

            } // End of check if SurfIterations = 0

            // Set surface emissivities and temperatures
            // Also, for Carroll method, calculate numerators and denominators of radiant temperature
            Real64 CarrollMRTNumerator(0.0);
            Real64 CarrollMRTDenominator(0.0);
            Real64 CarrollMRTInKTo4th; // Carroll MRT
            for (size_type ZoneSurfNum = 0; ZoneSurfNum < s_zone_Surfaces; ++ZoneSurfNum) {
                int const SurfNum = zone_info.SurfacePtr[ZoneSurfNum];
                auto const &surf = state.dataSurface->Surface(SurfNum);
                auto const &surfWindow = state.dataSurface->SurfaceWindow(SurfNum);
                int const constrNum = surf.Construction;
                auto const &construct = state.dataConstruction->Construct(constrNum);
                if (construct.WindowTypeEQL) {
                    SurfaceTempRad[ZoneSurfNum] = state.dataSurface->SurfWinEffInsSurfTemp(SurfNum);
                    SurfaceEmiss[ZoneSurfNum] = WindowEquivalentLayer::EQLWindowInsideEffectiveEmiss(state, constrNum);
                } else if (construct.WindowTypeBSDF && state.dataSurface->SurfWinShadingFlag(SurfNum) == DataSurfaces::WinShadingType::IntShade) {
                    auto &surfShade = state.dataSurface->surfShades(SurfNum);
                    SurfaceTempRad[ZoneSurfNum] = state.dataSurface->SurfWinEffInsSurfTemp(SurfNum);
                    SurfaceEmiss[ZoneSurfNum] = surfShade.effShadeEmi + surfShade.effGlassEmi;
                } else if (construct.WindowTypeBSDF) {
                    SurfaceTempRad[ZoneSurfNum] = state.dataSurface->SurfWinEffInsSurfTemp(SurfNum);
                    SurfaceEmiss[ZoneSurfNum] = construct.InsideAbsorpThermal;
                } else if (construct.TypeIsWindow && surf.OriginalClass != DataSurfaces::SurfaceClass::TDD_Diffuser) {
                    if (SurfIterations == 0 && NOT_SHADED(state.dataSurface->SurfWinShadingFlag(SurfNum))) {
                        // If the window is bare this TS and it is the first time through we use the previous TS glass
                        // temperature whether or not the window was shaded in the previous TS. If the window was shaded
                        // the previous time step this temperature is a better starting value than the shade temperature.
                        SurfaceTempRad[ZoneSurfNum] = surfWindow.thetaFace[2 * construct.TotGlassLayers] - Constant::Kelvin;
                        SurfaceEmiss[ZoneSurfNum] = construct.InsideAbsorpThermal;
                        // For windows with an interior shade or blind an effective inside surface temp
                        // and emiss is used here that is a weighted combination of shade/blind and glass temp and emiss.
                    } else if (ANY_INTERIOR_SHADE_BLIND(state.dataSurface->SurfWinShadingFlag(SurfNum))) {
                        SurfaceTempRad[ZoneSurfNum] = state.dataSurface->SurfWinEffInsSurfTemp(SurfNum);
                        SurfaceEmiss[ZoneSurfNum] = state.dataHeatBalSurf->SurfAbsThermalInt(SurfNum);
                    } else {
                        SurfaceTempRad[ZoneSurfNum] = SurfaceTemp(SurfNum);
                        SurfaceEmiss[ZoneSurfNum] = construct.InsideAbsorpThermal;
                    }
                } else {
                    SurfaceTempRad[ZoneSurfNum] = SurfaceTemp(SurfNum);
                    SurfaceEmiss[ZoneSurfNum] = construct.InsideAbsorpThermal;
                }
                if (state.dataHeatBalIntRadExchg->CarrollMethod) {
                    CarrollMRTNumerator += SurfaceTempRad[ZoneSurfNum] * zone_info.Fp[ZoneSurfNum] * zone_info.Area[ZoneSurfNum];
                    CarrollMRTDenominator += zone_info.Fp[ZoneSurfNum] * zone_info.Area[ZoneSurfNum];
                }
                SurfaceTempInKto4th[ZoneSurfNum] = pow_4(SurfaceTempRad[ZoneSurfNum] + Constant::Kelvin);
            }

            if (state.dataHeatBalIntRadExchg->CarrollMethod) {
                if (CarrollMRTDenominator > 0.0) {
                    CarrollMRTInKTo4th = pow_4(CarrollMRTNumerator / CarrollMRTDenominator + Constant::Kelvin);
                } else {
                    // Likely only one surface in this enclosure
                    CarrollMRTInKTo4th = 293.15; // arbitrary value, IR will be zero
                }
                // These are the money loops
                for (size_type RecZoneSurfNum = 0; RecZoneSurfNum < s_zone_Surfaces; ++RecZoneSurfNum) {
                    int const RecSurfNum = zone_info.SurfacePtr[RecZoneSurfNum];
                    int const ConstrNumRec = state.dataSurface->Surface(RecSurfNum).Construction;
                    auto const &rec_construct = state.dataConstruction->Construct(ConstrNumRec);
                    auto &netLWRadToRecSurf = NetLWRadToSurf(RecSurfNum);
                    if (rec_construct.TypeIsWindow) {
                        //                        auto& rec_surface_window(SurfaceWindow(RecSurfNum));
                        Real64 CarrollMRTInKTo4thWin = CarrollMRTInKTo4th; // arbitrary value, IR will be zero
                        Real64 CarrollMRTNumeratorWin(0.0);
                        Real64 CarrollMRTDenominatorWin(0.0);
                        for (size_type SendZoneSurfNum = 0; SendZoneSurfNum < s_zone_Surfaces; ++SendZoneSurfNum) {
                            if (SendZoneSurfNum != RecZoneSurfNum) {
                                CarrollMRTNumeratorWin +=
                                    SurfaceTempRad[SendZoneSurfNum] * zone_info.Fp[SendZoneSurfNum] * zone_info.Area[SendZoneSurfNum];
                                CarrollMRTDenominatorWin += zone_info.Fp[SendZoneSurfNum] * zone_info.Area[SendZoneSurfNum];
                            }
                        }
                        if (CarrollMRTDenominatorWin > 0.0) {
                            CarrollMRTInKTo4thWin = pow_4(CarrollMRTNumeratorWin / CarrollMRTDenominatorWin + Constant::Kelvin);
                        }
                        state.dataSurface->SurfWinIRfromParentZone(RecSurfNum) +=
                            (zone_info.Fp[RecZoneSurfNum] * CarrollMRTInKTo4thWin) / SurfaceEmiss[RecZoneSurfNum];
                    }
                    netLWRadToRecSurf += zone_info.Fp[RecZoneSurfNum] * (CarrollMRTInKTo4th - SurfaceTempInKto4th[RecZoneSurfNum]);
                }
            } else {
                for (size_type RecZoneSurfNum = 0; RecZoneSurfNum < s_zone_Surfaces; ++RecZoneSurfNum) {
                    int const RecSurfNum = zone_info.SurfacePtr[RecZoneSurfNum];
                    int const ConstrNumRec = state.dataSurface->Surface(RecSurfNum).Construction;
                    auto const &rec_construct = state.dataConstruction->Construct(ConstrNumRec);
                    auto &netLWRadToRecSurf = NetLWRadToSurf(RecSurfNum);

                    // Calculate net long-wave radiation for opaque surfaces and incident
                    // long-wave radiation for windows.
                    if (rec_construct.TypeIsWindow) {      // Window
                                                           //                        auto& rec_surface_window(SurfaceWindow(RecSurfNum));
                        Real64 scriptF_acc(0.0);           // Local accumulator
                        Real64 netLWRadToRecSurf_cor(0.0); // Correction
                        Real64 IRfromParentZone_acc(0.0);  // Local accumulator
                        for (size_type SendZoneSurfNum = 0; SendZoneSurfNum < s_zone_Surfaces; ++SendZoneSurfNum) {
                            size_type lSR = RecZoneSurfNum * s_zone_Surfaces + SendZoneSurfNum;
                            Real64 const scriptF(zone_ScriptF[lSR]); // [ lSR ] == ( SendZoneSurfNum+1, RecZoneSurfNum+1 )
                            Real64 const scriptF_temp_ink_4th(scriptF * SurfaceTempInKto4th[SendZoneSurfNum]);
                            // Calculate interior LW incident on window rather than net LW for use in window layer heat balance calculation.
                            IRfromParentZone_acc += scriptF_temp_ink_4th;

                            if (RecZoneSurfNum != SendZoneSurfNum) {
                                scriptF_acc += scriptF;
                            } else {
                                netLWRadToRecSurf_cor = scriptF_temp_ink_4th;
                            }

                            // Per BG -- this should never happened.  (CR6346,CR6550 caused this to be put in.  Now removed. LKL 1/2013)
                            //          IF (SurfaceWindow(RecSurfNum)%IRfromParentZone < 0.0) THEN
                            //            CALL ShowRecurringWarningErrorAtEnd(state, 'CalcInteriorRadExchange: Window_IRFromParentZone negative,
                            //            Window="'// &
                            //                TRIM(Surface(RecSurfNum)%Name)//'"',  &
                            //                SurfaceWindow(RecSurfNum)%IRErrCount)
                            //            CALL ShowRecurringContinueErrorAtEnd(state, '..occurs in Zone="'//TRIM(Surface(RecSurfNum)%ZoneName)//  &
                            //                '", reset to 0.0 for remaining calculations.',SurfaceWindow(RecSurfNum)%IRErrCountC)
                            //            SurfaceWindow(RecSurfNum)%IRfromParentZone=0.0
                            //          ENDIF
                        }
                        netLWRadToRecSurf += IRfromParentZone_acc - netLWRadToRecSurf_cor - (scriptF_acc * SurfaceTempInKto4th[RecZoneSurfNum]);
                        state.dataSurface->SurfWinIRfromParentZone(RecSurfNum) += IRfromParentZone_acc / SurfaceEmiss[RecZoneSurfNum];
                    } else {
                        Real64 netLWRadToRecSurf_acc(0.0); // Local accumulator
                        zone_ScriptF[RecZoneSurfNum * s_zone_Surfaces + RecZoneSurfNum] = 0;
                        for (size_type SendZoneSurfNum = 0; SendZoneSurfNum < s_zone_Surfaces; ++SendZoneSurfNum) {
                            size_type lSR = RecZoneSurfNum * s_zone_Surfaces + SendZoneSurfNum;
                            netLWRadToRecSurf_acc +=
                                zone_ScriptF[lSR] * (SurfaceTempInKto4th[SendZoneSurfNum] -
                                                     SurfaceTempInKto4th[RecZoneSurfNum]); // [ lSR ] == ( SendZoneSurfNum+1, RecZoneSurfNum+1 )
                        }
                        netLWRadToRecSurf += netLWRadToRecSurf_acc;
                    }
                }
            }
        }

        // Automatic Surface Multipliers: Update values of surfaces not simulated
        if (state.dataSurface->UseRepresentativeSurfaceCalculations) {
            for (int SurfNum : state.dataSurface->AllHTSurfaceList) {
                int RepSurfNum = state.dataSurface->Surface(SurfNum).RepresentativeCalcSurfNum;
                if (SurfNum != RepSurfNum) {
                    state.dataSurface->SurfWinIRfromParentZone(SurfNum) = state.dataSurface->SurfWinIRfromParentZone(RepSurfNum);
                    NetLWRadToSurf(SurfNum) = NetLWRadToSurf(RepSurfNum);
                }
            }
        }
    }

    void UpdateMovableInsulationFlag(EnergyPlusData &state, bool &change, int const SurfNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   July 2016

        // PURPOSE OF THIS SUBROUTINE:
        // To determine if any changes in interior movable insulation have happened.
        // If there have been changes due to a schedule change AND a change in properties,
        // then the matrices which are used to calculate interior radiation must be recalculated.

        auto &s_surf = state.dataSurface;

        change = false;

        auto &movInsul = s_surf->intMovInsuls(SurfNum);
        if (movInsul.present != movInsul.presentPrevTS) {
            change = (std::abs(state.dataConstruction->Construct(s_surf->Surface(SurfNum).Construction).InsideAbsorpThermal -
                               state.dataMaterial->materials(movInsul.matNum)->AbsorpThermal) > 0.01);
        }
    }

    void InitInteriorRadExchange(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   September 2000

        // PURPOSE OF THIS SUBROUTINE:
        // Initializes the various parameters for Hottel's ScriptF method for
        // the grey interchange between surfaces in an enclosure.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        static constexpr std::string_view RoutineName("InitInteriorRadExchange: ");
        bool NoUserInputF; // Logical flag signifying no input F's for zone
        bool ErrorsFound(false);
        Real64 CheckValue1;
        Real64 CheckValue2;
        Real64 FinalCheckValue;
        Array2D<Real64> SaveApproximateViewFactors; // Save for View Factor reporting
        Real64 RowSum;
        Real64 FixedRowSum;
        int NumIterations;
        std::string Option1; // view factor report option

        auto &ViewFactorReport = state.dataHeatBalIntRadExchg->ViewFactorReport;

        General::ScanForReports(state, "ViewFactorInfo", ViewFactorReport, _, Option1);

        if (ViewFactorReport) { // Print heading
            print(state.files.eio, "{}\n", "! <Surface View Factor and Grey Interchange Information>");
            print(state.files.eio, "{}\n", "! <View Factor - Zone/Enclosure Information>,Zone/Enclosure Name,Number of Surfaces");
            print(state.files.eio,
                  "{}\n",
                  "! <View Factor - Surface Information>,Surface Name,Surface Class,Area {m2},Azimuth,Tilt,Thermal Emissivity,#Sides,Vertices");
            print(state.files.eio, "{}\n", "! <View Factor / Grey Interchange Type>,Surface Name(s)");
            print(state.files.eio, "{}\n", "! <View Factor>,Surface Name,Surface Class,Row Sum,View Factors for each Surface");
        }

        state.dataHeatBalIntRadExchg->MaxNumOfRadEnclosureSurfs = 0;
        for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfRadiantEnclosures; ++enclosureNum) {
            auto &thisEnclosure(state.dataViewFactor->EnclRadInfo(enclosureNum));
            if (enclosureNum == 1) {
                if (state.dataGlobal->DisplayAdvancedReportVariables) {
                    print(state.files.eio,
                          "{}\n",
                          "! <Surface View Factor Check Values>,Zone/Enclosure Name,Original Check Value,Calculated Fixed Check Value,Final Check "
                          "Value,Number of Iterations,Fixed RowSum Convergence,Used RowSum Convergence");
                }
            }
            int numEnclosureSurfaces = 0;
            for (int spaceNum : thisEnclosure.spaceNums) {
                // Note that Space.Surfaces only includes HT surfs, see SurfaceGeometry::CreateMissingSpaces
                // But it also includes air boundary surfaces which need to be excluded here
                for (int surfNum : state.dataHeatBal->space(spaceNum).surfaces) {
                    if (state.dataSurface->Surface(surfNum).IsAirBoundarySurf) {
                        continue;
                    }
                    // Automatic Surface Multipliers: Only include representative surfaces
                    if (surfNum == state.dataSurface->Surface(surfNum).RepresentativeCalcSurfNum) {
                        ++numEnclosureSurfaces;
                    }
                }
            }
            thisEnclosure.NumOfSurfaces = numEnclosureSurfaces;
            state.dataHeatBalIntRadExchg->MaxNumOfRadEnclosureSurfs =
                max(state.dataHeatBalIntRadExchg->MaxNumOfRadEnclosureSurfs, numEnclosureSurfaces);
            if (numEnclosureSurfaces < 1) {
                ShowSevereError(state, format("{}No surfaces in enclosure={}.", RoutineName, thisEnclosure.Name));
                ErrorsFound = true;
            }

            // Allocate the parts of the derived type
            thisEnclosure.F.dimension(numEnclosureSurfaces, numEnclosureSurfaces, 0.0);
            thisEnclosure.ScriptF.dimension(numEnclosureSurfaces, numEnclosureSurfaces, 0.0);
            thisEnclosure.Area.dimension(numEnclosureSurfaces, 0.0);
            thisEnclosure.Emissivity.dimension(numEnclosureSurfaces, 0.0);
            thisEnclosure.Azimuth.dimension(numEnclosureSurfaces, 0.0);
            thisEnclosure.Tilt.dimension(numEnclosureSurfaces, 0.0);
            thisEnclosure.Fp.dimension(numEnclosureSurfaces, 1.0);
            thisEnclosure.FMRT.dimension(numEnclosureSurfaces, 0.0);
            thisEnclosure.SurfacePtr.dimension(numEnclosureSurfaces, 0);

            // Initialize the enclosure surface arrays
            int enclosureSurfNum = 0;
            for (int const spaceNum : thisEnclosure.spaceNums) {
                int priorZoneTotEnclSurfs = enclosureSurfNum;
                for (int surfNum : state.dataHeatBal->space(spaceNum).surfaces) {
                    if (state.dataSurface->Surface(surfNum).IsAirBoundarySurf) {
                        continue;
                    }
                    // Automatic Surface Multipliers: Only include representative surfaces
                    if (surfNum == state.dataSurface->Surface(surfNum).RepresentativeCalcSurfNum) {
                        ++enclosureSurfNum;
                        thisEnclosure.SurfacePtr(enclosureSurfNum) = surfNum;
                        thisEnclosure.Area(enclosureSurfNum) = state.dataSurface->Surface(surfNum).Area;
                        thisEnclosure.Emissivity(enclosureSurfNum) =
                            state.dataConstruction->Construct(state.dataSurface->Surface(surfNum).Construction).InsideAbsorpThermal;
                        thisEnclosure.Azimuth(enclosureSurfNum) = state.dataSurface->Surface(surfNum).Azimuth;
                        thisEnclosure.Tilt(enclosureSurfNum) = state.dataSurface->Surface(surfNum).Tilt;
                    }
                }

                for (int surfNum : state.dataHeatBal->space(spaceNum).surfaces) {
                    if (state.dataSurface->Surface(surfNum).IsAirBoundarySurf) {
                        continue;
                    }
                    // Map non-representative surfaces
                    if (surfNum != state.dataSurface->Surface(surfNum).RepresentativeCalcSurfNum) {
                        // Automatic Surface Multipliers: search for corresponding enclosure surface number
                        for (int enclSNum = priorZoneTotEnclSurfs + 1; enclSNum <= enclosureSurfNum; ++enclSNum) {
                            if (thisEnclosure.SurfacePtr(enclSNum) == state.dataSurface->Surface(surfNum).RepresentativeCalcSurfNum) {
                                // enclosureSurfMap[allSurfNum] = enclSNum;
                                //  Increase the area for the combined enclosure surface
                                thisEnclosure.Area(enclSNum) += state.dataSurface->Surface(surfNum).Area;
                            }
                        }
                    }
                    // Store SurfaceReportNums to maintain original reporting order
                    for (int enclSNum = priorZoneTotEnclSurfs + 1; enclSNum <= enclosureSurfNum; ++enclSNum) {
                        if (thisEnclosure.SurfacePtr(enclSNum) == state.dataSurface->AllSurfaceListReportOrder[surfNum - 1]) {
                            thisEnclosure.SurfaceReportNums.push_back(enclSNum);
                            break;
                        }
                    }
                }
            }

            if (thisEnclosure.NumOfSurfaces == 1) {
                // If there is only one surface in a zone, then there is no radiant exchange
                thisEnclosure.F = 0.0;
                thisEnclosure.ScriptF = 0.0;
                thisEnclosure.Fp = 0.0;
                thisEnclosure.FMRT = 0.0;
                if (state.dataGlobal->DisplayAdvancedReportVariables) {
                    print(state.files.eio, "Surface View Factor Check Values,{},0,0,0,-1,0,0\n", thisEnclosure.Name);
                }
                continue; // Go to the next enclosure in the loop
            }

            // Process View Factors
            if (state.dataHeatBalIntRadExchg->CarrollMethod) {

                // User View Factors cannot be used with Carroll method.
                if (state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "ZoneProperty:UserViewFactors:BySurfaceName")) {
                    ShowWarningError(state, "ZoneProperty:UserViewFactors:BySurfaceName objects have been defined, however View");
                    ShowContinueError(state, "  Factors are not used when Zone Radiant Exchange Algorithm is set to CarrollMRT.");
                }
                CalcFMRT(state, thisEnclosure.NumOfSurfaces, thisEnclosure.Area, thisEnclosure.FMRT);
                CalcFp(thisEnclosure.NumOfSurfaces, thisEnclosure.Emissivity, thisEnclosure.FMRT, thisEnclosure.Fp);
            } else {
                //  Get user supplied view factors if available in idf.

                NoUserInputF = true;

                constexpr std::string_view cCurrentModuleObject = "ZoneProperty:UserViewFactors:BySurfaceName";
                int NumZonesWithUserFbyS = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
                if (NumZonesWithUserFbyS > 0) {

                    GetInputViewFactorsbyName(state,
                                              thisEnclosure.Name,
                                              thisEnclosure.NumOfSurfaces,
                                              thisEnclosure.F,
                                              thisEnclosure.SurfacePtr,
                                              NoUserInputF,
                                              ErrorsFound); // Obtains user input view factors from input file
                }

                if (NoUserInputF) {

                    // Calculate the view factors and make sure they satisfy reciprocity
                    CalcApproximateViewFactors(state,
                                               thisEnclosure.NumOfSurfaces,
                                               thisEnclosure.Area,
                                               thisEnclosure.Azimuth,
                                               thisEnclosure.Tilt,
                                               thisEnclosure.F,
                                               thisEnclosure.SurfacePtr);
                }

                if (ViewFactorReport) { // Allocate and save user or approximate view factors for reporting.
                    SaveApproximateViewFactors.allocate(thisEnclosure.NumOfSurfaces, thisEnclosure.NumOfSurfaces);
                    SaveApproximateViewFactors = thisEnclosure.F;
                }

                bool anyIntMassInZone = DoesZoneHaveInternalMass(state, thisEnclosure.NumOfSurfaces, thisEnclosure.SurfacePtr);
                FixViewFactors(state,
                               thisEnclosure.NumOfSurfaces,
                               thisEnclosure.Area,
                               thisEnclosure.F,
                               thisEnclosure.Name,
                               thisEnclosure.spaceNums,
                               CheckValue1,
                               CheckValue2,
                               FinalCheckValue,
                               NumIterations,
                               FixedRowSum,
                               anyIntMassInZone);

                // Calculate the script F factors
                CalcScriptF(state, thisEnclosure.NumOfSurfaces, thisEnclosure.Area, thisEnclosure.F, thisEnclosure.Emissivity, thisEnclosure.ScriptF);
                if (ViewFactorReport) { // Write to SurfInfo File
                    // Zone Surface Information Output
                    print(
                        state.files.eio, "Surface View Factor - Zone/Enclosure Information,{},{}\n", thisEnclosure.Name, thisEnclosure.NumOfSurfaces);

                    for (int SurfNum : thisEnclosure.SurfaceReportNums) {
                        print(state.files.eio,
                              "Surface View Factor - Surface Information,{},{},{:.4R},{:.4R},{:.4R},{:.4R},{}",
                              state.dataSurface->Surface(thisEnclosure.SurfacePtr(SurfNum)).Name,
                              cSurfaceClass(state.dataSurface->Surface(thisEnclosure.SurfacePtr(SurfNum)).Class),
                              thisEnclosure.Area(SurfNum),
                              thisEnclosure.Azimuth(SurfNum),
                              thisEnclosure.Tilt(SurfNum),
                              thisEnclosure.Emissivity(SurfNum),
                              state.dataSurface->Surface(thisEnclosure.SurfacePtr(SurfNum)).Sides);
                        for (int Vindex = 1; Vindex <= state.dataSurface->Surface(thisEnclosure.SurfacePtr(SurfNum)).Sides; ++Vindex) {
                            auto const &Vertex = state.dataSurface->Surface(thisEnclosure.SurfacePtr(SurfNum)).Vertex(Vindex);
                            print(state.files.eio, ",{:.4R},{:.4R},{:.4R}", Vertex.x, Vertex.y, Vertex.z);
                        }
                        print(state.files.eio, "\n");
                    }

                    print(state.files.eio, "Approximate or User Input ViewFactors,To Surface,Surface Class,RowSum");
                    for (int SurfNum : thisEnclosure.SurfaceReportNums) {
                        print(state.files.eio, ",{}", state.dataSurface->Surface(thisEnclosure.SurfacePtr(SurfNum)).Name);
                    }
                    print(state.files.eio, "\n");

                    for (int Findex : thisEnclosure.SurfaceReportNums) {
                        RowSum = sum(SaveApproximateViewFactors(_, Findex));
                        print(state.files.eio,
                              "{},{},{},{:.4R}",
                              "View Factor",
                              state.dataSurface->Surface(thisEnclosure.SurfacePtr(Findex)).Name,
                              cSurfaceClass(state.dataSurface->Surface(thisEnclosure.SurfacePtr(Findex)).Class),
                              RowSum);
                        for (int SurfNum : thisEnclosure.SurfaceReportNums) {
                            print(state.files.eio, ",{:.4R}", SaveApproximateViewFactors(SurfNum, Findex));
                        }
                        print(state.files.eio, "\n");
                    }

                    print(state.files.eio, "Final ViewFactors,To Surface,Surface Class,RowSum");
                    for (int SurfNum : thisEnclosure.SurfaceReportNums) {
                        print(state.files.eio, ",{}", state.dataSurface->Surface(thisEnclosure.SurfacePtr(SurfNum)).Name);
                    }
                    print(state.files.eio, "\n");

                    for (int Findex : thisEnclosure.SurfaceReportNums) {
                        RowSum = sum(thisEnclosure.F(_, Findex));
                        print(state.files.eio,
                              "{},{},{},{:.4R}",
                              "View Factor",
                              state.dataSurface->Surface(thisEnclosure.SurfacePtr(Findex)).Name,
                              cSurfaceClass(state.dataSurface->Surface(thisEnclosure.SurfacePtr(Findex)).Class),
                              RowSum);
                        for (int SurfNum : thisEnclosure.SurfaceReportNums) {
                            print(state.files.eio, ",{:.4R}", thisEnclosure.F(SurfNum, Findex));
                        }
                        print(state.files.eio, "\n");
                    }

                    if (Option1 == "IDF") {
                        // TODO Both "original" and "final" print the same output. This is likely a bug
                        // (discovered while updating output to {fmt}
                        // see:
                        // https://github.com/NREL/EnergyPlusArchive/commit/1c08247853c297dce59f3f53cde47ccfa67720c0#diff-124964a7e9b73ce494c1952ab1acdeeb
                        print(state.files.debug, "{}\n", "!======== original input factors ===========================");
                        print(state.files.debug, "ZoneProperty:UserViewFactors:BySurfaceName,{},\n", thisEnclosure.Name);
                        for (int SurfNum : thisEnclosure.SurfaceReportNums) {
                            for (int Findex : thisEnclosure.SurfaceReportNums) {
                                print(state.files.debug,
                                      "  {},{},{:.6R}",
                                      state.dataSurface->Surface(thisEnclosure.SurfacePtr(SurfNum)).Name,
                                      state.dataSurface->Surface(thisEnclosure.SurfacePtr(Findex)).Name,
                                      thisEnclosure.F(Findex, SurfNum));
                                if (!(SurfNum == thisEnclosure.NumOfSurfaces && Findex == thisEnclosure.NumOfSurfaces)) {
                                    print(state.files.debug, ",\n");
                                } else {
                                    print(state.files.debug, ";\n");
                                }
                            }
                        }
                        print(state.files.debug, "{}\n", "!============= end of data ======================");

                        print(state.files.debug, "{}\n", "!============ final view factors =======================");
                        print(state.files.debug, "ZoneProperty:UserViewFactors:BySurfaceName,{},\n", thisEnclosure.Name);
                        for (int SurfNum : thisEnclosure.SurfaceReportNums) {
                            for (int Findex : thisEnclosure.SurfaceReportNums) {
                                print(state.files.debug,
                                      "  {},{},{:.6R}",
                                      state.dataSurface->Surface(thisEnclosure.SurfacePtr(SurfNum)).Name,
                                      state.dataSurface->Surface(thisEnclosure.SurfacePtr(Findex)).Name,
                                      thisEnclosure.F(Findex, SurfNum));
                                if (!(SurfNum == thisEnclosure.SurfaceReportNums.back() && Findex == thisEnclosure.SurfaceReportNums.back())) {
                                    print(state.files.debug, ",\n");
                                } else {
                                    print(state.files.debug, ";\n");
                                }
                            }
                        }
                        print(state.files.debug, "{}\n", "!============= end of data ======================");
                    }

                    print(state.files.eio, "Script F Factors,X Surface");
                    for (int SurfNum : thisEnclosure.SurfaceReportNums) {
                        print(state.files.eio, ",{}", state.dataSurface->Surface(thisEnclosure.SurfacePtr(SurfNum)).Name);
                    }
                    print(state.files.eio, "\n");
                    for (int Findex : thisEnclosure.SurfaceReportNums) {
                        print(state.files.eio, "{},{}", "Script F Factor", state.dataSurface->Surface(thisEnclosure.SurfacePtr(Findex)).Name);
                        for (int SurfNum : thisEnclosure.SurfaceReportNums) {
                            print(state.files.eio, ",{:.4R}", thisEnclosure.ScriptF(Findex, SurfNum));
                        }
                        print(state.files.eio, "\n");
                    }

                    // Deallocate saved approximate/user view factors
                    SaveApproximateViewFactors.deallocate();
                }

                RowSum = 0.0;
                for (int Findex : thisEnclosure.SurfaceReportNums) {
                    RowSum += sum(thisEnclosure.F(_, Findex));
                }
                RowSum = std::abs(RowSum - thisEnclosure.NumOfSurfaces);
                FixedRowSum = std::abs(FixedRowSum - thisEnclosure.NumOfSurfaces);
                if (state.dataGlobal->DisplayAdvancedReportVariables) {
                    print(state.files.eio,
                          "Surface View Factor Check Values,{},{:.6R},{:.6R},{:.6R},{},{:.6R},{:.6R}\n",
                          thisEnclosure.Name,
                          CheckValue1,
                          CheckValue2,
                          FinalCheckValue,
                          NumIterations,
                          FixedRowSum,
                          RowSum);
                }
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, format("{}Errors found during initialization of radiant exchange.  Program terminated.", RoutineName));
        }
    }

    void InitSolarViewFactors(EnergyPlusData &state)
    {

        // Initializes view factors for diffuse solar distribution between surfaces in an enclosure.

        Array2D<Real64> SaveApproximateViewFactors; // Save for View Factor reporting
        std::string Option1;                        // view factor report option
        static constexpr std::string_view RoutineName("InitSolarViewFactors: ");

        bool ErrorsFound = false;
        bool ViewFactorReport = false;
        General::ScanForReports(state, "ViewFactorInfo", ViewFactorReport, _, Option1);

        if (ViewFactorReport) { // Print heading
            print(state.files.eio, "{}\n", "! <Solar View Factor Information>");
            print(state.files.eio, "{}\n", "! <Solar View Factor - Zone/Enclosure Information>,Zone/Enclosure Name,Number of Surfaces");
            print(state.files.eio,
                  "{}\n",
                  "! <Solar View Factor - Surface Information>,Surface Name,Surface Class,Area {m2},Azimuth,Tilt,Solar Absorbtance,#Sides,Vertices");
            print(state.files.eio, "{}\n", "! <Solar View Factor / Interchange Type>,Surface Name(s)");
            print(state.files.eio, "{}\n", "! <Solar View Factor>,Surface Name,Surface Class,Row Sum,View Factors for each Surface");
        }

        const std::string cCurrentModuleObject = "ZoneProperty:UserViewFactors:BySurfaceName";
        int NumZonesWithUserFbyS = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        if (NumZonesWithUserFbyS > 0) {
            AlignInputViewFactors(state, cCurrentModuleObject, ErrorsFound);
        }

        for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclosureNum) {
            auto &thisEnclosure = state.dataViewFactor->EnclSolInfo(enclosureNum);
            if (enclosureNum == 1) {
                if (state.dataGlobal->DisplayAdvancedReportVariables) {
                    print(state.files.eio,
                          "{}\n",
                          "! <Solar View Factor Check Values>,Zone/Enclosure Name,Original Check Value,Calculated Fixed Check "
                          "Value,Final Check Value,Number of Iterations,Fixed RowSum Convergence,Used RowSum "
                          "Convergence");
                }
            }
            int numEnclosureSurfaces = 0;
            for (int spaceNum : thisEnclosure.spaceNums) {
                // Note that Space.Surfaces only includes HT surfs, see SurfaceGeometry::CreateMissingSpaces
                // But it also includes air boundary surfaces which need to be excluded here
                for (int surfNum : state.dataHeatBal->space(spaceNum).surfaces) {
                    if (state.dataSurface->Surface(surfNum).IsAirBoundarySurf) {
                        continue;
                    }
                    ++numEnclosureSurfaces;
                }
            }
            thisEnclosure.NumOfSurfaces = numEnclosureSurfaces;
            if (numEnclosureSurfaces < 1) {
                ShowSevereError(state, format("{}No surfaces in enclosure={}.", RoutineName, thisEnclosure.Name));
                ErrorsFound = true;
            }

            // Allocate the parts of the derived type
            thisEnclosure.F.dimension(numEnclosureSurfaces, numEnclosureSurfaces, 0.0);
            thisEnclosure.Area.dimension(numEnclosureSurfaces, 0.0);
            thisEnclosure.SolAbsorptance.dimension(numEnclosureSurfaces, 0.0);
            thisEnclosure.Azimuth.dimension(numEnclosureSurfaces, 0.0);
            thisEnclosure.Tilt.dimension(numEnclosureSurfaces, 0.0);
            thisEnclosure.SurfacePtr.dimension(numEnclosureSurfaces, 0);

            // Initialize the surface pointer array
            int enclosureSurfNum = 0;
            for (int const spaceNum : thisEnclosure.spaceNums) {
                int priorZoneTotEnclSurfs = enclosureSurfNum;
                for (int surfNum : state.dataHeatBal->space(spaceNum).surfaces) {
                    if (state.dataSurface->Surface(surfNum).IsAirBoundarySurf) {
                        continue;
                    }
                    ++enclosureSurfNum;
                    thisEnclosure.SurfacePtr(enclosureSurfNum) = surfNum;
                    // Store pointers back to here
                    state.dataSurface->Surface(surfNum).SolarEnclSurfIndex = enclosureSurfNum;
                    state.dataSurface->Surface(surfNum).SolarEnclIndex = enclosureNum;
                    state.dataSurface->Surface(surfNum).RadEnclIndex = enclosureNum; // Radiant and Solar enclosures are parallel for now
                    if ((state.dataConstruction->Construct(state.dataSurface->Surface(surfNum).Construction).TypeIsWindow) &&
                        (state.dataSurface->Surface(surfNum).ExtBoundCond > 0) &&
                        (state.dataSurface->Surface(surfNum).BaseSurf != surfNum)) { // Interzone window present
                        thisEnclosure.HasInterZoneWindow = true;
                    }
                }
                // Store SurfaceReportNums to maintain original reporting order
                for (int surfNum : state.dataHeatBal->space(spaceNum).surfaces) {
                    for (int enclSNum = priorZoneTotEnclSurfs + 1; enclSNum <= enclosureSurfNum; ++enclSNum) {
                        if (thisEnclosure.SurfacePtr(enclSNum) == state.dataSurface->AllSurfaceListReportOrder[surfNum - 1]) {
                            thisEnclosure.SurfaceReportNums.push_back(enclSNum);
                            break;
                        }
                    }
                }
            }
            // Initialize the area and related arrays
            for (int enclSurfNum = 1; enclSurfNum <= thisEnclosure.NumOfSurfaces; ++enclSurfNum) {
                int const SurfNum = thisEnclosure.SurfacePtr(enclSurfNum);
                thisEnclosure.Area(enclSurfNum) = state.dataSurface->Surface(SurfNum).Area;
                thisEnclosure.SolAbsorptance(enclSurfNum) =
                    state.dataConstruction->Construct(state.dataSurface->Surface(SurfNum).Construction).InsideAbsorpSolar;
                thisEnclosure.Azimuth(enclSurfNum) = state.dataSurface->Surface(SurfNum).Azimuth;
                thisEnclosure.Tilt(enclSurfNum) = state.dataSurface->Surface(SurfNum).Tilt;
            }

            if (thisEnclosure.NumOfSurfaces == 1) {
                // If there is only one surface in a zone, then there is no solar distribution
                if (state.dataGlobal->DisplayAdvancedReportVariables) {
                    print(state.files.eio, "Solar View Factor Check Values,{},0,0,0,-1,0,0\n", thisEnclosure.Name);
                }
                continue; // Go to the next enclosure in the loop
            }

            //  Get user supplied view factors if available in idf.

            bool NoUserInputF = true;

            if (NumZonesWithUserFbyS > 0) {

                GetInputViewFactorsbyName(state,
                                          thisEnclosure.Name,
                                          thisEnclosure.NumOfSurfaces,
                                          thisEnclosure.F,
                                          thisEnclosure.SurfacePtr,
                                          NoUserInputF,
                                          ErrorsFound); // Obtains user input view factors from input file
            }

            if (NoUserInputF) {

                // Calculate the view factors and make sure they satisfy reciprocity
                CalcApproximateViewFactors(state,
                                           thisEnclosure.NumOfSurfaces,
                                           thisEnclosure.Area,
                                           thisEnclosure.Azimuth,
                                           thisEnclosure.Tilt,
                                           thisEnclosure.F,
                                           thisEnclosure.SurfacePtr);
            }

            if (ViewFactorReport) { // Allocate and save user or approximate view factors for reporting.
                SaveApproximateViewFactors.allocate(thisEnclosure.NumOfSurfaces, thisEnclosure.NumOfSurfaces);
                SaveApproximateViewFactors = thisEnclosure.F;
            }

            Real64 CheckValue1 = 0.0;
            Real64 CheckValue2 = 0.0;
            Real64 FinalCheckValue = 0.0;
            Real64 FixedRowSum = 0.0;
            int NumIterations = 0;

            bool anyIntMassInZone = DoesZoneHaveInternalMass(state, thisEnclosure.NumOfSurfaces, thisEnclosure.SurfacePtr);
            FixViewFactors(state,
                           thisEnclosure.NumOfSurfaces,
                           thisEnclosure.Area,
                           thisEnclosure.F,
                           thisEnclosure.Name,
                           thisEnclosure.spaceNums,
                           CheckValue1,
                           CheckValue2,
                           FinalCheckValue,
                           NumIterations,
                           FixedRowSum,
                           anyIntMassInZone);

            if (ViewFactorReport) { // Write to SurfInfo File
                // Zone Surface Information Output
                print(state.files.eio, "Solar View Factor - Zone/Enclosure Information,{},{}\n", thisEnclosure.Name, thisEnclosure.NumOfSurfaces);

                for (int SurfNum : thisEnclosure.SurfaceReportNums) {
                    print(state.files.eio,
                          "Solar View Factor - Surface Information,{},{},{:.4R},{:.4R},{:.4R},{:.4R},{}",
                          state.dataSurface->Surface(thisEnclosure.SurfacePtr(SurfNum)).Name,
                          cSurfaceClass(state.dataSurface->Surface(thisEnclosure.SurfacePtr(SurfNum)).Class),
                          thisEnclosure.Area(SurfNum),
                          thisEnclosure.Azimuth(SurfNum),
                          thisEnclosure.Tilt(SurfNum),
                          thisEnclosure.SolAbsorptance(SurfNum),
                          state.dataSurface->Surface(thisEnclosure.SurfacePtr(SurfNum)).Sides);

                    for (int Vindex = 1; Vindex <= state.dataSurface->Surface(thisEnclosure.SurfacePtr(SurfNum)).Sides; ++Vindex) {
                        auto const &Vertex = state.dataSurface->Surface(thisEnclosure.SurfacePtr(SurfNum)).Vertex(Vindex);
                        print(state.files.eio, ",{:.4R},{:.4R},{:.4R}", Vertex.x, Vertex.y, Vertex.z);
                    }
                    print(state.files.eio, "\n");
                }

                print(state.files.eio, "Approximate or User Input Solar ViewFactors,To Surface,Surface Class,RowSum");
                for (int SurfNum : thisEnclosure.SurfaceReportNums) {
                    print(state.files.eio, ",{}", state.dataSurface->Surface(thisEnclosure.SurfacePtr(SurfNum)).Name);
                }
                print(state.files.eio, "\n");

                for (int Findex : thisEnclosure.SurfaceReportNums) {
                    Real64 RowSum = sum(SaveApproximateViewFactors(_, Findex));
                    print(state.files.eio,
                          "Solar View Factor,{},{},{:.4R}",
                          state.dataSurface->Surface(thisEnclosure.SurfacePtr(Findex)).Name,
                          cSurfaceClass(state.dataSurface->Surface(thisEnclosure.SurfacePtr(Findex)).Class),
                          RowSum);
                    for (int SurfNum : thisEnclosure.SurfaceReportNums) {
                        print(state.files.eio, ",{:.4R}", SaveApproximateViewFactors(SurfNum, Findex));
                    }
                    print(state.files.eio, "\n");
                }

                print(state.files.eio, "Final Solar ViewFactors,To Surface,Surface Class,RowSum");
                for (int SurfNum : thisEnclosure.SurfaceReportNums) {
                    print(state.files.eio, ",{}", state.dataSurface->Surface(thisEnclosure.SurfacePtr(SurfNum)).Name);
                }
                print(state.files.eio, "\n");

                for (int Findex : thisEnclosure.SurfaceReportNums) {
                    Real64 RowSum = sum(thisEnclosure.F(_, Findex));
                    print(state.files.eio,
                          "{},{},{},{:.4R}",
                          "Solar View Factor",
                          state.dataSurface->Surface(thisEnclosure.SurfacePtr(Findex)).Name,
                          cSurfaceClass(state.dataSurface->Surface(thisEnclosure.SurfacePtr(Findex)).Class),
                          RowSum);
                    for (int SurfNum : thisEnclosure.SurfaceReportNums) {
                        print(state.files.eio, ",{:.4R}", thisEnclosure.F(SurfNum, Findex));
                    }
                    print(state.files.eio, "\n");
                }

                if (Option1 == "IDF") {
                    // TODO Both "original" and "final" print the same output. This is likely a bug
                    // see:
                    // https://github.com/NREL/EnergyPlusArchive/commit/1c08247853c297dce59f3f53cde47ccfa67720c0#diff-124964a7e9b73ce494c1952ab1acdeeb
                    print(state.files.debug, "{}\n", "!======== original input factors ===========================");
                    print(state.files.debug, "ZoneProperty:UserViewFactors:BySurfaceName,{},\n", thisEnclosure.Name);
                    for (int SurfNum : thisEnclosure.SurfaceReportNums) {
                        for (int Findex : thisEnclosure.SurfaceReportNums) {
                            print(state.files.debug,
                                  "  {},{},{:.6R}",
                                  state.dataSurface->Surface(thisEnclosure.SurfacePtr(SurfNum)).Name,
                                  state.dataSurface->Surface(thisEnclosure.SurfacePtr(Findex)).Name,
                                  thisEnclosure.F(Findex, SurfNum));
                            if (!(SurfNum == thisEnclosure.NumOfSurfaces && Findex == thisEnclosure.NumOfSurfaces)) {
                                print(state.files.debug, ",\n");
                            } else {
                                print(state.files.debug, ";\n");
                            }
                        }
                    }
                    print(state.files.debug, "{}\n", "!============= end of data ======================");

                    print(state.files.debug, "{}\n", "!============ final view factors =======================");
                    print(state.files.debug, "ZoneProperty:UserViewFactors:BySurfaceName,{},\n", thisEnclosure.Name);
                    for (int SurfNum : thisEnclosure.SurfaceReportNums) {
                        for (int Findex : thisEnclosure.SurfaceReportNums) {
                            print(state.files.debug,
                                  "  {},{},{:.6R}",
                                  state.dataSurface->Surface(thisEnclosure.SurfacePtr(SurfNum)).Name,
                                  state.dataSurface->Surface(thisEnclosure.SurfacePtr(Findex)).Name,
                                  thisEnclosure.F(Findex, SurfNum));
                            if (!(SurfNum == thisEnclosure.NumOfSurfaces && Findex == thisEnclosure.NumOfSurfaces)) {
                                print(state.files.debug, ",\n");
                            } else {
                                print(state.files.debug, ";\n");
                            }
                        }
                    }
                    print(state.files.debug, "{}\n", "!============= end of data ======================");
                }
                SaveApproximateViewFactors.deallocate();
            }

            Real64 RowSum = 0.0;
            for (int Findex : thisEnclosure.SurfaceReportNums) {
                RowSum += sum(thisEnclosure.F(_, Findex));
            }
            RowSum = std::abs(RowSum - thisEnclosure.NumOfSurfaces);
            FixedRowSum = std::abs(FixedRowSum - thisEnclosure.NumOfSurfaces);
            if (state.dataGlobal->DisplayAdvancedReportVariables) {
                print(state.files.eio,
                      "Solar View Factor Check Values,{},{:.6R},{:.6R},{:.6R},{},{:.6R},{:.6R}\n",
                      thisEnclosure.Name,
                      CheckValue1,
                      CheckValue2,
                      FinalCheckValue,
                      NumIterations,
                      FixedRowSum,
                      RowSum);
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, format("{}Errors found during initialization of diffuse solar distribution.  Program terminated.", RoutineName));
        }
    }

    void GetInputViewFactors(EnergyPlusData &state,
                             std::string const &ZoneName,              // Needed to check for user input view factors.
                             int const N,                              // NUMBER OF SURFACES
                             Array2A<Real64> F,                        // USER INPUT DIRECT VIEW FACTOR MATRIX (N X N)
                             [[maybe_unused]] const Array1D_int &SPtr, // pointer to actual surface number
                             bool &NoUserInputF,                       // Flag signifying no input F's for this
                             bool &ErrorsFound                         // True when errors are found in number of fields vs max args
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Curt Pedersen
        //       DATE WRITTEN   September 2005
        //       MODIFIED       Linda Lawrie;September 2010

        // PURPOSE OF THIS SUBROUTINE:
        // This routine gets the user view factor info.

        // Argument array dimensioning
        F.dim(N, N);
        // EP_SIZE_CHECK(SPtr, N);

        NoUserInputF = true;
        int UserFZoneIndex = state.dataInputProcessing->inputProcessor->getObjectItemNum(state, "ZoneProperty:UserViewFactors", ZoneName);
        if (UserFZoneIndex > 0) {
            int NumAlphas;
            int NumNums;
            int IOStat;
            NoUserInputF = false;

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     "ZoneProperty:UserViewFactors",
                                                                     UserFZoneIndex,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNums,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            if (NumNums < 3 * pow_2(N)) {
                std::string_view cCurrentModuleObject = "ZoneProperty:UserViewFactors";
                ShowSevereError(state, format("GetInputViewFactors: {}=\"{}\", not enough values.", cCurrentModuleObject, ZoneName));
                ShowContinueError(state, format("...Number of input values [{}] is less than the required number=[{}].", NumNums, 3 * pow_2(N)));
                ErrorsFound = true;
                NumNums = 0;
            }
            F = 0.0;
            for (int index = 1; index <= NumNums; index += 3) {
                int inx1 = state.dataIPShortCut->rNumericArgs(index);
                int inx2 = state.dataIPShortCut->rNumericArgs(index + 1);
                F(inx2, inx1) = state.dataIPShortCut->rNumericArgs(index + 2);
            }
        }
    }

    void AlignInputViewFactors(EnergyPlusData &state,
                               std::string const &cCurrentModuleObject, // Object type
                               bool &ErrorsFound                        // True when errors are found
    )
    {
        auto const instances = state.dataInputProcessing->inputProcessor->epJSON.find(cCurrentModuleObject);
        auto &instancesValue = instances.value();
        for (auto instance = instancesValue.begin(); instance != instancesValue.end(); ++instance) {
            auto const &fields = instance.value();
            std::string const thisSpaceOrSpaceListName = Util::makeUPPER(fields.at("zone_or_zonelist_or_space_or_spacelist_name").get<std::string>());
            // do not mark object as used here - let GetInputViewFactorsbyName do that

            // Look for matching radiant enclosure name (solar enclosures have the same names)
            bool enclMatchFound = false;
            for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfRadiantEnclosures; ++enclosureNum) {
                auto &thisEnclosure = state.dataViewFactor->EnclRadInfo(enclosureNum);
                if (Util::SameString(thisSpaceOrSpaceListName, thisEnclosure.Name)) {
                    // View factor space name matches enclosure name
                    enclMatchFound = true;
                    break;
                }
            }
            if (enclMatchFound) {
                continue; // We're done with this instance
            }
            // Find matching SpaceList name
            int spaceListNum = Util::FindItemInList(thisSpaceOrSpaceListName, state.dataHeatBal->spaceList);
            int zoneListNum = 0;
            int inputZoneNum = 0;
            size_t listSize = 0;
            if (spaceListNum > 0) {
                listSize = int(state.dataHeatBal->spaceList(spaceListNum).spaces.size());
            } else {
                // Check Zone and ZoneList for backwards compatibility
                zoneListNum = Util::FindItemInList(thisSpaceOrSpaceListName, state.dataHeatBal->ZoneList);
                if (zoneListNum > 0) {
                    for (int const zoneNum : state.dataHeatBal->ZoneList(zoneListNum).Zone) {
                        listSize += state.dataHeatBal->Zone(zoneNum).spaceIndexes.size();
                    }
                } else {
                    inputZoneNum = Util::FindItemInList(thisSpaceOrSpaceListName, state.dataHeatBal->Zone);
                    if (inputZoneNum > 0) {
                        listSize = state.dataHeatBal->Zone(inputZoneNum).spaceIndexes.size();
                    }
                }
            }

            if (listSize > 0) {
                // Look for radiant enclosure with same list of spaces (solar enclosures are the same, so this is sufficient)
                for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfRadiantEnclosures; ++enclosureNum) {
                    auto &thisEnclosure = state.dataViewFactor->EnclRadInfo(enclosureNum);
                    bool anySpaceNotFound = false;
                    // If the number of enclosure spaces is not the same as the number of spacelist space, go to the next enclosure
                    if (listSize != thisEnclosure.spaceNums.size()) {
                        continue;
                    }
                    if (spaceListNum > 0) {
                        for (int sListSpaceNum : state.dataHeatBal->spaceList(spaceListNum).spaces) {
                            // Search for matching spaces
                            bool thisSpaceFound = false;
                            for (int enclSpaceNum : thisEnclosure.spaceNums) {
                                if (enclSpaceNum == sListSpaceNum) {
                                    thisSpaceFound = true;
                                    break;
                                }
                            }
                            if (!thisSpaceFound) {
                                anySpaceNotFound = true;
                                break;
                            }
                        }
                    } else if (zoneListNum > 0) {
                        for (int zListZoneNum : state.dataHeatBal->ZoneList(zoneListNum).Zone) {
                            for (int spaceNum : state.dataHeatBal->Zone(zListZoneNum).spaceIndexes) {
                                // Search for matching spaces
                                bool thisSpaceFound = false;
                                for (int enclSpaceNum : thisEnclosure.spaceNums) {
                                    if (enclSpaceNum == spaceNum) {
                                        thisSpaceFound = true;
                                        break;
                                    }
                                }
                                if (!thisSpaceFound) {
                                    anySpaceNotFound = true;
                                    break;
                                }
                            }
                            if (anySpaceNotFound) {
                                break;
                            }
                        }
                    } else if (inputZoneNum > 0) {
                        for (int spaceNum : state.dataHeatBal->Zone(inputZoneNum).spaceIndexes) {
                            // Search for matching spaces
                            bool thisSpaceFound = false;
                            for (int enclSpaceNum : thisEnclosure.spaceNums) {
                                if (enclSpaceNum == spaceNum) {
                                    thisSpaceFound = true;
                                    break;
                                }
                            }
                            if (!thisSpaceFound) {
                                anySpaceNotFound = true;
                                break;
                            }
                        }
                    }
                    if (anySpaceNotFound) {
                        continue; // On to the next enclosure
                    } else {
                        enclMatchFound = true;
                        // If matching SpaceList or ZoneList or Zone found, set the radiant and solar enclosure names to match
                        thisEnclosure.Name = thisSpaceOrSpaceListName;
                        state.dataViewFactor->EnclSolInfo(enclosureNum).Name = thisSpaceOrSpaceListName;
                        break; // We're done with radiant enclosures
                    }
                }
            }
            if (!enclMatchFound) {
                if (spaceListNum > 0) {
                    ShowSevereError(state,
                                    format("AlignInputViewFactors: {}=\"{}\" found a matching SpaceList, but did not find a matching radiant or "
                                           "solar enclosure with the same spaces.",
                                           cCurrentModuleObject,
                                           thisSpaceOrSpaceListName));
                    ErrorsFound = true;

                } else if (zoneListNum > 0) {
                    ShowSevereError(state,
                                    format("AlignInputViewFactors: {}=\"{}\" found a matching ZoneList, but did not find a matching radiant or solar "
                                           "enclosure with the same spaces.",
                                           cCurrentModuleObject,
                                           thisSpaceOrSpaceListName));
                    ErrorsFound = true;

                } else {
                    ShowSevereError(state,
                                    format("AlignInputViewFactors: {}=\"{}\" did not find a matching radiant or solar enclosure name.",
                                           cCurrentModuleObject,
                                           thisSpaceOrSpaceListName));
                    ErrorsFound = true;
                }
            }
        }
    }

    void GetInputViewFactorsbyName(EnergyPlusData &state,
                                   std::string const &EnclosureName, // Needed to check for user input view factors.
                                   int const N,                      // NUMBER OF SURFACES
                                   Array2A<Real64> F,                // USER INPUT DIRECT VIEW FACTOR MATRIX (N X N)
                                   const Array1D_int &SPtr,          // pointer to actual surface number
                                   bool &NoUserInputF,               // Flag signifying no input F's for this
                                   bool &ErrorsFound                 // True when errors are found in number of fields vs max args
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Curt Pedersen
        //       DATE WRITTEN   September 2005
        //       MODIFIED       Linda Lawrie;September 2010

        // PURPOSE OF THIS SUBROUTINE:
        // This routine gets the user view factor info for an enclosure which could be a zone or a group of zones

        // Argument array dimensioning
        F.dim(N, N);
        EP_SIZE_CHECK(SPtr, N);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int UserFZoneIndex;
        Array1D_string enclosureSurfaceNames;
        NoUserInputF = true;
        UserFZoneIndex = state.dataInputProcessing->inputProcessor->getObjectItemNum(
            state, "ZoneProperty:UserViewFactors:BySurfaceName", "zone_or_zonelist_or_space_or_spacelist_name", EnclosureName);

        if (UserFZoneIndex > 0) {
            int NumAlphas;
            int NumNums;
            int IOStat;
            auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;
            enclosureSurfaceNames.allocate(N);
            for (int index = 1; index <= N; ++index) {
                enclosureSurfaceNames(index) = state.dataSurface->Surface(SPtr(index)).Name;
            }
            NoUserInputF = false;

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     "ZoneProperty:UserViewFactors:BySurfaceName",
                                                                     UserFZoneIndex,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     state.dataIPShortCut->rNumericArgs,
                                                                     NumNums,
                                                                     IOStat,
                                                                     state.dataIPShortCut->lNumericFieldBlanks,
                                                                     state.dataIPShortCut->lAlphaFieldBlanks,
                                                                     state.dataIPShortCut->cAlphaFieldNames,
                                                                     state.dataIPShortCut->cNumericFieldNames);

            F = 0.0;
            int numinx1 = 0;
            if (NumNums < pow_2(N)) {
                ShowWarningError(state, format("GetInputViewFactors: {}=\"{}\", not enough values.", cCurrentModuleObject, EnclosureName));
                ShowContinueError(state,
                                  format("...Number of input values [{}] is less than the required number=[{}] Missing surface pairs will have a "
                                         "zero view factor.",
                                         NumNums,
                                         pow_2(N)));
            }

            for (int index = 2; index <= NumAlphas; index += 2) {
                int inx1 = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(index), enclosureSurfaceNames, N);
                int inx2 = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(index + 1), enclosureSurfaceNames, N);
                if (inx1 == 0) {
                    ShowSevereError(state, format("GetInputViewFactors: {}=\"{}\", invalid surface name.", cCurrentModuleObject, EnclosureName));
                    ShowContinueError(state,
                                      format("...Surface name=\"{}\", not in this zone or enclosure.", state.dataIPShortCut->cAlphaArgs(index)));
                    ErrorsFound = true;
                }
                if (inx2 == 0) {
                    ShowSevereError(state, format("GetInputViewFactors: {}=\"{}\", invalid surface name.", cCurrentModuleObject, EnclosureName));
                    ShowContinueError(state,
                                      format("...Surface name=\"{}\", not in this zone or enclosure.", state.dataIPShortCut->cAlphaArgs(index + 2)));
                    ErrorsFound = true;
                }
                ++numinx1;
                if (inx1 > 0 && inx2 > 0) {
                    F(inx2, inx1) = state.dataIPShortCut->rNumericArgs(numinx1);
                }
            }

            enclosureSurfaceNames.deallocate();
        }
    }

    void CalcApproximateViewFactors(EnergyPlusData &state,
                                    int const N,                    // NUMBER OF SURFACES
                                    const Array1D<Real64> &A,       // AREA VECTOR- ASSUMED,BE N ELEMENTS LONG
                                    const Array1D<Real64> &Azimuth, // Facing angle of the surface (in degrees)
                                    const Array1D<Real64> &Tilt,    // Tilt angle of the surface (in degrees)
                                    Array2A<Real64> F,              // APPROXIMATE DIRECT VIEW FACTOR MATRIX (N X N)
                                    const Array1D_int &SPtr         // pointer to REAL(r64) surface number (for error message)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Curt Pedersen
        //       DATE WRITTEN   July 2000
        //       MODIFIED       March 2001 (RKS) to disallow surfaces facing the same direction to interact radiatively
        //                      May 2002 (COP) to include INTMASS, FLOOR, ROOF and CEILING.
        //       RE-ENGINEERED  September 2000 (RKS for EnergyPlus)

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine approximates view factors using an area weighting.
        // This is improved by one degree by not allowing surfaces facing the same
        // direction to "see" each other.

        // METHODOLOGY EMPLOYED:
        // Each surface sees some area of other surfaces within the zone.  The view
        // factors from the surface to the other seen surfaces are defined by their
        // area over the summed area of seen surfaces.  Surfaces facing the same angle
        // are assumed to not be able to see each other.
        //  Modified May 2002 to cover poorly defined surface orientation.  Now all thermal masses, roofs and
        //  ceilings are "seen" by other surfaces. Floors are seen by all other surfaces, but
        //  not by other floors.

        // Argument array dimensioning
        EP_SIZE_CHECK(A, N);
        EP_SIZE_CHECK(Azimuth, N);
        EP_SIZE_CHECK(Tilt, N);
        F.dim(N, N);
        EP_SIZE_CHECK(SPtr, N);

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr SameAngleLimit(10.0); // If the difference in the azimuth angles are above this value (degrees),
        // then the surfaces are assumed to be facing different directions.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int i; // DO loop counters for surfaces in the zone
        int j;
        Array1D<Real64> ZoneArea; // Sum of the area of all zone surfaces seen

        // Calculate the sum of the areas seen by all zone surfaces
        ZoneArea.dimension(N, 0.0);
        for (i = 1; i <= N; ++i) {
            for (j = 1; j <= N; ++j) {
                //  No surface sees itself and no floor sees another floor
                if (i == j || (state.dataSurface->Surface(SPtr(j)).Class == DataSurfaces::SurfaceClass::Floor &&
                               state.dataSurface->Surface(SPtr(i)).Class == DataSurfaces::SurfaceClass::Floor)) {
                    continue;
                }
                // All surface types see internal mass
                // All surface types see floors
                // Floors always see ceilings/roofs
                // All other surfaces whose tilt or facing angle differences are greater than 10 degrees see each other
                if ((state.dataSurface->Surface(SPtr(j)).Class == DataSurfaces::SurfaceClass::IntMass) ||
                    (state.dataSurface->Surface(SPtr(i)).Class == DataSurfaces::SurfaceClass::IntMass) ||
                    (state.dataSurface->Surface(SPtr(j)).Class == DataSurfaces::SurfaceClass::Floor) ||
                    (state.dataSurface->Surface(SPtr(i)).Class == DataSurfaces::SurfaceClass::Floor) ||
                    (std::abs(Azimuth(i) - Azimuth(j)) > SameAngleLimit && std::abs(Azimuth(i) - Azimuth(j)) < 360.0 - SameAngleLimit) ||
                    (std::abs(Tilt(i) - Tilt(j)) > SameAngleLimit)) {
                    ZoneArea(i) += A(j);
                }
            }
            if (ZoneArea(i) <= 0.0) {
                ShowWarningError(state, "CalcApproximateViewFactors: Zero area for all other zone surfaces.");
                ShowContinueError(state,
                                  format("Happens for Surface=\"{}\" in Zone={}",
                                         state.dataSurface->Surface(SPtr(i)).Name,
                                         state.dataHeatBal->Zone(state.dataSurface->Surface(SPtr(i)).Zone).Name));
            }
        }

        // Set up the approximate view factors.  First these are initialized to all zero.
        // This will clear out any junk leftover from whenever.  Then, for each zone
        // surface, set the view factor from that surface to other surfaces as the
        // area of the other surface divided by the sum of the area of all zone surfaces
        // that the original surface can actually see (calculated above).  This will
        // allow that the sum of all view factors from the original surface to all other
        // surfaces will equal unity.  F(I,J)=0 if I=J or if the surfaces face the same
        // direction.
        F = 0.0;
        for (i = 1; i <= N; ++i) {
            for (j = 1; j <= N; ++j) {
                //  No surface sees itself and no floor sees another floor
                if (i == j || (state.dataSurface->Surface(SPtr(j)).Class == DataSurfaces::SurfaceClass::Floor &&
                               state.dataSurface->Surface(SPtr(i)).Class == DataSurfaces::SurfaceClass::Floor)) {
                    continue;
                }
                // All surface types see internal mass
                // All surface types see floors
                // Floors always see ceilings/roofs
                // All other surfaces whose tilt or facing angle differences are greater than 10 degrees see each other
                if ((state.dataSurface->Surface(SPtr(j)).Class == DataSurfaces::SurfaceClass::IntMass) ||
                    (state.dataSurface->Surface(SPtr(i)).Class == DataSurfaces::SurfaceClass::IntMass) ||
                    (state.dataSurface->Surface(SPtr(j)).Class == DataSurfaces::SurfaceClass::Floor) ||
                    (state.dataSurface->Surface(SPtr(i)).Class == DataSurfaces::SurfaceClass::Floor) ||
                    (std::abs(Azimuth(i) - Azimuth(j)) > SameAngleLimit && std::abs(Azimuth(i) - Azimuth(j)) < 360.0 - SameAngleLimit) ||
                    (std::abs(Tilt(i) - Tilt(j)) > SameAngleLimit)) {
                    // avoid a divide by zero if there are no other surfaces in the zone that can be seen
                    if (ZoneArea(i) > 0.0) {
                        F(j, i) = A(j) / (ZoneArea(i));
                    }
                }
            }
        }

        ZoneArea.deallocate();
    }

    void FixViewFactors(EnergyPlusData &state,
                        int const N,                       // NUMBER OF SURFACES
                        const Array1D<Real64> &A,          // AREA VECTOR- ASSUMED,BE N ELEMENTS LONG
                        Array2A<Real64> F,                 // APPROXIMATE DIRECT VIEW FACTOR MATRIX (N X N)
                        std::string &enclName,             // Name of Enclosure being fixed
                        std::vector<int> const &spaceNums, // Zones which are part of this enclosure
                        Real64 &OriginalCheckValue,        // check of SUM(F) - N
                        Real64 &FixedCheckValue,           // check after fixed of SUM(F) - N
                        Real64 &FinalCheckValue,           // the one to go with
                        int &NumIterations,                // number of iterations to fixed
                        Real64 &RowSum,                    // RowSum of Fixed
                        bool const anyIntMassInZone        // are there any internal mass surfaces in the zone
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Curt Pedersen
        //       DATE WRITTEN   July 2000
        //       MODIFIED       September 2000 (RKS for EnergyPlus)
        //                      April 2005,COP added capability to handle a
        //                      surface larger than sum of all others (nonenclosure)
        //                      by using a Fii view factor for that surface. Process is
        //                      now much more robust and stable.

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine fixes approximate view factors and enforces reciprocity
        // and completeness.

        // METHODOLOGY EMPLOYED:
        // A(i)*F(i,j)=A(j)*F(j,i); F(i,i)=0.; SUM(F(i,j)=1.0, j=1,N)
        // Subroutine takes approximate view factors and enforces reciprocity by
        // averaging AiFij and AjFji.  Then it determines a set of row coefficients
        // which can be multipled by each AF product to force the sum of AiFij for
        // each row to equal Ai, and applies them. Completeness is checked, and if
        // not satisfied, the AF averaging and row modifications are repeated until
        // completeness is within a preselected small deviation from 1.0
        // The routine also checks the number of surfaces and if N<=3, just enforces reciprocity.

        // Argument array dimensioning
        EP_SIZE_CHECK(A, N);
        F.dim(N, N);

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr PrimaryConvergence(0.001);
        Real64 constexpr DifferenceConvergence(0.00001);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 ConvrgNew;
        Real64 CheckConvergeTolerance; // check value for actual warning

        bool Converged;
        // OriginalCheckValue is the first pass at a completeness check.  Even if this is zero,
        // there is no guarantee that reciprocity is satisfied.  As a result, the rest of this
        // routine is needed to correct any issues even if the user defined view factors
        // satisfy completeness (OriginalCheckValue = 0).
        OriginalCheckValue = std::abs(sum(F) - N);

        //  Allocate and zero arrays
        Array2D<Real64> FixedAF(F); // store for largest area check

        Real64 ConvrgOld = 10.0;
        Real64 LargestArea = maxval(A);
        bool severeErrorPresent = false;
        // Check for Strange Geometry
        // When one surface has an area that exceeds the sum of all other surface areas in a zone,
        // essentially the situation is a non-complete enclosure.  As a result, the view factors
        // calculated using the standard procedure below will not converge and may result in invalid
        // view factors where either reciprocity or completeness is not satisfied.  However, when
        // the largest surface is just slightly smaller than the rest of the surface areas in the
        // zone, it has been shown that there can still be problems.  The correction below can
        // be helpful in avoiding these problems.  So, the criteria below (with the 0.99 term added
        // into the comparison in the next line of code) intends to capture more cases that are
        // "unbalanced" in surface area distribution so other strange cases can take advantage of
        // this correction.  The use of 0.99 is simply to provide some reasonable boundary numerically
        // and does not have some derived theoretical basis.
        if (LargestArea > 0.99 * (sum(A) - LargestArea) && (N > 3)) {
            for (int i = 1; i <= N; ++i) {
                if (LargestArea != A(i)) {
                    continue;
                }
                state.dataHeatBalIntRadExchg->LargestSurf = i;
                break;
            }
            FixedAF(state.dataHeatBalIntRadExchg->LargestSurf, state.dataHeatBalIntRadExchg->LargestSurf) =
                min(0.9, 1.2 * LargestArea / sum(A)); // Give self view to big surface
        }

        //  Set up AF matrix.
        Array2D<Real64> AF(N, N); // = (AREA * DIRECT VIEW FACTOR) MATRIX
        for (int i = 1; i <= N; ++i) {
            for (int j = 1; j <= N; ++j) {
                AF(j, i) = FixedAF(j, i) * A(i);
            }
        }

        //  Enforce reciprocity by averaging AiFij and AjFji
        FixedAF = 0.5 * (AF + transpose(AF)); // Performance Slow way to average with transpose (heap use)

        AF.deallocate();

        Array2D<Real64> FixedF(N, N); // CORRECTED MATRIX OF VIEW FACTORS (N X N)

        NumIterations = 0;
        RowSum = 0.0;
        //  Check for physically unreasonable enclosures.

        if (N <= 3) {
            for (int i = 1; i <= N; ++i) {
                for (int j = 1; j <= N; ++j) {
                    FixedF(j, i) = FixedAF(j, i) / A(i);
                }
            }

            ShowWarningError(state, format("Surfaces in Zone/Enclosure=\"{}\" do not define an enclosure.", enclName));
            ShowContinueError(state, "Number of surfaces <= 3, view factors are set to force reciprocity but may not fulfill completeness.");
            ShowContinueError(state, "Reciprocity means that radiant exchange between two surfaces will match and not lead to an energy loss.");
            ShowContinueError(state,
                              "Completeness means that all of the view factors between a surface and the other surfaces in a zone add up to unity.");
            ShowContinueError(state,
                              "So, when there are three or less surfaces in a zone, EnergyPlus will make sure there are no losses of energy but");
            ShowContinueError(
                state, "it will not exchange the full amount of radiation with the rest of the zone as it would if there was a completed enclosure.");

            RowSum = sum(FixedF);
            if (RowSum > (N + 0.01)) {
                // Reciprocity enforced but there is more radiation than possible somewhere since the sum of one of the rows
                // is now greater than unity.  This should not be allowed as it can cause issues with the heat balance.
                // Correct this by finding the largest row summation and dividing all of the elements in the F matrix by
                // this max summation.  This will provide a cap on radiation so that no row has a sum greater than unity
                // and will still maintain reciprocity.
                Array1D<Real64> sumFixedF;
                Real64 MaxFixedFRowSum;
                sumFixedF.allocate(N);
                sumFixedF = 0.0;
                for (int i = 1; i <= N; ++i) {
                    for (int j = 1; j <= N; ++j) {
                        sumFixedF(i) += FixedF(i, j);
                    }
                    if (i == 1) {
                        MaxFixedFRowSum = sumFixedF(i);
                    } else {
                        if (sumFixedF(i) > MaxFixedFRowSum) {
                            MaxFixedFRowSum = sumFixedF(i);
                        }
                    }
                }
                sumFixedF.deallocate();
                if (MaxFixedFRowSum < 1.0) {
                    ShowFatalError(state, " FixViewFactors: Three surface or less zone failing ViewFactorFix correction which should never happen.");
                } else {
                    FixedF *= (1.0 / MaxFixedFRowSum);
                }
                RowSum = sum(FixedF); // needs to be recalculated
            }
            FinalCheckValue = FixedCheckValue = std::abs(RowSum - N);
            F = FixedF;
            for (int spaceNum : spaceNums) {
                state.dataHeatBal->Zone(state.dataHeatBal->space(spaceNum).zoneNum).EnforcedReciprocity = true;
            }
            return; // Do not iterate, stop with reciprocity satisfied.

        } //  N <= 3 Case

        //  Regular fix cases
        Array1D<Real64> RowCoefficient(N);
        Converged = false;
        while (!Converged) {
            ++NumIterations;
            for (int i = 1; i <= N; ++i) {
                // Determine row coefficients which will enforce closure.
                Real64 const sum_FixedAF_i(sum(FixedAF(_, i)));
                if (std::abs(sum_FixedAF_i) > 1.0e-10) {
                    RowCoefficient(i) = A(i) / sum_FixedAF_i;
                } else {
                    RowCoefficient(i) = 1.0;
                }
                FixedAF(_, i) *= RowCoefficient(i);
            }

            //  Enforce reciprocity by averaging AiFij and AjFji
            FixedAF = 0.5 * (FixedAF + transpose(FixedAF));

            //  Form FixedF matrix
            for (int i = 1; i <= N; ++i) {
                for (int j = 1; j <= N; ++j) {
                    FixedF(j, i) = FixedAF(j, i) / A(i);
                    if (std::abs(FixedF(j, i)) < 1.e-10) {
                        FixedF(j, i) = 0.0;
                        FixedAF(j, i) = 0.0;
                    }
                }
            }

            ConvrgNew = std::abs(sum(FixedF) - N);
            if (std::abs(ConvrgOld - ConvrgNew) < DifferenceConvergence || ConvrgNew <= PrimaryConvergence) { //  Change in sum of Fs must be small.
                Converged = true;
            }
            ConvrgOld = ConvrgNew;
            if (NumIterations > 400) { //  If everything goes bad,enforce reciprocity and go home.
                //  Enforce reciprocity by averaging AiFij and AjFji
                FixedAF = 0.5 * (FixedAF + transpose(FixedAF));

                //  Form FixedF matrix
                for (int i = 1; i <= N; ++i) {
                    for (int j = 1; j <= N; ++j) {
                        FixedF(j, i) = FixedAF(j, i) / A(i);
                    }
                }
                Real64 const sum_FixedF(sum(FixedF));
                FinalCheckValue = FixedCheckValue = CheckConvergeTolerance = std::abs(sum_FixedF - N);
                RowSum = sum_FixedF;
                if (CheckConvergeTolerance > 0.005) {
                    if (CheckConvergeTolerance > 0.1) {
                        ShowSevereError(
                            state,
                            format("FixViewFactors: View factors convergence has failed and will lead to heat balance errors in zone=\"{}\".",
                                   enclName));
                    }
                    ShowWarningError(
                        state,
                        format("FixViewFactors: View factors not complete. Check for bad surface descriptions or unenclosed zone=\"{}\".", enclName));
                    ShowContinueError(state,
                                      format("Enforced reciprocity has tolerance (ideal is "
                                             "0)=[{:.6R}], Row Sum (ideal is {})=[{:.2R}].",
                                             CheckConvergeTolerance,
                                             N,
                                             RowSum));
                    ShowContinueError(state,
                                      "If zone is unusual or tolerance is on the order of 0.001, view "
                                      "factors might be OK but results should be checked carefully.");
                    if (anyIntMassInZone) {
                        ShowContinueError(state,
                                          "For zones with internal mass like this one, this"
                                          "can happen when the internal mass has an area that"
                                          "is much larger than the other surfaces in the zone.");
                        ShowContinueError(state,
                                          "If a single thermal mass element exists in this zone"
                                          "that has an area that is larger than the sum of the"
                                          "rest of the surface areas, consider breaking it up"
                                          "into two or more separate internal mass elements.");
                    }
                }
                if (std::abs(FixedCheckValue) < std::abs(OriginalCheckValue)) {
                    F = FixedF;
                    FinalCheckValue = FixedCheckValue;
                }
                return;
            }
        }
        FixedCheckValue = ConvrgNew;
        if (FixedCheckValue < OriginalCheckValue) {
            F = FixedF;
            FinalCheckValue = FixedCheckValue;
        } else {
            FinalCheckValue = OriginalCheckValue;
            RowSum = sum(FixedF);
            if (std::abs(RowSum - N) < PrimaryConvergence) {
                F = FixedF;
                FinalCheckValue = FixedCheckValue;
            } else {
                ShowWarningError(
                    state,
                    format("FixViewFactors: View factors not complete. Check for bad surface descriptions or unenclosed zone=\"{}\".", enclName));
            }
        }
        if (severeErrorPresent) {
            ShowFatalError(state,
                           "FixViewFactors: View factor calculations significantly out "
                           "of tolerance.  See above messages for more information.");
        }
    }

    bool DoesZoneHaveInternalMass(EnergyPlusData &state, int const numZoneSurfaces, const Array1D_int &surfPointer)
    {
        for (int i = 1; i <= numZoneSurfaces; ++i) {
            if (state.dataSurface->Surface(surfPointer(i)).Class == DataSurfaces::SurfaceClass::IntMass) {
                return true;
            }
        }
        return false;
    }

    void CalcScriptF(EnergyPlusData &state,
                     int const N,              // Number of surfaces
                     Array1D<Real64> const &A, // AREA VECTOR- ASSUMED,BE N ELEMENTS LONG
                     Array2<Real64> const &F,  // DIRECT VIEW FACTOR MATRIX (N X N)
                     Array1D<Real64> &EMISS,   // VECTOR OF SURFACE EMISSIVITIES
                     Array2<Real64> &ScriptF   // MATRIX OF SCRIPT F FACTORS (N X N) //Tuned Transposed
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Curt Pedersen
        //       DATE WRITTEN   1980
        //       MODIFIED       July 2000 (COP for the ASHRAE Loads Toolkit)
        //       RE-ENGINEERED  September 2000 (RKS for EnergyPlus)
        //       RE-ENGINEERED  June 2014 (Stuart Mentzer): Performance tuned

        // PURPOSE OF THIS SUBROUTINE:
        // Determines Hottel's ScriptF coefficients which account for the total
        // grey interchange between surfaces in an enclosure.

        // REFERENCES:
        // Hottel, H. C. and A. F. Sarofim, Radiative Transfer, Ch 3, McGraw Hill, 1967.

        // SUBROUTINE ARGUMENTS:
        // --Must satisfy reciprocity and completeness:
        //  A(i)*F(i,j)=A(j)*F(j,i); F(i,i)=0.; SUM(F(i,j)=1.0, j=1,N)

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr MaxEmissLimit(0.99999); // Limit the emissivity internally/avoid a divide by zero error

        // Validate argument array dimensions
        assert(N >= 0); // Do we need to allow for N==0?
        assert((A.l() == 1) && (A.u() == N));
        assert((F.l1() == 1) && (F.u1() == N));
        assert((F.l2() == 1) && (F.u2() == N));
        assert((EMISS.l() == 1) && (EMISS.u() == N));
        assert(equal_dimensions(F, ScriptF));

#ifdef EP_Count_Calls
        ++state.dataTimingsData->NumCalcScriptF_Calls;
#endif

        // Load Cmatrix with AF (AREA * DIRECT VIEW FACTOR) matrix
        Array2D<Real64> Cmatrix(N, N);        // = (AF - EMISS/REFLECTANCE) matrix (but plays other roles)
        assert(equal_dimensions(Cmatrix, F)); // For linear indexing
        Array2D<Real64>::size_type l(0u);
        for (int j = 1; j <= N; ++j) {
            for (int i = 1; i <= N; ++i, ++l) {
                Cmatrix[l] = A(i) * F[l]; // [ l ] == ( i, j )
            }
        }

        // Load Cmatrix with (AF - EMISS/REFLECTANCE) matrix
        Array1D<Real64> Excite(N); // Excitation vector = A*EMISS/REFLECTANCE
        l = 0u;
        for (int i = 1; i <= N; ++i, l += N + 1) {
            Real64 EMISS_i(EMISS(i));
            if (EMISS_i > MaxEmissLimit) { // Check/limit EMISS for this surface to avoid divide by zero below
                EMISS_i = EMISS(i) = MaxEmissLimit;
                ShowWarningError(state, "A thermal emissivity above 0.99999 was detected. This is not allowed. Value was reset to 0.99999");
            }
            Real64 const EMISS_i_fac(A(i) / (1.0 - EMISS_i));
            Excite(i) = -EMISS_i * EMISS_i_fac; // Set up matrix columns for partial radiosity calculation
            Cmatrix[l] -= EMISS_i_fac;          // Coefficient matrix for partial radiosity calculation // [ l ] == ( i, i )
        }

        Array2D<Real64> Cinverse(N, N);       // Inverse of Cmatrix
        CalcMatrixInverse(Cmatrix, Cinverse); // SOLVE THE LINEAR SYSTEM
        Cmatrix.clear();                      // Release memory ASAP

        // Scale Cinverse colums by excitation to get partial radiosity matrix
        l = 0u;
        for (int j = 1; j <= N; ++j) {
            Real64 const e_j(Excite(j));
            for (int i = 1; i <= N; ++i, ++l) {
                Cinverse[l] *= e_j; // [ l ] == ( i, j )
            }
        }
        Excite.clear(); // Release memory ASAP

        // Form Script F matrix transposed
        assert(equal_dimensions(Cinverse, ScriptF)); // For linear indexing
        Array2D<Real64>::size_type m(0u);
        for (int i = 1; i <= N; ++i) { // Inefficient order for cache but can reuse multiplier so faster choice depends on N
            Real64 const EMISS_i(EMISS(i));
            Real64 const EMISS_fac(EMISS_i / (1.0 - EMISS_i));
            l = static_cast<Array2D<Real64>::size_type>(i - 1);
            for (int j = 1; j <= N; ++j, l += N, ++m) {
                if (i == j) {
                    //        ScriptF(I,J) = EMISS(I)/(1.0d0-EMISS(I))*(Jmatrix(I,J)-Delta*EMISS(I)), where Delta=1
                    ScriptF[m] = EMISS_fac * (Cinverse[l] - EMISS_i); // [ l ] = ( i, j ), [ m ] == ( j, i )
                } else {
                    //        ScriptF(I,J) = EMISS(I)/(1.0d0-EMISS(I))*(Jmatrix(I,J)-Delta*EMISS(I)), where Delta=0
                    ScriptF[m] = EMISS_fac * Cinverse[l]; // [ l ] == ( i, j ), [ m ] == ( j, i )
                }
            }
        }
    }

    void CalcMatrixInverse(Array2<Real64> &A, // Matrix: Gets reduced to L\U form
                           Array2<Real64> &I  // Returned as inverse matrix
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Jakob Asmundsson
        //       DATE WRITTEN   January 1999
        //       MODIFIED       September 2000 (RKS for EnergyPlus)
        //       RE-ENGINEERED  June 2014 (Stuart Mentzer): Performance/memory tuning rewrite

        // PURPOSE OF THIS SUBROUTINE:
        // To find the inverse of Matrix, using partial pivoting.

        // METHODOLOGY EMPLOYED:
        // Inverse is found using partial pivoting and Gauss elimination

        // REFERENCES:
        // Any Linear Algebra book

        // Validation
        assert(A.square());
        assert(A.I1() == A.I2());
        assert(equal_dimensions(A, I));

        // Initialization
        int const l(A.l1());
        int const u(A.u1());
        int const n(u - l + 1);
        I.to_identity(); // I starts out as identity

        // Could do row scaling here to improve condition and then check min pivot isn't too small

        // Compute in-place LU decomposition of [A|I] with row pivoting
        for (int i = l; i <= u; ++i) {

            // Find pivot row in column i below diagonal
            int iPiv = i;
            Real64 aPiv(std::abs(A(i, i)));
            int ik(A.index(i, i + 1));
            for (int k = i + 1; k <= u; ++k, ++ik) {
                Real64 const aAki(std::abs(A[ik])); // [ ik ] == ( i, k )
                if (aAki > aPiv) {
                    iPiv = k;
                    aPiv = aAki;
                }
            }
            assert(aPiv != 0.0); //? Is zero pivot possible for some user inputs? If so if test/handler needed

            // Swap row i with pivot row
            if (iPiv != i) {
                int ji(A.index(l, i));    // [ ji ] == ( j, i )
                int pj(A.index(l, iPiv)); // [ pj ] == ( j, iPiv )
                for (int j = l; j <= u; ++j, ji += n, pj += n) {
                    Real64 const Aij(A[ji]);
                    A[ji] = A[pj];
                    A[pj] = Aij;
                    Real64 const Iij(I[ji]);
                    I[ji] = I[pj];
                    I[pj] = Iij;
                }
            }

            // Put multipliers in column i and reduce block below A(i,i)
            Real64 const Aii_inv(1.0 / A(i, i));
            for (int k = i + 1; k <= u; ++k) {
                Real64 const multiplier(A(i, k) * Aii_inv);
                A(i, k) = multiplier;
                if (multiplier != 0.0) {
                    int ji(A.index(i + 1, i)); // [ ji ] == ( j, i )
                    int jk(A.index(i + 1, k)); // [ jk ] == ( j, k )
                    for (int j = i + 1; j <= u; ++j, ji += n, jk += n) {
                        A[jk] -= multiplier * A[ji];
                    }
                    ji = A.index(l, i);
                    jk = A.index(l, k);
                    for (int j = l; j <= u; ++j, ji += n, jk += n) {
                        Real64 const Iij(I[ji]);
                        if (Iij != 0.0) {
                            I[jk] -= multiplier * Iij;
                        }
                    }
                }
            }
        }

        // Perform back-substitution on [U|I] to put inverse in I
        for (int k = u; k >= l; --k) {
            Real64 const Akk_inv(1.0 / A(k, k));
            int jk(A.index(l, k)); // [ jk ] == ( j, k )
            for (int j = l; j <= u; ++j, jk += n) {
                I[jk] *= Akk_inv;
            }
            int ik(A.index(k, l));              // [ ik ] == ( i, k )
            for (int i = l; i < k; ++i, ++ik) { // Eliminate kth column entries from I in rows above k
                Real64 const Aik(A[ik]);
                int ji(A.index(l, i)); // [ ji ] == ( j, i )
                int jm(A.index(l, k)); // [ jm ] == ( k, j )
                for (int j = l; j <= u; ++j, ji += n, jm += n) {
                    I[ji] -= Aik * I[jm];
                }
            }
        }
    }

    void CalcFMRT(EnergyPlusData &state,
                  int const N,              // Number of surfaces
                  Array1D<Real64> const &A, // AREA VECTOR- ASSUMED,BE N ELEMENTS LONG
                  Array1D<Real64> &FMRT     // VECTOR OF MEAN RADIANT TEMPERATURE "VIEW FACTORS"
    )
    {
        double sumAF = 0.0;
        for (int iS = 0; iS < N; iS++) {
            FMRT[iS] = 1.0;
            sumAF += A[iS];
        }

        constexpr int maxIt = 100;
        constexpr double tol = 0.0001;
        double fLast;
        double sumAFNew = sumAF;
        for (unsigned i = 0; i < maxIt; i++) {
            double fChange = 0.;
            bool errorsFound(false);
            sumAF = sumAFNew;
            sumAFNew = 0.0;
            for (int iS = 0; iS < N; iS++) {
                fLast = FMRT[iS];
                FMRT[iS] = 1. / (1. - A[iS] * FMRT[iS] / (sumAF));
                if (FMRT[iS] > 100.) {
                    errorsFound = true;
                    ShowSevereError(state, "Geometry not compatible with Carroll MRT Zone Radiant Exchange method.");
                    break;
                }
                fChange += fabs(FMRT[iS] - fLast);
                sumAFNew += A[iS] * FMRT[iS];
            }

            if (errorsFound || fChange / N < tol) {
                break;
            }
            if (i >= maxIt) {
                errorsFound = true;
                ShowSevereError(state, "Carroll MRT Zone Radiant Exchange method unable to converge on \"view factor\" calculation.");
            }
            if (errorsFound) {
                ShowFatalError(state, "CalcFMRT: Errors found while calculating mean radiant temperature view factors.  Program terminated.");
            }
        }
        return;
    }

    void CalcFp(int const N,                  // Number of surfaces
                Array1D<Real64> const &EMISS, // VECTOR OF SURFACE EMISSIVITIES
                Array1D<Real64> const &FMRT,  // VECTOR OF MEAN RADIANT TEMPERATURE "VIEW FACTORS"
                Array1D<Real64> &Fp           // VECTOR OF OPPENHEIM RESISTANCE VALUES
    )
    {
        for (int iS = 0; iS < N; iS++) {
            Fp[iS] = Constant::StefanBoltzmann * EMISS[iS] / (EMISS[iS] / FMRT[iS] + 1. - EMISS[iS]); // actually sigma *
        }
        return;
    }

    int GetRadiantSystemSurface(EnergyPlusData &state,
                                std::string const &cCurrentModuleObject, // Calling Object type
                                std::string const &RadSysName,           // Calling Object name
                                int const RadSysZoneNum,                 // Radiant system zone number
                                std::string const &SurfaceName,          // Referenced surface name
                                bool &ErrorsFound                        // True when errors are found
    )
    {
        static constexpr std::string_view routineName("GetRadiantSystemSurface: "); // include trailing blank space

        // For radiant zone equipment, find the referenced surface and check if it is in the same zone or radiant enclosure
        int const surfNum = Util::FindItemInList(SurfaceName, state.dataSurface->Surface);

        // Trap for surfaces that do not exist
        if (surfNum == 0) {
            ShowSevereError(state, format("{}Invalid Surface name = {}", routineName, SurfaceName));
            ShowContinueError(state, format("Occurs for {} = {}", cCurrentModuleObject, RadSysName));
            ErrorsFound = true;
            return surfNum;
        }

        if (RadSysZoneNum == 0) {
            ShowSevereError(state, format("{}Invalid Zone number passed by {} = {}", routineName, cCurrentModuleObject, RadSysName));
            ErrorsFound = true;
            return surfNum;
        }

        // Check if the surface and equipment are in the same zone
        int const surfZoneNum = state.dataSurface->Surface(surfNum).Zone;
        if (surfZoneNum == 0) {
            // This should never happen
            ShowSevereError(state,
                            format("{}Somehow  the surface zone number is zero for{} = {} and Surface = {}",
                                   routineName,
                                   cCurrentModuleObject,
                                   RadSysName,
                                   SurfaceName)); // LCOV_EXCL_LINE
            ErrorsFound = true;                   // LCOV_EXCL_LINE
        } else if (surfZoneNum != RadSysZoneNum) {
            ShowSevereError(state, format("{}Surface = {} is not in the same zone  as the radiant equipment.", routineName, SurfaceName));
            ShowContinueError(state, format("Surface zone or enclosure = {}", state.dataHeatBal->Zone(surfZoneNum).Name));
            ShowContinueError(state, format("Radiant equipment zone or enclosure = {}", state.dataHeatBal->Zone(RadSysZoneNum).Name));
            ShowContinueError(state, format("Occurs for {} = {}", cCurrentModuleObject, RadSysName));
            ErrorsFound = true;
        }
        return surfNum;
    }

} // namespace HeatBalanceIntRadExchange

} // namespace EnergyPlus

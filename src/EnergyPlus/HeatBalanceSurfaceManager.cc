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
// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Array2D.hh>
#include <ObjexxFCL/Fmath.hh>
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/ChilledCeilingPanelSimple.hh>
#include <EnergyPlus/Construction.hh>
#include <EnergyPlus/ConvectionCoefficients.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataDaylighting.hh>
#include <EnergyPlus/DataDaylightingDevices.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalSurface.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataMoistureBalance.hh>
#include <EnergyPlus/DataMoistureBalanceEMPD.hh>
#include <EnergyPlus/DataRoomAirModel.hh>
#include <EnergyPlus/DataRuntimeLanguage.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataSurfaces.hh>
#include <EnergyPlus/DataSystemVariables.hh>
#include <EnergyPlus/DataViewFactorInformation.hh>
#include <EnergyPlus/DataWindowEquivalentLayer.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DaylightingDevices.hh>
#include <EnergyPlus/DaylightingManager.hh>
#include <EnergyPlus/DisplayRoutines.hh>
#include <EnergyPlus/EcoRoofManager.hh>
#include <EnergyPlus/ElectricBaseboardRadiator.hh>
#include <EnergyPlus/ExtendedHeatIndex.hh>
#include <EnergyPlus/FileSystem.hh>
#include <EnergyPlus/HWBaseboardRadiator.hh>
#include <EnergyPlus/HeatBalFiniteDiffManager.hh>
#include <EnergyPlus/HeatBalanceAirManager.hh>
#include <EnergyPlus/HeatBalanceHAMTManager.hh>
#include <EnergyPlus/HeatBalanceIntRadExchange.hh>
#include <EnergyPlus/HeatBalanceKivaManager.hh>
#include <EnergyPlus/HeatBalanceSurfaceManager.hh>
#include <EnergyPlus/HighTempRadiantSystem.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/InternalHeatGains.hh>
#include <EnergyPlus/LowTempRadiantSystem.hh>
#include <EnergyPlus/MoistureBalanceEMPDManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/OutputReportTabular.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SolarShading.hh>
#include <EnergyPlus/SteamBaseboardRadiator.hh>
#include <EnergyPlus/SurfaceGeometry.hh>
#include <EnergyPlus/SwimmingPool.hh>
#include <EnergyPlus/ThermalComfort.hh>
#include <EnergyPlus/TranspiredCollector.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WindowComplexManager.hh>
#include <EnergyPlus/WindowEquivalentLayer.hh>
#include <EnergyPlus/WindowManager.hh>
#include <EnergyPlus/WindowManagerExteriorData.hh>
#include <EnergyPlus/WindowManagerExteriorThermal.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>
#include <WCECommon.hpp>
#include <WCEMultiLayerOptics.hpp>
#include <WCESingleLayerOptics.hpp>
#include <WCETarcog.hpp>

namespace EnergyPlus::HeatBalanceSurfaceManager {

// Module containing the routines dealing with the Heat Balance of the surfaces

// MODULE INFORMATION:
//       MODIFIED       DJS (PSU Dec 2006) to add ecoroof

// PURPOSE OF THIS MODULE:
// To encapsulate the data and algorithms required to
// manage the simulation of the surface heat balance for the building.

// REFERENCES:
// The heat balance method is outlined in the "TARP Reference Manual", NIST, NBSIR 83-2655, Feb 1983.
// The methods are also summarized in many BSO Theses and papers.

// OTHER NOTES:
// This module was created from IBLAST subroutines

void ManageSurfaceHeatBalance(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   January 1998

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine manages the heat surface balance method of calculating
    // building thermal loads.  It is called from the HeatBalanceManager
    // at the time step level.  This driver manages the calls to all of
    // the other drivers and simulation algorithms.

    if (state.dataHeatBalSurfMgr->ManageSurfaceHeatBalancefirstTime) {
        DisplayString(state, "Initializing Surfaces");
    }
    InitSurfaceHeatBalance(state); // Initialize all heat balance related parameters

    // Solve the zone heat balance 'Detailed' solution
    // Call the outside and inside surface heat balances
    if (state.dataHeatBalSurfMgr->ManageSurfaceHeatBalancefirstTime) {
        DisplayString(state, "Calculate Outside Surface Heat Balance");
    }
    CalcHeatBalanceOutsideSurf(state);
    if (state.dataHeatBalSurfMgr->ManageSurfaceHeatBalancefirstTime) {
        DisplayString(state, "Calculate Inside Surface Heat Balance");
    }
    CalcHeatBalanceInsideSurf(state);

    // The air heat balance must be called before the temperature history
    // updates because there may be a radiant system in the building
    if (state.dataHeatBalSurfMgr->ManageSurfaceHeatBalancefirstTime) {
        DisplayString(state, "Calculate Air Heat Balance");
    }
    HeatBalanceAirManager::ManageAirHeatBalance(state);

    // IF NECESSARY, do one final "average" heat balance pass.  This is only
    // necessary if a radiant system is present and it was actually on for
    // part or all of the time step.
    UpdateFinalSurfaceHeatBalance(state);

    // Before we leave the Surface Manager the thermal histories need to be updated
    if (state.dataHeatBal->AnyCTF || state.dataHeatBal->AnyEMPD) {
        UpdateThermalHistories(state); // Update the thermal histories
    }

    if (state.dataHeatBal->AnyCondFD) {
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            auto const &surface = state.dataSurface->Surface(SurfNum);
            int const ConstrNum = surface.Construction;
            if (ConstrNum <= 0) {
                continue; // Shading surface, not really a heat transfer surface
            }
            if (state.dataConstruction->Construct(ConstrNum).TypeIsWindow) {
                continue; //  Windows simulated in Window module
            }
            if (surface.HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::CondFD) {
                continue;
            }
            state.dataHeatBalFiniteDiffMgr->SurfaceFD(SurfNum).UpdateMoistureBalance();
        }
    }

    ThermalComfort::ManageThermalComfort(state, false); // "Record keeping" for the zone

    ReportSurfaceHeatBalance(state);
    if (state.dataGlobal->ZoneSizingCalc) {
        OutputReportTabular::GatherComponentLoadsSurface(state);
    }

    CalcThermalResilience(state);

    if (state.dataOutRptTab->displayThermalResilienceSummary) {
        ReportThermalResilience(state);
    }

    if (state.dataOutRptTab->displayCO2ResilienceSummary) {
        ReportCO2Resilience(state);
    }

    if (state.dataOutRptTab->displayVisualResilienceSummary) {
        ReportVisualResilience(state);
    }

    state.dataHeatBalSurfMgr->ManageSurfaceHeatBalancefirstTime = false;
}

// Beginning Initialization Section of the Module
//******************************************************************************

void UpdateVariableAbsorptances(EnergyPlusData &state)
{
    auto &s_mat = state.dataMaterial;
    for (int surfNum : state.dataSurface->AllVaryAbsOpaqSurfaceList) {
        auto const &thisConstruct = state.dataConstruction->Construct(state.dataSurface->Surface(surfNum).Construction);
        auto const *thisMaterial = s_mat->materials(thisConstruct.LayerPoint(1));
        assert(thisMaterial != nullptr);
        if (thisMaterial->absorpVarCtrlSignal == Material::VariableAbsCtrlSignal::Scheduled) {
            if (thisMaterial->absorpThermalVarSched != nullptr) {
                state.dataHeatBalSurf->SurfAbsThermalExt(surfNum) = max(min(thisMaterial->absorpThermalVarSched->getCurrentVal(), 0.9999), 0.0001);
            }
            if (thisMaterial->absorpSolarVarSched != nullptr) {
                state.dataHeatBalSurf->SurfAbsSolarExt(surfNum) = max(min(thisMaterial->absorpThermalVarSched->getCurrentVal(), 0.9999), 0.0001);
            }
        } else {
            Real64 triggerValue;
            if (thisMaterial->absorpVarCtrlSignal == Material::VariableAbsCtrlSignal::SurfaceTemperature) {
                triggerValue = state.dataHeatBalSurf->SurfTempOut(surfNum);
            } else if (thisMaterial->absorpVarCtrlSignal == Material::VariableAbsCtrlSignal::SurfaceReceivedSolarRadiation) {
                triggerValue = state.dataHeatBal->SurfQRadSWOutIncident(surfNum);
            } else { // controlled by heating cooling mode
                int zoneNum = state.dataSurface->Surface(surfNum).Zone;
                bool isCooling = (state.dataZoneEnergyDemand->ZoneSysEnergyDemand(zoneNum).TotalOutputRequired < 0);
                triggerValue = static_cast<Real64>(isCooling);
            }
            if (thisMaterial->absorpThermalVarCurve != nullptr) {
                state.dataHeatBalSurf->SurfAbsThermalExt(surfNum) =
                    max(min(thisMaterial->absorpThermalVarCurve->value(state, triggerValue), 0.9999), 0.0001);
            }
            if (thisMaterial->absorpSolarVarCurve != nullptr) {
                state.dataHeatBalSurf->SurfAbsSolarExt(surfNum) =
                    max(min(thisMaterial->absorpSolarVarCurve->value(state, triggerValue), 0.9999), 0.0001);
            }
        }
    }
}

void InitSurfaceHeatBalance(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard J. Liesen
    //       DATE WRITTEN   January 1998
    //       MODIFIED       Nov. 1999, FCW,
    //                      Move ComputeIntThermalAbsorpFactors
    //                      so called every timestep
    //       MODIFIED       Aug. 2017
    //                      Add initializations of surface data to linked air node value if defined

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine is for surface initializations within the
    // heat balance.

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger record keeping events.

    //    // Using/Aliasing
    //    using namespace SolarShading;
    //    using HeatBalanceIntRadExchange::CalcInteriorRadExchange;
    //    using HeatBalFiniteDiffManager::InitHeatBalFiniteDiff;
    //    using InternalHeatGains::ManageInternalHeatGains;
    //
    //    auto &Surface = state.dataSurface->Surface;
    //
    if (state.dataHeatBalSurfMgr->InitSurfaceHeatBalancefirstTime) {
        DisplayString(state, "Initializing Outdoor environment for Surfaces");
    }

    // set zone level wind dir to global value
    // Initialize zone outdoor environmental variables
    // Bulk Initialization for Temperatures & WindSpeed
    // using the zone, modify the zone  Dry/Wet BulbTemps

    // Initialize surface outdoor environmental variables
    // Bulk Initialization for Temperatures & WindSpeed
    // using the surface centroids, modify the surface Dry/Wet BulbTemps
    DataSurfaces::SetSurfaceOutBulbTempAt(state);
    DataSurfaces::CheckSurfaceOutBulbTempAt(state);

    DataSurfaces::SetSurfaceWindSpeedAt(state);
    DataSurfaces::SetSurfaceWindDirAt(state);
    if (state.dataGlobal->AnyLocalEnvironmentsInModel) {
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            if (state.dataSurface->Surface(SurfNum).SurfLinkedOutAirNode > 0) {
                auto const &linkedNode = state.dataLoopNodes->Node(state.dataSurface->Surface(SurfNum).SurfLinkedOutAirNode);
                state.dataSurface->SurfOutDryBulbTemp(SurfNum) = linkedNode.OutAirDryBulb;
                state.dataSurface->SurfOutWetBulbTemp(SurfNum) = linkedNode.OutAirWetBulb;
                state.dataSurface->SurfOutWindSpeed(SurfNum) = linkedNode.OutAirWindSpeed;
                state.dataSurface->SurfOutWindDir(SurfNum) = linkedNode.OutAirWindDir;
            }
        }
    }
    // Overwriting surface and zone level environmental data with EMS override value
    if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            if (state.dataSurface->SurfOutDryBulbTempEMSOverrideOn(SurfNum)) {
                state.dataSurface->SurfOutDryBulbTemp(SurfNum) = state.dataSurface->SurfOutDryBulbTempEMSOverrideValue(SurfNum);
            }
            if (state.dataSurface->SurfOutWetBulbTempEMSOverrideOn(SurfNum)) {
                state.dataSurface->SurfOutWetBulbTemp(SurfNum) = state.dataSurface->SurfOutWetBulbTempEMSOverrideValue(SurfNum);
            }
            if (state.dataSurface->SurfWindSpeedEMSOverrideOn(SurfNum)) {
                state.dataSurface->SurfOutWindSpeed(SurfNum) = state.dataSurface->SurfWindSpeedEMSOverrideValue(SurfNum);
            }
            if (state.dataSurface->SurfWindDirEMSOverrideOn(SurfNum)) {
                state.dataSurface->SurfOutWindDir(SurfNum) = state.dataSurface->SurfWindDirEMSOverrideValue(SurfNum);
            }
            if (state.dataSurface->SurfViewFactorGroundEMSOverrideOn(SurfNum)) {
                state.dataSurface->Surface(SurfNum).ViewFactorGround = state.dataSurface->SurfViewFactorGroundEMSOverrideValue(SurfNum);
            }
        }
    }

    // Do the Begin Simulation initializations
    if (state.dataGlobal->BeginSimFlag) {
        AllocateSurfaceHeatBalArrays(state); // Allocate the Module Arrays before any inits take place
        state.dataHeatBalSurf->InterZoneWindow =
            std::any_of(state.dataViewFactor->EnclSolInfo.begin(),
                        state.dataViewFactor->EnclSolInfo.end(),
                        [](DataViewFactorInformation::EnclosureViewFactorInformation const &e) { return e.HasInterZoneWindow; });
    }
    if (state.dataGlobal->BeginSimFlag || state.dataGlobal->AnySurfPropOverridesInModel) {
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                int const firstSurf = thisSpace.HTSurfaceFirst;
                int const lastSurf = thisSpace.HTSurfaceLast;
                for (int SurfNum = firstSurf; SurfNum <= lastSurf; ++SurfNum) {
                    int ConstrNum = state.dataSurface->SurfActiveConstruction(SurfNum); // SurfActiveConstruction set above
                    auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);
                    state.dataHeatBalSurf->SurfAbsSolarInt(SurfNum) = thisConstruct.InsideAbsorpSolar;
                    state.dataHeatBalSurf->SurfAbsThermalInt(SurfNum) = thisConstruct.InsideAbsorpThermal;
                    state.dataHeatBalSurf->SurfRoughnessExt(SurfNum) = thisConstruct.OutsideRoughness;
                    state.dataHeatBalSurf->SurfAbsSolarExt(SurfNum) = thisConstruct.OutsideAbsorpSolar;
                    state.dataHeatBalSurf->SurfAbsThermalExt(SurfNum) = thisConstruct.OutsideAbsorpThermal;
                }
            }
        }
    }

    // variable thermal solar absorptance overrides
    UpdateVariableAbsorptances(state);

    // Do the Begin Environment initializations
    if (state.dataGlobal->BeginEnvrnFlag) {
        if (state.dataHeatBalSurfMgr->InitSurfaceHeatBalancefirstTime) {
            DisplayString(state, "Initializing Temperature and Flux Histories");
        }
        InitThermalAndFluxHistories(state); // Set initial temperature and flux histories
    }

    // Calc movable insulation properties
    if (state.dataSurface->AnyMovableInsulation) {
        EvalOutsideMovableInsulation(state);
        EvalInsideMovableInsulation(state);
    }

    // There are no daily initializations done in this portion of the surface heat balance
    // There are no hourly initializations done in this portion of the surface heat balance

    GetGroundSurfacesReflectanceAverage(state);

    // Need to be called each timestep in order to check if surface points to new construction (EMS) and if does then
    // complex fenestration needs to be initialized for additional states
    SolarShading::TimestepInitComplexFenestration(state);

    // Calculate exterior-surface multipliers that account for anisotropy of
    // sky radiance
    if (state.dataEnvrn->SunIsUp && state.dataEnvrn->DifSolarRad > 0.0) {
        SolarShading::AnisoSkyViewFactors(state);
    } else {
        state.dataSolarShading->SurfAnisoSkyMult = 0.0;
    }

    // Set shading flag for exterior windows (except flags related to daylighting) and
    // window construction (unshaded or shaded) to be used in heat balance calculation
    if (state.dataHeatBalSurfMgr->InitSurfaceHeatBalancefirstTime) {
        DisplayString(state, "Initializing Window Shading");
    }

    SolarShading::WindowShadingManager(state);

    SolarShading::CheckGlazingShadingStatusChange(state);

    // Calculate factors that are used to determine how much long-wave radiation from internal
    // gains is absorbed by interior surfaces
    if (state.dataHeatBalSurfMgr->InitSurfaceHeatBalancefirstTime) {
        DisplayString(state, "Computing Interior Absorption Factors");
    }
    if (state.dataHeatBalSurfMgr->InitSurfaceHeatBalancefirstTime) {
        HeatBalanceIntRadExchange::InitInteriorRadExchange(state);
    }
    ComputeIntThermalAbsorpFactors(state);

    // Calculate factors for diffuse solar absorbed by room surfaces and interior shades
    if (state.dataHeatBalSurfMgr->InitSurfaceHeatBalancefirstTime) {
        DisplayString(state, "Computing Interior Diffuse Solar Absorption Factors");
    }
    ComputeIntSWAbsorpFactors(state);

    if (state.dataHeatBalSurf->InterZoneWindow) {
        if (state.dataHeatBalSurfMgr->InitSurfaceHeatBalancefirstTime) {
            DisplayString(state, "Computing Interior Diffuse Solar Exchange through Interzone Windows");
        }
        ComputeDifSolExcZonesWIZWindows(state);
    }

    Dayltg::initDaylighting(state, state.dataHeatBalSurfMgr->InitSurfaceHeatBalancefirstTime);

    HeatBalanceIntRadExchange::CalcInteriorRadExchange(
        state, state.dataHeatBalSurf->SurfInsideTempHist(1), 0, state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea, _, "Main");

    if (state.dataSurface->AirflowWindows) {
        SolarShading::WindowGapAirflowControl(state);
    }

    // The order of these initializations is important currently.  Over time we hope to
    //  take the appropriate parts of these inits to the other heat balance managers
    if (state.dataHeatBalSurfMgr->InitSurfaceHeatBalancefirstTime) {
        DisplayString(state, "Initializing Solar Heat Gains");
    }

    InitSolarHeatGains(state);

    Dayltg::manageDaylighting(state);

    if (state.dataHeatBalSurfMgr->InitSurfaceHeatBalancefirstTime) {
        DisplayString(state, "Initializing Internal Heat Gains");
    }
    InternalHeatGains::ManageInternalHeatGains(state, false);
    if (state.dataHeatBalSurfMgr->InitSurfaceHeatBalancefirstTime) {
        DisplayString(state, "Initializing Interior Solar Distribution");
    }
    InitIntSolarDistribution(state);

    if (state.dataHeatBalSurfMgr->InitSurfaceHeatBalancefirstTime) {
        DisplayString(state, "Initializing Interior Convection Coefficients");
    }
    Convect::InitIntConvCoeff(state, state.dataHeatBalSurf->SurfTempInTmp);

    if (state.dataGlobal->BeginSimFlag) { // Now's the time to report surfaces, if desired
        //    if (firstTime) CALL DisplayString('Reporting Surfaces')
        //    CALL ReportSurfaces
        if (state.dataHeatBalSurfMgr->InitSurfaceHeatBalancefirstTime) {
            DisplayString(state, "Gathering Information for Predefined Reporting");
        }
        GatherForPredefinedReport(state);
    }

    // Initialize the temperature history terms for conduction through the surfaces
    if (state.dataHeatBal->AnyCondFD) {
        HeatBalFiniteDiffManager::InitHeatBalFiniteDiff(state);
    }

    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) { // Loop through all surfaces...
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurfOpaque = thisSpace.OpaqOrIntMassSurfaceFirst;
            int const lastSurfOpaque = thisSpace.OpaqOrIntMassSurfaceLast;
            for (int SurfNum = firstSurfOpaque; SurfNum <= lastSurfOpaque; ++SurfNum) {
                auto const &surface = state.dataSurface->Surface(SurfNum);
                if (surface.HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::CTF &&
                    surface.HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::EMPD) {
                    continue;
                }
                // Outside surface temp of "normal" windows not needed in Window5 calculation approach
                // Window layer temperatures are calculated in CalcHeatBalanceInsideSurf

                int const ConstrNum = surface.Construction;
                auto const &construct = state.dataConstruction->Construct(ConstrNum);
                state.dataHeatBalSurf->SurfCTFConstOutPart(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfCTFConstInPart(SurfNum) = 0.0;
                if (construct.NumCTFTerms <= 1) {
                    continue;
                }

                for (int Term = 1; Term <= construct.NumCTFTerms; ++Term) {
                    // [ l11 ] == ( 1, Term + 1, SurfNum ), [ l12 ] == ( 1, Term + 1, SurfNum )

                    // Sign convention for the various terms in the following two equations
                    // is based on the form of the Conduction Transfer Function equation
                    // given by:
                    // Qin,now  = (Sum of)(Y Tout) - (Sum of)(Z Tin) + (Sum of)(F Qin,old)
                    // Qout,now = (Sum of)(X Tout) - (Sum of)(Y Tin) + (Sum of)(F Qout,old)
                    // In both equations, flux is positive from outside to inside.

                    // Tuned Aliases and linear indexing
                    Real64 const ctf_cross(construct.CTFCross[Term]);

                    Real64 const TH11(state.dataHeatBalSurf->SurfOutsideTempHist(Term + 1)(SurfNum));
                    Real64 const TH12(state.dataHeatBalSurf->SurfInsideTempHist(Term + 1)(SurfNum));
                    Real64 const QH11(state.dataHeatBalSurf->SurfOutsideFluxHist(Term + 1)(SurfNum));
                    Real64 const QH12(state.dataHeatBalSurf->SurfInsideFluxHist(Term + 1)(SurfNum));
                    state.dataHeatBalSurf->SurfCTFConstInPart(SurfNum) +=
                        ctf_cross * TH11 - construct.CTFInside[Term] * TH12 + construct.CTFFlux[Term] * QH12;

                    state.dataHeatBalSurf->SurfCTFConstOutPart(SurfNum) +=
                        construct.CTFOutside[Term] * TH11 - ctf_cross * TH12 + construct.CTFFlux[Term] * QH11;
                }
            }
        }
    }
    if (state.dataHeatBal->AnyInternalHeatSourceInInput) {
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) { // Loop through all surfaces...
            for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                int const firstSurfOpaque = thisSpace.OpaqOrIntMassSurfaceFirst;
                int const lastSurfOpaque = thisSpace.OpaqOrIntMassSurfaceLast;
                for (int SurfNum = firstSurfOpaque; SurfNum <= lastSurfOpaque; ++SurfNum) {
                    auto const &surface = state.dataSurface->Surface(SurfNum);
                    int const ConstrNum = surface.Construction;
                    auto const &construct = state.dataConstruction->Construct(ConstrNum);
                    if (!construct.SourceSinkPresent) {
                        continue;
                    }
                    if (surface.HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::CTF &&
                        surface.HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::EMPD) {
                        continue;
                    }
                    state.dataHeatBalFanSys->CTFTsrcConstPart(SurfNum) = 0.0;
                    state.dataHeatBalFanSys->CTFTuserConstPart(SurfNum) = 0.0;
                    if (construct.NumCTFTerms <= 1) {
                        continue;
                    }
                    for (int Term = 1; Term <= construct.NumCTFTerms; ++Term) {
                        Real64 const TH11(state.dataHeatBalSurf->SurfOutsideTempHist(Term + 1)(SurfNum));
                        Real64 const TH12(state.dataHeatBalSurf->SurfInsideTempHist(Term + 1)(SurfNum));
                        Real64 const QsrcHist1(state.dataHeatBalSurf->SurfQsrcHist(SurfNum, Term + 1));

                        state.dataHeatBalSurf->SurfCTFConstInPart(SurfNum) += construct.CTFSourceIn[Term] * QsrcHist1;

                        state.dataHeatBalSurf->SurfCTFConstOutPart(SurfNum) += construct.CTFSourceOut[Term] * QsrcHist1;

                        state.dataHeatBalFanSys->CTFTsrcConstPart(SurfNum) +=
                            construct.CTFTSourceOut[Term] * TH11 + construct.CTFTSourceIn[Term] * TH12 + construct.CTFTSourceQ[Term] * QsrcHist1 +
                            construct.CTFFlux[Term] * state.dataHeatBalSurf->SurfTsrcHist(SurfNum, Term + 1);

                        state.dataHeatBalFanSys->CTFTuserConstPart(SurfNum) +=
                            construct.CTFTUserOut[Term] * TH11 + construct.CTFTUserIn[Term] * TH12 + construct.CTFTUserSource[Term] * QsrcHist1 +
                            construct.CTFFlux[Term] * state.dataHeatBalSurf->SurfTuserHist(SurfNum, Term + 1);
                    }
                }
            } // ...end of surfaces DO loop for initializing temperature history terms for the surface heat balances
        }
    }

    // Zero out all of the radiant system heat balance coefficient arrays
    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) { // Loop through all surfaces...
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurf = thisSpace.HTSurfaceFirst;
            int const lastSurf = thisSpace.HTSurfaceLast;
            for (int SurfNum = firstSurf; SurfNum <= lastSurf; ++SurfNum) {
                state.dataHeatBalFanSys->RadSysTiHBConstCoef(SurfNum) = 0.0;
                state.dataHeatBalFanSys->RadSysTiHBToutCoef(SurfNum) = 0.0;
                state.dataHeatBalFanSys->RadSysTiHBQsrcCoef(SurfNum) = 0.0;
                state.dataHeatBalFanSys->RadSysToHBConstCoef(SurfNum) = 0.0;
                state.dataHeatBalFanSys->RadSysToHBTinCoef(SurfNum) = 0.0;
                state.dataHeatBalFanSys->RadSysToHBQsrcCoef(SurfNum) = 0.0;

                state.dataHeatBalFanSys->QRadSysSource(SurfNum) = 0.0;
                state.dataHeatBalFanSys->QPVSysSource(SurfNum) = 0.0;
                state.dataHeatBalFanSys->QPoolSurfNumerator(SurfNum) = 0.0;
                state.dataHeatBalFanSys->PoolHeatTransCoefs(SurfNum) = 0.0;

            } // ...end of Zone Surf loop
        }
    } // ...end of Zone loop

    for (int surfNum : state.dataSurface->allGetsRadiantHeatSurfaceList) {
        auto &thisSurfQRadFromHVAC = state.dataHeatBalFanSys->surfQRadFromHVAC(surfNum);
        thisSurfQRadFromHVAC.HTRadSys = 0.0;
        thisSurfQRadFromHVAC.HWBaseboard = 0.0;
        thisSurfQRadFromHVAC.SteamBaseboard = 0.0;
        thisSurfQRadFromHVAC.ElecBaseboard = 0.0;
        thisSurfQRadFromHVAC.CoolingPanel = 0.0;
    }

    if (state.dataGlobal->ZoneSizingCalc) {
        GatherComponentLoadsSurfAbsFact(state);
    }

    if (state.dataHeatBalSurfMgr->InitSurfaceHeatBalancefirstTime) {
        DisplayString(state, "Completed Initializing Surface Heat Balance");
    }
    state.dataHeatBalSurfMgr->InitSurfaceHeatBalancefirstTime = false;
}

void GatherForPredefinedReport(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Jason Glazer
    //       DATE WRITTEN   August 2006

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine reports the information for the predefined reports
    // related to envelope components.

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    std::string surfName;
    Real64 mult;
    Real64 curAzimuth;
    Real64 curTilt;
    Real64 windowArea;
    Real64 frameWidth;
    Real64 frameArea;
    Real64 dividerArea;
    // counts for object count report
    int SurfaceClassCount = int(DataSurfaces::SurfaceClass::Num);
    Array1D_int numSurfaces(SurfaceClassCount);
    Array1D_int numExtSurfaces(SurfaceClassCount);
    int frameDivNum;
    bool isExterior;
    Array1D<Real64> computedNetArea; // holds the gross wall area minus the window and door areas

    // the following variables are for the CalcNominalWindowCond call but only SHGCSummer is needed
    Real64 nomCond;
    Real64 SHGCSummer;
    Real64 TransSolNorm;
    Real64 TransVisNorm;
    Real64 nomUfact;
    int errFlag;
    int curWSC;
    // following variables are totals for fenestration table
    Real64 windowAreaWMult(0.0);
    Real64 fenTotArea(0.0);
    Real64 fenTotAreaNorth(0.0);
    Real64 fenTotAreaNonNorth(0.0);
    Real64 ufactArea(0.0);
    Real64 ufactAreaNorth(0.0);
    Real64 ufactAreaNonNorth(0.0);
    Real64 shgcArea(0.0);
    Real64 shgcAreaNorth(0.0);
    Real64 shgcAreaNonNorth(0.0);
    Real64 vistranArea(0.0);
    Real64 vistranAreaNorth(0.0);
    Real64 vistranAreaNonNorth(0.0);
    Real64 intFenTotArea(0.0);
    Real64 intUfactArea(0.0);
    Real64 intShgcArea(0.0);
    Real64 intVistranArea(0.0);
    bool isNorth;

    constexpr std::array<std::string_view, static_cast<int>(DataSurfaces::WinShadingType::Num)> WindowShadingTypeNames = {
        "No Shade",  // 0
        "Shade Off", // 1
        "Interior Shade",
        "Switchable Glazing",
        "Exterior Shade",
        "Exterior Screen",
        "Interior Blind",
        "Exterior Blind",
        "Between Glass Shade",
        "Between Glass Blind",
    };

    constexpr std::array<std::string_view, static_cast<int>(DataSurfaces::WindowShadingControlType::Num)> WindowShadingControlTypeNames = {
        "Uncontrolled",
        "AlwaysOn",
        "AlwaysOff",
        "OnIfScheduleAllows",
        "OnIfHighSolarOnWindow",
        "OnIfHighHorizontalSolar",
        "OnIfHighOutdoorAirTemperature",
        "OnIfHighZoneAirTemperature",
        "OnIfHighZoneCooling",
        "OnIfHighGlare",
        "MeetDaylightIlluminanceSetpoint",
        "OnNightIfLowOutdoorTempAndOffDay",
        "OnNightIfLowInsideTempAndOffDay",
        "OnNightIfHeatingAndOffDay",
        "OnNightIfLowOutdoorTempAndOnDayIfCooling",
        "OnNightIfHeatingAndOnDayIfCooling",
        "OffNightAndOnDayIfCoolingAndHighSolarOnWindow",
        "OnNightAndOnDayIfCoolingAndHighSolarOnWindow",
        "OnIfHighOutdoorAirTempAndHighSolarOnWindow",
        "OnIfHighOutdoorAirTempAndHighHorizontalSolar",
        "OnIfHighZoneAirTempAndHighSolarOnWindow",
        "OnIfHighZoneAirTempAndHighHorizontalSolar"};

    constexpr std::array<std::string_view, static_cast<int>(DataSurfaces::NfrcProductOptions::Num)> NfrcProductNames = {
        "CasementDouble", "CasementSingle",   "DualAction",
        "Fixed",          "Garage",           "Greenhouse",
        "HingedEscape",   "HorizontalSlider", "Jal",
        "Pivoted",        "ProjectingSingle", "ProjectingDual",
        "DoorSidelite",   "Skylight",         "SlidingPatioDoor",
        "CurtainWall",    "SpandrelPanel",    "SideHingedDoor",
        "DoorTransom",    "TropicalAwning",   "TubularDaylightingDevice",
        "VerticalSlider"};

    constexpr std::array<Real64, static_cast<int>(DataSurfaces::NfrcProductOptions::Num)> NfrcWidth = {
        // width in meters from Table 4-3 of NFRC 100-2020
        1.200, 0.600, 1.200, //  CasementDouble,  CasementSingle,    DualAction,
        1.200, 2.134, 1.500, //  Fixed,           Garage,            Greenhouse,
        1.500, 1.500, 1.200, //  HingedEscape,    HorizontalSlider,  Jal,
        1.200, 1.500, 1.500, //  Pivoted,         ProjectingSingle,  ProjectingDual,
        0.600, 1.200, 2.000, //  DoorSidelite,    Skylight,          SlidingPatioDoor,
        2.000, 2.000, 1.920, //  CurtainWall,     SpandrelPanel,     SideHingedDoor,
        2.000, 1.500, 0.350, //  DoorTransom,     TropicalAwning,    TubularDaylightingDevice,
        1.200                //  VerticalSlider,
    };

    constexpr std::array<Real64, static_cast<int>(DataSurfaces::NfrcProductOptions::Num)> NfrcHeight = {
        // height in meters from Table 4-3 of NFRC 100-2020
        1.500, 1.500, 1.500, //  CasementDouble,  CasementSingle,    DualAction,
        1.500, 2.134, 1.200, //  Fixed,           Garage,            Greenhouse,
        1.200, 1.200, 1.500, //  HingedEscape,    HorizontalSlider,  Jal,
        1.500, 1.200, 0.600, //  Pivoted,         ProjectingSingle,  ProjectingDual,
        2.090, 1.200, 2.000, //  DoorSidelite,    Skylight,          SlidingPatioDoor,
        2.000, 1.200, 2.090, //  CurtainWall,     SpandrelPanel,     SideHingedDoor,
        0.600, 1.200, 0.350, //  DoorTransom,     TropicalAwning,    TubularDaylightingDevice,
        1.500                //  VerticalSlider,
    };

    constexpr std::array<DataSurfaces::NfrcVisionType, static_cast<int>(DataSurfaces::NfrcProductOptions::Num)> NfrcVision = {
        DataSurfaces::NfrcVisionType::DualHorizontal, DataSurfaces::NfrcVisionType::Single,
        DataSurfaces::NfrcVisionType::DualVertical, //  CasementDouble,  CasementSingle,    DualAction,
        DataSurfaces::NfrcVisionType::Single,         DataSurfaces::NfrcVisionType::Single,
        DataSurfaces::NfrcVisionType::Single, //  Fixed,           Garage,            Greenhouse,
        DataSurfaces::NfrcVisionType::Single,         DataSurfaces::NfrcVisionType::DualHorizontal,
        DataSurfaces::NfrcVisionType::Single, //  HingedEscape,    HorizontalSlider,  Jal,
        DataSurfaces::NfrcVisionType::Single,         DataSurfaces::NfrcVisionType::Single,
        DataSurfaces::NfrcVisionType::DualHorizontal, //  Pivoted,         ProjectingSingle,  ProjectingDual,
        DataSurfaces::NfrcVisionType::Single,         DataSurfaces::NfrcVisionType::Single,
        DataSurfaces::NfrcVisionType::DualHorizontal, //  DoorSidelite,    Skylight,          SlidingPatioDoor,
        DataSurfaces::NfrcVisionType::Single,         DataSurfaces::NfrcVisionType::Single,
        DataSurfaces::NfrcVisionType::Single, //  CurtainWall,     SpandrelPanel,     SideHingedDoor,
        DataSurfaces::NfrcVisionType::Single,         DataSurfaces::NfrcVisionType::Single,
        DataSurfaces::NfrcVisionType::Single,      //  DoorTransom,     TropicalAwning,    TubularDaylightingDevice,
        DataSurfaces::NfrcVisionType::DualVertical //  VerticalSlider
    };

    numSurfaces = 0;
    numExtSurfaces = 0;

    computedNetArea.allocate(state.dataSurface->TotSurfaces);
    computedNetArea = 0.0; // start at zero, add wall area and subtract window and door area

    // set up for EIO <FenestrationAssembly> output
    if (state.dataHeatBal->TotFrameDivider > 0 && state.dataGeneral->Constructions) {
        print(state.files.eio,
              "{}\n",
              "! <FenestrationAssembly>,Construction Name,Frame and Divider Name,NFRC Product Type,"
              "Assembly U-Factor {W/m2-K},Assembly SHGC,Assembly Visible Transmittance");
    }
    static constexpr std::string_view FenestrationAssemblyFormat("FenestrationAssembly,{},{},{},{:.3R},{:.3R},{:.3R}\n");
    std::vector<std::pair<int, int>> uniqConsFrame;
    std::pair<int, int> consAndFrame;

    // set up for EIO <FenestrationShadedState> output
    bool fenestrationShadedStateHeaderShown(false);
    static constexpr std::string_view FenestrationShadedStateFormat("FenestrationShadedState,{},{:.3R},{:.3R},{:.3R},{},{},{:.3R},{:.3R},{:.3R}\n");
    std::vector<std::pair<int, int>> uniqShdConsFrame;
    std::pair<int, int> shdConsAndFrame;

    for (int iSurf : state.dataSurface->AllSurfaceListReportOrder) {
        auto &surface = state.dataSurface->Surface(iSurf);
        surfName = surface.Name;
        // only exterior surfaces including underground
        if ((surface.ExtBoundCond == DataSurfaces::ExternalEnvironment) || (surface.ExtBoundCond == DataSurfaces::Ground) ||
            (surface.ExtBoundCond == DataSurfaces::GroundFCfactorMethod) || (surface.ExtBoundCond == DataSurfaces::KivaFoundation)) {
            isExterior = true;
            switch (surface.Class) {
            case DataSurfaces::SurfaceClass::Wall:
            case DataSurfaces::SurfaceClass::Floor:
            case DataSurfaces::SurfaceClass::Roof: {
                auto const &construct = state.dataConstruction->Construct(surface.Construction);
                auto const &thisZone = state.dataHeatBal->Zone(surface.Zone);
                mult = thisZone.Multiplier * thisZone.ListMultiplier;
                auto const &thisSpace = state.dataHeatBal->space(surface.spaceNum);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchOpCons, surfName, construct.Name);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchOpZone, surfName, thisZone.Name);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchOpSpace, surfName, thisSpace.Name);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchOpRefl, surfName, 1 - construct.OutsideAbsorpSolar);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchOpUfactNoFilm, surfName, state.dataHeatBal->NominalU(surface.Construction), 3);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchOpGrArea, surfName, surface.GrossArea * mult);
                computedNetArea(iSurf) += surface.GrossArea * mult;
                curAzimuth = surface.Azimuth;
                // Round to two decimals, like the display in tables
                // (PreDefTableEntry uses a fortran style write, that rounds rather than trim)
                curAzimuth = round(curAzimuth * 100.0) / 100.0;
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchOpAzimuth, surfName, curAzimuth);
                curTilt = surface.Tilt;
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchOpTilt, surfName, curTilt);
                if ((curTilt >= 60.0) && (curTilt < 180.0)) {
                    if ((curAzimuth >= 315.0) || (curAzimuth < 45.0)) {
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchOpDir, surfName, "N");
                    } else if ((curAzimuth >= 45.0) && (curAzimuth < 135.0)) {
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchOpDir, surfName, "E");
                    } else if ((curAzimuth >= 135.0) && (curAzimuth < 225.0)) {
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchOpDir, surfName, "S");
                    } else if ((curAzimuth >= 225.0) && (curAzimuth < 315.0)) {
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchOpDir, surfName, "W");
                    }
                }
            } break;
            case DataSurfaces::SurfaceClass::Window:
            case DataSurfaces::SurfaceClass::TDD_Dome: {
                auto &construct = state.dataConstruction->Construct(surface.Construction);
                auto const &thisZone = state.dataHeatBal->Zone(surface.Zone);
                mult = thisZone.Multiplier * thisZone.ListMultiplier * surface.Multiplier;
                auto const &thisSpace = state.dataHeatBal->space(surface.spaceNum);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenCons, surfName, construct.Name);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenZone, surfName, thisZone.Name);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenSpace, surfName, thisSpace.Name);
                // if the construction report is requested the SummerSHGC is already calculated
                if (construct.SummerSHGC != 0) {
                    SHGCSummer = construct.SummerSHGC;
                    TransVisNorm = construct.VisTransNorm;
                } else {
                    // must calculate Summer SHGC
                    if (!construct.WindowTypeEQL) {
                        Window::CalcNominalWindowCond(state, surface.Construction, 2, nomCond, SHGCSummer, TransSolNorm, TransVisNorm, errFlag);
                        construct.SummerSHGC = SHGCSummer;
                        construct.VisTransNorm = TransVisNorm;
                        construct.SolTransNorm = TransSolNorm;
                    }
                }
                // include the frame area if present
                windowArea = surface.GrossArea;
                frameArea = 0.0;
                dividerArea = 0.0;
                frameDivNum = surface.FrameDivider;
                if (frameDivNum != 0) {
                    auto const &frameDivider = state.dataSurface->FrameDivider(frameDivNum);
                    frameWidth = frameDivider.FrameWidth;
                    frameArea = (surface.Height + 2.0 * frameWidth) * (surface.Width + 2.0 * frameWidth) - (surface.Height * surface.Width);
                    windowArea += frameArea;
                    dividerArea = frameDivider.DividerWidth * (frameDivider.HorDividers * surface.Width + frameDivider.VertDividers * surface.Height -
                                                               frameDivider.HorDividers * frameDivider.VertDividers * frameDivider.DividerWidth);
                    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenFrameDivName, surfName, frameDivider.Name);
                    OutputReportPredefined::PreDefTableEntry(
                        state, state.dataOutRptPredefined->pdchFenFrameConductance, surfName, frameDivider.FrameConductance, 3);
                    OutputReportPredefined::PreDefTableEntry(
                        state, state.dataOutRptPredefined->pdchFenDividerConductance, surfName, frameDivider.DividerConductance, 3);

                    // report the selected NRFC product type (specific sizes) and the NFRC rating for the assembly (glass + frame + divider)
                    std::string_view NFRCname = NfrcProductNames[static_cast<int>(frameDivider.NfrcProductType)];
                    const Real64 windowWidth = NfrcWidth[static_cast<int>(frameDivider.NfrcProductType)];
                    const Real64 windowHeight = NfrcHeight[static_cast<int>(frameDivider.NfrcProductType)];
                    const DataSurfaces::NfrcVisionType vision = NfrcVision[static_cast<int>(frameDivider.NfrcProductType)];

                    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenAssemNfrcType, surfName, NFRCname);

                    Real64 uValueAssembly = 0.0;
                    Real64 shgcAssembly = 0.0;
                    Real64 vtAssembly = 0.0;

                    Window::GetWindowAssemblyNfrcForReport(
                        state, iSurf, surface.Construction, windowWidth, windowHeight, vision, uValueAssembly, shgcAssembly, vtAssembly);
                    if (state.dataWindowManager->inExtWindowModel->isExternalLibraryModel()) {
                        state.dataHeatBal->NominalU(surface.Construction) =
                            Window::GetIGUUValueForNFRCReport(state, iSurf, surface.Construction, windowWidth, windowHeight);
                        SHGCSummer = Window::GetSHGCValueForNFRCReporting(state, iSurf, surface.Construction, windowWidth, windowHeight);
                    }
                    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenAssemUfact, surfName, uValueAssembly, 3);
                    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenAssemSHGC, surfName, shgcAssembly, 3);
                    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenAssemVisTr, surfName, vtAssembly, 3);

                    // output EIO <FenestrationAssembly> for each unique combination of construction and frame/divider
                    if (state.dataGeneral->Constructions) {
                        consAndFrame = std::make_pair(surface.Construction, frameDivNum);
                        if (std::find(uniqConsFrame.begin(), uniqConsFrame.end(), consAndFrame) == uniqConsFrame.end()) {
                            uniqConsFrame.push_back(consAndFrame);
                            print(state.files.eio,
                                  FenestrationAssemblyFormat,
                                  construct.Name,
                                  frameDivider.Name,
                                  NFRCname,
                                  uValueAssembly,
                                  shgcAssembly,
                                  vtAssembly);
                        }
                    }
                }
                windowAreaWMult = windowArea * mult;
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenAreaOf1, surfName, windowArea);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenFrameAreaOf1, surfName, frameArea);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenDividerAreaOf1, surfName, dividerArea);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchFenGlassAreaOf1, surfName, windowArea - (frameArea + dividerArea));
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenArea, surfName, windowAreaWMult);
                computedNetArea(surface.BaseSurf) -= windowAreaWMult;
                nomUfact = state.dataHeatBal->NominalU(surface.Construction);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenUfact, surfName, nomUfact, 3);

                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenSHGC, surfName, SHGCSummer, 3);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenVisTr, surfName, TransVisNorm, 3);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenParent, surfName, surface.BaseSurfName);
                curAzimuth = surface.Azimuth;
                // Round to two decimals, like the display in tables
                curAzimuth = round(curAzimuth * 100.0) / 100.0;
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenAzimuth, surfName, curAzimuth);
                isNorth = false;
                curTilt = surface.Tilt;
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenTilt, surfName, curTilt);
                if ((curTilt >= 60.0) && (curTilt < 180.0)) {
                    if ((curAzimuth >= 315.0) || (curAzimuth < 45.0)) {
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenDir, surfName, "N");
                        isNorth = true;
                    } else if ((curAzimuth >= 45.0) && (curAzimuth < 135.0)) {
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenDir, surfName, "E");
                    } else if ((curAzimuth >= 135.0) && (curAzimuth < 225.0)) {
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenDir, surfName, "S");
                    } else if ((curAzimuth >= 225.0) && (curAzimuth < 315.0)) {
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenDir, surfName, "W");
                    }
                }

                // Report table for every shading control state
                const unsigned int totalStates = surface.windowShadingControlList.size();
                if (frameDivNum != 0) {
                    auto const &frameDivider = state.dataSurface->FrameDivider(frameDivNum);
                    for (unsigned int i = 0; i < totalStates; ++i) {
                        const Real64 windowWidth = NfrcWidth[static_cast<int>(frameDivider.NfrcProductType)];
                        const Real64 windowHeight = NfrcHeight[static_cast<int>(frameDivider.NfrcProductType)];
                        const DataSurfaces::NfrcVisionType vision = NfrcVision[static_cast<int>(frameDivider.NfrcProductType)];

                        const int stateConstrNum = surface.shadedConstructionList[i];
                        const Real64 stateUValue = Window::GetIGUUValueForNFRCReport(state, iSurf, stateConstrNum, windowWidth, windowHeight);
                        const Real64 stateSHGC = Window::GetSHGCValueForNFRCReporting(state, iSurf, stateConstrNum, windowWidth, windowHeight);
                        std::string const &constructionName = state.dataConstruction->Construct(stateConstrNum).Name;

                        OutputReportPredefined::PreDefTableEntry(
                            state, state.dataOutRptPredefined->pdchFenShdFrameDiv, constructionName, frameDivider.Name);
                        OutputReportPredefined::PreDefTableEntry(
                            state, state.dataOutRptPredefined->pdchFenShdUfact, constructionName, stateUValue, 3);
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenShdSHGC, constructionName, stateSHGC, 3);
                        OutputReportPredefined::PreDefTableEntry(state,
                                                                 state.dataOutRptPredefined->pdchFenShdVisTr,
                                                                 constructionName,
                                                                 state.dataConstruction->Construct(stateConstrNum).VisTransNorm,
                                                                 3);

                        Real64 stateAssemblyUValue{0.0};
                        Real64 stateAssemblySHGC{0.0};
                        Real64 stateAssemblyVT{0.0};

                        Window::GetWindowAssemblyNfrcForReport(
                            state, iSurf, stateConstrNum, windowWidth, windowHeight, vision, stateAssemblyUValue, stateAssemblySHGC, stateAssemblyVT);

                        std::string_view NFRCname = NfrcProductNames[static_cast<int>(frameDivider.NfrcProductType)];
                        OutputReportPredefined::PreDefTableEntry(
                            state, state.dataOutRptPredefined->pdchFenShdAssemNfrcType, constructionName, NFRCname);

                        OutputReportPredefined::PreDefTableEntry(
                            state, state.dataOutRptPredefined->pdchFenShdAssemUfact, constructionName, stateAssemblyUValue, 3);
                        OutputReportPredefined::PreDefTableEntry(
                            state, state.dataOutRptPredefined->pdchFenShdAssemSHGC, constructionName, stateAssemblySHGC, 3);
                        OutputReportPredefined::PreDefTableEntry(
                            state, state.dataOutRptPredefined->pdchFenShdAssemVisTr, constructionName, stateAssemblyVT, 3);

                        if (state.dataGeneral->Constructions) {
                            if (!fenestrationShadedStateHeaderShown) {
                                print(state.files.eio,
                                      "{}\n",
                                      "! <FenestrationShadedState>,Construction Name,Glass U-Factor {W/m2-K},"
                                      "Glass SHGC, Glass Visible Transmittance, Frame and Divider Name,NFRC Product Type,"
                                      "Assembly U-Factor {W/m2-K},Assembly SHGC,Assembly Visible Transmittance");
                                fenestrationShadedStateHeaderShown = true;
                            }

                            shdConsAndFrame = std::make_pair(stateConstrNum, frameDivNum);
                            if (std::find(uniqShdConsFrame.begin(), uniqShdConsFrame.end(), shdConsAndFrame) == uniqShdConsFrame.end()) {
                                uniqShdConsFrame.push_back(shdConsAndFrame);
                                print(state.files.eio,
                                      FenestrationShadedStateFormat,
                                      constructionName,
                                      stateUValue,
                                      stateSHGC,
                                      state.dataConstruction->Construct(stateConstrNum).VisTransNorm,
                                      frameDivider.Name,
                                      NFRCname,
                                      stateAssemblyUValue,
                                      stateAssemblySHGC,
                                      stateAssemblyVT);
                            }
                        }
                    }
                }

                curWSC = surface.activeWindowShadingControl;
                // compute totals for area weighted averages
                fenTotArea += windowAreaWMult;
                ufactArea += nomUfact * windowAreaWMult;
                shgcArea += SHGCSummer * windowAreaWMult;
                vistranArea += TransVisNorm * windowAreaWMult;
                if (isNorth) {
                    fenTotAreaNorth += windowAreaWMult;
                    ufactAreaNorth += nomUfact * windowAreaWMult;
                    shgcAreaNorth += SHGCSummer * windowAreaWMult;
                    vistranAreaNorth += TransVisNorm * windowAreaWMult;
                } else {
                    fenTotAreaNonNorth += windowAreaWMult;
                    ufactAreaNonNorth += nomUfact * windowAreaWMult;
                    shgcAreaNonNorth += SHGCSummer * windowAreaWMult;
                    vistranAreaNonNorth += TransVisNorm * windowAreaWMult;
                }
                // shading
                if (surface.HasShadeControl) {
                    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenSwitchable, surfName, "Yes");
                    OutputReportPredefined::PreDefTableEntry(
                        state, state.dataOutRptPredefined->pdchWscName, surfName, state.dataSurface->WindowShadingControl(curWSC).Name);
                    // shading report
                    OutputReportPredefined::PreDefTableEntry(
                        state,
                        state.dataOutRptPredefined->pdchWscShading,
                        surfName,
                        WindowShadingTypeNames[int(state.dataSurface->WindowShadingControl(curWSC).ShadingType)]);
                    OutputReportPredefined::PreDefTableEntry(
                        state,
                        state.dataOutRptPredefined->pdchWscControl,
                        surfName,
                        WindowShadingControlTypeNames[int(state.dataSurface->WindowShadingControl(curWSC).shadingControlType)]);

                    // output list of all possible shading constructions for shaded windows including those with storms
                    std::string names;
                    for (int construction : surface.shadedConstructionList) {
                        if (!names.empty()) {
                            names.append("; ");
                        }
                        names.append(state.dataConstruction->Construct(construction).Name);
                    }
                    for (int construction : surface.shadedStormWinConstructionList) {
                        if (!names.empty()) {
                            names.append("; ");
                        }
                        names.append(state.dataConstruction->Construct(construction).Name);
                    }
                    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchWscShadCons, surfName, names);

                    if (state.dataSurface->WindowShadingControl(curWSC).GlareControlIsActive) {
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchWscGlare, surfName, "Yes");
                    } else {
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchWscGlare, surfName, "No");
                    }
                } else {
                    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenSwitchable, surfName, "No");
                }
            } break;
            case DataSurfaces::SurfaceClass::Door: {
                auto const &thisZone = state.dataHeatBal->Zone(surface.Zone);
                mult = thisZone.Multiplier * thisZone.ListMultiplier;
                auto const &thisSpace = state.dataHeatBal->space(surface.spaceNum);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchDrCons, surfName, state.dataConstruction->Construct(surface.Construction).Name);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchDrZone, surfName, thisZone.Name);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchDrSpace, surfName, thisSpace.Name);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchDrUfactNoFilm, surfName, state.dataHeatBal->NominalU(surface.Construction), 3);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchDrGrArea, surfName, surface.GrossArea * mult);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchDrParent, surfName, surface.BaseSurfName);
                computedNetArea(surface.BaseSurf) -= surface.GrossArea * mult;
            } break;
            default:
                break;
            }
        } else {
            // interior surfaces
            isExterior = false;
            if ((surface.Class == DataSurfaces::SurfaceClass::Wall) || (surface.Class == DataSurfaces::SurfaceClass::Floor) ||
                (surface.Class == DataSurfaces::SurfaceClass::Roof) || (surface.Class == DataSurfaces::SurfaceClass::IntMass)) {
                auto const &construct = state.dataConstruction->Construct(surface.Construction);
                auto const &thisZone = state.dataHeatBal->Zone(surface.Zone);
                mult = thisZone.Multiplier * thisZone.ListMultiplier;
                auto const &thisSpace = state.dataHeatBal->space(surface.spaceNum);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntOpCons, surfName, construct.Name);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntOpZone, surfName, thisZone.Name);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntOpSpace, surfName, thisSpace.Name);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntOpAdjSurf, surfName, surface.ExtBoundCondName);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchIntOpRefl, surfName, 1 - construct.OutsideAbsorpSolar);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchIntOpUfactNoFilm, surfName, state.dataHeatBal->NominalU(surface.Construction), 3);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntOpGrArea, surfName, surface.GrossArea * mult);
                computedNetArea(iSurf) += surface.GrossArea * mult;
                curAzimuth = surface.Azimuth;
                // Round to two decimals, like the display in tables
                // (PreDefTableEntry uses a fortran style write, that rounds rather than trim)
                curAzimuth = round(curAzimuth * 100.0) / 100.0;
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntOpAzimuth, surfName, curAzimuth);
                curTilt = surface.Tilt;
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntOpTilt, surfName, curTilt);
                if ((curTilt >= 60.0) && (curTilt < 180.0)) {
                    if ((curAzimuth >= 315.0) || (curAzimuth < 45.0)) {
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntOpDir, surfName, "N");
                    } else if ((curAzimuth >= 45.0) && (curAzimuth < 135.0)) {
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntOpDir, surfName, "E");
                    } else if ((curAzimuth >= 135.0) && (curAzimuth < 225.0)) {
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntOpDir, surfName, "S");
                    } else if ((curAzimuth >= 225.0) && (curAzimuth < 315.0)) {
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntOpDir, surfName, "W");
                    }
                }
                // interior window report
            } else if ((surface.Class == DataSurfaces::SurfaceClass::Window) || (surface.Class == DataSurfaces::SurfaceClass::TDD_Dome)) {
                auto const &construct = state.dataConstruction->Construct(surface.Construction);
                auto const &thisZone = state.dataHeatBal->Zone(surface.Zone);
                mult = thisZone.Multiplier * thisZone.ListMultiplier * surface.Multiplier;
                auto const &thisSpace = state.dataHeatBal->space(surface.spaceNum);
                if (!has_prefix(surface.Name,
                                "iz-")) { // don't count created interzone surfaces that are mirrors of other surfaces
                    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntFenCons, surfName, construct.Name);
                    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntFenZone, surfName, thisZone.Name);
                    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntFenSpace, surfName, thisSpace.Name);
                    // include the frame area if present
                    windowArea = surface.GrossArea;
                    if (surface.FrameDivider != 0) {
                        frameWidth = state.dataSurface->FrameDivider(surface.FrameDivider).FrameWidth;
                        frameArea = (surface.Height + 2 * frameWidth) * (surface.Width + 2 * frameWidth) - (surface.Height * surface.Width);
                        windowArea += frameArea;
                    }
                    windowAreaWMult = windowArea * mult;
                    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntFenAreaOf1, surfName, windowArea);
                    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntFenArea, surfName, windowAreaWMult);
                    computedNetArea(surface.BaseSurf) -= windowAreaWMult;
                    nomUfact = state.dataHeatBal->NominalU(surface.Construction);
                    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntFenUfact, surfName, nomUfact, 3);
                    if (!construct.TypeIsAirBoundary) {
                        // Solar properties not applicable for air boundary surfaces
                        // if the construction report is requested the SummerSHGC is already calculated
                        if (construct.SummerSHGC != 0) {
                            SHGCSummer = construct.SummerSHGC;
                            TransVisNorm = construct.VisTransNorm;
                        } else {
                            // must calculate Summer SHGC
                            if (!construct.WindowTypeEQL) {
                                Window::CalcNominalWindowCond(
                                    state, surface.Construction, 2, nomCond, SHGCSummer, TransSolNorm, TransVisNorm, errFlag);
                            }
                        }
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntFenSHGC, surfName, SHGCSummer, 3);
                        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntFenVisTr, surfName, TransVisNorm, 3);
                        // compute totals for area weighted averages
                        intShgcArea += SHGCSummer * windowAreaWMult;
                        intVistranArea += TransVisNorm * windowAreaWMult;
                    }
                    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntFenParent, surfName, surface.BaseSurfName);
                    // compute totals for area weighted averages
                    intFenTotArea += windowAreaWMult;
                    intUfactArea += nomUfact * windowAreaWMult;
                }
            } else if (surface.Class == DataSurfaces::SurfaceClass::Door) {
                auto const &construct = state.dataConstruction->Construct(surface.Construction);
                auto const &thisZone = state.dataHeatBal->Zone(surface.Zone);
                mult = thisZone.Multiplier * thisZone.ListMultiplier;
                auto const &thisSpace = state.dataHeatBal->space(surface.spaceNum);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntDrCons, surfName, construct.Name);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntDrZone, surfName, thisZone.Name);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntDrSpace, surfName, thisSpace.Name);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchIntDrUfactNoFilm, surfName, state.dataHeatBal->NominalU(surface.Construction), 3);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntDrGrArea, surfName, surface.GrossArea * mult);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntDrParent, surfName, surface.BaseSurfName);
                computedNetArea(surface.BaseSurf) -= surface.GrossArea * mult;
            }
        }
        int currSurfaceClass = int(surface.Class);
        assert(currSurfaceClass < int(DataSurfaces::SurfaceClass::Num));
        assert(currSurfaceClass > int(DataSurfaces::SurfaceClass::None));
        ++numSurfaces(currSurfaceClass);
        if (isExterior) {
            ++numExtSurfaces(currSurfaceClass);
        }
        if (surface.Class == DataSurfaces::SurfaceClass::Window) {
            if (surface.OriginalClass == DataSurfaces::SurfaceClass::GlassDoor || surface.OriginalClass == DataSurfaces::SurfaceClass::TDD_Diffuser) {
                ++numSurfaces((int)surface.OriginalClass);
                if (isExterior) {
                    ++numExtSurfaces((int)surface.OriginalClass);
                }
            }
        }
    }
    // for fins and overhangs just add them explicitly since not otherwise classified
    int totOverhangs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Shading:Overhang") +
                       state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Shading:Overhang:Projection");
    numSurfaces(int(DataSurfaces::SurfaceClass::Overhang)) = totOverhangs;
    numExtSurfaces(int(DataSurfaces::SurfaceClass::Overhang)) = totOverhangs;
    int totFins = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Shading:Fin") +
                  state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Shading:Fin:Projection");
    numSurfaces(int(DataSurfaces::SurfaceClass::Fin)) = totFins;
    numExtSurfaces(int(DataSurfaces::SurfaceClass::Fin)) = totFins;
    // go through all the surfaces again and this time insert the net area results
    for (int iSurf : state.dataSurface->AllSurfaceListReportOrder) {
        auto const &surface = state.dataSurface->Surface(iSurf);
        DataSurfaces::SurfaceClass const SurfaceClass(surface.Class);
        // exterior surfaces including underground
        if ((surface.ExtBoundCond == DataSurfaces::ExternalEnvironment) || (surface.ExtBoundCond == DataSurfaces::Ground) ||
            (surface.ExtBoundCond == DataSurfaces::GroundFCfactorMethod) || (surface.ExtBoundCond == DataSurfaces::KivaFoundation)) {
            if ((SurfaceClass == DataSurfaces::SurfaceClass::Wall) || (SurfaceClass == DataSurfaces::SurfaceClass::Floor) ||
                (SurfaceClass == DataSurfaces::SurfaceClass::Roof)) {
                surfName = surface.Name;
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchOpNetArea, surfName, computedNetArea(iSurf));
            }
        } else {
            if ((SurfaceClass == DataSurfaces::SurfaceClass::Wall) || (SurfaceClass == DataSurfaces::SurfaceClass::Floor) ||
                (SurfaceClass == DataSurfaces::SurfaceClass::Roof)) {
                surfName = surface.Name;
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntOpNetArea, surfName, computedNetArea(iSurf));
            }
        } // interior surfaces
    }
    // total
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenArea, "Total or Average", fenTotArea);
    if (fenTotArea > 0.0) {
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenUfact, "Total or Average", ufactArea / fenTotArea, 3);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenSHGC, "Total or Average", shgcArea / fenTotArea, 3);
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenVisTr, "Total or Average", vistranArea / fenTotArea, 3);
    } else {
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenUfact, "Total or Average", "-");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenSHGC, "Total or Average", "-");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenVisTr, "Total or Average", "-");
    }
    // north
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenArea, "North Total or Average", fenTotAreaNorth);
    if (fenTotAreaNorth > 0.0) {
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchFenUfact, "North Total or Average", ufactAreaNorth / fenTotAreaNorth, 3);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchFenSHGC, "North Total or Average", shgcAreaNorth / fenTotAreaNorth, 3);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchFenVisTr, "North Total or Average", vistranAreaNorth / fenTotAreaNorth, 3);
    } else {
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenUfact, "North Total or Average", "-");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenSHGC, "North Total or Average", "-");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenVisTr, "North Total or Average", "-");
    }
    // non-north
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenArea, "Non-North Total or Average", fenTotAreaNonNorth);
    if (fenTotAreaNonNorth > 0.0) {
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchFenUfact, "Non-North Total or Average", ufactAreaNonNorth / fenTotAreaNonNorth, 3);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchFenSHGC, "Non-North Total or Average", shgcAreaNonNorth / fenTotAreaNonNorth, 3);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchFenVisTr, "Non-North Total or Average", vistranAreaNonNorth / fenTotAreaNonNorth, 3);
    } else {
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenUfact, "Non-North Total or Average", "-");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenSHGC, "Non-North Total or Average", "-");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchFenVisTr, "Non-North Total or Average", "-");
    }
    // interior fenestration totals
    OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntFenArea, "Total or Average", intFenTotArea);
    if (intFenTotArea > 0.0) {
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchIntFenUfact, "Total or Average", intUfactArea / intFenTotArea, 3);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchIntFenSHGC, "Total or Average", intShgcArea / intFenTotArea, 3);
        OutputReportPredefined::PreDefTableEntry(
            state, state.dataOutRptPredefined->pdchIntFenVisTr, "Total or Average", intVistranArea / intFenTotArea, 3);
    } else {
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntFenUfact, "Total or Average", "-");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntFenSHGC, "Total or Average", "-");
        OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchIntFenVisTr, "Total or Average", "-");
    }
    // counts
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntTot, "Wall", numSurfaces(int(DataSurfaces::SurfaceClass::Wall)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntExt, "Wall", numExtSurfaces(int(DataSurfaces::SurfaceClass::Wall)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntTot, "Floor", numSurfaces(int(DataSurfaces::SurfaceClass::Floor)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntExt, "Floor", numExtSurfaces(int(DataSurfaces::SurfaceClass::Floor)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntTot, "Roof", numSurfaces(int(DataSurfaces::SurfaceClass::Roof)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntExt, "Roof", numExtSurfaces(int(DataSurfaces::SurfaceClass::Roof)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntTot, "Internal Mass", numSurfaces(int(DataSurfaces::SurfaceClass::IntMass)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntExt, "Internal Mass", numExtSurfaces(int(DataSurfaces::SurfaceClass::IntMass)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntTot, "Building Detached Shading", numSurfaces(int(DataSurfaces::SurfaceClass::Detached_B)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntExt, "Building Detached Shading", numExtSurfaces(int(DataSurfaces::SurfaceClass::Detached_B)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntTot, "Fixed Detached Shading", numSurfaces(int(DataSurfaces::SurfaceClass::Detached_F)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntExt, "Fixed Detached Shading", numExtSurfaces(int(DataSurfaces::SurfaceClass::Detached_F)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntTot, "Window", numSurfaces(int(DataSurfaces::SurfaceClass::Window)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntExt, "Window", numExtSurfaces(int(DataSurfaces::SurfaceClass::Window)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntTot, "Door", numSurfaces(int(DataSurfaces::SurfaceClass::Door)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntExt, "Door", numExtSurfaces(int(DataSurfaces::SurfaceClass::Door)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntTot, "Glass Door", numSurfaces(int(DataSurfaces::SurfaceClass::GlassDoor)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntExt, "Glass Door", numExtSurfaces(int(DataSurfaces::SurfaceClass::GlassDoor)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntTot, "Shading", numSurfaces(int(DataSurfaces::SurfaceClass::Shading)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntExt, "Shading", numExtSurfaces(int(DataSurfaces::SurfaceClass::Shading)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntTot, "Overhang", numSurfaces(int(DataSurfaces::SurfaceClass::Overhang)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntExt, "Overhang", numExtSurfaces(int(DataSurfaces::SurfaceClass::Overhang)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntTot, "Fin", numSurfaces(int(DataSurfaces::SurfaceClass::Fin)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntExt, "Fin", numExtSurfaces(int(DataSurfaces::SurfaceClass::Fin)));
    OutputReportPredefined::PreDefTableEntry(
        state, state.dataOutRptPredefined->pdchSurfCntTot, "Tubular Daylighting Device Dome", numSurfaces(int(DataSurfaces::SurfaceClass::TDD_Dome)));
    OutputReportPredefined::PreDefTableEntry(state,
                                             state.dataOutRptPredefined->pdchSurfCntExt,
                                             "Tubular Daylighting Device Dome",
                                             numExtSurfaces(int(DataSurfaces::SurfaceClass::TDD_Dome)));
    OutputReportPredefined::PreDefTableEntry(state,
                                             state.dataOutRptPredefined->pdchSurfCntTot,
                                             "Tubular Daylighting Device Diffuser",
                                             numSurfaces(int(DataSurfaces::SurfaceClass::TDD_Diffuser)));
    OutputReportPredefined::PreDefTableEntry(state,
                                             state.dataOutRptPredefined->pdchSurfCntExt,
                                             "Tubular Daylighting Device Diffuser",
                                             numExtSurfaces(int(DataSurfaces::SurfaceClass::TDD_Diffuser)));
}

void AllocateSurfaceHeatBalArrays(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Richard Liesen
    //       DATE WRITTEN   February 1998

    // METHODOLOGY EMPLOYED:
    // Uses the status flags to trigger variable allocation.

    // Use the total number of surfaces to allocate variables to avoid a surface number limit
    state.dataHeatBalSurf->SurfCTFConstInPart.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfCTFConstOutPart.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfCTFCross0.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfCTFInside0.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfTempOutHist.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfCTFSourceIn0.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfQSourceSinkHist.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfIsAdiabatic.dimension(state.dataSurface->TotSurfaces, 0);
    state.dataHeatBalSurf->SurfIsSourceOrSink.dimension(state.dataSurface->TotSurfaces, 0);
    state.dataHeatBalSurf->SurfIsOperatingPool.dimension(state.dataSurface->TotSurfaces, 0);
    state.dataHeatBalSurf->SurfTempTerm.dimension(state.dataSurface->TotSurfaces, 0);
    state.dataHeatBalSurf->SurfTempDiv.dimension(state.dataSurface->TotSurfaces, 0);
    if (state.dataHeatBal->AnyInternalHeatSourceInInput) {
        state.dataHeatBalFanSys->CTFTsrcConstPart.dimension(state.dataSurface->TotSurfaces, 0.0);
        state.dataHeatBalFanSys->CTFTuserConstPart.dimension(state.dataSurface->TotSurfaces, 0.0);
    }

    state.dataHeatBal->SurfTempEffBulkAir.dimension(state.dataSurface->TotSurfaces, DataHeatBalance::ZoneInitialTemp);
    state.dataHeatBalSurf->SurfHConvInt.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfHConvExt.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfHAirExt.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfHSkyExt.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfHGrdExt.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfHSrdSurfExt.dimension(state.dataSurface->TotSurfaces, 0.0);

    state.dataHeatBalSurf->SurfTempIn.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfTempInsOld.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfTempInTmp.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurfMgr->RefAirTemp.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfQRadLWOutSrdSurfs.dimension(state.dataSurface->TotSurfaces, 0.0);

    state.dataHeatBal->SurfWinQRadSWwinAbs.dimension(state.dataSurface->TotSurfaces, DataWindowEquivalentLayer::CFSMAXNL + 1, 0.0);
    state.dataHeatBal->SurfWinInitialDifSolwinAbs.dimension(state.dataSurface->TotSurfaces, DataWindowEquivalentLayer::CFSMAXNL, 0.0);
    state.dataHeatBalSurf->SurfQRadSWOutMvIns.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBal->SurfQdotRadIntGainsInPerArea.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfQAdditionalHeatSourceOutside.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfQAdditionalHeatSourceInside.dimension(state.dataSurface->TotSurfaces, 0.0);

    state.dataHeatBalSurf->SurfInsideTempHist.allocate(Construction::MaxCTFTerms);
    state.dataHeatBalSurf->SurfOutsideTempHist.allocate(Construction::MaxCTFTerms);
    state.dataHeatBalSurf->SurfInsideFluxHist.allocate(Construction::MaxCTFTerms);
    state.dataHeatBalSurf->SurfOutsideFluxHist.allocate(Construction::MaxCTFTerms);
    for (int loop = 1; loop <= Construction::MaxCTFTerms; ++loop) {
        state.dataHeatBalSurf->SurfInsideTempHist(loop).dimension(state.dataSurface->TotSurfaces, 0);
        state.dataHeatBalSurf->SurfOutsideTempHist(loop).dimension(state.dataSurface->TotSurfaces, 0);
        state.dataHeatBalSurf->SurfInsideFluxHist(loop).dimension(state.dataSurface->TotSurfaces, 0);
        state.dataHeatBalSurf->SurfOutsideFluxHist(loop).dimension(state.dataSurface->TotSurfaces, 0);
    }

    if (!state.dataHeatBal->SimpleCTFOnly || state.dataGlobal->AnyEnergyManagementSystemInModel) {
        state.dataHeatBalSurf->SurfCurrNumHist.dimension(state.dataSurface->TotSurfaces, 0);

        state.dataHeatBalSurf->SurfInsideTempHistMaster.allocate(Construction::MaxCTFTerms);
        state.dataHeatBalSurf->SurfOutsideTempHistMaster.allocate(Construction::MaxCTFTerms);
        state.dataHeatBalSurf->SurfInsideFluxHistMaster.allocate(Construction::MaxCTFTerms);
        state.dataHeatBalSurf->SurfOutsideFluxHistMaster.allocate(Construction::MaxCTFTerms);

        for (int loop = 1; loop <= Construction::MaxCTFTerms; ++loop) {
            state.dataHeatBalSurf->SurfInsideTempHistMaster(loop).dimension(state.dataSurface->TotSurfaces, 0);
            state.dataHeatBalSurf->SurfOutsideTempHistMaster(loop).dimension(state.dataSurface->TotSurfaces, 0);
            state.dataHeatBalSurf->SurfInsideFluxHistMaster(loop).dimension(state.dataSurface->TotSurfaces, 0);
            state.dataHeatBalSurf->SurfOutsideFluxHistMaster(loop).dimension(state.dataSurface->TotSurfaces, 0);
        }
    }

    state.dataHeatBalSurf->SurfTempOut.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfTempInMovInsRep.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfQConvInRep.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfQdotConvInPerArea.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfQdotConvInRep.dimension(state.dataSurface->TotSurfaces, 0.0);

    state.dataHeatBalSurf->SurfQRadNetSurfInRep.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfQdotRadNetSurfInRep.dimension(state.dataSurface->TotSurfaces, 0.0);

    state.dataHeatBalSurf->SurfQRadSolarInRep.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfQdotRadSolarInRep.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfQdotRadSolarInRepPerArea.dimension(state.dataSurface->TotSurfaces, 0.0);

    state.dataHeatBalSurf->SurfQRadLightsInRep.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfQdotRadLightsInRep.dimension(state.dataSurface->TotSurfaces, 0.0);

    state.dataHeatBalSurf->SurfQRadIntGainsInRep.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfQdotRadIntGainsInRep.dimension(state.dataSurface->TotSurfaces, 0.0);

    state.dataHeatBalSurf->SurfQRadHVACInRep.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfQdotRadHVACInRep.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfQdotRadHVACInPerArea.dimension(state.dataSurface->TotSurfaces, 0.0);

    state.dataHeatBalSurf->SurfQConvOutReport.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfQdotConvOutPerArea.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfQdotConvOutRep.dimension(state.dataSurface->TotSurfaces, 0.0);

    state.dataHeatBalSurf->SurfQdotRadOutRep.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfQdotRadOutRepPerArea.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfQRadOutReport.dimension(state.dataSurface->TotSurfaces, 0.0);

    state.dataHeatBalSurf->SurfQAirExtReport.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfQHeatEmiReport.dimension(state.dataSurface->TotSurfaces, 0.0);

    state.dataHeatBal->SurfOpaqSWOutAbsTotalReport.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBal->SurfOpaqSWOutAbsEnergyReport.dimension(state.dataSurface->TotSurfaces, 0.0);

    state.dataHeatBalSurf->SurfOpaqInsFaceCond.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfOpaqInsFaceCondFlux.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfOpaqInsFaceCondGainRep.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfOpaqInsFaceCondLossRep.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfOpaqInsFaceCondEnergy.dimension(state.dataSurface->TotSurfaces, 0.0);

    state.dataHeatBalSurf->SurfOpaqOutFaceCond.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfOpaqOutFaceCondFlux.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfOpaqExtFaceCondGainRep.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfOpaqExtFaceCondLossRep.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfOpaqOutFaceCondEnergy.dimension(state.dataSurface->TotSurfaces, 0.0);

    state.dataHeatBalSurf->SurfOpaqAvgFaceCondGainRep.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfOpaqAvgFaceCondLossRep.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfOpaqAvgFaceCond.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfOpaqAvgFaceCondFlux.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfOpaqAvgFaceCondEnergy.dimension(state.dataSurface->TotSurfaces, 0.0);

    state.dataHeatBalSurf->SurfOpaqStorageCondGainRep.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfOpaqStorageCondLossRep.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfOpaqStorageCond.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfOpaqStorageCondFlux.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfOpaqStorageCondEnergy.dimension(state.dataSurface->TotSurfaces, 0.0);

    state.dataHeatBalSurf->SurfOpaqInsFaceBeamSolAbsorbed.dimension(state.dataSurface->TotSurfaces, 0.0);

    state.dataHeatBalSurf->SurfOpaqQRadSWOutAbs.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfOpaqQRadSWInAbs.dimension(state.dataSurface->TotSurfaces, 0.0);

    state.dataHeatBalSurf->SurfOpaqInitialDifSolInAbs.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfWinInitialDifSolInTrans.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfWinInitialBeamSolInTrans.dimension(state.dataSurface->TotSurfaces, 0.0);

    state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfQdotRadLightsInPerArea.dimension(state.dataSurface->TotSurfaces, 0.0);

    if (state.dataHeatBal->AnyInternalHeatSourceInInput) {
        state.dataHeatBalSurf->SurfTempSource.dimension(state.dataSurface->TotSurfaces, 0.0);
        state.dataHeatBalSurf->SurfTempUserLoc.dimension(state.dataSurface->TotSurfaces, 0.0);
        state.dataHeatBalSurf->SurfTsrcHist.dimension(state.dataSurface->TotSurfaces, Construction::MaxCTFTerms, 0.0);
        state.dataHeatBalSurf->SurfTuserHist.dimension(state.dataSurface->TotSurfaces, Construction::MaxCTFTerms, 0.0);
        state.dataHeatBalSurf->SurfQsrcHist.dimension(state.dataSurface->TotSurfaces, Construction::MaxCTFTerms, 0.0);
        state.dataHeatBalSurf->SurfTsrcHistM.dimension(state.dataSurface->TotSurfaces, Construction::MaxCTFTerms, 0.0);
        state.dataHeatBalSurf->SurfTuserHistM.dimension(state.dataSurface->TotSurfaces, Construction::MaxCTFTerms, 0.0);
        state.dataHeatBalSurf->SurfQsrcHistM.dimension(state.dataSurface->TotSurfaces, Construction::MaxCTFTerms, 0.0);
    }

    state.dataHeatBalFanSys->RadSysTiHBConstCoef.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalFanSys->RadSysTiHBToutCoef.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalFanSys->RadSysTiHBQsrcCoef.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalFanSys->RadSysToHBConstCoef.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalFanSys->RadSysToHBTinCoef.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalFanSys->RadSysToHBQsrcCoef.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalFanSys->QRadSysSource.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalFanSys->TCondFDSourceNode.dimension(state.dataSurface->TotSurfaces, 15.0);
    state.dataHeatBalFanSys->surfQRadFromHVAC.allocate(state.dataSurface->TotSurfaces);
    state.dataHeatBalFanSys->QRadSurfAFNDuct.dimension(state.dataSurface->TotSurfaces, 0.0);

    // allocate terms used for pool surface heat balance
    state.dataHeatBalFanSys->QPoolSurfNumerator.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalFanSys->PoolHeatTransCoefs.dimension(state.dataSurface->TotSurfaces, 0.0);

    // allocate term used as sink for PV electricity
    state.dataHeatBalFanSys->QPVSysSource.dimension(state.dataSurface->TotSurfaces, 0.0);

    // Allocate the moisture balance arrays
    state.dataMstBal->TempOutsideAirFD.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataMstBal->RhoVaporAirOut.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataMstBal->RhoVaporSurfIn.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataMstBal->RhoVaporAirIn.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataMstBal->HConvExtFD.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataMstBal->HMassConvExtFD.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataMstBal->HConvInFD.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataMstBal->HMassConvInFD.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataMstBal->HSkyFD.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataMstBal->HGrndFD.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataMstBal->HAirFD.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataMstBal->HSurrFD.dimension(state.dataSurface->TotSurfaces, 0.0);

    state.dataSurface->SurfSkySolarInc.dimension(state.dataSurface->TotSurfaces, 0);
    state.dataSurface->SurfGndSolarInc.dimension(state.dataSurface->TotSurfaces, 0);

    state.dataHeatBalSurf->SurfAbsSolarExt.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfAbsThermalExt.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfRoughnessExt.dimension(state.dataSurface->TotSurfaces, Material::SurfaceRoughness::Invalid);
    state.dataHeatBalSurf->SurfAbsSolarInt.dimension(state.dataSurface->TotSurfaces, 0.0);
    state.dataHeatBalSurf->SurfAbsThermalInt.dimension(state.dataSurface->TotSurfaces, 0.0);

    DisplayString(state, "Setting up Surface Reporting Variables");
    // Setup surface report variables CurrentModuleObject='Opaque Surfaces'
    for (int loop = 1; loop <= state.dataSurface->TotSurfaces; ++loop) {
        auto &surface = state.dataSurface->Surface(loop);
        if (!surface.HeatTransSurf) {
            continue;
        }
        SetupOutputVariable(state,
                            "Surface Inside Face Temperature",
                            Constant::Units::C,
                            state.dataHeatBalSurf->SurfTempIn(loop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surface.Name);
        SetupOutputVariable(state,
                            "Surface Inside Face Interior Movable Insulation Temperature",
                            Constant::Units::C,
                            state.dataHeatBalSurf->SurfTempInMovInsRep(loop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surface.Name);

        if (surface.ExtBoundCond != DataSurfaces::KivaFoundation) {
            SetupOutputVariable(state,
                                "Surface Outside Face Temperature",
                                Constant::Units::C,
                                state.dataHeatBalSurf->SurfTempOut(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
        }

        SetupOutputVariable(state,
                            "Surface Inside Face Adjacent Air Temperature",
                            Constant::Units::C,
                            state.dataHeatBal->SurfTempEffBulkAir(loop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surface.Name);
        SetupOutputVariable(state,
                            "Surface Inside Face Convection Heat Transfer Coefficient",
                            Constant::Units::W_m2K,
                            state.dataHeatBalSurf->SurfHConvInt(loop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surface.Name);
        SetupOutputVariable(state,
                            "Surface Inside Face Convection Heat Gain Rate",
                            Constant::Units::W,
                            state.dataHeatBalSurf->SurfQdotConvInRep(loop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surface.Name);
        SetupOutputVariable(state,
                            "Surface Inside Face Convection Heat Gain Rate per Area",
                            Constant::Units::W_m2,
                            state.dataHeatBalSurf->SurfQdotConvInPerArea(loop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surface.Name);
        SetupOutputVariable(state,
                            "Surface Inside Face Convection Heat Gain Energy",
                            Constant::Units::J,
                            state.dataHeatBalSurf->SurfQConvInRep(loop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            surface.Name);

        SetupOutputVariable(state,
                            "Surface Inside Face Net Surface Thermal Radiation Heat Gain Rate",
                            Constant::Units::W,
                            state.dataHeatBalSurf->SurfQdotRadNetSurfInRep(loop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surface.Name);
        SetupOutputVariable(state,
                            "Surface Inside Face Net Surface Thermal Radiation Heat Gain Rate per Area",
                            Constant::Units::W_m2,
                            state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(loop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surface.Name);
        SetupOutputVariable(state,
                            "Surface Inside Face Net Surface Thermal Radiation Heat Gain Energy",
                            Constant::Units::J,
                            state.dataHeatBalSurf->SurfQRadNetSurfInRep(loop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            surface.Name);

        if (surface.Class != DataSurfaces::SurfaceClass::Window) {
            SetupOutputVariable(state,
                                "Surface Inside Face Solar Radiation Heat Gain Rate",
                                Constant::Units::W,
                                state.dataHeatBalSurf->SurfQdotRadSolarInRep(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Inside Face Solar Radiation Heat Gain Rate per Area",
                                Constant::Units::W_m2,
                                state.dataHeatBalSurf->SurfQdotRadSolarInRepPerArea(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Inside Face Solar Radiation Heat Gain Energy",
                                Constant::Units::J,
                                state.dataHeatBalSurf->SurfQRadSolarInRep(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                surface.Name);

            SetupOutputVariable(state,
                                "Surface Inside Face Lights Radiation Heat Gain Rate",
                                Constant::Units::W,
                                state.dataHeatBalSurf->SurfQdotRadLightsInRep(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Inside Face Lights Radiation Heat Gain Rate per Area",
                                Constant::Units::W_m2,
                                state.dataHeatBalSurf->SurfQdotRadLightsInPerArea(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Inside Face Lights Radiation Heat Gain Energy",
                                Constant::Units::J,
                                state.dataHeatBalSurf->SurfQRadLightsInRep(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                surface.Name);
        }

        SetupOutputVariable(state,
                            "Surface Inside Face Internal Gains Radiation Heat Gain Rate",
                            Constant::Units::W,
                            state.dataHeatBalSurf->SurfQdotRadIntGainsInRep(loop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surface.Name);
        SetupOutputVariable(state,
                            "Surface Inside Face Internal Gains Radiation Heat Gain Rate per Area",
                            Constant::Units::W_m2,
                            state.dataHeatBal->SurfQdotRadIntGainsInPerArea(loop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surface.Name);
        SetupOutputVariable(state,
                            "Surface Inside Face Internal Gains Radiation Heat Gain Energy",
                            Constant::Units::J,
                            state.dataHeatBalSurf->SurfQRadIntGainsInRep(loop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            surface.Name);

        SetupOutputVariable(state,
                            "Surface Inside Face System Radiation Heat Gain Rate",
                            Constant::Units::W,
                            state.dataHeatBalSurf->SurfQdotRadHVACInRep(loop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surface.Name);
        SetupOutputVariable(state,
                            "Surface Inside Face System Radiation Heat Gain Rate per Area",
                            Constant::Units::W_m2,
                            state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(loop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surface.Name);
        SetupOutputVariable(state,
                            "Surface Inside Face System Radiation Heat Gain Energy",
                            Constant::Units::J,
                            state.dataHeatBalSurf->SurfQRadHVACInRep(loop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Sum,
                            surface.Name);

        if (surface.ExtBoundCond == DataSurfaces::ExternalEnvironment || state.dataGlobal->DisplayAdvancedReportVariables) {
            SetupOutputVariable(state,
                                "Surface Outside Face Outdoor Air Drybulb Temperature",
                                Constant::Units::C,
                                state.dataSurface->SurfOutDryBulbTemp(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Outdoor Air Wetbulb Temperature",
                                Constant::Units::C,
                                state.dataSurface->SurfOutWetBulbTemp(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Outdoor Air Wind Speed",
                                Constant::Units::m_s,
                                state.dataSurface->SurfOutWindSpeed(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Outdoor Air Wind Direction",
                                Constant::Units::deg,
                                state.dataSurface->SurfOutWindDir(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Convection Heat Gain Rate",
                                Constant::Units::W,
                                state.dataHeatBalSurf->SurfQdotConvOutRep(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Convection Heat Gain Rate per Area",
                                Constant::Units::W_m2,
                                state.dataHeatBalSurf->SurfQdotConvOutPerArea(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Convection Heat Gain Energy",
                                Constant::Units::J,
                                state.dataHeatBalSurf->SurfQConvOutReport(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Convection Heat Transfer Coefficient",
                                Constant::Units::W_m2K,
                                state.dataHeatBalSurf->SurfHConvExt(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Net Thermal Radiation Heat Gain Rate",
                                Constant::Units::W,
                                state.dataHeatBalSurf->SurfQdotRadOutRep(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Net Thermal Radiation Heat Gain Rate per Area",
                                Constant::Units::W_m2,
                                state.dataHeatBalSurf->SurfQdotRadOutRepPerArea(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Net Thermal Radiation Heat Gain Energy",
                                Constant::Units::J,
                                state.dataHeatBalSurf->SurfQRadOutReport(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Thermal Radiation to Air Heat Transfer Coefficient",
                                Constant::Units::W_m2K,
                                state.dataHeatBalSurf->SurfHAirExt(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Thermal Radiation to Sky Heat Transfer Coefficient",
                                Constant::Units::W_m2K,
                                state.dataHeatBalSurf->SurfHSkyExt(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Thermal Radiation to Ground Heat Transfer Coefficient",
                                Constant::Units::W_m2K,
                                state.dataHeatBalSurf->SurfHGrdExt(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Thermal Radiation to Air Heat Transfer Rate",
                                Constant::Units::W,
                                state.dataHeatBalSurf->SurfQAirExtReport(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Heat Emission to Air Rate",
                                Constant::Units::W,
                                state.dataHeatBalSurf->SurfQHeatEmiReport(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);

            if (surface.Class != DataSurfaces::SurfaceClass::Window) {
                SetupOutputVariable(state,
                                    "Surface Outside Face Solar Radiation Heat Gain Rate",
                                    Constant::Units::W,
                                    state.dataHeatBal->SurfOpaqSWOutAbsTotalReport(loop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surface.Name);
                SetupOutputVariable(state,
                                    "Surface Outside Face Solar Radiation Heat Gain Rate per Area",
                                    Constant::Units::W_m2,
                                    state.dataHeatBalSurf->SurfOpaqQRadSWOutAbs(loop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surface.Name);
                SetupOutputVariable(state,
                                    "Surface Outside Face Solar Radiation Heat Gain Energy",
                                    Constant::Units::J,
                                    state.dataHeatBal->SurfOpaqSWOutAbsEnergyReport(loop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    surface.Name);
            }
        }
        if (surface.Class == DataSurfaces::SurfaceClass::Floor || surface.Class == DataSurfaces::SurfaceClass::Wall ||
            surface.Class == DataSurfaces::SurfaceClass::IntMass || surface.Class == DataSurfaces::SurfaceClass::Roof ||
            surface.Class == DataSurfaces::SurfaceClass::Door) {
            //      IF (DisplayAdvancedReportVariables) THEN  !CurrentModuleObject='Opaque Surfaces(Advanced)'
            SetupOutputVariable(state,
                                "Surface Inside Face Conduction Heat Transfer Rate",
                                Constant::Units::W,
                                state.dataHeatBalSurf->SurfOpaqInsFaceCond(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Inside Face Conduction Heat Gain Rate",
                                Constant::Units::W,
                                state.dataHeatBalSurf->SurfOpaqInsFaceCondGainRep(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Inside Face Conduction Heat Loss Rate",
                                Constant::Units::W,
                                state.dataHeatBalSurf->SurfOpaqInsFaceCondLossRep(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Inside Face Conduction Heat Transfer Rate per Area",
                                Constant::Units::W_m2,
                                state.dataHeatBalSurf->SurfOpaqInsFaceCondFlux(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Inside Face Conduction Heat Transfer Energy",
                                Constant::Units::J,
                                state.dataHeatBalSurf->SurfOpaqInsFaceCondEnergy(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                surface.Name);

            if (surface.ExtBoundCond != DataSurfaces::KivaFoundation) {
                SetupOutputVariable(state,
                                    "Surface Outside Face Conduction Heat Transfer Rate",
                                    Constant::Units::W,
                                    state.dataHeatBalSurf->SurfOpaqOutFaceCond(loop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surface.Name);
                SetupOutputVariable(state,
                                    "Surface Outside Face Conduction Heat Gain Rate",
                                    Constant::Units::W,
                                    state.dataHeatBalSurf->SurfOpaqExtFaceCondGainRep(loop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surface.Name);
                SetupOutputVariable(state,
                                    "Surface Outside Face Conduction Heat Loss Rate",
                                    Constant::Units::W,
                                    state.dataHeatBalSurf->SurfOpaqExtFaceCondLossRep(loop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surface.Name);
                SetupOutputVariable(state,
                                    "Surface Outside Face Conduction Heat Transfer Rate per Area",
                                    Constant::Units::W_m2,
                                    state.dataHeatBalSurf->SurfOpaqOutFaceCondFlux(loop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surface.Name);
                SetupOutputVariable(state,
                                    "Surface Outside Face Conduction Heat Transfer Energy",
                                    Constant::Units::J,
                                    state.dataHeatBalSurf->SurfOpaqOutFaceCondEnergy(loop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    surface.Name);

                SetupOutputVariable(state,
                                    "Surface Average Face Conduction Heat Transfer Rate",
                                    Constant::Units::W,
                                    state.dataHeatBalSurf->SurfOpaqAvgFaceCond(loop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surface.Name);
                SetupOutputVariable(state,
                                    "Surface Average Face Conduction Heat Gain Rate",
                                    Constant::Units::W,
                                    state.dataHeatBalSurf->SurfOpaqAvgFaceCondGainRep(loop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surface.Name);
                SetupOutputVariable(state,
                                    "Surface Average Face Conduction Heat Loss Rate",
                                    Constant::Units::W,
                                    state.dataHeatBalSurf->SurfOpaqAvgFaceCondLossRep(loop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surface.Name);
                SetupOutputVariable(state,
                                    "Surface Average Face Conduction Heat Transfer Rate per Area",
                                    Constant::Units::W_m2,
                                    state.dataHeatBalSurf->SurfOpaqAvgFaceCondFlux(loop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surface.Name);
                SetupOutputVariable(state,
                                    "Surface Average Face Conduction Heat Transfer Energy",
                                    Constant::Units::J,
                                    state.dataHeatBalSurf->SurfOpaqAvgFaceCondEnergy(loop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    surface.Name);

                SetupOutputVariable(state,
                                    "Surface Heat Storage Rate",
                                    Constant::Units::W,
                                    state.dataHeatBalSurf->SurfOpaqStorageCond(loop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surface.Name);
                SetupOutputVariable(state,
                                    "Surface Heat Storage Gain Rate",
                                    Constant::Units::W,
                                    state.dataHeatBalSurf->SurfOpaqStorageCondGainRep(loop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surface.Name);
                SetupOutputVariable(state,
                                    "Surface Heat Storage Loss Rate",
                                    Constant::Units::W,
                                    state.dataHeatBalSurf->SurfOpaqStorageCondLossRep(loop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surface.Name);
                SetupOutputVariable(state,
                                    "Surface Heat Storage Rate per Area",
                                    Constant::Units::W_m2,
                                    state.dataHeatBalSurf->SurfOpaqStorageCondFlux(loop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Average,
                                    surface.Name);
                SetupOutputVariable(state,
                                    "Surface Heat Storage Energy",
                                    Constant::Units::J,
                                    state.dataHeatBalSurf->SurfOpaqStorageCondEnergy(loop),
                                    OutputProcessor::TimeStepType::Zone,
                                    OutputProcessor::StoreType::Sum,
                                    surface.Name);
            }

            //      ENDIF
            // CurrentModuleObject='Opaque Surfaces'

            SetupOutputVariable(state,
                                "Surface Inside Face Beam Solar Radiation Heat Gain Rate",
                                Constant::Units::W,
                                state.dataHeatBalSurf->SurfOpaqInsFaceBeamSolAbsorbed(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
        }
        if (state.dataConstruction->Construct(surface.Construction).SourceSinkPresent) {
            SetupOutputVariable(state,
                                "Surface Internal Source Location Temperature",
                                Constant::Units::C,
                                state.dataHeatBalSurf->SurfTempSource(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Internal User Specified Location Temperature",
                                Constant::Units::C,
                                state.dataHeatBalSurf->SurfTempUserLoc(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
        }

        if (surface.Class == DataSurfaces::SurfaceClass::Window) { // CurrentModuleObject='Windows'
            auto &surfShade = state.dataSurface->surfShades(loop);
            SetupOutputVariable(state,
                                "Surface Shading Device Is On Time Fraction",
                                Constant::Units::None,
                                state.dataSurface->SurfWinFracTimeShadingDeviceOn(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Storm Window On Off Status",
                                Constant::Units::None,
                                state.dataSurface->SurfWinStormWinFlag(loop),
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Window Blind Slat Angle",
                                Constant::Units::deg,
                                surfShade.blind.slatAngDeg,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
        }
        //    IF (DisplayAdvancedReportVariables) THEN  !CurrentModuleObject='Opaque Surfaces(Advanced)'
        SetupOutputVariable(state,
                            "Surface Inside Face Convection Classification Index",
                            Constant::Units::None,
                            state.dataSurface->surfIntConv(loop).convClassRpt,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surface.Name);
        SetupOutputVariable(state,
                            "Surface Inside Face Convection Model Equation Index",
                            Constant::Units::None,
                            state.dataSurface->surfIntConv(loop).hcModelEqRpt,
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surface.Name);
        SetupOutputVariable(state,
                            "Surface Inside Face Convection Reference Air Index",
                            Constant::Units::None,
                            state.dataSurface->SurfTAirRefRpt(loop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surface.Name);
        if (surface.ExtBoundCond == DataSurfaces::ExternalEnvironment) {
            SetupOutputVariable(state,
                                "Surface Outside Face Convection Classification Index",
                                Constant::Units::None,
                                state.dataSurface->surfExtConv(loop).convClassRpt,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Forced Convection Model Equation Index",
                                Constant::Units::None,
                                state.dataSurface->surfExtConv(loop).hfModelEqRpt,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
            SetupOutputVariable(state,
                                "Surface Outside Face Natural Convection Model Equation Index",
                                Constant::Units::None,
                                state.dataSurface->surfExtConv(loop).hnModelEqRpt,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
        }

        SetupOutputVariable(state,
                            "Surface Inside Face Heat Source Gain Rate per Area",
                            Constant::Units::W_m2,
                            state.dataHeatBalSurf->SurfQAdditionalHeatSourceInside(loop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surface.Name);
        SetupOutputVariable(state,
                            "Surface Outside Face Heat Source Gain Rate per Area",
                            Constant::Units::W_m2,
                            state.dataHeatBalSurf->SurfQAdditionalHeatSourceOutside(loop),
                            OutputProcessor::TimeStepType::Zone,
                            OutputProcessor::StoreType::Average,
                            surface.Name);

        //     ENDIF
        if (state.dataGlobal->DisplayAdvancedReportVariables) {
            SetupOutputVariable(state,
                                "Surface Construction Index",
                                Constant::Units::None,
                                surface.Construction,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                surface.Name);
        }
    }
    SetupOutputVariable(state,
                        "Site Total Surface Heat Emission to Air",
                        Constant::Units::J,
                        state.dataHeatBalSurf->SumSurfaceHeatEmission,
                        OutputProcessor::TimeStepType::Zone,
                        OutputProcessor::StoreType::Sum,
                        "Environment");
}

void InitThermalAndFluxHistories(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         George Walton
    //       DATE WRITTEN   March 1978
    //       RE-ENGINEERED  Feb98 (RKS)

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine sets the initial temperature and flux histories
    // needed for a stable and reasonable heat balance solution starting
    // point.

    // METHODOLOGY EMPLOYED:
    // This subroutine assumes that the simulation is at steady state at
    // the beginning and then begins to vary.  Thus, the temperatures, the
    // fluxes. and their histories can all be set to the same value.  Some
    // of the initializations depend on the surface characteristics.  This
    // requires a DO loop to perform the proper calculation.

    // REFERENCES:
    // (I)BLAST legacy routine INITTH

    // First do the "bulk" initializations of arrays sized to NumOfZones
    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        new (&state.dataZoneTempPredictorCorrector->zoneHeatBalance(zoneNum)) ZoneTempPredictorCorrector::ZoneHeatBalanceData();
        // Initialize the Zone Humidity Ratio here so that it is available for EMPD implementations
        auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(zoneNum);
        thisZoneHB.airHumRatAvg = state.dataEnvrn->OutHumRat;
        thisZoneHB.airHumRat = state.dataEnvrn->OutHumRat;
        state.dataHeatBalFanSys->TempTstatAir(zoneNum) = DataHeatBalance::ZoneInitialTemp;
    }
    for (auto &thisEnclosure : state.dataViewFactor->EnclRadInfo) {
        thisEnclosure.MRT = DataHeatBalance::ZoneInitialTemp;
    }
    // Reset spaceHeatBalance even if doSpaceHeatBalance is false, because spaceHB is used to gether zoneHB in some cases
    for (auto &thisSpaceHB : state.dataZoneTempPredictorCorrector->spaceHeatBalance) {
        new (&thisSpaceHB) ZoneTempPredictorCorrector::SpaceHeatBalanceData();
        // Initialize the Zone Humidity Ratio here so that it is available for EMPD implementations
        thisSpaceHB.airHumRatAvg = state.dataEnvrn->OutHumRat;
        thisSpaceHB.airHumRat = state.dataEnvrn->OutHumRat;
    }

    // "Bulk" initializations of arrays sized to TotSurfaces
    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurf = thisSpace.HTSurfaceFirst;
            int const lastSurf = thisSpace.HTSurfaceLast;
            for (int SurfNum = firstSurf; SurfNum <= lastSurf; ++SurfNum) {
                state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = DataHeatBalance::SurfInitialTemp;
                state.dataHeatBalSurf->SurfTempIn(SurfNum) = DataHeatBalance::SurfInitialTemp;        // module level array
                state.dataHeatBalSurf->SurfTempInTmp(SurfNum) = DataHeatBalance::SurfInitialTemp;     // module level array
                state.dataHeatBalSurf->SurfHConvInt(SurfNum) = DataHeatBalance::SurfInitialConvCoeff; // module level array
                state.dataHeatBalSurf->SurfHConvExt(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfHAirExt(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfHSkyExt(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfHGrdExt(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfTempOut(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfTempInMovInsRep(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQConvInRep(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQdotConvInRep(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQdotConvInPerArea(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQRadNetSurfInRep(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQdotRadNetSurfInRep(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQRadSolarInRep(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQdotRadSolarInRep(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQdotRadSolarInRepPerArea(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQRadLightsInRep(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQdotRadLightsInRep(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQRadIntGainsInRep(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQdotRadIntGainsInRep(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQRadHVACInRep(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQdotRadHVACInRep(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQConvOutReport(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQdotConvOutRep(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQdotConvOutPerArea(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQRadOutReport(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQdotRadOutRep(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQdotRadOutRepPerArea(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQAirExtReport(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQHeatEmiReport(SurfNum) = 0.0;
            } // end of  Surf array
            int const firstSurfOpaq = thisSpace.OpaqOrIntMassSurfaceFirst;
            int const lastSurfOpaq = thisSpace.OpaqOrIntMassSurfaceLast;
            if (firstSurfOpaq >= 0) {
                for (int SurfNum = firstSurfOpaq; SurfNum <= lastSurfOpaq; ++SurfNum) {
                    state.dataHeatBalSurf->SurfOpaqInsFaceCond(SurfNum) = 0.0;
                    state.dataHeatBalSurf->SurfOpaqInsFaceCondFlux(SurfNum) = 0.0;
                    state.dataHeatBalSurf->SurfOpaqInsFaceCondEnergy(SurfNum) = 0.0;
                    state.dataHeatBalSurf->SurfOpaqInsFaceBeamSolAbsorbed(SurfNum) = 0.0;
                } // end of Zone Surf
            }
            int const firstSurfWin = thisSpace.WindowSurfaceFirst;
            int const lastSurfWin = thisSpace.WindowSurfaceLast;
            if (firstSurfWin >= 0) {
                for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {
                    // Initialize window frame and divider temperatures
                    state.dataSurface->SurfWinFrameTempIn(SurfNum) = DataHeatBalance::SurfInitialTemp;
                    state.dataSurface->SurfWinFrameTempInOld(SurfNum) = DataHeatBalance::SurfInitialTemp;
                    state.dataSurface->SurfWinFrameTempSurfOut(SurfNum) = DataHeatBalance::SurfInitialTemp;
                    state.dataSurface->SurfWinDividerTempIn(SurfNum) = DataHeatBalance::SurfInitialTemp;
                    state.dataSurface->SurfWinDividerTempInOld(SurfNum) = DataHeatBalance::SurfInitialTemp;
                    state.dataSurface->SurfWinDividerTempSurfOut(SurfNum) = DataHeatBalance::SurfInitialTemp;

                    // Initialize previous-timestep shading indicators
                    state.dataSurface->SurfWinExtIntShadePrevTS(SurfNum) = DataSurfaces::WinShadingType::NoShade;
                    state.dataSurface->SurfWinShadingFlag(SurfNum) = DataSurfaces::WinShadingType::NoShade;
                } // end of Zone Surf
            }
        }
    } // end of Zone

    // "Bulk" initializations of temperature arrays with dimensions (TotSurface,MaxCTFTerms,2)
    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurf = thisSpace.HTSurfaceFirst;
            int const lastSurf = thisSpace.HTSurfaceLast;
            for (int CTFTermNum = 1; CTFTermNum <= Construction::MaxCTFTerms; ++CTFTermNum) {
                for (int SurfNum = firstSurf; SurfNum <= lastSurf; ++SurfNum) {
                    state.dataHeatBalSurf->SurfInsideTempHist(CTFTermNum)(SurfNum) = DataHeatBalance::SurfInitialTemp;
                    state.dataHeatBalSurf->SurfOutsideTempHist(CTFTermNum)(SurfNum) = DataHeatBalance::SurfInitialTemp;
                    state.dataHeatBalSurf->SurfInsideFluxHist(CTFTermNum)(SurfNum) = 0.0;
                    state.dataHeatBalSurf->SurfOutsideFluxHist(CTFTermNum)(SurfNum) = 0.0;
                }
            }
        }
    }
    if (!state.dataHeatBal->SimpleCTFOnly || state.dataGlobal->AnyEnergyManagementSystemInModel) {
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                int const firstSurf = thisSpace.HTSurfaceFirst;
                int const lastSurf = thisSpace.HTSurfaceLast;
                for (int SurfNum = firstSurf; SurfNum <= lastSurf; ++SurfNum) {
                    state.dataHeatBalSurf->SurfCurrNumHist(SurfNum) = 0;
                }
                for (int CTFTermNum = 1; CTFTermNum <= Construction::MaxCTFTerms; ++CTFTermNum) {
                    for (int SurfNum = firstSurf; SurfNum <= lastSurf; ++SurfNum) {
                        state.dataHeatBalSurf->SurfInsideTempHistMaster(CTFTermNum)(SurfNum) = DataHeatBalance::SurfInitialTemp;
                        state.dataHeatBalSurf->SurfOutsideTempHistMaster(CTFTermNum)(SurfNum) = DataHeatBalance::SurfInitialTemp;
                        state.dataHeatBalSurf->SurfInsideFluxHistMaster(CTFTermNum)(SurfNum) = 0.0;
                        state.dataHeatBalSurf->SurfOutsideFluxHistMaster(CTFTermNum)(SurfNum) = 0.0;
                    }
                }
            }
        }
    }
    if (state.dataHeatBal->AnyInternalHeatSourceInInput) {
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                int const firstSurf = thisSpace.HTSurfaceFirst;
                int const lastSurf = thisSpace.HTSurfaceLast;
                for (int SurfNum = firstSurf; SurfNum <= lastSurf; ++SurfNum) {
                    for (int CTFTermNum = 1; CTFTermNum <= Construction::MaxCTFTerms; ++CTFTermNum) {
                        state.dataHeatBalSurf->SurfTsrcHist(SurfNum, CTFTermNum) = DataHeatBalance::SurfInitialTemp;
                        state.dataHeatBalSurf->SurfTsrcHistM(SurfNum, CTFTermNum) = DataHeatBalance::SurfInitialTemp;
                        state.dataHeatBalSurf->SurfTuserHist(SurfNum, CTFTermNum) = DataHeatBalance::SurfInitialTemp;
                        state.dataHeatBalSurf->SurfTuserHistM(SurfNum, CTFTermNum) = DataHeatBalance::SurfInitialTemp;
                        state.dataHeatBalSurf->SurfQsrcHist(SurfNum, CTFTermNum) = 0.0;
                        state.dataHeatBalSurf->SurfQsrcHistM(SurfNum, CTFTermNum) = 0.0;
                    }
                }
            }
        }
    }
    state.dataHeatBal->CondFDRelaxFactor = state.dataHeatBal->CondFDRelaxFactorInput;

    // Perform other initializations that depend on the surface characteristics
    for (int CTFTermNum = 1; CTFTermNum <= state.dataHeatBal->MaxCTFTerms + 1; ++CTFTermNum) {
        for (int SurfNum : state.dataSurface->AllHTSurfaceList) {
            auto &surface = state.dataSurface->Surface(SurfNum);
            // Reset outside boundary conditions if necessary
            if ((surface.ExtBoundCond == DataSurfaces::ExternalEnvironment) || (surface.ExtBoundCond == DataSurfaces::OtherSideCondModeledExt)) {
                state.dataHeatBalSurf->SurfOutsideTempHist(CTFTermNum)(SurfNum) = state.dataSurface->SurfOutDryBulbTemp(SurfNum);
            } else if (surface.ExtBoundCond == DataSurfaces::Ground) {
                state.dataHeatBalSurf->SurfOutsideTempHist(CTFTermNum)(SurfNum) =
                    state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::BuildingSurface];
            } else if (surface.ExtBoundCond == DataSurfaces::GroundFCfactorMethod) {
                state.dataHeatBalSurf->SurfOutsideTempHist(CTFTermNum)(SurfNum) =
                    state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::FCFactorMethod];
            }
            // Initialize the flux histories
            state.dataHeatBalSurf->SurfOutsideFluxHist(CTFTermNum)(SurfNum) =
                state.dataConstruction->Construct(surface.Construction).UValue *
                (state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum) - state.dataHeatBalSurf->SurfInsideTempHist(1)(SurfNum));
            state.dataHeatBalSurf->SurfInsideFluxHist(CTFTermNum)(SurfNum) = state.dataHeatBalSurf->SurfOutsideFluxHist(2)(SurfNum);
        }
    }
    for (int SurfNum : state.dataSurface->AllHTSurfaceList) {
        if (state.dataSurface->SurfExtCavityPresent(SurfNum)) {
            state.dataHeatBal->ExtVentedCavity(state.dataSurface->SurfExtCavNum(SurfNum)).TbaffleLast = 20.0;
            state.dataHeatBal->ExtVentedCavity(state.dataSurface->SurfExtCavNum(SurfNum)).TairLast = 20.0;
        }
    }
    // Initialize Kiva convection algorithms
    for (int SurfNum : state.dataSurface->AllHTKivaSurfaceList) {
        state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].in = KIVA_CONST_CONV(3.076);
        state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].f = KIVA_HF_DEF;
        state.dataSurfaceGeometry->kivaManager.surfaceConvMap[SurfNum].out = KIVA_CONST_CONV(0.0);
    }
    if (!state.dataHeatBal->SimpleCTFOnly || state.dataGlobal->AnyEnergyManagementSystemInModel) {
        for (int SurfNum : state.dataSurface->AllHTSurfaceList) {
            auto &surface = state.dataSurface->Surface(SurfNum);
            if ((surface.ExtBoundCond == DataSurfaces::ExternalEnvironment) || (surface.ExtBoundCond == DataSurfaces::OtherSideCondModeledExt)) {
                for (int CTFTermNum = 1; CTFTermNum <= Construction::MaxCTFTerms; ++CTFTermNum) {
                    state.dataHeatBalSurf->SurfOutsideTempHistMaster(CTFTermNum)(SurfNum) = state.dataSurface->SurfOutDryBulbTemp(SurfNum);
                }
            } else if (surface.ExtBoundCond == DataSurfaces::Ground) {
                for (int CTFTermNum = 1; CTFTermNum <= Construction::MaxCTFTerms; ++CTFTermNum) {
                    state.dataHeatBalSurf->SurfOutsideTempHistMaster(CTFTermNum)(SurfNum) =
                        state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::BuildingSurface];
                }
            } else if (surface.ExtBoundCond == DataSurfaces::GroundFCfactorMethod) {
                for (int CTFTermNum = 1; CTFTermNum <= Construction::MaxCTFTerms; ++CTFTermNum) {
                    state.dataHeatBalSurf->SurfOutsideTempHistMaster(CTFTermNum)(SurfNum) =
                        state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::FCFactorMethod];
                }
            }
            for (int CTFTermNum = 2; CTFTermNum <= state.dataConstruction->Construct(surface.Construction).NumCTFTerms + 1; ++CTFTermNum) {
                state.dataHeatBalSurf->SurfOutsideFluxHistMaster(CTFTermNum)(SurfNum) = state.dataHeatBalSurf->SurfOutsideFluxHist(2)(SurfNum);
                state.dataHeatBalSurf->SurfInsideFluxHistMaster(CTFTermNum)(SurfNum) = state.dataHeatBalSurf->SurfOutsideFluxHist(2)(SurfNum);
            }
        }
    }

    if (state.dataSurface->TotOSCM >= 1) {
        for (int OSCMnum = 1; OSCMnum <= state.dataSurface->TotOSCM; ++OSCMnum) {
            auto &thisOSC = state.dataSurface->OSCM(OSCMnum);
            thisOSC.TConv = 20.0;
            thisOSC.HConv = 4.0;
            thisOSC.TRad = 20.0;
            thisOSC.HRad = 4.0;
        }
    }
}

void EvalOutsideMovableInsulation(EnergyPlusData &state)
{
    // This subroutine determines whether or not outside movable insulation on opaque surfaces is present at the current time.
    auto &s_mat = state.dataMaterial;
    auto &s_surf = state.dataSurface;

    for (int SurfNum : s_surf->extMovInsulSurfNums) {
        auto &movInsul = s_surf->extMovInsuls(SurfNum);
        Real64 MovInsulSchedVal = movInsul.sched->getCurrentVal();
        if (MovInsulSchedVal <= 0) { // Movable insulation not present at current time
            movInsul.present = false;
            int ConstrNum = s_surf->SurfActiveConstruction(SurfNum);
            auto const *thisMaterial = s_mat->materials(state.dataConstruction->Construct(ConstrNum).LayerPoint(1));
            state.dataHeatBalSurf->SurfAbsSolarExt(SurfNum) = thisMaterial->AbsorpSolar;
            state.dataHeatBalSurf->SurfAbsThermalExt(SurfNum) = thisMaterial->AbsorpThermal;
            state.dataHeatBalSurf->SurfRoughnessExt(SurfNum) = thisMaterial->Roughness;
            continue;
        }

        auto const *mat = s_mat->materials(movInsul.matNum);
        movInsul.present = true;
        movInsul.H = 1.0 / (MovInsulSchedVal * mat->Resistance);
        if (mat->group == Material::Group::Glass || mat->group == Material::Group::GlassEQL) {
            auto const *matGlass = dynamic_cast<Material::MaterialFen const *>(mat);
            assert(matGlass != nullptr);
            state.dataHeatBalSurf->SurfAbsSolarExt(SurfNum) = max(0.0, 1.0 - matGlass->Trans - matGlass->ReflectSolBeamFront);
        } else {
            state.dataHeatBalSurf->SurfAbsSolarExt(SurfNum) = mat->AbsorpSolar;
        }
        state.dataHeatBalSurf->SurfAbsThermalExt(SurfNum) = mat->AbsorpThermal;
        state.dataHeatBalSurf->SurfRoughnessExt(SurfNum) = mat->Roughness;
    }
}

void EvalInsideMovableInsulation(EnergyPlusData &state)
{
    auto &s_mat = state.dataMaterial;
    auto &s_surf = state.dataSurface;
    // This subroutine determines whether or not inside movable insulation is present at the current time.
    for (int SurfNum : s_surf->intMovInsulSurfNums) {
        auto &movInsul = s_surf->intMovInsuls(SurfNum);
        Real64 MovInsulSchedVal = movInsul.sched->getCurrentVal();
        if (MovInsulSchedVal <= 0.0) { // Movable insulation not present at current time
            movInsul.present = false;
            int ConstrNum = s_surf->SurfActiveConstruction(SurfNum);
            auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);
            state.dataHeatBalSurf->SurfAbsSolarInt(SurfNum) = thisConstruct.InsideAbsorpSolar;
            state.dataHeatBalSurf->SurfAbsThermalInt(SurfNum) = thisConstruct.InsideAbsorpThermal;
            continue;
        }

        auto const *mat = s_mat->materials(movInsul.matNum);

        movInsul.present = true;
        movInsul.H = 1.0 / (MovInsulSchedVal * mat->Resistance);
        if (mat->group == Material::Group::Glass || mat->group == Material::Group::GlassEQL) { // Glass is insulating?
            auto const *matGlass = dynamic_cast<Material::MaterialFen const *>(mat);
            assert(matGlass != nullptr);
            state.dataHeatBalSurf->SurfAbsSolarInt(SurfNum) = max(0.0, 1.0 - matGlass->Trans - matGlass->ReflectSolBeamFront);
        } else {
            state.dataHeatBalSurf->SurfAbsSolarInt(SurfNum) = mat->AbsorpSolar;
        }
        state.dataHeatBalSurf->SurfAbsThermalInt(SurfNum) = mat->AbsorpThermal;
    }
}

void InitSolarHeatGains(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Anonymous
    //       DATE WRITTEN   July 1977
    //       MODIFIED       Mar99 (FW): handle movable interior shades and
    //                                  switchable glazing
    //                      Oct99 (FW): account for Window5 glass calculation approach
    //                      May01 (FW): handle interior and exterior blinds
    //                      Sep03 (FW): initialize SurfaceWindow%FrameQRadOutAbs
    //                      May06 (RR): handle exterior window screens
    //       RE-ENGINEERED  Feb98 (RKS)

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine initializes the arrays associated with solar heat
    // gains for both individual surfaces and for zones.  As a result,
    // this routine sets the following variable arrays:
    // QBV(unused), QDV, QC, QD; SurfOpaqQRadSWOutAbs and SurfOpaqQRadSWInAbs (for opaque surfaces);
    // SurfWinQRadSWwinAbs (for windows)

    // METHODOLOGY EMPLOYED:
    // If the sun is down, all of the pertinent arrays are zeroed.  If the
    // sun is up, various calculations are made.

    // REFERENCES:
    // (I)BLAST legacy routine QSUN

    auto &s_mat = state.dataMaterial;
    auto &Surface = state.dataSurface->Surface;

    // Using/Aliasing
    using Dayltg::TransTDD;
    using SolarShading::CalcInteriorSolarDistribution;
    using namespace DataWindowEquivalentLayer;
    using SolarShading::SurfaceScheduledSolarInc;
    using SolarShading::WindowScheduledSolarAbs;

    // Why are these globals?
    auto &AbsDiffWin = state.dataHeatBalSurfMgr->AbsDiffWin;
    auto &AbsDiffWinGnd = state.dataHeatBalSurfMgr->AbsDiffWinGnd;
    auto &AbsDiffWinSky = state.dataHeatBalSurfMgr->AbsDiffWinSky;

    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        state.dataHeatBal->ZoneWinHeatGainRepEnergy(zoneNum) = 0.0;
        state.dataHeatBal->ZoneWinHeatLossRepEnergy(zoneNum) = 0.0;
        state.dataHeatBal->ZnOpqSurfInsFaceCondGnRepEnrg(zoneNum) = 0.0;
        state.dataHeatBal->ZnOpqSurfInsFaceCondLsRepEnrg(zoneNum) = 0.0;
        state.dataHeatBal->ZnOpqSurfExtFaceCondGnRepEnrg(zoneNum) = 0.0;
        state.dataHeatBal->ZnOpqSurfExtFaceCondLsRepEnrg(zoneNum) = 0.0;

        state.dataHeatBal->ZoneWinHeatGain(zoneNum) = 0.0;
        state.dataHeatBal->ZoneWinHeatGainRep(zoneNum) = 0.0;
        state.dataHeatBal->ZoneWinHeatLossRep(zoneNum) = 0.0;
        state.dataHeatBal->ZoneOpaqSurfInsFaceCond(zoneNum) = 0.0;
        state.dataHeatBal->ZoneOpaqSurfInsFaceCondGainRep(zoneNum) = 0.0;
        state.dataHeatBal->ZoneOpaqSurfInsFaceCondLossRep(zoneNum) = 0.0;
        state.dataHeatBal->ZoneOpaqSurfExtFaceCond(zoneNum) = 0.0;
        state.dataHeatBal->ZoneOpaqSurfExtFaceCondGainRep(zoneNum) = 0.0;
        state.dataHeatBal->ZoneOpaqSurfExtFaceCondLossRep(zoneNum) = 0.0;
    }
    for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
        state.dataHeatBal->EnclSolInitialDifSolReflW(enclNum) = 0.0;
    }
    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurfOpaq = thisSpace.OpaqOrIntMassSurfaceFirst;
            int const lastSurfOpaq = thisSpace.OpaqOrIntMassSurfaceLast;
            for (int SurfNum = firstSurfOpaq; SurfNum <= lastSurfOpaq; ++SurfNum) {
                state.dataHeatBalSurf->SurfOpaqInsFaceCondGainRep(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfOpaqInsFaceCondLossRep(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQdotRadLightsInPerArea(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfOpaqQRadSWOutAbs(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfOpaqInitialDifSolInAbs(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfOpaqInsFaceBeamSolAbsorbed(SurfNum) = 0.0;
                state.dataHeatBal->SurfOpaqSWOutAbsTotalReport(SurfNum) = 0.0;
                state.dataHeatBal->SurfOpaqSWOutAbsEnergyReport(SurfNum) = 0.0;
            }

            int const firstSurfWin = thisSpace.WindowSurfaceFirst;
            int const lastSurfWin = thisSpace.WindowSurfaceLast;
            for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {
                // Faster "inline" than calling SurfaceWindow( SurfNum ).InitSolarHeatGains()
                state.dataSurface->SurfWinFrameQRadOutAbs(SurfNum) = 0.0;
                state.dataSurface->SurfWinFrameQRadInAbs(SurfNum) = 0.0;
                state.dataSurface->SurfWinDividerQRadOutAbs(SurfNum) = 0.0;
                state.dataSurface->SurfWinDividerQRadInAbs(SurfNum) = 0.0;
                state.dataSurface->SurfWinIntSWAbsByShade(SurfNum) = 0.0;
                state.dataSurface->SurfWinIntLWAbsByShade(SurfNum) = 0.0;
                state.dataSurface->SurfWinConvHeatFlowNatural(SurfNum) = 0.0;
                state.dataSurface->SurfWinConvHeatGainToZoneAir(SurfNum) = 0.0;
                state.dataSurface->SurfWinRetHeatGainToZoneAir(SurfNum) = 0.0;
                state.dataSurface->SurfWinDividerHeatGain(SurfNum) = 0.0;
            }

            for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {
                state.dataSurface->SurfWinGainConvGlazToZoneRep(SurfNum) = 0.0;
                state.dataSurface->SurfWinGainIRGlazToZoneRep(SurfNum) = 0.0;
                state.dataSurface->SurfWinLossSWZoneToOutWinRep(SurfNum) = 0.0;
                state.dataSurface->SurfWinGainFrameDividerToZoneRep(SurfNum) = 0.0;
                state.dataSurface->SurfWinGainConvShadeToZoneRep(SurfNum) = 0.0;
                state.dataSurface->SurfWinGainIRShadeToZoneRep(SurfNum) = 0.0;
                state.dataSurface->SurfWinGapConvHtFlowRep(SurfNum) = 0.0;
                state.dataSurface->SurfWinShadingAbsorbedSolar(SurfNum) = 0.0;
                state.dataSurface->SurfWinSysSolTransmittance(SurfNum) = 0.0;
            }
            for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {
                state.dataSurface->SurfWinHeatGain(SurfNum) = 0.0;
                state.dataSurface->SurfWinHeatGainRep(SurfNum) = 0.0;
                state.dataSurface->SurfWinHeatLossRep(SurfNum) = 0.0;
                state.dataSurface->SurfWinHeatGainRepEnergy(SurfNum) = 0.0;
                state.dataSurface->SurfWinHeatLossRepEnergy(SurfNum) = 0.0;
                state.dataSurface->SurfWinGapConvHtFlowRepEnergy(SurfNum) = 0.0;
                state.dataSurface->SurfWinShadingAbsorbedSolarEnergy(SurfNum) = 0.0;
            }
            for (int Lay = 1; Lay <= DataWindowEquivalentLayer::CFSMAXNL + 1; Lay++) {
                for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {
                    state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, Lay) = 0.0;
                }
            }
        }
    }
    if (state.dataHeatBalSurfMgr->InitSurfaceHeatBalancefirstTime) {
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            state.dataSurface->SurfBmToDiffReflFacGnd(SurfNum) = Surface(SurfNum).ViewFactorGround;
            state.dataSurface->SurfSkyDiffReflFacGnd(SurfNum) = Surface(SurfNum).ViewFactorGround;
        }
    }
    bool currSolRadPositive =
        state.dataEnvrn->SunIsUp && (state.dataEnvrn->BeamSolarRad + state.dataEnvrn->GndSolarRad + state.dataEnvrn->DifSolarRad > 0.0);
    bool sunset = (!currSolRadPositive) && state.dataEnvrn->PreviousSolRadPositive;
    bool sunIsUpNoRad = state.dataEnvrn->SunIsUp && (!currSolRadPositive);
    bool resetSolar = state.dataGlobal->BeginEnvrnFlag || sunIsUpNoRad ||
                      sunset; // Reset at (1) Beginning of simulation (2) sunset time, and SunIsUp but not solar time.
    state.dataEnvrn->PreviousSolRadPositive = currSolRadPositive;

    if (currSolRadPositive || resetSolar) {
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            state.dataHeatBal->SurfBmIncInsSurfIntensRep(SurfNum) = 0.0;
            state.dataHeatBal->SurfBmIncInsSurfAmountRep(SurfNum) = 0.0;
            state.dataHeatBal->SurfIntBmIncInsSurfIntensRep(SurfNum) = 0.0;
            state.dataHeatBal->SurfIntBmIncInsSurfAmountRep(SurfNum) = 0.0;
            state.dataHeatBal->SurfIntBmIncInsSurfAmountRepEnergy(SurfNum) = 0.0;

            state.dataHeatBal->SurfQRadSWOutIncident(SurfNum) = 0.0;
            state.dataHeatBal->SurfQRadSWOutIncidentBeam(SurfNum) = 0.0;
            state.dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(SurfNum) = 0.0;
            state.dataHeatBal->SurfQRadSWOutIncidentGndDiffuse(SurfNum) = 0.0;

            state.dataHeatBal->SurfQRadSWOutIncBmToDiffReflGnd(SurfNum) = 0.0;
            state.dataHeatBal->SurfQRadSWOutIncSkyDiffReflGnd(SurfNum) = 0.0;
            state.dataHeatBal->SurfQRadSWOutIncBmToBmReflObs(SurfNum) = 0.0;
            state.dataHeatBal->SurfQRadSWOutIncBmToDiffReflObs(SurfNum) = 0.0;
            state.dataHeatBal->SurfQRadSWOutIncSkyDiffReflObs(SurfNum) = 0.0;

            state.dataSurface->SurfSkySolarInc(SurfNum) = 0.0;
            state.dataSurface->SurfGndSolarInc(SurfNum) = 0.0;
        }
        for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
            state.dataHeatBal->ZoneTransSolar(enclNum) = 0.0;
            state.dataHeatBal->ZoneBmSolFrExtWinsRep(enclNum) = 0.0;
            state.dataHeatBal->ZoneBmSolFrIntWinsRep(enclNum) = 0.0;
            state.dataHeatBal->ZoneDifSolFrExtWinsRep(enclNum) = 0.0;
            state.dataHeatBal->ZoneDifSolFrIntWinsRep(enclNum) = 0.0;
            state.dataHeatBal->ZoneTransSolarEnergy(enclNum) = 0.0;
            state.dataHeatBal->ZoneBmSolFrExtWinsRepEnergy(enclNum) = 0.0;
            state.dataHeatBal->ZoneBmSolFrIntWinsRepEnergy(enclNum) = 0.0;
            state.dataHeatBal->ZoneDifSolFrExtWinsRepEnergy(enclNum) = 0.0;
            state.dataHeatBal->ZoneDifSolFrIntWinsRepEnergy(enclNum) = 0.0;
        }
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                int const firstSurfWin = thisSpace.WindowSurfaceFirst;
                int const lastSurfWin = thisSpace.WindowSurfaceLast;
                for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {
                    state.dataSurface->SurfWinExtBeamAbsByShade(SurfNum) = 0.0;
                    state.dataSurface->SurfWinExtDiffAbsByShade(SurfNum) = 0.0;
                    state.dataSurface->SurfWinIntBeamAbsByShade(SurfNum) = 0.0;
                    state.dataSurface->SurfWinInitialDifSolAbsByShade(SurfNum) = 0.0;
                    state.dataHeatBal->SurfWinQRadSWwinAbsTot(SurfNum) = 0.0;
                    state.dataHeatBal->SurfWinQRadSWwinAbsTotEnergy(SurfNum) = 0.0;
                    state.dataHeatBal->SurfWinSWwinAbsTotalReport(SurfNum) = 0.0;
                    state.dataHeatBalSurf->SurfWinInitialDifSolInTrans(SurfNum) = 0.0;
                    state.dataHeatBalSurf->SurfWinInitialBeamSolInTrans(SurfNum) = 0.0;
                    state.dataHeatBal->SurfWinInitialDifSolInTransReport(SurfNum) = 0.0;
                }
                for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {
                    state.dataSurface->SurfWinBlTsolBmBm(SurfNum) = 0.0;
                    state.dataSurface->SurfWinBlTsolBmDif(SurfNum) = 0.0;
                    state.dataSurface->SurfWinBlTsolDifDif(SurfNum) = 0.0;
                    state.dataSurface->SurfWinBlGlSysTsolBmBm(SurfNum) = 0.0;
                    state.dataSurface->SurfWinBlGlSysTsolDifDif(SurfNum) = 0.0;
                    state.dataSurface->SurfWinScTsolBmBm(SurfNum) = 0.0;
                    state.dataSurface->SurfWinScTsolBmDif(SurfNum) = 0.0;
                    state.dataSurface->SurfWinScTsolDifDif(SurfNum) = 0.0;
                    state.dataSurface->SurfWinScGlSysTsolBmBm(SurfNum) = 0.0;
                    state.dataSurface->SurfWinScGlSysTsolDifDif(SurfNum) = 0.0;
                    state.dataSurface->SurfWinGlTsolBmBm(SurfNum) = 0.0;
                    state.dataSurface->SurfWinGlTsolBmDif(SurfNum) = 0.0;
                    state.dataSurface->SurfWinGlTsolDifDif(SurfNum) = 0.0;
                }
                for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {
                    state.dataSurface->SurfWinBmSolTransThruIntWinRep(SurfNum) = 0.0;
                    state.dataSurface->SurfWinBmSolAbsdOutsReveal(SurfNum) = 0.0;
                    state.dataSurface->SurfWinBmSolAbsdInsReveal(SurfNum) = 0.0;
                    state.dataSurface->SurfWinBmSolRefldInsReveal(SurfNum) = 0.0;
                    state.dataSurface->SurfWinOutsRevealDiffOntoGlazing(SurfNum) = 0.0;
                    state.dataSurface->SurfWinInsRevealDiffOntoGlazing(SurfNum) = 0.0;
                    state.dataSurface->SurfWinInsRevealDiffIntoZone(SurfNum) = 0.0;
                    state.dataSurface->SurfWinOutsRevealDiffOntoFrame(SurfNum) = 0.0;
                    state.dataSurface->SurfWinInsRevealDiffOntoFrame(SurfNum) = 0.0;
                }
                for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {
                    state.dataSurface->SurfWinBmSolRefldOutsRevealReport(SurfNum) = 0.0;
                    state.dataSurface->SurfWinBmSolRefldInsRevealReport(SurfNum) = 0.0;
                    state.dataSurface->SurfWinBmSolAbsdInsRevealReport(SurfNum) = 0.0;
                    state.dataSurface->SurfWinInsRevealDiffOntoGlazingReport(SurfNum) = 0.0;
                    state.dataSurface->SurfWinInsRevealDiffIntoZoneReport(SurfNum) = 0.0;
                    state.dataSurface->SurfWinInsRevealDiffOntoFrameReport(SurfNum) = 0.0;
                    state.dataSurface->SurfWinBmSolTransThruIntWinRepEnergy(SurfNum) = 0.0;
                    state.dataSurface->SurfWinBmSolRefldOutsRevealRepEnergy(SurfNum) = 0.0;
                    state.dataSurface->SurfWinBmSolRefldInsRevealRepEnergy(SurfNum) = 0.0;
                }

                for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {
                    state.dataSurface->SurfWinTransSolar(SurfNum) = 0.0;
                    state.dataSurface->SurfWinBmSolar(SurfNum) = 0.0;
                    state.dataSurface->SurfWinBmBmSolar(SurfNum) = 0.0;
                    state.dataSurface->SurfWinBmDifSolar(SurfNum) = 0.0;
                    state.dataSurface->SurfWinDifSolar(SurfNum) = 0.0;
                    state.dataSurface->SurfWinTransSolarEnergy(SurfNum) = 0.0;
                    state.dataSurface->SurfWinBmSolarEnergy(SurfNum) = 0.0;
                    state.dataSurface->SurfWinBmBmSolarEnergy(SurfNum) = 0.0;
                    state.dataSurface->SurfWinBmDifSolarEnergy(SurfNum) = 0.0;
                    state.dataSurface->SurfWinDifSolarEnergy(SurfNum) = 0.0;
                    state.dataHeatBal->SurfWinBSDFBeamDirectionRep(SurfNum) = 0;
                    state.dataHeatBal->SurfWinBSDFBeamThetaRep(SurfNum) = 0.0;
                    state.dataHeatBal->SurfWinBSDFBeamPhiRep(SurfNum) = 0.0;
                }
                for (int Lay = 1; Lay <= state.dataHeatBal->MaxSolidWinLayers; Lay++) {
                    for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {
                        state.dataHeatBal->SurfWinQRadSWwinAbsLayer(SurfNum, Lay) = 0.0;
                    }
                }
                for (int Lay = 1; Lay <= DataWindowEquivalentLayer::CFSMAXNL; Lay++) {
                    for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {
                        state.dataHeatBal->SurfWinInitialDifSolwinAbs(SurfNum, Lay) = 0.0;
                    }
                }
            }
        }
    }
    if (resetSolar) {
        for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclosureNum) {
            state.dataHeatBal->EnclSolQD(enclosureNum) = 0.0;
            state.dataHeatBal->EnclSolQDforDaylight(enclosureNum) = 0.0;
        }

        // TTD domes are currently not considered in the window list of a zone
        if ((int)state.dataDaylightingDevicesData->TDDPipe.size() > 0) {
            for (auto &e : state.dataDaylightingDevicesData->TDDPipe) {
                e.TransSolBeam = 0.0;
                e.TransSolDiff = 0.0;
                e.TransVisBeam = 0.0;
                e.TransVisDiff = 0.0;
                e.TransmittedSolar = 0.0;
                int SurfDome = e.Dome;
                state.dataSurface->SurfWinTransSolar(SurfDome) = 0.0;
                state.dataHeatBal->SurfQRadSWOutIncident(SurfDome) = 0.0;
                state.dataHeatBal->SurfWinQRadSWwinAbsTot(SurfDome) = 0.0;
                for (int Lay = 1; Lay <= DataWindowEquivalentLayer::CFSMAXNL + 1; Lay++) {
                    state.dataHeatBal->SurfWinQRadSWwinAbs(SurfDome, Lay) = 0.0;
                }
            }
        }

        if (state.dataSurface->CalcSolRefl) {
            for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
                state.dataSurface->SurfBmToBmReflFacObs(SurfNum) = 0.0;
                state.dataSurface->SurfBmToDiffReflFacObs(SurfNum) = 0.0;
                state.dataSurface->SurfBmToDiffReflFacGnd(SurfNum) = 0.0;
            }
        }
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            state.dataHeatBal->SurfInitialDifSolInAbsReport(SurfNum) = 0.0;
            state.dataHeatBal->SurfCosIncidenceAngle(SurfNum) = 0.0;
            state.dataHeatBal->SurfSWInAbsTotalReport(SurfNum) = 0.0;
            state.dataSurface->SurfWinProfileAngHor(SurfNum) = 0.0;
            state.dataSurface->SurfWinProfileAngVert(SurfNum) = 0.0;
            state.dataSurface->SurfWinSysSolReflectance(SurfNum) = 0.0;
            state.dataSurface->SurfWinSysSolAbsorptance(SurfNum) = 0.0;
        }
    }
    if (currSolRadPositive) { // Sun is up, calculate solar quantities
        assert(equal_dimensions(state.dataSurface->SurfReflFacBmToBmSolObs,
                                state.dataSurface->SurfReflFacBmToDiffSolObs)); // For linear indexing
        assert(equal_dimensions(state.dataSurface->SurfReflFacBmToBmSolObs,
                                state.dataSurface->SurfReflFacBmToDiffSolGnd)); // For linear indexing
        Real64 GndReflSolarRad = 0.0;
        Real64 GndSolarRadInc = max(state.dataEnvrn->BeamSolarRad * state.dataEnvrn->SOLCOS(3) + state.dataEnvrn->DifSolarRad, 0.0);

        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            state.dataSurface->Surface(SurfNum).IncSolMultiplier = GetSurfIncidentSolarMultiplier(state, SurfNum);
        }
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            Real64 SurfIncSolarMultiplier = state.dataSurface->Surface(SurfNum).IncSolMultiplier;
            state.dataSurface->SurfSkySolarInc(SurfNum) =
                state.dataEnvrn->DifSolarRad * SurfIncSolarMultiplier * state.dataSolarShading->SurfAnisoSkyMult(SurfNum);
            if (state.dataSurface->Surface(SurfNum).UseSurfPropertyGndSurfRefl) {
                GndReflSolarRad = GndSolarRadInc * SurfIncSolarMultiplier *
                                  state.dataSurface->GroundSurfsProperty(state.dataSurface->Surface(SurfNum).SurfPropertyGndSurfIndex).SurfsReflAvg;
                Surface(SurfNum).GndReflSolarRad = GndReflSolarRad;
            } else {
                GndReflSolarRad = state.dataEnvrn->GndSolarRad * SurfIncSolarMultiplier;
            }
            state.dataSurface->SurfGndSolarInc(SurfNum) = GndReflSolarRad * Surface(SurfNum).ViewFactorGround;
            state.dataSurface->SurfWinSkyGndSolarInc(SurfNum) = state.dataSurface->SurfGndSolarInc(SurfNum);
            state.dataSurface->SurfWinBmGndSolarInc(SurfNum) = 0.0;
        }
        if (state.dataSurface->CalcSolRefl) {
            // [ lSH ] == ( HourOfDay, SurfNum ) // [ lSP ] == ( PreviousHour, SurfNum )
            Array1D<Real64>::size_type lSH = state.dataSurface->SurfReflFacBmToBmSolObs.index(state.dataGlobal->HourOfDay, 1) - 1;
            Array1D<Real64>::size_type lSP = state.dataSurface->SurfReflFacBmToBmSolObs.index(state.dataGlobal->PreviousHour, 1) - 1;
            // For Complex Fenestrations:
            for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
                Real64 SurfIncSolarMultiplier = state.dataSurface->Surface(SurfNum).IncSolMultiplier;

                Real64 GndSurfReflectance = 0.0;
                if (state.dataSurface->Surface(SurfNum).UseSurfPropertyGndSurfRefl) {
                    GndSurfReflectance =
                        state.dataSurface->GroundSurfsProperty(state.dataSurface->Surface(SurfNum).SurfPropertyGndSurfIndex).SurfsReflAvg;
                } else {
                    GndSurfReflectance = state.dataEnvrn->GndReflectance;
                }
                Real64 currDifSolarRad = state.dataEnvrn->DifSolarRad * SurfIncSolarMultiplier;
                Real64 currBeamSolarRad = state.dataEnvrn->BeamSolarRad * SurfIncSolarMultiplier;
                state.dataSurface->SurfWinSkyGndSolarInc(SurfNum) =
                    currDifSolarRad * GndSurfReflectance * state.dataSurface->SurfReflFacSkySolGnd(SurfNum);
                state.dataSurface->SurfWinBmGndSolarInc(SurfNum) =
                    currBeamSolarRad * state.dataEnvrn->SOLCOS(3) * GndSurfReflectance * state.dataSurface->SurfBmToDiffReflFacGnd(SurfNum);
                state.dataSurface->SurfBmToBmReflFacObs(SurfNum) =
                    state.dataGlobal->WeightNow * state.dataSurface->SurfReflFacBmToBmSolObs[lSH + SurfNum] +
                    state.dataGlobal->WeightPreviousHour * state.dataSurface->SurfReflFacBmToBmSolObs[lSP + SurfNum];
                state.dataSurface->SurfBmToDiffReflFacObs(SurfNum) =
                    state.dataGlobal->WeightNow * state.dataSurface->SurfReflFacBmToDiffSolObs[lSH + SurfNum] +
                    state.dataGlobal->WeightPreviousHour * state.dataSurface->SurfReflFacBmToDiffSolObs[lSP + SurfNum];
                state.dataSurface->SurfBmToDiffReflFacGnd(SurfNum) =
                    state.dataGlobal->WeightNow * state.dataSurface->SurfReflFacBmToDiffSolGnd[lSH + SurfNum] +
                    state.dataGlobal->WeightPreviousHour * state.dataSurface->SurfReflFacBmToDiffSolGnd[lSP + SurfNum];
                // TH2 CR 9056
                state.dataSurface->SurfSkySolarInc(SurfNum) +=
                    currBeamSolarRad * (state.dataSurface->SurfBmToBmReflFacObs(SurfNum) + state.dataSurface->SurfBmToDiffReflFacObs(SurfNum)) +
                    currDifSolarRad * state.dataSurface->SurfReflFacSkySolObs(SurfNum);
                state.dataSurface->SurfGndSolarInc(SurfNum) =
                    currBeamSolarRad * state.dataEnvrn->SOLCOS(3) * GndSurfReflectance * state.dataSurface->SurfBmToDiffReflFacGnd(SurfNum) +
                    currDifSolarRad * GndSurfReflectance * state.dataSurface->SurfReflFacSkySolGnd(SurfNum);
                state.dataSurface->SurfSkyDiffReflFacGnd(SurfNum) = state.dataSurface->SurfReflFacSkySolGnd(SurfNum);
            }
        }

        SolarShading::CalcWindowProfileAngles(state);

        if (state.dataHeatBal->CalcWindowRevealReflection) {
            SolarShading::CalcBeamSolarOnWinRevealSurface(state);
        }

        if (state.dataWindowManager->inExtWindowModel->isExternalLibraryModel() && state.dataWindowManager->winOpticalModel->isSimplifiedModel()) {
            SolarShading::CalcAbsorbedOnExteriorOpaqueSurfaces(state);
            if (state.dataWindowManager->winOpticalModel->isSimplifiedModel()) {
                SolarShading::CalcInteriorSolarDistributionWCESimple(state);
            }
        } else {
            SolarShading::CalcInteriorSolarDistribution(state);
        }

        for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclosureNum) {

            // TH 3/24/2010 - QBV is not used!
            // unused      QBV(ZoneNum) = (CBZone(ZoneNum) + EnclSolDB(ZoneNum))*BeamSolarRad

            // RJH 08/30/07 - QDV does not seem to ever be used. NOT USED!
            // QDV(ZoneNum) = EnclSolDS(ZoneNum)*DifSolarRad &
            //                +EnclSolDG(ZoneNum)*GndSolarRad

            // Original QD calc used only for EnclSolQSDifSol and daylighting calcs
            // QDforDaylight(ZoneNum)  = EnclSolDB(ZoneNum)*BeamSolarRad  &
            //                          +EnclSolDS(ZoneNum)*DifSolarRad  &
            //                          +EnclSolDG(ZoneNum)*GndSolarRad

            //  Beam from interior windows (EnclSolDBIntWin) reflected from floor is counted in DayltgInterReflIllFrIntWins,
            //  EnclSolDB needs to subtract this part since it is already counted in EnclSolDB.
            //  Use EnclSolInitialDifSolReflW (Rob's previous work) as it better counts initial distribution of
            //   diffuse solar rather than using weighted area*absorptance
            state.dataHeatBal->EnclSolQDforDaylight(enclosureNum) =
                (state.dataHeatBal->EnclSolDB(enclosureNum) - state.dataHeatBal->EnclSolDBIntWin(enclosureNum)) * state.dataEnvrn->BeamSolarRad +
                state.dataHeatBal->EnclSolDBSSG(enclosureNum) + state.dataHeatBal->EnclSolInitialDifSolReflW(enclosureNum);

            // to exclude diffuse solar now absorbed/transmitted in CalcWinTransDifSolInitialDistribution
            // EnclSolDB(ZoneNum) is Diffuse Solar from beam reflected from interior surfaces
            // and transmitted through interior windows
            // EnclSolDB is a factor that when multiplied by BeamSolarRad [W/m2] gives Watts
            // QD(ZoneNum)  = EnclSolDB(ZoneNum)*BeamSolarRad  &
            //                +EnclSolDS(ZoneNum)*DifSolarRad  &
            //                +EnclSolDG(ZoneNum)*GndSolarRad
            state.dataHeatBal->EnclSolQD(enclosureNum) = state.dataHeatBal->EnclSolDB(enclosureNum) * state.dataEnvrn->BeamSolarRad +
                                                         state.dataHeatBal->EnclSolDBSSG(enclosureNum) +
                                                         state.dataHeatBal->EnclSolInitialDifSolReflW(enclosureNum);
        }

        // Flux of diffuse solar in each zone

        for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
            state.dataHeatBal->EnclSolQSDifSol(enclNum) = state.dataHeatBal->EnclSolQDforDaylight(enclNum);
        }

        if (state.dataHeatBalSurf->InterZoneWindow) {
            for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
                if (state.dataHeatBalSurf->EnclSolRecDifShortFromZ(enclNum)) {
                    Real64 EnclSolQSDifSol_sum(0.0); // Accumulator
                    int lZone(state.dataHeatBalSurf->ZoneFractDifShortZtoZ.index(enclNum,
                                                                                 1)); // Tuned Linear indexing
                    for (int otherEnclNum = 1; otherEnclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++otherEnclNum, ++lZone) {
                        if ((otherEnclNum != enclNum) && (state.dataHeatBalSurf->EnclSolRecDifShortFromZ(otherEnclNum))) {
                            EnclSolQSDifSol_sum += state.dataHeatBalSurf->ZoneFractDifShortZtoZ[lZone] *
                                                   state.dataHeatBal->EnclSolQDforDaylight(otherEnclNum); // [ lZone ] == ( enclNum, otherEnclNum )
                        }
                    }
                    state.dataHeatBal->EnclSolQSDifSol(enclNum) += EnclSolQSDifSol_sum;
                }
            }
        }

        for (int enclNum = 1; enclNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclNum) {
            if (state.dataHeatBalSurf->InterZoneWindow) {
                state.dataHeatBal->EnclSolQSDifSol(enclNum) *=
                    state.dataHeatBalSurf->ZoneFractDifShortZtoZ(enclNum, enclNum) * state.dataViewFactor->EnclSolInfo(enclNum).solVMULT;
            } else {
                state.dataHeatBal->EnclSolQSDifSol(enclNum) *= state.dataViewFactor->EnclSolInfo(enclNum).solVMULT;
            }
        }

        //    RJH - 09-12-07 commented out report variable calcs here since they refer to old distribution method
        //    DO SurfNum = 1, TotSurfaces
        //      IF (.NOT. Surface(SurfNum)%HeatTransSurf) CYCLE
        //!!! Following may need to be removed or changed when shelves are considered in adjacent reflection calculations
        //      IF (Surface(SurfNum)%Class == SurfaceClass::Shading) CYCLE
        //      ZoneNum = Surface(SurfNum)%Zone
        // Diffuse solar entering zone through exterior windows is assumed to be uniformly
        // distributed on inside face of surfaces of zone
        //      DifIncInsSurfIntensRep(SurfNum) = (EnclSolDS(ZoneNum)*DifSolarRad + EnclSolDG(ZoneNum)*GndSolarRad) /  &
        //        Zone(ZoneNum)%TotalSurfArea
        //      DifIncInsSurfAmountRep(SurfNum) = (Surface(SurfNum)%Area + SurfaceWindow(SurfNum)%DividerArea) *  &
        //        DifIncInsSurfIntensRep(SurfNum)
        //      DifIncInsSurfAmountRepEnergy(SurfNum) = DifIncInsSurfAmountRep(SurfNum) * TimeStepZoneSec
        //    END DO

        // Calculate Exterior Incident Short Wave (i.e. Solar) Radiation on shading surfaces
        if (state.dataSurface->BuildingShadingCount || state.dataSurface->FixedShadingCount || state.dataSurface->AttachedShadingCount) {
            for (int SurfNum = state.dataSurface->ShadingSurfaceFirst; SurfNum <= state.dataSurface->ShadingSurfaceLast; SurfNum++) {
                Real64 GndSurfReflectance = 0.0;
                if (state.dataSurface->Surface(SurfNum).UseSurfPropertyGndSurfRefl) {
                    GndSurfReflectance =
                        state.dataSurface->GroundSurfsProperty(state.dataSurface->Surface(SurfNum).SurfPropertyGndSurfIndex).SurfsReflAvg;
                } else {
                    GndSurfReflectance = state.dataEnvrn->GndReflectance;
                }
                // Cosine of incidence angle and solar incident on outside of surface, for reporting
                Real64 CosInc = state.dataHeatBal->SurfCosIncAng(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum);
                state.dataHeatBal->SurfCosIncidenceAngle(SurfNum) = CosInc;
                Real64 SurfIncSolarMultiplier = state.dataSurface->Surface(SurfNum).IncSolMultiplier;
                Real64 currDifSolarRad = state.dataEnvrn->DifSolarRad * SurfIncSolarMultiplier;
                Real64 currBeamSolarRad = state.dataEnvrn->BeamSolarRad * SurfIncSolarMultiplier;
                // Incident direct (unreflected) beam
                state.dataHeatBal->SurfQRadSWOutIncidentBeam(SurfNum) =
                    currBeamSolarRad * state.dataHeatBal->SurfSunlitFrac(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum) * CosInc;
                // Incident (unreflected) diffuse solar from sky -- TDD_Diffuser calculated differently
                state.dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(SurfNum) = currDifSolarRad * state.dataSolarShading->SurfAnisoSkyMult(SurfNum);
                // Incident diffuse solar from sky diffuse reflected from ground plus beam reflected from ground
                state.dataHeatBal->SurfQRadSWOutIncidentGndDiffuse(SurfNum) = state.dataSurface->SurfGndSolarInc(SurfNum);
                // Incident diffuse solar from beam-to-diffuse reflection from ground
                state.dataHeatBal->SurfQRadSWOutIncBmToDiffReflGnd(SurfNum) =
                    currBeamSolarRad * state.dataEnvrn->SOLCOS(3) * GndSurfReflectance * state.dataSurface->SurfBmToDiffReflFacGnd(SurfNum);
                // Incident diffuse solar from sky diffuse reflection from ground
                state.dataHeatBal->SurfQRadSWOutIncSkyDiffReflGnd(SurfNum) =
                    currDifSolarRad * GndSurfReflectance * state.dataSurface->SurfSkyDiffReflFacGnd(SurfNum);
                // Total incident solar. Beam and sky reflection from obstructions, if calculated, is included
                // in SkySolarInc.
                state.dataHeatBal->SurfQRadSWOutIncident(SurfNum) =
                    state.dataHeatBal->SurfQRadSWOutIncidentBeam(SurfNum) + state.dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(SurfNum) +
                    state.dataHeatBal->SurfQRadSWOutIncBmToDiffReflGnd(SurfNum) + state.dataHeatBal->SurfQRadSWOutIncSkyDiffReflGnd(SurfNum);
            }
        }

        Array1D<Real64> currBeamSolar(state.dataSurface->TotSurfaces); // Local variable for BeamSolarRad

        for (int SurfNum : state.dataSurface->AllExtSolarSurfaceList) {
            Real64 SurfIncSolarMultiplier = state.dataSurface->Surface(SurfNum).IncSolMultiplier;
            // Regular surface
            currBeamSolar(SurfNum) = state.dataEnvrn->BeamSolarRad * SurfIncSolarMultiplier;
            Real64 currDifSolarRad = state.dataEnvrn->DifSolarRad * SurfIncSolarMultiplier;
            // Cosine of incidence angle and solar incident on outside of surface, for reporting
            state.dataHeatBal->SurfCosIncidenceAngle(SurfNum) =
                state.dataHeatBal->SurfCosIncAng(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum);

            // Report variables for various incident solar quantities
            // Incident direct (unreflected) beam
            state.dataHeatBal->SurfQRadSWOutIncidentBeam(SurfNum) =
                currBeamSolar(SurfNum) * state.dataHeatBal->SurfSunlitFrac(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum) *
                state.dataHeatBal->SurfCosIncidenceAngle(SurfNum);

            Real64 GndSurfReflectance = 0.0;
            if (state.dataSurface->Surface(SurfNum).UseSurfPropertyGndSurfRefl) {
                GndSurfReflectance =
                    state.dataSurface->GroundSurfsProperty(state.dataSurface->Surface(SurfNum).SurfPropertyGndSurfIndex).SurfsReflAvg;
            } else {
                GndSurfReflectance = state.dataEnvrn->GndReflectance;
            }
            // Incident (unreflected) diffuse solar from sky -- TDD_Diffuser calculated differently
            state.dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(SurfNum) = currDifSolarRad * state.dataSolarShading->SurfAnisoSkyMult(SurfNum);
            // Incident diffuse solar from sky diffuse reflected from ground plus beam reflected from ground
            state.dataHeatBal->SurfQRadSWOutIncidentGndDiffuse(SurfNum) = state.dataSurface->SurfGndSolarInc(SurfNum);
            // Incident diffuse solar from beam-to-diffuse reflection from ground
            state.dataHeatBal->SurfQRadSWOutIncBmToDiffReflGnd(SurfNum) =
                currBeamSolar(SurfNum) * state.dataEnvrn->SOLCOS(3) * GndSurfReflectance * state.dataSurface->SurfBmToDiffReflFacGnd(SurfNum);

            // Incident diffuse solar from sky diffuse reflection from ground
            state.dataHeatBal->SurfQRadSWOutIncSkyDiffReflGnd(SurfNum) =
                currDifSolarRad * GndSurfReflectance * state.dataSurface->SurfSkyDiffReflFacGnd(SurfNum);
            // Total incident solar. Beam and sky reflection from obstructions, if calculated, is included
            // in SkySolarInc.
            // SurfQRadSWOutIncident(SurfNum) = SurfQRadSWOutIncidentBeam(SurfNum) + SkySolarInc + GndSolarInc
            // TH2 CR 9056
            state.dataHeatBal->SurfQRadSWOutIncident(SurfNum) =
                state.dataHeatBal->SurfQRadSWOutIncidentBeam(SurfNum) + state.dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(SurfNum) +
                state.dataHeatBal->SurfQRadSWOutIncBmToDiffReflGnd(SurfNum) + state.dataHeatBal->SurfQRadSWOutIncSkyDiffReflGnd(SurfNum);

            if (state.dataSurface->CalcSolRefl) {
                // Incident beam solar from beam-to-beam (specular) reflection from obstructions
                state.dataHeatBal->SurfQRadSWOutIncBmToBmReflObs(SurfNum) = state.dataSurface->SurfBmToBmReflFacObs(SurfNum) * currBeamSolar(SurfNum);
                // Incident diffuse solar from beam-to-diffuse reflection from obstructions
                state.dataHeatBal->SurfQRadSWOutIncBmToDiffReflObs(SurfNum) =
                    state.dataSurface->SurfBmToDiffReflFacObs(SurfNum) * currBeamSolar(SurfNum);
                // Incident diffuse solar from sky diffuse reflection from obstructions
                state.dataHeatBal->SurfQRadSWOutIncSkyDiffReflObs(SurfNum) = currDifSolarRad * state.dataSurface->SurfReflFacSkySolObs(SurfNum);
                // TH2 CR 9056: Add reflections from obstructions to the total incident
                state.dataHeatBal->SurfQRadSWOutIncident(SurfNum) += state.dataHeatBal->SurfQRadSWOutIncBmToBmReflObs(SurfNum) +
                                                                     state.dataHeatBal->SurfQRadSWOutIncBmToDiffReflObs(SurfNum) +
                                                                     state.dataHeatBal->SurfQRadSWOutIncSkyDiffReflObs(SurfNum);
            }
        }
        for (int PipeNum = 1; PipeNum <= (int)state.dataDaylightingDevicesData->TDDPipe.size(); ++PipeNum) {
            int const SurfNum = state.dataDaylightingDevicesData->TDDPipe(PipeNum).Diffuser; // TDD: Diffuser object number
            int const SurfNum2 = state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome;    // TDD: DOME object number
            int const ConstrNum = state.dataSurface->SurfActiveConstruction(SurfNum);
            auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);

            // Reconstruct the beam, sky, and ground radiation transmittance of just the TDD:DOME and TDD pipe
            // by dividing out diffuse solar transmittance of TDD:DIFFUSER
            Real64 ConInc = state.dataHeatBal->SurfCosIncAng(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum2);

            state.dataHeatBal->SurfCosIncidenceAngle(SurfNum) = ConInc;
            Real64 SurfIncSolarMultiplier = state.dataSurface->Surface(SurfNum).IncSolMultiplier;
            currBeamSolar(SurfNum) = state.dataEnvrn->BeamSolarRad * SurfIncSolarMultiplier *
                                     TransTDD(state, PipeNum, ConInc, Dayltg::RadType::SolarBeam) / thisConstruct.TransDiff;

            state.dataSurface->SurfSkySolarInc(SurfNum) = state.dataEnvrn->DifSolarRad * SurfIncSolarMultiplier *
                                                          state.dataSolarShading->SurfAnisoSkyMult(SurfNum2) *
                                                          TransTDD(state, PipeNum, ConInc, Dayltg::RadType::SolarAniso) / thisConstruct.TransDiff;

            state.dataSurface->SurfGndSolarInc(SurfNum) = state.dataSurface->SurfGndSolarInc(SurfNum2) *
                                                          state.dataDaylightingDevicesData->TDDPipe(PipeNum).TransSolIso / thisConstruct.TransDiff;
            // Incident direct (unreflected) beam
            state.dataHeatBal->SurfQRadSWOutIncidentBeam(SurfNum) = currBeamSolar(SurfNum) *
                                                                    state.dataHeatBal->SurfSunlitFrac(state.dataGlobal->HourOfDay,
                                                                                                      state.dataGlobal->TimeStep,
                                                                                                      SurfNum2) *
                                                                    ConInc; // NOTE: sunlit and coninc array set to SurfNum2

            // Incident (unreflected) diffuse solar from sky -- TDD_Diffuser calculated differently
            state.dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(SurfNum) = state.dataSurface->SurfSkySolarInc(SurfNum);
            state.dataHeatBal->SurfQRadSWOutIncident(SurfNum) =
                (state.dataHeatBal->SurfQRadSWOutIncidentBeam(SurfNum) + state.dataHeatBal->SurfQRadSWOutIncidentSkyDiffuse(SurfNum) +
                 state.dataHeatBal->SurfQRadSWOutIncBmToDiffReflGnd(SurfNum) + state.dataHeatBal->SurfQRadSWOutIncSkyDiffReflGnd(SurfNum));
        }

        for (int ShelfNum = 1; ShelfNum <= (int)state.dataDaylightingDevicesData->Shelf.size(); ++ShelfNum) {
            int SurfNum = state.dataDaylightingDevicesData->Shelf(ShelfNum).Window;       // Daylighting shelf object number
            int OutShelfSurf = state.dataDaylightingDevicesData->Shelf(ShelfNum).OutSurf; // Outside daylighting shelf present if > 0
            state.dataHeatBal->SurfCosIncidenceAngle(SurfNum) =
                state.dataHeatBal->SurfCosIncAng(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum);
            Real64 SurfIncSolarMultiplier = state.dataSurface->Surface(SurfNum).IncSolMultiplier;
            currBeamSolar(SurfNum) = state.dataEnvrn->BeamSolarRad * SurfIncSolarMultiplier;
            // Shelf diffuse solar radiation
            Real64 ShelfSolarRad =
                (currBeamSolar(SurfNum) * state.dataHeatBal->SurfSunlitFrac(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, OutShelfSurf) *
                     state.dataHeatBal->SurfCosIncAng(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, OutShelfSurf) +
                 state.dataEnvrn->DifSolarRad * SurfIncSolarMultiplier * state.dataSolarShading->SurfAnisoSkyMult(OutShelfSurf)) *
                state.dataDaylightingDevicesData->Shelf(ShelfNum).OutReflectSol;

            GndReflSolarRad = state.dataEnvrn->GndSolarRad * SurfIncSolarMultiplier;
            if (state.dataSurface->Surface(SurfNum).UseSurfPropertyGndSurfRefl) {
                GndReflSolarRad = state.dataSurface->Surface(SurfNum).GndReflSolarRad;
            }
            // Add all reflected solar from the outside shelf to the ground solar
            // NOTE:  If the shelf blocks part of the view to the ground, the user must reduce the ground view factor!!
            state.dataSurface->SurfGndSolarInc(SurfNum) =
                GndReflSolarRad * Surface(SurfNum).ViewFactorGround + ShelfSolarRad * state.dataDaylightingDevicesData->Shelf(ShelfNum).ViewFactor;
        }

        // Calculate Exterior and Interior Absorbed Short Wave Radiation
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                auto &thisSpace = state.dataHeatBal->space(spaceNum);
                int const firstSurfOpaq = thisSpace.OpaqOrIntMassSurfaceFirst;
                int const lastSurfOpaq = thisSpace.OpaqOrIntMassSurfaceLast;
                for (int SurfNum = firstSurfOpaq; SurfNum <= lastSurfOpaq; ++SurfNum) {
                    int const ConstrNum = state.dataSurface->Surface(SurfNum).Construction;
                    Real64 SurfIncSolarMultiplier = state.dataSurface->Surface(SurfNum).IncSolMultiplier;
                    currBeamSolar(SurfNum) = state.dataEnvrn->BeamSolarRad * SurfIncSolarMultiplier;
                    if (Surface(SurfNum).ExtSolar) {
                        Real64 AbsExt =
                            state.dataHeatBalSurf->SurfAbsSolarExt(SurfNum); // Absorptivity of outer most layer (or movable insulation if present)
                        state.dataHeatBalSurf->SurfOpaqQRadSWOutAbs(SurfNum) =
                            state.dataSurface->SurfOpaqAO(SurfNum) * currBeamSolar(SurfNum) +
                            AbsExt * (state.dataSurface->SurfSkySolarInc(SurfNum) + state.dataSurface->SurfGndSolarInc(SurfNum));
                    }
                    if (ConstrNum > 0) {
                        int SurfSolIncPtr = SolarShading::SurfaceScheduledSolarInc(state, SurfNum, ConstrNum);
                        if (SurfSolIncPtr == 0) {
                            if (state.dataConstruction->Construct(ConstrNum).TransDiff <= 0.0) {    // Opaque surface
                                int ShelfNum = state.dataSurface->SurfDaylightingShelfInd(SurfNum); // Daylighting shelf object number
                                int InShelfSurf = 0;                                                // Inside daylighting shelf surface number
                                if (ShelfNum > 0) {
                                    InShelfSurf = state.dataDaylightingDevicesData->Shelf(ShelfNum).InSurf; // Inside daylighting shelf present if > 0
                                }
                                state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(SurfNum) +=
                                    state.dataSurface->SurfOpaqAI(SurfNum) * currBeamSolar(SurfNum);
                                if (InShelfSurf > 0) { // Inside daylighting shelf
                                    // Shelf surface area is divided by 2 because only one side sees beam (Area was multiplied by 2 during init)
                                    state.dataHeatBalSurf->SurfOpaqInsFaceBeamSolAbsorbed(SurfNum) =
                                        state.dataSurface->SurfOpaqAI(SurfNum) * currBeamSolar(SurfNum) * (0.5 * Surface(SurfNum).Area);
                                } else { // Regular surface
                                    state.dataHeatBalSurf->SurfOpaqInsFaceBeamSolAbsorbed(SurfNum) =
                                        state.dataSurface->SurfOpaqAI(SurfNum) * currBeamSolar(SurfNum) * Surface(SurfNum).Area;
                                }
                            }
                        } else {
                            state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(SurfNum) += state.dataSurface->SurfOpaqAI(SurfNum);
                        }
                    }
                }
                int const firstSurfWin = thisSpace.WindowSurfaceFirst;
                int const lastSurfWin = thisSpace.WindowSurfaceLast;
                for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {
                    auto &surf = state.dataSurface->Surface(SurfNum);
                    auto const &surfWin = state.dataSurface->SurfaceWindow(SurfNum);
                    if (surf.ExtSolar || surf.OriginalClass == DataSurfaces::SurfaceClass::TDD_Diffuser) {
                        // Exclude special shading surfaces which required SurfOpaqQRadSWOut calculations above
                        int const ConstrNum = state.dataSurface->SurfActiveConstruction(SurfNum);
                        auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);
                        Real64 CosInc = state.dataHeatBal->SurfCosIncidenceAngle(SurfNum); // Cosine of incidence angle of beam solar on glass
                        Real64 BeamSolar = currBeamSolar(SurfNum);                         // Local variable for BeamSolarRad
                        Real64 SkySolarInc = state.dataSurface->SurfSkySolarInc(SurfNum);  // Sky diffuse solar incident on a surface
                        Real64 GndSolarInc = state.dataSurface->SurfGndSolarInc(SurfNum);  // Ground diffuse solar incident on a surface

                        DataSurfaces::WinShadingType ShadeFlag = state.dataSurface->SurfWinShadingFlag(SurfNum);

                        if (state.dataSurface->SurfWinWindowModelType(SurfNum) == DataSurfaces::WindowModel::Detailed &&
                            !state.dataWindowManager->inExtWindowModel->isExternalLibraryModel()) {
                            int TotGlassLay = thisConstruct.TotGlassLayers; // Number of glass layers
                            for (int Lay = 1; Lay <= TotGlassLay; ++Lay) {
                                state.dataHeatBalSurfMgr->AbsDiffWin(Lay) = thisConstruct.AbsDiff(Lay);
                            }

                            if (IS_SHADED(ShadeFlag)) { // Shaded window

                                int const ConstrNumSh = state.dataSurface->SurfWinActiveShadedConstruction(SurfNum); // Shaded window construction
                                auto const &constructionSh = state.dataConstruction->Construct(ConstrNumSh);

                                if (ANY_SHADE_SCREEN(ShadeFlag)) { // Shade/screen on
                                    for (int Lay = 1; Lay <= TotGlassLay; ++Lay) {
                                        state.dataHeatBalSurfMgr->AbsDiffWin(Lay) = constructionSh.AbsDiff(Lay);
                                    }
                                    state.dataSurface->SurfWinExtDiffAbsByShade(SurfNum) = constructionSh.AbsDiffShade * (SkySolarInc + GndSolarInc);

                                } else if (ANY_BLIND(ShadeFlag)) { // Blind on
                                    auto &surfShade = state.dataSurface->surfShades(SurfNum);
                                    int slatIdxLo = surfShade.blind.slatAngIdxLo;
                                    int slatIdxHi = surfShade.blind.slatAngIdxHi;
                                    Real64 interpFac = surfShade.blind.slatAngInterpFac;
                                    Real64 AbsDiffBlind;

                                    // For constructions, have to do interpolation whether we have movable slats or not
                                    for (int Lay = 1; Lay <= TotGlassLay; ++Lay) {
                                        auto const &dfAbsSlatLo = constructionSh.layerSlatBlindDfAbs(Lay)[slatIdxLo];
                                        auto const &dfAbsSlatHi = constructionSh.layerSlatBlindDfAbs(Lay)[slatIdxHi];

                                        AbsDiffWin(Lay) = Interp(dfAbsSlatLo.Sol.Ft.Df.Abs, dfAbsSlatHi.Sol.Ft.Df.Abs, interpFac);
                                        AbsDiffWinGnd(Lay) = Interp(dfAbsSlatLo.Sol.Ft.Df.AbsGnd, dfAbsSlatHi.Sol.Ft.Df.AbsGnd, interpFac);
                                        AbsDiffWinSky(Lay) = Interp(dfAbsSlatLo.Sol.Ft.Df.AbsSky, dfAbsSlatHi.Sol.Ft.Df.AbsSky, interpFac);
                                    }

                                    auto const &tarSlatLo = constructionSh.blindTARs[slatIdxLo];
                                    auto const &tarSlatHi = constructionSh.blindTARs[slatIdxHi];
                                    AbsDiffBlind = Interp(tarSlatLo.Sol.Ft.Df.Abs, tarSlatHi.Sol.Ft.Df.Abs, interpFac);

                                    state.dataSurface->SurfWinExtDiffAbsByShade(SurfNum) = AbsDiffBlind * (SkySolarInc + GndSolarInc);

                                    auto const *matBlind = dynamic_cast<Material::MaterialBlind const *>(s_mat->materials(surfShade.blind.matNum));
                                    if (matBlind->SlatOrientation == DataWindowEquivalentLayer::Orientation::Horizontal) {
                                        Real64 ACosTlt = std::abs(Surface(SurfNum).CosTilt);

                                        // Need to do these interpolations unless we want to cache this in surfShade.blind.
                                        Real64 AbsDiffBlindGnd = Interp(tarSlatLo.Sol.Ft.Df.AbsGnd, tarSlatHi.Sol.Ft.Df.AbsGnd, interpFac);
                                        Real64 AbsDiffBlindSky = Interp(tarSlatLo.Sol.Ft.Df.AbsSky, tarSlatHi.Sol.Ft.Df.AbsSky, interpFac);

                                        state.dataSurface->SurfWinExtDiffAbsByShade(SurfNum) =
                                            SkySolarInc * (0.5 * ACosTlt * AbsDiffBlindGnd + (1.0 - 0.5 * ACosTlt) * AbsDiffBlindSky) +
                                            GndSolarInc * ((1.0 - 0.5 * ACosTlt) * AbsDiffBlindGnd + 0.5 * ACosTlt * AbsDiffBlindSky);
                                    }
                                }

                                // Correct for shadowing of divider onto interior shading device (note that dividers are
                                // not allowed in windows with between-glass shade/blind)

                                if (ANY_INTERIOR_SHADE_BLIND(ShadeFlag) && state.dataSurface->SurfWinDividerArea(SurfNum) > 0.0) {
                                    state.dataSurface->SurfWinExtDiffAbsByShade(SurfNum) *= surfWin.glazedFrac;
                                }

                                if (ShadeFlag == DataSurfaces::WinShadingType::SwitchableGlazing) {        // Switchable glazing
                                    Real64 SwitchFac = state.dataSurface->SurfWinSwitchingFactor(SurfNum); // Switching factor for switchable glazing
                                    for (int Lay = 1; Lay <= TotGlassLay; ++Lay) {
                                        state.dataHeatBalSurfMgr->AbsDiffWin(Lay) =
                                            Window::InterpSw(SwitchFac, state.dataHeatBalSurfMgr->AbsDiffWin(Lay), constructionSh.AbsDiff(Lay));
                                    }
                                }

                            } // End of check if window has shading device on

                            state.dataHeatBal->SurfWinQRadSWwinAbsTot(SurfNum) = 0.0;
                            for (int Lay = 1; Lay <= TotGlassLay; ++Lay) {
                                state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, Lay) =
                                    state.dataHeatBalSurfMgr->AbsDiffWin(Lay) * (SkySolarInc + GndSolarInc) +
                                    state.dataSurface->SurfWinA(SurfNum, Lay) * BeamSolar;
                                // SurfWinA is from InteriorSolarDistribution
                                if (ANY_BLIND(ShadeFlag)) {
                                    int ConstrNumSh = Surface(SurfNum).activeShadedConstruction;
                                    auto const &constructionSh = state.dataConstruction->Construct(ConstrNumSh);
                                    auto const &surfShade = state.dataSurface->surfShades(SurfNum);
                                    auto const *matBlind = dynamic_cast<Material::MaterialBlind const *>(s_mat->materials(surfShade.blind.matNum));
                                    if (matBlind->SlatOrientation == DataWindowEquivalentLayer::Orientation::Horizontal) {

                                        Real64 ACosTlt = std::abs(Surface(SurfNum).CosTilt); // Absolute value of cosine of surface tilt angle

                                        int slatIdxLo = surfShade.blind.slatAngIdxLo;
                                        int slatIdxHi = surfShade.blind.slatAngIdxLo;
                                        Real64 interpFac = surfShade.blind.slatAngInterpFac;
                                        auto const &dfAbsSlatLo = constructionSh.layerSlatBlindDfAbs(Lay)[slatIdxLo];
                                        auto const &dfAbsSlatHi = constructionSh.layerSlatBlindDfAbs(Lay)[slatIdxHi];

                                        Real64 AbsDiffGlassLayGnd = Interp(dfAbsSlatLo.Sol.Ft.Df.AbsGnd, dfAbsSlatHi.Sol.Ft.Df.AbsGnd, interpFac);
                                        Real64 AbsDiffGlassLaySky = Interp(dfAbsSlatLo.Sol.Ft.Df.AbsSky, dfAbsSlatHi.Sol.Ft.Df.AbsSky, interpFac);

                                        state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, Lay) =
                                            SkySolarInc * (0.5 * ACosTlt * AbsDiffGlassLayGnd + (1.0 - 0.5 * ACosTlt) * AbsDiffGlassLaySky) +
                                            GndSolarInc * ((1.0 - 0.5 * ACosTlt) * AbsDiffGlassLayGnd + 0.5 * ACosTlt * AbsDiffGlassLaySky) +
                                            state.dataSurface->SurfWinA(SurfNum, Lay) * BeamSolar;
                                    }
                                }
                                // Total solar absorbed in solid layer (W), for reporting
                                state.dataHeatBal->SurfWinQRadSWwinAbsLayer(SurfNum, Lay) =
                                    state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, Lay) * Surface(SurfNum).Area;

                                // Total solar absorbed in all glass layers (W), for reporting
                                state.dataHeatBal->SurfWinQRadSWwinAbsTot(SurfNum) += state.dataHeatBal->SurfWinQRadSWwinAbsLayer(SurfNum, Lay);
                            }
                            state.dataHeatBal->SurfWinQRadSWwinAbsTotEnergy(SurfNum) =
                                state.dataHeatBal->SurfWinQRadSWwinAbsTot(SurfNum) * state.dataGlobal->TimeStepZoneSec;
                            // Need to do it this way for now because of scheduled surface gains. They do work only with
                            // BSDF windows and overwriting absorbtances will work only for ordinary windows
                            // } else if ( SurfaceWindow( SurfNum ).WindowModelType != WindowModel:: BSDF &&
                            //   SurfaceWindow( SurfNum ).WindowModelType != WindowModel:: EQL &&
                            //   inExtWindowModel->isExternalLibraryModel() ) {
                            //   TotSolidLay = Construct( ConstrNum ).TotSolidLayers;
                            //   for ( Lay = 1; Lay <= TotSolidLay; ++Lay ) {
                            //     SurfWinQRadSWwinAbs( Lay, SurfNum ) = SurfWinA( Lay, SurfNum ) *
                            //       ( SurfQRadSWOutIncident( SurfNum ) + QS( Surface( SurfNum ).Zone ) );
                            //   }
                        } else if (state.dataSurface->SurfWinWindowModelType(SurfNum) == DataSurfaces::WindowModel::BSDF) {
                            int TotSolidLay = state.dataConstruction->Construct(ConstrNum).TotSolidLayers;
                            // Number of solid layers in fenestration system (glass + shading)
                            int CurrentState = state.dataSurface->SurfaceWindow(SurfNum).ComplexFen.CurrentState;
                            // Current state for Complex Fenestration
                            // Examine for schedule surface gain
                            Real64 SurfSolAbs = SolarShading::WindowScheduledSolarAbs(
                                state,
                                SurfNum,
                                ConstrNum); // Pointer to scheduled surface gains object for fenestration systems

                            for (int Lay = 1; Lay <= TotSolidLay; ++Lay) {
                                if (SurfSolAbs != 0) {
                                    state.dataSurface->SurfWinA(SurfNum, Lay) =
                                        state.dataSurface->FenLayAbsSSG(SurfSolAbs).scheds(Lay)->getCurrentVal();
                                    // ABWin(Lay) = SurfWinA(SurfNum,Lay)
                                    state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, Lay) = state.dataSurface->SurfWinA(SurfNum, Lay);
                                } else {
                                    // Several notes about this equation.  First part is accounting for diffuse solar radiation for the ground
                                    // and from the sky.  Second item (SurfWinA(SurfNum,Lay) * BeamSolar) is accounting for absorbed solar
                                    // radiation originating from beam on exterior side.  Third item (SurfWinACFOverlap(SurfNum,Lay)) is
                                    // accounting for absorptances from beam hitting back of the window which passes through rest of exterior
                                    // windows
                                    state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, Lay) =
                                        state.dataSurface->SurfaceWindow(SurfNum).ComplexFen.State(CurrentState).WinSkyFtAbs(Lay) * SkySolarInc +
                                        state.dataSurface->SurfaceWindow(SurfNum).ComplexFen.State(CurrentState).WinSkyGndAbs(Lay) * GndSolarInc +
                                        state.dataSurface->SurfWinA(SurfNum, Lay) * BeamSolar +
                                        state.dataSurface->SurfWinACFOverlap(SurfNum, Lay) * BeamSolar;
                                }
                                // Total solar absorbed in solid layer (W), for reporting
                                state.dataHeatBal->SurfWinQRadSWwinAbsLayer(SurfNum, Lay) =
                                    state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, Lay) * Surface(SurfNum).Area;

                                // Total solar absorbed in all glass layers (W), for reporting
                                state.dataHeatBal->SurfWinQRadSWwinAbsTot(SurfNum) += state.dataHeatBal->SurfWinQRadSWwinAbsLayer(SurfNum, Lay);
                            }
                            state.dataHeatBal->SurfWinQRadSWwinAbsTotEnergy(SurfNum) =
                                state.dataHeatBal->SurfWinQRadSWwinAbsTot(SurfNum) * state.dataGlobal->TimeStepZoneSec;

                        } else if (state.dataSurface->SurfWinWindowModelType(SurfNum) == DataSurfaces::WindowModel::EQL) {
                            state.dataHeatBal->SurfWinQRadSWwinAbsTot(SurfNum) = 0.0;
                            // EQLNum = Construct(Surface(SurfNum)%Construction)%EQLConsPtr
                            int TotSolidLay =
                                state.dataWindowEquivLayer->CFS(state.dataConstruction->Construct(Surface(SurfNum).Construction).EQLConsPtr).NL;
                            for (int Lay = 1; Lay <= TotSolidLay; ++Lay) {
                                // Absorbed window components include:
                                // (1) beam solar radiation absorbed by all layers in the fenestration
                                // (2) sky and ground reflected diffuse solar radiation absorbed by all layers
                                // (3) diffuse short wave incident on the inside face of the fenestration.  The short wave internal sources
                                //     include light, ...
                                state.dataHeatBalSurfMgr->AbsDiffWin(Lay) = state.dataConstruction->Construct(ConstrNum).AbsDiffFrontEQL(Lay);
                                state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, Lay) =
                                    state.dataSurface->SurfWinA(SurfNum, Lay) * BeamSolar +
                                    state.dataHeatBalSurfMgr->AbsDiffWin(Lay) * (SkySolarInc + GndSolarInc);

                                // Total solar absorbed in solid layer (W), for reporting
                                state.dataHeatBal->SurfWinQRadSWwinAbsLayer(SurfNum, Lay) =
                                    state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, Lay) * Surface(SurfNum).Area;

                                // Total solar absorbed in all glass layers (W), for reporting
                                state.dataHeatBal->SurfWinQRadSWwinAbsTot(SurfNum) += state.dataHeatBal->SurfWinQRadSWwinAbsLayer(SurfNum, Lay);
                            }

                            state.dataHeatBal->SurfWinQRadSWwinAbsTotEnergy(SurfNum) =
                                state.dataHeatBal->SurfWinQRadSWwinAbsTot(SurfNum) * state.dataGlobal->TimeStepZoneSec;
                        } else if (state.dataWindowManager->inExtWindowModel->isExternalLibraryModel()) {
                            int SurfNum2 = SurfNum;
                            if (state.dataSurface->Surface(SurfNum).OriginalClass == DataSurfaces::SurfaceClass::TDD_Diffuser) {
                                SurfNum2 = state.dataDaylightingDevicesData->TDDPipe(state.dataSurface->SurfWinTDDPipeNum(SurfNum)).Dome;
                            }

                            std::pair<Real64, Real64> incomingAngle =
                                Window::getSunWCEAngles(state, SurfNum2, SingleLayerOptics::BSDFDirection::Incoming);
                            Real64 Theta = incomingAngle.first;
                            Real64 Phi = incomingAngle.second;

                            std::shared_ptr<MultiLayerOptics::CMultiLayerScattered> aLayer =
                                Window::CWindowConstructionsSimplified::instance(state).getEquivalentLayer(
                                    state, FenestrationCommon::WavelengthRange::Solar, ConstrNum);

                            size_t totLayers = aLayer->getNumOfLayers();
                            for (size_t Lay = 1; Lay <= totLayers; ++Lay) {
                                Real64 AbWinDiff = aLayer->getAbsorptanceLayer(
                                    Lay, FenestrationCommon::Side::Front, FenestrationCommon::ScatteringSimple::Diffuse, Theta, Phi);

                                state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, Lay) =
                                    AbWinDiff * (SkySolarInc + GndSolarInc) + state.dataSurface->SurfWinA(SurfNum, Lay) * BeamSolar;

                                // Total solar absorbed in solid layer (W), for reporting
                                state.dataHeatBal->SurfWinQRadSWwinAbsLayer(SurfNum, Lay) =
                                    state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, Lay) * Surface(SurfNum).Area;

                                // Total solar absorbed in all glass layers (W), for reporting
                                state.dataHeatBal->SurfWinQRadSWwinAbsTot(SurfNum) += state.dataHeatBal->SurfWinQRadSWwinAbsLayer(SurfNum, Lay);
                            }
                        }

                        // Solar absorbed by window frame and dividers
                        int FrDivNum = Surface(SurfNum).FrameDivider; // Frame/divider number
                        if (FrDivNum > 0) {
                            Real64 FrArea = state.dataSurface->SurfWinFrameArea(SurfNum);                    // Frame, divider area (m2)
                            Real64 FrProjOut = state.dataSurface->FrameDivider(FrDivNum).FrameProjectionOut; // Frame, divider outside projection (m)
                            Real64 FrProjIn = state.dataSurface->FrameDivider(FrDivNum).FrameProjectionIn;
                            Real64 DivArea = state.dataSurface->SurfWinDividerArea(SurfNum);
                            Real64 DivWidth = state.dataSurface->FrameDivider(FrDivNum).DividerWidth;
                            Real64 DivProjOut = state.dataSurface->FrameDivider(FrDivNum).DividerProjectionOut;
                            Real64 DivProjIn = state.dataSurface->FrameDivider(FrDivNum).DividerProjectionIn;
                            Real64 CosIncAngHorProj = 0.0;  // Cosine of incidence angle of sun on horizontal faces of a frame or divider projection
                            Real64 CosIncAngVertProj = 0.0; // Cosine of incidence angle of sun on vertical faces of a frame or divider projection
                            Real64 FracSunLit = 0.0;        // Fraction of window sunlit this time step
                            Real64 BeamFaceInc;             // Beam solar incident window plane this time step (W/m2)
                            Real64 DifSolarFaceInc;         // Diffuse solar incident on window plane this time step (W/m2)
                            Real64 SurfIncSolarMultiplier = state.dataSurface->Surface(SurfNum).IncSolMultiplier;
                            Real64 currBeamSolarRad = state.dataEnvrn->BeamSolarRad * SurfIncSolarMultiplier;
                            if (FrArea > 0.0 || DivArea > 0.0) {
                                FracSunLit = state.dataHeatBal->SurfSunlitFrac(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum);
                                BeamFaceInc = currBeamSolarRad *
                                              state.dataHeatBal->SurfSunlitFrac(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, SurfNum) *
                                              CosInc;
                                DifSolarFaceInc = SkySolarInc + GndSolarInc;
                            }
                            if (FracSunLit > 0.0) {
                                if ((FrArea > 0.0 && (FrProjOut > 0.0 || FrProjIn > 0.0)) ||
                                    (DivArea > 0.0 && (DivProjOut > 0.0 || DivProjIn > 0.0))) {
                                    // Dot products used to calculate beam solar incident on faces of
                                    // frame and divider perpendicular to the glass surface.
                                    // Note that SOLCOS is the current timestep's solar direction cosines.
                                    //                  PhiWin = ASIN(WALCOS(3,SurfNum))
                                    Real64 PhiWin =
                                        std::asin(Surface(SurfNum).OutNormVec(3)); // Altitude and azimuth angle of outward window normal (radians)
                                    Real64 ThWin = std::atan2(Surface(SurfNum).OutNormVec(2), Surface(SurfNum).OutNormVec(1));
                                    Real64 PhiSun = std::asin(state.dataEnvrn->SOLCOS(3)); // Altitude and azimuth angle of sun (radians)
                                    Real64 ThSun = std::atan2(state.dataEnvrn->SOLCOS(2), state.dataEnvrn->SOLCOS(1));
                                    Real64 const cos_PhiWin(std::cos(PhiWin));
                                    Real64 const cos_PhiSun(std::cos(PhiSun));
                                    CosIncAngHorProj =
                                        std::abs(std::sin(PhiWin) * cos_PhiSun * std::cos(ThWin - ThSun) - cos_PhiWin * std::sin(PhiSun));
                                    CosIncAngVertProj = std::abs(cos_PhiWin * cos_PhiSun * std::sin(ThWin - ThSun));
                                }
                            }
                            // Frame solar
                            // (A window shade or blind, if present, is assumed to not shade the frame, so no special
                            // treatment of frame solar needed if window has an exterior shade or blind.)
                            if (FrArea > 0.0) {
                                Real64 FrIncSolarOut = BeamFaceInc; // Total solar incident on outside of frame including solar
                                Real64 FrIncSolarIn = 0.0; // Total solar incident on inside of frame including solar on frame projection (W/m2)
                                Real64 TransDiffGl = 0.0;  // Diffuse solar transmittance
                                if (FrProjOut > 0.0 || FrProjIn > 0.0) {
                                    Real64 BeamFrHorFaceInc =
                                        currBeamSolarRad * CosIncAngHorProj *
                                        (Surface(SurfNum).Width - state.dataSurface->FrameDivider(FrDivNum).VertDividers * DivWidth) * FracSunLit /
                                        FrArea;
                                    Real64 BeamFrVertFaceInc =
                                        currBeamSolarRad * CosIncAngVertProj *
                                        (Surface(SurfNum).Height - state.dataSurface->FrameDivider(FrDivNum).HorDividers * DivWidth) * FracSunLit /
                                        FrArea;
                                    // Beam solar on outside of frame
                                    FrIncSolarOut += (BeamFrHorFaceInc + BeamFrVertFaceInc) * FrProjOut;
                                    if (FrProjIn > 0.0) {
                                        Real64 TransGl = Window::POLYF(CosInc, thisConstruct.TransSolBeamCoef);
                                        TransDiffGl = thisConstruct.TransDiff;
                                        if (ShadeFlag == DataSurfaces::WinShadingType::SwitchableGlazing) { // Switchable glazing
                                            Real64 SwitchFac = state.dataSurface->SurfWinSwitchingFactor(SurfNum);
                                            int ConstrNumSh = Surface(SurfNum).activeShadedConstruction;
                                            auto const &constructionSh = state.dataConstruction->Construct(ConstrNumSh);
                                            Real64 TransGlSh = Window::POLYF(CosInc, constructionSh.TransSolBeamCoef);
                                            TransGl = Window::InterpSw(SwitchFac, TransGl, TransGlSh);
                                            Real64 TransDiffGlSh = constructionSh.TransDiff;
                                            TransDiffGl = Window::InterpSw(SwitchFac, TransDiffGl, TransDiffGlSh);
                                        }
                                        // Beam solar on inside of frame
                                        FrIncSolarIn = (BeamFrHorFaceInc + BeamFrVertFaceInc) * FrProjIn * TransGl;
                                    }
                                }
                                // Beam plus diffuse solar on outside of frame
                                FrIncSolarOut += DifSolarFaceInc * (1.0 + 0.5 * state.dataSurface->SurfWinProjCorrFrOut(SurfNum));
                                state.dataSurface->SurfWinFrameQRadOutAbs(SurfNum) =
                                    FrIncSolarOut * state.dataSurface->SurfWinFrameSolAbsorp(SurfNum);
                                // Add diffuse from beam reflected from window outside reveal surfaces
                                state.dataSurface->SurfWinFrameQRadOutAbs(SurfNum) += currBeamSolarRad *
                                                                                      state.dataSurface->SurfWinOutsRevealDiffOntoFrame(SurfNum) *
                                                                                      state.dataSurface->SurfWinFrameSolAbsorp(SurfNum);

                                // Beam plus diffuse solar on inside of frame
                                FrIncSolarIn += DifSolarFaceInc * TransDiffGl * 0.5 * state.dataSurface->SurfWinProjCorrFrIn(SurfNum);
                                state.dataSurface->SurfWinFrameQRadInAbs(SurfNum) = FrIncSolarIn * state.dataSurface->SurfWinFrameSolAbsorp(SurfNum);
                                // Add diffuse from beam reflected from window inside reveal surfaces
                                state.dataSurface->SurfWinFrameQRadInAbs(SurfNum) += currBeamSolarRad *
                                                                                     state.dataSurface->SurfWinInsRevealDiffOntoFrame(SurfNum) *
                                                                                     state.dataSurface->SurfWinFrameSolAbsorp(SurfNum);
                            }

                            // Divider solar
                            // (An exterior shade or blind, when in place, is assumed to completely cover the divider.
                            // Dividers are not allowed on windows with between-glass shade/blind so DivProjOut and
                            // DivProjIn will be zero in this case.)
                            if (DivArea > 0.0) {                                                         // Solar absorbed by window divider
                                Real64 DividerAbs = state.dataSurface->SurfWinDividerSolAbsorp(SurfNum); // Window divider solar absorptance
                                if (state.dataSurface->SurfWinDividerType(SurfNum) == DataSurfaces::FrameDividerType::Suspended) {
                                    // Suspended (between-glass) divider; account for effect glass on outside of divider
                                    // (note that outside and inside projection for this type of divider are both zero)
                                    int MatNumGl = thisConstruct.LayerPoint(1); // Outer glass layer material number
                                    auto const *thisMaterial = dynamic_cast<Material::MaterialFen *>(s_mat->materials(MatNumGl));
                                    assert(thisMaterial != nullptr);
                                    Real64 TransGl = thisMaterial->Trans; // Outer glass layer material number, switched construction
                                    Real64 ReflGl = thisMaterial->ReflectSolBeamFront;
                                    Real64 AbsGl = 1.0 - TransGl - ReflGl;
                                    Real64 SwitchFac = state.dataSurface->SurfWinSwitchingFactor(SurfNum);
                                    int ConstrNumSh = Surface(SurfNum).activeShadedConstruction;
                                    if (ShadeFlag == DataSurfaces::WinShadingType::SwitchableGlazing) { // Switchable glazing
                                        auto const &constructionSh = state.dataConstruction->Construct(ConstrNumSh);
                                        Real64 MatNumGlSh = constructionSh.LayerPoint(1);
                                        auto const *thisMaterialSh = dynamic_cast<Material::MaterialGlass *>(s_mat->materials(MatNumGlSh));
                                        assert(thisMaterialSh != nullptr);
                                        Real64 TransGlSh = thisMaterialSh->Trans;
                                        Real64 ReflGlSh = thisMaterialSh->ReflectSolBeamFront;
                                        Real64 AbsGlSh = 1.0 - TransGlSh - ReflGlSh;
                                        TransGl = Window::InterpSw(SwitchFac, TransGl, TransGlSh);
                                        ReflGl = Window::InterpSw(SwitchFac, ReflGl, ReflGlSh);
                                        AbsGl = Window::InterpSw(SwitchFac, AbsGl, AbsGlSh);
                                    }
                                    Real64 DividerRefl = 1.0 - DividerAbs; // Window divider solar reflectance
                                    DividerAbs = AbsGl + TransGl * (DividerAbs + DividerRefl * AbsGl) / (1.0 - DividerRefl * ReflGl);
                                }

                                Real64 BeamDivHorFaceInc = 0.0;  // Beam solar on divider's horizontal outside projection faces (W/m2)
                                Real64 BeamDivVertFaceInc = 0.0; // Beam solar on divider's vertical outside projection faces (W/m2)
                                // Beam incident on horizontal and vertical projection faces of divider if no exterior shading
                                if (DivProjOut > 0.0 && !DataSurfaces::ANY_EXTERIOR_SHADE_BLIND_SCREEN(ShadeFlag)) {
                                    BeamDivHorFaceInc = currBeamSolarRad * CosIncAngHorProj * state.dataSurface->FrameDivider(FrDivNum).HorDividers *
                                                        DivProjOut *
                                                        (Surface(SurfNum).Width - state.dataSurface->FrameDivider(FrDivNum).VertDividers * DivWidth) *
                                                        FracSunLit / DivArea;
                                    BeamDivVertFaceInc =
                                        currBeamSolarRad * CosIncAngVertProj * state.dataSurface->FrameDivider(FrDivNum).VertDividers * DivProjOut *
                                        (Surface(SurfNum).Height - state.dataSurface->FrameDivider(FrDivNum).HorDividers * DivWidth) * FracSunLit /
                                        DivArea;
                                }
                                Real64 DivIncSolarOutBm =
                                    0.0; // Diffuse solar incident on outside of divider including beam on divider projection (W/m2)
                                Real64 DivIncSolarOutDif =
                                    0.0; // Diffuse solar incident on outside of divider including diffuse on divider projection (W/m2)
                                Real64 DivIncSolarInBm =
                                    0.0; // Diffuse solar incident on inside of divider including beam on divider projection (W/m2)
                                Real64 DivIncSolarInDif =
                                    0.0; // Diffuse solar incident on inside of divider including diffuse on divider projection (W/m2)
                                if (!ANY_EXTERIOR_SHADE_BLIND_SCREEN(ShadeFlag) &&
                                    !ANY_BETWEENGLASS_SHADE_BLIND(ShadeFlag)) { // No exterior or between-glass shading
                                    DivIncSolarOutBm = BeamFaceInc + BeamDivHorFaceInc + BeamDivVertFaceInc;
                                    DivIncSolarOutDif = DifSolarFaceInc * (1.0 + state.dataSurface->SurfWinProjCorrDivOut(SurfNum));
                                    if (DivProjIn > 0.0) {
                                        Real64 TransGl = Window::POLYF(CosInc, thisConstruct.TransSolBeamCoef);
                                        Real64 TransDiffGl = thisConstruct.TransDiff;                       // Diffuse solar transmittance
                                        if (ShadeFlag == DataSurfaces::WinShadingType::SwitchableGlazing) { // Switchable glazing
                                            Real64 SwitchFac = state.dataSurface->SurfWinSwitchingFactor(SurfNum);
                                            int ConstrNumSh = Surface(SurfNum).activeShadedConstruction;
                                            auto const &constructionSh = state.dataConstruction->Construct(ConstrNumSh);

                                            Real64 TransGlSh = Window::POLYF(CosInc, constructionSh.TransSolBeamCoef);
                                            // Outer glass solar trans, refl, absorptance if switched
                                            TransGl = Window::InterpSw(SwitchFac, TransGl, TransGlSh);
                                            Real64 TransDiffGlSh = constructionSh.TransDiff;
                                            // Diffuse solar transmittance, switched construction
                                            TransDiffGl = Window::InterpSw(SwitchFac, TransDiffGl, TransDiffGlSh);
                                        }
                                        // Beam plus diffuse solar on inside of divider
                                        // BeamDivHorFaceIncIn - Beam solar on divider's horizontal inside projection faces (W/m2)
                                        // BeamDivVertFaceIncIn - Beam solar on divider's vertical inside projection faces (W/m2)
                                        Real64 BeamDivHorFaceIncIn =
                                            currBeamSolarRad * CosIncAngHorProj * state.dataSurface->FrameDivider(FrDivNum).HorDividers * DivProjIn *
                                            (Surface(SurfNum).Width - state.dataSurface->FrameDivider(FrDivNum).VertDividers * DivWidth) *
                                            FracSunLit / DivArea;
                                        Real64 BeamDivVertFaceIncIn =
                                            currBeamSolarRad * CosIncAngVertProj * state.dataSurface->FrameDivider(FrDivNum).VertDividers *
                                            DivProjIn * (Surface(SurfNum).Height - state.dataSurface->FrameDivider(FrDivNum).HorDividers * DivWidth) *
                                            FracSunLit / DivArea;
                                        DivIncSolarInBm = TransGl * (BeamDivHorFaceIncIn + BeamDivVertFaceIncIn);
                                        DivIncSolarInDif = TransDiffGl * DifSolarFaceInc * state.dataSurface->SurfWinProjCorrDivIn(SurfNum);
                                    }
                                } else { // Exterior shade, screen or blind present

                                    DivIncSolarOutBm = BeamFaceInc * (1.0 + state.dataSurface->SurfWinProjCorrDivOut(SurfNum));
                                    DivIncSolarOutDif = DifSolarFaceInc * (1.0 + state.dataSurface->SurfWinProjCorrDivOut(SurfNum));
                                    DivIncSolarInBm = BeamFaceInc * state.dataSurface->SurfWinProjCorrDivIn(SurfNum) * thisConstruct.TransDiff;
                                    DivIncSolarInDif = DifSolarFaceInc * state.dataSurface->SurfWinProjCorrDivIn(SurfNum) * thisConstruct.TransDiff;
                                }

                                if (!ANY_EXTERIOR_SHADE_BLIND_SCREEN(ShadeFlag) && !ANY_BETWEENGLASS_SHADE_BLIND(ShadeFlag)) {
                                    // No exterior or between-glass shade, screen or blind
                                    state.dataSurface->SurfWinDividerQRadOutAbs(SurfNum) = DividerAbs * (DivIncSolarOutBm + DivIncSolarOutDif);
                                    state.dataSurface->SurfWinDividerQRadInAbs(SurfNum) = DividerAbs * (DivIncSolarInBm + DivIncSolarInDif);
                                    // Exterior shade, screen or blind

                                } else if (ShadeFlag == DataSurfaces::WinShadingType::ExtBlind) { // Exterior blind
                                    auto const &surfShade = state.dataSurface->surfShades(SurfNum);

                                    int profIdxLo = surfShade.blind.profAngIdxLo;
                                    int profIdxHi = surfShade.blind.profAngIdxHi;
                                    Real64 profInterpFac = surfShade.blind.profAngInterpFac;

                                    auto const &btarLo = surfShade.blind.TAR.Sol.Ft.Bm[profIdxLo];
                                    auto const &btarHi = surfShade.blind.TAR.Sol.Ft.Bm[profIdxHi];

                                    Real64 FrontDiffTrans = surfShade.blind.TAR.Sol.Ft.Df.Tra;
                                    Real64 TBlBmDif = Interp(btarLo.DfTra, btarHi.DfTra, profInterpFac);

                                    // TBlBmBm - Blind beam-beam solar transmittance
                                    Real64 TBlBmBm = surfShade.blind.bmBmTrans;
                                    state.dataSurface->SurfWinDividerQRadOutAbs(SurfNum) =
                                        DividerAbs * (DivIncSolarOutBm * (TBlBmBm + TBlBmDif) + DivIncSolarOutDif * FrontDiffTrans);
                                    state.dataSurface->SurfWinDividerQRadInAbs(SurfNum) =
                                        DividerAbs * (DivIncSolarInBm * (TBlBmBm + TBlBmDif) + DivIncSolarInDif * FrontDiffTrans);

                                } else if (ShadeFlag == DataSurfaces::WinShadingType::ExtShade) { // Exterior shade
                                    int ConstrNumSh = Surface(SurfNum).activeShadedConstruction;
                                    auto const &constructionSh = state.dataConstruction->Construct(ConstrNumSh);
                                    auto const *matFen = dynamic_cast<Material::MaterialFen const *>(s_mat->materials(constructionSh.LayerPoint(1)));
                                    assert(matFen != nullptr);
                                    state.dataSurface->SurfWinDividerQRadOutAbs(SurfNum) =
                                        DividerAbs * matFen->Trans * (DivIncSolarOutBm + DivIncSolarOutDif);
                                    state.dataSurface->SurfWinDividerQRadInAbs(SurfNum) =
                                        DividerAbs * matFen->Trans * (DivIncSolarInBm + DivIncSolarInDif);

                                } else if (ShadeFlag == DataSurfaces::WinShadingType::ExtScreen) { // Exterior screen
                                    int screenNum = surfWin.screenNum;
                                    auto const *screen = dynamic_cast<Material::MaterialScreen const *>(s_mat->materials(screenNum));
                                    assert(screen != nullptr);

                                    auto &surf = state.dataSurface->Surface(SurfNum);
                                    Real64 phi, theta;
                                    Material::GetRelativePhiTheta(
                                        surf.Tilt * Constant::DegToRad, surf.Azimuth * Constant::DegToRad, state.dataEnvrn->SOLCOS, phi, theta);
#ifdef PRECALC_INTERP_SCREEN
                                    int ip1, ip2, it1, it2; // hi/lo phi and theta map indices
                                    BilinearInterpCoeffs coeffs;
                                    Material::GetPhiThetaIndices(phi, theta, screen->dPhi, screen->dTheta, ip1, ip2, it1, it2);
                                    GetBilinearInterpCoeffs(
                                        phi, theta, ip1 * screen->dPhi, ip2 * screen->dPhi, it1 * screen->dTheta, it2 * screen->dTheta, coeffs);
                                    auto const &b11 = screen->btars[ip1][it1];
                                    auto const &b12 = screen->btars[ip1][it2];
                                    auto const &b21 = screen->btars[ip2][it1];
                                    auto const &b22 = screen->btars[ip2][it2];
                                    Real64 BmDfTrans = BilinearInterp(b11.DfTrans, b12.DfTrans, b21.DfTrans, b22.DfTrans, coeffs);
                                    Real64 BmBmTrans = BilinearInterp(b11.BmTrans, b12.BmTrans, b21.BmTrans, b22.BmTrans, coeffs);

                                    state.dataSurface->SurfWinDividerQRadOutAbs(SurfNum) =
                                        DividerAbs * (BmBmTrans + BmDfTrans) * (DivIncSolarOutBm + DivIncSolarOutDif);
                                    state.dataSurface->SurfWinDividerQRadInAbs(SurfNum) =
                                        DividerAbs * (BmBmTrans + BmDfTrans) * (DivIncSolarInBm + DivIncSolarInDif);
#else  // !PRECALC_INTERP_SCREEN
                                    Material::ScreenBmTransAbsRef btar;

                                    Material::CalcScreenTransmittance(state, screen, phi, theta, btar);
                                    Real64 BmDfTrans = btar.DfTrans;
                                    Real64 BmBmTrans = btar.BmTrans;

                                    state.dataSurface->SurfWinDividerQRadOutAbs(SurfNum) =
                                        DividerAbs * (BmBmTrans + BmDfTrans) * (DivIncSolarOutBm + DivIncSolarOutDif);
                                    state.dataSurface->SurfWinDividerQRadInAbs(SurfNum) =
                                        DividerAbs * (BmBmTrans + BmDfTrans) * (DivIncSolarInBm + DivIncSolarInDif);
#endif // PRECALC_INTERP_SCREEN
                                }
                            }
                        }
                    } // Surface(SurfNum)%ExtSolar
                } // end of surface window loop
            } // end of space loop
        } // end of zone loop
        for (int PipeNum = 1; PipeNum <= (int)state.dataDaylightingDevicesData->TDDPipe.size(); ++PipeNum) {
            int const SurfNum = state.dataDaylightingDevicesData->TDDPipe(PipeNum).Dome; // TDD: DOME object number
            int const ConstrNum = Surface(SurfNum).Construction;
            auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);
            int const TotGlassLay = thisConstruct.TotGlassLayers; // Number of glass layers
            state.dataHeatBal->SurfWinQRadSWwinAbsTot(SurfNum) = 0.0;
            for (int Lay = 1; Lay <= TotGlassLay; ++Lay) {
                state.dataHeatBalSurfMgr->AbsDiffWin(Lay) = thisConstruct.AbsDiff(Lay);
                state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, Lay) =
                    state.dataHeatBalSurfMgr->AbsDiffWin(Lay) *
                        (state.dataSurface->SurfSkySolarInc(SurfNum) + state.dataSurface->SurfGndSolarInc(SurfNum)) +
                    state.dataSurface->SurfWinA(SurfNum, Lay) * currBeamSolar(SurfNum);
                state.dataHeatBal->SurfWinQRadSWwinAbsLayer(SurfNum, Lay) =
                    state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, Lay) * Surface(SurfNum).Area;
                state.dataHeatBal->SurfWinQRadSWwinAbsTot(SurfNum) += state.dataHeatBal->SurfWinQRadSWwinAbsLayer(SurfNum, Lay);
                state.dataHeatBal->SurfWinQRadSWwinAbsTotEnergy(SurfNum) =
                    state.dataHeatBal->SurfWinQRadSWwinAbsTot(SurfNum) * state.dataGlobal->TimeStepZoneSec;
            }
        }

        // Average absorbed solar for representative surfaces
        if (state.dataSurface->UseRepresentativeSurfaceCalculations) {
            for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
                for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                    auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                    int firstSurf = thisSpace.OpaqOrIntMassSurfaceFirst;
                    int lastSurf = thisSpace.OpaqOrIntMassSurfaceLast;
                    for (int surfNum = firstSurf; surfNum <= lastSurf; ++surfNum) {
                        auto &surface = state.dataSurface->Surface(surfNum);
                        if (surface.ConstituentSurfaceNums.size() > 1) {
                            Real64 QoutAtot = 0.0;
                            Real64 QinAtot = 0.0;
                            Real64 Atot = 0.0;
                            for (int constSurfNum : surface.ConstituentSurfaceNums) {
                                QoutAtot += state.dataHeatBalSurf->SurfOpaqQRadSWOutAbs(constSurfNum) * state.dataSurface->Surface(constSurfNum).Area;
                                QinAtot += state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(constSurfNum) * state.dataSurface->Surface(constSurfNum).Area;
                                Atot += state.dataSurface->Surface(constSurfNum).Area;
                            }

                            state.dataHeatBalSurf->SurfOpaqQRadSWOutAbs(surfNum) = QoutAtot / Atot;
                            state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(surfNum) = QinAtot / Atot;
                        }
                    }
                    firstSurf = thisSpace.WindowSurfaceFirst;
                    lastSurf = thisSpace.WindowSurfaceLast;
                    for (int surfNum = firstSurf; surfNum <= lastSurf; ++surfNum) {
                        auto const &surface = state.dataSurface->Surface(surfNum);
                        if (surface.ExtSolar && surface.ConstituentSurfaceNums.size() > 1) {
                            // Absorbed in glazing
                            int totalGlassLayers =
                                state.dataConstruction->Construct(state.dataSurface->SurfActiveConstruction(surfNum)).TotGlassLayers;
                            for (int layer = 1; layer <= totalGlassLayers; ++layer) {
                                Real64 QAtot = 0.0;
                                Real64 Atot = 0.0;
                                for (int constSurfNum : surface.ConstituentSurfaceNums) {
                                    QAtot +=
                                        state.dataHeatBal->SurfWinQRadSWwinAbs(constSurfNum, layer) * state.dataSurface->Surface(constSurfNum).Area;
                                    Atot += state.dataSurface->Surface(constSurfNum).Area;
                                }

                                state.dataHeatBal->SurfWinQRadSWwinAbs(surfNum, layer) = QAtot / Atot;
                            }

                            // Absorbed by frame and dividers
                            if (surface.FrameDivider > 0) {
                                if (state.dataSurface->SurfWinFrameArea(surfNum) > 0.0) {
                                    Real64 QoutAtot = 0.0;
                                    Real64 QinAtot = 0.0;
                                    Real64 Atot = 0.0;
                                    for (int constSurfNum : surface.ConstituentSurfaceNums) {
                                        QoutAtot += state.dataSurface->SurfWinFrameQRadOutAbs(constSurfNum) *
                                                    state.dataSurface->SurfWinFrameArea(constSurfNum);
                                        QinAtot += state.dataSurface->SurfWinFrameQRadInAbs(constSurfNum) *
                                                   state.dataSurface->SurfWinFrameArea(constSurfNum);
                                        Atot += state.dataSurface->SurfWinFrameArea(constSurfNum);
                                    }

                                    state.dataSurface->SurfWinFrameQRadOutAbs(surfNum) = QoutAtot / Atot;
                                    state.dataSurface->SurfWinFrameQRadInAbs(surfNum) = QinAtot / Atot;
                                }
                                if (state.dataSurface->SurfWinDividerArea(surfNum) > 0.0) {
                                    Real64 QoutAtot = 0.0;
                                    Real64 QinAtot = 0.0;
                                    Real64 Atot = 0.0;
                                    for (int constSurfNum : surface.ConstituentSurfaceNums) {
                                        QoutAtot += state.dataSurface->SurfWinDividerQRadOutAbs(constSurfNum) *
                                                    state.dataSurface->SurfWinDividerArea(constSurfNum);
                                        QinAtot += state.dataSurface->SurfWinDividerQRadInAbs(constSurfNum) *
                                                   state.dataSurface->SurfWinDividerArea(constSurfNum);
                                        Atot += state.dataSurface->SurfWinDividerArea(constSurfNum);
                                    }

                                    state.dataSurface->SurfWinDividerQRadOutAbs(surfNum) = QoutAtot / Atot;
                                    state.dataSurface->SurfWinDividerQRadInAbs(surfNum) = QinAtot / Atot;
                                }
                            }
                        }
                    }
                }
            }
        }
    } // End of sun-up check
}

void InitIntSolarDistribution(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Anonymous
    //       DATE WRITTEN   July 1977
    //       MODIFIED       Oct 1999 (FW) to handle movable shades
    //                      May 2000 (FW) to handle window frame and dividers
    //                      May 2001 (FW) to handle window blinds
    //                      Jan 2002 (FW) mods for between-glass shade/blind
    //                      May 2006 (RR) to handle exterior window screens
    //       RE-ENGINEERED  Mar98 (RKS)

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine initializes the arrays associated with solar heat
    // gains for both individual surfaces and for zones.

    // METHODOLOGY EMPLOYED:
    // If the sun is down, all of the pertinent arrays are zeroed.  If the
    // sun is up, various calculations are made.

    // REFERENCES:
    // (I)BLAST legacy routine QSUN

    using Dayltg::DistributeTDDAbsorbedSolar;
    using namespace DataWindowEquivalentLayer;

    auto &s_mat = state.dataMaterial;
    // COMPUTE TOTAL SHORT-WAVE RADIATION ORIGINATING IN ZONE.
    // Note: If sun is not up, QS is only internal gains
    for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclosureNum) {
        Real64 sumSpaceQLTSW = 0.0;
        for (int spaceNum : state.dataViewFactor->EnclSolInfo(enclosureNum).spaceNums) {
            sumSpaceQLTSW += state.dataHeatBal->spaceIntGain(spaceNum).QLTSW;
        }
        state.dataHeatBal->EnclSolQSWRad(enclosureNum) = state.dataHeatBal->EnclSolQD(enclosureNum) + sumSpaceQLTSW;
        state.dataHeatBal->EnclSolQSWRadLights(enclosureNum) = sumSpaceQLTSW;
    }

    if (state.dataHeatBalSurf->InterZoneWindow) { // DO INTERZONE DISTRIBUTION.

        for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclosureNum) {

            if (state.dataHeatBalSurf->EnclSolRecDifShortFromZ(enclosureNum)) {

                for (int OtherenclosureNum = 1; OtherenclosureNum <= state.dataViewFactor->NumOfSolarEnclosures; ++OtherenclosureNum) {

                    if ((OtherenclosureNum != enclosureNum) && (state.dataHeatBalSurf->EnclSolRecDifShortFromZ(OtherenclosureNum))) {
                        Real64 sumSpaceQLTSW = 0.0;
                        for (int spaceNum : state.dataViewFactor->EnclSolInfo(OtherenclosureNum).spaceNums) {
                            sumSpaceQLTSW += state.dataHeatBal->spaceIntGain(spaceNum).QLTSW;
                        }
                        state.dataHeatBal->EnclSolQSWRad(enclosureNum) +=
                            state.dataHeatBalSurf->ZoneFractDifShortZtoZ(enclosureNum, OtherenclosureNum) *
                            (state.dataHeatBal->EnclSolQD(OtherenclosureNum) + sumSpaceQLTSW);
                        state.dataHeatBal->EnclSolQSWRadLights(enclosureNum) +=
                            state.dataHeatBalSurf->ZoneFractDifShortZtoZ(enclosureNum, OtherenclosureNum) * sumSpaceQLTSW;
                        state.dataHeatBal->ZoneDifSolFrIntWinsRep(enclosureNum) +=
                            state.dataHeatBalSurf->ZoneFractDifShortZtoZ(enclosureNum, OtherenclosureNum) *
                            state.dataHeatBal->EnclSolQD(OtherenclosureNum);
                        state.dataHeatBal->ZoneDifSolFrIntWinsRepEnergy(enclosureNum) =
                            state.dataHeatBal->ZoneDifSolFrIntWinsRep(enclosureNum) * state.dataGlobal->TimeStepZoneSec;
                    }
                }
            }
        }
    }

    // Beam and diffuse solar on inside surfaces from interior windows (for reporting)

    if (state.dataEnvrn->SunIsUp) {
        for (int SurfNum : state.dataSurface->AllHTSurfaceList) {
            auto const &surface = state.dataSurface->Surface(SurfNum);
            //!!! Following may need to be removed or changed when shelves are considered in adjacent reflection calculations
            if (surface.Class == DataSurfaces::SurfaceClass::Shading) {
                continue;
            }
            int const enclosureNum = surface.SolarEnclIndex;
            state.dataHeatBal->SurfIntBmIncInsSurfIntensRep(SurfNum) =
                state.dataHeatBal->ZoneBmSolFrIntWinsRep(enclosureNum) / state.dataViewFactor->EnclSolInfo(enclosureNum).TotalSurfArea;
            state.dataHeatBal->SurfIntBmIncInsSurfAmountRep(SurfNum) =
                state.dataHeatBal->SurfIntBmIncInsSurfIntensRep(SurfNum) * (surface.Area + state.dataSurface->SurfWinDividerArea(SurfNum));
            state.dataHeatBal->SurfIntBmIncInsSurfAmountRepEnergy(SurfNum) =
                state.dataHeatBal->SurfIntBmIncInsSurfAmountRep(SurfNum) * state.dataGlobal->TimeStepZoneSec;
        }
    }

    // COMPUTE CONVECTIVE GAINS AND ZONE FLUX DENSITY.
    for (int enclosureNum = 1; enclosureNum <= state.dataViewFactor->NumOfSolarEnclosures; ++enclosureNum) {
        auto const &thisSolEnclosure = state.dataViewFactor->EnclSolInfo(enclosureNum);
        if (state.dataHeatBalSurf->InterZoneWindow) {
            state.dataHeatBal->EnclSolQSWRad(enclosureNum) *=
                state.dataHeatBalSurf->ZoneFractDifShortZtoZ(enclosureNum, enclosureNum) * thisSolEnclosure.solVMULT;
            // CR 8695, VMULT not based on visible
            state.dataHeatBal->EnclSolQSWRadLights(enclosureNum) *=
                state.dataHeatBalSurf->ZoneFractDifShortZtoZ(enclosureNum, enclosureNum) * thisSolEnclosure.solVMULT;
        } else {
            state.dataHeatBal->EnclSolQSWRad(enclosureNum) *= thisSolEnclosure.solVMULT;
            state.dataHeatBal->EnclSolQSWRadLights(enclosureNum) *= thisSolEnclosure.solVMULT;
        }
    }

    // COMPUTE RADIANT GAINS ON SURFACES
    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurfOpaque = thisSpace.OpaqOrIntMassSurfaceFirst;
            int const lastSurfOpaque = thisSpace.OpaqOrIntMassSurfaceLast;
            for (int SurfNum = firstSurfOpaque; SurfNum <= lastSurfOpaque; ++SurfNum) {
                auto const &surface = state.dataSurface->Surface(SurfNum);
                int const solEnclosureNum = surface.SolarEnclIndex;
                int const ConstrNum = surface.Construction;
                auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);

                Real64 AbsIntSurf = state.dataHeatBalSurf->SurfAbsSolarInt(SurfNum);
                // TODO - AbsIntSurfVis = InsideAbsorpSolar instead of InsideAbsorpVis?
                Real64 AbsIntSurfVis = thisConstruct.InsideAbsorpSolar; // Inside opaque surface visible absorptance to fix CR 8695 change to

                state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(SurfNum) += state.dataHeatBal->EnclSolQSWRad(solEnclosureNum) * AbsIntSurf;
                state.dataHeatBalSurf->SurfQdotRadLightsInPerArea(SurfNum) += state.dataHeatBal->EnclSolQSWRadLights(solEnclosureNum) * AbsIntSurfVis;

                // Calculate absorbed solar on outside if movable exterior insulation in place
                if (state.dataSurface->AnyMovableInsulation) {
                    auto &movInsul = state.dataSurface->extMovInsuls(SurfNum);
                    if (movInsul.present) {
                        Real64 AbsExt = state.dataHeatBalSurf->SurfAbsSolarExt(SurfNum);
                        auto const *thisMaterial = s_mat->materials(thisConstruct.LayerPoint(1));
                        state.dataHeatBalSurf->SurfQRadSWOutMvIns(SurfNum) =
                            state.dataHeatBalSurf->SurfOpaqQRadSWOutAbs(SurfNum) * AbsExt / thisMaterial->AbsorpSolar;
                        // For transparent insulation, allow some sunlight to get through the movable insulation.
                        // The equation below is derived by taking what is transmitted through the layer and applying
                        // the fraction that is absorbed plus the back reflected portion (first order reflection only)
                        // to the plane between the transparent insulation and the exterior surface face.
                        auto const *matMovInsul = s_mat->materials(movInsul.matNum);
                        auto const *matFenMovInsul = dynamic_cast<Material::MaterialFen const *>(matMovInsul);
                        Real64 transMovInsul = (matFenMovInsul != nullptr) ? matFenMovInsul->Trans : 0.0;

                        state.dataHeatBalSurf->SurfOpaqQRadSWOutAbs(SurfNum) =
                            transMovInsul * state.dataHeatBalSurf->SurfQRadSWOutMvIns(SurfNum) *
                            ((thisMaterial->AbsorpSolar / AbsExt) + (1 - thisMaterial->AbsorpSolar));
                    }
                }
                // RJH 08/30/07 - Add SurfWinInitialDifSolInAbs, SurfWinInitialDifSolwinAbs, and SurfWinInitialDifSolAbsByShade
                // calced in CalcWinTransDifSolInitialDistribution to SurfOpaqQRadSWInAbs, SurfWinQRadSWwinAbs, and SurfWinIntSWAbsByShade here
                state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(SurfNum) += state.dataHeatBalSurf->SurfOpaqInitialDifSolInAbs(SurfNum);
            } // end of opaque

            int const firstSurfWin = thisSpace.WindowSurfaceFirst;
            int const lastSurfWin = thisSpace.WindowSurfaceLast;
            for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) { // Window
                auto const &surface = state.dataSurface->Surface(SurfNum);
                auto const &surfWin = state.dataSurface->SurfaceWindow(SurfNum);
                int const radEnclosureNum = surface.RadEnclIndex;
                int const solEnclosureNum = surface.SolarEnclIndex;
                int const ConstrNum = state.dataSurface->SurfActiveConstruction(SurfNum);
                auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);

                if (state.dataSurface->SurfWinWindowModelType(SurfNum) != DataSurfaces::WindowModel::EQL) {
                    int const ConstrNumSh = state.dataSurface->SurfWinActiveShadedConstruction(SurfNum);

                    int TotGlassLayers = thisConstruct.TotGlassLayers;
                    DataSurfaces::WinShadingType ShadeFlag = state.dataSurface->SurfWinShadingFlag(SurfNum);

                    // These calculations are repeated from InitInternalHeatGains for the Zone Component Loads Report
                    Real64 pulseMultipler = 0.01; // use to create a pulse for the load component report computations, the W/sqft pulse for the zone
                    if (!state.dataGlobal->doLoadComponentPulseNow) {
                        state.dataHeatBal->SurfQdotRadIntGainsInPerArea(SurfNum) =
                            state.dataViewFactor->EnclRadInfo(radEnclosureNum).radQThermalRad *
                            state.dataViewFactor->EnclRadInfo(radEnclosureNum).radThermAbsMult * state.dataHeatBalSurf->SurfAbsThermalInt(SurfNum);
                    } else {
                        // radiant value prior to adjustment for pulse for load component report
                        Real64 const curQL = state.dataViewFactor->EnclRadInfo(radEnclosureNum).radQThermalRad;
                        // for the loads component report during the special sizing run increase the radiant portion
                        // a small amount to create a "pulse" of heat that is used for the
                        // radiant value including adjustment for pulse for load component report
                        Real64 const adjQL = curQL + state.dataViewFactor->EnclRadInfo(radEnclosureNum).FloorArea * pulseMultipler;
                        // ITABSF is the Inside Thermal Absorptance
                        // EnclRadThermAbsMult is a multiplier for each zone/enclosure
                        // SurfQdotRadIntGainsInPerArea is the thermal radiation absorbed on inside surfaces
                        state.dataHeatBal->SurfQdotRadIntGainsInPerArea(SurfNum) =
                            adjQL * state.dataViewFactor->EnclRadInfo(radEnclosureNum).radThermAbsMult *
                            state.dataHeatBalSurf->SurfAbsThermalInt(SurfNum);
                    }

                    if (NOT_SHADED(ShadeFlag)) { // No window shading
                        for (int IGlass = 1; IGlass <= TotGlassLayers; ++IGlass) {
                            state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, IGlass) +=
                                state.dataHeatBal->EnclSolQSWRad(solEnclosureNum) * state.dataConstruction->Construct(ConstrNum).AbsDiffBack(IGlass);
                        }
                    } else if (ConstrNumSh != 0 && ShadeFlag != DataSurfaces::WinShadingType::SwitchableGlazing) {
                        // Interior, exterior or between-glass shade, screen or blind in place
                        auto const &constrSh = state.dataConstruction->Construct(ConstrNumSh);
                        for (int IGlass = 1; IGlass <= constrSh.TotGlassLayers; ++IGlass) {
                            if (DataSurfaces::ANY_SHADE_SCREEN(ShadeFlag)) {
                                state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, IGlass) +=
                                    state.dataHeatBal->EnclSolQSWRad(solEnclosureNum) * constrSh.AbsDiffBack(IGlass);
                            } else if (ShadeFlag == DataSurfaces::WinShadingType::IntBlind || ShadeFlag == DataSurfaces::WinShadingType::ExtBlind) {
                                auto const &surfShade = state.dataSurface->surfShades(SurfNum);
                                Real64 interpFac = surfShade.blind.slatAngInterpFac;
                                auto const &dfAbsSlatLo = constrSh.layerSlatBlindDfAbs(IGlass)[surfShade.blind.slatAngIdxLo];
                                auto const &dfAbsSlatHi = constrSh.layerSlatBlindDfAbs(IGlass)[surfShade.blind.slatAngIdxHi];
                                // Glass layer back diffuse solar absorptance when blind in place
                                Real64 BlAbsDiffBk = Interp(dfAbsSlatLo.Sol.Bk.Df.Abs, dfAbsSlatHi.Sol.Bk.Df.Abs, interpFac);

                                state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, IGlass) +=
                                    state.dataHeatBal->EnclSolQSWRad(solEnclosureNum) * BlAbsDiffBk;
                            }
                        }
                        if (ShadeFlag == DataSurfaces::WinShadingType::IntShade) {
                            state.dataSurface->SurfWinIntLWAbsByShade(SurfNum) = state.dataViewFactor->EnclRadInfo(radEnclosureNum).radQThermalRad *
                                                                                 constrSh.ShadeAbsorpThermal *
                                                                                 state.dataViewFactor->EnclRadInfo(radEnclosureNum).radThermAbsMult;
                        } else if (ShadeFlag == DataSurfaces::WinShadingType::IntBlind) {
                            auto const &surfShade = state.dataSurface->surfShades(SurfNum);
                            Real64 EffBlEmiss = surfShade.effShadeEmi; // Blind emissivity (thermal absorptance) as part of glazing system
                            state.dataSurface->SurfWinIntLWAbsByShade(SurfNum) = state.dataViewFactor->EnclRadInfo(radEnclosureNum).radQThermalRad *
                                                                                 EffBlEmiss *
                                                                                 state.dataViewFactor->EnclRadInfo(radEnclosureNum).radThermAbsMult;
                        }

                        if (DataSurfaces::ANY_SHADE_SCREEN(ShadeFlag)) {
                            state.dataSurface->SurfWinIntSWAbsByShade(SurfNum) =
                                state.dataHeatBal->EnclSolQSWRad(solEnclosureNum) * constrSh.AbsDiffBackShade;
                        } else if (DataSurfaces::ANY_BLIND(ShadeFlag)) {
                            auto const &surfShade = state.dataSurface->surfShades(SurfNum);
                            auto const &btarLo = constrSh.blindTARs[surfShade.blind.slatAngIdxLo];
                            auto const &btarHi = constrSh.blindTARs[surfShade.blind.slatAngIdxHi];
                            Real64 interpFac = surfShade.blind.slatAngInterpFac;
                            Real64 AbsDiffBkBl = Interp(btarLo.Sol.Bk.Df.Abs, btarHi.Sol.Bk.Df.Abs, interpFac);

                            state.dataSurface->SurfWinIntSWAbsByShade(SurfNum) = state.dataHeatBal->EnclSolQSWRad(solEnclosureNum) * AbsDiffBkBl;
                        }
                        // Correct for divider shadowing
                        if (DataSurfaces::ANY_EXTERIOR_SHADE_BLIND_SCREEN(ShadeFlag)) {
                            state.dataSurface->SurfWinIntSWAbsByShade(SurfNum) *= surfWin.glazedFrac;
                        }

                    } else if (ShadeFlag == DataSurfaces::WinShadingType::SwitchableGlazing) {       // Switchable glazing
                        auto const &constructionSh = state.dataConstruction->Construct(ConstrNumSh); // What was here before?
                        for (int IGlass = 1; IGlass <= TotGlassLayers; ++IGlass) {

                            state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, IGlass) +=
                                state.dataHeatBal->EnclSolQSWRad(solEnclosureNum) *
                                Window::InterpSw(state.dataSurface->SurfWinSwitchingFactor(SurfNum),
                                                 thisConstruct.AbsDiffBack(IGlass),
                                                 constructionSh.AbsDiffBack(IGlass));
                        }

                    } // End of shading flag check

                    // Note that FrameQRadInAbs is initially calculated in InitSolarHeatGains
                    if (state.dataSurface->SurfWinFrameArea(SurfNum) > 0.0) {
                        state.dataSurface->SurfWinFrameQRadInAbs(SurfNum) +=
                            (state.dataHeatBal->EnclSolQSWRad(solEnclosureNum) * state.dataSurface->SurfWinFrameSolAbsorp(SurfNum) +
                             (state.dataViewFactor->EnclRadInfo(radEnclosureNum).radQThermalRad *
                                  state.dataViewFactor->EnclRadInfo(radEnclosureNum).radThermAbsMult +
                              state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(SurfNum)) *
                                 state.dataSurface->SurfWinFrameEmis(SurfNum)) *
                            (1.0 + 0.5 * state.dataSurface->SurfWinProjCorrFrIn(SurfNum)); // Window has a frame
                    }
                    if (state.dataSurface->SurfWinDividerArea(SurfNum) > 0.0) {                     // Window has dividers
                        Real64 DividerThermAbs = state.dataSurface->SurfWinDividerEmis(SurfNum);    // Window divider thermal absorptance
                        Real64 DividerSolAbs = state.dataSurface->SurfWinDividerSolAbsorp(SurfNum); // Window divider solar absorptance
                        if (state.dataSurface->SurfWinDividerType(SurfNum) ==
                            DataSurfaces::FrameDividerType::Suspended) {                         // Suspended divider; account for inside glass
                            Real64 MatNumGl = thisConstruct.LayerPoint(thisConstruct.TotLayers); // Glass layer material number
                            auto const *thisMaterial = dynamic_cast<const Material::MaterialGlass *>(s_mat->materials(MatNumGl));
                            assert(thisMaterial != nullptr);
                            Real64 TransGl = thisMaterial->Trans; // Glass layer solar transmittance, reflectance, absorptance
                            Real64 ReflGl = thisMaterial->ReflectSolBeamBack;
                            Real64 AbsGl = 1.0 - TransGl - ReflGl;
                            Real64 DividerSolRefl = 1.0 - DividerSolAbs; // Window divider solar reflectance
                            DividerSolAbs = AbsGl + TransGl * (DividerSolAbs + DividerSolRefl * AbsGl) / (1.0 - DividerSolRefl * ReflGl);
                            DividerThermAbs = thisMaterial->AbsorpThermalBack;
                        }
                        // Correct for interior shade transmittance
                        if (ShadeFlag == DataSurfaces::WinShadingType::IntShade) {
                            auto const &constructionSh = state.dataConstruction->Construct(ConstrNumSh);
                            int MatNumSh = constructionSh.LayerPoint(constructionSh.TotLayers); // Shade layer material number
                            auto const *matSh = dynamic_cast<Material::MaterialFen const *>(s_mat->materials(MatNumSh));
                            assert(matSh != nullptr);
                            DividerSolAbs *= matSh->Trans;
                            DividerThermAbs *= matSh->TransThermal;

                        } else if (ShadeFlag == DataSurfaces::WinShadingType::IntBlind) {
                            auto const &surfShade = state.dataSurface->surfShades(SurfNum);
                            Real64 SolBackDiffDiffTrans = surfShade.blind.TAR.Sol.Bk.Df.Tra;
                            Real64 IRBackTrans = surfShade.blind.TAR.IR.Bk.Tra;

                            DividerSolAbs *= SolBackDiffDiffTrans;
                            DividerThermAbs *= IRBackTrans;
                        }
                        // Note that DividerQRadInAbs is initially calculated in InitSolarHeatGains
                        state.dataSurface->SurfWinDividerQRadInAbs(SurfNum) +=
                            (state.dataHeatBal->EnclSolQSWRad(solEnclosureNum) * DividerSolAbs +
                             (state.dataViewFactor->EnclRadInfo(radEnclosureNum).radQThermalRad *
                                  state.dataViewFactor->EnclRadInfo(radEnclosureNum).radThermAbsMult +
                              state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(SurfNum)) *
                                 DividerThermAbs) *
                            (1.0 + state.dataSurface->SurfWinProjCorrDivIn(SurfNum));
                    }
                } else {
                    // These calculations are repeated from InitInternalHeatGains for the Zone Component Loads Report
                    Real64 pulseMultipler = 0.01; // the W/sqft pulse for the zone
                    if (!state.dataGlobal->doLoadComponentPulseNow) {
                        state.dataHeatBal->SurfQdotRadIntGainsInPerArea(SurfNum) =
                            state.dataViewFactor->EnclRadInfo(radEnclosureNum).radQThermalRad *
                            state.dataViewFactor->EnclRadInfo(radEnclosureNum).radThermAbsMult * state.dataHeatBalSurf->SurfAbsThermalInt(SurfNum);
                    } else {
                        // radiant value prior to adjustment for pulse for load component report
                        Real64 const curQL = state.dataViewFactor->EnclRadInfo(radEnclosureNum).radQThermalRad;
                        // for the loads component report during the special sizing run increase the radiant portion
                        // a small amount to create a "pulse" of heat that is used for the
                        // radiant value including adjustment for pulse for load component report
                        Real64 const adjQL = curQL + state.dataViewFactor->EnclRadInfo(radEnclosureNum).FloorArea * pulseMultipler;
                        // ITABSF is the Inside Thermal Absorptance
                        // EnclRadThermAbsMult is a multiplier for each zone/radiant enclosure
                        // SurfQdotRadIntGainsInPerArea is the thermal radiation absorbed on inside surfaces
                        state.dataHeatBal->SurfQdotRadIntGainsInPerArea(SurfNum) =
                            adjQL * state.dataViewFactor->EnclRadInfo(radEnclosureNum).radThermAbsMult *
                            state.dataHeatBalSurf->SurfAbsThermalInt(SurfNum);
                    }
                    // Radiations absorbed by the window layers coming from zone side
                    int EQLNum = thisConstruct.EQLConsPtr;
                    for (int Lay = 1; Lay <= state.dataWindowEquivLayer->CFS(EQLNum).NL; ++Lay) {
                        state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, Lay) +=
                            state.dataHeatBal->EnclSolQSWRad(solEnclosureNum) * thisConstruct.AbsDiffBackEQL(Lay);
                    }
                    // Window frame has not been included for equivalent layer model yet

                } // end if for IF ( SurfaceWindow(SurfNum)%WindowModelType /= WindowModel:: EQL) THEN

                if (surface.ExtBoundCond > 0) { // Interzone surface
                    // Short-wave radiation absorbed in panes of corresponding window in adjacent zone
                    int SurfNumAdjZone = surface.ExtBoundCond; // Surface number in adjacent zone for interzone surfaces
                    if (state.dataSurface->SurfWinWindowModelType(SurfNumAdjZone) != DataSurfaces::WindowModel::EQL) {
                        int TotGlassLayers = thisConstruct.TotGlassLayers;
                        for (int IGlass = 1; IGlass <= TotGlassLayers; ++IGlass) {
                            state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNumAdjZone, IGlass) +=
                                state.dataHeatBal->EnclSolQSWRad(solEnclosureNum) *
                                state.dataConstruction->Construct(state.dataSurface->Surface(SurfNumAdjZone).Construction).AbsDiff(IGlass);
                            // Note that AbsDiff rather than AbsDiffBack is used in the above since the
                            // radiation from the current zone is incident on the outside of the adjacent
                            // zone's window.
                        }
                    } else { // IF (SurfaceWindow(SurfNumAdjZone)%WindowModelType == WindowModel:: EQL) THEN
                        int const AdjConstrNum = state.dataSurface->Surface(SurfNumAdjZone).Construction;
                        int const EQLNum = state.dataConstruction->Construct(AdjConstrNum).EQLConsPtr;
                        for (int Lay = 1; Lay <= state.dataWindowEquivLayer->CFS(EQLNum).NL; ++Lay) {
                            state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNumAdjZone, Lay) +=
                                state.dataHeatBal->EnclSolQSWRad(solEnclosureNum) * thisConstruct.AbsDiffFrontEQL(Lay);
                            // Note that AbsDiffFrontEQL rather than AbsDiffBackEQL is used in the above
                            // since the radiation from the current zone is incident on the outside of the
                            // adjacent zone's window.
                        }
                    }
                }

                if (state.dataSurface->SurfWinWindowModelType(SurfNum) == DataSurfaces::WindowModel::Detailed) {
                    int const ConstrNumSh = state.dataSurface->SurfWinActiveShadedConstruction(SurfNum);
                    int TotGlassLayers = state.dataConstruction->Construct(ConstrNum).TotGlassLayers;
                    DataSurfaces::WinShadingType ShadeFlag = state.dataSurface->SurfWinShadingFlag(SurfNum);
                    if (DataSurfaces::NOT_SHADED(ShadeFlag)) { // No window shading
                        for (int IGlass = 1; IGlass <= TotGlassLayers; ++IGlass) {
                            state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, IGlass) += state.dataHeatBal->SurfWinInitialDifSolwinAbs(SurfNum, IGlass);
                        }
                    } else if (ShadeFlag == DataSurfaces::WinShadingType::SwitchableGlazing) { // Switchable glazing
                        for (int IGlass = 1; IGlass <= TotGlassLayers; ++IGlass) {
                            state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, IGlass) += state.dataHeatBal->SurfWinInitialDifSolwinAbs(SurfNum, IGlass);
                        }
                    } else {
                        auto const &constructionSh = state.dataConstruction->Construct(ConstrNumSh);
                        // Interior, exterior or between-glass shade, screen or blind in place
                        for (int IGlass = 1; IGlass <= constructionSh.TotGlassLayers; ++IGlass) {
                            state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, IGlass) += state.dataHeatBal->SurfWinInitialDifSolwinAbs(SurfNum, IGlass);
                        }
                        if (DataSurfaces::ANY_SHADE_SCREEN(ShadeFlag) || DataSurfaces::ANY_BLIND(ShadeFlag)) {
                            state.dataSurface->SurfWinIntSWAbsByShade(SurfNum) += state.dataSurface->SurfWinInitialDifSolAbsByShade(SurfNum);
                        }
                    } // End of shading flag check
                } else if (state.dataSurface->SurfWinWindowModelType(SurfNum) == DataSurfaces::WindowModel::BSDF) {
                    int TotGlassLayers = thisConstruct.TotGlassLayers;
                    for (int IGlass = 1; IGlass <= TotGlassLayers; ++IGlass) {
                        state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, IGlass) += state.dataHeatBal->SurfWinInitialDifSolwinAbs(SurfNum, IGlass);
                    }
                } else if (state.dataSurface->SurfWinWindowModelType(SurfNum) == DataSurfaces::WindowModel::EQL) {

                    // ConstrNum   = Surface(SurfNum)%Construction
                    int EQLNum = thisConstruct.EQLConsPtr;

                    for (int Lay = 1; Lay <= state.dataWindowEquivLayer->CFS(EQLNum).NL; ++Lay) {
                        state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, Lay) += state.dataHeatBal->SurfWinInitialDifSolwinAbs(SurfNum, Lay);
                    }
                }

            } // End of window
        }
    }
    Dayltg::DistributeTDDAbsorbedSolar(state);
}

void ComputeIntThermalAbsorpFactors(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy Code (George Walton)
    //       DATE WRITTEN   Legacy: Dec 1976
    //       MODIFIED       Nov. 99, FCW: to take into account movable interior shades and switchable glazing
    //                      June 01, FCW: to take into account interior blinds.

    // PURPOSE OF THIS SUBROUTINE:
    // This routine computes the fractions of long-wave radiation from lights, equipment and people
    // that is absorbed by each zone surface.

    // METHODOLOGY EMPLOYED:
    // The fraction is assumed to be proportional to the product of the surface area times its thermal absorptivity.

    // REFERENCES:
    // BLAST Routine: CITAF - Compute Interior Thermal Absorption Factors
    auto &s_mat = state.dataMaterial;

    for (auto const &thisEnclosure : state.dataViewFactor->EnclRadInfo) {
        if (!thisEnclosure.radReCalc) {
            continue;
        }
        for (int spaceNum : thisEnclosure.spaceNums) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurfWin = thisSpace.WindowSurfaceFirst;
            int const lastSurfWin = thisSpace.WindowSurfaceLast;
            for (int SurfNum = firstSurfWin; SurfNum <= lastSurfWin; ++SurfNum) {
                DataSurfaces::WinShadingType ShadeFlag = state.dataSurface->SurfWinShadingFlag(SurfNum);
                if (DataSurfaces::ANY_INTERIOR_SHADE_BLIND(ShadeFlag)) {
                    auto const &surfShade = state.dataSurface->surfShades(SurfNum);
                    state.dataHeatBalSurf->SurfAbsThermalInt(SurfNum) = surfShade.effShadeEmi + surfShade.effGlassEmi;
                } else {
                    int ConstrNum = state.dataSurface->SurfActiveConstruction(SurfNum);
                    state.dataHeatBalSurf->SurfAbsThermalInt(SurfNum) = state.dataConstruction->Construct(ConstrNum).InsideAbsorpThermal;
                }
            }
        }
    }
    if (state.dataSurface->AnyMovableSlat) {
        for (int SurfNum : state.dataHeatBalSurf->SurfMovSlatsIndexList) {
            // For window with an interior shade or blind, emissivity is a combination of glass and shade/blind emissivity
            DataSurfaces::WinShadingType ShadeFlag = state.dataSurface->SurfWinShadingFlag(SurfNum);
            // Not sure we need to do this anymore
            if (DataSurfaces::ANY_INTERIOR_SHADE_BLIND(ShadeFlag)) {
                auto const &surfShade = state.dataSurface->surfShades(SurfNum);
                state.dataHeatBalSurf->SurfAbsThermalInt(SurfNum) = surfShade.effShadeEmi + surfShade.effGlassEmi;
            }
        }
    }

    for (auto &thisRadEnclosure : state.dataViewFactor->EnclRadInfo) {
        if (!thisRadEnclosure.radReCalc) {
            continue;
        }
        Real64 SUM1 = 0.0;
        for (int const SurfNum : thisRadEnclosure.SurfacePtr) {
            auto &surf = state.dataSurface->Surface(SurfNum);
            int const ConstrNum = surf.Construction;
            auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);
            DataSurfaces::WinShadingType ShadeFlag = state.dataSurface->SurfWinShadingFlag(SurfNum);
            if (ShadeFlag != DataSurfaces::WinShadingType::SwitchableGlazing) {
                SUM1 += surf.Area * state.dataHeatBalSurf->SurfAbsThermalInt(SurfNum);
            } else { // Switchable glazing
                SUM1 += surf.Area * Window::InterpSw(state.dataSurface->SurfWinSwitchingFactor(SurfNum),
                                                     thisConstruct.InsideAbsorpThermal,
                                                     state.dataConstruction->Construct(surf.activeShadedConstruction).InsideAbsorpThermal);
            }

            // Window frame and divider effects
            if (state.dataSurface->SurfWinFrameArea(SurfNum) > 0.0) {
                SUM1 += state.dataSurface->SurfWinFrameArea(SurfNum) * (1.0 + 0.5 * state.dataSurface->SurfWinProjCorrFrIn(SurfNum)) *
                        state.dataSurface->SurfWinFrameEmis(SurfNum);
            }
            if (state.dataSurface->SurfWinDividerArea(SurfNum) > 0.0) {
                Real64 DividerThermAbs = state.dataSurface->SurfWinDividerEmis(SurfNum); // Window divider thermal absorptance
                // Suspended (between-glass) divider; relevant emissivity is inner glass emissivity
                if (state.dataSurface->SurfWinDividerType(SurfNum) == DataSurfaces::FrameDividerType::Suspended) {
                    DividerThermAbs = thisConstruct.InsideAbsorpThermal;
                }
                if (DataSurfaces::ANY_INTERIOR_SHADE_BLIND(ShadeFlag)) {
                    // Interior shade or blind in place
                    int const ConstrNumSh = surf.activeShadedConstruction;
                    auto const &constructionSh = state.dataConstruction->Construct(ConstrNumSh);

                    if (state.dataSurface->SurfWinHasShadeOrBlindLayer(SurfNum)) {
                        auto const &surfShade = state.dataSurface->surfShades(SurfNum);
                        // Shade layer material number
                        int MatNumSh = constructionSh.LayerPoint(constructionSh.TotLayers);
                        auto const *matSh = dynamic_cast<Material::MaterialFen const *>(s_mat->materials(MatNumSh));
                        assert(matSh != nullptr);
                        // Shade or blind IR transmittance
                        Real64 TauShIR = matSh->TransThermal;
                        // Effective emissivity of shade or blind
                        Real64 EffShDevEmiss = surfShade.effShadeEmi;

                        if (ShadeFlag == DataSurfaces::WinShadingType::IntBlind) {
                            TauShIR = surfShade.blind.TAR.IR.Bk.Tra;
                        }
                        SUM1 += state.dataSurface->SurfWinDividerArea(SurfNum) * (EffShDevEmiss + DividerThermAbs * TauShIR);
                    } else {
                        // this is for EMS activated shade/blind but the window construction has no shade/blind layer
                        SUM1 += state.dataSurface->SurfWinDividerArea(SurfNum) * (1.0 + state.dataSurface->SurfWinProjCorrDivIn(SurfNum)) *
                                DividerThermAbs;
                    }
                } else {
                    SUM1 +=
                        state.dataSurface->SurfWinDividerArea(SurfNum) * (1.0 + state.dataSurface->SurfWinProjCorrDivIn(SurfNum)) * DividerThermAbs;
                }
            }

        } // End of loop over surfaces in zone/enclosure
        thisRadEnclosure.radThermAbsMult = 1.0 / SUM1;

    } // End of loop over enclosures
}

void ComputeIntSWAbsorpFactors(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Legacy (George Walton)
    //       DATE WRITTEN   Legacy (December 1980)
    //       MODIFIED       Nov. 99, FW; now called every time step to account for movable
    //                      window shades and insulation
    //                      Mar. 00, FW; change name from ComputeVisLightingAbsorpFactors
    //                      to ComputeIntSWAbsorpFactors
    //                      May 00, FW; add window frame and divider effects
    //                      June 01, FW: account for window blinds
    //                      Nov 01, FW: account for absorptance of exterior shades and interior or exterior blinds
    //                      Jan 03, FW: add between-glass shade/blind
    //                      May 06, RR: account for exterior window screens

    // PURPOSE OF THIS SUBROUTINE:
    // Computes VMULT, the inverse of the sum of area*(short-wave absorptance+transmittance) for
    // the surfaces in a zone. VMULT is used to calculate the zone interior diffuse short-wave radiation
    // absorbed by the inside of opaque zone surfaces or by the glass and shade/blind layers of zone windows.

    // Sets VCONV to zero (VCONV was formerly used to calculate convective gain due to short-wave
    // radiation absorbed by interior window shades).

    // REFERENCES:
    // BLAST Routine - CIVAF - Compute Surface Absorption Factors For Short Wave Radiation
    //                         From Zone Lights And Diffuse Solar.

    // Avoid a division by zero of the user has entered a bunch of surfaces with zero absorptivity on the inside
    Real64 constexpr SmallestAreaAbsProductAllowed(0.01);

    auto &s_mat = state.dataMaterial;

    for (auto &thisSolEnclosure : state.dataViewFactor->EnclSolInfo) {
        if (!thisSolEnclosure.radReCalc) {
            continue;
        }
        Real64 SUM1 = 0.0; // Intermediate calculation value for solar absorbed and transmitted

        for (int const SurfNum : thisSolEnclosure.SurfacePtr) {
            auto &thisSurf = state.dataSurface->Surface(SurfNum);
            int const ConstrNum = state.dataSurface->SurfActiveConstruction(SurfNum);
            auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);
            if (thisConstruct.TransDiff <= 0.0) {
                // Opaque surface
                Real64 AbsIntSurf = state.dataHeatBalSurf->SurfAbsSolarInt(SurfNum); // Inside surface short-wave absorptance
                SUM1 += thisSurf.Area * AbsIntSurf;

            } else {
                // Window
                if (!state.dataConstruction->Construct(thisSurf.Construction).WindowTypeEQL) {
                    DataSurfaces::WinShadingType ShadeFlag = state.dataSurface->SurfWinShadingFlag(SurfNum);

                    Real64 AbsDiffTotWin = 0.0; // Sum of window layer short-wave absorptances
                    int const ConstrNumSh = state.dataSurface->SurfWinActiveShadedConstruction(SurfNum);
                    Real64 SwitchFac = state.dataSurface->SurfWinSwitchingFactor(SurfNum);

                    // Sum of absorptances of glass layers
                    for (int Lay = 1; Lay <= thisConstruct.TotGlassLayers; ++Lay) {
                        Real64 AbsDiffLayWin = thisConstruct.AbsDiffBack(Lay); // Window layer short-wave absorptance

                        // Window with shade, screen or blind
                        if (ConstrNumSh != 0) {
                            auto const &constrSh = state.dataConstruction->Construct(ConstrNumSh);
                            if (DataSurfaces::ANY_SHADE_SCREEN(ShadeFlag)) {
                                AbsDiffLayWin = constrSh.AbsDiffBack(Lay);
                            } else if (DataSurfaces::ANY_BLIND(ShadeFlag)) {
                                auto const &surfShade = state.dataSurface->surfShades(SurfNum);
                                auto const &dfAbsSlatLo = constrSh.layerSlatBlindDfAbs(Lay)[surfShade.blind.slatAngIdxLo];
                                auto const &dfAbsSlatHi = constrSh.layerSlatBlindDfAbs(Lay)[surfShade.blind.slatAngIdxHi];
                                Real64 interpFac = surfShade.blind.slatAngInterpFac;
                                AbsDiffLayWin = Interp(dfAbsSlatLo.Sol.Bk.Df.Abs, dfAbsSlatHi.Sol.Bk.Df.Abs, interpFac);
                            }
                        }

                        // Switchable glazing
                        if (ShadeFlag == DataSurfaces::WinShadingType::SwitchableGlazing) {
                            assert(ConstrNumSh > 0); // Should this be included in the if above
                            auto const &constrSh = state.dataConstruction->Construct(ConstrNumSh);
                            AbsDiffLayWin = Window::InterpSw(SwitchFac, AbsDiffLayWin, constrSh.AbsDiffBack(Lay));
                        }
                        AbsDiffTotWin += AbsDiffLayWin;
                    }

                    Real64 TransDiffWin = thisConstruct.TransDiff; // Window diffuse short-wave transmittance
                    Real64 DiffAbsShade = 0.0;                     // Diffuse short-wave shade or blind absorptance

                    // Window with shade, screen or blind

                    if (ConstrNumSh != 0) {
                        auto const &constrSh = state.dataConstruction->Construct(ConstrNumSh);
                        if (ANY_SHADE_SCREEN(ShadeFlag)) {
                            TransDiffWin = constrSh.TransDiff;
                            DiffAbsShade = constrSh.AbsDiffBackShade;
                        } else if (ANY_BLIND(ShadeFlag)) {
                            auto const &surfShade = state.dataSurface->surfShades(SurfNum);
                            auto const &btarSlatLo = constrSh.blindTARs[surfShade.blind.slatAngIdxLo];
                            auto const &btarSlatHi = constrSh.blindTARs[surfShade.blind.slatAngIdxHi];
                            Real64 interpFac = surfShade.blind.slatAngInterpFac;
                            TransDiffWin = Interp(btarSlatLo.Sol.Ft.Df.Tra, btarSlatHi.Sol.Ft.Df.Tra, interpFac);
                            DiffAbsShade = Interp(btarSlatLo.Sol.Bk.Df.Abs, btarSlatHi.Sol.Bk.Df.Abs, interpFac);
                        }
                    }

                    // Switchable glazing

                    if (ShadeFlag == DataSurfaces::WinShadingType::SwitchableGlazing) {
                        assert(ConstrNumSh > 0);
                        auto const &constructionSh = state.dataConstruction->Construct(ConstrNumSh);
                        TransDiffWin = Window::InterpSw(SwitchFac, TransDiffWin, constructionSh.TransDiff);
                    }

                    SUM1 += thisSurf.Area * (TransDiffWin + AbsDiffTotWin + DiffAbsShade);

                    // Window frame and divider effects (shade area is glazed area plus divider area)

                    if (state.dataSurface->SurfWinFrameArea(SurfNum) > 0.0) {
                        SUM1 += state.dataSurface->SurfWinFrameArea(SurfNum) * state.dataSurface->SurfWinFrameSolAbsorp(SurfNum) *
                                (1.0 + 0.5 * state.dataSurface->SurfWinProjCorrFrIn(SurfNum));
                    }
                    if (state.dataSurface->SurfWinDividerArea(SurfNum) > 0.0) {
                        Real64 DividerAbs = state.dataSurface->SurfWinDividerSolAbsorp(SurfNum); // Window divider solar absorptance
                        if (state.dataSurface->SurfWinDividerType(SurfNum) == DataSurfaces::FrameDividerType::Suspended) {
                            // Suspended (between-glass) divider: account for glass on inside of divider
                            Real64 MatNumGl = thisConstruct.LayerPoint(thisConstruct.TotLayers); // Glass material number
                            auto const *thisMaterial = dynamic_cast<const Material::MaterialGlass *>(s_mat->materials(MatNumGl));
                            assert(thisMaterial != nullptr);
                            Real64 TransGl = thisMaterial->Trans; // Glass layer short-wave transmittance, reflectance, absorptance
                            Real64 ReflGl = thisMaterial->ReflectSolBeamBack;
                            Real64 AbsGl = 1.0 - TransGl - ReflGl;
                            Real64 DividerRefl = 1.0 - DividerAbs; // Window divider short-wave reflectance
                            DividerAbs = AbsGl + TransGl * (DividerAbs + DividerRefl * AbsGl) / (1.0 - DividerRefl * ReflGl);
                        }
                        if (ANY_INTERIOR_SHADE_BLIND(ShadeFlag)) {
                            SUM1 += state.dataSurface->SurfWinDividerArea(SurfNum) * (DividerAbs + DiffAbsShade);
                        } else {
                            SUM1 += state.dataSurface->SurfWinDividerArea(SurfNum) * (1.0 + state.dataSurface->SurfWinProjCorrDivIn(SurfNum)) *
                                    DividerAbs;
                        }
                    }
                } else { // equivalent layer window
                    // In equivalent layer window solid layers (Glazing and shades) are treated equally
                    // frames and dividers are not supported
                    Real64 AbsDiffTotWin = 0.0;
                    Real64 AbsDiffLayWin = 0.0;
                    Real64 TransDiffWin = thisConstruct.TransDiff;
                    for (int Lay = 1; Lay <= state.dataWindowEquivLayer->CFS(thisConstruct.EQLConsPtr).NL; ++Lay) {
                        AbsDiffLayWin = thisConstruct.AbsDiffBackEQL(Lay);
                        AbsDiffTotWin += AbsDiffLayWin;
                    }
                    SUM1 += thisSurf.Area * (TransDiffWin + AbsDiffTotWin);
                }
            } // End of check if opaque surface or window
        } // End of loop over surfaces in zone

        if (SUM1 > SmallestAreaAbsProductAllowed) { // Everything is okay, proceed with the regular calculation
            thisSolEnclosure.solVMULT = 1.0 / SUM1;

        } else { // the sum of area*solar absorptance for all surfaces in the zone is zero--either the user screwed up
            // or they really want to disallow any solar from being absorbed on the inside surfaces.  Fire off a
            // nasty warning message and then assume that no solar is ever absorbed (basically everything goes
            // back out whatever window is there.  Note that this also assumes that the shade has no effect.
            // That's probably not correct, but how correct is it to assume that no solar is absorbed anywhere
            // in the zone?
            if (thisSolEnclosure.solAbsFirstCalc) {
                ShowWarningError(
                    state,
                    format("ComputeIntSWAbsorbFactors: Sum of area times inside solar absorption for all surfaces is zero in Enclosure: {}",
                           thisSolEnclosure.Name));
                thisSolEnclosure.solAbsFirstCalc = false;
            }
            thisSolEnclosure.solVMULT = 0.0;
        }
    } // End of enclosure loop
}

void ComputeDifSolExcZonesWIZWindows(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       MODIFIED       Jun 2007 - Lawrie - Speed enhancements.
    //       RE-ENGINEERED  Winkelmann, Lawrie

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine computes the diffuse solar exchange factors between enclosures with
    // interzone windows.

    int const numEnclosures = state.dataViewFactor->NumOfSolarEnclosures;
    if (!allocated(state.dataHeatBalSurf->ZoneFractDifShortZtoZ)) {
        state.dataHeatBalSurf->ZoneFractDifShortZtoZ.allocate(numEnclosures, numEnclosures);
        state.dataHeatBalSurf->EnclSolRecDifShortFromZ.allocate(numEnclosures);
        state.dataHeatBalSurfMgr->DiffuseArray.allocate(numEnclosures, numEnclosures);
    }

    state.dataHeatBalSurf->EnclSolRecDifShortFromZ = false;
    state.dataHeatBalSurf->ZoneFractDifShortZtoZ.to_identity();
    state.dataHeatBalSurfMgr->DiffuseArray.to_identity();

    //      IF (.not. ANY(Zone%HasInterZoneWindow)) RETURN  ! this caused massive diffs
    if (state.dataGlobal->KickOffSimulation || state.dataGlobal->KickOffSizing) {
        return;
    }
    //            Compute fraction transmitted in one pass.

    for (int const SurfNum : state.dataSurface->AllHTWindowSurfaceList) {
        auto &surface = state.dataSurface->Surface(SurfNum);
        if (surface.ExtBoundCond <= 0) {
            continue;
        }
        if (surface.ExtBoundCond == SurfNum) {
            continue;
        }
        if (state.dataConstruction->Construct(surface.Construction).TransDiff <= 0.0) {
            continue;
        }

        int surfEnclNum = surface.SolarEnclIndex;
        if (!state.dataViewFactor->EnclSolInfo(surfEnclNum).HasInterZoneWindow) {
            continue;
        }
        int MZ = state.dataSurface->Surface(surface.ExtBoundCond).SolarEnclIndex;
        state.dataHeatBalSurf->ZoneFractDifShortZtoZ(surfEnclNum, MZ) += state.dataConstruction->Construct(surface.Construction).TransDiff *
                                                                         state.dataViewFactor->EnclSolInfo(surfEnclNum).solVMULT * surface.Area;
        if (state.dataViewFactor->EnclSolInfo(surfEnclNum).solVMULT != 0.0) {
            state.dataHeatBalSurf->EnclSolRecDifShortFromZ(surfEnclNum) = true;
        }
    }
    //          Compute fractions for multiple passes.

    Array2D<Real64>::size_type l(0u), m(0u), d(0u);
    for (int NZ = 1; NZ <= numEnclosures; ++NZ, d += numEnclosures + 1) {
        m = NZ - 1;
        Real64 D_d(0.0); // Local accumulator
        for (int MZ = 1; MZ <= numEnclosures; ++MZ, ++l, m += numEnclosures) {
            if (MZ == NZ) {
                continue;
            }
            state.dataHeatBalSurfMgr->DiffuseArray[l] =
                state.dataHeatBalSurf->ZoneFractDifShortZtoZ[l] /
                (1.0 - state.dataHeatBalSurf->ZoneFractDifShortZtoZ[l] *
                           state.dataHeatBalSurf->ZoneFractDifShortZtoZ[m]); // [ l ] == ( MZ, NZ ), [ m ] == ( NZ, MZ )
            D_d += state.dataHeatBalSurf->ZoneFractDifShortZtoZ[m] * state.dataHeatBalSurfMgr->DiffuseArray[l];
        }
        state.dataHeatBalSurfMgr->DiffuseArray[d] += D_d; // [ d ] == ( NZ, NZ )
    }

    state.dataHeatBalSurf->ZoneFractDifShortZtoZ = state.dataHeatBalSurfMgr->DiffuseArray;
    // added for CR 7999 & 7869
    assert(state.dataHeatBalSurf->ZoneFractDifShortZtoZ.isize1() == numEnclosures);
    assert(state.dataHeatBalSurf->ZoneFractDifShortZtoZ.isize2() == numEnclosures);
    for (int NZ = 1; NZ <= numEnclosures; ++NZ) {
        for (int MZ = 1; MZ <= numEnclosures; ++MZ) {
            if (MZ == NZ) {
                continue;
            }
            if (state.dataHeatBalSurf->ZoneFractDifShortZtoZ(MZ, NZ) > 0.0) {
                state.dataHeatBalSurf->EnclSolRecDifShortFromZ(NZ) = true;
                break;
            }
        }
    }

    //           Compute fractions for multiple zones.

    for (int IZ = 1; IZ <= numEnclosures; ++IZ) {
        if (!state.dataHeatBalSurf->EnclSolRecDifShortFromZ(IZ)) {
            continue;
        }

        for (int JZ = 1; JZ <= numEnclosures; ++JZ) {
            if (!state.dataHeatBalSurf->EnclSolRecDifShortFromZ(JZ)) {
                continue;
            }
            if (IZ == JZ) {
                continue;
            }
            if (state.dataHeatBalSurfMgr->DiffuseArray(IZ, JZ) == 0.0) {
                continue;
            }

            for (int KZ = 1; KZ <= numEnclosures; ++KZ) {
                if (!state.dataHeatBalSurf->EnclSolRecDifShortFromZ(KZ)) {
                    continue;
                }
                if (IZ == KZ) {
                    continue;
                }
                if (JZ == KZ) {
                    continue;
                }
                if (state.dataHeatBalSurfMgr->DiffuseArray(JZ, KZ) == 0.0) {
                    continue;
                }
                state.dataHeatBalSurf->ZoneFractDifShortZtoZ(IZ, KZ) +=
                    state.dataHeatBalSurfMgr->DiffuseArray(JZ, KZ) * state.dataHeatBalSurfMgr->DiffuseArray(IZ, JZ);

                for (int LZ = 1; LZ <= numEnclosures; ++LZ) {
                    if (!state.dataHeatBalSurf->EnclSolRecDifShortFromZ(LZ)) {
                        continue;
                    }
                    if (IZ == LZ) {
                        continue;
                    }
                    if (JZ == LZ) {
                        continue;
                    }
                    if (KZ == LZ) {
                        continue;
                    }
                    if (state.dataHeatBalSurfMgr->DiffuseArray(KZ, LZ) == 0.0) {
                        continue;
                    }
                    state.dataHeatBalSurf->ZoneFractDifShortZtoZ(IZ, LZ) += state.dataHeatBalSurfMgr->DiffuseArray(KZ, LZ) *
                                                                            state.dataHeatBalSurfMgr->DiffuseArray(JZ, KZ) *
                                                                            state.dataHeatBalSurfMgr->DiffuseArray(IZ, JZ);

                    for (int MZ = 1; MZ <= numEnclosures; ++MZ) {
                        if (!state.dataHeatBalSurf->EnclSolRecDifShortFromZ(MZ)) {
                            continue;
                        }
                        if (IZ == MZ) {
                            continue;
                        }
                        if (JZ == MZ) {
                            continue;
                        }
                        if (KZ == MZ) {
                            continue;
                        }
                        if (LZ == MZ) {
                            continue;
                        }
                        if (state.dataHeatBalSurfMgr->DiffuseArray(LZ, MZ) == 0.0) {
                            continue;
                        }
                        state.dataHeatBalSurf->ZoneFractDifShortZtoZ(IZ, MZ) +=
                            state.dataHeatBalSurfMgr->DiffuseArray(LZ, MZ) * state.dataHeatBalSurfMgr->DiffuseArray(KZ, LZ) *
                            state.dataHeatBalSurfMgr->DiffuseArray(JZ, KZ) * state.dataHeatBalSurfMgr->DiffuseArray(IZ, JZ);
                    } // MZ Loop

                } // LZ Loop

            } // KZ Loop

        } // JZ Loop

    } // IZ Loop
} // ComputeDifSolExcZoneWIZWindows()

void InitEMSControlledSurfaceProperties(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   April 2011

    // PURPOSE OF THIS SUBROUTINE:
    // initialize material and construction surface properties if being overridden by EMS

    // METHODOLOGY EMPLOYED:
    // update solar, thermal and visible absorptance values when actuated by EMS

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int TotLayers;       // count of material layers in a construction
    int InsideMaterNum;  // integer pointer for inside face's material layer
    int OutsideMaterNum; // integer pointer for outside face's material layer

    auto &s_mat = state.dataMaterial;

    state.dataGlobal->AnySurfPropOverridesInModel = false;
    // first determine if anything needs to be done, once yes, then always init
    for (auto const *mat : s_mat->materials) {
        if (mat->group != Material::Group::Regular) {
            continue;
        }

        if ((mat->AbsorpSolarEMSOverrideOn) || (mat->AbsorpThermalEMSOverrideOn) || (mat->AbsorpVisibleEMSOverrideOn)) {
            state.dataGlobal->AnySurfPropOverridesInModel = true;
            break;
        }
    }

    if (!state.dataGlobal->AnySurfPropOverridesInModel) {
        return; // quick return if nothing has ever needed to be done
    }

    // first, loop over materials
    // why is this a second loop?
    for (auto *mat : s_mat->materials) {
        if (mat->group != Material::Group::Regular) {
            continue;
        }

        mat->AbsorpSolar = mat->AbsorpSolarEMSOverrideOn ? max(min(mat->AbsorpSolarEMSOverride, 0.9999), 0.0001) : mat->AbsorpSolarInput;
        mat->AbsorpThermal = mat->AbsorpThermalEMSOverrideOn ? max(min(mat->AbsorpThermalEMSOverride, 0.9999), 0.0001) : mat->AbsorpThermalInput;
        mat->AbsorpVisible = mat->AbsorpVisibleEMSOverrideOn ? max(min(mat->AbsorpVisibleEMSOverride, 0.9999), 0.0001) : mat->AbsorpVisibleInput;
    } // loop over materials

    // second, loop over constructions
    for (auto &thisConstruct : state.dataConstruction->Construct) {
        if (thisConstruct.TypeIsWindow) {
            continue; // only override opaque constructions
        }
        TotLayers = thisConstruct.TotLayers;
        if (TotLayers == 0) {
            continue; // error condition
        }
        InsideMaterNum = thisConstruct.LayerPoint(TotLayers);
        if (InsideMaterNum != 0) {
            auto const *mat = s_mat->materials(InsideMaterNum);
            thisConstruct.InsideAbsorpVis = mat->AbsorpVisible;
            thisConstruct.InsideAbsorpSolar = mat->AbsorpSolar;
            thisConstruct.InsideAbsorpThermal = mat->AbsorpThermal;
        }

        OutsideMaterNum = thisConstruct.LayerPoint(1);
        if (OutsideMaterNum != 0) {
            auto const *mat = s_mat->materials(OutsideMaterNum);
            thisConstruct.OutsideAbsorpVis = mat->AbsorpVisible;
            thisConstruct.OutsideAbsorpSolar = mat->AbsorpSolar;
            thisConstruct.OutsideAbsorpThermal = mat->AbsorpThermal;
        }
    } // for (ConstrNum)
} // InitEMSControlledSurfaceProperties()

void InitEMSControlledConstructions(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   Jan 2012

    // PURPOSE OF THIS SUBROUTINE:
    // change construction on surface if overridden by EMS

    state.dataGlobal->AnyConstrOverridesInModel = false;
    for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
        if (state.dataSurface->SurfEMSConstructionOverrideON(SurfNum)) {
            state.dataGlobal->AnyConstrOverridesInModel = true;
            break;
        }
    }
    if (!state.dataGlobal->AnyConstrOverridesInModel) {
        return;
    }

    for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
        auto &surface = state.dataSurface->Surface(SurfNum);

        if (state.dataSurface->SurfEMSConstructionOverrideON(SurfNum) && (state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum) > 0)) {

            if (state.dataConstruction->Construct(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum))
                    .TypeIsWindow) { // okay, always allow windows
                state.dataRuntimeLang->EMSConstructActuatorChecked(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum), SurfNum) = true;
                state.dataRuntimeLang->EMSConstructActuatorIsOkay(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum), SurfNum) = true;
            }

            if ((state.dataRuntimeLang->EMSConstructActuatorChecked(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum), SurfNum)) &&
                (state.dataRuntimeLang->EMSConstructActuatorIsOkay(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum), SurfNum))) {

                surface.Construction = state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum);
                state.dataConstruction->Construct(surface.Construction).IsUsed = true;
                state.dataSurface->SurfActiveConstruction(SurfNum) = state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum);

            } else { // have not checked yet or is not okay, so see if we need to warn about incompatible
                if (!state.dataRuntimeLang->EMSConstructActuatorChecked(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum), SurfNum)) {
                    // check if constructions appear compatible

                    if (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CTF ||
                        surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::EMPD) {
                        // compare old construction to new construction and see if terms match
                        // set as okay and turn false if find a big problem
                        state.dataRuntimeLang->EMSConstructActuatorIsOkay(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum), SurfNum) =
                            true;
                        state.dataRuntimeLang->EMSConstructActuatorChecked(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum), SurfNum) =
                            true;
                        if (state.dataConstruction->Construct(surface.Construction).NumHistories !=
                            state.dataConstruction->Construct(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum)).NumHistories) {
                            // throw warning, but allow
                            ShowWarningError(state,
                                             "InitEMSControlledConstructions: EMS Construction State Actuator may be unrealistic, incompatible "
                                             "CTF timescales are being used.");
                            ShowContinueError(state,
                                              format("Construction named = {} has CTF timesteps = {}",
                                                     state.dataConstruction->Construct(surface.Construction).Name,
                                                     state.dataConstruction->Construct(surface.Construction).NumHistories));
                            ShowContinueError(
                                state,
                                format("While construction named = {} has CTF timesteps = {}",
                                       state.dataConstruction->Construct(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum)).Name,
                                       state.dataConstruction->Construct(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum)).NumHistories));
                            ShowContinueError(
                                state,
                                format("Transient heat transfer modeling may not be valid for surface name = {}, and the simulation continues",
                                       surface.Name));
                        }
                        if (state.dataConstruction->Construct(surface.Construction).NumCTFTerms !=
                            state.dataConstruction->Construct(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum)).NumCTFTerms) {
                            // throw warning, but allow
                            ShowWarningError(state,
                                             "InitEMSControlledConstructions: EMS Construction State Actuator may be unrealistic, incompatible "
                                             "CTF terms are being used.");
                            ShowContinueError(state,
                                              format("Construction named = {} has number of CTF terms = {}",
                                                     state.dataConstruction->Construct(surface.Construction).Name,
                                                     state.dataConstruction->Construct(surface.Construction).NumCTFTerms));
                            ShowContinueError(
                                state,
                                format("While construction named = {} has number of CTF terms = {}",
                                       state.dataConstruction->Construct(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum)).Name,
                                       state.dataConstruction->Construct(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum)).NumCTFTerms));
                            ShowContinueError(state,
                                              format("The actuator is allowed but the transient heat transfer modeling may not be valid for surface "
                                                     "name = {}, and the simulation continues",
                                                     surface.Name));
                        }

                        if (state.dataConstruction->Construct(surface.Construction).SourceSinkPresent) {
                            if (!state.dataConstruction->Construct(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum)).SourceSinkPresent) {
                                // throw warning, and do not allow
                                ShowSevereError(state, "InitEMSControlledConstructions: EMS Construction State Actuator not valid.");
                                ShowContinueError(state,
                                                  format("Construction named = {} has internal source/sink",
                                                         state.dataConstruction->Construct(surface.Construction).Name));
                                ShowContinueError(
                                    state,
                                    format("While construction named = {} is not an internal source/sink construction",
                                           state.dataConstruction->Construct(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum)).Name));
                                ShowContinueError(
                                    state,
                                    format("This actuator is not allowed for surface name = {}, and the simulation continues without the override",
                                           surface.Name));

                                state.dataRuntimeLang->EMSConstructActuatorIsOkay(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum),
                                                                                  SurfNum) = false;
                            }
                        }

                        if (state.dataRuntimeLang->EMSConstructActuatorIsOkay(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum),
                                                                              SurfNum)) {
                            surface.Construction = state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum);
                        }

                    } else if (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD) {
                        state.dataRuntimeLang->EMSConstructActuatorIsOkay(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum), SurfNum) =
                            true;
                        state.dataRuntimeLang->EMSConstructActuatorChecked(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum), SurfNum) =
                            true;
                        if (state.dataHeatBalFiniteDiffMgr->ConstructFD(surface.Construction).TotNodes !=
                            state.dataHeatBalFiniteDiffMgr->ConstructFD(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum)).TotNodes) {
                            // throw warning, and do not allow
                            ShowSevereError(state, "InitEMSControlledConstructions: EMS Construction State Actuator not valid.");
                            ShowContinueError(state,
                                              format("Construction named = {} has number of finite difference nodes ={}",
                                                     state.dataConstruction->Construct(surface.Construction).Name,
                                                     state.dataHeatBalFiniteDiffMgr->ConstructFD(surface.Construction).TotNodes));
                            ShowContinueError(
                                state,
                                format("While construction named = {}has number of finite difference nodes ={}",
                                       state.dataConstruction->Construct(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum)).Name,
                                       state.dataHeatBalFiniteDiffMgr->ConstructFD(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum))
                                           .TotNodes));
                            ShowContinueError(
                                state,
                                format("This actuator is not allowed for surface name = {}, and the simulation continues without the override",
                                       surface.Name));

                            state.dataRuntimeLang->EMSConstructActuatorIsOkay(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum), SurfNum) =
                                false;
                        }

                        if (state.dataConstruction->Construct(surface.Construction).SourceSinkPresent) {
                            if (!state.dataConstruction->Construct(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum)).SourceSinkPresent) {
                                // throw warning, and do not allow
                                ShowSevereError(state, "InitEMSControlledConstructions: EMS Construction State Actuator not valid.");
                                ShowContinueError(state,
                                                  format("Construction named = {} has internal source/sink",
                                                         state.dataConstruction->Construct(surface.Construction).Name));
                                ShowContinueError(
                                    state,
                                    format("While construction named = {} is not an internal source/sink construction",
                                           state.dataConstruction->Construct(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum)).Name));
                                ShowContinueError(
                                    state,
                                    format("This actuator is not allowed for surface name = {}, and the simulation continues without the override",
                                           surface.Name));

                                state.dataRuntimeLang->EMSConstructActuatorIsOkay(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum),
                                                                                  SurfNum) = false;
                            }
                        }

                        if (state.dataRuntimeLang->EMSConstructActuatorIsOkay(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum),
                                                                              SurfNum)) {
                            surface.Construction = state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum);
                        }

                    } else if (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::HAMT) { // don't allow
                        ShowSevereError(state,
                                        "InitEMSControlledConstructions: EMS Construction State Actuator not available with Heat transfer "
                                        "algorithm CombinedHeatAndMoistureFiniteElement.");
                        ShowContinueError(
                            state,
                            format("This actuator is not allowed for surface name = {}, and the simulation continues without the override",
                                   surface.Name));
                        state.dataRuntimeLang->EMSConstructActuatorChecked(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum), SurfNum) =
                            true;
                        state.dataRuntimeLang->EMSConstructActuatorIsOkay(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum), SurfNum) =
                            false;

                    } else if (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::Kiva) { // don't allow
                        ShowSevereError(state,
                                        "InitEMSControlledConstructions: EMS Construction State Actuator not available for Surfaces with "
                                        "Foundation Outside Boundary Condition.");
                        ShowContinueError(
                            state,
                            format("This actuator is not allowed for surface name = {}, and the simulation continues without the override",
                                   surface.Name));
                        state.dataRuntimeLang->EMSConstructActuatorChecked(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum), SurfNum) =
                            true;
                        state.dataRuntimeLang->EMSConstructActuatorIsOkay(state.dataSurface->SurfEMSConstructionOverrideValue(SurfNum), SurfNum) =
                            false;
                    }

                } else {
                    // do nothing, has been checked and is not okay with single warning already issued.
                }
            }
        } else {
            surface.Construction = surface.ConstructionStoredInputValue;
            state.dataSurface->SurfActiveConstruction(SurfNum) = surface.ConstructionStoredInputValue;
        }
    } // for (SurfNum)
} // InitEMSControlledConstructions()

// End Initialization Section of the Module
//******************************************************************************

// Begin Algorithm Section of the Module
//******************************************************************************

// Beginning of Record Keeping subroutines for the HB Module
// *****************************************************************************

void UpdateIntermediateSurfaceHeatBalanceResults(EnergyPlusData &state, ObjexxFCL::Optional_int_const ZoneToResimulate)
{
    int firstZone = 1;
    int lastZone = state.dataGlobal->NumOfZones;

    if (present(ZoneToResimulate)) {
        firstZone = ZoneToResimulate;
        lastZone = ZoneToResimulate;
    }

    for (int zoneNum = firstZone; zoneNum <= lastZone; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurf = thisSpace.WindowSurfaceFirst;
            int const lastSurf = thisSpace.WindowSurfaceLast;
            for (int surfNum = firstSurf; surfNum <= lastSurf; ++surfNum) {
                if (state.dataSurface->Surface(surfNum).ExtSolar) { // WindowManager's definition of ZoneWinHeatGain/Loss
                    state.dataHeatBal->ZoneWinHeatGain(zoneNum) += state.dataSurface->SurfWinHeatGain(surfNum);
                }
            }
            // Update zone window heat gain reports (these intermediate values are also used for Sensible Heat Gain Summary in GatherHeatGainReport)
            if (state.dataHeatBal->ZoneWinHeatGain(zoneNum) >= 0.0) {
                state.dataHeatBal->ZoneWinHeatGainRep(zoneNum) = state.dataHeatBal->ZoneWinHeatGain(zoneNum);
                state.dataHeatBal->ZoneWinHeatGainRepEnergy(zoneNum) =
                    state.dataHeatBal->ZoneWinHeatGainRep(zoneNum) * state.dataGlobal->TimeStepZoneSec;
            } else {
                state.dataHeatBal->ZoneWinHeatLossRep(zoneNum) = -state.dataHeatBal->ZoneWinHeatGain(zoneNum);
                state.dataHeatBal->ZoneWinHeatLossRepEnergy(zoneNum) =
                    state.dataHeatBal->ZoneWinHeatLossRep(zoneNum) * state.dataGlobal->TimeStepZoneSec;
            }
        }
    }

    if (state.dataSurface->UseRepresentativeSurfaceCalculations) {
        UpdateNonRepresentativeSurfaceResults(state, ZoneToResimulate);
    }

    // Opaque or window surfaces (Skip TDD:DOME objects. Inside temp is handled by TDD:DIFFUSER.)
    for (int zoneNum = firstZone; zoneNum <= lastZone; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurf = thisSpace.OpaqOrWinSurfaceFirst;
            int const lastSurf = thisSpace.OpaqOrWinSurfaceLast;
            for (int surfNum = firstSurf; surfNum <= lastSurf; ++surfNum) {
                state.dataHeatBalSurf->SurfQdotConvInPerArea(surfNum) =
                    -state.dataHeatBalSurf->SurfHConvInt(surfNum) *
                    (state.dataHeatBalSurf->SurfTempIn(surfNum) - state.dataHeatBalSurfMgr->RefAirTemp(surfNum));
            }
        }
    }
    // Opaque surfaces
    for (int zoneNum = firstZone; zoneNum <= lastZone; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurf = thisSpace.OpaqOrIntMassSurfaceFirst;
            int const lastSurf = thisSpace.OpaqOrIntMassSurfaceLast;
            for (int surfNum = firstSurf; surfNum <= lastSurf; ++surfNum) {
                state.dataHeatBalSurf->SurfQdotRadSolarInRepPerArea(surfNum) =
                    state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(surfNum) - state.dataHeatBalSurf->SurfQdotRadLightsInPerArea(surfNum);
            }
        }
    }
    // Inside face conduction calculation for Kiva surfaces
    for (int surfNum : state.dataSurface->AllHTKivaSurfaceList) {
        state.dataHeatBalSurf->SurfOpaqInsFaceCondFlux(surfNum) =
            -(state.dataHeatBalSurf->SurfQdotConvInPerArea(surfNum) + state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(surfNum) +
              state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(surfNum) + state.dataHeatBal->SurfQdotRadIntGainsInPerArea(surfNum) +
              state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(surfNum));
    }
}

void UpdateNonRepresentativeSurfaceResults(EnergyPlusData &state, ObjexxFCL::Optional_int_const ZoneToResimulate)
{
    int firstZone = 1;
    int lastZone = state.dataGlobal->NumOfZones;

    if (present(ZoneToResimulate)) {
        firstZone = ZoneToResimulate;
        lastZone = ZoneToResimulate;
    }

    for (int zoneNum = firstZone; zoneNum <= lastZone; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            // Heat transfer surfaces
            int firstSurf = thisSpace.HTSurfaceFirst;
            int lastSurf = thisSpace.HTSurfaceLast;
            for (int surfNum = firstSurf; surfNum <= lastSurf; ++surfNum) {
                auto const &surface = state.dataSurface->Surface(surfNum);
                int repSurfNum = surface.RepresentativeCalcSurfNum;

                if (surfNum != repSurfNum) {
#if 0
                // Check for divergence
                Real64 surfConv = -state.dataHeatBalSurf->SurfHConvInt(surfNum) *
                                  (state.dataHeatBalSurf->SurfTempIn(surfNum) - state.dataHeatBalSurfMgr->RefAirTemp(surfNum));
                Real64 repSurfConv = -state.dataHeatBalSurf->SurfHConvInt(repSurfNum) *
                                     (state.dataHeatBalSurf->SurfTempIn(repSurfNum) - state.dataHeatBalSurfMgr->RefAirTemp(repSurfNum));
                Real64 diff = surfConv - repSurfConv;
                if (std::abs(diff) > 3.0 && state.dataSurface->Surface(repSurfNum).ConstituentSurfaceNums.size() == 2) {
                    ShowWarningError(state, format("Difference in representative surface convection {:.3R} W/m2", diff));
                    ShowContinueErrorTimeStamp(state, "");
                    ShowContinueError(state, format("  Original Surface: {}", surface.Name));
                    ShowContinueError(state, format("    Inside surface temperature: {:.3R} C", state.dataHeatBalSurf->SurfTempIn(surfNum)));
                    ShowContinueError(state,
                                      format("    Inside convection coefficient: {:.3R} W/m2-K", state.dataHeatBalSurf->SurfHConvInt(surfNum)));
                    ShowContinueError(state,
                                      format("    Sunlit fraction: {:.3R}",
                                             state.dataHeatBal->SurfSunlitFrac(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, surfNum)));
                    ShowContinueError(state, format("    Outside absorbed solar: {:.3R} W/m2", state.dataHeatBalSurf->SurfOpaqQRadSWOutAbs(surfNum)));
                    ShowContinueError(state,
                                      format("    Outside long wave radiation: {:.3R} W/m2", state.dataHeatBalSurf->QdotRadOutRepPerArea(surfNum)));
                    ShowContinueError(state, format("  Representative Surface: {}", state.dataSurface->Surface(repSurfNum).Name));
                    ShowContinueError(state, format("    Inside surface temperature: {:.3R} C", state.dataHeatBalSurf->SurfTempIn(repSurfNum)));
                    ShowContinueError(state,
                                      format("    Inside convection coefficient: {:.3R} W/m2-K", state.dataHeatBalSurf->SurfHConvInt(repSurfNum)));
                    ShowContinueError(state,
                                      format("    Sunlit fraction: {:.3R}",
                                             state.dataHeatBal->SurfSunlitFrac(state.dataGlobal->HourOfDay, state.dataGlobal->TimeStep, repSurfNum)));
                    ShowContinueError(state,
                                      format("    Outside absorbed solar: {:.3R} W/m2", state.dataHeatBalSurf->SurfOpaqQRadSWOutAbs(repSurfNum)));
                    ShowContinueError(
                        state, format("    Outside long wave radiation: {:.3R} W/m2", state.dataHeatBalSurf->QdotRadOutRepPerArea(repSurfNum)));
                }
#endif

                    // Surface Heat Balance Arrays
                    state.dataHeatBalSurf->SurfTempIn(surfNum) = state.dataHeatBalSurf->SurfTempIn(repSurfNum);
                    state.dataHeatBalSurf->SurfTempOut(surfNum) = state.dataHeatBalSurf->SurfTempOut(repSurfNum);
                    state.dataHeatBal->SurfTempEffBulkAir(surfNum) = state.dataHeatBal->SurfTempEffBulkAir(repSurfNum);
                    state.dataHeatBalSurf->SurfHConvInt(surfNum) = state.dataHeatBalSurf->SurfHConvInt(repSurfNum);
                    state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(surfNum) = state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(repSurfNum);
                    state.dataHeatBal->SurfQdotRadIntGainsInPerArea(surfNum) = state.dataHeatBal->SurfQdotRadIntGainsInPerArea(repSurfNum);
                    state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(surfNum) = state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(repSurfNum);

                    state.dataHeatBalSurf->SurfQdotConvOutPerArea(surfNum) = state.dataHeatBalSurf->SurfQdotConvOutPerArea(repSurfNum);
                    state.dataHeatBalSurf->SurfHConvExt(surfNum) = state.dataHeatBalSurf->SurfHConvExt(repSurfNum);
                    state.dataHeatBalSurf->SurfQdotRadOutRepPerArea(surfNum) = state.dataHeatBalSurf->SurfQdotRadOutRepPerArea(repSurfNum);
                    state.dataHeatBalSurf->SurfHAirExt(surfNum) = state.dataHeatBalSurf->SurfHAirExt(repSurfNum);
                    state.dataHeatBalSurf->SurfHSkyExt(surfNum) = state.dataHeatBalSurf->SurfHSkyExt(repSurfNum);
                    state.dataHeatBalSurf->SurfHGrdExt(surfNum) = state.dataHeatBalSurf->SurfHGrdExt(repSurfNum);

                    state.dataSurface->SurfTAirRef(surfNum) = state.dataSurface->SurfTAirRef(repSurfNum);
                    if (state.dataSurface->SurfTAirRef(surfNum) != DataSurfaces::RefAirTemp::Invalid) {
                        state.dataSurface->SurfTAirRefRpt(surfNum) = DataSurfaces::SurfTAirRefReportVals[state.dataSurface->SurfTAirRef(surfNum)];
                    }

                    state.dataSurface->surfExtConv(surfNum).hfModelEq = state.dataSurface->surfExtConv(repSurfNum).hfModelEq;
                    state.dataSurface->surfExtConv(surfNum).hnModelEq = state.dataSurface->surfExtConv(repSurfNum).hnModelEq;

                    state.dataHeatBalSurf->SurfQAdditionalHeatSourceInside(surfNum) =
                        state.dataHeatBalSurf->SurfQAdditionalHeatSourceInside(repSurfNum);
                    state.dataHeatBalSurf->SurfQAdditionalHeatSourceOutside(surfNum) =
                        state.dataHeatBalSurf->SurfQAdditionalHeatSourceOutside(repSurfNum);

                    // Internal (non reporting variables)
                    state.dataHeatBalSurf->SurfTempInTmp(surfNum) = state.dataHeatBalSurf->SurfTempInTmp(repSurfNum);
                    state.dataHeatBalSurfMgr->RefAirTemp(surfNum) = state.dataHeatBalSurfMgr->RefAirTemp(repSurfNum);
                }
            }

            // Opaque surfaces
            firstSurf = thisSpace.OpaqOrIntMassSurfaceFirst;
            lastSurf = thisSpace.OpaqOrIntMassSurfaceLast;
            for (int surfNum = firstSurf; surfNum <= lastSurf; ++surfNum) {
                auto const &surface = state.dataSurface->Surface(surfNum);
                int repSurfNum = surface.RepresentativeCalcSurfNum;

                if (surfNum != repSurfNum) {
                    // Surface Heat Balance Arrays
                    state.dataHeatBalSurf->SurfQdotRadLightsInPerArea(surfNum) = state.dataHeatBalSurf->SurfQdotRadLightsInPerArea(repSurfNum);
                    state.dataHeatBalSurf->SurfOpaqQRadSWOutAbs(surfNum) = state.dataHeatBalSurf->SurfOpaqQRadSWOutAbs(repSurfNum);
                    state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(surfNum) = state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(repSurfNum);
                }
            }

            // Window surfaces
            firstSurf = thisSpace.WindowSurfaceFirst;
            lastSurf = thisSpace.WindowSurfaceLast;
            for (int surfNum = firstSurf; surfNum <= lastSurf; ++surfNum) {
                auto const &surface = state.dataSurface->Surface(surfNum);
                int repSurfNum = surface.RepresentativeCalcSurfNum;

                if (surfNum != repSurfNum) {
                    Real64 areaRatio = surface.Area / state.dataSurface->Surface(surfNum).Area;

                    // Glazing
                    state.dataSurface->SurfWinGainConvGlazToZoneRep(surfNum) =
                        state.dataSurface->SurfWinGainConvGlazToZoneRep(repSurfNum) * areaRatio;
                    state.dataSurface->SurfWinGainIRGlazToZoneRep(surfNum) = state.dataSurface->SurfWinGainIRGlazToZoneRep(repSurfNum) * areaRatio;

                    // Frame
                    Real64 frameHeatGain = 0.0;
                    if (state.dataSurface->SurfWinFrameArea(surfNum) > 0.0) {
                        Real64 frameAreaRatio = state.dataSurface->SurfWinFrameArea(surfNum) / state.dataSurface->SurfWinFrameArea(repSurfNum);
                        state.dataSurface->SurfWinFrameHeatGain(surfNum) = state.dataSurface->SurfWinFrameHeatGain(repSurfNum) * frameAreaRatio;
                        state.dataSurface->SurfWinFrameHeatLoss(surfNum) = state.dataSurface->SurfWinFrameHeatLoss(repSurfNum) * frameAreaRatio;
                        state.dataSurface->SurfWinFrameTempIn(surfNum) = state.dataSurface->SurfWinFrameTempIn(repSurfNum);
                        state.dataSurface->SurfWinFrameTempSurfOut(surfNum) = state.dataSurface->SurfWinFrameTempSurfOut(repSurfNum);
                        frameHeatGain = state.dataSurface->SurfWinFrameHeatGain(surfNum) - state.dataSurface->SurfWinFrameHeatLoss(surfNum);
                    }

                    // Divider
                    Real64 dividerHeatGain = 0.0;
                    if (state.dataSurface->SurfWinDividerArea(surfNum) > 0.0) {
                        Real64 dividerAreaRatio = state.dataSurface->SurfWinDividerArea(surfNum) / state.dataSurface->SurfWinDividerArea(repSurfNum);
                        state.dataSurface->SurfWinDividerHeatGain(surfNum) = state.dataSurface->SurfWinDividerHeatGain(repSurfNum) * dividerAreaRatio;
                        state.dataSurface->SurfWinDividerHeatLoss(surfNum) = state.dataSurface->SurfWinDividerHeatLoss(repSurfNum) * dividerAreaRatio;
                        state.dataSurface->SurfWinDividerTempIn(surfNum) = state.dataSurface->SurfWinDividerTempIn(repSurfNum);
                        state.dataSurface->SurfWinDividerTempSurfOut(surfNum) = state.dataSurface->SurfWinDividerTempSurfOut(repSurfNum);
                        dividerHeatGain = state.dataSurface->SurfWinDividerHeatGain(surfNum) - state.dataSurface->SurfWinDividerHeatLoss(surfNum);
                    }

                    state.dataSurface->SurfWinGainFrameDividerToZoneRep(surfNum) = frameHeatGain + dividerHeatGain;

                    // Whole window
                    state.dataSurface->SurfWinHeatGain(surfNum) = (state.dataSurface->SurfWinHeatGain(repSurfNum) -
                                                                   state.dataSurface->SurfWinGainFrameDividerToZoneRep(repSurfNum) * areaRatio) +
                                                                  state.dataSurface->SurfWinGainFrameDividerToZoneRep(surfNum);
                }
            }
        }
    }
}

void UpdateFinalSurfaceHeatBalance(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   December 2000

    // PURPOSE OF THIS SUBROUTINE:
    // If a radiant system is present and was on for part of the time step,
    // then we probably need to make yet another pass through the heat balance.
    // This is necessary because the heat source/sink to the surface that is
    // the radiant system may have varied during the system time steps.

    // METHODOLOGY EMPLOYED:
    // First, determine whether or not the radiant system was running.  If
    // any of the Qsource terms are non-zero, then it was running.  Then,
    // update the current source terms with the "average" value calculated
    // by the radiant system algorithm.  This requires the "USE" of the
    // radiant algorithm module.  Finally, using this source value, redo
    // the inside and outside heat balances.

    bool LowTempRadSysOn;     // .TRUE. if a low temperature radiant system is running
    bool HighTempRadSysOn;    // .TRUE. if a high temperature radiant system is running
    bool HWBaseboardSysOn;    // .TRUE. if a water baseboard heater is running
    bool SteamBaseboardSysOn; // .TRUE. if a steam baseboard heater is running
    bool ElecBaseboardSysOn;  // .TRUE. if a steam baseboard heater is running
    bool CoolingPanelSysOn;   // true if a simple cooling panel is running
    bool SwimmingPoolOn;      // true if a pool is present (running)

    LowTempRadiantSystem::UpdateRadSysSourceValAvg(state, LowTempRadSysOn);
    HighTempRadiantSystem::UpdateHTRadSourceValAvg(state, HighTempRadSysOn);
    HWBaseboardRadiator::UpdateBBRadSourceValAvg(state, HWBaseboardSysOn);
    SteamBaseboardRadiator::UpdateBBSteamRadSourceValAvg(state, SteamBaseboardSysOn);
    ElectricBaseboardRadiator::UpdateBBElecRadSourceValAvg(state, ElecBaseboardSysOn);
    CoolingPanelSimple::UpdateCoolingPanelSourceValAvg(state, CoolingPanelSysOn);
    SwimmingPool::UpdatePoolSourceValAvg(state, SwimmingPoolOn);

    if (LowTempRadSysOn || HighTempRadSysOn || HWBaseboardSysOn || SteamBaseboardSysOn || ElecBaseboardSysOn || CoolingPanelSysOn || SwimmingPoolOn) {
        // Solve the zone heat balance 'Detailed' solution
        // Call the outside and inside surface heat balances
        CalcHeatBalanceOutsideSurf(state);
        CalcHeatBalanceInsideSurf(state);
    }
}

void UpdateThermalHistories(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Russ Taylor
    //       DATE WRITTEN   June 1990
    //       RE-ENGINEERED  Mar98 (RKS)

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine updates and shifts the thermal and flux histories.

    // METHODOLOGY EMPLOYED:
    // If a surface runs on the user selected subhourly time step, then the
    // history terms for the temperatures and fluxes must simply be updated
    // and shifted.  However, if the surface runs at a different (longer) time
    // step, then the "master" history series is used for the interpolated
    // update scheme.

    // REFERENCES:
    // (I)BLAST legacy routine UTHRMH
    // Taylor et.al., Impact of Simultaneous Simulation of Buildings and
    // Mechanical Systems in Heat Balance Based Energy Analysis Programs
    // on System Response and Control, Building Simulation '91, IBPSA, Nice, France.

    if (state.dataHeatBalSurfMgr->UpdateThermalHistoriesFirstTimeFlag) {
        state.dataHeatBalSurfMgr->QExt1.dimension(state.dataSurface->TotSurfaces, 0.0);
        state.dataHeatBalSurfMgr->QInt1.dimension(state.dataSurface->TotSurfaces, 0.0);
        state.dataHeatBalSurfMgr->TempInt1.dimension(state.dataSurface->TotSurfaces, 0.0);
        state.dataHeatBalSurfMgr->TempExt1.dimension(state.dataSurface->TotSurfaces, 0.0);
        state.dataHeatBalSurfMgr->SumTime.dimension(state.dataSurface->TotSurfaces, 0.0);
        if (state.dataHeatBal->AnyInternalHeatSourceInInput) {
            state.dataHeatBalSurfMgr->Qsrc1.dimension(state.dataSurface->TotSurfaces, 0.0);
            state.dataHeatBalSurfMgr->Tsrc1.dimension(state.dataSurface->TotSurfaces, 0.0);
            state.dataHeatBalSurfMgr->Tuser1.dimension(state.dataSurface->TotSurfaces, 0.0);
        }
        state.dataHeatBalSurfMgr->UpdateThermalHistoriesFirstTimeFlag = false;
    }

    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurfOpaq = thisSpace.OpaqOrIntMassSurfaceFirst;
            int const lastSurfOpaq = thisSpace.OpaqOrIntMassSurfaceLast;
            for (int SurfNum = firstSurfOpaq; SurfNum <= lastSurfOpaq; ++SurfNum) {
                // Loop through all (heat transfer) surfaces...  [ l11 ] = ( 1, 1, SurfNum ), [ l21 ] = ( 2, 1, SurfNum )
                auto const &surface = state.dataSurface->Surface(SurfNum);

                if ((surface.HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::CTF) &&
                    (surface.HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::EMPD)) {
                    continue;
                }

                int const ConstrNum = surface.Construction;
                auto const &construct = state.dataConstruction->Construct(ConstrNum);

                if (construct.NumCTFTerms == 0) {
                    continue; // Skip surfaces with no history terms
                }

                // Sign convention for the various terms in the following two equations
                // is based on the form of the Conduction Transfer Function equation
                // given by:
                // Qin,now  = (Sum of)(Y Tout) - (Sum of)(Z Tin) + (Sum of)(F Qin,old) + (Sum of)(V Qsrc)
                // Qout,now = (Sum of)(X Tout) - (Sum of)(Y Tin) + (Sum of)(F Qout,old) + (Sum of)(W Qsrc)
                // In both equations, flux is positive from outside to inside.  The V and W terms are for radiant systems only.

                // Set current inside flux:
                Real64 const SurfOutsideTempCurr = state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum);
                Real64 SurfInsideFluxHistCurr = SurfOutsideTempCurr * construct.CTFCross[0] -
                                                state.dataHeatBalSurf->SurfTempIn(SurfNum) * construct.CTFInside[0] +
                                                state.dataHeatBalSurf->SurfCTFConstInPart(SurfNum); // Heat source/sink term for radiant systems
                // Only HT opaq surfaces are evaluated, previous if (surface.Class == SurfaceClass::Floor || surface.Class == SurfaceClass::Wall ||
                // surface.Class == SurfaceClass::IntMass || surface.Class == SurfaceClass::Roof || surface.Class == SurfaceClass::Door) checks are
                // redundant.
                if (construct.SourceSinkPresent) {
                    SurfInsideFluxHistCurr += state.dataHeatBalSurf->SurfQsrcHist(SurfNum, 1) * construct.CTFSourceIn[0];
                }
                state.dataHeatBalSurf->SurfOpaqInsFaceCond(SurfNum) = surface.Area * SurfInsideFluxHistCurr;
                state.dataHeatBalSurf->SurfOpaqInsFaceCondFlux(SurfNum) = SurfInsideFluxHistCurr; // for reporting
                state.dataHeatBalSurf->SurfInsideFluxHist(1)(SurfNum) = SurfInsideFluxHistCurr;

                // Update the temperature at the source/sink location (if one is present)
                if (construct.SourceSinkPresent) {
                    state.dataHeatBalSurf->SurfTempSource(SurfNum) = state.dataHeatBalSurf->SurfTsrcHist(SurfNum, 1) =
                        SurfOutsideTempCurr * construct.CTFTSourceOut[0] + state.dataHeatBalSurf->SurfTempIn(SurfNum) * construct.CTFTSourceIn[0] +
                        state.dataHeatBalSurf->SurfQsrcHist(SurfNum, 1) * construct.CTFTSourceQ[0] +
                        state.dataHeatBalFanSys->CTFTsrcConstPart(SurfNum);
                    state.dataHeatBalSurf->SurfTempUserLoc(SurfNum) = state.dataHeatBalSurf->SurfTuserHist(SurfNum, 1) =
                        SurfOutsideTempCurr * construct.CTFTUserOut[0] + state.dataHeatBalSurf->SurfTempIn(SurfNum) * construct.CTFTUserIn[0] +
                        state.dataHeatBalSurf->SurfQsrcHist(SurfNum, 1) * construct.CTFTUserSource[0] +
                        state.dataHeatBalFanSys->CTFTuserConstPart(SurfNum);
                }

                // Set current outside flux:
                if (construct.SourceSinkPresent) {
                    state.dataHeatBalSurf->SurfOutsideFluxHist(1)(SurfNum) =
                        SurfOutsideTempCurr * construct.CTFOutside[0] - state.dataHeatBalSurf->SurfTempIn(SurfNum) * construct.CTFCross[0] +
                        state.dataHeatBalSurf->SurfQsrcHist(SurfNum, 1) * construct.CTFSourceOut[0] +
                        state.dataHeatBalSurf->SurfCTFConstOutPart(SurfNum); // Heat source/sink term for radiant systems
                } else {
                    state.dataHeatBalSurf->SurfOutsideFluxHist(1)(SurfNum) = SurfOutsideTempCurr * construct.CTFOutside[0] -
                                                                             state.dataHeatBalSurf->SurfTempIn(SurfNum) * construct.CTFCross[0] +
                                                                             state.dataHeatBalSurf->SurfCTFConstOutPart(SurfNum);
                }
                // switch sign for balance at outside face
                state.dataHeatBalSurf->SurfOpaqOutFaceCondFlux(SurfNum) = -state.dataHeatBalSurf->SurfOutsideFluxHist(1)(SurfNum);
                state.dataHeatBalSurf->SurfOpaqOutFaceCond(SurfNum) = surface.Area * state.dataHeatBalSurf->SurfOpaqOutFaceCondFlux(SurfNum);
            }
        }
    } // ...end of loop over all (heat transfer) surfaces...

    if (state.dataHeatBal->SimpleCTFOnly && !state.dataGlobal->AnyConstrOverridesInModel) {
        // Temporarily save the rvalue references of the last term arrays
        Array1D<Real64> insideTemp(std::move(state.dataHeatBalSurf->SurfInsideTempHist(state.dataHeatBal->MaxCTFTerms + 1)));
        Array1D<Real64> outsideTemp(std::move(state.dataHeatBalSurf->SurfOutsideTempHist(state.dataHeatBal->MaxCTFTerms + 1)));
        Array1D<Real64> insideFlux(std::move(state.dataHeatBalSurf->SurfInsideFluxHist(state.dataHeatBal->MaxCTFTerms + 1)));
        Array1D<Real64> outsideFlux(std::move(state.dataHeatBalSurf->SurfOutsideFluxHist(state.dataHeatBal->MaxCTFTerms + 1)));
        // Shifting its internal pointer to data to the new object; Using the (Array1D && a) overload of the "=" operator
        for (int HistTermNum = state.dataHeatBal->MaxCTFTerms + 1; HistTermNum >= 3; --HistTermNum) {
            state.dataHeatBalSurf->SurfInsideTempHist(HistTermNum) = std::move(state.dataHeatBalSurf->SurfInsideTempHist(HistTermNum - 1));
            state.dataHeatBalSurf->SurfOutsideTempHist(HistTermNum) = std::move(state.dataHeatBalSurf->SurfOutsideTempHist(HistTermNum - 1));
            state.dataHeatBalSurf->SurfInsideFluxHist(HistTermNum) = std::move(state.dataHeatBalSurf->SurfInsideFluxHist(HistTermNum - 1));
            state.dataHeatBalSurf->SurfOutsideFluxHist(HistTermNum) = std::move(state.dataHeatBalSurf->SurfOutsideFluxHist(HistTermNum - 1));
        }
        // Reuse the pointers of the last term arrays for the second term arrays
        state.dataHeatBalSurf->SurfInsideTempHist(2) = std::move(insideTemp);
        state.dataHeatBalSurf->SurfOutsideTempHist(2) = std::move(outsideTemp);
        state.dataHeatBalSurf->SurfInsideFluxHist(2) = std::move(insideFlux);
        state.dataHeatBalSurf->SurfOutsideFluxHist(2) = std::move(outsideFlux);
        // Hard copy the values of the the 1st term to the 2nd (copying data instead of pointers to protect the 1st term arrays used in run time)
        state.dataHeatBalSurf->SurfInsideTempHist(2) = state.dataHeatBalSurf->SurfInsideTempHist(1);
        state.dataHeatBalSurf->SurfOutsideTempHist(2) = state.dataHeatBalSurf->SurfOutsideTempHist(1);
        state.dataHeatBalSurf->SurfInsideFluxHist(2) = state.dataHeatBalSurf->SurfInsideFluxHist(1);
        state.dataHeatBalSurf->SurfOutsideFluxHist(2) = state.dataHeatBalSurf->SurfOutsideFluxHist(1);
        return;
    }

    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurfOpaq = thisSpace.OpaqOrIntMassSurfaceFirst;
            int const lastSurfOpaq = thisSpace.OpaqOrIntMassSurfaceLast;
            for (int SurfNum = firstSurfOpaq; SurfNum <= lastSurfOpaq; ++SurfNum) {
                auto const &surface = state.dataSurface->Surface(SurfNum);
                // Loop through all (heat transfer) surfaces...  [ l11 ] = ( 1, 1, SurfNum ), [ l21 ] = ( 2, 1, SurfNum )
                if ((surface.HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::CTF) &&
                    (surface.HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::EMPD)) {
                    continue;
                }
                if (state.dataHeatBalSurf->SurfCurrNumHist(SurfNum) == 0) { // First time step in a block for a surface, update arrays
                    state.dataHeatBalSurfMgr->TempExt1(SurfNum) = state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum);
                    state.dataHeatBalSurfMgr->TempInt1(SurfNum) = state.dataHeatBalSurf->SurfInsideTempHist(1)(SurfNum);
                    state.dataHeatBalSurfMgr->QExt1(SurfNum) = state.dataHeatBalSurf->SurfOutsideFluxHist(1)(SurfNum);
                    state.dataHeatBalSurfMgr->QInt1(SurfNum) = state.dataHeatBalSurf->SurfInsideFluxHist(1)(SurfNum);
                }
            }
        }

    } // ...end of loop over all (heat transfer) surfaces...
    if (state.dataHeatBal->AnyInternalHeatSourceInInput) {
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                int const firstSurfOpaq = thisSpace.OpaqOrIntMassSurfaceFirst;
                int const lastSurfOpaq = thisSpace.OpaqOrIntMassSurfaceLast;
                for (int SurfNum = firstSurfOpaq; SurfNum <= lastSurfOpaq; ++SurfNum) {
                    auto const &surface = state.dataSurface->Surface(SurfNum);
                    // Loop through all (heat transfer) surfaces...  [ l11 ] = ( 1, 1, SurfNum ), [ l21 ] = ( 2, 1, SurfNum )
                    if ((surface.HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::CTF) &&
                        (surface.HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::EMPD)) {
                        continue;
                    }
                    if (state.dataHeatBalSurf->SurfCurrNumHist(SurfNum) == 0) { // First time step in a block for a surface, update arrays
                        state.dataHeatBalSurfMgr->Tsrc1(SurfNum) = state.dataHeatBalSurf->SurfTsrcHist(SurfNum, 1);
                        state.dataHeatBalSurfMgr->Tuser1(SurfNum) = state.dataHeatBalSurf->SurfTuserHist(SurfNum, 1);
                        state.dataHeatBalSurfMgr->Qsrc1(SurfNum) = state.dataHeatBalSurf->SurfQsrcHist(SurfNum, 1);
                    }
                }
            }
        } // ...end of loop over all (heat transfer) surfaces...
    }

    // SHIFT TEMPERATURE AND FLUX HISTORIES:
    // SHIFT AIR TEMP AND FLUX SHIFT VALUES WHEN AT BOTTOM OF ARRAY SPACE.
    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurfOpaq = thisSpace.OpaqOrIntMassSurfaceFirst;
            int const lastSurfOpaq = thisSpace.OpaqOrIntMassSurfaceLast;
            for (int SurfNum = firstSurfOpaq; SurfNum <= lastSurfOpaq; ++SurfNum) {
                auto const &surface = state.dataSurface->Surface(SurfNum);

                if ((surface.HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::CTF) &&
                    (surface.HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::EMPD)) {
                    continue;
                }

                int const ConstrNum = surface.Construction;
                auto const &construct = state.dataConstruction->Construct(ConstrNum);

                ++state.dataHeatBalSurf->SurfCurrNumHist(SurfNum);
                state.dataHeatBalSurfMgr->SumTime(SurfNum) = double(state.dataHeatBalSurf->SurfCurrNumHist(SurfNum)) * state.dataGlobal->TimeStepZone;

                if (state.dataHeatBalSurf->SurfCurrNumHist(SurfNum) == construct.NumHistories) {
                    state.dataHeatBalSurf->SurfCurrNumHist(SurfNum) = 0;

                    if (construct.NumCTFTerms > 1) {
                        int const numCTFTerms(construct.NumCTFTerms);
                        for (int HistTermNum = numCTFTerms + 1; HistTermNum >= 3; --HistTermNum) { // Tuned Linear indexing
                            state.dataHeatBalSurf->SurfInsideTempHistMaster(HistTermNum)(SurfNum) =
                                state.dataHeatBalSurf->SurfInsideTempHistMaster(HistTermNum - 1)(SurfNum);
                            state.dataHeatBalSurf->SurfOutsideTempHistMaster(HistTermNum)(SurfNum) =
                                state.dataHeatBalSurf->SurfOutsideTempHistMaster(HistTermNum - 1)(SurfNum);
                            state.dataHeatBalSurf->SurfInsideFluxHistMaster(HistTermNum)(SurfNum) =
                                state.dataHeatBalSurf->SurfInsideFluxHistMaster(HistTermNum - 1)(SurfNum);
                            state.dataHeatBalSurf->SurfOutsideFluxHistMaster(HistTermNum)(SurfNum) =
                                state.dataHeatBalSurf->SurfOutsideFluxHistMaster(HistTermNum - 1)(SurfNum);
                            state.dataHeatBalSurf->SurfOutsideTempHist(HistTermNum)(SurfNum) =
                                state.dataHeatBalSurf->SurfOutsideTempHistMaster(HistTermNum - 1)(SurfNum);
                            state.dataHeatBalSurf->SurfInsideTempHist(HistTermNum)(SurfNum) =
                                state.dataHeatBalSurf->SurfInsideTempHistMaster(HistTermNum - 1)(SurfNum);
                            state.dataHeatBalSurf->SurfOutsideFluxHist(HistTermNum)(SurfNum) =
                                state.dataHeatBalSurf->SurfOutsideFluxHistMaster(HistTermNum - 1)(SurfNum);
                            state.dataHeatBalSurf->SurfInsideFluxHist(HistTermNum)(SurfNum) =
                                state.dataHeatBalSurf->SurfInsideFluxHistMaster(HistTermNum - 1)(SurfNum);
                        }
                    }

                    state.dataHeatBalSurf->SurfOutsideTempHistMaster(2)(SurfNum) = state.dataHeatBalSurfMgr->TempExt1(SurfNum);
                    state.dataHeatBalSurf->SurfInsideTempHistMaster(2)(SurfNum) = state.dataHeatBalSurfMgr->TempInt1(SurfNum);
                    state.dataHeatBalSurf->SurfOutsideFluxHistMaster(2)(SurfNum) = state.dataHeatBalSurfMgr->QExt1(SurfNum);
                    state.dataHeatBalSurf->SurfInsideFluxHistMaster(2)(SurfNum) = state.dataHeatBalSurfMgr->QInt1(SurfNum);

                    state.dataHeatBalSurf->SurfOutsideTempHist(2)(SurfNum) = state.dataHeatBalSurf->SurfOutsideTempHistMaster(2)(SurfNum);
                    state.dataHeatBalSurf->SurfInsideTempHist(2)(SurfNum) = state.dataHeatBalSurf->SurfInsideTempHistMaster(2)(SurfNum);
                    state.dataHeatBalSurf->SurfOutsideFluxHist(2)(SurfNum) = state.dataHeatBalSurf->SurfOutsideFluxHistMaster(2)(SurfNum);
                    state.dataHeatBalSurf->SurfInsideFluxHist(2)(SurfNum) = state.dataHeatBalSurf->SurfInsideFluxHistMaster(2)(SurfNum);
                } else {
                    Real64 const sum_steps(state.dataHeatBalSurfMgr->SumTime(SurfNum) / construct.CTFTimeStep);
                    if (construct.NumCTFTerms > 1) {
                        int const numCTFTerms(construct.NumCTFTerms);
                        for (int HistTermNum = numCTFTerms + 1; HistTermNum >= 3; --HistTermNum) { // Tuned Linear indexing
                            // TH(SideNum, TermNum, SurfNum) = (THM(SideNum, TermNum, SurfNum) -
                            //                                 (THM(SideNum, TermNum, SurfNum) - THM(SideNum, TermNum - 1, SurfNum)) * sum_steps;
                            Real64 const THM_Out_1(state.dataHeatBalSurf->SurfOutsideTempHistMaster(HistTermNum)(SurfNum));
                            Real64 const THM_In_1(state.dataHeatBalSurf->SurfInsideTempHistMaster(HistTermNum)(SurfNum));
                            Real64 const THM_Out_2(state.dataHeatBalSurf->SurfOutsideTempHistMaster(HistTermNum - 1)(SurfNum));
                            Real64 const THM_In_2(state.dataHeatBalSurf->SurfInsideTempHistMaster(HistTermNum - 1)(SurfNum));
                            state.dataHeatBalSurf->SurfOutsideTempHist(HistTermNum)(SurfNum) = THM_Out_1 - (THM_Out_1 - THM_Out_2) * sum_steps;
                            state.dataHeatBalSurf->SurfInsideTempHist(HistTermNum)(SurfNum) = THM_In_1 - (THM_In_1 - THM_In_2) * sum_steps;

                            Real64 const QHM_Out_1(state.dataHeatBalSurf->SurfOutsideFluxHistMaster(HistTermNum)(SurfNum));
                            Real64 const QHM_In_1(state.dataHeatBalSurf->SurfInsideFluxHistMaster(HistTermNum)(SurfNum));
                            Real64 const QHM_Out_2(state.dataHeatBalSurf->SurfOutsideFluxHistMaster(HistTermNum - 1)(SurfNum));
                            Real64 const QHM_In_2(state.dataHeatBalSurf->SurfInsideFluxHistMaster(HistTermNum - 1)(SurfNum));
                            state.dataHeatBalSurf->SurfOutsideFluxHist(HistTermNum)(SurfNum) = QHM_Out_1 - (QHM_Out_1 - QHM_Out_2) * sum_steps;
                            state.dataHeatBalSurf->SurfInsideFluxHist(HistTermNum)(SurfNum) = QHM_In_1 - (QHM_In_1 - QHM_In_2) * sum_steps;
                        }
                    }
                    // TH( 1, 2, SurfNum ) = THM( 1, 2, SurfNum ) - ( THM( 1, 2, SurfNum ) - TempExt1( SurfNum ) ) * sum_steps;
                    state.dataHeatBalSurf->SurfOutsideTempHist(2)(SurfNum) =
                        state.dataHeatBalSurf->SurfOutsideTempHistMaster(2)(SurfNum) -
                        (state.dataHeatBalSurf->SurfOutsideTempHistMaster(2)(SurfNum) - state.dataHeatBalSurfMgr->TempExt1(SurfNum)) * sum_steps;
                    state.dataHeatBalSurf->SurfInsideTempHist(2)(SurfNum) =
                        state.dataHeatBalSurf->SurfInsideTempHistMaster(2)(SurfNum) -
                        (state.dataHeatBalSurf->SurfInsideTempHistMaster(2)(SurfNum) - state.dataHeatBalSurfMgr->TempInt1(SurfNum)) * sum_steps;
                    state.dataHeatBalSurf->SurfOutsideFluxHist(2)(SurfNum) =
                        state.dataHeatBalSurf->SurfOutsideFluxHistMaster(2)(SurfNum) -
                        (state.dataHeatBalSurf->SurfOutsideFluxHistMaster(2)(SurfNum) - state.dataHeatBalSurfMgr->QExt1(SurfNum)) * sum_steps;
                    state.dataHeatBalSurf->SurfInsideFluxHist(2)(SurfNum) =
                        state.dataHeatBalSurf->SurfInsideFluxHistMaster(2)(SurfNum) -
                        (state.dataHeatBalSurf->SurfInsideFluxHistMaster(2)(SurfNum) - state.dataHeatBalSurfMgr->QInt1(SurfNum)) * sum_steps;
                }
            }
        }
    } // ...end of loop over all (heat transfer) surfaces

    if (state.dataHeatBal->AnyInternalHeatSourceInInput) {
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                int const firstSurfOpaq = thisSpace.OpaqOrIntMassSurfaceFirst;
                int const lastSurfOpaq = thisSpace.OpaqOrIntMassSurfaceLast;
                for (int SurfNum = firstSurfOpaq; SurfNum <= lastSurfOpaq; ++SurfNum) {
                    auto const &surface = state.dataSurface->Surface(SurfNum);
                    int const ConstrNum = surface.Construction;
                    auto const &construct = state.dataConstruction->Construct(ConstrNum);
                    if (!construct.SourceSinkPresent) {
                        continue;
                    }
                    if ((surface.HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::CTF) &&
                        (surface.HeatTransferAlgorithm != DataSurfaces::HeatTransferModel::EMPD)) {
                        continue;
                    }

                    if (state.dataHeatBalSurf->SurfCurrNumHist(SurfNum) == 0) { // First time step in a block for a surface, update arrays
                        if (construct.NumCTFTerms > 1) {
                            int const numCTFTerms = construct.NumCTFTerms;
                            int m = state.dataHeatBalSurf->SurfTsrcHistM.index(SurfNum, numCTFTerms);
                            int m1 = m + 1;
                            for (int HistTermNum = numCTFTerms + 1; HistTermNum >= 3; --HistTermNum, --m, --m1) { // Tuned Linear indexing
                                // SurfTsrcHist( SurfNum, HistTerm ) = SurfTsrcHistM( SurfNum, HHistTerm ) = SurfTsrcHistM( SurfNum, HistTermNum - 1
                                // ); SurfQsrcHist( SurfNum, HistTerm ) = SurfQsrcHistM( SurfNum, HHistTerm ) = SurfQsrcHistM( SurfNum, HistTermNum -
                                // 1 );
                                state.dataHeatBalSurf->SurfTsrcHist[m1] = state.dataHeatBalSurf->SurfTsrcHistM[m1] =
                                    state.dataHeatBalSurf->SurfTsrcHistM[m];
                                state.dataHeatBalSurf->SurfQsrcHist[m1] = state.dataHeatBalSurf->SurfQsrcHistM[m1] =
                                    state.dataHeatBalSurf->SurfQsrcHistM[m];
                                state.dataHeatBalSurf->SurfTuserHist[m1] = state.dataHeatBalSurf->SurfTuserHistM[m1] =
                                    state.dataHeatBalSurf->SurfTuserHistM[m];
                            }
                        }
                        state.dataHeatBalSurf->SurfTsrcHistM(SurfNum, 2) = state.dataHeatBalSurfMgr->Tsrc1(SurfNum);
                        state.dataHeatBalSurf->SurfTuserHistM(SurfNum, 2) = state.dataHeatBalSurfMgr->Tuser1(SurfNum);
                        state.dataHeatBalSurf->SurfQsrcHistM(SurfNum, 2) = state.dataHeatBalSurfMgr->Qsrc1(SurfNum);
                        state.dataHeatBalSurf->SurfTsrcHist(SurfNum, 2) = state.dataHeatBalSurf->SurfTsrcHistM(SurfNum, 2);
                        state.dataHeatBalSurf->SurfTuserHist(SurfNum, 2) = state.dataHeatBalSurf->SurfTuserHistM(SurfNum, 2);
                        state.dataHeatBalSurf->SurfQsrcHist(SurfNum, 2) = state.dataHeatBalSurf->SurfQsrcHistM(SurfNum, 2);
                    } else {
                        Real64 const sum_steps(state.dataHeatBalSurfMgr->SumTime(SurfNum) / construct.CTFTimeStep);

                        if (construct.NumCTFTerms > 1) {
                            int const numCTFTerms = construct.NumCTFTerms;
                            int m = state.dataHeatBalSurf->SurfTsrcHistM.index(SurfNum, numCTFTerms);
                            int m1 = m + 1;
                            for (int HistTermNum = numCTFTerms + 1; HistTermNum >= 3; --HistTermNum, --m, --m1) { // Tuned Linear indexing [ l ] == ()
                                // Real64 const SurfTsrcHistM_elem( SurfTsrcHistM( SurfNum, HistTermNum ) );
                                // SurfTsrcHist( SurfNum, HistTermNum ) = SurfTsrcHistM_elem - ( SurfTsrcHistM_elem - SurfTsrcHistM( SurfNum,
                                // HistTermNum
                                // - 1 ) ) * sum_steps;  Real64 const QsrcHistM_elem( SurfQsrcHistM( SurfNum, HistTermNum ) );  SurfQsrcHist( SurfNum,
                                // HistTermNum ) = QsrcHistM_elem - ( QsrcHistM_elem - SurfQsrcHistM( SurfNum, HistTermNum - 1 ) ) * sum_steps;
                                Real64 const TsrcHistM_m1(state.dataHeatBalSurf->SurfTsrcHistM[m1]);
                                state.dataHeatBalSurf->SurfTsrcHist[m1] =
                                    TsrcHistM_m1 - (TsrcHistM_m1 - state.dataHeatBalSurf->SurfTsrcHistM[m]) * sum_steps;
                                Real64 const QsrcHistM_m1(state.dataHeatBalSurf->SurfQsrcHistM[m1]);
                                state.dataHeatBalSurf->SurfQsrcHist[m1] =
                                    QsrcHistM_m1 - (QsrcHistM_m1 - state.dataHeatBalSurf->SurfQsrcHistM[m]) * sum_steps;
                                Real64 const TuserHistM_m1(state.dataHeatBalSurf->SurfTuserHistM[m1]);
                                state.dataHeatBalSurf->SurfTuserHist[m1] =
                                    TuserHistM_m1 - (TuserHistM_m1 - state.dataHeatBalSurf->SurfTuserHistM[m]) * sum_steps;
                            }
                        }
                        // Tuned Linear indexing
                        // SurfTsrcHist( SurfNum, 2 ) = SurfTsrcHistM( SurfNum, 2 ) - ( SurfTsrcHistM( SurfNum, 2 ) - Tsrc1( SurfNum ) ) * sum_steps;
                        // SurfQsrcHist( SurfNum, 2 ) = SurfQsrcHistM( SurfNum, 2 ) - ( SurfQsrcHistM( SurfNum, 2 ) - Qsrc1( SurfNum ) ) * sum_steps;
                        int const l2 = state.dataHeatBalSurf->SurfTsrcHist.index(SurfNum, 2);
                        state.dataHeatBalSurf->SurfTsrcHist[l2] =
                            state.dataHeatBalSurf->SurfTsrcHistM[l2] -
                            (state.dataHeatBalSurf->SurfTsrcHistM[l2] - state.dataHeatBalSurfMgr->Tsrc1(SurfNum)) * sum_steps;
                        state.dataHeatBalSurf->SurfQsrcHist[l2] =
                            state.dataHeatBalSurf->SurfQsrcHistM[l2] -
                            (state.dataHeatBalSurf->SurfQsrcHistM[l2] - state.dataHeatBalSurfMgr->Qsrc1(SurfNum)) * sum_steps;
                        state.dataHeatBalSurf->SurfTuserHist[l2] =
                            state.dataHeatBalSurf->SurfTuserHistM[l2] -
                            (state.dataHeatBalSurf->SurfTuserHistM[l2] - state.dataHeatBalSurfMgr->Tuser1(SurfNum)) * sum_steps;
                    }
                }
            }
        } // ...end of loop over all (heat transfer) surfaces...
    } // ...end of AnyInternalHeatSourceInInput
}

void CalculateZoneMRT(EnergyPlusData &state,
                      ObjexxFCL::Optional_int_const ZoneToResimulate) // if passed in, then only calculate surfaces that have this zone
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   November 2000

    // PURPOSE OF THIS SUBROUTINE:
    // Calculates the current zone and enclosure MRT for thermal comfort and radiation
    // calculation purposes.

    if (state.dataHeatBalSurfMgr->CalculateZoneMRTfirstTime) {
        state.dataHeatBalSurfMgr->SurfaceAE.allocate(state.dataSurface->TotSurfaces);
        state.dataHeatBalSurfMgr->ZoneAESum.allocate(state.dataGlobal->NumOfZones);
        state.dataHeatBalSurfMgr->SurfaceAE = 0.0;
        state.dataHeatBalSurfMgr->ZoneAESum = 0.0;
        for (auto &encl : state.dataViewFactor->EnclRadInfo) {
            encl.sumAE = 0.0;
        }
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            auto const &surface = state.dataSurface->Surface(SurfNum);
            if (surface.HeatTransSurf) {
                auto &thisSurfAE = state.dataHeatBalSurfMgr->SurfaceAE(SurfNum);
                thisSurfAE = surface.Area * state.dataConstruction->Construct(surface.Construction).InsideAbsorpThermal;
                int ZoneNum = surface.Zone;
                if (ZoneNum > 0) {
                    state.dataHeatBalSurfMgr->ZoneAESum(ZoneNum) += thisSurfAE;
                }
                if (surface.RadEnclIndex > 0) {
                    state.dataViewFactor->EnclRadInfo(surface.RadEnclIndex).sumAE += thisSurfAE;
                }
            }
        }
    }

    // Zero sumAET for applicable enclosures
    if (present(ZoneToResimulate)) {
        for (int spaceNum : state.dataHeatBal->Zone(ZoneToResimulate).spaceIndexes) {
            int enclNum = state.dataHeatBal->space(spaceNum).radiantEnclosureNum;
            state.dataViewFactor->EnclRadInfo(enclNum).sumAET = 0.0;
            state.dataViewFactor->EnclRadInfo(enclNum).reCalcMRT = true;
        }
    } else {
        for (auto &thisEnclosure : state.dataViewFactor->EnclRadInfo) {
            thisEnclosure.reCalcMRT = true;
        }
    }
    for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
        if (present(ZoneToResimulate) && (ZoneNum != ZoneToResimulate)) {
            continue;
        }
        auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);
        if (state.dataHeatBalSurfMgr->ZoneAESum(ZoneNum) > 0.01) {
            Real64 zoneSumAET = 0.0;
            for (int spaceNum : state.dataHeatBal->Zone(ZoneNum).spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {
                    Real64 surfAET = state.dataHeatBalSurfMgr->SurfaceAE(SurfNum) * state.dataHeatBalSurf->SurfTempIn(SurfNum);
                    zoneSumAET += surfAET;
                    state.dataViewFactor->EnclRadInfo(state.dataSurface->Surface(SurfNum).RadEnclIndex).sumAET += surfAET;
                }
            }
            thisZoneHB.MRT = zoneSumAET / state.dataHeatBalSurfMgr->ZoneAESum(ZoneNum);
        } else {
            if (state.dataHeatBalSurfMgr->CalculateZoneMRTfirstTime) {
                ShowWarningError(
                    state,
                    format("Zone areas*inside surface emissivities are summing to zero, for Zone=\"{}\"", state.dataHeatBal->Zone(ZoneNum).Name));
                ShowContinueError(state, "As a result, MRT will be set to MAT for that zone");
            }
            thisZoneHB.MRT = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).MAT;
        }
    }
    // Calculate MRT for applicable enclosures
    for (auto &thisEnclosure : state.dataViewFactor->EnclRadInfo) {
        if (!thisEnclosure.reCalcMRT) {
            continue;
        }
        if (thisEnclosure.sumAE > 0.01) {
            thisEnclosure.sumAET = 0.0;
            for (int surfNum : thisEnclosure.SurfacePtr) {
                Real64 surfAET = state.dataHeatBalSurfMgr->SurfaceAE(surfNum) * state.dataHeatBalSurf->SurfTempIn(surfNum);
                thisEnclosure.sumAET += surfAET;
            }
            thisEnclosure.MRT = thisEnclosure.sumAET / thisEnclosure.sumAE;
        } else {
            if (state.dataHeatBalSurfMgr->CalculateZoneMRTfirstTime) {
                ShowWarningError(state,
                                 format("Enclosure areas*inside surface emissivities are summing to zero, for Enclosure=\"{}\"", thisEnclosure.Name));
                ShowContinueError(state, "As a result, MRT will be set to the volume weighted average MAT for that enclosure");
            }
            Real64 sumMATVol = 0.0;
            Real64 sumVol = 0.0;
            Real64 sumMAT = 0.0;
            for (auto &spaceNum : thisEnclosure.spaceNums) {
                Real64 spaceVolume = state.dataHeatBal->space(spaceNum).Volume;
                Real64 spaceMAT = state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum).MAT;
                sumVol += spaceVolume;
                sumMATVol += spaceMAT * spaceVolume;
                sumMAT += spaceMAT;
            }
            if (sumVol > 0.01) {
                thisEnclosure.MRT = sumMATVol / sumVol;
            } else {
                thisEnclosure.MRT = sumMAT / (int)thisEnclosure.spaceNums.size();
            }
        }
        // Set space MRTs
        for (int spaceNum : thisEnclosure.spaceNums) {
            state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum).MRT = thisEnclosure.MRT;
        }
    }

    state.dataHeatBalSurfMgr->CalculateZoneMRTfirstTime = false;
}

// End of Record Keeping subroutines for the HB Module
// *****************************************************************************

// Beginning of Reporting subroutines for the HB Module
// *****************************************************************************

void CalcThermalResilience(EnergyPlusData &state)
{
    // This function calculate timestep-wise heat index and humidex.

    // The computation of the heat index is a refinement of a result obtained by multiple regression analysis
    // carried out by Lans P. Rothfusz and described in a 1990 National Weather Service (NWS)
    // Technical Attachment (SR 90-23).
    // Reference: https://www.wpc.ncep.noaa.gov/html/heatindex_equation.shtml

    // The current formula for determining the humidex was developed by J. M. Masterton and F. A. Richardson of
    // Canada's Atmospheric Environment Service in 1979.
    // Reference: Masterson, J., and F. Richardson, 1979: Humidex, a method of quantifying human
    // discomfort due to excessive heat and humidity CLI 1-79, Environment Canada, Atmospheric Environment Service
    //        using OutputProcessor::ReqRepVars;
    if (state.dataHeatBalSurfMgr->ManageSurfaceHeatBalancefirstTime) {
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            SetupOutputVariable(state,
                                "Zone Heat Index",
                                Constant::Units::C,
                                state.dataHeatBal->Resilience(ZoneNum).ZoneHeatIndex,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(ZoneNum).Name);
            SetupOutputVariable(state,
                                "Zone Humidity Index",
                                Constant::Units::None,
                                state.dataHeatBal->Resilience(ZoneNum).ZoneHumidex,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Average,
                                state.dataHeatBal->Zone(ZoneNum).Name);
        }
        for (auto const *reqVar : state.dataOutputProcessor->reqVars) {
            if (reqVar->name == "Zone Heat Index") {
                state.dataHeatBalSurfMgr->reportVarHeatIndex = true;
            } else if (reqVar->name == "Zone Humidity Index") {
                state.dataHeatBalSurfMgr->reportVarHumidex = true;
            }
        }
    }

    // Calculate Heat Index and Humidex.
    // The heat index equation set is fit to Fahrenheit units, so the zone air temperature values are first convert to F,
    // then heat index is calculated and converted back to C.
    if (state.dataHeatBalSurfMgr->reportVarHeatIndex || state.dataOutRptTab->displayThermalResilienceSummary) {
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            Real64 const ZoneT = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).ZTAV;
            Real64 const ZoneW = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).airHumRatAvg;
            Real64 const ZoneRH = Psychrometrics::PsyRhFnTdbWPb(state, ZoneT, ZoneW, state.dataEnvrn->OutBaroPress) * 100.0;
            Real64 const ZoneTF = ZoneT * (9.0 / 5.0) + 32.0;
            if (state.dataHeatBal->heatIndexMethod == DataHeatBalance::HeatIndexMethod::Simplified) {
                Real64 constexpr c1 = -42.379;
                Real64 constexpr c2 = 2.04901523;
                Real64 constexpr c3 = 10.14333127;
                Real64 constexpr c4 = -.22475541;
                Real64 constexpr c5 = -.00683783;
                Real64 constexpr c6 = -.05481717;
                Real64 constexpr c7 = .00122874;
                Real64 constexpr c8 = .00085282;
                Real64 constexpr c9 = -.00000199;
                Real64 HI;

                if (ZoneTF < 80) {
                    HI = 0.5 * (ZoneTF + 61.0 + (ZoneTF - 68.0) * 1.2 + (ZoneRH * 0.094));
                } else {
                    HI = c1 + c2 * ZoneTF + c3 * ZoneRH + c4 * ZoneTF * ZoneRH + c5 * ZoneTF * ZoneTF + c6 * ZoneRH * ZoneRH +
                         c7 * ZoneTF * ZoneTF * ZoneRH + c8 * ZoneTF * ZoneRH * ZoneRH + c9 * ZoneTF * ZoneTF * ZoneRH * ZoneRH;
                    if (ZoneRH < 13 && ZoneTF < 112) {
                        HI -= (13 - ZoneRH) / 4 * std::sqrt((17 - abs(ZoneTF - 95)) / 17);
                    } else if (ZoneRH > 85 && ZoneTF < 87) {
                        HI += (ZoneRH - 85) / 10 * (87 - ZoneTF) / 5;
                    }
                }
                HI = (HI - 32.0) * (5.0 / 9.0);
                state.dataHeatBal->Resilience(ZoneNum).ZoneHeatIndex = HI;
            } else {
                // calculate extended heat index
                state.dataHeatBal->Resilience(ZoneNum).ZoneHeatIndex =
                    ExtendedHI::heatindex(state, ZoneT + Constant::Kelvin, ZoneRH / 100.0) - Constant::Kelvin;
            }
        }
    }
    if (state.dataHeatBalSurfMgr->reportVarHumidex || state.dataOutRptTab->displayThermalResilienceSummary) {
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            Real64 const ZoneW = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).airHumRatAvg;
            Real64 const ZoneT = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).ZTAV;
            Real64 const TDewPointK = Psychrometrics::PsyTdpFnWPb(state, ZoneW, state.dataEnvrn->OutBaroPress) + Constant::Kelvin;
            Real64 const e = 6.11 * std::exp(5417.7530 * ((1 / 273.16) - (1 / TDewPointK)));
            Real64 const h = 5.0 / 9.0 * (e - 10.0);
            Real64 const Humidex = ZoneT + h;
            state.dataHeatBal->Resilience(ZoneNum).ZoneHumidex = Humidex;
        }
    }
}

void ReportThermalResilience(EnergyPlusData &state)
{

    Array1D_bool reportPeriodFlags;
    if (state.dataWeather->TotReportPers > 0) {
        reportPeriodFlags.dimension(state.dataWeather->TotThermalReportPers, false);
        General::findReportPeriodIdx(state, state.dataWeather->ThermalReportPeriodInput, state.dataWeather->TotThermalReportPers, reportPeriodFlags);
    }

    auto &ort = state.dataOutRptTab;
    for (int i = 1; i <= state.dataWeather->TotThermalReportPers; i++) {
        if (reportPeriodFlags(i)) {
            int curResMeterNumber = ort->meterNumTotalsBEPS(1);
            state.dataWeather->ThermalReportPeriodInput(i).totalElectricityUse += GetCurrentMeterValue(state, curResMeterNumber);
        }
    }

    if (state.dataHeatBalSurfMgr->reportThermalResilienceFirstTime) {
        int constexpr HINoBins = 5;                     // Heat Index range - number of bins
        int constexpr HumidexNoBins = 5;                // Humidex range - number of bins
        int constexpr SETNoBins = 5;                    // SET report column numbers
        int constexpr ColdHourOfSafetyNoBins = 5;       // Cold Stress Hour of Safety number of columns
        int constexpr HeatHourOfSafetyNoBins = 5;       // Heat Stress Hour of Safety number of columns
        int constexpr UnmetDegreeHourNoBins = 6;        // Unmet Degree Hour number of columns
        int constexpr DiscomfortWtExceedHourNoBins = 4; // Unmet Degree Hour number of columns

        if (state.dataHeatBal->TotPeople == 0) {
            state.dataHeatBalSurfMgr->hasPierceSET = false;
        }
        for (int iPeople = 1; iPeople <= state.dataHeatBal->TotPeople; ++iPeople) {
            if (!state.dataHeatBal->People(iPeople).Pierce) {
                state.dataHeatBalSurfMgr->hasPierceSET = false;
            }
        }
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            // the whole period
            // user specified reporting period
            for (int i = 1; i <= state.dataWeather->TotThermalReportPers; i++) {
                state.dataHeatBalFanSys->ZoneHeatIndexHourBinsRepPeriod(ZoneNum, i).assign(HINoBins, 0.0);
                state.dataHeatBalFanSys->ZoneHeatIndexOccuHourBinsRepPeriod(ZoneNum, i).assign(HINoBins, 0.0);
                state.dataHeatBalFanSys->ZoneHeatIndexOccupiedHourBinsRepPeriod(ZoneNum, i).assign(HINoBins, 0.0);
                state.dataHeatBalFanSys->ZoneHumidexHourBinsRepPeriod(ZoneNum, i).assign(HumidexNoBins, 0.0);
                state.dataHeatBalFanSys->ZoneHumidexOccupiedHourBinsRepPeriod(ZoneNum, i).assign(HumidexNoBins, 0.0);
                state.dataHeatBalFanSys->ZoneHumidexOccuHourBinsRepPeriod(ZoneNum, i).assign(HumidexNoBins, 0.0);
                if (state.dataHeatBalSurfMgr->hasPierceSET) {
                    state.dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(ZoneNum, i).assign(SETNoBins, 0.0);
                    state.dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(ZoneNum, i).assign(SETNoBins, 0.0);
                }
                state.dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(ZoneNum, i).assign(ColdHourOfSafetyNoBins, 0.0);
                state.dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(ZoneNum, i).assign(HeatHourOfSafetyNoBins, 0.0);
                state.dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(ZoneNum, i).assign(UnmetDegreeHourNoBins, 0.0);
                state.dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(ZoneNum, i).assign(DiscomfortWtExceedHourNoBins, 0.0);
                state.dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(ZoneNum, i).assign(DiscomfortWtExceedHourNoBins, 0.0);
            }
            state.dataHeatBalFanSys->lowSETLongestHoursRepPeriod = 0.0;
            state.dataHeatBalFanSys->highSETLongestHoursRepPeriod = 0.0;
            state.dataHeatBalFanSys->lowSETLongestStartRepPeriod = 0.0;
            state.dataHeatBalFanSys->highSETLongestStartRepPeriod = 0.0;
        }
        state.dataHeatBalSurfMgr->lowSETLongestHours.assign(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalSurfMgr->highSETLongestHours.assign(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalSurfMgr->lowSETLongestStart.assign(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalSurfMgr->highSETLongestStart.assign(state.dataGlobal->NumOfZones, 0.0);
        state.dataHeatBalSurfMgr->reportThermalResilienceFirstTime = false;
    }

    // Count hours only during weather simulation periods
    if (Constant::KindOfSim::RunPeriodWeather == state.dataGlobal->KindOfSim && !state.dataGlobal->WarmupFlag) {
        // use default value if there are no user inputs
        Real64 ColdTempThresh = 15.56;
        Real64 HeatTempThresh = 30.0;
        // Trace current time step Zone Pierce SET; NaN if no occupant or SET not calculated
        // Record last time step SET to trace SET unmet duration;

        Real64 valueNotInit = -999.0;
        Real64 nearThreshold = 1.0;
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            state.dataHeatBal->Resilience(ZoneNum).PierceSET = valueNotInit;
            state.dataHeatBal->Resilience(ZoneNum).PMV = valueNotInit;
        }
        for (int iPeople = 1; iPeople <= state.dataHeatBal->TotPeople; ++iPeople) {
            int ZoneNum = state.dataHeatBal->People(iPeople).ZonePtr;
            state.dataHeatBal->Resilience(ZoneNum).ZoneNumOcc =
                state.dataHeatBal->People(iPeople).NumberOfPeople * state.dataHeatBal->People(iPeople).sched->getCurrentVal();
            state.dataHeatBal->Resilience(ZoneNum).ZonePierceSETLastStep = state.dataHeatBal->Resilience(ZoneNum).ZonePierceSET;
            if (state.dataHeatBal->People(iPeople).Pierce) {
                state.dataHeatBal->Resilience(ZoneNum).ZonePierceSET = state.dataThermalComforts->ThermalComfortData(iPeople).PierceSET;
            } else {
                state.dataHeatBal->Resilience(ZoneNum).ZonePierceSET = -1;
            }

            Real64 NumOcc = state.dataHeatBal->Resilience(ZoneNum).ZoneNumOcc;
            Real64 Temperature = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).ZTAV;
            ColdTempThresh = state.dataHeatBal->People(iPeople).ColdStressTempThresh;
            bool &CrossedColdThresh = state.dataHeatBal->Resilience(ZoneNum).CrossedColdThresh;
            if (Temperature > ColdTempThresh) { // safe
                if (!CrossedColdThresh) {
                    // compute the number of hours before threshold is reached
                    state.dataHeatBal->Resilience(ZoneNum).ZoneColdHourOfSafetyBins[0] += state.dataGlobal->TimeStepZone;
                }
            } else { // danger
                // compute the total number of hours when the zone temperature falls in the dangerous range throughout the reporting period
                state.dataHeatBal->Resilience(ZoneNum).ZoneColdHourOfSafetyBins[2] += state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneColdHourOfSafetyBins[3] += NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneColdHourOfSafetyBins[4] += (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                // first time crossing threshold
                if (!CrossedColdThresh) {
                    // compute the time when the zone crosses the threshold temperature
                    int encodedMonDayHrMin;
                    General::EncodeMonDayHrMin(encodedMonDayHrMin,
                                               state.dataEnvrn->Month,
                                               state.dataEnvrn->DayOfMonth,
                                               state.dataGlobal->HourOfDay,
                                               state.dataGlobal->TimeStepZone * (state.dataGlobal->TimeStep - 1) * 60);
                    // fixme: not sure how to aggregate by zone
                    state.dataHeatBal->Resilience(ZoneNum).ZoneColdHourOfSafetyBins[1] = encodedMonDayHrMin;
                    CrossedColdThresh = true;
                }
            }
            HeatTempThresh = state.dataHeatBal->People(iPeople).HeatStressTempThresh;
            bool &CrossedHeatThresh = state.dataHeatBal->Resilience(ZoneNum).CrossedHeatThresh;
            if (Temperature < HeatTempThresh) { // safe
                if (!CrossedHeatThresh) {
                    // compute the number of hours before threshold is reached
                    state.dataHeatBal->Resilience(ZoneNum).ZoneHeatHourOfSafetyBins[0] += state.dataGlobal->TimeStepZone;
                }
            } else { // danger
                // compute the total number of hours when the zone temperature falls in the dangerous range throughout the reporting period
                state.dataHeatBal->Resilience(ZoneNum).ZoneHeatHourOfSafetyBins[2] += state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneHeatHourOfSafetyBins[3] += NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneHeatHourOfSafetyBins[4] += (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                // first time crossing threshold
                if (!CrossedHeatThresh) {
                    // compute the time when the zone crosses the threshold temperature
                    int encodedMonDayHrMin;
                    General::EncodeMonDayHrMin(encodedMonDayHrMin,
                                               state.dataEnvrn->Month,
                                               state.dataEnvrn->DayOfMonth,
                                               state.dataGlobal->HourOfDay,
                                               state.dataGlobal->TimeStepZone * (state.dataGlobal->TimeStep - 1) * 60);
                    state.dataHeatBal->Resilience(ZoneNum).ZoneHeatHourOfSafetyBins[1] = encodedMonDayHrMin;
                    CrossedHeatThresh = true;
                }
            }

            Real64 VeryHotPMVThresh = 3.0;
            Real64 WarmPMVThresh = 0.7;
            Real64 CoolPMVThresh = -0.7;
            Real64 VeryColdPMVThresh = -3.0;
            Real64 PMV = state.dataThermalComforts->ThermalComfortData(iPeople).FangerPMV;
            if (PMV < VeryColdPMVThresh) {
                state.dataHeatBal->Resilience(ZoneNum).ZoneDiscomfortWtExceedOccuHourBins[0] +=
                    (VeryColdPMVThresh - PMV) * NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneDiscomfortWtExceedOccupiedHourBins[0] +=
                    (VeryColdPMVThresh - PMV) * (NumOcc > 0) * state.dataGlobal->TimeStepZone;
            }
            if (PMV < CoolPMVThresh) {
                state.dataHeatBal->Resilience(ZoneNum).ZoneDiscomfortWtExceedOccuHourBins[1] +=
                    (CoolPMVThresh - PMV) * NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneDiscomfortWtExceedOccupiedHourBins[1] +=
                    (CoolPMVThresh - PMV) * (NumOcc > 0) * state.dataGlobal->TimeStepZone;
            }
            if (PMV > WarmPMVThresh) {
                state.dataHeatBal->Resilience(ZoneNum).ZoneDiscomfortWtExceedOccuHourBins[2] +=
                    (PMV - WarmPMVThresh) * NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneDiscomfortWtExceedOccupiedHourBins[2] +=
                    (PMV - WarmPMVThresh) * (NumOcc > 0) * state.dataGlobal->TimeStepZone;
            }
            if (PMV > VeryHotPMVThresh) {
                state.dataHeatBal->Resilience(ZoneNum).ZoneDiscomfortWtExceedOccuHourBins[3] +=
                    (PMV - VeryHotPMVThresh) * NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneDiscomfortWtExceedOccupiedHourBins[3] +=
                    (PMV - VeryHotPMVThresh) * (NumOcc > 0) * state.dataGlobal->TimeStepZone;
            }

            // check whether PierceSET changed for people in a zone
            if (state.dataHeatBal->Resilience(ZoneNum).PierceSET < valueNotInit + nearThreshold) {
                state.dataHeatBal->Resilience(ZoneNum).PierceSET = state.dataHeatBal->Resilience(ZoneNum).ZonePierceSET;
            } else {
                if (state.dataHeatBal->Resilience(ZoneNum).PierceSET != state.dataHeatBal->Resilience(ZoneNum).ZonePierceSET) {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   fmt::format("Zone {} has multiple people objects with different PierceSet.", ZoneNum),
                                                   state.dataHeatBalFanSys->PierceSETerrorIndex);
                }
            }

            // check whether PierceSET, PMV, etc. changed for different people in a zone
            if (state.dataHeatBal->Resilience(ZoneNum).PMV < valueNotInit + nearThreshold) {
                state.dataHeatBal->Resilience(ZoneNum).PMV = PMV;
            } else {
                if (state.dataHeatBal->Resilience(ZoneNum).PMV != PMV) {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   fmt::format("Zone {} has multiple people objects with different PMV.", ZoneNum),
                                                   state.dataHeatBalFanSys->PMVerrorIndex);
                }
            }

            for (int i = 1; i <= state.dataWeather->TotThermalReportPers; i++) {
                if (reportPeriodFlags(i)) {
                    int ReportPeriodIdx = i;
                    ColdTempThresh = state.dataHeatBal->People(iPeople).ColdStressTempThresh;
                    bool &CrossedColdThreshRepPeriod = state.dataHeatBalFanSys->CrossedColdThreshRepPeriod(ZoneNum, ReportPeriodIdx);
                    if (Temperature > ColdTempThresh) { // safe
                        if (!CrossedColdThreshRepPeriod) {
                            // compute the number of hours before threshold is reached
                            state.dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(ZoneNum, ReportPeriodIdx)[0] += state.dataGlobal->TimeStepZone;
                        }
                    } else { // danger
                        // compute the total number of hours when the zone temperature falls in the dangerous range throughout the reporting period
                        state.dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(ZoneNum, ReportPeriodIdx)[2] += state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(ZoneNum, ReportPeriodIdx)[3] +=
                            NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(ZoneNum, ReportPeriodIdx)[4] +=
                            (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                        // first time crossing threshold
                        if (!CrossedColdThreshRepPeriod) {
                            // compute the time when the zone crosses the threshold temperature
                            int encodedMonDayHrMin;
                            General::EncodeMonDayHrMin(encodedMonDayHrMin,
                                                       state.dataEnvrn->Month,
                                                       state.dataEnvrn->DayOfMonth,
                                                       state.dataGlobal->HourOfDay,
                                                       state.dataGlobal->TimeStepZone * (state.dataGlobal->TimeStep - 1) * 60);
                            // fixme: not sure how to aggregate by zone
                            state.dataHeatBalFanSys->ZoneColdHourOfSafetyBinsRepPeriod(ZoneNum, ReportPeriodIdx)[1] = encodedMonDayHrMin;
                            CrossedColdThreshRepPeriod = true;
                        }
                    }
                    HeatTempThresh = state.dataHeatBal->People(iPeople).HeatStressTempThresh;
                    bool &CrossedHeatThreshRepPeriod = state.dataHeatBalFanSys->CrossedHeatThreshRepPeriod(ZoneNum, ReportPeriodIdx);
                    if (Temperature < HeatTempThresh) { // safe
                        if (!CrossedHeatThreshRepPeriod) {
                            // compute the number of hours before threshold is reached
                            state.dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(ZoneNum, ReportPeriodIdx)[0] += state.dataGlobal->TimeStepZone;
                        }
                    } else { // danger
                        // compute the total number of hours when the zone temperature falls in the dangerous range throughout the reporting period
                        state.dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(ZoneNum, ReportPeriodIdx)[2] += state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(ZoneNum, ReportPeriodIdx)[3] +=
                            NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(ZoneNum, ReportPeriodIdx)[4] +=
                            (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                        // first time crossing threshold
                        if (!CrossedHeatThreshRepPeriod) {
                            // compute the time when the zone crosses the threshold temperature
                            int encodedMonDayHrMin;
                            General::EncodeMonDayHrMin(encodedMonDayHrMin,
                                                       state.dataEnvrn->Month,
                                                       state.dataEnvrn->DayOfMonth,
                                                       state.dataGlobal->HourOfDay,
                                                       state.dataGlobal->TimeStepZone * (state.dataGlobal->TimeStep - 1) * 60);
                            state.dataHeatBalFanSys->ZoneHeatHourOfSafetyBinsRepPeriod(ZoneNum, ReportPeriodIdx)[1] = encodedMonDayHrMin;
                            CrossedHeatThreshRepPeriod = true;
                        }
                    }

                    if (PMV < VeryColdPMVThresh) {
                        state.dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[0] +=
                            (VeryColdPMVThresh - PMV) * NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[0] +=
                            (VeryColdPMVThresh - PMV) * (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    }
                    if (PMV < CoolPMVThresh) {
                        state.dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[1] +=
                            (CoolPMVThresh - PMV) * NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[1] +=
                            (CoolPMVThresh - PMV) * (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    }
                    if (PMV > WarmPMVThresh) {
                        state.dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[2] +=
                            (PMV - WarmPMVThresh) * NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[2] +=
                            (PMV - WarmPMVThresh) * (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    }
                    if (PMV > VeryHotPMVThresh) {
                        state.dataHeatBalFanSys->ZoneDiscomfortWtExceedOccuHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[3] +=
                            (PMV - VeryHotPMVThresh) * NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneDiscomfortWtExceedOccupiedHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[3] +=
                            (PMV - VeryHotPMVThresh) * (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    }
                }
            }
        }
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            Real64 HI = state.dataHeatBal->Resilience(ZoneNum).ZoneHeatIndex;
            Real64 Humidex = state.dataHeatBal->Resilience(ZoneNum).ZoneHumidex;

            Real64 NumOcc = state.dataHeatBal->Resilience(ZoneNum).ZoneNumOcc;
            if (HI <= 26.7) {
                state.dataHeatBal->Resilience(ZoneNum).ZoneHeatIndexHourBins[0] += state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneHeatIndexOccuHourBins[0] += NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneHeatIndexOccupiedHourBins[0] += (NumOcc > 0) * state.dataGlobal->TimeStepZone;
            } else if (HI > 26.7 && HI <= 32.2) {
                state.dataHeatBal->Resilience(ZoneNum).ZoneHeatIndexHourBins[1] += state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneHeatIndexOccuHourBins[1] += NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneHeatIndexOccupiedHourBins[1] += (NumOcc > 0) * state.dataGlobal->TimeStepZone;
            } else if (HI > 32.2 && HI <= 39.4) {
                state.dataHeatBal->Resilience(ZoneNum).ZoneHeatIndexHourBins[2] += state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneHeatIndexOccuHourBins[2] += NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneHeatIndexOccupiedHourBins[2] += (NumOcc > 0) * state.dataGlobal->TimeStepZone;
            } else if (HI > 39.4 && HI <= 51.7) {
                state.dataHeatBal->Resilience(ZoneNum).ZoneHeatIndexHourBins[3] += state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneHeatIndexOccuHourBins[3] += NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneHeatIndexOccupiedHourBins[3] += (NumOcc > 0) * state.dataGlobal->TimeStepZone;
            } else {
                state.dataHeatBal->Resilience(ZoneNum).ZoneHeatIndexHourBins[4] += state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneHeatIndexOccuHourBins[4] += NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneHeatIndexOccupiedHourBins[4] += (NumOcc > 0) * state.dataGlobal->TimeStepZone;
            }

            if (Humidex <= 29) {
                state.dataHeatBal->Resilience(ZoneNum).ZoneHumidexHourBins[0] += state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneHumidexOccuHourBins[0] += NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneHumidexOccupiedHourBins[0] += (NumOcc > 0) * state.dataGlobal->TimeStepZone;
            } else if (Humidex > 29 && Humidex <= 40) {
                state.dataHeatBal->Resilience(ZoneNum).ZoneHumidexHourBins[1] += state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneHumidexOccuHourBins[1] += NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneHumidexOccupiedHourBins[1] += (NumOcc > 0) * state.dataGlobal->TimeStepZone;
            } else if (Humidex > 40 && Humidex <= 45) {
                state.dataHeatBal->Resilience(ZoneNum).ZoneHumidexHourBins[2] += state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneHumidexOccuHourBins[2] += NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneHumidexOccupiedHourBins[2] += (NumOcc > 0) * state.dataGlobal->TimeStepZone;
            } else if (Humidex > 45 && Humidex <= 50) {
                state.dataHeatBal->Resilience(ZoneNum).ZoneHumidexHourBins[3] += state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneHumidexOccuHourBins[3] += NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneHumidexOccupiedHourBins[3] += (NumOcc > 0) * state.dataGlobal->TimeStepZone;
            } else {
                state.dataHeatBal->Resilience(ZoneNum).ZoneHumidexHourBins[4] += state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneHumidexOccuHourBins[4] += NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneHumidexOccupiedHourBins[4] += (NumOcc > 0) * state.dataGlobal->TimeStepZone;
            }

            Real64 Temperature = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).ZTAV;
            auto const &zoneTstatSetpt = state.dataHeatBalFanSys->zoneTstatSetpts(ZoneNum);
            Real64 CoolingSetpoint = zoneTstatSetpt.setptHi;
            Real64 HeatingSetpoint = zoneTstatSetpt.setptLo;

            if ((CoolingSetpoint > 0) && (Temperature > CoolingSetpoint)) {
                state.dataHeatBal->Resilience(ZoneNum).ZoneUnmetDegreeHourBins[0] += (Temperature - CoolingSetpoint) * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneUnmetDegreeHourBins[1] +=
                    NumOcc * (Temperature - CoolingSetpoint) * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneUnmetDegreeHourBins[2] +=
                    (NumOcc > 0) * (Temperature - CoolingSetpoint) * state.dataGlobal->TimeStepZone;
            }
            if ((HeatingSetpoint > 0) && (Temperature < HeatingSetpoint)) {
                state.dataHeatBal->Resilience(ZoneNum).ZoneUnmetDegreeHourBins[3] += (HeatingSetpoint - Temperature) * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneUnmetDegreeHourBins[4] +=
                    NumOcc * (HeatingSetpoint - Temperature) * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneUnmetDegreeHourBins[5] +=
                    (NumOcc > 0) * (HeatingSetpoint - Temperature) * state.dataGlobal->TimeStepZone;
            }

            if (state.dataHeatBalSurfMgr->hasPierceSET) {
                int encodedMonDayHrMin;
                Real64 PierceSET = state.dataHeatBal->Resilience(ZoneNum).ZonePierceSET;
                Real64 PierceSETLast = state.dataHeatBal->Resilience(ZoneNum).ZonePierceSETLastStep;

                if (PierceSET <= 12.2) {
                    state.dataHeatBal->Resilience(ZoneNum).ZoneLowSETHours[0] += (12.2 - PierceSET) * state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->Resilience(ZoneNum).ZoneLowSETHours[1] += (12.2 - PierceSET) * NumOcc * state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->Resilience(ZoneNum).ZoneLowSETHours[2] += (12.2 - PierceSET) * (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    // Reset duration when last step is out of range.
                    if (PierceSETLast == -1 || PierceSETLast > 12.2) {
                        General::EncodeMonDayHrMin(encodedMonDayHrMin,
                                                   state.dataEnvrn->Month,
                                                   state.dataEnvrn->DayOfMonth,
                                                   state.dataGlobal->HourOfDay,
                                                   state.dataGlobal->TimeStepZone * (state.dataGlobal->TimeStep - 1) * 60);
                        state.dataHeatBalSurfMgr->lowSETLongestHours[ZoneNum - 1] = 0;
                        state.dataHeatBalSurfMgr->lowSETLongestStart[ZoneNum - 1] = encodedMonDayHrMin;
                    }
                    // Keep the longest duration record.
                    state.dataHeatBalSurfMgr->lowSETLongestHours[ZoneNum - 1] += state.dataGlobal->TimeStepZone;
                    if (state.dataHeatBalSurfMgr->lowSETLongestHours[ZoneNum - 1] > state.dataHeatBal->Resilience(ZoneNum).ZoneLowSETHours[3] &&
                        state.dataHeatBal->Resilience(ZoneNum).ZoneNumOcc > 0) {
                        state.dataHeatBal->Resilience(ZoneNum).ZoneLowSETHours[3] = state.dataHeatBalSurfMgr->lowSETLongestHours[ZoneNum - 1];
                        state.dataHeatBal->Resilience(ZoneNum).ZoneLowSETHours[4] = state.dataHeatBalSurfMgr->lowSETLongestStart[ZoneNum - 1];
                    }
                } else if (PierceSET > 30) {
                    state.dataHeatBal->Resilience(ZoneNum).ZoneHighSETHours[0] += (PierceSET - 30) * state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->Resilience(ZoneNum).ZoneHighSETHours[1] += (PierceSET - 30) * NumOcc * state.dataGlobal->TimeStepZone;
                    state.dataHeatBal->Resilience(ZoneNum).ZoneHighSETHours[2] += (PierceSET - 30) * (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    if (PierceSETLast == -1 || PierceSETLast <= 30) {
                        General::EncodeMonDayHrMin(encodedMonDayHrMin,
                                                   state.dataEnvrn->Month,
                                                   state.dataEnvrn->DayOfMonth,
                                                   state.dataGlobal->HourOfDay,
                                                   state.dataGlobal->TimeStepZone * (state.dataGlobal->TimeStep - 1) * 60);
                        state.dataHeatBalSurfMgr->highSETLongestHours[ZoneNum - 1] = 0;
                        state.dataHeatBalSurfMgr->highSETLongestStart[ZoneNum - 1] = encodedMonDayHrMin;
                    }
                    state.dataHeatBalSurfMgr->highSETLongestHours[ZoneNum - 1] += state.dataGlobal->TimeStepZone;
                    if (state.dataHeatBalSurfMgr->highSETLongestHours[ZoneNum - 1] > state.dataHeatBal->Resilience(ZoneNum).ZoneHighSETHours[3] &&
                        state.dataHeatBal->Resilience(ZoneNum).ZoneNumOcc > 0) {
                        state.dataHeatBal->Resilience(ZoneNum).ZoneHighSETHours[3] = state.dataHeatBalSurfMgr->highSETLongestHours[ZoneNum - 1];
                        state.dataHeatBal->Resilience(ZoneNum).ZoneHighSETHours[4] = state.dataHeatBalSurfMgr->highSETLongestStart[ZoneNum - 1];
                    }
                }

                if (state.dataHeatBal->Resilience(ZoneNum).ZoneNumOcc == 0) {
                    state.dataHeatBalSurfMgr->lowSETLongestHours[ZoneNum - 1] = 0;
                    state.dataHeatBalSurfMgr->highSETLongestHours[ZoneNum - 1] = 0;
                }
            }

            for (int i = 1; i <= state.dataWeather->TotThermalReportPers; i++) {
                if (reportPeriodFlags(i)) {
                    int ReportPeriodIdx = i;

                    if (HI <= 26.7) {
                        state.dataHeatBalFanSys->ZoneHeatIndexHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[0] += state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneHeatIndexOccuHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[0] +=
                            NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneHeatIndexOccupiedHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[0] +=
                            (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    } else if (HI > 26.7 && HI <= 32.2) {
                        state.dataHeatBalFanSys->ZoneHeatIndexHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[1] += state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneHeatIndexOccuHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[1] +=
                            NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneHeatIndexOccupiedHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[1] +=
                            (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    } else if (HI > 32.2 && HI <= 39.4) {
                        state.dataHeatBalFanSys->ZoneHeatIndexHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[2] += state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneHeatIndexOccuHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[2] +=
                            NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneHeatIndexOccupiedHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[2] +=
                            (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    } else if (HI > 39.4 && HI <= 51.7) {
                        state.dataHeatBalFanSys->ZoneHeatIndexHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[3] += state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneHeatIndexOccuHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[3] +=
                            NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneHeatIndexOccupiedHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[3] +=
                            (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    } else {
                        state.dataHeatBalFanSys->ZoneHeatIndexHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[4] += state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneHeatIndexOccuHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[4] +=
                            NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneHeatIndexOccupiedHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[4] +=
                            (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    }

                    if (Humidex <= 29) {
                        state.dataHeatBalFanSys->ZoneHumidexHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[0] += state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneHumidexOccuHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[0] +=
                            NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneHumidexOccupiedHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[0] +=
                            (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    } else if (Humidex > 29 && Humidex <= 40) {
                        state.dataHeatBalFanSys->ZoneHumidexHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[1] += state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneHumidexOccuHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[1] +=
                            NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneHumidexOccupiedHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[1] +=
                            (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    } else if (Humidex > 40 && Humidex <= 45) {
                        state.dataHeatBalFanSys->ZoneHumidexHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[2] += state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneHumidexOccuHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[2] +=
                            NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneHumidexOccupiedHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[2] +=
                            (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    } else if (Humidex > 45 && Humidex <= 50) {
                        state.dataHeatBalFanSys->ZoneHumidexHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[3] += state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneHumidexOccuHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[3] +=
                            NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneHumidexOccupiedHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[3] +=
                            (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    } else {
                        state.dataHeatBalFanSys->ZoneHumidexHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[4] += state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneHumidexOccuHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[4] +=
                            NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneHumidexOccupiedHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[4] +=
                            (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    }

                    if (state.dataHeatBalSurfMgr->hasPierceSET) {
                        int encodedMonDayHrMin;
                        Real64 PierceSET = state.dataHeatBal->Resilience(ZoneNum).ZonePierceSET;
                        Real64 PierceSETLast = state.dataHeatBal->Resilience(ZoneNum).ZonePierceSETLastStep;
                        if (PierceSET <= 12.2) {
                            state.dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(ZoneNum, ReportPeriodIdx)[0] +=
                                (12.2 - PierceSET) * state.dataGlobal->TimeStepZone;
                            state.dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(ZoneNum, ReportPeriodIdx)[1] +=
                                (12.2 - PierceSET) * NumOcc * state.dataGlobal->TimeStepZone;
                            state.dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(ZoneNum, ReportPeriodIdx)[2] +=
                                (12.2 - PierceSET) * (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                            // Reset duration when last step is out of range
                            if (PierceSETLast == -1 || PierceSETLast > 12.2) {
                                General::EncodeMonDayHrMin(encodedMonDayHrMin,
                                                           state.dataEnvrn->Month,
                                                           state.dataEnvrn->DayOfMonth,
                                                           state.dataGlobal->HourOfDay,
                                                           state.dataGlobal->TimeStepZone * (state.dataGlobal->TimeStep - 1) * 60);
                                state.dataHeatBalFanSys->lowSETLongestHoursRepPeriod(ZoneNum, ReportPeriodIdx) = 0;
                                state.dataHeatBalFanSys->lowSETLongestStartRepPeriod(ZoneNum, ReportPeriodIdx) = encodedMonDayHrMin;
                            } else if (General::isReportPeriodBeginning(state, ReportPeriodIdx)) { // or when it is the start of the period
                                General::EncodeMonDayHrMin(encodedMonDayHrMin,
                                                           state.dataEnvrn->Month,
                                                           state.dataEnvrn->DayOfMonth,
                                                           state.dataGlobal->HourOfDay,
                                                           state.dataGlobal->TimeStepZone * state.dataGlobal->TimeStep * 60);
                                state.dataHeatBalFanSys->highSETLongestHoursRepPeriod(ZoneNum, ReportPeriodIdx) = 0;
                                state.dataHeatBalFanSys->highSETLongestStartRepPeriod(ZoneNum, ReportPeriodIdx) = encodedMonDayHrMin;
                            }
                            // Keep the longest duration record.
                            state.dataHeatBalFanSys->lowSETLongestHoursRepPeriod(ZoneNum, ReportPeriodIdx) += state.dataGlobal->TimeStepZone;
                            if (state.dataHeatBalFanSys->lowSETLongestHoursRepPeriod(ZoneNum, ReportPeriodIdx) >
                                    state.dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(ZoneNum, ReportPeriodIdx)[3] &&
                                state.dataHeatBal->Resilience(ZoneNum).ZoneNumOcc > 0) {
                                state.dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(ZoneNum, ReportPeriodIdx)[3] =
                                    state.dataHeatBalFanSys->lowSETLongestHoursRepPeriod(ZoneNum, ReportPeriodIdx);
                                state.dataHeatBalFanSys->ZoneLowSETHoursRepPeriod(ZoneNum, ReportPeriodIdx)[4] =
                                    state.dataHeatBalFanSys->lowSETLongestStartRepPeriod(ZoneNum, ReportPeriodIdx);
                            }
                        } else if (PierceSET > 30) {
                            state.dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(ZoneNum, ReportPeriodIdx)[0] +=
                                (PierceSET - 30) * state.dataGlobal->TimeStepZone;
                            state.dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(ZoneNum, ReportPeriodIdx)[1] +=
                                (PierceSET - 30) * NumOcc * state.dataGlobal->TimeStepZone;
                            state.dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(ZoneNum, ReportPeriodIdx)[2] +=
                                (PierceSET - 30) * (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                            // Reset duration when last step is out of range.
                            if (PierceSETLast == -1 || PierceSETLast <= 30) {
                                General::EncodeMonDayHrMin(encodedMonDayHrMin,
                                                           state.dataEnvrn->Month,
                                                           state.dataEnvrn->DayOfMonth,
                                                           state.dataGlobal->HourOfDay,
                                                           state.dataGlobal->TimeStepZone * (state.dataGlobal->TimeStep - 1) * 60);
                                state.dataHeatBalFanSys->highSETLongestHoursRepPeriod(ZoneNum, ReportPeriodIdx) = 0;
                                state.dataHeatBalFanSys->highSETLongestStartRepPeriod(ZoneNum, ReportPeriodIdx) = encodedMonDayHrMin;
                            } else if (General::isReportPeriodBeginning(state, ReportPeriodIdx)) { // or when it is the start of the period
                                General::EncodeMonDayHrMin(encodedMonDayHrMin,
                                                           state.dataEnvrn->Month,
                                                           state.dataEnvrn->DayOfMonth,
                                                           state.dataGlobal->HourOfDay,
                                                           state.dataGlobal->TimeStepZone * state.dataGlobal->TimeStep * 60);
                                state.dataHeatBalFanSys->highSETLongestHoursRepPeriod(ZoneNum, ReportPeriodIdx) = 0;
                                state.dataHeatBalFanSys->highSETLongestStartRepPeriod(ZoneNum, ReportPeriodIdx) = encodedMonDayHrMin;
                            }
                            state.dataHeatBalFanSys->highSETLongestHoursRepPeriod(ZoneNum, ReportPeriodIdx) += state.dataGlobal->TimeStepZone;
                            if (state.dataHeatBalFanSys->highSETLongestHoursRepPeriod(ZoneNum, ReportPeriodIdx) >
                                    state.dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(ZoneNum, ReportPeriodIdx)[3] &&
                                state.dataHeatBal->Resilience(ZoneNum).ZoneNumOcc > 0) {
                                state.dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(ZoneNum, ReportPeriodIdx)[3] =
                                    state.dataHeatBalFanSys->highSETLongestHoursRepPeriod(ZoneNum, ReportPeriodIdx);
                                state.dataHeatBalFanSys->ZoneHighSETHoursRepPeriod(ZoneNum, ReportPeriodIdx)[4] =
                                    state.dataHeatBalFanSys->highSETLongestStartRepPeriod(ZoneNum, ReportPeriodIdx);
                            }
                        }
                        if (state.dataHeatBal->Resilience(ZoneNum).ZoneNumOcc == 0) {
                            state.dataHeatBalFanSys->lowSETLongestHoursRepPeriod(ZoneNum, ReportPeriodIdx) = 0;
                            state.dataHeatBalFanSys->highSETLongestHoursRepPeriod(ZoneNum, ReportPeriodIdx) = 0;
                        }
                    }

                    if ((CoolingSetpoint > 0) && (Temperature > CoolingSetpoint)) {
                        state.dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[0] +=
                            (Temperature - CoolingSetpoint) * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[1] +=
                            NumOcc * (Temperature - CoolingSetpoint) * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[2] +=
                            (NumOcc > 0) * (Temperature - CoolingSetpoint) * state.dataGlobal->TimeStepZone;
                    }
                    if ((HeatingSetpoint > 0) && (Temperature < HeatingSetpoint)) {
                        state.dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[3] +=
                            (HeatingSetpoint - Temperature) * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[4] +=
                            NumOcc * (HeatingSetpoint - Temperature) * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneUnmetDegreeHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[5] +=
                            (NumOcc > 0) * (HeatingSetpoint - Temperature) * state.dataGlobal->TimeStepZone;
                    }
                }
            }
        } // loop over zones
    }
}

void ReportCO2Resilience(EnergyPlusData &state)
{
    if (state.dataHeatBalSurfMgr->reportCO2ResilienceFirstTime) {
        int NoBins = 3;
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            for (int i = 1; i <= state.dataWeather->TotCO2ReportPers; i++) {
                state.dataHeatBalFanSys->ZoneCO2LevelHourBinsRepPeriod(ZoneNum, i).assign(NoBins, 0.0);
                state.dataHeatBalFanSys->ZoneCO2LevelOccuHourBinsRepPeriod(ZoneNum, i).assign(NoBins, 0.0);
                state.dataHeatBalFanSys->ZoneCO2LevelOccupiedHourBinsRepPeriod(ZoneNum, i).assign(NoBins, 0.0);
            }
        }
        state.dataHeatBalSurfMgr->reportCO2ResilienceFirstTime = false;
        if (!state.dataContaminantBalance->Contaminant.CO2Simulation) {
            if (state.dataOutRptTab->displayCO2ResilienceSummaryExplicitly) {
                ShowWarningError(state,
                                 "Writing Annual CO2 Resilience Summary - CO2 Level Hours reports: "
                                 "Zone Air CO2 Concentration output is required, "
                                 "but no ZoneAirContaminantBalance object is defined.");
            }
            state.dataOutRptTab->displayCO2ResilienceSummary = false;
            return;
        }
    }

    if (Constant::KindOfSim::RunPeriodWeather == state.dataGlobal->KindOfSim && !state.dataGlobal->WarmupFlag) {
        for (auto const &people : state.dataHeatBal->People) {
            state.dataHeatBal->Resilience(people.ZonePtr).ZoneNumOcc = people.NumberOfPeople * people.sched->getCurrentVal();
        }

        Array1D_bool reportPeriodFlags;
        if (state.dataWeather->TotReportPers > 0) {
            reportPeriodFlags.dimension(state.dataWeather->TotCO2ReportPers, false);
            General::findReportPeriodIdx(state, state.dataWeather->CO2ReportPeriodInput, state.dataWeather->TotCO2ReportPers, reportPeriodFlags);
        }

        auto &ort = state.dataOutRptTab;
        for (int i = 1; i <= state.dataWeather->TotCO2ReportPers; i++) {
            if (reportPeriodFlags(i)) {
                int curResMeterNumber = ort->meterNumTotalsBEPS(1);
                state.dataWeather->CO2ReportPeriodInput(i).totalElectricityUse += GetCurrentMeterValue(state, curResMeterNumber);
            }
        }

        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            Real64 ZoneAirCO2 = state.dataContaminantBalance->ZoneAirCO2Avg(ZoneNum);

            Real64 NumOcc = state.dataHeatBal->Resilience(ZoneNum).ZoneNumOcc;
            if (ZoneAirCO2 <= 1000) {
                state.dataHeatBal->Resilience(ZoneNum).ZoneCO2LevelHourBins[0] += state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneCO2LevelOccuHourBins[0] += NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneCO2LevelOccupiedHourBins[0] += (NumOcc > 0) * state.dataGlobal->TimeStepZone;
            } else if (ZoneAirCO2 > 1000 && ZoneAirCO2 <= 5000) {
                state.dataHeatBal->Resilience(ZoneNum).ZoneCO2LevelHourBins[1] += state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneCO2LevelOccuHourBins[1] += NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneCO2LevelOccupiedHourBins[1] += (NumOcc > 0) * state.dataGlobal->TimeStepZone;
            } else {
                state.dataHeatBal->Resilience(ZoneNum).ZoneCO2LevelHourBins[2] += state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneCO2LevelOccuHourBins[2] += NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneCO2LevelOccupiedHourBins[2] += (NumOcc > 0) * state.dataGlobal->TimeStepZone;
            }
            for (int i = 1; i <= state.dataWeather->TotCO2ReportPers; i++) {
                if (reportPeriodFlags(i)) {
                    int ReportPeriodIdx = i;
                    if (ZoneAirCO2 <= 1000) {
                        state.dataHeatBalFanSys->ZoneCO2LevelHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[0] += state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneCO2LevelOccuHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[0] +=
                            NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneCO2LevelOccupiedHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[0] +=
                            (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    } else if (ZoneAirCO2 > 1000 && ZoneAirCO2 <= 5000) {
                        state.dataHeatBalFanSys->ZoneCO2LevelHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[1] += state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneCO2LevelOccuHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[1] +=
                            NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneCO2LevelOccupiedHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[1] +=
                            (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    } else {
                        state.dataHeatBalFanSys->ZoneCO2LevelHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[2] += state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneCO2LevelOccuHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[2] +=
                            NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneCO2LevelOccupiedHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[2] +=
                            (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    }
                }
            }
        }
    } // loop over zones
}

void ReportVisualResilience(EnergyPlusData &state)
{
    if (state.dataHeatBalSurfMgr->reportVisualResilienceFirstTime) {
        int NoBins = 4;
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            for (int i = 1; i <= state.dataWeather->TotVisualReportPers; i++) {
                state.dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(ZoneNum, i).assign(NoBins, 0.0);
                state.dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(ZoneNum, i).assign(NoBins, 0.0);
                state.dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(ZoneNum, i).assign(NoBins, 0.0);
            }
        }
        state.dataHeatBalSurfMgr->reportVisualResilienceFirstTime = false;
        if ((int)state.dataDayltg->daylightControl.size() == 0) {
            if (state.dataOutRptTab->displayVisualResilienceSummaryExplicitly) {
                ShowWarningError(state,
                                 "Writing Annual Visual Resilience Summary - Lighting Level Hours reports: "
                                 "Zone Average Daylighting Reference Point Illuminance output is required, "
                                 "but no Daylighting Control Object is defined.");
            }
            state.dataOutRptTab->displayVisualResilienceSummary = false;
            return;
        }
    }

    if (Constant::KindOfSim::RunPeriodWeather == state.dataGlobal->KindOfSim && !state.dataGlobal->WarmupFlag) {
        for (auto const &people : state.dataHeatBal->People) {
            state.dataHeatBal->Resilience(people.ZonePtr).ZoneNumOcc = people.NumberOfPeople * people.sched->getCurrentVal();
        }
        // Accumulate across daylighting controls first
        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            state.dataDayltg->ZoneDaylight(ZoneNum).zoneAvgIllumSum = 0.0;
        }
        for (int daylightCtrlNum = 1; daylightCtrlNum <= (int)state.dataDayltg->daylightControl.size(); ++daylightCtrlNum) {
            auto &thisDaylightControl = state.dataDayltg->daylightControl(daylightCtrlNum);
            if (thisDaylightControl.PowerReductionFactor > 0) {
                for (int refPt = 1; refPt <= thisDaylightControl.TotalDaylRefPoints; ++refPt) {
                    state.dataDayltg->ZoneDaylight(thisDaylightControl.zoneIndex).zoneAvgIllumSum += thisDaylightControl.refPts(refPt).illumSetPoint;
                }
            } else {
                for (int refPt = 1; refPt <= thisDaylightControl.TotalDaylRefPoints; ++refPt) {
                    state.dataDayltg->ZoneDaylight(thisDaylightControl.zoneIndex).zoneAvgIllumSum +=
                        thisDaylightControl.refPts(refPt).lums[(int)DataSurfaces::Lum::Illum];
                }
            }
        }

        Array1D_bool reportPeriodFlags;
        if (state.dataWeather->TotReportPers > 0) {
            reportPeriodFlags.dimension(state.dataWeather->TotVisualReportPers, false);
            General::findReportPeriodIdx(
                state, state.dataWeather->VisualReportPeriodInput, state.dataWeather->TotVisualReportPers, reportPeriodFlags);
        }

        auto &ort = state.dataOutRptTab;
        for (int i = 1; i <= state.dataWeather->TotVisualReportPers; i++) {
            if (reportPeriodFlags(i)) {
                int curResMeterNumber = ort->meterNumTotalsBEPS(1);
                state.dataWeather->VisualReportPeriodInput(i).totalElectricityUse += GetCurrentMeterValue(state, curResMeterNumber);
            }
        }

        for (int ZoneNum = 1; ZoneNum <= state.dataGlobal->NumOfZones; ++ZoneNum) {
            if (state.dataDayltg->ZoneDaylight(ZoneNum).totRefPts == 0) {
                continue;
            }
            // Now divide by total reference points to get average
            Real64 avgZoneIllum = state.dataDayltg->ZoneDaylight(ZoneNum).zoneAvgIllumSum / state.dataDayltg->ZoneDaylight(ZoneNum).totRefPts;

            Real64 NumOcc = state.dataHeatBal->Resilience(ZoneNum).ZoneNumOcc;
            if (avgZoneIllum <= 100) {
                state.dataHeatBal->Resilience(ZoneNum).ZoneLightingLevelHourBins[0] += state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneLightingLevelOccuHourBins[0] += NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneLightingLevelOccupiedHourBins[0] += (NumOcc > 0) * state.dataGlobal->TimeStepZone;
            } else if (avgZoneIllum > 100 && avgZoneIllum <= 300) {
                state.dataHeatBal->Resilience(ZoneNum).ZoneLightingLevelHourBins[1] += state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneLightingLevelOccuHourBins[1] += NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneLightingLevelOccupiedHourBins[1] += (NumOcc > 0) * state.dataGlobal->TimeStepZone;
            } else if (avgZoneIllum > 300 && avgZoneIllum <= 500) {
                state.dataHeatBal->Resilience(ZoneNum).ZoneLightingLevelHourBins[2] += state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneLightingLevelOccuHourBins[2] += NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneLightingLevelOccupiedHourBins[2] += (NumOcc > 0) * state.dataGlobal->TimeStepZone;
            } else {
                state.dataHeatBal->Resilience(ZoneNum).ZoneLightingLevelHourBins[3] += state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneLightingLevelOccuHourBins[3] += NumOcc * state.dataGlobal->TimeStepZone;
                state.dataHeatBal->Resilience(ZoneNum).ZoneLightingLevelOccupiedHourBins[3] += (NumOcc > 0) * state.dataGlobal->TimeStepZone;
            }
            for (int i = 1; i <= state.dataWeather->TotVisualReportPers; i++) {
                if (reportPeriodFlags(i)) {
                    int ReportPeriodIdx = i;
                    if (avgZoneIllum <= 100) {
                        state.dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[0] += state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[0] +=
                            NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[0] +=
                            (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    } else if (avgZoneIllum > 100 && avgZoneIllum <= 300) {
                        state.dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[1] += state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[1] +=
                            NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[1] +=
                            (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    } else if (avgZoneIllum > 300 && avgZoneIllum <= 500) {
                        state.dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[2] += state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[2] +=
                            NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[2] +=
                            (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    } else {
                        state.dataHeatBalFanSys->ZoneLightingLevelHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[3] += state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneLightingLevelOccuHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[3] +=
                            NumOcc * state.dataGlobal->TimeStepZone;
                        state.dataHeatBalFanSys->ZoneLightingLevelOccupiedHourBinsRepPeriod(ZoneNum, ReportPeriodIdx)[3] +=
                            (NumOcc > 0) * state.dataGlobal->TimeStepZone;
                    }
                }
            }
        }
    } // loop over zones
}

void ReportSurfaceHeatBalance(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Linda Lawrie
    //       DATE WRITTEN   Oct 2000

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine puts the reporting part of the HBSurface Module in one area.

    SolarShading::ReportSurfaceShading(state);

    if (state.dataSurface->UseRepresentativeSurfaceCalculations) {
        ReportNonRepresentativeSurfaceResults(state);
    }

    // Set derived surface output variables and other record keeping - after iterations are complete - all HT surfaces

    // Opaque or window surfaces
    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurf = thisSpace.OpaqOrWinSurfaceFirst;
            int const lastSurf = thisSpace.OpaqOrWinSurfaceLast;
            for (int surfNum = firstSurf; surfNum <= lastSurf; ++surfNum) {
                auto const &surface = state.dataSurface->Surface(surfNum);
                // Inside Face Convection - sign convention is positive means energy going into inside face from the air.
                state.dataHeatBalSurf->SurfQdotConvInRep(surfNum) = surface.Area * state.dataHeatBalSurf->SurfQdotConvInPerArea(surfNum);
                state.dataHeatBalSurf->SurfQConvInRep(surfNum) =
                    state.dataHeatBalSurf->SurfQdotConvInRep(surfNum) * state.dataGlobal->TimeStepZoneSec;

                state.dataHeatBalSurf->SurfQdotRadNetSurfInRep(surfNum) = state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(surfNum) * surface.Area;
                state.dataHeatBalSurf->SurfQRadNetSurfInRep(surfNum) =
                    state.dataHeatBalSurf->SurfQdotRadNetSurfInRep(surfNum) * state.dataGlobal->TimeStepZoneSec;

                state.dataHeatBalSurf->SurfQdotRadIntGainsInRep(surfNum) = state.dataHeatBal->SurfQdotRadIntGainsInPerArea(surfNum) * surface.Area;
                state.dataHeatBalSurf->SurfQRadIntGainsInRep(surfNum) =
                    state.dataHeatBalSurf->SurfQdotRadIntGainsInRep(surfNum) * state.dataGlobal->TimeStepZoneSec;

                state.dataHeatBalSurf->SurfQdotRadHVACInRep(surfNum) = state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(surfNum) * surface.Area;
                state.dataHeatBalSurf->SurfQRadHVACInRep(surfNum) =
                    state.dataHeatBalSurf->SurfQdotRadHVACInRep(surfNum) * state.dataGlobal->TimeStepZoneSec;

                state.dataHeatBalSurf->SurfQdotConvOutRep(surfNum) = state.dataHeatBalSurf->SurfQdotConvOutPerArea(surfNum) * surface.Area;

                state.dataHeatBalSurf->SurfQConvOutReport(surfNum) =
                    state.dataHeatBalSurf->SurfQdotConvOutRep(surfNum) * state.dataGlobal->TimeStepZoneSec;

                state.dataHeatBalSurf->SurfQdotRadOutRep(surfNum) = state.dataHeatBalSurf->SurfQdotRadOutRepPerArea(surfNum) * surface.Area;
                state.dataHeatBalSurf->SurfQRadOutReport(surfNum) =
                    state.dataHeatBalSurf->SurfQdotRadOutRep(surfNum) * state.dataGlobal->TimeStepZoneSec;

                // Calculate surface heat emission to the air, positive values indicates heat transfer from surface to the outside
                state.dataHeatBalSurf->SurfQAirExtReport(surfNum) =
                    surface.Area * state.dataHeatBalSurf->SurfHAirExt(surfNum) *
                    (state.dataHeatBalSurf->SurfTempOut(surfNum) - state.dataSurface->SurfOutDryBulbTemp(surfNum));

                // Subtract since SurfQdotConvOutRep's convention is opposite (positive values indicate heat transfer from the outside to the surface)
                state.dataHeatBalSurf->SurfQHeatEmiReport(surfNum) =
                    state.dataHeatBalSurf->SurfQAirExtReport(surfNum) - state.dataHeatBalSurf->SurfQdotConvOutRep(surfNum);
            }
        }
    }

    if (state.dataOutRptTab->displayHeatEmissionsSummary) {
        state.dataHeatBalSurf->SumSurfaceHeatEmission = 0.0;
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            if (state.dataSurface->Surface(SurfNum).ExtBoundCond == DataSurfaces::ExternalEnvironment) {
                state.dataHeatBalSurf->SumSurfaceHeatEmission +=
                    state.dataHeatBalSurf->SurfQHeatEmiReport(SurfNum) * state.dataGlobal->TimeStepZoneSec;
            }
        }
    }

    // Window surfaces
    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurf = thisSpace.WindowSurfaceFirst;
            int const lastSurf = thisSpace.WindowSurfaceLast;
            for (int surfNum = firstSurf; surfNum <= lastSurf; ++surfNum) {
                auto const &surface = state.dataSurface->Surface(surfNum);
                state.dataHeatBal->SurfWinInitialDifSolInTransReport(surfNum) =
                    state.dataHeatBalSurf->SurfWinInitialDifSolInTrans(surfNum) * surface.Area;

                // Absorbed short wave radiation
                int TotGlassLayers;
                int const constrNum = state.dataSurface->SurfActiveConstruction(surfNum);
                auto const &thisConstruct = state.dataConstruction->Construct(constrNum);
                int const constrNumSh = state.dataSurface->SurfWinActiveShadedConstruction(surfNum);
                DataSurfaces::WinShadingType ShadeFlag = state.dataSurface->SurfWinShadingFlag(surfNum);
                if (state.dataSurface->SurfWinWindowModelType(surfNum) == DataSurfaces::WindowModel::EQL) {
                    TotGlassLayers = state.dataWindowEquivLayer->CFS(thisConstruct.EQLConsPtr).NL;
                } else if (state.dataSurface->SurfWinWindowModelType(surfNum) == DataSurfaces::WindowModel::BSDF) {
                    TotGlassLayers = thisConstruct.TotSolidLayers;
                } else if (DataSurfaces::NOT_SHADED(ShadeFlag) || ShadeFlag == DataSurfaces::WinShadingType::SwitchableGlazing) {
                    TotGlassLayers = thisConstruct.TotGlassLayers;
                } else {
                    // Interior, exterior or between-glass shade, screen or blind in place
                    TotGlassLayers = state.dataConstruction->Construct(constrNumSh).TotGlassLayers;
                }
                state.dataHeatBal->SurfWinSWwinAbsTotalReport(surfNum) = 0.0;
                state.dataHeatBal->SurfSWInAbsTotalReport(surfNum) = 0.0;
                state.dataHeatBal->SurfInitialDifSolInAbsReport(surfNum) = 0.0;
                for (int lay = 1; lay <= TotGlassLayers; ++lay) {
                    // Initial Transmitted Diffuse Solar Absorbed on Inside of Surface[W]
                    state.dataHeatBal->SurfInitialDifSolInAbsReport(surfNum) +=
                        state.dataHeatBal->SurfWinInitialDifSolwinAbs(surfNum, lay) * surface.Area;
                    // Total Shortwave Radiation Absorbed on Inside of Surface[W]
                    state.dataHeatBal->SurfSWInAbsTotalReport(surfNum) += state.dataHeatBal->SurfWinQRadSWwinAbs(surfNum, lay) * surface.Area;
                    // Total Shortwave Absorbed:All solid Layers[W]
                    state.dataHeatBal->SurfWinSWwinAbsTotalReport(surfNum) += state.dataHeatBal->SurfWinQRadSWwinAbs(surfNum, lay) * surface.Area;
                }

                // Window heat gain/loss
                if (state.dataSurface->SurfWinHeatGain(surfNum) >= 0.0) {
                    state.dataSurface->SurfWinHeatGainRep(surfNum) = state.dataSurface->SurfWinHeatGain(surfNum);
                    state.dataSurface->SurfWinHeatGainRepEnergy(surfNum) =
                        state.dataSurface->SurfWinHeatGainRep(surfNum) * state.dataGlobal->TimeStepZoneSec;
                } else {
                    state.dataSurface->SurfWinHeatLossRep(surfNum) = -state.dataSurface->SurfWinHeatGain(surfNum);
                    state.dataSurface->SurfWinHeatLossRepEnergy(surfNum) =
                        state.dataSurface->SurfWinHeatLossRep(surfNum) * state.dataGlobal->TimeStepZoneSec;
                }
                state.dataSurface->SurfWinHeatTransferRepEnergy(surfNum) =
                    state.dataSurface->SurfWinHeatGain(surfNum) * state.dataGlobal->TimeStepZoneSec;
                if (state.dataSurface->Surface(surfNum).OriginalClass == DataSurfaces::SurfaceClass::TDD_Diffuser) { // Tubular daylighting device
                    int pipeNum = state.dataSurface->SurfWinTDDPipeNum(surfNum);
                    state.dataDaylightingDevicesData->TDDPipe(pipeNum).HeatGain = state.dataSurface->SurfWinHeatGainRep(surfNum);
                    state.dataDaylightingDevicesData->TDDPipe(pipeNum).HeatLoss = state.dataSurface->SurfWinHeatLossRep(surfNum);
                }
            }
        }
    }

    if (state.dataSurface->AnyMovableInsulation) {
        ReportIntMovInsInsideSurfTemp(state);
    }

    // Opaque heat transfer surfaces
    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            int const firstSurf = thisSpace.OpaqOrIntMassSurfaceFirst;
            int const lastSurf = thisSpace.OpaqOrIntMassSurfaceLast;
            for (int surfNum = firstSurf; surfNum <= lastSurf; ++surfNum) {
                auto const &surface = state.dataSurface->Surface(surfNum);

                state.dataHeatBal->SurfOpaqSWOutAbsTotalReport(surfNum) = state.dataHeatBalSurf->SurfOpaqQRadSWOutAbs(surfNum) * surface.Area;
                state.dataHeatBal->SurfOpaqSWOutAbsEnergyReport(surfNum) =
                    state.dataHeatBal->SurfOpaqSWOutAbsTotalReport(surfNum) * state.dataGlobal->TimeStepZoneSec;

                state.dataHeatBalSurf->SurfQdotRadSolarInRep(surfNum) = state.dataHeatBalSurf->SurfQdotRadSolarInRepPerArea(surfNum) * surface.Area;
                state.dataHeatBalSurf->SurfQRadSolarInRep(surfNum) =
                    state.dataHeatBalSurf->SurfQdotRadSolarInRep(surfNum) * state.dataGlobal->TimeStepZoneSec;

                state.dataHeatBalSurf->SurfQdotRadLightsInRep(surfNum) = state.dataHeatBalSurf->SurfQdotRadLightsInPerArea(surfNum) * surface.Area;
                state.dataHeatBalSurf->SurfQRadLightsInRep(surfNum) =
                    state.dataHeatBalSurf->SurfQdotRadLightsInRep(surfNum) * state.dataGlobal->TimeStepZoneSec;

                // Initial Transmitted Diffuse Solar Absorbed on Inside of Surface[W]
                state.dataHeatBal->SurfInitialDifSolInAbsReport(surfNum) = state.dataHeatBalSurf->SurfOpaqInitialDifSolInAbs(surfNum) * surface.Area;
                // Total Shortwave Radiation Absorbed on Inside of Surface[W]
                state.dataHeatBal->SurfSWInAbsTotalReport(surfNum) = state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(surfNum) * surface.Area;

                // inside face conduction updates
                state.dataHeatBalSurf->SurfOpaqInsFaceCond(surfNum) = state.dataHeatBalSurf->SurfOpaqInsFaceCondFlux(surfNum) * surface.Area;
                state.dataHeatBalSurf->SurfOpaqInsFaceCondEnergy(surfNum) =
                    state.dataHeatBalSurf->SurfOpaqInsFaceCond(surfNum) * state.dataGlobal->TimeStepZoneSec;
                state.dataHeatBalSurf->SurfOpaqInsFaceCondGainRep(surfNum) = 0.0;
                state.dataHeatBalSurf->SurfOpaqInsFaceCondLossRep(surfNum) = 0.0;
                if (state.dataHeatBalSurf->SurfOpaqInsFaceCond(surfNum) >= 0.0) {
                    state.dataHeatBalSurf->SurfOpaqInsFaceCondGainRep(surfNum) = state.dataHeatBalSurf->SurfOpaqInsFaceCond(surfNum);
                } else {
                    state.dataHeatBalSurf->SurfOpaqInsFaceCondLossRep(surfNum) = -state.dataHeatBalSurf->SurfOpaqInsFaceCond(surfNum);
                }

                // outside face conduction updates
                state.dataHeatBalSurf->SurfOpaqOutFaceCond(surfNum) = surface.Area * state.dataHeatBalSurf->SurfOpaqOutFaceCondFlux(surfNum);
                state.dataHeatBalSurf->SurfOpaqOutFaceCondEnergy(surfNum) =
                    state.dataHeatBalSurf->SurfOpaqOutFaceCond(surfNum) * state.dataGlobal->TimeStepZoneSec;
                state.dataHeatBalSurf->SurfOpaqExtFaceCondGainRep(surfNum) = 0.0;
                state.dataHeatBalSurf->SurfOpaqExtFaceCondLossRep(surfNum) = 0.0;
                if (state.dataHeatBalSurf->SurfOpaqOutFaceCond(surfNum) >= 0.0) {
                    state.dataHeatBalSurf->SurfOpaqExtFaceCondGainRep(surfNum) = state.dataHeatBalSurf->SurfOpaqOutFaceCond(surfNum);
                } else {
                    state.dataHeatBalSurf->SurfOpaqExtFaceCondLossRep(surfNum) = -state.dataHeatBalSurf->SurfOpaqOutFaceCond(surfNum);
                }
            }
            for (int surfNum = firstSurf; surfNum <= lastSurf; ++surfNum) {
                // do average surface conduction updates

                state.dataHeatBalSurf->SurfOpaqAvgFaceCond(surfNum) =
                    (state.dataHeatBalSurf->SurfOpaqInsFaceCond(surfNum) - state.dataHeatBalSurf->SurfOpaqOutFaceCond(surfNum)) / 2.0;
                state.dataHeatBalSurf->SurfOpaqAvgFaceCondFlux(surfNum) =
                    (state.dataHeatBalSurf->SurfOpaqInsFaceCondFlux(surfNum) - state.dataHeatBalSurf->SurfOpaqOutFaceCondFlux(surfNum)) / 2.0;
                state.dataHeatBalSurf->SurfOpaqAvgFaceCondEnergy(surfNum) =
                    state.dataHeatBalSurf->SurfOpaqAvgFaceCond(surfNum) * state.dataGlobal->TimeStepZoneSec;
                state.dataHeatBalSurf->SurfOpaqAvgFaceCondGainRep(surfNum) = 0.0;
                state.dataHeatBalSurf->SurfOpaqAvgFaceCondLossRep(surfNum) = 0.0;
                if (state.dataHeatBalSurf->SurfOpaqAvgFaceCond(surfNum) >= 0.0) {
                    state.dataHeatBalSurf->SurfOpaqAvgFaceCondGainRep(surfNum) = state.dataHeatBalSurf->SurfOpaqAvgFaceCond(surfNum);
                } else {
                    state.dataHeatBalSurf->SurfOpaqAvgFaceCondLossRep(surfNum) = -state.dataHeatBalSurf->SurfOpaqAvgFaceCond(surfNum);
                }

                // do surface storage rate updates
                state.dataHeatBalSurf->SurfOpaqStorageCondFlux(surfNum) =
                    -(state.dataHeatBalSurf->SurfOpaqInsFaceCondFlux(surfNum) + state.dataHeatBalSurf->SurfOpaqOutFaceCondFlux(surfNum));
                state.dataHeatBalSurf->SurfOpaqStorageCond(surfNum) =
                    -(state.dataHeatBalSurf->SurfOpaqInsFaceCond(surfNum) + state.dataHeatBalSurf->SurfOpaqOutFaceCond(surfNum));
                state.dataHeatBalSurf->SurfOpaqStorageCondEnergy(surfNum) =
                    state.dataHeatBalSurf->SurfOpaqStorageCond(surfNum) * state.dataGlobal->TimeStepZoneSec;
                state.dataHeatBalSurf->SurfOpaqStorageCondGainRep(surfNum) = 0.0;
                state.dataHeatBalSurf->SurfOpaqStorageCondLossRep(surfNum) = 0.0;
                if (state.dataHeatBalSurf->SurfOpaqStorageCond(surfNum) >= 0.0) {
                    state.dataHeatBalSurf->SurfOpaqStorageCondGainRep(surfNum) = state.dataHeatBalSurf->SurfOpaqStorageCond(surfNum);
                } else {
                    state.dataHeatBalSurf->SurfOpaqStorageCondLossRep(surfNum) = -state.dataHeatBalSurf->SurfOpaqStorageCond(surfNum);
                }
            }
        }
    }

    if (state.dataGlobal->ZoneSizingCalc && state.dataGlobal->CompLoadReportIsReq) {
        // This is by surface, so it works for both space and zone component loads
        int TimeStepInDay = (state.dataGlobal->HourOfDay - 1) * state.dataGlobal->TimeStepsInHour + state.dataGlobal->TimeStep;
        auto &surfCLDayTS = state.dataOutRptTab->surfCompLoads[state.dataSize->CurOverallSimDay - 1].ts[TimeStepInDay - 1];
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                int firstSurf = thisSpace.OpaqOrIntMassSurfaceFirst;
                int lastSurf = thisSpace.OpaqOrIntMassSurfaceLast;
                for (int surfNum = firstSurf; surfNum <= lastSurf; ++surfNum) {
                    surfCLDayTS.surf[surfNum - 1].lightSWRadSeq = state.dataHeatBalSurf->SurfQdotRadLightsInRep(surfNum);
                    surfCLDayTS.surf[surfNum - 1].feneSolarRadSeq = state.dataHeatBalSurf->SurfQdotRadSolarInRep(surfNum);
                }
                firstSurf = thisSpace.OpaqOrWinSurfaceFirst;
                lastSurf = thisSpace.OpaqOrWinSurfaceLast;
                for (int surfNum = firstSurf; surfNum <= lastSurf; ++surfNum) {
                    auto const &surface = state.dataSurface->Surface(surfNum);
                    if (!state.dataGlobal->WarmupFlag) {
                        if (state.dataGlobal->isPulseZoneSizing) {
                            surfCLDayTS.surf[surfNum - 1].loadConvectedWithPulse = state.dataHeatBalSurf->SurfQdotConvInRep(surfNum);
                        } else {
                            surfCLDayTS.surf[surfNum - 1].loadConvectedNormal = state.dataHeatBalSurf->SurfQdotConvInRep(surfNum);
                            surfCLDayTS.surf[surfNum - 1].netSurfRadSeq = state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(surfNum) * surface.Area;
                        }
                    }
                }
            }
        }
    }

    if (state.dataGlobal->DisplayAdvancedReportVariables) {
        for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
            for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                int const firstSurf = thisSpace.OpaqOrIntMassSurfaceFirst;
                int const lastSurf = thisSpace.OpaqOrIntMassSurfaceLast;
                for (int surfNum = firstSurf; surfNum <= lastSurf; ++surfNum) {
                    state.dataHeatBal->ZoneOpaqSurfInsFaceCond(zoneNum) += state.dataHeatBalSurf->SurfOpaqInsFaceCond(surfNum);
                    state.dataHeatBal->ZoneOpaqSurfExtFaceCond(zoneNum) += state.dataHeatBalSurf->SurfOpaqOutFaceCond(surfNum);
                }
                if (state.dataHeatBal->ZoneOpaqSurfInsFaceCond(zoneNum) >= 0.0) {
                    state.dataHeatBal->ZoneOpaqSurfInsFaceCondGainRep(zoneNum) = state.dataHeatBal->ZoneOpaqSurfInsFaceCond(zoneNum);
                    state.dataHeatBal->ZnOpqSurfInsFaceCondGnRepEnrg(zoneNum) =
                        state.dataHeatBal->ZoneOpaqSurfInsFaceCondGainRep(zoneNum) * state.dataGlobal->TimeStepZoneSec;
                } else {
                    state.dataHeatBal->ZoneOpaqSurfInsFaceCondLossRep(zoneNum) = -state.dataHeatBal->ZoneOpaqSurfInsFaceCond(zoneNum);
                    state.dataHeatBal->ZnOpqSurfInsFaceCondLsRepEnrg(zoneNum) =
                        state.dataHeatBal->ZoneOpaqSurfInsFaceCondLossRep(zoneNum) * state.dataGlobal->TimeStepZoneSec;
                }

                if (state.dataHeatBal->ZoneOpaqSurfExtFaceCond(zoneNum) >= 0.0) {
                    state.dataHeatBal->ZoneOpaqSurfExtFaceCondGainRep(zoneNum) = state.dataHeatBal->ZoneOpaqSurfExtFaceCond(zoneNum);
                    state.dataHeatBal->ZnOpqSurfExtFaceCondGnRepEnrg(zoneNum) =
                        state.dataHeatBal->ZoneOpaqSurfExtFaceCondGainRep(zoneNum) * state.dataGlobal->TimeStepZoneSec;
                } else {
                    state.dataHeatBal->ZoneOpaqSurfExtFaceCondLossRep(zoneNum) = -state.dataHeatBal->ZoneOpaqSurfExtFaceCond(zoneNum);
                    state.dataHeatBal->ZnOpqSurfExtFaceCondLsRepEnrg(zoneNum) =
                        state.dataHeatBal->ZoneOpaqSurfExtFaceCondLossRep(zoneNum) * state.dataGlobal->TimeStepZoneSec;
                }
            }
        }
    }
}

void ReportNonRepresentativeSurfaceResults(EnergyPlusData &state)
{
    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            // Heat transfer surfaces
            int firstSurf = thisSpace.HTSurfaceFirst;
            int lastSurf = thisSpace.HTSurfaceLast;
            for (int surfNum = firstSurf; surfNum <= lastSurf; ++surfNum) {
                auto const &surface = state.dataSurface->Surface(surfNum);
                int repSurfNum = surface.RepresentativeCalcSurfNum;
                if (surfNum != repSurfNum) {
                    state.dataSurface->surfIntConv(surfNum).convClassRpt = state.dataSurface->surfIntConv(repSurfNum).convClassRpt;
                    state.dataSurface->surfExtConv(surfNum).convClassRpt = state.dataSurface->surfExtConv(repSurfNum).convClassRpt;
                }
            }

            // Windows
            if (state.dataGlobal->DisplayAdvancedReportVariables) {
                firstSurf = thisSpace.WindowSurfaceFirst;
                lastSurf = thisSpace.WindowSurfaceLast;
                for (int surfNum = firstSurf; surfNum <= lastSurf; ++surfNum) {
                    auto const &surface = state.dataSurface->Surface(surfNum);
                    int repSurfNum = surface.RepresentativeCalcSurfNum;
                    if (surfNum != repSurfNum) {
                        Real64 areaRatio = surface.Area / state.dataSurface->Surface(surfNum).Area;
                        state.dataSurface->SurfWinGainConvGlazToZoneRep(surfNum) =
                            state.dataSurface->SurfWinGainConvGlazToZoneRep(repSurfNum) * areaRatio;
                        state.dataSurface->SurfWinGainIRGlazToZoneRep(surfNum) =
                            state.dataSurface->SurfWinGainIRGlazToZoneRep(repSurfNum) * areaRatio;
                        state.dataSurface->SurfWinLossSWZoneToOutWinRep(surfNum) =
                            state.dataSurface->SurfWinLossSWZoneToOutWinRep(repSurfNum) * areaRatio;
                    }
                }
            }
        }
    }
}

void ReportIntMovInsInsideSurfTemp(EnergyPlusData &state)
{
    state.dataHeatBalSurf->SurfTempInMovInsRep = state.dataHeatBalSurf->SurfTempIn;
    for (int SurfNum : state.dataSurface->intMovInsulSurfNums) {
        if (state.dataSurface->intMovInsuls(SurfNum).present) {
            state.dataHeatBalSurf->SurfTempInMovInsRep(SurfNum) = state.dataHeatBalSurf->SurfTempInTmp(SurfNum);
        }
    }
}
// End of Reporting subroutines for the HB Module
// *****************************************************************************

// *****************************************************************************
// *****************************************************************************
// *****************************************************************************
// *****************************************************************************

// Formerly EXTERNAL SUBROUTINES (heavily related to HeatBalanceSurfaceManager) now moved into namespace

void CalcHeatBalanceOutsideSurf(EnergyPlusData &state,
                                ObjexxFCL::Optional_int_const ZoneToResimulate) // if passed in, then only calculate surfaces that have this zone
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         George Walton
    //       DATE WRITTEN   December 1979
    //       MODIFIED       Jun 1990 (RDT for new CTF arrays);
    //                      Aug 2000 (RJL for MTF moisture calculations)
    //                      Sep 2000 (RKS for new radiant exchange algorithm)
    //                      Dec 2000 (RKS for radiant system model addition)
    //                      Apr 2002 (COP removed denominator from OSC calculation
    //                      Jul 2008 (P.Biddulph include calls to HAMT)
    //                      Jul 2011, M.J. Witte and C.O. Pedersen, add new fields to OSC for last T, max and min
    //                      Sep 2011 LKL/BG - resimulate only zones needing it for Radiant systems
    //       RE-ENGINEERED  Mar 1998 (RKS)

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine performs a heat balance on the outside face of each
    // surface in the building.

    // METHODOLOGY EMPLOYED:
    // Various boundary conditions are set and additional parameters are set-
    // up.  Then, the proper heat balance equation is selected based on the
    // presence of movable insulation, thermal mass of the surface construction,
    // and convection model being used.

    // REFERENCES:
    // (I)BLAST legacy routine HBOUT
    // 1989 ASHRAE Handbook of Fundamentals (Figure 1 on p. 22.4, convection correlations)

    //    // Using/Aliasing
    //    using namespace DataEnvironment;
    //    using namespace DataHeatBalance;
    //    using namespace DataHeatBalSurface;
    //    using namespace DataSurfaces;
    //    using HeatBalanceIntRadExchange::CalcInteriorRadExchange;
    //    using namespace Psychrometrics;
    //    using EcoRoofManager::CalcEcoRoof;
    //
    //    // Locals
    //    // SUBROUTINE ARGUMENT DEFINITIONS:
    //
    //>>>>>>> origin/develop
    // SUBROUTINE PARAMETER DEFINITIONS:
    constexpr std::string_view RoutineNameGroundTemp("CalcHeatBalanceOutsideSurf:GroundTemp");
    constexpr std::string_view RoutineNameGroundTempFC("CalcHeatBalanceOutsideSurf:GroundTempFC");
    constexpr std::string_view RoutineNameOtherSideCoefNoCalcExt("CalcHeatBalanceOutsideSurf:OtherSideCoefNoCalcExt");
    constexpr std::string_view RoutineNameOtherSideCoefCalcExt("CalcHeatBalanceOutsideSurf:OtherSideCoefCalcExt");
    constexpr std::string_view RoutineNameOSCM("CalcHeatBalanceOutsideSurf:OSCM");
    constexpr std::string_view RoutineNameExtEnvWetSurf("CalcHeatBalanceOutsideSurf:extEnvWetSurf");
    constexpr std::string_view RoutineNameExtEnvDrySurf("CalcHeatBalanceOutsideSurf:extEnvDrySurf");
    constexpr std::string_view RoutineNameNoWind("CalcHeatBalanceOutsideSurf:nowind");
    constexpr std::string_view RoutineNameOther("CalcHeatBalanceOutsideSurf:interior/other");
    constexpr std::string_view RoutineNameIZPart("CalcHeatBalanceOutsideSurf:IZPart");
    constexpr std::string_view HBSurfManGroundHAMT("HBSurfMan:Ground:HAMT");
    constexpr std::string_view HBSurfManRainHAMT("HBSurfMan:Rain:HAMT");
    constexpr std::string_view HBSurfManDrySurfCondFD("HBSurfMan:DrySurf:CondFD");
    constexpr std::string_view Outside("Outside");

    auto &s_mat = state.dataMaterial;

    bool MovInsulErrorFlag = false; // Movable Insulation error flag

    // set ground surfaces average temperature
    GetGroundSurfacesTemperatureAverage(state);

    // set surrounding surfaces average temperature
    GetSurroundingSurfacesTemperatureAverage(state);

    auto &Surface = state.dataSurface->Surface;

    if (state.dataHeatBal->AnyInternalHeatSourceInInput) {
        for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
            // Need to transfer any source/sink for a surface to the local array.  Note that
            // the local array is flux (W/m2) while the QRadSysSource is heat transfer (W).
            // This must be done at this location so that this is always updated correctly.
            if (Surface(SurfNum).Area > 0.0) {
                state.dataHeatBalSurf->SurfQsrcHist(SurfNum, 1) =
                    state.dataHeatBalFanSys->QRadSysSource(SurfNum) / Surface(SurfNum).Area; // Make sure we don't divide by zero...
            }

            // next we add source (actually a sink) from any integrated PV
            if (Surface(SurfNum).Area > 0.0) {
                state.dataHeatBalSurf->SurfQsrcHist(SurfNum, 1) +=
                    state.dataHeatBalFanSys->QPVSysSource(SurfNum) / Surface(SurfNum).Area; // Make sure we don't divide by zero...
            }
        }
    }

    if (present(ZoneToResimulate)) {
        HeatBalanceIntRadExchange::CalcInteriorRadExchange(
            state, state.dataHeatBalSurf->SurfInsideTempHist(1), 0, state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea, ZoneToResimulate, Outside);
    } else {
        HeatBalanceIntRadExchange::CalcInteriorRadExchange(
            state, state.dataHeatBalSurf->SurfInsideTempHist(1), 0, state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea, _, Outside);
    }

    // Calculate heat extract due to additional heat flux source term as the surface boundary condition
    for (int surfNum : state.dataSurface->allOutsideSourceSurfaceList) {
        state.dataHeatBalSurf->SurfQAdditionalHeatSourceOutside(surfNum) = Surface(surfNum).outsideHeatSourceTermSched->getCurrentVal();
    }
    for (int zoneNum = 1; zoneNum <= state.dataGlobal->NumOfZones; ++zoneNum) { // Loop through all surfaces...
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto &thisSpace = state.dataHeatBal->space(spaceNum);
            for (int SurfNum = thisSpace.HTSurfaceFirst; SurfNum <= thisSpace.HTSurfaceLast; ++SurfNum) {
                if (Surface(SurfNum).Class == DataSurfaces::SurfaceClass::Window) {
                    continue;
                }
                if (present(ZoneToResimulate)) {
                    if ((zoneNum != ZoneToResimulate) && (state.dataSurface->SurfAdjacentZone(SurfNum) != ZoneToResimulate)) {
                        continue; // skip surfaces that are not associated with this zone
                    }
                }
                // Interior windows in partitions use "normal" heat balance calculations
                // For rest, Outside surface temp of windows not needed in Window5 calculation approach.
                // Window layer temperatures are calculated in CalcHeatBalanceInsideSurf

                // Initializations for this surface
                int const ConstrNum = state.dataSurface->SurfActiveConstruction(SurfNum);
                auto const &thisConstruct = state.dataConstruction->Construct(ConstrNum);
                Real64 HMovInsul = 0.0; // "Convection" coefficient of movable insulation
                Real64 HSky = 0.0;      // "Convection" coefficient from sky to surface
                Real64 HGround = 0.0;   // "Convection" coefficient from ground to surface
                Real64 HAir = 0.0;      // "Convection" coefficient from air to surface (radiation)
                Real64 HSurrr = 0.0;    // "Linearized radiation" coefficient from surrounding surfaces to surface
                state.dataHeatBalSurf->SurfHConvExt(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfHAirExt(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfHSkyExt(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfHGrdExt(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfQRadLWOutSrdSurfs(SurfNum) = 0.0;
                state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum) = 0.0;

                // Calculate the current outside surface temperature TH(SurfNum,1,1) for the
                // various different boundary conditions
                switch (Surface(SurfNum).ExtBoundCond) {
                case DataSurfaces::Ground: { // Surface in contact with ground
                    state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum) =
                        state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::BuildingSurface];

                    // Set the only radiant system heat balance coefficient that is non-zero for this case
                    if (thisConstruct.SourceSinkPresent) {
                        state.dataHeatBalFanSys->RadSysToHBConstCoef(SurfNum) = state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum);
                    }

                    // start HAMT
                    if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::HAMT) {
                        // Set variables used in the HAMT moisture balance
                        state.dataMstBal->TempOutsideAirFD(SurfNum) =
                            state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::BuildingSurface];
                        state.dataMstBal->RhoVaporAirOut(SurfNum) = Psychrometrics::PsyRhovFnTdbRh(
                            state, state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::BuildingSurface], 1.0, HBSurfManGroundHAMT);
                        state.dataMstBal->HConvExtFD(SurfNum) = state.dataHeatBal->HighHConvLimit;

                        state.dataMstBal->HMassConvExtFD(SurfNum) =
                            state.dataMstBal->HConvExtFD(SurfNum) /
                            ((Psychrometrics::PsyRhoAirFnPbTdbW(
                                  state,
                                  state.dataEnvrn->OutBaroPress,
                                  state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::BuildingSurface],
                                  Psychrometrics::PsyWFnTdbRhPb(state,
                                                                state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::BuildingSurface],
                                                                1.0,
                                                                state.dataEnvrn->OutBaroPress,
                                                                RoutineNameGroundTemp)) +
                              state.dataMstBal->RhoVaporAirOut(SurfNum)) *
                             Psychrometrics::PsyCpAirFnW(state.dataEnvrn->OutHumRat));

                        state.dataMstBal->HSkyFD(SurfNum) = HSky;
                        state.dataMstBal->HGrndFD(SurfNum) = HGround;
                        state.dataMstBal->HAirFD(SurfNum) = HAir;
                    }
                    // end HAMT

                    if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD) {
                        // Set variables used in the FD moisture balance
                        state.dataMstBal->TempOutsideAirFD(SurfNum) =
                            state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::BuildingSurface];
                        state.dataMstBal->RhoVaporAirOut(SurfNum) = Psychrometrics::PsyRhovFnTdbRhLBnd0C(
                            state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::BuildingSurface], 1.0);
                        state.dataMstBal->HConvExtFD(SurfNum) = state.dataHeatBal->HighHConvLimit;
                        state.dataMstBal->HMassConvExtFD(SurfNum) =
                            state.dataMstBal->HConvExtFD(SurfNum) /
                            ((Psychrometrics::PsyRhoAirFnPbTdbW(
                                  state,
                                  state.dataEnvrn->OutBaroPress,
                                  state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::BuildingSurface],
                                  Psychrometrics::PsyWFnTdbRhPb(state,
                                                                state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::BuildingSurface],
                                                                1.0,
                                                                state.dataEnvrn->OutBaroPress,
                                                                RoutineNameGroundTemp)) +
                              state.dataMstBal->RhoVaporAirOut(SurfNum)) *
                             Psychrometrics::PsyCpAirFnW(state.dataEnvrn->OutHumRat));
                        state.dataMstBal->HSkyFD(SurfNum) = HSky;
                        state.dataMstBal->HGrndFD(SurfNum) = HGround;
                        state.dataMstBal->HAirFD(SurfNum) = HAir;
                    }
                    // Added for FCfactor grounds
                } break;
                case DataSurfaces::GroundFCfactorMethod: { // Surface in contact with ground
                    state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum) =
                        state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::FCFactorMethod];

                    // Set the only radiant system heat balance coefficient that is non-zero for this case
                    if (thisConstruct.SourceSinkPresent) {
                        state.dataHeatBalFanSys->RadSysToHBConstCoef(SurfNum) = state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum);
                    }

                    if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::HAMT) {
                        // Set variables used in the HAMT moisture balance
                        state.dataMstBal->TempOutsideAirFD(SurfNum) =
                            state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::FCFactorMethod];
                        state.dataMstBal->RhoVaporAirOut(SurfNum) = Psychrometrics::PsyRhovFnTdbRh(
                            state, state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::FCFactorMethod], 1.0, HBSurfManGroundHAMT);
                        state.dataMstBal->HConvExtFD(SurfNum) = state.dataHeatBal->HighHConvLimit;

                        state.dataMstBal->HMassConvExtFD(SurfNum) =
                            state.dataMstBal->HConvExtFD(SurfNum) /
                            ((Psychrometrics::PsyRhoAirFnPbTdbW(
                                  state,
                                  state.dataEnvrn->OutBaroPress,
                                  state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::FCFactorMethod],
                                  Psychrometrics::PsyWFnTdbRhPb(state,
                                                                state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::FCFactorMethod],
                                                                1.0,
                                                                state.dataEnvrn->OutBaroPress,
                                                                RoutineNameGroundTempFC)) +
                              state.dataMstBal->RhoVaporAirOut(SurfNum)) *
                             Psychrometrics::PsyCpAirFnW(state.dataEnvrn->OutHumRat));

                        state.dataMstBal->HSkyFD(SurfNum) = HSky;
                        state.dataMstBal->HGrndFD(SurfNum) = HGround;
                        state.dataMstBal->HAirFD(SurfNum) = HAir;
                    }

                    if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD) {
                        // Set variables used in the FD moisture balance
                        state.dataMstBal->TempOutsideAirFD(SurfNum) =
                            state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::FCFactorMethod];
                        state.dataMstBal->RhoVaporAirOut(SurfNum) = Psychrometrics::PsyRhovFnTdbRhLBnd0C(
                            state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::FCFactorMethod], 1.0);
                        state.dataMstBal->HConvExtFD(SurfNum) = state.dataHeatBal->HighHConvLimit;
                        state.dataMstBal->HMassConvExtFD(SurfNum) =
                            state.dataMstBal->HConvExtFD(SurfNum) /
                            ((Psychrometrics::PsyRhoAirFnPbTdbW(
                                  state,
                                  state.dataEnvrn->OutBaroPress,
                                  state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::FCFactorMethod],
                                  Psychrometrics::PsyWFnTdbRhPb(state,
                                                                state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::FCFactorMethod],
                                                                1.0,
                                                                state.dataEnvrn->OutBaroPress,
                                                                RoutineNameGroundTempFC)) +
                              state.dataMstBal->RhoVaporAirOut(SurfNum)) *
                             Psychrometrics::PsyCpAirFnW(state.dataEnvrn->OutHumRat));
                        state.dataMstBal->HSkyFD(SurfNum) = HSky;
                        state.dataMstBal->HGrndFD(SurfNum) = HGround;
                        state.dataMstBal->HAirFD(SurfNum) = HAir;
                    }
                } break;
                case DataSurfaces::OtherSideCoefNoCalcExt: {
                    // Use Other Side Coefficients to determine the surface film coefficient and
                    // the exterior boundary condition temperature

                    int OPtr = Surface(SurfNum).OSCPtr;
                    // Set surface temp from previous timestep
                    if (state.dataGlobal->BeginTimeStepFlag) {
                        state.dataSurface->OSC(OPtr).TOutsideSurfPast = state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum);
                    }

                    if (state.dataSurface->OSC(OPtr).constTempSched != nullptr) { // Determine outside temperature from schedule
                        state.dataSurface->OSC(OPtr).ConstTemp = state.dataSurface->OSC(OPtr).constTempSched->getCurrentVal();
                    }

                    //  Allow for modification of TemperatureCoefficient with unitary sine wave.
                    Real64 ConstantTempCoef; // Temperature Coefficient as input or modified using sine wave COP mod
                    if (state.dataSurface->OSC(OPtr).SinusoidalConstTempCoef) { // Sine wave C4
                        ConstantTempCoef = std::sin(2 * Constant::Pi * state.dataGlobal->CurrentTime / state.dataSurface->OSC(OPtr).SinusoidPeriod);
                    } else {
                        ConstantTempCoef = state.dataSurface->OSC(OPtr).ConstTempCoef;
                    }

                    state.dataSurface->OSC(OPtr).OSCTempCalc =
                        (state.dataSurface->OSC(OPtr).ZoneAirTempCoef * state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum).MAT +
                         state.dataSurface->OSC(OPtr).ExtDryBulbCoef * state.dataSurface->SurfOutDryBulbTemp(SurfNum) +
                         ConstantTempCoef * state.dataSurface->OSC(OPtr).ConstTemp +
                         state.dataSurface->OSC(OPtr).GroundTempCoef *
                             state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::BuildingSurface] +
                         state.dataSurface->OSC(OPtr).WindSpeedCoef * state.dataSurface->SurfOutWindSpeed(SurfNum) *
                             state.dataSurface->SurfOutDryBulbTemp(SurfNum) +
                         state.dataSurface->OSC(OPtr).TPreviousCoef * state.dataSurface->OSC(OPtr).TOutsideSurfPast);

                    // Enforce max/min limits if applicable
                    if (state.dataSurface->OSC(OPtr).MinLimitPresent) {
                        state.dataSurface->OSC(OPtr).OSCTempCalc =
                            max(state.dataSurface->OSC(OPtr).MinTempLimit, state.dataSurface->OSC(OPtr).OSCTempCalc);
                    }
                    if (state.dataSurface->OSC(OPtr).MaxLimitPresent) {
                        state.dataSurface->OSC(OPtr).OSCTempCalc =
                            min(state.dataSurface->OSC(OPtr).MaxTempLimit, state.dataSurface->OSC(OPtr).OSCTempCalc);
                    }

                    state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum) = state.dataSurface->OSC(OPtr).OSCTempCalc;

                    // Set the only radiant system heat balance coefficient that is non-zero for this case
                    if (thisConstruct.SourceSinkPresent) {
                        state.dataHeatBalFanSys->RadSysToHBConstCoef(SurfNum) = state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum);
                    }

                    if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD ||
                        Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::HAMT) {
                        // Set variables used in the FD moisture balance and HAMT
                        state.dataMstBal->TempOutsideAirFD(SurfNum) = state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum);
                        state.dataMstBal->RhoVaporAirOut(SurfNum) = Psychrometrics::PsyRhovFnTdbWPb(
                            state.dataMstBal->TempOutsideAirFD(SurfNum), state.dataEnvrn->OutHumRat, state.dataEnvrn->OutBaroPress);
                        state.dataMstBal->HConvExtFD(SurfNum) = state.dataHeatBal->HighHConvLimit;
                        state.dataMstBal->HMassConvExtFD(SurfNum) =
                            state.dataMstBal->HConvExtFD(SurfNum) /
                            ((Psychrometrics::PsyRhoAirFnPbTdbW(state,
                                                                state.dataEnvrn->OutBaroPress,
                                                                state.dataMstBal->TempOutsideAirFD(SurfNum),
                                                                Psychrometrics::PsyWFnTdbRhPb(state,
                                                                                              state.dataMstBal->TempOutsideAirFD(SurfNum),
                                                                                              1.0,
                                                                                              state.dataEnvrn->OutBaroPress,
                                                                                              RoutineNameOtherSideCoefNoCalcExt)) +
                              state.dataMstBal->RhoVaporAirOut(SurfNum)) *
                             Psychrometrics::PsyCpAirFnW(state.dataEnvrn->OutHumRat));
                        state.dataMstBal->HSkyFD(SurfNum) = HSky;
                        state.dataMstBal->HGrndFD(SurfNum) = HGround;
                        state.dataMstBal->HAirFD(SurfNum) = HAir;
                    }
                    // This ends the calculations for this surface and goes on to the next SurfNum
                } break;
                case DataSurfaces::OtherSideCoefCalcExt: { // A surface with other side coefficients that define the outside environment
                    // First, set up the outside convection coefficient and the exterior temperature
                    // boundary condition for the surface
                    int OPtr = Surface(SurfNum).OSCPtr;
                    // Set surface temp from previous timestep
                    if (state.dataGlobal->BeginTimeStepFlag) {
                        state.dataSurface->OSC(OPtr).TOutsideSurfPast = state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum);
                    }

                    if (state.dataSurface->OSC(OPtr).constTempSched != nullptr) { // Determine outside temperature from schedule
                        state.dataSurface->OSC(OPtr).ConstTemp = state.dataSurface->OSC(OPtr).constTempSched->getCurrentVal();
                    }

                    state.dataHeatBalSurf->SurfHConvExt(SurfNum) = state.dataSurface->OSC(OPtr).SurfFilmCoef;

                    state.dataSurface->OSC(OPtr).OSCTempCalc =
                        (state.dataSurface->OSC(OPtr).ZoneAirTempCoef * state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum).MAT +
                         state.dataSurface->OSC(OPtr).ExtDryBulbCoef * state.dataSurface->SurfOutDryBulbTemp(SurfNum) +
                         state.dataSurface->OSC(OPtr).ConstTempCoef * state.dataSurface->OSC(OPtr).ConstTemp +
                         state.dataSurface->OSC(OPtr).GroundTempCoef *
                             state.dataEnvrn->GroundTemp[(int)DataEnvironment::GroundTempType::BuildingSurface] +
                         state.dataSurface->OSC(OPtr).WindSpeedCoef * state.dataSurface->SurfOutWindSpeed(SurfNum) *
                             state.dataSurface->SurfOutDryBulbTemp(SurfNum) +
                         state.dataSurface->OSC(OPtr).TPreviousCoef * state.dataSurface->OSC(OPtr).TOutsideSurfPast);

                    // Enforce max/min limits if applicable
                    if (state.dataSurface->OSC(OPtr).MinLimitPresent) {
                        state.dataSurface->OSC(OPtr).OSCTempCalc =
                            max(state.dataSurface->OSC(OPtr).MinTempLimit, state.dataSurface->OSC(OPtr).OSCTempCalc);
                    }
                    if (state.dataSurface->OSC(OPtr).MaxLimitPresent) {
                        state.dataSurface->OSC(OPtr).OSCTempCalc =
                            min(state.dataSurface->OSC(OPtr).MaxTempLimit, state.dataSurface->OSC(OPtr).OSCTempCalc);
                    }

                    Real64 TempExt = state.dataSurface->OSC(OPtr).OSCTempCalc;

                    // Set the only radiant system heat balance coefficient that is non-zero for this case
                    if (state.dataConstruction->Construct(ConstrNum).SourceSinkPresent) {
                        state.dataHeatBalFanSys->RadSysToHBConstCoef(SurfNum) = state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum);
                    }

                    if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD ||
                        Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::HAMT) {
                        // Set variables used in the FD moisture balance and HAMT
                        state.dataMstBal->TempOutsideAirFD(SurfNum) = TempExt;
                        state.dataMstBal->RhoVaporAirOut(SurfNum) = Psychrometrics::PsyRhovFnTdbWPb(
                            state.dataMstBal->TempOutsideAirFD(SurfNum), state.dataEnvrn->OutHumRat, state.dataEnvrn->OutBaroPress);
                        state.dataMstBal->HConvExtFD(SurfNum) = state.dataHeatBalSurf->SurfHConvExt(SurfNum);
                        state.dataMstBal->HMassConvExtFD(SurfNum) =
                            state.dataMstBal->HConvExtFD(SurfNum) /
                            ((Psychrometrics::PsyRhoAirFnPbTdbW(state,
                                                                state.dataEnvrn->OutBaroPress,
                                                                state.dataMstBal->TempOutsideAirFD(SurfNum),
                                                                Psychrometrics::PsyWFnTdbRhPb(state,
                                                                                              state.dataMstBal->TempOutsideAirFD(SurfNum),
                                                                                              1.0,
                                                                                              state.dataEnvrn->OutBaroPress,
                                                                                              RoutineNameOtherSideCoefCalcExt)) +
                              state.dataMstBal->RhoVaporAirOut(SurfNum)) *
                             Psychrometrics::PsyCpAirFnW(state.dataEnvrn->OutHumRat));
                        state.dataMstBal->HSkyFD(SurfNum) = state.dataHeatBalSurf->SurfHSkyExt(SurfNum);
                        state.dataMstBal->HGrndFD(SurfNum) = state.dataHeatBalSurf->SurfHGrdExt(SurfNum);
                        state.dataMstBal->HAirFD(SurfNum) = state.dataHeatBalSurf->SurfHAirExt(SurfNum);
                    }

                    // Call the outside surface temp calculation and pass the necessary terms
                    if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CTF ||
                        Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::EMPD) {
                        CalcOutsideSurfTemp(state, SurfNum, spaceNum, ConstrNum, HMovInsul, TempExt, MovInsulErrorFlag);
                        if (MovInsulErrorFlag) {
                            ShowFatalError(state, "CalcOutsideSurfTemp: Program terminates due to preceding conditions.");
                        }
                    }
                    // This ends the calculations for this surface and goes on to the next SurfNum
                } break;
                case DataSurfaces::OtherSideCondModeledExt: { // A surface with other side conditions determined from separate, dynamic component
                    // modeling that defines the "outside environment"
                    // First, set up the outside convection coefficient and the exterior temperature
                    // boundary condition for the surface
                    int OPtr = Surface(SurfNum).OSCMPtr;
                    // EMS overrides
                    if (state.dataSurface->OSCM(OPtr).EMSOverrideOnTConv) {
                        state.dataSurface->OSCM(OPtr).TConv = state.dataSurface->OSCM(OPtr).EMSOverrideTConvValue;
                    }
                    if (state.dataSurface->OSCM(OPtr).EMSOverrideOnHConv) {
                        state.dataSurface->OSCM(OPtr).HConv = state.dataSurface->OSCM(OPtr).EMSOverrideHConvValue;
                    }
                    if (state.dataSurface->OSCM(OPtr).EMSOverrideOnTRad) {
                        state.dataSurface->OSCM(OPtr).TRad = state.dataSurface->OSCM(OPtr).EMSOverrideTRadValue;
                    }
                    if (state.dataSurface->OSCM(OPtr).EMSOverrideOnHrad) {
                        state.dataSurface->OSCM(OPtr).HRad = state.dataSurface->OSCM(OPtr).EMSOverrideHradValue;
                    }
                    state.dataHeatBalSurf->SurfHConvExt(SurfNum) = state.dataSurface->OSCM(OPtr).HConv;

                    Real64 TempExt = state.dataSurface->OSCM(OPtr).TConv;

                    // Set the only radiant system heat balance coefficient that is non-zero for this case
                    if (state.dataConstruction->Construct(ConstrNum).SourceSinkPresent) {
                        state.dataHeatBalFanSys->RadSysToHBConstCoef(SurfNum) = state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum);
                    }

                    if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD ||
                        Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::HAMT) {
                        // Set variables used in the FD moisture balance and HAMT
                        state.dataMstBal->TempOutsideAirFD(SurfNum) = TempExt;
                        state.dataMstBal->RhoVaporAirOut(SurfNum) = Psychrometrics::PsyRhovFnTdbWPb(
                            state.dataMstBal->TempOutsideAirFD(SurfNum), state.dataEnvrn->OutHumRat, state.dataEnvrn->OutBaroPress);
                        state.dataMstBal->HConvExtFD(SurfNum) = state.dataHeatBalSurf->SurfHConvExt(SurfNum);
                        state.dataMstBal->HMassConvExtFD(SurfNum) =
                            state.dataMstBal->HConvExtFD(SurfNum) /
                            ((Psychrometrics::PsyRhoAirFnPbTdbW(
                                  state,
                                  state.dataEnvrn->OutBaroPress,
                                  state.dataMstBal->TempOutsideAirFD(SurfNum),
                                  Psychrometrics::PsyWFnTdbRhPb(
                                      state, state.dataMstBal->TempOutsideAirFD(SurfNum), 1.0, state.dataEnvrn->OutBaroPress, RoutineNameOSCM)) +
                              state.dataMstBal->RhoVaporAirOut(SurfNum)) *
                             Psychrometrics::PsyCpAirFnW(state.dataEnvrn->OutHumRat));
                        state.dataMstBal->HSkyFD(SurfNum) = state.dataSurface->OSCM(OPtr).HRad; // CR 8046, use sky term for surface to baffle IR
                        state.dataMstBal->HGrndFD(SurfNum) = 0.0; // CR 8046, null out and use only sky term for surface to baffle IR
                        state.dataMstBal->HAirFD(SurfNum) = 0.0;  // CR 8046, null out and use only sky term for surface to baffle IR
                    }

                    // Call the outside surface temp calculation and pass the necessary terms
                    if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CTF ||
                        Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::EMPD) {

                        if (state.dataSurface->SurfExtCavityPresent(SurfNum)) {
                            CalcExteriorVentedCavity(state, SurfNum);
                        }

                        CalcOutsideSurfTemp(state, SurfNum, spaceNum, ConstrNum, HMovInsul, TempExt, MovInsulErrorFlag);
                        if (MovInsulErrorFlag) {
                            ShowFatalError(state, "CalcOutsideSurfTemp: Program terminates due to preceding conditions.");
                        }

                    } else if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD ||
                               Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::HAMT) {
                        if (state.dataSurface->SurfExtCavityPresent(SurfNum)) {
                            CalcExteriorVentedCavity(state, SurfNum);
                        }
                    }
                    // This ends the calculations for this surface and goes on to the next SurfNum
                } break;
                case DataSurfaces::ExternalEnvironment: {
                    // checking the EcoRoof presented in the external environment
                    // recompute each load by calling ecoroof

                    Real64 TempExt;

                    if (state.dataSurface->SurfExtEcoRoof(SurfNum)) {
                        EcoRoofManager::CalcEcoRoof(state, SurfNum, ConstrNum, TempExt);
                        continue;
                    }
                    // Roughness index of the exterior surface
                    Material::SurfaceRoughness RoughSurf = state.dataHeatBalSurf->SurfRoughnessExt(SurfNum);
                    // Thermal absorptance of the exterior surface
                    Real64 AbsThermSurf = state.dataHeatBalSurf->SurfAbsThermalExt(SurfNum);
                    HMovInsul = 0;
                    // Check for outside movable insulation
                    if (state.dataSurface->AnyMovableInsulation && state.dataSurface->extMovInsuls(SurfNum).present) {
                        HMovInsul = state.dataSurface->extMovInsuls(SurfNum).H;
                    }

                    // Check for exposure to wind (exterior environment)
                    if (Surface(SurfNum).ExtWind) {

                        // Calculate exterior heat transfer coefficients with windspeed (windspeed is calculated internally in subroutine)
                        Convect::InitExtConvCoeff(state,
                                                  SurfNum,
                                                  HMovInsul,
                                                  RoughSurf,
                                                  AbsThermSurf,
                                                  state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum),
                                                  state.dataHeatBalSurf->SurfHConvExt(SurfNum),
                                                  state.dataHeatBalSurf->SurfHSkyExt(SurfNum),
                                                  state.dataHeatBalSurf->SurfHGrdExt(SurfNum),
                                                  state.dataHeatBalSurf->SurfHAirExt(SurfNum),
                                                  state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum));

                        if (state.dataEnvrn->IsRain) { // Raining: since wind exposed, outside surface gets wet

                            if (state.dataSurface->surfExtConv(SurfNum).userModelNum == 0) { // Reset SurfHcExt because of wetness
                                state.dataHeatBalSurf->SurfHConvExt(SurfNum) = 1000.0;
                            } else { // User set
                                state.dataHeatBalSurf->SurfHConvExt(SurfNum) = Convect::SetExtConvCoeff(state, SurfNum);
                            }

                            TempExt = state.dataSurface->SurfOutWetBulbTemp(SurfNum);

                            // start HAMT
                            if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::HAMT) {
                                // Set variables used in the HAMT moisture balance
                                state.dataMstBal->TempOutsideAirFD(SurfNum) = TempExt;
                                state.dataMstBal->RhoVaporAirOut(SurfNum) =
                                    Psychrometrics::PsyRhovFnTdbRh(state, state.dataMstBal->TempOutsideAirFD(SurfNum), 1.0, HBSurfManRainHAMT);
                                state.dataMstBal->HConvExtFD(SurfNum) = state.dataHeatBalSurf->SurfHConvExt(SurfNum);
                                state.dataMstBal->HMassConvExtFD(SurfNum) =
                                    state.dataMstBal->HConvExtFD(SurfNum) /
                                    ((Psychrometrics::PsyRhoAirFnPbTdbW(state,
                                                                        state.dataEnvrn->OutBaroPress,
                                                                        state.dataMstBal->TempOutsideAirFD(SurfNum),
                                                                        Psychrometrics::PsyWFnTdbRhPb(state,
                                                                                                      state.dataMstBal->TempOutsideAirFD(SurfNum),
                                                                                                      1.0,
                                                                                                      state.dataEnvrn->OutBaroPress,
                                                                                                      RoutineNameExtEnvWetSurf)) +
                                      state.dataMstBal->RhoVaporAirOut(SurfNum)) *
                                     Psychrometrics::PsyCpAirFnW(state.dataEnvrn->OutHumRat));
                                state.dataMstBal->HSkyFD(SurfNum) = state.dataHeatBalSurf->SurfHSkyExt(SurfNum);
                                state.dataMstBal->HGrndFD(SurfNum) = state.dataHeatBalSurf->SurfHGrdExt(SurfNum);
                                state.dataMstBal->HAirFD(SurfNum) = state.dataHeatBalSurf->SurfHAirExt(SurfNum);
                            }
                            // end HAMT
                            if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD) {
                                // Set variables used in the FD moisture balance
                                state.dataMstBal->TempOutsideAirFD(SurfNum) = TempExt;
                                state.dataMstBal->RhoVaporAirOut(SurfNum) =
                                    Psychrometrics::PsyRhovFnTdbRhLBnd0C(state.dataMstBal->TempOutsideAirFD(SurfNum), 1.0);
                                state.dataMstBal->HConvExtFD(SurfNum) = state.dataHeatBalSurf->SurfHConvExt(SurfNum);
                                state.dataMstBal->HMassConvExtFD(SurfNum) =
                                    state.dataMstBal->HConvExtFD(SurfNum) /
                                    ((Psychrometrics::PsyRhoAirFnPbTdbW(state,
                                                                        state.dataEnvrn->OutBaroPress,
                                                                        state.dataMstBal->TempOutsideAirFD(SurfNum),
                                                                        Psychrometrics::PsyWFnTdbRhPb(state,
                                                                                                      state.dataMstBal->TempOutsideAirFD(SurfNum),
                                                                                                      1.0,
                                                                                                      state.dataEnvrn->OutBaroPress,
                                                                                                      RoutineNameExtEnvWetSurf)) +
                                      state.dataMstBal->RhoVaporAirOut(SurfNum)) *
                                     Psychrometrics::PsyCpAirFnW(state.dataEnvrn->OutHumRat));
                                state.dataMstBal->HSkyFD(SurfNum) = state.dataHeatBalSurf->SurfHSkyExt(SurfNum);
                                state.dataMstBal->HGrndFD(SurfNum) = state.dataHeatBalSurf->SurfHGrdExt(SurfNum);
                                state.dataMstBal->HAirFD(SurfNum) = state.dataHeatBalSurf->SurfHAirExt(SurfNum);
                                state.dataMstBal->HSurrFD(SurfNum) = state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum);
                            }

                        } else { // Surface is dry, use the normal correlation

                            TempExt = state.dataSurface->SurfOutDryBulbTemp(SurfNum);

                            if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD ||
                                Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::HAMT) {
                                // Set variables used in the FD moisture balance and HAMT
                                state.dataMstBal->TempOutsideAirFD(SurfNum) = TempExt;
                                state.dataMstBal->RhoVaporAirOut(SurfNum) = Psychrometrics::PsyRhovFnTdbWPb(
                                    state.dataMstBal->TempOutsideAirFD(SurfNum), state.dataEnvrn->OutHumRat, state.dataEnvrn->OutBaroPress);
                                state.dataMstBal->HConvExtFD(SurfNum) = state.dataHeatBalSurf->SurfHConvExt(SurfNum);
                                state.dataMstBal->HMassConvExtFD(SurfNum) =
                                    state.dataMstBal->HConvExtFD(SurfNum) /
                                    ((Psychrometrics::PsyRhoAirFnPbTdbW(state,
                                                                        state.dataEnvrn->OutBaroPress,
                                                                        state.dataMstBal->TempOutsideAirFD(SurfNum),
                                                                        Psychrometrics::PsyWFnTdbRhPb(state,
                                                                                                      state.dataMstBal->TempOutsideAirFD(SurfNum),
                                                                                                      1.0,
                                                                                                      state.dataEnvrn->OutBaroPress,
                                                                                                      RoutineNameExtEnvDrySurf)) +
                                      state.dataMstBal->RhoVaporAirOut(SurfNum)) *
                                     Psychrometrics::PsyCpAirFnW(state.dataEnvrn->OutHumRat));
                                //  check for saturation conditions of air
                                // Local temporary saturated vapor density for checking
                                Real64 RhoVaporSat =
                                    Psychrometrics::PsyRhovFnTdbRh(state, state.dataMstBal->TempOutsideAirFD(SurfNum), 1.0, HBSurfManDrySurfCondFD);
                                if (state.dataMstBal->RhoVaporAirOut(SurfNum) > RhoVaporSat) {
                                    state.dataMstBal->RhoVaporAirOut(SurfNum) = RhoVaporSat;
                                }
                                state.dataMstBal->HSkyFD(SurfNum) = state.dataHeatBalSurf->SurfHSkyExt(SurfNum);
                                state.dataMstBal->HGrndFD(SurfNum) = state.dataHeatBalSurf->SurfHGrdExt(SurfNum);
                                state.dataMstBal->HAirFD(SurfNum) = state.dataHeatBalSurf->SurfHAirExt(SurfNum);
                                state.dataMstBal->HSurrFD(SurfNum) = state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum);
                            }
                        }

                    } else { // No wind

                        // Calculate exterior heat transfer coefficients for windspeed = 0
                        Convect::InitExtConvCoeff(state,
                                                  SurfNum,
                                                  HMovInsul,
                                                  RoughSurf,
                                                  AbsThermSurf,
                                                  state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum),
                                                  state.dataHeatBalSurf->SurfHConvExt(SurfNum),
                                                  state.dataHeatBalSurf->SurfHSkyExt(SurfNum),
                                                  state.dataHeatBalSurf->SurfHGrdExt(SurfNum),
                                                  state.dataHeatBalSurf->SurfHAirExt(SurfNum),
                                                  state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum));

                        TempExt = state.dataSurface->SurfOutDryBulbTemp(SurfNum);

                        if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD ||
                            Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::HAMT) {
                            // Set variables used in the FD moisture balance and HAMT
                            state.dataMstBal->TempOutsideAirFD(SurfNum) = TempExt;
                            state.dataMstBal->RhoVaporAirOut(SurfNum) = Psychrometrics::PsyRhovFnTdbWPb(
                                state.dataMstBal->TempOutsideAirFD(SurfNum), state.dataEnvrn->OutHumRat, state.dataEnvrn->OutBaroPress);
                            state.dataMstBal->HConvExtFD(SurfNum) = state.dataHeatBalSurf->SurfHConvExt(SurfNum);
                            state.dataMstBal->HMassConvExtFD(SurfNum) =
                                state.dataMstBal->HConvExtFD(SurfNum) /
                                ((Psychrometrics::PsyRhoAirFnPbTdbW(state,
                                                                    state.dataEnvrn->OutBaroPress,
                                                                    state.dataMstBal->TempOutsideAirFD(SurfNum),
                                                                    Psychrometrics::PsyWFnTdbRhPb(state,
                                                                                                  state.dataMstBal->TempOutsideAirFD(SurfNum),
                                                                                                  1.0,
                                                                                                  state.dataEnvrn->OutBaroPress,
                                                                                                  RoutineNameNoWind)) +
                                  state.dataMstBal->RhoVaporAirOut(SurfNum)) *
                                 Psychrometrics::PsyCpAirFnW(state.dataEnvrn->OutHumRat));
                            state.dataMstBal->HSkyFD(SurfNum) = state.dataHeatBalSurf->SurfHSkyExt(SurfNum);
                            state.dataMstBal->HGrndFD(SurfNum) = state.dataHeatBalSurf->SurfHGrdExt(SurfNum);
                            state.dataMstBal->HAirFD(SurfNum) = state.dataHeatBalSurf->SurfHAirExt(SurfNum);
                            state.dataMstBal->HSurrFD(SurfNum) = state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum);
                        }
                    }
                    // Calculate LWR from surrounding surfaces if defined for an exterior surface
                    if (state.dataSurface->Surface(SurfNum).SurfHasSurroundingSurfProperty) {
                        int SrdSurfsNum = state.dataSurface->Surface(SurfNum).SurfSurroundingSurfacesNum;
                        // Absolute temperature of the outside surface of an exterior surface
                        Real64 TSurf = state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum) + Constant::Kelvin;
                        for (int SrdSurfNum = 1; SrdSurfNum <= state.dataSurface->SurroundingSurfsProperty(SrdSurfsNum).TotSurroundingSurface;
                             SrdSurfNum++) {
                            // View factor of a surrounding surface
                            Real64 SrdSurfViewFac = state.dataSurface->SurroundingSurfsProperty(SrdSurfsNum).SurroundingSurfs(SrdSurfNum).ViewFactor;
                            // Absolute temperature of a surrounding surface
                            Real64 SrdSurfTempAbs =
                                state.dataSurface->SurroundingSurfsProperty(SrdSurfsNum).SurroundingSurfs(SrdSurfNum).tempSched->getCurrentVal() +
                                Constant::Kelvin;
                            state.dataHeatBalSurf->SurfQRadLWOutSrdSurfs(SurfNum) +=
                                Constant::StefanBoltzmann * AbsThermSurf * SrdSurfViewFac * (pow_4(SrdSurfTempAbs) - pow_4(TSurf));
                        }
                    }

                    if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CTF ||
                        Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::EMPD ||
                        Surface(SurfNum).Class == DataSurfaces::SurfaceClass::TDD_Dome) {
                        CalcOutsideSurfTemp(state, SurfNum, spaceNum, ConstrNum, HMovInsul, TempExt, MovInsulErrorFlag);
                        if (MovInsulErrorFlag) {
                            ShowFatalError(state, "CalcOutsideSurfTemp: Program terminates due to preceding conditions.");
                        }
                    }
                } break;

                case DataSurfaces::KivaFoundation: {
                    auto const *thisMaterial = s_mat->materials(state.dataConstruction->Construct(ConstrNum).LayerPoint(1));
                    Material::SurfaceRoughness RoughSurf = thisMaterial->Roughness;
                    Real64 AbsThermSurf = thisMaterial->AbsorpThermal;

                    // Set Kiva exterior convection algorithms
                    Convect::InitExtConvCoeff(state,
                                              SurfNum,
                                              HMovInsul,
                                              RoughSurf,
                                              AbsThermSurf,
                                              state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum),
                                              state.dataHeatBalSurf->SurfHConvExt(SurfNum),
                                              state.dataHeatBalSurf->SurfHSkyExt(SurfNum),
                                              state.dataHeatBalSurf->SurfHGrdExt(SurfNum),
                                              state.dataHeatBalSurf->SurfHAirExt(SurfNum),
                                              state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum));
                } break;

                default: { // for interior or other zone surfaces

                    if (Surface(SurfNum).ExtBoundCond == SurfNum) { // Regular partition/internal mass

                        state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum) = state.dataHeatBalSurf->SurfTempIn(SurfNum);

                        // No need to set any radiant system heat balance coefficients here--will be done during inside heat balance

                        if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD ||
                            Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::HAMT) {
                            // Set variables used in the FD moisture balance HAMT
                            state.dataMstBal->TempOutsideAirFD(SurfNum) = state.dataHeatBalSurf->SurfTempIn(SurfNum);
                            state.dataMstBal->RhoVaporAirOut(SurfNum) = state.dataMstBal->RhoVaporAirIn(SurfNum);
                            state.dataMstBal->HConvExtFD(SurfNum) = state.dataHeatBalSurf->SurfHConvInt(SurfNum);
                            state.dataMstBal->HMassConvExtFD(SurfNum) =
                                state.dataMstBal->HConvExtFD(SurfNum) /
                                ((Psychrometrics::PsyRhoAirFnPbTdbW(
                                      state,
                                      state.dataEnvrn->OutBaroPress,
                                      state.dataMstBal->TempOutsideAirFD(SurfNum),
                                      Psychrometrics::PsyWFnTdbRhPb(
                                          state, state.dataMstBal->TempOutsideAirFD(SurfNum), 1.0, state.dataEnvrn->OutBaroPress, RoutineNameOther)) +
                                  state.dataMstBal->RhoVaporAirOut(SurfNum)) *
                                 Psychrometrics::PsyCpAirFnW(state.dataEnvrn->OutHumRat));
                            state.dataMstBal->HSkyFD(SurfNum) = 0.0;
                            state.dataMstBal->HGrndFD(SurfNum) = 0.0;
                            state.dataMstBal->HAirFD(SurfNum) = 0.0;
                            state.dataMstBal->HSurrFD(SurfNum) = 0.0;
                        }

                    } else { // Interzone partition

                        state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum) =
                            state.dataHeatBalSurf->SurfInsideTempHist(1)(Surface(SurfNum).ExtBoundCond);

                        // No need to set any radiant system heat balance coefficients here--will be done during inside heat balance

                        if (Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD ||
                            Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::HAMT) {
                            // Set variables used in the FD moisture balance and HAMT
                            state.dataMstBal->TempOutsideAirFD(SurfNum) = state.dataHeatBalSurf->SurfInsideTempHist(1)(Surface(SurfNum).ExtBoundCond);
                            state.dataMstBal->RhoVaporAirOut(SurfNum) = state.dataMstBal->RhoVaporAirIn(Surface(SurfNum).ExtBoundCond);
                            state.dataMstBal->HConvExtFD(SurfNum) = state.dataHeatBalSurf->SurfHConvInt(Surface(SurfNum).ExtBoundCond);
                            state.dataMstBal->HMassConvExtFD(SurfNum) =
                                state.dataMstBal->HConvExtFD(SurfNum) /
                                ((Psychrometrics::PsyRhoAirFnPbTdbW(state,
                                                                    state.dataEnvrn->OutBaroPress,
                                                                    state.dataMstBal->TempOutsideAirFD(SurfNum),
                                                                    Psychrometrics::PsyWFnTdbRhPb(state,
                                                                                                  state.dataMstBal->TempOutsideAirFD(SurfNum),
                                                                                                  1.0,
                                                                                                  state.dataEnvrn->OutBaroPress,
                                                                                                  RoutineNameIZPart)) +
                                  state.dataMstBal->RhoVaporAirOut(SurfNum)) *
                                 Psychrometrics::PsyCpAirFnW(state.dataEnvrn->OutHumRat));
                            state.dataMstBal->HSkyFD(SurfNum) = 0.0;
                            state.dataMstBal->HGrndFD(SurfNum) = 0.0;
                            state.dataMstBal->HAirFD(SurfNum) = 0.0;
                            state.dataMstBal->HSurrFD(SurfNum) = 0.0;
                        }
                    }
                    // This ends the calculations for this surface and goes on to the next SurfNum
                } break;
                }

                state.dataHeatBalSurf->SurfQdotConvOutPerArea(SurfNum) = GetQdotConvOutPerArea(state, SurfNum);
            }
        }
    } // ...end of DO loop over all surface (actually heat transfer surfaces)
}

Real64 GetQdotConvOutPerArea(EnergyPlusData &state, int const SurfNum)
{
    auto const &surface = state.dataSurface->Surface(SurfNum);
    int OPtr = surface.OSCMPtr;
    if (surface.OSCMPtr > 0) { // Optr is set above in this case, use OSCM boundary data
        return -state.dataSurface->OSCM(OPtr).HConv * (state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum) - state.dataSurface->OSCM(OPtr).TConv);
    } else {
        if (state.dataEnvrn->IsRain) {
            return -state.dataHeatBalSurf->SurfHConvExt(SurfNum) *
                   (state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum) - state.dataSurface->SurfOutWetBulbTemp(SurfNum));
        } else {
            return -state.dataHeatBalSurf->SurfHConvExt(SurfNum) *
                   (state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum) - state.dataSurface->SurfOutDryBulbTemp(SurfNum));
        }
    }
}

void CalcHeatBalanceInsideSurf(EnergyPlusData &state,
                               ObjexxFCL::Optional_int_const ZoneToResimulate) // if passed in, then only calculate surfaces that have this zone
{
    if (state.dataHeatBalSurfMgr->calcHeatBalInsideSurfFirstTime) {
        if (state.dataHeatBal->AnyEMPD) {
            state.dataHeatBalSurf->MinIterations = DataHeatBalSurface::MinEMPDIterations;
        }
        if (state.dataGlobal->DisplayAdvancedReportVariables) {
            SetupOutputVariable(state,
                                "Surface Inside Face Heat Balance Calculation Iteration Count",
                                Constant::Units::None,
                                state.dataHeatBal->InsideSurfIterations,
                                OutputProcessor::TimeStepType::Zone,
                                OutputProcessor::StoreType::Sum,
                                "Simulation");
        }
        // Precompute whether CTF temperature limits will be needed
        state.dataHeatBalSurf->Zone_has_mixed_HT_models.resize(state.dataGlobal->NumOfZones + 1, false);
        for (int iZone = 1; iZone <= state.dataGlobal->NumOfZones; ++iZone) {
            for (int spaceNum : state.dataHeatBal->Zone(iZone).spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                for (int iSurf = thisSpace.HTSurfaceFirst, eSurf = thisSpace.HTSurfaceLast; iSurf <= eSurf; ++iSurf) {
                    DataSurfaces::HeatTransferModel const alg = state.dataSurface->Surface(iSurf).HeatTransferAlgorithm;
                    if ((alg == DataSurfaces::HeatTransferModel::CondFD) || (alg == DataSurfaces::HeatTransferModel::HAMT) ||
                        (alg == DataSurfaces::HeatTransferModel::Kiva)) {
                        state.dataHeatBalSurf->Zone_has_mixed_HT_models[iZone] = true;
                        break;
                    }
                }
            }
        }
        state.dataHeatBalSurfMgr->calcHeatBalInsideSurfFirstTime = false;
    }

    if (state.dataGlobal->BeginEnvrnFlag && state.dataHeatBalSurfMgr->calcHeatBalInsideSurEnvrnFlag) {
        state.dataHeatBalSurf->SurfTempInsOld = 23.0;
        state.dataHeatBalSurfMgr->RefAirTemp = 23.0;
        state.dataHeatBal->SurfTempEffBulkAir = 23.0;
        state.dataHeatBalSurfMgr->calcHeatBalInsideSurfWarmupErrCount = 0;
        state.dataHeatBalSurfMgr->calcHeatBalInsideSurEnvrnFlag = false;

        // Initialize Kiva instances ground temperatures
        if (state.dataHeatBal->AnyKiva) {
            state.dataSurfaceGeometry->kivaManager.initKivaInstances(state);
        }
    }
    if (!state.dataGlobal->BeginEnvrnFlag) {
        state.dataHeatBalSurfMgr->calcHeatBalInsideSurEnvrnFlag = true;
    }

    sumSurfQdotRadHVAC(state);

    // Pass correct list of surfaces to CalcHeatBalanceInsideSurf2
    bool const PartialResimulate(present(ZoneToResimulate));

    if (!PartialResimulate) {
        if (state.dataHeatBal->AllCTF) {
            CalcHeatBalanceInsideSurf2CTFOnly(state, 1, state.dataGlobal->NumOfZones, state.dataSurface->AllIZSurfaceList);
        } else {
            CalcHeatBalanceInsideSurf2(state,
                                       state.dataSurface->AllHTSurfaceList,
                                       state.dataSurface->AllIZSurfaceList,
                                       state.dataSurface->AllHTNonWindowSurfaceList,
                                       state.dataSurface->AllHTWindowSurfaceList);
        }
    } else {
        auto const &zoneHTSurfList = state.dataHeatBal->Zone(ZoneToResimulate).ZoneHTSurfaceList;
        auto const &zoneIZSurfList = state.dataHeatBal->Zone(ZoneToResimulate).ZoneIZSurfaceList;
        auto const &zoneHTNonWindowSurfList = state.dataHeatBal->Zone(ZoneToResimulate).ZoneHTNonWindowSurfaceList;
        auto const &zoneHTWindowSurfList = state.dataHeatBal->Zone(ZoneToResimulate).ZoneHTWindowSurfaceList;
        // Cannot use CalcHeatBalanceInsideSurf2CTFOnly because resimulated zone includes adjacent interzone surfaces
        CalcHeatBalanceInsideSurf2(state, zoneHTSurfList, zoneIZSurfList, zoneHTNonWindowSurfList, zoneHTWindowSurfList, ZoneToResimulate);
    }
    CalculateZoneMRT(state, ZoneToResimulate); // Update here so that the proper value of MRT is available to radiant systems
    UpdateIntermediateSurfaceHeatBalanceResults(state, ZoneToResimulate);
}

void CalcHeatBalanceInsideSurf2(EnergyPlusData &state,
                                const std::vector<int> &HTSurfs,          // Heat transfer surfaces to simulate (opaque and windows)
                                const std::vector<int> &IZSurfs,          // Interzone heat transfer surfaces to simulate
                                const std::vector<int> &HTNonWindowSurfs, // Non-window heat transfer surfaces to simulate
                                const std::vector<int> &HTWindowSurfs,    // Window heat transfer surfaces to simulate
                                ObjexxFCL::Optional_int_const ZoneToResimulate)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         George Walton
    //       DATE WRITTEN   December 1979
    //       MODIFIED       Jun 1990 (RDT for new CTF arrays)
    //                      Dec 1999 (FCW for window calculation)
    //                      May 2000 (FCW for window frame and dividers)
    //                      Aug 2000 (RJL for MTF moisture calculations)
    //                      Sep 2000 (RKS for new radiant exchange algorithm)
    //                      Dec 2000 (RKS for radiant system model addition)
    //                      Jul 2003 (CC) set the reference temperatures for inside surface heat balance
    //                                    depending on convection algorithms and/or air models used
    //                      May 2006 (RR  account for exterior window screen)
    //                      Jul 2008 (P. Biddulph include calls to HAMT)
    //                      Sep 2011 LKL/BG - resimulate only zones needing it for Radiant systems
    //       RE-ENGINEERED  Mar 1998 (RKS)

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine performs a heat balance on the inside face of each
    // surface in the building.

    // METHODOLOGY EMPLOYED:
    // Various boundary conditions are set and additional parameters are set-
    // up.  Then, the proper heat balance equation is selected based on whether
    // the surface is a partition or not and on whether or not movable
    // insulation is present on the inside face.

    // REFERENCES:
    // (I)BLAST legacy routine HBSRF

    constexpr std::string_view rhoAirZone("RhoAirZone");
    constexpr std::string_view wsurf("Wsurf");
    constexpr std::string_view HBSurfManInsideSurf("HB,SurfMan:InsideSurf");
    constexpr std::string_view Inside("Inside");

    Real64 TempSurfOutTmp; // Local Temporary Surface temperature for the outside surface face
    Real64 SurfTempInSat;  // Local temporary surface dew point temperature

    Real64 Wsurf;         // Moisture ratio for HAMT
    Real64 RhoAirZone;    // Zone moisture density for HAMT
    int OtherSideZoneNum; // Zone Number index for other side of an interzone partition HAMT

    auto &s_mat = state.dataMaterial;
    // determine reference air temperatures
    for (int SurfNum : HTSurfs) {

        // These conditions are not used in every SurfNum loop here so we don't use them to skip surfaces
        if (state.dataSurface->Surface(SurfNum).Class == DataSurfaces::SurfaceClass::TDD_Dome) {
            continue; // Skip TDD:DOME objects.  Inside temp is handled by TDD:DIFFUSER.
        }
        Real64 RefAirTemp = state.dataSurface->Surface(SurfNum).getInsideAirTemperature(state, SurfNum);
        state.dataHeatBalSurfMgr->RefAirTemp(SurfNum) = RefAirTemp;
        state.dataHeatBal->SurfTempEffBulkAir(SurfNum) = state.dataHeatBalSurfMgr->RefAirTemp(SurfNum);
    }

    // Following variables must be reset due to possible recall of this routine by radiant and Resimulate routines.
    // CalcWindowHeatBalance is called, then, multiple times and these need to be initialized before each call to
    // CalcWindowHeatBalance.
    // Only for Surface(SurfNum).Class == DataSurfaces::SurfaceClass::Window
    for (int surfNum : HTWindowSurfs) {
        state.dataSurface->SurfWinHeatGain(surfNum) = 0.0;
        state.dataSurface->SurfWinHeatGainRep(surfNum) = 0.0;
        state.dataSurface->SurfWinHeatLossRep(surfNum) = 0.0;
        state.dataSurface->SurfWinGainConvGlazToZoneRep(surfNum) = 0.0;
        state.dataSurface->SurfWinGainIRGlazToZoneRep(surfNum) = 0.0;
        state.dataSurface->SurfWinLossSWZoneToOutWinRep(surfNum) = 0.0;
        state.dataSurface->SurfWinGainFrameDividerToZoneRep(surfNum) = 0.0;
        state.dataSurface->SurfWinGainConvShadeToZoneRep(surfNum) = 0.0;
        state.dataSurface->SurfWinGainIRShadeToZoneRep(surfNum) = 0.0;
        state.dataSurface->SurfWinFrameQRadOutAbs(surfNum) = 0.0;
        state.dataSurface->SurfWinFrameQRadInAbs(surfNum) = 0.0;
        state.dataSurface->SurfWinDividerQRadOutAbs(surfNum) = 0.0;
        state.dataSurface->SurfWinDividerQRadInAbs(surfNum) = 0.0;
    }

    state.dataHeatBal->InsideSurfIterations = 0;

    // Calculate heat extract due to additional heat flux source term as the surface boundary condition
    for (int surfNum : state.dataSurface->allInsideSourceSurfaceList) {
        state.dataHeatBalSurf->SurfQAdditionalHeatSourceInside(surfNum) =
            state.dataSurface->Surface(surfNum).insideHeatSourceTermSched->getCurrentVal();
    }

    // Calculate Kiva instances
    if (state.dataHeatBal->AnyKiva) {
        if (((state.dataSurfaceGeometry->kivaManager.settings.timestepType == HeatBalanceKivaManager::KivaManager::Settings::HOURLY &&
              state.dataGlobal->TimeStep == 1) ||
             state.dataSurfaceGeometry->kivaManager.settings.timestepType == HeatBalanceKivaManager::KivaManager::Settings::TIMESTEP) &&
            !state.dataGlobal->WarmupFlag) {
            state.dataSurfaceGeometry->kivaManager.calcKivaInstances(state);
        }
    }

    bool Converged = false; // .TRUE. if inside heat balance has converged
    while (!Converged) {    // Start of main inside heat balance DO loop...

        state.dataHeatBalSurf->SurfTempInsOld = state.dataHeatBalSurf->SurfTempIn; // Keep track of last iteration's temperature values

        if (state.dataHeatBal->AnyKiva) {
            for (auto const &kivaSurf : state.dataSurfaceGeometry->kivaManager.surfaceMap) {
                state.dataHeatBalSurf->SurfTempIn(kivaSurf.first) = kivaSurf.second.results.Trad - Constant::Kelvin;
            }
        }

        HeatBalanceIntRadExchange::CalcInteriorRadExchange(state,
                                                           state.dataHeatBalSurf->SurfTempIn,
                                                           state.dataHeatBal->InsideSurfIterations,
                                                           state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea,
                                                           ZoneToResimulate,
                                                           Inside); // Update the radiation balance

        if (state.dataHeatBal->AnyKiva) {
            for (auto const &kivaSurf : state.dataSurfaceGeometry->kivaManager.surfaceMap) {
                state.dataHeatBalSurf->SurfTempIn(kivaSurf.first) = state.dataHeatBalSurf->SurfTempInsOld(kivaSurf.first);
            }
        }

        // Every 30 iterations, recalculate the inside convection coefficients in case
        // there has been a significant drift in the surface temperatures predicted.
        // This is not fool-proof and it basically means that the outside surface
        // heat balance is in error (potentially) once HConvIn is re-evaluated.
        // The choice of 30 is not significant--just want to do this a couple of
        // times before the iteration limit is hit.
        if ((state.dataHeatBal->InsideSurfIterations > 0) &&
            (mod(state.dataHeatBal->InsideSurfIterations, DataHeatBalSurface::ItersReevalConvCoeff) == 0)) {
            Convect::InitIntConvCoeff(state, state.dataHeatBalSurf->SurfTempIn, ZoneToResimulate);
        }

        if (state.dataHeatBal->AnyEMPD || state.dataHeatBal->AnyHAMT) {
            for (int SurfNum : HTSurfs) {
                auto const &surface = state.dataSurface->Surface(SurfNum);
                if (surface.Class == DataSurfaces::SurfaceClass::TDD_Dome) {
                    continue; // Skip TDD:DOME objects.  Inside temp is handled by TDD:DIFFUSER.
                }

                // Calculate the inside surface moisture quantities
                // calculate the inside surface moisture transfer conditions
                // check for saturation conditions of air
                if ((surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::EMPD) ||
                    (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::HAMT)) {
                    int ZoneNum = surface.Zone;
                    Real64 const MAT_zone(state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).MAT);
                    Real64 const ZoneAirHumRat_zone(max(state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).airHumRat, 1.0e-5));
                    Real64 const HConvIn_surf(state.dataMstBal->HConvInFD(SurfNum) = state.dataHeatBalSurf->SurfHConvInt(SurfNum));

                    state.dataMstBal->RhoVaporAirIn(SurfNum) =
                        min(Psychrometrics::PsyRhovFnTdbWPb_fast(MAT_zone, ZoneAirHumRat_zone, state.dataEnvrn->OutBaroPress),
                            Psychrometrics::PsyRhovFnTdbRh(state, MAT_zone, 1.0, HBSurfManInsideSurf));
                    state.dataMstBal->HMassConvInFD(SurfNum) =
                        HConvIn_surf / (Psychrometrics::PsyRhoAirFnPbTdbW_fast(state, state.dataEnvrn->OutBaroPress, MAT_zone, ZoneAirHumRat_zone) *
                                        Psychrometrics::PsyCpAirFnW_fast(ZoneAirHumRat_zone));
                }
            }
        }

        for (int SurfNum : HTNonWindowSurfs) {
            // Perform heat balance on the inside face of the surface ...
            // The following are possibilities here:
            //   (a) the surface is a pool (no movable insulation, no source/sink, only CTF solution algorithm)
            //   (b) the surface is a partition, in which case the temperature of both sides are the same
            //   (c) standard (or interzone) opaque surface with no movable insulation, normal heat balance equation
            //   (d) standard (or interzone) window: call to CalcWindowHeatBalance to get window layer temperatures
            //   (e) standard opaque surface with movable insulation, special two-part equation
            // In the surface calculation there are the following Algorithm types for opaque surfaces that
            // do not have movable insulation:
            //   (a) the regular CTF calc (SolutionAlgo = UseCTF)
            //   (b) the EMPD calc (Solutionalgo = UseEMPD)
            //   (c) the CondFD calc (SolutionAlgo = UseCondFD)
            //   (d) the HAMT calc (solutionalgo = UseHAMT).

            auto const &surface = state.dataSurface->Surface(SurfNum);
            if (state.dataSurface->UseRepresentativeSurfaceCalculations) {
                int repSurfNum = surface.RepresentativeCalcSurfNum;
                if (SurfNum != repSurfNum) {
                    continue;
                }
            }
            int const ZoneNum = surface.Zone;
            Real64 &TH11 = state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum);
            int const ConstrNum = surface.Construction;
            auto const &construct = state.dataConstruction->Construct(ConstrNum);
            Real64 const MAT_zone = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).MAT;
            Real64 const HConvIn_surf = state.dataMstBal->HConvInFD(SurfNum) = state.dataHeatBalSurf->SurfHConvInt(SurfNum);

            if (surface.ExtBoundCond == SurfNum) {
                // CR6869 -- let Window HB take care of it      IF (Surface(SurfNum)%ExtBoundCond == SurfNum) THEN
                // Surface is adiabatic
                if (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CTF ||
                    surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::EMPD) { // Regular CTF Surface and/or EMPD surface

                    if (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::EMPD) {
                        MoistureBalanceEMPDManager::CalcMoistureBalanceEMPD(
                            state, SurfNum, state.dataHeatBalSurf->SurfTempInTmp(SurfNum), MAT_zone, SurfTempInSat);
                    }
                    // Pre-calculate a few terms
                    Real64 const TempTerm(
                        state.dataHeatBalSurf->SurfCTFConstInPart(SurfNum) + state.dataHeatBal->SurfQdotRadIntGainsInPerArea(SurfNum) +
                        state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(SurfNum) + state.dataHeatBalSurf->SurfQAdditionalHeatSourceInside(SurfNum) +
                        HConvIn_surf * state.dataHeatBalSurfMgr->RefAirTemp(SurfNum) + state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(SurfNum) +
                        state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(SurfNum) +
                        (state.dataHeatBalFanSys->QRadSurfAFNDuct(SurfNum) / state.dataGlobal->TimeStepZoneSec));
                    Real64 const TempDiv(1.0 / (construct.CTFInside[0] - construct.CTFCross[0] + HConvIn_surf + DataHeatBalSurface::IterDampConst));
                    // Calculate the current inside surface temperature
                    if ((!state.dataSurface->SurfIsPool(SurfNum)) ||
                        ((state.dataSurface->SurfIsPool(SurfNum)) &&
                         (std::abs(state.dataHeatBalFanSys->QPoolSurfNumerator(SurfNum)) < DataHeatBalSurface::PoolIsOperatingLimit) &&
                         (std::abs(state.dataHeatBalFanSys->PoolHeatTransCoefs(SurfNum)) < DataHeatBalSurface::PoolIsOperatingLimit))) {
                        if (construct.SourceSinkPresent) {
                            state.dataHeatBalSurf->SurfTempInTmp(SurfNum) =
                                (TempTerm + construct.CTFSourceIn[0] * state.dataHeatBalSurf->SurfQsrcHist(SurfNum, 1) +
                                 DataHeatBalSurface::IterDampConst * state.dataHeatBalSurf->SurfTempInsOld(SurfNum)) *
                                TempDiv; // Constant portion of conduction eq (history terms) | LW radiation from internal sources | SW radiation
                            // from internal sources | Convection from surface to zone air | Net radiant exchange with other zone
                            // surfaces | Heat source/sink term for radiant systems | (if there is one present) | Radiant flux from a
                            // high temperature radiant heater | Radiant flux from a hot water baseboard heater | Radiant flux from a
                            // steam baseboard heater | Radiant flux from an electric baseboard heater | Iterative damping term (for
                            // stability) | Conduction term (both partition sides same temp) | Conduction term (both partition sides
                            // same temp) | Convection and damping term | Radiation from AFN ducts
                        } else {
                            state.dataHeatBalSurf->SurfTempInTmp(SurfNum) =
                                (TempTerm + DataHeatBalSurface::IterDampConst * state.dataHeatBalSurf->SurfTempInsOld(SurfNum)) *
                                TempDiv; // Constant portion of conduction eq (history terms) | LW radiation from internal sources | SW radiation
                            // from internal sources | Convection from surface to zone air | Net radiant exchange with other zone
                            // surfaces | Heat source/sink term for radiant systems | (if there is one present) | Radiant flux from a
                            // high temperature radiant heater | Radiant flux from a hot water baseboard heater | Radiant flux from a
                            // steam baseboard heater | Radiant flux from an electric baseboard heater | Iterative damping term (for
                            // stability) | Conduction term (both partition sides same temp) | Conduction term (both partition sides
                            // same temp) | Convection and damping term | Radiation from AFN ducts
                        }
                    } else { // this is a pool and it has been simulated this time step
                        state.dataHeatBalSurf->SurfTempInTmp(SurfNum) =
                            (state.dataHeatBalSurf->SurfCTFConstInPart(SurfNum) + state.dataHeatBalFanSys->QPoolSurfNumerator(SurfNum) +
                             DataHeatBalSurface::IterDampConst * state.dataHeatBalSurf->SurfTempInsOld(SurfNum)) /
                            (construct.CTFInside[0] - construct.CTFCross[0] + state.dataHeatBalFanSys->PoolHeatTransCoefs(SurfNum) +
                             DataHeatBalSurface::IterDampConst); // Constant part of conduction eq (history terms) | Pool modified terms (see
                        // non-pool equation for details) | Iterative damping term (for stability) |
                        // Conduction term (both partition sides same temp) | Pool and damping term
                    }
                    if (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::EMPD) {
                        state.dataHeatBalSurf->SurfTempInTmp(SurfNum) -=
                            state.dataMstBalEMPD->HeatFluxLatent(SurfNum) * TempDiv; // Conduction term (both partition sides same temp) |
                        // Conduction term (both partition sides same temp) |
                        // Convection and damping term
                        if (SurfTempInSat > state.dataHeatBalSurf->SurfTempInTmp(SurfNum)) {
                            state.dataHeatBalSurf->SurfTempInTmp(SurfNum) = SurfTempInSat; // Surface temp cannot be below dew point
                        }
                    }
                    // if any mixed heat transfer models in zone, apply limits to CTF result
                    if (state.dataHeatBalSurf->Zone_has_mixed_HT_models[ZoneNum]) {
                        state.dataHeatBalSurf->SurfTempInTmp(SurfNum) =
                            max(DataHeatBalSurface::MinSurfaceTempLimit,
                                min(state.dataHeatBalSurf->MaxSurfaceTempLimit,
                                    state.dataHeatBalSurf->SurfTempInTmp(SurfNum))); // Limit Check //Tuned Precomputed condition to eliminate loop
                    }

                    if (construct.SourceSinkPresent) { // Set the appropriate parameters for the radiant system

                        // Radiant system does not need the damping coefficient terms (hopefully) // Partitions are assumed to be symmetric
                        Real64 const RadSysDiv(1.0 / (construct.CTFInside[0] - construct.CTFCross[0] + HConvIn_surf));
                        state.dataHeatBalFanSys->RadSysToHBConstCoef(SurfNum) = state.dataHeatBalFanSys->RadSysTiHBConstCoef(SurfNum) =
                            TempTerm * RadSysDiv; // Constant portion of conduction eq (history terms) | LW radiation from internal sources | SW
                        // radiation from internal sources | Convection from surface to zone air | Radiant flux from
                        // high temperature radiant heater | Radiant flux from a hot water baseboard heater | Radiant
                        // flux from a steam baseboard heater | Radiant flux from an electric baseboard heater | Net
                        // radiant exchange with other zone surfaces | Cond term (both partition sides same temp) | Cond
                        // term (both partition sides same temp) | Convection and damping term
                        state.dataHeatBalFanSys->RadSysToHBTinCoef(SurfNum) = state.dataHeatBalFanSys->RadSysTiHBToutCoef(SurfNum) =
                            0.0; // The outside temp is assumed to be equal to the inside temp for a partition
                        state.dataHeatBalFanSys->RadSysToHBQsrcCoef(SurfNum) = state.dataHeatBalFanSys->RadSysTiHBQsrcCoef(SurfNum) =
                            construct.CTFSourceIn[0] * RadSysDiv; // QTF term for the source | Cond term (both partition sides same temp) | Cond
                        // term (both partition sides same temp) | Convection and damping term
                    }

                } else if (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD ||
                           surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::HAMT) {

                    if (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::HAMT) {
                        HeatBalanceHAMTManager::ManageHeatBalHAMT(state,
                                                                  SurfNum,
                                                                  state.dataHeatBalSurf->SurfTempInTmp(SurfNum),
                                                                  TempSurfOutTmp); // HAMT
                    }

                    if (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD) {
                        HeatBalFiniteDiffManager::ManageHeatBalFiniteDiff(
                            state, SurfNum, state.dataHeatBalSurf->SurfTempInTmp(SurfNum), TempSurfOutTmp);
                    }

                    TH11 = TempSurfOutTmp;
                }

                state.dataHeatBalSurf->SurfTempIn(SurfNum) = state.dataHeatBalSurf->SurfTempInTmp(SurfNum);

            } else { // Standard surface or interzone surface
                bool movableInsulPresent = state.dataSurface->AnyMovableInsulation && state.dataSurface->intMovInsuls(SurfNum).present;
                if (!movableInsulPresent) { // No movable insulation present, normal heat balance equation

                    if (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CTF ||
                        surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::EMPD) { // Regular CTF Surface and/or EMPD surface

                        if (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::EMPD) {
                            MoistureBalanceEMPDManager::CalcMoistureBalanceEMPD(
                                state, SurfNum, state.dataHeatBalSurf->SurfTempInTmp(SurfNum), MAT_zone, SurfTempInSat);
                        }
                        // Pre-calculate a few terms
                        Real64 const TempTerm(
                            state.dataHeatBalSurf->SurfCTFConstInPart(SurfNum) + state.dataHeatBal->SurfQdotRadIntGainsInPerArea(SurfNum) +
                            state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(SurfNum) + state.dataHeatBalSurf->SurfQAdditionalHeatSourceInside(SurfNum) +
                            HConvIn_surf * state.dataHeatBalSurfMgr->RefAirTemp(SurfNum) + state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(SurfNum) +
                            state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(SurfNum) +
                            (state.dataHeatBalFanSys->QRadSurfAFNDuct(SurfNum) / state.dataGlobal->TimeStepZoneSec));
                        Real64 const TempDiv(1.0 / (construct.CTFInside[0] + HConvIn_surf + DataHeatBalSurface::IterDampConst));
                        // Calculate the current inside surface temperature
                        if ((!state.dataSurface->SurfIsPool(SurfNum)) ||
                            ((state.dataSurface->SurfIsPool(SurfNum)) &&
                             (std::abs(state.dataHeatBalFanSys->QPoolSurfNumerator(SurfNum)) < DataHeatBalSurface::PoolIsOperatingLimit) &&
                             (std::abs(state.dataHeatBalFanSys->PoolHeatTransCoefs(SurfNum)) < DataHeatBalSurface::PoolIsOperatingLimit))) {
                            if (construct.SourceSinkPresent) {
                                state.dataHeatBalSurf->SurfTempInTmp(SurfNum) =
                                    (TempTerm + construct.CTFSourceIn[0] * state.dataHeatBalSurf->SurfQsrcHist(SurfNum, 1) +
                                     DataHeatBalSurface::IterDampConst * state.dataHeatBalSurf->SurfTempInsOld(SurfNum) +
                                     construct.CTFCross[0] * TH11) *
                                    TempDiv; // Constant part of conduction eq (history terms) | LW radiation from internal sources | SW
                                // radiation from internal sources | Convection from surface to zone air | Net radiant exchange
                                // with other zone surfaces | Heat source/sink term for radiant systems | (if there is one
                                // present) | Radiant flux from high temp radiant heater | Radiant flux from a hot water
                                // baseboard heater | Radiant flux from a steam baseboard heater | Radiant flux from an electric
                                // baseboard heater | Iterative damping term (for stability) | Current conduction from | the
                                // outside surface | Coefficient for conduction (current time) | Convection and damping term |
                                // Radiation from AFN ducts
                            } else {
                                state.dataHeatBalSurf->SurfTempInTmp(SurfNum) =
                                    (TempTerm + DataHeatBalSurface::IterDampConst * state.dataHeatBalSurf->SurfTempInsOld(SurfNum) +
                                     construct.CTFCross[0] * TH11) *
                                    TempDiv; // Constant part of conduction eq (history terms) | LW radiation from internal sources | SW
                                // radiation from internal sources | Convection from surface to zone air | Net radiant exchange
                                // with other zone surfaces | Heat source/sink term for radiant systems | (if there is one
                                // present) | Radiant flux from high temp radiant heater | Radiant flux from a hot water
                                // baseboard heater | Radiant flux from a steam baseboard heater | Radiant flux from an electric
                                // baseboard heater | Iterative damping term (for stability) | Current conduction from | the
                                // outside surface | Coefficient for conduction (current time) | Convection and damping term |
                                // Radiation from AFN ducts
                            }
                        } else { // surface is a pool and the pool has been simulated this time step
                            state.dataHeatBalSurf->SurfTempInTmp(SurfNum) =
                                (state.dataHeatBalSurf->SurfCTFConstInPart(SurfNum) + state.dataHeatBalFanSys->QPoolSurfNumerator(SurfNum) +
                                 DataHeatBalSurface::IterDampConst * state.dataHeatBalSurf->SurfTempInsOld(SurfNum) + construct.CTFCross[0] * TH11) /
                                (construct.CTFInside[0] + state.dataHeatBalFanSys->PoolHeatTransCoefs(SurfNum) +
                                 DataHeatBalSurface::IterDampConst); // Constant part of conduction eq (history terms) | Pool modified terms
                            // (see non-pool equation for details) | Iterative damping term (for
                            // stability) | Current conduction from | the outside surface |
                            // Coefficient for conduction (current time) | Pool and damping term
                        }
                        if (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::EMPD) {
                            state.dataHeatBalSurf->SurfTempInTmp(SurfNum) -=
                                state.dataMstBalEMPD->HeatFluxLatent(SurfNum) *
                                TempDiv; // Coefficient for conduction (current time) | Convection and damping term
                            if (SurfTempInSat > state.dataHeatBalSurf->SurfTempInTmp(SurfNum)) {
                                state.dataHeatBalSurf->SurfTempInTmp(SurfNum) = SurfTempInSat; // Surface temp cannot be below dew point
                            }
                        }
                        // if any mixed heat transfer models in zone, apply limits to CTF result
                        if (state.dataHeatBalSurf->Zone_has_mixed_HT_models[ZoneNum]) {
                            state.dataHeatBalSurf->SurfTempInTmp(SurfNum) = max(
                                DataHeatBalSurface::MinSurfaceTempLimit,
                                min(state.dataHeatBalSurf->MaxSurfaceTempLimit,
                                    state.dataHeatBalSurf->SurfTempInTmp(SurfNum))); // Limit Check //Tuned Precomputed condition to eliminate loop
                        }

                        if (construct.SourceSinkPresent) { // Set the appropriate parameters for the radiant system

                            // Radiant system does not need the damping coefficient terms (hopefully)
                            Real64 const RadSysDiv(1.0 / (construct.CTFInside[0] + HConvIn_surf));
                            state.dataHeatBalFanSys->RadSysTiHBConstCoef(SurfNum) =
                                TempTerm * RadSysDiv; // Constant portion of cond eq (history terms) | LW radiation from internal sources | SW
                            // radiation from internal sources | Convection from surface to zone air | Radiant flux
                            // from high temp radiant heater | Radiant flux from a hot water baseboard heater |
                            // Radiant flux from a steam baseboard heater | Radiant flux from an electric baseboard
                            // heater | Net radiant exchange with other zone surfaces | Cond term (both partition
                            // sides same temp) | Convection and damping term
                            state.dataHeatBalFanSys->RadSysTiHBToutCoef(SurfNum) =
                                construct.CTFCross[0] * RadSysDiv; // Outside temp=inside temp for a partition |
                            // Cond term (both partition sides same temp) |
                            // Convection and damping term
                            state.dataHeatBalFanSys->RadSysTiHBQsrcCoef(SurfNum) =
                                construct.CTFSourceIn[0] * RadSysDiv; // QTF term for the source | Cond term (both
                            // partition sides same temp) | Convection and
                            // damping term

                            if (surface.ExtBoundCond > 0) { // This is an interzone partition and we need to set outside params
                                // The inside coefficients of one side are equal to the outside coefficients of the other side.  But,
                                // the inside coefficients are set up once the heat balance equation for that side has been calculated.
                                // For both sides to actually have been set, we have to wait until we get to the second side in the surface
                                // derived type.  At that point, both inside coefficient sets have been evaluated.
                                if (surface.ExtBoundCond < SurfNum) { // Both of the inside coefficients have now been set
                                    int OtherSideSurfNum = surface.ExtBoundCond;
                                    state.dataHeatBalFanSys->RadSysToHBConstCoef(OtherSideSurfNum) =
                                        state.dataHeatBalFanSys->RadSysTiHBConstCoef(SurfNum);
                                    state.dataHeatBalFanSys->RadSysToHBTinCoef(OtherSideSurfNum) =
                                        state.dataHeatBalFanSys->RadSysTiHBToutCoef(SurfNum);
                                    state.dataHeatBalFanSys->RadSysToHBQsrcCoef(OtherSideSurfNum) =
                                        state.dataHeatBalFanSys->RadSysTiHBQsrcCoef(SurfNum);
                                    state.dataHeatBalFanSys->RadSysToHBConstCoef(SurfNum) =
                                        state.dataHeatBalFanSys->RadSysTiHBConstCoef(OtherSideSurfNum);
                                    state.dataHeatBalFanSys->RadSysToHBTinCoef(SurfNum) =
                                        state.dataHeatBalFanSys->RadSysTiHBToutCoef(OtherSideSurfNum);
                                    state.dataHeatBalFanSys->RadSysToHBQsrcCoef(SurfNum) =
                                        state.dataHeatBalFanSys->RadSysTiHBQsrcCoef(OtherSideSurfNum);
                                }
                            }
                        }

                    } else if (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD ||
                               surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::HAMT) {

                        if (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::HAMT) {
                            if (surface.ExtBoundCond > 0) {
                                // HAMT get the correct other side zone zone air temperature --
                                int OtherSideSurfNum = surface.ExtBoundCond;
                                // ZoneNum = surface.Zone;
                                OtherSideZoneNum = state.dataSurface->Surface(OtherSideSurfNum).Zone;
                                state.dataMstBal->TempOutsideAirFD(SurfNum) =
                                    state.dataZoneTempPredictorCorrector->zoneHeatBalance(OtherSideZoneNum).MAT;
                            }
                            HeatBalanceHAMTManager::ManageHeatBalHAMT(state, SurfNum, state.dataHeatBalSurf->SurfTempInTmp(SurfNum), TempSurfOutTmp);
                        }

                        if (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD) {
                            HeatBalFiniteDiffManager::ManageHeatBalFiniteDiff(
                                state, SurfNum, state.dataHeatBalSurf->SurfTempInTmp(SurfNum), TempSurfOutTmp);
                        }

                        TH11 = TempSurfOutTmp;

                    } else if (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::Kiva) {
                        // Read Kiva results for each surface
                        state.dataHeatBalSurf->SurfTempInTmp(SurfNum) =
                            state.dataSurfaceGeometry->kivaManager.surfaceMap[SurfNum].results.Tconv - Constant::Kelvin;

                        TH11 = 0.0;
                    }

                    state.dataHeatBalSurf->SurfTempIn(SurfNum) = state.dataHeatBalSurf->SurfTempInTmp(SurfNum);

                } else { // Movable insulation present
                    Real64 HMovInsul = state.dataSurface->intMovInsuls(SurfNum).H;
                    if (construct.SourceSinkPresent) {

                        ShowSevereError(state, "Interior movable insulation is not valid with embedded sources/sinks");
                        ShowContinueError(state, format("Construction {} contains an internal source or sink but also uses", construct.Name));
                        ShowContinueError(state,
                                          format("interior movable insulation {} for a surface with that construction.",
                                                 s_mat->materials(state.dataSurface->intMovInsuls(SurfNum).matNum)->Name));
                        ShowContinueError(state,
                                          "This is not currently allowed because the heat balance equations do not currently accommodate "
                                          "this combination.");
                        ShowFatalError(state, "CalcHeatBalanceInsideSurf: Program terminates due to preceding conditions.");
                    }

                    Real64 F1 = HMovInsul / (HMovInsul + HConvIn_surf + DataHeatBalSurface::IterDampConst);

                    state.dataHeatBalSurf->SurfTempIn(SurfNum) =
                        (state.dataHeatBalSurf->SurfCTFConstInPart(SurfNum) + state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(SurfNum) +
                         construct.CTFCross[0] * TH11 +
                         F1 * (state.dataHeatBal->SurfQdotRadIntGainsInPerArea(SurfNum) +
                               HConvIn_surf * state.dataHeatBalSurfMgr->RefAirTemp(SurfNum) +
                               state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(SurfNum) + state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(SurfNum) +
                               state.dataHeatBalSurf->SurfQAdditionalHeatSourceInside(SurfNum) +
                               DataHeatBalSurface::IterDampConst * state.dataHeatBalSurf->SurfTempInsOld(SurfNum))) /
                        (construct.CTFInside[0] + HMovInsul - F1 * HMovInsul); // Convection from surface to zone air

                    state.dataHeatBalSurf->SurfTempInTmp(SurfNum) =
                        (construct.CTFInside[0] * state.dataHeatBalSurf->SurfTempIn(SurfNum) +
                         HMovInsul * state.dataHeatBalSurf->SurfTempIn(SurfNum) - state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(SurfNum) -
                         state.dataHeatBalSurf->SurfCTFConstInPart(SurfNum) - construct.CTFCross[0] * TH11) /
                        (HMovInsul);
                    // if any mixed heat transfer models in zone, apply limits to CTF result
                    if (state.dataHeatBalSurf->Zone_has_mixed_HT_models[ZoneNum]) {
                        state.dataHeatBalSurf->SurfTempInTmp(SurfNum) =
                            max(DataHeatBalSurface::MinSurfaceTempLimit,
                                min(state.dataHeatBalSurf->MaxSurfaceTempLimit,
                                    state.dataHeatBalSurf->SurfTempInTmp(SurfNum))); // Limit Check //Tuned Precomputed condition to eliminate loop
                    }
                }
            }
        }
        for (int SurfNum : HTWindowSurfs) {
            auto &surface = state.dataSurface->Surface(SurfNum);
            if (state.dataSurface->UseRepresentativeSurfaceCalculations) {
                int repSurfNum = surface.RepresentativeCalcSurfNum;
                if (SurfNum != repSurfNum) {
                    continue;
                }
            }
            Real64 &TH11 = state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum);
            int const ConstrNum = state.dataSurface->SurfActiveConstruction(SurfNum); // Not const, because storm window may change this
            auto const &construct = state.dataConstruction->Construct(ConstrNum);
            if (surface.OriginalClass == DataSurfaces::SurfaceClass::TDD_Diffuser) { // Tubular daylighting device
                // Lookup up the TDD:DOME object
                int const pipeNum = state.dataSurface->SurfWinTDDPipeNum(SurfNum);
                int const domeNum = state.dataDaylightingDevicesData->TDDPipe(pipeNum).Dome;
                // Ueff = 1 / effective R value between TDD:DOME and TDD:DIFFUSER
                Real64 Ueff = 1.0 / state.dataDaylightingDevicesData->TDDPipe(pipeNum).Reff;

                // Similar to opaque surface but outside surface temp of TDD:DOME is used, and no embedded sources/sinks.
                // Absorbed shortwave radiation is treated similar to a regular window, but only 1 glass layer is allowed.
                //   = SurfWinQRadSWwinAbs(SurfNum,1)/2.0
                Real64 const HConvIn_surf(state.dataMstBal->HConvInFD(SurfNum) = state.dataHeatBalSurf->SurfHConvInt(SurfNum));
                state.dataHeatBalSurf->SurfTempInTmp(SurfNum) =
                    (state.dataHeatBal->SurfQdotRadIntGainsInPerArea(SurfNum) + state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, 1) / 2.0 +
                     state.dataHeatBalSurf->SurfQAdditionalHeatSourceInside(SurfNum) + HConvIn_surf * state.dataHeatBalSurfMgr->RefAirTemp(SurfNum) +
                     state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(SurfNum) +
                     DataHeatBalSurface::IterDampConst * state.dataHeatBalSurf->SurfTempInsOld(SurfNum) +
                     Ueff * state.dataHeatBalSurf->SurfOutsideTempHist(1)(domeNum)) /
                    (Ueff + HConvIn_surf +
                     DataHeatBalSurface::IterDampConst); // LW radiation from internal sources | SW radiation from internal sources and
                                                         // solar | Convection from surface to zone air | Net radiant exchange with
                                                         // other zone surfaces | Iterative damping term (for stability) | Current
                                                         // conduction from the outside surface | Coefficient for conduction (current
                                                         // time) | Convection and damping term
                state.dataHeatBalSurf->SurfTempIn(SurfNum) = state.dataHeatBalSurf->SurfTempInTmp(SurfNum);

                Real64 const Sigma_Temp_4(Constant::StefanBoltzmann * pow_4(state.dataHeatBalSurf->SurfTempIn(SurfNum) + Constant::Kelvin));

                // fill out report vars for components of Window Heat Gain
                state.dataSurface->SurfWinGainConvGlazToZoneRep(SurfNum) =
                    HConvIn_surf * surface.Area * (state.dataHeatBalSurf->SurfTempIn(SurfNum) - state.dataHeatBalSurfMgr->RefAirTemp(SurfNum));
                state.dataSurface->SurfWinGainIRGlazToZoneRep(SurfNum) =
                    state.dataConstruction->Construct(surface.Construction).InsideAbsorpThermal * surface.Area *
                    (Sigma_Temp_4 - (state.dataSurface->SurfWinIRfromParentZone(SurfNum) + state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(SurfNum)));
                state.dataSurface->SurfWinLossSWZoneToOutWinRep(SurfNum) =
                    state.dataHeatBal->EnclSolQSWRad(surface.SolarEnclIndex) * surface.Area *
                        (1 - state.dataConstruction->Construct(surface.Construction).ReflectSolDiffBack) +
                    state.dataHeatBalSurf->SurfWinInitialBeamSolInTrans(SurfNum);

                // Calculate window heat gain for TDD:DIFFUSER since this calculation is usually done in WindowManager
                state.dataSurface->SurfWinHeatGain(SurfNum) =
                    state.dataSurface->SurfWinTransSolar(SurfNum) + state.dataSurface->SurfWinGainConvGlazToZoneRep(SurfNum) +
                    state.dataSurface->SurfWinGainIRGlazToZoneRep(SurfNum) - state.dataSurface->SurfWinLossSWZoneToOutWinRep(SurfNum) -
                    surface.Area * state.dataHeatBalSurf->SurfWinInitialDifSolInTrans(SurfNum);
                // Net transmitted solar | Convection | IR exchange | IR
                // Zone diffuse interior shortwave reflected back into the TDD

            } else {                                                // Regular window
                if (state.dataHeatBal->InsideSurfIterations == 0) { // Do windows only once
                    // Get outside convection coeff for exterior window here to avoid calling
                    // InitExteriorConvectionCoeff from CalcWindowHeatBalance, which avoids circular reference
                    // (HeatBalanceSurfaceManager USEing and WindowManager and
                    // WindowManager USEing HeatBalanceSurfaceManager)
                    if (surface.ExtBoundCond == DataSurfaces::ExternalEnvironment) {
                        auto const *thisMaterial = s_mat->materials(construct.LayerPoint(1));
                        Material::SurfaceRoughness RoughSurf = thisMaterial->Roughness; // Outside surface roughness
                        Real64 EmisOut = thisMaterial->AbsorpThermalFront;              // Glass outside surface emissivity
                        DataSurfaces::WinShadingType const shading_flag(state.dataSurface->SurfWinShadingFlag(SurfNum));
                        if (ANY_EXTERIOR_SHADE_BLIND_SCREEN(shading_flag)) {
                            // Exterior shade in place
                            int const ConstrNumSh = surface.activeShadedConstruction;
                            if (ConstrNumSh != 0) {
                                auto const &constructionSh = state.dataConstruction->Construct(ConstrNumSh);
                                auto const *thisMaterial2 = s_mat->materials(constructionSh.LayerPoint(1));
                                assert(thisMaterial2 != nullptr);
                                RoughSurf = thisMaterial2->Roughness;
                                EmisOut = thisMaterial2->AbsorpThermal;
                            }
                        }

                        // Get the outside effective emissivity for Equivalent layer model
                        if (construct.WindowTypeEQL) {
                            EmisOut = WindowEquivalentLayer::EQLWindowOutsideEffectiveEmiss(state, ConstrNum);
                        }
                        // Set Exterior Convection Coefficient...
                        if (state.dataSurface->surfExtConv(SurfNum).userModelNum != 0) {

                            state.dataHeatBalSurf->SurfHConvExt(SurfNum) = Convect::SetExtConvCoeff(state, SurfNum);

                        } else if (surface.ExtWind) { // Window is exposed to wind (and possibly rain)

                            // Calculate exterior heat transfer coefficients with windspeed (windspeed is calculated internally in
                            // subroutine)
                            Convect::InitExtConvCoeff(state,
                                                      SurfNum,
                                                      0.0,
                                                      RoughSurf,
                                                      EmisOut,
                                                      TH11,
                                                      state.dataHeatBalSurf->SurfHConvExt(SurfNum),
                                                      state.dataHeatBalSurf->SurfHSkyExt(SurfNum),
                                                      state.dataHeatBalSurf->SurfHGrdExt(SurfNum),
                                                      state.dataHeatBalSurf->SurfHAirExt(SurfNum),
                                                      state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum));

                            if (state.dataEnvrn->IsRain) {                             // Raining: since wind exposed, outside window surface gets wet
                                state.dataHeatBalSurf->SurfHConvExt(SurfNum) = 1000.0; // Reset SurfHcExt because of wetness
                            }

                        } else { // Not Wind exposed

                            // Calculate exterior heat transfer coefficients for windspeed = 0
                            Convect::InitExtConvCoeff(state,
                                                      SurfNum,
                                                      0.0,
                                                      RoughSurf,
                                                      EmisOut,
                                                      TH11,
                                                      state.dataHeatBalSurf->SurfHConvExt(SurfNum),
                                                      state.dataHeatBalSurf->SurfHSkyExt(SurfNum),
                                                      state.dataHeatBalSurf->SurfHGrdExt(SurfNum),
                                                      state.dataHeatBalSurf->SurfHAirExt(SurfNum),
                                                      state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum));
                        }
                    } else { // Interior Surface

                        if (state.dataSurface->surfExtConv(SurfNum).userModelNum != 0) {
                            state.dataHeatBalSurf->SurfHConvExt(SurfNum) = Convect::SetExtConvCoeff(state, SurfNum);
                        } else {
                            // Exterior Convection Coefficient for the Interior or Interzone Window is the Interior Convection Coeff of
                            // same
                            state.dataHeatBalSurf->SurfHConvExt(SurfNum) = state.dataHeatBalSurf->SurfHConvInt(surface.ExtBoundCond);
                        }
                    }

                    // Following call determines inside surface temperature of glazing, and of
                    // frame and/or divider, if present
                    Window::CalcWindowHeatBalance(
                        state, SurfNum, state.dataHeatBalSurf->SurfHConvExt(SurfNum), state.dataHeatBalSurf->SurfTempInTmp(SurfNum), TH11);
                    state.dataHeatBalSurf->SurfTempIn(SurfNum) = state.dataHeatBalSurf->SurfTempInTmp(SurfNum);
                }
            }
        } // ...end of inside surface heat balance equation selection

        for (int SurfNum : HTSurfs) {
            int const ZoneNum = state.dataSurface->Surface(SurfNum).Zone;
            auto &zone = state.dataHeatBal->Zone(ZoneNum);
            Real64 &TH11 = state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum);
            Real64 &TH12 = state.dataHeatBalSurf->SurfInsideTempHist(1)(SurfNum);
            TH12 = state.dataHeatBalSurf->SurfTempIn(SurfNum);
            state.dataHeatBalSurf->SurfTempOut(SurfNum) = TH11; // For reporting
            if (state.dataSurface->Surface(SurfNum).OriginalClass == DataSurfaces::SurfaceClass::TDD_Dome) {
                continue;
            }
            if (state.dataSurface->Surface(SurfNum).OriginalClass == DataSurfaces::SurfaceClass::TDD_Diffuser) { // Tubular daylighting device
                // Tubular daylighting devices are treated as one big object with an effective R value.
                // The outside face temperature of the TDD:DOME and the inside face temperature of the
                // TDD:DIFFUSER are calculated with the outside and inside heat balances respectively.
                // Below, the resulting temperatures are copied to the inside face of the TDD:DOME
                // and the outside face of the TDD:DIFFUSER for reporting.

                // Set inside temp variables of TDD:DOME equal to inside temp of TDD:DIFFUSER
                int domeNum = state.dataDaylightingDevicesData->TDDPipe(state.dataSurface->SurfWinTDDPipeNum(SurfNum)).Dome;
                state.dataHeatBalSurf->SurfInsideTempHist(1)(domeNum) = state.dataHeatBalSurf->SurfTempIn(domeNum) =
                    state.dataHeatBalSurf->SurfTempInTmp(domeNum) = state.dataHeatBalSurf->SurfTempIn(SurfNum);

                // Set outside temp reporting variable of TDD:DOME (since it gets skipped otherwise)
                // Reset outside temp variables of TDD:DIFFUSER equal to outside temp of TDD:DOME
                TH11 = state.dataHeatBalSurf->SurfTempOut(SurfNum) = state.dataHeatBalSurf->SurfTempOut(domeNum) =
                    state.dataHeatBalSurf->SurfOutsideTempHist(1)(domeNum);
            }

            if ((TH12 > state.dataHeatBalSurf->MaxSurfaceTempLimit) || (TH12 < DataHeatBalSurface::MinSurfaceTempLimit)) {
                TestSurfTempCalcHeatBalanceInsideSurf(state, TH12, SurfNum, zone, state.dataHeatBalSurfMgr->calcHeatBalInsideSurfWarmupErrCount);
            }

        } // ...end of main loops over all surfaces for inside heat balances

        // Interzone surface updating: interzone surfaces have other side temperatures
        // which can vary as the simulation iterates through the inside heat
        // balance.  This block is intended to "lock" the opposite side (outside)
        // temperatures to the correct value, namely the value calculated by the
        // inside surface heat balance for the other side.
        //        assert(state.dataHeatBalSurf->TH.index(1, 1, 1) == 0u); // Assumed for linear indexing below
        //        int const l211(state.dataHeatBalSurf->TH.index(2, 1, 1) - 1);
        for (int SurfNum : IZSurfs) {
            int const surfExtBoundCond = state.dataSurface->Surface(SurfNum).ExtBoundCond;
            // Set the outside surface temperature to the inside surface temperature of the interzone pair.
            // By going through all of the surfaces, this should pick up the other side as well as affect the next iteration.
            // [ SurfNum - 1 ] == ( 1, 1, SurfNum )
            // [ l211 + surfExtBoundCond ] == ( 2, 1, surfExtBoundCond )
            state.dataHeatBalSurf->SurfTempOut(SurfNum) = state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum) =
                state.dataHeatBalSurf->SurfInsideTempHist(1)(surfExtBoundCond);
        }

        ++state.dataHeatBal->InsideSurfIterations;

        // Convergence check - Loop through all relevant non-window surfaces to check for convergence...
        Real64 MaxDelTemp = 0.0; // Maximum change in surface temperature for any opaque surface from one iteration to the next
        for (int SurfNum : HTNonWindowSurfs) {
            MaxDelTemp = max(std::abs(state.dataHeatBalSurf->SurfTempIn(SurfNum) - state.dataHeatBalSurf->SurfTempInsOld(SurfNum)), MaxDelTemp);
            if (state.dataSurface->Surface(SurfNum).HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::CondFD) {
                // also check all internal nodes as well as surface faces
                MaxDelTemp = max(MaxDelTemp, state.dataHeatBalFiniteDiffMgr->SurfaceFD(SurfNum).MaxNodeDelTemp);
            }
        } // ...end of loop to check for convergence

        if (!state.dataHeatBal->AnyCondFD) {
            if (MaxDelTemp <= state.dataHeatBal->MaxAllowedDelTemp) {
                Converged = true;
            }
        } else {
            if (MaxDelTemp <= state.dataHeatBal->MaxAllowedDelTempCondFD) {
                Converged = true;
            }

            // resets relaxation factor to speed up iterations when under-relaxation is not needed.
            if (state.dataHeatBal->InsideSurfIterations <= 1) {
                state.dataHeatBal->CondFDRelaxFactor = state.dataHeatBal->CondFDRelaxFactorInput;
            }
            if ((state.dataHeatBal->InsideSurfIterations > DataHeatBalSurface::IterationsForCondFDRelaxChange) && !Converged) {
                // adjust relaxation factor down, assume large number of iterations is result of instability
                state.dataHeatBal->CondFDRelaxFactor *= 0.9;
                if (state.dataHeatBal->CondFDRelaxFactor < 0.1) {
                    state.dataHeatBal->CondFDRelaxFactor = 0.1;
                }
            }
        }

#ifdef EP_Count_Calls
        state.dataTimingsData->NumMaxInsideSurfIterations =
            max(state.dataTimingsData->NumMaxInsideSurfIterations, state.dataHeatBal->InsideSurfIterations);
#endif

        if (state.dataHeatBal->InsideSurfIterations < state.dataHeatBalSurf->MinIterations) {
            Converged = false;
        }

        if (state.dataHeatBal->InsideSurfIterations > DataHeatBalSurface::MaxIterations) {
            if (!state.dataGlobal->WarmupFlag) {
                ++state.dataHeatBalSurfMgr->calcHeatBalInsideSurfErrCount;
                if (state.dataHeatBalSurfMgr->calcHeatBalInsideSurfErrCount < 16) {
                    if (!state.dataHeatBal->AnyCondFD) {
                        ShowWarningError(state,
                                         format("Inside surface heat balance did not converge with Max Temp Difference [C] ={:.3R} vs Max "
                                                "Allowed Temp Diff [C] ={:.3R}",
                                                MaxDelTemp,
                                                state.dataHeatBal->MaxAllowedDelTemp));
                        ShowContinueErrorTimeStamp(state, "");
                    } else {
                        ShowWarningError(state,
                                         format("Inside surface heat balance did not converge with Max Temp Difference [C] ={:.3R} vs Max "
                                                "Allowed Temp Diff [C] ={:.6R}",
                                                MaxDelTemp,
                                                state.dataHeatBal->MaxAllowedDelTempCondFD));
                        ShowContinueErrorTimeStamp(state, "");
                    }
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   "Inside surface heat balance convergence problem continues",
                                                   state.dataHeatBalSurfMgr->calcHeatBalInsideSurfErrPointer,
                                                   MaxDelTemp,
                                                   MaxDelTemp,
                                                   _,
                                                   "[C]",
                                                   "[C]");
                }
            }
            break; // iteration loop
        }

    } // ...end of main inside heat balance DO loop (ends when Converged)

    // Update SumHmXXXX for non-window EMPD or HAMT surfaces
    if (state.dataHeatBal->AnyEMPD || state.dataHeatBal->AnyHAMT) {

        // these SumHmA* variables are only used for EMPD and HAMT and should be reset each time step (and every iteration)
        for (auto &thisZoneHB : state.dataZoneTempPredictorCorrector->zoneHeatBalance) {
            thisZoneHB.SumHmAW = 0.0;
            thisZoneHB.SumHmARa = 0.0;
            thisZoneHB.SumHmARaW = 0.0;
        }

        for (int SurfNum : HTNonWindowSurfs) {
            auto const &surface = state.dataSurface->Surface(SurfNum);
            int ZoneNum = surface.Zone;
            auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum);

            if (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::HAMT) {
                HeatBalanceHAMTManager::UpdateHeatBalHAMT(state, SurfNum);

                Real64 const FD_Area_fac(state.dataMstBal->HMassConvInFD(SurfNum) * surface.Area);

                thisZoneHB.SumHmAW += FD_Area_fac * (state.dataMstBal->RhoVaporSurfIn(SurfNum) - state.dataMstBal->RhoVaporAirIn(SurfNum));

                Real64 const MAT_zone(state.dataZoneTempPredictorCorrector->zoneHeatBalance(surface.Zone).MAT);
                RhoAirZone = Psychrometrics::PsyRhoAirFnPbTdbW(
                    state,
                    state.dataEnvrn->OutBaroPress,
                    MAT_zone,
                    Psychrometrics::PsyWFnTdbRhPb(
                        state,
                        MAT_zone,
                        Psychrometrics::PsyRhFnTdbRhov(state, MAT_zone, state.dataMstBal->RhoVaporAirIn(SurfNum), rhoAirZone),
                        state.dataEnvrn->OutBaroPress));

                Real64 const surfInTemp(state.dataHeatBalSurf->SurfTempInTmp(SurfNum));
                Wsurf =
                    Psychrometrics::PsyWFnTdbRhPb(state,
                                                  surfInTemp,
                                                  Psychrometrics::PsyRhFnTdbRhov(state, surfInTemp, state.dataMstBal->RhoVaporSurfIn(SurfNum), wsurf),
                                                  state.dataEnvrn->OutBaroPress);

                thisZoneHB.SumHmARa += FD_Area_fac * RhoAirZone;

                thisZoneHB.SumHmARaW += FD_Area_fac * state.dataMstBal->RhoVaporSurfIn(SurfNum); // old eq'n: FD_Area_fac * RhoAirZone * Wsurf;

            } else if (surface.HeatTransferAlgorithm == DataSurfaces::HeatTransferModel::EMPD) {
                // need to calculate the amount of moisture that is entering or
                // leaving the zone  Qm [kg/sec] = hmi * Area * (Del Rhov)
                // {Hmi [m/sec];     Area [m2];    Rhov [kg moist/m3]  }
                // Positive values are into the zone and negative values are
                // leaving the zone.  SumHmAw is the sum of the moisture entering or
                // leaving the zone from all of the surfaces and is a rate.  Multiply
                // by time to get the actual amount affecting the zone volume of air.

                MoistureBalanceEMPDManager::UpdateMoistureBalanceEMPD(state, SurfNum);
                state.dataMstBal->RhoVaporSurfIn(SurfNum) = state.dataMstBalEMPD->RVSurface(SurfNum);
                Real64 const FD_Area_fac(state.dataMstBal->HMassConvInFD(SurfNum) * surface.Area);
                thisZoneHB.SumHmAW += FD_Area_fac * (state.dataMstBal->RhoVaporSurfIn(SurfNum) - state.dataMstBal->RhoVaporAirIn(SurfNum));
                Real64 const MAT_zone(state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).MAT);
                thisZoneHB.SumHmARa +=
                    FD_Area_fac *
                    Psychrometrics::PsyRhoAirFnPbTdbW(
                        state,
                        state.dataEnvrn->OutBaroPress,
                        MAT_zone,
                        Psychrometrics::PsyWFnTdbRhPb(state,
                                                      MAT_zone,
                                                      Psychrometrics::PsyRhFnTdbRhovLBnd0C(state, MAT_zone, state.dataMstBal->RhoVaporAirIn(SurfNum)),
                                                      state.dataEnvrn->OutBaroPress)); // surfInTemp, PsyWFnTdbRhPb( surfInTemp, PsyRhFnTdbRhovLBnd0C(
                // surfInTemp, RhoVaporAirIn( SurfNum ) ), OutBaroPress ) );
                thisZoneHB.SumHmARaW += FD_Area_fac * state.dataMstBal->RhoVaporSurfIn(SurfNum);
            }
        }
    }
}

void CalcHeatBalanceInsideSurf2CTFOnly(EnergyPlusData &state,
                                       const int FirstZone,             // First zone to simulate
                                       const int LastZone,              // Last zone to simulate
                                       const std::vector<int> &IZSurfs, // Last zone to simulate
                                       ObjexxFCL::Optional_int_const ZoneToResimulate)
{

    // This function performs a heat balance on the inside face of each
    // surface in the building. It is a copy of CalcHeatBalanceInsideSurf,
    // simplified for CTF surfaces only.

    // REFERENCES:
    // (I)BLAST legacy routine HBSRF

    auto &s_mat = state.dataMaterial;
    auto &Surface = state.dataSurface->Surface;

    constexpr std::string_view Inside("Inside");

    if (state.dataHeatBalSurfMgr->calcHeatBalInsideSurfCTFOnlyFirstTime) {
        // Set up coefficient arrays that never change - loop over non-window HT surfaces
        for (int zoneNum = FirstZone; zoneNum <= LastZone; ++zoneNum) {
            for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                int const firstSurf = thisSpace.OpaqOrIntMassSurfaceFirst;
                int const lastSurf = thisSpace.OpaqOrIntMassSurfaceLast;
                for (int surfNum = firstSurf; surfNum <= lastSurf; ++surfNum) {
                    int const ConstrNum = Surface(surfNum).Construction;
                    auto const &construct = state.dataConstruction->Construct(ConstrNum);
                    if (Surface(surfNum).ExtBoundCond == surfNum) {
                        state.dataHeatBalSurf->SurfIsAdiabatic(surfNum) = 1;
                    } else {
                        state.dataHeatBalSurf->SurfIsAdiabatic(surfNum) = 0;
                    }
                    if (construct.SourceSinkPresent) {
                        state.dataHeatBalSurf->SurfIsSourceOrSink(surfNum) = 1;
                    } else {
                        state.dataHeatBalSurf->SurfIsSourceOrSink(surfNum) = 0;
                    }
                }
            }
        }

        state.dataHeatBalSurfMgr->calcHeatBalInsideSurfCTFOnlyFirstTime = false;
    }

    for (int zoneNum = FirstZone; zoneNum <= LastZone; ++zoneNum) {
        for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
            auto const &thisSpace = state.dataHeatBal->space(spaceNum);
            // loop over all heat transfer surface except TDD Dome.
            int const firstSurf = thisSpace.OpaqOrWinSurfaceFirst;
            int const lastSurf = thisSpace.OpaqOrWinSurfaceLast;
            // determine reference air temperatures and other variable terms - loop over all surfaces
            for (int surfNum = firstSurf; surfNum <= lastSurf; ++surfNum) {
                auto const &surface = Surface(surfNum);
                if (state.dataSurface->UseRepresentativeSurfaceCalculations) {
                    int repSurfNum = surface.RepresentativeCalcSurfNum;
                    if (surfNum != repSurfNum) {
                        continue;
                    }
                }

                int const ConstrNum = Surface(surfNum).Construction;
                auto const &construct = state.dataConstruction->Construct(ConstrNum);
                state.dataHeatBalSurf->SurfCTFCross0(surfNum) = construct.CTFCross[0];
                state.dataHeatBalSurf->SurfCTFInside0(surfNum) = construct.CTFInside[0];
                state.dataHeatBalSurf->SurfCTFSourceIn0(surfNum) = construct.CTFSourceIn[0];
                state.dataHeatBalSurf->SurfTempOutHist(surfNum) = state.dataHeatBalSurf->SurfOutsideTempHist(1)(surfNum);
                if (construct.SourceSinkPresent) {
                    state.dataHeatBalSurf->SurfQSourceSinkHist(surfNum) = state.dataHeatBalSurf->SurfQsrcHist(surfNum, 1);
                }

                // The special heat balance terms for pools are used only when the pool is operating, so IsPool can change
                if (state.dataSurface->SurfIsPool(surfNum)) {
                    if ((std::abs(state.dataHeatBalFanSys->QPoolSurfNumerator(surfNum)) >= DataHeatBalSurface::PoolIsOperatingLimit) ||
                        (std::abs(state.dataHeatBalFanSys->PoolHeatTransCoefs(surfNum)) >= DataHeatBalSurface::PoolIsOperatingLimit)) {
                        state.dataHeatBalSurf->SurfIsOperatingPool(surfNum) = 1;
                    } else {
                        state.dataHeatBalSurf->SurfIsOperatingPool(surfNum) = 0;
                    }
                }
                Real64 RefAirTemp = state.dataSurface->Surface(surfNum).getInsideAirTemperature(state, surfNum);
                state.dataHeatBalSurfMgr->RefAirTemp(surfNum) = RefAirTemp;
                state.dataHeatBal->SurfTempEffBulkAir(surfNum) = state.dataHeatBalSurfMgr->RefAirTemp(surfNum);
            }

            // Following variables must be reset due to possible recall of this routine by radiant and Resimulate routines.
            // CalcWindowHeatBalance is called, then, multiple times and these need to be initialized before each call to
            // CalcWindowHeatBalance.
            // Only for Surface(SurfNum).Class == DataSurfaces::SurfaceClass::Window
            int const firstWindowSurf = thisSpace.WindowSurfaceFirst;
            int const lastWindowSurf = thisSpace.WindowSurfaceLast;
            for (int surfNum = firstWindowSurf; surfNum <= lastWindowSurf; ++surfNum) {
                state.dataSurface->SurfWinHeatGain(surfNum) = 0.0;
                state.dataSurface->SurfWinHeatGainRep(surfNum) = 0.0;
                state.dataSurface->SurfWinHeatLossRep(surfNum) = 0.0;
                state.dataSurface->SurfWinGainConvGlazToZoneRep(surfNum) = 0.0;
                state.dataSurface->SurfWinGainIRGlazToZoneRep(surfNum) = 0.0;
                state.dataSurface->SurfWinLossSWZoneToOutWinRep(surfNum) = 0.0;
                state.dataSurface->SurfWinGainFrameDividerToZoneRep(surfNum) = 0.0;
                state.dataSurface->SurfWinGainConvShadeToZoneRep(surfNum) = 0.0;
                state.dataSurface->SurfWinGainIRShadeToZoneRep(surfNum) = 0.0;
                state.dataSurface->SurfWinFrameQRadOutAbs(surfNum) = 0.0;
                state.dataSurface->SurfWinFrameQRadInAbs(surfNum) = 0.0;
                state.dataSurface->SurfWinDividerQRadOutAbs(surfNum) = 0.0;
                state.dataSurface->SurfWinDividerQRadInAbs(surfNum) = 0.0;
            }

            // Calculate heat extract due to additional heat flux source term as the surface boundary condition
            for (int surfNum : state.dataSurface->allInsideSourceSurfaceList) {
                state.dataHeatBalSurf->SurfQAdditionalHeatSourceInside(surfNum) =
                    state.dataSurface->Surface(surfNum).insideHeatSourceTermSched->getCurrentVal();
            }

            // Set up coefficient arrays prior to calculations and precalc terms that do no change during iteration - non-window surfaces
            int const firstNonWinSurf = thisSpace.OpaqOrIntMassSurfaceFirst;
            int const lastNonWinSurf = thisSpace.OpaqOrIntMassSurfaceLast;
            Real64 const timeStepZoneSeconds = state.dataGlobal->TimeStepZoneSec; // local for vectorization
            Real64 const iterDampConstant = DataHeatBalSurface::IterDampConst;    // local for vectorization
            // this loop auto-vectorizes
            for (int surfNum = firstNonWinSurf; surfNum <= lastNonWinSurf; ++surfNum) {
                auto const &surface = Surface(surfNum);
                if (state.dataSurface->UseRepresentativeSurfaceCalculations) {
                    int repSurfNum = surface.RepresentativeCalcSurfNum;
                    if (surfNum != repSurfNum) {
                        continue;
                    }
                }

                // Pre-calculate a few terms before the iteration loop
                state.dataHeatBalSurf->SurfTempTerm(surfNum) =
                    state.dataHeatBalSurf->SurfCTFConstInPart(surfNum) + state.dataHeatBal->SurfQdotRadIntGainsInPerArea(surfNum) +
                    state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(surfNum) + state.dataHeatBalSurf->SurfQAdditionalHeatSourceInside(surfNum) +
                    state.dataHeatBalSurf->SurfHConvInt(surfNum) * state.dataHeatBalSurfMgr->RefAirTemp(surfNum) +
                    state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(surfNum) +
                    (state.dataHeatBalFanSys->QRadSurfAFNDuct(surfNum) / timeStepZoneSeconds);
                state.dataHeatBalSurf->SurfTempDiv(surfNum) =
                    1.0 / (state.dataHeatBalSurf->SurfCTFInside0(surfNum) -
                           state.dataHeatBalSurf->SurfIsAdiabatic(surfNum) * state.dataHeatBalSurf->SurfCTFCross0(surfNum) +
                           state.dataHeatBalSurf->SurfIsOperatingPool(surfNum) * state.dataHeatBalFanSys->PoolHeatTransCoefs(surfNum) +
                           (!state.dataHeatBalSurf->SurfIsOperatingPool(surfNum)) * state.dataHeatBalSurf->SurfHConvInt(surfNum) + iterDampConstant);
            }
        }
    }

    state.dataHeatBal->InsideSurfIterations = 0;
    bool Converged = false; // .TRUE. if inside heat balance has converged
    while (!Converged) {    // Start of main inside heat balance iteration loop...

        state.dataHeatBalSurf->SurfTempInsOld = state.dataHeatBalSurf->SurfTempIn; // Keep track of last iteration's temperature values

        HeatBalanceIntRadExchange::CalcInteriorRadExchange(state,
                                                           state.dataHeatBalSurf->SurfTempIn,
                                                           state.dataHeatBal->InsideSurfIterations,
                                                           state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea,
                                                           ZoneToResimulate,
                                                           Inside); // Update the radiation balance

        // Every 30 iterations, recalculate the inside convection coefficients in case
        // there has been a significant drift in the surface temperatures predicted.
        // This is not fool-proof and it basically means that the outside surface
        // heat balance is in error (potentially) once HConvIn is re-evaluated.
        // The choice of 30 is not significant--just want to do this a couple of
        // times before the iteration limit is hit.
        if ((state.dataHeatBal->InsideSurfIterations > 0) &&
            (mod(state.dataHeatBal->InsideSurfIterations, DataHeatBalSurface::ItersReevalConvCoeff) == 0)) {
            Convect::InitIntConvCoeff(state, state.dataHeatBalSurf->SurfTempIn, ZoneToResimulate);
            // Since HConvIn has changed re-calculate a few terms - non-window surfaces
            for (int zoneNum = FirstZone; zoneNum <= LastZone; ++zoneNum) {
                for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                    auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                    int const firstSurf = thisSpace.OpaqOrIntMassSurfaceFirst;
                    int const lastSurf = thisSpace.OpaqOrIntMassSurfaceLast;

                    Real64 const timeStepZoneSeconds = state.dataGlobal->TimeStepZoneSec; // local for vectorization
                    Real64 const iterDampConstant = DataHeatBalSurface::IterDampConst;    // local for vectorization
                    // this loop auto-vectorizes
                    for (int surfNum = firstSurf; surfNum <= lastSurf; ++surfNum) {
                        auto const &surface = Surface(surfNum);
                        if (state.dataSurface->UseRepresentativeSurfaceCalculations) {
                            int repSurfNum = surface.RepresentativeCalcSurfNum;
                            if (surfNum != repSurfNum) {
                                continue;
                            }
                        }

                        state.dataHeatBalSurf->SurfTempTerm(surfNum) =
                            state.dataHeatBalSurf->SurfCTFConstInPart(surfNum) + state.dataHeatBal->SurfQdotRadIntGainsInPerArea(surfNum) +
                            state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(surfNum) + state.dataHeatBalSurf->SurfQAdditionalHeatSourceInside(surfNum) +
                            state.dataHeatBalSurf->SurfHConvInt(surfNum) * state.dataHeatBalSurfMgr->RefAirTemp(surfNum) +
                            state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(surfNum) +
                            (state.dataHeatBalFanSys->QRadSurfAFNDuct(surfNum) / timeStepZoneSeconds);
                        state.dataHeatBalSurf->SurfTempDiv(surfNum) =
                            1.0 / (state.dataHeatBalSurf->SurfCTFInside0(surfNum) -
                                   state.dataHeatBalSurf->SurfIsAdiabatic(surfNum) * state.dataHeatBalSurf->SurfCTFCross0(surfNum) +
                                   state.dataHeatBalSurf->SurfIsOperatingPool(surfNum) * state.dataHeatBalFanSys->PoolHeatTransCoefs(surfNum) +
                                   (!state.dataHeatBalSurf->SurfIsOperatingPool(surfNum)) * state.dataHeatBalSurf->SurfHConvInt(surfNum) +
                                   iterDampConstant);
                    }
                }
            }
        }

        // Loop over non-window surfaces
        for (int zoneNum = FirstZone; zoneNum <= LastZone; ++zoneNum) {
            for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                int const firstNonWinSurf = thisSpace.OpaqOrIntMassSurfaceFirst;
                int const lastNonWinSurf = thisSpace.OpaqOrIntMassSurfaceLast;
                Real64 const iterDampConstant = DataHeatBalSurface::IterDampConst; // local for vectorization
                // this loop auto-vectorizes
                for (int surfNum = firstNonWinSurf; surfNum <= lastNonWinSurf; ++surfNum) {
                    // Perform heat balance on the inside face of the surface ...
                    // The following are possibilities here (this function only does CTF, see CalcHeatBalanceInsideSurf2 for others):
                    //   (a) the surface is a pool (no movable insulation, no source/sink, only CTF solution algorithm)
                    //   (b) the surface is adiabatic (a partition), in which case the temperature of both sides are the same
                    //   (c) standard (or interzone) opaque surface with no movable insulation, normal heat balance equation
                    //   (d) standard (or interzone) window: call to CalcWindowHeatBalance to get window layer temperatures
                    //   (e) standard opaque surface with movable insulation, special two-part equation
                    // In the surface calculation there are the following Algorithm types for opaque surfaces that
                    // do not have movable insulation:
                    //   (a) the regular CTF calc (SolutionAlgo = UseCTF)
                    //   (b) the EMPD calc (Solutionalgo = UseEMPD)
                    //   (c) the CondFD calc (SolutionAlgo = UseCondFD)
                    //   (d) the HAMT calc (solutionalgo = UseHAMT).

                    // For adiabatic surface:
                    // Adiabatic:   TempDiv = (1.0 / (construct.CTFInside(0) - construct.CTFCross[0] + HConvIn_surf + IterDampConst));
                    // Adiabatic:   SurfTempInTmp(SurfNum) = (TempTerm + IterDampConst * SurfTempInsOld(SurfNum)) * TempDiv;
                    // Ad+Source:   SurfTempInTmp(SurfNum) = (TempTerm + construct.CTFSourceIn[0] * SurfQsrcHist(SurfNum, 1) + IterDampConst *
                    // SurfTempInsOld(SurfNum)) * TempDiv; Ad+Pool:     TempDiv = (1.0 / (construct.CTFInside(0) - construct.CTFCross[0] +
                    // PoolHeatTransCoefs(SurfNum) + IterDampConst); Ad+Pool:     SurfTempInTmp(SurfNum) = (SurfCTFConstInPart(SurfNum) +
                    // QPoolSurfNumerator(SurfNum) + IterDampConst * SurfTempInsOld(SurfNum)) * TempDiv;

                    // For standard or interzone surface:
                    // Standard:    TempDiv = (1.0 / (construct.CTFInside(0) + HConvIn_surf + IterDampConst));
                    // Standard:    SurfTempInTmp(SurfNum) = (TempTerm + IterDampConst * SurfTempInsOld(SurfNum) + construct.CTFCross[0] * TH11) *
                    // TempDiv; Std+Source:  SurfTempInTmp(SurfNum) = (TempTerm + construct.CTFSourceIn[0] * SurfQsrcHist(SurfNum, 1) + IterDampConst
                    // * SurfTempInsOld(SurfNum)) * TempDiv; Std+Pool:    TempDiv = (1.0 / (construct.CTFInside(0) + PoolHeatTransCoefs(SurfNum) +
                    // IterDampConst); Std+Pool:    SurfTempInTmp(SurfNum) = (SurfCTFConstInPart(SurfNum) + QPoolSurfNumerator(SurfNum) +
                    // IterDampConst* SurfTempInsOld(SurfNum) + construct.CTFCross[0] * TH11) * TempDiv;

                    // Composite with Adiabatic/Source/Pool flags:
                    //              TempDiv = (1.0 / (construct.CTFInside(0) - SurfIsAdiabatic*construct.CTFCross[0]+
                    //              SurfIsOperatingPool*PoolHeatTransCoefs(SurfNum) + IsNotPoolSurf*HConvIn_surf + IterDampConst));
                    //              SurfTempInTmp(SurfNum) = (IsNotPoolSurf*TempTerm + IsSource*construct.CTFSourceIn[0] * SurfQsrcHist(SurfNum, 1) +
                    //              SurfIsOperatingPool*SurfCTFConstInPart(SurfNum) + SurfIsOperatingPool*QPoolSurfNumerator(SurfNum)
                    //                                        + IterDampConst * SurfTempInsOld(SurfNum)+
                    //                                        IsNotAdiabatic*IsNotSource*construct.CTFCross[0]
                    //                                        * TH11) * TempDiv;

                    // Calculate the current inside surface temperature
                    state.dataHeatBalSurf->SurfTempInTmp(surfNum) =
                        ((!state.dataHeatBalSurf->SurfIsOperatingPool(surfNum)) *
                             (state.dataHeatBalSurf->SurfTempTerm(surfNum) + state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(surfNum)) +
                         state.dataHeatBalSurf->SurfIsSourceOrSink(surfNum) * state.dataHeatBalSurf->SurfCTFSourceIn0(surfNum) *
                             state.dataHeatBalSurf->SurfQSourceSinkHist(surfNum) +
                         state.dataHeatBalSurf->SurfIsOperatingPool(surfNum) * state.dataHeatBalSurf->SurfCTFConstInPart(surfNum) +
                         state.dataHeatBalSurf->SurfIsOperatingPool(surfNum) * state.dataHeatBalFanSys->QPoolSurfNumerator(surfNum) +
                         iterDampConstant * state.dataHeatBalSurf->SurfTempInsOld(surfNum) +
                         (!state.dataHeatBalSurf->SurfIsAdiabatic(surfNum)) * state.dataHeatBalSurf->SurfCTFCross0(surfNum) *
                             state.dataHeatBalSurf->SurfTempOutHist(surfNum)) *
                        state.dataHeatBalSurf->SurfTempDiv(surfNum);
                    // Constant part of conduction eq (history terms) | LW radiation from internal sources | SW
                    // radiation from internal sources | Convection from surface to zone air | Net radiant
                    // exchange with other zone surfaces | Heat source/sink term for radiant systems | (if there
                    // is one present) | Radiant flux from high temp radiant heater | Radiant flux from a hot
                    // water baseboard heater | Radiant flux from a steam baseboard heater | Radiant flux from
                    // an electric baseboard heater | Iterative damping term (for stability) | Current
                    // conduction from | the outside surface | Coefficient for conduction (current time) |
                    // Convection and damping term | Radiation from AFN ducts

                    state.dataHeatBalSurf->SurfTempIn(surfNum) = state.dataHeatBalSurf->SurfTempInTmp(surfNum);
                }

                // Loop over non-window surfaces (includes TubularDaylightingDomes)
                for (int surfNum = firstNonWinSurf; surfNum <= lastNonWinSurf; ++surfNum) {
                    auto &movInsul = state.dataSurface->intMovInsuls(surfNum);

                    bool movableInsulPresent = state.dataSurface->AnyMovableInsulation && movInsul.present;
                    if (movableInsulPresent) { // Movable insulation present, recalc surface temps
                        Real64 HMovInsul = movInsul.H;
                        Real64 F1 = HMovInsul / (HMovInsul + state.dataHeatBalSurf->SurfHConvInt(surfNum) + DataHeatBalSurface::IterDampConst);
                        state.dataHeatBalSurf->SurfTempIn(surfNum) =
                            (state.dataHeatBalSurf->SurfCTFConstInPart(surfNum) + state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(surfNum) +
                             state.dataHeatBalSurf->SurfCTFCross0(surfNum) * state.dataHeatBalSurf->SurfTempOutHist(surfNum) +
                             F1 * (state.dataHeatBal->SurfQdotRadIntGainsInPerArea(surfNum) +
                                   state.dataHeatBalSurf->SurfHConvInt(surfNum) * state.dataHeatBalSurfMgr->RefAirTemp(surfNum) +
                                   state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(surfNum) +
                                   state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(surfNum) +
                                   state.dataHeatBalSurf->SurfQAdditionalHeatSourceInside(surfNum) +
                                   DataHeatBalSurface::IterDampConst * state.dataHeatBalSurf->SurfTempInsOld(surfNum))) /
                            (state.dataHeatBalSurf->SurfCTFInside0(surfNum) + HMovInsul - F1 * HMovInsul); // Convection from surface to zone air

                        state.dataHeatBalSurf->SurfTempInTmp(surfNum) =
                            (state.dataHeatBalSurf->SurfCTFInside0(surfNum) * state.dataHeatBalSurf->SurfTempIn(surfNum) +
                             HMovInsul * state.dataHeatBalSurf->SurfTempIn(surfNum) - state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(surfNum) -
                             state.dataHeatBalSurf->SurfCTFConstInPart(surfNum) -
                             state.dataHeatBalSurf->SurfCTFCross0(surfNum) * state.dataHeatBalSurf->SurfTempOutHist(surfNum)) /
                            (HMovInsul);
                    }

                    if (state.dataHeatBal->AnyInternalHeatSourceInInput) {
                        if (state.dataConstruction->Construct(Surface(surfNum).Construction).SourceSinkPresent) {
                            // Set the appropriate parameters for the radiant system
                            // Radiant system does not need the damping coefficient terms (hopefully)
                            Real64 const RadSysDiv(1.0 /
                                                   (state.dataHeatBalSurf->SurfCTFInside0(surfNum) + state.dataHeatBalSurf->SurfHConvInt(surfNum)));
                            Real64 const TempTerm(
                                state.dataHeatBalSurf->SurfCTFConstInPart(surfNum) + state.dataHeatBal->SurfQdotRadIntGainsInPerArea(surfNum) +
                                state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(surfNum) +
                                state.dataHeatBalSurf->SurfQAdditionalHeatSourceInside(surfNum) +
                                state.dataHeatBalSurf->SurfHConvInt(surfNum) * state.dataHeatBalSurfMgr->RefAirTemp(surfNum) +
                                state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(surfNum) + state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(surfNum) +
                                (state.dataHeatBalFanSys->QRadSurfAFNDuct(surfNum) / state.dataGlobal->TimeStepZoneSec));
                            state.dataHeatBalFanSys->RadSysTiHBConstCoef(surfNum) =
                                TempTerm * RadSysDiv; // Constant portion of cond eq (history terms) | LW radiation from internal sources | SW
                            // radiation from internal sources | Convection from surface to zone air | Radiant flux
                            // from high temp radiant heater | Radiant flux from a hot water baseboard heater |
                            // Radiant flux from a steam baseboard heater | Radiant flux from an electric baseboard
                            // heater | Net radiant exchange with other zone surfaces | Cond term (both partition
                            // sides same temp) | Convection and damping term
                            state.dataHeatBalFanSys->RadSysTiHBToutCoef(surfNum) =
                                state.dataHeatBalSurf->SurfCTFCross0(surfNum) * RadSysDiv; // Outside temp=inside temp for a partition |
                            // Cond term (both partition sides same temp) |
                            // Convection and damping term
                            state.dataHeatBalFanSys->RadSysTiHBQsrcCoef(surfNum) =
                                state.dataHeatBalSurf->SurfCTFSourceIn0(surfNum) * RadSysDiv; // QTF term for the source | Cond term (both
                            // partition sides same temp) | Convection and
                            // damping term

                            if (Surface(surfNum).ExtBoundCond > 0) { // This is an interzone partition and we need to set outside params
                                // The inside coefficients of one side are equal to the outside coefficients of the other side.  But,
                                // the inside coefficients are set up once the heat balance equation for that side has been calculated.
                                // For both sides to actually have been set, we have to wait until we get to the second side in the surface
                                // derived type.  At that point, both inside coefficient sets have been evaluated.
                                if (Surface(surfNum).ExtBoundCond <= surfNum) { // Both of the inside coefficients have now been set
                                    int OtherSideSurfNum = Surface(surfNum).ExtBoundCond;
                                    state.dataHeatBalFanSys->RadSysToHBConstCoef(OtherSideSurfNum) =
                                        state.dataHeatBalFanSys->RadSysTiHBConstCoef(surfNum);
                                    state.dataHeatBalFanSys->RadSysToHBTinCoef(OtherSideSurfNum) =
                                        state.dataHeatBalFanSys->RadSysTiHBToutCoef(surfNum);
                                    state.dataHeatBalFanSys->RadSysToHBQsrcCoef(OtherSideSurfNum) =
                                        state.dataHeatBalFanSys->RadSysTiHBQsrcCoef(surfNum);
                                    state.dataHeatBalFanSys->RadSysToHBConstCoef(surfNum) =
                                        state.dataHeatBalFanSys->RadSysTiHBConstCoef(OtherSideSurfNum);
                                    state.dataHeatBalFanSys->RadSysToHBTinCoef(surfNum) =
                                        state.dataHeatBalFanSys->RadSysTiHBToutCoef(OtherSideSurfNum);
                                    state.dataHeatBalFanSys->RadSysToHBQsrcCoef(surfNum) =
                                        state.dataHeatBalFanSys->RadSysTiHBQsrcCoef(OtherSideSurfNum);
                                }
                            }
                        }
                    }
                }

                // Loop over window surfaces
                int const firstWindowSurf = thisSpace.WindowSurfaceFirst;
                int const lastWindowSurf = thisSpace.WindowSurfaceLast;
                for (int surfNum = firstWindowSurf; surfNum <= lastWindowSurf; ++surfNum) {
                    auto &surface = state.dataSurface->Surface(surfNum);
                    if (state.dataSurface->UseRepresentativeSurfaceCalculations) {
                        int repSurfNum = surface.RepresentativeCalcSurfNum;
                        if (surfNum != repSurfNum) {
                            continue;
                        }
                    }
                    Real64 &TH11(state.dataHeatBalSurf->SurfOutsideTempHist(1)(surfNum));
                    int const ConstrNum = state.dataSurface->SurfActiveConstruction(surfNum);
                    auto const &construct = state.dataConstruction->Construct(ConstrNum);
                    if (surface.OriginalClass == DataSurfaces::SurfaceClass::TDD_Diffuser) { // Tubular daylighting device
                        // Lookup up the TDD:DOME object
                        int const pipeNum = state.dataSurface->SurfWinTDDPipeNum(surfNum);
                        int const domeNum = state.dataDaylightingDevicesData->TDDPipe(pipeNum).Dome;
                        // Ueff = 1 / effective R value between TDD:DOME and TDD:DIFFUSER
                        Real64 Ueff = 1.0 / state.dataDaylightingDevicesData->TDDPipe(pipeNum).Reff;

                        // Similar to opaque surface but outside surface temp of TDD:DOME is used, and no embedded sources/sinks.
                        // Absorbed shortwave radiation is treated similar to a regular window, but only 1 glass layer is allowed.
                        //   = SurfWinQRadSWwinAbs(surfNum,1)/2.0
                        Real64 const HConvIn_surf(state.dataMstBal->HConvInFD(surfNum) = state.dataHeatBalSurf->SurfHConvInt(surfNum));
                        state.dataHeatBalSurf->SurfTempInTmp(surfNum) =
                            (state.dataHeatBal->SurfQdotRadIntGainsInPerArea(surfNum) + state.dataHeatBal->SurfWinQRadSWwinAbs(surfNum, 1) / 2.0 +
                             state.dataHeatBalSurf->SurfQAdditionalHeatSourceInside(surfNum) +
                             HConvIn_surf * state.dataHeatBalSurfMgr->RefAirTemp(surfNum) +
                             state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(surfNum) +
                             DataHeatBalSurface::IterDampConst * state.dataHeatBalSurf->SurfTempInsOld(surfNum) +
                             Ueff * state.dataHeatBalSurf->SurfOutsideTempHist(1)(domeNum)) /
                            (Ueff + HConvIn_surf +
                             DataHeatBalSurface::IterDampConst); // LW radiation from internal sources | SW radiation from internal sources and
                                                                 // solar | Convection from surface to zone air | Net radiant exchange with
                                                                 // other zone surfaces | Iterative damping term (for stability) | Current
                                                                 // conduction from the outside surface | Coefficient for conduction (current
                                                                 // time) | Convection and damping term
                        state.dataHeatBalSurf->SurfTempIn(surfNum) = state.dataHeatBalSurf->SurfTempInTmp(surfNum);
                        Real64 const Sigma_Temp_4(Constant::StefanBoltzmann * pow_4(state.dataHeatBalSurf->SurfTempIn(surfNum) + Constant::Kelvin));

                        // fill out report vars for components of Window Heat Gain
                        state.dataSurface->SurfWinGainConvGlazToZoneRep(surfNum) =
                            HConvIn_surf * surface.Area *
                            (state.dataHeatBalSurf->SurfTempIn(surfNum) - state.dataHeatBalSurfMgr->RefAirTemp(surfNum));
                        state.dataSurface->SurfWinGainIRGlazToZoneRep(surfNum) =
                            state.dataConstruction->Construct(surface.Construction).InsideAbsorpThermal * surface.Area *
                            (Sigma_Temp_4 -
                             (state.dataSurface->SurfWinIRfromParentZone(surfNum) + state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(surfNum)));
                        state.dataSurface->SurfWinLossSWZoneToOutWinRep(surfNum) =
                            state.dataHeatBal->EnclSolQSWRad(surface.SolarEnclIndex) * surface.Area *
                                (1 - state.dataConstruction->Construct(surface.Construction).ReflectSolDiffBack) +
                            state.dataHeatBalSurf->SurfWinInitialBeamSolInTrans(surfNum);

                        // Calculate window heat gain for TDD:DIFFUSER since this calculation is usually done in WindowManager
                        state.dataSurface->SurfWinHeatGain(surfNum) =
                            state.dataSurface->SurfWinTransSolar(surfNum) + state.dataSurface->SurfWinGainConvGlazToZoneRep(surfNum) +
                            state.dataSurface->SurfWinGainIRGlazToZoneRep(surfNum) - state.dataSurface->SurfWinLossSWZoneToOutWinRep(surfNum) -
                            surface.Area * state.dataHeatBalSurf->SurfWinInitialDifSolInTrans(surfNum);
                        // Net transmitted solar | Convection | IR exchange | IR
                        // Zone diffuse interior shortwave reflected back into the TDD
                    } else {                                                // Regular window
                        if (state.dataHeatBal->InsideSurfIterations == 0) { // Do windows only once
                            // Get outside convection coeff for exterior window here to avoid calling
                            // InitExteriorConvectionCoeff from CalcWindowHeatBalance, which avoids circular reference
                            // (HeatBalanceSurfaceManager USEing and WindowManager and
                            // WindowManager USEing HeatBalanceSurfaceManager)
                            if (surface.ExtBoundCond == DataSurfaces::ExternalEnvironment) {
                                auto const *thisMaterial = s_mat->materials(construct.LayerPoint(1));
                                assert(thisMaterial != nullptr);
                                Material::SurfaceRoughness RoughSurf = thisMaterial->Roughness; // Outside surface roughness
                                Real64 EmisOut = thisMaterial->AbsorpThermalFront;              // Glass outside surface emissivity
                                DataSurfaces::WinShadingType const shading_flag = state.dataSurface->SurfWinShadingFlag(surfNum);
                                if (DataSurfaces::ANY_EXTERIOR_SHADE_BLIND_SCREEN(shading_flag)) {
                                    // Exterior shade in place
                                    int const ConstrNumSh = Surface(surfNum).activeShadedConstruction;
                                    if (ConstrNumSh != 0) {
                                        auto const &constructionSh = state.dataConstruction->Construct(ConstrNumSh);
                                        auto const *thisMaterial2 = s_mat->materials(constructionSh.LayerPoint(1));
                                        RoughSurf = thisMaterial2->Roughness;
                                        EmisOut = thisMaterial2->AbsorpThermal;
                                    }
                                }

                                // Get the outside effective emissivity for Equivalent layer model
                                if (construct.WindowTypeEQL) {
                                    EmisOut = WindowEquivalentLayer::EQLWindowOutsideEffectiveEmiss(state, ConstrNum);
                                }
                                // Set Exterior Convection Coefficient...
                                if (state.dataSurface->surfExtConv(surfNum).userModelNum != 0) {

                                    state.dataHeatBalSurf->SurfHConvExt(surfNum) = Convect::SetExtConvCoeff(state, surfNum);

                                } else if (surface.ExtWind) { // Window is exposed to wind (and possibly rain)

                                    // Calculate exterior heat transfer coefficients with windspeed (windspeed is calculated internally in
                                    // subroutine)
                                    Convect::InitExtConvCoeff(state,
                                                              surfNum,
                                                              0.0,
                                                              RoughSurf,
                                                              EmisOut,
                                                              TH11,
                                                              state.dataHeatBalSurf->SurfHConvExt(surfNum),
                                                              state.dataHeatBalSurf->SurfHSkyExt(surfNum),
                                                              state.dataHeatBalSurf->SurfHGrdExt(surfNum),
                                                              state.dataHeatBalSurf->SurfHAirExt(surfNum),
                                                              state.dataHeatBalSurf->SurfHSrdSurfExt(surfNum));

                                    if (state.dataEnvrn->IsRain) { // Raining: since wind exposed, outside window surface gets wet
                                        state.dataHeatBalSurf->SurfHConvExt(surfNum) = 1000.0; // Reset SurfHcExt because of wetness
                                    }

                                } else { // Not Wind exposed

                                    // Calculate exterior heat transfer coefficients for windspeed = 0
                                    Convect::InitExtConvCoeff(state,
                                                              surfNum,
                                                              0.0,
                                                              RoughSurf,
                                                              EmisOut,
                                                              TH11,
                                                              state.dataHeatBalSurf->SurfHConvExt(surfNum),
                                                              state.dataHeatBalSurf->SurfHSkyExt(surfNum),
                                                              state.dataHeatBalSurf->SurfHGrdExt(surfNum),
                                                              state.dataHeatBalSurf->SurfHAirExt(surfNum),
                                                              state.dataHeatBalSurf->SurfHSrdSurfExt(surfNum));
                                }

                            } else { // Interior Surface

                                if (state.dataSurface->surfExtConv(surfNum).userModelNum != 0) {
                                    state.dataHeatBalSurf->SurfHConvExt(surfNum) = Convect::SetExtConvCoeff(state, surfNum);
                                } else {
                                    // Exterior Convection Coefficient for the Interior or Interzone Window is the Interior Convection Coeff of
                                    // same
                                    state.dataHeatBalSurf->SurfHConvExt(surfNum) = state.dataHeatBalSurf->SurfHConvInt(surface.ExtBoundCond);
                                }
                            }

                            // Following call determines inside surface temperature of glazing, and of
                            // frame and/or divider, if present
                            Window::CalcWindowHeatBalance(
                                state, surfNum, state.dataHeatBalSurf->SurfHConvExt(surfNum), state.dataHeatBalSurf->SurfTempInTmp(surfNum), TH11);
                            state.dataHeatBalSurf->SurfTempIn(surfNum) = state.dataHeatBalSurf->SurfTempInTmp(surfNum);
                        }
                    }
                }

                int const firstSurf = thisSpace.OpaqOrWinSurfaceFirst;
                int const lastSurf = thisSpace.OpaqOrWinSurfaceLast;
                for (int surfNum = firstSurf; surfNum <= lastSurf; ++surfNum) {
                    auto &zone = state.dataHeatBal->Zone(zoneNum);

                    Real64 &TH11 = state.dataHeatBalSurf->SurfOutsideTempHist(1)(surfNum);
                    Real64 &TH12 = state.dataHeatBalSurf->SurfInsideTempHist(1)(surfNum);
                    TH12 = state.dataHeatBalSurf->SurfTempIn(surfNum);
                    state.dataHeatBalSurf->SurfTempOut(surfNum) = TH11;                                                  // For reporting
                    if (state.dataSurface->Surface(surfNum).OriginalClass == DataSurfaces::SurfaceClass::TDD_Diffuser) { // Tubular daylighting device
                        // Tubular daylighting devices are treated as one big object with an effective R value.
                        // The outside face temperature of the TDD:DOME and the inside face temperature of the
                        // TDD:DIFFUSER are calculated with the outside and inside heat balances respectively.
                        // Below, the resulting temperatures are copied to the inside face of the TDD:DOME
                        // and the outside face of the TDD:DIFFUSER for reporting.

                        // Set inside temp variables of TDD:DOME equal to inside temp of TDD:DIFFUSER
                        int domeNum = state.dataDaylightingDevicesData->TDDPipe(state.dataSurface->SurfWinTDDPipeNum(surfNum)).Dome;
                        state.dataHeatBalSurf->SurfInsideTempHist(1)(domeNum) = state.dataHeatBalSurf->SurfTempIn(domeNum) =
                            state.dataHeatBalSurf->SurfTempInTmp(domeNum) = state.dataHeatBalSurf->SurfTempIn(surfNum);

                        // Set outside temp reporting variable of TDD:DOME (since it gets skipped otherwise)
                        // Reset outside temp variables of TDD:DIFFUSER equal to outside temp of TDD:DOME
                        TH11 = state.dataHeatBalSurf->SurfTempOut(surfNum) = state.dataHeatBalSurf->SurfTempOut(domeNum) =
                            state.dataHeatBalSurf->SurfOutsideTempHist(1)(domeNum);
                    }

                    if ((TH12 > state.dataHeatBalSurf->MaxSurfaceTempLimit) || (TH12 < DataHeatBalSurface::MinSurfaceTempLimit)) {
                        TestSurfTempCalcHeatBalanceInsideSurf(
                            state, TH12, surfNum, zone, state.dataHeatBalSurfMgr->calcHeatBalInsideSurfWarmupErrCount);
                    }
                }
            }
        } // ...end of main loops over all surfaces for inside heat balances

        // Interzone surface updating: interzone surfaces have other side temperatures
        // which can vary as the simulation iterates through the inside heat
        // balance.  This block is intended to "lock" the opposite side (outside)
        // temperatures to the correct value, namely the value calculated by the
        // inside surface heat balance for the other side.
        //        assert(state.dataHeatBalSurf->TH.index(1, 1, 1) == 0u); // Assumed for linear indexing below
        //        int const l211(state.dataHeatBalSurf->TH.index(2, 1, 1) - 1);
        for (int SurfNum : IZSurfs) {
            int const surfExtBoundCond = Surface(SurfNum).ExtBoundCond;
            // Set the outside surface temperature to the inside surface temperature of the interzone pair.
            // By going through all of the surfaces, this should pick up the other side as well as affect the next iteration.
            // [ SurfNum - 1 ] == ( 1, 1, SurfNum )
            // [ l211 + surfExtBoundCond ] == ( 2, 1, surfExtBoundCond )
            state.dataHeatBalSurf->SurfTempOut(SurfNum) = state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum) =
                state.dataHeatBalSurf->SurfInsideTempHist(1)(surfExtBoundCond);
            state.dataHeatBalSurf->SurfTempOutHist(SurfNum) = state.dataHeatBalSurf->SurfTempOut(SurfNum);
        }

        ++state.dataHeatBal->InsideSurfIterations;

        // Convergence check - Loop through all relevant non-window surfaces to check for convergence...
        Real64 MaxDelTemp = 0.0; // Maximum change in surface temperature for any opaque surface from one iteration to the next
        for (int zoneNum = FirstZone; zoneNum <= LastZone; ++zoneNum) {
            for (int spaceNum : state.dataHeatBal->Zone(zoneNum).spaceIndexes) {
                auto const &thisSpace = state.dataHeatBal->space(spaceNum);
                int const firstNonWinSurf = thisSpace.OpaqOrIntMassSurfaceFirst;
                int const lastNonWinSurf = thisSpace.OpaqOrIntMassSurfaceLast;
                for (int surfNum = firstNonWinSurf; surfNum <= lastNonWinSurf; ++surfNum) {
                    Real64 delta = state.dataHeatBalSurf->SurfTempIn(surfNum) - state.dataHeatBalSurf->SurfTempInsOld(surfNum);
                    Real64 absDif = std::abs(delta);
                    MaxDelTemp = std::max(absDif, MaxDelTemp);
                }
            }
        } // ...end of loop to check for convergence

        if (MaxDelTemp <= state.dataHeatBal->MaxAllowedDelTemp) {
            Converged = true;
        }

#ifdef EP_Count_Calls
        state.dataTimingsData->NumMaxInsideSurfIterations =
            max(state.dataTimingsData->NumMaxInsideSurfIterations, state.dataHeatBal->InsideSurfIterations);
#endif

        if (state.dataHeatBal->InsideSurfIterations < state.dataHeatBalSurf->MinIterations) {
            Converged = false;
        }

        if (state.dataHeatBal->InsideSurfIterations > DataHeatBalSurface::MaxIterations) {
            if (!state.dataGlobal->WarmupFlag) {
                ++state.dataHeatBalSurfMgr->calcHeatBalInsideSurfErrCount;
                if (state.dataHeatBalSurfMgr->calcHeatBalInsideSurfErrCount < 16) {
                    ShowWarningError(state,
                                     format("Inside surface heat balance did not converge with Max Temp Difference [C] ={:.3R} vs Max Allowed "
                                            "Temp Diff [C] ={:.6R}",
                                            MaxDelTemp,
                                            state.dataHeatBal->MaxAllowedDelTempCondFD));
                    ShowContinueErrorTimeStamp(state, "");
                } else {
                    ShowRecurringWarningErrorAtEnd(state,
                                                   "Inside surface heat balance convergence problem continues",
                                                   state.dataHeatBalSurfMgr->calcHeatBalInsideSurfErrPointer,
                                                   MaxDelTemp,
                                                   MaxDelTemp,
                                                   _,
                                                   "[C]",
                                                   "[C]");
                }
            }
            break; // iteration loop
        }

    } // ...end of main inside heat balance iteration loop (ends when Converged)
}

void sumSurfQdotRadHVAC(EnergyPlusData &state)
{
    for (int surfNum : state.dataSurface->allGetsRadiantHeatSurfaceList) {
        auto const &thisSurfQRadFromHVAC = state.dataHeatBalFanSys->surfQRadFromHVAC(surfNum);
        state.dataHeatBalSurf->SurfQdotRadHVACInPerArea(surfNum) = thisSurfQRadFromHVAC.HTRadSys + thisSurfQRadFromHVAC.HWBaseboard +
                                                                   thisSurfQRadFromHVAC.SteamBaseboard + thisSurfQRadFromHVAC.ElecBaseboard +
                                                                   thisSurfQRadFromHVAC.CoolingPanel;
    }
}

void TestSurfTempCalcHeatBalanceInsideSurf(EnergyPlusData &state, Real64 TH12, int const SurfNum, DataHeatBalance::ZoneData &zone, int WarmupSurfTemp)
{
    std::string surfName = state.dataSurface->Surface(SurfNum).Name;

    if ((TH12 > state.dataHeatBalSurf->MaxSurfaceTempLimit) || (TH12 < DataHeatBalSurface::MinSurfaceTempLimit)) {
        if (state.dataGlobal->WarmupFlag) {
            ++WarmupSurfTemp;
        }
        if (!state.dataGlobal->WarmupFlag || WarmupSurfTemp > 10 || state.dataGlobal->DisplayExtraWarnings) {
            if (TH12 < DataHeatBalSurface::MinSurfaceTempLimit) {
                if (state.dataSurface->SurfLowTempErrCount(SurfNum) == 0) {
                    ShowSevereMessage(
                        state, format(R"(Temperature (low) out of bounds [{:.2R}] for zone="{}", for surface="{}")", TH12, zone.Name, surfName));
                    ShowContinueErrorTimeStamp(state, "");
                    if (!zone.TempOutOfBoundsReported) {
                        ShowContinueError(state, format("Zone=\"{}\", Diagnostic Details:", zone.Name));
                        if (zone.FloorArea > 0.0) {
                            ShowContinueError(state, format("...Internal Heat Gain [{:.3R}] W/m2", zone.InternalHeatGains / zone.FloorArea));
                        } else {
                            ShowContinueError(state, format("...Internal Heat Gain (no floor) [{:.3R}] W", zone.InternalHeatGains));
                        }
                        if (state.afn->simulation_control.type == AirflowNetwork::ControlType::NoMultizoneOrDistribution) {
                            ShowContinueError(state, format("...Infiltration/Ventilation [{:.3R}] m3/s", zone.NominalInfilVent));
                            ShowContinueError(state, format("...Mixing/Cross Mixing [{:.3R}] m3/s", zone.NominalMixing));
                        } else {
                            ShowContinueError(state, "...Airflow Network Simulation: Nominal Infiltration/Ventilation/Mixing not available.");
                        }
                        if (zone.IsControlled) {
                            ShowContinueError(state, "...Zone is part of HVAC controlled system.");
                        } else {
                            ShowContinueError(state, "...Zone is not part of HVAC controlled system.");
                        }
                        zone.TempOutOfBoundsReported = true;
                    }
                    ShowRecurringSevereErrorAtEnd(state,
                                                  "Temperature (low) out of bounds for zone=" + zone.Name + " for surface=" + surfName,
                                                  state.dataSurface->SurfLowTempErrCount(SurfNum),
                                                  TH12,
                                                  TH12,
                                                  _,
                                                  "C",
                                                  "C");
                } else {
                    ShowRecurringSevereErrorAtEnd(state,
                                                  "Temperature (low) out of bounds for zone=" + zone.Name + " for surface=" + surfName,
                                                  state.dataSurface->SurfLowTempErrCount(SurfNum),
                                                  TH12,
                                                  TH12,
                                                  _,
                                                  "C",
                                                  "C");
                }
            } else {
                if (state.dataSurface->SurfHighTempErrCount(SurfNum) == 0) {
                    ShowSevereMessage(
                        state, format(R"(Temperature (high) out of bounds ({:.2R}] for zone="{}", for surface="{}")", TH12, zone.Name, surfName));
                    ShowContinueErrorTimeStamp(state, "");
                    if (!zone.TempOutOfBoundsReported) {
                        ShowContinueError(state, format("Zone=\"{}\", Diagnostic Details:", zone.Name));
                        if (zone.FloorArea > 0.0) {
                            ShowContinueError(state, format("...Internal Heat Gain [{:.3R}] W/m2", zone.InternalHeatGains / zone.FloorArea));
                        } else {
                            ShowContinueError(state, format("...Internal Heat Gain (no floor) [{:.3R}] W", zone.InternalHeatGains));
                        }
                        if (state.afn->simulation_control.type == AirflowNetwork::ControlType::NoMultizoneOrDistribution) {
                            ShowContinueError(state, format("...Infiltration/Ventilation [{:.3R}] m3/s", zone.NominalInfilVent));
                            ShowContinueError(state, format("...Mixing/Cross Mixing [{:.3R}] m3/s", zone.NominalMixing));
                        } else {
                            ShowContinueError(state, "...Airflow Network Simulation: Nominal Infiltration/Ventilation/Mixing not available.");
                        }
                        if (zone.IsControlled) {
                            ShowContinueError(state, "...Zone is part of HVAC controlled system.");
                        } else {
                            ShowContinueError(state, "...Zone is not part of HVAC controlled system.");
                        }
                        zone.TempOutOfBoundsReported = true;
                    }
                    ShowRecurringSevereErrorAtEnd(state,
                                                  "Temperature (high) out of bounds for zone=" + zone.Name + " for surface=" + surfName,
                                                  state.dataSurface->SurfHighTempErrCount(SurfNum),
                                                  TH12,
                                                  TH12,
                                                  _,
                                                  "C",
                                                  "C");
                } else {
                    ShowRecurringSevereErrorAtEnd(state,
                                                  "Temperature (high) out of bounds for zone=" + zone.Name + " for surface=" + surfName,
                                                  state.dataSurface->SurfHighTempErrCount(SurfNum),
                                                  TH12,
                                                  TH12,
                                                  _,
                                                  "C",
                                                  "C");
                }
            }
            if (zone.EnforcedReciprocity) {
                if (WarmupSurfTemp > 3) {
                    ShowSevereError(state, format("CalcHeatBalanceInsideSurf: Zone=\"{}\" has view factor enforced reciprocity", zone.Name));
                    ShowContinueError(state, " and is having temperature out of bounds errors. Please correct zone geometry and rerun.");
                    ShowFatalError(state, "CalcHeatBalanceInsideSurf: Program terminates due to preceding conditions.");
                }
            } else if (WarmupSurfTemp > 10) {
                ShowFatalError(state, "CalcHeatBalanceInsideSurf: Program terminates due to preceding conditions.");
            }
        }
    }
    if ((TH12 > state.dataHeatBalSurf->MaxSurfaceTempLimitBeforeFatal) || (TH12 < DataHeatBalSurface::MinSurfaceTempLimitBeforeFatal)) {
        if (!state.dataGlobal->WarmupFlag) {
            if (TH12 < DataHeatBalSurface::MinSurfaceTempLimitBeforeFatal) {
                ShowSevereError(state,
                                format(R"(Temperature (low) out of bounds [{:.2R}] for zone="{}", for surface="{}")", TH12, zone.Name, surfName));
                ShowContinueErrorTimeStamp(state, "");
                if (!zone.TempOutOfBoundsReported) {
                    ShowContinueError(state, format("Zone=\"{}\", Diagnostic Details:", zone.Name));
                    if (zone.FloorArea > 0.0) {
                        ShowContinueError(state, format("...Internal Heat Gain [{:.3R}] W/m2", zone.InternalHeatGains / zone.FloorArea));
                    } else {
                        ShowContinueError(state, format("...Internal Heat Gain (no floor) [{:.3R}] W", zone.InternalHeatGains / zone.FloorArea));
                    }
                    if (state.afn->simulation_control.type == AirflowNetwork::ControlType::NoMultizoneOrDistribution) {
                        ShowContinueError(state, format("...Infiltration/Ventilation [{:.3R}] m3/s", zone.NominalInfilVent));
                        ShowContinueError(state, format("...Mixing/Cross Mixing [{:.3R}] m3/s", zone.NominalMixing));
                    } else {
                        ShowContinueError(state, "...Airflow Network Simulation: Nominal Infiltration/Ventilation/Mixing not available.");
                    }
                    if (zone.IsControlled) {
                        ShowContinueError(state, "...Zone is part of HVAC controlled system.");
                    } else {
                        ShowContinueError(state, "...Zone is not part of HVAC controlled system.");
                    }
                    zone.TempOutOfBoundsReported = true;
                }
                ShowFatalError(state, "Program terminates due to preceding condition.");
            } else {
                ShowSevereError(state,
                                format(R"(Temperature (high) out of bounds [{:.2R}] for zone="{}", for surface="{}")", TH12, zone.Name, surfName));
                ShowContinueErrorTimeStamp(state, "");
                if (!zone.TempOutOfBoundsReported) {
                    ShowContinueError(state, format("Zone=\"{}\", Diagnostic Details:", zone.Name));
                    if (zone.FloorArea > 0.0) {
                        ShowContinueError(state, format("...Internal Heat Gain [{:.3R}] W/m2", zone.InternalHeatGains / zone.FloorArea));
                    } else {
                        ShowContinueError(state, format("...Internal Heat Gain (no floor) [{:.3R}] W", zone.InternalHeatGains / zone.FloorArea));
                    }
                    if (state.afn->simulation_control.type == AirflowNetwork::ControlType::NoMultizoneOrDistribution) {
                        ShowContinueError(state, format("...Infiltration/Ventilation [{:.3R}] m3/s", zone.NominalInfilVent));
                        ShowContinueError(state, format("...Mixing/Cross Mixing [{:.3R}] m3/s", zone.NominalMixing));
                    } else {
                        ShowContinueError(state, "...Airflow Network Simulation: Nominal Infiltration/Ventilation/Mixing not available.");
                    }
                    if (zone.IsControlled) {
                        ShowContinueError(state, "...Zone is part of HVAC controlled system.");
                    } else {
                        ShowContinueError(state, "...Zone is not part of HVAC controlled system.");
                    }
                    zone.TempOutOfBoundsReported = true;
                }
                ShowFatalError(state, "Program terminates due to preceding condition.");
            }
        } else {
            if (TH12 < -10000. || TH12 > 10000.) {
                ShowSevereError(
                    state,
                    format(R"(CalcHeatBalanceInsideSurf: The temperature of {:.2R} C for zone="{}", for surface="{}")", TH12, zone.Name, surfName));
                ShowContinueError(state, "..is very far out of bounds during warmup. This may be an indication of a malformed zone.");
                ShowContinueErrorTimeStamp(state, "");
                ShowFatalError(state, "Program terminates due to preceding condition.");
            }
        }
    }
}

void CalcOutsideSurfTemp(EnergyPlusData &state,
                         int const SurfNum,      // Surface number DO loop counter
                         int const spaceNum,     // Space number the current surface is attached to
                         int const ConstrNum,    // Construction index for the current surface
                         Real64 const HMovInsul, // "Convection" coefficient of movable insulation
                         Real64 const TempExt,   // Exterior temperature boundary condition
                         bool &ErrorFlag         // Error flag for movable insulation problem
)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         George Walton
    //       DATE WRITTEN   December 1979
    //       MODIFIED       Jun 1990 (RDT for new CTF arrays)
    //                      Jul 2000 (RJL for Moisture algorithms)
    //                      Sep 2000 (RKS for new radiant exchange algorithm)
    //                      Dec 2000 (RKS for radiant system model addition)
    //                      Aug 2010 (BG added radiant heat flow rate reporting)
    //       RE-ENGINEERED  Mar 1998 (RKS)

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine performs a heat balance on the outside face of each
    // surface in the building.  NOTE that this also sets some coefficients
    // that are needed for radiant system modeling.  Thus, it is extremely
    // important that if someone makes changes to the heat balance equations
    // at a later date that they must also make changes to the coefficient
    // setting portion of this subroutine as well.

    // METHODOLOGY EMPLOYED:
    // Various boundary conditions are set and additional parameters are set-
    // up.  Then, the proper heat balance equation is selected based on the
    // presence of movable insulation, thermal mass of the surface construction,
    // and convection model being used.

    // REFERENCES:
    // (I)BLAST legacy routine HBOUT
    // 1989 ASHRAE Handbook of Fundamentals (Figure 1 on p. 22.4, convection correlations)

    // Determine whether or not movable insulation is present
    bool MovInsulPresent = (HMovInsul > 0.0); // .TRUE. if movable insulation is currently present for surface
    bool QuickConductionSurf;                 // .TRUE. if the cross CTF term is relatively large
    Real64 F1;                                // Intermediate calculation variable
    Real64 F2;                                // Intermediate calculation variable
    // Determine whether this surface is a "slow conductive" or "quick conductive"
    // surface.  Designates are inherited from BLAST.  Basically, a "quick" surface
    // requires the inside heat balance to be accounted for in the heat balance
    // while a "slow" surface can used the last time step's value for inside
    // surface temperature.
    auto &s_mat = state.dataMaterial;

    auto &surface = state.dataSurface->Surface(SurfNum);
    auto const &construct = state.dataConstruction->Construct(ConstrNum);
    if (construct.CTFCross[0] > 0.01) {
        QuickConductionSurf = true;
        F1 = construct.CTFCross[0] / (construct.CTFInside[0] + state.dataHeatBalSurf->SurfHConvInt(SurfNum));
    } else {
        QuickConductionSurf = false;
    }

    Real64 TSky = state.dataEnvrn->SkyTemp;
    Real64 TGround = state.dataEnvrn->OutDryBulbTemp;
    Real64 TSrdSurfs = 0.0;

    if (surface.SurfHasSurroundingSurfProperty) {
        int SrdSurfsNum = surface.SurfSurroundingSurfacesNum;
        if (state.dataSurface->SurroundingSurfsProperty(SrdSurfsNum).skyTempSched != nullptr) {
            TSky = state.dataSurface->SurroundingSurfsProperty(SrdSurfsNum).skyTempSched->getCurrentVal();
        }
        if (state.dataSurface->SurroundingSurfsProperty(SrdSurfsNum).groundTempSched != nullptr) {
            TGround = state.dataSurface->SurroundingSurfsProperty(SrdSurfsNum).groundTempSched->getCurrentVal();
        }
        TSrdSurfs = state.dataSurface->Surface(SurfNum).SrdSurfTemp;
    }
    if (surface.UseSurfPropertyGndSurfTemp) {
        TGround = state.dataSurface->GroundSurfsProperty(surface.SurfPropertyGndSurfIndex).SurfsTempAvg;
    }

    // Now, calculate the outside surface temperature using the proper heat balance equation.
    // Each case has been separated out into its own IF-THEN block for clarity.  Additional
    // cases can simply be added anywhere in the following section.  This is the last step
    // in the main loop.  Once the proper heat balance is done, the simulation goes on to
    // the next SurfNum.

    // Outside heat balance case: Tubular daylighting device
    Real64 &TH11(state.dataHeatBalSurf->SurfOutsideTempHist(1)(SurfNum));
    if (surface.Class == DataSurfaces::SurfaceClass::TDD_Dome) {

        // Lookup up the TDD:DIFFUSER object
        int PipeNum = state.dataSurface->SurfWinTDDPipeNum(SurfNum);
        int SurfNum2 = state.dataDaylightingDevicesData->TDDPipe(PipeNum).Diffuser;
        int spaceNum2 = state.dataSurface->Surface(SurfNum2).spaceNum;
        Real64 Ueff = 1.0 / state.dataDaylightingDevicesData->TDDPipe(PipeNum).Reff; // 1 / effective R value between TDD:DOME and TDD:DIFFUSER
        F1 = Ueff / (Ueff + state.dataHeatBalSurf->SurfHConvInt(SurfNum2));

        // Similar to opaque surface but inside conditions of TDD:DIFFUSER are used, and no embedded sources/sinks.
        // Absorbed shortwave radiation is treated similar to a regular window, but only 1 glass layer is allowed.
        //   SurfOpaqQRadSWOutAbs(SurfNum) does not apply for TDD:DOME, must use SurfWinQRadSWwinAbs(SurfNum,1)/2.0 instead.
        //+Construct(ConstrNum)%CTFSourceOut[0]     &   TDDs cannot be radiant systems
        // *SurfQsrcHist(1,SurfNum)                     &
        //+Construct(ConstrNum)%CTFSourceIn[0] &   TDDs cannot be radiant systems
        // *SurfQsrcHist(1,SurfNum)                &
        TH11 = (state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum, 1) / 2.0 +
                (state.dataHeatBalSurf->SurfHConvExt(SurfNum) + state.dataHeatBalSurf->SurfHAirExt(SurfNum)) * TempExt +
                state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum) * TSrdSurfs + state.dataHeatBalSurf->SurfQAdditionalHeatSourceOutside(SurfNum) +
                state.dataHeatBalSurf->SurfHSkyExt(SurfNum) * TSky + state.dataHeatBalSurf->SurfHGrdExt(SurfNum) * TGround +
                F1 * (state.dataHeatBal->SurfWinQRadSWwinAbs(SurfNum2, 1) / 2.0 + state.dataHeatBal->SurfQdotRadIntGainsInPerArea(SurfNum2) +
                      state.dataHeatBalSurf->SurfHConvInt(SurfNum2) * state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum2).MAT +
                      state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(SurfNum2))) /
               (Ueff + state.dataHeatBalSurf->SurfHConvExt(SurfNum) + state.dataHeatBalSurf->SurfHAirExt(SurfNum) +
                state.dataHeatBalSurf->SurfHSkyExt(SurfNum) + state.dataHeatBalSurf->SurfHGrdExt(SurfNum) +
                state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum) -
                F1 * Ueff); // Instead of SurfOpaqQRadSWOutAbs(SurfNum) | ODB used to approx ground surface temp | Use TDD:DIFFUSER surface | Use
                            // TDD:DIFFUSER surface | Use TDD:DIFFUSER surface and zone | Use TDD:DIFFUSER surface

        // Outside heat balance case: No movable insulation, slow conduction
    } else if ((!MovInsulPresent) && (!QuickConductionSurf)) {
        // Add LWR from surrounding surfaces
        if (surface.OSCMPtr == 0) {
            if (construct.SourceSinkPresent) {
                TH11 = (-state.dataHeatBalSurf->SurfCTFConstOutPart(SurfNum) + state.dataHeatBalSurf->SurfOpaqQRadSWOutAbs(SurfNum) +
                        state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum) * TSrdSurfs +
                        (state.dataHeatBalSurf->SurfHConvExt(SurfNum) + state.dataHeatBalSurf->SurfHAirExt(SurfNum)) * TempExt +
                        state.dataHeatBalSurf->SurfQAdditionalHeatSourceOutside(SurfNum) + state.dataHeatBalSurf->SurfHSkyExt(SurfNum) * TSky +
                        state.dataHeatBalSurf->SurfHGrdExt(SurfNum) * TGround + construct.CTFCross[0] * state.dataHeatBalSurf->SurfTempIn(SurfNum) +
                        construct.CTFSourceOut[0] * state.dataHeatBalSurf->SurfQsrcHist(SurfNum, 1)) /
                       (construct.CTFOutside[0] + state.dataHeatBalSurf->SurfHConvExt(SurfNum) + state.dataHeatBalSurf->SurfHAirExt(SurfNum) +
                        state.dataHeatBalSurf->SurfHSkyExt(SurfNum) + state.dataHeatBalSurf->SurfHGrdExt(SurfNum) +
                        state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum)); // ODB used to approx ground surface temp
            } else {
                TH11 = (-state.dataHeatBalSurf->SurfCTFConstOutPart(SurfNum) + state.dataHeatBalSurf->SurfOpaqQRadSWOutAbs(SurfNum) +
                        state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum) * TSrdSurfs +
                        (state.dataHeatBalSurf->SurfHConvExt(SurfNum) + state.dataHeatBalSurf->SurfHAirExt(SurfNum)) * TempExt +
                        state.dataHeatBalSurf->SurfQAdditionalHeatSourceOutside(SurfNum) + state.dataHeatBalSurf->SurfHSkyExt(SurfNum) * TSky +
                        state.dataHeatBalSurf->SurfHGrdExt(SurfNum) * TGround + construct.CTFCross[0] * state.dataHeatBalSurf->SurfTempIn(SurfNum)) /
                       (construct.CTFOutside[0] + state.dataHeatBalSurf->SurfHConvExt(SurfNum) + state.dataHeatBalSurf->SurfHAirExt(SurfNum) +
                        state.dataHeatBalSurf->SurfHSkyExt(SurfNum) + state.dataHeatBalSurf->SurfHGrdExt(SurfNum) +
                        state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum)); // ODB used to approx ground surface temp
            }
            // Outside Heat Balance case: Other Side Conditions Model
        } else { //( Surface(SurfNum)%OSCMPtr > 0 ) THEN
            // local copies of variables for clarity in radiation terms
            // TODO: - int OSCMPtr; // "Pointer" to OSCM data structure (other side conditions from a model)
            Real64 RadTemp =
                state.dataSurface->OSCM(surface.OSCMPtr).TRad; // local value for Effective radiation temperature for OtherSideConditions model
            Real64 HRad = state.dataSurface->OSCM(surface.OSCMPtr).HRad; // local value for effective (linearized) radiation coefficient

            // patterned after "No movable insulation, slow conduction," but with new radiation terms and no sun,
            if (construct.SourceSinkPresent) {
                TH11 = (-state.dataHeatBalSurf->SurfCTFConstOutPart(SurfNum) + state.dataHeatBalSurf->SurfHConvExt(SurfNum) * TempExt +
                        state.dataHeatBalSurf->SurfQAdditionalHeatSourceOutside(SurfNum) + HRad * RadTemp +
                        construct.CTFCross[0] * state.dataHeatBalSurf->SurfTempIn(SurfNum) +
                        construct.CTFSourceOut[0] * state.dataHeatBalSurf->SurfQsrcHist(SurfNum, 1)) /
                       (construct.CTFOutside[0] + state.dataHeatBalSurf->SurfHConvExt(SurfNum) + HRad);
            } else {
                TH11 = (-state.dataHeatBalSurf->SurfCTFConstOutPart(SurfNum) + state.dataHeatBalSurf->SurfHConvExt(SurfNum) * TempExt +
                        state.dataHeatBalSurf->SurfQAdditionalHeatSourceOutside(SurfNum) + HRad * RadTemp +
                        construct.CTFCross[0] * state.dataHeatBalSurf->SurfTempIn(SurfNum)) /
                       (construct.CTFOutside[0] + state.dataHeatBalSurf->SurfHConvExt(SurfNum) + HRad);
            }
        }
        // Outside heat balance case: No movable insulation, quick conduction
    } else if ((!MovInsulPresent) && (QuickConductionSurf)) {
        if (surface.OSCMPtr == 0) {
            if (construct.SourceSinkPresent) {
                TH11 = (-state.dataHeatBalSurf->SurfCTFConstOutPart(SurfNum) + state.dataHeatBalSurf->SurfOpaqQRadSWOutAbs(SurfNum) +
                        state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum) * TSrdSurfs +
                        (state.dataHeatBalSurf->SurfHConvExt(SurfNum) + state.dataHeatBalSurf->SurfHAirExt(SurfNum)) * TempExt +
                        state.dataHeatBalSurf->SurfQAdditionalHeatSourceOutside(SurfNum) + state.dataHeatBalSurf->SurfHSkyExt(SurfNum) * TSky +
                        state.dataHeatBalSurf->SurfHGrdExt(SurfNum) * TGround +
                        construct.CTFSourceOut[0] * state.dataHeatBalSurf->SurfQsrcHist(SurfNum, 1) +
                        F1 * (state.dataHeatBalSurf->SurfCTFConstInPart(SurfNum) + state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(SurfNum) +
                              state.dataHeatBal->SurfQdotRadIntGainsInPerArea(SurfNum) +
                              state.dataHeatBalSurf->SurfHConvInt(SurfNum) * state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum).MAT +
                              state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(SurfNum))) /
                       (construct.CTFOutside[0] + state.dataHeatBalSurf->SurfHConvExt(SurfNum) + state.dataHeatBalSurf->SurfHAirExt(SurfNum) +
                        state.dataHeatBalSurf->SurfHSkyExt(SurfNum) + state.dataHeatBalSurf->SurfHGrdExt(SurfNum) +
                        state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum) -
                        F1 * construct.CTFCross[0]); // ODB used to approx ground surface temp | MAT use here is problem for room air models
            } else {
                TH11 = (-state.dataHeatBalSurf->SurfCTFConstOutPart(SurfNum) + state.dataHeatBalSurf->SurfOpaqQRadSWOutAbs(SurfNum) +
                        state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum) * TSrdSurfs +
                        (state.dataHeatBalSurf->SurfHConvExt(SurfNum) + state.dataHeatBalSurf->SurfHAirExt(SurfNum)) * TempExt +
                        state.dataHeatBalSurf->SurfQAdditionalHeatSourceOutside(SurfNum) + state.dataHeatBalSurf->SurfHSkyExt(SurfNum) * TSky +
                        state.dataHeatBalSurf->SurfHGrdExt(SurfNum) * TGround +
                        F1 * (state.dataHeatBalSurf->SurfCTFConstInPart(SurfNum) + state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(SurfNum) +
                              state.dataHeatBal->SurfQdotRadIntGainsInPerArea(SurfNum) +
                              state.dataHeatBalSurf->SurfHConvInt(SurfNum) * state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum).MAT +
                              state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(SurfNum))) /
                       (construct.CTFOutside[0] + state.dataHeatBalSurf->SurfHConvExt(SurfNum) + state.dataHeatBalSurf->SurfHAirExt(SurfNum) +
                        state.dataHeatBalSurf->SurfHSkyExt(SurfNum) + state.dataHeatBalSurf->SurfHGrdExt(SurfNum) +
                        state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum) -
                        F1 * construct.CTFCross[0]); // ODB used to approx ground surface temp | MAT use here is problem for room air models
            }
            // Outside Heat Balance case: Other Side Conditions Model
        } else { //( Surface(SurfNum)%OSCMPtr > 0 ) THEN
            // local copies of variables for clarity in radiation terms
            Real64 RadTemp = state.dataSurface->OSCM(surface.OSCMPtr).TRad;
            Real64 HRad = state.dataSurface->OSCM(surface.OSCMPtr).HRad;
            // patterned after "No movable insulation, quick conduction," but with new radiation terms and no sun,
            if (construct.SourceSinkPresent) {
                TH11 = (-state.dataHeatBalSurf->SurfCTFConstOutPart(SurfNum) + state.dataHeatBalSurf->SurfHConvExt(SurfNum) * TempExt +
                        state.dataHeatBalSurf->SurfQAdditionalHeatSourceOutside(SurfNum) + HRad * RadTemp +
                        construct.CTFSourceOut[0] * state.dataHeatBalSurf->SurfQsrcHist(SurfNum, 1) +
                        F1 * (state.dataHeatBalSurf->SurfCTFConstInPart(SurfNum) + state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(SurfNum) +
                              state.dataHeatBal->SurfQdotRadIntGainsInPerArea(SurfNum) +
                              construct.CTFSourceIn[0] * state.dataHeatBalSurf->SurfQsrcHist(SurfNum, 1) +
                              state.dataHeatBalSurf->SurfHConvInt(SurfNum) * state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum).MAT +
                              state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(SurfNum))) /
                       (construct.CTFOutside[0] + state.dataHeatBalSurf->SurfHConvExt(SurfNum) + HRad -
                        F1 * construct.CTFCross[0]); // MAT use here is problem for room air models
            } else {
                TH11 = (-state.dataHeatBalSurf->SurfCTFConstOutPart(SurfNum) + state.dataHeatBalSurf->SurfHConvExt(SurfNum) * TempExt +
                        state.dataHeatBalSurf->SurfQAdditionalHeatSourceOutside(SurfNum) + HRad * RadTemp +
                        F1 * (state.dataHeatBalSurf->SurfCTFConstInPart(SurfNum) + state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(SurfNum) +
                              state.dataHeatBal->SurfQdotRadIntGainsInPerArea(SurfNum) +
                              state.dataHeatBalSurf->SurfHConvInt(SurfNum) * state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum).MAT +
                              state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(SurfNum))) /
                       (construct.CTFOutside[0] + state.dataHeatBalSurf->SurfHConvExt(SurfNum) + HRad -
                        F1 * construct.CTFCross[0]); // MAT use here is problem for room air models
            }
        }
        // Outside heat balance case: Movable insulation, slow conduction
    } else if ((MovInsulPresent) && (!QuickConductionSurf)) {

        F2 = HMovInsul / (HMovInsul + state.dataHeatBalSurf->SurfHConvExt(SurfNum) + state.dataHeatBalSurf->SurfHAirExt(SurfNum) +
                          state.dataHeatBalSurf->SurfHSkyExt(SurfNum) + state.dataHeatBalSurf->SurfHGrdExt(SurfNum) +
                          state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum));

        TH11 = (-state.dataHeatBalSurf->SurfCTFConstOutPart(SurfNum) + state.dataHeatBalSurf->SurfOpaqQRadSWOutAbs(SurfNum) +
                construct.CTFCross[0] * state.dataHeatBalSurf->SurfTempIn(SurfNum) +
                F2 * (state.dataHeatBalSurf->SurfQRadSWOutMvIns(SurfNum) +
                      (state.dataHeatBalSurf->SurfHConvExt(SurfNum) + state.dataHeatBalSurf->SurfHAirExt(SurfNum)) * TempExt +
                      state.dataHeatBalSurf->SurfQAdditionalHeatSourceOutside(SurfNum) + state.dataHeatBalSurf->SurfHSkyExt(SurfNum) * TSky +
                      state.dataHeatBalSurf->SurfHGrdExt(SurfNum) * TGround + state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum) * TSrdSurfs)) /
               (construct.CTFOutside[0] + HMovInsul - F2 * HMovInsul); // ODB used to approx ground surface temp

        // Outside heat balance case: Movable insulation, quick conduction
    } else if ((MovInsulPresent) && (QuickConductionSurf)) {

        F2 = HMovInsul / (HMovInsul + state.dataHeatBalSurf->SurfHConvExt(SurfNum) + state.dataHeatBalSurf->SurfHAirExt(SurfNum) +
                          state.dataHeatBalSurf->SurfHSkyExt(SurfNum) + state.dataHeatBalSurf->SurfHGrdExt(SurfNum) +
                          state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum));

        TH11 = (-state.dataHeatBalSurf->SurfCTFConstOutPart(SurfNum) + state.dataHeatBalSurf->SurfOpaqQRadSWOutAbs(SurfNum) +
                F1 * (state.dataHeatBalSurf->SurfCTFConstInPart(SurfNum) + state.dataHeatBalSurf->SurfOpaqQRadSWInAbs(SurfNum) +
                      state.dataHeatBal->SurfQdotRadIntGainsInPerArea(SurfNum) +
                      state.dataHeatBalSurf->SurfHConvInt(SurfNum) * state.dataZoneTempPredictorCorrector->spaceHeatBalance(spaceNum).MAT +
                      state.dataHeatBalSurf->SurfQdotRadNetLWInPerArea(SurfNum)) +
                F2 * (state.dataHeatBalSurf->SurfQRadSWOutMvIns(SurfNum) +
                      (state.dataHeatBalSurf->SurfHConvExt(SurfNum) + state.dataHeatBalSurf->SurfHAirExt(SurfNum)) * TempExt +
                      state.dataHeatBalSurf->SurfQAdditionalHeatSourceOutside(SurfNum) + state.dataHeatBalSurf->SurfHSkyExt(SurfNum) * TSky +
                      state.dataHeatBalSurf->SurfHGrdExt(SurfNum) * TGround + state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum) * TSrdSurfs)) /
               (construct.CTFOutside[0] + HMovInsul - F2 * HMovInsul - F1 * construct.CTFCross[0]); // ODB used to approx ground surface temp

    } // ...end of outside heat balance cases IF-THEN block

    // multiply out linearized radiation coeffs for reporting
    Real64 const HExtSurf_fac(
        -(state.dataHeatBalSurf->SurfHSkyExt(SurfNum) * (TH11 - TSky) + state.dataHeatBalSurf->SurfHAirExt(SurfNum) * (TH11 - TempExt) +
          state.dataHeatBalSurf->SurfHGrdExt(SurfNum) * (TH11 - TGround) + state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum) * (TH11 - TSrdSurfs)));
    state.dataHeatBalSurf->SurfQdotRadOutRepPerArea(SurfNum) = HExtSurf_fac;

    // Set the radiant system heat balance coefficients if this surface is also a radiant system
    if (construct.SourceSinkPresent) {

        if (MovInsulPresent) {
            // Note: if movable insulation is ever added back in correctly, the heat balance equations above must be fixed
            ShowSevereError(state, "Exterior movable insulation is not valid with embedded sources/sinks");
            ShowContinueError(state, format("Construction {} contains an internal source or sink but also uses", construct.Name));
            ShowContinueError(state,
                              format("exterior movable insulation {} for a surface with that construction.",
                                     s_mat->materials(state.dataSurface->extMovInsuls(SurfNum).matNum)->Name));
            ShowContinueError(state,
                              "This is not currently allowed because the heat balance equations do not currently accommodate this combination.");
            ErrorFlag = true;
            return;

        } else {
            Real64 const RadSysDiv(1.0 / (construct.CTFOutside[0] + state.dataHeatBalSurf->SurfHConvExt(SurfNum) +
                                          state.dataHeatBalSurf->SurfHAirExt(SurfNum) + state.dataHeatBalSurf->SurfHSkyExt(SurfNum) +
                                          state.dataHeatBalSurf->SurfHGrdExt(SurfNum) + state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum) +
                                          state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum)));

            state.dataHeatBalFanSys->RadSysToHBConstCoef(SurfNum) =
                (-state.dataHeatBalSurf->SurfCTFConstOutPart(SurfNum) + state.dataHeatBalSurf->SurfOpaqQRadSWOutAbs(SurfNum) +
                 state.dataHeatBalSurf->SurfHSrdSurfExt(SurfNum) * TSrdSurfs +
                 (state.dataHeatBalSurf->SurfHConvExt(SurfNum) + state.dataHeatBalSurf->SurfHAirExt(SurfNum)) * TempExt +
                 state.dataHeatBalSurf->SurfHSkyExt(SurfNum) * TSky + state.dataHeatBalSurf->SurfHGrdExt(SurfNum) * TGround) *
                RadSysDiv; // ODB used to approx ground surface temp

            state.dataHeatBalFanSys->RadSysToHBTinCoef(SurfNum) = construct.CTFCross[0] * RadSysDiv;

            state.dataHeatBalFanSys->RadSysToHBQsrcCoef(SurfNum) = construct.CTFSourceOut[0] * RadSysDiv;
        }
    }
}

void CalcExteriorVentedCavity(EnergyPlusData &state, int const SurfNum) // index of surface
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         B Griffith
    //       DATE WRITTEN   January 2005

    // PURPOSE OF THIS SUBROUTINE:
    // manages calculating the temperatures of baffle and air cavity for
    // multi-skin configuration.

    // METHODOLOGY EMPLOYED:
    // derived from CalcPassiveTranspiredCollector

    // local working variables
    Real64 HrPlen;
    Real64 HcPlen;
    Real64 Isc;
    Real64 MdotVent;
    Real64 VdotWind;
    Real64 VdotThermal;

    int CavNum = state.dataSurface->SurfExtCavNum(SurfNum);
    Real64 TempExt = state.dataSurface->SurfOutDryBulbTemp(SurfNum);
    Real64 OutHumRatExt = Psychrometrics::PsyWFnTdbTwbPb(
        state, state.dataSurface->SurfOutDryBulbTemp(SurfNum), state.dataSurface->SurfOutWetBulbTemp(SurfNum), state.dataEnvrn->OutBaroPress);
    Real64 RhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, TempExt, OutHumRatExt);
    Real64 holeArea = state.dataHeatBal->ExtVentedCavity(CavNum).ActualArea * state.dataHeatBal->ExtVentedCavity(CavNum).Porosity;
    // Aspect Ratio of gap
    Real64 AspRat = state.dataHeatBal->ExtVentedCavity(CavNum).HdeltaNPL * 2.0 / state.dataHeatBal->ExtVentedCavity(CavNum).PlenGapThick;
    Real64 TmpTscoll = state.dataHeatBal->ExtVentedCavity(CavNum).TbaffleLast;
    Real64 TmpTaPlen = state.dataHeatBal->ExtVentedCavity(CavNum).TairLast;

    // all the work is done in this routine located in GeneralRoutines.cc

    for (int iter = 1; iter <= 3; ++iter) { // this is a sequential solution approach.

        TranspiredCollector::CalcPassiveExteriorBaffleGap(state,
                                                          state.dataHeatBal->ExtVentedCavity(CavNum).SurfPtrs,
                                                          holeArea,
                                                          state.dataHeatBal->ExtVentedCavity(CavNum).Cv,
                                                          state.dataHeatBal->ExtVentedCavity(CavNum).Cd,
                                                          state.dataHeatBal->ExtVentedCavity(CavNum).HdeltaNPL,
                                                          state.dataHeatBal->ExtVentedCavity(CavNum).SolAbsorp,
                                                          state.dataHeatBal->ExtVentedCavity(CavNum).LWEmitt,
                                                          state.dataHeatBal->ExtVentedCavity(CavNum).Tilt,
                                                          AspRat,
                                                          state.dataHeatBal->ExtVentedCavity(CavNum).PlenGapThick,
                                                          state.dataHeatBal->ExtVentedCavity(CavNum).BaffleRoughness,
                                                          state.dataHeatBal->ExtVentedCavity(CavNum).QdotSource,
                                                          TmpTscoll,
                                                          TmpTaPlen,
                                                          HcPlen,
                                                          HrPlen,
                                                          Isc,
                                                          MdotVent,
                                                          VdotWind,
                                                          VdotThermal);

    } // sequential solution
    // now fill results into derived types
    state.dataHeatBal->ExtVentedCavity(CavNum).Isc = Isc;
    state.dataHeatBal->ExtVentedCavity(CavNum).TAirCav = TmpTaPlen;
    state.dataHeatBal->ExtVentedCavity(CavNum).Tbaffle = TmpTscoll;
    state.dataHeatBal->ExtVentedCavity(CavNum).HrPlen = HrPlen;
    state.dataHeatBal->ExtVentedCavity(CavNum).HcPlen = HcPlen;
    state.dataHeatBal->ExtVentedCavity(CavNum).PassiveACH =
        (MdotVent / RhoAir) *
        (1.0 / (state.dataHeatBal->ExtVentedCavity(CavNum).ProjArea * state.dataHeatBal->ExtVentedCavity(CavNum).PlenGapThick)) *
        Constant::rSecsInHour;
    state.dataHeatBal->ExtVentedCavity(CavNum).PassiveMdotVent = MdotVent;
    state.dataHeatBal->ExtVentedCavity(CavNum).PassiveMdotWind = VdotWind * RhoAir;
    state.dataHeatBal->ExtVentedCavity(CavNum).PassiveMdotTherm = VdotThermal * RhoAir;

    // now do some updates
    state.dataHeatBal->ExtVentedCavity(CavNum).TairLast = state.dataHeatBal->ExtVentedCavity(CavNum).TAirCav;
    state.dataHeatBal->ExtVentedCavity(CavNum).TbaffleLast = state.dataHeatBal->ExtVentedCavity(CavNum).Tbaffle;

    // update the OtherSideConditionsModel coefficients.
    int thisOSCM = state.dataHeatBal->ExtVentedCavity(CavNum).OSCMPtr;

    state.dataSurface->OSCM(thisOSCM).TConv = state.dataHeatBal->ExtVentedCavity(CavNum).TAirCav;
    state.dataSurface->OSCM(thisOSCM).HConv = state.dataHeatBal->ExtVentedCavity(CavNum).HcPlen;
    state.dataSurface->OSCM(thisOSCM).TRad = state.dataHeatBal->ExtVentedCavity(CavNum).Tbaffle;
    state.dataSurface->OSCM(thisOSCM).HRad = state.dataHeatBal->ExtVentedCavity(CavNum).HrPlen;
}

void GatherComponentLoadsSurfAbsFact(EnergyPlusData &state)
{
    // SUBROUTINE INFORMATION:
    //       AUTHOR         Jason Glazer
    //       DATE WRITTEN   September 2012

    // PURPOSE OF THIS SUBROUTINE:
    //   Gather values during sizing used for surface absorption factors

    // METHODOLOGY EMPLOYED:
    //   Save sequence of values for report during sizing.

    // This is by surface, so it works for both space and zone component loads
    if (state.dataGlobal->CompLoadReportIsReq && !state.dataGlobal->isPulseZoneSizing) {
        int TimeStepInDay = (state.dataGlobal->HourOfDay - 1) * state.dataGlobal->TimeStepsInHour + state.dataGlobal->TimeStep;
        auto &surfCLDayTS = state.dataOutRptTab->surfCompLoads[state.dataSize->CurOverallSimDay - 1].ts[TimeStepInDay - 1];
        for (int jSurf = 1; jSurf <= state.dataSurface->TotSurfaces; ++jSurf) {
            auto const &surface = state.dataSurface->Surface(jSurf);
            if (!surface.HeatTransSurf || surface.Zone == 0) {
                continue; // Skip non-heat transfer surfaces
            }
            if (surface.Class == DataSurfaces::SurfaceClass::TDD_Dome) {
                continue; // Skip tubular daylighting device domes
            }
            surfCLDayTS.surf[jSurf - 1].ITABSFseq = state.dataHeatBalSurf->SurfAbsThermalInt(jSurf);
            surfCLDayTS.surf[jSurf - 1].TMULTseq = state.dataViewFactor->EnclRadInfo(surface.RadEnclIndex).radThermAbsMult;
        }
    }
}

Real64 GetSurfIncidentSolarMultiplier(EnergyPlusData &state, int SurfNum)
{
    if (!state.dataSurface->Surface(SurfNum).hasIncSolMultiplier) {
        return 1.0;
    } else if (state.dataSurface->SurfIncSolMultiplier(SurfNum).sched != nullptr) {
        return state.dataSurface->SurfIncSolMultiplier(SurfNum).sched->getCurrentVal() * state.dataSurface->SurfIncSolMultiplier(SurfNum).Scaler;
    } else {
        return state.dataSurface->SurfIncSolMultiplier(SurfNum).Scaler;
    }
}

void InitSurfacePropertyViewFactors(EnergyPlusData &state)
{

    // purpose:
    //   Initializes sky and ground surfaces view factors of exterior surfaces
    //   used by SurfaceProperty:LocalEnvironment
    //   view factors are constant hence should be set only once

    if (!state.dataGlobal->AnyLocalEnvironmentsInModel) {
        return;
    }
    if (!state.dataHeatBalSurfMgr->InitSurfaceHeatBalancefirstTime) {
        return;
    }

    for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
        auto &Surface = state.dataSurface->Surface(SurfNum);
        if (Surface.SurfHasSurroundingSurfProperty || Surface.IsSurfPropertyGndSurfacesDefined) {

            int GndSurfsNum = 0;
            int SrdSurfsNum = 0;
            Real64 SrdSurfsViewFactor = 0.0;
            Real64 SurfsSkyViewFactor = 0.0;
            Real64 GroundSurfsViewFactor = 0.0;
            bool IsSkyViewFactorSet = false;
            bool IsGroundViewFactorSet = false;
            bool SetGroundViewFactorObject = false;
            if (Surface.SurfHasSurroundingSurfProperty) {
                SrdSurfsNum = Surface.SurfSurroundingSurfacesNum;
                auto &SrdSurfsProperty = state.dataSurface->SurroundingSurfsProperty(SrdSurfsNum);
                SurfsSkyViewFactor = SrdSurfsProperty.SkyViewFactor;
                IsSkyViewFactorSet = SrdSurfsProperty.IsSkyViewFactorSet;
                if (SurfsSkyViewFactor > 0.0) {
                    SrdSurfsViewFactor += SurfsSkyViewFactor;
                }
                if (!Surface.IsSurfPropertyGndSurfacesDefined) {
                    SrdSurfsViewFactor += SrdSurfsProperty.GroundViewFactor;
                    IsGroundViewFactorSet = SrdSurfsProperty.IsGroundViewFactorSet;
                    GroundSurfsViewFactor = SrdSurfsProperty.GroundViewFactor;
                }
                for (int SrdSurfNum = 1; SrdSurfNum <= SrdSurfsProperty.TotSurroundingSurface; SrdSurfNum++) {
                    SrdSurfsViewFactor += SrdSurfsProperty.SurroundingSurfs(SrdSurfNum).ViewFactor;
                }
            }
            if (Surface.IsSurfPropertyGndSurfacesDefined) {
                GndSurfsNum = Surface.SurfPropertyGndSurfIndex;
                IsGroundViewFactorSet = state.dataSurface->GroundSurfsProperty(GndSurfsNum).IsGroundViewFactorSet;
                GroundSurfsViewFactor = state.dataSurface->GroundSurfsProperty(GndSurfsNum).SurfsViewFactorSum;
                SrdSurfsViewFactor += GroundSurfsViewFactor;
            }

            // Check if the sum of all defined view factors > 1.0
            if (SrdSurfsViewFactor > 1.0) {
                ShowSevereError(state, format("Illegal surrounding surfaces view factors for {}.", Surface.Name));
                ShowContinueError(state, " The sum of sky, ground, and all surrounding surfaces view factors should be less than or equal to 1.0.");
            }
            if (IsSkyViewFactorSet && IsGroundViewFactorSet) {
                // If both surface sky and ground view factor defined, overwrite with the defined value
                Surface.ViewFactorSkyIR = SurfsSkyViewFactor;
                Surface.ViewFactorGroundIR = GroundSurfsViewFactor;
            } else if (IsSkyViewFactorSet && !IsGroundViewFactorSet) {
                // If only sky view factor defined, ground view factor = 1 - all other defined view factors.
                Surface.ViewFactorSkyIR = SurfsSkyViewFactor;
                Surface.ViewFactorGroundIR = 1 - SrdSurfsViewFactor;
                if (GndSurfsNum > 0) {
                    SetGroundViewFactorObject = true;
                    state.dataSurface->GroundSurfsProperty(GndSurfsNum).IsGroundViewFactorSet = true;
                } else {
                    state.dataSurface->SurroundingSurfsProperty(SrdSurfsNum).GroundViewFactor = Surface.ViewFactorGroundIR;
                }
            } else if (!IsSkyViewFactorSet && IsGroundViewFactorSet) {
                // If only ground view factor defined, sky view factor = 1 - all other defined view factors.
                Surface.ViewFactorGroundIR = GroundSurfsViewFactor;
                Surface.ViewFactorSkyIR = 1 - SrdSurfsViewFactor;
                if (SrdSurfsNum > 0) {
                    state.dataSurface->SurroundingSurfsProperty(SrdSurfsNum).IsSkyViewFactorSet = true;
                    state.dataSurface->SurroundingSurfsProperty(SrdSurfsNum).SkyViewFactor = Surface.ViewFactorSkyIR;
                }
            } else {
                // If neither ground nor sky view factor specified, continue to use the original proportion.
                Surface.ViewFactorSkyIR *= 1 - SrdSurfsViewFactor;
                Surface.ViewFactorGroundIR *= 1 - SrdSurfsViewFactor;
                if (SrdSurfsNum > 0) {
                    state.dataSurface->SurroundingSurfsProperty(SrdSurfsNum).IsSkyViewFactorSet = true;
                    state.dataSurface->SurroundingSurfsProperty(SrdSurfsNum).SkyViewFactor = Surface.ViewFactorSkyIR;
                    if (GndSurfsNum == 0) {
                        state.dataSurface->SurroundingSurfsProperty(SrdSurfsNum).GroundViewFactor = Surface.ViewFactorGroundIR;
                        state.dataSurface->SurroundingSurfsProperty(SrdSurfsNum).IsGroundViewFactorSet = true;
                    }
                }
                if (GndSurfsNum > 0) {
                    SetGroundViewFactorObject = true;
                    state.dataSurface->GroundSurfsProperty(GndSurfsNum).IsGroundViewFactorSet = true;
                }
            }
            if (SetGroundViewFactorObject) {
                ReSetGroundSurfacesViewFactor(state, SurfNum);
            }
        }
    }
}

void GetGroundSurfacesTemperatureAverage(EnergyPlusData &state)
{
    //  returns ground surfaces average temperature (deg C)
    //  ground surfaces viewed by a building exterior surface
    //  ground surfaces temperature weighed using view factors

    if (!state.dataGlobal->AnyLocalEnvironmentsInModel) {
        return;
    }

    for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {
        if (!state.dataSurface->Surface(SurfNum).IsSurfPropertyGndSurfacesDefined) {
            continue;
        }
        auto &GndSurfsProperty = state.dataSurface->GroundSurfsProperty(state.dataSurface->Surface(SurfNum).SurfPropertyGndSurfIndex);
        if (GndSurfsProperty.SurfsViewFactorSum == 0.0) {
            state.dataSurface->Surface(SurfNum).UseSurfPropertyGndSurfTemp = false;
            continue;
        }
        Real64 GndSurfaceTemp = 0.0;
        Real64 GndSurfViewFactor = 0.0;
        Real64 GndSurfaceTempSum = 0.0;
        for (int gSurfNum = 1; gSurfNum <= GndSurfsProperty.NumGndSurfs; gSurfNum++) {
            GndSurfViewFactor = GndSurfsProperty.GndSurfs(gSurfNum).ViewFactor;
            if (GndSurfViewFactor == 0.0) {
                continue;
            }
            if (GndSurfsProperty.GndSurfs(gSurfNum).tempSched == nullptr) {
                continue;
            }
            GndSurfaceTemp = GndSurfsProperty.GndSurfs(gSurfNum).tempSched->getCurrentVal();
            GndSurfaceTempSum += GndSurfViewFactor * pow_4(GndSurfaceTemp + Constant::Kelvin);
        }
        if (GndSurfaceTempSum == 0.0) {
            GndSurfsProperty.SurfsTempAvg = 0.0;
            state.dataSurface->Surface(SurfNum).UseSurfPropertyGndSurfTemp = false;
            continue;
        }
        GndSurfsProperty.SurfsTempAvg = root_4(GndSurfaceTempSum / GndSurfsProperty.SurfsViewFactorSum) - Constant::Kelvin;
    }
}

void GetGroundSurfacesReflectanceAverage(EnergyPlusData &state)
{
    //  returns ground surfaces average reflectance (dimensionless)
    //  ground reflectance viewed by a building exterior surface
    //  ground surfaces reflectance weighed using view factors

    if (!state.dataGlobal->AnyLocalEnvironmentsInModel) {
        return;
    }
    for (int SurfNum = 1; SurfNum <= state.dataSurface->TotSurfaces; ++SurfNum) {

        if (!state.dataSurface->Surface(SurfNum).IsSurfPropertyGndSurfacesDefined) {
            continue;
        }
        auto &GndSurfsProperty = state.dataSurface->GroundSurfsProperty(state.dataSurface->Surface(SurfNum).SurfPropertyGndSurfIndex);
        if (GndSurfsProperty.SurfsViewFactorSum == 0.0) {
            state.dataSurface->Surface(SurfNum).UseSurfPropertyGndSurfRefl = false;
            continue;
        }
        Real64 GndSurfRefl = 0.0;
        Real64 GndSurfsReflSum = 0.0;
        for (int gSurfNum = 1; gSurfNum <= GndSurfsProperty.NumGndSurfs; gSurfNum++) {
            if (GndSurfsProperty.GndSurfs(gSurfNum).reflSched == nullptr) {
                continue;
            }
            GndSurfRefl = GndSurfsProperty.GndSurfs(gSurfNum).reflSched->getCurrentVal();
            GndSurfsReflSum += GndSurfsProperty.GndSurfs(gSurfNum).ViewFactor * GndSurfRefl;
        }
        if (GndSurfsReflSum == 0.0) {
            GndSurfsProperty.SurfsReflAvg = 0.0;
            state.dataSurface->Surface(SurfNum).UseSurfPropertyGndSurfRefl = false;
            continue;
        }
        GndSurfsProperty.SurfsReflAvg = GndSurfsReflSum / GndSurfsProperty.SurfsViewFactorSum;
    }
}

void ReSetGroundSurfacesViewFactor(EnergyPlusData &state, int const SurfNum)
{
    //  resets ground view factors based on view factors identity
    //  the ground view factor value is set to the first element
    //  when the ground view factor input field is blank

    if (!state.dataSurface->Surface(SurfNum).IsSurfPropertyGndSurfacesDefined) {
        return;
    }
    auto &GndSurfsProperty = state.dataSurface->GroundSurfsProperty(state.dataSurface->Surface(SurfNum).SurfPropertyGndSurfIndex);
    GndSurfsProperty.SurfsViewFactorSum = state.dataSurface->Surface(SurfNum).ViewFactorGroundIR;

    if (GndSurfsProperty.SurfsViewFactorSum == 0.0) {
        state.dataSurface->Surface(SurfNum).UseSurfPropertyGndSurfRefl = false;
        state.dataSurface->Surface(SurfNum).UseSurfPropertyGndSurfTemp = false;
        return;
    }
    GndSurfsProperty.GndSurfs(1).ViewFactor = GndSurfsProperty.SurfsViewFactorSum;
}

void GetSurroundingSurfacesTemperatureAverage(EnergyPlusData &state)
{
    //  returns surrounding surfaces average temperature (deg C)
    //  surrounding surfaces viewed by an exterior surface
    //  surrounding surfaces temperature weighed using view factors

    if (!state.dataGlobal->AnyLocalEnvironmentsInModel) {
        return;
    }

    for (auto &surface : state.dataSurface->Surface) {
        if (!surface.SurfHasSurroundingSurfProperty) {
            continue;
        }
        // local vars
        Real64 SrdSurfaceTemp = 0.0;
        Real64 SrdSurfaceTempSum = 0.0;
        auto &SrdSurfsProperty = state.dataSurface->SurroundingSurfsProperty(surface.SurfSurroundingSurfacesNum);
        for (int SrdSurfNum = 1; SrdSurfNum <= SrdSurfsProperty.TotSurroundingSurface; SrdSurfNum++) {
            SrdSurfaceTemp = SrdSurfsProperty.SurroundingSurfs(SrdSurfNum).tempSched->getCurrentVal() + Constant::Kelvin;
            SrdSurfaceTempSum += SrdSurfsProperty.SurroundingSurfs(SrdSurfNum).ViewFactor * pow_4(SrdSurfaceTemp);
        }
        surface.SrdSurfTemp = root_4(SrdSurfaceTempSum / surface.ViewFactorSrdSurfs) - Constant::Kelvin;
    }
}
} // namespace EnergyPlus::HeatBalanceSurfaceManager

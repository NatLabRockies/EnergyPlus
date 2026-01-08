// EnergyPlus, Copyright (c) 1996-2023, The Board of Trustees of the University of Illinois,
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

// EnergyPlus Headers
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchInputManager.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Coils/CoilCoolingDX.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneControls.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/Furnaces.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HVACControllers.hh>
#include <EnergyPlus/HVACHXAssistedCoolingCoil.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/IntegratedHeatPump.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SteamCoils.hh>
#include <EnergyPlus/WaterCoils.hh>
#include <EnergyPlus/WaterToAirHeatPump.hh>
#include <EnergyPlus/WaterToAirHeatPumpSimple.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace Furnaces {
    // Module containing the Furnace and Unitary System simulation routines

    // MODULE INFORMATION:
    //       AUTHOR         Dan Fisher
    //       DATE WRITTEN   Jan 2001
    //       MODIFIED       Richard Liesen, Feb 2001; Don Shirey, Mar/Oct 2001, Oct 2003
    //                      Richard Raustad, Nov 2006 - allow draw through fan and alternate air flow in cooling,
    //                      heating, and when no cooling or heating is required.
    //                      Bereket Nigusse, FSEC, June 2010 - deprecated supply air flow fraction through controlled
    //                      zone from the furnace object input field. Now, the flow fraction is calculated internally
    //                      B. Nigusse, FSEC, Jan 2012 - added steam and hot water heating coils as an option
    //                      Bo Shen, ORNL, March 2012 - added variable-speed water source heat pump cooling and heating coils, using curve-fits
    //                      Bo Shen, ORNL, July 2012 - added variable-speed air source heat pump cooling and heating coils, using curve-fits
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and algorithms required to
    // manage the Furnace/Unitary System Compound Component

    // METHODOLOGY EMPLOYED:
    // Calculates the part-load ratio of the HVAC system to meet the zone sensible load. For non-heat pump HVAC systems,
    // if humidity control is specified and the latent capacity at the sensible PLR is insufficient to meet the latent load,
    // calculate a latent part-load ratio to meet the zone sensible load (MultiMode dehumidificaiton control) or the zone
    // latent load (CoolReheat dehumidification control). Use the greater of the sensible PLR and latent PLR to control
    // the HVAC system.
    // Subroutines:
    // SimFurnace - Top level simulate routine CALLed by other modules. Each child object is simulated a final time after
    //              the part-load ratio for child components has been determined.
    //  Note: A supplemental heater augments the heating capacity for both air-to-air and water-to-air heat pump systems.
    //        A reheat coil is used for the HeatCool furnace/unitarysystem to offset the sensible cooling when the
    //        dehumidification control type is COOLREHEAT. Both the supplemental and reheat heating coil load is calculated
    //        in the Calc routines. The actual simulation of these coils is performed in the SimFurnace routine (i.e. the
    //        supplemental and reheat coil loads are passed as 0 to CalcFurnaceOutput).
    // CalcNewZoneHeatOnlyFlowRates - HeatOnly furnace/unitarysystem routine.
    //                                Calculates a part-load ratio to meet the sensible load.
    // CalcNewZoneHeatCoolFlowRates - HeatCool furnace/unitarysystem and air-to-air HeatPump routine.
    //                                Calculates a part-load ratio for the system (sensible and/or latent).
    //                                For dehumidification control type COOLREHEAT, both a sensible and latent PLR
    //                                may exist for a single time step (heating and dehumidificaiton can occur). For all
    //                                other system types, only a single PLR is allowed for any given time step.
    //                                Order of simulation depends on dehumidification control option as described below.
    // Dehumidificaiton control options (non-heat pump versions):
    // Dehumidification Control NONE:   Cooling performance is simulated first and then heating performance. If a HX
    //                                  assisted cooling coil is selected, the HX is always active (cooling).
    // Dehumidification Control COOLREHEAT: For cooling operation, the sensible capacity is calculated to
    //                                      meet the thermostat setpoint. If a HX assisted cooling coil is selected,
    //                                      the HX is always active. If the latent load is not met by operating the
    //                                      system at the sensible PLR, a new PLR is calculated to meet the humidistat
    //                                      setpoint. The reheat coil load is then calculated to meet the HEATING
    //                                      setpoint temperature.
    // Dehumidification Control MULTIMODE: For cooling operation, the sensible capacity is calculated to
    //                                     meet the thermostat setpoint. If a HX assisted cooling coil is selected,
    //                                     the HX is off for this calculation. If the latent load is not met by operating
    //                                     the system at the sensible PLR, a new PLR is calculated with the HX operating
    //                                     and the target is the zone SENSIBLE load (thermostat setpoint). Humidity is not
    //                                     controlled in this mode. No reheat coil is used in this configuration.
    // CalcWaterToAirHeatPump - Water-to-air HeatPump routine.
    //                          Calculates a part-load ratio to meet the sensible load.
    // CalcFurnaceOutput - Simulates each child component in order.
    //                     For cooling operation, the heating coil is off.
    //                     For heating operation, the cooling coil is off.
    //                     Reheat or supplemental heating coil is simulated here just to pass the inlet node conditions
    //                     to the output node (actual simulation of these coils is done on the final simulation of the
    //                     child components in SimFurnace).
    //                     Fan is simulated based on placement (drawthru or blowthru).
    // REFERENCES:

    // OTHER NOTES:

    // USE STATEMENTS:
    // Use statements for data only modules
    // Using/Aliasing
    using namespace DataLoopNode;
    using namespace DataHVACGlobals;
    using namespace DataZoneEquipment;
    using Psychrometrics::PsyCpAirFnW;
    using Psychrometrics::PsyHfgAirFnWTdb;
    using Psychrometrics::PsyHFnTdbW;
    using Psychrometrics::PsyRhoAirFnPbTdbW;
    using Psychrometrics::PsyTdbFnHW;
    using namespace ScheduleManager;
    using DXCoils::SimDXCoil;
    using Fans::SimulateFanComponents;

    // Data
    // MODULE PARAMETER DEFINITIONS
    static constexpr std::string_view BlankString;

    constexpr std::string_view fluidNameSteam("STEAM");

    // DERIVED TYPE DEFINITIONS

    // MODULE VARIABLE DECLARATIONS:

    // Subroutine Specifications for the Module
    // Driver/Manager Routines

    // Get Input routines for module

    // Initialization routines for module

    // Calculate routines to check convergence

    // Supporting routines for module

    // modules for variable speed heat pump

    // Reporting routines for module

    // Object Data

    // Utility routines for module
    // na

    // MODULE SUBROUTINES:
    //*************************************************************************

    // Functions

    void SimFurnace(EnergyPlusData &state,
                    std::string_view FurnaceName,
                    bool const FirstHVACIteration,
                    int const AirLoopNum, // Primary air loop number
                    int &CompIndex        // Pointer to which furnace
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   Jan 2001
        //       MODIFIED       Richard Liesen, Oct 2001 - Richard Raustad; Bo Shen, March 2012, for VS WSHP
        //       RE-ENGINEERED  Feb 2001

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine manages Furnace component simulation.

        // METHODOLOGY EMPLOYED:
        // Call the calc routine to determine an operating PLR. Resimulate child components at this PLR.
        // A supplemental heater augments the heating capacity for both air-to-air and water-to-air heat pump systems.
        // A reheat coil is used for the HeatCool furnace/unitarysystem to offset the sensible cooling when the
        // dehumidification control type is COOLREHEAT. Both the supplemental and reheat heating coil load is calculated
        // in the Calc routines and returned here through subroutine arguments. The actual simulation of these coils is
        // performed here (i.e. the supplemental and reheat coil loads are passed as 0 to CalcFurnaceOutput).

        // Using/Aliasing
        using HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil;
        using namespace DataZoneEnergyDemands;

        using WaterToAirHeatPumpSimple::SimWatertoAirHPSimple;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int FurnaceNum;           // Furnace number
        Real64 HeatCoilLoad(0.0); // Zone heating coil load
        Real64 ReheatCoilLoad;    // Load to be met by the reheat coil (if high humidity control)
        Real64 ZoneLoad;          // Control zone sensible load
        Real64 MoistureLoad;      // Control zone latent load
        Real64 H2OHtOfVap;        // Heat of vaporization of air
        int FurnaceInletNode;     // Inlet node to furnace or unitary system
        Real64 FurnaceSavMdot;    // saved furnace inlet air mass flow rate [m3/s]
        Real64 Dummy(0.0);
        CompressorOperation CompressorOp; // compressor operation; 1=on, 0=off
        Real64 OnOffAirFlowRatio;         // Ratio of compressor ON air flow to AVERAGE air flow over time step
        int FanOpMode;                    // Fan operating mode (1=CycFanCycCoil, 2=ContFanCycCoil)
        bool HXUnitOn;                    // flag to control HX assisted cooling coil
        Real64 ZoneLoadToCoolSPSequenced;
        Real64 ZoneLoadToHeatSPSequenced;

        Real64 QActual;           // actual heating coil output (W)
        bool SuppHeatingCoilFlag; // true if supplemental heating coil
        Real64 TempMassFlowRateMaxAvail;

        // Obtains and Allocates Furnace related parameters from input file
        if (state.dataFurnaces->GetFurnaceInputFlag) { // First time subroutine has been entered
            // Get the furnace input
            GetFurnaceInput(state);
            state.dataFurnaces->GetFurnaceInputFlag = false;
        }

        // Save the current AFNLoopHeatingCoilMaxRTF for comparison with the one calculated below
        Real64 refAFNLoopHeatingCoilMaxRTF(0.0);
        if (state.afn->distribution_simulated) {
            refAFNLoopHeatingCoilMaxRTF = state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).AFNLoopHeatingCoilMaxRTF;
        }

        // Find the correct Furnace
        if (CompIndex == 0) {
            FurnaceNum = UtilityRoutines::FindItemInList(FurnaceName, state.dataFurnaces->Furnace);
            if (FurnaceNum == 0) {
                ShowFatalError(state, format("SimFurnace: Unit not found={}", FurnaceName));
            }
            CompIndex = FurnaceNum;
        } else {
            FurnaceNum = CompIndex;
            if (FurnaceNum > state.dataFurnaces->NumFurnaces || FurnaceNum < 1) {
                ShowFatalError(state,
                               format("SimFurnace:  Invalid CompIndex passed={}, Number of Units={}, Entered Unit name={}",
                                      FurnaceNum,
                                      state.dataFurnaces->NumFurnaces,
                                      FurnaceName));
            }
            if (state.dataFurnaces->CheckEquipName(FurnaceNum)) {
                if (FurnaceName != state.dataFurnaces->Furnace(FurnaceNum).Name) {
                    ShowFatalError(state,
                                   format("SimFurnace: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                          FurnaceNum,
                                          FurnaceName,
                                          state.dataFurnaces->Furnace(FurnaceNum).Name));
                }
                state.dataFurnaces->CheckEquipName(FurnaceNum) = false;
            }
        }

        HXUnitOn = false;
        OnOffAirFlowRatio = 0.0;
        // here we need to deal with sequenced zone equip
        ZoneLoad = 0.0;
        if (state.dataFurnaces->Furnace(FurnaceNum).ZoneSequenceCoolingNum > 0 &&
            state.dataFurnaces->Furnace(FurnaceNum).ZoneSequenceHeatingNum > 0) {
            ZoneLoadToCoolSPSequenced = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                                            .SequencedOutputRequiredToCoolingSP(state.dataFurnaces->Furnace(FurnaceNum).ZoneSequenceCoolingNum);
            ZoneLoadToHeatSPSequenced = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                                            .SequencedOutputRequiredToHeatingSP(state.dataFurnaces->Furnace(FurnaceNum).ZoneSequenceHeatingNum);
            if (ZoneLoadToHeatSPSequenced > 0.0 && ZoneLoadToCoolSPSequenced > 0.0 &&
                state.dataHeatBalFanSys->TempControlType(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum) !=
                    DataHVACGlobals::ThermostatType::SingleCooling) {
                ZoneLoad = ZoneLoadToHeatSPSequenced;
            } else if (ZoneLoadToHeatSPSequenced > 0.0 && ZoneLoadToCoolSPSequenced > 0.0 &&
                       state.dataHeatBalFanSys->TempControlType(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum) ==
                           DataHVACGlobals::ThermostatType::SingleCooling) {
                ZoneLoad = 0.0;
            } else if (ZoneLoadToHeatSPSequenced < 0.0 && ZoneLoadToCoolSPSequenced < 0.0 &&
                       state.dataHeatBalFanSys->TempControlType(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum) !=
                           DataHVACGlobals::ThermostatType::SingleHeating) {
                ZoneLoad = ZoneLoadToCoolSPSequenced;
            } else if (ZoneLoadToHeatSPSequenced < 0.0 && ZoneLoadToCoolSPSequenced < 0.0 &&
                       state.dataHeatBalFanSys->TempControlType(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum) ==
                           DataHVACGlobals::ThermostatType::SingleHeating) {
                ZoneLoad = 0.0;
            } else if (ZoneLoadToHeatSPSequenced <= 0.0 && ZoneLoadToCoolSPSequenced >= 0.0) {
                ZoneLoad = 0.0;
            }
            MoistureLoad = state.dataZoneEnergyDemand->ZoneSysMoistureDemand(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                               .SequencedOutputRequiredToDehumidSP(state.dataFurnaces->Furnace(FurnaceNum).ZoneSequenceCoolingNum);
        } else {
            ZoneLoad =
                state.dataZoneEnergyDemand->ZoneSysEnergyDemand(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum).RemainingOutputRequired;
            MoistureLoad = state.dataZoneEnergyDemand->ZoneSysMoistureDemand(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                               .OutputRequiredToDehumidifyingSP;
        }

        H2OHtOfVap = PsyHfgAirFnWTdb(state.dataLoopNodes->Node(state.dataFurnaces->Furnace(FurnaceNum).NodeNumOfControlledZone).HumRat,
                                     state.dataLoopNodes->Node(state.dataFurnaces->Furnace(FurnaceNum).NodeNumOfControlledZone).Temp);

        MoistureLoad *= H2OHtOfVap;

        // Initialize Furnace Flows
        InitFurnace(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, FanOpMode, ZoneLoad, MoistureLoad, FirstHVACIteration);

        FurnaceInletNode = state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum;

        // MassFlowRateMaxAvail issues are impeding non-VAV air loop equipment by limiting air flow
        // temporarily open up flow limits while simulating, and then set this same value at the INLET after this parent has simulated
        TempMassFlowRateMaxAvail = state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRateMaxAvail;
        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRateMaxAvail = state.dataFurnaces->Furnace(FurnaceNum).DesignMassFlowRate;

        FurnaceSavMdot = state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate;
        CompressorOp = CompressorOperation::On;
        state.dataFurnaces->CoolHeatPLRRat = 1.0;

        // Simulate correct system type (1 of 4 choices)
        switch (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num) {
            // Simulate HeatOnly systems:
        case Furnace_HeatOnly:
        case UnitarySys_HeatOnly: {
            // Update the furnace flow rates
            CalcNewZoneHeatOnlyFlowRates(state, FurnaceNum, FirstHVACIteration, ZoneLoad, HeatCoilLoad, OnOffAirFlowRatio);

            if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == BlowThru) {
                // simulate fan
                SimulateFanComponents(
                    state, BlankString, FirstHVACIteration, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->FanSpeedRatio);
            }

            // simulate furnace heating coil
            SuppHeatingCoilFlag = false; // if true simulates supplemental heating coil
            CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, FanOpMode, QActual);

            if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == DrawThru) {
                // simulate fan
                SimulateFanComponents(
                    state, BlankString, FirstHVACIteration, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->FanSpeedRatio);
            }
        } break;
            // Simulate HeatCool sytems:
        case Furnace_HeatCool:
        case UnitarySys_HeatCool: {
            if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == Coil_CoolingAirToAirVariableSpeed) {
                // variable speed cooling coil
                HeatCoilLoad = 0.0;
                if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP)
                    state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).ControlledZoneTemp =
                        state.dataLoopNodes->Node(state.dataFurnaces->Furnace(FurnaceNum).NodeNumOfControlledZone).Temp;
                SimVariableSpeedHP(state, FurnaceNum, FirstHVACIteration, AirLoopNum, ZoneLoad, MoistureLoad, OnOffAirFlowRatio);
            } else {
                // calculate the system flow rate
                if (!FirstHVACIteration && state.dataFurnaces->Furnace(FurnaceNum).OpMode == CycFanCycCoil && state.dataFurnaces->CoolingLoad &&
                    state.dataAirLoop->AirLoopControlInfo(AirLoopNum).EconoActive) {
                    // for cycling fan, cooling load, check whether furnace can meet load with compressor off
                    CompressorOp = CompressorOperation::Off;
                    CalcNewZoneHeatCoolFlowRates(state,
                                                 FurnaceNum,
                                                 FirstHVACIteration,
                                                 CompressorOp,
                                                 ZoneLoad,
                                                 MoistureLoad,
                                                 HeatCoilLoad,
                                                 ReheatCoilLoad,
                                                 OnOffAirFlowRatio,
                                                 HXUnitOn);
                    if (state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio >= 1.0 ||
                        state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio >= 1.0 ||
                        (state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio <= 0.0 &&
                         state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio <= 0.0)) {
                        // compressor on (reset inlet air mass flow rate to starting value)
                        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = FurnaceSavMdot;
                        CompressorOp = CompressorOperation::On;
                        CalcNewZoneHeatCoolFlowRates(state,
                                                     FurnaceNum,
                                                     FirstHVACIteration,
                                                     CompressorOp,
                                                     ZoneLoad,
                                                     MoistureLoad,
                                                     HeatCoilLoad,
                                                     ReheatCoilLoad,
                                                     OnOffAirFlowRatio,
                                                     HXUnitOn);
                    }
                } else {
                    // compressor on
                    CalcNewZoneHeatCoolFlowRates(state,
                                                 FurnaceNum,
                                                 FirstHVACIteration,
                                                 CompressorOp,
                                                 ZoneLoad,
                                                 MoistureLoad,
                                                 HeatCoilLoad,
                                                 ReheatCoilLoad,
                                                 OnOffAirFlowRatio,
                                                 HXUnitOn);
                }

                if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == BlowThru) {
                    // simulate fan
                    SimulateFanComponents(
                        state, BlankString, FirstHVACIteration, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->FanSpeedRatio);
                }

                if (!state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilUpstream) {
                    // simulate furnace heating coil
                    SuppHeatingCoilFlag = false; // if true simulates supplemental heating coil
                    CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, FanOpMode, QActual);
                }

                // simulate furnace DX cooling coil
                if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == CoilDX_CoolingHXAssisted) {
                    SimHXAssistedCoolingCoil(state,
                                             BlankString,
                                             FirstHVACIteration,
                                             CompressorOp,
                                             state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio,
                                             state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                             FanOpMode,
                                             HXUnitOn,
                                             OnOffAirFlowRatio,
                                             state.dataFurnaces->EconomizerFlag);
                } else {
                    SimDXCoil(state,
                              BlankString,
                              CompressorOp,
                              FirstHVACIteration,
                              state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                              FanOpMode,
                              state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio,
                              OnOffAirFlowRatio,
                              state.dataFurnaces->CoolHeatPLRRat);
                }

                if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilUpstream) {
                    // simulate furnace heating coil
                    SuppHeatingCoilFlag = false; // if true simulates supplemental heating coil
                    CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, FanOpMode, QActual);
                }

                if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == DrawThru) {
                    // simulate fan
                    SimulateFanComponents(
                        state, BlankString, FirstHVACIteration, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->FanSpeedRatio);
                }

                // Simulate furnace reheat coil if a humidistat is used or if the reheat coil is present
                if (state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::CoolReheat ||
                    state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex > 0) {
                    SuppHeatingCoilFlag = true; // if truee simulates supplemental heating coil
                    CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, ReheatCoilLoad, FanOpMode, QActual);
                }
            }
        } break;
            // Simulate air-to-air heat pumps:
        case UnitarySys_HeatPump_AirToAir: {
            if (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingAirToAirVariableSpeed) {
                // variable speed heat pump
                HeatCoilLoad = 0.0;
                if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                    state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).ControlledZoneTemp =
                        state.dataLoopNodes->Node(state.dataFurnaces->Furnace(FurnaceNum).NodeNumOfControlledZone).Temp;
                    state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).IDFanID =
                        state.dataFurnaces->Furnace(FurnaceNum).FanIndex;
                    state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).IDFanName = BlankString;
                    state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).IDFanPlace =
                        state.dataFurnaces->Furnace(FurnaceNum).FanPlace;
                }

                SimVariableSpeedHP(state, FurnaceNum, FirstHVACIteration, AirLoopNum, ZoneLoad, MoistureLoad, OnOffAirFlowRatio);
            } else {
                // Update the furnace flow rates
                if (!FirstHVACIteration && state.dataFurnaces->Furnace(FurnaceNum).OpMode == CycFanCycCoil && state.dataFurnaces->CoolingLoad &&
                    state.dataAirLoop->AirLoopControlInfo(AirLoopNum).EconoActive) {
                    // for cycling fan, cooling load, check whether furnace can meet load with compressor off
                    CompressorOp = CompressorOperation::Off;
                    CalcNewZoneHeatCoolFlowRates(state,
                                                 FurnaceNum,
                                                 FirstHVACIteration,
                                                 CompressorOp,
                                                 ZoneLoad,
                                                 MoistureLoad,
                                                 HeatCoilLoad,
                                                 ReheatCoilLoad,
                                                 OnOffAirFlowRatio,
                                                 HXUnitOn);
                    if (state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio >= 1.0 ||
                        state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio >= 1.0 ||
                        (state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio <= 0.0 &&
                         state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio <= 0.0)) {
                        // compressor on (reset inlet air mass flow rate to starting value)
                        CompressorOp = CompressorOperation::On;
                        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = FurnaceSavMdot;
                        CalcNewZoneHeatCoolFlowRates(state,
                                                     FurnaceNum,
                                                     FirstHVACIteration,
                                                     CompressorOp,
                                                     ZoneLoad,
                                                     MoistureLoad,
                                                     HeatCoilLoad,
                                                     ReheatCoilLoad,
                                                     OnOffAirFlowRatio,
                                                     HXUnitOn);
                    }
                } else {
                    // compressor on
                    CalcNewZoneHeatCoolFlowRates(state,
                                                 FurnaceNum,
                                                 FirstHVACIteration,
                                                 CompressorOp,
                                                 ZoneLoad,
                                                 MoistureLoad,
                                                 HeatCoilLoad,
                                                 ReheatCoilLoad,
                                                 OnOffAirFlowRatio,
                                                 HXUnitOn);
                }

                if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == BlowThru) {
                    SimulateFanComponents(
                        state, BlankString, FirstHVACIteration, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->FanSpeedRatio);
                }

                if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == CoilDX_CoolingHXAssisted) {
                    SimHXAssistedCoolingCoil(state,
                                             BlankString,
                                             FirstHVACIteration,
                                             CompressorOp,
                                             state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio,
                                             state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                             FanOpMode,
                                             HXUnitOn,
                                             OnOffAirFlowRatio,
                                             state.dataFurnaces->EconomizerFlag);
                } else {
                    SimDXCoil(state,
                              BlankString,
                              CompressorOp,
                              FirstHVACIteration,
                              state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                              FanOpMode,
                              state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio,
                              OnOffAirFlowRatio);
                }
                SimDXCoil(state,
                          BlankString,
                          CompressorOp,
                          FirstHVACIteration,
                          state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex,
                          FanOpMode,
                          state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio,
                          OnOffAirFlowRatio);
                if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == DrawThru) {
                    SimulateFanComponents(
                        state, BlankString, FirstHVACIteration, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->FanSpeedRatio);
                }

                // Simulate furnace reheat coil if a humidistat is present, the dehumidification type of coolreheat and
                // reheat coil load exists
                if (state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::CoolReheat &&
                    ReheatCoilLoad > 0.0) {
                    SuppHeatingCoilFlag = true; // if truee simulates supplemental heating coil
                    CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, ReheatCoilLoad, FanOpMode, QActual);
                } else {
                    SuppHeatingCoilFlag = true; // if true simulates supplemental heating coil
                    CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, FanOpMode, QActual);
                }
            }
        } break;
        // Simulate water-to-air systems:
        case UnitarySys_HeatPump_WaterToAir: {
            if (state.dataFurnaces->Furnace(FurnaceNum).WatertoAirHPType == WatertoAir_Simple) {
                // Update the furnace flow rates
                //   When CompressorOp logic is added to the child cooling coil (COIL:WaterToAirHP:EquationFit:Cooling), then this logic
                //   needs to be reinstated... to align with Unitary/Furnace HeatCool and Unitary Air-to-Air Heat Pump (see above).
                if (!FirstHVACIteration && state.dataFurnaces->Furnace(FurnaceNum).OpMode == CycFanCycCoil && state.dataFurnaces->CoolingLoad &&
                    state.dataAirLoop->AirLoopControlInfo(AirLoopNum).EconoActive) {
                    // for cycling fan, cooling load, check whether furnace can meet load with compressor off
                    CompressorOp = CompressorOperation::Off;
                    CalcNewZoneHeatCoolFlowRates(state,
                                                 FurnaceNum,
                                                 FirstHVACIteration,
                                                 CompressorOp,
                                                 ZoneLoad,
                                                 MoistureLoad,
                                                 HeatCoilLoad,
                                                 ReheatCoilLoad,
                                                 OnOffAirFlowRatio,
                                                 HXUnitOn);
                    if (state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio >= 1.0 ||
                        state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio >= 1.0 ||
                        (state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio <= 0.0 &&
                         state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio <= 0.0)) {
                        // compressor on (reset inlet air mass flow rate to starting value)
                        CompressorOp = CompressorOperation::On;
                        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = FurnaceSavMdot;
                        CalcNewZoneHeatCoolFlowRates(state,
                                                     FurnaceNum,
                                                     FirstHVACIteration,
                                                     CompressorOp,
                                                     ZoneLoad,
                                                     MoistureLoad,
                                                     HeatCoilLoad,
                                                     ReheatCoilLoad,
                                                     OnOffAirFlowRatio,
                                                     HXUnitOn);
                    }
                } else {
                    // compressor on
                    CalcNewZoneHeatCoolFlowRates(state,
                                                 FurnaceNum,
                                                 FirstHVACIteration,
                                                 CompressorOp,
                                                 ZoneLoad,
                                                 MoistureLoad,
                                                 HeatCoilLoad,
                                                 ReheatCoilLoad,
                                                 OnOffAirFlowRatio,
                                                 HXUnitOn);
                }
                if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == BlowThru) {
                    SimulateFanComponents(
                        state, BlankString, FirstHVACIteration, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->FanSpeedRatio);
                }

                SimWatertoAirHPSimple(state,
                                      BlankString,
                                      state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                      state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand,
                                      state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilLatentDemand,
                                      state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                                      state.dataFurnaces->Furnace(FurnaceNum).WSHPRuntimeFrac,
                                      state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                                      state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                                      state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                                      CompressorOp,
                                      state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio,
                                      FirstHVACIteration);
                SimWatertoAirHPSimple(state,
                                      BlankString,
                                      state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex,
                                      state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand,
                                      Dummy,
                                      state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                                      state.dataFurnaces->Furnace(FurnaceNum).WSHPRuntimeFrac,
                                      state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                                      state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                                      state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                                      CompressorOp,
                                      state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio,
                                      FirstHVACIteration);

                if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == DrawThru) {
                    SimulateFanComponents(
                        state, BlankString, FirstHVACIteration, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->FanSpeedRatio);
                }
                if (state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::CoolReheat &&
                    ReheatCoilLoad > 0.0) {
                    SuppHeatingCoilFlag = true; // if true simulates supplemental heating coil
                    CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, ReheatCoilLoad, FanOpMode, QActual);
                } else {
                    SuppHeatingCoilFlag = true; // if true simulates supplemental heating coil
                    CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, FanOpMode, QActual);
                }
            } else if (state.dataFurnaces->Furnace(FurnaceNum).WatertoAirHPType == WatertoAir_ParEst) {

                // simulate the heat pump
                HeatCoilLoad = 0.0;
                CalcWaterToAirHeatPump(state, AirLoopNum, FurnaceNum, FirstHVACIteration, CompressorOp, ZoneLoad, MoistureLoad);
            } else if (state.dataFurnaces->Furnace(FurnaceNum).WatertoAirHPType == WatertoAir_VarSpeedEquationFit) {
                // simulate the heat pump
                HeatCoilLoad = 0.0;
                if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP)
                    state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).ControlledZoneTemp =
                        state.dataLoopNodes->Node(state.dataFurnaces->Furnace(FurnaceNum).NodeNumOfControlledZone).Temp;
                SimVariableSpeedHP(state, FurnaceNum, FirstHVACIteration, AirLoopNum, ZoneLoad, MoistureLoad, OnOffAirFlowRatio);

            } else if (state.dataFurnaces->Furnace(FurnaceNum).WatertoAirHPType == WatertoAir_VarSpeedLooUpTable) {
                HeatCoilLoad = 0.0; // Added: Used below
            } else {
                assert(false); //? If all possible states covered by if conditions change to HeatCoilLoad = 0.0;
            }
        } break;
        default: {
            // will never get here, all system types are simulated above
            assert(false);
        } break;
        }

        // set the econo lockout flags
        if (state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio > 0.0 &&
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CanLockoutEconoWithCompressor) {
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).ReqstEconoLockoutWithCompressor = true;
        } else {
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).ReqstEconoLockoutWithCompressor = false;
        }

        if ((HeatCoilLoad > 0.0 || state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio > 0.0) &&
            (state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CanLockoutEconoWithCompressor ||
             state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CanLockoutEconoWithHeating)) {
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).ReqstEconoLockoutWithHeating = true;
        } else {
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).ReqstEconoLockoutWithHeating = false;
        }

        if (state.dataFurnaces->Furnace(FurnaceNum).OpMode == CycFanCycCoil) {
            state.dataAirLoop->AirLoopFlow(AirLoopNum).FanPLR = state.dataFurnaces->Furnace(FurnaceNum).FanPartLoadRatio;
        } else {
            state.dataAirLoop->AirLoopFlow(AirLoopNum).FanPLR = 1.0; // 1 means constant fan does not cycle.
        }

        // Report the current Furnace output
        ReportFurnace(state, FurnaceNum, AirLoopNum);

        // Get the actual maximum RTF for AFN simulations
        if (state.afn->distribution_simulated) {
            Real64 heatingCoilRTF = 0.0;
            Real64 suppHeatingCoilRTF = 0.0;
            bool errorFound(false);
            switch (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num) {
            case Coil_HeatingGasOrOtherFuel:
            case Coil_HeatingElectric:
            case Coil_HeatingDesuperheater: {
                int heatingCoilIndex;
                HeatingCoils::GetCoilIndex(state, state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilName, heatingCoilIndex, errorFound);
                if (heatingCoilIndex > 0) {
                    heatingCoilRTF = state.dataHeatingCoils->HeatingCoil(heatingCoilIndex).RTF;
                }
            } break;
            default:;
            }
            if (errorFound) {
                ShowSevereError(state, format("The index of \"{}\" is not found", state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilName));
                ShowContinueError(state, format("...occurs for {}", state.dataFurnaces->Furnace(FurnaceNum).Name));
                errorFound = false;
            }
            switch (state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType_Num) {
            case Coil_HeatingGasOrOtherFuel:
            case Coil_HeatingElectric:
            case Coil_HeatingDesuperheater: {
                int suppHeatingCoilIndex;
                HeatingCoils::GetCoilIndex(state, state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilName, suppHeatingCoilIndex, errorFound);
                if (suppHeatingCoilIndex > 0) {
                    suppHeatingCoilRTF = state.dataHeatingCoils->HeatingCoil(suppHeatingCoilIndex).RTF;
                }
            } break;
            default:;
            }
            if (errorFound) {
                ShowSevereError(state, format("The index of \"{}\" is not found", state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilName));
                ShowContinueError(state, format("...occurs for {}", state.dataFurnaces->Furnace(FurnaceNum).Name));
                errorFound = false;
            }
            state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).AFNLoopHeatingCoilMaxRTF =
                max(refAFNLoopHeatingCoilMaxRTF, heatingCoilRTF, suppHeatingCoilRTF);
        }

        
        // Reset OnOffFanPartLoadFraction to 1 in case another on/off fan is called without a part-load curve
        state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;

        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRateMaxAvail = TempMassFlowRateMaxAvail;
    }

    // Get Input Section of the Module
    //******************************************************************************

    void GetFurnaceInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   Feb 2001
        //       MODIFIED       Don Shirey and Rich Raustad, Mar/Oct 2001, Mar 2003
        //                      Bereket Nigusse, April 2010 - deprecated supply air flow fraction through
        //                      controlled zone from the input field.
        //                      Bo Shen, March 2012, add inputs for VS WSHP,
        //                      Bo Shen, ORNL, July 2012 - added variable-speed air source heat pump cooling and heating coils, using curve-fits
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for fans and coils and stores it in the Furnace data structures

        // METHODOLOGY EMPLOYED:
        // Uses "Get" routines to read in data.

        // Using/Aliasing
        using BranchNodeConnections::SetUpCompSets;
        using BranchNodeConnections::TestCompSet;
        using DXCoils::GetCoilTypeNum;
        using DXCoils::GetDXCoilIndex;
        using DXCoils::SetDXCoolingCoilData;
        using Fans::GetFanAvailSchPtr;
        using Fans::GetFanDesignVolumeFlowRate;
        using Fans::GetFanIndex;
        using Fans::GetFanInletNode;
        using Fans::GetFanOutletNode;
        using Fans::GetFanType;
        using FluidProperties::GetSatDensityRefrig;
        using HeatingCoils::GetHeatingCoilPLFCurveIndex;
        using HeatingCoils::GetHeatingCoilTypeNum;
        using HVACHXAssistedCoolingCoil::GetActualDXCoilIndex;
        using HVACHXAssistedCoolingCoil::GetHXDXCoilIndex;
        using HVACHXAssistedCoolingCoil::GetHXDXCoilName;
        using NodeInputManager::GetOnlySingleNode;
        using SteamCoils::GetCoilAirOutletNode;
        using SteamCoils::GetCoilSteamInletNode;
        using VariableSpeedCoils::GetCoilAirFlowRateVariableSpeed;
        using VariableSpeedCoils::GetCoilCapacityVariableSpeed;
        using VariableSpeedCoils::GetCoilIndexVariableSpeed;
        using VariableSpeedCoils::GetCoilInletNodeVariableSpeed;
        using VariableSpeedCoils::GetCoilOutletNodeVariableSpeed;
        using VariableSpeedCoils::GetVSCoilCondenserInletNode;
        using VariableSpeedCoils::SetVarSpeedCoilData;
        using WaterCoils::GetCoilMaxWaterFlowRate;
        using WaterCoils::GetCoilWaterInletNode;
        using WaterToAirHeatPumpSimple::SetSimpleWSHPData;

        using EMSManager::ManageEMS;
        using HVACControllers::CheckCoilWaterInletNode;
        using IntegratedHeatPump::GetCoilIndexIHP;
        using OutAirNodeManager::CheckOutAirNodeNumber;

        // Locals
        std::string_view CurrentModuleObject; // Object type for getting and error messages

        // SUBROUTINE PARAMETER DEFINITIONS:
        std::string_view constexpr getUnitaryHeatOnly("GetUnitaryHeatOnly");
        std::string_view constexpr getAirLoopHVACHeatCoolInput("GetAirLoopHVACHeatCoolInput");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int FurnaceNum;                // The Furnace that you are currently loading input into
        int GetObjectNum;              // The index to each specific object name
        int NumFields;                 // Total number of fields in object
        int NumAlphas;                 // Total number of alpha fields in object
        int MaxAlphas;                 // Maximum number of alpha fields in all objects
        int NumNumbers;                // Total number of numeric fields in object
        int MaxNumbers;                // Maximum number of numeric fields in all objects
        int IOStatus;                  // Function call status
        Array1D<Real64> Numbers;       // Numeric data
        Array1D_string Alphas;         // Alpha data
        Array1D_string cAlphaFields;   // Alpha field names
        Array1D_string cNumericFields; // Numeric field names
        Array1D_bool lAlphaBlanks;     // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;   // Logical array, numeric field input BLANK = .TRUE.
        std::string CompSetFanInlet;
        std::string CompSetFanOutlet;
        std::string CompSetCoolInlet;
        std::string CompSetHeatInlet;
        std::string CompSetHeatOutlet;
        bool ErrorsFound(false);       // If errors detected in input
        bool IsNotOK;                  // Flag to verify name
        int NumHeatOnly;               // Number of heat only furnaces
        int NumHeatCool;               // Number of heat/cool furnaces
        int HeatOnlyNum;               // Index to heat only furnaces
        int HeatCoolNum;               // Index to heat/cool furnaces
        int NumUnitaryHeatOnly;        // Number of heat only unitary systems
        int NumUnitaryHeatCool;        // Number of heat/cool unitary systems
        int UnitaryHeatOnlyNum;        // Index to heat only furnaces
        int UnitaryHeatCoolNum;        // Index to heat/cool unitary systems
        int NumWaterToAirHeatPump;     // Number of water-to-air heat pumps
        int NumHeatPump;               // Number of air-to-air or water-to-air heat pumps
        int HeatPumpNum;               // Index to air-to-air heat pumps
        bool AirNodeFound;             // Used to determine if control zone is valid
        bool AirLoopFound;             // Used to determine if control zone is served by furnace air loop
        int BranchNum;                 // Used to determine if control zone is served by furnace air loop
        int CompNum;                   // Used to determine if control zone is served by furnace air loop
        int TstatZoneNum;              // Used to determine if control zone has a thermostat object
        int HStatZoneNum;              // Used to determine if control zone has a humidistat object
        bool errFlag;                  // Mining function error flag
        int FanInletNode;              // Used for node checking warning messages
        int FanOutletNode;             // Used for node checking warning messages
        int CoolingCoilInletNode;      // Used for node checking warning messages
        int CoolingCoilOutletNode;     // Used for node checking warning messages
        int HeatingCoilInletNode;      // Used for node checking warning messages
        int HeatingCoilOutletNode;     // Used for node checking warning messages
        int SupHeatCoilInletNode;      // Used for node checking warning messages
        int SupHeatCoilOutletNode;     // Used for node checking warning messages
        int ReheatCoilInletNode;       // Used for node checking warning messages
        int ReheatCoilOutletNode;      // Used for node checking warning messages
        Real64 FanVolFlowRate;         // Fan Max Flow Rate from Fan object (for comparisons to validity)
        int FurnaceType_Num;           // Integer equivalent of Furnace or UnitarySystem "type"
        std::string CoolingCoilType;   // Used in mining function CALLS
        std::string CoolingCoilName;   // Used in mining function CALLS
        std::string HeatingCoilType;   // Used in mining function CALLS
        std::string HeatingCoilName;   // Used in mining function CALLS
        std::string ReheatingCoilType; // Used in mining function CALLS
        std::string ReheatingCoilName; // Used in mining function CALLS
        std::string SuppHeatCoilType;  // Used in mining function CALLS
        std::string SuppHeatCoilName;  // Used in mining function CALLS
        std::string FanType;           // Used in mining function CALLS
        std::string FanName;           // Used in mining function CALLS
        bool PrintMessage;             // Used in mining function CALLS
        int HeatingCoilPLFCurveIndex;  // index of heating coil PLF curve
        int SteamIndex;                // steam coil index
        Real64 SteamDensity;           // density of steam at 100C
        int DXCoilIndex;               // Index to DX coil in HXAssited object
        std::string IHPCoilName;       // IHP cooling coil name
        int IHPCoilIndex(0);           // IHP cooling coil id
        auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;
        DataLoopNode::ConnectionObjectType currentModuleObjectType;

        state.dataFurnaces->GetFurnaceInputFlag = false;
        MaxNumbers = 0;
        MaxAlphas = 0;

        CurrentModuleObject = "AirLoopHVAC:Unitary:Furnace:HeatOnly";
        NumHeatOnly = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumFields, NumAlphas, NumNumbers);
        MaxNumbers = max(MaxNumbers, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);

        CurrentModuleObject = "AirLoopHVAC:Unitary:Furnace:HeatCool";
        NumHeatCool = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumFields, NumAlphas, NumNumbers);
        MaxNumbers = max(MaxNumbers, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);

        CurrentModuleObject = "AirLoopHVAC:UnitaryHeatOnly";
        NumUnitaryHeatOnly = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumFields, NumAlphas, NumNumbers);
        MaxNumbers = max(MaxNumbers, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);

        CurrentModuleObject = "AirLoopHVAC:UnitaryHeatCool";
        NumUnitaryHeatCool = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumFields, NumAlphas, NumNumbers);
        MaxNumbers = max(MaxNumbers, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);

        CurrentModuleObject = "AirLoopHVAC:UnitaryHeatPump:AirToAir";
        NumHeatPump = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumFields, NumAlphas, NumNumbers);
        MaxNumbers = max(MaxNumbers, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);

        CurrentModuleObject = "AirLoopHVAC:UnitaryHeatPump:WaterToAir";
        NumWaterToAirHeatPump = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumFields, NumAlphas, NumNumbers);
        MaxNumbers = max(MaxNumbers, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);

        Alphas.allocate(MaxAlphas);
        Numbers.dimension(MaxNumbers, 0.0);
        cAlphaFields.allocate(MaxAlphas);
        cNumericFields.allocate(MaxNumbers);
        lAlphaBlanks.dimension(MaxAlphas, true);
        lNumericBlanks.dimension(MaxNumbers, true);

        state.dataFurnaces->NumFurnaces = NumHeatOnly + NumHeatCool + NumUnitaryHeatOnly + NumUnitaryHeatCool + NumHeatPump + NumWaterToAirHeatPump;

        if (state.dataFurnaces->NumFurnaces > 0) {
            state.dataFurnaces->Furnace.allocate(state.dataFurnaces->NumFurnaces);
            state.dataFurnaces->UniqueFurnaceNames.reserve(state.dataFurnaces->NumFurnaces);
        }
        state.dataFurnaces->CheckEquipName.dimension(state.dataFurnaces->NumFurnaces, true);

        IHPCoilIndex = 0;

        // Get the data for the HeatOnly Furnace
        for (HeatOnlyNum = 1; HeatOnlyNum <= NumHeatOnly + NumUnitaryHeatOnly; ++HeatOnlyNum) {

            FanInletNode = 0;
            FanOutletNode = 0;
            FanVolFlowRate = 0.0;
            HeatingCoilInletNode = 0;
            HeatingCoilOutletNode = 0;
            CoolingCoilType = ' ';
            CoolingCoilName = ' ';
            HeatingCoilType = ' ';
            HeatingCoilName = ' ';

            //       Furnace and UnitarySystem objects are both read in here.
            //       Will still have 2 differently named objects for the user, but read in with 1 DO loop.
            if (HeatOnlyNum <= NumHeatOnly) {
                CurrentModuleObject = "AirLoopHVAC:Unitary:Furnace:HeatOnly";
                currentModuleObjectType = DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryFurnaceHeatOnly;
                FurnaceType_Num = Furnace_HeatOnly;
                GetObjectNum = HeatOnlyNum;
            } else {
                CurrentModuleObject = "AirLoopHVAC:UnitaryHeatOnly";
                currentModuleObjectType = DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatOnly;
                FurnaceType_Num = UnitarySys_HeatOnly;
                GetObjectNum = HeatOnlyNum - NumHeatOnly;
            }

            FurnaceNum = HeatOnlyNum;
            state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num = FurnaceType_Num;
            state.dataFurnaces->Furnace(FurnaceNum).iterationMode.allocate(3);

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     GetObjectNum,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            GlobalNames::VerifyUniqueInterObjectName(
                state, state.dataFurnaces->UniqueFurnaceNames, Alphas(1), CurrentModuleObject, cAlphaFields(1), ErrorsFound);

            state.dataFurnaces->Furnace(FurnaceNum).Name = Alphas(1);
            if (lAlphaBlanks(2)) {
                state.dataFurnaces->Furnace(FurnaceNum).SchedPtr = DataGlobalConstants::ScheduleAlwaysOn;
            } else {
                state.dataFurnaces->Furnace(FurnaceNum).SchedPtr = GetScheduleIndex(state, Alphas(2));
                if (state.dataFurnaces->Furnace(FurnaceNum).SchedPtr == 0) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(2), Alphas(2)));
                    ErrorsFound = true;
                }
            }

            state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum = GetOnlySingleNode(state,
                                                                                            Alphas(3),
                                                                                            ErrorsFound,
                                                                                            currentModuleObjectType,
                                                                                            Alphas(1),
                                                                                            DataLoopNode::NodeFluidType::Air,
                                                                                            DataLoopNode::ConnectionType::Inlet,
                                                                                            NodeInputManager::CompFluidStream::Primary,
                                                                                            ObjectIsParent);
            state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum = GetOnlySingleNode(state,
                                                                                             Alphas(4),
                                                                                             ErrorsFound,
                                                                                             currentModuleObjectType,
                                                                                             Alphas(1),
                                                                                             DataLoopNode::NodeFluidType::Air,
                                                                                             DataLoopNode::ConnectionType::Outlet,
                                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                                             ObjectIsParent);

            TestCompSet(state, CurrentModuleObject, Alphas(1), Alphas(3), Alphas(4), "Air Nodes");

            state.dataFurnaces->Furnace(FurnaceNum).FanSchedPtr = GetScheduleIndex(state, Alphas(5));
            if (!lAlphaBlanks(5) && state.dataFurnaces->Furnace(FurnaceNum).FanSchedPtr == 0) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(5), Alphas(5)));
                ErrorsFound = true;
            } else if (lAlphaBlanks(5)) {
                state.dataFurnaces->Furnace(FurnaceNum).OpMode = CycFanCycCoil;
            }

            // Get the Controlling Zone or Location of the Furnace Thermostat

            state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum = UtilityRoutines::FindItemInList(Alphas(6), state.dataHeatBal->Zone);
            if (state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum == 0) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(6), Alphas(6)));
                ErrorsFound = true;
            }

            // Get the node number for the zone with the thermostat
            if (state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum > 0) {
                AirNodeFound = false;
                AirLoopFound = false;
                int ControlledZoneNum = state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum;
                //             Find the controlled zone number for the specified thermostat location
                state.dataFurnaces->Furnace(FurnaceNum).NodeNumOfControlledZone = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ZoneNode;
                //             Determine if furnace is on air loop served by the thermostat location specified
                for (int zoneInNode = 1; zoneInNode <= state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).NumInletNodes; ++zoneInNode) {
                    int AirLoopNumber = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNodeAirLoopNum(zoneInNode);
                    state.dataFurnaces->Furnace(FurnaceNum).airloopNum = AirLoopNumber;
                    if (AirLoopNumber > 0) {
                        for (BranchNum = 1; BranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).NumBranches; ++BranchNum) {
                            for (CompNum = 1; CompNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).TotalComponents;
                                 ++CompNum) {
                                if (!UtilityRoutines::SameString(
                                        state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).Comp(CompNum).Name,
                                        state.dataFurnaces->Furnace(FurnaceNum).Name) ||
                                    !UtilityRoutines::SameString(
                                        state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).Comp(CompNum).TypeOf,
                                        CurrentModuleObject))
                                    continue;
                                AirLoopFound = true;
                                state.dataFurnaces->Furnace(FurnaceNum).ZoneInletNode =
                                    state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNode(zoneInNode);
                                break;
                            }
                            if (AirLoopFound) break;
                        }
                        for (TstatZoneNum = 1; TstatZoneNum <= state.dataZoneCtrls->NumTempControlledZones; ++TstatZoneNum) {
                            if (state.dataZoneCtrls->TempControlledZone(TstatZoneNum).ActualZoneNum !=
                                state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                                continue;
                            AirNodeFound = true;
                        }
                        for (TstatZoneNum = 1; TstatZoneNum <= state.dataZoneCtrls->NumComfortControlledZones; ++TstatZoneNum) {
                            if (state.dataZoneCtrls->ComfortControlledZone(TstatZoneNum).ActualZoneNum !=
                                state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                                continue;
                            AirNodeFound = true;
                        }
                    }
                    if (AirLoopFound) break;
                }
                if (!AirNodeFound) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowSevereError(state, "Did not find Air Node (Zone with Thermostat).");
                    ShowContinueError(state, format("Specified {} = {}", cAlphaFields(6), Alphas(6)));
                    ShowContinueError(
                        state, "Both a ZoneHVAC:EquipmentConnections object and a ZoneControl:Thermostat object must be specified for this zone.");
                    ErrorsFound = true;
                }
                if (!AirLoopFound) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowSevereError(state, "Did not find correct Primary Air Loop.");
                    ShowContinueError(state, format("Specified {} = {} is not served by this AirLoopHVAC equipment.", cAlphaFields(6), Alphas(6)));
                    ErrorsFound = true;
                }
            }

            // Get fan data
            FanType = Alphas(7);
            FanName = Alphas(8);
            errFlag = false;
            GetFanType(state, FanName, state.dataFurnaces->Furnace(FurnaceNum).FanType_Num, errFlag, CurrentModuleObject, Alphas(1));
            if (errFlag) {
                ErrorsFound = true;
            }
            if (state.dataFurnaces->Furnace(FurnaceNum).FanType_Num == FanType_SimpleOnOff ||
                state.dataFurnaces->Furnace(FurnaceNum).FanType_Num == FanType_SimpleConstVolume) {

                ValidateComponent(state, FanType, FanName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;

                } else { // mine data from fan object

                    // Get the fan index
                    errFlag = false;
                    GetFanIndex(state, FanName, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Set the Design Fan Volume Flow Rate
                    errFlag = false;
                    FanVolFlowRate = GetFanDesignVolumeFlowRate(state, FanType, FanName, errFlag);
                    state.dataFurnaces->Furnace(FurnaceNum).ActualFanVolFlowRate = FanVolFlowRate;

                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} ={}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Get the Fan Inlet Node
                    errFlag = false;
                    FanInletNode = GetFanInletNode(state, FanType, FanName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Get the Fan Outlet Node
                    errFlag = false;
                    FanOutletNode = GetFanOutletNode(state, FanType, FanName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Get the fan's availabitlity schedule
                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).FanAvailSchedPtr = GetFanAvailSchPtr(state, FanType, FanName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Check fan's schedule for cycling fan operation if constant volume fan is used
                    if (state.dataFurnaces->Furnace(FurnaceNum).FanSchedPtr > 0 &&
                        state.dataFurnaces->Furnace(FurnaceNum).FanType_Num == FanType_SimpleConstVolume) {
                        if (!CheckScheduleValueMinMax(state, state.dataFurnaces->Furnace(FurnaceNum).FanSchedPtr, ">", 0.0, "<=", 1.0)) {
                            ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                            ShowContinueError(state, format("For {} = {}", cAlphaFields(7), Alphas(7)));
                            ShowContinueError(state, "Fan operating mode must be continuous (fan operating mode schedule values > 0).");
                            ShowContinueError(state, format("Error found in {} = {}", cAlphaFields(5), Alphas(5)));
                            ShowContinueError(state, "...schedule values must be (>0., <=1.)");
                            ErrorsFound = true;
                        }
                    } else if (lAlphaBlanks(5) && state.dataFurnaces->Furnace(FurnaceNum).FanType_Num != FanType_SimpleOnOff) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ShowContinueError(state, format("{} = {}", cAlphaFields(7), Alphas(7)));
                        ShowContinueError(state, format("Fan type must be Fan:OnOff when {} = Blank.", cAlphaFields(5)));
                        ErrorsFound = true;
                    }

                } // IF (IsNotOK) THEN

            } else { // wrong fan type
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(7), Alphas(7)));
                ErrorsFound = true;
            } // IF (state.dataFurnaces->Furnace(FurnaceNum)%FanType_Num == FanType_SimpleOnOff .OR. &

            if (UtilityRoutines::SameString(Alphas(9), "BlowThrough")) state.dataFurnaces->Furnace(FurnaceNum).FanPlace = BlowThru;
            if (UtilityRoutines::SameString(Alphas(9), "DrawThrough")) state.dataFurnaces->Furnace(FurnaceNum).FanPlace = DrawThru;
            if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == 0) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(9), Alphas(9)));
                ErrorsFound = true;
            }

            // Get coil data
            HeatingCoilType = Alphas(10);
            HeatingCoilName = Alphas(11);
            state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType = HeatingCoilType;
            state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilName = HeatingCoilName;
            if (UtilityRoutines::SameString(HeatingCoilType, "Coil:Heating:Fuel") ||
                UtilityRoutines::SameString(HeatingCoilType, "Coil:Heating:Electric")) {
                errFlag = false;
                state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num =
                    HeatingCoils::GetHeatingCoilTypeNum(state, HeatingCoilType, HeatingCoilName, errFlag);
                if (errFlag) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else {
                    ValidateComponent(state, HeatingCoilType, HeatingCoilName, IsNotOK, CurrentModuleObject);
                    if (IsNotOK) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;

                    } else { // mine data from heating coil object

                        // Get index to Heating Coil
                        errFlag = false;
                        HeatingCoils::GetCoilIndex(state, HeatingCoilName, state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex, errFlag);
                        if (errFlag) {
                            ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                            ErrorsFound = true;
                        }

                        // Get the furnace design capacity
                        errFlag = false;
                        state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity =
                            HeatingCoils::GetCoilCapacity(state, HeatingCoilType, HeatingCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError(state, format("...occurs in {} ={}", CurrentModuleObject, Alphas(1)));
                            ErrorsFound = true;
                        }

                        // Get the Heating Coil Inlet Node
                        errFlag = false;
                        HeatingCoilInletNode = HeatingCoils::GetCoilInletNode(state, HeatingCoilType, HeatingCoilName, errFlag);
                        state.dataFurnaces->Furnace(FurnaceNum).HWCoilAirInletNode = HeatingCoilInletNode;
                        if (errFlag) {
                            ShowContinueError(state, format("...occurs in {} ={}", CurrentModuleObject, Alphas(1)));
                            ErrorsFound = true;
                        }

                        // Get the Heating Coil Outlet Node
                        errFlag = false;
                        HeatingCoilOutletNode = HeatingCoils::GetCoilOutletNode(state, HeatingCoilType, HeatingCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError(state, format("...occurs in {} ={}", CurrentModuleObject, Alphas(1)));
                            ErrorsFound = true;
                        }

                    } // IF (IsNotOK) THEN
                }

            } else if (UtilityRoutines::SameString(HeatingCoilType, "Coil:Heating:Water")) {
                state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num = Coil_HeatingWater;
                ValidateComponent(state, HeatingCoilType, HeatingCoilName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else { // mine data from heating coil object

                    // Get the Heating Coil water Inlet or control Node number
                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).CoilControlNode =
                        GetCoilWaterInletNode(state, "Coil:Heating:Water", HeatingCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the Heating Coil hot water max volume flow rate
                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).MaxHeatCoilFluidFlow =
                        GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", HeatingCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the Heating Coil Inlet Node
                    errFlag = false;
                    HeatingCoilInletNode = WaterCoils::GetCoilInletNode(state, "Coil:Heating:Water", HeatingCoilName, errFlag);
                    state.dataFurnaces->Furnace(FurnaceNum).HWCoilAirInletNode = HeatingCoilInletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the Heating Coil Outlet Node
                    errFlag = false;
                    HeatingCoilOutletNode = WaterCoils::GetCoilOutletNode(state, "Coil:Heating:Water", HeatingCoilName, errFlag);
                    state.dataFurnaces->Furnace(FurnaceNum).HWCoilAirOutletNode = HeatingCoilOutletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // check if user has also used a water coil controller, which they should not do
                    errFlag = false;
                    CheckCoilWaterInletNode(state, state.dataFurnaces->Furnace(FurnaceNum).CoilControlNode, errFlag);
                    if (!errFlag) { // then did find a controller so that is bad
                        ShowSevereError(state,
                                        format("{} = {} has a conflicting Controller:WaterCoil object",
                                               CurrentModuleObject,
                                               state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ShowContinueError(state, "Hot water coils are controlled directly by unitary and furnace systems.");
                        ShowContinueError(state, "No water coil controller should be input for the coil.");
                        ErrorsFound = true;
                    }
                }

            } else if (UtilityRoutines::SameString(HeatingCoilType, "Coil:Heating:Steam")) {
                state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num = Coil_HeatingSteam;
                ValidateComponent(state, HeatingCoilType, HeatingCoilName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else { // mine data from heating coil object

                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex =
                        SteamCoils::GetSteamCoilIndex(state, "COIL:HEATING:STEAM", HeatingCoilName, errFlag);
                    if (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex == 0) {
                        ShowSevereError(state, format("{} illegal {} = {}", CurrentModuleObject, cAlphaFields(11), HeatingCoilName));
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the Heating Coil steam inlet node number
                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).CoilControlNode =
                        GetCoilSteamInletNode(state, "COIL:HEATING:STEAM", HeatingCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the Heating Coil steam max volume flow rate
                    state.dataFurnaces->Furnace(FurnaceNum).MaxHeatCoilFluidFlow =
                        SteamCoils::GetCoilMaxSteamFlowRate(state, state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex, errFlag);
                    if (state.dataFurnaces->Furnace(FurnaceNum).MaxHeatCoilFluidFlow > 0.0) {
                        SteamIndex = 0; // Function GetSatDensityRefrig will look up steam index if 0 is passed
                        SteamDensity =
                            GetSatDensityRefrig(state, fluidNameSteam, state.dataFurnaces->TempSteamIn, 1.0, SteamIndex, getUnitaryHeatOnly);
                        state.dataFurnaces->Furnace(FurnaceNum).MaxHeatCoilFluidFlow *= SteamDensity;
                    }

                    // Get the Heating Coil Inlet Node
                    errFlag = false;
                    HeatingCoilInletNode =
                        SteamCoils::GetCoilAirInletNode(state, state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex, HeatingCoilName, errFlag);
                    state.dataFurnaces->Furnace(FurnaceNum).HWCoilAirInletNode = HeatingCoilInletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the Heating Coil Outlet Node
                    errFlag = false;
                    HeatingCoilOutletNode =
                        GetCoilAirOutletNode(state, state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex, HeatingCoilName, errFlag);
                    state.dataFurnaces->Furnace(FurnaceNum).HWCoilAirOutletNode = HeatingCoilOutletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }
                }

            } else {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(11), Alphas(11)));
                ErrorsFound = true;
            } // IF (Furnace(FurnaceNum)%HeatingCoilType_Num == Coil_HeatingGasOrOtherFuel .OR. &, etc.

            // Add component sets array
            if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == BlowThru) {
                CompSetFanInlet = Alphas(3);
                CompSetFanOutlet = state.dataLoopNodes->NodeID(FanOutletNode);
                CompSetHeatInlet = state.dataLoopNodes->NodeID(FanOutletNode);
                CompSetHeatOutlet = Alphas(4);
                // Fan inlet node name must not be the same as the furnace inlet node name
                if (FanInletNode != state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    if (FurnaceType_Num == Furnace_HeatOnly) {
                        ShowContinueError(
                            state, "When a blow through fan is specified, the fan inlet node name must be the same as the furnace inlet node name.");
                        ShowContinueError(state, format("...Fan inlet node name     = {}", state.dataLoopNodes->NodeID(FanInletNode)));
                        ShowContinueError(state,
                                          format("...Furnace inlet node name = {}",
                                                 state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum)));
                    } else {
                        ShowContinueError(
                            state,
                            "When a blow through fan is specified, the fan inlet node name must be the same as the unitary system inlet node name.");
                        ShowContinueError(state, format("...Fan inlet node name            = {}", state.dataLoopNodes->NodeID(FanInletNode)));
                        ShowContinueError(state,
                                          format("...Unitary System inlet node name = {}",
                                                 state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum)));
                    }
                    ErrorsFound = true;
                }
                // Fan outlet node name must be the same as the heating coil inlet node name
                if (FanOutletNode != HeatingCoilInletNode) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(
                        state,
                        "When a blow through fan is specified, the fan outlet node name must be the same as the heating coil inlet node name.");
                    ShowContinueError(state, format("...Fan outlet node name         = {}", state.dataLoopNodes->NodeID(FanOutletNode)));
                    ShowContinueError(state, format("...Heating coil inlet node name = {}", state.dataLoopNodes->NodeID(HeatingCoilInletNode)));
                    ErrorsFound = true;
                }
                // Heating coil outlet node name must be the same as the furnace outlet node name
                if (HeatingCoilOutletNode != state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    if (FurnaceType_Num == Furnace_HeatOnly) {
                        ShowContinueError(state,
                                          "When a blow through fan is specified, the heating coil outlet node name must be the same as the furnace "
                                          "outlet node name.");
                        ShowContinueError(state, format("...Heating coil outlet node name = {}", state.dataLoopNodes->NodeID(HeatingCoilOutletNode)));
                        ShowContinueError(state,
                                          format("...Furnace outlet node name      = {}",
                                                 state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum)));
                    } else {
                        ShowContinueError(state,
                                          "When a blow through fan is specified, the heating coil outlet node name must be the same as the unitary "
                                          "system outlet node name.");
                        ShowContinueError(state,
                                          format("...Heating coil outlet node name  = {}", state.dataLoopNodes->NodeID(HeatingCoilOutletNode)));
                        ShowContinueError(state,
                                          format("...UnitarySystem outlet node name = {}",
                                                 state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum)));
                    }
                    ErrorsFound = true;
                }
            } else { // draw through fan
                CompSetHeatInlet = Alphas(3);
                CompSetHeatOutlet = state.dataLoopNodes->NodeID(FanInletNode);
                CompSetFanInlet = state.dataLoopNodes->NodeID(FanInletNode);
                CompSetFanOutlet = Alphas(4);
                // Heating coil inlet node name must not be the same as the furnace inlet node name
                if (HeatingCoilInletNode != state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    if (FurnaceType_Num == Furnace_HeatOnly) {
                        ShowContinueError(state,
                                          "When a draw through fan is specified, the heating coil inlet node name must be the same as the furnace "
                                          "inlet node name.");
                        ShowContinueError(state, format("...Heating coil inlet node name = {}", state.dataLoopNodes->NodeID(HeatingCoilInletNode)));
                        ShowContinueError(state,
                                          format("...Furnace inlet node name      = {}",
                                                 state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum)));
                    } else {
                        ShowContinueError(state,
                                          "When a draw through fan is specified, the heating coil inlet node name must be the same as the unitary "
                                          "system inlet node name.");
                        ShowContinueError(state, format("...Heating coil inlet node name  = {}", state.dataLoopNodes->NodeID(HeatingCoilInletNode)));
                        ShowContinueError(state,
                                          format("...UnitarySystem inlet node name = {}",
                                                 state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum)));
                    }
                    ErrorsFound = true;
                }
                // Heating coil outlet node name must be the same as the fan inlet node name
                if (HeatingCoilOutletNode != FanInletNode) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(
                        state,
                        "When a draw through fan is specified, the heating coil outlet node name must be the same as the fan inlet node name.");
                    ShowContinueError(state, format("...Heating coil outlet node name = {}", state.dataLoopNodes->NodeID(HeatingCoilOutletNode)));
                    ShowContinueError(state, format("...Fan inlet node name           = {}", state.dataLoopNodes->NodeID(FanInletNode)));
                    ErrorsFound = true;
                }
                // Fan coil outlet node name must be the same as the furnace outlet node name
                if (FanOutletNode != state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    if (FurnaceType_Num == Furnace_HeatOnly) {
                        ShowContinueError(
                            state,
                            "When a draw through fan is specified, the fan outlet node name must be the same as the furnace outlet node name.");
                        ShowContinueError(state, format("...Fan outlet node name     = {}", state.dataLoopNodes->NodeID(FanOutletNode)));
                        ShowContinueError(state,
                                          format("...Furnace outlet node name = {}",
                                                 state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum)));
                    } else {
                        ShowContinueError(state,
                                          "When a draw through fan is specified, the fan outlet node name must be the same as the unitary system "
                                          "outlet node name.");
                        ShowContinueError(state, format("...Fan outlet node name           = {}", state.dataLoopNodes->NodeID(FanOutletNode)));
                        ShowContinueError(state,
                                          format("...UnitarySystem outlet node name = {}",
                                                 state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum)));
                    }
                    ErrorsFound = true;
                }
            }

            // Add fan to component sets array
            SetUpCompSets(
                state, CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name, Alphas(7), Alphas(8), CompSetFanInlet, CompSetFanOutlet);
            // Add heating coil to component sets array
            SetUpCompSets(state,
                          CurrentModuleObject,
                          state.dataFurnaces->Furnace(FurnaceNum).Name,
                          Alphas(10),
                          Alphas(11),
                          CompSetHeatInlet,
                          CompSetHeatOutlet);

            // Set the furnace max outlet temperature
            state.dataFurnaces->Furnace(FurnaceNum).DesignMaxOutletTemp = Numbers(1);

            // Set the furnace design fan volumetric flow rate
            state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate = Numbers(2);

            // Compare the flow rates.
            if (FanVolFlowRate != DataSizing::AutoSize && state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate != DataSizing::AutoSize) {
                if (state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate > FanVolFlowRate) {
                    ShowWarningError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(
                        state, format("... The {} > Max Volume Flow Rate defined in the associated fan object, should be <=.", cNumericFields(2)));
                    ShowContinueError(state,
                                      format("... Entered value = {:.4R}... Fan [{} = {}] Max Value = {:.4R}",
                                             state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate,
                                             FanType,
                                             FanName,
                                             FanVolFlowRate));
                    ShowContinueError(state, " The HVAC system  flow rate is reset to the fan flow rate and the simulation continues.");
                    state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate = FanVolFlowRate;
                }
            }
            if (state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate != DataSizing::AutoSize) {
                if (state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate <= 0.0) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("... The {} <= 0.0, it must be > 0.0.", cNumericFields(2)));
                    ShowContinueError(state, format("... Entered value = {:.2R}", state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate));
                    ErrorsFound = true;
                }
            }

            //       HeatOnly furnace has only 1 flow rate, initialize other variables used in this module
            state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow = state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate;
            state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow = state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate;
            state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlow = state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate;
            state.dataFurnaces->Furnace(FurnaceNum).AirFlowControl = AirFlowControlConstFan::UseCompressorOnFlow;

            // Set heating convergence tolerance
            state.dataFurnaces->Furnace(FurnaceNum).HeatingConvergenceTolerance = 0.001;

            // set minimum outdoor temperature for compressor operation
            SetMinOATCompressor(state, FurnaceNum, cCurrentModuleObject, ErrorsFound);

        } // End of the HeatOnly Furnace Loop

        // Get the data for the HeatCool Furnace or UnitarySystem
        for (HeatCoolNum = 1; HeatCoolNum <= NumHeatCool + NumUnitaryHeatCool; ++HeatCoolNum) {

            FanInletNode = 0;
            FanOutletNode = 0;
            FanVolFlowRate = 0.0;
            CoolingCoilInletNode = 0;
            CoolingCoilOutletNode = 0;
            HeatingCoilInletNode = 0;
            HeatingCoilOutletNode = 0;
            ReheatCoilInletNode = 0;
            ReheatCoilOutletNode = 0;
            CoolingCoilType = ' ';
            CoolingCoilName = ' ';
            HeatingCoilType = ' ';
            HeatingCoilName = ' ';

            //      Furnace and UnitarySystem objects are both read in here.
            //      Will still have 2 differently named objects for the user, but read in with 1 DO loop.
            if (HeatCoolNum <= NumHeatCool) {
                CurrentModuleObject = "AirLoopHVAC:Unitary:Furnace:HeatCool";
                currentModuleObjectType = DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryFurnaceHeatCool;
                FurnaceType_Num = Furnace_HeatCool;
                GetObjectNum = HeatCoolNum;
            } else {
                CurrentModuleObject = "AirLoopHVAC:UnitaryHeatCool";
                currentModuleObjectType = DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatCool;
                FurnaceType_Num = UnitarySys_HeatCool;
                GetObjectNum = HeatCoolNum - NumHeatCool;
            }

            FurnaceNum = HeatCoolNum + NumHeatOnly + NumUnitaryHeatOnly;
            state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num = FurnaceType_Num;
            state.dataFurnaces->Furnace(FurnaceNum).iterationMode.allocate(3);

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     GetObjectNum,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            GlobalNames::VerifyUniqueInterObjectName(
                state, state.dataFurnaces->UniqueFurnaceNames, Alphas(1), CurrentModuleObject, cAlphaFields(1), ErrorsFound);

            state.dataFurnaces->Furnace(FurnaceNum).Name = Alphas(1);
            if (lAlphaBlanks(2)) {
                state.dataFurnaces->Furnace(FurnaceNum).SchedPtr = DataGlobalConstants::ScheduleAlwaysOn;
            } else {
                state.dataFurnaces->Furnace(FurnaceNum).SchedPtr = GetScheduleIndex(state, Alphas(2));
                if (state.dataFurnaces->Furnace(FurnaceNum).SchedPtr == 0) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(2), Alphas(2)));
                    ErrorsFound = true;
                }
            }

            state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum = GetOnlySingleNode(state,
                                                                                            Alphas(3),
                                                                                            ErrorsFound,
                                                                                            currentModuleObjectType,
                                                                                            Alphas(1),
                                                                                            DataLoopNode::NodeFluidType::Air,
                                                                                            DataLoopNode::ConnectionType::Inlet,
                                                                                            NodeInputManager::CompFluidStream::Primary,
                                                                                            ObjectIsParent);
            state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum = GetOnlySingleNode(state,
                                                                                             Alphas(4),
                                                                                             ErrorsFound,
                                                                                             currentModuleObjectType,
                                                                                             Alphas(1),
                                                                                             DataLoopNode::NodeFluidType::Air,
                                                                                             DataLoopNode::ConnectionType::Outlet,
                                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                                             ObjectIsParent);

            TestCompSet(state, CurrentModuleObject, Alphas(1), Alphas(3), Alphas(4), "Air Nodes");

            state.dataFurnaces->Furnace(FurnaceNum).FanSchedPtr = GetScheduleIndex(state, Alphas(5));
            if (!lAlphaBlanks(5) && state.dataFurnaces->Furnace(FurnaceNum).FanSchedPtr == 0) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(5), Alphas(5)));
                ErrorsFound = true;
            } else if (lAlphaBlanks(5)) {
                state.dataFurnaces->Furnace(FurnaceNum).OpMode = CycFanCycCoil;
            }

            // Get the Controlling Zone or Location of the Furnace Thermostat
            state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum = UtilityRoutines::FindItemInList(Alphas(6), state.dataHeatBal->Zone);
            if (state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum == 0) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(6), Alphas(6)));
                ErrorsFound = true;
            }

            // Get the node number for the zone with the thermostat
            if (state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum > 0) {
                AirNodeFound = false;
                AirLoopFound = false;
                int ControlledZoneNum = state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum;
                //             Find the controlled zone number for the specified thermostat location
                state.dataFurnaces->Furnace(FurnaceNum).NodeNumOfControlledZone = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ZoneNode;
                //             Determine if system is on air loop served by the thermostat location specified
                for (int zoneInNode = 1; zoneInNode <= state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).NumInletNodes; ++zoneInNode) {
                    int AirLoopNumber = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNodeAirLoopNum(zoneInNode);
                    state.dataFurnaces->Furnace(FurnaceNum).airloopNum = AirLoopNumber;
                    if (AirLoopNumber > 0) {
                        for (BranchNum = 1; BranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).NumBranches; ++BranchNum) {
                            for (CompNum = 1; CompNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).TotalComponents;
                                 ++CompNum) {
                                if (!UtilityRoutines::SameString(
                                        state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).Comp(CompNum).Name, Alphas(1)) ||
                                    !UtilityRoutines::SameString(
                                        state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).Comp(CompNum).TypeOf,
                                        CurrentModuleObject))
                                    continue;
                                AirLoopFound = true;
                                state.dataFurnaces->Furnace(FurnaceNum).ZoneInletNode =
                                    state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNode(zoneInNode);
                                break;
                            }
                            if (AirLoopFound) break;
                        }
                        for (TstatZoneNum = 1; TstatZoneNum <= state.dataZoneCtrls->NumTempControlledZones; ++TstatZoneNum) {
                            if (state.dataZoneCtrls->TempControlledZone(TstatZoneNum).ActualZoneNum !=
                                state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                                continue;
                            AirNodeFound = true;
                        }
                        for (TstatZoneNum = 1; TstatZoneNum <= state.dataZoneCtrls->NumComfortControlledZones; ++TstatZoneNum) {
                            if (state.dataZoneCtrls->ComfortControlledZone(TstatZoneNum).ActualZoneNum !=
                                state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                                continue;
                            AirNodeFound = true;
                        }
                    }
                    if (AirLoopFound) break;
                }
                if (!AirNodeFound) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, "Did not find air node (zone with thermostat).");
                    ShowContinueError(state, format("Specified {} = {}", cAlphaFields(6), Alphas(6)));
                    ShowContinueError(
                        state, "Both a ZoneHVAC:EquipmentConnections object and a ZoneControl:Thermostat object must be specified for this zone.");
                    ErrorsFound = true;
                }
                if (!AirLoopFound) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowSevereError(state, "Did not find correct AirLoopHVAC.");
                    ShowContinueError(state, format("Specified {} = {}", cAlphaFields(6), Alphas(6)));
                    ErrorsFound = true;
                }
            }

            // Get fan data
            FanType = Alphas(7);
            FanName = Alphas(8);

            errFlag = false;
            GetFanType(state, FanName, state.dataFurnaces->Furnace(FurnaceNum).FanType_Num, errFlag, CurrentModuleObject, Alphas(1));
            if (errFlag) {
                ErrorsFound = true;
            }

            if (state.dataFurnaces->Furnace(FurnaceNum).FanType_Num == FanType_SimpleOnOff ||
                state.dataFurnaces->Furnace(FurnaceNum).FanType_Num == FanType_SimpleConstVolume) {
                ValidateComponent(state, FanType, FanName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("In Furnace={}", Alphas(1)));
                    ErrorsFound = true;

                } else { // mine data from fan object

                    // Get the fan index
                    errFlag = false;
                    GetFanIndex(state, FanName, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Get the Design Fan Volume Flow Rate
                    errFlag = false;
                    FanVolFlowRate = GetFanDesignVolumeFlowRate(state, FanType, FanName, errFlag);
                    state.dataFurnaces->Furnace(FurnaceNum).ActualFanVolFlowRate = FanVolFlowRate;
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} \"{}\"", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Get the Fan Inlet Node
                    errFlag = false;
                    FanInletNode = GetFanInletNode(state, FanType, FanName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Get the Fan Outlet Node
                    errFlag = false;
                    FanOutletNode = GetFanOutletNode(state, FanType, FanName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Get the fan's availability schedule
                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).FanAvailSchedPtr = GetFanAvailSchPtr(state, FanType, FanName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Check fan's schedule for cycling fan operation if constant volume fan is used
                    if (state.dataFurnaces->Furnace(FurnaceNum).FanSchedPtr > 0 &&
                        state.dataFurnaces->Furnace(FurnaceNum).FanType_Num == FanType_SimpleConstVolume) {
                        if (!CheckScheduleValueMinMax(state, state.dataFurnaces->Furnace(FurnaceNum).FanSchedPtr, ">", 0.0, "<=", 1.0)) {
                            ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                            ShowContinueError(state, format("For {} = {}", cAlphaFields(7), Alphas(7)));
                            ShowContinueError(state, "Fan operating mode must be continuous (fan operating mode schedule values > 0).");
                            ShowContinueError(state, format("Error found in {} = {}", cAlphaFields(5), Alphas(5)));
                            ShowContinueError(state, "...schedule values must be (>0., <=1.)");
                            ErrorsFound = true;
                        }
                    } else if (lAlphaBlanks(5) && state.dataFurnaces->Furnace(FurnaceNum).FanType_Num != FanType_SimpleOnOff) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ShowContinueError(state, format("{} = {}", cAlphaFields(7), Alphas(7)));
                        ShowContinueError(state, format("Fan type must be Fan:OnOff when {} = Blank.", cAlphaFields(5)));
                        ErrorsFound = true;
                    }

                } // IF (IsNotOK) THEN

            } else {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(7), Alphas(7)));
                ErrorsFound = true;
            } //  IF (TFurnace(FurnaceNum)%FanType_Num == FanType_SimpleOnOff .OR. &, etc.

            if (UtilityRoutines::SameString(Alphas(9), "BlowThrough")) state.dataFurnaces->Furnace(FurnaceNum).FanPlace = BlowThru;
            if (UtilityRoutines::SameString(Alphas(9), "DrawThrough")) state.dataFurnaces->Furnace(FurnaceNum).FanPlace = DrawThru;
            if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == 0) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(9), Alphas(9)));
                ErrorsFound = true;
            }

            // Get coil data
            HeatingCoilType = Alphas(10);
            HeatingCoilName = Alphas(11);
            HeatingCoilPLFCurveIndex = 0;
            state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType = HeatingCoilType;
            state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilName = HeatingCoilName;
            if (UtilityRoutines::SameString(HeatingCoilType, "Coil:Heating:Fuel") ||
                UtilityRoutines::SameString(HeatingCoilType, "Coil:Heating:Electric")) {
                errFlag = false;
                state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num =
                    HeatingCoils::GetHeatingCoilTypeNum(state, HeatingCoilType, HeatingCoilName, errFlag);
                if (errFlag) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else {

                    ValidateComponent(state, HeatingCoilType, HeatingCoilName, IsNotOK, CurrentModuleObject);
                    if (IsNotOK) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;

                    } else { // mine data from heating coil

                        // Get heating coil index
                        errFlag = false;
                        HeatingCoils::GetCoilIndex(state, HeatingCoilName, state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex, errFlag);
                        if (errFlag) {
                            ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                            ErrorsFound = true;
                        }

                        // Get the design heating capacity
                        errFlag = false;
                        state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity =
                            HeatingCoils::GetCoilCapacity(state, HeatingCoilType, HeatingCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                            ErrorsFound = true;
                        }

                        // Get the Heating Coil Inlet Node
                        errFlag = false;
                        HeatingCoilInletNode = HeatingCoils::GetCoilInletNode(state, HeatingCoilType, HeatingCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                            ErrorsFound = true;
                        }

                        // Get the Heating Coil Outlet Node
                        errFlag = false;
                        HeatingCoilOutletNode = HeatingCoils::GetCoilOutletNode(state, HeatingCoilType, HeatingCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                            ErrorsFound = true;
                        }

                        // Get the Heating Coil PLF Curve Index
                        errFlag = false;
                        HeatingCoilPLFCurveIndex = GetHeatingCoilPLFCurveIndex(state, HeatingCoilType, HeatingCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                            ErrorsFound = true;
                        }

                    } // IF (IsNotOK) THEN
                }

            } else if (UtilityRoutines::SameString(HeatingCoilType, "Coil:Heating:Water")) {
                state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num = Coil_HeatingWater;
                ValidateComponent(state, HeatingCoilType, HeatingCoilName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else { // mine data from heating coil object

                    // Get the Heating Coil water Inlet or control Node number
                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).CoilControlNode =
                        GetCoilWaterInletNode(state, "Coil:Heating:Water", HeatingCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the Heating Coil hot water max volume flow rate
                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).MaxHeatCoilFluidFlow =
                        GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", HeatingCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the Heating Coil Inlet Node
                    errFlag = false;
                    HeatingCoilInletNode = WaterCoils::GetCoilInletNode(state, "Coil:Heating:Water", HeatingCoilName, errFlag);
                    state.dataFurnaces->Furnace(FurnaceNum).HWCoilAirInletNode = HeatingCoilInletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the Heating Coil Outlet Node
                    errFlag = false;
                    HeatingCoilOutletNode = WaterCoils::GetCoilOutletNode(state, "Coil:Heating:Water", HeatingCoilName, errFlag);
                    state.dataFurnaces->Furnace(FurnaceNum).HWCoilAirOutletNode = HeatingCoilOutletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // check if user has also used a water coil controller, which they should not do
                    errFlag = false;
                    CheckCoilWaterInletNode(state, state.dataFurnaces->Furnace(FurnaceNum).CoilControlNode, errFlag);
                    if (!errFlag) { // then did find a controller so that is bad
                        ShowSevereError(state,
                                        format("{} = {} has a conflicting Controller:WaterCoil object",
                                               CurrentModuleObject,
                                               state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ShowContinueError(state, "Hot water coils are controlled directly by unitary and furnace systems.");
                        ShowContinueError(state, "No water coil controller should be input for the coil.");
                        ErrorsFound = true;
                    }
                }

            } else if (UtilityRoutines::SameString(HeatingCoilType, "Coil:Heating:Steam")) {
                state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num = Coil_HeatingSteam;
                ValidateComponent(state, HeatingCoilType, HeatingCoilName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else { // mine data from heating coil object

                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex =
                        SteamCoils::GetSteamCoilIndex(state, "COIL:HEATING:STEAM", HeatingCoilName, errFlag);
                    if (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex == 0) {
                        ShowSevereError(state, format("{} illegal {} = {}", CurrentModuleObject, cAlphaFields(11), HeatingCoilName));
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the Heating Coil steam inlet node number
                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).CoilControlNode =
                        GetCoilSteamInletNode(state, "Coil:Heating:Steam", HeatingCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the Heating Coil steam max volume flow rate
                    state.dataFurnaces->Furnace(FurnaceNum).MaxHeatCoilFluidFlow =
                        SteamCoils::GetCoilMaxSteamFlowRate(state, state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex, errFlag);
                    if (state.dataFurnaces->Furnace(FurnaceNum).MaxHeatCoilFluidFlow > 0.0) {
                        SteamIndex = 0; // Function GetSatDensityRefrig will look up steam index if 0 is passed
                        SteamDensity =
                            GetSatDensityRefrig(state, fluidNameSteam, state.dataFurnaces->TempSteamIn, 1.0, SteamIndex, getAirLoopHVACHeatCoolInput);
                        state.dataFurnaces->Furnace(FurnaceNum).MaxHeatCoilFluidFlow *= SteamDensity;
                    }

                    // Get the Heating Coil Inlet Node
                    errFlag = false;
                    HeatingCoilInletNode =
                        SteamCoils::GetCoilAirInletNode(state, state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex, HeatingCoilName, errFlag);
                    state.dataFurnaces->Furnace(FurnaceNum).HWCoilAirInletNode = HeatingCoilInletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the Heating Coil Outlet Node
                    errFlag = false;
                    HeatingCoilOutletNode =
                        GetCoilAirOutletNode(state, state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex, HeatingCoilName, errFlag);
                    state.dataFurnaces->Furnace(FurnaceNum).HWCoilAirOutletNode = HeatingCoilOutletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }
                }

            } else {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(11), Alphas(11)));
                ErrorsFound = true;
            } // IF (Furnace(FurnaceNum)%HeatingCoilType_Num == Coil_HeatingGasOrOtherFuel .OR. &, etc.

            // Get Cooling Coil Information if available
            CoolingCoilType = Alphas(12);
            CoolingCoilName = Alphas(13);
            //       Find the type of coil. Do not print message since this may not be the correct coil type.
            errFlag = false;
            PrintMessage = false;

            if (UtilityRoutines::SameString(CoolingCoilType, "COIL:COOLING:DX:VARIABLESPEED") ||
                UtilityRoutines::SameString(CoolingCoilType, "COILSYSTEM:INTEGRATEDHEATPUMP:AIRSOURCE")) {
                state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num = Coil_CoolingAirToAirVariableSpeed;
                if (UtilityRoutines::SameString(CoolingCoilType, "COILSYSTEM:INTEGRATEDHEATPUMP:AIRSOURCE"))
                    state.dataFurnaces->Furnace(FurnaceNum).bIsIHP = true;
            } else {
                state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num =
                    GetCoilTypeNum(state, CoolingCoilType, CoolingCoilName, errFlag, PrintMessage);
            }

            // If coil type not found, check to see if a HX assisted cooling coil is used.
            if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == 0) {
                errFlag = false;
                state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num =
                    HVACHXAssistedCoolingCoil::GetCoilGroupTypeNum(state, CoolingCoilType, CoolingCoilName, errFlag, PrintMessage);
            }

            if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == CoilDX_CoolingSingleSpeed) {
                ValidateComponent(state, CoolingCoilType, CoolingCoilName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;

                } else { // mine data from DX cooling coil

                    // Get DX cooling coil index
                    DXCoils::GetDXCoilIndex(state, CoolingCoilName, state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex, IsNotOK);
                    if (IsNotOK) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Get DX cooling coil capacity
                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).DesignCoolingCapacity =
                        DXCoils::GetCoilCapacity(state, CoolingCoilType, CoolingCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Get the Cooling Coil Nodes
                    errFlag = false;
                    CoolingCoilInletNode = DXCoils::GetCoilInletNode(state, CoolingCoilType, CoolingCoilName, errFlag);
                    CoolingCoilOutletNode = DXCoils::GetCoilOutletNode(state, CoolingCoilType, CoolingCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Get outdoor condenser node from DX coil object
                    errFlag = false;
                    if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == Coil_CoolingAirToAirVariableSpeed) {
                        if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                            IHPCoilIndex = GetCoilIndexIHP(state, CoolingCoilType, CoolingCoilName, errFlag);
                            IHPCoilName = state.dataIntegratedHP->IntegratedHeatPumps(IHPCoilIndex).SCCoilName;
                            state.dataFurnaces->Furnace(FurnaceNum).CondenserNodeNum = GetVSCoilCondenserInletNode(state, IHPCoilName, errFlag);
                        } else {
                            state.dataFurnaces->Furnace(FurnaceNum).CondenserNodeNum = GetVSCoilCondenserInletNode(state, CoolingCoilName, errFlag);
                        }
                    } else {
                        state.dataFurnaces->Furnace(FurnaceNum).CondenserNodeNum =
                            DXCoils::GetCoilCondenserInletNode(state, CoolingCoilType, CoolingCoilName, errFlag);
                    }
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                } // IF (IsNotOK) THEN

                // Push heating coil PLF curve index to DX coil
                if (HeatingCoilPLFCurveIndex > 0) {
                    SetDXCoolingCoilData(state, state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex, ErrorsFound, HeatingCoilPLFCurveIndex);
                }

            } else if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == CoilDX_CoolingHXAssisted) {
                ValidateComponent(state, CoolingCoilType, CoolingCoilName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;

                } else { // mine data from heat exchanger assisted cooling coil

                    // Get DX heat exchanger assisted cooling coil index
                    GetHXDXCoilIndex(state, CoolingCoilName, state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex, IsNotOK);
                    if (IsNotOK) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Get DX cooling coil capacity
                    state.dataFurnaces->Furnace(FurnaceNum).DesignCoolingCapacity =
                        HVACHXAssistedCoolingCoil::GetCoilCapacity(state, CoolingCoilType, CoolingCoilName, errFlag);
                    errFlag = false;
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Get the Cooling Coil Nodes
                    errFlag = false;
                    CoolingCoilInletNode = HVACHXAssistedCoolingCoil::GetCoilInletNode(state, CoolingCoilType, CoolingCoilName, errFlag);
                    CoolingCoilOutletNode = HVACHXAssistedCoolingCoil::GetCoilOutletNode(state, CoolingCoilType, CoolingCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Get outdoor condenser node from heat exchanger assisted DX coil object
                    errFlag = false;
                    std::string ChildCoolingCoilName = HVACHXAssistedCoolingCoil::GetHXDXCoilName(state, CoolingCoilType, CoolingCoilName, IsNotOK);
                    std::string ChildCoolingCoilType = HVACHXAssistedCoolingCoil::GetHXDXCoilType(state, CoolingCoilType, CoolingCoilName, IsNotOK);
                    if (IsNotOK) {
                        ShowContinueError(state, format("Occurs in {} = {}", cCurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == CoilDX_CoolingHXAssisted) {
                    if (UtilityRoutines::SameString(ChildCoolingCoilType, "COIL:COOLING:DX")) {

                        int childCCIndex = CoilCoolingDX::factory(state, ChildCoolingCoilName);
                        if (childCCIndex < 0) {
                            ShowContinueError(state, format("Occurs in {} = {}", cCurrentModuleObject, Alphas(1)));
                            errFlag = true;
                            ErrorsFound = true;
                        }
                        auto &newCoil = state.dataCoilCooingDX->coilCoolingDXs[childCCIndex];

                        state.dataFurnaces->Furnace(FurnaceNum).CondenserNodeNum = newCoil.condInletNodeIndex;

                    }
                    // else if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == Coil_CoolingAirToAirVariableSpeed) {
                    else if (UtilityRoutines::SameString(ChildCoolingCoilType, "Coil:Cooling:DX:VariableSpeed")) {
                        if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                            IHPCoilIndex = GetCoilIndexIHP(state, CoolingCoilType, CoolingCoilName, errFlag);
                            IHPCoilName = state.dataIntegratedHP->IntegratedHeatPumps(IHPCoilIndex).SCCoilName;
                            state.dataFurnaces->Furnace(FurnaceNum).CondenserNodeNum = GetVSCoilCondenserInletNode(state, IHPCoilName, errFlag);
                        } else {
                            state.dataFurnaces->Furnace(FurnaceNum).CondenserNodeNum = GetVSCoilCondenserInletNode(state, CoolingCoilName, errFlag);
                        }
                    } else {
                        state.dataFurnaces->Furnace(FurnaceNum).CondenserNodeNum = DXCoils::GetCoilCondenserInletNode(
                            state, "COIL:COOLING:DX:SINGLESPEED", GetHXDXCoilName(state, CoolingCoilType, CoolingCoilName, errFlag), errFlag);
                    }

                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Push heating coil PLF curve index to DX coil
                    if (HeatingCoilPLFCurveIndex > 0) {
                        // get the actual index to the DX cooling coil object
                        DXCoilIndex = GetActualDXCoilIndex(state, CoolingCoilType, CoolingCoilName, ErrorsFound);
                        state.dataFurnaces->Furnace(FurnaceNum).ActualDXCoilIndexForHXAssisted = DXCoilIndex;
                        int ActualCoolCoilType =
                            HVACHXAssistedCoolingCoil::GetCoilObjectTypeNum(state, CoolingCoilType, CoolingCoilName, errFlag, true);
                        if (ActualCoolCoilType == DataHVACGlobals::CoilDX_CoolingSingleSpeed) {
                            SetDXCoolingCoilData(state, DXCoilIndex, ErrorsFound, HeatingCoilPLFCurveIndex);
                        }
                        // what could we do for VS coil here? odd thing here
                    }

                } // IF (IsNotOK) THEN
            } else if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == Coil_CoolingAirToAirVariableSpeed) {
                // BOS ADDED, AUG/2012, VARIIABLE SPEED DX COOLING COIL
                //  Furnace(FurnaceNum)%DXCoolCoilType = 'COIL:COOLING:DX:VARIABLESPEED'
                //  Furnace(FurnaceNum)%DXCoolCoilName = CoolingCoilName
                if (UtilityRoutines::SameString(CoolingCoilType, "COILSYSTEM:INTEGRATEDHEATPUMP:AIRSOURCE"))
                    state.dataFurnaces->Furnace(FurnaceNum).bIsIHP = true;
                ValidateComponent(state, CoolingCoilType, CoolingCoilName, IsNotOK, CurrentModuleObject);

                if (IsNotOK) {
                    ShowContinueError(state, format("...specified in {}=\"{}\".", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else {
                    errFlag = false;
                    if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                        state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex = GetCoilIndexIHP(state, CoolingCoilType, CoolingCoilName, errFlag);
                        IHPCoilName =
                            state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).SCCoilName;
                    } else {
                        state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex =
                            GetCoilIndexVariableSpeed(state, CoolingCoilType, CoolingCoilName, errFlag);
                        IHPCoilName = CoolingCoilName;
                    }

                    if (errFlag) {
                        ShowContinueError(state, format("...specified in {}=\"{}\".", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                        CoolingCoilInletNode = GetCoilInletNodeVariableSpeed(state, "COIL:COOLING:DX:VARIABLESPEED", IHPCoilName, errFlag);
                        CoolingCoilOutletNode = GetCoilOutletNodeVariableSpeed(state, "COIL:COOLING:DX:VARIABLESPEED", IHPCoilName, errFlag);
                        state.dataFurnaces->Furnace(FurnaceNum).CondenserNodeNum = GetVSCoilCondenserInletNode(state, IHPCoilName, errFlag);
                    } else {
                        CoolingCoilInletNode = GetCoilInletNodeVariableSpeed(state, CoolingCoilType, CoolingCoilName, errFlag);
                        CoolingCoilOutletNode = GetCoilOutletNodeVariableSpeed(state, CoolingCoilType, CoolingCoilName, errFlag);
                        state.dataFurnaces->Furnace(FurnaceNum).CondenserNodeNum = GetVSCoilCondenserInletNode(state, CoolingCoilName, errFlag);
                    }

                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }
                }
            } else {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(12), Alphas(12)));
                ErrorsFound = true;
            }

            if (UtilityRoutines::SameString(Alphas(14), "None") || UtilityRoutines::SameString(Alphas(14), "Multimode") ||
                UtilityRoutines::SameString(Alphas(14), "CoolReheat")) {
                AirNodeFound = false;
                if (UtilityRoutines::SameString(Alphas(14), "Multimode")) {
                    state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num = DehumidificationControlMode::Multimode;
                    state.dataFurnaces->Furnace(FurnaceNum).Humidistat = true;
                    if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num != CoilDX_CoolingHXAssisted) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(14), Alphas(14)));
                        ShowContinueError(state, "Multimode control must be used with a Heat Exchanger Assisted Cooling Coil.");
                        if (lAlphaBlanks(15)) {
                            ShowContinueError(state,
                                              "Dehumidification control type is assumed to be None since a reheat coil has not been specified and "
                                              "the simulation continues.");
                            state.dataFurnaces->Furnace(FurnaceNum).Humidistat = false;
                            state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num = DehumidificationControlMode::None;
                        } else {
                            ShowContinueError(state, "Dehumidification control type is assumed to be CoolReheat and the simulation continues.");
                            state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num = DehumidificationControlMode::CoolReheat;
                        }
                    }
                }
                if (UtilityRoutines::SameString(Alphas(14), "CoolReheat")) {
                    state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num = DehumidificationControlMode::CoolReheat;
                    state.dataFurnaces->Furnace(FurnaceNum).Humidistat = true;
                    if (lAlphaBlanks(15)) {
                        ShowWarningError(state, format("{} \"{}\"", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state,
                                          "Dehumidification control type is assumed to be None since a reheat coil has not been specified and the "
                                          "simulation continues.");
                        state.dataFurnaces->Furnace(FurnaceNum).Humidistat = false;
                        state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num = DehumidificationControlMode::None;
                    }
                }
                if (UtilityRoutines::SameString(Alphas(14), "None")) {
                    state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num = DehumidificationControlMode::None;
                    state.dataFurnaces->Furnace(FurnaceNum).Humidistat = false;
                }
                if (state.dataFurnaces->Furnace(FurnaceNum).Humidistat) {
                    for (HStatZoneNum = 1; HStatZoneNum <= state.dataZoneCtrls->NumHumidityControlZones; ++HStatZoneNum) {
                        if (state.dataZoneCtrls->HumidityControlZone(HStatZoneNum).ActualZoneNum !=
                            state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                            continue;
                        AirNodeFound = true;
                    }
                    if (!AirNodeFound) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state, "Did not find Air Node (Zone with Humidistat).");
                        ShowContinueError(state, format("Specified {} = {}", cAlphaFields(6), Alphas(6)));
                        ErrorsFound = true;
                    }
                }
            } else { // invalid input
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(14), Alphas(14)));
                state.dataFurnaces->Furnace(FurnaceNum).Humidistat = false;
                ErrorsFound = true;
            }

            //       Check placement of cooling coil with respect to fan placement and dehumidification control type
            if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == BlowThru) {
                if (FanOutletNode == HeatingCoilInletNode &&
                    state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num != DehumidificationControlMode::CoolReheat) {
                    state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilUpstream = false;
                }
            } else {
                if (HeatingCoilOutletNode == CoolingCoilInletNode &&
                    state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num != DehumidificationControlMode::CoolReheat) {
                    state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilUpstream = false;
                }
            }

            // Get reheat coil data if humidistat is used
            ReheatingCoilType = Alphas(15);
            ReheatingCoilName = Alphas(16);
            state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType = ReheatingCoilType;
            state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilName = ReheatingCoilName;
            errFlag = false;
            if (!lAlphaBlanks(15)) {
                if (UtilityRoutines::SameString(ReheatingCoilType, "Coil:Heating:Fuel") ||
                    UtilityRoutines::SameString(ReheatingCoilType, "Coil:Heating:Electric") ||
                    UtilityRoutines::SameString(ReheatingCoilType, "Coil:Heating:Desuperheater")) {

                    state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType_Num =
                        HeatingCoils::GetHeatingCoilTypeNum(state, ReheatingCoilType, ReheatingCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    } else {

                        ValidateComponent(state, ReheatingCoilType, ReheatingCoilName, IsNotOK, CurrentModuleObject);
                        if (IsNotOK) {
                            ShowContinueError(state, format("In {} \"{}\"", CurrentModuleObject, Alphas(1)));
                            ErrorsFound = true;

                        } else { // mine data from reheat coil

                            // Get the heating coil index
                            HeatingCoils::GetCoilIndex(state, ReheatingCoilName, state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex, IsNotOK);
                            if (IsNotOK) {
                                ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                                ErrorsFound = true;
                            }

                            // Get the design supplemental heating capacity
                            errFlag = false;
                            state.dataFurnaces->Furnace(FurnaceNum).DesignSuppHeatingCapacity =
                                HeatingCoils::GetCoilCapacity(state, ReheatingCoilType, ReheatingCoilName, errFlag);
                            if (errFlag) {
                                ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                                ErrorsFound = true;
                            }

                            // Get the Reheat Coil Inlet Node
                            errFlag = false;
                            ReheatCoilInletNode = HeatingCoils::GetCoilInletNode(state, ReheatingCoilType, ReheatingCoilName, errFlag);
                            if (errFlag) {
                                ShowContinueError(state, format("...occurs in {} \"{}\"", CurrentModuleObject, Alphas(1)));
                                ErrorsFound = true;
                            }

                            // Get the Reheat Coil Outlet Node
                            errFlag = false;
                            ReheatCoilOutletNode = HeatingCoils::GetCoilOutletNode(state, ReheatingCoilType, ReheatingCoilName, errFlag);
                            if (errFlag) {
                                ShowContinueError(state, format("...occurs in {} \"{}\"", CurrentModuleObject, Alphas(1)));
                                ErrorsFound = true;
                            }

                        } // IF (IsNotOK) THEN
                    }

                } else if (UtilityRoutines::SameString(ReheatingCoilType, "Coil:Heating:Water")) {
                    state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType_Num = Coil_HeatingWater;
                    ValidateComponent(state, ReheatingCoilType, ReheatingCoilName, IsNotOK, CurrentModuleObject);
                    if (IsNotOK) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    } else { // mine data from heating coil object

                        // Get the Heating Coil water Inlet or control Node number
                        errFlag = false;
                        state.dataFurnaces->Furnace(FurnaceNum).SuppCoilControlNode =
                            GetCoilWaterInletNode(state, "Coil:Heating:Water", ReheatingCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                            ErrorsFound = true;
                        }

                        // Get the ReHeat Coil hot water max volume flow rate
                        errFlag = false;
                        state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow =
                            GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", ReheatingCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                            ErrorsFound = true;
                        }

                        // Get the ReHeat Coil Inlet Node
                        errFlag = false;
                        ReheatCoilInletNode = WaterCoils::GetCoilInletNode(state, "Coil:Heating:Water", ReheatingCoilName, errFlag);
                        state.dataFurnaces->Furnace(FurnaceNum).SuppCoilAirInletNode = ReheatCoilInletNode;
                        if (errFlag) {
                            ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                            ErrorsFound = true;
                        }

                        // Get the ReHeat Coil Outlet Node
                        errFlag = false;
                        ReheatCoilOutletNode = WaterCoils::GetCoilOutletNode(state, "Coil:Heating:Water", ReheatingCoilName, errFlag);
                        state.dataFurnaces->Furnace(FurnaceNum).SuppCoilAirOutletNode = ReheatCoilOutletNode;
                        if (errFlag) {
                            ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                            ErrorsFound = true;
                        }

                        // check if user has also used a water coil controller, which they should not do
                        errFlag = false;
                        CheckCoilWaterInletNode(state, state.dataFurnaces->Furnace(FurnaceNum).CoilControlNode, errFlag);
                        if (!errFlag) { // then did find a controller so that is bad
                            ShowSevereError(state,
                                            format("{} = {} has a conflicting Controller:WaterCoil object",
                                                   CurrentModuleObject,
                                                   state.dataFurnaces->Furnace(FurnaceNum).Name));
                            ShowContinueError(state, "Hot water coils are controlled directly by unitary and furnace systems.");
                            ShowContinueError(state, "No water coil controller should be input for the coil.");
                            ErrorsFound = true;
                        }
                    }

                } else if (UtilityRoutines::SameString(ReheatingCoilType, "Coil:Heating:Steam")) {
                    state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType_Num = Coil_HeatingSteam;
                    ValidateComponent(state, ReheatingCoilType, ReheatingCoilName, IsNotOK, CurrentModuleObject);
                    if (IsNotOK) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    } else { // mine data from heating coil object

                        errFlag = false;
                        state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex =
                            SteamCoils::GetSteamCoilIndex(state, "COIL:HEATING:STEAM", ReheatingCoilName, errFlag);
                        if (state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex == 0) {
                            ShowSevereError(state, format("{} illegal {} = {}", CurrentModuleObject, cAlphaFields(11), ReheatingCoilName));
                            ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                            ErrorsFound = true;
                        }

                        // Get the Heating Coil steam inlet node number
                        errFlag = false;
                        state.dataFurnaces->Furnace(FurnaceNum).SuppCoilControlNode =
                            GetCoilSteamInletNode(state, "Coil:Heating:Steam", ReheatingCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                            ErrorsFound = true;
                        }

                        // Get the Heating Coil steam max volume flow rate
                        state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow =
                            SteamCoils::GetCoilMaxSteamFlowRate(state, state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex, errFlag);
                        if (state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow > 0.0) {
                            SteamIndex = 0; // Function GetSatDensityRefrig will look up steam index if 0 is passed
                            SteamDensity = GetSatDensityRefrig(
                                state, fluidNameSteam, state.dataFurnaces->TempSteamIn, 1.0, SteamIndex, getAirLoopHVACHeatCoolInput);
                            state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow =
                                SteamCoils::GetCoilMaxSteamFlowRate(state, state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex, errFlag) *
                                SteamDensity;
                        }

                        // Get the Heating Coil Inlet Node
                        errFlag = false;
                        ReheatCoilInletNode = SteamCoils::GetCoilAirInletNode(
                            state, state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex, ReheatingCoilName, errFlag);
                        state.dataFurnaces->Furnace(FurnaceNum).SuppCoilAirInletNode = ReheatCoilInletNode;
                        if (errFlag) {
                            ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                            ErrorsFound = true;
                        }

                        // Get the Heating Coil Outlet Node
                        errFlag = false;
                        ReheatCoilOutletNode =
                            GetCoilAirOutletNode(state, state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex, ReheatingCoilName, errFlag);
                        state.dataFurnaces->Furnace(FurnaceNum).SuppCoilAirOutletNode = ReheatCoilOutletNode;
                        if (errFlag) {
                            ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                            ErrorsFound = true;
                        }
                    }

                } else { // Illeagal heating coil
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(15), Alphas(15)));
                    ErrorsFound = true;
                } // IF (Furnace(FurnaceNum)%SuppHeatCoilType_Num == Coil_HeatingGasOrOtherFuel .OR. &, etc.

            } // IF(.NOT. lAlphaBlanks(15))THEN

            if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == BlowThru) {

                if (FanInletNode != state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum) {
                    ShowSevereError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                    if (FurnaceType_Num == Furnace_HeatCool) {
                        ShowContinueError(
                            state, "When a blow through fan is specified, the fan inlet node name must be the same as the furnace inlet node name.");
                        ShowContinueError(state, format("...Fan inlet node name     = {}", state.dataLoopNodes->NodeID(FanInletNode)));
                        ShowContinueError(state,
                                          format("...Furnace inlet node name = {}",
                                                 state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum)));
                    } else {
                        ShowContinueError(
                            state,
                            "When a blow through fan is specified, the fan inlet node name must be the same as the unitary system inlet node name.");
                        ShowContinueError(state, format("...Fan inlet node name           = {}", state.dataLoopNodes->NodeID(FanInletNode)));
                        ShowContinueError(state,
                                          format("...UnitarySystem inlet node name = {}",
                                                 state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum)));
                    }
                    ErrorsFound = true;
                }
                if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilUpstream) {
                    if (FanOutletNode != CoolingCoilInletNode) {
                        ShowSevereError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(
                            state,
                            "When a blow through fan is specified, the fan outlet node name must be the same as the cooling coil inlet node name.");
                        ShowContinueError(state, format("...Fan outlet node name         = {}", state.dataLoopNodes->NodeID(FanOutletNode)));
                        ShowContinueError(state, format("...Cooling coil inlet node name = {}", state.dataLoopNodes->NodeID(CoolingCoilInletNode)));
                        ErrorsFound = true;
                    }
                    if (CoolingCoilOutletNode != HeatingCoilInletNode) {
                        ShowSevereError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state, "The cooling coil outlet node name must be the same as the heating coil inlet node name.");
                        ShowContinueError(state, format("...Cooling coil outlet node name = {}", state.dataLoopNodes->NodeID(CoolingCoilOutletNode)));
                        ShowContinueError(state, format("...Heating coil inlet node name  = {}", state.dataLoopNodes->NodeID(HeatingCoilInletNode)));
                        ErrorsFound = true;
                    }
                    if ((state.dataFurnaces->Furnace(FurnaceNum).Humidistat &&
                         state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::CoolReheat) ||
                        ReheatCoilInletNode > 0) {
                        if (HeatingCoilOutletNode != ReheatCoilInletNode) {
                            ShowSevereError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                            ShowContinueError(state,
                                              "When a blow through fan is specified, the heating coil outlet node name must be the same as the "
                                              "reheat coil inlet node name.");
                            ShowContinueError(state,
                                              format("...Heating coil outlet node name = {}", state.dataLoopNodes->NodeID(HeatingCoilOutletNode)));
                            ShowContinueError(state,
                                              format("...Reheat coil inlet node name   = {}", state.dataLoopNodes->NodeID(ReheatCoilInletNode)));
                            ErrorsFound = true;
                        }
                        if (ReheatCoilOutletNode != state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum) {
                            ShowSevereError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                            if (FurnaceType_Num == Furnace_HeatCool) {
                                ShowContinueError(state, "The reheat coil outlet node name must be the same as the furnace outlet node name.");
                                ShowContinueError(state,
                                                  format("...Reheat coil outlet node name = {}", state.dataLoopNodes->NodeID(ReheatCoilOutletNode)));
                                ShowContinueError(state,
                                                  format("...Furnace outlet node name     = {}",
                                                         state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum)));
                            } else {
                                ShowContinueError(state, "The reheat coil outlet node name must be the same as the unitary system outlet node name.");
                                ShowContinueError(
                                    state, format("...Reheat coil outlet node name   = {}", state.dataLoopNodes->NodeID(ReheatCoilOutletNode)));
                                ShowContinueError(state,
                                                  format("...UnitarySystem outlet node name = {}",
                                                         state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum)));
                            }
                            ErrorsFound = true;
                        }
                    } else { // IF((Furnace(FurnaceNum)%Humidistat ...
                        // Heating coil outlet node name must be the same as the furnace outlet node name
                        if (HeatingCoilOutletNode != state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum) {
                            ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                            if (FurnaceType_Num == Furnace_HeatOnly) {
                                ShowContinueError(state,
                                                  "When a blow through fan is specified, the heating coil outlet node name must be the same as the "
                                                  "furnace outlet node name.");
                                ShowContinueError(
                                    state, format("...Heating coil outlet node name = {}", state.dataLoopNodes->NodeID(HeatingCoilOutletNode)));
                                ShowContinueError(state,
                                                  format("...Furnace outlet node name      = {}",
                                                         state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum)));
                            } else {
                                ShowContinueError(state,
                                                  "When a blow through fan is specified, the heating coil outlet node name must be the same as the "
                                                  "unitary system outlet node name.");
                                ShowContinueError(
                                    state, format("...Heating coil outlet node name  = {}", state.dataLoopNodes->NodeID(HeatingCoilOutletNode)));
                                ShowContinueError(state,
                                                  format("...UnitarySystem outlet node name = {}",
                                                         state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum)));
                            }
                            ErrorsFound = true;
                        }
                    }
                } else { // IF(Furnace(FurnaceNum)%CoolingCoilUpstream)THEN
                    if (FanOutletNode != HeatingCoilInletNode) {
                        ShowSevereError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(
                            state,
                            "When a blow through fan is specified, the fan outlet node name must be the same as the heating coil inlet node name.");
                        ShowContinueError(state, format("...Fan outlet node name         = {}", state.dataLoopNodes->NodeID(FanOutletNode)));
                        ShowContinueError(state, format("...Heating coil inlet node name = {}", state.dataLoopNodes->NodeID(HeatingCoilInletNode)));
                        ErrorsFound = true;
                    }
                    if (HeatingCoilOutletNode != CoolingCoilInletNode) {
                        ShowSevereError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state, "The heating coil outlet node name must be the same as the cooling coil inlet node name.");
                        ShowContinueError(state, format("...Heating coil outlet node name = {}", state.dataLoopNodes->NodeID(HeatingCoilOutletNode)));
                        ShowContinueError(state, format("...Cooling coil inlet node name  = {}", state.dataLoopNodes->NodeID(CoolingCoilInletNode)));
                        ErrorsFound = true;
                    }
                    if (CoolingCoilOutletNode != state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum) {
                        ShowSevereError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                        if (FurnaceType_Num == Furnace_HeatCool) {
                            ShowContinueError(state,
                                              "When a blow through fan is specified, the cooling coil outlet node name must be the same as the "
                                              "furnace outlet node name.");
                            ShowContinueError(state,
                                              format("...Cooling coil outlet node name = {}", state.dataLoopNodes->NodeID(CoolingCoilOutletNode)));
                            ShowContinueError(state,
                                              format("...Furnace outlet node name      = {}",
                                                     state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum)));
                        } else {
                            ShowContinueError(state,
                                              "When a blow through fan is specified, the cooling coil outlet node name must be the same as the "
                                              "unitary system outlet node name.");
                            ShowContinueError(state,
                                              format("...Cooling coil outlet node name   = {}", state.dataLoopNodes->NodeID(CoolingCoilOutletNode)));
                            ShowContinueError(state,
                                              format("...UnitarySystem outlet node name  = {}",
                                                     state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum)));
                        }
                        ErrorsFound = true;
                    }
                }

            } else { // ELSE from IF(Furnace(FurnaceNum)%FanPlace .EQ. BlowThru)THEN

                if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilUpstream) {
                    if (CoolingCoilInletNode != state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum) {
                        ShowSevereError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                        if (FurnaceType_Num == Furnace_HeatCool) {
                            ShowContinueError(state,
                                              "When a draw through fan is specified, the cooling coil inlet node name must be the same as the "
                                              "furnace inlet node name.");
                            ShowContinueError(state,
                                              format("...Cooling coil inlet node name = {}", state.dataLoopNodes->NodeID(CoolingCoilInletNode)));
                            ShowContinueError(state,
                                              format("...Furnace inlet node name      = {}",
                                                     state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum)));
                        } else {
                            ShowContinueError(state,
                                              "When a draw through fan is specified, the cooling coil inlet node name must be the same as the "
                                              "unitary system inlet node name.");
                            ShowContinueError(state,
                                              format("...Cooling coil inlet node name  = {}", state.dataLoopNodes->NodeID(CoolingCoilInletNode)));
                            ShowContinueError(state,
                                              format("...UnitarySystem inlet node name = {}",
                                                     state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum)));
                        }
                        ErrorsFound = true;
                    }
                    if (CoolingCoilOutletNode != HeatingCoilInletNode) {
                        ShowSevereError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state, "The cooling coil outlet node name must be the same as the heating coil inlet node name.");
                        ShowContinueError(state, format("...Cooling coil outlet node name = {}", state.dataLoopNodes->NodeID(CoolingCoilOutletNode)));
                        ShowContinueError(state, format("...Heating coil inlet node name  = {}", state.dataLoopNodes->NodeID(HeatingCoilInletNode)));
                        ErrorsFound = true;
                    }
                    if (HeatingCoilOutletNode != FanInletNode) {
                        ShowSevereError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(
                            state,
                            "When a draw through fan is specified, the heating coil outlet node name must be the same as the fan inlet node name.");
                        ShowContinueError(state, format("...Heating coil outlet node name = {}", state.dataLoopNodes->NodeID(HeatingCoilOutletNode)));
                        ShowContinueError(state, format("...Fan inlet node name           = {}", state.dataLoopNodes->NodeID(FanInletNode)));
                        ErrorsFound = true;
                    }
                    if ((state.dataFurnaces->Furnace(FurnaceNum).Humidistat &&
                         state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::CoolReheat) ||
                        ReheatCoilInletNode > 0) {
                        if (FanOutletNode != ReheatCoilInletNode) {
                            ShowSevereError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                            ShowContinueError(state,
                                              "When a draw through fan is specified, the fan outlet node name must be the same as the reheat coil "
                                              "inlet node name.");
                            ShowContinueError(state, format("...Fan outlet node name        = {}", state.dataLoopNodes->NodeID(FanOutletNode)));
                            ShowContinueError(state, format("...Reheat coil inlet node name = {}", state.dataLoopNodes->NodeID(ReheatCoilInletNode)));
                            ErrorsFound = true;
                        }
                        if (ReheatCoilOutletNode != state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum) {
                            ShowSevereError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                            if (FurnaceType_Num == Furnace_HeatCool) {
                                ShowContinueError(state, "The reheat coil outlet node name must be the same as the furnace outlet node name.");
                                ShowContinueError(state,
                                                  format("...Reheat coil outlet node name = {}", state.dataLoopNodes->NodeID(ReheatCoilOutletNode)));
                                ShowContinueError(state,
                                                  format("...Furnace outlet node name     = {}",
                                                         state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum)));
                            } else {
                                ShowContinueError(state, "The reheat coil outlet node name must be the same as the unitary system outlet node name.");
                                ShowContinueError(
                                    state, format("...Reheat coil outlet node name   = {}", state.dataLoopNodes->NodeID(ReheatCoilOutletNode)));
                                ShowContinueError(state,
                                                  format("...UnitarySystem outlet node name = {}",
                                                         state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum)));
                            }
                            ErrorsFound = true;
                        }
                    } else {
                        if (FanOutletNode != state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum) {
                            ShowSevereError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                            ShowContinueError(state,
                                              "When a draw through fan is specified, the fan outlet node name must be the same as the unitary system "
                                              "outlet node name.");
                            ShowContinueError(state, format("...Fan outlet node name        = {}", state.dataLoopNodes->NodeID(FanOutletNode)));
                            ShowContinueError(state,
                                              format("...Unitary system outlet node name = {}",
                                                     state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum)));
                            ErrorsFound = true;
                        }
                    }
                } else { // IF(Furnace(FurnaceNum)%CoolingCoilUpstream)THEN
                    if (HeatingCoilInletNode != state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum) {
                        ShowSevereError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                        if (FurnaceType_Num == Furnace_HeatCool) {
                            ShowContinueError(state,
                                              "When a draw through fan is specified, the heating coil inlet node name must be the same as the "
                                              "furnace inlet node name.");
                            ShowContinueError(state,
                                              format("...Heating coil inlet node name = {}", state.dataLoopNodes->NodeID(HeatingCoilInletNode)));
                            ShowContinueError(state,
                                              format("...Furnace inlet node name      = {}",
                                                     state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum)));
                        } else {
                            ShowContinueError(state,
                                              "When a draw through fan is specified, the heating coil inlet node name must be the same as the "
                                              "unitary system inlet node name.");
                            ShowContinueError(state,
                                              format("...Heating coil inlet node name  = {}", state.dataLoopNodes->NodeID(HeatingCoilInletNode)));
                            ShowContinueError(state,
                                              format("...UnitarySystem inlet node name = {}",
                                                     state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum)));
                        }
                        ErrorsFound = true;
                    }
                    if (HeatingCoilOutletNode != CoolingCoilInletNode) {
                        ShowSevereError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state, "The heating coil outlet node name must be the same as the cooling coil inlet node name.");
                        ShowContinueError(state, format("...Heating coil outlet node name = {}", state.dataLoopNodes->NodeID(HeatingCoilOutletNode)));
                        ShowContinueError(state, format("...Cooling coil inlet node name  = {}", state.dataLoopNodes->NodeID(CoolingCoilInletNode)));
                        ErrorsFound = true;
                    }
                    if (CoolingCoilOutletNode != FanInletNode) {
                        ShowSevereError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(
                            state,
                            "When a draw through fan is specified, the cooling coil outlet node name must be the same as the fan inlet node name.");
                        ShowContinueError(state, format("...Cooling coil outlet node name = {}", state.dataLoopNodes->NodeID(CoolingCoilOutletNode)));
                        ShowContinueError(state, format("...Fan inlet node name           = {}", state.dataLoopNodes->NodeID(FanInletNode)));
                        ErrorsFound = true;
                    }
                    if (FanOutletNode != state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum) {
                        ShowSevereError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                        if (FurnaceType_Num == Furnace_HeatCool) {
                            ShowContinueError(
                                state,
                                "When a draw through fan is specified, the fan outlet node name must be the same as the furnace outlet node name.");
                            ShowContinueError(state, format("...Fan outlet node name     = {}", state.dataLoopNodes->NodeID(FanOutletNode)));
                            ShowContinueError(state,
                                              format("...Furnace outlet node name = {}",
                                                     state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum)));
                        } else {
                            ShowContinueError(state,
                                              "When a draw through fan is specified, the fan outlet node name must be the same as the unitary system "
                                              "outlet node name.");
                            ShowContinueError(state, format("...Fan outlet node name           = {}", state.dataLoopNodes->NodeID(FanOutletNode)));
                            ShowContinueError(state,
                                              format("...UnitarySystem outlet node name = {}",
                                                     state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum)));
                        }
                        ErrorsFound = true;
                    }
                }
            } // ELSE from IF(Furnace(FurnaceNum)%FanPlace .EQ. BlowThru)THEN

            // Add fan to component sets array
            SetUpCompSets(state,
                          CurrentModuleObject,
                          Alphas(1),
                          Alphas(7),
                          Alphas(8),
                          state.dataLoopNodes->NodeID(FanInletNode),
                          state.dataLoopNodes->NodeID(FanOutletNode));

            // Add DX cooling coil to component sets array
            if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                SetUpCompSets(state,
                              CurrentModuleObject,
                              Alphas(1),
                              Alphas(12),
                              Alphas(13) + " Cooling Coil",
                              state.dataLoopNodes->NodeID(CoolingCoilInletNode),
                              state.dataLoopNodes->NodeID(CoolingCoilOutletNode));
            } else {
                SetUpCompSets(state,
                              CurrentModuleObject,
                              Alphas(1),
                              Alphas(12),
                              Alphas(13),
                              state.dataLoopNodes->NodeID(CoolingCoilInletNode),
                              state.dataLoopNodes->NodeID(CoolingCoilOutletNode));
            }

            // Add heating coil to component sets array
            if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                SetUpCompSets(state,
                              CurrentModuleObject,
                              Alphas(1),
                              Alphas(10),
                              Alphas(11) + " Heating Coil",
                              state.dataLoopNodes->NodeID(HeatingCoilInletNode),
                              state.dataLoopNodes->NodeID(HeatingCoilOutletNode));
            } else {
                SetUpCompSets(state,
                              CurrentModuleObject,
                              Alphas(1),
                              Alphas(10),
                              Alphas(11),
                              state.dataLoopNodes->NodeID(HeatingCoilInletNode),
                              state.dataLoopNodes->NodeID(HeatingCoilOutletNode));
            }

            if (ReheatCoilInletNode > 0) {

                // Add reheating coil to component sets array
                SetUpCompSets(state,
                              CurrentModuleObject,
                              Alphas(1),
                              Alphas(15),
                              Alphas(16),
                              state.dataLoopNodes->NodeID(ReheatCoilInletNode),
                              state.dataLoopNodes->NodeID(ReheatCoilOutletNode));
            }

            // Set the furnace max outlet temperature
            state.dataFurnaces->Furnace(FurnaceNum).DesignMaxOutletTemp = Numbers(1);

            state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow = Numbers(2);
            if (state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow <= 0 &&
                state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {:.7T}", cNumericFields(2), Numbers(2)));
                ErrorsFound = true;
            }

            state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow = Numbers(3);
            if (state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow <= 0 &&
                state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {:.7T}", cNumericFields(3), Numbers(3)));
                ErrorsFound = true;
            }

            state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlow = Numbers(4);
            if (state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlow < 0 &&
                state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {:.7T}", cNumericFields(4), Numbers(4)));
                ErrorsFound = true;
            }

            if (Numbers(2) != DataSizing::AutoSize && Numbers(3) != DataSizing::AutoSize && Numbers(4) != DataSizing::AutoSize) {
                state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate = max(Numbers(2), Numbers(3), Numbers(4));
            } else {
                state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate = DataSizing::AutoSize;
            }

            if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == Coil_CoolingAirToAirVariableSpeed) {
                errFlag = false;
                if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                    state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex = GetCoilIndexIHP(state, CoolingCoilType, CoolingCoilName, errFlag);
                    IHPCoilName = state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).SCCoilName;
                    state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow =
                        GetCoilAirFlowRateVariableSpeed(state, "COIL:COOLING:DX:VARIABLESPEED", IHPCoilName, errFlag);
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow =
                        GetCoilAirFlowRateVariableSpeed(state, CoolingCoilType, CoolingCoilName, errFlag);
                }

                if (errFlag) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                }

                state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlow =
                    min(state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow, state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow);
                if (state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow != DataSizing::AutoSize &&
                    state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow != DataSizing::AutoSize) {
                    state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate =
                        max(state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow, state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow);
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate = DataSizing::AutoSize;
                }
            }

            if (FanVolFlowRate != DataSizing::AutoSize) {
                if (FanVolFlowRate < state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow &&
                    state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow != DataSizing::AutoSize) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(
                        state,
                        format("... air flow rate = {:.7T} in fan object {} is less than the maximum HVAC system air flow rate in cooling mode.",
                               FanVolFlowRate,
                               FanName));
                    ShowContinueError(state, format(" The {} is reset to the fan flow rate and the simulation continues.", cNumericFields(2)));
                    state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow = FanVolFlowRate;
                    state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate = FanVolFlowRate;
                }
                if (FanVolFlowRate < state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow &&
                    state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow != DataSizing::AutoSize) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(
                        state,
                        format("... air flow rate = {:.7T} in fan object {} is less than the maximum HVAC system air flow rate in heating mode.",
                               FanVolFlowRate,
                               FanName));
                    ShowContinueError(state, format(" The {} is reset to the fan flow rate and the simulation continues.", cNumericFields(3)));
                    state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow = FanVolFlowRate;
                    state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate = FanVolFlowRate;
                }
            }

            if (state.dataFurnaces->Furnace(FurnaceNum).FanSchedPtr > 0) {
                if (!CheckScheduleValueMinMax(state, state.dataFurnaces->Furnace(FurnaceNum).FanSchedPtr, ">=", 0.0, "<=", 0.0)) {
                    //           set air flow control mode:
                    //             UseCompressorOnFlow = operate at last cooling or heating air flow requested when compressor is off
                    //             UseCompressorOffFlow = operate at value specified by user
                    //           AirFlowControl only valid if fan opmode = ContFanCycComp
                    if (state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlow == 0.0) {
                        state.dataFurnaces->Furnace(FurnaceNum).AirFlowControl = AirFlowControlConstFan::UseCompressorOnFlow;
                    } else {
                        state.dataFurnaces->Furnace(FurnaceNum).AirFlowControl = AirFlowControlConstFan::UseCompressorOffFlow;
                    }
                }
            }

            if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == Coil_CoolingAirToAirVariableSpeed) {
                errFlag = false;
                if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                    state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex = GetCoilIndexIHP(state, CoolingCoilType, CoolingCoilName, errFlag);
                    IHPCoilName = state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).SCCoilName;
                    state.dataFurnaces->Furnace(FurnaceNum).DesignCoolingCapacity =
                        GetCoilCapacityVariableSpeed(state, "COIL:COOLING:DX:VARIABLESPEED", IHPCoilName, errFlag);
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).DesignCoolingCapacity =
                        GetCoilCapacityVariableSpeed(state, CoolingCoilType, CoolingCoilName, errFlag);
                }

                if (errFlag) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                }
            }

            // Set heating convergence tolerance
            state.dataFurnaces->Furnace(FurnaceNum).HeatingConvergenceTolerance = 0.001;

            // Set cooling convergence tolerance
            state.dataFurnaces->Furnace(FurnaceNum).CoolingConvergenceTolerance = 0.001;

            // set minimum outdoor temperature for compressor operation
            SetMinOATCompressor(state, FurnaceNum, cCurrentModuleObject, ErrorsFound);

        } // End of the HeatCool Furnace Loop

        // Get the data for the Unitary System HeatPump AirToAir (UnitarySystem:HeatPump:AirToAir)
        for (HeatPumpNum = 1; HeatPumpNum <= NumHeatPump; ++HeatPumpNum) {

            CurrentModuleObject = "AirLoopHVAC:UnitaryHeatPump:AirToAir";
            FanInletNode = 0;
            FanOutletNode = 0;
            CoolingCoilInletNode = 0;
            CoolingCoilOutletNode = 0;
            HeatingCoilInletNode = 0;
            HeatingCoilOutletNode = 0;
            SupHeatCoilInletNode = 0;
            SupHeatCoilOutletNode = 0;
            CoolingCoilType = ' ';
            CoolingCoilName = ' ';
            HeatingCoilType = ' ';
            HeatingCoilName = ' ';

            FurnaceNum = NumHeatOnly + NumHeatCool + NumUnitaryHeatOnly + NumUnitaryHeatCool + HeatPumpNum;
            state.dataFurnaces->Furnace(FurnaceNum).iterationMode.allocate(3);

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     HeatPumpNum,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            GlobalNames::VerifyUniqueInterObjectName(
                state, state.dataFurnaces->UniqueFurnaceNames, Alphas(1), CurrentModuleObject, cAlphaFields(1), ErrorsFound);

            state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num = UnitarySys_HeatPump_AirToAir;
            state.dataFurnaces->Furnace(FurnaceNum).Name = Alphas(1);
            if (lAlphaBlanks(2)) {
                state.dataFurnaces->Furnace(FurnaceNum).SchedPtr = DataGlobalConstants::ScheduleAlwaysOn;
            } else {
                state.dataFurnaces->Furnace(FurnaceNum).SchedPtr = GetScheduleIndex(state, Alphas(2));
                if (state.dataFurnaces->Furnace(FurnaceNum).SchedPtr == 0) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(2), Alphas(2)));
                    ErrorsFound = true;
                }
            }

            state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum =
                GetOnlySingleNode(state,
                                  Alphas(3),
                                  ErrorsFound,
                                  DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatPumpAirToAir,
                                  Alphas(1),
                                  DataLoopNode::NodeFluidType::Air,
                                  DataLoopNode::ConnectionType::Inlet,
                                  NodeInputManager::CompFluidStream::Primary,
                                  ObjectIsParent);

            state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum =
                GetOnlySingleNode(state,
                                  Alphas(4),
                                  ErrorsFound,
                                  DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatPumpAirToAir,
                                  Alphas(1),
                                  DataLoopNode::NodeFluidType::Air,
                                  DataLoopNode::ConnectionType::Outlet,
                                  NodeInputManager::CompFluidStream::Primary,
                                  ObjectIsParent);

            TestCompSet(state, CurrentModuleObject, Alphas(1), Alphas(3), Alphas(4), "Air Nodes");

            // Get the Controlling Zone or Location of the Furnace Thermostat
            state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum = UtilityRoutines::FindItemInList(Alphas(5), state.dataHeatBal->Zone);
            if (state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum == 0) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(5), Alphas(5)));
                ErrorsFound = true;
            }

            // Get the node number for the zone with the thermostat
            if (state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum > 0) {
                AirNodeFound = false;
                AirLoopFound = false;
                int ControlledZoneNum = state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum;
                //             Find the controlled zone number for the specified thermostat location
                state.dataFurnaces->Furnace(FurnaceNum).NodeNumOfControlledZone = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ZoneNode;
                //             Determine if furnace is on air loop served by the thermostat location specified
                for (int zoneInNode = 1; zoneInNode <= state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).NumInletNodes; ++zoneInNode) {
                    int AirLoopNumber = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNodeAirLoopNum(zoneInNode);
                    state.dataFurnaces->Furnace(FurnaceNum).airloopNum = AirLoopNumber;
                    if (AirLoopNumber > 0) {
                        for (BranchNum = 1; BranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).NumBranches; ++BranchNum) {
                            for (CompNum = 1; CompNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).TotalComponents;
                                 ++CompNum) {
                                if (!UtilityRoutines::SameString(
                                        state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).Comp(CompNum).Name, Alphas(1)) ||
                                    !UtilityRoutines::SameString(
                                        state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).Comp(CompNum).TypeOf,
                                        CurrentModuleObject))
                                    continue;
                                AirLoopFound = true;
                                state.dataFurnaces->Furnace(FurnaceNum).ZoneInletNode =
                                    state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNode(zoneInNode);
                                break;
                            }
                            if (AirLoopFound) break;
                        }
                        for (TstatZoneNum = 1; TstatZoneNum <= state.dataZoneCtrls->NumTempControlledZones; ++TstatZoneNum) {
                            if (state.dataZoneCtrls->TempControlledZone(TstatZoneNum).ActualZoneNum !=
                                state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                                continue;
                            AirNodeFound = true;
                        }
                        for (TstatZoneNum = 1; TstatZoneNum <= state.dataZoneCtrls->NumComfortControlledZones; ++TstatZoneNum) {
                            if (state.dataZoneCtrls->ComfortControlledZone(TstatZoneNum).ActualZoneNum !=
                                state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                                continue;
                            AirNodeFound = true;
                        }
                    }
                    if (AirLoopFound) break;
                }
                if (!AirNodeFound) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, "Did not find air node (zone with thermostat).");
                    ShowContinueError(state, format("Specified {} = {}", cAlphaFields(5), Alphas(5)));
                    ShowContinueError(
                        state, "Both a ZoneHVAC:EquipmentConnections object and a ZoneControl:Thermostat object must be specified for this zone.");
                    ErrorsFound = true;
                }
                if (!AirLoopFound) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowSevereError(state, "Did not find correct AirLoopHVAC.");
                    ShowContinueError(state, format("Specified {} = {}", cAlphaFields(5), Alphas(5)));
                    ErrorsFound = true;
                }
            }

            // Get fan data
            FanType = Alphas(6);
            FanName = Alphas(7);

            errFlag = false;
            GetFanType(state, FanName, state.dataFurnaces->Furnace(FurnaceNum).FanType_Num, errFlag, CurrentModuleObject, Alphas(1));
            if (errFlag) {
                ErrorsFound = true;
            }

            if (state.dataFurnaces->Furnace(FurnaceNum).FanType_Num == FanType_SimpleOnOff ||
                state.dataFurnaces->Furnace(FurnaceNum).FanType_Num == FanType_SimpleConstVolume) {
                ValidateComponent(state, FanType, FanName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;

                } else { // mine data from fan object

                    // Get the fan index
                    errFlag = false;
                    GetFanIndex(state, FanName, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Get the fan inlet node number
                    errFlag = false;
                    FanInletNode = GetFanInletNode(state, FanType, FanName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Get the fan outlet node number
                    errFlag = false;
                    FanOutletNode = GetFanOutletNode(state, FanType, FanName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Get the fan availability schedule
                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).FanAvailSchedPtr = GetFanAvailSchPtr(state, FanType, FanName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Get the Design Fan Volume Flow Rate
                    errFlag = false;
                    FanVolFlowRate = GetFanDesignVolumeFlowRate(state, FanType, FanName, errFlag);
                    state.dataFurnaces->Furnace(FurnaceNum).ActualFanVolFlowRate = FanVolFlowRate;
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                } // IF (IsNotOK) THEN

            } else {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(6), Alphas(6)));
                ErrorsFound = true;
            }

            // Get heating coil type and name data
            HeatingCoilType = Alphas(8);
            HeatingCoilName = Alphas(9);

            errFlag = false;

            if (UtilityRoutines::SameString(HeatingCoilType, "COIL:HEATING:DX:VARIABLESPEED") ||
                UtilityRoutines::SameString(HeatingCoilType, "COILSYSTEM:INTEGRATEDHEATPUMP:AIRSOURCE")) {
                state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num = Coil_HeatingAirToAirVariableSpeed;
                if (UtilityRoutines::SameString(HeatingCoilType, "COILSYSTEM:INTEGRATEDHEATPUMP:AIRSOURCE"))
                    state.dataFurnaces->Furnace(FurnaceNum).bIsIHP = true;
            } else {
                state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num = GetCoilTypeNum(state, HeatingCoilType, HeatingCoilName, errFlag);
            }

            if (errFlag) {
                ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                ErrorsFound = true;
            }

            if (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == CoilDX_HeatingEmpirical) {
                ValidateComponent(state, HeatingCoilType, HeatingCoilName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;

                } else { // mine data from DX heating coil

                    DXCoils::GetDXCoilIndex(state, HeatingCoilName, state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex, IsNotOK);
                    if (IsNotOK) {
                        ShowContinueError(state, format("...occurs {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Get the Heating Coil Node Names
                    errFlag = false;
                    HeatingCoilInletNode = DXCoils::GetCoilInletNode(state, HeatingCoilType, HeatingCoilName, errFlag);
                    HeatingCoilOutletNode = DXCoils::GetCoilOutletNode(state, HeatingCoilType, HeatingCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Get the design heating capacity
                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity =
                        DXCoils::GetCoilCapacity(state, HeatingCoilType, HeatingCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} ={}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                } // IF (IsNotOK) THEN
            } else if (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingAirToAirVariableSpeed) {
                ValidateComponent(state, HeatingCoilType, HeatingCoilName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else {
                    if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                        state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex = GetCoilIndexIHP(state, HeatingCoilType, HeatingCoilName, errFlag);
                        IHPCoilIndex = state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex;
                        IHPCoilName = state.dataIntegratedHP->IntegratedHeatPumps(IHPCoilIndex).SHCoilName;
                        HeatingCoilInletNode = GetCoilInletNodeVariableSpeed(state, "COIL:HEATING:DX:VARIABLESPEED", IHPCoilName, errFlag);
                        HeatingCoilOutletNode = GetCoilOutletNodeVariableSpeed(state, "COIL:HEATING:DX:VARIABLESPEED", IHPCoilName, errFlag);
                    } else {
                        state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex =
                            GetCoilIndexVariableSpeed(state, HeatingCoilType, HeatingCoilName, errFlag);
                        HeatingCoilInletNode = GetCoilInletNodeVariableSpeed(state, HeatingCoilType, HeatingCoilName, errFlag);
                        HeatingCoilOutletNode = GetCoilOutletNodeVariableSpeed(state, HeatingCoilType, HeatingCoilName, errFlag);
                    }
                }
            } else {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(8), Alphas(8)));
                ErrorsFound = true;
            }

            // Get Cooling Coil Information if available
            CoolingCoilType = Alphas(10);
            CoolingCoilName = Alphas(11);

            if (UtilityRoutines::SameString(CoolingCoilType, "COIL:COOLING:DX:VARIABLESPEED") ||
                UtilityRoutines::SameString(CoolingCoilType, "COILSYSTEM:INTEGRATEDHEATPUMP:AIRSOURCE")) {
                state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num = Coil_CoolingAirToAirVariableSpeed;
                if (UtilityRoutines::SameString(CoolingCoilType, "COILSYSTEM:INTEGRATEDHEATPUMP:AIRSOURCE"))
                    state.dataFurnaces->Furnace(FurnaceNum).bIsIHP = true;
            }

            ValidateComponent(state, CoolingCoilType, CoolingCoilName, IsNotOK, CurrentModuleObject);

            if (IsNotOK) {
                ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                ErrorsFound = true;

            } else { // mine data from DX cooling coil

                errFlag = false;
                PrintMessage = false;

                if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num != Coil_CoolingAirToAirVariableSpeed) {
                    state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num =
                        GetCoilTypeNum(state, CoolingCoilType, CoolingCoilName, errFlag, PrintMessage);
                }

                // If coil type not found, check to see if a HX assisted cooling coil is used.
                if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == 0) {
                    errFlag = false;
                    PrintMessage = false;
                    state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num =
                        HVACHXAssistedCoolingCoil::GetCoilGroupTypeNum(state, CoolingCoilType, CoolingCoilName, errFlag, PrintMessage);
                }

                if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == CoilDX_CoolingSingleSpeed) {

                    // Get the cooling coil node numbers
                    errFlag = false;
                    DXCoils::GetDXCoilIndex(state, CoolingCoilName, state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex, errFlag);
                    CoolingCoilInletNode = DXCoils::GetCoilInletNode(state, CoolingCoilType, CoolingCoilName, errFlag);
                    CoolingCoilOutletNode = DXCoils::GetCoilOutletNode(state, CoolingCoilType, CoolingCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Get the DX cooling coil design capacity
                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).DesignCoolingCapacity =
                        DXCoils::GetCoilCapacity(state, CoolingCoilType, CoolingCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                } else if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == CoilDX_CoolingHXAssisted) {

                    // Get the cooling coil node numbers
                    errFlag = false;
                    GetHXDXCoilIndex(state, CoolingCoilName, state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex, errFlag);
                    CoolingCoilInletNode = HVACHXAssistedCoolingCoil::GetCoilInletNode(state, CoolingCoilType, CoolingCoilName, errFlag);
                    CoolingCoilOutletNode = HVACHXAssistedCoolingCoil::GetCoilOutletNode(state, CoolingCoilType, CoolingCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // Get the heat exchanger assisted cooling coil design capacity
                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).DesignCoolingCapacity =
                        HVACHXAssistedCoolingCoil::GetCoilCapacity(state, CoolingCoilType, CoolingCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }

                    // get the actual index to the DX cooling coil object
                    DXCoilIndex = GetActualDXCoilIndex(state, CoolingCoilType, CoolingCoilName, ErrorsFound);
                    state.dataFurnaces->Furnace(FurnaceNum).ActualDXCoilIndexForHXAssisted = DXCoilIndex;

                } else if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == Coil_CoolingAirToAirVariableSpeed) {
                    // BOS ADDED, AUG/2012, VARIIABLE SPEED DX COOLING COIL
                    //  Furnace(FurnaceNum)%DXCoolCoilType = 'COIL:COOLING:DX:VARIABLESPEED'
                    //  Furnace(FurnaceNum)%DXCoolCoilName = CoolingCoilName
                    ValidateComponent(state, CoolingCoilType, CoolingCoilName, IsNotOK, CurrentModuleObject);
                    if (IsNotOK) {
                        ShowContinueError(state, format("...specified in {}=\"{}\".", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    } else {
                        errFlag = false;
                        if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                            state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex =
                                GetCoilIndexIHP(state, CoolingCoilType, CoolingCoilName, errFlag);
                            IHPCoilName =
                                state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).SCCoilName;
                        } else {
                            state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex =
                                GetCoilIndexVariableSpeed(state, CoolingCoilType, CoolingCoilName, errFlag);
                            IHPCoilName = CoolingCoilName;
                        }

                        if (errFlag) {
                            ShowContinueError(state, format("...specified in {}=\"{}\".", CurrentModuleObject, Alphas(1)));
                            ErrorsFound = true;
                        }

                        if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                            CoolingCoilInletNode = GetCoilInletNodeVariableSpeed(state, "COIL:COOLING:DX:VARIABLESPEED", IHPCoilName, errFlag);
                            CoolingCoilOutletNode = GetCoilOutletNodeVariableSpeed(state, "COIL:COOLING:DX:VARIABLESPEED", IHPCoilName, errFlag);
                            state.dataFurnaces->Furnace(FurnaceNum).CondenserNodeNum = GetVSCoilCondenserInletNode(state, IHPCoilName, errFlag);
                        } else {
                            CoolingCoilInletNode = GetCoilInletNodeVariableSpeed(state, CoolingCoilType, CoolingCoilName, errFlag);
                            CoolingCoilOutletNode = GetCoilOutletNodeVariableSpeed(state, CoolingCoilType, CoolingCoilName, errFlag);
                            state.dataFurnaces->Furnace(FurnaceNum).CondenserNodeNum = GetVSCoilCondenserInletNode(state, CoolingCoilName, errFlag);
                        }

                        if (errFlag) {
                            ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                            ErrorsFound = true;
                        }
                    }
                } else {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(10), Alphas(10)));
                    ErrorsFound = true;
                }
            }

            if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == Coil_CoolingAirToAirVariableSpeed &&
                state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingAirToAirVariableSpeed) {
                // Furnace(FurnaceNum)%WatertoAirHPType = WatertoAir_VarSpeedEquationFit
                if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                    SetVarSpeedCoilData(
                        state,
                        state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).SCCoilIndex,
                        ErrorsFound,
                        _,
                        state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).SHCoilIndex);
                } else {
                    SetVarSpeedCoilData(state,
                                        state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                        ErrorsFound,
                                        _,
                                        state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex);
                }
            }

            // Get supplemental heating coil information
            SuppHeatCoilType = Alphas(12);
            SuppHeatCoilName = Alphas(13);
            state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType = SuppHeatCoilType;
            state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilName = SuppHeatCoilName;
            errFlag = false;
            if (UtilityRoutines::SameString(SuppHeatCoilType, "Coil:Heating:Fuel") ||
                UtilityRoutines::SameString(SuppHeatCoilType, "Coil:Heating:Electric")) {

                state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType_Num =
                    HeatingCoils::GetHeatingCoilTypeNum(state, SuppHeatCoilType, SuppHeatCoilName, errFlag);
                if (errFlag) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else {
                    IsNotOK = false;
                    ValidateComponent(state, SuppHeatCoilType, SuppHeatCoilName, IsNotOK, CurrentModuleObject);
                    if (IsNotOK) {
                        ShowContinueError(state, format("In {} \"{}\"", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;

                    } else { // mine data from the supplemental heating coil

                        HeatingCoils::GetCoilIndex(state, SuppHeatCoilName, state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex, IsNotOK);
                        if (IsNotOK) {
                            ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                            ErrorsFound = true;
                        }

                        // Get the Supplemental Heating Coil Inlet Node Number
                        errFlag = false;
                        SupHeatCoilInletNode = HeatingCoils::GetCoilInletNode(state, SuppHeatCoilType, SuppHeatCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError(state, format("...occurs in {} \"{}\"", CurrentModuleObject, Alphas(1)));
                            ErrorsFound = true;
                        }

                        // Get the Supplemental Heating Coil Outlet Node Number
                        errFlag = false;
                        SupHeatCoilOutletNode = HeatingCoils::GetCoilOutletNode(state, SuppHeatCoilType, SuppHeatCoilName, errFlag);

                        if (errFlag) {
                            ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                            ErrorsFound = true;
                        }

                        // Get the supplemental heating coil design capacity
                        errFlag = false;
                        state.dataFurnaces->Furnace(FurnaceNum).DesignSuppHeatingCapacity =
                            HeatingCoils::GetCoilCapacity(state, SuppHeatCoilType, SuppHeatCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                            ErrorsFound = true;
                        }

                    } // IF (IsNotOK) THEN
                }
            } else if (UtilityRoutines::SameString(SuppHeatCoilType, "Coil:Heating:Water")) {
                state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType_Num = Coil_HeatingWater;
                ValidateComponent(state, SuppHeatCoilType, SuppHeatCoilName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else { // mine data from heating coil object

                    // Get the Heating Coil water Inlet or control Node number
                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).SuppCoilControlNode =
                        GetCoilWaterInletNode(state, "Coil:Heating:Water", SuppHeatCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the ReHeat Coil hot water max volume flow rate
                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow =
                        GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", SuppHeatCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the ReHeat Coil Inlet Node
                    errFlag = false;
                    SupHeatCoilInletNode = WaterCoils::GetCoilInletNode(state, "Coil:Heating:Water", SuppHeatCoilName, errFlag);
                    state.dataFurnaces->Furnace(FurnaceNum).SuppCoilAirInletNode = SupHeatCoilInletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the ReHeat Coil Outlet Node
                    errFlag = false;
                    SupHeatCoilOutletNode = WaterCoils::GetCoilOutletNode(state, "Coil:Heating:Water", SuppHeatCoilName, errFlag);
                    state.dataFurnaces->Furnace(FurnaceNum).SuppCoilAirOutletNode = SupHeatCoilOutletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }
                    errFlag = false;
                    CheckCoilWaterInletNode(state, state.dataFurnaces->Furnace(FurnaceNum).CoilControlNode, errFlag);
                    if (!errFlag) { // then did find a controller so that is bad
                        ShowSevereError(state,
                                        format("{} = {} has a conflicting Controller:WaterCoil object",
                                               CurrentModuleObject,
                                               state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ShowContinueError(state, "Hot water coils are controlled directly by unitary and furnace systems.");
                        ShowContinueError(state, "No water coil controller should be input for the coil.");
                        ErrorsFound = true;
                    }
                }

            } else if (UtilityRoutines::SameString(SuppHeatCoilType, "Coil:Heating:Steam")) {
                state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType_Num = Coil_HeatingSteam;
                ValidateComponent(state, SuppHeatCoilType, SuppHeatCoilName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else { // mine data from heating coil object

                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex =
                        SteamCoils::GetSteamCoilIndex(state, "COIL:HEATING:STEAM", SuppHeatCoilName, errFlag);
                    if (state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex == 0) {
                        ShowSevereError(state, format("{} illegal {} = {}", CurrentModuleObject, cAlphaFields(12), SuppHeatCoilName));
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the Heating Coil steam inlet node number
                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).SuppCoilControlNode =
                        GetCoilSteamInletNode(state, "Coil:Heating:Steam", SuppHeatCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the Heating Coil steam max volume flow rate
                    state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow =
                        SteamCoils::GetCoilMaxSteamFlowRate(state, state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex, errFlag);
                    if (state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow > 0.0) {
                        SteamIndex = 0; // Function GetSatDensityRefrig will look up steam index if 0 is passed
                        SteamDensity =
                            GetSatDensityRefrig(state, fluidNameSteam, state.dataFurnaces->TempSteamIn, 1.0, SteamIndex, getAirLoopHVACHeatCoolInput);
                        state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow =
                            SteamCoils::GetCoilMaxSteamFlowRate(state, state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex, errFlag) *
                            SteamDensity;
                    }

                    // Get the Heating Coil Inlet Node
                    errFlag = false;
                    SupHeatCoilInletNode =
                        SteamCoils::GetCoilAirInletNode(state, state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex, SuppHeatCoilName, errFlag);
                    state.dataFurnaces->Furnace(FurnaceNum).SuppCoilAirInletNode = SupHeatCoilInletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the Heating Coil Outlet Node
                    errFlag = false;
                    SupHeatCoilOutletNode =
                        GetCoilAirOutletNode(state, state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex, SuppHeatCoilName, errFlag);
                    state.dataFurnaces->Furnace(FurnaceNum).SuppCoilAirOutletNode = SupHeatCoilOutletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }
                }

            } else {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(12), Alphas(12)));
                ErrorsFound = true;
            } // IF (Furnace(FurnaceNum)%HeatingCoilType_Num == Coil_HeatingGasOrOtherFuel .OR. &, etc.

            if (UtilityRoutines::SameString(Alphas(14), "BlowThrough")) state.dataFurnaces->Furnace(FurnaceNum).FanPlace = BlowThru;
            if (UtilityRoutines::SameString(Alphas(14), "DrawThrough")) state.dataFurnaces->Furnace(FurnaceNum).FanPlace = DrawThru;
            if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == 0) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(14), Alphas(14)));
                ErrorsFound = true;
            }

            state.dataFurnaces->Furnace(FurnaceNum).FanSchedPtr = GetScheduleIndex(state, Alphas(15));
            if (!lAlphaBlanks(15) && state.dataFurnaces->Furnace(FurnaceNum).FanSchedPtr == 0) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(15), Alphas(15)));
                ErrorsFound = true;
            } else if (lAlphaBlanks(15)) {
                state.dataFurnaces->Furnace(FurnaceNum).OpMode = CycFanCycCoil;
                if (state.dataFurnaces->Furnace(FurnaceNum).FanType_Num != FanType_SimpleOnOff) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                    ShowContinueError(state, format("{} = {}", cAlphaFields(6), Alphas(6)));
                    ShowContinueError(state, format("Fan type must be Fan:OnOff when {} = Blank.", cAlphaFields(15)));
                    ErrorsFound = true;
                }
            }

            if (state.dataFurnaces->Furnace(FurnaceNum).FanType_Num == FanType_SimpleConstVolume) {
                if (state.dataFurnaces->Furnace(FurnaceNum).FanSchedPtr > 0) {
                    if (!CheckScheduleValueMinMax(state, state.dataFurnaces->Furnace(FurnaceNum).FanSchedPtr, ">", 0.0, "<=", 1.0)) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state, format("For {} = {}", cAlphaFields(7), Alphas(7)));
                        ShowContinueError(state, "Fan operating mode must be continuous (fan operating mode schedule values > 0).");
                        ShowContinueError(state, format("Error found in {} = {}", cAlphaFields(15), Alphas(15)));
                        ShowContinueError(state, "...schedule values must be (>0., <=1.)");
                        ErrorsFound = true;
                    }
                }
            }

            // Dehumidification Control Type
            if (UtilityRoutines::SameString(Alphas(16), "None") || UtilityRoutines::SameString(Alphas(16), "Multimode") ||
                UtilityRoutines::SameString(Alphas(16), "CoolReheat")) {
                AirNodeFound = false;
                if (UtilityRoutines::SameString(Alphas(16), "Multimode")) {
                    state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num = DehumidificationControlMode::Multimode;
                    state.dataFurnaces->Furnace(FurnaceNum).Humidistat = true;
                    if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num != CoilDX_CoolingHXAssisted) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(16), Alphas(16)));
                        ShowContinueError(state, "Multimode control must be used with a Heat Exchanger Assisted Cooling Coil.");
                        ErrorsFound = true;
                    }
                }
                if (UtilityRoutines::SameString(Alphas(16), "CoolReheat")) {
                    state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num = DehumidificationControlMode::CoolReheat;
                    state.dataFurnaces->Furnace(FurnaceNum).Humidistat = true;
                }
                if (UtilityRoutines::SameString(Alphas(16), "None")) {
                    state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num = DehumidificationControlMode::None;
                    state.dataFurnaces->Furnace(FurnaceNum).Humidistat = false;
                }
                if (state.dataFurnaces->Furnace(FurnaceNum).Humidistat) {
                    for (HStatZoneNum = 1; HStatZoneNum <= state.dataZoneCtrls->NumHumidityControlZones; ++HStatZoneNum) {
                        if (state.dataZoneCtrls->HumidityControlZone(HStatZoneNum).ActualZoneNum !=
                            state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                            continue;
                        AirNodeFound = true;
                    }
                    if (!AirNodeFound) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state, "Did not find Air Node (Zone with Humidistat).");
                        ShowContinueError(state, format("Specified {} = {}", cAlphaFields(5), Alphas(5)));
                        ErrorsFound = true;
                    }
                }
            } else { // invalid input or blank
                if (!lAlphaBlanks(16)) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(16), Alphas(16)));
                    ErrorsFound = true;
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).Humidistat = false;
                    state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num = DehumidificationControlMode::None;
                }
            }

            // Check node names for child components
            if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == BlowThru) {
                if (FanInletNode != state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum) {
                    ShowSevereError(state, format("For {} \"{}\"", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(
                        state,
                        "When a blow through fan is specified, the fan inlet node name must be the same as the unitary system inlet node name.");
                    ShowContinueError(state, format("...Fan inlet node name            = {}", state.dataLoopNodes->NodeID(FanInletNode)));
                    ShowContinueError(state,
                                      format("...Unitary system inlet node name = {}",
                                             state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum)));
                    ErrorsFound = true;
                }
                if (FanOutletNode != CoolingCoilInletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(
                        state,
                        "When a blow through fan is specified, the fan outlet node name must be the same as the cooling coil inlet node name.");
                    ShowContinueError(state, format("...Fan outlet node name         = {}", state.dataLoopNodes->NodeID(FanOutletNode)));
                    ShowContinueError(state, format("...Cooling coil inlet node name = {}", state.dataLoopNodes->NodeID(CoolingCoilInletNode)));
                    ErrorsFound = true;
                }
                if (CoolingCoilOutletNode != HeatingCoilInletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, "The cooling coil outlet node name must be the same as the heating coil inlet node name.");
                    ShowContinueError(state, format("...Cooling coil outlet node name = {}", state.dataLoopNodes->NodeID(CoolingCoilOutletNode)));
                    ShowContinueError(state, format("...Heating coil inlet node name  = {}", state.dataLoopNodes->NodeID(HeatingCoilInletNode)));
                    ErrorsFound = true;
                }
                if (HeatingCoilOutletNode != SupHeatCoilInletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state,
                                      "When a blow through fan is specified, the heating coil outlet node name must be the same as the supplemental "
                                      "heating coil inlet node name.");
                    ShowContinueError(
                        state, format("...Heating coil outlet node name              = {}", state.dataLoopNodes->NodeID(HeatingCoilOutletNode)));
                    ShowContinueError(
                        state, format("...Supplemental heating coil inlet node name  = {}", state.dataLoopNodes->NodeID(SupHeatCoilInletNode)));
                    ErrorsFound = true;
                }
                if (SupHeatCoilOutletNode != state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum) {
                    ShowSevereError(state, format("For {} \"{}\"", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state,
                                      "The supplemental heating coil outlet node name must be the same as the unitary system outlet node name.");
                    ShowContinueError(
                        state, format("...Supplemental heating coil outlet node name = {}", state.dataLoopNodes->NodeID(SupHeatCoilOutletNode)));
                    ShowContinueError(state,
                                      format("...Unitary system outlet node name            = {}",
                                             state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum)));
                    ErrorsFound = true;
                }
            } else {
                if (CoolingCoilInletNode != state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum) {
                    ShowSevereError(state, format("For {} \"{}\"", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state,
                                      "When a draw through fan is specified, the cooling coil inlet node name must be the same as the unitary system "
                                      "inlet node name.");
                    ShowContinueError(state, format("...Cooling coil inlet node name   = {}", state.dataLoopNodes->NodeID(CoolingCoilInletNode)));
                    ShowContinueError(state,
                                      format("...Unitary system inlet node name = {}",
                                             state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum)));
                    ErrorsFound = true;
                }
                if (CoolingCoilOutletNode != HeatingCoilInletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, "The cooling coil outlet node name must be the same as the heating coil inlet node name.");
                    ShowContinueError(state, format("...Cooling coil outlet node name = {}", state.dataLoopNodes->NodeID(CoolingCoilOutletNode)));
                    ShowContinueError(state, format("...Heating coil inlet node name  = {}", state.dataLoopNodes->NodeID(HeatingCoilInletNode)));
                    ErrorsFound = true;
                }
                if (HeatingCoilOutletNode != FanInletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(
                        state,
                        "When a draw through fan is specified, the heating coil outlet node name must be the same as the fan inlet node name.");
                    ShowContinueError(state, format("...Heating coil outlet node name = {}", state.dataLoopNodes->NodeID(HeatingCoilOutletNode)));
                    ShowContinueError(state, format("...Fan inlet node name           = {}", state.dataLoopNodes->NodeID(FanInletNode)));
                    ErrorsFound = true;
                }
                if (FanOutletNode != SupHeatCoilInletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state,
                                      "When a draw through fan is specified, the fan outlet node name must be the same as the supplemental heating "
                                      "coil inlet node name.");
                    ShowContinueError(state,
                                      format("...Fan outlet node name                       = {}", state.dataLoopNodes->NodeID(FanOutletNode)));
                    ShowContinueError(
                        state, format("...Supplemental heating coil inlet node name  = {}", state.dataLoopNodes->NodeID(SupHeatCoilInletNode)));
                    ErrorsFound = true;
                }
                if (SupHeatCoilOutletNode != state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum) {
                    ShowSevereError(state, format("For {} \"{}\"", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state,
                                      "The supplemental heating coil outlet node name must be the same as the unitary system outlet node name.");
                    ShowContinueError(
                        state, format("...Supplemental heating coil outlet node name = {}", state.dataLoopNodes->NodeID(SupHeatCoilOutletNode)));
                    ShowContinueError(state,
                                      format("...Unitary system outlet node name            = {}",
                                             state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum)));
                    ErrorsFound = true;
                }
            }

            // Add component sets array
            if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == BlowThru) {
                CompSetFanInlet = Alphas(3);
                CompSetCoolInlet = "UNDEFINED";
            } else {
                CompSetFanInlet = "UNDEFINED";
                CompSetCoolInlet = Alphas(3);
            }
            SetUpCompSets(state, CurrentModuleObject, Alphas(1), Alphas(6), Alphas(7), CompSetFanInlet, "UNDEFINED");

            // Add DX cooling coil to component sets array
            if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                SetUpCompSets(state, CurrentModuleObject, Alphas(1), Alphas(10), Alphas(11) + " Cooling Coil", CompSetCoolInlet, "UNDEFINED");
            } else {
                SetUpCompSets(state, CurrentModuleObject, Alphas(1), Alphas(10), Alphas(11), CompSetCoolInlet, "UNDEFINED");
            }
            // Add DX heating coil to component sets array
            if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                SetUpCompSets(state, CurrentModuleObject, Alphas(1), Alphas(8), Alphas(9) + " Heating Coil", "UNDEFINED", "UNDEFINED");
            } else {
                SetUpCompSets(state, CurrentModuleObject, Alphas(1), Alphas(8), Alphas(9), "UNDEFINED", "UNDEFINED");
            }

            // Add supplemental heating coil to component sets array
            SetUpCompSets(state, CurrentModuleObject, Alphas(1), Alphas(12), Alphas(13), "UNDEFINED", Alphas(4));

            state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow = Numbers(1);
            if (state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow <= 0 &&
                state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {:.7T}", cNumericFields(1), Numbers(1)));
                ErrorsFound = true;
            }

            state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow = Numbers(2);
            if (state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow <= 0 &&
                state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {:.7T}", cNumericFields(2), Numbers(2)));
                ErrorsFound = true;
            }

            state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlow = Numbers(3);
            if (state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlow < 0 &&
                state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {:.7T}", cNumericFields(3), Numbers(3)));
                ErrorsFound = true;
            }

            if (state.dataFurnaces->Furnace(FurnaceNum).FanSchedPtr > 0) {
                if (!CheckScheduleValueMinMax(
                        state, state.dataFurnaces->Furnace(FurnaceNum).FanSchedPtr, ">=", 0.0, "<=", 0.0)) { // Autodesk:Note Range is 0 to 0?
                    //           set air flow control mode:
                    //             UseCompressorOnFlow = operate at last cooling or heating air flow requested when compressor is off
                    //             UseCompressorOffFlow = operate at value specified by user
                    //           AirFlowControl only valid if fan opmode = ContFanCycComp
                    if (state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlow == 0.0) {
                        state.dataFurnaces->Furnace(FurnaceNum).AirFlowControl = AirFlowControlConstFan::UseCompressorOnFlow;
                    } else {
                        state.dataFurnaces->Furnace(FurnaceNum).AirFlowControl = AirFlowControlConstFan::UseCompressorOffFlow;
                    }
                }
            }

            if (Numbers(1) != DataSizing::AutoSize && Numbers(2) != DataSizing::AutoSize && Numbers(3) != DataSizing::AutoSize) {
                state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate = max(Numbers(1), Numbers(2), Numbers(3));
            } else {
                state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate = DataSizing::AutoSize;
            }

            if (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingAirToAirVariableSpeed) {
                errFlag = false;

                if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                    IHPCoilName = state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).SHCoilName;
                    state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow =
                        GetCoilAirFlowRateVariableSpeed(state, "COIL:HEATING:DX:VARIABLESPEED", IHPCoilName, errFlag);
                    IHPCoilName = state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).SCCoilName;
                    state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow =
                        GetCoilAirFlowRateVariableSpeed(state, "COIL:COOLING:DX:VARIABLESPEED", IHPCoilName, errFlag);
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow =
                        GetCoilAirFlowRateVariableSpeed(state, HeatingCoilType, HeatingCoilName, errFlag);
                    state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow =
                        GetCoilAirFlowRateVariableSpeed(state, CoolingCoilType, CoolingCoilName, errFlag);
                }

                if (errFlag) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                }

                state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlow =
                    min(state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow, state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow);
                if (state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow != DataSizing::AutoSize &&
                    state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow != DataSizing::AutoSize) {
                    state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate =
                        max(state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow, state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow);
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate = DataSizing::AutoSize;
                }
            }

            if (FanVolFlowRate != DataSizing::AutoSize) {
                if (FanVolFlowRate < state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow &&
                    state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow != DataSizing::AutoSize) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(
                        state,
                        format("... air flow rate = {:.7T} in fan object {} is less than the maximum HVAC system air flow rate in cooling mode.",
                               FanVolFlowRate,
                               FanName));
                    ShowContinueError(state, format(" The {} is reset to the fan flow rate and the simulation continues.", cNumericFields(1)));
                    state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow = FanVolFlowRate;
                    state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate = FanVolFlowRate;
                }
                if (FanVolFlowRate < state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow &&
                    state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow != DataSizing::AutoSize) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(
                        state,
                        format("... air flow rate = {:.7T} in fan object {} is less than the maximum HVAC system air flow rate in heating mode.",
                               FanVolFlowRate,
                               FanName));
                    ShowContinueError(state, format(" The {} is reset to the fan flow rate and the simulation continues.", cNumericFields(2)));
                    state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow = FanVolFlowRate;
                    state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate = FanVolFlowRate;
                }
            }

            // Set heating convergence tolerance
            state.dataFurnaces->Furnace(FurnaceNum).HeatingConvergenceTolerance = 0.001;

            //       Mine heatpump outdoor condenser node from DX coil object
            errFlag = false;
            if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == CoilDX_CoolingSingleSpeed) {
                state.dataFurnaces->Furnace(FurnaceNum).CondenserNodeNum =
                    DXCoils::GetCoilCondenserInletNode(state, CoolingCoilType, CoolingCoilName, errFlag);
            } else if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == Coil_CoolingAirToAirVariableSpeed) {
                if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                    IHPCoilName = state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).SCCoilName;
                    state.dataFurnaces->Furnace(FurnaceNum).CondenserNodeNum = GetVSCoilCondenserInletNode(state, IHPCoilName, errFlag);
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).CondenserNodeNum = GetVSCoilCondenserInletNode(state, CoolingCoilName, errFlag);
                }
            } else {
                state.dataFurnaces->Furnace(FurnaceNum).CondenserNodeNum = DXCoils::GetCoilCondenserInletNode(
                    state, "Coil:Cooling:DX:SingleSpeed", GetHXDXCoilName(state, CoolingCoilType, CoolingCoilName, errFlag), errFlag);
            }
            if (errFlag) {
                ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                ErrorsFound = true;
            }

            if (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingAirToAirVariableSpeed) {
                errFlag = false;
                if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                    IHPCoilName = state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).SHCoilName;
                    state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity =
                        GetCoilCapacityVariableSpeed(state, "Coil:Heating:DX:VariableSpeed", IHPCoilName, errFlag);
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity =
                        GetCoilCapacityVariableSpeed(state, HeatingCoilType, HeatingCoilName, errFlag);
                }

                if (errFlag) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                }
            }

            if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == Coil_CoolingAirToAirVariableSpeed) {
                errFlag = false;
                if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                    IHPCoilName = state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).SCCoilName;
                    state.dataFurnaces->Furnace(FurnaceNum).DesignCoolingCapacity =
                        GetCoilCapacityVariableSpeed(state, "COIL:COOLING:DX:VARIABLESPEED", IHPCoilName, errFlag);
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).DesignCoolingCapacity =
                        GetCoilCapacityVariableSpeed(state, CoolingCoilType, CoolingCoilName, errFlag);
                }

                if (errFlag) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                }
            }

            // Set cooling convergence tolerance
            state.dataFurnaces->Furnace(FurnaceNum).CoolingConvergenceTolerance = 0.001;

            // Set the furnace max outlet temperature
            state.dataFurnaces->Furnace(FurnaceNum).DesignMaxOutletTemp = Numbers(4);

            // Set maximum supply air temperature for supplemental heating coil
            state.dataFurnaces->Furnace(FurnaceNum).MaxOATSuppHeat = Numbers(5);

            // set minimum outdoor temperature for compressor operation
            SetMinOATCompressor(state, FurnaceNum, cCurrentModuleObject, ErrorsFound);

        } // End of the Unitary System HeatPump Loop

        // Get the Input for the Water to Air Heat Pump (UnitarySystem:HeatPump:WaterToAir)
        for (HeatPumpNum = 1; HeatPumpNum <= NumWaterToAirHeatPump; ++HeatPumpNum) {

            CurrentModuleObject = "AirLoopHVAC:UnitaryHeatPump:WaterToAir";
            FanInletNode = 0;
            FanOutletNode = 0;
            CoolingCoilInletNode = 0;
            CoolingCoilOutletNode = 0;
            HeatingCoilInletNode = 0;
            HeatingCoilOutletNode = 0;
            SupHeatCoilInletNode = 0;
            SupHeatCoilOutletNode = 0;
            CoolingCoilType = ' ';
            CoolingCoilName = ' ';
            HeatingCoilType = ' ';
            HeatingCoilName = ' ';

            FurnaceNum = NumHeatOnly + NumHeatCool + NumUnitaryHeatOnly + NumUnitaryHeatCool + NumHeatPump + HeatPumpNum;
            state.dataFurnaces->Furnace(FurnaceNum).iterationMode.allocate(3);

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     HeatPumpNum,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            GlobalNames::VerifyUniqueInterObjectName(
                state, state.dataFurnaces->UniqueFurnaceNames, Alphas(1), CurrentModuleObject, cAlphaFields(1), ErrorsFound);

            state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num = UnitarySys_HeatPump_WaterToAir;
            state.dataFurnaces->Furnace(FurnaceNum).Name = Alphas(1);
            if (lAlphaBlanks(2)) {
                state.dataFurnaces->Furnace(FurnaceNum).SchedPtr = DataGlobalConstants::ScheduleAlwaysOn;
            } else {
                state.dataFurnaces->Furnace(FurnaceNum).SchedPtr = GetScheduleIndex(state, Alphas(2));
                if (state.dataFurnaces->Furnace(FurnaceNum).SchedPtr == 0) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(2), Alphas(2)));
                    ErrorsFound = true;
                }
            }

            state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum =
                GetOnlySingleNode(state,
                                  Alphas(3),
                                  ErrorsFound,
                                  DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatPumpWaterToAir,
                                  Alphas(1),
                                  DataLoopNode::NodeFluidType::Air,
                                  DataLoopNode::ConnectionType::Inlet,
                                  NodeInputManager::CompFluidStream::Primary,
                                  ObjectIsParent);

            state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum =
                GetOnlySingleNode(state,
                                  Alphas(4),
                                  ErrorsFound,
                                  DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatPumpWaterToAir,
                                  Alphas(1),
                                  DataLoopNode::NodeFluidType::Air,
                                  DataLoopNode::ConnectionType::Outlet,
                                  NodeInputManager::CompFluidStream::Primary,
                                  ObjectIsParent);

            TestCompSet(state, CurrentModuleObject, Alphas(1), Alphas(3), Alphas(4), "Air Nodes");

            // Get the Controlling Zone or Location of the Furnace Thermostat
            state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum = UtilityRoutines::FindItemInList(Alphas(5), state.dataHeatBal->Zone);
            if (state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum == 0) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(5), Alphas(5)));
                ErrorsFound = true;
            }

            // Get the node number for the zone with the thermostat
            if (state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum > 0) {
                AirNodeFound = false;
                AirLoopFound = false;
                int ControlledZoneNum = state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum;
                //             Find the controlled zone number for the specified thermostat location
                state.dataFurnaces->Furnace(FurnaceNum).NodeNumOfControlledZone = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ZoneNode;
                //             Determine if furnace is on air loop served by the thermostat location specified
                for (int zoneInNode = 1; zoneInNode <= state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).NumInletNodes; ++zoneInNode) {
                    int AirLoopNumber = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNodeAirLoopNum(zoneInNode);
                    state.dataFurnaces->Furnace(FurnaceNum).airloopNum = AirLoopNumber;
                    if (AirLoopNumber > 0) {
                        for (BranchNum = 1; BranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).NumBranches; ++BranchNum) {
                            for (CompNum = 1; CompNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).TotalComponents;
                                 ++CompNum) {
                                if (!UtilityRoutines::SameString(
                                        state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).Comp(CompNum).Name, Alphas(1)) ||
                                    !UtilityRoutines::SameString(
                                        state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).Comp(CompNum).TypeOf,
                                        CurrentModuleObject))
                                    continue;
                                AirLoopFound = true;
                                state.dataFurnaces->Furnace(FurnaceNum).ZoneInletNode =
                                    state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNode(zoneInNode);
                                break;
                            }
                            if (AirLoopFound) break;
                        }
                        for (TstatZoneNum = 1; TstatZoneNum <= state.dataZoneCtrls->NumTempControlledZones; ++TstatZoneNum) {
                            if (state.dataZoneCtrls->TempControlledZone(TstatZoneNum).ActualZoneNum !=
                                state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                                continue;
                            AirNodeFound = true;
                        }
                        for (TstatZoneNum = 1; TstatZoneNum <= state.dataZoneCtrls->NumComfortControlledZones; ++TstatZoneNum) {
                            if (state.dataZoneCtrls->ComfortControlledZone(TstatZoneNum).ActualZoneNum !=
                                state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                                continue;
                            AirNodeFound = true;
                        }
                    }
                    if (AirLoopFound) break;
                }
                if (!AirNodeFound) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, "Did not find air node (zone with thermostat).");
                    ShowContinueError(state, format("Specified {} = {}", cAlphaFields(5), Alphas(5)));
                    ShowContinueError(
                        state, "Both a ZoneHVAC:EquipmentConnections object and a ZoneControl:Thermostat object must be specified for this zone.");
                    ErrorsFound = true;
                }
                if (!AirLoopFound) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowSevereError(state, "Did not find correct AirLoopHVAC.");
                    ShowContinueError(state, format("Specified {} = {}", cAlphaFields(5), Alphas(5)));
                    ErrorsFound = true;
                }
            }

            // Get fan data
            FanType = Alphas(6);
            FanName = Alphas(7);
            errFlag = false;
            GetFanType(state, FanName, state.dataFurnaces->Furnace(FurnaceNum).FanType_Num, errFlag, CurrentModuleObject, Alphas(1));
            if (errFlag) {
                ErrorsFound = true;
            }

            if (state.dataFurnaces->Furnace(FurnaceNum).FanType_Num == FanType_SimpleOnOff) {
                ValidateComponent(state, FanType, FanName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else {
                    errFlag = false;
                    GetFanIndex(state, FanName, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }
                    errFlag = false;
                    FanInletNode = GetFanInletNode(state, FanType, FanName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }
                    errFlag = false;
                    FanOutletNode = GetFanOutletNode(state, FanType, FanName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }
                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).FanAvailSchedPtr = GetFanAvailSchPtr(state, FanType, FanName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    }
                }
            } else {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(6), Alphas(6)));
                ErrorsFound = true;
            }

            // Get heating coil type and name data
            if (Alphas(8) == "COIL:HEATING:WATERTOAIRHEATPUMP:PARAMETERESTIMATION") {
                HeatingCoilType = Alphas(8);
                state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num = Coil_HeatingWaterToAirHP;
                HeatingCoilName = Alphas(9);
                ValidateComponent(state, HeatingCoilType, HeatingCoilName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex =
                        WaterToAirHeatPump::GetCoilIndex(state, HeatingCoilType, HeatingCoilName, errFlag);
                    HeatingCoilInletNode = WaterToAirHeatPump::GetCoilInletNode(state, HeatingCoilType, HeatingCoilName, errFlag);
                    HeatingCoilOutletNode = WaterToAirHeatPump::GetCoilOutletNode(state, HeatingCoilType, HeatingCoilName, errFlag);
                }
            } else if (Alphas(8) == "COIL:HEATING:WATERTOAIRHEATPUMP:EQUATIONFIT") {
                HeatingCoilType = Alphas(8);
                state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num = Coil_HeatingWaterToAirHPSimple;
                HeatingCoilName = Alphas(9);
                ValidateComponent(state, HeatingCoilType, HeatingCoilName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex =
                        WaterToAirHeatPumpSimple::GetCoilIndex(state, HeatingCoilType, HeatingCoilName, errFlag);
                    HeatingCoilInletNode = WaterToAirHeatPumpSimple::GetCoilInletNode(state, HeatingCoilType, HeatingCoilName, errFlag);
                    HeatingCoilOutletNode = WaterToAirHeatPumpSimple::GetCoilOutletNode(state, HeatingCoilType, HeatingCoilName, errFlag);
                }
            } else if (Alphas(8) == "COIL:HEATING:WATERTOAIRHEATPUMP:VARIABLESPEEDEQUATIONFIT") {
                HeatingCoilType = Alphas(8);
                state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num = Coil_HeatingWaterToAirHPVSEquationFit;
                HeatingCoilName = Alphas(9);
                ValidateComponent(state, HeatingCoilType, HeatingCoilName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex =
                        GetCoilIndexVariableSpeed(state, HeatingCoilType, HeatingCoilName, errFlag);
                    HeatingCoilInletNode = GetCoilInletNodeVariableSpeed(state, HeatingCoilType, HeatingCoilName, errFlag);
                    HeatingCoilOutletNode = GetCoilOutletNodeVariableSpeed(state, HeatingCoilType, HeatingCoilName, errFlag);
                }
            } else {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(8), Alphas(8)));
                ErrorsFound = true;
            }

            // Get Cooling Coil Information if available
            if (Alphas(10) == "COIL:COOLING:WATERTOAIRHEATPUMP:PARAMETERESTIMATION") {
                CoolingCoilType = Alphas(10);
                state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num = Coil_CoolingWaterToAirHP;
                CoolingCoilName = Alphas(11);
                ValidateComponent(state, CoolingCoilType, CoolingCoilName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex =
                        WaterToAirHeatPump::GetCoilIndex(state, CoolingCoilType, CoolingCoilName, errFlag);
                    CoolingCoilInletNode = WaterToAirHeatPump::GetCoilInletNode(state, CoolingCoilType, CoolingCoilName, errFlag);
                    CoolingCoilOutletNode = WaterToAirHeatPump::GetCoilOutletNode(state, CoolingCoilType, CoolingCoilName, errFlag);
                }
            } else if (Alphas(10) == "COIL:COOLING:WATERTOAIRHEATPUMP:EQUATIONFIT") {
                CoolingCoilType = Alphas(10);
                state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num = Coil_CoolingWaterToAirHPSimple;
                CoolingCoilName = Alphas(11);
                ValidateComponent(state, CoolingCoilType, CoolingCoilName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex =
                        WaterToAirHeatPumpSimple::GetCoilIndex(state, CoolingCoilType, CoolingCoilName, errFlag);
                    CoolingCoilInletNode = WaterToAirHeatPumpSimple::GetCoilInletNode(state, CoolingCoilType, CoolingCoilName, errFlag);
                    CoolingCoilOutletNode = WaterToAirHeatPumpSimple::GetCoilOutletNode(state, CoolingCoilType, CoolingCoilName, errFlag);
                }
            } else if (Alphas(10) == "COIL:COOLING:WATERTOAIRHEATPUMP:VARIABLESPEEDEQUATIONFIT") {
                CoolingCoilType = Alphas(10);
                state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num = Coil_CoolingWaterToAirHPVSEquationFit;
                CoolingCoilName = Alphas(11);
                ValidateComponent(state, CoolingCoilType, CoolingCoilName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex =
                        GetCoilIndexVariableSpeed(state, CoolingCoilType, CoolingCoilName, errFlag);
                    CoolingCoilInletNode = GetCoilInletNodeVariableSpeed(state, CoolingCoilType, CoolingCoilName, errFlag);
                    CoolingCoilOutletNode = GetCoilOutletNodeVariableSpeed(state, CoolingCoilType, CoolingCoilName, errFlag);
                }
            } else {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(10), Alphas(10)));
                ErrorsFound = true;
            }

            if (NumAlphas >= 18) {
                // get water flow mode info before CALL SetSimpleWSHPData
                if (UtilityRoutines::SameString(Alphas(18), "Constant")) state.dataFurnaces->Furnace(FurnaceNum).WaterCyclingMode = WaterConstant;
                if (UtilityRoutines::SameString(Alphas(18), "Cycling")) state.dataFurnaces->Furnace(FurnaceNum).WaterCyclingMode = WaterCycling;
                if (UtilityRoutines::SameString(Alphas(18), "ConstantOnDemand"))
                    state.dataFurnaces->Furnace(FurnaceNum).WaterCyclingMode = WaterConstantOnDemand;
                // default to draw through if not specified in input
                if (lAlphaBlanks(18)) state.dataFurnaces->Furnace(FurnaceNum).WaterCyclingMode = WaterCycling;
            } else {
                state.dataFurnaces->Furnace(FurnaceNum).WaterCyclingMode = WaterCycling;
            }
            if (state.dataFurnaces->Furnace(FurnaceNum).WaterCyclingMode == 0) {
                ShowSevereError(state, format("{} illegal {} = {}", CurrentModuleObject, cAlphaFields(18), Alphas(18)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                ErrorsFound = true;
            }

            // end get water flow mode info
            if (Alphas(8) == "COIL:HEATING:WATERTOAIRHEATPUMP:EQUATIONFIT" && Alphas(10) == "COIL:COOLING:WATERTOAIRHEATPUMP:EQUATIONFIT") {
                state.dataFurnaces->Furnace(FurnaceNum).WatertoAirHPType = WatertoAir_Simple;
                SetSimpleWSHPData(state,
                                  state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                  ErrorsFound,
                                  state.dataFurnaces->Furnace(FurnaceNum).WaterCyclingMode,
                                  _,
                                  state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex);
            } else if (Alphas(8) == "COIL:HEATING:WATERTOAIRHEATPUMP:PARAMETERESTIMATION" &&
                       Alphas(10) == "COIL:COOLING:WATERTOAIRHEATPUMP:PARAMETERESTIMATION") {
                state.dataFurnaces->Furnace(FurnaceNum).WatertoAirHPType = WatertoAir_ParEst;
            } else if (Alphas(8) == "COIL:HEATING:WATERTOAIRHEATPUMP:VARIABLESPEEDEQUATIONFIT" &&
                       Alphas(10) == "COIL:COOLING:WATERTOAIRHEATPUMP:VARIABLESPEEDEQUATIONFIT") {
                state.dataFurnaces->Furnace(FurnaceNum).WatertoAirHPType = WatertoAir_VarSpeedEquationFit;
                SetVarSpeedCoilData(state,
                                    state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                    ErrorsFound,
                                    _,
                                    state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex);
            } else {
                ShowContinueError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, "Cooling coil and heating coil should be of same general type");
                ErrorsFound = true;
            }

            // Get supplemental heating coil information

            SuppHeatCoilType = Alphas(12);
            SuppHeatCoilName = Alphas(13);
            state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType = SuppHeatCoilType;
            state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilName = SuppHeatCoilName;
            errFlag = false;
            if (UtilityRoutines::SameString(SuppHeatCoilType, "Coil:Heating:Fuel") ||
                UtilityRoutines::SameString(SuppHeatCoilType, "Coil:Heating:Electric")) {

                state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType_Num =
                    HeatingCoils::GetHeatingCoilTypeNum(state, SuppHeatCoilType, SuppHeatCoilName, errFlag);
                if (errFlag) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else {
                    IsNotOK = false;
                    ValidateComponent(state, SuppHeatCoilType, SuppHeatCoilName, IsNotOK, CurrentModuleObject);
                    if (IsNotOK) {
                        ShowContinueError(state, format("In {} \"{}\"", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;

                    } else { // mine data from the supplemental heating coil

                        HeatingCoils::GetCoilIndex(state, SuppHeatCoilName, state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex, IsNotOK);
                        if (IsNotOK) {
                            ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                            ErrorsFound = true;
                        }

                        // Get the Supplemental Heating Coil Inlet Node Number
                        errFlag = false;
                        SupHeatCoilInletNode = HeatingCoils::GetCoilInletNode(state, SuppHeatCoilType, SuppHeatCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError(state, format("...occurs in {} \"{}\"", CurrentModuleObject, Alphas(1)));
                            ErrorsFound = true;
                        }

                        // Get the Supplemental Heating Coil Outlet Node Number
                        errFlag = false;
                        SupHeatCoilOutletNode = HeatingCoils::GetCoilOutletNode(state, SuppHeatCoilType, SuppHeatCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                            ErrorsFound = true;
                        }

                        // Get the supplemental heating coil design capacity
                        errFlag = false;
                        state.dataFurnaces->Furnace(FurnaceNum).DesignSuppHeatingCapacity =
                            HeatingCoils::GetCoilCapacity(state, SuppHeatCoilType, SuppHeatCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                            ErrorsFound = true;
                        }

                    } // IF (IsNotOK) THEN
                }
            } else if (UtilityRoutines::SameString(SuppHeatCoilType, "Coil:Heating:Water")) {
                state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType_Num = Coil_HeatingWater;
                ValidateComponent(state, SuppHeatCoilType, SuppHeatCoilName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else { // mine data from heating coil object

                    // Get the Heating Coil water Inlet or control Node number
                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).SuppCoilControlNode =
                        GetCoilWaterInletNode(state, "Coil:Heating:Water", SuppHeatCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the ReHeat Coil hot water max volume flow rate
                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow =
                        GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", SuppHeatCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the ReHeat Coil Inlet Node
                    errFlag = false;
                    SupHeatCoilInletNode = WaterCoils::GetCoilInletNode(state, "Coil:Heating:Water", SuppHeatCoilName, errFlag);
                    state.dataFurnaces->Furnace(FurnaceNum).SuppCoilAirInletNode = SupHeatCoilInletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the ReHeat Coil Outlet Node
                    errFlag = false;
                    SupHeatCoilOutletNode = WaterCoils::GetCoilOutletNode(state, "Coil:Heating:Water", SuppHeatCoilName, errFlag);
                    state.dataFurnaces->Furnace(FurnaceNum).SuppCoilAirOutletNode = SupHeatCoilOutletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    errFlag = false;
                    CheckCoilWaterInletNode(state, state.dataFurnaces->Furnace(FurnaceNum).CoilControlNode, errFlag);
                    if (!errFlag) { // then did find a controller so that is bad
                        ShowSevereError(state,
                                        format("{} = {} has a conflicting Controller:WaterCoil object",
                                               CurrentModuleObject,
                                               state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ShowContinueError(state, "Hot water coils are controlled directly by unitary and furnace systems.");
                        ShowContinueError(state, "No water coil controller should be input for the coil.");
                        ErrorsFound = true;
                    }
                }

            } else if (UtilityRoutines::SameString(SuppHeatCoilType, "Coil:Heating:Steam")) {
                state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType_Num = Coil_HeatingSteam;
                ValidateComponent(state, SuppHeatCoilType, SuppHeatCoilName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else { // mine data from heating coil object

                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex =
                        SteamCoils::GetSteamCoilIndex(state, SuppHeatCoilType, SuppHeatCoilName, errFlag);
                    if (state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex == 0) {
                        ShowSevereError(state, format("{} illegal {} = {}", CurrentModuleObject, cAlphaFields(12), SuppHeatCoilName));
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the Heating Coil steam inlet node number
                    errFlag = false;
                    state.dataFurnaces->Furnace(FurnaceNum).SuppCoilControlNode =
                        GetCoilSteamInletNode(state, "Coil:Heating:Steam", SuppHeatCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the Heating Coil steam max volume flow rate
                    state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow =
                        SteamCoils::GetCoilMaxSteamFlowRate(state, state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex, errFlag);
                    if (state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow > 0.0) {
                        SteamIndex = 0; // Function GetSatDensityRefrig will look up steam index if 0 is passed
                        SteamDensity =
                            GetSatDensityRefrig(state, fluidNameSteam, state.dataFurnaces->TempSteamIn, 1.0, SteamIndex, getAirLoopHVACHeatCoolInput);
                        state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow =
                            SteamCoils::GetCoilMaxSteamFlowRate(state, state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex, errFlag) *
                            SteamDensity;
                    }

                    // Get the Heating Coil Inlet Node
                    errFlag = false;
                    SupHeatCoilInletNode =
                        SteamCoils::GetCoilAirInletNode(state, state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex, SuppHeatCoilName, errFlag);
                    state.dataFurnaces->Furnace(FurnaceNum).SuppCoilAirInletNode = SupHeatCoilInletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }

                    // Get the Heating Coil Outlet Node
                    errFlag = false;
                    SupHeatCoilOutletNode =
                        GetCoilAirOutletNode(state, state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex, SuppHeatCoilName, errFlag);
                    state.dataFurnaces->Furnace(FurnaceNum).SuppCoilAirOutletNode = SupHeatCoilOutletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ErrorsFound = true;
                    }
                }

            } else {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(12), Alphas(12)));
                ErrorsFound = true;
            } // IF (Furnace(FurnaceNum)%HeatingCoilType_Num == Coil_HeatingGasOrOtherFuel .OR. &, etc.

            if (lAlphaBlanks(14)) {
                state.dataFurnaces->Furnace(FurnaceNum).CondenserNodeNum = 0;
            } else {
                state.dataFurnaces->Furnace(FurnaceNum).CondenserNodeNum =
                    GetOnlySingleNode(state,
                                      Alphas(14),
                                      ErrorsFound,
                                      DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatPumpWaterToAir,
                                      Alphas(1),
                                      DataLoopNode::NodeFluidType::Air,
                                      DataLoopNode::ConnectionType::OutsideAirReference,
                                      NodeInputManager::CompFluidStream::Primary,
                                      ObjectIsNotParent);
                // need better verification.
                if (!CheckOutAirNodeNumber(state, state.dataFurnaces->Furnace(FurnaceNum).CondenserNodeNum)) {
                    ShowSevereError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format(" Node name of outdoor dry-bulb temperature sensor not valid outdoor air node= {}", Alphas(14)));
                    ShowContinueError(state, "...does not appear in an OutdoorAir:NodeList or as an OutdoorAir:Node.");
                    ErrorsFound = true;
                }
            }

            if (UtilityRoutines::SameString(Alphas(15), "BlowThrough")) state.dataFurnaces->Furnace(FurnaceNum).FanPlace = BlowThru;
            if (UtilityRoutines::SameString(Alphas(15), "DrawThrough")) state.dataFurnaces->Furnace(FurnaceNum).FanPlace = DrawThru;
            if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == 0) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(15), Alphas(15)));
                ErrorsFound = true;
            }

            state.dataFurnaces->Furnace(FurnaceNum).FanSchedPtr = GetScheduleIndex(state, Alphas(16));
            if (!lAlphaBlanks(16) && state.dataFurnaces->Furnace(FurnaceNum).FanSchedPtr == 0) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(16), Alphas(16)));
                ErrorsFound = true;
            } else if (lAlphaBlanks(16)) {
                state.dataFurnaces->Furnace(FurnaceNum).OpMode = CycFanCycCoil;
                if (state.dataFurnaces->Furnace(FurnaceNum).FanType_Num != FanType_SimpleOnOff) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                    ShowContinueError(state, format("{} = {}", cAlphaFields(6), Alphas(6)));
                    ShowContinueError(state, format("Fan type must be Fan:OnOff when {} = Blank.", cAlphaFields(16)));
                    ErrorsFound = true;
                }
            }

            // add the Dehumidification Type
            if (UtilityRoutines::SameString(Alphas(17), "None") || UtilityRoutines::SameString(Alphas(17), "CoolReheat")) {
                AirNodeFound = false;
                if (UtilityRoutines::SameString(Alphas(17), "CoolReheat")) {
                    state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num = DehumidificationControlMode::CoolReheat;
                    state.dataFurnaces->Furnace(FurnaceNum).Humidistat = true;
                    if (lAlphaBlanks(17)) {
                        ShowWarningError(state, format("{} \"{}\"", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state,
                                          "Dehumidification control type is assumed to be None since a supplemental reheat coil has not been "
                                          "specified and the simulation continues.");
                        state.dataFurnaces->Furnace(FurnaceNum).Humidistat = false;
                        state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num = DehumidificationControlMode::None;
                    }
                }
                if (UtilityRoutines::SameString(Alphas(17), "None")) {
                    state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num = DehumidificationControlMode::None;
                    state.dataFurnaces->Furnace(FurnaceNum).Humidistat = false;
                }
                if (state.dataFurnaces->Furnace(FurnaceNum).Humidistat) {
                    for (HStatZoneNum = 1; HStatZoneNum <= state.dataZoneCtrls->NumHumidityControlZones; ++HStatZoneNum) {
                        if (state.dataZoneCtrls->HumidityControlZone(HStatZoneNum).ActualZoneNum !=
                            state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                            continue;
                        AirNodeFound = true;
                    }
                    if (!AirNodeFound) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state, "Did not find Air Node (Zone with Humidistat).");
                        ShowContinueError(state, format("Specified {} = {}", cAlphaFields(5), Alphas(5)));
                        ErrorsFound = true;
                    }
                }
            } else { // invalid input or blank
                if (!lAlphaBlanks(17)) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(17), Alphas(17)));
                    ErrorsFound = true;
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).Humidistat = false;
                    state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num = DehumidificationControlMode::None;
                }
            }

            // Add fan to component sets array

            if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == BlowThru) {
                CompSetFanInlet = Alphas(3);
                CompSetCoolInlet = "UNDEFINED";
                if (FanInletNode != state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum) {
                    ShowSevereError(
                        state, format("For {} = {}, Mismatch between unitary system inlet node and fan inlet node.", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, "..For \"BlowThrough\" fan, the inlet node name for the HeatPump should match the fan inlet node name.");
                    ShowContinueError(state,
                                      format("..HeatPump Inlet Node = {}",
                                             state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum)));
                    ShowContinueError(state, format("..Fan Inlet Node      = {}", state.dataLoopNodes->NodeID(FanInletNode)));
                    ErrorsFound = true;
                }
                if (FanOutletNode != CoolingCoilInletNode) {
                    ShowSevereError(
                        state, format("For {} = {}, Mismatch between fan outlet node and cooling coil inlet node.", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, "..For \"BlowThrough\" fan, the fan outlet node name must match the cooling coil inlet node name.");
                    ShowContinueError(state, format("..Fan outlet node         = {}", state.dataLoopNodes->NodeID(FanOutletNode)));
                    ShowContinueError(state, format("..Cooling coil inlet node = {}", state.dataLoopNodes->NodeID(CoolingCoilInletNode)));
                    ErrorsFound = true;
                }
                if (CoolingCoilOutletNode != HeatingCoilInletNode) {
                    ShowSevereError(state,
                                    format("For {} = {}, Mismatch between cooling coil outlet node and heating coil inlet node.",
                                           CurrentModuleObject,
                                           Alphas(1)));
                    ShowContinueError(state, "..The cooling coil outlet node name must match the heating coil inlet node name.");
                    ShowContinueError(state, format("..Cooling coil outlet node = {}", state.dataLoopNodes->NodeID(CoolingCoilOutletNode)));
                    ShowContinueError(state, format("..Heating coil inlet node  = {}", state.dataLoopNodes->NodeID(HeatingCoilInletNode)));
                    ErrorsFound = true;
                }
                if (HeatingCoilOutletNode != SupHeatCoilInletNode) {
                    ShowSevereError(state,
                                    format("For {} = {}, Mismatch between heating coil outlet node and supplemental heating coil inlet node.",
                                           CurrentModuleObject,
                                           Alphas(1)));
                    ShowContinueError(
                        state,
                        "..For \"BlowThrough\" fan, the heating coil outlet node name must match the supplemental heating coil inlet node name.");
                    ShowContinueError(state,
                                      format("..Heating coil outlet node             = {}", state.dataLoopNodes->NodeID(HeatingCoilOutletNode)));
                    ShowContinueError(state,
                                      format("..Supplemental heating coil inlet node = {}", state.dataLoopNodes->NodeID(SupHeatCoilInletNode)));
                    ErrorsFound = true;
                }
                if (SupHeatCoilOutletNode != state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum) {
                    ShowSevereError(state,
                                    format("For {} = {}, Mismatch between supplemental heating coil outlet node and HeatPump outlet node.",
                                           CurrentModuleObject,
                                           Alphas(1)));
                    ShowContinueError(state, "..The supplemental heating coil outlet node name must match the HeatPump outlet node name.");
                    ShowContinueError(state,
                                      format("..Supplemental heating coil outlet node = {}", state.dataLoopNodes->NodeID(SupHeatCoilOutletNode)));
                    ShowContinueError(state,
                                      format("..HeatPump outlet node                  = {}",
                                             state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum)));
                    ErrorsFound = true;
                }
            } else {
                CompSetFanInlet = "UNDEFINED";
                CompSetCoolInlet = Alphas(3);
                if (CoolingCoilInletNode != state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum) {
                    ShowSevereError(state,
                                    format("For {} = {}, Mismatch between unitary system inlet node and cooling coil inlet node.",
                                           CurrentModuleObject,
                                           Alphas(1)));
                    ShowContinueError(
                        state, "..For \"DrawThrough\" fan, the inlet node name for the HeatPump should match the cooling coil inlet node name.");
                    ShowContinueError(state,
                                      format("..HeatPump inlet node     = {}",
                                             state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum)));
                    ShowContinueError(state, format("..Cooling coil inlet node = {}", state.dataLoopNodes->NodeID(CoolingCoilInletNode)));
                    ErrorsFound = true;
                }
                if (CoolingCoilOutletNode != HeatingCoilInletNode) {
                    ShowSevereError(state,
                                    format("For {} = {}, Mismatch between cooling coil outlet node and heating coil inlet node.",
                                           CurrentModuleObject,
                                           Alphas(1)));
                    ShowContinueError(state, "..The outlet node name for the cooling coil should match the heating coil inlet node name.");
                    ShowContinueError(state, format("..Cooling coil outlet node = {}", state.dataLoopNodes->NodeID(CoolingCoilOutletNode)));
                    ShowContinueError(state, format("..Heating coil inlet node  = {}", state.dataLoopNodes->NodeID(HeatingCoilInletNode)));
                    ErrorsFound = true;
                }
                if (HeatingCoilOutletNode != FanInletNode) {
                    ShowSevereError(
                        state, format("For {} = {}, Mismatch between heating coil outlet node and fan inlet node.", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state,
                                      "..For \"DrawThrough\" fan, the outlet node name for the heating coil should match the fan inlet node name.");
                    ShowContinueError(state, format("..Heating coil outlet node = {}", state.dataLoopNodes->NodeID(HeatingCoilOutletNode)));
                    ShowContinueError(state, format("..Fan inlet node           = {}", state.dataLoopNodes->NodeID(FanInletNode)));
                    ErrorsFound = true;
                }
                if (FanOutletNode != SupHeatCoilInletNode) {
                    ShowSevereError(state,
                                    format("For {} = {}, Mismatch between fan outlet node and supplemental heating coil inlet node.",
                                           CurrentModuleObject,
                                           Alphas(1)));
                    ShowContinueError(
                        state,
                        "..For \"DrawThrough\" fan, the outlet node name for the fan should match the supplemental heating coil inlet node name.");
                    ShowContinueError(state, format("..Fan outlet node                      = {}", state.dataLoopNodes->NodeID(FanOutletNode)));
                    ShowContinueError(state,
                                      format("..Supplemental heating coil inlet node = {}", state.dataLoopNodes->NodeID(SupHeatCoilInletNode)));
                    ErrorsFound = true;
                }
                if (SupHeatCoilOutletNode != state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum) {
                    ShowSevereError(state,
                                    format("For {} = {}, Mismatch between supplemental heating coil outlet node and HeatPump outlet node.",
                                           CurrentModuleObject,
                                           Alphas(1)));
                    ShowContinueError(state, "..The supplemental heating coil outlet node name must match the HeatPump outlet node name.");
                    ShowContinueError(state,
                                      format("..Supplemental heating coil outlet node = {}", state.dataLoopNodes->NodeID(SupHeatCoilOutletNode)));
                    ShowContinueError(state,
                                      format("..HeatPump outlet node                  = {}",
                                             state.dataLoopNodes->NodeID(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum)));
                    ErrorsFound = true;
                }
            }
            //  (Set up validation here for the fan or cooling coil inlet?)
            SetUpCompSets(state, CurrentModuleObject, Alphas(1), Alphas(6), Alphas(7), CompSetFanInlet, "UNDEFINED");

            // Add DX heating coil to component sets array
            SetUpCompSets(state, CurrentModuleObject, Alphas(1), Alphas(8), Alphas(9), "UNDEFINED", "UNDEFINED");

            // Add DX cooling coil to component sets array
            SetUpCompSets(state, CurrentModuleObject, Alphas(1), Alphas(10), Alphas(11), CompSetCoolInlet, "UNDEFINED");

            // Add supplemental heating coil to component sets array
            SetUpCompSets(state, CurrentModuleObject, Alphas(1), Alphas(12), Alphas(13), "UNDEFINED", Alphas(4));

            // Set the Design Fan Volume Flow Rate
            errFlag = false;
            FanVolFlowRate = GetFanDesignVolumeFlowRate(state, FanType, FanName, errFlag);
            state.dataFurnaces->Furnace(FurnaceNum).ActualFanVolFlowRate = FanVolFlowRate;
            if (errFlag) {
                ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                ErrorsFound = true;
            }

            // CR8094 - simple water to air heat pump MUST operate at the same flow rate specified in the coil objects
            //        Furnace(FurnaceNum)%DesignFanVolFlowRate = Numbers(1)
            //        Furnace(FurnaceNum)%MaxHeatAirVolFlow    = Furnace(FurnaceNum)%DesignFanVolFlowRate
            //        Furnace(FurnaceNum)%MaxCoolAirVolFlow    = Furnace(FurnaceNum)%DesignFanVolFlowRate

            // parameter estimate model only specifies air flow rate in parent object
            if (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingWaterToAirHP) {
                state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow = Numbers(1);
                state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow = Numbers(1);
                // simple HP model specifies air flow rate in both the parent and child coils. Use coil air flow rates.
                // simple HP model air flow rate input will not be used.
            } else if (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingWaterToAirHPSimple) {
                errFlag = false;
                state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow =
                    WaterToAirHeatPumpSimple::GetCoilAirFlowRate(state, HeatingCoilType, HeatingCoilName, errFlag);
                state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow =
                    WaterToAirHeatPumpSimple::GetCoilAirFlowRate(state, CoolingCoilType, CoolingCoilName, errFlag);
                if (errFlag) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                }
            } else if (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingWaterToAirHPVSEquationFit) {
                errFlag = false;
                state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow =
                    GetCoilAirFlowRateVariableSpeed(state, HeatingCoilType, HeatingCoilName, errFlag);
                state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow =
                    GetCoilAirFlowRateVariableSpeed(state, CoolingCoilType, CoolingCoilName, errFlag);
                if (errFlag) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                }
            }

            state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlow =
                min(state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow, state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow);
            if (state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow != DataSizing::AutoSize &&
                state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow != DataSizing::AutoSize) {
                state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate =
                    max(state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow, state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow);
            } else {
                state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate = DataSizing::AutoSize;
            }

            state.dataFurnaces->Furnace(FurnaceNum).AirFlowControl = AirFlowControlConstFan::UseCompressorOnFlow;

            if (FanVolFlowRate != DataSizing::AutoSize && state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate != DataSizing::AutoSize) {
                if (state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate > FanVolFlowRate) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, "... has a Cooling or Heating Air Flow Rate > Max Fan Volume Flow Rate, should be <=.");
                    ShowContinueError(state,
                                      format("... Entered value={:.2R}... Fan [{}:{}] Max Value={:.2R}",
                                             state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate,
                                             FanType,
                                             FanName,
                                             FanVolFlowRate));
                }
            }
            if (FanVolFlowRate != DataSizing::AutoSize && state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate != DataSizing::AutoSize) {
                if (state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate <= 0.0) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, "... has a Design Fan Flow Rate <= 0.0, it must be >0.0");
                    ShowContinueError(state, format("... Entered value={:.2R}", state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate));
                    ErrorsFound = true;
                }
            }

            // Set the heat pump heating coil capacity
            //  Get from coil module.
            if (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingWaterToAirHP) {
                errFlag = false;
                state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity =
                    WaterToAirHeatPump::GetCoilCapacity(state, HeatingCoilType, HeatingCoilName, errFlag);
                if (errFlag) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                }
            } else if (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingWaterToAirHPSimple) {
                errFlag = false;
                state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity =
                    WaterToAirHeatPumpSimple::GetCoilCapacity(state, HeatingCoilType, HeatingCoilName, errFlag);
                if (errFlag) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                }
            } else if (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingWaterToAirHPVSEquationFit) {
                errFlag = false;
                state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity =
                    GetCoilCapacityVariableSpeed(state, HeatingCoilType, HeatingCoilName, errFlag);
                if (errFlag) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                }
            }
            // Set the heat pump heating coil convergence
            state.dataFurnaces->Furnace(FurnaceNum).HeatingConvergenceTolerance = Numbers(2);
            // Set the heat pump cooling coil capacity (Total capacity)
            //  Get from coil module.
            if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == Coil_CoolingWaterToAirHP) {
                errFlag = false;
                state.dataFurnaces->Furnace(FurnaceNum).DesignCoolingCapacity =
                    WaterToAirHeatPump::GetCoilCapacity(state, CoolingCoilType, CoolingCoilName, errFlag);
                if (errFlag) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                }
            } else if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == Coil_CoolingWaterToAirHPSimple) {
                errFlag = false;
                state.dataFurnaces->Furnace(FurnaceNum).DesignCoolingCapacity =
                    WaterToAirHeatPumpSimple::GetCoilCapacity(state, CoolingCoilType, CoolingCoilName, errFlag);
                if (errFlag) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                }
            } else if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == Coil_CoolingWaterToAirHPVSEquationFit) {
                errFlag = false;
                state.dataFurnaces->Furnace(FurnaceNum).DesignCoolingCapacity =
                    GetCoilCapacityVariableSpeed(state, CoolingCoilType, CoolingCoilName, errFlag);
                if (errFlag) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                }
            }
            // Set the heat pump cooling coil convergence
            state.dataFurnaces->Furnace(FurnaceNum).CoolingConvergenceTolerance = Numbers(3);
            // Set the heatpump cycling rate
            state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour = Numbers(4);

            // Set the heat pump time constant
            state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant = Numbers(5);

            // Set the heat pump on-cycle power use fraction
            state.dataFurnaces->Furnace(FurnaceNum).OnCyclePowerFraction = Numbers(6);

            // Set the heat pump fan delay time
            state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime = Numbers(7);

            // Set the heatpump design supplemental heating capacity
            //  Get from coil module.

            // Set the heatpump max outlet temperature
            state.dataFurnaces->Furnace(FurnaceNum).DesignMaxOutletTemp = Numbers(8);

            // Set maximum supply air temperature for supplemental heating coil
            state.dataFurnaces->Furnace(FurnaceNum).MaxOATSuppHeat = Numbers(9);

            // set minimum outdoor temperature for compressor operation
            SetMinOATCompressor(state, FurnaceNum, cCurrentModuleObject, ErrorsFound);

        } // End of the Unitary System WaterToAirHeatPump Loop

        Alphas.deallocate();
        Numbers.deallocate();

        if (ErrorsFound) {
            ShowFatalError(state, "Errors found in getting Furnace or Unitary System input.");
        }

        for (HeatOnlyNum = 1; HeatOnlyNum <= NumHeatOnly; ++HeatOnlyNum) {
            FurnaceNum = HeatOnlyNum;
            // Setup Report variables for the Furnace that are not reported in the components themselves
            SetupOutputVariable(state,
                                "Unitary System Fan Part Load Ratio",
                                OutputProcessor::Unit::None,
                                state.dataFurnaces->Furnace(FurnaceNum).FanPartLoadRatio,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataFurnaces->Furnace(FurnaceNum).Name);
            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSActuator(state,
                                 "AirLoopHVAC:Unitary:Furnace:HeatOnly",
                                 state.dataFurnaces->Furnace(FurnaceNum).Name,
                                 "Autosized Supply Air Flow Rate",
                                 "[m3/s]",
                                 state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRateEMSOverrideOn,
                                 state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRateEMSOverrideValue);
            }
        }

        for (UnitaryHeatOnlyNum = NumHeatOnly + 1; UnitaryHeatOnlyNum <= NumHeatOnly + NumUnitaryHeatOnly; ++UnitaryHeatOnlyNum) {
            FurnaceNum = UnitaryHeatOnlyNum;
            // Setup Report variables for Unitary System that are not reported in the components themselves
            SetupOutputVariable(state,
                                "Unitary System Fan Part Load Ratio",
                                OutputProcessor::Unit::None,
                                state.dataFurnaces->Furnace(FurnaceNum).FanPartLoadRatio,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataFurnaces->Furnace(FurnaceNum).Name);
            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSActuator(state,
                                 "AirLoopHVAC:UnitaryHeatOnly",
                                 state.dataFurnaces->Furnace(FurnaceNum).Name,
                                 "Autosized Supply Air Flow Rate",
                                 "[m3/s]",
                                 state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRateEMSOverrideOn,
                                 state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRateEMSOverrideValue);
            }
        }

        for (HeatCoolNum = NumHeatOnly + NumUnitaryHeatOnly + 1; HeatCoolNum <= NumHeatOnly + NumUnitaryHeatOnly + NumHeatCool; ++HeatCoolNum) {
            FurnaceNum = HeatCoolNum;
            // Setup Report variables for the Furnace that are not reported in the components themselves
            SetupOutputVariable(state,
                                "Unitary System Fan Part Load Ratio",
                                OutputProcessor::Unit::None,
                                state.dataFurnaces->Furnace(FurnaceNum).FanPartLoadRatio,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataFurnaces->Furnace(FurnaceNum).Name);
            SetupOutputVariable(state,
                                "Unitary System Compressor Part Load Ratio",
                                OutputProcessor::Unit::None,
                                state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataFurnaces->Furnace(FurnaceNum).Name);

            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSActuator(state,
                                 "AirLoopHVAC:Unitary:Furnace:HeatCool",
                                 state.dataFurnaces->Furnace(FurnaceNum).Name,
                                 "Autosized Supply Air Flow Rate",
                                 "[m3/s]",
                                 state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRateEMSOverrideOn,
                                 state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRateEMSOverrideValue);
                SetupEMSActuator(state,
                                 "AirLoopHVAC:Unitary:Furnace:HeatCool",
                                 state.dataFurnaces->Furnace(FurnaceNum).Name,
                                 "Autosized Supply Air Flow Rate During Cooling Operation",
                                 "[m3/s]",
                                 state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlowEMSOverrideOn,
                                 state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlowEMSOverrideValue);
                SetupEMSActuator(state,
                                 "AirLoopHVAC:Unitary:Furnace:HeatCool",
                                 state.dataFurnaces->Furnace(FurnaceNum).Name,
                                 "Autosized Supply Air Flow Rate During Heating Operation",
                                 "[m3/s]",
                                 state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlowEMSOverrideOn,
                                 state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlowEMSOverrideValue);
                SetupEMSActuator(state,
                                 "AirLoopHVAC:Unitary:Furnace:HeatCool",
                                 state.dataFurnaces->Furnace(FurnaceNum).Name,
                                 "Autosized Supply Air Flow Rate During No Heating or Cooling Operation",
                                 "[m3/s]",
                                 state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlowEMSOverrideOn,
                                 state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlowEMSOverrideValue);
            }
        }

        for (UnitaryHeatCoolNum = NumHeatOnly + NumHeatCool + NumUnitaryHeatOnly + 1;
             UnitaryHeatCoolNum <= NumHeatOnly + NumHeatCool + NumUnitaryHeatOnly + NumUnitaryHeatCool;
             ++UnitaryHeatCoolNum) {
            FurnaceNum = UnitaryHeatCoolNum;
            // Setup Report variables for Unitary System that are not reported in the components themselves
            SetupOutputVariable(state,
                                "Unitary System Fan Part Load Ratio",
                                OutputProcessor::Unit::None,
                                state.dataFurnaces->Furnace(FurnaceNum).FanPartLoadRatio,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataFurnaces->Furnace(FurnaceNum).Name);
            SetupOutputVariable(state,
                                "Unitary System Compressor Part Load Ratio",
                                OutputProcessor::Unit::None,
                                state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataFurnaces->Furnace(FurnaceNum).Name);
            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSActuator(state,
                                 "AirLoopHVAC:UnitaryHeatCool",
                                 state.dataFurnaces->Furnace(FurnaceNum).Name,
                                 "Autosized Supply Air Flow Rate",
                                 "[m3/s]",
                                 state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRateEMSOverrideOn,
                                 state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRateEMSOverrideValue);
                SetupEMSActuator(state,
                                 "AirLoopHVAC:UnitaryHeatCool",
                                 state.dataFurnaces->Furnace(FurnaceNum).Name,
                                 "Autosized Supply Air Flow Rate During Cooling Operation",
                                 "[m3/s]",
                                 state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlowEMSOverrideOn,
                                 state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlowEMSOverrideValue);
                SetupEMSActuator(state,
                                 "AirLoopHVAC:UnitaryHeatCool",
                                 state.dataFurnaces->Furnace(FurnaceNum).Name,
                                 "Autosized Supply Air Flow Rate During Heating Operation",
                                 "[m3/s]",
                                 state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlowEMSOverrideOn,
                                 state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlowEMSOverrideValue);
                SetupEMSActuator(state,
                                 "AirLoopHVAC:UnitaryHeatCool",
                                 state.dataFurnaces->Furnace(FurnaceNum).Name,
                                 "Autosized Supply Air Flow Rate During No Heating or Cooling Operation",
                                 "[m3/s]",
                                 state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlowEMSOverrideOn,
                                 state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlowEMSOverrideValue);
            }
        }

        for (HeatPumpNum = NumHeatOnly + NumHeatCool + NumUnitaryHeatOnly + NumUnitaryHeatCool + 1;
             HeatPumpNum <= state.dataFurnaces->NumFurnaces - NumWaterToAirHeatPump;
             ++HeatPumpNum) {
            FurnaceNum = HeatPumpNum;
            // Setup Report variables for Unitary System that are not reported in the components themselves
            SetupOutputVariable(state,
                                "Unitary System Fan Part Load Ratio",
                                OutputProcessor::Unit::None,
                                state.dataFurnaces->Furnace(FurnaceNum).FanPartLoadRatio,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataFurnaces->Furnace(FurnaceNum).Name);
            SetupOutputVariable(state,
                                "Unitary System Compressor Part Load Ratio",
                                OutputProcessor::Unit::None,
                                state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataFurnaces->Furnace(FurnaceNum).Name);
            SetupOutputVariable(state,
                                "Unitary System Dehumidification Induced Heating Demand Rate",
                                OutputProcessor::Unit::W,
                                state.dataFurnaces->Furnace(FurnaceNum).DehumidInducedHeatingDemandRate,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataFurnaces->Furnace(FurnaceNum).Name);

            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSActuator(state,
                                 "AirLoopHVAC:UnitaryHeatPump:AirToAir",
                                 state.dataFurnaces->Furnace(FurnaceNum).Name,
                                 "Autosized Supply Air Flow Rate",
                                 "[m3/s]",
                                 state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRateEMSOverrideOn,
                                 state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRateEMSOverrideValue);
            }
        }

        for (HeatPumpNum = NumHeatOnly + NumHeatCool + NumUnitaryHeatOnly + NumUnitaryHeatCool + NumHeatPump + 1;
             HeatPumpNum <= state.dataFurnaces->NumFurnaces;
             ++HeatPumpNum) {
            FurnaceNum = HeatPumpNum;
            // Setup Report variables for Unitary System that are not reported in the components themselves
            SetupOutputVariable(state,
                                "Unitary System Fan Part Load Ratio",
                                OutputProcessor::Unit::None,
                                state.dataFurnaces->Furnace(FurnaceNum).FanPartLoadRatio,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataFurnaces->Furnace(FurnaceNum).Name);
            SetupOutputVariable(state,
                                "Unitary System Compressor Part Load Ratio",
                                OutputProcessor::Unit::None,
                                state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataFurnaces->Furnace(FurnaceNum).Name);
            SetupOutputVariable(state,
                                "Unitary System Requested Sensible Cooling Rate",
                                OutputProcessor::Unit::W,
                                state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataFurnaces->Furnace(FurnaceNum).Name);
            SetupOutputVariable(state,
                                "Unitary System Requested Latent Cooling Rate",
                                OutputProcessor::Unit::W,
                                state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilLatentDemand,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataFurnaces->Furnace(FurnaceNum).Name);
            SetupOutputVariable(state,
                                "Unitary System Requested Heating Rate",
                                OutputProcessor::Unit::W,
                                state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataFurnaces->Furnace(FurnaceNum).Name);
            SetupOutputVariable(state,
                                "Unitary System Dehumidification Induced Heating Demand Rate",
                                OutputProcessor::Unit::W,
                                state.dataFurnaces->Furnace(FurnaceNum).DehumidInducedHeatingDemandRate,
                                OutputProcessor::SOVTimeStepType::System,
                                OutputProcessor::SOVStoreType::Average,
                                state.dataFurnaces->Furnace(FurnaceNum).Name);

            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSActuator(state,
                                 "AirLoopHVAC:UnitaryHeatPump:WaterToAir",
                                 state.dataFurnaces->Furnace(FurnaceNum).Name,
                                 "Autosized Supply Air Flow Rate",
                                 "[m3/s]",
                                 state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRateEMSOverrideOn,
                                 state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRateEMSOverrideValue);
            }
        }

        if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
            for (FurnaceNum = 1; FurnaceNum <= state.dataFurnaces->NumFurnaces; ++FurnaceNum) {
                SetupEMSInternalVariable(state,
                                         "Unitary HVAC Design Heating Capacity",
                                         state.dataFurnaces->Furnace(FurnaceNum).Name,
                                         "[W]",
                                         state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity);
                SetupEMSInternalVariable(state,
                                         "Unitary HVAC Design Cooling Capacity",
                                         state.dataFurnaces->Furnace(FurnaceNum).Name,
                                         "[W]",
                                         state.dataFurnaces->Furnace(FurnaceNum).DesignCoolingCapacity);
                SetupEMSActuator(state,
                                 "Unitary HVAC",
                                 state.dataFurnaces->Furnace(FurnaceNum).Name,
                                 "Sensible Load Request",
                                 "[W]",
                                 state.dataFurnaces->Furnace(FurnaceNum).EMSOverrideSensZoneLoadRequest,
                                 state.dataFurnaces->Furnace(FurnaceNum).EMSSensibleZoneLoadValue);
                SetupEMSActuator(state,
                                 "Unitary HVAC",
                                 state.dataFurnaces->Furnace(FurnaceNum).Name,
                                 "Moisture Load Request",
                                 "[W]",
                                 state.dataFurnaces->Furnace(FurnaceNum).EMSOverrideMoistZoneLoadRequest,
                                 state.dataFurnaces->Furnace(FurnaceNum).EMSMoistureZoneLoadValue);
            }
        }
        bool anyRan;
        ManageEMS(state, EMSManager::EMSCallFrom::ComponentGetInput, anyRan, ObjexxFCL::Optional_int_const());
    }

    // End of Get Input subroutines for this Module
    //******************************************************************************

    // Beginning Initialization Section of the Module
    //******************************************************************************

    void InitFurnace(EnergyPlusData &state,
                     int const FurnaceNum,         // index to Furnace
                     int const AirLoopNum,         // index to air loop
                     Real64 &OnOffAirFlowRatio,    // ratio of on to off air mass flow rate
                     int &OpMode,                  // fan operating mode
                     Real64 &ZoneLoad,             // zone sensible load to be met (modified here as needed) (W)
                     Real64 &MoistureLoad,         // zone moisture load (W)
                     bool const FirstHVACIteration // TRUE if first HVAC iteration
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard J. Liesen
        //       DATE WRITTEN   Feb 2001
        //       MODIFIED       Oct 2001, Richard Raustad
        //                      Sep 2008, R. Raustad - revised logic to determine load to be met
        //                      Bereket Nigusse, June 2010 - added a procedure to calculate supply air flow fraction
        //                      through controlled zone
        //                      Bo Shen, March 2012 - for VS WSHP
        //                      Bo Shen, ORNL, July 2012 - added variable-speed air source heat pump cooling and heating coils, using curve-fits

        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the Furnace Components.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.
        // The HeatCool furnace/unitarysystem and air-to-air heat pump may have alternate air flow rates
        // in cooling, heating, and when no cooling or heating is needed. Set up the coil (comp) ON and OFF
        // air flow rates during InitFurnace. Use these flow rates during the Calc routines to set the
        // average mass flow rates based on PLR.

        // REFERENCES:

        // Using/Aliasing
        using Fans::GetFanDesignVolumeFlowRate;
        using Fans::GetFanSpeedRatioCurveIndex;

        using Fans::GetFanVolFlow;
        using FluidProperties::GetDensityGlycol;
        using FluidProperties::GetSatDensityRefrig;
        using PlantUtilities::InitComponentNodes;
        using PlantUtilities::ScanPlantLoopsForObject;
        using PlantUtilities::SetComponentFlowRate;
        using SteamCoils::SimulateSteamCoilComponents;
        using WaterCoils::GetCoilMaxWaterFlowRate;
        using WaterCoils::SimulateWaterCoilComponents;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr Small5WLoad(5.0);
        std::string_view constexpr RoutineName("InitFurnace");

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool errFlag;          // error flag for mining functions
        Real64 FanVolFlowRate; // fan volumetric flow rate (m3/s)
        Real64 QZnReq;         // furnace load based on control zone frac (W)
        Real64 PartLoadRatio;  // furnace part-load ratio
        Real64 SensibleOutput; // no load sensible output (coils off) (W)
        Real64 LatentOutput;   // no load latent output (coils off) (W)
        Real64 QToCoolSetPt;   // sensible load to cooling setpoint (W)
        Real64 QToHeatSetPt;   // sensible load to heating setpoint (W)
        int ZoneInNode;        // Zone inlet node number in the controlled zone
        // calculation (kg/kg)
        Real64 DeltaMassRate; // Difference of mass flow rate between
        // inlet node and system outlet node
        Real64 MassFlowRate; // mass flow rate to calculate loss
        std::string FanType; // used in warning messages
        std::string FanName; // used in warning messages

        int ZoneInSysIndex(0);                            // number of zone inlet nodes counter in an airloop
        int NumAirLoopZones(0);                           // number of zone inlet nodes in an air loop
        int ZoneInletNodeNum(0);                          // zone inlet nodes node number
        Real64 SumOfMassFlowRateMax(0.0);                 // the sum of mass flow rates at inlet to zones in an airloop
        Real64 CntrlZoneTerminalUnitMassFlowRateMax(0.0); // Maximum mass flow rate through controlled zone terminal unit

        bool ErrorsFound(false);                 // flag returned from mining call
        Real64 mdot(0.0);                        // local temporary for mass flow rate (kg/s)
        Real64 rho(0.0);                         // local for fluid density
        int SteamIndex(0);                       // index of steam quality for steam heating coil
        Real64 SteamDensity(0.0);                // density of steam at 100C, used for steam heating coils
        Real64 CoilMaxVolFlowRate(0.0);          // coil fluid maximum volume flow rate
        Real64 QActual(0.0);                     // coil actual capacity
        Real64 SUPHEATERLOAD(0.0);               // SUPPLEMENTAL HEATER LOAD
        int NumOfSpeedCooling;                   // Number of speeds for cooling
        int NumOfSpeedHeating;                   // Number of speeds for heating
        int InNode;                              // Inlet node number in MSHP loop
        int OutNode;                             // Outlet node number in MSHP loop
        Real64 RhoAir;                           // Air density at InNode
        int IHPIndex(0);                         // coil id of IHP coil
        Furnaces::ModeOfOperation OperatingMode; // track cooling, heating, and no cooling or heating modes
        Furnaces::ModeOfOperation OperatingModeMinusOne;
        Furnaces::ModeOfOperation OperatingModeMinusTwo;
        bool Oscillate; // detection of oscillating operating modes

        InNode = state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum;
        OutNode = state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum;

        auto &Node = state.dataLoopNodes->Node;

        if (state.dataFurnaces->InitFurnaceMyOneTimeFlag) {
            // initialize the environment and sizing flags
            state.dataFurnaces->MyEnvrnFlag.allocate(state.dataFurnaces->NumFurnaces);
            state.dataFurnaces->MySizeFlag.allocate(state.dataFurnaces->NumFurnaces);
            state.dataFurnaces->MySecondOneTimeFlag.allocate(state.dataFurnaces->NumFurnaces);
            state.dataFurnaces->MyFanFlag.allocate(state.dataFurnaces->NumFurnaces);
            state.dataFurnaces->MyCheckFlag.allocate(state.dataFurnaces->NumFurnaces);
            state.dataFurnaces->MyFlowFracFlag.allocate(state.dataFurnaces->NumFurnaces);
            state.dataFurnaces->MyPlantScanFlag.allocate(state.dataFurnaces->NumFurnaces);
            state.dataFurnaces->MySuppCoilPlantScanFlag.allocate(state.dataFurnaces->NumFurnaces);
            state.dataFurnaces->MyEnvrnFlag = true;
            state.dataFurnaces->MySizeFlag = true;
            state.dataFurnaces->MySecondOneTimeFlag = true;
            state.dataFurnaces->MyFanFlag = true;
            state.dataFurnaces->MyCheckFlag = true;
            state.dataFurnaces->MyFlowFracFlag = true;
            state.dataFurnaces->InitFurnaceMyOneTimeFlag = false;
            state.dataFurnaces->MyPlantScanFlag = true;
            state.dataFurnaces->MySuppCoilPlantScanFlag = true;
        }

        if (state.dataGlobal->BeginEnvrnFlag && state.dataFurnaces->MyAirLoopPass) {
            state.dataFurnaces->AirLoopPass = 0;
            state.dataFurnaces->MyAirLoopPass = false;
        }
        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataFurnaces->MyAirLoopPass = true;
        }

        ++state.dataFurnaces->AirLoopPass;
        if (state.dataFurnaces->AirLoopPass > 2) state.dataFurnaces->AirLoopPass = 1;

        if (!state.dataGlobal->SysSizingCalc && state.dataFurnaces->MySizeFlag(FurnaceNum)) {
            // for each furnace, do the sizing once.
            SizeFurnace(state, FurnaceNum, FirstHVACIteration);
            state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac = 1.0;

            state.dataFurnaces->MySizeFlag(FurnaceNum) = false;
            // Pass the fan cycling schedule index up to the air loop. Set the air loop unitary system flag.
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).CycFanSchedPtr = state.dataFurnaces->Furnace(FurnaceNum).FanSchedPtr;
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).UnitarySys = true;
            // RR this is wrong, Op mode needs to be updated each time atep
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).FanOpMode = state.dataFurnaces->Furnace(FurnaceNum).OpMode;

            // Check that heat pump heating capacity is within 20% of cooling capacity
            if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_AirToAir) {
                if (std::abs(state.dataFurnaces->Furnace(FurnaceNum).DesignCoolingCapacity -
                             state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity) /
                        state.dataFurnaces->Furnace(FurnaceNum).DesignCoolingCapacity >
                    0.2) {
                    ShowWarningError(state,
                                     format("{} \"{}\" heating capacity is disproportionate (> 20% different) to total cooling capacity",
                                            cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                            state.dataFurnaces->Furnace(FurnaceNum).Name));
                }
            }
        }

        if (!state.dataGlobal->DoingSizing && state.dataFurnaces->MySecondOneTimeFlag(FurnaceNum)) {
            // sizing all done.  check fan air flow rates
            errFlag = false;
            FanVolFlowRate = GetFanDesignVolumeFlowRate(state, BlankString, BlankString, errFlag, state.dataFurnaces->Furnace(FurnaceNum).FanIndex);
            state.dataFurnaces->Furnace(FurnaceNum).ActualFanVolFlowRate = FanVolFlowRate;
            if (errFlag) {
                ShowContinueError(state,
                                  format("...occurs in {} ={}",
                                         cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                         state.dataFurnaces->Furnace(FurnaceNum).Name));
            }
            if (FanVolFlowRate != DataSizing::AutoSize) {
                if (state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate > FanVolFlowRate) {
                    ShowWarningError(state,
                                     format("{}={} has a Design Fan Volume Flow Rate > Max Fan Volume Flow Rate, should be <=",
                                            cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                            state.dataFurnaces->Furnace(FurnaceNum).Name));
                    ShowContinueError(state,
                                      format("... Entered value={:.2R}... Fan [{}] Max Value={:.2R}",
                                             state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate,
                                             cFanTypes(state.dataFurnaces->Furnace(FurnaceNum).FanType_Num),
                                             FanVolFlowRate));
                }
                if (state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate <= 0.0) {
                    ShowSevereError(state,
                                    format("{}={} has a Design Fan Volume Flow Rate <= 0.0, it must be >0.0",
                                           cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                           state.dataFurnaces->Furnace(FurnaceNum).Name));
                    ShowContinueError(state, format("... Entered value={:.2R}", state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate));
                }

                state.dataFurnaces->MySecondOneTimeFlag(FurnaceNum) = false;
            }
        }

        // Scan hot water and steam heating coil plant components for one time initializations
        if (state.dataFurnaces->MyPlantScanFlag(FurnaceNum) && allocated(state.dataPlnt->PlantLoop)) {
            if ((state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingWater) ||
                (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingSteam)) {

                if (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingWater) {

                    errFlag = false;
                    ScanPlantLoopsForObject(state,
                                            state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilName,
                                            DataPlant::PlantEquipmentType::CoilWaterSimpleHeating,
                                            state.dataFurnaces->Furnace(FurnaceNum).plantLoc,
                                            errFlag,
                                            _,
                                            _,
                                            _,
                                            _,
                                            _);
                    if (errFlag) {
                        ShowFatalError(state, "InitFurnace: Program terminated for previous conditions.");
                    }
                    state.dataFurnaces->Furnace(FurnaceNum).MaxHeatCoilFluidFlow =
                        GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilName, ErrorsFound);
                    if (state.dataFurnaces->Furnace(FurnaceNum).MaxHeatCoilFluidFlow > 0.0) {
                        rho = GetDensityGlycol(state,
                                               state.dataPlnt->PlantLoop(state.dataFurnaces->Furnace(FurnaceNum).plantLoc.loopNum).FluidName,
                                               DataGlobalConstants::HWInitConvTemp,
                                               state.dataPlnt->PlantLoop(state.dataFurnaces->Furnace(FurnaceNum).plantLoc.loopNum).FluidIndex,
                                               RoutineName);
                        state.dataFurnaces->Furnace(FurnaceNum).MaxHeatCoilFluidFlow *= rho;
                    }
                } else if (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingSteam) {

                    errFlag = false;
                    ScanPlantLoopsForObject(state,
                                            state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilName,
                                            DataPlant::PlantEquipmentType::CoilSteamAirHeating,
                                            state.dataFurnaces->Furnace(FurnaceNum).plantLoc,
                                            errFlag,
                                            _,
                                            _,
                                            _,
                                            _,
                                            _);
                    if (errFlag) {
                        ShowFatalError(state, "InitFurnace: Program terminated for previous conditions.");
                    }
                    state.dataFurnaces->Furnace(FurnaceNum).MaxHeatCoilFluidFlow =
                        SteamCoils::GetCoilMaxSteamFlowRate(state, state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex, ErrorsFound);
                    if (state.dataFurnaces->Furnace(FurnaceNum).MaxHeatCoilFluidFlow > 0.0) {
                        SteamIndex = 0; // Function GetSatDensityRefrig will look up steam index if 0 is passed
                        SteamDensity = GetSatDensityRefrig(state, fluidNameSteam, state.dataFurnaces->TempSteamIn, 1.0, SteamIndex, RoutineName);
                        state.dataFurnaces->Furnace(FurnaceNum).MaxHeatCoilFluidFlow *= SteamDensity;
                    }
                }
                // fill outlet node for coil
                state.dataFurnaces->Furnace(FurnaceNum).CoilOutletNode =
                    DataPlant::CompData::getPlantComponent(state, state.dataFurnaces->Furnace(FurnaceNum).plantLoc).NodeNumOut;
                state.dataFurnaces->MyPlantScanFlag(FurnaceNum) = false;
            } else { // pthp not connected to plant
                state.dataFurnaces->MyPlantScanFlag(FurnaceNum) = false;
            }
        } else if (state.dataFurnaces->MyPlantScanFlag(FurnaceNum) && !state.dataGlobal->AnyPlantInModel) {
            state.dataFurnaces->MyPlantScanFlag(FurnaceNum) = false;
        }

        // Scan Supplemental hot water and steam heating coil plant components for one time initializations
        if (state.dataFurnaces->MySuppCoilPlantScanFlag(FurnaceNum) && allocated(state.dataPlnt->PlantLoop)) {
            if ((state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType_Num == Coil_HeatingWater) ||
                (state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType_Num == Coil_HeatingSteam)) {

                if (state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType_Num == Coil_HeatingWater) {
                    errFlag = false;
                    ScanPlantLoopsForObject(state,
                                            state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilName,
                                            DataPlant::PlantEquipmentType::CoilWaterSimpleHeating,
                                            state.dataFurnaces->Furnace(FurnaceNum).SuppPlantLoc,
                                            errFlag,
                                            _,
                                            _,
                                            _,
                                            _,
                                            _);
                    if (errFlag) {
                        ShowFatalError(state, "InitFurnace: Program terminated for previous conditions.");
                    }
                    state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow =
                        GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilName, ErrorsFound);
                    if (state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow > 0.0) {
                        rho = GetDensityGlycol(state,
                                               state.dataPlnt->PlantLoop(state.dataFurnaces->Furnace(FurnaceNum).SuppPlantLoc.loopNum).FluidName,
                                               DataGlobalConstants::HWInitConvTemp,
                                               state.dataPlnt->PlantLoop(state.dataFurnaces->Furnace(FurnaceNum).SuppPlantLoc.loopNum).FluidIndex,
                                               RoutineName);
                        state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow *= rho;
                    }
                } else if (state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType_Num == Coil_HeatingSteam) {
                    errFlag = false;
                    ScanPlantLoopsForObject(state,
                                            state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilName,
                                            DataPlant::PlantEquipmentType::CoilSteamAirHeating,
                                            state.dataFurnaces->Furnace(FurnaceNum).SuppPlantLoc,
                                            errFlag,
                                            _,
                                            _,
                                            _,
                                            _,
                                            _);
                    if (errFlag) {
                        ShowFatalError(state, "InitFurnace: Program terminated for previous conditions.");
                    }
                    state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow =
                        SteamCoils::GetCoilMaxSteamFlowRate(state, state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex, ErrorsFound);
                    if (state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow > 0.0) {
                        SteamIndex = 0; // Function GetSatDensityRefrig will look up steam index if 0 is passed
                        SteamDensity = GetSatDensityRefrig(state, fluidNameSteam, state.dataFurnaces->TempSteamIn, 1.0, SteamIndex, RoutineName);
                        state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow *= SteamDensity;
                    }
                }
                // fill outlet node for coil
                state.dataFurnaces->Furnace(FurnaceNum).SuppCoilOutletNode =
                    DataPlant::CompData::getPlantComponent(state, state.dataFurnaces->Furnace(FurnaceNum).SuppPlantLoc).NodeNumOut;
                state.dataFurnaces->MySuppCoilPlantScanFlag(FurnaceNum) = false;
            } else { // pthp not connected to plant
                state.dataFurnaces->MySuppCoilPlantScanFlag(FurnaceNum) = false;
            }

        } else if (state.dataFurnaces->MySuppCoilPlantScanFlag(FurnaceNum) && !state.dataGlobal->AnyPlantInModel) {
            state.dataFurnaces->MySuppCoilPlantScanFlag(FurnaceNum) = false;
        }

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && state.dataFurnaces->MyEnvrnFlag(FurnaceNum)) {
            // Change the Volume Flow Rates to Mass Flow Rates
            state.dataFurnaces->Furnace(FurnaceNum).DesignMassFlowRate =
                state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate * state.dataEnvrn->StdRhoAir;
            state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirMassFlow =
                state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow * state.dataEnvrn->StdRhoAir;
            state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirMassFlow =
                state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow * state.dataEnvrn->StdRhoAir;
            state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirMassFlow =
                state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlow * state.dataEnvrn->StdRhoAir;
            state.dataFurnaces->Furnace(FurnaceNum).WSHPRuntimeFrac = 0.0;
            state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 0.0;
            state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand = 0.0;
            state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilLatentDemand = 0.0;
            state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand = 0.0;

            state.dataFurnaces->Furnace(FurnaceNum).SenLoadLoss = 0.0;
            if (state.dataFurnaces->Furnace(FurnaceNum).Humidistat) {
                state.dataFurnaces->Furnace(FurnaceNum).LatLoadLoss = 0.0;
            }

            //   set fluid-side hardware limits
            if (state.dataFurnaces->Furnace(FurnaceNum).CoilControlNode > 0) {

                if (state.dataFurnaces->Furnace(FurnaceNum).MaxHeatCoilFluidFlow == DataSizing::AutoSize) {
                    // If water coil max water flow rate is autosized, simulate once in order to mine max flow rate
                    if (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingWater) {
                        SimulateWaterCoilComponents(state,
                                                    state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilName,
                                                    FirstHVACIteration,
                                                    state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex);
                        CoilMaxVolFlowRate = GetCoilMaxWaterFlowRate(
                            state, "Coil:Heating:Water", state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilName, ErrorsFound);
                        if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                            rho = GetDensityGlycol(state,
                                                   state.dataPlnt->PlantLoop(state.dataFurnaces->Furnace(FurnaceNum).plantLoc.loopNum).FluidName,
                                                   DataGlobalConstants::HWInitConvTemp,
                                                   state.dataPlnt->PlantLoop(state.dataFurnaces->Furnace(FurnaceNum).plantLoc.loopNum).FluidIndex,
                                                   RoutineName);
                            state.dataFurnaces->Furnace(FurnaceNum).MaxHeatCoilFluidFlow = CoilMaxVolFlowRate * rho;
                        }
                    }
                    // If steam coil max steam flow rate is autosized, simulate once in order to mine max flow rate
                    if (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingSteam) {
                        SimulateSteamCoilComponents(state,
                                                    state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilName,
                                                    FirstHVACIteration,
                                                    state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex,
                                                    1.0,
                                                    QActual); // QCoilReq, simulate any load > 0 to get max capacity
                        CoilMaxVolFlowRate =
                            SteamCoils::GetCoilMaxSteamFlowRate(state, state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex, ErrorsFound);
                        if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                            SteamIndex = 0; // Function GetSatDensityRefrig will look up steam index if 0 is passed
                            SteamDensity = GetSatDensityRefrig(state, fluidNameSteam, state.dataFurnaces->TempSteamIn, 1.0, SteamIndex, RoutineName);
                            state.dataFurnaces->Furnace(FurnaceNum).MaxHeatCoilFluidFlow = CoilMaxVolFlowRate * SteamDensity;
                        }
                    }
                }

                InitComponentNodes(state,
                                   0.0,
                                   state.dataFurnaces->Furnace(FurnaceNum).MaxHeatCoilFluidFlow,
                                   state.dataFurnaces->Furnace(FurnaceNum).CoilControlNode,
                                   state.dataFurnaces->Furnace(FurnaceNum).CoilOutletNode);
            }
            if (state.dataFurnaces->Furnace(FurnaceNum).SuppCoilControlNode > 0) {
                if (state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow == DataSizing::AutoSize) {
                    if (state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType_Num == Coil_HeatingWater) {
                        // If water coil max water flow rate is autosized, simulate once in order to mine max flow rate
                        SimulateWaterCoilComponents(state,
                                                    state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilName,
                                                    FirstHVACIteration,
                                                    state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex);
                        CoilMaxVolFlowRate = GetCoilMaxWaterFlowRate(
                            state, "Coil:Heating:Water", state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilName, ErrorsFound);
                        if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                            rho = GetDensityGlycol(state,
                                                   state.dataPlnt->PlantLoop(state.dataFurnaces->Furnace(FurnaceNum).SuppPlantLoc.loopNum).FluidName,
                                                   DataGlobalConstants::HWInitConvTemp,
                                                   state.dataPlnt->PlantLoop(state.dataFurnaces->Furnace(FurnaceNum).SuppPlantLoc.loopNum).FluidIndex,
                                                   RoutineName);
                            state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow = CoilMaxVolFlowRate * rho;
                        }
                    }
                    if (state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType_Num == Coil_HeatingSteam) {
                        SimulateSteamCoilComponents(state,
                                                    state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilName,
                                                    FirstHVACIteration,
                                                    state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex,
                                                    1.0,
                                                    QActual); // QCoilReq, simulate any load > 0 to get max capacity
                        CoilMaxVolFlowRate =
                            SteamCoils::GetCoilMaxSteamFlowRate(state, state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex, ErrorsFound);
                        if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                            SteamIndex = 0; // Function GetSatDensityRefrig will look up steam index if 0 is passed
                            SteamDensity = GetSatDensityRefrig(state, fluidNameSteam, state.dataFurnaces->TempSteamIn, 1.0, SteamIndex, RoutineName);
                            state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow = CoilMaxVolFlowRate * SteamDensity;
                        }
                    }
                    InitComponentNodes(state,
                                       0.0,
                                       state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow,
                                       state.dataFurnaces->Furnace(FurnaceNum).SuppCoilControlNode,
                                       state.dataFurnaces->Furnace(FurnaceNum).SuppCoilOutletNode);
                }
            }
            state.dataFurnaces->MyEnvrnFlag(FurnaceNum) = false;
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataFurnaces->MyEnvrnFlag(FurnaceNum) = true;
        }

        if (state.dataFurnaces->MyFanFlag(FurnaceNum)) {
            if (state.dataFurnaces->Furnace(FurnaceNum).ActualFanVolFlowRate != DataSizing::AutoSize) {
                if (state.dataFurnaces->Furnace(FurnaceNum).ActualFanVolFlowRate > 0.0) {
                    state.dataFurnaces->Furnace(FurnaceNum).HeatingSpeedRatio =
                        state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow / state.dataFurnaces->Furnace(FurnaceNum).ActualFanVolFlowRate;
                    state.dataFurnaces->Furnace(FurnaceNum).CoolingSpeedRatio =
                        state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow / state.dataFurnaces->Furnace(FurnaceNum).ActualFanVolFlowRate;
                    state.dataFurnaces->Furnace(FurnaceNum).NoHeatCoolSpeedRatio = state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlow /
                                                                                   state.dataFurnaces->Furnace(FurnaceNum).ActualFanVolFlowRate;
                }
                if (GetFanSpeedRatioCurveIndex(state, FanType, FanName, state.dataFurnaces->Furnace(FurnaceNum).FanIndex) > 0) {
                    if (state.dataFurnaces->Furnace(FurnaceNum).ActualFanVolFlowRate == state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow &&
                        state.dataFurnaces->Furnace(FurnaceNum).ActualFanVolFlowRate == state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow &&
                        state.dataFurnaces->Furnace(FurnaceNum).ActualFanVolFlowRate ==
                            state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlow) {
                        ShowWarningError(state,
                                         format("{} \"{}\"",
                                                cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                                state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ShowContinueError(state, format("...For fan type and name = {} \"{}\"", FanType, FanName));
                        ShowContinueError(state,
                                          "...Fan power ratio function of speed ratio curve has no impact if fan volumetric flow rate is the same as "
                                          "the unitary system volumetric flow rate.");
                        ShowContinueError(state,
                                          format("...Fan volumetric flow rate            = {:.5R} m3/s.",
                                                 state.dataFurnaces->Furnace(FurnaceNum).ActualFanVolFlowRate));
                        ShowContinueError(state,
                                          format("...Unitary system volumetric flow rate = {:.5R} m3/s.",
                                                 state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow));
                    }
                }
                state.dataFurnaces->MyFanFlag(FurnaceNum) = false;
            } else {
                state.dataFurnaces->Furnace(FurnaceNum).ActualFanVolFlowRate =
                    GetFanDesignVolumeFlowRate(state, BlankString, BlankString, errFlag, state.dataFurnaces->Furnace(FurnaceNum).FanIndex);
            }
        }

        if (allocated(state.dataZoneEquip->ZoneEquipConfig) && state.dataFurnaces->MyCheckFlag(FurnaceNum)) {
            int zoneNum = state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum;
            int zoneInlet = state.dataFurnaces->Furnace(FurnaceNum).ZoneInletNode;
            int coolingPriority = 0;
            int heatingPriority = 0;
            // setup furnace zone equipment sequence information based on finding matching air terminal
            if (state.dataZoneEquip->ZoneEquipConfig(zoneNum).EquipListIndex > 0) {
                state.dataZoneEquip->ZoneEquipList(state.dataZoneEquip->ZoneEquipConfig(zoneNum).EquipListIndex)
                    .getPrioritiesForInletNode(state, zoneInlet, coolingPriority, heatingPriority);
                state.dataFurnaces->Furnace(FurnaceNum).ZoneSequenceCoolingNum = coolingPriority;
                state.dataFurnaces->Furnace(FurnaceNum).ZoneSequenceHeatingNum = heatingPriority;
            }
            state.dataFurnaces->MyCheckFlag(FurnaceNum) = false;
            if (state.dataFurnaces->Furnace(FurnaceNum).ZoneSequenceCoolingNum == 0 ||
                state.dataFurnaces->Furnace(FurnaceNum).ZoneSequenceHeatingNum == 0) {
                ShowSevereError(state,
                                format("{} \"{}\": Airloop air terminal in the zone equipment list for zone = {} not found or is not allowed Zone "
                                       "Equipment Cooling or Heating Sequence = 0.",
                                       cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                       state.dataFurnaces->Furnace(FurnaceNum).Name,
                                       state.dataHeatBal->Zone(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum).Name));
                ShowFatalError(state,
                               format("Subroutine InitFurnace: Errors found in getting {} input.  Preceding condition(s) causes termination.",
                                      cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num)));
            }
        }

        // Find the number of zones (zone Inlet Nodes) attached to an air loop from the air loop number
        NumAirLoopZones =
            state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesCooled + state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesHeated;
        if (allocated(state.dataAirLoop->AirToZoneNodeInfo) && state.dataFurnaces->MyFlowFracFlag(FurnaceNum)) {
            state.dataFurnaces->FlowFracFlagReady = true;
            for (ZoneInSysIndex = 1; ZoneInSysIndex <= NumAirLoopZones; ++ZoneInSysIndex) {
                // zone inlet nodes for cooling
                if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesCooled > 0) {
                    if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolInletNodes(ZoneInSysIndex) == -999) {
                        // the data structure for the zones inlet nodes has not been filled
                        state.dataFurnaces->FlowFracFlagReady = false;
                    }
                }
                // zone inlet nodes for heating
                if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesHeated > 0) {
                    if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitHeatInletNodes(ZoneInSysIndex) == -999) {
                        // the data structure for the zones inlet nodes has not been filled
                        state.dataFurnaces->FlowFracFlagReady = false;
                    }
                }
            }
        }

        if (state.dataFurnaces->MyFlowFracFlag(FurnaceNum)) {
            if (allocated(state.dataAirLoop->AirToZoneNodeInfo) && state.dataFurnaces->FlowFracFlagReady) {
                SumOfMassFlowRateMax = 0.0; // initialize the sum of the maximum flows
                for (ZoneInSysIndex = 1; ZoneInSysIndex <= NumAirLoopZones; ++ZoneInSysIndex) {
                    ZoneInletNodeNum = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolInletNodes(ZoneInSysIndex);
                    SumOfMassFlowRateMax += Node(ZoneInletNodeNum).MassFlowRateMax;
                    if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).CoolCtrlZoneNums(ZoneInSysIndex) ==
                        state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum) {
                        CntrlZoneTerminalUnitMassFlowRateMax = Node(ZoneInletNodeNum).MassFlowRateMax;
                    }
                }
                if (SumOfMassFlowRateMax != 0.0) {
                    if (CntrlZoneTerminalUnitMassFlowRateMax >= SmallAirVolFlow) {
                        state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac = CntrlZoneTerminalUnitMassFlowRateMax / SumOfMassFlowRateMax;
                    } else {
                        ShowSevereError(state,
                                        format("{} = {}",
                                               cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                               state.dataFurnaces->Furnace(FurnaceNum).Name));
                        ShowContinueError(state, " The Fraction of Supply Air Flow That Goes Through the Controlling Zone is set to 1.");
                    }
                    BaseSizer::reportSizerOutput(state,
                                                 cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                                 state.dataFurnaces->Furnace(FurnaceNum).Name,
                                                 "Fraction of Supply Air Flow That Goes Through the Controlling Zone",
                                                 state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac);
                    state.dataFurnaces->MyFlowFracFlag(FurnaceNum) = false;
                }
            }
        }

        // Calculate air distribution losses
        if (!FirstHVACIteration && state.dataFurnaces->AirLoopPass == 1) {
            ZoneInNode = state.dataFurnaces->Furnace(FurnaceNum).ZoneInletNode;
            MassFlowRate = Node(ZoneInNode).MassFlowRate / state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac;
            if (state.afn->distribution_simulated) {
                DeltaMassRate = Node(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum).MassFlowRate -
                                Node(ZoneInNode).MassFlowRate / state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac;
                if (DeltaMassRate < 0.0) DeltaMassRate = 0.0;
            } else {
                MassFlowRate = Node(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum).MassFlowRate;
                DeltaMassRate = 0.0;
            }
            Real64 TotalOutput(0.0);         // total output rate, {W}
            Real64 SensibleOutputDelta(0.0); // delta sensible output rate, {W}
            Real64 LatentOutputDelta(0.0);   // delta latent output rate, {W}
            Real64 TotalOutputDelta(0.0);    // delta total output rate, {W}
            CalcZoneSensibleLatentOutput(MassFlowRate,
                                         Node(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum).Temp,
                                         Node(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum).HumRat,
                                         Node(ZoneInNode).Temp,
                                         Node(ZoneInNode).HumRat,
                                         state.dataFurnaces->Furnace(FurnaceNum).SenLoadLoss,
                                         state.dataFurnaces->Furnace(FurnaceNum).LatLoadLoss,
                                         TotalOutput);
            CalcZoneSensibleLatentOutput(DeltaMassRate,
                                         Node(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum).Temp,
                                         Node(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum).HumRat,
                                         Node(state.dataFurnaces->Furnace(FurnaceNum).NodeNumOfControlledZone).Temp,
                                         Node(state.dataFurnaces->Furnace(FurnaceNum).NodeNumOfControlledZone).HumRat,
                                         SensibleOutputDelta,
                                         LatentOutputDelta,
                                         TotalOutputDelta);
            state.dataFurnaces->Furnace(FurnaceNum).SenLoadLoss = state.dataFurnaces->Furnace(FurnaceNum).SenLoadLoss + SensibleOutputDelta;
            if (std::abs(state.dataFurnaces->Furnace(FurnaceNum).SensibleLoadMet) > 0.0) {
                if (std::abs(state.dataFurnaces->Furnace(FurnaceNum).SenLoadLoss / state.dataFurnaces->Furnace(FurnaceNum).SensibleLoadMet) < 0.001)
                    state.dataFurnaces->Furnace(FurnaceNum).SenLoadLoss = 0.0;
            }
            if (state.dataFurnaces->Furnace(FurnaceNum).Humidistat) {
                state.dataFurnaces->Furnace(FurnaceNum).LatLoadLoss = state.dataFurnaces->Furnace(FurnaceNum).LatLoadLoss + LatentOutputDelta;
                if (std::abs(state.dataFurnaces->Furnace(FurnaceNum).LatentLoadMet) > 0.0) {
                    if (std::abs(state.dataFurnaces->Furnace(FurnaceNum).LatLoadLoss / state.dataFurnaces->Furnace(FurnaceNum).LatentLoadMet) < 0.001)
                        state.dataFurnaces->Furnace(FurnaceNum).LatLoadLoss = 0.0;
                }
            }
        }

        if (state.dataFurnaces->Furnace(FurnaceNum).FanSchedPtr > 0) {
            if (GetCurrentScheduleValue(state, state.dataFurnaces->Furnace(FurnaceNum).FanSchedPtr) == 0.0) {
                state.dataFurnaces->Furnace(FurnaceNum).OpMode = CycFanCycCoil;
            } else {
                state.dataFurnaces->Furnace(FurnaceNum).OpMode = ContFanCycCoil;
            }
            if (AirLoopNum > 0) {
                state.dataAirLoop->AirLoopControlInfo(AirLoopNum).FanOpMode = state.dataFurnaces->Furnace(FurnaceNum).OpMode;
            }
        }

        OpMode = state.dataFurnaces->Furnace(FurnaceNum).OpMode;
        state.dataFurnaces->EconomizerFlag = state.dataAirLoop->AirLoopControlInfo(AirLoopNum).EconoActive;

        if (state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac > 0.0) {
            QZnReq = ZoneLoad / state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac;
            MoistureLoad /= state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac;
            ZoneLoad = QZnReq;
        } else {
            QZnReq = ZoneLoad;
        }

        // Original thermostat control logic (works only for cycling fan systems)
        if (QZnReq > SmallLoad && QZnReq > (Small5WLoad / state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac) &&
            !state.dataZoneEnergyDemand->CurDeadBandOrSetback(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)) {
            state.dataFurnaces->HeatingLoad = true;
            state.dataFurnaces->CoolingLoad = false;
        } else if (QZnReq < (-1.0 * SmallLoad) &&
                   std::abs(QZnReq) > (Small5WLoad / state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac) &&
                   !state.dataZoneEnergyDemand->CurDeadBandOrSetback(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)) {
            state.dataFurnaces->HeatingLoad = false;
            state.dataFurnaces->CoolingLoad = true;
        } else {
            state.dataFurnaces->HeatingLoad = false;
            state.dataFurnaces->CoolingLoad = false;
        }

        if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_AirToAir ||
            (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_WaterToAir &&
             (state.dataFurnaces->Furnace(FurnaceNum).WatertoAirHPType == WatertoAir_Simple ||
              state.dataFurnaces->Furnace(FurnaceNum).WatertoAirHPType == WatertoAir_VarSpeedEquationFit))) {
            if (MoistureLoad < 0.0 && state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::CoolReheat) {
                state.dataFurnaces->HPDehumidificationLoadFlag = true;
                state.dataFurnaces->HeatingLoad = false;
                state.dataFurnaces->CoolingLoad = true;
            } else {
                state.dataFurnaces->HPDehumidificationLoadFlag = false;
            }
        }

        // Check for heat only furnace
        if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num != Furnace_HeatOnly &&
            state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num != UnitarySys_HeatOnly) {

            if (GetCurrentScheduleValue(state, state.dataFurnaces->Furnace(FurnaceNum).SchedPtr) > 0.0) {
                if ((state.dataFurnaces->HeatingLoad || state.dataFurnaces->CoolingLoad) ||
                    (state.dataFurnaces->Furnace(FurnaceNum).Humidistat && MoistureLoad < 0.0)) {
                    PartLoadRatio = 1.0;
                } else {
                    PartLoadRatio = 0.0;
                }
            } else {
                PartLoadRatio = 0.0;
            }
        } else {
            PartLoadRatio = 1.0;
        }

        // get current time step operating capacity of water and steam coils
        // (dependent on entering water and steam temperature)
        if (FirstHVACIteration) {
            if (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingWater) {
                // set water-side mass flow rates
                Node(state.dataFurnaces->Furnace(FurnaceNum).HWCoilAirInletNode).MassFlowRate = state.dataFurnaces->CompOnMassFlow;
                mdot = state.dataFurnaces->Furnace(FurnaceNum).MaxHeatCoilFluidFlow;
                SetComponentFlowRate(state,
                                     mdot,
                                     state.dataFurnaces->Furnace(FurnaceNum).CoilControlNode,
                                     state.dataFurnaces->Furnace(FurnaceNum).CoilOutletNode,
                                     state.dataFurnaces->Furnace(FurnaceNum).plantLoc);
                //     simulate water coil to find operating capacity
                SimulateWaterCoilComponents(state,
                                            state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilName,
                                            FirstHVACIteration,
                                            state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex,
                                            QActual);
                state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity = QActual;

            } // from IF(state.dataFurnaces->Furnace(FurnaceNum)%HeatingCoilType_Num == Coil_HeatingWater) THEN

            if (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingSteam) {
                // set air-side and steam-side mass flow rates
                Node(state.dataFurnaces->Furnace(FurnaceNum).HWCoilAirInletNode).MassFlowRate = state.dataFurnaces->CompOnMassFlow;
                mdot = state.dataFurnaces->Furnace(FurnaceNum).MaxHeatCoilFluidFlow;
                SetComponentFlowRate(state,
                                     mdot,
                                     state.dataFurnaces->Furnace(FurnaceNum).CoilControlNode,
                                     state.dataFurnaces->Furnace(FurnaceNum).CoilOutletNode,
                                     state.dataFurnaces->Furnace(FurnaceNum).plantLoc);

                //     simulate steam coil to find operating capacity
                SimulateSteamCoilComponents(state,
                                            state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilName,
                                            FirstHVACIteration,
                                            state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex,
                                            1.0,
                                            QActual); // QCoilReq, simulate any load > 0 to get max capacity of steam coil
                state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity =
                    SteamCoils::GetCoilCapacity(state,
                                                state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType,
                                                state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilName,
                                                ErrorsFound);

            } // from IF(Furnace(FurnaceNum)%HeatingCoilType_Num == Coil_HeatingSteam) THEN

            if (state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType_Num == Coil_HeatingWater) {

                //     set air-side and steam-side mass flow rates
                Node(state.dataFurnaces->Furnace(FurnaceNum).SuppCoilAirInletNode).MassFlowRate = state.dataFurnaces->CompOnMassFlow;
                mdot = state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow;
                SetComponentFlowRate(state,
                                     mdot,
                                     state.dataFurnaces->Furnace(FurnaceNum).SuppCoilControlNode,
                                     state.dataFurnaces->Furnace(FurnaceNum).SuppCoilOutletNode,
                                     state.dataFurnaces->Furnace(FurnaceNum).SuppPlantLoc);

                //     simulate water coil to find operating capacity
                SimulateWaterCoilComponents(state,
                                            state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilName,
                                            FirstHVACIteration,
                                            state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex,
                                            QActual);
                state.dataFurnaces->Furnace(FurnaceNum).DesignSuppHeatingCapacity = QActual;

            } // from IF(Furnace(FurnaceNum)%SuppHeatCoilType_Num == Coil_HeatingWater) THEN
            if (state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType_Num == Coil_HeatingSteam) {
                //     set air-side and steam-side mass flow rates
                Node(state.dataFurnaces->Furnace(FurnaceNum).SuppCoilAirInletNode).MassFlowRate = state.dataFurnaces->CompOnMassFlow;
                mdot = state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow;
                SetComponentFlowRate(state,
                                     mdot,
                                     state.dataFurnaces->Furnace(FurnaceNum).SuppCoilControlNode,
                                     state.dataFurnaces->Furnace(FurnaceNum).SuppCoilOutletNode,
                                     state.dataFurnaces->Furnace(FurnaceNum).SuppPlantLoc);

                //     simulate steam coil to find operating capacity
                SimulateSteamCoilComponents(state,
                                            state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilName,
                                            FirstHVACIteration,
                                            state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex,
                                            1.0,
                                            QActual); // QCoilReq, simulate any load > 0 to get max capacity of steam coil
                state.dataFurnaces->Furnace(FurnaceNum).DesignSuppHeatingCapacity =
                    SteamCoils::GetCoilCapacity(state,
                                                state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType,
                                                state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilName,
                                                ErrorsFound);

            } // from IF(Furnace(FurnaceNum)%SuppHeatCoilType_Num == Coil_HeatingSteam) THEN
        }     // from IF( FirstHVACIteration ) THEN

        if (state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling > 0) { // BoS, variable-speed water source hp
            // Furnace(FurnaceNum)%IdleMassFlowRate = RhoAir*Furnace(FurnaceNum)%IdleVolumeAirRate
            NumOfSpeedCooling = state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling;
            NumOfSpeedHeating = state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedHeating;
            // IF MSHP system was not autosized and the fan is autosized, check that fan volumetric flow rate is greater than MSHP flow rates
            if (state.dataFurnaces->Furnace(FurnaceNum).CheckFanFlow) {
                state.dataFurnaces->CurrentModuleObject = "AirLoopHVAC:UnitaryHeatPump:VariableSpeed";
                GetFanVolFlow(state, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->Furnace(FurnaceNum).FanVolFlow);

                if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) // set max fan flow rate to the IHP collection
                {
                    IHPIndex = state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex;
                }

                if (state.dataFurnaces->Furnace(FurnaceNum).FanVolFlow != DataSizing::AutoSize) {
                    //     Check fan versus system supply air flow rates
                    if (state.dataFurnaces->Furnace(FurnaceNum).FanVolFlow + 1e-10 <
                        state.dataFurnaces->Furnace(FurnaceNum).CoolVolumeFlowRate(NumOfSpeedCooling)) {
                        ShowWarningError(state,
                                         format("{} - air flow rate = {:.7T} in fan object is less than the MSHP system air flow rate when cooling "
                                                "is required ({:.7T}).",
                                                state.dataFurnaces->CurrentModuleObject,
                                                state.dataFurnaces->Furnace(FurnaceNum).FanVolFlow,
                                                state.dataFurnaces->Furnace(FurnaceNum).CoolVolumeFlowRate(NumOfSpeedCooling)));
                        ShowContinueError(
                            state, " The MSHP system flow rate when cooling is required is reset to the fan flow rate and the simulation continues.");
                        ShowContinueError(
                            state,
                            format(" Occurs in {} = {}", state.dataFurnaces->CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        state.dataFurnaces->Furnace(FurnaceNum).CoolVolumeFlowRate(NumOfSpeedCooling) =
                            state.dataFurnaces->Furnace(FurnaceNum).FanVolFlow;

                        if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) // set max fan flow rate to the IHP collection
                        {
                            state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).MaxCoolAirVolFlow =
                                state.dataFurnaces->Furnace(FurnaceNum).FanVolFlow;
                            state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).MaxCoolAirMassFlow =
                                state.dataFurnaces->Furnace(FurnaceNum).FanVolFlow * state.dataEnvrn->StdRhoAir;
                        }

                        // Check flow rates in other speeds and ensure flow rates are not above the max flow rate
                        for (int i = NumOfSpeedCooling - 1; i >= 1; --i) {
                            if (state.dataFurnaces->Furnace(FurnaceNum).CoolVolumeFlowRate(i) >
                                state.dataFurnaces->Furnace(FurnaceNum).CoolVolumeFlowRate(i + 1)) {
                                ShowContinueError(state,
                                                  format(" The MSHP system flow rate when cooling is required is reset to the flow rate at higher "
                                                         "speed and the simulation continues at Speed{}.",
                                                         i));
                                ShowContinueError(state,
                                                  format(" Occurs in {} = {}",
                                                         state.dataFurnaces->CurrentModuleObject,
                                                         state.dataFurnaces->Furnace(FurnaceNum).Name));
                                state.dataFurnaces->Furnace(FurnaceNum).CoolVolumeFlowRate(i) =
                                    state.dataFurnaces->Furnace(FurnaceNum).CoolVolumeFlowRate(i + 1);
                            }
                        }
                    }
                    if (NumOfSpeedHeating > 0) {
                        if (state.dataFurnaces->Furnace(FurnaceNum).FanVolFlow + 1e-10 <
                            state.dataFurnaces->Furnace(FurnaceNum).HeatVolumeFlowRate(NumOfSpeedHeating)) {
                            ShowWarningError(state,
                                             format("{} - air flow rate = {:.7T} in fan object is less than the MSHP system air flow rate when "
                                                    "heating is required ({:.7T}).",
                                                    state.dataFurnaces->CurrentModuleObject,
                                                    state.dataFurnaces->Furnace(FurnaceNum).FanVolFlow,
                                                    state.dataFurnaces->Furnace(FurnaceNum).HeatVolumeFlowRate(NumOfSpeedHeating)));
                            ShowContinueError(
                                state,
                                " The MSHP system flow rate when heating is required is reset to the fan flow rate and the simulation continues.");
                            ShowContinueError(
                                state,
                                format(" Occurs in {} = {}", state.dataFurnaces->CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                            state.dataFurnaces->Furnace(FurnaceNum).HeatVolumeFlowRate(NumOfSpeedHeating) =
                                state.dataFurnaces->Furnace(FurnaceNum).FanVolFlow;

                            if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) // set max fan flow rate to the IHP collection
                            {
                                state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex)
                                    .MaxHeatAirVolFlow = state.dataFurnaces->Furnace(FurnaceNum).FanVolFlow;
                                state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex)
                                    .MaxHeatAirMassFlow = state.dataFurnaces->Furnace(FurnaceNum).FanVolFlow * state.dataEnvrn->StdRhoAir;
                            }

                            for (int i = NumOfSpeedHeating - 1; i >= 1; --i) {
                                if (state.dataFurnaces->Furnace(FurnaceNum).HeatVolumeFlowRate(i) >
                                    state.dataFurnaces->Furnace(FurnaceNum).HeatVolumeFlowRate(i + 1)) {
                                    ShowContinueError(state,
                                                      format(" The MSHP system flow rate when heating is required is reset to the flow rate at "
                                                             "higher speed and the simulation continues at Speed{}.",
                                                             i));
                                    ShowContinueError(state,
                                                      format(" Occurs in {} system = {}",
                                                             state.dataFurnaces->CurrentModuleObject,
                                                             state.dataFurnaces->Furnace(FurnaceNum).Name));
                                    state.dataFurnaces->Furnace(FurnaceNum).HeatVolumeFlowRate(i) =
                                        state.dataFurnaces->Furnace(FurnaceNum).HeatVolumeFlowRate(i + 1);
                                }
                            }
                        }
                    }
                    if (state.dataFurnaces->Furnace(FurnaceNum).FanVolFlow < state.dataFurnaces->Furnace(FurnaceNum).IdleVolumeAirRate &&
                        state.dataFurnaces->Furnace(FurnaceNum).IdleVolumeAirRate != 0.0) {
                        ShowWarningError(state,
                                         format("{} - air flow rate = {:.7T} in fan object is less than the MSHP system air flow rate when no "
                                                "heating or cooling is needed ({:.7T}).",
                                                state.dataFurnaces->CurrentModuleObject,
                                                state.dataFurnaces->Furnace(FurnaceNum).FanVolFlow,
                                                state.dataFurnaces->Furnace(FurnaceNum).IdleVolumeAirRate));
                        ShowContinueError(state,
                                          " The MSHP system flow rate when no heating or cooling is needed is reset to the fan flow rate and the "
                                          "simulation continues.");
                        ShowContinueError(
                            state,
                            format(" Occurs in {} = {}", state.dataFurnaces->CurrentModuleObject, state.dataFurnaces->Furnace(FurnaceNum).Name));
                        state.dataFurnaces->Furnace(FurnaceNum).IdleVolumeAirRate = state.dataFurnaces->Furnace(FurnaceNum).FanVolFlow;
                    }
                    RhoAir = state.dataEnvrn->StdRhoAir;
                    // set the mass flow rates from the reset volume flow rates
                    for (int i = 1; i <= NumOfSpeedCooling; ++i) {
                        state.dataFurnaces->Furnace(FurnaceNum).CoolMassFlowRate(i) =
                            RhoAir * state.dataFurnaces->Furnace(FurnaceNum).CoolVolumeFlowRate(i);
                        if (state.dataFurnaces->Furnace(FurnaceNum).FanVolFlow > 0.0) {
                            state.dataFurnaces->Furnace(FurnaceNum).MSCoolingSpeedRatio(i) =
                                state.dataFurnaces->Furnace(FurnaceNum).CoolVolumeFlowRate(i) / state.dataFurnaces->Furnace(FurnaceNum).FanVolFlow;
                        }
                    }
                    for (int i = 1; i <= NumOfSpeedHeating; ++i) {
                        state.dataFurnaces->Furnace(FurnaceNum).HeatMassFlowRate(i) =
                            RhoAir * state.dataFurnaces->Furnace(FurnaceNum).HeatVolumeFlowRate(i);
                        if (state.dataFurnaces->Furnace(FurnaceNum).FanVolFlow > 0.0) {
                            state.dataFurnaces->Furnace(FurnaceNum).MSHeatingSpeedRatio(i) =
                                state.dataFurnaces->Furnace(FurnaceNum).HeatVolumeFlowRate(i) / state.dataFurnaces->Furnace(FurnaceNum).FanVolFlow;
                        }
                    }
                    state.dataFurnaces->Furnace(FurnaceNum).IdleMassFlowRate = RhoAir * state.dataFurnaces->Furnace(FurnaceNum).IdleVolumeAirRate;
                    if (state.dataFurnaces->Furnace(FurnaceNum).FanVolFlow > 0.0) {
                        state.dataFurnaces->Furnace(FurnaceNum).IdleSpeedRatio =
                            state.dataFurnaces->Furnace(FurnaceNum).IdleVolumeAirRate / state.dataFurnaces->Furnace(FurnaceNum).FanVolFlow;
                    }
                    // set the node max and min mass flow rates based on reset volume flow rates
                    if (NumOfSpeedCooling > 0 && NumOfSpeedHeating == 0) {
                        Node(InNode).MassFlowRateMax = max(state.dataFurnaces->Furnace(FurnaceNum).CoolMassFlowRate(NumOfSpeedCooling),
                                                           state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirMassFlow);
                        Node(InNode).MassFlowRateMaxAvail = max(state.dataFurnaces->Furnace(FurnaceNum).CoolMassFlowRate(NumOfSpeedCooling),
                                                                state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirMassFlow);
                    } else if (NumOfSpeedCooling == 0 && NumOfSpeedHeating > 0) {
                        Node(InNode).MassFlowRateMax = max(state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirMassFlow,
                                                           state.dataFurnaces->Furnace(FurnaceNum).HeatMassFlowRate(NumOfSpeedHeating));
                        Node(InNode).MassFlowRateMaxAvail = max(state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirMassFlow,
                                                                state.dataFurnaces->Furnace(FurnaceNum).HeatMassFlowRate(NumOfSpeedHeating));
                    } else {
                        Node(InNode).MassFlowRateMax = max(state.dataFurnaces->Furnace(FurnaceNum).CoolMassFlowRate(NumOfSpeedCooling),
                                                           state.dataFurnaces->Furnace(FurnaceNum).HeatMassFlowRate(NumOfSpeedHeating));
                        Node(InNode).MassFlowRateMaxAvail = max(state.dataFurnaces->Furnace(FurnaceNum).CoolMassFlowRate(NumOfSpeedCooling),
                                                                state.dataFurnaces->Furnace(FurnaceNum).HeatMassFlowRate(NumOfSpeedHeating));
                    }
                    Node(InNode).MassFlowRateMin = 0.0;
                    Node(InNode).MassFlowRateMinAvail = 0.0;
                    Node(OutNode) = Node(InNode);
                }
            }

            state.dataFurnaces->Furnace(FurnaceNum).CheckFanFlow = false;

            SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, OpMode, QZnReq, MoistureLoad, PartLoadRatio);
        } else {
            SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, OpMode, QZnReq, MoistureLoad, PartLoadRatio);
        }

        // Check ventilation/fan load for constant fan systems to see if load to be met changes
        // Same IF logic used in Subroutine SetAverageAirFlow to determine if unit is ON or OFF

        QToCoolSetPt = 0.0;
        QToHeatSetPt = 0.0;
        if (OpMode == ContFanCycCoil && GetCurrentScheduleValue(state, state.dataFurnaces->Furnace(FurnaceNum).SchedPtr) > 0.0 &&
            ((GetCurrentScheduleValue(state, state.dataFurnaces->Furnace(FurnaceNum).FanAvailSchedPtr) > 0.0 || state.dataHVACGlobal->TurnFansOn) &&
             !state.dataHVACGlobal->TurnFansOff)) {

            if (state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling > 0) {
                CalcVarSpeedHeatPump(state,
                                     FurnaceNum,
                                     false,
                                     CompressorOperation::Off,
                                     1,
                                     0.0,
                                     0.0,
                                     SensibleOutput,
                                     LatentOutput,
                                     0.0,
                                     0.0,
                                     OnOffAirFlowRatio,
                                     SUPHEATERLOAD);
            } else {
                CalcFurnaceOutput(state,
                                  FurnaceNum,
                                  false,
                                  0,
                                  CompressorOperation::Off,
                                  0.0,
                                  0.0,
                                  0.0,
                                  0.0,
                                  SensibleOutput,
                                  LatentOutput,
                                  OnOffAirFlowRatio,
                                  false);
            }

            if (state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac > 0.0) {
                if (state.dataFurnaces->Furnace(FurnaceNum).ZoneSequenceCoolingNum > 0 &&
                    state.dataFurnaces->Furnace(FurnaceNum).ZoneSequenceHeatingNum > 0) {
                    QToCoolSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                                       .SequencedOutputRequiredToCoolingSP(state.dataFurnaces->Furnace(FurnaceNum).ZoneSequenceCoolingNum) /
                                   state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac;
                    QToHeatSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                                       .SequencedOutputRequiredToHeatingSP(state.dataFurnaces->Furnace(FurnaceNum).ZoneSequenceHeatingNum) /
                                   state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac;
                } else {
                    QToCoolSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                                       .OutputRequiredToCoolingSP /
                                   state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac;
                    QToHeatSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                                       .OutputRequiredToHeatingSP /
                                   state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac;
                }
                //     If the furnace has a net cooling capacity (SensibleOutput < 0) and
                //     the zone temp is above the Tstat heating setpoint (QToHeatSetPt < 0) and
                //     the net cooling capacity does not just offset the cooling load
                if (SensibleOutput < 0.0 && QToHeatSetPt < 0.0 &&
                    std::abs(QToCoolSetPt - SensibleOutput) > (Small5WLoad / state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac)) {
                    //       Only switch modes when humidistat is not used or no moisture load exists, otherwise let
                    //       reheat coil pick up load
                    //        IF((SensibleOutput .LT. QToHeatSetPt .AND. .NOT. Furnace(FurnaceNum)%Humidistat) .OR. &
                    //           (SensibleOutput .LT. QToHeatSetPt .AND. Furnace(FurnaceNum)%Humidistat .AND. MoistureLoad .GE. 0.0))THEN
                    if ((SensibleOutput < QToHeatSetPt && !state.dataFurnaces->Furnace(FurnaceNum).Humidistat) ||
                        (SensibleOutput < QToHeatSetPt && state.dataFurnaces->Furnace(FurnaceNum).Humidistat && MoistureLoad >= 0.0)) {
                        QZnReq = QToHeatSetPt;
                        state.dataFurnaces->CoolingLoad = false;
                        //         Don't set mode TRUE unless mode is allowed. Also check for floating zone.
                        if (state.dataHeatBalFanSys->TempControlType(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum) ==
                                DataHVACGlobals::ThermostatType::SingleCooling ||
                            state.dataHeatBalFanSys->TempControlType(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum) ==
                                DataHVACGlobals::ThermostatType::Uncontrolled) {
                            state.dataFurnaces->HeatingLoad = false;
                        } else {
                            state.dataFurnaces->HeatingLoad = true;
                        }

                        if (state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling > 0) {
                            SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, OpMode, QZnReq, MoistureLoad, PartLoadRatio);
                            CalcVarSpeedHeatPump(state,
                                                 FurnaceNum,
                                                 false,
                                                 CompressorOperation::Off,
                                                 1,
                                                 0.0,
                                                 0.0,
                                                 SensibleOutput,
                                                 LatentOutput,
                                                 0.0,
                                                 0.0,
                                                 OnOffAirFlowRatio,
                                                 SUPHEATERLOAD);
                        } else {
                            SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, OpMode, QZnReq, MoistureLoad, PartLoadRatio);
                            CalcFurnaceOutput(state,
                                              FurnaceNum,
                                              false,
                                              0,
                                              CompressorOperation::Off,
                                              0.0,
                                              0.0,
                                              0.0,
                                              0.0,
                                              SensibleOutput,
                                              LatentOutput,
                                              OnOffAirFlowRatio,
                                              false);
                        }
                        if (SensibleOutput > QToHeatSetPt) {
                            //           If changing operating mode (flow rates) does not overshoot heating setpoint, turn off heating
                            QZnReq = 0.0;
                            state.dataFurnaces->HeatingLoad = false;
                            if (state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling > 0) {
                                SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, OpMode, QZnReq, MoistureLoad, PartLoadRatio);
                                //               CALL SetOnOffMassFlowRateVSCoil(FurnaceNum, Furnace(FurnaceNum)%ControlZoneNum, FirstHVACIteration, &
                                //                    AirLoopNum, OnOffAirFlowRatio, OpMode, QZnReq, MoistureLoad, PartLoadRatio)
                            } else {
                                SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, OpMode, QZnReq, MoistureLoad, PartLoadRatio);
                            }
                        }
                    } else if (SensibleOutput < QZnReq) {
                        //         If the net cooling capacity meets the zone cooling load but does not overshoot heating setpoint, turn off cooling
                        //         (dehumidification may still occur)
                        QZnReq = 0.0;
                        state.dataFurnaces->CoolingLoad = false;
                        if (state.dataFurnaces->HPDehumidificationLoadFlag) {
                            state.dataFurnaces->CoolingLoad = true;
                            state.dataFurnaces->HeatingLoad = false;
                        }
                        if (state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling > 0) {
                            SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, OpMode, QZnReq, MoistureLoad, PartLoadRatio);
                            //               CALL SetOnOffMassFlowRateVSCoil(FurnaceNum, Furnace(FurnaceNum)%ControlZoneNum, FirstHVACIteration, &
                            //                    AirLoopNum, OnOffAirFlowRatio, OpMode, QZnReq, MoistureLoad, PartLoadRatio)
                        } else {
                            SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, OpMode, QZnReq, MoistureLoad, PartLoadRatio);
                        }
                    }
                    //     the net cooling capacity just offsets the cooling load, turn off cooling
                } else if (SensibleOutput < 0.0 && QToCoolSetPt < 0.0 &&
                           std::abs(QToCoolSetPt - SensibleOutput) <
                               (Small5WLoad / state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac)) {
                    state.dataFurnaces->CoolingLoad = false;
                    if (state.dataFurnaces->HPDehumidificationLoadFlag) {
                        state.dataFurnaces->CoolingLoad = true;
                        state.dataFurnaces->HeatingLoad = false;
                    }
                } // SensibleOutput .LT. 0.0d0 .AND. QToHeatSetPt .LT. 0.0d0

                //     If the furnace has a net heating capacity and the zone temp is below the Tstat cooling setpoint and
                //     the net heating capacity does not just offset the heating load
                if (SensibleOutput > 0.0 && QToCoolSetPt > 0.0 &&
                    std::abs(SensibleOutput - QToHeatSetPt) > (Small5WLoad / state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac)) {
                    if (SensibleOutput > QToCoolSetPt) {
                        QZnReq = QToCoolSetPt;
                        //         Don't set mode TRUE unless mode is allowed. Also check for floating zone.
                        if (state.dataHeatBalFanSys->TempControlType(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum) ==
                                DataHVACGlobals::ThermostatType::SingleHeating ||
                            state.dataHeatBalFanSys->TempControlType(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum) ==
                                DataHVACGlobals::ThermostatType::Uncontrolled) {
                            state.dataFurnaces->CoolingLoad = false;
                        } else {
                            state.dataFurnaces->CoolingLoad = true;
                        }
                        state.dataFurnaces->HeatingLoad = false;

                        if (state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling > 0) {
                            SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, OpMode, QZnReq, MoistureLoad, PartLoadRatio);
                            //           CALL SetOnOffMassFlowRateVSCoil(FurnaceNum, Furnace(FurnaceNum)%ControlZoneNum, FirstHVACIteration, &
                            //                    AirLoopNum, OnOffAirFlowRatio, OpMode, QZnReq, MoistureLoad, PartLoadRatio)
                            CalcVarSpeedHeatPump(state,
                                                 FurnaceNum,
                                                 false,
                                                 CompressorOperation::Off,
                                                 1,
                                                 0.0,
                                                 0.0,
                                                 SensibleOutput,
                                                 LatentOutput,
                                                 0.0,
                                                 0.0,
                                                 OnOffAirFlowRatio,
                                                 SUPHEATERLOAD);
                        } else {
                            SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, OpMode, QZnReq, MoistureLoad, PartLoadRatio);
                            CalcFurnaceOutput(state,
                                              FurnaceNum,
                                              false,
                                              0,
                                              CompressorOperation::Off,
                                              0.0,
                                              0.0,
                                              0.0,
                                              0.0,
                                              SensibleOutput,
                                              LatentOutput,
                                              OnOffAirFlowRatio,
                                              false);
                        }
                        if (SensibleOutput < QToCoolSetPt) {
                            //           If changing operating mode (flow rates) does not overshoot cooling setpoint, turn off cooling
                            if (state.dataFurnaces->HPDehumidificationLoadFlag) {
                                state.dataFurnaces->CoolingLoad = true;
                                state.dataFurnaces->HeatingLoad = false;
                            } else {
                                QZnReq = 0.0;
                                state.dataFurnaces->CoolingLoad = false;
                            }
                            if (state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling > 0) {
                                SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, OpMode, QZnReq, MoistureLoad, PartLoadRatio);
                                //               CALL SetOnOffMassFlowRateVSCoil(FurnaceNum, Furnace(FurnaceNum)%ControlZoneNum, FirstHVACIteration, &
                                //                     AirLoopNum, OnOffAirFlowRatio, OpMode, QZnReq, MoistureLoad, PartLoadRatio)
                            } else {
                                SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, OpMode, QZnReq, MoistureLoad, PartLoadRatio);
                            }
                        }
                    } else if (SensibleOutput > QZnReq) {
                        //         If the net heating capacity meets the zone heating load but does not overshoot, turn off heating
                        QZnReq = 0.0;
                        state.dataFurnaces->HeatingLoad = false;
                        if (state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling > 0) {
                            //            CALL SetOnOffMassFlowRateVSCoil(FurnaceNum, Furnace(FurnaceNum)%ControlZoneNum, FirstHVACIteration, &
                            //                        AirLoopNum, OnOffAirFlowRatio, OpMode, QZnReq, MoistureLoad, PartLoadRatio)
                            SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, OpMode, QZnReq, MoistureLoad, PartLoadRatio);
                        } else {
                            SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, OpMode, QZnReq, MoistureLoad, PartLoadRatio);
                        }
                    }
                    //     the net heating capacity just offsets the heating load, turn off heating
                } else if (SensibleOutput > 0.0 && QToHeatSetPt > 0.0 &&
                           std::abs(SensibleOutput - QToHeatSetPt) <
                               (Small5WLoad / state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac)) {
                    state.dataFurnaces->HeatingLoad = false;
                } // SensibleOutput .GT. 0.0d0 .AND. QToCoolSetPt .GT. 0.0d0
            }     // Furnace(FurnaceNum)%ControlZoneMassFlowFrac .GT. 0.0d0
            ZoneLoad = QZnReq;
        } // OpMode .EQ. ContFanCycCoil

        if (FirstHVACIteration) {
            state.dataFurnaces->Furnace(FurnaceNum).iterationCounter = 0;
            state.dataFurnaces->Furnace(FurnaceNum).iterationMode = Furnaces::ModeOfOperation::NoCoolHeat;
        }
        state.dataFurnaces->Furnace(FurnaceNum).iterationCounter += 1;

        // push iteration mode stack and set current mode
        state.dataFurnaces->Furnace(FurnaceNum).iterationMode(3) = state.dataFurnaces->Furnace(FurnaceNum).iterationMode(2);
        state.dataFurnaces->Furnace(FurnaceNum).iterationMode(2) = state.dataFurnaces->Furnace(FurnaceNum).iterationMode(1);
        if (state.dataFurnaces->CoolingLoad) {
            state.dataFurnaces->Furnace(FurnaceNum).iterationMode(1) = Furnaces::ModeOfOperation::CoolingMode;
        } else if (state.dataFurnaces->HeatingLoad) {
            state.dataFurnaces->Furnace(FurnaceNum).iterationMode(1) = Furnaces::ModeOfOperation::HeatingMode;
        } else {
            state.dataFurnaces->Furnace(FurnaceNum).iterationMode(1) = Furnaces::ModeOfOperation::NoCoolHeat;
        }

        // IF small loads to meet or not converging, just shut down unit
        if (std::abs(ZoneLoad) < Small5WLoad) {
            ZoneLoad = 0.0;
            state.dataFurnaces->CoolingLoad = false;
            state.dataFurnaces->HeatingLoad = false;
        } else if (state.dataFurnaces->Furnace(FurnaceNum).iterationCounter > (state.dataHVACGlobal->MinAirLoopIterationsAfterFirst + 4)) {
            // attempt to lock output (air flow) if oscillations are detected
            OperatingMode = state.dataFurnaces->Furnace(FurnaceNum).iterationMode(1);
            OperatingModeMinusOne = state.dataFurnaces->Furnace(FurnaceNum).iterationMode(2);
            OperatingModeMinusTwo = state.dataFurnaces->Furnace(FurnaceNum).iterationMode(3);
            Oscillate = true;
            if (OperatingMode == OperatingModeMinusOne && OperatingMode == OperatingModeMinusTwo) Oscillate = false;
            if (Oscillate) {
                if (QToCoolSetPt < 0.0) {
                    state.dataFurnaces->HeatingLoad = false;
                    state.dataFurnaces->CoolingLoad = true;
                    ZoneLoad = QToCoolSetPt;
                } else if (QToHeatSetPt > 0.0) {
                    state.dataFurnaces->HeatingLoad = true;
                    state.dataFurnaces->CoolingLoad = false;
                    ZoneLoad = QToHeatSetPt;
                } else {
                    state.dataFurnaces->HeatingLoad = false;
                    state.dataFurnaces->CoolingLoad = false;
                    ZoneLoad = 0.0;
                }
            }
        }

        // EMS override point
        if (state.dataFurnaces->Furnace(FurnaceNum).EMSOverrideSensZoneLoadRequest)
            ZoneLoad = state.dataFurnaces->Furnace(FurnaceNum).EMSSensibleZoneLoadValue;
        if (state.dataFurnaces->Furnace(FurnaceNum).EMSOverrideMoistZoneLoadRequest)
            MoistureLoad = state.dataFurnaces->Furnace(FurnaceNum).EMSMoistureZoneLoadValue;
        if (state.dataFurnaces->Furnace(FurnaceNum).EMSOverrideSensZoneLoadRequest ||
            state.dataFurnaces->Furnace(FurnaceNum).EMSOverrideMoistZoneLoadRequest) {
            if ((ZoneLoad != 0.0) && (state.dataFurnaces->Furnace(FurnaceNum).EMSOverrideSensZoneLoadRequest)) {
                PartLoadRatio = 1.0;
            } else if ((MoistureLoad != 0.0) && (state.dataFurnaces->Furnace(FurnaceNum).EMSOverrideMoistZoneLoadRequest)) {
                PartLoadRatio = 1.0;
            } else {
                PartLoadRatio = 0.0;
            }
            if (state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling > 0) {
                SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, OpMode, QZnReq, MoistureLoad, PartLoadRatio);
                //       CALL SetOnOffMassFlowRateVSCoil(FurnaceNum, Furnace(FurnaceNum)%ControlZoneNum, FirstHVACIteration, &
                //                AirLoopNum, OnOffAirFlowRatio, OpMode, QZnReq, MoistureLoad, PartLoadRatio)
            } else {
                SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, OpMode, ZoneLoad, MoistureLoad, PartLoadRatio);
            }
        }

        // AirflowNetwork global variable
        if (state.afn->distribution_simulated) {
            state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).AFNLoopHeatingCoilMaxRTF = 0.0;
        }
    }

    void SetOnOffMassFlowRate(EnergyPlusData &state,
                              int const FurnaceNum,                   // index to furnace
                              [[maybe_unused]] int const AirLoopNum,  // index to air loop !unused1208
                              Real64 &OnOffAirFlowRatio,              // ratio of coil on to coil off air flow rate
                              int const OpMode,                       // fan operating mode
                              [[maybe_unused]] Real64 const ZoneLoad, // sensible load to be met (W) !unused1208
                              Real64 const MoistureLoad,              // moisture load to be met (W)
                              Real64 const PartLoadRatio              // coil part-load ratio
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   Sep 2008
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the Furnace Components.

        // METHODOLOGY EMPLOYED:
        // The HeatCool furnace/unitarysystem and air-to-air heat pump may have alternate air flow rates
        // in cooling, heating, and when no cooling or heating is needed. Set up the coil (comp) ON and OFF
        // air flow rates. Use these flow rates during the Calc routines to set the average mass flow rates
        // based on PLR.

        // REFERENCES:
        // na

        // Using/Aliasing

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // na

        // Check for heat only furnace
        if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num != Furnace_HeatOnly &&
            state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num != UnitarySys_HeatOnly) {

            // Set the system mass flow rates
            if (OpMode == ContFanCycCoil) {
                // Set the compressor or coil ON mass flow rate
                // constant fan mode
                if (state.dataFurnaces->HeatingLoad) {
                    //       IF a heating and moisture load exists, operate at the cooling mass flow rate ELSE operate at the heating flow rate
                    if (MoistureLoad < 0.0 && state.dataFurnaces->Furnace(FurnaceNum).Humidistat &&
                        state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::CoolReheat) {
                        state.dataFurnaces->CompOnMassFlow = state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirMassFlow;
                        state.dataFurnaces->CompOnFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).CoolingSpeedRatio;
                    } else {
                        state.dataFurnaces->CompOnMassFlow = state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirMassFlow;
                        state.dataFurnaces->CompOnFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).HeatingSpeedRatio;
                    }
                    state.dataFurnaces->Furnace(FurnaceNum).LastMode = Furnaces::ModeOfOperation::HeatingMode;
                    //     IF a cooling load exists, operate at the cooling mass flow rate
                } else if (state.dataFurnaces->CoolingLoad) {
                    state.dataFurnaces->CompOnMassFlow = state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirMassFlow;
                    state.dataFurnaces->CompOnFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).CoolingSpeedRatio;
                    state.dataFurnaces->Furnace(FurnaceNum).LastMode = Furnaces::ModeOfOperation::CoolingMode;
                    //     If no load exists, set the compressor on mass flow rate.
                    //     Set equal the mass flow rate when no heating or cooling is needed if no moisture load exists.
                    //     If the user has set the off mass flow rate to 0, set according to the last operating mode.
                } else {
                    if (MoistureLoad < 0.0 && state.dataFurnaces->Furnace(FurnaceNum).Humidistat &&
                        state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::CoolReheat) {
                        state.dataFurnaces->CompOnMassFlow = state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirMassFlow;
                        state.dataFurnaces->CompOnFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).CoolingSpeedRatio;
                    } else {
                        state.dataFurnaces->CompOnMassFlow = state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirMassFlow;
                        state.dataFurnaces->CompOnFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).HeatingSpeedRatio;
                        //         User may have entered a 0 for MaxNoCoolHeatAirMassFlow
                        if (state.dataFurnaces->CompOnMassFlow == 0.0) {
                            if (state.dataFurnaces->Furnace(FurnaceNum).LastMode == Furnaces::ModeOfOperation::HeatingMode) {
                                state.dataFurnaces->CompOnMassFlow = state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirMassFlow;
                                state.dataFurnaces->CompOnFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).HeatingSpeedRatio;
                            } else {
                                state.dataFurnaces->CompOnMassFlow = state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirMassFlow;
                                state.dataFurnaces->CompOnFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).CoolingSpeedRatio;
                            }
                        }
                    }
                }

                //     Set the compressor or coil OFF mass flow rate based on LOGICAL flag
                //     UseCompressorOnFlow is used when the user does not enter a value for no cooling or heating flow rate
                if (state.dataFurnaces->Furnace(FurnaceNum).AirFlowControl == AirFlowControlConstFan::UseCompressorOnFlow) {
                    if (state.dataFurnaces->Furnace(FurnaceNum).LastMode == Furnaces::ModeOfOperation::HeatingMode) {
                        if (MoistureLoad < 0.0 && state.dataFurnaces->Furnace(FurnaceNum).Humidistat &&
                            state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::CoolReheat) {
                            state.dataFurnaces->CompOffMassFlow = state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirMassFlow;
                            state.dataFurnaces->CompOffFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).CoolingSpeedRatio;
                        } else {
                            state.dataFurnaces->CompOffMassFlow = state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirMassFlow;
                            state.dataFurnaces->CompOffFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).HeatingSpeedRatio;
                        }
                    } else {
                        state.dataFurnaces->CompOffMassFlow = state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirMassFlow;
                        state.dataFurnaces->CompOffFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).CoolingSpeedRatio;
                    }
                    //     ELSE use the user specified value
                } else {
                    state.dataFurnaces->CompOffMassFlow = state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirMassFlow;
                    state.dataFurnaces->CompOffFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).NoHeatCoolSpeedRatio;
                }
            } else {
                //     cycling fan mode
                if (state.dataFurnaces->HeatingLoad ||
                    (state.dataFurnaces->Furnace(FurnaceNum).Humidistat && MoistureLoad < 0.0 &&
                     state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::CoolReheat)) {

                    if (state.dataFurnaces->Furnace(FurnaceNum).Humidistat && MoistureLoad < 0.0 &&
                        state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::CoolReheat) {
                        state.dataFurnaces->CompOnMassFlow = state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirMassFlow;
                        state.dataFurnaces->CompOnFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).CoolingSpeedRatio;
                        state.dataFurnaces->Furnace(FurnaceNum).LastMode = Furnaces::ModeOfOperation::CoolingMode;
                    } else {
                        state.dataFurnaces->CompOnMassFlow = state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirMassFlow;
                        state.dataFurnaces->CompOnFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).HeatingSpeedRatio;
                        state.dataFurnaces->Furnace(FurnaceNum).LastMode = Furnaces::ModeOfOperation::HeatingMode;
                    }
                } else if (state.dataFurnaces->CoolingLoad) {
                    state.dataFurnaces->CompOnMassFlow = state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirMassFlow;
                    state.dataFurnaces->CompOnFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).CoolingSpeedRatio;
                } else {
                    state.dataFurnaces->CompOnMassFlow = 0.0;
                    state.dataFurnaces->CompOnFlowRatio = 0.0;
                }
                state.dataFurnaces->CompOffMassFlow = 0.0;
                state.dataFurnaces->CompOffFlowRatio = 0.0;
            }
        } else { //  Is a HeatOnly furnace

            state.dataFurnaces->CompOnMassFlow = state.dataFurnaces->Furnace(FurnaceNum).DesignMassFlowRate;
            state.dataFurnaces->CompOnFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).HeatingSpeedRatio;
            if (OpMode == ContFanCycCoil) {
                state.dataFurnaces->CompOffMassFlow = state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirMassFlow;
                state.dataFurnaces->CompOffFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).HeatingSpeedRatio;
            } else {
                state.dataFurnaces->CompOffMassFlow = 0.0;
                state.dataFurnaces->CompOffFlowRatio = 0.0;
            }

        } // End check for heat only furnace or water-to-air heat pump

        // Set the system mass flow rates
        SetAverageAirFlow(state, FurnaceNum, PartLoadRatio, OnOffAirFlowRatio);
    }

    void SizeFurnace(EnergyPlusData &state, int const FurnaceNum, bool const FirstHVACIteration)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   January 2002
        //       MODIFIED       Bereket Nigusse, May 2010, removed the autosize option for the input field supply air
        //                                                 flow fraction through controlled zone.
        //                      Bo Shen, March 2012, size the air flow rates at individual speed levels for VS WSHP
        //                      Bo Shen, ORNL, July 2012 - added variable-speed air source heat pump cooling and heating coils, using curve-fits
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing Furnace Components for which nominal cpacities
        // and flow rates have not been specified in the input

        // METHODOLOGY EMPLOYED:
        // Obtains heating capacities and flow rates from the zone or system sizing arrays.
        // NOTE: In UNITARYSYSTEM:HEATPUMP:AIRTOAIR we are sizing the heating capacity to be
        // equal to the cooling capacity.  Thus the cooling and
        // and heating capacities of a DX heat pump system will be identical. In real life the ARI
        // heating and cooling capacities are close but not identical.

        // REFERENCES:
        // na

        // Using/Aliasing
        using namespace DataSizing;
        using EMSManager::ManageEMS;

        using HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil;
        using IntegratedHeatPump::SizeIHP;
        using VariableSpeedCoils::SimVariableSpeedCoils;
        using WaterToAirHeatPumpSimple::SimWatertoAirHPSimple;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int ThisCtrlZoneNum;      // the controlled zone number of the control zone !!!
        int Iter;                 // iteration count
        Real64 MulSpeedFlowScale; // variable speed air flow scaling factor
        int IHPCoilIndex(0);      // refer to cooling or heating coil in IHP
        Real64 dummy(0.0);
        bool anyRan;
        ManageEMS(state, EMSManager::EMSCallFrom::UnitarySystemSizing, anyRan, ObjexxFCL::Optional_int_const()); // calling point

        ThisCtrlZoneNum = 0;
        state.dataSize->DXCoolCap = 0.0;
        state.dataSize->UnitaryHeatCap = 0.0;
        state.dataSize->SuppHeatCap = 0.0;

        if (state.dataFurnaces->Furnace(FurnaceNum).FanType_Num == DataHVACGlobals::FanType_SystemModelObject) {
            state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).supFanVecIndex = state.dataFurnaces->Furnace(FurnaceNum).FanIndex;
            state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).supFanModelType = DataAirSystems::ObjectVectorOOFanSystemModel;
            state.dataSize->DataFanEnumType = DataAirSystems::ObjectVectorOOFanSystemModel;
            state.dataSize->DataFanIndex = state.dataFurnaces->Furnace(FurnaceNum).FanIndex;
        } else {
            state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).SupFanNum = state.dataFurnaces->Furnace(FurnaceNum).FanIndex;
            state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).supFanModelType = DataAirSystems::StructArrayLegacyFanModels;
            state.dataSize->DataFanEnumType = DataAirSystems::StructArrayLegacyFanModels;
            state.dataSize->DataFanIndex = state.dataFurnaces->Furnace(FurnaceNum).FanIndex;
        }
        if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == BlowThru) {
            state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).supFanLocation = DataAirSystems::FanPlacement::BlowThru;
        } else if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == DrawThru) {
            state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).supFanLocation = DataAirSystems::FanPlacement::DrawThru;
        }

        if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == CoilDX_CoolingSingleSpeed) {
            SimDXCoil(state, BlankString, CompressorOperation::On, true, state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex, 1, 0.0);
        } else if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == CoilDX_CoolingHXAssisted) {
            int HXCC_Index = state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex;
            int childCCType_Num = state.dataHVACAssistedCC->HXAssistedCoil(HXCC_Index).CoolingCoilType_Num;
            if (childCCType_Num == DataHVACGlobals::CoilDX_Cooling) {
                int childCCIndex = state.dataHVACAssistedCC->HXAssistedCoil(HXCC_Index).CoolingCoilIndex;
                if (childCCIndex < 0) {
                    ShowContinueError(state, "Occurs in sizing HeatExchangerAssistedCoolingCoil.");
                }
                auto &newCoil = state.dataCoilCooingDX->coilCoolingDXs[childCCIndex];
                newCoil.size(state);
            }
            SimHXAssistedCoolingCoil(state,
                                     BlankString,
                                     true,
                                     CompressorOperation::On,
                                     0.0,
                                     state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                     1,
                                     false,
                                     1.0,
                                     false);
        } else if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == Coil_CoolingWaterToAirHPSimple) {
            SimWatertoAirHPSimple(state,
                                  BlankString,
                                  state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                  state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand,
                                  state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilLatentDemand,
                                  0,
                                  0.0,
                                  state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                                  state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                                  state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                                  CompressorOperation::Off,
                                  0.0,
                                  FirstHVACIteration); // CoolPartLoadRatio
            if (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingWaterToAirHPSimple) {
                SimWatertoAirHPSimple(state,
                                      BlankString,
                                      state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex,
                                      state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand,
                                      dummy,
                                      0.0,
                                      0.0,
                                      state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                                      state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                                      state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                                      CompressorOperation::Off,
                                      0.0,
                                      FirstHVACIteration);
            }
        } else if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == Coil_CoolingWaterToAirHPVSEquationFit ||
                   state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == Coil_CoolingAirToAirVariableSpeed) {
            if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                SizeIHP(state, state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex);
                IHPCoilIndex = state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).SCCoilIndex;
                state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling = state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).NumOfSpeeds;
                MulSpeedFlowScale = state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).RatedAirVolFlowRate /
                                    state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex)
                                        .MSRatedAirVolFlowRate(state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).NormSpedLevel);
                state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).CoolVolFlowScale =
                    MulSpeedFlowScale;
            } else {
                SimVariableSpeedCoils(state,
                                      BlankString,
                                      state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                      0,
                                      state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                                      state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                                      state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                                      CompressorOperation::Off,
                                      0.0,
                                      1,
                                      0.0,
                                      0.0,
                                      0.0,
                                      0.0); // conduct the sizing operation in the VS WSHP
                state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling =
                    state.dataVariableSpeedCoils->VarSpeedCoil(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).NumOfSpeeds;
                MulSpeedFlowScale =
                    state.dataVariableSpeedCoils->VarSpeedCoil(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).RatedAirVolFlowRate /
                    state.dataVariableSpeedCoils->VarSpeedCoil(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex)
                        .MSRatedAirVolFlowRate(
                            state.dataVariableSpeedCoils->VarSpeedCoil(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).NormSpedLevel);
                IHPCoilIndex = state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex;
            }

            for (Iter = 1; Iter <= state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling; ++Iter) {
                state.dataFurnaces->Furnace(FurnaceNum).CoolVolumeFlowRate(Iter) =
                    state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedAirVolFlowRate(Iter) * MulSpeedFlowScale;
                state.dataFurnaces->Furnace(FurnaceNum).CoolMassFlowRate(Iter) =
                    state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedAirMassFlowRate(Iter) * MulSpeedFlowScale;
                state.dataFurnaces->Furnace(FurnaceNum).MSCoolingSpeedRatio(Iter) =
                    state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedAirVolFlowRate(Iter) /
                    state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex)
                        .MSRatedAirVolFlowRate(state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling);
            }

            if (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingWaterToAirHPVSEquationFit ||
                state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingAirToAirVariableSpeed) {

                if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                    SizeIHP(state, state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex);
                    IHPCoilIndex = state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).SHCoilIndex;
                    state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedHeating = state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).NumOfSpeeds;
                    MulSpeedFlowScale = state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).RatedAirVolFlowRate /
                                        state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex)
                                            .MSRatedAirVolFlowRate(state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).NormSpedLevel);
                    state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).HeatVolFlowScale =
                        MulSpeedFlowScale;
                } else {
                    SimVariableSpeedCoils(state,
                                          BlankString,
                                          state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex,
                                          0,
                                          state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                                          state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                                          state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                                          CompressorOperation::Off,
                                          0.0,
                                          1,
                                          0.0,
                                          0.0,
                                          0.0,
                                          0.0); // conduct the sizing operation in the VS WSHP
                    state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedHeating =
                        state.dataVariableSpeedCoils->VarSpeedCoil(state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex).NumOfSpeeds;
                    MulSpeedFlowScale =
                        state.dataVariableSpeedCoils->VarSpeedCoil(state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex).RatedAirVolFlowRate /
                        state.dataVariableSpeedCoils->VarSpeedCoil(state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex)
                            .MSRatedAirVolFlowRate(
                                state.dataVariableSpeedCoils->VarSpeedCoil(state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex).NormSpedLevel);
                    IHPCoilIndex = state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex;
                }

                for (Iter = 1; Iter <= state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedHeating; ++Iter) {
                    state.dataFurnaces->Furnace(FurnaceNum).HeatVolumeFlowRate(Iter) =
                        state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedAirVolFlowRate(Iter) * MulSpeedFlowScale;
                    state.dataFurnaces->Furnace(FurnaceNum).HeatMassFlowRate(Iter) =
                        state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedAirMassFlowRate(Iter) * MulSpeedFlowScale;
                    state.dataFurnaces->Furnace(FurnaceNum).MSHeatingSpeedRatio(Iter) =
                        state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedAirVolFlowRate(Iter) /
                        state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex)
                            .MSRatedAirVolFlowRate(state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedHeating);
                }
            }

            if (state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedHeating > 0) {
                state.dataFurnaces->Furnace(FurnaceNum).IdleMassFlowRate =
                    min(state.dataFurnaces->Furnace(FurnaceNum).HeatMassFlowRate(1), state.dataFurnaces->Furnace(FurnaceNum).CoolMassFlowRate(1));
                state.dataFurnaces->Furnace(FurnaceNum).IdleSpeedRatio = min(state.dataFurnaces->Furnace(FurnaceNum).MSHeatingSpeedRatio(1),
                                                                             state.dataFurnaces->Furnace(FurnaceNum).MSCoolingSpeedRatio(1));
                state.dataFurnaces->Furnace(FurnaceNum).IdleVolumeAirRate =
                    min(state.dataFurnaces->Furnace(FurnaceNum).HeatVolumeFlowRate(1), state.dataFurnaces->Furnace(FurnaceNum).CoolVolumeFlowRate(1));
            } else {
                state.dataFurnaces->Furnace(FurnaceNum).IdleMassFlowRate = state.dataFurnaces->Furnace(FurnaceNum).CoolMassFlowRate(1);
                state.dataFurnaces->Furnace(FurnaceNum).IdleSpeedRatio = state.dataFurnaces->Furnace(FurnaceNum).MSCoolingSpeedRatio(1);
                state.dataFurnaces->Furnace(FurnaceNum).IdleVolumeAirRate = state.dataFurnaces->Furnace(FurnaceNum).CoolVolumeFlowRate(1);
            }

            if (state.dataFurnaces->Furnace(FurnaceNum).OpMode == ContFanCycCoil) {
                state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlow = state.dataFurnaces->Furnace(FurnaceNum).IdleVolumeAirRate;
                state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirMassFlow = state.dataFurnaces->Furnace(FurnaceNum).IdleMassFlowRate;
                state.dataFurnaces->Furnace(FurnaceNum).NoHeatCoolSpeedRatio = state.dataFurnaces->Furnace(FurnaceNum).IdleSpeedRatio;
            }
        }

        if (state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate == AutoSize) {

            if (state.dataSize->CurSysNum > 0) {

                CheckSysSizing(
                    state, cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num), state.dataFurnaces->Furnace(FurnaceNum).Name);
                if (state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow >= SmallAirVolFlow) {
                    state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate =
                        state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow;
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate = 0.0;
                }

                if (state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRateEMSOverrideOn) {
                    state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate =
                        state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRateEMSOverrideValue;
                }

                BaseSizer::reportSizerOutput(state,
                                             cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                             state.dataFurnaces->Furnace(FurnaceNum).Name,
                                             "Supply Air Flow Rate [m3/s]",
                                             state.dataFurnaces->Furnace(FurnaceNum).DesignFanVolFlowRate);
            }
        }

        if (state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow == AutoSize) {

            if (state.dataSize->CurSysNum > 0) {

                CheckSysSizing(
                    state, cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num), state.dataFurnaces->Furnace(FurnaceNum).Name);
                if (state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow >= SmallAirVolFlow) {
                    state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow =
                        state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow;
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow = 0.0;
                }

                if (state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlowEMSOverrideOn) {
                    state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow =
                        state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlowEMSOverrideValue;
                }
                BaseSizer::reportSizerOutput(state,
                                             cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                             state.dataFurnaces->Furnace(FurnaceNum).Name,
                                             "Supply Air Flow Rate During Heating Operation [m3/s]",
                                             state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirVolFlow);
            }
        }

        if (state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow == AutoSize) {

            if (state.dataSize->CurSysNum > 0) {

                CheckSysSizing(
                    state, cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num), state.dataFurnaces->Furnace(FurnaceNum).Name);
                if (state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow >= SmallAirVolFlow) {
                    state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow =
                        state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow;
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow = 0.0;
                }

                if (state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlowEMSOverrideOn) {
                    state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow =
                        state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlowEMSOverrideValue;
                }

                BaseSizer::reportSizerOutput(state,
                                             cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                             state.dataFurnaces->Furnace(FurnaceNum).Name,
                                             "Supply Air Flow Rate During Cooling Operation [m3/s]",
                                             state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirVolFlow);
            }
        }

        if (state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlow == AutoSize) {

            if (state.dataSize->CurSysNum > 0) {

                CheckSysSizing(
                    state, cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num), state.dataFurnaces->Furnace(FurnaceNum).Name);
                if (state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow >= SmallAirVolFlow) {
                    state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlow =
                        state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow;
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlow = 0.0;
                }

                if (state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlowEMSOverrideOn) {
                    state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlow =
                        state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlowEMSOverrideValue;
                }

                BaseSizer::reportSizerOutput(state,
                                             cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                             state.dataFurnaces->Furnace(FurnaceNum).Name,
                                             "Supply Air Flow Rate When No Cooling or Heating is Needed [m3/s]",
                                             state.dataFurnaces->Furnace(FurnaceNum).MaxNoCoolHeatAirVolFlow);
            }
        }

        if (state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity == AutoSize) {

            if (state.dataSize->CurSysNum > 0) {

                if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_AirToAir ||
                    state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_WaterToAir) {

                    CheckSysSizing(
                        state, cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num), state.dataFurnaces->Furnace(FurnaceNum).Name);

                    if (state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num == Coil_HeatingWaterToAirHPSimple) {
                        state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity =
                            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex)
                                .RatedCapHeat;
                    } else {
                        state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity = state.dataSize->DXCoolCap;
                    }

                } else {

                    CheckSysSizing(
                        state, cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num), state.dataFurnaces->Furnace(FurnaceNum).Name);

                    state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).HeatCap;
                    if (state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).FractionOfAutosizedHeatingCapacity > 0) {
                        // apply sizing factor and reset it to 1.0 so it doesn't get apply again during heating coil sizing
                        state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity *=
                            state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).FractionOfAutosizedHeatingCapacity;
                        state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).FractionOfAutosizedHeatingCapacity = 1.0;
                    }
                }

                if (state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity < SmallLoad) {
                    state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity = 0.0;
                }

                BaseSizer::reportSizerOutput(state,
                                             cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                             state.dataFurnaces->Furnace(FurnaceNum).Name,
                                             "Nominal Heating Capacity [W]",
                                             state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity);
            }
        }

        if (state.dataFurnaces->Furnace(FurnaceNum).DesignCoolingCapacity == AutoSize) {

            if (state.dataSize->CurSysNum > 0) {

                CheckSysSizing(
                    state, cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num), state.dataFurnaces->Furnace(FurnaceNum).Name);
                if (state.dataSize->DXCoolCap >= SmallLoad) {
                    state.dataFurnaces->Furnace(FurnaceNum).DesignCoolingCapacity = state.dataSize->DXCoolCap;
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).DesignCoolingCapacity = 0.0;
                }
                BaseSizer::reportSizerOutput(state,
                                             cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                             state.dataFurnaces->Furnace(FurnaceNum).Name,
                                             "Nominal Cooling Capacity [W]",
                                             state.dataFurnaces->Furnace(FurnaceNum).DesignCoolingCapacity);
            }
        }

        if (state.dataFurnaces->Furnace(FurnaceNum).DesignMaxOutletTemp == AutoSize) {

            if (state.dataSize->CurSysNum > 0) {

                CheckSysSizing(
                    state, cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num), state.dataFurnaces->Furnace(FurnaceNum).Name);
                state.dataFurnaces->Furnace(FurnaceNum).DesignMaxOutletTemp = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).HeatSupTemp;
                BaseSizer::reportSizerOutput(state,
                                             cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                             state.dataFurnaces->Furnace(FurnaceNum).Name,
                                             "Maximum Supply Air Temperature from Supplemental Heater [C]",
                                             state.dataFurnaces->Furnace(FurnaceNum).DesignMaxOutletTemp);
            }
        }

        if (state.dataFurnaces->Furnace(FurnaceNum).DesignSuppHeatingCapacity == AutoSize) {

            if (state.dataSize->CurSysNum > 0) {

                CheckSysSizing(
                    state, cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num), state.dataFurnaces->Furnace(FurnaceNum).Name);
                if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_AirToAir ||
                    state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_WaterToAir) {
                    // set the supplemental heating capacity to the actual heating load
                    state.dataFurnaces->Furnace(FurnaceNum).DesignSuppHeatingCapacity =
                        state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).HeatCap;
                    // if reheat needed for humidity control, make sure supplemental heating is at least as big
                    // as the cooling capacity
                    if (state.dataFurnaces->Furnace(FurnaceNum).Humidistat &&
                        state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::CoolReheat) {
                        state.dataFurnaces->Furnace(FurnaceNum).DesignSuppHeatingCapacity =
                            max(state.dataFurnaces->Furnace(FurnaceNum).DesignSuppHeatingCapacity,
                                state.dataFurnaces->Furnace(FurnaceNum).DesignCoolingCapacity);
                        if (state.dataFurnaces->Furnace(FurnaceNum).DesignSuppHeatingCapacity < SmallLoad) {
                            state.dataFurnaces->Furnace(FurnaceNum).DesignSuppHeatingCapacity = 0.0;
                        }
                    }

                } else {

                    if (state.dataFurnaces->Furnace(FurnaceNum).Humidistat &&
                        state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::CoolReheat) {
                        state.dataFurnaces->Furnace(FurnaceNum).DesignSuppHeatingCapacity =
                            state.dataFurnaces->Furnace(FurnaceNum).DesignCoolingCapacity;
                    } else {
                        state.dataFurnaces->Furnace(FurnaceNum).DesignSuppHeatingCapacity = 0.0;
                    }
                }

                BaseSizer::reportSizerOutput(state,
                                             cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                             state.dataFurnaces->Furnace(FurnaceNum).Name,
                                             "Supplemental Heating Coil Nominal Capacity [W]",
                                             state.dataFurnaces->Furnace(FurnaceNum).DesignSuppHeatingCapacity);
            }
        }

        state.dataSize->UnitaryHeatCap = state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity;
        state.dataSize->SuppHeatCap = state.dataFurnaces->Furnace(FurnaceNum).DesignSuppHeatingCapacity;
    }

    // End Initialization Section of the Module
    //******************************************************************************

    // Beginning of Update subroutines for the Furnace Module
    // *****************************************************************************

    void CalcNewZoneHeatOnlyFlowRates(EnergyPlusData &state,
                                      int const FurnaceNum,          // Index to furnace
                                      bool const FirstHVACIteration, // Iteration flag
                                      Real64 const ZoneLoad,         // load to be met by furnace (W)
                                      Real64 &HeatCoilLoad,          // actual load passed to heating coil (W)
                                      Real64 &OnOffAirFlowRatio      // ratio of coil on to coil off air flow rate
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   Feb 2001
        //       MODIFIED       Don Shirey and R. Raustad, Mar 2001 & Mar 2003
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the coil outlet nodes by simulating a heat-only
        // furnace or unitary system.

        // METHODOLOGY EMPLOYED:
        // Determine the operating PLR to meet the zone sensible load.

        // REFERENCES:
        // na

        // Using/Aliasing
        using namespace ScheduleManager;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        int constexpr MaxIter(15);    // maximum number of iterations
        Real64 constexpr MinPLR(0.0); // minimum part load ratio allowed

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 Error(1.0);
        Real64 SystemSensibleLoad;   // Sensible load to be met by furnace (W)
        Real64 FullSensibleOutput;   // Full sensible output of furnace (W)
        Real64 FullLatentOutput;     // Full latent output of furnace = 0 (W)
        Real64 NoSensibleOutput;     // Sensible output of furnace with no heating allowed (W)
        Real64 NoLatentOutput;       // Latent output of furnace = 0 (W)
        Real64 PartLoadRatio;        // Part load ratio of furnace
        Real64 HeatErrorToler;       // Error tolerance in heating mode
        Real64 IterRelax;            // Relaxation factor for iterations
        Real64 ActualSensibleOutput; // Actual furnace sensible capacity
        Real64 ActualLatentOutput;   // Actual furnace latent capacity = 0
        Real64 deltaT;               // Heater outlet temp minus design heater outlet temp
        //  CHARACTER(len=20) :: ErrNum = ' '         ! For displaying error message in cooling
        //  INTEGER,SAVE      :: ErrCount = 0
        int FurnaceInletNode;  // Node number of furnace inlet
        int FurnaceOutletNode; // Node number of furnace outlet
        int OpMode;            // Mode of Operation (fan cycling or fan continuous)
        // Set local variables

        // Retrieve the load on the controlled zone
        FurnaceOutletNode = state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum;
        FurnaceInletNode = state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum;
        int ControlZoneNode = state.dataFurnaces->Furnace(FurnaceNum).NodeNumOfControlledZone;
        OpMode = state.dataFurnaces->Furnace(FurnaceNum).OpMode;
        state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace = state.dataFurnaces->Furnace(FurnaceNum).DesignMassFlowRate;
        state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio = 0.0;
        //  OnOffAirFlowRatio = 1.0

        auto &Node = state.dataLoopNodes->Node;
        // Calculate the Cp Air of zone
        Real64 cpair = PsyCpAirFnW(Node(ControlZoneNode).HumRat);

        if (FirstHVACIteration) {
            HeatCoilLoad = ZoneLoad;
            state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
            Node(FurnaceInletNode).MassFlowRate = state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace;
        } else {
            // If Furnace runs then set HeatCoilLoad on Heating Coil and the Mass Flow
            if ((GetCurrentScheduleValue(state, state.dataFurnaces->Furnace(FurnaceNum).SchedPtr) > 0.0) &&
                (Node(FurnaceInletNode).MassFlowRate > 0.0) && (state.dataFurnaces->HeatingLoad)) {

                Node(FurnaceInletNode).MassFlowRate = state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace;
                HeatCoilLoad = state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity;
                SystemSensibleLoad = ZoneLoad;

                // Get no load result
                if (OpMode == CycFanCycCoil) {
                    Node(FurnaceInletNode).MassFlowRate = 0.0;
                }
                if (OpMode == ContFanCycCoil) {
                    state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0; // The on/off fan will not cycle, so set part-load fraction = 1
                }

                //     Set the inlet mass flow rate based on user specified coil OFF flow rate
                PartLoadRatio = 0.0;
                SetAverageAirFlow(state, FurnaceNum, PartLoadRatio, OnOffAirFlowRatio);

                CalcFurnaceOutput(state,
                                  FurnaceNum,
                                  FirstHVACIteration,
                                  OpMode,
                                  CompressorOperation::On,
                                  0.0,
                                  0.0,
                                  0.0,
                                  0.0,
                                  NoSensibleOutput,
                                  NoLatentOutput,
                                  OnOffAirFlowRatio,
                                  false);

                Node(FurnaceInletNode).MassFlowRate = state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace;

                // Set fan part-load fraction equal to 1 while getting full load result
                state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
                OnOffAirFlowRatio = 1.0;

                // Get full load result
                CalcFurnaceOutput(state,
                                  FurnaceNum,
                                  FirstHVACIteration,
                                  OpMode,
                                  CompressorOperation::On,
                                  0.0,
                                  1.0,
                                  HeatCoilLoad,
                                  0.0,
                                  FullSensibleOutput,
                                  FullLatentOutput,
                                  OnOffAirFlowRatio,
                                  false);

                // Since we are heating, we expect FullSensibleOutput to be > 0 and FullSensibleOutput > NoSensibleOutput
                // Check that this is the case; if not set PartLoadRatio = 0.0d0 (off) and return

                if (FullSensibleOutput > NoSensibleOutput) {
                    PartLoadRatio =
                        max(MinPLR, min(1.0, std::abs(SystemSensibleLoad - NoSensibleOutput) / std::abs(FullSensibleOutput - NoSensibleOutput)));
                    if (OpMode == CycFanCycCoil) {
                        Node(FurnaceInletNode).MassFlowRate = state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace * PartLoadRatio;
                        HeatCoilLoad = state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity * PartLoadRatio;
                    } else { // ContFanCycCoil
                        if (Node(FurnaceOutletNode).Temp > state.dataFurnaces->Furnace(FurnaceNum).DesignMaxOutletTemp) {
                            deltaT = Node(FurnaceOutletNode).Temp - state.dataFurnaces->Furnace(FurnaceNum).DesignMaxOutletTemp;
                            if (HeatCoilLoad > state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity)
                                HeatCoilLoad = state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity;
                            HeatCoilLoad -= Node(FurnaceInletNode).MassFlowRate * cpair * deltaT;
                        } else {
                            HeatCoilLoad = SystemSensibleLoad - NoSensibleOutput;
                        }
                    }

                    // Calculate the part load ratio through iteration
                    HeatErrorToler =
                        state.dataFurnaces->Furnace(FurnaceNum).HeatingConvergenceTolerance; // Error tolerance for convergence from input deck
                    Error = 1.0;                  // initialize error value for comparison against tolerance
                    state.dataFurnaces->Iter = 0; // initialize iteration counter
                    IterRelax = 0.9;              // relaxation factor for iterations
                    while (state.dataFurnaces->Iter <= MaxIter) {

                        if (OpMode == CycFanCycCoil)
                            Node(FurnaceInletNode).MassFlowRate = state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace * PartLoadRatio;
                        CalcFurnaceOutput(state,
                                          FurnaceNum,
                                          FirstHVACIteration,
                                          OpMode,
                                          CompressorOperation::On,
                                          0.0,
                                          PartLoadRatio,
                                          HeatCoilLoad,
                                          0.0,
                                          ActualSensibleOutput,
                                          ActualLatentOutput,
                                          OnOffAirFlowRatio,
                                          false);

                        if (SystemSensibleLoad != 0.0) Error = (SystemSensibleLoad - ActualSensibleOutput) / (SystemSensibleLoad);
                        if (std::abs(Error) <= HeatErrorToler) break;
                        PartLoadRatio = max(
                            MinPLR,
                            min(1.0,
                                PartLoadRatio + IterRelax * (SystemSensibleLoad - ActualSensibleOutput) / (FullSensibleOutput - NoSensibleOutput)));

                        //        limit the heating coil outlet air temperature to DesignMaxOutletTemp
                        if (Node(FurnaceOutletNode).Temp > state.dataFurnaces->Furnace(FurnaceNum).DesignMaxOutletTemp) {
                            deltaT = Node(FurnaceOutletNode).Temp - state.dataFurnaces->Furnace(FurnaceNum).DesignMaxOutletTemp;
                            if (HeatCoilLoad > state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity)
                                HeatCoilLoad = state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity;
                            HeatCoilLoad -= Node(FurnaceInletNode).MassFlowRate * cpair * deltaT;
                            CalcFurnaceOutput(state,
                                              FurnaceNum,
                                              FirstHVACIteration,
                                              OpMode,
                                              CompressorOperation::On,
                                              0.0,
                                              PartLoadRatio,
                                              HeatCoilLoad,
                                              0.0,
                                              ActualSensibleOutput,
                                              ActualLatentOutput,
                                              OnOffAirFlowRatio,
                                              false);

                            if (SystemSensibleLoad != 0.0) Error = (SystemSensibleLoad - ActualSensibleOutput) / (SystemSensibleLoad);
                            PartLoadRatio = max(MinPLR,
                                                min(1.0,
                                                    PartLoadRatio + IterRelax * (SystemSensibleLoad - ActualSensibleOutput) /
                                                                        (FullSensibleOutput - NoSensibleOutput)));
                        } else {
                            HeatCoilLoad = state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity * PartLoadRatio;
                        }

                        if (PartLoadRatio == MinPLR) break;
                        if (PartLoadRatio == 1.0) break;
                        ++state.dataFurnaces->Iter;
                        if (state.dataFurnaces->Iter == 7) IterRelax = 0.7;
                        if (state.dataFurnaces->Iter == 15) IterRelax = 0.4;
                    }

                    if (state.dataFurnaces->Iter > MaxIter) {
                        if (state.dataFurnaces->Furnace(FurnaceNum).HeatingMaxIterIndex2 == 0) {
                            ShowWarningMessage(state,
                                               format("{} \"{}\" -- Exceeded max heating iterations ({}) while adjusting furnace runtime.",
                                                      cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                                      state.dataFurnaces->Furnace(FurnaceNum).Name,
                                                      MaxIter));
                            ShowContinueErrorTimeStamp(state, "");
                        }
                        ShowRecurringWarningErrorAtEnd(state,
                                                       cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num) + " \"" +
                                                           state.dataFurnaces->Furnace(FurnaceNum).Name +
                                                           "\" -- Exceeded max heating iterations error continues...",
                                                       state.dataFurnaces->Furnace(FurnaceNum).HeatingMaxIterIndex2);
                    }

                } else { // ELSE from IF(FullSensibleOutput.GT.NoSensibleOutput)THEN above
                    // Set part load ratio to 1 and run heater at design heating capacity
                    PartLoadRatio = 1.0;
                    HeatCoilLoad = state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity;
                }
                // Set the final results
                //      IF (OpMode .EQ. CycFanCycCoil) THEN
                //        Furnace(FurnaceNum)%MdotFurnace = Furnace(FurnaceNum)%MdotFurnace * PartLoadRatio
                //      END IF
                state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace = Node(FurnaceInletNode).MassFlowRate;

            } else if ((GetCurrentScheduleValue(state, state.dataFurnaces->Furnace(FurnaceNum).SchedPtr) > 0.0) &&
                       (Node(FurnaceInletNode).MassFlowRate > 0.0) && (OpMode == ContFanCycCoil)) {
                HeatCoilLoad = 0.0;
            } else { // no heating and no flow
                state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace = 0.0;
                HeatCoilLoad = 0.0;
            } // End of the Scheduled Furnace If block

        } // End of the FirstHVACIteration control of the mass flow If block

        // Set the fan inlet node flow rates
        Node(FurnaceInletNode).MassFlowRateMaxAvail = state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace;
        Node(FurnaceInletNode).MassFlowRate = state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace;
    }

    void CalcNewZoneHeatCoolFlowRates(EnergyPlusData &state,
                                      int const FurnaceNum,
                                      bool const FirstHVACIteration,
                                      CompressorOperation const CompressorOp, // compressor operation flag (1=On, 0=Off)
                                      Real64 const ZoneLoad,                  // the control zone load (watts)
                                      Real64 const MoistureLoad,              // the control zone latent load (watts)
                                      Real64 &HeatCoilLoad,                   // Heating load to be met by heating coil ( excluding heat pump DX coil)
                                      Real64 &ReheatCoilLoad,    // Heating load to be met by reheat coil using hstat (excluding HP DX coil)
                                      Real64 &OnOffAirFlowRatio, // Ratio of compressor ON air flow to AVERAGE air flow over time step
                                      bool &HXUnitOn             // flag to control HX based on zone moisture load
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   Feb 2001
        //       MODIFIED       R. Raustad and D. Shirey, Feb/Mar/Sept/Oct/Dec 2001, Jan/Oct 2002
        //       RE-ENGINEERED  R. Raustad, Feb. 2005 (added RegulaFalsi for iteration technique)

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the coil outlet nodes.

        // METHODOLOGY EMPLOYED:
        // Determine the operating PLR to meet the zone sensible load. If a humidistat is specified, determine
        // the operating PLR (greater of the sensible and latent PLR) to meet the zone SENSIBLE load
        // (Multimode dehumidification control) or zone LATENT load (CoolReheat dehumidification control).
        // For dehumidification control type COOLREHEAT, both a sensible and latent PLR may exist for a
        // single time step (heating and dehumidificaiton can occur). For all other sytem types,
        // only a single PLR is allowed for any given time step.
        // Order of simulation depends on dehumidification control option as described below.
        // Dehumidificaiton control options:
        // Dehumidification Control NONE:   Cooling performance is simulated first and then heating performance. If a HX
        //                                  assisted cooling coil is selected, the HX is always active.
        // Dehumidification Control COOLREHEAT: Continuous Fan Operation:
        //                                      For cooling operation, the sensible and latent capacities are calculated to
        //                                      meet the thermostat setpoint. If a HX assisted cooling coil is selected,
        //                                      the HX is always active. If the latent load is not met by operating the
        //                                      system at the sensible PLR, a new PLR is calculated to meet the humidistat
        //                                      setpoint. The reheat coil load is then calculated to meet the HEATING
        //                                      setpoint temperature.
        //                                      Cycling Fan Operation:
        //                                      The heating part-load ratio is calculated first. Since the fan will be
        //                                      controlled at the higher of the heating or cooling PLR's, a ratio of the
        //                                      cooling to heating PLR is used to pass to the cooling coil (MAX=1). This allows
        //                                      the cooling coil to operate at the heating PLR when the heating PLR is
        //                                      higher than the cooling PLR. The sensible and latent capacities are then
        //                                      calculated to meet the thermostat setpoint.
        //                                      If a HX assisted cooling coil is selected, the HX is always active.
        //                                      If the latent load is not met by operating the system at the sensible PLR,
        //                                      a new PLR is calculated to meet the humidistat setpoint.
        //                                      The reheat coil load is then calculated to meet the HEATING setpoint temperature.
        // Dehumidification Control MULTIMODE: For cooling operation, the sensible and latent capacities are calculated to
        //                                     meet the thermostat setpoint. If a HX assisted cooling coil is selected,
        //                                     the HX is off for this calculation. If the latent load is not met by operating
        //                                     the system at the sensible PLR, a new PLR is calculated with the HX operating
        //                                     and the target is the thermostat setpoint. Humidity is not controlled in this
        //                                     mode. No reheat coil is used in this configuration.
        //  Note: A supplemental heater augments the heating capacity for air-to-air heat pumps.
        //        A reheat coil is used for the HeatCool furnace/unitarysystem to offset the sensible cooling when the
        //        dehumidification control type is COOLREHEAT. Both the supplemental and reheat heating coil load is calculated
        //        in the Calc routines. The actual simulation of these coils is performed in the SimFurnace routine (i.e. the
        //        supplemental and reheat coil loads are passed as 0 to CalcFurnaceOutput).

        // Using/Aliasing
        using namespace ScheduleManager;
        using namespace DataZoneEnergyDemands;

        // SUBROUTINE PARAMETER DEFINITIONS:
        int constexpr MaxIter(100);   // maximum number of iterations
        Real64 constexpr MinPLR(0.0); // minimum part load ratio allowed

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 SystemMoistureLoad;   // Total latent load to be removed by furnace/unitary system
        Real64 deltaT;               // Temperature rise across heating coil (C)
        Real64 TempOutHeatingCoil;   // Temperature leaving heating coil (C)
        Real64 FullSensibleOutput;   // Full sensible output of AC (W)
        Real64 FullLatentOutput;     // Full latent output of AC (W)
        Real64 NoCoolOutput;         // Sensible output of AC with no cooling allowed (W)
        Real64 NoHeatOutput;         // Sensible output of heater with no heating allowed (W)
        Real64 NoLatentOutput;       // Latent output of AC with no cooling allowed (W)
        int FurnaceInletNode;        // Inlet node to furnace or unitary system
        int FurnaceOutletNode;       // Outlet node of furnace or unitary system
        int OpMode;                  // Mode of Operation (fan cycling = 1 or fan continuous = 2)
        Real64 CoolErrorToler;       // Error tolerance in cooling mode
        Real64 HeatErrorToler;       // Error tolerance in heating mode
        Real64 ActualSensibleOutput; // Actual furnace sensible capacity
        Real64 ActualLatentOutput;   // Actual furnace latent capacity
        Real64 PartLoadRatio;        // Part load ratio (greater of sensible or latent part load ratio for cooling,
        // or heating PLR)
        Real64 LatentPartLoadRatio; // Part load ratio to meet dehumidification load
        Real64 TempCoolOutput;      // Temporary Sensible output of AC while iterating on PLR (W)
        Real64 TempHeatOutput;      // Temporary Sensible output of heating coil while iterating on PLR (W)
        Real64 TempLatentOutput;    // Temporary Latent output of AC at increasing PLR (W)
        //                                           ! (Temp variables are used to find min PLR for positive latent removal)
        std::array<Real64, 10> Par;    // parameters passed to RegulaFalsi function
        int SolFlag;                   // return flag from RegulaFalsi
        Real64 TempMinPLR;             // Temporary min latent PLR when hum control is required and iter is exceeded
        Real64 TempMinPLR2;            // Temporary min latent PLR when cyc fan hum control is required and iter is exceeded
        Real64 TempMaxPLR;             // Temporary max latent PLR when hum control is required and iter is exceeded
        Real64 QToHeatSetPt;           // Load required to meet heating setpoint temp (>0 is a heating load)
        Real64 CoolingHeatingPLRRatio; // ratio of cooling to heating PLR (MAX=1). Used in heating mode.
        Real64 HeatingSensibleOutput;
        Real64 HeatingLatentOutput;
        Real64 OutdoorDryBulbTemp; // secondary coil (condenser) entering dry bulb temperature

        auto &CoolCoilLoad = state.dataFurnaces->CoolCoilLoad;
        auto &SystemSensibleLoad = state.dataFurnaces->SystemSensibleLoad;
        auto &HumControl = state.dataFurnaces->HumControl;

        // Set local variables
        FurnaceOutletNode = state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum;
        FurnaceInletNode = state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum;
        int ControlZoneNode = state.dataFurnaces->Furnace(FurnaceNum).NodeNumOfControlledZone;
        OpMode = state.dataFurnaces->Furnace(FurnaceNum).OpMode;
        HumControl = false;
        // Calculate the Cp Air of zone
        Real64 cpair = PsyCpAirFnW(state.dataLoopNodes->Node(ControlZoneNode).HumRat);
        NoHeatOutput = 0.0;
        SystemSensibleLoad = 0.0;
        ReheatCoilLoad = 0.0;
        HeatCoilLoad = 0.0;
        ReheatCoilLoad = 0.0;
        PartLoadRatio = 0.0;

        if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_AirToAir) {
            if (state.dataDXCoils->DXCoil(state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex)
                    .IsSecondaryDXCoilInZone) { // assumes compressor is in same location as secondary coil
                OutdoorDryBulbTemp =
                    state.dataZoneTempPredictorCorrector
                        ->zoneHeatBalance(state.dataDXCoils->DXCoil(state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex).SecZonePtr)
                        .ZT;
            } else if (state.dataDXCoils->DXCoil(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).IsSecondaryDXCoilInZone) {
                OutdoorDryBulbTemp =
                    state.dataZoneTempPredictorCorrector
                        ->zoneHeatBalance(state.dataDXCoils->DXCoil(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).SecZonePtr)
                        .ZT;
            } else {
                if (state.dataFurnaces->Furnace(FurnaceNum).CondenserNodeNum > 0) {
                    OutdoorDryBulbTemp = state.dataLoopNodes->Node(state.dataFurnaces->Furnace(FurnaceNum).CondenserNodeNum).Temp;
                } else {
                    OutdoorDryBulbTemp = state.dataEnvrn->OutDryBulbTemp;
                }
            }
        } else {
            OutdoorDryBulbTemp = state.dataEnvrn->OutDryBulbTemp;
        }
        if (FirstHVACIteration) {
            // Set selected values during first HVAC iteration

            // Init for heating
            if (state.dataFurnaces->HeatingLoad) {
                if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_AirToAir ||
                    (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_WaterToAir &&
                     state.dataFurnaces->Furnace(FurnaceNum).WatertoAirHPType == WatertoAir_Simple)) {
                    state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio = 1.0;
                    HeatCoilLoad = 0.0;
                    state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand = 0.0;
                    state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand = 0.0;
                    state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilLatentDemand = 0.0;
                } else { // for furnaces
                    state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio = 0.0;
                    HeatCoilLoad = ZoneLoad;
                    state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace;
                    state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand = 0.0;
                    state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand = 0.0;
                    state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilLatentDemand = 0.0;
                }
                ReheatCoilLoad = 0.0;
                state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio = 0.0;

                // Init for cooling
            } else if (state.dataFurnaces->CoolingLoad) {
                // air to air heat pumps
                state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio = 1.0;
                state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio = 0.0;
                HeatCoilLoad = 0.0;
                ReheatCoilLoad = 0.0;

                // Init for moisture load only
            } else {
                state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio = 0.0;
                state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio = 0.0;
                HeatCoilLoad = 0.0;
                ReheatCoilLoad = 0.0;
                state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand = 0.0;
                state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand = 0.0;
                state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilLatentDemand = 0.0;
            }

            SetAverageAirFlow(
                state,
                FurnaceNum,
                max(state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio, state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio),
                OnOffAirFlowRatio);
            //  if dehumidification load exists (for heat pumps) turn on the supplmental heater
            if (state.dataFurnaces->HPDehumidificationLoadFlag) HumControl = true;
        } else { // not FirstHVACIteration
            // Init for heating
            if (state.dataFurnaces->HeatingLoad) {
                CoolCoilLoad = 0.0;
                if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_AirToAir ||
                    (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_WaterToAir &&
                     state.dataFurnaces->Furnace(FurnaceNum).WatertoAirHPType == WatertoAir_Simple)) {
                    SystemSensibleLoad = ZoneLoad;
                    SystemMoistureLoad = 0.0;
                    HeatCoilLoad = 0.0;
                    state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand = SystemSensibleLoad;
                    state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand = 0.0;
                    state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilLatentDemand = 0.0;
                } else {
                    SystemMoistureLoad = MoistureLoad;
                    HeatCoilLoad = ZoneLoad;
                }

                // Init for cooling
            } else if (state.dataFurnaces->CoolingLoad) {
                CoolCoilLoad = ZoneLoad;
                SystemMoistureLoad = MoistureLoad;
                HeatCoilLoad = 0.0;
                state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand = std::abs(CoolCoilLoad);
                state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilLatentDemand = std::abs(SystemMoistureLoad);
                state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand = 0.0;

                // Init for latent
            } else {
                SystemMoistureLoad = MoistureLoad;
                CoolCoilLoad = 0.0;
                HeatCoilLoad = 0.0;
                // set report variables
                state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand = 0.0;
                state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilLatentDemand = SystemMoistureLoad;
                state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand = 0.0;
            }
            HeatingSensibleOutput = 0.0;
            HeatingLatentOutput = 0.0;
            ReheatCoilLoad = 0.0;
            state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio = 0.0;
            state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio = 0.0;
            state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 0.0;
            state.dataFurnaces->Furnace(FurnaceNum).DehumidInducedHeatingDemandRate = 0.0;

            // When humidity control is used with cycling fan control and a heating load exists, if a moisture load
            // also exists, the heating PLR must be available for the cooling coil calculations.
            //*********** Heating Section ************
            // If Furnace runs with a heating load then set HeatCoilLoad on Heating Coil and the Mass Flow
            //         (Node(FurnaceInletNode)%MassFlowRate .gt. 0.0d0) .and. &
            if ((GetCurrentScheduleValue(state, state.dataFurnaces->Furnace(FurnaceNum).SchedPtr) > 0.0) && (state.dataFurnaces->HeatingLoad)) {

                //    Heat pumps only calculate a single PLR each time step (i.e. only cooling or heating allowed in a single time step)
                if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_AirToAir ||
                    (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_WaterToAir &&
                     state.dataFurnaces->Furnace(FurnaceNum).WatertoAirHPType == WatertoAir_Simple)) {

                    state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace;

                    // Get no load result
                    if (OpMode == CycFanCycCoil) {
                        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = 0.0;
                    }

                    //     Set the inlet mass flow rate based on user specified coil OFF flow rate
                    PartLoadRatio = 0.0;

                    SetAverageAirFlow(state, FurnaceNum, PartLoadRatio, OnOffAirFlowRatio);

                    // Set the input parameters for CalcFurnaceOutput
                    state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 0.0; // compressor off
                    state.dataFurnaces->Furnace(FurnaceNum).WSHPRuntimeFrac = 0.0;

                    CalcFurnaceOutput(state,
                                      FurnaceNum,
                                      FirstHVACIteration,
                                      OpMode,
                                      CompressorOp,
                                      0.0,
                                      MinPLR,
                                      0.0,
                                      0.0,
                                      NoHeatOutput,
                                      NoLatentOutput,
                                      OnOffAirFlowRatio,
                                      false);

                    PartLoadRatio = 1.0;
                    state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace;

                    state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 1.0; // compressor ON
                    state.dataFurnaces->Furnace(FurnaceNum).WSHPRuntimeFrac = 1.0;

                    // Set fan part-load fraction equal to 1 while getting full load result
                    state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
                    OnOffAirFlowRatio = 1.0;

                    // Get full load result
                    CalcFurnaceOutput(state,
                                      FurnaceNum,
                                      FirstHVACIteration,
                                      OpMode,
                                      CompressorOp,
                                      0.0,
                                      PartLoadRatio,
                                      0.0,
                                      0.0,
                                      FullSensibleOutput,
                                      FullLatentOutput,
                                      OnOffAirFlowRatio,
                                      false);

                    // Check that SystemSensibleLoad is between FullSensibleOutput and NoHeatOutput
                    // If so then calculate PartLoadRatio for the DX Heating coil
                    if (SystemSensibleLoad < FullSensibleOutput && SystemSensibleLoad > NoHeatOutput) {

                        // Calculate the part load ratio through iteration
                        HeatErrorToler =
                            state.dataFurnaces->Furnace(FurnaceNum).HeatingConvergenceTolerance; // Error tolerance for convergence from input deck

                        SolFlag = 0; // # of iterations if positive, -1 means failed to converge, -2 means bounds are incorrect
                        Par[0] = double(FurnaceNum);
                        Par[1] = 0.0; // FLAG, if 1.0 then FirstHVACIteration equals TRUE, if 0.0 then FirstHVACIteration equals false
                        if (FirstHVACIteration) Par[1] = 1.0;
                        Par[2] = double(OpMode);
                        Par[3] = double(CompressorOp);
                        Par[4] = SystemSensibleLoad;
                        Par[5] = 0.0;               // FLAG, 0.0 if heating load, 1.0 if cooling or moisture load
                        Par[6] = 1.0;               // FLAG, 0.0 if latent load, 1.0 if sensible load to be met
                        Par[7] = OnOffAirFlowRatio; // Ratio of compressor ON mass flow rate to AVERAGE mass flow rate over time step
                        Par[8] = 0.0;               // HXUnitOn is always false for HX
                        Par[9] = 0.0;
                        //         HeatErrorToler is in fraction of load, MaxIter = 30, SolFalg = # of iterations or error as appropriate
                        auto f = [&state, FurnaceNum, FirstHVACIteration, OpMode, CompressorOp, SystemSensibleLoad](Real64 const PartLoadRatio) {
                            return CalcFurnaceResidual(state,
                                                       PartLoadRatio,
                                                       FurnaceNum,
                                                       FirstHVACIteration,
                                                       OpMode,
                                                       CompressorOp,
                                                       SystemSensibleLoad,
                                                       0.0,  // par6_loadFlag,
                                                       1.0,  // par7_sensLatentFlag,
                                                       0.0,  // par9_HXOnFlag,
                                                       0.0); // par10_HeatingCoilPLR);
                        };
                        General::SolveRoot(state, HeatErrorToler, MaxIter, SolFlag, PartLoadRatio, f, 0.0, 1.0);
                        //         OnOffAirFlowRatio is updated during the above iteration. Reset to correct value based on PLR.
                        OnOffAirFlowRatio = state.dataFurnaces->OnOffAirFlowRatioSave;
                        if (SolFlag < 0) {
                            if (SolFlag == -1) {
                                CalcFurnaceOutput(state,
                                                  FurnaceNum,
                                                  FirstHVACIteration,
                                                  OpMode,
                                                  CompressorOp,
                                                  0.0,
                                                  PartLoadRatio,
                                                  0.0,
                                                  0.0,
                                                  TempHeatOutput,
                                                  TempLatentOutput,
                                                  OnOffAirFlowRatio,
                                                  false);
                                if (std::abs(SystemSensibleLoad - TempHeatOutput) > SmallLoad) {
                                    if (state.dataFurnaces->Furnace(FurnaceNum).DXHeatingMaxIterIndex == 0) {
                                        ShowWarningMessage(state,
                                                           format("Heating coil control failed to converge for {}:{}",
                                                                  cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                                                  state.dataFurnaces->Furnace(FurnaceNum).Name));
                                        ShowContinueError(state,
                                                          "  Iteration limit exceeded in calculating DX heating coil sensible part-load ratio.");
                                        ShowContinueErrorTimeStamp(
                                            state,
                                            format("Sensible load to be met by DX heating coil = {:.2T} (watts), sensible output of DX heating "
                                                   "coil = {:.2T} (watts), and the simulation continues.",
                                                   SystemSensibleLoad,
                                                   TempHeatOutput));
                                    }
                                    ShowRecurringWarningErrorAtEnd(
                                        state,
                                        cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num) + " \"" +
                                            state.dataFurnaces->Furnace(FurnaceNum).Name +
                                            "\" - Iteration limit exceeded in calculating DX sensible heating part-load ratio error continues. "
                                            "Sensible load statistics:",
                                        state.dataFurnaces->Furnace(FurnaceNum).DXHeatingMaxIterIndex,
                                        SystemSensibleLoad,
                                        SystemSensibleLoad);
                                }
                            } else if (SolFlag == -2) {
                                if (state.dataFurnaces->Furnace(FurnaceNum).DXHeatingRegulaFalsiFailedIndex == 0) {
                                    ShowWarningMessage(state,
                                                       format("Heating coil control failed for {}:{}",
                                                              cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                                              state.dataFurnaces->Furnace(FurnaceNum).Name));
                                    ShowContinueError(state, "  DX sensible heating part-load ratio determined to be outside the range of 0-1.");
                                    ShowContinueErrorTimeStamp(
                                        state,
                                        format("Sensible load to be met by DX heating coil = {:.2T} (watts), and the simulation continues.",
                                               SystemSensibleLoad));
                                }
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num) + " \"" +
                                        state.dataFurnaces->Furnace(FurnaceNum).Name +
                                        "\" -  DX sensible heating part-load ratio out of range error continues. Sensible load statistics:",
                                    state.dataFurnaces->Furnace(FurnaceNum).DXHeatingRegulaFalsiFailedIndex,
                                    SystemSensibleLoad,
                                    SystemSensibleLoad);
                            }
                        }

                        state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio = PartLoadRatio;
                        //       Check if Heat Pump compressor is allowed to run based on outdoor temperature
                        if (OutdoorDryBulbTemp > state.dataFurnaces->Furnace(FurnaceNum).MinOATCompressorHeating) {
                            state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = PartLoadRatio;
                        } else {
                            state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 0.0;
                        }
                    } else if (SystemSensibleLoad > FullSensibleOutput) {
                        //       SystemSensibleLoad is greater than full DX Heating coil output so heat pump runs entire
                        //       timestep and additional supplemental heating is required
                        state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio = 1.0;
                        if (OutdoorDryBulbTemp > state.dataFurnaces->Furnace(FurnaceNum).MinOATCompressorHeating) {
                            //       Check to see if Heat Pump compressor was allowed to run based on outdoor temperature
                            state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 1.0;
                        } else {
                            state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 0.0;
                        }
                    } else if (SystemSensibleLoad < NoHeatOutput) {
                        //       SystemSensibleLoad is less than minimum DX Heating coil output so heat pump does not run and
                        //       the load will be met by the supplemental heater
                        state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 0.0;
                        state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio = 1.0;
                    }
                    if (state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio == 1.0) {
                        //       Determine the load on the supplemental heating coil
                        if ((SystemSensibleLoad - FullSensibleOutput) > state.dataFurnaces->Furnace(FurnaceNum).DesignSuppHeatingCapacity) {
                            HeatCoilLoad = state.dataFurnaces->Furnace(FurnaceNum).DesignSuppHeatingCapacity;
                            TempOutHeatingCoil = state.dataLoopNodes->Node(FurnaceOutletNode).Temp +
                                                 HeatCoilLoad / (cpair * state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace);
                        } else if (SystemSensibleLoad < NoHeatOutput) {
                            HeatCoilLoad = max(0.0, SystemSensibleLoad); // BG 10/22/2008 need a case for when its all suppl heat
                            TempOutHeatingCoil = state.dataLoopNodes->Node(FurnaceInletNode).Temp +
                                                 HeatCoilLoad / (cpair * state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace);
                        } else {
                            HeatCoilLoad = max(0.0, (SystemSensibleLoad - FullSensibleOutput));
                            TempOutHeatingCoil = state.dataLoopNodes->Node(FurnaceOutletNode).Temp +
                                                 HeatCoilLoad / (cpair * state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace);
                        }
                        if (OutdoorDryBulbTemp > state.dataFurnaces->Furnace(FurnaceNum).MaxOATSuppHeat) {
                            HeatCoilLoad = 0.0;
                            if (SystemSensibleLoad < NoHeatOutput) {
                                TempOutHeatingCoil = state.dataLoopNodes->Node(FurnaceInletNode).Temp;
                            } else {
                                TempOutHeatingCoil = state.dataLoopNodes->Node(FurnaceOutletNode).Temp;
                            }
                        }
                        if ((TempOutHeatingCoil > state.dataFurnaces->Furnace(FurnaceNum).DesignMaxOutletTemp) && (HeatCoilLoad > 0.0)) {
                            // deltaT = Furnace(FurnaceNum)%DesignMaxOutletTemp - Node(FurnaceOutletNode)%Temp
                            // BG 10/22/2008 above made no sense if DX heat is off and its all supplemental,
                            //  because Node(FurnaceOutletNode)%Temp will have been calc'd with full DX heat in last faux call to CalcFurnaceOutput

                            Real64 cpairSupply = PsyCpAirFnW(state.dataLoopNodes->Node(FurnaceInletNode).HumRat);
                            deltaT = (state.dataFurnaces->Furnace(FurnaceNum).DesignMaxOutletTemp - TempOutHeatingCoil);
                            HeatCoilLoad += (state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate * cpairSupply * deltaT);
                            HeatCoilLoad = max(0.0, HeatCoilLoad);
                        }
                    } else {
                        HeatCoilLoad = 0.0;
                    }
                    PartLoadRatio = 0.0;

                    //   HeatCool systems can have both a sensible and latent PLR in a single time step
                    //   (i.e. both cooling and heating can occur in a single time step)
                } else { // else not a heatpump DX coil ** non-HP heating coils are not DX so testing if OutdoorDryBulbTemp < MinOATCompressorHeating
                         // is not necessary **

                    state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace;
                    HeatCoilLoad = state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity;
                    SystemSensibleLoad = ZoneLoad;

                    // Get no load result
                    if (OpMode == CycFanCycCoil) {
                        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = 0.0;
                    }
                    if (OpMode == ContFanCycCoil) {
                        state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0; // The on/off fan will not cycle, so set part-load fraction = 1
                    }

                    //     Set the inlet mass flow rate based on user specified coil OFF flow rate
                    PartLoadRatio = 0.0;
                    SetAverageAirFlow(state, FurnaceNum, PartLoadRatio, OnOffAirFlowRatio);

                    CalcFurnaceOutput(state,
                                      FurnaceNum,
                                      FirstHVACIteration,
                                      OpMode,
                                      CompressorOp,
                                      0.0,
                                      MinPLR,
                                      0.0,
                                      0.0,
                                      NoHeatOutput,
                                      NoLatentOutput,
                                      OnOffAirFlowRatio,
                                      false);

                    if (NoHeatOutput < SystemSensibleLoad) {
                        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace;

                        // Set fan part-load fraction equal to 1 while getting full load result
                        state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
                        OnOffAirFlowRatio = 1.0;

                        // Get full load result
                        CalcFurnaceOutput(state,
                                          FurnaceNum,
                                          FirstHVACIteration,
                                          OpMode,
                                          CompressorOp,
                                          0.0,
                                          1.0,
                                          HeatCoilLoad,
                                          0.0,
                                          FullSensibleOutput,
                                          FullLatentOutput,
                                          OnOffAirFlowRatio,
                                          false);
                    } else {
                        FullSensibleOutput = NoHeatOutput + 0.000000001;
                    }

                    // Since we are heating, we expect FullSensibleOutput to be > 0 and FullSensibleOutput > NoSensibleOutput
                    // Check that this is the case; if not set PartLoadRatio = 0.0 (off) and return

                    if (FullSensibleOutput > NoHeatOutput) {

                        //       check bounds on sensible output prior to iteration using RegulaFalsi
                        if (FullSensibleOutput <= SystemSensibleLoad) {
                            PartLoadRatio = 1.0;
                            //         save modified HeatCoilLoad in case it was reset because outlet temp > DesignMaxOutletTemp
                            if (state.dataFurnaces->ModifiedHeatCoilLoad > 0.0) {
                                HeatCoilLoad = state.dataFurnaces->ModifiedHeatCoilLoad;
                            } else {
                                HeatCoilLoad = state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity;
                            }
                        } else if (NoHeatOutput >= SystemSensibleLoad) {
                            PartLoadRatio = 0.0;
                            HeatCoilLoad = 0.0;
                        } else {

                            // Calculate the part load ratio through iteration
                            HeatErrorToler = state.dataFurnaces->Furnace(FurnaceNum)
                                                 .HeatingConvergenceTolerance; // Error tolerance for convergence from input deck

                            SolFlag = 0; // # of iterations if positive, -1 means failed to converge, -2 means bounds are incorrect
                            Par[0] = double(FurnaceNum);
                            Par[1] = 0.0; // FLAG, if 1.0 then FirstHVACIteration equals TRUE, if 0.0 then FirstHVACIteration equals false
                            if (FirstHVACIteration) Par[1] = 1.0;
                            Par[2] = double(OpMode);
                            Par[3] = double(CompressorOp);
                            Par[4] = SystemSensibleLoad;
                            Par[5] = 0.0;               // FLAG, 0.0 if heating load, 1.0 if cooling or moisture load
                            Par[6] = 1.0;               // FLAG, 0.0 if latent load, 1.0 if sensible load to be met
                            Par[7] = OnOffAirFlowRatio; // Ratio of compressor ON mass flow rate to AVERAGE mass flow rate over time step
                            Par[8] = 0.0;               // HXUnitOn is always false for HX
                            Par[9] = 0.0;
                            //         HeatErrorToler is in fraction load, MaxIter = 30, SolFalg = # of iterations or error as appropriate
                            auto f = [&state, FurnaceNum, FirstHVACIteration, OpMode, CompressorOp, SystemSensibleLoad](Real64 const PartLoadRatio) {
                                return CalcFurnaceResidual(state,
                                                           PartLoadRatio,
                                                           FurnaceNum,
                                                           FirstHVACIteration,
                                                           OpMode,
                                                           CompressorOp,
                                                           SystemSensibleLoad,
                                                           0.0,  // par6_loadFlag,
                                                           1.0,  // par7_sensLatentFlag,
                                                           0.0,  // par9_HXOnFlag,
                                                           0.0); // par10_HeatingCoilPLR);
                            };
                            General::SolveRoot(state, HeatErrorToler, MaxIter, SolFlag, PartLoadRatio, f, 0.0, 1.0);
                            //         OnOffAirFlowRatio is updated during the above iteration. Reset to correct value based on PLR.
                            OnOffAirFlowRatio = state.dataFurnaces->OnOffAirFlowRatioSave;
                            //         Reset HeatCoilLoad calculated in CalcFurnaceResidual (in case it was reset because output temp >
                            //         DesignMaxOutletTemp)
                            if (state.dataFurnaces->ModifiedHeatCoilLoad > 0.0) {
                                HeatCoilLoad = state.dataFurnaces->ModifiedHeatCoilLoad;
                            } else {
                                HeatCoilLoad = state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity * PartLoadRatio;
                            }
                            if (SolFlag == -1) {

                                //           RegulaFalsi may not find heating PLR when the maximum supply air temperature is exceeded.
                                //           If iteration limit is exceeded, find tighter boundary of solution and repeat RegulaFalsi
                                TempMaxPLR = -0.1;
                                TempHeatOutput = NoHeatOutput;
                                while ((TempHeatOutput - SystemSensibleLoad) < 0.0 && TempMaxPLR < 1.0) {
                                    //             find upper limit of HeatingPLR
                                    TempMaxPLR += 0.1;
                                    HeatCoilLoad = state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity * TempMaxPLR;
                                    CalcFurnaceOutput(state,
                                                      FurnaceNum,
                                                      FirstHVACIteration,
                                                      OpMode,
                                                      CompressorOp,
                                                      0.0,
                                                      TempMaxPLR,
                                                      HeatCoilLoad,
                                                      0.0,
                                                      TempHeatOutput,
                                                      TempLatentOutput,
                                                      OnOffAirFlowRatio,
                                                      false);
                                }
                                TempMinPLR = TempMaxPLR;
                                while ((TempHeatOutput - SystemSensibleLoad) > 0.0 && TempMinPLR > 0.0) {
                                    //             pull upper limit of HeatingPLR down to last valid limit (i.e. heat output still exceeds
                                    //             SystemSensibleLoad)
                                    TempMaxPLR = TempMinPLR;
                                    //             find minimum limit of HeatingPLR
                                    TempMinPLR -= 0.01;

                                    HeatCoilLoad = state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity * TempMinPLR;
                                    CalcFurnaceOutput(state,
                                                      FurnaceNum,
                                                      FirstHVACIteration,
                                                      OpMode,
                                                      CompressorOp,
                                                      0.0,
                                                      TempMinPLR,
                                                      HeatCoilLoad,
                                                      0.0,
                                                      TempHeatOutput,
                                                      TempLatentOutput,
                                                      OnOffAirFlowRatio,
                                                      false);
                                }
                                //           Now solve again with tighter PLR limits
                                auto f = // (AUTO_OK_LAMBDA)
                                    [&state, FurnaceNum, FirstHVACIteration, OpMode, CompressorOp, SystemSensibleLoad](Real64 const PartLoadRatio) {
                                        return CalcFurnaceResidual(state,
                                                                   PartLoadRatio,
                                                                   FurnaceNum,
                                                                   FirstHVACIteration,
                                                                   OpMode,
                                                                   CompressorOp,
                                                                   SystemSensibleLoad,
                                                                   0.0,  // par6_loadFlag,
                                                                   1.0,  // par7_sensLatentFlag,
                                                                   0.0,  // par9_HXOnFlag,
                                                                   0.0); // par10_HeatingCoilPLR);
                                    };
                                General::SolveRoot(state, HeatErrorToler, MaxIter, SolFlag, PartLoadRatio, f, TempMinPLR, TempMaxPLR);
                                if (state.dataFurnaces->ModifiedHeatCoilLoad > 0.0) {
                                    HeatCoilLoad = state.dataFurnaces->ModifiedHeatCoilLoad;
                                } else {
                                    HeatCoilLoad = state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity * PartLoadRatio;
                                }
                                CalcFurnaceOutput(state,
                                                  FurnaceNum,
                                                  FirstHVACIteration,
                                                  OpMode,
                                                  CompressorOp,
                                                  0.0,
                                                  PartLoadRatio,
                                                  HeatCoilLoad,
                                                  0.0,
                                                  TempHeatOutput,
                                                  TempLatentOutput,
                                                  OnOffAirFlowRatio,
                                                  false);

                                //           After iterating with tighter boundaries, if still out of tolerance, show warning.
                                if (SolFlag == -1 && std::abs(SystemSensibleLoad - TempHeatOutput) > SmallLoad) {
                                    if (state.dataFurnaces->Furnace(FurnaceNum).HeatingMaxIterIndex == 0) {
                                        ShowWarningMessage(state,
                                                           format("Heating coil control failed to converge for {}:{}",
                                                                  cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                                                  state.dataFurnaces->Furnace(FurnaceNum).Name));
                                        ShowContinueError(state, "  Iteration limit exceeded in calculating heating coil sensible part-load ratio.");
                                        ShowContinueErrorTimeStamp(state,
                                                                   format("Sensible load to be met by heating coil = {:.2T} (watts), sensible output "
                                                                          "of heating coil = {:.2T} (watts), and the simulation continues.",
                                                                          SystemSensibleLoad,
                                                                          TempHeatOutput));
                                    }
                                    ShowRecurringWarningErrorAtEnd(state,
                                                                   cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num) + " \"" +
                                                                       state.dataFurnaces->Furnace(FurnaceNum).Name +
                                                                       "\" - Iteration limit exceeded in calculating sensible heating part-load "
                                                                       "ratio error continues. Sensible load statistics:",
                                                                   state.dataFurnaces->Furnace(FurnaceNum).HeatingMaxIterIndex,
                                                                   SystemSensibleLoad,
                                                                   SystemSensibleLoad);
                                }
                            } else if (SolFlag == -2) {
                                if (state.dataFurnaces->Furnace(FurnaceNum).HeatingRegulaFalsiFailedIndex == 0) {
                                    ShowWarningMessage(state,
                                                       format("Heating coil control failed for {}:{}",
                                                              cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                                              state.dataFurnaces->Furnace(FurnaceNum).Name));
                                    ShowContinueError(state, "  Sensible heating part-load ratio determined to be outside the range of 0-1.");
                                    ShowContinueErrorTimeStamp(
                                        state,
                                        format("Sensible load to be met by heating coil = {:.2T} (watts), and the simulation continues.",
                                               SystemSensibleLoad));
                                }
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num) + " \"" +
                                        state.dataFurnaces->Furnace(FurnaceNum).Name +
                                        "\" -  Sensible heating part-load ratio out of range error continues. Sensible load statistics:",
                                    state.dataFurnaces->Furnace(FurnaceNum).HeatingRegulaFalsiFailedIndex,
                                    SystemSensibleLoad,
                                    SystemSensibleLoad);
                            }
                        }

                    } else { // ELSE from IF(FullSensibleOutput.GT.NoSensibleOutput)THEN above
                        // Set part load ratio to 1 and run heater at design heating capacity
                        PartLoadRatio = 1.0;
                        HeatCoilLoad = state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity;
                    }

                } // End of IF HeatPump

            } // End of IF for heating

            // Non-heat pump systems do not set a heating PLR, set it here for use with the DX cooling coil calculations.
            // Set this variable back to 0 for non-heat pump systems at the end of this routine.
            state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio = max(PartLoadRatio, state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio);
            CalcFurnaceOutput(state,
                              FurnaceNum,
                              FirstHVACIteration,
                              OpMode,
                              CompressorOp,
                              0.0,
                              state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio,
                              HeatCoilLoad,
                              0.0,
                              HeatingSensibleOutput,
                              HeatingLatentOutput,
                              OnOffAirFlowRatio,
                              false);

            if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_AirToAir ||
                (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_WaterToAir &&
                 state.dataFurnaces->Furnace(FurnaceNum).WatertoAirHPType == WatertoAir_Simple && state.dataFurnaces->CoolingLoad)) {
                HeatingSensibleOutput = 0.0;
                HeatingLatentOutput = 0.0;
            }
            //***********Cooling Section*****************
            // Simulate if scheduled ON and cooling load or if a moisture load exists when using a humidistat
            // Check of HeatingLatentOutput is used to reduce overshoot during simultaneous heating and cooling
            // Setback flag is used to avoid continued RH control when Tstat is setback (RH should float down)
            if ((GetCurrentScheduleValue(state, state.dataFurnaces->Furnace(FurnaceNum).SchedPtr) > 0.0 && state.dataFurnaces->CoolingLoad) ||
                (state.dataFurnaces->Furnace(FurnaceNum).Humidistat &&
                 state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::CoolReheat &&
                 (SystemMoistureLoad < 0.0 || (SystemMoistureLoad >= 0.0 && HeatingLatentOutput > SystemMoistureLoad &&
                                               !state.dataZoneEnergyDemand->Setback(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum))))) {

                //     For cooling operation, the first step is to set the HX operation flag in case a HX assisted coil is used.
                //      (if a HX assisted coil is not used, this flag is not used. It's only used in the CALL to SimHXAssistedCoolingCoil)
                //     Check the dehumidification control type:
                //           For dehumidification control options CoolReheat and None, the HX is always active (locked ON).
                //           For dehumidification control option Multimode, the system is operated first with the HX off.
                //           If the moisture load is not met, the HX will then be turned on and the system is re-simulated.

                if (state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::CoolReheat ||
                    state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::None) {
                    HXUnitOn = true;
                } else {
                    HXUnitOn = false;
                }

                //     The next step is to determine the system output at no load (PLR=0) and full load (PLR=1)

                //     Set the inlet mass flow rate based on user specified coil OFF flow rate
                PartLoadRatio = 0.0;

                state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 0.0; // compressor off
                state.dataFurnaces->Furnace(FurnaceNum).WSHPRuntimeFrac = 0.0;

                //     SetAverageAirFlow calculates the operating mass flow rate based on PLR and the user specified inputs
                //     for MaxCoolAirMassFlow and MaxNoCoolHeatAirMassFlow.
                //     Air flow rate is set according to max of cooling and heating PLR if heating and latent load exists.
                if (OpMode == CycFanCycCoil && state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio > 0.0 &&
                    state.dataFurnaces->Furnace(FurnaceNum).Humidistat &&
                    state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::CoolReheat &&
                    (SystemMoistureLoad < 0.0 || (SystemMoistureLoad >= 0.0 && HeatingLatentOutput > SystemMoistureLoad &&
                                                  !state.dataZoneEnergyDemand->Setback(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)))) {
                    CoolingHeatingPLRRatio = min(1.0, PartLoadRatio / state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio);
                    SetAverageAirFlow(
                        state, FurnaceNum, max(PartLoadRatio, state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio), OnOffAirFlowRatio);

                } else {
                    CoolingHeatingPLRRatio = 1.0;
                    SetAverageAirFlow(state, FurnaceNum, PartLoadRatio, OnOffAirFlowRatio);
                }

                // Get no load result (coils simulated OFF)
                CalcFurnaceOutput(state,
                                  FurnaceNum,
                                  FirstHVACIteration,
                                  OpMode,
                                  CompressorOp,
                                  MinPLR,
                                  PartLoadRatio,
                                  0.0,
                                  0.0,
                                  NoCoolOutput,
                                  NoLatentOutput,
                                  OnOffAirFlowRatio,
                                  HXUnitOn,
                                  CoolingHeatingPLRRatio);

                //     Don't calculate full load output if no load output can meet sensible load
                if (NoCoolOutput >= CoolCoilLoad && (CoolCoilLoad != 0.0 || state.dataFurnaces->HPDehumidificationLoadFlag)) {
                    //       Set full mass flow rate for full load calculation
                    state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace;

                    // Set fan part-load fraction equal to 1 while getting full load result
                    state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
                    OnOffAirFlowRatio = 1.0;
                    PartLoadRatio = 1.0;
                    state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 1.0; // compressor ON
                    state.dataFurnaces->Furnace(FurnaceNum).WSHPRuntimeFrac = 1.0;

                    // Get full load result (coils simulated full ON)
                    CalcFurnaceOutput(state,
                                      FurnaceNum,
                                      FirstHVACIteration,
                                      OpMode,
                                      CompressorOp,
                                      PartLoadRatio,
                                      0.0,
                                      0.0,
                                      0.0,
                                      FullSensibleOutput,
                                      FullLatentOutput,
                                      OnOffAirFlowRatio,
                                      HXUnitOn);
                } else {
                    FullSensibleOutput = NoCoolOutput - 0.00000001;
                }

                //     The next step is to compare the results of the full load and no load results
                //     1) Since we are cooling, we expect FullSensibleOutput < NoCoolOutput
                //        Check that this is the case; if not set PartLoadRatio = 0.0 (off)
                //     2) Verify that the load to be met is within the range of available output
                //        (i.e. between FullSensibleOutput and NoCoolOutput)
                //     3) Set PLR if load is out of range or RegulaFalsi on PLR if system can meet the load
                if (FullSensibleOutput < NoCoolOutput) {
                    if (CoolCoilLoad != 0.0 || state.dataFurnaces->HPDehumidificationLoadFlag) {

                        //           check bounds on sensible output prior to iteration using RegulaFalsi
                        //           Negative value represents cooling load, IF FullSensibleOutput .GT. CoolCoilLoad, load is greater than capacity
                        if (FullSensibleOutput >= CoolCoilLoad) {
                            PartLoadRatio = 1.0;
                            //           Likewise IF NoCoolOutput .LT. CoolCoilLoad, then load can be met using only the fan (constant fan mode only)
                        } else if (NoCoolOutput <= CoolCoilLoad) {
                            PartLoadRatio = 0.0;
                            //           ELSE load is between NoCoolOutput and FullSensibleOuput, find PLR required to meet load
                        } else {

                            // Calculate the sensible part load ratio through iteration
                            CoolErrorToler = state.dataFurnaces->Furnace(FurnaceNum)
                                                 .CoolingConvergenceTolerance; // Error tolerance for convergence from input deck
                            SolFlag = 0; // # of iterations if positive, -1 means failed to converge, -2 means bounds are incorrect
                            Par[0] = double(FurnaceNum);
                            Par[1] = 0.0; // FLAG, if 1.0 then FirstHVACIteration equals TRUE, if 0.0 then FirstHVACIteration equals false
                            if (FirstHVACIteration) Par[1] = 1.0;
                            Par[2] = double(OpMode);
                            Par[3] = double(CompressorOp);
                            Par[4] = CoolCoilLoad;
                            Par[5] = 1.0;               // FLAG, 0.0 if heating load, 1.0 if cooling or moisture load
                            Par[6] = 1.0;               // FLAG, 0.0 if latent load, 1.0 if sensible load to be met
                            Par[7] = OnOffAirFlowRatio; // Ratio of compressor ON mass flow rate to AVERAGE mass flow rate over time step
                            Real64 par8_HXFlag = HXUnitOn ? 1.0 : 0.0;
                            if (HXUnitOn) {
                                Par[8] = 1.0;
                            } else {
                                Par[8] = 0.0;
                            }
                            //             Par(10) is the heating coil PLR, set this value to 0 for sensible PLR calculations.
                            Par[9] = 0.0;
                            //             CoolErrorToler is in fraction of load, MaxIter = 30, SolFalg = # of iterations or error as appropriate
                            auto f = [&state, FurnaceNum, FirstHVACIteration, OpMode, CompressorOp, CoolCoilLoad, par8_HXFlag](
                                         Real64 const PartLoadRatio) {
                                return CalcFurnaceResidual(state,
                                                           PartLoadRatio,
                                                           FurnaceNum,
                                                           FirstHVACIteration,
                                                           OpMode,
                                                           CompressorOp,
                                                           CoolCoilLoad,
                                                           1.0,         // par6_loadFlag,
                                                           1.0,         // par7_sensLatentFlag,
                                                           par8_HXFlag, // par9_HXOnFlag,
                                                           0.0);        // par10_HeatingCoilPLR);
                            };
                            General::SolveRoot(state, CoolErrorToler, MaxIter, SolFlag, PartLoadRatio, f, 0.0, 1.0);
                            //             OnOffAirFlowRatio is updated during the above iteration. Reset to correct value based on PLR.
                            OnOffAirFlowRatio = state.dataFurnaces->OnOffAirFlowRatioSave;
                            if (SolFlag < 0) {
                                if (SolFlag == -1) {
                                    CalcFurnaceOutput(state,
                                                      FurnaceNum,
                                                      FirstHVACIteration,
                                                      OpMode,
                                                      CompressorOp,
                                                      PartLoadRatio,
                                                      0.0,
                                                      0.0,
                                                      0.0,
                                                      TempCoolOutput,
                                                      TempLatentOutput,
                                                      OnOffAirFlowRatio,
                                                      HXUnitOn);
                                    if (!state.dataGlobal->WarmupFlag) {
                                        if (std::abs(CoolCoilLoad - TempCoolOutput) > SmallLoad) {
                                            if (state.dataFurnaces->Furnace(FurnaceNum).SensibleMaxIterIndex == 0) {
                                                ShowWarningMessage(state,
                                                                   format("Cooling coil control failed to converge for {}:{}",
                                                                          cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                                                          state.dataFurnaces->Furnace(FurnaceNum).Name));
                                                ShowContinueError(
                                                    state, "  Iteration limit exceeded in calculating DX cooling coil sensible part-load ratio.");
                                                ShowContinueErrorTimeStamp(state,
                                                                           format("Sensible load to be met by DX coil = {:.2T} (watts), sensible "
                                                                                  "output of DX coil = {:.2T} (watts), and the simulation continues.",
                                                                                  CoolCoilLoad,
                                                                                  TempCoolOutput));
                                            }
                                            ShowRecurringWarningErrorAtEnd(state,
                                                                           cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num) +
                                                                               " \"" + state.dataFurnaces->Furnace(FurnaceNum).Name +
                                                                               "\" - Iteration limit exceeded in calculating sensible cooling "
                                                                               "part-load ratio error continues. Sensible load statistics:",
                                                                           state.dataFurnaces->Furnace(FurnaceNum).SensibleMaxIterIndex,
                                                                           CoolCoilLoad,
                                                                           CoolCoilLoad);
                                        }
                                    }
                                } else if (SolFlag == -2) {
                                    if (!state.dataGlobal->WarmupFlag) {
                                        if (state.dataFurnaces->Furnace(FurnaceNum).SensibleRegulaFalsiFailedIndex == 0) {
                                            ShowWarningMessage(state,
                                                               format("Cooling coil control failed for {}:{}",
                                                                      cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                                                      state.dataFurnaces->Furnace(FurnaceNum).Name));
                                            ShowContinueError(state, "  Cooling sensible part-load ratio determined to be outside the range of 0-1.");
                                            ShowContinueErrorTimeStamp(state, format("  Cooling sensible load = {:.2T}", CoolCoilLoad));
                                        }
                                        ShowRecurringWarningErrorAtEnd(
                                            state,
                                            cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num) + " \"" +
                                                state.dataFurnaces->Furnace(FurnaceNum).Name +
                                                "\" - Cooling sensible part-load ratio out of range error continues. Sensible cooling load "
                                                "statistics:",
                                            state.dataFurnaces->Furnace(FurnaceNum).SensibleRegulaFalsiFailedIndex,
                                            CoolCoilLoad,
                                            CoolCoilLoad);
                                    }
                                }
                            }
                        }

                    } else {
                        PartLoadRatio = 0.0;
                    } // EndIf for IF(CoolCoilLoad.NE.0.0)

                    //       Calculate the delivered capacity from the PLR caculated above
                    CalcFurnaceOutput(state,
                                      FurnaceNum,
                                      FirstHVACIteration,
                                      OpMode,
                                      CompressorOp,
                                      PartLoadRatio,
                                      state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio,
                                      0.0,
                                      0.0,
                                      TempCoolOutput,
                                      TempLatentOutput,
                                      OnOffAirFlowRatio,
                                      HXUnitOn);

                    //       Calculate the latent part load ratio through iteration
                    //       Negative SystemMoistureLoad means dehumidification load is present
                    //       IF this furnace uses MultiMode control AND there is a moisture load AND the moisture load met by the furnace in
                    //       cooling only mode above is sufficient to meet the moisture demand OR there is no sensible load (PLR=0 from above)
                    //       then set LatentPartLoadRatio to 0 (no additional dehumidification is required).
                    if (state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::Multimode &&
                        ((SystemMoistureLoad < 0.0 && TempLatentOutput < SystemMoistureLoad) || PartLoadRatio == 0.0)) {
                        LatentPartLoadRatio = 0.0;
                        //       ELSE calculate a new PLR for valid dehumidification control types if a moisture load exists.
                    } else if (state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num != DehumidificationControlMode::None &&
                               (SystemMoistureLoad < 0.0 ||
                                (SystemMoistureLoad >= 0.0 && TempLatentOutput > SystemMoistureLoad &&
                                 !state.dataZoneEnergyDemand->Setback(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)))) {

                        //         IF the furnace uses dehumidification control MultiMode, turn on the HX and calculate the latent output with
                        //         the HX ON to compare to the moisture load predicted by the humidistat.
                        if (state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::Multimode) {
                            HXUnitOn = true;
                            state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace;
                            // Set fan part-load fraction equal to 1 while getting full load result
                            state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
                            OnOffAirFlowRatio = 1.0;
                            // Get full load result
                            CalcFurnaceOutput(state,
                                              FurnaceNum,
                                              FirstHVACIteration,
                                              OpMode,
                                              CompressorOp,
                                              1.0,
                                              0.0,
                                              0.0,
                                              0.0,
                                              TempCoolOutput,
                                              TempLatentOutput,
                                              OnOffAirFlowRatio,
                                              HXUnitOn);
                        }

                        //         Set the global cooling to heating PLR ratio. CoolHeatPLRRat = MIN(1,CoolingPLR/HeatingPLR)
                        state.dataFurnaces->CoolHeatPLRRat = 1.0; // means cooling dominated operation (applies to cycling fan mode)

                        if (TempLatentOutput > SystemMoistureLoad) {
                            //           Set full mass flow rate for full load calculation
                            state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace;

                            // Set fan part-load fraction equal to 1 while getting full load result
                            state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
                            OnOffAirFlowRatio = 1.0;
                            state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 1.0; // compressor ON
                            state.dataFurnaces->Furnace(FurnaceNum).WSHPRuntimeFrac = 1.0;

                            // Get full load result (coils simulated full ON)
                            CalcFurnaceOutput(state,
                                              FurnaceNum,
                                              FirstHVACIteration,
                                              OpMode,
                                              CompressorOp,
                                              1.0,
                                              0.0,
                                              0.0,
                                              0.0,
                                              TempCoolOutput,
                                              TempLatentOutput,
                                              OnOffAirFlowRatio,
                                              HXUnitOn);
                        }

                        //         check bounds on latent output prior to iteration using RegulaFalsi
                        if (TempLatentOutput > SystemMoistureLoad ||
                            (state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::Multimode &&
                             TempCoolOutput > CoolCoilLoad)) {
                            LatentPartLoadRatio = 1.0;
                        } else if (NoLatentOutput < SystemMoistureLoad || HeatingLatentOutput < SystemMoistureLoad) {
                            LatentPartLoadRatio = 0.0;
                        } else {

                            CoolErrorToler = state.dataFurnaces->Furnace(FurnaceNum).CoolingConvergenceTolerance; // Error tolerance for convergence

                            SolFlag = 0; // # of iterations if positive, -1 means failed to converge, -2 means bounds are incorrect
                            Par[0] = double(FurnaceNum);
                            Par[1] = 0.0; // FLAG, if 1.0 then FirstHVACIteration equals TRUE, if 0.0 then FirstHVACIteration equals false
                            if (FirstHVACIteration) Par[1] = 1.0;
                            Par[2] = double(OpMode);
                            Par[3] = double(CompressorOp);
                            //           Multimode always controls to meet the SENSIBLE load (however, HXUnitOn is now TRUE)
                            Real64 par4_load;
                            if (state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::Multimode) {
                                Par[4] = CoolCoilLoad;
                                par4_load = CoolCoilLoad;
                            } else {
                                Par[4] = SystemMoistureLoad;
                                par4_load = SystemMoistureLoad;
                            }
                            Par[5] = 1.0; // FLAG, 0.0 if heating load, 1.0 if cooling or moisture load
                            //           Multimode always controls to meet the SENSIBLE load (however, HXUnitOn is now TRUE)
                            Real64 par6_LatentSens;
                            if (state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::Multimode) {
                                par6_LatentSens = 1.0;
                                Par[6] = 1.0; // FLAG, 0.0 if latent load, 1.0 if sensible load to be met
                            } else {
                                par6_LatentSens = 0.0;
                                Par[6] = 0.0;
                            }
                            Par[7] = OnOffAirFlowRatio; // Ratio of compressor ON mass flow rate to AVERAGE mass flow rate over time step
                            Real64 par8_HXUnit = HXUnitOn ? 1.0 : 0.0;
                            if (HXUnitOn) {
                                Par[8] = 1.0;
                            } else {
                                Par[8] = 0.0;
                            }
                            //           Par(10) used only with cycling fan.
                            //           Par(10) is the heating coil PLR, set this value only if there is a heating load (heating PLR > 0)
                            //           and the latent PLR is being calculated. Otherwise set Par(10) to 0.
                            Real64 par9_HtgCoilPLR;
                            if (OpMode == CycFanCycCoil && state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio > 0.0 && Par[6] == 0.0) {
                                par9_HtgCoilPLR = state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio;
                                Par[9] = state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio;
                            } else {
                                par9_HtgCoilPLR = 0.0;
                                Par[9] = 0.0;
                            }
                            auto f = [&state,
                                      FurnaceNum,
                                      FirstHVACIteration,
                                      OpMode,
                                      CompressorOp,
                                      par4_load,
                                      par6_LatentSens,
                                      par8_HXUnit,
                                      par9_HtgCoilPLR](Real64 const PartLoadRatio) {
                                return CalcFurnaceResidual(state,
                                                           PartLoadRatio,
                                                           FurnaceNum,
                                                           FirstHVACIteration,
                                                           OpMode,
                                                           CompressorOp,
                                                           par4_load,
                                                           1.0,              // par6_loadFlag,
                                                           par6_LatentSens,  // par7_sensLatentFlag,
                                                           par8_HXUnit,      // par9_HXOnFlag,
                                                           par9_HtgCoilPLR); // par10_HeatingCoilPLR);
                            };
                            //           CoolErrorToler is in fraction of load, MaxIter = 30, SolFalg = # of iterations or error as appropriate
                            General::SolveRoot(state, CoolErrorToler, MaxIter, SolFlag, LatentPartLoadRatio, f, 0.0, 1.0);
                            //           OnOffAirFlowRatio is updated during the above iteration. Reset to correct value based on PLR.
                            OnOffAirFlowRatio = state.dataFurnaces->OnOffAirFlowRatioSave;
                            if (SolFlag == -1) {
                                //             RegulaFalsi may not find latent PLR when the latent degradation model is used.
                                //             If iteration limit is exceeded, find tighter boundary of solution and repeat RegulaFalsi
                                TempMaxPLR = -0.1;
                                TempLatentOutput = NoLatentOutput;
                                while ((TempLatentOutput - SystemMoistureLoad) > 0.0 && TempMaxPLR < 1.0) {
                                    //               find upper limit of LatentPLR
                                    TempMaxPLR += 0.1;

                                    //               Same calculation as is done in Function CalcFurnaceResidual for latent PLR calculation.
                                    //               Set cooling to heating PLR for use with Subroutine CalcFurnaceOutput. IF Par(10) = 0,
                                    //               heating PLR = 0 so set the CoolingHeatingPLRRatio to 1 so the cooling PLR is used in the
                                    //               DX cooling coil calculations.
                                    if (Par[9] > 0.0) {
                                        //                 Par(10) = Furnace(FurnaceNum)%HeatPartLoadRatio
                                        //                 OpMode = CycFan and Furnace(FurnaceNum)%HeatPartLoadRatio must be > 0 for Part(10) to be
                                        //                 greater than 0
                                        CoolingHeatingPLRRatio = min(1.0, TempMaxPLR / state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio);
                                    } else {
                                        CoolingHeatingPLRRatio = 1.0;
                                    }

                                    CalcFurnaceOutput(state,
                                                      FurnaceNum,
                                                      FirstHVACIteration,
                                                      OpMode,
                                                      CompressorOp,
                                                      TempMaxPLR,
                                                      0.0,
                                                      0.0,
                                                      0.0,
                                                      TempCoolOutput,
                                                      TempLatentOutput,
                                                      OnOffAirFlowRatio,
                                                      HXUnitOn,
                                                      CoolingHeatingPLRRatio);
                                }
                                TempMinPLR = TempMaxPLR;
                                while ((TempLatentOutput - SystemMoistureLoad) < 0.0 && TempMinPLR > 0.0) {
                                    //               pull upper limit of LatentPLR down to last valid limit (i.e. latent output still exceeds
                                    //               SystemMoisuterLoad) CR7558 - relax final limits to allow HX assisted coils to converge
                                    TempMaxPLR = TempMinPLR + 0.001;
                                    //               find minimum limit of Latent PLR
                                    TempMinPLR -= 0.001;

                                    //               Set cooling to heating PLR for use with Subroutine CalcFurnaceOutput.
                                    if (Par[9] > 0.0) {
                                        //                 Par(10) = Furnace(FurnaceNum)%HeatPartLoadRatio
                                        //                 OpMode = CycFan and Furnace(FurnaceNum)%HeatPartLoadRatio must be > 0 for Part(10) to be
                                        //                 greater than 0 Since the latent output of cycling fan systems is 0 at PLR=0, do not allow
                                        //                 the PLR to be 0, otherwise RegulaFalsi can fail when a heating and moisture load exists and
                                        //                 heating PLR > latent PLR.
                                        TempMinPLR2 = max(0.0000000001, TempMinPLR);
                                        CoolingHeatingPLRRatio = min(1.0, TempMinPLR2 / state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio);
                                    } else {
                                        TempMinPLR2 = TempMinPLR;
                                        CoolingHeatingPLRRatio = 1.0;
                                    }

                                    CalcFurnaceOutput(state,
                                                      FurnaceNum,
                                                      FirstHVACIteration,
                                                      OpMode,
                                                      CompressorOp,
                                                      TempMinPLR2,
                                                      0.0,
                                                      0.0,
                                                      0.0,
                                                      TempCoolOutput,
                                                      TempLatentOutput,
                                                      OnOffAirFlowRatio,
                                                      HXUnitOn,
                                                      CoolingHeatingPLRRatio);
                                }
                                //             tighter boundary of solution has been found, call RegulaFalsi a second time
                                auto f = [&state,
                                          FurnaceNum,
                                          FirstHVACIteration,
                                          OpMode,
                                          CompressorOp,
                                          par4_load,
                                          par6_LatentSens,
                                          par8_HXUnit,
                                          par9_HtgCoilPLR](Real64 const PartLoadRatio) {
                                    return CalcFurnaceResidual(state,
                                                               PartLoadRatio,
                                                               FurnaceNum,
                                                               FirstHVACIteration,
                                                               OpMode,
                                                               CompressorOp,
                                                               par4_load,
                                                               1.0,              // par6_loadFlag,
                                                               par6_LatentSens,  // par7_sensLatentFlag,
                                                               par8_HXUnit,      // par9_HXOnFlag,
                                                               par9_HtgCoilPLR); // par10_HeatingCoilPLR);
                                };
                                General::SolveRoot(state, CoolErrorToler, MaxIter, SolFlag, LatentPartLoadRatio, f, TempMinPLR2, TempMaxPLR);
                                //             OnOffAirFlowRatio is updated during the above iteration. Reset to correct value based on PLR.
                                OnOffAirFlowRatio = state.dataFurnaces->OnOffAirFlowRatioSave;
                                if (SolFlag == -1) {

                                    //               Set cooling to heating PLR for use with Subroutine CalcFurnaceOutput.
                                    if (Par[9] > 0.0) {
                                        //                 Par(10) = Furnace(FurnaceNum)%HeatPartLoadRatio
                                        //                 OpMode = CycFan and Furnace(FurnaceNum)%HeatPartLoadRatio must be > 0 for Part(10) to be
                                        //                 greater than 0
                                        CoolingHeatingPLRRatio =
                                            min(1.0, LatentPartLoadRatio / state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio);
                                    } else {
                                        CoolingHeatingPLRRatio = 1.0;
                                    }

                                    CalcFurnaceOutput(state,
                                                      FurnaceNum,
                                                      FirstHVACIteration,
                                                      OpMode,
                                                      CompressorOp,
                                                      LatentPartLoadRatio,
                                                      0.0,
                                                      0.0,
                                                      0.0,
                                                      TempCoolOutput,
                                                      TempLatentOutput,
                                                      OnOffAirFlowRatio,
                                                      HXUnitOn,
                                                      CoolingHeatingPLRRatio);
                                    if (std::abs((SystemMoistureLoad - TempLatentOutput) / SystemMoistureLoad) > CoolErrorToler &&
                                        std::abs(SystemMoistureLoad - TempLatentOutput) > 10.0) {
                                        if (!state.dataGlobal->WarmupFlag) {
                                            if (state.dataFurnaces->Furnace(FurnaceNum).LatentMaxIterIndex == 0) {
                                                ShowWarningMessage(state,
                                                                   format("Cooling coil control failed to converge for {}:{}",
                                                                          cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                                                          state.dataFurnaces->Furnace(FurnaceNum).Name));
                                                ShowContinueError(state,
                                                                  "  Iteration limit exceeded in calculating cooling coil latent part-load ratio.");
                                                ShowContinueError(
                                                    state,
                                                    format("  Latent load convergence error (percent) = {:.2T}",
                                                           100.0 * std::abs((SystemMoistureLoad - TempLatentOutput) / SystemMoistureLoad)));
                                                ShowContinueErrorTimeStamp(state,
                                                                           format("Moisture load to be met by DX coil = {:.2T} (watts), Latent "
                                                                                  "output of DX coil = {:.2T} (watts), and the simulation continues.",
                                                                                  SystemMoistureLoad,
                                                                                  TempLatentOutput));
                                            }
                                            ShowRecurringWarningErrorAtEnd(
                                                state,
                                                cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num) + " \"" +
                                                    state.dataFurnaces->Furnace(FurnaceNum).Name +
                                                    "\" - Iteration limit exceeded in calculating latent part-load ratio error continues. Latent "
                                                    "load convergence error (percent) statistics follow.",
                                                state.dataFurnaces->Furnace(FurnaceNum).LatentMaxIterIndex,
                                                100.0 * std::abs((SystemMoistureLoad - TempLatentOutput) / SystemMoistureLoad),
                                                100.0 * std::abs((SystemMoistureLoad - TempLatentOutput) / SystemMoistureLoad));
                                        }
                                    }
                                } else if (SolFlag == -2) {
                                    if (state.dataFurnaces->Furnace(FurnaceNum).LatentRegulaFalsiFailedIndex2 == 0) {
                                        ShowWarningMessage(state,
                                                           format("Cooling coil control failed for {}:{}",
                                                                  cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                                                  state.dataFurnaces->Furnace(FurnaceNum).Name));
                                        ShowContinueError(state,
                                                          format("  Latent part-load ratio determined to be outside the range of {:.3T} to {:.3T}.",
                                                                 TempMinPLR,
                                                                 TempMaxPLR));
                                        ShowContinueErrorTimeStamp(state,
                                                                   format("A PLR of {:.3T} will be used and the simulation continues.", TempMinPLR));
                                    }
                                    ShowRecurringWarningErrorAtEnd(
                                        state,
                                        cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num) + " \"" +
                                            state.dataFurnaces->Furnace(FurnaceNum).Name +
                                            "\" - Cooling sensible part-load ratio out of range error continues. System moisture load statistics:",
                                        state.dataFurnaces->Furnace(FurnaceNum).LatentRegulaFalsiFailedIndex2,
                                        SystemMoistureLoad,
                                        SystemMoistureLoad);
                                    LatentPartLoadRatio = TempMinPLR;
                                }
                            } else if (SolFlag == -2) {
                                if (state.dataFurnaces->Furnace(FurnaceNum).LatentRegulaFalsiFailedIndex == 0) {
                                    ShowWarningMessage(state,
                                                       format("Cooling coil control failed for {}:{}",
                                                              cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                                              state.dataFurnaces->Furnace(FurnaceNum).Name));
                                    ShowContinueError(state, "  Latent part-load ratio determined to be outside the range of 0-1.");
                                    ShowContinueErrorTimeStamp(state, "A PLR of 0 will be used and the simulation continues.");
                                }
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num) + " \"" +
                                        state.dataFurnaces->Furnace(FurnaceNum).Name +
                                        "\" - Latent part-load ratio out of range or 0-1 error continues. System moisture load statistics:",
                                    state.dataFurnaces->Furnace(FurnaceNum).LatentRegulaFalsiFailedIndex,
                                    SystemMoistureLoad,
                                    SystemMoistureLoad);
                                LatentPartLoadRatio = 0.0;
                            }
                        }

                        //         Cooling to heating PLR ratio is now known as CoolHeatPLRRat (Module level global set in CalcFurnaceOutput
                        //         This same variable is use in Subroutine SimFurnace for final calculations.
                        //         Get the actual output in case reheat needs to be calculated (HumControl=TRUE [latent PLR > sensible PLR])
                        CalcFurnaceOutput(state,
                                          FurnaceNum,
                                          FirstHVACIteration,
                                          OpMode,
                                          CompressorOp,
                                          LatentPartLoadRatio,
                                          0.0,
                                          0.0,
                                          0.0,
                                          ActualSensibleOutput,
                                          ActualLatentOutput,
                                          OnOffAirFlowRatio,
                                          HXUnitOn,
                                          state.dataFurnaces->CoolHeatPLRRat);

                    } else {
                        LatentPartLoadRatio = 0.0;
                    } // ENDIF for valid dehumidification control types

                    //       IF a humidistat is used and there is a moisture load, check if the latent PLR is greater than the (sensible) PLR
                    //        IF(LatentPartLoadRatio .GT. PartLoadRatio .and. SystemMoistureLoad .lt. 0.0 .and. Furnace(FurnaceNum)%Humidistat) THEN
                    if (LatentPartLoadRatio > PartLoadRatio && state.dataFurnaces->Furnace(FurnaceNum).Humidistat) {
                        //         For dehumidification mode CoolReheat, compare the Sensible and Latent PLR values, if latentPLR is greater
                        //         than PLR (sensible), then overcooling is required and reheat will be activated using the HumControl flag.
                        if (state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::CoolReheat) {
                            PartLoadRatio = LatentPartLoadRatio;
                            HumControl = true;
                        }
                        //         For dehumidification mode MultiMode, compare the Sensible and Latent PLR values, if latentPLR is
                        //         greater than PLR (sensible), then use the latent PLR to control the unit.
                        //         For MultiMode control, the latent PLR is found by enabling the HX and calculating a PLR required to meet the
                        //         sensible load. Overcooling is not required, and reheat will not be activated using the HumControl flag.
                        if (state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::Multimode) {
                            PartLoadRatio = LatentPartLoadRatio;
                        }
                    }

                    state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio = PartLoadRatio;
                    if (CompressorOp == CompressorOperation::Off) {
                        state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 0.0;
                    } else {
                        state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = PartLoadRatio;
                    }

                } else { // ELSE from IF(FullSensibleOutput.LT.NoCoolOutput)THEN above
                    // CR8679 - Unitary Heat Cool control problem, will not run to meeting cooling load
                    // underlying problem is that FullSensibleOutput is greater than 0 due to very high inlet temp, so the system should be on
                    // NoCoolOutput was 0 since the defect file is a cycling fan system and the system was turned off

                    // if FullSensibleOutput > NoCoolOutput, it means the system cannot meet the load and will run full out
                    // this same logic for WSHP does not seem to work (only the Unitary Heat Pump Compressor Part-Load Ratio report
                    // variable was affected in the HeatPumpWaterToAirRHControl.idf file while other variables showed very small diffs).
                    // The defect files meter.csv showed 2% diffs so this IF test is used to keep the results the same in that file.
                    // Additional logic is used here to make sure the coil actually turned on, e.g., if DX coil PLR > 0 then set to 1,
                    // otherwise 0 (to make sure coil is actually ON and not off due to schedule, OAT, or other reason).
                    // The global variable DXCoilPartLoadRatio(DXCoilNum) is not yet used for the WSHP to make the same check.
                    if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_WaterToAir) {
                        state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio = 0.0;
                        state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 0.0;
                    } else {
                        if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num == CoilDX_CoolingHXAssisted) {

                            // VS coil issue here...
                            if (state.dataDXCoils->DXCoilPartLoadRatio(state.dataFurnaces->Furnace(FurnaceNum).ActualDXCoilIndexForHXAssisted) >
                                0.0) {
                                state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio = 1.0;
                                state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 1.0;
                            } else {
                                state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio = 0.0;
                                state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 0.0;
                            }
                        } else {
                            if (state.dataDXCoils->DXCoilPartLoadRatio(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex) > 0.0) {
                                state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio = 1.0;
                                state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 1.0;
                            } else {
                                state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio = 0.0;
                                state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 0.0;
                            }
                        }
                    }
                }

                //     Calculate the reheat coil output
                if (HumControl) { // HumControl = .TRUE. if a Humidistat is installed and dehumdification control type is CoolReheat
                    if (state.dataFurnaces->Furnace(FurnaceNum).ZoneSequenceHeatingNum > 0) {
                        QToHeatSetPt = (state.dataZoneEnergyDemand->ZoneSysEnergyDemand(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                                            .SequencedOutputRequiredToHeatingSP(state.dataFurnaces->Furnace(FurnaceNum).ZoneSequenceHeatingNum) /
                                        state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac);
                    } else {
                        QToHeatSetPt = (state.dataZoneEnergyDemand->ZoneSysEnergyDemand(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                                            .OutputRequiredToHeatingSP /
                                        state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac);
                    }
                    //       Cooling mode or floating condition and dehumidification is required
                    if (QToHeatSetPt < 0.0) {
                        //         Calculate the reheat coil load wrt the heating setpoint temperature. Reheat coil picks up
                        //         the entire excess sensible cooling (DX cooling coil and impact of outdoor air).
                        ReheatCoilLoad = max(0.0, (QToHeatSetPt - ActualSensibleOutput));
                        state.dataFurnaces->Furnace(FurnaceNum).DehumidInducedHeatingDemandRate = ReheatCoilLoad;
                        //       Heating mode and dehumidification is required
                    } else if (QToHeatSetPt >= 0.0) {
                        //         Calculate the reheat coil load as the sensible capacity of the DX cooling coil only. Let
                        //         the heating coil pick up the load due to outdoor air.
                        ReheatCoilLoad = max(0.0, (ActualSensibleOutput - NoCoolOutput) * (-1.0));
                        //         Dehumidification is not required
                        if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_AirToAir ||
                            (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_WaterToAir &&
                             state.dataFurnaces->Furnace(FurnaceNum).WatertoAirHPType == WatertoAir_Simple)) {
                            ReheatCoilLoad = max(QToHeatSetPt, QToHeatSetPt - ActualSensibleOutput);
                        }
                        state.dataFurnaces->Furnace(FurnaceNum).DehumidInducedHeatingDemandRate = max(0.0, ActualSensibleOutput * (-1.0));
                    } else {
                        ReheatCoilLoad = 0.0;
                    }
                } else {
                    //       No humidistat installed
                    ReheatCoilLoad = 0.0;
                }
            } // End of cooling section IF statement

            if (NoHeatOutput > SystemSensibleLoad && ReheatCoilLoad > 0.0) {
                // Reduce reheat coil load if you are controlling high humidity but outside air
                // and/or the supply air fan is providing enough heat to meet the system sensible load.
                // This will bring the zone temp closer to the heating setpoint temp.
                ReheatCoilLoad = max(0.0, ReheatCoilLoad - (NoHeatOutput - SystemSensibleLoad));
            }

            // Set the final air flow. MdotFurnace will be used to set the fan part-load ratio in ReportFurnace
            if (HumControl && SystemMoistureLoad < 0.0) {
                if (OpMode == CycFanCycCoil) {
                    //       set the flow rate at the maximum of the cooling and heating PLR's
                    SetAverageAirFlow(
                        state,
                        FurnaceNum,
                        max(state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio, state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio),
                        OnOffAirFlowRatio);
                } else {
                    //       ELSE set the flow rate at the cooling PLR
                    SetAverageAirFlow(state, FurnaceNum, state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio, OnOffAirFlowRatio);
                }
            } else {
                SetAverageAirFlow(
                    state,
                    FurnaceNum,
                    max(state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio, state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio),
                    OnOffAirFlowRatio);
            }
            state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace = state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate;

            if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_AirToAir ||
                (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_WaterToAir &&
                 state.dataFurnaces->Furnace(FurnaceNum).WatertoAirHPType == WatertoAir_Simple)) {
            } else {
                // Non-HeatPump (non-DX) heating coils do not set PLR, reset to 0 here. This variable was set for non-DX
                // coils to allow the SetAverageAirFlow CALL above to set the correct air mass flow rate. See this
                // IF block above in heating section. HeatPLR is not set in the ELSE part of the IF (only HeatCoilLoad is set).
                state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio = 0.0;
            }

            //*********HVAC Scheduled OFF*************
            // No heating or cooling or dehumidification
            //!!LKL discrepancy with < 0?
            if (GetCurrentScheduleValue(state, state.dataFurnaces->Furnace(FurnaceNum).SchedPtr) == 0.0 ||
                state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate == 0.0) {
                state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace = 0.0;
                CoolCoilLoad = 0.0;
                HeatCoilLoad = 0.0;
                ReheatCoilLoad = 0.0;
                state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0; // System off, so set on/off fan part-load fraction = 1
                state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio = 0.0;
                state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio = 0.0;
                state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 0.0;
                // set report variables
                state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand = 0.0;
                state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilLatentDemand = 0.0;
                state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand = 0.0;
            }

        } // End of the FirstHVACIteration control of the mass flow If block

        // Set the fan inlet node flow rates
        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRateMaxAvail = state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace;
        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace;
    }

    void CalcWaterToAirHeatPump(EnergyPlusData &state,
                                int const AirLoopNum,                   // index to air loop
                                int const FurnaceNum,                   // index to Furnace
                                bool const FirstHVACIteration,          // TRUE on first HVAC iteration
                                CompressorOperation const CompressorOp, // compressor operation flag (1=On, 0=Off)
                                Real64 const ZoneLoad,                  // the control zone load (watts)
                                Real64 const MoistureLoad               // the control zone latent load (watts)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   Feb 2004
        //       MODIFIED       R. Raustad (Oct 2006) Revised iteration technique
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine manages the heat pump simulation

        // METHODOLOGY EMPLOYED:
        // Calculate the part-load ratio required to meet the zone sensible load.

        // SUBROUTINE PARAMETER DEFINITIONS:
        int constexpr MaxIter(600);   // maximum number of iterations
        Real64 constexpr MinPLR(0.0); // minimum part load ratio allowed

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 OnOffAirFlowRatio;           // Ratio of compressor ON air mass flow to AVERAGE air mass flow over time step
        Real64 ZoneSensLoadMet;             // Actual zone sensible load met by heat pump (W)
        Real64 ZoneLatLoadMet;              // Actual zone latent load met by heat pump (W)
        Real64 ZoneSensLoadMetFanONCompON;  // Max Zone sensible load heat pump can meet (W)
        Real64 ZoneLatLoadMetFanONCompON;   // Max Zone latentload heat pump can meet (W)
        Real64 ZoneSensLoadMetFanONCompOFF; // control zone sensible load met using only outside air
        // and fan heat (no coil output) (W)
        Real64 ZoneLatLoadMetFanONCompOFF; // control zone Latent   load met using only outside air
        // and fan heat (no coil output) (W)
        Real64 HPCoilSensDemand;   // Heat pump sensible demand
        Real64 HPCoilSensCapacity; // Heat pump sensible capacity
        int FurnaceInletNode;      // heat pump Inlet node
        int FurnaceOutletNode;     // heat pump Outlet node

        int OASysInletNode;        // node number of return air inlet to OA sys
        int OASysOutletNode;       // node number of mixed air outlet of OA sys
        int OpMode;                // Mode of Operation (fan cycling = 1 or fan continuous = 2)
        bool HumControl;           // Logical flag signaling when dehumidification is required
        Real64 SuppHeatCoilLoad;   // Load passed to supplemental heater (W)
        Real64 CoolErrorToler;     // convergence tolerance used in cooling mode
        Real64 HeatErrorToler;     // convergence tolerance used in heating mode
        int SolFlag;               // flag returned from iteration routine to denote problems
        std::array<Real64, 9> Par; // parameters passed to iteration routine

        auto &TotalZoneLatentLoad = state.dataFurnaces->TotalZoneLatentLoad;
        auto &TotalZoneSensLoad = state.dataFurnaces->TotalZoneSensLoad;
        auto &CoolPartLoadRatio = state.dataFurnaces->CoolPartLoadRatio;
        auto &HeatPartLoadRatio = state.dataFurnaces->HeatPartLoadRatio;
        auto &Dummy2 = state.dataFurnaces->Dummy2;

        // Set local variables
        Dummy2 = 0.0;
        OnOffAirFlowRatio = 1.0;
        FurnaceOutletNode = state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum;
        FurnaceInletNode = state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum;
        if (state.dataAirLoop->AirToOANodeInfo(AirLoopNum).OASysExists) {
            OASysOutletNode = state.dataAirLoop->AirToOANodeInfo(AirLoopNum).OASysOutletNodeNum;
            OASysInletNode = state.dataAirLoop->AirToOANodeInfo(AirLoopNum).OASysInletNodeNum;
        }
        OpMode = state.dataFurnaces->Furnace(FurnaceNum).OpMode;
        state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace = state.dataFurnaces->Furnace(FurnaceNum).DesignMassFlowRate;
        HumControl = false;

        //*********INITIAL CALCULATIONS****************
        // set the fan part load fraction
        // Note: OnOffFanPartLoadFraction is passed to the
        //       fan module by DataHVACGlobals.  It should be
        //     set =1 for all cases except cycling fan/cycling
        //     coil. For this case it is set to the part load
        //     factor.  In SimOnOffFan, the part load ratio is
        //     divided by the part load factor (OnOffFanPartLoadFraction)
        //     in order to match the run time fraction of the cycling
        //     fan with the run time fraction of the cycling compressor
        if (FirstHVACIteration) state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;

        // Calc Zone sensible loads for heating (+) and cooling (-)
        TotalZoneSensLoad = ZoneLoad;

        // Set latent load for heating
        if (state.dataFurnaces->HeatingLoad) {
            TotalZoneLatentLoad = 0.0;

            // Set latent load for cooling and no sensible load condition
        } else {
            TotalZoneLatentLoad = MoistureLoad;
        }

        //*********COOLING CALCULATIONS****************
        // IF scheduled on...
        // AND air flow rate is greater than zero...
        // AND the air system has a cooling load and is not set back or in the deadband...
        // OR the system is controlled by a humidistat and there is a latent load
        if ((GetCurrentScheduleValue(state, state.dataFurnaces->Furnace(FurnaceNum).SchedPtr) > 0.0 &&
             state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate > 0.0) &&
            ((state.dataFurnaces->CoolingLoad) ||
             (state.dataFurnaces->Furnace(FurnaceNum).Humidistat && state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilLatentDemand < 0.0))) {

            // Set the air flow rate to the design flow rate and set the fan operation fraction to 1 (continuous operation)
            state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = state.dataFurnaces->Furnace(FurnaceNum).DesignMassFlowRate;
            state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0; // see 'Note' under INITIAL CALCULATIONS

            //         !Set the operation flag to run the fan continuously
            //         OpMode = ContFanCycCoil

            // Set the input parameters for CalcFurnaceOutput
            state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand = 0.0;
            state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilLatentDemand = 0.0;
            state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand = 0.0;
            state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 0.0; // compressor off
            state.dataFurnaces->Furnace(FurnaceNum).InitHeatPump = true;     // initialization call to Calc Furnace
            state.dataFurnaces->Furnace(FurnaceNum).WSHPRuntimeFrac = 0.0;
            CoolPartLoadRatio = 0.0;

            // Get no load result in order to calculate the effect of the fan and the mixed air equipment
            CalcFurnaceOutput(state,
                              FurnaceNum,
                              FirstHVACIteration,
                              OpMode,
                              CompressorOp,
                              CoolPartLoadRatio,
                              HeatPartLoadRatio,
                              Dummy2,
                              Dummy2,
                              ZoneSensLoadMetFanONCompOFF,
                              ZoneLatLoadMetFanONCompOFF,
                              OnOffAirFlowRatio,
                              false);

            // Set the input parameters for CalcFurnaceOutput
            state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand = 1.0;
            state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 1.0; // compressor ON
            state.dataFurnaces->Furnace(FurnaceNum).WSHPRuntimeFrac = 1.0;
            CoolPartLoadRatio = 1.0;

            // Get full load result in order to estimate the operating part load ratio for continuous fan operation
            CalcFurnaceOutput(state,
                              FurnaceNum,
                              FirstHVACIteration,
                              OpMode,
                              CompressorOp,
                              CoolPartLoadRatio,
                              HeatPartLoadRatio,
                              Dummy2,
                              Dummy2,
                              ZoneSensLoadMetFanONCompON,
                              ZoneLatLoadMetFanONCompON,
                              OnOffAirFlowRatio,
                              false);

            // Calculate the heating coil demand for continuous fan operation as:
            //    (the zone sensible load - the zone sensible load met by fan heat and mixed air)
            // Note; The sensible zone load met by fan heat and mixed air is calculated as:
            //     mdotsys(control zone inlet enthalpy - control zone outlet enthalpy)
            // This accounts for the negative sign in the equation.
            HPCoilSensDemand = TotalZoneSensLoad - ZoneSensLoadMetFanONCompOFF;

            // Calculate the heating coil capacity for continuous fan operation as:
            //    (the zone sensible load met by fan heat and mixed air and coil
            //   - the zone sensible load met by fan heat and mixed air)
            HPCoilSensCapacity = ZoneSensLoadMetFanONCompON - ZoneSensLoadMetFanONCompOFF;

            // Calculate the part load ratio for continuous fan operation with cycling coil
            if (HPCoilSensCapacity == 0.0) {
                CoolPartLoadRatio = 0.0;
            } else {
                CoolPartLoadRatio = max(MinPLR, min(1.0, std::abs(HPCoilSensDemand) / std::abs(HPCoilSensCapacity)));
            }

            state.dataFurnaces->Furnace(FurnaceNum).InitHeatPump = false;

            //       check bounds on sensible output prior to iteration using RegulaFalsi
            if (ZoneSensLoadMetFanONCompON > TotalZoneSensLoad) {
                CoolPartLoadRatio = 1.0;
                HPCoilSensDemand = std::abs(ZoneSensLoadMetFanONCompON - ZoneSensLoadMetFanONCompOFF);
                state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand = HPCoilSensDemand;
            } else if (ZoneSensLoadMetFanONCompOFF < TotalZoneSensLoad) {
                CoolPartLoadRatio = 0.0;
                state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 0.0; // compressor OFF
                state.dataFurnaces->Furnace(FurnaceNum).WSHPRuntimeFrac = 0.0;
                state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand = 0.0;
                CalcFurnaceOutput(state,
                                  FurnaceNum,
                                  FirstHVACIteration,
                                  OpMode,
                                  CompressorOp,
                                  CoolPartLoadRatio,
                                  HeatPartLoadRatio,
                                  Dummy2,
                                  Dummy2,
                                  ZoneSensLoadMetFanONCompOFF,
                                  ZoneLatLoadMetFanONCompOFF,
                                  OnOffAirFlowRatio,
                                  false);
            } else {
                //         Calculate the sensible part load ratio through iteration
                CoolErrorToler = state.dataFurnaces->Furnace(FurnaceNum).CoolingConvergenceTolerance;
                SolFlag = 0; // # of iterations if positive, -1 means failed to converge, -2 means bounds are incorrect
                Par[0] = double(FurnaceNum);
                Par[1] = 0.0; // FLAG, if 1.0 then FirstHVACIteration equals TRUE, if 0.0 then FirstHVACIteration equals false
                if (FirstHVACIteration) Par[1] = 1.0;
                Par[2] = double(OpMode);
                Par[3] = double(CompressorOp);
                Par[4] = TotalZoneSensLoad;
                Par[5] = 1.0;                         // FLAG, 0.0 if heating load, 1.0 if cooling or moisture load
                Par[6] = 1.0;                         // FLAG, 0.0 if latent load, 1.0 if sensible load to be met
                Par[7] = ZoneSensLoadMetFanONCompOFF; // Output with fan ON compressor OFF
                Par[8] = 0.0;                         // HX is off for water-to-air HP
                //         CoolErrorToler is in fraction of load, MaxIter = 600, SolFalg = # of iterations or error as appropriate
                auto f = [&state, FurnaceNum, FirstHVACIteration, OpMode, CompressorOp, TotalZoneSensLoad, ZoneSensLoadMetFanONCompOFF](
                             Real64 const PartLoadRatio) {
                    return CalcWaterToAirResidual(state,
                                                  PartLoadRatio,
                                                  FurnaceNum,
                                                  FirstHVACIteration,
                                                  OpMode,
                                                  CompressorOp,
                                                  TotalZoneSensLoad,
                                                  1.0,
                                                  1.0,
                                                  ZoneSensLoadMetFanONCompOFF,
                                                  0.0);
                };
                General::SolveRoot(state, CoolErrorToler, MaxIter, SolFlag, CoolPartLoadRatio, f, 0.0, 1.0);
                if (SolFlag == -1 && !state.dataGlobal->WarmupFlag && !FirstHVACIteration) {
                    state.dataHVACGlobal->OnOffFanPartLoadFraction = state.dataFurnaces->OnOffFanPartLoadFractionSave;
                    CalcFurnaceOutput(state,
                                      FurnaceNum,
                                      FirstHVACIteration,
                                      OpMode,
                                      CompressorOp,
                                      CoolPartLoadRatio,
                                      0.0,
                                      0.0,
                                      0.0,
                                      ZoneSensLoadMet,
                                      ZoneLatLoadMet,
                                      OnOffAirFlowRatio,
                                      false);
                    if (std::abs(ZoneSensLoadMet - TotalZoneSensLoad) / TotalZoneSensLoad > CoolErrorToler) {
                        if (state.dataFurnaces->Furnace(FurnaceNum).SensibleMaxIterIndex == 0) {
                            ShowWarningMessage(state,
                                               format("Cooling coil control failed to converge for {}:{}",
                                                      cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                                      state.dataFurnaces->Furnace(FurnaceNum).Name));
                            ShowContinueError(state, "  Iteration limit exceeded in calculating DX cooling coil sensible part-load ratio.");
                            ShowContinueErrorTimeStamp(state,
                                                       format("Sensible load to be met by DX coil = {:.2T} (watts), sensible output of DX coil = "
                                                              "{:.2T} (watts), and the simulation continues.",
                                                              TotalZoneSensLoad,
                                                              ZoneSensLoadMet));
                        }
                        ShowRecurringWarningErrorAtEnd(state,
                                                       cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num) + " \"" +
                                                           state.dataFurnaces->Furnace(FurnaceNum).Name +
                                                           "\" - Iteration limit exceeded in calculating sensible cooling part-load ratio error "
                                                           "continues. Sensible load statistics:",
                                                       state.dataFurnaces->Furnace(FurnaceNum).SensibleMaxIterIndex,
                                                       TotalZoneSensLoad,
                                                       TotalZoneSensLoad);
                    }
                } else if (SolFlag == -2 && !state.dataGlobal->WarmupFlag && !FirstHVACIteration) {
                    CoolPartLoadRatio = max(MinPLR, min(1.0, std::abs(HPCoilSensDemand) / std::abs(HPCoilSensCapacity)));
                    state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
                    CalcFurnaceOutput(state,
                                      FurnaceNum,
                                      FirstHVACIteration,
                                      OpMode,
                                      CompressorOp,
                                      CoolPartLoadRatio,
                                      0.0,
                                      0.0,
                                      0.0,
                                      ZoneSensLoadMet,
                                      ZoneLatLoadMet,
                                      OnOffAirFlowRatio,
                                      false);
                    if ((ZoneSensLoadMet - TotalZoneSensLoad) / TotalZoneSensLoad > CoolErrorToler) {
                        if (state.dataFurnaces->Furnace(FurnaceNum).SensibleRegulaFalsiFailedIndex == 0) {
                            ShowWarningMessage(state,
                                               format("Cooling coil control failed for {}:{}",
                                                      cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                                      state.dataFurnaces->Furnace(FurnaceNum).Name));
                            ShowContinueError(state, "  Cooling sensible part-load ratio determined to be outside the range of 0-1.");
                            ShowContinueError(
                                state,
                                format("  An estimated part-load ratio = {:.2T} will be used and the simulation continues.", CoolPartLoadRatio));
                            ShowContinueError(
                                state, format("  The estimated part-load ratio provides a cooling sensible capacity = {:.2T}", ZoneSensLoadMet));
                            ShowContinueErrorTimeStamp(state, format("  Cooling sensible load required = {:.2T}", TotalZoneSensLoad));
                        }
                        ShowRecurringWarningErrorAtEnd(
                            state,
                            cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num) + " \"" +
                                state.dataFurnaces->Furnace(FurnaceNum).Name +
                                "\" - Cooling sensible part-load ratio out of range error continues. Sensible cooling load statistics:",
                            state.dataFurnaces->Furnace(FurnaceNum).SensibleRegulaFalsiFailedIndex,
                            TotalZoneSensLoad,
                            TotalZoneSensLoad);
                    }
                }
            }

            if (OpMode == CycFanCycCoil) {
                state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace *= CoolPartLoadRatio;
            }

            //*********HEATING CALCULATIONS****************
            // If Furnace runs with a heating load then set HeatCoilLoad on Heating Coil and the Mass Flow
        } else if ((GetCurrentScheduleValue(state, state.dataFurnaces->Furnace(FurnaceNum).SchedPtr) > 0.0) &&
                   (state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate > 0.0) && state.dataFurnaces->HeatingLoad) {

            // Set the air flow rate to the design flow rate and set the fan operation fraction to 1 (continuous operation)
            state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = state.dataFurnaces->Furnace(FurnaceNum).DesignMassFlowRate;
            state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0; // see 'Note' under INITIAL CALCULATIONS

            //         !Set the operation flag to run the fan continuously
            //         OpMode = ContFanCycCoil

            // Set the input parameters for CalcFurnaceOutput
            state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand = 0.0;
            state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilLatentDemand = 0.0;
            state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand = 0.0;
            state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 0.0; // compressor off
            state.dataFurnaces->Furnace(FurnaceNum).InitHeatPump = true;     // initialization call to Calc Furnace
            state.dataFurnaces->Furnace(FurnaceNum).WSHPRuntimeFrac = 0.0;
            HeatPartLoadRatio = 0.0;

            // Get no load result in order to calculate the effect of the fan and the mixed air equipment
            CalcFurnaceOutput(state,
                              FurnaceNum,
                              FirstHVACIteration,
                              OpMode,
                              CompressorOp,
                              CoolPartLoadRatio,
                              HeatPartLoadRatio,
                              Dummy2,
                              Dummy2,
                              ZoneSensLoadMetFanONCompOFF,
                              ZoneLatLoadMetFanONCompOFF,
                              OnOffAirFlowRatio,
                              false);

            // Set the input parameters for CalcFurnaceOutput
            state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand = 1.0;
            state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 1.0; // compressor ON
            state.dataFurnaces->Furnace(FurnaceNum).WSHPRuntimeFrac = 1.0;
            HeatPartLoadRatio = 1.0;

            // Get full load result in order to estimate the operating part load ratio for continuous fan operation

            CalcFurnaceOutput(state,
                              FurnaceNum,
                              FirstHVACIteration,
                              OpMode,
                              CompressorOp,
                              CoolPartLoadRatio,
                              HeatPartLoadRatio,
                              Dummy2,
                              Dummy2,
                              ZoneSensLoadMetFanONCompON,
                              ZoneLatLoadMetFanONCompON,
                              OnOffAirFlowRatio,
                              false);

            // Calculate the heating coil demand for continuous fan operation as:
            //    (the zone sensible load - the zone sensible load met by fan heat and mixed air)
            // Note; The sensible zone load met by fan heat and mixed air is calculated as:
            //     mdotsys(control zone inlet enthalpy - control zone outlet enthalpy)
            // This accounts for the negative sign in the equation.
            HPCoilSensDemand = TotalZoneSensLoad - ZoneSensLoadMetFanONCompOFF;

            // Calculate the heating coil capacity for continuous fan operation as:
            //    (the zone sensible load met by fan heat and mixed air and coil
            //   - the zone sensible load met by fan heat and mixed air)
            HPCoilSensCapacity = ZoneSensLoadMetFanONCompON - ZoneSensLoadMetFanONCompOFF;

            // Calculate the part load ratio for continuous fan operation with cycling coil
            if (HPCoilSensCapacity == 0.0) {
                HeatPartLoadRatio = 0.0;
            } else {
                HeatPartLoadRatio = max(MinPLR, min(1.0, std::abs(HPCoilSensDemand) / std::abs(HPCoilSensCapacity)));
            }

            state.dataFurnaces->Furnace(FurnaceNum).InitHeatPump = false;

            //       check bounds on sensible output prior to iteration using RegulaFalsi
            if (ZoneSensLoadMetFanONCompON < TotalZoneSensLoad) {
                HeatPartLoadRatio = 1.0;
                ZoneSensLoadMet = ZoneSensLoadMetFanONCompON;
                HPCoilSensDemand = std::abs(ZoneSensLoadMetFanONCompON - ZoneSensLoadMetFanONCompOFF);
                state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand = HPCoilSensDemand;
            } else if (ZoneSensLoadMetFanONCompOFF > TotalZoneSensLoad) {
                HeatPartLoadRatio = 0.0;
                ZoneSensLoadMet = ZoneSensLoadMetFanONCompOFF;
                state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 0.0; // compressor ON
                state.dataFurnaces->Furnace(FurnaceNum).WSHPRuntimeFrac = 0.0;
                CalcFurnaceOutput(state,
                                  FurnaceNum,
                                  FirstHVACIteration,
                                  OpMode,
                                  CompressorOp,
                                  CoolPartLoadRatio,
                                  HeatPartLoadRatio,
                                  Dummy2,
                                  Dummy2,
                                  ZoneSensLoadMet,
                                  ZoneLatLoadMet,
                                  OnOffAirFlowRatio,
                                  false);
            } else {
                //         Calculate the sensible part load ratio through iteration
                HeatErrorToler = state.dataFurnaces->Furnace(FurnaceNum).HeatingConvergenceTolerance;
                SolFlag = 0; // # of iterations if positive, -1 means failed to converge, -2 means bounds are incorrect
                Par[0] = double(FurnaceNum);
                Par[1] = 0.0; // FLAG, if 1.0 then FirstHVACIteration equals TRUE, if 0.0 then FirstHVACIteration equals false
                if (FirstHVACIteration) Par[1] = 1.0;
                Par[2] = double(OpMode);
                Par[3] = double(CompressorOp);
                Par[4] = TotalZoneSensLoad;
                Par[5] = 0.0;                         // FLAG, 0.0 if heating load, 1.0 if cooling or moisture load
                Par[6] = 1.0;                         // FLAG, 0.0 if latent load, 1.0 if sensible load to be met
                Par[7] = ZoneSensLoadMetFanONCompOFF; // Output with fan ON compressor OFF
                Par[8] = 0.0;                         // HX is OFF for water-to-air HP
                //         HeatErrorToler is in fraction of load, MaxIter = 600, SolFalg = # of iterations or error as appropriate
                auto f = [&state, FurnaceNum, FirstHVACIteration, OpMode, CompressorOp, TotalZoneSensLoad, ZoneSensLoadMetFanONCompOFF](
                             Real64 const PartLoadRatio) {
                    return CalcWaterToAirResidual(state,
                                                  PartLoadRatio,
                                                  FurnaceNum,
                                                  FirstHVACIteration,
                                                  OpMode,
                                                  CompressorOp,
                                                  TotalZoneSensLoad,
                                                  0.0,
                                                  1.0,
                                                  ZoneSensLoadMetFanONCompOFF,
                                                  0.0);
                };
                General::SolveRoot(state, HeatErrorToler, MaxIter, SolFlag, HeatPartLoadRatio, f, 0.0, 1.0);
                state.dataHVACGlobal->OnOffFanPartLoadFraction = state.dataFurnaces->OnOffFanPartLoadFractionSave;
                CalcFurnaceOutput(state,
                                  FurnaceNum,
                                  FirstHVACIteration,
                                  OpMode,
                                  CompressorOp,
                                  CoolPartLoadRatio,
                                  HeatPartLoadRatio,
                                  Dummy2,
                                  Dummy2,
                                  ZoneSensLoadMet,
                                  ZoneLatLoadMet,
                                  OnOffAirFlowRatio,
                                  false);
                if (SolFlag == -1 && !state.dataGlobal->WarmupFlag && !FirstHVACIteration) {
                    if (std::abs(ZoneSensLoadMet - TotalZoneSensLoad) / TotalZoneSensLoad > HeatErrorToler) {
                        if (state.dataFurnaces->Furnace(FurnaceNum).WSHPHeatMaxIterIndex == 0) {
                            ShowWarningMessage(state,
                                               format("Heating coil control failed to converge for {}:{}",
                                                      cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                                      state.dataFurnaces->Furnace(FurnaceNum).Name));
                            ShowContinueError(state, "  Iteration limit exceeded in calculating DX heating coil sensible part-load ratio.");
                            ShowContinueErrorTimeStamp(state,
                                                       format("Sensible load to be met by DX coil = {:.2T} (watts), sensible output of DX coil = "
                                                              "{:.2T} (watts), and the simulation continues.",
                                                              TotalZoneSensLoad,
                                                              ZoneSensLoadMet));
                        }
                        ShowRecurringWarningErrorAtEnd(
                            state,
                            cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num) + " \"" +
                                state.dataFurnaces->Furnace(FurnaceNum).Name +
                                "\" - Iteration limit exceeded in calculating sensible heating part-load ratio error continues.",
                            state.dataFurnaces->Furnace(FurnaceNum).WSHPHeatMaxIterIndex,
                            TotalZoneSensLoad,
                            TotalZoneSensLoad);
                    }
                } else if (SolFlag == -2) {
                    HeatPartLoadRatio = max(MinPLR, min(1.0, std::abs(HPCoilSensDemand) / std::abs(HPCoilSensCapacity)));
                    CalcFurnaceOutput(state,
                                      FurnaceNum,
                                      FirstHVACIteration,
                                      OpMode,
                                      CompressorOp,
                                      0.0,
                                      HeatPartLoadRatio,
                                      0.0,
                                      0.0,
                                      ZoneSensLoadMet,
                                      ZoneLatLoadMet,
                                      OnOffAirFlowRatio,
                                      false);
                    if ((ZoneSensLoadMet - TotalZoneSensLoad) / TotalZoneSensLoad > HeatErrorToler) {
                        if (state.dataFurnaces->Furnace(FurnaceNum).WSHPHeatRegulaFalsiFailedIndex == 0) {
                            ShowWarningError(state,
                                             format("Heating coil control failed for {}:{}",
                                                    cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                                    state.dataFurnaces->Furnace(FurnaceNum).Name));
                            ShowContinueError(state, "  Heating sensible part-load ratio determined to be outside the range of 0-1.");
                            ShowContinueError(
                                state,
                                format("  An estimated part-load ratio = {:.2T} will be used and the simulation continues.", HeatPartLoadRatio));
                            ShowContinueError(
                                state, format("  The estimated part-load ratio provides a heating sensible capacity = {:.2T}", ZoneSensLoadMet));
                            ShowContinueErrorTimeStamp(state, format("  Heating sensible load required = {:.2T}", TotalZoneSensLoad));
                        }
                        ShowRecurringWarningErrorAtEnd(state,
                                                       cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num) + " \"" +
                                                           state.dataFurnaces->Furnace(FurnaceNum).Name +
                                                           "\" - Heating sensible part-load ratio out of range error continues.",
                                                       state.dataFurnaces->Furnace(FurnaceNum).WSHPHeatRegulaFalsiFailedIndex,
                                                       TotalZoneSensLoad,
                                                       TotalZoneSensLoad);
                    }
                }
            }

            //       CALL supplemental heater if required
            if ((TotalZoneSensLoad - ZoneSensLoadMet) > SmallLoad && HeatPartLoadRatio >= 1.0) {
                SuppHeatCoilLoad = TotalZoneSensLoad - ZoneSensLoadMet;
                CalcFurnaceOutput(state,
                                  FurnaceNum,
                                  FirstHVACIteration,
                                  OpMode,
                                  CompressorOp,
                                  CoolPartLoadRatio,
                                  HeatPartLoadRatio,
                                  SuppHeatCoilLoad,
                                  Dummy2,
                                  ZoneSensLoadMet,
                                  ZoneLatLoadMet,
                                  OnOffAirFlowRatio,
                                  false);
            }

            if (OpMode == CycFanCycCoil) {
                state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace *= HeatPartLoadRatio;
            }

            //**********HVAC Scheduled ON, but no cooling, dehumidification or heating load*********
        } else if (GetCurrentScheduleValue(state, state.dataFurnaces->Furnace(FurnaceNum).SchedPtr) > 0.0) {
            state.dataFurnaces->Furnace(FurnaceNum).InitHeatPump = true; // initialization call to Calc Furnace
            HeatPartLoadRatio = 0.0;
            CoolPartLoadRatio = 0.0;
            state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0; //! see 'Note' under INITIAL CALCULATIONS
            // set report variables
            state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 0.0;
            state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand = 0.0;
            state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilLatentDemand = 0.0;
            state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand = 0.0;
            if (OpMode == CycFanCycCoil) {
                state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace = 0.0;
                state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0; // see 'Note' under INITIAL CALCULATIONS
                CalcFurnaceOutput(state,
                                  FurnaceNum,
                                  FirstHVACIteration,
                                  OpMode,
                                  CompressorOp,
                                  CoolPartLoadRatio,
                                  HeatPartLoadRatio,
                                  Dummy2,
                                  Dummy2,
                                  ZoneSensLoadMet,
                                  ZoneLatLoadMet,
                                  OnOffAirFlowRatio,
                                  false);
                state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace = 0.0;
            } else { // continuous fan, cycling coil
                CalcFurnaceOutput(state,
                                  FurnaceNum,
                                  FirstHVACIteration,
                                  OpMode,
                                  CompressorOp,
                                  CoolPartLoadRatio,
                                  HeatPartLoadRatio,
                                  Dummy2,
                                  Dummy2,
                                  ZoneSensLoadMet,
                                  ZoneLatLoadMet,
                                  OnOffAirFlowRatio,
                                  false);
            }
            //*********No heating or cooling or dehumidification*********
        } else {
            state.dataFurnaces->Furnace(FurnaceNum).InitHeatPump = true; // initialization call to Calc Furnace
            state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace = 0.0;
            HeatPartLoadRatio = 0.0;
            CoolPartLoadRatio = 0.0;
            state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0; // see 'Note' under INITIAL CALCULATIONS
            state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = 0.0;
            state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand = 0.0;
            state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilLatentDemand = 0.0;
            state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand = 0.0;
            CalcFurnaceOutput(state,
                              FurnaceNum,
                              FirstHVACIteration,
                              OpMode,
                              CompressorOp,
                              CoolPartLoadRatio,
                              HeatPartLoadRatio,
                              Dummy2,
                              Dummy2,
                              ZoneSensLoadMet,
                              ZoneLatLoadMet,
                              OnOffAirFlowRatio,
                              false);
            state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace = 0.0;
        }

        // Set the fan inlet node flow rates
        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRateMaxAvail = state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace;
        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace;
    }

    void CalcFurnaceOutput(EnergyPlusData &state,
                           int const FurnaceNum,
                           bool const FirstHVACIteration,
                           int const FanOpMode,                    // Cycling fan or constant fan
                           CompressorOperation const CompressorOp, // Compressor on/off; 1=on, 0=off
                           Real64 const CoolPartLoadRatio,         // DX cooling coil part load ratio
                           Real64 const HeatPartLoadRatio,         // DX heating coil part load ratio (0 for other heating coil types)
                           Real64 const HeatCoilLoad,              // Heating coil load for gas heater
                           Real64 const ReheatCoilLoad,            // Reheating coil load for gas heater
                           Real64 &SensibleLoadMet,                // Sensible cooling load met (furnace outlet with respect to control zone temp)
                           Real64 &LatentLoadMet,     // Latent cooling load met (furnace outlet with respect to control zone humidity ratio)
                           Real64 &OnOffAirFlowRatio, // Ratio of compressor ON mass flow rate to AVERAGE
                           bool const HXUnitOn,       // flag to enable HX based on zone moisture load
                           ObjexxFCL::Optional<Real64 const> CoolingHeatingPLRRat // cooling PLR to heating PLR ratio, used for cycling fan RH control
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   Sept 2001
        //       MODIFIED       Dec 2001
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine calculates to sensible and latent loads met by the DX coils
        // specified.  Load met is the outlet node with respect to the control zone's
        // temperature and humidity ratio.

        // METHODOLOGY EMPLOYED:
        // Simulate each child object in the correct order for each system type. This routine is used in the
        // RegulaFalsi function CALL. Air mass flow rate is set each iteration based on PLR.

        // REFERENCES:
        // na

        // Using/Aliasing
        using HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil;
        using WaterToAirHeatPump::SimWatertoAirHP;
        using WaterToAirHeatPumpSimple::SimWatertoAirHPSimple;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int FurnaceInletNode;     // Furnace inlet node number
        int FurnaceOutletNode;    // Furnace outlet node number
        Real64 AirMassFlow;       // Furnace inlet node temperature
        Real64 WSHPRuntimeFrac;   // Compressor runtime fraction
        Real64 CompPartLoadRatio; // Compressor part load ratio
        Real64 Dummy;             // dummy variable
        Real64 Tout;              // Temporary variable used when outlet temp > DesignMaxOutletTemp
        Real64 Wout;              // Temporary variable used when outlet temp > DesignMaxOutletTemp
        int CoolingCoilType_Num;  // Numeric Equivalent for CoolingCoilType
        int HeatingCoilType_Num;  // Numeric Equivalent for HeatingCoilType
        Real64 QActual;           // heating coil load met or delivered
        bool SuppHeatingCoilFlag; // .TRUE. if supplemental heating coil

        FurnaceOutletNode = state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum;
        FurnaceInletNode = state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum;
        CoolingCoilType_Num = state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilType_Num;
        HeatingCoilType_Num = state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num;
        WSHPRuntimeFrac = state.dataFurnaces->Furnace(FurnaceNum).WSHPRuntimeFrac;
        CompPartLoadRatio = state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio;
        state.dataFurnaces->ModifiedHeatCoilLoad = 0.0;

        if (present(CoolingHeatingPLRRat)) {
            state.dataFurnaces->CoolHeatPLRRat = CoolingHeatingPLRRat;
        } else {
            state.dataFurnaces->CoolHeatPLRRat = 1.0;
        }

        // Cooling to Heating PLR Ratio (CoolHeatPLRRat) is used to track the air mass flow rate of both the heating
        // and cooling coils when RH control is used and the heating coil operates longer than the cooling coil.
        // When CoolPartLoadRatio/CoolHeatPLRRat is used, the PLR calculated is acutally the PLR for the heating
        // coil (heating PLR is greater than cooling PLR), it is this PLR that determines the air mass flow rate.
        // When MAX(HeatPartLoadRatio,CoolPartLoadRatio) is used, only one of these values is non-zero.
        if (FanOpMode == CycFanCycCoil) {
            if (state.dataFurnaces->CoolHeatPLRRat < 1.0) {
                if (state.dataFurnaces->CoolHeatPLRRat > 0.0) {
                    state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate =
                        state.dataFurnaces->CompOnMassFlow * CoolPartLoadRatio / state.dataFurnaces->CoolHeatPLRRat;
                    if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num != UnitarySys_HeatPump_WaterToAir) {
                        SetAverageAirFlow(state, FurnaceNum, CoolPartLoadRatio / state.dataFurnaces->CoolHeatPLRRat, OnOffAirFlowRatio);
                    }
                } else {
                    state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = state.dataFurnaces->CompOnMassFlow * CoolPartLoadRatio;
                    if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num != UnitarySys_HeatPump_WaterToAir) {
                        SetAverageAirFlow(state, FurnaceNum, max(HeatPartLoadRatio, CoolPartLoadRatio), OnOffAirFlowRatio);
                    }
                }
            } else {
                state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate =
                    state.dataFurnaces->CompOnMassFlow * max(HeatPartLoadRatio, CoolPartLoadRatio);
                if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num != UnitarySys_HeatPump_WaterToAir) {
                    SetAverageAirFlow(state, FurnaceNum, max(HeatPartLoadRatio, CoolPartLoadRatio), OnOffAirFlowRatio);
                }
            }
        } else {
            if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num != UnitarySys_HeatPump_WaterToAir) {
                SetAverageAirFlow(state, FurnaceNum, max(HeatPartLoadRatio, CoolPartLoadRatio), OnOffAirFlowRatio);
            }
        }

        AirMassFlow = state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate;
        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRateMaxAvail = AirMassFlow;

        // Simulate the air-to-air heat pump
        if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_AirToAir) {
            //   Simulate blow-thru fan and non-linear coils twice to update PLF used by the ONOFF Fan
            if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == BlowThru) {
                SimulateFanComponents(
                    state, BlankString, FirstHVACIteration, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->FanSpeedRatio);
                if (CoolingCoilType_Num == CoilDX_CoolingHXAssisted) {
                    SimHXAssistedCoolingCoil(state,
                                             BlankString,
                                             FirstHVACIteration,
                                             CompressorOp,
                                             CoolPartLoadRatio,
                                             state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                             FanOpMode,
                                             HXUnitOn,
                                             OnOffAirFlowRatio,
                                             state.dataFurnaces->EconomizerFlag);
                } else {
                    SimDXCoil(state,
                              BlankString,
                              CompressorOp,
                              FirstHVACIteration,
                              state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                              FanOpMode,
                              CoolPartLoadRatio,
                              OnOffAirFlowRatio);
                }
                SimDXCoil(state,
                          BlankString,
                          CompressorOp,
                          FirstHVACIteration,
                          state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex,
                          FanOpMode,
                          HeatPartLoadRatio,
                          OnOffAirFlowRatio);
                SimulateFanComponents(
                    state, BlankString, FirstHVACIteration, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->FanSpeedRatio);
            }
            //   Simulate cooling and heating coils
            if (CoolingCoilType_Num == CoilDX_CoolingHXAssisted) {
                SimHXAssistedCoolingCoil(state,
                                         BlankString,
                                         FirstHVACIteration,
                                         CompressorOp,
                                         CoolPartLoadRatio,
                                         state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                         FanOpMode,
                                         HXUnitOn,
                                         OnOffAirFlowRatio,
                                         state.dataFurnaces->EconomizerFlag);
            } else {
                SimDXCoil(state,
                          BlankString,
                          CompressorOp,
                          FirstHVACIteration,
                          state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                          FanOpMode,
                          CoolPartLoadRatio,
                          OnOffAirFlowRatio);
            }
            SimDXCoil(state,
                      BlankString,
                      CompressorOp,
                      FirstHVACIteration,
                      state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex,
                      FanOpMode,
                      HeatPartLoadRatio,
                      OnOffAirFlowRatio);
            //   Simulate the draw-thru fan
            if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == DrawThru) {
                SimulateFanComponents(
                    state, BlankString, FirstHVACIteration, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->FanSpeedRatio);
            }
            //   Simulate the supplemental heating coil
            if (state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::CoolReheat && ReheatCoilLoad > 0.0) {
                SuppHeatingCoilFlag = true;
                CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, ReheatCoilLoad, FanOpMode, QActual);
            } else {
                // equivalent to QCoilReq=0.0d0 or ReHeatCoilLoad = 0.0d0
                SuppHeatingCoilFlag = true;
                CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, ReheatCoilLoad, FanOpMode, QActual);
            }
            // Simulate the parameter estimate water-to-air heat pump
        } else if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_WaterToAir &&
                   state.dataFurnaces->Furnace(FurnaceNum).WatertoAirHPType == WatertoAir_Simple) {
            //    Simulate blow-thru fan and non-linear coils twice to update PLF used by the ONOFF Fan
            if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == BlowThru) {
                SimulateFanComponents(
                    state, BlankString, FirstHVACIteration, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->FanSpeedRatio);
                // COIL:WATERTOAIRHPSIMPLE:COOLING
                SimWatertoAirHPSimple(state,
                                      BlankString,
                                      state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                      state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand,
                                      state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilLatentDemand,
                                      FanOpMode,
                                      WSHPRuntimeFrac,
                                      state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                                      state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                                      state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                                      CompressorOp,
                                      CoolPartLoadRatio,
                                      FirstHVACIteration); // CoolPartLoadRatio
                Dummy = 0.0;
                // COIL:WATERTOAIRHPSIMPLE:HEATING
                SimWatertoAirHPSimple(state,
                                      BlankString,
                                      state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex,
                                      state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand,
                                      Dummy,
                                      FanOpMode,
                                      WSHPRuntimeFrac,
                                      state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                                      state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                                      state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                                      CompressorOp,
                                      HeatPartLoadRatio,
                                      FirstHVACIteration); // HeatPartLoadRatio
                //      Simulate the whole thing a second time so that the correct PLF required by the coils is used by the Fan. *******
                SimulateFanComponents(
                    state, BlankString, FirstHVACIteration, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->FanSpeedRatio);
            }
            //    Simulate the cooling and heating coils
            // COIL:WATERTOAIRHPSIMPLE:COOLING
            SimWatertoAirHPSimple(state,
                                  BlankString,
                                  state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                  state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand,
                                  state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilLatentDemand,
                                  FanOpMode,
                                  WSHPRuntimeFrac,
                                  state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                                  state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                                  state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                                  CompressorOp,
                                  CoolPartLoadRatio,
                                  FirstHVACIteration); // CoolPartLoadRatio
            Dummy = 0.0;
            // COIL:WATERTOAIRHPSIMPLE:HEATING
            SimWatertoAirHPSimple(state,
                                  BlankString,
                                  state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex,
                                  state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand,
                                  Dummy,
                                  FanOpMode,
                                  WSHPRuntimeFrac,
                                  state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                                  state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                                  state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                                  CompressorOp,
                                  HeatPartLoadRatio,
                                  FirstHVACIteration); // HeatPartLoadRatio
            //     Simulate the draw-thru fan
            if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == BlowThru) {
                SimulateFanComponents(
                    state, BlankString, FirstHVACIteration, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->FanSpeedRatio);
            }
            //     Simulate the supplemental heating coil
            if (state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::CoolReheat && ReheatCoilLoad > 0.0) {
                SuppHeatingCoilFlag = true; // if true simulates supplemental heating coil
                CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, ReheatCoilLoad, FanOpMode, QActual);
            } else {
                SuppHeatingCoilFlag = true; // if true simulates supplemental heating coil
                CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, FanOpMode, QActual);
            }
            // Simulate the detailed water-to-air heat pump
        } else if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_WaterToAir &&
                   state.dataFurnaces->Furnace(FurnaceNum).WatertoAirHPType == WatertoAir_ParEst) {
            //    Simulate the draw-thru fan
            if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == BlowThru) {
                SimulateFanComponents(
                    state, BlankString, FirstHVACIteration, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->FanSpeedRatio);
            }
            //    Simulate the cooling and heating coils
            SimWatertoAirHP(state,
                            BlankString,
                            state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                            state.dataFurnaces->Furnace(FurnaceNum).DesignMassFlowRate,
                            FanOpMode,
                            FirstHVACIteration,
                            WSHPRuntimeFrac,
                            state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                            state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                            state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                            state.dataFurnaces->Furnace(FurnaceNum).InitHeatPump,
                            state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand,
                            state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilLatentDemand,
                            CompressorOp,
                            CoolPartLoadRatio);
            Dummy = 0.0;
            SimWatertoAirHP(state,
                            BlankString,
                            state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex,
                            state.dataFurnaces->Furnace(FurnaceNum).DesignMassFlowRate,
                            FanOpMode,
                            FirstHVACIteration,
                            WSHPRuntimeFrac,
                            state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                            state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                            state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                            state.dataFurnaces->Furnace(FurnaceNum).InitHeatPump,
                            state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand,
                            Dummy,
                            CompressorOp,
                            HeatPartLoadRatio);
            //    Simulate the draw-thru fan
            if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == DrawThru) {
                SimulateFanComponents(
                    state, BlankString, FirstHVACIteration, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->FanSpeedRatio);
            }
            //    Simulate the supplemental heating coil
            HeatingCoils::SimulateHeatingCoilComponents(
                state, BlankString, FirstHVACIteration, HeatCoilLoad, state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex, _, true, FanOpMode);

        } else { // ELSE it's not a heat pump
            //   Simulate blow-thru fan
            if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == BlowThru) {

                SimulateFanComponents(
                    state, BlankString, FirstHVACIteration, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->FanSpeedRatio);

                //     For non-linear coils, simulate coil to update PLF used by the ONOFF Fan
                if (state.dataFurnaces->Furnace(FurnaceNum).FanType_Num == FanType_SimpleOnOff) {
                    if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num != UnitarySys_HeatOnly &&
                        state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num != Furnace_HeatOnly) {

                        if (!state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilUpstream) {
                            SuppHeatingCoilFlag = false; // if false simulates heating coil
                            CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, FanOpMode, QActual);
                        }

                        if (CoolingCoilType_Num == CoilDX_CoolingHXAssisted) {
                            SimHXAssistedCoolingCoil(state,
                                                     BlankString,
                                                     FirstHVACIteration,
                                                     CompressorOp,
                                                     CoolPartLoadRatio,
                                                     state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                                     FanOpMode,
                                                     HXUnitOn,
                                                     OnOffAirFlowRatio,
                                                     state.dataFurnaces->EconomizerFlag);
                        } else {
                            SimDXCoil(state,
                                      BlankString,
                                      CompressorOp,
                                      FirstHVACIteration,
                                      state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                      FanOpMode,
                                      CoolPartLoadRatio,
                                      OnOffAirFlowRatio,
                                      state.dataFurnaces->CoolHeatPLRRat);
                        }
                    }

                    if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilUpstream) {
                        SuppHeatingCoilFlag = false; // if false simulates heating coil
                        CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, FanOpMode, QActual);
                    }
                    SimulateFanComponents(
                        state, BlankString, FirstHVACIteration, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->FanSpeedRatio);
                } // Simple OnOff fan

            } // Blow thru fan

            //   Simulate the cooling and heating coils
            if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num != UnitarySys_HeatOnly &&
                state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num != Furnace_HeatOnly) {

                if (!state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilUpstream) {
                    SuppHeatingCoilFlag = false; // if false simulates heating coil
                    CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, FanOpMode, QActual);
                }

                if (CoolingCoilType_Num == CoilDX_CoolingHXAssisted) {
                    SimHXAssistedCoolingCoil(state,
                                             BlankString,
                                             FirstHVACIteration,
                                             CompressorOp,
                                             CoolPartLoadRatio,
                                             state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                             FanOpMode,
                                             HXUnitOn,
                                             OnOffAirFlowRatio,
                                             state.dataFurnaces->EconomizerFlag);
                } else {
                    SimDXCoil(state,
                              BlankString,
                              CompressorOp,
                              FirstHVACIteration,
                              state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                              FanOpMode,
                              CoolPartLoadRatio,
                              OnOffAirFlowRatio,
                              state.dataFurnaces->CoolHeatPLRRat);
                }
            }

            if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilUpstream) {
                SuppHeatingCoilFlag = false; // if false simulates heating coil
                CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, FanOpMode, QActual);
            }
            //   Simulate the draw-thru fan
            if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == DrawThru) {
                SimulateFanComponents(
                    state, BlankString, FirstHVACIteration, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->FanSpeedRatio);
            }
            if (state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::CoolReheat ||
                state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex > 0) {
                SuppHeatingCoilFlag = true; // if truee simulates supplemental heating coil
                CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, ReheatCoilLoad, FanOpMode, QActual);
            }
        } // IF(Furnace(FurnaceNum)%FurnaceType_Num == UnitarySys_HeatPump_AirToAir)THEN

        // check the DesignMaxOutletTemp and reset if necessary (for Coil:Gas:Heating or Coil:Electric:Heating only)
        if (state.dataLoopNodes->Node(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum).Temp >
            state.dataFurnaces->Furnace(FurnaceNum).DesignMaxOutletTemp) {
            Wout = state.dataLoopNodes->Node(FurnaceOutletNode).HumRat;
            Tout = state.dataFurnaces->Furnace(FurnaceNum).DesignMaxOutletTemp;
            state.dataFurnaces->ModifiedHeatCoilLoad =
                HeatCoilLoad - (AirMassFlow * PsyCpAirFnW(Wout) * (state.dataLoopNodes->Node(FurnaceOutletNode).Temp - Tout));
            state.dataLoopNodes->Node(FurnaceOutletNode).Temp = Tout;
        }

        // If the fan runs continually do not allow coils to set OnOffFanPartLoadRatio.
        if (FanOpMode == ContFanCycCoil) state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;

        Real64 SensibleOutput(0.0); // sensible output rate, {W}
        Real64 LatentOutput(0.0);   // latent output rate, {W}
        Real64 TotalOutput(0.0);    // total output rate, {W}
        CalcZoneSensibleLatentOutput(AirMassFlow,
                                     state.dataLoopNodes->Node(FurnaceOutletNode).Temp,
                                     state.dataLoopNodes->Node(FurnaceOutletNode).HumRat,
                                     state.dataLoopNodes->Node(state.dataFurnaces->Furnace(FurnaceNum).NodeNumOfControlledZone).Temp,
                                     state.dataLoopNodes->Node(state.dataFurnaces->Furnace(FurnaceNum).NodeNumOfControlledZone).HumRat,
                                     SensibleOutput,
                                     LatentOutput,
                                     TotalOutput);
        SensibleLoadMet = SensibleOutput - state.dataFurnaces->Furnace(FurnaceNum).SenLoadLoss;
        state.dataFurnaces->Furnace(FurnaceNum).SensibleLoadMet = SensibleLoadMet;

        if (state.dataFurnaces->Furnace(FurnaceNum).Humidistat) {
            LatentLoadMet = LatentOutput - state.dataFurnaces->Furnace(FurnaceNum).LatLoadLoss;
        } else {
            LatentLoadMet = 0.0;
        }
        state.dataFurnaces->Furnace(FurnaceNum).LatentLoadMet = LatentLoadMet;
    }

    //        End of Update subroutines for the Furnace Module
    // *****************************************************************************

    Real64 CalcFurnaceResidual(EnergyPlusData &state,
                               Real64 const PartLoadRatio, // DX cooling coil part load ratio
                               int FurnaceNum,
                               bool FirstHVACIteration,
                               int FanOpMode,
                               CompressorOperation CompressorOp,
                               Real64 LoadToBeMet,
                               Real64 par6_loadFlag,
                               Real64 par7_sensLatentFlag,
                               Real64 par9_HXOnFlag,
                               Real64 par10_HeatingCoilPLR)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   Feb 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // To calculate the part-load ratio for cooling and heating coils

        //   Parameter description example:
        //       Par(1)  = REAL(FurnaceNum,r64) ! Index to furnace
        //       Par(2)  = 0.0                  ! FirstHVACIteration FLAG, if 1.0 then TRUE, if 0.0 then FALSE
        //       Par(3)  = REAL(OpMode,r64)     ! Fan control, if 1.0 then cycling fan, if 0.0 then continuous fan
        //       Par(4)  = REAL(CompressorOp,r64)     ! Compressor control, if 1.0 then compressor ON, if 0.0 then compressor OFF
        //       Par(5)  = CoolCoilLoad         ! Sensible or Latent load to be met by furnace
        //       Par(6)  = 1.0                  ! Type of load FLAG, 0.0 if heating load, 1.0 if cooling or moisture load
        //       Par(7)  = 1.0                  ! Output calculation FLAG, 0.0 for latent capacity, 1.0 for sensible capacity
        //       Par(8)  = OnOffAirFlowRatio    ! Ratio of compressor ON air mass flow to AVERAGE air mass flow over time step
        //       Par(9)  = HXUnitOn             ! flag to enable HX, 1=ON and 2=OFF
        //       Par(10) = HeatingCoilPLR       ! used to calculate latent degradation for cycling fan RH control

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 CoolPartLoadRatio;      // DX cooling coil part load ratio
        Real64 HeatPartLoadRatio;      // DX heating coil part load ratio (0 for other heating coil types)
        Real64 HeatCoilLoad;           // Heating coil load for gas heater
        Real64 SensibleLoadMet;        // Sensible cooling load met (furnace outlet with respect to control zone temp)
        Real64 LatentLoadMet;          // Latent cooling load met (furnace outlet with respect to control zone humidity ratio)
        Real64 OnOffAirFlowRatio;      // Ratio of compressor ON air mass flow to AVERAGE air mass flow over time step
        Real64 RuntimeFrac;            // heat pump runtime fraction
        Real64 CoolingHeatingPLRRatio; // ratio of cooling PLR to heating PLR, used for cycling fan RH control
        bool HXUnitOn;                 // flag to enable HX based on zone moisture load
        bool errFlag;                  // flag denoting error in runtime calculation

        //        // Convert parameters to usable variables
        //        int FurnaceNum = int(Par(1));
        //        bool FirstHVACIteration = Par(2) == 1.0;
        //        int FanOpMode = int(Par(3));
        //        CompressorOperation CompressorOp = static_cast<CompressorOperation>(Par(4));
        //        Real64 LoadToBeMet = Par(5);
        //        Real64 par6_loadFlag = Par(6);
        //        Real64 par7_sensLatentFlag = Par(7);
        //        Real64 par9_HXOnFlag = Par(9);
        //        Real64 par10_HeatingCoilPLR = Par(10);

        if (par6_loadFlag == 1.0) {
            CoolPartLoadRatio = PartLoadRatio;
            HeatPartLoadRatio = 0.0;
            HeatCoilLoad = 0.0;
        } else {
            CoolPartLoadRatio = 0.0;
            HeatPartLoadRatio = PartLoadRatio;

            int const HeatingCoilType_Num(state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num);
            if (HeatingCoilType_Num == Coil_HeatingGasOrOtherFuel || HeatingCoilType_Num == Coil_HeatingElectric ||
                HeatingCoilType_Num == Coil_HeatingWater || HeatingCoilType_Num == Coil_HeatingSteam) {
                HeatCoilLoad = state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity * PartLoadRatio;
            } else {
                HeatCoilLoad = 0.0;
            }
        }

        //  OnOffAirFlowRatio = Par(8)
        if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_WaterToAir) {
            HeatPumpRunFrac(state, FurnaceNum, PartLoadRatio, errFlag, RuntimeFrac);
            state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = PartLoadRatio;
            state.dataFurnaces->Furnace(FurnaceNum).WSHPRuntimeFrac = RuntimeFrac;
        }

        if (par9_HXOnFlag == 1.0) {
            HXUnitOn = true;
        } else {
            HXUnitOn = false;
        }

        if (par10_HeatingCoilPLR > 0.0) {
            //    Par(10) = Furnace(FurnaceNum)%HeatPartLoadRatio
            //    FanOpMode = CycFan and Furnace(FurnaceNum)%HeatPartLoadRatio must be > 0 for Part(10) to be greater than 0
            //    This variable used when in heating mode and dehumidification (cooling) is required.
            CoolingHeatingPLRRatio = min(1.0, CoolPartLoadRatio / state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio);
        } else {
            CoolingHeatingPLRRatio = 1.0;
        }

        // Subroutine arguments
        CalcFurnaceOutput(state,
                          FurnaceNum,
                          FirstHVACIteration,
                          FanOpMode,
                          CompressorOp,
                          CoolPartLoadRatio,
                          HeatPartLoadRatio,
                          HeatCoilLoad,
                          0.0,
                          SensibleLoadMet,
                          LatentLoadMet,
                          OnOffAirFlowRatio,
                          HXUnitOn,
                          CoolingHeatingPLRRatio);

        // Calculate residual based on output calculation flag
        if (par7_sensLatentFlag == 1.0) {
            if (LoadToBeMet == 0.0) {
                return (SensibleLoadMet - LoadToBeMet) / 100.0;
            } else {
                return (SensibleLoadMet - LoadToBeMet) / LoadToBeMet;
            }
        } else {
            if (LoadToBeMet == 0.0) {
                return (LatentLoadMet - LoadToBeMet) / 100.0;
            } else {
                return (LatentLoadMet - LoadToBeMet) / LoadToBeMet;
            }
        }
    }

    Real64 CalcWaterToAirResidual(EnergyPlusData &state,
                                  Real64 const PartLoadRatio, // DX cooling coil part load ratio
                                  int FurnaceNum,
                                  bool FirstHVACIteration,
                                  int FanOpMode,
                                  CompressorOperation CompressorOp,
                                  Real64 LoadToBeMet,
                                  Real64 par6_loadTypeFlag,
                                  Real64 par7_latentOrSensible,
                                  Real64 ZoneSensLoadMetFanONCompOFF,
                                  Real64 par9_HXUnitOne)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   October 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // To calculate the part-load ratio for water to air HP's
        // this is used for parameter estimation WAHPs but not equation fit WAHPs

        //   Parameter description example:
        //     Par(1)  = REAL(FurnaceNum,r64) ! Index to furnace
        //     Par(2)  = 0.0                  ! FirstHVACIteration FLAG, if 1.0 then TRUE, if 0.0 then FALSE
        //     Par(3)  = REAL(OpMode,r64)     ! Fan control, if 1.0 then cycling fan, if 0.0 then continuous fan
        //     Par(4)  = REAL(CompressorOp,r64)     ! Compressor control, if 1.0 then compressor ON, if 0.0 then compressor OFF
        //     Par(5)  = CoolCoilLoad         ! Sensible or Latent load to be met by furnace
        //     Par(6)  = 1.0                  ! Type of load FLAG, 0.0 if heating load, 1.0 if cooling or moisture load
        //     Par(7)  = 1.0                  ! Output calculation FLAG, 0.0 for latent capacity, 1.0 for sensible capacity
        //     Par(8)  = ZoneSensLoadMetFanONCompOFF  ! Output with fan ON compressor OFF

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 CoolPartLoadRatio; // DX cooling coil part load ratio
        Real64 HeatPartLoadRatio; // DX heating coil part load ratio (0 for other heating coil types)
        Real64 HeatCoilLoad;      // Heating coil load for gas heater
        Real64 ZoneSensLoadMet;   // Sensible cooling load met (furnace outlet with respect to control zone temp)
        Real64 ZoneLatLoadMet;    // Latent cooling load met (furnace outlet with respect to control zone humidity ratio)
        bool errFlag;
        Real64 RuntimeFrac;
        Real64 Dummy;
        Real64 HPCoilSensDemand;
        Real64 OnOffAirFlowRatio;
        bool HXUnitOn; // flag to enable HX based on zone moisture load (not valid for water-to-air HP's

        // Convert parameters to usable variables
        //        int FurnaceNum = int(Par[0]);
        //        bool FirstHVACIteration = Par[1] == 1.0;
        //        int FanOpMode = int(Par[2]);
        //        CompressorOperation CompressorOp = static_cast<CompressorOperation>(Par[3]);
        //        Real64 LoadToBeMet = Par[4];
        //        Real64 par6_loadTypeFlag = Par[5];
        //        Real64 par7_latentOrSensible = Par[6];
        //        Real64 ZoneSensLoadMetFanONCompOFF = Par[7];
        //        Real64 par9_HXUnitOne = Par[8];

        if (par6_loadTypeFlag == 1.0) {
            CoolPartLoadRatio = PartLoadRatio;
            HeatPartLoadRatio = 0.0;
            HeatCoilLoad = 0.0;
        } else {
            CoolPartLoadRatio = 0.0;
            HeatPartLoadRatio = PartLoadRatio;
        }

        // calculate the run time fraction
        HeatPumpRunFrac(state, FurnaceNum, PartLoadRatio, errFlag, RuntimeFrac);

        // update the fan part load factor
        // see 'Note' under INITIAL CALCULATIONS
        if (par6_loadTypeFlag == 1.0) {
            if (RuntimeFrac > 0.0) {
                state.dataHVACGlobal->OnOffFanPartLoadFraction = CoolPartLoadRatio / RuntimeFrac;
            } else {
                state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
            }
        } else {
            if (RuntimeFrac > 0.0) {
                state.dataHVACGlobal->OnOffFanPartLoadFraction = PartLoadRatio / RuntimeFrac;
                //   Else IF(RuntimeFrac == 0.0d0)THEN
                //     OnOffFanPartLoadFraction = 0.0
            } else {
                state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
            }
        }
        state.dataFurnaces->OnOffFanPartLoadFractionSave = state.dataHVACGlobal->OnOffFanPartLoadFraction;
        // update fan and compressor run times
        state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = PartLoadRatio;
        state.dataFurnaces->Furnace(FurnaceNum).WSHPRuntimeFrac = RuntimeFrac;

        // Calculate the heating coil demand as (the zone sensible load - load met by fan heat and mixed air)
        // Note; The load met by fan heat and mixed air is calculated as mdot(zoneinletenthalpy-zoneoutletenthalpy)
        // This accounts for the negative sign in the equation.

        // Calculate the heat coil sensible capacity as the load met by the system with the fan and compressor on less
        // the load met by the system with the compressor off.
        //  HPCoilSensCapacity = ZoneSensLoadMetFanONCompON - ZoneSensLoadMetFanONCompOFF

        // Set input parameters for heat pump coil model
        HPCoilSensDemand = LoadToBeMet - RuntimeFrac * ZoneSensLoadMetFanONCompOFF;
        //  HPCoilSensDemand = LoadToBeMet  - PartLoadRatio*ZoneSensLoadMetFanONCompOFF
        if (par6_loadTypeFlag == 1.0) {
            state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand = 0.0;
            state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand = std::abs(HPCoilSensDemand);
        } else {
            state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand = HPCoilSensDemand;
            state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand = 0.0;
        }
        state.dataFurnaces->Furnace(FurnaceNum).InitHeatPump = false; // initialization call to Calc Furnace

        // Calculate the zone loads met and the new part load ratio and for the specified run time
        Dummy = 0.0;
        OnOffAirFlowRatio = 1.0;
        if (par9_HXUnitOne == 1.0) {
            HXUnitOn = true;
        } else {
            HXUnitOn = false;
        }

        //  Subroutine arguments
        //  CALL CalcFurnaceOutput(FurnaceNum,FirstHVACIteration,FanOpMode,CompressorOp,CoolPartLoadRatio,&
        //                         HeatPartLoadRatio, HeatCoilLoad, ReHeatCoilLoad, SensibleLoadMet, LatentLoadMet, HXUnitOn)
        CalcFurnaceOutput(state,
                          FurnaceNum,
                          FirstHVACIteration,
                          FanOpMode,
                          CompressorOp,
                          CoolPartLoadRatio,
                          HeatPartLoadRatio,
                          Dummy,
                          Dummy,
                          ZoneSensLoadMet,
                          ZoneLatLoadMet,
                          OnOffAirFlowRatio,
                          HXUnitOn);

        // Calculate residual based on output calculation flag
        if (par7_latentOrSensible == 1.0) {
            return (ZoneSensLoadMet - LoadToBeMet) / LoadToBeMet;
        } else {
            return (ZoneLatLoadMet - LoadToBeMet) / LoadToBeMet;
        }
    }

    void SetAverageAirFlow(EnergyPlusData &state,
                           int const FurnaceNum,       // Unit index
                           Real64 const PartLoadRatio, // unit part load ratio
                           Real64 &OnOffAirFlowRatio   // ratio of compressor ON airflow to AVERAGE airflow over timestep
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2005
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Set the average air mass flow rates using the part-load fraction of the HVAC system for this time step
        // Set OnOffAirFlowRatio to be used by DX coils

        // METHODOLOGY EMPLOYED:
        // The air flow rate in cooling, heating, and no cooling or heating can be different.
        // Calculate the air flow rate based on initializations made in InitFurnace.

        // REFERENCES:
        // na

        // Using/Aliasing
        using namespace DataZoneEnergyDemands;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int InletNode;              // inlet node number for furnace
        Real64 AverageUnitMassFlow; // average supply air mass flow rate over time step

        InletNode = state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum;

        AverageUnitMassFlow = (PartLoadRatio * state.dataFurnaces->CompOnMassFlow) + ((1 - PartLoadRatio) * state.dataFurnaces->CompOffMassFlow);
        if (state.dataFurnaces->CompOffFlowRatio > 0.0) {
            state.dataFurnaces->FanSpeedRatio =
                (PartLoadRatio * state.dataFurnaces->CompOnFlowRatio) + ((1 - PartLoadRatio) * state.dataFurnaces->CompOffFlowRatio);
        } else {
            state.dataFurnaces->FanSpeedRatio = state.dataFurnaces->CompOnFlowRatio;
        }
        // IF the furnace is scheduled on or nightime cycle overrides fan schedule. Uses same logic as fan.
        if (GetCurrentScheduleValue(state, state.dataFurnaces->Furnace(FurnaceNum).SchedPtr) > 0.0 &&
            ((GetCurrentScheduleValue(state, state.dataFurnaces->Furnace(FurnaceNum).FanAvailSchedPtr) > 0.0 || state.dataHVACGlobal->TurnFansOn) &&
             !state.dataHVACGlobal->TurnFansOff)) {
            state.dataLoopNodes->Node(InletNode).MassFlowRate = AverageUnitMassFlow;
            state.dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = AverageUnitMassFlow;
            if (AverageUnitMassFlow > 0.0) {
                OnOffAirFlowRatio = state.dataFurnaces->CompOnMassFlow / AverageUnitMassFlow;
            } else {
                OnOffAirFlowRatio = 0.0;
            }
        } else {
            state.dataLoopNodes->Node(InletNode).MassFlowRate = 0.0;
            OnOffAirFlowRatio = 1.0;
        }

        state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace = state.dataFurnaces->CompOnMassFlow;
        state.dataFurnaces->OnOffAirFlowRatioSave = OnOffAirFlowRatio;
    }

    void HeatPumpRunFrac(EnergyPlusData &state,
                         int const FurnaceNum, // Furnace Index Number
                         Real64 const PLR,     // part load ratio
                         bool &errFlag,        // part load factor out of range flag
                         Real64 &RuntimeFrac   // the required run time fraction to meet part load
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Kenneth Tang
        //       DATE WRITTEN   Apr 2004
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine calculates the PLF based on the PLR. Parameters required are
        // thermostat cycling rate (Nmax), heat pump time constant (tau), and the fraction
        // of on-cycle power use (pr)

        // METHODOLOGY EMPLOYED:
        // NA

        // REFERENCES:
        // (1) Henderson, H. I., K. Rengarajan.1996. A Model to predict the latent capacity
        // of air conditioners and heat pumps at part-load conditions with constant fan
        // operation. ASHRAE Transactions 102 (1): 266-274

        // (2) Henderson, H.I. Jr., Y.J. Huang and Danny Parker. 1999. Residential Equipment
        // Part Load Curves for Use in DOE-2.  Environmental Energy Technologies Division,
        // Ernest Orlando Lawrence Berkeley National Laboratory.

        // USE STATEMENTS:

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 PartLoadFactor; // Part load factor
        Real64 Nmax;           // Maximum cycling rate [cycles/hr]
        Real64 tau;            // Heat pump time constant [s]
        Real64 pr;             // On-cycle power use fraction [~]
        Real64 error;          // Calculation error
        Real64 PLF1;           // ith term of part load factor
        Real64 PLF2;           // (i+1)th term of part load factor
        Real64 A;              // Variable for simplify equation
        int NumIteration;      // Iteration Counter

        Nmax = state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour;
        tau = state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant;
        pr = state.dataFurnaces->Furnace(FurnaceNum).OnCyclePowerFraction;

        // Initialize
        errFlag = false;
        error = 1.0;
        NumIteration = 0;

        // Initial guess for part load fraction
        PLF1 = 1.0;

        // Calculate PLF using successive substitution until convergence is achieved
        while (true) {
            ++NumIteration;

            if (PLR == 1) {
                // Set part load fraction, PLF1=1.0 if PLR=1.0 and exit loop
                PLF1 = 1.0;
                goto LOOPPLF_exit;
            }

            if (NumIteration > 100) {
                // Exit loop if interation exceed 100
                errFlag = true;
                PLF1 = 1.0;
                goto LOOPPLF_exit;
            }

            if (error < 0.00001) {
                // Exit loop if convergence is achieved
                goto LOOPPLF_exit;

            } else {
                // Calculate PLF
                A = 4.0 * tau * (Nmax / 3600.0) * (1 - PLR / PLF1);
                if (A < 1.5e-3) {
                    // A safety check to prevent PLF2 = 1 - A * (1 - Exp(-1 / A))
                    // from "float underflow error". Occurs when PLR is very close to 1.0,
                    // small A value, thus Exp(-1/A) = 0
                    PLF2 = 1 - A;
                } else {
                    PLF2 = 1.0 - A * (1.0 - std::exp(-1.0 / A));
                }
                error = std::abs((PLF2 - PLF1) / PLF1);
                PLF1 = PLF2;
            }
        }
    LOOPPLF_exit:;

        // Adjust PLF for the off cycle power consumption if
        // on-cycle power use is specified by the user
        if (pr > 0.0) {
            PartLoadFactor = PLR / ((PLR / PLF1) + (1 - PLR / PLF1) * pr);
        } else {
            PartLoadFactor = PLF1;
        }

        if (PartLoadFactor <= 0.0) {
            PartLoadFactor = 0.0;
            RuntimeFrac = 0.0;
            errFlag = true;
        } else {
            RuntimeFrac = PLR / PartLoadFactor;
        }

        if (RuntimeFrac > 1.0) {
            RuntimeFrac = 1.0;
        }
    }

    // Beginning of Reporting subroutines for the Furnace Module
    // *****************************************************************************

    void ReportFurnace(EnergyPlusData &state, int const FurnaceNum, int const AirLoopNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   Feb 2001
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the report variable for the coils.

        // METHODOLOGY EMPLOYED:
        // Update fan part-load ratio based on mass flow rate ratio.
        // Update global variables used by AirflowNetwork module.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 ratio;
        Real64 OnOffRatio;

        // Report the Furnace Fan Part Load Ratio
        if (state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling < 1) {
            if (state.dataFurnaces->Furnace(FurnaceNum).DesignMassFlowRate > 0.0) {
                state.dataFurnaces->Furnace(FurnaceNum).FanPartLoadRatio =
                    state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace / state.dataFurnaces->Furnace(FurnaceNum).DesignMassFlowRate;
            } else {
                state.dataFurnaces->Furnace(FurnaceNum).FanPartLoadRatio = 0.0;
            }
        }

        // Set mass flow rates during on and off cylce using an OnOff fan
        if (state.afn->distribution_simulated) {
            state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopSystemOnMassFlowrate = state.dataFurnaces->CompOnMassFlow;
            state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopSystemOffMassFlowrate = state.dataFurnaces->CompOffMassFlow;
            state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopFanOperationMode = state.dataFurnaces->Furnace(FurnaceNum).OpMode;
            state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio = state.dataFurnaces->Furnace(FurnaceNum).FanPartLoadRatio;
            OnOffRatio = state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio;
            if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatPump_AirToAir) {
                state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio =
                    max(state.dataFurnaces->Furnace(FurnaceNum).FanPartLoadRatio,
                        state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio,
                        state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio);
                state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio =
                    min(1.0, state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio);
            }
            if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatCool) {
                if (state.dataFurnaces->Furnace(FurnaceNum).HeatPartLoadRatio == 0.0 &&
                    state.dataFurnaces->Furnace(FurnaceNum).CoolPartLoadRatio == 0.0 &&
                    state.dataFurnaces->Furnace(FurnaceNum).FanPartLoadRatio > 0.0) {
                    if (state.dataFurnaces->CompOnMassFlow < max(state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirMassFlow,
                                                                 state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirMassFlow) &&
                        state.dataFurnaces->CompOnMassFlow > 0.0) {
                        ratio = max(state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirMassFlow,
                                    state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirMassFlow) /
                                state.dataFurnaces->CompOnMassFlow;
                        state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio =
                            state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio * ratio;
                    }
                }
            }
        }
        if (state.dataFurnaces->Furnace(FurnaceNum).FirstPass) {
            if (!state.dataGlobal->SysSizingCalc) {
                DataSizing::resetHVACSizingGlobals(state, 0, state.dataSize->CurSysNum, state.dataFurnaces->Furnace(FurnaceNum).FirstPass);
            }
        }
        state.dataHVACGlobal->OnOffFanPartLoadFraction =
            1.0; // reset to 1 in case blow through fan configuration (fan resets to 1, but for blow thru fans coil sets back down < 1)
    }

    void CalcNonDXHeatingCoils(EnergyPlusData &state,
                               int const FurnaceNum,           // Furnace Index
                               bool const SuppHeatingCoilFlag, // .TRUE. if supplemental heating coil
                               bool const FirstHVACIteration,  // flag for first HVAC iteration in the time step
                               Real64 const QCoilLoad,         // load met by unit (watts)
                               int const FanMode,              // fan operation mode
                               Real64 &HeatCoilLoadmet         // Heating Load Met
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Bereket Nigusse, FSEC/UCF
        //       DATE WRITTEN   January 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine simulates the four non dx heating coil types: Gas, Electric, hot water and steam.

        // METHODOLOGY EMPLOYED:
        // Simply calls the different heating coil component.  The hot water flow rate matching the coil load
        // is calculated iteratively.

        // Using/Aliasing
        using DataHVACGlobals::SmallLoad;
        using PlantUtilities::SetComponentFlowRate;
        using SteamCoils::SimulateSteamCoilComponents;
        using WaterCoils::SimulateWaterCoilComponents;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr ErrTolerance(0.001); // convergence limit for hotwater coil
        int constexpr SolveMaxIter(50);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 QActual;         // actual heating load
        Real64 mdot;            // heating coil steam or hot water mass flow rate
        Real64 MinWaterFlow;    // coil minimum hot water mass flow rate, kg/s
        Real64 MaxHotWaterFlow; // coil maximum hot water mass flow rate, kg/s
        Real64 HotWaterMdot;    // actual hot water mass flow rate
        std::array<Real64, 4> Par;
        int SolFlag;
        std::string &HeatingCoilName = state.dataFurnaces->HeatingCoilName; // name of heating coil
        int CoilTypeNum(0);                                                 // heating coil type number
        int HeatingCoilIndex(0);                                            // heating coil index
        int CoilControlNode(0);                                             // control node for hot water and steam heating coils
        int CoilOutletNode(0);                                              // air outlet node of the heatiing coils
        PlantLocation plantLoc{};                                           // plant loop location

        QActual = 0.0;

        if (SuppHeatingCoilFlag) {
            HeatingCoilName = state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilName;
            HeatingCoilIndex = state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex;
            CoilControlNode = state.dataFurnaces->Furnace(FurnaceNum).SuppCoilControlNode;
            CoilOutletNode = state.dataFurnaces->Furnace(FurnaceNum).SuppCoilOutletNode;
            CoilTypeNum = state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilType_Num;
            plantLoc = state.dataFurnaces->Furnace(FurnaceNum).SuppPlantLoc;
            MaxHotWaterFlow = state.dataFurnaces->Furnace(FurnaceNum).MaxSuppCoilFluidFlow;
        } else {
            HeatingCoilName = state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilName;
            HeatingCoilIndex = state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex;
            CoilControlNode = state.dataFurnaces->Furnace(FurnaceNum).CoilControlNode;
            CoilOutletNode = state.dataFurnaces->Furnace(FurnaceNum).CoilOutletNode;
            CoilTypeNum = state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilType_Num;
            plantLoc = state.dataFurnaces->Furnace(FurnaceNum).plantLoc;
            MaxHotWaterFlow = state.dataFurnaces->Furnace(FurnaceNum).MaxHeatCoilFluidFlow;
        }

        switch (CoilTypeNum) {
        case Coil_HeatingGasOrOtherFuel:
        case Coil_HeatingElectric:
        case Coil_HeatingDesuperheater: {
            HeatingCoils::SimulateHeatingCoilComponents(
                state, HeatingCoilName, FirstHVACIteration, QCoilLoad, HeatingCoilIndex, QActual, SuppHeatingCoilFlag, FanMode);
        } break;
        case Coil_HeatingWater: {
            if (QCoilLoad > SmallLoad) {
                SetComponentFlowRate(state, MaxHotWaterFlow, CoilControlNode, CoilOutletNode, plantLoc);
                SimulateWaterCoilComponents(state, HeatingCoilName, FirstHVACIteration, HeatingCoilIndex, QActual, FanMode);

                if (QActual > (QCoilLoad + SmallLoad)) {
                    // control water flow to obtain output matching QCoilLoad
                    MinWaterFlow = 0.0;
                    Par[0] = double(FurnaceNum);
                    if (FirstHVACIteration) {
                        Par[1] = 1.0;
                    } else {
                        Par[1] = 0.0;
                    }
                    Par[2] = QCoilLoad;
                    if (SuppHeatingCoilFlag) {
                        Par[3] = 1.0;
                    } else {
                        Par[3] = 0.0;
                    }
                    auto f = [&state, FurnaceNum, FirstHVACIteration, QCoilLoad, SuppHeatingCoilFlag](Real64 const HWFlow) {
                        Real64 QCoilRequested = QCoilLoad;

                        // FUNCTION LOCAL VARIABLE DECLARATIONS:
                        Real64 QCoilActual;   // delivered coil load, W
                        Real64 mdot = HWFlow; // to get non-const argument
                        QCoilActual = QCoilRequested;
                        if (!SuppHeatingCoilFlag) {
                            PlantUtilities::SetComponentFlowRate(state,
                                                                 mdot,
                                                                 state.dataFurnaces->Furnace(FurnaceNum).CoilControlNode,
                                                                 state.dataFurnaces->Furnace(FurnaceNum).CoilOutletNode,
                                                                 state.dataFurnaces->Furnace(FurnaceNum).plantLoc);
                            WaterCoils::SimulateWaterCoilComponents(state,
                                                                    state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilName,
                                                                    FirstHVACIteration,
                                                                    state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex,
                                                                    QCoilActual,
                                                                    state.dataFurnaces->Furnace(FurnaceNum).OpMode);
                        } else {
                            // supplemental coil
                            PlantUtilities::SetComponentFlowRate(state,
                                                                 mdot,
                                                                 state.dataFurnaces->Furnace(FurnaceNum).SuppCoilControlNode,
                                                                 state.dataFurnaces->Furnace(FurnaceNum).SuppCoilOutletNode,
                                                                 state.dataFurnaces->Furnace(FurnaceNum).SuppPlantLoc);
                            // simulate the hot water supplemental heating coil
                            WaterCoils::SimulateWaterCoilComponents(state,
                                                                    state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilName,
                                                                    FirstHVACIteration,
                                                                    state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex,
                                                                    QCoilActual,
                                                                    state.dataFurnaces->Furnace(FurnaceNum).OpMode);
                        }
                        return QCoilRequested != 0.0 ? (QCoilActual - QCoilRequested) / QCoilRequested : 0.0;
                    };
                    General::SolveRoot(state, ErrTolerance, SolveMaxIter, SolFlag, HotWaterMdot, f, MinWaterFlow, MaxHotWaterFlow);
                    if (SolFlag == -1) {
                        if (state.dataFurnaces->Furnace(FurnaceNum).HotWaterCoilMaxIterIndex == 0) {
                            ShowWarningMessage(state,
                                               format("CalcNonDXHeatingCoils: Hot water coil control failed for {}=\"{}\"",
                                                      cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                                      state.dataFurnaces->Furnace(FurnaceNum).Name));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state, format("  Iteration limit [{}] exceeded in calculating hot water mass flow rate", SolveMaxIter));
                        }
                        ShowRecurringWarningErrorAtEnd(
                            state,
                            format("CalcNonDXHeatingCoils: Hot water coil control failed (iteration limit [{}]) for {}=\"{}",
                                   SolveMaxIter,
                                   cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                   state.dataFurnaces->Furnace(FurnaceNum).Name),
                            state.dataFurnaces->Furnace(FurnaceNum).HotWaterCoilMaxIterIndex);
                    } else if (SolFlag == -2) {
                        if (state.dataFurnaces->Furnace(FurnaceNum).HotWaterCoilMaxIterIndex2 == 0) {
                            ShowWarningMessage(state,
                                               format("CalcNonDXHeatingCoils: Hot water coil control failed (maximum flow limits) for {}=\"{}\"",
                                                      cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num),
                                                      state.dataFurnaces->Furnace(FurnaceNum).Name));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state, "...Bad hot water maximum flow rate limits");
                            ShowContinueError(state, format("...Given minimum water flow rate={:.3R} kg/s", MinWaterFlow));
                            ShowContinueError(state, format("...Given maximum water flow rate={:.3R} kg/s", MaxHotWaterFlow));
                        }
                        ShowRecurringWarningErrorAtEnd(state,
                                                       "CalcNonDXHeatingCoils: Hot water coil control failed (flow limits) for " +
                                                           cFurnaceTypes(state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num) + "=\"" +
                                                           state.dataFurnaces->Furnace(FurnaceNum).Name + "\"",
                                                       state.dataFurnaces->Furnace(FurnaceNum).HotWaterCoilMaxIterIndex2,
                                                       MaxHotWaterFlow,
                                                       MinWaterFlow,
                                                       _,
                                                       "[kg/s]",
                                                       "[kg/s]");
                    }
                }
            } else {
                mdot = 0.0;
                SetComponentFlowRate(state, mdot, CoilControlNode, CoilOutletNode, plantLoc);
            }
            // simulate the hot water heating coil
            SimulateWaterCoilComponents(state, HeatingCoilName, FirstHVACIteration, HeatingCoilIndex, QActual, FanMode);
        } break;
        case Coil_HeatingSteam: {
            if (QCoilLoad > SmallLoad) {
                SetComponentFlowRate(state, MaxHotWaterFlow, CoilControlNode, CoilOutletNode, plantLoc);
                // simulate the steam heating coil
                SimulateSteamCoilComponents(state, HeatingCoilName, FirstHVACIteration, HeatingCoilIndex, QCoilLoad, QActual, FanMode);
            } else {
                mdot = 0.0;
                SetComponentFlowRate(state, mdot, CoilControlNode, CoilOutletNode, plantLoc);
                // simulate the steam heating coil
                SimulateSteamCoilComponents(state, HeatingCoilName, FirstHVACIteration, HeatingCoilIndex, QCoilLoad, QActual, FanMode);
            }
        } break;
        default:
            break;
        }

        HeatCoilLoadmet = QActual;
    }

    //        End of Reporting subroutines for the Furnace Module

    //******************************************************************************

    void SimVariableSpeedHP(EnergyPlusData &state,
                            int const FurnaceNum,          // number of the current engine driven Heat Pump being simulated
                            bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                            int const AirLoopNum,          // index to air loop
                            Real64 const QZnReq,           // required zone load
                            Real64 const QLatReq,          // required latent load
                            Real64 &OnOffAirFlowRatio      // ratio of compressor ON airflow to AVERAGE airflow over timestep
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Bo Shen, based on HVACMultiSpeedHeatPump:CalcMSHeatPump
        //       DATE WRITTEN   March, 2012

        // PURPOSE OF THIS SUBROUTINE:
        // Simulate a multispeed heat pump; adjust its output to match the
        // required system load.

        // METHODOLOGY EMPLOYED:
        // Calls ControlMSHPOutput to obtain the desired unit output

        using namespace DataZoneEnergyDemands;
        using DataHVACGlobals::SmallLoad;
        using DataHVACGlobals::SmallMassFlow;
        using IntegratedHeatPump::DecideWorkMode;

        Real64 PartLoadFrac; // compressor part load fraction
        Real64 SpeedRatio;   // compressor speed ratio
        bool UnitOn;         // TRUE if unit is on
        int OutletNode;      // MSHP air outlet node
        int InletNode;       // MSHP air inlet node
        Real64 AirMassFlow;  // air mass flow rate [kg/s]
        int OpMode;          // operating mode (fan cycling or continious; DX coil always cycles)
        int ZoneNum;         // Controlled zone number
        Real64 QTotUnitOut;  // capacity output
        auto &SpeedNum = state.dataFurnaces->SpeedNum;
        auto &SupHeaterLoad = state.dataFurnaces->SupHeaterLoad;
        Real64 TotalZoneLatentLoad;       // Total ZONE latent load
        Real64 TotalZoneSensibleLoad;     // Total ZONE sensible load
        Real64 SystemSensibleLoad;        // Positive value means heating required
        CompressorOperation CompressorOp; // compressor operation; 1=on, 0=off
        Real64 SaveMassFlowRate;          // saved inlet air mass flow rate [kg/s]
        Real64 QSensUnitOut;              // sensible capacity output
        Real64 QLatUnitOut;               // latent capacity output
        Real64 ActualSensibleOutput;      // Actual furnace sensible capacity
        Real64 ReheatCoilLoad;            // reheat coil load due to dehumidification
        Real64 QToHeatSetPt;              // Load required to meet heating setpoint temp (>0 is a heating load)
        Real64 NoCompOutput;              // output when no active compressor [W]
        int TotBranchNum;                 // total exit branch number
        int ZoneSideNodeNum;              // zone equip supply node
        bool EconoActive;                 // TRUE if Economizer is active

        // to be removed by furnace/unitary system

        // zero DX coils, and supplemental electric heater electricity consumption
        state.dataHVACGlobal->DXElecHeatingPower = 0.0;
        state.dataHVACGlobal->DXElecCoolingPower = 0.0;
        state.dataFurnaces->SaveCompressorPLR = 0.0;
        state.dataHVACGlobal->ElecHeatingCoilPower = 0.0;
        state.dataHVACGlobal->SuppHeatingCoilPower = 0.0;
        state.dataHVACGlobal->DefrostElecPower = 0.0;

        SystemSensibleLoad = QZnReq;
        TotalZoneSensibleLoad = QZnReq;
        TotalZoneLatentLoad = QLatReq;

        // initialize local variables
        UnitOn = true;
        OutletNode = state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum;
        InletNode = state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum;
        AirMassFlow = state.dataFurnaces->Furnace(FurnaceNum).DesignMassFlowRate;
        OpMode = state.dataFurnaces->Furnace(FurnaceNum).OpMode;
        ZoneNum = state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum;
        CompressorOp = CompressorOperation::On;

        // Set latent load for heating
        if (state.dataFurnaces->HeatingLoad) {
            state.dataFurnaces->Furnace(FurnaceNum).HeatCoolMode = Furnaces::ModeOfOperation::HeatingMode;
            // Set latent load for cooling and no sensible load condition
        } else if (state.dataFurnaces->CoolingLoad) {
            state.dataFurnaces->Furnace(FurnaceNum).HeatCoolMode = Furnaces::ModeOfOperation::CoolingMode;
        } else {
            state.dataFurnaces->Furnace(FurnaceNum).HeatCoolMode = Furnaces::ModeOfOperation::NoCoolHeat;
        }

        // set the on/off flags
        if (state.dataFurnaces->Furnace(FurnaceNum).OpMode == CycFanCycCoil) {
            // cycling unit only runs if there is a cooling or heating load.
            if (std::abs(QZnReq) < SmallLoad || AirMassFlow < SmallMassFlow || state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) {
                UnitOn = false;
            }
        } else if (state.dataFurnaces->Furnace(FurnaceNum).OpMode == ContFanCycCoil) {
            // continuous unit: fan runs if scheduled on; coil runs only if there is a cooling or heating load
            if (AirMassFlow < SmallMassFlow) {
                UnitOn = false;
            }
        }

        state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;

        if (AirLoopNum != 0) {
            EconoActive = state.dataAirLoop->AirLoopControlInfo(AirLoopNum).EconoActive;
        } else {
            EconoActive = false;
        }

        SaveMassFlowRate = state.dataLoopNodes->Node(InletNode).MassFlowRate;
        // decide current working mode for IHP
        if ((FirstHVACIteration) && (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP))
            DecideWorkMode(state, state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex, TotalZoneSensibleLoad, TotalZoneLatentLoad);

        if (!FirstHVACIteration && state.dataFurnaces->Furnace(FurnaceNum).OpMode == CycFanCycCoil &&
            (QZnReq < (-1.0 * SmallLoad) || TotalZoneLatentLoad < (-1.0 * SmallLoad)) && EconoActive) {
            // for cycling fan, cooling load, check whether furnace can meet load with compressor off
            CompressorOp = CompressorOperation::Off;
            ControlVSHPOutput(state,
                              FurnaceNum,
                              FirstHVACIteration,
                              CompressorOp,
                              OpMode,
                              TotalZoneSensibleLoad,
                              TotalZoneLatentLoad,
                              ZoneNum,
                              SpeedNum,
                              SpeedRatio,
                              PartLoadFrac,
                              OnOffAirFlowRatio,
                              SupHeaterLoad);

            TotalZoneSensibleLoad = QZnReq;
            TotalZoneLatentLoad = QLatReq;

            if (SpeedNum == state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling && SpeedRatio == 1.0) {
                // compressor on (reset inlet air mass flow rate to starting value)
                state.dataLoopNodes->Node(InletNode).MassFlowRate = SaveMassFlowRate;
                CompressorOp = CompressorOperation::On;
                ControlVSHPOutput(state,
                                  FurnaceNum,
                                  FirstHVACIteration,
                                  CompressorOp,
                                  OpMode,
                                  TotalZoneSensibleLoad,
                                  TotalZoneLatentLoad,
                                  ZoneNum,
                                  SpeedNum,
                                  SpeedRatio,
                                  PartLoadFrac,
                                  OnOffAirFlowRatio,
                                  SupHeaterLoad);
            }
        } else {
            // compressor on
            CompressorOp = CompressorOperation::On;

            //     if ( QZnReq < -1000.0 .AND. FurnaceNum == 1 ) then
            //       CompressorOp      = On
            //     end if
            ControlVSHPOutput(state,
                              FurnaceNum,
                              FirstHVACIteration,
                              CompressorOp,
                              OpMode,
                              TotalZoneSensibleLoad,
                              TotalZoneLatentLoad,
                              ZoneNum,
                              SpeedNum,
                              SpeedRatio,
                              PartLoadFrac,
                              OnOffAirFlowRatio,
                              SupHeaterLoad);
        }

        if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatCool) {
            state.dataFurnaces->SaveCompressorPLR = PartLoadFrac;
        } else {
            if (SpeedNum > 1) {
                state.dataFurnaces->SaveCompressorPLR = 1.0;
            }

            if (PartLoadFrac == 1.0 && state.dataFurnaces->SaveCompressorPLR < 1.0) {
                PartLoadFrac = state.dataFurnaces->SaveCompressorPLR;
            }
        }

        ReheatCoilLoad = 0.0;
        TotalZoneSensibleLoad = QZnReq;
        TotalZoneLatentLoad = QLatReq;
        //     Calculate the reheat coil output
        if ((GetCurrentScheduleValue(state, state.dataFurnaces->Furnace(FurnaceNum).SchedPtr) > 0.0) &&
            (state.dataFurnaces->Furnace(FurnaceNum).Humidistat &&
             state.dataFurnaces->Furnace(FurnaceNum).DehumidControlType_Num == DehumidificationControlMode::CoolReheat &&
             (QLatReq < 0.0))) { // if a Humidistat is installed and dehumdification control type is CoolReheat
            CalcVarSpeedHeatPump(state,
                                 FurnaceNum,
                                 FirstHVACIteration,
                                 CompressorOp,
                                 SpeedNum,
                                 SpeedRatio,
                                 PartLoadFrac,
                                 ActualSensibleOutput,
                                 QLatUnitOut,
                                 TotalZoneSensibleLoad,
                                 TotalZoneLatentLoad,
                                 OnOffAirFlowRatio,
                                 ReheatCoilLoad);
            if (state.dataFurnaces->Furnace(FurnaceNum).ZoneSequenceHeatingNum > 0) {
                QToHeatSetPt = (state.dataZoneEnergyDemand->ZoneSysEnergyDemand(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                                    .SequencedOutputRequiredToHeatingSP(state.dataFurnaces->Furnace(FurnaceNum).ZoneSequenceHeatingNum) /
                                state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac);
            } else {
                QToHeatSetPt = (state.dataZoneEnergyDemand->ZoneSysEnergyDemand(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum)
                                    .OutputRequiredToHeatingSP /
                                state.dataFurnaces->Furnace(FurnaceNum).ControlZoneMassFlowFrac);
            }
            //       Cooling mode or floating condition and dehumidification is required
            if (QToHeatSetPt < 0.0) {
                //         Calculate the reheat coil load wrt the heating setpoint temperature. Reheat coil picks up
                //         the entire excess sensible cooling (DX cooling coil and impact of outdoor air).
                ReheatCoilLoad = max(0.0, (QToHeatSetPt - ActualSensibleOutput));
                state.dataFurnaces->Furnace(FurnaceNum).DehumidInducedHeatingDemandRate = ReheatCoilLoad;
                //       Heating mode and dehumidification is required
            } else if (QToHeatSetPt >= 0.0) {
                ReheatCoilLoad = max(QToHeatSetPt, QToHeatSetPt - ActualSensibleOutput);
                state.dataFurnaces->Furnace(FurnaceNum).DehumidInducedHeatingDemandRate = max(0.0, ActualSensibleOutput * (-1.0));
            } else {
                ReheatCoilLoad = 0.0;
            }

            SupHeaterLoad = 0.0;
            CalcVarSpeedHeatPump(state,
                                 FurnaceNum,
                                 FirstHVACIteration,
                                 CompressorOp,
                                 1,
                                 0.0,
                                 0.0,
                                 NoCompOutput,
                                 QLatUnitOut,
                                 0.0,
                                 0.0,
                                 OnOffAirFlowRatio,
                                 SupHeaterLoad);

            if (NoCompOutput > SystemSensibleLoad && SystemSensibleLoad > 0.0 && ReheatCoilLoad > 0.0) {
                // Reduce reheat coil load if you are controlling high humidity but outside air
                // and/or the supply air fan is providing enough heat to meet the system sensible load.
                // This will bring the zone temp closer to the heating setpoint temp.
                ReheatCoilLoad = max(0.0, ReheatCoilLoad - (NoCompOutput - SystemSensibleLoad));
            }
        } else {
            //       No humidistat installed
            ReheatCoilLoad = 0.0;
        }

        TotalZoneSensibleLoad = QZnReq;
        TotalZoneLatentLoad = QLatReq;
        if (ReheatCoilLoad > 0.0) {
            CalcVarSpeedHeatPump(state,
                                 FurnaceNum,
                                 FirstHVACIteration,
                                 CompressorOp,
                                 SpeedNum,
                                 SpeedRatio,
                                 PartLoadFrac,
                                 QSensUnitOut,
                                 QLatUnitOut,
                                 TotalZoneSensibleLoad,
                                 TotalZoneLatentLoad,
                                 OnOffAirFlowRatio,
                                 ReheatCoilLoad);
        } else {
            CalcVarSpeedHeatPump(state,
                                 FurnaceNum,
                                 FirstHVACIteration,
                                 CompressorOp,
                                 SpeedNum,
                                 SpeedRatio,
                                 PartLoadFrac,
                                 QSensUnitOut,
                                 QLatUnitOut,
                                 TotalZoneSensibleLoad,
                                 TotalZoneLatentLoad,
                                 OnOffAirFlowRatio,
                                 SupHeaterLoad);
        }

        // calculate delivered capacity
        AirMassFlow = state.dataLoopNodes->Node(InletNode).MassFlowRate;

        state.dataFurnaces->Furnace(FurnaceNum).MdotFurnace = AirMassFlow;

        QTotUnitOut = AirMassFlow * (state.dataLoopNodes->Node(OutletNode).Enthalpy -
                                     state.dataLoopNodes->Node(state.dataFurnaces->Furnace(FurnaceNum).NodeNumOfControlledZone).Enthalpy);

        state.dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = AirMassFlow;
        state.dataLoopNodes->Node(OutletNode).MassFlowRateMaxAvail = AirMassFlow;

        if (!FirstHVACIteration && AirMassFlow > 0.0 && AirLoopNum > 0) {
            TotBranchNum = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).NumOutletBranches;
            if (TotBranchNum == 1) {
                ZoneSideNodeNum = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).ZoneEquipSupplyNodeNum(1);
                // THE MASS FLOW PRECISION of the system solver is not enough for some small air flow rate iterations , BY DEBUGGING
                // it may cause mass flow rate occilations between airloop and zoneequip
                // specify the air flow rate directly for one-to-one system, when the iteration deviation is closing the solver precision level
                // 0.02 is 2 * HVACFlowRateToler, in order to accomodate the system solver precision level
                if (std::abs(AirMassFlow - state.dataLoopNodes->Node(ZoneSideNodeNum).MassFlowRate) < 0.02)
                    state.dataLoopNodes->Node(ZoneSideNodeNum).MassFlowRateMaxAvail = AirMassFlow;
                state.dataLoopNodes->Node(ZoneSideNodeNum).MassFlowRate = AirMassFlow;
            }

            // the below might be useful if more divergences occur
            // Node(PrimaryAirSystem(AirLoopNumber)%Branch(1)%NodeNumIn)%MassFlowRateMaxAvail = AirMassFlow
            // Node(PrimaryAirSystem(AirLoopNumber)%Branch(1)%NodeNumIn)%MassFlowRate = AirMassFlow
        }

        // report variables
        state.dataFurnaces->Furnace(FurnaceNum).DehumidInducedHeatingDemandRate = ReheatCoilLoad;
        if (QZnReq > SmallLoad) { // HEATING LOAD
            state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand = 0.0;
            state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand = QZnReq;
        } else {
            state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilSensDemand = std::abs(QZnReq);
            state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilSensDemand = 0.0;
        }

        state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = state.dataFurnaces->SaveCompressorPLR;
        if (state.dataFurnaces->Furnace(FurnaceNum).OpMode == CycFanCycCoil) {
            if (SupHeaterLoad > 0.0) {
                state.dataFurnaces->Furnace(FurnaceNum).FanPartLoadRatio = 1.0;
            } else {
                if (SpeedNum < 2) {
                    state.dataFurnaces->Furnace(FurnaceNum).FanPartLoadRatio = PartLoadFrac;
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).FanPartLoadRatio = 1.0;
                }
            }
        } else {
            if (UnitOn) {
                state.dataFurnaces->Furnace(FurnaceNum).FanPartLoadRatio = 1.0;
            } else {
                if (SpeedNum < 2) {
                    state.dataFurnaces->Furnace(FurnaceNum).FanPartLoadRatio = PartLoadFrac;
                } else {
                    state.dataFurnaces->Furnace(FurnaceNum).FanPartLoadRatio = 1.0;
                }
            }
        }
    }

    //******************************************************************************

    void ControlVSHPOutput(EnergyPlusData &state,
                           int const FurnaceNum,                   // Unit index of engine driven heat pump
                           bool const FirstHVACIteration,          // flag for 1st HVAC iteration in the time step
                           CompressorOperation const CompressorOp, // compressor operation; 1=on, 0=off
                           int const OpMode,                       // operating mode: CycFanCycCoil | ContFanCycCoil
                           Real64 &QZnReq,                         // cooling or heating output needed by zone [W]
                           Real64 &QLatReq,                        // latent cooling output needed by zone [W]
                           int const ZoneNum,                      // Index to zone number
                           int &SpeedNum,                          // Speed number
                           Real64 &SpeedRatio,                     // unit speed ratio for DX coils
                           Real64 &PartLoadFrac,                   // unit part load fraction
                           Real64 &OnOffAirFlowRatio,              // ratio of compressor ON airflow to AVERAGE airflow over timestep
                           Real64 &SupHeaterLoad                   // Supplemental heater load [W]
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Bo Shen, based on HVACMultiSpeedHeatPump:ControlMSHPOutput
        //       DATE WRITTEN   March,  2012

        // PURPOSE OF THIS SUBROUTINE:
        // Determine the part load fraction at low speed, and speed ratio at high speed for this time step.

        // METHODOLOGY EMPLOYED:
        // Use RegulaFalsi technique to iterate on part-load ratio until convergence is achieved.

        // Using/Aliasing
        using IntegratedHeatPump::GetCurWorkMode;
        using IntegratedHeatPump::GetMaxSpeedNumIHP;
        using IntegratedHeatPump::IHPOperationMode;
        using Psychrometrics::PsyCpAirFnW;

        // SUBROUTINE PARAMETER DEFINITIONS:
        int constexpr MaxIte(500); // maximum number of iterations

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 FullOutput;          // unit full output when compressor is operating [W]
        Real64 LowOutput;           // unit full output at low speed [W]
        Real64 TempOutput;          // unit output when iteration limit exceeded [W]
        Real64 NoCompOutput;        // output when no active compressor [W]
        Real64 LatOutput;           // latent capacity output
        Real64 ErrorToler;          // error tolerance
        int SolFla;                 // Flag of RegulaFalsi solver
        std::array<Real64, 10> Par; // Parameters passed to RegulaFalsi
        Real64 QCoilActual;         // coil load actually delivered returned to calling component
        int i;                      // Speed index
        int ErrCountCyc(0);         // Counter used to minimize the occurrence of output warnings
        int ErrCountVar(0);         // Counter used to minimize the occurrence of output warnings
        IHPOperationMode IHPMode(IHPOperationMode::Idle);

        SupHeaterLoad = 0.0;
        PartLoadFrac = 0.0;
        SpeedRatio = 0.0;
        SpeedNum = 1;
        LatOutput = 0.0;
        Real64 noLatOutput = 0.0;
        ErrorToler = 0.001; // Error tolerance for convergence from input deck

        if (GetCurrentScheduleValue(state, state.dataFurnaces->Furnace(FurnaceNum).SchedPtr) == 0.0) return;

        // Get result when DX coil is off
        SupHeaterLoad = 0.0;
        CalcVarSpeedHeatPump(state,
                             FurnaceNum,
                             FirstHVACIteration,
                             CompressorOp,
                             SpeedNum,
                             SpeedRatio,
                             PartLoadFrac,
                             NoCompOutput,
                             noLatOutput,
                             0.0,
                             0.0,
                             OnOffAirFlowRatio,
                             SupHeaterLoad);

        if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
            IHPMode = GetCurWorkMode(state, state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex);
            if ((IHPOperationMode::DedicatedWaterHtg == IHPMode) || (IHPOperationMode::SCWHMatchWH == IHPMode)) { // cooling capacity is a resultant
                return;
            }
        }

        // If cooling and NoCompOutput < QZnReq, the coil needs to be off
        // If heating and NoCompOutput > QZnReq, the coil needs to be off
        // If no cooling or heating and no latent load, the coil needs to be off
        if (QZnReq < -SmallLoad) {
            if (NoCompOutput < QZnReq && QLatReq >= -SmallLoad) return;
        } else if (QZnReq > SmallLoad) {
            if (NoCompOutput > QZnReq && QLatReq >= -SmallLoad) return;
            if (QLatReq <= -SmallLoad) QZnReq = 0.0; // Zero heating load to allow dehumidification
        } else {
            if (QLatReq >= -SmallLoad) return;
        }

        // Get full load result
        PartLoadFrac = 1.0;
        SpeedRatio = 1.0;
        if (state.dataFurnaces->Furnace(FurnaceNum).HeatCoolMode == Furnaces::ModeOfOperation::HeatingMode) {
            SpeedNum = state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedHeating;
        } else if (state.dataFurnaces->Furnace(FurnaceNum).HeatCoolMode == Furnaces::ModeOfOperation::CoolingMode) {
            SpeedNum = state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling;
        } else if (QLatReq < -SmallLoad) {
            SpeedNum = state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling;
        } else {
            SpeedNum = 1;
            PartLoadFrac = 0.0;
        }

        if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP)
            SpeedNum = GetMaxSpeedNumIHP(state, state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex);

        CalcVarSpeedHeatPump(state,
                             FurnaceNum,
                             FirstHVACIteration,
                             CompressorOp,
                             SpeedNum,
                             SpeedRatio,
                             PartLoadFrac,
                             FullOutput,
                             LatOutput,
                             QZnReq,
                             QLatReq,
                             OnOffAirFlowRatio,
                             SupHeaterLoad);

        if (QLatReq < (-1.0 * SmallLoad)) { // dehumidification mode
            if (QLatReq <= LatOutput || (QZnReq < -SmallLoad && QZnReq <= FullOutput) || (QZnReq > SmallLoad && QZnReq >= FullOutput)) {
                PartLoadFrac = 1.0;
                SpeedRatio = 1.0;
                state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = PartLoadFrac;
                state.dataFurnaces->Furnace(FurnaceNum).CompSpeedRatio = SpeedRatio;
                state.dataFurnaces->Furnace(FurnaceNum).CompSpeedNum = SpeedNum;
                return;
            }
            ErrorToler = 0.001; // Error tolerance for convergence from input deck
        } else if (QZnReq < (-1.0 * SmallLoad)) {
            if (QZnReq <= FullOutput) {
                PartLoadFrac = 1.0;
                SpeedRatio = 1.0;
                state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = PartLoadFrac;
                state.dataFurnaces->Furnace(FurnaceNum).CompSpeedRatio = SpeedRatio;
                state.dataFurnaces->Furnace(FurnaceNum).CompSpeedNum = SpeedNum;
                return;
            }
            ErrorToler = 0.001; // Error tolerance for convergence from input deck
        } else {
            if (QZnReq >= FullOutput) {
                PartLoadFrac = 1.0;
                SpeedRatio = 1.0;
                // may need supplemental heating so don't return in heating mode
            }
            ErrorToler = 0.001; // Error tolerance for convergence from input deck
        }

        if ((QZnReq < -SmallLoad && NoCompOutput - QZnReq > SmallLoad) || (QZnReq > SmallLoad && QZnReq - NoCompOutput > SmallLoad)) {
            if ((QZnReq > SmallLoad && QZnReq < FullOutput) || (QZnReq < (-1.0 * SmallLoad) && QZnReq > FullOutput)) {

                Par[0] = FurnaceNum;
                Par[1] = ZoneNum;
                if (FirstHVACIteration) {
                    Par[2] = 1.0;
                } else {
                    Par[2] = 0.0;
                }
                Par[3] = OpMode;
                Par[4] = QZnReq;
                Par[5] = OnOffAirFlowRatio;
                Par[6] = SupHeaterLoad;
                Par[8] = static_cast<int>(CompressorOp);
                Par[9] = 1.0;
                // Check whether the low speed coil can meet the load or not
                CalcVarSpeedHeatPump(state,
                                     FurnaceNum,
                                     FirstHVACIteration,
                                     CompressorOp,
                                     1,
                                     0.0,
                                     1.0,
                                     LowOutput,
                                     LatOutput,
                                     QZnReq,
                                     QLatReq,
                                     OnOffAirFlowRatio,
                                     SupHeaterLoad);
                if ((QZnReq > SmallLoad && QZnReq <= LowOutput) || (QZnReq < (-1.0 * SmallLoad) && QZnReq >= LowOutput)) {
                    // Calculate the part load fraction
                    SpeedRatio = 0.0;
                    SpeedNum = 1;
                    auto f = // (AUTO_OK_LAMBDA)
                        [&state, FurnaceNum, FirstHVACIteration, QZnReq, OnOffAirFlowRatio, SupHeaterLoad, CompressorOp](Real64 const PartLoadFrac) {
                            return VSHPCyclingResidual(
                                state, PartLoadFrac, FurnaceNum, FirstHVACIteration, QZnReq, OnOffAirFlowRatio, SupHeaterLoad, CompressorOp, 1.0);
                        };
                    General::SolveRoot(state, ErrorToler, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                    if (SolFla == -1) {
                        if (!state.dataGlobal->WarmupFlag) {
                            if (ErrCountCyc == 0) {
                                ++ErrCountCyc;
                                ShowWarningError(state,
                                                 format("Iteration limit exceeded calculating VS WSHP unit cycling ratio, for unit={}",
                                                        state.dataFurnaces->Furnace(FurnaceNum).Name));
                                ShowContinueErrorTimeStamp(state, format("Cycling ratio returned={:.2R}", PartLoadFrac));
                            } else {
                                ++ErrCountCyc;
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    state.dataFurnaces->Furnace(FurnaceNum).Name +
                                        "\": Iteration limit warning exceeding calculating DX unit cycling ratio  continues...",
                                    state.dataFurnaces->Furnace(FurnaceNum).ErrIndexCyc,
                                    PartLoadFrac,
                                    PartLoadFrac);
                            }
                        }
                    } else if (SolFla == -2) {
                        ShowFatalError(state,
                                       format("VS WSHP unit cycling ratio calculation failed: cycling limits exceeded, for unit={}",
                                              state.dataFurnaces->Furnace(FurnaceNum).Name));
                    }
                } else {
                    // Check to see which speed to meet the load
                    PartLoadFrac = 1.0;
                    SpeedRatio = 1.0;
                    if (QZnReq < (-1.0 * SmallLoad)) { // Cooling
                        for (i = 2; i <= state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling; ++i) {
                            CalcVarSpeedHeatPump(state,
                                                 FurnaceNum,
                                                 FirstHVACIteration,
                                                 CompressorOp,
                                                 i,
                                                 SpeedRatio,
                                                 PartLoadFrac,
                                                 TempOutput,
                                                 LatOutput,
                                                 QZnReq,
                                                 QLatReq,
                                                 OnOffAirFlowRatio,
                                                 SupHeaterLoad);

                            if (QZnReq >= TempOutput) {
                                SpeedNum = i;
                                break;
                            }
                        }
                    } else {
                        for (i = 2; i <= state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedHeating; ++i) {
                            CalcVarSpeedHeatPump(state,
                                                 FurnaceNum,
                                                 FirstHVACIteration,
                                                 CompressorOp,
                                                 i,
                                                 SpeedRatio,
                                                 PartLoadFrac,
                                                 TempOutput,
                                                 LatOutput,
                                                 QZnReq,
                                                 QLatReq,
                                                 OnOffAirFlowRatio,
                                                 SupHeaterLoad);
                            if (QZnReq <= TempOutput) {
                                SpeedNum = i;
                                break;
                            }
                        }
                    }
                    Par[7] = SpeedNum;
                    auto f = [&state, FurnaceNum, FirstHVACIteration, QZnReq, OnOffAirFlowRatio, SupHeaterLoad, SpeedNum, CompressorOp](
                                 Real64 const SpeedRatio) {
                        return VSHPSpeedResidual(
                            state, SpeedRatio, FurnaceNum, FirstHVACIteration, QZnReq, OnOffAirFlowRatio, SupHeaterLoad, SpeedNum, CompressorOp, 1.0);
                    };
                    General::SolveRoot(state, ErrorToler, MaxIte, SolFla, SpeedRatio, f, 1.0e-10, 1.0);
                    if (SolFla == -1) {
                        if (!state.dataGlobal->WarmupFlag) {
                            if (ErrCountVar == 0) {
                                ++ErrCountVar;
                                ShowWarningError(state,
                                                 format("Iteration limit exceeded calculating VS WSHP unit speed ratio, for unit={}",
                                                        state.dataFurnaces->Furnace(FurnaceNum).Name));
                                ShowContinueErrorTimeStamp(state, format("Speed ratio returned=[{:.2R}], Speed number ={}", SpeedRatio, SpeedNum));
                            } else {
                                ++ErrCountVar;
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    state.dataFurnaces->Furnace(FurnaceNum).Name +
                                        "\": Iteration limit warning exceeding calculating DX unit speed ratio continues...",
                                    state.dataFurnaces->Furnace(FurnaceNum).ErrIndexVar,
                                    SpeedRatio,
                                    SpeedRatio);
                            }
                        }
                    } else if (SolFla == -2) {
                        ShowFatalError(state,
                                       format("VS WSHP unit compressor speed calculation failed: speed limits exceeded, for unit={}",
                                              state.dataFurnaces->Furnace(FurnaceNum).Name));
                    }
                }
            } else {
                LatOutput = noLatOutput; // reset full output if not needed for sensible load
                SpeedNum = 1;            // reset speed from full output test
            }
        } else {
            LatOutput = noLatOutput; // reset full output if not needed for sensible load
            SpeedNum = 1;            // reset speed from full output test
        }
        // meet the latent load
        if (QLatReq < -SmallLoad && QLatReq < LatOutput) {
            PartLoadFrac = 1.0;
            SpeedRatio = 1.0;
            for (i = SpeedNum; i <= state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling; ++i) {
                CalcVarSpeedHeatPump(state,
                                     FurnaceNum,
                                     FirstHVACIteration,
                                     CompressorOp,
                                     i,
                                     SpeedRatio,
                                     PartLoadFrac,
                                     TempOutput,
                                     LatOutput,
                                     QZnReq,
                                     QLatReq,
                                     OnOffAirFlowRatio,
                                     SupHeaterLoad);

                if (QLatReq > LatOutput) {
                    SpeedNum = i;
                    break;
                }
            }
            if (QLatReq - LatOutput > SmallLoad) {
                Par[0] = FurnaceNum;
                Par[1] = ZoneNum;
                if (FirstHVACIteration) {
                    Par[2] = 1.0;
                } else {
                    Par[2] = 0.0;
                }
                Par[3] = OpMode;
                Par[4] = QLatReq;
                Par[5] = OnOffAirFlowRatio;
                Par[6] = SupHeaterLoad;
                Par[7] = SpeedNum;
                Par[8] = static_cast<int>(CompressorOp);
                Par[9] = 0.0;
                if (SpeedNum < 2) {
                    auto f = // (AUTO_OK_LAMBDA)
                        [&state, FurnaceNum, FirstHVACIteration, QLatReq, OnOffAirFlowRatio, SupHeaterLoad, CompressorOp](Real64 const PartLoadFrac) {
                            return VSHPCyclingResidual(
                                state, PartLoadFrac, FurnaceNum, FirstHVACIteration, QLatReq, OnOffAirFlowRatio, SupHeaterLoad, CompressorOp, 0.0);
                        };
                    General::SolveRoot(state, ErrorToler, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                } else {
                    auto f = [&state, FurnaceNum, FirstHVACIteration, QLatReq, OnOffAirFlowRatio, SupHeaterLoad, SpeedNum, CompressorOp](
                                 Real64 const SpeedRatio) {
                        return VSHPSpeedResidual(state,
                                                 SpeedRatio,
                                                 FurnaceNum,
                                                 FirstHVACIteration,
                                                 QLatReq,
                                                 OnOffAirFlowRatio,
                                                 SupHeaterLoad,
                                                 SpeedNum,
                                                 CompressorOp,
                                                 0.0);
                    };
                    General::SolveRoot(state, ErrorToler, MaxIte, SolFla, SpeedRatio, f, 1.0e-10, 1.0);
                }
                if (SolFla == -1) {
                    if (!state.dataGlobal->WarmupFlag) {
                        if (ErrCountVar == 0) {
                            ++ErrCountVar;
                            ShowWarningError(state,
                                             format("Iteration limit exceeded calculating VS WSHP unit speed ratio, for unit={}",
                                                    state.dataFurnaces->Furnace(FurnaceNum).Name));
                            ShowContinueErrorTimeStamp(state, format("Speed ratio returned=[{:.2R}], Speed number ={}", SpeedRatio, SpeedNum));
                        } else {
                            ++ErrCountVar;
                            ShowRecurringWarningErrorAtEnd(state,
                                                           state.dataFurnaces->Furnace(FurnaceNum).Name +
                                                               "\": Iteration limit warning exceeding calculating DX unit speed ratio continues...",
                                                           state.dataFurnaces->Furnace(FurnaceNum).ErrIndexVar,
                                                           SpeedRatio,
                                                           SpeedRatio);
                        }
                    }
                } else if (SolFla == -2) {
                    ShowFatalError(state,
                                   format("VS WSHP unit compressor speed calculation failed: speed limits exceeded, for unit={}",
                                          state.dataFurnaces->Furnace(FurnaceNum).Name));
                }
            }
        }
        // end meet the latent load

        // if the heating coil cannot meet the load, trim with supplemental heater
        // occurs with constant fan mode when compressor is on or off
        // occurs with cycling fan mode when compressor PLR is equal to 1
        if ((QZnReq > SmallLoad && QZnReq > FullOutput) && (state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex != 0)) {
            PartLoadFrac = 1.0;
            SpeedRatio = 1.0;
            if (state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedHeating > 0)
                SpeedNum = state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedHeating; // maximum heating speed, avoid zero for cooling only mode

            if (state.dataEnvrn->OutDryBulbTemp <= state.dataFurnaces->Furnace(FurnaceNum).MaxOATSuppHeat) {
                SupHeaterLoad = QZnReq - FullOutput;
            } else {
                SupHeaterLoad = 0.0;
            }
            CalcVarSpeedHeatPump(state,
                                 FurnaceNum,
                                 FirstHVACIteration,
                                 CompressorOp,
                                 SpeedNum,
                                 SpeedRatio,
                                 PartLoadFrac,
                                 TempOutput,
                                 LatOutput,
                                 QZnReq,
                                 QLatReq,
                                 OnOffAirFlowRatio,
                                 SupHeaterLoad);
        }

        // check the outlet of the supplemental heater to be lower than the maximum supplemental heater supply air temperature
        if (state.dataLoopNodes->Node(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum).Temp >
                state.dataFurnaces->Furnace(FurnaceNum).DesignMaxOutletTemp &&
            SupHeaterLoad > 0.0) {

            //   If the supply air temperature is to high, turn off the supplemental heater to recalculate the outlet temperature
            CalcNonDXHeatingCoils(state, FurnaceNum, true, FirstHVACIteration, 0.0, OpMode, QCoilActual);
            //   If the outlet temperature is below the maximum supplemental heater supply air temperature, reduce the load passed to
            //   the supplemental heater, otherwise leave the supplemental heater off. If the supplemental heater is to be turned on,
            //   use the outlet conditions when the supplemental heater was off (CALL above) as the inlet conditions for the calculation
            //   of supplemental heater load to just meet the maximum supply air temperature from the supplemental heater.
            if (state.dataLoopNodes->Node(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum).Temp <
                state.dataFurnaces->Furnace(FurnaceNum).DesignMaxOutletTemp) {
                Real64 CpAir = PsyCpAirFnW(state.dataLoopNodes->Node(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum).HumRat);
                SupHeaterLoad = state.dataLoopNodes->Node(state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum).MassFlowRate * CpAir *
                                (state.dataFurnaces->Furnace(FurnaceNum).DesignMaxOutletTemp -
                                 state.dataLoopNodes->Node(state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum).Temp);

            } else {
                SupHeaterLoad = 0.0;
            }
        }

        // prepare module level output
        state.dataFurnaces->Furnace(FurnaceNum).CompPartLoadRatio = PartLoadFrac;
        state.dataFurnaces->Furnace(FurnaceNum).CompSpeedRatio = SpeedRatio;
        state.dataFurnaces->Furnace(FurnaceNum).CompSpeedNum = SpeedNum;
        state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilLatentDemand = std::abs(QLatReq);

        if (state.dataFurnaces->Furnace(FurnaceNum).OpMode == ContFanCycCoil) {
            state.dataFurnaces->Furnace(FurnaceNum).FanPartLoadRatio = 1.0;
        } else {
            state.dataFurnaces->Furnace(FurnaceNum).FanPartLoadRatio = PartLoadFrac;
        }
    }

    //******************************************************************************

    void CalcVarSpeedHeatPump(EnergyPlusData &state,
                              int const FurnaceNum,                   // Variable speed heat pump number
                              bool const FirstHVACIteration,          // Flag for 1st HVAC iteration
                              CompressorOperation const CompressorOp, // Compressor on/off; 1=on, 0=off
                              int const SpeedNum,                     // Speed number
                              Real64 const SpeedRatio,                // Compressor speed ratio
                              Real64 const PartLoadFrac,              // Compressor part load fraction
                              Real64 &SensibleLoadMet,                // Sensible cooling load met (furnace outlet with respect to control zone temp)
                              Real64 &LatentLoadMet,     // Latent cooling load met (furnace outlet with respect to control zone humidity ratio)
                              Real64 const QZnReq,       // Zone load (W)
                              Real64 const QLatReq,      // Zone latent load []
                              Real64 &OnOffAirFlowRatio, // Ratio of compressor ON airflow to AVERAGE airflow over timestep
                              Real64 &SupHeaterLoad      // supplemental heater load (W)
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Bo Shen, based on HVACMultiSpeedHeatPump:CalcMSHeatPump
        //       DATE WRITTEN:    March 2012

        // PURPOSE OF THIS SUBROUTINE:
        //  This routine will calcultes MSHP performance based on given system load

        // Using/Aliasing
        using Fans::SimulateFanComponents;
        using IntegratedHeatPump::SimIHP;
        using VariableSpeedCoils::SimVariableSpeedCoils;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        //  INTEGER, PARAMETER  ::   On  = 1           ! Compressor on flag
        //  INTEGER, PARAMETER  ::   Off = 2           ! Compressor off flag

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVMS TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int OutletNode;           // MSHP air outlet node
        int InletNode;            // MSHP air inlet node
        Real64 AirMassFlow;       // Air mass flow rate [kg/s]
        Real64 SavePartloadRatio; // part-load ratio
        Real64 SaveSpeedRatio;    // speed ratio
        Real64 QCoilActual;       // coil load actually delivered returned to calling component
        Real64 ErrorToler;        // supplemental heating coil convergence tollerance
        bool SuppHeatingCoilFlag; // whether to turn on the supplemental heater
        Real64 HeatCoilLoad;      // REQUIRED HEAT COIL LOAD

        InletNode = state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum;
        OutletNode = state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum;

        HeatCoilLoad = 0.0;
        state.dataFurnaces->SaveCompressorPLR = 0.0;
        SavePartloadRatio = 0.0;
        ErrorToler = 0.001;

        // Set inlet air mass flow rate based on PLR and compressor on/off air flow rates
        SetVSHPAirFlow(state, FurnaceNum, PartLoadFrac, OnOffAirFlowRatio, SpeedNum, SpeedRatio);

        if ((SupHeaterLoad > 1.0e-10) && (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatCool) &&
            (state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex == 0)) {
            // ONLY HEATING COIL, NO SUPPLEMENTAL COIL, USED FOR REHEAT DURING DUHMI
            HeatCoilLoad = state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity * PartLoadFrac; // REHEAT IN FAN ON TIME

            if (HeatCoilLoad > SupHeaterLoad) HeatCoilLoad = SupHeaterLoad; // HEATING COIL RUN TIME < FAN ON TIME

        } else if ((QZnReq > SmallLoad) && (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatCool)) {
            HeatCoilLoad = state.dataFurnaces->Furnace(FurnaceNum).DesignHeatingCapacity * PartLoadFrac;
        } else {
            HeatCoilLoad = 0.0;
        }

        AirMassFlow = state.dataLoopNodes->Node(InletNode).MassFlowRate;
        // if blow through, simulate fan then coils
        if (state.dataFurnaces->Furnace(FurnaceNum).FanPlace == BlowThru) {
            SimulateFanComponents(
                state, BlankString, FirstHVACIteration, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->FanSpeedRatio);

            if ((!state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilUpstream) &&
                (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatCool)) {
                // simulate furnace heating coil
                SuppHeatingCoilFlag = false; // if true simulates supplemental heating coil
                CalcNonDXHeatingCoils(state,
                                      FurnaceNum,
                                      SuppHeatingCoilFlag,
                                      FirstHVACIteration,
                                      HeatCoilLoad,
                                      state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                                      QCoilActual);
            }

            if ((QZnReq < (-1.0 * SmallLoad) || (QLatReq < (-1.0 * SmallLoad))) &&
                (state.dataEnvrn->OutDryBulbTemp >=
                 state.dataFurnaces->Furnace(FurnaceNum).MinOATCompressorCooling)) { // COOLING MODE or dehumidification mode

                if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                    SimIHP(state,
                           BlankString,
                           state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                           state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                           state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                           state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                           state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                           CompressorOp,
                           PartLoadFrac,
                           SpeedNum,
                           SpeedRatio,
                           QZnReq,
                           QLatReq,
                           false,
                           false,
                           OnOffAirFlowRatio);
                } else {
                    SimVariableSpeedCoils(state,
                                          BlankString,
                                          state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                          state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                                          state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                                          state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                                          state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                                          CompressorOp,
                                          PartLoadFrac,
                                          SpeedNum,
                                          SpeedRatio,
                                          QZnReq,
                                          QLatReq,
                                          OnOffAirFlowRatio);
                }

                SavePartloadRatio = PartLoadFrac;
                SaveSpeedRatio = SpeedRatio;

                state.dataFurnaces->SaveCompressorPLR =
                    state.dataVariableSpeedCoils->VarSpeedCoil(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).PartLoadRatio;
            } else {
                if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                    SimIHP(state,
                           BlankString,
                           state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                           state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                           state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                           state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                           state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                           CompressorOp,
                           PartLoadFrac,
                           SpeedNum,
                           SpeedRatio,
                           QZnReq,
                           QLatReq,
                           false,
                           false,
                           OnOffAirFlowRatio);
                } else {
                    SimVariableSpeedCoils(state,
                                          BlankString,
                                          state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                          state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                                          state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                                          state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                                          state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                                          CompressorOp,
                                          0.0,
                                          1,
                                          0.0,
                                          0.0,
                                          0.0,
                                          OnOffAirFlowRatio);
                }
            }

            if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num != UnitarySys_HeatCool) {
                if ((QZnReq > SmallLoad) && state.dataFurnaces->HeatingLoad) {
                    if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                        SimIHP(state,
                               BlankString,
                               state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex,
                               state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                               state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                               state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                               state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                               CompressorOp,
                               PartLoadFrac,
                               SpeedNum,
                               SpeedRatio,
                               QZnReq,
                               QLatReq,
                               false,
                               false,
                               OnOffAirFlowRatio);
                    } else {
                        SimVariableSpeedCoils(state,
                                              BlankString,
                                              state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex,
                                              state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                                              state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                                              state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                                              state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                                              CompressorOp,
                                              PartLoadFrac,
                                              SpeedNum,
                                              SpeedRatio,
                                              QZnReq,
                                              QLatReq,
                                              OnOffAirFlowRatio);
                    }

                    SavePartloadRatio = PartLoadFrac;
                    SaveSpeedRatio = SpeedRatio;

                    state.dataFurnaces->SaveCompressorPLR =
                        state.dataVariableSpeedCoils->VarSpeedCoil(state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex).PartLoadRatio;
                } else {
                    if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                        SimIHP(state,
                               BlankString,
                               state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                               state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                               state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                               state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                               state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                               CompressorOp,
                               PartLoadFrac,
                               SpeedNum,
                               SpeedRatio,
                               QZnReq,
                               QLatReq,
                               false,
                               false,
                               OnOffAirFlowRatio);
                    } else {
                        SimVariableSpeedCoils(state,
                                              BlankString,
                                              state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex,
                                              state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                                              state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                                              state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                                              state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                                              CompressorOp,
                                              0.0,
                                              1,
                                              0.0,
                                              0.0,
                                              0.0,
                                              OnOffAirFlowRatio);
                    }
                }
            } else if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilUpstream &&
                       (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatCool)) {
                // simulate furnace heating coil
                SuppHeatingCoilFlag = false; // if true simulates supplemental heating coil
                CalcNonDXHeatingCoils(state,
                                      FurnaceNum,
                                      SuppHeatingCoilFlag,
                                      FirstHVACIteration,
                                      HeatCoilLoad,
                                      state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                                      QCoilActual);
            }

            // Call twice to ensure the fan outlet conditions are updated
            SimulateFanComponents(
                state, BlankString, FirstHVACIteration, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->FanSpeedRatio);

            if ((!state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilUpstream) &&
                (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatCool)) {
                // simulate furnace heating coil
                SuppHeatingCoilFlag = false; // if true simulates supplemental heating coil
                CalcNonDXHeatingCoils(state,
                                      FurnaceNum,
                                      SuppHeatingCoilFlag,
                                      FirstHVACIteration,
                                      HeatCoilLoad,
                                      state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                                      QCoilActual);
            }

            if ((QZnReq < (-1.0 * SmallLoad) || (QLatReq < (-1.0 * SmallLoad))) &&
                (state.dataEnvrn->OutDryBulbTemp >= state.dataFurnaces->Furnace(FurnaceNum).MinOATCompressorCooling)) {

                if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                    SimIHP(state,
                           BlankString,
                           state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                           state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                           state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                           state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                           state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                           CompressorOp,
                           PartLoadFrac,
                           SpeedNum,
                           SpeedRatio,
                           QZnReq,
                           QLatReq,
                           false,
                           false,
                           OnOffAirFlowRatio);
                } else {
                    SimVariableSpeedCoils(state,
                                          BlankString,
                                          state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                          state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                                          state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                                          state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                                          state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                                          CompressorOp,
                                          PartLoadFrac,
                                          SpeedNum,
                                          SpeedRatio,
                                          QZnReq,
                                          QLatReq,
                                          OnOffAirFlowRatio);
                }

                SavePartloadRatio = PartLoadFrac;
                SaveSpeedRatio = SpeedRatio;
                state.dataFurnaces->SaveCompressorPLR =
                    state.dataVariableSpeedCoils->VarSpeedCoil(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).PartLoadRatio;
            } else {

                if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                    SimIHP(state,
                           BlankString,
                           state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                           state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                           state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                           state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                           state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                           CompressorOp,
                           PartLoadFrac,
                           SpeedNum,
                           SpeedRatio,
                           QZnReq,
                           QLatReq,
                           false,
                           false,
                           OnOffAirFlowRatio);
                } else {
                    SimVariableSpeedCoils(state,
                                          BlankString,
                                          state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                          state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                                          state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                                          state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                                          state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                                          CompressorOp,
                                          0.0,
                                          1,
                                          0.0,
                                          0.0,
                                          0.0,
                                          OnOffAirFlowRatio);
                }
            }

            if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num != UnitarySys_HeatCool) {
                if ((QZnReq > SmallLoad) && state.dataFurnaces->HeatingLoad) {
                    if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                        SimIHP(state,
                               BlankString,
                               state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex,
                               state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                               state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                               state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                               state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                               CompressorOp,
                               PartLoadFrac,
                               SpeedNum,
                               SpeedRatio,
                               QZnReq,
                               QLatReq,
                               false,
                               false,
                               OnOffAirFlowRatio);
                    } else {
                        SimVariableSpeedCoils(state,
                                              BlankString,
                                              state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex,
                                              state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                                              state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                                              state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                                              state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                                              CompressorOp,
                                              PartLoadFrac,
                                              SpeedNum,
                                              SpeedRatio,
                                              QZnReq,
                                              QLatReq,
                                              OnOffAirFlowRatio);
                    }

                    SavePartloadRatio = PartLoadFrac;
                    SaveSpeedRatio = SpeedRatio;
                    state.dataFurnaces->SaveCompressorPLR =
                        state.dataVariableSpeedCoils->VarSpeedCoil(state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex).PartLoadRatio;
                } else {
                    if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                        SimIHP(state,
                               BlankString,
                               state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                               state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                               state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                               state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                               state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                               CompressorOp,
                               PartLoadFrac,
                               SpeedNum,
                               SpeedRatio,
                               QZnReq,
                               QLatReq,
                               false,
                               false,
                               OnOffAirFlowRatio);
                    } else {
                        SimVariableSpeedCoils(state,
                                              BlankString,
                                              state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex,
                                              state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                                              state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                                              state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                                              state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                                              CompressorOp,
                                              0.0,
                                              1,
                                              0.0,
                                              0.0,
                                              0.0,
                                              OnOffAirFlowRatio);
                    }
                }
            } else if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilUpstream &&
                       (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatCool)) {
                // simulate furnace heating coil
                SuppHeatingCoilFlag = false; // if true simulates supplemental heating coil
                CalcNonDXHeatingCoils(state,
                                      FurnaceNum,
                                      SuppHeatingCoilFlag,
                                      FirstHVACIteration,
                                      HeatCoilLoad,
                                      state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                                      QCoilActual);
            }

            //  Simulate supplemental heating coil for blow through fan
            if (state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex > 0) {
                SuppHeatingCoilFlag = true; // if true simulates supplemental heating coil
                CalcNonDXHeatingCoils(state,
                                      FurnaceNum,
                                      SuppHeatingCoilFlag,
                                      FirstHVACIteration,
                                      SupHeaterLoad,
                                      state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                                      QCoilActual);
            }
        } else { // otherwise simulate DX coils then fan then supplemental heater

            if ((!state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilUpstream) &&
                (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatCool)) {
                // simulate furnace heating coil
                SuppHeatingCoilFlag = false; // if true simulates supplemental heating coil
                CalcNonDXHeatingCoils(state,
                                      FurnaceNum,
                                      SuppHeatingCoilFlag,
                                      FirstHVACIteration,
                                      HeatCoilLoad,
                                      state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                                      QCoilActual);
            }

            if ((QZnReq < (-1.0 * SmallLoad) || (QLatReq < (-1.0 * SmallLoad))) &&
                (state.dataEnvrn->OutDryBulbTemp >= state.dataFurnaces->Furnace(FurnaceNum).MinOATCompressorCooling)) {

                if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                    SimIHP(state,
                           BlankString,
                           state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                           state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                           state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                           state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                           state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                           CompressorOp,
                           PartLoadFrac,
                           SpeedNum,
                           SpeedRatio,
                           QZnReq,
                           QLatReq,
                           false,
                           false,
                           OnOffAirFlowRatio);
                } else {
                    SimVariableSpeedCoils(state,
                                          BlankString,
                                          state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                          state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                                          state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                                          state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                                          state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                                          CompressorOp,
                                          PartLoadFrac,
                                          SpeedNum,
                                          SpeedRatio,
                                          QZnReq,
                                          QLatReq,
                                          OnOffAirFlowRatio);
                }

                SavePartloadRatio = PartLoadFrac;
                SaveSpeedRatio = SpeedRatio;

                state.dataFurnaces->SaveCompressorPLR =
                    state.dataVariableSpeedCoils->VarSpeedCoil(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).PartLoadRatio;
            } else {
                if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                    SimIHP(state,
                           BlankString,
                           state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                           state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                           state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                           state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                           state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                           CompressorOp,
                           PartLoadFrac,
                           SpeedNum,
                           SpeedRatio,
                           QZnReq,
                           QLatReq,
                           false,
                           false,
                           OnOffAirFlowRatio);
                } else {
                    SimVariableSpeedCoils(state,
                                          BlankString,
                                          state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                          state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                                          state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                                          state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                                          state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                                          CompressorOp,
                                          0.0,
                                          1,
                                          0.0,
                                          0.0,
                                          0.0,
                                          OnOffAirFlowRatio);
                }
            }

            if (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num != UnitarySys_HeatCool) {
                if (QZnReq > SmallLoad && (state.dataEnvrn->OutDryBulbTemp >= state.dataFurnaces->Furnace(FurnaceNum).MinOATCompressorCooling)) {

                    if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                        SimIHP(state,
                               BlankString,
                               state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex,
                               state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                               state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                               state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                               state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                               CompressorOp,
                               PartLoadFrac,
                               SpeedNum,
                               SpeedRatio,
                               QZnReq,
                               QLatReq,
                               false,
                               false,
                               OnOffAirFlowRatio);
                    } else {
                        SimVariableSpeedCoils(state,
                                              BlankString,
                                              state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex,
                                              state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                                              state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                                              state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                                              state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                                              CompressorOp,
                                              PartLoadFrac,
                                              SpeedNum,
                                              SpeedRatio,
                                              QZnReq,
                                              QLatReq,
                                              OnOffAirFlowRatio);
                    }

                    SavePartloadRatio = PartLoadFrac;
                    SaveSpeedRatio = SpeedRatio;
                    state.dataFurnaces->SaveCompressorPLR =
                        state.dataVariableSpeedCoils->VarSpeedCoil(state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex).PartLoadRatio;
                } else {
                    if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
                        SimIHP(state,
                               BlankString,
                               state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                               state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                               state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                               state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                               state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                               CompressorOp,
                               PartLoadFrac,
                               SpeedNum,
                               SpeedRatio,
                               QZnReq,
                               QLatReq,
                               false,
                               false,
                               OnOffAirFlowRatio);
                    } else {
                        SimVariableSpeedCoils(state,
                                              BlankString,
                                              state.dataFurnaces->Furnace(FurnaceNum).HeatingCoilIndex,
                                              state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                                              state.dataFurnaces->Furnace(FurnaceNum).MaxONOFFCyclesperHour,
                                              state.dataFurnaces->Furnace(FurnaceNum).HPTimeConstant,
                                              state.dataFurnaces->Furnace(FurnaceNum).FanDelayTime,
                                              CompressorOp,
                                              0.0,
                                              1,
                                              0.0,
                                              0.0,
                                              0.0,
                                              OnOffAirFlowRatio);
                    }
                }
            } else if (state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilUpstream &&
                       (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatCool)) {
                // simulate furnace heating coil
                SuppHeatingCoilFlag = false; // if true simulates supplemental heating coil
                CalcNonDXHeatingCoils(state,
                                      FurnaceNum,
                                      SuppHeatingCoilFlag,
                                      FirstHVACIteration,
                                      HeatCoilLoad,
                                      state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                                      QCoilActual);
            }

            SimulateFanComponents(
                state, BlankString, FirstHVACIteration, state.dataFurnaces->Furnace(FurnaceNum).FanIndex, state.dataFurnaces->FanSpeedRatio);
            //  Simulate supplemental heating coil for draw through fan
            if (state.dataFurnaces->Furnace(FurnaceNum).SuppHeatCoilIndex > 0) {
                SuppHeatingCoilFlag = true; // if true simulates supplemental heating coil
                CalcNonDXHeatingCoils(state,
                                      FurnaceNum,
                                      SuppHeatingCoilFlag,
                                      FirstHVACIteration,
                                      SupHeaterLoad,
                                      state.dataFurnaces->Furnace(FurnaceNum).OpMode,
                                      QCoilActual);
            }
        }

        // If the fan runs continually do not allow coils to set OnOffFanPartLoadRatio.
        if (state.dataFurnaces->Furnace(FurnaceNum).OpMode == ContFanCycCoil) state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;

        auto &outNode = state.dataLoopNodes->Node(OutletNode);
        auto &zoneNode = state.dataLoopNodes->Node(state.dataFurnaces->Furnace(FurnaceNum).NodeNumOfControlledZone);
        Real64 zoneEnthalpy = PsyHFnTdbW(zoneNode.Temp, zoneNode.HumRat);
        Real64 outletEnthalpy = PsyHFnTdbW(outNode.Temp, outNode.HumRat);
        Real64 totalLoadMet = AirMassFlow * (outletEnthalpy - zoneEnthalpy);
        SensibleLoadMet =
            AirMassFlow * Psychrometrics::PsyDeltaHSenFnTdb2W2Tdb1W1(outNode.Temp, outNode.HumRat, zoneNode.Temp, zoneNode.HumRat); // sensible {W};
        LatentLoadMet = totalLoadMet - SensibleLoadMet;
        state.dataFurnaces->Furnace(FurnaceNum).LatentLoadMet = LatentLoadMet;
    }

    //******************************************************************************

    Real64 VSHPCyclingResidual(EnergyPlusData &state,
                               Real64 const PartLoadFrac, // compressor cycling ratio (1.0 is continuous, 0.0 is off)
                               int FurnaceNum,
                               // int ZoneNum,
                               bool FirstHVACIteration,
                               // int OpMode,
                               Real64 LoadToBeMet,
                               Real64 OnOffAirFlowRatio,
                               Real64 SupHeaterLoad,
                               CompressorOperation CompressorOp,
                               Real64 par9_SensLatFlag)
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Bo Shen, based on HVACMultiSpeedHeatPump:MSHPCyclingResidual
        //       DATE WRITTEN   March, 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        //  Calculates residual function ((ActualOutput - QZnReq)/QZnReq)
        //  MSHP output depends on the part load ratio which is being varied to zero the residual.

        // METHODOLOGY EMPLOYED:
        //  Calls CalcMSHeatPump to get ActualOutput at the given part load ratio
        //  and calculates the residual as defined above

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        // par[0] = FurnaceNum
        // par[1] = Zone Num
        // par[2] = FirstHVACIteration
        // par[3] = OpMode
        // par[4] = QZnReq, load to be met
        // par[5] = OnOffAirFlowRatio
        // par[6] = SupHeaterLoad
        // par[7] = NOT USED
        // par[8] = CompressorOp
        // par[9] = 1.0 to meet sensible load
        //        int FurnaceNum = int(Par[0]);
        //        int ZoneNum = int(Par[1]);
        //        bool FirstHVACIteration = (Par[2] == 1.0);
        //        int OpMode = int(Par[3]);
        //        Real64 LoadToBeMet = Par[4];
        //        Real64 OnOffAirFlowRatio = Par[5];
        //        Real64 SupHeaterLoad = Par[6];
        //        CompressorOperation CompressorOp = static_cast<CompressorOperation>(Par[8]);
        //        Real64 par9_SensLatFlag = Par[9];

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 QZnReq;          // zone sensible load (W)
        Real64 QZnLat;          // zone latent load (W)
        Real64 ZoneSensLoadMet; // delivered sensible capacity of MSHP
        Real64 ZoneLatLoadMet;  // delivered latent capacity of MSHP
        Real64 ResScale;        // Residual scale

        QZnReq = 0.0;
        QZnLat = 0.0;
        if (par9_SensLatFlag == 1.0) {
            QZnReq = LoadToBeMet;
        } else {
            QZnLat = LoadToBeMet;
        }

        CalcVarSpeedHeatPump(state,
                             FurnaceNum,
                             FirstHVACIteration,
                             CompressorOp,
                             1,
                             0.0,
                             PartLoadFrac,
                             ZoneSensLoadMet,
                             ZoneLatLoadMet,
                             QZnReq,
                             QZnLat,
                             OnOffAirFlowRatio,
                             SupHeaterLoad);

        ResScale = std::abs(LoadToBeMet);
        if (ResScale < 100.0) {
            ResScale = 100.0;
        } else {
            ResScale = LoadToBeMet;
        }

        // Calculate residual based on output calculation flag
        if (par9_SensLatFlag == 1.0) {
            return (ZoneSensLoadMet - LoadToBeMet) / ResScale;
        } else {
            return (ZoneLatLoadMet - LoadToBeMet) / ResScale;
        }
    }

    //******************************************************************************

    Real64 VSHPSpeedResidual(EnergyPlusData &state,
                             Real64 const SpeedRatio, // compressor cycling ratio (1.0 is continuous, 0.0 is off)
                             int FurnaceNum,
                             // int ZoneNum,
                             bool FirstHVACIteration,
                             // int OpMode
                             Real64 LoadToBeMet,
                             Real64 OnOffAirFlowRatio,
                             Real64 SupHeaterLoad,
                             int SpeedNum,
                             CompressorOperation CompressorOp,
                             Real64 par9_SensLatFlag)
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Bo Shen, , based on HVACMultiSpeedHeatPump:MSHPVarSpeedgResidual
        //       DATE WRITTEN   March, 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        //  Calculates residual function ((ActualOutput - QZnReq)/QZnReq)
        //  MSHP output depends on the part load ratio which is being varied to zero the residual.

        // METHODOLOGY EMPLOYED:
        //  Calls CalcMSHeatPump to get ActualOutput at the given speed ratio (partload ratio for high speed)
        //  and calculates the residual as defined above

        Real64 QZnReq = 0.0;
        Real64 QZnLat = 0.0;
        if (par9_SensLatFlag == 1.0) {
            QZnReq = LoadToBeMet;
        } else {
            QZnLat = LoadToBeMet;
        }

        Real64 ZoneSensLoadMet; // delivered sensible capacity of MSHP
        Real64 ZoneLatLoadMet;  // delivered latent capacity of MSHP
        CalcVarSpeedHeatPump(state,
                             FurnaceNum,
                             FirstHVACIteration,
                             CompressorOp,
                             SpeedNum,
                             SpeedRatio,
                             1.0,
                             ZoneSensLoadMet,
                             ZoneLatLoadMet,
                             QZnReq,
                             QZnLat,
                             OnOffAirFlowRatio,
                             SupHeaterLoad);

        Real64 ResScale = std::abs(LoadToBeMet);
        if (ResScale < 100.0) {
            ResScale = 100.0;
        } else {
            ResScale = LoadToBeMet;
        }

        // Calculate residual based on output calculation flag
        if (par9_SensLatFlag == 1.0) {
            return (ZoneSensLoadMet - LoadToBeMet) / ResScale;
        } else {
            return (ZoneLatLoadMet - LoadToBeMet) / ResScale;
        }
    }

    void SetVSHPAirFlow(EnergyPlusData &state,
                        int const FurnaceNum,                        // Unit index
                        Real64 const PartLoadRatio,                  // unit part load ratio
                        Real64 &OnOffAirFlowRatio,                   // ratio of compressor ON airflow to average airflow over timestep
                        ObjexxFCL::Optional_int_const SpeedNum,      // Speed number
                        ObjexxFCL::Optional<Real64 const> SpeedRatio // Speed ratio
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Bo Shen, based on HVACMultiSpeedHeatPump:SetAverageAirFlow
        //       DATE WRITTEN   March, 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na
        // PURPOSE OF THIS SUBROUTINE:
        // Set the average air mass flow rates using the part load fraction of the heat pump for this time step
        // Set OnOffAirFlowRatio to be used by DX coils

        // METHODOLOGY EMPLOYED:
        // na

        // REFERENCES:
        // na

        // Using/Aliasing
        using IntegratedHeatPump::GetAirMassFlowRateIHP;
        using IntegratedHeatPump::GetMaxSpeedNumIHP;
        using IntegratedHeatPump::IHPOperationMode;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int InletNode;              // inlet node number for PTHPNum
        Real64 AverageUnitMassFlow; // average supply air mass flow rate over time step
        int OutNode;                // Outlet node number in MSHP loop

        InletNode = state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum;
        OutNode = state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum;

        state.dataHVACGlobal->MSHPMassFlowRateLow = 0.0;  // Mass flow rate at low speed
        state.dataHVACGlobal->MSHPMassFlowRateHigh = 0.0; // Mass flow rate at high speed

        if (state.dataFurnaces->Furnace(FurnaceNum).OpMode == ContFanCycCoil) {
            state.dataFurnaces->CompOffMassFlow = state.dataFurnaces->Furnace(FurnaceNum).IdleMassFlowRate;
            state.dataFurnaces->CompOffFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).IdleSpeedRatio;
        } else {
            state.dataFurnaces->CompOffMassFlow = 0.0;
            state.dataFurnaces->CompOffFlowRatio = 0.0;
        }

        if (state.dataFurnaces->CoolingLoad && (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatCool)) {
            if (state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling > 0) {
                state.dataFurnaces->CompOnMassFlow =
                    state.dataFurnaces->Furnace(FurnaceNum).CoolMassFlowRate(state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling);
                state.dataFurnaces->CompOnFlowRatio =
                    state.dataFurnaces->Furnace(FurnaceNum).MSCoolingSpeedRatio(state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling);
                state.dataHVACGlobal->MSHPMassFlowRateLow =
                    state.dataFurnaces->Furnace(FurnaceNum).CoolMassFlowRate(state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling);
                state.dataHVACGlobal->MSHPMassFlowRateHigh =
                    state.dataFurnaces->Furnace(FurnaceNum).CoolMassFlowRate(state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedCooling);
            } else {
                state.dataFurnaces->CompOnMassFlow = state.dataFurnaces->Furnace(FurnaceNum).MaxCoolAirMassFlow;
                state.dataFurnaces->CompOnFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).CoolingSpeedRatio;
            }
            AverageUnitMassFlow = (PartLoadRatio * state.dataFurnaces->CompOnMassFlow) + ((1 - PartLoadRatio) * state.dataFurnaces->CompOffMassFlow);
            if (state.dataFurnaces->CompOffFlowRatio > 0.0) {
                state.dataFurnaces->FanSpeedRatio =
                    (PartLoadRatio * state.dataFurnaces->CompOnFlowRatio) + ((1 - PartLoadRatio) * state.dataFurnaces->CompOffFlowRatio);
            } else {
                state.dataFurnaces->FanSpeedRatio = state.dataFurnaces->CompOnFlowRatio;
            }
        } else if (state.dataFurnaces->HeatingLoad && (state.dataFurnaces->Furnace(FurnaceNum).FurnaceType_Num == UnitarySys_HeatCool)) {
            if (state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedHeating > 0) {
                state.dataFurnaces->CompOnMassFlow =
                    state.dataFurnaces->Furnace(FurnaceNum).HeatMassFlowRate(state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedHeating);
                state.dataFurnaces->CompOnFlowRatio =
                    state.dataFurnaces->Furnace(FurnaceNum).MSHeatingSpeedRatio(state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedHeating);
                state.dataHVACGlobal->MSHPMassFlowRateLow =
                    state.dataFurnaces->Furnace(FurnaceNum).HeatMassFlowRate(state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedHeating);
                state.dataHVACGlobal->MSHPMassFlowRateHigh =
                    state.dataFurnaces->Furnace(FurnaceNum).HeatMassFlowRate(state.dataFurnaces->Furnace(FurnaceNum).NumOfSpeedHeating);
            } else {
                state.dataFurnaces->CompOnMassFlow = state.dataFurnaces->Furnace(FurnaceNum).MaxHeatAirMassFlow;
                state.dataFurnaces->CompOnFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).HeatingSpeedRatio;
            }
            AverageUnitMassFlow = (PartLoadRatio * state.dataFurnaces->CompOnMassFlow) + ((1 - PartLoadRatio) * state.dataFurnaces->CompOffMassFlow);
            if (state.dataFurnaces->CompOffFlowRatio > 0.0) {
                state.dataFurnaces->FanSpeedRatio =
                    (PartLoadRatio * state.dataFurnaces->CompOnFlowRatio) + ((1 - PartLoadRatio) * state.dataFurnaces->CompOffFlowRatio);
            } else {
                state.dataFurnaces->FanSpeedRatio = state.dataFurnaces->CompOnFlowRatio;
            }
        } else if (state.dataFurnaces->Furnace(FurnaceNum).bIsIHP) {
            if (!state.dataZoneEnergyDemand->CurDeadBandOrSetback(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum) && present(SpeedNum)) {
                // if(present(SpeedNum)) {
                state.dataFurnaces->CompOnMassFlow =
                    GetAirMassFlowRateIHP(state, state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex, SpeedNum, SpeedRatio, false);
                state.dataFurnaces->CompOnFlowRatio =
                    state.dataFurnaces->CompOnMassFlow /
                    GetAirMassFlowRateIHP(state,
                                          state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                          GetMaxSpeedNumIHP(state, state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex),
                                          1.0,
                                          false);
                state.dataHVACGlobal->MSHPMassFlowRateLow =
                    GetAirMassFlowRateIHP(state, state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex, SpeedNum, 0.0, false);
                state.dataHVACGlobal->MSHPMassFlowRateHigh =
                    GetAirMassFlowRateIHP(state, state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex, SpeedNum, 1.0, false);
            }

            // Set up fan flow rate during compressor off time
            if (state.dataFurnaces->Furnace(FurnaceNum).OpMode == ContFanCycCoil && present(SpeedNum)) {
                if (state.dataFurnaces->Furnace(FurnaceNum).AirFlowControl == AirFlowControlConstFan::UseCompressorOnFlow &&
                    state.dataFurnaces->CompOnMassFlow > 0.0) {
                    state.dataFurnaces->CompOffMassFlow =
                        GetAirMassFlowRateIHP(state, state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex, SpeedNum, 1.0, false);
                    state.dataFurnaces->CompOffFlowRatio =
                        state.dataFurnaces->CompOffMassFlow /
                        GetAirMassFlowRateIHP(state,
                                              state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex,
                                              GetMaxSpeedNumIHP(state, state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex),
                                              1.0,
                                              false);
                }
            }

            if (present(SpeedNum)) {
                if (SpeedNum > 1) {
                    AverageUnitMassFlow = state.dataFurnaces->CompOnMassFlow;
                    state.dataFurnaces->FanSpeedRatio = state.dataFurnaces->CompOnFlowRatio;
                } else {
                    AverageUnitMassFlow =
                        (PartLoadRatio * state.dataFurnaces->CompOnMassFlow) + ((1 - PartLoadRatio) * state.dataFurnaces->CompOffMassFlow);
                    if (state.dataFurnaces->CompOffFlowRatio > 0.0) {
                        state.dataFurnaces->FanSpeedRatio =
                            (PartLoadRatio * state.dataFurnaces->CompOnFlowRatio) + ((1 - PartLoadRatio) * state.dataFurnaces->CompOffFlowRatio);
                    } else {
                        state.dataFurnaces->FanSpeedRatio = state.dataFurnaces->CompOnFlowRatio;
                    }
                }
            } else {
                AverageUnitMassFlow =
                    (PartLoadRatio * state.dataFurnaces->CompOnMassFlow) + ((1 - PartLoadRatio) * state.dataFurnaces->CompOffMassFlow);
                if (state.dataFurnaces->CompOffFlowRatio > 0.0) {
                    state.dataFurnaces->FanSpeedRatio =
                        (PartLoadRatio * state.dataFurnaces->CompOnFlowRatio) + ((1 - PartLoadRatio) * state.dataFurnaces->CompOffFlowRatio);
                } else {
                    state.dataFurnaces->FanSpeedRatio = state.dataFurnaces->CompOnFlowRatio;
                }
            }

            if (IHPOperationMode::SCWHMatchWH ==
                state.dataIntegratedHP->IntegratedHeatPumps(state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex).CurMode) {
                state.dataFurnaces->CompOnMassFlow =
                    GetAirMassFlowRateIHP(state, state.dataFurnaces->Furnace(FurnaceNum).CoolingCoilIndex, SpeedNum, SpeedRatio, false);
                AverageUnitMassFlow = state.dataFurnaces->CompOnMassFlow;
            }
        } else {
            if (!state.dataZoneEnergyDemand->CurDeadBandOrSetback(state.dataFurnaces->Furnace(FurnaceNum).ControlZoneNum) && present(SpeedNum)) {
                if (state.dataFurnaces->Furnace(FurnaceNum).HeatCoolMode == Furnaces::ModeOfOperation::HeatingMode) {
                    if (SpeedNum == 1) {
                        state.dataFurnaces->CompOnMassFlow = state.dataFurnaces->Furnace(FurnaceNum).HeatMassFlowRate(SpeedNum);
                        state.dataFurnaces->CompOnFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).MSHeatingSpeedRatio(SpeedNum);
                        state.dataHVACGlobal->MSHPMassFlowRateLow = state.dataFurnaces->Furnace(FurnaceNum).HeatMassFlowRate(1);
                        state.dataHVACGlobal->MSHPMassFlowRateHigh = state.dataFurnaces->Furnace(FurnaceNum).HeatMassFlowRate(1);
                    } else if (SpeedNum > 1) {
                        state.dataFurnaces->CompOnMassFlow =
                            SpeedRatio * state.dataFurnaces->Furnace(FurnaceNum).HeatMassFlowRate(SpeedNum) +
                            (1.0 - SpeedRatio) * state.dataFurnaces->Furnace(FurnaceNum).HeatMassFlowRate(SpeedNum - 1);
                        state.dataFurnaces->CompOnFlowRatio =
                            SpeedRatio * state.dataFurnaces->Furnace(FurnaceNum).MSHeatingSpeedRatio(SpeedNum) +
                            (1.0 - SpeedRatio) * state.dataFurnaces->Furnace(FurnaceNum).MSHeatingSpeedRatio(SpeedNum - 1);
                        state.dataHVACGlobal->MSHPMassFlowRateLow = state.dataFurnaces->Furnace(FurnaceNum).HeatMassFlowRate(SpeedNum - 1);
                        state.dataHVACGlobal->MSHPMassFlowRateHigh = state.dataFurnaces->Furnace(FurnaceNum).HeatMassFlowRate(SpeedNum);
                    }
                } else if (state.dataFurnaces->Furnace(FurnaceNum).HeatCoolMode == Furnaces::ModeOfOperation::CoolingMode) {
                    if (SpeedNum == 1) {
                        state.dataFurnaces->CompOnMassFlow = state.dataFurnaces->Furnace(FurnaceNum).CoolMassFlowRate(SpeedNum);
                        state.dataFurnaces->CompOnFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).MSCoolingSpeedRatio(SpeedNum);
                        state.dataHVACGlobal->MSHPMassFlowRateLow = state.dataFurnaces->Furnace(FurnaceNum).CoolMassFlowRate(1);
                        state.dataHVACGlobal->MSHPMassFlowRateHigh = state.dataFurnaces->Furnace(FurnaceNum).CoolMassFlowRate(1);
                    } else if (SpeedNum > 1) {
                        state.dataFurnaces->CompOnMassFlow =
                            SpeedRatio * state.dataFurnaces->Furnace(FurnaceNum).CoolMassFlowRate(SpeedNum) +
                            (1.0 - SpeedRatio) * state.dataFurnaces->Furnace(FurnaceNum).CoolMassFlowRate(SpeedNum - 1);
                        state.dataFurnaces->CompOnFlowRatio =
                            SpeedRatio * state.dataFurnaces->Furnace(FurnaceNum).MSCoolingSpeedRatio(SpeedNum) +
                            (1.0 - SpeedRatio) * state.dataFurnaces->Furnace(FurnaceNum).MSCoolingSpeedRatio(SpeedNum - 1);
                        state.dataHVACGlobal->MSHPMassFlowRateLow = state.dataFurnaces->Furnace(FurnaceNum).CoolMassFlowRate(SpeedNum - 1);
                        state.dataHVACGlobal->MSHPMassFlowRateHigh = state.dataFurnaces->Furnace(FurnaceNum).CoolMassFlowRate(SpeedNum);
                    }
                }
            }

            // Set up fan flow rate during compressor off time
            if (state.dataFurnaces->Furnace(FurnaceNum).OpMode == ContFanCycCoil && present(SpeedNum)) {
                if (state.dataFurnaces->Furnace(FurnaceNum).AirFlowControl == AirFlowControlConstFan::UseCompressorOnFlow &&
                    state.dataFurnaces->CompOnMassFlow > 0.0) {
                    if (SpeedNum == 1) { // LOWEST SPEED USE IDLE FLOW
                        state.dataFurnaces->CompOffMassFlow = state.dataFurnaces->Furnace(FurnaceNum).IdleMassFlowRate;
                        state.dataFurnaces->CompOffFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).IdleSpeedRatio;
                    } else if (state.dataFurnaces->Furnace(FurnaceNum).LastMode == Furnaces::ModeOfOperation::HeatingMode) {
                        state.dataFurnaces->CompOffMassFlow = state.dataFurnaces->Furnace(FurnaceNum).HeatMassFlowRate(SpeedNum);
                        state.dataFurnaces->CompOffFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).MSHeatingSpeedRatio(SpeedNum);
                    } else {
                        state.dataFurnaces->CompOffMassFlow = state.dataFurnaces->Furnace(FurnaceNum).CoolMassFlowRate(SpeedNum);
                        state.dataFurnaces->CompOffFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).MSCoolingSpeedRatio(SpeedNum);
                    }
                }
            }

            if (present(SpeedNum)) {
                if (SpeedNum > 1) {
                    AverageUnitMassFlow = state.dataFurnaces->CompOnMassFlow;
                    state.dataFurnaces->FanSpeedRatio = state.dataFurnaces->CompOnFlowRatio;
                } else {
                    AverageUnitMassFlow =
                        (PartLoadRatio * state.dataFurnaces->CompOnMassFlow) + ((1 - PartLoadRatio) * state.dataFurnaces->CompOffMassFlow);
                    if (state.dataFurnaces->CompOffFlowRatio > 0.0) {
                        state.dataFurnaces->FanSpeedRatio =
                            (PartLoadRatio * state.dataFurnaces->CompOnFlowRatio) + ((1 - PartLoadRatio) * state.dataFurnaces->CompOffFlowRatio);
                    } else {
                        state.dataFurnaces->FanSpeedRatio = state.dataFurnaces->CompOnFlowRatio;
                    }
                }
            } else {
                AverageUnitMassFlow =
                    (PartLoadRatio * state.dataFurnaces->CompOnMassFlow) + ((1 - PartLoadRatio) * state.dataFurnaces->CompOffMassFlow);
                if (state.dataFurnaces->CompOffFlowRatio > 0.0) {
                    state.dataFurnaces->FanSpeedRatio =
                        (PartLoadRatio * state.dataFurnaces->CompOnFlowRatio) + ((1 - PartLoadRatio) * state.dataFurnaces->CompOffFlowRatio);
                } else {
                    state.dataFurnaces->FanSpeedRatio = state.dataFurnaces->CompOnFlowRatio;
                }
            }
        }

        if (GetCurrentScheduleValue(state, state.dataFurnaces->Furnace(FurnaceNum).SchedPtr) == 0.0) {
            state.dataLoopNodes->Node(InletNode).MassFlowRate = 0.0;
            OnOffAirFlowRatio = 0.0;
        } else {
            state.dataLoopNodes->Node(InletNode).MassFlowRate = AverageUnitMassFlow;
            state.dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = AverageUnitMassFlow;
            if (AverageUnitMassFlow > 0.0) {
                OnOffAirFlowRatio = state.dataFurnaces->CompOnMassFlow / AverageUnitMassFlow;
            } else {
                OnOffAirFlowRatio = 0.0;
            }
        }

        state.dataLoopNodes->Node(OutNode).MassFlowRate = state.dataLoopNodes->Node(InletNode).MassFlowRate;

        //  IF(ABS(Node(OutNode)%MassFlowRate - 0.435)  < 0.001) THEN
        //    Node(OutNode)%MassFlowRate  = Node(InletNode)%MassFlowRate
        //  END IF
    }

    void SetOnOffMassFlowRateVSCoil(EnergyPlusData &state,
                                    int const FurnaceNum,                       // index to furnace
                                    int const ZoneNum,                          // index to zone
                                    bool const FirstHVACIteration,              // Flag for 1st HVAC iteration
                                    [[maybe_unused]] int const AirLoopNum,      // index to air loop !unused1208
                                    Real64 &OnOffAirFlowRatio,                  // ratio of coil on to coil off air flow rate
                                    [[maybe_unused]] int const OpMode,          // fan operating mode
                                    [[maybe_unused]] Real64 const QZnReq,       // sensible load to be met (W) !unused1208
                                    [[maybe_unused]] Real64 const MoistureLoad, // moisture load to be met (W)
                                    Real64 &PartLoadRatio                       // coil part-load ratio
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Bo Shen
        //       DATE WRITTEN   March 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the Furnace Components.

        // METHODOLOGY EMPLOYED:
        // The HeatCool furnace/unitarysystem and air-to-air heat pump may have alternate air flow rates
        // in cooling, heating, and when no cooling or heating is needed. Set up the coil (comp) ON and OFF
        // air flow rates. Use these flow rates during the Calc routines to set the average mass flow rates
        // based on PLR.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int InNode;  // Inlet node number in MSHP loop
        int OutNode; // Outlet node number in MSHP loop

        InNode = state.dataFurnaces->Furnace(FurnaceNum).FurnaceInletNodeNum;
        OutNode = state.dataFurnaces->Furnace(FurnaceNum).FurnaceOutletNodeNum;

        if (state.dataFurnaces->CoolingLoad) {
            state.dataFurnaces->Furnace(FurnaceNum).HeatCoolMode = Furnaces::ModeOfOperation::CoolingMode;
        } else if (state.dataFurnaces->HeatingLoad) {
            state.dataFurnaces->Furnace(FurnaceNum).HeatCoolMode = Furnaces::ModeOfOperation::HeatingMode;
        } else {
            state.dataFurnaces->Furnace(FurnaceNum).HeatCoolMode = Furnaces::ModeOfOperation::NoCoolHeat;
        }

        // Set the inlet node mass flow rate
        if (state.dataFurnaces->Furnace(FurnaceNum).OpMode == ContFanCycCoil) {
            // constant fan mode
            if ((state.dataFurnaces->Furnace(FurnaceNum).HeatCoolMode == Furnaces::ModeOfOperation::HeatingMode) &&
                !state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) {
                state.dataFurnaces->CompOnMassFlow = state.dataFurnaces->Furnace(FurnaceNum).HeatMassFlowRate(1);
                state.dataFurnaces->CompOnFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).MSHeatingSpeedRatio(1);
                state.dataFurnaces->Furnace(FurnaceNum).LastMode = Furnaces::ModeOfOperation::HeatingMode;
            } else if ((state.dataFurnaces->Furnace(FurnaceNum).HeatCoolMode == Furnaces::ModeOfOperation::CoolingMode) &&
                       !state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) {
                state.dataFurnaces->CompOnMassFlow = state.dataFurnaces->Furnace(FurnaceNum).CoolMassFlowRate(1);
                state.dataFurnaces->CompOnFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).MSCoolingSpeedRatio(1);
                state.dataFurnaces->Furnace(FurnaceNum).LastMode = Furnaces::ModeOfOperation::CoolingMode;
            } else {
                state.dataFurnaces->CompOnMassFlow = state.dataFurnaces->Furnace(FurnaceNum).IdleMassFlowRate;
                state.dataFurnaces->CompOnFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).IdleSpeedRatio;
            }
            state.dataFurnaces->CompOffMassFlow = state.dataFurnaces->Furnace(FurnaceNum).IdleMassFlowRate;
            state.dataFurnaces->CompOffFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).IdleSpeedRatio;
        } else {
            // cycling fan mode
            if ((state.dataFurnaces->Furnace(FurnaceNum).HeatCoolMode == Furnaces::ModeOfOperation::HeatingMode) &&
                !state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) {
                state.dataFurnaces->CompOnMassFlow = state.dataFurnaces->Furnace(FurnaceNum).HeatMassFlowRate(1);
                state.dataFurnaces->CompOnFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).MSHeatingSpeedRatio(1);
            } else if ((state.dataFurnaces->Furnace(FurnaceNum).HeatCoolMode == Furnaces::ModeOfOperation::CoolingMode) &&
                       !state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) {
                state.dataFurnaces->CompOnMassFlow = state.dataFurnaces->Furnace(FurnaceNum).CoolMassFlowRate(1);
                state.dataFurnaces->CompOnFlowRatio = state.dataFurnaces->Furnace(FurnaceNum).MSCoolingSpeedRatio(1);
            } else {
                state.dataFurnaces->CompOnMassFlow = 0.0;
                state.dataFurnaces->CompOnFlowRatio = 0.0;
            }
            state.dataFurnaces->CompOffMassFlow = 0.0;
            state.dataFurnaces->CompOffFlowRatio = 0.0;
        }

        // Set the inlet node mass flow rate
        if (GetCurrentScheduleValue(state, state.dataFurnaces->Furnace(FurnaceNum).FanAvailSchedPtr) > 0.0 &&
            state.dataFurnaces->CompOnMassFlow != 0.0) {
            OnOffAirFlowRatio = 1.0;
            if (FirstHVACIteration) {
                state.dataLoopNodes->Node(InNode).MassFlowRate = state.dataFurnaces->CompOnMassFlow;
                PartLoadRatio = 0.0;
            } else {
                if (state.dataFurnaces->Furnace(FurnaceNum).HeatCoolMode != Furnaces::ModeOfOperation::NoCoolHeat) {
                    PartLoadRatio = 1.0;
                } else {
                    PartLoadRatio = 0.0;
                }
            }
        } else {
            PartLoadRatio = 0.0;
            state.dataLoopNodes->Node(InNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(OutNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(OutNode).MassFlowRateMaxAvail = 0.0;
            OnOffAirFlowRatio = 1.0;
        }

        // Set the system mass flow rates
        SetVSHPAirFlow(state, FurnaceNum, PartLoadRatio, OnOffAirFlowRatio);
    }

    void SetMinOATCompressor(EnergyPlusData &state,
                             int const FurnaceNum,                    // index to furnace
                             std::string const &cCurrentModuleObject, // type of furnace
                             bool &ErrorsFound                        // GetInput logical that errors were found
    )
    {
        bool errFlag = false;
        auto &furnace = state.dataFurnaces->Furnace(FurnaceNum);

        // Set minimum OAT for heat pump compressor operation in heating mode
        if (furnace.CoolingCoilType_Num == CoilDX_CoolingSingleSpeed) {
            furnace.MinOATCompressorCooling = DXCoils::GetMinOATCompressor(state, furnace.CoolingCoilIndex, errFlag);
        } else if (furnace.CoolingCoilType_Num == CoilDX_CoolingHXAssisted) {
            std::string ChildCoolingCoilType = state.dataHVACAssistedCC->HXAssistedCoil(furnace.CoolingCoilIndex).CoolingCoilType;
            std::string ChildCoolingCoilName = state.dataHVACAssistedCC->HXAssistedCoil(furnace.CoolingCoilIndex).CoolingCoilName;

            if (UtilityRoutines::SameString(ChildCoolingCoilType, "COIL:COOLING:DX")) {
                int childCCIndex_DX = CoilCoolingDX::factory(state, ChildCoolingCoilName);
                if (childCCIndex_DX < 0) {
                    ShowContinueError(state, format("Occurs in {} = {}", cCurrentModuleObject, furnace.Name));
                    errFlag = true;
                    ErrorsFound = true;
                }
                auto &newCoil = state.dataCoilCooingDX->coilCoolingDXs[childCCIndex_DX];
                furnace.MinOATCompressorCooling = newCoil.performance.minOutdoorDrybulb;
            } else if (UtilityRoutines::SameString(ChildCoolingCoilType, "Coil:Cooling:DX:VariableSpeed")) {
                int childCCIndex_VS = state.dataHVACAssistedCC->HXAssistedCoil(furnace.CoolingCoilIndex).CoolingCoilIndex;
                furnace.MinOATCompressorCooling = VariableSpeedCoils::GetVSCoilMinOATCompressor(state, childCCIndex_VS, errFlag);
            } else { // Single speed
                int childCCIndex_SP = state.dataHVACAssistedCC->HXAssistedCoil(furnace.CoolingCoilIndex).CoolingCoilIndex;
                furnace.MinOATCompressorCooling = DXCoils::GetMinOATCompressor(state, childCCIndex_SP, errFlag);
            }
        } else if (furnace.CoolingCoilType_Num == Coil_CoolingAirToAirVariableSpeed) {
            furnace.MinOATCompressorCooling = VariableSpeedCoils::GetVSCoilMinOATCompressor(state, furnace.CoolingCoilIndex, errFlag);
        } else {
            furnace.MinOATCompressorCooling = -1000.0;
        }
        if (errFlag) {
            ShowContinueError(state, format("...occurs in {} = {}", cCurrentModuleObject, furnace.Name));
            ErrorsFound = true;
        }

        // Set minimum OAT for heat pump compressor operation in heating mode
        errFlag = false;
        if (furnace.HeatingCoilType_Num == Coil_HeatingAirToAirVariableSpeed) {
            furnace.MinOATCompressorHeating = VariableSpeedCoils::GetVSCoilMinOATCompressor(state, furnace.HeatingCoilIndex, errFlag);
        } else if (furnace.HeatingCoilType_Num == CoilDX_HeatingEmpirical) {
            furnace.MinOATCompressorHeating = DXCoils::GetMinOATCompressor(state, furnace.HeatingCoilIndex, errFlag);
        } else {
            furnace.MinOATCompressorHeating = -1000.0;
        }
        if (errFlag) {
            ShowContinueError(state, format("...occurs in {} = {}", cCurrentModuleObject, furnace.Name));
            ErrorsFound = true;
        }
    }

} // namespace Furnaces

} // namespace EnergyPlus

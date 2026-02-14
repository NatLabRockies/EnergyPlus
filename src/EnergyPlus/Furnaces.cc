// EnergyPlus, Copyright (c) 1996-present, The Board of Trustees of the University of Illinois,
// The Regents of the University of California, through Lawrence Berkeley National Laboratory
// (subject to receipt of any required approvals from the U.S. Dept. of Energy), Oak Ridge
// National Laboratory, managed by UT-Battelle, Alliance for Energy Innovation, LLC, and other
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
#include <EnergyPlus/Autosizing/CoolingCapacitySizing.hh>
#include <EnergyPlus/Autosizing/HeatingCapacitySizing.hh>
#include <EnergyPlus/BranchInputManager.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Coils/CoilCoolingDX.hh>
#include <EnergyPlus/CurveManager.hh>
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
#include <EnergyPlus/OutputReportPredefined.hh>
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

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and algorithms required to
    // manage the Furnace/Unitary System Compound Component

    // METHODOLOGY EMPLOYED:
    // Calculates the part-load ratio of the HVAC system to meet the zone sensible load. For non-heat pump HVAC systems,
    // if humidity control is specified and the latent capacity at the sensible PLR is insufficient to meet the latent load,
    // calculate a latent part-load ratio to meet the zone sensible load (MultiMode dehumidification control) or the zone
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
    //                                may exist for a single time step (heating and dehumidification can occur). For all
    //                                other system types, only a single PLR is allowed for any given time step.
    //                                Order of simulation depends on dehumidification control option as described below.
    // Dehumidification control options (non-heat pump versions):
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

    // MODULE PARAMETER DEFINITIONS
    static constexpr std::string_view BlankString;

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

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int FurnaceNum;           // Furnace number
        Real64 HeatCoilLoad(0.0); // Zone heating coil load
        Real64 ReheatCoilLoad;    // Load to be met by the reheat coil (if high humidity control)
        Real64 MoistureLoad;      // Control zone latent load
        Real64 Dummy(0.0);
        HVAC::FanOp fanOp = HVAC::FanOp::Invalid; // Fan operating mode (1=FanOp::Cycling, 2=FanOp::Continuous)

        Real64 QActual;           // actual heating coil output (W)
        bool SuppHeatingCoilFlag; // true if supplemental heating coil

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
            FurnaceNum = Util::FindItemInList(FurnaceName, state.dataFurnaces->Furnaces);
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
                if (FurnaceName != state.dataFurnaces->Furnaces(FurnaceNum).Name) {
                    ShowFatalError(state,
                                   format("SimFurnace: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                          FurnaceNum,
                                          FurnaceName,
                                          state.dataFurnaces->Furnaces(FurnaceNum).Name));
                }
                state.dataFurnaces->CheckEquipName(FurnaceNum) = false;
            }
        }

        bool HXUnitOn = false;          // flag to control HX assisted cooling coil
        Real64 OnOffAirFlowRatio = 0.0; // Ratio of compressor ON air flow to AVERAGE air flow over time step
        // here we need to deal with sequenced zone equip sensible load in control zone
        Real64 ZoneLoad = 0.0;

        auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);
        auto &zoneSysEnergyDemand = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(furnace.ControlZoneNum);
        if (furnace.ZoneSequenceCoolingNum > 0 && furnace.ZoneSequenceHeatingNum > 0) {
            Real64 ZoneLoadToCoolSPSequenced = zoneSysEnergyDemand.SequencedOutputRequiredToCoolingSP(furnace.ZoneSequenceCoolingNum);
            Real64 ZoneLoadToHeatSPSequenced = zoneSysEnergyDemand.SequencedOutputRequiredToHeatingSP(furnace.ZoneSequenceHeatingNum);
            auto const &tempControlType = state.dataHeatBalFanSys->TempControlType(furnace.ControlZoneNum);
            if (ZoneLoadToHeatSPSequenced > 0.0 && ZoneLoadToCoolSPSequenced > 0.0 && tempControlType != HVAC::SetptType::SingleCool) {
                ZoneLoad = ZoneLoadToHeatSPSequenced;
            } else if (ZoneLoadToHeatSPSequenced > 0.0 && ZoneLoadToCoolSPSequenced > 0.0 && tempControlType == HVAC::SetptType::SingleCool) {
                ZoneLoad = 0.0;
            } else if (ZoneLoadToHeatSPSequenced < 0.0 && ZoneLoadToCoolSPSequenced < 0.0 && tempControlType != HVAC::SetptType::SingleHeat) {
                ZoneLoad = ZoneLoadToCoolSPSequenced;
            } else if (ZoneLoadToHeatSPSequenced < 0.0 && ZoneLoadToCoolSPSequenced < 0.0 && tempControlType == HVAC::SetptType::SingleHeat) {
                ZoneLoad = 0.0;
            } else if (ZoneLoadToHeatSPSequenced <= 0.0 && ZoneLoadToCoolSPSequenced >= 0.0) {
                ZoneLoad = 0.0;
            }
            MoistureLoad = state.dataZoneEnergyDemand->ZoneSysMoistureDemand(furnace.ControlZoneNum)
                               .SequencedOutputRequiredToDehumidSP(furnace.ZoneSequenceCoolingNum);
        } else {
            ZoneLoad = zoneSysEnergyDemand.RemainingOutputRequired;
            MoistureLoad = state.dataZoneEnergyDemand->ZoneSysMoistureDemand(furnace.ControlZoneNum).OutputRequiredToDehumidifyingSP;
        }

        // H2OHtOfVap
        MoistureLoad *= Psychrometrics::PsyHfgAirFnWTdb(state.dataLoopNodes->Node(furnace.NodeNumOfControlledZone).HumRat,
                                                        state.dataLoopNodes->Node(furnace.NodeNumOfControlledZone).Temp);

        // Initialize Furnace Flows
        InitFurnace(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, fanOp, ZoneLoad, MoistureLoad, FirstHVACIteration);

        int FurnaceInletNode = furnace.FurnaceInletNode;

        // MassFlowRateMaxAvail issues are impeding non-VAV air loop equipment by limiting air flow
        // temporarily open up flow limits while simulating, and then set this same value at the INLET after this parent has simulated
        Real64 TempMassFlowRateMaxAvail = state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRateMaxAvail;
        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRateMaxAvail = furnace.DesignMassFlowRate;

        Real64 FurnaceSavMdot = state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate;
        HVAC::CompressorOp compressorOp = HVAC::CompressorOp::On;
        state.dataFurnaces->CoolHeatPLRRat = 1.0;

        // Simulate correct system type (1 of 4 choices)
        switch (furnace.type) {
            // Simulate HeatOnly systems:
        case HVAC::UnitarySysType::Furnace_HeatOnly:
        case HVAC::UnitarySysType::Unitary_HeatOnly: {
            // Update the furnace flow rates
            CalcNewZoneHeatOnlyFlowRates(state, FurnaceNum, FirstHVACIteration, ZoneLoad, HeatCoilLoad, OnOffAirFlowRatio);

            if (furnace.fanPlace == HVAC::FanPlace::BlowThru) {
                // simulate fan
                state.dataFans->fans(furnace.FanIndex)->simulate(state, FirstHVACIteration, state.dataFurnaces->FanSpeedRatio);
            }

            // simulate furnace heating coil
            SuppHeatingCoilFlag = false; // if true simulates supplemental heating coil
            CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, fanOp, QActual);

            if (furnace.fanPlace == HVAC::FanPlace::DrawThru) {
                // simulate fan
                state.dataFans->fans(furnace.FanIndex)->simulate(state, FirstHVACIteration, state.dataFurnaces->FanSpeedRatio);
            }
        } break;
            // Simulate HeatCool systems:
        case HVAC::UnitarySysType::Furnace_HeatCool:
        case HVAC::UnitarySysType::Unitary_HeatCool: {
            if (furnace.coolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
                // variable speed cooling coil
                HeatCoilLoad = 0.0;
                if (furnace.isIHP)
                    state.dataIntegratedHP->IntegratedHeatPumps(furnace.CoolCoilNum).ControlledZoneTemp =
                        state.dataLoopNodes->Node(furnace.NodeNumOfControlledZone).Temp;
                SimVariableSpeedHP(state, FurnaceNum, FirstHVACIteration, AirLoopNum, ZoneLoad, MoistureLoad, OnOffAirFlowRatio);
            } else {
                // calculate the system flow rate
                if (!FirstHVACIteration && furnace.fanOp == HVAC::FanOp::Cycling && state.dataFurnaces->CoolingLoad &&
                    state.dataAirLoop->AirLoopControlInfo(AirLoopNum).EconoActive) {
                    // for cycling fan, cooling load, check whether furnace can meet load with compressor off
                    compressorOp = HVAC::CompressorOp::Off;
                    CalcNewZoneHeatCoolFlowRates(state,
                                                 FurnaceNum,
                                                 FirstHVACIteration,
                                                 compressorOp,
                                                 ZoneLoad,
                                                 MoistureLoad,
                                                 HeatCoilLoad,
                                                 ReheatCoilLoad,
                                                 OnOffAirFlowRatio,
                                                 HXUnitOn);
                    if (furnace.CoolPartLoadRatio >= 1.0 || furnace.HeatPartLoadRatio >= 1.0 ||
                        (furnace.CoolPartLoadRatio <= 0.0 && furnace.HeatPartLoadRatio <= 0.0)) {
                        // compressor on (reset inlet air mass flow rate to starting value)
                        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = FurnaceSavMdot;
                        compressorOp = HVAC::CompressorOp::On;
                        CalcNewZoneHeatCoolFlowRates(state,
                                                     FurnaceNum,
                                                     FirstHVACIteration,
                                                     compressorOp,
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
                                                 compressorOp,
                                                 ZoneLoad,
                                                 MoistureLoad,
                                                 HeatCoilLoad,
                                                 ReheatCoilLoad,
                                                 OnOffAirFlowRatio,
                                                 HXUnitOn);
                }

                if (furnace.fanPlace == HVAC::FanPlace::BlowThru) {
                    // simulate fan
                    state.dataFans->fans(furnace.FanIndex)->simulate(state, FirstHVACIteration, state.dataFurnaces->FanSpeedRatio);
                }

                if (!furnace.CoolCoilUpstream) {
                    // simulate furnace heating coil
                    SuppHeatingCoilFlag = false; // if true simulates supplemental heating coil
                    CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, fanOp, QActual);
                }

                // simulate furnace DX cooling coil
                if (furnace.coolCoilType == HVAC::CoilType::CoolingDXHXAssisted) {
                    HXAssistCoil::SimHXAssistedCoolingCoil(state,
                                                           furnace.CoolCoilNum,
                                                           FirstHVACIteration,
                                                           compressorOp,
                                                           furnace.CoolPartLoadRatio,
                                                           fanOp,
                                                           HXUnitOn,
                                                           OnOffAirFlowRatio,
                                                           state.dataFurnaces->EconomizerFlag);
                } else {
                    DXCoils::SimDXCoil(state,
                                       furnace.CoolCoilNum,
                                       compressorOp,
                                       FirstHVACIteration,
                                       fanOp,
                                       furnace.CoolPartLoadRatio,
                                       OnOffAirFlowRatio,
                                       state.dataFurnaces->CoolHeatPLRRat);
                }

                if (furnace.CoolCoilUpstream) {
                    // simulate furnace heating coil
                    SuppHeatingCoilFlag = false; // if true simulates supplemental heating coil
                    CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, fanOp, QActual);
                }

                if (furnace.fanPlace == HVAC::FanPlace::DrawThru) {
                    // simulate fan
                    state.dataFans->fans(furnace.FanIndex)->simulate(state, FirstHVACIteration, state.dataFurnaces->FanSpeedRatio);
                }

                // Simulate furnace reheat coil if a humidistat is used or if the reheat coil is present
                if (furnace.DehumidControlType_Num == DehumidificationControlMode::CoolReheat || furnace.SuppHeatCoilNum > 0) {
                    SuppHeatingCoilFlag = true; // if true simulates supplemental heating coil
                    CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, ReheatCoilLoad, fanOp, QActual);
                }
            }
        } break;
            // Simulate air-to-air heat pumps:
        case HVAC::UnitarySysType::Unitary_HeatPump_AirToAir: {
            if (furnace.heatCoilType == HVAC::CoilType::HeatingDXVariableSpeed) {
                // variable speed heat pump
                HeatCoilLoad = 0.0;
                if (furnace.isIHP) {
                    auto &integratedHP = state.dataIntegratedHP->IntegratedHeatPumps(furnace.CoolCoilNum);
                    integratedHP.ControlledZoneTemp = state.dataLoopNodes->Node(furnace.NodeNumOfControlledZone).Temp;
                    integratedHP.IDFanID = furnace.FanIndex; // why do this every time?
                    integratedHP.IDFanName = BlankString;
                    integratedHP.fanPlace = furnace.fanPlace;
                }

                SimVariableSpeedHP(state, FurnaceNum, FirstHVACIteration, AirLoopNum, ZoneLoad, MoistureLoad, OnOffAirFlowRatio);
            } else {
                // Update the furnace flow rates
                if (!FirstHVACIteration && furnace.fanOp == HVAC::FanOp::Cycling && state.dataFurnaces->CoolingLoad &&
                    state.dataAirLoop->AirLoopControlInfo(AirLoopNum).EconoActive) {
                    // for cycling fan, cooling load, check whether furnace can meet load with compressor off
                    compressorOp = HVAC::CompressorOp::Off;
                    CalcNewZoneHeatCoolFlowRates(state,
                                                 FurnaceNum,
                                                 FirstHVACIteration,
                                                 compressorOp,
                                                 ZoneLoad,
                                                 MoistureLoad,
                                                 HeatCoilLoad,
                                                 ReheatCoilLoad,
                                                 OnOffAirFlowRatio,
                                                 HXUnitOn);
                    if (furnace.CoolPartLoadRatio >= 1.0 || furnace.HeatPartLoadRatio >= 1.0 ||
                        (furnace.CoolPartLoadRatio <= 0.0 && furnace.HeatPartLoadRatio <= 0.0)) {
                        // compressor on (reset inlet air mass flow rate to starting value)
                        compressorOp = HVAC::CompressorOp::On;
                        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = FurnaceSavMdot;
                        CalcNewZoneHeatCoolFlowRates(state,
                                                     FurnaceNum,
                                                     FirstHVACIteration,
                                                     compressorOp,
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
                                                 compressorOp,
                                                 ZoneLoad,
                                                 MoistureLoad,
                                                 HeatCoilLoad,
                                                 ReheatCoilLoad,
                                                 OnOffAirFlowRatio,
                                                 HXUnitOn);
                }

                if (furnace.fanPlace == HVAC::FanPlace::BlowThru) {
                    state.dataFans->fans(furnace.FanIndex)->simulate(state, FirstHVACIteration, state.dataFurnaces->FanSpeedRatio);
                }

                if (furnace.coolCoilType == HVAC::CoilType::CoolingDXHXAssisted) {
                    HXAssistCoil::SimHXAssistedCoolingCoil(state,
                                                                        BlankString,
                                                                        FirstHVACIteration,
                                                                        compressorOp,
                                                                        furnace.CoolPartLoadRatio,
                                                                        furnace.CoolCoilNum,
                                                                        fanOp,
                                                                        HXUnitOn,
                                                                        OnOffAirFlowRatio,
                                                                        state.dataFurnaces->EconomizerFlag);
                } else {
                    DXCoils::SimDXCoil(state,
                                       BlankString,
                                       compressorOp,
                                       FirstHVACIteration,
                                       furnace.CoolCoilNum,
                                       fanOp,
                                       furnace.CoolPartLoadRatio,
                                       OnOffAirFlowRatio);
                }
                DXCoils::SimDXCoil(state,
                                   BlankString,
                                   compressorOp,
                                   FirstHVACIteration,
                                   furnace.HeatCoilNum,
                                   fanOp,
                                   furnace.HeatPartLoadRatio,
                                   OnOffAirFlowRatio);
                if (furnace.fanPlace == HVAC::FanPlace::DrawThru) {
                    state.dataFans->fans(furnace.FanIndex)->simulate(state, FirstHVACIteration, state.dataFurnaces->FanSpeedRatio);
                }

                // Simulate furnace reheat coil if a humidistat is present, the dehumidification type of coolreheat and
                // reheat coil load exists
                if (furnace.DehumidControlType_Num == DehumidificationControlMode::CoolReheat && ReheatCoilLoad > 0.0) {
                    SuppHeatingCoilFlag = true; // if true simulates supplemental heating coil
                    CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, ReheatCoilLoad, fanOp, QActual);
                } else {
                    SuppHeatingCoilFlag = true; // if true simulates supplemental heating coil
                    CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, fanOp, QActual);
                }
            }
        } break;
        // Simulate water-to-air systems:
        case HVAC::UnitarySysType::Unitary_HeatPump_WaterToAir: {
            if (furnace.WatertoAirHPType == WAHPCoilType::Simple) {
                // Update the furnace flow rates
                //   When CompressorOp logic is added to the child cooling coil (COIL:WaterToAirHP:EquationFit:Cooling), then this logic
                //   needs to be reinstated... to align with Unitary/Furnace HeatCool and Unitary Air-to-Air Heat Pump (see above).
                if (!FirstHVACIteration && furnace.fanOp == HVAC::FanOp::Cycling && state.dataFurnaces->CoolingLoad &&
                    state.dataAirLoop->AirLoopControlInfo(AirLoopNum).EconoActive) {
                    // for cycling fan, cooling load, check whether furnace can meet load with compressor off
                    compressorOp = HVAC::CompressorOp::Off;
                    CalcNewZoneHeatCoolFlowRates(state,
                                                 FurnaceNum,
                                                 FirstHVACIteration,
                                                 compressorOp,
                                                 ZoneLoad,
                                                 MoistureLoad,
                                                 HeatCoilLoad,
                                                 ReheatCoilLoad,
                                                 OnOffAirFlowRatio,
                                                 HXUnitOn);
                    if (furnace.CoolPartLoadRatio >= 1.0 || furnace.HeatPartLoadRatio >= 1.0 ||
                        (furnace.CoolPartLoadRatio <= 0.0 && furnace.HeatPartLoadRatio <= 0.0)) {
                        // compressor on (reset inlet air mass flow rate to starting value)
                        compressorOp = HVAC::CompressorOp::On;
                        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = FurnaceSavMdot;
                        CalcNewZoneHeatCoolFlowRates(state,
                                                     FurnaceNum,
                                                     FirstHVACIteration,
                                                     compressorOp,
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
                                                 compressorOp,
                                                 ZoneLoad,
                                                 MoistureLoad,
                                                 HeatCoilLoad,
                                                 ReheatCoilLoad,
                                                 OnOffAirFlowRatio,
                                                 HXUnitOn);
                }
                if (furnace.fanPlace == HVAC::FanPlace::BlowThru) {
                    state.dataFans->fans(furnace.FanIndex)->simulate(state, FirstHVACIteration, state.dataFurnaces->FanSpeedRatio);
                }

                WaterToAirHeatPumpSimple::SimWatertoAirHPSimple(state,
                                                                BlankString,
                                                                furnace.CoolCoilNum,
                                                                furnace.CoolingCoilSensDemand,
                                                                furnace.CoolingCoilLatentDemand,
                                                                furnace.fanOp,
                                                                compressorOp,
                                                                furnace.CoolPartLoadRatio,
                                                                FirstHVACIteration);
                WaterToAirHeatPumpSimple::SimWatertoAirHPSimple(state,
                                                                BlankString,
                                                                furnace.HeatCoilNum,
                                                                furnace.HeatingCoilSensDemand,
                                                                Dummy,
                                                                furnace.fanOp,
                                                                compressorOp,
                                                                furnace.HeatPartLoadRatio,
                                                                FirstHVACIteration);

                if (furnace.fanPlace == HVAC::FanPlace::DrawThru) {
                    state.dataFans->fans(furnace.FanIndex)->simulate(state, FirstHVACIteration, state.dataFurnaces->FanSpeedRatio);
                }
                if (furnace.DehumidControlType_Num == DehumidificationControlMode::CoolReheat && ReheatCoilLoad > 0.0) {
                    SuppHeatingCoilFlag = true; // if true simulates supplemental heating coil
                    CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, ReheatCoilLoad, fanOp, QActual);
                } else {
                    SuppHeatingCoilFlag = true; // if true simulates supplemental heating coil
                    CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, fanOp, QActual);
                }
            } else if (furnace.WatertoAirHPType == WAHPCoilType::ParEst) {

                // simulate the heat pump
                HeatCoilLoad = 0.0;
                CalcWaterToAirHeatPump(state, FurnaceNum, FirstHVACIteration, compressorOp, ZoneLoad, MoistureLoad);
            } else if (furnace.WatertoAirHPType == WAHPCoilType::VarSpeedEquationFit) {
                // simulate the heat pump
                HeatCoilLoad = 0.0;
                if (furnace.isIHP)
                    state.dataIntegratedHP->IntegratedHeatPumps(furnace.CoolCoilNum).ControlledZoneTemp =
                        state.dataLoopNodes->Node(furnace.NodeNumOfControlledZone).Temp;
                SimVariableSpeedHP(state, FurnaceNum, FirstHVACIteration, AirLoopNum, ZoneLoad, MoistureLoad, OnOffAirFlowRatio);

            } else if (furnace.WatertoAirHPType == WAHPCoilType::VarSpeedLookupTable) {
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
        auto &airLoopControlInfo = state.dataAirLoop->AirLoopControlInfo(AirLoopNum);
        if (furnace.CompPartLoadRatio > 0.0 && airLoopControlInfo.CanLockoutEconoWithCompressor) {
            airLoopControlInfo.ReqstEconoLockoutWithCompressor = true;
        } else {
            airLoopControlInfo.ReqstEconoLockoutWithCompressor = false;
        }

        if ((HeatCoilLoad > 0.0 || furnace.HeatPartLoadRatio > 0.0) &&
            (airLoopControlInfo.CanLockoutEconoWithCompressor || airLoopControlInfo.CanLockoutEconoWithHeating)) {
            airLoopControlInfo.ReqstEconoLockoutWithHeating = true;
        } else {
            airLoopControlInfo.ReqstEconoLockoutWithHeating = false;
        }

        if (furnace.fanOp == HVAC::FanOp::Cycling) {
            state.dataAirLoop->AirLoopFlow(AirLoopNum).FanPLR = furnace.FanPartLoadRatio;
        } else {
            state.dataAirLoop->AirLoopFlow(AirLoopNum).FanPLR = 1.0; // 1 means constant fan does not cycle.
        }

        // Report the current Furnace output
        ReportFurnace(state, FurnaceNum, AirLoopNum);

        // Get the actual maximum RTF for AFN simulations
        if (state.afn->distribution_simulated) {
            Real64 heatingCoilRTF = 0.0;
            Real64 suppHeatingCoilRTF = 0.0;
            switch (furnace.heatCoilType) {
            case HVAC::CoilType::HeatingGasOrOtherFuel:
            case HVAC::CoilType::HeatingElectric:
            case HVAC::CoilType::HeatingDesuperheater: {
                int heatingCoilIndex = HeatingCoils::GetCoilIndex(state, furnace.HeatCoilName);
                if (heatingCoilIndex == 0) {
                    ShowSevereError(state, format("The index of \"{}\" is not found", furnace.HeatCoilName));
                    ShowContinueError(state, format("...occurs for {}", furnace.Name));
                } else {
                    heatingCoilRTF = state.dataHeatingCoils->HeatingCoil(heatingCoilIndex).RTF;
                }
            } break;
            default:;
            }

            switch (furnace.suppHeatCoilType) {
            case HVAC::CoilType::HeatingGasOrOtherFuel:
            case HVAC::CoilType::HeatingElectric:
            case HVAC::CoilType::HeatingDesuperheater: {
              int suppHeatingCoilIndex = HeatingCoils::GetCoilIndex(state, furnace.SuppHeatCoilName);
                if (suppHeatingCoilIndex == 0) {
                    ShowSevereError(state, format("The index of \"{}\" is not found", furnace.SuppHeatCoilName));
                    ShowContinueError(state, format("...occurs for {}", furnace.Name));
                } else {
                    suppHeatingCoilRTF = state.dataHeatingCoils->HeatingCoil(suppHeatingCoilIndex).RTF;
                }
            } break;
            default:;
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

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for fans and coils and stores it in the Furnace data structures

        // METHODOLOGY EMPLOYED:
        // Uses "Get" routines to read in data.

        // SUBROUTINE PARAMETER DEFINITIONS:
        std::string_view constexpr getUnitaryHeatOnly("GetUnitaryHeatOnly");
        std::string_view constexpr getAirLoopHVACHeatCoolInput("GetAirLoopHVACHeatCoolInput");
        std::string_view constexpr routineName = "GetFurnaceInput";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int FurnaceNum;                // The Furnace that you are currently loading input into
        int GetObjectNum;              // The index to each specific object name
        int NumFields;                 // Total number of fields in object
        int NumAlphas;                 // Total number of alpha fields in object
        int NumNumbers;                // Total number of numeric fields in object
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
        bool AirNodeFound;             // Used to determine if control zone is valid
        bool AirLoopFound;             // Used to determine if control zone is served by furnace air loop
        int TstatZoneNum;              // Used to determine if control zone has a thermostat object
        int HStatZoneNum;              // Used to determine if control zone has a humidistat object
        bool errFlag;                  // Mining function error flag
        std::string FanName;           // Used in mining function CALLS
        auto &cCurrentModuleObject = state.dataIPShortCut->cCurrentModuleObject;
        DataLoopNode::ConnectionObjectType currentModuleObjectType;

        auto &s_node = state.dataLoopNodes;
          
        state.dataFurnaces->GetFurnaceInputFlag = false;
        int MaxNumbers = 0;
        int MaxAlphas = 0;

        std::string_view CurrentModuleObject = "AirLoopHVAC:Unitary:Furnace:HeatOnly";
        int NumHeatOnly = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumFields, NumAlphas, NumNumbers);
        MaxNumbers = max(MaxNumbers, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);

        CurrentModuleObject = "AirLoopHVAC:Unitary:Furnace:HeatCool";
        int NumHeatCool = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumFields, NumAlphas, NumNumbers);
        MaxNumbers = max(MaxNumbers, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);

        CurrentModuleObject = "AirLoopHVAC:UnitaryHeatOnly";
        int NumUnitaryHeatOnly = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumFields, NumAlphas, NumNumbers);
        MaxNumbers = max(MaxNumbers, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);

        CurrentModuleObject = "AirLoopHVAC:UnitaryHeatCool";
        int NumUnitaryHeatCool = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumFields, NumAlphas, NumNumbers);
        MaxNumbers = max(MaxNumbers, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);

        CurrentModuleObject = "AirLoopHVAC:UnitaryHeatPump:AirToAir";
        int NumHeatPump = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumFields, NumAlphas, NumNumbers);
        MaxNumbers = max(MaxNumbers, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);

        CurrentModuleObject = "AirLoopHVAC:UnitaryHeatPump:WaterToAir";
        int NumWaterToAirHeatPump = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
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
            state.dataFurnaces->Furnaces.allocate(state.dataFurnaces->NumFurnaces);
            state.dataFurnaces->UniqueFurnaceNames.reserve(state.dataFurnaces->NumFurnaces);
        }
        state.dataFurnaces->CheckEquipName.dimension(state.dataFurnaces->NumFurnaces, true);

        // Get the data for the HeatOnly Furnace
        for (int HeatOnlyNum = 1; HeatOnlyNum <= NumHeatOnly + NumUnitaryHeatOnly; ++HeatOnlyNum) {

            FurnaceNum = HeatOnlyNum;
            auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);

            //       Furnace and UnitarySystem objects are both read in here.
            //       Will still have 2 differently named objects for the user, but read in with 1 DO loop.
            if (HeatOnlyNum <= NumHeatOnly) {
                CurrentModuleObject = "AirLoopHVAC:Unitary:Furnace:HeatOnly";
                currentModuleObjectType = DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryFurnaceHeatOnly;
                furnace.type = HVAC::UnitarySysType::Furnace_HeatOnly;
                GetObjectNum = HeatOnlyNum;
            } else {
                CurrentModuleObject = "AirLoopHVAC:UnitaryHeatOnly";
                currentModuleObjectType = DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatOnly;
                furnace.type = HVAC::UnitarySysType::Unitary_HeatOnly;
                GetObjectNum = HeatOnlyNum - NumHeatOnly;
            }

            furnace.iterationMode.allocate(3);

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

            furnace.Name = Alphas(1);
            ErrorObjectHeader eoh{routineName, cAlphaFields(1), furnace.Name};

            if (lAlphaBlanks(2)) {
                furnace.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((furnace.availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ErrorsFound = true;
            }

            furnace.FurnaceInletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                  Alphas(3),
                                                                                  ErrorsFound,
                                                                                  currentModuleObjectType,
                                                                                  Alphas(1),
                                                                                  DataLoopNode::NodeFluidType::Air,
                                                                                  DataLoopNode::ConnectionType::Inlet,
                                                                                  NodeInputManager::CompFluidStream::Primary,
                                                                                  DataLoopNode::ObjectIsParent);
            furnace.FurnaceOutletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                   Alphas(4),
                                                                                   ErrorsFound,
                                                                                   currentModuleObjectType,
                                                                                   Alphas(1),
                                                                                   DataLoopNode::NodeFluidType::Air,
                                                                                   DataLoopNode::ConnectionType::Outlet,
                                                                                   NodeInputManager::CompFluidStream::Primary,
                                                                                   DataLoopNode::ObjectIsParent);

            BranchNodeConnections::TestCompSet(state, CurrentModuleObject, Alphas(1), Alphas(3), Alphas(4), "Air Nodes");

            if (lAlphaBlanks(5)) {
                furnace.fanOp = HVAC::FanOp::Cycling;
            } else if ((furnace.fanOpModeSched = Sched::GetSchedule(state, Alphas(5))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(5), Alphas(5));
                ErrorsFound = true;
            }

            // Get the Controlling Zone or Location of the Furnace Thermostat

            furnace.ControlZoneNum = Util::FindItemInList(Alphas(6), state.dataHeatBal->Zone);
            if (furnace.ControlZoneNum == 0) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(6), Alphas(6)));
                ErrorsFound = true;
            }

            // Get the node number for the zone with the thermostat
            if (furnace.ControlZoneNum > 0) {
                AirNodeFound = false;
                AirLoopFound = false;
                int ControlledZoneNum = furnace.ControlZoneNum;
                //             Find the controlled zone number for the specified thermostat location
                furnace.NodeNumOfControlledZone = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ZoneNode;
                //             Determine if furnace is on air loop served by the thermostat location specified
                for (int zoneInNode = 1; zoneInNode <= state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).NumInletNodes; ++zoneInNode) {
                    int AirLoopNumber = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNodeAirLoopNum(zoneInNode);
                    furnace.airloopNum = AirLoopNumber;
                    if (AirLoopNumber > 0) {
                        for (int BranchNum = 1; BranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).NumBranches; ++BranchNum) {
                            for (int CompNum = 1;
                                 CompNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).TotalComponents;
                                 ++CompNum) {
                                if (!Util::SameString(state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).Comp(CompNum).Name,
                                                      furnace.Name) ||
                                    !Util::SameString(
                                        state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).Comp(CompNum).TypeOf,
                                        CurrentModuleObject)) {
                                    continue;
                                }
                                AirLoopFound = true;
                                furnace.ZoneInletNode = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNode(zoneInNode);
                                break;
                            }
                            if (AirLoopFound) {
                                break;
                            }
                        }
                        for (TstatZoneNum = 1; TstatZoneNum <= state.dataZoneCtrls->NumTempControlledZones; ++TstatZoneNum) {
                            if (state.dataZoneCtrls->TempControlledZone(TstatZoneNum).ActualZoneNum != furnace.ControlZoneNum) continue;
                            AirNodeFound = true;
                        }
                        for (TstatZoneNum = 1; TstatZoneNum <= state.dataZoneCtrls->NumComfortControlledZones; ++TstatZoneNum) {
                            if (state.dataZoneCtrls->ComfortControlledZone(TstatZoneNum).ActualZoneNum != furnace.ControlZoneNum) continue;
                            AirNodeFound = true;
                        }
                    }
                    if (AirLoopFound) {
                        break;
                    }
                }
                if (!AirNodeFound) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, "Did not find Air Node (Zone with Thermostat).");
                    ShowContinueError(state, format("Specified {} = {}", cAlphaFields(6), Alphas(6)));
                    ShowContinueError(
                        state, "Both a ZoneHVAC:EquipmentConnections object and a ZoneControl:Thermostat object must be specified for this zone.");
                    ErrorsFound = true;
                }
                if (!AirLoopFound) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, "Did not find correct Primary Air Loop.");
                    ShowContinueError(state, format("Specified {} = {} is not served by this AirLoopHVAC equipment.", cAlphaFields(6), Alphas(6)));
                    ErrorsFound = true;
                }
            }

            // Get fan data
            FanName = Alphas(8);
            errFlag = false;

            furnace.fanType = static_cast<HVAC::FanType>(getEnumValue(HVAC::fanTypeNamesUC, Alphas(7)));
            if (furnace.fanType != HVAC::FanType::OnOff && furnace.fanType != HVAC::FanType::Constant) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(7), Alphas(7)));
                ErrorsFound = true;

            } else if ((furnace.FanIndex = Fans::GetFanIndex(state, FanName)) == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(8), FanName);
                ErrorsFound = true;

            } else {
                auto *fan = state.dataFans->fans(furnace.FanIndex);
                furnace.ActualFanVolFlowRate = fan->maxAirFlowRate;
                furnace.FanInletNode = fan->inletNodeNum;
                furnace.FanOutletNode = fan->outletNodeNum;
                furnace.fanAvailSched = fan->availSched;

                // Check fan's schedule for cycling fan operation if constant volume fan is used
                if (furnace.fanOpModeSched != nullptr && furnace.fanType == HVAC::FanType::Constant) {
                    if (!furnace.fanOpModeSched->checkMinMaxVals(state, Clusive::Ex, 0.0, Clusive::In, 1.0)) {
                        Sched::ShowSevereBadMinMax(
                            state,
                            eoh,
                            cAlphaFields(5),
                            Alphas(5),
                            Clusive::Ex,
                            0.0,
                            Clusive::In,
                            1.0,
                            format("For {} = {}, Fan operating mode must be continuous (schedule values > 0)", cAlphaFields(7), Alphas(7)));

                        ErrorsFound = true;
                    }
                } else if (lAlphaBlanks(5) && furnace.fanType != HVAC::FanType::OnOff) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, furnace.Name));
                    ShowContinueError(state, format("{} = {}", cAlphaFields(7), Alphas(7)));
                    ShowContinueError(state, format("Fan type must be Fan:OnOff when {} = Blank.", cAlphaFields(5)));
                    ErrorsFound = true;
                }
            }

            furnace.fanPlace = static_cast<HVAC::FanPlace>(getEnumValue(HVAC::fanPlaceNamesUC, Alphas(9)));
            assert(furnace.fanPlace != HVAC::FanPlace::Invalid);

            // Get coil data
            furnace.HeatCoilName = Alphas(11);
            furnace.heatCoilType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Alphas(10)));
            
            if (furnace.heatCoilType == HVAC::CoilType::HeatingGasOrOtherFuel ||
                furnace.heatCoilType == HVAC::CoilType::HeatingElectric) {

                // Get index to Heating Coil
                furnace.HeatCoilNum = HeatingCoils::GetCoilIndex(state, furnace.HeatCoilName);
                if (furnace.HeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(11), Alphas(11));
                    ErrorsFound = true;
                } else {
                    furnace.DesignHeatingCapacity = HeatingCoils::GetCoilCapacity(state, furnace.HeatCoilNum);
                    furnace.HeatCoilAirInletNode = HeatingCoils::GetCoilAirInletNode(state, furnace.HeatCoilNum);
                    furnace.HeatCoilAirOutletNode = HeatingCoils::GetCoilAirOutletNode(state, furnace.HeatCoilNum);

                    assert(furnace.HeatCoilAirInletNode != 0 && furnace.HeatCoilAirOutletNode != 0);
                }
            } else if (furnace.heatCoilType == HVAC::CoilType::HeatingWater) {

                furnace.HeatCoilNum = WaterCoils::GetCoilIndex(state, furnace.HeatCoilName);
                if (furnace.HeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(11), Alphas(11));
                    ErrorsFound = true;
                } else {
                    furnace.HeatCoilControlNode = WaterCoils::GetCoilWaterInletNode(state, furnace.HeatCoilNum);
                    furnace.MaxHeatCoilFluidFlow = WaterCoils::GetCoilMaxWaterFlowRate(state, furnace.HeatCoilNum);
                    furnace.HeatCoilAirInletNode = WaterCoils::GetCoilAirInletNode(state, furnace.HeatCoilNum);
                    furnace.HeatCoilAirOutletNode = WaterCoils::GetCoilAirOutletNode(state, furnace.HeatCoilNum);
                    
                    HVACControllers::CheckCoilWaterInletNode(state, furnace.HeatCoilControlNode, errFlag);
                    if (!errFlag) { // then did find a controller so that is bad
                        ShowSevereError(state,
                                        format("{} = {} has a conflicting Controller:WaterCoil object", CurrentModuleObject, furnace.Name));
                        ShowContinueError(state, "Hot water coils are controlled directly by unitary and furnace systems.");
                        ShowContinueError(state, "No water coil controller should be input for the coil.");
                        ErrorsFound = true;
                    }
                }

            } else if (furnace.heatCoilType == HVAC::CoilType::HeatingSteam) {
                furnace.HeatCoilNum = SteamCoils::GetCoilIndex(state, furnace.HeatCoilName);
                if (furnace.HeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(11), furnace.HeatCoilName);
                    ErrorsFound = true;
                } else {
                    furnace.HeatCoilControlNode = SteamCoils::GetCoilSteamInletNode(state, furnace.HeatCoilNum);
                    furnace.MaxHeatCoilFluidFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, furnace.HeatCoilNum);
                    if (furnace.MaxHeatCoilFluidFlow > 0.0) {
                        furnace.MaxHeatCoilFluidFlow *= Fluid::GetSteam(state)->getSatDensity(state, 100.0, 1.0, getUnitaryHeatOnly);
                    }

                    furnace.HeatCoilAirInletNode = SteamCoils::GetCoilAirInletNode(state, furnace.HeatCoilNum);
                    furnace.HeatCoilAirOutletNode = SteamCoils::GetCoilAirOutletNode(state, furnace.HeatCoilNum);
                }
            } else {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(11), Alphas(11));
                ErrorsFound = true;
            } // IF (Furnace(FurnaceNum)%HeatingCoilType_Num == Coil_HeatingGasOrOtherFuel .OR. &, etc.

            // Add component sets array
            if (furnace.fanPlace == HVAC::FanPlace::BlowThru) {
                CompSetFanInlet = Alphas(3);
                CompSetFanOutlet = s_node->NodeID(furnace.FanOutletNode);
                CompSetHeatInlet = s_node->NodeID(furnace.FanOutletNode);
                CompSetHeatOutlet = Alphas(4);
                // Fan inlet node name must not be the same as the furnace inlet node name
                if (furnace.FanInletNode != furnace.FurnaceInletNode) {
                    if (furnace.type == HVAC::UnitarySysType::Furnace_HeatOnly) {
                        ShowSevereCustom(state, eoh, 
                                         "When a blow through fan is specified, the fan inlet node name must be the same as the furnace inlet node name.");
                        ShowContinueError(state, format("...Fan inlet node name     = {}", s_node->NodeID(furnace.FanInletNode)));
                        ShowContinueError(state, format("...Furnace inlet node name = {}", s_node->NodeID(furnace.FurnaceInletNode)));
                    } else {
                        ShowSevereCustom(state, eoh, 
                                         "When a blow through fan is specified, the fan inlet node name must be the same as the unitary system inlet node name.");
                        ShowContinueError(state, format("...Fan inlet node name            = {}", s_node->NodeID(furnace.FanInletNode)));
                        ShowContinueError(state, format("...Unitary System inlet node name = {}", s_node->NodeID(furnace.FurnaceInletNode)));
                    }
                    ErrorsFound = true;
                }
                // Fan outlet node name must be the same as the heating coil inlet node name
                if (furnace.FanOutletNode != furnace.HeatCoilAirInletNode) {
                    ShowSevereCustom(state, eoh, 
                                    "When a blow through fan is specified, the fan outlet node name must be the same as the heating coil inlet node name.");
                    ShowContinueError(state, format("...Fan outlet node name         = {}", s_node->NodeID(furnace.FanOutletNode)));
                    ShowContinueError(state, format("...Heating coil inlet node name = {}", s_node->NodeID(furnace.HeatCoilAirInletNode)));
                    ErrorsFound = true;
                }
                // Heating coil outlet node name must be the same as the furnace outlet node name
                if (furnace.HeatCoilAirOutletNode != furnace.FurnaceOutletNode) {
                    if (furnace.type == HVAC::UnitarySysType::Furnace_HeatOnly) {
                        ShowSevereCustom(state, eoh, 
                                         "When a blow through fan is specified, the heating coil outlet node name must be the same as the furnace "
                                         "outlet node name.");
                        ShowContinueError(state, format("...Heating coil outlet node name = {}", s_node->NodeID(furnace.HeatCoilAirOutletNode)));
                        ShowContinueError(state, format("...Furnace outlet node name      = {}", s_node->NodeID(furnace.FurnaceOutletNode)));
                    } else {
                        ShowSevereCustom(state, eoh, 
                                         "When a blow through fan is specified, the heating coil outlet node name must be the same as the unitary "
                                         "system outlet node name.");
                        ShowContinueError(state, format("...Heating coil outlet node name  = {}", s_node->NodeID(furnace.HeatCoilAirOutletNode)));
                        ShowContinueError(state, format("...UnitarySystem outlet node name = {}", s_node->NodeID(furnace.FurnaceOutletNode)));
                    }
                    ErrorsFound = true;
                }
            } else { // draw through fan
                CompSetHeatInlet = Alphas(3);
                CompSetHeatOutlet = s_node->NodeID(furnace.FanInletNode);
                CompSetFanInlet = s_node->NodeID(furnace.FanInletNode);
                CompSetFanOutlet = Alphas(4);
                // Heating coil inlet node name must not be the same as the furnace inlet node name
                if (furnace.HeatCoilAirInletNode != furnace.FurnaceInletNode) {
                    if (furnace.type == HVAC::UnitarySysType::Furnace_HeatOnly) {
                        ShowSevereCustom(state, eoh, 
                                         "When a draw through fan is specified, the heating coil inlet node name must be the same as the furnace "
                                         "inlet node name.");
                        ShowContinueError(state, format("...Heating coil inlet node name = {}", s_node->NodeID(furnace.HeatCoilAirInletNode)));
                        ShowContinueError(state, format("...Furnace inlet node name      = {}", s_node->NodeID(furnace.FurnaceInletNode)));
                    } else {
                        ShowSevereCustom(state, eoh,
                                         "When a draw through fan is specified, the heating coil inlet node name must be the same as the unitary "
                                         "system inlet node name.");
                        ShowContinueError(state, format("...Heating coil inlet node name  = {}", s_node->NodeID(furnace.HeatCoilAirInletNode)));
                        ShowContinueError(state, format("...UnitarySystem inlet node name = {}", s_node->NodeID(furnace.FurnaceInletNode)));
                    }
                    ErrorsFound = true;
                }
                // Heating coil outlet node name must be the same as the fan inlet node name
                if (furnace.HeatCoilAirOutletNode != furnace.FanInletNode) {
                    ShowSevereCustom(state, eoh, "When a draw through fan is specified, the heating coil outlet node name must be the same as the fan inlet node name.");
                    ShowContinueError(state, format("...Heating coil outlet node name = {}", s_node->NodeID(furnace.HeatCoilAirOutletNode)));
                    ShowContinueError(state, format("...Fan inlet node name           = {}", s_node->NodeID(furnace.FanInletNode)));
                    ErrorsFound = true;
                }
                // Fan coil outlet node name must be the same as the furnace outlet node name
                if (furnace.FanOutletNode != furnace.FurnaceOutletNode) {
                    if (furnace.type == HVAC::UnitarySysType::Furnace_HeatOnly) {
                        ShowSevereCustom(state, eoh, 
                                         "When a draw through fan is specified, the fan outlet node name must be the same as the furnace outlet node name.");
                        ShowContinueError(state, format("...Fan outlet node name     = {}", s_node->NodeID(furnace.FanOutletNode)));
                        ShowContinueError(state, format("...Furnace outlet node name = {}", s_node->NodeID(furnace.FurnaceOutletNode)));
                    } else {
                        ShowSevereCustom(state, eoh, 
                                         "When a draw through fan is specified, the fan outlet node name must be the same as the unitary system "
                                         "outlet node name.");
                        ShowContinueError(state, format("...Fan outlet node name           = {}", s_node->NodeID(furnace.FanOutletNode)));
                        ShowContinueError(state, format("...UnitarySystem outlet node name = {}", s_node->NodeID(furnace.FurnaceOutletNode)));
                    }
                    ErrorsFound = true;
                }
            }

            // Add fan to component sets array
            BranchNodeConnections::SetUpCompSets(
                state, CurrentModuleObject, furnace.Name, Alphas(7), Alphas(8), CompSetFanInlet, CompSetFanOutlet);
            // Add heating coil to component sets array
            BranchNodeConnections::SetUpCompSets(
                state, CurrentModuleObject, furnace.Name, Alphas(10), Alphas(11), CompSetHeatInlet, CompSetHeatOutlet);

            // Set the furnace max outlet temperature
            furnace.DesignMaxOutletTemp = Numbers(1);

            // Set the furnace design fan volumetric flow rate
            furnace.DesignFanVolFlowRate = Numbers(2);

            // Compare the flow rates.
            if (furnace.ActualFanVolFlowRate != DataSizing::AutoSize && furnace.DesignFanVolFlowRate != DataSizing::AutoSize) {
                if (furnace.DesignFanVolFlowRate > furnace.ActualFanVolFlowRate) {
                    ShowWarningError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(
                        state, format("... The {} > Max Volume Flow Rate defined in the associated fan object, should be <=.", cNumericFields(2)));
                    ShowContinueError(state,
                                      format("... Entered value = {:.4R}... Fan [{} = {}] Max Value = {:.4R}",
                                             furnace.DesignFanVolFlowRate,
                                             HVAC::fanTypeNames[(int)furnace.fanType],
                                             FanName,
                                             furnace.ActualFanVolFlowRate));
                    ShowContinueError(state, " The HVAC system  flow rate is reset to the fan flow rate and the simulation continues.");
                    furnace.DesignFanVolFlowRate = furnace.ActualFanVolFlowRate;
                }
            }
            if (furnace.DesignFanVolFlowRate != DataSizing::AutoSize) {
                if (furnace.DesignFanVolFlowRate <= 0.0) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("... The {} <= 0.0, it must be > 0.0.", cNumericFields(2)));
                    ShowContinueError(state, format("... Entered value = {:.2R}", furnace.DesignFanVolFlowRate));
                    ErrorsFound = true;
                }
            }

            //       HeatOnly furnace has only 1 flow rate, initialize other variables used in this module
            furnace.MaxHeatAirVolFlow = furnace.DesignFanVolFlowRate;
            furnace.MaxCoolAirVolFlow = furnace.DesignFanVolFlowRate;
            furnace.MaxNoCoolHeatAirVolFlow = furnace.DesignFanVolFlowRate;
            furnace.AirFlowControl = AirFlowControlConstFan::UseCompressorOnFlow;

            // Set heating convergence tolerance
            furnace.HeatingConvergenceTolerance = 0.001;

            // set minimum outdoor temperature for compressor operation
            SetMinOATCompressor(state, FurnaceNum, cCurrentModuleObject, ErrorsFound);

        } // End of the HeatOnly Furnace Loop

        // Get the data for the HeatCool Furnace or UnitarySystem
        for (int HeatCoolNum = 1; HeatCoolNum <= NumHeatCool + NumUnitaryHeatCool; ++HeatCoolNum) {

            FurnaceNum = HeatCoolNum + NumHeatOnly + NumUnitaryHeatOnly;
            auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);

            //      Furnace and UnitarySystem objects are both read in here.
            //      Will still have 2 differently named objects for the user, but read in with 1 DO loop.
            if (HeatCoolNum <= NumHeatCool) {
                CurrentModuleObject = "AirLoopHVAC:Unitary:Furnace:HeatCool";
                currentModuleObjectType = DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryFurnaceHeatCool;
                furnace.type = HVAC::UnitarySysType::Furnace_HeatCool;
                GetObjectNum = HeatCoolNum;
            } else {
                CurrentModuleObject = "AirLoopHVAC:UnitaryHeatCool";
                currentModuleObjectType = DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatCool;
                furnace.type = HVAC::UnitarySysType::Unitary_HeatCool;
                GetObjectNum = HeatCoolNum - NumHeatCool;
            }

            furnace.iterationMode.allocate(3);

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

            furnace.Name = Alphas(1);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, furnace.Name};

            if (lAlphaBlanks(2)) {
                furnace.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((furnace.availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ErrorsFound = true;
            }

            furnace.FurnaceInletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                  Alphas(3),
                                                                                  ErrorsFound,
                                                                                  currentModuleObjectType,
                                                                                  Alphas(1),
                                                                                  DataLoopNode::NodeFluidType::Air,
                                                                                  DataLoopNode::ConnectionType::Inlet,
                                                                                  NodeInputManager::CompFluidStream::Primary,
                                                                                  DataLoopNode::ObjectIsParent);
            furnace.FurnaceOutletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                   Alphas(4),
                                                                                   ErrorsFound,
                                                                                   currentModuleObjectType,
                                                                                   Alphas(1),
                                                                                   DataLoopNode::NodeFluidType::Air,
                                                                                   DataLoopNode::ConnectionType::Outlet,
                                                                                   NodeInputManager::CompFluidStream::Primary,
                                                                                   DataLoopNode::ObjectIsParent);

            BranchNodeConnections::TestCompSet(state, CurrentModuleObject, Alphas(1), Alphas(3), Alphas(4), "Air Nodes");

            if (lAlphaBlanks(5)) {
                furnace.fanOp = HVAC::FanOp::Cycling;
            } else if ((furnace.fanOpModeSched = Sched::GetSchedule(state, Alphas(5))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(5), Alphas(5));
                ErrorsFound = true;
            }

            // Get the Controlling Zone or Location of the Furnace Thermostat
            furnace.ControlZoneNum = Util::FindItemInList(Alphas(6), state.dataHeatBal->Zone);
            if (furnace.ControlZoneNum == 0) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(6), Alphas(6)));
                ErrorsFound = true;
            }

            // Get the node number for the zone with the thermostat
            if (furnace.ControlZoneNum > 0) {
                AirNodeFound = false;
                AirLoopFound = false;
                int ControlledZoneNum = furnace.ControlZoneNum;
                //             Find the controlled zone number for the specified thermostat location
                furnace.NodeNumOfControlledZone = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ZoneNode;
                //             Determine if system is on air loop served by the thermostat location specified
                for (int zoneInNode = 1; zoneInNode <= state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).NumInletNodes; ++zoneInNode) {
                    int AirLoopNumber = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNodeAirLoopNum(zoneInNode);
                    furnace.airloopNum = AirLoopNumber;
                    if (AirLoopNumber > 0) {
                        for (int BranchNum = 1; BranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).NumBranches; ++BranchNum) {
                            for (int CompNum = 1;
                                 CompNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).TotalComponents;
                                 ++CompNum) {
                                if (!Util::SameString(state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).Comp(CompNum).Name,
                                                      Alphas(1)) ||
                                    !Util::SameString(
                                        state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).Comp(CompNum).TypeOf,
                                        CurrentModuleObject)) {
                                    continue;
                                }
                                AirLoopFound = true;
                                furnace.ZoneInletNode = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNode(zoneInNode);
                                break;
                            }
                            if (AirLoopFound) {
                                break;
                            }
                        }
                        for (TstatZoneNum = 1; TstatZoneNum <= state.dataZoneCtrls->NumTempControlledZones; ++TstatZoneNum) {
                            if (state.dataZoneCtrls->TempControlledZone(TstatZoneNum).ActualZoneNum != furnace.ControlZoneNum) continue;
                            AirNodeFound = true;
                        }
                        for (TstatZoneNum = 1; TstatZoneNum <= state.dataZoneCtrls->NumComfortControlledZones; ++TstatZoneNum) {
                            if (state.dataZoneCtrls->ComfortControlledZone(TstatZoneNum).ActualZoneNum != furnace.ControlZoneNum) continue;
                            AirNodeFound = true;
                        }
                    }
                    if (AirLoopFound) {
                        break;
                    }
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
                    ShowContinueError(state, "Did not find correct AirLoopHVAC.");
                    ShowContinueError(state, format("Specified {} = {}", cAlphaFields(6), Alphas(6)));
                    ErrorsFound = true;
                }
            }

            // Get fan data
            FanName = Alphas(8);

            furnace.fanType = static_cast<HVAC::FanType>(getEnumValue(HVAC::fanTypeNamesUC, Alphas(7)));

            if (furnace.fanType != HVAC::FanType::OnOff && furnace.fanType != HVAC::FanType::Constant) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(7), Alphas(7)));
                ErrorsFound = true;

            } else if ((furnace.FanIndex = Fans::GetFanIndex(state, FanName)) == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(8), FanName);
                ErrorsFound = true;

            } else {
                auto *fan = state.dataFans->fans(furnace.FanIndex);
                furnace.ActualFanVolFlowRate = fan->maxAirFlowRate;
                furnace.FanInletNode = fan->inletNodeNum;
                furnace.FanOutletNode = fan->outletNodeNum;
                furnace.fanAvailSched = fan->availSched;

                // Check fan's schedule for cycling fan operation if constant volume fan is used
                if (furnace.fanOpModeSched != nullptr && furnace.fanType == HVAC::FanType::Constant) {
                    if (!furnace.fanOpModeSched->checkMinMaxVals(state, Clusive::Ex, 0.0, Clusive::In, 1.0)) {
                        Sched::ShowSevereBadMinMax( // MAKE THIS STOP!!!!!
                            state,
                            eoh,
                            cAlphaFields(5),
                            Alphas(5),
                            Clusive::In,
                            0.0,
                            Clusive::In,
                            1.0,
                            format("For {} = {}, fan operating mode must be continuous (schedule values > 0)", cAlphaFields(7), Alphas(7)));
                        ErrorsFound = true;
                    }
                } else if (lAlphaBlanks(5) && furnace.fanType != HVAC::FanType::OnOff) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, furnace.Name));
                    ShowContinueError(state, format("{} = {}", cAlphaFields(7), Alphas(7)));
                    ShowContinueError(state, format("Fan type must be Fan:OnOff when {} = Blank.", cAlphaFields(5)));
                    ErrorsFound = true;
                }
            }

            furnace.fanPlace = static_cast<HVAC::FanPlace>(getEnumValue(HVAC::fanPlaceNamesUC, Alphas(9)));
            assert(furnace.fanPlace != HVAC::FanPlace::Invalid);

            // Get coil data
            furnace.heatCoilType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Alphas(10)));
            furnace.HeatCoilName = Alphas(11);
            furnace.HeatCoilPLFCurveIndex = 0;
            
            if (furnace.heatCoilType == HVAC::CoilType::HeatingGasOrOtherFuel ||
                furnace.heatCoilType == HVAC::CoilType::HeatingElectric) {
              
                furnace.HeatCoilNum = HeatingCoils::GetCoilIndex(state, furnace.HeatCoilName);
                if (furnace.HeatCoilNum == 0) { 
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(11), Alphas(11));
                    ErrorsFound = true;
                } else {
                    furnace.DesignHeatingCapacity = HeatingCoils::GetCoilCapacity(state, furnace.HeatCoilNum);
                    furnace.HeatCoilAirInletNode = HeatingCoils::GetCoilAirInletNode(state, furnace.HeatCoilNum);
                    furnace.HeatCoilAirOutletNode = HeatingCoils::GetCoilAirOutletNode(state, furnace.HeatCoilNum);
                    furnace.HeatCoilPLFCurveIndex = HeatingCoils::GetCoilPLFCurveIndex(state, furnace.HeatCoilNum);
                }

            } else if (furnace.heatCoilType == HVAC::CoilType::HeatingWater) {
                furnace.HeatCoilNum = WaterCoils::GetCoilIndex(state, furnace.HeatCoilName);
                if (furnace.HeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(11), Alphas(11));
                    ErrorsFound = true;
                } else {
                    furnace.HeatCoilControlNode = WaterCoils::GetCoilWaterInletNode(state, furnace.HeatCoilNum);
                    furnace.MaxHeatCoilFluidFlow = WaterCoils::GetCoilMaxWaterFlowRate(state, furnace.HeatCoilNum);
                    furnace.HeatCoilAirInletNode = WaterCoils::GetCoilAirInletNode(state, furnace.HeatCoilNum);
                    furnace.HeatCoilAirOutletNode = WaterCoils::GetCoilAirOutletNode(state, furnace.HeatCoilNum);
                    // check if user has also used a water coil controller, which they should not do
                    errFlag = false;
                    HVACControllers::CheckCoilWaterInletNode(state, furnace.HeatCoilControlNode, errFlag);
                    if (!errFlag) { // then did find a controller so that is bad
                        ShowSevereError(state,
                                        format("{} = {} has a conflicting Controller:WaterCoil object", CurrentModuleObject, furnace.Name));
                        ShowContinueError(state, "Hot water coils are controlled directly by unitary and furnace systems.");
                        ShowContinueError(state, "No water coil controller should be input for the coil.");
                        ErrorsFound = true;
                    }
                }
                
            } else if (furnace.heatCoilType == HVAC::CoilType::HeatingSteam) {
                furnace.HeatCoilNum = SteamCoils::GetCoilIndex(state, furnace.HeatCoilName);
                if (furnace.HeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(11), furnace.HeatCoilName);
                    ErrorsFound = true;
                } else {
                    furnace.HeatCoilControlNode = SteamCoils::GetCoilSteamInletNode(state, furnace.HeatCoilNum);
                    furnace.MaxHeatCoilFluidFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, furnace.HeatCoilNum);
                    if (furnace.MaxHeatCoilFluidFlow > 0.0) {
                        furnace.MaxHeatCoilFluidFlow *= Fluid::GetSteam(state)->getSatDensity(state, 100.0, 1.0, getAirLoopHVACHeatCoolInput);
                    }

                    furnace.HeatCoilAirInletNode = SteamCoils::GetCoilAirInletNode(state, furnace.HeatCoilNum);
                    furnace.HeatCoilAirOutletNode = SteamCoils::GetCoilAirOutletNode(state, furnace.HeatCoilNum);
                }

            } else {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(11), Alphas(11));
                ErrorsFound = true;
            } // IF (Furnace(FurnaceNum)%HeatingCoilType_Num == Coil_HeatingGasOrOtherFuel .OR. &, etc.

            // Get Cooling Coil Information if available
            furnace.coolCoilType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Alphas(12)));
            furnace.CoolCoilName = Alphas(13);
            //       Find the type of coil. Do not print message since this may not be the correct coil type.
            if (Util::SameString(Alphas(12), "COILSYSTEM:INTEGRATEDHEATPUMP:AIRSOURCE")) {
                furnace.coolCoilType = HVAC::CoilType::CoolingDXVariableSpeed;
                furnace.isIHP = true;
            }
            
            if (furnace.coolCoilType == HVAC::CoilType::CoolingDXSingleSpeed) {

                furnace.CoolCoilNum = DXCoils::GetCoilIndex(state, furnace.CoolCoilName);
                if (furnace.CoolCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(13), furnace.CoolCoilName);
                    ErrorsFound = true;
                } else {
                    furnace.DesignCoolingCapacity = DXCoils::GetCoilCapacity(state, furnace.CoolCoilNum);
                    furnace.CoolCoilAirInletNode = DXCoils::GetCoilAirInletNode(state, furnace.CoolCoilNum);
                    furnace.CoolCoilAirOutletNode = DXCoils::GetCoilAirOutletNode(state, furnace.CoolCoilNum);
                }

            } else if (furnace.coolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
                if (furnace.isIHP) {
                    furnace.ihpName = furnace.CoolCoilName;
                    furnace.ihpNum = IntegratedHeatPump::GetIHPIndex(state, furnace.ihpName);
                    if (furnace.ihpNum == 0) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFields(13), furnace.ihpName);
                        ErrorsFound = true;
                    } else {
                        furnace.CoolCoilName = state.dataIntegratedHP->IntegratedHeatPumps(furnace.ihpNum).SCCoilName;
                        furnace.CoolCoilNum = VariableSpeedCoils::GetCoilIndex(state, furnace.CoolCoilName);
                        // This probably doesn't need to be done because the IHP itself did this check.
                        if (furnace.CoolCoilNum == 0) {
                            ShowSevereItemNotFound(state, eoh, format("{}.SCCoilName", cAlphaFields(13)), furnace.CoolCoilName);
                            ErrorsFound = true;
                        } else {
                            furnace.CondenserNodeNum = VariableSpeedCoils::GetCoilCondenserInletNode(state, furnace.CoolCoilNum);
                        }
                    }
                } else { // !furnace.isIHP
                    furnace.CoolCoilNum = VariableSpeedCoils::GetCoilIndex(state, furnace.CoolCoilName);
                    if (furnace.CoolCoilNum == 0) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFields(13), furnace.CoolCoilName);
                        ErrorsFound = true;
                    }
                }                  

                // At this point, CoolCoilNum should be set correctly regardless of whether this is an IHP or not
                if (furnace.CoolCoilNum != 0) {
                    furnace.CoolCoilAirInletNode = VariableSpeedCoils::GetCoilAirInletNode(state, furnace.CoolCoilNum);
                    furnace.CoolCoilAirOutletNode = VariableSpeedCoils::GetCoilAirOutletNode(state, furnace.CoolCoilNum);
                    furnace.CondenserNodeNum = VariableSpeedCoils::GetCoilCondenserInletNode(state, furnace.CoolCoilNum);
                    furnace.DesignCoolingCapacity = VariableSpeedCoils::GetCoilCapacity(state, furnace.CoolCoilNum);
                    furnace.MaxCoolAirVolFlow = VariableSpeedCoils::GetCoilAirFlowRate(state, furnace.CoolCoilNum);
                }
                
                // Push heating coil PLF curve index to DX coil
                if (furnace.HeatCoilPLFCurveIndex > 0) {
                    DXCoils::SetCoilHeatingPLFCurve(state, furnace.CoolCoilNum, furnace.HeatCoilPLFCurveIndex);
                }

            } else if (furnace.coolCoilType == HVAC::CoilType::CoolingDXHXAssisted) {
                // At this point, the variables that reference the
                // HXAssisted coil become the hxCoolCoil vars so that
                // the (plain) CoolCoil vars can reference the child DX Coil
                // that is embedded in the HXAssisted coil
                furnace.CoolCoilNum = HXAssistCoil::GetCoilIndex(state, furnace.CoolCoilName);
                if (furnace.CoolCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(13), furnace.CoolCoilName);
                    ErrorsFound = true;
                } else {
                    furnace.DesignCoolingCapacity = HXAssistCoil::GetCoilCapacity(state, furnace.CoolCoilNum);
                    furnace.CoolCoilAirInletNode = HXAssistCoil::GetCoilAirInletNode(state, furnace.CoolCoilNum);
                    furnace.CoolCoilAirOutletNode = HXAssistCoil::GetCoilAirOutletNode(state, furnace.CoolCoilNum);
                     
                    furnace.childCoolCoilName = HXAssistCoil::GetCoilChildCoilName(state, furnace.CoolCoilNum);
                    furnace.childCoolCoilType = HXAssistCoil::GetCoilChildCoilType(state, furnace.CoolCoilNum);
                    furnace.childCoolCoilNum = HXAssistCoil::GetCoilChildCoilIndex(state, furnace.CoolCoilNum);
                    
                    // if (furnace.CoolingCoilType_Num == CoilDX_CoolingHXAssisted) {
                    if (furnace.childCoolCoilType == HVAC::CoilType::CoolingDX) {
                        auto const &coilDX = state.dataCoilCoolingDX->coilCoolingDXs[furnace.childCoolCoilNum];
                        furnace.CondenserNodeNum = coilDX.condInletNodeIndex;
                    } else if (furnace.childCoolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
                        furnace.CondenserNodeNum = VariableSpeedCoils::GetCoilCondenserInletNode(state, furnace.childCoolCoilNum);
                    } else {
                        furnace.CondenserNodeNum = DXCoils::GetCoilCondenserInletNode(state, furnace.CoolCoilNum);
                    }

                    // Push heating coil PLF curve index to DX coil
                    if (furnace.HeatCoilPLFCurveIndex > 0) {
                        // get the actual index to the DX cooling coil object
                        if (furnace.childCoolCoilType == HVAC::CoilType::CoolingDXSingleSpeed) {
                            DXCoils::SetCoilHeatingPLFCurve(state, furnace.childCoolCoilNum, furnace.HeatCoilPLFCurveIndex);
                        }
                        // what could we do for VS coil here? odd thing here
                    }
                 }

            } else {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(12), Alphas(12));
                ErrorsFound = true;
            }

            if (Util::SameString(Alphas(14), "None") || Util::SameString(Alphas(14), "Multimode") || Util::SameString(Alphas(14), "CoolReheat")) {
                AirNodeFound = false;
                if (Util::SameString(Alphas(14), "Multimode")) {
                    furnace.DehumidControlType_Num = DehumidificationControlMode::Multimode;
                    furnace.Humidistat = true;
                    if (furnace.coolCoilType != HVAC::CoilType::CoolingDXHXAssisted) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(14), Alphas(14)));
                        ShowContinueError(state, "Multimode control must be used with a Heat Exchanger Assisted Cooling Coil.");
                        if (lAlphaBlanks(15)) {
                            ShowContinueError(state,
                                              "Dehumidification control type is assumed to be None since a reheat coil has not been specified and "
                                              "the simulation continues.");
                            furnace.Humidistat = false;
                            furnace.DehumidControlType_Num = DehumidificationControlMode::None;
                        } else {
                            ShowContinueError(state, "Dehumidification control type is assumed to be CoolReheat and the simulation continues.");
                            furnace.DehumidControlType_Num = DehumidificationControlMode::CoolReheat;
                        }
                    }
                } else if (Util::SameString(Alphas(14), "CoolReheat")) {
                    furnace.DehumidControlType_Num = DehumidificationControlMode::CoolReheat;
                    furnace.Humidistat = true;
                    if (lAlphaBlanks(15)) {
                        ShowWarningError(state, format("{} \"{}\"", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state,
                                          "Dehumidification control type is assumed to be None since a reheat coil has not been specified and the "
                                          "simulation continues.");
                        furnace.Humidistat = false;
                        furnace.DehumidControlType_Num = DehumidificationControlMode::None;
                    }
                } else if (Util::SameString(Alphas(14), "None")) {
                    furnace.DehumidControlType_Num = DehumidificationControlMode::None;
                    furnace.Humidistat = false;
                }
                
                if (furnace.Humidistat) {
                    for (HStatZoneNum = 1; HStatZoneNum <= state.dataZoneCtrls->NumHumidityControlZones; ++HStatZoneNum) {
                        if (state.dataZoneCtrls->HumidityControlZone(HStatZoneNum).ActualZoneNum != furnace.ControlZoneNum) continue;
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
                ShowSevereInvalidKey(state, eoh, cAlphaFields(14), Alphas(14));
                furnace.Humidistat = false;
                ErrorsFound = true;
            }

            //       Check placement of cooling coil with respect to fan placement and dehumidification control type
            if (furnace.fanPlace == HVAC::FanPlace::BlowThru) {
                if (furnace.FanOutletNode == furnace.HeatCoilAirInletNode &&
                    furnace.DehumidControlType_Num != DehumidificationControlMode::CoolReheat) {
                    furnace.CoolCoilUpstream = false;
                }
            } else {
                if (furnace.HeatCoilAirOutletNode == furnace.CoolCoilAirInletNode &&
                    furnace.DehumidControlType_Num != DehumidificationControlMode::CoolReheat) {
                    furnace.CoolCoilUpstream = false;
                }
            }

            // Get reheat coil data if humidistat is used
            furnace.suppHeatCoilType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Alphas(15)));
            furnace.SuppHeatCoilName = Alphas(16);

            if (!lAlphaBlanks(15)) {
                if (furnace.suppHeatCoilType == HVAC::CoilType::HeatingGasOrOtherFuel ||
                    furnace.suppHeatCoilType == HVAC::CoilType::HeatingElectric ||
                    furnace.suppHeatCoilType == HVAC::CoilType::HeatingDesuperheater) {

                    // Get the heating coil index
                    furnace.SuppHeatCoilNum = HeatingCoils::GetCoilIndex(state, furnace.SuppHeatCoilName);
                    if (furnace.SuppHeatCoilNum == 0) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFields(15), furnace.SuppHeatCoilName);
                        ErrorsFound = true;
                    } else {
                        furnace.DesignSuppHeatingCapacity = HeatingCoils::GetCoilCapacity(state, furnace.SuppHeatCoilNum);
                        furnace.SuppHeatCoilAirInletNode = HeatingCoils::GetCoilAirInletNode(state, furnace.SuppHeatCoilNum);
                        furnace.SuppHeatCoilAirOutletNode = HeatingCoils::GetCoilAirOutletNode(state, furnace.SuppHeatCoilNum);
                    }

                } else if (furnace.suppHeatCoilType == HVAC::CoilType::HeatingWater) {
                    // Get the Heating Coil water Inlet or control Node number
                    furnace.SuppHeatCoilNum = WaterCoils::GetCoilIndex(state, furnace.SuppHeatCoilName);
                    if (furnace.SuppHeatCoilNum == 0) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFields(16), furnace.SuppHeatCoilName);
                        ErrorsFound = true;
                    } else {
                        furnace.SuppHeatCoilControlNode = WaterCoils::GetCoilWaterInletNode(state, furnace.SuppHeatCoilNum);
                        furnace.MaxSuppHeatCoilFluidFlow = WaterCoils::GetCoilMaxWaterFlowRate(state, furnace.SuppHeatCoilNum);
                        furnace.SuppHeatCoilAirInletNode = WaterCoils::GetCoilAirInletNode(state, furnace.SuppHeatCoilNum);
                        furnace.SuppHeatCoilAirOutletNode = WaterCoils::GetCoilAirOutletNode(state, furnace.SuppHeatCoilNum);
                        // check if user has also used a water coil controller, which they should not do
                        errFlag = false;
                        HVACControllers::CheckCoilWaterInletNode(state, furnace.SuppHeatCoilControlNode, errFlag);
                        if (!errFlag) { // then did find a controller so that is bad
                            ShowSevereError(state,
                                            format("{} = {} has a conflicting Controller:WaterCoil object", CurrentModuleObject, furnace.Name));
                            ShowContinueError(state, "Hot water coils are controlled directly by unitary and furnace systems.");
                            ShowContinueError(state, "No water coil controller should be input for the coil.");
                            ErrorsFound = true;
                        }
                    }

                } else if (furnace.suppHeatCoilType == HVAC::CoilType::HeatingSteam) {
                    furnace.SuppHeatCoilNum = SteamCoils::GetCoilIndex(state, furnace.SuppHeatCoilName);
                    if (furnace.SuppHeatCoilNum == 0) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFields(16), furnace.SuppHeatCoilName);
                        ErrorsFound = true;
                    } else {
                        furnace.SuppHeatCoilControlNode = SteamCoils::GetCoilSteamInletNode(state, furnace.SuppHeatCoilNum);
                        // Get the Heating Coil steam max volume flow rate
                        furnace.MaxSuppHeatCoilFluidFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, furnace.SuppHeatCoilNum);
                        if (furnace.MaxSuppHeatCoilFluidFlow > 0.0) {
                            furnace.MaxSuppHeatCoilFluidFlow *= Fluid::GetSteam(state)->getSatDensity(state, 100.0, 1.0, getAirLoopHVACHeatCoolInput);
                        }

                        furnace.SuppHeatCoilAirInletNode = SteamCoils::GetCoilAirInletNode(state, furnace.SuppHeatCoilNum);
                        furnace.SuppHeatCoilAirOutletNode = SteamCoils::GetCoilAirOutletNode(state, furnace.SuppHeatCoilNum);
                    }

                } else { // Illegal heating coil
                    ShowSevereInvalidKey(state, eoh, cAlphaFields(15), Alphas(15));
                    ErrorsFound = true;
                } // IF (Furnace(FurnaceNum)%SuppHeatCoilType_Num == Coil_HeatingGasOrOtherFuel .OR. &, etc.

            } // IF(.NOT. lAlphaBlanks(15))THEN

            if (furnace.fanPlace == HVAC::FanPlace::BlowThru) {

                if (furnace.FanInletNode != furnace.FurnaceInletNode) {
                    if (furnace.type == HVAC::UnitarySysType::Furnace_HeatCool) {
                        ShowSevereCustom(state, eoh, 
                                         "When a blow through fan is specified, the fan inlet node name must be the same as the furnace inlet node name.");
                        ShowContinueError(state, format("...Fan inlet node name     = {}", s_node->NodeID(furnace.FanInletNode)));
                        ShowContinueError(state, format("...Furnace inlet node name = {}", s_node->NodeID(furnace.FurnaceInletNode)));
                    } else {
                        ShowSevereCustom(state, eoh,
                            "When a blow through fan is specified, the fan inlet node name must be the same as the unitary system inlet node name.");
                        ShowContinueError(state, format("...Fan inlet node name           = {}", s_node->NodeID(furnace.FanInletNode)));
                        ShowContinueError(state, format("...UnitarySystem inlet node name = {}", s_node->NodeID(furnace.FurnaceInletNode)));
                    }
                    ErrorsFound = true;
                }
                if (furnace.CoolCoilUpstream) {
                    if (furnace.FanOutletNode != furnace.CoolCoilAirInletNode) {
                        ShowSevereCustom(state, eoh, 
                            "When a blow through fan is specified, the fan outlet node name must be the same as the cooling coil inlet node name.");
                        ShowContinueError(state, format("...Fan outlet node name         = {}", s_node->NodeID(furnace.FanOutletNode)));
                        ShowContinueError(state, format("...Cooling coil inlet node name = {}", s_node->NodeID(furnace.CoolCoilAirInletNode)));
                        ErrorsFound = true;
                    }
                    if (furnace.CoolCoilAirOutletNode != furnace.HeatCoilAirInletNode) {
                        ShowSevereCustom(state, eoh,
                                       "The cooling coil outlet node name must be the same as the heating coil inlet node name.");
                        ShowContinueError(state, format("...Cooling coil outlet node name = {}", s_node->NodeID(furnace.CoolCoilAirOutletNode)));
                        ShowContinueError(state, format("...Heating coil inlet node name  = {}", s_node->NodeID(furnace.HeatCoilAirInletNode)));
                        ErrorsFound = true;
                    }
                    if ((furnace.Humidistat && furnace.DehumidControlType_Num == DehumidificationControlMode::CoolReheat) ||
                        furnace.SuppHeatCoilAirInletNode > 0) {
                        if (furnace.HeatCoilAirOutletNode != furnace.SuppHeatCoilAirInletNode) {
                            ShowSevereCustom(state, eoh,
                                              "When a blow through fan is specified, the heating coil outlet node name must be the same as the "
                                              "reheat coil inlet node name.");
                            ShowContinueError(state, format("...Heating coil outlet node name = {}", s_node->NodeID(furnace.HeatCoilAirOutletNode)));
                            ShowContinueError(state, format("...Reheat coil inlet node name   = {}", s_node->NodeID(furnace.SuppHeatCoilAirInletNode)));
                            ErrorsFound = true;
                        }
                        if (furnace.SuppHeatCoilAirOutletNode != furnace.FurnaceOutletNode) {
                            if (furnace.type == HVAC::UnitarySysType::Furnace_HeatCool) {
                                ShowSevereCustom(state, eoh, "The reheat coil outlet node name must be the same as the furnace outlet node name.");
                                ShowContinueError(state, format("...Reheat coil outlet node name = {}", s_node->NodeID(furnace.SuppHeatCoilAirOutletNode)));
                                ShowContinueError(state, format("...Furnace outlet node name     = {}", s_node->NodeID(furnace.FurnaceOutletNode)));
                            } else {
                                ShowContinueError(state, "The reheat coil outlet node name must be the same as the unitary system outlet node name.");
                                ShowContinueError(state, format("...Reheat coil outlet node name   = {}", s_node->NodeID(furnace.SuppHeatCoilAirOutletNode)));
                                ShowContinueError(state, format("...UnitarySystem outlet node name = {}", s_node->NodeID(furnace.FurnaceOutletNode)));
                            }
                            ErrorsFound = true;
                        }
                    } else { // IF((Furnace(FurnaceNum)%Humidistat ...
                        // Heating coil outlet node name must be the same as the furnace outlet node name
                        if (furnace.HeatCoilAirOutletNode != furnace.FurnaceOutletNode) {
                            if (furnace.type == HVAC::UnitarySysType::Furnace_HeatOnly) {
                                ShowSevereCustom(state, eoh, 
                                    "When a blow through fan is specified, the heating coil outlet node name must be the same as the "
                                                 "furnace outlet node name.");
                                ShowContinueError(state, format("...Heating coil outlet node name = {}", s_node->NodeID(furnace.HeatCoilAirOutletNode)));
                                ShowContinueError(state, format("...Furnace outlet node name      = {}", s_node->NodeID(furnace.FurnaceOutletNode)));
                            } else {
                                ShowSevereCustom(state, eoh,
                                                  "When a blow through fan is specified, the heating coil outlet node name must be the same as the "
                                                  "unitary system outlet node name.");
                                ShowContinueError(state, format("...Heating coil outlet node name  = {}", s_node->NodeID(furnace.HeatCoilAirOutletNode)));
                                ShowContinueError(state, format("...UnitarySystem outlet node name = {}", s_node->NodeID(furnace.FurnaceOutletNode)));
                            }
                            ErrorsFound = true;
                        }
                    }
                } else { // IF(Furnace(FurnaceNum)%CoolingCoilUpstream)THEN
                    if (furnace.FanOutletNode != furnace.HeatCoilAirInletNode) {
                        ShowSevereCustom(state, eoh, 
                            "When a blow through fan is specified, the fan outlet node name must be the same as the heating coil inlet node name.");
                        ShowContinueError(state, format("...Fan outlet node name         = {}", s_node->NodeID(furnace.FanOutletNode)));
                        ShowContinueError(state, format("...Heating coil inlet node name = {}", s_node->NodeID(furnace.HeatCoilAirInletNode)));
                        ErrorsFound = true;
                    }
                    if (furnace.HeatCoilAirOutletNode != furnace.CoolCoilAirInletNode) {
                        ShowSevereError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state, "The heating coil outlet node name must be the same as the cooling coil inlet node name.");
                        ShowContinueError(state, format("...Heating coil outlet node name = {}", s_node->NodeID(furnace.HeatCoilAirOutletNode)));
                        ShowContinueError(state, format("...Cooling coil inlet node name  = {}", s_node->NodeID(furnace.CoolCoilAirInletNode)));
                        ErrorsFound = true;
                    }
                    if (furnace.CoolCoilAirOutletNode != furnace.FurnaceOutletNode) {
                        if (furnace.type == HVAC::UnitarySysType::Furnace_HeatCool) {
                            ShowSevereCustom(state, eoh, 
                                              "When a blow through fan is specified, the cooling coil outlet node name must be the same as the "
                                              "furnace outlet node name.");
                            ShowContinueError(state, format("...Cooling coil outlet node name = {}", s_node->NodeID(furnace.CoolCoilAirOutletNode)));
                            ShowContinueError(state, format("...Furnace outlet node name      = {}", s_node->NodeID(furnace.FurnaceOutletNode)));
                        } else {
                            ShowSevereCustom(state, eoh,
                                              "When a blow through fan is specified, the cooling coil outlet node name must be the same as the "
                                              "unitary system outlet node name.");
                            ShowContinueError(state, format("...Cooling coil outlet node name   = {}", s_node->NodeID(furnace.CoolCoilAirOutletNode)));
                            ShowContinueError(state, format("...UnitarySystem outlet node name  = {}", s_node->NodeID(furnace.FurnaceOutletNode)));
                        }
                        ErrorsFound = true;
                    }
                }

            } else { // ELSE from IF(Furnace(FurnaceNum)%FanPlace .EQ. BlowThru)THEN

                if (furnace.CoolCoilUpstream) {
                    if (furnace.CoolCoilAirInletNode != furnace.FurnaceInletNode) {
                        if (furnace.type == HVAC::UnitarySysType::Furnace_HeatCool) {
                            ShowSevereCustom(state, eoh, 
                                             "When a draw through fan is specified, the cooling coil inlet node name must be the same as the "
                                             "furnace inlet node name.");
                            ShowContinueError(state, format("...Cooling coil inlet node name = {}", s_node->NodeID(furnace.CoolCoilAirInletNode)));
                            ShowContinueError(state, format("...Furnace inlet node name      = {}", s_node->NodeID(furnace.FurnaceInletNode)));
                        } else {
                           ShowSevereCustom(state, eoh,
                                           "When a draw through fan is specified, the cooling coil inlet node name must be the same as the "
                                           "unitary system inlet node name.");
                            ShowContinueError(state, format("...Cooling coil inlet node name  = {}", s_node->NodeID(furnace.CoolCoilAirInletNode)));
                            ShowContinueError(state, format("...UnitarySystem inlet node name = {}", s_node->NodeID(furnace.FurnaceInletNode)));
                        }
                        ErrorsFound = true;
                    }
                    if (furnace.CoolCoilAirOutletNode != furnace.HeatCoilAirInletNode) {
                        ShowSevereError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state, "The cooling coil outlet node name must be the same as the heating coil inlet node name.");
                        ShowContinueError(state, format("...Cooling coil outlet node name = {}", s_node->NodeID(furnace.CoolCoilAirOutletNode)));
                        ShowContinueError(state, format("...Heating coil inlet node name  = {}", s_node->NodeID(furnace.HeatCoilAirInletNode)));
                        ErrorsFound = true;
                    }
                    if (furnace.HeatCoilAirOutletNode != furnace.FanInletNode) {
                        ShowSevereCustom(state, eoh, 
                            "When a draw through fan is specified, the heating coil outlet node name must be the same as the fan inlet node name.");
                        ShowContinueError(state, format("...Heating coil outlet node name = {}", s_node->NodeID(furnace.HeatCoilAirOutletNode)));
                        ShowContinueError(state, format("...Fan inlet node name           = {}", s_node->NodeID(furnace.FanInletNode)));
                        ErrorsFound = true;
                    }
                    if ((furnace.Humidistat && furnace.DehumidControlType_Num == DehumidificationControlMode::CoolReheat) ||
                        furnace.SuppHeatCoilAirInletNode > 0) {
                        if (furnace.FanOutletNode != furnace.SuppHeatCoilAirInletNode) {
                            ShowSevereCustom(state, eoh, 
                                              "When a draw through fan is specified, the fan outlet node name must be the same as the reheat coil "
                                              "inlet node name.");
                            ShowContinueError(state, format("...Fan outlet node name        = {}", s_node->NodeID(furnace.FanOutletNode)));
                            ShowContinueError(state, format("...Reheat coil inlet node name = {}", s_node->NodeID(furnace.SuppHeatCoilAirInletNode)));
                            ErrorsFound = true;
                        }
                        if (furnace.SuppHeatCoilAirOutletNode != furnace.FurnaceOutletNode) {
                            if (furnace.type == HVAC::UnitarySysType::Furnace_HeatCool) {
                                ShowSevereCustom(state, eoh, "The reheat coil outlet node name must be the same as the furnace outlet node name.");
                                ShowContinueError(state, format("...Reheat coil outlet node name = {}", s_node->NodeID(furnace.SuppHeatCoilAirOutletNode)));
                                ShowContinueError(state, format("...Furnace outlet node name     = {}", s_node->NodeID(furnace.FurnaceOutletNode)));
                            } else {
                                ShowContinueError(state, "The reheat coil outlet node name must be the same as the unitary system outlet node name.");
                                ShowContinueError(
                                    state, format("...Reheat coil outlet node name   = {}", s_node->NodeID(furnace.SuppHeatCoilAirOutletNode)));
                                ShowContinueError(
                                    state,
                                    format("...UnitarySystem outlet node name = {}", s_node->NodeID(furnace.FurnaceOutletNode)));
                            }
                            ErrorsFound = true;
                        }
                    } else {
                        if (furnace.FanOutletNode != furnace.FurnaceOutletNode) {
                            ShowSevereCustom(state, eoh, 
                                              "When a draw through fan is specified, the fan outlet node name must be the same as the unitary system "
                                              "outlet node name.");
                            ShowContinueError(state, format("...Fan outlet node name        = {}", s_node->NodeID(furnace.FanOutletNode)));
                            ShowContinueError(state, format("...Unitary system outlet node name = {}", s_node->NodeID(furnace.FurnaceOutletNode)));
                            ErrorsFound = true;
                        }
                    }
                } else { // IF(Furnace(FurnaceNum)%CoolingCoilUpstream)THEN
                    if (furnace.HeatCoilAirInletNode != furnace.FurnaceInletNode) {
                        if (furnace.type == HVAC::UnitarySysType::Furnace_HeatCool) {
                            ShowSevereCustom(state, eoh,
                                              "When a draw through fan is specified, the heating coil inlet node name must be the same as the "
                                              "furnace inlet node name.");
                            ShowContinueError(state, format("...Heating coil inlet node name = {}", s_node->NodeID(furnace.HeatCoilAirInletNode)));
                            ShowContinueError(state, format("...Furnace inlet node name      = {}", s_node->NodeID(furnace.FurnaceInletNode)));
                        } else {
                            ShowSevereCustom(state, eoh, 
                                             "When a draw through fan is specified, the heating coil inlet node name must be the same as the "
                                             "unitary system inlet node name.");
                            ShowContinueError(state, format("...Heating coil inlet node name  = {}", s_node->NodeID(furnace.HeatCoilAirInletNode)));
                            ShowContinueError(state, format("...UnitarySystem inlet node name = {}", s_node->NodeID(furnace.FurnaceInletNode)));
                        }
                        ErrorsFound = true;
                    }
                    if (furnace.HeatCoilAirOutletNode != furnace.CoolCoilAirInletNode) {
                        ShowSevereCustom(state, eoh, "The heating coil outlet node name must be the same as the cooling coil inlet node name.");
                        ShowContinueError(state, format("...Heating coil outlet node name = {}", s_node->NodeID(furnace.HeatCoilAirOutletNode)));
                        ShowContinueError(state, format("...Cooling coil inlet node name  = {}", s_node->NodeID(furnace.CoolCoilAirInletNode)));
                        ErrorsFound = true;
                    }
                    if (furnace.CoolCoilAirOutletNode != furnace.FanInletNode) {
                        ShowSevereCustom(state, eoh, 
                            "When a draw through fan is specified, the cooling coil outlet node name must be the same as the fan inlet node name.");
                        ShowContinueError(state, format("...Cooling coil outlet node name = {}", s_node->NodeID(furnace.CoolCoilAirOutletNode)));
                        ShowContinueError(state, format("...Fan inlet node name           = {}", s_node->NodeID(furnace.FanInletNode)));
                        ErrorsFound = true;
                    }
                    if (furnace.FanOutletNode != furnace.FurnaceOutletNode) {
                          if (furnace.type == HVAC::UnitarySysType::Furnace_HeatCool) {
                            ShowSevereCustom(state, eoh, 
                                           "When a draw through fan is specified, the fan outlet node name must be the same as the furnace outlet node name.");
                            ShowContinueError(state, format("...Fan outlet node name     = {}", s_node->NodeID(furnace.FanOutletNode)));
                            ShowContinueError(state, format("...Furnace outlet node name = {}", s_node->NodeID(furnace.FurnaceOutletNode)));
                        } else {
                            ShowSevereCustom(state, eoh,
                                              "When a draw through fan is specified, the fan outlet node name must be the same as the unitary system "
                                              "outlet node name.");
                            ShowContinueError(state, format("...Fan outlet node name           = {}", s_node->NodeID(furnace.FanOutletNode)));
                            ShowContinueError(state, format("...UnitarySystem outlet node name = {}", s_node->NodeID(furnace.FurnaceOutletNode)));
                        }
                        ErrorsFound = true;
                    }
                }
            } // ELSE from IF(Furnace(FurnaceNum)%FanPlace .EQ. BlowThru)THEN

            // Add fan to component sets array
            BranchNodeConnections::SetUpCompSets(state,
                                                 CurrentModuleObject,
                                                 Alphas(1),
                                                 Alphas(7),
                                                 Alphas(8),
                                                 s_node->NodeID(furnace.FanInletNode),
                                                 s_node->NodeID(furnace.FanOutletNode));

            // Add DX cooling coil to component sets array
            if (furnace.isIHP) {
                BranchNodeConnections::SetUpCompSets(state,
                                                     CurrentModuleObject,
                                                     Alphas(1),
                                                     Alphas(12),
                                                     Alphas(13) + " Cooling Coil",
                                                     s_node->NodeID(furnace.CoolCoilAirInletNode),
                                                     s_node->NodeID(furnace.CoolCoilAirOutletNode));
            } else {
                BranchNodeConnections::SetUpCompSets(state,
                                                     CurrentModuleObject,
                                                     Alphas(1),
                                                     Alphas(12),
                                                     Alphas(13),
                                                     s_node->NodeID(furnace.CoolCoilAirInletNode),
                                                     s_node->NodeID(furnace.CoolCoilAirOutletNode));
            }

            // Add heating coil to component sets array
            if (furnace.isIHP) {
                BranchNodeConnections::SetUpCompSets(state,
                                                     CurrentModuleObject,
                                                     Alphas(1),
                                                     Alphas(10),
                                                     Alphas(11) + " Heating Coil",
                                                     s_node->NodeID(furnace.HeatCoilAirInletNode),
                                                     s_node->NodeID(furnace.HeatCoilAirOutletNode));
            } else {
                BranchNodeConnections::SetUpCompSets(state,
                                                     CurrentModuleObject,
                                                     Alphas(1),
                                                     Alphas(10),
                                                     Alphas(11),
                                                     s_node->NodeID(furnace.HeatCoilAirInletNode),
                                                     s_node->NodeID(furnace.HeatCoilAirOutletNode));
            }

            if (furnace.SuppHeatCoilAirInletNode > 0) {

                // Add reheating coil to component sets array
                BranchNodeConnections::SetUpCompSets(state,
                                                     CurrentModuleObject,
                                                     Alphas(1),
                                                     Alphas(15),
                                                     Alphas(16),
                                                     s_node->NodeID(furnace.SuppHeatCoilAirInletNode),
                                                     s_node->NodeID(furnace.SuppHeatCoilAirOutletNode));
            }

            // Set the furnace max outlet temperature
            furnace.DesignMaxOutletTemp = Numbers(1);

            furnace.MaxCoolAirVolFlow = Numbers(2);
            if (furnace.MaxCoolAirVolFlow <= 0 && furnace.MaxCoolAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {:.7T}", cNumericFields(2), Numbers(2)));
                ErrorsFound = true;
            }

            furnace.MaxHeatAirVolFlow = Numbers(3);
            if (furnace.MaxHeatAirVolFlow <= 0 && furnace.MaxHeatAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {:.7T}", cNumericFields(3), Numbers(3)));
                ErrorsFound = true;
            }

            furnace.MaxNoCoolHeatAirVolFlow = Numbers(4);
            if (furnace.MaxNoCoolHeatAirVolFlow < 0 && furnace.MaxNoCoolHeatAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {:.7T}", cNumericFields(4), Numbers(4)));
                ErrorsFound = true;
            }

            if (Numbers(2) != DataSizing::AutoSize && Numbers(3) != DataSizing::AutoSize && Numbers(4) != DataSizing::AutoSize) {
                furnace.DesignFanVolFlowRate = max(Numbers(2), Numbers(3), Numbers(4));
            } else {
                furnace.DesignFanVolFlowRate = DataSizing::AutoSize;
            }

            if (furnace.coolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
                furnace.MaxNoCoolHeatAirVolFlow = min(furnace.MaxHeatAirVolFlow, furnace.MaxCoolAirVolFlow);
                if (furnace.MaxHeatAirVolFlow != DataSizing::AutoSize && furnace.MaxCoolAirVolFlow != DataSizing::AutoSize) {
                    furnace.DesignFanVolFlowRate = max(furnace.MaxHeatAirVolFlow, furnace.MaxCoolAirVolFlow);
                } else {
                    furnace.DesignFanVolFlowRate = DataSizing::AutoSize;
                }
            }

            if (furnace.ActualFanVolFlowRate != DataSizing::AutoSize) {
                if (furnace.ActualFanVolFlowRate < furnace.MaxCoolAirVolFlow && furnace.MaxCoolAirVolFlow != DataSizing::AutoSize) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(
                        state,
                        format("... air flow rate = {:.7T} in fan object {} is less than the maximum HVAC system air flow rate in cooling mode.",
                               furnace.ActualFanVolFlowRate,
                               FanName));
                    ShowContinueError(state, format(" The {} is reset to the fan flow rate and the simulation continues.", cNumericFields(2)));
                    furnace.MaxCoolAirVolFlow = furnace.ActualFanVolFlowRate;
                    furnace.DesignFanVolFlowRate = furnace.ActualFanVolFlowRate;
                }
                if (furnace.ActualFanVolFlowRate < furnace.MaxHeatAirVolFlow && furnace.MaxHeatAirVolFlow != DataSizing::AutoSize) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(
                        state,
                        format("... air flow rate = {:.7T} in fan object {} is less than the maximum HVAC system air flow rate in heating mode.",
                               furnace.ActualFanVolFlowRate,
                               FanName));
                    ShowContinueError(state, format(" The {} is reset to the fan flow rate and the simulation continues.", cNumericFields(3)));
                    furnace.MaxHeatAirVolFlow = furnace.ActualFanVolFlowRate;
                    furnace.DesignFanVolFlowRate = furnace.ActualFanVolFlowRate;
                }
            }

            if (furnace.fanOpModeSched != nullptr) {
                // Is this correct? 0.0 for max also?
                if (!furnace.fanOpModeSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 0.0)) {
                    //           set air flow control mode:
                    //             UseCompressorOnFlow = operate at last cooling or heating air flow requested when compressor is off
                    //             UseCompressorOffFlow = operate at value specified by user
                    //           AirFlowControl only valid if fan opmode = ContFanCycComp
                    if (furnace.MaxNoCoolHeatAirVolFlow == 0.0) {
                        furnace.AirFlowControl = AirFlowControlConstFan::UseCompressorOnFlow;
                    } else {
                        furnace.AirFlowControl = AirFlowControlConstFan::UseCompressorOffFlow;
                    }
                }
            }


            // Set heating convergence tolerance
            furnace.HeatingConvergenceTolerance = 0.001;

            // Set cooling convergence tolerance
            furnace.CoolingConvergenceTolerance = 0.001;

            // set minimum outdoor temperature for compressor operation
            SetMinOATCompressor(state, FurnaceNum, cCurrentModuleObject, ErrorsFound);

        } // End of the HeatCool Furnace Loop

        // Get the data for the Unitary System HeatPump AirToAir (UnitarySystem:HeatPump:AirToAir)
        for (int HeatPumpNum = 1; HeatPumpNum <= NumHeatPump; ++HeatPumpNum) {

            CurrentModuleObject = "AirLoopHVAC:UnitaryHeatPump:AirToAir";

            FurnaceNum = NumHeatOnly + NumHeatCool + NumUnitaryHeatOnly + NumUnitaryHeatCool + HeatPumpNum;
            auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);
            furnace.iterationMode.allocate(3);

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

            furnace.type = HVAC::UnitarySysType::Unitary_HeatPump_AirToAir;
            furnace.Name = Alphas(1);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, furnace.Name};

            if (lAlphaBlanks(2)) {
                furnace.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((furnace.availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ErrorsFound = true;
            }

            furnace.FurnaceInletNode =
                NodeInputManager::GetOnlySingleNode(state,
                                                    Alphas(3),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatPumpAirToAir,
                                                    Alphas(1),
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Inlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);

            furnace.FurnaceOutletNode =
                NodeInputManager::GetOnlySingleNode(state,
                                                    Alphas(4),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatPumpAirToAir,
                                                    Alphas(1),
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Outlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);

            BranchNodeConnections::TestCompSet(state, CurrentModuleObject, Alphas(1), Alphas(3), Alphas(4), "Air Nodes");

            // Get the Controlling Zone or Location of the Furnace Thermostat
            furnace.ControlZoneNum = Util::FindItemInList(Alphas(5), state.dataHeatBal->Zone);
            if (furnace.ControlZoneNum == 0) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(5), Alphas(5)));
                ErrorsFound = true;
            }

            // Get the node number for the zone with the thermostat
            if (furnace.ControlZoneNum > 0) {
                AirNodeFound = false;
                AirLoopFound = false;
                int ControlledZoneNum = furnace.ControlZoneNum;
                //             Find the controlled zone number for the specified thermostat location
                furnace.NodeNumOfControlledZone = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ZoneNode;
                //             Determine if furnace is on air loop served by the thermostat location specified
                for (int zoneInNode = 1; zoneInNode <= state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).NumInletNodes; ++zoneInNode) {
                    int AirLoopNumber = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNodeAirLoopNum(zoneInNode);
                    furnace.airloopNum = AirLoopNumber;
                    if (AirLoopNumber > 0) {
                        for (int BranchNum = 1; BranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).NumBranches; ++BranchNum) {
                            for (int CompNum = 1;
                                 CompNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).TotalComponents;
                                 ++CompNum) {
                                if (!Util::SameString(state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).Comp(CompNum).Name,
                                                      Alphas(1)) ||
                                    !Util::SameString(
                                        state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).Comp(CompNum).TypeOf,
                                        CurrentModuleObject)) {
                                    continue;
                                }
                                AirLoopFound = true;
                                furnace.ZoneInletNode = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNode(zoneInNode);
                                break;
                            }
                            if (AirLoopFound) {
                                break;
                            }
                        }
                        for (TstatZoneNum = 1; TstatZoneNum <= state.dataZoneCtrls->NumTempControlledZones; ++TstatZoneNum) {
                            if (state.dataZoneCtrls->TempControlledZone(TstatZoneNum).ActualZoneNum != furnace.ControlZoneNum) continue;
                            AirNodeFound = true;
                        }
                        for (TstatZoneNum = 1; TstatZoneNum <= state.dataZoneCtrls->NumComfortControlledZones; ++TstatZoneNum) {
                            if (state.dataZoneCtrls->ComfortControlledZone(TstatZoneNum).ActualZoneNum != furnace.ControlZoneNum) continue;
                            AirNodeFound = true;
                        }
                    }
                    if (AirLoopFound) {
                        break;
                    }
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
                    ShowContinueError(state, "Did not find correct AirLoopHVAC.");
                    ShowContinueError(state, format("Specified {} = {}", cAlphaFields(5), Alphas(5)));
                    ErrorsFound = true;
                }
            }

            // Get fan data
            FanName = Alphas(7);

            furnace.fanType = static_cast<HVAC::FanType>(getEnumValue(HVAC::fanTypeNamesUC, Alphas(6)));

            if (furnace.fanType == HVAC::FanType::OnOff || furnace.fanType == HVAC::FanType::Constant) {

                if ((furnace.FanIndex = Fans::GetFanIndex(state, FanName)) == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(7), FanName);
                    ErrorsFound = true;
                } else {
                    auto *fan = state.dataFans->fans(furnace.FanIndex);
                    furnace.FanInletNode = fan->inletNodeNum;
                    furnace.FanOutletNode = fan->outletNodeNum;
                    furnace.fanAvailSched = fan->availSched;
                    furnace.ActualFanVolFlowRate = fan->maxAirFlowRate;
                }
            }

            // Get heating coil type and name data
            furnace.heatCoilType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Alphas(8)));
            furnace.HeatCoilName = Alphas(9);

            if (Util::SameString(Alphas(8), "COILSYSTEM:INTEGRATEDHEATPUMP:AIRSOURCE")) {
                furnace.isIHP = true;
                furnace.ihpName = furnace.HeatCoilName;
                furnace.ihpNum = IntegratedHeatPump::GetIHPIndex(state, furnace.ihpName);
                if (furnace.ihpNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(9), furnace.ihpName);
                    ErrorsFound = true;
                } else {
                    // HeatCoilName and HeatCoilType now reference the
                    // VariableSpeedCoil in the IHP, not the enclosing
                    // IHP
                    furnace.heatCoilType = HVAC::CoilType::HeatingDXVariableSpeed;
                    furnace.HeatCoilName = state.dataIntegratedHP->IntegratedHeatPumps(furnace.ihpNum).SHCoilName;
                }
            }

            if (furnace.heatCoilType == HVAC::CoilType::HeatingDXSingleSpeed) {
                furnace.HeatCoilNum = DXCoils::GetCoilIndex(state, furnace.HeatCoilName);
                if (furnace.HeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(9), furnace.HeatCoilName);
                    ErrorsFound = true;
                } else {
                    furnace.HeatCoilAirInletNode = DXCoils::GetCoilAirInletNode(state, furnace.HeatCoilNum);
                    furnace.HeatCoilAirOutletNode = DXCoils::GetCoilAirOutletNode(state, furnace.HeatCoilNum);
                    furnace.DesignHeatingCapacity = DXCoils::GetCoilCapacity(state, furnace.HeatCoilNum);
                }
                
            } else if (furnace.heatCoilType == HVAC::CoilType::HeatingDXVariableSpeed) {
                // At this point, it shouldn't matter whether the coil is embedded in an IHP or not
                furnace.HeatCoilNum = VariableSpeedCoils::GetCoilIndex(state, furnace.HeatCoilName);
                if (furnace.HeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(9), furnace.HeatCoilName);
                    ErrorsFound = true;
                } else {
                    furnace.HeatCoilAirInletNode = VariableSpeedCoils::GetCoilAirInletNode(state, furnace.HeatCoilNum);
                    furnace.HeatCoilAirOutletNode = VariableSpeedCoils::GetCoilAirOutletNode(state, furnace.HeatCoilNum);
                    furnace.DesignHeatingCapacity = VariableSpeedCoils::GetCoilCapacity(state, furnace.HeatCoilNum);
                }
            } else {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(8), Alphas(8));
                ErrorsFound = true;
            }

            // Get Cooling Coil Information if available
            furnace.coolCoilType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Alphas(10)));
            furnace.CoolCoilName = Alphas(11);

            if (Util::SameString(Alphas(10), "COILSYSTEM:INTEGRATEDHEATPUMP:AIRSOURCE")) {

                // Is this a legitimate check? Both heating and cooling colis have to be the same integrated heat pump?
                if (!furnace.isIHP) { 
                    ShowSevereCustom(state, eoh, "Cooling Coil is an integrated heat pump, but heating coil is not.");
                    ErrorsFound = true;
                } else if (!Util::SameString(furnace.CoolCoilName, furnace.ihpName)) {
                    ShowSevereCustom(state, eoh, "Cooling Coil and cooling coil name different integrated heat pumps.");
                    ErrorsFound = true;
                } else {
                    furnace.coolCoilType = HVAC::CoilType::CoolingDXVariableSpeed;
                }
            }
            
            // If coil type not found, check to see if a HX assisted cooling coil is used.
            // Is this actually needed?
#ifdef GET_OUT            
            if (furnace.coolCoilType == HVAC::CoilType::Invalid) {
                int hxCoilIndex = HXAssistCoil::GetHXCoilIndex(state, furnace.CoolCoilName);
                if (hxCoilIndex == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(9), furnace.CoolCoilName);
                    ErrorsFound = true;
                } else {
                    furnace.coolCoilType = HXAssistCoil::GetHXCoilType(state, furnace.CoolCoilName);
                }
            }
#endif // GET_OUT
            
            // Single-speed DX Coil
            if (furnace.coolCoilType == HVAC::CoilType::CoolingDXSingleSpeed) {
                furnace.CoolCoilNum = DXCoils::GetCoilIndex(state, furnace.CoolCoilName);
                if (furnace.CoolCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(11), furnace.CoolCoilName);
                    ErrorsFound = true;
                } else {
                    furnace.CoolCoilAirInletNode = DXCoils::GetCoilAirInletNode(state, furnace.CoolCoilNum);
                    furnace.CoolCoilAirOutletNode = DXCoils::GetCoilAirOutletNode(state, furnace.CoolCoilNum);
                    furnace.DesignCoolingCapacity = DXCoils::GetCoilCapacity(state, furnace.CoolCoilNum);
                    furnace.CondenserNodeNum = DXCoils::GetCoilCondenserInletNode(state, furnace.CoolCoilNum);
                }

            } else if (furnace.coolCoilType == HVAC::CoilType::CoolingDXHXAssisted) {
                // hxCoolCoil are the HXAssisted object parameters
                // CoolCoil variables become the embedded DXCoil parameters
                furnace.CoolCoilNum = HXAssistCoil::GetCoilIndex(state, furnace.CoolCoilName);
                if (furnace.CoolCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(11), furnace.CoolCoilName);
                    ErrorsFound = true;
                } else {
                    furnace.childCoolCoilType = HXAssistCoil::GetCoilChildCoilType(state, furnace.CoolCoilNum);
                    furnace.childCoolCoilName = HXAssistCoil::GetCoilChildCoilName(state, furnace.CoolCoilNum);
                    furnace.childCoolCoilNum = HXAssistCoil::GetCoilChildCoilIndex(state, furnace.CoolCoilNum);
                    
                    furnace.CoolCoilAirInletNode = DXCoils::GetCoilAirInletNode(state, furnace.childCoolCoilNum);
                    furnace.CoolCoilAirOutletNode = DXCoils::GetCoilAirOutletNode(state, furnace.childCoolCoilNum);
                    furnace.DesignCoolingCapacity = DXCoils::GetCoilCapacity(state, furnace.childCoolCoilNum);
                
                    furnace.CondenserNodeNum = DXCoils::GetCoilCondenserInletNode(state, furnace.childCoolCoilNum);
                }

            } else if (furnace.coolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
                // BOS ADDED, AUG/2012, VARIIABLE SPEED DX COOLING COIL
                //  Furnace(FurnaceNum)%DXCoolCoilType = 'COIL:COOLING:DX:VARIABLESPEED'
                //  Furnace(FurnaceNum)%DXCoolCoilName = CoolingCoilName
                if (furnace.isIHP) {
                    furnace.ihpNum = IntegratedHeatPump::GetIHPIndex(state, furnace.ihpName);
                    if (furnace.ihpNum == 0) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFields(11), furnace.ihpName);
                        ErrorsFound = true;
                    } else {
                        furnace.CoolCoilName = state.dataIntegratedHP->IntegratedHeatPumps(furnace.ihpNum).SCCoilName;
                        furnace.CoolCoilNum = VariableSpeedCoils::GetCoilIndex(state, furnace.CoolCoilName);
                        if (furnace.CoolCoilNum == 0) {
                            ShowSevereItemNotFound(state, eoh, format("{}.SCCoilName", furnace.ihpName), furnace.CoolCoilName);
                            ErrorsFound = true;
                        } 
                    }
                } else { // furnace.bIsHP
                    furnace.CoolCoilNum = VariableSpeedCoils::GetCoilIndex(state, furnace.CoolCoilName);
                    if (furnace.CoolCoilNum == 0) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFields(11), furnace.CoolCoilName);
                        ErrorsFound = true;
                    }
                }

                // Shoud be properly set up whether this is an IHP or not
                if (furnace.CoolCoilNum != 0) { 
                    furnace.CoolCoilAirInletNode = VariableSpeedCoils::GetCoilAirInletNode(state, furnace.CoolCoilNum);
                    furnace.CoolCoilAirOutletNode = VariableSpeedCoils::GetCoilAirOutletNode(state, furnace.CoolCoilNum);
                    furnace.CondenserNodeNum = VariableSpeedCoils::GetCoilCondenserInletNode(state, furnace.CoolCoilNum);
                    furnace.DesignCoolingCapacity = VariableSpeedCoils::GetCoilCapacity(state, furnace.CoolCoilNum);
                }
            } else {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(10), Alphas(10));
                ErrorsFound = true;
            }

            if (furnace.coolCoilType == HVAC::CoilType::CoolingDXVariableSpeed &&
                furnace.heatCoilType == HVAC::CoilType::HeatingDXVariableSpeed) {
                // Furnace(FurnaceNum)%WatertoAirHPType = WatertoAir_VarSpeedEquationFit
                if (furnace.isIHP) {
                    VariableSpeedCoils::SetCoilData(state,
                                                    state.dataIntegratedHP->IntegratedHeatPumps(furnace.CoolCoilNum).SCCoilNum,
                                                    _,
                                                    state.dataIntegratedHP->IntegratedHeatPumps(furnace.CoolCoilNum).SHCoilNum);
                } else {
                    VariableSpeedCoils::SetCoilData(state, furnace.CoolCoilNum, _, furnace.HeatCoilNum);
                }
            }

            // Get supplemental heating coil information
            furnace.suppHeatCoilType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Alphas(12)));
            furnace.SuppHeatCoilName = Alphas(13);

            if (furnace.suppHeatCoilType == HVAC::CoilType::HeatingGasOrOtherFuel ||
                furnace.suppHeatCoilType == HVAC::CoilType::HeatingElectric) {

                furnace.SuppHeatCoilNum = HeatingCoils::GetCoilIndex(state, furnace.SuppHeatCoilName);
                if (furnace.SuppHeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(13), furnace.SuppHeatCoilName);
                    ErrorsFound = true;
                } else {
                    furnace.SuppHeatCoilAirInletNode = HeatingCoils::GetCoilAirInletNode(state, furnace.SuppHeatCoilNum);
                    furnace.SuppHeatCoilAirOutletNode = HeatingCoils::GetCoilAirOutletNode(state, furnace.SuppHeatCoilNum);
                    furnace.DesignSuppHeatingCapacity = HeatingCoils::GetCoilCapacity(state, furnace.SuppHeatCoilNum);
                }

            } else if (furnace.suppHeatCoilType == HVAC::CoilType::HeatingWater) {
                furnace.SuppHeatCoilNum = WaterCoils::GetCoilIndex(state, furnace.SuppHeatCoilName);
                if (furnace.SuppHeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(13), furnace.SuppHeatCoilName);
                    ErrorsFound = true;
                } else {
                    furnace.SuppHeatCoilControlNode = WaterCoils::GetCoilWaterInletNode(state, furnace.SuppHeatCoilNum);
                    furnace.MaxSuppHeatCoilFluidFlow = WaterCoils::GetCoilMaxWaterFlowRate(state, furnace.SuppHeatCoilNum);
                    furnace.SuppHeatCoilAirInletNode = WaterCoils::GetCoilAirInletNode(state, furnace.SuppHeatCoilNum);
                    furnace.SuppHeatCoilAirOutletNode = WaterCoils::GetCoilAirOutletNode(state, furnace.SuppHeatCoilNum);

                    HVACControllers::CheckCoilWaterInletNode(state, furnace.SuppHeatCoilControlNode, errFlag);
                    if (!errFlag) { // then did find a controller so that is bad
                        ShowSevereError(state,
                                        format("{} = {} has a conflicting Controller:WaterCoil object", CurrentModuleObject, furnace.Name));
                        ShowContinueError(state, "Hot water coils are controlled directly by unitary and furnace systems.");
                        ShowContinueError(state, "No water coil controller should be input for the coil.");
                        ErrorsFound = true;
                    }
                }

            } else if (furnace.suppHeatCoilType == HVAC::CoilType::HeatingSteam) {
                 furnace.SuppHeatCoilNum = SteamCoils::GetCoilIndex(state, furnace.SuppHeatCoilName);
                 if (furnace.SuppHeatCoilNum == 0) {
                     ShowSevereItemNotFound(state, eoh, cAlphaFields(13), furnace.SuppHeatCoilName);
                     ErrorsFound = true;
                 } else {
                     furnace.SuppHeatCoilControlNode = SteamCoils::GetCoilSteamInletNode(state, furnace.SuppHeatCoilNum);
                     furnace.MaxSuppHeatCoilFluidFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, furnace.SuppHeatCoilNum);
                     if (furnace.MaxSuppHeatCoilFluidFlow > 0.0) {
                         furnace.MaxSuppHeatCoilFluidFlow *= Fluid::GetSteam(state)->getSatDensity(state, 100.0, 1.0, getAirLoopHVACHeatCoolInput);
                     }

                     furnace.SuppHeatCoilAirInletNode = SteamCoils::GetCoilAirInletNode(state, furnace.SuppHeatCoilNum);
                     furnace.SuppHeatCoilAirOutletNode = SteamCoils::GetCoilAirOutletNode(state, furnace.SuppHeatCoilNum);
                 }

            } else {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(12), Alphas(12));
                ErrorsFound = true;
            } // IF (Furnace(FurnaceNum)%HeatingCoilType_Num == Coil_HeatingGasOrOtherFuel .OR. &, etc.

            furnace.fanPlace = static_cast<HVAC::FanPlace>(getEnumValue(HVAC::fanPlaceNamesUC, Alphas(14)));
            assert(furnace.fanPlace != HVAC::FanPlace::Invalid);

            if (lAlphaBlanks(15)) {
                furnace.fanOp = HVAC::FanOp::Cycling;
                if (furnace.fanType != HVAC::FanType::OnOff) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, furnace.Name));
                    ShowContinueError(state, format("{} = {}", cAlphaFields(6), Alphas(6)));
                    ShowContinueError(state, format("Fan type must be Fan:OnOff when {} = Blank.", cAlphaFields(15)));
                    ErrorsFound = true;
                }
            } else if ((furnace.fanOpModeSched = Sched::GetSchedule(state, Alphas(15))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(15), Alphas(15));
                ErrorsFound = true;
            }

            if (furnace.fanType == HVAC::FanType::Constant &&
                furnace.fanOpModeSched != nullptr &&
                !furnace.fanOpModeSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 1.0)) {
                Sched::ShowSevereBadMinMax( // MAKE THIS INSANE FORMATTING STOP, I'M BEGGING YOU
                    state,
                    eoh,
                    cAlphaFields(15),
                    Alphas(15),
                    Clusive::In,
                    0.0,
                    Clusive::In,
                    1.0,
                    format("For {} = {}, fan operating mode must be continuous (schedule values > 0)", cAlphaFields(7), Alphas(7)));
                ErrorsFound = true;
            }

            // Dehumidification Control Type
            if (Util::SameString(Alphas(16), "None") || Util::SameString(Alphas(16), "Multimode") || Util::SameString(Alphas(16), "CoolReheat")) {
                AirNodeFound = false;
                if (Util::SameString(Alphas(16), "Multimode")) {
                    furnace.DehumidControlType_Num = DehumidificationControlMode::Multimode;
                    furnace.Humidistat = true;
                    if (furnace.coolCoilType != HVAC::CoilType::CoolingDXHXAssisted) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(16), Alphas(16)));
                        ShowContinueError(state, "Multimode control must be used with a Heat Exchanger Assisted Cooling Coil.");
                        ErrorsFound = true;
                    }
                } else if (Util::SameString(Alphas(16), "CoolReheat")) {
                    furnace.DehumidControlType_Num = DehumidificationControlMode::CoolReheat;
                    furnace.Humidistat = true;
                } else if (Util::SameString(Alphas(16), "None")) {
                    furnace.DehumidControlType_Num = DehumidificationControlMode::None;
                    furnace.Humidistat = false;
                }
                if (furnace.Humidistat) {
                    for (HStatZoneNum = 1; HStatZoneNum <= state.dataZoneCtrls->NumHumidityControlZones; ++HStatZoneNum) {
                        if (state.dataZoneCtrls->HumidityControlZone(HStatZoneNum).ActualZoneNum != furnace.ControlZoneNum) continue;
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
                    furnace.Humidistat = false;
                    furnace.DehumidControlType_Num = DehumidificationControlMode::None;
                }
            }

            // Check node names for child components
            if (furnace.fanPlace == HVAC::FanPlace::BlowThru) {
                if (furnace.FanInletNode != furnace.FurnaceInletNode) {
                    ShowSevereCustom(state, eoh, "When a blow through fan is specified, the fan inlet node name must be the same as the unitary system inlet node name.");
                    ShowContinueError(state, format("...Fan inlet node name            = {}", s_node->NodeID(furnace.FanInletNode)));
                    ShowContinueError(state, format("...Unitary system inlet node name = {}", s_node->NodeID(furnace.FurnaceInletNode)));
                    ErrorsFound = true;
                }
                if (furnace.FanOutletNode != furnace.CoolCoilAirInletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, "When a blow through fan is specified, the fan outlet node name must be the same as the cooling coil inlet node name.");
                    ShowContinueError(state, format("...Fan outlet node name         = {}", s_node->NodeID(furnace.FanOutletNode)));
                    ShowContinueError(state, format("...Cooling coil inlet node name = {}", s_node->NodeID(furnace.CoolCoilAirInletNode)));
                    ErrorsFound = true;
                }
                if (furnace.CoolCoilAirOutletNode != furnace.HeatCoilAirInletNode) {
                    ShowSevereCustom(state, eoh, "The cooling coil outlet node name must be the same as the heating coil inlet node name.");
                    ShowContinueError(state, format("...Cooling coil outlet node name = {}", s_node->NodeID(furnace.CoolCoilAirOutletNode)));
                    ShowContinueError(state, format("...Heating coil inlet node name  = {}", s_node->NodeID(furnace.HeatCoilAirInletNode)));
                    ErrorsFound = true;
                }
                if (furnace.HeatCoilAirOutletNode != furnace.SuppHeatCoilAirInletNode) {
                    ShowSevereCustom(state, eoh, "When a blow through fan is specified, the heating coil outlet node name must be the same as the supplemental "
                                     "heating coil inlet node name.");
                    ShowContinueError(state, format("...Heating coil outlet node name              = {}", s_node->NodeID(furnace.HeatCoilAirOutletNode)));
                    ShowContinueError(state, format("...Supplemental heating coil inlet node name  = {}", s_node->NodeID(furnace.SuppHeatCoilAirInletNode)));
                    ErrorsFound = true;
                }
                if (furnace.SuppHeatCoilAirOutletNode != furnace.FurnaceOutletNode) {
                    ShowSevereCustom(state, eoh, "The supplemental heating coil outlet node name must be the same as the unitary system outlet node name.");
                    ShowContinueError(state, format("...Supplemental heating coil outlet node name = {}", s_node->NodeID(furnace.SuppHeatCoilAirOutletNode)));
                    ShowContinueError(state, format("...Unitary system outlet node name            = {}", s_node->NodeID(furnace.FurnaceOutletNode)));
                    ErrorsFound = true;
                }
            } else {
                if (furnace.CoolCoilAirInletNode != furnace.FurnaceInletNode) {
                    ShowSevereCustom(state, eoh, 
                                     "When a draw through fan is specified, the cooling coil inlet node name must be the same as the unitary system "
                                     "inlet node name.");
                    ShowContinueError(state, format("...Cooling coil inlet node name   = {}", s_node->NodeID(furnace.CoolCoilAirInletNode)));
                    ShowContinueError(state, format("...Unitary system inlet node name = {}", s_node->NodeID(furnace.FurnaceInletNode)));
                    ErrorsFound = true;
                }
                if (furnace.CoolCoilAirOutletNode != furnace.HeatCoilAirInletNode) {
                    ShowSevereCustom(state, eoh, "The cooling coil outlet node name must be the same as the heating coil inlet node name.");
                    ShowContinueError(state, format("...Cooling coil outlet node name = {}", s_node->NodeID(furnace.CoolCoilAirOutletNode)));
                    ShowContinueError(state, format("...Heating coil inlet node name  = {}", s_node->NodeID(furnace.HeatCoilAirInletNode)));
                    ErrorsFound = true;
                }
                if (furnace.HeatCoilAirOutletNode != furnace.FanInletNode) {
                    ShowSevereCustom(state, eoh, "When a draw through fan is specified, the heating coil outlet node name must be the same as the fan inlet node name.");
                    ShowContinueError(state, format("...Heating coil outlet node name = {}", s_node->NodeID(furnace.HeatCoilAirOutletNode)));
                    ShowContinueError(state, format("...Fan inlet node name           = {}", s_node->NodeID(furnace.FanInletNode)));
                    ErrorsFound = true;
                }
                if (furnace.FanOutletNode != furnace.SuppHeatCoilAirInletNode) {
                    ShowSevereCustom(state, eoh, "When a draw through fan is specified, the fan outlet node name must be the same as the supplemental heating "
                                    "coil inlet node name.");
                    ShowContinueError(state, format("...Fan outlet node name                       = {}", s_node->NodeID(furnace.FanOutletNode)));
                    ShowContinueError(state, format("...Supplemental heating coil inlet node name  = {}", s_node->NodeID(furnace.SuppHeatCoilAirInletNode)));
                    ErrorsFound = true;
                }
                if (furnace.SuppHeatCoilAirOutletNode != furnace.FurnaceOutletNode) {
                    ShowSevereCustom(state, eoh, "The supplemental heating coil outlet node name must be the same as the unitary system outlet node name.");
                    ShowContinueError(state, format("...Supplemental heating coil outlet node name = {}", s_node->NodeID(furnace.SuppHeatCoilAirOutletNode)));
                    ShowContinueError(state, format("...Unitary system outlet node name            = {}", s_node->NodeID(furnace.FurnaceOutletNode)));
                    ErrorsFound = true;
                }
            }

            // Add component sets array
            if (furnace.fanPlace == HVAC::FanPlace::BlowThru) {
                CompSetFanInlet = Alphas(3);
                CompSetCoolInlet = "UNDEFINED";
            } else {
                CompSetFanInlet = "UNDEFINED";
                CompSetCoolInlet = Alphas(3);
            }
            BranchNodeConnections::SetUpCompSets(state, CurrentModuleObject, Alphas(1), Alphas(6), Alphas(7), CompSetFanInlet, "UNDEFINED");

            // Add DX cooling coil to component sets array
            if (furnace.isIHP) {
                BranchNodeConnections::SetUpCompSets(
                    state, CurrentModuleObject, Alphas(1), Alphas(10), Alphas(11) + " Cooling Coil", CompSetCoolInlet, "UNDEFINED");
            } else {
                BranchNodeConnections::SetUpCompSets(state, CurrentModuleObject, Alphas(1), Alphas(10), Alphas(11), CompSetCoolInlet, "UNDEFINED");
            }
            // Add DX heating coil to component sets array
            if (furnace.isIHP) {
                BranchNodeConnections::SetUpCompSets(
                    state, CurrentModuleObject, Alphas(1), Alphas(8), Alphas(9) + " Heating Coil", "UNDEFINED", "UNDEFINED");
            } else {
                BranchNodeConnections::SetUpCompSets(state, CurrentModuleObject, Alphas(1), Alphas(8), Alphas(9), "UNDEFINED", "UNDEFINED");
            }

            // Add supplemental heating coil to component sets array
            BranchNodeConnections::SetUpCompSets(state, CurrentModuleObject, Alphas(1), Alphas(12), Alphas(13), "UNDEFINED", Alphas(4));

            furnace.MaxCoolAirVolFlow = Numbers(1);
            if (furnace.MaxCoolAirVolFlow <= 0 && furnace.MaxCoolAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {:.7T}", cNumericFields(1), Numbers(1)));
                ErrorsFound = true;
            }

            furnace.MaxHeatAirVolFlow = Numbers(2);
            if (furnace.MaxHeatAirVolFlow <= 0 && furnace.MaxHeatAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {:.7T}", cNumericFields(2), Numbers(2)));
                ErrorsFound = true;
            }

            furnace.MaxNoCoolHeatAirVolFlow = Numbers(3);
            if (furnace.MaxNoCoolHeatAirVolFlow < 0 && furnace.MaxNoCoolHeatAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {:.7T}", cNumericFields(3), Numbers(3)));
                ErrorsFound = true;
            }

            if (furnace.fanOpModeSched != nullptr) {
                if (!furnace.fanOpModeSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 0.0)) { // Autodesk:Note Range is 0 to 0?
                    //           set air flow control mode:
                    //             UseCompressorOnFlow = operate at last cooling or heating air flow requested when compressor is off
                    //             UseCompressorOffFlow = operate at value specified by user
                    //           AirFlowControl only valid if fan opmode = ContFanCycComp
                    if (furnace.MaxNoCoolHeatAirVolFlow == 0.0) {
                        furnace.AirFlowControl = AirFlowControlConstFan::UseCompressorOnFlow;
                    } else {
                        furnace.AirFlowControl = AirFlowControlConstFan::UseCompressorOffFlow;
                    }
                }
            }

            if (Numbers(1) != DataSizing::AutoSize && Numbers(2) != DataSizing::AutoSize && Numbers(3) != DataSizing::AutoSize) {
                furnace.DesignFanVolFlowRate = max(Numbers(1), Numbers(2), Numbers(3));
            } else {
                furnace.DesignFanVolFlowRate = DataSizing::AutoSize;
            }

            if (furnace.heatCoilType == HVAC::CoilType::HeatingDXVariableSpeed) {
                // We've done the IHP thing, HeatCoilNum and CoolCoilNum should be properly set
                furnace.MaxHeatAirVolFlow = VariableSpeedCoils::GetCoilAirFlowRate(state, furnace.HeatCoilNum);
                furnace.MaxCoolAirVolFlow = VariableSpeedCoils::GetCoilAirFlowRate(state, furnace.CoolCoilNum);

                furnace.MaxNoCoolHeatAirVolFlow = min(furnace.MaxHeatAirVolFlow, furnace.MaxCoolAirVolFlow);
                if (furnace.MaxHeatAirVolFlow != DataSizing::AutoSize && furnace.MaxCoolAirVolFlow != DataSizing::AutoSize) {
                    furnace.DesignFanVolFlowRate = max(furnace.MaxHeatAirVolFlow, furnace.MaxCoolAirVolFlow);
                } else {
                    furnace.DesignFanVolFlowRate = DataSizing::AutoSize;
                }
            }

            if (furnace.ActualFanVolFlowRate != DataSizing::AutoSize) {
                if (furnace.ActualFanVolFlowRate < furnace.MaxCoolAirVolFlow && furnace.MaxCoolAirVolFlow != DataSizing::AutoSize) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(
                        state,
                        format("... air flow rate = {:.7T} in fan object {} is less than the maximum HVAC system air flow rate in cooling mode.",
                               furnace.ActualFanVolFlowRate,
                               FanName));
                    ShowContinueError(state, format(" The {} is reset to the fan flow rate and the simulation continues.", cNumericFields(1)));
                    furnace.MaxCoolAirVolFlow = furnace.ActualFanVolFlowRate;
                    furnace.DesignFanVolFlowRate = furnace.ActualFanVolFlowRate;
                }
                if (furnace.ActualFanVolFlowRate < furnace.MaxHeatAirVolFlow && furnace.MaxHeatAirVolFlow != DataSizing::AutoSize) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(
                        state,
                        format("... air flow rate = {:.7T} in fan object {} is less than the maximum HVAC system air flow rate in heating mode.",
                               furnace.ActualFanVolFlowRate,
                               FanName));
                    ShowContinueError(state, format(" The {} is reset to the fan flow rate and the simulation continues.", cNumericFields(2)));
                    furnace.MaxHeatAirVolFlow = furnace.ActualFanVolFlowRate;
                    furnace.DesignFanVolFlowRate = furnace.ActualFanVolFlowRate;
                }
            }

            // Set heating convergence tolerance
            furnace.HeatingConvergenceTolerance = 0.001;

            // Set cooling convergence tolerance
            furnace.CoolingConvergenceTolerance = 0.001;

            // Set the furnace max outlet temperature
            furnace.DesignMaxOutletTemp = Numbers(4);

            // Set maximum supply air temperature for supplemental heating coil
            furnace.MaxOATSuppHeat = Numbers(5);
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchDXHeatCoilSuppHiT, furnace.HeatCoilName, furnace.MaxOATSuppHeat);
            furnace.HeatSizingRatio = Numbers(6);
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchDXHeatCoilSizingRatio, furnace.HeatCoilName, furnace.HeatSizingRatio);

            // set minimum outdoor temperature for compressor operation
            SetMinOATCompressor(state, FurnaceNum, cCurrentModuleObject, ErrorsFound);

        } // End of the Unitary System HeatPump Loop

        // Get the Input for the Water to Air Heat Pump (UnitarySystem:HeatPump:WaterToAir)
        for (int HeatPumpNum = 1; HeatPumpNum <= NumWaterToAirHeatPump; ++HeatPumpNum) {

            CurrentModuleObject = "AirLoopHVAC:UnitaryHeatPump:WaterToAir";

            FurnaceNum = NumHeatOnly + NumHeatCool + NumUnitaryHeatOnly + NumUnitaryHeatCool + NumHeatPump + HeatPumpNum;
            auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);
            furnace.iterationMode.allocate(3);

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

            furnace.type = HVAC::UnitarySysType::Unitary_HeatPump_WaterToAir;
            furnace.Name = Alphas(1);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, furnace.Name};

            if (lAlphaBlanks(2)) {
                furnace.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((furnace.availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ErrorsFound = true;
            }

            furnace.FurnaceInletNode =
                NodeInputManager::GetOnlySingleNode(state,
                                                    Alphas(3),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatPumpWaterToAir,
                                                    Alphas(1),
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Inlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);

            furnace.FurnaceOutletNode =
                NodeInputManager::GetOnlySingleNode(state,
                                                    Alphas(4),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatPumpWaterToAir,
                                                    Alphas(1),
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Outlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);

            BranchNodeConnections::TestCompSet(state, CurrentModuleObject, Alphas(1), Alphas(3), Alphas(4), "Air Nodes");

            // Get the Controlling Zone or Location of the Furnace Thermostat
            furnace.ControlZoneNum = Util::FindItemInList(Alphas(5), state.dataHeatBal->Zone);
            if (furnace.ControlZoneNum == 0) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(5), Alphas(5)));
                ErrorsFound = true;
            }

            // Get the node number for the zone with the thermostat
            if (furnace.ControlZoneNum > 0) {
                AirNodeFound = false;
                AirLoopFound = false;
                int ControlledZoneNum = furnace.ControlZoneNum;
                //             Find the controlled zone number for the specified thermostat location
                furnace.NodeNumOfControlledZone = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ZoneNode;
                //             Determine if furnace is on air loop served by the thermostat location specified
                for (int zoneInNode = 1; zoneInNode <= state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).NumInletNodes; ++zoneInNode) {
                    int AirLoopNumber = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNodeAirLoopNum(zoneInNode);
                    furnace.airloopNum = AirLoopNumber;
                    if (AirLoopNumber > 0) {
                        for (int BranchNum = 1; BranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).NumBranches; ++BranchNum) {
                            for (int CompNum = 1;
                                 CompNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).TotalComponents;
                                 ++CompNum) {
                                if (!Util::SameString(state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).Comp(CompNum).Name,
                                                      Alphas(1)) ||
                                    !Util::SameString(
                                        state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).Comp(CompNum).TypeOf,
                                        CurrentModuleObject)) {
                                    continue;
                                }
                                AirLoopFound = true;
                                furnace.ZoneInletNode = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNode(zoneInNode);
                                break;
                            }
                            if (AirLoopFound) {
                                break;
                            }
                        }
                        for (TstatZoneNum = 1; TstatZoneNum <= state.dataZoneCtrls->NumTempControlledZones; ++TstatZoneNum) {
                            if (state.dataZoneCtrls->TempControlledZone(TstatZoneNum).ActualZoneNum != furnace.ControlZoneNum) continue;
                            AirNodeFound = true;
                        }
                        for (TstatZoneNum = 1; TstatZoneNum <= state.dataZoneCtrls->NumComfortControlledZones; ++TstatZoneNum) {
                            if (state.dataZoneCtrls->ComfortControlledZone(TstatZoneNum).ActualZoneNum != furnace.ControlZoneNum) continue;
                            AirNodeFound = true;
                        }
                    }
                    if (AirLoopFound) {
                        break;
                    }
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
                    ShowContinueError(state, "Did not find correct AirLoopHVAC.");
                    ShowContinueError(state, format("Specified {} = {}", cAlphaFields(5), Alphas(5)));
                    ErrorsFound = true;
                }
            }

            // Get fan data
            FanName = Alphas(7);
            errFlag = false;
            furnace.fanType = static_cast<HVAC::FanType>(getEnumValue(HVAC::fanTypeNamesUC, Alphas(6)));

            if (furnace.fanType != HVAC::FanType::OnOff) {
                ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(6), Alphas(6)));
                ErrorsFound = true;

            } else if ((furnace.FanIndex = Fans::GetFanIndex(state, FanName)) == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(7), FanName);
                ErrorsFound = true;

            } else {
                auto *fan = state.dataFans->fans(furnace.FanIndex);
                furnace.FanInletNode = fan->inletNodeNum;
                furnace.FanOutletNode = fan->outletNodeNum;
                furnace.fanAvailSched = fan->availSched;
            }

            furnace.heatCoilType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Alphas(8)));
            furnace.HeatCoilName = Alphas(9);

            // Get heating coil type and name data
            if (furnace.heatCoilType == HVAC::CoilType::HeatingWAHP) {
                furnace.HeatCoilNum = WaterToAirHeatPump::GetCoilIndex(state, furnace.HeatCoilName);
                if (furnace.HeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(9), furnace.HeatCoilName);
                    ErrorsFound = true;
                } else {
                    furnace.HeatCoilAirInletNode = WaterToAirHeatPump::GetCoilInletNode(state, furnace.HeatCoilNum);
                    furnace.HeatCoilAirOutletNode = WaterToAirHeatPump::GetCoilOutletNode(state, furnace.HeatCoilNum);
                }
                
            } else if (furnace.heatCoilType == HVAC::CoilType::HeatingWAHPSimple) {
                furnace.HeatCoilNum = WaterToAirHeatPumpSimple::GetCoilIndex(state, furnace.HeatCoilName);
                if (furnace.HeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(9), furnace.HeatCoilName);
                    ErrorsFound = true;
                } else {
                    furnace.HeatCoilAirInletNode = WaterToAirHeatPumpSimple::GetCoilAirInletNode(state, furnace.HeatCoilNum);
                    furnace.HeatCoilAirOutletNode = WaterToAirHeatPumpSimple::GetCoilAirOutletNode(state, furnace.HeatCoilNum);
                }
                
            } else if (furnace.heatCoilType == HVAC::CoilType::HeatingWAHPVariableSpeedEquationFit) {
                furnace.HeatCoilNum = VariableSpeedCoils::GetCoilIndex(state, furnace.HeatCoilName);
                if (furnace.HeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(9), furnace.HeatCoilName);
                    ErrorsFound = true;
                } else {
                    furnace.HeatCoilAirInletNode = VariableSpeedCoils::GetCoilAirInletNode(state, furnace.HeatCoilNum);
                    furnace.HeatCoilAirOutletNode = VariableSpeedCoils::GetCoilAirOutletNode(state, furnace.HeatCoilNum);
                }
            } else {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(8), Alphas(8));
                ErrorsFound = true;
            }

            // Get Cooling Coil Information if available
            furnace.coolCoilType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Alphas(10)));
            furnace.CoolCoilName = Alphas(11);

            if (furnace.coolCoilType == HVAC::CoilType::CoolingWAHP) {
                furnace.CoolCoilNum = WaterToAirHeatPump::GetCoilIndex(state, furnace.CoolCoilName);
                if (furnace.CoolCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(11), furnace.CoolCoilName);
                    ErrorsFound = true;
                } else {
                    furnace.CoolCoilAirInletNode = WaterToAirHeatPump::GetCoilInletNode(state, furnace.CoolCoilNum);
                    furnace.CoolCoilAirOutletNode = WaterToAirHeatPump::GetCoilOutletNode(state, furnace.CoolCoilNum);
                }
                
            } else if (furnace.coolCoilType == HVAC::CoilType::CoolingWAHPSimple) { 
                furnace.CoolCoilNum = WaterToAirHeatPumpSimple::GetCoilIndex(state, furnace.CoolCoilName);
                if (furnace.CoolCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(11), furnace.CoolCoilName);
                    ErrorsFound = true;
                } else {
                    furnace.CoolCoilAirInletNode = WaterToAirHeatPumpSimple::GetCoilAirInletNode(state, furnace.CoolCoilNum);
                    furnace.CoolCoilAirOutletNode = WaterToAirHeatPumpSimple::GetCoilAirOutletNode(state, furnace.CoolCoilNum);
                }
            
            } else if (furnace.coolCoilType == HVAC::CoilType::CoolingWAHPVariableSpeedEquationFit) { 
                furnace.CoolCoilNum = VariableSpeedCoils::GetCoilIndex(state, furnace.CoolCoilName);
                if (furnace.CoolCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(11), furnace.CoolCoilName);
                    ErrorsFound = true;
                } else {
                    furnace.CoolCoilAirInletNode = VariableSpeedCoils::GetCoilAirInletNode(state, furnace.CoolCoilNum);
                    furnace.CoolCoilAirOutletNode = VariableSpeedCoils::GetCoilAirOutletNode(state, furnace.CoolCoilNum);
                }
            } else {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(10), Alphas(10));
                ErrorsFound = true;
            }

            furnace.WaterCyclingMode = (NumAlphas < 18 || lAlphaBlanks(18))
                                               ? HVAC::WaterFlow::Cycling
                                               : static_cast<HVAC::WaterFlow>(getEnumValue(HVAC::waterFlowNamesUC, Alphas(18)));

            // end get water flow mode info
            if (furnace.heatCoilType == HVAC::CoilType::HeatingWAHPSimple &&
                furnace.coolCoilType == HVAC::CoilType::CoolingWAHPSimple) {
                furnace.WatertoAirHPType = WAHPCoilType::Simple;
                WaterToAirHeatPumpSimple::SetSimpleWSHPData(
                    state, furnace.CoolCoilNum, ErrorsFound, furnace.WaterCyclingMode, _, furnace.HeatCoilNum);
            } else if (furnace.heatCoilType == HVAC::CoilType::HeatingWAHP &&
                       furnace.coolCoilType == HVAC::CoilType::CoolingWAHP) {
                furnace.WatertoAirHPType = WAHPCoilType::ParEst;
            } else if (furnace.heatCoilType == HVAC::CoilType::HeatingWAHPVariableSpeedEquationFit &&
                       furnace.coolCoilType == HVAC::CoilType::CoolingWAHPVariableSpeedEquationFit) { 
                furnace.WatertoAirHPType = WAHPCoilType::VarSpeedEquationFit;
                VariableSpeedCoils::SetCoilData(state, furnace.CoolCoilNum, _, furnace.HeatCoilNum);
            } else {
                ShowSevereCustom(state, eoh, "Cooling coil and heating coil should be of same general type");
                ErrorsFound = true;
            }

            // Get supplemental heating coil information
            furnace.suppHeatCoilType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Alphas(12)));
            furnace.SuppHeatCoilName = Alphas(13);

            if (furnace.suppHeatCoilType == HVAC::CoilType::HeatingGasOrOtherFuel ||
                furnace.suppHeatCoilType == HVAC::CoilType::HeatingElectric) {

                furnace.SuppHeatCoilNum = HeatingCoils::GetCoilIndex(state, furnace.SuppHeatCoilName);
                if (furnace.SuppHeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(13), furnace.SuppHeatCoilName);
                    ErrorsFound = true;
                } else {
                    furnace.SuppHeatCoilAirInletNode = HeatingCoils::GetCoilAirInletNode(state, furnace.SuppHeatCoilNum);
                    furnace.SuppHeatCoilAirOutletNode = HeatingCoils::GetCoilAirOutletNode(state, furnace.SuppHeatCoilNum);
                    furnace.DesignSuppHeatingCapacity = HeatingCoils::GetCoilCapacity(state, furnace.SuppHeatCoilNum);
                }
                
            } else if (furnace.suppHeatCoilType == HVAC::CoilType::HeatingWater) {
                furnace.SuppHeatCoilNum = WaterCoils::GetCoilIndex(state, furnace.SuppHeatCoilName);
                if (furnace.SuppHeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(13), furnace.SuppHeatCoilName);
                    ErrorsFound = true;
                } else {
                    furnace.SuppHeatCoilControlNode = WaterCoils::GetCoilWaterInletNode(state, furnace.SuppHeatCoilNum);
                    furnace.MaxSuppHeatCoilFluidFlow = WaterCoils::GetCoilMaxWaterFlowRate(state, furnace.SuppHeatCoilNum);
                    furnace.SuppHeatCoilAirInletNode = WaterCoils::GetCoilAirInletNode(state, furnace.SuppHeatCoilNum);
                    furnace.SuppHeatCoilAirOutletNode = WaterCoils::GetCoilAirOutletNode(state, furnace.SuppHeatCoilNum);
                    HVACControllers::CheckCoilWaterInletNode(state, furnace.SuppHeatCoilControlNode, errFlag);
                    if (!errFlag) { // then did find a controller so that is bad
                        ShowSevereError(state,
                                        format("{} = {} has a conflicting Controller:WaterCoil object", CurrentModuleObject, furnace.Name));
                        ShowContinueError(state, "Hot water coils are controlled directly by unitary and furnace systems.");
                        ShowContinueError(state, "No water coil controller should be input for the coil.");
                        ErrorsFound = true;
                    }
                }

            } else if (furnace.suppHeatCoilType == HVAC::CoilType::HeatingSteam) {
                furnace.SuppHeatCoilNum = SteamCoils::GetCoilIndex(state, furnace.SuppHeatCoilName);
                if (furnace.SuppHeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(13), furnace.SuppHeatCoilName);
                    ErrorsFound = true;
                } else {
                    furnace.SuppHeatCoilControlNode = SteamCoils::GetCoilSteamInletNode(state, furnace.SuppHeatCoilNum);
                    furnace.MaxSuppHeatCoilFluidFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, furnace.SuppHeatCoilNum);
                    if (furnace.MaxSuppHeatCoilFluidFlow > 0.0) {
                        furnace.MaxSuppHeatCoilFluidFlow *= Fluid::GetSteam(state)->getSatDensity(state, 100.0, 1.0, getAirLoopHVACHeatCoolInput);
                    }

                    furnace.SuppHeatCoilAirInletNode = SteamCoils::GetCoilAirInletNode(state, furnace.SuppHeatCoilNum);
                    furnace.SuppHeatCoilAirOutletNode = SteamCoils::GetCoilAirOutletNode(state, furnace.SuppHeatCoilNum);
                }

            } else {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(12), Alphas(12));
                ErrorsFound = true;
            } // IF (Furnace(FurnaceNum)%HeatingCoilType_Num == Coil_HeatingGasOrOtherFuel .OR. &, etc.

            if (lAlphaBlanks(14)) {
                furnace.CondenserNodeNum = 0;
            } else {
                furnace.CondenserNodeNum =
                    NodeInputManager::GetOnlySingleNode(state,
                                                        Alphas(14),
                                                        ErrorsFound,
                                                        DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatPumpWaterToAir,
                                                        Alphas(1),
                                                        DataLoopNode::NodeFluidType::Air,
                                                        DataLoopNode::ConnectionType::OutsideAirReference,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        DataLoopNode::ObjectIsNotParent);
                // need better verification.
                if (!OutAirNodeManager::CheckOutAirNodeNumber(state, furnace.CondenserNodeNum)) {
                    ShowSevereError(state, format("For {} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format(" Node name of outdoor dry-bulb temperature sensor not valid outdoor air node= {}", Alphas(14)));
                    ShowContinueError(state, "...does not appear in an OutdoorAir:NodeList or as an OutdoorAir:Node.");
                    ErrorsFound = true;
                }
            }

            furnace.fanPlace = static_cast<HVAC::FanPlace>(getEnumValue(HVAC::fanPlaceNamesUC, Alphas(15)));
            assert(furnace.fanPlace != HVAC::FanPlace::Invalid);

            if (lAlphaBlanks(16)) {
                furnace.fanOp = HVAC::FanOp::Cycling;
                if (furnace.fanType != HVAC::FanType::OnOff) {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, furnace.Name));
                    ShowContinueError(state, format("{} = {}", cAlphaFields(6), Alphas(6)));
                    ShowContinueError(state, format("Fan type must be Fan:OnOff when {} = Blank.", cAlphaFields(16)));
                    ErrorsFound = true;
                }
            } else if ((furnace.fanOpModeSched = Sched::GetSchedule(state, Alphas(16))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(16), Alphas(16));
                ErrorsFound = true;
            }

            // add the Dehumidification Type
            if (Util::SameString(Alphas(17), "None") || Util::SameString(Alphas(17), "CoolReheat")) {
                AirNodeFound = false;
                if (Util::SameString(Alphas(17), "CoolReheat")) {
                    furnace.DehumidControlType_Num = DehumidificationControlMode::CoolReheat;
                    furnace.Humidistat = true;
                    if (lAlphaBlanks(17)) {
                        ShowWarningError(state, format("{} \"{}\"", CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state,
                                          "Dehumidification control type is assumed to be None since a supplemental reheat coil has not been "
                                          "specified and the simulation continues.");
                        furnace.Humidistat = false;
                        furnace.DehumidControlType_Num = DehumidificationControlMode::None;
                    }
                }
                if (Util::SameString(Alphas(17), "None")) {
                    furnace.DehumidControlType_Num = DehumidificationControlMode::None;
                    furnace.Humidistat = false;
                }
                if (furnace.Humidistat) {
                    for (HStatZoneNum = 1; HStatZoneNum <= state.dataZoneCtrls->NumHumidityControlZones; ++HStatZoneNum) {
                        if (state.dataZoneCtrls->HumidityControlZone(HStatZoneNum).ActualZoneNum != furnace.ControlZoneNum) continue;
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
                    furnace.Humidistat = false;
                    furnace.DehumidControlType_Num = DehumidificationControlMode::None;
                }
            }

            // Add fan to component sets array
            if (furnace.fanPlace == HVAC::FanPlace::BlowThru) {
                CompSetFanInlet = Alphas(3);
                CompSetCoolInlet = "UNDEFINED";
                if (furnace.FanInletNode != furnace.FurnaceInletNode) {
                    ShowSevereCustom(state, eoh, "Mismatch between unitary system inlet node and fan inlet node.");
                    ShowContinueError(state, "..For \"BlowThrough\" fan, the inlet node name for the HeatPump should match the fan inlet node name.");
                    ShowContinueError(state, format("..HeatPump Inlet Node = {}", s_node->NodeID(furnace.FurnaceInletNode)));
                    ShowContinueError(state, format("..Fan Inlet Node      = {}", s_node->NodeID(furnace.FanInletNode)));
                    ErrorsFound = true;
                }
                if (furnace.FanOutletNode != furnace.CoolCoilAirInletNode) {
                    ShowSevereCustom(state, eoh, "Mismatch between fan outlet node and cooling coil inlet node.");
                    ShowContinueError(state, "..For \"BlowThrough\" fan, the fan outlet node name must match the cooling coil inlet node name.");
                    ShowContinueError(state, format("..Fan outlet node         = {}", s_node->NodeID(furnace.FanOutletNode)));
                    ShowContinueError(state, format("..Cooling coil inlet node = {}", s_node->NodeID(furnace.CoolCoilAirInletNode)));
                    ErrorsFound = true;
                }
                if (furnace.CoolCoilAirOutletNode != furnace.HeatCoilAirInletNode) {
                    ShowSevereCustom(state, eoh, "Mismatch between cooling coil outlet node and heating coil inlet node.");
                    ShowContinueError(state, "..The cooling coil outlet node name must match the heating coil inlet node name.");
                    ShowContinueError(state, format("..Cooling coil outlet node = {}", s_node->NodeID(furnace.CoolCoilAirOutletNode)));
                    ShowContinueError(state, format("..Heating coil inlet node  = {}", s_node->NodeID(furnace.HeatCoilAirInletNode)));
                    ErrorsFound = true;
                }
                if (furnace.HeatCoilAirOutletNode != furnace.SuppHeatCoilAirInletNode) {
                    ShowSevereCustom(state, eoh, "Mismatch between heating coil outlet node and supplemental heating coil inlet node.");
                    ShowContinueError(state, "..For \"BlowThrough\" fan, the heating coil outlet node name must match the supplemental heating coil inlet node name.");
                    ShowContinueError(state, format("..Heating coil outlet node             = {}", s_node->NodeID(furnace.HeatCoilAirOutletNode)));
                    ShowContinueError(state, format("..Supplemental heating coil inlet node = {}", s_node->NodeID(furnace.SuppHeatCoilAirInletNode)));
                    ErrorsFound = true;
                }
                if (furnace.SuppHeatCoilAirOutletNode != furnace.FurnaceOutletNode) {
                    ShowSevereCustom(state, eoh, "Mismatch between supplemental heating coil outlet node and HeatPump outlet node.");
                    ShowContinueError(state, "..The supplemental heating coil outlet node name must match the HeatPump outlet node name.");
                    ShowContinueError(state, format("..Supplemental heating coil outlet node = {}", s_node->NodeID(furnace.SuppHeatCoilAirOutletNode)));
                    ShowContinueError(state, format("..HeatPump outlet node                  = {}", s_node->NodeID(furnace.FurnaceOutletNode)));
                    ErrorsFound = true;
                }
            } else {
                CompSetFanInlet = "UNDEFINED";
                CompSetCoolInlet = Alphas(3);
                if (furnace.CoolCoilAirInletNode != furnace.FurnaceInletNode) {
                    ShowSevereCustom(state, eoh, "Mismatch between unitary system inlet node and cooling coil inlet node.");
                    ShowContinueError(state, "..For \"DrawThrough\" fan, the inlet node name for the HeatPump should match the cooling coil inlet node name.");
                    ShowContinueError(state, format("..HeatPump inlet node     = {}", s_node->NodeID(furnace.FurnaceInletNode)));
                    ShowContinueError(state, format("..Cooling coil inlet node = {}", s_node->NodeID(furnace.CoolCoilAirInletNode)));
                    ErrorsFound = true;
                }
                if (furnace.CoolCoilAirOutletNode != furnace.HeatCoilAirInletNode) {
                    ShowSevereCustom(state, eoh, "Mismatch between cooling coil outlet node and heating coil inlet node.");
                    ShowContinueError(state, "..The outlet node name for the cooling coil should match the heating coil inlet node name.");
                    ShowContinueError(state, format("..Cooling coil outlet node = {}", s_node->NodeID(furnace.CoolCoilAirOutletNode)));
                    ShowContinueError(state, format("..Heating coil inlet node  = {}", s_node->NodeID(furnace.HeatCoilAirInletNode)));
                    ErrorsFound = true;
                }
                if (furnace.HeatCoilAirOutletNode != furnace.FanInletNode) {
                    ShowSevereCustom(state, eoh, "Mismatch between heating coil outlet node and fan inlet node.");
                    ShowContinueError(state, "..For \"DrawThrough\" fan, the outlet node name for the heating coil should match the fan inlet node name.");
                    ShowContinueError(state, format("..Heating coil outlet node = {}", s_node->NodeID(furnace.HeatCoilAirOutletNode)));
                    ShowContinueError(state, format("..Fan inlet node           = {}", s_node->NodeID(furnace.FanInletNode)));
                    ErrorsFound = true;
                }
                if (furnace.FanOutletNode != furnace.SuppHeatCoilAirInletNode) {
                    ShowSevereCustom(state, eoh, "Mismatch between fan outlet node and supplemental heating coil inlet node.");
                    ShowContinueError(state, "For \"DrawThrough\" fan, the outlet node name for the fan should match the supplemental heating coil inlet node name.");
                    ShowContinueError(state, format("..Fan outlet node                      = {}", s_node->NodeID(furnace.FanOutletNode)));
                    ShowContinueError(state, format("..Supplemental heating coil inlet node = {}", s_node->NodeID(furnace.SuppHeatCoilAirInletNode)));
                    ErrorsFound = true;
                }
                if (furnace.SuppHeatCoilAirOutletNode != furnace.FurnaceOutletNode) {
                    ShowSevereCustom(state, eoh, "Mismatch between supplemental heating coil outlet node and HeatPump outlet node."
                                     "..The supplemental heating coil outlet node name must match the HeatPump outlet node name.");
                    ShowContinueError(state, format("..Supplemental heating coil outlet node = {}", s_node->NodeID(furnace.SuppHeatCoilAirOutletNode)));
                    ShowContinueError(state, format("..HeatPump outlet node                  = {}", s_node->NodeID(furnace.FurnaceOutletNode)));
                    ErrorsFound = true;
                }
            }
            //  (Set up validation here for the fan or cooling coil inlet?)
            BranchNodeConnections::SetUpCompSets(state, CurrentModuleObject, Alphas(1), Alphas(6), Alphas(7), CompSetFanInlet, "UNDEFINED");

            // Add DX heating coil to component sets array
            BranchNodeConnections::SetUpCompSets(state, CurrentModuleObject, Alphas(1), Alphas(8), Alphas(9), "UNDEFINED", "UNDEFINED");

            // Add DX cooling coil to component sets array
            BranchNodeConnections::SetUpCompSets(state, CurrentModuleObject, Alphas(1), Alphas(10), Alphas(11), CompSetCoolInlet, "UNDEFINED");

            // Add supplemental heating coil to component sets array
            BranchNodeConnections::SetUpCompSets(state, CurrentModuleObject, Alphas(1), Alphas(12), Alphas(13), "UNDEFINED", Alphas(4));

            // Set the Design Fan Volume Flow Rate
            furnace.ActualFanVolFlowRate = state.dataFans->fans(furnace.FanIndex)->maxAirFlowRate;

            // CR8094 - simple water to air heat pump MUST operate at the same flow rate specified in the coil objects
            //        Furnace(FurnaceNum)%DesignFanVolFlowRate = Numbers(1)
            //        Furnace(FurnaceNum)%MaxHeatAirVolFlow    = Furnace(FurnaceNum)%DesignFanVolFlowRate
            //        Furnace(FurnaceNum)%MaxCoolAirVolFlow    = Furnace(FurnaceNum)%DesignFanVolFlowRate

            // parameter estimate model only specifies air flow rate in parent object
            if (furnace.heatCoilType == HVAC::CoilType::HeatingWAHP) {
                furnace.MaxHeatAirVolFlow = Numbers(1);
                furnace.MaxCoolAirVolFlow = Numbers(1);

            // simple HP model specifies air flow rate in both the parent and child coils. Use coil air flow rates.
            // simple HP model air flow rate input will not be used.
            } else if (furnace.heatCoilType == HVAC::CoilType::HeatingWAHPSimple) {
                furnace.MaxHeatAirVolFlow = WaterToAirHeatPumpSimple::GetCoilAirFlowRate(state, furnace.HeatCoilNum);
                furnace.MaxCoolAirVolFlow = WaterToAirHeatPumpSimple::GetCoilAirFlowRate(state, furnace.CoolCoilNum);

            } else if (furnace.heatCoilType == HVAC::CoilType::HeatingWAHPVariableSpeedEquationFit) {
                furnace.MaxHeatAirVolFlow = VariableSpeedCoils::GetCoilAirFlowRate(state, furnace.HeatCoilNum);
                furnace.MaxCoolAirVolFlow = VariableSpeedCoils::GetCoilAirFlowRate(state, furnace.CoolCoilNum);
            }

            furnace.MaxNoCoolHeatAirVolFlow = min(furnace.MaxHeatAirVolFlow, furnace.MaxCoolAirVolFlow);
            if (furnace.MaxHeatAirVolFlow != DataSizing::AutoSize && furnace.MaxCoolAirVolFlow != DataSizing::AutoSize) {
                furnace.DesignFanVolFlowRate = max(furnace.MaxHeatAirVolFlow, furnace.MaxCoolAirVolFlow);
            } else {
                furnace.DesignFanVolFlowRate = DataSizing::AutoSize;
            }

            furnace.AirFlowControl = AirFlowControlConstFan::UseCompressorOnFlow;

            if (furnace.ActualFanVolFlowRate != DataSizing::AutoSize && furnace.DesignFanVolFlowRate != DataSizing::AutoSize) {
                if (furnace.DesignFanVolFlowRate > furnace.ActualFanVolFlowRate) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, "... has a Cooling or Heating Air Flow Rate > Max Fan Volume Flow Rate, should be <=.");
                    ShowContinueError(state,
                                      format("... Entered value={:.2R}... Fan [{}:{}] Max Value={:.2R}",
                                             furnace.DesignFanVolFlowRate,
                                             HVAC::fanTypeNames[(int)furnace.fanType],
                                             FanName,
                                             furnace.ActualFanVolFlowRate));
                }
            }
            if (furnace.ActualFanVolFlowRate != DataSizing::AutoSize && furnace.DesignFanVolFlowRate != DataSizing::AutoSize) {
                if (furnace.DesignFanVolFlowRate <= 0.0) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, "... has a Design Fan Flow Rate <= 0.0, it must be >0.0");
                    ShowContinueError(state, format("... Entered value={:.2R}", furnace.DesignFanVolFlowRate));
                    ErrorsFound = true;
                }
            }

            // Set the heat pump heating coil capacity
            //  Get from coil module.
            if (furnace.heatCoilType == HVAC::CoilType::HeatingWAHP) {
                furnace.DesignHeatingCapacity = WaterToAirHeatPump::GetCoilCapacity(state, furnace.HeatCoilNum);
            } else if (furnace.heatCoilType == HVAC::CoilType::HeatingWAHPSimple) {
                furnace.DesignHeatingCapacity = WaterToAirHeatPumpSimple::GetCoilCapacity(state, furnace.HeatCoilNum);
            } else if (furnace.heatCoilType == HVAC::CoilType::HeatingWAHPVariableSpeedEquationFit) {
                furnace.DesignHeatingCapacity = VariableSpeedCoils::GetCoilCapacity(state, furnace.HeatCoilNum);
            }
            // Set the heat pump heating coil convergence
            furnace.HeatingConvergenceTolerance = Numbers(2);
            // Set the heat pump cooling coil capacity (Total capacity)
            //  Get from coil module.
            if (furnace.coolCoilType == HVAC::CoilType::CoolingWAHP) {
                furnace.DesignCoolingCapacity = WaterToAirHeatPump::GetCoilCapacity(state, furnace.CoolCoilNum);
            } else if (furnace.coolCoilType == HVAC::CoilType::CoolingWAHPSimple) {
                furnace.DesignCoolingCapacity = WaterToAirHeatPumpSimple::GetCoilCapacity(state, furnace.CoolCoilNum);
            } else if (furnace.coolCoilType == HVAC::CoilType::CoolingWAHPVariableSpeedEquationFit) {
                furnace.DesignCoolingCapacity = VariableSpeedCoils::GetCoilCapacity(state, furnace.CoolCoilNum);
            }
            // Set the heat pump cooling coil convergence
            furnace.CoolingConvergenceTolerance = Numbers(3);

            // Set the heatpump design supplemental heating capacity
            //  Get from coil module.

            // Set the heatpump max outlet temperature
            furnace.DesignMaxOutletTemp = Numbers(4);

            // Set maximum supply air temperature for supplemental heating coil
            furnace.MaxOATSuppHeat = Numbers(5);
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchDXHeatCoilSuppHiT, furnace.HeatCoilName, furnace.MaxOATSuppHeat);
            furnace.HeatSizingRatio = Numbers(6);
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchDXHeatCoilSizingRatio, furnace.HeatCoilName, furnace.HeatSizingRatio);

            // set minimum outdoor temperature for compressor operation
            SetMinOATCompressor(state, FurnaceNum, cCurrentModuleObject, ErrorsFound);

        } // End of the Unitary System WaterToAirHeatPump Loop

        Alphas.deallocate();
        Numbers.deallocate();

        if (ErrorsFound) {
            ShowFatalError(state, "Errors found in getting Furnace or Unitary System input.");
        }

        for (int HeatOnlyNum = 1; HeatOnlyNum <= NumHeatOnly; ++HeatOnlyNum) {
            FurnaceNum = HeatOnlyNum;
            auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);
            // Setup Report variables for the Furnace that are not reported in the components themselves
            SetupOutputVariable(state,
                                "Unitary System Fan Part Load Ratio",
                                Constant::Units::None,
                                furnace.FanPartLoadRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                furnace.Name);
            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSActuator(state,
                                 "AirLoopHVAC:Unitary:Furnace:HeatOnly",
                                 furnace.Name,
                                 "Autosized Supply Air Flow Rate",
                                 "[m3/s]",
                                 furnace.DesignFanVolFlowRateEMSOverrideOn,
                                 furnace.DesignFanVolFlowRateEMSOverrideValue);
            }
        }

        for (int UnitaryHeatOnlyNum = NumHeatOnly + 1; UnitaryHeatOnlyNum <= NumHeatOnly + NumUnitaryHeatOnly; ++UnitaryHeatOnlyNum) {
            FurnaceNum = UnitaryHeatOnlyNum;
            auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);
            // Setup Report variables for Unitary System that are not reported in the components themselves
            SetupOutputVariable(state,
                                "Unitary System Fan Part Load Ratio",
                                Constant::Units::None,
                                furnace.FanPartLoadRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                furnace.Name);
            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSActuator(state,
                                 "AirLoopHVAC:UnitaryHeatOnly",
                                 furnace.Name,
                                 "Autosized Supply Air Flow Rate",
                                 "[m3/s]",
                                 furnace.DesignFanVolFlowRateEMSOverrideOn,
                                 furnace.DesignFanVolFlowRateEMSOverrideValue);
            }
        }

        for (int HeatCoolNum = NumHeatOnly + NumUnitaryHeatOnly + 1; HeatCoolNum <= NumHeatOnly + NumUnitaryHeatOnly + NumHeatCool; ++HeatCoolNum) {
            FurnaceNum = HeatCoolNum;
            auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);
            // Setup Report variables for the Furnace that are not reported in the components themselves
            SetupOutputVariable(state,
                                "Unitary System Fan Part Load Ratio",
                                Constant::Units::None,
                                furnace.FanPartLoadRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                furnace.Name);
            SetupOutputVariable(state,
                                "Unitary System Compressor Part Load Ratio",
                                Constant::Units::None,
                                furnace.CompPartLoadRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                furnace.Name);

            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSActuator(state,
                                 "AirLoopHVAC:Unitary:Furnace:HeatCool",
                                 furnace.Name,
                                 "Autosized Supply Air Flow Rate",
                                 "[m3/s]",
                                 furnace.DesignFanVolFlowRateEMSOverrideOn,
                                 furnace.DesignFanVolFlowRateEMSOverrideValue);
                SetupEMSActuator(state,
                                 "AirLoopHVAC:Unitary:Furnace:HeatCool",
                                 furnace.Name,
                                 "Autosized Supply Air Flow Rate During Cooling Operation",
                                 "[m3/s]",
                                 furnace.MaxCoolAirVolFlowEMSOverrideOn,
                                 furnace.MaxCoolAirVolFlowEMSOverrideValue);
                SetupEMSActuator(state,
                                 "AirLoopHVAC:Unitary:Furnace:HeatCool",
                                 furnace.Name,
                                 "Autosized Supply Air Flow Rate During Heating Operation",
                                 "[m3/s]",
                                 furnace.MaxHeatAirVolFlowEMSOverrideOn,
                                 furnace.MaxHeatAirVolFlowEMSOverrideValue);
                SetupEMSActuator(state,
                                 "AirLoopHVAC:Unitary:Furnace:HeatCool",
                                 furnace.Name,
                                 "Autosized Supply Air Flow Rate During No Heating or Cooling Operation",
                                 "[m3/s]",
                                 furnace.MaxNoCoolHeatAirVolFlowEMSOverrideOn,
                                 furnace.MaxNoCoolHeatAirVolFlowEMSOverrideValue);
            }
        }

        for (int UnitaryHeatCoolNum = NumHeatOnly + NumHeatCool + NumUnitaryHeatOnly + 1;
             UnitaryHeatCoolNum <= NumHeatOnly + NumHeatCool + NumUnitaryHeatOnly + NumUnitaryHeatCool;
             ++UnitaryHeatCoolNum) {
            FurnaceNum = UnitaryHeatCoolNum;
            auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);
            // Setup Report variables for Unitary System that are not reported in the components themselves
            SetupOutputVariable(state,
                                "Unitary System Fan Part Load Ratio",
                                Constant::Units::None,
                                furnace.FanPartLoadRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                furnace.Name);
            SetupOutputVariable(state,
                                "Unitary System Compressor Part Load Ratio",
                                Constant::Units::None,
                                furnace.CompPartLoadRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                furnace.Name);
            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSActuator(state,
                                 "AirLoopHVAC:UnitaryHeatCool",
                                 furnace.Name,
                                 "Autosized Supply Air Flow Rate",
                                 "[m3/s]",
                                 furnace.DesignFanVolFlowRateEMSOverrideOn,
                                 furnace.DesignFanVolFlowRateEMSOverrideValue);
                SetupEMSActuator(state,
                                 "AirLoopHVAC:UnitaryHeatCool",
                                 furnace.Name,
                                 "Autosized Supply Air Flow Rate During Cooling Operation",
                                 "[m3/s]",
                                 furnace.MaxCoolAirVolFlowEMSOverrideOn,
                                 furnace.MaxCoolAirVolFlowEMSOverrideValue);
                SetupEMSActuator(state,
                                 "AirLoopHVAC:UnitaryHeatCool",
                                 furnace.Name,
                                 "Autosized Supply Air Flow Rate During Heating Operation",
                                 "[m3/s]",
                                 furnace.MaxHeatAirVolFlowEMSOverrideOn,
                                 furnace.MaxHeatAirVolFlowEMSOverrideValue);
                SetupEMSActuator(state,
                                 "AirLoopHVAC:UnitaryHeatCool",
                                 furnace.Name,
                                 "Autosized Supply Air Flow Rate During No Heating or Cooling Operation",
                                 "[m3/s]",
                                 furnace.MaxNoCoolHeatAirVolFlowEMSOverrideOn,
                                 furnace.MaxNoCoolHeatAirVolFlowEMSOverrideValue);
            }
        }

        for (int HeatPumpNum = NumHeatOnly + NumHeatCool + NumUnitaryHeatOnly + NumUnitaryHeatCool + 1;
             HeatPumpNum <= state.dataFurnaces->NumFurnaces - NumWaterToAirHeatPump;
             ++HeatPumpNum) {
            FurnaceNum = HeatPumpNum;
            auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);
            // Setup Report variables for Unitary System that are not reported in the components themselves
            SetupOutputVariable(state,
                                "Unitary System Fan Part Load Ratio",
                                Constant::Units::None,
                                furnace.FanPartLoadRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                furnace.Name);
            SetupOutputVariable(state,
                                "Unitary System Compressor Part Load Ratio",
                                Constant::Units::None,
                                furnace.CompPartLoadRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                furnace.Name);
            SetupOutputVariable(state,
                                "Unitary System Dehumidification Induced Heating Demand Rate",
                                Constant::Units::W,
                                furnace.DehumidInducedHeatingDemandRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                furnace.Name);

            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSActuator(state,
                                 "AirLoopHVAC:UnitaryHeatPump:AirToAir",
                                 furnace.Name,
                                 "Autosized Supply Air Flow Rate",
                                 "[m3/s]",
                                 furnace.DesignFanVolFlowRateEMSOverrideOn,
                                 furnace.DesignFanVolFlowRateEMSOverrideValue);
            }
        }

        for (int HeatPumpNum = NumHeatOnly + NumHeatCool + NumUnitaryHeatOnly + NumUnitaryHeatCool + NumHeatPump + 1;
             HeatPumpNum <= state.dataFurnaces->NumFurnaces;
             ++HeatPumpNum) {
            FurnaceNum = HeatPumpNum;
            auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);
            // Setup Report variables for Unitary System that are not reported in the components themselves
            SetupOutputVariable(state,
                                "Unitary System Fan Part Load Ratio",
                                Constant::Units::None,
                                furnace.FanPartLoadRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                furnace.Name);
            SetupOutputVariable(state,
                                "Unitary System Compressor Part Load Ratio",
                                Constant::Units::None,
                                furnace.CompPartLoadRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                furnace.Name);
            SetupOutputVariable(state,
                                "Unitary System Requested Sensible Cooling Rate",
                                Constant::Units::W,
                                furnace.CoolingCoilSensDemand,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                furnace.Name);
            SetupOutputVariable(state,
                                "Unitary System Requested Latent Cooling Rate",
                                Constant::Units::W,
                                furnace.CoolingCoilLatentDemand,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                furnace.Name);
            SetupOutputVariable(state,
                                "Unitary System Requested Heating Rate",
                                Constant::Units::W,
                                furnace.HeatingCoilSensDemand,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                furnace.Name);
            SetupOutputVariable(state,
                                "Unitary System Dehumidification Induced Heating Demand Rate",
                                Constant::Units::W,
                                furnace.DehumidInducedHeatingDemandRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                furnace.Name);

            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSActuator(state,
                                 "AirLoopHVAC:UnitaryHeatPump:WaterToAir",
                                 furnace.Name,
                                 "Autosized Supply Air Flow Rate",
                                 "[m3/s]",
                                 furnace.DesignFanVolFlowRateEMSOverrideOn,
                                 furnace.DesignFanVolFlowRateEMSOverrideValue);
            }
        }

        if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
            for (FurnaceNum = 1; FurnaceNum <= state.dataFurnaces->NumFurnaces; ++FurnaceNum) {
                auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);
                SetupEMSInternalVariable(state, "Unitary HVAC Design Heating Capacity", furnace.Name, "[W]", furnace.DesignHeatingCapacity);
                SetupEMSInternalVariable(state, "Unitary HVAC Design Cooling Capacity", furnace.Name, "[W]", furnace.DesignCoolingCapacity);
                SetupEMSActuator(state,
                                 "Unitary HVAC",
                                 furnace.Name,
                                 "Sensible Load Request",
                                 "[W]",
                                 furnace.EMSOverrideSensZoneLoadRequest,
                                 furnace.EMSSensibleZoneLoadValue);
                SetupEMSActuator(state,
                                 "Unitary HVAC",
                                 furnace.Name,
                                 "Moisture Load Request",
                                 "[W]",
                                 furnace.EMSOverrideMoistZoneLoadRequest,
                                 furnace.EMSMoistureZoneLoadValue);
            }
        }
        bool anyRan;
        EMSManager::ManageEMS(state, EMSManager::EMSCallFrom::ComponentGetInput, anyRan, ObjexxFCL::Optional_int_const());
    }

    // End of Get Input subroutines for this Module
    //******************************************************************************

    // Beginning Initialization Section of the Module
    //******************************************************************************

    void InitFurnace(EnergyPlusData &state,
                     int const FurnaceNum,         // index to Furnace
                     int const AirLoopNum,         // index to air loop
                     Real64 &OnOffAirFlowRatio,    // ratio of on to off air mass flow rate
                     HVAC::FanOp &fanOp,           // fan operating mode
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

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the Furnace Components.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.
        // The HeatCool furnace/unitarysystem and air-to-air heat pump may have alternate air flow rates
        // in cooling, heating, and when no cooling or heating is needed. Set up the coil (comp) ON and OFF
        // air flow rates during InitFurnace. Use these flow rates during the Calc routines to set the
        // average mass flow rates based on PLR.

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr Small5WLoad(5.0);
        std::string_view constexpr RoutineName("InitFurnace");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool errFlag;          // error flag for mining functions
        Real64 QZnReq;         // furnace load based on control zone frac (W)
        Real64 PartLoadRatio;  // furnace part-load ratio
        Real64 SensibleOutput; // no load sensible output (coils off) (W)
        Real64 LatentOutput;   // no load latent output (coils off) (W)
        Real64 QToCoolSetPt;   // sensible load to cooling setpoint (W)
        Real64 QToHeatSetPt;   // sensible load to heating setpoint (W)
        // calculation (kg/kg)
        Real64 DeltaMassRate; // Difference of mass flow rate between
        // inlet node and system outlet node
        Real64 MassFlowRate; // mass flow rate to calculate loss

        Real64 SumOfMassFlowRateMax(0.0);                 // the sum of mass flow rates at inlet to zones in an airloop
        Real64 CntrlZoneTerminalUnitMassFlowRateMax(0.0); // Maximum mass flow rate through controlled zone terminal unit

        Real64 mdot(0.0);                        // local temporary for mass flow rate (kg/s)
        Real64 rho(0.0);                         // local for fluid density
        Real64 CoilMaxVolFlowRate(0.0);          // coil fluid maximum volume flow rate
        Real64 QActual(0.0);                     // coil actual capacity
        Real64 SUPHEATERLOAD(0.0);               // SUPPLEMENTAL HEATER LOAD
        Real64 RhoAir;                           // Air density at InNode
        Furnaces::ModeOfOperation OperatingMode; // track cooling, heating, and no cooling or heating modes
        Furnaces::ModeOfOperation OperatingModeMinusOne;
        Furnaces::ModeOfOperation OperatingModeMinusTwo;
        bool Oscillate; // detection of oscillating operating modes

        auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);
        int InNode = furnace.FurnaceInletNode;
        int OutNode = furnace.FurnaceOutletNode;

        if (state.dataFurnaces->InitFurnaceMyOneTimeFlag) {
            // initialize the environment and sizing flags
            state.dataFurnaces->MyEnvrnFlag.allocate(state.dataFurnaces->NumFurnaces);
            state.dataFurnaces->MySizeFlag.allocate(state.dataFurnaces->NumFurnaces);
            state.dataFurnaces->MySecondOneTimeFlag.allocate(state.dataFurnaces->NumFurnaces);
            state.dataFurnaces->MyFanFlag.allocate(state.dataFurnaces->NumFurnaces);
            state.dataFurnaces->MyCheckFlag.allocate(state.dataFurnaces->NumFurnaces);
            state.dataFurnaces->MyFlowFracFlag.allocate(state.dataFurnaces->NumFurnaces);
            state.dataFurnaces->MyPlantScanFlag.allocate(state.dataFurnaces->NumFurnaces);
            state.dataFurnaces->MySuppHeatCoilPlantScanFlag.allocate(state.dataFurnaces->NumFurnaces);
            state.dataFurnaces->MyEnvrnFlag = true;
            state.dataFurnaces->MySizeFlag = true;
            state.dataFurnaces->MySecondOneTimeFlag = true;
            state.dataFurnaces->MyFanFlag = true;
            state.dataFurnaces->MyCheckFlag = true;
            state.dataFurnaces->MyFlowFracFlag = true;
            state.dataFurnaces->InitFurnaceMyOneTimeFlag = false;
            state.dataFurnaces->MyPlantScanFlag = true;
            state.dataFurnaces->MySuppHeatCoilPlantScanFlag = true;
        }

        if (state.dataGlobal->BeginEnvrnFlag && state.dataFurnaces->MyAirLoopPass) {
            state.dataFurnaces->AirLoopPass = 0;
            state.dataFurnaces->MyAirLoopPass = false;
        }
        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataFurnaces->MyAirLoopPass = true;
        }

        ++state.dataFurnaces->AirLoopPass;
        if (state.dataFurnaces->AirLoopPass > 2) {
            state.dataFurnaces->AirLoopPass = 1;
        }

        if (!state.dataGlobal->SysSizingCalc && state.dataFurnaces->MySizeFlag(FurnaceNum)) {
            // for each furnace, do the sizing once.
            SizeFurnace(state, FurnaceNum, FirstHVACIteration);
            furnace.ControlZoneMassFlowFrac = 1.0;

            state.dataFurnaces->MySizeFlag(FurnaceNum) = false;
            // Pass the fan cycling schedule index up to the air loop. Set the air loop unitary system flag.
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).cycFanSched = furnace.fanOpModeSched;
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).UnitarySys = true;

            // RR this is wrong, Op mode needs to be updated each time atep
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).fanOp = furnace.fanOp;

            // Check that heat pump heating capacity is within 20% of cooling capacity
            if (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_AirToAir) {
                if (std::abs(furnace.DesignCoolingCapacity - furnace.DesignHeatingCapacity) / furnace.DesignCoolingCapacity > 0.2) {
                    ShowWarningError(state,
                                     format("{} \"{}\" heating capacity is disproportionate (> 20% different) to total cooling capacity",
                                            HVAC::unitarySysTypeNames[(int)furnace.type],
                                            furnace.Name));
                }
            }
        }

        if (!state.dataGlobal->DoingSizing && state.dataFurnaces->MySecondOneTimeFlag(FurnaceNum)) {
            // sizing all done.  check fan air flow rates
            furnace.ActualFanVolFlowRate = state.dataFans->fans(furnace.FanIndex)->maxAirFlowRate;
            if (furnace.ActualFanVolFlowRate != DataSizing::AutoSize) {
                if (furnace.DesignFanVolFlowRate > furnace.ActualFanVolFlowRate) {
                    ShowWarningError(state,
                                     format("{}={} has a Design Fan Volume Flow Rate > Max Fan Volume Flow Rate, should be <=",
                                            HVAC::unitarySysTypeNames[(int)furnace.type],
                                            furnace.Name));
                    ShowContinueError(state,
                                      format("... Entered value={:.2R}... Fan [{}] Max Value={:.2R}",
                                             furnace.DesignFanVolFlowRate,
                                             HVAC::fanTypeNames[(int)furnace.fanType],
                                             furnace.ActualFanVolFlowRate));
                }
                if (furnace.DesignFanVolFlowRate <= 0.0) {
                    ShowSevereError(state,
                                    format("{}={} has a Design Fan Volume Flow Rate <= 0.0, it must be >0.0",
                                           HVAC::unitarySysTypeNames[(int)furnace.type],
                                           furnace.Name));
                    ShowContinueError(state, format("... Entered value={:.2R}", furnace.DesignFanVolFlowRate));
                }

                state.dataFurnaces->MySecondOneTimeFlag(FurnaceNum) = false;
            }
        }

        // Scan hot water and steam heating coil plant components for one time initializations
        if (state.dataFurnaces->MyPlantScanFlag(FurnaceNum) && allocated(state.dataPlnt->PlantLoop)) {
            if (furnace.heatCoilType == HVAC::CoilType::HeatingWater ||
                furnace.heatCoilType == HVAC::CoilType::HeatingSteam) {

                if (furnace.heatCoilType == HVAC::CoilType::HeatingWater) {

                    errFlag = false;
                    PlantUtilities::ScanPlantLoopsForObject(state,
                                                            furnace.HeatCoilName,
                                                            DataPlant::PlantEquipmentType::CoilWaterSimpleHeating,
                                                            furnace.plantLoc,
                                                            errFlag,
                                                            _,
                                                            _,
                                                            _,
                                                            _,
                                                            _);
                    if (errFlag) {
                        ShowFatalError(state, "InitFurnace: Program terminated for previous conditions.");
                    }
                    furnace.MaxHeatCoilFluidFlow = WaterCoils::GetCoilMaxWaterFlowRate(state, furnace.HeatCoilNum);
                    if (furnace.MaxHeatCoilFluidFlow > 0.0) {
                        Real64 rho = state.dataPlnt->PlantLoop(furnace.plantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                        furnace.MaxHeatCoilFluidFlow *= rho;
                    }
                } else if (furnace.heatCoilType == HVAC::CoilType::HeatingSteam) {

                    errFlag = false;
                    PlantUtilities::ScanPlantLoopsForObject(state,
                                                            furnace.HeatCoilName,
                                                            DataPlant::PlantEquipmentType::CoilSteamAirHeating,
                                                            furnace.plantLoc,
                                                            errFlag,
                                                            _,
                                                            _,
                                                            _,
                                                            _,
                                                            _);
                    if (errFlag) {
                        ShowFatalError(state, "InitFurnace: Program terminated for previous conditions.");
                    }
                    furnace.MaxHeatCoilFluidFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, furnace.HeatCoilNum);
                    if (furnace.MaxHeatCoilFluidFlow > 0.0) {
                        furnace.MaxHeatCoilFluidFlow *= Fluid::GetSteam(state)->getSatDensity(state, 100.0, 1.0, RoutineName);
                    }
                }
                // fill outlet node for coil
                furnace.CoilOutletNode = DataPlant::CompData::getPlantComponent(state, furnace.plantLoc).NodeNumOut;
                state.dataFurnaces->MyPlantScanFlag(FurnaceNum) = false;
            } else { // pthp not connected to plant
                state.dataFurnaces->MyPlantScanFlag(FurnaceNum) = false;
            }
        } else if (state.dataFurnaces->MyPlantScanFlag(FurnaceNum) && !state.dataGlobal->AnyPlantInModel) {
            state.dataFurnaces->MyPlantScanFlag(FurnaceNum) = false;
        }

        // Scan Supplemental hot water and steam heating coil plant components for one time initializations
        if (state.dataFurnaces->MySuppHeatCoilPlantScanFlag(FurnaceNum) && allocated(state.dataPlnt->PlantLoop)) {
            if (furnace.suppHeatCoilType == HVAC::CoilType::HeatingWater ||
                furnace.suppHeatCoilType == HVAC::CoilType::HeatingSteam) {

                if (furnace.suppHeatCoilType == HVAC::CoilType::HeatingWater) {
                    errFlag = false;
                    PlantUtilities::ScanPlantLoopsForObject(state,
                                                            furnace.SuppHeatCoilName,
                                                            DataPlant::PlantEquipmentType::CoilWaterSimpleHeating,
                                                            furnace.SuppPlantLoc,
                                                            errFlag,
                                                            _,
                                                            _,
                                                            _,
                                                            _,
                                                            _);
                    if (errFlag) {
                        ShowFatalError(state, "InitFurnace: Program terminated for previous conditions.");
                    }
                    furnace.MaxSuppHeatCoilFluidFlow = WaterCoils::GetCoilMaxWaterFlowRate(state, furnace.SuppHeatCoilNum);
                    if (furnace.MaxSuppHeatCoilFluidFlow > 0.0) {
                        Real64 rho = state.dataPlnt->PlantLoop(furnace.SuppPlantLoc.loopNum)
                                  .glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                        furnace.MaxSuppHeatCoilFluidFlow *= rho;
                    }
                } else if (furnace.suppHeatCoilType == HVAC::CoilType::HeatingSteam) {
                    errFlag = false;
                    PlantUtilities::ScanPlantLoopsForObject(state,
                                                            furnace.SuppHeatCoilName,
                                                            DataPlant::PlantEquipmentType::CoilSteamAirHeating,
                                                            furnace.SuppPlantLoc,
                                                            errFlag,
                                                            _,
                                                            _,
                                                            _,
                                                            _,
                                                            _);
                    if (errFlag) {
                        ShowFatalError(state, "InitFurnace: Program terminated for previous conditions.");
                    }
                    furnace.MaxSuppHeatCoilFluidFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, furnace.SuppHeatCoilNum);
                    if (furnace.MaxSuppHeatCoilFluidFlow > 0.0) {
                        furnace.MaxSuppHeatCoilFluidFlow *= Fluid::GetSteam(state)->getSatDensity(state, 100.0, 1.0, RoutineName);
                    }
                }
                // fill outlet node for coil
                furnace.SuppHeatCoilAirOutletNode = DataPlant::CompData::getPlantComponent(state, furnace.SuppPlantLoc).NodeNumOut;
                state.dataFurnaces->MySuppHeatCoilPlantScanFlag(FurnaceNum) = false;
            } else { // pthp not connected to plant
                state.dataFurnaces->MySuppHeatCoilPlantScanFlag(FurnaceNum) = false;
            }

        } else if (state.dataFurnaces->MySuppHeatCoilPlantScanFlag(FurnaceNum) && !state.dataGlobal->AnyPlantInModel) {
            state.dataFurnaces->MySuppHeatCoilPlantScanFlag(FurnaceNum) = false;
        }

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && state.dataFurnaces->MyEnvrnFlag(FurnaceNum)) {
            // Change the Volume Flow Rates to Mass Flow Rates
            furnace.DesignMassFlowRate = furnace.DesignFanVolFlowRate * state.dataEnvrn->StdRhoAir;
            furnace.MaxCoolAirMassFlow = furnace.MaxCoolAirVolFlow * state.dataEnvrn->StdRhoAir;
            furnace.MaxHeatAirMassFlow = furnace.MaxHeatAirVolFlow * state.dataEnvrn->StdRhoAir;
            furnace.MaxNoCoolHeatAirMassFlow = furnace.MaxNoCoolHeatAirVolFlow * state.dataEnvrn->StdRhoAir;
            furnace.CompPartLoadRatio = 0.0;
            furnace.CoolingCoilSensDemand = 0.0;
            furnace.CoolingCoilLatentDemand = 0.0;
            furnace.HeatingCoilSensDemand = 0.0;

            furnace.SenLoadLoss = 0.0;
            if (furnace.Humidistat) {
                furnace.LatLoadLoss = 0.0;
            }

            //   set fluid-side hardware limits
            if (furnace.HeatCoilControlNode > 0) {

                if (furnace.MaxHeatCoilFluidFlow == DataSizing::AutoSize) {
                    // If water coil max water flow rate is autosized, simulate once in order to mine max flow rate
                    if (furnace.heatCoilType == HVAC::CoilType::HeatingWater) {
                        WaterCoils::SimulateWaterCoilComponents(state, furnace.HeatCoilName, FirstHVACIteration, furnace.HeatCoilNum);
                        CoilMaxVolFlowRate = WaterCoils::GetCoilMaxWaterFlowRate(state, furnace.HeatCoilNum);
                        if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                            furnace.MaxHeatCoilFluidFlow = CoilMaxVolFlowRate *
                              state.dataPlnt->PlantLoop(furnace.plantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                        }
                    }
                    // If steam coil max steam flow rate is autosized, simulate once in order to mine max flow rate
                    if (furnace.heatCoilType == HVAC::CoilType::HeatingSteam) {
                        SteamCoils::SimulateSteamCoilComponents(state,
                                                                furnace.HeatCoilName,
                                                                FirstHVACIteration,
                                                                furnace.HeatCoilNum,
                                                                1.0,
                                                                QActual); // QCoilReq, simulate any load > 0 to get max capacity
                        CoilMaxVolFlowRate = SteamCoils::GetCoilMaxSteamFlowRate(state, furnace.HeatCoilNum);
                        if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                            furnace.MaxHeatCoilFluidFlow = CoilMaxVolFlowRate * Fluid::GetSteam(state)->getSatDensity(state, 100.0, 1.0, RoutineName);
                        }
                    }
                }

                PlantUtilities::InitComponentNodes(
                    state, 0.0, furnace.MaxHeatCoilFluidFlow, furnace.HeatCoilControlNode, furnace.CoilOutletNode);
            }
            if (furnace.SuppHeatCoilControlNode > 0) {
                if (furnace.MaxSuppHeatCoilFluidFlow == DataSizing::AutoSize) {
                    if (furnace.suppHeatCoilType == HVAC::CoilType::HeatingWater) {
                        // If water coil max water flow rate is autosized, simulate once in order to mine max flow rate
                        WaterCoils::SimulateWaterCoilComponents(
                            state, furnace.SuppHeatCoilName, FirstHVACIteration, furnace.SuppHeatCoilNum);
                        CoilMaxVolFlowRate = WaterCoils::GetCoilMaxWaterFlowRate(state, furnace.SuppHeatCoilNum);
                        if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                            rho = state.dataPlnt->PlantLoop(furnace.SuppPlantLoc.loopNum)
                                      .glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                            furnace.MaxSuppHeatCoilFluidFlow = CoilMaxVolFlowRate * rho;
                        }
                    }
                    if (furnace.suppHeatCoilType == HVAC::CoilType::HeatingSteam) {
                        SteamCoils::SimulateSteamCoilComponents(state,
                                                                furnace.SuppHeatCoilName,
                                                                FirstHVACIteration,
                                                                furnace.SuppHeatCoilNum,
                                                                1.0,
                                                                QActual); // QCoilReq, simulate any load > 0 to get max capacity
                        CoilMaxVolFlowRate = SteamCoils::GetCoilMaxSteamFlowRate(state, furnace.SuppHeatCoilNum);
                        if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                            furnace.MaxSuppHeatCoilFluidFlow = CoilMaxVolFlowRate * Fluid::GetSteam(state)->getSatDensity(state, 100.0, 1.0, RoutineName);
                        }
                    }
                    PlantUtilities::InitComponentNodes(
                        state, 0.0, furnace.MaxSuppHeatCoilFluidFlow, furnace.SuppHeatCoilControlNode, furnace.SuppHeatCoilAirOutletNode);
                }
            }
            state.dataFurnaces->MyEnvrnFlag(FurnaceNum) = false;
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataFurnaces->MyEnvrnFlag(FurnaceNum) = true;
        }

        if (state.dataFurnaces->MyFanFlag(FurnaceNum)) {
            if (furnace.ActualFanVolFlowRate != DataSizing::AutoSize) {
                if (furnace.ActualFanVolFlowRate > 0.0) {
                    furnace.HeatingSpeedRatio = furnace.MaxHeatAirVolFlow / furnace.ActualFanVolFlowRate;
                    furnace.CoolingSpeedRatio = furnace.MaxCoolAirVolFlow / furnace.ActualFanVolFlowRate;
                    furnace.NoHeatCoolSpeedRatio = furnace.MaxNoCoolHeatAirVolFlow / furnace.ActualFanVolFlowRate;
                }
                if (dynamic_cast<Fans::FanComponent *>(state.dataFans->fans(furnace.FanIndex))->powerRatioAtSpeedRatioCurveNum > 0) {
                    if (furnace.ActualFanVolFlowRate == furnace.MaxHeatAirVolFlow &&
                        furnace.ActualFanVolFlowRate == furnace.MaxCoolAirVolFlow &&
                        furnace.ActualFanVolFlowRate == furnace.MaxNoCoolHeatAirVolFlow) {
                        std::string FanName = state.dataFans->fans(furnace.FanIndex)->Name;
                        ShowWarningError(state, format("{} \"{}\"", HVAC::unitarySysTypeNames[(int)furnace.type], furnace.Name));
                        ShowContinueError(state,
                                          format("...For fan type and name = {} \"{}\"", HVAC::fanTypeNames[(int)furnace.fanType], FanName));
                        ShowContinueError(state,
                                          "...Fan power ratio function of speed ratio curve has no impact if fan volumetric flow rate is the same as "
                                          "the unitary system volumetric flow rate.");
                        ShowContinueError(state, format("...Fan volumetric flow rate            = {:.5R} m3/s.", furnace.ActualFanVolFlowRate));
                        ShowContinueError(state, format("...Unitary system volumetric flow rate = {:.5R} m3/s.", furnace.MaxHeatAirVolFlow));
                    }
                }
                state.dataFurnaces->MyFanFlag(FurnaceNum) = false;
            } else {
                furnace.ActualFanVolFlowRate = state.dataFans->fans(furnace.FanIndex)->maxAirFlowRate;
            }
        }

        if (allocated(state.dataZoneEquip->ZoneEquipConfig) && state.dataFurnaces->MyCheckFlag(FurnaceNum)) {
            int zoneNum = furnace.ControlZoneNum;
            int zoneInlet = furnace.ZoneInletNode;
            // setup furnace zone equipment sequence information based on finding matching air terminal
            if (state.dataZoneEquip->ZoneEquipConfig(zoneNum).EquipListIndex > 0) {
                int coolingPriority = 0;
                int heatingPriority = 0;
                state.dataZoneEquip->ZoneEquipList(state.dataZoneEquip->ZoneEquipConfig(zoneNum).EquipListIndex)
                    .getPrioritiesForInletNode(state, zoneInlet, coolingPriority, heatingPriority);
                furnace.ZoneSequenceCoolingNum = coolingPriority;
                furnace.ZoneSequenceHeatingNum = heatingPriority;
            }
            state.dataFurnaces->MyCheckFlag(FurnaceNum) = false;
            if (furnace.ZoneSequenceCoolingNum == 0 || furnace.ZoneSequenceHeatingNum == 0) {
                ShowSevereError(state,
                                format("{} \"{}\": Airloop air terminal in the zone equipment list for zone = {} not found or is not allowed Zone "
                                       "Equipment Cooling or Heating Sequence = 0.",
                                       HVAC::unitarySysTypeNames[(int)furnace.type],
                                       furnace.Name,
                                       state.dataHeatBal->Zone(furnace.ControlZoneNum).Name));
                ShowFatalError(state,
                               format("Subroutine InitFurnace: Errors found in getting {} input.  Preceding condition(s) causes termination.",
                                      HVAC::unitarySysTypeNames[(int)furnace.type]));
            }
        }

        // Find the number of zones (zone Inlet Nodes) attached to an air loop from the air loop number
        int NumAirLoopZones =
            state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesCooled + state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesHeated;
        if (allocated(state.dataAirLoop->AirToZoneNodeInfo) && state.dataFurnaces->MyFlowFracFlag(FurnaceNum)) {
            state.dataFurnaces->FlowFracFlagReady = true;
            for (int ZoneInSysIndex = 1; ZoneInSysIndex <= NumAirLoopZones; ++ZoneInSysIndex) {
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
                for (int ZoneInSysIndex = 1; ZoneInSysIndex <= NumAirLoopZones; ++ZoneInSysIndex) {
                    int ZoneInletNodeNum = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolInletNodes(ZoneInSysIndex);
                    SumOfMassFlowRateMax += state.dataLoopNodes->Node(ZoneInletNodeNum).MassFlowRateMax;
                    if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).CoolCtrlZoneNums(ZoneInSysIndex) == furnace.ControlZoneNum) {
                        CntrlZoneTerminalUnitMassFlowRateMax = state.dataLoopNodes->Node(ZoneInletNodeNum).MassFlowRateMax;
                    }
                }
                if (SumOfMassFlowRateMax != 0.0) {
                    if (CntrlZoneTerminalUnitMassFlowRateMax >= HVAC::SmallAirVolFlow) {
                        furnace.ControlZoneMassFlowFrac = CntrlZoneTerminalUnitMassFlowRateMax / SumOfMassFlowRateMax;
                    } else {
                        ShowSevereError(state, format("{} = {}", HVAC::unitarySysTypeNames[(int)furnace.type], furnace.Name));
                        ShowContinueError(state, " The Fraction of Supply Air Flow That Goes Through the Controlling Zone is set to 1.");
                    }
                    BaseSizer::reportSizerOutput(state,
                                                 HVAC::unitarySysTypeNames[(int)furnace.type],
                                                 furnace.Name,
                                                 "Fraction of Supply Air Flow That Goes Through the Controlling Zone",
                                                 furnace.ControlZoneMassFlowFrac);
                    state.dataFurnaces->MyFlowFracFlag(FurnaceNum) = false;
                }
            }
        }

        // Calculate air distribution losses
        if (!FirstHVACIteration && state.dataFurnaces->AirLoopPass == 1) {
            int ZoneInNode = furnace.ZoneInletNode;
            MassFlowRate = state.dataLoopNodes->Node(ZoneInNode).MassFlowRate / furnace.ControlZoneMassFlowFrac;
            if (state.afn->distribution_simulated) {
                DeltaMassRate = state.dataLoopNodes->Node(furnace.FurnaceOutletNode).MassFlowRate -
                                state.dataLoopNodes->Node(ZoneInNode).MassFlowRate / furnace.ControlZoneMassFlowFrac;
                if (DeltaMassRate < 0.0) DeltaMassRate = 0.0;
            } else {
                MassFlowRate = state.dataLoopNodes->Node(furnace.FurnaceOutletNode).MassFlowRate;
                DeltaMassRate = 0.0;
            }
            Real64 TotalOutput(0.0);         // total output rate, {W}
            Real64 SensibleOutputDelta(0.0); // delta sensible output rate, {W}
            Real64 LatentOutputDelta(0.0);   // delta latent output rate, {W}
            Real64 TotalOutputDelta(0.0);    // delta total output rate, {W}
            CalcZoneSensibleLatentOutput(MassFlowRate,
                                         state.dataLoopNodes->Node(furnace.FurnaceOutletNode).Temp,
                                         state.dataLoopNodes->Node(furnace.FurnaceOutletNode).HumRat,
                                         state.dataLoopNodes->Node(ZoneInNode).Temp,
                                         state.dataLoopNodes->Node(ZoneInNode).HumRat,
                                         furnace.SenLoadLoss,
                                         furnace.LatLoadLoss,
                                         TotalOutput);
            CalcZoneSensibleLatentOutput(DeltaMassRate,
                                         state.dataLoopNodes->Node(furnace.FurnaceOutletNode).Temp,
                                         state.dataLoopNodes->Node(furnace.FurnaceOutletNode).HumRat,
                                         state.dataLoopNodes->Node(furnace.NodeNumOfControlledZone).Temp,
                                         state.dataLoopNodes->Node(furnace.NodeNumOfControlledZone).HumRat,
                                         SensibleOutputDelta,
                                         LatentOutputDelta,
                                         TotalOutputDelta);
            furnace.SenLoadLoss = furnace.SenLoadLoss + SensibleOutputDelta;
            if (std::abs(furnace.SensibleLoadMet) > 0.0) {
                if (std::abs(furnace.SenLoadLoss / furnace.SensibleLoadMet) < 0.001) furnace.SenLoadLoss = 0.0;
            }
            if (furnace.Humidistat) {
                furnace.LatLoadLoss = furnace.LatLoadLoss + LatentOutputDelta;
                if (std::abs(furnace.LatentLoadMet) > 0.0) {
                    if (std::abs(furnace.LatLoadLoss / furnace.LatentLoadMet) < 0.001) furnace.LatLoadLoss = 0.0;
                }
            }
        }

        if (furnace.fanOpModeSched != nullptr) {
            furnace.fanOp = (furnace.fanOpModeSched->getCurrentVal() == 0.0) ? HVAC::FanOp::Cycling : HVAC::FanOp::Continuous;
            if (AirLoopNum > 0) {
                state.dataAirLoop->AirLoopControlInfo(AirLoopNum).fanOp = furnace.fanOp;
            }
        }

        fanOp = furnace.fanOp;
        state.dataFurnaces->EconomizerFlag = state.dataAirLoop->AirLoopControlInfo(AirLoopNum).EconoActive;

        if (furnace.ControlZoneMassFlowFrac > 0.0) {
            QZnReq = ZoneLoad / furnace.ControlZoneMassFlowFrac;
            MoistureLoad /= furnace.ControlZoneMassFlowFrac;
            ZoneLoad = QZnReq;
        } else {
            QZnReq = ZoneLoad;
        }

        // Original thermostat control logic (works only for cycling fan systems)
        if (QZnReq > HVAC::SmallLoad && QZnReq > (Small5WLoad / furnace.ControlZoneMassFlowFrac) &&
            !state.dataZoneEnergyDemand->CurDeadBandOrSetback(furnace.ControlZoneNum)) {
            state.dataFurnaces->HeatingLoad = true;
            state.dataFurnaces->CoolingLoad = false;
        } else if (QZnReq < -HVAC::SmallLoad && std::abs(QZnReq) > (Small5WLoad / furnace.ControlZoneMassFlowFrac) &&
                   !state.dataZoneEnergyDemand->CurDeadBandOrSetback(furnace.ControlZoneNum)) {
            state.dataFurnaces->HeatingLoad = false;
            state.dataFurnaces->CoolingLoad = true;
        } else {
            state.dataFurnaces->HeatingLoad = false;
            state.dataFurnaces->CoolingLoad = false;
        }

        if (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_AirToAir ||
            (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_WaterToAir &&
             (furnace.WatertoAirHPType == WAHPCoilType::Simple || furnace.WatertoAirHPType == WAHPCoilType::VarSpeedEquationFit))) {
            if (MoistureLoad < 0.0 && furnace.DehumidControlType_Num == DehumidificationControlMode::CoolReheat) {
                state.dataFurnaces->HPDehumidificationLoadFlag = true;
                state.dataFurnaces->HeatingLoad = false;
                state.dataFurnaces->CoolingLoad = true;
            } else {
                state.dataFurnaces->HPDehumidificationLoadFlag = false;
            }
        }

        // Check for heat only furnace
        if (furnace.type != HVAC::UnitarySysType::Furnace_HeatOnly && furnace.type != HVAC::UnitarySysType::Unitary_HeatOnly) {

            if (furnace.availSched->getCurrentVal() > 0.0) {
                if ((state.dataFurnaces->HeatingLoad || state.dataFurnaces->CoolingLoad) || (furnace.Humidistat && MoistureLoad < 0.0)) {
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
            if (furnace.heatCoilType == HVAC::CoilType::HeatingWater) {
                // set water-side mass flow rates
                state.dataLoopNodes->Node(furnace.HeatCoilAirInletNode).MassFlowRate = state.dataFurnaces->CompOnMassFlow;
                mdot = furnace.MaxHeatCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(state, mdot, furnace.HeatCoilControlNode, furnace.CoilOutletNode, furnace.plantLoc);
                //     simulate water coil to find operating capacity
                WaterCoils::SimulateWaterCoilComponents(
                    state, furnace.HeatCoilName, FirstHVACIteration, furnace.HeatCoilNum, QActual);
                furnace.DesignHeatingCapacity = QActual;

            } // from IF(furnace%heatCoilType == Coil_HeatingWater) THEN

            if (furnace.heatCoilType == HVAC::CoilType::HeatingSteam) {
                // set air-side and steam-side mass flow rates
                state.dataLoopNodes->Node(furnace.HeatCoilAirInletNode).MassFlowRate = state.dataFurnaces->CompOnMassFlow;
                mdot = furnace.MaxHeatCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(state, mdot, furnace.HeatCoilControlNode, furnace.CoilOutletNode, furnace.plantLoc);

                //     simulate steam coil to find operating capacity
                SteamCoils::SimulateSteamCoilComponents(state,
                                                        furnace.HeatCoilName,
                                                        FirstHVACIteration,
                                                        furnace.HeatCoilNum,
                                                        1.0,
                                                        QActual); // QCoilReq, simulate any load > 0 to get max capacity of steam coil
                furnace.DesignHeatingCapacity = SteamCoils::GetCoilCapacity(state, furnace.HeatCoilNum);

            } // from IF(Furnace(FurnaceNum)%heatCoilType == Coil_HeatingSteam) THEN

            if (furnace.suppHeatCoilType == HVAC::CoilType::HeatingWater) {

                //     set air-side and steam-side mass flow rates
                state.dataLoopNodes->Node(furnace.SuppHeatCoilAirInletNode).MassFlowRate = state.dataFurnaces->CompOnMassFlow;
                mdot = furnace.MaxSuppHeatCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(
                    state, mdot, furnace.SuppHeatCoilControlNode, furnace.SuppHeatCoilAirOutletNode, furnace.SuppPlantLoc);

                //     simulate water coil to find operating capacity
                WaterCoils::SimulateWaterCoilComponents(
                    state, furnace.SuppHeatCoilName, FirstHVACIteration, furnace.SuppHeatCoilNum, QActual);
                furnace.DesignSuppHeatingCapacity = QActual;

            } // from IF(Furnace(FurnaceNum)%suppHeatCoilType == Coil_HeatingWater) THEN
            if (furnace.suppHeatCoilType == HVAC::CoilType::HeatingSteam) {
                //     set air-side and steam-side mass flow rates
                state.dataLoopNodes->Node(furnace.SuppHeatCoilAirInletNode).MassFlowRate = state.dataFurnaces->CompOnMassFlow;
                mdot = furnace.MaxSuppHeatCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(
                    state, mdot, furnace.SuppHeatCoilControlNode, furnace.SuppHeatCoilAirOutletNode, furnace.SuppPlantLoc);

                //     simulate steam coil to find operating capacity
                SteamCoils::SimulateSteamCoilComponents(state,
                                                        furnace.SuppHeatCoilName,
                                                        FirstHVACIteration,
                                                        furnace.SuppHeatCoilNum,
                                                        1.0,
                                                        QActual); // QCoilReq, simulate any load > 0 to get max capacity of steam coil
                furnace.DesignSuppHeatingCapacity = SteamCoils::GetCoilCapacity(state, furnace.SuppHeatCoilNum);

            } // from IF(Furnace(FurnaceNum)%suppHeatCoilType == Coil_HeatingSteam) THEN
        }     // from IF( FirstHVACIteration ) THEN

        if (furnace.NumOfSpeedCooling > 0) { // BoS, variable-speed water source hp
            // Furnace(FurnaceNum)%IdleMassFlowRate = RhoAir*Furnace(FurnaceNum)%IdleVolumeAirRate
            int NumOfSpeedCooling = furnace.NumOfSpeedCooling;
            int NumOfSpeedHeating = furnace.NumOfSpeedHeating;
            // IF MSHP system was not autosized and the fan is autosized, check that fan volumetric flow rate is greater than MSHP flow rates
            if (furnace.CheckFanFlow) {
                state.dataFurnaces->CurrentModuleObject = "AirLoopHVAC:UnitaryHeatPump:VariableSpeed";
                furnace.FanVolFlow = state.dataFans->fans(furnace.FanIndex)->maxAirFlowRate;

                if (furnace.FanVolFlow != DataSizing::AutoSize) {
                    //     Check fan versus system supply air flow rates
                    if (furnace.FanVolFlow + 1e-10 < furnace.CoolVolumeFlowRate(NumOfSpeedCooling)) {
                        ShowWarningError(state,
                                         format("{} - air flow rate = {:.7T} in fan object is less than the MSHP system air flow rate when cooling "
                                                "is required ({:.7T}).",
                                                state.dataFurnaces->CurrentModuleObject,
                                                furnace.FanVolFlow,
                                                furnace.CoolVolumeFlowRate(NumOfSpeedCooling)));
                        ShowContinueError(
                            state, " The MSHP system flow rate when cooling is required is reset to the fan flow rate and the simulation continues.");
                        ShowContinueError(state, format(" Occurs in {} = {}", state.dataFurnaces->CurrentModuleObject, furnace.Name));
                        furnace.CoolVolumeFlowRate(NumOfSpeedCooling) = furnace.FanVolFlow;

                        if (furnace.isIHP) // set max fan flow rate to the IHP collection
                        {
                            state.dataIntegratedHP->IntegratedHeatPumps(furnace.ihpNum).MaxCoolAirVolFlow = furnace.FanVolFlow;
                            state.dataIntegratedHP->IntegratedHeatPumps(furnace.ihpNum).MaxCoolAirMassFlow =
                                furnace.FanVolFlow * state.dataEnvrn->StdRhoAir;
                        }

                        // Check flow rates in other speeds and ensure flow rates are not above the max flow rate
                        for (int i = NumOfSpeedCooling - 1; i >= 1; --i) {
                            if (furnace.CoolVolumeFlowRate(i) > furnace.CoolVolumeFlowRate(i + 1)) {
                                ShowContinueError(state,
                                                  format(" The MSHP system flow rate when cooling is required is reset to the flow rate at higher "
                                                         "speed and the simulation continues at Speed{}.",
                                                         i));
                                ShowContinueError(state, format(" Occurs in {} = {}", state.dataFurnaces->CurrentModuleObject, furnace.Name));
                                furnace.CoolVolumeFlowRate(i) = furnace.CoolVolumeFlowRate(i + 1);
                            }
                        }
                    }
                    if (NumOfSpeedHeating > 0) {
                        if (furnace.FanVolFlow + 1e-10 < furnace.HeatVolumeFlowRate(NumOfSpeedHeating)) {
                            ShowWarningError(state,
                                             format("{} - air flow rate = {:.7T} in fan object is less than the MSHP system air flow rate when "
                                                    "heating is required ({:.7T}).",
                                                    state.dataFurnaces->CurrentModuleObject,
                                                    furnace.FanVolFlow,
                                                    furnace.HeatVolumeFlowRate(NumOfSpeedHeating)));
                            ShowContinueError(
                                state,
                                " The MSHP system flow rate when heating is required is reset to the fan flow rate and the simulation continues.");
                            ShowContinueError(state, format(" Occurs in {} = {}", state.dataFurnaces->CurrentModuleObject, furnace.Name));
                            furnace.HeatVolumeFlowRate(NumOfSpeedHeating) = furnace.FanVolFlow;

                            if (furnace.isIHP) // set max fan flow rate to the IHP collection
                            {
                                // ihpIndex is the index of the IHP, CoolCoilNum is the index of the coil in the IHP
                                state.dataIntegratedHP->IntegratedHeatPumps(furnace.ihpNum).MaxHeatAirVolFlow = furnace.FanVolFlow;
                                state.dataIntegratedHP->IntegratedHeatPumps(furnace.ihpNum).MaxHeatAirMassFlow =
                                    furnace.FanVolFlow * state.dataEnvrn->StdRhoAir;
                            }

                            for (int i = NumOfSpeedHeating - 1; i >= 1; --i) {
                                if (furnace.HeatVolumeFlowRate(i) > furnace.HeatVolumeFlowRate(i + 1)) {
                                    ShowContinueError(state,
                                                      format(" The MSHP system flow rate when heating is required is reset to the flow rate at "
                                                             "higher speed and the simulation continues at Speed{}.",
                                                             i));
                                    ShowContinueError(state,
                                                      format(" Occurs in {} system = {}", state.dataFurnaces->CurrentModuleObject, furnace.Name));
                                    furnace.HeatVolumeFlowRate(i) = furnace.HeatVolumeFlowRate(i + 1);
                                }
                            }
                        }
                    }
                    if (furnace.FanVolFlow < furnace.IdleVolumeAirRate && furnace.IdleVolumeAirRate != 0.0) {
                        ShowWarningError(state,
                                         format("{} - air flow rate = {:.7T} in fan object is less than the MSHP system air flow rate when no "
                                                "heating or cooling is needed ({:.7T}).",
                                                state.dataFurnaces->CurrentModuleObject,
                                                furnace.FanVolFlow,
                                                furnace.IdleVolumeAirRate));
                        ShowContinueError(state,
                                          " The MSHP system flow rate when no heating or cooling is needed is reset to the fan flow rate and the "
                                          "simulation continues.");
                        ShowContinueError(state, format(" Occurs in {} = {}", state.dataFurnaces->CurrentModuleObject, furnace.Name));
                        furnace.IdleVolumeAirRate = furnace.FanVolFlow;
                    }
                    RhoAir = state.dataEnvrn->StdRhoAir;
                    // set the mass flow rates from the reset volume flow rates
                    for (int i = 1; i <= NumOfSpeedCooling; ++i) {
                        furnace.CoolMassFlowRate(i) = RhoAir * furnace.CoolVolumeFlowRate(i);
                        if (furnace.FanVolFlow > 0.0) {
                            furnace.MSCoolingSpeedRatio(i) = furnace.CoolVolumeFlowRate(i) / furnace.FanVolFlow;
                        }
                    }
                    for (int i = 1; i <= NumOfSpeedHeating; ++i) {
                        furnace.HeatMassFlowRate(i) = RhoAir * furnace.HeatVolumeFlowRate(i);
                        if (furnace.FanVolFlow > 0.0) {
                            furnace.MSHeatingSpeedRatio(i) = furnace.HeatVolumeFlowRate(i) / furnace.FanVolFlow;
                        }
                    }
                    furnace.IdleMassFlowRate = RhoAir * furnace.IdleVolumeAirRate;
                    if (furnace.FanVolFlow > 0.0) {
                        furnace.IdleSpeedRatio = furnace.IdleVolumeAirRate / furnace.FanVolFlow;
                    }
                    // set the node max and min mass flow rates based on reset volume flow rates
                    if (NumOfSpeedCooling > 0 && NumOfSpeedHeating == 0) {
                        state.dataLoopNodes->Node(InNode).MassFlowRateMax =
                            max(furnace.CoolMassFlowRate(NumOfSpeedCooling), furnace.MaxHeatAirMassFlow);
                        state.dataLoopNodes->Node(InNode).MassFlowRateMaxAvail =
                            max(furnace.CoolMassFlowRate(NumOfSpeedCooling), furnace.MaxHeatAirMassFlow);
                    } else if (NumOfSpeedCooling == 0 && NumOfSpeedHeating > 0) {
                        state.dataLoopNodes->Node(InNode).MassFlowRateMax =
                            max(furnace.MaxCoolAirMassFlow, furnace.HeatMassFlowRate(NumOfSpeedHeating));
                        state.dataLoopNodes->Node(InNode).MassFlowRateMaxAvail =
                            max(furnace.MaxCoolAirMassFlow, furnace.HeatMassFlowRate(NumOfSpeedHeating));
                    } else {
                        state.dataLoopNodes->Node(InNode).MassFlowRateMax =
                            max(furnace.CoolMassFlowRate(NumOfSpeedCooling), furnace.HeatMassFlowRate(NumOfSpeedHeating));
                        state.dataLoopNodes->Node(InNode).MassFlowRateMaxAvail =
                            max(furnace.CoolMassFlowRate(NumOfSpeedCooling), furnace.HeatMassFlowRate(NumOfSpeedHeating));
                    }
                    state.dataLoopNodes->Node(InNode).MassFlowRateMin = 0.0;
                    state.dataLoopNodes->Node(InNode).MassFlowRateMinAvail = 0.0;
                    state.dataLoopNodes->Node(OutNode) = state.dataLoopNodes->Node(InNode);
                }
            }

            furnace.CheckFanFlow = false;
        }
        SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, fanOp, QZnReq, MoistureLoad, PartLoadRatio);

        // Check ventilation/fan load for constant fan systems to see if load to be met changes
        // Same IF logic used in Subroutine SetAverageAirFlow to determine if unit is ON or OFF

        QToCoolSetPt = 0.0;
        QToHeatSetPt = 0.0;
        if (fanOp == HVAC::FanOp::Continuous && furnace.availSched->getCurrentVal() > 0.0 &&
            ((furnace.fanAvailSched->getCurrentVal() > 0.0 || state.dataHVACGlobal->TurnFansOn) && !state.dataHVACGlobal->TurnFansOff)) {

            if (furnace.NumOfSpeedCooling > 0) {
                CalcVarSpeedHeatPump(state,
                                     FurnaceNum,
                                     false,
                                     HVAC::CompressorOp::Off,
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
                                  HVAC::FanOp::Invalid, // Looks like Invalid is used to mean that the fan is off here?
                                  HVAC::CompressorOp::Off,
                                  0.0,
                                  0.0,
                                  0.0,
                                  0.0,
                                  SensibleOutput,
                                  LatentOutput,
                                  OnOffAirFlowRatio,
                                  false);
            }

            if (furnace.ControlZoneMassFlowFrac > 0.0) {
                if (furnace.ZoneSequenceCoolingNum > 0 && furnace.ZoneSequenceHeatingNum > 0) {
                    QToCoolSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(furnace.ControlZoneNum)
                                       .SequencedOutputRequiredToCoolingSP(furnace.ZoneSequenceCoolingNum) /
                                   furnace.ControlZoneMassFlowFrac;
                    QToHeatSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(furnace.ControlZoneNum)
                                       .SequencedOutputRequiredToHeatingSP(furnace.ZoneSequenceHeatingNum) /
                                   furnace.ControlZoneMassFlowFrac;
                } else {
                    QToCoolSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(furnace.ControlZoneNum).OutputRequiredToCoolingSP /
                                   furnace.ControlZoneMassFlowFrac;
                    QToHeatSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(furnace.ControlZoneNum).OutputRequiredToHeatingSP /
                                   furnace.ControlZoneMassFlowFrac;
                }
                //     If the furnace has a net cooling capacity (SensibleOutput < 0) and
                //     the zone temp is above the Tstat heating setpoint (QToHeatSetPt < 0) and
                //     the net cooling capacity does not just offset the cooling load
                if (SensibleOutput < 0.0 && QToHeatSetPt < 0.0 &&
                    std::abs(QToCoolSetPt - SensibleOutput) > (Small5WLoad / furnace.ControlZoneMassFlowFrac)) {
                    //       Only switch modes when humidistat is not used or no moisture load exists, otherwise let
                    //       reheat coil pick up load
                    //        IF((SensibleOutput .LT. QToHeatSetPt .AND. .NOT. Furnace(FurnaceNum)%Humidistat) .OR. &
                    //           (SensibleOutput .LT. QToHeatSetPt .AND. Furnace(FurnaceNum)%Humidistat .AND. MoistureLoad .GE. 0.0))THEN
                    if ((SensibleOutput < QToHeatSetPt && !furnace.Humidistat) ||
                        (SensibleOutput < QToHeatSetPt && furnace.Humidistat && MoistureLoad >= 0.0)) {
                        QZnReq = QToHeatSetPt;
                        state.dataFurnaces->CoolingLoad = false;
                        //         Don't set mode TRUE unless mode is allowed. Also check for floating zone.
                        if (state.dataHeatBalFanSys->TempControlType(furnace.ControlZoneNum) == HVAC::SetptType::SingleCool ||
                            state.dataHeatBalFanSys->TempControlType(furnace.ControlZoneNum) == HVAC::SetptType::Uncontrolled) {
                            state.dataFurnaces->HeatingLoad = false;
                        } else {
                            state.dataFurnaces->HeatingLoad = true;
                        }

                        SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, fanOp, QZnReq, MoistureLoad, PartLoadRatio);
                        if (furnace.NumOfSpeedCooling > 0) {
                            CalcVarSpeedHeatPump(state,
                                                 FurnaceNum,
                                                 false,
                                                 HVAC::CompressorOp::Off,
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
                                              HVAC::FanOp::Invalid,
                                              HVAC::CompressorOp::Off,
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
                            // If changing operating mode (flow rates) does not overshoot heating setpoint, turn off heating
                            QZnReq = 0.0;
                            state.dataFurnaces->HeatingLoad = false;
                            SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, fanOp, QZnReq, MoistureLoad, PartLoadRatio);
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
                        SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, fanOp, QZnReq, MoistureLoad, PartLoadRatio);
                    }
                    // the net cooling capacity just offsets the cooling load, turn off cooling
                } else if (SensibleOutput < 0.0 && QToCoolSetPt < 0.0 &&
                           std::abs(QToCoolSetPt - SensibleOutput) < (Small5WLoad / furnace.ControlZoneMassFlowFrac)) {
                    state.dataFurnaces->CoolingLoad = false;
                    if (state.dataFurnaces->HPDehumidificationLoadFlag) {
                        state.dataFurnaces->CoolingLoad = true;
                        state.dataFurnaces->HeatingLoad = false;
                    }
                } // SensibleOutput .LT. 0.0d0 .AND. QToHeatSetPt .LT. 0.0d0

                //     If the furnace has a net heating capacity and the zone temp is below the Tstat cooling setpoint and
                //     the net heating capacity does not just offset the heating load
                if (SensibleOutput > 0.0 && QToCoolSetPt > 0.0 &&
                    std::abs(SensibleOutput - QToHeatSetPt) > (Small5WLoad / furnace.ControlZoneMassFlowFrac)) {
                    if (SensibleOutput > QToCoolSetPt) {
                        QZnReq = QToCoolSetPt;
                        //         Don't set mode TRUE unless mode is allowed. Also check for floating zone.
                        if (state.dataHeatBalFanSys->TempControlType(furnace.ControlZoneNum) == HVAC::SetptType::SingleHeat ||
                            state.dataHeatBalFanSys->TempControlType(furnace.ControlZoneNum) == HVAC::SetptType::Uncontrolled) {
                            state.dataFurnaces->CoolingLoad = false;
                        } else {
                            state.dataFurnaces->CoolingLoad = true;
                        }
                        state.dataFurnaces->HeatingLoad = false;

                        SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, fanOp, QZnReq, MoistureLoad, PartLoadRatio);
                        if (furnace.NumOfSpeedCooling > 0) {
                            CalcVarSpeedHeatPump(state,
                                                 FurnaceNum,
                                                 false,
                                                 HVAC::CompressorOp::Off,
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
                                              HVAC::FanOp::Invalid,
                                              HVAC::CompressorOp::Off,
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
                            SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, fanOp, QZnReq, MoistureLoad, PartLoadRatio);
                        }
                    } else if (SensibleOutput > QZnReq) {
                        //         If the net heating capacity meets the zone heating load but does not overshoot, turn off heating
                        QZnReq = 0.0;
                        state.dataFurnaces->HeatingLoad = false;
                        SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, fanOp, QZnReq, MoistureLoad, PartLoadRatio);
                    }
                    //     the net heating capacity just offsets the heating load, turn off heating
                } else if (SensibleOutput > 0.0 && QToHeatSetPt > 0.0 &&
                           std::abs(SensibleOutput - QToHeatSetPt) < (Small5WLoad / furnace.ControlZoneMassFlowFrac)) {
                    state.dataFurnaces->HeatingLoad = false;
                } // SensibleOutput .GT. 0.0d0 .AND. QToCoolSetPt .GT. 0.0d0
            } // Furnace(FurnaceNum)%ControlZoneMassFlowFrac .GT. 0.0d0
            ZoneLoad = QZnReq;
        } // fanOp .EQ. FanOp::Continuous

        if (FirstHVACIteration) {
            furnace.iterationCounter = 0;
            furnace.iterationMode = Furnaces::ModeOfOperation::NoCoolHeat;
        }
        furnace.iterationCounter += 1;

        // push iteration mode stack and set current mode
        furnace.iterationMode(3) = furnace.iterationMode(2);
        furnace.iterationMode(2) = furnace.iterationMode(1);
        if (state.dataFurnaces->CoolingLoad) {
            furnace.iterationMode(1) = Furnaces::ModeOfOperation::CoolingMode;
        } else if (state.dataFurnaces->HeatingLoad) {
            furnace.iterationMode(1) = Furnaces::ModeOfOperation::HeatingMode;
        } else {
            furnace.iterationMode(1) = Furnaces::ModeOfOperation::NoCoolHeat;
        }

        // IF small loads to meet or not converging, just shut down unit
        if (std::abs(ZoneLoad) < Small5WLoad) {
            ZoneLoad = 0.0;
            state.dataFurnaces->CoolingLoad = false;
            state.dataFurnaces->HeatingLoad = false;
        } else if (furnace.iterationCounter > (state.dataHVACGlobal->MinAirLoopIterationsAfterFirst + 4)) {
            // attempt to lock output (air flow) if oscillations are detected
            OperatingMode = furnace.iterationMode(1);
            OperatingModeMinusOne = furnace.iterationMode(2);
            OperatingModeMinusTwo = furnace.iterationMode(3);
            Oscillate = true;
            if (OperatingMode == OperatingModeMinusOne && OperatingMode == OperatingModeMinusTwo) {
                Oscillate = false;
            }
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
        if (furnace.EMSOverrideSensZoneLoadRequest) ZoneLoad = furnace.EMSSensibleZoneLoadValue;
        if (furnace.EMSOverrideMoistZoneLoadRequest) MoistureLoad = furnace.EMSMoistureZoneLoadValue;
        if (furnace.EMSOverrideSensZoneLoadRequest || furnace.EMSOverrideMoistZoneLoadRequest) {
            if ((ZoneLoad != 0.0) && (furnace.EMSOverrideSensZoneLoadRequest)) {
                PartLoadRatio = 1.0;
            } else if ((MoistureLoad != 0.0) && (furnace.EMSOverrideMoistZoneLoadRequest)) {
                PartLoadRatio = 1.0;
            } else {
                PartLoadRatio = 0.0;
            }
            if (furnace.NumOfSpeedCooling > 0) {
                SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, fanOp, QZnReq, MoistureLoad, PartLoadRatio);
            } else {
                // This line is suspicious - all other calls to SetOnOffMassFlowRate pass in QZnReq, not ZoneLoad
                // either way, it seems these two should be using the same parameters.
                SetOnOffMassFlowRate(state, FurnaceNum, AirLoopNum, OnOffAirFlowRatio, fanOp, ZoneLoad, MoistureLoad, PartLoadRatio);
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
                              HVAC::FanOp const fanOp,                // fan operating mode
                              [[maybe_unused]] Real64 const ZoneLoad, // sensible load to be met (W) !unused1208
                              Real64 const MoistureLoad,              // moisture load to be met (W)
                              Real64 const PartLoadRatio              // coil part-load ratio
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   Sep 2008

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the Furnace Components.

        // METHODOLOGY EMPLOYED:
        // The HeatCool furnace/unitarysystem and air-to-air heat pump may have alternate air flow rates
        // in cooling, heating, and when no cooling or heating is needed. Set up the coil (comp) ON and OFF
        // air flow rates. Use these flow rates during the Calc routines to set the average mass flow rates
        // based on PLR.

        auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);
        // Check for heat only furnace
        if (furnace.type != HVAC::UnitarySysType::Furnace_HeatOnly &&
            furnace.type != HVAC::UnitarySysType::Unitary_HeatOnly) {

            // Set the system mass flow rates
            if (fanOp == HVAC::FanOp::Continuous) {
                // Set the compressor or coil ON mass flow rate
                // constant fan mode
                if (state.dataFurnaces->HeatingLoad) {
                    //       IF a heating and moisture load exists, operate at the cooling mass flow rate ELSE operate at the heating flow rate
                    if (MoistureLoad < 0.0 && furnace.Humidistat &&
                        furnace.DehumidControlType_Num == DehumidificationControlMode::CoolReheat) {
                        state.dataFurnaces->CompOnMassFlow = furnace.MaxCoolAirMassFlow;
                        state.dataFurnaces->CompOnFlowRatio = furnace.CoolingSpeedRatio;
                    } else {
                        state.dataFurnaces->CompOnMassFlow = furnace.MaxHeatAirMassFlow;
                        state.dataFurnaces->CompOnFlowRatio = furnace.HeatingSpeedRatio;
                    }
                    furnace.LastMode = Furnaces::ModeOfOperation::HeatingMode;
                    //     IF a cooling load exists, operate at the cooling mass flow rate
                } else if (state.dataFurnaces->CoolingLoad) {
                    state.dataFurnaces->CompOnMassFlow = furnace.MaxCoolAirMassFlow;
                    state.dataFurnaces->CompOnFlowRatio = furnace.CoolingSpeedRatio;
                    furnace.LastMode = Furnaces::ModeOfOperation::CoolingMode;
                    //     If no load exists, set the compressor on mass flow rate.
                    //     Set equal the mass flow rate when no heating or cooling is needed if no moisture load exists.
                    //     If the user has set the off mass flow rate to 0, set according to the last operating mode.
                } else {
                    if (MoistureLoad < 0.0 && furnace.Humidistat &&
                        furnace.DehumidControlType_Num == DehumidificationControlMode::CoolReheat) {
                        state.dataFurnaces->CompOnMassFlow = furnace.MaxCoolAirMassFlow;
                        state.dataFurnaces->CompOnFlowRatio = furnace.CoolingSpeedRatio;
                    } else {
                        state.dataFurnaces->CompOnMassFlow = furnace.MaxNoCoolHeatAirMassFlow;
                        state.dataFurnaces->CompOnFlowRatio = furnace.HeatingSpeedRatio;
                        //         User may have entered a 0 for MaxNoCoolHeatAirMassFlow
                        if (state.dataFurnaces->CompOnMassFlow == 0.0) {
                            if (furnace.LastMode == Furnaces::ModeOfOperation::HeatingMode) {
                                state.dataFurnaces->CompOnMassFlow = furnace.MaxHeatAirMassFlow;
                                state.dataFurnaces->CompOnFlowRatio = furnace.HeatingSpeedRatio;
                            } else {
                                state.dataFurnaces->CompOnMassFlow = furnace.MaxCoolAirMassFlow;
                                state.dataFurnaces->CompOnFlowRatio = furnace.CoolingSpeedRatio;
                            }
                        }
                    }
                }

                //     Set the compressor or coil OFF mass flow rate based on LOGICAL flag
                //     UseCompressorOnFlow is used when the user does not enter a value for no cooling or heating flow rate
                if (furnace.AirFlowControl == AirFlowControlConstFan::UseCompressorOnFlow) {
                    if (furnace.LastMode == Furnaces::ModeOfOperation::HeatingMode) {
                        if (MoistureLoad < 0.0 && furnace.Humidistat &&
                            furnace.DehumidControlType_Num == DehumidificationControlMode::CoolReheat) {
                            state.dataFurnaces->CompOffMassFlow = furnace.MaxCoolAirMassFlow;
                            state.dataFurnaces->CompOffFlowRatio = furnace.CoolingSpeedRatio;
                        } else {
                            state.dataFurnaces->CompOffMassFlow = furnace.MaxHeatAirMassFlow;
                            state.dataFurnaces->CompOffFlowRatio = furnace.HeatingSpeedRatio;
                        }
                    } else {
                        state.dataFurnaces->CompOffMassFlow = furnace.MaxCoolAirMassFlow;
                        state.dataFurnaces->CompOffFlowRatio = furnace.CoolingSpeedRatio;
                    }
                    //     ELSE use the user specified value
                } else {
                    state.dataFurnaces->CompOffMassFlow = furnace.MaxNoCoolHeatAirMassFlow;
                    state.dataFurnaces->CompOffFlowRatio = furnace.NoHeatCoolSpeedRatio;
                }
            } else {
                //     cycling fan mode
                if (state.dataFurnaces->HeatingLoad ||
                    (furnace.Humidistat && MoistureLoad < 0.0 && furnace.DehumidControlType_Num == DehumidificationControlMode::CoolReheat)) {

                    if (furnace.Humidistat && MoistureLoad < 0.0 &&
                        furnace.DehumidControlType_Num == DehumidificationControlMode::CoolReheat) {
                        state.dataFurnaces->CompOnMassFlow = furnace.MaxCoolAirMassFlow;
                        state.dataFurnaces->CompOnFlowRatio = furnace.CoolingSpeedRatio;
                        furnace.LastMode = Furnaces::ModeOfOperation::CoolingMode;
                    } else {
                        state.dataFurnaces->CompOnMassFlow = furnace.MaxHeatAirMassFlow;
                        state.dataFurnaces->CompOnFlowRatio = furnace.HeatingSpeedRatio;
                        furnace.LastMode = Furnaces::ModeOfOperation::HeatingMode;
                    }
                } else if (state.dataFurnaces->CoolingLoad) {
                    state.dataFurnaces->CompOnMassFlow = furnace.MaxCoolAirMassFlow;
                    state.dataFurnaces->CompOnFlowRatio = furnace.CoolingSpeedRatio;
                } else {
                    state.dataFurnaces->CompOnMassFlow = 0.0;
                    state.dataFurnaces->CompOnFlowRatio = 0.0;
                }
                state.dataFurnaces->CompOffMassFlow = 0.0;
                state.dataFurnaces->CompOffFlowRatio = 0.0;
            }
        } else { //  Is a HeatOnly furnace

            state.dataFurnaces->CompOnMassFlow = furnace.DesignMassFlowRate;
            state.dataFurnaces->CompOnFlowRatio = furnace.HeatingSpeedRatio;
            if (fanOp == HVAC::FanOp::Continuous) {
                state.dataFurnaces->CompOffMassFlow = furnace.MaxNoCoolHeatAirMassFlow;
                state.dataFurnaces->CompOffFlowRatio = furnace.HeatingSpeedRatio;
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

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing Furnace Components for which nominal capacities
        // and flow rates have not been specified in the input

        // METHODOLOGY EMPLOYED:
        // Obtains heating capacities and flow rates from the zone or system sizing arrays.
        // NOTE: In UNITARYSYSTEM:HEATPUMP:AIRTOAIR we are sizing the heating capacity to be
        // equal to the cooling capacity.  Thus the cooling and
        // and heating capacities of a DX heat pump system will be identical. In real life the ARI
        // heating and cooling capacities are close but not identical.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Iter;                 // iteration count
        Real64 MulSpeedFlowScale; // variable speed air flow scaling factor
        Real64 dummy(0.0);
        bool anyRan;
        std::string_view constexpr RoutineName = "SizeFurnace";
        EMSManager::ManageEMS(state, EMSManager::EMSCallFrom::UnitarySystemSizing, anyRan, ObjexxFCL::Optional_int_const()); // calling point

        state.dataSize->DXCoolCap = 0.0;
        state.dataSize->UnitaryHeatCap = 0.0;
        state.dataSize->SuppHeatCap = 0.0;
        auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);

        state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).supFanNum = furnace.FanIndex;
        state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).supFanType = furnace.fanType;
        state.dataSize->DataFanType = furnace.fanType;
        state.dataSize->DataFanIndex = furnace.FanIndex;

        state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).supFanPlace = furnace.fanPlace;

        // ACCA Manual S sizing
        bool const isHeatPump = furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_AirToAir ||
                                furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_WaterToAir;
        Real64 SysTotCoolingLoad;
        Real64 SysCoolingLoad;
        Real64 SysCoolingCapacity;
        Real64 SysLatCoolingLoad;
        Real64 SysHeatingCapacity;
        Real64 SysHeatingLoad;
        if (isHeatPump && !state.dataSize->FinalSysSizing.empty() &&
            state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).heatCoilSizingMethod != DataSizing::HeatCoilSizMethod::None &&
            !furnace.isIHP) {
            bool const PrintFlag = false;
            bool errorsFound = false;
            std::string_view CompType;
            std::string_view CompName;
            auto const &finalSysSizing = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum);
            if (finalSysSizing.SizingOption == DataSizing::SizingConcurrence::NonCoincident) {
                state.dataSize->DataFlowUsedForSizing = finalSysSizing.NonCoinCoolMassFlow / state.dataEnvrn->StdRhoAir;
            } else {
                state.dataSize->DataFlowUsedForSizing = finalSysSizing.CoinCoolMassFlow / state.dataEnvrn->StdRhoAir;
            }

            if (furnace.coolCoilType == HVAC::CoilType::CoolingWAHPSimple) {
                // auto const &thisCoil = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(thisFurnace.CoolingCoilIndex);
                state.dataSize->DataTotCapCurveIndex = 0; // this model uses a non-standard CapFT curve, disregard CapFT for now
            } else if (furnace.coolCoilType == HVAC::CoilType::CoolingWAHPVariableSpeedEquationFit ||
                       furnace.coolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
                auto const &thisCoil = state.dataVariableSpeedCoils->VarSpeedCoil(furnace.CoolCoilNum);
                state.dataSize->DataTotCapCurveIndex = thisCoil.MSCCapFTemp(thisCoil.NumOfSpeeds);
            } else {
                if (!state.dataDXCoils->DXCoil.empty() && furnace.CoolCoilNum <= (int)state.dataDXCoils->DXCoil.size()) {
                    auto const &thisCoil = state.dataDXCoils->DXCoil(furnace.CoolCoilNum);
                    state.dataSize->DataTotCapCurveIndex = thisCoil.CCapFTemp(thisCoil.NumCapacityStages);
                } else {
                    ShowWarningError(state,
                                     format("Developer Error in Heat Pump ACCA Sizing: cooling coil not found for {}:{} with coil type = {}.",
                                            furnace.type,
                                            furnace.Name,
                                            HVAC::coilTypeNames[(int)furnace.coolCoilType]));
                }
            }
            Real64 TempSize = furnace.DesignCoolingCapacity;
            CoolingCapacitySizer sizingCoolingCapacity;
            sizingCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            SysCoolingCapacity = sizingCoolingCapacity.size(state, TempSize, errorsFound);
            SysCoolingLoad = finalSysSizing.SensCoolCap;
            SysTotCoolingLoad = finalSysSizing.TotCoolCap;
            SysLatCoolingLoad = SysTotCoolingLoad - SysCoolingLoad;

            if (finalSysSizing.SizingOption == DataSizing::SizingConcurrence::NonCoincident) {
                state.dataSize->DataFlowUsedForSizing = finalSysSizing.NonCoinHeatMassFlow / state.dataEnvrn->StdRhoAir;
            } else {
                state.dataSize->DataFlowUsedForSizing = finalSysSizing.CoinHeatMassFlow / state.dataEnvrn->StdRhoAir;
            }

            TempSize = furnace.DesignHeatingCapacity;
            HeatingCapacitySizer sizingHeatingCapacity;
            sizingHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            SysHeatingCapacity = sizingHeatingCapacity.size(state, TempSize, errorsFound) / furnace.HeatSizingRatio;
            SysHeatingLoad = finalSysSizing.HeatCap;

            DataSizing::setHeatPumpSize(state, SysCoolingCapacity, SysHeatingCapacity, furnace.HeatSizingRatio);
            auto &EqSizing = state.dataSize->UnitarySysEqSizing(state.dataSize->CurSysNum);
            EqSizing.CoolingCapacity = true;
            EqSizing.DesCoolingLoad = SysCoolingCapacity;
            EqSizing.HeatingCapacity = true;
            EqSizing.DesHeatingLoad = SysHeatingCapacity;
            state.dataSize->DataFlowUsedForSizing = 0.0;
        }

        if (furnace.coolCoilType == HVAC::CoilType::CoolingDXSingleSpeed) {
            DXCoils::SimDXCoil(state, BlankString, HVAC::CompressorOp::On, true, furnace.CoolCoilNum, HVAC::FanOp::Cycling, 0.0);
        } else if (furnace.coolCoilType == HVAC::CoilType::CoolingDXHXAssisted) {
            auto const &hxCoil = state.dataHVACAssistedCC->HXAssistedCoils(furnace.CoolCoilNum);
            if (hxCoil.coolCoilType == HVAC::CoilType::CoolingDX) {
                if (hxCoil.CoolCoilNum < 0) {
                    ShowContinueError(state, "Occurs in sizing HeatExchangerAssistedCoolingCoil.");
                }
                auto &newCoil = state.dataCoilCoolingDX->coilCoolingDXs[hxCoil.CoolCoilNum];
                newCoil.size(state);
            }
            HXAssistCoil::SimHXAssistedCoolingCoil(
                state, furnace.CoolCoilNum, true, HVAC::CompressorOp::On, 0.0, HVAC::FanOp::Cycling, false, 1.0, false);

        } else if (furnace.coolCoilType == HVAC::CoilType::CoolingWAHPSimple) {
            WaterToAirHeatPumpSimple::SimWatertoAirHPSimple(state,
                                                            furnace.CoolCoilNum,
                                                            furnace.CoolingCoilSensDemand,
                                                            furnace.CoolingCoilLatentDemand,
                                                            HVAC::FanOp::Invalid, // Using invalid to mean off?
                                                            HVAC::CompressorOp::Off,
                                                            0.0,
                                                            FirstHVACIteration); // CoolPartLoadRatio
            if (furnace.heatCoilType == HVAC::CoilType::HeatingWAHPSimple) {
                WaterToAirHeatPumpSimple::SimWatertoAirHPSimple(state,
                                                                furnace.HeatCoilNum,
                                                                furnace.HeatingCoilSensDemand,
                                                                dummy,
                                                                HVAC::FanOp::Invalid, // using Invalid to mean off?
                                                                HVAC::CompressorOp::Off,
                                                                0.0,
                                                                FirstHVACIteration);
            }
            
        } else if (furnace.coolCoilType == HVAC::CoilType::CoolingWAHPVariableSpeedEquationFit ||
                   furnace.coolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
            auto const &vsCoil = state.dataVariableSpeedCoils->VarSpeedCoil(furnace.CoolCoilNum);
            furnace.NumOfSpeedCooling = vsCoil.NumOfSpeeds;
            
            if (furnace.isIHP) {
                IntegratedHeatPump::SizeIHP(state, furnace.ihpNum);
                MulSpeedFlowScale = vsCoil.RatedAirVolFlowRate / vsCoil.MSRatedAirVolFlowRate(vsCoil.NormSpedLevel);
                state.dataIntegratedHP->IntegratedHeatPumps(furnace.ihpNum).CoolVolFlowScale = MulSpeedFlowScale;
            } else {
                VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                          BlankString,
                                                          furnace.CoolCoilNum,
                                                          HVAC::FanOp::Invalid, // USing Invalid for off?
                                                          HVAC::CompressorOp::Off,
                                                          0.0,
                                                          1,
                                                          0.0,
                                                          0.0,
                                                          0.0,
                                                          0.0); // conduct the sizing operation in the VS WSHP
                MulSpeedFlowScale = vsCoil.RatedAirVolFlowRate / vsCoil.MSRatedAirVolFlowRate(vsCoil.NormSpedLevel);
            }

            for (Iter = 1; Iter <= furnace.NumOfSpeedCooling; ++Iter) {
                furnace.CoolVolumeFlowRate(Iter) = vsCoil.MSRatedAirVolFlowRate(Iter) * MulSpeedFlowScale;
                furnace.CoolMassFlowRate(Iter) = vsCoil.MSRatedAirMassFlowRate(Iter) * MulSpeedFlowScale;
                furnace.MSCoolingSpeedRatio(Iter) = vsCoil.MSRatedAirVolFlowRate(Iter) / vsCoil.MSRatedAirVolFlowRate(furnace.NumOfSpeedCooling);
            }

            if (furnace.heatCoilType == HVAC::CoilType::HeatingWAHPVariableSpeedEquationFit ||
                furnace.heatCoilType == HVAC::CoilType::HeatingDXVariableSpeed) {

                // In the case of IHP, HeatCoilNum is the index of
                // the Coil in the IHP.  IHP index is the index of the
                // IHP object.
                auto &vsCoil = state.dataVariableSpeedCoils->VarSpeedCoil(furnace.HeatCoilNum);
                furnace.NumOfSpeedHeating = vsCoil.NumOfSpeeds;
                
                if (furnace.isIHP) {
                    IntegratedHeatPump::SizeIHP(state, furnace.ihpNum);
                    MulSpeedFlowScale = vsCoil.RatedAirVolFlowRate / vsCoil.MSRatedAirVolFlowRate(vsCoil.NormSpedLevel);
                    state.dataIntegratedHP->IntegratedHeatPumps(furnace.ihpNum).HeatVolFlowScale = MulSpeedFlowScale;
                } else {
                    VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                              BlankString,
                                                              furnace.HeatCoilNum,
                                                              HVAC::FanOp::Invalid, // Invalid for off?
                                                              HVAC::CompressorOp::Off,
                                                              0.0,
                                                              1,
                                                              0.0,
                                                              0.0,
                                                              0.0,
                                                              0.0); // conduct the sizing operation in the VS WSHP
                    MulSpeedFlowScale = vsCoil.RatedAirVolFlowRate / vsCoil.MSRatedAirVolFlowRate(vsCoil.NormSpedLevel);
                }

                for (Iter = 1; Iter <= furnace.NumOfSpeedHeating; ++Iter) {
                    furnace.HeatVolumeFlowRate(Iter) = vsCoil.MSRatedAirVolFlowRate(Iter) * MulSpeedFlowScale;
                    furnace.HeatMassFlowRate(Iter) = vsCoil.MSRatedAirMassFlowRate(Iter) * MulSpeedFlowScale;
                    furnace.MSHeatingSpeedRatio(Iter) = vsCoil.MSRatedAirVolFlowRate(Iter) / vsCoil.MSRatedAirVolFlowRate(furnace.NumOfSpeedHeating);
                }
            }

            if (furnace.NumOfSpeedHeating > 0) {
                furnace.IdleMassFlowRate = min(furnace.HeatMassFlowRate(1), furnace.CoolMassFlowRate(1));
                furnace.IdleSpeedRatio = min(furnace.MSHeatingSpeedRatio(1), furnace.MSCoolingSpeedRatio(1));
                furnace.IdleVolumeAirRate = min(furnace.HeatVolumeFlowRate(1), furnace.CoolVolumeFlowRate(1));
            } else {
                furnace.IdleMassFlowRate = furnace.CoolMassFlowRate(1);
                furnace.IdleSpeedRatio = furnace.MSCoolingSpeedRatio(1);
                furnace.IdleVolumeAirRate = furnace.CoolVolumeFlowRate(1);
            }

            if (furnace.fanOp == HVAC::FanOp::Continuous) {
                furnace.MaxNoCoolHeatAirVolFlow = furnace.IdleVolumeAirRate;
                furnace.MaxNoCoolHeatAirMassFlow = furnace.IdleMassFlowRate;
                furnace.NoHeatCoolSpeedRatio = furnace.IdleSpeedRatio;
            }
        }

        // ACCA Manual S reporting
        if (isHeatPump && state.dataSize->CurSysNum > 0 && !state.dataSize->FinalSysSizing.empty() &&
            state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).heatCoilSizingMethod != DataSizing::HeatCoilSizMethod::None &&
            !furnace.isIHP) {
            std::string_view cCoilName;
            std::string_view hCoilName;
            Real64 coilSHR;
            if (furnace.coolCoilType == HVAC::CoilType::CoolingWAHPSimple) {
                auto const &thisCoil = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(furnace.CoolCoilNum);
                cCoilName = thisCoil.Name;
                coilSHR = 0.8;
            } else if (furnace.coolCoilType == HVAC::CoilType::CoolingWAHPVariableSpeedEquationFit ||
                       furnace.coolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
                auto const &thisCoil = state.dataVariableSpeedCoils->VarSpeedCoil(furnace.CoolCoilNum);
                cCoilName = thisCoil.Name;
                coilSHR = thisCoil.MSRatedSHR(thisCoil.NumOfSpeeds);
            } else {
                auto const &thisCoil = state.dataDXCoils->DXCoil(furnace.CoolCoilNum);
                cCoilName = thisCoil.Name;
                coilSHR = (thisCoil.NumOfSpeeds == 0) ? thisCoil.RatedSHR(1) : thisCoil.RatedSHR(thisCoil.NumOfSpeeds);
            }
            if (furnace.heatCoilType == HVAC::CoilType::HeatingWAHPSimple) {
                auto const &thisCoil = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(furnace.HeatCoilNum);
                hCoilName = thisCoil.Name;
            } else if (furnace.heatCoilType == HVAC::CoilType::HeatingWAHPVariableSpeedEquationFit ||
                       furnace.heatCoilType == HVAC::CoilType::HeatingDXVariableSpeed) {
                auto const &thisCoil = state.dataVariableSpeedCoils->VarSpeedCoil(furnace.HeatCoilNum);
                hCoilName = thisCoil.Name;
            } else {
                auto const &thisCoil = state.dataDXCoils->DXCoil(furnace.HeatCoilNum);
                hCoilName = thisCoil.Name;
            }
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchMSHPType, cCoilName, HVAC::unitarySysTypeNamesUC[int(furnace.type)]);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSHPName, cCoilName, furnace.Name);
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchMSCoilType, cCoilName, HVAC::coilTypeNames[(int)furnace.coolCoilType]);
            OutputReportPredefined::PreDefTableEntry(state,
                                                     state.dataOutRptPredefined->pdchMSSizMethod,
                                                     cCoilName,
                                                     DataSizing::HeatCoilSizMethodNamesUC[static_cast<int>(
                                                         state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).heatCoilSizingMethod)]);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSTotLoad, cCoilName, SysTotCoolingLoad);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSSensLoad, cCoilName, SysCoolingLoad);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSTotCapacity, cCoilName, SysCoolingCapacity);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSSensCapacity, cCoilName, SysCoolingCapacity * coilSHR);
            Real64 coilTotFactor = (SysTotCoolingLoad > 0.0) ? SysCoolingCapacity / SysTotCoolingLoad : 1.0;
            Real64 coilSensFactor = (SysCoolingLoad > 0.0) ? SysCoolingCapacity * coilSHR / SysCoolingLoad : 1.0;
            Real64 coilLatFactor = (SysLatCoolingLoad > 0.0) ? SysCoolingCapacity * (1.0 - coilSHR) / SysLatCoolingLoad : 1.0;
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSTotRatio, cCoilName, coilTotFactor);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSSensRatio, cCoilName, coilSensFactor);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSLatRatio, cCoilName, coilLatFactor);

            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchMSHPType, hCoilName, HVAC::unitarySysTypeNamesUC[(int)furnace.type]);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSHPName, hCoilName, furnace.Name);
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchMSCoilType, hCoilName, HVAC::coilTypeNames[(int)furnace.heatCoilType]);
            OutputReportPredefined::PreDefTableEntry(state,
                                                     state.dataOutRptPredefined->pdchMSSizMethod,
                                                     hCoilName,
                                                     DataSizing::HeatCoilSizMethodNamesUC[static_cast<int>(
                                                         state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).heatCoilSizingMethod)]);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSTotLoad, hCoilName, SysHeatingLoad);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSSensLoad, hCoilName, SysHeatingLoad);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSTotCapacity, hCoilName, SysHeatingCapacity);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSSensCapacity, hCoilName, SysHeatingCapacity);
            coilTotFactor = (SysHeatingLoad > 0.0) ? SysHeatingCapacity / SysHeatingLoad : 1.0;
            coilLatFactor = 0.0;
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSTotRatio, hCoilName, coilTotFactor);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSSensRatio, hCoilName, coilTotFactor);
            OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSLatRatio, hCoilName, coilLatFactor);
        }

        if (furnace.DesignFanVolFlowRate == DataSizing::AutoSize) {

            if (state.dataSize->CurSysNum > 0) {

                CheckSysSizing(state, HVAC::unitarySysTypeNames[(int)furnace.type], furnace.Name);
                if (state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow >= HVAC::SmallAirVolFlow) {
                    furnace.DesignFanVolFlowRate = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow;
                } else {
                    furnace.DesignFanVolFlowRate = 0.0;
                }

                if (furnace.DesignFanVolFlowRateEMSOverrideOn) {
                    furnace.DesignFanVolFlowRate = furnace.DesignFanVolFlowRateEMSOverrideValue;
                }

                BaseSizer::reportSizerOutput(state,
                                             HVAC::unitarySysTypeNames[(int)furnace.type],
                                             furnace.Name,
                                             "Supply Air Flow Rate [m3/s]",
                                             furnace.DesignFanVolFlowRate);
            }
        }

        if (furnace.MaxHeatAirVolFlow == DataSizing::AutoSize) {

            if (state.dataSize->CurSysNum > 0) {

                CheckSysSizing(state, HVAC::unitarySysTypeNames[(int)furnace.type], furnace.Name);
                if (state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow >= HVAC::SmallAirVolFlow) {
                    furnace.MaxHeatAirVolFlow = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow;
                } else {
                    furnace.MaxHeatAirVolFlow = 0.0;
                }

                if (furnace.MaxHeatAirVolFlowEMSOverrideOn) {
                    furnace.MaxHeatAirVolFlow = furnace.MaxHeatAirVolFlowEMSOverrideValue;
                }
                BaseSizer::reportSizerOutput(state,
                                             HVAC::unitarySysTypeNames[(int)furnace.type],
                                             furnace.Name,
                                             "Supply Air Flow Rate During Heating Operation [m3/s]",
                                             furnace.MaxHeatAirVolFlow);
            }
        }

        if (furnace.MaxCoolAirVolFlow == DataSizing::AutoSize) {

            if (state.dataSize->CurSysNum > 0) {

                CheckSysSizing(state, HVAC::unitarySysTypeNames[(int)furnace.type], furnace.Name);
                if (state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow >= HVAC::SmallAirVolFlow) {
                    furnace.MaxCoolAirVolFlow = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow;
                } else {
                    furnace.MaxCoolAirVolFlow = 0.0;
                }

                if (furnace.MaxCoolAirVolFlowEMSOverrideOn) {
                    furnace.MaxCoolAirVolFlow = furnace.MaxCoolAirVolFlowEMSOverrideValue;
                }

                BaseSizer::reportSizerOutput(state,
                                             HVAC::unitarySysTypeNames[(int)furnace.type],
                                             furnace.Name,
                                             "Supply Air Flow Rate During Cooling Operation [m3/s]",
                                             furnace.MaxCoolAirVolFlow);
            }
        }

        if (furnace.MaxNoCoolHeatAirVolFlow == DataSizing::AutoSize) {

            if (state.dataSize->CurSysNum > 0) {

                CheckSysSizing(state, HVAC::unitarySysTypeNames[(int)furnace.type], furnace.Name);
                if (state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow >= HVAC::SmallAirVolFlow) {
                    furnace.MaxNoCoolHeatAirVolFlow = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow;
                } else {
                    furnace.MaxNoCoolHeatAirVolFlow = 0.0;
                }

                if (furnace.MaxNoCoolHeatAirVolFlowEMSOverrideOn) {
                    furnace.MaxNoCoolHeatAirVolFlow = furnace.MaxNoCoolHeatAirVolFlowEMSOverrideValue;
                }

                BaseSizer::reportSizerOutput(state,
                                             HVAC::unitarySysTypeNames[(int)furnace.type],
                                             furnace.Name,
                                             "Supply Air Flow Rate When No Cooling or Heating is Needed [m3/s]",
                                             furnace.MaxNoCoolHeatAirVolFlow);
            }
        }

        if (furnace.DesignHeatingCapacity == DataSizing::AutoSize) {

            if (state.dataSize->CurSysNum > 0) {

                if (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_AirToAir ||
                    furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_WaterToAir) {

                    CheckSysSizing(state, HVAC::unitarySysTypeNames[(int)furnace.type], furnace.Name);

                    if (furnace.heatCoilType == HVAC::CoilType::HeatingWAHPSimple) {
                        furnace.DesignHeatingCapacity =
                            state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(furnace.HeatCoilNum).RatedCapHeat;
                    } else {
                        furnace.DesignHeatingCapacity = state.dataSize->DXCoolCap;
                    }

                } else {

                    CheckSysSizing(state, HVAC::unitarySysTypeNames[(int)furnace.type], furnace.Name);

                    furnace.DesignHeatingCapacity = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).HeatCap;
                    if (state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).FractionOfAutosizedHeatingCapacity > 0) {
                        // apply sizing factor and reset it to 1.0 so it doesn't get apply again during heating coil sizing
                        furnace.DesignHeatingCapacity *=
                            state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).FractionOfAutosizedHeatingCapacity;
                        state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).FractionOfAutosizedHeatingCapacity = 1.0;
                    }
                }

                if (furnace.DesignHeatingCapacity < HVAC::SmallLoad) {
                    furnace.DesignHeatingCapacity = 0.0;
                }

                BaseSizer::reportSizerOutput(state,
                                             HVAC::unitarySysTypeNames[(int)furnace.type],
                                             furnace.Name,
                                             "Nominal Heating Capacity [W]",
                                             furnace.DesignHeatingCapacity);
            }
        }

        if (furnace.DesignCoolingCapacity == DataSizing::AutoSize) {

            if (state.dataSize->CurSysNum > 0) {

                CheckSysSizing(state, HVAC::unitarySysTypeNames[(int)furnace.type], furnace.Name);
                if (state.dataSize->DXCoolCap >= HVAC::SmallLoad) {
                    furnace.DesignCoolingCapacity = state.dataSize->DXCoolCap;
                } else {
                    furnace.DesignCoolingCapacity = 0.0;
                }
                BaseSizer::reportSizerOutput(state,
                                             HVAC::unitarySysTypeNames[(int)furnace.type],
                                             furnace.Name,
                                             "Nominal Cooling Capacity [W]",
                                             furnace.DesignCoolingCapacity);
            }
        }

        if (furnace.DesignMaxOutletTemp == DataSizing::AutoSize) {

            if (state.dataSize->CurSysNum > 0) {

                CheckSysSizing(state, HVAC::unitarySysTypeNames[(int)furnace.type], furnace.Name);
                furnace.DesignMaxOutletTemp = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).HeatSupTemp;
                BaseSizer::reportSizerOutput(state,
                                             HVAC::unitarySysTypeNames[(int)furnace.type],
                                             furnace.Name,
                                             "Maximum Supply Air Temperature from Supplemental Heater [C]",
                                             furnace.DesignMaxOutletTemp);
            }
        }

        if (furnace.DesignSuppHeatingCapacity == DataSizing::AutoSize) {

            if (state.dataSize->CurSysNum > 0) {

                CheckSysSizing(state, HVAC::unitarySysTypeNames[(int)furnace.type], furnace.Name);
                if (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_AirToAir ||
                    furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_WaterToAir) {
                    // set the supplemental heating capacity to the actual heating load
                    furnace.DesignSuppHeatingCapacity = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).HeatCap;
                    // if reheat needed for humidity control, make sure supplemental heating is at least as big
                    // as the cooling capacity
                    if (furnace.Humidistat && furnace.DehumidControlType_Num == DehumidificationControlMode::CoolReheat) {
                        furnace.DesignSuppHeatingCapacity = max(furnace.DesignSuppHeatingCapacity, furnace.DesignCoolingCapacity);
                        if (furnace.DesignSuppHeatingCapacity < HVAC::SmallLoad) {
                            furnace.DesignSuppHeatingCapacity = 0.0;
                        }
                    }

                } else {

                    if (furnace.Humidistat && furnace.DehumidControlType_Num == DehumidificationControlMode::CoolReheat) {
                        furnace.DesignSuppHeatingCapacity = furnace.DesignCoolingCapacity;
                    } else {
                        furnace.DesignSuppHeatingCapacity = 0.0;
                    }
                }

                BaseSizer::reportSizerOutput(state,
                                             HVAC::unitarySysTypeNames[(int)furnace.type],
                                             furnace.Name,
                                             "Supplemental Heating Coil Nominal Capacity [W]",
                                             furnace.DesignSuppHeatingCapacity);
            }
        }

        state.dataSize->UnitaryHeatCap = furnace.DesignHeatingCapacity;
        state.dataSize->SuppHeatCap = furnace.DesignSuppHeatingCapacity;
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

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the coil outlet nodes by simulating a heat-only
        // furnace or unitary system.

        // METHODOLOGY EMPLOYED:
        // Determine the operating PLR to meet the zone sensible load.

        // SUBROUTINE PARAMETER DEFINITIONS:
        int constexpr MaxIter(15);    // maximum number of iterations
        Real64 constexpr MinPLR(0.0); // minimum part load ratio allowed

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

        auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);
        // Retrieve the load on the controlled zone
        auto &furnaceInNode = state.dataLoopNodes->Node(furnace.FurnaceInletNode);
        auto const &furnaceOutNode = state.dataLoopNodes->Node(furnace.FurnaceOutletNode);
        int ControlZoneNode = furnace.NodeNumOfControlledZone;
        HVAC::FanOp fanOp = furnace.fanOp; // fan operating mode
        furnace.MdotFurnace = furnace.DesignMassFlowRate;
        furnace.CoolPartLoadRatio = 0.0;

        // Calculate the Cp Air of zone
        Real64 cpair = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(ControlZoneNode).HumRat);

        if (FirstHVACIteration) {
            HeatCoilLoad = ZoneLoad;
            state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
        } else {
            // If Furnace runs then set HeatCoilLoad on Heating Coil and the Mass Flow
            if ((furnace.availSched->getCurrentVal() > 0.0) && (furnaceInNode.MassFlowRate > 0.0) && (state.dataFurnaces->HeatingLoad)) {

                furnaceInNode.MassFlowRate = furnace.MdotFurnace;
                HeatCoilLoad = furnace.DesignHeatingCapacity;
                SystemSensibleLoad = ZoneLoad;

                // Get no load result
                if (fanOp == HVAC::FanOp::Cycling) {
                    furnaceInNode.MassFlowRate = 0.0;
                }
                if (fanOp == HVAC::FanOp::Continuous) {
                    state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0; // The on/off fan will not cycle, so set part-load fraction = 1
                }

                //     Set the inlet mass flow rate based on user specified coil OFF flow rate
                PartLoadRatio = 0.0;
                SetAverageAirFlow(state, FurnaceNum, PartLoadRatio, OnOffAirFlowRatio);

                CalcFurnaceOutput(state,
                                  FurnaceNum,
                                  FirstHVACIteration,
                                  fanOp,
                                  HVAC::CompressorOp::On,
                                  0.0,
                                  0.0,
                                  0.0,
                                  0.0,
                                  NoSensibleOutput,
                                  NoLatentOutput,
                                  OnOffAirFlowRatio,
                                  false);

                furnaceInNode.MassFlowRate = furnace.MdotFurnace;

                // Set fan part-load fraction equal to 1 while getting full load result
                state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
                OnOffAirFlowRatio = 1.0;

                // Get full load result
                CalcFurnaceOutput(state,
                                  FurnaceNum,
                                  FirstHVACIteration,
                                  fanOp,
                                  HVAC::CompressorOp::On,
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
                    if (fanOp == HVAC::FanOp::Cycling) {
                        furnaceInNode.MassFlowRate = furnace.MdotFurnace * PartLoadRatio;
                        HeatCoilLoad = furnace.DesignHeatingCapacity * PartLoadRatio;
                    } else { // FanOp::Continuous
                        if (furnaceOutNode.Temp > furnace.DesignMaxOutletTemp) {
                            deltaT = furnaceOutNode.Temp - furnace.DesignMaxOutletTemp;
                            if (HeatCoilLoad > furnace.DesignHeatingCapacity) HeatCoilLoad = furnace.DesignHeatingCapacity;
                            HeatCoilLoad -= furnaceInNode.MassFlowRate * cpair * deltaT;
                        } else {
                            HeatCoilLoad = SystemSensibleLoad - NoSensibleOutput;
                        }
                    }

                    // Calculate the part load ratio through iteration
                    HeatErrorToler = furnace.HeatingConvergenceTolerance; // Error tolerance for convergence from input deck
                    Error = 1.0;                                              // initialize error value for comparison against tolerance
                    state.dataFurnaces->Iter = 0;                             // initialize iteration counter
                    IterRelax = 0.9;                                          // relaxation factor for iterations
                    while (state.dataFurnaces->Iter <= MaxIter) {

                        if (fanOp == HVAC::FanOp::Cycling) furnaceInNode.MassFlowRate = furnace.MdotFurnace * PartLoadRatio;
                        CalcFurnaceOutput(state,
                                          FurnaceNum,
                                          FirstHVACIteration,
                                          fanOp,
                                          HVAC::CompressorOp::On,
                                          0.0,
                                          PartLoadRatio,
                                          HeatCoilLoad,
                                          0.0,
                                          ActualSensibleOutput,
                                          ActualLatentOutput,
                                          OnOffAirFlowRatio,
                                          false);

                        if (SystemSensibleLoad != 0.0) {
                            Error = (SystemSensibleLoad - ActualSensibleOutput) / (SystemSensibleLoad);
                        }
                        if (std::abs(Error) <= HeatErrorToler) {
                            break;
                        }
                        PartLoadRatio = max(
                            MinPLR,
                            min(1.0,
                                PartLoadRatio + IterRelax * (SystemSensibleLoad - ActualSensibleOutput) / (FullSensibleOutput - NoSensibleOutput)));

                        // limit the heating coil outlet air temperature to DesignMaxOutletTemp
                        if (furnaceOutNode.Temp > furnace.DesignMaxOutletTemp) {
                            deltaT = furnaceOutNode.Temp - furnace.DesignMaxOutletTemp;
                            if (HeatCoilLoad > furnace.DesignHeatingCapacity) HeatCoilLoad = furnace.DesignHeatingCapacity;
                            HeatCoilLoad -= furnaceInNode.MassFlowRate * cpair * deltaT;
                            CalcFurnaceOutput(state,
                                              FurnaceNum,
                                              FirstHVACIteration,
                                              fanOp,
                                              HVAC::CompressorOp::On,
                                              0.0,
                                              PartLoadRatio,
                                              HeatCoilLoad,
                                              0.0,
                                              ActualSensibleOutput,
                                              ActualLatentOutput,
                                              OnOffAirFlowRatio,
                                              false);

                            if (SystemSensibleLoad != 0.0) {
                                Error = (SystemSensibleLoad - ActualSensibleOutput) / (SystemSensibleLoad);
                            }
                            PartLoadRatio = max(MinPLR,
                                                min(1.0,
                                                    PartLoadRatio + IterRelax * (SystemSensibleLoad - ActualSensibleOutput) /
                                                                        (FullSensibleOutput - NoSensibleOutput)));
                        } else {
                            HeatCoilLoad = furnace.DesignHeatingCapacity * PartLoadRatio;
                        }

                        if (PartLoadRatio == MinPLR) {
                            break;
                        }
                        if (PartLoadRatio == 1.0) {
                            break;
                        }
                        ++state.dataFurnaces->Iter;
                        if (state.dataFurnaces->Iter == 7) {
                            IterRelax = 0.7;
                        }
                        if (state.dataFurnaces->Iter == 15) {
                            IterRelax = 0.4;
                        }
                    }

                    if (state.dataFurnaces->Iter > MaxIter) {
                        if (furnace.HeatingMaxIterIndex2 == 0) {
                            ShowWarningMessage(state,
                                               format("{} \"{}\" -- Exceeded max heating iterations ({}) while adjusting furnace runtime.",
                                                      HVAC::unitarySysTypeNames[(int)furnace.type],
                                                      furnace.Name,
                                                      MaxIter));
                            ShowContinueErrorTimeStamp(state, "");
                        }
                        ShowRecurringWarningErrorAtEnd(state,
                                                       format("{} \"{}\" -- Exceeded max heating iterations error continues...",
                                                              HVAC::unitarySysTypeNames[(int)furnace.type],
                                                              furnace.Name),
                                                       furnace.HeatingMaxIterIndex2);
                    }

                } else { // ELSE from IF(FullSensibleOutput.GT.NoSensibleOutput)THEN above
                    // Set part load ratio to 1 and run heater at design heating capacity
                    PartLoadRatio = 1.0;
                    HeatCoilLoad = furnace.DesignHeatingCapacity;
                }
                // Set the final results
                //      IF (fanOp .EQ. FanOp::Cycling) THEN
                //        Furnace(FurnaceNum)%MdotFurnace = Furnace(FurnaceNum)%MdotFurnace * PartLoadRatio
                //      END IF
                furnace.MdotFurnace = furnaceInNode.MassFlowRate;

            } else if ((furnace.availSched->getCurrentVal() > 0.0) && (furnaceInNode.MassFlowRate > 0.0) && (fanOp == HVAC::FanOp::Continuous)) {
                HeatCoilLoad = 0.0;
            } else { // no heating and no flow
                furnace.MdotFurnace = 0.0;
                HeatCoilLoad = 0.0;
            } // End of the Scheduled Furnace If block

        } // End of the FirstHVACIteration control of the mass flow If block

        // Set the fan inlet node flow rates
        furnaceInNode.MassFlowRateMaxAvail = furnace.MdotFurnace;
        furnaceInNode.MassFlowRate = furnace.MdotFurnace;
    }

    void CalcNewZoneHeatCoolFlowRates(EnergyPlusData &state,
                                      int const FurnaceNum,
                                      bool const FirstHVACIteration,
                                      HVAC::CompressorOp const compressorOp, // compressor operation flag (1=On, 0=Off)
                                      Real64 const ZoneLoad,                 // the control zone load (watts)
                                      Real64 const MoistureLoad,             // the control zone latent load (watts)
                                      Real64 &HeatCoilLoad,                  // Heating load to be met by heating coil ( excluding heat pump DX coil)
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
        // single time step (heating and dehumidification can occur). For all other system types,
        // only a single PLR is allowed for any given time step.
        // Order of simulation depends on dehumidification control option as described below.
        // Dehumidification control options:
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
        Real64 TempMinPLR;             // Temporary min latent PLR when hum control is required and iter is exceeded
        Real64 TempMinPLR2;            // Temporary min latent PLR when cyc fan hum control is required and iter is exceeded
        Real64 TempMaxPLR;             // Temporary max latent PLR when hum control is required and iter is exceeded
        Real64 QToHeatSetPt;           // Load required to meet heating setpoint temp (>0 is a heating load)
        Real64 CoolingHeatingPLRRatio; // ratio of cooling to heating PLR (MAX=1). Used in heating mode.
        Real64 HeatingSensibleOutput;
        Real64 HeatingLatentOutput;
        Real64 OutdoorDryBulbTemp; // secondary coil (condenser) entering dry bulb temperature

        Real64 &SystemSensibleLoad = state.dataFurnaces->SystemSensibleLoad;
        auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);
        // Set local variables
        int FurnaceOutletNode = furnace.FurnaceOutletNode;
        int FurnaceInletNode = furnace.FurnaceInletNode;
        int ControlZoneNode = furnace.NodeNumOfControlledZone;
        HVAC::FanOp fanOp = furnace.fanOp; // fan operating mode
        bool HumControl = false;
        // Calculate the Cp Air of zone
        Real64 cpair = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(ControlZoneNode).HumRat);
        NoHeatOutput = 0.0;
        SystemSensibleLoad = 0.0;
        ReheatCoilLoad = 0.0;
        HeatCoilLoad = 0.0;
        ReheatCoilLoad = 0.0;
        PartLoadRatio = 0.0;

        if (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_AirToAir) {
            if (state.dataDXCoils->DXCoil(furnace.HeatCoilNum)
                    .IsSecondaryDXCoilInZone) { // assumes compressor is in same location as secondary coil
                OutdoorDryBulbTemp =
                    state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataDXCoils->DXCoil(furnace.HeatCoilNum).SecZonePtr).ZT;
            } else if (state.dataDXCoils->DXCoil(furnace.CoolCoilNum).IsSecondaryDXCoilInZone) {
                OutdoorDryBulbTemp =
                    state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataDXCoils->DXCoil(furnace.CoolCoilNum).SecZonePtr).ZT;
            } else {
                if (furnace.CondenserNodeNum > 0) {
                    OutdoorDryBulbTemp = state.dataLoopNodes->Node(furnace.CondenserNodeNum).Temp;
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
                if (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_AirToAir ||
                    (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_WaterToAir && furnace.WatertoAirHPType == WAHPCoilType::Simple)) {
                    furnace.HeatPartLoadRatio = 1.0;
                    HeatCoilLoad = 0.0;
                    furnace.HeatingCoilSensDemand = 0.0;
                    furnace.CoolingCoilSensDemand = 0.0;
                    furnace.CoolingCoilLatentDemand = 0.0;
                } else { // for furnaces
                    furnace.HeatPartLoadRatio = 0.0;
                    HeatCoilLoad = ZoneLoad;
                    state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = furnace.MdotFurnace;
                    furnace.HeatingCoilSensDemand = 0.0;
                    furnace.CoolingCoilSensDemand = 0.0;
                    furnace.CoolingCoilLatentDemand = 0.0;
                }
                ReheatCoilLoad = 0.0;
                furnace.CoolPartLoadRatio = 0.0;

                // Init for cooling
            } else if (state.dataFurnaces->CoolingLoad) {
                // air to air heat pumps
                furnace.CoolPartLoadRatio = 1.0;
                furnace.HeatPartLoadRatio = 0.0;
                HeatCoilLoad = 0.0;
                ReheatCoilLoad = 0.0;

                // Init for moisture load only
            } else {
                furnace.CoolPartLoadRatio = 0.0;
                furnace.HeatPartLoadRatio = 0.0;
                HeatCoilLoad = 0.0;
                ReheatCoilLoad = 0.0;
                furnace.HeatingCoilSensDemand = 0.0;
                furnace.CoolingCoilSensDemand = 0.0;
                furnace.CoolingCoilLatentDemand = 0.0;
            }

            SetAverageAirFlow(state, FurnaceNum, max(furnace.HeatPartLoadRatio, furnace.CoolPartLoadRatio), OnOffAirFlowRatio);
            //  if dehumidification load exists (for heat pumps) turn on the supplemental heater
            if (state.dataFurnaces->HPDehumidificationLoadFlag) {
                HumControl = true;
            }
        } else { // not FirstHVACIteration
            // Init for heating
            Real64 &CoolCoilLoad = state.dataFurnaces->CoolCoilLoad;
            if (state.dataFurnaces->HeatingLoad) {
                CoolCoilLoad = 0.0;
                if (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_AirToAir ||
                    (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_WaterToAir && furnace.WatertoAirHPType == WAHPCoilType::Simple)) {
                    SystemSensibleLoad = ZoneLoad;
                    SystemMoistureLoad = 0.0;
                    HeatCoilLoad = 0.0;
                    furnace.HeatingCoilSensDemand = SystemSensibleLoad;
                    furnace.CoolingCoilSensDemand = 0.0;
                    furnace.CoolingCoilLatentDemand = 0.0;
                } else {
                    SystemMoistureLoad = MoistureLoad;
                    HeatCoilLoad = ZoneLoad;
                }

                // Init for cooling
            } else if (state.dataFurnaces->CoolingLoad) {
                CoolCoilLoad = ZoneLoad;
                SystemMoistureLoad = MoistureLoad;
                HeatCoilLoad = 0.0;
                furnace.CoolingCoilSensDemand = std::abs(CoolCoilLoad);
                furnace.CoolingCoilLatentDemand = std::abs(SystemMoistureLoad);
                furnace.HeatingCoilSensDemand = 0.0;

                // Init for latent
            } else {
                SystemMoistureLoad = MoistureLoad;
                CoolCoilLoad = 0.0;
                HeatCoilLoad = 0.0;
                // set report variables
                furnace.CoolingCoilSensDemand = 0.0;
                furnace.CoolingCoilLatentDemand = SystemMoistureLoad;
                furnace.HeatingCoilSensDemand = 0.0;
            }
            HeatingSensibleOutput = 0.0;
            HeatingLatentOutput = 0.0;
            ReheatCoilLoad = 0.0;
            furnace.CoolPartLoadRatio = 0.0;
            furnace.HeatPartLoadRatio = 0.0;
            furnace.CompPartLoadRatio = 0.0;
            furnace.DehumidInducedHeatingDemandRate = 0.0;

            // When humidity control is used with cycling fan control and a heating load exists, if a moisture load
            // also exists, the heating PLR must be available for the cooling coil calculations.
            //*********** Heating Section ************
            // If Furnace runs with a heating load then set HeatCoilLoad on Heating Coil and the Mass Flow
            //         (Node(FurnaceInletNode)%MassFlowRate .gt. 0.0d0) .and. &
            if ((furnace.availSched->getCurrentVal() > 0.0) && (state.dataFurnaces->HeatingLoad)) {

                //    Heat pumps only calculate a single PLR each time step (i.e. only cooling or heating allowed in a single time step)
                if (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_AirToAir ||
                    (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_WaterToAir && furnace.WatertoAirHPType == WAHPCoilType::Simple)) {

                    state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = furnace.MdotFurnace;

                    // Get no load result
                    if (fanOp == HVAC::FanOp::Cycling) {
                        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = 0.0;
                    }

                    //     Set the inlet mass flow rate based on user specified coil OFF flow rate
                    PartLoadRatio = 0.0;

                    SetAverageAirFlow(state, FurnaceNum, PartLoadRatio, OnOffAirFlowRatio);

                    // Set the input parameters for CalcFurnaceOutput
                    furnace.CompPartLoadRatio = 0.0; // compressor off

                    CalcFurnaceOutput(state,
                                      FurnaceNum,
                                      FirstHVACIteration,
                                      fanOp,
                                      compressorOp,
                                      0.0,
                                      MinPLR,
                                      0.0,
                                      0.0,
                                      NoHeatOutput,
                                      NoLatentOutput,
                                      OnOffAirFlowRatio,
                                      false);

                    PartLoadRatio = 1.0;
                    state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = furnace.MdotFurnace;

                    furnace.CompPartLoadRatio = 1.0; // compressor ON

                    // Set fan part-load fraction equal to 1 while getting full load result
                    state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
                    OnOffAirFlowRatio = 1.0;

                    // Get full load result
                    CalcFurnaceOutput(state,
                                      FurnaceNum,
                                      FirstHVACIteration,
                                      fanOp,
                                      compressorOp,
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
                        HeatErrorToler = furnace.HeatingConvergenceTolerance; // Error tolerance for convergence from input deck

                        int SolFlag = 0; // # of iterations if positive, -1 means failed to converge, -2 means bounds are incorrect
                        // HeatErrorToler is in fraction of load, MaxIter = 30, SolFalg = # of iterations or error as appropriate
                        auto f = [&state, FurnaceNum, FirstHVACIteration, fanOp, compressorOp, SystemSensibleLoad](Real64 const PartLoadRatio) {
                            return CalcFurnaceResidual(state,
                                                       PartLoadRatio,
                                                       FurnaceNum,
                                                       FirstHVACIteration,
                                                       fanOp,
                                                       compressorOp,
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
                                                  fanOp,
                                                  compressorOp,
                                                  0.0,
                                                  PartLoadRatio,
                                                  0.0,
                                                  0.0,
                                                  TempHeatOutput,
                                                  TempLatentOutput,
                                                  OnOffAirFlowRatio,
                                                  false);
                                if (std::abs(SystemSensibleLoad - TempHeatOutput) > HVAC::SmallLoad) {
                                    if (furnace.DXHeatingMaxIterIndex == 0) {
                                        ShowWarningMessage(state,
                                                           format("Heating coil control failed to converge for {}:{}",
                                                                  HVAC::unitarySysTypeNames[(int)furnace.type],
                                                                  furnace.Name));
                                        ShowContinueError(state,
                                                          "  Iteration limit exceeded in calculating DX heating coil sensible part-load ratio.");
                                        ShowContinueErrorTimeStamp(
                                            state,
                                            format("Sensible load to be met by DX heating coil = {:.2T} (watts), sensible output of DX heating "
                                                   "coil = {:.2T} (watts), and the simulation continues.",
                                                   SystemSensibleLoad,
                                                   TempHeatOutput));
                                    }
                                    ShowRecurringWarningErrorAtEnd(state,
                                                                   format("{} \"{}\" - Iteration limit exceeded in calculating DX sensible heating "
                                                                          "part-load ratio error continues. "
                                                                          "Sensible load statistics:",
                                                                          HVAC::unitarySysTypeNames[(int)furnace.type],
                                                                          furnace.Name),
                                                                   furnace.DXHeatingMaxIterIndex,
                                                                   SystemSensibleLoad,
                                                                   SystemSensibleLoad);
                                }
                            } else if (SolFlag == -2) {
                                if (furnace.DXHeatingRegulaFalsiFailedIndex == 0) {
                                    ShowWarningMessage(state,
                                                       format("Heating coil control failed for {}:{}",
                                                              HVAC::unitarySysTypeNames[(int)furnace.type],
                                                              furnace.Name));
                                    ShowContinueError(state, "  DX sensible heating part-load ratio determined to be outside the range of 0-1.");
                                    ShowContinueErrorTimeStamp(
                                        state,
                                        format("Sensible load to be met by DX heating coil = {:.2T} (watts), and the simulation continues.",
                                               SystemSensibleLoad));
                                }
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    format("{} \"{}\" -  DX sensible heating part-load ratio out of range error continues. Sensible load statistics:",
                                           HVAC::unitarySysTypeNames[(int)furnace.type],
                                           furnace.Name),
                                    furnace.DXHeatingRegulaFalsiFailedIndex,
                                    SystemSensibleLoad,
                                    SystemSensibleLoad);
                            }
                        }

                        furnace.HeatPartLoadRatio = PartLoadRatio;
                        //       Check if Heat Pump compressor is allowed to run based on outdoor temperature
                        if (OutdoorDryBulbTemp > furnace.MinOATCompressorHeating) {
                            furnace.CompPartLoadRatio = PartLoadRatio;
                        } else {
                            furnace.CompPartLoadRatio = 0.0;
                        }
                    } else if (SystemSensibleLoad > FullSensibleOutput) {
                        //       SystemSensibleLoad is greater than full DX Heating coil output so heat pump runs entire
                        //       timestep and additional supplemental heating is required
                        furnace.HeatPartLoadRatio = 1.0;
                        if (OutdoorDryBulbTemp > furnace.MinOATCompressorHeating) {
                            //       Check to see if Heat Pump compressor was allowed to run based on outdoor temperature
                            furnace.CompPartLoadRatio = 1.0;
                        } else {
                            furnace.CompPartLoadRatio = 0.0;
                        }
                    } else if (SystemSensibleLoad < NoHeatOutput) {
                        //       SystemSensibleLoad is less than minimum DX Heating coil output so heat pump does not run and
                        //       the load will be met by the supplemental heater
                        furnace.CompPartLoadRatio = 0.0;
                        furnace.HeatPartLoadRatio = 1.0;
                    }
                    if (furnace.HeatPartLoadRatio == 1.0) {
                        //       Determine the load on the supplemental heating coil
                        if ((SystemSensibleLoad - FullSensibleOutput) > furnace.DesignSuppHeatingCapacity) {
                            HeatCoilLoad = furnace.DesignSuppHeatingCapacity;
                            TempOutHeatingCoil = state.dataLoopNodes->Node(FurnaceOutletNode).Temp + HeatCoilLoad / (cpair * furnace.MdotFurnace);
                        } else if (SystemSensibleLoad < NoHeatOutput) {
                            HeatCoilLoad = max(0.0, SystemSensibleLoad); // BG 10/22/2008 need a case for when its all suppl heat
                            TempOutHeatingCoil = state.dataLoopNodes->Node(FurnaceInletNode).Temp + HeatCoilLoad / (cpair * furnace.MdotFurnace);
                        } else {
                            HeatCoilLoad = max(0.0, (SystemSensibleLoad - FullSensibleOutput));
                            TempOutHeatingCoil = state.dataLoopNodes->Node(FurnaceOutletNode).Temp + HeatCoilLoad / (cpair * furnace.MdotFurnace);
                        }
                        if (OutdoorDryBulbTemp > furnace.MaxOATSuppHeat) {
                            HeatCoilLoad = 0.0;
                            if (SystemSensibleLoad < NoHeatOutput) {
                                TempOutHeatingCoil = state.dataLoopNodes->Node(FurnaceInletNode).Temp;
                            } else {
                                TempOutHeatingCoil = state.dataLoopNodes->Node(FurnaceOutletNode).Temp;
                            }
                        }
                        if ((TempOutHeatingCoil > furnace.DesignMaxOutletTemp) && (HeatCoilLoad > 0.0)) {
                            // deltaT = Furnace(FurnaceNum)%DesignMaxOutletTemp - Node(FurnaceOutletNode)%Temp
                            // BG 10/22/2008 above made no sense if DX heat is off and its all supplemental,
                            //  because Node(FurnaceOutletNode)%Temp will have been calc'd with full DX heat in last faux call to CalcFurnaceOutput

                            Real64 cpairSupply = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(FurnaceInletNode).HumRat);
                            deltaT = (furnace.DesignMaxOutletTemp - TempOutHeatingCoil);
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

                    state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = furnace.MdotFurnace;
                    HeatCoilLoad = furnace.DesignHeatingCapacity;
                    SystemSensibleLoad = ZoneLoad;

                    // Get no load result
                    if (fanOp == HVAC::FanOp::Cycling) {
                        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = 0.0;
                    }
                    if (fanOp == HVAC::FanOp::Continuous) {
                        state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0; // The on/off fan will not cycle, so set part-load fraction = 1
                    }

                    //     Set the inlet mass flow rate based on user specified coil OFF flow rate
                    PartLoadRatio = 0.0;
                    SetAverageAirFlow(state, FurnaceNum, PartLoadRatio, OnOffAirFlowRatio);

                    CalcFurnaceOutput(state,
                                      FurnaceNum,
                                      FirstHVACIteration,
                                      fanOp,
                                      compressorOp,
                                      0.0,
                                      MinPLR,
                                      0.0,
                                      0.0,
                                      NoHeatOutput,
                                      NoLatentOutput,
                                      OnOffAirFlowRatio,
                                      false);

                    if (NoHeatOutput < SystemSensibleLoad) {
                        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = furnace.MdotFurnace;

                        // Set fan part-load fraction equal to 1 while getting full load result
                        state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
                        OnOffAirFlowRatio = 1.0;

                        // Get full load result
                        CalcFurnaceOutput(state,
                                          FurnaceNum,
                                          FirstHVACIteration,
                                          fanOp,
                                          compressorOp,
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

                        // check bounds on sensible output prior to iteration using RegulaFalsi
                        if (FullSensibleOutput <= SystemSensibleLoad) {
                            PartLoadRatio = 1.0;
                            //         save modified HeatCoilLoad in case it was reset because outlet temp > DesignMaxOutletTemp
                            if (state.dataFurnaces->ModifiedHeatCoilLoad > 0.0) {
                                HeatCoilLoad = state.dataFurnaces->ModifiedHeatCoilLoad;
                            } else {
                                HeatCoilLoad = furnace.DesignHeatingCapacity;
                            }
                        } else if (NoHeatOutput >= SystemSensibleLoad) {
                            PartLoadRatio = 0.0;
                            HeatCoilLoad = 0.0;
                        } else {

                            // Calculate the part load ratio through iteration
                            HeatErrorToler = furnace.HeatingConvergenceTolerance; // Error tolerance for convergence from input deck

                            int SolFlag = 0; // # of iterations if positive, -1 means failed to converge, -2 means bounds are incorrect
                            // HeatErrorToler is in fraction load, MaxIter = 30, SolFalg = # of iterations or error as appropriate
                            auto f = [&state, FurnaceNum, FirstHVACIteration, fanOp, compressorOp, SystemSensibleLoad](Real64 const PartLoadRatio) {
                                return CalcFurnaceResidual(state,
                                                           PartLoadRatio,
                                                           FurnaceNum,
                                                           FirstHVACIteration,
                                                           fanOp,
                                                           compressorOp,
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
                                HeatCoilLoad = furnace.DesignHeatingCapacity * PartLoadRatio;
                            }
                            if (SolFlag == -1) {

                                //           RegulaFalsi may not find heating PLR when the maximum supply air temperature is exceeded.
                                //           If iteration limit is exceeded, find tighter boundary of solution and repeat RegulaFalsi
                                TempMaxPLR = -0.1;
                                TempHeatOutput = NoHeatOutput;
                                while ((TempHeatOutput - SystemSensibleLoad) < 0.0 && TempMaxPLR < 1.0) {
                                    //             find upper limit of HeatingPLR
                                    TempMaxPLR += 0.1;
                                    HeatCoilLoad = furnace.DesignHeatingCapacity * TempMaxPLR;
                                    CalcFurnaceOutput(state,
                                                      FurnaceNum,
                                                      FirstHVACIteration,
                                                      fanOp,
                                                      compressorOp,
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

                                    HeatCoilLoad = furnace.DesignHeatingCapacity * TempMinPLR;
                                    CalcFurnaceOutput(state,
                                                      FurnaceNum,
                                                      FirstHVACIteration,
                                                      fanOp,
                                                      compressorOp,
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
                                auto f2 = // (AUTO_OK_LAMBDA)
                                    [&state, FurnaceNum, FirstHVACIteration, fanOp, compressorOp, SystemSensibleLoad](Real64 const PartLoadRatio) {
                                        return CalcFurnaceResidual(state,
                                                                   PartLoadRatio,
                                                                   FurnaceNum,
                                                                   FirstHVACIteration,
                                                                   fanOp,
                                                                   compressorOp,
                                                                   SystemSensibleLoad,
                                                                   0.0,  // par6_loadFlag,
                                                                   1.0,  // par7_sensLatentFlag,
                                                                   0.0,  // par9_HXOnFlag,
                                                                   0.0); // par10_HeatingCoilPLR);
                                    };
                                General::SolveRoot(state, HeatErrorToler, MaxIter, SolFlag, PartLoadRatio, f2, TempMinPLR, TempMaxPLR);
                                if (state.dataFurnaces->ModifiedHeatCoilLoad > 0.0) {
                                    HeatCoilLoad = state.dataFurnaces->ModifiedHeatCoilLoad;
                                } else {
                                    HeatCoilLoad = furnace.DesignHeatingCapacity * PartLoadRatio;
                                }
                                CalcFurnaceOutput(state,
                                                  FurnaceNum,
                                                  FirstHVACIteration,
                                                  fanOp,
                                                  compressorOp,
                                                  0.0,
                                                  PartLoadRatio,
                                                  HeatCoilLoad,
                                                  0.0,
                                                  TempHeatOutput,
                                                  TempLatentOutput,
                                                  OnOffAirFlowRatio,
                                                  false);

                                //           After iterating with tighter boundaries, if still out of tolerance, show warning.
                                if (SolFlag == -1 && std::abs(SystemSensibleLoad - TempHeatOutput) > HVAC::SmallLoad) {
                                    if (furnace.HeatingMaxIterIndex == 0) {
                                        ShowWarningMessage(state,
                                                           format("Heating coil control failed to converge for {}:{}",
                                                                  HVAC::unitarySysTypeNames[(int)furnace.type],
                                                                  furnace.Name));
                                        ShowContinueError(state, "  Iteration limit exceeded in calculating heating coil sensible part-load ratio.");
                                        ShowContinueErrorTimeStamp(state,
                                                                   format("Sensible load to be met by heating coil = {:.2T} (watts), sensible output "
                                                                          "of heating coil = {:.2T} (watts), and the simulation continues.",
                                                                          SystemSensibleLoad,
                                                                          TempHeatOutput));
                                    }
                                    ShowRecurringWarningErrorAtEnd(
                                        state,
                                        format("{} \"{}\" - Iteration limit exceeded in calculating sensible heating part-load "
                                               "ratio error continues. Sensible load statistics:",
                                               HVAC::unitarySysTypeNames[(int)furnace.type],
                                               furnace.Name),
                                        furnace.HeatingMaxIterIndex,
                                        SystemSensibleLoad,
                                        SystemSensibleLoad);
                                }
                            } else if (SolFlag == -2) {
                                if (furnace.HeatingRegulaFalsiFailedIndex == 0) {
                                    ShowWarningMessage(state,
                                                       format("Heating coil control failed for {}:{}",
                                                              HVAC::unitarySysTypeNames[(int)furnace.type],
                                                              furnace.Name));
                                    ShowContinueError(state, "  Sensible heating part-load ratio determined to be outside the range of 0-1.");
                                    ShowContinueErrorTimeStamp(
                                        state,
                                        format("Sensible load to be met by heating coil = {:.2T} (watts), and the simulation continues.",
                                               SystemSensibleLoad));
                                }
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    format("{} \"{}\" -  Sensible heating part-load ratio out of range error continues. Sensible load statistics:",
                                           HVAC::unitarySysTypeNames[(int)furnace.type],
                                           furnace.Name),
                                    furnace.HeatingRegulaFalsiFailedIndex,
                                    SystemSensibleLoad,
                                    SystemSensibleLoad);
                            }
                        }

                    } else { // ELSE from IF(FullSensibleOutput.GT.NoSensibleOutput)THEN above
                        // Set part load ratio to 1 and run heater at design heating capacity
                        PartLoadRatio = 1.0;
                        HeatCoilLoad = furnace.DesignHeatingCapacity;
                    }

                } // End of IF HeatPump

            } // End of IF for heating

            // Non-heat pump systems do not set a heating PLR, set it here for use with the DX cooling coil calculations.
            // Set this variable back to 0 for non-heat pump systems at the end of this routine.
            furnace.HeatPartLoadRatio = max(PartLoadRatio, furnace.HeatPartLoadRatio);
            CalcFurnaceOutput(state,
                              FurnaceNum,
                              FirstHVACIteration,
                              fanOp,
                              compressorOp,
                              0.0,
                              furnace.HeatPartLoadRatio,
                              HeatCoilLoad,
                              0.0,
                              HeatingSensibleOutput,
                              HeatingLatentOutput,
                              OnOffAirFlowRatio,
                              false);

            if (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_AirToAir ||
                (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_WaterToAir && furnace.WatertoAirHPType == WAHPCoilType::Simple &&
                 state.dataFurnaces->CoolingLoad)) {
                HeatingSensibleOutput = 0.0;
                HeatingLatentOutput = 0.0;
            }
            //***********Cooling Section*****************
            // Simulate if scheduled ON and cooling load or if a moisture load exists when using a humidistat
            // Check of HeatingLatentOutput is used to reduce overshoot during simultaneous heating and cooling
            // Setback flag is used to avoid continued RH control when Tstat is setback (RH should float down)
            if ((furnace.availSched->getCurrentVal() > 0.0 && state.dataFurnaces->CoolingLoad) ||
                (furnace.Humidistat && furnace.DehumidControlType_Num == DehumidificationControlMode::CoolReheat &&
                 (SystemMoistureLoad < 0.0 || (SystemMoistureLoad >= 0.0 && HeatingLatentOutput > SystemMoistureLoad &&
                                               !state.dataZoneEnergyDemand->Setback(furnace.ControlZoneNum))))) {

                //     For cooling operation, the first step is to set the HX operation flag in case a HX assisted coil is used.
                //      (if a HX assisted coil is not used, this flag is not used. It's only used in the CALL to SimHXAssistedCoolingCoil)
                //     Check the dehumidification control type:
                //           For dehumidification control options CoolReheat and None, the HX is always active (locked ON).
                //           For dehumidification control option Multimode, the system is operated first with the HX off.
                //           If the moisture load is not met, the HX will then be turned on and the system is re-simulated.

                if (furnace.DehumidControlType_Num == DehumidificationControlMode::CoolReheat ||
                    furnace.DehumidControlType_Num == DehumidificationControlMode::None) {
                    HXUnitOn = true;
                } else {
                    HXUnitOn = false;
                }

                //     The next step is to determine the system output at no load (PLR=0) and full load (PLR=1)

                //     Set the inlet mass flow rate based on user specified coil OFF flow rate
                PartLoadRatio = 0.0;

                furnace.CompPartLoadRatio = 0.0; // compressor off

                //     SetAverageAirFlow calculates the operating mass flow rate based on PLR and the user specified inputs
                //     for MaxCoolAirMassFlow and MaxNoCoolHeatAirMassFlow.
                //     Air flow rate is set according to max of cooling and heating PLR if heating and latent load exists.
                if (fanOp == HVAC::FanOp::Cycling && furnace.HeatPartLoadRatio > 0.0 && furnace.Humidistat &&
                    furnace.DehumidControlType_Num == DehumidificationControlMode::CoolReheat &&
                    (SystemMoistureLoad < 0.0 || (SystemMoistureLoad >= 0.0 && HeatingLatentOutput > SystemMoistureLoad &&
                                                  !state.dataZoneEnergyDemand->Setback(furnace.ControlZoneNum)))) {
                    CoolingHeatingPLRRatio = min(1.0, PartLoadRatio / furnace.HeatPartLoadRatio);
                    SetAverageAirFlow(state, FurnaceNum, max(PartLoadRatio, furnace.HeatPartLoadRatio), OnOffAirFlowRatio);

                } else {
                    CoolingHeatingPLRRatio = 1.0;
                    SetAverageAirFlow(state, FurnaceNum, PartLoadRatio, OnOffAirFlowRatio);
                }

                // Get no load result (coils simulated OFF)
                CalcFurnaceOutput(state,
                                  FurnaceNum,
                                  FirstHVACIteration,
                                  fanOp,
                                  compressorOp,
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
                    state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = furnace.MdotFurnace;

                    // Set fan part-load fraction equal to 1 while getting full load result
                    state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
                    OnOffAirFlowRatio = 1.0;
                    PartLoadRatio = 1.0;
                    furnace.CompPartLoadRatio = 1.0; // compressor ON

                    // Get full load result (coils simulated full ON)
                    CalcFurnaceOutput(state,
                                      FurnaceNum,
                                      FirstHVACIteration,
                                      fanOp,
                                      compressorOp,
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

                        // check bounds on sensible output prior to iteration using RegulaFalsi
                        // Negative value represents cooling load, IF FullSensibleOutput .GT. CoolCoilLoad, load is greater than capacity
                        if (FullSensibleOutput >= CoolCoilLoad) {
                            PartLoadRatio = 1.0;
                            //           Likewise IF NoCoolOutput .LT. CoolCoilLoad, then load can be met using only the fan (constant fan mode only)
                        } else if (NoCoolOutput <= CoolCoilLoad) {
                            PartLoadRatio = 0.0;
                            //           ELSE load is between NoCoolOutput and FullSensibleOuput, find PLR required to meet load
                        } else {

                            // Calculate the sensible part load ratio through iteration
                            CoolErrorToler = furnace.CoolingConvergenceTolerance; // Error tolerance for convergence from input deck
                            int SolFlag = 0; // # of iterations if positive, -1 means failed to converge, -2 means bounds are incorrect
                            Real64 par8_HXFlag = HXUnitOn ? 1.0 : 0.0;
                            // CoolErrorToler is in fraction of load, MaxIter = 30, SolFalg = # of iterations or error as appropriate
                            auto f =
                                [&state, FurnaceNum, FirstHVACIteration, fanOp, compressorOp, CoolCoilLoad, par8_HXFlag](Real64 const PartLoadRatio) {
                                    return CalcFurnaceResidual(state,
                                                               PartLoadRatio,
                                                               FurnaceNum,
                                                               FirstHVACIteration,
                                                               fanOp,
                                                               compressorOp,
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
                                                      fanOp,
                                                      compressorOp,
                                                      PartLoadRatio,
                                                      0.0,
                                                      0.0,
                                                      0.0,
                                                      TempCoolOutput,
                                                      TempLatentOutput,
                                                      OnOffAirFlowRatio,
                                                      HXUnitOn);
                                    if (!state.dataGlobal->WarmupFlag) {
                                        if (std::abs(CoolCoilLoad - TempCoolOutput) > HVAC::SmallLoad) {
                                            if (furnace.SensibleMaxIterIndex == 0) {
                                                ShowWarningMessage(state,
                                                                   format("Cooling coil control failed to converge for {}:{}",
                                                                          HVAC::unitarySysTypeNames[(int)furnace.type],
                                                                          furnace.Name));
                                                ShowContinueError(
                                                    state, "  Iteration limit exceeded in calculating DX cooling coil sensible part-load ratio.");
                                                ShowContinueErrorTimeStamp(state,
                                                                           format("Sensible load to be met by DX coil = {:.2T} (watts), sensible "
                                                                                  "output of DX coil = {:.2T} (watts), and the simulation continues.",
                                                                                  CoolCoilLoad,
                                                                                  TempCoolOutput));
                                            }
                                            ShowRecurringWarningErrorAtEnd(
                                                state,
                                                format("{} \"{}\" - Iteration limit exceeded in calculating sensible cooling "
                                                       "part-load ratio error continues. Sensible load statistics:",
                                                       HVAC::unitarySysTypeNames[(int)furnace.type],
                                                       furnace.Name),
                                                furnace.SensibleMaxIterIndex,
                                                CoolCoilLoad,
                                                CoolCoilLoad);
                                        }
                                    }
                                } else if (SolFlag == -2) {
                                    if (!state.dataGlobal->WarmupFlag) {
                                        if (furnace.SensibleRegulaFalsiFailedIndex == 0) {
                                            ShowWarningMessage(state,
                                                               format("Cooling coil control failed for {}:{}",
                                                                      HVAC::unitarySysTypeNames[(int)furnace.type],
                                                                      furnace.Name));
                                            ShowContinueError(state, "  Cooling sensible part-load ratio determined to be outside the range of 0-1.");
                                            ShowContinueErrorTimeStamp(state, format("  Cooling sensible load = {:.2T}", CoolCoilLoad));
                                        }
                                        ShowRecurringWarningErrorAtEnd(
                                            state,
                                            format("{} \"{}\" - Cooling sensible part-load ratio out of range error continues. Sensible cooling load "
                                                   "statistics:",
                                                   HVAC::unitarySysTypeNames[(int)furnace.type],
                                                   furnace.Name),
                                            furnace.SensibleRegulaFalsiFailedIndex,
                                            CoolCoilLoad,
                                            CoolCoilLoad);
                                    }
                                }
                            }
                        }

                    } else {
                        PartLoadRatio = 0.0;
                    } // EndIf for IF(CoolCoilLoad.NE.0.0)

                    //       Calculate the delivered capacity from the PLR calculated above
                    CalcFurnaceOutput(state,
                                      FurnaceNum,
                                      FirstHVACIteration,
                                      fanOp,
                                      compressorOp,
                                      PartLoadRatio,
                                      furnace.HeatPartLoadRatio,
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
                    if (furnace.DehumidControlType_Num == DehumidificationControlMode::Multimode &&
                        ((SystemMoistureLoad < 0.0 && TempLatentOutput < SystemMoistureLoad) || PartLoadRatio == 0.0)) {
                        LatentPartLoadRatio = 0.0;
                        //       ELSE calculate a new PLR for valid dehumidification control types if a moisture load exists.
                    } else if (furnace.DehumidControlType_Num != DehumidificationControlMode::None &&
                               (SystemMoistureLoad < 0.0 || (SystemMoistureLoad >= 0.0 && TempLatentOutput > SystemMoistureLoad &&
                                                             !state.dataZoneEnergyDemand->Setback(furnace.ControlZoneNum)))) {

                        //         IF the furnace uses dehumidification control MultiMode, turn on the HX and calculate the latent output with
                        //         the HX ON to compare to the moisture load predicted by the humidistat.
                        if (furnace.DehumidControlType_Num == DehumidificationControlMode::Multimode) {
                            HXUnitOn = true;
                            state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = furnace.MdotFurnace;
                            // Set fan part-load fraction equal to 1 while getting full load result
                            state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
                            OnOffAirFlowRatio = 1.0;
                            // Get full load result
                            CalcFurnaceOutput(state,
                                              FurnaceNum,
                                              FirstHVACIteration,
                                              fanOp,
                                              compressorOp,
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
                            state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = furnace.MdotFurnace;

                            // Set fan part-load fraction equal to 1 while getting full load result
                            state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
                            OnOffAirFlowRatio = 1.0;
                            furnace.CompPartLoadRatio = 1.0; // compressor ON

                            // Get full load result (coils simulated full ON)
                            CalcFurnaceOutput(state,
                                              FurnaceNum,
                                              FirstHVACIteration,
                                              fanOp,
                                              compressorOp,
                                              1.0,
                                              0.0,
                                              0.0,
                                              0.0,
                                              TempCoolOutput,
                                              TempLatentOutput,
                                              OnOffAirFlowRatio,
                                              HXUnitOn);
                        }

                        // check bounds on latent output prior to iteration using RegulaFalsi
                        if (TempLatentOutput > SystemMoistureLoad ||
                            (furnace.DehumidControlType_Num == DehumidificationControlMode::Multimode && TempCoolOutput > CoolCoilLoad)) {
                            LatentPartLoadRatio = 1.0;
                        } else if (NoLatentOutput < SystemMoistureLoad || HeatingLatentOutput < SystemMoistureLoad) {
                            LatentPartLoadRatio = 0.0;
                        } else {

                            CoolErrorToler = furnace.CoolingConvergenceTolerance; // Error tolerance for convergence

                            int SolFlag = 0; // # of iterations if positive, -1 means failed to converge, -2 means bounds are incorrect
                            // Multimode always controls to meet the SENSIBLE load (however, HXUnitOn is now TRUE)
                            Real64 par4_load;
                            if (furnace.DehumidControlType_Num == DehumidificationControlMode::Multimode) {
                                par4_load = CoolCoilLoad;
                            } else {
                                par4_load = SystemMoistureLoad;
                            }
                            // Multimode always controls to meet the SENSIBLE load (however, HXUnitOn is now TRUE)
                            Real64 par6_LatentSens;
                            if (furnace.DehumidControlType_Num == DehumidificationControlMode::Multimode) {
                                par6_LatentSens = 1.0;
                            } else {
                                par6_LatentSens = 0.0;
                            }
                            Real64 par8_HXUnit = HXUnitOn ? 1.0 : 0.0;
                            Real64 par9_HtgCoilPLR;
                            if (fanOp == HVAC::FanOp::Cycling && furnace.HeatPartLoadRatio > 0.0 && par6_LatentSens == 0.0) {
                                par9_HtgCoilPLR = furnace.HeatPartLoadRatio;
                            } else {
                                par9_HtgCoilPLR = 0.0;
                            }
                            auto f = [&state,
                                      FurnaceNum,
                                      FirstHVACIteration,
                                      fanOp,
                                      compressorOp,
                                      par4_load,
                                      par6_LatentSens,
                                      par8_HXUnit,
                                      par9_HtgCoilPLR](Real64 const PartLoadRatio) {
                                return CalcFurnaceResidual(state,
                                                           PartLoadRatio,
                                                           FurnaceNum,
                                                           FirstHVACIteration,
                                                           fanOp,
                                                           compressorOp,
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
                                    if (par9_HtgCoilPLR > 0.0) {
                                        //                 Par(10) = Furnace(FurnaceNum)%HeatPartLoadRatio
                                        //                 fanOp = CycFan and Furnace(FurnaceNum)%HeatPartLoadRatio must be > 0 for Part(10) to be
                                        //                 greater than 0
                                        CoolingHeatingPLRRatio = min(1.0, TempMaxPLR / furnace.HeatPartLoadRatio);
                                    } else {
                                        CoolingHeatingPLRRatio = 1.0;
                                    }

                                    CalcFurnaceOutput(state,
                                                      FurnaceNum,
                                                      FirstHVACIteration,
                                                      fanOp,
                                                      compressorOp,
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
                                    if (par9_HtgCoilPLR > 0.0) {
                                        //                 Par(10) = Furnace(FurnaceNum)%HeatPartLoadRatio
                                        //                 fanOp = CycFan and Furnace(FurnaceNum)%HeatPartLoadRatio must be > 0 for Part(10) to be
                                        //                 greater than 0 Since the latent output of cycling fan systems is 0 at PLR=0, do not allow
                                        //                 the PLR to be 0, otherwise RegulaFalsi can fail when a heating and moisture load exists and
                                        //                 heating PLR > latent PLR.
                                        TempMinPLR2 = max(0.0000000001, TempMinPLR);
                                        CoolingHeatingPLRRatio = min(1.0, TempMinPLR2 / furnace.HeatPartLoadRatio);
                                    } else {
                                        TempMinPLR2 = TempMinPLR;
                                        CoolingHeatingPLRRatio = 1.0;
                                    }

                                    CalcFurnaceOutput(state,
                                                      FurnaceNum,
                                                      FirstHVACIteration,
                                                      fanOp,
                                                      compressorOp,
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
                                auto f2 = [&state,
                                           FurnaceNum,
                                           FirstHVACIteration,
                                           fanOp,
                                           compressorOp,
                                           par4_load,
                                           par6_LatentSens,
                                           par8_HXUnit,
                                           par9_HtgCoilPLR](Real64 const PartLoadRatio) {
                                    return CalcFurnaceResidual(state,
                                                               PartLoadRatio,
                                                               FurnaceNum,
                                                               FirstHVACIteration,
                                                               fanOp,
                                                               compressorOp,
                                                               par4_load,
                                                               1.0,              // par6_loadFlag,
                                                               par6_LatentSens,  // par7_sensLatentFlag,
                                                               par8_HXUnit,      // par9_HXOnFlag,
                                                               par9_HtgCoilPLR); // par10_HeatingCoilPLR);
                                };
                                General::SolveRoot(state, CoolErrorToler, MaxIter, SolFlag, LatentPartLoadRatio, f2, TempMinPLR2, TempMaxPLR);
                                //             OnOffAirFlowRatio is updated during the above iteration. Reset to correct value based on PLR.
                                OnOffAirFlowRatio = state.dataFurnaces->OnOffAirFlowRatioSave;
                                if (SolFlag == -1) {

                                    //               Set cooling to heating PLR for use with Subroutine CalcFurnaceOutput.
                                    if (par9_HtgCoilPLR > 0.0) {
                                        //                 Par(10) = Furnace(FurnaceNum)%HeatPartLoadRatio
                                        //                 fanOp = CycFan and Furnace(FurnaceNum)%HeatPartLoadRatio must be > 0 for Part(10) to be
                                        //                 greater than 0
                                        CoolingHeatingPLRRatio = min(1.0, LatentPartLoadRatio / furnace.HeatPartLoadRatio);
                                    } else {
                                        CoolingHeatingPLRRatio = 1.0;
                                    }

                                    CalcFurnaceOutput(state,
                                                      FurnaceNum,
                                                      FirstHVACIteration,
                                                      fanOp,
                                                      compressorOp,
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
                                            if (furnace.LatentMaxIterIndex == 0) {
                                                ShowWarningMessage(state,
                                                                   format("Cooling coil control failed to converge for {}:{}",
                                                                          HVAC::unitarySysTypeNames[(int)furnace.type],
                                                                          furnace.Name));
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
                                                format("{} \"{}\" - Iteration limit exceeded in calculating latent part-load ratio error continues. "
                                                       "Latent "
                                                       "load convergence error (percent) statistics follow.",
                                                       HVAC::unitarySysTypeNames[(int)furnace.type],
                                                       furnace.Name),
                                                furnace.LatentMaxIterIndex,
                                                100.0 * std::abs((SystemMoistureLoad - TempLatentOutput) / SystemMoistureLoad),
                                                100.0 * std::abs((SystemMoistureLoad - TempLatentOutput) / SystemMoistureLoad));
                                        }
                                    }
                                } else if (SolFlag == -2) {
                                    if (furnace.LatentRegulaFalsiFailedIndex2 == 0) {
                                        ShowWarningMessage(state,
                                                           format("Cooling coil control failed for {}:{}",
                                                                  HVAC::unitarySysTypeNames[(int)furnace.type],
                                                                  furnace.Name));
                                        ShowContinueError(state,
                                                          format("  Latent part-load ratio determined to be outside the range of {:.3T} to {:.3T}.",
                                                                 TempMinPLR,
                                                                 TempMaxPLR));
                                        ShowContinueErrorTimeStamp(state,
                                                                   format("A PLR of {:.3T} will be used and the simulation continues.", TempMinPLR));
                                    }
                                    ShowRecurringWarningErrorAtEnd(state,
                                                                   format("{} \"{}\" - Cooling sensible part-load ratio out of range error "
                                                                          "continues. System moisture load statistics:",
                                                                          HVAC::unitarySysTypeNames[(int)furnace.type],
                                                                          furnace.Name),
                                                                   furnace.LatentRegulaFalsiFailedIndex2,
                                                                   SystemMoistureLoad,
                                                                   SystemMoistureLoad);
                                    LatentPartLoadRatio = TempMinPLR;
                                }
                            } else if (SolFlag == -2) {
                                if (furnace.LatentRegulaFalsiFailedIndex == 0) {
                                    ShowWarningMessage(state,
                                                       format("Cooling coil control failed for {}:{}",
                                                              HVAC::unitarySysTypeNames[(int)furnace.type],
                                                              furnace.Name));
                                    ShowContinueError(state, "  Latent part-load ratio determined to be outside the range of 0-1.");
                                    ShowContinueErrorTimeStamp(state, "A PLR of 0 will be used and the simulation continues.");
                                }
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    format("{} \"{}\" - Latent part-load ratio out of range or 0-1 error continues. System moisture load statistics:",
                                           HVAC::unitarySysTypeNames[(int)furnace.type],
                                           furnace.Name),
                                    furnace.LatentRegulaFalsiFailedIndex,
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
                                          fanOp,
                                          compressorOp,
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
                    if (LatentPartLoadRatio > PartLoadRatio && furnace.Humidistat) {
                        // For dehumidification mode CoolReheat, compare the Sensible and Latent PLR values, if latentPLR is greater
                        // than PLR (sensible), then overcooling is required and reheat will be activated using the HumControl flag.
                        if (furnace.DehumidControlType_Num == DehumidificationControlMode::CoolReheat) {
                            PartLoadRatio = LatentPartLoadRatio;
                            HumControl = true;
                        }
                        // For dehumidification mode MultiMode, compare the Sensible and Latent PLR values, if latentPLR is
                        // greater than PLR (sensible), then use the latent PLR to control the unit.
                        // For MultiMode control, the latent PLR is found by enabling the HX and calculating a PLR required to meet the
                        // sensible load. Overcooling is not required, and reheat will not be activated using the HumControl flag.
                        if (furnace.DehumidControlType_Num == DehumidificationControlMode::Multimode) {
                            PartLoadRatio = LatentPartLoadRatio;
                        }
                    }

                    furnace.CoolPartLoadRatio = PartLoadRatio;
                    if (compressorOp == HVAC::CompressorOp::Off) {
                        furnace.CompPartLoadRatio = 0.0;
                    } else {
                        furnace.CompPartLoadRatio = PartLoadRatio;
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
                    if (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_WaterToAir) {
                        furnace.CoolPartLoadRatio = 0.0;
                        furnace.CompPartLoadRatio = 0.0;
                    } else {
                        if (furnace.coolCoilType == HVAC::CoilType::CoolingDXHXAssisted) {

                            // VS coil issue here...
                            if (state.dataDXCoils->DXCoilPartLoadRatio(furnace.ActualDXCoilIndexForHXAssisted) > 0.0) {
                                furnace.CoolPartLoadRatio = 1.0;
                                furnace.CompPartLoadRatio = 1.0;
                            } else {
                                furnace.CoolPartLoadRatio = 0.0;
                                furnace.CompPartLoadRatio = 0.0;
                            }
                        } else {
                            if (state.dataDXCoils->DXCoilPartLoadRatio(furnace.CoolCoilNum) > 0.0) {
                                furnace.CoolPartLoadRatio = 1.0;
                                furnace.CompPartLoadRatio = 1.0;
                            } else {
                                furnace.CoolPartLoadRatio = 0.0;
                                furnace.CompPartLoadRatio = 0.0;
                            }
                        }
                    }
                }

                //     Calculate the reheat coil output
                if (HumControl) { // HumControl = .TRUE. if a Humidistat is installed and dehumidification control type is CoolReheat
                    if (furnace.ZoneSequenceHeatingNum > 0) {
                        QToHeatSetPt = (state.dataZoneEnergyDemand->ZoneSysEnergyDemand(furnace.ControlZoneNum)
                                            .SequencedOutputRequiredToHeatingSP(furnace.ZoneSequenceHeatingNum) /
                                        furnace.ControlZoneMassFlowFrac);
                    } else {
                        QToHeatSetPt = (state.dataZoneEnergyDemand->ZoneSysEnergyDemand(furnace.ControlZoneNum).OutputRequiredToHeatingSP /
                                        furnace.ControlZoneMassFlowFrac);
                    }
                    //       Cooling mode or floating condition and dehumidification is required
                    if (QToHeatSetPt < 0.0) {
                        //         Calculate the reheat coil load wrt the heating setpoint temperature. Reheat coil picks up
                        //         the entire excess sensible cooling (DX cooling coil and impact of outdoor air).
                        ReheatCoilLoad = max(0.0, (QToHeatSetPt - ActualSensibleOutput));
                        furnace.DehumidInducedHeatingDemandRate = ReheatCoilLoad;
                        //       Heating mode and dehumidification is required
                    } else {
                        //         Calculate the reheat coil load as the sensible capacity of the DX cooling coil only. Let
                        //         the heating coil pick up the load due to outdoor air.
                        ReheatCoilLoad = max(0.0, (ActualSensibleOutput - NoCoolOutput) * (-1.0));
                        //         Dehumidification is not required
                        if (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_AirToAir ||
                            (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_WaterToAir &&
                             furnace.WatertoAirHPType == WAHPCoilType::Simple)) {
                            ReheatCoilLoad = max(QToHeatSetPt, QToHeatSetPt - ActualSensibleOutput);
                        }
                        furnace.DehumidInducedHeatingDemandRate = max(0.0, ActualSensibleOutput * (-1.0));
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
                if (fanOp == HVAC::FanOp::Cycling) {
                    //       set the flow rate at the maximum of the cooling and heating PLR's
                    SetAverageAirFlow(state, FurnaceNum, max(furnace.CoolPartLoadRatio, furnace.HeatPartLoadRatio), OnOffAirFlowRatio);
                } else {
                    //       ELSE set the flow rate at the cooling PLR
                    SetAverageAirFlow(state, FurnaceNum, furnace.CoolPartLoadRatio, OnOffAirFlowRatio);
                }
            } else {
                SetAverageAirFlow(state, FurnaceNum, max(furnace.CoolPartLoadRatio, furnace.HeatPartLoadRatio), OnOffAirFlowRatio);
            }
            furnace.MdotFurnace = state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate;

            if (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_AirToAir ||
                (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_WaterToAir && furnace.WatertoAirHPType == WAHPCoilType::Simple)) {
            } else {
                // Non-HeatPump (non-DX) heating coils do not set PLR, reset to 0 here. This variable was set for non-DX
                // coils to allow the SetAverageAirFlow CALL above to set the correct air mass flow rate. See this
                // IF block above in heating section. HeatPLR is not set in the ELSE part of the IF (only HeatCoilLoad is set).
                furnace.HeatPartLoadRatio = 0.0;
            }

            //*********HVAC Scheduled OFF*************
            // No heating or cooling or dehumidification
            //!!LKL discrepancy with < 0?
            if (furnace.availSched->getCurrentVal() == 0.0 || state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate == 0.0) {
                furnace.MdotFurnace = 0.0;
                CoolCoilLoad = 0.0;
                HeatCoilLoad = 0.0;
                ReheatCoilLoad = 0.0;
                state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0; // System off, so set on/off fan part-load fraction = 1
                furnace.CoolPartLoadRatio = 0.0;
                furnace.HeatPartLoadRatio = 0.0;
                furnace.CompPartLoadRatio = 0.0;
                // set report variables
                furnace.CoolingCoilSensDemand = 0.0;
                furnace.CoolingCoilLatentDemand = 0.0;
                furnace.HeatingCoilSensDemand = 0.0;
            }

        } // End of the FirstHVACIteration control of the mass flow If block

        // Set the fan inlet node flow rates
        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRateMaxAvail = furnace.MdotFurnace;
        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = furnace.MdotFurnace;
    }

    void CalcWaterToAirHeatPump(EnergyPlusData &state,
                                int const FurnaceNum,                  // index to Furnace
                                bool const FirstHVACIteration,         // TRUE on first HVAC iteration
                                HVAC::CompressorOp const compressorOp, // compressor operation flag (1=On, 0=Off)
                                Real64 const ZoneLoad,                 // the control zone load (watts)
                                Real64 const MoistureLoad              // the control zone latent load (watts)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Dan Fisher
        //       DATE WRITTEN   Feb 2004
        //       MODIFIED       R. Raustad (Oct 2006) Revised iteration technique

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine manages the heat pump simulation

        // METHODOLOGY EMPLOYED:
        // Calculate the part-load ratio required to meet the zone sensible load.

        // SUBROUTINE PARAMETER DEFINITIONS:
        int constexpr MaxIter(600);   // maximum number of iterations
        Real64 constexpr MinPLR(0.0); // minimum part load ratio allowed

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 ZoneSensLoadMet;             // Actual zone sensible load met by heat pump (W)
        Real64 ZoneLatLoadMet;              // Actual zone latent load met by heat pump (W)
        Real64 ZoneSensLoadMetFanONCompON;  // Max Zone sensible load heat pump can meet (W)
        Real64 ZoneLatLoadMetFanONCompON;   // Max Zone latent load heat pump can meet (W)
        Real64 ZoneSensLoadMetFanONCompOFF; // control zone sensible load met using only outside air
        // and fan heat (no coil output) (W)
        Real64 ZoneLatLoadMetFanONCompOFF; // control zone Latent   load met using only outside air
        // and fan heat (no coil output) (W)
        Real64 HPCoilSensDemand;   // Heat pump sensible demand
        Real64 HPCoilSensCapacity; // Heat pump sensible capacity

        Real64 SuppHeatCoilLoad; // Load passed to supplemental heater (W)
        Real64 CoolErrorToler;   // convergence tolerance used in cooling mode
        Real64 HeatErrorToler;   // convergence tolerance used in heating mode
        int SolFlag;             // flag returned from iteration routine to denote problems

        Real64 &TotalZoneLatentLoad = state.dataFurnaces->TotalZoneLatentLoad;
        Real64 &TotalZoneSensLoad = state.dataFurnaces->TotalZoneSensLoad;
        Real64 &CoolPartLoadRatio = state.dataFurnaces->CoolPartLoadRatio;
        Real64 &HeatPartLoadRatio = state.dataFurnaces->HeatPartLoadRatio;
        auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);

        // Set local variables
        Real64 Dummy2 = 0.0;            // used as dummy heat and reheat coil load
        Real64 OnOffAirFlowRatio = 1.0; // Ratio of compressor ON air mass flow to AVERAGE air mass flow over time step
        int FurnaceInletNode = furnace.FurnaceInletNode;
        HVAC::FanOp fanOp = furnace.fanOp; // fan operating mode
        furnace.MdotFurnace = furnace.DesignMassFlowRate;

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
        if (FirstHVACIteration) {
            state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
        }

        // Calc Zone sensible loads for heating (+) and cooling (-)
        TotalZoneSensLoad = ZoneLoad;

        if (state.dataFurnaces->HeatingLoad) {
            TotalZoneLatentLoad = 0.0; // Set latent load for heating
        } else {
            TotalZoneLatentLoad = MoistureLoad; // Set latent load for cooling and no sensible load condition
        }

        //*********COOLING CALCULATIONS****************
        // IF scheduled on...
        // AND air flow rate is greater than zero...
        // AND the air system has a cooling load and is not set back or in the deadband...
        // OR the system is controlled by a humidistat and there is a latent load
        if ((furnace.availSched->getCurrentVal() > 0.0 && state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate > 0.0) &&
            ((state.dataFurnaces->CoolingLoad) || (furnace.Humidistat && furnace.CoolingCoilLatentDemand < 0.0))) {

            // Set the air flow rate to the design flow rate and set the fan operation fraction to 1 (continuous operation)
            state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = furnace.DesignMassFlowRate;
            state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0; // see 'Note' under INITIAL CALCULATIONS

            //         !Set the operation flag to run the fan continuously
            //         fanOp = FanOp::Continuous

            // Set the input parameters for CalcFurnaceOutput
            furnace.HeatingCoilSensDemand = 0.0;
            furnace.CoolingCoilLatentDemand = 0.0;
            furnace.CoolingCoilSensDemand = 0.0;
            furnace.CompPartLoadRatio = 0.0; // compressor off
            furnace.InitHeatPump = true;     // initialization call to Calc Furnace
            CoolPartLoadRatio = 0.0;

            // Get no load result in order to calculate the effect of the fan and the mixed air equipment
            CalcFurnaceOutput(state,
                              FurnaceNum,
                              FirstHVACIteration,
                              fanOp,
                              compressorOp,
                              CoolPartLoadRatio,
                              HeatPartLoadRatio,
                              Dummy2,
                              Dummy2,
                              ZoneSensLoadMetFanONCompOFF,
                              ZoneLatLoadMetFanONCompOFF,
                              OnOffAirFlowRatio,
                              false);

            // Set the input parameters for CalcFurnaceOutput
            furnace.CoolingCoilSensDemand = 1.0;
            furnace.CompPartLoadRatio = 1.0; // compressor ON
            CoolPartLoadRatio = 1.0;

            // Get full load result in order to estimate the operating part load ratio for continuous fan operation
            CalcFurnaceOutput(state,
                              FurnaceNum,
                              FirstHVACIteration,
                              fanOp,
                              compressorOp,
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

            furnace.InitHeatPump = false;

            // check bounds on sensible output prior to iteration using RegulaFalsi
            if (ZoneSensLoadMetFanONCompON > TotalZoneSensLoad) {
                CoolPartLoadRatio = 1.0;
                HPCoilSensDemand = std::abs(ZoneSensLoadMetFanONCompON - ZoneSensLoadMetFanONCompOFF);
                furnace.CoolingCoilSensDemand = HPCoilSensDemand;
            } else if (ZoneSensLoadMetFanONCompOFF < TotalZoneSensLoad) {
                CoolPartLoadRatio = 0.0;
                furnace.CompPartLoadRatio = 0.0; // compressor OFF
                furnace.CoolingCoilSensDemand = 0.0;
                CalcFurnaceOutput(state,
                                  FurnaceNum,
                                  FirstHVACIteration,
                                  fanOp,
                                  compressorOp,
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
                CoolErrorToler = furnace.CoolingConvergenceTolerance;
                SolFlag = 0; // # of iterations if positive, -1 means failed to converge, -2 means bounds are incorrect
                // CoolErrorToler is in fraction of load, MaxIter = 600, SolFalg = # of iterations or error as appropriate
                auto f = [&state, FurnaceNum, FirstHVACIteration, fanOp, compressorOp, TotalZoneSensLoad, ZoneSensLoadMetFanONCompOFF](
                             Real64 const PartLoadRatio) {
                    return CalcWaterToAirResidual(state,
                                                  PartLoadRatio,
                                                  FurnaceNum,
                                                  FirstHVACIteration,
                                                  fanOp,
                                                  compressorOp,
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
                                      fanOp,
                                      compressorOp,
                                      CoolPartLoadRatio,
                                      0.0,
                                      0.0,
                                      0.0,
                                      ZoneSensLoadMet,
                                      ZoneLatLoadMet,
                                      OnOffAirFlowRatio,
                                      false);
                    if (std::abs(ZoneSensLoadMet - TotalZoneSensLoad) / TotalZoneSensLoad > CoolErrorToler) {
                        if (furnace.SensibleMaxIterIndex == 0) {
                            ShowWarningMessage(state,
                                               format("Cooling coil control failed to converge for {}:{}",
                                                      HVAC::unitarySysTypeNames[(int)furnace.type],
                                                      furnace.Name));
                            ShowContinueError(state, "  Iteration limit exceeded in calculating DX cooling coil sensible part-load ratio.");
                            ShowContinueErrorTimeStamp(state,
                                                       format("Sensible load to be met by DX coil = {:.2T} (watts), sensible output of DX coil = "
                                                              "{:.2T} (watts), and the simulation continues.",
                                                              TotalZoneSensLoad,
                                                              ZoneSensLoadMet));
                        }
                        ShowRecurringWarningErrorAtEnd(
                            state,
                            format("{} \"{}\" - Iteration limit exceeded in calculating sensible cooling part-load ratio error "
                                   "continues. Sensible load statistics:",
                                   HVAC::unitarySysTypeNames[(int)furnace.type],
                                   furnace.Name),
                            furnace.SensibleMaxIterIndex,
                            TotalZoneSensLoad,
                            TotalZoneSensLoad);
                    }
                } else if (SolFlag == -2 && !state.dataGlobal->WarmupFlag && !FirstHVACIteration) {
                    CoolPartLoadRatio = max(MinPLR, min(1.0, std::abs(HPCoilSensDemand) / std::abs(HPCoilSensCapacity)));
                    state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
                    CalcFurnaceOutput(state,
                                      FurnaceNum,
                                      FirstHVACIteration,
                                      fanOp,
                                      compressorOp,
                                      CoolPartLoadRatio,
                                      0.0,
                                      0.0,
                                      0.0,
                                      ZoneSensLoadMet,
                                      ZoneLatLoadMet,
                                      OnOffAirFlowRatio,
                                      false);
                    if ((ZoneSensLoadMet - TotalZoneSensLoad) / TotalZoneSensLoad > CoolErrorToler) {
                        if (furnace.SensibleRegulaFalsiFailedIndex == 0) {
                            ShowWarningMessage(
                                state,
                                format("Cooling coil control failed for {}:{}", HVAC::unitarySysTypeNames[(int)furnace.type], furnace.Name));
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
                            format("{} \"{}\" - Cooling sensible part-load ratio out of range error continues. Sensible cooling load statistics:",
                                   HVAC::unitarySysTypeNames[(int)furnace.type],
                                   furnace.Name),
                            furnace.SensibleRegulaFalsiFailedIndex,
                            TotalZoneSensLoad,
                            TotalZoneSensLoad);
                    }
                }
            }

            if (fanOp == HVAC::FanOp::Cycling) {
                furnace.MdotFurnace *= CoolPartLoadRatio;
            }

            //*********HEATING CALCULATIONS****************
            // If Furnace runs with a heating load then set HeatCoilLoad on Heating Coil and the Mass Flow
        } else if ((furnace.availSched->getCurrentVal() > 0.0) && (state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate > 0.0) &&
                   state.dataFurnaces->HeatingLoad) {

            // Set the air flow rate to the design flow rate and set the fan operation fraction to 1 (continuous operation)
            state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = furnace.DesignMassFlowRate;
            state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0; // see 'Note' under INITIAL CALCULATIONS

            //         !Set the operation flag to run the fan continuously
            //         fanOp = FanOp::Continuous

            // Set the input parameters for CalcFurnaceOutput
            furnace.HeatingCoilSensDemand = 0.0;
            furnace.CoolingCoilLatentDemand = 0.0;
            furnace.CoolingCoilSensDemand = 0.0;
            furnace.CompPartLoadRatio = 0.0; // compressor off
            furnace.InitHeatPump = true;     // initialization call to Calc Furnace
            HeatPartLoadRatio = 0.0;

            // Get no load result in order to calculate the effect of the fan and the mixed air equipment
            CalcFurnaceOutput(state,
                              FurnaceNum,
                              FirstHVACIteration,
                              fanOp,
                              compressorOp,
                              CoolPartLoadRatio,
                              HeatPartLoadRatio,
                              Dummy2,
                              Dummy2,
                              ZoneSensLoadMetFanONCompOFF,
                              ZoneLatLoadMetFanONCompOFF,
                              OnOffAirFlowRatio,
                              false);

            // Set the input parameters for CalcFurnaceOutput
            furnace.HeatingCoilSensDemand = 1.0;
            furnace.CompPartLoadRatio = 1.0; // compressor ON
            HeatPartLoadRatio = 1.0;

            // Get full load result in order to estimate the operating part load ratio for continuous fan operation

            CalcFurnaceOutput(state,
                              FurnaceNum,
                              FirstHVACIteration,
                              fanOp,
                              compressorOp,
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

            furnace.InitHeatPump = false;

            // check bounds on sensible output prior to iteration using RegulaFalsi
            if (ZoneSensLoadMetFanONCompON < TotalZoneSensLoad) {
                HeatPartLoadRatio = 1.0;
                ZoneSensLoadMet = ZoneSensLoadMetFanONCompON;
                HPCoilSensDemand = std::abs(ZoneSensLoadMetFanONCompON - ZoneSensLoadMetFanONCompOFF);
                furnace.HeatingCoilSensDemand = HPCoilSensDemand;
            } else if (ZoneSensLoadMetFanONCompOFF > TotalZoneSensLoad) {
                HeatPartLoadRatio = 0.0;
                ZoneSensLoadMet = ZoneSensLoadMetFanONCompOFF;
                furnace.CompPartLoadRatio = 0.0; // compressor ON
                CalcFurnaceOutput(state,
                                  FurnaceNum,
                                  FirstHVACIteration,
                                  fanOp,
                                  compressorOp,
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
                HeatErrorToler = furnace.HeatingConvergenceTolerance;
                SolFlag = 0; // # of iterations if positive, -1 means failed to converge, -2 means bounds are incorrect
                // HeatErrorToler is in fraction of load, MaxIter = 600, SolFalg = # of iterations or error as appropriate
                auto f = [&state, FurnaceNum, FirstHVACIteration, fanOp, compressorOp, TotalZoneSensLoad, ZoneSensLoadMetFanONCompOFF](
                             Real64 const PartLoadRatio) {
                    return CalcWaterToAirResidual(state,
                                                  PartLoadRatio,
                                                  FurnaceNum,
                                                  FirstHVACIteration,
                                                  fanOp,
                                                  compressorOp,
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
                                  fanOp,
                                  compressorOp,
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
                        if (furnace.WSHPHeatMaxIterIndex == 0) {
                            ShowWarningMessage(state,
                                               format("Heating coil control failed to converge for {}:{}",
                                                      HVAC::unitarySysTypeNames[(int)furnace.type],
                                                      furnace.Name));
                            ShowContinueError(state, "  Iteration limit exceeded in calculating DX heating coil sensible part-load ratio.");
                            ShowContinueErrorTimeStamp(state,
                                                       format("Sensible load to be met by DX coil = {:.2T} (watts), sensible output of DX coil = "
                                                              "{:.2T} (watts), and the simulation continues.",
                                                              TotalZoneSensLoad,
                                                              ZoneSensLoadMet));
                        }
                        ShowRecurringWarningErrorAtEnd(
                            state,
                            format("{} \"{}\" - Iteration limit exceeded in calculating sensible heating part-load ratio error continues.",
                                   HVAC::unitarySysTypeNames[(int)furnace.type],
                                   furnace.Name),
                            furnace.WSHPHeatMaxIterIndex,
                            TotalZoneSensLoad,
                            TotalZoneSensLoad);
                    }
                } else if (SolFlag == -2) {
                    HeatPartLoadRatio = max(MinPLR, min(1.0, std::abs(HPCoilSensDemand) / std::abs(HPCoilSensCapacity)));
                    CalcFurnaceOutput(state,
                                      FurnaceNum,
                                      FirstHVACIteration,
                                      fanOp,
                                      compressorOp,
                                      0.0,
                                      HeatPartLoadRatio,
                                      0.0,
                                      0.0,
                                      ZoneSensLoadMet,
                                      ZoneLatLoadMet,
                                      OnOffAirFlowRatio,
                                      false);
                    if ((ZoneSensLoadMet - TotalZoneSensLoad) / TotalZoneSensLoad > HeatErrorToler) {
                        if (furnace.WSHPHeatRegulaFalsiFailedIndex == 0) {
                            ShowWarningError(
                                state,
                                format("Heating coil control failed for {}:{}", HVAC::unitarySysTypeNames[(int)furnace.type], furnace.Name));
                            ShowContinueError(state, "  Heating sensible part-load ratio determined to be outside the range of 0-1.");
                            ShowContinueError(
                                state,
                                format("  An estimated part-load ratio = {:.2T} will be used and the simulation continues.", HeatPartLoadRatio));
                            ShowContinueError(
                                state, format("  The estimated part-load ratio provides a heating sensible capacity = {:.2T}", ZoneSensLoadMet));
                            ShowContinueErrorTimeStamp(state, format("  Heating sensible load required = {:.2T}", TotalZoneSensLoad));
                        }
                        ShowRecurringWarningErrorAtEnd(state,
                                                       format("{} \"{}\" - Heating sensible part-load ratio out of range error continues.",
                                                              HVAC::unitarySysTypeNames[(int)furnace.type],
                                                              furnace.Name),
                                                       furnace.WSHPHeatRegulaFalsiFailedIndex,
                                                       TotalZoneSensLoad,
                                                       TotalZoneSensLoad);
                    }
                }
            }

            //       CALL supplemental heater if required
            if ((TotalZoneSensLoad - ZoneSensLoadMet) > HVAC::SmallLoad && HeatPartLoadRatio >= 1.0) {
                SuppHeatCoilLoad = TotalZoneSensLoad - ZoneSensLoadMet;
                CalcFurnaceOutput(state,
                                  FurnaceNum,
                                  FirstHVACIteration,
                                  fanOp,
                                  compressorOp,
                                  CoolPartLoadRatio,
                                  HeatPartLoadRatio,
                                  SuppHeatCoilLoad,
                                  Dummy2,
                                  ZoneSensLoadMet,
                                  ZoneLatLoadMet,
                                  OnOffAirFlowRatio,
                                  false);
            }

            if (fanOp == HVAC::FanOp::Cycling) {
                furnace.MdotFurnace *= HeatPartLoadRatio;
            }

            //**********HVAC Scheduled ON, but no cooling, dehumidification or heating load*********
        } else if (furnace.availSched->getCurrentVal() > 0.0) {
            furnace.InitHeatPump = true; // initialization call to Calc Furnace
            HeatPartLoadRatio = 0.0;
            CoolPartLoadRatio = 0.0;
            state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0; //! see 'Note' under INITIAL CALCULATIONS
            // set report variables
            furnace.CompPartLoadRatio = 0.0;
            furnace.CoolingCoilSensDemand = 0.0;
            furnace.CoolingCoilLatentDemand = 0.0;
            furnace.HeatingCoilSensDemand = 0.0;
            if (fanOp == HVAC::FanOp::Cycling) {
                furnace.MdotFurnace = 0.0;
                state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0; // see 'Note' under INITIAL CALCULATIONS
                CalcFurnaceOutput(state,
                                  FurnaceNum,
                                  FirstHVACIteration,
                                  fanOp,
                                  compressorOp,
                                  CoolPartLoadRatio,
                                  HeatPartLoadRatio,
                                  Dummy2,
                                  Dummy2,
                                  ZoneSensLoadMet,
                                  ZoneLatLoadMet,
                                  OnOffAirFlowRatio,
                                  false);
                furnace.MdotFurnace = 0.0;
            } else { // continuous fan, cycling coil
                CalcFurnaceOutput(state,
                                  FurnaceNum,
                                  FirstHVACIteration,
                                  fanOp,
                                  compressorOp,
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
            furnace.InitHeatPump = true; // initialization call to Calc Furnace
            furnace.MdotFurnace = 0.0;
            HeatPartLoadRatio = 0.0;
            CoolPartLoadRatio = 0.0;
            state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0; // see 'Note' under INITIAL CALCULATIONS
            furnace.CompPartLoadRatio = 0.0;
            furnace.CoolingCoilSensDemand = 0.0;
            furnace.CoolingCoilLatentDemand = 0.0;
            furnace.HeatingCoilSensDemand = 0.0;
            CalcFurnaceOutput(state,
                              FurnaceNum,
                              FirstHVACIteration,
                              fanOp,
                              compressorOp,
                              CoolPartLoadRatio,
                              HeatPartLoadRatio,
                              Dummy2,
                              Dummy2,
                              ZoneSensLoadMet,
                              ZoneLatLoadMet,
                              OnOffAirFlowRatio,
                              false);
            furnace.MdotFurnace = 0.0;
        }

        // Set the fan inlet node flow rates
        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRateMaxAvail = furnace.MdotFurnace;
        state.dataLoopNodes->Node(FurnaceInletNode).MassFlowRate = furnace.MdotFurnace;
    }

    void CalcFurnaceOutput(EnergyPlusData &state,
                           int const FurnaceNum,
                           bool const FirstHVACIteration,
                           HVAC::FanOp const fanOp,               // Cycling fan or constant fan
                           HVAC::CompressorOp const compressorOp, // Compressor on/off; 1=on, 0=off
                           Real64 const CoolPartLoadRatio,        // DX cooling coil part load ratio
                           Real64 const HeatPartLoadRatio,        // DX heating coil part load ratio (0 for other heating coil types)
                           Real64 const HeatCoilLoad,             // Heating coil load for gas heater
                           Real64 const ReheatCoilLoad,           // Reheating coil load for gas heater
                           Real64 &SensibleLoadMet,               // Sensible cooling load met (furnace outlet with respect to control zone temp)
                           Real64 &LatentLoadMet,              // Latent cooling load met (furnace outlet with respect to control zone humidity ratio)
                           Real64 &OnOffAirFlowRatio,          // Ratio of compressor ON mass flow rate to AVERAGE
                           bool const HXUnitOn,                // flag to enable HX based on zone moisture load
                           Real64 const CoolingHeatingPLRRatio // cooling PLR to heating PLR ratio, used for cycling fan RH control
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   Sept 2001
        //       MODIFIED       Dec 2001

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine calculates to sensible and latent loads met by the DX coils
        // specified.  Load met is the outlet node with respect to the control zone's
        // temperature and humidity ratio.

        // METHODOLOGY EMPLOYED:
        // Simulate each child object in the correct order for each system type. This routine is used in the
        // RegulaFalsi function CALL. Air mass flow rate is set each iteration based on PLR.

        auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);
        auto &inletNode = state.dataLoopNodes->Node(furnace.FurnaceInletNode);
        Real64 QActual = 0.0; // heating coil load met or delivered
        state.dataFurnaces->ModifiedHeatCoilLoad = 0.0;

        state.dataFurnaces->CoolHeatPLRRat = CoolingHeatingPLRRatio;

        // Cooling to Heating PLR Ratio (CoolHeatPLRRat) is used to track the air mass flow rate of both the heating
        // and cooling coils when RH control is used and the heating coil operates longer than the cooling coil.
        // When CoolPartLoadRatio/CoolHeatPLRRat is used, the PLR calculated is actually the PLR for the heating
        // coil (heating PLR is greater than cooling PLR), it is this PLR that determines the air mass flow rate.
        // When MAX(HeatPartLoadRatio,CoolPartLoadRatio) is used, only one of these values is non-zero.
        if (fanOp == HVAC::FanOp::Cycling) {
            if (state.dataFurnaces->CoolHeatPLRRat < 1.0) {
                if (state.dataFurnaces->CoolHeatPLRRat > 0.0) {
                    inletNode.MassFlowRate = state.dataFurnaces->CompOnMassFlow * CoolPartLoadRatio / state.dataFurnaces->CoolHeatPLRRat;
                    if (furnace.type != HVAC::UnitarySysType::Unitary_HeatPump_WaterToAir) {
                        SetAverageAirFlow(state, FurnaceNum, CoolPartLoadRatio / state.dataFurnaces->CoolHeatPLRRat, OnOffAirFlowRatio);
                    }
                } else {
                    inletNode.MassFlowRate = state.dataFurnaces->CompOnMassFlow * CoolPartLoadRatio;
                    if (furnace.type != HVAC::UnitarySysType::Unitary_HeatPump_WaterToAir) {
                        SetAverageAirFlow(state, FurnaceNum, max(HeatPartLoadRatio, CoolPartLoadRatio), OnOffAirFlowRatio);
                    }
                }
            } else {
                inletNode.MassFlowRate = state.dataFurnaces->CompOnMassFlow * max(HeatPartLoadRatio, CoolPartLoadRatio);
                if (furnace.type != HVAC::UnitarySysType::Unitary_HeatPump_WaterToAir) {
                    SetAverageAirFlow(state, FurnaceNum, max(HeatPartLoadRatio, CoolPartLoadRatio), OnOffAirFlowRatio);
                }
            }
        } else {
            if (furnace.type != HVAC::UnitarySysType::Unitary_HeatPump_WaterToAir) {
                SetAverageAirFlow(state, FurnaceNum, max(HeatPartLoadRatio, CoolPartLoadRatio), OnOffAirFlowRatio);
            }
        }

        inletNode.MassFlowRateMaxAvail = inletNode.MassFlowRate;

        // Simulate the air-to-air heat pump
        if (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_AirToAir) {
            //   Simulate blow-thru fan and non-linear coils twice to update PLF used by the ONOFF Fan
            if (furnace.fanPlace == HVAC::FanPlace::BlowThru) {
                state.dataFans->fans(furnace.FanIndex)->simulate(state, FirstHVACIteration, state.dataFurnaces->FanSpeedRatio);
                if (furnace.coolCoilType == HVAC::CoilType::CoolingDXHXAssisted) {
                    HXAssistCoil::SimHXAssistedCoolingCoil(state,
                                                                        BlankString,
                                                                        FirstHVACIteration,
                                                                        compressorOp,
                                                                        CoolPartLoadRatio,
                                                                        furnace.CoolCoilNum,
                                                                        fanOp,
                                                                        HXUnitOn,
                                                                        OnOffAirFlowRatio,
                                                                        state.dataFurnaces->EconomizerFlag);
                } else {
                    DXCoils::SimDXCoil(state,
                                       BlankString,
                                       compressorOp,
                                       FirstHVACIteration,
                                       furnace.CoolCoilNum,
                                       fanOp,
                                       CoolPartLoadRatio,
                                       OnOffAirFlowRatio);
                }
                DXCoils::SimDXCoil(
                    state, BlankString, compressorOp, FirstHVACIteration, furnace.HeatCoilNum, fanOp, HeatPartLoadRatio, OnOffAirFlowRatio);
                state.dataFans->fans(furnace.FanIndex)->simulate(state, FirstHVACIteration, state.dataFurnaces->FanSpeedRatio);
            }
            //   Simulate cooling and heating coils
            if (furnace.coolCoilType == HVAC::CoilType::CoolingDXHXAssisted) {
                HXAssistCoil::SimHXAssistedCoolingCoil(state,
                                                                    BlankString,
                                                                    FirstHVACIteration,
                                                                    compressorOp,
                                                                    CoolPartLoadRatio,
                                                                    furnace.CoolCoilNum,
                                                                    fanOp,
                                                                    HXUnitOn,
                                                                    OnOffAirFlowRatio,
                                                                    state.dataFurnaces->EconomizerFlag);
            } else {
                DXCoils::SimDXCoil(
                    state, BlankString, compressorOp, FirstHVACIteration, furnace.CoolCoilNum, fanOp, CoolPartLoadRatio, OnOffAirFlowRatio);
            }
            DXCoils::SimDXCoil(
                state, BlankString, compressorOp, FirstHVACIteration, furnace.HeatCoilNum, fanOp, HeatPartLoadRatio, OnOffAirFlowRatio);
            //   Simulate the draw-thru fan
            if (furnace.fanPlace == HVAC::FanPlace::DrawThru) {
                state.dataFans->fans(furnace.FanIndex)->simulate(state, FirstHVACIteration, state.dataFurnaces->FanSpeedRatio);
            }
            //   Simulate the supplemental heating coil
            if (furnace.DehumidControlType_Num == DehumidificationControlMode::CoolReheat && ReheatCoilLoad > 0.0) {
                bool SuppHeatingCoilFlag = true;
                CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, ReheatCoilLoad, fanOp, QActual);
            } else {
                // equivalent to QCoilReq=0.0d0 or ReHeatCoilLoad = 0.0d0
                bool SuppHeatingCoilFlag = true;
                CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, ReheatCoilLoad, fanOp, QActual);
            }
            // Simulate the parameter estimate water-to-air heat pump
        } else if (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_WaterToAir && furnace.WatertoAirHPType == WAHPCoilType::Simple) {
            //    Simulate blow-thru fan and non-linear coils twice to update PLF used by the ONOFF Fan
            if (furnace.fanPlace == HVAC::FanPlace::BlowThru) {
                state.dataFans->fans(furnace.FanIndex)->simulate(state, FirstHVACIteration, state.dataFurnaces->FanSpeedRatio);
                // COIL:WATERTOAIRHPSIMPLE:COOLING
                WaterToAirHeatPumpSimple::SimWatertoAirHPSimple(state,
                                                                BlankString,
                                                                furnace.CoolCoilNum,
                                                                furnace.CoolingCoilSensDemand,
                                                                furnace.CoolingCoilLatentDemand,
                                                                fanOp,
                                                                compressorOp,
                                                                CoolPartLoadRatio,
                                                                FirstHVACIteration); // CoolPartLoadRatio
                Real64 Dummy = 0.0;
                // COIL:WATERTOAIRHPSIMPLE:HEATING
                WaterToAirHeatPumpSimple::SimWatertoAirHPSimple(state,
                                                                BlankString,
                                                                furnace.HeatCoilNum,
                                                                furnace.HeatingCoilSensDemand,
                                                                Dummy,
                                                                fanOp,
                                                                compressorOp,
                                                                HeatPartLoadRatio,
                                                                FirstHVACIteration); // HeatPartLoadRatio
                //      Simulate the whole thing a second time so that the correct PLF required by the coils is used by the Fan. *******
                state.dataFans->fans(furnace.FanIndex)->simulate(state, FirstHVACIteration, state.dataFurnaces->FanSpeedRatio);
            }
            //    Simulate the cooling and heating coils
            // COIL:WATERTOAIRHPSIMPLE:COOLING
            WaterToAirHeatPumpSimple::SimWatertoAirHPSimple(state,
                                                            BlankString,
                                                            furnace.CoolCoilNum,
                                                            furnace.CoolingCoilSensDemand,
                                                            furnace.CoolingCoilLatentDemand,
                                                            fanOp,
                                                            compressorOp,
                                                            CoolPartLoadRatio,
                                                            FirstHVACIteration); // CoolPartLoadRatio
            Real64 Dummy = 0.0;
            // COIL:WATERTOAIRHPSIMPLE:HEATING
            WaterToAirHeatPumpSimple::SimWatertoAirHPSimple(state,
                                                            BlankString,
                                                            furnace.HeatCoilNum,
                                                            furnace.HeatingCoilSensDemand,
                                                            Dummy,
                                                            fanOp,
                                                            compressorOp,
                                                            HeatPartLoadRatio,
                                                            FirstHVACIteration); // HeatPartLoadRatio
            //     Simulate the draw-thru fan
            if (furnace.fanPlace == HVAC::FanPlace::BlowThru) {
                state.dataFans->fans(furnace.FanIndex)->simulate(state, FirstHVACIteration, state.dataFurnaces->FanSpeedRatio);
            }
            //     Simulate the supplemental heating coil
            if (furnace.DehumidControlType_Num == DehumidificationControlMode::CoolReheat && ReheatCoilLoad > 0.0) {
                bool SuppHeatingCoilFlag = true; // if true simulates supplemental heating coil
                CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, ReheatCoilLoad, fanOp, QActual);
            } else {
                bool SuppHeatingCoilFlag = true; // if true simulates supplemental heating coil
                CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, fanOp, QActual);
            }
            // Simulate the detailed water-to-air heat pump
        } else if (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_WaterToAir && furnace.WatertoAirHPType == WAHPCoilType::ParEst) {
            //    Simulate the draw-thru fan
            if (furnace.fanPlace == HVAC::FanPlace::BlowThru) {
                state.dataFans->fans(furnace.FanIndex)->simulate(state, FirstHVACIteration, state.dataFurnaces->FanSpeedRatio);
            }
            //    Simulate the cooling and heating coils
            WaterToAirHeatPump::SimWatertoAirHP(state,
                                                BlankString,
                                                furnace.CoolCoilNum,
                                                furnace.DesignMassFlowRate,
                                                fanOp,
                                                FirstHVACIteration,
                                                furnace.InitHeatPump,
                                                furnace.CoolingCoilSensDemand,
                                                furnace.CoolingCoilLatentDemand,
                                                compressorOp,
                                                CoolPartLoadRatio);
            Real64 Dummy = 0.0;
            WaterToAirHeatPump::SimWatertoAirHP(state,
                                                BlankString,
                                                furnace.HeatCoilNum,
                                                furnace.DesignMassFlowRate,
                                                fanOp,
                                                FirstHVACIteration,
                                                furnace.InitHeatPump,
                                                furnace.HeatingCoilSensDemand,
                                                Dummy,
                                                compressorOp,
                                                HeatPartLoadRatio);
            //    Simulate the draw-thru fan
            if (furnace.fanPlace == HVAC::FanPlace::DrawThru) {
                state.dataFans->fans(furnace.FanIndex)->simulate(state, FirstHVACIteration, state.dataFurnaces->FanSpeedRatio);
            }
            //    Simulate the supplemental heating coil
            HeatingCoils::SimulateHeatingCoilComponents(
                state, BlankString, FirstHVACIteration, HeatCoilLoad, furnace.SuppHeatCoilNum, _, true, fanOp);

        } else { // ELSE it's not a heat pump
            //   Simulate blow-thru fan
            if (furnace.fanPlace == HVAC::FanPlace::BlowThru) {

                state.dataFans->fans(furnace.FanIndex)->simulate(state, FirstHVACIteration, state.dataFurnaces->FanSpeedRatio);

                //     For non-linear coils, simulate coil to update PLF used by the ONOFF Fan
                if (furnace.fanType == HVAC::FanType::OnOff) {
                    if (furnace.type != HVAC::UnitarySysType::Unitary_HeatOnly && furnace.type != HVAC::UnitarySysType::Furnace_HeatOnly) {

                        if (!furnace.CoolCoilUpstream) {
                            bool SuppHeatingCoilFlag = false; // if false simulates heating coil
                            CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, fanOp, QActual);
                        }

                        if (furnace.coolCoilType == HVAC::CoilType::CoolingDXHXAssisted) {
                            HXAssistCoil::SimHXAssistedCoolingCoil(state,
                                                                                BlankString,
                                                                                FirstHVACIteration,
                                                                                compressorOp,
                                                                                CoolPartLoadRatio,
                                                                                furnace.CoolCoilNum,
                                                                                fanOp,
                                                                                HXUnitOn,
                                                                                OnOffAirFlowRatio,
                                                                                state.dataFurnaces->EconomizerFlag);
                        } else {
                            DXCoils::SimDXCoil(state,
                                               BlankString,
                                               compressorOp,
                                               FirstHVACIteration,
                                               furnace.CoolCoilNum,
                                               fanOp,
                                               CoolPartLoadRatio,
                                               OnOffAirFlowRatio,
                                               state.dataFurnaces->CoolHeatPLRRat);
                        }
                    }

                    if (furnace.CoolCoilUpstream) {
                        bool SuppHeatingCoilFlag = false; // if false simulates heating coil
                        CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, fanOp, QActual);
                    }
                    state.dataFans->fans(furnace.FanIndex)->simulate(state, FirstHVACIteration, state.dataFurnaces->FanSpeedRatio);
                } // Simple OnOff fan

            } // Blow thru fan

            //   Simulate the cooling and heating coils
            if (furnace.type != HVAC::UnitarySysType::Unitary_HeatOnly && furnace.type != HVAC::UnitarySysType::Furnace_HeatOnly) {

                if (!furnace.CoolCoilUpstream) {
                    bool SuppHeatingCoilFlag = false; // if false simulates heating coil
                    CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, fanOp, QActual);
                }

                if (furnace.coolCoilType == HVAC::CoilType::CoolingDXHXAssisted) {
                    HXAssistCoil::SimHXAssistedCoolingCoil(state,
                                                                        BlankString,
                                                                        FirstHVACIteration,
                                                                        compressorOp,
                                                                        CoolPartLoadRatio,
                                                                        furnace.CoolCoilNum,
                                                                        fanOp,
                                                                        HXUnitOn,
                                                                        OnOffAirFlowRatio,
                                                                        state.dataFurnaces->EconomizerFlag);
                } else {
                    DXCoils::SimDXCoil(state,
                                       BlankString,
                                       compressorOp,
                                       FirstHVACIteration,
                                       furnace.CoolCoilNum,
                                       fanOp,
                                       CoolPartLoadRatio,
                                       OnOffAirFlowRatio,
                                       state.dataFurnaces->CoolHeatPLRRat);
                }
            }

            if (furnace.CoolCoilUpstream) {
                bool SuppHeatingCoilFlag = false; // if false simulates heating coil
                CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, fanOp, QActual);
            }
            //   Simulate the draw-thru fan
            if (furnace.fanPlace == HVAC::FanPlace::DrawThru) {
                state.dataFans->fans(furnace.FanIndex)->simulate(state, FirstHVACIteration, state.dataFurnaces->FanSpeedRatio);
            }
            if (furnace.DehumidControlType_Num == DehumidificationControlMode::CoolReheat || furnace.SuppHeatCoilNum > 0) {
                bool SuppHeatingCoilFlag = true; // if true simulates supplemental heating coil
                CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, ReheatCoilLoad, fanOp, QActual);
            }
        } // IF(Furnace(FurnaceNum)%type == UnitarySys_HeatPump_AirToAir)THEN

        // Get mass flow rate after components are simulated
        auto &outletNode = state.dataLoopNodes->Node(furnace.FurnaceOutletNode);
        Real64 AirMassFlow = inletNode.MassFlowRate; // this should be outlet node as in 9897?

        // check the DesignMaxOutletTemp and reset if necessary (for Coil:Gas:Heating or Coil:Electric:Heating only)
        if (outletNode.Temp > furnace.DesignMaxOutletTemp) {
            Real64 Wout = outletNode.HumRat;
            Real64 Tout = furnace.DesignMaxOutletTemp;
            state.dataFurnaces->ModifiedHeatCoilLoad = HeatCoilLoad - (AirMassFlow * Psychrometrics::PsyCpAirFnW(Wout) * (outletNode.Temp - Tout));
            outletNode.Temp = Tout;
        }

        // If the fan runs continually do not allow coils to set OnOffFanPartLoadRatio.
        if (fanOp == HVAC::FanOp::Continuous) {
            state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
        }

        Real64 SensibleOutput = 0.0; // sensible output rate, {W}
        Real64 LatentOutput = 0.0;   // latent output rate, {W}
        Real64 TotalOutput = 0.0;    // total output rate, {W}
        CalcZoneSensibleLatentOutput(AirMassFlow,
                                     outletNode.Temp,
                                     outletNode.HumRat,
                                     state.dataLoopNodes->Node(furnace.NodeNumOfControlledZone).Temp,
                                     state.dataLoopNodes->Node(furnace.NodeNumOfControlledZone).HumRat,
                                     SensibleOutput,
                                     LatentOutput,
                                     TotalOutput);
        SensibleLoadMet = SensibleOutput - furnace.SenLoadLoss;
        furnace.SensibleLoadMet = SensibleLoadMet;

        if (furnace.Humidistat) {
            LatentLoadMet = LatentOutput - furnace.LatLoadLoss;
        } else {
            LatentLoadMet = 0.0;
        }
        furnace.LatentLoadMet = LatentLoadMet;
    }

    //        End of Update subroutines for the Furnace Module
    // *****************************************************************************

    Real64 CalcFurnaceResidual(EnergyPlusData &state,
                               Real64 const PartLoadRatio, // DX cooling coil part load ratio
                               int FurnaceNum,
                               bool FirstHVACIteration,
                               HVAC::FanOp const fanOp,
                               HVAC::CompressorOp compressorOp,
                               Real64 LoadToBeMet,
                               Real64 par6_loadFlag,
                               Real64 par7_sensLatentFlag,
                               Real64 par9_HXOnFlag,
                               Real64 par10_HeatingCoilPLR)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   Feb 2005

        // PURPOSE OF THIS SUBROUTINE:
        // To calculate the part-load ratio for cooling and heating coils

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 CoolPartLoadRatio;      // DX cooling coil part load ratio
        Real64 HeatPartLoadRatio;      // DX heating coil part load ratio (0 for other heating coil types)
        Real64 HeatCoilLoad;           // Heating coil load for gas heater
        Real64 SensibleLoadMet;        // Sensible cooling load met (furnace outlet with respect to control zone temp)
        Real64 LatentLoadMet;          // Latent cooling load met (furnace outlet with respect to control zone humidity ratio)
        Real64 OnOffAirFlowRatio;      // Ratio of compressor ON air mass flow to AVERAGE air mass flow over time step
        Real64 CoolingHeatingPLRRatio; // ratio of cooling PLR to heating PLR, used for cycling fan RH control
        bool HXUnitOn;                 // flag to enable HX based on zone moisture load

        //        // Convert parameters to usable variables
        //        int FurnaceNum = int(Par(1));
        //        bool FirstHVACIteration = Par(2) == 1.0;
        //        int FanfanOp = int(Par(3));
        //        CompressorOperation CompressorOp = static_cast<CompressorOperation>(Par(4));
        //        Real64 LoadToBeMet = Par(5);
        //        Real64 par6_loadFlag = Par(6);
        //        Real64 par7_sensLatentFlag = Par(7);
        //        Real64 par9_HXOnFlag = Par(9);
        //        Real64 par10_HeatingCoilPLR = Par(10);

        auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);
        
        if (par6_loadFlag == 1.0) {
            CoolPartLoadRatio = PartLoadRatio;
            HeatPartLoadRatio = 0.0;
            HeatCoilLoad = 0.0;
        } else {
            CoolPartLoadRatio = 0.0;
            HeatPartLoadRatio = PartLoadRatio;

            if (furnace.heatCoilType == HVAC::CoilType::HeatingGasOrOtherFuel || furnace.heatCoilType == HVAC::CoilType::HeatingElectric ||
                furnace.heatCoilType == HVAC::CoilType::HeatingWater || furnace.heatCoilType == HVAC::CoilType::HeatingSteam) {
                HeatCoilLoad = furnace.DesignHeatingCapacity * PartLoadRatio;
            } else {
                HeatCoilLoad = 0.0;
            }
        }

        //  OnOffAirFlowRatio = Par(8)
        if (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_WaterToAir) {
            furnace.CompPartLoadRatio = PartLoadRatio;
        }

        if (par9_HXOnFlag == 1.0) {
            HXUnitOn = true;
        } else {
            HXUnitOn = false;
        }

        if (par10_HeatingCoilPLR > 0.0) {
            //    Par(10) = Furnace(FurnaceNum)%HeatPartLoadRatio
            //    FanOp = CycFan and Furnace(FurnaceNum)%HeatPartLoadRatio must be > 0 for Part(10) to be greater than 0
            //    This variable used when in heating mode and dehumidification (cooling) is required.
            CoolingHeatingPLRRatio = min(1.0, CoolPartLoadRatio / furnace.HeatPartLoadRatio);
        } else {
            CoolingHeatingPLRRatio = 1.0;
        }

        // Subroutine arguments
        CalcFurnaceOutput(state,
                          FurnaceNum,
                          FirstHVACIteration,
                          fanOp,
                          compressorOp,
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
            }
            return (SensibleLoadMet - LoadToBeMet) / LoadToBeMet;
        }
        if (LoadToBeMet == 0.0) {
            return (LatentLoadMet - LoadToBeMet) / 100.0;
        }
        return (LatentLoadMet - LoadToBeMet) / LoadToBeMet;
    }

    Real64 CalcWaterToAirResidual(EnergyPlusData &state,
                                  Real64 const PartLoadRatio, // DX cooling coil part load ratio
                                  int FurnaceNum,
                                  bool FirstHVACIteration,
                                  HVAC::FanOp const fanOp,
                                  HVAC::CompressorOp compressorOp,
                                  Real64 LoadToBeMet,
                                  Real64 par6_loadTypeFlag,
                                  Real64 par7_latentOrSensible,
                                  Real64 ZoneSensLoadMetFanONCompOFF,
                                  Real64 par9_HXUnitOne)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   October 2006

        // PURPOSE OF THIS SUBROUTINE:
        // To calculate the part-load ratio for water to air HP's
        // this is used for parameter estimation WAHPs but not equation fit WAHPs

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 CoolPartLoadRatio; // DX cooling coil part load ratio
        Real64 HeatPartLoadRatio; // DX heating coil part load ratio (0 for other heating coil types)
        Real64 HeatCoilLoad;      // Heating coil load for gas heater
        Real64 ZoneSensLoadMet;   // Sensible cooling load met (furnace outlet with respect to control zone temp)
        Real64 ZoneLatLoadMet;    // Latent cooling load met (furnace outlet with respect to control zone humidity ratio)
        Real64 Dummy;
        Real64 HPCoilSensDemand;
        Real64 OnOffAirFlowRatio;
        bool HXUnitOn; // flag to enable HX based on zone moisture load (not valid for water-to-air HP's

        // Convert parameters to usable variables
        //        int FurnaceNum = int(Par[0]);
        //        bool FirstHVACIteration = Par[1] == 1.0;
        //        int FanOp = int(Par[2]);
        //        CompressorOperation CompressorOp = static_cast<CompressorOperation>(Par[3]);
        //        Real64 LoadToBeMet = Par[4];
        //        Real64 par6_loadTypeFlag = Par[5];
        //        Real64 par7_latentOrSensible = Par[6];
        //        Real64 ZoneSensLoadMetFanONCompOFF = Par[7];
        //        Real64 par9_HXUnitOne = Par[8];

        int CoilIndex;

        auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);
        
        if (par6_loadTypeFlag == 1.0) {
            CoolPartLoadRatio = PartLoadRatio;
            HeatPartLoadRatio = 0.0;
            HeatCoilLoad = 0.0;
            CoilIndex = furnace.CoolCoilNum;
        } else {
            CoolPartLoadRatio = 0.0;
            HeatPartLoadRatio = PartLoadRatio;
            CoilIndex = furnace.HeatCoilNum;
        }

        // Get child component RuntimeFrac
        Real64 RuntimeFrac;
        switch (furnace.WatertoAirHPType) {
        case WAHPCoilType::Simple: {
            RuntimeFrac = state.dataWaterToAirHeatPumpSimple->SimpleWatertoAirHP(CoilIndex).RunFrac;
            break;
        }
        case WAHPCoilType::ParEst: {
            RuntimeFrac = state.dataWaterToAirHeatPump->WatertoAirHP(CoilIndex).RunFrac;
            break;
        }
        case WAHPCoilType::VarSpeedEquationFit: {
            RuntimeFrac = state.dataVariableSpeedCoils->VarSpeedCoil(CoilIndex).RunFrac;
            break;
        }
        default:
            RuntimeFrac = 1.0; // Programming error. Assert failure?
        }

        state.dataFurnaces->OnOffFanPartLoadFractionSave = state.dataHVACGlobal->OnOffFanPartLoadFraction;
        // update fan and compressor run times
        furnace.CompPartLoadRatio = PartLoadRatio;

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
            furnace.HeatingCoilSensDemand = 0.0;
            furnace.CoolingCoilSensDemand = std::abs(HPCoilSensDemand);
        } else {
            furnace.HeatingCoilSensDemand = HPCoilSensDemand;
            furnace.CoolingCoilSensDemand = 0.0;
        }
        furnace.InitHeatPump = false; // initialization call to Calc Furnace

        // Calculate the zone loads met and the new part load ratio and for the specified run time
        Dummy = 0.0;
        OnOffAirFlowRatio = 1.0;
        if (par9_HXUnitOne == 1.0) {
            HXUnitOn = true;
        } else {
            HXUnitOn = false;
        }

        //  Subroutine arguments
        //  CALL CalcFurnaceOutput(FurnaceNum,FirstHVACIteration,FanOp,compressorOp,CoolPartLoadRatio,&
        //                         HeatPartLoadRatio, HeatCoilLoad, ReHeatCoilLoad, SensibleLoadMet, LatentLoadMet, HXUnitOn)
        CalcFurnaceOutput(state,
                          FurnaceNum,
                          FirstHVACIteration,
                          fanOp,
                          compressorOp,
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
        }
        return (ZoneLatLoadMet - LoadToBeMet) / LoadToBeMet;
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

        // PURPOSE OF THIS SUBROUTINE:
        // Set the average air mass flow rates using the part-load fraction of the HVAC system for this time step
        // Set OnOffAirFlowRatio to be used by DX coils

        // METHODOLOGY EMPLOYED:
        // The air flow rate in cooling, heating, and no cooling or heating can be different.
        // Calculate the air flow rate based on initializations made in InitFurnace.
        auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);
        int InletNode = furnace.FurnaceInletNode;
        Real64 AverageUnitMassFlow =
            (PartLoadRatio * state.dataFurnaces->CompOnMassFlow) + ((1 - PartLoadRatio) * state.dataFurnaces->CompOffMassFlow);
        if (state.dataFurnaces->CompOffFlowRatio > 0.0) {
            state.dataFurnaces->FanSpeedRatio =
                (PartLoadRatio * state.dataFurnaces->CompOnFlowRatio) + ((1 - PartLoadRatio) * state.dataFurnaces->CompOffFlowRatio);
        } else {
            state.dataFurnaces->FanSpeedRatio = state.dataFurnaces->CompOnFlowRatio;
        }

        // IF the furnace is scheduled on or nightime cycle overrides fan schedule. Uses same logic as fan.
        if (furnace.availSched->getCurrentVal() > 0.0 &&
            ((furnace.fanAvailSched->getCurrentVal() > 0.0 || state.dataHVACGlobal->TurnFansOn) &&
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

        furnace.MdotFurnace = state.dataFurnaces->CompOnMassFlow;
        state.dataFurnaces->OnOffAirFlowRatioSave = OnOffAirFlowRatio;
    }

    // Beginning of Reporting subroutines for the Furnace Module
    // *****************************************************************************

    void ReportFurnace(EnergyPlusData &state, int const FurnaceNum, int const AirLoopNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Liesen
        //       DATE WRITTEN   Feb 2001

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the report variable for the coils.

        // METHODOLOGY EMPLOYED:
        // Update fan part-load ratio based on mass flow rate ratio.
        // Update global variables used by AirflowNetwork module.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 ratio;
        Real64 OnOffRatio;
        auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);

        // Report the Furnace Fan Part Load Ratio
        if (furnace.NumOfSpeedCooling < 1) {
            if (furnace.DesignMassFlowRate > 0.0) {
                furnace.FanPartLoadRatio = furnace.MdotFurnace / furnace.DesignMassFlowRate;
            } else {
                furnace.FanPartLoadRatio = 0.0;
            }
        }

        // Set mass flow rates during on and off cycle using an OnOff fan
        if (state.afn->distribution_simulated) {
            state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopSystemOnMassFlowrate = state.dataFurnaces->CompOnMassFlow;
            state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopSystemOffMassFlowrate = state.dataFurnaces->CompOffMassFlow;
            state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopFanOperationMode = furnace.fanOp;
            state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio = furnace.FanPartLoadRatio;
            OnOffRatio = state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio;
            if (furnace.type == HVAC::UnitarySysType::Unitary_HeatPump_AirToAir) {
                state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio =
                    max(furnace.FanPartLoadRatio, furnace.HeatPartLoadRatio, furnace.CoolPartLoadRatio);
                state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio =
                    min(1.0, state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio);
            }
            if (furnace.type == HVAC::UnitarySysType::Unitary_HeatCool) {
                if (furnace.HeatPartLoadRatio == 0.0 && furnace.CoolPartLoadRatio == 0.0 && furnace.FanPartLoadRatio > 0.0) {
                    if (state.dataFurnaces->CompOnMassFlow < max(furnace.MaxCoolAirMassFlow, furnace.MaxHeatAirMassFlow) &&
                        state.dataFurnaces->CompOnMassFlow > 0.0) {
                        ratio = max(furnace.MaxCoolAirMassFlow, furnace.MaxHeatAirMassFlow) / state.dataFurnaces->CompOnMassFlow;
                        state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio =
                            state.dataAirLoop->AirLoopAFNInfo(AirLoopNum).LoopOnOffFanPartLoadRatio * ratio;
                    }
                }
            }
        }
        if (furnace.FirstPass) {
            if (!state.dataGlobal->SysSizingCalc) {
                DataSizing::resetHVACSizingGlobals(state, 0, state.dataSize->CurSysNum, furnace.FirstPass);
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
                               HVAC::FanOp const fanOp,        // fan operation mode
                               Real64 &HeatCoilLoadmet         // Heating Load Met
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         Bereket Nigusse, FSEC/UCF
        //       DATE WRITTEN   January 2012

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine simulates the four non dx heating coil types: Gas, Electric, hot water and steam.

        // METHODOLOGY EMPLOYED:
        // Simply calls the different heating coil component.  The hot water flow rate matching the coil load
        // is calculated iteratively.

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr ErrTolerance(0.001); // convergence limit for hotwater coil
        int constexpr SolveMaxIter(50);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        std::string HeatCoilName;
        Real64 mdot;              // heating coil steam or hot water mass flow rate
        Real64 MinWaterFlow;      // coil minimum hot water mass flow rate, kg/s
        Real64 MaxHotWaterFlow;   // coil maximum hot water mass flow rate, kg/s
        Real64 HotWaterMdot;      // actual hot water mass flow rate
        HVAC::CoilType coilType;       // heating coil type number
        int HeatCoilNum(0);  // heating coil index
        int CoilControlNode(0);   // control node for hot water and steam heating coils
        int CoilOutletNode(0);    // air outlet node of the heating coils
        PlantLocation plantLoc{}; // plant loop location

        Real64 QActual = 0.0;                                               // actual heating load
        auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);

        if (SuppHeatingCoilFlag) {
            HeatCoilName = furnace.SuppHeatCoilName;
            HeatCoilNum = furnace.SuppHeatCoilNum;
            CoilControlNode = furnace.SuppHeatCoilControlNode;
            CoilOutletNode = furnace.SuppHeatCoilAirOutletNode;
            coilType = furnace.suppHeatCoilType;
            plantLoc = furnace.SuppPlantLoc;
            MaxHotWaterFlow = furnace.MaxSuppHeatCoilFluidFlow;
        } else {
            HeatCoilName = furnace.HeatCoilName;
            HeatCoilNum = furnace.HeatCoilNum;
            CoilControlNode = furnace.HeatCoilControlNode;
            CoilOutletNode = furnace.CoilOutletNode;
            coilType = furnace.heatCoilType;
            plantLoc = furnace.plantLoc;
            MaxHotWaterFlow = furnace.MaxHeatCoilFluidFlow;
        }

        switch (coilType) {
        case HVAC::CoilType::HeatingGasOrOtherFuel:
        case HVAC::CoilType::HeatingElectric:
        case HVAC::CoilType::HeatingDesuperheater: {
            HeatingCoils::SimulateHeatingCoilComponents(
                state, HeatCoilName, FirstHVACIteration, QCoilLoad, HeatCoilNum, QActual, SuppHeatingCoilFlag, fanOp);
        } break;
        case HVAC::CoilType::HeatingWater: {
            if (QCoilLoad > HVAC::SmallLoad) {
                PlantUtilities::SetComponentFlowRate(state, MaxHotWaterFlow, CoilControlNode, CoilOutletNode, plantLoc);
                WaterCoils::SimulateWaterCoilComponents(state, HeatCoilName, FirstHVACIteration, HeatCoilNum, QActual, fanOp);

                if (QActual > (QCoilLoad + HVAC::SmallLoad)) {
                    // control water flow to obtain output matching QCoilLoad
                    MinWaterFlow = 0.0;
                    auto f = [&state, FurnaceNum, FirstHVACIteration, QCoilLoad, SuppHeatingCoilFlag](Real64 const HWFlow) {
                        auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);
                        Real64 QCoilRequested = QCoilLoad;

                        // FUNCTION LOCAL VARIABLE DECLARATIONS:
                        Real64 QCoilActual;   // delivered coil load, W
                        Real64 mdot = HWFlow; // to get non-const argument
                        QCoilActual = QCoilRequested;
                        if (!SuppHeatingCoilFlag) {
                            PlantUtilities::SetComponentFlowRate(state,
                                                                 mdot,
                                                                 furnace.HeatCoilControlNode,
                                                                 furnace.CoilOutletNode,
                                                                 furnace.plantLoc);
                            WaterCoils::SimulateWaterCoilComponents(state,
                                                                    furnace.HeatCoilName,
                                                                    FirstHVACIteration,
                                                                    furnace.HeatCoilNum,
                                                                    QCoilActual,
                                                                    furnace.fanOp);
                        } else {
                            // supplemental coil
                            PlantUtilities::SetComponentFlowRate(state,
                                                                 mdot,
                                                                 furnace.SuppHeatCoilControlNode,
                                                                 furnace.SuppHeatCoilAirOutletNode,
                                                                 furnace.SuppPlantLoc);
                            // simulate the hot water supplemental heating coil
                            WaterCoils::SimulateWaterCoilComponents(state,
                                                                    furnace.SuppHeatCoilName,
                                                                    FirstHVACIteration,
                                                                    furnace.SuppHeatCoilNum,
                                                                    QCoilActual,
                                                                    furnace.fanOp);
                        }
                        return QCoilRequested != 0.0 ? (QCoilActual - QCoilRequested) / QCoilRequested : 0.0;
                    };
                    int SolFlag = 0;
                    General::SolveRoot(state, ErrTolerance, SolveMaxIter, SolFlag, HotWaterMdot, f, MinWaterFlow, MaxHotWaterFlow);
                    if (SolFlag == -1) {
                        if (furnace.HotWaterCoilMaxIterIndex == 0) {
                            ShowWarningMessage(state,
                                               format("CalcNonDXHeatingCoils: Hot water coil control failed for {}=\"{}\"",
                                                      HVAC::unitarySysTypeNames[(int)furnace.type],
                                                      furnace.Name));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state, format("  Iteration limit [{}] exceeded in calculating hot water mass flow rate", SolveMaxIter));
                        }
                        ShowRecurringWarningErrorAtEnd(
                            state,
                            format("CalcNonDXHeatingCoils: Hot water coil control failed (iteration limit [{}]) for {}=\"{}",
                                   SolveMaxIter,
                                   HVAC::unitarySysTypeNames[(int)furnace.type],
                                   furnace.Name),
                            furnace.HotWaterCoilMaxIterIndex);
                    } else if (SolFlag == -2) {
                        if (furnace.HotWaterCoilMaxIterIndex2 == 0) {
                            ShowWarningMessage(state,
                                               format("CalcNonDXHeatingCoils: Hot water coil control failed (maximum flow limits) for {}=\"{}\"",
                                                      HVAC::unitarySysTypeNames[(int)furnace.type],
                                                      furnace.Name));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state, "...Bad hot water maximum flow rate limits");
                            ShowContinueError(state, format("...Given minimum water flow rate={:.3R} kg/s", MinWaterFlow));
                            ShowContinueError(state, format("...Given maximum water flow rate={:.3R} kg/s", MaxHotWaterFlow));
                        }
                        ShowRecurringWarningErrorAtEnd(state,
                                                       format("CalcNonDXHeatingCoils: Hot water coil control failed (flow limits) for {}=\"{}\"",
                                                              HVAC::unitarySysTypeNames[(int)furnace.type],
                                                              furnace.Name),
                                                       furnace.HotWaterCoilMaxIterIndex2,
                                                       MaxHotWaterFlow,
                                                       MinWaterFlow,
                                                       _,
                                                       "[kg/s]",
                                                       "[kg/s]");
                    }
                }
            } else {
                mdot = 0.0;
                PlantUtilities::SetComponentFlowRate(state, mdot, CoilControlNode, CoilOutletNode, plantLoc);
            }
            // simulate the hot water heating coil
            WaterCoils::SimulateWaterCoilComponents(state, HeatCoilName, FirstHVACIteration, HeatCoilNum, QActual, fanOp);
        } break;
        case HVAC::CoilType::HeatingSteam: {
            if (QCoilLoad > HVAC::SmallLoad) {
                PlantUtilities::SetComponentFlowRate(state, MaxHotWaterFlow, CoilControlNode, CoilOutletNode, plantLoc);
                // simulate the steam heating coil
                SteamCoils::SimulateSteamCoilComponents(state, HeatCoilName, FirstHVACIteration, HeatCoilNum, QCoilLoad, QActual, fanOp);
            } else {
                mdot = 0.0;
                PlantUtilities::SetComponentFlowRate(state, mdot, CoilControlNode, CoilOutletNode, plantLoc);
                // simulate the steam heating coil
                SteamCoils::SimulateSteamCoilComponents(state, HeatCoilName, FirstHVACIteration, HeatCoilNum, QCoilLoad, QActual, fanOp);
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

        Real64 PartLoadFrac; // compressor part load fraction
        Real64 SpeedRatio;   // compressor speed ratio
        Real64 QTotUnitOut;  // capacity output
        auto &SpeedNum = state.dataFurnaces->SpeedNum;
        auto &SupHeaterLoad = state.dataFurnaces->SupHeaterLoad;
        HVAC::CompressorOp compressorOp; // compressor operation; 1=on, 0=off
        Real64 QSensUnitOut;             // sensible capacity output
        Real64 QLatUnitOut;              // latent capacity output
        Real64 ActualSensibleOutput;     // Actual furnace sensible capacity
        Real64 QToHeatSetPt;             // Load required to meet heating setpoint temp (>0 is a heating load)
        Real64 NoCompOutput;             // output when no active compressor [W]
        bool EconoActive;                // TRUE if Economizer is active

        // zero DX coils, and supplemental electric heater electricity consumption
        state.dataHVACGlobal->DXElecHeatingPower = 0.0;
        state.dataHVACGlobal->DXElecCoolingPower = 0.0;
        state.dataFurnaces->SaveCompressorPLR = 0.0;
        state.dataHVACGlobal->ElecHeatingCoilPower = 0.0;
        state.dataHVACGlobal->SuppHeatingCoilPower = 0.0;
        state.dataHVACGlobal->DefrostElecPower = 0.0;

        Real64 SystemSensibleLoad = QZnReq; // Positive value means heating required
        Real64 TotalZoneSensibleLoad = QZnReq;
        Real64 TotalZoneLatentLoad = QLatReq;
        auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);
        // initialize local variables
        bool UnitOn = true;
        int OutletNode = furnace.FurnaceOutletNode;
        int InletNode = furnace.FurnaceInletNode;
        Real64 AirMassFlow = furnace.DesignMassFlowRate;
        HVAC::FanOp fanOp = furnace.fanOp; // fan operating mode
        int ZoneNum = furnace.ControlZoneNum;

        // Set latent load for heating
        if (state.dataFurnaces->HeatingLoad) {
            furnace.HeatCoolMode = Furnaces::ModeOfOperation::HeatingMode;
            // Set latent load for cooling and no sensible load condition
        } else if (state.dataFurnaces->CoolingLoad) {
            furnace.HeatCoolMode = Furnaces::ModeOfOperation::CoolingMode;
        } else {
            furnace.HeatCoolMode = Furnaces::ModeOfOperation::NoCoolHeat;
        }

        // set the on/off flags
        if (furnace.fanOp == HVAC::FanOp::Cycling) {
            // cycling unit only runs if there is a cooling or heating load.
            if (std::abs(QZnReq) < HVAC::SmallLoad || AirMassFlow < HVAC::SmallMassFlow ||
                state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) {
                UnitOn = false;
            }
        } else if (furnace.fanOp == HVAC::FanOp::Continuous) {
            // continuous unit: fan runs if scheduled on; coil runs only if there is a cooling or heating load
            if (AirMassFlow < HVAC::SmallMassFlow) {
                UnitOn = false;
            }
        }

        state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
        EconoActive = (AirLoopNum != 0) ? state.dataAirLoop->AirLoopControlInfo(AirLoopNum).EconoActive : false;

        Real64 SaveMassFlowRate = state.dataLoopNodes->Node(InletNode).MassFlowRate;
        // decide current working mode for IHP
        if ((FirstHVACIteration) && (furnace.isIHP))
            IntegratedHeatPump::DecideWorkMode(state, furnace.ihpNum, TotalZoneSensibleLoad, TotalZoneLatentLoad);

        if (!FirstHVACIteration && furnace.fanOp == HVAC::FanOp::Cycling &&
            (QZnReq < (-1.0 * HVAC::SmallLoad) || TotalZoneLatentLoad < (-HVAC::SmallLoad)) && EconoActive) {
            // for cycling fan, cooling load, check whether furnace can meet load with compressor off
            compressorOp = HVAC::CompressorOp::Off;
            ControlVSHPOutput(state,
                              FurnaceNum,
                              FirstHVACIteration,
                              compressorOp,
                              fanOp,
                              TotalZoneSensibleLoad,
                              TotalZoneLatentLoad,
                              SpeedNum,
                              SpeedRatio,
                              PartLoadFrac,
                              OnOffAirFlowRatio,
                              SupHeaterLoad);

            TotalZoneSensibleLoad = QZnReq;
            TotalZoneLatentLoad = QLatReq;

            if (SpeedNum == furnace.NumOfSpeedCooling && SpeedRatio == 1.0) {
                // compressor on (reset inlet air mass flow rate to starting value)
                state.dataLoopNodes->Node(InletNode).MassFlowRate = SaveMassFlowRate;
                compressorOp = HVAC::CompressorOp::On;
                ControlVSHPOutput(state,
                                  FurnaceNum,
                                  FirstHVACIteration,
                                  compressorOp,
                                  fanOp,
                                  TotalZoneSensibleLoad,
                                  TotalZoneLatentLoad,
                                  SpeedNum,
                                  SpeedRatio,
                                  PartLoadFrac,
                                  OnOffAirFlowRatio,
                                  SupHeaterLoad);
            }
        } else {
            // compressor on
            compressorOp = HVAC::CompressorOp::On;

            ControlVSHPOutput(state,
                              FurnaceNum,
                              FirstHVACIteration,
                              compressorOp,
                              fanOp,
                              TotalZoneSensibleLoad,
                              TotalZoneLatentLoad,
                              SpeedNum,
                              SpeedRatio,
                              PartLoadFrac,
                              OnOffAirFlowRatio,
                              SupHeaterLoad);
        }

        if (furnace.type == HVAC::UnitarySysType::Unitary_HeatCool) {
            state.dataFurnaces->SaveCompressorPLR = PartLoadFrac;
        } else {
            if (SpeedNum > 1) {
                state.dataFurnaces->SaveCompressorPLR = 1.0;
            }

            if (PartLoadFrac == 1.0 && state.dataFurnaces->SaveCompressorPLR < 1.0) {
                PartLoadFrac = state.dataFurnaces->SaveCompressorPLR;
            }
        }

        Real64 ReheatCoilLoad = 0.0;
        TotalZoneSensibleLoad = QZnReq;
        TotalZoneLatentLoad = QLatReq;
        //     Calculate the reheat coil output
        if ((furnace.availSched->getCurrentVal() > 0.0) &&
            (furnace.Humidistat && furnace.DehumidControlType_Num == DehumidificationControlMode::CoolReheat &&
             (QLatReq < 0.0))) { // if a Humidistat is installed and dehumidification control type is CoolReheat
            CalcVarSpeedHeatPump(state,
                                 FurnaceNum,
                                 FirstHVACIteration,
                                 compressorOp,
                                 SpeedNum,
                                 SpeedRatio,
                                 PartLoadFrac,
                                 ActualSensibleOutput,
                                 QLatUnitOut,
                                 TotalZoneSensibleLoad,
                                 TotalZoneLatentLoad,
                                 OnOffAirFlowRatio,
                                 ReheatCoilLoad);
            if (furnace.ZoneSequenceHeatingNum > 0) {
                QToHeatSetPt = (state.dataZoneEnergyDemand->ZoneSysEnergyDemand(furnace.ControlZoneNum)
                                    .SequencedOutputRequiredToHeatingSP(furnace.ZoneSequenceHeatingNum) /
                                furnace.ControlZoneMassFlowFrac);
            } else {
                QToHeatSetPt = (state.dataZoneEnergyDemand->ZoneSysEnergyDemand(furnace.ControlZoneNum).OutputRequiredToHeatingSP /
                                furnace.ControlZoneMassFlowFrac);
            }
            //       Cooling mode or floating condition and dehumidification is required
            if (QToHeatSetPt < 0.0) {
                //         Calculate the reheat coil load wrt the heating setpoint temperature. Reheat coil picks up
                //         the entire excess sensible cooling (DX cooling coil and impact of outdoor air).
                ReheatCoilLoad = max(0.0, (QToHeatSetPt - ActualSensibleOutput));
                furnace.DehumidInducedHeatingDemandRate = ReheatCoilLoad;
                //       Heating mode and dehumidification is required
            } else {
                ReheatCoilLoad = max(QToHeatSetPt, QToHeatSetPt - ActualSensibleOutput);
                furnace.DehumidInducedHeatingDemandRate = max(0.0, ActualSensibleOutput * (-1.0));
            }

            SupHeaterLoad = 0.0;
            CalcVarSpeedHeatPump(state,
                                 FurnaceNum,
                                 FirstHVACIteration,
                                 compressorOp,
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
                                 compressorOp,
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
                                 compressorOp,
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

        furnace.MdotFurnace = AirMassFlow;

        QTotUnitOut =
            AirMassFlow * (state.dataLoopNodes->Node(OutletNode).Enthalpy - state.dataLoopNodes->Node(furnace.NodeNumOfControlledZone).Enthalpy);

        state.dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = AirMassFlow;
        state.dataLoopNodes->Node(OutletNode).MassFlowRateMaxAvail = AirMassFlow;

        if (!FirstHVACIteration && AirMassFlow > 0.0 && AirLoopNum > 0) {
            int TotBranchNum = state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).NumOutletBranches;
            if (TotBranchNum == 1) {
                int ZoneSideNodeNum = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).ZoneEquipSupplyNodeNum(1);
                // THE MASS FLOW PRECISION of the system solver is not enough for some small air flow rate iterations , BY DEBUGGING
                // it may cause mass flow rate oscillations between airloop and zoneequip
                // specify the air flow rate directly for one-to-one system, when the iteration deviation is closing the solver precision level
                // 0.02 is 2 * HVACFlowRateToler, in order to accommodate the system solver precision level
                if (std::abs(AirMassFlow - state.dataLoopNodes->Node(ZoneSideNodeNum).MassFlowRate) < 0.02) {
                    state.dataLoopNodes->Node(ZoneSideNodeNum).MassFlowRateMaxAvail = AirMassFlow;
                }
                state.dataLoopNodes->Node(ZoneSideNodeNum).MassFlowRate = AirMassFlow;
            }

            // the below might be useful if more divergences occur
            // Node(PrimaryAirSystem(AirLoopNumber)%Branch(1)%NodeNumIn)%MassFlowRateMaxAvail = AirMassFlow
            // Node(PrimaryAirSystem(AirLoopNumber)%Branch(1)%NodeNumIn)%MassFlowRate = AirMassFlow
        }

        // report variables
        furnace.DehumidInducedHeatingDemandRate = ReheatCoilLoad;
        if (QZnReq > HVAC::SmallLoad) { // HEATING LOAD
            furnace.CoolingCoilSensDemand = 0.0;
            furnace.HeatingCoilSensDemand = QZnReq;
        } else {
            furnace.CoolingCoilSensDemand = std::abs(QZnReq);
            furnace.HeatingCoilSensDemand = 0.0;
        }

        furnace.CompPartLoadRatio = state.dataFurnaces->SaveCompressorPLR;
        if (furnace.fanOp == HVAC::FanOp::Cycling) {
            if (SupHeaterLoad > 0.0) {
                furnace.FanPartLoadRatio = 1.0;
            } else {
                if (SpeedNum < 2) {
                    furnace.FanPartLoadRatio = PartLoadFrac;
                } else {
                    furnace.FanPartLoadRatio = 1.0;
                }
            }
        } else {
            if (UnitOn) {
                furnace.FanPartLoadRatio = 1.0;
            } else {
                if (SpeedNum < 2) {
                    furnace.FanPartLoadRatio = PartLoadFrac;
                } else {
                    furnace.FanPartLoadRatio = 1.0;
                }
            }
        }
    }

    //******************************************************************************

    void ControlVSHPOutput(EnergyPlusData &state,
                           int const FurnaceNum,                  // Unit index of engine driven heat pump
                           bool const FirstHVACIteration,         // flag for 1st HVAC iteration in the time step
                           HVAC::CompressorOp const compressorOp, // compressor operation; 1=on, 0=off
                           HVAC::FanOp const fanOp,               // operating mode: FanOp::Cycling | FanOp::Continuous
                           Real64 &QZnReq,                        // cooling or heating output needed by zone [W]
                           Real64 QLatReq,                        // latent cooling output needed by zone [W]
                           int &SpeedNum,                         // Speed number
                           Real64 &SpeedRatio,                    // unit speed ratio for DX coils
                           Real64 &PartLoadFrac,                  // unit part load fraction
                           Real64 &OnOffAirFlowRatio,             // ratio of compressor ON airflow to AVERAGE airflow over timestep
                           Real64 &SupHeaterLoad                  // Supplemental heater load [W]
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Bo Shen, based on HVACMultiSpeedHeatPump:ControlMSHPOutput
        //       DATE WRITTEN   March,  2012

        // PURPOSE OF THIS SUBROUTINE:
        // Determine the part load fraction at low speed, and speed ratio at high speed for this time step.

        // METHODOLOGY EMPLOYED:
        // Use RegulaFalsi technique to iterate on part-load ratio until convergence is achieved.

        // SUBROUTINE PARAMETER DEFINITIONS:
        int constexpr MaxIte(500); // maximum number of iterations

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 FullOutput;   // unit full output when compressor is operating [W]
        Real64 LowOutput;    // unit full output at low speed [W]
        Real64 TempOutput;   // unit output when iteration limit exceeded [W]
        Real64 NoCompOutput; // output when no active compressor [W]
        int SolFla;          // Flag of RegulaFalsi solver
        Real64 QCoilActual;  // coil load actually delivered returned to calling component
        int i;               // Speed index
        IntegratedHeatPump::IHPOperationMode IHPMode(IntegratedHeatPump::IHPOperationMode::Idle);

        SupHeaterLoad = 0.0;
        PartLoadFrac = 0.0;
        SpeedRatio = 0.0;
        SpeedNum = 1;
        Real64 LatOutput = 0.0;
        Real64 noLatOutput = 0.0;
        Real64 ErrorToler = 0.001; // Error tolerance for convergence from input deck

        auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);

        if (furnace.availSched->getCurrentVal() == 0.0) return;

        // Get result when DX coil is off
        SupHeaterLoad = 0.0;
        CalcVarSpeedHeatPump(state,
                             FurnaceNum,
                             FirstHVACIteration,
                             compressorOp,
                             SpeedNum,
                             SpeedRatio,
                             PartLoadFrac,
                             NoCompOutput,
                             noLatOutput,
                             0.0,
                             0.0,
                             OnOffAirFlowRatio,
                             SupHeaterLoad);

        if (furnace.isIHP) {
            IHPMode = IntegratedHeatPump::GetCurWorkMode(state, furnace.ihpNum);
            if ((IntegratedHeatPump::IHPOperationMode::DedicatedWaterHtg == IHPMode) ||
                (IntegratedHeatPump::IHPOperationMode::SCWHMatchWH == IHPMode)) { // cooling capacity is a resultant
                return;
            }
        }

        // If cooling and NoCompOutput < QZnReq, the coil needs to be off
        // If heating and NoCompOutput > QZnReq, the coil needs to be off
        // If no cooling or heating and no latent load, the coil needs to be off
        if (QZnReq < -HVAC::SmallLoad) {
            if (NoCompOutput < QZnReq && QLatReq >= -HVAC::SmallLoad) {
                return;
            }
        } else if (QZnReq > HVAC::SmallLoad) {
            if (NoCompOutput > QZnReq && QLatReq >= -HVAC::SmallLoad) {
                return;
            }
            if (QLatReq <= -HVAC::SmallLoad) {
                QZnReq = 0.0; // Zero heating load to allow dehumidification
            }
        } else {
            if (QLatReq >= -HVAC::SmallLoad) {
                return;
            }
        }

        // Get full load result
        PartLoadFrac = 1.0;
        SpeedRatio = 1.0;
        if (furnace.HeatCoolMode == Furnaces::ModeOfOperation::HeatingMode) {
            SpeedNum = furnace.NumOfSpeedHeating;
        } else if (furnace.HeatCoolMode == Furnaces::ModeOfOperation::CoolingMode) {
            SpeedNum = furnace.NumOfSpeedCooling;
        } else if (QLatReq < -HVAC::SmallLoad) {
            SpeedNum = furnace.NumOfSpeedCooling;
        } else {
            SpeedNum = 1;
            PartLoadFrac = 0.0;
        }

        if (furnace.isIHP) SpeedNum = IntegratedHeatPump::GetMaxSpeedNumIHP(state, furnace.ihpNum);
        // WAS
        // if (thisFurnace.bIsIHP) {
        //    SpeedNum = IntegratedHeatPump::GetMaxSpeedNumIHP(state, thisFurnace.CoolingCoilIndex);        

        CalcVarSpeedHeatPump(state,
                             FurnaceNum,
                             FirstHVACIteration,
                             compressorOp,
                             SpeedNum,
                             SpeedRatio,
                             PartLoadFrac,
                             FullOutput,
                             LatOutput,
                             QZnReq,
                             QLatReq,
                             OnOffAirFlowRatio,
                             SupHeaterLoad);

        if (QLatReq < (-1.0 * HVAC::SmallLoad)) { // dehumidification mode
            if (QLatReq <= LatOutput || (QZnReq < -HVAC::SmallLoad && QZnReq <= FullOutput) || (QZnReq > HVAC::SmallLoad && QZnReq >= FullOutput)) {
                PartLoadFrac = 1.0;
                SpeedRatio = 1.0;
                furnace.CompPartLoadRatio = PartLoadFrac;
                furnace.CompSpeedRatio = SpeedRatio;
                furnace.CompSpeedNum = SpeedNum;
                return;
            }
        } else if (QZnReq < -HVAC::SmallLoad) {
            if (QZnReq <= FullOutput) {
                PartLoadFrac = 1.0;
                SpeedRatio = 1.0;
                furnace.CompPartLoadRatio = PartLoadFrac;
                furnace.CompSpeedRatio = SpeedRatio;
                furnace.CompSpeedNum = SpeedNum;
                return;
            }
        } else {
            if (QZnReq >= FullOutput) {
                PartLoadFrac = 1.0;
                SpeedRatio = 1.0;
                // may need supplemental heating so don't return in heating mode
            }
        }

        if ((QZnReq < -HVAC::SmallLoad && NoCompOutput - QZnReq > HVAC::SmallLoad) ||
            (QZnReq > HVAC::SmallLoad && QZnReq - NoCompOutput > HVAC::SmallLoad)) {
            if ((QZnReq > HVAC::SmallLoad && QZnReq < FullOutput) || (QZnReq < (-1.0 * HVAC::SmallLoad) && QZnReq > FullOutput)) {
                // Check whether the low speed coil can meet the load or not
                CalcVarSpeedHeatPump(state,
                                     FurnaceNum,
                                     FirstHVACIteration,
                                     compressorOp,
                                     1,
                                     0.0,
                                     1.0,
                                     LowOutput,
                                     LatOutput,
                                     QZnReq,
                                     QLatReq,
                                     OnOffAirFlowRatio,
                                     SupHeaterLoad);
                if ((QZnReq > HVAC::SmallLoad && QZnReq <= LowOutput) || (QZnReq < (-HVAC::SmallLoad) && QZnReq >= LowOutput)) {
                    // Calculate the part load fraction
                    SpeedRatio = 0.0;
                    SpeedNum = 1;
                    auto f = // (AUTO_OK_LAMBDA)
                        [&state, FurnaceNum, FirstHVACIteration, QZnReq, OnOffAirFlowRatio, SupHeaterLoad, compressorOp](Real64 const PartLoadFrac) {
                            return VSHPCyclingResidual(
                                state, PartLoadFrac, FurnaceNum, FirstHVACIteration, QZnReq, OnOffAirFlowRatio, SupHeaterLoad, compressorOp, 1.0);
                        };
                    General::SolveRoot(state, ErrorToler, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                    if (SolFla == -1) {
                        if (!state.dataGlobal->WarmupFlag) {
                            if (furnace.ErrCountCyc == 0) {
                                ++furnace.ErrCountCyc;
                                ShowWarningError(
                                    state, format("Iteration limit exceeded calculating VS WSHP unit cycling ratio, for unit={}", furnace.Name));
                                ShowContinueErrorTimeStamp(state, format("Cycling ratio returned={:.2R}", PartLoadFrac));
                            } else {
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    furnace.Name + "\": Iteration limit warning exceeding calculating DX unit cycling ratio  continues...",
                                    furnace.ErrIndexCyc,
                                    PartLoadFrac,
                                    PartLoadFrac);
                            }
                        }
                    } else if (SolFla == -2) {
                        ShowFatalError(
                            state, format("VS WSHP unit cycling ratio calculation failed: cycling limits exceeded, for unit={}", furnace.Name));
                    }
                } else {
                    // Check to see which speed to meet the load
                    PartLoadFrac = 1.0;
                    SpeedRatio = 1.0;
                    if (QZnReq < -HVAC::SmallLoad) { // Cooling
                        for (i = 2; i <= furnace.NumOfSpeedCooling; ++i) {
                            CalcVarSpeedHeatPump(state,
                                                 FurnaceNum,
                                                 FirstHVACIteration,
                                                 compressorOp,
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
                        for (i = 2; i <= furnace.NumOfSpeedHeating; ++i) {
                            CalcVarSpeedHeatPump(state,
                                                 FurnaceNum,
                                                 FirstHVACIteration,
                                                 compressorOp,
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
                    auto f = [&state, FurnaceNum, FirstHVACIteration, QZnReq, OnOffAirFlowRatio, SupHeaterLoad, SpeedNum, compressorOp](
                                 Real64 const SpeedRatio) {
                        return VSHPSpeedResidual(
                            state, SpeedRatio, FurnaceNum, FirstHVACIteration, QZnReq, OnOffAirFlowRatio, SupHeaterLoad, SpeedNum, compressorOp, 1.0);
                    };
                    General::SolveRoot(state, ErrorToler, MaxIte, SolFla, SpeedRatio, f, 1.0e-10, 1.0);
                    if (SolFla == -1) {
                        if (!state.dataGlobal->WarmupFlag) {
                            if (furnace.ErrCountVar == 0) {
                                ++furnace.ErrCountVar;
                                ShowWarningError(
                                    state, format("Iteration limit exceeded calculating VS WSHP unit speed ratio, for unit={}", furnace.Name));
                                ShowContinueErrorTimeStamp(state, format("Speed ratio returned=[{:.2R}], Speed number ={}", SpeedRatio, SpeedNum));
                            } else {
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    furnace.Name + "\": Iteration limit warning exceeding calculating DX unit speed ratio continues...",
                                    furnace.ErrIndexVar,
                                    SpeedRatio,
                                    SpeedRatio);
                            }
                        }
                    } else if (SolFla == -2) {
                        ShowFatalError(
                            state, format("VS WSHP unit compressor speed calculation failed: speed limits exceeded, for unit={}", furnace.Name));
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
        if (QLatReq < -HVAC::SmallLoad && QLatReq < LatOutput) {
            PartLoadFrac = 1.0;
            SpeedRatio = 1.0;
            for (i = SpeedNum; i <= furnace.NumOfSpeedCooling; ++i) {
                CalcVarSpeedHeatPump(state,
                                     FurnaceNum,
                                     FirstHVACIteration,
                                     compressorOp,
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
            if (QLatReq - LatOutput > HVAC::SmallLoad) {
                if (SpeedNum < 2) {
                    auto f = // (AUTO_OK_LAMBDA)
                        [&state, FurnaceNum, FirstHVACIteration, QLatReq, OnOffAirFlowRatio, SupHeaterLoad, compressorOp](Real64 const PartLoadFrac) {
                            return VSHPCyclingResidual(
                                state, PartLoadFrac, FurnaceNum, FirstHVACIteration, QLatReq, OnOffAirFlowRatio, SupHeaterLoad, compressorOp, 0.0);
                        };
                    General::SolveRoot(state, ErrorToler, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                } else {
                    auto f = [&state, FurnaceNum, FirstHVACIteration, QLatReq, OnOffAirFlowRatio, SupHeaterLoad, SpeedNum, compressorOp](
                                 Real64 const SpeedRatio) {
                        return VSHPSpeedResidual(state,
                                                 SpeedRatio,
                                                 FurnaceNum,
                                                 FirstHVACIteration,
                                                 QLatReq,
                                                 OnOffAirFlowRatio,
                                                 SupHeaterLoad,
                                                 SpeedNum,
                                                 compressorOp,
                                                 0.0);
                    };
                    General::SolveRoot(state, ErrorToler, MaxIte, SolFla, SpeedRatio, f, 1.0e-10, 1.0);
                }
                if (SolFla == -1) {
                    if (!state.dataGlobal->WarmupFlag) {
                        if (furnace.ErrCountVar2 == 0) {
                            ++furnace.ErrCountVar2;
                            ShowWarningError(state,
                                             format("Iteration limit exceeded calculating VS WSHP unit speed ratio, for unit={}", furnace.Name));
                            ShowContinueErrorTimeStamp(state, format("Speed ratio returned=[{:.2R}], Speed number ={}", SpeedRatio, SpeedNum));
                        } else {
                            ShowRecurringWarningErrorAtEnd(state,
                                                           furnace.Name +
                                                               "\": Iteration limit warning exceeding calculating DX unit speed ratio continues...",
                                                           furnace.ErrIndexVar,
                                                           SpeedRatio,
                                                           SpeedRatio);
                        }
                    }
                } else if (SolFla == -2) {
                    ShowFatalError(state,
                                   format("VS WSHP unit compressor speed calculation failed: speed limits exceeded, for unit={}", furnace.Name));
                }
            }
        }
        // end meet the latent load

        // if the heating coil cannot meet the load, trim with supplemental heater
        // occurs with constant fan mode when compressor is on or off
        // occurs with cycling fan mode when compressor PLR is equal to 1
        if ((QZnReq > HVAC::SmallLoad && QZnReq > FullOutput) && (furnace.SuppHeatCoilNum != 0)) {
            PartLoadFrac = 1.0;
            SpeedRatio = 1.0;
            if (furnace.NumOfSpeedHeating > 0)
                SpeedNum = furnace.NumOfSpeedHeating; // maximum heating speed, avoid zero for cooling only mode

            if (state.dataEnvrn->OutDryBulbTemp <= furnace.MaxOATSuppHeat) {
                SupHeaterLoad = QZnReq - FullOutput;
            } else {
                SupHeaterLoad = 0.0;
            }
            CalcVarSpeedHeatPump(state,
                                 FurnaceNum,
                                 FirstHVACIteration,
                                 compressorOp,
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
        if (state.dataLoopNodes->Node(furnace.FurnaceOutletNode).Temp > furnace.DesignMaxOutletTemp && SupHeaterLoad > 0.0) {

            //   If the supply air temperature is to high, turn off the supplemental heater to recalculate the outlet temperature
            CalcNonDXHeatingCoils(state, FurnaceNum, true, FirstHVACIteration, 0.0, fanOp, QCoilActual);
            //   If the outlet temperature is below the maximum supplemental heater supply air temperature, reduce the load passed to
            //   the supplemental heater, otherwise leave the supplemental heater off. If the supplemental heater is to be turned on,
            //   use the outlet conditions when the supplemental heater was off (CALL above) as the inlet conditions for the calculation
            //   of supplemental heater load to just meet the maximum supply air temperature from the supplemental heater.
            if (state.dataLoopNodes->Node(furnace.FurnaceOutletNode).Temp < furnace.DesignMaxOutletTemp) {
                Real64 CpAir = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(furnace.FurnaceOutletNode).HumRat);
                SupHeaterLoad = state.dataLoopNodes->Node(furnace.FurnaceInletNode).MassFlowRate * CpAir *
                                (furnace.DesignMaxOutletTemp - state.dataLoopNodes->Node(furnace.FurnaceOutletNode).Temp);

            } else {
                SupHeaterLoad = 0.0;
            }
        }

        // prepare module level output
        furnace.CompPartLoadRatio = PartLoadFrac;
        furnace.CompSpeedRatio = SpeedRatio;
        furnace.CompSpeedNum = SpeedNum;
        furnace.CoolingCoilLatentDemand = std::abs(QLatReq);

        if (furnace.fanOp == HVAC::FanOp::Continuous) {
            furnace.FanPartLoadRatio = 1.0;
        } else {
            furnace.FanPartLoadRatio = PartLoadFrac;
        }
    }

    //******************************************************************************

    void CalcVarSpeedHeatPump(EnergyPlusData &state,
                              int const FurnaceNum,                  // Variable speed heat pump number
                              bool const FirstHVACIteration,         // Flag for 1st HVAC iteration
                              HVAC::CompressorOp const compressorOp, // Compressor on/off; 1=on, 0=off
                              int const SpeedNum,                    // Speed number
                              Real64 const SpeedRatio,               // Compressor speed ratio
                              Real64 const PartLoadFrac,             // Compressor part load fraction
                              Real64 &SensibleLoadMet,               // Sensible cooling load met (furnace outlet with respect to control zone temp)
                              Real64 &LatentLoadMet,     // Latent cooling load met (furnace outlet with respect to control zone humidity ratio)
                              Real64 const QZnReq,       // Zone load (W)
                              Real64 const QLatReq,      // Zone latent load []
                              Real64 &OnOffAirFlowRatio, // Ratio of compressor ON airflow to AVERAGE airflow over timestep
                              Real64 const SupHeaterLoad // supplemental heater load (W)
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Bo Shen, based on HVACMultiSpeedHeatPump:CalcMSHeatPump
        //       DATE WRITTEN:    March 2012

        // PURPOSE OF THIS SUBROUTINE:
        //  This routine will calculates MSHP performance based on given system load

        Real64 SavePartloadRatio = 0.0; // part-load ratio
        Real64 SaveSpeedRatio = 0.0;    // speed ratio
        Real64 QCoilActual = 0.0;       // coil load actually delivered returned to calling component
        Real64 HeatCoilLoad = 0.0;      // required heating coil load

        state.dataFurnaces->SaveCompressorPLR = 0.0;

        // Set inlet air mass flow rate based on PLR and compressor on/off air flow rates
        SetVSHPAirFlow(state, FurnaceNum, PartLoadFrac, OnOffAirFlowRatio, SpeedNum, SpeedRatio);

        auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);

        if ((SupHeaterLoad > 1.0e-10) && (furnace.type == HVAC::UnitarySysType::Unitary_HeatCool) && (furnace.SuppHeatCoilNum == 0)) {
            // ONLY HEATING COIL, NO SUPPLEMENTAL COIL, USED FOR REHEAT DURING DUHMI
            HeatCoilLoad = furnace.DesignHeatingCapacity * PartLoadFrac; // REHEAT IN FAN ON TIME

            if (HeatCoilLoad > SupHeaterLoad) {
                HeatCoilLoad = SupHeaterLoad; // HEATING COIL RUN TIME < FAN ON TIME
            }

        } else if ((QZnReq > HVAC::SmallLoad) && (furnace.type == HVAC::UnitarySysType::Unitary_HeatCool)) {
            HeatCoilLoad = furnace.DesignHeatingCapacity * PartLoadFrac;
        } else {
            HeatCoilLoad = 0.0;
        }

        Real64 AirMassFlow = state.dataLoopNodes->Node(furnace.FurnaceInletNode).MassFlowRate;
        // if blow through, simulate fan then coils
        if (furnace.fanPlace == HVAC::FanPlace::BlowThru) {
            state.dataFans->fans(furnace.FanIndex)->simulate(state, FirstHVACIteration, state.dataFurnaces->FanSpeedRatio);

            if ((!furnace.CoolCoilUpstream) && (furnace.type == HVAC::UnitarySysType::Unitary_HeatCool)) {
                // simulate thisFurnace heating coil
                bool SuppHeatingCoilFlag = false; // if true simulates supplemental heating coil
                CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, furnace.fanOp, QCoilActual);
            }

            if ((QZnReq < -HVAC::SmallLoad || (QLatReq < -HVAC::SmallLoad)) &&
                (state.dataEnvrn->OutDryBulbTemp >= furnace.MinOATCompressorCooling)) { // COOLING MODE or dehumidification mode

                if (furnace.isIHP) {
                    IntegratedHeatPump::SimIHP(state,
                                               BlankString,
                                               furnace.ihpNum,
                                               furnace.fanOp,
                                               compressorOp,
                                               PartLoadFrac,
                                               SpeedNum,
                                               SpeedRatio,
                                               QZnReq,
                                               QLatReq,
                                               false,
                                               false,
                                               OnOffAirFlowRatio);
                } else {
                    VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                              BlankString,
                                                              furnace.CoolCoilNum,
                                                              furnace.fanOp,
                                                              compressorOp,
                                                              PartLoadFrac,
                                                              SpeedNum,
                                                              SpeedRatio,
                                                              QZnReq,
                                                              QLatReq,
                                                              OnOffAirFlowRatio);
                }

                SavePartloadRatio = PartLoadFrac;
                SaveSpeedRatio = SpeedRatio;

                state.dataFurnaces->SaveCompressorPLR = state.dataVariableSpeedCoils->VarSpeedCoil(furnace.CoolCoilNum).PartLoadRatio;
            } else {
                if (furnace.isIHP) {
                    IntegratedHeatPump::SimIHP(state,
                                               BlankString,
                                               furnace.ihpNum,
                                               furnace.fanOp,
                                               compressorOp,
                                               PartLoadFrac,
                                               SpeedNum,
                                               SpeedRatio,
                                               QZnReq,
                                               QLatReq,
                                               false,
                                               false,
                                               OnOffAirFlowRatio);
                } else {
                    VariableSpeedCoils::SimVariableSpeedCoils(
                        state, BlankString, furnace.CoolCoilNum, furnace.fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRatio);
                }
            }

            if (furnace.type != HVAC::UnitarySysType::Unitary_HeatCool) {
                if ((QZnReq > HVAC::SmallLoad) && state.dataFurnaces->HeatingLoad) {
                    if (furnace.isIHP) {
                        IntegratedHeatPump::SimIHP(state,
                                                   BlankString,
                                                   furnace.ihpNum,
                                                   furnace.fanOp,
                                                   compressorOp,
                                                   PartLoadFrac,
                                                   SpeedNum,
                                                   SpeedRatio,
                                                   QZnReq,
                                                   QLatReq,
                                                   false,
                                                   false,
                                                   OnOffAirFlowRatio);
                    } else {
                        VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                  BlankString,
                                                                  furnace.HeatCoilNum,
                                                                  furnace.fanOp,
                                                                  compressorOp,
                                                                  PartLoadFrac,
                                                                  SpeedNum,
                                                                  SpeedRatio,
                                                                  QZnReq,
                                                                  QLatReq,
                                                                  OnOffAirFlowRatio);
                    }

                    SavePartloadRatio = PartLoadFrac;
                    SaveSpeedRatio = SpeedRatio;

                    state.dataFurnaces->SaveCompressorPLR = state.dataVariableSpeedCoils->VarSpeedCoil(furnace.HeatCoilNum).PartLoadRatio;
                } else {
                    if (furnace.isIHP) {
                        IntegratedHeatPump::SimIHP(state,
                                                   BlankString,
                                                   furnace.ihpNum,
                                                   furnace.fanOp,
                                                   compressorOp,
                                                   PartLoadFrac,
                                                   SpeedNum,
                                                   SpeedRatio,
                                                   QZnReq,
                                                   QLatReq,
                                                   false,
                                                   false,
                                                   OnOffAirFlowRatio);
                    } else {
                        VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                  BlankString,
                                                                  furnace.HeatCoilNum,
                                                                  furnace.fanOp,
                                                                  compressorOp,
                                                                  0.0,
                                                                  1,
                                                                  0.0,
                                                                  0.0,
                                                                  0.0,
                                                                  OnOffAirFlowRatio);
                    }
                }
            } else if (furnace.CoolCoilUpstream && (furnace.type == HVAC::UnitarySysType::Unitary_HeatCool)) {
                // simulate thisFurnace heating coil
                bool SuppHeatingCoilFlag = false; // if true simulates supplemental heating coil
                CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, furnace.fanOp, QCoilActual);
            }

            // Call twice to ensure the fan outlet conditions are updated
            state.dataFans->fans(furnace.FanIndex)->simulate(state, FirstHVACIteration, state.dataFurnaces->FanSpeedRatio);

            if ((!furnace.CoolCoilUpstream) && (furnace.type == HVAC::UnitarySysType::Unitary_HeatCool)) {
                // simulate thisFurnace heating coil
                bool SuppHeatingCoilFlag = false; // if true simulates supplemental heating coil
                CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, furnace.fanOp, QCoilActual);
            }

            if ((QZnReq < -HVAC::SmallLoad || (QLatReq < -HVAC::SmallLoad)) &&
                (state.dataEnvrn->OutDryBulbTemp >= furnace.MinOATCompressorCooling)) {

                if (furnace.isIHP) {
                    IntegratedHeatPump::SimIHP(state,
                                               BlankString,
                                               furnace.ihpNum,
                                               furnace.fanOp,
                                               compressorOp,
                                               PartLoadFrac,
                                               SpeedNum,
                                               SpeedRatio,
                                               QZnReq,
                                               QLatReq,
                                               false,
                                               false,
                                               OnOffAirFlowRatio);
                } else {
                    VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                              BlankString,
                                                              furnace.CoolCoilNum,
                                                              furnace.fanOp,
                                                              compressorOp,
                                                              PartLoadFrac,
                                                              SpeedNum,
                                                              SpeedRatio,
                                                              QZnReq,
                                                              QLatReq,
                                                              OnOffAirFlowRatio);
                }

                SavePartloadRatio = PartLoadFrac;
                SaveSpeedRatio = SpeedRatio;
                state.dataFurnaces->SaveCompressorPLR = state.dataVariableSpeedCoils->VarSpeedCoil(furnace.CoolCoilNum).PartLoadRatio;
            } else {

                if (furnace.isIHP) {
                    IntegratedHeatPump::SimIHP(state,
                                               BlankString,
                                               furnace.ihpNum,
                                               furnace.fanOp,
                                               compressorOp,
                                               PartLoadFrac,
                                               SpeedNum,
                                               SpeedRatio,
                                               QZnReq,
                                               QLatReq,
                                               false,
                                               false,
                                               OnOffAirFlowRatio);
                } else {
                    VariableSpeedCoils::SimVariableSpeedCoils(
                        state, BlankString, furnace.CoolCoilNum, furnace.fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRatio);
                }
            }

            if (furnace.type != HVAC::UnitarySysType::Unitary_HeatCool) {
                if ((QZnReq > HVAC::SmallLoad) && state.dataFurnaces->HeatingLoad) {
                    if (furnace.isIHP) {
                        IntegratedHeatPump::SimIHP(state,
                                                   BlankString,
                                                   furnace.ihpNum,
                                                   furnace.fanOp,
                                                   compressorOp,
                                                   PartLoadFrac,
                                                   SpeedNum,
                                                   SpeedRatio,
                                                   QZnReq,
                                                   QLatReq,
                                                   false,
                                                   false,
                                                   OnOffAirFlowRatio);
                    } else {
                        VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                  BlankString,
                                                                  furnace.HeatCoilNum,
                                                                  furnace.fanOp,
                                                                  compressorOp,
                                                                  PartLoadFrac,
                                                                  SpeedNum,
                                                                  SpeedRatio,
                                                                  QZnReq,
                                                                  QLatReq,
                                                                  OnOffAirFlowRatio);
                    }

                    SavePartloadRatio = PartLoadFrac;
                    SaveSpeedRatio = SpeedRatio;
                    state.dataFurnaces->SaveCompressorPLR = state.dataVariableSpeedCoils->VarSpeedCoil(furnace.HeatCoilNum).PartLoadRatio;
                } else {
                    if (furnace.isIHP) {
                        IntegratedHeatPump::SimIHP(state,
                                                   BlankString,
                                                   furnace.ihpNum,
                                                   furnace.fanOp,
                                                   compressorOp,
                                                   PartLoadFrac,
                                                   SpeedNum,
                                                   SpeedRatio,
                                                   QZnReq,
                                                   QLatReq,
                                                   false,
                                                   false,
                                                   OnOffAirFlowRatio);
                    } else {
                        VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                  BlankString,
                                                                  furnace.HeatCoilNum,
                                                                  furnace.fanOp,
                                                                  compressorOp,
                                                                  0.0,
                                                                  1,
                                                                  0.0,
                                                                  0.0,
                                                                  0.0,
                                                                  OnOffAirFlowRatio);
                    }
                }
            } else if (furnace.CoolCoilUpstream && (furnace.type == HVAC::UnitarySysType::Unitary_HeatCool)) {
                // simulate thisFurnace heating coil
                bool SuppHeatingCoilFlag = false; // if true simulates supplemental heating coil
                CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, furnace.fanOp, QCoilActual);
            }

            //  Simulate supplemental heating coil for blow through fan
            if (furnace.SuppHeatCoilNum > 0) {
                bool SuppHeatingCoilFlag = true; // if true simulates supplemental heating coil
                CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, SupHeaterLoad, furnace.fanOp, QCoilActual);
            }
        } else { // otherwise simulate DX coils then fan then supplemental heater

            if ((!furnace.CoolCoilUpstream) && (furnace.type == HVAC::UnitarySysType::Unitary_HeatCool)) {
                // simulate thisFurnace heating coil
                bool SuppHeatingCoilFlag = false; // if true simulates supplemental heating coil
                CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, furnace.fanOp, QCoilActual);
            }

            if ((QZnReq < -HVAC::SmallLoad || (QLatReq < -HVAC::SmallLoad)) &&
                (state.dataEnvrn->OutDryBulbTemp >= furnace.MinOATCompressorCooling)) {

                if (furnace.isIHP) {
                    IntegratedHeatPump::SimIHP(state,
                                               BlankString,
                                               furnace.ihpNum,
                                               furnace.fanOp,
                                               compressorOp,
                                               PartLoadFrac,
                                               SpeedNum,
                                               SpeedRatio,
                                               QZnReq,
                                               QLatReq,
                                               false,
                                               false,
                                               OnOffAirFlowRatio);
                } else {
                    VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                              BlankString,
                                                              furnace.CoolCoilNum,
                                                              furnace.fanOp,
                                                              compressorOp,
                                                              PartLoadFrac,
                                                              SpeedNum,
                                                              SpeedRatio,
                                                              QZnReq,
                                                              QLatReq,
                                                              OnOffAirFlowRatio);
                }

                SavePartloadRatio = PartLoadFrac;
                SaveSpeedRatio = SpeedRatio;

                state.dataFurnaces->SaveCompressorPLR = state.dataVariableSpeedCoils->VarSpeedCoil(furnace.CoolCoilNum).PartLoadRatio;
            } else {
                if (furnace.isIHP) {
                    IntegratedHeatPump::SimIHP(state,
                                               BlankString,
                                               furnace.ihpNum,
                                               furnace.fanOp,
                                               compressorOp,
                                               PartLoadFrac,
                                               SpeedNum,
                                               SpeedRatio,
                                               QZnReq,
                                               QLatReq,
                                               false,
                                               false,
                                               OnOffAirFlowRatio);
                } else {
                    VariableSpeedCoils::SimVariableSpeedCoils(
                        state, BlankString, furnace.CoolCoilNum, furnace.fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRatio);
                }
            }

            if (furnace.type != HVAC::UnitarySysType::Unitary_HeatCool) {
                if (QZnReq > HVAC::SmallLoad && (state.dataEnvrn->OutDryBulbTemp >= furnace.MinOATCompressorCooling)) {

                    if (furnace.isIHP) {
                        IntegratedHeatPump::SimIHP(state,
                                                   BlankString,
                                                   furnace.ihpNum,
                                                   furnace.fanOp,
                                                   compressorOp,
                                                   PartLoadFrac,
                                                   SpeedNum,
                                                   SpeedRatio,
                                                   QZnReq,
                                                   QLatReq,
                                                   false,
                                                   false,
                                                   OnOffAirFlowRatio);
                    } else {
                        VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                  BlankString,
                                                                  furnace.HeatCoilNum,
                                                                  furnace.fanOp,
                                                                  compressorOp,
                                                                  PartLoadFrac,
                                                                  SpeedNum,
                                                                  SpeedRatio,
                                                                  QZnReq,
                                                                  QLatReq,
                                                                  OnOffAirFlowRatio);
                    }

                    SavePartloadRatio = PartLoadFrac;
                    SaveSpeedRatio = SpeedRatio;
                    state.dataFurnaces->SaveCompressorPLR = state.dataVariableSpeedCoils->VarSpeedCoil(furnace.HeatCoilNum).PartLoadRatio;
                } else {
                    if (furnace.isIHP) {
                        IntegratedHeatPump::SimIHP(state,
                                                   BlankString,
                                                   furnace.ihpNum,
                                                   furnace.fanOp,
                                                   compressorOp,
                                                   PartLoadFrac,
                                                   SpeedNum,
                                                   SpeedRatio,
                                                   QZnReq,
                                                   QLatReq,
                                                   false,
                                                   false,
                                                   OnOffAirFlowRatio);
                    } else {
                        VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                  BlankString,
                                                                  furnace.HeatCoilNum,
                                                                  furnace.fanOp,
                                                                  compressorOp,
                                                                  0.0,
                                                                  1,
                                                                  0.0,
                                                                  0.0,
                                                                  0.0,
                                                                  OnOffAirFlowRatio);
                    }
                }
            } else if (furnace.CoolCoilUpstream && (furnace.type == HVAC::UnitarySysType::Unitary_HeatCool)) {
                // simulate thisFurnace heating coil
                bool SuppHeatingCoilFlag = false; // if true simulates supplemental heating coil
                CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, HeatCoilLoad, furnace.fanOp, QCoilActual);
            }

            state.dataFans->fans(furnace.FanIndex)->simulate(state, FirstHVACIteration, state.dataFurnaces->FanSpeedRatio);
            //  Simulate supplemental heating coil for draw through fan
            if (furnace.SuppHeatCoilNum > 0) {
                bool SuppHeatingCoilFlag = true; // if true simulates supplemental heating coil
                CalcNonDXHeatingCoils(state, FurnaceNum, SuppHeatingCoilFlag, FirstHVACIteration, SupHeaterLoad, furnace.fanOp, QCoilActual);
            }
        }

        // If the fan runs continually do not allow coils to set OnOffFanPartLoadRatio.
        if (furnace.fanOp == HVAC::FanOp::Continuous) state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;

        auto &outNode = state.dataLoopNodes->Node(furnace.FurnaceOutletNode);
        auto &zoneNode = state.dataLoopNodes->Node(furnace.NodeNumOfControlledZone);
        Real64 zoneEnthalpy = Psychrometrics::PsyHFnTdbW(zoneNode.Temp, zoneNode.HumRat);
        Real64 outletEnthalpy = Psychrometrics::PsyHFnTdbW(outNode.Temp, outNode.HumRat);
        Real64 totalLoadMet = AirMassFlow * (outletEnthalpy - zoneEnthalpy);
        SensibleLoadMet =
            AirMassFlow * Psychrometrics::PsyDeltaHSenFnTdb2W2Tdb1W1(outNode.Temp, outNode.HumRat, zoneNode.Temp, zoneNode.HumRat); // sensible {W};
        LatentLoadMet = totalLoadMet - SensibleLoadMet;
        furnace.LatentLoadMet = LatentLoadMet;
    }

    //******************************************************************************

    Real64 VSHPCyclingResidual(EnergyPlusData &state,
                               Real64 const PartLoadFrac, // compressor cycling ratio (1.0 is continuous, 0.0 is off)
                               int FurnaceNum,
                               // int ZoneNum,
                               bool FirstHVACIteration,
                               // int fanOp,
                               Real64 LoadToBeMet,
                               Real64 OnOffAirFlowRatio,
                               Real64 SupHeaterLoad,
                               HVAC::CompressorOp compressorOp,
                               Real64 par9_SensLatFlag)
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Bo Shen, based on HVACMultiSpeedHeatPump:MSHPCyclingResidual
        //       DATE WRITTEN   March, 2012

        // PURPOSE OF THIS FUNCTION:
        //  Calculates residual function ((ActualOutput - QZnReq)/QZnReq)
        //  MSHP output depends on the part load ratio which is being varied to zero the residual.

        // METHODOLOGY EMPLOYED:
        //  Calls CalcMSHeatPump to get ActualOutput at the given part load ratio
        //  and calculates the residual as defined above

        // int FurnaceNum = int(Par[0]);
        // int ZoneNum = int(Par[1]);
        // bool FirstHVACIteration = (Par[2] == 1.0);
        // int fanOp = int(Par[3]);
        // Real64 LoadToBeMet = Par[4];
        // Real64 OnOffAirFlowRatio = Par[5];
        // Real64 SupHeaterLoad = Par[6];
        // CompressorOperation CompressorOp = static_cast<CompressorOperation>(Par[8]);
        // Real64 par9_SensLatFlag = Par[9];

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 ZoneSensLoadMet; // delivered sensible capacity of MSHP
        Real64 ZoneLatLoadMet;  // delivered latent capacity of MSHP

        Real64 QZnReq = 0.0;
        Real64 QZnLat = 0.0;
        if (par9_SensLatFlag == 1.0) {
            QZnReq = LoadToBeMet;
        } else {
            QZnLat = LoadToBeMet;
        }

        CalcVarSpeedHeatPump(state,
                             FurnaceNum,
                             FirstHVACIteration,
                             compressorOp,
                             1,
                             0.0,
                             PartLoadFrac,
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
        }
        return (ZoneLatLoadMet - LoadToBeMet) / ResScale;
    }

    //******************************************************************************

    Real64 VSHPSpeedResidual(EnergyPlusData &state,
                             Real64 const SpeedRatio, // compressor cycling ratio (1.0 is continuous, 0.0 is off)
                             int FurnaceNum,
                             // int ZoneNum,
                             bool FirstHVACIteration,
                             // int fanOp
                             Real64 LoadToBeMet,
                             Real64 OnOffAirFlowRatio,
                             Real64 SupHeaterLoad,
                             int SpeedNum,
                             HVAC::CompressorOp compressorOp,
                             Real64 par9_SensLatFlag)
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Bo Shen, , based on HVACMultiSpeedHeatPump:MSHPVarSpeedgResidual
        //       DATE WRITTEN   March, 2012

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
                             compressorOp,
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
        }
        return (ZoneLatLoadMet - LoadToBeMet) / ResScale;
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

        // PURPOSE OF THIS SUBROUTINE:
        // Set the average air mass flow rates using the part load fraction of the heat pump for this time step
        // Set OnOffAirFlowRatio to be used by DX coils

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 AverageUnitMassFlow; // average supply air mass flow rate over time step

        auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);

        state.dataHVACGlobal->MSHPMassFlowRateLow = 0.0;  // Mass flow rate at low speed
        state.dataHVACGlobal->MSHPMassFlowRateHigh = 0.0; // Mass flow rate at high speed

        if (furnace.fanOp == HVAC::FanOp::Continuous) {
            state.dataFurnaces->CompOffMassFlow = furnace.IdleMassFlowRate;
            state.dataFurnaces->CompOffFlowRatio = furnace.IdleSpeedRatio;
        } else {
            state.dataFurnaces->CompOffMassFlow = 0.0;
            state.dataFurnaces->CompOffFlowRatio = 0.0;
        }

        if (state.dataFurnaces->CoolingLoad && (furnace.type == HVAC::UnitarySysType::Unitary_HeatCool)) {
            if (furnace.NumOfSpeedCooling > 0) {
                state.dataFurnaces->CompOnMassFlow = furnace.CoolMassFlowRate(furnace.NumOfSpeedCooling);
                state.dataFurnaces->CompOnFlowRatio = furnace.MSCoolingSpeedRatio(furnace.NumOfSpeedCooling);
                state.dataHVACGlobal->MSHPMassFlowRateLow = furnace.CoolMassFlowRate(furnace.NumOfSpeedCooling);
                state.dataHVACGlobal->MSHPMassFlowRateHigh = furnace.CoolMassFlowRate(furnace.NumOfSpeedCooling);
            } else {
                state.dataFurnaces->CompOnMassFlow = furnace.MaxCoolAirMassFlow;
                state.dataFurnaces->CompOnFlowRatio = furnace.CoolingSpeedRatio;
            }
            AverageUnitMassFlow = (PartLoadRatio * state.dataFurnaces->CompOnMassFlow) + ((1 - PartLoadRatio) * state.dataFurnaces->CompOffMassFlow);
            if (state.dataFurnaces->CompOffFlowRatio > 0.0) {
                state.dataFurnaces->FanSpeedRatio =
                    (PartLoadRatio * state.dataFurnaces->CompOnFlowRatio) + ((1 - PartLoadRatio) * state.dataFurnaces->CompOffFlowRatio);
            } else {
                state.dataFurnaces->FanSpeedRatio = state.dataFurnaces->CompOnFlowRatio;
            }
        } else if (state.dataFurnaces->HeatingLoad && (furnace.type == HVAC::UnitarySysType::Unitary_HeatCool)) {
            if (furnace.NumOfSpeedHeating > 0) {
                state.dataFurnaces->CompOnMassFlow = furnace.HeatMassFlowRate(furnace.NumOfSpeedHeating);
                state.dataFurnaces->CompOnFlowRatio = furnace.MSHeatingSpeedRatio(furnace.NumOfSpeedHeating);
                state.dataHVACGlobal->MSHPMassFlowRateLow = furnace.HeatMassFlowRate(furnace.NumOfSpeedHeating);
                state.dataHVACGlobal->MSHPMassFlowRateHigh = furnace.HeatMassFlowRate(furnace.NumOfSpeedHeating);
            } else {
                state.dataFurnaces->CompOnMassFlow = furnace.MaxHeatAirMassFlow;
                state.dataFurnaces->CompOnFlowRatio = furnace.HeatingSpeedRatio;
            }
            AverageUnitMassFlow = (PartLoadRatio * state.dataFurnaces->CompOnMassFlow) + ((1 - PartLoadRatio) * state.dataFurnaces->CompOffMassFlow);
            if (state.dataFurnaces->CompOffFlowRatio > 0.0) {
                state.dataFurnaces->FanSpeedRatio =
                    (PartLoadRatio * state.dataFurnaces->CompOnFlowRatio) + ((1 - PartLoadRatio) * state.dataFurnaces->CompOffFlowRatio);
            } else {
                state.dataFurnaces->FanSpeedRatio = state.dataFurnaces->CompOnFlowRatio;
            }
        } else if (furnace.isIHP) {
            if (!state.dataZoneEnergyDemand->CurDeadBandOrSetback(furnace.ControlZoneNum) && present(SpeedNum)) {
                // if(present(SpeedNum)) {
                state.dataFurnaces->CompOnMassFlow =
                    IntegratedHeatPump::GetAirMassFlowRateIHP(state, furnace.ihpNum, SpeedNum, SpeedRatio, false);
                state.dataFurnaces->CompOnFlowRatio =
                    state.dataFurnaces->CompOnMassFlow /
                    IntegratedHeatPump::GetAirMassFlowRateIHP(
                        state, furnace.ihpNum, IntegratedHeatPump::GetMaxSpeedNumIHP(state, furnace.ihpNum), 1.0, false);
                state.dataHVACGlobal->MSHPMassFlowRateLow =
                    IntegratedHeatPump::GetAirMassFlowRateIHP(state, furnace.ihpNum, SpeedNum, 0.0, false);
                state.dataHVACGlobal->MSHPMassFlowRateHigh =
                    IntegratedHeatPump::GetAirMassFlowRateIHP(state, furnace.ihpNum, SpeedNum, 1.0, false);
            }

            // Set up fan flow rate during compressor off time
            if (furnace.fanOp == HVAC::FanOp::Continuous && present(SpeedNum)) {
                if (furnace.AirFlowControl == AirFlowControlConstFan::UseCompressorOnFlow && state.dataFurnaces->CompOnMassFlow > 0.0) {
                    state.dataFurnaces->CompOffMassFlow =
                        IntegratedHeatPump::GetAirMassFlowRateIHP(state, furnace.ihpNum, SpeedNum, 1.0, false);
                    state.dataFurnaces->CompOffFlowRatio =
                        state.dataFurnaces->CompOffMassFlow /
                        IntegratedHeatPump::GetAirMassFlowRateIHP(state,
                                                                  furnace.ihpNum,
                                                                  IntegratedHeatPump::GetMaxSpeedNumIHP(state, furnace.ihpNum),
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

            if (IntegratedHeatPump::IHPOperationMode::SCWHMatchWH ==
                state.dataIntegratedHP->IntegratedHeatPumps(furnace.ihpNum).CurMode) {
                state.dataFurnaces->CompOnMassFlow =
                    IntegratedHeatPump::GetAirMassFlowRateIHP(state, furnace.ihpNum, SpeedNum, SpeedRatio, false);
                AverageUnitMassFlow = state.dataFurnaces->CompOnMassFlow;
            }
        } else {
            if (!state.dataZoneEnergyDemand->CurDeadBandOrSetback(furnace.ControlZoneNum) && present(SpeedNum)) {
                if (furnace.HeatCoolMode == Furnaces::ModeOfOperation::HeatingMode) {
                    if (SpeedNum == 1) {
                        state.dataFurnaces->CompOnMassFlow = furnace.HeatMassFlowRate(SpeedNum);
                        state.dataFurnaces->CompOnFlowRatio = furnace.MSHeatingSpeedRatio(SpeedNum);
                        state.dataHVACGlobal->MSHPMassFlowRateLow = furnace.HeatMassFlowRate(1);
                        state.dataHVACGlobal->MSHPMassFlowRateHigh = furnace.HeatMassFlowRate(1);
                    } else if (SpeedNum > 1) {
                        state.dataFurnaces->CompOnMassFlow =
                            SpeedRatio * furnace.HeatMassFlowRate(SpeedNum) + (1.0 - SpeedRatio) * furnace.HeatMassFlowRate(SpeedNum - 1);
                        state.dataFurnaces->CompOnFlowRatio = SpeedRatio * furnace.MSHeatingSpeedRatio(SpeedNum) +
                                                              (1.0 - SpeedRatio) * furnace.MSHeatingSpeedRatio(SpeedNum - 1);
                        state.dataHVACGlobal->MSHPMassFlowRateLow = furnace.HeatMassFlowRate(SpeedNum - 1);
                        state.dataHVACGlobal->MSHPMassFlowRateHigh = furnace.HeatMassFlowRate(SpeedNum);
                    }
                } else if (furnace.HeatCoolMode == Furnaces::ModeOfOperation::CoolingMode) {
                    if (SpeedNum == 1) {
                        state.dataFurnaces->CompOnMassFlow = furnace.CoolMassFlowRate(SpeedNum);
                        state.dataFurnaces->CompOnFlowRatio = furnace.MSCoolingSpeedRatio(SpeedNum);
                        state.dataHVACGlobal->MSHPMassFlowRateLow = furnace.CoolMassFlowRate(1);
                        state.dataHVACGlobal->MSHPMassFlowRateHigh = furnace.CoolMassFlowRate(1);
                    } else if (SpeedNum > 1) {
                        state.dataFurnaces->CompOnMassFlow =
                            SpeedRatio * furnace.CoolMassFlowRate(SpeedNum) + (1.0 - SpeedRatio) * furnace.CoolMassFlowRate(SpeedNum - 1);
                        state.dataFurnaces->CompOnFlowRatio = SpeedRatio * furnace.MSCoolingSpeedRatio(SpeedNum) +
                                                              (1.0 - SpeedRatio) * furnace.MSCoolingSpeedRatio(SpeedNum - 1);
                        state.dataHVACGlobal->MSHPMassFlowRateLow = furnace.CoolMassFlowRate(SpeedNum - 1);
                        state.dataHVACGlobal->MSHPMassFlowRateHigh = furnace.CoolMassFlowRate(SpeedNum);
                    }
                }
            }

            // Set up fan flow rate during compressor off time
            if (furnace.fanOp == HVAC::FanOp::Continuous && present(SpeedNum)) {
                if (furnace.AirFlowControl == AirFlowControlConstFan::UseCompressorOnFlow && state.dataFurnaces->CompOnMassFlow > 0.0) {
                    if (SpeedNum == 1) { // LOWEST SPEED USE IDLE FLOW
                        state.dataFurnaces->CompOffMassFlow = furnace.IdleMassFlowRate;
                        state.dataFurnaces->CompOffFlowRatio = furnace.IdleSpeedRatio;
                    } else if (furnace.LastMode == Furnaces::ModeOfOperation::HeatingMode) {
                        state.dataFurnaces->CompOffMassFlow = furnace.HeatMassFlowRate(SpeedNum);
                        state.dataFurnaces->CompOffFlowRatio = furnace.MSHeatingSpeedRatio(SpeedNum);
                    } else {
                        state.dataFurnaces->CompOffMassFlow = furnace.CoolMassFlowRate(SpeedNum);
                        state.dataFurnaces->CompOffFlowRatio = furnace.MSCoolingSpeedRatio(SpeedNum);
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

        if ((furnace.availSched->getCurrentVal() == 0.0) || state.dataHVACGlobal->TurnFansOff ||
            (furnace.fanAvailSched->getCurrentVal() == 0.0 && !state.dataHVACGlobal->TurnFansOn)) {
            state.dataLoopNodes->Node(furnace.FurnaceInletNode).MassFlowRate = 0.0;
            OnOffAirFlowRatio = 0.0;
        } else {
            state.dataLoopNodes->Node(furnace.FurnaceInletNode).MassFlowRate = AverageUnitMassFlow;
            state.dataLoopNodes->Node(furnace.FurnaceInletNode).MassFlowRateMaxAvail = AverageUnitMassFlow;
            if (AverageUnitMassFlow > 0.0) {
                OnOffAirFlowRatio = state.dataFurnaces->CompOnMassFlow / AverageUnitMassFlow;
            } else {
                OnOffAirFlowRatio = 0.0;
            }
        }

        state.dataLoopNodes->Node(furnace.FurnaceOutletNode).MassFlowRate =
            state.dataLoopNodes->Node(furnace.FurnaceInletNode).MassFlowRate;
    }

    void SetMinOATCompressor(EnergyPlusData &state,
                             int const FurnaceNum,                    // index to furnace
                             std::string const &cCurrentModuleObject, // type of furnace
                             bool &ErrorsFound                        // GetInput logical that errors were found
    )
    {
        bool errFlag = false;
        auto &furnace = state.dataFurnaces->Furnaces(FurnaceNum);

        // Set minimum OAT for heat pump compressor operation in heating mode

        // If this is an HXAssistedCoil then CoolCoilNum is already the index of the embedded coil
        if (furnace.coolCoilType == HVAC::CoilType::CoolingDXSingleSpeed) {
            furnace.MinOATCompressorCooling = DXCoils::GetCoilMinOATCompressor(state, furnace.CoolCoilNum);

        } else if (furnace.coolCoilType == HVAC::CoilType::CoolingDXHXAssisted) {
            if (furnace.childCoolCoilType == HVAC::CoilType::CoolingDX) {
                auto const &dxCoil = state.dataCoilCoolingDX->coilCoolingDXs[furnace.childCoolCoilNum];
                furnace.MinOATCompressorCooling = dxCoil.performance->minOutdoorDrybulb;
            } else if (furnace.childCoolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
                furnace.MinOATCompressorCooling = VariableSpeedCoils::GetCoilMinOATCompressor(state, furnace.childCoolCoilNum);
            } else { 
                furnace.MinOATCompressorCooling = DXCoils::GetCoilMinOATCompressor(state, furnace.childCoolCoilNum);
            }
            
        } else if (furnace.coolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
            furnace.MinOATCompressorCooling = VariableSpeedCoils::GetCoilMinOATCompressor(state, furnace.CoolCoilNum);
        } else {
            furnace.MinOATCompressorCooling = -1000.0;
        }
        if (errFlag) {
            ShowContinueError(state, format("...occurs in {} = {}", cCurrentModuleObject, furnace.Name));
            ErrorsFound = true;
        }

        // Set minimum OAT for heat pump compressor operation in heating mode
        errFlag = false;
        if (furnace.heatCoilType == HVAC::CoilType::HeatingDXVariableSpeed) {
            furnace.MinOATCompressorHeating = VariableSpeedCoils::GetCoilMinOATCompressor(state, furnace.HeatCoilNum);
        } else if (furnace.heatCoilType == HVAC::CoilType::HeatingDXSingleSpeed) {
            furnace.MinOATCompressorHeating = DXCoils::GetCoilMinOATCompressor(state, furnace.HeatCoilNum);
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

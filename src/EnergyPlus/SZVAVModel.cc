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
#include <cstdlib>

// ObjexxFCL Headers
#include <ObjexxFCL/string.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/SZVAVModel.hh>
#include <EnergyPlus/SizingManager.hh>
#include <EnergyPlus/UnitarySystem.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace SZVAVModel {

    // Module containing routines for general use

    // Data
    // This module should not contain variables in the module sense as it is
    // intended strictly to provide "interfaces" to routines used by other
    // parts of the simulation.

    // MODULE PARAMETER DEFINITIONS

    // Functions

    void calcSZVAVModel(EnergyPlusData &state,
                        FanCoilUnits::FanCoilData &SZVAVModel,
                        int const SysIndex,
                        bool const FirstHVACIteration,
                        bool const CoolingLoad,
                        bool const HeatingLoad,
                        Real64 const ZoneLoad,
                        [[maybe_unused]] Real64 &OnOffAirFlowRatio,
                        [[maybe_unused]] bool const HXUnitOn,
                        [[maybe_unused]] int const AirLoopNum,
                        Real64 &PartLoadRatio,
                        [[maybe_unused]] HVAC::CompressorOp const CompressorONFlag)
    {

        int constexpr MaxIter(100); // maximum number of iterations
        int SolFlag(0);             // return flag from RegulaFalsi for sensible load
        std::string MessagePrefix;  // label for warning reporting

        Real64 lowBoundaryLoad(0.0);
        Real64 highBoundaryLoad(0.0);
        Real64 minHumRat(0.0);
        Real64 outletTemp(0.0);
        bool coilActive(false);
        Real64 AirMassFlow(0.0);

        Real64 maxCoilFluidFlow(0.0);
        Real64 maxOutletTemp(0.0);
        Real64 minAirMassFlow(0.0);
        Real64 maxAirMassFlow(0.0);
        Real64 lowSpeedFanRatio(0.0);
        int coilFluidInletNode(0);
        int coilFluidOutletNode(0);
        PlantLocation coilPlantLoc{};
        int coilAirInletNode(0);
        int coilAirOutletNode(0);

        Real64 TempSensOutput; // iterative sensible capacity [W]
                               //        Real64 TempLatOutput; // iterative latent capacity [W]

        // set up mode specific variables to use in common function calls
        if (CoolingLoad) {
            maxCoilFluidFlow = SZVAVModel.MaxCoolCoilFluidFlow;
            maxOutletTemp = SZVAVModel.DesignMinOutletTemp;
            minAirMassFlow = SZVAVModel.MaxNoCoolHeatAirMassFlow;
            maxAirMassFlow = SZVAVModel.MaxCoolAirMassFlow;
            lowSpeedFanRatio = SZVAVModel.LowSpeedCoolFanRatio;
            coilFluidInletNode = SZVAVModel.CoolCoilFluidInletNode;
            coilFluidOutletNode = SZVAVModel.CoolCoilFluidOutletNodeNum;
            coilPlantLoc = SZVAVModel.CoolCoilPlantLoc;
            coilAirInletNode = SZVAVModel.CoolCoilInletNodeNum;
            coilAirOutletNode = SZVAVModel.CoolCoilOutletNodeNum;
        } else if (HeatingLoad) {
            maxCoilFluidFlow = SZVAVModel.MaxHeatCoilFluidFlow;
            maxOutletTemp = SZVAVModel.DesignMaxOutletTemp;
            minAirMassFlow = SZVAVModel.MaxNoCoolHeatAirMassFlow;
            maxAirMassFlow = SZVAVModel.MaxHeatAirMassFlow;
            lowSpeedFanRatio = SZVAVModel.LowSpeedHeatFanRatio;
            coilFluidInletNode = SZVAVModel.HeatCoilFluidInletNode;
            coilFluidOutletNode = SZVAVModel.HeatCoilFluidOutletNodeNum;
            coilPlantLoc = SZVAVModel.HeatCoilPlantLoc;
            coilAirInletNode = SZVAVModel.HeatCoilInletNodeNum;
            coilAirOutletNode = SZVAVModel.HeatCoilOutletNodeNum;
        } else { // should never get here, protect against uninitialized variables
            maxCoilFluidFlow = 0.0;
            maxOutletTemp = 0.0;
            minAirMassFlow = 0.0;
            maxAirMassFlow = 0.0;
            lowSpeedFanRatio = 0.0;
            coilFluidInletNode = 0;
            coilFluidOutletNode = 0;
            coilPlantLoc = {0, DataPlant::LoopSideLocation::Invalid, 0, 0};
            coilAirInletNode = 0;
            coilAirOutletNode = 0;
        }
        int InletNode = SZVAVModel.AirInNode;
        Real64 InletTemp = state.dataLoopNodes->Node(InletNode).Temp;
        int OutletNode = SZVAVModel.AirOutNode;
        Real64 ZoneTemp = state.dataLoopNodes->Node(SZVAVModel.NodeNumOfControlledZone).Temp;
        Real64 ZoneHumRat = state.dataLoopNodes->Node(SZVAVModel.NodeNumOfControlledZone).HumRat;
        // initialize flow variables to 0
        Real64 lowWaterMdot = 0.0;
        // Real64 SupHeaterLoad = 0.0;

        // model attempts to control air flow rate and coil capacity in specific operating regions:
        // Region 1 (R1) - minimum air flow rate at modulated coil capacity (up to min/max temperature limits)
        // Region 2 (R2) - modulated air flow rate and coil capacity (up to max air flow rate while maintaining min/max temperature limits)
        // Region 3 (R3) - maximum air flow rate and modulated/increased coil capacity (allow increased capacity at full air flow rate to meet
        // remaining load)
        //
        //                |    |                   |    |    ^            ^ = supply air temperature
        //                |    |                   |    | ^               * = supply air flow rate
        //                |    |                   |^^^^| <--- maximum supply air temperature
        //                |    |                ^  |    |
        //                |    |              ^    |    |
        //     ***********|    |            ^      |    |**************   <-- max unit air flow rate
        //                |*   |          ^        |   *|
        //                | *  |        ^          |  * |
        //                |  * |      ^            | *  |
        //                |   *|    ^              |*   |
        //                |    |*******************|    |                 <-- min unit air flow rate
        //          R3    | R2 | ^       R1        | R2 |    R3
        //   min SAT -->  |^^^^|                   |    |
        //               ^
        //             ^                  |
        //  (-) increasing cooling load < 0 > increasing heating load (+)
        //
        // Notes: SAT is allowed to decrease/increase once air flow rate is max'ed out otherwise load would not be met
        //        Load here is not the zone load, it's the load the system must meet to meet the Tstat set point (i.e., OA can alter required
        //        capacity) lowSpeedFanRatio = min/max unit air flow rate
        //
        // Step 1: calculate load at Region 1 lower (cooling) or upper (heating) boundary at minimum air flow rate
        //         - if load can be met, test maximum output (PLR = 1) before calling RootSolver
        //         - if maximum capacity is greater than load, solve for PLR
        // Step 2: calculate load at Region 3 lower (cooling) or upper (heating) boundary at maximum air flow rate
        //         - if load is less than boundary load, solve for air flow and PLR that meet the load
        //         - ELSE
        // Step 3: solve for Region 3 PLR
        //       DONE
        //

        // Step 1: Determine boundary for region 1
        // calculate sensible load based on minimum air flow rate and specified supply air temperature limit
        if (SZVAVModel.ATMixerExists) {
            if (SZVAVModel.ATMixerType == HVAC::MixerType::SupplySide) {
                // Air terminal supply side mixer
                lowBoundaryLoad =
                    minAirMassFlow * (Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(SZVAVModel.ATMixerOutNode).Temp, ZoneHumRat) -
                                      Psychrometrics::PsyHFnTdbW(ZoneTemp, ZoneHumRat));
            } else {
                // Air terminal inlet side mixer
                lowBoundaryLoad =
                    minAirMassFlow * (Psychrometrics::PsyHFnTdbW(maxOutletTemp, ZoneHumRat) - Psychrometrics::PsyHFnTdbW(ZoneTemp, ZoneHumRat));
            }
        } else {
            minHumRat = min(state.dataLoopNodes->Node(InletNode).HumRat, state.dataLoopNodes->Node(OutletNode).HumRat);
            lowBoundaryLoad =
                minAirMassFlow * (Psychrometrics::PsyHFnTdbW(maxOutletTemp, minHumRat) - Psychrometrics::PsyHFnTdbW(InletTemp, minHumRat));
        }

        if ((CoolingLoad && lowBoundaryLoad < ZoneLoad) || (HeatingLoad && lowBoundaryLoad > ZoneLoad)) { // in Region 1 of figure
            // Step 1: set min air flow and full coil capacity
            PartLoadRatio = 1.0; // full coil capacity
            SZVAVModel.FanPartLoadRatio =
                0.0; // minimum fan PLR, air flow = ( fanPartLoadRatio * maxAirMassFlow ) + ( ( 1.0 - fanPartLoadRatio ) * minAirMassFlow )
            state.dataLoopNodes->Node(InletNode).MassFlowRate = minAirMassFlow;
            // set max water flow rate and check to see if plant limits flow
            if (coilPlantLoc.loopNum > 0) {
                PlantUtilities::SetComponentFlowRate(state, maxCoilFluidFlow, coilFluidInletNode, coilFluidOutletNode, coilPlantLoc);
            }

            if (HeatingLoad) { // Function UnitarySystems::calcUnitarySystemToLoad, 4th and 5th arguments are CoolPLR and HeatPLR
                // set the water flow ratio so water coil gets proper flow
                if (SZVAVModel.MaxHeatCoilFluidFlow > 0.0) {
                    SZVAVModel.HeatCoilWaterFlowRatio = maxCoilFluidFlow / SZVAVModel.MaxHeatCoilFluidFlow;
                }
            }
            FanCoilUnits::Calc4PipeFanCoil(state, SysIndex, SZVAVModel.ControlZoneNum, FirstHVACIteration, TempSensOutput, PartLoadRatio);

            coilActive = std::abs(state.dataLoopNodes->Node(coilAirInletNode).Temp - state.dataLoopNodes->Node(coilAirOutletNode).Temp) > 0;

            if (!coilActive) { // if the coil is schedule off or the plant cannot provide water
                if (coilPlantLoc.loopNum > 0) {
                    state.dataLoopNodes->Node(coilFluidInletNode).MassFlowRate = 0.0;
                    PlantUtilities::SetComponentFlowRate(
                        state, state.dataLoopNodes->Node(coilFluidInletNode).MassFlowRate, coilFluidInletNode, coilFluidOutletNode, coilPlantLoc);
                }
                return;
            }

            if ((CoolingLoad && TempSensOutput < ZoneLoad) || (HeatingLoad && TempSensOutput > ZoneLoad)) { // low speed fan can meet load

                auto f = [&state, SysIndex, FirstHVACIteration, &SZVAVModel, ZoneLoad, coilFluidInletNode, maxCoilFluidFlow, minAirMassFlow](
                             Real64 const PLR) {
                    return FanCoilUnits::CalcFanCoilWaterFlowResidual(state,
                                                                      PLR,
                                                                      SysIndex,
                                                                      FirstHVACIteration,
                                                                      SZVAVModel.ControlZoneNum,
                                                                      ZoneLoad,
                                                                      SZVAVModel.AirInNode,
                                                                      coilFluidInletNode,
                                                                      maxCoilFluidFlow,
                                                                      minAirMassFlow);
                };
                General::SolveRoot(state, 0.001, MaxIter, SolFlag, PartLoadRatio, f, 0.0, 1.0);
                if (SolFlag < 0) {
                    MessagePrefix = "Step 1: ";
                }

                if (coilPlantLoc.loopNum > 0) {
                    PlantUtilities::SetComponentFlowRate(
                        state, state.dataLoopNodes->Node(coilFluidInletNode).MassFlowRate, coilFluidInletNode, coilFluidOutletNode, coilPlantLoc);
                }
            }

        } else {

            // Step 2: Load is greater than allowed in region 1, determine boundary load for region 3
            // only difference in this calculation is using maxAirMassFlow instead of minAirMassFlow, just use the ratio to adjust previous
            // calculation
            highBoundaryLoad = lowBoundaryLoad * maxAirMassFlow / minAirMassFlow;

            if ((CoolingLoad && highBoundaryLoad < ZoneLoad) || (HeatingLoad && highBoundaryLoad > ZoneLoad)) { // in Region 2 of figure

                outletTemp = state.dataLoopNodes->Node(OutletNode).Temp;
                minHumRat = state.dataLoopNodes->Node(SZVAVModel.NodeNumOfControlledZone).HumRat;
                if (outletTemp < ZoneTemp) {
                    minHumRat = state.dataLoopNodes->Node(OutletNode).HumRat;
                }
                outletTemp = maxOutletTemp;
                AirMassFlow = min(maxAirMassFlow,
                                  (ZoneLoad / (Psychrometrics::PsyHFnTdbW(outletTemp, minHumRat) - Psychrometrics::PsyHFnTdbW(ZoneTemp, minHumRat))));
                AirMassFlow = max(minAirMassFlow, AirMassFlow);
                SZVAVModel.FanPartLoadRatio = ((AirMassFlow - (maxAirMassFlow * lowSpeedFanRatio)) / ((1.0 - lowSpeedFanRatio) * maxAirMassFlow));

                state.dataLoopNodes->Node(InletNode).MassFlowRate = AirMassFlow;
                // does unit have capacity less than load at this air flow rate
                if (coilFluidInletNode > 0) {
                    state.dataLoopNodes->Node(coilFluidInletNode).MassFlowRate = lowWaterMdot;
                }
                FanCoilUnits::Calc4PipeFanCoil(state, SysIndex, SZVAVModel.ControlZoneNum, FirstHVACIteration, TempSensOutput, 0.0);
                if ((CoolingLoad && (TempSensOutput > ZoneLoad)) || (HeatingLoad && (TempSensOutput < ZoneLoad))) {
                    // can unit get there with max water flow?
                    if (coilFluidInletNode > 0) {
                        state.dataLoopNodes->Node(coilFluidInletNode).MassFlowRate = maxCoilFluidFlow;
                    }
                    FanCoilUnits::Calc4PipeFanCoil(state, SysIndex, SZVAVModel.ControlZoneNum, FirstHVACIteration, TempSensOutput, 1.0);

                    // set max water flow rate and check to see if plant limits flow
                    if (coilPlantLoc.loopNum > 0) {
                        PlantUtilities::SetComponentFlowRate(state, maxCoilFluidFlow, coilFluidInletNode, coilFluidOutletNode, coilPlantLoc);
                    }

                    if ((CoolingLoad && (TempSensOutput < ZoneLoad)) || (HeatingLoad && (TempSensOutput > ZoneLoad))) {
                        if (SZVAVModel.heatCoilType == HVAC::CoilType::HeatingWater || !HeatingLoad) {
                            auto f = [&state, SysIndex, FirstHVACIteration, &SZVAVModel, ZoneLoad, coilFluidInletNode, maxCoilFluidFlow, AirMassFlow](
                                         Real64 const PLR) {
                                return FanCoilUnits::CalcFanCoilWaterFlowResidual(state,
                                                                                  PLR,
                                                                                  SysIndex,
                                                                                  FirstHVACIteration,
                                                                                  SZVAVModel.ControlZoneNum,
                                                                                  ZoneLoad,
                                                                                  SZVAVModel.AirInNode,
                                                                                  coilFluidInletNode,
                                                                                  maxCoilFluidFlow,
                                                                                  AirMassFlow);
                            };
                            General::SolveRoot(state, 0.001, MaxIter, SolFlag, PartLoadRatio, f, 0.0, 1.0);
                        } else {
                            auto f = [&state, SysIndex, FirstHVACIteration, &SZVAVModel, ZoneLoad](Real64 const PartLoadRatio) {
                                return FanCoilUnits::CalcFanCoilLoadResidual(
                                    state, SysIndex, FirstHVACIteration, SZVAVModel.ControlZoneNum, ZoneLoad, PartLoadRatio);
                            };
                            General::SolveRoot(state, 0.001, MaxIter, SolFlag, PartLoadRatio, f, 0.0, 1.0);
                        }
                        outletTemp = state.dataLoopNodes->Node(OutletNode).Temp;
                        if ((CoolingLoad && outletTemp < maxOutletTemp) || (HeatingLoad && outletTemp > maxOutletTemp)) {
                            // must do something here to maintain outlet temp while in Region 2
                        }
                        if (SolFlag < 0) {
                            MessagePrefix = "Step 2: ";
                        }
                    } else { // not enough capacity at this air flow rate. Unit does have enough capacity a full water/air, otherwise wouldn't be here
                        // this is different from the PTUnit and UnitarySys routines in this module
                        // find the water flow rate that meets the min load at region 1/2 boundary
                        if (SZVAVModel.heatCoilType == HVAC::CoilType::HeatingWater || !HeatingLoad) {
                            auto f = // (AUTO_OK_LAMBDA)
                                [&state, SysIndex, FirstHVACIteration, &SZVAVModel, ZoneLoad, coilFluidInletNode, maxCoilFluidFlow, minAirMassFlow](
                                    Real64 const PLR) {
                                    return FanCoilUnits::CalcFanCoilWaterFlowResidual(state,
                                                                                      PLR,
                                                                                      SysIndex,
                                                                                      FirstHVACIteration,
                                                                                      SZVAVModel.ControlZoneNum,
                                                                                      ZoneLoad,
                                                                                      SZVAVModel.AirInNode,
                                                                                      coilFluidInletNode,
                                                                                      maxCoilFluidFlow,
                                                                                      minAirMassFlow);
                                };
                            General::SolveRoot(state, 0.001, MaxIter, SolFlag, lowWaterMdot, f, 0.0, 1.0);
                            Real64 minFlow = lowWaterMdot;
                            if (SolFlag < 0) {
                                MessagePrefix = "Step 2a: ";
                            } else {
                                minFlow = 0.0;
                            }
                            auto f2 = [&state, SysIndex, FirstHVACIteration, &SZVAVModel, ZoneLoad, coilFluidInletNode, minFlow](Real64 const PLR) {
                                return FanCoilUnits::CalcFanCoilAirAndWaterFlowResidual(state,
                                                                                        PLR,
                                                                                        SysIndex,
                                                                                        FirstHVACIteration,
                                                                                        SZVAVModel.ControlZoneNum,
                                                                                        ZoneLoad,
                                                                                        SZVAVModel.AirInNode,
                                                                                        coilFluidInletNode,
                                                                                        minFlow);
                            };
                            General::SolveRoot(state, 0.001, MaxIter, SolFlag, PartLoadRatio, f2, 0.0, 1.0);
                            if (SolFlag < 0) {
                                MessagePrefix = "Step 2b: ";
                            }
                        } else {
                            auto f = [&state, SysIndex, FirstHVACIteration, &SZVAVModel, ZoneLoad](Real64 const PartLoadRatio) {
                                return FanCoilUnits::CalcFanCoilLoadResidual(
                                    state, SysIndex, FirstHVACIteration, SZVAVModel.ControlZoneNum, ZoneLoad, PartLoadRatio);
                            };
                            General::SolveRoot(state, 0.001, MaxIter, SolFlag, PartLoadRatio, f, 0.0, 1.0);
                            if (SolFlag < 0) {
                                MessagePrefix = "Step 2: ";
                            }
                        }
                    }
                } else { // too much capacity when coil off, could lower air flow rate here to meet load if air flow is above minimum
                    if (SZVAVModel.heatCoilType == HVAC::CoilType::HeatingWater || !HeatingLoad) {
                        auto f2 = [&state, SysIndex, FirstHVACIteration, &SZVAVModel, ZoneLoad, coilFluidInletNode](Real64 const PLR) {
                            return FanCoilUnits::CalcFanCoilAirAndWaterFlowResidual(state,
                                                                                    PLR,
                                                                                    SysIndex,
                                                                                    FirstHVACIteration,
                                                                                    SZVAVModel.ControlZoneNum,
                                                                                    ZoneLoad,
                                                                                    SZVAVModel.AirInNode,
                                                                                    coilFluidInletNode,
                                                                                    0.0);
                        };
                        General::SolveRoot(state, 0.001, MaxIter, SolFlag, PartLoadRatio, f2, 0.0, 1.0);
                    } else {
                        auto f = [&state, SysIndex, FirstHVACIteration, &SZVAVModel, ZoneLoad](Real64 const PartLoadRatio) {
                            return FanCoilUnits::CalcFanCoilLoadResidual(
                                state, SysIndex, FirstHVACIteration, SZVAVModel.ControlZoneNum, ZoneLoad, PartLoadRatio);
                        };
                        General::SolveRoot(state, 0.001, MaxIter, SolFlag, PartLoadRatio, f, 0.0, 1.0);
                    }
                    if (SolFlag < 0) {
                        MessagePrefix = "Step 2c: ";
                    }
                }

            } else { // in region 3 of figure

                PartLoadRatio = 1.0; // full coil capacity
                SZVAVModel.FanPartLoadRatio =
                    1.0; // minimum fan PLR, air flow = ( fanPartLoadRatio * maxAirMassFlow ) + ( ( 1.0 - fanPartLoadRatio ) * minAirMassFlow )
                state.dataLoopNodes->Node(InletNode).MassFlowRate = maxAirMassFlow;
                // set max water flow rate and check to see if plant limits flow
                if (coilPlantLoc.loopNum > 0) {
                    PlantUtilities::SetComponentFlowRate(state, maxCoilFluidFlow, coilFluidInletNode, coilFluidOutletNode, coilPlantLoc);
                }

                if (HeatingLoad) { // Function UnitarySystems::calcUnitarySystemToLoad, 4th and 5th arguments are CoolPLR and HeatPLR
                    // set the water flow ratio so water coil gets proper flow
                    if (SZVAVModel.MaxHeatCoilFluidFlow > 0.0) {
                        SZVAVModel.HeatCoilWaterFlowRatio = maxCoilFluidFlow / SZVAVModel.MaxHeatCoilFluidFlow;
                    }
                }
                FanCoilUnits::Calc4PipeFanCoil(state, SysIndex, SZVAVModel.ControlZoneNum, FirstHVACIteration, TempSensOutput, PartLoadRatio);
                coilActive = std::abs(state.dataLoopNodes->Node(coilAirInletNode).Temp - state.dataLoopNodes->Node(coilAirOutletNode).Temp) > 0;
                if (!coilActive) { // if the coil is schedule off or the plant cannot provide water
                    if (coilPlantLoc.loopNum > 0) {
                        state.dataLoopNodes->Node(coilFluidInletNode).MassFlowRate = 0.0;
                        PlantUtilities::SetComponentFlowRate(
                            state, state.dataLoopNodes->Node(coilFluidInletNode).MassFlowRate, coilFluidInletNode, coilFluidOutletNode, coilPlantLoc);
                    }
                    return;
                }

                if ((CoolingLoad && ZoneLoad < TempSensOutput) || (HeatingLoad && ZoneLoad > TempSensOutput)) {
                    return; // system cannot meet load, leave at max capacity
                }

                // check if coil off is less than load
                PartLoadRatio = 0.0; // no coil capacity at full air flow
                if (coilPlantLoc.loopNum > 0) {
                    state.dataLoopNodes->Node(coilFluidInletNode).MassFlowRate = 0.0;
                    PlantUtilities::SetComponentFlowRate(
                        state, state.dataLoopNodes->Node(coilFluidInletNode).MassFlowRate, coilFluidInletNode, coilFluidOutletNode, coilPlantLoc);
                }
                FanCoilUnits::Calc4PipeFanCoil(state, SysIndex, SZVAVModel.ControlZoneNum, FirstHVACIteration, TempSensOutput, PartLoadRatio);
                if ((CoolingLoad && ZoneLoad < TempSensOutput) || (HeatingLoad && ZoneLoad > TempSensOutput)) {
                    // otherwise iterate on load
                    if (SZVAVModel.heatCoilType == HVAC::CoilType::HeatingWater || !HeatingLoad) {
                        auto f = [&state, SysIndex, FirstHVACIteration, &SZVAVModel, ZoneLoad, coilFluidInletNode, maxCoilFluidFlow, maxAirMassFlow](
                                     Real64 const PLR) {
                            return FanCoilUnits::CalcFanCoilWaterFlowResidual(state,
                                                                              PLR,
                                                                              SysIndex,
                                                                              FirstHVACIteration,
                                                                              SZVAVModel.ControlZoneNum,
                                                                              ZoneLoad,
                                                                              SZVAVModel.AirInNode,
                                                                              coilFluidInletNode,
                                                                              maxCoilFluidFlow,
                                                                              maxAirMassFlow);
                        };
                        General::SolveRoot(state, 0.001, MaxIter, SolFlag, PartLoadRatio, f, 0.0, 1.0);
                    } else {
                        auto f = [&state, SysIndex, FirstHVACIteration, &SZVAVModel, ZoneLoad](Real64 const PartLoadRatio) {
                            return FanCoilUnits::CalcFanCoilLoadResidual(
                                state, SysIndex, FirstHVACIteration, SZVAVModel.ControlZoneNum, ZoneLoad, PartLoadRatio);
                        };
                        General::SolveRoot(state, 0.001, MaxIter, SolFlag, PartLoadRatio, f, 0.0, 1.0);
                    }
                    if (SolFlag < 0) {
                        MessagePrefix = "Step 3: ";
                    }
                } else { // too much capacity at full air flow with coil off, operate coil and fan in unison
                    if (SZVAVModel.heatCoilType == HVAC::CoilType::HeatingWater || !HeatingLoad) {
                        auto f2 = [&state, SysIndex, FirstHVACIteration, &SZVAVModel, ZoneLoad, coilFluidInletNode](Real64 const PLR) {
                            return FanCoilUnits::CalcFanCoilAirAndWaterFlowResidual(state,
                                                                                    PLR,
                                                                                    SysIndex,
                                                                                    FirstHVACIteration,
                                                                                    SZVAVModel.ControlZoneNum,
                                                                                    ZoneLoad,
                                                                                    SZVAVModel.AirInNode,
                                                                                    coilFluidInletNode,
                                                                                    0.0);
                        };
                        General::SolveRoot(state, 0.001, MaxIter, SolFlag, PartLoadRatio, f2, 0.0, 1.0);
                    } else {
                        auto f = [&state, SysIndex, FirstHVACIteration, &SZVAVModel, ZoneLoad](Real64 const PartLoadRatio) {
                            return FanCoilUnits::CalcFanCoilLoadResidual(
                                state, SysIndex, FirstHVACIteration, SZVAVModel.ControlZoneNum, ZoneLoad, PartLoadRatio);
                        };
                        General::SolveRoot(state, 0.001, MaxIter, SolFlag, PartLoadRatio, f, 0.0, 1.0);
                    }
                    if (SolFlag < 0) {
                        MessagePrefix = "Step 3a: ";
                    }
                }
            }

            if (coilPlantLoc.loopNum > 0) {
                PlantUtilities::SetComponentFlowRate(
                    state, state.dataLoopNodes->Node(coilFluidInletNode).MassFlowRate, coilFluidInletNode, coilFluidOutletNode, coilPlantLoc);
            }
        }

        if (SolFlag < 0) {
            if (SolFlag == -1) {
                // get capacity for warning
                FanCoilUnits::Calc4PipeFanCoil(state, SysIndex, SZVAVModel.ControlZoneNum, FirstHVACIteration, TempSensOutput, PartLoadRatio);

                if (std::abs(TempSensOutput - ZoneLoad) * SZVAVModel.ControlZoneMassFlowFrac >
                    15.0) { // water coil can provide same output at varying water PLR (model discontinuity?)
                    if (SZVAVModel.MaxIterIndex == 0) {
                        ShowWarningMessage(
                            state, std::format("{}Coil control failed to converge for {}:{}", MessagePrefix, SZVAVModel.UnitType, SZVAVModel.Name));
                        ShowContinueError(state, "  Iteration limit exceeded in calculating system sensible part-load ratio.");
                        ShowContinueErrorTimeStamp(
                            state,
                            std::format("Sensible load to be met = {:.2f} (watts), sensible output = {:.2f} (watts), and the simulation continues.",
                                        ZoneLoad,
                                        TempSensOutput));
                    }
                    ShowRecurringWarningErrorAtEnd(
                        state,
                        SZVAVModel.UnitType + " \"" + SZVAVModel.Name +
                            "\" - Iteration limit exceeded in calculating sensible part-load ratio error continues. Sensible load statistics:",
                        SZVAVModel.MaxIterIndex,
                        ZoneLoad,
                        ZoneLoad);
                }
            } else if (SolFlag == -2) {
                if (SZVAVModel.RegulaFalsiFailedIndex == 0) {
                    ShowWarningMessage(state, std::format("{}Coil control failed for {}:{}", MessagePrefix, SZVAVModel.UnitType, SZVAVModel.Name));
                    ShowContinueError(state, "  sensible part-load ratio determined to be outside the range of 0-1.");
                    ShowContinueErrorTimeStamp(state,
                                               std::format("Sensible load to be met = {:.2f} (watts), and the simulation continues.", ZoneLoad));
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               SZVAVModel.UnitType + " \"" + SZVAVModel.Name +
                                                   "\" - sensible part-load ratio out of range error continues. Sensible load statistics:",
                                               SZVAVModel.RegulaFalsiFailedIndex,
                                               ZoneLoad,
                                               ZoneLoad);
            }
        }
    }

    void calcSZVAVModel(EnergyPlusData &state,
                        UnitarySystems::UnitarySys &SZVAVModel,
                        bool const FirstHVACIteration,
                        bool const CoolingLoad,
                        bool const HeatingLoad,
                        Real64 const ZoneLoad,
                        Real64 &OnOffAirFlowRatio,
                        bool const HXUnitOn,
                        int const AirLoopNum,
                        Real64 &PartLoadRatio,
                        HVAC::CompressorOp const CompressorONFlag)
    {

        int constexpr MaxIter(100); // maximum number of iterations
        int SolFlag(0);             // return flag from RegulaFalsi for sensible load
        std::string MessagePrefix;  // label for warning reporting

        Real64 boundaryLoadMet(0.0);
        Real64 minHumRat(0.0);
        Real64 outletTemp(0.0);
        bool coilActive(false);
        Real64 AirMassFlow(0.0);

        Real64 maxCoilFluidFlow(0.0);
        Real64 maxOutletTemp(0.0);
        Real64 minAirMassFlow(0.0);
        Real64 maxAirMassFlow(0.0);
        Real64 lowSpeedFanRatio(0.0);
        int coilFluidInletNode(0);
        int coilFluidOutletNode(0);
        PlantLocation coilPlantLoc{};
        int coilAirInletNode(0);
        int coilAirOutletNode(0);
        Real64 HeatCoilLoad(0.0);
        Real64 SupHeaterLoad(0.0);
        bool iterWaterAirOrNot(false);

        Real64 TempSensOutput; // iterative sensible capacity [W]
        Real64 TempLatOutput;  // iterative latent capacity [W]

        // set up mode specific variables to use in common function calls
        if (CoolingLoad) {
            maxCoilFluidFlow = SZVAVModel.MaxCoolCoilFluidFlow;
            maxOutletTemp = SZVAVModel.DesignMinOutletTemp;
            minAirMassFlow = SZVAVModel.MaxNoCoolHeatAirMassFlow;
            maxAirMassFlow = SZVAVModel.MaxCoolAirMassFlow;
            lowSpeedFanRatio = SZVAVModel.LowSpeedCoolFanRatio;
            coilFluidInletNode = SZVAVModel.CoolCoilFluidInletNode;
            coilFluidOutletNode = SZVAVModel.CoolCoilFluidOutletNodeNum;
            coilPlantLoc = SZVAVModel.CoolCoilPlantLoc;
            coilAirInletNode = SZVAVModel.CoolCoilInletNodeNum;
            coilAirOutletNode = SZVAVModel.CoolCoilOutletNodeNum;
        } else if (HeatingLoad) {
            maxCoilFluidFlow = SZVAVModel.MaxHeatCoilFluidFlow;
            maxOutletTemp = SZVAVModel.DesignMaxOutletTemp;
            minAirMassFlow = SZVAVModel.MaxNoCoolHeatAirMassFlow;
            maxAirMassFlow = SZVAVModel.MaxHeatAirMassFlow;
            lowSpeedFanRatio = SZVAVModel.LowSpeedHeatFanRatio;
            coilFluidInletNode = SZVAVModel.HeatCoilFluidInletNode;
            coilFluidOutletNode = SZVAVModel.HeatCoilFluidOutletNodeNum;
            coilPlantLoc = SZVAVModel.HeatCoilPlantLoc;
            coilAirInletNode = SZVAVModel.HeatCoilInletNodeNum;
            coilAirOutletNode = SZVAVModel.HeatCoilOutletNodeNum;
        } else { // should never get here, protect against uninitialized variables
            maxCoilFluidFlow = 0.0;
            maxOutletTemp = 0.0;
            minAirMassFlow = 0.0;
            maxAirMassFlow = 0.0;
            lowSpeedFanRatio = 0.0;
            coilFluidInletNode = 0;
            coilFluidOutletNode = 0;
            coilPlantLoc = {0, DataPlant::LoopSideLocation::Invalid, 0, 0};
            coilAirInletNode = 0;
            coilAirOutletNode = 0;
        }
        // set up RegulaFalsi variables
        int InletNode = SZVAVModel.AirInNode;
        Real64 InletTemp = state.dataLoopNodes->Node(InletNode).Temp;
        int OutletNode = SZVAVModel.AirOutNode;
        Real64 ZoneTemp = state.dataLoopNodes->Node(SZVAVModel.NodeNumOfControlledZone).Temp;
        Real64 ZoneHumRat = state.dataLoopNodes->Node(SZVAVModel.NodeNumOfControlledZone).HumRat;
        SZVAVModel.m_SimASHRAEModelOn = true;

        // model attempts to control air flow rate and coil capacity in specific operating regions:
        // Region 1 (R1) - minimum air flow rate at modulated coil capacity (up to min/max temperature limits)
        // Region 2 (R2) - modulated air flow rate and coil capacity (up to max air flow rate while maintaining min/max temperature limits)
        // Region 3 (R3) - maximum air flow rate and modulated/increased coil capacity (allow increased capacity at full air flow rate to meet
        // remaining load)
        //
        //                |    |                   |    |    ^            ^ = supply air temperature
        //                |    |                   |    | ^               * = supply air flow rate
        //                |    |                   |^^^^| <--- maximum supply air temperature
        //                |    |                ^  |    |
        //                |    |              ^    |    |
        //     ***********|    |            ^      |    |**************   <-- max unit air flow rate
        //                |*   |          ^        |   *|
        //                | *  |        ^          |  * |
        //                |  * |      ^            | *  |
        //                |   *|    ^              |*   |
        //                |    |*******************|    |                 <-- min unit air flow rate
        //          R3    | R2 | ^       R1        | R2 |    R3
        //   min SAT -->  |^^^^|                   |    |
        //               ^
        //             ^                  |
        //  (-) increasing cooling load < 0 > increasing heating load (+)
        //
        // Notes: SAT is allowed to decrease/increase once air flow rate is max'ed out otherwise load would not be met
        //        Load here is not the zone load, it's the load the system must meet to meet the Tstat set point (i.e., OA can alter required
        //        capacity) lowSpeedFanRatio = min/max unit air flow rate
        //
        // Step 1: calculate load at Region 1 lower (cooling) or upper (heating) boundary at minimum air flow rate
        //         - if load can be met, test maximum output (PLR = 1) before calling RootSolver
        //         - if maximum capacity is greater than load, solve for PLR
        // Step 2: calculate load at Region 3 lower (cooling) or upper (heating) boundary at maximum air flow rate
        //         - if load is less than boundary load, solve for air flow and PLR that meet the load
        //         - ELSE
        // Step 3: solve for Region 3 PLR
        //       DONE
        //

        // Step 1: Determine boundary for region 1
        // calculate sensible load based on minimum air flow rate and specified supply air temperature limit
        if (SZVAVModel.ATMixerExists) {
            if (SZVAVModel.ATMixerType == HVAC::MixerType::SupplySide) {
                // Air terminal supply side mixer
                boundaryLoadMet =
                    minAirMassFlow * (Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(SZVAVModel.ATMixerOutNode).Temp, ZoneHumRat) -
                                      Psychrometrics::PsyHFnTdbW(ZoneTemp, ZoneHumRat));
            } else {
                // Air terminal inlet side mixer
                boundaryLoadMet =
                    minAirMassFlow * (Psychrometrics::PsyHFnTdbW(maxOutletTemp, ZoneHumRat) - Psychrometrics::PsyHFnTdbW(ZoneTemp, ZoneHumRat));
            }
        } else {
            minHumRat = min(state.dataLoopNodes->Node(InletNode).HumRat, state.dataLoopNodes->Node(OutletNode).HumRat);
            boundaryLoadMet =
                minAirMassFlow * (Psychrometrics::PsyHFnTdbW(maxOutletTemp, minHumRat) - Psychrometrics::PsyHFnTdbW(InletTemp, minHumRat));
        }

        if ((CoolingLoad && boundaryLoadMet < ZoneLoad) || (HeatingLoad && boundaryLoadMet > ZoneLoad)) { // in Region 1 of figure
            // Step 1: set min air flow and full coil capacity
            PartLoadRatio = 1.0; // full coil capacity
            SZVAVModel.FanPartLoadRatio =
                0.0; // minimum fan PLR, air flow = ( fanPartLoadRatio * maxAirMassFlow ) + ( ( 1.0 - fanPartLoadRatio ) * minAirMassFlow )
            state.dataLoopNodes->Node(InletNode).MassFlowRate = minAirMassFlow;
            // set max water flow rate and check to see if plant limits flow
            if (coilPlantLoc.loopNum > 0) {
                PlantUtilities::SetComponentFlowRate(state, maxCoilFluidFlow, coilFluidInletNode, coilFluidOutletNode, coilPlantLoc);
            }

            if (CoolingLoad) { // Function CalcUnitarySystemToLoad, 4th and 5th arguments are CoolPLR and HeatPLR
                // set the water flow ratio so water coil gets proper flow
                if (SZVAVModel.MaxCoolCoilFluidFlow > 0.0) {
                    SZVAVModel.CoolCoilWaterFlowRatio = maxCoilFluidFlow / SZVAVModel.MaxCoolCoilFluidFlow;
                }
                SZVAVModel.calcUnitarySystemToLoad(state,
                                                   AirLoopNum,
                                                   FirstHVACIteration,
                                                   PartLoadRatio,
                                                   0.0,
                                                   OnOffAirFlowRatio,
                                                   TempSensOutput,
                                                   TempLatOutput,
                                                   HXUnitOn,
                                                   HeatCoilLoad,
                                                   SupHeaterLoad,
                                                   CompressorONFlag);
            } else {
                if (SZVAVModel.MaxHeatCoilFluidFlow > 0.0) {
                    SZVAVModel.HeatCoilWaterFlowRatio = maxCoilFluidFlow / SZVAVModel.MaxHeatCoilFluidFlow;
                }
                SZVAVModel.calcUnitarySystemToLoad(state,
                                                   AirLoopNum,
                                                   FirstHVACIteration,
                                                   0.0,
                                                   PartLoadRatio,
                                                   OnOffAirFlowRatio,
                                                   TempSensOutput,
                                                   TempLatOutput,
                                                   HXUnitOn,
                                                   ZoneLoad,
                                                   SupHeaterLoad,
                                                   CompressorONFlag);
            }

            coilActive = std::abs(state.dataLoopNodes->Node(coilAirInletNode).Temp - state.dataLoopNodes->Node(coilAirOutletNode).Temp) > 0;

            if (!coilActive) { // if the coil is schedule off or the plant cannot provide water
                if (coilPlantLoc.loopNum > 0) {
                    state.dataLoopNodes->Node(coilFluidInletNode).MassFlowRate = 0.0;
                    PlantUtilities::SetComponentFlowRate(
                        state, state.dataLoopNodes->Node(coilFluidInletNode).MassFlowRate, coilFluidInletNode, coilFluidOutletNode, coilPlantLoc);
                }
                return;
            }

            if ((CoolingLoad && TempSensOutput < ZoneLoad) || (HeatingLoad && TempSensOutput > ZoneLoad)) { // low speed fan can meet load
                // don't iterate on air flow in region 1 using iterWaterAirOrNot = false as default
                auto fR1 = [&state,
                            FirstHVACIteration,
                            ZoneLoad,
                            &SZVAVModel,
                            OnOffAirFlowRatio,
                            AirLoopNum,
                            coilFluidInletNode,
                            lowSpeedFanRatio,
                            maxCoilFluidFlow,
                            minAirMassFlow,
                            maxAirMassFlow,
                            iterWaterAirOrNot,
                            CoolingLoad](Real64 const PartLoadRatio) {
                    return SZVAVModel.calcUnitarySystemWaterFlowResidual(state,
                                                                         PartLoadRatio, // coil part load ratio
                                                                         FirstHVACIteration,
                                                                         ZoneLoad,
                                                                         SZVAVModel.AirInNode,
                                                                         OnOffAirFlowRatio,
                                                                         AirLoopNum,
                                                                         coilFluidInletNode,
                                                                         maxCoilFluidFlow,
                                                                         lowSpeedFanRatio,
                                                                         minAirMassFlow,
                                                                         0.0,
                                                                         maxAirMassFlow,
                                                                         CoolingLoad,
                                                                         iterWaterAirOrNot);
                };
                General::SolveRoot(state, 0.001, MaxIter, SolFlag, PartLoadRatio, fR1, 0.0, 1.0);
                if (SolFlag < 0) {
                    MessagePrefix = "Step 1: ";
                }

                if (coilPlantLoc.loopNum > 0) {
                    PlantUtilities::SetComponentFlowRate(
                        state, state.dataLoopNodes->Node(coilFluidInletNode).MassFlowRate, coilFluidInletNode, coilFluidOutletNode, coilPlantLoc);
                }
            }

        } else {

            // Step 2: Load is greater than allowed in region 1, determine boundary load for region 3
            // only difference in this calculation is using maxAirMassFlow instead of minAirMassFlow, just use the ratio to adjust previous
            // calculation
            boundaryLoadMet *= maxAirMassFlow / minAirMassFlow;

            if ((CoolingLoad && boundaryLoadMet < ZoneLoad) || (HeatingLoad && boundaryLoadMet > ZoneLoad)) { // in Region 2 of figure

                iterWaterAirOrNot = true; // iterate on air and/or water flow in region 2
                outletTemp = state.dataLoopNodes->Node(OutletNode).Temp;
                minHumRat = state.dataLoopNodes->Node(SZVAVModel.NodeNumOfControlledZone).HumRat;
                if (outletTemp < ZoneTemp) {
                    minHumRat = state.dataLoopNodes->Node(OutletNode).HumRat;
                }
                outletTemp = maxOutletTemp;
                AirMassFlow = min(maxAirMassFlow,
                                  (ZoneLoad / (Psychrometrics::PsyHFnTdbW(outletTemp, minHumRat) - Psychrometrics::PsyHFnTdbW(ZoneTemp, minHumRat))));
                AirMassFlow = max(minAirMassFlow, AirMassFlow);
                SZVAVModel.FanPartLoadRatio = ((AirMassFlow - (maxAirMassFlow * lowSpeedFanRatio)) / ((1.0 - lowSpeedFanRatio) * maxAirMassFlow));

                auto fR2 = [&state,
                            FirstHVACIteration,
                            ZoneLoad,
                            &SZVAVModel,
                            OnOffAirFlowRatio,
                            AirLoopNum,
                            coilFluidInletNode,
                            lowSpeedFanRatio,
                            AirMassFlow,
                            maxAirMassFlow,
                            iterWaterAirOrNot,
                            CoolingLoad,
                            maxCoilFluidFlow](Real64 const PartLoadRatio) {
                    return SZVAVModel.calcUnitarySystemWaterFlowResidual(state,
                                                                         PartLoadRatio, // coil part load ratio
                                                                         FirstHVACIteration,
                                                                         ZoneLoad,
                                                                         SZVAVModel.AirInNode,
                                                                         OnOffAirFlowRatio,
                                                                         AirLoopNum,
                                                                         coilFluidInletNode,
                                                                         maxCoilFluidFlow,
                                                                         lowSpeedFanRatio,
                                                                         AirMassFlow,
                                                                         0.0,
                                                                         maxAirMassFlow,
                                                                         CoolingLoad,
                                                                         iterWaterAirOrNot);
                };
                General::SolveRoot(state, 0.001, MaxIter, SolFlag, PartLoadRatio, fR2, 0.0, 1.0);
                if (SolFlag == -2 && ((CoolingLoad && SZVAVModel.m_CoolingSpeedNum < SZVAVModel.m_NumOfSpeedCooling) ||
                                      (HeatingLoad && SZVAVModel.m_HeatingSpeedNum < SZVAVModel.m_NumOfSpeedHeating))) {
                    // attempt to meet the load with the next speed
                    int szVAVModelSpeed = 0;
                    int szVAVModelSpeedMax = 0;
                    if (CoolingLoad) {
                        szVAVModelSpeed = SZVAVModel.m_CoolingSpeedNum + 1;
                        szVAVModelSpeedMax = SZVAVModel.m_NumOfSpeedCooling;
                    } else {
                        szVAVModelSpeed = SZVAVModel.m_HeatingSpeedNum + 1;
                        szVAVModelSpeedMax = SZVAVModel.m_NumOfSpeedHeating;
                    }
                    for (int szVAVSpeed = szVAVModelSpeed; szVAVSpeed <= szVAVModelSpeedMax; ++szVAVSpeed) {
                        if (CoolingLoad) {
                            SZVAVModel.m_CoolingSpeedNum = szVAVSpeed;
                        } else {
                            SZVAVModel.m_HeatingSpeedNum = szVAVSpeed;
                        }
                        auto f = [&state,
                                  FirstHVACIteration,
                                  ZoneLoad,
                                  &SZVAVModel,
                                  OnOffAirFlowRatio,
                                  AirLoopNum,
                                  coilFluidInletNode,
                                  lowSpeedFanRatio,
                                  AirMassFlow,
                                  maxAirMassFlow,
                                  iterWaterAirOrNot,
                                  CoolingLoad,
                                  maxCoilFluidFlow](Real64 const PartLoadRatio) {
                            return SZVAVModel.calcUnitarySystemWaterFlowResidual(state,
                                                                                 PartLoadRatio,
                                                                                 FirstHVACIteration,
                                                                                 ZoneLoad,
                                                                                 SZVAVModel.AirInNode,
                                                                                 OnOffAirFlowRatio,
                                                                                 AirLoopNum,
                                                                                 coilFluidInletNode,
                                                                                 maxCoilFluidFlow,
                                                                                 lowSpeedFanRatio,
                                                                                 AirMassFlow,
                                                                                 0.0,
                                                                                 maxAirMassFlow,
                                                                                 CoolingLoad,
                                                                                 iterWaterAirOrNot);
                        };
                        General::SolveRoot(state, 0.001, MaxIter, SolFlag, PartLoadRatio, f, 0.0, 1.0);
                        if (SolFlag > 0) {
                            break;
                        }
                    }
                    if (SolFlag < 0) {
                        MessagePrefix = "Step 2: ";
                    }
                }

            } else { // in region 3 of figure

                PartLoadRatio = 1.0; // full coil capacity
                SZVAVModel.FanPartLoadRatio =
                    1.0; // minimum fan PLR, air flow = ( fanPartLoadRatio * maxAirMassFlow ) + ( ( 1.0 - fanPartLoadRatio ) * minAirMassFlow )
                state.dataLoopNodes->Node(InletNode).MassFlowRate = maxAirMassFlow;
                // set max water flow rate and check to see if plant limits flow
                if (coilPlantLoc.loopNum > 0) {
                    PlantUtilities::SetComponentFlowRate(state, maxCoilFluidFlow, coilFluidInletNode, coilFluidOutletNode, coilPlantLoc);
                }

                if (CoolingLoad) { // Function CalcUnitarySystemToLoad, 4th and 5th arguments are CoolPLR and HeatPLR
                    // set the water flow ratio so water coil gets proper flow
                    if (SZVAVModel.MaxCoolCoilFluidFlow > 0.0) {
                        SZVAVModel.CoolCoilWaterFlowRatio = maxCoilFluidFlow / SZVAVModel.MaxCoolCoilFluidFlow;
                    }
                    SZVAVModel.calcUnitarySystemToLoad(state,
                                                       AirLoopNum,
                                                       FirstHVACIteration,
                                                       PartLoadRatio,
                                                       0.0,
                                                       OnOffAirFlowRatio,
                                                       TempSensOutput,
                                                       TempLatOutput,
                                                       HXUnitOn,
                                                       HeatCoilLoad,
                                                       SupHeaterLoad,
                                                       CompressorONFlag);
                } else {
                    if (SZVAVModel.MaxHeatCoilFluidFlow > 0.0) {
                        SZVAVModel.HeatCoilWaterFlowRatio = maxCoilFluidFlow / SZVAVModel.MaxHeatCoilFluidFlow;
                    }
                    SZVAVModel.calcUnitarySystemToLoad(state,
                                                       AirLoopNum,
                                                       FirstHVACIteration,
                                                       0.0,
                                                       PartLoadRatio,
                                                       OnOffAirFlowRatio,
                                                       TempSensOutput,
                                                       TempLatOutput,
                                                       HXUnitOn,
                                                       ZoneLoad,
                                                       SupHeaterLoad,
                                                       CompressorONFlag);
                }

                coilActive = std::abs(state.dataLoopNodes->Node(coilAirInletNode).Temp - state.dataLoopNodes->Node(coilAirOutletNode).Temp) > 0;

                if (!coilActive) { // if the coil is schedule off or the plant cannot provide water
                    if (coilPlantLoc.loopNum > 0) {
                        state.dataLoopNodes->Node(coilFluidInletNode).MassFlowRate = 0.0;
                        PlantUtilities::SetComponentFlowRate(
                            state, state.dataLoopNodes->Node(coilFluidInletNode).MassFlowRate, coilFluidInletNode, coilFluidOutletNode, coilPlantLoc);
                    }
                    return;
                }

                if ((CoolingLoad && ZoneLoad < TempSensOutput) || (HeatingLoad && ZoneLoad > TempSensOutput)) {
                    return; // system cannot meet load, leave at max capacity
                }

                iterWaterAirOrNot = false; // don't iterate on air flow in region 3
                // otherwise iterate on load
                auto fR3 = [&state,
                            FirstHVACIteration,
                            ZoneLoad,
                            &SZVAVModel,
                            OnOffAirFlowRatio,
                            AirLoopNum,
                            coilFluidInletNode,
                            lowSpeedFanRatio,
                            maxCoilFluidFlow,
                            maxAirMassFlow,
                            iterWaterAirOrNot,
                            CoolingLoad](Real64 const PartLoadRatio) {
                    return SZVAVModel.calcUnitarySystemWaterFlowResidual(state,
                                                                         PartLoadRatio, // coil part load ratio
                                                                         FirstHVACIteration,
                                                                         ZoneLoad,
                                                                         SZVAVModel.AirInNode,
                                                                         OnOffAirFlowRatio,
                                                                         AirLoopNum,
                                                                         coilFluidInletNode,
                                                                         maxCoilFluidFlow,
                                                                         lowSpeedFanRatio,
                                                                         maxAirMassFlow,
                                                                         0.0,
                                                                         maxAirMassFlow,
                                                                         CoolingLoad,
                                                                         iterWaterAirOrNot);
                };
                General::SolveRoot(state, 0.001, MaxIter, SolFlag, PartLoadRatio, fR3, 0.0, 1.0);
                //                Par[12] = maxAirMassFlow; // operating air flow rate, minAirMassFlow indicates low speed air flow rate,
                //                maxAirMassFlow indicates full
                //                                          // air flow
                //                Par[13] = 0.0;            // SA Temp target, 0 means iterate on load and not SA temperature
                //                General::SolveRoot(state, 0.001, MaxIter, SolFlag, PartLoadRatio, thisSys.calcUnitarySystemWaterFlowResidual,
                //                0.0, 1.0, Par);
                if (SolFlag < 0) {
                    MessagePrefix = "Step 3: ";
                }
            }

            if (coilPlantLoc.loopNum > 0) {
                PlantUtilities::SetComponentFlowRate(
                    state, state.dataLoopNodes->Node(coilFluidInletNode).MassFlowRate, coilFluidInletNode, coilFluidOutletNode, coilPlantLoc);
            }
        }

        if (SolFlag < 0) {
            if (SolFlag == -1) {
                // get capacity for warning
                if (CoolingLoad) { // Function CalcUnitarySystemToLoad, 4th and 5th arguments are CoolPLR and HeatPLR
                    SZVAVModel.calcUnitarySystemToLoad(state,
                                                       AirLoopNum,
                                                       FirstHVACIteration,
                                                       PartLoadRatio,
                                                       0.0,
                                                       OnOffAirFlowRatio,
                                                       TempSensOutput,
                                                       TempLatOutput,
                                                       HXUnitOn,
                                                       HeatCoilLoad,
                                                       SupHeaterLoad,
                                                       CompressorONFlag);
                } else {
                    SZVAVModel.calcUnitarySystemToLoad(state,
                                                       AirLoopNum,
                                                       FirstHVACIteration,
                                                       0.0,
                                                       PartLoadRatio,
                                                       OnOffAirFlowRatio,
                                                       TempSensOutput,
                                                       TempLatOutput,
                                                       HXUnitOn,
                                                       ZoneLoad,
                                                       SupHeaterLoad,
                                                       CompressorONFlag);
                }

                if (std::abs(TempSensOutput - ZoneLoad) * SZVAVModel.ControlZoneMassFlowFrac >
                    15.0) { // water coil can provide same output at varying water PLR (model discontinuity?)
                    if (SZVAVModel.MaxIterIndex == 0) {
                        ShowWarningMessage(
                            state, std::format("{}Coil control failed to converge for {}:{}", MessagePrefix, SZVAVModel.UnitType, SZVAVModel.Name));
                        ShowContinueError(state, "  Iteration limit exceeded in calculating system sensible part-load ratio.");
                        ShowContinueErrorTimeStamp(
                            state,
                            std::format("Sensible load to be met = {:.2f} (watts), sensible output = {:.2f} (watts), and the simulation continues.",
                                        ZoneLoad,
                                        TempSensOutput));
                    }
                    ShowRecurringWarningErrorAtEnd(
                        state,
                        SZVAVModel.UnitType + " \"" + SZVAVModel.Name +
                            "\" - Iteration limit exceeded in calculating sensible part-load ratio error continues. Sensible load statistics:",
                        SZVAVModel.MaxIterIndex,
                        ZoneLoad,
                        ZoneLoad);
                }
            } else if (SolFlag == -2) {
                if (SZVAVModel.RegulaFalsiFailedIndex == 0) {
                    ShowWarningMessage(state, std::format("{}Coil control failed for {}:{}", MessagePrefix, SZVAVModel.UnitType, SZVAVModel.Name));
                    ShowContinueError(state, "  sensible part-load ratio determined to be outside the range of 0-1.");
                    ShowContinueErrorTimeStamp(state,
                                               std::format("Sensible load to be met = {:.2f} (watts), and the simulation continues.", ZoneLoad));
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               SZVAVModel.UnitType + " \"" + SZVAVModel.Name +
                                                   "\" - sensible part-load ratio out of range error continues. Sensible load statistics:",
                                               SZVAVModel.RegulaFalsiFailedIndex,
                                               ZoneLoad,
                                               ZoneLoad);
            }
        }
    }

} // namespace SZVAVModel

} // namespace EnergyPlus

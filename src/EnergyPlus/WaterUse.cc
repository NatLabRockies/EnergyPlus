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
#include <cmath>

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataWater.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/HeatBalanceInternalHeatGains.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterManager.hh>
#include <EnergyPlus/WaterUse.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace WaterUse {

    // MODULE INFORMATION:
    //       AUTHOR         Peter Graham Ellis
    //       DATE WRITTEN   August 2006
    //       MODIFIED       Brent Griffith, plant upgrade

    void SimulateWaterUse(EnergyPlusData &state, bool FirstHVACIteration)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   August 2006
        //       MODIFIED       Brent Griffith, March 2010, separated plant connected to different sim routine

        // PURPOSE OF THIS SUBROUTINE:
        // This routine is called from non zone equipment manager and serves to call
        // water use and connections that are not connected to a full plant loop

        int constexpr MaxIterations(100);
        Real64 constexpr Tolerance(0.1); // Make input?

        if (state.dataWaterUse->getWaterUseInputFlag) {
            GetWaterUseInput(state);
            state.dataWaterUse->getWaterUseInputFlag = false;
        }

        if (state.dataGlobal->BeginEnvrnFlag && state.dataWaterUse->MyEnvrnFlagLocal) {
            if (state.dataWaterUse->numWaterEquipment > 0) {
                for (auto &e : state.dataWaterUse->WaterEquipment) {
                    e.SensibleRate = 0.0;
                    e.SensibleEnergy = 0.0;
                    e.LatentRate = 0.0;
                    e.LatentEnergy = 0.0;
                    e.MixedTemp = 0.0;
                    e.TotalMassFlowRate = 0.0;
                    e.DrainTemp = 0.0;
                }
            }

            if (state.dataWaterUse->numWaterConnections > 0) {
                for (auto &e : state.dataWaterUse->WaterConnections) {
                    e.TotalMassFlowRate = 0.0;
                }
            }

            state.dataWaterUse->MyEnvrnFlagLocal = false;
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataWaterUse->MyEnvrnFlagLocal = true;
        }

        // Simulate all unconnected WATER USE EQUIPMENT objects
        for (auto &waterEquipment : state.dataWaterUse->WaterEquipment) {
            if (waterEquipment.Connections == 0) {
                waterEquipment.CalcEquipmentFlowRates(state);
                waterEquipment.CalcEquipmentDrainTemp(state);
            }
        } // WaterEquipNum

        ReportStandAloneWaterUse(state);

        // Simulate WATER USE CONNECTIONS objects and connected WATER USE EQUIPMENT objects
        for (auto &waterConnection : state.dataWaterUse->WaterConnections) {

            if (!waterConnection.StandAlone) {
                continue; // only model non plant connections here
            }

            waterConnection.InitConnections(state);

            int NumIteration = 0;

            while (true) {
                ++NumIteration;

                waterConnection.CalcConnectionsFlowRates(state, FirstHVACIteration);
                waterConnection.CalcConnectionsDrainTemp(state);
                waterConnection.CalcConnectionsHeatRecovery(state);

                if (waterConnection.TempError < Tolerance) {
                    break;
                } else if (NumIteration > MaxIterations) {
                    if (!state.dataGlobal->WarmupFlag) {
                        if (waterConnection.MaxIterationsErrorIndex == 0) {
                            ShowWarningError(state,
                                             format("WaterUse:Connections = {}:  Heat recovery temperature did not converge", waterConnection.Name));
                            ShowContinueErrorTimeStamp(state, "");
                        }
                        ShowRecurringWarningErrorAtEnd(state,
                                                       "WaterUse:Connections = " + waterConnection.Name +
                                                           ":  Heat recovery temperature did not converge",
                                                       waterConnection.MaxIterationsErrorIndex);
                    }
                    break;
                }

            } // WHILE

            waterConnection.UpdateWaterConnections(state);
            waterConnection.ReportWaterUse(state);

        } // WaterConnNum
    }

    PlantComponent *WaterConnectionsType::factory(EnergyPlusData &state, std::string const &objectName)
    {
        // Process the input data
        if (state.dataWaterUse->getWaterUseInputFlag) {
            GetWaterUseInput(state);
            state.dataWaterUse->getWaterUseInputFlag = false;
        }

        // Now look for this particular object in the list
        for (auto &thisWC : state.dataWaterUse->WaterConnections) {
            if (thisWC.Name == objectName) {
                return &thisWC;
            }
        }
        // If we didn't find it, fatal
        ShowFatalError(state, format("LocalWaterUseConnectionFactory: Error getting inputs for object named: {}", objectName)); // LCOV_EXCL_LINE
        // Shut up the compiler
        return nullptr; // LCOV_EXCL_LINE
    }

    void WaterConnectionsType::simulate(EnergyPlusData &state,
                                        [[maybe_unused]] const PlantLocation &calledFromLocation,
                                        bool FirstHVACIteration,
                                        [[maybe_unused]] Real64 &CurLoad,
                                        [[maybe_unused]] bool RunFlag)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith March 2010, Demand Side Update
        //       DATE WRITTEN   August 2006

        // PURPOSE OF THIS SUBROUTINE:
        // Plant sim call for plant loop connected water use and connections

        int constexpr MaxIterations(100);
        Real64 constexpr Tolerance(0.1); // Make input?

        if (state.dataGlobal->BeginEnvrnFlag && this->MyEnvrnFlag) {
            if (state.dataWaterUse->numWaterEquipment > 0) {
                for (auto &waterEquipment : state.dataWaterUse->WaterEquipment) {
                    waterEquipment.reset();
                    if (waterEquipment.setupMyOutputVars) {
                        waterEquipment.setupOutputVars(state);
                        waterEquipment.setupMyOutputVars = false;
                    }
                }
            }

            if (state.dataWaterUse->numWaterConnections > 0) {
                for (auto &waterConnections : state.dataWaterUse->WaterConnections) {
                    waterConnections.TotalMassFlowRate = 0.0;
                }
            }

            this->MyEnvrnFlag = false;
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            this->MyEnvrnFlag = true;
        }

        this->InitConnections(state);

        int NumIteration = 0;

        while (true) {
            ++NumIteration;

            this->CalcConnectionsFlowRates(state, FirstHVACIteration);
            this->CalcConnectionsDrainTemp(state);
            this->CalcConnectionsHeatRecovery(state);

            if (this->TempError < Tolerance) {
                break;
            } else if (NumIteration > MaxIterations) {
                if (!state.dataGlobal->WarmupFlag) {
                    if (this->MaxIterationsErrorIndex == 0) {
                        ShowWarningError(state, format("WaterUse:Connections = {}:  Heat recovery temperature did not converge", this->Name));
                        ShowContinueErrorTimeStamp(state, "");
                    }
                    ShowRecurringWarningErrorAtEnd(state,
                                                   "WaterUse:Connections = " + this->Name + ":  Heat recovery temperature did not converge",
                                                   this->MaxIterationsErrorIndex);
                }
                break;
            }
        } // WHILE

        this->UpdateWaterConnections(state);
        this->ReportWaterUse(state);
    }

    void GetWaterUseInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   August 2006

        static constexpr std::string_view routineName = "GetWaterUseInput";

        bool ErrorsFound(false); // Set to true if errors in input, fatal at end of routine
        int IOStatus;            // Used in GetObjectItem
        int NumAlphas;           // Number of Alphas for each GetObjectItem call
        int NumNumbers;          // Number of Numbers for each GetObjectItem call

        constexpr std::array<std::string_view, static_cast<int>(HeatRecovHX::Num)> HeatRecoverHXNamesUC{"IDEAL", "COUNTERFLOW", "CROSSFLOW"};

        constexpr std::array<std::string_view, static_cast<int>(HeatRecovConfig::Num)> HeatRecoveryConfigNamesUC{
            "PLANT", "EQUIPMENT", "PLANTANDEQUIPMENT"};

        state.dataIPShortCut->cCurrentModuleObject = "WaterUse:Equipment";
        state.dataWaterUse->numWaterEquipment =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, state.dataIPShortCut->cCurrentModuleObject);

        if (state.dataWaterUse->numWaterEquipment > 0) {
            state.dataWaterUse->WaterEquipment.allocate(state.dataWaterUse->numWaterEquipment);

            for (int WaterEquipNum = 1; WaterEquipNum <= state.dataWaterUse->numWaterEquipment; ++WaterEquipNum) {
                auto &thisWEq = state.dataWaterUse->WaterEquipment(WaterEquipNum);
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         state.dataIPShortCut->cCurrentModuleObject,
                                                                         WaterEquipNum,
                                                                         state.dataIPShortCut->cAlphaArgs,
                                                                         NumAlphas,
                                                                         state.dataIPShortCut->rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         _,
                                                                         state.dataIPShortCut->lAlphaFieldBlanks,
                                                                         state.dataIPShortCut->cAlphaFieldNames,
                                                                         state.dataIPShortCut->cNumericFieldNames);

                ErrorObjectHeader eoh{routineName, state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};
                Util::IsNameEmpty(state, state.dataIPShortCut->cAlphaArgs(1), state.dataIPShortCut->cCurrentModuleObject, ErrorsFound);
                thisWEq.Name = state.dataIPShortCut->cAlphaArgs(1);

                thisWEq.EndUseSubcatName = state.dataIPShortCut->cAlphaArgs(2);

                thisWEq.PeakVolFlowRate = state.dataIPShortCut->rNumericArgs(1);

                if ((NumAlphas <= 2) || (state.dataIPShortCut->lAlphaFieldBlanks(3))) {
                } else if ((thisWEq.flowRateFracSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(3))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(3), state.dataIPShortCut->cAlphaArgs(3));
                    ErrorsFound = true;
                }

                if ((NumAlphas <= 3) || (state.dataIPShortCut->lAlphaFieldBlanks(4))) {
                } else if ((thisWEq.targetTempSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(4))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(4), state.dataIPShortCut->cAlphaArgs(4));
                    ErrorsFound = true;
                }

                if ((NumAlphas <= 4) || (state.dataIPShortCut->lAlphaFieldBlanks(5))) {
                } else if ((thisWEq.hotTempSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(5))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(5), state.dataIPShortCut->cAlphaArgs(5));
                    ErrorsFound = true;
                }

                if ((NumAlphas <= 5) || (state.dataIPShortCut->lAlphaFieldBlanks(6))) {
                } else if ((thisWEq.coldTempSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(6))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(6), state.dataIPShortCut->cAlphaArgs(6));
                    ErrorsFound = true;
                }

                if ((NumAlphas <= 6) || (state.dataIPShortCut->lAlphaFieldBlanks(7))) {
                } else if ((thisWEq.Zone = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(7), state.dataHeatBal->Zone)) == 0) {
                    ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(7), state.dataIPShortCut->cAlphaArgs(7));
                    ErrorsFound = true;
                }

                if ((NumAlphas <= 7) || (state.dataIPShortCut->lAlphaFieldBlanks(8))) {
                } else if ((thisWEq.sensibleFracSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(8))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(8), state.dataIPShortCut->cAlphaArgs(8));
                    ErrorsFound = true;
                }

                if ((NumAlphas <= 8) || (state.dataIPShortCut->lAlphaFieldBlanks(9))) {
                } else if ((thisWEq.latentFracSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(9))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(9), state.dataIPShortCut->cAlphaArgs(9));
                    ErrorsFound = true;
                }

            } // WaterEquipNum

            if (ErrorsFound) {
                ShowFatalError(state, format("Errors found in processing input for {}", state.dataIPShortCut->cCurrentModuleObject));
            }
        }

        state.dataIPShortCut->cCurrentModuleObject = "WaterUse:Connections";
        state.dataWaterUse->numWaterConnections =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, state.dataIPShortCut->cCurrentModuleObject);

        if (state.dataWaterUse->numWaterConnections > 0) {
            state.dataWaterUse->WaterConnections.allocate(state.dataWaterUse->numWaterConnections);

            for (int WaterConnNum = 1; WaterConnNum <= state.dataWaterUse->numWaterConnections; ++WaterConnNum) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         state.dataIPShortCut->cCurrentModuleObject,
                                                                         WaterConnNum,
                                                                         state.dataIPShortCut->cAlphaArgs,
                                                                         NumAlphas,
                                                                         state.dataIPShortCut->rNumericArgs,
                                                                         NumNumbers,
                                                                         IOStatus,
                                                                         _,
                                                                         state.dataIPShortCut->lAlphaFieldBlanks,
                                                                         state.dataIPShortCut->cAlphaFieldNames,
                                                                         state.dataIPShortCut->cNumericFieldNames);

                ErrorObjectHeader eoh{routineName, state.dataIPShortCut->cCurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};

                Util::IsNameEmpty(state, state.dataIPShortCut->cAlphaArgs(1), state.dataIPShortCut->cCurrentModuleObject, ErrorsFound);
                auto &waterConnection = state.dataWaterUse->WaterConnections(WaterConnNum);
                waterConnection.Name = state.dataIPShortCut->cAlphaArgs(1);

                if ((!state.dataIPShortCut->lAlphaFieldBlanks(2)) || (!state.dataIPShortCut->lAlphaFieldBlanks(3))) {
                    waterConnection.InletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                    state.dataIPShortCut->cAlphaArgs(2),
                                                                                    ErrorsFound,
                                                                                    DataLoopNode::ConnectionObjectType::WaterUseConnections,
                                                                                    waterConnection.Name,
                                                                                    DataLoopNode::NodeFluidType::Water,
                                                                                    DataLoopNode::ConnectionType::Inlet,
                                                                                    NodeInputManager::CompFluidStream::Primary,
                                                                                    DataLoopNode::ObjectIsNotParent);
                    waterConnection.OutletNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                     state.dataIPShortCut->cAlphaArgs(3),
                                                                                     ErrorsFound,
                                                                                     DataLoopNode::ConnectionObjectType::WaterUseConnections,
                                                                                     waterConnection.Name,
                                                                                     DataLoopNode::NodeFluidType::Water,
                                                                                     DataLoopNode::ConnectionType::Outlet,
                                                                                     NodeInputManager::CompFluidStream::Primary,
                                                                                     DataLoopNode::ObjectIsNotParent);

                    // Check plant connections
                    BranchNodeConnections::TestCompSet(state,
                                                       state.dataIPShortCut->cCurrentModuleObject,
                                                       waterConnection.Name,
                                                       state.dataIPShortCut->cAlphaArgs(2),
                                                       state.dataIPShortCut->cAlphaArgs(3),
                                                       "DHW Nodes");
                } else {
                    // If no plant nodes are connected, simulate in stand-alone mode.
                    waterConnection.StandAlone = true;
                }

                if (!state.dataIPShortCut->lAlphaFieldBlanks(4)) {
                    WaterManager::SetupTankDemandComponent(state,
                                                           waterConnection.Name,
                                                           state.dataIPShortCut->cCurrentModuleObject,
                                                           state.dataIPShortCut->cAlphaArgs(4),
                                                           ErrorsFound,
                                                           waterConnection.SupplyTankNum,
                                                           waterConnection.TankDemandID);
                }

                if (!state.dataIPShortCut->lAlphaFieldBlanks(5)) {
                    WaterManager::SetupTankSupplyComponent(state,
                                                           waterConnection.Name,
                                                           state.dataIPShortCut->cCurrentModuleObject,
                                                           state.dataIPShortCut->cAlphaArgs(5),
                                                           ErrorsFound,
                                                           waterConnection.RecoveryTankNum,
                                                           waterConnection.TankSupplyID);
                }

                if (state.dataIPShortCut->lAlphaFieldBlanks(6)) {
                } else if ((waterConnection.hotTempSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(6))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(6), state.dataIPShortCut->cAlphaArgs(6));
                    ErrorsFound = true;
                }

                if (state.dataIPShortCut->lAlphaFieldBlanks(7)) {
                } else if ((waterConnection.coldTempSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(7))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(7), state.dataIPShortCut->cAlphaArgs(7));
                    ErrorsFound = true;
                }

                if ((!state.dataIPShortCut->lAlphaFieldBlanks(8)) && (state.dataIPShortCut->cAlphaArgs(8) != "NONE")) {
                    waterConnection.HeatRecovery = true;
                    waterConnection.HeatRecoveryHX =
                        static_cast<HeatRecovHX>(getEnumValue(HeatRecoverHXNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(8))));
                    if (waterConnection.HeatRecoveryHX == HeatRecovHX::Invalid) {
                        ShowSevereError(state,
                                        format("Invalid {} = {}", state.dataIPShortCut->cAlphaFieldNames(8), state.dataIPShortCut->cAlphaArgs(8)));
                        ShowContinueError(state, format("Entered in {} = {}", state.dataIPShortCut->cCurrentModuleObject, waterConnection.Name));
                        ErrorsFound = true;
                    }

                    waterConnection.HeatRecoveryConfig =
                        static_cast<HeatRecovConfig>(getEnumValue(HeatRecoveryConfigNamesUC, Util::makeUPPER(state.dataIPShortCut->cAlphaArgs(9))));
                    if (waterConnection.HeatRecoveryConfig == HeatRecovConfig::Invalid) {
                        ShowSevereError(state,
                                        format("Invalid {} = {}", state.dataIPShortCut->cAlphaFieldNames(9), state.dataIPShortCut->cAlphaArgs(9)));
                        ShowContinueError(state, format("Entered in {} = {}", state.dataIPShortCut->cCurrentModuleObject, waterConnection.Name));
                        ErrorsFound = true;
                    }
                }

                waterConnection.HXUA = state.dataIPShortCut->rNumericArgs(1);

                waterConnection.myWaterEquipArr.allocate(NumAlphas - 9);

                for (int AlphaNum = 10; AlphaNum <= NumAlphas; ++AlphaNum) {
                    int WaterEquipNum = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(AlphaNum), state.dataWaterUse->WaterEquipment);

                    if (WaterEquipNum == 0) {
                        ShowSevereError(
                            state,
                            format("Invalid {} = {}", state.dataIPShortCut->cAlphaFieldNames(AlphaNum), state.dataIPShortCut->cAlphaArgs(AlphaNum)));
                        ShowContinueError(state, format("Entered in {} = {}", state.dataIPShortCut->cCurrentModuleObject, waterConnection.Name));
                        ErrorsFound = true;
                    } else {
                        if (state.dataWaterUse->WaterEquipment(WaterEquipNum).Connections > 0) {
                            ShowSevereError(state,
                                            format("{} = {}:  WaterUse:Equipment = {} is already referenced by another object.",
                                                   state.dataIPShortCut->cCurrentModuleObject,
                                                   waterConnection.Name,
                                                   state.dataIPShortCut->cAlphaArgs(AlphaNum)));
                            ErrorsFound = true;
                        } else {
                            state.dataWaterUse->WaterEquipment(WaterEquipNum).Connections = WaterConnNum;

                            ++waterConnection.NumWaterEquipment;
                            waterConnection.myWaterEquipArr(waterConnection.NumWaterEquipment) = WaterEquipNum;

                            waterConnection.PeakVolFlowRate +=
                                state.dataWaterUse->WaterEquipment(WaterEquipNum).PeakVolFlowRate; // this does not include possible multipliers
                        }
                    }
                }

            } // WaterConnNum

            if (ErrorsFound) {
                ShowFatalError(state, format("Errors found in processing input for {}", state.dataIPShortCut->cCurrentModuleObject));
            }

            if (state.dataWaterUse->numWaterConnections > 0) {
                state.dataWaterUse->CheckEquipName.allocate(state.dataWaterUse->numWaterConnections);
                state.dataWaterUse->CheckEquipName = true;
            }
        }

        // determine connection's peak mass flow rates.
        if (state.dataWaterUse->numWaterConnections > 0) {
            for (int WaterConnNum = 1; WaterConnNum <= state.dataWaterUse->numWaterConnections; ++WaterConnNum) {
                auto &waterConnection = state.dataWaterUse->WaterConnections(WaterConnNum);
                waterConnection.PeakMassFlowRate = 0.0;
                for (int WaterEquipNum = 1; WaterEquipNum <= waterConnection.NumWaterEquipment; ++WaterEquipNum) {
                    auto &thisWEq = state.dataWaterUse->WaterEquipment(waterConnection.myWaterEquipArr(WaterEquipNum));
                    if (thisWEq.Zone > 0) {
                        waterConnection.PeakMassFlowRate += thisWEq.PeakVolFlowRate * calcH2ODensity(state) *
                                                            state.dataHeatBal->Zone(thisWEq.Zone).Multiplier *
                                                            state.dataHeatBal->Zone(thisWEq.Zone).ListMultiplier;
                    } else { // can't have multipliers
                        waterConnection.PeakMassFlowRate += thisWEq.PeakVolFlowRate * calcH2ODensity(state);
                    }
                }
                PlantUtilities::RegisterPlantCompDesignFlow(
                    state, waterConnection.InletNode, waterConnection.PeakMassFlowRate / calcH2ODensity(state));
            }
        }
        // need a good place to set a bool to calculate WaterUse hot and cold flow rates in CalcEquipmentFlowRates
        // WaterUse can be used with or without WaterUse:Connections, with or without WaterUse:Equipment hot temp schedule
        for (auto &waterEquipment : state.dataWaterUse->WaterEquipment) {
            // set logical if either hot water temp or target temp schedule are missing (will use cold water otherwise)
            // if a connections object is used then don't need to hot temp schedule
            waterEquipment.allowHotControl =
                (waterEquipment.targetTempSched != nullptr && waterEquipment.hotTempSched != nullptr) || waterEquipment.Connections;
        }
    }

    void WaterEquipmentType::setupOutputVars(EnergyPlusData &state)
    {
        SetupOutputVariable(state,
                            "Water Use Equipment Hot Water Mass Flow Rate",
                            Constant::Units::kg_s,
                            this->HotMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Equipment Cold Water Mass Flow Rate",
                            Constant::Units::kg_s,
                            this->ColdMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Equipment Total Mass Flow Rate",
                            Constant::Units::kg_s,
                            this->TotalMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Equipment Hot Water Volume Flow Rate",
                            Constant::Units::m3_s,
                            this->HotVolFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Equipment Cold Water Volume Flow Rate",
                            Constant::Units::m3_s,
                            this->ColdVolFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Equipment Total Volume Flow Rate",
                            Constant::Units::m3_s,
                            this->TotalVolFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Equipment Hot Water Volume",
                            Constant::Units::m3,
                            this->HotVolume,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Equipment Cold Water Volume",
                            Constant::Units::m3,
                            this->ColdVolume,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Equipment Total Volume",
                            Constant::Units::m3,
                            this->TotalVolume,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name,
                            Constant::eResource::Water,
                            OutputProcessor::Group::Plant,
                            OutputProcessor::EndUseCat::WaterSystem,
                            this->EndUseSubcatName);
        SetupOutputVariable(state,
                            "Water Use Equipment Mains Water Volume",
                            Constant::Units::m3,
                            this->TotalVolume,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name,
                            Constant::eResource::MainsWater,
                            OutputProcessor::Group::Plant,
                            OutputProcessor::EndUseCat::WaterSystem,
                            this->EndUseSubcatName);

        SetupOutputVariable(state,
                            "Water Use Equipment Hot Water Temperature",
                            Constant::Units::C,
                            this->HotTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Equipment Cold Water Temperature",
                            Constant::Units::C,
                            this->ColdTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Equipment Target Water Temperature",
                            Constant::Units::C,
                            this->TargetTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Equipment Mixed Water Temperature",
                            Constant::Units::C,
                            this->MixedTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Equipment Drain Water Temperature",
                            Constant::Units::C,
                            this->DrainTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Equipment Heating Rate",
                            Constant::Units::W,
                            this->Power,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        if (this->Connections == 0) {
            SetupOutputVariable(state,
                                "Water Use Equipment Heating Energy",
                                Constant::Units::J,
                                this->Energy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::DistrictHeatingWater,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::WaterSystem,
                                this->EndUseSubcatName);

        } else if (state.dataWaterUse->WaterConnections(this->Connections).StandAlone) {
            SetupOutputVariable(state,
                                "Water Use Equipment Heating Energy",
                                Constant::Units::J,
                                this->Energy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::DistrictHeatingWater,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::WaterSystem,
                                this->EndUseSubcatName);

        } else { // The EQUIPMENT is coupled to a plant loop via a CONNECTIONS object
            SetupOutputVariable(state,
                                "Water Use Equipment Heating Energy",
                                Constant::Units::J,
                                this->Energy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::WaterSystem,
                                this->EndUseSubcatName);
        }

        if (this->Zone > 0) {
            SetupOutputVariable(state,
                                "Water Use Equipment Zone Sensible Heat Gain Rate",
                                Constant::Units::W,
                                this->SensibleRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Water Use Equipment Zone Sensible Heat Gain Energy",
                                Constant::Units::J,
                                this->SensibleEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name);

            SetupOutputVariable(state,
                                "Water Use Equipment Zone Latent Gain Rate",
                                Constant::Units::W,
                                this->LatentRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Water Use Equipment Zone Latent Gain Energy",
                                Constant::Units::J,
                                this->LatentEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name);

            SetupOutputVariable(state,
                                "Water Use Equipment Zone Moisture Gain Mass Flow Rate",
                                Constant::Units::kg_s,
                                this->MoistureRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                this->Name);
            SetupOutputVariable(state,
                                "Water Use Equipment Zone Moisture Gain Mass",
                                Constant::Units::kg,
                                this->MoistureMass,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name);

            SetupZoneInternalGain(state,
                                  this->Zone,
                                  this->Name,
                                  DataHeatBalance::IntGainType::WaterUseEquipment,
                                  &this->SensibleRateNoMultiplier,
                                  nullptr,
                                  nullptr,
                                  &this->LatentRateNoMultiplier);
        }
    }

    void WaterConnectionsType::setupOutputVars(EnergyPlusData &state)
    {
        SetupOutputVariable(state,
                            "Water Use Connections Hot Water Mass Flow Rate",
                            Constant::Units::kg_s,
                            this->HotMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Connections Cold Water Mass Flow Rate",
                            Constant::Units::kg_s,
                            this->ColdMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Connections Total Mass Flow Rate",
                            Constant::Units::kg_s,
                            this->TotalMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Connections Drain Water Mass Flow Rate",
                            Constant::Units::kg_s,
                            this->DrainMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Connections Heat Recovery Mass Flow Rate",
                            Constant::Units::kg_s,
                            this->RecoveryMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Connections Hot Water Volume Flow Rate",
                            Constant::Units::m3_s,
                            this->HotVolFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Connections Cold Water Volume Flow Rate",
                            Constant::Units::m3_s,
                            this->ColdVolFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Connections Total Volume Flow Rate",
                            Constant::Units::m3_s,
                            this->TotalVolFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Connections Hot Water Volume",
                            Constant::Units::m3,
                            this->HotVolume,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Connections Cold Water Volume",
                            Constant::Units::m3,
                            this->ColdVolume,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Connections Total Volume",
                            Constant::Units::m3,
                            this->TotalVolume,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name); //, &
        // ResourceTypeKey='Water', EndUseKey='DHW', EndUseSubKey=EndUseSubcategoryName, GroupKey='Plant')
        // tHIS WAS double counting

        SetupOutputVariable(state,
                            "Water Use Connections Hot Water Temperature",
                            Constant::Units::C,
                            this->HotTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Connections Cold Water Temperature",
                            Constant::Units::C,
                            this->ColdTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Connections Drain Water Temperature",
                            Constant::Units::C,
                            this->DrainTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Connections Return Water Temperature",
                            Constant::Units::C,
                            this->ReturnTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Connections Waste Water Temperature",
                            Constant::Units::C,
                            this->WasteTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Connections Heat Recovery Water Temperature",
                            Constant::Units::C,
                            this->RecoveryTemp,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Connections Heat Recovery Effectiveness",
                            Constant::Units::None,
                            this->Effectiveness,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);

        SetupOutputVariable(state,
                            "Water Use Connections Heat Recovery Rate",
                            Constant::Units::W,
                            this->RecoveryRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            this->Name);
        SetupOutputVariable(state,
                            "Water Use Connections Heat Recovery Energy",
                            Constant::Units::J,
                            this->RecoveryEnergy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            this->Name);
        // Does this go on a meter?

        // To do:  Add report variable for starved flow when tank can't deliver?

        if (!this->StandAlone) {
            SetupOutputVariable(state,
                                "Water Use Connections Plant Hot Water Energy",
                                Constant::Units::J,
                                this->Energy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                this->Name,
                                Constant::eResource::PlantLoopHeatingDemand,
                                OutputProcessor::Group::Plant,
                                OutputProcessor::EndUseCat::WaterSystem);
        }
    }

    void WaterEquipmentType::CalcEquipmentFlowRates(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   August 2006

        // PURPOSE OF THIS SUBROUTINE:
        // Calculate desired hot and cold water flow rates

        Real64 TempDiff;
        Real64 constexpr EPSILON(1.e-3);

        if (this->setupMyOutputVars) {
            this->setupOutputVars(state);
            this->setupMyOutputVars = false;
        }

        if (this->Connections > 0) {
            // Get water temperature conditions from the CONNECTIONS object
            this->ColdTemp = state.dataWaterUse->WaterConnections(this->Connections).ColdTemp;
            this->HotTemp = state.dataWaterUse->WaterConnections(this->Connections).HotTemp;

        } else {
            // Get water temperature conditions from the WATER USE EQUIPMENT schedules
            this->ColdTemp = (this->coldTempSched != nullptr) ? this->coldTempSched->getCurrentVal() : state.dataEnvrn->WaterMainsTemp;
            this->HotTemp = (this->hotTempSched != nullptr) ? this->hotTempSched->getCurrentVal() : this->ColdTemp;
        }

        if (this->targetTempSched != nullptr) {
            this->TargetTemp = this->targetTempSched->getCurrentVal();
        } else if (this->allowHotControl) { // If no TargetTempSchedule, but allowHotControl is set, use all hot water if applicable
            this->TargetTemp = this->HotTemp;
        } else { // If no TargetTempSchedule, use all cold water
            this->TargetTemp = this->ColdTemp;
        }

        // Get the requested total flow rate
        this->TotalVolFlowRate = this->PeakVolFlowRate;
        if (this->Zone > 0) {
            this->TotalVolFlowRate *= state.dataHeatBal->Zone(this->Zone).Multiplier * state.dataHeatBal->Zone(this->Zone).ListMultiplier;
        }
        if (this->flowRateFracSched != nullptr) {
            this->TotalVolFlowRate *= this->flowRateFracSched->getCurrentVal();
        }

        this->TotalMassFlowRate = this->TotalVolFlowRate * calcH2ODensity(state);

        // Calculate hot and cold water mixing at the tap
        if (this->TotalMassFlowRate > 0.0 && this->allowHotControl) {
            // Calculate the flow rates needed to meet the target temperature
            if (this->TargetTemp <= this->ColdTemp + EPSILON) {
                // don't need to mix (use cold) or hot water flow would be very small if within EPSILON above cold temp (use cold)
                this->HotMassFlowRate = 0.0;
                if (!state.dataGlobal->WarmupFlag && this->TargetTemp < this->ColdTemp) {
                    // print error for variables of target water temperature
                    ++this->TargetCWTempErrorCount;
                    TempDiff = this->ColdTemp - this->TargetTemp;
                    if (this->TargetCWTempErrorCount < 2) {
                        ShowWarningError(
                            state,
                            format("CalcEquipmentFlowRates: \"{}\" - Target water temperature is less than the cold water temperature by ({:.2R} C)",
                                   this->Name,
                                   TempDiff));
                        ShowContinueErrorTimeStamp(state, "");
                        ShowContinueError(state, format("...target water temperature     = {:.2R} C", this->TargetTemp));
                        ShowContinueError(state, format("...cold water temperature       = {:.2R} C", this->ColdTemp));
                        ShowContinueError(state,
                                          "...Target water temperature should be greater than or equal to the cold water temperature. "
                                          "Verify temperature setpoints and schedules.");
                    } else {
                        ShowRecurringWarningErrorAtEnd(
                            state,
                            format(
                                "\"{}\" - Target water temperature should be greater than or equal to the cold water temperature error continues...",
                                this->Name),
                            this->TargetCWTempErrIndex,
                            TempDiff,
                            TempDiff);
                    }
                }
            } else if (this->TargetTemp >= this->HotTemp) {
                // don't need to mix (use hot), or need to purge stagnant hot water temps (use hot)
                this->HotMassFlowRate = this->TotalMassFlowRate;
                if (!state.dataGlobal->WarmupFlag) {
                    // print error for variables of target water temperature
                    if (this->ColdTemp > (this->HotTemp + EPSILON)) {
                        // print error for variables of hot water temperature
                        ++this->CWHWTempErrorCount;
                        TempDiff = this->ColdTemp - this->HotTemp;
                        if (this->CWHWTempErrorCount < 2) {
                            ShowWarningError(
                                state,
                                format("CalcEquipmentFlowRates: \"{}\" - Hot water temperature is less than the cold water temperature by ({:.2R} C)",
                                       this->Name,
                                       TempDiff));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state, format("...hot water temperature        = {:.2R} C", this->HotTemp));
                            ShowContinueError(state, format("...cold water temperature       = {:.2R} C", this->ColdTemp));
                            ShowContinueError(state,
                                              "...Hot water temperature should be greater than or equal to the cold water temperature. "
                                              "Verify temperature setpoints and schedules.");
                        } else {
                            ShowRecurringWarningErrorAtEnd(
                                state,
                                format("\"{}\" - Hot water temperature should be greater than the cold water temperature error continues... ",
                                       this->Name),
                                this->CWHWTempErrIndex,
                                TempDiff,
                                TempDiff);
                        }
                    } else if (this->TargetTemp > this->HotTemp) {
                        TempDiff = this->TargetTemp - this->HotTemp;
                        ++this->TargetHWTempErrorCount;
                        if (this->TargetHWTempErrorCount < 2) {
                            ShowWarningError(state,
                                             format("CalcEquipmentFlowRates: \"{}\" - Target water temperature is greater than the hot water "
                                                    "temperature by ({:.2R} C)",
                                                    this->Name,
                                                    TempDiff));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state, format("...target water temperature     = {:.2R} C", this->TargetTemp));
                            ShowContinueError(state, format("...hot water temperature        = {:.2R} C", this->HotTemp));
                            ShowContinueError(state,
                                              "...Target water temperature should be less than or equal to the hot water temperature. "
                                              "Verify temperature setpoints and schedules.");
                        } else {
                            ShowRecurringWarningErrorAtEnd(state,
                                                           format("\"{}\" - Target water temperature should be less than or equal to the hot "
                                                                  "water temperature error continues...",
                                                                  this->Name),
                                                           this->TargetHWTempErrIndex,
                                                           TempDiff,
                                                           TempDiff);
                        }
                    }
                }
            } else {
                // Check hot less than cold temp, else calculate hot flow.
                if (this->HotTemp <= this->ColdTemp + EPSILON) {
                    // will need to avoid divide by 0 and stagnant region. Target temp is greater than ColdTemp so use hot water
                    // continue using hot water until hot water temp is EPSILON C above cold water temp (i.e., avoid very small denominator)
                    this->HotMassFlowRate = this->TotalMassFlowRate;
                    if (!state.dataGlobal->WarmupFlag && this->HotTemp < this->ColdTemp) {
                        // print error for variables of hot water temperature
                        ++this->CWHWTempErrorCount;
                        TempDiff = this->ColdTemp - this->HotTemp;
                        if (this->CWHWTempErrorCount < 2) {
                            ShowWarningError(state,
                                             format("CalcEquipmentFlowRates: \"{}\" - Hot water temperature is less than the cold water "
                                                    "temperature by ({:.2R} C)",
                                                    this->Name,
                                                    TempDiff));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state, format("...hot water temperature        = {:.2R} C", this->HotTemp));
                            ShowContinueError(state, format("...cold water temperature       = {:.2R} C", this->ColdTemp));
                            ShowContinueError(state,
                                              "...Hot water temperature should be greater than or equal to the cold water temperature. "
                                              "Verify temperature setpoints and schedules.");
                        } else {
                            ShowRecurringWarningErrorAtEnd(
                                state,
                                format("\"{}\" - Hot water temperature should be greater than the cold water temperature error continues... ",
                                       this->Name),
                                this->CWHWTempErrIndex,
                                TempDiff,
                                TempDiff);
                        }
                    }
                } else {
                    // HotMassFlowRate should always be between 0 and TotalMassFlowRate
                    this->HotMassFlowRate = this->TotalMassFlowRate * (this->TargetTemp - this->ColdTemp) / (this->HotTemp - this->ColdTemp);
                }
            }

            this->ColdMassFlowRate = this->TotalMassFlowRate - this->HotMassFlowRate;
            this->MixedTemp = (this->ColdMassFlowRate * this->ColdTemp + this->HotMassFlowRate * this->HotTemp) / this->TotalMassFlowRate;
            // there should be no out of bounds results
            assert(this->ColdMassFlowRate >= 0.0 && this->ColdMassFlowRate <= this->TotalMassFlowRate);
            assert(this->HotMassFlowRate >= 0.0 && this->HotMassFlowRate <= this->TotalMassFlowRate);
            assert(std::abs(this->HotMassFlowRate + this->ColdMassFlowRate - this->TotalMassFlowRate) < EPSILON);
        } else {
            this->HotMassFlowRate = 0.0;
            this->ColdMassFlowRate = this->TotalMassFlowRate;
            this->MixedTemp = this->TargetTemp;
        }
    }

    void WaterEquipmentType::CalcEquipmentDrainTemp(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   August 2006

        // PURPOSE OF THIS SUBROUTINE:
        // Calculate drainwater temperature and heat and moisture gains to zone.

        static constexpr std::string_view RoutineName("CalcEquipmentDrainTemp");

        this->SensibleRate = 0.0;
        this->SensibleEnergy = 0.0;
        this->LatentRate = 0.0;
        this->LatentEnergy = 0.0;

        if ((this->Zone == 0) || (this->TotalMassFlowRate == 0.0)) {
            this->DrainTemp = this->MixedTemp;
            this->DrainMassFlowRate = this->TotalMassFlowRate;

        } else {
            auto &thisZoneHB = state.dataZoneTempPredictorCorrector->zoneHeatBalance(this->Zone);

            if (this->sensibleFracSched == nullptr) {
                this->SensibleRate = 0.0;
                this->SensibleEnergy = 0.0;
            } else {
                this->SensibleRate = this->sensibleFracSched->getCurrentVal() * this->TotalMassFlowRate *
                                     Psychrometrics::CPHW(Constant::InitConvTemp) * (this->MixedTemp - thisZoneHB.MAT);
                this->SensibleEnergy = this->SensibleRate * state.dataHVACGlobal->TimeStepSysSec;
            }

            if (this->latentFracSched == nullptr) {
                this->LatentRate = 0.0;
                this->LatentEnergy = 0.0;
            } else {
                Real64 ZoneHumRat = thisZoneHB.airHumRat;
                Real64 ZoneHumRatSat = Psychrometrics::PsyWFnTdbRhPb(state,
                                                                     thisZoneHB.MAT,
                                                                     1.0,
                                                                     state.dataEnvrn->OutBaroPress,
                                                                     RoutineName); // Humidratio at 100% relative humidity
                Real64 RhoAirDry = Psychrometrics::PsyRhoAirFnPbTdbW(state, state.dataEnvrn->OutBaroPress, thisZoneHB.MAT, 0.0);
                Real64 ZoneMassMax =
                    (ZoneHumRatSat - ZoneHumRat) * RhoAirDry * state.dataHeatBal->Zone(this->Zone).Volume; // Max water that can be evaporated to zone
                Real64 FlowMassMax = this->TotalMassFlowRate * state.dataHVACGlobal->TimeStepSysSec;       // Max water in flow
                Real64 MoistureMassMax = min(ZoneMassMax, FlowMassMax);

                this->MoistureMass = this->latentFracSched->getCurrentVal() * MoistureMassMax;
                this->MoistureRate = this->MoistureMass / (state.dataHVACGlobal->TimeStepSysSec);

                this->LatentRate = this->MoistureRate * Psychrometrics::PsyHfgAirFnWTdb(ZoneHumRat, thisZoneHB.MAT);
                this->LatentEnergy = this->LatentRate * state.dataHVACGlobal->TimeStepSysSec;
            }

            this->DrainMassFlowRate = this->TotalMassFlowRate - this->MoistureRate;

            if (this->DrainMassFlowRate == 0.0) {
                this->DrainTemp = this->MixedTemp;
            } else {
                this->DrainTemp = (this->TotalMassFlowRate * Psychrometrics::CPHW(Constant::InitConvTemp) * this->MixedTemp - this->SensibleRate -
                                   this->LatentRate) /
                                  (this->DrainMassFlowRate * Psychrometrics::CPHW(Constant::InitConvTemp));
            }
        }
    }

    void WaterConnectionsType::InitConnections(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   August 2006
        //       MODIFIED       Brent Griffith 2010, demand side update

        // Set the cold water temperature
        if (this->SupplyTankNum > 0) {
            this->ColdSupplyTemp = state.dataWaterData->WaterStorage(this->SupplyTankNum).Twater;

        } else if (this->coldTempSched != nullptr) {
            this->ColdSupplyTemp = this->coldTempSched->getCurrentVal();

        } else {
            this->ColdSupplyTemp = state.dataEnvrn->WaterMainsTemp;
        }

        // Initially set ColdTemp to the ColdSupplyTemp; with heat recovery, ColdTemp will change during iteration
        this->ColdTemp = this->ColdSupplyTemp;

        // Set the hot water temperature
        if (this->StandAlone) {
            this->HotTemp = (this->hotTempSched != nullptr) ? this->hotTempSched->getCurrentVal() : this->ColdTemp;
        } else {

            if (state.dataGlobal->BeginEnvrnFlag && this->Init) {
                // Clear node initial conditions
                if (this->InletNode > 0 && this->OutletNode > 0) {
                    PlantUtilities::InitComponentNodes(state, 0.0, this->PeakMassFlowRate, this->InletNode, this->OutletNode);

                    this->ReturnTemp = state.dataLoopNodes->Node(this->InletNode).Temp;
                }

                this->Init = false;
            }

            if (!state.dataGlobal->BeginEnvrnFlag) {
                this->Init = true;
            }

            if (this->InletNode > 0) {
                if (!state.dataGlobal->DoingSizing) {
                    this->HotTemp = state.dataLoopNodes->Node(this->InletNode).Temp;
                } else {
                    // plant loop will not be running so need a value here.
                    // should change to use tank setpoint but water use connections don't have knowledge of the tank they are fed by
                    this->HotTemp = 60.0;
                }
            }
        }
    }

    void WaterConnectionsType::CalcConnectionsFlowRates(EnergyPlusData &state, bool FirstHVACIteration)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   August 2006

        // PURPOSE OF THIS SUBROUTINE:
        // Calculate summed values for WATER USE CONNECTIONS (to prepare to request flow from plant, and for reporting).

        this->ColdMassFlowRate = 0.0;
        this->HotMassFlowRate = 0.0;

        for (int Loop = 1; Loop <= this->NumWaterEquipment; ++Loop) {
            auto &thisWEq = state.dataWaterUse->WaterEquipment(this->myWaterEquipArr(Loop));

            thisWEq.CalcEquipmentFlowRates(state);

            this->ColdMassFlowRate += thisWEq.ColdMassFlowRate;
            this->HotMassFlowRate += thisWEq.HotMassFlowRate;
        } // Loop

        this->TotalMassFlowRate = this->ColdMassFlowRate + this->HotMassFlowRate;

        if (!this->StandAlone) { // Interact with the plant loop
            if (this->InletNode > 0) {
                if (FirstHVACIteration) {
                    // Request the mass flow rate from the demand side manager
                    PlantUtilities::SetComponentFlowRate(state, this->HotMassFlowRate, this->InletNode, this->OutletNode, this->plantLoc);

                } else {
                    Real64 DesiredHotWaterMassFlow = this->HotMassFlowRate;
                    PlantUtilities::SetComponentFlowRate(state, DesiredHotWaterMassFlow, this->InletNode, this->OutletNode, this->plantLoc);
                    // readjust if more than actual available mass flow rate determined by the demand side manager
                    if ((this->HotMassFlowRate != DesiredHotWaterMassFlow) && (this->HotMassFlowRate > 0.0)) { // plant didn't give what was asked for

                        Real64 AvailableFraction = DesiredHotWaterMassFlow / this->HotMassFlowRate;

                        this->ColdMassFlowRate = this->TotalMassFlowRate - this->HotMassFlowRate; // Preserve the total mass flow rate

                        // Proportionally reduce hot water and increase cold water for all WATER USE EQUIPMENT
                        for (int Loop = 1; Loop <= this->NumWaterEquipment; ++Loop) {
                            auto &thisWEq = state.dataWaterUse->WaterEquipment(this->myWaterEquipArr(Loop));

                            // Recalculate flow rates for water equipment within connection
                            thisWEq.HotMassFlowRate *= AvailableFraction;
                            thisWEq.ColdMassFlowRate = thisWEq.TotalMassFlowRate - thisWEq.HotMassFlowRate;

                            // Recalculate mixed water temperature
                            if (thisWEq.TotalMassFlowRate > 0.0) {
                                thisWEq.MixedTemp = (thisWEq.ColdMassFlowRate * thisWEq.ColdTemp + thisWEq.HotMassFlowRate * thisWEq.HotTemp) /
                                                    thisWEq.TotalMassFlowRate;
                            } else {
                                thisWEq.MixedTemp = thisWEq.TargetTemp;
                            }
                        } // Loop
                    }
                }
            }
        }

        if (this->SupplyTankNum > 0) {
            // Set the demand request for supply water from water storage tank
            this->ColdVolFlowRate = this->ColdMassFlowRate / calcH2ODensity(state);
            state.dataWaterData->WaterStorage(this->SupplyTankNum).VdotRequestDemand(this->TankDemandID) = this->ColdVolFlowRate;

            // Check if cold flow rate should be starved by restricted flow from tank
            // Currently, the tank flow is not really starved--water continues to flow at the tank water temperature
            // But the user can see the error by comparing report variables for TankVolFlowRate < ColdVolFlowRate
            this->TankVolFlowRate = state.dataWaterData->WaterStorage(this->SupplyTankNum).VdotAvailDemand(this->TankDemandID);
            this->TankMassFlowRate = this->TankVolFlowRate * calcH2ODensity(state);
        }
    }

    void WaterConnectionsType::CalcConnectionsDrainTemp(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   August 2006

        Real64 MassFlowTempSum = 0.0;
        this->DrainMassFlowRate = 0.0;

        for (int Loop = 1; Loop <= this->NumWaterEquipment; ++Loop) {
            auto &thisWEq = state.dataWaterUse->WaterEquipment(this->myWaterEquipArr(Loop));

            thisWEq.CalcEquipmentDrainTemp(state);

            this->DrainMassFlowRate += thisWEq.DrainMassFlowRate;
            MassFlowTempSum += thisWEq.DrainMassFlowRate * thisWEq.DrainTemp;
        } // Loop

        if (this->DrainMassFlowRate > 0.0) {
            this->DrainTemp = MassFlowTempSum / this->DrainMassFlowRate;
        } else {
            this->DrainTemp = this->HotTemp;
        }

        this->DrainVolFlowRate = this->DrainMassFlowRate * calcH2ODensity(state);
    }

    void WaterConnectionsType::CalcConnectionsHeatRecovery(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   August 2006

        // PURPOSE OF THIS SUBROUTINE:
        // Calculate drainwater heat recovery

        if (!this->HeatRecovery) {
            this->RecoveryTemp = this->ColdSupplyTemp;
            this->ReturnTemp = this->ColdSupplyTemp;
            this->WasteTemp = this->DrainTemp;

        } else if (this->TotalMassFlowRate == 0.0) {
            this->Effectiveness = 0.0;
            this->RecoveryRate = 0.0;
            this->RecoveryTemp = this->ColdSupplyTemp;
            this->ReturnTemp = this->ColdSupplyTemp;
            this->WasteTemp = this->DrainTemp;

        } else { // WaterConnections(WaterConnNum)%TotalMassFlowRate > 0.0

            switch (this->HeatRecoveryConfig) {
            case HeatRecovConfig::Plant: {
                this->RecoveryMassFlowRate = this->HotMassFlowRate;
            } break;
            case HeatRecovConfig::Equipment: {
                this->RecoveryMassFlowRate = this->ColdMassFlowRate;
            } break;
            case HeatRecovConfig::PlantAndEquip: {
                this->RecoveryMassFlowRate = this->TotalMassFlowRate;
            } break;
            default:
                break;
            }

            Real64 HXCapacityRate = Psychrometrics::CPHW(Constant::InitConvTemp) * this->RecoveryMassFlowRate;
            Real64 DrainCapacityRate = Psychrometrics::CPHW(Constant::InitConvTemp) * this->DrainMassFlowRate;
            Real64 MinCapacityRate = min(DrainCapacityRate, HXCapacityRate);

            switch (this->HeatRecoveryHX) {
            case HeatRecovHX::Ideal: {
                this->Effectiveness = 1.0;
            } break;
            case HeatRecovHX::CounterFlow: { // Unmixed
                Real64 CapacityRatio = MinCapacityRate / max(DrainCapacityRate, HXCapacityRate);
                Real64 NTU = this->HXUA / MinCapacityRate;
                if (CapacityRatio == 1.0) {
                    this->Effectiveness = NTU / (1.0 + NTU);
                } else {
                    Real64 ExpVal = std::exp(-NTU * (1.0 - CapacityRatio));
                    this->Effectiveness = (1.0 - ExpVal) / (1.0 - CapacityRatio * ExpVal);
                }
            } break;
            case HeatRecovHX::CrossFlow: { // Unmixed
                Real64 CapacityRatio = MinCapacityRate / max(DrainCapacityRate, HXCapacityRate);
                Real64 NTU = this->HXUA / MinCapacityRate;
                this->Effectiveness = 1.0 - std::exp((std::pow(NTU, 0.22) / CapacityRatio) * (std::exp(-CapacityRatio * std::pow(NTU, 0.78)) - 1.0));
            } break;
            default:
                break;
            }

            this->RecoveryRate = this->Effectiveness * MinCapacityRate * (this->DrainTemp - this->ColdSupplyTemp);
            this->RecoveryTemp = this->ColdSupplyTemp + this->RecoveryRate / (Psychrometrics::CPHW(Constant::InitConvTemp) * this->TotalMassFlowRate);
            this->WasteTemp = this->DrainTemp - this->RecoveryRate / (Psychrometrics::CPHW(Constant::InitConvTemp) * this->TotalMassFlowRate);

            if (this->RecoveryTankNum > 0) {
                state.dataWaterData->WaterStorage(this->RecoveryTankNum).VdotAvailSupply(this->TankSupplyID) = this->DrainVolFlowRate;
                state.dataWaterData->WaterStorage(this->RecoveryTankNum).TwaterSupply(this->TankSupplyID) = this->WasteTemp;
            }

            switch (this->HeatRecoveryConfig) {
            case HeatRecovConfig::Plant: {
                this->TempError = 0.0; // No feedback back to the cold supply
                this->ReturnTemp = this->RecoveryTemp;
            } break;
            case HeatRecovConfig::Equipment: {
                this->TempError = std::abs(this->ColdTemp - this->RecoveryTemp);

                this->ColdTemp = this->RecoveryTemp;
                this->ReturnTemp = this->ColdSupplyTemp;
            } break;
            case HeatRecovConfig::PlantAndEquip: {
                this->TempError = std::abs(this->ColdTemp - this->RecoveryTemp);

                this->ColdTemp = this->RecoveryTemp;
                this->ReturnTemp = this->RecoveryTemp;
            } break;
            default:
                break;
            }
        }
    }

    void WaterConnectionsType::UpdateWaterConnections(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   August 2006

        // PURPOSE OF THIS SUBROUTINE:
        // Updates the node variables with local variables.

        if (this->InletNode > 0 && this->OutletNode > 0) {
            // Pass all variables from inlet to outlet node
            PlantUtilities::SafeCopyPlantNode(state, this->InletNode, this->OutletNode, this->plantLoc.loopNum);

            // Set outlet node variables that are possibly changed
            state.dataLoopNodes->Node(this->OutletNode).Temp = this->ReturnTemp;
            // should add enthalpy update to return?
        }
    }

    void ReportStandAloneWaterUse(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith, Peter Graham Ellis
        //       DATE WRITTEN   Nov. 2011
        //       MODIFIED       Brent Griffith, March 2010 added argument

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates report variables for stand alone water use

        for (int WaterEquipNum = 1; WaterEquipNum <= state.dataWaterUse->numWaterEquipment; ++WaterEquipNum) {
            auto &thisWEq = state.dataWaterUse->WaterEquipment(WaterEquipNum);
            thisWEq.ColdVolFlowRate = thisWEq.ColdMassFlowRate / calcH2ODensity(state);
            thisWEq.HotVolFlowRate = thisWEq.HotMassFlowRate / calcH2ODensity(state);
            thisWEq.TotalVolFlowRate = thisWEq.ColdVolFlowRate + thisWEq.HotVolFlowRate;

            thisWEq.ColdVolume = thisWEq.ColdVolFlowRate * state.dataHVACGlobal->TimeStepSysSec;
            thisWEq.HotVolume = thisWEq.HotVolFlowRate * state.dataHVACGlobal->TimeStepSysSec;
            thisWEq.TotalVolume = thisWEq.TotalVolFlowRate * state.dataHVACGlobal->TimeStepSysSec;

            if (thisWEq.Connections == 0) {
                thisWEq.Power = thisWEq.HotMassFlowRate * Psychrometrics::CPHW(Constant::InitConvTemp) * (thisWEq.HotTemp - thisWEq.ColdTemp);
            } else {
                thisWEq.Power = thisWEq.HotMassFlowRate * Psychrometrics::CPHW(Constant::InitConvTemp) *
                                (thisWEq.HotTemp - state.dataWaterUse->WaterConnections(thisWEq.Connections).ReturnTemp);
            }

            thisWEq.Energy = thisWEq.Power * state.dataHVACGlobal->TimeStepSysSec;
        }
    }

    void WaterConnectionsType::ReportWaterUse(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   August 2006
        //       MODIFIED       Brent Griffith, March 2010 added argument

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates report variables.

        for (int Loop = 1; Loop <= this->NumWaterEquipment; ++Loop) {

            auto &thisWEq = state.dataWaterUse->WaterEquipment(this->myWaterEquipArr(Loop));

            thisWEq.ColdVolFlowRate = thisWEq.ColdMassFlowRate / calcH2ODensity(state);
            thisWEq.HotVolFlowRate = thisWEq.HotMassFlowRate / calcH2ODensity(state);
            thisWEq.TotalVolFlowRate = thisWEq.ColdVolFlowRate + thisWEq.HotVolFlowRate;
            thisWEq.ColdVolume = thisWEq.ColdVolFlowRate * state.dataHVACGlobal->TimeStepSysSec;
            thisWEq.HotVolume = thisWEq.HotVolFlowRate * state.dataHVACGlobal->TimeStepSysSec;
            thisWEq.TotalVolume = thisWEq.TotalVolFlowRate * state.dataHVACGlobal->TimeStepSysSec;

            if (thisWEq.Connections == 0) {
                thisWEq.Power = thisWEq.HotMassFlowRate * Psychrometrics::CPHW(Constant::InitConvTemp) * (thisWEq.HotTemp - thisWEq.ColdTemp);
            } else {
                thisWEq.Power = thisWEq.HotMassFlowRate * Psychrometrics::CPHW(Constant::InitConvTemp) *
                                (thisWEq.HotTemp - state.dataWaterUse->WaterConnections(thisWEq.Connections).ReturnTemp);
            }

            thisWEq.Energy = thisWEq.Power * state.dataHVACGlobal->TimeStepSysSec;
        }

        this->ColdVolFlowRate = this->ColdMassFlowRate / calcH2ODensity(state);
        this->HotVolFlowRate = this->HotMassFlowRate / calcH2ODensity(state);
        this->TotalVolFlowRate = this->ColdVolFlowRate + this->HotVolFlowRate;
        this->ColdVolume = this->ColdVolFlowRate * state.dataHVACGlobal->TimeStepSysSec;
        this->HotVolume = this->HotVolFlowRate * state.dataHVACGlobal->TimeStepSysSec;
        this->TotalVolume = this->TotalVolFlowRate * state.dataHVACGlobal->TimeStepSysSec;
        this->Power = this->HotMassFlowRate * Psychrometrics::CPHW(Constant::InitConvTemp) * (this->HotTemp - this->ReturnTemp);
        this->Energy = this->Power * state.dataHVACGlobal->TimeStepSysSec;
        this->RecoveryEnergy = this->RecoveryRate * state.dataHVACGlobal->TimeStepSysSec;
    }
    void WaterConnectionsType::oneTimeInit_new(EnergyPlusData &state)
    {

        this->setupOutputVars(state);

        if (allocated(state.dataPlnt->PlantLoop) && !this->StandAlone) {
            bool errFlag = false;
            PlantUtilities::ScanPlantLoopsForObject(
                state, this->Name, DataPlant::PlantEquipmentType::WaterUseConnection, this->plantLoc, errFlag, _, _, _, _, _);
            if (errFlag) {
                ShowFatalError(state, "InitConnections: Program terminated due to previous condition(s).");
            }
        }
    }
    void WaterConnectionsType::oneTimeInit([[maybe_unused]] EnergyPlusData &state)
    {
    }

    void CalcWaterUseZoneGains(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Peter Graham Ellis
        //       DATE WRITTEN   August 2006

        // PURPOSE OF THIS SUBROUTINE:
        // Calculates the zone internal gains due to water use sensible and latent loads.

        static bool MyEnvrnFlagLocal = true;

        if (state.dataWaterUse->numWaterEquipment == 0) {
            return;
        }

        if (state.dataGlobal->BeginEnvrnFlag && MyEnvrnFlagLocal) {
            for (auto &e : state.dataWaterUse->WaterEquipment) {
                e.SensibleRate = 0.0;
                e.SensibleEnergy = 0.0;
                e.SensibleRateNoMultiplier = 0.0;
                e.LatentRate = 0.0;
                e.LatentEnergy = 0.0;
                e.LatentRateNoMultiplier = 0.0;
                e.MixedTemp = 0.0;
                e.TotalMassFlowRate = 0.0;
                e.DrainTemp = 0.0;
                e.ColdVolFlowRate = 0.0;
                e.HotVolFlowRate = 0.0;
                e.TotalVolFlowRate = 0.0;
                e.ColdMassFlowRate = 0.0;
                e.HotMassFlowRate = 0.0;
            }
            MyEnvrnFlagLocal = false;
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            MyEnvrnFlagLocal = true;
        }

        for (int WaterEquipNum = 1; WaterEquipNum <= state.dataWaterUse->numWaterEquipment; ++WaterEquipNum) {
            if (state.dataWaterUse->WaterEquipment(WaterEquipNum).Zone == 0) {
                continue;
            }
            int ZoneNum = state.dataWaterUse->WaterEquipment(WaterEquipNum).Zone;
            state.dataWaterUse->WaterEquipment(WaterEquipNum).SensibleRateNoMultiplier =
                state.dataWaterUse->WaterEquipment(WaterEquipNum).SensibleRate /
                (state.dataHeatBal->Zone(ZoneNum).Multiplier * state.dataHeatBal->Zone(ZoneNum).ListMultiplier);
            state.dataWaterUse->WaterEquipment(WaterEquipNum).LatentRateNoMultiplier =
                state.dataWaterUse->WaterEquipment(WaterEquipNum).LatentRate /
                (state.dataHeatBal->Zone(ZoneNum).Multiplier * state.dataHeatBal->Zone(ZoneNum).ListMultiplier);
        }
    }

    Real64 calcH2ODensity(EnergyPlusData &state)
    {
        static constexpr std::string_view RoutineName{"calcH2ODensity"};

        if (state.dataWaterUse->calcRhoH2O) {
            state.dataWaterUse->rhoH2OStd = Fluid::GetWater(state)->getDensity(state, Constant::InitConvTemp, RoutineName);
            state.dataWaterUse->calcRhoH2O = false;
        }
        return state.dataWaterUse->rhoH2OStd;
    }

} // namespace WaterUse
} // namespace EnergyPlus

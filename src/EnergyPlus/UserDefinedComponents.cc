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
#include <cmath>
#include <format>

// EnergyPlus Headers
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataRuntimeLanguage.hh>
#include <EnergyPlus/DataWater.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HeatBalanceInternalHeatGains.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/Plant/PlantLocation.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/PluginManager.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/UserDefinedComponents.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterManager.hh>

namespace EnergyPlus {

namespace UserDefinedComponents {

    // Module containing the routines dealing with the User Defined HVAC and Plant component models

    // MODULE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   January 2012
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // Collect component models for custom program with Erl.

    PlantComponent *UserPlantComponentStruct::factory(EnergyPlusData &state, std::string const &objectName)
    {
        // Process the input data
        if (state.dataUserDefinedComponents->GetPlantCompInput) {
            GetUserDefinedPlantComponents(state);
            state.dataUserDefinedComponents->GetPlantCompInput = false;
        }
        // Now look for this particular object
        for (auto &thisComp : state.dataUserDefinedComponents->UserPlantComp) {
            if (thisComp.Name == objectName) {
                return &thisComp;
            }
        }
        // If we didn't find it, fatal
        ShowFatalError(state,
                       std::format("LocalUserDefinedPlantComponentFactory: Error getting inputs for object named: {}", objectName)); // LCOV_EXCL_LINE
    }

    void UserPlantComponentStruct::onInitLoopEquip(EnergyPlusData &state, const PlantLocation &calledFromLocation)
    {
        Real64 myLoad = 0.0;
        int thisLoop = -1;

        this->oneTimeInit(state);

        for (int loop = 0; loop < this->NumPlantConnections; ++loop) {
            if (calledFromLocation.loopNum != this->Loop[loop].plantLoc.loopNum) {
                continue;
            }
            if (calledFromLocation.loopSideNum != this->Loop[loop].plantLoc.loopSideNum) {
                continue;
            }
            thisLoop = loop;
            break;
        }

        if (thisLoop >= 0) {
            this->initialize(state, thisLoop, myLoad);

            auto &plantConnection = this->Loop[thisLoop];

            if (plantConnection.ErlInitProgramMngr > 0) {
                bool anyEMSRan;
                EMSManager::ManageEMS(state, EMSManager::EMSCallFrom::UserDefinedComponentModel, anyEMSRan, plantConnection.ErlInitProgramMngr);
            } else if (plantConnection.initPluginLocation > -1) {
                state.dataPluginManager->pluginManager->runSingleUserDefinedPlugin(state, plantConnection.initPluginLocation);
            } else if (plantConnection.initCallbackIndex > -1) {
                state.dataPluginManager->pluginManager->runSingleUserDefinedCallback(state, plantConnection.initCallbackIndex);
            }

            PlantUtilities::InitComponentNodes(
                state, plantConnection.MassFlowRateMin, plantConnection.MassFlowRateMax, plantConnection.InletNodeNum, plantConnection.OutletNodeNum);

            PlantUtilities::RegisterPlantCompDesignFlow(state, plantConnection.InletNodeNum, plantConnection.DesignVolumeFlowRate);

        } else {
            // throw warning
            ShowFatalError(
                state,
                std::format("SimUserDefinedPlantComponent: did not find where called from. Loop number called from ={}, loop side called from ={}.",
                            calledFromLocation.loopNum,
                            DataPlant::DemandSupplyNames[static_cast<int>(calledFromLocation.loopSideNum)]));
        }
    }

    void UserPlantComponentStruct::getDesignCapacities(
        [[maybe_unused]] EnergyPlusData &state, const PlantLocation &calledFromLocation, Real64 &MaxLoad, Real64 &MinLoad, Real64 &OptLoad)
    {
        int thisLoop = -1;
        for (int loop = 0; loop < this->NumPlantConnections; ++loop) {
            if (calledFromLocation.loopNum != this->Loop[loop].plantLoc.loopNum) {
                continue;
            }
            if (calledFromLocation.loopSideNum != this->Loop[loop].plantLoc.loopSideNum) {
                continue;
            }
            thisLoop = loop;
            break;
        }

        if (thisLoop < 0) {
            ShowFatalError(state,
                           std::format("SimUserDefinedPlantComponent: did not find plant connection for {}. Loop number called from ={}, "
                                       "loop side called from ={}.",
                                       this->Name,
                                       calledFromLocation.loopNum,
                                       DataPlant::DemandSupplyNames[static_cast<int>(calledFromLocation.loopSideNum)]));
        }

        auto const &plantConnection = this->Loop[thisLoop];
        MinLoad = plantConnection.MinLoad;
        MaxLoad = plantConnection.MaxLoad;
        OptLoad = plantConnection.OptLoad;
    }

    void UserPlantComponentStruct::UserPlantComponentStruct::simulate(EnergyPlusData &state,
                                                                      const EnergyPlus::PlantLocation &calledFromLocation,
                                                                      [[maybe_unused]] bool FirstHVACIteration,
                                                                      Real64 &CurLoad,
                                                                      [[maybe_unused]] bool RunFlag)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Jan 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // User Defined plant generic component

        if (state.dataGlobal->BeginEnvrnFlag) {
            this->onInitLoopEquip(state, calledFromLocation);
        }

        bool anyEMSRan;
        int thisLoop = -1;

        for (int loop = 0; loop < this->NumPlantConnections; ++loop) {
            if (calledFromLocation.loopNum != this->Loop[loop].plantLoc.loopNum) {
                continue;
            }
            if (calledFromLocation.loopSideNum != this->Loop[loop].plantLoc.loopSideNum) {
                continue;
            }
            thisLoop = loop;
            break;
        }

        if (thisLoop < 0) {
            ShowFatalError(state,
                           std::format("SimUserDefinedPlantComponent: did not find plant connection for {}. Loop number called from ={}, "
                                       "loop side called from ={}.",
                                       this->Name,
                                       calledFromLocation.loopNum,
                                       DataPlant::DemandSupplyNames[static_cast<int>(calledFromLocation.loopSideNum)]));
        }

        this->initialize(state, thisLoop, CurLoad);

        auto &plantConnection = this->Loop[thisLoop];

        if (plantConnection.ErlSimProgramMngr > 0) {
            EMSManager::ManageEMS(state, EMSManager::EMSCallFrom::UserDefinedComponentModel, anyEMSRan, plantConnection.ErlSimProgramMngr);
        } else if (plantConnection.simPluginLocation > -1) {
            state.dataPluginManager->pluginManager->runSingleUserDefinedPlugin(state, plantConnection.simPluginLocation);
        } else if (plantConnection.simCallbackIndex > -1) {
            state.dataPluginManager->pluginManager->runSingleUserDefinedCallback(state, plantConnection.simCallbackIndex);
        }

        if (this->ErlSimProgramMngr > 0) {
            EMSManager::ManageEMS(state, EMSManager::EMSCallFrom::UserDefinedComponentModel, anyEMSRan, this->ErlSimProgramMngr);
        } else if (this->simPluginLocation > -1) {
            state.dataPluginManager->pluginManager->runSingleUserDefinedPlugin(state, this->simPluginLocation);
        } else if (this->simCallbackIndex > -1) {
            state.dataPluginManager->pluginManager->runSingleUserDefinedCallback(state, this->simCallbackIndex);
        }

        this->report(state, thisLoop);
    }

    void SimCoilUserDefined(EnergyPlusData &state,
                            std::string_view EquipName, // user name for component
                            int &CompIndex,
                            int const AirLoopNum,
                            bool &HeatingActive,
                            bool &CoolingActive)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Feb. 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        int CompNum;

        if (state.dataUserDefinedComponents->GetPlantCompInput) {
            GetUserDefinedPlantComponents(state);
            state.dataUserDefinedComponents->GetPlantCompInput = false;
        }

        // Find the correct Equipment
        if (CompIndex == 0) {
            CompNum = Util::FindItemInList(EquipName, state.dataUserDefinedComponents->UserCoil);
            if (CompNum == 0) {
                ShowFatalError(state, "SimUserDefinedPlantComponent: User Defined Coil not found");
            }
            CompIndex = CompNum;
        } else {
            CompNum = CompIndex;
            if (CompNum < 1 || CompNum > state.dataUserDefinedComponents->NumUserCoils) {
                ShowFatalError(state,
                               std::format("SimUserDefinedPlantComponent: Invalid CompIndex passed={}, Number of units ={}, Entered Unit name = {}",
                                           CompNum,
                                           state.dataUserDefinedComponents->NumUserCoils,
                                           EquipName));
            }
            if (state.dataUserDefinedComponents->CheckUserCoilName[CompNum - 1]) {
                if (EquipName != state.dataUserDefinedComponents->UserCoil(CompNum).Name) {
                    ShowFatalError(
                        state,
                        std::format("SimUserDefinedPlantComponent: Invalid CompIndex passed={}, Unit name={}, stored unit name for that index={}",
                                    CompNum,
                                    EquipName,
                                    state.dataUserDefinedComponents->UserCoil(CompNum).Name));
                }
                state.dataUserDefinedComponents->CheckUserCoilName[CompNum - 1] = false;
            }
        }
        bool anyEMSRan;
        if (state.dataGlobal->BeginEnvrnFlag) {
            if (state.dataUserDefinedComponents->UserCoil(CompNum).ErlInitProgramMngr > 0) {
                EMSManager::ManageEMS(state,
                                      EMSManager::EMSCallFrom::UserDefinedComponentModel,
                                      anyEMSRan,
                                      state.dataUserDefinedComponents->UserCoil(CompNum).ErlInitProgramMngr);
            } else if (state.dataUserDefinedComponents->UserCoil(CompNum).initPluginLocation > -1) {
                state.dataPluginManager->pluginManager->runSingleUserDefinedPlugin(
                    state, state.dataUserDefinedComponents->UserCoil(CompNum).initPluginLocation);
            } else if (state.dataUserDefinedComponents->UserCoil(CompNum).initCallbackIndex > -1) {
                state.dataPluginManager->pluginManager->runSingleUserDefinedCallback(
                    state, state.dataUserDefinedComponents->UserCoil(CompNum).initCallbackIndex);
            }

            if (state.dataUserDefinedComponents->UserCoil(CompNum).PlantIsConnected) {

                PlantUtilities::InitComponentNodes(state,
                                                   state.dataUserDefinedComponents->UserCoil(CompNum).Loop.MassFlowRateMin,
                                                   state.dataUserDefinedComponents->UserCoil(CompNum).Loop.MassFlowRateMax,
                                                   state.dataUserDefinedComponents->UserCoil(CompNum).Loop.InletNodeNum,
                                                   state.dataUserDefinedComponents->UserCoil(CompNum).Loop.OutletNodeNum);

                PlantUtilities::RegisterPlantCompDesignFlow(state,
                                                            state.dataUserDefinedComponents->UserCoil(CompNum).Loop.InletNodeNum,
                                                            state.dataUserDefinedComponents->UserCoil(CompNum).Loop.DesignVolumeFlowRate);
            }
        }

        state.dataUserDefinedComponents->UserCoil(CompNum).initialize(state);

        if (state.dataUserDefinedComponents->UserCoil(CompNum).ErlSimProgramMngr > 0) {
            EMSManager::ManageEMS(state,
                                  EMSManager::EMSCallFrom::UserDefinedComponentModel,
                                  anyEMSRan,
                                  state.dataUserDefinedComponents->UserCoil(CompNum).ErlSimProgramMngr);
        } else if (state.dataUserDefinedComponents->UserCoil(CompNum).simPluginLocation > -1) {
            state.dataPluginManager->pluginManager->runSingleUserDefinedPlugin(state,
                                                                               state.dataUserDefinedComponents->UserCoil(CompNum).simPluginLocation);
        } else if (state.dataUserDefinedComponents->UserCoil(CompNum).simCallbackIndex > -1) {
            state.dataPluginManager->pluginManager->runSingleUserDefinedCallback(state,
                                                                                 state.dataUserDefinedComponents->UserCoil(CompNum).simCallbackIndex);
        }

        state.dataUserDefinedComponents->UserCoil(CompNum).report(state);

        if (AirLoopNum != -1) { // IF the system is not an equipment of outdoor air unit
            auto const &primaryAirConnection = state.dataUserDefinedComponents->UserCoil(CompNum).AirConnections[primaryConnIdx];
            // determine if heating or cooling on primary air stream
            HeatingActive = state.dataLoopNodes->Node(primaryAirConnection.InletNodeNum).Temp <
                            state.dataLoopNodes->Node(primaryAirConnection.OutletNodeNum).Temp;

            Real64 EnthInlet = Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(primaryAirConnection.InletNodeNum).Temp,
                                                          state.dataLoopNodes->Node(primaryAirConnection.InletNodeNum).HumRat);
            Real64 EnthOutlet = Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(primaryAirConnection.OutletNodeNum).Temp,
                                                           state.dataLoopNodes->Node(primaryAirConnection.OutletNodeNum).HumRat);
            CoolingActive = EnthInlet > EnthOutlet;
        }
    }

    void SimZoneAirUserDefined(EnergyPlusData &state,
                               std::string_view CompName,      // name of the packaged terminal heat pump
                               int const ZoneNum,              // number of zone being served
                               Real64 &SensibleOutputProvided, // sensible capacity delivered to zone
                               Real64 &LatentOutputProvided,   // Latent add/removal  (kg/s), dehumid = negative
                               int &CompIndex                  // index to zone hvac unit
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   February, 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        int CompNum;

        if (state.dataUserDefinedComponents->GetInput) {
            GetUserDefinedComponents(state);
            state.dataUserDefinedComponents->GetInput = false;
        }

        // Find the correct Equipment
        if (CompIndex == 0) {
            CompNum = Util::FindItemInList(CompName, state.dataUserDefinedComponents->UserZoneAirHVAC);
            if (CompNum == 0) {
                ShowFatalError(state, "SimUserDefinedPlantComponent: User Defined Coil not found");
            }
            CompIndex = CompNum;
        } else {
            CompNum = CompIndex;
            if (CompNum < 1 || CompNum > state.dataUserDefinedComponents->NumUserZoneAir) {
                ShowFatalError(state,
                               std::format("SimUserDefinedPlantComponent: Invalid CompIndex passed={}, Number of units ={}, Entered Unit name = {}",
                                           CompNum,
                                           state.dataUserDefinedComponents->NumUserZoneAir,
                                           CompName));
            }
            if (state.dataUserDefinedComponents->CheckUserZoneAirName[CompNum - 1]) {
                if (CompName != state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).Name) {
                    ShowFatalError(
                        state,
                        std::format("SimUserDefinedPlantComponent: Invalid CompIndex passed={}, Unit name={}, stored unit name for that index={}",
                                    CompNum,
                                    CompName,
                                    state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).Name));
                }
                state.dataUserDefinedComponents->CheckUserZoneAirName[CompNum - 1] = false;
            }
        }
        bool anyEMSRan;
        if (state.dataGlobal->BeginEnvrnFlag) {
            state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).initialize(state, ZoneNum);

            if (state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).ErlInitProgramMngr > 0) {
                EMSManager::ManageEMS(state,
                                      EMSManager::EMSCallFrom::UserDefinedComponentModel,
                                      anyEMSRan,
                                      state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).ErlInitProgramMngr);
            } else if (state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).initPluginLocation > -1) {
                state.dataPluginManager->pluginManager->runSingleUserDefinedPlugin(
                    state, state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).initPluginLocation);
            } else if (state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).initCallbackIndex > -1) {
                state.dataPluginManager->pluginManager->runSingleUserDefinedCallback(
                    state, state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).initCallbackIndex);
            }
            if (state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).NumPlantConnections > 0) {
                for (int loop = 0; loop < state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).NumPlantConnections; ++loop) {
                    auto const &plantConnection = state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).Loop[loop];

                    PlantUtilities::InitComponentNodes(state,
                                                       plantConnection.MassFlowRateMin,
                                                       plantConnection.MassFlowRateMax,
                                                       plantConnection.InletNodeNum,
                                                       plantConnection.OutletNodeNum);

                    PlantUtilities::RegisterPlantCompDesignFlow(state, plantConnection.InletNodeNum, plantConnection.DesignVolumeFlowRate);
                }
            }

        } // BeginEnvrnFlag

        state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).initialize(state, ZoneNum);

        if (state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).ErlSimProgramMngr > 0) {
            EMSManager::ManageEMS(state,
                                  EMSManager::EMSCallFrom::UserDefinedComponentModel,
                                  anyEMSRan,
                                  state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).ErlSimProgramMngr);
        } else if (state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).simPluginLocation > -1) {
            state.dataPluginManager->pluginManager->runSingleUserDefinedPlugin(
                state, state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).simPluginLocation);
        } else if (state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).simCallbackIndex > -1) {
            state.dataPluginManager->pluginManager->runSingleUserDefinedCallback(
                state, state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).simCallbackIndex);
        }

        state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).report(state);

        // calculate delivered capacity
        Real64 AirMassFlow =
            min(state.dataLoopNodes->Node(state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).AirConnection.InletNodeNum).MassFlowRate,
                state.dataLoopNodes->Node(state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).AirConnection.OutletNodeNum).MassFlowRate);
        // calculate sensible load met using delta enthalpy at a constant (minimum) humidity ratio)
        Real64 MinHumRat =
            min(state.dataLoopNodes->Node(state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).AirConnection.InletNodeNum).HumRat,
                state.dataLoopNodes->Node(state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).AirConnection.OutletNodeNum).HumRat);
        SensibleOutputProvided =
            AirMassFlow *
            (Psychrometrics::PsyHFnTdbW(
                 state.dataLoopNodes->Node(state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).AirConnection.OutletNodeNum).Temp, MinHumRat) -
             Psychrometrics::PsyHFnTdbW(
                 state.dataLoopNodes->Node(state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).AirConnection.InletNodeNum).Temp, MinHumRat));

        Real64 SpecHumOut = state.dataLoopNodes->Node(state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).AirConnection.OutletNodeNum).HumRat;
        Real64 SpecHumIn = state.dataLoopNodes->Node(state.dataUserDefinedComponents->UserZoneAirHVAC(CompNum).AirConnection.InletNodeNum).HumRat;
        LatentOutputProvided = AirMassFlow * (SpecHumOut - SpecHumIn); // Latent rate, kg/s (dehumid = negative)
    }

    void SimAirTerminalUserDefined(EnergyPlusData &state,
                                   std::string_view CompName,
                                   [[maybe_unused]] bool const FirstHVACIteration,
                                   int const ZoneNum,
                                   [[maybe_unused]] int const ZoneNodeNum,
                                   int &CompIndex)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   March 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // simulation call for generic air terminal

        int CompNum;

        if (state.dataUserDefinedComponents->GetAirTerminalInput) {
            GetUserDefinedAirComponent(state);
            state.dataUserDefinedComponents->GetAirTerminalInput = false;
        }

        // Find the correct Equipment
        if (CompIndex == 0) {
            CompNum = Util::FindItemInList(CompName, state.dataUserDefinedComponents->UserAirTerminal);
            if (CompNum == 0) {
                ShowFatalError(state, "SimUserDefinedPlantComponent: User Defined Coil not found");
            }
            CompIndex = CompNum;
        } else {
            CompNum = CompIndex;
            if (CompNum < 1 || CompNum > state.dataUserDefinedComponents->NumUserAirTerminals) {
                ShowFatalError(state,
                               std::format("SimUserDefinedPlantComponent: Invalid CompIndex passed={}, Number of units ={}, Entered Unit name = {}",
                                           CompNum,
                                           state.dataUserDefinedComponents->NumUserAirTerminals,
                                           CompName));
            }
            if (state.dataUserDefinedComponents->CheckUserAirTerminal[CompNum - 1]) {
                if (CompName != state.dataUserDefinedComponents->UserAirTerminal(CompNum).Name) {
                    ShowFatalError(
                        state,
                        std::format("SimUserDefinedPlantComponent: Invalid CompIndex passed={}, Unit name={}, stored unit name for that index={}",
                                    CompNum,
                                    CompName,
                                    state.dataUserDefinedComponents->UserAirTerminal(CompNum).Name));
                }
                state.dataUserDefinedComponents->CheckUserAirTerminal[CompNum - 1] = false;
            }
        }
        bool anyEMSRan;
        if (state.dataGlobal->BeginEnvrnFlag) {
            state.dataUserDefinedComponents->UserAirTerminal(CompNum).initialize(state, ZoneNum);

            if (state.dataUserDefinedComponents->UserAirTerminal(CompNum).ErlInitProgramMngr > 0) {
                EMSManager::ManageEMS(state,
                                      EMSManager::EMSCallFrom::UserDefinedComponentModel,
                                      anyEMSRan,
                                      state.dataUserDefinedComponents->UserAirTerminal(CompNum).ErlInitProgramMngr);
            } else if (state.dataUserDefinedComponents->UserAirTerminal(CompNum).initPluginLocation > -1) {
                state.dataPluginManager->pluginManager->runSingleUserDefinedPlugin(
                    state, state.dataUserDefinedComponents->UserAirTerminal(CompNum).initPluginLocation);
            } else if (state.dataUserDefinedComponents->UserAirTerminal(CompNum).initCallbackIndex > -1) {
                state.dataPluginManager->pluginManager->runSingleUserDefinedCallback(
                    state, state.dataUserDefinedComponents->UserAirTerminal(CompNum).initCallbackIndex);
            }
            if (state.dataUserDefinedComponents->UserAirTerminal(CompNum).NumPlantConnections > 0) {
                for (int loop = 0; loop < state.dataUserDefinedComponents->UserAirTerminal(CompNum).NumPlantConnections; ++loop) {
                    auto const &plantConnection = state.dataUserDefinedComponents->UserAirTerminal(CompNum).Loop[loop];

                    PlantUtilities::InitComponentNodes(state,
                                                       plantConnection.MassFlowRateMin,
                                                       plantConnection.MassFlowRateMax,
                                                       plantConnection.InletNodeNum,
                                                       plantConnection.OutletNodeNum);

                    PlantUtilities::RegisterPlantCompDesignFlow(state, plantConnection.InletNodeNum, plantConnection.DesignVolumeFlowRate);
                }
            }

        } // BeginEnvrnFlag

        state.dataUserDefinedComponents->UserAirTerminal(CompNum).initialize(state, ZoneNum);

        if (state.dataUserDefinedComponents->UserAirTerminal(CompNum).ErlSimProgramMngr > 0) {
            EMSManager::ManageEMS(state,
                                  EMSManager::EMSCallFrom::UserDefinedComponentModel,
                                  anyEMSRan,
                                  state.dataUserDefinedComponents->UserAirTerminal(CompNum).ErlSimProgramMngr);
        } else if (state.dataUserDefinedComponents->UserAirTerminal(CompNum).simPluginLocation > -1) {
            state.dataPluginManager->pluginManager->runSingleUserDefinedPlugin(
                state, state.dataUserDefinedComponents->UserAirTerminal(CompNum).simPluginLocation);
        } else if (state.dataUserDefinedComponents->UserAirTerminal(CompNum).simCallbackIndex > -1) {
            state.dataPluginManager->pluginManager->runSingleUserDefinedCallback(
                state, state.dataUserDefinedComponents->UserAirTerminal(CompNum).simCallbackIndex);
        }

        state.dataUserDefinedComponents->UserAirTerminal(CompNum).report(state);
    }

    void GetUserDefinedPlantComponents(EnergyPlusData &state)
    {
        bool ErrorsFound(false);
        int NumAlphas; // Number of elements in the alpha array
        int NumNums;   // Number of elements in the numeric array
        int IOStat;    // IO Status when calling get input subroutine
        int TotalArgs; // argument for call to GetObjectDefMaxArgs
        Array1D_string cAlphaFieldNames;
        Array1D_bool lAlphaFieldBlanks;
        Array1D_string cAlphaArgs;
        Array1D<Real64> rNumericArgs;
        std::string cCurrentModuleObject;

        cCurrentModuleObject = "PlantComponent:UserDefined";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);

        cAlphaFieldNames.allocate(NumAlphas);
        cAlphaArgs.allocate(NumAlphas);
        lAlphaFieldBlanks.dimension(NumAlphas, false);
        rNumericArgs.dimension(NumNums, 0.0);

        // need to make sure GetEMSInput has run...

        state.dataUserDefinedComponents->NumUserPlantComps =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        if (state.dataUserDefinedComponents->NumUserPlantComps > 0) {
            state.dataUserDefinedComponents->UserPlantComp.allocate(state.dataUserDefinedComponents->NumUserPlantComps);
            state.dataUserDefinedComponents->CheckUserPlantCompName.assign(state.dataUserDefinedComponents->NumUserPlantComps, true);
            for (int CompLoop = 1; CompLoop <= state.dataUserDefinedComponents->NumUserPlantComps; ++CompLoop) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         CompLoop,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNums,
                                                                         IOStat,
                                                                         _,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         _);

                state.dataUserDefinedComponents->UserPlantComp(CompLoop).Name = cAlphaArgs(1);

                // now get program manager for model simulations
                if (!lAlphaFieldBlanks(2)) {
                    int StackMngrNum = Util::FindItemInList(cAlphaArgs(2), state.dataRuntimeLang->EMSProgramCallManager);
                    if (StackMngrNum > 0) { // found it
                        state.dataUserDefinedComponents->UserPlantComp(CompLoop).ErlSimProgramMngr = StackMngrNum;
                    } else {
                        // check Python Plugins
                        state.dataUserDefinedComponents->UserPlantComp(CompLoop).simPluginLocation =
                            state.dataPluginManager->pluginManager->getLocationOfUserDefinedPlugin(state, cAlphaArgs(2));
                        if (state.dataUserDefinedComponents->UserPlantComp(CompLoop).simPluginLocation == -1) {
                            state.dataUserDefinedComponents->UserPlantComp(CompLoop).simCallbackIndex =
                                state.dataPluginManager->pluginManager->getUserDefinedCallbackIndex(state, cAlphaArgs(2));
                            if (state.dataUserDefinedComponents->UserPlantComp(CompLoop).simCallbackIndex == -1) {
                                ShowSevereError(state, std::format("Invalid {}={}", cAlphaFieldNames(2), cAlphaArgs(2)));
                                ShowContinueError(state, std::format("Entered in {}={}", cCurrentModuleObject, cAlphaArgs(1)));
                                ShowContinueError(
                                    state,
                                    "Program Manager Name not found as an EMS Program Manager, API callback or a Python Plugin Instance object.");
                                ErrorsFound = true;
                            }
                        }
                    }
                }

                int NumPlantConnections = std::floor(rNumericArgs(1));
                auto &userPlantComp = state.dataUserDefinedComponents->UserPlantComp(CompLoop);

                if ((NumPlantConnections >= 1) && (NumPlantConnections <= 4)) {
                    userPlantComp.Loop.resize(NumPlantConnections);
                    userPlantComp.NumPlantConnections = NumPlantConnections;
                    for (int connectionIndex = 0; connectionIndex < NumPlantConnections; ++connectionIndex) {
                        int const connectionNum = connectionIndex + 1;
                        auto &plantConnection = userPlantComp.Loop[connectionIndex];
                        const std::string LoopStr = std::to_string(connectionNum);
                        int aArgCount = connectionIndex * 6 + 3;
                        plantConnection.InletNodeNum = Node::GetOnlySingleNode(state,
                                                                               cAlphaArgs(aArgCount),
                                                                               ErrorsFound,
                                                                               Node::ConnectionObjectType::PlantComponentUserDefined,
                                                                               cAlphaArgs(1),
                                                                               Node::FluidType::Water,
                                                                               Node::ConnectionType::Inlet,
                                                                               static_cast<Node::CompFluidStream>(connectionNum),
                                                                               Node::ObjectIsNotParent);
                        plantConnection.OutletNodeNum = Node::GetOnlySingleNode(state,
                                                                                cAlphaArgs(aArgCount + 1),
                                                                                ErrorsFound,
                                                                                Node::ConnectionObjectType::PlantComponentUserDefined,
                                                                                cAlphaArgs(1),
                                                                                Node::FluidType::Water,
                                                                                Node::ConnectionType::Outlet,
                                                                                static_cast<Node::CompFluidStream>(connectionNum),
                                                                                Node::ObjectIsNotParent);

                        Node::TestCompSet(
                            state, cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(aArgCount), cAlphaArgs(aArgCount + 1), "Plant Nodes " + LoopStr);

                        {
                            plantConnection.HowLoadServed = static_cast<DataPlant::HowMet>(
                                getEnumValue(DataPlant::HowMetTypeNamesUC, Util::makeUPPER(cAlphaArgs(aArgCount + 2))));
                            if (plantConnection.HowLoadServed == DataPlant::HowMet::ByNominalCapLowOutLimit) {
                                // actuator for low out limit
                                SetupEMSActuator(state,
                                                 "Plant Connection " + LoopStr,
                                                 userPlantComp.Name,
                                                 "Low Outlet Temperature Limit",
                                                 "[C]",
                                                 state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                                 plantConnection.LowOutTempLimit);
                            } else if (plantConnection.HowLoadServed == DataPlant::HowMet::ByNominalCapHiOutLimit) {
                                // actuator for hi out limit
                                SetupEMSActuator(state,
                                                 "Plant Connection " + LoopStr,
                                                 userPlantComp.Name,
                                                 "High Outlet Temperature Limit",
                                                 "[C]",
                                                 state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                                 plantConnection.HiOutTempLimit);
                            }
                        }

                        {
                            plantConnection.FlowPriority = static_cast<DataPlant::LoopFlowStatus>(
                                getEnumValue(DataPlant::LoopFlowStatusTypeNamesUC, Util::makeUPPER(cAlphaArgs(aArgCount + 3))));
                        }

                        // find program manager for initial setup, begin environment and sizing of this plant connection
                        if (!lAlphaFieldBlanks(aArgCount + 4)) {
                            int StackMngrNum = Util::FindItemInList(cAlphaArgs(aArgCount + 4), state.dataRuntimeLang->EMSProgramCallManager);
                            if (StackMngrNum > 0) { // found it
                                plantConnection.ErlInitProgramMngr = StackMngrNum;
                            } else {
                                plantConnection.initPluginLocation =
                                    state.dataPluginManager->pluginManager->getLocationOfUserDefinedPlugin(state, cAlphaArgs(aArgCount + 4));
                                if (plantConnection.initPluginLocation == -1) {
                                    plantConnection.initCallbackIndex =
                                        state.dataPluginManager->pluginManager->getUserDefinedCallbackIndex(state, cAlphaArgs(aArgCount + 4));
                                    if (plantConnection.initCallbackIndex == -1) {
                                        ShowSevereError(state,
                                                        std::format("Invalid {}={}", cAlphaFieldNames(aArgCount + 4), cAlphaArgs(aArgCount + 4)));
                                        ShowContinueError(state, std::format("Entered in {}={}", cCurrentModuleObject, cAlphaArgs(1)));
                                        ShowContinueError(state,
                                                          "Program Manager Name not found as an EMS Program Manager, API callback, or a Python "
                                                          "Plugin Instance object.");
                                        ErrorsFound = true;
                                    }
                                }
                            }
                        }

                        // find program to call for model simulations for just this plant connection
                        if (!lAlphaFieldBlanks(aArgCount + 5)) {
                            int StackMngrNum = Util::FindItemInList(cAlphaArgs(aArgCount + 5), state.dataRuntimeLang->EMSProgramCallManager);
                            if (StackMngrNum > 0) { // found it
                                plantConnection.ErlSimProgramMngr = StackMngrNum;
                            } else {
                                plantConnection.simPluginLocation =
                                    state.dataPluginManager->pluginManager->getLocationOfUserDefinedPlugin(state, cAlphaArgs(aArgCount + 5));
                                if (plantConnection.simPluginLocation == -1) {
                                    plantConnection.simCallbackIndex =
                                        state.dataPluginManager->pluginManager->getUserDefinedCallbackIndex(state, cAlphaArgs(aArgCount + 5));
                                    if (plantConnection.simCallbackIndex == -1) {
                                        ShowSevereError(state,
                                                        std::format("Invalid {}={}", cAlphaFieldNames(aArgCount + 4), cAlphaArgs(aArgCount + 4)));
                                        ShowContinueError(state, std::format("Entered in {}={}", cCurrentModuleObject, cAlphaArgs(1)));
                                        ShowContinueError(state, "Program Manager Name not found as EMS Program, API callback, or Python Plugin.");
                                        ErrorsFound = true;
                                    }
                                }
                            }
                        }
                        // Setup Internal Variables
                        // model input related internal variables
                        SetupEMSInternalVariable(
                            state, "Inlet Temperature for Plant Connection " + LoopStr, userPlantComp.Name, "[C]", plantConnection.InletTemp);
                        SetupEMSInternalVariable(state,
                                                 "Inlet Mass Flow Rate for Plant Connection " + LoopStr,
                                                 userPlantComp.Name,
                                                 "[kg/s]",
                                                 plantConnection.InletMassFlowRate);
                        if (plantConnection.HowLoadServed != DataPlant::HowMet::NoneDemand) {
                            SetupEMSInternalVariable(
                                state, "Load Request for Plant Connection " + LoopStr, userPlantComp.Name, "[W]", plantConnection.MyLoad);
                        }
                        SetupEMSInternalVariable(
                            state, "Inlet Density for Plant Connection " + LoopStr, userPlantComp.Name, "[kg/m3]", plantConnection.InletRho);
                        SetupEMSInternalVariable(
                            state, "Inlet Specific Heat for Plant Connection " + LoopStr, userPlantComp.Name, "[J/kg-C]", plantConnection.InletCp);
                        // model results related actuators
                        SetupEMSActuator(state,
                                         "Plant Connection " + LoopStr,
                                         userPlantComp.Name,
                                         "Outlet Temperature",
                                         "[C]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         plantConnection.OutletTemp);
                        SetupEMSActuator(state,
                                         "Plant Connection " + LoopStr,
                                         userPlantComp.Name,
                                         "Mass Flow Rate",
                                         "[kg/s]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         plantConnection.MassFlowRateRequest);
                        // model initialization and sizing related actuators
                        SetupEMSActuator(state,
                                         "Plant Connection " + LoopStr,
                                         userPlantComp.Name,
                                         "Minimum Mass Flow Rate",
                                         "[kg/s]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         plantConnection.MassFlowRateMin);
                        SetupEMSActuator(state,
                                         "Plant Connection " + LoopStr,
                                         userPlantComp.Name,
                                         "Maximum Mass Flow Rate",
                                         "[kg/s]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         plantConnection.MassFlowRateMax);
                        SetupEMSActuator(state,
                                         "Plant Connection " + LoopStr,
                                         userPlantComp.Name,
                                         "Design Volume Flow Rate",
                                         "[m3/s]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         plantConnection.DesignVolumeFlowRate);
                        SetupEMSActuator(state,
                                         "Plant Connection " + LoopStr,
                                         userPlantComp.Name,
                                         "Minimum Loading Capacity",
                                         "[W]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         plantConnection.MinLoad);
                        SetupEMSActuator(state,
                                         "Plant Connection " + LoopStr,
                                         userPlantComp.Name,
                                         "Maximum Loading Capacity",
                                         "[W]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         plantConnection.MaxLoad);
                        SetupEMSActuator(state,
                                         "Plant Connection " + LoopStr,
                                         userPlantComp.Name,
                                         "Optimal Loading Capacity",
                                         "[W]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         plantConnection.OptLoad);
                    }
                }

                if (!lAlphaFieldBlanks(27)) {
                    state.dataUserDefinedComponents->UserPlantComp(CompLoop).Air.InletNodeNum =
                        Node::GetOnlySingleNode(state,
                                                cAlphaArgs(27),
                                                ErrorsFound,
                                                Node::ConnectionObjectType::PlantComponentUserDefined,
                                                state.dataUserDefinedComponents->UserPlantComp(CompLoop).Name,
                                                Node::FluidType::Air,
                                                Node::ConnectionType::OutsideAirReference,
                                                Node::CompFluidStream::Primary,
                                                Node::ObjectIsNotParent);
                    // model input related internal variables
                    SetupEMSInternalVariable(state,
                                             "Inlet Temperature for Air Connection",
                                             state.dataUserDefinedComponents->UserPlantComp(CompLoop).Name,
                                             "[C]",
                                             state.dataUserDefinedComponents->UserPlantComp(CompLoop).Air.InletTemp);
                    SetupEMSInternalVariable(state,
                                             "Inlet Mass Flow Rate for Air Connection",
                                             state.dataUserDefinedComponents->UserPlantComp(CompLoop).Name,
                                             "[kg/s]",
                                             state.dataUserDefinedComponents->UserPlantComp(CompLoop).Air.InletMassFlowRate);
                    SetupEMSInternalVariable(state,
                                             "Inlet Humidity Ratio for Air Connection",
                                             state.dataUserDefinedComponents->UserPlantComp(CompLoop).Name,
                                             "[kgWater/kgDryAir]",
                                             state.dataUserDefinedComponents->UserPlantComp(CompLoop).Air.InletHumRat);
                    SetupEMSInternalVariable(state,
                                             "Inlet Density for Air Connection",
                                             state.dataUserDefinedComponents->UserPlantComp(CompLoop).Name,
                                             "[kg/m3]",
                                             state.dataUserDefinedComponents->UserPlantComp(CompLoop).Air.InletRho);
                    SetupEMSInternalVariable(state,
                                             "Inlet Specific Heat for Air Connection",
                                             state.dataUserDefinedComponents->UserPlantComp(CompLoop).Name,
                                             "[J/kg-C]",
                                             state.dataUserDefinedComponents->UserPlantComp(CompLoop).Air.InletCp);
                }

                if (!lAlphaFieldBlanks(28)) {
                    state.dataUserDefinedComponents->UserPlantComp(CompLoop).Air.OutletNodeNum =
                        Node::GetOnlySingleNode(state,
                                                cAlphaArgs(28),
                                                ErrorsFound,
                                                Node::ConnectionObjectType::PlantComponentUserDefined,
                                                state.dataUserDefinedComponents->UserPlantComp(CompLoop).Name,
                                                Node::FluidType::Air,
                                                Node::ConnectionType::ReliefAir,
                                                Node::CompFluidStream::Primary,
                                                Node::ObjectIsNotParent);
                    // outlet air node results
                    SetupEMSActuator(state,
                                     "Air Connection",
                                     state.dataUserDefinedComponents->UserPlantComp(CompLoop).Name,
                                     "Outlet Temperature",
                                     "[C]",
                                     state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                     state.dataUserDefinedComponents->UserPlantComp(CompLoop).Air.OutletTemp);
                    SetupEMSActuator(state,
                                     "Air Connection",
                                     state.dataUserDefinedComponents->UserPlantComp(CompLoop).Name,
                                     "Outlet Humidity Ratio",
                                     "[kgWater/kgDryAir]",
                                     state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                     state.dataUserDefinedComponents->UserPlantComp(CompLoop).Air.OutletHumRat);
                    SetupEMSActuator(state,
                                     "Air Connection",
                                     state.dataUserDefinedComponents->UserPlantComp(CompLoop).Name,
                                     "Mass Flow Rate",
                                     "[kg/s]",
                                     state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                     state.dataUserDefinedComponents->UserPlantComp(CompLoop).Air.OutletMassFlowRate);
                }

                if (!lAlphaFieldBlanks(29)) {
                    WaterManager::SetupTankDemandComponent(state,
                                                           cAlphaArgs(1),
                                                           cCurrentModuleObject,
                                                           cAlphaArgs(29),
                                                           ErrorsFound,
                                                           state.dataUserDefinedComponents->UserPlantComp(CompLoop).Water.SupplyTankID,
                                                           state.dataUserDefinedComponents->UserPlantComp(CompLoop).Water.SupplyTankDemandARRID);

                    state.dataUserDefinedComponents->UserPlantComp(CompLoop).Water.SuppliedByWaterSystem = true;
                    SetupEMSActuator(state,
                                     "Water System",
                                     state.dataUserDefinedComponents->UserPlantComp(CompLoop).Name,
                                     "Supplied Volume Flow Rate",
                                     "[m3/s]",
                                     state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                     state.dataUserDefinedComponents->UserPlantComp(CompLoop).Water.SupplyVdotRequest);
                }

                if (!lAlphaFieldBlanks(30)) {
                    WaterManager::SetupTankSupplyComponent(state,
                                                           cAlphaArgs(1),
                                                           cCurrentModuleObject,
                                                           cAlphaArgs(30),
                                                           ErrorsFound,
                                                           state.dataUserDefinedComponents->UserPlantComp(CompLoop).Water.CollectionTankID,
                                                           state.dataUserDefinedComponents->UserPlantComp(CompLoop).Water.CollectionTankSupplyARRID);
                    state.dataUserDefinedComponents->UserPlantComp(CompLoop).Water.CollectsToWaterSystem = true;
                    SetupEMSActuator(state,
                                     "Water System",
                                     state.dataUserDefinedComponents->UserPlantComp(CompLoop).Name,
                                     "Collected Volume Flow Rate",
                                     "[m3/s]",
                                     state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                     state.dataUserDefinedComponents->UserPlantComp(CompLoop).Water.CollectedVdot);
                }

                if (!lAlphaFieldBlanks(31)) {

                    state.dataUserDefinedComponents->UserPlantComp(CompLoop).Zone.ZoneNum =
                        Util::FindItemInList(cAlphaArgs(31), state.dataHeatBal->Zone);
                    if (state.dataUserDefinedComponents->UserPlantComp(CompLoop).Zone.ZoneNum == 0) {
                        ShowSevereError(
                            state, std::format("{} = {}:  Ambient Zone Name not found = {}", cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(31)));
                        ErrorsFound = true;
                    } else {
                        state.dataUserDefinedComponents->UserPlantComp(CompLoop).Zone.DeviceHasInternalGains = true;
                        SetupZoneInternalGain(state,
                                              state.dataUserDefinedComponents->UserPlantComp(CompLoop).Zone.ZoneNum,
                                              cAlphaArgs(1),
                                              DataHeatBalance::IntGainType::PlantComponentUserDefined,
                                              &state.dataUserDefinedComponents->UserPlantComp(CompLoop).Zone.ConvectionGainRate,
                                              &state.dataUserDefinedComponents->UserPlantComp(CompLoop).Zone.ReturnAirConvectionGainRate,
                                              &state.dataUserDefinedComponents->UserPlantComp(CompLoop).Zone.ThermalRadiationGainRate,
                                              &state.dataUserDefinedComponents->UserPlantComp(CompLoop).Zone.LatentGainRate,
                                              &state.dataUserDefinedComponents->UserPlantComp(CompLoop).Zone.ReturnAirLatentGainRate,
                                              &state.dataUserDefinedComponents->UserPlantComp(CompLoop).Zone.CarbonDioxideGainRate,
                                              &state.dataUserDefinedComponents->UserPlantComp(CompLoop).Zone.GenericContamGainRate);

                        SetupEMSActuator(state,
                                         "Component Zone Internal Gain",
                                         state.dataUserDefinedComponents->UserPlantComp(CompLoop).Name,
                                         "Sensible Heat Gain Rate",
                                         "[W]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         state.dataUserDefinedComponents->UserPlantComp(CompLoop).Zone.ConvectionGainRate);
                        SetupEMSActuator(state,
                                         "Component Zone Internal Gain",
                                         state.dataUserDefinedComponents->UserPlantComp(CompLoop).Name,
                                         "Return Air Heat Sensible Gain Rate",
                                         "[W]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         state.dataUserDefinedComponents->UserPlantComp(CompLoop).Zone.ReturnAirConvectionGainRate);
                        SetupEMSActuator(state,
                                         "Component Zone Internal Gain",
                                         state.dataUserDefinedComponents->UserPlantComp(CompLoop).Name,
                                         "Thermal Radiation Heat Gain Rate",
                                         "[W]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         state.dataUserDefinedComponents->UserPlantComp(CompLoop).Zone.ThermalRadiationGainRate);
                        SetupEMSActuator(state,
                                         "Component Zone Internal Gain",
                                         state.dataUserDefinedComponents->UserPlantComp(CompLoop).Name,
                                         "Latent Heat Gain Rate",
                                         "[W]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         state.dataUserDefinedComponents->UserPlantComp(CompLoop).Zone.LatentGainRate);
                        SetupEMSActuator(state,
                                         "Component Zone Internal Gain",
                                         state.dataUserDefinedComponents->UserPlantComp(CompLoop).Name,
                                         "Return Air Latent Heat Gain Rate",
                                         "[W]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         state.dataUserDefinedComponents->UserPlantComp(CompLoop).Zone.ReturnAirLatentGainRate);
                        SetupEMSActuator(state,
                                         "Component Zone Internal Gain",
                                         state.dataUserDefinedComponents->UserPlantComp(CompLoop).Name,
                                         "Carbon Dioxide Gain Rate",
                                         "[W]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         state.dataUserDefinedComponents->UserPlantComp(CompLoop).Zone.CarbonDioxideGainRate);
                        SetupEMSActuator(state,
                                         "Component Zone Internal Gain",
                                         state.dataUserDefinedComponents->UserPlantComp(CompLoop).Name,
                                         "Gaseous Contaminant Gain Rate",
                                         "[W]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         state.dataUserDefinedComponents->UserPlantComp(CompLoop).Zone.GenericContamGainRate);
                    }
                }

                // make sure user has entered at least some erl program managers to actually calculate something
                int MgrCountTest = 0;
                if (userPlantComp.ErlSimProgramMngr > 0) {
                    MgrCountTest = 1;
                }
                for (auto const &plantConnection : userPlantComp.Loop) {
                    if (plantConnection.ErlInitProgramMngr > 0) {
                        ++MgrCountTest;
                    }
                    if (plantConnection.ErlSimProgramMngr > 0) {
                        ++MgrCountTest;
                    }
                    if (plantConnection.initPluginLocation >= 0) {
                        ++MgrCountTest;
                    }
                    if (plantConnection.simPluginLocation >= 0) {
                        ++MgrCountTest;
                    }
                    if (plantConnection.initCallbackIndex >= 0) {
                        ++MgrCountTest;
                    }
                    if (plantConnection.simCallbackIndex >= 0) {
                        ++MgrCountTest;
                    }
                }
                if (MgrCountTest == 0) {
                    ShowSevereError(state, std::format("Invalid {}={}", cCurrentModuleObject, cAlphaArgs(1)));
                    ShowContinueError(state, "At least one program calling manager is needed.");
                    ErrorsFound = true;
                }
            }
        } // NumUserPlantComps > 0

        if (ErrorsFound) {
            ShowFatalError(state, std::format("GetUserDefinedComponents: Errors found in processing {} input.", cCurrentModuleObject));
        }

        cCurrentModuleObject = "Coil:UserDefined";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);

        cAlphaFieldNames.allocate(NumAlphas);
        cAlphaArgs.allocate(NumAlphas);
        lAlphaFieldBlanks.dimension(NumAlphas, false);
        rNumericArgs.dimension(NumNums, 0.0);

        state.dataUserDefinedComponents->NumUserCoils = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);

        if (state.dataUserDefinedComponents->NumUserCoils > 0) {
            state.dataUserDefinedComponents->UserCoil.allocate(state.dataUserDefinedComponents->NumUserCoils);
            state.dataUserDefinedComponents->CheckUserCoilName.assign(state.dataUserDefinedComponents->NumUserCoils, true);
            for (int CompLoop = 1; CompLoop <= state.dataUserDefinedComponents->NumUserCoils; ++CompLoop) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         CompLoop,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNums,
                                                                         IOStat,
                                                                         _,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         _);

                // ErrorsFound will be set to True if problem was found, left untouched otherwise
                GlobalNames::VerifyUniqueCoilName(state, cCurrentModuleObject, cAlphaArgs(1), ErrorsFound, cCurrentModuleObject + " Name");

                state.dataUserDefinedComponents->UserCoil(CompLoop).Name = cAlphaArgs(1);

                // now get program manager for model simulations
                if (!lAlphaFieldBlanks(2)) {
                    int StackMngrNum = Util::FindItemInList(cAlphaArgs(2), state.dataRuntimeLang->EMSProgramCallManager);
                    if (StackMngrNum > 0) { // found it
                        state.dataUserDefinedComponents->UserCoil(CompLoop).ErlSimProgramMngr = StackMngrNum;
                    } else {
                        state.dataUserDefinedComponents->UserCoil(CompLoop).simPluginLocation =
                            state.dataPluginManager->pluginManager->getLocationOfUserDefinedPlugin(state, cAlphaArgs(2));
                        if (state.dataUserDefinedComponents->UserCoil(CompLoop).simPluginLocation == -1) {
                            state.dataUserDefinedComponents->UserCoil(CompLoop).simCallbackIndex =
                                state.dataPluginManager->pluginManager->getUserDefinedCallbackIndex(state, cAlphaArgs(2));
                            if (state.dataUserDefinedComponents->UserCoil(CompLoop).simCallbackIndex == -1) {
                                ShowSevereError(state, std::format("Invalid {}={}", cAlphaFieldNames(2), cAlphaArgs(2)));
                                ShowContinueError(state, std::format("Entered in {}={}", cCurrentModuleObject, cAlphaArgs(1)));
                                ShowContinueError(
                                    state,
                                    "Program Manager Name not found as an EMS Program Manager, API callback, or a Python Plugin Instance object.");
                                ErrorsFound = true;
                            }
                        }
                    }
                }

                // now get program manager for model initializations
                if (!lAlphaFieldBlanks(3)) {
                    int StackMngrNum = Util::FindItemInList(cAlphaArgs(3), state.dataRuntimeLang->EMSProgramCallManager);
                    if (StackMngrNum > 0) { // found it
                        state.dataUserDefinedComponents->UserCoil(CompLoop).ErlInitProgramMngr = StackMngrNum;
                    } else {
                        state.dataUserDefinedComponents->UserCoil(CompLoop).initPluginLocation =
                            state.dataPluginManager->pluginManager->getLocationOfUserDefinedPlugin(state, cAlphaArgs(3));
                        if (state.dataUserDefinedComponents->UserCoil(CompLoop).initPluginLocation == -1) {
                            state.dataUserDefinedComponents->UserCoil(CompLoop).initCallbackIndex =
                                state.dataPluginManager->pluginManager->getUserDefinedCallbackIndex(state, cAlphaArgs(3));
                            if (state.dataUserDefinedComponents->UserCoil(CompLoop).initCallbackIndex == -1) {
                                ShowSevereError(state, std::format("Invalid {}={}", cAlphaFieldNames(3), cAlphaArgs(3)));
                                ShowContinueError(state, std::format("Entered in {}={}", cCurrentModuleObject, cAlphaArgs(1)));
                                ShowContinueError(
                                    state,
                                    "Program Manager Name not found as an EMS Program Manager, API callback or a Python Plugin Instance object.");
                                ErrorsFound = true;
                            }
                        }
                    }
                }

                int NumAirConnections = std::floor(rNumericArgs(1));
                auto &userCoil = state.dataUserDefinedComponents->UserCoil(CompLoop);
                if ((NumAirConnections >= 1) && (NumAirConnections <= 2)) {
                    userCoil.AirConnections.resize(NumAirConnections);
                    userCoil.NumAirConnections = NumAirConnections;
                    for (int connectionIndex = 0; connectionIndex < NumAirConnections; ++connectionIndex) {
                        int const connectionNum = connectionIndex + 1;
                        int aArgCount = connectionIndex * 2 + 4;
                        auto &airConnection = userCoil.AirConnections[connectionIndex];
                        airConnection.InletNodeNum = Node::GetOnlySingleNode(state,
                                                                             cAlphaArgs(aArgCount),
                                                                             ErrorsFound,
                                                                             Node::ConnectionObjectType::CoilUserDefined,
                                                                             userCoil.Name,
                                                                             Node::FluidType::Air,
                                                                             Node::ConnectionType::Inlet,
                                                                             static_cast<Node::CompFluidStream>(connectionNum),
                                                                             Node::ObjectIsNotParent);

                        const std::string LoopStr = std::to_string(connectionNum);
                        // model input related internal variables
                        SetupEMSInternalVariable(
                            state, "Inlet Temperature for Air Connection " + LoopStr, userCoil.Name, "[C]", airConnection.InletTemp);
                        SetupEMSInternalVariable(
                            state, "Inlet Mass Flow Rate for Air Connection " + LoopStr, userCoil.Name, "[kg/s]", airConnection.InletMassFlowRate);
                        SetupEMSInternalVariable(state,
                                                 "Inlet Humidity Ratio for Air Connection " + LoopStr,
                                                 userCoil.Name,
                                                 "[kgWater/kgDryAir]",
                                                 airConnection.InletHumRat);
                        SetupEMSInternalVariable(
                            state, "Inlet Density for Air Connection " + LoopStr, userCoil.Name, "[kg/m3]", airConnection.InletRho);
                        SetupEMSInternalVariable(
                            state, "Inlet Specific Heat for Air Connection " + LoopStr, userCoil.Name, "[J/kg-C]", airConnection.InletCp);

                        airConnection.OutletNodeNum = Node::GetOnlySingleNode(state,
                                                                              cAlphaArgs(aArgCount + 1),
                                                                              ErrorsFound,
                                                                              Node::ConnectionObjectType::CoilUserDefined,
                                                                              userCoil.Name,
                                                                              Node::FluidType::Air,
                                                                              Node::ConnectionType::Outlet,
                                                                              static_cast<Node::CompFluidStream>(connectionNum),
                                                                              Node::ObjectIsNotParent);
                        SetupEMSActuator(state,
                                         "Air Connection " + LoopStr,
                                         userCoil.Name,
                                         "Outlet Temperature",
                                         "[C]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         airConnection.OutletTemp);
                        SetupEMSActuator(state,
                                         "Air Connection " + LoopStr,
                                         userCoil.Name,
                                         "Outlet Humidity Ratio",
                                         "[kgWater/kgDryAir]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         airConnection.OutletHumRat);
                        SetupEMSActuator(state,
                                         "Air Connection " + LoopStr,
                                         userCoil.Name,
                                         "Mass Flow Rate",
                                         "[kg/s]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         airConnection.OutletMassFlowRate);

                        Node::TestCompSet(
                            state, cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(aArgCount), cAlphaArgs(aArgCount + 1), "Air Nodes " + LoopStr);
                    }

                    if (!lAlphaFieldBlanks(8)) {
                        if (cAlphaArgs(8) == "YES") {
                            state.dataUserDefinedComponents->UserCoil(CompLoop).PlantIsConnected = true;
                        } else if (cAlphaArgs(8) == "NO") {
                            state.dataUserDefinedComponents->UserCoil(CompLoop).PlantIsConnected = false;
                        }
                    } else {
                        state.dataUserDefinedComponents->UserCoil(CompLoop).PlantIsConnected = false;
                    }

                    if (state.dataUserDefinedComponents->UserCoil(CompLoop).PlantIsConnected) { // get input
                        state.dataUserDefinedComponents->UserCoil(CompLoop).Loop.InletNodeNum =
                            Node::GetOnlySingleNode(state,
                                                    cAlphaArgs(9),
                                                    ErrorsFound,
                                                    Node::ConnectionObjectType::CoilUserDefined,
                                                    cAlphaArgs(1),
                                                    Node::FluidType::Water,
                                                    Node::ConnectionType::Inlet,
                                                    Node::CompFluidStream::Tertiary,
                                                    Node::ObjectIsNotParent);
                        state.dataUserDefinedComponents->UserCoil(CompLoop).Loop.OutletNodeNum =
                            Node::GetOnlySingleNode(state,
                                                    cAlphaArgs(10),
                                                    ErrorsFound,
                                                    Node::ConnectionObjectType::CoilUserDefined,
                                                    cAlphaArgs(1),
                                                    Node::FluidType::Water,
                                                    Node::ConnectionType::Outlet,
                                                    Node::CompFluidStream::Tertiary,
                                                    Node::ObjectIsNotParent);

                        Node::TestCompSet(state, cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(9), cAlphaArgs(10), "Plant Nodes");

                        // this model is only for plant connections that are "Demand"
                        state.dataUserDefinedComponents->UserCoil(CompLoop).Loop.HowLoadServed = DataPlant::HowMet::NoneDemand;
                        // this model is only for plant connections that are needy and turn loop on
                        state.dataUserDefinedComponents->UserCoil(CompLoop).Loop.FlowPriority = DataPlant::LoopFlowStatus::NeedyAndTurnsLoopOn;

                        // Setup Internal Variables
                        // model input related internal variables
                        SetupEMSInternalVariable(state,
                                                 "Inlet Temperature for Plant Connection",
                                                 state.dataUserDefinedComponents->UserCoil(CompLoop).Name,
                                                 "[C]",
                                                 state.dataUserDefinedComponents->UserCoil(CompLoop).Loop.InletTemp);
                        SetupEMSInternalVariable(state,
                                                 "Inlet Mass Flow Rate for Plant Connection",
                                                 state.dataUserDefinedComponents->UserCoil(CompLoop).Name,
                                                 "[kg/s]",
                                                 state.dataUserDefinedComponents->UserCoil(CompLoop).Loop.InletMassFlowRate);
                        SetupEMSInternalVariable(state,
                                                 "Inlet Density for Plant Connection",
                                                 state.dataUserDefinedComponents->UserCoil(CompLoop).Name,
                                                 "[kg/m3]",
                                                 state.dataUserDefinedComponents->UserCoil(CompLoop).Loop.InletRho);
                        SetupEMSInternalVariable(state,
                                                 "Inlet Specific Heat for Plant Connection",
                                                 state.dataUserDefinedComponents->UserCoil(CompLoop).Name,
                                                 "[J/kg-C]",
                                                 state.dataUserDefinedComponents->UserCoil(CompLoop).Loop.InletCp);
                        // model results related actuators
                        SetupEMSActuator(state,
                                         "Plant Connection",
                                         state.dataUserDefinedComponents->UserCoil(CompLoop).Name,
                                         "Outlet Temperature",
                                         "[C]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         state.dataUserDefinedComponents->UserCoil(CompLoop).Loop.OutletTemp);
                        SetupEMSActuator(state,
                                         "Plant Connection",
                                         state.dataUserDefinedComponents->UserCoil(CompLoop).Name,
                                         "Mass Flow Rate",
                                         "[kg/s]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         state.dataUserDefinedComponents->UserCoil(CompLoop).Loop.MassFlowRateRequest);
                        // model initialization and sizing related actuators
                        SetupEMSActuator(state,
                                         "Plant Connection",
                                         state.dataUserDefinedComponents->UserCoil(CompLoop).Name,
                                         "Design Volume Flow Rate",
                                         "[m3/s]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         state.dataUserDefinedComponents->UserCoil(CompLoop).Loop.DesignVolumeFlowRate);

                        SetupEMSActuator(state,
                                         "Plant Connection",
                                         state.dataUserDefinedComponents->UserCoil(CompLoop).Name,
                                         "Minimum Mass Flow Rate",
                                         "[kg/s]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         state.dataUserDefinedComponents->UserCoil(CompLoop).Loop.MassFlowRateMin);
                        SetupEMSActuator(state,
                                         "Plant Connection",
                                         state.dataUserDefinedComponents->UserCoil(CompLoop).Name,
                                         "Maximum Mass Flow Rate",
                                         "[kg/s]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         state.dataUserDefinedComponents->UserCoil(CompLoop).Loop.MassFlowRateMax);
                    }

                    if (!lAlphaFieldBlanks(11)) {
                        WaterManager::SetupTankDemandComponent(state,
                                                               cAlphaArgs(1),
                                                               cCurrentModuleObject,
                                                               cAlphaArgs(11),
                                                               ErrorsFound,
                                                               state.dataUserDefinedComponents->UserCoil(CompLoop).Water.SupplyTankID,
                                                               state.dataUserDefinedComponents->UserCoil(CompLoop).Water.SupplyTankDemandARRID);

                        state.dataUserDefinedComponents->UserCoil(CompLoop).Water.SuppliedByWaterSystem = true;
                        SetupEMSActuator(state,
                                         "Water System",
                                         state.dataUserDefinedComponents->UserCoil(CompLoop).Name,
                                         "Supplied Volume Flow Rate",
                                         "[m3/s]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         state.dataUserDefinedComponents->UserCoil(CompLoop).Water.SupplyVdotRequest);
                    }

                    if (!lAlphaFieldBlanks(12)) {
                        WaterManager::SetupTankSupplyComponent(state,
                                                               cAlphaArgs(1),
                                                               cCurrentModuleObject,
                                                               cAlphaArgs(12),
                                                               ErrorsFound,
                                                               state.dataUserDefinedComponents->UserCoil(CompLoop).Water.CollectionTankID,
                                                               state.dataUserDefinedComponents->UserCoil(CompLoop).Water.CollectionTankSupplyARRID);
                        state.dataUserDefinedComponents->UserCoil(CompLoop).Water.CollectsToWaterSystem = true;
                        SetupEMSActuator(state,
                                         "Water System",
                                         state.dataUserDefinedComponents->UserCoil(CompLoop).Name,
                                         "Collected Volume Flow Rate",
                                         "[m3/s]",
                                         state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                         state.dataUserDefinedComponents->UserCoil(CompLoop).Water.CollectedVdot);
                    }

                    if (!lAlphaFieldBlanks(13)) {

                        state.dataUserDefinedComponents->UserCoil(CompLoop).Zone.ZoneNum =
                            Util::FindItemInList(cAlphaArgs(13), state.dataHeatBal->Zone);
                        if (state.dataUserDefinedComponents->UserCoil(CompLoop).Zone.ZoneNum == 0) {
                            ShowSevereError(
                                state,
                                std::format("{} = {}:  Ambient Zone Name not found = {}", cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(13)));
                            ErrorsFound = true;
                        } else {
                            state.dataUserDefinedComponents->UserCoil(CompLoop).Zone.DeviceHasInternalGains = true;
                            SetupZoneInternalGain(state,
                                                  state.dataUserDefinedComponents->UserCoil(CompLoop).Zone.ZoneNum,
                                                  cAlphaArgs(1),
                                                  DataHeatBalance::IntGainType::CoilUserDefined,
                                                  &state.dataUserDefinedComponents->UserCoil(CompLoop).Zone.ConvectionGainRate,
                                                  &state.dataUserDefinedComponents->UserCoil(CompLoop).Zone.ReturnAirConvectionGainRate,
                                                  &state.dataUserDefinedComponents->UserCoil(CompLoop).Zone.ThermalRadiationGainRate,
                                                  &state.dataUserDefinedComponents->UserCoil(CompLoop).Zone.LatentGainRate,
                                                  &state.dataUserDefinedComponents->UserCoil(CompLoop).Zone.ReturnAirLatentGainRate,
                                                  &state.dataUserDefinedComponents->UserCoil(CompLoop).Zone.CarbonDioxideGainRate,
                                                  &state.dataUserDefinedComponents->UserCoil(CompLoop).Zone.GenericContamGainRate);

                            SetupEMSActuator(state,
                                             "Component Zone Internal Gain",
                                             state.dataUserDefinedComponents->UserCoil(CompLoop).Name,
                                             "Sensible Heat Gain Rate",
                                             "[W]",
                                             state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                             state.dataUserDefinedComponents->UserCoil(CompLoop).Zone.ConvectionGainRate);
                            SetupEMSActuator(state,
                                             "Component Zone Internal Gain",
                                             state.dataUserDefinedComponents->UserCoil(CompLoop).Name,
                                             "Return Air Heat Sensible Gain Rate",
                                             "[W]",
                                             state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                             state.dataUserDefinedComponents->UserCoil(CompLoop).Zone.ReturnAirConvectionGainRate);
                            SetupEMSActuator(state,
                                             "Component Zone Internal Gain",
                                             state.dataUserDefinedComponents->UserCoil(CompLoop).Name,
                                             "Thermal Radiation Heat Gain Rate",
                                             "[W]",
                                             state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                             state.dataUserDefinedComponents->UserCoil(CompLoop).Zone.ThermalRadiationGainRate);
                            SetupEMSActuator(state,
                                             "Component Zone Internal Gain",
                                             state.dataUserDefinedComponents->UserCoil(CompLoop).Name,
                                             "Latent Heat Gain Rate",
                                             "[W]",
                                             state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                             state.dataUserDefinedComponents->UserCoil(CompLoop).Zone.LatentGainRate);
                            SetupEMSActuator(state,
                                             "Component Zone Internal Gain",
                                             state.dataUserDefinedComponents->UserCoil(CompLoop).Name,
                                             "Return Air Latent Heat Gain Rate",
                                             "[W]",
                                             state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                             state.dataUserDefinedComponents->UserCoil(CompLoop).Zone.ReturnAirLatentGainRate);
                            SetupEMSActuator(state,
                                             "Component Zone Internal Gain",
                                             state.dataUserDefinedComponents->UserCoil(CompLoop).Name,
                                             "Carbon Dioxide Gain Rate",
                                             "[W]",
                                             state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                             state.dataUserDefinedComponents->UserCoil(CompLoop).Zone.CarbonDioxideGainRate);
                            SetupEMSActuator(state,
                                             "Component Zone Internal Gain",
                                             state.dataUserDefinedComponents->UserCoil(CompLoop).Name,
                                             "Gaseous Contaminant Gain Rate",
                                             "[W]",
                                             state.dataUserDefinedComponents->lDummy_EMSActuatedPlantComp,
                                             state.dataUserDefinedComponents->UserCoil(CompLoop).Zone.GenericContamGainRate);
                        }
                    }
                }
            }

        } // NumUserCoils > 0

        if (ErrorsFound) {
            ShowFatalError(state, std::format("GetUserDefinedComponents: Errors found in processing {} input.", cCurrentModuleObject));
        }
    }

    void GetUserDefinedComponents(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Jan 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        bool ErrorsFound(false);
        int NumAlphas; // Number of elements in the alpha array
        int NumNums;   // Number of elements in the numeric array
        int TotalArgs; // argument for call to GetObjectDefMaxArgs
        Array1D_string cAlphaFieldNames;
        Array1D_bool lAlphaFieldBlanks;
        Array1D_string cAlphaArgs;
        Array1D<Real64> rNumericArgs;
        std::string cCurrentModuleObject;

        if (state.dataUserDefinedComponents->GetPlantCompInput) {
            GetUserDefinedPlantComponents(state);
            state.dataUserDefinedComponents->GetPlantCompInput = false;
        }

        cCurrentModuleObject = "ZoneHVAC:ForcedAir:UserDefined";
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);

        cAlphaFieldNames.allocate(NumAlphas);
        cAlphaArgs.allocate(NumAlphas);
        lAlphaFieldBlanks.dimension(NumAlphas, false);
        rNumericArgs.dimension(NumNums, 0.0);

        state.dataUserDefinedComponents->NumUserZoneAir = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        if (state.dataUserDefinedComponents->NumUserZoneAir > 0) {
            state.dataUserDefinedComponents->UserZoneAirHVAC.allocate(state.dataUserDefinedComponents->NumUserZoneAir);
            state.dataUserDefinedComponents->CheckUserZoneAirName.assign(state.dataUserDefinedComponents->NumUserZoneAir, true);
            int IOStat; // IO Status when calling get input subroutine
            for (int CompLoop = 1; CompLoop <= state.dataUserDefinedComponents->NumUserZoneAir; ++CompLoop) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         CompLoop,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNums,
                                                                         IOStat,
                                                                         _,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         _);

                state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name = cAlphaArgs(1);

                // now get program manager for model simulations
                if (!lAlphaFieldBlanks(2)) {
                    int StackMngrNum = Util::FindItemInList(cAlphaArgs(2), state.dataRuntimeLang->EMSProgramCallManager);
                    if (StackMngrNum > 0) { // found it
                        state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).ErlSimProgramMngr = StackMngrNum;
                    } else {
                        state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).simPluginLocation =
                            state.dataPluginManager->pluginManager->getLocationOfUserDefinedPlugin(state, cAlphaArgs(2));
                        if (state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).simPluginLocation == -1) {
                            state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).simCallbackIndex =
                                state.dataPluginManager->pluginManager->getUserDefinedCallbackIndex(state, cAlphaArgs(2));
                            if (state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).simCallbackIndex == -1) {
                                ShowSevereError(state, std::format("Invalid {}={}", cAlphaFieldNames(2), cAlphaArgs(2)));
                                ShowContinueError(state, std::format("Entered in {}={}", cCurrentModuleObject, cAlphaArgs(1)));
                                ShowContinueError(
                                    state,
                                    "Program Manager Name not found as an EMS Program Manager, API callback or a Python Plugin Instance object.");
                                ErrorsFound = true;
                            }
                        }
                    }
                }

                // now get program manager for model initializations
                if (!lAlphaFieldBlanks(3)) {
                    int StackMngrNum = Util::FindItemInList(cAlphaArgs(3), state.dataRuntimeLang->EMSProgramCallManager);
                    if (StackMngrNum > 0) { // found it
                        state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).ErlInitProgramMngr = StackMngrNum;
                    } else {
                        state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).initPluginLocation =
                            state.dataPluginManager->pluginManager->getLocationOfUserDefinedPlugin(state, cAlphaArgs(3));
                        if (state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).initPluginLocation == -1) {
                            state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).initCallbackIndex =
                                state.dataPluginManager->pluginManager->getUserDefinedCallbackIndex(state, cAlphaArgs(3));
                            if (state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).initCallbackIndex == -1) {
                                ShowSevereError(state, std::format("Invalid {}={}", cAlphaFieldNames(3), cAlphaArgs(3)));
                                ShowContinueError(state, std::format("Entered in {}={}", cCurrentModuleObject, cAlphaArgs(1)));
                                ShowContinueError(
                                    state,
                                    "Program Manager Name not found as an EMS Program Manager, API callback or a Python Plugin Instance object.");
                                ErrorsFound = true;
                            }
                        }
                    }
                }

                state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).AirConnection.InletNodeNum =
                    Node::GetOnlySingleNode(state,
                                            cAlphaArgs(4),
                                            ErrorsFound,
                                            Node::ConnectionObjectType::ZoneHVACForcedAirUserDefined,
                                            state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                            Node::FluidType::Air,
                                            Node::ConnectionType::Inlet,
                                            Node::CompFluidStream::Primary,
                                            Node::ObjectIsNotParent);
                // model input related internal variables
                SetupEMSInternalVariable(state,
                                         "Inlet Temperature for Primary Air Connection",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                         "[C]",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).AirConnection.InletTemp);
                SetupEMSInternalVariable(state,
                                         "Inlet Humidity Ratio for Primary Air Connection",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                         "[kgWater/kgDryAir]",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).AirConnection.InletHumRat);
                SetupEMSInternalVariable(state,
                                         "Inlet Density for Primary Air Connection",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                         "[kg/m3]",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).AirConnection.InletRho);
                SetupEMSInternalVariable(state,
                                         "Inlet Specific Heat for Primary Air Connection",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                         "[J/kg-C]",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).AirConnection.InletCp);

                SetupEMSInternalVariable(state,
                                         "Remaining Sensible Load to Heating Setpoint",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                         "[W]",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).RemainingOutputToHeatingSP);
                SetupEMSInternalVariable(state,
                                         "Remaining Sensible Load to Cooling Setpoint",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                         "[W]",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).RemainingOutputToCoolingSP);
                SetupEMSInternalVariable(state,
                                         "Remaining Latent Load to Humidifying Setpoint",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                         "[kg/s]",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).RemainingOutputReqToHumidSP);
                SetupEMSInternalVariable(state,
                                         "Remaining Latent Load to Dehumidifying Setpoint",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                         "[kg/s]",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).RemainingOutputReqToDehumidSP);

                SetupEMSActuator(state,
                                 "Primary Air Connection",
                                 state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                 "Inlet Mass Flow Rate",
                                 "[kg/s]",
                                 state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                 state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).AirConnection.InletMassFlowRate);
                state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).AirConnection.OutletNodeNum =
                    Node::GetOnlySingleNode(state,
                                            cAlphaArgs(5),
                                            ErrorsFound,
                                            Node::ConnectionObjectType::ZoneHVACForcedAirUserDefined,
                                            state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                            Node::FluidType::Air,
                                            Node::ConnectionType::Outlet,
                                            Node::CompFluidStream::Primary,
                                            Node::ObjectIsNotParent);
                SetupEMSActuator(state,
                                 "Primary Air Connection",
                                 state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                 "Outlet Temperature",
                                 "[C]",
                                 state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                 state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).AirConnection.OutletTemp);
                SetupEMSActuator(state,
                                 "Primary Air Connection",
                                 state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                 "Outlet Humidity Ratio",
                                 "[kgWater/kgDryAir]",
                                 state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                 state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).AirConnection.OutletHumRat);
                SetupEMSActuator(state,
                                 "Primary Air Connection",
                                 state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                 "Outlet Mass Flow Rate",
                                 "[kg/s]",
                                 state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                 state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).AirConnection.OutletMassFlowRate);

                if (!lAlphaFieldBlanks(6)) {
                    state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).SourceAir.InletNodeNum =
                        Node::GetOnlySingleNode(state,
                                                cAlphaArgs(6),
                                                ErrorsFound,
                                                Node::ConnectionObjectType::ZoneHVACForcedAirUserDefined,
                                                state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                                Node::FluidType::Air,
                                                Node::ConnectionType::Inlet,
                                                Node::CompFluidStream::Secondary,
                                                Node::ObjectIsNotParent);
                    // model input related internal variables
                    SetupEMSInternalVariable(state,
                                             "Inlet Temperature for Secondary Air Connection",
                                             state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                             "[C]",
                                             state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).SourceAir.InletTemp);

                    SetupEMSInternalVariable(state,
                                             "Inlet Humidity Ratio for Secondary Air Connection",
                                             state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                             "[kgWater/kgDryAir]",
                                             state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).SourceAir.InletHumRat);
                    SetupEMSInternalVariable(state,
                                             "Inlet Density for Secondary Air Connection",
                                             state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                             "[kg/m3]",
                                             state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).SourceAir.InletRho);
                    SetupEMSInternalVariable(state,
                                             "Inlet Specific Heat for Secondary Air Connection",
                                             state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                             "[J/kg-C]",
                                             state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).SourceAir.InletCp);
                    SetupEMSActuator(state,
                                     "Secondary Air Connection",
                                     state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                     "Inlet Mass Flow Rate",
                                     "[kg/s]",
                                     state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                     state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).SourceAir.InletMassFlowRate);
                }

                if (!lAlphaFieldBlanks(7)) {
                    state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).SourceAir.OutletNodeNum =
                        Node::GetOnlySingleNode(state,
                                                cAlphaArgs(7),
                                                ErrorsFound,
                                                Node::ConnectionObjectType::ZoneHVACForcedAirUserDefined,
                                                state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                                Node::FluidType::Air,
                                                Node::ConnectionType::Outlet,
                                                Node::CompFluidStream::Secondary,
                                                Node::ObjectIsNotParent);
                    SetupEMSActuator(state,
                                     "Secondary Air Connection",
                                     state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                     "Outlet Temperature",
                                     "[C]",
                                     state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                     state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).SourceAir.OutletTemp);
                    SetupEMSActuator(state,
                                     "Secondary Air Connection",
                                     state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                     "Outlet Humidity Ratio",
                                     "[kgWater/kgDryAir]",
                                     state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                     state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).SourceAir.OutletHumRat);
                    SetupEMSActuator(state,
                                     "Secondary Air Connection",
                                     state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                     "Mass Flow Rate",
                                     "[kg/s]",
                                     state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                     state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).SourceAir.OutletMassFlowRate);
                }

                if ((state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).SourceAir.InletNodeNum > 0) &&
                    (state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).SourceAir.OutletNodeNum > 0)) {
                    //  CALL TestCompSet(state, TRIM(cCurrentModuleObject),cAlphaArgs(1),cAlphaArgs(6),cAlphaArgs(7),'Air Nodes')
                }

                int NumPlantConnections = std::floor(rNumericArgs(1));
                auto &userZoneAirHVAC = state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop);
                if ((NumPlantConnections >= 1) && (NumPlantConnections <= 3)) {
                    userZoneAirHVAC.NumPlantConnections = NumPlantConnections;
                    userZoneAirHVAC.Loop.resize(NumPlantConnections);
                    for (int connectionIndex = 0; connectionIndex < NumPlantConnections; ++connectionIndex) {
                        int const connectionNum = connectionIndex + 1;
                        int aArgCount = connectionIndex * 2 + 8;
                        auto &plantConnection = userZoneAirHVAC.Loop[connectionIndex];
                        plantConnection.InletNodeNum = Node::GetOnlySingleNode(state,
                                                                               cAlphaArgs(aArgCount),
                                                                               ErrorsFound,
                                                                               Node::ConnectionObjectType::ZoneHVACForcedAirUserDefined,
                                                                               cAlphaArgs(1),
                                                                               Node::FluidType::Water,
                                                                               Node::ConnectionType::Inlet,
                                                                               static_cast<Node::CompFluidStream>(connectionNum + 2),
                                                                               Node::ObjectIsNotParent);
                        plantConnection.OutletNodeNum = Node::GetOnlySingleNode(state,
                                                                                cAlphaArgs(aArgCount + 1),
                                                                                ErrorsFound,
                                                                                Node::ConnectionObjectType::ZoneHVACForcedAirUserDefined,
                                                                                cAlphaArgs(1),
                                                                                Node::FluidType::Water,
                                                                                Node::ConnectionType::Outlet,
                                                                                static_cast<Node::CompFluidStream>(connectionNum + 2),
                                                                                Node::ObjectIsNotParent);
                        Node::TestCompSet(
                            state, cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(aArgCount), cAlphaArgs(aArgCount + 1), "Plant Nodes");
                        plantConnection.HowLoadServed = DataPlant::HowMet::NoneDemand;
                        plantConnection.FlowPriority = DataPlant::LoopFlowStatus::NeedyAndTurnsLoopOn;
                        // Setup Internal Variables
                        const std::string LoopStr = std::to_string(connectionNum);
                        // model input related internal variables
                        SetupEMSInternalVariable(
                            state, "Inlet Temperature for Plant Connection " + LoopStr, userZoneAirHVAC.Name, "[C]", plantConnection.InletTemp);
                        SetupEMSInternalVariable(state,
                                                 "Inlet Mass Flow Rate for Plant Connection " + LoopStr,
                                                 userZoneAirHVAC.Name,
                                                 "[kg/s]",
                                                 plantConnection.InletMassFlowRate);
                        SetupEMSInternalVariable(
                            state, "Inlet Density for Plant Connection " + LoopStr, userZoneAirHVAC.Name, "[kg/m3]", plantConnection.InletRho);
                        SetupEMSInternalVariable(
                            state, "Inlet Specific Heat for Plant Connection " + LoopStr, userZoneAirHVAC.Name, "[J/kg-C]", plantConnection.InletCp);
                        // model results related actuators
                        SetupEMSActuator(state,
                                         "Plant Connection " + LoopStr,
                                         userZoneAirHVAC.Name,
                                         "Outlet Temperature",
                                         "[C]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         plantConnection.OutletTemp);
                        SetupEMSActuator(state,
                                         "Plant Connection " + LoopStr,
                                         userZoneAirHVAC.Name,
                                         "Mass Flow Rate",
                                         "[kg/s]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         plantConnection.MassFlowRateRequest);
                        // model initialization and sizing related actuators
                        SetupEMSActuator(state,
                                         "Plant Connection " + LoopStr,
                                         userZoneAirHVAC.Name,
                                         "Minimum Mass Flow Rate",
                                         "[kg/s]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         plantConnection.MassFlowRateMin);
                        SetupEMSActuator(state,
                                         "Plant Connection " + LoopStr,
                                         userZoneAirHVAC.Name,
                                         "Maximum Mass Flow Rate",
                                         "[kg/s]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         plantConnection.MassFlowRateMax);
                        SetupEMSActuator(state,
                                         "Plant Connection " + LoopStr,
                                         userZoneAirHVAC.Name,
                                         "Design Volume Flow Rate",
                                         "[m3/s]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         plantConnection.DesignVolumeFlowRate);
                    }
                }

                if (!lAlphaFieldBlanks(14)) {
                    WaterManager::SetupTankDemandComponent(state,
                                                           cAlphaArgs(1),
                                                           cCurrentModuleObject,
                                                           cAlphaArgs(14),
                                                           ErrorsFound,
                                                           state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Water.SupplyTankID,
                                                           state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Water.SupplyTankDemandARRID);

                    state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Water.SuppliedByWaterSystem = true;
                    SetupEMSActuator(state,
                                     "Water System",
                                     state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                     "Supplied Volume Flow Rate",
                                     "[m3/s]",
                                     state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                     state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Water.SupplyVdotRequest);
                }

                if (!lAlphaFieldBlanks(15)) {
                    WaterManager::SetupTankSupplyComponent(
                        state,
                        cAlphaArgs(1),
                        cCurrentModuleObject,
                        cAlphaArgs(15),
                        ErrorsFound,
                        state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Water.CollectionTankID,
                        state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Water.CollectionTankSupplyARRID);
                    state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Water.CollectsToWaterSystem = true;
                    SetupEMSActuator(state,
                                     "Water System",
                                     state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                     "Collected Volume Flow Rate",
                                     "[m3/s]",
                                     state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                     state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Water.CollectedVdot);
                }

                if (!lAlphaFieldBlanks(16)) {

                    state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Zone.ZoneNum =
                        Util::FindItemInList(cAlphaArgs(16), state.dataHeatBal->Zone);
                    if (state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Zone.ZoneNum == 0) {
                        ShowSevereError(
                            state, std::format("{} = {}:  Ambient Zone Name not found = {}", cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(16)));
                        ErrorsFound = true;
                    } else {
                        state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Zone.DeviceHasInternalGains = true;
                        SetupZoneInternalGain(state,
                                              state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Zone.ZoneNum,
                                              cAlphaArgs(1),
                                              DataHeatBalance::IntGainType::ZoneHVACForcedAirUserDefined,
                                              &state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Zone.ConvectionGainRate,
                                              &state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Zone.ReturnAirConvectionGainRate,
                                              &state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Zone.ThermalRadiationGainRate,
                                              &state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Zone.LatentGainRate,
                                              &state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Zone.ReturnAirLatentGainRate,
                                              &state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Zone.CarbonDioxideGainRate,
                                              &state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Zone.GenericContamGainRate);

                        SetupEMSActuator(state,
                                         "Component Zone Internal Gain",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                         "Sensible Heat Gain Rate",
                                         "[W]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Zone.ConvectionGainRate);
                        SetupEMSActuator(state,
                                         "Component Zone Internal Gain",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                         "Return Air Heat Sensible Gain Rate",
                                         "[W]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Zone.ReturnAirConvectionGainRate);
                        SetupEMSActuator(state,
                                         "Component Zone Internal Gain",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                         "Thermal Radiation Heat Gain Rate",
                                         "[W]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Zone.ThermalRadiationGainRate);
                        SetupEMSActuator(state,
                                         "Component Zone Internal Gain",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                         "Latent Heat Gain Rate",
                                         "[W]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Zone.LatentGainRate);
                        SetupEMSActuator(state,
                                         "Component Zone Internal Gain",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                         "Return Air Latent Heat Gain Rate",
                                         "[W]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Zone.ReturnAirLatentGainRate);
                        SetupEMSActuator(state,
                                         "Component Zone Internal Gain",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                         "Carbon Dioxide Gain Rate",
                                         "[m3/s]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Zone.CarbonDioxideGainRate);
                        SetupEMSActuator(state,
                                         "Component Zone Internal Gain",
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Name,
                                         "Gaseous Contaminant Gain Rate",
                                         "[m3/s]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         state.dataUserDefinedComponents->UserZoneAirHVAC(CompLoop).Zone.GenericContamGainRate);
                    }
                }
            }
        } // NumUserZoneAir > 0

        if (ErrorsFound) {
            ShowFatalError(state, std::format("GetUserDefinedComponents: Errors found in processing {} input.", cCurrentModuleObject));
        }
    }

    void GetUserDefinedAirComponent(EnergyPlusData &state)
    {

        bool ErrorsFound(false);
        int NumAlphas; // Number of elements in the alpha array
        int NumNums;   // Number of elements in the numeric array
        int TotalArgs; // argument for call to GetObjectDefMaxArgs
        Array1D_string cAlphaFieldNames;
        Array1D_bool lAlphaFieldBlanks;
        Array1D_string cAlphaArgs;
        Array1D<Real64> rNumericArgs;
        std::string cCurrentModuleObject;

        if (state.dataUserDefinedComponents->GetPlantCompInput) {
            GetUserDefinedPlantComponents(state);
            state.dataUserDefinedComponents->GetPlantCompInput = false;
        }

        cCurrentModuleObject = "AirTerminal:SingleDuct:UserDefined";

        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, cCurrentModuleObject, TotalArgs, NumAlphas, NumNums);

        cAlphaFieldNames.allocate(NumAlphas);
        cAlphaArgs.allocate(NumAlphas);
        lAlphaFieldBlanks.dimension(NumAlphas, false);
        rNumericArgs.dimension(NumNums, 0.0);

        state.dataUserDefinedComponents->NumUserAirTerminals =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, cCurrentModuleObject);
        if (state.dataUserDefinedComponents->NumUserAirTerminals > 0) {
            state.dataUserDefinedComponents->UserAirTerminal.allocate(state.dataUserDefinedComponents->NumUserAirTerminals);
            state.dataUserDefinedComponents->CheckUserAirTerminal.assign(state.dataUserDefinedComponents->NumUserAirTerminals, true);
            int IOStat; // IO Status when calling get input subroutine
            for (int CompLoop = 1; CompLoop <= state.dataUserDefinedComponents->NumUserAirTerminals; ++CompLoop) {
                state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                         cCurrentModuleObject,
                                                                         CompLoop,
                                                                         cAlphaArgs,
                                                                         NumAlphas,
                                                                         rNumericArgs,
                                                                         NumNums,
                                                                         IOStat,
                                                                         _,
                                                                         lAlphaFieldBlanks,
                                                                         cAlphaFieldNames,
                                                                         _);

                state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name = cAlphaArgs(1);

                // now get program manager for model simulations
                if (!lAlphaFieldBlanks(2)) {
                    int StackMngrNum = Util::FindItemInList(cAlphaArgs(2), state.dataRuntimeLang->EMSProgramCallManager);
                    if (StackMngrNum > 0) { // found it
                        state.dataUserDefinedComponents->UserAirTerminal(CompLoop).ErlSimProgramMngr = StackMngrNum;
                    } else {
                        state.dataUserDefinedComponents->UserAirTerminal(CompLoop).simPluginLocation =
                            state.dataPluginManager->pluginManager->getLocationOfUserDefinedPlugin(state, cAlphaArgs(2));
                        if (state.dataUserDefinedComponents->UserAirTerminal(CompLoop).simPluginLocation == -1) {
                            state.dataUserDefinedComponents->UserAirTerminal(CompLoop).simCallbackIndex =
                                state.dataPluginManager->pluginManager->getUserDefinedCallbackIndex(state, cAlphaArgs(2));
                            if (state.dataUserDefinedComponents->UserAirTerminal(CompLoop).simCallbackIndex == -1) {
                                ShowSevereError(state, std::format("Invalid {}={}", cAlphaFieldNames(2), cAlphaArgs(2)));
                                ShowContinueError(state, std::format("Entered in {}={}", cCurrentModuleObject, cAlphaArgs(1)));
                                ShowContinueError(
                                    state,
                                    "Program Manager Name not found as an EMS Program Manager, API callback or a Python Plugin Instance object.");
                                ErrorsFound = true;
                            }
                        }
                    }
                }

                // now get program manager for model initializations
                if (!lAlphaFieldBlanks(3)) {
                    int StackMngrNum = Util::FindItemInList(cAlphaArgs(3), state.dataRuntimeLang->EMSProgramCallManager);
                    if (StackMngrNum > 0) { // found it
                        state.dataUserDefinedComponents->UserAirTerminal(CompLoop).ErlInitProgramMngr = StackMngrNum;
                    } else {
                        state.dataUserDefinedComponents->UserAirTerminal(CompLoop).initPluginLocation =
                            state.dataPluginManager->pluginManager->getLocationOfUserDefinedPlugin(state, cAlphaArgs(3));
                        if (state.dataUserDefinedComponents->UserAirTerminal(CompLoop).initPluginLocation == -1) {
                            state.dataUserDefinedComponents->UserAirTerminal(CompLoop).initCallbackIndex =
                                state.dataPluginManager->pluginManager->getUserDefinedCallbackIndex(state, cAlphaArgs(3));
                            if (state.dataUserDefinedComponents->UserAirTerminal(CompLoop).initCallbackIndex == -1) {
                                ShowSevereError(state, std::format("Invalid {}={}", cAlphaFieldNames(3), cAlphaArgs(3)));
                                ShowContinueError(state, std::format("Entered in {}={}", cCurrentModuleObject, cAlphaArgs(1)));
                                ShowContinueError(
                                    state,
                                    "Program Manager Name not found as an EMS Program Manager, API callback or a Python Plugin Instance object.");
                                ErrorsFound = true;
                            }
                        }
                    }
                }

                state.dataUserDefinedComponents->UserAirTerminal(CompLoop).AirConnection.InletNodeNum =
                    Node::GetOnlySingleNode(state,
                                            cAlphaArgs(4),
                                            ErrorsFound,
                                            Node::ConnectionObjectType::AirTerminalSingleDuctUserDefined,
                                            state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                            Node::FluidType::Air,
                                            Node::ConnectionType::Inlet,
                                            Node::CompFluidStream::Primary,
                                            Node::ObjectIsNotParent,
                                            cAlphaFieldNames(4));
                // model input related internal variables
                SetupEMSInternalVariable(state,
                                         "Inlet Temperature for Primary Air Connection",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                         "[C]",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).AirConnection.InletTemp);
                SetupEMSInternalVariable(state,
                                         "Inlet Humidity Ratio for Primary Air Connection",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                         "[kgWater/kgDryAir]",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).AirConnection.InletHumRat);
                SetupEMSInternalVariable(state,
                                         "Inlet Density for Primary Air Connection",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                         "[kg/m3]",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).AirConnection.InletRho);
                SetupEMSInternalVariable(state,
                                         "Inlet Specific Heat for Primary Air Connection",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                         "[J/kg-C]",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).AirConnection.InletCp);

                SetupEMSInternalVariable(state,
                                         "Remaining Sensible Load to Heating Setpoint",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                         "[W]",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).RemainingOutputToHeatingSP);
                SetupEMSInternalVariable(state,
                                         "Remaining Sensible Load to Cooling Setpoint",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                         "[W]",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).RemainingOutputToCoolingSP);
                SetupEMSInternalVariable(state,
                                         "Remaining Latent Load to Humidifying Setpoint",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                         "[kg/s]",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).RemainingOutputReqToHumidSP);
                SetupEMSInternalVariable(state,
                                         "Remaining Latent Load to Dehumidifying Setpoint",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                         "[kg/s]",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).RemainingOutputReqToDehumidSP);

                SetupEMSActuator(state,
                                 "Primary Air Connection",
                                 state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                 "Inlet Mass Flow Rate",
                                 "[kg/s]",
                                 state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                 state.dataUserDefinedComponents->UserAirTerminal(CompLoop).AirConnection.InletMassFlowRate);
                state.dataUserDefinedComponents->UserAirTerminal(CompLoop).AirConnection.OutletNodeNum =
                    Node::GetOnlySingleNode(state,
                                            cAlphaArgs(5),
                                            ErrorsFound,
                                            Node::ConnectionObjectType::AirTerminalSingleDuctUserDefined,
                                            state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                            Node::FluidType::Air,
                                            Node::ConnectionType::Outlet,
                                            Node::CompFluidStream::Primary,
                                            Node::ObjectIsNotParent,
                                            cAlphaFieldNames(5));
                SetupEMSActuator(state,
                                 "Primary Air Connection",
                                 state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                 "Outlet Temperature",
                                 "[C]",
                                 state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                 state.dataUserDefinedComponents->UserAirTerminal(CompLoop).AirConnection.OutletTemp);
                SetupEMSActuator(state,
                                 "Primary Air Connection",
                                 state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                 "Outlet Humidity Ratio",
                                 "[kgWater/kgDryAir]",
                                 state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                 state.dataUserDefinedComponents->UserAirTerminal(CompLoop).AirConnection.OutletHumRat);
                SetupEMSActuator(state,
                                 "Primary Air Connection",
                                 state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                 "Outlet Mass Flow Rate",
                                 "[kg/s]",
                                 state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                 state.dataUserDefinedComponents->UserAirTerminal(CompLoop).AirConnection.OutletMassFlowRate);
                Node::TestCompSet(state, cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(4), cAlphaArgs(5), "Air Nodes");

                if (state.dataUserDefinedComponents->NumUserAirTerminals > 0) { // Skip this code if the only User Defined type is ZoneHVAC
                    for (int ADUNum = 1; ADUNum <= (int)state.dataDefineEquipment->AirDistUnit.size(); ++ADUNum) {
                        if (state.dataUserDefinedComponents->UserAirTerminal(CompLoop).AirConnection.OutletNodeNum ==
                            state.dataDefineEquipment->AirDistUnit(ADUNum).OutletNodeNum) {
                            //        AirDistUnit(ADUNum)%InletNodeNum = IndUnitIUNum)%InletNodeNum
                            state.dataUserDefinedComponents->UserAirTerminal(CompLoop).ADUNum = ADUNum;
                        }
                    }
                    // one assumes if there isn't one assigned, it's an error?
                    if (state.dataUserDefinedComponents->UserAirTerminal(CompLoop).ADUNum == 0) {
                        ShowSevereError(state,
                                        std::format("GetUserDefinedComponents: No matching Air Distribution Unit for {} = {}",
                                                    cCurrentModuleObject,
                                                    state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name));
                        ShowContinueError(state,
                                          std::format("...should have outlet node={}",
                                                      state.dataLoopNodes->NodeID(
                                                          state.dataUserDefinedComponents->UserAirTerminal(CompLoop).AirConnection.OutletNodeNum)));
                        //          ErrorsFound=.TRUE.
                    }

                    // Fill the Zone Equipment data with the inlet node number of this unit.
                    for (int CtrlZone = 1; CtrlZone <= state.dataGlobal->NumOfZones; ++CtrlZone) {
                        if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZone).IsControlled) {
                            continue;
                        }
                        for (int SupAirIn = 1; SupAirIn <= state.dataZoneEquip->ZoneEquipConfig(CtrlZone).NumInletNodes; ++SupAirIn) {
                            if (state.dataUserDefinedComponents->UserAirTerminal(CompLoop).AirConnection.OutletNodeNum ==
                                state.dataZoneEquip->ZoneEquipConfig(CtrlZone).InletNode(SupAirIn)) {
                                if (state.dataZoneEquip->ZoneEquipConfig(CtrlZone).AirDistUnitCool(SupAirIn).OutNode > 0) {
                                    ShowSevereError(state, "Error in connecting a terminal unit to a zone");
                                    ShowContinueError(
                                        state,
                                        std::format("{} already connects to another zone",
                                                    state.dataLoopNodes->NodeID(
                                                        state.dataUserDefinedComponents->UserAirTerminal(CompLoop).AirConnection.OutletNodeNum)));
                                    ShowContinueError(state,
                                                      std::format("Occurs for terminal unit {} = {}",
                                                                  cCurrentModuleObject,
                                                                  state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name));
                                    ShowContinueError(state, "Check terminal unit node names for errors");
                                    ErrorsFound = true;
                                } else {
                                    state.dataZoneEquip->ZoneEquipConfig(CtrlZone).AirDistUnitCool(SupAirIn).InNode =
                                        state.dataUserDefinedComponents->UserAirTerminal(CompLoop).AirConnection.InletNodeNum;
                                    state.dataZoneEquip->ZoneEquipConfig(CtrlZone).AirDistUnitCool(SupAirIn).OutNode =
                                        state.dataUserDefinedComponents->UserAirTerminal(CompLoop).AirConnection.OutletNodeNum;
                                    state.dataDefineEquipment->AirDistUnit(state.dataUserDefinedComponents->UserAirTerminal(CompLoop).ADUNum)
                                        .TermUnitSizingNum =
                                        state.dataZoneEquip->ZoneEquipConfig(CtrlZone).AirDistUnitCool(SupAirIn).TermUnitSizingIndex;
                                    state.dataDefineEquipment->AirDistUnit(state.dataUserDefinedComponents->UserAirTerminal(CompLoop).ADUNum)
                                        .ZoneEqNum = CtrlZone;
                                }

                                state.dataUserDefinedComponents->UserAirTerminal(CompLoop).ActualCtrlZoneNum = CtrlZone;
                            }
                        }
                    }
                }

                if (!lAlphaFieldBlanks(6)) {
                    state.dataUserDefinedComponents->UserAirTerminal(CompLoop).SourceAir.InletNodeNum =
                        Node::GetOnlySingleNode(state,
                                                cAlphaArgs(6),
                                                ErrorsFound,
                                                Node::ConnectionObjectType::AirTerminalSingleDuctUserDefined,
                                                state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                                Node::FluidType::Air,
                                                Node::ConnectionType::Inlet,
                                                Node::CompFluidStream::Secondary,
                                                Node::ObjectIsNotParent,
                                                cAlphaFieldNames(6));
                    // model input related internal variables
                    SetupEMSInternalVariable(state,
                                             "Inlet Temperature for Secondary Air Connection",
                                             state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                             "[C]",
                                             state.dataUserDefinedComponents->UserAirTerminal(CompLoop).SourceAir.InletTemp);

                    SetupEMSInternalVariable(state,
                                             "Inlet Humidity Ratio for Secondary Air Connection",
                                             state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                             "[kgWater/kgDryAir]",
                                             state.dataUserDefinedComponents->UserAirTerminal(CompLoop).SourceAir.InletHumRat);
                    SetupEMSInternalVariable(state,
                                             "Inlet Density for Secondary Air Connection",
                                             state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                             "[kg/m3]",
                                             state.dataUserDefinedComponents->UserAirTerminal(CompLoop).SourceAir.InletRho);
                    SetupEMSInternalVariable(state,
                                             "Inlet Specific Heat for Secondary Air Connection",
                                             state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                             "[J/kg-C]",
                                             state.dataUserDefinedComponents->UserAirTerminal(CompLoop).SourceAir.InletCp);
                    SetupEMSActuator(state,
                                     "Secondary Air Connection",
                                     state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                     "Inlet Mass Flow Rate",
                                     "[kg/s]",
                                     state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                     state.dataUserDefinedComponents->UserAirTerminal(CompLoop).SourceAir.InletMassFlowRate);
                }

                if (!lAlphaFieldBlanks(7)) {
                    state.dataUserDefinedComponents->UserAirTerminal(CompLoop).SourceAir.OutletNodeNum =
                        Node::GetOnlySingleNode(state,
                                                cAlphaArgs(7),
                                                ErrorsFound,
                                                Node::ConnectionObjectType::AirTerminalSingleDuctUserDefined,
                                                state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                                Node::FluidType::Air,
                                                Node::ConnectionType::Outlet,
                                                Node::CompFluidStream::Secondary,
                                                Node::ObjectIsNotParent,
                                                cAlphaFieldNames(7));
                    SetupEMSActuator(state,
                                     "Secondary Air Connection",
                                     state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                     "Outlet Temperature",
                                     "[C]",
                                     state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                     state.dataUserDefinedComponents->UserAirTerminal(CompLoop).SourceAir.OutletTemp);
                    SetupEMSActuator(state,
                                     "Secondary Air Connection",
                                     state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                     "Outlet Humidity Ratio",
                                     "[kgWater/kgDryAir]",
                                     state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                     state.dataUserDefinedComponents->UserAirTerminal(CompLoop).SourceAir.OutletHumRat);
                    SetupEMSActuator(state,
                                     "Secondary Air Connection",
                                     state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                     "Mass Flow Rate",
                                     "[kg/s]",
                                     state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                     state.dataUserDefinedComponents->UserAirTerminal(CompLoop).SourceAir.OutletMassFlowRate);
                }

                if ((state.dataUserDefinedComponents->UserAirTerminal(CompLoop).SourceAir.InletNodeNum > 0) &&
                    (state.dataUserDefinedComponents->UserAirTerminal(CompLoop).SourceAir.OutletNodeNum > 0)) {
                }

                int NumPlantConnections = std::floor(rNumericArgs(1));
                auto &userAirTerminal = state.dataUserDefinedComponents->UserAirTerminal(CompLoop);
                if ((NumPlantConnections >= 1) && (NumPlantConnections <= 2)) {
                    userAirTerminal.NumPlantConnections = NumPlantConnections;
                    userAirTerminal.Loop.resize(NumPlantConnections);
                    for (int connectionIndex = 0; connectionIndex < NumPlantConnections; ++connectionIndex) {
                        int const connectionNum = connectionIndex + 1;
                        int aArgCount = connectionIndex * 2 + 8;
                        auto &plantConnection = userAirTerminal.Loop[connectionIndex];
                        plantConnection.InletNodeNum = Node::GetOnlySingleNode(state,
                                                                               cAlphaArgs(aArgCount),
                                                                               ErrorsFound,
                                                                               Node::ConnectionObjectType::AirTerminalSingleDuctUserDefined,
                                                                               cAlphaArgs(1),
                                                                               Node::FluidType::Water,
                                                                               Node::ConnectionType::Inlet,
                                                                               static_cast<Node::CompFluidStream>(connectionNum + 2),
                                                                               Node::ObjectIsNotParent,
                                                                               cAlphaFieldNames(aArgCount));
                        plantConnection.OutletNodeNum = Node::GetOnlySingleNode(state,
                                                                                cAlphaArgs(aArgCount + 1),
                                                                                ErrorsFound,
                                                                                Node::ConnectionObjectType::AirTerminalSingleDuctUserDefined,
                                                                                cAlphaArgs(1),
                                                                                Node::FluidType::Water,
                                                                                Node::ConnectionType::Outlet,
                                                                                static_cast<Node::CompFluidStream>(connectionNum + 2),
                                                                                Node::ObjectIsNotParent,
                                                                                cAlphaFieldNames(aArgCount + 1));
                        Node::TestCompSet(
                            state, cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(aArgCount), cAlphaArgs(aArgCount + 1), "Plant Nodes");
                        plantConnection.HowLoadServed = DataPlant::HowMet::NoneDemand;
                        plantConnection.FlowPriority = DataPlant::LoopFlowStatus::NeedyAndTurnsLoopOn;
                        // Setup Internal Variables
                        const std::string LoopStr = std::to_string(connectionNum);
                        // model input related internal variables
                        SetupEMSInternalVariable(
                            state, "Inlet Temperature for Plant Connection " + LoopStr, userAirTerminal.Name, "[C]", plantConnection.InletTemp);
                        SetupEMSInternalVariable(state,
                                                 "Inlet Mass Flow Rate for Plant Connection " + LoopStr,
                                                 userAirTerminal.Name,
                                                 "[kg/s]",
                                                 plantConnection.InletMassFlowRate);
                        SetupEMSInternalVariable(
                            state, "Inlet Density for Plant Connection " + LoopStr, userAirTerminal.Name, "[kg/m3]", plantConnection.InletRho);
                        SetupEMSInternalVariable(
                            state, "Inlet Specific Heat for Plant Connection " + LoopStr, userAirTerminal.Name, "[J/kg-C]", plantConnection.InletCp);
                        // model results related actuators
                        SetupEMSActuator(state,
                                         "Plant Connection " + LoopStr,
                                         userAirTerminal.Name,
                                         "Outlet Temperature",
                                         "[C]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         plantConnection.OutletTemp);
                        SetupEMSActuator(state,
                                         "Plant Connection " + LoopStr,
                                         userAirTerminal.Name,
                                         "Mass Flow Rate",
                                         "[kg/s]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         plantConnection.MassFlowRateRequest);
                        // model initialization and sizing related actuators
                        SetupEMSActuator(state,
                                         "Plant Connection " + LoopStr,
                                         userAirTerminal.Name,
                                         "Minimum Mass Flow Rate",
                                         "[kg/s]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         plantConnection.MassFlowRateMin);
                        SetupEMSActuator(state,
                                         "Plant Connection " + LoopStr,
                                         userAirTerminal.Name,
                                         "Maximum Mass Flow Rate",
                                         "[kg/s]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         plantConnection.MassFlowRateMax);
                        SetupEMSActuator(state,
                                         "Plant Connection " + LoopStr,
                                         userAirTerminal.Name,
                                         "Design Volume Flow Rate",
                                         "[m3/s]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         plantConnection.DesignVolumeFlowRate);
                    }
                }

                if (!lAlphaFieldBlanks(12)) {
                    WaterManager::SetupTankDemandComponent(state,
                                                           cAlphaArgs(1),
                                                           cCurrentModuleObject,
                                                           cAlphaArgs(12),
                                                           ErrorsFound,
                                                           state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Water.SupplyTankID,
                                                           state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Water.SupplyTankDemandARRID);

                    state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Water.SuppliedByWaterSystem = true;
                    SetupEMSActuator(state,
                                     "Water System",
                                     state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                     "Supplied Volume Flow Rate",
                                     "[m3/s]",
                                     state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                     state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Water.SupplyVdotRequest);
                }

                if (!lAlphaFieldBlanks(13)) {
                    WaterManager::SetupTankSupplyComponent(
                        state,
                        cAlphaArgs(1),
                        cCurrentModuleObject,
                        cAlphaArgs(13),
                        ErrorsFound,
                        state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Water.CollectionTankID,
                        state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Water.CollectionTankSupplyARRID);
                    state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Water.CollectsToWaterSystem = true;
                    SetupEMSActuator(state,
                                     "Water System",
                                     state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                     "Collected Volume Flow Rate",
                                     "[m3/s]",
                                     state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                     state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Water.CollectedVdot);
                }

                if (!lAlphaFieldBlanks(14)) {

                    state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Zone.ZoneNum =
                        Util::FindItemInList(cAlphaArgs(14), state.dataHeatBal->Zone);
                    if (state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Zone.ZoneNum == 0) {
                        ShowSevereError(
                            state, std::format("{} = {}:  Ambient Zone Name not found = {}", cCurrentModuleObject, cAlphaArgs(1), cAlphaArgs(14)));
                        ErrorsFound = true;
                    } else {
                        state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Zone.DeviceHasInternalGains = true;
                        SetupZoneInternalGain(state,
                                              state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Zone.ZoneNum,
                                              cAlphaArgs(1),
                                              DataHeatBalance::IntGainType::AirTerminalUserDefined,
                                              &state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Zone.ConvectionGainRate,
                                              &state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Zone.ReturnAirConvectionGainRate,
                                              &state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Zone.ThermalRadiationGainRate,
                                              &state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Zone.LatentGainRate,
                                              &state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Zone.ReturnAirLatentGainRate,
                                              &state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Zone.CarbonDioxideGainRate,
                                              &state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Zone.GenericContamGainRate);

                        SetupEMSActuator(state,
                                         "Component Zone Internal Gain",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                         "Sensible Heat Gain Rate",
                                         "[W]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Zone.ConvectionGainRate);
                        SetupEMSActuator(state,
                                         "Component Zone Internal Gain",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                         "Return Air Heat Sensible Gain Rate",
                                         "[W]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Zone.ReturnAirConvectionGainRate);
                        SetupEMSActuator(state,
                                         "Component Zone Internal Gain",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                         "Thermal Radiation Heat Gain Rate",
                                         "[W]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Zone.ThermalRadiationGainRate);
                        SetupEMSActuator(state,
                                         "Component Zone Internal Gain",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                         "Latent Heat Gain Rate",
                                         "[W]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Zone.LatentGainRate);
                        SetupEMSActuator(state,
                                         "Component Zone Internal Gain",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                         "Return Air Latent Heat Gain Rate",
                                         "[W]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Zone.ReturnAirLatentGainRate);
                        SetupEMSActuator(state,
                                         "Component Zone Internal Gain",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                         "Carbon Dioxide Gain Rate",
                                         "[W]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Zone.CarbonDioxideGainRate);
                        SetupEMSActuator(state,
                                         "Component Zone Internal Gain",
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Name,
                                         "Gaseous Contaminant Gain Rate",
                                         "[W]",
                                         state.dataUserDefinedComponents->lDummy_GetUserDefComp,
                                         state.dataUserDefinedComponents->UserAirTerminal(CompLoop).Zone.GenericContamGainRate);
                    }
                }
            }
        } // NumUserZoneAir > 0

        if (ErrorsFound) {
            ShowFatalError(state, std::format("GetUserDefinedComponents: Errors found in processing {} input.", cCurrentModuleObject));
        }
    }

    void UserPlantComponentStruct::initialize(EnergyPlusData &state, int LoopNum, Real64 MyLoad)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR         <author>
        //       DATE WRITTEN   <date_written>
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        static constexpr std::string_view RoutineName("InitPlantUserComponent");

        this->oneTimeInit(state);

        if (LoopNum < 0 || LoopNum >= this->NumPlantConnections) {
            return;
        }

        // fill internal variable targets
        auto &plantConnection = this->Loop[LoopNum];
        plantConnection.MyLoad = MyLoad;
        plantConnection.InletRho =
            plantConnection.plantLoc.loop->glycol->getDensity(state, state.dataLoopNodes->Node(plantConnection.InletNodeNum).Temp, RoutineName);
        plantConnection.InletCp =
            plantConnection.plantLoc.loop->glycol->getSpecificHeat(state, state.dataLoopNodes->Node(plantConnection.InletNodeNum).Temp, RoutineName);
        plantConnection.InletMassFlowRate = state.dataLoopNodes->Node(plantConnection.InletNodeNum).MassFlowRate;
        plantConnection.InletTemp = state.dataLoopNodes->Node(plantConnection.InletNodeNum).Temp;
        if (this->Air.InletNodeNum > 0) {
            this->Air.InletRho = Psychrometrics::PsyRhoAirFnPbTdbW(state,
                                                                   state.dataEnvrn->OutBaroPress,
                                                                   state.dataLoopNodes->Node(this->Air.InletNodeNum).Temp,
                                                                   state.dataLoopNodes->Node(this->Air.InletNodeNum).HumRat,
                                                                   RoutineName);
            this->Air.InletCp = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(this->Air.InletNodeNum).HumRat);
            this->Air.InletTemp = state.dataLoopNodes->Node(this->Air.InletNodeNum).Temp;
            this->Air.InletMassFlowRate = state.dataLoopNodes->Node(this->Air.InletNodeNum).MassFlowRate;
            this->Air.InletHumRat = state.dataLoopNodes->Node(this->Air.InletNodeNum).HumRat;
        }
    }

    void UserCoilComponentStruct::initialize(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         <author>
        //       DATE WRITTEN   <date_written>
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        static constexpr std::string_view RoutineName("InitCoilUserDefined");

        if (this->myOneTimeFlag) {
            if (this->PlantIsConnected) {
                bool errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(
                    state, this->Name, DataPlant::PlantEquipmentType::CoilUserDefined, this->Loop.plantLoc, errFlag);
                if (errFlag) {
                    ShowFatalError(state, "InitPlantUserComponent: Program terminated due to previous condition(s).");
                }
                // set user input for flow priority
                DataPlant::CompData::getPlantComponent(state, this->Loop.plantLoc).FlowPriority = this->Loop.FlowPriority;

                // set user input for how loads served
                DataPlant::CompData::getPlantComponent(state, this->Loop.plantLoc).HowLoadServed = this->Loop.HowLoadServed;
            }
            this->myOneTimeFlag = false;
        }

        // fill internal variable targets
        for (int loop = 0; loop < this->NumAirConnections; ++loop) {
            auto &airConnection = this->AirConnections[loop];
            airConnection.InletRho = Psychrometrics::PsyRhoAirFnPbTdbW(state,
                                                                       state.dataEnvrn->OutBaroPress,
                                                                       state.dataLoopNodes->Node(airConnection.InletNodeNum).Temp,
                                                                       state.dataLoopNodes->Node(airConnection.InletNodeNum).HumRat,
                                                                       RoutineName);

            airConnection.InletCp = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(airConnection.InletNodeNum).HumRat);
            airConnection.InletTemp = state.dataLoopNodes->Node(airConnection.InletNodeNum).Temp;
            airConnection.InletMassFlowRate = state.dataLoopNodes->Node(airConnection.InletNodeNum).MassFlowRate;
            airConnection.InletHumRat = state.dataLoopNodes->Node(airConnection.InletNodeNum).HumRat;
        }

        if (this->PlantIsConnected) {
            this->Loop.InletRho =
                this->Loop.plantLoc.loop->glycol->getDensity(state, state.dataLoopNodes->Node(this->Loop.InletNodeNum).Temp, RoutineName);
            this->Loop.InletCp =
                this->Loop.plantLoc.loop->glycol->getSpecificHeat(state, state.dataLoopNodes->Node(this->Loop.InletNodeNum).Temp, RoutineName);
            this->Loop.InletTemp = state.dataLoopNodes->Node(this->Loop.InletNodeNum).Temp;
            this->Loop.InletMassFlowRate = state.dataLoopNodes->Node(this->Loop.InletNodeNum).MassFlowRate;
        }
    }

    void UserZoneHVACForcedAirComponentStruct::initialize(EnergyPlusData &state, int const ZoneNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   Feb. 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // initialize data for user-defined zone HVAC forced air component model

        static constexpr std::string_view RoutineName("InitZoneAirUserDefined");

        if (this->myOneTimeFlag) {
            if (this->NumPlantConnections > 0) {
                for (int loop = 0; loop < this->NumPlantConnections; ++loop) {
                    auto &plantConnection = this->Loop[loop];
                    bool errFlag = false;
                    PlantUtilities::ScanPlantLoopsForObject(state,
                                                            this->Name,
                                                            DataPlant::PlantEquipmentType::ZoneHVACAirUserDefined,
                                                            plantConnection.plantLoc,
                                                            errFlag,
                                                            _,
                                                            _,
                                                            _,
                                                            plantConnection.InletNodeNum,
                                                            ObjexxFCL::Optional_int_const());
                    if (errFlag) {
                        ShowFatalError(state, "InitPlantUserComponent: Program terminated due to previous condition(s).");
                    }
                    // set user input for flow priority
                    DataPlant::CompData::getPlantComponent(state, plantConnection.plantLoc).FlowPriority = plantConnection.FlowPriority;

                    // set user input for how loads served
                    DataPlant::CompData::getPlantComponent(state, plantConnection.plantLoc).HowLoadServed = plantConnection.HowLoadServed;
                }
            }
            this->myOneTimeFlag = false;
        }
        // fill internal variable targets
        this->RemainingOutputToHeatingSP = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToHeatSP;
        this->RemainingOutputToCoolingSP = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToCoolSP;
        this->RemainingOutputReqToDehumidSP = state.dataZoneEnergyDemand->ZoneSysMoistureDemand(ZoneNum).RemainingOutputReqToDehumidSP;
        this->RemainingOutputReqToHumidSP = state.dataZoneEnergyDemand->ZoneSysMoistureDemand(ZoneNum).RemainingOutputReqToHumidSP;

        this->AirConnection.InletRho = Psychrometrics::PsyRhoAirFnPbTdbW(state,
                                                                         state.dataEnvrn->OutBaroPress,
                                                                         state.dataLoopNodes->Node(this->AirConnection.InletNodeNum).Temp,
                                                                         state.dataLoopNodes->Node(this->AirConnection.InletNodeNum).HumRat,
                                                                         RoutineName);
        this->AirConnection.InletCp = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(this->AirConnection.InletNodeNum).HumRat);
        this->AirConnection.InletTemp = state.dataLoopNodes->Node(this->AirConnection.InletNodeNum).Temp;
        this->AirConnection.InletHumRat = state.dataLoopNodes->Node(this->AirConnection.InletNodeNum).HumRat;

        if (this->SourceAir.InletNodeNum > 0) {
            this->SourceAir.InletRho = Psychrometrics::PsyRhoAirFnPbTdbW(state,
                                                                         state.dataEnvrn->OutBaroPress,
                                                                         state.dataLoopNodes->Node(this->SourceAir.InletNodeNum).Temp,
                                                                         state.dataLoopNodes->Node(this->SourceAir.InletNodeNum).HumRat,
                                                                         RoutineName);
            this->SourceAir.InletCp = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(this->SourceAir.InletNodeNum).HumRat);
            this->SourceAir.InletTemp = state.dataLoopNodes->Node(this->SourceAir.InletNodeNum).Temp;
            this->SourceAir.InletHumRat = state.dataLoopNodes->Node(this->SourceAir.InletNodeNum).HumRat;
        }

        if (this->NumPlantConnections > 0) {
            for (int loop = 0; loop < this->NumPlantConnections; ++loop) {
                auto &plantConnection = this->Loop[loop];
                plantConnection.InletRho = plantConnection.plantLoc.loop->glycol->getDensity(
                    state, state.dataLoopNodes->Node(plantConnection.InletNodeNum).Temp, RoutineName);
                plantConnection.InletCp = plantConnection.plantLoc.loop->glycol->getSpecificHeat(
                    state, state.dataLoopNodes->Node(plantConnection.InletNodeNum).Temp, RoutineName);
                plantConnection.InletTemp = state.dataLoopNodes->Node(plantConnection.InletNodeNum).Temp;
                plantConnection.InletMassFlowRate = state.dataLoopNodes->Node(plantConnection.InletNodeNum).MassFlowRate;
            }
        }
    }

    void UserAirTerminalComponentStruct::initialize(EnergyPlusData &state, int const ZoneNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   March 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        static constexpr std::string_view RoutineName("InitAirTerminalUserDefined");

        if (this->myOneTimeFlag) {
            if (this->NumPlantConnections > 0) {
                for (int loop = 0; loop < this->NumPlantConnections; ++loop) {
                    auto &plantConnection = this->Loop[loop];
                    bool errFlag = false;
                    PlantUtilities::ScanPlantLoopsForObject(state,
                                                            this->Name,
                                                            DataPlant::PlantEquipmentType::AirTerminalUserDefined,
                                                            plantConnection.plantLoc,
                                                            errFlag,
                                                            _,
                                                            _,
                                                            _,
                                                            plantConnection.InletNodeNum,
                                                            ObjexxFCL::Optional_int_const());
                    if (errFlag) {
                        ShowFatalError(state, "InitPlantUserComponent: Program terminated due to previous condition(s).");
                    }
                    // set user input for flow priority
                    DataPlant::CompData::getPlantComponent(state, plantConnection.plantLoc).FlowPriority = plantConnection.FlowPriority;

                    // set user input for how loads served
                    DataPlant::CompData::getPlantComponent(state, plantConnection.plantLoc).HowLoadServed = plantConnection.HowLoadServed;
                }
            }
            this->myOneTimeFlag = false;
        }
        // fill internal variable targets
        this->RemainingOutputToHeatingSP = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToHeatSP;
        this->RemainingOutputToCoolingSP = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToCoolSP;
        this->RemainingOutputReqToDehumidSP = state.dataZoneEnergyDemand->ZoneSysMoistureDemand(ZoneNum).RemainingOutputReqToDehumidSP;
        this->RemainingOutputReqToHumidSP = state.dataZoneEnergyDemand->ZoneSysMoistureDemand(ZoneNum).RemainingOutputReqToHumidSP;

        this->AirConnection.InletRho = Psychrometrics::PsyRhoAirFnPbTdbW(state,
                                                                         state.dataEnvrn->OutBaroPress,
                                                                         state.dataLoopNodes->Node(this->AirConnection.InletNodeNum).Temp,
                                                                         state.dataLoopNodes->Node(this->AirConnection.InletNodeNum).HumRat,
                                                                         RoutineName);
        this->AirConnection.InletCp = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(this->AirConnection.InletNodeNum).HumRat);
        this->AirConnection.InletTemp = state.dataLoopNodes->Node(this->AirConnection.InletNodeNum).Temp;
        this->AirConnection.InletHumRat = state.dataLoopNodes->Node(this->AirConnection.InletNodeNum).HumRat;

        if (this->SourceAir.InletNodeNum > 0) {
            this->SourceAir.InletRho = Psychrometrics::PsyRhoAirFnPbTdbW(state,
                                                                         state.dataEnvrn->OutBaroPress,
                                                                         state.dataLoopNodes->Node(this->SourceAir.InletNodeNum).Temp,
                                                                         state.dataLoopNodes->Node(this->SourceAir.InletNodeNum).HumRat,
                                                                         RoutineName);
            this->SourceAir.InletCp = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(this->SourceAir.InletNodeNum).HumRat);
            this->SourceAir.InletTemp = state.dataLoopNodes->Node(this->SourceAir.InletNodeNum).Temp;
            this->SourceAir.InletHumRat = state.dataLoopNodes->Node(this->SourceAir.InletNodeNum).HumRat;
        }

        if (this->NumPlantConnections > 0) {
            for (int loop = 0; loop < this->NumPlantConnections; ++loop) {
                auto &plantConnection = this->Loop[loop];
                plantConnection.InletRho = plantConnection.plantLoc.loop->glycol->getDensity(
                    state, state.dataLoopNodes->Node(plantConnection.InletNodeNum).Temp, RoutineName);
                plantConnection.InletCp = plantConnection.plantLoc.loop->glycol->getSpecificHeat(
                    state, state.dataLoopNodes->Node(plantConnection.InletNodeNum).Temp, RoutineName);
                plantConnection.InletTemp = state.dataLoopNodes->Node(plantConnection.InletNodeNum).Temp;
                plantConnection.InletMassFlowRate = state.dataLoopNodes->Node(plantConnection.InletNodeNum).MassFlowRate;
            }
        }
    }

    void UserPlantComponentStruct::report(EnergyPlusData &state, int const LoopNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Feb. 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // report model results

        // METHODOLOGY EMPLOYED:
        // copy actuated values to structures elsewhere in program.

        if (LoopNum < 0 || LoopNum >= this->NumPlantConnections) {
            return;
        }

        auto &plantConnection = this->Loop[LoopNum];

        PlantUtilities::SafeCopyPlantNode(state, plantConnection.InletNodeNum, plantConnection.OutletNodeNum);

        // unload Actuators to node data structure

        state.dataLoopNodes->Node(plantConnection.OutletNodeNum).Temp = plantConnection.OutletTemp;

        // make mass flow requests, just this loop
        PlantUtilities::SetComponentFlowRate(
            state, plantConnection.MassFlowRateRequest, plantConnection.InletNodeNum, plantConnection.OutletNodeNum, plantConnection.plantLoc);

        if (this->Air.OutletNodeNum > 0) {
            state.dataLoopNodes->Node(this->Air.OutletNodeNum).Temp = this->Air.OutletTemp;
            state.dataLoopNodes->Node(this->Air.OutletNodeNum).HumRat = this->Air.OutletHumRat;
            state.dataLoopNodes->Node(this->Air.OutletNodeNum).MassFlowRate = this->Air.OutletMassFlowRate;
            state.dataLoopNodes->Node(this->Air.OutletNodeNum).Enthalpy = Psychrometrics::PsyHFnTdbW(this->Air.OutletTemp, this->Air.OutletHumRat);
        }

        if (this->Water.SuppliedByWaterSystem) {
            state.dataWaterData->WaterStorage(this->Water.SupplyTankID).VdotRequestDemand(this->Water.SupplyTankDemandARRID) =
                this->Water.SupplyVdotRequest;
        }

        if (this->Water.CollectsToWaterSystem) {
            state.dataWaterData->WaterStorage(this->Water.CollectionTankID).VdotAvailSupply(this->Water.CollectionTankSupplyARRID) =
                this->Water.CollectedVdot;
        }

        if (plantConnection.HowLoadServed == DataPlant::HowMet::ByNominalCapLowOutLimit) {
            DataPlant::CompData::getPlantComponent(state, plantConnection.plantLoc).MinOutletTemp = plantConnection.LowOutTempLimit;
        }

        if (plantConnection.HowLoadServed == DataPlant::HowMet::ByNominalCapHiOutLimit) {
            DataPlant::CompData::getPlantComponent(state, plantConnection.plantLoc).MaxOutletTemp = plantConnection.HiOutTempLimit;
        }
    }
    void UserPlantComponentStruct::oneTimeInit(EnergyPlusData &state)
    {

        if (this->myOneTimeFlag) {
            // locate the connections to the plant loops
            for (int connectionIndex = 0; connectionIndex < this->NumPlantConnections; ++connectionIndex) {
                auto &plantConnection = this->Loop[connectionIndex];
                bool errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(state,
                                                        this->Name,
                                                        DataPlant::PlantEquipmentType::PlantComponentUserDefined,
                                                        plantConnection.plantLoc,
                                                        errFlag,
                                                        _,
                                                        _,
                                                        _,
                                                        plantConnection.InletNodeNum,
                                                        ObjexxFCL::Optional_int_const());
                if (errFlag) {
                    ShowFatalError(state, "InitPlantUserComponent: Program terminated due to previous condition(s).");
                }

                // set user input for flow priority
                DataPlant::CompData::getPlantComponent(state, plantConnection.plantLoc).FlowPriority = plantConnection.FlowPriority;

                // set user input for how loads served
                DataPlant::CompData::getPlantComponent(state, plantConnection.plantLoc).HowLoadServed = plantConnection.HowLoadServed;
            }

            this->myOneTimeFlag = false;
        }
    }

    void UserCoilComponentStruct::report(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Feb. 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // report model outputs

        for (int loop = 0; loop < this->NumAirConnections; ++loop) {
            auto &airConnection = this->AirConnections[loop];
            if (airConnection.OutletNodeNum > 0) {
                state.dataLoopNodes->Node(airConnection.OutletNodeNum).Temp = airConnection.OutletTemp;
                state.dataLoopNodes->Node(airConnection.OutletNodeNum).HumRat = airConnection.OutletHumRat;
                state.dataLoopNodes->Node(airConnection.OutletNodeNum).MassFlowRate = airConnection.OutletMassFlowRate;
                state.dataLoopNodes->Node(airConnection.OutletNodeNum).Enthalpy =
                    Psychrometrics::PsyHFnTdbW(airConnection.OutletTemp, airConnection.OutletHumRat);

                state.dataLoopNodes->Node(airConnection.OutletNodeNum).MassFlowRateMinAvail =
                    state.dataLoopNodes->Node(airConnection.InletNodeNum).MassFlowRateMinAvail;
                state.dataLoopNodes->Node(airConnection.OutletNodeNum).MassFlowRateMaxAvail =
                    state.dataLoopNodes->Node(airConnection.InletNodeNum).MassFlowRateMaxAvail;
            }
        }

        if (this->PlantIsConnected) {
            // make mass flow requests
            PlantUtilities::SetComponentFlowRate(
                state, this->Loop.MassFlowRateRequest, this->Loop.InletNodeNum, this->Loop.OutletNodeNum, this->Loop.plantLoc);
            PlantUtilities::SafeCopyPlantNode(state, this->Loop.InletNodeNum, this->Loop.OutletNodeNum);
            // unload Actuators to node data structure
            state.dataLoopNodes->Node(this->Loop.OutletNodeNum).Temp = this->Loop.OutletTemp;
        }

        if (this->Water.SuppliedByWaterSystem) {
            state.dataWaterData->WaterStorage(this->Water.SupplyTankID).VdotRequestDemand(this->Water.SupplyTankDemandARRID) =
                this->Water.SupplyVdotRequest;
        }

        if (this->Water.CollectsToWaterSystem) {
            state.dataWaterData->WaterStorage(this->Water.CollectionTankID).VdotAvailSupply(this->Water.CollectionTankSupplyARRID) =
                this->Water.CollectedVdot;
        }
    }

    void UserZoneHVACForcedAirComponentStruct::report(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   Feb. 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // report model outputs

        state.dataLoopNodes->Node(this->AirConnection.InletNodeNum).MassFlowRate = this->AirConnection.InletMassFlowRate;

        state.dataLoopNodes->Node(this->AirConnection.OutletNodeNum).Temp = this->AirConnection.OutletTemp;
        state.dataLoopNodes->Node(this->AirConnection.OutletNodeNum).HumRat = this->AirConnection.OutletHumRat;
        state.dataLoopNodes->Node(this->AirConnection.OutletNodeNum).MassFlowRate = this->AirConnection.OutletMassFlowRate;
        state.dataLoopNodes->Node(this->AirConnection.OutletNodeNum).Enthalpy =
            Psychrometrics::PsyHFnTdbW(this->AirConnection.OutletTemp, this->AirConnection.OutletHumRat);

        if (this->SourceAir.OutletNodeNum > 0) {
            state.dataLoopNodes->Node(this->SourceAir.OutletNodeNum).Temp = this->SourceAir.OutletTemp;
            state.dataLoopNodes->Node(this->SourceAir.OutletNodeNum).HumRat = this->SourceAir.OutletHumRat;
            state.dataLoopNodes->Node(this->SourceAir.OutletNodeNum).MassFlowRate = this->SourceAir.OutletMassFlowRate;
            state.dataLoopNodes->Node(this->SourceAir.OutletNodeNum).Enthalpy =
                Psychrometrics::PsyHFnTdbW(this->SourceAir.OutletTemp, this->SourceAir.OutletHumRat);
        }

        if (this->NumPlantConnections > 0) {
            for (int loop = 0; loop < this->NumPlantConnections; ++loop) {
                auto &plantConnection = this->Loop[loop];
                // make mass flow requests
                PlantUtilities::SetComponentFlowRate(state,
                                                     plantConnection.MassFlowRateRequest,
                                                     plantConnection.InletNodeNum,
                                                     plantConnection.OutletNodeNum,
                                                     plantConnection.plantLoc);
                PlantUtilities::SafeCopyPlantNode(state, plantConnection.InletNodeNum, plantConnection.OutletNodeNum);
                // unload Actuators to node data structure
                state.dataLoopNodes->Node(plantConnection.OutletNodeNum).Temp = plantConnection.OutletTemp;
            }
        }

        if (this->Water.SuppliedByWaterSystem) {
            state.dataWaterData->WaterStorage(this->Water.SupplyTankID).VdotRequestDemand(this->Water.SupplyTankDemandARRID) =
                this->Water.SupplyVdotRequest;
        }

        if (this->Water.CollectsToWaterSystem) {
            state.dataWaterData->WaterStorage(this->Water.CollectionTankID).VdotAvailSupply(this->Water.CollectionTankSupplyARRID) =
                this->Water.CollectedVdot;
        }
    }

    void UserAirTerminalComponentStruct::report(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith
        //       DATE WRITTEN   March 2012
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        state.dataLoopNodes->Node(this->AirConnection.InletNodeNum).MassFlowRate = this->AirConnection.InletMassFlowRate;

        state.dataLoopNodes->Node(this->AirConnection.OutletNodeNum).Temp = this->AirConnection.OutletTemp;
        state.dataLoopNodes->Node(this->AirConnection.OutletNodeNum).HumRat = this->AirConnection.OutletHumRat;
        state.dataLoopNodes->Node(this->AirConnection.OutletNodeNum).MassFlowRate = this->AirConnection.OutletMassFlowRate;
        state.dataLoopNodes->Node(this->AirConnection.OutletNodeNum).Enthalpy =
            Psychrometrics::PsyHFnTdbW(this->AirConnection.OutletTemp, this->AirConnection.OutletHumRat);
        if (this->SourceAir.OutletNodeNum > 0) {
            state.dataLoopNodes->Node(this->SourceAir.OutletNodeNum).Temp = this->SourceAir.OutletTemp;
            state.dataLoopNodes->Node(this->SourceAir.OutletNodeNum).HumRat = this->SourceAir.OutletHumRat;
            state.dataLoopNodes->Node(this->SourceAir.OutletNodeNum).MassFlowRate = this->SourceAir.OutletMassFlowRate;
            state.dataLoopNodes->Node(this->SourceAir.OutletNodeNum).Enthalpy =
                Psychrometrics::PsyHFnTdbW(this->SourceAir.OutletTemp, this->SourceAir.OutletHumRat);
        }

        if (this->NumPlantConnections > 0) {
            for (int loop = 0; loop < this->NumPlantConnections; ++loop) {
                auto &plantConnection = this->Loop[loop];
                // make mass flow requests
                PlantUtilities::SetComponentFlowRate(state,
                                                     plantConnection.MassFlowRateRequest,
                                                     plantConnection.InletNodeNum,
                                                     plantConnection.OutletNodeNum,
                                                     plantConnection.plantLoc);
                PlantUtilities::SafeCopyPlantNode(state, plantConnection.InletNodeNum, plantConnection.OutletNodeNum);
                // unload Actuators to node data structure
                state.dataLoopNodes->Node(plantConnection.OutletNodeNum).Temp = plantConnection.OutletTemp;
            }
        }

        if (this->Water.SuppliedByWaterSystem) {
            state.dataWaterData->WaterStorage(this->Water.SupplyTankID).VdotRequestDemand(this->Water.SupplyTankDemandARRID) =
                this->Water.SupplyVdotRequest;
        }

        if (this->Water.CollectsToWaterSystem) {
            state.dataWaterData->WaterStorage(this->Water.CollectionTankID).VdotAvailSupply(this->Water.CollectionTankSupplyARRID) =
                this->Water.CollectedVdot;
        }
    }

    void GetUserDefinedCoilIndex(
        EnergyPlusData &state, std::string const &CoilName, int &CoilIndex, bool &ErrorsFound, std::string const &CurrentModuleObject)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   August 2013
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine sets an index for a given user defined Cooling Coil -- issues error message if that
        // coil is not a legal user defined Cooling Coil.

        // Obtains and allocates TESCoil related parameters from input file
        if (state.dataUserDefinedComponents->GetInput) { // First time subroutine has been called, get input data
            GetUserDefinedComponents(state);
            state.dataUserDefinedComponents->GetInput = false; // Set logic flag to disallow getting the input data on future calls to this subroutine
        }

        if (state.dataUserDefinedComponents->NumUserCoils > 0) {
            CoilIndex = Util::FindItem(CoilName, state.dataUserDefinedComponents->UserCoil, state.dataUserDefinedComponents->NumUserCoils);
        } else {
            CoilIndex = 0;
        }

        if (CoilIndex == 0) {
            ShowSevereError(state, std::format("{}, GetUserDefinedCoilIndex: User Defined Cooling Coil not found={}", CurrentModuleObject, CoilName));
            ErrorsFound = true;
        }
    }

    void GetUserDefinedCoilAirInletNode(
        EnergyPlusData &state, std::string const &CoilName, int &CoilAirInletNode, bool &ErrorsFound, std::string const &CurrentModuleObject)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2015
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets a given user defined Cooling Coil's air inlet node -- issues error message if that
        // coil is not a legal user defined Cooling Coil and sets air node to 0, otherwise, returns inlet air node number.

        int CoilIndex;

        // Obtains and allocates TESCoil related parameters from input file
        if (state.dataUserDefinedComponents->GetInput) { // First time subroutine has been called, get input data
            GetUserDefinedComponents(state);
            state.dataUserDefinedComponents->GetInput = false; // Set logic flag to disallow getting the input data on future calls to this subroutine
        }

        if (state.dataUserDefinedComponents->NumUserCoils > 0) {
            CoilIndex = Util::FindItem(CoilName, state.dataUserDefinedComponents->UserCoil, state.dataUserDefinedComponents->NumUserCoils);
        } else {
            CoilIndex = 0;
        }

        if (CoilIndex == 0) {
            ShowSevereError(state, std::format("{}, GetTESCoilIndex: TES Cooling Coil not found={}", CurrentModuleObject, CoilName));
            ErrorsFound = true;
            CoilAirInletNode = 0;
        } else {
            CoilAirInletNode = state.dataUserDefinedComponents->UserCoil(CoilIndex).AirConnections[primaryConnIdx].InletNodeNum;
        }
    }

    void GetUserDefinedCoilAirOutletNode(
        EnergyPlusData &state, std::string const &CoilName, int &CoilAirOutletNode, bool &ErrorsFound, std::string const &CurrentModuleObject)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2015
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine gets a given user defined Cooling Coil's air outlet node -- issues error message if that
        // coil is not a legal user defined Cooling Coil and sets air node to 0, otherwise, returns outlet air node number.

        int CoilIndex;

        // Obtains and allocates TESCoil related parameters from input file
        if (state.dataUserDefinedComponents->GetInput) { // First time subroutine has been called, get input data
            GetUserDefinedComponents(state);
            state.dataUserDefinedComponents->GetInput = false; // Set logic flag to disallow getting the input data on future calls to this subroutine
        }

        if (state.dataUserDefinedComponents->NumUserCoils > 0) {
            CoilIndex = Util::FindItem(CoilName, state.dataUserDefinedComponents->UserCoil, state.dataUserDefinedComponents->NumUserCoils);
        } else {
            CoilIndex = 0;
        }

        if (CoilIndex == 0) {
            ShowSevereError(state, std::format("{}, GetTESCoilIndex: TES Cooling Coil not found={}", CurrentModuleObject, CoilName));
            ErrorsFound = true;
            CoilAirOutletNode = 0;
        } else {
            CoilAirOutletNode = state.dataUserDefinedComponents->UserCoil(CoilIndex).AirConnections[primaryConnIdx].OutletNodeNum;
        }
    }

} // namespace UserDefinedComponents

} // namespace EnergyPlus

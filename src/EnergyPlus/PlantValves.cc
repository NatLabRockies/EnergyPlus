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

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>

// EnergyPlus Headers
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataBranchAirLoopPlant.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/PlantValves.hh>
#include <EnergyPlus/UtilityRoutines.hh>

namespace EnergyPlus {

namespace PlantValves {

    // Module containing the routines dealing with the <module_name>

    // MODULE INFORMATION:
    //       AUTHOR         B. Griffith
    //       DATE WRITTEN   Jan, 2006
    //       MODIFIED       Nov 2010, B. Griffith, plant upgrades
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // Collect "valve" type models for Plant loops

    using namespace DataLoopNode;

    PlantComponent *TemperValveData::factory(EnergyPlusData &state, std::string objectName)
    {
        // Process the input data for valves if it hasn't been done already
        if (state.dataPlantValves->GetTemperingValves) {
            GetPlantValvesInput(state);
            state.dataPlantValves->GetTemperingValves = false;
        }
        // Now look for this particular pipe in the list
        for (auto &valve : state.dataPlantValves->TemperValve) {
            if (valve.Name == objectName) {
                return &valve;
            }
        }
        // If we didn't find it, fatal
        ShowFatalError(state, format("TemperValveDataFactory: Error getting inputs for valve named: {}", objectName)); // LCOV_EXCL_LINE
        // Shut up the compiler
        return nullptr; // LCOV_EXCL_LINE
    }

    void TemperValveData::simulate(EnergyPlusData &state,
                                   [[maybe_unused]] const PlantLocation &calledFromLocation,
                                   [[maybe_unused]] bool FirstHVACIteration,
                                   [[maybe_unused]] Real64 &CurLoad,
                                   [[maybe_unused]] bool RunFlag)
    {
        this->initialize(state);
        this->calculate(state);
        PlantUtilities::SafeCopyPlantNode(state, this->PltInletNodeNum, this->PltOutletNodeNum);
        Real64 mdot = this->MixedMassFlowRate * this->FlowDivFract;
        if (this->plantLoc.loopNum > 0) {
            PlantUtilities::SetComponentFlowRate(state, mdot, this->PltInletNodeNum, this->PltOutletNodeNum, this->plantLoc);
        }
    }

    void TemperValveData::getDesignCapacities([[maybe_unused]] EnergyPlusData &state,
                                              [[maybe_unused]] const PlantLocation &calledFromLocation,
                                              Real64 &MaxLoad,
                                              Real64 &MinLoad,
                                              Real64 &OptLoad)
    {
        MaxLoad = 0.0;
        MinLoad = 0.0;
        OptLoad = 0.0;
    }

    void GetPlantValvesInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Brent Griffith
        //       DATE WRITTEN   Jan. 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // get input from user

        // METHODOLOGY EMPLOYED:
        // usual method using InputProcessor

        // Using/Aliasing
        using BranchNodeConnections::TestCompSet;
        using NodeInputManager::GetOnlySingleNode;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Item;                        // Item to be "gotten"
        Array1D_string Alphas(6);        // Alpha items for object
        Array1D<Real64> Numbers(1);      // Numeric items for object
        int NumAlphas;                   // Number of Alphas for each GetObjectItem call
        int NumNumbers;                  // Number of Numbers for each GetObjectItem call
        int IOStatus;                    // Used in GetObjectItem
        bool ErrorsFound(false);         // Set to true if errors in input, fatal at end of routine
        std::string CurrentModuleObject; // for ease in renaming.

        CurrentModuleObject = "TemperingValve";
        state.dataPlantValves->NumTemperingValves = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);

        state.dataPlantValves->TemperValve.allocate(state.dataPlantValves->NumTemperingValves);

        for (Item = 1; Item <= state.dataPlantValves->NumTemperingValves; ++Item) {

            state.dataInputProcessing->inputProcessor->getObjectItem(
                state, CurrentModuleObject, Item, Alphas, NumAlphas, Numbers, NumNumbers, IOStatus);
            //  <process, noting errors>
            state.dataPlantValves->TemperValve(Item).Name = Alphas(1);
            // Get Plant Inlet Node
            state.dataPlantValves->TemperValve(Item).PltInletNodeNum = GetOnlySingleNode(state,
                                                                                         Alphas(2),
                                                                                         ErrorsFound,
                                                                                         DataLoopNode::ConnectionObjectType::TemperingValve,
                                                                                         Alphas(1),
                                                                                         DataLoopNode::NodeFluidType::Water,
                                                                                         DataLoopNode::ConnectionType::Inlet,
                                                                                         NodeInputManager::CompFluidStream::Primary,
                                                                                         ObjectIsNotParent);
            // Get Plant Outlet Node
            state.dataPlantValves->TemperValve(Item).PltOutletNodeNum = GetOnlySingleNode(state,
                                                                                          Alphas(3),
                                                                                          ErrorsFound,
                                                                                          DataLoopNode::ConnectionObjectType::TemperingValve,
                                                                                          Alphas(1),
                                                                                          DataLoopNode::NodeFluidType::Water,
                                                                                          DataLoopNode::ConnectionType::Outlet,
                                                                                          NodeInputManager::CompFluidStream::Primary,
                                                                                          ObjectIsNotParent);

            // Get Stream 2 Source Node
            state.dataPlantValves->TemperValve(Item).PltStream2NodeNum = GetOnlySingleNode(state,
                                                                                           Alphas(4),
                                                                                           ErrorsFound,
                                                                                           DataLoopNode::ConnectionObjectType::TemperingValve,
                                                                                           Alphas(1),
                                                                                           DataLoopNode::NodeFluidType::Water,
                                                                                           DataLoopNode::ConnectionType::Sensor,
                                                                                           NodeInputManager::CompFluidStream::Primary,
                                                                                           ObjectIsNotParent);
            // Get Mixed water Setpoint
            state.dataPlantValves->TemperValve(Item).PltSetPointNodeNum = GetOnlySingleNode(state,
                                                                                            Alphas(5),
                                                                                            ErrorsFound,
                                                                                            DataLoopNode::ConnectionObjectType::TemperingValve,
                                                                                            Alphas(1),
                                                                                            DataLoopNode::NodeFluidType::Water,
                                                                                            DataLoopNode::ConnectionType::SetPoint,
                                                                                            NodeInputManager::CompFluidStream::Primary,
                                                                                            ObjectIsNotParent);

            // Get Pump outlet
            state.dataPlantValves->TemperValve(Item).PltPumpOutletNodeNum = GetOnlySingleNode(state,
                                                                                              Alphas(6),
                                                                                              ErrorsFound,
                                                                                              DataLoopNode::ConnectionObjectType::TemperingValve,
                                                                                              Alphas(1),
                                                                                              DataLoopNode::NodeFluidType::Water,
                                                                                              DataLoopNode::ConnectionType::Sensor,
                                                                                              NodeInputManager::CompFluidStream::Primary,
                                                                                              ObjectIsNotParent);

            // Note most checks on user input are made in second pass thru init routine

            TestCompSet(state, CurrentModuleObject, Alphas(1), Alphas(2), Alphas(3), "Supply Side Water Nodes");
        }

        for (Item = 1; Item <= state.dataPlantValves->NumTemperingValves; ++Item) {

            SetupOutputVariable(state,
                                "Tempering Valve Flow Fraction",
                                Constant::Units::None,
                                state.dataPlantValves->TemperValve(Item).FlowDivFract,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataPlantValves->TemperValve(Item).Name);
        }

        if (ErrorsFound) {
            ShowFatalError(state, format("GetPlantValvesInput: {} Errors found in input", CurrentModuleObject));
        }
    }

    void TemperValveData::initialize(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith, NREL
        //       DATE WRITTEN   Jan. 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // initialize data for valve modeling

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int InletNode;   // local working variable for inlet node number
        int OutletNode;  // local working variable for outlet node number
        int Strm2Node;   // local working variable for stream 2 outlet node number
        int SetPntNode;  // local working variable for setpoint node number
        int PumpOutNode; // local working variable for pump outlet node number

        bool InNodeOnSplitter; // input data check
        bool PumpOutNodeOkay;  // input data check
        bool ErrorsFound;      // input data check
        bool TwoBranchesBetwn; // input data check
        bool SetPointNodeOkay; // input data check
        bool Stream2NodeOkay;  // input data check
        bool IsBranchActive;   // input data check

        bool errFlag;

        if (state.dataPlantValves->OneTimeInitFlag) {
            state.dataPlantValves->OneTimeInitFlag = false;
        } else {
            // delay checks one pass so more of plant data structure gets filled in
            if (this->compDelayedInitFlag) {
                // do some checks on input data
                // Search thru PlantLoop Data Structure to check some things.
                // Locate the component on the plant loops for later usage
                errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(
                    state, this->Name, DataPlant::PlantEquipmentType::ValveTempering, this->plantLoc, errFlag, _, _, _, _, _);

                if (errFlag) {
                    ShowFatalError(state, "InitPlantValves: Program terminated due to previous condition(s).");
                }
                // init logical flags
                ErrorsFound = false;
                InNodeOnSplitter = false;
                PumpOutNodeOkay = false;
                TwoBranchesBetwn = false;
                SetPointNodeOkay = false;
                Stream2NodeOkay = false;
                IsBranchActive = false;

                // . A) find indexes of PlantLoop, Half loop, and Branch by searching CompData
                for (auto &thisPlantLoop : state.dataPlnt->PlantLoop) {
                    for (auto &thisLoopSide : thisPlantLoop.LoopSide) {
                        int branchCtr = 0;
                        for (auto const &thisBranch : thisLoopSide.Branch) {
                            branchCtr++;
                            for (auto const &thisComp : thisBranch.Comp) {

                                if ((thisComp.Type == DataPlant::PlantEquipmentType::ValveTempering) &&
                                    (thisComp.Name == this->Name)) { // we found it.

                                    // is branch control type 'Active'
                                    if (thisBranch.controlType == DataBranchAirLoopPlant::ControlType::Active) {
                                        IsBranchActive = true;
                                    }

                                    // is Valve inlet node an outlet node of a splitter
                                    if (thisLoopSide.Splitter.Exists) {
                                        if (allocated(thisLoopSide.Splitter.NodeNumOut)) {
                                            if (any_eq(thisLoopSide.Splitter.NodeNumOut, this->PltInletNodeNum)) {
                                                InNodeOnSplitter = true;
                                            }
                                        } // allocated

                                        // are there only 2 branches between splitter and mixer?
                                        if (thisLoopSide.Splitter.TotalOutletNodes == 2) {
                                            TwoBranchesBetwn = true;
                                        }
                                    } // has splitter

                                    // is stream 2 node an inlet to the mixer ?
                                    if (thisLoopSide.Mixer.Exists) {
                                        if (any_eq(thisLoopSide.Mixer.NodeNumIn, this->PltStream2NodeNum)) {
                                            int thisInnerBranchCtr = 0;
                                            for (auto const &thisInnerBranch : thisLoopSide.Branch) {
                                                thisInnerBranchCtr++;
                                                if (branchCtr == thisInnerBranchCtr) {
                                                    continue; // already looped into this one
                                                }
                                                for (auto const &thisInnerComp : thisInnerBranch.Comp) {
                                                    if (thisInnerComp.NodeNumOut == this->PltStream2NodeNum) {
                                                        Stream2NodeOkay = true;
                                                    }
                                                }
                                            }
                                        }
                                    } // has mixer

                                    // is pump node really the outlet of a branch with a pump?
                                    for (auto const &thisInnerBranch : thisLoopSide.Branch) {
                                        if (thisInnerBranch.NodeNumOut == this->PltPumpOutletNodeNum) {
                                            for (auto const &thisInnerComp : thisInnerBranch.Comp) {
                                                if (DataPlant::PlantEquipmentTypeIsPump[static_cast<int>(thisInnerComp.Type)]) {
                                                    PumpOutNodeOkay = true;
                                                }
                                            }
                                        }
                                    }

                                    // does sensor node agree with plant loop setpoint?
                                    if (thisPlantLoop.TempSetPointNodeNum == this->PltSetPointNodeNum) {
                                        SetPointNodeOkay = true;
                                    }

                                } // found item

                            } // comps
                        } // Branches
                    } // Loop Sides
                } // Plant loops

                if (!IsBranchActive) {
                    ShowSevereError(state, "TemperingValve object needs to be on an ACTIVE branch");
                    ErrorsFound = true;
                }

                if (!InNodeOnSplitter) {
                    ShowSevereError(state, "TemperingValve object needs to be between a Splitter and Mixer");
                    ErrorsFound = true;
                }

                if (!PumpOutNodeOkay) {
                    ShowSevereError(state, "TemperingValve object needs to reference a node that is the outlet of a pump on its loop");
                    ErrorsFound = true;
                }

                if (!TwoBranchesBetwn) {
                    ShowSevereError(state, "TemperingValve object needs exactly two branches between a Splitter and Mixer");
                    ErrorsFound = true;
                }

                if (!SetPointNodeOkay) {
                    ShowSevereError(state, "TemperingValve object setpoint node not valid.  Check Setpoint manager for Plant Loop Temp Setpoint");
                    ErrorsFound = true;
                }

                if (!Stream2NodeOkay) {
                    ShowSevereError(state, "TemperingValve object stream 2 source node not valid.");
                    ShowContinueError(state, "Check that node is a component outlet, enters a mixer, and on the other branch");
                    ErrorsFound = true;
                }
                if (ErrorsFound) {
                    ShowFatalError(state, format("Errors found in input, TemperingValve object {}", this->Name));
                }
                this->compDelayedInitFlag = false;
            } // my two time flag for input checking

        } // my one time flag for input checking

        InletNode = this->PltInletNodeNum;
        OutletNode = this->PltOutletNodeNum;
        Strm2Node = this->PltStream2NodeNum;
        SetPntNode = this->PltSetPointNodeNum;
        PumpOutNode = this->PltPumpOutletNodeNum;

        if (state.dataGlobal->BeginEnvrnFlag && this->environmentInit) {

            if ((InletNode > 0) && (OutletNode > 0)) {
                //   Node(InletNode)%Temp = 0.0
                PlantUtilities::InitComponentNodes(
                    state, 0.0, state.dataLoopNodes->Node(PumpOutNode).MassFlowRateMax, this->PltInletNodeNum, this->PltOutletNodeNum);
            }
            this->environmentInit = false;
        }

        if (!state.dataGlobal->BeginEnvrnFlag) {
            this->environmentInit = true;
        }

        if (InletNode > 0) {
            this->InletTemp = state.dataLoopNodes->Node(InletNode).Temp;
        }
        if (Strm2Node > 0) {
            this->Stream2SourceTemp = state.dataLoopNodes->Node(Strm2Node).Temp;
        }
        if (SetPntNode > 0) {
            this->SetPointTemp = state.dataLoopNodes->Node(SetPntNode).TempSetPoint;
        }
        if (PumpOutNode > 0) {
            this->MixedMassFlowRate = state.dataLoopNodes->Node(PumpOutNode).MassFlowRate;
        }
    }

    void TemperValveData::calculate(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         B. Griffith, NREL
        //       DATE WRITTEN   Jan. 2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        //  This routine does the calculations for Valves.
        //  Currently only one type of valve, for Tempering.

        // METHODOLOGY EMPLOYED:
        //   Tempering valve calculations involve computing a flow fraction
        //     that should be diverted.  See update routine for setting flow rates.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 Tin;  // local working variable for Inlet Temperature (C)
        Real64 Tset; // local working variable for Setpoint Temperature (C)
        Real64 Ts2;  // local Working Variable for Stream 2 outlet Temperature (C)

        if (state.dataGlobal->KickOffSimulation) {
            return;
        }

        if (state.dataPlnt->PlantLoop(this->plantLoc.loopNum).LoopSide(this->plantLoc.loopSideNum).FlowLock == DataPlant::FlowLock::Unlocked) {
            Tin = this->InletTemp;
            Tset = this->SetPointTemp;
            Ts2 = this->Stream2SourceTemp;

            if (Ts2 <= Tset) {
                this->FlowDivFract = 0.0;
            } else { // Divert some or all flow
                if (Tin < Ts2) {
                    this->FlowDivFract = (Ts2 - Tset) / (Ts2 - Tin);
                } else {
                    this->FlowDivFract = 1.0;
                }
            }
        } else if (state.dataPlnt->PlantLoop(this->plantLoc.loopNum).LoopSide(this->plantLoc.loopSideNum).FlowLock ==
                   DataPlant::FlowLock::Locked) { // don't recalc diversion, just reuse current flows
            if (this->MixedMassFlowRate > 0.0) {
                this->FlowDivFract = state.dataLoopNodes->Node(this->PltOutletNodeNum).MassFlowRate / this->MixedMassFlowRate;
            } else {
                this->FlowDivFract = 0.0;
            }
        }

        if (this->FlowDivFract < 0.0) {
            this->FlowDivFract = 0.0;
        }
        if (this->FlowDivFract > 1.0) {
            this->FlowDivFract = 1.0;
        }
    }
    void TemperValveData::oneTimeInit([[maybe_unused]] EnergyPlusData &state)
    {
    }
    void TemperValveData::oneTimeInit_new([[maybe_unused]] EnergyPlusData &state)
    {
    }

} // namespace PlantValves

} // namespace EnergyPlus

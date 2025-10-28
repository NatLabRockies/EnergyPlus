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
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataDefineEquip.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/HVACCooledBeam.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterCoils.hh>

namespace EnergyPlus {

namespace HVACCooledBeam {

    // Module containing routines dealing with cooled beam units

    // MODULE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   February 2, 2008
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and algorithms needed to simulate cooled beam units

    // METHODOLOGY EMPLOYED:
    // Cooled beam units are treated as terminal units. There is a fixed amount of supply air delivered
    // either directly through a diffuser or through the cooled beam units. Thermodynamically the
    // situation is similar to 4 pipe induction terminal units. The detailed methodology follows the
    // method in DOE-2.1E.

    // Using/Aliasing
    using namespace DataLoopNode;
    using HVAC::SmallAirVolFlow;
    using HVAC::SmallLoad;
    using HVAC::SmallMassFlow;
    using HVAC::SmallWaterVolFlow;
    using Psychrometrics::PsyCpAirFnW;
    using Psychrometrics::PsyHFnTdbW;
    using Psychrometrics::PsyRhoAirFnPbTdbW;

    void SimCoolBeam(EnergyPlusData &state,
                     std::string_view CompName,     // name of the cooled beam unit
                     bool const FirstHVACIteration, // TRUE if first HVAC iteration in time step
                     int const ZoneNum,             // index of zone served by the unit
                     int const ZoneNodeNum,         // zone node number of zone served by the unit
                     int &CompIndex,                // which cooled beam unit in data structure
                     Real64 &NonAirSysOutput        // convective cooling by the beam system [W]
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   Feb 3, 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Manages the simulation of a cooled beam unit.
        // Called from SimZoneAirLoopEquipment in module ZoneAirLoopEquipmentManager.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int CBNum; // index of cooled beam unit being simulated

        // First time SimIndUnit is called, get the input for all the cooled beam units
        if (state.dataHVACCooledBeam->GetInputFlag) {
            GetCoolBeams(state);
            state.dataHVACCooledBeam->GetInputFlag = false;
        }

        // Get the  unit index
        if (CompIndex == 0) {
            CBNum = Util::FindItemInList(CompName, state.dataHVACCooledBeam->CoolBeam);
            if (CBNum == 0) {
                ShowFatalError(state, format("SimCoolBeam: Cool Beam Unit not found={}", CompName));
            }
            CompIndex = CBNum;
        } else {
            CBNum = CompIndex;
            if (CBNum > state.dataHVACCooledBeam->NumCB || CBNum < 1) {
                ShowFatalError(state,
                               format("SimCoolBeam: Invalid CompIndex passed={}, Number of Cool Beam Units={}, System name={}",
                                      CompIndex,
                                      state.dataHVACCooledBeam->NumCB,
                                      CompName));
            }
            if (state.dataHVACCooledBeam->CheckEquipName(CBNum)) {
                if (CompName != state.dataHVACCooledBeam->CoolBeam(CBNum).Name) {
                    ShowFatalError(state,
                                   format("SimCoolBeam: Invalid CompIndex passed={}, Cool Beam Unit name={}, stored Cool Beam Unit for that index={}",
                                          CompIndex,
                                          CompName,
                                          state.dataHVACCooledBeam->CoolBeam(CBNum).Name));
                }
                state.dataHVACCooledBeam->CheckEquipName(CBNum) = false;
            }
        }
        if (CBNum == 0) {
            ShowFatalError(state, format("Cool Beam Unit not found = {}", CompName));
        }

        state.dataSize->CurTermUnitSizingNum =
            state.dataDefineEquipment->AirDistUnit(state.dataHVACCooledBeam->CoolBeam(CBNum).ADUNum).TermUnitSizingNum;
        // initialize the unit
        InitCoolBeam(state, CBNum, FirstHVACIteration);

        ControlCoolBeam(state, CBNum, ZoneNum, ZoneNodeNum, FirstHVACIteration, NonAirSysOutput);

        // Update the current unit's outlet nodes. No update needed
        UpdateCoolBeam(state, CBNum);

        // Fill the report variables. There are no report variables
        ReportCoolBeam(state, CBNum);
    }

    void GetCoolBeams(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   Feb 3, 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for cool beam units and stores it in the
        // cool beam unit data structures

        // METHODOLOGY EMPLOYED:
        // Uses "Get" routines to read in data.

        // Using/Aliasing
        using BranchNodeConnections::TestCompSet;
        using NodeInputManager::GetOnlySingleNode;
        using namespace DataSizing;
        using WaterCoils::GetCoilWaterInletNode;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetCoolBeams "); // include trailing blank space
        static constexpr std::string_view routineName = "GetCoolBeams";

        int CBIndex;                     // loop index
        std::string CurrentModuleObject; // for ease in getting objects
        Array1D_string Alphas;           // Alpha input items for object
        Array1D_string cAlphaFields;     // Alpha field names
        Array1D_string cNumericFields;   // Numeric field names
        Array1D<Real64> Numbers;         // Numeric input items for object
        Array1D_bool lAlphaBlanks;       // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;     // Logical array, numeric field input BLANK = .TRUE.
        int NumAlphas(0);                // Number of Alphas for each GetObjectItem call
        int NumNumbers(0);               // Number of Numbers for each GetObjectItem call
        int TotalArgs(0);                // Total number of alpha and numeric arguments (max) for a
        //  certain object in the input file
        int IOStatus;            // Used in GetObjectItem
        bool ErrorsFound(false); // Set to true if errors in input, fatal at end of routine
        int CtrlZone;            // controlled zome do loop index
        int SupAirIn;            // controlled zone supply air inlet index
        bool AirNodeFound;
        int ADUNum;

        auto &CoolBeam = state.dataHVACCooledBeam->CoolBeam;
        auto &CheckEquipName = state.dataHVACCooledBeam->CheckEquipName;

        // find the number of cooled beam units
        CurrentModuleObject = "AirTerminal:SingleDuct:ConstantVolume:CooledBeam";
        // Update Num in state and make local convenience copy
        int NumCB = state.dataHVACCooledBeam->NumCB = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        // allocate the data structures
        CoolBeam.allocate(NumCB);
        CheckEquipName.dimension(NumCB, true);

        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, TotalArgs, NumAlphas, NumNumbers);
        NumAlphas = 7;
        NumNumbers = 16;
        TotalArgs = 23;

        Alphas.allocate(NumAlphas);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNumbers);
        Numbers.dimension(NumNumbers, 0.0);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNumbers, true);

        // loop over cooled beam units; get and load the input data
        for (CBIndex = 1; CBIndex <= NumCB; ++CBIndex) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CBIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};
            int CBNum = CBIndex;

            CoolBeam(CBNum).Name = Alphas(1);
            CoolBeam(CBNum).UnitType = CurrentModuleObject;
            CoolBeam(CBNum).UnitType_Num = 1;
            CoolBeam(CBNum).CBTypeString = Alphas(3);
            if (Util::SameString(CoolBeam(CBNum).CBTypeString, "Passive")) {
                CoolBeam(CBNum).CBType = CooledBeamType::Passive;
            } else if (Util::SameString(CoolBeam(CBNum).CBTypeString, "Active")) {
                CoolBeam(CBNum).CBType = CooledBeamType::Active;
            } else {
                ShowSevereError(state, format("Illegal {} = {}.", cAlphaFields(3), CoolBeam(CBNum).CBTypeString));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, CoolBeam(CBNum).Name));
                ErrorsFound = true;
            }

            if (lAlphaBlanks(2)) {
                CoolBeam(CBNum).availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((CoolBeam(CBNum).availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) { // convert schedule name to pointer
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ErrorsFound = true;
            }
            CoolBeam(CBNum).AirInNode = GetOnlySingleNode(state,
                                                          Alphas(4),
                                                          ErrorsFound,
                                                          DataLoopNode::ConnectionObjectType::AirTerminalSingleDuctConstantVolumeCooledBeam,
                                                          Alphas(1),
                                                          DataLoopNode::NodeFluidType::Air,
                                                          DataLoopNode::ConnectionType::Inlet,
                                                          NodeInputManager::CompFluidStream::Primary,
                                                          ObjectIsNotParent,
                                                          cAlphaFields(4));
            CoolBeam(CBNum).AirOutNode = GetOnlySingleNode(state,
                                                           Alphas(5),
                                                           ErrorsFound,
                                                           DataLoopNode::ConnectionObjectType::AirTerminalSingleDuctConstantVolumeCooledBeam,
                                                           Alphas(1),
                                                           DataLoopNode::NodeFluidType::Air,
                                                           DataLoopNode::ConnectionType::Outlet,
                                                           NodeInputManager::CompFluidStream::Primary,
                                                           ObjectIsNotParent,
                                                           cAlphaFields(5));
            CoolBeam(CBNum).CWInNode = GetOnlySingleNode(state,
                                                         Alphas(6),
                                                         ErrorsFound,
                                                         DataLoopNode::ConnectionObjectType::AirTerminalSingleDuctConstantVolumeCooledBeam,
                                                         Alphas(1),
                                                         DataLoopNode::NodeFluidType::Water,
                                                         DataLoopNode::ConnectionType::Inlet,
                                                         NodeInputManager::CompFluidStream::Secondary,
                                                         ObjectIsNotParent,
                                                         cAlphaFields(6));
            CoolBeam(CBNum).CWOutNode = GetOnlySingleNode(state,
                                                          Alphas(7),
                                                          ErrorsFound,
                                                          DataLoopNode::ConnectionObjectType::AirTerminalSingleDuctConstantVolumeCooledBeam,
                                                          Alphas(1),
                                                          DataLoopNode::NodeFluidType::Water,
                                                          DataLoopNode::ConnectionType::Outlet,
                                                          NodeInputManager::CompFluidStream::Secondary,
                                                          ObjectIsNotParent,
                                                          cAlphaFields(7));
            CoolBeam(CBNum).MaxAirVolFlow = Numbers(1);
            CoolBeam(CBNum).MaxCoolWaterVolFlow = Numbers(2);
            CoolBeam(CBNum).NumBeams = Numbers(3);
            CoolBeam(CBNum).BeamLength = Numbers(4);
            CoolBeam(CBNum).DesInletWaterTemp = Numbers(5);
            CoolBeam(CBNum).DesOutletWaterTemp = Numbers(6);
            CoolBeam(CBNum).CoilArea = Numbers(7);
            CoolBeam(CBNum).a = Numbers(8);
            CoolBeam(CBNum).n1 = Numbers(9);
            CoolBeam(CBNum).n2 = Numbers(10);
            CoolBeam(CBNum).n3 = Numbers(11);
            CoolBeam(CBNum).a0 = Numbers(12);
            CoolBeam(CBNum).K1 = Numbers(13);
            CoolBeam(CBNum).n = Numbers(14);
            CoolBeam(CBNum).Kin = Numbers(15);
            CoolBeam(CBNum).InDiam = Numbers(16);

            // Register component set data
            TestCompSet(state,
                        CurrentModuleObject,
                        CoolBeam(CBNum).Name,
                        state.dataLoopNodes->NodeID(CoolBeam(CBNum).AirInNode),
                        state.dataLoopNodes->NodeID(CoolBeam(CBNum).AirOutNode),
                        "Air Nodes");
            TestCompSet(state,
                        CurrentModuleObject,
                        CoolBeam(CBNum).Name,
                        state.dataLoopNodes->NodeID(CoolBeam(CBNum).CWInNode),
                        state.dataLoopNodes->NodeID(CoolBeam(CBNum).CWOutNode),
                        "Water Nodes");

            // Setup the Cooled Beam reporting variables
            // CurrentModuleObject = "AirTerminal:SingleDuct:ConstantVolume:CooledBeam"
            SetupOutputVariable(state,
                                "Zone Air Terminal Beam Sensible Cooling Energy",
                                Constant::Units::J,
                                CoolBeam(CBNum).BeamCoolingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                CoolBeam(CBNum).Name,
                                Constant::eResource::EnergyTransfer,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::CoolingCoils);
            SetupOutputVariable(state,
                                "Zone Air Terminal Beam Chilled Water Energy",
                                Constant::Units::J,
                                CoolBeam(CBNum).BeamCoolingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                CoolBeam(CBNum).Name,
                                Constant::eResource::PlantLoopCoolingDemand,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::CoolingCoils);
            SetupOutputVariable(state,
                                "Zone Air Terminal Beam Sensible Cooling Rate",
                                Constant::Units::W,
                                CoolBeam(CBNum).BeamCoolingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                CoolBeam(CBNum).Name);
            SetupOutputVariable(state,
                                "Zone Air Terminal Supply Air Sensible Cooling Energy",
                                Constant::Units::J,
                                CoolBeam(CBNum).SupAirCoolingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                CoolBeam(CBNum).Name);
            SetupOutputVariable(state,
                                "Zone Air Terminal Supply Air Sensible Cooling Rate",
                                Constant::Units::W,
                                CoolBeam(CBNum).SupAirCoolingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                CoolBeam(CBNum).Name);
            SetupOutputVariable(state,
                                "Zone Air Terminal Supply Air Sensible Heating Energy",
                                Constant::Units::J,
                                CoolBeam(CBNum).SupAirHeatingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                CoolBeam(CBNum).Name);
            SetupOutputVariable(state,
                                "Zone Air Terminal Supply Air Sensible Heating Rate",
                                Constant::Units::W,
                                CoolBeam(CBNum).SupAirHeatingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                CoolBeam(CBNum).Name);

            SetupOutputVariable(state,
                                "Zone Air Terminal Outdoor Air Volume Flow Rate",
                                Constant::Units::m3_s,
                                CoolBeam(CBNum).OutdoorAirFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                CoolBeam(CBNum).Name);

            for (ADUNum = 1; ADUNum <= (int)state.dataDefineEquipment->AirDistUnit.size(); ++ADUNum) {
                if (CoolBeam(CBNum).AirOutNode == state.dataDefineEquipment->AirDistUnit(ADUNum).OutletNodeNum) {
                    CoolBeam(CBNum).ADUNum = ADUNum;
                    state.dataDefineEquipment->AirDistUnit(ADUNum).InletNodeNum = CoolBeam(CBNum).AirInNode;
                }
            }
            // one assumes if there isn't one assigned, it's an error?
            if (CoolBeam(CBNum).ADUNum == 0) {
                ShowSevereError(
                    state,
                    format("{}No matching Air Distribution Unit, for Unit = [{},{}].", RoutineName, CurrentModuleObject, CoolBeam(CBNum).Name));
                ShowContinueError(state, format("...should have outlet node={}", state.dataLoopNodes->NodeID(CoolBeam(CBNum).AirOutNode)));
                ErrorsFound = true;
            } else {

                // Fill the Zone Equipment data with the supply air inlet node number of this unit.
                AirNodeFound = false;
                for (CtrlZone = 1; CtrlZone <= state.dataGlobal->NumOfZones; ++CtrlZone) {
                    if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZone).IsControlled) {
                        continue;
                    }
                    for (SupAirIn = 1; SupAirIn <= state.dataZoneEquip->ZoneEquipConfig(CtrlZone).NumInletNodes; ++SupAirIn) {
                        if (CoolBeam(CBNum).AirOutNode == state.dataZoneEquip->ZoneEquipConfig(CtrlZone).InletNode(SupAirIn)) {
                            state.dataZoneEquip->ZoneEquipConfig(CtrlZone).AirDistUnitCool(SupAirIn).InNode = CoolBeam(CBNum).AirInNode;
                            state.dataZoneEquip->ZoneEquipConfig(CtrlZone).AirDistUnitCool(SupAirIn).OutNode = CoolBeam(CBNum).AirOutNode;
                            state.dataDefineEquipment->AirDistUnit(CoolBeam(CBNum).ADUNum).TermUnitSizingNum =
                                state.dataZoneEquip->ZoneEquipConfig(CtrlZone).AirDistUnitCool(SupAirIn).TermUnitSizingIndex;
                            state.dataDefineEquipment->AirDistUnit(CoolBeam(CBNum).ADUNum).ZoneEqNum = CtrlZone;
                            CoolBeam(CBNum).CtrlZoneNum = CtrlZone;
                            CoolBeam(CBNum).ctrlZoneInNodeIndex = SupAirIn;
                            AirNodeFound = true;
                            break;
                        }
                    }
                }
            }
            if (!AirNodeFound) {
                ShowSevereError(state, format("The outlet air node from the {} = {}", CurrentModuleObject, CoolBeam(CBNum).Name));
                ShowContinueError(state, format("did not have a matching Zone Equipment Inlet Node, Node ={}", Alphas(5)));
                ErrorsFound = true;
            }
        }

        Alphas.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        Numbers.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        if (ErrorsFound) {
            ShowFatalError(state, format("{}Errors found in getting input. Preceding conditions cause termination.", RoutineName));
        }
    }

    void InitCoolBeam(EnergyPlusData &state,
                      int const CBNum,              // number of the current cooled beam unit being simulated
                      bool const FirstHVACIteration // TRUE if first air loop solution this HVAC step
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   February 6, 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initialization of the cooled beam units

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        // Using/Aliasing
        using DataZoneEquipment::CheckZoneEquipmentList;
        using PlantUtilities::InitComponentNodes;
        using PlantUtilities::ScanPlantLoopsForObject;
        using PlantUtilities::SetComponentFlowRate;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("InitCoolBeam");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int InAirNode;    // supply air inlet node number
        int OutAirNode;   // unit air outlet node
        int InWaterNode;  // unit inlet chilled water node
        int OutWaterNode; // unit outlet chilled water node
        Real64 RhoAir;    // air density at outside pressure and standard temperature and humidity
        Real64 rho;       // local fluid density

        auto &coolBeam = state.dataHVACCooledBeam->CoolBeam(CBNum);
        auto &ZoneEquipmentListChecked = state.dataHVACCooledBeam->ZoneEquipmentListChecked;
        int NumCB = state.dataHVACCooledBeam->NumCB;

        if (coolBeam.PlantLoopScanFlag && allocated(state.dataPlnt->PlantLoop)) {
            bool errFlag = false;
            ScanPlantLoopsForObject(
                state, coolBeam.Name, DataPlant::PlantEquipmentType::CooledBeamAirTerminal, coolBeam.CWPlantLoc, errFlag, _, _, _, _, _);
            if (errFlag) {
                ShowFatalError(state, "InitCoolBeam: Program terminated for previous conditions.");
            }
            coolBeam.PlantLoopScanFlag = false;
        }

        if (!ZoneEquipmentListChecked && state.dataZoneEquip->ZoneEquipInputsFilled) {
            std::string CurrentModuleObject = "AirTerminal:SingleDuct:ConstantVolume:CooledBeam";
            ZoneEquipmentListChecked = true;
            // Check to see if there is a Air Distribution Unit on the Zone Equipment List
            for (int Loop = 1; Loop <= NumCB; ++Loop) {
                if (coolBeam.ADUNum == 0) {
                    continue;
                }
                if (CheckZoneEquipmentList(state, "ZONEHVAC:AIRDISTRIBUTIONUNIT", state.dataDefineEquipment->AirDistUnit(coolBeam.ADUNum).Name)) {
                    continue;
                }
                ShowSevereError(state,
                                format("InitCoolBeam: ADU=[Air Distribution Unit,{}] is not on any ZoneHVAC:EquipmentList.",
                                       state.dataDefineEquipment->AirDistUnit(coolBeam.ADUNum).Name));
                ShowContinueError(state, format("...Unit=[{},{}] will not be simulated.", CurrentModuleObject, coolBeam.Name));
            }
        }

        if (!state.dataGlobal->SysSizingCalc && coolBeam.MySizeFlag && !coolBeam.PlantLoopScanFlag) {

            SizeCoolBeam(state, CBNum);

            InWaterNode = coolBeam.CWInNode;
            OutWaterNode = coolBeam.CWOutNode;
            rho = state.dataPlnt->PlantLoop(coolBeam.CWPlantLoc.loopNum).glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
            coolBeam.MaxCoolWaterMassFlow = rho * coolBeam.MaxCoolWaterVolFlow;
            InitComponentNodes(state, 0.0, coolBeam.MaxCoolWaterMassFlow, InWaterNode, OutWaterNode);
            coolBeam.MySizeFlag = false;
        }

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && coolBeam.MyEnvrnFlag) {
            RhoAir = state.dataEnvrn->StdRhoAir;
            InAirNode = coolBeam.AirInNode;
            OutAirNode = coolBeam.AirOutNode;
            // set the mass flow rates from the input volume flow rates
            coolBeam.MaxAirMassFlow = RhoAir * coolBeam.MaxAirVolFlow;
            state.dataLoopNodes->Node(InAirNode).MassFlowRateMax = coolBeam.MaxAirMassFlow;
            state.dataLoopNodes->Node(OutAirNode).MassFlowRateMax = coolBeam.MaxAirMassFlow;
            state.dataLoopNodes->Node(InAirNode).MassFlowRateMin = 0.0;
            state.dataLoopNodes->Node(OutAirNode).MassFlowRateMin = 0.0;

            InWaterNode = coolBeam.CWInNode;
            OutWaterNode = coolBeam.CWOutNode;
            InitComponentNodes(state, 0.0, coolBeam.MaxCoolWaterMassFlow, InWaterNode, OutWaterNode);

            if (coolBeam.AirLoopNum == 0) { // fill air loop index
                if (coolBeam.CtrlZoneNum > 0 && coolBeam.ctrlZoneInNodeIndex > 0) {
                    coolBeam.AirLoopNum =
                        state.dataZoneEquip->ZoneEquipConfig(coolBeam.CtrlZoneNum).InletNodeAirLoopNum(coolBeam.ctrlZoneInNodeIndex);
                    state.dataDefineEquipment->AirDistUnit(coolBeam.ADUNum).AirLoopNum = coolBeam.AirLoopNum;
                }
            }

            coolBeam.MyEnvrnFlag = false;
        } // end one time inits

        if (!state.dataGlobal->BeginEnvrnFlag) {
            coolBeam.MyEnvrnFlag = true;
        }

        InAirNode = coolBeam.AirInNode;
        OutAirNode = coolBeam.AirOutNode;

        // Do the start of HVAC time step initializations
        if (FirstHVACIteration) {
            // check for upstream zero flow. If nonzero and schedule ON, set primary flow to max
            if (coolBeam.availSched->getCurrentVal() > 0.0 && state.dataLoopNodes->Node(InAirNode).MassFlowRate > 0.0) {
                state.dataLoopNodes->Node(InAirNode).MassFlowRate = coolBeam.MaxAirMassFlow;
            } else {
                state.dataLoopNodes->Node(InAirNode).MassFlowRate = 0.0;
            }
            // reset the max and min avail flows
            if (coolBeam.availSched->getCurrentVal() > 0.0 && state.dataLoopNodes->Node(InAirNode).MassFlowRateMaxAvail > 0.0) {
                state.dataLoopNodes->Node(InAirNode).MassFlowRateMaxAvail = coolBeam.MaxAirMassFlow;
                state.dataLoopNodes->Node(InAirNode).MassFlowRateMinAvail = coolBeam.MaxAirMassFlow;
            } else {
                state.dataLoopNodes->Node(InAirNode).MassFlowRateMaxAvail = 0.0;
                state.dataLoopNodes->Node(InAirNode).MassFlowRateMinAvail = 0.0;
            }
        }

        // do these initializations every time step
        InWaterNode = coolBeam.CWInNode;
        coolBeam.TWIn = state.dataLoopNodes->Node(InWaterNode).Temp;
        coolBeam.SupAirCoolingRate = 0.0;
        coolBeam.SupAirHeatingRate = 0.0;
    }

    void SizeCoolBeam(EnergyPlusData &state, int const CBNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   February 10, 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing cooled beam units for which flow rates have not been
        // specified in the input

        // METHODOLOGY EMPLOYED:
        // Accesses zone sizing array for air flow rates and zone and plant sizing arrays to
        // calculate coil water flow rates.

        // Using/Aliasing
        using namespace DataSizing;
        using PlantUtilities::MyPlantSizingIndex;
        using PlantUtilities::RegisterPlantCompDesignFlow;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        static constexpr std::string_view RoutineName("SizeCoolBeam");
        int PltSizCoolNum(0);          // index of plant sizing object for the cooling loop
        int NumBeams(0);               // number of beams in the zone
        Real64 DesCoilLoad(0.0);       // total cooling capacity of the beams in the zone [W]
        Real64 DesLoadPerBeam(0.0);    // cooling capacity per individual beam [W]
        Real64 DesAirVolFlow(0.0);     // design total supply air flow rate [m3/s]
        Real64 DesAirFlowPerBeam(0.0); // design supply air volumetric flow per beam [m3/s]
        Real64 RhoAir(0.0);
        Real64 CpAir(0.0);
        Real64 WaterVel(0.0);            // design water velocity in beam
        Real64 IndAirFlowPerBeamL(0.0);  // induced volumetric air flow rate per beam length [m3/s-m]
        Real64 DT(0.0);                  // air - water delta T [C]
        Real64 LengthX(0.0);             // test value for beam length [m]
        Real64 Length(0.0);              // beam length [m]
        Real64 ConvFlow(0.0);            // convective and induced air mass flow rate across beam per beam plan area [kg/s-m2]
        Real64 K(0.0);                   // coil (beam) heat transfer coefficient [W/m2-K]
        Real64 WaterVolFlowPerBeam(0.0); // Cooling water volumetric flow per beam [m3]
        bool ErrorsFound;
        Real64 rho; // local fluid density
        Real64 Cp;  // local fluid specific heat

        PltSizCoolNum = 0;
        DesAirVolFlow = 0.0;
        CpAir = 0.0;
        RhoAir = state.dataEnvrn->StdRhoAir;
        ErrorsFound = false;

        auto &coolBeam = state.dataHVACCooledBeam->CoolBeam(CBNum);

        // find the appropriate Plant Sizing object
        if (coolBeam.MaxAirVolFlow == AutoSize || coolBeam.BeamLength == AutoSize) {
            PltSizCoolNum = MyPlantSizingIndex(state, "cooled beam unit", coolBeam.Name, coolBeam.CWInNode, coolBeam.CWOutNode, ErrorsFound);
        }

        if (coolBeam.Kin == Constant::AutoCalculate) {
            if (coolBeam.CBType == CooledBeamType::Passive) {
                coolBeam.Kin = 0.0;
            } else {
                coolBeam.Kin = 2.0;
            }
            BaseSizer::reportSizerOutput(state, coolBeam.UnitType, coolBeam.Name, "Coefficient of Induction Kin", coolBeam.Kin);
        }

        if (coolBeam.MaxAirVolFlow == AutoSize) {

            if (state.dataSize->CurTermUnitSizingNum > 0) {

                CheckZoneSizing(state, coolBeam.UnitType, coolBeam.Name);
                coolBeam.MaxAirVolFlow = max(state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesCoolVolFlow,
                                             state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesHeatVolFlow);
                if (coolBeam.MaxAirVolFlow < SmallAirVolFlow) {
                    coolBeam.MaxAirVolFlow = 0.0;
                }
                BaseSizer::reportSizerOutput(state, coolBeam.UnitType, coolBeam.Name, "Supply Air Flow Rate [m3/s]", coolBeam.MaxAirVolFlow);
            }
        }

        if (coolBeam.MaxCoolWaterVolFlow == AutoSize) {

            if ((state.dataSize->CurZoneEqNum > 0) && (state.dataSize->CurTermUnitSizingNum > 0)) {

                CheckZoneSizing(state, coolBeam.UnitType, coolBeam.Name);

                if (PltSizCoolNum > 0) {

                    if (state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesCoolMassFlow >= SmallAirVolFlow) {
                        DesAirVolFlow = coolBeam.MaxAirVolFlow;
                        CpAir = PsyCpAirFnW(state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).CoolDesHumRat);
                        // the design cooling coil load is the zone load minus whatever the central system does. Note that
                        // DesCoolCoilInTempTU is really the primary air inlet temperature for the unit.
                        if (state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).ZoneTempAtCoolPeak > 0.0) {
                            DesCoilLoad = state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).NonAirSysDesCoolLoad -
                                          CpAir * RhoAir * DesAirVolFlow *
                                              (state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).ZoneTempAtCoolPeak -
                                               state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesCoolCoilInTempTU);
                        } else {
                            DesCoilLoad = CpAir * RhoAir * DesAirVolFlow *
                                          (state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).DesCoolCoilInTempTU -
                                           state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).ZoneSizThermSetPtHi);
                        }

                        rho = state.dataPlnt->PlantLoop(coolBeam.CWPlantLoc.loopNum).glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);

                        Cp = state.dataPlnt->PlantLoop(coolBeam.CWPlantLoc.loopNum)
                                 .glycol->getSpecificHeat(state, Constant::CWInitConvTemp, RoutineName);

                        coolBeam.MaxCoolWaterVolFlow = DesCoilLoad / ((coolBeam.DesOutletWaterTemp - coolBeam.DesInletWaterTemp) * Cp * rho);
                        coolBeam.MaxCoolWaterVolFlow = max(coolBeam.MaxCoolWaterVolFlow, 0.0);
                        if (coolBeam.MaxCoolWaterVolFlow < SmallWaterVolFlow) {
                            coolBeam.MaxCoolWaterVolFlow = 0.0;
                        }
                    } else {
                        coolBeam.MaxCoolWaterVolFlow = 0.0;
                    }

                    BaseSizer::reportSizerOutput(
                        state, coolBeam.UnitType, coolBeam.Name, "Maximum Total Chilled Water Flow Rate [m3/s]", coolBeam.MaxCoolWaterVolFlow);
                } else {
                    ShowSevereError(state, "Autosizing of water flow requires a cooling loop Sizing:Plant object");
                    ShowContinueError(state, format("Occurs in{} Object={}", coolBeam.UnitType, coolBeam.Name));
                    ErrorsFound = true;
                }
            }
        }

        BaseSizer::calcCoilWaterFlowRates(state,
                                          coolBeam.Name,
                                          coolBeam.UnitType,
                                          coolBeam.MaxCoolWaterVolFlow,
                                          coolBeam.CWPlantLoc.loopNum,
                                          state.dataSize->CurZoneEqNum,
                                          state.dataSize->CurSysNum,
                                          state.dataSize->CurOASysNum,
                                          state.dataSize->FinalZoneSizing,
                                          state.dataSize->FinalSysSizing);

        if (coolBeam.NumBeams == AutoSize) {
            rho = state.dataPlnt->PlantLoop(coolBeam.CWPlantLoc.loopNum).glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);

            NumBeams = int(coolBeam.MaxCoolWaterVolFlow * rho / NomMassFlowPerBeam) + 1;
            coolBeam.NumBeams = double(NumBeams);
            BaseSizer::reportSizerOutput(state, coolBeam.UnitType, coolBeam.Name, "Number of Beams", coolBeam.NumBeams);
        }

        if (coolBeam.BeamLength == AutoSize) {

            if (state.dataSize->CurTermUnitSizingNum > 0) {

                CheckZoneSizing(state, coolBeam.UnitType, coolBeam.Name);

                if (PltSizCoolNum > 0) {
                    rho = state.dataPlnt->PlantLoop(coolBeam.CWPlantLoc.loopNum).glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);

                    Cp = state.dataPlnt->PlantLoop(coolBeam.CWPlantLoc.loopNum).glycol->getSpecificHeat(state, Constant::CWInitConvTemp, RoutineName);
                    DesCoilLoad = coolBeam.MaxCoolWaterVolFlow * (coolBeam.DesOutletWaterTemp - coolBeam.DesInletWaterTemp) * Cp * rho;
                    if (DesCoilLoad > 0.0) {
                        DesLoadPerBeam = DesCoilLoad / NumBeams;
                        DesAirFlowPerBeam = coolBeam.MaxAirVolFlow / NumBeams;
                        WaterVolFlowPerBeam = coolBeam.MaxCoolWaterVolFlow / NumBeams;
                        WaterVel = WaterVolFlowPerBeam / (Constant::Pi * pow_2(coolBeam.InDiam) / 4.0);
                        if (state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).ZoneTempAtCoolPeak > 0.0) {
                            DT = state.dataSize->TermUnitFinalZoneSizing(state.dataSize->CurTermUnitSizingNum).ZoneTempAtCoolPeak -
                                 0.5 * (coolBeam.DesInletWaterTemp + coolBeam.DesOutletWaterTemp);
                            if (DT <= 0.0) {
                                DT = 7.8;
                            }
                        } else {
                            DT = 7.8;
                        }
                        LengthX = 1.0;
                        for (int Iter = 1; Iter <= 100; ++Iter) {
                            IndAirFlowPerBeamL = coolBeam.K1 * std::pow(DT, coolBeam.n) + coolBeam.Kin * DesAirFlowPerBeam / LengthX;
                            ConvFlow = (IndAirFlowPerBeamL / coolBeam.a0) * RhoAir;
                            if (WaterVel > MinWaterVel) {
                                K = coolBeam.a * std::pow(DT, coolBeam.n1) * std::pow(ConvFlow, coolBeam.n2) * std::pow(WaterVel, coolBeam.n3);
                            } else {
                                K = coolBeam.a * std::pow(DT, coolBeam.n1) * std::pow(ConvFlow, coolBeam.n2) * std::pow(MinWaterVel, coolBeam.n3) *
                                    (WaterVel / MinWaterVel);
                            }
                            Length = DesLoadPerBeam / (K * coolBeam.CoilArea * DT);
                            if (coolBeam.Kin <= 0.0) {
                                break;
                            }
                            // Check for convergence
                            if (std::abs(Length - LengthX) > 0.01) {
                                // New guess for length
                                LengthX += 0.5 * (Length - LengthX);
                            } else {
                                break; // convergence achieved
                            }
                        }
                    } else {
                        Length = 0.0;
                    }
                    coolBeam.BeamLength = Length;
                    coolBeam.BeamLength = max(coolBeam.BeamLength, 1.0);
                    BaseSizer::reportSizerOutput(state, coolBeam.UnitType, coolBeam.Name, "Beam Length [m]", coolBeam.BeamLength);
                } else {
                    ShowSevereError(state, "Autosizing of cooled beam length requires a cooling loop Sizing:Plant object");
                    ShowContinueError(state, format("Occurs in{} Object={}", coolBeam.UnitType, coolBeam.Name));
                    ErrorsFound = true;
                }
            }
        }

        // save the design water volumetric flow rate for use by the water loop sizing algorithms
        if (coolBeam.MaxCoolWaterVolFlow > 0.0) {
            RegisterPlantCompDesignFlow(state, coolBeam.CWInNode, coolBeam.MaxCoolWaterVolFlow);
        }

        if (ErrorsFound) {
            ShowFatalError(state, "Preceding cooled beam sizing errors cause program termination");
        }
    }

    void ControlCoolBeam(EnergyPlusData &state,
                         int const CBNum,                                // number of the current unit being simulated
                         int const ZoneNum,                              // number of zone being served
                         int const ZoneNodeNum,                          // zone node number
                         [[maybe_unused]] bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                         Real64 &NonAirSysOutput                         // convective cooling by the beam system [W]
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   Feb 12, 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Simulate a cooled beam unit;

        // METHODOLOGY EMPLOYED:
        // (1) From the zone load and the Supply air inlet conditions calculate the beam load
        // (2) If there is a beam load, vary the water flow rate to match the beam load

        // REFERENCES:
        // na

        // Using/Aliasing
        using namespace DataZoneEnergyDemands;
        using PlantUtilities::SetComponentFlowRate;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // na

        // INTERFACE BLOCK SPECIFICATIONS:
        // na

        // DERIVED TYPE DEFINITIONS:
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 QZnReq;                // heating or cooling needed by zone [Watts]
        Real64 QToHeatSetPt;          // [W]  remaining load to heating setpoint
        Real64 QToCoolSetPt;          // [W]  remaining load to cooling setpoint
        Real64 QMin(0.0);             // cooled beam output at minimum water flow [W]
        Real64 QMax(0.0);             // cooled beam output at maximum water flow [W]
        Real64 QSup(0.0);             // heating or cooling by supply air [W]
        Real64 PowerMet(0.0);         // power supplied
        Real64 CWFlow(0.0);           // cold water flow [kg/s]
        Real64 AirMassFlow(0.0);      // air mass flow rate for the cooled beam system [kg/s]
        Real64 MaxColdWaterFlow(0.0); // max water mass flow rate for the cooled beam system [kg/s]
        Real64 MinColdWaterFlow(0.0); // min water mass flow rate for the cooled beam system [kg/s]
        Real64 CpAirZn(0.0);          // specific heat of air at zone conditions [J/kg-C]
        Real64 CpAirSys(0.0);         // specific heat of air at supply air conditions [J/kg-C]
        Real64 TWOut(0.0);            // outlet water tamperature [C]
        int ControlNode;              // the water inlet node
        int InAirNode;                // the air inlet node
        bool UnitOn;                  // TRUE if unit is on
        Real64 ErrTolerance;
        auto &coolBeam = state.dataHVACCooledBeam->CoolBeam(CBNum);

        UnitOn = true;
        PowerMet = 0.0;
        InAirNode = coolBeam.AirInNode;
        ControlNode = coolBeam.CWInNode;
        AirMassFlow = state.dataLoopNodes->Node(InAirNode).MassFlowRateMaxAvail;
        QZnReq = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputRequired;
        QToHeatSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToHeatSP;
        QToCoolSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToCoolSP;
        CpAirZn = PsyCpAirFnW(state.dataLoopNodes->Node(ZoneNodeNum).HumRat);
        CpAirSys = PsyCpAirFnW(state.dataLoopNodes->Node(InAirNode).HumRat);
        MaxColdWaterFlow = coolBeam.MaxCoolWaterMassFlow;
        SetComponentFlowRate(state, MaxColdWaterFlow, coolBeam.CWInNode, coolBeam.CWOutNode, coolBeam.CWPlantLoc);
        MinColdWaterFlow = 0.0;
        SetComponentFlowRate(state, MinColdWaterFlow, coolBeam.CWInNode, coolBeam.CWOutNode, coolBeam.CWPlantLoc);

        if (coolBeam.availSched->getCurrentVal() <= 0.0) {
            UnitOn = false;
        }
        if (MaxColdWaterFlow <= SmallMassFlow) {
            UnitOn = false;
        }

        // Set the unit's air inlet nodes mass flow rates
        state.dataLoopNodes->Node(InAirNode).MassFlowRate = AirMassFlow;
        // set the air volumetric flow rate per beam
        coolBeam.BeamFlow = state.dataLoopNodes->Node(InAirNode).MassFlowRate / (state.dataEnvrn->StdRhoAir * coolBeam.NumBeams);
        // fire the unit at min water flow
        CalcCoolBeam(state, CBNum, ZoneNodeNum, MinColdWaterFlow, QMin, TWOut);
        // cooling by supply air
        QSup = AirMassFlow * (CpAirSys * state.dataLoopNodes->Node(InAirNode).Temp - CpAirZn * state.dataLoopNodes->Node(ZoneNodeNum).Temp);
        // load on the beams is QToCoolSetPt-QSup
        if (UnitOn) {
            if ((QToCoolSetPt - QSup) < -SmallLoad) {
                // There is a cooling demand on the cooled beam system.
                // First, see if the system can meet the load
                CalcCoolBeam(state, CBNum, ZoneNodeNum, MaxColdWaterFlow, QMax, TWOut);
                if ((QMax < QToCoolSetPt - QSup - SmallLoad) && (QMax != QMin)) {
                    // The cooled beam system can meet the demand.
                    // Set up the iterative calculation of chilled water flow rate
                    ErrTolerance = 0.01;
                    auto f = [&state, CBNum, ZoneNodeNum, QToCoolSetPt, QSup, QMin, QMax](Real64 const CWFlow) {
                        Real64 const par3 = QToCoolSetPt - QSup;
                        Real64 UnitOutput(0.0);
                        Real64 TWOut(0.0);
                        CalcCoolBeam(state, CBNum, ZoneNodeNum, CWFlow, UnitOutput, TWOut);
                        return (par3 - UnitOutput) / (QMax - QMin);
                    };
                    int SolFlag = 0;
                    General::SolveRoot(state, ErrTolerance, 50, SolFlag, CWFlow, f, MinColdWaterFlow, MaxColdWaterFlow);
                    if (SolFlag == -1) {
                        ShowWarningError(state, format("Cold water control failed in cooled beam unit {}", coolBeam.Name));
                        ShowContinueError(state, "  Iteration limit exceeded in calculating cold water mass flow rate");
                    } else if (SolFlag == -2) {
                        ShowWarningError(state, format("Cold water control failed in cooled beam unit {}", coolBeam.Name));
                        ShowContinueError(state, "  Bad cold water flow limits");
                    }
                } else {
                    // unit maxed out
                    CWFlow = MaxColdWaterFlow;
                }
            } else {
                // unit has no load
                CWFlow = MinColdWaterFlow;
            }
        } else {
            // unit Off
            CWFlow = MinColdWaterFlow;
        }
        // Get the cooling output at the chosen water flow rate
        CalcCoolBeam(state, CBNum, ZoneNodeNum, CWFlow, PowerMet, TWOut);
        coolBeam.BeamCoolingRate = -PowerMet;
        if (QSup < 0.0) {
            coolBeam.SupAirCoolingRate = std::abs(QSup);
        } else {
            coolBeam.SupAirHeatingRate = QSup;
        }
        coolBeam.CoolWaterMassFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRate;
        coolBeam.TWOut = TWOut;
        coolBeam.EnthWaterOut = state.dataLoopNodes->Node(ControlNode).Enthalpy + coolBeam.BeamCoolingRate;
        //  Node(ControlNode)%MassFlowRate = CWFlow
        NonAirSysOutput = PowerMet;
    }

    void CalcCoolBeam(EnergyPlusData &state,
                      int const CBNum,     // Unit index
                      int const ZoneNode,  // zone node number
                      Real64 const CWFlow, // cold water flow [kg/s]
                      Real64 &LoadMet,     // load met by unit [W]
                      Real64 &TWOut        // chilled water outlet temperature [C]
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   Feb 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Simulate a cooled beam given the chilled water flow rate

        // METHODOLOGY EMPLOYED:
        // Uses the cooled beam equations; iteratively varies water outlet  temperature
        // until air-side and water-side cooling outputs match.

        // REFERENCES:
        // na

        // Using/Aliasing
        using PlantUtilities::SetComponentFlowRate;

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("CalcCoolBeam");

        // INTERFACE BLOCK SPECIFICATIONS
        // na

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int Iter(0);                // TWOut iteration index
        Real64 TWIn(0.0);           // Inlet water temperature [C]
        Real64 ZTemp(0.0);          // zone air temperature [C]
        Real64 WaterCoolPower(0.0); // cooling power from water side [W]
        Real64 DT(0.0);             // approximate air - water delta T [C]
        Real64 IndFlow(0.0);        // induced air flow rate per beam length [m3/s-m]
        Real64 CoilFlow(0.0);       // mass air flow rate of air passing through "coil" [kg/m2-s]
        Real64 WaterVel(0.0);       // water velocity [m/s]
        Real64 K(0.0);              // coil heat transfer coefficient [W/m2-K]
        Real64 AirCoolPower(0.0);   // cooling power from the air side [W]
        Real64 Diff;                // difference between water side cooling power and air side cooling power [W]
        Real64 CWFlowPerBeam(0.0);  // water mass flow rate per beam
        Real64 Coeff(0.0);          // iteration parameter
        Real64 Delta(0.0);
        Real64 mdot(0.0);
        Real64 Cp;  // local fluid specific heat
        Real64 rho; // local fluid density

        // test CWFlow against plant
        mdot = CWFlow;
        auto const &coolBeam = state.dataHVACCooledBeam->CoolBeam(CBNum);

        SetComponentFlowRate(state, mdot, coolBeam.CWInNode, coolBeam.CWOutNode, coolBeam.CWPlantLoc);

        CWFlowPerBeam = mdot / coolBeam.NumBeams;
        TWIn = coolBeam.TWIn;

        Cp = state.dataPlnt->PlantLoop(coolBeam.CWPlantLoc.loopNum).glycol->getSpecificHeat(state, TWIn, RoutineName);

        rho = state.dataPlnt->PlantLoop(coolBeam.CWPlantLoc.loopNum).glycol->getDensity(state, TWIn, RoutineName);

        TWOut = TWIn + 2.0;
        ZTemp = state.dataLoopNodes->Node(ZoneNode).Temp;
        if (mdot <= 0.0 || TWIn <= 0.0) {
            LoadMet = 0.0;
            TWOut = TWIn;
            return;
        }
        for (Iter = 1; Iter <= 200; ++Iter) {
            if (Iter > 50 && Iter < 100) {
                Coeff = 0.1 * Coeff2;
            } else if (Iter > 100) {
                Coeff = 0.01 * Coeff2;
            } else {
                Coeff = Coeff2;
            }

            WaterCoolPower = CWFlowPerBeam * Cp * (TWOut - TWIn);
            DT = max(ZTemp - 0.5 * (TWIn + TWOut), 0.0);
            IndFlow = coolBeam.K1 * std::pow(DT, coolBeam.n) + coolBeam.Kin * coolBeam.BeamFlow / coolBeam.BeamLength;
            CoilFlow = (IndFlow / coolBeam.a0) * state.dataEnvrn->StdRhoAir;
            WaterVel = CWFlowPerBeam / (rho * Constant::Pi * pow_2(coolBeam.InDiam) / 4.0);
            if (WaterVel > MinWaterVel) {
                K = coolBeam.a * std::pow(DT, coolBeam.n1) * std::pow(CoilFlow, coolBeam.n2) * std::pow(WaterVel, coolBeam.n3);
            } else {
                K = coolBeam.a * std::pow(DT, coolBeam.n1) * std::pow(CoilFlow, coolBeam.n2) * std::pow(MinWaterVel, coolBeam.n3) *
                    (WaterVel / MinWaterVel);
            }
            AirCoolPower = K * coolBeam.CoilArea * DT * coolBeam.BeamLength;
            Diff = WaterCoolPower - AirCoolPower;
            Delta = TWOut * (std::abs(Diff) / Coeff);
            if (std::abs(Diff) > 0.1) {
                if (Diff < 0.0) {
                    TWOut += Delta;      // increase TWout
                    if (TWOut > ZTemp) { // check that water outlet temperature is less than zone temperature
                        WaterCoolPower = 0.0;
                        TWOut = ZTemp;
                        break;
                    }
                } else {
                    TWOut -= Delta; // Decrease TWout
                    if (TWOut < TWIn) {
                        TWOut = TWIn;
                    }
                }
            } else {
                // water and air side outputs have converged
                break;
            }
        }
        LoadMet = -WaterCoolPower * coolBeam.NumBeams;
    }

    void UpdateCoolBeam(EnergyPlusData &state, int const CBNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   Feb 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the cooled beam unit outlet nodes

        // METHODOLOGY EMPLOYED:
        // Data is moved from the cooled beam unit data structure to the unit outlet nodes.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        auto &coolBeam = state.dataHVACCooledBeam->CoolBeam(CBNum);
        auto const &airInletNode = state.dataLoopNodes->Node(coolBeam.AirInNode);
        auto &airOutletNode = state.dataLoopNodes->Node(coolBeam.AirOutNode);

        // Set the outlet air nodes of the unit; note that all quantities are unchanged
        airOutletNode.MassFlowRate = airInletNode.MassFlowRate;
        airOutletNode.Temp = airInletNode.Temp;
        airOutletNode.HumRat = airInletNode.HumRat;
        airOutletNode.Enthalpy = airInletNode.Enthalpy;

        // Set the outlet water nodes for the unit
        PlantUtilities::SafeCopyPlantNode(state, coolBeam.CWInNode, coolBeam.CWOutNode);

        state.dataLoopNodes->Node(coolBeam.CWOutNode).Temp = coolBeam.TWOut;
        state.dataLoopNodes->Node(coolBeam.CWOutNode).Enthalpy = coolBeam.EnthWaterOut;

        // Set the air outlet nodes for properties that just pass through & not used
        airOutletNode.Quality = airInletNode.Quality;
        airOutletNode.Press = airInletNode.Press;
        airOutletNode.MassFlowRateMin = airInletNode.MassFlowRateMin;
        airOutletNode.MassFlowRateMax = airInletNode.MassFlowRateMax;
        airOutletNode.MassFlowRateMinAvail = airInletNode.MassFlowRateMinAvail;
        airOutletNode.MassFlowRateMaxAvail = airInletNode.MassFlowRateMaxAvail;

        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            airOutletNode.CO2 = airInletNode.CO2;
        }

        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            airOutletNode.GenContam = airInletNode.GenContam;
        }
    }

    void ReportCoolBeam(EnergyPlusData &state, int const CBNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   Feb 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine updates the report variable for the cooled beam units

        // METHODOLOGY EMPLOYED:
        // NA

        // REFERENCES:
        // na

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

        Real64 ReportingConstant;
        auto &coolBeam = state.dataHVACCooledBeam->CoolBeam(CBNum);

        ReportingConstant = state.dataHVACGlobal->TimeStepSysSec;
        // report the WaterCoil energy from this component
        coolBeam.BeamCoolingEnergy = coolBeam.BeamCoolingRate * ReportingConstant;
        coolBeam.SupAirCoolingEnergy = coolBeam.SupAirCoolingRate * ReportingConstant;
        coolBeam.SupAirHeatingEnergy = coolBeam.SupAirHeatingRate * ReportingConstant;

        // set zone OA volume flow rate report variable
        coolBeam.CalcOutdoorAirVolumeFlowRate(state);
    }

    void CoolBeamData::CalcOutdoorAirVolumeFlowRate(EnergyPlusData &state)
    {
        // calculates zone outdoor air volume flow rate using the supply air flow rate and OA fraction
        if (this->AirLoopNum > 0) {
            this->OutdoorAirFlowRate = (state.dataLoopNodes->Node(this->AirOutNode).MassFlowRate / state.dataEnvrn->StdRhoAir) *
                                       state.dataAirLoop->AirLoopFlow(this->AirLoopNum).OAFrac;
        } else {
            this->OutdoorAirFlowRate = 0.0;
        }
    }

    void CoolBeamData::reportTerminalUnit(EnergyPlusData &state)
    {
        // populate the predefined equipment summary report related to air terminals
        auto &orp = state.dataOutRptPredefined;
        auto &adu = state.dataDefineEquipment->AirDistUnit(this->ADUNum);
        if (!state.dataSize->TermUnitFinalZoneSizing.empty()) {
            auto &sizing = state.dataSize->TermUnitFinalZoneSizing(adu.TermUnitSizingNum);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMinFlow, adu.Name, sizing.DesCoolVolFlowMin);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMinOutdoorFlow, adu.Name, sizing.MinOA);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermSupCoolingSP, adu.Name, sizing.CoolDesTemp);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermSupHeatingSP, adu.Name, sizing.HeatDesTemp);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermHeatingCap, adu.Name, sizing.DesHeatLoad);
            OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermCoolingCap, adu.Name, sizing.DesCoolLoad);
        }
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermTypeInp, adu.Name, this->UnitType);
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermPrimFlow, adu.Name, this->MaxAirVolFlow);
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermSecdFlow, adu.Name, "n/a");
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMinFlowSch, adu.Name, "n/a");
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMaxFlowReh, adu.Name, "n/a");
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermMinOAflowSch, adu.Name, "n/a");
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermHeatCoilType, adu.Name, "n/a");
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermCoolCoilType, adu.Name, this->CBTypeString);
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermFanType, adu.Name, "n/a");
        OutputReportPredefined::PreDefTableEntry(state, orp->pdchAirTermFanName, adu.Name, "n/a");
    }

} // namespace HVACCooledBeam

} // namespace EnergyPlus

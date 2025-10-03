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
#include <EnergyPlus/Autosizing/CoolingAirFlowSizing.hh>
#include <EnergyPlus/Autosizing/CoolingCapacitySizing.hh>
#include <EnergyPlus/Autosizing/SystemAirFlowSizing.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/HVACHXAssistedCoolingCoil.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ReportCoilSelection.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/VariableSpeedCoils.hh>
#include <EnergyPlus/WindowAC.hh>

namespace EnergyPlus {

namespace WindowAC {

    // Module containing the routines dealing window air conditioner units

    // MODULE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   May 2000
    //       MODIFIED       Richard Raustad, FSEC Oct 2003
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and algorithms needed to simulate window air
    // conditioner units.

    // METHODOLOGY EMPLOYED:
    // Units are modeled as a collection of components: outside air mixer,
    // fan and DX coil. Control is by means of cycling: either continuous
    // air flow with the DX compressor cycling on/off or the entire unit -
    // fan and compressor cycling on/off. Cycling behavior is not explicitly
    // modeled - instead cycling inefficiencies must be included in the efficiency
    // curves of the DX module.

    using namespace DataLoopNode;
    using namespace DataSizing;
    using HVAC::CoilDX_CoolingHXAssisted;
    using HVAC::CoilDX_CoolingSingleSpeed;
    using HVAC::SmallAirVolFlow;
    using HVAC::SmallLoad;
    using HVAC::SmallMassFlow;
    using Psychrometrics::PsyCpAirFnW;
    using Psychrometrics::PsyHFnTdbW;
    using Psychrometrics::PsyRhoAirFnPbTdbW;

    void SimWindowAC(EnergyPlusData &state,
                     std::string_view CompName,     // name of the window AC unit
                     int const ZoneNum,             // number of zone being served
                     bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                     Real64 &PowerMet,              // Sensible power supplied by window AC (W)
                     Real64 &LatOutputProvided,     // Latent add/removal supplied by window AC (kg/s), dehumid = negative
                     int &CompIndex                 // component index
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   May 2000
        //       MODIFIED       Don Shirey, Aug 2009 (LatOutputProvided)
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Manages the simulation of a window AC unit. Called from SimZone Equipment

        int WindACNum;                     // index of window AC unit being simulated
        Real64 QZnReq;                     // zone load (W)
        Real64 RemainingOutputToCoolingSP; // - remaining load to cooling setpoint (W)

        // First time SimWindowAC is called, get the input for all the window AC units
        if (state.dataWindowAC->GetWindowACInputFlag) {
            GetWindowAC(state);
            state.dataWindowAC->GetWindowACInputFlag = false;
        }

        // Find the correct Window AC Equipment
        if (CompIndex == 0) {
            WindACNum = Util::FindItemInList(CompName, state.dataWindowAC->WindAC);
            if (WindACNum == 0) {
                ShowFatalError(state, format("SimWindowAC: Unit not found={}", CompName));
            }
            CompIndex = WindACNum;
        } else {
            WindACNum = CompIndex;
            if (WindACNum > state.dataWindowAC->NumWindAC || WindACNum < 1) {
                ShowFatalError(state,
                               format("SimWindowAC:  Invalid CompIndex passed={}, Number of Units={}, Entered Unit name={}",
                                      WindACNum,
                                      state.dataWindowAC->NumWindAC,
                                      CompName));
            }
            if (state.dataWindowAC->CheckEquipName(WindACNum)) {
                if (CompName != state.dataWindowAC->WindAC(WindACNum).Name) {
                    ShowFatalError(state,
                                   format("SimWindowAC: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                          WindACNum,
                                          CompName,
                                          state.dataWindowAC->WindAC(WindACNum).Name));
                }
                state.dataWindowAC->CheckEquipName(WindACNum) = false;
            }
        }

        RemainingOutputToCoolingSP = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToCoolSP;

        if (RemainingOutputToCoolingSP < 0.0 && state.dataHeatBalFanSys->TempControlType(ZoneNum) != HVAC::SetptType::SingleHeat) {
            QZnReq = RemainingOutputToCoolingSP;
        } else {
            QZnReq = 0.0;
        }

        state.dataSize->ZoneEqDXCoil = true;
        state.dataSize->ZoneCoolingOnlyFan = true;

        // Initialize the window AC unit
        InitWindowAC(state, WindACNum, QZnReq, ZoneNum, FirstHVACIteration);

        SimCyclingWindowAC(state, WindACNum, ZoneNum, FirstHVACIteration, PowerMet, QZnReq, LatOutputProvided);

        // Report the result of the simulation
        ReportWindowAC(state, WindACNum);

        state.dataSize->ZoneEqDXCoil = false;
        state.dataSize->ZoneCoolingOnlyFan = false;
    }

    void GetWindowAC(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   May 2000
        //       MODIFIED       Chandan Sharma, FSEC, March 2011: Added zone sys avail manager
        //                      Bereket Nigusse, FSEC, April 2011: eliminated input node names,
        //                                                         added OA Mixer object type
        //                                                         and fan object type
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for window AC units and stores it in window AC data structures

        // METHODOLOGY EMPLOYED:
        // Uses "Get" routines to read in data.

        using BranchNodeConnections::SetUpCompSets;

        using NodeInputManager::GetOnlySingleNode;
        auto &GetDXCoilOutletNode(DXCoils::GetCoilOutletNode);
        auto &GetDXHXAsstdCoilOutletNode(HVACHXAssistedCoolingCoil::GetCoilOutletNode);
        using MixedAir::GetOAMixerIndex;
        using MixedAir::GetOAMixerNodeNumbers;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetWindowAC: "); // include trailing blank space
        static constexpr std::string_view routineName = "GetWindowAC";  // include trailing blank space

        int WindACIndex; // loop index
        int WindACNum;   // current window AC number
        std::string CompSetFanInlet;
        std::string CompSetCoolInlet;
        std::string CompSetFanOutlet;
        std::string CompSetCoolOutlet;
        int NumAlphas;                   // Number of Alphas for each GetObjectItem call
        int NumNumbers;                  // Number of Numbers for each GetObjectItem call
        Array1D_int OANodeNums(4);       // Node numbers of Outdoor air mixer (OA, EA, RA, MA)
        int IOStatus;                    // Used in GetObjectItem
        bool ErrorsFound(false);         // Set to true if errors in input, fatal at end of routine
        Real64 FanVolFlow;               // Fan volumetric flow rate
        bool CoilNodeErrFlag;            // Used in error messages for mining coil outlet node number
        std::string CurrentModuleObject; // Object type for getting and error messages
        Array1D_string Alphas;           // Alpha input items for object
        Array1D_string cAlphaFields;     // Alpha field names
        Array1D_string cNumericFields;   // Numeric field names
        Array1D<Real64> Numbers;         // Numeric input items for object
        Array1D_bool lAlphaBlanks;       // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;     // Logical array, numeric field input BLANK = .TRUE.
        int TotalArgs(0);                // Total number of alpha and numeric arguments (max) for a
        int CtrlZone;                    // index to loop counter
        int NodeNum;                     // index to loop counter
        bool ZoneNodeNotFound;           // used in error checking

        // find the number of each type of window AC unit
        CurrentModuleObject = "ZoneHVAC:WindowAirConditioner";

        state.dataWindowAC->NumWindACCyc = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataWindowAC->NumWindAC = state.dataWindowAC->NumWindACCyc;
        // allocate the data structures
        state.dataWindowAC->WindAC.allocate(state.dataWindowAC->NumWindAC);
        state.dataWindowAC->CheckEquipName.dimension(state.dataWindowAC->NumWindAC, true);
        state.dataWindowAC->WindACNumericFields.allocate(state.dataWindowAC->NumWindAC);

        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, TotalArgs, NumAlphas, NumNumbers);

        Alphas.allocate(NumAlphas);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNumbers);
        Numbers.dimension(NumNumbers, 0.0);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNumbers, true);

        // loop over window AC units; get and load the input data
        for (WindACIndex = 1; WindACIndex <= state.dataWindowAC->NumWindACCyc; ++WindACIndex) {

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     WindACIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            WindACNum = WindACIndex;

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, Alphas(1)};

            state.dataWindowAC->WindACNumericFields(WindACNum).FieldNames.allocate(NumNumbers);
            state.dataWindowAC->WindACNumericFields(WindACNum).FieldNames = "";
            state.dataWindowAC->WindACNumericFields(WindACNum).FieldNames = cNumericFields;
            Util::IsNameEmpty(state, Alphas(1), CurrentModuleObject, ErrorsFound);

            state.dataWindowAC->WindAC(WindACNum).Name = Alphas(1);
            state.dataWindowAC->WindAC(WindACNum).UnitType = state.dataWindowAC->WindowAC_UnitType; // 'ZoneHVAC:WindowAirConditioner'

            if (lAlphaBlanks(2)) {
                state.dataWindowAC->WindAC(WindACNum).availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((state.dataWindowAC->WindAC(WindACNum).availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ErrorsFound = true;
            }

            state.dataWindowAC->WindAC(WindACNum).MaxAirVolFlow = Numbers(1);
            state.dataWindowAC->WindAC(WindACNum).OutAirVolFlow = Numbers(2);

            state.dataWindowAC->WindAC(WindACNum).AirInNode = GetOnlySingleNode(state,
                                                                                Alphas(3),
                                                                                ErrorsFound,
                                                                                DataLoopNode::ConnectionObjectType::ZoneHVACWindowAirConditioner,
                                                                                Alphas(1),
                                                                                DataLoopNode::NodeFluidType::Air,
                                                                                DataLoopNode::ConnectionType::Inlet,
                                                                                NodeInputManager::CompFluidStream::Primary,
                                                                                ObjectIsParent);

            state.dataWindowAC->WindAC(WindACNum).AirOutNode = GetOnlySingleNode(state,
                                                                                 Alphas(4),
                                                                                 ErrorsFound,
                                                                                 DataLoopNode::ConnectionObjectType::ZoneHVACWindowAirConditioner,
                                                                                 Alphas(1),
                                                                                 DataLoopNode::NodeFluidType::Air,
                                                                                 DataLoopNode::ConnectionType::Outlet,
                                                                                 NodeInputManager::CompFluidStream::Primary,
                                                                                 ObjectIsParent);

            state.dataWindowAC->WindAC(WindACNum).OAMixType = Alphas(5);
            state.dataWindowAC->WindAC(WindACNum).OAMixName = Alphas(6);
            // Get outdoor air mixer node numbers
            bool errFlag = false;
            ValidateComponent(state,
                              state.dataWindowAC->WindAC(WindACNum).OAMixType,
                              state.dataWindowAC->WindAC(WindACNum).OAMixName,
                              errFlag,
                              CurrentModuleObject);
            if (errFlag) {
                ShowContinueError(state, format("specified in {} = \"{}\".", CurrentModuleObject, state.dataWindowAC->WindAC(WindACNum).Name));
                ErrorsFound = true;
            } else {
                // Get outdoor air mixer node numbers
                OANodeNums = GetOAMixerNodeNumbers(state, state.dataWindowAC->WindAC(WindACNum).OAMixName, errFlag);
                if (errFlag) {
                    ShowContinueError(state,
                                      format("that was specified in {} = \"{}\"", CurrentModuleObject, state.dataWindowAC->WindAC(WindACNum).Name));
                    ShowContinueError(state, "..OutdoorAir:Mixer is required. Enter an OutdoorAir:Mixer object with this name.");
                    ErrorsFound = true;
                } else {
                    state.dataWindowAC->WindAC(WindACNum).OutsideAirNode = OANodeNums(1);
                    state.dataWindowAC->WindAC(WindACNum).AirReliefNode = OANodeNums(2);
                    state.dataWindowAC->WindAC(WindACNum).ReturnAirNode = OANodeNums(3);
                    state.dataWindowAC->WindAC(WindACNum).MixedAirNode = OANodeNums(4);
                }
            }

            auto &windAC = state.dataWindowAC->WindAC(WindACNum);

            windAC.FanName = Alphas(8);

            windAC.fanType = static_cast<HVAC::FanType>(getEnumValue(HVAC::fanTypeNamesUC, Alphas(7)));

            if (windAC.fanType != HVAC::FanType::OnOff && windAC.fanType != HVAC::FanType::Constant && windAC.fanType != HVAC::FanType::SystemModel) {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(8), Alphas(8), "Fan Type must be Fan:OnOff, Fan:ConstantVolume, or Fan:SystemModel.");

            } else if ((windAC.FanIndex = Fans::GetFanIndex(state, windAC.FanName)) == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(8), windAC.FanName);

            } else {
                auto *fan = state.dataFans->fans(windAC.FanIndex);
                assert(windAC.fanType == fan->type);

                FanVolFlow = fan->maxAirFlowRate;
                if (FanVolFlow != AutoSize) {
                    if (FanVolFlow < windAC.MaxAirVolFlow) {
                        ShowWarningError(state,
                                         format("Air flow rate = {:.7T} in fan object {} is less than the maximum supply air flow "
                                                "rate ({:.7T}) in the {} object.",
                                                FanVolFlow,
                                                windAC.FanName,
                                                windAC.MaxAirVolFlow,
                                                CurrentModuleObject));
                        ShowContinueError(
                            state, format(" The fan flow rate must be >= to the {} in the {} object.", cNumericFields(1), CurrentModuleObject));
                        ShowContinueError(state, format(" Occurs in {} = {}", CurrentModuleObject, state.dataWindowAC->WindAC(WindACNum).Name));
                        ErrorsFound = true;
                    }
                }
                windAC.fanAvailSched = fan->availSched;
            }

            state.dataWindowAC->WindAC(WindACNum).DXCoilName = Alphas(10);

            if (Util::SameString(Alphas(9), "Coil:Cooling:DX:SingleSpeed") ||
                Util::SameString(Alphas(9), "CoilSystem:Cooling:DX:HeatExchangerAssisted") ||
                Util::SameString(Alphas(9), "Coil:Cooling:DX:VariableSpeed")) {
                state.dataWindowAC->WindAC(WindACNum).DXCoilType = Alphas(9);
                CoilNodeErrFlag = false;
                if (Util::SameString(Alphas(9), "Coil:Cooling:DX:SingleSpeed")) {
                    state.dataWindowAC->WindAC(WindACNum).DXCoilType_Num = CoilDX_CoolingSingleSpeed;
                    state.dataWindowAC->WindAC(WindACNum).CoilOutletNodeNum = GetDXCoilOutletNode(
                        state, state.dataWindowAC->WindAC(WindACNum).DXCoilType, state.dataWindowAC->WindAC(WindACNum).DXCoilName, CoilNodeErrFlag);
                } else if (Util::SameString(Alphas(9), "CoilSystem:Cooling:DX:HeatExchangerAssisted")) {
                    state.dataWindowAC->WindAC(WindACNum).DXCoilType_Num = CoilDX_CoolingHXAssisted;
                    state.dataWindowAC->WindAC(WindACNum).CoilOutletNodeNum = GetDXHXAsstdCoilOutletNode(
                        state, state.dataWindowAC->WindAC(WindACNum).DXCoilType, state.dataWindowAC->WindAC(WindACNum).DXCoilName, CoilNodeErrFlag);
                } else if (Util::SameString(Alphas(9), "Coil:Cooling:DX:VariableSpeed")) {
                    state.dataWindowAC->WindAC(WindACNum).DXCoilType_Num = HVAC::Coil_CoolingAirToAirVariableSpeed;
                    state.dataWindowAC->WindAC(WindACNum).CoilOutletNodeNum = VariableSpeedCoils::GetCoilOutletNodeVariableSpeed(
                        state, state.dataWindowAC->WindAC(WindACNum).DXCoilType, state.dataWindowAC->WindAC(WindACNum).DXCoilName, CoilNodeErrFlag);
                    state.dataWindowAC->WindAC(WindACNum).DXCoilNumOfSpeeds =
                        VariableSpeedCoils::GetVSCoilNumOfSpeeds(state, state.dataWindowAC->WindAC(WindACNum).DXCoilName, ErrorsFound);
                }
                if (CoilNodeErrFlag) {
                    ShowContinueError(state,
                                      format(" that was specified in {} = \"{}\".", CurrentModuleObject, state.dataWindowAC->WindAC(WindACNum).Name));
                    ErrorsFound = true;
                }
            } else {
                ShowWarningError(state, format("Invalid {} = {}", cAlphaFields(9), Alphas(9)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataWindowAC->WindAC(WindACNum).Name));
                ErrorsFound = true;
            }

            if (lAlphaBlanks(11)) {
                state.dataWindowAC->WindAC(WindACNum).fanOp = HVAC::FanOp::Cycling;
            } else if ((state.dataWindowAC->WindAC(WindACNum).fanOpModeSched = Sched::GetSchedule(state, Alphas(11))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(11), Alphas(11));
                ErrorsFound = true;
            }

            state.dataWindowAC->WindAC(WindACNum).fanPlace = static_cast<HVAC::FanPlace>(getEnumValue(HVAC::fanPlaceNamesUC, Alphas(12)));
            assert(state.dataWindowAC->WindAC(WindACNum).fanPlace != HVAC::FanPlace::Invalid);

            state.dataWindowAC->WindAC(WindACNum).ConvergenceTol = Numbers(3);

            if (!lAlphaBlanks(13)) {
                state.dataWindowAC->WindAC(WindACNum).AvailManagerListName = Alphas(13);
            }

            state.dataWindowAC->WindAC(WindACNum).HVACSizingIndex = 0;
            if (!lAlphaBlanks(14)) {
                state.dataWindowAC->WindAC(WindACNum).HVACSizingIndex = Util::FindItemInList(Alphas(14), state.dataSize->ZoneHVACSizing);
                if (state.dataWindowAC->WindAC(WindACNum).HVACSizingIndex == 0) {
                    ShowSevereError(state, format("{} = {} not found.", cAlphaFields(14), Alphas(14)));
                    ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, state.dataWindowAC->WindAC(WindACNum).Name));
                    ErrorsFound = true;
                }
            }

            // Add fan to component sets array
            if (state.dataWindowAC->WindAC(WindACNum).fanPlace == HVAC::FanPlace::BlowThru) {

                // Window AC air inlet node must be the same as a zone exhaust node and the OA Mixer return node
                // check that Window AC air inlet node is the same as a zone exhaust node.
                ZoneNodeNotFound = true;
                for (CtrlZone = 1; CtrlZone <= state.dataGlobal->NumOfZones; ++CtrlZone) {
                    if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZone).IsControlled) {
                        continue;
                    }
                    for (NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(CtrlZone).NumExhaustNodes; ++NodeNum) {
                        if (state.dataWindowAC->WindAC(WindACNum).AirInNode == state.dataZoneEquip->ZoneEquipConfig(CtrlZone).ExhaustNode(NodeNum)) {
                            ZoneNodeNotFound = false;
                            break;
                        }
                    }
                }
                if (ZoneNodeNotFound) {
                    ShowSevereError(state,
                                    format("{} = \"{}\". Window AC air inlet node name must be the same as a zone exhaust node name.",
                                           CurrentModuleObject,
                                           state.dataWindowAC->WindAC(WindACNum).Name));
                    ShowContinueError(state, "..Zone exhaust node name is specified in ZoneHVAC:EquipmentConnections object.");
                    ShowContinueError(
                        state,
                        format("..Window AC air inlet node name = {}", state.dataLoopNodes->NodeID(state.dataWindowAC->WindAC(WindACNum).AirInNode)));
                    ErrorsFound = true;
                }
                // check that Window AC air outlet node is a zone inlet node.
                ZoneNodeNotFound = true;
                for (CtrlZone = 1; CtrlZone <= state.dataGlobal->NumOfZones; ++CtrlZone) {
                    if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZone).IsControlled) {
                        continue;
                    }
                    for (NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(CtrlZone).NumInletNodes; ++NodeNum) {
                        if (state.dataWindowAC->WindAC(WindACNum).AirOutNode == state.dataZoneEquip->ZoneEquipConfig(CtrlZone).InletNode(NodeNum)) {
                            state.dataWindowAC->WindAC(WindACNum).ZonePtr = CtrlZone;
                            ZoneNodeNotFound = false;
                            break;
                        }
                    }
                }
                if (ZoneNodeNotFound) {
                    ShowSevereError(state,
                                    format("{} = \"{}\". Window AC air outlet node name must be the same as a zone inlet node name.",
                                           CurrentModuleObject,
                                           state.dataWindowAC->WindAC(WindACNum).Name));
                    ShowContinueError(state, "..Zone inlet node name is specified in ZoneHVAC:EquipmentConnections object.");
                    ShowContinueError(state,
                                      format("..Window AC air outlet node name = {}",
                                             state.dataLoopNodes->NodeID(state.dataWindowAC->WindAC(WindACNum).AirOutNode)));
                    ErrorsFound = true;
                }
                CompSetFanInlet = state.dataLoopNodes->NodeID(state.dataWindowAC->WindAC(WindACNum).MixedAirNode);
                CompSetFanOutlet = "UNDEFINED";
                CompSetCoolInlet = "UNDEFINED";
                CompSetCoolOutlet = state.dataLoopNodes->NodeID(state.dataWindowAC->WindAC(WindACNum).AirOutNode);
            } else { // draw through fan from IF (WindAC(WindACNum)%FanPlace == BlowThru) THEN
                // check that Window AC air inlet node is the same as a zone exhaust node.
                ZoneNodeNotFound = true;
                for (CtrlZone = 1; CtrlZone <= state.dataGlobal->NumOfZones; ++CtrlZone) {
                    if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZone).IsControlled) {
                        continue;
                    }
                    for (NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(CtrlZone).NumExhaustNodes; ++NodeNum) {
                        if (state.dataWindowAC->WindAC(WindACNum).AirInNode == state.dataZoneEquip->ZoneEquipConfig(CtrlZone).ExhaustNode(NodeNum)) {
                            ZoneNodeNotFound = false;
                            break;
                        }
                    }
                }
                if (ZoneNodeNotFound) {
                    ShowSevereError(state,
                                    format("{} = \"{}\". Window AC air inlet node name must be the same as a zone exhaust node name.",
                                           CurrentModuleObject,
                                           state.dataWindowAC->WindAC(WindACNum).Name));
                    ShowContinueError(state, "..Zone exhaust node name is specified in ZoneHVAC:EquipmentConnections object.");
                    ShowContinueError(
                        state,
                        format("..Window AC inlet node name = {}", state.dataLoopNodes->NodeID(state.dataWindowAC->WindAC(WindACNum).AirInNode)));
                    ErrorsFound = true;
                }
                // check that Window AC air outlet node is the same as a zone inlet node.
                ZoneNodeNotFound = true;
                for (CtrlZone = 1; CtrlZone <= state.dataGlobal->NumOfZones; ++CtrlZone) {
                    if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZone).IsControlled) {
                        continue;
                    }
                    for (NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(CtrlZone).NumInletNodes; ++NodeNum) {
                        if (state.dataWindowAC->WindAC(WindACNum).AirOutNode == state.dataZoneEquip->ZoneEquipConfig(CtrlZone).InletNode(NodeNum)) {
                            state.dataWindowAC->WindAC(WindACNum).ZonePtr = CtrlZone;
                            ZoneNodeNotFound = false;
                            break;
                        }
                    }
                }
                if (ZoneNodeNotFound) {
                    ShowSevereError(state,
                                    format("{} = \"{}\". Window AC air outlet node name must be the same as a zone inlet node name.",
                                           CurrentModuleObject,
                                           state.dataWindowAC->WindAC(WindACNum).Name));
                    ShowContinueError(state, "..Zone inlet node name is specified in ZoneHVAC:EquipmentConnections object.");
                    ShowContinueError(
                        state,
                        format("..Window AC outlet node name = {}", state.dataLoopNodes->NodeID(state.dataWindowAC->WindAC(WindACNum).AirOutNode)));
                    ErrorsFound = true;
                }
                CompSetFanInlet = state.dataLoopNodes->NodeID(state.dataWindowAC->WindAC(WindACNum).CoilOutletNodeNum);
                CompSetFanOutlet = state.dataLoopNodes->NodeID(state.dataWindowAC->WindAC(WindACNum).AirOutNode);
                CompSetCoolInlet = state.dataLoopNodes->NodeID(state.dataWindowAC->WindAC(WindACNum).MixedAirNode);
                CompSetCoolOutlet = state.dataLoopNodes->NodeID(state.dataWindowAC->WindAC(WindACNum).CoilOutletNodeNum);
            }
            // Add fan to component sets array
            SetUpCompSets(state,
                          state.dataWindowAC->cWindowAC_UnitTypes(state.dataWindowAC->WindAC(WindACNum).UnitType),
                          state.dataWindowAC->WindAC(WindACNum).Name,
                          HVAC::fanTypeNames[(int)state.dataWindowAC->WindAC(WindACNum).fanType],
                          state.dataWindowAC->WindAC(WindACNum).FanName,
                          CompSetFanInlet,
                          CompSetFanOutlet);

            // Add cooling coil to component sets array
            SetUpCompSets(state,
                          state.dataWindowAC->cWindowAC_UnitTypes(state.dataWindowAC->WindAC(WindACNum).UnitType),
                          state.dataWindowAC->WindAC(WindACNum).Name,
                          state.dataWindowAC->WindAC(WindACNum).DXCoilType,
                          state.dataWindowAC->WindAC(WindACNum).DXCoilName,
                          CompSetCoolInlet,
                          CompSetCoolOutlet);

            // Set up component set for OA mixer - use OA node and Mixed air node
            SetUpCompSets(state,
                          state.dataWindowAC->cWindowAC_UnitTypes(state.dataWindowAC->WindAC(WindACNum).UnitType),
                          state.dataWindowAC->WindAC(WindACNum).Name,
                          state.dataWindowAC->WindAC(WindACNum).OAMixType,
                          state.dataWindowAC->WindAC(WindACNum).OAMixName,
                          state.dataLoopNodes->NodeID(state.dataWindowAC->WindAC(WindACNum).OutsideAirNode),
                          state.dataLoopNodes->NodeID(state.dataWindowAC->WindAC(WindACNum).MixedAirNode));
        }

        Alphas.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        Numbers.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        if (ErrorsFound) {
            ShowFatalError(state,
                           format("{}Errors found in getting {} input.  Preceding condition causes termination.", RoutineName, CurrentModuleObject));
        }

        for (WindACNum = 1; WindACNum <= state.dataWindowAC->NumWindAC; ++WindACNum) {
            // Setup Report variables for the Fan Coils
            SetupOutputVariable(state,
                                "Zone Window Air Conditioner Total Cooling Rate",
                                Constant::Units::W,
                                state.dataWindowAC->WindAC(WindACNum).TotCoolEnergyRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataWindowAC->WindAC(WindACNum).Name);
            SetupOutputVariable(state,
                                "Zone Window Air Conditioner Total Cooling Energy",
                                Constant::Units::J,
                                state.dataWindowAC->WindAC(WindACNum).TotCoolEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataWindowAC->WindAC(WindACNum).Name);
            SetupOutputVariable(state,
                                "Zone Window Air Conditioner Sensible Cooling Rate",
                                Constant::Units::W,
                                state.dataWindowAC->WindAC(WindACNum).SensCoolEnergyRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataWindowAC->WindAC(WindACNum).Name);
            SetupOutputVariable(state,
                                "Zone Window Air Conditioner Sensible Cooling Energy",
                                Constant::Units::J,
                                state.dataWindowAC->WindAC(WindACNum).SensCoolEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataWindowAC->WindAC(WindACNum).Name);
            SetupOutputVariable(state,
                                "Zone Window Air Conditioner Latent Cooling Rate",
                                Constant::Units::W,
                                state.dataWindowAC->WindAC(WindACNum).LatCoolEnergyRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataWindowAC->WindAC(WindACNum).Name);
            SetupOutputVariable(state,
                                "Zone Window Air Conditioner Latent Cooling Energy",
                                Constant::Units::J,
                                state.dataWindowAC->WindAC(WindACNum).LatCoolEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataWindowAC->WindAC(WindACNum).Name);
            SetupOutputVariable(state,
                                "Zone Window Air Conditioner Electricity Rate",
                                Constant::Units::W,
                                state.dataWindowAC->WindAC(WindACNum).ElecPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataWindowAC->WindAC(WindACNum).Name);
            SetupOutputVariable(state,
                                "Zone Window Air Conditioner Electricity Energy",
                                Constant::Units::J,
                                state.dataWindowAC->WindAC(WindACNum).ElecConsumption,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                state.dataWindowAC->WindAC(WindACNum).Name);
            SetupOutputVariable(state,
                                "Zone Window Air Conditioner Fan Part Load Ratio",
                                Constant::Units::None,
                                state.dataWindowAC->WindAC(WindACNum).FanPartLoadRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataWindowAC->WindAC(WindACNum).Name);
            SetupOutputVariable(state,
                                "Zone Window Air Conditioner Compressor Part Load Ratio",
                                Constant::Units::None,
                                state.dataWindowAC->WindAC(WindACNum).CompPartLoadRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataWindowAC->WindAC(WindACNum).Name);
            SetupOutputVariable(state,
                                "Zone Window Air Conditioner Fan Availability Status",
                                Constant::Units::None,
                                state.dataWindowAC->WindAC(WindACNum).availStatus,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                state.dataWindowAC->WindAC(WindACNum).Name);
            if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
                SetupEMSActuator(state,
                                 "Window Air Conditioner",
                                 state.dataWindowAC->WindAC(WindACNum).Name,
                                 "Part Load Ratio",
                                 "[fraction]",
                                 state.dataWindowAC->WindAC(WindACNum).EMSOverridePartLoadFrac,
                                 state.dataWindowAC->WindAC(WindACNum).EMSValueForPartLoadFrac);
            }
        }
        for (WindACNum = 1; WindACNum <= state.dataWindowAC->NumWindAC; ++WindACNum) {
            state.dataRptCoilSelection->coilSelectionReportObj->setCoilSupplyFanInfo(state,
                                                                                     state.dataWindowAC->WindAC(WindACNum).DXCoilName,
                                                                                     state.dataWindowAC->WindAC(WindACNum).DXCoilType,
                                                                                     state.dataWindowAC->WindAC(WindACNum).FanName,
                                                                                     state.dataWindowAC->WindAC(WindACNum).fanType,
                                                                                     state.dataWindowAC->WindAC(WindACNum).FanIndex);
        }
    }

    void InitWindowAC(EnergyPlusData &state,
                      int const WindACNum,          // number of the current window AC unit being simulated
                      Real64 &QZnReq,               // zone load (modified as needed) (W)
                      int const ZoneNum,            // index to zone
                      bool const FirstHVACIteration // TRUE when first HVAC iteration
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   May 2000
        //       MODIFIED       Chandan Sharma, FSEC, March 2011: Added zone sys avail manager

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the Window AC Components.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        // Do the one time initializations
        if (state.dataWindowAC->MyOneTimeFlag) {

            state.dataWindowAC->MyEnvrnFlag.allocate(state.dataWindowAC->NumWindAC);
            state.dataWindowAC->MySizeFlag.allocate(state.dataWindowAC->NumWindAC);
            state.dataWindowAC->MyZoneEqFlag.allocate(state.dataWindowAC->NumWindAC);
            state.dataWindowAC->MyEnvrnFlag = true;
            state.dataWindowAC->MySizeFlag = true;
            state.dataWindowAC->MyZoneEqFlag = true;
            state.dataWindowAC->MyOneTimeFlag = false;
        }

        if (allocated(state.dataAvail->ZoneComp)) {
            auto &availMgr = state.dataAvail->ZoneComp(DataZoneEquipment::ZoneEquipType::WindowAirConditioner).ZoneCompAvailMgrs(WindACNum);
            if (state.dataWindowAC->MyZoneEqFlag(WindACNum)) { // initialize the name of each availability manager list and zone number
                availMgr.AvailManagerListName = state.dataWindowAC->WindAC(WindACNum).AvailManagerListName;
                availMgr.ZoneNum = ZoneNum;
                state.dataWindowAC->MyZoneEqFlag(WindACNum) = false;
            }
            state.dataWindowAC->WindAC(WindACNum).availStatus = availMgr.availStatus;
        }

        // need to check all Window AC units to see if they are on Zone Equipment List or issue warning
        if (!state.dataWindowAC->ZoneEquipmentListChecked && state.dataZoneEquip->ZoneEquipInputsFilled) {
            state.dataWindowAC->ZoneEquipmentListChecked = true;
            for (int Loop = 1; Loop <= state.dataWindowAC->NumWindAC; ++Loop) {
                if (DataZoneEquipment::CheckZoneEquipmentList(state,
                                                              state.dataWindowAC->cWindowAC_UnitTypes(state.dataWindowAC->WindAC(Loop).UnitType),
                                                              state.dataWindowAC->WindAC(Loop).Name)) {
                    continue;
                }
                ShowSevereError(state,
                                format("InitWindowAC: Window AC Unit=[{},{}] is not on any ZoneHVAC:EquipmentList.  It will not be simulated.",
                                       state.dataWindowAC->cWindowAC_UnitTypes(state.dataWindowAC->WindAC(Loop).UnitType),
                                       state.dataWindowAC->WindAC(Loop).Name));
            }
        }

        if (!state.dataGlobal->SysSizingCalc && state.dataWindowAC->MySizeFlag(WindACNum)) {

            SizeWindowAC(state, WindACNum);

            state.dataWindowAC->MySizeFlag(WindACNum) = false;
        }

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && state.dataWindowAC->MyEnvrnFlag(WindACNum)) {
            int InNode = state.dataWindowAC->WindAC(WindACNum).AirInNode;
            int OutNode = state.dataWindowAC->WindAC(WindACNum).AirOutNode;
            int OutsideAirNode = state.dataWindowAC->WindAC(WindACNum).OutsideAirNode;
            Real64 RhoAir = state.dataEnvrn->StdRhoAir;
            // set the mass flow rates from the input volume flow rates
            state.dataWindowAC->WindAC(WindACNum).MaxAirMassFlow = RhoAir * state.dataWindowAC->WindAC(WindACNum).MaxAirVolFlow;
            state.dataWindowAC->WindAC(WindACNum).OutAirMassFlow = RhoAir * state.dataWindowAC->WindAC(WindACNum).OutAirVolFlow;
            // set the node max and min mass flow rates
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMax = state.dataWindowAC->WindAC(WindACNum).OutAirMassFlow;
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMin = 0.0;
            state.dataLoopNodes->Node(OutNode).MassFlowRateMax = state.dataWindowAC->WindAC(WindACNum).MaxAirMassFlow;
            state.dataLoopNodes->Node(OutNode).MassFlowRateMin = 0.0;
            state.dataLoopNodes->Node(InNode).MassFlowRateMax = state.dataWindowAC->WindAC(WindACNum).MaxAirMassFlow;
            state.dataLoopNodes->Node(InNode).MassFlowRateMin = 0.0;
            state.dataWindowAC->MyEnvrnFlag(WindACNum) = false;
        } // end one time inits

        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataWindowAC->MyEnvrnFlag(WindACNum) = true;
        }

        if (state.dataWindowAC->WindAC(WindACNum).fanOpModeSched != nullptr) {
            if (state.dataWindowAC->WindAC(WindACNum).fanOpModeSched->getCurrentVal() == 0.0) {
                state.dataWindowAC->WindAC(WindACNum).fanOp = HVAC::FanOp::Cycling;
            } else {
                state.dataWindowAC->WindAC(WindACNum).fanOp = HVAC::FanOp::Continuous;
            }
        }

        // These initializations are done every iteration
        int InletNode = state.dataWindowAC->WindAC(WindACNum).AirInNode;
        int OutsideAirNode = state.dataWindowAC->WindAC(WindACNum).OutsideAirNode;
        int AirRelNode = state.dataWindowAC->WindAC(WindACNum).AirReliefNode;
        // Set the inlet node mass flow rate
        if (state.dataWindowAC->WindAC(WindACNum).availSched->getCurrentVal() <= 0.0 ||
            (state.dataWindowAC->WindAC(WindACNum).fanAvailSched->getCurrentVal() <= 0.0 && !state.dataHVACGlobal->TurnFansOn) ||
            state.dataHVACGlobal->TurnFansOff) {
            state.dataWindowAC->WindAC(WindACNum).PartLoadFrac = 0.0;
            state.dataLoopNodes->Node(InletNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = 0.0;
            state.dataLoopNodes->Node(InletNode).MassFlowRateMinAvail = 0.0;
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMaxAvail = 0.0;
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMinAvail = 0.0;
            state.dataLoopNodes->Node(AirRelNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(AirRelNode).MassFlowRateMaxAvail = 0.0;
            state.dataLoopNodes->Node(AirRelNode).MassFlowRateMinAvail = 0.0;
        } else {
            state.dataWindowAC->WindAC(WindACNum).PartLoadFrac = 1.0;
            state.dataLoopNodes->Node(InletNode).MassFlowRate = state.dataWindowAC->WindAC(WindACNum).MaxAirMassFlow;
            state.dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = state.dataLoopNodes->Node(InletNode).MassFlowRate;
            state.dataLoopNodes->Node(InletNode).MassFlowRateMinAvail = state.dataLoopNodes->Node(InletNode).MassFlowRate;
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate = state.dataWindowAC->WindAC(WindACNum).OutAirMassFlow;
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMaxAvail = state.dataWindowAC->WindAC(WindACNum).OutAirMassFlow;
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMinAvail = 0.0;
            state.dataLoopNodes->Node(AirRelNode).MassFlowRate = state.dataWindowAC->WindAC(WindACNum).OutAirMassFlow;
            state.dataLoopNodes->Node(AirRelNode).MassFlowRateMaxAvail = state.dataWindowAC->WindAC(WindACNum).OutAirMassFlow;
            state.dataLoopNodes->Node(AirRelNode).MassFlowRateMinAvail = 0.0;
        }

        // Original thermostat control logic (works only for cycling fan systems)
        if (QZnReq < (-1.0 * HVAC::SmallLoad) && !state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum) &&
            state.dataWindowAC->WindAC(WindACNum).PartLoadFrac > 0.0) {
            state.dataWindowAC->CoolingLoad = true;
        } else {
            state.dataWindowAC->CoolingLoad = false;
        }

        // Constant fan systems are tested for ventilation load to determine if load to be met changes.
        if (state.dataWindowAC->WindAC(WindACNum).fanOp == HVAC::FanOp::Continuous && state.dataWindowAC->WindAC(WindACNum).PartLoadFrac > 0.0 &&
            (state.dataWindowAC->WindAC(WindACNum).fanAvailSched->getCurrentVal() > 0.0 || state.dataHVACGlobal->TurnFansOn) &&
            !state.dataHVACGlobal->TurnFansOff) {

            Real64 NoCompOutput; // sensible load delivered with compressor off (W)
            CalcWindowACOutput(state, WindACNum, FirstHVACIteration, state.dataWindowAC->WindAC(WindACNum).fanOp, 0.0, false, NoCompOutput);

            Real64 QToCoolSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToCoolSP;

            // If the unit has a net heating capacity and the zone temp is below the Tstat cooling setpoint
            if (NoCompOutput > (-1.0 * HVAC::SmallLoad) && QToCoolSetPt > (-1.0 * HVAC::SmallLoad) &&
                state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) {
                if (NoCompOutput > QToCoolSetPt) {
                    QZnReq = QToCoolSetPt;
                    state.dataWindowAC->CoolingLoad = true;
                }
            }
        }
    }

    void SizeWindowAC(EnergyPlusData &state, int const WindACNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   January 2002
        //       MODIFIED       August 2013 Daeho Kang, add component sizing table entries
        //                      July 2014, B. Nigusse, added scalable sizing

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing Window AC  Unit components for which flow rates have not been
        // specified in the input.

        // METHODOLOGY EMPLOYED:
        // Obtains flow rates from the zone or system sizing arrays

        // Using/Aliasing
        using namespace DataSizing;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("SizeWindowAC: "); // include trailing blank space

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        auto &windowAC = state.dataWindowAC->WindAC(WindACNum);

        std::string const CompType = "ZoneHVAC:WindowAirConditioner"; // component name
        std::string const CompName = windowAC.Name;                   // component type
        Real64 TempSize = AutoSize;                                   // autosized value of coil input field
        bool PrintFlag = false;                                       // TRUE when sizing information is reported in the eio file

        state.dataSize->DataFracOfAutosizedCoolingAirflow = 1.0;
        state.dataSize->DataFracOfAutosizedHeatingAirflow = 1.0;
        state.dataSize->DataFracOfAutosizedCoolingCapacity = 1.0;
        state.dataSize->DataFracOfAutosizedHeatingCapacity = 1.0;
        state.dataSize->DataScalableSizingON = false;
        state.dataSize->ZoneHeatingOnlyFan = false;
        state.dataSize->ZoneCoolingOnlyFan = true;
        state.dataSize->DataScalableCapSizingON = false;
        state.dataSize->DataZoneNumber = windowAC.ZonePtr;
        state.dataSize->DataFanType = windowAC.fanType;
        state.dataSize->DataFanIndex = windowAC.FanIndex;
        state.dataSize->DataFanPlacement = windowAC.fanPlace;

        if (state.dataSize->CurZoneEqNum > 0) {
            auto &zoneEqSizing = state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum);

            if (windowAC.HVACSizingIndex > 0) {
                // N1 , \field Maximum Supply Air Flow Rate
                auto const &zoneHVACSizing = state.dataSize->ZoneHVACSizing(windowAC.HVACSizingIndex);

                int SizingMethod = HVAC::CoolingAirflowSizing;
                PrintFlag = true;
                int const SAFMethod = zoneHVACSizing.CoolingSAFMethod;
                zoneEqSizing.SizingMethod(SizingMethod) = SAFMethod;
                if (SAFMethod == None || SAFMethod == SupplyAirFlowRate || SAFMethod == FlowPerFloorArea ||
                    SAFMethod == FractionOfAutosizedCoolingAirflow) {
                    if (SAFMethod == SupplyAirFlowRate) {
                        if (zoneHVACSizing.MaxCoolAirVolFlow > 0.0) {
                            zoneEqSizing.AirVolFlow = zoneHVACSizing.MaxCoolAirVolFlow;
                            zoneEqSizing.SystemAirFlow = true;
                        }
                        TempSize = zoneHVACSizing.MaxCoolAirVolFlow;
                    } else if (SAFMethod == FlowPerFloorArea) {
                        zoneEqSizing.SystemAirFlow = true;
                        zoneEqSizing.AirVolFlow =
                            zoneHVACSizing.MaxCoolAirVolFlow * state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                        TempSize = zoneEqSizing.AirVolFlow;
                        state.dataSize->DataScalableSizingON = true;
                    } else if (SAFMethod == FractionOfAutosizedCoolingAirflow) {
                        state.dataSize->DataFracOfAutosizedCoolingAirflow = zoneHVACSizing.MaxCoolAirVolFlow;
                        TempSize = AutoSize;
                        state.dataSize->DataScalableSizingON = true;
                    } else {
                        TempSize = zoneHVACSizing.MaxCoolAirVolFlow;
                    }
                    bool errorsFound = false;
                    CoolingAirFlowSizer sizingCoolingAirFlow;
                    std::string stringOverride = "Maximum Supply Air Flow Rate [m3/s]";
                    if (state.dataGlobal->isEpJSON) {
                        stringOverride = "maximum_supply_air_flow_rate [m3/s]";
                    }
                    sizingCoolingAirFlow.overrideSizingString(stringOverride);
                    // sizingCoolingAirFlow.setHVACSizingIndexData(windowAC.HVACSizingIndex);
                    sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                    windowAC.MaxAirVolFlow = sizingCoolingAirFlow.size(state, TempSize, errorsFound);

                } else if (SAFMethod == FlowPerCoolingCapacity) {
                    SizingMethod = HVAC::CoolingCapacitySizing;
                    TempSize = AutoSize;
                    PrintFlag = false;
                    state.dataSize->DataScalableSizingON = true;
                    state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolVolFlow;
                    if (zoneHVACSizing.CoolingCapMethod == FractionOfAutosizedCoolingCapacity) {
                        state.dataSize->DataFracOfAutosizedCoolingCapacity = zoneHVACSizing.ScaledCoolingCapacity;
                    }
                    bool errorsFound = false;
                    CoolingCapacitySizer sizerCoolingCapacity;
                    sizerCoolingCapacity.overrideSizingString("");
                    sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                    state.dataSize->DataCapacityUsedForSizing = sizerCoolingCapacity.size(state, TempSize, errorsFound);
                    state.dataSize->DataFlowPerCoolingCapacity = zoneHVACSizing.MaxCoolAirVolFlow;
                    PrintFlag = true;
                    TempSize = AutoSize;
                    errorsFound = false;
                    CoolingAirFlowSizer sizingCoolingAirFlow;
                    std::string stringOverride = "Maximum Supply Air Flow Rate [m3/s]";
                    if (state.dataGlobal->isEpJSON) {
                        stringOverride = "maximum_supply_air_flow_rate [m3/s]";
                    }
                    sizingCoolingAirFlow.overrideSizingString(stringOverride);
                    // sizingCoolingAirFlow.setHVACSizingIndexData(windowAC.HVACSizingIndex);
                    sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                    windowAC.MaxAirVolFlow = sizingCoolingAirFlow.size(state, TempSize, errorsFound);
                }
                // DataScalableSizingON = false;

                // initialize capacity sizing variables: cooling
                // capacity sizing methods (HeatingDesignCapacity, CapacityPerFloorArea, FractionOfAutosizedCoolingCapacity, and
                // FractionOfAutosizedHeatingCapacity )
                int const CapSizingMethod = zoneHVACSizing.CoolingCapMethod;
                zoneEqSizing.SizingMethod(SizingMethod) = CapSizingMethod;
                if (CapSizingMethod == CoolingDesignCapacity || CapSizingMethod == CapacityPerFloorArea ||
                    CapSizingMethod == FractionOfAutosizedCoolingCapacity) {
                    if (CapSizingMethod == CoolingDesignCapacity) {
                        if (zoneHVACSizing.ScaledCoolingCapacity > 0.0) {
                            zoneEqSizing.CoolingCapacity = true;
                            zoneEqSizing.DesCoolingLoad = zoneHVACSizing.ScaledCoolingCapacity;
                        }
                    } else if (CapSizingMethod == CapacityPerFloorArea) {
                        zoneEqSizing.CoolingCapacity = true;
                        zoneEqSizing.DesCoolingLoad =
                            zoneHVACSizing.ScaledCoolingCapacity * state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                        state.dataSize->DataScalableCapSizingON = true;
                    } else if (CapSizingMethod == FractionOfAutosizedCoolingCapacity) {
                        state.dataSize->DataFracOfAutosizedCoolingCapacity = zoneHVACSizing.ScaledCoolingCapacity;
                        state.dataSize->DataScalableCapSizingON = true;
                    }
                }
            } else {
                // no scalable sizing method has been specified. Sizing proceeds using the method
                // specified in the zoneHVAC object
                // N1 , \field Maximum Supply Air Flow Rate
                int FieldNum = 1;
                PrintFlag = true;
                std::string stringOverride = state.dataWindowAC->WindACNumericFields(WindACNum).FieldNames(FieldNum) + " [m3/s]";
                TempSize = windowAC.MaxAirVolFlow;
                bool errorsFound = false;
                SystemAirFlowSizer sizerSystemAirFlow;
                sizerSystemAirFlow.overrideSizingString(stringOverride);
                // sizerSystemAirFlow.setHVACSizingIndexData(windowAC.HVACSizingIndex);
                sizerSystemAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                windowAC.MaxAirVolFlow = sizerSystemAirFlow.size(state, TempSize, errorsFound);
            }

            if (windowAC.OutAirVolFlow == AutoSize) {

                CheckZoneSizing(state, state.dataWindowAC->cWindowAC_UnitTypes(windowAC.UnitType), windowAC.Name);
                windowAC.OutAirVolFlow = min(state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).MinOA, windowAC.MaxAirVolFlow);
                if (windowAC.OutAirVolFlow < SmallAirVolFlow) {
                    windowAC.OutAirVolFlow = 0.0;
                }
                BaseSizer::reportSizerOutput(state,
                                             state.dataWindowAC->cWindowAC_UnitTypes(windowAC.UnitType),
                                             windowAC.Name,
                                             "Maximum Outdoor Air Flow Rate [m3/s]",
                                             windowAC.OutAirVolFlow);
            }

            zoneEqSizing.OAVolFlow = windowAC.OutAirVolFlow;
            zoneEqSizing.AirVolFlow = windowAC.MaxAirVolFlow;
            // Make the Fan be sized by this
            zoneEqSizing.CoolingAirFlow = true;
            zoneEqSizing.CoolingAirVolFlow = windowAC.MaxAirVolFlow;
        }

        state.dataSize->DataScalableCapSizingON = false;
    }

    void SimCyclingWindowAC(EnergyPlusData &state,
                            int const WindACNum,                // number of the current window AC unit being simulated
                            [[maybe_unused]] int const ZoneNum, // number of zone being served !unused1208
                            bool const FirstHVACIteration,      // TRUE if 1st HVAC simulation of system timestep
                            Real64 &PowerMet,                   // Sensible power supplied (W)
                            Real64 const QZnReq,                // Sensible load to be met (W)
                            Real64 &LatOutputProvided           // Latent power supplied (kg/s), negative = dehumidification
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   May 2000
        //       MODIFIED       Buhl/Shirey Mar 2001, Shirey Aug 2009 (LatOutputProvided)

        // PURPOSE OF THIS SUBROUTINE:
        // Simulate a cycling window air conditioner unit; adjust its output to match the
        // remaining zone load.

        // METHODOLOGY EMPLOYED:
        // If unit is on, calls ControlWindACOutput to obtain the desired unit output

        Real64 PartLoadFrac; // unit part load fraction
        bool HXUnitOn;       // Used to control HX heat recovery as needed

        // zero the DX coil electricity consumption

        state.dataHVACGlobal->DXElecCoolingPower = 0.0;
        // initialize local variables
        bool UnitOn = true;
        bool CoilOn = true;
        Real64 QUnitOut = 0.0;     // Dry air sens. cooling provided by AC unit [watts]
        Real64 LatentOutput = 0.0; // Latent (moisture) add/removal rate, negative is dehumidification [kg/s]
        int OutletNode = state.dataWindowAC->WindAC(WindACNum).AirOutNode;
        int InletNode = state.dataWindowAC->WindAC(WindACNum).AirInNode;
        Real64 AirMassFlow = state.dataLoopNodes->Node(InletNode).MassFlowRate;
        // Real64 CpAir = PsyCpAirFnW(state.dataLoopNodes->Node(InletNode).HumRat); // inlet air specific heat [J/kg-C]
        HVAC::FanOp fanOp = state.dataWindowAC->WindAC(WindACNum).fanOp;

        // set the on/off flags
        if (state.dataWindowAC->WindAC(WindACNum).fanOp == HVAC::FanOp::Cycling) {
            // cycling unit: only runs if there is a load.
            if (!state.dataWindowAC->CoolingLoad || AirMassFlow < SmallMassFlow) {
                UnitOn = false;
                CoilOn = false;
            }
        } else if (state.dataWindowAC->WindAC(WindACNum).fanOp == HVAC::FanOp::Continuous) {
            // continuous unit: fan runs if scheduled on; coil runs only if cooling load
            if (AirMassFlow < SmallMassFlow) {
                UnitOn = false;
                CoilOn = false;
            } else if (!state.dataWindowAC->CoolingLoad) {
                CoilOn = false;
            }
        }

        state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;

        if (UnitOn && CoilOn) {
            HXUnitOn = false;
            ControlCycWindACOutput(state, WindACNum, FirstHVACIteration, fanOp, QZnReq, PartLoadFrac, HXUnitOn);
        } else {
            PartLoadFrac = 0.0;
            HXUnitOn = false;
        }

        state.dataWindowAC->WindAC(WindACNum).PartLoadFrac = PartLoadFrac;

        CalcWindowACOutput(state, WindACNum, FirstHVACIteration, fanOp, PartLoadFrac, HXUnitOn, QUnitOut);

        // Reseting AirMassFlow to inlet node mass flow rate since inlet mass flow rate may be getting
        // manipulated in subroutine CalcWindowACOutput

        AirMassFlow = state.dataLoopNodes->Node(InletNode).MassFlowRate;
        Real64 MinHumRat = min(state.dataLoopNodes->Node(InletNode).HumRat, state.dataLoopNodes->Node(OutletNode).HumRat);
        QUnitOut = AirMassFlow * (PsyHFnTdbW(state.dataLoopNodes->Node(OutletNode).Temp, MinHumRat) -
                                  PsyHFnTdbW(state.dataLoopNodes->Node(InletNode).Temp, MinHumRat));

        Real64 SensCoolOut = AirMassFlow * (PsyHFnTdbW(state.dataLoopNodes->Node(OutletNode).Temp, MinHumRat) -
                                            PsyHFnTdbW(state.dataLoopNodes->Node(InletNode).Temp, MinHumRat));

        // CR9155 Remove specific humidity calculations
        Real64 SpecHumOut = state.dataLoopNodes->Node(OutletNode).HumRat;
        Real64 SpecHumIn = state.dataLoopNodes->Node(InletNode).HumRat;
        LatentOutput = AirMassFlow * (SpecHumOut - SpecHumIn); // Latent rate, kg/s

        Real64 QTotUnitOut = AirMassFlow * (state.dataLoopNodes->Node(OutletNode).Enthalpy - state.dataLoopNodes->Node(InletNode).Enthalpy);

        // report variables
        state.dataWindowAC->WindAC(WindACNum).CompPartLoadRatio = state.dataWindowAC->WindAC(WindACNum).PartLoadFrac;
        if (state.dataWindowAC->WindAC(WindACNum).fanOp == HVAC::FanOp::Cycling) {
            state.dataWindowAC->WindAC(WindACNum).FanPartLoadRatio = state.dataWindowAC->WindAC(WindACNum).PartLoadFrac;
        } else {
            if (UnitOn) {
                state.dataWindowAC->WindAC(WindACNum).FanPartLoadRatio = 1.0;
            } else {
                state.dataWindowAC->WindAC(WindACNum).FanPartLoadRatio = 0.0;
            }
        }
        state.dataWindowAC->WindAC(WindACNum).SensCoolEnergyRate = std::abs(min(0.0, SensCoolOut));
        state.dataWindowAC->WindAC(WindACNum).TotCoolEnergyRate = std::abs(min(0.0, QTotUnitOut));
        state.dataWindowAC->WindAC(WindACNum).SensCoolEnergyRate =
            min(state.dataWindowAC->WindAC(WindACNum).SensCoolEnergyRate, state.dataWindowAC->WindAC(WindACNum).TotCoolEnergyRate);
        state.dataWindowAC->WindAC(WindACNum).LatCoolEnergyRate =
            state.dataWindowAC->WindAC(WindACNum).TotCoolEnergyRate - state.dataWindowAC->WindAC(WindACNum).SensCoolEnergyRate;
        Real64 locFanElecPower = state.dataFans->fans(state.dataWindowAC->WindAC(WindACNum).FanIndex)->totalPower;
        state.dataWindowAC->WindAC(WindACNum).ElecPower = locFanElecPower + state.dataHVACGlobal->DXElecCoolingPower;

        PowerMet = QUnitOut;
        LatOutputProvided = LatentOutput;
    }

    void ReportWindowAC(EnergyPlusData &state, int const WindACNum) // number of the current AC unit being simulated
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   May 2000

        // PURPOSE OF THIS SUBROUTINE:
        // Fills some of the report variables for the window AC units

        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

        state.dataWindowAC->WindAC(WindACNum).SensCoolEnergy = state.dataWindowAC->WindAC(WindACNum).SensCoolEnergyRate * TimeStepSysSec;
        state.dataWindowAC->WindAC(WindACNum).TotCoolEnergy = state.dataWindowAC->WindAC(WindACNum).TotCoolEnergyRate * TimeStepSysSec;
        state.dataWindowAC->WindAC(WindACNum).LatCoolEnergy = state.dataWindowAC->WindAC(WindACNum).LatCoolEnergyRate * TimeStepSysSec;
        state.dataWindowAC->WindAC(WindACNum).ElecConsumption = state.dataWindowAC->WindAC(WindACNum).ElecPower * TimeStepSysSec;

        if (state.dataWindowAC->WindAC(WindACNum).FirstPass) { // reset sizing flags so other zone equipment can size normally
            if (!state.dataGlobal->SysSizingCalc) {
                DataSizing::resetHVACSizingGlobals(state, state.dataSize->CurZoneEqNum, 0, state.dataWindowAC->WindAC(WindACNum).FirstPass);
            }
        }
    }

    void CalcWindowACOutput(EnergyPlusData &state,
                            int const WindACNum,           // Unit index in fan coil array
                            bool const FirstHVACIteration, // flag for 1st HVAV iteration in the time step
                            HVAC::FanOp const fanOp,       // operating mode: FanOp::Cycling | FanOp::Continuous
                            Real64 const PartLoadFrac,     // unit part load fraction
                            bool const HXUnitOn,           // Flag to toggle HX heat recovery as needed
                            Real64 &LoadMet                // load met by unit (watts)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   May 2000
        //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers

        // PURPOSE OF THIS SUBROUTINE:
        // Simulate the components making up the cycling window AC unit.

        // METHODOLOGY EMPLOYED:
        // Simulates the unit components sequentially in the air flow direction.

        int OutletNode = state.dataWindowAC->WindAC(WindACNum).AirOutNode;
        int InletNode = state.dataWindowAC->WindAC(WindACNum).AirInNode;
        int OutsideAirNode = state.dataWindowAC->WindAC(WindACNum).OutsideAirNode;
        int AirRelNode = state.dataWindowAC->WindAC(WindACNum).AirReliefNode;
        // for cycling fans, pretend we have VAV
        if (fanOp == HVAC::FanOp::Cycling) {
            state.dataLoopNodes->Node(InletNode).MassFlowRate = state.dataLoopNodes->Node(InletNode).MassFlowRateMax * PartLoadFrac;
            // Don't let the outside air flow be > supply air flow
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate =
                min(state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMax, state.dataLoopNodes->Node(InletNode).MassFlowRate);
            state.dataLoopNodes->Node(AirRelNode).MassFlowRate = state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate;
        }
        Real64 AirMassFlow = state.dataLoopNodes->Node(InletNode).MassFlowRate;
        MixedAir::SimOAMixer(state, state.dataWindowAC->WindAC(WindACNum).OAMixName, state.dataWindowAC->WindAC(WindACNum).OAMixIndex);

        // if blow through, simulate fan then coil. For draw through, simulate coil then fan.
        if (state.dataWindowAC->WindAC(WindACNum).fanPlace == HVAC::FanPlace::BlowThru) {
            state.dataFans->fans(state.dataWindowAC->WindAC(WindACNum).FanIndex)->simulate(state, FirstHVACIteration, PartLoadFrac);
        }

        if (state.dataWindowAC->WindAC(WindACNum).DXCoilType_Num == CoilDX_CoolingHXAssisted) {
            HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(state,
                                                                state.dataWindowAC->WindAC(WindACNum).DXCoilName,
                                                                FirstHVACIteration,
                                                                HVAC::CompressorOp::On,
                                                                PartLoadFrac,
                                                                state.dataWindowAC->WindAC(WindACNum).DXCoilIndex,
                                                                state.dataWindowAC->WindAC(WindACNum).fanOp,
                                                                HXUnitOn);
        } else if (state.dataWindowAC->WindAC(WindACNum).DXCoilType_Num == HVAC::Coil_CoolingAirToAirVariableSpeed) {
            Real64 QZnReq(-1.0);           // Zone load (W), input to variable-speed DX coil
            Real64 QLatReq(0.0);           // Zone latent load, input to variable-speed DX coil
            Real64 OnOffAirFlowRatio(1.0); // ratio of compressor on flow to average flow over time step

            VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                      state.dataWindowAC->WindAC(WindACNum).DXCoilName,
                                                      state.dataWindowAC->WindAC(WindACNum).DXCoilIndex,
                                                      state.dataWindowAC->WindAC(WindACNum).fanOp,
                                                      HVAC::CompressorOp::On,
                                                      PartLoadFrac,
                                                      state.dataWindowAC->WindAC(WindACNum).DXCoilNumOfSpeeds,
                                                      1.0,
                                                      QZnReq,
                                                      QLatReq,
                                                      OnOffAirFlowRatio);

        } else {
            DXCoils::SimDXCoil(state,
                               state.dataWindowAC->WindAC(WindACNum).DXCoilName,
                               HVAC::CompressorOp::On,
                               FirstHVACIteration,
                               state.dataWindowAC->WindAC(WindACNum).DXCoilIndex,
                               state.dataWindowAC->WindAC(WindACNum).fanOp,
                               PartLoadFrac);
        }

        if (state.dataWindowAC->WindAC(WindACNum).fanPlace == HVAC::FanPlace::DrawThru) {
            state.dataFans->fans(state.dataWindowAC->WindAC(WindACNum).FanIndex)->simulate(state, FirstHVACIteration, PartLoadFrac);
        }

        Real64 MinHumRat = min(state.dataLoopNodes->Node(InletNode).HumRat, state.dataLoopNodes->Node(OutletNode).HumRat);
        LoadMet = AirMassFlow * (PsyHFnTdbW(state.dataLoopNodes->Node(OutletNode).Temp, MinHumRat) -
                                 PsyHFnTdbW(state.dataLoopNodes->Node(InletNode).Temp, MinHumRat));
    }

    void ControlCycWindACOutput(EnergyPlusData &state,
                                int const WindACNum,           // Unit index in fan coil array
                                bool const FirstHVACIteration, // flag for 1st HVAV iteration in the time step
                                HVAC::FanOp const fanOp,       // operating mode: FanOp::Cycling | FanOp::Continuous
                                Real64 const QZnReq,           // cooling output needed by zone [W]
                                Real64 &PartLoadFrac,          // unit part load fraction
                                bool &HXUnitOn                 // Used to control HX heat recovery as needed
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   May 2000
        //       MODIFIED       Shirey, May 2001

        // PURPOSE OF THIS SUBROUTINE:
        // Determine the part load fraction of the air conditioner for this time step

        // METHODOLOGY EMPLOYED:
        // Linear interpolation between max and min outputs

        int constexpr MaxIter(50);    // maximum number of iterations
        Real64 constexpr MinPLF(0.0); // minimum part load factor allowed

        Real64 FullOutput;   // unit full output [W]
        Real64 NoCoolOutput; // output when no active cooling [W]
        Real64 ActualOutput; // output at current partloadfrac [W]

        // DX Cooling HX assisted coils can cycle the heat exchanger, see if coil ON, HX OFF can meet humidity setpoint if one exists
        if (state.dataWindowAC->WindAC(WindACNum).DXCoilType_Num == CoilDX_CoolingHXAssisted) {
            // Check for a setpoint at the HX outlet node, if it doesn't exist always run the HX
            if (state.dataLoopNodes->Node(state.dataWindowAC->WindAC(WindACNum).CoilOutletNodeNum).HumRatMax == SensedNodeFlagValue) {
                HXUnitOn = true;
            } else {
                HXUnitOn = false;
            }
        } else {
            HXUnitOn = false;
        }

        if (state.dataWindowAC->WindAC(WindACNum).EMSOverridePartLoadFrac) {

            PartLoadFrac = state.dataWindowAC->WindAC(WindACNum).EMSValueForPartLoadFrac;
        }

        // Get result when DX coil is off
        CalcWindowACOutput(state, WindACNum, FirstHVACIteration, fanOp, 0.0, HXUnitOn, NoCoolOutput);

        // If NoCoolOutput < QZnReq, the coil needs to be off
        if (NoCoolOutput < QZnReq) {
            PartLoadFrac = 0.0;
            return;
        }

        // Get full load result
        CalcWindowACOutput(state, WindACNum, FirstHVACIteration, fanOp, 1.0, HXUnitOn, FullOutput);

        // Since we are cooling, we expect FullOutput to be < 0 and FullOutput < NoCoolOutput
        // Check that this is the case; if not set PartLoadFrac = 0.0 (off) and return
        if (FullOutput >= 0.0 || FullOutput >= NoCoolOutput) {
            PartLoadFrac = 0.0;
            return;
        }

        // If the QZnReq <= FullOutput the unit needs to run full out
        if (QZnReq <= FullOutput && state.dataWindowAC->WindAC(WindACNum).DXCoilType_Num != CoilDX_CoolingHXAssisted) {
            PartLoadFrac = 1.0;
            return;
        }

        // If the QZnReq <= FullOutput and a HXAssisted coil is used, check the node setpoint for a maximum humidity ratio set point
        // HumRatMax will be equal to -999 if no setpoint exists or some set point managers may still use 0 as a no moisture load indicator
        if (QZnReq <= FullOutput && state.dataWindowAC->WindAC(WindACNum).DXCoilType_Num == CoilDX_CoolingHXAssisted &&
            state.dataLoopNodes->Node(state.dataWindowAC->WindAC(WindACNum).CoilOutletNodeNum).HumRatMax <= 0.0) {
            PartLoadFrac = 1.0;
            return;
        }

        // QZnReq should now be greater than FullOutput and less than NoCoolOutput)
        // Calculate the part load fraction

        PartLoadFrac = max(MinPLF, std::abs(QZnReq - NoCoolOutput) / std::abs(FullOutput - NoCoolOutput));

        Real64 ErrorToler = state.dataWindowAC->WindAC(WindACNum).ConvergenceTol; // Error tolerance for convergence from input deck
        Real64 Error = 1.0;                                                       // initialize error value for comparison against tolerance
        int Iter = 0;                                                             // initialize iteration counter
        Real64 Relax = 1.0;

        while ((std::abs(Error) > ErrorToler) && (Iter <= MaxIter) && PartLoadFrac > MinPLF) {
            // Get result when DX coil is operating at partloadfrac
            CalcWindowACOutput(state, WindACNum, FirstHVACIteration, fanOp, PartLoadFrac, HXUnitOn, ActualOutput);
            Error = (QZnReq - ActualOutput) / QZnReq;
            Real64 DelPLF = (QZnReq - ActualOutput) / FullOutput;
            PartLoadFrac += Relax * DelPLF;
            PartLoadFrac = max(MinPLF, min(1.0, PartLoadFrac));
            ++Iter;
            if (Iter == 16) {
                Relax = 0.5;
            }
        }
        if (Iter > MaxIter) {
            if (state.dataWindowAC->WindAC(WindACNum).MaxIterIndex1 == 0) {
                ShowWarningMessage(state,
                                   format("ZoneHVAC:WindowAirConditioner=\"{}\" -- Exceeded max iterations while adjusting compressor sensible "
                                          "runtime to meet the zone load within the cooling convergence tolerance.",
                                          state.dataWindowAC->WindAC(WindACNum).Name));
                ShowContinueErrorTimeStamp(state, format("Iterations={}", MaxIter));
            }
            ShowRecurringWarningErrorAtEnd(state,
                                           "ZoneHVAC:WindowAirConditioner=\"" + state.dataWindowAC->WindAC(WindACNum).Name +
                                               "\"  -- Exceeded max iterations error (sensible runtime) continues...",
                                           state.dataWindowAC->WindAC(WindACNum).MaxIterIndex1);
        }

        // HX is off up until this point where the outlet air humidity ratio is tested to see if HX needs to be turned on
        if (state.dataWindowAC->WindAC(WindACNum).DXCoilType_Num == CoilDX_CoolingHXAssisted &&
            state.dataLoopNodes->Node(state.dataWindowAC->WindAC(WindACNum).CoilOutletNodeNum).HumRatMax <
                state.dataLoopNodes->Node(state.dataWindowAC->WindAC(WindACNum).CoilOutletNodeNum).HumRat &&
            state.dataLoopNodes->Node(state.dataWindowAC->WindAC(WindACNum).CoilOutletNodeNum).HumRatMax > 0.0) {

            //   Run the HX to recovery energy and improve latent performance
            HXUnitOn = true;

            //   Get full load result
            CalcWindowACOutput(state, WindACNum, FirstHVACIteration, fanOp, 1.0, HXUnitOn, FullOutput);

            if (state.dataLoopNodes->Node(state.dataWindowAC->WindAC(WindACNum).CoilOutletNodeNum).HumRatMax <
                    state.dataLoopNodes->Node(state.dataWindowAC->WindAC(WindACNum).CoilOutletNodeNum).HumRat ||
                QZnReq <= FullOutput) {
                PartLoadFrac = 1.0;
                return;
            }

            Error = 1.0; // initialize error value for comparison against tolerance
            Iter = 0;    // initialize iteration counter
            Relax = 1.0;

            while ((std::abs(Error) > ErrorToler) && (Iter <= MaxIter) && PartLoadFrac > MinPLF) {
                // Get result when DX coil is operating at partloadfrac
                CalcWindowACOutput(state, WindACNum, FirstHVACIteration, fanOp, PartLoadFrac, HXUnitOn, ActualOutput);
                Error = (QZnReq - ActualOutput) / QZnReq;
                Real64 DelPLF = (QZnReq - ActualOutput) / FullOutput;
                PartLoadFrac += Relax * DelPLF;
                PartLoadFrac = max(MinPLF, min(1.0, PartLoadFrac));
                ++Iter;
                if (Iter == 16) {
                    Relax = 0.5;
                }
            }
            if (Iter > MaxIter) {
                if (state.dataWindowAC->WindAC(WindACNum).MaxIterIndex2 == 0) {
                    ShowWarningMessage(state,
                                       format("ZoneHVAC:WindowAirConditioner=\"{}\" -- Exceeded max iterations while adjusting compressor latent "
                                              "runtime to meet the zone load within the cooling convergence tolerance.",
                                              state.dataWindowAC->WindAC(WindACNum).Name));
                    ShowContinueErrorTimeStamp(state, format("Iterations={}", MaxIter));
                }
                ShowRecurringWarningErrorAtEnd(state,
                                               "ZoneHVAC:WindowAirConditioner=\"" + state.dataWindowAC->WindAC(WindACNum).Name +
                                                   "\"  -- Exceeded max iterations error (latent runtime) continues...",
                                               state.dataWindowAC->WindAC(WindACNum).MaxIterIndex2);
            }

        } // WindAC(WindACNum)%DXCoilType_Num == CoilDX_CoolingHXAssisted && *
    }

    bool getWindowACNodeNumber(EnergyPlusData &state, int const nodeNumber)
    {
        if (state.dataWindowAC->GetWindowACInputFlag) {
            GetWindowAC(state);
            state.dataWindowAC->GetWindowACInputFlag = false;
        }

        for (int windowACIndex = 1; windowACIndex <= state.dataWindowAC->NumWindAC; ++windowACIndex) {
            auto &windowAC = state.dataWindowAC->WindAC(windowACIndex);
            int FanInletNodeIndex = state.dataFans->fans(windowAC.FanIndex)->inletNodeNum;
            int FanOutletNodeIndex = state.dataFans->fans(windowAC.FanIndex)->outletNodeNum;

            if (windowAC.OutAirVolFlow == 0 &&
                (nodeNumber == windowAC.OutsideAirNode || nodeNumber == windowAC.MixedAirNode || nodeNumber == windowAC.AirReliefNode ||
                 nodeNumber == FanInletNodeIndex || nodeNumber == FanOutletNodeIndex || nodeNumber == windowAC.AirInNode ||
                 nodeNumber == windowAC.CoilOutletNodeNum || nodeNumber == windowAC.AirOutNode || nodeNumber == windowAC.ReturnAirNode)) {
                return true;
            }
        }
        return false;
    }

    int GetWindowACZoneInletAirNode(EnergyPlusData &state, int const WindACNum)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   Dec  2006

        // PURPOSE OF THIS FUNCTION:
        // lookup function for zone inlet node

        // Return value
        int GetWindowACZoneInletAirNode;

        if (state.dataWindowAC->GetWindowACInputFlag) {
            GetWindowAC(state);
            state.dataWindowAC->GetWindowACInputFlag = false;
        }

        GetWindowACZoneInletAirNode = state.dataWindowAC->WindAC(WindACNum).AirOutNode;

        return GetWindowACZoneInletAirNode;
    }

    int GetWindowACOutAirNode(EnergyPlusData &state, int const WindACNum)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   Dec  2006

        // PURPOSE OF THIS FUNCTION:
        // lookup function for OA inlet node

        if (state.dataWindowAC->GetWindowACInputFlag) {
            GetWindowAC(state);
            state.dataWindowAC->GetWindowACInputFlag = false;
        }

        return state.dataWindowAC->WindAC(WindACNum).OutsideAirNode;
    }

    int GetWindowACReturnAirNode(EnergyPlusData &state, int const WindACNum)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   Dec  2006

        // PURPOSE OF THIS FUNCTION:
        // lookup function for mixer return air node for ventilation load reporting

        // Return value
        int GetWindowACReturnAirNode;

        if (state.dataWindowAC->GetWindowACInputFlag) {
            GetWindowAC(state);
            state.dataWindowAC->GetWindowACInputFlag = false;
        }

        if (WindACNum > 0 && WindACNum <= state.dataWindowAC->NumWindAC) {
            if (state.dataWindowAC->WindAC(WindACNum).OAMixIndex > 0) {
                GetWindowACReturnAirNode = MixedAir::GetOAMixerReturnNodeNumber(state, state.dataWindowAC->WindAC(WindACNum).OAMixIndex);
            } else {
                GetWindowACReturnAirNode = 0;
            }
        } else {
            GetWindowACReturnAirNode = 0;
        }

        return GetWindowACReturnAirNode;
    }

    int GetWindowACMixedAirNode(EnergyPlusData &state, int const WindACNum)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   Dec  2006

        // PURPOSE OF THIS FUNCTION:
        // lookup function for mixed air node for ventilation rate reporting

        // Return value
        int GetWindowACMixedAirNode;

        if (state.dataWindowAC->GetWindowACInputFlag) {
            GetWindowAC(state);
            state.dataWindowAC->GetWindowACInputFlag = false;
        }

        if (WindACNum > 0 && WindACNum <= state.dataWindowAC->NumWindAC) {
            if (state.dataWindowAC->WindAC(WindACNum).OAMixIndex > 0) {
                GetWindowACMixedAirNode = MixedAir::GetOAMixerMixedNodeNumber(state, state.dataWindowAC->WindAC(WindACNum).OAMixIndex);
            } else {
                GetWindowACMixedAirNode = 0;
            }
        } else {
            GetWindowACMixedAirNode = 0;
        }

        return GetWindowACMixedAirNode;
    }

    int getWindowACIndex(EnergyPlusData &state, std::string_view CompName)
    {
        if (state.dataWindowAC->GetWindowACInputFlag) {
            GetWindowAC(state);
            state.dataWindowAC->GetWindowACInputFlag = false;
        }

        for (int WindACIndex = 1; WindACIndex <= state.dataWindowAC->NumWindAC; ++WindACIndex) {
            if (Util::SameString(state.dataWindowAC->WindAC(WindACIndex).Name, CompName)) {
                return WindACIndex;
            }
        }

        return 0;
    }

} // namespace WindowAC

} // namespace EnergyPlus

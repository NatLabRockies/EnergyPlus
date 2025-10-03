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
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataIPShortCuts.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/DesiccantDehumidifiers.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HVACDXHeatPumpSystem.hh>
#include <EnergyPlus/HVACHXAssistedCoolingCoil.hh>
#include <EnergyPlus/HeatRecovery.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutdoorAirUnit.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SteamCoils.hh>
#include <EnergyPlus/UnitarySystem.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterCoils.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace OutdoorAirUnit {
    // Module containing the routines dealing with the outdoor air unit

    // MODULE INFORMATION:
    //       AUTHOR         Young Tae Chae, Rick Strand
    //       DATE WRITTEN   AUG. 2009
    //       MODIFIED
    //                      Feb 2013 Bereket Nigusse, FSEC
    //                        Added DX Coil Model For 100% OA systems
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // Simulate zone outdoor air unit.

    // METHODOLOGY EMPLOYED:
    // Systems are modeled as a collection of components:
    // fan, heat recovery, dehumidifier, heating coil and/or cooling coil plus an integrated control
    // algorithm that adjusts the hot or cold water flow to meet the setpoint
    // condition.

    // Using/Aliasing
    using namespace DataLoopNode;
    using HVAC::SmallAirVolFlow;
    using HVAC::SmallLoad;
    using HVAC::SmallMassFlow;
    using namespace Psychrometrics;

    // component types addressed by this module
    constexpr static std::string_view ZoneHVACOAUnit = {"ZoneHVAC:OutdoorAirUnit"};
    constexpr static std::string_view ZoneHVACEqList = {"ZoneHVAC:OutdoorAirUnit:EquipmentList"};

    void SimOutdoorAirUnit(EnergyPlusData &state,
                           std::string_view CompName,     // name of the outdoor air unit
                           int const ZoneNum,             // number of zone being served
                           bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                           Real64 &PowerMet,              // Sensible power supplied (W)
                           Real64 &LatOutputProvided,     // Latent add/removal supplied by window AC (kg/s), dehumid = negative
                           int &CompIndex)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   May 2000
        //       MODIFIED       na
        //       RE-ENGINEERED
        // This is re-engineered by Rick Strand and Young T. Chae for OutdoorAirUnit (July, 2009)

        // PURPOSE OF THIS SUBROUTINE:
        // This is the main driver subroutine for the outdoor air control unit simulation.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int OAUnitNum = 0; // index of outdoor air unit being simulated

        if (state.dataOutdoorAirUnit->GetOutdoorAirUnitInputFlag) {
            GetOutdoorAirUnitInputs(state);
            state.dataOutdoorAirUnit->GetOutdoorAirUnitInputFlag = false;
        }

        // Find the correct Outdoor Air Unit

        if (CompIndex == 0) {
            OAUnitNum = Util::FindItemInList(CompName, state.dataOutdoorAirUnit->OutAirUnit);
            if (OAUnitNum == 0) {
                ShowFatalError(state, format("ZoneHVAC:OutdoorAirUnit not found={}", CompName));
            }
            CompIndex = OAUnitNum;
        } else {
            OAUnitNum = CompIndex;
            if (OAUnitNum > state.dataOutdoorAirUnit->NumOfOAUnits || OAUnitNum < 1) {
                ShowFatalError(state,
                               format("SimOutdoorAirUnit:  Invalid CompIndex passed={}, Number of Units={}, Entered Unit name={}",
                                      OAUnitNum,
                                      state.dataOutdoorAirUnit->NumOfOAUnits,
                                      CompName));
            }
            if (state.dataOutdoorAirUnit->CheckEquipName(OAUnitNum)) {
                if (CompName != state.dataOutdoorAirUnit->OutAirUnit(OAUnitNum).Name) {
                    ShowFatalError(state,
                                   format("SimOutdoorAirUnit: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                          OAUnitNum,
                                          CompName,
                                          state.dataOutdoorAirUnit->OutAirUnit(OAUnitNum).Name));
                }
                state.dataOutdoorAirUnit->CheckEquipName(OAUnitNum) = false;
            }
        }

        state.dataSize->ZoneEqOutdoorAirUnit = true;

        if (state.dataGlobal->ZoneSizingCalc || state.dataGlobal->SysSizingCalc) {
            return;
        }

        InitOutdoorAirUnit(state, OAUnitNum, ZoneNum, FirstHVACIteration);

        CalcOutdoorAirUnit(state, OAUnitNum, ZoneNum, FirstHVACIteration, PowerMet, LatOutputProvided);

        ReportOutdoorAirUnit(state, OAUnitNum);

        state.dataSize->ZoneEqOutdoorAirUnit = false;
    }

    void GetOutdoorAirUnitInputs(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Young Tae Chae, Rick Strand
        //       DATE WRITTEN   July 2009
        //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine obtains the input for the outdoor air control unit and sets
        // up the appropriate derived type.

        // METHODOLOGY EMPLOYED:
        // Standard EnergyPlus methodology.

        // REFERENCES:
        // Fred Buhl's fan coil module (FanCoilUnits.cc)
        // Kwang Ho Lee's Unit Ventilator Module (UnitVentilator.cc)
        // Young Tae Chae's Ventilated Slab System (VentilatedSlab.cc)
        // Mixed Air.cc

        // Using/Aliasing
        using BranchNodeConnections::SetUpCompSets;
        using BranchNodeConnections::TestCompSet;
        using NodeInputManager::GetOnlySingleNode;
        using SteamCoils::GetCoilAirInletNode;
        using SteamCoils::GetCoilAirOutletNode;
        using SteamCoils::GetCoilMaxSteamFlowRate;
        using SteamCoils::GetCoilSteamInletNode;
        using SteamCoils::GetCoilSteamOutletNode;
        using SteamCoils::GetSteamCoilIndex;
        using namespace DataLoopNode;
        using HeatingCoils::GetCoilInletNode;
        using HeatingCoils::GetCoilOutletNode;
        using OutAirNodeManager::CheckAndAddAirNodeNumber;
        using WaterCoils::GetCoilWaterInletNode;
        using WaterCoils::GetCoilWaterOutletNode;
        using WaterCoils::GetWaterCoilIndex;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetOutdoorAirUnitInputs: "); // include trailing blank space
        static constexpr std::string_view routineName = "GetOutdoorAirUnitInputs";  // include trailing blank space

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        if (!state.dataOutdoorAirUnit->GetOutdoorAirUnitInputFlag) {
            return;
        }

        int NumAlphas = 0;        // Number of elements in the alpha array
        int NumNums = 0;          // Number of elements in the numeric array
        Array1D_string AlphArray; // character string data
        Array1D<Real64> NumArray; // numeric data
        int IOStat = -1;          // IO Status when calling get input subroutine
        bool ErrorsFound = false;

        int MaxNums = 0;                 // Maximum number of numeric input fields
        int MaxAlphas = 0;               // Maximum number of alpha input fields
        int TotalArgs = 0;               // Total number of alpha and numeric arguments (max) for a
        bool IsValid;                    // Set for outside air node check
        Array1D_string cAlphaArgs;       // Alpha input items for object
        std::string CurrentModuleObject; // Object type for getting and messages
        Array1D_string cAlphaFields;     // Alpha field names
        Array1D_string cNumericFields;   // Numeric field names
        Array1D_bool lAlphaBlanks;       // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;     // Logical array, numeric field input BLANK = .TRUE.

        // Figure out how many outdoor air units there are in the input file

        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, ZoneHVACOAUnit, TotalArgs, NumAlphas, NumNums);
        MaxNums = max(MaxNums, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, ZoneHVACEqList, TotalArgs, NumAlphas, NumNums);
        MaxNums = max(MaxNums, NumNums);
        MaxAlphas = max(MaxAlphas, NumAlphas);

        AlphArray.allocate(MaxAlphas);
        cAlphaFields.allocate(MaxAlphas);
        NumArray.dimension(MaxNums, 0.0);
        cNumericFields.allocate(MaxNums);
        lAlphaBlanks.dimension(MaxAlphas, true);
        lNumericBlanks.dimension(MaxNums, true);
        cAlphaArgs.allocate(NumAlphas);

        CurrentModuleObject = ZoneHVACOAUnit;
        state.dataOutdoorAirUnit->NumOfOAUnits = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);

        state.dataOutdoorAirUnit->OutAirUnit.allocate(state.dataOutdoorAirUnit->NumOfOAUnits);
        state.dataOutdoorAirUnit->SupplyFanUniqueNames.reserve(static_cast<unsigned>(state.dataOutdoorAirUnit->NumOfOAUnits));
        state.dataOutdoorAirUnit->ExhaustFanUniqueNames.reserve(static_cast<unsigned>(state.dataOutdoorAirUnit->NumOfOAUnits));
        state.dataOutdoorAirUnit->ComponentListUniqueNames.reserve(static_cast<unsigned>(state.dataOutdoorAirUnit->NumOfOAUnits));
        state.dataOutdoorAirUnit->MyOneTimeErrorFlag.dimension(state.dataOutdoorAirUnit->NumOfOAUnits, true);
        state.dataOutdoorAirUnit->CheckEquipName.dimension(state.dataOutdoorAirUnit->NumOfOAUnits, true);

        for (int OAUnitNum = 1; OAUnitNum <= state.dataOutdoorAirUnit->NumOfOAUnits; ++OAUnitNum) {

            auto &thisOutAirUnit = state.dataOutdoorAirUnit->OutAirUnit(OAUnitNum);

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     OAUnitNum,
                                                                     state.dataIPShortCut->cAlphaArgs,
                                                                     NumAlphas,
                                                                     NumArray,
                                                                     NumNums,
                                                                     IOStat,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1)};
            Util::IsNameEmpty(state, state.dataIPShortCut->cAlphaArgs(1), CurrentModuleObject, ErrorsFound);

            // A1
            thisOutAirUnit.Name = state.dataIPShortCut->cAlphaArgs(1);

            // A2
            if (lAlphaBlanks(2)) {
                thisOutAirUnit.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((thisOutAirUnit.availSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaArgs(2), state.dataIPShortCut->cAlphaArgs(2));
                ErrorsFound = true;
            }

            // A3
            thisOutAirUnit.ZoneName = state.dataIPShortCut->cAlphaArgs(3);
            thisOutAirUnit.ZonePtr = Util::FindItemInList(state.dataIPShortCut->cAlphaArgs(3), state.dataHeatBal->Zone);

            if (thisOutAirUnit.ZonePtr == 0) {
                if (lAlphaBlanks(3)) {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {} is required but input is blank.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           state.dataIPShortCut->cAlphaArgs(3)));
                } else {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {}=\"{}\" not found.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           state.dataIPShortCut->cAlphaArgs(3),
                                           state.dataIPShortCut->cAlphaArgs(3)));
                }
                ErrorsFound = true;
            }
            thisOutAirUnit.ZoneNodeNum = state.dataHeatBal->Zone(thisOutAirUnit.ZonePtr).SystemZoneNodeNumber;
            // Outside air information:
            // N1
            thisOutAirUnit.OutAirVolFlow = NumArray(1);
            // A4
            if (lAlphaBlanks(4)) {
                ShowSevereEmptyField(state, eoh, cAlphaFields(4));
                ErrorsFound = true;
            } else if ((thisOutAirUnit.outAirSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(4))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(4), state.dataIPShortCut->cAlphaArgs(4));
                ErrorsFound = true;
            }

            // A5
            thisOutAirUnit.SFanName = state.dataIPShortCut->cAlphaArgs(5);
            GlobalNames::IntraObjUniquenessCheck(state,
                                                 state.dataIPShortCut->cAlphaArgs(5),
                                                 CurrentModuleObject,
                                                 cAlphaFields(5),
                                                 state.dataOutdoorAirUnit->SupplyFanUniqueNames,
                                                 ErrorsFound);

            if ((thisOutAirUnit.SFan_Index = Fans::GetFanIndex(state, thisOutAirUnit.SFanName)) == 0) {
                ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(5), thisOutAirUnit.SFanName);
                ErrorsFound = true;
            } else {
                auto *fan = state.dataFans->fans(thisOutAirUnit.SFan_Index);
                thisOutAirUnit.supFanType = fan->type;
                thisOutAirUnit.SFanMaxAirVolFlow = fan->maxAirFlowRate;
                thisOutAirUnit.supFanAvailSched = fan->availSched;
            }
            // A6 :Fan Place
            thisOutAirUnit.supFanPlace = static_cast<HVAC::FanPlace>(getEnumValue(HVAC::fanPlaceNamesUC, state.dataIPShortCut->cAlphaArgs(6)));

            // A7

            if (lAlphaBlanks(7)) {
                thisOutAirUnit.ExtFan = false;
                if (!state.dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance) {
                    ShowWarningError(state,
                                     format("{}=\"{}\", {} is blank.", CurrentModuleObject, state.dataIPShortCut->cAlphaArgs(1), cAlphaFields(7)));
                    ShowContinueError(state,
                                      "Unbalanced mass flow rates between supply from outdoor air and exhaust from zone air will be introduced.");
                }
            } else if (!lAlphaBlanks(7)) {
                thisOutAirUnit.ExtFanName = state.dataIPShortCut->cAlphaArgs(7);
                GlobalNames::IntraObjUniquenessCheck(state,
                                                     state.dataIPShortCut->cAlphaArgs(7),
                                                     CurrentModuleObject,
                                                     cAlphaFields(7),
                                                     state.dataOutdoorAirUnit->ExhaustFanUniqueNames,
                                                     ErrorsFound);

                if ((thisOutAirUnit.ExtFan_Index = Fans::GetFanIndex(state, thisOutAirUnit.ExtFanName)) == 0) {
                    ShowSevereItemNotFound(state, eoh, state.dataIPShortCut->cAlphaFieldNames(7), thisOutAirUnit.ExtFanName);
                    ErrorsFound = true;
                } else {
                    auto *fan = state.dataFans->fans(thisOutAirUnit.ExtFan_Index);
                    thisOutAirUnit.extFanType = fan->type;
                    thisOutAirUnit.EFanMaxAirVolFlow = fan->maxAirFlowRate;
                    thisOutAirUnit.extFanAvailSched = fan->availSched;
                }
                thisOutAirUnit.ExtFan = true;
            }

            // N2
            thisOutAirUnit.ExtAirVolFlow = NumArray(2);
            if ((thisOutAirUnit.ExtFan) && (!state.dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance)) {
                if (NumArray(2) != NumArray(1)) {
                    ShowWarningError(state,
                                     format("{}=\"{}\", {} and {} are not equal. This may cause unbalanced flow.",
                                            CurrentModuleObject,
                                            state.dataIPShortCut->cAlphaArgs(1),
                                            cNumericFields(1),
                                            cNumericFields(2)));
                    ShowContinueError(state, format("{}={:.3R}= and {}{:.3R}", cNumericFields(1), NumArray(1), cNumericFields(2), NumArray(2)));
                }
            }
            // A8

            // convert schedule name to pointer
            if (thisOutAirUnit.ExtFan) {
                if (lAlphaBlanks(8)) {
                    ShowSevereEmptyField(state, eoh, cAlphaFields(8));
                    ErrorsFound = true;
                } else if ((thisOutAirUnit.extAirSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(8))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(8), state.dataIPShortCut->cAlphaArgs(8));
                    ErrorsFound = true;
                } else if ((thisOutAirUnit.extAirSched != thisOutAirUnit.outAirSched) &&
                           (!state.dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance)) {
                    ShowWarningError(
                        state,
                        format("{}=\"{}\", different schedule inputs for outdoor air and exhaust air schedules may cause unbalanced mass flow.",
                               CurrentModuleObject,
                               state.dataIPShortCut->cAlphaArgs(1)));
                    ShowContinueError(state,
                                      format("{}={} and {}={}",
                                             cAlphaFields(4),
                                             state.dataIPShortCut->cAlphaArgs(4),
                                             cAlphaFields(8),
                                             state.dataIPShortCut->cAlphaArgs(8)));
                }

                SetUpCompSets(
                    state, CurrentModuleObject, thisOutAirUnit.Name, "UNDEFINED", state.dataIPShortCut->cAlphaArgs(7), "UNDEFINED", "UNDEFINED");
            }

            // Process the unit control type
            if (lAlphaBlanks(9)) {
                ShowWarningEmptyField(state, eoh, cAlphaFields(9), "Control reset to Unconditioned Control.");
                thisOutAirUnit.controlType = OAUnitCtrlType::Neutral;
            } else {
                constexpr std::array<std::string_view, (int)OAUnitCtrlType::Num> ctrlTypeNamesUC = {
                    "NEUTRALCONTROL", "INVALID-UNCONDITIONED", "TEMPERATURECONTROL"};
                OAUnitCtrlType tmpCtrlType = static_cast<OAUnitCtrlType>(getEnumValue(ctrlTypeNamesUC, state.dataIPShortCut->cAlphaArgs(9)));
                if (tmpCtrlType == OAUnitCtrlType::Invalid) {
                    ShowWarningEmptyField(state, eoh, cAlphaFields(9), "Control reset to Unconditioned Control.");
                } else if (tmpCtrlType == OAUnitCtrlType::Neutral || tmpCtrlType == OAUnitCtrlType::Temperature) {
                    thisOutAirUnit.controlType = tmpCtrlType;
                }
            }

            // A10:High Control Temp :
            if (lAlphaBlanks(10)) {
            } else if ((thisOutAirUnit.hiCtrlTempSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(10))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(10), state.dataIPShortCut->cAlphaArgs(10));
                ErrorsFound = true;
            }

            // A11:Low Control Temp :
            if (lAlphaBlanks(11)) {
            } else if ((thisOutAirUnit.loCtrlTempSched = Sched::GetSchedule(state, state.dataIPShortCut->cAlphaArgs(11))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(11), state.dataIPShortCut->cAlphaArgs(11));
                ErrorsFound = true;
            }

            thisOutAirUnit.CompOutSetTemp = 0.0;

            // A12~A15 : Node Condition

            // Main air nodes (except outside air node):

            thisOutAirUnit.AirOutletNode = GetOnlySingleNode(state,
                                                             state.dataIPShortCut->cAlphaArgs(13),
                                                             ErrorsFound,
                                                             DataLoopNode::ConnectionObjectType::ZoneHVACOutdoorAirUnit,
                                                             state.dataIPShortCut->cAlphaArgs(1),
                                                             DataLoopNode::NodeFluidType::Air,
                                                             DataLoopNode::ConnectionType::Outlet,
                                                             NodeInputManager::CompFluidStream::Primary,
                                                             ObjectIsParent);
            if (!lAlphaBlanks(14)) {
                thisOutAirUnit.AirInletNode = GetOnlySingleNode(state,
                                                                state.dataIPShortCut->cAlphaArgs(14),
                                                                ErrorsFound,
                                                                DataLoopNode::ConnectionObjectType::ZoneHVACOutdoorAirUnit,
                                                                state.dataIPShortCut->cAlphaArgs(1),
                                                                DataLoopNode::NodeFluidType::Air,
                                                                DataLoopNode::ConnectionType::Inlet,
                                                                NodeInputManager::CompFluidStream::Primary,
                                                                ObjectIsParent);
            } else {
                if (thisOutAirUnit.ExtFan) {
                    ShowSevereError(state,
                                    format("{}=\"{}\" invalid {} cannot be blank when there is an exhaust fan.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cAlphaFields(14)));
                    ErrorsFound = true;
                }
            }

            thisOutAirUnit.SFanOutletNode = GetOnlySingleNode(state,
                                                              state.dataIPShortCut->cAlphaArgs(15),
                                                              ErrorsFound,
                                                              DataLoopNode::ConnectionObjectType::ZoneHVACOutdoorAirUnit,
                                                              state.dataIPShortCut->cAlphaArgs(1),
                                                              DataLoopNode::NodeFluidType::Air,
                                                              DataLoopNode::ConnectionType::Internal,
                                                              NodeInputManager::CompFluidStream::Primary,
                                                              ObjectIsNotParent);

            //  Set connection type to 'OutdoorAir', because this is hardwired to OA conditions
            thisOutAirUnit.OutsideAirNode = GetOnlySingleNode(state,
                                                              state.dataIPShortCut->cAlphaArgs(12),
                                                              ErrorsFound,
                                                              DataLoopNode::ConnectionObjectType::ZoneHVACOutdoorAirUnit,
                                                              state.dataIPShortCut->cAlphaArgs(1),
                                                              DataLoopNode::NodeFluidType::Air,
                                                              DataLoopNode::ConnectionType::OutsideAirReference,
                                                              NodeInputManager::CompFluidStream::Primary,
                                                              ObjectIsNotParent);

            if (!lAlphaBlanks(12)) {
                CheckAndAddAirNodeNumber(state, thisOutAirUnit.OutsideAirNode, IsValid);
                if (!IsValid) {
                    ShowWarningError(state,
                                     format("{}=\"{}\", Adding OutdoorAir:Node={}",
                                            CurrentModuleObject,
                                            state.dataIPShortCut->cAlphaArgs(1),
                                            state.dataIPShortCut->cAlphaArgs(12)));
                }
            }

            // When the fan position is "BlowThru", Each node is set up

            if (thisOutAirUnit.supFanPlace == HVAC::FanPlace::BlowThru) {
                SetUpCompSets(state,
                              CurrentModuleObject,
                              thisOutAirUnit.Name,
                              "UNDEFINED",
                              state.dataIPShortCut->cAlphaArgs(5),
                              state.dataIPShortCut->cAlphaArgs(12),
                              state.dataIPShortCut->cAlphaArgs(15));
            }

            // A16 : component list

            GlobalNames::IntraObjUniquenessCheck(state,
                                                 state.dataIPShortCut->cAlphaArgs(16),
                                                 CurrentModuleObject,
                                                 cAlphaFields(16),
                                                 state.dataOutdoorAirUnit->ComponentListUniqueNames,
                                                 ErrorsFound);
            std::string const ComponentListName = state.dataIPShortCut->cAlphaArgs(16);
            thisOutAirUnit.ComponentListName = ComponentListName;
            if (!lAlphaBlanks(16)) {
                int const ListNum = state.dataInputProcessing->inputProcessor->getObjectItemNum(state, ZoneHVACEqList, ComponentListName);
                if (ListNum > 0) {
                    state.dataInputProcessing->inputProcessor->getObjectItem(
                        state, ZoneHVACEqList, ListNum, AlphArray, NumAlphas, NumArray, NumNums, IOStat);
                    int NumInList = (NumAlphas - 1) / 2; // potential problem if puts in type but not name
                    if (mod(NumAlphas - 1, 2) != 0) {
                        ++NumInList;
                    }
                    thisOutAirUnit.NumComponents = NumInList;
                    thisOutAirUnit.OAEquip.allocate(NumInList);

                    // Get information of component
                    for (int InListNum = 1; InListNum <= NumInList; ++InListNum) {
                        thisOutAirUnit.OAEquip(InListNum).ComponentName = AlphArray(InListNum * 2 + 1);

                        thisOutAirUnit.OAEquip(InListNum).Type =
                            static_cast<CompType>(getEnumValue(CompTypeNamesUC, Util::makeUPPER(AlphArray(InListNum * 2))));

                        int const CompNum = InListNum;

                        // Coil Types
                        switch (thisOutAirUnit.OAEquip(InListNum).Type) {
                        case CompType::WaterCoil_Cooling: {
                            thisOutAirUnit.OAEquip(CompNum).CoilType = DataPlant::PlantEquipmentType::CoilWaterCooling;
                            thisOutAirUnit.OAEquip(CompNum).ComponentIndex =
                                GetWaterCoilIndex(state,
                                                  CompTypeNamesUC[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                  thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                  ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilAirInletNode =
                                WaterCoils::GetCoilInletNode(state,
                                                             CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                             thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                             ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilAirOutletNode =
                                WaterCoils::GetCoilOutletNode(state,
                                                              CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                              thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                              ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilWaterInletNode =
                                GetCoilWaterInletNode(state,
                                                      CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                      thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                      ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilWaterOutletNode =
                                GetCoilWaterOutletNode(state,
                                                       CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                       thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                       ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).MaxVolWaterFlow =
                                WaterCoils::GetCoilMaxWaterFlowRate(state,
                                                                    CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                                    thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                                    ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).MinVolWaterFlow = 0.0;
                            break;
                        }
                        case CompType::WaterCoil_SimpleHeat: {
                            thisOutAirUnit.OAEquip(CompNum).CoilType = DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
                            thisOutAirUnit.OAEquip(CompNum).ComponentIndex =
                                GetWaterCoilIndex(state,
                                                  CompTypeNamesUC[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                  thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                  ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilAirInletNode =
                                WaterCoils::GetCoilInletNode(state,
                                                             CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                             thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                             ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilAirOutletNode = WaterCoils::GetCoilOutletNode(
                                state, "Coil:Heating:Water", thisOutAirUnit.OAEquip(CompNum).ComponentName, ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilWaterInletNode =
                                GetCoilWaterInletNode(state,
                                                      CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                      thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                      ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilWaterOutletNode =
                                GetCoilWaterOutletNode(state,
                                                       CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                       thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                       ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).MaxVolWaterFlow = WaterCoils::GetCoilMaxWaterFlowRate(
                                state, "Coil:Heating:Water", thisOutAirUnit.OAEquip(CompNum).ComponentName, ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).MinVolWaterFlow = 0.0;
                            break;
                        }
                        case CompType::SteamCoil_AirHeat: {
                            thisOutAirUnit.OAEquip(CompNum).CoilType = DataPlant::PlantEquipmentType::CoilSteamAirHeating;
                            thisOutAirUnit.OAEquip(CompNum).ComponentIndex =
                                GetSteamCoilIndex(state,
                                                  CompTypeNamesUC[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                  thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                  ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilAirInletNode = GetCoilAirInletNode(
                                state, thisOutAirUnit.OAEquip(CompNum).ComponentIndex, thisOutAirUnit.OAEquip(CompNum).ComponentName, ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilAirOutletNode = GetCoilAirOutletNode(
                                state, thisOutAirUnit.OAEquip(CompNum).ComponentIndex, thisOutAirUnit.OAEquip(CompNum).ComponentName, ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilWaterInletNode = GetCoilSteamInletNode(
                                state, thisOutAirUnit.OAEquip(CompNum).ComponentIndex, thisOutAirUnit.OAEquip(CompNum).ComponentName, ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilWaterOutletNode =
                                GetCoilSteamOutletNode(state,
                                                       CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                       thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                       ErrorsFound);

                            thisOutAirUnit.OAEquip(CompNum).MaxVolWaterFlow =
                                GetCoilMaxSteamFlowRate(state, thisOutAirUnit.OAEquip(CompNum).ComponentIndex, ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).MinVolWaterFlow = 0.0;
                            // below: no extra error needed if steam properties not in input
                            // file because getting the steam coil will have done that.
                            thisOutAirUnit.OAEquip(CompNum).FluidIndex = Fluid::GetRefrigNum(state, "STEAM");
                            break;
                        }
                        case CompType::WaterCoil_DetailedCool: {
                            thisOutAirUnit.OAEquip(CompNum).ComponentIndex =
                                GetWaterCoilIndex(state,
                                                  CompTypeNamesUC[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                  thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                  ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilType = DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling;
                            thisOutAirUnit.OAEquip(CompNum).CoilAirInletNode =
                                WaterCoils::GetCoilInletNode(state,
                                                             CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                             thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                             ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilAirOutletNode =
                                WaterCoils::GetCoilOutletNode(state,
                                                              CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                              thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                              ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilWaterInletNode =
                                GetCoilWaterInletNode(state,
                                                      CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                      thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                      ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilWaterOutletNode =
                                GetCoilWaterOutletNode(state,
                                                       CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                       thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                       ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).MaxVolWaterFlow =
                                WaterCoils::GetCoilMaxWaterFlowRate(state,
                                                                    CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                                    thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                                    ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).MinVolWaterFlow = 0.0;
                            break;
                        }
                        case CompType::WaterCoil_CoolingHXAsst: {
                            thisOutAirUnit.OAEquip(CompNum).CoilAirInletNode =
                                HVACHXAssistedCoolingCoil::GetCoilInletNode(state,
                                                                            CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                                            thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                                            ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilAirOutletNode =
                                HVACHXAssistedCoolingCoil::GetCoilOutletNode(state,
                                                                             CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                                             thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                                             ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilWaterInletNode =
                                GetCoilWaterInletNode(state,
                                                      CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                      thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                      ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilWaterOutletNode =
                                GetCoilWaterOutletNode(state,
                                                       CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                       thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                       ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).MaxVolWaterFlow = HVACHXAssistedCoolingCoil::GetCoilMaxWaterFlowRate(
                                state,
                                CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).MinVolWaterFlow = 0.0;
                            break;
                        }
                        case CompType::Coil_ElectricHeat: {
                            // Get OutAirUnit( OAUnitNum ).OAEquip( CompNum ).ComponentIndex, 2 types of mining functions to choose from
                            HeatingCoils::GetCoilIndex(
                                state, thisOutAirUnit.OAEquip(CompNum).ComponentName, thisOutAirUnit.OAEquip(CompNum).ComponentIndex, ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilAirInletNode =
                                HeatingCoils::GetCoilInletNode(state,
                                                               CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                               thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                               ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilAirOutletNode =
                                HeatingCoils::GetCoilOutletNode(state,
                                                                CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                                thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                                ErrorsFound);
                            break;
                        }
                        case CompType::Coil_GasHeat: {
                            // Get OutAirUnit( OAUnitNum ).OAEquip( CompNum ).ComponentIndex, 2 types of mining functions to choose from
                            HeatingCoils::GetCoilIndex(
                                state, thisOutAirUnit.OAEquip(CompNum).ComponentName, thisOutAirUnit.OAEquip(CompNum).ComponentIndex, ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilAirInletNode =
                                GetCoilInletNode(state,
                                                 CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                 thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                 ErrorsFound);
                            thisOutAirUnit.OAEquip(CompNum).CoilAirOutletNode =
                                GetCoilOutletNode(state,
                                                  CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)],
                                                  thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                  ErrorsFound);
                            break;
                        }
                        case CompType::DXSystem: {
                            // set the data for 100% DOAS DX cooling coil
                            // is a different function call needed here? similar to one in HVACDXSystem
                            // CheckDXCoolingCoilInOASysExists(state, thisOutAirUnit.OAEquip(CompNum).ComponentName);
                            break;
                        }
                        case CompType::DXHeatPumpSystem: {
                            break;
                        }
                        case CompType::UnitarySystemModel: {
                            UnitarySystems::UnitarySys thisSys;
                            thisOutAirUnit.OAEquip(CompNum).compPointer = thisSys.factory(
                                state, HVAC::UnitarySysType::Unitary_AnyCoilType, thisOutAirUnit.OAEquip(CompNum).ComponentName, false, OAUnitNum);
                            UnitarySystems::UnitarySys::checkUnitarySysCoilInOASysExists(
                                state, thisOutAirUnit.OAEquip(CompNum).ComponentName, OAUnitNum);

                            // Heat recovery
                            break;
                        }
                        case CompType::HeatXchngrFP:
                        case CompType::HeatXchngrSL: {
                            //        CASE('HEATEXCHANGER:DESICCANT:BALANCEDFLOW')
                            //          thisOutAirUnit%OAEquip(CompNum)%Type= CompType::HeatXchngr

                            // Desiccant Dehumidifier
                            OutputReportPredefined::PreDefTableEntry(state,
                                                                     state.dataOutRptPredefined->pdchAirHRZoneHVACName,
                                                                     thisOutAirUnit.OAEquip(CompNum).ComponentName,
                                                                     thisOutAirUnit.Name);

                            break;
                        }
                        case CompType::Desiccant: {
                            // Future Enhancement
                            //        CASE('DEHUMIDIFIER:DESICCANT:SYSTEM')
                            //          thisOutAirUnit%OAEquip(CompNum)%Type= CompType::Desiccant
                            break;
                        }
                        default: {
                            ShowSevereError(state,
                                            format("{}= \"{}\" invalid Outside Air Component=\"{}\".",
                                                   CurrentModuleObject,
                                                   AlphArray(1),
                                                   CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(CompNum).Type)]));
                            ErrorsFound = true;
                        }
                        }

                        // Add equipment to component sets array
                        // Node set up
                        if (thisOutAirUnit.supFanPlace == HVAC::FanPlace::BlowThru) {
                            if (InListNum == 1) { // the component is the first one
                                SetUpCompSets(state,
                                              "ZoneHVAC:OutdoorAirUnit",
                                              thisOutAirUnit.Name,
                                              CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(InListNum).Type)],
                                              thisOutAirUnit.OAEquip(InListNum).ComponentName,
                                              state.dataIPShortCut->cAlphaArgs(15),
                                              "UNDEFINED");
                            } else if (InListNum != NumInList) { // the component is placed in b/w components
                                SetUpCompSets(state,
                                              "ZoneHVAC:OutdoorAirUnit",
                                              thisOutAirUnit.Name,
                                              CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(InListNum).Type)],
                                              thisOutAirUnit.OAEquip(InListNum).ComponentName,
                                              "UNDEFINED",
                                              "UNDEFINED");
                            } else { // (InListNum == NumInList) => the component is the last one
                                SetUpCompSets(state,
                                              "ZoneHVAC:OutdoorAirUnit",
                                              thisOutAirUnit.Name,
                                              CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(InListNum).Type)],
                                              thisOutAirUnit.OAEquip(InListNum).ComponentName,
                                              "UNDEFINED",
                                              state.dataIPShortCut->cAlphaArgs(13));
                            }
                            // If fan is on the end of equipment.
                        } else if (thisOutAirUnit.supFanPlace == HVAC::FanPlace::DrawThru) {
                            if (InListNum == 1) {
                                SetUpCompSets(state,
                                              "ZoneHVAC:OutdoorAirUnit",
                                              thisOutAirUnit.Name,
                                              CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(InListNum).Type)],
                                              thisOutAirUnit.OAEquip(InListNum).ComponentName,
                                              state.dataIPShortCut->cAlphaArgs(12),
                                              "UNDEFINED");
                            } else if (InListNum != NumInList) {
                                SetUpCompSets(state,
                                              "ZoneHVAC:OutdoorAirUnit",
                                              thisOutAirUnit.Name,
                                              CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(InListNum).Type)],
                                              thisOutAirUnit.OAEquip(InListNum).ComponentName,
                                              "UNDEFINED",
                                              "UNDEFINED");
                            } else { // (InListNum == NumInList) => the component is the last one
                                SetUpCompSets(state,
                                              "ZoneHVAC:OutdoorAirUnit",
                                              thisOutAirUnit.Name,
                                              CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(InListNum).Type)],
                                              thisOutAirUnit.OAEquip(InListNum).ComponentName,
                                              "UNDEFINED",
                                              "UNDEFINED");
                            }
                        }
                        // Must call after SetUpCompSets since this will add another CoilSystem:Cooling:DX object in CompSets
                        if (CompTypeNamesUC[static_cast<int>(thisOutAirUnit.OAEquip(InListNum).Type)] == "COILSYSTEM:COOLING:DX") {
                            UnitarySystems::UnitarySys::checkUnitarySysCoilInOASysExists(
                                state, thisOutAirUnit.OAEquip(CompNum).ComponentName, OAUnitNum);
                        }
                    } // End Inlist

                    // In case of draw through, the last component is linked with the zone air supply node
                    if (thisOutAirUnit.supFanPlace == HVAC::FanPlace::DrawThru) {
                        SetUpCompSets(state,
                                      CurrentModuleObject,
                                      thisOutAirUnit.Name,
                                      "UNDEFINED",
                                      state.dataIPShortCut->cAlphaArgs(5),
                                      "UNDEFINED",
                                      state.dataIPShortCut->cAlphaArgs(13));
                    }

                } else { // when ListNum<0
                    ShowSevereError(state,
                                    format("{} = \"{}\" invalid {}=\"{}\" not found.",
                                           CurrentModuleObject,
                                           state.dataIPShortCut->cAlphaArgs(1),
                                           cAlphaFields(16),
                                           state.dataIPShortCut->cAlphaArgs(16)));
                    ErrorsFound = true;
                }
            } else { // when Equipment list is left blanked
                ShowSevereError(state,
                                format("{} = \"{}\" invalid {} is blank and must be entered.",
                                       CurrentModuleObject,
                                       state.dataIPShortCut->cAlphaArgs(1),
                                       cAlphaFields(16)));
                ErrorsFound = true;
            }
            if (!lAlphaBlanks(17)) {
                thisOutAirUnit.AvailManagerListName = state.dataIPShortCut->cAlphaArgs(17);
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, format("{}Errors found in getting {}.", RoutineName, CurrentModuleObject));
        }

        AlphArray.deallocate();
        cAlphaFields.deallocate();
        NumArray.deallocate();
        cNumericFields.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        state.dataOutdoorAirUnit->GetOutdoorAirUnitInputFlag = false;

        // Setup Report variables for the zone outdoor air unit CurrentModuleObject='ZoneHVAC:OutdoorAirUnit'
        for (int OAUnitNum = 1; OAUnitNum <= state.dataOutdoorAirUnit->NumOfOAUnits; ++OAUnitNum) {

            auto &thisOutAirUnit = state.dataOutdoorAirUnit->OutAirUnit(OAUnitNum);

            SetupOutputVariable(state,
                                "Zone Outdoor Air Unit Total Heating Rate",
                                Constant::Units::W,
                                thisOutAirUnit.TotHeatingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisOutAirUnit.Name);
            SetupOutputVariable(state,
                                "Zone Outdoor Air Unit Total Heating Energy",
                                Constant::Units::J,
                                thisOutAirUnit.TotHeatingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisOutAirUnit.Name);
            SetupOutputVariable(state,
                                "Zone Outdoor Air Unit Sensible Heating Rate",
                                Constant::Units::W,
                                thisOutAirUnit.SensHeatingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisOutAirUnit.Name);
            SetupOutputVariable(state,
                                "Zone Outdoor Air Unit Sensible Heating Energy",
                                Constant::Units::J,
                                thisOutAirUnit.SensHeatingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisOutAirUnit.Name);
            SetupOutputVariable(state,
                                "Zone Outdoor Air Unit Latent Heating Rate",
                                Constant::Units::W,
                                thisOutAirUnit.LatHeatingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisOutAirUnit.Name);
            SetupOutputVariable(state,
                                "Zone Outdoor Air Unit Latent Heating Energy",
                                Constant::Units::J,
                                thisOutAirUnit.LatHeatingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisOutAirUnit.Name);
            SetupOutputVariable(state,
                                "Zone Outdoor Air Unit Total Cooling Rate",
                                Constant::Units::W,
                                thisOutAirUnit.TotCoolingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisOutAirUnit.Name);
            SetupOutputVariable(state,
                                "Zone Outdoor Air Unit Total Cooling Energy",
                                Constant::Units::J,
                                thisOutAirUnit.TotCoolingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisOutAirUnit.Name);
            SetupOutputVariable(state,
                                "Zone Outdoor Air Unit Sensible Cooling Rate",
                                Constant::Units::W,
                                thisOutAirUnit.SensCoolingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisOutAirUnit.Name);
            SetupOutputVariable(state,
                                "Zone Outdoor Air Unit Sensible Cooling Energy",
                                Constant::Units::J,
                                thisOutAirUnit.SensCoolingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisOutAirUnit.Name);
            SetupOutputVariable(state,
                                "Zone Outdoor Air Unit Latent Cooling Rate",
                                Constant::Units::W,
                                thisOutAirUnit.LatCoolingRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisOutAirUnit.Name);
            SetupOutputVariable(state,
                                "Zone Outdoor Air Unit Latent Cooling Energy",
                                Constant::Units::J,
                                thisOutAirUnit.LatCoolingEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisOutAirUnit.Name);
            SetupOutputVariable(state,
                                "Zone Outdoor Air Unit Air Mass Flow Rate",
                                Constant::Units::kg_s,
                                thisOutAirUnit.AirMassFlow,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisOutAirUnit.Name);
            SetupOutputVariable(state,
                                "Zone Outdoor Air Unit Fan Electricity Rate",
                                Constant::Units::W,
                                thisOutAirUnit.ElecFanRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisOutAirUnit.Name);
            SetupOutputVariable(state,
                                "Zone Outdoor Air Unit Fan Electricity Energy",
                                Constant::Units::J,
                                thisOutAirUnit.ElecFanEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisOutAirUnit.Name);
            SetupOutputVariable(state,
                                "Zone Outdoor Air Unit Fan Availability Status",
                                Constant::Units::None,
                                thisOutAirUnit.availStatus,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisOutAirUnit.Name);
            //! Note that the outdoor air unit fan electric is NOT metered because this value is already metered through the fan component
        }
    }

    void InitOutdoorAirUnit(EnergyPlusData &state,
                            int const OAUnitNum,          // index for the current outdoor air unit
                            int const ZoneNum,            // number of zone being served
                            bool const FirstHVACIteration // TRUE if 1st HVAC simulation of system timestep
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Young Tae Chae, Rick Strand
        //       DATE WRITTEN   July 2009
        //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine initializes all of the data elements which are necessary
        // to simulate a zone outdoor air control unit.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        // REFERENCES:
        // na

        // Using/Aliasing
        using DataZoneEquipment::CheckZoneEquipmentList;
        using HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil;
        using PlantUtilities::InitComponentNodes;
        using PlantUtilities::ScanPlantLoopsForObject;
        using SteamCoils::GetCoilMaxSteamFlowRate;
        using WaterCoils::SimulateWaterCoilComponents;

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view CurrentModuleObject("ZoneHVAC:OutdoorAirUnit");
        static constexpr std::string_view RoutineName("SizeOutdoorAirUnit");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // Do the one time initializations

        auto &thisOutAirUnit = state.dataOutdoorAirUnit->OutAirUnit(OAUnitNum);

        Real64 const RhoAir = state.dataEnvrn->StdRhoAir;
        int const InNode = thisOutAirUnit.AirInletNode;
        int const OutNode = thisOutAirUnit.AirOutletNode;
        int const OutsideAirNode = thisOutAirUnit.OutsideAirNode;
        Real64 const OAFrac = thisOutAirUnit.outAirSched->getCurrentVal();

        if (state.dataOutdoorAirUnit->MyOneTimeFlag) {

            state.dataOutdoorAirUnit->MyEnvrnFlag.dimension(state.dataOutdoorAirUnit->NumOfOAUnits, true);
            state.dataOutdoorAirUnit->MySizeFlag.dimension(state.dataOutdoorAirUnit->NumOfOAUnits, true);
            state.dataOutdoorAirUnit->MyPlantScanFlag.dimension(state.dataOutdoorAirUnit->NumOfOAUnits, true);
            state.dataOutdoorAirUnit->MyZoneEqFlag.dimension(state.dataOutdoorAirUnit->NumOfOAUnits, true);
            state.dataOutdoorAirUnit->MyOneTimeFlag = false;
        }

        if (allocated(state.dataAvail->ZoneComp)) {
            auto &availMgr = state.dataAvail->ZoneComp(DataZoneEquipment::ZoneEquipType::OutdoorAirUnit).ZoneCompAvailMgrs(OAUnitNum);
            if (state.dataOutdoorAirUnit->MyZoneEqFlag(OAUnitNum)) { // initialize the name of each availability manager list and zone number
                availMgr.AvailManagerListName = thisOutAirUnit.AvailManagerListName;
                availMgr.ZoneNum = ZoneNum;
                state.dataOutdoorAirUnit->MyZoneEqFlag(OAUnitNum) = false;
            }
            thisOutAirUnit.availStatus = availMgr.availStatus;
        }

        if (state.dataOutdoorAirUnit->MyPlantScanFlag(OAUnitNum) && allocated(state.dataPlnt->PlantLoop)) {
            for (int compLoop = 1; compLoop <= thisOutAirUnit.NumComponents; ++compLoop) {

                CompType const Type = thisOutAirUnit.OAEquip(compLoop).Type;

                switch (Type) {
                case CompType::WaterCoil_Cooling:
                case CompType::WaterCoil_DetailedCool:
                case CompType::WaterCoil_SimpleHeat:
                case CompType::SteamCoil_AirHeat:

                {
                    bool errFlag = false;
                    ScanPlantLoopsForObject(state,
                                            thisOutAirUnit.OAEquip(compLoop).ComponentName,
                                            thisOutAirUnit.OAEquip(compLoop).CoilType,
                                            thisOutAirUnit.OAEquip(compLoop).plantLoc,
                                            errFlag,
                                            _,
                                            _,
                                            _,
                                            _,
                                            _);
                    if (errFlag) {
                        ShowFatalError(state, "InitOutdoorAirUnit: Program terminated for previous conditions.");
                    }
                    break;
                }
                default:
                    break;
                }
            }

            state.dataOutdoorAirUnit->MyPlantScanFlag(OAUnitNum) = false;
        } else if (state.dataOutdoorAirUnit->MyPlantScanFlag(OAUnitNum) && !state.dataGlobal->AnyPlantInModel) {
            state.dataOutdoorAirUnit->MyPlantScanFlag(OAUnitNum) = false;
        }

        // need to check all zone outdoor air control units to see if they are on Zone Equipment List or issue warning
        if (!state.dataOutdoorAirUnit->ZoneEquipmentListChecked && state.dataZoneEquip->ZoneEquipInputsFilled) {
            state.dataOutdoorAirUnit->ZoneEquipmentListChecked = true;
            for (int Loop = 1; Loop <= state.dataOutdoorAirUnit->NumOfOAUnits; ++Loop) {
                if (CheckZoneEquipmentList(state, CurrentModuleObject, state.dataOutdoorAirUnit->OutAirUnit(Loop).Name)) {
                    continue;
                }
                ShowSevereError(
                    state,
                    format("InitOutdoorAirUnit: Zone Outdoor Air Unit=[{},{}] is not on any ZoneHVAC:EquipmentList.  It will not be simulated.",
                           CurrentModuleObject,
                           state.dataOutdoorAirUnit->OutAirUnit(Loop).Name));
            }
        }

        if (!state.dataGlobal->SysSizingCalc && state.dataOutdoorAirUnit->MySizeFlag(OAUnitNum) &&
            !state.dataOutdoorAirUnit->MyPlantScanFlag(OAUnitNum)) {

            SizeOutdoorAirUnit(state, OAUnitNum);

            state.dataOutdoorAirUnit->MySizeFlag(OAUnitNum) = false;
        }

        // Do the one time initializations
        if (state.dataGlobal->BeginEnvrnFlag && state.dataOutdoorAirUnit->MyEnvrnFlag(OAUnitNum)) {
            // Node Conditions
            thisOutAirUnit.OutAirMassFlow = RhoAir * OAFrac * thisOutAirUnit.OutAirVolFlow;
            thisOutAirUnit.SMaxAirMassFlow = RhoAir * OAFrac * thisOutAirUnit.SFanMaxAirVolFlow;

            if (thisOutAirUnit.ExtFan) {
                // set the exhaust air mass flow rate from input
                Real64 const EAFrac = thisOutAirUnit.extAirSched->getCurrentVal();
                thisOutAirUnit.ExtAirMassFlow = RhoAir * EAFrac * thisOutAirUnit.ExtAirVolFlow;
                thisOutAirUnit.EMaxAirMassFlow = RhoAir * EAFrac * thisOutAirUnit.EFanMaxAirVolFlow;

                state.dataLoopNodes->Node(InNode).MassFlowRateMax = thisOutAirUnit.EMaxAirMassFlow;
                state.dataLoopNodes->Node(InNode).MassFlowRateMin = 0.0;
            }
            // set the node max and min mass flow rates
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMax = thisOutAirUnit.SMaxAirMassFlow;
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMin = 0.0;
            state.dataLoopNodes->Node(OutNode).MassFlowRate = thisOutAirUnit.EMaxAirMassFlow;

            if (!state.dataOutdoorAirUnit->MyPlantScanFlag(OAUnitNum)) {
                bool errFlag = false;
                for (int compLoop = 1; compLoop <= thisOutAirUnit.NumComponents; ++compLoop) {
                    if ((thisOutAirUnit.OAEquip(compLoop).Type == CompType::WaterCoil_Cooling) ||
                        (thisOutAirUnit.OAEquip(compLoop).Type == CompType::WaterCoil_DetailedCool)) {
                        thisOutAirUnit.OAEquip(compLoop).MaxVolWaterFlow =
                            WaterCoils::GetCoilMaxWaterFlowRate(state,
                                                                CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(compLoop).Type)],
                                                                thisOutAirUnit.OAEquip(compLoop).ComponentName,
                                                                errFlag);
                        Real64 const rho = state.dataPlnt->PlantLoop(thisOutAirUnit.OAEquip(compLoop).plantLoc.loopNum)
                                               .glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
                        thisOutAirUnit.OAEquip(compLoop).MaxWaterMassFlow = rho * thisOutAirUnit.OAEquip(compLoop).MaxVolWaterFlow;
                        thisOutAirUnit.OAEquip(compLoop).MinWaterMassFlow = rho * thisOutAirUnit.OAEquip(compLoop).MinVolWaterFlow;
                        InitComponentNodes(state,
                                           thisOutAirUnit.OAEquip(compLoop).MinWaterMassFlow,
                                           thisOutAirUnit.OAEquip(compLoop).MaxWaterMassFlow,
                                           thisOutAirUnit.OAEquip(compLoop).CoilWaterInletNode,
                                           thisOutAirUnit.OAEquip(compLoop).CoilWaterOutletNode);
                    }

                    if (thisOutAirUnit.OAEquip(compLoop).Type == CompType::WaterCoil_SimpleHeat) {
                        thisOutAirUnit.OAEquip(compLoop).MaxVolWaterFlow =
                            WaterCoils::GetCoilMaxWaterFlowRate(state,
                                                                CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(compLoop).Type)],
                                                                thisOutAirUnit.OAEquip(compLoop).ComponentName,
                                                                errFlag);
                        Real64 const rho = state.dataPlnt->PlantLoop(thisOutAirUnit.OAEquip(compLoop).plantLoc.loopNum)
                                               .glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                        thisOutAirUnit.OAEquip(compLoop).MaxWaterMassFlow = rho * thisOutAirUnit.OAEquip(compLoop).MaxVolWaterFlow;
                        thisOutAirUnit.OAEquip(compLoop).MinWaterMassFlow = rho * thisOutAirUnit.OAEquip(compLoop).MinVolWaterFlow;
                        InitComponentNodes(state,
                                           thisOutAirUnit.OAEquip(compLoop).MinWaterMassFlow,
                                           thisOutAirUnit.OAEquip(compLoop).MaxWaterMassFlow,
                                           thisOutAirUnit.OAEquip(compLoop).CoilWaterInletNode,
                                           thisOutAirUnit.OAEquip(compLoop).CoilWaterOutletNode);
                    }
                    if (thisOutAirUnit.OAEquip(compLoop).Type == CompType::SteamCoil_AirHeat) {
                        thisOutAirUnit.OAEquip(compLoop).MaxVolWaterFlow =
                            GetCoilMaxSteamFlowRate(state, thisOutAirUnit.OAEquip(compLoop).ComponentIndex, errFlag);
                        Real64 const rho = state.dataPlnt->PlantLoop(thisOutAirUnit.OAEquip(compLoop).plantLoc.loopNum)
                                               .steam->getSatDensity(state, Constant::SteamInitConvTemp, 1.0, RoutineName);
                        thisOutAirUnit.OAEquip(compLoop).MaxWaterMassFlow = rho * thisOutAirUnit.OAEquip(compLoop).MaxVolWaterFlow;
                        thisOutAirUnit.OAEquip(compLoop).MinWaterMassFlow = rho * thisOutAirUnit.OAEquip(compLoop).MinVolWaterFlow;
                        InitComponentNodes(state,
                                           thisOutAirUnit.OAEquip(compLoop).MinWaterMassFlow,
                                           thisOutAirUnit.OAEquip(compLoop).MaxWaterMassFlow,
                                           thisOutAirUnit.OAEquip(compLoop).CoilWaterInletNode,
                                           thisOutAirUnit.OAEquip(compLoop).CoilWaterOutletNode);
                    }
                    if (thisOutAirUnit.OAEquip(compLoop).Type == CompType::WaterCoil_CoolingHXAsst) {
                        thisOutAirUnit.OAEquip(compLoop).MaxVolWaterFlow =
                            WaterCoils::GetCoilMaxWaterFlowRate(state,
                                                                CompTypeNames[static_cast<int>(thisOutAirUnit.OAEquip(compLoop).Type)],
                                                                thisOutAirUnit.OAEquip(compLoop).ComponentName,
                                                                errFlag);
                        Real64 const rho = state.dataPlnt->PlantLoop(thisOutAirUnit.OAEquip(compLoop).plantLoc.loopNum)
                                               .glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
                        thisOutAirUnit.OAEquip(compLoop).MaxWaterMassFlow = rho * thisOutAirUnit.OAEquip(compLoop).MaxVolWaterFlow;
                        thisOutAirUnit.OAEquip(compLoop).MinWaterMassFlow = rho * thisOutAirUnit.OAEquip(compLoop).MinVolWaterFlow;
                        InitComponentNodes(state,
                                           thisOutAirUnit.OAEquip(compLoop).MinWaterMassFlow,
                                           thisOutAirUnit.OAEquip(compLoop).MaxWaterMassFlow,
                                           thisOutAirUnit.OAEquip(compLoop).CoilWaterInletNode,
                                           thisOutAirUnit.OAEquip(compLoop).CoilWaterOutletNode);
                    }
                }
            }
            state.dataOutdoorAirUnit->MyEnvrnFlag(OAUnitNum) = false;

        } // ...end start of environment inits

        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataOutdoorAirUnit->MyEnvrnFlag(OAUnitNum) = true;
        }

        // These initializations are done every iteration...
        // Set all the output variable
        thisOutAirUnit.TotHeatingRate = 0.0;
        thisOutAirUnit.SensHeatingRate = 0.0;
        thisOutAirUnit.LatHeatingRate = 0.0;
        thisOutAirUnit.TotCoolingRate = 0.0;
        thisOutAirUnit.SensCoolingRate = 0.0;
        thisOutAirUnit.LatCoolingRate = 0.0;
        thisOutAirUnit.AirMassFlow = 0.0;
        thisOutAirUnit.ElecFanRate = 0.0;
        // Node Set

        // set the mass flow rates from the input volume flow rates
        if (OAFrac > 0.0 || (state.dataHVACGlobal->TurnFansOn && !state.dataHVACGlobal->TurnFansOff)) { // fan is available
            thisOutAirUnit.OutAirMassFlow = RhoAir * OAFrac * thisOutAirUnit.OutAirVolFlow;
        } else {
            thisOutAirUnit.OutAirMassFlow = 0.0;
        }

        // set the exhaust air mass flow rate from input
        if (thisOutAirUnit.ExtFan) {
            if (thisOutAirUnit.extFanAvailSched != nullptr) {
                thisOutAirUnit.ExtAirMassFlow = RhoAir * thisOutAirUnit.ExtAirVolFlow * thisOutAirUnit.extAirSched->getCurrentVal();
            } else {
                thisOutAirUnit.ExtAirMassFlow = 0.0;
            }
            state.dataLoopNodes->Node(InNode).MassFlowRate = thisOutAirUnit.ExtAirMassFlow;
            state.dataLoopNodes->Node(InNode).MassFlowRateMaxAvail = thisOutAirUnit.ExtAirMassFlow;
            state.dataLoopNodes->Node(InNode).MassFlowRateMinAvail = 0.0;
        } else if (!thisOutAirUnit.ExtFan) {
            thisOutAirUnit.ExtAirMassFlow = 0.0;
        }

        // First, set the flow conditions up so that there is flow through the unit

        state.dataLoopNodes->Node(OutNode).MassFlowRate = thisOutAirUnit.OutAirMassFlow;
        state.dataLoopNodes->Node(OutNode).MassFlowRateMaxAvail = thisOutAirUnit.OutAirMassFlow;
        state.dataLoopNodes->Node(OutNode).MassFlowRateMinAvail = 0.0;
        state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate = thisOutAirUnit.OutAirMassFlow;
        state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMaxAvail = thisOutAirUnit.OutAirMassFlow;
        state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMinAvail = 0.0;

        // Just in case the system is off and conditions do not get sent through
        // the system for some reason, set the outlet conditions equal to the inlet
        // conditions of the zone outdoor air control unit
        if (thisOutAirUnit.ExtFan) {
            state.dataLoopNodes->Node(OutNode).Temp = state.dataLoopNodes->Node(InNode).Temp;
            state.dataLoopNodes->Node(OutNode).Press = state.dataLoopNodes->Node(InNode).Press;
            state.dataLoopNodes->Node(OutNode).HumRat = state.dataLoopNodes->Node(InNode).HumRat;
            state.dataLoopNodes->Node(OutNode).Enthalpy = state.dataLoopNodes->Node(InNode).Enthalpy;
        } else {
            state.dataLoopNodes->Node(OutNode).Temp = state.dataLoopNodes->Node(OutsideAirNode).Temp;
            state.dataLoopNodes->Node(OutNode).Press = state.dataLoopNodes->Node(OutsideAirNode).Press;
            state.dataLoopNodes->Node(OutNode).HumRat = state.dataLoopNodes->Node(OutsideAirNode).HumRat;
            state.dataLoopNodes->Node(OutNode).Enthalpy = state.dataLoopNodes->Node(OutsideAirNode).Enthalpy;
        }
        // These initializations only need to be done once at the start of the iterations...
        if (FirstHVACIteration || state.dataHVACGlobal->ShortenTimeStepSys) {
            // Initialize the outside air conditions...
            state.dataLoopNodes->Node(OutsideAirNode).Temp = state.dataLoopNodes->Node(OutsideAirNode).OutAirDryBulb;
            state.dataLoopNodes->Node(OutsideAirNode).HumRat = state.dataEnvrn->OutHumRat;
            state.dataLoopNodes->Node(OutsideAirNode).Press = state.dataEnvrn->OutBaroPress;
        }
    }

    void SizeOutdoorAirUnit(EnergyPlusData &state, int const OAUnitNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Young Tae Chae, Rick Strand
        //       DATE WRITTEN   July 2009
        //       MODIFIED       Brent Griffith, March 2010, autosize OA flow rate
        //                      August 2013 Daeho Kang, add component sizing table entries
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing zone outdoor air control unit components for which flow rates have not been
        // specified in the input.

        // METHODOLOGY EMPLOYED:
        // Obtains flow rates from the zone sizing arrays and plant sizing data.

        // Using/Aliasing
        using namespace DataSizing;

        using HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil;
        using PlantUtilities::MyPlantSizingIndex;
        using SteamCoils::SimulateSteamCoilComponents;
        using WaterCoils::SimulateWaterCoilComponents;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool IsAutoSize = false;        // Indicator to autosize
        Real64 OutAirVolFlowDes = 0.0;  // Autosized outdoor air flow for reporting
        Real64 OutAirVolFlowUser = 0.0; // Hardsized outdoor air flow for reporting
        Real64 ExtAirVolFlowDes = 0.0;  // Autosized exhaust air flow for reporting
        Real64 ExtAirVolFlowUser = 0.0; // Hardsized exhaust air flow for reporting

        bool ErrorsFound = false;

        auto &thisOutAirUnit = state.dataOutdoorAirUnit->OutAirUnit(OAUnitNum);

        state.dataSize->DataFanType = thisOutAirUnit.supFanType;
        state.dataSize->DataFanIndex = thisOutAirUnit.SFan_Index;
        state.dataSize->DataFanPlacement = thisOutAirUnit.supFanPlace;

        if (thisOutAirUnit.OutAirVolFlow == AutoSize) {
            IsAutoSize = true;
        }

        if (state.dataSize->CurZoneEqNum > 0) {
            if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
                if (thisOutAirUnit.OutAirVolFlow > 0.0) {
                    BaseSizer::reportSizerOutput(
                        state, ZoneHVACOAUnit, thisOutAirUnit.Name, "User-Specified Outdoor Air Flow Rate [m3/s]", thisOutAirUnit.OutAirVolFlow);
                }
            } else {
                CheckZoneSizing(state, std::string(ZoneHVACOAUnit), thisOutAirUnit.Name);
                OutAirVolFlowDes = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).MinOA;
                if (OutAirVolFlowDes < SmallAirVolFlow) {
                    OutAirVolFlowDes = 0.0;
                }
                if (IsAutoSize) {
                    thisOutAirUnit.OutAirVolFlow = OutAirVolFlowDes;
                    BaseSizer::reportSizerOutput(
                        state, ZoneHVACOAUnit, thisOutAirUnit.Name, "Design Size Outdoor Air Flow Rate [m3/s]", OutAirVolFlowDes);
                } else {
                    if (thisOutAirUnit.OutAirVolFlow > 0.0 && OutAirVolFlowDes > 0.0) {
                        OutAirVolFlowUser = thisOutAirUnit.OutAirVolFlow;
                        BaseSizer::reportSizerOutput(
                            state, ZoneHVACOAUnit, thisOutAirUnit.Name, "User-Specified Outdoor Air Flow Rate [m3/s]", OutAirVolFlowUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(OutAirVolFlowDes - OutAirVolFlowUser) / OutAirVolFlowUser) > state.dataSize->AutoVsHardSizingThreshold) {
                                BaseSizer::reportSizerOutput(
                                    state, ZoneHVACOAUnit, thisOutAirUnit.Name, "Design Size Outdoor Air Flow Rate [m3/s]", OutAirVolFlowDes);
                                ShowMessage(state,
                                            format("SizeOutdoorAirUnit: Potential issue with equipment sizing for ZoneHVAC:OutdoorAirUnit {}",
                                                   thisOutAirUnit.Name));
                                ShowContinueError(state, format("User-Specified Outdoor Air Flow Rate of {:.5R} [m3/s]", OutAirVolFlowUser));
                                ShowContinueError(state, format("differs from Design Size Outdoor Air Flow Rate of {:.5R} [m3/s]", OutAirVolFlowDes));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }
            }
        }

        IsAutoSize = false;
        if (thisOutAirUnit.ExtAirVolFlow == AutoSize) {
            IsAutoSize = true;
        }
        if (state.dataSize->CurZoneEqNum > 0) {
            if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
                if (thisOutAirUnit.ExtAirVolFlow > 0.0) {
                    BaseSizer::reportSizerOutput(
                        state, ZoneHVACOAUnit, thisOutAirUnit.Name, "User-Specified Exhaust Air Flow Rate [m3/s]", thisOutAirUnit.ExtAirVolFlow);
                }
            } else {
                // set exhaust flow equal to the oa inlet flow
                ExtAirVolFlowDes = thisOutAirUnit.OutAirVolFlow;
                if (IsAutoSize) {
                    thisOutAirUnit.ExtAirVolFlow = ExtAirVolFlowDes;
                    BaseSizer::reportSizerOutput(
                        state, ZoneHVACOAUnit, thisOutAirUnit.Name, "Design Size Exhaust Air Flow Rate [m3/s]", ExtAirVolFlowDes);
                } else {
                    if (thisOutAirUnit.ExtAirVolFlow > 0.0 && ExtAirVolFlowDes > 0.0) {
                        ExtAirVolFlowUser = thisOutAirUnit.ExtAirVolFlow;
                        BaseSizer::reportSizerOutput(
                            state, ZoneHVACOAUnit, thisOutAirUnit.Name, "User-Specified Exhaust Air Flow Rate [m3/s]", ExtAirVolFlowUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(ExtAirVolFlowDes - ExtAirVolFlowUser) / ExtAirVolFlowUser) > state.dataSize->AutoVsHardSizingThreshold) {
                                BaseSizer::reportSizerOutput(
                                    state, ZoneHVACOAUnit, thisOutAirUnit.Name, "Design Size Exhaust Air Flow Rate [m3/s]", ExtAirVolFlowDes);
                                ShowMessage(state,
                                            format("SizeOutdoorAirUnit: Potential issue with equipment sizing for ZoneHVAC:OutdoorAirUnit {}",
                                                   thisOutAirUnit.Name));
                                ShowContinueError(state, format("User-Specified Exhaust Air Flow Rate of {:.5R} [m3/s]", ExtAirVolFlowUser));
                                ShowContinueError(state, format("differs from Design Size Exhaust Air Flow Rate of {:.5R} [m3/s]", ExtAirVolFlowDes));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }
            }
        }

        state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum).CoolingAirFlow = true;
        state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum).HeatingAirFlow = true;
        state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum).CoolingAirVolFlow = thisOutAirUnit.OutAirVolFlow;
        state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum).HeatingAirVolFlow = thisOutAirUnit.OutAirVolFlow;
        state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum).OAVolFlow = thisOutAirUnit.OutAirVolFlow;

        if (thisOutAirUnit.SFanMaxAirVolFlow == AutoSize) {
            state.dataFans->fans(thisOutAirUnit.SFan_Index)->simulate(state, true, _, _);
            thisOutAirUnit.SFanMaxAirVolFlow = state.dataFans->fans(thisOutAirUnit.SFan_Index)->maxAirFlowRate;
        }
        if (thisOutAirUnit.ExtFan) {
            if (thisOutAirUnit.EFanMaxAirVolFlow == AutoSize) {
                state.dataFans->fans(thisOutAirUnit.ExtFan_Index)->simulate(state, true, _, _);
                thisOutAirUnit.EFanMaxAirVolFlow = state.dataFans->fans(thisOutAirUnit.ExtFan_Index)->maxAirFlowRate;
            }
        }

        for (int CompNum = 1; CompNum <= thisOutAirUnit.NumComponents; ++CompNum) {
            auto &thisOAEquip = thisOutAirUnit.OAEquip(CompNum);
            if ((thisOAEquip.Type == CompType::WaterCoil_Cooling) || (thisOAEquip.Type == CompType::WaterCoil_DetailedCool)) {
                if (thisOAEquip.MaxVolWaterFlow == AutoSize) {
                    SimulateWaterCoilComponents(state, thisOAEquip.ComponentName, true, thisOAEquip.ComponentIndex, _, HVAC::FanOp::Cycling, 0.0);
                }
            }
            if (thisOAEquip.Type == CompType::WaterCoil_SimpleHeat) {
                if (thisOAEquip.MaxVolWaterFlow == AutoSize) {
                    SimulateWaterCoilComponents(state, thisOAEquip.ComponentName, true, thisOAEquip.ComponentIndex, _, HVAC::FanOp::Cycling, 0.0);
                }
            }
            if (thisOAEquip.Type == CompType::SteamCoil_AirHeat) {
                if (thisOAEquip.MaxVolWaterFlow == AutoSize) {
                    SimulateSteamCoilComponents(state, thisOAEquip.ComponentName, true, thisOAEquip.ComponentIndex);
                }
            }
            if (thisOAEquip.Type == CompType::WaterCoil_CoolingHXAsst) {
                if (thisOAEquip.MaxVolWaterFlow == AutoSize) {
                    SimHXAssistedCoolingCoil(
                        state, thisOAEquip.ComponentName, true, HVAC::CompressorOp::On, 0.0, thisOAEquip.ComponentIndex, HVAC::FanOp::Continuous);
                }
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, "Preceding sizing errors cause program termination");
        }
    }

    void CalcOutdoorAirUnit(EnergyPlusData &state,
                            int &OAUnitNum,                // number of the current unit being simulated
                            int const ZoneNum,             // number of zone being served
                            bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                            Real64 &PowerMet,              // power supplied
                            Real64 &LatOutputProvided      // Latent power supplied (kg/s), negative = dehumidification
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Young Tae Chae, Rick Strand
        //       DATE WRITTEN   June 2008
        //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine mainly controls the action of the outdoor air unit
        // (or more exactly, it controls the coil outlet temperature of the unit)
        // based on the user input for controls and the defined controls
        // algorithms.

        // METHODOLOGY EMPLOYED:
        // Outdoor air unit is controlled based on user input and what is happening in the
        // simulation.
        // Note: controls are strictly temperature based and do not factor
        // humidity into the equation (not an enthalpy economy cycle but rather
        // a simple return air cycle).

        // REFERENCES:
        // ASHRAE Systems and Equipment Handbook (SI), 1996. page 31.3

        // USE STATEMENTS:

        // Using/Aliasing
        auto &thisOutAirUnit = state.dataOutdoorAirUnit->OutAirUnit(OAUnitNum);

        auto &TurnFansOff = state.dataHVACGlobal->TurnFansOff;
        auto &TurnFansOn = state.dataHVACGlobal->TurnFansOn;
        using HeatingCoils::CheckHeatingCoilSchedule;
        using HVACHXAssistedCoolingCoil::CheckHXAssistedCoolingCoilSchedule;

        // Locals

        // SUBROUTINE ARGUMENT DEFINITIONS:

        // SUBROUTINE PARAMETER DEFINITIONS:
        // INTERFACE BLOCK SPECIFICATIONS

        // DERIVED TYPE DEFINITIONS
        // na

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 DesOATemp;      // Design OA Temp degree C
        Real64 AirMassFlow;    // air mass flow rate [kg/s]
        Real64 QTotUnitOut;    // total unit output [watts]
        Real64 QUnitOut = 0.0; // heating or sens. cooling provided by fan coil unit [watts]
        Real64 LatLoadMet;     // heating or sens. cooling provided by fan coil unit [watts]
        Real64 MinHumRat;      // desired temperature after mixing inlet and outdoor air [degrees C]
        Real64 SetPointTemp;   // temperature that will be used to control the radiant system [Celsius]
        Real64 HiCtrlTemp;     // Current high point in setpoint temperature range
        Real64 LoCtrlTemp;     // Current low point in setpoint temperature range
        Real64 AirInEnt;       // RE-calculate the Enthalpy of supply air
        Real64 AirOutletTemp = 0.0;
        Real64 ZoneSupAirEnt; // Specific humidity ratio of inlet air (kg moisture / kg moist air)
        // Latent output
        Real64 LatentOutput; // Latent (moisture) add/removal rate, negative is dehumidification [kg/s]
        Real64 SpecHumOut;   // Specific humidity ratio of outlet air (kg moisture / kg moist air)
        Real64 SpecHumIn;    // Specific humidity ratio of inlet air (kg moisture / kg moist air)
        Real64 ZoneAirEnt;   // zone air enthalpy J/kg

        // initialize local variables
        int const InletNode = thisOutAirUnit.AirInletNode;        // Unit air inlet node, only used if ExtFan
        int const SFanOutletNode = thisOutAirUnit.SFanOutletNode; // Unit supply fan outlet node
        int const OutletNode = thisOutAirUnit.AirOutletNode;      // air outlet node
        int const OutsideAirNode = thisOutAirUnit.OutsideAirNode; // outside air node
        OAUnitCtrlType const UnitControlType = thisOutAirUnit.controlType;

        thisOutAirUnit.CompOutSetTemp = 0.0;
        thisOutAirUnit.FanEffect = false;

        if ((thisOutAirUnit.availSched->getCurrentVal() <= 0) || (thisOutAirUnit.outAirSched->getCurrentVal() <= 0) ||
            ((thisOutAirUnit.supFanAvailSched->getCurrentVal() <= 0) && !TurnFansOn) || TurnFansOff) {
            // System is off or has no load upon the unit; set the flow rates to zero and then
            // simulate the components with the no flow conditions
            if (thisOutAirUnit.ExtFan) {
                state.dataLoopNodes->Node(InletNode).MassFlowRate = 0.0;
                state.dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = 0.0;
                state.dataLoopNodes->Node(InletNode).MassFlowRateMinAvail = 0.0;
            }
            state.dataLoopNodes->Node(SFanOutletNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(SFanOutletNode).MassFlowRateMaxAvail = 0.0;
            state.dataLoopNodes->Node(SFanOutletNode).MassFlowRateMinAvail = 0.0;
            state.dataLoopNodes->Node(OutletNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(OutletNode).MassFlowRateMaxAvail = 0.0;
            state.dataLoopNodes->Node(OutletNode).MassFlowRateMinAvail = 0.0;
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMaxAvail = 0.0;
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMinAvail = 0.0;

            // Node condition
            if (thisOutAirUnit.ExtFan) {
                state.dataLoopNodes->Node(InletNode).Temp = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).MAT;
                state.dataLoopNodes->Node(SFanOutletNode).Temp = state.dataLoopNodes->Node(InletNode).Temp;
            } else {
                state.dataLoopNodes->Node(SFanOutletNode).Temp = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).MAT;
            }
            state.dataLoopNodes->Node(OutletNode).Temp = state.dataLoopNodes->Node(SFanOutletNode).Temp;

            if (thisOutAirUnit.supFanPlace == HVAC::FanPlace::BlowThru) {
                state.dataFans->fans(thisOutAirUnit.SFan_Index)->simulate(state, FirstHVACIteration, _);

                SimZoneOutAirUnitComps(state, OAUnitNum, FirstHVACIteration);
                if (thisOutAirUnit.ExtFan) {
                    state.dataFans->fans(thisOutAirUnit.ExtFan_Index)->simulate(state, FirstHVACIteration, _, _);
                }

            } else if (thisOutAirUnit.supFanPlace == HVAC::FanPlace::DrawThru) {
                SimZoneOutAirUnitComps(state, OAUnitNum, FirstHVACIteration);
                state.dataFans->fans(thisOutAirUnit.SFan_Index)->simulate(state, FirstHVACIteration, _, _);

                if (thisOutAirUnit.ExtFan) {
                    state.dataFans->fans(thisOutAirUnit.ExtFan_Index)->simulate(state, FirstHVACIteration, _, _);
                }
            }

        } else { // System On

            // Flowrate Check
            if (state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate > 0.0) {
                state.dataLoopNodes->Node(OutsideAirNode).MassFlowRate = thisOutAirUnit.OutAirMassFlow;
            }

            // Fan Positioning Check

            if (thisOutAirUnit.ExtFan) {
                state.dataLoopNodes->Node(InletNode).MassFlowRate = thisOutAirUnit.ExtAirMassFlow;
            }

            // Air mass balance check
            if ((std::abs(thisOutAirUnit.ExtAirMassFlow - thisOutAirUnit.OutAirMassFlow) > 0.001) &&
                (!state.dataHeatBal->ZoneAirMassFlow.EnforceZoneMassBalance)) {
                if (!thisOutAirUnit.FlowError) {
                    ShowWarningError(state, "Air mass flow between zone supply and exhaust is not balanced. Only the first occurrence is reported.");
                    ShowContinueError(state, format("Occurs in ZoneHVAC:OutdoorAirUnit Object= {}", thisOutAirUnit.Name));
                    ShowContinueError(state,
                                      "Air mass balance is required by other outdoor air units: Fan:ZoneExhaust, ZoneMixing, ZoneCrossMixing, or "
                                      "other air flow control inputs.");
                    ShowContinueErrorTimeStamp(state,
                                               format("The outdoor mass flow rate = {:.3R} and the exhaust mass flow rate = {:.3R}.",
                                                      thisOutAirUnit.OutAirMassFlow,
                                                      thisOutAirUnit.ExtAirMassFlow));
                    thisOutAirUnit.FlowError = true;
                }
            }

            if (thisOutAirUnit.supFanPlace == HVAC::FanPlace::BlowThru) {
                state.dataFans->fans(thisOutAirUnit.SFan_Index)->simulate(state, FirstHVACIteration, _, _);
                DesOATemp = state.dataLoopNodes->Node(SFanOutletNode).Temp;
            } else if (thisOutAirUnit.supFanPlace == HVAC::FanPlace::DrawThru) {
                DesOATemp = state.dataLoopNodes->Node(OutsideAirNode).Temp;
            }

            // Control type check
            switch (UnitControlType) {
            case OAUnitCtrlType::Neutral: {
                SetPointTemp = state.dataZoneTempPredictorCorrector->zoneHeatBalance(ZoneNum).MAT;
                // Neutral Control Condition
                if (DesOATemp == SetPointTemp) {
                    thisOutAirUnit.OperatingMode = Operation::NeutralMode;
                    AirOutletTemp = DesOATemp;
                    thisOutAirUnit.CompOutSetTemp = DesOATemp;
                    SimZoneOutAirUnitComps(state, OAUnitNum, FirstHVACIteration);
                } else {
                    if (DesOATemp < SetPointTemp) { // Heating MODE
                        thisOutAirUnit.OperatingMode = Operation::HeatingMode;
                        AirOutletTemp = SetPointTemp;
                        thisOutAirUnit.CompOutSetTemp = AirOutletTemp;
                        SimZoneOutAirUnitComps(state, OAUnitNum, FirstHVACIteration);
                    } else { // Cooling Mode
                        thisOutAirUnit.OperatingMode = Operation::CoolingMode;
                        AirOutletTemp = SetPointTemp;
                        thisOutAirUnit.CompOutSetTemp = AirOutletTemp;
                        SimZoneOutAirUnitComps(state, OAUnitNum, FirstHVACIteration);
                    }
                }
                // SetPoint Temperature Condition
            } break;
            case OAUnitCtrlType::Temperature: {
                SetPointTemp = DesOATemp;
                HiCtrlTemp = thisOutAirUnit.hiCtrlTempSched->getCurrentVal();
                LoCtrlTemp = thisOutAirUnit.loCtrlTempSched->getCurrentVal();
                if ((DesOATemp <= HiCtrlTemp) && (DesOATemp >= LoCtrlTemp)) {
                    thisOutAirUnit.OperatingMode = Operation::NeutralMode;
                    AirOutletTemp = DesOATemp;
                    thisOutAirUnit.CompOutSetTemp = DesOATemp;
                    SimZoneOutAirUnitComps(state, OAUnitNum, FirstHVACIteration);
                } else {
                    if (SetPointTemp < LoCtrlTemp) {
                        thisOutAirUnit.OperatingMode = Operation::HeatingMode;
                        AirOutletTemp = LoCtrlTemp;
                        thisOutAirUnit.CompOutSetTemp = AirOutletTemp;
                        SimZoneOutAirUnitComps(state, OAUnitNum, FirstHVACIteration);
                    } else if (SetPointTemp > HiCtrlTemp) {
                        thisOutAirUnit.OperatingMode = Operation::CoolingMode;
                        AirOutletTemp = HiCtrlTemp;
                        thisOutAirUnit.CompOutSetTemp = AirOutletTemp;
                        SimZoneOutAirUnitComps(state, OAUnitNum, FirstHVACIteration);
                    }
                }
            } break;
            default:
                break;
            }

            // Fan positioning
            if (thisOutAirUnit.supFanPlace == HVAC::FanPlace::DrawThru) {
                state.dataFans->fans(thisOutAirUnit.SFan_Index)->simulate(state, FirstHVACIteration, _, _);

                thisOutAirUnit.FanEffect = true; // RE-Simulation to take over the supply fan effect
                thisOutAirUnit.FanCorTemp = (state.dataLoopNodes->Node(OutletNode).Temp - thisOutAirUnit.CompOutSetTemp);
                SimZoneOutAirUnitComps(state, OAUnitNum, FirstHVACIteration);
                state.dataFans->fans(thisOutAirUnit.SFan_Index)->simulate(state, FirstHVACIteration, _, _);
                thisOutAirUnit.FanEffect = false;
            }
            if (thisOutAirUnit.ExtFan) {
                state.dataFans->fans(thisOutAirUnit.ExtFan_Index)->simulate(state, FirstHVACIteration, _, _);
            }
        } // ...end of system ON/OFF IF-THEN block

        AirMassFlow = state.dataLoopNodes->Node(OutletNode).MassFlowRate;
        MinHumRat = min(state.dataLoopNodes->Node(OutletNode).HumRat, state.dataLoopNodes->Node(thisOutAirUnit.ZoneNodeNum).HumRat);

        AirInEnt = PsyHFnTdbW(state.dataLoopNodes->Node(OutletNode).Temp, MinHumRat);                   // zone supply air node enthalpy
        ZoneAirEnt = PsyHFnTdbW(state.dataLoopNodes->Node(thisOutAirUnit.ZoneNodeNum).Temp, MinHumRat); // zone air enthalpy
        QUnitOut = AirMassFlow * (AirInEnt - ZoneAirEnt);                                               // Senscooling

        // CR9155 Remove specific humidity calculations
        SpecHumOut = state.dataLoopNodes->Node(OutletNode).HumRat;
        SpecHumIn = state.dataLoopNodes->Node(thisOutAirUnit.ZoneNodeNum).HumRat;
        LatentOutput = AirMassFlow * (SpecHumOut - SpecHumIn); // Latent rate (kg/s), dehumid = negative

        ZoneAirEnt =
            PsyHFnTdbW(state.dataLoopNodes->Node(thisOutAirUnit.ZoneNodeNum).Temp, state.dataLoopNodes->Node(thisOutAirUnit.ZoneNodeNum).HumRat);

        ZoneSupAirEnt = PsyHFnTdbW(state.dataLoopNodes->Node(OutletNode).Temp, state.dataLoopNodes->Node(OutletNode).HumRat);
        QTotUnitOut = AirMassFlow * (ZoneSupAirEnt - ZoneAirEnt);
        LatLoadMet = QTotUnitOut - QUnitOut; // watts

        // Report variables...

        if (QUnitOut < 0.0) {
            thisOutAirUnit.SensCoolingRate = std::abs(QUnitOut);
            thisOutAirUnit.SensHeatingRate = 0.0;
        } else {
            thisOutAirUnit.SensCoolingRate = 0.0;
            thisOutAirUnit.SensHeatingRate = QUnitOut;
        }

        if (QTotUnitOut < 0.0) {
            thisOutAirUnit.TotCoolingRate = std::abs(QTotUnitOut);
            thisOutAirUnit.TotHeatingRate = 0.0;
        } else {
            thisOutAirUnit.TotCoolingRate = 0.0;
            thisOutAirUnit.TotHeatingRate = QTotUnitOut;
        }

        if (LatLoadMet < 0.0) {
            thisOutAirUnit.LatCoolingRate = std::abs(LatLoadMet);
            thisOutAirUnit.LatHeatingRate = 0.0;
        } else {
            thisOutAirUnit.LatCoolingRate = 0.0;
            thisOutAirUnit.LatHeatingRate = LatLoadMet;
        }

        // OutAirUnit( OAUnitNum ).ElecFanRate = FanElecPower;  //Issue #5524 this would only get the last fan called, not both if there are two
        thisOutAirUnit.ElecFanRate = 0.0;
        thisOutAirUnit.ElecFanRate += state.dataFans->fans(thisOutAirUnit.SFan_Index)->totalPower;

        if (thisOutAirUnit.ExtFan) {
            thisOutAirUnit.ElecFanRate += state.dataFans->fans(thisOutAirUnit.ExtFan_Index)->totalPower;
        }

        PowerMet = QUnitOut;
        LatOutputProvided = LatentOutput;
    }

    void SimZoneOutAirUnitComps(EnergyPlusData &state, int const OAUnitNum, bool const FirstHVACIteration)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   Oct 1998
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE
        // Simulate the controllers and components in the outside air system.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool const Sim = true;

        auto &thisOutAirUnit = state.dataOutdoorAirUnit->OutAirUnit(OAUnitNum);
        for (int EquipNum = 1; EquipNum <= thisOutAirUnit.NumComponents; ++EquipNum) {
            auto &thisOAEquip = thisOutAirUnit.OAEquip(EquipNum);
            SimOutdoorAirEquipComps(state,
                                    OAUnitNum,
                                    CompTypeNames[static_cast<int>(thisOAEquip.Type)],
                                    thisOAEquip.ComponentName,
                                    EquipNum,
                                    thisOAEquip.Type,
                                    FirstHVACIteration,
                                    thisOAEquip.ComponentIndex,
                                    Sim);
        }
    }

    void SimOutdoorAirEquipComps(EnergyPlusData &state,
                                 int const OAUnitNum,          // actual outdoor air unit num
                                 std::string_view EquipType,   // the component type
                                 std::string const &EquipName, // the component Name
                                 int const EquipNum,
                                 [[maybe_unused]] CompType const CompTypeNum, // Component Type -- Integerized for this module
                                 bool const FirstHVACIteration,
                                 int &CompIndex,
                                 bool const Sim // if TRUE, simulate component
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Young Tae Chae, Rick Strand
        //       DATE WRITTEN   June 2008
        //       MODIFIED
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // Outdoor air unit has various coil options. This subroutine defines the coil loads and execute
        // to simulate each components
        // METHODOLOGY EMPLOYED:

        // REFERENCES:

        // USE STATEMENTS:

        // Using/Aliasing
        using DesiccantDehumidifiers::SimDesiccantDehumidifier;
        using HeatRecovery::SimHeatRecovery;
        using HVAC::SmallLoad;
        using HVACDXHeatPumpSystem::SimDXHeatPumpSystem;
        using HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil;
        using WaterCoils::SimulateWaterCoilComponents;

        // SUBROUTINE LOCAL VARIABLE DEFINITIONS
        Real64 QCompReq;
        Real64 MaxWaterFlow;
        Real64 MinWaterFlow;
        Real64 QUnitOut;
        Real64 Dxsystemouttemp;

        auto &thisOutAirUnit = state.dataOutdoorAirUnit->OutAirUnit(OAUnitNum);
        auto &thisOAEquip = thisOutAirUnit.OAEquip(EquipNum);
        int const InletNodeNum = thisOAEquip.CoilAirInletNode;
        int const OutletNodeNum = thisOAEquip.CoilAirOutletNode;

        int UnitNum = OAUnitNum;
        int SimCompNum = EquipNum;

        Real64 const CompAirOutTemp = thisOutAirUnit.CompOutSetTemp;
        Operation const OpMode = thisOutAirUnit.OperatingMode;
        CompType const EquipTypeNum = thisOAEquip.Type;
        Real64 const OAMassFlow = thisOutAirUnit.OutAirMassFlow;

        // check the fan positioning
        bool const DrawFan = thisOutAirUnit.FanEffect;
        Real64 const FanEffect = DrawFan ? thisOutAirUnit.FanCorTemp : 0.0;

        // checking equipment index

        {
            switch (EquipTypeNum) {
            // Heat recovery
            case CompType::HeatXchngrFP: // 'HeatExchanger:AirToAir:FlatPlate',
            case CompType::HeatXchngrSL: // 'HeatExchanger:AirToAir:SensibleAndLatent',
                                         // 'HeatExchanger:Desiccant:BalancedFlow' - unused
            {

                if (Sim) {
                    SimHeatRecovery(state, EquipName, FirstHVACIteration, CompIndex, HVAC::FanOp::Continuous, _, _, _, _, false, false);
                }
            } break;
            // Desiccant Dehumidifier
            case CompType::Desiccant: { // 'Dehumidifier:Desiccant:NoFans'
                if (Sim) {
                    SimDesiccantDehumidifier(state, EquipName, FirstHVACIteration, CompIndex);
                }

            } break;
            case CompType::WaterCoil_SimpleHeat: { // ('Coil:Heating:Water')

                if (Sim) {
                    int const ControlNode = thisOAEquip.CoilWaterInletNode;
                    MaxWaterFlow = thisOAEquip.MaxWaterMassFlow;
                    MinWaterFlow = thisOAEquip.MinWaterMassFlow;
                    // On the first HVAC iteration the system values are given to the controller, but after that
                    // the demand limits are in place and there needs to be feedback to the Zone Equipment
                    if ((!FirstHVACIteration) && (ControlNode > 0)) {
                        MaxWaterFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRateMaxAvail;
                        MinWaterFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRateMinAvail;
                    }
                    auto const &whCoilInletNode = state.dataLoopNodes->Node(InletNodeNum);
                    // auto &whCoilOutletNode = state.dataLoopNodes->Node(OutletNodeNum);

                    Real64 const CpAirZn = PsyCpAirFnW(whCoilInletNode.HumRat);

                    if ((OpMode == Operation::NeutralMode) || (OpMode == Operation::CoolingMode) || (whCoilInletNode.Temp > CompAirOutTemp)) {
                        QCompReq = 0.0;
                    } else {
                        QCompReq = CpAirZn * OAMassFlow * ((CompAirOutTemp - whCoilInletNode.Temp) - FanEffect);
                        if (std::abs(QCompReq) < SmallLoad) {
                            QCompReq = 0.0;
                        }
                        if (QCompReq < 0.0) {
                            QCompReq = 0.0; // coil can heat only
                        }
                    }

                    ControlCompOutput(state,
                                      thisOutAirUnit.Name,
                                      std::string(ZoneHVACOAUnit),
                                      UnitNum,
                                      FirstHVACIteration,
                                      QCompReq,
                                      ControlNode,
                                      MaxWaterFlow,
                                      MinWaterFlow,
                                      0.0001,
                                      thisOutAirUnit.ControlCompTypeNum,
                                      thisOutAirUnit.CompErrIndex,
                                      _,
                                      _,
                                      _,
                                      2,
                                      SimCompNum,
                                      thisOAEquip.plantLoc);
                }
            } break;
            case CompType::SteamCoil_AirHeat: { // 'Coil:Heating:Steam'
                if (Sim) {
                    CalcOAUnitCoilComps(state, UnitNum, FirstHVACIteration, SimCompNum, QUnitOut);
                }
            } break;
            case CompType::Coil_ElectricHeat: // 'Coil:Heating:Electric'
            case CompType::Coil_GasHeat: {    // 'Coil:Heating:Fuel'
                if (Sim) {
                    //     stand-alone coils are temperature controlled (do not pass QCoilReq in argument list, QCoilReq overrides temp SP)
                    CalcOAUnitCoilComps(state, UnitNum, FirstHVACIteration, SimCompNum, QUnitOut);
                }
            } break;
                // water cooling coil Types
            case CompType::WaterCoil_Cooling: { // 'Coil:Cooling:Water'
                if (Sim) {
                    int const ControlNode = thisOAEquip.CoilWaterInletNode;
                    MaxWaterFlow = thisOAEquip.MaxWaterMassFlow;
                    MinWaterFlow = thisOAEquip.MinWaterMassFlow;
                    // On the first HVAC iteration the system values are given to the controller, but after that
                    // the demand limits are in place and there needs to be feedback to the Zone Equipment
                    if ((!FirstHVACIteration) && (ControlNode > 0)) {
                        MaxWaterFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRateMaxAvail;
                        MinWaterFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRateMinAvail;
                    }

                    auto const &wcCoilInletNode = state.dataLoopNodes->Node(InletNodeNum);
                    auto &wcCoilOutletNode = state.dataLoopNodes->Node(OutletNodeNum);

                    Real64 const CpAirZn = PsyCpAirFnW(wcCoilInletNode.HumRat);
                    if ((OpMode == Operation::NeutralMode) || (OpMode == Operation::HeatingMode) || (wcCoilInletNode.Temp < CompAirOutTemp)) {
                        QCompReq = 0.0;
                        wcCoilOutletNode.Temp = wcCoilInletNode.Temp;
                        wcCoilOutletNode.HumRat = wcCoilInletNode.HumRat;
                        wcCoilOutletNode.MassFlowRate = wcCoilInletNode.MassFlowRate;

                    } else {

                        QCompReq = CpAirZn * OAMassFlow * ((CompAirOutTemp - wcCoilInletNode.Temp) - FanEffect);
                        if (std::abs(QCompReq) < SmallLoad) {
                            QCompReq = 0.0;
                        }
                        if (QCompReq > 0.0) {
                            QCompReq = 0.0; // coil can cool only
                        }
                    }

                    ControlCompOutput(state,
                                      thisOutAirUnit.Name,
                                      std::string(ZoneHVACOAUnit),
                                      UnitNum,
                                      FirstHVACIteration,
                                      QCompReq,
                                      ControlNode,
                                      MaxWaterFlow,
                                      MinWaterFlow,
                                      0.001,
                                      thisOutAirUnit.ControlCompTypeNum,
                                      thisOutAirUnit.CompErrIndex,
                                      _,
                                      _,
                                      _,
                                      1,
                                      SimCompNum,
                                      thisOAEquip.plantLoc);
                }
            } break;
            case CompType::WaterCoil_DetailedCool: { // 'Coil:Cooling:Water:DetailedGeometry'
                if (Sim) {
                    int const ControlNode = thisOAEquip.CoilWaterInletNode;
                    MaxWaterFlow = thisOAEquip.MaxWaterMassFlow;
                    MinWaterFlow = thisOAEquip.MinWaterMassFlow;
                    // On the first HVAC iteration the system values are given to the controller, but after that
                    // the demand limits are in place and there needs to be feedback to the Zone Equipment
                    if ((!FirstHVACIteration) && (ControlNode > 0)) {
                        MaxWaterFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRateMaxAvail;
                        MinWaterFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRateMinAvail;
                    }
                    auto const &wcCoilInletNode = state.dataLoopNodes->Node(InletNodeNum);
                    // auto &wcCoilOutletNode = state.dataLoopNodes->Node(OutletNodeNum);

                    Real64 const CpAirZn = PsyCpAirFnW(wcCoilInletNode.HumRat);

                    if ((OpMode == Operation::NeutralMode) || (OpMode == Operation::HeatingMode) || (wcCoilInletNode.Temp < CompAirOutTemp)) {
                        QCompReq = 0.0;
                    } else {

                        QCompReq = CpAirZn * OAMassFlow * ((CompAirOutTemp - wcCoilInletNode.Temp) - FanEffect);
                        if (std::abs(QCompReq) < SmallLoad) {
                            QCompReq = 0.0;
                        }
                        if (QCompReq > 0.0) {
                            QCompReq = 0.0; // coil can cool only
                        }
                    }

                    ControlCompOutput(state,
                                      thisOutAirUnit.Name,
                                      "ZONEHVAC:OUTDOORAIRUNIT",
                                      UnitNum,
                                      FirstHVACIteration,
                                      QCompReq,
                                      ControlNode,
                                      MaxWaterFlow,
                                      MinWaterFlow,
                                      0.001,
                                      thisOutAirUnit.ControlCompTypeNum,
                                      thisOutAirUnit.CompErrIndex,
                                      _,
                                      _,
                                      _,
                                      1,
                                      SimCompNum,
                                      thisOAEquip.plantLoc);
                }
            } break;
            case CompType::WaterCoil_CoolingHXAsst: { // 'CoilSystem:Cooling:Water:HeatExchangerAssisted'
                if (Sim) {
                    int const ControlNode = thisOAEquip.CoilWaterInletNode;
                    MaxWaterFlow = thisOAEquip.MaxWaterMassFlow;
                    MinWaterFlow = 0.0;
                    // On the first HVAC iteration the system values are given to the controller, but after that
                    // the demand limits are in place and there needs to be feedback to the Zone Equipment
                    if ((!FirstHVACIteration) && (ControlNode > 0)) {
                        MaxWaterFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRateMaxAvail;
                        MinWaterFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRateMinAvail;
                    }
                    auto const &wcCoilInletNode = state.dataLoopNodes->Node(InletNodeNum);
                    // auto &wcCoilOutletNode = state.dataLoopNodes->Node(OutletNodeNum);

                    Real64 const CpAirZn = PsyCpAirFnW(wcCoilInletNode.HumRat);
                    if ((OpMode == Operation::NeutralMode) || (OpMode == Operation::HeatingMode) || (wcCoilInletNode.Temp < CompAirOutTemp)) {
                        QCompReq = 0.0;
                    } else {
                        QCompReq = CpAirZn * OAMassFlow * ((CompAirOutTemp - wcCoilInletNode.Temp) - FanEffect);
                        if (std::abs(QCompReq) < SmallLoad) {
                            QCompReq = 0.0;
                        }
                        if (QCompReq > 0.0) {
                            QCompReq = 0.0; // coil can cool only
                        }
                    }
                    ControlCompOutput(state,
                                      thisOutAirUnit.Name,
                                      "ZONEHVAC:OUTDOORAIRUNIT",
                                      UnitNum,
                                      FirstHVACIteration,
                                      QCompReq,
                                      ControlNode,
                                      MaxWaterFlow,
                                      MinWaterFlow,
                                      0.001,
                                      thisOutAirUnit.ControlCompTypeNum,
                                      thisOutAirUnit.CompErrIndex,
                                      _,
                                      _,
                                      _,
                                      1,
                                      SimCompNum,
                                      thisOAEquip.plantLoc);
                }
            } break;
            case CompType::DXSystem: { // CoilSystem:Cooling:DX  old 'CompType:UnitaryCoolOnly'
                if (Sim) {
                    if (thisOAEquip.compPointer == nullptr) {
                        UnitarySystems::UnitarySys thisSys;
                        thisOAEquip.compPointer =
                            thisSys.factory(state, HVAC::UnitarySysType::Unitary_AnyCoilType, thisOAEquip.ComponentName, false, OAUnitNum);
                        UnitarySystems::UnitarySys::checkUnitarySysCoilInOASysExists(state, thisOAEquip.ComponentName, OAUnitNum);
                    }
                    if (((OpMode == Operation::NeutralMode) && (thisOutAirUnit.controlType == OAUnitCtrlType::Temperature)) ||
                        (OpMode == Operation::HeatingMode)) {
                        Dxsystemouttemp = 100.0; // There is no cooling demand for the DX system.
                    } else {
                        Dxsystemouttemp = CompAirOutTemp - FanEffect;
                    }
                    Real64 sensOut = 0.0;
                    Real64 latOut = 0.0;
                    int DXSystemIndex = 0;
                    thisOAEquip.compPointer->simulate(state,
                                                      EquipName,
                                                      FirstHVACIteration,
                                                      -1,
                                                      DXSystemIndex,
                                                      state.dataOutdoorAirUnit->HeatActive,
                                                      state.dataOutdoorAirUnit->CoolActive,
                                                      UnitNum,
                                                      Dxsystemouttemp,
                                                      false,
                                                      sensOut,
                                                      latOut);
                }
            } break;
            case CompType::DXHeatPumpSystem: {
                if (Sim) {
                    if (((OpMode == Operation::NeutralMode) && (thisOutAirUnit.controlType == OAUnitCtrlType::Temperature)) ||
                        (OpMode == Operation::CoolingMode)) {
                        Dxsystemouttemp = -20.0; // There is no heating demand for the DX system.
                    } else {
                        Dxsystemouttemp = CompAirOutTemp - FanEffect;
                    }
                    int DXSystemIndex = 0;
                    SimDXHeatPumpSystem(state, EquipName, FirstHVACIteration, -1, DXSystemIndex, UnitNum, Dxsystemouttemp);
                }
            } break;
                // RAR need new CompType:UnitarySystem object here
            case CompType::UnitarySystemModel: { // 'CompType:UnitarySystem'
                if (Sim) {
                    // This may have to be done in the unitary system object since there can be both cooling and heating
                    if (((OpMode == Operation::NeutralMode) && (thisOutAirUnit.controlType == OAUnitCtrlType::Temperature)) ||
                        (OpMode == Operation::HeatingMode)) {
                        Dxsystemouttemp = 100.0; // There is no cooling demand.
                    } else if (((OpMode == Operation::NeutralMode) && (thisOutAirUnit.controlType == OAUnitCtrlType::Temperature)) ||
                               (OpMode == Operation::CoolingMode)) {
                        Dxsystemouttemp = -20.0; // There is no heating demand.
                    } else {
                        Dxsystemouttemp = CompAirOutTemp - FanEffect;
                    }
                    Real64 sensOut = 0.0;
                    Real64 latOut = 0.0;
                    int DXSystemIndex = 0;
                    thisOAEquip.compPointer->simulate(state,
                                                      EquipName,
                                                      FirstHVACIteration,
                                                      -1,
                                                      DXSystemIndex,
                                                      state.dataOutdoorAirUnit->HeatActive,
                                                      state.dataOutdoorAirUnit->CoolActive,
                                                      UnitNum,
                                                      Dxsystemouttemp,
                                                      false,
                                                      sensOut,
                                                      latOut);
                }
            } break;
            default: {
                ShowFatalError(state, format("Invalid Outdoor Air Unit Component={}", EquipType)); // validate
            } break;
            }
        }
    }

    void CalcOAUnitCoilComps(EnergyPlusData &state,
                             int const CompNum, // actual outdoor air unit num
                             bool const FirstHVACIteration,
                             int const EquipIndex, // Component Type -- Integerized for this module
                             Real64 &LoadMet)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Young Tae Chae, Rick Strand
        //       DATE WRITTEN   June 2009
        //       MODIFIED
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine mainly controls the action of water components in the unit

        // METHODOLOGY EMPLOYED:

        // REFERENCES:

        // USE STATEMENTS:

        // Using/Aliasing
        using HVAC::SmallLoad;
        using HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil;
        using SteamCoils::SimulateSteamCoilComponents;
        using WaterCoils::SimulateWaterCoilComponents;

        // SUBROUTINE ARGUMENT DEFINITIONS:

        // Locals
        // SUBROUTINE LOCAL VARIABLE DEFINITIONS
        int CoilIndex = 0;

        auto &thisOutAirUnit = state.dataOutdoorAirUnit->OutAirUnit(CompNum);
        auto &thisOAEquip = thisOutAirUnit.OAEquip(EquipIndex);
        int const InletNodeNum = thisOAEquip.CoilAirInletNode;
        int const OutletNodeNum = thisOAEquip.CoilAirOutletNode;
        auto const &oaInletNode = state.dataLoopNodes->Node(InletNodeNum);
        auto &oaOutletNode = state.dataLoopNodes->Node(OutletNodeNum);

        CompType const CoilTypeNum = thisOAEquip.Type;
        Operation const OpMode = thisOutAirUnit.OperatingMode;
        Real64 const CoilAirOutTemp = thisOutAirUnit.CompOutSetTemp;
        bool const DrawFan = thisOutAirUnit.FanEffect;
        Real64 const FanEffect = DrawFan ? thisOutAirUnit.FanCorTemp : 0.0;

        // Actual equipment load
        auto setupQCompReq = [&OpMode, &oaInletNode, &oaOutletNode, &CoilAirOutTemp, &FanEffect]() -> Real64 {
            Real64 QCompReq = 0.0;
            if ((OpMode == Operation::NeutralMode) || (OpMode == Operation::CoolingMode) || (oaInletNode.Temp > CoilAirOutTemp)) {
                QCompReq = 0.0;
            } else {
                oaOutletNode.MassFlowRate = oaInletNode.MassFlowRate;
                Real64 const CpAirZn = PsyCpAirFnW(oaInletNode.HumRat);
                QCompReq = oaInletNode.MassFlowRate * CpAirZn * ((CoilAirOutTemp - oaInletNode.Temp) - FanEffect);
                if (std::abs(QCompReq) < SmallLoad) {
                    QCompReq = 0.0;
                }
            }
            if (QCompReq <= 0.0) {
                QCompReq = 0.0; // a heating coil can only heat, not cool
                oaOutletNode.Temp = oaInletNode.Temp;
                oaOutletNode.HumRat = oaInletNode.HumRat;
                oaOutletNode.MassFlowRate = oaInletNode.MassFlowRate;
            }
            return QCompReq;
        };

        switch (CoilTypeNum) {
        case CompType::Coil_ElectricHeat: {
            Real64 const QCompReq = setupQCompReq();
            HeatingCoils::SimulateHeatingCoilComponents(state, thisOAEquip.ComponentName, FirstHVACIteration, QCompReq, CoilIndex);
            Real64 const AirMassFlow = oaInletNode.MassFlowRate;
            LoadMet = AirMassFlow * (PsyHFnTdbW(oaOutletNode.Temp, oaInletNode.HumRat) - PsyHFnTdbW(oaInletNode.Temp, oaInletNode.HumRat));

        } break;
        case CompType::Coil_GasHeat: { // 'Coil:Heating:Steam'
            Real64 const QCompReq = setupQCompReq();
            HeatingCoils::SimulateHeatingCoilComponents(state, thisOAEquip.ComponentName, FirstHVACIteration, QCompReq, CoilIndex);
            Real64 const AirMassFlow = oaInletNode.MassFlowRate;
            LoadMet = AirMassFlow * (PsyHFnTdbW(oaOutletNode.Temp, oaInletNode.HumRat) - PsyHFnTdbW(oaInletNode.Temp, oaInletNode.HumRat));

        } break;
        case CompType::SteamCoil_AirHeat: { // 'Coil:Heating:Steam'
            Real64 const QCompReq = setupQCompReq();
            SimulateSteamCoilComponents(state, thisOAEquip.ComponentName, FirstHVACIteration, CoilIndex, QCompReq);
            Real64 const AirMassFlow = oaInletNode.MassFlowRate;
            LoadMet = AirMassFlow * (PsyHFnTdbW(oaOutletNode.Temp, oaInletNode.HumRat) - PsyHFnTdbW(oaInletNode.Temp, oaInletNode.HumRat));

        } break;
        case CompType::WaterCoil_SimpleHeat: // 'Coil:Heating:Water')
        case CompType::WaterCoil_Cooling:    // 'Coil:Cooling:Water'
        case CompType::WaterCoil_DetailedCool: {
            SimulateWaterCoilComponents(state, thisOAEquip.ComponentName, FirstHVACIteration, CoilIndex);
            Real64 const AirMassFlow = oaInletNode.MassFlowRate;
            LoadMet = AirMassFlow * (PsyHFnTdbW(oaOutletNode.Temp, oaInletNode.HumRat) - PsyHFnTdbW(oaInletNode.Temp, oaInletNode.HumRat));

        } break;
        case CompType::WaterCoil_CoolingHXAsst: {
            SimHXAssistedCoolingCoil(
                state, thisOAEquip.ComponentName, FirstHVACIteration, HVAC::CompressorOp::On, 0.0, CoilIndex, HVAC::FanOp::Continuous);
            Real64 const AirMassFlow = oaInletNode.MassFlowRate;
            LoadMet = AirMassFlow * (PsyHFnTdbW(oaOutletNode.Temp, oaInletNode.HumRat) - PsyHFnTdbW(oaInletNode.Temp, oaInletNode.HumRat));
        } break;
        default:
            ShowFatalError(state, format("Invalid Coil Type = {}", CoilTypeNum)); // validate
            break;
        }
    }

    // SUBROUTINE UpdateOutdoorAirUnit

    // No update routine needed in this module since all of the updates happen on
    // the Node derived type directly and these updates are done by other routines.

    // END SUBROUTINE UpdateOutdoorAirUnit

    void ReportOutdoorAirUnit(EnergyPlusData &state,
                              int const OAUnitNum) // Index for the outdoor air unit under consideration within the derived types
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Young T. Chae
        //       DATE WRITTEN   Oct. 2009
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine simply produces output for the outdoor air unit.
        // METHODOLOGY EMPLOYED:
        // Standard EnergyPlus methodology.

        // Using/Aliasing
        Real64 const TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

        auto &thisOutAirUnit = state.dataOutdoorAirUnit->OutAirUnit(OAUnitNum);
        thisOutAirUnit.TotHeatingEnergy = thisOutAirUnit.TotHeatingRate * TimeStepSysSec;
        thisOutAirUnit.SensHeatingEnergy = thisOutAirUnit.SensHeatingRate * TimeStepSysSec;
        thisOutAirUnit.LatHeatingEnergy = thisOutAirUnit.LatHeatingRate * TimeStepSysSec;
        thisOutAirUnit.SensCoolingEnergy = thisOutAirUnit.SensCoolingRate * TimeStepSysSec;
        thisOutAirUnit.LatCoolingEnergy = thisOutAirUnit.LatCoolingRate * TimeStepSysSec;
        thisOutAirUnit.TotCoolingEnergy = thisOutAirUnit.TotCoolingRate * TimeStepSysSec;
        thisOutAirUnit.AirMassFlow = thisOutAirUnit.OutAirMassFlow;
        thisOutAirUnit.ElecFanEnergy = thisOutAirUnit.ElecFanRate * TimeStepSysSec;

        if (thisOutAirUnit.FirstPass) { // reset sizing flags so other zone equipment can size normally
            if (!state.dataGlobal->SysSizingCalc) {
                DataSizing::resetHVACSizingGlobals(state, state.dataSize->CurZoneEqNum, 0, thisOutAirUnit.FirstPass);
            }
        }
    }

    int GetOutdoorAirUnitOutAirNode(EnergyPlusData &state, int const OAUnitNum)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   Dec  2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // lookup function for OA inlet node

        // Return value
        int GetOutdoorAirUnitOutAirNode = 0;

        if (state.dataOutdoorAirUnit->GetOutdoorAirUnitInputFlag) {
            OutdoorAirUnit::GetOutdoorAirUnitInputs(state);
            state.dataOutdoorAirUnit->GetOutdoorAirUnitInputFlag = false;
        }

        if (OAUnitNum > 0 && OAUnitNum <= state.dataOutdoorAirUnit->NumOfOAUnits) {
            GetOutdoorAirUnitOutAirNode = state.dataOutdoorAirUnit->OutAirUnit(OAUnitNum).OutsideAirNode;
        }

        return GetOutdoorAirUnitOutAirNode;
    }

    int GetOutdoorAirUnitZoneInletNode(EnergyPlusData &state, int const OAUnitNum)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   Dec  2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // lookup function for OA inlet node

        // Return value
        int GetOutdoorAirUnitZoneInletNode = 0;

        if (state.dataOutdoorAirUnit->GetOutdoorAirUnitInputFlag) {
            OutdoorAirUnit::GetOutdoorAirUnitInputs(state);
            state.dataOutdoorAirUnit->GetOutdoorAirUnitInputFlag = false;
        }

        if (OAUnitNum > 0 && OAUnitNum <= state.dataOutdoorAirUnit->NumOfOAUnits) {
            GetOutdoorAirUnitZoneInletNode = state.dataOutdoorAirUnit->OutAirUnit(OAUnitNum).AirOutletNode;
        }

        return GetOutdoorAirUnitZoneInletNode;
    }

    int GetOutdoorAirUnitReturnAirNode(EnergyPlusData &state, int const OAUnitNum)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   Dec  2006
        //       MODIFIED       na
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS FUNCTION:
        // lookup function for OA inlet node

        // Return value
        int GetOutdoorAirUnitReturnAirNode = 0;

        if (state.dataOutdoorAirUnit->GetOutdoorAirUnitInputFlag) {
            OutdoorAirUnit::GetOutdoorAirUnitInputs(state);
            state.dataOutdoorAirUnit->GetOutdoorAirUnitInputFlag = false;
        }

        if (OAUnitNum > 0 && OAUnitNum <= state.dataOutdoorAirUnit->NumOfOAUnits) {
            GetOutdoorAirUnitReturnAirNode = state.dataOutdoorAirUnit->OutAirUnit(OAUnitNum).AirInletNode;
        }

        return GetOutdoorAirUnitReturnAirNode;
    }

    int getOutdoorAirUnitEqIndex(EnergyPlusData &state, std::string_view EquipName)
    {
        if (state.dataOutdoorAirUnit->GetOutdoorAirUnitInputFlag) {
            OutdoorAirUnit::GetOutdoorAirUnitInputs(state);
            state.dataOutdoorAirUnit->GetOutdoorAirUnitInputFlag = false;
        }

        for (int OAUnitNum = 1; OAUnitNum <= state.dataOutdoorAirUnit->NumOfOAUnits; ++OAUnitNum) {
            if (Util::SameString(state.dataOutdoorAirUnit->OutAirUnit(OAUnitNum).Name, EquipName)) {
                return OAUnitNum;
            }
        }

        return 0;
    }

} // namespace OutdoorAirUnit

} // namespace EnergyPlus

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
#include <EnergyPlus/Autosizing/HeatingAirFlowSizing.hh>
#include <EnergyPlus/Autosizing/HeatingCapacitySizing.hh>
#include <EnergyPlus/Autosizing/SystemAirFlowSizing.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataContaminantBalance.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/HVACHXAssistedCoolingCoil.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SingleDuct.hh>
#include <EnergyPlus/SteamCoils.hh>
#include <EnergyPlus/UnitVentilator.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterCoils.hh>
#include <EnergyPlus/ZonePlenum.hh>

namespace EnergyPlus {

namespace UnitVentilator {

    // Module containing the routines dealing with the Unit Ventilator

    // MODULE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   May 2000
    //       MODIFIED       March 2001   (addition of gas and electric coils)
    //                      October 2003 (addition of cooling coil type)
    //       MODIFIED       Bereket Nigusse, FSEC, October 2013, Added cycling fan operating mode

    // PURPOSE OF THIS MODULE:
    // To simulate unit ventilators.

    // METHODOLOGY EMPLOYED:
    // Units are modeled as a collection of components: outside air mixer,
    // fan, heating coil and/or cooling coil plus an integrated control
    // algorithm that adjusts the hot or cold water flow to meet the zone
    // load.  Outside air mixing is handled locally as either fixed percent
    // or as attempting to meet a prescribed mixed air temperature.

    // REFERENCES:
    // ASHRAE Systems and Equipment Handbook (SI), 1996. pp. 31.1-31.3
    // Fred Buhl's fan coil module (FanCoilUnits.cc)

    static constexpr std::array<std::string_view, static_cast<int>(CoilsUsed::Num)> CoilsUsedNamesUC = {
        "NONE", "HEATINGANDCOOLING", "HEATING", "COOLING"};
    static constexpr std::array<std::string_view, static_cast<int>(OAControl::Num)> OAControlNamesUC = {
        "VARIABLEPERCENT", "FIXEDTEMPERATURE", "FIXEDAMOUNT"};
    static constexpr std::array<std::string_view, static_cast<int>(HeatCoilType::Num)> HeatCoilTypeNamesUC = {
        "COIL:HEATING:ELECTRIC", "COIL:HEATING:FUEL", "COIL:HEATING:WATER", "COIL:HEATING:STEAM"};
    static constexpr std::array<std::string_view, static_cast<int>(CoolCoilType::Num)> CoolCoilTypeNamesUC = {
        "COIL:COOLING:WATER", "COIL:COOLING:WATER:DETAILEDGEOMETRY", "COILSYSTEM:COOLING:WATER:HEATEXCHANGERASSISTED"};

    void SimUnitVentilator(EnergyPlusData &state,
                           std::string_view CompName,     // name of the fan coil unit
                           int const ZoneNum,             // number of zone being served
                           bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                           Real64 &PowerMet,              // Sensible power supplied (W)
                           Real64 &LatOutputProvided,     // Latent add/removal supplied by window AC (kg/s), dehumid = negative
                           int &CompIndex)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   May 2000
        //       MODIFIED       Don Shirey, Aug 2009 (LatOutputProvided)

        // PURPOSE OF THIS SUBROUTINE:
        // This is the main driver subroutine for the Unit Ventilator simulation.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int UnitVentNum; // index of unit ventilator being simulated

        if (state.dataUnitVentilators->GetUnitVentilatorInputFlag) {
            GetUnitVentilatorInput(state);
            state.dataUnitVentilators->GetUnitVentilatorInputFlag = false;
        }

        // Find the correct Unit Ventilator Equipment
        if (CompIndex == 0) {
            UnitVentNum = Util::FindItemInList(CompName, state.dataUnitVentilators->UnitVent);
            if (UnitVentNum == 0) {
                ShowFatalError(state, format("SimUnitVentilator: Unit not found={}", CompName));
            }
            CompIndex = UnitVentNum;
        } else {
            UnitVentNum = CompIndex;
            if (UnitVentNum > state.dataUnitVentilators->NumOfUnitVents || UnitVentNum < 1) {
                ShowFatalError(state,
                               format("SimUnitVentilator:  Invalid CompIndex passed={}, Number of Units={}, Entered Unit name={}",
                                      UnitVentNum,
                                      state.dataUnitVentilators->NumOfUnitVents,
                                      CompName));
            }
            if (state.dataUnitVentilators->CheckEquipName(UnitVentNum)) {
                if (CompName != state.dataUnitVentilators->UnitVent(UnitVentNum).Name) {
                    ShowFatalError(state,
                                   format("SimUnitVentilator: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                          UnitVentNum,
                                          CompName,
                                          state.dataUnitVentilators->UnitVent(UnitVentNum).Name));
                }
                state.dataUnitVentilators->CheckEquipName(UnitVentNum) = false;
            }
        }

        state.dataSize->ZoneEqUnitVent = true;

        InitUnitVentilator(state, UnitVentNum, FirstHVACIteration, ZoneNum);

        CalcUnitVentilator(state, UnitVentNum, ZoneNum, FirstHVACIteration, PowerMet, LatOutputProvided);

        ReportUnitVentilator(state, UnitVentNum);

        state.dataSize->ZoneEqUnitVent = false;
    }

    void GetUnitVentilatorInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   May 2000
        //       MODIFIED       Chandan Sharma, FSEC, March 2011: Added zone sys avail manager
        //                      Bereket Nigusse, FSEC, April 2011: eliminated input node names
        //                                                         & added fan object type

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine obtains the input for unit ventilators and sets
        // up the appropriate derived type.

        // REFERENCES:
        // Fred Buhl's fan coil module (FanCoilUnits.cc)

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetUnitVentilatorInput: "); // include trailing blank
        static constexpr std::string_view routineName = "GetUnitVentilatorInput";  // include trailing blank

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool ErrorsFound(false);       // Set to true if errors in input, fatal at end of routine
        int IOStatus;                  // Used in GetObjectItem
        bool IsNotOK;                  // TRUE if there was a problem with a list name
        int NumFields;                 // Total number of fields in object
        int NumAlphas;                 // Number of Alphas for each GetObjectItem call
        int NumNumbers;                // Number of Numbers for each GetObjectItem call
        bool IsValid;                  // Set for outside air node check
        bool errFlag(false);           // interim error flag
        std::string cCoolingCoilType;  // Cooling coil object type
        std::string cHeatingCoilType;  // Heating coil object type
        Real64 FanVolFlow;             // volumetric flow rate of fan
        Array1D_string Alphas;         // Alpha items for object
        Array1D<Real64> Numbers;       // Numeric items for object
        Array1D_string cAlphaFields;   // Alpha field names
        Array1D_string cNumericFields; // Numeric field names
        Array1D_bool lAlphaBlanks;     // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;   // Logical array, numeric field input BLANK = .TRUE.
        bool ZoneNodeNotFound;         // used in error checking

        // Figure out how many unit ventilators there are in the input file

        std::string CurrentModuleObject = state.dataUnitVentilators->cMO_UnitVentilator;
        state.dataUnitVentilators->NumOfUnitVents = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumFields, NumAlphas, NumNumbers);

        Alphas.allocate(NumAlphas);
        Numbers.dimension(NumNumbers, 0.0);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNumbers);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNumbers, true);

        // Allocate the local derived type and do one-time initializations for all parts of it
        if (state.dataUnitVentilators->NumOfUnitVents > 0) {
            state.dataUnitVentilators->UnitVent.allocate(state.dataUnitVentilators->NumOfUnitVents);
            state.dataUnitVentilators->CheckEquipName.allocate(state.dataUnitVentilators->NumOfUnitVents);
            state.dataUnitVentilators->UnitVentNumericFields.allocate(state.dataUnitVentilators->NumOfUnitVents);
        }
        state.dataUnitVentilators->CheckEquipName = true;

        // Loop over all of the unit ventilators found in the input file.
        for (int UnitVentNum = 1; UnitVentNum <= state.dataUnitVentilators->NumOfUnitVents; ++UnitVentNum) {

            auto &unitVent = state.dataUnitVentilators->UnitVent(UnitVentNum);

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     UnitVentNum,
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

            state.dataUnitVentilators->UnitVentNumericFields(UnitVentNum).FieldNames.allocate(NumNumbers);
            state.dataUnitVentilators->UnitVentNumericFields(UnitVentNum).FieldNames = "";
            state.dataUnitVentilators->UnitVentNumericFields(UnitVentNum).FieldNames = cNumericFields;
            Util::IsNameEmpty(state, Alphas(1), CurrentModuleObject, ErrorsFound);

            unitVent.Name = Alphas(1);
            if (lAlphaBlanks(2)) {
                unitVent.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((unitVent.availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ErrorsFound = true;
            }

            unitVent.MaxAirVolFlow = Numbers(1);

            // Outside air information:
            unitVent.MinOutAirVolFlow = Numbers(2);

            if (lAlphaBlanks(4)) {
                ShowSevereEmptyField(state, eoh, cAlphaFields(4));
                ErrorsFound = true;
            } else if ((unitVent.minOASched = Sched::GetSchedule(state, Alphas(4))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(4), Alphas(4));
                ErrorsFound = true;
            }

            unitVent.OutAirVolFlow = Numbers(3);
            cCoolingCoilType = "";
            cHeatingCoilType = "";

            unitVent.OAControlType = static_cast<OAControl>(getEnumValue(OAControlNamesUC, Alphas(3)));
            switch (unitVent.OAControlType) {
            case OAControl::VariablePercent:
            case OAControl::FixedAmount: {
                if (lAlphaBlanks(5)) {
                    ShowSevereEmptyField(state, eoh, cAlphaFields(5));
                    ErrorsFound = true;
                } else if ((unitVent.maxOASched = Sched::GetSchedule(state, Alphas(5))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(5), Alphas(5));
                    ErrorsFound = true;
                } else if (!unitVent.maxOASched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 1.0)) {
                    Sched::ShowSevereBadMinMax(state, eoh, cAlphaFields(5), Alphas(5), Clusive::In, 0.0, Clusive::In, 1.0);
                    ErrorsFound = true;
                }
            } break;

            case OAControl::FixedTemperature: {
                if (lAlphaBlanks(5)) {
                    ShowSevereEmptyField(state, eoh, cAlphaFields(5));
                    ErrorsFound = true;
                } else if ((unitVent.tempSched = Sched::GetSchedule(state, Alphas(5))) == nullptr) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(5), Alphas(5));
                    ErrorsFound = true;
                }
            } break;

            default: {
                assert(false);
            } break;
            } // end (switch)

            // Main air nodes (except outside air node):
            // For node connections, this object is both a parent and a non-parent, because the
            // OA mixing box is not called out as a separate component, its nodes must be connected
            // as ObjectIsNotParent.  But for the fan and coils, the nodes are connected as ObjectIsParent
            // To support the diagramming tool, the unit ventilator inlet node must appear both as
            // an inlet to the unit ventilator parent object and as an inlet to the implied
            // non-parent OA mixing box within the unit ventilator.
            // Because there is overlap between the nodes that are parent and non-parent, use a different
            // object type for the non parent nodes
            unitVent.AirInNode = NodeInputManager::GetOnlySingleNode(state,
                                                                     Alphas(6),
                                                                     ErrorsFound,
                                                                     DataLoopNode::ConnectionObjectType::ZoneHVACUnitVentilator,
                                                                     Alphas(1),
                                                                     DataLoopNode::NodeFluidType::Air,
                                                                     DataLoopNode::ConnectionType::Inlet,
                                                                     NodeInputManager::CompFluidStream::Primary,
                                                                     DataLoopNode::ObjectIsParent);
            unitVent.AirOutNode = NodeInputManager::GetOnlySingleNode(state,
                                                                      Alphas(7),
                                                                      ErrorsFound,
                                                                      DataLoopNode::ConnectionObjectType::ZoneHVACUnitVentilator,
                                                                      Alphas(1),
                                                                      DataLoopNode::NodeFluidType::Air,
                                                                      DataLoopNode::ConnectionType::Outlet,
                                                                      NodeInputManager::CompFluidStream::Primary,
                                                                      DataLoopNode::ObjectIsParent);

            // Get AirTerminal mixer data
            SingleDuct::GetATMixer(state,
                                   unitVent.Name,
                                   unitVent.ATMixerName,
                                   unitVent.ATMixerIndex,
                                   unitVent.ATMixerType,
                                   unitVent.ATMixerPriNode,
                                   unitVent.ATMixerSecNode,
                                   unitVent.ATMixerOutNode,
                                   unitVent.AirOutNode);
            if (unitVent.ATMixerType == HVAC::MixerType::InletSide || unitVent.ATMixerType == HVAC::MixerType::SupplySide) {
                unitVent.ATMixerExists = true;
            }
            unitVent.ZonePtr =
                DataZoneEquipment::GetZoneEquipControlledZoneNum(state, DataZoneEquipment::ZoneEquipType::UnitVentilator, unitVent.Name);
            if (unitVent.ZonePtr == 0) {
                ErrorsFound = true;
            }

            if (!unitVent.ATMixerExists) {
                unitVent.AirInNode = NodeInputManager::GetOnlySingleNode(state,
                                                                         Alphas(6),
                                                                         ErrorsFound,
                                                                         DataLoopNode::ConnectionObjectType::ZoneHVACUnitVentilator,
                                                                         Alphas(1) + "-OA MIXER",
                                                                         DataLoopNode::NodeFluidType::Air,
                                                                         DataLoopNode::ConnectionType::Inlet,
                                                                         NodeInputManager::CompFluidStream::Primary,
                                                                         DataLoopNode::ObjectIsNotParent);
            }

            unitVent.FanName = Alphas(12);
            unitVent.fanType = static_cast<HVAC::FanType>(getEnumValue(HVAC::fanTypeNamesUC, Alphas(11)));

            if (unitVent.fanType != HVAC::FanType::Constant && unitVent.fanType != HVAC::FanType::VAV && unitVent.fanType != HVAC::FanType::OnOff &&
                unitVent.fanType != HVAC::FanType::SystemModel) {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(11), Alphas(11));
                ShowContinueError(state, "Fan Type must be Fan:OnOff, Fan:ConstantVolume, Fan:VariableVolume, or Fan:SystemModel");
                ErrorsFound = true;

            } else if ((unitVent.Fan_Index = Fans::GetFanIndex(state, unitVent.FanName)) == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(12), unitVent.FanName);
                ErrorsFound = true;

            } else {
                auto *fan = state.dataFans->fans(unitVent.Fan_Index);
                unitVent.FanOutletNode = fan->outletNodeNum;
                unitVent.fanAvailSched = fan->availSched; // Get the fan's availability schedule
                FanVolFlow = fan->maxAirFlowRate;
                if (FanVolFlow != DataSizing::AutoSize && unitVent.MaxAirVolFlow != DataSizing::AutoSize && FanVolFlow < unitVent.MaxAirVolFlow) {
                    ShowSevereError(state, format("{}{}=\"{}\".", RoutineName, CurrentModuleObject, unitVent.Name));
                    ShowContinueError(state,
                                      format("...air flow rate [{:.7T}] in fan object {} is less than the unit ventilator maximum "
                                             "supply air flow rate [{:.7T}].",
                                             FanVolFlow,
                                             unitVent.FanName,
                                             unitVent.MaxAirVolFlow));
                    ShowContinueError(state,
                                      "...the fan flow rate must be greater than or equal to the unit ventilator maximum supply "
                                      "air flow rate.");
                    ErrorsFound = true;
                } else if (FanVolFlow == DataSizing::AutoSize && unitVent.MaxAirVolFlow != DataSizing::AutoSize) {
                    ShowWarningError(state, format("{}{}=\"{}\".", RoutineName, CurrentModuleObject, unitVent.Name));
                    ShowContinueError(state, "...the fan flow rate is autosized while the unit ventilator flow rate is not.");
                    ShowContinueError(state, "...this can lead to unexpected results where the fan flow rate is less than required.");
                } else if (FanVolFlow != DataSizing::AutoSize && unitVent.MaxAirVolFlow == DataSizing::AutoSize) {
                    ShowWarningError(state, format("{}{}=\"{}\".", RoutineName, CurrentModuleObject, unitVent.Name));
                    ShowContinueError(state, "...the unit ventilator flow rate is autosized while the fan flow rate is not.");
                    ShowContinueError(state, "...this can lead to unexpected results where the fan flow rate is less than required.");
                }
            }

            // For node connections, this object is both a parent and a non-parent, because the
            // OA mixing box is not called out as a separate component, its nodes must be connected
            // as ObjectIsNotParent.  But for the fan and coils, the nodes are connected as ObjectIsParent
            // Because there is overlap between the nodes that are parent and non-parent, use a different
            // object type for the non parent nodes
            //  Set connection type to 'OutdoorAir', because this is hardwired to OA conditions
            if (!unitVent.ATMixerExists) {
                unitVent.OutsideAirNode = NodeInputManager::GetOnlySingleNode(state,
                                                                              Alphas(8),
                                                                              ErrorsFound,
                                                                              DataLoopNode::ConnectionObjectType::ZoneHVACUnitVentilator,
                                                                              Alphas(1) + "-OA MIXER",
                                                                              DataLoopNode::NodeFluidType::Air,
                                                                              DataLoopNode::ConnectionType::OutsideAirReference,
                                                                              NodeInputManager::CompFluidStream::Primary,
                                                                              DataLoopNode::ObjectIsNotParent);
                if (!lAlphaBlanks(8)) {
                    OutAirNodeManager::CheckAndAddAirNodeNumber(state, unitVent.OutsideAirNode, IsValid);
                    if (!IsValid) {
                        ShowWarningError(state, format("{}{} Adding {}={}", RoutineName, CurrentModuleObject, cAlphaFields(8), Alphas(8)));
                    }
                }

                unitVent.AirReliefNode = NodeInputManager::GetOnlySingleNode(state,
                                                                             Alphas(9),
                                                                             ErrorsFound,
                                                                             DataLoopNode::ConnectionObjectType::ZoneHVACUnitVentilator,
                                                                             Alphas(1) + "-OA MIXER",
                                                                             DataLoopNode::NodeFluidType::Air,
                                                                             DataLoopNode::ConnectionType::ReliefAir,
                                                                             NodeInputManager::CompFluidStream::Primary,
                                                                             DataLoopNode::ObjectIsNotParent);

                unitVent.OAMixerOutNode = NodeInputManager::GetOnlySingleNode(state,
                                                                              Alphas(10),
                                                                              ErrorsFound,
                                                                              DataLoopNode::ConnectionObjectType::ZoneHVACUnitVentilator,
                                                                              Alphas(1) + "-OA MIXER",
                                                                              DataLoopNode::NodeFluidType::Air,
                                                                              DataLoopNode::ConnectionType::Outlet,
                                                                              NodeInputManager::CompFluidStream::Primary,
                                                                              DataLoopNode::ObjectIsNotParent);
            } else {
                unitVent.OutsideAirNode = unitVent.ATMixerPriNode;
                unitVent.OAMixerOutNode = unitVent.ATMixerOutNode;
                if (!lAlphaBlanks(8) || !lAlphaBlanks(9) || !lAlphaBlanks(10)) {
                    ShowWarningError(state, format("{}{}=\"{}\" is connected to central DOA.", RoutineName, CurrentModuleObject, unitVent.Name));
                    if (!lAlphaBlanks(8)) {
                        ShowContinueError(state, format("... input field {} should have been blank. Specified = {}", cAlphaFields(8), Alphas(8)));
                    }
                    if (!lAlphaBlanks(9)) {
                        ShowContinueError(state, format("... input field {} should have been blank. Specified = {}", cAlphaFields(9), Alphas(9)));
                    }
                    if (!lAlphaBlanks(10)) {
                        ShowContinueError(state, format("... input field {} should have been blank. Specified = {}", cAlphaFields(10), Alphas(10)));
                    }
                }
            }

            if (unitVent.OAControlType == OAControl::FixedAmount) {
                unitVent.OutAirVolFlow = unitVent.MinOutAirVolFlow;
                unitVent.maxOASched = unitVent.minOASched;
            }

            if (!unitVent.ATMixerExists) {
                // Add fan to component sets array
                BranchNodeConnections::SetUpCompSets(state,
                                                     CurrentModuleObject,
                                                     unitVent.Name,
                                                     Alphas(11),
                                                     unitVent.FanName,
                                                     state.dataLoopNodes->NodeID(unitVent.OAMixerOutNode),
                                                     state.dataLoopNodes->NodeID(unitVent.FanOutletNode));
            } else {
                if (unitVent.ATMixerType == HVAC::MixerType::InletSide) {
                    // Add fan to component sets array
                    BranchNodeConnections::SetUpCompSets(state,
                                                         CurrentModuleObject,
                                                         unitVent.Name,
                                                         Alphas(11),
                                                         unitVent.FanName,
                                                         state.dataLoopNodes->NodeID(unitVent.ATMixerOutNode),
                                                         state.dataLoopNodes->NodeID(unitVent.FanOutletNode));
                }
                if (unitVent.ATMixerType == HVAC::MixerType::SupplySide) {
                    // Add fan to component sets array
                    BranchNodeConnections::SetUpCompSets(state,
                                                         CurrentModuleObject,
                                                         unitVent.Name,
                                                         Alphas(11),
                                                         unitVent.FanName,
                                                         state.dataLoopNodes->NodeID(unitVent.AirInNode),
                                                         state.dataLoopNodes->NodeID(unitVent.FanOutletNode));
                }
            }

            if (!lAlphaBlanks(19)) {
                unitVent.AvailManagerListName = Alphas(19);
            }

            unitVent.HVACSizingIndex = 0;
            if (!lAlphaBlanks(20)) {
                unitVent.HVACSizingIndex = Util::FindItemInList(Alphas(20), state.dataSize->ZoneHVACSizing);
                if (unitVent.HVACSizingIndex == 0) {
                    ShowSevereError(state, format("{} = {} not found.", cAlphaFields(20), Alphas(20)));
                    ShowContinueError(state, format("Occurs in {} = \"{}\".", CurrentModuleObject, unitVent.Name));
                    ErrorsFound = true;
                }
            }

            unitVent.CoilOption = (CoilsUsed)getEnumValue(CoilsUsedNamesUC, Alphas(13));

            if (lAlphaBlanks(14)) {
                unitVent.fanOp = (unitVent.fanType == HVAC::FanType::OnOff || unitVent.fanType == HVAC::FanType::SystemModel)
                                     ? HVAC::FanOp::Cycling
                                     : HVAC::FanOp::Continuous;
            } else if ((unitVent.fanOpModeSched = Sched::GetSchedule(state, Alphas(14))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(14), Alphas(14));
                ErrorsFound = true;
            } else if ((unitVent.fanType == HVAC::FanType::Constant) &&
                       !unitVent.fanOpModeSched->checkMinMaxVals(state, Clusive::Ex, 0.0, Clusive::In, 1.0)) {
                Sched::ShowSevereBadMinMax(state, eoh, cAlphaFields(14), Alphas(14), Clusive::Ex, 0.0, Clusive::In, 1.0);
                ErrorsFound = true;
            }

            // Get Coil information
            if (unitVent.CoilOption == CoilsUsed::Both || unitVent.CoilOption == CoilsUsed::Heating) {
                if ((!lAlphaBlanks(16))) {
                    unitVent.HCoilPresent = true;
                    cHeatingCoilType = Alphas(15);
                    unitVent.HCoilTypeCh = cHeatingCoilType;
                    unitVent.HCoilType = (HeatCoilType)getEnumValue(HeatCoilTypeNamesUC, cHeatingCoilType);
                    unitVent.HeatingCoilType = (DataPlant::PlantEquipmentType)getEnumValue(DataPlant::PlantEquipTypeNamesUC, cHeatingCoilType);

                    unitVent.HCoilName = Alphas(16);
                    ValidateComponent(state, cHeatingCoilType, unitVent.HCoilName, IsNotOK, CurrentModuleObject);
                    if (IsNotOK) {
                        ShowContinueError(state, format("...specified in {} = \"{}\".", CurrentModuleObject, unitVent.Name));
                        ErrorsFound = true;
                    } else {
                        // The heating coil control node is necessary for a hot water coil, but not necessary for electric or gas.
                        if (unitVent.HCoilType == HeatCoilType::Water || unitVent.HCoilType == HeatCoilType::Steam) {
                            // mine the hot water or steam node from the coil object
                            if (unitVent.HCoilType == HeatCoilType::Water) {
                                unitVent.HCoil_Index = WaterCoils::GetCompIndex(state, WaterCoils::CoilModel::HeatingSimple, unitVent.HCoilName);
                                unitVent.HotControlNode = state.dataWaterCoils->WaterCoil(unitVent.HCoil_Index).WaterInletNodeNum;
                                unitVent.MaxVolHotWaterFlow = state.dataWaterCoils->WaterCoil(unitVent.HCoil_Index).MaxWaterVolFlowRate;
                                // Could probably remove MaxVolHotSteamFlow here
                                unitVent.MaxVolHotSteamFlow = unitVent.MaxVolHotWaterFlow;
                            } else {
                                unitVent.HCoil_Index = SteamCoils::GetCompIndex(state, unitVent.HCoilName);
                                unitVent.HotControlNode = state.dataSteamCoils->SteamCoil(unitVent.HCoil_Index).SteamInletNodeNum;
                                // Could probably replace MaxVolHotWaterFlow here with MaxVolHotSteamFlow
                                unitVent.MaxVolHotWaterFlow = state.dataSteamCoils->SteamCoil(unitVent.HCoil_Index).MaxSteamVolFlowRate;
                                // unitVent.MaxVolHotWaterFlow =
                                //    SteamCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Steam", unitVent.HCoilName, ErrorsFound);
                                unitVent.MaxVolHotSteamFlow = unitVent.MaxVolHotWaterFlow;
                            }
                        }
                    }

                    unitVent.HotControlOffset = Numbers(4);
                    // Set default convergence tolerance
                    if (unitVent.HotControlOffset <= 0.0) {
                        unitVent.HotControlOffset = 0.001;
                    }
                } else { // heating coil is required for these options
                    ShowSevereError(state, format("{}{}=\"{}\".", RoutineName, CurrentModuleObject, unitVent.Name));
                    ShowContinueError(state, format("a heating coil is required for {}=\"{}\".", cAlphaFields(13), Alphas(13)));
                    ErrorsFound = true;
                } // IF (.NOT. lAlphaBlanks(15)) THEN - from the start of heating coil information
            } // is option both or heating only

            if (unitVent.CoilOption == CoilsUsed::Both || unitVent.CoilOption == CoilsUsed::Cooling) {
                if (!lAlphaBlanks(18)) {
                    unitVent.CCoilPresent = true;
                    errFlag = false;

                    cCoolingCoilType = Alphas(17);
                    unitVent.CCoilTypeCh = cCoolingCoilType;
                    unitVent.CCoilType = (CoolCoilType)getEnumValue(CoolCoilTypeNamesUC, cCoolingCoilType);
                    unitVent.CoolingCoilType = (DataPlant::PlantEquipmentType)getEnumValue(DataPlant::PlantEquipTypeNamesUC, cCoolingCoilType);
                    unitVent.CCoilPlantName = Alphas(18);

                    if (cCoolingCoilType == "COILSYSTEM:COOLING:WATER:HEATEXCHANGERASSISTED") {
                        unitVent.CCoilType = CoolCoilType::HXAssisted;
                        HVACHXAssistedCoolingCoil::GetHXCoilTypeAndName(
                            state, cCoolingCoilType, Alphas(18), ErrorsFound, unitVent.CCoilPlantType, unitVent.CCoilPlantName);
                        if (Util::SameString(unitVent.CCoilPlantType, "Coil:Cooling:Water")) {
                            unitVent.CoolingCoilType = DataPlant::PlantEquipmentType::CoilWaterCooling;
                        } else if (Util::SameString(unitVent.CCoilPlantType, "Coil:Cooling:Water:DetailedGeometry")) {
                            unitVent.CoolingCoilType = DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling;
                        } else {
                            ShowSevereError(state, format("{}{}=\"{}\".", RoutineName, CurrentModuleObject, unitVent.Name));
                            ShowContinueError(state, format("For: {}=\"{}\".", cAlphaFields(17), Alphas(17)));
                            ShowContinueError(state, format("Invalid Coil Type={}, Name={}", unitVent.CCoilPlantType, unitVent.CCoilPlantName));
                            ShowContinueError(state,
                                              "must be \"Coil:Cooling:Water\", \"Coil:Cooling:Water:DetailedGeometry\" or, "
                                              "\"CoilSystem:Cooling:Water:HeatExchangerAssisted\".");
                            errFlag = true;
                            ErrorsFound = true;
                        }
                    }

                    if (!errFlag) {
                        unitVent.CCoilName = Alphas(18);
                        ValidateComponent(state, cCoolingCoilType, unitVent.CCoilName, IsNotOK, CurrentModuleObject);
                        if (IsNotOK) {
                            ShowContinueError(state, format("...specified in {} = \"{}\".", CurrentModuleObject, unitVent.Name));
                            ErrorsFound = true;
                        } else {
                            if (unitVent.CCoilType != CoolCoilType::HXAssisted) {
                                WaterCoils::CoilModel coilModel = WaterCoils::CoilModel::CoolingSimple;
                                if (unitVent.CCoilType == CoolCoilType::Detailed) {
                                    coilModel = WaterCoils::CoilModel::CoolingDetailed;
                                }
                                unitVent.CCoil_Index = WaterCoils::GetCompIndex(state, coilModel, unitVent.CCoilName);
                                unitVent.ColdControlNode = state.dataWaterCoils->WaterCoil(unitVent.CCoil_Index).WaterInletNodeNum;
                                unitVent.MaxVolColdWaterFlow = state.dataWaterCoils->WaterCoil(unitVent.CCoil_Index).MaxWaterVolFlowRate;
                            } else {
                                // special case, call the parent and return the child water inlet node and water volume flow rate
                                unitVent.ColdControlNode =
                                    HVACHXAssistedCoolingCoil::GetCoilWaterInletNode(state, unitVent.CCoilTypeCh, unitVent.CCoilName, errFlag);
                                unitVent.MaxVolColdWaterFlow = HVACHXAssistedCoolingCoil::GetCoilMaxWaterFlowRate(
                                    state, "CoilSystem:Cooling:Water:HeatExchangerAssisted", unitVent.CCoilName, errFlag);
                            }
                            // Other error checks should trap before it gets to this point in the code, but including just in case.
                            if (errFlag) {
                                ShowContinueError(state, format("...specified in {} = \"{}\".", CurrentModuleObject, unitVent.Name));
                                ErrorsFound = true;
                            }
                        }
                    }

                    unitVent.MinVolColdWaterFlow = 0.0;
                    unitVent.ColdControlOffset = Numbers(5);
                    // Set default convergence tolerance
                    if (unitVent.ColdControlOffset <= 0.0) {
                        unitVent.ColdControlOffset = 0.001;
                    }
                } else { // Cooling Coil is required for this/these options
                    ShowSevereError(state, format("{}{}=\"{}\".", RoutineName, CurrentModuleObject, unitVent.Name));
                    ShowContinueError(state, format("a cooling coil is required for {}=\"{}\".", cAlphaFields(13), Alphas(13)));
                    ErrorsFound = true;
                } // IF (.NOT. lAlphaBlanks(17)) THEN - from the start of cooling coil information
            }
            if (!unitVent.ATMixerExists) {
                // check that unit ventilator air inlet node is the same as a zone exhaust node
                ZoneNodeNotFound = true;
                for (int NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(unitVent.ZonePtr).NumExhaustNodes; ++NodeNum) {
                    if (unitVent.AirInNode == state.dataZoneEquip->ZoneEquipConfig(unitVent.ZonePtr).ExhaustNode(NodeNum)) {
                        ZoneNodeNotFound = false;
                        break;
                    }
                }
                if (ZoneNodeNotFound) {
                    bool InletNodeFound = false;
                    if (unitVent.ZonePtr > 0) {
                        InletNodeFound = ZonePlenum::ValidateInducedNode(state,
                                                                         unitVent.AirInNode,
                                                                         state.dataZoneEquip->ZoneEquipConfig(unitVent.ZonePtr).NumReturnNodes,
                                                                         state.dataZoneEquip->ZoneEquipConfig(unitVent.ZonePtr).ReturnNode);
                    }
                    if (!InletNodeFound) {
                        ShowSevereError(state,
                                        format("{} = \"{}\". Unit ventilator air inlet node name must be the same either as a zone exhaust node name "
                                               "or an induce air node in ZoePlenum.",
                                               CurrentModuleObject,
                                               unitVent.Name));
                        ShowContinueError(state, "..Zone exhaust node name is specified in ZoneHVAC:EquipmentConnections object.");
                        ShowContinueError(state, "..Induced Air Outlet Node name is specified in AirLoopHVAC:ReturnPlenum object.");
                        ShowContinueError(state,
                                          format("..Unit ventilator unit air inlet node name = {}", state.dataLoopNodes->NodeID(unitVent.AirInNode)));
                        ErrorsFound = true;
                    }
                }
                // check that unit ventilator air outlet node is the same as a zone inlet node.
                ZoneNodeNotFound = true;
                for (int NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(unitVent.ZonePtr).NumInletNodes; ++NodeNum) {
                    if (unitVent.AirOutNode == state.dataZoneEquip->ZoneEquipConfig(unitVent.ZonePtr).InletNode(NodeNum)) {
                        ZoneNodeNotFound = false;
                        break;
                    }
                }
                if (ZoneNodeNotFound) {
                    ShowSevereError(state,
                                    format("{} = \"{}\". Unit ventilator air outlet node name must be the same as a zone inlet node name.",
                                           CurrentModuleObject,
                                           unitVent.Name));
                    ShowContinueError(state, "..Zone inlet node name is specified in ZoneHVAC:EquipmentConnections object.");
                    ShowContinueError(state, format("..Unit ventilator air outlet node name = {}", state.dataLoopNodes->NodeID(unitVent.AirOutNode)));
                    ErrorsFound = true;
                }
            } else {
                if (unitVent.ATMixerType == HVAC::MixerType::InletSide) {
                    // check that unit ventilator air outlet node is the same as a zone inlet node.
                    ZoneNodeNotFound = true;
                    for (int NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(unitVent.ZonePtr).NumInletNodes; ++NodeNum) {
                        if (unitVent.AirOutNode == state.dataZoneEquip->ZoneEquipConfig(unitVent.ZonePtr).InletNode(NodeNum)) {
                            ZoneNodeNotFound = false;
                            break;
                        }
                    }
                    if (ZoneNodeNotFound) {
                        ShowSevereError(state,
                                        format("{} = \"{}\". Unit ventilator air outlet node name must be the same as a zone inlet node name.",
                                               CurrentModuleObject,
                                               unitVent.Name));
                        ShowContinueError(state, "..Zone inlet node name is specified in ZoneHVAC:EquipmentConnections object.");
                        ShowContinueError(state,
                                          format("..Unit ventilator air outlet node name = {}", state.dataLoopNodes->NodeID(unitVent.AirOutNode)));
                        ErrorsFound = true;
                    }

                    // check that the air mixer out node is the unit ventilator air inlet node
                    if (unitVent.AirInNode != unitVent.ATMixerOutNode) {
                        ShowSevereError(state,
                                        format("{} = \"{}\". unit ventilator air inlet node name must be the same as the mixer outlet node name.",
                                               CurrentModuleObject,
                                               unitVent.Name));
                        ShowContinueError(state, "..Air terminal mixer outlet node name is specified in AirTerminal:SingleDuct:Mixer object.");
                        ShowContinueError(state,
                                          format("..Unit ventilator air inlet node name = {}", state.dataLoopNodes->NodeID(unitVent.AirInNode)));
                        ErrorsFound = true;
                    }
                }
                if (unitVent.ATMixerType == HVAC::MixerType::SupplySide) {
                    // check that the mixer secondary air node is the unit ventilator air outlet node
                    if (unitVent.AirOutNode != unitVent.ATMixerSecNode) {
                        ShowSevereError(
                            state,
                            format("{} = \"{}\". unit ventilator air outlet node name must be the same as the mixer secondary air inlet node name.",
                                   CurrentModuleObject,
                                   unitVent.Name));
                        ShowContinueError(state, "..Air terminal mixer secondary node name is specified in AirTerminal:SingleDuct:Mixer object.");
                        ShowContinueError(state,
                                          format("..Unit ventilator air outlet node name = {}", state.dataLoopNodes->NodeID(unitVent.AirOutNode)));
                        ErrorsFound = true;
                    }

                    // check that air teminal mixer outlet node is the same as a zone inlet node.
                    ZoneNodeNotFound = true;
                    for (int NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(unitVent.ZonePtr).NumInletNodes; ++NodeNum) {
                        if (unitVent.ATMixerOutNode == state.dataZoneEquip->ZoneEquipConfig(unitVent.ZonePtr).InletNode(NodeNum)) {
                            ZoneNodeNotFound = false;
                            break;
                        }
                    }
                    if (ZoneNodeNotFound) {
                        ShowSevereError(state,
                                        format("{} = \"{}\". Air mixer outlet node name must be the same as a zone inlet node name.",
                                               CurrentModuleObject,
                                               unitVent.Name));
                        ShowContinueError(state, "..Zone inlet node name is specified in ZoneHVAC:EquipmentConnections object.");
                        ShowContinueError(state,
                                          format("..Air terminal mixer outlet node name = {}", state.dataLoopNodes->NodeID(unitVent.ATMixerOutNode)));
                        ErrorsFound = true;
                    } else {
                        bool ExhastNodeNotFound = true;
                        // check exhaust node
                        for (int NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(unitVent.ZonePtr).NumExhaustNodes; ++NodeNum) {
                            if (unitVent.AirInNode == state.dataZoneEquip->ZoneEquipConfig(unitVent.ZonePtr).ExhaustNode(NodeNum)) {
                                ExhastNodeNotFound = false;
                                break;
                            }
                        }
                        // check induce node
                        if (ExhastNodeNotFound) {
                            bool InletNodeFound = false;
                            if (unitVent.ZonePtr > 0) {
                                InletNodeFound =
                                    ZonePlenum::ValidateInducedNode(state,
                                                                    unitVent.AirInNode,
                                                                    state.dataZoneEquip->ZoneEquipConfig(unitVent.ZonePtr).NumReturnNodes,
                                                                    state.dataZoneEquip->ZoneEquipConfig(unitVent.ZonePtr).ReturnNode);
                            }
                            if (!InletNodeFound) {
                                ShowSevereError(state, format("{}{}=\"{}\".", RoutineName, CurrentModuleObject, unitVent.Name));
                                ShowContinueError(
                                    state,
                                    "..UnitVentilator inlet node name must be the same as either a zone exhaust node name or an induced "
                                    "air node in ZonePlenum.");
                                ShowContinueError(state, "..Zone exhaust node name is specified in ZoneHVAC:EquipmentConnections object.");
                                ShowContinueError(state, "..Induced Air Outlet Node name is specified in AirLoopHVAC:ReturnPlenum object.");
                                ShowContinueError(state,
                                                  format("..UnitVentilator inlet node name = {}", state.dataLoopNodes->NodeID(unitVent.AirInNode)));
                                ErrorsFound = true;
                            }
                        }
                    }
                }
            }
            {
                switch (unitVent.CoilOption) {
                case CoilsUsed::Both: {
                    // Add cooling coil to component sets array when present
                    BranchNodeConnections::SetUpCompSets(state,
                                                         CurrentModuleObject,
                                                         unitVent.Name,
                                                         cCoolingCoilType,
                                                         unitVent.CCoilName,
                                                         state.dataLoopNodes->NodeID(unitVent.FanOutletNode),
                                                         "UNDEFINED");

                    // Add heating coil to component sets array when cooling coil present
                    BranchNodeConnections::SetUpCompSets(state,
                                                         CurrentModuleObject,
                                                         unitVent.Name,
                                                         cHeatingCoilType,
                                                         unitVent.HCoilName,
                                                         "UNDEFINED",
                                                         state.dataLoopNodes->NodeID(unitVent.AirOutNode));
                } break;
                case CoilsUsed::Heating: {
                    // Add heating coil to component sets array when no cooling coil present
                    BranchNodeConnections::SetUpCompSets(state,
                                                         CurrentModuleObject,
                                                         unitVent.Name,
                                                         cHeatingCoilType,
                                                         unitVent.HCoilName,
                                                         state.dataLoopNodes->NodeID(unitVent.FanOutletNode),
                                                         state.dataLoopNodes->NodeID(unitVent.AirOutNode));
                } break;
                case CoilsUsed::Cooling: {
                    // Add cooling coil to component sets array when no heating coil present
                    BranchNodeConnections::SetUpCompSets(state,
                                                         CurrentModuleObject,
                                                         unitVent.Name,
                                                         cCoolingCoilType,
                                                         unitVent.CCoilName,
                                                         state.dataLoopNodes->NodeID(unitVent.FanOutletNode),
                                                         state.dataLoopNodes->NodeID(unitVent.AirOutNode));
                } break;
                default: {
                } break;
                }
            }
        } // ...loop over all of the unit ventilators found in the input file

        Alphas.deallocate();
        Numbers.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        if (ErrorsFound) {
            ShowFatalError(state, format("{}Errors found in input.", RoutineName));
        }

        // Setup Report variables for the Unit Ventilators, CurrentModuleObject='ZoneHVAC:UnitVentilator'
        for (int UnitVentNum = 1; UnitVentNum <= state.dataUnitVentilators->NumOfUnitVents; ++UnitVentNum) {

            auto &unitVent = state.dataUnitVentilators->UnitVent(UnitVentNum);
            auto &coilReportObj = state.dataRptCoilSelection->coilSelectionReportObj;

            SetupOutputVariable(state,
                                "Zone Unit Ventilator Heating Rate",
                                Constant::Units::W,
                                unitVent.HeatPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                unitVent.Name);
            SetupOutputVariable(state,
                                "Zone Unit Ventilator Heating Energy",
                                Constant::Units::J,
                                unitVent.HeatEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                unitVent.Name);
            SetupOutputVariable(state,
                                "Zone Unit Ventilator Total Cooling Rate",
                                Constant::Units::W,
                                unitVent.TotCoolPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                unitVent.Name);
            SetupOutputVariable(state,
                                "Zone Unit Ventilator Total Cooling Energy",
                                Constant::Units::J,
                                unitVent.TotCoolEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                unitVent.Name);
            SetupOutputVariable(state,
                                "Zone Unit Ventilator Sensible Cooling Rate",
                                Constant::Units::W,
                                unitVent.SensCoolPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                unitVent.Name);
            SetupOutputVariable(state,
                                "Zone Unit Ventilator Sensible Cooling Energy",
                                Constant::Units::J,
                                unitVent.SensCoolEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                unitVent.Name);
            SetupOutputVariable(state,
                                "Zone Unit Ventilator Fan Electricity Rate",
                                Constant::Units::W,
                                unitVent.ElecPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                unitVent.Name);
            // Note that the unit vent fan electric is NOT metered because this value is already metered through the fan component
            SetupOutputVariable(state,
                                "Zone Unit Ventilator Fan Electricity Energy",
                                Constant::Units::J,
                                unitVent.ElecEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                unitVent.Name);
            SetupOutputVariable(state,
                                "Zone Unit Ventilator Fan Availability Status",
                                Constant::Units::None,
                                unitVent.availStatus,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                unitVent.Name);
            if (unitVent.fanType == HVAC::FanType::OnOff) {
                SetupOutputVariable(state,
                                    "Zone Unit Ventilator Fan Part Load Ratio",
                                    Constant::Units::None,
                                    unitVent.FanPartLoadRatio,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    unitVent.Name);
            }

            if (unitVent.HCoilPresent) {
                coilReportObj->setCoilSupplyFanInfo(
                    state, unitVent.HCoilName, unitVent.HCoilTypeCh, unitVent.FanName, unitVent.fanType, unitVent.Fan_Index);
            }
            if (unitVent.CCoilPresent) {
                coilReportObj->setCoilSupplyFanInfo(
                    state, unitVent.CCoilName, unitVent.CCoilTypeCh, unitVent.FanName, unitVent.fanType, unitVent.Fan_Index);
            }
        }
    }

    void InitUnitVentilator(EnergyPlusData &state,
                            int const UnitVentNum,         // index for the current unit ventilator
                            bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                            int const ZoneNum              // number of zone being served
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   May 2000
        //       MODIFIED       Chandan Sharma, FSEC, March 2011: Added zone sys avail manager

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine initializes all of the data elements which are necessary
        // to simulate a unit ventilator.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        auto &unitVent = state.dataUnitVentilators->UnitVent(UnitVentNum);

        static constexpr std::string_view RoutineName("InitUnitVentilator");

        bool SetMassFlowRateToZero = false; // TRUE when mass flow rates need to be set to zero

        // Do the one time initializations
        if (state.dataUnitVentilators->MyOneTimeFlag) {

            state.dataUnitVentilators->MyEnvrnFlag.allocate(state.dataUnitVentilators->NumOfUnitVents);
            state.dataUnitVentilators->MySizeFlag.allocate(state.dataUnitVentilators->NumOfUnitVents);
            state.dataUnitVentilators->MyPlantScanFlag.allocate(state.dataUnitVentilators->NumOfUnitVents);
            state.dataUnitVentilators->MyZoneEqFlag.allocate(state.dataUnitVentilators->NumOfUnitVents);
            state.dataUnitVentilators->MyEnvrnFlag = true;
            state.dataUnitVentilators->MySizeFlag = true;
            state.dataUnitVentilators->MyPlantScanFlag = true;
            state.dataUnitVentilators->MyZoneEqFlag = true;
            state.dataUnitVentilators->MyOneTimeFlag = false;
        }

        if (allocated(state.dataAvail->ZoneComp)) {
            auto &availMgr = state.dataAvail->ZoneComp(DataZoneEquipment::ZoneEquipType::UnitVentilator).ZoneCompAvailMgrs(UnitVentNum);
            if (state.dataUnitVentilators->MyZoneEqFlag(UnitVentNum)) { // initialize the name of each availability manager list and zone number
                availMgr.AvailManagerListName = unitVent.AvailManagerListName;
                availMgr.ZoneNum = ZoneNum;
                state.dataUnitVentilators->MyZoneEqFlag(UnitVentNum) = false;
            }
            unitVent.availStatus = availMgr.availStatus;
        }

        if (state.dataUnitVentilators->MyPlantScanFlag(UnitVentNum) && allocated(state.dataPlnt->PlantLoop)) {
            if ((unitVent.HeatingCoilType == DataPlant::PlantEquipmentType::CoilWaterSimpleHeating) ||
                (unitVent.HeatingCoilType == DataPlant::PlantEquipmentType::CoilSteamAirHeating)) {
                bool errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(
                    state, unitVent.HCoilName, unitVent.HeatingCoilType, unitVent.HWplantLoc, errFlag, _, _, _, _, _);
                if (errFlag) {
                    ShowContinueError(state, format("Reference Unit=\"{}\", type=ZoneHVAC:UnitVentilator", unitVent.Name));
                    ShowFatalError(state, "InitUnitVentilator: Program terminated due to previous condition(s).");
                }

                unitVent.HotCoilOutNodeNum = DataPlant::CompData::getPlantComponent(state, unitVent.HWplantLoc).NodeNumOut;
            }
            if ((unitVent.CoolingCoilType == DataPlant::PlantEquipmentType::CoilWaterCooling) ||
                (unitVent.CoolingCoilType == DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling)) {
                bool errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(
                    state, unitVent.CCoilPlantName, unitVent.CoolingCoilType, unitVent.CWPlantLoc, errFlag, _, _, _, _, _);
                if (errFlag) {
                    ShowContinueError(state, format("Reference Unit=\"{}\", type=ZoneHVAC:UnitVentilator", unitVent.Name));
                    ShowFatalError(state, "InitUnitVentilator: Program terminated due to previous condition(s).");
                }

                unitVent.ColdCoilOutNodeNum = DataPlant::CompData::getPlantComponent(state, unitVent.CWPlantLoc).NodeNumOut;
            } else {
                if (unitVent.CCoilPresent) {
                    ShowFatalError(state, format("InitUnitVentilator: Unit={}, invalid cooling coil type. Program terminated.", unitVent.Name));
                }
            }
            state.dataUnitVentilators->MyPlantScanFlag(UnitVentNum) = false;
        } else if (state.dataUnitVentilators->MyPlantScanFlag(UnitVentNum) && !state.dataGlobal->AnyPlantInModel) {
            state.dataUnitVentilators->MyPlantScanFlag(UnitVentNum) = false;
        }

        if (!state.dataUnitVentilators->ZoneEquipmentListChecked && state.dataZoneEquip->ZoneEquipInputsFilled) {
            state.dataUnitVentilators->ZoneEquipmentListChecked = true;
            for (int Loop = 1; Loop <= state.dataUnitVentilators->NumOfUnitVents; ++Loop) {
                if (DataZoneEquipment::CheckZoneEquipmentList(state, "ZoneHVAC:UnitVentilator", state.dataUnitVentilators->UnitVent(Loop).Name)) {
                    continue;
                }
                ShowSevereError(
                    state,
                    format("InitUnitVentilator: Unit=[UNIT VENTILATOR,{}] is not on any ZoneHVAC:EquipmentList.  It will not be simulated.",
                           state.dataUnitVentilators->UnitVent(Loop).Name));
            }
        }

        if (!state.dataGlobal->SysSizingCalc && state.dataUnitVentilators->MySizeFlag(UnitVentNum) &&
            !state.dataUnitVentilators->MyPlantScanFlag(UnitVentNum)) {

            SizeUnitVentilator(state, UnitVentNum);

            state.dataUnitVentilators->MySizeFlag(UnitVentNum) = false;
        }

        int InNode = unitVent.AirInNode;
        int OutNode = unitVent.AirOutNode;
        int OutsideAirNode = unitVent.OutsideAirNode;
        int AirRelNode = unitVent.AirReliefNode;

        // Do the one time initializations
        if (state.dataGlobal->BeginEnvrnFlag && state.dataUnitVentilators->MyEnvrnFlag(UnitVentNum) &&
            !state.dataUnitVentilators->MyPlantScanFlag(UnitVentNum)) {
            Real64 RhoAir = state.dataEnvrn->StdRhoAir;

            // set the mass flow rates from the input volume flow rates
            unitVent.MaxAirMassFlow = RhoAir * unitVent.MaxAirVolFlow;
            unitVent.OutAirMassFlow = RhoAir * unitVent.OutAirVolFlow;
            unitVent.MinOutAirMassFlow = RhoAir * unitVent.MinOutAirVolFlow;
            if (unitVent.OutAirMassFlow > unitVent.MaxAirMassFlow) {
                unitVent.OutAirMassFlow = unitVent.MaxAirMassFlow;
                unitVent.MinOutAirMassFlow = unitVent.OutAirMassFlow * (unitVent.MinOutAirVolFlow / unitVent.OutAirVolFlow);
                ShowWarningError(state,
                                 format("Outdoor air mass flow rate higher than unit flow rate, reset to unit flow rate for {}", unitVent.Name));
            }

            // set the node max and min mass flow rates
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMax = unitVent.OutAirMassFlow;
            state.dataLoopNodes->Node(OutsideAirNode).MassFlowRateMin = 0.0;

            state.dataLoopNodes->Node(OutNode).MassFlowRateMax = unitVent.MaxAirMassFlow;
            state.dataLoopNodes->Node(OutNode).MassFlowRateMin = 0.0;

            state.dataLoopNodes->Node(InNode).MassFlowRateMax = unitVent.MaxAirMassFlow;
            state.dataLoopNodes->Node(InNode).MassFlowRateMin = 0.0;

            if (unitVent.HCoilPresent) { // Only initialize these if a heating coil is actually present

                if (unitVent.HCoilType == HeatCoilType::Water) {

                    Real64 rho =
                        state.dataPlnt->PlantLoop(unitVent.HWplantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);

                    unitVent.MaxHotWaterFlow = rho * unitVent.MaxVolHotWaterFlow;
                    unitVent.MinHotWaterFlow = rho * unitVent.MinVolHotWaterFlow;

                    PlantUtilities::InitComponentNodes(
                        state, unitVent.MinHotWaterFlow, unitVent.MaxHotWaterFlow, unitVent.HotControlNode, unitVent.HotCoilOutNodeNum);
                }
                if (unitVent.HCoilType == HeatCoilType::Steam) {
                    Real64 TempSteamIn = 100.00;
                    Real64 SteamDensity = unitVent.HCoil_fluid->getSatDensity(state, TempSteamIn, 1.0, RoutineName);
                    unitVent.MaxHotSteamFlow = SteamDensity * unitVent.MaxVolHotSteamFlow;
                    unitVent.MinHotSteamFlow = SteamDensity * unitVent.MinVolHotSteamFlow;

                    PlantUtilities::InitComponentNodes(
                        state, unitVent.MinHotSteamFlow, unitVent.MaxHotSteamFlow, unitVent.HotControlNode, unitVent.HotCoilOutNodeNum);
                }
            } //(UnitVent(UnitVentNum)%HCoilPresent)

            if (unitVent.CCoilPresent) { // Only initialize these if a cooling coil is actually present
                Real64 rho = state.dataPlnt->PlantLoop(unitVent.CWPlantLoc.loopNum).glycol->getDensity(state, 5.0, RoutineName);

                unitVent.MaxColdWaterFlow = rho * unitVent.MaxVolColdWaterFlow;
                unitVent.MinColdWaterFlow = rho * unitVent.MinVolColdWaterFlow;
                PlantUtilities::InitComponentNodes(
                    state, unitVent.MinColdWaterFlow, unitVent.MaxColdWaterFlow, unitVent.ColdControlNode, unitVent.ColdCoilOutNodeNum);
            }
            state.dataUnitVentilators->MyEnvrnFlag(UnitVentNum) = false;
        } // ...end start of environment inits

        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataUnitVentilators->MyEnvrnFlag(UnitVentNum) = true;
        }

        // These initializations are done every iteration...

        state.dataUnitVentilators->QZnReq = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputRequired; // zone load needed
        unitVent.FanPartLoadRatio = 0.0;

        if (unitVent.fanOpModeSched != nullptr) {
            unitVent.fanOp = (unitVent.fanOpModeSched->getCurrentVal() == 0.0) ? HVAC::FanOp::Cycling : HVAC::FanOp::Continuous;
        }

        if (unitVent.availSched->getCurrentVal() > 0) {
            if ((unitVent.fanAvailSched->getCurrentVal() > 0 || state.dataHVACGlobal->TurnFansOn) && !state.dataHVACGlobal->TurnFansOff) {
                if ((std::abs(state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputRequired) < HVAC::SmallLoad) ||
                    (state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum))) {
                    SetMassFlowRateToZero = true;
                }
            } else {
                SetMassFlowRateToZero = true;
            }
        } else {
            SetMassFlowRateToZero = true;
        }

        auto &inNode(state.dataLoopNodes->Node(InNode));
        auto &outNode(state.dataLoopNodes->Node(OutNode));
        auto &oaNode(state.dataLoopNodes->Node(OutsideAirNode));
        if (SetMassFlowRateToZero) {
            inNode.MassFlowRate = 0.0;
            inNode.MassFlowRateMaxAvail = 0.0;
            inNode.MassFlowRateMinAvail = 0.0;
            outNode.MassFlowRate = 0.0;
            outNode.MassFlowRateMaxAvail = 0.0;
            outNode.MassFlowRateMinAvail = 0.0;
            oaNode.MassFlowRate = 0.0;
            oaNode.MassFlowRateMaxAvail = 0.0;
            oaNode.MassFlowRateMinAvail = 0.0;
            if (!unitVent.ATMixerExists) {
                auto &relNode(state.dataLoopNodes->Node(AirRelNode));
                relNode.MassFlowRate = 0.0;
                relNode.MassFlowRateMaxAvail = 0.0;
                relNode.MassFlowRateMinAvail = 0.0;
            }
        } else {
            inNode.MassFlowRate = unitVent.MaxAirMassFlow;
            inNode.MassFlowRateMaxAvail = unitVent.MaxAirMassFlow;
            inNode.MassFlowRateMinAvail = unitVent.MaxAirMassFlow;
            outNode.MassFlowRate = unitVent.MaxAirMassFlow;
            outNode.MassFlowRateMaxAvail = unitVent.MaxAirMassFlow;
            outNode.MassFlowRateMinAvail = unitVent.MaxAirMassFlow;
            oaNode.MassFlowRate = unitVent.OutAirMassFlow;
            oaNode.MassFlowRateMaxAvail = unitVent.OutAirMassFlow;
            oaNode.MassFlowRateMinAvail = unitVent.OutAirMassFlow;
            if (!unitVent.ATMixerExists) {
                auto &relNode(state.dataLoopNodes->Node(AirRelNode));
                relNode.MassFlowRate = unitVent.OutAirMassFlow;
                relNode.MassFlowRateMaxAvail = unitVent.OutAirMassFlow;
                relNode.MassFlowRateMinAvail = unitVent.OutAirMassFlow;
            }
        }

        // Initialize the relief air (same as inlet conditions to the unit ventilator...
        // Note that mass flow rates will be taken care of later.
        if (!unitVent.ATMixerExists) {
            state.dataLoopNodes->Node(AirRelNode) = state.dataLoopNodes->Node(InNode);
        }
        state.dataUnitVentilators->OAMassFlowRate = 0.0;

        // Just in case the unit is off and conditions do not get sent through
        // the unit for some reason, set the outlet conditions equal to the inlet
        // conditions of the unit ventilator
        outNode.Temp = inNode.Temp;
        outNode.Press = inNode.Press;
        outNode.HumRat = inNode.HumRat;
        outNode.Enthalpy = inNode.Enthalpy;

        // These initializations only need to be done once at the start of the iterations...
        if (FirstHVACIteration) {
            // Initialize the outside air conditions...
            if (!unitVent.ATMixerExists) {
                state.dataLoopNodes->Node(OutsideAirNode).Temp = state.dataLoopNodes->Node(OutsideAirNode).OutAirDryBulb;
            }
        }
    }

    void SizeUnitVentilator(EnergyPlusData &state, int const UnitVentNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   February 2002
        //       MODIFIED       August 2013 Daeho Kang, add component sizing table entries
        //                      July 2014, B. Nigusse, added scalable sizing

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing Unit Ventilator components for which flow rates have not been
        // specified in the input.

        // METHODOLOGY EMPLOYED:
        // Obtains flow rates from the zone sizing arrays and plant sizing data.

        auto &unitVent = state.dataUnitVentilators->UnitVent(UnitVentNum);
        auto &ZoneEqSizing = state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum);

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("SizeUnitVentilator");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 DesCoolingLoad = 0.0;
        Real64 DesHeatingLoad = 0.0;
        Real64 TempSteamIn = 0.0;
        Real64 EnthSteamInDry = 0.0;
        Real64 EnthSteamOutWet = 0.0;
        Real64 LatentHeatSteam = 0.0;
        Real64 SteamDensity = 0.0;
        int CoilWaterOutletNode = 0;
        std::string CoolingCoilName;
        std::string CoolingCoilType;
        Real64 rho = 0.0;
        Real64 Cp = 0.0;

        Real64 TempSize;  // autosized value of coil input field
        int SizingMethod; // Integer representation of sizing method name (e.g., CoolingAirflowSizing, HeatingAirflowSizing, CoolingCapacitySizing,
                          // HeatingCapacitySizing, etc.)
        bool PrintFlag;   // TRUE when sizing information is reported in the eio file
        int CapSizingMethod(0);    // capacity sizing methods (HeatingDesignCapacity, CapacityPerFloorArea, FractionOfAutosizedCoolingCapacity, and
                                   // FractionOfAutosizedHeatingCapacity )
        Real64 WaterCoilSizDeltaT; // water coil deltaT for design water flow rate autosizing

        int PltSizHeatNum = 0;
        bool ErrorsFound = false;
        bool IsAutoSize = false;
        Real64 OutAirVolFlowDes = 0.0;
        Real64 OutAirVolFlowUser = 0.0;
        Real64 MinOutAirVolFlowDes = 0.0;
        Real64 MinOutAirVolFlowUser = 0.0;
        Real64 MaxVolHotWaterFlowDes = 0.0;
        Real64 MaxVolHotWaterFlowUser = 0.0;
        Real64 MaxVolHotSteamFlowDes = 0.0;
        Real64 MaxVolHotSteamFlowUser = 0.0;
        Real64 MaxVolColdWaterFlowDes = 0.0;
        Real64 MaxVolColdWaterFlowUser = 0.0;
        Real64 CoolingAirVolFlowScalable = 0.0;
        Real64 HeatingAirVolFlowScalable = 0.0;
        state.dataSize->DataScalableSizingON = false;
        state.dataSize->DataScalableCapSizingON = false;
        std::string CompType = state.dataUnitVentilators->cMO_UnitVentilator;
        std::string CompName = unitVent.Name;
        state.dataSize->DataZoneNumber = unitVent.ZonePtr;
        bool DoWaterCoilSizing = false;

        state.dataSize->DataFanType = unitVent.fanType;
        state.dataSize->DataFanIndex = unitVent.Fan_Index;
        // unit ventilator is always blow thru
        state.dataSize->DataFanPlacement = HVAC::FanPlace::BlowThru;

        state.dataSize->ZoneCoolingOnlyFan = (unitVent.CoilOption == CoilsUsed::Both) || (unitVent.CoilOption == CoilsUsed::Cooling);
        state.dataSize->ZoneHeatingOnlyFan = (unitVent.CoilOption == CoilsUsed::Both) || (unitVent.CoilOption == CoilsUsed::Heating);

        if (state.dataSize->CurZoneEqNum > 0) {
            if (unitVent.HVACSizingIndex > 0) {
                auto const &zoneHVACSizing = state.dataSize->ZoneHVACSizing(unitVent.HVACSizingIndex);

                // initialize OA flow for sizing other inputs (e.g., inlet temp, capacity, etc.)
                if (unitVent.OutAirVolFlow == DataSizing::AutoSize) {
                    ZoneEqSizing.OAVolFlow = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).MinOA;
                } else {
                    ZoneEqSizing.OAVolFlow = unitVent.OutAirVolFlow;
                }
                if (unitVent.ATMixerExists) {     // set up ATMixer conditions for scalable capacity sizing
                    ZoneEqSizing.OAVolFlow = 0.0; // Equipment OA flow should always be 0 when ATMixer is used
                    SingleDuct::setATMixerSizingProperties(state, unitVent.ATMixerIndex, unitVent.ZonePtr, state.dataSize->CurZoneEqNum);
                }

                // N1 , \field Maximum Supply Air Flow Rate
                PrintFlag = true;

                if (zoneHVACSizing.CoolingSAFMethod > 0 && state.dataSize->ZoneCoolingOnlyFan && !state.dataSize->ZoneHeatingOnlyFan) {

                    int SAFMethod = zoneHVACSizing.CoolingSAFMethod;
                    SizingMethod = HVAC::CoolingAirflowSizing;
                    ZoneEqSizing.SizingMethod(SizingMethod) = SAFMethod;
                    switch (SAFMethod) {
                    case DataSizing::None:
                    case DataSizing::SupplyAirFlowRate:
                    case DataSizing::FlowPerFloorArea:
                    case DataSizing::FractionOfAutosizedCoolingAirflow: {
                        switch (SAFMethod) {
                        case DataSizing::SupplyAirFlowRate: {
                            if (zoneHVACSizing.MaxCoolAirVolFlow > 0.0) {
                                ZoneEqSizing.AirVolFlow = zoneHVACSizing.MaxCoolAirVolFlow;
                                ZoneEqSizing.SystemAirFlow = true;
                            }
                            TempSize = zoneHVACSizing.MaxCoolAirVolFlow;
                        } break;
                        case DataSizing::FlowPerFloorArea: {
                            ZoneEqSizing.SystemAirFlow = true;
                            ZoneEqSizing.AirVolFlow =
                                zoneHVACSizing.MaxCoolAirVolFlow * state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                            TempSize = ZoneEqSizing.AirVolFlow;
                            state.dataSize->DataScalableSizingON = true;
                        } break;
                        case DataSizing::FractionOfAutosizedCoolingAirflow: {
                            state.dataSize->DataFracOfAutosizedCoolingAirflow = zoneHVACSizing.MaxCoolAirVolFlow;
                            TempSize = DataSizing::AutoSize;
                            state.dataSize->DataScalableSizingON = true;
                        } break;
                        default: {
                            TempSize = zoneHVACSizing.MaxCoolAirVolFlow;
                        } break;
                        }
                        bool errorsFound = false;
                        CoolingAirFlowSizer sizingCoolingAirFlow;
                        sizingCoolingAirFlow.overrideSizingString(state.dataGlobal->isEpJSON ? "maximum_supply_air_flow_rate [m3/s]"
                                                                                             : "Maximum Supply Air Flow Rate [m3/s]");
                        // sizingCoolingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                        sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        CoolingAirVolFlowScalable = sizingCoolingAirFlow.size(state, TempSize, errorsFound);
                    } break;
                    case DataSizing::FlowPerCoolingCapacity: {
                        SizingMethod = HVAC::CoolingCapacitySizing;
                        TempSize = DataSizing::AutoSize;
                        PrintFlag = false;
                        state.dataSize->DataScalableSizingON = true;
                        state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolVolFlow;
                        CoolingCapacitySizer sizerCoolingCapacity;
                        sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        state.dataSize->DataAutosizedCoolingCapacity = sizerCoolingCapacity.size(state, TempSize, ErrorsFound);
                        state.dataSize->DataFlowPerCoolingCapacity = zoneHVACSizing.MaxCoolAirVolFlow;
                        PrintFlag = true;
                        TempSize = DataSizing::AutoSize;
                        bool errorsFound = false;
                        CoolingAirFlowSizer sizingCoolingAirFlow;
                        sizingCoolingAirFlow.overrideSizingString(state.dataGlobal->isEpJSON ? "maximum_supply_air_flow_rate [m3/s]"
                                                                                             : "Maximum Supply Air Flow Rate [m3/s]");
                        // sizingCoolingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                        sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        CoolingAirVolFlowScalable = sizingCoolingAirFlow.size(state, TempSize, errorsFound);
                    } break;
                    default: {
                    } break;
                    }
                    // DataScalableSizingON = false;

                } else if (zoneHVACSizing.HeatingSAFMethod > 0 && state.dataSize->ZoneHeatingOnlyFan && !state.dataSize->ZoneCoolingOnlyFan) {
                    SizingMethod = HVAC::HeatingAirflowSizing;
                    int SAFMethod = zoneHVACSizing.HeatingSAFMethod;
                    ZoneEqSizing.SizingMethod(SizingMethod) = SAFMethod;
                    switch (SAFMethod) {
                    case DataSizing::None:
                    case DataSizing::SupplyAirFlowRate:
                    case DataSizing::FlowPerFloorArea:
                    case DataSizing::FractionOfAutosizedHeatingAirflow: {
                        switch (SAFMethod) {
                        case DataSizing::SupplyAirFlowRate: {
                            if (zoneHVACSizing.MaxHeatAirVolFlow > 0.0) {
                                ZoneEqSizing.AirVolFlow = zoneHVACSizing.MaxHeatAirVolFlow;
                                ZoneEqSizing.SystemAirFlow = true;
                            }
                            TempSize = zoneHVACSizing.MaxHeatAirVolFlow;
                        } break;
                        case DataSizing::FlowPerFloorArea: {
                            ZoneEqSizing.SystemAirFlow = true;
                            ZoneEqSizing.AirVolFlow =
                                zoneHVACSizing.MaxHeatAirVolFlow * state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                            TempSize = ZoneEqSizing.AirVolFlow;
                            state.dataSize->DataScalableSizingON = true;
                        } break;
                        case DataSizing::FractionOfAutosizedHeatingAirflow: {
                            state.dataSize->DataFracOfAutosizedHeatingAirflow = zoneHVACSizing.MaxHeatAirVolFlow;
                            TempSize = DataSizing::AutoSize;
                            state.dataSize->DataScalableSizingON = true;
                        } break;
                        default: {
                            TempSize = zoneHVACSizing.MaxHeatAirVolFlow;
                        } break;
                        }
                        bool errorsFound = false;
                        HeatingAirFlowSizer sizingHeatingAirFlow;
                        sizingHeatingAirFlow.overrideSizingString(state.dataGlobal->isEpJSON ? "maximum_supply_air_flow_rate [m3/s]"
                                                                                             : "Maximum Supply Air Flow Rate [m3/s]");
                        // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                        sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        HeatingAirVolFlowScalable = sizingHeatingAirFlow.size(state, TempSize, errorsFound);
                    } break;
                    case DataSizing::FlowPerHeatingCapacity: {
                        TempSize = DataSizing::AutoSize;
                        PrintFlag = false;
                        state.dataSize->DataScalableSizingON = true;
                        state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatVolFlow;
                        bool errorsFound = false;
                        HeatingCapacitySizer sizerHeatingCapacity;
                        sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        state.dataSize->DataAutosizedHeatingCapacity = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                        state.dataSize->DataFlowPerHeatingCapacity = zoneHVACSizing.MaxHeatAirVolFlow;
                        PrintFlag = true;
                        TempSize = DataSizing::AutoSize;
                        errorsFound = false;
                        HeatingAirFlowSizer sizingHeatingAirFlow;
                        sizingHeatingAirFlow.overrideSizingString(state.dataGlobal->isEpJSON ? "maximum_supply_air_flow_rate [m3/s]"
                                                                                             : "Maximum Supply Air Flow Rate [m3/s]");
                        // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                        sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        HeatingAirVolFlowScalable = sizingHeatingAirFlow.size(state, TempSize, errorsFound);
                    } break;
                    default: {
                    } break;
                    }
                    // DataScalableSizingON = false;
                } else {

                    if (unitVent.CoilOption != CoilsUsed::None) {
                        if (zoneHVACSizing.CoolingSAFMethod > 0) {
                            int SAFMethod = zoneHVACSizing.CoolingSAFMethod;
                            SizingMethod = HVAC::CoolingAirflowSizing;
                            ZoneEqSizing.SizingMethod(SizingMethod) = SAFMethod;
                            switch (SAFMethod) {
                            case DataSizing::None:
                            case DataSizing::SupplyAirFlowRate:
                            case DataSizing::FlowPerFloorArea:
                            case DataSizing::FractionOfAutosizedCoolingAirflow: {
                                switch (SAFMethod) {
                                case DataSizing::SupplyAirFlowRate: {
                                    if (zoneHVACSizing.MaxCoolAirVolFlow > 0.0) {
                                        ZoneEqSizing.AirVolFlow = zoneHVACSizing.MaxCoolAirVolFlow;
                                        ZoneEqSizing.SystemAirFlow = true;
                                    }
                                    TempSize = zoneHVACSizing.MaxCoolAirVolFlow;
                                } break;
                                case DataSizing::FlowPerFloorArea: {
                                    ZoneEqSizing.SystemAirFlow = true;
                                    ZoneEqSizing.AirVolFlow =
                                        zoneHVACSizing.MaxCoolAirVolFlow * state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                                    TempSize = ZoneEqSizing.AirVolFlow;
                                    state.dataSize->DataScalableSizingON = true;
                                } break;
                                case DataSizing::FractionOfAutosizedCoolingAirflow: {
                                    state.dataSize->DataFracOfAutosizedCoolingAirflow = zoneHVACSizing.MaxCoolAirVolFlow;
                                    TempSize = DataSizing::AutoSize;
                                    state.dataSize->DataScalableSizingON = true;
                                } break;
                                default: {
                                    TempSize = zoneHVACSizing.MaxCoolAirVolFlow;
                                } break;
                                }
                                bool errorsFound = false;
                                CoolingAirFlowSizer sizingCoolingAirFlow;
                                sizingCoolingAirFlow.overrideSizingString(state.dataGlobal->isEpJSON ? "maximum_supply_air_flow_rate [m3/s]"
                                                                                                     : "Maximum Supply Air Flow Rate [m3/s]");
                                // sizingCoolingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                                sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                CoolingAirVolFlowScalable = sizingCoolingAirFlow.size(state, TempSize, errorsFound);
                            } break;
                            case DataSizing::FlowPerCoolingCapacity: {
                                SizingMethod = HVAC::CoolingCapacitySizing;
                                TempSize = DataSizing::AutoSize;
                                PrintFlag = false;
                                state.dataSize->DataScalableSizingON = true;
                                state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolVolFlow;
                                CoolingCapacitySizer sizerCoolingCapacity;
                                sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                state.dataSize->DataAutosizedCoolingCapacity = sizerCoolingCapacity.size(state, TempSize, ErrorsFound);
                                state.dataSize->DataFlowPerCoolingCapacity = zoneHVACSizing.MaxCoolAirVolFlow;
                                PrintFlag = true;
                                TempSize = DataSizing::AutoSize;
                                bool errorsFound = false;
                                CoolingAirFlowSizer sizingCoolingAirFlow;
                                sizingCoolingAirFlow.overrideSizingString(state.dataGlobal->isEpJSON ? "maximum_supply_air_flow_rate [m3/s]"
                                                                                                     : "Maximum Supply Air Flow Rate [m3/s]");
                                // sizingCoolingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                                sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                CoolingAirVolFlowScalable = sizingCoolingAirFlow.size(state, TempSize, errorsFound);
                            } break;
                            default: {
                            } break;
                            }
                        } else if (zoneHVACSizing.HeatingSAFMethod > 0) {
                            SizingMethod = HVAC::HeatingAirflowSizing;
                            int SAFMethod = zoneHVACSizing.HeatingSAFMethod;
                            ZoneEqSizing.SizingMethod(SizingMethod) = SAFMethod;
                            switch (SAFMethod) {
                            case DataSizing::None:
                            case DataSizing::SupplyAirFlowRate:
                            case DataSizing::FlowPerFloorArea:
                            case DataSizing::FractionOfAutosizedHeatingAirflow: {
                                switch (SAFMethod) {
                                case DataSizing::SupplyAirFlowRate: {
                                    if (zoneHVACSizing.MaxHeatAirVolFlow > 0.0) {
                                        ZoneEqSizing.AirVolFlow = zoneHVACSizing.MaxHeatAirVolFlow;
                                        ZoneEqSizing.SystemAirFlow = true;
                                    }
                                    TempSize = zoneHVACSizing.MaxHeatAirVolFlow;
                                } break;
                                case DataSizing::FlowPerFloorArea: {
                                    ZoneEqSizing.SystemAirFlow = true;
                                    ZoneEqSizing.AirVolFlow =
                                        zoneHVACSizing.MaxHeatAirVolFlow * state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                                    TempSize = ZoneEqSizing.AirVolFlow;
                                    state.dataSize->DataScalableSizingON = true;
                                } break;
                                case DataSizing::FractionOfAutosizedHeatingAirflow: {
                                    state.dataSize->DataFracOfAutosizedHeatingAirflow = zoneHVACSizing.MaxHeatAirVolFlow;
                                    TempSize = DataSizing::AutoSize;
                                    state.dataSize->DataScalableSizingON = true;
                                } break;
                                default: {
                                    TempSize = zoneHVACSizing.MaxHeatAirVolFlow;
                                } break;
                                }
                                bool errorsFound = false;
                                HeatingAirFlowSizer sizingHeatingAirFlow;
                                sizingHeatingAirFlow.overrideSizingString(state.dataGlobal->isEpJSON ? "maximum_supply_air_flow_rate [m3/s]"
                                                                                                     : "Maximum Supply Air Flow Rate [m3/s]");
                                // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                                sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                HeatingAirVolFlowScalable = sizingHeatingAirFlow.size(state, TempSize, errorsFound);
                            } break;
                            case DataSizing::FlowPerHeatingCapacity: {
                                TempSize = DataSizing::AutoSize;
                                PrintFlag = false;
                                state.dataSize->DataScalableSizingON = true;
                                state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatVolFlow;
                                bool errorsFound = false;
                                HeatingCapacitySizer sizerHeatingCapacity;
                                sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                state.dataSize->DataAutosizedHeatingCapacity = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                                state.dataSize->DataFlowPerHeatingCapacity = zoneHVACSizing.MaxHeatAirVolFlow;
                                PrintFlag = true;
                                TempSize = DataSizing::AutoSize;
                                errorsFound = false;
                                HeatingAirFlowSizer sizingHeatingAirFlow;
                                sizingHeatingAirFlow.overrideSizingString(state.dataGlobal->isEpJSON ? "maximum_supply_air_flow_rate [m3/s]"
                                                                                                     : "Maximum Supply Air Flow Rate [m3/s]");
                                // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                                sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                HeatingAirVolFlowScalable = sizingHeatingAirFlow.size(state, TempSize, errorsFound);
                            } break;
                            default: {
                            } break;
                            }
                        }
                        // DataScalableSizingON = false;
                    } else { // if (unitVent.CoilOption != CoilsUsed::None)

                        PrintFlag = true;
                        if (unitVent.MaxAirVolFlow == DataSizing::AutoSize) {
                            TempSize = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).MinOA;
                        } else {
                            TempSize = unitVent.MaxAirVolFlow;
                        }
                        bool errorsFound = false;
                        SystemAirFlowSizer sizerSystemAirFlow;
                        // sizerSystemAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                        sizerSystemAirFlow.overrideSizingString(state.dataGlobal->isEpJSON ? "maximum_supply_air_flow_rate [m3/s]"
                                                                                           : "Maximum Supply Air Flow Rate [m3/s]");
                        sizerSystemAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        HeatingAirVolFlowScalable = sizerSystemAirFlow.size(state, TempSize, errorsFound);
                    }
                }

                unitVent.MaxAirVolFlow = max(CoolingAirVolFlowScalable, HeatingAirVolFlowScalable);

            } else {
                // no scalble sizing method has been specified. Sizing proceeds using the method specified in the zoneHVAC object
                // N1 , \field Maximum Supply Air Flow Rate
                PrintFlag = true;
                if (unitVent.CoilOption == CoilsUsed::None) {

                    if (unitVent.MaxAirVolFlow == DataSizing::AutoSize) {
                        TempSize = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).MinOA;
                    } else {
                        TempSize = unitVent.MaxAirVolFlow;
                    }

                } else {
                    TempSize = unitVent.MaxAirVolFlow;
                }
                bool errorsFound = false;
                SystemAirFlowSizer sizerSystemAirFlow;
                // sizerSystemAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                sizerSystemAirFlow.overrideSizingString(state.dataGlobal->isEpJSON ? "maximum_supply_air_flow_rate [m3/s]"
                                                                                   : "Maximum Supply Air Flow Rate [m3/s]");
                sizerSystemAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                unitVent.MaxAirVolFlow = sizerSystemAirFlow.size(state, TempSize, errorsFound);
            }
        }

        IsAutoSize = false;
        if (unitVent.OutAirVolFlow == DataSizing::AutoSize) {
            IsAutoSize = true;
        }
        if (state.dataSize->CurZoneEqNum > 0) {
            if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
                if (unitVent.OutAirVolFlow > 0.0) {
                    BaseSizer::reportSizerOutput(state,
                                                 state.dataUnitVentilators->cMO_UnitVentilator,
                                                 unitVent.Name,
                                                 "User-Specified Maximum Outdoor Air Flow Rate [m3/s]",
                                                 unitVent.OutAirVolFlow);
                }
            } else {
                CheckZoneSizing(state, state.dataUnitVentilators->cMO_UnitVentilator, unitVent.Name);
                if (unitVent.OAControlType == OAControl::FixedAmount) {
                    OutAirVolFlowDes = min(state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).MinOA, unitVent.MaxAirVolFlow);
                } else {
                    OutAirVolFlowDes = unitVent.MaxAirVolFlow;
                }

                if (IsAutoSize) {
                    unitVent.OutAirVolFlow = OutAirVolFlowDes;
                    BaseSizer::reportSizerOutput(state,
                                                 state.dataUnitVentilators->cMO_UnitVentilator,
                                                 unitVent.Name,
                                                 "Design Size Maximum Outdoor Air Flow Rate [m3/s]",
                                                 OutAirVolFlowDes);
                } else {
                    if (unitVent.OutAirVolFlow > 0.0 && OutAirVolFlowDes > 0.0) {
                        OutAirVolFlowUser = unitVent.OutAirVolFlow;
                        BaseSizer::reportSizerOutput(state,
                                                     state.dataUnitVentilators->cMO_UnitVentilator,
                                                     unitVent.Name,
                                                     "Design Size Maximum Outdoor Air Flow Rate [m3/s]",
                                                     OutAirVolFlowDes,
                                                     "User-Specified Maximum Outdoor Air Flow Rate [m3/s]",
                                                     OutAirVolFlowUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(OutAirVolFlowDes - OutAirVolFlowUser) / OutAirVolFlowUser) > state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state,
                                            format("SizeUnitVentilator: Potential issue with equipment sizing for {} {}",
                                                   state.dataUnitVentilators->cMO_UnitVentilator,
                                                   unitVent.Name));
                                ShowContinueError(state, format("User-Specified Maximum Outdoor Air Flow Rate of {:.5R} [m3/s]", OutAirVolFlowUser));
                                ShowContinueError(
                                    state, format("differs from Design Size Maximum Outdoor Air Flow Rate of {:.5R} [m3/s]", OutAirVolFlowDes));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }
            }
            ZoneEqSizing.OAVolFlow = unitVent.OutAirVolFlow;

            if (unitVent.ATMixerExists) {     // set up ATMixer conditions for use in component sizing
                ZoneEqSizing.OAVolFlow = 0.0; // Equipment OA flow should always be 0 when ATMixer is used
                SingleDuct::setATMixerSizingProperties(state, unitVent.ATMixerIndex, unitVent.ZonePtr, state.dataSize->CurZoneEqNum);
            }
        }

        IsAutoSize = false;
        if (unitVent.MinOutAirVolFlow == DataSizing::AutoSize) {
            IsAutoSize = true;
        }
        if (state.dataSize->CurZoneEqNum > 0) {
            if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
                if (unitVent.MinOutAirVolFlow > 0.0) {
                    BaseSizer::reportSizerOutput(state,
                                                 state.dataUnitVentilators->cMO_UnitVentilator,
                                                 unitVent.Name,
                                                 "User-Specified Minimum Outdoor Air Flow Rate [m3/s]",
                                                 unitVent.MinOutAirVolFlow);
                }
            } else {
                CheckZoneSizing(state, state.dataUnitVentilators->cMO_UnitVentilator, unitVent.Name);
                MinOutAirVolFlowDes = min(state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).MinOA, unitVent.MaxAirVolFlow);
                if (MinOutAirVolFlowDes < HVAC::SmallAirVolFlow) {
                    MinOutAirVolFlowDes = 0.0;
                }
                if (IsAutoSize) {
                    unitVent.MinOutAirVolFlow = MinOutAirVolFlowDes;
                    BaseSizer::reportSizerOutput(state,
                                                 state.dataUnitVentilators->cMO_UnitVentilator,
                                                 unitVent.Name,
                                                 "Design Size Minimum Outdoor Air Flow Rate [m3/s]",
                                                 MinOutAirVolFlowDes);
                } else {
                    if (unitVent.MinOutAirVolFlow > 0.0 && MinOutAirVolFlowDes > 0.0) {
                        MinOutAirVolFlowUser = unitVent.MinOutAirVolFlow;
                        BaseSizer::reportSizerOutput(state,
                                                     state.dataUnitVentilators->cMO_UnitVentilator,
                                                     unitVent.Name,
                                                     "Design Size Minimum Outdoor Air Flow Rate [m3/s]",
                                                     MinOutAirVolFlowDes,
                                                     "User-Specified Minimum Outdoor Air Flow Rate [m3/s]",
                                                     MinOutAirVolFlowUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(MinOutAirVolFlowDes - MinOutAirVolFlowUser) / MinOutAirVolFlowUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(state,
                                            format("SizeUnitVentilator: Potential issue with equipment sizing for {} = \"{}\".",
                                                   state.dataUnitVentilators->cMO_UnitVentilator,
                                                   unitVent.Name));
                                ShowContinueError(state,
                                                  format("User-Specified Minimum Outdoor Air Flow Rate of {:.5R} [m3/s]", MinOutAirVolFlowUser));
                                ShowContinueError(
                                    state, format("differs from Design Size Minimum Outdoor Air Flow Rate of {:.5R} [m3/s]", MinOutAirVolFlowDes));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }
            }
        }

        IsAutoSize = false;
        if (unitVent.MaxVolHotWaterFlow == DataSizing::AutoSize) {
            IsAutoSize = true;
        }
        if (unitVent.HCoilType == HeatCoilType::Water) {
            if (state.dataSize->CurZoneEqNum > 0) {
                if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
                    if (unitVent.MaxVolHotWaterFlow > 0.0) {
                        BaseSizer::reportSizerOutput(state,
                                                     state.dataUnitVentilators->cMO_UnitVentilator,
                                                     unitVent.Name,
                                                     "User-Specified Maximum Hot Water Flow [m3/s]",
                                                     unitVent.MaxVolHotWaterFlow);
                    }
                } else {
                    CheckZoneSizing(state, state.dataUnitVentilators->cMO_UnitVentilator, unitVent.Name);

                    CoilWaterOutletNode = WaterCoils::GetCoilWaterOutletNode(state, "Coil:Heating:Water", unitVent.HCoilName, ErrorsFound);
                    if (IsAutoSize) {
                        PltSizHeatNum = PlantUtilities::MyPlantSizingIndex(
                            state, "COIL:HEATING:WATER", unitVent.HCoilName, unitVent.HotControlNode, CoilWaterOutletNode, ErrorsFound);

                        if (state.dataWaterCoils->WaterCoil(unitVent.HCoil_Index).UseDesignWaterDeltaTemp) {
                            WaterCoilSizDeltaT = state.dataWaterCoils->WaterCoil(unitVent.HCoil_Index).DesignWaterDeltaTemp;
                            DoWaterCoilSizing = true;
                        } else {
                            if (PltSizHeatNum > 0) {
                                WaterCoilSizDeltaT = state.dataSize->PlantSizData(PltSizHeatNum).DeltaT;
                                DoWaterCoilSizing = true;
                            } else {
                                DoWaterCoilSizing = false;
                                // If there is no heating Plant Sizing object and autosizing was requested, issue fatal error message
                                ShowSevereError(state, "Autosizing of water flow requires a heating loop Sizing:Plant object");
                                ShowContinueError(state,
                                                  format("Occurs in {} = \"{}\"", state.dataUnitVentilators->cMO_UnitVentilator, unitVent.Name));
                                ErrorsFound = true;
                            }
                        }

                        if (DoWaterCoilSizing) {
                            if (state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatMassFlow >= HVAC::SmallAirVolFlow) {
                                SizingMethod = HVAC::HeatingCapacitySizing;
                                if (unitVent.HVACSizingIndex > 0) {
                                    auto const &zoneHVACSizing = state.dataSize->ZoneHVACSizing(unitVent.HVACSizingIndex);
                                    CapSizingMethod = zoneHVACSizing.HeatingCapMethod;
                                    ZoneEqSizing.SizingMethod(SizingMethod) = CapSizingMethod;
                                    switch (CapSizingMethod) {
                                    case DataSizing::HeatingDesignCapacity:
                                    case DataSizing::CapacityPerFloorArea:
                                    case DataSizing::FractionOfAutosizedHeatingCapacity: {
                                        switch (CapSizingMethod) {
                                        case DataSizing::HeatingDesignCapacity: {
                                            if (zoneHVACSizing.ScaledHeatingCapacity > 0.0) {
                                                ZoneEqSizing.HeatingCapacity = true;
                                                ZoneEqSizing.DesHeatingLoad = zoneHVACSizing.ScaledHeatingCapacity;
                                            } else {
                                                state.dataSize->DataFlowUsedForSizing =
                                                    state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatVolFlow;
                                            }
                                            TempSize = zoneHVACSizing.ScaledHeatingCapacity;
                                        } break;
                                        case DataSizing::CapacityPerFloorArea: {
                                            ZoneEqSizing.HeatingCapacity = true;
                                            ZoneEqSizing.DesHeatingLoad = zoneHVACSizing.ScaledHeatingCapacity *
                                                                          state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                                            state.dataSize->DataScalableCapSizingON = true;
                                        } break;
                                        case DataSizing::FractionOfAutosizedHeatingCapacity: {
                                            state.dataSize->DataFracOfAutosizedHeatingCapacity = zoneHVACSizing.ScaledHeatingCapacity;
                                            state.dataSize->DataFlowUsedForSizing =
                                                state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatVolFlow;
                                            TempSize = DataSizing::AutoSize;
                                            state.dataSize->DataScalableCapSizingON = true;
                                        } break;
                                        default: {
                                        } break;
                                        }
                                    } break;
                                    default: {
                                    } break;
                                    }
                                    PrintFlag = false;
                                    bool errorsFound = false;
                                    HeatingCapacitySizer sizerHeatingCapacity;
                                    sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                    DesHeatingLoad = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                                    state.dataSize->DataScalableCapSizingON = false;
                                } else {
                                    PrintFlag = false;
                                    TempSize = DataSizing::AutoSize;
                                    state.dataSize->DataFlowUsedForSizing =
                                        state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatVolFlow;
                                    bool errorsFound = false;
                                    HeatingCapacitySizer sizerHeatingCapacity;
                                    sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                    DesHeatingLoad = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                                }
                                rho = state.dataPlnt->PlantLoop(unitVent.HWplantLoc.loopNum)
                                          .glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                                Cp = state.dataPlnt->PlantLoop(unitVent.HWplantLoc.loopNum)
                                         .glycol->getSpecificHeat(state, Constant::HWInitConvTemp, RoutineName);
                                MaxVolHotWaterFlowDes = DesHeatingLoad / (WaterCoilSizDeltaT * Cp * rho);

                            } else {
                                MaxVolHotWaterFlowDes = 0.0;
                            }
                        }
                        unitVent.MaxVolHotWaterFlow = MaxVolHotWaterFlowDes;
                        BaseSizer::reportSizerOutput(state,
                                                     state.dataUnitVentilators->cMO_UnitVentilator,
                                                     unitVent.Name,
                                                     "Design Size Maximum Hot Water Flow [m3/s]",
                                                     MaxVolHotWaterFlowDes);
                    } else {
                        if (unitVent.MaxVolHotWaterFlow > 0.0 && MaxVolHotWaterFlowDes > 0.0) {
                            MaxVolHotWaterFlowUser = unitVent.MaxVolHotWaterFlow;
                            BaseSizer::reportSizerOutput(state,
                                                         state.dataUnitVentilators->cMO_UnitVentilator,
                                                         unitVent.Name,
                                                         "Design Size Maximum Hot Water Flow [m3/s]",
                                                         MaxVolHotWaterFlowDes,
                                                         "User-Specified Maximum Hot Water Flow [m3/s]",
                                                         MaxVolHotWaterFlowUser);
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(MaxVolHotWaterFlowDes - MaxVolHotWaterFlowUser) / MaxVolHotWaterFlowUser) >
                                    state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(state,
                                                format("SizeUnitVentilator: Potential issue with equipment sizing for {} {}",
                                                       state.dataUnitVentilators->cMO_UnitVentilator,
                                                       unitVent.Name));
                                    ShowContinueError(state,
                                                      format("User-Specified Maximum Hot Water Flow of {:.5R} [m3/s]", MaxVolHotWaterFlowUser));
                                    ShowContinueError(
                                        state, format("differs from Design Size Maximum Hot Water Flow of {:.5R} [m3/s]", MaxVolHotWaterFlowDes));
                                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                                }
                            }
                        }
                    }
                }
            }
        } else {
            unitVent.MaxVolHotWaterFlow = 0.0;
        }

        IsAutoSize = false;
        if (unitVent.MaxVolHotSteamFlow == DataSizing::AutoSize) {
            IsAutoSize = true;
        }
        if (unitVent.HCoilType == HeatCoilType::Steam) {
            if (state.dataSize->CurZoneEqNum > 0) {
                if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
                    if (unitVent.MaxVolHotSteamFlow > 0.0) {
                        BaseSizer::reportSizerOutput(state,
                                                     state.dataUnitVentilators->cMO_UnitVentilator,
                                                     unitVent.Name,
                                                     "User-Specified Maximum Steam Flow [m3/s]",
                                                     unitVent.MaxVolHotSteamFlow);
                    }
                } else {
                    CheckZoneSizing(state, state.dataUnitVentilators->cMO_UnitVentilator, unitVent.Name);

                    int CoilSteamOutletNode = SteamCoils::GetCoilSteamOutletNode(state, "Coil:Heating:Steam", unitVent.HCoilName, ErrorsFound);
                    if (IsAutoSize) {
                        PltSizHeatNum = PlantUtilities::MyPlantSizingIndex(
                            state, "Coil:Heating:Steam", unitVent.HCoilName, unitVent.HotControlNode, CoilSteamOutletNode, ErrorsFound);
                        if (PltSizHeatNum > 0) {
                            if (state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatMassFlow >= HVAC::SmallAirVolFlow) {
                                SizingMethod = HVAC::HeatingCapacitySizing;
                                if (unitVent.HVACSizingIndex > 0) {
                                    auto const &zoneHVACSizing = state.dataSize->ZoneHVACSizing(unitVent.HVACSizingIndex);
                                    CapSizingMethod = zoneHVACSizing.HeatingCapMethod;
                                    ZoneEqSizing.SizingMethod(SizingMethod) = CapSizingMethod;
                                    switch (CapSizingMethod) {
                                    case DataSizing::HeatingDesignCapacity:
                                    case DataSizing::CapacityPerFloorArea:
                                    case DataSizing::FractionOfAutosizedHeatingCapacity: {
                                        if (CapSizingMethod == DataSizing::HeatingDesignCapacity) {
                                            if (zoneHVACSizing.ScaledHeatingCapacity > 0.0) {
                                                ZoneEqSizing.HeatingCapacity = true;
                                                ZoneEqSizing.DesHeatingLoad = zoneHVACSizing.ScaledHeatingCapacity;
                                            } else {
                                                state.dataSize->DataFlowUsedForSizing =
                                                    state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatVolFlow;
                                            }
                                            TempSize = zoneHVACSizing.ScaledHeatingCapacity;
                                        } else if (CapSizingMethod == DataSizing::CapacityPerFloorArea) {
                                            ZoneEqSizing.HeatingCapacity = true;
                                            ZoneEqSizing.DesHeatingLoad = zoneHVACSizing.ScaledHeatingCapacity *
                                                                          state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                                            state.dataSize->DataScalableCapSizingON = true;
                                        } else if (CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                                            state.dataSize->DataFracOfAutosizedHeatingCapacity = zoneHVACSizing.ScaledHeatingCapacity;
                                            state.dataSize->DataFlowUsedForSizing =
                                                state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatVolFlow;
                                            TempSize = DataSizing::AutoSize;
                                            state.dataSize->DataScalableCapSizingON = true;
                                        }
                                    } break;
                                    default: {
                                    } break;
                                    }

                                    PrintFlag = false;
                                    bool errorsFound = false;
                                    HeatingCapacitySizer sizerHeatingCapacity;
                                    sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                    DesHeatingLoad = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                                    state.dataSize->DataScalableCapSizingON = false;
                                } else {
                                    PrintFlag = false;
                                    TempSize = DataSizing::AutoSize;
                                    state.dataSize->DataFlowUsedForSizing =
                                        state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatVolFlow;
                                    bool errorsFound = false;
                                    HeatingCapacitySizer sizerHeatingCapacity;
                                    sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                    DesHeatingLoad = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                                }
                                TempSteamIn = 100.00;
                                auto *steam = Fluid::GetSteam(state);
                                EnthSteamInDry = steam->getSatEnthalpy(state, TempSteamIn, 1.0, RoutineName);
                                EnthSteamOutWet = steam->getSatEnthalpy(state, TempSteamIn, 0.0, RoutineName);
                                LatentHeatSteam = EnthSteamInDry - EnthSteamOutWet;
                                SteamDensity = steam->getSatDensity(state, TempSteamIn, 1.0, RoutineName);
                                Cp =
                                    Fluid::GetWater(state)->getSpecificHeat(state, state.dataSize->PlantSizData(PltSizHeatNum).ExitTemp, RoutineName);
                                MaxVolHotSteamFlowDes =
                                    DesHeatingLoad / (SteamDensity * (LatentHeatSteam + state.dataSize->PlantSizData(PltSizHeatNum).DeltaT * Cp));
                            } else {
                                MaxVolHotSteamFlowDes = 0.0;
                            }
                        } else {
                            ShowSevereError(state, "Autosizing of Steam flow requires a heating loop Sizing:Plant object");
                            ShowContinueError(state, format("Occurs in {} = \"{}\"", state.dataUnitVentilators->cMO_UnitVentilator, unitVent.Name));
                            ErrorsFound = true;
                        }
                        unitVent.MaxVolHotSteamFlow = MaxVolHotSteamFlowDes;
                        BaseSizer::reportSizerOutput(state,
                                                     state.dataUnitVentilators->cMO_UnitVentilator,
                                                     unitVent.Name,
                                                     "Design Size Maximum Steam Flow [m3/s]",
                                                     MaxVolHotSteamFlowDes);
                    } else {
                        if (unitVent.MaxVolHotSteamFlow > 0.0 && MaxVolHotSteamFlowDes > 0.0) {
                            MaxVolHotSteamFlowUser = unitVent.MaxVolHotSteamFlow;
                            BaseSizer::reportSizerOutput(state,
                                                         state.dataUnitVentilators->cMO_UnitVentilator,
                                                         unitVent.Name,
                                                         "Design Size Maximum Steam Flow [m3/s]",
                                                         MaxVolHotSteamFlowDes,
                                                         "User-Specified Maximum Steam Flow [m3/s]",
                                                         MaxVolHotSteamFlowUser);
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(MaxVolHotSteamFlowDes - MaxVolHotSteamFlowUser) / MaxVolHotSteamFlowUser) >
                                    state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(state,
                                                format("SizeUnitVentilator: Potential issue with equipment sizing for {} = \"{}\"",
                                                       state.dataUnitVentilators->cMO_UnitVentilator,
                                                       unitVent.Name));
                                    ShowContinueError(state, format("User-Specified Maximum Steam Flow of {:.5R} [m3/s]", MaxVolHotSteamFlowUser));
                                    ShowContinueError(state,
                                                      format("differs from Design Size Maximum Steam Flow of {:.5R} [m3/s]", MaxVolHotSteamFlowDes));
                                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                                }
                            }
                        }
                    }
                }
            }
        } else {
            unitVent.MaxVolHotSteamFlow = 0.0;
        }

        IsAutoSize = false;
        if (unitVent.MaxVolColdWaterFlow == DataSizing::AutoSize) {
            IsAutoSize = true;
        }
        if (unitVent.CCoilType == CoolCoilType::Water || unitVent.CCoilType == CoolCoilType::Detailed ||
            unitVent.CCoilType == CoolCoilType::HXAssisted) {

            if (state.dataSize->CurZoneEqNum > 0) {
                if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
                    if (unitVent.MaxVolColdWaterFlow > 0.0) {
                        BaseSizer::reportSizerOutput(state,
                                                     state.dataUnitVentilators->cMO_UnitVentilator,
                                                     unitVent.Name,
                                                     "User-Specified Maximum Cold Water Flow [m3/s]",
                                                     unitVent.MaxVolColdWaterFlow);
                    }
                } else {
                    CheckZoneSizing(state, state.dataUnitVentilators->cMO_UnitVentilator, unitVent.Name);

                    if (unitVent.CCoilType == CoolCoilType::HXAssisted) {
                        CoolingCoilName = HVACHXAssistedCoolingCoil::GetHXDXCoilName(state, unitVent.CCoilTypeCh, unitVent.CCoilName, ErrorsFound);
                        CoolingCoilType = HVACHXAssistedCoolingCoil::GetHXCoilType(state, unitVent.CCoilTypeCh, unitVent.CCoilName, ErrorsFound);
                    } else {
                        CoolingCoilName = unitVent.CCoilName;
                        CoolingCoilType = unitVent.CCoilTypeCh;
                    }
                    CoilWaterOutletNode = WaterCoils::GetCoilWaterOutletNode(state, CoolingCoilType, CoolingCoilName, ErrorsFound);
                    if (IsAutoSize) {
                        int PltSizCoolNum = PlantUtilities::MyPlantSizingIndex(
                            state, CoolingCoilType, CoolingCoilName, unitVent.ColdControlNode, CoilWaterOutletNode, ErrorsFound);
                        if (state.dataWaterCoils->WaterCoil(unitVent.CCoil_Index).UseDesignWaterDeltaTemp) {
                            WaterCoilSizDeltaT = state.dataWaterCoils->WaterCoil(unitVent.CCoil_Index).DesignWaterDeltaTemp;
                            DoWaterCoilSizing = true;
                        } else {
                            if (PltSizCoolNum > 0) {
                                WaterCoilSizDeltaT = state.dataSize->PlantSizData(PltSizCoolNum).DeltaT;
                                DoWaterCoilSizing = true;
                            } else {
                                DoWaterCoilSizing = false;
                                // If there is no cooling Plant Sizing object and autosizing was requested, issue fatal error message
                                ShowSevereError(state, "Autosizing of water coil requires a cooling loop Sizing:Plant object");
                                ShowContinueError(state,
                                                  format("Occurs in {} = \"{}\"", state.dataUnitVentilators->cMO_UnitVentilator, unitVent.Name));
                                ErrorsFound = true;
                            }
                        }
                        if (DoWaterCoilSizing) {
                            if (state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolMassFlow >= HVAC::SmallAirVolFlow) {
                                SizingMethod = HVAC::CoolingCapacitySizing;
                                if (unitVent.HVACSizingIndex > 0) {
                                    auto const &zoneHVACSizing = state.dataSize->ZoneHVACSizing(unitVent.HVACSizingIndex);
                                    CapSizingMethod = zoneHVACSizing.CoolingCapMethod;
                                    ZoneEqSizing.SizingMethod(SizingMethod) = CapSizingMethod;
                                    switch (CapSizingMethod) {
                                    case DataSizing::CoolingDesignCapacity:
                                    case DataSizing::CapacityPerFloorArea:
                                    case DataSizing::FractionOfAutosizedCoolingCapacity: {
                                        if (CapSizingMethod == DataSizing::CoolingDesignCapacity) {
                                            if (zoneHVACSizing.ScaledCoolingCapacity > 0.0) {
                                                ZoneEqSizing.CoolingCapacity = true;
                                                ZoneEqSizing.DesCoolingLoad = zoneHVACSizing.ScaledCoolingCapacity;
                                            } else {
                                                state.dataSize->DataFlowUsedForSizing =
                                                    state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolVolFlow;
                                            }
                                            TempSize = zoneHVACSizing.ScaledCoolingCapacity;
                                        } else if (CapSizingMethod == DataSizing::CapacityPerFloorArea) {
                                            ZoneEqSizing.CoolingCapacity = true;
                                            ZoneEqSizing.DesCoolingLoad = zoneHVACSizing.ScaledCoolingCapacity *
                                                                          state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                                            state.dataSize->DataScalableCapSizingON = true;
                                        } else if (CapSizingMethod == DataSizing::FractionOfAutosizedCoolingCapacity) {
                                            state.dataSize->DataFracOfAutosizedHeatingCapacity = zoneHVACSizing.ScaledCoolingCapacity;
                                            state.dataSize->DataFlowUsedForSizing =
                                                state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolVolFlow;
                                            TempSize = DataSizing::AutoSize;
                                            state.dataSize->DataScalableCapSizingON = true;
                                        }
                                    } break;
                                    default: {
                                    } break;
                                    }
                                    PrintFlag = false;
                                    CoolingCapacitySizer sizerCoolingCapacity;
                                    sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                    DesCoolingLoad = sizerCoolingCapacity.size(state, TempSize, ErrorsFound);
                                    state.dataSize->DataScalableCapSizingON = false;
                                } else {
                                    PrintFlag = false;
                                    TempSize = DataSizing::AutoSize;
                                    state.dataSize->DataFlowUsedForSizing =
                                        state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolVolFlow;
                                    CoolingCapacitySizer sizerCoolingCapacity;
                                    sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                    DesCoolingLoad = sizerCoolingCapacity.size(state, TempSize, ErrorsFound);
                                }
                                rho = state.dataPlnt->PlantLoop(unitVent.CWPlantLoc.loopNum).glycol->getDensity(state, 5., RoutineName);
                                Cp = state.dataPlnt->PlantLoop(unitVent.CWPlantLoc.loopNum).glycol->getSpecificHeat(state, 5., RoutineName);
                                MaxVolColdWaterFlowDes = DesCoolingLoad / (WaterCoilSizDeltaT * Cp * rho);

                                if (MaxVolColdWaterFlowDes < 0.0) {
                                    ShowWarningError(state, "Autosizing of water flow resulted in negative value.");
                                    ShowContinueError(state,
                                                      format("Occurs in {} = \"{}\"", state.dataUnitVentilators->cMO_UnitVentilator, unitVent.Name));
                                    ShowContinueError(state, "...Sizing information found during sizing simulation:");
                                    ShowContinueError(state, format("...Calculated coil design load = {:.3T} W", DesCoolingLoad));
                                    ShowContinueError(state, format("...Calculated water flow rate  = {:.3T} m3/s", MaxVolColdWaterFlowDes));
                                    ShowContinueError(state,
                                                      "...Water flow rate will be set to 0. Check sizing inputs for zone and plant, inputs for water "
                                                      "cooling coil object, and design day specifications.");
                                    ShowContinueError(state, "...Consider autosizing all inputs if not already doing so.");
                                    MaxVolColdWaterFlowDes = 0.0;
                                }
                            } else {
                                MaxVolColdWaterFlowDes = 0.0;
                            }
                        }
                        unitVent.MaxVolColdWaterFlow = MaxVolColdWaterFlowDes;
                        BaseSizer::reportSizerOutput(state,
                                                     state.dataUnitVentilators->cMO_UnitVentilator,
                                                     unitVent.Name,
                                                     "Design Size Maximum Cold Water Flow [m3/s]",
                                                     MaxVolColdWaterFlowDes);
                    } else {
                        if (unitVent.MaxVolColdWaterFlow > 0.0 && MaxVolColdWaterFlowDes > 0.0) {
                            MaxVolColdWaterFlowUser = unitVent.MaxVolColdWaterFlow;
                            BaseSizer::reportSizerOutput(state,
                                                         state.dataUnitVentilators->cMO_UnitVentilator,
                                                         unitVent.Name,
                                                         "Design Size Maximum Cold Water Flow [m3/s]",
                                                         MaxVolColdWaterFlowDes,
                                                         "User-Specified Maximum Cold Water Flow [m3/s]",
                                                         MaxVolColdWaterFlowUser);
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(MaxVolColdWaterFlowDes - MaxVolColdWaterFlowUser) / MaxVolColdWaterFlowUser) >
                                    state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(state,
                                                format("SizeUnitVentilator: Potential issue with equipment sizing for {} = \"{}\"",
                                                       state.dataUnitVentilators->cMO_UnitVentilator,
                                                       unitVent.Name));
                                    ShowContinueError(state,
                                                      format("User-Specified Maximum Cold Water Flow of {:.5R} [m3/s]", MaxVolColdWaterFlowUser));
                                    ShowContinueError(
                                        state, format("differs from Design Size Maximum Cold Water Flow of {:.5R} [m3/s]", MaxVolColdWaterFlowDes));
                                    ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                    ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                                }
                            }
                        }
                    }
                }
            }
        }

        // set the design air flow rates for the heating and cooling coils
        if (unitVent.CCoilType == CoolCoilType::HXAssisted) {
            CoolingCoilName = HVACHXAssistedCoolingCoil::GetHXDXCoilName(state, unitVent.CCoilTypeCh, unitVent.CCoilName, ErrorsFound);
            CoolingCoilType = HVACHXAssistedCoolingCoil::GetHXCoilType(state, unitVent.CCoilTypeCh, unitVent.CCoilName, ErrorsFound);
        } else {
            CoolingCoilName = unitVent.CCoilName;
            CoolingCoilType = unitVent.CCoilTypeCh;
        }
        WaterCoils::SetCoilDesFlow(state, CoolingCoilType, CoolingCoilName, unitVent.MaxAirVolFlow, ErrorsFound);
        WaterCoils::SetCoilDesFlow(state, unitVent.HCoilTypeCh, unitVent.HCoilName, unitVent.MaxAirVolFlow, ErrorsFound);

        if (state.dataSize->CurZoneEqNum > 0) {
            ZoneEqSizing.MaxHWVolFlow = unitVent.MaxVolHotWaterFlow;
            ZoneEqSizing.MaxCWVolFlow = unitVent.MaxVolColdWaterFlow;
        }

        if (ErrorsFound) {
            ShowFatalError(state, "Preceding sizing errors cause program termination");
        }
    }

    void CalcUnitVentilator(EnergyPlusData &state,
                            int &UnitVentNum,              // number of the current fan coil unit being simulated
                            int const ZoneNum,             // number of zone being served
                            bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                            Real64 &PowerMet,              // Sensible power supplied (W)
                            Real64 &LatOutputProvided      // Latent power supplied (kg/s), negative = dehumidification
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   May 2000
        //       MODIFIED       Don Shirey, Aug 2009 (LatOutputProvided)
        //                      July 2012, Chandan Sharma - FSEC: Added zone sys avail managers

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine mainly controls the action of the unit ventilator
        // (or more exactly, it controls the amount of outside air brought in)
        // based on the user input for controls and the defined controls
        // algorithms.  There are currently (at the initial creation of this
        // subroutine) two control methods: variable percent (ASHRAE "Cycle I"
        // or "Cycle II") and fixed temperature (ASHRAE "Cycle III").

        // METHODOLOGY EMPLOYED:
        // Unit is controlled based on user input and what is happening in the
        // simulation.  There are various cases to consider:
        // 1. OFF: Unit is schedule off or there is no load on it.  All flow
        //    rates are set to zero and the temperatures are set to zone conditions
        //    (except for the outside air inlet).
        // 2. HEATING/VARIABLE PERCENT: The unit is on, there is a heating load,
        //    and variable percent control is specified.  The outside air fraction
        //    is set to the minimum outside air fraction (schedule based) and the
        //    heating coil is activated.
        // 3. HEATING/FIXED TEMPERATURE: The unit is on, there is a heating load,
        //    and fixed temperature control is specified.  The outside air fraction
        //    is varied in an attempt to obtain a mixed air temperature equal to
        //    the user specified temperature (schedule based).  The heating coil
        //    is activated, if necessary.
        // 4. COOLING/NO COIL: The unit is on, there is a cooling load, and no
        //    coil is present or it has been scheduled off.  Set the amount of
        //    outside air based on the control type.  Simulate the "mixing box".
        // 5. COOLING/WITH COIL: The unit is on, there is a cooling load, and
        //    a cooling coil is present and scheduled on.  Tries to use outside
        //    air as best as possible and then calls a cooling coil
        // Note: controls are strictly temperature based and do not factor
        // humidity into the equation (not an enthalpy economy cycle but rather
        // a simple return air economy cycle).  In addition, temperature predictions
        // are not strict energy balances here in the control routine though
        // in the mixing routine an energy balance is preserved.

        // REFERENCES:
        // ASHRAE Systems and Equipment Handbook (SI), 1996. page 31.3

        // Using/Aliasing
        auto &unitVent = state.dataUnitVentilators->UnitVent(UnitVentNum);

        // Smallest allowed temperature difference for comparisons (below this value the temperatures are assumed equal)
        Real64 constexpr LowTempDiff(0.1);
        // Smallest allowed outside air fraction difference for comparison (below this value the fractions are assumed equal)
        Real64 constexpr LowOAFracDiff(0.01);
        int constexpr MaxIter(50);

        Real64 AirMassFlow; // air mass flow rate [kg/sec]
        Real64 MaxOAFrac;   // maximum possible outside air fraction
        Real64 MinOAFrac;   // minimum possible outside air fraction
        Real64 Tdesired;    // desired temperature after mixing inlet and outdoor air [degrees C]
        Real64 Tinlet;      // temperature of air coming into the unit ventilator [degrees C]
        Real64 Toutdoor;    // temperature of outdoor air being introduced into the unit ventilator [degrees C]
        Real64 mdot;

        switch (unitVent.CoilOption) {
        case CoilsUsed::Both:
        case CoilsUsed::Heating: {
            switch (unitVent.HCoilType) {
            case HeatCoilType::Water: {
                WaterCoils::CheckWaterCoilSchedule(state, unitVent.HCoilName, unitVent.HCoilSchedValue, unitVent.HCoil_Index);
            } break;
            case HeatCoilType::Steam: {
                SteamCoils::CheckSteamCoilSchedule(state, "Coil:Heating:Steam", unitVent.HCoilName, unitVent.HCoilSchedValue, unitVent.HCoil_Index);
            } break;
            case HeatCoilType::Electric: {
                HeatingCoils::CheckHeatingCoilSchedule(
                    state, "Coil:Heating:Electric", unitVent.HCoilName, unitVent.HCoilSchedValue, unitVent.HCoil_Index);
            } break;
            case HeatCoilType::Gas: {
                HeatingCoils::CheckHeatingCoilSchedule(
                    state, "Coil:Heating:Fuel", unitVent.HCoilName, unitVent.HCoilSchedValue, unitVent.HCoil_Index);
            } break;
            default: {
            } break;
            }
        } break;
        default: {
        } break;
        }

        switch (unitVent.CoilOption) {
        case CoilsUsed::Both:
        case CoilsUsed::Cooling: {
            switch (unitVent.CCoilType) {
            case CoolCoilType::Water: {
                WaterCoils::CheckWaterCoilSchedule(state, unitVent.CCoilName, unitVent.CCoilSchedValue, unitVent.CCoil_Index);
            } break;
            case CoolCoilType::Detailed: {
                WaterCoils::CheckWaterCoilSchedule(state, unitVent.CCoilName, unitVent.CCoilSchedValue, unitVent.CCoil_Index);
            } break;
            case CoolCoilType::HXAssisted: {
                HVACHXAssistedCoolingCoil::CheckHXAssistedCoolingCoilSchedule(
                    state, "CoilSystem:Cooling:Water:HeatExchangerAssisted", unitVent.CCoilName, unitVent.CCoilSchedValue, unitVent.CCoil_Index);
            } break;
            default: {
                assert(false);
            } break;
            }
        } break;
        default: {
        } break;
        }

        // initialize local variables
        Real64 QUnitOut = 0.0;
        Real64 ControlOffset = 0.0;
        Real64 MaxWaterFlow = 0.0;
        Real64 MinWaterFlow = 0.0;
        Real64 NoOutput = 0.0;
        Real64 FullOutput = 0.0;
        HVAC::FanOp fanOp = unitVent.fanOp;
        Real64 PartLoadFrac = 0.0;

        auto const &inletNode = state.dataLoopNodes->Node(unitVent.AirInNode);
        auto const &outletNode = state.dataLoopNodes->Node(unitVent.AirOutNode);
        auto const &outsideAirNode = state.dataLoopNodes->Node(unitVent.OutsideAirNode);

        if ((std::abs(state.dataUnitVentilators->QZnReq) < HVAC::SmallLoad) || (state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) ||
            (unitVent.availSched->getCurrentVal() <= 0) ||
            ((unitVent.fanAvailSched->getCurrentVal() <= 0 && !state.dataHVACGlobal->TurnFansOn) || state.dataHVACGlobal->TurnFansOff)) {

            // Unit is off or has no load upon it; set the flow rates to zero and then
            // simulate the components with the no flow conditions
            AirMassFlow = outletNode.MassFlowRate;
            state.dataUnitVentilators->HCoilOn = false;
            if (unitVent.HotControlNode > 0) {
                mdot = 0.0;
                PlantUtilities::SetComponentFlowRate(state, mdot, unitVent.HotControlNode, unitVent.HotCoilOutNodeNum, unitVent.HWplantLoc);
            }
            if (unitVent.ColdControlNode > 0) {
                mdot = 0.0;
                PlantUtilities::SetComponentFlowRate(state, mdot, unitVent.ColdControlNode, unitVent.ColdCoilOutNodeNum, unitVent.CWPlantLoc);
            }

            if (fanOp == HVAC::FanOp::Cycling) {
                CalcUnitVentilatorComponents(state, UnitVentNum, FirstHVACIteration, QUnitOut, fanOp, PartLoadFrac);
                if (inletNode.MassFlowRateMax > 0.0) {
                    unitVent.FanPartLoadRatio = inletNode.MassFlowRate / inletNode.MassFlowRateMax;
                }
            } else {
                CalcUnitVentilatorComponents(state, UnitVentNum, FirstHVACIteration, QUnitOut);
            }
        } else { // Unit is on-->this section is intended to control the outside air and the main
            // result is to set the outside air flow rate variable OAMassFlowRate
            int SolFlag = 0; // # of iterations IF positive, -1 means failed to converge, -2 means bounds are incorrect
            unitVent.FanPartLoadRatio = 1.0;
            if (state.dataUnitVentilators->QZnReq > HVAC::SmallLoad) { // HEATING MODE

                int ControlNode = unitVent.HotControlNode;
                ControlOffset = unitVent.HotControlOffset;
                MaxWaterFlow = unitVent.MaxHotWaterFlow;
                MinWaterFlow = unitVent.MinHotWaterFlow;
                // On the first HVAC iteration the system values are given to the controller, but after that
                // the demand limits are in place and there needs to be feedback to the Zone Equipment
                if (!FirstHVACIteration && unitVent.HCoilType == HeatCoilType::Water) {
                    MaxWaterFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRateMaxAvail;
                    MinWaterFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRateMinAvail;
                }

                state.dataUnitVentilators->HCoilOn = true;

                if (outsideAirNode.MassFlowRate > 0.0) {
                    MinOAFrac = unitVent.minOASched->getCurrentVal() * (unitVent.MinOutAirMassFlow / outsideAirNode.MassFlowRate);
                } else {
                    MinOAFrac = 0.0;
                }
                MinOAFrac = min(1.0, max(0.0, MinOAFrac));

                if ((!unitVent.HCoilPresent) || (unitVent.HCoilSchedValue <= 0.0)) {
                    // In heating mode, but there is no coil to provide heating.  This is handled
                    // differently than if there was a heating coil present.  Fixed temperature
                    // will still try to vary the amount of outside air to meet the desired
                    // mixed air temperature, while variable percent will go to full ventilation
                    // when it is most advantageous.

                    {
                        switch (unitVent.OAControlType) {
                        case OAControl::FixedAmount: {
                            // In this control type, the outdoor air flow rate is fixed to the minimum value
                            // which is equal to the maximum value, regardless of all the other conditions.

                            state.dataUnitVentilators->OAMassFlowRate = MinOAFrac * outsideAirNode.MassFlowRate;
                        } break;
                        case OAControl::VariablePercent: {
                            // This algorithm is probably a bit simplistic in that it just bounces
                            // back and forth between the maximum outside air and the minimum.  In
                            // REAL(r64)ity, a system *might* vary between the two based on the load in
                            // the zone.
                            Tinlet = inletNode.Temp;
                            Toutdoor = outsideAirNode.Temp;

                            if (Tinlet >= Toutdoor) {

                                state.dataUnitVentilators->OAMassFlowRate = MinOAFrac * outsideAirNode.MassFlowRate;

                            } else { // Tinlet < Toutdoor

                                MaxOAFrac = unitVent.maxOASched->getCurrentVal();
                                state.dataUnitVentilators->OAMassFlowRate = MaxOAFrac * outsideAirNode.MassFlowRate;
                            }
                        } break;

                        case OAControl::FixedTemperature: {
                            // In heating mode, the outside air for "fixed temperature" attempts
                            // to control the outside air fraction so that a desired temperature
                            // is met (if possible).  If this desired temperature is between the
                            // outside air temperature and the zone air temperature (inlet air
                            // temperature), then this is possible.  If not, the control will try
                            // to maximize the amount of air coming from the source that is closer
                            // in temperature to the desired temperature.
                            Tdesired = unitVent.tempSched->getCurrentVal();
                            Tinlet = inletNode.Temp;
                            Toutdoor = outsideAirNode.Temp;
                            MaxOAFrac = 1.0;

                            if (std::abs(Tinlet - Toutdoor) <= LowTempDiff) { // no difference in indoor and outdoor conditions-->set OA to minimum
                                state.dataUnitVentilators->OAMassFlowRate = MinOAFrac * outsideAirNode.MassFlowRate;
                            } else if (std::abs(MaxOAFrac - MinOAFrac) <= LowOAFracDiff) { // no difference in outside air fractions
                                state.dataUnitVentilators->OAMassFlowRate = MinOAFrac * outsideAirNode.MassFlowRate;
                            } else if (((Tdesired <= Tinlet) && (Tdesired >= Toutdoor)) || ((Tdesired >= Tinlet) && (Tdesired <= Toutdoor))) {
                                // Desired temperature is between the inlet and outdoor temperatures
                                // so vary the flow rate between no outside air and no recirculation air
                                // then applying the maximum and minimum limits the user has scheduled
                                // to make sure too much/little outside air is being introduced
                                state.dataUnitVentilators->OAMassFlowRate = ((Tdesired - Tinlet) / (Toutdoor - Tinlet)) * inletNode.MassFlowRate;
                                state.dataUnitVentilators->OAMassFlowRate =
                                    max(state.dataUnitVentilators->OAMassFlowRate, (MinOAFrac * outsideAirNode.MassFlowRate));
                                state.dataUnitVentilators->OAMassFlowRate =
                                    min(state.dataUnitVentilators->OAMassFlowRate, (MaxOAFrac * outsideAirNode.MassFlowRate));
                            } else if ((Tdesired < Tinlet) && (Tdesired < Toutdoor)) {
                                // Desired temperature is below both the inlet and outdoor temperatures
                                // so use whichever flow rate (max or min) that will get closer
                                if (Tinlet < Toutdoor) { // Tinlet closer to Tdesired so use minimum outside air
                                    state.dataUnitVentilators->OAMassFlowRate = MinOAFrac * outsideAirNode.MassFlowRate;
                                } else { // Toutdoor closer to Tdesired so use maximum outside air
                                    state.dataUnitVentilators->OAMassFlowRate = MaxOAFrac * outsideAirNode.MassFlowRate;
                                }
                            } else if ((Tdesired > Tinlet) && (Tdesired > Toutdoor)) {
                                // Desired temperature is above both the inlet and outdoor temperatures
                                // so use whichever flow rate (max or min) that will get closer
                                if (Tinlet > Toutdoor) { // Tinlet closer to Tdesired so use minimum outside air
                                    state.dataUnitVentilators->OAMassFlowRate = MinOAFrac * outsideAirNode.MassFlowRate;
                                } else { // Toutdoor closer to Tdesired so use maximum outside air
                                    state.dataUnitVentilators->OAMassFlowRate = MaxOAFrac * outsideAirNode.MassFlowRate;
                                }
                            } else {
                                // It should NEVER get to this point, but just in case...
                                ShowFatalError(state,
                                               format("ZoneHVAC:UnitVentilator simulation control: illogical condition for {}", unitVent.Name));
                            }
                        } break;
                        default: {
                            assert(false);
                        } break;
                        }
                    }

                    if (fanOp == HVAC::FanOp::Cycling) {
                        CalcUnitVentilatorComponents(state, UnitVentNum, FirstHVACIteration, QUnitOut, fanOp, PartLoadFrac);
                        if (inletNode.MassFlowRateMax > 0.0) {
                            unitVent.FanPartLoadRatio = inletNode.MassFlowRate / inletNode.MassFlowRateMax;
                        }
                    } else {
                        CalcUnitVentilatorComponents(state, UnitVentNum, FirstHVACIteration, QUnitOut);
                    }

                } else { //  Coil/no coil block
                    // There is a heating load and there is a heating coil present (presumably).
                    // Variable percent will throttle outside air back to the minimum while
                    // fixed temperature will still try to vary the outside air amount to meet
                    // the desired mixed air temperature.

                    {
                        switch (unitVent.OAControlType) {
                        case OAControl::FixedAmount: {
                            // In this control type, the outdoor air flow rate is fixed to the maximum value
                            // which is equal to the minimum value, regardless of all the other conditions.
                            state.dataUnitVentilators->OAMassFlowRate = MinOAFrac * outsideAirNode.MassFlowRate;
                        } break;
                        case OAControl::VariablePercent: {
                            // In heating mode, the outside air for "variable percent" control
                            // is set to the minimum value
                            state.dataUnitVentilators->OAMassFlowRate = MinOAFrac * outsideAirNode.MassFlowRate;
                        } break;

                        case OAControl::FixedTemperature: {
                            // In heating mode, the outside air for "fixed temperature" attempts
                            // to control the outside air fraction so that a desired temperature
                            // is met (if possible).  If this desired temperature is between the
                            // outside air temperature and the zone air temperature (inlet air
                            // temperature), then this is possible.  If not, the control will try
                            // to maximize the amount of air coming from the source that is closer
                            // in temperature to the desired temperature.
                            Tdesired = unitVent.tempSched->getCurrentVal();
                            Tinlet = inletNode.Temp;
                            Toutdoor = outsideAirNode.Temp;
                            MaxOAFrac = 1.0;

                            if (std::abs(Tinlet - Toutdoor) <= LowTempDiff) { // no difference in indoor and outdoor conditions-->set OA to minimum
                                state.dataUnitVentilators->OAMassFlowRate = MinOAFrac * outsideAirNode.MassFlowRate;
                            } else if (std::abs(MaxOAFrac - MinOAFrac) <= LowOAFracDiff) { // no difference in outside air fractions
                                state.dataUnitVentilators->OAMassFlowRate = MinOAFrac * outsideAirNode.MassFlowRate;
                            } else if (((Tdesired <= Tinlet) && (Tdesired >= Toutdoor)) || ((Tdesired >= Tinlet) && (Tdesired <= Toutdoor))) {
                                // Desired temperature is between the inlet and outdoor temperatures
                                // so vary the flow rate between no outside air and no recirculation air
                                // then applying the maximum and minimum limits the user has scheduled
                                // to make sure too much/little outside air is being introduced
                                state.dataUnitVentilators->OAMassFlowRate = ((Tdesired - Tinlet) / (Toutdoor - Tinlet)) * inletNode.MassFlowRate;
                                state.dataUnitVentilators->OAMassFlowRate =
                                    max(state.dataUnitVentilators->OAMassFlowRate, (MinOAFrac * outsideAirNode.MassFlowRate));
                                state.dataUnitVentilators->OAMassFlowRate =
                                    min(state.dataUnitVentilators->OAMassFlowRate, (MaxOAFrac * outsideAirNode.MassFlowRate));
                            } else if ((Tdesired < Tinlet) && (Tdesired < Toutdoor)) {
                                // Desired temperature is below both the inlet and outdoor temperatures
                                // so use whichever flow rate (max or min) that will get closer
                                if (Tinlet < Toutdoor) { // Tinlet closer to Tdesired so use minimum outside air
                                    state.dataUnitVentilators->OAMassFlowRate = MinOAFrac * outsideAirNode.MassFlowRate;
                                } else { // Toutdoor closer to Tdesired so use maximum outside air
                                    state.dataUnitVentilators->OAMassFlowRate = MaxOAFrac * outsideAirNode.MassFlowRate;
                                }
                            } else if ((Tdesired > Tinlet) && (Tdesired > Toutdoor)) {
                                // Desired temperature is above both the inlet and outdoor temperatures
                                // so use whichever flow rate (max or min) that will get closer
                                if (Tinlet > Toutdoor) { // Tinlet closer to Tdesired so use minimum outside air
                                    state.dataUnitVentilators->OAMassFlowRate = MinOAFrac * outsideAirNode.MassFlowRate;
                                } else { // Toutdoor closer to Tdesired so use maximum outside air
                                    state.dataUnitVentilators->OAMassFlowRate = MaxOAFrac * outsideAirNode.MassFlowRate;
                                }
                            } else {
                                // It should NEVER get to this point, but just in case...
                                ShowFatalError(state,
                                               format("ZoneHVAC:UnitVentilator simulation control: illogical condition for {}", unitVent.Name));
                            }
                        } break;
                        default: {
                            assert(false);
                        } break;
                        }
                    }

                    if (fanOp == HVAC::FanOp::Cycling) {

                        // Find part load ratio of unit ventilator coils
                        PartLoadFrac = 0.0;
                        CalcUnitVentilatorComponents(state, UnitVentNum, FirstHVACIteration, NoOutput, fanOp, PartLoadFrac);
                        if ((NoOutput - state.dataUnitVentilators->QZnReq) < HVAC::SmallLoad) {
                            // Unit ventilator is unable to meet the load with coil off, set PLR = 1
                            PartLoadFrac = 1.0;
                            CalcUnitVentilatorComponents(state, UnitVentNum, FirstHVACIteration, FullOutput, fanOp, PartLoadFrac);
                            if ((FullOutput - state.dataUnitVentilators->QZnReq) > HVAC::SmallLoad) {
                                // Unit ventilator full load capacity is able to meet the load, Find PLR
                                // Tolerance is in fraction of load, MaxIter = 30, SolFalg = # of iterations or error as appropriate
                                auto f = [&state, UnitVentNum, FirstHVACIteration, fanOp](Real64 const PartLoadRatio) {
                                    Real64 QUnitOut = 0.0; // heating/Cooling provided by unit ventilator [watts]
                                    CalcUnitVentilatorComponents(state, UnitVentNum, FirstHVACIteration, QUnitOut, fanOp, PartLoadRatio);
                                    if (state.dataUnitVentilators->QZnReq) {
                                        return (QUnitOut - state.dataUnitVentilators->QZnReq) / state.dataUnitVentilators->QZnReq;
                                    } else {
                                        return 0.0;
                                    }
                                };
                                General::SolveRoot(state, 0.001, MaxIter, SolFlag, PartLoadFrac, f, 0.0, 1.0);
                            }
                        }

                        CalcUnitVentilatorComponents(state, UnitVentNum, FirstHVACIteration, QUnitOut, fanOp, PartLoadFrac);
                        unitVent.PartLoadFrac = PartLoadFrac;
                        unitVent.FanPartLoadRatio = PartLoadFrac;

                    } else { // Not a cycling operating mode

                        {
                            switch (unitVent.HCoilType) {
                            case HeatCoilType::Water: {
                                // control water flow to obtain output matching QZnReq
                                ControlCompOutput(state,
                                                  unitVent.Name,
                                                  state.dataUnitVentilators->cMO_UnitVentilator,
                                                  UnitVentNum,
                                                  FirstHVACIteration,
                                                  state.dataUnitVentilators->QZnReq,
                                                  ControlNode,
                                                  MaxWaterFlow,
                                                  MinWaterFlow,
                                                  ControlOffset,
                                                  unitVent.ControlCompTypeNum,
                                                  unitVent.CompErrIndex,
                                                  _,
                                                  _,
                                                  _,
                                                  _,
                                                  _,
                                                  unitVent.HWplantLoc);
                            } break;
                            case HeatCoilType::Gas:
                            case HeatCoilType::Electric:
                            case HeatCoilType::Steam: {
                                CalcUnitVentilatorComponents(state, UnitVentNum, FirstHVACIteration, QUnitOut);
                            } break;
                            default: {
                                assert(false);
                            } break;
                            }
                        }
                    }
                } //  Coil/no coil block

            } else { // COOLING MODE

                int ControlNode = unitVent.ColdControlNode;
                ControlOffset = unitVent.ColdControlOffset;
                MaxWaterFlow = unitVent.MaxColdWaterFlow;
                MinWaterFlow = unitVent.MinColdWaterFlow;
                // On the first HVAC iteration the system values are given to the controller, but after that
                // the demand limits are in place and there needs to be feedback to the Zone Equipment
                if ((!FirstHVACIteration) && (ControlNode > 0) && (unitVent.CCoilPresent)) {
                    MaxWaterFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRateMaxAvail;
                    MinWaterFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRateMinAvail;
                }
                state.dataUnitVentilators->HCoilOn = false;

                Tinlet = inletNode.Temp;
                Toutdoor = outsideAirNode.Temp;

                if (outsideAirNode.MassFlowRate > 0.0) {
                    MinOAFrac = unitVent.minOASched->getCurrentVal() * (unitVent.MinOutAirMassFlow / outsideAirNode.MassFlowRate);
                } else {
                    MinOAFrac = 0.0;
                }
                MinOAFrac = min(1.0, max(0.0, MinOAFrac));

                if ((!unitVent.CCoilPresent) || (unitVent.CCoilSchedValue <= 0.0)) {
                    // In cooling mode, but there is no coil to provide cooling.  This is handled
                    // differently than if there was a cooling coil present.  Fixed temperature
                    // will still try to vary the amount of outside air to meet the desired
                    // mixed air temperature, while variable percent will go to full ventilation
                    // when it is most advantageous.
                    {
                        switch (unitVent.OAControlType) {
                        case OAControl::FixedAmount: {
                            // In this control type, the outdoor air flow rate is fixed to the maximum value
                            // which is equal to the minimum value, regardless of all the other conditions.
                            state.dataUnitVentilators->OAMassFlowRate = MinOAFrac * outsideAirNode.MassFlowRate;
                        } break;
                        case OAControl::VariablePercent: {
                            state.dataUnitVentilators->OAMassFlowRate = SetOAMassFlowRateForCoolingVariablePercent(
                                state, UnitVentNum, MinOAFrac, outsideAirNode.MassFlowRate, unitVent.maxOASched->getCurrentVal(), Tinlet, Toutdoor);
                        } break;

                        case OAControl::FixedTemperature: {
                            // This is basically the same algorithm as for the heating case...
                            Tdesired = unitVent.tempSched->getCurrentVal();
                            MaxOAFrac = 1.0;

                            if (std::abs(Tinlet - Toutdoor) <= LowTempDiff) { // no difference in indoor and outdoor conditions-->set OA to minimum
                                state.dataUnitVentilators->OAMassFlowRate = MinOAFrac * outsideAirNode.MassFlowRate;
                            } else if (std::abs(MaxOAFrac - MinOAFrac) <= LowOAFracDiff) { // no difference in outside air fractions
                                state.dataUnitVentilators->OAMassFlowRate = MinOAFrac * outsideAirNode.MassFlowRate;
                            } else if (((Tdesired <= Tinlet) && (Tdesired >= Toutdoor)) || ((Tdesired >= Tinlet) && (Tdesired <= Toutdoor))) {
                                // Desired temperature is between the inlet and outdoor temperatures
                                // so vary the flow rate between no outside air and no recirculation air
                                // then applying the maximum and minimum limits the user has scheduled
                                // to make sure too much/little outside air is being introduced
                                state.dataUnitVentilators->OAMassFlowRate = ((Tdesired - Tinlet) / (Toutdoor - Tinlet)) * inletNode.MassFlowRate;
                                state.dataUnitVentilators->OAMassFlowRate =
                                    max(state.dataUnitVentilators->OAMassFlowRate, (MinOAFrac * outsideAirNode.MassFlowRate));
                                state.dataUnitVentilators->OAMassFlowRate =
                                    min(state.dataUnitVentilators->OAMassFlowRate, (MaxOAFrac * outsideAirNode.MassFlowRate));
                            } else if ((Tdesired < Tinlet) && (Tdesired < Toutdoor)) {
                                // Desired temperature is below both the inlet and outdoor temperatures
                                // so use whichever flow rate (max or min) that will get closer
                                if (Tinlet < Toutdoor) { // Tinlet closer to Tdesired so use minimum outside air
                                    state.dataUnitVentilators->OAMassFlowRate = MinOAFrac * outsideAirNode.MassFlowRate;
                                } else { // Toutdoor closer to Tdesired so use maximum outside air
                                    state.dataUnitVentilators->OAMassFlowRate = MaxOAFrac * outsideAirNode.MassFlowRate;
                                }
                            } else if ((Tdesired > Tinlet) && (Tdesired > Toutdoor)) {
                                // Desired temperature is above both the inlet and outdoor temperatures
                                // so use whichever flow rate (max or min) that will get closer
                                if (Tinlet > Toutdoor) { // Tinlet closer to Tdesired so use minimum outside air
                                    state.dataUnitVentilators->OAMassFlowRate = MinOAFrac * outsideAirNode.MassFlowRate;
                                } else { // Toutdoor closer to Tdesired so use maximum outside air
                                    state.dataUnitVentilators->OAMassFlowRate = MaxOAFrac * outsideAirNode.MassFlowRate;
                                }
                            } else {
                                // It should NEVER get to this point, but just in case...
                                ShowFatalError(state,
                                               format("ZoneHVAC:UnitVentilator simulation control: illogical condition for {}", unitVent.Name));
                            }
                        } break;
                        default: {
                            assert(false);
                        } break;
                        }
                    }

                    if (fanOp == HVAC::FanOp::Cycling) {
                        CalcUnitVentilatorComponents(state, UnitVentNum, FirstHVACIteration, QUnitOut, fanOp, PartLoadFrac);
                        if (inletNode.MassFlowRateMax > 0.0) {
                            unitVent.FanPartLoadRatio = inletNode.MassFlowRate / inletNode.MassFlowRateMax;
                        }
                    } else {
                        CalcUnitVentilatorComponents(state, UnitVentNum, FirstHVACIteration, QUnitOut);
                    }

                } else {
                    // There is a cooling load and there is a cooling coil present (presumably).
                    // Variable percent will throttle outside air back to the minimum while
                    // fixed temperature will still try to vary the outside air amount to meet
                    // the desired mixed air temperature.

                    {
                        switch (unitVent.OAControlType) {
                        case OAControl::FixedAmount: {
                            // In this control type, the outdoor air flow rate is fixed to the maximum value
                            // which is equal to the minimum value, regardless of all the other conditions.
                            state.dataUnitVentilators->OAMassFlowRate = MinOAFrac * outsideAirNode.MassFlowRate;
                        } break;

                        case OAControl::VariablePercent: {
                            state.dataUnitVentilators->OAMassFlowRate = SetOAMassFlowRateForCoolingVariablePercent(
                                state, UnitVentNum, MinOAFrac, outsideAirNode.MassFlowRate, unitVent.maxOASched->getCurrentVal(), Tinlet, Toutdoor);
                        } break;

                        case OAControl::FixedTemperature: {
                            // This is basically the same algorithm as for the heating case...
                            Tdesired = unitVent.tempSched->getCurrentVal();

                            MaxOAFrac = 1.0;

                            if (std::abs(Tinlet - Toutdoor) <= LowTempDiff) { // no difference in indoor and outdoor conditions-->set OA to minimum
                                state.dataUnitVentilators->OAMassFlowRate = MinOAFrac * outsideAirNode.MassFlowRate;
                            } else if (std::abs(MaxOAFrac - MinOAFrac) <= LowOAFracDiff) { // no difference in outside air fractions
                                state.dataUnitVentilators->OAMassFlowRate = MinOAFrac * outsideAirNode.MassFlowRate;
                            } else if (((Tdesired <= Tinlet) && (Tdesired >= Toutdoor)) || ((Tdesired >= Tinlet) && (Tdesired <= Toutdoor))) {
                                // Desired temperature is between the inlet and outdoor temperatures
                                // so vary the flow rate between no outside air and no recirculation air
                                // then applying the maximum and minimum limits the user has scheduled
                                // to make sure too much/little outside air is being introduced
                                state.dataUnitVentilators->OAMassFlowRate = ((Tdesired - Tinlet) / (Toutdoor - Tinlet)) * inletNode.MassFlowRate;
                                state.dataUnitVentilators->OAMassFlowRate =
                                    max(state.dataUnitVentilators->OAMassFlowRate, (MinOAFrac * outsideAirNode.MassFlowRate));
                                state.dataUnitVentilators->OAMassFlowRate =
                                    min(state.dataUnitVentilators->OAMassFlowRate, (MaxOAFrac * outsideAirNode.MassFlowRate));
                            } else if ((Tdesired < Tinlet) && (Tdesired < Toutdoor)) {
                                // Desired temperature is below both the inlet and outdoor temperatures
                                // so use whichever flow rate (max or min) that will get closer
                                if (Tinlet < Toutdoor) { // Tinlet closer to Tdesired so use minimum outside air
                                    state.dataUnitVentilators->OAMassFlowRate = MinOAFrac * outsideAirNode.MassFlowRate;
                                } else { // Toutdoor closer to Tdesired so use maximum outside air
                                    state.dataUnitVentilators->OAMassFlowRate = MaxOAFrac * outsideAirNode.MassFlowRate;
                                }
                            } else if ((Tdesired > Tinlet) && (Tdesired > Toutdoor)) {
                                // Desired temperature is above both the inlet and outdoor temperatures
                                // so use whichever flow rate (max or min) that will get closer
                                if (Tinlet > Toutdoor) { // Tinlet closer to Tdesired so use minimum outside air
                                    state.dataUnitVentilators->OAMassFlowRate = MinOAFrac * outsideAirNode.MassFlowRate;
                                } else { // Toutdoor closer to Tdesired so use maximum outside air
                                    state.dataUnitVentilators->OAMassFlowRate = MaxOAFrac * outsideAirNode.MassFlowRate;
                                }
                            } else {
                                // It should NEVER get to this point, but just in case...
                                ShowFatalError(state,
                                               format("ZoneHVAC:UnitVentilator simulation control: illogical condition for {}", unitVent.Name));
                            }
                        } break;
                        default: {
                            assert(false);
                        } break;
                        }
                    }

                    if (fanOp == HVAC::FanOp::Cycling) {

                        state.dataUnitVentilators->HCoilOn = false;
                        // Find part load ratio of unit ventilator coils
                        PartLoadFrac = 0.0;
                        CalcUnitVentilatorComponents(state, UnitVentNum, FirstHVACIteration, NoOutput, fanOp, PartLoadFrac);
                        if ((NoOutput - state.dataUnitVentilators->QZnReq) > HVAC::SmallLoad) {
                            // Unit ventilator is unable to meet the load with coil off, set PLR = 1
                            PartLoadFrac = 1.0;
                            CalcUnitVentilatorComponents(state, UnitVentNum, FirstHVACIteration, FullOutput, fanOp, PartLoadFrac);
                            if ((FullOutput - state.dataUnitVentilators->QZnReq) < HVAC::SmallLoad) {
                                // Unit ventilator full load capacity is able to meet the load, Find PLR
                                // Tolerance is in fraction of load, MaxIter = 30, SolFalg = # of iterations or error as appropriate
                                auto f = [&state, UnitVentNum, FirstHVACIteration, fanOp](Real64 const PartLoadRatio) {
                                    Real64 QUnitOut = 0.0; // heating/Cooling provided by unit ventilator [watts]

                                    // Convert parameters to usable variables
                                    CalcUnitVentilatorComponents(state, UnitVentNum, FirstHVACIteration, QUnitOut, fanOp, PartLoadRatio);
                                    if (state.dataUnitVentilators->QZnReq) {
                                        return (QUnitOut - state.dataUnitVentilators->QZnReq) / state.dataUnitVentilators->QZnReq;
                                    }
                                    return 0.0;
                                };
                                General::SolveRoot(state, 0.001, MaxIter, SolFlag, PartLoadFrac, f, 0.0, 1.0);
                            }
                        }
                        CalcUnitVentilatorComponents(state, UnitVentNum, FirstHVACIteration, QUnitOut, fanOp, PartLoadFrac);
                        unitVent.PartLoadFrac = PartLoadFrac;
                        unitVent.FanPartLoadRatio = PartLoadFrac;

                    } else { // NOT a cycling operating mode
                        // control water flow to obtain output matching QZnReq
                        state.dataUnitVentilators->HCoilOn = false;
                        ControlCompOutput(state,
                                          unitVent.Name,
                                          state.dataUnitVentilators->cMO_UnitVentilator,
                                          UnitVentNum,
                                          FirstHVACIteration,
                                          state.dataUnitVentilators->QZnReq,
                                          ControlNode,
                                          MaxWaterFlow,
                                          MinWaterFlow,
                                          ControlOffset,
                                          unitVent.ControlCompTypeNum,
                                          unitVent.CompErrIndex,
                                          _,
                                          _,
                                          _,
                                          _,
                                          _,
                                          unitVent.CWPlantLoc);

                    } // end from IF (fanOp .EQ. HVAC::FanOp::Cycling) THEN
                }

            } // ...end of HEATING/COOLING IF-THEN block

            AirMassFlow = outletNode.MassFlowRate;
            QUnitOut = AirMassFlow *
                       (Psychrometrics::PsyHFnTdbW(outletNode.Temp, inletNode.HumRat) - Psychrometrics::PsyHFnTdbW(inletNode.Temp, inletNode.HumRat));

        } // ...end of unit ON/OFF IF-THEN block

        Real64 QTotUnitOut = AirMassFlow * (outletNode.Enthalpy - inletNode.Enthalpy);

        // Report variables...
        unitVent.HeatPower = max(0.0, QUnitOut);
        unitVent.SensCoolPower = std::abs(min(0.0, QUnitOut));
        unitVent.TotCoolPower = std::abs(min(0.0, QTotUnitOut));
        unitVent.ElecPower = state.dataFans->fans(unitVent.Fan_Index)->totalPower;

        PowerMet = QUnitOut;
        LatOutputProvided = AirMassFlow * (outletNode.HumRat - inletNode.HumRat); // Latent rate (kg/s), dehumid = negative;
    }

    void CalcUnitVentilatorComponents(EnergyPlusData &state,
                                      int const UnitVentNum,                         // Unit index in unit ventilator array
                                      bool const FirstHVACIteration,                 // flag for 1st HVAV iteration in the time step
                                      Real64 &LoadMet,                               // load met by unit (watts)
                                      ObjexxFCL::Optional<HVAC::FanOp const> fanOp,  // Fan Type
                                      ObjexxFCL::Optional<Real64 const> PartLoadFrac // Part Load Ratio of coil and fan
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   May 2000
        //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine launches the individual component simulations.
        // This is called either when the unit is off to carry null conditions
        // through the unit or during control iterations to continue updating
        // what is going on within the unit.

        // METHODOLOGY EMPLOYED:
        // Simply calls the different components in order.  Only slight wrinkles
        // here are that the unit ventilator has it's own outside air mixer and
        // that a cooling coil must be present in order to call a cooling coil
        // simulation.  Other than that, the subroutine is very straightforward.

        auto &unitVent = state.dataUnitVentilators->UnitVent(UnitVentNum);

        Real64 mdot; // hot water or steam mass flow rate for current time step

        auto &inletNode = state.dataLoopNodes->Node(unitVent.AirInNode);
        auto &outletNode = state.dataLoopNodes->Node(unitVent.AirOutNode);
        state.dataUnitVentilators->ZoneNode = state.dataZoneEquip->ZoneEquipConfig(unitVent.ZonePtr).ZoneNode;
        Real64 QCoilReq = state.dataUnitVentilators->QZnReq;

        if (fanOp != HVAC::FanOp::Cycling) {

            if (unitVent.ATMixerExists) {
                state.dataUnitVentilators->ATMixOutNode = unitVent.ATMixerOutNode;
                state.dataUnitVentilators->ATMixerPriNode = unitVent.ATMixerPriNode;
                if (unitVent.ATMixerType == HVAC::MixerType::InletSide) {
                    // set the primary air inlet mass flow rate
                    state.dataLoopNodes->Node(state.dataUnitVentilators->ATMixerPriNode).MassFlowRate =
                        min(min(state.dataLoopNodes->Node(state.dataUnitVentilators->ATMixerPriNode).MassFlowRateMaxAvail,
                                state.dataUnitVentilators->OAMassFlowRate),
                            inletNode.MassFlowRate);
                    // now calculate the the mixer outlet conditions (and the secondary air inlet flow rate)
                    SingleDuct::SimATMixer(state, unitVent.ATMixerName, FirstHVACIteration, unitVent.ATMixerIndex);
                }
            } else {
                SimUnitVentOAMixer(state, UnitVentNum, fanOp);
            }
            if (unitVent.fanType != HVAC::FanType::SystemModel) {
                state.dataFans->fans(unitVent.Fan_Index)->simulate(state, FirstHVACIteration);
            } else {
                state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0; // used for cycling fan, set to 1.0 to be sure
                state.dataFans->fans(unitVent.Fan_Index)->simulate(state, FirstHVACIteration);
            }

            if (unitVent.CCoilPresent) {
                if (unitVent.CCoilType == CoolCoilType::HXAssisted) {
                    HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(
                        state, unitVent.CCoilName, FirstHVACIteration, HVAC::CompressorOp::On, 0.0, unitVent.CCoil_Index, HVAC::FanOp::Continuous);
                } else {
                    WaterCoils::SimulateWaterCoilComponents(state, unitVent.CCoilName, FirstHVACIteration, unitVent.CCoil_Index);
                }
            }

            if (unitVent.HCoilPresent) {

                {
                    switch (unitVent.HCoilType) {
                    case HeatCoilType::Water: {
                        WaterCoils::SimulateWaterCoilComponents(state, unitVent.HCoilName, FirstHVACIteration, unitVent.HCoil_Index);
                    } break;
                    case HeatCoilType::Steam: {
                        if (!state.dataUnitVentilators->HCoilOn) {
                            QCoilReq = 0.0;
                        } else {
                            int HCoilInAirNode = unitVent.FanOutletNode;
                            Real64 CpAirZn = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(unitVent.AirInNode).HumRat);
                            QCoilReq = state.dataUnitVentilators->QZnReq -
                                       state.dataLoopNodes->Node(HCoilInAirNode).MassFlowRate * CpAirZn *
                                           (state.dataLoopNodes->Node(HCoilInAirNode).Temp - state.dataLoopNodes->Node(unitVent.AirInNode).Temp);
                        }

                        if (QCoilReq < 0.0) {
                            QCoilReq = 0.0; // a heating coil can only heat, not cool
                        }

                        SteamCoils::SimulateSteamCoilComponents(state, unitVent.HCoilName, FirstHVACIteration, unitVent.HCoil_Index, QCoilReq);
                    } break;
                    case HeatCoilType::Electric:
                    case HeatCoilType::Gas: {
                        if (!state.dataUnitVentilators->HCoilOn) {
                            QCoilReq = 0.0;
                        } else {
                            int HCoilInAirNode = unitVent.FanOutletNode;
                            Real64 CpAirZn = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(unitVent.AirInNode).HumRat);
                            QCoilReq = state.dataUnitVentilators->QZnReq -
                                       state.dataLoopNodes->Node(HCoilInAirNode).MassFlowRate * CpAirZn *
                                           (state.dataLoopNodes->Node(HCoilInAirNode).Temp - state.dataLoopNodes->Node(unitVent.AirInNode).Temp);
                        }

                        if (QCoilReq < 0.0) {
                            QCoilReq = 0.0; // a heating coil can only heat, not cool
                        }

                        HeatingCoils::SimulateHeatingCoilComponents(state, unitVent.HCoilName, FirstHVACIteration, QCoilReq, unitVent.HCoil_Index);
                    } break;
                    default: {
                        assert(false);
                    } break;
                    }
                }

            } // (UnitVent(UnitVentNum)%HCoilPresent)

        } else { // Fan is Fan:OnOff and is cycling

            inletNode.MassFlowRate = inletNode.MassFlowRateMax * PartLoadFrac;
            // Set the fan inlet node maximum available mass flow rates for cycling fans
            inletNode.MassFlowRateMaxAvail = inletNode.MassFlowRate;

            if (unitVent.ATMixerExists) {
                state.dataUnitVentilators->ATMixOutNode = unitVent.ATMixerOutNode;
                state.dataUnitVentilators->ATMixerPriNode = unitVent.ATMixerPriNode;
                if (unitVent.ATMixerType == HVAC::MixerType::InletSide) {
                    // set the primary air inlet mass flow rate
                    state.dataLoopNodes->Node(state.dataUnitVentilators->ATMixerPriNode).MassFlowRate =
                        min(min(state.dataLoopNodes->Node(state.dataUnitVentilators->ATMixerPriNode).MassFlowRateMaxAvail,
                                state.dataUnitVentilators->OAMassFlowRate),
                            inletNode.MassFlowRate);
                    // now calculate the mixer outlet conditions (and the secondary air inlet flow rate)
                    SingleDuct::SimATMixer(state, unitVent.ATMixerName, FirstHVACIteration, unitVent.ATMixerIndex);
                }
            } else {
                SimUnitVentOAMixer(state, UnitVentNum, fanOp);
            }

            state.dataFans->fans(unitVent.Fan_Index)->simulate(state, FirstHVACIteration, _, _);

            if (unitVent.CCoilPresent) {

                // where is mdot set ?
                CalcMdotCCoilCycFan(state, mdot, QCoilReq, state.dataUnitVentilators->QZnReq, UnitVentNum, PartLoadFrac);
                PlantUtilities::SetComponentFlowRate(state, mdot, unitVent.ColdControlNode, unitVent.ColdCoilOutNodeNum, unitVent.CWPlantLoc);

                if (unitVent.CCoilType == CoolCoilType::HXAssisted) {
                    HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(
                        state, unitVent.CCoilName, FirstHVACIteration, HVAC::CompressorOp::On, PartLoadFrac, unitVent.CCoil_Index, fanOp);
                } else {
                    WaterCoils::SimulateWaterCoilComponents(
                        state, unitVent.CCoilName, FirstHVACIteration, unitVent.CCoil_Index, QCoilReq, fanOp, PartLoadFrac);
                }
            }

            if (unitVent.HCoilPresent) {

                {
                    switch (unitVent.HCoilType) {
                    case HeatCoilType::Water: {
                        if (!state.dataUnitVentilators->HCoilOn) {
                            QCoilReq = 0.0;
                            mdot = 0.0;
                        } else {
                            int HCoilInAirNode = unitVent.FanOutletNode;
                            Real64 CpAirZn = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(unitVent.AirInNode).HumRat);
                            QCoilReq = state.dataUnitVentilators->QZnReq -
                                       state.dataLoopNodes->Node(HCoilInAirNode).MassFlowRate * CpAirZn *
                                           (state.dataLoopNodes->Node(HCoilInAirNode).Temp - state.dataLoopNodes->Node(unitVent.AirInNode).Temp);
                            mdot = unitVent.MaxHotWaterFlow * PartLoadFrac;
                        }

                        if (QCoilReq < 0.0) {
                            QCoilReq = 0.0; // a heating coil can only heat, not cool
                        }
                        PlantUtilities::SetComponentFlowRate(state, mdot, unitVent.HotControlNode, unitVent.HotCoilOutNodeNum, unitVent.HWplantLoc);
                        WaterCoils::SimulateWaterCoilComponents(
                            state, unitVent.HCoilName, FirstHVACIteration, unitVent.HCoil_Index, QCoilReq, fanOp, PartLoadFrac);
                    } break;
                    case HeatCoilType::Steam: {
                        if (!state.dataUnitVentilators->HCoilOn) {
                            QCoilReq = 0.0;
                            mdot = 0.0;
                        } else {
                            int HCoilInAirNode = unitVent.FanOutletNode;
                            Real64 CpAirZn = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(unitVent.AirInNode).HumRat);
                            QCoilReq = state.dataUnitVentilators->QZnReq -
                                       state.dataLoopNodes->Node(HCoilInAirNode).MassFlowRate * CpAirZn *
                                           (state.dataLoopNodes->Node(HCoilInAirNode).Temp - state.dataLoopNodes->Node(unitVent.AirInNode).Temp);
                            mdot = unitVent.MaxHotSteamFlow * PartLoadFrac;
                        }

                        if (QCoilReq < 0.0) {
                            QCoilReq = 0.0;
                        }
                        PlantUtilities::SetComponentFlowRate(state, mdot, unitVent.HotControlNode, unitVent.HotCoilOutNodeNum, unitVent.HWplantLoc);
                        SteamCoils::SimulateSteamCoilComponents(
                            state, unitVent.HCoilName, FirstHVACIteration, unitVent.HCoil_Index, QCoilReq, _, fanOp, PartLoadFrac);
                    } break;
                    case HeatCoilType::Electric:
                    case HeatCoilType::Gas: {
                        if (!state.dataUnitVentilators->HCoilOn) {
                            QCoilReq = 0.0;
                        } else {
                            int HCoilInAirNode = unitVent.FanOutletNode;
                            Real64 CpAirZn = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(unitVent.AirInNode).HumRat);
                            QCoilReq = state.dataUnitVentilators->QZnReq -
                                       state.dataLoopNodes->Node(HCoilInAirNode).MassFlowRate * CpAirZn *
                                           (state.dataLoopNodes->Node(HCoilInAirNode).Temp - state.dataLoopNodes->Node(unitVent.AirInNode).Temp);
                        }
                        if (QCoilReq < 0.0) {
                            QCoilReq = 0.0;
                        }
                        HeatingCoils::SimulateHeatingCoilComponents(
                            state, unitVent.HCoilName, FirstHVACIteration, QCoilReq, unitVent.HCoil_Index, _, _, fanOp, PartLoadFrac);
                    } break;
                    default: {
                        assert(false);
                    } break;
                    }
                }
            }
        }
        Real64 AirMassFlow = outletNode.MassFlowRate;
        // calculate delivered load
        if (unitVent.ATMixerExists) {
            auto &ATMixOutNode(state.dataLoopNodes->Node(state.dataUnitVentilators->ATMixOutNode));
            auto &ATMixerPriNode(state.dataLoopNodes->Node(state.dataUnitVentilators->ATMixerPriNode));
            if (unitVent.ATMixerType == HVAC::MixerType::SupplySide) {
                // set the primary air inlet mass flow rate
                ATMixerPriNode.MassFlowRate = min(ATMixerPriNode.MassFlowRateMaxAvail, state.dataUnitVentilators->OAMassFlowRate);
                // now calculate the the mixer outlet conditions (and the secondary air inlet flow rate)
                SingleDuct::SimATMixer(state, unitVent.ATMixerName, FirstHVACIteration, unitVent.ATMixerIndex);
                Real64 SpecHumMin = min(ATMixOutNode.HumRat, inletNode.HumRat); // (kg moisture / kg moist air)
                LoadMet = ATMixOutNode.MassFlowRate *
                          (Psychrometrics::PsyHFnTdbW(ATMixOutNode.Temp, SpecHumMin) - Psychrometrics::PsyHFnTdbW(inletNode.Temp, SpecHumMin));
            } else {
                // ATM Mixer on inlet side
                auto &zoneNode(state.dataLoopNodes->Node(state.dataUnitVentilators->ZoneNode));
                LoadMet = AirMassFlow *
                          (Psychrometrics::PsyHFnTdbW(outletNode.Temp, zoneNode.HumRat) - Psychrometrics::PsyHFnTdbW(zoneNode.Temp, zoneNode.HumRat));
            }
        } else {
            LoadMet = AirMassFlow *
                      (Psychrometrics::PsyHFnTdbW(outletNode.Temp, inletNode.HumRat) - Psychrometrics::PsyHFnTdbW(inletNode.Temp, inletNode.HumRat));
        }
    }

    void SimUnitVentOAMixer(EnergyPlusData &state,
                            int const UnitVentNum,  // Unit index in unit ventilator array
                            HVAC::FanOp const fanOp // unit ventilator fan operating mode
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   May 2000

        // PURPOSE OF THIS SUBROUTINE:
        // This responsibility of this subroutine is to set the air flow rates
        // through the mixing box portion of the unit ventilator and then perform
        // an energy balance to arrive at outlet conditions which then would
        // serve as inlet conditions to the coils (or outlet conditions for
        // the device).  There is some question as to whether this needs to be
        // called every time the coils and fan are called since how the fans and
        // coil operate won't presumable change how the mixer operates.  The
        // method in which this routine is called is slightly cleaner though
        // from a code readability standpoint though less efficient.

        // METHODOLOGY EMPLOYED:
        // The OAMassFlowRate has already been calculated in the main control
        // algorithm.  Use this flow rate to establish all of the other flow
        // rates and perform an energy balance on the mixing of the return and
        // outdoor air streams.

        auto &unitVent = state.dataUnitVentilators->UnitVent(UnitVentNum);
        auto &airRelNode = state.dataLoopNodes->Node(unitVent.AirReliefNode);
        auto const &inletNode = state.dataLoopNodes->Node(unitVent.AirInNode);
        auto &OAMixOutNode = state.dataLoopNodes->Node(unitVent.OAMixerOutNode);
        auto &outsideAirNode = state.dataLoopNodes->Node(unitVent.OutsideAirNode);
        Real64 OutAirMassFlowRate = state.dataUnitVentilators->OAMassFlowRate;

        // Limit the outdoor air mass flow rate if cycling fan
        if (fanOp == HVAC::FanOp::Cycling) {
            OutAirMassFlowRate = min(state.dataUnitVentilators->OAMassFlowRate, inletNode.MassFlowRate);
        }

        // "Resolve" the air flow rates...
        outsideAirNode.MassFlowRate = OutAirMassFlowRate;
        outsideAirNode.MassFlowRateMinAvail = OutAirMassFlowRate;
        outsideAirNode.MassFlowRateMaxAvail = OutAirMassFlowRate;

        airRelNode.MassFlowRate = OutAirMassFlowRate;
        airRelNode.MassFlowRateMinAvail = OutAirMassFlowRate;
        airRelNode.MassFlowRateMaxAvail = OutAirMassFlowRate;

        OAMixOutNode.MassFlowRate = inletNode.MassFlowRate;
        OAMixOutNode.MassFlowRateMinAvail = inletNode.MassFlowRate;
        OAMixOutNode.MassFlowRateMaxAvail = inletNode.MassFlowRate;

        // "Inlet" conditions for InletNode and OutsideAirNode have already
        // been set elsewhere so we just need to set the "outlet" conditions
        airRelNode.Temp = inletNode.Temp;
        airRelNode.Press = inletNode.Press;
        airRelNode.HumRat = inletNode.HumRat;
        airRelNode.Enthalpy = inletNode.Enthalpy;

        Real64 OAFraction = 0.0; // Outside air fraction of inlet air
        if (inletNode.MassFlowRate > 0.0) {
            OAFraction = outsideAirNode.MassFlowRate / inletNode.MassFlowRate;
        }

        // Perform an energy and moisture mass balance on the mixing portion of the unit ventilator
        OAMixOutNode.Enthalpy = OAFraction * outsideAirNode.Enthalpy + (1.0 - OAFraction) * inletNode.Enthalpy;
        OAMixOutNode.HumRat = OAFraction * outsideAirNode.HumRat + (1.0 - OAFraction) * inletNode.HumRat;

        // Find the other key state points based on calculated conditions
        OAMixOutNode.Temp = Psychrometrics::PsyTdbFnHW(OAMixOutNode.Enthalpy, OAMixOutNode.HumRat);
        OAMixOutNode.Press = inletNode.Press;

        if (state.dataContaminantBalance->Contaminant.CO2Simulation) {
            airRelNode.CO2 = inletNode.CO2;
            OAMixOutNode.CO2 = OAFraction * outsideAirNode.CO2 + (1.0 - OAFraction) * inletNode.CO2;
        }
        if (state.dataContaminantBalance->Contaminant.GenericContamSimulation) {
            airRelNode.GenContam = inletNode.GenContam;
            OAMixOutNode.GenContam = OAFraction * outsideAirNode.GenContam + (1.0 - OAFraction) * inletNode.GenContam;
        }
    }

    void ReportUnitVentilator(EnergyPlusData &state, int const UnitVentNum) // Unit index in unit ventilator array
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   May 2000

        // Using/Aliasing
        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;
        auto &unitVent = state.dataUnitVentilators->UnitVent(UnitVentNum);

        unitVent.HeatEnergy = unitVent.HeatPower * TimeStepSysSec;
        unitVent.SensCoolEnergy = unitVent.SensCoolPower * TimeStepSysSec;
        unitVent.TotCoolEnergy = unitVent.TotCoolPower * TimeStepSysSec;
        unitVent.ElecEnergy = unitVent.ElecPower * TimeStepSysSec;

        if (unitVent.FirstPass) { // reset sizing flags so other zone equipment can size normally
            if (!state.dataGlobal->SysSizingCalc) {
                DataSizing::resetHVACSizingGlobals(state, state.dataSize->CurZoneEqNum, 0, unitVent.FirstPass);
            }
        }
    }

    int GetUnitVentilatorOutAirNode(EnergyPlusData &state, int const UnitVentNum)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   Dec  2006

        int GetUnitVentilatorOutAirNode;

        if (state.dataUnitVentilators->GetUnitVentilatorInputFlag) {
            GetUnitVentilatorInput(state);
            state.dataUnitVentilators->GetUnitVentilatorInputFlag = false;
        }

        GetUnitVentilatorOutAirNode = 0;
        if (UnitVentNum > 0 && UnitVentNum <= state.dataUnitVentilators->NumOfUnitVents) {
            GetUnitVentilatorOutAirNode = state.dataUnitVentilators->UnitVent(UnitVentNum).OutsideAirNode;
        }

        return GetUnitVentilatorOutAirNode;
    }

    int GetUnitVentilatorZoneInletAirNode(EnergyPlusData &state, int const UnitVentNum)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   Dec  2006

        int GetUnitVentilatorZoneInletAirNode;

        if (state.dataUnitVentilators->GetUnitVentilatorInputFlag) {
            GetUnitVentilatorInput(state);
            state.dataUnitVentilators->GetUnitVentilatorInputFlag = false;
        }

        GetUnitVentilatorZoneInletAirNode = 0;
        if (UnitVentNum > 0 && UnitVentNum <= state.dataUnitVentilators->NumOfUnitVents) {
            GetUnitVentilatorZoneInletAirNode = state.dataUnitVentilators->UnitVent(UnitVentNum).AirOutNode;
        }

        return GetUnitVentilatorZoneInletAirNode;
    }

    int GetUnitVentilatorMixedAirNode(EnergyPlusData &state, int const UnitVentNum)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   Dec  2006

        int GetUnitVentilatorMixedAirNode;

        if (state.dataUnitVentilators->GetUnitVentilatorInputFlag) {
            GetUnitVentilatorInput(state);
            state.dataUnitVentilators->GetUnitVentilatorInputFlag = false;
        }

        GetUnitVentilatorMixedAirNode = 0;
        if (UnitVentNum > 0 && UnitVentNum <= state.dataUnitVentilators->NumOfUnitVents) {
            GetUnitVentilatorMixedAirNode = state.dataUnitVentilators->UnitVent(UnitVentNum).OAMixerOutNode;
        }

        return GetUnitVentilatorMixedAirNode;
    }

    int GetUnitVentilatorReturnAirNode(EnergyPlusData &state, int const UnitVentNum)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   Dec  2006

        int GetUnitVentilatorReturnAirNode;

        if (state.dataUnitVentilators->GetUnitVentilatorInputFlag) {
            GetUnitVentilatorInput(state);
            state.dataUnitVentilators->GetUnitVentilatorInputFlag = false;
        }

        GetUnitVentilatorReturnAirNode = 0;
        if (UnitVentNum > 0 && UnitVentNum <= state.dataUnitVentilators->NumOfUnitVents) {
            GetUnitVentilatorReturnAirNode = state.dataUnitVentilators->UnitVent(UnitVentNum).AirInNode;
        }

        return GetUnitVentilatorReturnAirNode;
    }

    int getUnitVentilatorIndex(EnergyPlusData &state, std::string_view CompName)
    {
        if (state.dataUnitVentilators->GetUnitVentilatorInputFlag) {
            GetUnitVentilatorInput(state);
            state.dataUnitVentilators->GetUnitVentilatorInputFlag = false;
        }
        for (int UnitVentNum = 1; UnitVentNum <= state.dataUnitVentilators->NumOfUnitVents; ++UnitVentNum) {
            if (Util::SameString(state.dataUnitVentilators->UnitVent(UnitVentNum).Name, CompName)) {
                return UnitVentNum;
            }
        }

        return 0;
    }

    Real64 SetOAMassFlowRateForCoolingVariablePercent(EnergyPlusData &state,
                                                      int const UnitVentNum,     // Unit Ventilator index
                                                      Real64 const MinOAFrac,    // Minimum Outside Air Fraction
                                                      Real64 const MassFlowRate, // Design Outside Air Mass Flow Rate
                                                      Real64 const MaxOAFrac,    // Maximum Outside Air Fraction
                                                      Real64 const Tinlet,       // Inlet Temperature to Unit or Zone Temperature
                                                      Real64 const Toutdoor      // Outdoor Air Temperature
    )
    {

        Real64 ActualOAMassFlowRate = 0.0; // Result or return value

        if (Tinlet <= Toutdoor) {

            ActualOAMassFlowRate = MinOAFrac * MassFlowRate;

        } else { // Tinlet > Toutdoor
            // Use cooler outside air to provide "free" cooling without over-cooling.
            // First, use a simple load equals mass flow times Cp time Delta T equation to find OA Mass Flow Rate.
            // This must include the enthalpy difference across the fan.  Otherwise, this will potentially put a
            // small load on the cooling coil (if it exists) or will leave a small load that is not met when it could be.
            // Then, limit the OA Mass Flow Rate between the MinOA flow and the MaxOA flow.

            auto &unitVent = state.dataUnitVentilators->UnitVent(UnitVentNum);
            Real64 EnthDiffAcrossFan(0.0); // Temperature difference across the fan
            if (!unitVent.ATMixerExists) {
                EnthDiffAcrossFan =
                    state.dataLoopNodes->Node(unitVent.FanOutletNode).Enthalpy - state.dataLoopNodes->Node(unitVent.OAMixerOutNode).Enthalpy;
            } else {
                if (unitVent.ATMixerType == HVAC::MixerType::InletSide) {
                    EnthDiffAcrossFan =
                        state.dataLoopNodes->Node(unitVent.FanOutletNode).Enthalpy - state.dataLoopNodes->Node(unitVent.ATMixerOutNode).Enthalpy;
                }
                if (unitVent.ATMixerType == HVAC::MixerType::SupplySide) {
                    EnthDiffAcrossFan =
                        state.dataLoopNodes->Node(unitVent.FanOutletNode).Enthalpy - state.dataLoopNodes->Node(unitVent.AirInNode).Enthalpy;
                }
            }

            ActualOAMassFlowRate = (std::abs(state.dataUnitVentilators->QZnReq) + (MassFlowRate * std::abs(EnthDiffAcrossFan))) /
                                   (Psychrometrics::PsyCpAirFnW(state.dataEnvrn->OutHumRat) * (Tinlet - Toutdoor));

            ActualOAMassFlowRate = max(ActualOAMassFlowRate, (MinOAFrac * MassFlowRate));
            ActualOAMassFlowRate = min(ActualOAMassFlowRate, (MaxOAFrac * MassFlowRate));
        }

        return ActualOAMassFlowRate;
    }

    void CalcMdotCCoilCycFan(EnergyPlusData &state,
                             Real64 &mdot,              // mass flow rate
                             Real64 &QCoilReq,          // Remaining load to cooling coil
                             Real64 const QZnReq,       // Zone load to setpoint
                             int const UnitVentNum,     // Unit Ventilator index
                             Real64 const PartLoadRatio // Part load ratio for unit ventilator
    )
    {

        if (QZnReq >= 0.0) { // Heating requested so no cooling coil needed
            mdot = 0.0;
        } else { // Cooling so set first guess at flow rate
            mdot = state.dataUnitVentilators->UnitVent(UnitVentNum).MaxColdWaterFlow * PartLoadRatio;
        }

        // Check to see what outside air will do, "turn off" cooling coil if OA can handle the load
        int CCoilInAirNode = state.dataUnitVentilators->UnitVent(UnitVentNum).FanOutletNode;
        int AirInNode = state.dataUnitVentilators->UnitVent(UnitVentNum).AirInNode;
        Real64 CpAirZn = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(AirInNode).HumRat);
        QCoilReq = QZnReq - state.dataLoopNodes->Node(CCoilInAirNode).MassFlowRate * CpAirZn *
                                (state.dataLoopNodes->Node(CCoilInAirNode).Temp - state.dataLoopNodes->Node(AirInNode).Temp);
        if (QCoilReq > -HVAC::SmallLoad) {
            QCoilReq = 0.0;
            mdot = 0.0;
        }
    }

} // namespace UnitVentilator

} // namespace EnergyPlus

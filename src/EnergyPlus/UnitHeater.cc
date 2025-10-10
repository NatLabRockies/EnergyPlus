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
#include <EnergyPlus/Autosizing/HeatingAirFlowSizing.hh>
#include <EnergyPlus/Autosizing/HeatingCapacitySizing.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
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
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ReportCoilSelection.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SteamCoils.hh>
#include <EnergyPlus/UnitHeater.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterCoils.hh>

namespace EnergyPlus {

namespace UnitHeater {

    // Module containing the routines dealing with the Unit Heater

    // MODULE INFORMATION:
    //       AUTHOR         Rick Strand
    //       DATE WRITTEN   May 2000
    //       MODIFIED       Brent Griffith, Sept 2010, plant upgrades, fluid properties
    //       MODIFIED       Bereket Nigusse, FSEC, October 2013, Added cycling fan operating mode

    // PURPOSE OF THIS MODULE:
    // To simulate unit heaters.  It is assumed that unit heaters are zone equipment
    // without any connection to outside air other than through a separately defined
    // air loop.

    // METHODOLOGY EMPLOYED:
    // Units are modeled as a collection of a fan and a heating coil.  The fan
    // can either be a continuously running fan or an on-off fan which turns on
    // only when there is actually a heating load.  This fan control works together
    // with the unit operation schedule to determine what the unit heater actually
    // does at a given point in time.

    // REFERENCES:
    // ASHRAE Systems and Equipment Handbook (SI), 1996. pp. 31.3-31.8
    // Rick Strand's unit heater module which was based upon Fred Buhl's fan coil
    // module (FanCoilUnits.cc)

    void SimUnitHeater(EnergyPlusData &state,
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
        // This is the main driver subroutine for the Unit Heater simulation.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int UnitHeatNum; // index of unit heater being simulated

        if (state.dataUnitHeaters->GetUnitHeaterInputFlag) {
            GetUnitHeaterInput(state);
            state.dataUnitHeaters->GetUnitHeaterInputFlag = false;
        }

        // Find the correct Unit Heater Equipment
        if (CompIndex == 0) {
            UnitHeatNum = Util::FindItemInList(CompName, state.dataUnitHeaters->UnitHeat);
            if (UnitHeatNum == 0) {
                ShowFatalError(state, format("SimUnitHeater: Unit not found={}", CompName));
            }
            CompIndex = UnitHeatNum;
        } else {
            UnitHeatNum = CompIndex;
            if (UnitHeatNum > state.dataUnitHeaters->NumOfUnitHeats || UnitHeatNum < 1) {
                ShowFatalError(state,
                               format("SimUnitHeater:  Invalid CompIndex passed={}, Number of Units={}, Entered Unit name={}",
                                      UnitHeatNum,
                                      state.dataUnitHeaters->NumOfUnitHeats,
                                      CompName));
            }
            if (state.dataUnitHeaters->CheckEquipName(UnitHeatNum)) {
                if (CompName != state.dataUnitHeaters->UnitHeat(UnitHeatNum).Name) {
                    ShowFatalError(state,
                                   format("SimUnitHeater: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                          UnitHeatNum,
                                          CompName,
                                          state.dataUnitHeaters->UnitHeat(UnitHeatNum).Name));
                }
                state.dataUnitHeaters->CheckEquipName(UnitHeatNum) = false;
            }
        }

        state.dataSize->ZoneEqUnitHeater = true;

        InitUnitHeater(state, UnitHeatNum, ZoneNum, FirstHVACIteration);

        state.dataSize->ZoneHeatingOnlyFan = true;

        CalcUnitHeater(state, UnitHeatNum, ZoneNum, FirstHVACIteration, PowerMet, LatOutputProvided);

        state.dataSize->ZoneHeatingOnlyFan = false;

        //  CALL UpdateUnitHeater

        ReportUnitHeater(state, UnitHeatNum);

        state.dataSize->ZoneEqUnitHeater = false;
    }

    void GetUnitHeaterInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   May 2000
        //       MODIFIED       Chandan Sharma, FSEC, March 2011: Added ZoneHVAC sys avail manager
        //                      Bereket Nigusse, FSEC, April 2011: eliminated input node names
        //                                                         & added fan object type

        // PURPOSE OF THIS SUBROUTINE:
        // Obtain the user input data for all of the unit heaters in the input file.

        // METHODOLOGY EMPLOYED:
        // Standard EnergyPlus methodology.

        // REFERENCES:
        // Fred Buhl's fan coil module (FanCoilUnits.cc)

        static constexpr std::string_view RoutineName("GetUnitHeaterInput: "); // include trailing blank space
        static constexpr std::string_view routineName = "GetUnitHeaterInput";  // include trailing blank space

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        bool ErrorsFound(false); // Set to true if errors in input, fatal at end of routine
        int IOStatus;            // Used in GetObjectItem
        bool IsNotOK;            // TRUE if there was a problem with a list name
        bool errFlag(false);     // interim error flag
        int NumAlphas;           // Number of Alphas for each GetObjectItem call
        int NumNumbers;          // Number of Numbers for each GetObjectItem call
        int NumFields;           // Total number of fields in object

        Real64 FanVolFlow;             // Fan volumetric flow rate
        Array1D_string Alphas;         // Alpha items for object
        Array1D<Real64> Numbers;       // Numeric items for object
        Array1D_string cAlphaFields;   // Alpha field names
        Array1D_string cNumericFields; // Numeric field names
        Array1D_bool lAlphaBlanks;     // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;   // Logical array, numeric field input BLANK = .TRUE.
        int CtrlZone;                  // index to loop counter
        int NodeNum;                   // index to loop counter

        auto &s_node = state.dataLoopNodes;
        
        // Figure out how many unit heaters there are in the input file
        std::string CurrentModuleObject = state.dataUnitHeaters->cMO_UnitHeater;
        state.dataUnitHeaters->NumOfUnitHeats = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, CurrentModuleObject, NumFields, NumAlphas, NumNumbers);

        Alphas.allocate(NumAlphas);
        Numbers.dimension(NumNumbers, 0.0);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNumbers);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNumbers, true);

        // Allocate the local derived type and do one-time initializations for all parts of it
        if (state.dataUnitHeaters->NumOfUnitHeats > 0) {
            state.dataUnitHeaters->UnitHeat.allocate(state.dataUnitHeaters->NumOfUnitHeats);
            state.dataUnitHeaters->CheckEquipName.allocate(state.dataUnitHeaters->NumOfUnitHeats);
            state.dataUnitHeaters->UnitHeatNumericFields.allocate(state.dataUnitHeaters->NumOfUnitHeats);
        }
        state.dataUnitHeaters->CheckEquipName = true;

        for (int UnitHeatNum = 1; UnitHeatNum <= state.dataUnitHeaters->NumOfUnitHeats;
             ++UnitHeatNum) { // Begin looping over all of the unit heaters found in the input file...

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     UnitHeatNum,
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

            state.dataUnitHeaters->UnitHeatNumericFields(UnitHeatNum).FieldNames.allocate(NumNumbers);
            state.dataUnitHeaters->UnitHeatNumericFields(UnitHeatNum).FieldNames = "";
            state.dataUnitHeaters->UnitHeatNumericFields(UnitHeatNum).FieldNames = cNumericFields;
            Util::IsNameEmpty(state, Alphas(1), CurrentModuleObject, ErrorsFound);

            state.dataUnitHeaters->UnitHeat(UnitHeatNum).Name = Alphas(1);

            if (lAlphaBlanks(2)) {
                state.dataUnitHeaters->UnitHeat(UnitHeatNum).availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((state.dataUnitHeaters->UnitHeat(UnitHeatNum).availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ErrorsFound = true;
            }

            // Main air nodes (except outside air node):
            state.dataUnitHeaters->UnitHeat(UnitHeatNum).AirInNode =
                NodeInputManager::GetOnlySingleNode(state,
                                                    Alphas(3),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::ZoneHVACUnitHeater,
                                                    Alphas(1),
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Inlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);

            state.dataUnitHeaters->UnitHeat(UnitHeatNum).AirOutNode =
                NodeInputManager::GetOnlySingleNode(state,
                                                    Alphas(4),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::ZoneHVACUnitHeater,
                                                    Alphas(1),
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Outlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);

            auto &unitHeat = state.dataUnitHeaters->UnitHeat(UnitHeatNum);
            // Fan information:
            unitHeat.fanType = static_cast<HVAC::FanType>(getEnumValue(HVAC::fanTypeNamesUC, Alphas(5)));
            if (unitHeat.fanType != HVAC::FanType::Constant && unitHeat.fanType != HVAC::FanType::VAV && unitHeat.fanType != HVAC::FanType::OnOff &&
                unitHeat.fanType != HVAC::FanType::SystemModel) {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(5), Alphas(5), "Fan Type must be Fan:ConstantVolume, Fan:VariableVolume, or Fan:OnOff");
                ErrorsFound = true;
            }

            unitHeat.FanName = Alphas(6);
            unitHeat.MaxAirVolFlow = Numbers(1);

            if ((unitHeat.Fan_Index = Fans::GetFanIndex(state, unitHeat.FanName)) == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(6), unitHeat.FanName);
                ErrorsFound = true;

            } else {
                auto *fan = state.dataFans->fans(unitHeat.Fan_Index);

                unitHeat.FanOutletNode = fan->outletNodeNum;

                FanVolFlow = fan->maxAirFlowRate;

                if (FanVolFlow != DataSizing::AutoSize && unitHeat.MaxAirVolFlow != DataSizing::AutoSize && FanVolFlow < unitHeat.MaxAirVolFlow) {
                    ShowSevereError(state, format("Specified in {} = {}", CurrentModuleObject, unitHeat.Name));
                    ShowContinueError(
                        state,
                        format("...air flow rate ({:.7T}) in fan object {} is less than the unit heater maximum supply air flow rate ({:.7T}).",
                               FanVolFlow,
                               unitHeat.FanName,
                               unitHeat.MaxAirVolFlow));
                    ShowContinueError(state, "...the fan flow rate must be greater than or equal to the unit heater maximum supply air flow rate.");
                    ErrorsFound = true;
                } else if (FanVolFlow == DataSizing::AutoSize && unitHeat.MaxAirVolFlow != DataSizing::AutoSize) {
                    ShowWarningError(state, format("Specified in {} = {}", CurrentModuleObject, unitHeat.Name));
                    ShowContinueError(state, "...the fan flow rate is autosized while the unit heater flow rate is not.");
                    ShowContinueError(state, "...this can lead to unexpected results where the fan flow rate is less than required.");
                } else if (FanVolFlow != DataSizing::AutoSize && unitHeat.MaxAirVolFlow == DataSizing::AutoSize) {
                    ShowWarningError(state, format("Specified in {} = {}", CurrentModuleObject, unitHeat.Name));
                    ShowContinueError(state, "...the unit heater flow rate is autosized while the fan flow rate is not.");
                    ShowContinueError(state, "...this can lead to unexpected results where the fan flow rate is less than required.");
                }
                unitHeat.fanAvailSched = fan->availSched;
            }

            // Heating coil information:
            unitHeat.heatCoilType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Alphas(7)));
            switch (unitHeat.heatCoilType) {
            case HVAC::CoilType::HeatingWater: {
                unitHeat.HeatCoilPlantType = DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
            } break;

            case HVAC::CoilType::HeatingSteam: {
                unitHeat.HeatCoilPlantType = DataPlant::PlantEquipmentType::CoilSteamAirHeating;
            } break;
                  
            case HVAC::CoilType::HeatingElectric:
            case HVAC::CoilType::HeatingGasOrOtherFuel: {
            } break;
                  
            default: {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(7), Alphas(7));
                ErrorsFound = true;
                errFlag = true;
            } break;
            }
            
            if (!errFlag) {
                unitHeat.HeatCoilName = Alphas(8);
                if (unitHeat.heatCoilType == HVAC::CoilType::HeatingWater) {
                    unitHeat.HeatCoilNum = WaterCoils::GetCoilIndex(state, unitHeat.HeatCoilName);
                    if (unitHeat.HeatCoilNum == 0) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFields(8), unitHeat.HeatCoilName);
                        ErrorsFound = true;
                    } else {
                        unitHeat.HotControlNode = WaterCoils::GetCoilWaterInletNode(state, unitHeat.HeatCoilNum);
                    }
                    
                } else if (unitHeat.heatCoilType == HVAC::CoilType::HeatingSteam) {
                    unitHeat.HeatCoilNum = SteamCoils::GetCoilIndex(state, unitHeat.HeatCoilName);
                    if (unitHeat.HeatCoilNum == 0) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFields(8), unitHeat.HeatCoilName);
                        ErrorsFound = true;
                    } else {
                        unitHeat.HotControlNode = SteamCoils::GetCoilSteamInletNode(state, unitHeat.HeatCoilNum);
                        unitHeat.HeatCoilFluid = Fluid::GetSteam(state);
                    }

                } else if (unitHeat.heatCoilType == HVAC::CoilType::HeatingElectric ||
                           unitHeat.heatCoilType == HVAC::CoilType::HeatingGasOrOtherFuel) {
                    unitHeat.HeatCoilNum = HeatingCoils::GetCoilIndex(state, unitHeat.HeatCoilName);
                    if (unitHeat.HeatCoilNum == 0) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFields(8), unitHeat.HeatCoilName);
                        ErrorsFound = true;
                    }
                }
            }

            if (lAlphaBlanks(9)) {
                unitHeat.fanOp = (unitHeat.fanType == HVAC::FanType::OnOff || unitHeat.fanType == HVAC::FanType::SystemModel)
                                     ? HVAC::FanOp::Cycling
                                     : HVAC::FanOp::Continuous;
            } else if ((state.dataUnitHeaters->UnitHeat(UnitHeatNum).fanOpModeSched = Sched::GetSchedule(state, Alphas(9))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(9), Alphas(9));
                ErrorsFound = true;
            } else if (state.dataUnitHeaters->UnitHeat(UnitHeatNum).fanType == HVAC::FanType::Constant &&
                       !state.dataUnitHeaters->UnitHeat(UnitHeatNum).fanOpModeSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 1.0)) {
                Sched::ShowSevereBadMinMax(state, eoh, cAlphaFields(9), Alphas(9), Clusive::In, 0.0, Clusive::In, 1.0);
                ErrorsFound = true;
            }

            unitHeat.FanOperatesDuringNoHeating = Alphas(10);
            if ((!Util::SameString(unitHeat.FanOperatesDuringNoHeating, "Yes")) && (!Util::SameString(unitHeat.FanOperatesDuringNoHeating, "No"))) {
                ErrorsFound = true;
                ShowSevereError(state, format("Illegal {} = {}", cAlphaFields(10), Alphas(10)));
                ShowContinueError(state, format("Occurs in {}={}", CurrentModuleObject, unitHeat.Name));
            } else if (Util::SameString(unitHeat.FanOperatesDuringNoHeating, "No")) {
                unitHeat.FanOffNoHeating = true;
            }

            unitHeat.MaxVolHotWaterFlow = Numbers(2);
            unitHeat.MinVolHotWaterFlow = Numbers(3);
            unitHeat.MaxVolHotSteamFlow = Numbers(2);
            unitHeat.MinVolHotSteamFlow = Numbers(3);

            unitHeat.HotControlOffset = Numbers(4);
            // Set default convergence tolerance
            if (unitHeat.HotControlOffset <= 0.0) {
                unitHeat.HotControlOffset = 0.001;
            }

            if (!lAlphaBlanks(11)) {
                unitHeat.AvailManagerListName = Alphas(11);
            }

            unitHeat.HVACSizingIndex = 0;
            if (!lAlphaBlanks(12)) {
                unitHeat.HVACSizingIndex = Util::FindItemInList(Alphas(12), state.dataSize->ZoneHVACSizing);
                if (unitHeat.HVACSizingIndex == 0) {
                    ShowSevereError(state, format("{} = {} not found.", cAlphaFields(12), Alphas(12)));
                    ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, unitHeat.Name));
                    ErrorsFound = true;
                }
            }

            // check that unit heater air inlet node must be the same as a zone exhaust node
            bool ZoneNodeNotFound = true;
            for (CtrlZone = 1; CtrlZone <= state.dataGlobal->NumOfZones; ++CtrlZone) {
                if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZone).IsControlled) {
                    continue;
                }
                for (NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(CtrlZone).NumExhaustNodes; ++NodeNum) {
                    if (unitHeat.AirInNode == state.dataZoneEquip->ZoneEquipConfig(CtrlZone).ExhaustNode(NodeNum)) {
                        ZoneNodeNotFound = false;
                        break;
                    }
                }
            }
            if (ZoneNodeNotFound) {
                ShowSevereError(state,
                                format("{} = \"{}\". Unit heater air inlet node name must be the same as a zone exhaust node name.",
                                       CurrentModuleObject,
                                       unitHeat.Name));
                ShowContinueError(state, "..Zone exhaust node name is specified in ZoneHVAC:EquipmentConnections object.");
                ShowContinueError(state, format("..Unit heater air inlet node name = {}", s_node->NodeID(unitHeat.AirInNode)));
                ErrorsFound = true;
            }
            // check that unit heater air outlet node is a zone inlet node.
            ZoneNodeNotFound = true;
            for (CtrlZone = 1; CtrlZone <= state.dataGlobal->NumOfZones; ++CtrlZone) {
                if (!state.dataZoneEquip->ZoneEquipConfig(CtrlZone).IsControlled) {
                    continue;
                }
                for (NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(CtrlZone).NumInletNodes; ++NodeNum) {
                    if (unitHeat.AirOutNode == state.dataZoneEquip->ZoneEquipConfig(CtrlZone).InletNode(NodeNum)) {
                        unitHeat.ZonePtr = CtrlZone;
                        ZoneNodeNotFound = false;
                        break;
                    }
                }
            }
            if (ZoneNodeNotFound) {
                ShowSevereError(state,
                                format("{} = \"{}\". Unit heater air outlet node name must be the same as a zone inlet node name.",
                                       CurrentModuleObject,
                                       unitHeat.Name));
                ShowContinueError(state, "..Zone inlet node name is specified in ZoneHVAC:EquipmentConnections object.");
                ShowContinueError(state, format("..Unit heater air outlet node name = {}", s_node->NodeID(unitHeat.AirOutNode)));
                ErrorsFound = true;
            }

            // Add fan to component sets array
            BranchNodeConnections::SetUpCompSets(state,
                                                 CurrentModuleObject,
                                                 unitHeat.Name,
                                                 HVAC::fanTypeNamesUC[(int)unitHeat.fanType],
                                                 unitHeat.FanName,
                                                 s_node->NodeID(unitHeat.AirInNode),
                                                 s_node->NodeID(unitHeat.FanOutletNode));

            // Add heating coil to component sets array
            BranchNodeConnections::SetUpCompSets(state,
                                                 CurrentModuleObject,
                                                 unitHeat.Name,
                                                 HVAC::coilTypeNames[(int)unitHeat.heatCoilType],
                                                 unitHeat.HeatCoilName,
                                                 s_node->NodeID(unitHeat.FanOutletNode),
                                                 s_node->NodeID(unitHeat.AirOutNode));

        } // ...loop over all of the unit heaters found in the input file

        Alphas.deallocate();
        Numbers.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        if (ErrorsFound) {
            ShowFatalError(state, format("{}Errors found in input", RoutineName));
        }

        // Setup Report variables for the Unit Heaters, CurrentModuleObject='ZoneHVAC:UnitHeater'
        for (int UnitHeatNum = 1; UnitHeatNum <= state.dataUnitHeaters->NumOfUnitHeats; ++UnitHeatNum) {
            auto &unitHeat = state.dataUnitHeaters->UnitHeat(UnitHeatNum);
            SetupOutputVariable(state,
                                "Zone Unit Heater Heating Rate",
                                Constant::Units::W,
                                unitHeat.HeatPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                unitHeat.Name);
            SetupOutputVariable(state,
                                "Zone Unit Heater Heating Energy",
                                Constant::Units::J,
                                unitHeat.HeatEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                unitHeat.Name);
            SetupOutputVariable(state,
                                "Zone Unit Heater Fan Electricity Rate",
                                Constant::Units::W,
                                unitHeat.ElecPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                unitHeat.Name);
            // Note that the unit heater fan electric is NOT metered because this value is already metered through the fan component
            SetupOutputVariable(state,
                                "Zone Unit Heater Fan Electricity Energy",
                                Constant::Units::J,
                                unitHeat.ElecEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                unitHeat.Name);
            SetupOutputVariable(state,
                                "Zone Unit Heater Fan Availability Status",
                                Constant::Units::None,
                                (int &)unitHeat.availStatus,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                unitHeat.Name);
            if (unitHeat.fanType == HVAC::FanType::OnOff) {
                SetupOutputVariable(state,
                                    "Zone Unit Heater Fan Part Load Ratio",
                                    Constant::Units::None,
                                    unitHeat.FanPartLoadRatio,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    unitHeat.Name);
            }
            ReportCoilSelection::setCoilSupplyFanInfo(
                state, unitHeat.HeatCoilName, unitHeat.heatCoilType, unitHeat.FanName, unitHeat.fanType, unitHeat.Fan_Index);
        }
    }

    void InitUnitHeater(EnergyPlusData &state,
                        int const UnitHeatNum,                         // index for the current unit heater
                        int const ZoneNum,                             // number of zone being served
                        [[maybe_unused]] bool const FirstHVACIteration // TRUE if 1st HVAC simulation of system timestep
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   May 2000
        //       MODIFIED       Chandan Sharma, FSEC, March 2011: Added ZoneHVAC sys avail manager

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine initializes all of the data elements which are necessary
        // to simulate a unit heater.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("InitUnitHeater");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int InNode;    // inlet node number in unit heater loop
        int OutNode;   // outlet node number in unit heater loop
        Real64 RhoAir; // air density at InNode
        Real64 TempSteamIn;
        Real64 SteamDensity;
        Real64 rho; // local fluid density

        auto &s_node = state.dataLoopNodes;
        
        auto &unitHeat = state.dataUnitHeaters->UnitHeat(UnitHeatNum);
        // Do the one time initializations
        if (state.dataUnitHeaters->InitUnitHeaterOneTimeFlag) {

            state.dataUnitHeaters->MyEnvrnFlag.allocate(state.dataUnitHeaters->NumOfUnitHeats);
            state.dataUnitHeaters->MySizeFlag.allocate(state.dataUnitHeaters->NumOfUnitHeats);
            state.dataUnitHeaters->MyPlantScanFlag.allocate(state.dataUnitHeaters->NumOfUnitHeats);
            state.dataUnitHeaters->MyZoneEqFlag.allocate(state.dataUnitHeaters->NumOfUnitHeats);
            state.dataUnitHeaters->MyEnvrnFlag = true;
            state.dataUnitHeaters->MySizeFlag = true;
            state.dataUnitHeaters->MyPlantScanFlag = true;
            state.dataUnitHeaters->MyZoneEqFlag = true;
            state.dataUnitHeaters->InitUnitHeaterOneTimeFlag = false;
        }

        if (allocated(state.dataAvail->ZoneComp)) {
            auto &availMgr = state.dataAvail->ZoneComp(DataZoneEquipment::ZoneEquipType::UnitHeater).ZoneCompAvailMgrs(UnitHeatNum);
            if (state.dataUnitHeaters->MyZoneEqFlag(UnitHeatNum)) { // initialize the name of each availability manager list and zone number
                availMgr.AvailManagerListName = unitHeat.AvailManagerListName;
                availMgr.ZoneNum = ZoneNum;
                state.dataUnitHeaters->MyZoneEqFlag(UnitHeatNum) = false;
            }
            unitHeat.availStatus = availMgr.availStatus;
        }

        if (state.dataUnitHeaters->MyPlantScanFlag(UnitHeatNum) && allocated(state.dataPlnt->PlantLoop)) {
            if ((unitHeat.HeatCoilPlantType == DataPlant::PlantEquipmentType::CoilWaterSimpleHeating) ||
                (unitHeat.HeatCoilPlantType == DataPlant::PlantEquipmentType::CoilSteamAirHeating)) {
                bool errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(state,
                                                        unitHeat.HeatCoilName,
                                                        unitHeat.HeatCoilPlantType,
                                                        unitHeat.HWplantLoc,
                                                        errFlag,
                                                        _,
                                                        _,
                                                        _,
                                                        _,
                                                        _);
                if (errFlag) {
                    ShowContinueError(state,
                                      format("Reference Unit=\"{}\", type=ZoneHVAC:UnitHeater", unitHeat.Name));
                    ShowFatalError(state, "InitUnitHeater: Program terminated due to previous condition(s).");
                }

                unitHeat.HotCoilOutNodeNum =
                    DataPlant::CompData::getPlantComponent(state, unitHeat.HWplantLoc).NodeNumOut;
            }
            state.dataUnitHeaters->MyPlantScanFlag(UnitHeatNum) = false;
        } else if (state.dataUnitHeaters->MyPlantScanFlag(UnitHeatNum) && !state.dataGlobal->AnyPlantInModel) {
            state.dataUnitHeaters->MyPlantScanFlag(UnitHeatNum) = false;
        }
        // need to check all units to see if they are on Zone Equipment List or issue warning
        if (!state.dataUnitHeaters->ZoneEquipmentListChecked && state.dataZoneEquip->ZoneEquipInputsFilled) {
            state.dataUnitHeaters->ZoneEquipmentListChecked = true;
            for (int Loop = 1; Loop <= state.dataUnitHeaters->NumOfUnitHeats; ++Loop) {
                if (DataZoneEquipment::CheckZoneEquipmentList(state, "ZoneHVAC:UnitHeater", state.dataUnitHeaters->UnitHeat(Loop).Name)) {
                    continue;
                }
                ShowSevereError(state,
                                format("InitUnitHeater: Unit=[UNIT HEATER,{}] is not on any ZoneHVAC:EquipmentList.  It will not be simulated.",
                                       state.dataUnitHeaters->UnitHeat(Loop).Name));
            }
        }

        if (!state.dataGlobal->SysSizingCalc && state.dataUnitHeaters->MySizeFlag(UnitHeatNum) &&
            !state.dataUnitHeaters->MyPlantScanFlag(UnitHeatNum)) {

            SizeUnitHeater(state, UnitHeatNum);

            state.dataUnitHeaters->MySizeFlag(UnitHeatNum) = false;
        } // Do the one time initializations

        if (state.dataGlobal->BeginEnvrnFlag && state.dataUnitHeaters->MyEnvrnFlag(UnitHeatNum) &&
            !state.dataUnitHeaters->MyPlantScanFlag(UnitHeatNum)) {
            InNode = unitHeat.AirInNode;
            OutNode = unitHeat.AirOutNode;
            RhoAir = state.dataEnvrn->StdRhoAir;

            // set the mass flow rates from the input volume flow rates
            unitHeat.MaxAirMassFlow = RhoAir * unitHeat.MaxAirVolFlow;

            // set the node max and min mass flow rates
            s_node->Node(OutNode).MassFlowRateMax = unitHeat.MaxAirMassFlow;
            s_node->Node(OutNode).MassFlowRateMin = 0.0;

            s_node->Node(InNode).MassFlowRateMax = unitHeat.MaxAirMassFlow;
            s_node->Node(InNode).MassFlowRateMin = 0.0;

            if (unitHeat.heatCoilType == HVAC::CoilType::HeatingWater) {
                rho = state.dataPlnt->PlantLoop(unitHeat.HWplantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);

                unitHeat.MaxHotWaterFlow = rho * unitHeat.MaxVolHotWaterFlow;
                unitHeat.MinHotWaterFlow = rho * unitHeat.MinVolHotWaterFlow;
                PlantUtilities::InitComponentNodes(state,
                                                   unitHeat.MinHotWaterFlow,
                                                   unitHeat.MaxHotWaterFlow,
                                                   unitHeat.HotControlNode,
                                                   unitHeat.HotCoilOutNodeNum);
            }
            if (unitHeat.heatCoilType == HVAC::CoilType::HeatingSteam) {
                TempSteamIn = 100.00;
                SteamDensity = unitHeat.HeatCoilFluid->getSatDensity(state, TempSteamIn, 1.0, RoutineName);
                unitHeat.MaxHotSteamFlow = SteamDensity * unitHeat.MaxVolHotSteamFlow;
                unitHeat.MinHotSteamFlow = SteamDensity * unitHeat.MinVolHotSteamFlow;

                PlantUtilities::InitComponentNodes(state,
                                                   unitHeat.MinHotSteamFlow,
                                                   unitHeat.MaxHotSteamFlow,
                                                   unitHeat.HotControlNode,
                                                   unitHeat.HotCoilOutNodeNum);
            }

            state.dataUnitHeaters->MyEnvrnFlag(UnitHeatNum) = false;
        } // ...end start of environment inits

        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataUnitHeaters->MyEnvrnFlag(UnitHeatNum) = true;
        }

        // These initializations are done every iteration...
        InNode = unitHeat.AirInNode;
        OutNode = unitHeat.AirOutNode;

        state.dataUnitHeaters->QZnReq = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToHeatSP; // zone load needed
        if (unitHeat.fanOpModeSched != nullptr) {
            if (unitHeat.fanOpModeSched->getCurrentVal() == 0.0 &&
                unitHeat.fanType == HVAC::FanType::OnOff) {
                unitHeat.fanOp = HVAC::FanOp::Cycling;
            } else {
                unitHeat.fanOp = HVAC::FanOp::Continuous;
            }
            if ((state.dataUnitHeaters->QZnReq < HVAC::SmallLoad) || state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) {
                // Unit is available, but there is no load on it or we are in setback/deadband
                if (!unitHeat.FanOffNoHeating && unitHeat.fanOpModeSched->getCurrentVal() > 0.0) {
                    unitHeat.fanOp = HVAC::FanOp::Continuous;
                }
            }
        }

        state.dataUnitHeaters->SetMassFlowRateToZero = false;
        if (unitHeat.availSched->getCurrentVal() > 0) {
            if ((unitHeat.fanAvailSched->getCurrentVal() > 0 || state.dataHVACGlobal->TurnFansOn) &&
                !state.dataHVACGlobal->TurnFansOff) {
                if (unitHeat.FanOffNoHeating &&
                    ((state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToHeatSP < HVAC::SmallLoad) ||
                     (state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)))) {
                    state.dataUnitHeaters->SetMassFlowRateToZero = true;
                }
            } else {
                state.dataUnitHeaters->SetMassFlowRateToZero = true;
            }
        } else {
            state.dataUnitHeaters->SetMassFlowRateToZero = true;
        }

        if (state.dataUnitHeaters->SetMassFlowRateToZero) {
            s_node->Node(InNode).MassFlowRate = 0.0;
            s_node->Node(InNode).MassFlowRateMaxAvail = 0.0;
            s_node->Node(InNode).MassFlowRateMinAvail = 0.0;
            s_node->Node(OutNode).MassFlowRate = 0.0;
            s_node->Node(OutNode).MassFlowRateMaxAvail = 0.0;
            s_node->Node(OutNode).MassFlowRateMinAvail = 0.0;
        } else {
            s_node->Node(InNode).MassFlowRate = unitHeat.MaxAirMassFlow;
            s_node->Node(InNode).MassFlowRateMaxAvail = unitHeat.MaxAirMassFlow;
            s_node->Node(InNode).MassFlowRateMinAvail = unitHeat.MaxAirMassFlow;
            s_node->Node(OutNode).MassFlowRate = unitHeat.MaxAirMassFlow;
            s_node->Node(OutNode).MassFlowRateMaxAvail = unitHeat.MaxAirMassFlow;
            s_node->Node(OutNode).MassFlowRateMinAvail = unitHeat.MaxAirMassFlow;
        }

        // Just in case the unit is off and conditions do not get sent through
        // the unit for some reason, set the outlet conditions equal to the inlet
        // conditions of the unit heater
        s_node->Node(OutNode).Temp = s_node->Node(InNode).Temp;
        s_node->Node(OutNode).Press = s_node->Node(InNode).Press;
        s_node->Node(OutNode).HumRat = s_node->Node(InNode).HumRat;
        s_node->Node(OutNode).Enthalpy = s_node->Node(InNode).Enthalpy;
    }

    void SizeUnitHeater(EnergyPlusData &state, int const UnitHeatNum)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   February 2002
        //       MODIFIED       August 2013 Daeho Kang, add component sizing table entries
        //                      July 2014, B. Nigusse, added scalable sizing
        //       RE-ENGINEERED  na

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing Unit Heater components for which flow rates have not been
        // specified in the input.

        // METHODOLOGY EMPLOYED:
        // Obtains flow rates from the zone sizing arrays and plant sizing data.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("SizeUnitHeater");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int PltSizHeatNum; // index of plant sizing object for 1st heating loop
        Real64 DesCoilLoad;
        Real64 TempSteamIn;
        Real64 EnthSteamInDry;
        Real64 EnthSteamOutWet;
        Real64 LatentHeatSteam;
        Real64 SteamDensity;
        Real64 Cp;                 // local temporary for fluid specific heat
        Real64 rho;                // local temporary for fluid density
        std::string SizingString;  // input field sizing description (e.g., Nominal Capacity)
        Real64 TempSize;           // autosized value of coil input field
        bool PrintFlag;            // TRUE when sizing information is reported in the eio file
        int zoneHVACIndex;         // index of zoneHVAC equipment sizing specification
        Real64 WaterCoilSizDeltaT; // water coil deltaT for design water flow rate autosizing

        int &CurZoneEqNum = state.dataSize->CurZoneEqNum;

        bool ErrorsFound = false;
        Real64 MaxAirVolFlowDes = 0.0;
        Real64 MaxAirVolFlowUser = 0.0;
        Real64 MaxVolHotWaterFlowDes = 0.0;
        Real64 MaxVolHotWaterFlowUser = 0.0;
        Real64 MaxVolHotSteamFlowDes = 0.0;
        Real64 MaxVolHotSteamFlowUser = 0.0;

        auto &unitHeat = state.dataUnitHeaters->UnitHeat(UnitHeatNum);
        
        state.dataSize->DataScalableSizingON = false;
        state.dataSize->DataScalableCapSizingON = false;
        state.dataSize->ZoneHeatingOnlyFan = true;
        std::string CompType = "ZoneHVAC:UnitHeater";
        std::string const &CompName = unitHeat.Name;
        state.dataSize->DataZoneNumber = unitHeat.ZonePtr;
        state.dataSize->DataFanType = unitHeat.fanType;
        state.dataSize->DataFanIndex = unitHeat.Fan_Index;
        // unit heater is always blow thru
        state.dataSize->DataFanPlacement = HVAC::FanPlace::BlowThru;

        if (CurZoneEqNum > 0) {
            auto &ZoneEqSizing = state.dataSize->ZoneEqSizing(CurZoneEqNum);
            if (unitHeat.HVACSizingIndex > 0) {
                zoneHVACIndex = unitHeat.HVACSizingIndex;
                int SizingMethod = HVAC::HeatingAirflowSizing;
                int FieldNum = 1; //  N1 , \field Maximum Supply Air Flow Rate
                PrintFlag = true;
                SizingString = state.dataUnitHeaters->UnitHeatNumericFields(UnitHeatNum).FieldNames(FieldNum) + " [m3/s]";
                int SAFMethod = state.dataSize->ZoneHVACSizing(zoneHVACIndex).HeatingSAFMethod;
                ZoneEqSizing.SizingMethod(SizingMethod) = SAFMethod;
                if (SAFMethod == DataSizing::None || SAFMethod == DataSizing::SupplyAirFlowRate || SAFMethod == DataSizing::FlowPerFloorArea ||
                    SAFMethod == DataSizing::FractionOfAutosizedHeatingAirflow) {
                    if (SAFMethod == DataSizing::SupplyAirFlowRate) {
                        if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow > 0.0) {
                            ZoneEqSizing.AirVolFlow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                            ZoneEqSizing.SystemAirFlow = true;
                        }
                        TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                    } else if (SAFMethod == DataSizing::FlowPerFloorArea) {
                        ZoneEqSizing.SystemAirFlow = true;
                        ZoneEqSizing.AirVolFlow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow *
                                                  state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                        TempSize = ZoneEqSizing.AirVolFlow;
                        state.dataSize->DataScalableSizingON = true;
                    } else if (SAFMethod == DataSizing::FractionOfAutosizedHeatingAirflow) {
                        state.dataSize->DataFracOfAutosizedCoolingAirflow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                        TempSize = DataSizing::AutoSize;
                        state.dataSize->DataScalableSizingON = true;
                    } else {
                        TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                    }
                    bool errorsFound = false;
                    HeatingAirFlowSizer sizingHeatingAirFlow;
                    sizingHeatingAirFlow.overrideSizingString(SizingString);
                    // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                    sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                    unitHeat.MaxAirVolFlow = sizingHeatingAirFlow.size(state, TempSize, errorsFound);

                } else if (SAFMethod == DataSizing::FlowPerHeatingCapacity) {
                    TempSize = DataSizing::AutoSize;
                    PrintFlag = false;
                    state.dataSize->DataScalableSizingON = true;
                    state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(CurZoneEqNum).DesHeatVolFlow;
                    bool errorsFound = false;
                    HeatingCapacitySizer sizerHeatingCapacity;
                    sizerHeatingCapacity.overrideSizingString(SizingString);
                    sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                    TempSize = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                    if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).HeatingCapMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                        state.dataSize->DataFracOfAutosizedHeatingCapacity = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity;
                    }
                    state.dataSize->DataAutosizedHeatingCapacity = TempSize;
                    state.dataSize->DataFlowPerHeatingCapacity = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                    PrintFlag = true;
                    TempSize = DataSizing::AutoSize;
                    errorsFound = false;
                    HeatingAirFlowSizer sizingHeatingAirFlow;
                    sizingHeatingAirFlow.overrideSizingString(SizingString);
                    // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                    sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                    unitHeat.MaxAirVolFlow = sizingHeatingAirFlow.size(state, TempSize, errorsFound);
                }
                state.dataSize->DataScalableSizingON = false;
            } else {
                // no scalble sizing method has been specified. Sizing proceeds using the method
                // specified in the zoneHVAC object
                int FieldNum = 1; // N1 , \field Maximum Supply Air Flow Rate
                PrintFlag = true;
                SizingString = state.dataUnitHeaters->UnitHeatNumericFields(UnitHeatNum).FieldNames(FieldNum) + " [m3/s]";
                TempSize = unitHeat.MaxAirVolFlow;
                bool errorsFound = false;
                HeatingAirFlowSizer sizingHeatingAirFlow;
                sizingHeatingAirFlow.overrideSizingString(SizingString);
                // sizingHeatingAirFlow.setHVACSizingIndexData(FanCoil(FanCoilNum).HVACSizingIndex);
                sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                unitHeat.MaxAirVolFlow = sizingHeatingAirFlow.size(state, TempSize, errorsFound);
            }
        }

        bool IsAutoSize = false;
        if (unitHeat.MaxVolHotWaterFlow == DataSizing::AutoSize) {
            IsAutoSize = true;
        }

        if (unitHeat.heatCoilType == HVAC::CoilType::HeatingWater) {

            if (CurZoneEqNum > 0) {
                if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
                    if (unitHeat.MaxVolHotWaterFlow > 0.0) {
                        BaseSizer::reportSizerOutput(state,
                                                     "ZoneHVAC:UnitHeater",
                                                     unitHeat.Name,
                                                     "User-Specified Maximum Hot Water Flow [m3/s]",
                                                     unitHeat.MaxVolHotWaterFlow);
                    }
                } else {
                    CheckZoneSizing(state, "ZoneHVAC:UnitHeater", unitHeat.Name);

                    int CoilWaterInletNode = WaterCoils::GetCoilWaterInletNode(state, unitHeat.HeatCoilNum);
                    int CoilWaterOutletNode = WaterCoils::GetCoilWaterOutletNode(state, unitHeat.HeatCoilNum);

                    if (IsAutoSize) {
                        bool DoWaterCoilSizing = false; // if TRUE do water coil sizing calculation
                        PltSizHeatNum = PlantUtilities::MyPlantSizingIndex(state,
                                                                           HVAC::coilTypeNames[(int)unitHeat.heatCoilType],
                                                                           unitHeat.HeatCoilName,
                                                                           CoilWaterInletNode,
                                                                           CoilWaterOutletNode,
                                                                           ErrorsFound);
                        
                        if (state.dataWaterCoils->WaterCoil(unitHeat.HeatCoilNum).UseDesignWaterDeltaTemp) {
                            WaterCoilSizDeltaT = state.dataWaterCoils->WaterCoil(unitHeat.HeatCoilNum).DesignWaterDeltaTemp;
                            DoWaterCoilSizing = true;
                        } else {
                            if (PltSizHeatNum > 0) {
                                WaterCoilSizDeltaT = state.dataSize->PlantSizData(PltSizHeatNum).DeltaT;
                                DoWaterCoilSizing = true;
                            } else {
                                DoWaterCoilSizing = false;
                                // If there is no heating Plant Sizing object and autosizing was requested, issue fatal error message
                                ShowSevereError(state, "Autosizing of water coil requires a heating loop Sizing:Plant object");
                                ShowContinueError(
                                    state, format("Occurs in ZoneHVAC:UnitHeater Object={}", unitHeat.Name));
                                ErrorsFound = true;
                            }
                        }

                        if (DoWaterCoilSizing) {
                            auto &ZoneEqSizing = state.dataSize->ZoneEqSizing(CurZoneEqNum);
                            int SizingMethod = HVAC::HeatingCapacitySizing;
                            if (unitHeat.HVACSizingIndex > 0) {
                                zoneHVACIndex = unitHeat.HVACSizingIndex;
                                int CapSizingMethod = state.dataSize->ZoneHVACSizing(zoneHVACIndex).HeatingCapMethod;
                                ZoneEqSizing.SizingMethod(SizingMethod) = CapSizingMethod;
                                if (CapSizingMethod == DataSizing::HeatingDesignCapacity || CapSizingMethod == DataSizing::CapacityPerFloorArea ||
                                    CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                                    if (CapSizingMethod == DataSizing::HeatingDesignCapacity) {
                                        if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity == DataSizing::AutoSize) {
                                            ZoneEqSizing.DesHeatingLoad = state.dataSize->FinalZoneSizing(CurZoneEqNum).DesHeatLoad;
                                        } else {
                                            ZoneEqSizing.DesHeatingLoad = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity;
                                        }
                                        ZoneEqSizing.HeatingCapacity = true;
                                        TempSize = DataSizing::AutoSize;
                                    } else if (CapSizingMethod == DataSizing::CapacityPerFloorArea) {
                                        ZoneEqSizing.HeatingCapacity = true;
                                        ZoneEqSizing.DesHeatingLoad = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity *
                                                                      state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                                        state.dataSize->DataScalableCapSizingON = true;
                                    } else if (CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                                        state.dataSize->DataFracOfAutosizedHeatingCapacity =
                                            state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity;
                                        state.dataSize->DataScalableCapSizingON = true;
                                        TempSize = DataSizing::AutoSize;
                                    }
                                }
                                PrintFlag = false;
                                bool errorsFound = false;
                                HeatingCapacitySizer sizerHeatingCapacity;
                                sizerHeatingCapacity.overrideSizingString(SizingString);
                                sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                DesCoilLoad = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                                state.dataSize->DataScalableCapSizingON = false;
                            } else {
                                SizingString = "";
                                PrintFlag = false;
                                TempSize = DataSizing::AutoSize;
                                ZoneEqSizing.HeatingCapacity = true;
                                ZoneEqSizing.DesHeatingLoad = state.dataSize->FinalZoneSizing(CurZoneEqNum).DesHeatLoad;
                                bool errorsFound = false;
                                HeatingCapacitySizer sizerHeatingCapacity;
                                sizerHeatingCapacity.overrideSizingString(SizingString);
                                sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                DesCoilLoad = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                            }

                            if (DesCoilLoad >= HVAC::SmallLoad) {
                                rho = state.dataPlnt->PlantLoop(unitHeat.HWplantLoc.loopNum)
                                          .glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                                Cp = state.dataPlnt->PlantLoop(unitHeat.HWplantLoc.loopNum)
                                         .glycol->getSpecificHeat(state, Constant::HWInitConvTemp, RoutineName);
                                MaxVolHotWaterFlowDes = DesCoilLoad / (WaterCoilSizDeltaT * Cp * rho);
                            } else {
                                MaxVolHotWaterFlowDes = 0.0;
                            }
                        }
                        unitHeat.MaxVolHotWaterFlow = MaxVolHotWaterFlowDes;
                        BaseSizer::reportSizerOutput(state,
                                                     "ZoneHVAC:UnitHeater",
                                                     unitHeat.Name,
                                                     "Design Size Maximum Hot Water Flow [m3/s]",
                                                     MaxVolHotWaterFlowDes);
                    } else {
                        if (unitHeat.MaxVolHotWaterFlow > 0.0 && MaxVolHotWaterFlowDes > 0.0) {
                            MaxVolHotWaterFlowUser = unitHeat.MaxVolHotWaterFlow;
                            BaseSizer::reportSizerOutput(state,
                                                         "ZoneHVAC:UnitHeater",
                                                         unitHeat.Name,
                                                         "Design Size Maximum Hot Water Flow [m3/s]",
                                                         MaxVolHotWaterFlowDes,
                                                         "User-Specified Maximum Hot Water Flow [m3/s]",
                                                         MaxVolHotWaterFlowUser);
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(MaxVolHotWaterFlowDes - MaxVolHotWaterFlowUser) / MaxVolHotWaterFlowUser) >
                                    state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(state,
                                                format("SizeUnitHeater: Potential issue with equipment sizing for ZoneHVAC:UnitHeater {}",
                                                       unitHeat.Name));
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
            unitHeat.MaxVolHotWaterFlow = 0.0;
        }

        IsAutoSize = false;
        if (unitHeat.MaxVolHotSteamFlow == DataSizing::AutoSize) {
            IsAutoSize = true;
        }

        if (unitHeat.heatCoilType == HVAC::CoilType::HeatingSteam) {

            if (CurZoneEqNum > 0) {
                if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) { // Simulation continue
                    if (unitHeat.MaxVolHotSteamFlow > 0.0) {
                        BaseSizer::reportSizerOutput(state,
                                                     "ZoneHVAC:UnitHeater",
                                                     unitHeat.Name,
                                                     "User-Specified Maximum Steam Flow [m3/s]",
                                                     unitHeat.MaxVolHotSteamFlow);
                    }
                } else {
                    auto &ZoneEqSizing = state.dataSize->ZoneEqSizing(CurZoneEqNum);
                    CheckZoneSizing(state, "ZoneHVAC:UnitHeater", unitHeat.Name);

                    int CoilSteamInletNode = SteamCoils::GetCoilSteamInletNode(state, unitHeat.HeatCoilNum);
                    int CoilSteamOutletNode = SteamCoils::GetCoilSteamInletNode(state, unitHeat.HeatCoilNum);
                    if (IsAutoSize) {
                        PltSizHeatNum = PlantUtilities::MyPlantSizingIndex(state,
                                                                           "Coil:Heating:Steam",
                                                                           unitHeat.HeatCoilName,
                                                                           CoilSteamInletNode,
                                                                           CoilSteamOutletNode,
                                                                           ErrorsFound);
                        if (PltSizHeatNum > 0) {
                            if (unitHeat.HVACSizingIndex > 0) {
                                zoneHVACIndex = unitHeat.HVACSizingIndex;
                                int SizingMethod = HVAC::HeatingCapacitySizing;
                                int CapSizingMethod = state.dataSize->ZoneHVACSizing(zoneHVACIndex).HeatingCapMethod;
                                ZoneEqSizing.SizingMethod(SizingMethod) = CapSizingMethod;
                                if (CapSizingMethod == DataSizing::HeatingDesignCapacity || CapSizingMethod == DataSizing::CapacityPerFloorArea ||
                                    CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                                    if (CapSizingMethod == DataSizing::HeatingDesignCapacity) {
                                        if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity == DataSizing::AutoSize) {
                                            ZoneEqSizing.DesHeatingLoad = state.dataSize->FinalZoneSizing(CurZoneEqNum).DesHeatLoad;
                                        } else {
                                            ZoneEqSizing.DesHeatingLoad = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity;
                                        }
                                        ZoneEqSizing.HeatingCapacity = true;
                                        TempSize = DataSizing::AutoSize;
                                    } else if (CapSizingMethod == DataSizing::CapacityPerFloorArea) {
                                        ZoneEqSizing.HeatingCapacity = true;
                                        ZoneEqSizing.DesHeatingLoad = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity *
                                                                      state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                                        state.dataSize->DataScalableCapSizingON = true;
                                    } else if (CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                                        state.dataSize->DataFracOfAutosizedHeatingCapacity =
                                            state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity;
                                        TempSize = DataSizing::AutoSize;
                                        state.dataSize->DataScalableCapSizingON = true;
                                    }
                                }
                                PrintFlag = false;
                                bool errorsFound = false;
                                HeatingCapacitySizer sizerHeatingCapacity;
                                sizerHeatingCapacity.overrideSizingString(SizingString);
                                sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                DesCoilLoad = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                                state.dataSize->DataScalableCapSizingON = false;
                            } else {
                                DesCoilLoad = state.dataSize->FinalZoneSizing(CurZoneEqNum).DesHeatLoad;
                            }
                            if (DesCoilLoad >= HVAC::SmallLoad) {
                                TempSteamIn = 100.00;
                                auto *steam = Fluid::GetSteam(state);
                                EnthSteamInDry = steam->getSatEnthalpy(state, TempSteamIn, 1.0, RoutineName);
                                EnthSteamOutWet = steam->getSatEnthalpy(state, TempSteamIn, 0.0, RoutineName);
                                LatentHeatSteam = EnthSteamInDry - EnthSteamOutWet;
                                SteamDensity = steam->getSatDensity(state, TempSteamIn, 1.0, RoutineName);
                                MaxVolHotSteamFlowDes =
                                    DesCoilLoad / (SteamDensity * (LatentHeatSteam +
                                                                   state.dataSize->PlantSizData(PltSizHeatNum).DeltaT *
                                                                       Psychrometrics::CPHW(state.dataSize->PlantSizData(PltSizHeatNum).ExitTemp)));
                            } else {
                                MaxVolHotSteamFlowDes = 0.0;
                            }
                        } else {
                            ShowSevereError(state, "Autosizing of Steam flow requires a heating loop Sizing:Plant object");
                            ShowContinueError(state,
                                              format("Occurs in ZoneHVAC:UnitHeater Object={}", unitHeat.Name));
                            ErrorsFound = true;
                        }
                        unitHeat.MaxVolHotSteamFlow = MaxVolHotSteamFlowDes;
                        BaseSizer::reportSizerOutput(state,
                                                     "ZoneHVAC:UnitHeater",
                                                     unitHeat.Name,
                                                     "Design Size Maximum Steam Flow [m3/s]",
                                                     MaxVolHotSteamFlowDes);
                    } else {
                        if (unitHeat.MaxVolHotSteamFlow > 0.0 && MaxVolHotSteamFlowDes > 0.0) {
                            MaxVolHotSteamFlowUser = unitHeat.MaxVolHotSteamFlow;
                            BaseSizer::reportSizerOutput(state,
                                                         "ZoneHVAC:UnitHeater",
                                                         unitHeat.Name,
                                                         "Design Size Maximum Steam Flow [m3/s]",
                                                         MaxVolHotSteamFlowDes,
                                                         "User-Specified Maximum Steam Flow [m3/s]",
                                                         MaxVolHotSteamFlowUser);
                            if (state.dataGlobal->DisplayExtraWarnings) {
                                if ((std::abs(MaxVolHotSteamFlowDes - MaxVolHotSteamFlowUser) / MaxVolHotSteamFlowUser) >
                                    state.dataSize->AutoVsHardSizingThreshold) {
                                    ShowMessage(state,
                                                format("SizeUnitHeater: Potential issue with equipment sizing for ZoneHVAC:UnitHeater {}",
                                                       unitHeat.Name));
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
            unitHeat.MaxVolHotSteamFlow = 0.0;
        }

        // set the design air flow rate for the heating coil
        // This was previously applied to all coil types (I guess it just generated severe errors?)
        // Why is this not applied to Steam and Heating coils?
        if (unitHeat.heatCoilType == HVAC::CoilType::HeatingWater) {
            WaterCoils::SetCoilDesFlow(state, unitHeat.HeatCoilNum, unitHeat.MaxAirVolFlow);
        }
        
        if (CurZoneEqNum > 0) {
            auto &ZoneEqSizing = state.dataSize->ZoneEqSizing(CurZoneEqNum);
            ZoneEqSizing.MaxHWVolFlow = unitHeat.MaxVolHotWaterFlow;
        }

        if (ErrorsFound) {
            ShowFatalError(state, "Preceding sizing errors cause program termination");
        }
    }

    void CalcUnitHeater(EnergyPlusData &state,
                        int &UnitHeatNum,              // number of the current fan coil unit being simulated
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
        // This subroutine mainly controls the action of the unit heater
        // based on the user input for controls and the defined controls
        // algorithms.  There are currently (at the initial creation of this
        // subroutine) two control methods: on-off fan operation or continuous
        // fan operation.

        // METHODOLOGY EMPLOYED:
        // Unit is controlled based on user input and what is happening in the
        // simulation.  There are various cases to consider:
        // 1. OFF: Unit is schedule off.  All flow rates are set to zero and
        //    the temperatures are set to zone conditions.
        // 2. NO LOAD OR COOLING/ON-OFF FAN CONTROL: Unit is available, but
        //    there is no heating load.  All flow rates are set to zero and
        //    the temperatures are set to zone conditions.
        // 3. NO LOAD OR COOLING/CONTINUOUS FAN CONTROL: Unit is available and
        //    the fan is running (if it is scheduled to be available also).
        //    No heating is provided, only circulation via the fan running.
        // 4. HEATING: The unit is on/available and there is a heating load.
        //    The heating coil is modulated (constant fan speed) to meet the
        //    heating load.

        // REFERENCES:
        // ASHRAE Systems and Equipment Handbook (SI), 1996. page 31.7

        // SUBROUTINE PARAMETER DEFINITIONS:
        int constexpr MaxIter = 100; // maximum number of iterations

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 SpecHumOut; // Specific humidity ratio of outlet air (kg moisture / kg moist air)
        Real64 SpecHumIn;  // Specific humidity ratio of inlet air (kg moisture / kg moist air)
        Real64 mdot;       // local temporary for fluid mass flow rate

        // initialize local variables
        Real64 QUnitOut = 0.0;
        Real64 NoOutput = 0.0;
        Real64 FullOutput = 0.0;
        Real64 LatentOutput = 0.0; // Latent (moisture) add/removal rate, negative is dehumidification [kg/s]
        Real64 MaxWaterFlow = 0.0;
        Real64 MinWaterFlow = 0.0;
        Real64 PartLoadFrac = 0.0;

        auto &s_node = state.dataLoopNodes;
        auto &unitHeat = state.dataUnitHeaters->UnitHeat(UnitHeatNum);

        Real64 ControlOffset = unitHeat.HotControlOffset;
        HVAC::FanOp fanOp = unitHeat.fanOp;

        if (fanOp != HVAC::FanOp::Cycling) {

            if (unitHeat.availSched->getCurrentVal() <= 0 ||
                ((unitHeat.fanAvailSched->getCurrentVal() <= 0 && !state.dataHVACGlobal->TurnFansOn) ||
                 state.dataHVACGlobal->TurnFansOff)) {
                // Case 1: OFF-->unit schedule says that it it not available
                //         OR child fan in not available OR child fan not being cycled ON by sys avail manager
                //         OR child fan being forced OFF by sys avail manager
                state.dataUnitHeaters->HCoilOn = false;
                if (unitHeat.heatCoilType == HVAC::CoilType::HeatingWater) {
                    mdot = 0.0; // try to turn off

                    PlantUtilities::SetComponentFlowRate(state,
                                                         mdot,
                                                         unitHeat.HotControlNode,
                                                         unitHeat.HotCoilOutNodeNum,
                                                         unitHeat.HWplantLoc);

                } else if (unitHeat.heatCoilType == HVAC::CoilType::HeatingSteam) {
                    mdot = 0.0; // try to turn off

                    PlantUtilities::SetComponentFlowRate(state,
                                                         mdot,
                                                         unitHeat.HotControlNode,
                                                         unitHeat.HotCoilOutNodeNum,
                                                         unitHeat.HWplantLoc);
                }
                CalcUnitHeaterComponents(state, UnitHeatNum, FirstHVACIteration, QUnitOut);

            } else if ((state.dataUnitHeaters->QZnReq < HVAC::SmallLoad) || state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) {
                // Unit is available, but there is no load on it or we are in setback/deadband
                if (!unitHeat.FanOffNoHeating) {

                    // Case 2: NO LOAD OR COOLING/ON-OFF FAN CONTROL-->turn everything off
                    //         because there is no load on the unit heater
                    state.dataUnitHeaters->HCoilOn = false;
                    if (unitHeat.heatCoilType == HVAC::CoilType::HeatingWater) {
                        mdot = 0.0; // try to turn off

                        PlantUtilities::SetComponentFlowRate(state,
                                                             mdot,
                                                             unitHeat.HotControlNode,
                                                             unitHeat.HotCoilOutNodeNum,
                                                             unitHeat.HWplantLoc);

                    } else if (unitHeat.heatCoilType == HVAC::CoilType::HeatingSteam) {
                        mdot = 0.0; // try to turn off

                        PlantUtilities::SetComponentFlowRate(state,
                                                             mdot,
                                                             unitHeat.HotControlNode,
                                                             unitHeat.HotCoilOutNodeNum,
                                                             unitHeat.HWplantLoc);
                    }
                    CalcUnitHeaterComponents(state, UnitHeatNum, FirstHVACIteration, QUnitOut);

                } else {
                    // Case 3: NO LOAD OR COOLING/CONTINUOUS FAN CONTROL-->let the fan
                    //         continue to run even though there is no load (air circulation)
                    // Note that the flow rates were already set in the initialization routine
                    // so there is really nothing else left to do except call the components.

                    state.dataUnitHeaters->HCoilOn = false;
                    if (unitHeat.heatCoilType == HVAC::CoilType::HeatingWater) {
                        mdot = 0.0; // try to turn off

                        if (unitHeat.HWplantLoc.loopNum > 0) {
                            PlantUtilities::SetComponentFlowRate(state,
                                                                 mdot,
                                                                 unitHeat.HotControlNode,
                                                                 unitHeat.HotCoilOutNodeNum,
                                                                 unitHeat.HWplantLoc);
                        }

                    } else if (unitHeat.heatCoilType == HVAC::CoilType::HeatingSteam) {
                        mdot = 0.0; // try to turn off
                        if (unitHeat.HWplantLoc.loopNum > 0) {
                            PlantUtilities::SetComponentFlowRate(state,
                                                                 mdot,
                                                                 unitHeat.HotControlNode,
                                                                 unitHeat.HotCoilOutNodeNum,
                                                                 unitHeat.HWplantLoc);
                        }
                    }

                    CalcUnitHeaterComponents(state, UnitHeatNum, FirstHVACIteration, QUnitOut);
                }

            } else { // Case 4: HEATING-->unit is available and there is a heating load

                switch (unitHeat.heatCoilType) {

                case HVAC::CoilType::HeatingWater: {

                    // On the first HVAC iteration the system values are given to the controller, but after that
                    // the demand limits are in place and there needs to be feedback to the Zone Equipment
                    if (FirstHVACIteration) {
                        MaxWaterFlow = unitHeat.MaxHotWaterFlow;
                        MinWaterFlow = unitHeat.MinHotWaterFlow;
                    } else {
                        MaxWaterFlow = s_node->Node(unitHeat.HotControlNode).MassFlowRateMaxAvail;
                        MinWaterFlow = s_node->Node(unitHeat.HotControlNode).MassFlowRateMinAvail;
                    }
                    // control water flow to obtain output matching QZnReq
                    ControlCompOutput(state,
                                      unitHeat.Name,
                                      state.dataUnitHeaters->cMO_UnitHeater,
                                      UnitHeatNum,
                                      FirstHVACIteration,
                                      state.dataUnitHeaters->QZnReq,
                                      unitHeat.HotControlNode,
                                      MaxWaterFlow,
                                      MinWaterFlow,
                                      unitHeat.HotControlOffset,
                                      unitHeat.ControlCompTypeNum,
                                      unitHeat.CompErrIndex,
                                      _,
                                      _,
                                      _,
                                      _,
                                      _,
                                      unitHeat.HWplantLoc);
                } break;
                  
                case HVAC::CoilType::HeatingElectric:
                case HVAC::CoilType::HeatingGasOrOtherFuel:
                case HVAC::CoilType::HeatingSteam: {
                    state.dataUnitHeaters->HCoilOn = true;
                    CalcUnitHeaterComponents(state, UnitHeatNum, FirstHVACIteration, QUnitOut);
                } break;
                default:
                    break;
                }
            }
            if (s_node->Node(unitHeat.AirInNode).MassFlowRateMax > 0.0) {
                unitHeat.FanPartLoadRatio =
                    s_node->Node(unitHeat.AirInNode).MassFlowRate / s_node->Node(unitHeat.AirInNode).MassFlowRateMax;
            }
        } else { // OnOff fan and cycling
            if ((state.dataUnitHeaters->QZnReq < HVAC::SmallLoad) || (state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) ||
                unitHeat.availSched->getCurrentVal() <= 0 ||
                ((unitHeat.fanAvailSched->getCurrentVal() <= 0 && !state.dataHVACGlobal->TurnFansOn) ||
                 state.dataHVACGlobal->TurnFansOff)) {
                // Case 1: OFF-->unit schedule says that it it not available
                //         OR child fan in not available OR child fan not being cycled ON by sys avail manager
                //         OR child fan being forced OFF by sys avail manager
                PartLoadFrac = 0.0;
                state.dataUnitHeaters->HCoilOn = false;
                CalcUnitHeaterComponents(state, UnitHeatNum, FirstHVACIteration, QUnitOut, fanOp, PartLoadFrac);

                if (s_node->Node(unitHeat.AirInNode).MassFlowRateMax > 0.0) {
                    unitHeat.FanPartLoadRatio =
                        s_node->Node(unitHeat.AirInNode).MassFlowRate / s_node->Node(unitHeat.AirInNode).MassFlowRateMax;
                }

            } else { // Case 4: HEATING-->unit is available and there is a heating load

                state.dataUnitHeaters->HCoilOn = true;

                // Find part load ratio of unit heater coils
                PartLoadFrac = 0.0;
                CalcUnitHeaterComponents(state, UnitHeatNum, FirstHVACIteration, NoOutput, fanOp, PartLoadFrac);
                if ((NoOutput - state.dataUnitHeaters->QZnReq) < HVAC::SmallLoad) {
                    // Unit heater is unable to meet the load with coil off, set PLR = 1
                    PartLoadFrac = 1.0;
                    CalcUnitHeaterComponents(state, UnitHeatNum, FirstHVACIteration, FullOutput, fanOp, PartLoadFrac);
                    if ((FullOutput - state.dataUnitHeaters->QZnReq) > HVAC::SmallLoad) {
                        // Unit heater full load capacity is able to meet the load, Find PLR

                        auto f = [&state, UnitHeatNum, FirstHVACIteration, fanOp](Real64 const PartLoadRatio) {
                            // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
                            Real64 QUnitOut; // heating provided by unit heater [watts]

                            CalcUnitHeaterComponents(state, UnitHeatNum, FirstHVACIteration, QUnitOut, fanOp, PartLoadRatio);

                            // Calculate residual based on output calculation flag
                            if (state.dataUnitHeaters->QZnReq != 0.0) {
                                return (QUnitOut - state.dataUnitHeaters->QZnReq) / state.dataUnitHeaters->QZnReq;
                            } else {
                                return 0.0;
                            }
                        };

                        // Tolerance is in fraction of load, MaxIter = 30, SolFalg = # of iterations or error as appropriate
                        int SolFlag = 0; // # of iterations IF positive, -1 means failed to converge, -2 means bounds are incorrect
                        General::SolveRoot(state, 0.001, MaxIter, SolFlag, PartLoadFrac, f, 0.0, 1.0);
                    }
                }

                CalcUnitHeaterComponents(state, UnitHeatNum, FirstHVACIteration, QUnitOut, fanOp, PartLoadFrac);

            } // ...end of unit ON/OFF IF-THEN block
            unitHeat.PartLoadFrac = PartLoadFrac;
            unitHeat.FanPartLoadRatio = PartLoadFrac;
            s_node->Node(unitHeat.AirOutNode).MassFlowRate = s_node->Node(unitHeat.AirInNode).MassFlowRate;
        }

        // CR9155 Remove specific humidity calculations
        SpecHumOut = s_node->Node(unitHeat.AirOutNode).HumRat;
        SpecHumIn = s_node->Node(unitHeat.AirInNode).HumRat;
        LatentOutput = s_node->Node(unitHeat.AirOutNode).MassFlowRate * (SpecHumOut - SpecHumIn); // Latent rate (kg/s), dehumid = negative

        QUnitOut = s_node->Node(unitHeat.AirOutNode).MassFlowRate *
                   (Psychrometrics::PsyHFnTdbW(s_node->Node(unitHeat.AirOutNode).Temp, s_node->Node(unitHeat.AirInNode).HumRat) -
                    Psychrometrics::PsyHFnTdbW(s_node->Node(unitHeat.AirInNode).Temp, s_node->Node(unitHeat.AirInNode).HumRat));

        // Report variables...
        unitHeat.HeatPower = max(0.0, QUnitOut);
        unitHeat.ElecPower =
            state.dataFans->fans(unitHeat.Fan_Index)->totalPower;

        PowerMet = QUnitOut;
        LatOutputProvided = LatentOutput;
    }

    void CalcUnitHeaterComponents(EnergyPlusData &state,
                                  int const UnitHeatNum,         // Unit index in unit heater array
                                  bool const FirstHVACIteration, // flag for 1st HVAV iteration in the time step
                                  Real64 &LoadMet,               // load met by unit (watts)
                                  HVAC::FanOp const fanOp,       // fan operating mode
                                  Real64 const PartLoadRatio     // part-load ratio
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
        // Simply calls the different components in order.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 AirMassFlow; // total mass flow through the unit
        Real64 CpAirZn;     // specific heat of dry air at zone conditions (zone conditions same as unit inlet)
        Real64 mdot;        // local temporary for fluid mass flow rate

        auto &s_node = state.dataLoopNodes;
        auto &unitHeat = state.dataUnitHeaters->UnitHeat(UnitHeatNum);
        Real64 QCoilReq = 0.0;

        if (fanOp != HVAC::FanOp::Cycling) {
            state.dataFans->fans(unitHeat.Fan_Index)->simulate(state, FirstHVACIteration, _, _);

            switch (unitHeat.heatCoilType) {

            case HVAC::CoilType::HeatingWater: {
                WaterCoils::SimulateWaterCoilComponents(state, unitHeat.HeatCoilName, FirstHVACIteration, unitHeat.HeatCoilNum);
            } break;
              
            case HVAC::CoilType::HeatingSteam: {

                if (!state.dataUnitHeaters->HCoilOn) {
                    QCoilReq = 0.0;
                } else {
                    CpAirZn = Psychrometrics::PsyCpAirFnW(s_node->Node(unitHeat.AirInNode).HumRat);
                    QCoilReq =
                        state.dataUnitHeaters->QZnReq - s_node->Node(unitHeat.FanOutletNode).MassFlowRate * CpAirZn *
                                                            (s_node->Node(unitHeat.FanOutletNode).Temp -
                                                             s_node->Node(unitHeat.AirInNode).Temp);
                }
                if (QCoilReq < 0.0) QCoilReq = 0.0; // a heating coil can only heat, not cool
                SteamCoils::SimulateSteamCoilComponents(state, unitHeat.HeatCoilName, FirstHVACIteration, unitHeat.HeatCoilNum, QCoilReq);
            } break;
              
            case HVAC::CoilType::HeatingElectric:
            case HVAC::CoilType::HeatingGasOrOtherFuel: {

                if (!state.dataUnitHeaters->HCoilOn) {
                    QCoilReq = 0.0;
                } else {
                    CpAirZn = Psychrometrics::PsyCpAirFnW(s_node->Node(unitHeat.AirInNode).HumRat);
                    QCoilReq =
                        state.dataUnitHeaters->QZnReq - s_node->Node(unitHeat.FanOutletNode).MassFlowRate * CpAirZn *
                                                            (s_node->Node(unitHeat.FanOutletNode).Temp -
                                                             s_node->Node(unitHeat.AirInNode).Temp);
                }
                if (QCoilReq < 0.0) QCoilReq = 0.0; // a heating coil can only heat, not cool
                HeatingCoils::SimulateHeatingCoilComponents(state, unitHeat.HeatCoilName, FirstHVACIteration, QCoilReq, unitHeat.HeatCoilNum);
            } break;
            default:
                break;
            }

            AirMassFlow = s_node->Node(unitHeat.AirOutNode).MassFlowRate;

            s_node->Node(unitHeat.AirInNode).MassFlowRate =
                s_node->Node(unitHeat.AirOutNode).MassFlowRate; // maintain continuity through unit heater

        } else { // OnOff fan cycling

            s_node->Node(unitHeat.AirInNode).MassFlowRate = s_node->Node(unitHeat.AirInNode).MassFlowRateMax * PartLoadRatio;
            AirMassFlow = s_node->Node(unitHeat.AirInNode).MassFlowRate;
            // Set the fan inlet node maximum available mass flow rates for cycling fans
            s_node->Node(unitHeat.AirInNode).MassFlowRateMaxAvail = AirMassFlow;

            if (QCoilReq < 0.0) QCoilReq = 0.0; // a heating coil can only heat, not cool
            state.dataFans->fans(unitHeat.Fan_Index)->simulate(state, FirstHVACIteration, _, _);

            switch (unitHeat.heatCoilType) {

            case HVAC::CoilType::HeatingWater: {

                if (!state.dataUnitHeaters->HCoilOn) {
                    mdot = 0.0;
                    QCoilReq = 0.0;
                } else {
                    CpAirZn = Psychrometrics::PsyCpAirFnW(s_node->Node(unitHeat.AirInNode).HumRat);
                    QCoilReq =
                        state.dataUnitHeaters->QZnReq - s_node->Node(unitHeat.FanOutletNode).MassFlowRate * CpAirZn *
                                                            (s_node->Node(unitHeat.FanOutletNode).Temp -
                                                             s_node->Node(unitHeat.AirInNode).Temp);
                    mdot = unitHeat.MaxHotWaterFlow * PartLoadRatio;
                }
                if (QCoilReq < 0.0) {
                    QCoilReq = 0.0; // a heating coil can only heat, not cool
                }
                PlantUtilities::SetComponentFlowRate(state,
                                                     mdot,
                                                     unitHeat.HotControlNode,
                                                     unitHeat.HotCoilOutNodeNum,
                                                     unitHeat.HWplantLoc);
                WaterCoils::SimulateWaterCoilComponents(state,
                                                        unitHeat.HeatCoilName,
                                                        FirstHVACIteration,
                                                        unitHeat.HeatCoilNum,
                                                        QCoilReq,
                                                        fanOp,
                                                        PartLoadRatio);
                break;
            }
            case HVAC::CoilType::HeatingSteam: {
                if (!state.dataUnitHeaters->HCoilOn) {
                    mdot = 0.0;
                    QCoilReq = 0.0;
                } else {
                    CpAirZn = Psychrometrics::PsyCpAirFnW(s_node->Node(unitHeat.AirInNode).HumRat);
                    QCoilReq =
                        state.dataUnitHeaters->QZnReq - s_node->Node(unitHeat.FanOutletNode).MassFlowRate * CpAirZn *
                                                            (s_node->Node(unitHeat.FanOutletNode).Temp -
                                                             s_node->Node(unitHeat.AirInNode).Temp);
                    mdot = unitHeat.MaxHotSteamFlow * PartLoadRatio;
                }
                if (QCoilReq < 0.0) {
                    QCoilReq = 0.0; // a heating coil can only heat, not cool
                }
                PlantUtilities::SetComponentFlowRate(state,
                                                     mdot,
                                                     unitHeat.HotControlNode,
                                                     unitHeat.HotCoilOutNodeNum,
                                                     unitHeat.HWplantLoc);
                SteamCoils::SimulateSteamCoilComponents(state,
                                                        unitHeat.HeatCoilName,
                                                        FirstHVACIteration,
                                                        unitHeat.HeatCoilNum,
                                                        QCoilReq,
                                                        _,
                                                        fanOp,
                                                        PartLoadRatio);
                break;
            }
            case HVAC::CoilType::HeatingElectric:
            case HVAC::CoilType::HeatingGasOrOtherFuel: {

                if (!state.dataUnitHeaters->HCoilOn) {
                    QCoilReq = 0.0;
                } else {
                    CpAirZn = Psychrometrics::PsyCpAirFnW(s_node->Node(unitHeat.AirInNode).HumRat);
                    QCoilReq =
                        state.dataUnitHeaters->QZnReq - s_node->Node(unitHeat.FanOutletNode).MassFlowRate * CpAirZn *
                                                            (s_node->Node(unitHeat.FanOutletNode).Temp -
                                                             s_node->Node(unitHeat.AirInNode).Temp);
                }
                if (QCoilReq < 0.0) {
                    QCoilReq = 0.0; // a heating coil can only heat, not cool
                }
                HeatingCoils::SimulateHeatingCoilComponents(state,
                                                            unitHeat.HeatCoilName,
                                                            FirstHVACIteration,
                                                            QCoilReq,
                                                            unitHeat.HeatCoilNum,
                                                            _,
                                                            _,
                                                            fanOp,
                                                            PartLoadRatio);
                break;
            }
            default:
                break;
            }
            s_node->Node(unitHeat.AirOutNode).MassFlowRate = s_node->Node(unitHeat.AirInNode).MassFlowRate; // maintain continuity through unit heater
        }
        LoadMet = AirMassFlow * (Psychrometrics::PsyHFnTdbW(s_node->Node(unitHeat.AirOutNode).Temp, s_node->Node(unitHeat.AirInNode).HumRat) -
                                 Psychrometrics::PsyHFnTdbW(s_node->Node(unitHeat.AirInNode).Temp, s_node->Node(unitHeat.AirInNode).HumRat));
    }

    // SUBROUTINE UpdateUnitHeater

    // No update routine needed in this module since all of the updates happen on
    // the Node derived type directly and these updates are done by other routines.

    // END SUBROUTINE UpdateUnitHeater

    void ReportUnitHeater(EnergyPlusData &state, int const UnitHeatNum) // Unit index in unit heater array
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Rick Strand
        //       DATE WRITTEN   May 2000

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine needs a description.

        // METHODOLOGY EMPLOYED:
        // Needs description, as appropriate.

        // Using/Aliasing
        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

        auto &unitHeat = state.dataUnitHeaters->UnitHeat(UnitHeatNum);
        
        unitHeat.HeatEnergy = unitHeat.HeatPower * TimeStepSysSec;
        unitHeat.ElecEnergy = unitHeat.ElecPower * TimeStepSysSec;

        if (unitHeat.FirstPass) { // reset sizing flags so other zone equipment can size normally
            if (!state.dataGlobal->SysSizingCalc) {
                DataSizing::resetHVACSizingGlobals(state, state.dataSize->CurZoneEqNum, 0, unitHeat.FirstPass);
            }
        }
    }

    int getUnitHeaterIndex(EnergyPlusData &state, std::string_view CompName)
    {
        if (state.dataUnitHeaters->GetUnitHeaterInputFlag) {
            GetUnitHeaterInput(state);
            state.dataUnitHeaters->GetUnitHeaterInputFlag = false;
        }

        for (int UnitHeatNum = 1; UnitHeatNum <= state.dataUnitHeaters->NumOfUnitHeats; ++UnitHeatNum) {
            if (Util::SameString(state.dataUnitHeaters->UnitHeat(UnitHeatNum).Name, CompName)) {
                return UnitHeatNum;
            }
        }

        return 0;
    }

} // namespace UnitHeater

} // namespace EnergyPlus

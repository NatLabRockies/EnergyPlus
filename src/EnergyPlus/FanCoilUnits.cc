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
#include <EnergyPlus/Autosizing/All_Simple_Sizing.hh>
#include <EnergyPlus/Autosizing/CoolingAirFlowSizing.hh>
#include <EnergyPlus/Autosizing/CoolingCapacitySizing.hh>
#include <EnergyPlus/Autosizing/HeatingAirFlowSizing.hh>
#include <EnergyPlus/Autosizing/HeatingCapacitySizing.hh>
#include <EnergyPlus/Autosizing/SystemAirFlowSizing.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataHeatBalFanSys.hh>
#include <EnergyPlus/DataHeatBalance.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataPrecisionGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/FanCoilUnits.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/HVACHXAssistedCoolingCoil.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ReportCoilSelection.hh>
#include <EnergyPlus/SZVAVModel.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SingleDuct.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterCoils.hh>
#include <EnergyPlus/ZoneEquipmentManager.hh>
#include <EnergyPlus/ZonePlenum.hh>

namespace EnergyPlus {

namespace FanCoilUnits {

    // Module containing the routines dealing with 2 and 4 pipe fan coil units

    // MODULE INFORMATION:
    //       AUTHOR         Fred Buhl
    //       DATE WRITTEN   March 2000
    //       MODIFIED       October 2003 (FSEC added cooling coil type)
    //                      June 2010    Arnaud Flament LBNL added 3-speed and variables-speed fan capacity control;
    //                                   outside air schedule; and removed coil water inlet node inputs
    //                      Sept 2010    Brent Griffith, plant upgrades for water coils, fluid properties

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and algorithms needed to simulate 2 and 4 pipe
    // fan coil units.

    // METHODOLOGY EMPLOYED:
    // Units are modeled as a collection of components: outside air mixer,
    // fan, heating coil and/or cooling coil plus an integrated control
    // algorithm that adjusts the hot or cold water flow to meet the zone
    // load. Or varies the air flow rate to meet the zone load. Or both.

    void SimFanCoilUnit(EnergyPlusData &state,
                        std::string_view CompName,     // name of the fan coil unit
                        int const ControlledZoneNum,   // number of zone being served
                        bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                        Real64 &PowerMet,              // Sensible power supplied (W)
                        Real64 &LatOutputProvided,     // Latent add/removal supplied by window AC (kg/s), dehumid = negative
                        int &CompIndex)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   March 2000
        //       MODIFIED       Don Shirey, Aug 2009 (LatOutputProvided)

        // PURPOSE OF THIS SUBROUTINE:
        // Manages the simulation of a fan coil unit. Called from SimZone Equipment

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int FanCoilNum; // index of fan coil unit being simulated

        // First time SimFanCoilUnit is called, get the input for all the fan coil units
        if (state.dataFanCoilUnits->GetFanCoilInputFlag) {
            GetFanCoilUnits(state);
            state.dataFanCoilUnits->GetFanCoilInputFlag = false;
        }

        // Find the correct Fan Coil Equipment
        if (CompIndex == 0) {
            FanCoilNum = Util::FindItemInList(CompName, state.dataFanCoilUnits->FanCoil);
            if (FanCoilNum == 0) {
                ShowFatalError(state, format("SimFanCoil: Unit not found={}", CompName));
            }
            CompIndex = FanCoilNum;
        } else {
            FanCoilNum = CompIndex;
            if (FanCoilNum > state.dataFanCoilUnits->NumFanCoils || FanCoilNum < 1) {
                ShowFatalError(state,
                               format("SimFanCoil:  Invalid CompIndex passed={}, Number of Units={}, Entered Unit name={}",
                                      FanCoilNum,
                                      state.dataFanCoilUnits->NumFanCoils,
                                      CompName));
            }
            if (state.dataFanCoilUnits->CheckEquipName(FanCoilNum)) {
                if (CompName != state.dataFanCoilUnits->FanCoil(FanCoilNum).Name) {
                    ShowFatalError(state,
                                   format("SimFanCoil: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                          FanCoilNum,
                                          CompName,
                                          state.dataFanCoilUnits->FanCoil(FanCoilNum).Name));
                }
                state.dataFanCoilUnits->CheckEquipName(FanCoilNum) = false;
            }
        }

        state.dataSize->ZoneEqFanCoil = true;

        // Initialize the fan coil unit
        InitFanCoilUnits(state, FanCoilNum, ControlledZoneNum);

        // Select the correct unit type
        if (state.dataFanCoilUnits->FanCoil(FanCoilNum).UnitType_Num == FanCoilUnit_4Pipe) {
            Sim4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, PowerMet, LatOutputProvided);
        }

        // Report the result of the simulation
        ReportFanCoilUnit(state, FanCoilNum);

        state.dataSize->ZoneEqFanCoil = false;
    }

    void GetFanCoilUnits(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   March 2000
        //       MODIFIED       Bereket Nigusse, FSEC, April 2011: eliminated input node names
        //                                                         added OA Mixer object type
        //                                                         and fan object type
        //                      Chandan Sharma, FSEC, July 2012: Added zone sys avail managers

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for fan coil units and stores it in fan coil data structures

        // METHODOLOGY EMPLOYED:
        // Uses "Get" routines to read in data.

        // Locals
        // SUBROUTINE ARGUMENT DEFINITIONS:
        static constexpr std::string_view RoutineName("GetFanCoilUnits: "); // include trailing blank space
        static constexpr std::string_view routineName = "GetFanCoilUnits";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;                 // Number of Alphas for each GetObjectItem call
        int NumNumbers;                // Number of Numbers for each GetObjectItem call
        Array1D_int OANodeNums(4);     // Node numbers of Outdoor air mixer (OA, EA, RA, MA)
        int IOStatus;                  // Used in GetObjectItem
        bool IsNotOK;                  // Flag to verify name
        Array1D_string Alphas;         // Alpha input items for object
        Array1D_string cAlphaFields;   // Alpha field names
        Array1D_string cNumericFields; // Numeric field names
        Array1D<Real64> Numbers;       // Numeric input items for object
        Array1D_bool lAlphaBlanks;     // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;   // Logical array, numeric field input BLANK = .TRUE.
        int NodeNum;                   // index to loop counter
        std::string ATMixerName;

        auto &ErrorsFound = state.dataFanCoilUnits->ErrorsFound;
        bool &errFlag = state.dataFanCoilUnits->errFlag;

        // find the number of each type of fan coil unit

        std::string CurrentModuleObject = state.dataFanCoilUnits->cMO_FanCoil;
        state.dataFanCoilUnits->Num4PipeFanCoils = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);
        state.dataFanCoilUnits->NumFanCoils = state.dataFanCoilUnits->Num4PipeFanCoils;
        // allocate the data structures
        state.dataFanCoilUnits->FanCoil.allocate(state.dataFanCoilUnits->NumFanCoils);
        state.dataFanCoilUnits->FanCoilNumericFields.allocate(state.dataFanCoilUnits->NumFanCoils);
        state.dataFanCoilUnits->CheckEquipName.dimension(state.dataFanCoilUnits->NumFanCoils, true);

        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, CurrentModuleObject, state.dataFanCoilUnits->TotalArgs, NumAlphas, NumNumbers);
        Alphas.allocate(NumAlphas);
        cAlphaFields.allocate(NumAlphas);
        cNumericFields.allocate(NumNumbers);
        Numbers.dimension(NumNumbers, 0.0);
        lAlphaBlanks.dimension(NumAlphas, true);
        lNumericBlanks.dimension(NumNumbers, true);

        // loop over 4 pipe fan coil units; get and load the input data
        for (int FanCoilIndex = 1; FanCoilIndex <= state.dataFanCoilUnits->Num4PipeFanCoils; ++FanCoilIndex) {
            auto &fanCoil = state.dataFanCoilUnits->FanCoil(FanCoilIndex);

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     FanCoilIndex,
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

            state.dataFanCoilUnits->FanCoilNumericFields(FanCoilIndex).FieldNames.allocate(NumNumbers);
            state.dataFanCoilUnits->FanCoilNumericFields(FanCoilIndex).FieldNames = "";
            state.dataFanCoilUnits->FanCoilNumericFields(FanCoilIndex).FieldNames = cNumericFields;

            fanCoil.Name = Alphas(1);
            fanCoil.UnitType = CurrentModuleObject;
            fanCoil.UnitType_Num = FanCoilUnit_4Pipe;

            if (lAlphaBlanks(2)) {
                fanCoil.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((fanCoil.availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ErrorsFound = true;
            }
            constexpr std::array<std::string_view, static_cast<int>(CCM::Num)> CapCtrlMethUC{"CONSTANTFANVARIABLEFLOW",
                                                                                             "CYCLINGFAN",
                                                                                             "VARIABLEFANVARIABLEFLOW",
                                                                                             "VARIABLEFANCONSTANTFLOW",
                                                                                             "MULTISPEEDFAN",
                                                                                             "ASHRAE90VARIABLEFAN"};
            std::string capCtrlMeth = Alphas(3);
            fanCoil.CapCtrlMeth_Num = static_cast<CCM>(getEnumValue(CapCtrlMethUC, capCtrlMeth));
            if (fanCoil.CapCtrlMeth_Num == CCM::ASHRAE) {
                fanCoil.DesZoneCoolingLoad = DataSizing::AutoSize;
                fanCoil.DesZoneHeatingLoad = DataSizing::AutoSize;
                fanCoil.fanOp = HVAC::FanOp::Continuous;
            }

            if (!lAlphaBlanks(4) && ((fanCoil.oaSched = Sched::GetSchedule(state, Alphas(4))) == nullptr)) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(4), Alphas(4));
                ErrorsFound = true;
            }
            fanCoil.MaxAirVolFlow = Numbers(1);
            fanCoil.LowSpeedRatio = Numbers(2);
            fanCoil.MedSpeedRatio = Numbers(3);
            // check if low speed ratio < medium speed ratio, if not : warning & set to default values
            if (fanCoil.LowSpeedRatio > fanCoil.MedSpeedRatio) {
                ShowWarningError(state, format("{}{}=\"{}\",", RoutineName, CurrentModuleObject, fanCoil.Name));
                ShowContinueError(state, format("... {} is greater than the medium speed supply air flow ratio.", cNumericFields(2)));
                ShowContinueError(state, format("... Fan Coil Unit low speed supply air flow ratio = {:.5T} ", fanCoil.LowSpeedRatio));
                ShowContinueError(state, format("... Fan Coit Unit medium speed supply air flow ratio = {:.5T} ", fanCoil.MedSpeedRatio));
                ShowContinueError(state,
                                  "... Fan Coil Unit low speed supply air flow ratio and medium speed supply air flow ratio set to default values");
                fanCoil.LowSpeedRatio = 1.0 / 3.0;
                fanCoil.MedSpeedRatio = 2.0 / 3.0;
            }

            fanCoil.OutAirVolFlow = Numbers(4);

            fanCoil.AirInNode = NodeInputManager::GetOnlySingleNode(state,
                                                                    Alphas(5),
                                                                    ErrorsFound,
                                                                    DataLoopNode::ConnectionObjectType::ZoneHVACFourPipeFanCoil,
                                                                    Alphas(1),
                                                                    DataLoopNode::NodeFluidType::Air,
                                                                    DataLoopNode::ConnectionType::Inlet,
                                                                    NodeInputManager::CompFluidStream::Primary,
                                                                    DataLoopNode::ObjectIsParent); // air input node

            fanCoil.AirOutNode = NodeInputManager::GetOnlySingleNode(state,
                                                                     Alphas(6),
                                                                     ErrorsFound,
                                                                     DataLoopNode::ConnectionObjectType::ZoneHVACFourPipeFanCoil,
                                                                     Alphas(1),
                                                                     DataLoopNode::NodeFluidType::Air,
                                                                     DataLoopNode::ConnectionType::Outlet,
                                                                     NodeInputManager::CompFluidStream::Primary,
                                                                     DataLoopNode::ObjectIsParent); // air outlet node

            fanCoil.OAMixType = Alphas(7);
            fanCoil.OAMixName = Alphas(8);
            // check to see if local OA mixer specified
            if (!lAlphaBlanks(8)) {
                errFlag = false;
                ValidateComponent(state, fanCoil.OAMixType, fanCoil.OAMixName, errFlag, CurrentModuleObject);
                if (errFlag) {
                    ShowContinueError(state, format("specified in {} = \"{}\".", CurrentModuleObject, fanCoil.Name));
                    ErrorsFound = true;
                } else {
                    // Get outdoor air mixer node numbers
                    OANodeNums = MixedAir::GetOAMixerNodeNumbers(state, fanCoil.OAMixName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("that was specified in {} = {}", CurrentModuleObject, fanCoil.Name));
                        ShowContinueError(state, "..OutdoorAir:Mixer is required. Enter an OutdoorAir:Mixer object with this name.");
                        ErrorsFound = true;
                    } else {
                        fanCoil.OutsideAirNode = OANodeNums(1);
                        fanCoil.AirReliefNode = OANodeNums(2);
                        fanCoil.MixedAirNode = OANodeNums(4);
                    }
                }
            }

            fanCoil.CCoilName = Alphas(12);
            fanCoil.MaxColdWaterVolFlow = Numbers(5);
            fanCoil.MinColdWaterVolFlow = Numbers(6);
            fanCoil.ColdControlOffset = Numbers(7);
            fanCoil.HCoilName = Alphas(14);
            fanCoil.HCoilType = Alphas(13);
            fanCoil.MaxHotWaterVolFlow = Numbers(8);
            fanCoil.MinHotWaterVolFlow = Numbers(9);
            fanCoil.HotControlOffset = Numbers(10);

            if (Util::SameString(Alphas(11), "Coil:Cooling:Water") || Util::SameString(Alphas(11), "Coil:Cooling:Water:DetailedGeometry") ||
                Util::SameString(Alphas(11), "CoilSystem:Cooling:Water:HeatExchangerAssisted")) {
                fanCoil.CCoilType = Alphas(11);
                if (Util::SameString(Alphas(11), "Coil:Cooling:Water")) {
                    fanCoil.CCoilType_Num = CCoil::Water;
                    fanCoil.CCoilPlantName = fanCoil.CCoilName;
                    fanCoil.CCoilPlantType = DataPlant::PlantEquipmentType::CoilWaterCooling;
                }
                if (Util::SameString(Alphas(11), "Coil:Cooling:Water:DetailedGeometry")) {
                    fanCoil.CCoilType_Num = CCoil::Detailed;
                    fanCoil.CCoilPlantName = fanCoil.CCoilName;
                    fanCoil.CCoilPlantType = DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling;
                }
                std::string CCoilType;
                if (Util::SameString(Alphas(11), "CoilSystem:Cooling:Water:HeatExchangerAssisted")) {
                    fanCoil.CCoilType_Num = CCoil::HXAssist;
                    HVACHXAssistedCoolingCoil::GetHXCoilTypeAndName(
                        state, fanCoil.CCoilType, fanCoil.CCoilName, ErrorsFound, CCoilType, fanCoil.CCoilPlantName);
                    if (Util::SameString(CCoilType, "Coil:Cooling:Water")) {
                        fanCoil.CCoilPlantType = DataPlant::PlantEquipmentType::CoilWaterCooling;
                    } else if (Util::SameString(CCoilType, "Coil:Cooling:Water:DetailedGeometry")) {
                        fanCoil.CCoilPlantType = DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling;
                    } else {
                        ShowSevereError(state, format("{}{}=\"{}\", invalid", RoutineName, CurrentModuleObject, fanCoil.Name));
                        ShowContinueError(state, format("For: {}=\"{}\".", cAlphaFields(11), Alphas(11)));
                        ShowContinueError(state, format("Invalid Coil Type={}, Name={}", CCoilType, fanCoil.CCoilPlantName));
                        ShowContinueError(state, "must be \"Coil:Cooling:Water\" or \"Coil:Cooling:Water:DetailedGeometry\"");
                        ErrorsFound = true;
                    }
                }
                IsNotOK = false;
                ValidateComponent(state, fanCoil.CCoilType, fanCoil.CCoilName, IsNotOK, fanCoil.UnitType);
                if (IsNotOK) {
                    ShowContinueError(state, format("...specified in {}=\"{}\".", CurrentModuleObject, fanCoil.Name));
                    ErrorsFound = true;
                } else {
                    if (fanCoil.CCoilType_Num != CCoil::HXAssist) {
                        // mine the cold water node from the coil object
                        int coilIndex = WaterCoils::GetWaterCoilIndex(state, fanCoil.CCoilType, fanCoil.CCoilName, IsNotOK);
                        // Other error checks should trap before it gets to this point in the code, but including just in case.
                        if (IsNotOK) {
                            ShowContinueError(state, format("...specified in {}=\"{}\".", CurrentModuleObject, fanCoil.Name));
                            ErrorsFound = true;
                        } else {
                            fanCoil.CoolCoilFluidInletNode = state.dataWaterCoils->WaterCoil(coilIndex).WaterInletNodeNum;
                            fanCoil.CoolCoilInletNodeNum = state.dataWaterCoils->WaterCoil(coilIndex).AirInletNodeNum;
                            fanCoil.CoolCoilOutletNodeNum = state.dataWaterCoils->WaterCoil(coilIndex).AirOutletNodeNum;
                        }
                    } else {
                        // mine the cold water node from the coil object
                        int coilIndex = WaterCoils::GetWaterCoilIndex(state, CCoilType, fanCoil.CCoilPlantName, IsNotOK);
                        // Other error checks should trap before it gets to this point in the code, but including just in case.
                        if (IsNotOK) {
                            ShowContinueError(state, format("...specified in {}=\"{}\".", CurrentModuleObject, fanCoil.Name));
                            ErrorsFound = true;
                        } else {
                            fanCoil.CoolCoilFluidInletNode = state.dataWaterCoils->WaterCoil(coilIndex).WaterInletNodeNum;
                            fanCoil.CoolCoilInletNodeNum = state.dataWaterCoils->WaterCoil(coilIndex).AirInletNodeNum;
                            fanCoil.CoolCoilOutletNodeNum = state.dataWaterCoils->WaterCoil(coilIndex).AirOutletNodeNum;
                        }
                    }
                }
            } else {
                ShowSevereError(state, format("{}{}=\"{}\", invalid", RoutineName, CurrentModuleObject, fanCoil.Name));
                ShowContinueError(state, format("illegal value: {}=\"{}\".", cAlphaFields(11), Alphas(11)));
                ErrorsFound = true;
            }

            if (Util::SameString(Alphas(13), "Coil:Heating:Water")) {
                fanCoil.HCoilType_Num = HCoil::Water;
                fanCoil.HCoilPlantTypeOf = DataPlant::PlantEquipmentType::CoilWaterSimpleHeating;
                IsNotOK = false;
                ValidateComponent(state, fanCoil.HCoilType, fanCoil.HCoilName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...specified in {}=\"{}\".", CurrentModuleObject, fanCoil.Name));
                    ErrorsFound = true;
                } else {
                    // mine the hot water node from the coil object
                    int coilIndex = WaterCoils::GetWaterCoilIndex(state, fanCoil.HCoilType, fanCoil.HCoilName, IsNotOK);
                    if (IsNotOK) {
                        ShowContinueError(state, format("...specified in {}=\"{}\".", CurrentModuleObject, fanCoil.Name));
                        ErrorsFound = true;
                    } else {
                        fanCoil.HeatCoilFluidInletNode = state.dataWaterCoils->WaterCoil(coilIndex).WaterInletNodeNum;
                        fanCoil.HeatCoilInletNodeNum = state.dataWaterCoils->WaterCoil(coilIndex).AirInletNodeNum;
                        fanCoil.HeatCoilOutletNodeNum = state.dataWaterCoils->WaterCoil(coilIndex).AirOutletNodeNum;
                    }
                }
            } else if (Util::SameString(Alphas(13), "Coil:Heating:Electric")) {
                fanCoil.HCoilType_Num = HCoil::Electric;
                IsNotOK = false;
                ValidateComponent(state, fanCoil.HCoilType, fanCoil.HCoilName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...specified in {}=\"{}\".", CurrentModuleObject, fanCoil.Name));
                    ErrorsFound = true;
                } else {
                    int coilIndex;
                    HeatingCoils::GetCoilIndex(state, fanCoil.HCoilName, coilIndex, IsNotOK);
                    if (IsNotOK) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, fanCoil.Name));
                        ErrorsFound = true;
                    } else {
                        fanCoil.DesignHeatingCapacity = state.dataHeatingCoils->HeatingCoil(coilIndex).NominalCapacity;
                        fanCoil.HeatCoilInletNodeNum = state.dataHeatingCoils->HeatingCoil(coilIndex).AirInletNodeNum;
                        fanCoil.HeatCoilOutletNodeNum = state.dataHeatingCoils->HeatingCoil(coilIndex).AirOutletNodeNum;
                    }
                }
            } else {
                ShowSevereError(state, format("{}{}=\"{}\", invalid", RoutineName, CurrentModuleObject, fanCoil.Name));
                ShowContinueError(state, format("illegal value: {}=\"{}\".", cAlphaFields(13), Alphas(13)));
                ErrorsFound = true;
            }

            fanCoil.FanName = Alphas(10);

            if (!lAlphaBlanks(15)) {
                fanCoil.AvailManagerListName = Alphas(15);
            }

            fanCoil.HVACSizingIndex = 0;
            if (!lAlphaBlanks(16)) {
                fanCoil.HVACSizingIndex = Util::FindItemInList(Alphas(16), state.dataSize->ZoneHVACSizing);
                if (fanCoil.HVACSizingIndex == 0) {
                    ShowSevereError(state, format("{} = {} not found.", cAlphaFields(16), Alphas(16)));
                    ShowContinueError(state, format("Occurs in {} = {}", state.dataFanCoilUnits->cMO_FanCoil, fanCoil.Name));
                    ErrorsFound = true;
                }
            }

            fanCoil.fanType = static_cast<HVAC::FanType>(getEnumValue(HVAC::fanTypeNamesUC, Alphas(9)));
            if (fanCoil.fanType != HVAC::FanType::Constant && fanCoil.fanType != HVAC::FanType::VAV && fanCoil.fanType != HVAC::FanType::OnOff &&
                fanCoil.fanType != HVAC::FanType::SystemModel) {
                ShowSevereInvalidKey(state,
                                     eoh,
                                     cAlphaFields(9),
                                     Alphas(9),
                                     "Fan Type must be Fan:OnOff, Fan:ConstantVolume, Fan:VariableVolume, or Fan:SystemModel.");
                ErrorsFound = true;
            } else if ((fanCoil.FanIndex = Fans::GetFanIndex(state, fanCoil.FanName)) == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(10), Alphas(10));
                ErrorsFound = true;
            } else {
                auto *fan = state.dataFans->fans(fanCoil.FanIndex);

                if (fanCoil.fanType != fan->type) {
                    ShowSevereCustom(state,
                                     eoh,
                                     format("{} was specified as having type {}, but has type {}",
                                            fanCoil.FanName,
                                            HVAC::fanTypeNamesUC[(int)fanCoil.fanType],
                                            HVAC::fanTypeNamesUC[(int)fan->type]));
                    ErrorsFound = true;
                }

                fanCoil.fanAvailSched = fan->availSched;
                fanCoil.FanAirVolFlow = fan->maxAirFlowRate;

                if (fanCoil.MaxAirVolFlow > fanCoil.FanAirVolFlow && fanCoil.FanAirVolFlow != DataSizing::AutoSize) {
                    ShowWarningError(state, format("{}{}: {}", RoutineName, fanCoil.UnitType, fanCoil.Name));
                    ShowContinueError(state, format("... {} is greater than the maximum fan flow rate.", cNumericFields(1)));
                    ShowContinueError(state, format("... Fan Coil Unit flow = {:.5T} m3/s.", fanCoil.MaxAirVolFlow));
                    ShowContinueError(state, format("... Fan = {}: {}", HVAC::fanTypeNames[(int)fanCoil.fanType], fanCoil.FanName));
                    ShowContinueError(state, format("... Fan flow = {:.5T} m3/s.", fanCoil.FanAirVolFlow));
                    ShowContinueError(state, "... Fan Coil Unit flow rate reduced to match the fan flow rate and the simulation continues.");
                    fanCoil.MaxAirVolFlow = fanCoil.FanAirVolFlow;
                }

                if (fanCoil.fanType == HVAC::FanType::Constant || fanCoil.fanType == HVAC::FanType::VAV || fanCoil.fanType == HVAC::FanType::OnOff) {
                    // Get fan air volume flow rate
                    // Check that the fan volumetric flow rate is greater than or equal to the FCU volumetric flow rate

                    // Check that the fan type match with the capacity control method selected
                    if ((fanCoil.CapCtrlMeth_Num == CCM::ConsFanVarFlow && (fanCoil.fanType == HVAC::FanType::VAV)) ||
                        (fanCoil.CapCtrlMeth_Num == CCM::CycFan && fanCoil.fanType != HVAC::FanType::OnOff) ||
                        (fanCoil.CapCtrlMeth_Num == CCM::VarFanVarFlow && fanCoil.fanType != HVAC::FanType::VAV) ||
                        (fanCoil.CapCtrlMeth_Num == CCM::VarFanConsFlow && fanCoil.fanType != HVAC::FanType::VAV)) {
                        ShowSevereError(state, format("{}{}: {}", RoutineName, fanCoil.UnitType, fanCoil.Name));
                        ShowContinueError(state,
                                          format("...the fan type of the object : {} does not match with the capacity control method selected : "
                                                 "{} please see I/O reference",
                                                 fanCoil.FanName,
                                                 capCtrlMeth));
                        ShowContinueError(state, "...for ConstantFanVariableFlow a Fan:OnOff or Fan:ConstantVolume is valid.");
                        ShowContinueError(state, "...for CyclingFan a Fan:OnOff is valid.");
                        ShowContinueError(state, "...for VariableFanVariableFlow or VariableFanConstantFlow a Fan:VariableVolume is valid.");
                        ErrorsFound = true;
                    }
                } else {
                    // check that for VariableFanVariableFlow or VariableFanConstantFlow that the fan speed control is continuous
                    if (fanCoil.CapCtrlMeth_Num == CCM::VarFanVarFlow || fanCoil.CapCtrlMeth_Num == CCM::VarFanConsFlow ||
                        fanCoil.CapCtrlMeth_Num == CCM::ASHRAE) { // then expect continuous speed control fan
                        if (dynamic_cast<Fans::FanSystem *>(state.dataFans->fans(fanCoil.FanIndex))->speedControl != Fans::SpeedControl::Continuous) {
                            ShowSevereError(state, format("{}{}: {}", RoutineName, fanCoil.UnitType, fanCoil.Name));
                            ShowContinueError(state,
                                              format("...the fan type of the object : {} does not match with the capacity control method selected : "
                                                     "{} please see I/O reference",
                                                     fanCoil.FanName,
                                                     capCtrlMeth));
                            ShowContinueError(
                                state,
                                "...for VariableFanVariableFlow or VariableFanConstantFlow a Fan:SystemModel should have Continuous speed control.");
                            ErrorsFound = true;
                        }
                    }
                }
            }

            // check low speed fan ratio when using ASHRAE90.1 capacity control method
            if (fanCoil.CapCtrlMeth_Num == CCM::ASHRAE) {
                if (fanCoil.LowSpeedRatio > 0.5) {
                    ShowWarningError(state, format("{}{}=\"{}\",", RoutineName, CurrentModuleObject, fanCoil.Name));
                    ShowContinueError(state, format("... {} is greater than the 50% of the supply air flow ratio.", cNumericFields(2)));
                    ShowContinueError(state, format("... Fan Coil Unit low speed supply air flow ratio = {:.5T} ", fanCoil.LowSpeedRatio));
                } else if (fanCoil.LowSpeedRatio == 0.0) {
                    ShowWarningError(state, format("{}{}=\"{}\",", RoutineName, CurrentModuleObject, fanCoil.Name));
                    ShowContinueError(state, format("... {} is equal to 0.", cNumericFields(2)));
                    ShowContinueError(state, "... Fan Coil Unit low speed supply air flow ratio should be greater than 0 to comply with ASHRAE90.1.");
                    ShowContinueError(state, "... Fan Coil Unit low speed supply air flow ratio set to 0.5");
                    fanCoil.LowSpeedRatio = 0.5;
                }
            }

            // Set defaults for convergence tolerance
            if (fanCoil.ColdControlOffset <= 0.0) {
                fanCoil.ColdControlOffset = 0.001;
            }
            if (fanCoil.HotControlOffset <= 0.0) {
                fanCoil.HotControlOffset = 0.001;
            }

            // check for inlet side air mixer
            SingleDuct::GetATMixer(state,
                                   fanCoil.Name,
                                   ATMixerName,
                                   state.dataFanCoilUnits->ATMixerNum,
                                   state.dataFanCoilUnits->ATMixerType,
                                   state.dataFanCoilUnits->ATMixerPriNode,
                                   state.dataFanCoilUnits->ATMixerSecNode,
                                   state.dataFanCoilUnits->ATMixerOutNode,
                                   fanCoil.AirOutNode);
            fanCoil.ControlZoneNum =
                DataZoneEquipment::GetZoneEquipControlledZoneNum(state, DataZoneEquipment::ZoneEquipType::FourPipeFanCoil, fanCoil.Name);
            if (fanCoil.ControlZoneNum == 0) {
                ErrorsFound = true;
            }
            if (state.dataFanCoilUnits->ATMixerType == HVAC::MixerType::InletSide) {
                // save the air terminal mixer data in the fan coil data array
                fanCoil.ATMixerExists = true;
                fanCoil.ATMixerIndex = state.dataFanCoilUnits->ATMixerNum;
                fanCoil.ATMixerName = ATMixerName;
                fanCoil.ATMixerType = HVAC::MixerType::InletSide;
                fanCoil.ATMixerPriNode = state.dataFanCoilUnits->ATMixerPriNode;
                fanCoil.ATMixerSecNode = state.dataFanCoilUnits->ATMixerSecNode;
                fanCoil.ATMixerOutNode = state.dataFanCoilUnits->ATMixerOutNode;
                // check that fan coil doesn' have local outside air
                if (!lAlphaBlanks(8)) {
                    ShowSevereError(
                        state,
                        format("{} = \"{}\". Fan coil unit has local as well as central outdoor air specified", CurrentModuleObject, fanCoil.Name));
                }
                // check that the air teminal mixer out node is the fan coil inlet node
                if (fanCoil.AirInNode != state.dataFanCoilUnits->ATMixerOutNode) {
                    ShowSevereError(
                        state,
                        format("{} = \"{}\". Fan coil unit air inlet node name must be the same as an air terminal mixer outlet node name.",
                               CurrentModuleObject,
                               fanCoil.Name));
                    ShowContinueError(state, "..Air terminal mixer outlet node name is specified in AirTerminal:SingleDuct:InletSideMixer object.");
                    ShowContinueError(state, format("..Fan coil unit air inlet node name = {}", state.dataLoopNodes->NodeID(fanCoil.AirInNode)));
                    ErrorsFound = true;
                }
                // check for supply side air terminal mixer
            } else if (state.dataFanCoilUnits->ATMixerType == HVAC::MixerType::SupplySide) {
                // save the air terminal mixer data in the fan coil data array
                fanCoil.ATMixerExists = true;
                fanCoil.ATMixerIndex = state.dataFanCoilUnits->ATMixerNum;
                fanCoil.ATMixerName = ATMixerName;
                fanCoil.ATMixerType = HVAC::MixerType::SupplySide;
                fanCoil.ATMixerPriNode = state.dataFanCoilUnits->ATMixerPriNode;
                fanCoil.ATMixerSecNode = state.dataFanCoilUnits->ATMixerSecNode;
                fanCoil.ATMixerOutNode = state.dataFanCoilUnits->ATMixerOutNode;
                // check that fan coil doesn' have local outside air
                if (!lAlphaBlanks(8)) {
                    ShowSevereError(
                        state,
                        format("{} = \"{}\". Fan coil unit has local as well as central outdoor air specified", CurrentModuleObject, fanCoil.Name));
                }
                // check that the air teminal mixer secondary air inlet node is the fan coil outlet node
                if (fanCoil.AirOutNode != state.dataFanCoilUnits->ATMixerSecNode) {
                    ShowSevereError(state,
                                    format("{} = \"{}\". Fan coil unit air outlet node name must be the same as the air terminal mixer secondary air "
                                           "inlet node name.",
                                           CurrentModuleObject,
                                           fanCoil.Name));
                    ShowContinueError(
                        state, "..Air terminal mixer secondary inlet node name is specified in AirTerminal:SingleDuct:SupplySideMixer object.");
                    ShowContinueError(state, format("..Fan coil unit air outlet node name = {}", state.dataLoopNodes->NodeID(fanCoil.AirOutNode)));
                    ErrorsFound = true;
                }
                bool ZoneNodeNotFound = true;
                for (NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(fanCoil.ControlZoneNum).NumExhaustNodes; ++NodeNum) {
                    if (fanCoil.AirInNode == state.dataZoneEquip->ZoneEquipConfig(fanCoil.ControlZoneNum).ExhaustNode(NodeNum)) {
                        ZoneNodeNotFound = false;
                        break;
                    }
                }
                if (ZoneNodeNotFound) {
                    bool InletNodeFound = false;
                    if (fanCoil.ControlZoneNum > 0) {
                        InletNodeFound = ZonePlenum::ValidateInducedNode(state,
                                                                         fanCoil.AirInNode,
                                                                         state.dataZoneEquip->ZoneEquipConfig(fanCoil.ControlZoneNum).NumReturnNodes,
                                                                         state.dataZoneEquip->ZoneEquipConfig(fanCoil.ControlZoneNum).ReturnNode);
                    }
                    if (!InletNodeFound) {
                        ShowSevereError(state, format("{}{}=\"{}\"", RoutineName, CurrentModuleObject, fanCoil.Name));
                        ShowContinueError(state,
                                          "..FanCoil inlet node name must be the same as either a zone exhaust node name or an induced "
                                          "air node in ZonePlenum.");
                        ShowContinueError(state, "..Zone exhaust node name is specified in ZoneHVAC:EquipmentConnections object.");
                        ShowContinueError(state, "..Induced Air Outlet Node name is specified in AirLoopHVAC:ReturnPlenum object.");
                        ShowContinueError(state, format("..FanCoil inlet node name = {}", state.dataLoopNodes->NodeID(fanCoil.AirInNode)));
                        ErrorsFound = true;
                    }
                }
                // no air terminal mixer; do the normal connectivity checks
            } else {
                // check that the fan coil inlet node is the same as one of the zone exhaust nodes
                state.dataFanCoilUnits->ZoneExNodeNotFound = true;
                for (NodeNum = 1; NodeNum <= state.dataZoneEquip->ZoneEquipConfig(fanCoil.ControlZoneNum).NumExhaustNodes; ++NodeNum) {
                    if (fanCoil.AirInNode == state.dataZoneEquip->ZoneEquipConfig(fanCoil.ControlZoneNum).ExhaustNode(NodeNum)) {
                        state.dataFanCoilUnits->ZoneExNodeNotFound = false;
                        break;
                    }
                }
                if (state.dataFanCoilUnits->ZoneExNodeNotFound) {
                    bool InletNodeFound = false;
                    InletNodeFound = ZonePlenum::ValidateInducedNode(state,
                                                                     fanCoil.AirInNode,
                                                                     state.dataZoneEquip->ZoneEquipConfig(fanCoil.ControlZoneNum).NumReturnNodes,
                                                                     state.dataZoneEquip->ZoneEquipConfig(fanCoil.ControlZoneNum).ReturnNode);

                    if (!InletNodeFound) {
                        ShowSevereError(state,
                                        format("{} = \"{}\". Fan coil unit air inlet node name must be the same either as a zone exhaust node name "
                                               "or an induce air node in ZoePlenum.",
                                               CurrentModuleObject,
                                               fanCoil.Name));
                        ShowContinueError(state, "..Zone exhaust node name is specified in ZoneHVAC:EquipmentConnections object.");
                        ShowContinueError(state, "..Induced Air Outlet Node name is specified in AirLoopHVAC:ReturnPlenum object.");
                        ShowContinueError(state, format("..Fan coil unit air inlet node name = {}", state.dataLoopNodes->NodeID(fanCoil.AirInNode)));
                        ErrorsFound = true;
                    }
                }
                // check that the fan coil outlet node is the same as one of the zone inlet nodes
                state.dataFanCoilUnits->ZoneInNodeNotFound = true;
                if (fanCoil.ControlZoneNum > 0) {
                    fanCoil.NodeNumOfControlledZone = state.dataZoneEquip->ZoneEquipConfig(fanCoil.ControlZoneNum).ZoneNode;
                    state.dataFanCoilUnits->ZoneInNodeNotFound = false;
                }

                if (state.dataFanCoilUnits->ZoneInNodeNotFound) {
                    ShowSevereError(state,
                                    format("{} = \"{}\". Fan coil unit air outlet node name must be the same as a zone inlet node name.",
                                           CurrentModuleObject,
                                           fanCoil.Name));
                    ShowContinueError(state, "..Zone inlet node name is specified in ZoneHVAC:EquipmentConnections object.");
                    ShowContinueError(state, format("..Fan coil unit air outlet node name = {}", state.dataLoopNodes->NodeID(fanCoil.AirOutNode)));

                    ErrorsFound = true;
                }
            }
            if (fanCoil.CapCtrlMeth_Num == CCM::MultiSpeedFan) {
                if (!lAlphaBlanks(17)) {
                    fanCoil.fanOpModeSched = Sched::GetSchedule(state, Alphas(17));
                    if (fanCoil.fanType != HVAC::FanType::OnOff && fanCoil.fanType != HVAC::FanType::SystemModel) {
                        ShowSevereError(state, format("{} = {}", CurrentModuleObject, fanCoil.Name));
                        ShowContinueError(state, format("For {} = {}", cAlphaFields(17), Alphas(17)));
                        ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(9), Alphas(9)));
                        ShowContinueError(state, "...fan operating schedule is allowed for on off or system model fan type only )");
                        ErrorsFound = true;
                    } else if (fanCoil.fanOpModeSched == nullptr) {
                        ShowSevereItemNotFound(state, eoh, cAlphaFields(17), Alphas(17));
                        ErrorsFound = true;
                    }
                } else if (fanCoil.fanType == HVAC::FanType::OnOff || fanCoil.fanType == HVAC::FanType::SystemModel) {
                    fanCoil.fanOp = HVAC::FanOp::Cycling;
                }
            }

            if (!lNumericBlanks(11)) {
                fanCoil.DesignMinOutletTemp = Numbers(11);
                if (lNumericBlanks(12)) {
                    ShowWarningError(state, format("{}{}=\"{}\",", RoutineName, CurrentModuleObject, fanCoil.Name));
                    ShowContinueError(state, format("... {} and {} must be used in unison.", cNumericFields(11), cNumericFields(12)));
                    ErrorsFound = true;
                }
            }

            if (!lNumericBlanks(12)) {
                fanCoil.DesignMaxOutletTemp = Numbers(12);
                if (fanCoil.DesignMinOutletTemp != DataSizing::AutoSize && fanCoil.DesignMaxOutletTemp != DataSizing::AutoSize) {
                    if (fanCoil.DesignMaxOutletTemp < fanCoil.DesignMinOutletTemp) {
                        ShowWarningError(state, format("{}{}=\"{}\",", RoutineName, CurrentModuleObject, fanCoil.Name));
                        ShowContinueError(state, format("... {} is greater than {}.", cNumericFields(11), cNumericFields(12)));
                        ShowContinueError(state, format("... {} = {:.2T} [C].", cNumericFields(11), fanCoil.DesignMinOutletTemp));
                        ShowContinueError(state, format("... {} = {:.2T} [C].", cNumericFields(12), fanCoil.DesignMaxOutletTemp));
                        ErrorsFound = true;
                    }
                }
                if (lNumericBlanks(11)) {
                    ShowWarningError(state, format("{}{}=\"{}\",", RoutineName, CurrentModuleObject, fanCoil.Name));
                    ShowContinueError(state, format("... {} and {} must be used in unison.", cNumericFields(11), cNumericFields(12)));
                    ErrorsFound = true;
                }
            }

            if (fanCoil.DesignMinOutletTemp > 0.0 && fanCoil.DesignMaxOutletTemp > 0.0) {
                fanCoil.ASHRAETempControl = true;
            } else if (fanCoil.DesignMinOutletTemp == DataSizing::AutoSize || fanCoil.DesignMaxOutletTemp == DataSizing::AutoSize) {
                fanCoil.ASHRAETempControl = true;
            }

            // Set up component set for supply fan
            if (fanCoil.OutsideAirNode > 0) {
                BranchNodeConnections::SetUpCompSets(state,
                                                     fanCoil.UnitType,
                                                     fanCoil.Name,
                                                     HVAC::fanTypeNames[(int)fanCoil.fanType],
                                                     fanCoil.FanName,
                                                     state.dataLoopNodes->NodeID(fanCoil.MixedAirNode),
                                                     "UNDEFINED");
            } else {
                BranchNodeConnections::SetUpCompSets(state,
                                                     fanCoil.UnitType,
                                                     fanCoil.Name,
                                                     HVAC::fanTypeNames[(int)fanCoil.fanType],
                                                     fanCoil.FanName,
                                                     state.dataLoopNodes->NodeID(fanCoil.AirInNode),
                                                     "UNDEFINED");
            }
            // Set up component set for cooling coil
            BranchNodeConnections::SetUpCompSets(
                state, fanCoil.UnitType, fanCoil.Name, fanCoil.CCoilType, fanCoil.CCoilName, "UNDEFINED", "UNDEFINED");

            // Set up component set for heating coil
            BranchNodeConnections::SetUpCompSets(state,
                                                 fanCoil.UnitType,
                                                 fanCoil.Name,
                                                 fanCoil.HCoilType,
                                                 fanCoil.HCoilName,
                                                 "UNDEFINED",
                                                 state.dataLoopNodes->NodeID(fanCoil.AirOutNode));

            // Set up component set for OA mixer - use OA node and Mixed air node
            if (fanCoil.OutsideAirNode > 0) {
                BranchNodeConnections::SetUpCompSets(state,
                                                     fanCoil.UnitType,
                                                     fanCoil.Name,
                                                     fanCoil.OAMixType,
                                                     fanCoil.OAMixName,
                                                     state.dataLoopNodes->NodeID(fanCoil.OutsideAirNode),
                                                     state.dataLoopNodes->NodeID(fanCoil.MixedAirNode));
            }
        }

        Alphas.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        Numbers.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();

        if (ErrorsFound) {
            ShowFatalError(state, format("{}Errors found in input. Preceding condition(s) cause termination.", RoutineName));
        }

        for (auto &fanCoil : state.dataFanCoilUnits->FanCoil) {
            // Setup Report variables for the Fan Coils
            // CurrentModuleObject='ZoneHVAC:FourPipeFanCoil'
            SetupOutputVariable(state,
                                "Fan Coil Heating Rate",
                                Constant::Units::W,
                                fanCoil.HeatPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                fanCoil.Name);
            SetupOutputVariable(state,
                                "Fan Coil Heating Energy",
                                Constant::Units::J,
                                fanCoil.HeatEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                fanCoil.Name);
            SetupOutputVariable(state,
                                "Fan Coil Total Cooling Rate",
                                Constant::Units::W,
                                fanCoil.TotCoolPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                fanCoil.Name);
            SetupOutputVariable(state,
                                "Fan Coil Total Cooling Energy",
                                Constant::Units::J,
                                fanCoil.TotCoolEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                fanCoil.Name);
            SetupOutputVariable(state,
                                "Fan Coil Sensible Cooling Rate",
                                Constant::Units::W,
                                fanCoil.SensCoolPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                fanCoil.Name);
            SetupOutputVariable(state,
                                "Fan Coil Sensible Cooling Energy",
                                Constant::Units::J,
                                fanCoil.SensCoolEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                fanCoil.Name);
            SetupOutputVariable(state,
                                "Fan Coil Fan Electricity Rate",
                                Constant::Units::W,
                                fanCoil.ElecPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                fanCoil.Name);
            SetupOutputVariable(state,
                                "Fan Coil Fan Electricity Energy",
                                Constant::Units::J,
                                fanCoil.ElecEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                fanCoil.Name);
            if (fanCoil.CapCtrlMeth_Num == CCM::CycFan || fanCoil.CapCtrlMeth_Num == CCM::MultiSpeedFan) {
                SetupOutputVariable(state,
                                    "Fan Coil Runtime Fraction",
                                    Constant::Units::None,
                                    fanCoil.PLR,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    fanCoil.Name);
                SetupOutputVariable(state,
                                    "Fan Coil Fan Speed Level",
                                    Constant::Units::None,
                                    fanCoil.SpeedFanSel,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    fanCoil.Name);
                if (fanCoil.CapCtrlMeth_Num == CCM::MultiSpeedFan) {
                    SetupOutputVariable(state,
                                        "Fan Coil Speed Ratio",
                                        Constant::Units::None,
                                        fanCoil.SpeedRatio,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        fanCoil.Name);
                    SetupOutputVariable(state,
                                        "Fan Coil Part Load Ratio",
                                        Constant::Units::None,
                                        fanCoil.PLR,
                                        OutputProcessor::TimeStepType::System,
                                        OutputProcessor::StoreType::Average,
                                        fanCoil.Name);
                }
            }
            if (fanCoil.CapCtrlMeth_Num == CCM::VarFanVarFlow || fanCoil.CapCtrlMeth_Num == CCM::VarFanConsFlow) {
                SetupOutputVariable(state,
                                    "Fan Coil Part Load Ratio",
                                    Constant::Units::None,
                                    fanCoil.PLR,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    fanCoil.Name);
            }
            SetupOutputVariable(state,
                                "Fan Coil Availability Status",
                                Constant::Units::None,
                                fanCoil.availStatus,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                fanCoil.Name);

            state.dataRptCoilSelection->coilSelectionReportObj->setCoilSupplyFanInfo(
                state, fanCoil.CCoilName, fanCoil.CCoilType, fanCoil.FanName, fanCoil.fanType, fanCoil.FanIndex);
            state.dataRptCoilSelection->coilSelectionReportObj->setCoilSupplyFanInfo(
                state, fanCoil.HCoilName, fanCoil.HCoilType, fanCoil.FanName, fanCoil.fanType, fanCoil.FanIndex);
        }
    }

    void InitFanCoilUnits(EnergyPlusData &state,
                          int const FanCoilNum,       // number of the current fan coil unit being simulated
                          int const ControlledZoneNum // number of zone being served
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   March 2000
        //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the Fan Coil Components.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("InitFanCoilUnits");

        auto &fanCoil = state.dataFanCoilUnits->FanCoil(FanCoilNum);

        // Do the one time initializations
        if (state.dataFanCoilUnits->InitFanCoilUnitsOneTimeFlag) {

            state.dataFanCoilUnits->MyEnvrnFlag.allocate(state.dataFanCoilUnits->NumFanCoils);
            state.dataFanCoilUnits->MySizeFlag.allocate(state.dataFanCoilUnits->NumFanCoils);
            state.dataFanCoilUnits->MyPlantScanFlag.allocate(state.dataFanCoilUnits->NumFanCoils);
            state.dataFanCoilUnits->MyZoneEqFlag.allocate(state.dataFanCoilUnits->NumFanCoils);
            state.dataFanCoilUnits->MyEnvrnFlag = true;
            state.dataFanCoilUnits->MySizeFlag = true;
            state.dataFanCoilUnits->MyPlantScanFlag = true;
            state.dataFanCoilUnits->MyZoneEqFlag = true;
            state.dataFanCoilUnits->InitFanCoilUnitsOneTimeFlag = false;
        }

        if (allocated(state.dataAvail->ZoneComp)) {
            auto &availMgr = state.dataAvail->ZoneComp(DataZoneEquipment::ZoneEquipType::FourPipeFanCoil).ZoneCompAvailMgrs(FanCoilNum);
            if (state.dataFanCoilUnits->MyZoneEqFlag(FanCoilNum)) { // initialize the name of each availability manager list and zone number
                availMgr.AvailManagerListName = fanCoil.AvailManagerListName;
                availMgr.ZoneNum = ControlledZoneNum;
                state.dataFanCoilUnits->MyZoneEqFlag(FanCoilNum) = false;
            }
            fanCoil.availStatus = availMgr.availStatus;
        }

        if (state.dataFanCoilUnits->MyPlantScanFlag(FanCoilNum) && allocated(state.dataPlnt->PlantLoop)) {
            bool errFlag = false;
            if (fanCoil.HCoilType_Num == HCoil::Water) {
                PlantUtilities::ScanPlantLoopsForObject(
                    state, fanCoil.HCoilName, fanCoil.HCoilPlantTypeOf, fanCoil.HeatCoilPlantLoc, errFlag, _, _, _, _, _);

                if (errFlag) {
                    ShowContinueError(state, format("Reference Unit=\"{}\", type={}", fanCoil.Name, fanCoil.UnitType));
                    ShowFatalError(state, "InitFanCoilUnits: Program terminated for previous conditions.");
                }

                fanCoil.HeatCoilFluidOutletNodeNum = DataPlant::CompData::getPlantComponent(state, fanCoil.HeatCoilPlantLoc).NodeNumOut;

            } else if (fanCoil.HCoilType_Num == HCoil::Electric) {
                // do nothing, valid type
            } else {
                ShowFatalError(state, format("InitFanCoilUnits: FanCoil={}, invalid heating coil type. Program terminated.", fanCoil.Name));
            }

            if ((fanCoil.CCoilPlantType == DataPlant::PlantEquipmentType::CoilWaterCooling) ||
                (fanCoil.CCoilPlantType == DataPlant::PlantEquipmentType::CoilWaterDetailedFlatCooling)) {
                PlantUtilities::ScanPlantLoopsForObject(
                    state, fanCoil.CCoilPlantName, fanCoil.CCoilPlantType, fanCoil.CoolCoilPlantLoc, errFlag, _, _, _, _, _);
                if (errFlag) {
                    ShowContinueError(state, format("Reference Unit=\"{}\", type={}", fanCoil.Name, fanCoil.UnitType));
                    ShowFatalError(state, "InitFanCoilUnits: Program terminated for previous conditions.");
                }
                fanCoil.CoolCoilFluidOutletNodeNum = DataPlant::CompData::getPlantComponent(state, fanCoil.CoolCoilPlantLoc).NodeNumOut;
            } else {
                ShowFatalError(state, format("InitFanCoilUnits: FanCoil={}, invalid cooling coil type. Program terminated.", fanCoil.Name));
            }

            state.dataFanCoilUnits->MyPlantScanFlag(FanCoilNum) = false;
        }

        if (!state.dataFanCoilUnits->InitFanCoilUnitsCheckInZoneEquipmentListFlag && state.dataZoneEquip->ZoneEquipInputsFilled) {
            state.dataFanCoilUnits->InitFanCoilUnitsCheckInZoneEquipmentListFlag = true;
            for (int Loop = 1; Loop <= state.dataFanCoilUnits->NumFanCoils; ++Loop) {
                if (DataZoneEquipment::CheckZoneEquipmentList(
                        state, state.dataFanCoilUnits->FanCoil(Loop).UnitType, state.dataFanCoilUnits->FanCoil(Loop).Name)) {
                    continue;
                }
                ShowSevereError(state,
                                format("InitFanCoil: FanCoil Unit=[{},{}] is not on any ZoneHVAC:EquipmentList.  It will not be simulated.",
                                       state.dataFanCoilUnits->FanCoil(Loop).UnitType,
                                       state.dataFanCoilUnits->FanCoil(Loop).Name));
            }
        }

        if (!state.dataGlobal->SysSizingCalc && state.dataFanCoilUnits->MySizeFlag(FanCoilNum) &&
            !state.dataFanCoilUnits->MyPlantScanFlag(FanCoilNum)) {

            SizeFanCoilUnit(state, FanCoilNum, ControlledZoneNum);

            state.dataFanCoilUnits->MySizeFlag(FanCoilNum) = false;
        }

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && state.dataFanCoilUnits->MyEnvrnFlag(FanCoilNum) &&
            !state.dataFanCoilUnits->MyPlantScanFlag(FanCoilNum)) {
            Real64 RhoAir = state.dataEnvrn->StdRhoAir;
            // set the mass flow rates from the input volume flow rates
            fanCoil.MaxAirMassFlow = RhoAir * fanCoil.MaxAirVolFlow;
            fanCoil.OutAirMassFlow = RhoAir * fanCoil.OutAirVolFlow;

            if (fanCoil.HCoilType_Num == HCoil::Water) {
                Real64 rho =
                    state.dataPlnt->PlantLoop(fanCoil.HeatCoilPlantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                fanCoil.MaxHeatCoilFluidFlow = rho * fanCoil.MaxHotWaterVolFlow;
                fanCoil.MinHotWaterFlow = rho * fanCoil.MinHotWaterVolFlow;
            }

            Real64 rho = state.dataPlnt->PlantLoop(fanCoil.CoolCoilPlantLoc.loopNum).glycol->getDensity(state, Constant::CWInitConvTemp, RoutineName);
            fanCoil.MaxCoolCoilFluidFlow = rho * fanCoil.MaxColdWaterVolFlow;
            fanCoil.MinColdWaterFlow = rho * fanCoil.MinColdWaterVolFlow;

            // set the node max and min mass flow rates
            if (fanCoil.HCoilType_Num == HCoil::Water) {
                PlantUtilities::InitComponentNodes(
                    state, fanCoil.MinHotWaterFlow, fanCoil.MaxHeatCoilFluidFlow, fanCoil.HeatCoilFluidInletNode, fanCoil.HeatCoilFluidOutletNodeNum);
            }

            PlantUtilities::InitComponentNodes(
                state, fanCoil.MinColdWaterFlow, fanCoil.MaxCoolCoilFluidFlow, fanCoil.CoolCoilFluidInletNode, fanCoil.CoolCoilFluidOutletNodeNum);

            if (fanCoil.OutsideAirNode > 0) {
                state.dataLoopNodes->Node(fanCoil.OutsideAirNode).MassFlowRateMax = fanCoil.OutAirMassFlow;
                state.dataLoopNodes->Node(fanCoil.OutsideAirNode).MassFlowRateMin = 0.0;
            }
            state.dataLoopNodes->Node(fanCoil.AirOutNode).MassFlowRateMax = fanCoil.MaxAirMassFlow;
            state.dataLoopNodes->Node(fanCoil.AirOutNode).MassFlowRateMin = 0.0;
            state.dataLoopNodes->Node(fanCoil.AirInNode).MassFlowRateMax = fanCoil.MaxAirMassFlow;
            state.dataLoopNodes->Node(fanCoil.AirInNode).MassFlowRateMin = 0.0;
            state.dataFanCoilUnits->MyEnvrnFlag(FanCoilNum) = false;
        } // end one time inits

        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataFanCoilUnits->MyEnvrnFlag(FanCoilNum) = true;
        }

        // These initializations are done every iteration
        fanCoil.SpeedRatio = 0.0;
        if (fanCoil.fanOpModeSched != nullptr) {
            fanCoil.fanOp = (fanCoil.fanOpModeSched->getCurrentVal() == 0.0) ? HVAC::FanOp::Cycling : HVAC::FanOp::Continuous;
        }
        // Set the inlet node mass flow rate
        if (((fanCoil.availSched->getCurrentVal() > 0.0 && fanCoil.fanAvailSched->getCurrentVal() > 0.0) || state.dataHVACGlobal->TurnFansOn) &&
            !state.dataHVACGlobal->TurnFansOff) {
            state.dataLoopNodes->Node(fanCoil.AirInNode).MassFlowRate = fanCoil.MaxAirMassFlow;
            state.dataLoopNodes->Node(fanCoil.AirInNode).MassFlowRateMaxAvail = state.dataLoopNodes->Node(fanCoil.AirInNode).MassFlowRate;
            state.dataLoopNodes->Node(fanCoil.AirInNode).MassFlowRateMinAvail = 0.0;

            if (fanCoil.OutsideAirNode > 0) {
                state.dataLoopNodes->Node(fanCoil.OutsideAirNode).MassFlowRate = fanCoil.OutAirMassFlow;
                state.dataLoopNodes->Node(fanCoil.OutsideAirNode).MassFlowRateMaxAvail = fanCoil.OutAirMassFlow;
                state.dataLoopNodes->Node(fanCoil.OutsideAirNode).MassFlowRateMinAvail = fanCoil.OutAirMassFlow;
                state.dataLoopNodes->Node(fanCoil.AirReliefNode).MassFlowRate = fanCoil.OutAirMassFlow;
                state.dataLoopNodes->Node(fanCoil.AirReliefNode).MassFlowRateMaxAvail = fanCoil.OutAirMassFlow;
                state.dataLoopNodes->Node(fanCoil.AirReliefNode).MassFlowRateMinAvail = fanCoil.OutAirMassFlow;
            }

        } else {
            state.dataLoopNodes->Node(fanCoil.AirInNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(fanCoil.AirInNode).MassFlowRateMaxAvail = 0.0;
            state.dataLoopNodes->Node(fanCoil.AirInNode).MassFlowRateMinAvail = 0.0;
            if (fanCoil.OutsideAirNode > 0) {
                state.dataLoopNodes->Node(fanCoil.OutsideAirNode).MassFlowRate = 0.0;
                state.dataLoopNodes->Node(fanCoil.OutsideAirNode).MassFlowRateMaxAvail = 0.0;
                state.dataLoopNodes->Node(fanCoil.OutsideAirNode).MassFlowRateMinAvail = 0.0;
                state.dataLoopNodes->Node(fanCoil.AirReliefNode).MassFlowRate = 0.0;
                state.dataLoopNodes->Node(fanCoil.AirReliefNode).MassFlowRateMaxAvail = 0.0;
                state.dataLoopNodes->Node(fanCoil.AirReliefNode).MassFlowRateMinAvail = 0.0;
            }
        }
    }

    void SizeFanCoilUnit(EnergyPlusData &state,
                         int const FanCoilNum,
                         int const ControlledZoneNum // index into ZoneEquipConfig array
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   January 2002
        //       MODIFIED       August 2013 Daeho Kang, add component sizing table entries
        //                      July 2014, B. Nigusse, added scalable sizing

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing Fan Coil Unit components for which flow rates have not been
        // specified in the input.

        // METHODOLOGY EMPLOYED:
        // Obtains flow rates from the zone or system sizing arrays and plant sizing data.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("SizeFanCoilUnit: "); // include trailing blank space
        static constexpr std::string_view RoutineNameNoSpace("SizeFanCoilUnit");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 DesCoilLoad; // coil load used for sizing [W]
        std::string CoolingCoilName;
        std::string CoolingCoilType;
        Real64 rho;
        Real64 Cp;
        int zoneHVACIndex;        // index of zoneHVAC equipment sizing specification
        std::string SizingString; // input field sizing description (e.g., Nominal Capacity)
        Real64 TempSize;          // autosized value of coil input field
        int SizingMethod; // Integer representation of sizing method name (e.g., CoolingAirflowSizing, HeatingAirflowSizing, CoolingCapacitySizing,
                          // HeatingCapacitySizing, etc.)
        bool PrintFlag;   // TRUE when sizing information is reported in the eio file
                          // FractionOfAutosizedHeatingAirflow ...)
        Real64 WaterCoilSizDeltaT; // water coil deltaT for design water flow rate autosizing
        int CoilNum;               // index of water coil object

        bool ErrorsFound = false;             // TRUE if errors found during sizing
        bool IsAutoSize = false;              // Indicator to autosize for reporting
        Real64 MaxAirVolFlowDes = 0.0;        // Autosized max air flow for reporting
        Real64 MaxAirVolFlowUser = 0.0;       // Hardsized max air flow for reporting
        Real64 OutAirVolFlowDes = 0.0;        // Autosized outdoor air flow for reporting
        Real64 OutAirVolFlowUser = 0.0;       // Hardsized outdoor air flow for reporting
        Real64 MaxHotWaterVolFlowDes = 0.0;   // Autosized hot water flow for reporting
        Real64 MaxHotWaterVolFlowUser = 0.0;  // Hardsized hot water flow for reporting
        Real64 MaxColdWaterVolFlowDes = 0.0;  // Autosized cold water flow for reporting
        Real64 MaxColdWaterVolFlowUser = 0.0; // Hardsized cold water flow for reporting
        Real64 CoolingAirVolFlowDes = 0.0;    // cooling supply air flow rate
        Real64 HeatingAirVolFlowDes = 0.0;    // heating supply air flow rate

        state.dataSize->ZoneHeatingOnlyFan = false;
        state.dataSize->ZoneCoolingOnlyFan = false;
        state.dataSize->DataScalableSizingON = false;
        state.dataSize->DataScalableCapSizingON = false;

        state.dataSize->DataFracOfAutosizedCoolingAirflow = 1.0;
        state.dataSize->DataFracOfAutosizedHeatingAirflow = 1.0;
        state.dataSize->DataFracOfAutosizedCoolingCapacity = 1.0;
        state.dataSize->DataFracOfAutosizedHeatingCapacity = 1.0;

        auto &fanCoil = state.dataFanCoilUnits->FanCoil(FanCoilNum);

        std::string CompType = fanCoil.UnitType;
        std::string CompName = fanCoil.Name;
        state.dataSize->DataZoneNumber = fanCoil.ControlZoneNum;
        state.dataSize->DataFanType = fanCoil.fanType;
        state.dataSize->DataFanIndex = fanCoil.FanIndex;
        // fan coil unit is always blow thru
        state.dataSize->DataFanPlacement = HVAC::FanPlace::BlowThru;

        auto &zoneEqSizing = state.dataSize->ZoneEqSizing(state.dataSize->CurZoneEqNum);

        if (state.dataSize->CurZoneEqNum > 0) {
            if (fanCoil.HVACSizingIndex > 0) {

                // initialize OA flow for sizing other inputs (e.g., inlet temp, capacity, etc.)
                if (fanCoil.OutAirVolFlow == DataSizing::AutoSize) {
                    zoneEqSizing.OAVolFlow = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).MinOA;
                } else {
                    zoneEqSizing.OAVolFlow = fanCoil.OutAirVolFlow;
                }
                if (fanCoil.ATMixerExists) {      // set up ATMixer conditions for scalable capacity sizing
                    zoneEqSizing.OAVolFlow = 0.0; // Equipment OA flow should always be 0 when ATMixer is used
                    SingleDuct::setATMixerSizingProperties(state, fanCoil.ATMixerIndex, ControlledZoneNum, state.dataSize->CurZoneEqNum);
                }

                zoneHVACIndex = fanCoil.HVACSizingIndex;
                int FieldNum = 1; // IDD numeric field number where input field description is found
                PrintFlag = true;
                int SAFMethod; // supply air flow rate sizing method (SupplyAirFlowRate, FlowPerFloorArea, FractionOfAutosizedCoolingAirflow,
                SizingString = state.dataFanCoilUnits->FanCoilNumericFields(FanCoilNum).FieldNames(FieldNum) + " [m3/s]";
                if (state.dataGlobal->isEpJSON) {
                    SizingString = "maximum_supply_air_flow_rate [m3/s]";
                }
                if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).CoolingSAFMethod > 0) {
                    SizingMethod = HVAC::CoolingAirflowSizing;
                    SAFMethod = state.dataSize->ZoneHVACSizing(zoneHVACIndex).CoolingSAFMethod;
                    zoneEqSizing.SizingMethod(SizingMethod) = SAFMethod;
                    if (SAFMethod == DataSizing::SupplyAirFlowRate || SAFMethod == DataSizing::FlowPerFloorArea ||
                        SAFMethod == DataSizing::FractionOfAutosizedCoolingAirflow) {
                        if (SAFMethod == DataSizing::SupplyAirFlowRate) {
                            if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow > 0.0) {
                                zoneEqSizing.AirVolFlow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                                zoneEqSizing.SystemAirFlow = true;
                            }
                            TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                        } else if (SAFMethod == DataSizing::FlowPerFloorArea) {
                            zoneEqSizing.SystemAirFlow = true;
                            zoneEqSizing.AirVolFlow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow *
                                                      state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                            TempSize = zoneEqSizing.AirVolFlow;
                            state.dataSize->DataScalableSizingON = true;
                        } else if (SAFMethod == DataSizing::FractionOfAutosizedCoolingAirflow) {
                            state.dataSize->DataFracOfAutosizedCoolingAirflow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                            TempSize = DataSizing::AutoSize;
                            state.dataSize->DataScalableSizingON = true;
                        } else {
                            TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                        }
                        CoolingAirFlowSizer sizingCoolingAirFlow;
                        sizingCoolingAirFlow.overrideSizingString(SizingString);
                        // sizingCoolingAirFlow.setHVACSizingIndexData(fanCoil.HVACSizingIndex);
                        sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        CoolingAirVolFlowDes = sizingCoolingAirFlow.size(state, TempSize, ErrorsFound);

                    } else if (SAFMethod == DataSizing::FlowPerCoolingCapacity) {
                        SizingMethod = HVAC::CoolingCapacitySizing;
                        TempSize = DataSizing::AutoSize;
                        PrintFlag = false;
                        CoolingCapacitySizer sizerCoolingCapacity;
                        sizerCoolingCapacity.overrideSizingString(SizingString);
                        sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        state.dataSize->DataAutosizedCoolingCapacity = sizerCoolingCapacity.size(state, TempSize, ErrorsFound);
                        if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).CoolingCapMethod == DataSizing::FractionOfAutosizedCoolingCapacity) {
                            state.dataSize->DataFracOfAutosizedCoolingCapacity = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity;
                        }
                        state.dataSize->DataFlowPerCoolingCapacity = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow;
                        PrintFlag = true;
                        TempSize = DataSizing::AutoSize;
                        state.dataSize->DataScalableSizingON = true;
                        CoolingAirFlowSizer sizingCoolingAirFlow;
                        sizingCoolingAirFlow.overrideSizingString(SizingString);
                        // sizingCoolingAirFlow.setHVACSizingIndexData(fanCoil.HVACSizingIndex);
                        sizingCoolingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        CoolingAirVolFlowDes = sizingCoolingAirFlow.size(state, TempSize, ErrorsFound);
                    }
                } else if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).HeatingSAFMethod > 0) {
                    // now do heating supply air flow rate sizing
                    SizingMethod = HVAC::HeatingAirflowSizing;
                    SAFMethod = state.dataSize->ZoneHVACSizing(zoneHVACIndex).HeatingSAFMethod;
                    zoneEqSizing.SizingMethod(SizingMethod) = SAFMethod;
                    if (SAFMethod == DataSizing::SupplyAirFlowRate || SAFMethod == DataSizing::FlowPerFloorArea ||
                        SAFMethod == DataSizing::FractionOfAutosizedHeatingAirflow) {
                        if (SAFMethod == DataSizing::SupplyAirFlowRate) {
                            if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow > 0.0) {
                                zoneEqSizing.AirVolFlow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                                zoneEqSizing.SystemAirFlow = true;
                            }
                            TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                        } else if (SAFMethod == DataSizing::FlowPerFloorArea) {
                            zoneEqSizing.SystemAirFlow = true;
                            zoneEqSizing.AirVolFlow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow *
                                                      state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                            TempSize = zoneEqSizing.AirVolFlow;
                            state.dataSize->DataScalableSizingON = true;
                        } else if (SAFMethod == DataSizing::FractionOfAutosizedHeatingAirflow) {
                            state.dataSize->DataFracOfAutosizedHeatingAirflow = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                            TempSize = DataSizing::AutoSize;
                            state.dataSize->DataScalableSizingON = true;
                        } else {
                            TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow;
                        }
                        bool errorsFound = false;
                        HeatingAirFlowSizer sizingHeatingAirFlow;
                        sizingHeatingAirFlow.overrideSizingString(SizingString);
                        // sizingHeatingAirFlow.setHVACSizingIndexData(fanCoil.HVACSizingIndex);
                        sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        HeatingAirVolFlowDes = sizingHeatingAirFlow.size(state, TempSize, errorsFound);
                    } else if (SAFMethod == DataSizing::FlowPerHeatingCapacity) {
                        SizingMethod = HVAC::HeatingCapacitySizing;
                        TempSize = DataSizing::AutoSize;
                        PrintFlag = false;
                        state.dataSize->DataScalableSizingON = true;
                        // initialize OA flow for sizing capacity
                        if (fanCoil.OutAirVolFlow == DataSizing::AutoSize) {
                            zoneEqSizing.OAVolFlow = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).MinOA;
                        } else {
                            zoneEqSizing.OAVolFlow = fanCoil.OutAirVolFlow;
                        }
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
                        // sizingHeatingAirFlow.setHVACSizingIndexData(fanCoil.HVACSizingIndex);
                        sizingHeatingAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                        HeatingAirVolFlowDes = sizingHeatingAirFlow.size(state, TempSize, errorsFound);
                    }
                }

                if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxCoolAirVolFlow == DataSizing::AutoSize ||
                    state.dataSize->ZoneHVACSizing(zoneHVACIndex).MaxHeatAirVolFlow == DataSizing::AutoSize) {
                    IsAutoSize = true;
                    fanCoil.MaxAirVolFlow = DataSizing::AutoSize;
                    MaxAirVolFlowDes = max(CoolingAirVolFlowDes, HeatingAirVolFlowDes);
                } else {
                    fanCoil.MaxAirVolFlow = max(CoolingAirVolFlowDes, HeatingAirVolFlowDes);
                    MaxAirVolFlowDes = 0.0;
                }
            } else {
                // SizingString = "Supply Air Maximum Flow Rate [m3/s]";
                TempSize = fanCoil.MaxAirVolFlow;
                PrintFlag = true;
                if (fanCoil.MaxAirVolFlow == DataSizing::AutoSize) {
                    IsAutoSize = true;
                    SystemAirFlowSizer sizerSystemAirFlow;
                    // sizerSystemAirFlow.setHVACSizingIndexData(fanCoil.HVACSizingIndex);
                    sizerSystemAirFlow.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                    MaxAirVolFlowDes = sizerSystemAirFlow.size(state, TempSize, ErrorsFound);
                } else {
                    MaxAirVolFlowDes = 0.0;
                }
            }
        }

        if (state.dataSize->CurZoneEqNum > 0) {
            if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) {

            } else {
                if (MaxAirVolFlowDes < HVAC::SmallAirVolFlow) {
                    MaxAirVolFlowDes = 0.0;
                }

                //     If fan is autosized, get fan volumetric flow rate
                if (fanCoil.FanAirVolFlow == DataSizing::AutoSize) {
                    state.dataFans->fans(fanCoil.FanIndex)->simulate(state, true, _, _);
                    fanCoil.FanAirVolFlow = state.dataFans->fans(fanCoil.FanIndex)->maxAirFlowRate;
                }
                //     Check that the fan volumetric flow rate is greater than or equal to the FCU volumetric flow rate
                if (MaxAirVolFlowDes > fanCoil.FanAirVolFlow) {
                    ShowWarningError(state, format("{}{}: {}", RoutineName, fanCoil.UnitType, fanCoil.Name));
                    ShowContinueError(state, "... Maximum supply air flow rate is greater than the maximum fan flow rate.");
                    ShowContinueError(state, format("... Fan Coil Unit flow = {:.5T} [m3/s].", MaxAirVolFlowDes));
                    ShowContinueError(state, format("... Fan = {}: {}", HVAC::fanTypeNames[(int)fanCoil.fanType], fanCoil.FanName));
                    ShowContinueError(state, format("... Fan flow = {:.5T} [m3/s].", fanCoil.FanAirVolFlow));
                    ShowContinueError(state, "... Fan Coil Unit flow rate reduced to match the fan flow rate and the simulation continues.");
                    MaxAirVolFlowDes = fanCoil.FanAirVolFlow;
                }

                if (IsAutoSize) {
                    fanCoil.MaxAirVolFlow = MaxAirVolFlowDes;
                } else { // Hard size with sizing data
                    if (fanCoil.MaxAirVolFlow > 0.0 && MaxAirVolFlowDes > 0.0) {
                        MaxAirVolFlowUser = fanCoil.MaxAirVolFlow;
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(MaxAirVolFlowDes - MaxAirVolFlowUser) / MaxAirVolFlowUser) > state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(
                                    state,
                                    format("SizeFanCoilUnit: Potential issue with equipment sizing for {} {}", fanCoil.UnitType, fanCoil.Name));
                                ShowContinueError(state, format("User-Specified Supply Air Maximum Flow Rate of {:.5R} [m3/s]", MaxAirVolFlowUser));
                                ShowContinueError(state,
                                                  format("differs from Design Size Supply Air Maximum Flow Rate of {:.5R} [m3/s]", MaxAirVolFlowDes));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }
            }
        } else if (fanCoil.FanAirVolFlow == DataSizing::AutoSize) {
            state.dataFans->fans(fanCoil.FanIndex)->simulate(state, true, _, _);
            fanCoil.FanAirVolFlow = state.dataFans->fans(fanCoil.FanIndex)->maxAirFlowRate;

            //   Check that the fan volumetric flow rate is greater than or equal to the FCU volumetric flow rate
            if (fanCoil.MaxAirVolFlow > fanCoil.FanAirVolFlow) {
                ShowWarningError(state, format("{}{}: {}", RoutineName, fanCoil.UnitType, fanCoil.Name));
                ShowContinueError(state, "... Maximum supply air flow rate is greater than the maximum fan flow rate.");
                ShowContinueError(state, format("... Fan Coil Unit flow = {:.5T} m3/s.", fanCoil.MaxAirVolFlow));
                ShowContinueError(state, format("... Fan = {}: {}", HVAC::fanTypeNames[(int)fanCoil.fanType], fanCoil.FanName));
                ShowContinueError(state, format("... Fan flow = {:.5T} m3/s.", fanCoil.FanAirVolFlow));
                ShowContinueError(state, "... Fan Coil Unit flow rate reduced to match the fan flow rate and the simulation continues.");
                fanCoil.MaxAirVolFlow = fanCoil.FanAirVolFlow;
            }
        }

        IsAutoSize = false;
        if (fanCoil.OutAirVolFlow == DataSizing::AutoSize) {
            IsAutoSize = true;
        }

        if (state.dataSize->CurZoneEqNum > 0) {
            if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) {
                if (fanCoil.OutAirVolFlow > 0.0) {
                    BaseSizer::reportSizerOutput(
                        state, fanCoil.UnitType, fanCoil.Name, "User-Specified Maximum Outdoor Air Flow Rate [m3/s]", fanCoil.OutAirVolFlow);
                }
            } else {
                CheckZoneSizing(state, fanCoil.UnitType, fanCoil.Name);
                OutAirVolFlowDes = min(state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).MinOA, fanCoil.MaxAirVolFlow);
                if (OutAirVolFlowDes < HVAC::SmallAirVolFlow) {
                    OutAirVolFlowDes = 0.0;
                }
                if (IsAutoSize) {
                    fanCoil.OutAirVolFlow = OutAirVolFlowDes;
                    BaseSizer::reportSizerOutput(
                        state, fanCoil.UnitType, fanCoil.Name, "Design Size Maximum Outdoor Air Flow Rate [m3/s]", OutAirVolFlowDes);
                } else {
                    if (fanCoil.OutAirVolFlow > 0.0 && OutAirVolFlowDes > 0.0) {
                        OutAirVolFlowUser = fanCoil.OutAirVolFlow;
                        BaseSizer::reportSizerOutput(state,
                                                     fanCoil.UnitType,
                                                     fanCoil.Name,
                                                     "Design Size Maximum Outdoor Air Flow Rate [m3/s]",
                                                     OutAirVolFlowDes,
                                                     "User-Specified Maximum Outdoor Air Flow Rate [m3/s]",
                                                     OutAirVolFlowUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(OutAirVolFlowDes - OutAirVolFlowUser) / OutAirVolFlowUser) > state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(
                                    state,
                                    format("SizeFanCoilUnit: Potential issue with equipment sizing for {} {}", fanCoil.UnitType, fanCoil.Name));
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
            zoneEqSizing.OAVolFlow = fanCoil.OutAirVolFlow; // sets OA frac in sizing

            if (fanCoil.ATMixerExists) {      // set up ATMixer conditions for use in component sizing
                zoneEqSizing.OAVolFlow = 0.0; // Equipment OA flow should always be 0 when ATMixer is used
                SingleDuct::setATMixerSizingProperties(state, fanCoil.ATMixerIndex, ControlledZoneNum, state.dataSize->CurZoneEqNum);
            }
        }

        if (fanCoil.HCoilType_Num == HCoil::Water) {
            IsAutoSize = false;
            if (fanCoil.MaxHotWaterVolFlow == DataSizing::AutoSize) {
                IsAutoSize = true;
            }

            if (state.dataSize->CurZoneEqNum > 0) {
                if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) {
                    if (fanCoil.MaxHotWaterVolFlow > 0.0) {
                        BaseSizer::reportSizerOutput(
                            state, fanCoil.UnitType, fanCoil.Name, "User-Specified Maximum Hot Water Flow [m3/s]", fanCoil.MaxHotWaterVolFlow);
                    }
                } else {
                    state.dataFanCoilUnits->CoilWaterInletNode =
                        WaterCoils::GetCoilWaterInletNode(state, "Coil:Heating:Water", fanCoil.HCoilName, ErrorsFound);
                    state.dataFanCoilUnits->CoilWaterOutletNode =
                        WaterCoils::GetCoilWaterOutletNode(state, "Coil:Heating:Water", fanCoil.HCoilName, ErrorsFound);
                    if (IsAutoSize) {
                        int PltSizHeatNum = PlantUtilities::MyPlantSizingIndex(state,
                                                                               "Coil:Heating:Water",
                                                                               fanCoil.HCoilName,
                                                                               state.dataFanCoilUnits->CoilWaterInletNode,
                                                                               state.dataFanCoilUnits->CoilWaterOutletNode,
                                                                               ErrorsFound);
                        CoilNum = WaterCoils::GetWaterCoilIndex(state, "COIL:HEATING:WATER", fanCoil.HCoilName, ErrorsFound);
                        bool DoWaterCoilSizing; // if TRUE do water coil sizing calculation
                        if (state.dataWaterCoils->WaterCoil(CoilNum).UseDesignWaterDeltaTemp) {
                            WaterCoilSizDeltaT = state.dataWaterCoils->WaterCoil(CoilNum).DesignWaterDeltaTemp;
                            DoWaterCoilSizing = true;
                        } else {
                            if (PltSizHeatNum > 0) {
                                WaterCoilSizDeltaT = state.dataSize->PlantSizData(PltSizHeatNum).DeltaT;
                                DoWaterCoilSizing = true;
                            } else {
                                DoWaterCoilSizing = false;
                                // If there is no heating Plant Sizing object and autosizing was requested, issue fatal error message
                                ShowSevereError(state, "Autosizing of water coil requires a heating loop Sizing:Plant object");
                                ShowContinueError(state, format("Occurs in {} Object={}", fanCoil.UnitType, fanCoil.Name));
                                ErrorsFound = true;
                            }
                        }
                        if (DoWaterCoilSizing) {
                            SizingMethod = HVAC::HeatingCapacitySizing;
                            if (state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatMassFlow > 0.0) {
                                state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatOAFlowFrac =
                                    min(fanCoil.OutAirVolFlow / state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatMassFlow, 1.0);
                            } else {
                                state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatOAFlowFrac = 0.0;
                            }
                            if (fanCoil.HVACSizingIndex > 0) {
                                zoneHVACIndex = fanCoil.HVACSizingIndex;
                                int CapSizingMethod = state.dataSize->ZoneHVACSizing(zoneHVACIndex).HeatingCapMethod;
                                zoneEqSizing.SizingMethod(SizingMethod) = CapSizingMethod;
                                if (CapSizingMethod == DataSizing::HeatingDesignCapacity || CapSizingMethod == DataSizing::CapacityPerFloorArea ||
                                    CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                                    if (CapSizingMethod == DataSizing::HeatingDesignCapacity) {
                                        if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity > 0.0) {
                                            zoneEqSizing.HeatingCapacity = true;
                                            zoneEqSizing.DesHeatingLoad = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity;
                                        }
                                        TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity;
                                    } else if (CapSizingMethod == DataSizing::CapacityPerFloorArea) {
                                        if (state.dataSize->ZoneSizingRunDone) {
                                            PrintFlag = false;
                                            TempSize = DataSizing::AutoSize;
                                            state.dataSize->DataFlowUsedForSizing =
                                                state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatVolFlow;
                                            bool errorsFound = false;
                                            HeatingCapacitySizer sizerHeatingCapacity;
                                            sizerHeatingCapacity.overrideSizingString(SizingString);
                                            sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                            zoneEqSizing.DesHeatingLoad = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                                            zoneEqSizing.HeatingCapacity = true;
                                        }
                                        TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity *
                                                   state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                                        state.dataSize->DataScalableCapSizingON = true;
                                    } else if (CapSizingMethod == DataSizing::FractionOfAutosizedHeatingCapacity) {
                                        CheckZoneSizing(state, CompType, CompName);
                                        PrintFlag = false;
                                        TempSize = DataSizing::AutoSize;
                                        state.dataSize->DataFlowUsedForSizing =
                                            state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatVolFlow;
                                        bool errorsFound = false;
                                        HeatingCapacitySizer sizerHeatingCapacity;
                                        sizerHeatingCapacity.overrideSizingString(SizingString);
                                        sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                        zoneEqSizing.DesHeatingLoad = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                                        zoneEqSizing.HeatingCapacity = true;
                                        TempSize = zoneEqSizing.DesHeatingLoad * state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledHeatingCapacity;
                                        state.dataSize->DataScalableCapSizingON = true;
                                    }
                                }
                                SizingString = "Heating Design Capacity [W]";
                                PrintFlag = false;
                                bool errorsFound = false;
                                HeatingCapacitySizer sizerHeatingCapacity;
                                sizerHeatingCapacity.overrideSizingString(SizingString);
                                sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                DesCoilLoad = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                                state.dataSize->DataScalableCapSizingON = false;
                                state.dataSize->DataFlowUsedForSizing = 0.0;

                            } else {
                                SizingString = "Heating Design Capacity [W]";
                                PrintFlag = false;
                                TempSize = DataSizing::AutoSize;
                                bool errorsFound = false;
                                HeatingCapacitySizer sizerHeatingCapacity;
                                sizerHeatingCapacity.overrideSizingString(SizingString);
                                sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                DesCoilLoad = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                            }
                            fanCoil.DesHeatingLoad = DesCoilLoad;
                            if (DesCoilLoad >= HVAC::SmallLoad) {
                                rho = state.dataPlnt->PlantLoop(fanCoil.HeatCoilPlantLoc.loopNum)
                                          .glycol->getDensity(state, Constant::HWInitConvTemp, RoutineNameNoSpace);
                                Cp = state.dataPlnt->PlantLoop(fanCoil.HeatCoilPlantLoc.loopNum)
                                         .glycol->getSpecificHeat(state, Constant::HWInitConvTemp, RoutineNameNoSpace);

                                MaxHotWaterVolFlowDes = DesCoilLoad / (WaterCoilSizDeltaT * Cp * rho);
                            } else {
                                MaxHotWaterVolFlowDes = 0.0;
                            }
                        }
                    }
                }

                if (IsAutoSize) {
                    fanCoil.MaxHotWaterVolFlow = MaxHotWaterVolFlowDes;
                    BaseSizer::reportSizerOutput(
                        state, fanCoil.UnitType, fanCoil.Name, "Design Size Maximum Hot Water Flow [m3/s]", MaxHotWaterVolFlowDes);
                } else { // Hard size with sizing data
                    if (fanCoil.MaxHotWaterVolFlow > 0.0 && MaxHotWaterVolFlowDes > 0.0) {
                        MaxHotWaterVolFlowDes = fanCoil.MaxHotWaterVolFlow;
                        BaseSizer::reportSizerOutput(state,
                                                     fanCoil.UnitType,
                                                     fanCoil.Name,
                                                     "Design Size Maximum Hot Water Flow [m3/s]",
                                                     MaxHotWaterVolFlowDes,
                                                     "User-Specified Maximum Hot Water Flow [m3/s]",
                                                     MaxHotWaterVolFlowUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(MaxHotWaterVolFlowDes - MaxHotWaterVolFlowUser) / MaxHotWaterVolFlowUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(
                                    state,
                                    format("SizeFanCoilUnit: Potential issue with equipment sizing for {} {}", fanCoil.UnitType, fanCoil.Name));
                                ShowContinueError(state, format("User-Specified Maximum Hot Water Flow of {:.5R} [m3/s]", MaxHotWaterVolFlowUser));
                                ShowContinueError(state,
                                                  format("differs from Design Size Maximum Hot Water Flow of {:.5R} [m3/s]", MaxHotWaterVolFlowDes));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }
            }
        } else if (fanCoil.HCoilType_Num == HCoil::Electric) {
            if (fanCoil.DesignHeatingCapacity == DataSizing::AutoSize) {
                CompName = fanCoil.HCoilName;
                CompType = fanCoil.HCoilType;
                SizingMethod = HVAC::HeatingCapacitySizing;
                PrintFlag = false;
                TempSize = fanCoil.DesignHeatingCapacity;
                SizingString = "Nominal Heating Capacity [W]";
                bool errorsFound = false;
                HeatingCapacitySizer sizerHeatingCapacity;
                sizerHeatingCapacity.overrideSizingString(SizingString);
                sizerHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                fanCoil.DesignHeatingCapacity = sizerHeatingCapacity.size(state, TempSize, errorsFound);
                fanCoil.DesHeatingLoad = fanCoil.DesignHeatingCapacity;
            }
        }

        IsAutoSize = false;
        if (fanCoil.MaxColdWaterVolFlow == DataSizing::AutoSize) {
            IsAutoSize = true;
        }
        if (state.dataSize->CurZoneEqNum > 0) {
            if (!IsAutoSize && !state.dataSize->ZoneSizingRunDone) {
                if (fanCoil.MaxColdWaterVolFlow > 0.0) {
                    BaseSizer::reportSizerOutput(
                        state, fanCoil.UnitType, fanCoil.Name, "User-Specified Maximum Cold Water Flow [m3/s]", fanCoil.MaxColdWaterVolFlow);
                }
            } else {
                if (Util::SameString(fanCoil.CCoilType, "CoilSystem:Cooling:Water:HeatExchangerAssisted")) {
                    CoolingCoilName = HVACHXAssistedCoolingCoil::GetHXDXCoilName(state, fanCoil.CCoilType, fanCoil.CCoilName, ErrorsFound);
                    CoolingCoilType = HVACHXAssistedCoolingCoil::GetHXCoilType(state, fanCoil.CCoilType, fanCoil.CCoilName, ErrorsFound);
                } else {
                    CoolingCoilName = fanCoil.CCoilName;
                    CoolingCoilType = fanCoil.CCoilType;
                }
                state.dataFanCoilUnits->CoilWaterInletNode = WaterCoils::GetCoilWaterInletNode(state, CoolingCoilType, CoolingCoilName, ErrorsFound);
                state.dataFanCoilUnits->CoilWaterOutletNode =
                    WaterCoils::GetCoilWaterOutletNode(state, CoolingCoilType, CoolingCoilName, ErrorsFound);
                if (IsAutoSize) {
                    int PltSizCoolNum = PlantUtilities::MyPlantSizingIndex(state,
                                                                           CoolingCoilType,
                                                                           CoolingCoilName,
                                                                           state.dataFanCoilUnits->CoilWaterInletNode,
                                                                           state.dataFanCoilUnits->CoilWaterOutletNode,
                                                                           ErrorsFound);
                    CoilNum = WaterCoils::GetWaterCoilIndex(state, CoolingCoilType, CoolingCoilName, ErrorsFound);
                    bool DoWaterCoilSizing; // if TRUE do water coil sizing calculation
                    if (state.dataWaterCoils->WaterCoil(CoilNum).UseDesignWaterDeltaTemp) {
                        WaterCoilSizDeltaT = state.dataWaterCoils->WaterCoil(CoilNum).DesignWaterDeltaTemp;
                        DoWaterCoilSizing = true;
                    } else {
                        if (PltSizCoolNum > 0) {
                            WaterCoilSizDeltaT = state.dataSize->PlantSizData(PltSizCoolNum).DeltaT;
                            DoWaterCoilSizing = true;
                        } else {
                            DoWaterCoilSizing = false;
                            // If there is no cooling Plant Sizing object and autosizing was requested, issue fatal error message
                            ShowSevereError(state, "Autosizing of water coil requires a cooling loop Sizing:Plant object");
                            ShowContinueError(state, format("Occurs in {} Object={}", fanCoil.UnitType, fanCoil.Name));
                            ErrorsFound = true;
                        }
                    }

                    if (DoWaterCoilSizing) {
                        SizingMethod = HVAC::CoolingCapacitySizing;
                        if (state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolMassFlow > 0.0) {
                            state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolOAFlowFrac =
                                min(fanCoil.OutAirVolFlow / state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolMassFlow, 1.0);
                        } else {
                            state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolOAFlowFrac = 0.0;
                        }
                        if (fanCoil.HVACSizingIndex > 0) {
                            zoneHVACIndex = fanCoil.HVACSizingIndex;
                            int CapSizingMethod = state.dataSize->ZoneHVACSizing(zoneHVACIndex).CoolingCapMethod;
                            zoneEqSizing.SizingMethod(SizingMethod) = CapSizingMethod;
                            if (CapSizingMethod == DataSizing::CoolingDesignCapacity || CapSizingMethod == DataSizing::CapacityPerFloorArea ||
                                CapSizingMethod == DataSizing::FractionOfAutosizedCoolingCapacity) {
                                if (CapSizingMethod == DataSizing::CoolingDesignCapacity) {
                                    if (state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity > 0.0) {
                                        zoneEqSizing.CoolingCapacity = true;
                                        zoneEqSizing.DesCoolingLoad = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity;
                                    } else {
                                        state.dataSize->DataFlowUsedForSizing =
                                            state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolVolFlow;
                                    }
                                    TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity;
                                } else if (CapSizingMethod == DataSizing::CapacityPerFloorArea) {
                                    if (state.dataSize->ZoneSizingRunDone) {
                                        CheckZoneSizing(state, CompType, CompName);
                                        PrintFlag = false;
                                        TempSize = DataSizing::AutoSize;
                                        state.dataSize->DataFlowUsedForSizing =
                                            state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolVolFlow;
                                        CoolingCapacitySizer sizerCoolingCapacity;
                                        sizerCoolingCapacity.overrideSizingString(SizingString);
                                        sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                        zoneEqSizing.DesCoolingLoad = sizerCoolingCapacity.size(state, TempSize, ErrorsFound);
                                        zoneEqSizing.CoolingCapacity = true;
                                    }
                                    TempSize = state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity *
                                               state.dataHeatBal->Zone(state.dataSize->DataZoneNumber).FloorArea;
                                    state.dataSize->DataScalableCapSizingON = true;
                                } else if (CapSizingMethod == DataSizing::FractionOfAutosizedCoolingCapacity) {
                                    PrintFlag = false;
                                    TempSize = DataSizing::AutoSize;
                                    state.dataSize->DataFlowUsedForSizing =
                                        state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolVolFlow;
                                    CoolingCapacitySizer sizerCoolingCapacity2;
                                    sizerCoolingCapacity2.overrideSizingString(SizingString);
                                    sizerCoolingCapacity2.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                                    zoneEqSizing.DesCoolingLoad = sizerCoolingCapacity2.size(state, TempSize, ErrorsFound);
                                    zoneEqSizing.CoolingCapacity = true;
                                    TempSize = zoneEqSizing.DesCoolingLoad * state.dataSize->ZoneHVACSizing(zoneHVACIndex).ScaledCoolingCapacity;
                                    state.dataSize->DataScalableCapSizingON = true;
                                }
                            }
                            SizingString = "Cooling Design Capacity [W]";
                            PrintFlag = false;
                            CoolingCapacitySizer sizerCoolingCapacity3;
                            sizerCoolingCapacity3.overrideSizingString(SizingString);
                            sizerCoolingCapacity3.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                            DesCoilLoad = sizerCoolingCapacity3.size(state, TempSize, ErrorsFound);
                            state.dataSize->DataScalableCapSizingON = false;
                            state.dataSize->DataFlowUsedForSizing = 0.0;
                        } else {
                            SizingString = "Cooling Design Capacity [W]";
                            PrintFlag = false;
                            TempSize = DataSizing::AutoSize;
                            state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolVolFlow;
                            CoolingCapacitySizer sizerCoolingCapacity;
                            sizerCoolingCapacity.overrideSizingString(SizingString);
                            sizerCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                            DesCoilLoad = sizerCoolingCapacity.size(state, TempSize, ErrorsFound);
                        }
                        fanCoil.DesCoolingLoad = DesCoilLoad;
                        if (DesCoilLoad >= HVAC::SmallLoad) {
                            rho = state.dataPlnt->PlantLoop(fanCoil.CoolCoilPlantLoc.loopNum).glycol->getDensity(state, 5., RoutineNameNoSpace);
                            Cp = state.dataPlnt->PlantLoop(fanCoil.CoolCoilPlantLoc.loopNum).glycol->getSpecificHeat(state, 5., RoutineNameNoSpace);
                            MaxColdWaterVolFlowDes = DesCoilLoad / (WaterCoilSizDeltaT * Cp * rho);
                        } else {
                            MaxColdWaterVolFlowDes = 0.0;
                        }
                    }
                    fanCoil.MaxColdWaterVolFlow = MaxColdWaterVolFlowDes;
                    BaseSizer::reportSizerOutput(
                        state, fanCoil.UnitType, fanCoil.Name, "Design Size Maximum Cold Water Flow [m3/s]", MaxColdWaterVolFlowDes);
                } else { // Hard size with sizing data
                    if (fanCoil.MaxColdWaterVolFlow > 0.0 && MaxColdWaterVolFlowDes > 0.0) {
                        MaxColdWaterVolFlowUser = fanCoil.MaxColdWaterVolFlow;
                        BaseSizer::reportSizerOutput(state,
                                                     fanCoil.UnitType,
                                                     fanCoil.Name,
                                                     "Design Size Maximum Cold Water Flow [m3/s]",
                                                     MaxColdWaterVolFlowDes,
                                                     "User-Specified Maximum Cold Water Flow [m3/s]",
                                                     MaxColdWaterVolFlowUser);
                        if (state.dataGlobal->DisplayExtraWarnings) {
                            if ((std::abs(MaxColdWaterVolFlowDes - MaxColdWaterVolFlowUser) / MaxColdWaterVolFlowUser) >
                                state.dataSize->AutoVsHardSizingThreshold) {
                                ShowMessage(
                                    state,
                                    format("SizeFanCoilUnit: Potential issue with equipment sizing for {} {}", fanCoil.UnitType, fanCoil.Name));
                                ShowContinueError(state, format("User-Specified Maximum Cold Water Flow of {:.5R}[m3/s]", MaxColdWaterVolFlowUser));
                                ShowContinueError(state,
                                                  format("differs from Design Size Maximum Cold Water Flow of {:.5R}[m3/s]", MaxColdWaterVolFlowDes));
                                ShowContinueError(state, "This may, or may not, indicate mismatched component sizes.");
                                ShowContinueError(state, "Verify that the value entered is intended and is consistent with other components.");
                            }
                        }
                    }
                }
            }

            if (fanCoil.CapCtrlMeth_Num == CCM::ASHRAE && !fanCoil.ASHRAETempControl) {

                CompType = fanCoil.UnitType;
                CompName = fanCoil.Name;
                PrintFlag = true;

                ZoneCoolingLoadSizer sizerZoneCoolingLoad;
                sizerZoneCoolingLoad.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                fanCoil.DesZoneCoolingLoad = sizerZoneCoolingLoad.size(state, fanCoil.DesZoneCoolingLoad, ErrorsFound);
                fanCoil.DesZoneCoolingLoad *= -1.0;

                ZoneHeatingLoadSizer sizerZoneHeatingLoad;
                sizerZoneHeatingLoad.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                fanCoil.DesZoneHeatingLoad = sizerZoneHeatingLoad.size(state, fanCoil.DesZoneHeatingLoad, ErrorsFound);

                fanCoil.DSOAPtr = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).ZoneDesignSpecOAIndex;

            } else if (fanCoil.CapCtrlMeth_Num == CCM::ASHRAE && fanCoil.ASHRAETempControl) {

                CompType = fanCoil.UnitType;
                CompName = fanCoil.Name;
                Real64 capacityMultiplier = 0.6; // 60% of design zone load for water coils
                state.dataSize->DataCapacityUsedForSizing = fanCoil.DesCoolingLoad * capacityMultiplier;
                bool SizingDesRunThisZone = false;
                CheckThisZoneForSizing(state, state.dataSize->CurZoneEqNum, SizingDesRunThisZone);
                if (SizingDesRunThisZone) {
                    state.dataSize->DataCapacityUsedForSizing =
                        state.dataSize->FinalZoneSizing(fanCoil.ControlZoneNum).DesCoolLoad * capacityMultiplier;
                } else {
                    state.dataSize->DataCapacityUsedForSizing = fanCoil.DesCoolingLoad * capacityMultiplier;
                }
                state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolVolFlow;
                PrintFlag = true;
                ASHRAEMinSATCoolingSizer sizerASHRAEMinSATCooling;
                sizerASHRAEMinSATCooling.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                fanCoil.DesignMinOutletTemp = sizerASHRAEMinSATCooling.size(state, fanCoil.DesignMinOutletTemp, ErrorsFound);

                if (SizingDesRunThisZone) {
                    state.dataSize->DataCapacityUsedForSizing =
                        state.dataSize->FinalZoneSizing(fanCoil.ControlZoneNum).DesHeatLoad * capacityMultiplier;
                } else {
                    state.dataSize->DataCapacityUsedForSizing = fanCoil.DesHeatingLoad * capacityMultiplier;
                }
                state.dataSize->DataFlowUsedForSizing = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatVolFlow;
                ASHRAEMaxSATHeatingSizer sizerASHRAEMaxSATHeating;
                sizerASHRAEMaxSATHeating.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
                fanCoil.DesignMaxOutletTemp = sizerASHRAEMaxSATHeating.size(state, fanCoil.DesignMaxOutletTemp, ErrorsFound);

                state.dataSize->DataCapacityUsedForSizing = 0.0; // reset so other routines don't use this inadvertently
                state.dataSize->DataFlowUsedForSizing = 0.0;

                SizingDesRunThisZone = false;
                CheckThisZoneForSizing(state, state.dataSize->CurZoneEqNum, SizingDesRunThisZone);

                if (SizingDesRunThisZone) {

                    fanCoil.DesZoneCoolingLoad =
                        -1.0 * (fanCoil.DesCoolingLoad / state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).CoolSizingFactor);
                    fanCoil.DesZoneHeatingLoad =
                        fanCoil.DesHeatingLoad / state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).HeatSizingFactor;
                    fanCoil.DSOAPtr = state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).ZoneDesignSpecOAIndex;

                } else {

                    fanCoil.DesZoneCoolingLoad = -1.0 * fanCoil.DesCoolingLoad;
                    fanCoil.DesZoneHeatingLoad = fanCoil.DesHeatingLoad;
                }
            }

        } // if ( CurZoneEqNum > 0 )

        // set the design air flow rates for the heating and cooling coils
        if (Util::SameString(fanCoil.CCoilType, "CoilSystem:Cooling:Water:HeatExchangerAssisted")) {
            CoolingCoilName = HVACHXAssistedCoolingCoil::GetHXDXCoilName(state, fanCoil.CCoilType, fanCoil.CCoilName, ErrorsFound);
            CoolingCoilType = HVACHXAssistedCoolingCoil::GetHXCoilType(state, fanCoil.CCoilType, fanCoil.CCoilName, ErrorsFound);
        } else {
            CoolingCoilName = fanCoil.CCoilName;
            CoolingCoilType = fanCoil.CCoilType;
        }
        if (state.dataSize->ZoneSizingRunDone) {
            WaterCoils::SetCoilDesFlow(
                state, CoolingCoilType, CoolingCoilName, state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesCoolVolFlow, ErrorsFound);
            WaterCoils::SetCoilDesFlow(state,
                                       fanCoil.HCoilType,
                                       fanCoil.HCoilName,
                                       state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).DesHeatVolFlow,
                                       ErrorsFound);
        } else {
            WaterCoils::SetCoilDesFlow(state, CoolingCoilType, CoolingCoilName, fanCoil.MaxAirVolFlow, ErrorsFound);
            WaterCoils::SetCoilDesFlow(state, fanCoil.HCoilType, fanCoil.HCoilName, fanCoil.MaxAirVolFlow, ErrorsFound);
        }
        if (state.dataSize->CurZoneEqNum > 0) {
            zoneEqSizing.MaxHWVolFlow = fanCoil.MaxHotWaterVolFlow;
            zoneEqSizing.MaxCWVolFlow = fanCoil.MaxColdWaterVolFlow;
            zoneEqSizing.AirVolFlow = fanCoil.MaxAirVolFlow;
            zoneEqSizing.DesCoolingLoad = fanCoil.DesCoolingLoad;
            zoneEqSizing.DesHeatingLoad = fanCoil.DesHeatingLoad;
            zoneEqSizing.DesignSizeFromParent = true;
        }

        if (ErrorsFound) {
            ShowFatalError(state, "Preceding sizing errors cause program termination");
        }
    }

    void Sim4PipeFanCoil(EnergyPlusData &state,
                         int &FanCoilNum,               // number of the current fan coil unit being simulated
                         int const ControlledZoneNum,   // index into ZoneEqupConfig
                         bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                         Real64 &PowerMet,              // Sensible power supplied (W)
                         Real64 &LatOutputProvided      // Latent power supplied (kg/s), negative = dehumidification
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   March 2000
        //       MODIFIED       Don Shirey, Aug 2009 (LatOutputProvided)
        //       MODIFIED       Arnaud Flament June 2010 (added airflow capacity control methods)
        //       MODIFIED      R. Raustad, FSEC, Feb 2016 (added ASHRAE 90.1 SZVAV system control)

        // PURPOSE OF THIS SUBROUTINE:
        // Simulate a 4 pipe fan coil unit; adjust its output to match the
        // remaining zone load.

        // METHODOLOGY EMPLOYED:
        // If unit is on, calls ControlCompOutput to obtain the desired unit output

        // REFERENCES:
        // SZVAV sysetm control:
        // ASHRAE 90.1 2010 Section 6.4.3.10 - Single Zone Variable-Air-volume Controls (described in Trane newsletter entitled Understanding
        // Single-Zone VAV Systems) Trane Engineers Newsletter -
        // https://www.trane.com/content/dam/Trane/Commercial/global/products-systems/education-training/engineers-newsletters/airside-design/admapn047en_0413.pdf
        //

        int constexpr MaxIterCycl(100);

        Real64 PLRMin;       // minimum PLR used for tighter control of air and water flow rate
        Real64 PLRMax;       // maximum PLR used for tighter control of air and water flow rate
        Real64 QTotUnitOut;  // total unit output [watts]
        Real64 QUnitOutMaxC; // unit output with full active cooling [W]
        Real64 QUnitOutMaxH; // unit output with full active heating [W]
        Real64 SpecHumOut;   // Specific humidity ratio of outlet air (kg moisture / kg moist air)
        Real64 SpecHumIn;    // Specific humidity ratio of inlet air (kg moisture / kg moist air)
        Real64 DelPLR;
        Real64 mdot;
        // Real64 Low_mdot;
        Real64 QSensUnitOutNoATM; // unit output not including air added by supply side air terminal mixer
        int SolFlag;              // return flag from RegulaFalsi for sensible load
        Real64 OAVolumeFlowRate;  // OA volume flow rate based on design specifications object [m3/s]
        Real64 OAMassFlow;        // OA mass flow rate based on design specifications object [kg/s]
        Real64 RhoAir;            // density of air [kg/m3]
        Real64 MinSAMassFlowRate; // minimum supply air mass flow rate [kg/s]
        Real64 MaxSAMassFlowRate; // maximum supply air mass flow rate [kg/s]
        // Real64 FCOutletTempOn;        // ASHRAE outlet air temperature when coil is on [C]
        Real64 CWFlow;       // cold water mass flow rate solution [kg/s]
        Real64 CWFlowBypass; // cold water bypassed mass flow rate [kg/s]

        auto &fanCoil = state.dataFanCoilUnits->FanCoil(FanCoilNum);

        // initialize local variables
        bool UnitOn = true;         // TRUE if unit is on
        Real64 QUnitOut = 0.0;      // heating or sens. cooling provided by fan coil unit [watts]
        Real64 QUnitOutMax = 0.0;   // heating or sens. cooling provided by fan coil unit (running during an entire timestep)
        Real64 PLR = 0.0;           // Part Load Ratio, fraction of time step fancoil is on
        Real64 LatentOutput = 0.0;  // Latent (moisture) add/removal rate, negative is dehumidification [kg/s]
        Real64 QUnitOutNoHC = 0.0;  // unit output with no active heating or cooling [W]
        Real64 QCoilHeatSP = 0.0;   // coil load to the heating setpoint [W]
        Real64 QCoilCoolSP = 0.0;   // coil load to the cooling setpoint [W]
        Real64 QZnReq = 0.0;        // heating or cooling needed by zone [watts]
        Real64 ControlOffset = 0.0; // tolerance for output control
        Real64 MaxWaterFlow = 0.0;  // maximum water flow for heating or cooling [kg/sec]
        Real64 MinWaterFlow = 0.0;  // minimum water flow for heating or cooling [kg/sec]
        int OutletNode = fanCoil.AirOutNode;
        int InletNode = fanCoil.AirInNode;
        Real64 AirMassFlow = state.dataLoopNodes->Node(InletNode).MassFlowRate; // air mass flow rate [kg/sec]
        Real64 Error = 1.0;                                                     // Error between QZnReq and QUnitOut
        Real64 AbsError = 2.0 * HVAC::SmallLoad;                                // Absolute error between QZnReq and QUnitOut [W]   !FB
        Real64 Relax = 1.0;
        Real64 HWFlow = 0.0;         // hot water mass flow rate solution [kg/s]
        Real64 HWFlowBypass = 0.0;   // hot water bypassed mass flow rate [kg/s]
        Real64 MdotLockH = 0.0;      // saved value of locked chilled water mass flow rate [kg/s]
        Real64 MdotLockC = 0.0;      // saved value of locked hot water mass flow rate [kg/s]
        bool ColdFlowLocked = false; // if true cold water flow is locked
        bool HotFlowLocked = false;  // if true Hot water flow is locked

        // select capacity control method
        switch (fanCoil.CapCtrlMeth_Num) {
        case CCM::ConsFanVarFlow: {

            if (AirMassFlow < HVAC::SmallMassFlow) {
                UnitOn = false;
            }
            // zero the hot & cold water flows

            // set water coil flow rate to 0 to calculate coil off capacity (only valid while flow is unlocked)
            mdot = 0.0;
            PlantUtilities::SetComponentFlowRate(
                state, mdot, fanCoil.CoolCoilFluidInletNode, fanCoil.CoolCoilFluidOutletNodeNum, fanCoil.CoolCoilPlantLoc);
            if (state.dataPlnt->PlantLoop(fanCoil.CoolCoilPlantLoc.loopNum).LoopSide(fanCoil.CoolCoilPlantLoc.loopSideNum).FlowLock ==
                DataPlant::FlowLock::Locked) {
                ColdFlowLocked = true; // check for flow lock
            }
            if (fanCoil.HCoilType_Num == HCoil::Water) {
                mdot = 0.0;
                PlantUtilities::SetComponentFlowRate(
                    state, mdot, fanCoil.HeatCoilFluidInletNode, fanCoil.HeatCoilFluidOutletNodeNum, fanCoil.HeatCoilPlantLoc);
                if (state.dataPlnt->PlantLoop(fanCoil.HeatCoilPlantLoc.loopNum).LoopSide(fanCoil.HeatCoilPlantLoc.loopSideNum).FlowLock ==
                    DataPlant::FlowLock::Locked) {
                    HotFlowLocked = true; // save locked flow
                }
            }
            // obtain unit output with no active heating/cooling
            Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOutNoHC, 0.0);

            if (ColdFlowLocked || HotFlowLocked) {
                QUnitOutNoHC = fanCoil.QUnitOutNoHC;
            } else { // continue to update QUnitOutNoHC while flow is unlocked
                fanCoil.QUnitOutNoHC = QUnitOutNoHC;
            }

            // then calculate the loads at the coils
            QCoilHeatSP = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlledZoneNum).RemainingOutputReqToHeatSP - QUnitOutNoHC;
            QCoilCoolSP = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlledZoneNum).RemainingOutputReqToCoolSP - QUnitOutNoHC;

            // if cooling
            if (UnitOn && QCoilCoolSP < -HVAC::SmallLoad &&
                state.dataHeatBalFanSys->TempControlType(ControlledZoneNum) != HVAC::SetptType::SingleHeat) {
                int ControlNode = fanCoil.CoolCoilFluidInletNode;
                ControlOffset = fanCoil.ColdControlOffset;
                MaxWaterFlow = fanCoil.MaxCoolCoilFluidFlow;
                MinWaterFlow = fanCoil.MinColdWaterFlow;
                // On the first HVAC iteration the system values are given to the controller, but after that
                // the demand limits are in place and there needs to be feedback to the Zone Equipment
                if (!FirstHVACIteration) {
                    MaxWaterFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRateMaxAvail;
                    MinWaterFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRateMinAvail;
                }
                // get full load result
                mdot = MaxWaterFlow;
                PlantUtilities::SetComponentFlowRate(
                    state, mdot, fanCoil.CoolCoilFluidInletNode, fanCoil.CoolCoilFluidOutletNodeNum, fanCoil.CoolCoilPlantLoc);
                Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOutMaxC);
                if (!ColdFlowLocked) {
                    fanCoil.QUnitOutMaxC = QUnitOutMaxC;
                } else {
                    QUnitOutMaxC = fanCoil.QUnitOutMaxC;
                    MdotLockC = mdot; // save locked flow
                }
                QZnReq = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlledZoneNum).RemainingOutputReqToCoolSP;
                if (QUnitOutMaxC < QZnReq) {
                    // more cooling than required, find reduced water flow rate to meet the load
                    // solve for the cold water flow rate with no limit set by flow rate lockdown
                    auto f = [&state, FanCoilNum, FirstHVACIteration, ControlledZoneNum, QZnReq](Real64 const CWFlow) {
                        return CalcFanCoilCWLoadResidual(state, CWFlow, FanCoilNum, FirstHVACIteration, ControlledZoneNum, QZnReq);
                    };
                    General::SolveRoot(state, 0.001, MaxIterCycl, SolFlag, CWFlow, f, 0.0, MaxWaterFlow);
                    if (SolFlag == -1) {
                        // tighten limits on water flow rate to see if this allows convergence
                        state.dataFanCoilUnits->CoolingLoad = true;
                        state.dataFanCoilUnits->HeatingLoad = false;
                        TightenWaterFlowLimits(state,
                                               FanCoilNum,
                                               state.dataFanCoilUnits->CoolingLoad,
                                               state.dataFanCoilUnits->HeatingLoad,
                                               fanCoil.CoolCoilFluidInletNode,
                                               ControlledZoneNum,
                                               FirstHVACIteration,
                                               QZnReq,
                                               MinWaterFlow,
                                               MaxWaterFlow);
                        General::SolveRoot(state, 0.001, MaxIterCycl, SolFlag, CWFlow, f, MinWaterFlow, MaxWaterFlow);
                        if (SolFlag == -1) {
                            ++fanCoil.ConvgErrCountC;
                            if (fanCoil.ConvgErrCountC < 2) {
                                ShowWarningError(state, format("Cold Water control failed in fan coil unit {}", fanCoil.Name));
                                ShowContinueError(state, "  Iteration limit exceeded in calculating water flow rate ");
                                state.dataLoopNodes->Node(fanCoil.CoolCoilFluidInletNode).MassFlowRate = CWFlow;
                                Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut);
                                ShowContinueErrorTimeStamp(state, format("Load Request = {}, Final Capacity = {}", QZnReq, QUnitOut));
                                ShowContinueErrorTimeStamp(
                                    state,
                                    format("Min water flow used during iterations = {}, Max water flow used during iterations = {}",
                                           MinWaterFlow,
                                           MaxWaterFlow));
                                ShowContinueErrorTimeStamp(state, format("Water flow rate on last iteration = {}", CWFlow));
                                ShowContinueErrorTimeStamp(state, "..Water flow rate set to last iteration value ");
                            } else {
                                ShowRecurringWarningErrorAtEnd(
                                    state, "Cold water flow Iteration limit exceeded in fan coil unit " + fanCoil.Name, fanCoil.MaxIterIndexC);
                            }
                        } else if (SolFlag == -2) {
                            ++fanCoil.LimitErrCountC;
                            if (fanCoil.LimitErrCountC < 2) {
                                ShowWarningError(state, format("Cold Water control failed in fan coil unit {}", fanCoil.Name));
                                ShowContinueError(state, "  Bad cold water mass flow limits");
                                ShowContinueErrorTimeStamp(state, "..Water flow rate set to lower limit ");
                            } else {
                                ShowRecurringWarningErrorAtEnd(
                                    state, "Cold Water control failed in fan coil unit " + fanCoil.Name, fanCoil.BadMassFlowLimIndexC);
                            }
                        }
                    } else if (SolFlag == -2) {
                        ++fanCoil.LimitErrCountC;
                        if (fanCoil.LimitErrCountC < 2) {
                            ShowWarningError(state, format("Cold Water control failed in fan coil unit {}", fanCoil.Name));
                            ShowContinueError(state, "  Bad cold water mass flow limits");
                            ShowContinueErrorTimeStamp(state, "..Water flow rate set to lower limit ");
                        } else {
                            ShowRecurringWarningErrorAtEnd(
                                state, "Cold Water control failed in fan coil unit " + fanCoil.Name, fanCoil.BadMassFlowLimIndexC);
                        }
                    }
                } else {
                    // demand greater than capacity
                    CWFlow = MaxWaterFlow;
                }
                if (!ColdFlowLocked) {
                    mdot = CWFlow; // not flowlocked - set flow to CWFlow
                    PlantUtilities::SetComponentFlowRate(
                        state, mdot, fanCoil.CoolCoilFluidInletNode, fanCoil.CoolCoilFluidOutletNodeNum, fanCoil.CoolCoilPlantLoc);
                    Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut); // get QUnitOut
                } else {
                    // flow lock on
                    if (MdotLockC > CWFlow) { // if mdot > CWFlow, bypass extra flow
                        Calc4PipeFanCoil(state,
                                         FanCoilNum,
                                         ControlledZoneNum,
                                         FirstHVACIteration,
                                         QUnitOut); // get QUnitOut with CWFlow; rest will be bypassed
                        state.dataLoopNodes->Node(fanCoil.CoolCoilFluidInletNode).MassFlowRate =
                            MdotLockC; // reset flow to locked value. Since lock is on, must do this by hand
                        state.dataLoopNodes->Node(fanCoil.CoolCoilFluidOutletNodeNum).MassFlowRate = MdotLockC;
                        // Keep soln flow rate but reset outlet water temperature - i.e. bypass extra water
                        CWFlowBypass = MdotLockC - CWFlow;
                        // change water outlet temperature and enthalpy
                        state.dataLoopNodes->Node(fanCoil.CoolCoilFluidOutletNodeNum).Temp =
                            (CWFlowBypass * state.dataLoopNodes->Node(fanCoil.CoolCoilFluidInletNode).Temp +
                             CWFlow * state.dataLoopNodes->Node(fanCoil.CoolCoilFluidOutletNodeNum).Temp) /
                            MdotLockC;
                        state.dataLoopNodes->Node(fanCoil.CoolCoilFluidOutletNodeNum).Enthalpy =
                            (CWFlowBypass * state.dataLoopNodes->Node(fanCoil.CoolCoilFluidInletNode).Enthalpy +
                             CWFlow * state.dataLoopNodes->Node(fanCoil.CoolCoilFluidOutletNodeNum).Enthalpy) /
                            MdotLockC;
                    } else {
                        // if MdotLockC <= CWFlow use MdotLockC as is
                        state.dataLoopNodes->Node(fanCoil.CoolCoilFluidInletNode).MassFlowRate =
                            MdotLockC; // reset flow to locked value. Since lock is on, must do this by hand
                        state.dataLoopNodes->Node(fanCoil.CoolCoilFluidOutletNodeNum).MassFlowRate = MdotLockC;
                        Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut);
                    }
                }
                QUnitOut = calcZoneSensibleOutput(AirMassFlow,
                                                  state.dataLoopNodes->Node(OutletNode).Temp,
                                                  state.dataLoopNodes->Node(InletNode).Temp,
                                                  state.dataLoopNodes->Node(InletNode).HumRat);

                // if heating
            } else if (UnitOn && QCoilHeatSP > HVAC::SmallLoad &&
                       state.dataHeatBalFanSys->TempControlType(ControlledZoneNum) != HVAC::SetptType::SingleCool) {
                // get full load result
                if (fanCoil.HCoilType_Num == HCoil::Water) { // if HW Coil
                    int ControlNode = fanCoil.HeatCoilFluidInletNode;
                    ControlOffset = fanCoil.HotControlOffset;
                    MaxWaterFlow = fanCoil.MaxHeatCoilFluidFlow;
                    MinWaterFlow = fanCoil.MinHotWaterFlow;
                    // On the first HVAC iteration the system values are given to the controller, but after that
                    // the demand limits are in place and there needs to be feedback to the Zone Equipment
                    if (!FirstHVACIteration) {
                        MaxWaterFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRateMaxAvail;
                        MinWaterFlow = state.dataLoopNodes->Node(ControlNode).MassFlowRateMinAvail;
                    }
                    mdot = MaxWaterFlow;
                    PlantUtilities::SetComponentFlowRate(
                        state, mdot, fanCoil.HeatCoilFluidInletNode, fanCoil.HeatCoilFluidOutletNodeNum, fanCoil.HeatCoilPlantLoc);
                    Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOutMaxH);
                    if (!HotFlowLocked) {
                        fanCoil.QUnitOutMaxH = QUnitOutMaxH;
                    } else {
                        QUnitOutMaxH = fanCoil.QUnitOutMaxH;
                        MdotLockH = mdot; // save locked flow
                    }
                } else {
                    // not HW coil
                    Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOutMaxH, 1.0);
                }
                QZnReq = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlledZoneNum).RemainingOutputReqToHeatSP;
                if (QUnitOutMaxH > QZnReq) {
                    // more heating than required, find reduced water flow rate to meet the load
                    if (fanCoil.HCoilType_Num == HCoil::Water) {
                        // solve for the hot water flow rate with no limit set by flow rate lockdown
                        auto f = [&state, FirstHVACIteration, FanCoilNum, ControlledZoneNum, QZnReq](Real64 HWFlow) {
                            // To calculate the part-load ratio for the FCU with electric heating coil
                            Real64 QUnitOut; // delivered capacity [W]
                            state.dataLoopNodes->Node(state.dataFanCoilUnits->FanCoil(FanCoilNum).HeatCoilFluidInletNode).MassFlowRate = HWFlow;
                            Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut, 1.0);
                            // Calculate residual based on output magnitude
                            if (std::abs(QZnReq) <= 100.0) {
                                return (QUnitOut - QZnReq) / 100.0;
                            } else {
                                return (QUnitOut - QZnReq) / QZnReq;
                            }
                        };
                        General::SolveRoot(state, 0.001, MaxIterCycl, SolFlag, HWFlow, f, 0.0, MaxWaterFlow);
                        if (SolFlag == -1) {
                            // tighten limits on water flow rate to see if this allows convergence
                            state.dataFanCoilUnits->CoolingLoad = false;
                            state.dataFanCoilUnits->HeatingLoad = true;
                            TightenWaterFlowLimits(state,
                                                   FanCoilNum,
                                                   state.dataFanCoilUnits->CoolingLoad,
                                                   state.dataFanCoilUnits->HeatingLoad,
                                                   fanCoil.HeatCoilFluidInletNode,
                                                   ControlledZoneNum,
                                                   FirstHVACIteration,
                                                   QZnReq,
                                                   MinWaterFlow,
                                                   MaxWaterFlow);
                            General::SolveRoot(state, 0.001, MaxIterCycl, SolFlag, HWFlow, f, MinWaterFlow, MaxWaterFlow);
                            if (SolFlag == -1) {
                                ++fanCoil.ConvgErrCountH;
                                if (fanCoil.ConvgErrCountH < 2) {
                                    ShowWarningError(state, format("Hot Water control failed in fan coil unit {}", fanCoil.Name));
                                    ShowContinueError(state, "  Iteration limit exceeded in calculating water flow rate ");
                                    state.dataLoopNodes->Node(fanCoil.HeatCoilFluidInletNode).MassFlowRate = HWFlow;
                                    Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut);
                                    ShowContinueErrorTimeStamp(state, format("Load Request = {}, Final Capacity = {}", QZnReq, QUnitOut));
                                    ShowContinueErrorTimeStamp(
                                        state,
                                        format("Min water flow used during iterations = {}, Max water flow used during iterations = {}",
                                               MinWaterFlow,
                                               MaxWaterFlow));
                                    ShowContinueErrorTimeStamp(state, format("Water flow rate on last iteration = {}", HWFlow));
                                    ShowContinueErrorTimeStamp(state, "..Water flow rate set to last iteration value ");
                                } else {
                                    ShowRecurringWarningErrorAtEnd(
                                        state, "Hot water flow Iteration limit exceeded in fan coil unit " + fanCoil.Name, fanCoil.MaxIterIndexH);
                                }
                            } else if (SolFlag == -2) {
                                ++fanCoil.LimitErrCountH;
                                if (fanCoil.LimitErrCountH < 2) {
                                    ShowWarningError(state, format("Hot Water control failed in fan coil unit {}", fanCoil.Name));
                                    ShowContinueError(state, "  Bad hot water mass flow limits");
                                    ShowContinueErrorTimeStamp(state, "..Water flow rate set to lower limit ");
                                } else {
                                    ShowRecurringWarningErrorAtEnd(
                                        state, "Hot Water control failed in fan coil unit " + fanCoil.Name, fanCoil.BadMassFlowLimIndexH);
                                }
                            }
                        } else if (SolFlag == -2) {
                            ++fanCoil.LimitErrCountH;
                            if (fanCoil.LimitErrCountH < 2) {
                                ShowWarningError(state, format("Hot Water control failed in fan coil unit {}", fanCoil.Name));
                                ShowContinueError(state, "  Bad hot water mass flow limits");
                                ShowContinueErrorTimeStamp(state, "..Water flow rate set to lower limit ");
                            } else {
                                ShowRecurringWarningErrorAtEnd(
                                    state, "Hot Water control failed in fan coil unit " + fanCoil.Name, fanCoil.BadMassFlowLimIndexH);
                            }
                        }
                    } else {
                        auto f = [&state, FirstHVACIteration, FanCoilNum, ControlledZoneNum, QZnReq](Real64 const PartLoadRatio) {
                            return CalcFanCoilLoadResidual(state, FanCoilNum, FirstHVACIteration, ControlledZoneNum, QZnReq, PartLoadRatio);
                        };
                        General::SolveRoot(state, 0.001, MaxIterCycl, SolFlag, PLR, f, 0.0, 1.0);
                    }
                } else {
                    // demand greater than capacity
                    if (fanCoil.HCoilType_Num == HCoil::Water) {
                        HWFlow = MaxWaterFlow;
                    } else {
                        Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut, 1.0);
                    }
                }
                if (fanCoil.HCoilType_Num == HCoil::Water) {
                    if (!HotFlowLocked) {
                        mdot = HWFlow; // not flowlocked - set flow to HWFlow
                        PlantUtilities::SetComponentFlowRate(
                            state, mdot, fanCoil.HeatCoilFluidInletNode, fanCoil.HeatCoilFluidOutletNodeNum, fanCoil.HeatCoilPlantLoc);
                        Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut); // get QUnitOut
                    } else {
                        // flow lock on
                        if (MdotLockH > HWFlow) { // if mdot > HWFlow, bypass extra flow
                            Calc4PipeFanCoil(state,
                                             FanCoilNum,
                                             ControlledZoneNum,
                                             FirstHVACIteration,
                                             QUnitOut); // get QUnitOut with HWFlow; rest will be bypassed
                            state.dataLoopNodes->Node(fanCoil.HeatCoilFluidInletNode).MassFlowRate =
                                MdotLockH; // reset flow to locked value. Since lock is on, must do this by hand
                            state.dataLoopNodes->Node(fanCoil.HeatCoilFluidOutletNodeNum).MassFlowRate = MdotLockH;
                            // Keep soln flow rate but reset outlet water temperature - i.e. bypass extra water
                            HWFlowBypass = MdotLockH - HWFlow;
                            // change outlet water temperature and enthalpy
                            state.dataLoopNodes->Node(fanCoil.HeatCoilFluidOutletNodeNum).Temp =
                                (HWFlowBypass * state.dataLoopNodes->Node(fanCoil.HeatCoilFluidInletNode).Temp +
                                 HWFlow * state.dataLoopNodes->Node(fanCoil.HeatCoilFluidOutletNodeNum).Temp) /
                                MdotLockH;
                            state.dataLoopNodes->Node(fanCoil.HeatCoilFluidOutletNodeNum).Enthalpy =
                                (HWFlowBypass * state.dataLoopNodes->Node(fanCoil.HeatCoilFluidInletNode).Enthalpy +
                                 HWFlow * state.dataLoopNodes->Node(fanCoil.HeatCoilFluidOutletNodeNum).Enthalpy) /
                                MdotLockH;
                        } else {
                            // if MdotLockH <= HWFlow use MdotLockH as is
                            state.dataLoopNodes->Node(fanCoil.HeatCoilFluidInletNode).MassFlowRate =
                                MdotLockH; // reset flow to locked value. Since lock is on, must do this by hand
                            state.dataLoopNodes->Node(fanCoil.HeatCoilFluidOutletNodeNum).MassFlowRate = MdotLockH;
                            Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut);
                        }
                    }
                }
                QUnitOut = calcZoneSensibleOutput(AirMassFlow,
                                                  state.dataLoopNodes->Node(OutletNode).Temp,
                                                  state.dataLoopNodes->Node(InletNode).Temp,
                                                  state.dataLoopNodes->Node(InletNode).HumRat);
            } else {
                // no action
                QUnitOut = QUnitOutNoHC;
            }

            // CR9155 Remove specific humidity calculations
            SpecHumOut = state.dataLoopNodes->Node(OutletNode).HumRat;
            SpecHumIn = state.dataLoopNodes->Node(InletNode).HumRat;
            LatentOutput = AirMassFlow * (SpecHumOut - SpecHumIn); // Latent rate (kg/s), dehumid = negative
            QTotUnitOut = AirMassFlow * (state.dataLoopNodes->Node(OutletNode).Enthalpy - state.dataLoopNodes->Node(InletNode).Enthalpy);
            // report variables
            fanCoil.HeatPower = max(0.0, QUnitOut);
            fanCoil.SensCoolPower = std::abs(min(DataPrecisionGlobals::constant_zero, QUnitOut));
            fanCoil.TotCoolPower = std::abs(min(DataPrecisionGlobals::constant_zero, QTotUnitOut));
            fanCoil.ElecPower = state.dataFans->fans(fanCoil.FanIndex)->totalPower;

            PowerMet = QUnitOut;
            LatOutputProvided = LatentOutput;

            // cycling fan constant water flow AND VarFanVarFlow
        } break;
        case CCM::CycFan:
        case CCM::VarFanVarFlow: {

            if (state.dataZoneEnergyDemand->CurDeadBandOrSetback(ControlledZoneNum) || AirMassFlow < HVAC::SmallMassFlow) {
                UnitOn = false;
            }

            // zero the hot & cold water flows
            mdot = 0.0;
            PlantUtilities::SetComponentFlowRate(
                state, mdot, fanCoil.CoolCoilFluidInletNode, fanCoil.CoolCoilFluidOutletNodeNum, fanCoil.CoolCoilPlantLoc);
            if (state.dataPlnt->PlantLoop(fanCoil.CoolCoilPlantLoc.loopNum).LoopSide(fanCoil.CoolCoilPlantLoc.loopSideNum).FlowLock ==
                DataPlant::FlowLock::Locked) {
                ColdFlowLocked = true; // check for flow lock
            }
            if (fanCoil.HCoilType_Num == HCoil::Water) {
                mdot = 0.0;
                PlantUtilities::SetComponentFlowRate(
                    state, mdot, fanCoil.HeatCoilFluidInletNode, fanCoil.HeatCoilFluidOutletNodeNum, fanCoil.HeatCoilPlantLoc);
                if (state.dataPlnt->PlantLoop(fanCoil.HeatCoilPlantLoc.loopNum).LoopSide(fanCoil.HeatCoilPlantLoc.loopSideNum).FlowLock ==
                    DataPlant::FlowLock::Locked) {
                    HotFlowLocked = true; // save locked flow
                }
            }

            // obtain unit output with no active heating/cooling
            Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOutNoHC, 0.0);

            // get the loads at the coil
            QCoilHeatSP = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlledZoneNum).RemainingOutputReqToHeatSP - QUnitOutNoHC;
            QCoilCoolSP = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlledZoneNum).RemainingOutputReqToCoolSP - QUnitOutNoHC;

            // speed fan selection only for multispeed cycling fan
            if (UnitOn && (fanCoil.CapCtrlMeth_Num == CCM::CycFan)) {
                QZnReq = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlledZoneNum).RemainingOutputRequired;

                // set water side mass flow rate
                if (QCoilCoolSP < 0) {
                    state.dataLoopNodes->Node(fanCoil.CoolCoilFluidInletNode).MassFlowRate = fanCoil.MaxCoolCoilFluidFlow;
                } else if (QCoilHeatSP > 0 && fanCoil.HCoilType_Num != HCoil::Electric) {
                    state.dataLoopNodes->Node(fanCoil.HeatCoilFluidInletNode).MassFlowRate = fanCoil.MaxHeatCoilFluidFlow;
                }

                state.dataLoopNodes->Node(InletNode).MassFlowRateMax = fanCoil.LowSpeedRatio * fanCoil.MaxAirMassFlow;
                fanCoil.SpeedFanSel = 1;
                fanCoil.SpeedFanRatSel = fanCoil.LowSpeedRatio;
                Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOutMax);
                if (std::abs(QUnitOutMax) < std::abs(QZnReq)) {
                    state.dataLoopNodes->Node(InletNode).MassFlowRateMax = fanCoil.MedSpeedRatio * fanCoil.MaxAirMassFlow;
                    fanCoil.SpeedFanSel = 2;
                    fanCoil.SpeedFanRatSel = fanCoil.MedSpeedRatio;
                    Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOutMax);
                }
                if (std::abs(QUnitOutMax) < std::abs(QZnReq)) {
                    fanCoil.SpeedFanSel = 3;
                    fanCoil.SpeedFanRatSel = 1.0;
                    state.dataLoopNodes->Node(InletNode).MassFlowRateMax = fanCoil.MaxAirMassFlow;
                }
            } else {
                fanCoil.SpeedFanSel = 0;
            }

            // meet the coil load adjusted for fan operation
            if (UnitOn && QCoilCoolSP < (-1.0 * HVAC::SmallLoad) &&
                state.dataHeatBalFanSys->TempControlType(ControlledZoneNum) != HVAC::SetptType::SingleHeat) {
                // cooling coil action, maximum cold water flow
                mdot = fanCoil.MaxCoolCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(
                    state, mdot, fanCoil.CoolCoilFluidInletNode, fanCoil.CoolCoilFluidOutletNodeNum, fanCoil.CoolCoilPlantLoc);

                QZnReq = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlledZoneNum).RemainingOutputReqToCoolSP;
                ControlOffset = fanCoil.ColdControlOffset;

                // get the maximum output of the fcu
                Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOutMax); // call without PLR means PLR = 1

                if (QUnitOutMax < QZnReq) {
                    // more cooling than required, find reduced air and water flow rate to meet the load
                    // solve for the cold water flow rate with no limit set by flow rate lockdown
                    auto f = [&state, FanCoilNum, FirstHVACIteration, ControlledZoneNum, QZnReq](Real64 const PLR) {
                        return CalcFanCoilPLRResidual(state,
                                                      PLR,
                                                      FanCoilNum,
                                                      FirstHVACIteration,
                                                      ControlledZoneNum,
                                                      state.dataFanCoilUnits->FanCoil(FanCoilNum).CoolCoilFluidInletNode,
                                                      QZnReq);
                    };
                    General::SolveRoot(state, 0.001, MaxIterCycl, SolFlag, PLR, f, 0.0, 1.0);
                    if (SolFlag == -1) {
                        // tighten limits on water flow rate to see if this allows convergence
                        state.dataFanCoilUnits->CoolingLoad = true;
                        state.dataFanCoilUnits->HeatingLoad = false;
                        TightenAirAndWaterFlowLimits(state,
                                                     FanCoilNum,
                                                     state.dataFanCoilUnits->CoolingLoad,
                                                     state.dataFanCoilUnits->HeatingLoad,
                                                     fanCoil.CoolCoilFluidInletNode,
                                                     ControlledZoneNum,
                                                     FirstHVACIteration,
                                                     QZnReq,
                                                     PLRMin,
                                                     PLRMax);
                        General::SolveRoot(state, 0.001, MaxIterCycl, SolFlag, PLR, f, PLRMin, PLRMax);
                        if (SolFlag == -1) {
                            ++fanCoil.ConvgErrCountC;
                            if (fanCoil.ConvgErrCountC < 2) {
                                ShowWarningError(state, format("Part-load ratio cooling control failed in fan coil unit {}", fanCoil.Name));
                                ShowContinueError(state, "  Iteration limit exceeded in calculating FCU part-load ratio ");
                                state.dataLoopNodes->Node(fanCoil.CoolCoilFluidInletNode).MassFlowRate = PLR * fanCoil.MaxCoolCoilFluidFlow;
                                Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut, PLR);
                                ShowContinueErrorTimeStamp(state, format("Load Request = {}, Final Capacity = {}", QZnReq, QUnitOut));
                                ShowContinueErrorTimeStamp(
                                    state,
                                    format("Min part-load used during iterations = {}, Max part-load used during iterations = {}", PLRMin, PLRMax));
                                ShowContinueErrorTimeStamp(state, format("Part-load ratio on last iteration = {}", PLR));
                                ShowContinueErrorTimeStamp(state, "..Part-load ratio set to last iteration value ");
                            } else {
                                ShowRecurringWarningErrorAtEnd(state,
                                                               "Part-load ratio cooling iteration limit exceeded in fan coil unit " + fanCoil.Name,
                                                               fanCoil.MaxIterIndexC);
                            }
                        } else if (SolFlag == -2) {
                            ++fanCoil.LimitErrCountC;
                            if (fanCoil.LimitErrCountC < 2) {
                                ShowWarningError(state, format("Part-load ratio cooling control failed in fan coil unit {}", fanCoil.Name));
                                ShowContinueError(state, "  Bad part-load ratio limits");
                                ShowContinueErrorTimeStamp(state, format("..Part-load ratio set to {}", PLRMin));
                            } else {
                                ShowRecurringWarningErrorAtEnd(
                                    state, "Part-load ratio cooling control failed in fan coil unit " + fanCoil.Name, fanCoil.BadMassFlowLimIndexC);
                            }
                        }
                    } else if (SolFlag == -2) {
                        ++fanCoil.LimitErrCountC;
                        if (fanCoil.LimitErrCountC < 2) {
                            ShowWarningError(state, format("Part-load ratio control failed in fan coil unit {}", fanCoil.Name));
                            ShowContinueError(state, "  Bad part-load ratio limits");
                            ShowContinueErrorTimeStamp(state, "..Part-load ratio set to 0");
                        } else {
                            ShowRecurringWarningErrorAtEnd(
                                state, "Part-load ratio control failed in fan coil unit " + fanCoil.Name, fanCoil.BadMassFlowLimIndexC);
                        }
                    }
                    mdot = PLR * fanCoil.MaxCoolCoilFluidFlow;
                    PlantUtilities::SetComponentFlowRate(
                        state, mdot, fanCoil.CoolCoilFluidInletNode, fanCoil.CoolCoilFluidOutletNodeNum, fanCoil.CoolCoilPlantLoc);
                } else {
                    PLR = 1.0;
                    mdot = PLR * fanCoil.MaxCoolCoilFluidFlow;
                    PlantUtilities::SetComponentFlowRate(
                        state, mdot, fanCoil.CoolCoilFluidInletNode, fanCoil.CoolCoilFluidOutletNodeNum, fanCoil.CoolCoilPlantLoc);
                }

                // at the end calculate output
                Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut, PLR);

            } else if (UnitOn && QCoilHeatSP > HVAC::SmallLoad &&
                       state.dataHeatBalFanSys->TempControlType(ControlledZoneNum) != HVAC::SetptType::SingleCool) {
                // heating coil action, maximun hot water flow

                if (fanCoil.HCoilType_Num == HCoil::Water) {
                    mdot = fanCoil.MaxHeatCoilFluidFlow;
                    PlantUtilities::SetComponentFlowRate(
                        state, mdot, fanCoil.HeatCoilFluidInletNode, fanCoil.HeatCoilFluidOutletNodeNum, fanCoil.HeatCoilPlantLoc);
                }

                QZnReq = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlledZoneNum).RemainingOutputReqToHeatSP;
                ControlOffset = fanCoil.HotControlOffset;

                // get the maximum output of the fcu
                Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOutMax);
                // calculate the PLR, if load greater than output, PLR = 1 (output = max)
                if (QUnitOutMax > QZnReq) {
                    // more heating than required, find reduced water flow rate to meet the load
                    if (fanCoil.HCoilType_Num == HCoil::Water) {
                        // solve for the hot water flow rate with no limit set by flow rate lockdown
                        auto f = [&state, FanCoilNum, FirstHVACIteration, ControlledZoneNum, QZnReq](Real64 const PLR) {
                            return CalcFanCoilPLRResidual(state,
                                                          PLR,
                                                          FanCoilNum,
                                                          FirstHVACIteration,
                                                          ControlledZoneNum,
                                                          state.dataFanCoilUnits->FanCoil(FanCoilNum).HeatCoilFluidInletNode,
                                                          QZnReq);
                        };
                        General::SolveRoot(state, 0.001, MaxIterCycl, SolFlag, PLR, f, 0.0, 1.0);
                        if (SolFlag == -1) {
                            // tighten limits on water flow rate to see if this allows convergence
                            state.dataFanCoilUnits->CoolingLoad = false;
                            state.dataFanCoilUnits->HeatingLoad = true;
                            TightenAirAndWaterFlowLimits(state,
                                                         FanCoilNum,
                                                         state.dataFanCoilUnits->CoolingLoad,
                                                         state.dataFanCoilUnits->HeatingLoad,
                                                         fanCoil.HeatCoilFluidInletNode,
                                                         ControlledZoneNum,
                                                         FirstHVACIteration,
                                                         QZnReq,
                                                         PLRMin,
                                                         PLRMax);
                            General::SolveRoot(state, 0.001, MaxIterCycl, SolFlag, PLR, f, PLRMin, PLRMax);
                            if (SolFlag == -1) {
                                ++fanCoil.ConvgErrCountH;
                                if (fanCoil.ConvgErrCountH < 2) {
                                    ShowWarningError(state, format("Part-load ratio heating control failed in fan coil unit {}", fanCoil.Name));
                                    ShowContinueError(state, "  Iteration limit exceeded in calculating FCU part-load ratio ");
                                    state.dataLoopNodes->Node(fanCoil.HeatCoilFluidInletNode).MassFlowRate = PLR * fanCoil.MaxHeatCoilFluidFlow;
                                    Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut, PLR);
                                    ShowContinueErrorTimeStamp(state, format("Load Request = {}, Final Capacity = {}", QZnReq, QUnitOut));
                                    ShowContinueErrorTimeStamp(
                                        state,
                                        format("Min part-load ratio used during iterations = {}, Max part-load used during iterations = {}",
                                               PLRMin,
                                               PLRMax));
                                    ShowContinueErrorTimeStamp(state, format("Part-load ratio on last iteration = {}", PLR));
                                    ShowContinueErrorTimeStamp(state, "..Part-load ratio set to last iteration value ");
                                } else {
                                    ShowRecurringWarningErrorAtEnd(state,
                                                                   "Part-load ratio heating iteration limit exceeded in fan coil unit " +
                                                                       fanCoil.Name,
                                                                   fanCoil.MaxIterIndexH);
                                }
                            } else if (SolFlag == -2) {
                                ++fanCoil.LimitErrCountH;
                                if (fanCoil.LimitErrCountH < 2) {
                                    ShowWarningError(state, format("Part-load ratio heating control failed in fan coil unit {}", fanCoil.Name));
                                    ShowContinueError(state, "  Bad hot part-load ratio limits");
                                    ShowContinueErrorTimeStamp(state, format("..Part-load ratio set to {}", PLRMin));
                                } else {
                                    ShowRecurringWarningErrorAtEnd(state,
                                                                   "Part-load ratio heating control failed in fan coil unit " + fanCoil.Name,
                                                                   fanCoil.BadMassFlowLimIndexH);
                                }
                            }
                        } else if (SolFlag == -2) {
                            ++fanCoil.LimitErrCountH;
                            if (fanCoil.LimitErrCountH < 2) {
                                ShowWarningError(state, format("Part-load ratio heating control failed in fan coil unit {}", fanCoil.Name));
                                ShowContinueError(state, "  Bad part-load ratio limits");
                                ShowContinueErrorTimeStamp(state, "..Part-load ratio set to 0");
                            } else {
                                ShowRecurringWarningErrorAtEnd(
                                    state, "Part-load ratio heating control failed in fan coil unit " + fanCoil.Name, fanCoil.BadMassFlowLimIndexH);
                            }
                        }
                        HWFlow = PLR * fanCoil.MaxHeatCoilFluidFlow;
                        PlantUtilities::SetComponentFlowRate(
                            state, HWFlow, fanCoil.HeatCoilFluidInletNode, fanCoil.HeatCoilFluidOutletNodeNum, fanCoil.HeatCoilPlantLoc);

                    } else {
                        auto f = [&state, FirstHVACIteration, FanCoilNum, ControlledZoneNum, QZnReq](Real64 const PartLoadRatio) {
                            return CalcFanCoilLoadResidual(state, FanCoilNum, FirstHVACIteration, ControlledZoneNum, QZnReq, PartLoadRatio);
                        };
                        General::SolveRoot(state, 0.001, MaxIterCycl, SolFlag, PLR, f, 0.0, 1.0);
                    }
                } else {
                    PLR = 1.0;
                    if (fanCoil.HCoilType_Num == HCoil::Water) {
                        mdot = PLR * fanCoil.MaxHeatCoilFluidFlow;
                        PlantUtilities::SetComponentFlowRate(
                            state, mdot, fanCoil.HeatCoilFluidInletNode, fanCoil.HeatCoilFluidOutletNodeNum, fanCoil.HeatCoilPlantLoc);
                    }
                }

                // at the end calculate output with adjusted PLR
                Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut, PLR);

            } else {
                // no action, zero the air flow rate, the unit is off
                state.dataLoopNodes->Node(InletNode).MassFlowRate = 0.0;
                state.dataLoopNodes->Node(OutletNode).MassFlowRate = 0.0;
                fanCoil.SpeedFanSel = 0;
                PLR = 0.0;
                Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut, PLR);
            }

            AirMassFlow = state.dataLoopNodes->Node(InletNode).MassFlowRate;
            // CR9155 Remove specific humidity calculations
            SpecHumOut = state.dataLoopNodes->Node(OutletNode).HumRat;
            SpecHumIn = state.dataLoopNodes->Node(InletNode).HumRat;
            LatentOutput = AirMassFlow * (SpecHumOut - SpecHumIn); // Latent rate (kg/s), dehumid = negative
            QTotUnitOut = AirMassFlow * (state.dataLoopNodes->Node(OutletNode).Enthalpy - state.dataLoopNodes->Node(InletNode).Enthalpy);
            // report variables
            fanCoil.HeatPower = max(0.0, QUnitOut);
            fanCoil.SensCoolPower = std::abs(min(DataPrecisionGlobals::constant_zero, QUnitOut));
            fanCoil.TotCoolPower = std::abs(min(DataPrecisionGlobals::constant_zero, QTotUnitOut));
            fanCoil.ElecPower = state.dataFans->fans(fanCoil.FanIndex)->totalPower;

            fanCoil.PLR = PLR;
            PowerMet = QUnitOut;
            LatOutputProvided = LatentOutput;

        } break;
        case CCM::ASHRAE: {

            if (AirMassFlow < HVAC::SmallMassFlow) {
                UnitOn = false;
            }

            //  zero the hot & cold water flows
            mdot = 0.0;
            PlantUtilities::SetComponentFlowRate(
                state, mdot, fanCoil.CoolCoilFluidInletNode, fanCoil.CoolCoilFluidOutletNodeNum, fanCoil.CoolCoilPlantLoc);

            if (fanCoil.HCoilType_Num == HCoil::Water) {
                mdot = 0.0;
                PlantUtilities::SetComponentFlowRate(
                    state, mdot, fanCoil.HeatCoilFluidInletNode, fanCoil.HeatCoilFluidOutletNodeNum, fanCoil.HeatCoilPlantLoc);
            }

            OAMassFlow = 0.0;

            // determine minimum outdoor air flow rate
            if (fanCoil.DSOAPtr > 0 && fanCoil.OutsideAirNode > 0) {
                OAVolumeFlowRate = DataSizing::calcDesignSpecificationOutdoorAir(state, fanCoil.DSOAPtr, ControlledZoneNum, true, true);
                RhoAir = Psychrometrics::PsyRhoAirFnPbTdbW(state,
                                                           state.dataLoopNodes->Node(fanCoil.OutsideAirNode).Press,
                                                           state.dataLoopNodes->Node(fanCoil.OutsideAirNode).Temp,
                                                           state.dataLoopNodes->Node(fanCoil.OutsideAirNode).HumRat);
                OAMassFlow = OAVolumeFlowRate * RhoAir;
            }

            MinSAMassFlowRate = min(max(OAMassFlow, fanCoil.MaxAirMassFlow * fanCoil.LowSpeedRatio), fanCoil.MaxAirMassFlow);
            MaxSAMassFlowRate = fanCoil.MaxAirMassFlow;
            state.dataFanCoilUnits->HeatingLoad = false;
            state.dataFanCoilUnits->CoolingLoad = false;
            if (UnitOn) {
                state.dataLoopNodes->Node(InletNode).MassFlowRate = MinSAMassFlowRate;
                fanCoil.MaxNoCoolHeatAirMassFlow = MinSAMassFlowRate;
                fanCoil.MaxCoolAirMassFlow = MaxSAMassFlowRate;
                fanCoil.MaxHeatAirMassFlow = MaxSAMassFlowRate;
                fanCoil.LowSpeedCoolFanRatio = MinSAMassFlowRate / MaxSAMassFlowRate;
                fanCoil.LowSpeedHeatFanRatio = MinSAMassFlowRate / MaxSAMassFlowRate;

                Calc4PipeFanCoil(state,
                                 FanCoilNum,
                                 ControlledZoneNum,
                                 FirstHVACIteration,
                                 QUnitOutNoHC,
                                 0.0); // needs PLR=0 for electric heating coil, otherwise will run at full capacity

                QCoilCoolSP = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlledZoneNum).RemainingOutputReqToCoolSP;
                QCoilHeatSP = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlledZoneNum).RemainingOutputReqToHeatSP;

                if (QCoilHeatSP > 0.0 && QCoilCoolSP > 0.0 &&
                    state.dataHeatBalFanSys->TempControlType(ControlledZoneNum) != HVAC::SetptType::SingleCool) {
                    QZnReq = QCoilHeatSP;
                    state.dataFanCoilUnits->HeatingLoad = true;
                } else if (QCoilHeatSP > 0.0 && QCoilCoolSP > 0.0 &&
                           state.dataHeatBalFanSys->TempControlType(ControlledZoneNum) == HVAC::SetptType::SingleCool) {
                    QZnReq = 0.0;
                } else if (QCoilHeatSP < 0.0 && QCoilCoolSP < 0.0 &&
                           state.dataHeatBalFanSys->TempControlType(ControlledZoneNum) != HVAC::SetptType::SingleHeat) {
                    QZnReq = QCoilCoolSP;
                    state.dataFanCoilUnits->CoolingLoad = true;
                } else if (QCoilHeatSP < 0.0 && QCoilCoolSP < 0.0 &&
                           state.dataHeatBalFanSys->TempControlType(ControlledZoneNum) == HVAC::SetptType::SingleHeat) {
                    QZnReq = 0.0;
                } else if (QCoilHeatSP <= 0.0 && QCoilCoolSP >= 0.0) {
                    QZnReq = 0.0;
                }
            }

            if (state.dataFanCoilUnits->CoolingLoad) {

                state.dataLoopNodes->Node(InletNode).MassFlowRate = MaxSAMassFlowRate;

                mdot = fanCoil.MaxCoolCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(
                    state, mdot, fanCoil.CoolCoilFluidInletNode, fanCoil.CoolCoilFluidOutletNodeNum, fanCoil.CoolCoilPlantLoc);

            } else if (state.dataFanCoilUnits->HeatingLoad) {

                state.dataLoopNodes->Node(InletNode).MassFlowRate = MaxSAMassFlowRate;

                if (fanCoil.HCoilType_Num == HCoil::Water) {
                    mdot = fanCoil.MaxHeatCoilFluidFlow;
                    PlantUtilities::SetComponentFlowRate(
                        state, mdot, fanCoil.HeatCoilFluidInletNode, fanCoil.HeatCoilFluidOutletNodeNum, fanCoil.HeatCoilPlantLoc);
                }
            }

            Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOutMax);

            if ((state.dataFanCoilUnits->CoolingLoad && QUnitOutMax < QZnReq) || (state.dataFanCoilUnits->HeatingLoad && QUnitOutMax > QZnReq)) {
                if ((state.dataFanCoilUnits->CoolingLoad && QUnitOutNoHC < QZnReq) ||
                    (state.dataFanCoilUnits->HeatingLoad && QUnitOutNoHC > QZnReq)) {
                    PLR = 0.0;
                    fanCoil.FanPartLoadRatio = 0.0; // set SZVAV model variable
                    state.dataLoopNodes->Node(InletNode).MassFlowRate =
                        MinSAMassFlowRate; // = min air flow rate + ((max-min) air flow rate * FanPartLoadRatio)
                    mdot = 0.0;
                    PlantUtilities::SetComponentFlowRate(
                        state, mdot, fanCoil.CoolCoilFluidInletNode, fanCoil.CoolCoilFluidOutletNodeNum, fanCoil.CoolCoilPlantLoc);

                    if (fanCoil.HCoilType_Num == HCoil::Water) {
                        mdot = 0.0;
                        PlantUtilities::SetComponentFlowRate(
                            state, mdot, fanCoil.HeatCoilFluidInletNode, fanCoil.HeatCoilFluidOutletNodeNum, fanCoil.HeatCoilPlantLoc);
                    }
                } else {
                    Real64 OnOffAirFlowRatio = 1.0;
                    bool HXUnitOn = false;
                    int AirLoopNum = 0;
                    HVAC::CompressorOp CompressorOnFlag = HVAC::CompressorOp::Off;
                    auto &SZVAVModel(fanCoil);
                    // seems like passing these (arguments 2-n) as an array (similar to Par) would make this more uniform across different
                    // models
                    SZVAVModel::calcSZVAVModel(state,
                                               SZVAVModel,
                                               FanCoilNum,
                                               FirstHVACIteration,
                                               state.dataFanCoilUnits->CoolingLoad,
                                               state.dataFanCoilUnits->HeatingLoad,
                                               QZnReq,
                                               OnOffAirFlowRatio,
                                               HXUnitOn,
                                               AirLoopNum,
                                               PLR,
                                               CompressorOnFlag);
                }
            } else if ((state.dataFanCoilUnits->CoolingLoad && QUnitOutMax > QZnReq && QZnReq < 0.0) ||
                       (state.dataFanCoilUnits->HeatingLoad && QUnitOutMax < QZnReq && QZnReq > 0.0)) {
                // load is larger than capacity, thus run the fancoil unit at full capacity
                PLR = 1.0;
            }
            Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut, PLR);
            PowerMet = QUnitOut;
            AirMassFlow = state.dataLoopNodes->Node(InletNode).MassFlowRate;
            // CR9155 Remove specific humidity calculations
            SpecHumOut = state.dataLoopNodes->Node(OutletNode).HumRat;
            SpecHumIn = state.dataLoopNodes->Node(InletNode).HumRat;
            // Latent rate (kg/s), dehumid = negative
            LatOutputProvided = AirMassFlow * (SpecHumOut - SpecHumIn);
            fanCoil.PLR = PLR;

            // cycling fan constant water flow AND VarFanVarFlow
        } break;
        case CCM::VarFanConsFlow: {

            if (state.dataZoneEnergyDemand->CurDeadBandOrSetback(ControlledZoneNum) || AirMassFlow < HVAC::SmallMassFlow) {
                UnitOn = false;
            }

            //  zero the hot & cold water flows
            //    Node(fanCoil%CoolCoilFluidInletNode)%MassFlowRate = 0.0
            //    Node(fanCoil%HeatCoilFluidInletNode)%MassFlowRate = 0.0
            mdot = 0.0;
            PlantUtilities::SetComponentFlowRate(
                state, mdot, fanCoil.CoolCoilFluidInletNode, fanCoil.CoolCoilFluidOutletNodeNum, fanCoil.CoolCoilPlantLoc);

            if (fanCoil.HCoilType_Num == HCoil::Water) {
                mdot = 0.0;
                PlantUtilities::SetComponentFlowRate(
                    state, mdot, fanCoil.HeatCoilFluidInletNode, fanCoil.HeatCoilFluidOutletNodeNum, fanCoil.HeatCoilPlantLoc);
            }
            Calc4PipeFanCoil(state,
                             FanCoilNum,
                             ControlledZoneNum,
                             FirstHVACIteration,
                             QUnitOutNoHC,
                             0.0); // needs PLR=0 for electric heating coil, otherwise will run at full capacity

            int Iter = 0;
            if (UnitOn && state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlledZoneNum).RemainingOutputReqToCoolSP < (-1.0 * HVAC::SmallLoad) &&
                state.dataHeatBalFanSys->TempControlType(ControlledZoneNum) != HVAC::SetptType::SingleHeat) {
                // cooling coil action, maximum cold water flow
                mdot = fanCoil.MaxCoolCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(
                    state, mdot, fanCoil.CoolCoilFluidInletNode, fanCoil.CoolCoilFluidOutletNodeNum, fanCoil.CoolCoilPlantLoc);
                QZnReq = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlledZoneNum).RemainingOutputReqToCoolSP;
                ControlOffset = fanCoil.ColdControlOffset;

                // get the maximum output of the fcu
                Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOutMax);
                // calculate the PLR, if load greater than output, PLR = 1 (output = max)
                if (QUnitOutMax != 0.0) {
                    PLR = std::abs(QZnReq / QUnitOutMax);
                }
                if (PLR > 1.0) {
                    PLR = 1.0;
                }

                // adjust the PLR to meet the cooling load calling Calc4PipeFanCoil repeatedly with the PLR adjusted
                while (std::abs(Error) > ControlOffset && std::abs(AbsError) > HVAC::SmallLoad && Iter < MaxIterCycl && PLR != 1.0) {
                    Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut, PLR);
                    Error = (QZnReq - QUnitOut) / QZnReq;
                    AbsError = QZnReq - QUnitOut;
                    DelPLR = (QZnReq - QUnitOut) / QUnitOutMax;
                    PLR += Relax * DelPLR;
                    PLR = max(0.0, min(1.0, PLR));
                    ++Iter;
                    if (Iter == 32) {
                        Relax = 0.5;
                    }
                    if (Iter == 65) {
                        Relax = 0.25;
                    }
                }

                // warning if not converged
                if (Iter > (MaxIterCycl - 1)) {
                    if (fanCoil.MaxIterIndexC == 0) {
                        ShowWarningMessage(state,
                                           format("ZoneHVAC:FourPipeFanCoil=\"{}\" -- Exceeded max iterations while adjusting cycling fan sensible "
                                                  "runtime to meet the zone load within the cooling convergence tolerance.",
                                                  fanCoil.Name));
                        ShowContinueErrorTimeStamp(state, format("Iterations={}", MaxIterCycl));
                    }
                    ShowRecurringWarningErrorAtEnd(state,
                                                   "ZoneHVAC:FourPipeFanCoil=\"" + fanCoil.Name +
                                                       "\"  -- Exceeded max iterations error (sensible runtime) continues...",
                                                   fanCoil.MaxIterIndexC);
                }

                // at the end calculate output with adjusted PLR
                Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut, PLR);

            } else if (UnitOn && state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlledZoneNum).RemainingOutputReqToHeatSP > HVAC::SmallLoad &&
                       state.dataHeatBalFanSys->TempControlType(ControlledZoneNum) != HVAC::SetptType::SingleCool) {
                // heating coil action, maximun hot water flow
                if (fanCoil.HCoilType_Num == HCoil::Water) {
                    mdot = fanCoil.MaxHeatCoilFluidFlow;
                    PlantUtilities::SetComponentFlowRate(
                        state, mdot, fanCoil.HeatCoilFluidInletNode, fanCoil.HeatCoilFluidOutletNodeNum, fanCoil.HeatCoilPlantLoc);
                }
                QZnReq = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ControlledZoneNum).RemainingOutputReqToHeatSP;
                ControlOffset = fanCoil.HotControlOffset;

                // get the maximum output of the fcu
                Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOutMax);
                // calculate the PLR, if load greater than output, PLR = 1 (output = max)
                if (QUnitOutMax != 0.0) {
                    PLR = std::abs(QZnReq / QUnitOutMax);
                }
                if (PLR > 1.0) {
                    PLR = 1.0;
                }

                // adjust the PLR to meet the heating load calling Calc4PipeFanCoil repeatedly with the PLR adjusted
                while (std::abs(Error) > ControlOffset && std::abs(AbsError) > HVAC::SmallLoad && Iter < MaxIterCycl && PLR != 1.0) {
                    Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut, PLR);
                    Error = (QZnReq - QUnitOut) / QZnReq;
                    AbsError = QZnReq - QUnitOut;
                    DelPLR = (QZnReq - QUnitOut) / QUnitOutMax;
                    PLR += Relax * DelPLR;
                    PLR = max(0.0, min(1.0, PLR));
                    ++Iter;
                    if (Iter == 32) {
                        Relax = 0.5;
                    }
                    if (Iter == 65) {
                        Relax = 0.25;
                    }
                }

                // warning if not converged
                if (Iter > (MaxIterCycl - 1)) {
                    if (fanCoil.MaxIterIndexH == 0) {
                        ShowWarningMessage(state,
                                           format("ZoneHVAC:FourPipeFanCoil=\"{}\" -- Exceeded max iterations while adjusting cycling fan sensible "
                                                  "runtime to meet the zone load within the heating convergence tolerance.",
                                                  fanCoil.Name));
                        ShowContinueError(state, format("...Requested zone load = {:.3T} [W]", QZnReq));
                        ShowContinueError(state, format("...Fan coil capacity   = {:.3T} [W]", QUnitOut));
                        ShowContinueErrorTimeStamp(state, format("Iterations={}", MaxIterCycl));
                    }
                    ShowRecurringWarningErrorAtEnd(state,
                                                   "ZoneHVAC:FourPipeFanCoil=\"" + fanCoil.Name +
                                                       "\"  -- Exceeded max iterations error (sensible runtime) continues...",
                                                   fanCoil.MaxIterIndexH);
                }

                // at the end calculate output with adjusted PLR
                Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut, PLR);

                // this part of the code is just if we want ventilation in the deadband zone
                // ELSE IF (AirMassFlow .gt. 0.0d0) THEN
                // if fan scheduled available : just ventilation, PLR = 1
                // QUnitOut = QUnitOutNOHC
                // PLR = 1.

            } else {
                // no action, zero the air flow rate, the unit is off
                state.dataLoopNodes->Node(InletNode).MassFlowRate = 0.0;
                state.dataLoopNodes->Node(OutletNode).MassFlowRate = 0.0;
                fanCoil.SpeedFanSel = 0;
                PLR = 0.0;
                Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut, PLR);
            }

            AirMassFlow = state.dataLoopNodes->Node(InletNode).MassFlowRate;
            // CR9155 Remove specific humidity calculations
            SpecHumOut = state.dataLoopNodes->Node(OutletNode).HumRat;
            SpecHumIn = state.dataLoopNodes->Node(InletNode).HumRat;
            LatentOutput = AirMassFlow * (SpecHumOut - SpecHumIn); // Latent rate (kg/s), dehumid = negative
            QSensUnitOutNoATM = calcZoneSensibleOutput(AirMassFlow,
                                                       state.dataLoopNodes->Node(OutletNode).Temp,
                                                       state.dataLoopNodes->Node(InletNode).Temp,
                                                       state.dataLoopNodes->Node(InletNode).HumRat);
            QTotUnitOut = AirMassFlow * (state.dataLoopNodes->Node(OutletNode).Enthalpy - state.dataLoopNodes->Node(InletNode).Enthalpy);
            // report variables
            fanCoil.HeatPower = max(0.0, QSensUnitOutNoATM);
            fanCoil.SensCoolPower = std::abs(min(DataPrecisionGlobals::constant_zero, QSensUnitOutNoATM));
            fanCoil.TotCoolPower = std::abs(min(DataPrecisionGlobals::constant_zero, QTotUnitOut));
            fanCoil.ElecPower = state.dataFans->fans(fanCoil.FanIndex)->totalPower;

            fanCoil.PLR = PLR;
            PowerMet = QUnitOut;
            LatOutputProvided = LatentOutput;

        } break;
        case CCM::MultiSpeedFan: {
            // call multi-speed fan staging calculation
            SimMultiStage4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut);
            AirMassFlow = state.dataLoopNodes->Node(InletNode).MassFlowRate;
            SpecHumOut = state.dataLoopNodes->Node(OutletNode).HumRat;
            SpecHumIn = state.dataLoopNodes->Node(InletNode).HumRat;
            LatentOutput = AirMassFlow * (SpecHumOut - SpecHumIn); // Latent rate (kg/s), dehumid = negative
            QSensUnitOutNoATM = calcZoneSensibleOutput(AirMassFlow,
                                                       state.dataLoopNodes->Node(OutletNode).Temp,
                                                       state.dataLoopNodes->Node(InletNode).Temp,
                                                       state.dataLoopNodes->Node(InletNode).HumRat);
            QTotUnitOut = AirMassFlow * (state.dataLoopNodes->Node(OutletNode).Enthalpy - state.dataLoopNodes->Node(InletNode).Enthalpy);
            // report variables
            fanCoil.HeatPower = max(0.0, QSensUnitOutNoATM);
            fanCoil.SensCoolPower = std::abs(min(DataPrecisionGlobals::constant_zero, QSensUnitOutNoATM));
            fanCoil.TotCoolPower = std::abs(min(DataPrecisionGlobals::constant_zero, QTotUnitOut));
            fanCoil.ElecPower = state.dataFans->fans(fanCoil.FanIndex)->totalPower;

            PowerMet = QUnitOut;
            LatOutputProvided = LatentOutput;
        } break;
        default:
            break;
        }
    }

    void TightenWaterFlowLimits(EnergyPlusData &state,
                                int const FanCoilNum,          // Unit index in fan coil array
                                bool const CoolingLoad,        // true if zone requires cooling
                                bool const HeatingLoad,        // true if zone requires heating
                                int const WaterControlNode,    // water control node, either cold or hot water
                                int const ControlledZoneNum,   // controlling zone index
                                bool const FirstHVACIteration, //  TRUE if 1st HVAC simulation of system timestep
                                Real64 const QZnReq,           // zone load [W]
                                Real64 &MinWaterFlow,          // minimum water flow rate
                                Real64 &MaxWaterFlow           // maximum water flow rate
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         R. Raustad, FSEC
        //       DATE WRITTEN   May 2016

        // PURPOSE OF THIS SUBROUTINE:
        // Find tighter limits of water flow rate for fan coil unit.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 QUnitOut; // fan coil delivered capacity [W]
        Real64 mdot;     // water flow rate passed to fan coil unit [kg/s]

        // RegulaFalsi can reach max iteration when low water flow rate is required to meet load. Test at 10% of flow before iterating
        mdot = MaxWaterFlow * 0.1;
        state.dataLoopNodes->Node(WaterControlNode).MassFlowRate = mdot;
        Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut);
        if ((CoolingLoad && QUnitOut < QZnReq) || (HeatingLoad && QUnitOut > QZnReq)) {
            MaxWaterFlow = mdot;
            // RegulaFalsi can reach max iteration when low water flow rate is required to meet load. Test at 1% of flow before iterating
            mdot *= 0.1;
            state.dataLoopNodes->Node(WaterControlNode).MassFlowRate = mdot;
            Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut);
            if ((CoolingLoad && QUnitOut < QZnReq) || (HeatingLoad && QUnitOut > QZnReq)) {
                MaxWaterFlow = mdot;
                // RegulaFalsi can reach max iteration when low water flow rate is required to meet load. Test at 0.1% of flow before iterating
                mdot *= 0.1;
                state.dataLoopNodes->Node(WaterControlNode).MassFlowRate = mdot;
                Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut);
                if ((CoolingLoad && QUnitOut < QZnReq) || (HeatingLoad && QUnitOut > QZnReq)) {
                    MaxWaterFlow = mdot;
                    // RegulaFalsi can reach max iteration when low water flow rate is required to meet load. Test at 0.01% of flow before iterating
                    mdot *= 0.1;
                    state.dataLoopNodes->Node(WaterControlNode).MassFlowRate = mdot;
                    Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut);
                    if ((CoolingLoad && QUnitOut < QZnReq) || (HeatingLoad && QUnitOut > QZnReq)) {
                        MaxWaterFlow = mdot;
                        // RegulaFalsi can reach max iteration when low water flow rate is required to meet load. Test at 0.001% of flow before
                        // iterating
                        mdot *= 0.1;
                        state.dataLoopNodes->Node(WaterControlNode).MassFlowRate = mdot;
                        Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut);
                        if ((CoolingLoad && QUnitOut < QZnReq) || (HeatingLoad && QUnitOut > QZnReq)) {
                            MaxWaterFlow = mdot;
                        } else {
                            MinWaterFlow = mdot;
                        }
                    } else {
                        MinWaterFlow = mdot;
                    }
                } else {
                    MinWaterFlow = mdot;
                }
            } else {
                MinWaterFlow = mdot;
            }
        } else {
            MinWaterFlow = mdot;
        }
    }

    void TightenAirAndWaterFlowLimits(EnergyPlusData &state,
                                      int const FanCoilNum,          // Unit index in fan coil array
                                      bool const CoolingLoad,        // true if zone requires cooling
                                      bool const HeatingLoad,        // true if zone requires heating
                                      int const WaterControlNode,    // water control node, either cold or hot water
                                      int const ControlledZoneNum,   // controlling zone index
                                      bool const FirstHVACIteration, //  TRUE if 1st HVAC simulation of system timestep
                                      Real64 const QZnReq,           // zone load [W]
                                      Real64 &PLRMin,                // minimum part-load ratio
                                      Real64 &PLRMax                 // maximum part-load ratio
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         R. Raustad, FSEC
        //       DATE WRITTEN   August 2016

        // PURPOSE OF THIS SUBROUTINE:
        // Find tighter limits of air and water flow rate for fan coil unit.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 QUnitOut; // fan coil delivered capacity [W]

        auto const &fanCoil = state.dataFanCoilUnits->FanCoil(FanCoilNum);

        // RegulaFalsi can reach max iteration when low water flow rate is required to meet load. Test at 100% of flow before iterating
        PLRMin = 0.0;
        PLRMax = 1.0;
        Real64 PLR = 1.0; // operating part-load ratio
        if (WaterControlNode == fanCoil.CoolCoilFluidInletNode) {
            state.dataLoopNodes->Node(WaterControlNode).MassFlowRate = PLR * fanCoil.MaxCoolCoilFluidFlow;
        } else if (WaterControlNode == fanCoil.HeatCoilFluidInletNode && fanCoil.HCoilType_Num != HCoil::Electric) {
            state.dataLoopNodes->Node(WaterControlNode).MassFlowRate = PLR * fanCoil.MaxHeatCoilFluidFlow;
        }
        Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut, PLR);
        if ((CoolingLoad && QUnitOut < QZnReq) || (HeatingLoad && QUnitOut > QZnReq)) {
            PLRMax = PLR;
            PLR *= 0.1;
            // RegulaFalsi can reach max iteration when low water flow rate is required to meet load. Test at 10% of flow before iterating
            if (WaterControlNode == fanCoil.CoolCoilFluidInletNode) {
                state.dataLoopNodes->Node(WaterControlNode).MassFlowRate = PLR * fanCoil.MaxCoolCoilFluidFlow;
            } else if (WaterControlNode == fanCoil.HeatCoilFluidInletNode && fanCoil.HCoilType_Num != HCoil::Electric) {
                state.dataLoopNodes->Node(WaterControlNode).MassFlowRate = PLR * fanCoil.MaxHeatCoilFluidFlow;
            }
            Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut, PLR);
            if ((CoolingLoad && QUnitOut < QZnReq) || (HeatingLoad && QUnitOut > QZnReq)) {
                PLRMax = PLR;
                PLR *= 0.1;
                // RegulaFalsi can reach max iteration when low water flow rate is required to meet load. Test at 1% of flow before iterating
                if (WaterControlNode == fanCoil.CoolCoilFluidInletNode) {
                    state.dataLoopNodes->Node(WaterControlNode).MassFlowRate = PLR * fanCoil.MaxCoolCoilFluidFlow;
                } else if (WaterControlNode == fanCoil.HeatCoilFluidInletNode && fanCoil.HCoilType_Num != HCoil::Electric) {
                    state.dataLoopNodes->Node(WaterControlNode).MassFlowRate = PLR * fanCoil.MaxHeatCoilFluidFlow;
                }
                Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut, PLR);
                if ((CoolingLoad && QUnitOut < QZnReq) || (HeatingLoad && QUnitOut > QZnReq)) {
                    PLRMax = PLR;
                    PLR *= 0.1;
                    // RegulaFalsi can reach max iteration when low water flow rate is required to meet load. Test at 0.1% of flow before iterating
                    if (WaterControlNode == fanCoil.CoolCoilFluidInletNode) {
                        state.dataLoopNodes->Node(WaterControlNode).MassFlowRate = PLR * fanCoil.MaxCoolCoilFluidFlow;
                    } else if (WaterControlNode == fanCoil.HeatCoilFluidInletNode && fanCoil.HCoilType_Num != HCoil::Electric) {
                        state.dataLoopNodes->Node(WaterControlNode).MassFlowRate = PLR * fanCoil.MaxHeatCoilFluidFlow;
                    }
                    Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut, PLR);
                    if ((CoolingLoad && QUnitOut < QZnReq) || (HeatingLoad && QUnitOut > QZnReq)) {
                        PLRMax = PLR;
                        PLR *= 0.1;
                        // RegulaFalsi can reach max iteration when low water flow rate is required to meet load. Test at 0.01% of flow before
                        // iterating
                        if (WaterControlNode == fanCoil.CoolCoilFluidInletNode) {
                            state.dataLoopNodes->Node(WaterControlNode).MassFlowRate = PLR * fanCoil.MaxCoolCoilFluidFlow;
                        } else if (WaterControlNode == fanCoil.HeatCoilFluidInletNode && fanCoil.HCoilType_Num != HCoil::Electric) {
                            state.dataLoopNodes->Node(WaterControlNode).MassFlowRate = PLR * fanCoil.MaxHeatCoilFluidFlow;
                        }
                        Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut, PLR);
                        if ((CoolingLoad && QUnitOut < QZnReq) || (HeatingLoad && QUnitOut > QZnReq)) {
                            PLRMax = PLR;
                        } else {
                            PLRMin = PLR;
                        }
                    } else {
                        PLRMin = PLR;
                    }
                } else {
                    PLRMin = PLR;
                }
            } else {
                PLRMin = PLR;
            }
        } else {
            PLRMin = PLR;
        }
    }

    void Calc4PipeFanCoil(EnergyPlusData &state,
                          int const FanCoilNum,            // Unit index in fan coil array
                          int const ControlledZoneNum,     // ZoneEquipConfig index
                          bool const FirstHVACIteration,   // flag for 1st HVAV iteration in the time step
                          Real64 &LoadMet,                 // load met by unit (watts)
                          ObjexxFCL::Optional<Real64> PLR, // Part Load Ratio, fraction of time step fancoil is on
                          Real64 eHeatCoilCyclingR         // electric heating coil cycling ratio  used with MultiSpeedFan capacity control
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   March 2000
        //       MODIFIED       July 2012, Chandan Sharma - FSEC: Added zone sys avail managers

        // PURPOSE OF THIS SUBROUTINE:
        // Simulate the components making up the 4 pipe fan coil unit.

        // METHODOLOGY EMPLOYED:
        // Simulates the unit components sequentially in the air flow direction.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 AirMassFlow;            // total mass flow through the unit
        Real64 PartLoad;               // if PLR present PartLoad = PLR
        Real64 OASchedValue;           // value of OASchedValue, =1 if not schedule
        Real64 ElecHeaterControl(1.0); // 1 or 0, enables or disables heating coil
        Real64 FanSpeedRatio;          // ratio of actual fan flow to max design fan flow

        auto &fanCoil = state.dataFanCoilUnits->FanCoil(FanCoilNum);

        // if PLR present in arguments, get its value, else default PLR = 1
        if (present(PLR)) {
            PartLoad = PLR;
        } else {
            PartLoad = 1.0;
        }

        int OutletNode = fanCoil.AirOutNode;
        int InletNode = fanCoil.AirInNode;
        state.dataFanCoilUnits->ZoneNode = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ZoneNode;

        // Assume the unit is able to vary the flow. A cycling unit is treated as
        // if it were variable flow, with the flow being the averaqe flow over the time step
        if (((fanCoil.availSched->getCurrentVal() > 0.0 && fanCoil.fanAvailSched->getCurrentVal() > 0.0) || state.dataHVACGlobal->TurnFansOn) &&
            !state.dataHVACGlobal->TurnFansOff) {
            if (fanCoil.CapCtrlMeth_Num != CCM::ConsFanVarFlow) {
                if (fanCoil.CapCtrlMeth_Num != CCM::ASHRAE) {
                    state.dataLoopNodes->Node(InletNode).MassFlowRate = PartLoad * state.dataLoopNodes->Node(InletNode).MassFlowRateMax;
                }
            } else {
                state.dataLoopNodes->Node(InletNode).MassFlowRate = state.dataLoopNodes->Node(InletNode).MassFlowRateMax;
            }
        }

        // use the value of the outside air schedule if present
        OASchedValue = (fanCoil.oaSched != nullptr) ? fanCoil.oaSched->getCurrentVal() : 1.0;

        if (fanCoil.ATMixerExists) {
            state.dataFanCoilUnits->ATMixOutNode = fanCoil.ATMixerOutNode;
            if (fanCoil.ATMixerType == HVAC::MixerType::InletSide) {
                // set the primary air inlet mass flow rate
                state.dataLoopNodes->Node(fanCoil.ATMixerPriNode).MassFlowRate =
                    min(state.dataLoopNodes->Node(fanCoil.ATMixerPriNode).MassFlowRateMaxAvail, state.dataLoopNodes->Node(InletNode).MassFlowRate);
                // now calculate the the mixer outlet conditions (and the secondary air inlet flow rate)
                // the mixer outlet flow rate has already been set above (it is the "inlet" node flow rate)
                SingleDuct::SimATMixer(state, fanCoil.ATMixerName, FirstHVACIteration, fanCoil.ATMixerIndex);
            }
            AirMassFlow = state.dataLoopNodes->Node(InletNode).MassFlowRate;
        } else {
            // OutdoorAir:Mixer
            if (fanCoil.CapCtrlMeth_Num == CCM::CycFan) {
                state.dataLoopNodes->Node(fanCoil.OutsideAirNode).MassFlowRate =
                    min(OASchedValue * state.dataLoopNodes->Node(fanCoil.OutsideAirNode).MassFlowRateMax * PartLoad * fanCoil.SpeedFanRatSel,
                        state.dataLoopNodes->Node(InletNode).MassFlowRate);
            } else if (fanCoil.CapCtrlMeth_Num == CCM::MultiSpeedFan) {
                state.dataLoopNodes->Node(fanCoil.OutsideAirNode).MassFlowRate =
                    min(OASchedValue * state.dataLoopNodes->Node(fanCoil.OutsideAirNode).MassFlowRateMax * PartLoad *
                            state.dataFanCoilUnits->FanFlowRatio,
                        state.dataLoopNodes->Node(InletNode).MassFlowRate);
            } else {
                if (fanCoil.CapCtrlMeth_Num != CCM::ConsFanVarFlow && fanCoil.CapCtrlMeth_Num != CCM::ASHRAE) {
                    state.dataLoopNodes->Node(fanCoil.OutsideAirNode).MassFlowRate =
                        min(OASchedValue * state.dataLoopNodes->Node(fanCoil.OutsideAirNode).MassFlowRateMax * PartLoad,
                            state.dataLoopNodes->Node(InletNode).MassFlowRate);
                } else {
                    state.dataLoopNodes->Node(fanCoil.OutsideAirNode).MassFlowRate =
                        min(OASchedValue * state.dataLoopNodes->Node(fanCoil.OutsideAirNode).MassFlowRateMax,
                            state.dataLoopNodes->Node(InletNode).MassFlowRate);
                }
            }
            state.dataLoopNodes->Node(fanCoil.AirReliefNode).MassFlowRate = state.dataLoopNodes->Node(fanCoil.OutsideAirNode).MassFlowRate;
            AirMassFlow = state.dataLoopNodes->Node(InletNode).MassFlowRate;
            MixedAir::SimOAMixer(state, fanCoil.OAMixName, fanCoil.OAMixIndex);
        }

        if (fanCoil.CapCtrlMeth_Num == CCM::CycFan) {
            // cycling fan coil unit calculation
            if (fanCoil.SpeedFanSel == 1) {
                state.dataFans->fans(fanCoil.FanIndex)->simulate(state, FirstHVACIteration, fanCoil.LowSpeedRatio);
            } else if (fanCoil.SpeedFanSel == 2) {
                state.dataFans->fans(fanCoil.FanIndex)->simulate(state, FirstHVACIteration, fanCoil.MedSpeedRatio);
            } else if (fanCoil.SpeedFanSel == 3) {
                state.dataFans->fans(fanCoil.FanIndex)->simulate(state, FirstHVACIteration, 1.0);
            } else { // using 1.0 here for fan speed ratio seems wrong if FCU max flow rate is different than the fan maximum flow rate
                state.dataFans->fans(fanCoil.FanIndex)->simulate(state, FirstHVACIteration, 0.0, _, 0.0);
            }
            if (fanCoil.CCoilType_Num == CCoil::HXAssist) {
                HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(
                    state, fanCoil.CCoilName, FirstHVACIteration, HVAC::CompressorOp::On, 0.0, fanCoil.CCoilName_Index, HVAC::FanOp::Continuous);
            } else {
                WaterCoils::SimulateWaterCoilComponents(
                    state, fanCoil.CCoilName, FirstHVACIteration, fanCoil.CCoilName_Index, _, HVAC::FanOp::Cycling, PLR);
            }
            if (fanCoil.HCoilType_Num == HCoil::Water) {
                WaterCoils::SimulateWaterCoilComponents(
                    state, fanCoil.HCoilName, FirstHVACIteration, fanCoil.HCoilName_Index, _, HVAC::FanOp::Cycling, PLR);
            } else {
                if (state.dataLoopNodes->Node(fanCoil.CoolCoilFluidInletNode).MassFlowRate > 0.0) {
                    ElecHeaterControl = 0.0;
                }
                HeatingCoils::SimulateHeatingCoilComponents(state,
                                                            fanCoil.HCoilName,
                                                            FirstHVACIteration,
                                                            fanCoil.DesignHeatingCapacity * PartLoad * ElecHeaterControl,
                                                            fanCoil.HCoilName_Index,
                                                            _,
                                                            false,
                                                            HVAC::FanOp::Continuous,
                                                            PartLoad);
            }

        } else if (fanCoil.CapCtrlMeth_Num == CCM::MultiSpeedFan) {
            if (fanCoil.fanType != HVAC::FanType::SystemModel) {
                state.dataFans->fans(fanCoil.FanIndex)->simulate(state, FirstHVACIteration, state.dataFanCoilUnits->FanFlowRatio);
            } else {
                // FanFlowRatio needs to be accurate here for new fan model
                Real64 ActFanFlowRatio = state.dataFanCoilUnits->FanFlowRatio * PartLoad;
                state.dataFans->fans(fanCoil.FanIndex)->simulate(state, FirstHVACIteration, _, _, ActFanFlowRatio);
            }
            if (fanCoil.CCoilType_Num == CCoil::HXAssist) {
                HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(
                    state, fanCoil.CCoilName, FirstHVACIteration, HVAC::CompressorOp::On, 0.0, fanCoil.CCoilName_Index, HVAC::FanOp::Continuous);
            } else {
                WaterCoils::SimulateWaterCoilComponents(
                    state, fanCoil.CCoilName, FirstHVACIteration, fanCoil.CCoilName_Index, _, HVAC::FanOp::Cycling, PLR);
            }
            if (fanCoil.HCoilType_Num == HCoil::Water) {
                WaterCoils::SimulateWaterCoilComponents(
                    state, fanCoil.HCoilName, FirstHVACIteration, fanCoil.HCoilName_Index, _, HVAC::FanOp::Cycling, PLR);
            } else {
                if (state.dataLoopNodes->Node(fanCoil.CoolCoilFluidInletNode).MassFlowRate > 0.0) {
                    ElecHeaterControl = 0.0;
                }
                Real64 QZnReq = 0.0;
                if (fanCoil.fanOp == HVAC::FanOp::Continuous) {
                    QZnReq = fanCoil.DesignHeatingCapacity * state.dataFanCoilUnits->FanFlowRatio * eHeatCoilCyclingR * ElecHeaterControl;
                } else {
                    // proportionally reduce the full flow capacity based on fan flow fraction
                    QZnReq = fanCoil.DesignHeatingCapacity * state.dataFanCoilUnits->FanFlowRatio * PartLoad * eHeatCoilCyclingR * ElecHeaterControl;
                }
                HeatingCoils::SimulateHeatingCoilComponents(state,
                                                            fanCoil.HCoilName,
                                                            FirstHVACIteration,
                                                            QZnReq,
                                                            fanCoil.HCoilName_Index,
                                                            _,
                                                            false,
                                                            fanCoil.fanOp, // fanCoil.FanOpMode, // FanOp::Continuous, FanOp::Cycling
                                                            PartLoad);
            }
        } else { // capacity control method is VariableFanVariableFlow, VariableFanConstantFlow, or ASHRAE90.1

            // calculate fan speed ratio for Fan:OnOff or Fan:SystemModel (not used for other fan types). Only used in fan:OnOff model if performance
            // curves are present.
            FanSpeedRatio = state.dataLoopNodes->Node(InletNode).MassFlowRate / (fanCoil.FanAirVolFlow * state.dataEnvrn->StdRhoAir);

            // Constant fan and variable flow calculation AND variable fan

            state.dataFans->fans(fanCoil.FanIndex)->simulate(state, FirstHVACIteration, FanSpeedRatio, _, FanSpeedRatio);

            if (fanCoil.CCoilType_Num == CCoil::HXAssist) {
                HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(
                    state, fanCoil.CCoilName, FirstHVACIteration, HVAC::CompressorOp::On, 0.0, fanCoil.CCoilName_Index, HVAC::FanOp::Continuous);
            } else {
                WaterCoils::SimulateWaterCoilComponents(state, fanCoil.CCoilName, FirstHVACIteration, fanCoil.CCoilName_Index);
            }
            if (fanCoil.HCoilType_Num == HCoil::Water) {
                WaterCoils::SimulateWaterCoilComponents(state, fanCoil.HCoilName, FirstHVACIteration, fanCoil.HCoilName_Index);
            } else {
                if (state.dataLoopNodes->Node(fanCoil.CoolCoilFluidInletNode).MassFlowRate > 0.0) {
                    ElecHeaterControl = 0.0;
                }
                HeatingCoils::SimulateHeatingCoilComponents(state,
                                                            fanCoil.HCoilName,
                                                            FirstHVACIteration,
                                                            fanCoil.DesignHeatingCapacity * PartLoad * ElecHeaterControl,
                                                            fanCoil.HCoilName_Index,
                                                            _,
                                                            false,
                                                            HVAC::FanOp::Continuous,
                                                            PartLoad);
            }
        }

        if (fanCoil.ATMixerExists) {
            if (fanCoil.ATMixerType == HVAC::MixerType::SupplySide) {
                // Now calculate the ATM mixer if it is on the supply side of the zone unit
                SingleDuct::SimATMixer(state, fanCoil.ATMixerName, FirstHVACIteration, fanCoil.ATMixerIndex);
                LoadMet = calcZoneSensibleOutput(state.dataLoopNodes->Node(state.dataFanCoilUnits->ATMixOutNode).MassFlowRate,
                                                 state.dataLoopNodes->Node(state.dataFanCoilUnits->ATMixOutNode).Temp,
                                                 state.dataLoopNodes->Node(state.dataFanCoilUnits->ZoneNode).Temp,
                                                 state.dataLoopNodes->Node(state.dataFanCoilUnits->ZoneNode).HumRat);
            } else {
                // ATM Mixer on inlet side
                LoadMet = calcZoneSensibleOutput(AirMassFlow,
                                                 state.dataLoopNodes->Node(OutletNode).Temp,
                                                 state.dataLoopNodes->Node(state.dataFanCoilUnits->ZoneNode).Temp,
                                                 state.dataLoopNodes->Node(state.dataFanCoilUnits->ZoneNode).HumRat);
            }
        } else {
            LoadMet = calcZoneSensibleOutput(AirMassFlow,
                                             state.dataLoopNodes->Node(OutletNode).Temp,
                                             state.dataLoopNodes->Node(InletNode).Temp,
                                             state.dataLoopNodes->Node(InletNode).HumRat);
        }
    }

    void SimMultiStage4PipeFanCoil(EnergyPlusData &state,
                                   int &FanCoilNum,               // number of the current fan coil unit being simulated
                                   int const ZoneNum,             // number of zone being served
                                   bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                                   Real64 &PowerMet               // Sensible power supplied (W)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Bereket Nigusse
        //       DATE WRITTEN   July 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Manages multi-speed fancoil unit simulation;

        // METHODOLOGY EMPLOYED:
        // Selects the appropriate fan speed for a given zone heating or cooling load
        // and determines whether heating or cooling is required, then runs the hot
        // or chilled water coils.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 mdot; // chilled or hot water flow rate through the water coils

        auto &fanCoil = state.dataFanCoilUnits->FanCoil(FanCoilNum);
        auto &HeatingLoad = state.dataFanCoilUnits->HeatingLoad;
        auto &CoolingLoad = state.dataFanCoilUnits->CoolingLoad;

        // initialize local variables
        bool UnitOn = true;         // TRUE if unit is on
        Real64 SpeedRatio = 0.0;    // ratio between lower and higher fan speed
        Real64 PartLoadRatio = 0.0; // Part Load Ratio, fraction of time step fancoil is on
        Real64 QZnReq = 0.0;        // heating or cooling needed by zone [watts]
        Real64 QUnitOut = 0.0;      // heating or sens. cooling provided by fan coil unit [watts]
        Real64 QUnitOutMax = 0.0;   // heating or sens. cooling provided by fan coil unit (running during an entire timestep)
        Real64 QUnitOutNoHC = 0.0;  // unit output with no active heating or cooling [W]

        int OutletNode = fanCoil.AirOutNode;
        int InletNode = fanCoil.AirInNode;
        Real64 AirMassFlow = state.dataLoopNodes->Node(InletNode).MassFlowRate;

        if (state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum) || AirMassFlow < HVAC::SmallMassFlow) {
            UnitOn = false;
        }

        fanCoil.SpeedFanSel = 1;
        fanCoil.SpeedFanRatSel = fanCoil.LowSpeedRatio;
        state.dataFanCoilUnits->FanFlowRatio = fanCoil.SpeedFanRatSel;
        AirMassFlow = fanCoil.LowSpeedRatio * fanCoil.MaxAirMassFlow;
        state.dataLoopNodes->Node(InletNode).MassFlowRate = AirMassFlow;
        state.dataLoopNodes->Node(InletNode).MassFlowRateMax = AirMassFlow;
        state.dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = AirMassFlow;
        state.dataLoopNodes->Node(InletNode).MassFlowRateMinAvail = AirMassFlow;

        if (fanCoil.HCoilType_Num == HCoil::Water) {
            mdot = 0.0;
            PlantUtilities::SetComponentFlowRate(
                state, mdot, fanCoil.HeatCoilFluidInletNode, fanCoil.HeatCoilFluidOutletNodeNum, fanCoil.HeatCoilPlantLoc);
        }
        mdot = 0.0;
        PlantUtilities::SetComponentFlowRate(
            state, mdot, fanCoil.CoolCoilFluidInletNode, fanCoil.CoolCoilFluidOutletNodeNum, fanCoil.CoolCoilPlantLoc);
        // no load output, requires setting eHeatCoilCyclingR = 0.0, for electric heating coils
        Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOutNoHC, _, 0.0);

        Real64 QCoilCoolSP = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToCoolSP;
        Real64 QCoilHeatSP = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputReqToHeatSP;
        state.dataFanCoilUnits->HeatingLoad = false;
        state.dataFanCoilUnits->CoolingLoad = false;

        if (QCoilHeatSP > 0.0 && QCoilCoolSP > 0.0 && state.dataHeatBalFanSys->TempControlType(ZoneNum) != HVAC::SetptType::SingleCool) {
            QZnReq = QCoilHeatSP;
            HeatingLoad = true;
        } else if (QCoilHeatSP > 0.0 && QCoilCoolSP > 0.0 && state.dataHeatBalFanSys->TempControlType(ZoneNum) == HVAC::SetptType::SingleCool) {
            QZnReq = 0.0;
        } else if (QCoilHeatSP < 0.0 && QCoilCoolSP < 0.0 && state.dataHeatBalFanSys->TempControlType(ZoneNum) != HVAC::SetptType::SingleHeat) {
            QZnReq = QCoilCoolSP;
            CoolingLoad = true;
        } else if (QCoilHeatSP < 0.0 && QCoilCoolSP < 0.0 && state.dataHeatBalFanSys->TempControlType(ZoneNum) == HVAC::SetptType::SingleHeat) {
            QZnReq = 0.0;
        } else if (QCoilHeatSP <= 0.0 && QCoilCoolSP >= 0.0) {
            QZnReq = 0.0;
        }

        // Zone load calculation for constant fan systems, adopted from unitary system
        if (fanCoil.fanOp == HVAC::FanOp::Continuous) {
            switch (state.dataHeatBalFanSys->TempControlType(ZoneNum)) {
            case HVAC::SetptType::SingleHeat: {
                CoolingLoad = false;
                // No heating load and constant fan pushes zone below heating set point
                if (QUnitOutNoHC < 0.0 && QCoilHeatSP < 0.0 && QUnitOutNoHC - QCoilHeatSP < -HVAC::SmallLoad) {
                    HeatingLoad = true;
                    CoolingLoad = false;
                    QZnReq = QCoilHeatSP;
                }
            } break;
            case HVAC::SetptType::SingleCool: {
                HeatingLoad = false;
                // No heating load and constant fan pushes zone above cooling set point
                if (QUnitOutNoHC > 0.0 && QCoilCoolSP > 0.0 && QUnitOutNoHC - QCoilCoolSP > HVAC::SmallLoad) {
                    HeatingLoad = false;
                    CoolingLoad = true;
                    QZnReq = QCoilCoolSP;
                }
            } break;
            case HVAC::SetptType::SingleHeatCool: {
                // zone temp above cooling and heating set point temps
                if (QCoilHeatSP < 0.0 && QCoilCoolSP < 0.0) {
                    // zone pushed below heating set point
                    if (QUnitOutNoHC < 0.0 && QCoilHeatSP - QUnitOutNoHC > HVAC::SmallLoad) {
                        HeatingLoad = true;
                        CoolingLoad = false;
                        QZnReq = QCoilHeatSP;
                    }
                    // zone temp below heating set point temp
                } else if (QCoilHeatSP > 0.0 && QCoilCoolSP > 0.0) {
                    // zone pushed above cooling set point
                    if (QUnitOutNoHC > 0.0 && QCoilCoolSP - QUnitOutNoHC > HVAC::SmallLoad) {
                        HeatingLoad = false;
                        CoolingLoad = true;
                        QZnReq = QCoilCoolSP;
                    }
                }
            } break;
            case HVAC::SetptType::DualHeatCool: {
                // zone temp above cooling and heating set point temps
                if (QCoilHeatSP < 0.0 && QCoilCoolSP < 0.0) {
                    // zone pushed into deadband
                    if (QUnitOutNoHC < 0.0 && QCoilCoolSP - QUnitOutNoHC > HVAC::SmallLoad) {
                        HeatingLoad = false;
                        CoolingLoad = false;
                        QZnReq = 0.0;
                    }
                    // zone pushed below heating set point
                    if (QUnitOutNoHC < 0.0 && QCoilHeatSP - QUnitOutNoHC > HVAC::SmallLoad) {
                        HeatingLoad = true;
                        CoolingLoad = false;
                        QZnReq = QCoilHeatSP;
                    }
                    // zone temp below heating set point temp
                } else if (QCoilHeatSP > 0.0 && QCoilCoolSP > 0.0) {
                    // zone pushed into deadband
                    if (QUnitOutNoHC > 0.0 && QUnitOutNoHC - QCoilHeatSP > HVAC::SmallLoad) {
                        HeatingLoad = false;
                        CoolingLoad = false;
                        QZnReq = 0.0;
                    }
                    // zone pushed above cooling set point
                    if (QUnitOutNoHC > 0.0 && QUnitOutNoHC - QCoilCoolSP > HVAC::SmallLoad) {
                        HeatingLoad = false;
                        CoolingLoad = true;
                        QZnReq = QCoilCoolSP;
                    }
                    // zone temp between set point temps
                } else if (QCoilHeatSP < 0.0 && QCoilCoolSP > 0.0) {
                    // zone pushed below heating set point
                    if (QUnitOutNoHC < 0.0 && QUnitOutNoHC - QCoilHeatSP < -HVAC::SmallLoad) {
                        HeatingLoad = true;
                        CoolingLoad = false;
                        QZnReq = QCoilHeatSP;
                        // zone pushed above cooling set point
                    } else if (QUnitOutNoHC > 0.0 && QUnitOutNoHC - QCoilCoolSP > HVAC::SmallLoad) {
                        HeatingLoad = false;
                        CoolingLoad = true;
                        QZnReq = QCoilCoolSP;
                    }
                }
            } break;
            default:
                break;
            }
            // IF small loads to meet, just shut down unit
            if (std::abs(QZnReq) < FanCoilUnits::Small5WLoad) {
                QZnReq = 0.0;
                CoolingLoad = false;
                HeatingLoad = false;
            }
        }

        if (UnitOn && QZnReq < (-1.0 * FanCoilUnits::Small5WLoad) && CoolingLoad) {
            if (fanCoil.HCoilType_Num == HCoil::Water) {
                mdot = 0.0;
                PlantUtilities::SetComponentFlowRate(
                    state, mdot, fanCoil.HeatCoilFluidInletNode, fanCoil.HeatCoilFluidOutletNodeNum, fanCoil.HeatCoilPlantLoc);
            }
            mdot = fanCoil.MaxCoolCoilFluidFlow;
            PlantUtilities::SetComponentFlowRate(
                state, mdot, fanCoil.CoolCoilFluidInletNode, fanCoil.CoolCoilFluidOutletNodeNum, fanCoil.CoolCoilPlantLoc);
            // select fan speed
            fanCoil.SpeedFanSel = 1;
            fanCoil.SpeedFanRatSel = fanCoil.LowSpeedRatio;
            state.dataFanCoilUnits->FanFlowRatio = fanCoil.SpeedFanRatSel;
            AirMassFlow = fanCoil.LowSpeedRatio * fanCoil.MaxAirMassFlow;
            state.dataLoopNodes->Node(InletNode).MassFlowRate = AirMassFlow;
            state.dataLoopNodes->Node(InletNode).MassFlowRateMax = AirMassFlow;
            state.dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = AirMassFlow;
            state.dataLoopNodes->Node(InletNode).MassFlowRateMinAvail = AirMassFlow;
            Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOutMax);
            if (std::abs(QUnitOutMax) < std::abs(QZnReq)) {
                fanCoil.SpeedFanSel = 2;
                fanCoil.SpeedFanRatSel = fanCoil.MedSpeedRatio;
                state.dataFanCoilUnits->FanFlowRatio = fanCoil.SpeedFanRatSel;
                AirMassFlow = fanCoil.MedSpeedRatio * fanCoil.MaxAirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRate = AirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRateMax = AirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = AirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRateMinAvail = fanCoil.LowSpeedRatio * fanCoil.MaxAirMassFlow;
                Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOutMax);
            }
            if (std::abs(QUnitOutMax) < std::abs(QZnReq)) {
                fanCoil.SpeedFanSel = 3;
                fanCoil.SpeedFanRatSel = 1.0;
                state.dataFanCoilUnits->FanFlowRatio = fanCoil.SpeedFanRatSel;
                AirMassFlow = fanCoil.MaxAirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRate = AirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRateMax = AirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = AirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRateMinAvail = fanCoil.MedSpeedRatio * fanCoil.MaxAirMassFlow;
            }
            CalcMultiStage4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QZnReq, SpeedRatio, PartLoadRatio, QUnitOut);

        } else if (UnitOn && QZnReq > FanCoilUnits::Small5WLoad && HeatingLoad) {

            mdot = 0.0;
            PlantUtilities::SetComponentFlowRate(
                state, mdot, fanCoil.CoolCoilFluidInletNode, fanCoil.CoolCoilFluidOutletNodeNum, fanCoil.CoolCoilPlantLoc);

            if (fanCoil.HCoilType_Num == HCoil::Water) {
                mdot = fanCoil.MaxHeatCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(
                    state, mdot, fanCoil.HeatCoilFluidInletNode, fanCoil.HeatCoilFluidOutletNodeNum, fanCoil.HeatCoilPlantLoc);
            }
            // select fan speed
            fanCoil.SpeedFanSel = 1;
            fanCoil.SpeedFanRatSel = fanCoil.LowSpeedRatio;
            state.dataFanCoilUnits->FanFlowRatio = fanCoil.SpeedFanRatSel;
            AirMassFlow = fanCoil.LowSpeedRatio * fanCoil.MaxAirMassFlow;
            state.dataLoopNodes->Node(InletNode).MassFlowRate = AirMassFlow;
            state.dataLoopNodes->Node(InletNode).MassFlowRateMax = AirMassFlow;
            state.dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = AirMassFlow;
            state.dataLoopNodes->Node(InletNode).MassFlowRateMinAvail = AirMassFlow;
            Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOutMax);
            if (std::abs(QUnitOutMax) < std::abs(QZnReq)) {
                fanCoil.SpeedFanSel = 2;
                fanCoil.SpeedFanRatSel = fanCoil.MedSpeedRatio;
                state.dataFanCoilUnits->FanFlowRatio = fanCoil.SpeedFanRatSel;
                AirMassFlow = fanCoil.MedSpeedRatio * fanCoil.MaxAirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRate = AirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRateMax = AirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = AirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRateMinAvail = fanCoil.LowSpeedRatio * fanCoil.MaxAirMassFlow;
                Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOutMax);
            }
            if (std::abs(QUnitOutMax) < std::abs(QZnReq)) {
                fanCoil.SpeedFanSel = 3;
                fanCoil.SpeedFanRatSel = 1.0;
                state.dataFanCoilUnits->FanFlowRatio = fanCoil.SpeedFanRatSel;
                AirMassFlow = fanCoil.MaxAirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRate = AirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRateMax = AirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = AirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRateMinAvail = fanCoil.MedSpeedRatio * fanCoil.MaxAirMassFlow;
            }

            CalcMultiStage4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QZnReq, SpeedRatio, PartLoadRatio, QUnitOut);

        } else {
            // SpeedRatio = 0.0;
            if (fanCoil.fanOp == HVAC::FanOp::Continuous) {
                PartLoadRatio = 1.0;
                fanCoil.SpeedFanSel = 1;
                fanCoil.SpeedFanRatSel = fanCoil.LowSpeedRatio;
                state.dataFanCoilUnits->FanFlowRatio = fanCoil.SpeedFanRatSel;
                AirMassFlow = fanCoil.LowSpeedRatio * fanCoil.MaxAirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRate = AirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRateMax = AirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = AirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRateMinAvail = AirMassFlow;
            } else {
                PartLoadRatio = 0.0;
                AirMassFlow = 0.0;
                state.dataLoopNodes->Node(InletNode).MassFlowRate = AirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRateMax = AirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = AirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRateMinAvail = AirMassFlow;
                state.dataLoopNodes->Node(InletNode).MassFlowRate = 0.0;
                state.dataLoopNodes->Node(OutletNode).MassFlowRate = 0.0;
                fanCoil.SpeedFanSel = 0;
                state.dataFanCoilUnits->FanFlowRatio = 0.0;
            }

            mdot = 0.0;
            if (fanCoil.HCoilType_Num == HCoil::Water) {
                PlantUtilities::SetComponentFlowRate(
                    state, mdot, fanCoil.HeatCoilFluidInletNode, fanCoil.HeatCoilFluidOutletNodeNum, fanCoil.HeatCoilPlantLoc);
            }
            PlantUtilities::SetComponentFlowRate(
                state, mdot, fanCoil.CoolCoilFluidInletNode, fanCoil.CoolCoilFluidOutletNodeNum, fanCoil.CoolCoilPlantLoc);
            // No load output, eHeatCoilCyclingR = 0.0 for electric heating coil
            Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, PartLoadRatio, 0.0);
        }
        // output variable
        state.dataLoopNodes->Node(OutletNode).MassFlowRate = state.dataLoopNodes->Node(InletNode).MassFlowRate;
        fanCoil.PLR = PartLoadRatio;
        fanCoil.SpeedRatio = SpeedRatio;
        PowerMet = QUnitOut;
    }

    void CalcMultiStage4PipeFanCoil(EnergyPlusData &state,
                                    int &FanCoilNum,               // number of the current fan coil unit being simulated
                                    int const ZoneNum,             // number of zone being served
                                    bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                                    Real64 const QZnReq,           // current zone cooling or heating load
                                    Real64 &SpeedRatio,            // fan coil speed ratio
                                    Real64 &PartLoadRatio,         // fan coil part load ratio
                                    Real64 &PowerMet               // Sensible power supplied (W)
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Bereket Nigusse
        //       DATE WRITTEN   July 2015

        // PURPOSE OF THIS SUBROUTINE:
        // Simulate a multi-stage fan 4 pipe fan coil unit; adjust its output to
        // match the remaining zone load.

        // METHODOLOGY EMPLOYED:
        // If this unit is on, calculated the speed ratio when cycling between
        // consecutive fan speeds. The hot or chilled water flows either at
        // maximum or zero.  The water flow rate is set to zero if there is no
        // load.

        // SUBROUTINE PARAMETER DEFINITIONS:
        constexpr int MaxIterCycl(100);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 QUnitOutMaxHS;  // higher fan speed output
        Real64 QUnitOutMaxLS;  // lower fan speed output
        Real64 HighSpeedRatio; // fan flow ratio at low speed
        Real64 LowSpeedRatio;  // fan flow ratio at low speed
        Real64 DelPLR;
        int SolFlag; // return flag from RegulaFalsi for sensible load

        auto &fanCoil = state.dataFanCoilUnits->FanCoil(FanCoilNum);

        // initialize local variables
        Real64 mdot = 0.0;                                 // chilled or hot water flow rate through the water coils
        Real64 PLR = 1.0;                                  // Part Load Ratio, fraction of time step fancoil is on
        Real64 SRatio = 0.0;                               // capacity speed ratio of the for multi-stage fan fancoil unit
        Real64 QUnitOut = 0.0;                             // heating or sens. cooling provided by fan coil unit [watts]
        Real64 QUnitOutMax = 0.0;                          // max heating or sens. cooling provided by fan coil unit [watts]
        Real64 ControlOffset = 0.0;                        // tolerance for output control
        Real64 FanElecPowerHS = 0.0;                       // fan electric power calculated at (fan) higher speed
        Real64 FanElecPowerLS = 0.0;                       // fan electric power calculated at (fan) lower speed
        Real64 AirMassFlowAvg = 0.0;                       // supply air flow rate weighted by speed ratio
        Real64 AirMassFlowLow = 0.0;                       // supply air flow rate at lower speed
        Real64 AirMassFlowHigh = 0.0;                      // supply air flow rate at higher speed
        Real64 AbsError = 2.0 * FanCoilUnits::Small5WLoad; // Absolute error between QZnReq and QUnitOut [W]   !FB
        Real64 Error = 1.0;                                // Error between QZnReq and QUnitOut
        Real64 Relax = 1.0;
        int Iter = 0; // iteration counter

        auto &inletNode = state.dataLoopNodes->Node(fanCoil.AirInNode);

        if (QZnReq < (-1.0 * FanCoilUnits::Small5WLoad) && state.dataFanCoilUnits->CoolingLoad) {
            ControlOffset = fanCoil.ColdControlOffset;
            if (fanCoil.SpeedFanSel == 1) {
                Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOutMax);
                PLR = std::abs(QZnReq / QUnitOutMax);
                if (PLR > 1.0) {
                    PLR = 1.0;
                }
                // adjust the PLR to meet the cooling load by calling Calc4PipeFanCoil repeatedly
                while (std::abs(Error) > ControlOffset && std::abs(AbsError) > FanCoilUnits::Small5WLoad && Iter < MaxIterCycl && PLR != 1.0) {
                    inletNode.MassFlowRateMinAvail = inletNode.MassFlowRate;
                    mdot = PLR * fanCoil.MaxCoolCoilFluidFlow;
                    PlantUtilities::SetComponentFlowRate(
                        state, mdot, fanCoil.CoolCoilFluidInletNode, fanCoil.CoolCoilFluidOutletNodeNum, fanCoil.CoolCoilPlantLoc);
                    if (fanCoil.fanOp == HVAC::FanOp::Continuous) {
                        Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut);
                    } else {
                        Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, PLR);
                    }
                    Error = (QZnReq - QUnitOut) / QZnReq;
                    AbsError = QZnReq - QUnitOut;
                    DelPLR = (QZnReq - QUnitOut) / QUnitOutMax;
                    PLR += Relax * DelPLR;
                    PLR = max(0.0, min(1.0, PLR));
                    ++Iter;
                    if (Iter == 32) {
                        Relax = 0.5;
                    }
                    if (Iter == 65) {
                        Relax = 0.25;
                    }
                    if (Iter > 70 && PLR == 0.0 && DelPLR < 0.0) {
                        Error = 0.0;
                    }
                }
                if (fanCoil.fanOp == HVAC::FanOp::Continuous) {
                    Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut);
                } else {
                    Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, PLR);
                }
                // warning if not converged
                if (Iter > (MaxIterCycl - 1)) {
                    if (fanCoil.MaxIterIndexC == 0) {
                        ShowWarningMessage(state,
                                           format("ZoneHVAC:FourPipeFanCoil=\"{}\" -- Exceeded max iterations while adjusting cycling fan sensible "
                                                  "runtime to meet the zone load within the cooling convergence tolerance.",
                                                  fanCoil.Name));
                        ShowContinueErrorTimeStamp(state, format("Iterations={}", MaxIterCycl));
                    }
                    ShowRecurringWarningErrorAtEnd(state,
                                                   "ZoneHVAC:FourPipeFanCoil=\"" + fanCoil.Name +
                                                       "\"  -- Exceeded max iterations error (sensible runtime) continues...",
                                                   fanCoil.MaxIterIndexC);
                }

            } else {
                if (fanCoil.SpeedFanSel == 2) {
                    HighSpeedRatio = fanCoil.MedSpeedRatio;
                    LowSpeedRatio = fanCoil.LowSpeedRatio;
                } else {
                    HighSpeedRatio = 1;
                    LowSpeedRatio = fanCoil.MedSpeedRatio;
                }
                // get capacity at lower speed
                fanCoil.SpeedFanRatSel = LowSpeedRatio;
                fanCoil.SpeedFanSel = fanCoil.SpeedFanSel - 1;
                AirMassFlowLow = LowSpeedRatio * fanCoil.MaxAirMassFlow;
                inletNode.MassFlowRate = AirMassFlowLow;
                inletNode.MassFlowRateMax = AirMassFlowLow;
                inletNode.MassFlowRateMaxAvail = AirMassFlowLow;
                inletNode.MassFlowRateMinAvail = AirMassFlowLow;
                state.dataFanCoilUnits->FanFlowRatio = LowSpeedRatio;
                Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOutMaxLS);
                FanElecPowerLS = state.dataFans->fans(fanCoil.FanIndex)->totalPower;
                // get capacity at higher speed
                fanCoil.SpeedFanRatSel = HighSpeedRatio;
                fanCoil.SpeedFanSel = fanCoil.SpeedFanSel + 1;
                AirMassFlowHigh = HighSpeedRatio * fanCoil.MaxAirMassFlow;
                inletNode.MassFlowRate = AirMassFlowHigh;
                inletNode.MassFlowRateMax = AirMassFlowHigh;
                inletNode.MassFlowRateMaxAvail = AirMassFlowHigh;
                inletNode.MassFlowRateMinAvail = AirMassFlowLow;
                state.dataFanCoilUnits->FanFlowRatio = HighSpeedRatio;
                Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOutMaxHS);
                FanElecPowerHS = state.dataFans->fans(fanCoil.FanIndex)->totalPower;
                // calc speed ratio
                if (std::abs(QZnReq) > std::abs(QUnitOutMaxHS)) {
                    SRatio = 1.0;
                    AirMassFlowAvg = AirMassFlowHigh;
                    inletNode.MassFlowRate = AirMassFlowHigh;
                    inletNode.MassFlowRateMax = AirMassFlowHigh;
                    inletNode.MassFlowRateMaxAvail = AirMassFlowHigh;
                    inletNode.MassFlowRateMinAvail = AirMassFlowLow;
                    state.dataFanCoilUnits->FanFlowRatio = HighSpeedRatio;
                    Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut);
                } else {
                    SRatio = std::abs((QZnReq - QUnitOutMaxLS) / (QUnitOutMaxHS - QUnitOutMaxLS));
                    if (SRatio > 1.0) {
                        SRatio = 1.0;
                    }
                    AirMassFlowAvg = AirMassFlowHigh * SRatio + AirMassFlowLow * (1.0 - SRatio);
                    inletNode.MassFlowRate = AirMassFlowAvg;
                    inletNode.MassFlowRateMax = AirMassFlowAvg;
                    inletNode.MassFlowRateMaxAvail = AirMassFlowAvg;
                    inletNode.MassFlowRateMinAvail = AirMassFlowLow;
                    state.dataFanCoilUnits->FanFlowRatio = HighSpeedRatio * SRatio + LowSpeedRatio * (1.0 - SRatio);
                    Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut);
                    // adjust the PLR to meet the cooling load by calling Calc4PipeFanCoil repeatedly
                    while (std::abs(Error) > ControlOffset && std::abs(AbsError) > FanCoilUnits::Small5WLoad && Iter < MaxIterCycl && SRatio != 1.0) {
                        AirMassFlowAvg = AirMassFlowHigh * SRatio + AirMassFlowLow * (1.0 - SRatio);
                        state.dataFanCoilUnits->FanFlowRatio = HighSpeedRatio * SRatio + LowSpeedRatio * (1.0 - SRatio);
                        inletNode.MassFlowRate = AirMassFlowAvg;
                        inletNode.MassFlowRateMax = AirMassFlowAvg;
                        inletNode.MassFlowRateMaxAvail = AirMassFlowAvg;
                        inletNode.MassFlowRateMinAvail = AirMassFlowLow;
                        Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut);
                        Error = (QZnReq - QUnitOut) / QZnReq;
                        AbsError = QZnReq - QUnitOut;
                        DelPLR = (QZnReq - QUnitOut) / (QUnitOutMaxHS - QUnitOutMaxLS);
                        SRatio += Relax * DelPLR;
                        SRatio = max(0.0, min(1.0, SRatio));
                        ++Iter;
                        if (Iter == 32) {
                            Relax = 0.5;
                        }
                        if (Iter == 65) {
                            Relax = 0.25;
                        }
                        if (Iter > 70 && SRatio == 0.0 && DelPLR < 0.0) {
                            Error = 0.0;
                        }
                    }
                }
            }
        } else if (QZnReq > FanCoilUnits::Small5WLoad && state.dataFanCoilUnits->HeatingLoad) {
            ControlOffset = fanCoil.HotControlOffset;
            if (fanCoil.SpeedFanSel == 1) {
                Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOutMax);
                PLR = std::abs(QZnReq / QUnitOutMax);
                if (PLR > 1.0) {
                    PLR = 1.0;
                }
                if (fanCoil.HCoilType_Num == HCoil::Water) {
                    // adjust the PLR to meet the heating load by calling Calc4PipeFanCoil repeatedly
                    while (std::abs(Error) > ControlOffset && std::abs(AbsError) > FanCoilUnits::Small5WLoad && Iter < MaxIterCycl && PLR != 1.0) {
                        inletNode.MassFlowRateMinAvail = inletNode.MassFlowRate;
                        mdot = PLR * fanCoil.MaxHeatCoilFluidFlow;
                        PlantUtilities::SetComponentFlowRate(
                            state, mdot, fanCoil.HeatCoilFluidInletNode, fanCoil.HeatCoilFluidOutletNodeNum, fanCoil.HeatCoilPlantLoc);
                        if (fanCoil.fanOp == HVAC::FanOp::Continuous) {
                            Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut);
                        } else {
                            Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, PLR);
                        }
                        Error = (QZnReq - QUnitOut) / QZnReq;
                        AbsError = QZnReq - QUnitOut;
                        DelPLR = (QZnReq - QUnitOut) / QUnitOutMax;
                        PLR += Relax * DelPLR;
                        PLR = max(0.0, min(1.0, PLR));
                        ++Iter;
                        if (Iter == 32) {
                            Relax = 0.5;
                        }
                        if (Iter == 65) {
                            Relax = 0.25;
                        }
                        if (Iter > 70 && PLR == 0.0 && DelPLR < 0.0) {
                            Error = 0.0; // exit loop if PLR = 0
                        }
                    }
                    if (fanCoil.fanOp == HVAC::FanOp::Continuous) {
                        Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut);
                    } else {
                        Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, PLR);
                    }
                    // warning if not converged
                    if (Iter > (MaxIterCycl - 1)) {
                        if (fanCoil.MaxIterIndexH == 0) {
                            ShowWarningMessage(state,
                                               format("ZoneHVAC:FourPipeFanCoil=\"{}\" -- Exceeded max iterations while adjusting cycling fan "
                                                      "sensible runtime to meet the zone load within the heating convergence tolerance.",
                                                      fanCoil.Name));
                            ShowContinueErrorTimeStamp(state, format("Iterations={}", MaxIterCycl));
                        }
                        ShowRecurringWarningErrorAtEnd(state,
                                                       "ZoneHVAC:FourPipeFanCoil=\"" + fanCoil.Name +
                                                           "\"  -- Exceeded max iterations error (sensible runtime) continues...",
                                                       fanCoil.MaxIterIndexH);
                    }
                } else {
                    Real64 eHeatCoilPLR = PLR;
                    // electric heating coil
                    if (QUnitOutMax > QZnReq) {
                        // heating coil output is larger than required, mudulate the electric heating coil output to meet the load
                        inletNode.MassFlowRateMinAvail = inletNode.MassFlowRate;

                        if (fanCoil.fanOp == HVAC::FanOp::Continuous) {
                            auto f = [&state, FanCoilNum, FirstHVACIteration, ZoneNum, QZnReq](Real64 const CyclingR) {
                                return CalcFanCoilHeatCoilPLRResidual(state, CyclingR, FanCoilNum, FirstHVACIteration, ZoneNum, QZnReq);
                            };
                            General::SolveRoot(state, 0.001, MaxIterCycl, SolFlag, eHeatCoilPLR, f, 0.0, 1.0);
                        } else {
                            auto f = [&state, FirstHVACIteration, FanCoilNum, ZoneNum, QZnReq](Real64 const PartLoadRatio) {
                                return CalcFanCoilLoadResidual(state, FanCoilNum, FirstHVACIteration, ZoneNum, QZnReq, PartLoadRatio);
                            };
                            General::SolveRoot(state, 0.001, MaxIterCycl, SolFlag, eHeatCoilPLR, f, 0.0, 1.0);
                        }
                        if (SolFlag == -1) {
                            ++fanCoil.ConvgErrCountH;
                            if (fanCoil.ConvgErrCountH < 2) {
                                ShowWarningError(state, format("Electric heating coil control failed in fan coil unit {}", fanCoil.Name));
                                ShowContinueError(state, "  Iteration limit exceeded in calculating electric heating coil capacity modulation ");
                                Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, _, eHeatCoilPLR);
                                ShowContinueErrorTimeStamp(state, format("Load Request = {}, Final Capacity = {}", QZnReq, QUnitOut));
                                ShowContinueErrorTimeStamp(
                                    state, format("Electric heating coil part load ratio used during last iterations = {}", eHeatCoilPLR));
                            } else {
                                ShowRecurringWarningErrorAtEnd(
                                    state, "Electric heating coil Iteration limit exceeded in fan coil unit " + fanCoil.Name, fanCoil.MaxIterIndexH);
                            }
                        } else if (SolFlag == -2) {
                            ++fanCoil.LimitErrCountH;
                            if (fanCoil.LimitErrCountH < 2) {
                                ShowWarningError(state,
                                                 format("Part load ratio electric heating coil control failed in fan coil unit {}", fanCoil.Name));
                                ShowContinueError(state, "  Bad par load ratio limits");
                                ShowContinueErrorTimeStamp(state, "..Par load ratio set to 0");
                            } else {
                                ShowRecurringWarningErrorAtEnd(state,
                                                               "Part load ratio electric heating coil control failed in fan coil unit " +
                                                                   fanCoil.Name,
                                                               fanCoil.BadMassFlowLimIndexH);
                            }
                        }
                    } else {
                        eHeatCoilPLR = 1.0;
                    }
                    PLR = eHeatCoilPLR;
                    // at the end calculate output
                    if (fanCoil.fanOp == HVAC::FanOp::Continuous) {
                        Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, _, eHeatCoilPLR);
                    } else {
                        Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, PLR);
                    }
                }

            } else {
                if (fanCoil.SpeedFanSel == 2) {
                    HighSpeedRatio = fanCoil.MedSpeedRatio;
                    LowSpeedRatio = fanCoil.LowSpeedRatio;
                } else {
                    HighSpeedRatio = 1;
                    LowSpeedRatio = fanCoil.MedSpeedRatio;
                }
                // get capacity at lower speed ratio
                fanCoil.SpeedFanRatSel = LowSpeedRatio;
                fanCoil.SpeedFanSel = fanCoil.SpeedFanSel - 1;
                AirMassFlowLow = LowSpeedRatio * fanCoil.MaxAirMassFlow;
                inletNode.MassFlowRate = AirMassFlowLow;
                inletNode.MassFlowRateMax = AirMassFlowLow;
                inletNode.MassFlowRateMaxAvail = AirMassFlowLow;
                inletNode.MassFlowRateMinAvail = AirMassFlowLow;
                state.dataFanCoilUnits->FanFlowRatio = LowSpeedRatio;
                Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOutMaxLS);
                FanElecPowerLS = state.dataFans->fans(fanCoil.FanIndex)->totalPower;
                // get capacity at higher speed
                fanCoil.SpeedFanRatSel = HighSpeedRatio;
                fanCoil.SpeedFanSel = fanCoil.SpeedFanSel + 1;
                AirMassFlowHigh = HighSpeedRatio * fanCoil.MaxAirMassFlow;
                inletNode.MassFlowRate = AirMassFlowHigh;
                inletNode.MassFlowRateMax = AirMassFlowHigh;
                inletNode.MassFlowRateMaxAvail = AirMassFlowHigh;
                inletNode.MassFlowRateMinAvail = AirMassFlowLow;
                state.dataFanCoilUnits->FanFlowRatio = HighSpeedRatio;
                Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOutMaxHS);
                FanElecPowerHS = state.dataFans->fans(fanCoil.FanIndex)->totalPower;
                // calc speed ratio
                if (std::abs(QZnReq) > std::abs(QUnitOutMaxHS)) {
                    SRatio = 1.0;
                    AirMassFlowAvg = AirMassFlowHigh;
                    inletNode.MassFlowRate = AirMassFlowAvg;
                    inletNode.MassFlowRateMax = AirMassFlowAvg;
                    inletNode.MassFlowRateMaxAvail = AirMassFlowAvg;
                    inletNode.MassFlowRateMinAvail = AirMassFlowLow;
                    state.dataFanCoilUnits->FanFlowRatio = HighSpeedRatio;
                    Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut);
                } else {
                    SRatio = std::abs((QZnReq - QUnitOutMaxLS) / (QUnitOutMaxHS - QUnitOutMaxLS));
                    if (SRatio > 1.0) {
                        SRatio = 1.0;
                    }
                    AirMassFlowAvg = AirMassFlowHigh * SRatio + AirMassFlowLow * (1.0 - SRatio);
                    inletNode.MassFlowRate = AirMassFlowAvg;
                    inletNode.MassFlowRateMax = AirMassFlowAvg;
                    inletNode.MassFlowRateMaxAvail = AirMassFlowAvg;
                    inletNode.MassFlowRateMinAvail = AirMassFlowLow;
                    state.dataFanCoilUnits->FanFlowRatio = HighSpeedRatio * SRatio + LowSpeedRatio * (1.0 - SRatio);
                    Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut);
                    ControlOffset = fanCoil.HotControlOffset;
                    // adjust the PLR to meet the heating load calling Calc4PipeFanCoil repeatedly
                    while (std::abs(Error) > ControlOffset && std::abs(AbsError) > FanCoilUnits::Small5WLoad && Iter < MaxIterCycl && SRatio != 1.0) {
                        AirMassFlowAvg = AirMassFlowHigh * SRatio + AirMassFlowLow * (1.0 - SRatio);
                        inletNode.MassFlowRate = AirMassFlowAvg;
                        inletNode.MassFlowRateMax = AirMassFlowAvg;
                        inletNode.MassFlowRateMaxAvail = AirMassFlowAvg;
                        inletNode.MassFlowRateMinAvail = AirMassFlowLow;
                        state.dataFanCoilUnits->FanFlowRatio = HighSpeedRatio * SRatio + LowSpeedRatio * (1.0 - SRatio);
                        Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut);
                        Error = (QZnReq - QUnitOut) / QZnReq;
                        AbsError = QZnReq - QUnitOut;
                        DelPLR = (QZnReq - QUnitOut) / (QUnitOutMaxHS - QUnitOutMaxLS);
                        SRatio += Relax * DelPLR;
                        SRatio = max(0.0, min(1.0, SRatio));
                        ++Iter;
                        if (Iter == 32) {
                            Relax = 0.5;
                        }
                        if (Iter == 65) {
                            Relax = 0.25;
                        }
                        if (Iter > 70 && SRatio == 0.0 && DelPLR < 0.0) {
                            Error = 0.0;
                        }
                    }
                }
                // FanElecPower = FanElecPowerHS * SRatio + FanElecPowerLS * ( 1.0 - SRatio ); // why set the ugly global here?
                fanCoil.ElecPower = FanElecPowerHS * SRatio + FanElecPowerLS * (1.0 - SRatio);
            }
        }
        state.dataLoopNodes->Node(fanCoil.AirOutNode).MassFlowRate = inletNode.MassFlowRate;
        PartLoadRatio = PLR;
        SpeedRatio = SRatio;
        PowerMet = QUnitOut;
    }

    void ReportFanCoilUnit(EnergyPlusData &state, int const FanCoilNum) // number of the current fan coil unit being simulated
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Fred Buhl
        //       DATE WRITTEN   March 2000

        // PURPOSE OF THIS SUBROUTINE:
        // Fills some of the report variables for the fan coil units

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 ReportingConstant = state.dataHVACGlobal->TimeStepSysSec;

        state.dataFanCoilUnits->FanCoil(FanCoilNum).HeatEnergy = state.dataFanCoilUnits->FanCoil(FanCoilNum).HeatPower * ReportingConstant;
        state.dataFanCoilUnits->FanCoil(FanCoilNum).SensCoolEnergy = state.dataFanCoilUnits->FanCoil(FanCoilNum).SensCoolPower * ReportingConstant;
        state.dataFanCoilUnits->FanCoil(FanCoilNum).TotCoolEnergy = state.dataFanCoilUnits->FanCoil(FanCoilNum).TotCoolPower * ReportingConstant;
        state.dataFanCoilUnits->FanCoil(FanCoilNum).ElecEnergy = state.dataFanCoilUnits->FanCoil(FanCoilNum).ElecPower * ReportingConstant;

        if (state.dataFanCoilUnits->FanCoil(FanCoilNum).FirstPass) { // reset sizing flags so other zone equipment can size normally
            if (!state.dataGlobal->SysSizingCalc) {
                DataSizing::resetHVACSizingGlobals(state, state.dataSize->CurZoneEqNum, 0, state.dataFanCoilUnits->FanCoil(FanCoilNum).FirstPass);
            }
        }
    }

    int GetFanCoilZoneInletAirNode(EnergyPlusData &state, int const FanCoilNum)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   Dec  2006

        // PURPOSE OF THIS FUNCTION:
        // lookup function for OA inlet node for ventilation rate reporting

        if (state.dataFanCoilUnits->GetFanCoilInputFlag) {
            GetFanCoilUnits(state);
            state.dataFanCoilUnits->GetFanCoilInputFlag = false;
        }

        if (FanCoilNum > 0 && FanCoilNum <= state.dataFanCoilUnits->NumFanCoils) {
            return state.dataFanCoilUnits->FanCoil(FanCoilNum).AirOutNode;
        }

        return 0;
    }

    int GetFanCoilOutAirNode(EnergyPlusData &state, int const FanCoilNum)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   Dec  2006

        // PURPOSE OF THIS FUNCTION:
        // lookup function for OA inlet node for ventilation rate reporting

        if (state.dataFanCoilUnits->GetFanCoilInputFlag) {
            GetFanCoilUnits(state);
            state.dataFanCoilUnits->GetFanCoilInputFlag = false;
        }

        if (FanCoilNum > 0 && FanCoilNum <= state.dataFanCoilUnits->NumFanCoils) {
            return state.dataFanCoilUnits->FanCoil(FanCoilNum).OutsideAirNode;
        }

        return 0;
    }

    int GetFanCoilReturnAirNode(EnergyPlusData &state, int const FanCoilNum)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   Dec  2006

        // PURPOSE OF THIS FUNCTION:
        // lookup function for mixer's return node

        if (state.dataFanCoilUnits->GetFanCoilInputFlag) {
            GetFanCoilUnits(state);
            state.dataFanCoilUnits->GetFanCoilInputFlag = false;
        }

        if (FanCoilNum > 0 && FanCoilNum <= state.dataFanCoilUnits->NumFanCoils) {
            if (state.dataFanCoilUnits->FanCoil(FanCoilNum).OAMixIndex > 0) {
                return MixedAir::GetOAMixerReturnNodeNumber(state, state.dataFanCoilUnits->FanCoil(FanCoilNum).OAMixIndex);
            }
        }

        return 0;
    }

    int GetFanCoilMixedAirNode(EnergyPlusData &state, int const FanCoilNum)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         B Griffith
        //       DATE WRITTEN   Dec  2006

        // PURPOSE OF THIS FUNCTION:
        // lookup function for mixer's return node

        if (state.dataFanCoilUnits->GetFanCoilInputFlag) {
            GetFanCoilUnits(state);
            state.dataFanCoilUnits->GetFanCoilInputFlag = false;
        }

        if (FanCoilNum > 0 && FanCoilNum <= state.dataFanCoilUnits->NumFanCoils) {
            if (state.dataFanCoilUnits->FanCoil(FanCoilNum).OAMixIndex > 0) {
                return MixedAir::GetOAMixerMixedNodeNumber(state, state.dataFanCoilUnits->FanCoil(FanCoilNum).OAMixIndex);
            }
        }

        return 0;
    }

    Real64 CalcFanCoilLoadResidual(EnergyPlusData &state,
                                   int FanCoilNum,            // Index to this fan coil unit
                                   bool FirstHVACIteration,   // FirstHVACIteration flag
                                   int ControlledZoneNum,     // zone index
                                   Real64 QZnReq,             // Sensible load to be met [W]
                                   Real64 const PartLoadRatio // coil part load ratio
    )
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   July 2015

        // PURPOSE OF THIS SUBROUTINE:
        // To calculate the part-load ratio for the FCU with electric heating coil

        Real64 QUnitOut; // delivered capacity [W]
        Calc4PipeFanCoil(state,
                         FanCoilNum,
                         ControlledZoneNum,
                         FirstHVACIteration,
                         QUnitOut,
                         PartLoadRatio); // needs PLR=0 for electric heating coil, otherwise will run a full capacity
        // Calculate residual based on output magnitude
        if (std::abs(QZnReq) <= 100.0) {
            return (QUnitOut - QZnReq) / 100.0;
        } else {
            return (QUnitOut - QZnReq) / QZnReq;
        }
    }

    Real64 CalcFanCoilPLRResidual(EnergyPlusData &state,
                                  Real64 const PLR,        // part-load ratio of air and water mass flow rate
                                  int FanCoilNum,          // Index to this fan coil unit
                                  bool FirstHVACIteration, // FirstHVACIteration flag
                                  int ControlledZoneNum,   // zone index
                                  int WaterControlNode,    // water node to control
                                  Real64 QZnReq            // Sensible load to be met [W] // Function parameters
    )
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   August 2016

        // PURPOSE OF THIS SUBROUTINE:
        Real64 QUnitOut; // delivered capacity [W]
        if (WaterControlNode == state.dataFanCoilUnits->FanCoil(FanCoilNum).CoolCoilFluidInletNode) {
            state.dataLoopNodes->Node(WaterControlNode).MassFlowRate = PLR * state.dataFanCoilUnits->FanCoil(FanCoilNum).MaxCoolCoilFluidFlow;
            Calc4PipeFanCoil(state,
                             FanCoilNum,
                             ControlledZoneNum,
                             FirstHVACIteration,
                             QUnitOut,
                             PLR); // needs PLR=0 for electric heating coil, otherwise will run a full capacity
        } else if (WaterControlNode == state.dataFanCoilUnits->FanCoil(FanCoilNum).HeatCoilFluidInletNode &&
                   state.dataFanCoilUnits->FanCoil(FanCoilNum).HCoilType_Num != HCoil::Electric) {
            state.dataLoopNodes->Node(WaterControlNode).MassFlowRate = PLR * state.dataFanCoilUnits->FanCoil(FanCoilNum).MaxHeatCoilFluidFlow;
            Calc4PipeFanCoil(state,
                             FanCoilNum,
                             ControlledZoneNum,
                             FirstHVACIteration,
                             QUnitOut,
                             PLR); // needs PLR=0 for electric heating coil, otherwise will run a full capacity
        } else {
            Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut, PLR); // needs PLR=1 for electric heating coil
        }

        // Calculate residual based on output magnitude
        if (std::abs(QZnReq) <= 100.0) {
            return (QUnitOut - QZnReq) / 100.0;
        } else {
            return (QUnitOut - QZnReq) / QZnReq;
        }
    }

    Real64 CalcFanCoilHeatCoilPLRResidual(EnergyPlusData &state,
                                          Real64 const CyclingR, // electric heating coil cycling ratio
                                          int const FanCoilNum,
                                          bool const FirstHVACIteration,
                                          int const ZoneNum,
                                          Real64 const QZnReq)
    {
        // PURPOSE OF THIS SUBROUTINE:
        // Calculate electric heating coil cycling ratio of FanCoilUnit with MultiSpeedFan
        // capacity control method when running with at lowest speed for a continuous
        // fan operating mode.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 QUnitOut;            // delivered capacity [W]
        Real64 constexpr PLR = 1.0; // fan coil unit PLR

        // electric heating coil cycling ratio at minimum air flow for constant fan operating mode
        Calc4PipeFanCoil(state, FanCoilNum, ZoneNum, FirstHVACIteration, QUnitOut, PLR, CyclingR);

        // Calculate residual based on output magnitude
        if (std::abs(QZnReq) <= 100.0) {
            return (QUnitOut - QZnReq) / 100.0;
        } else {
            return (QUnitOut - QZnReq) / QZnReq;
        }
    }

    Real64 CalcFanCoilCWLoadResidual(EnergyPlusData &state,
                                     Real64 const CWFlow, // water mass flow rate [kg/s]
                                     int const FanCoilNum,
                                     bool const FirstHVACIteration,
                                     int const ControlledZoneNum,
                                     Real64 const QZnReq)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Fred Buhl Jan 2016
        //       DATE WRITTEN   July 2015

        // PURPOSE OF THIS SUBROUTINE:
        // To calculate the part-load ratio for the FCU with electric heating coil

        Real64 QUnitOut; // delivered capacity [W]
        state.dataLoopNodes->Node(state.dataFanCoilUnits->FanCoil(FanCoilNum).CoolCoilFluidInletNode).MassFlowRate = CWFlow;
        Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut, 1.0);

        // Calculate residual based on output magnitude
        if (std::abs(QZnReq) <= 100.0) {
            return (QUnitOut - QZnReq) / 100.0;
        } else {
            return (QUnitOut - QZnReq) / QZnReq;
        }
    }

    Real64 CalcFanCoilWaterFlowResidual(EnergyPlusData &state,
                                        Real64 const PLR,
                                        int FanCoilNum,
                                        bool FirstHVACIteration,
                                        int ControlledZoneNum,
                                        Real64 QZnReq,
                                        int AirInNode,
                                        int WaterControlNode,
                                        Real64 maxCoilFluidFlow,
                                        Real64 AirMassFlowRate)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   December 2015

        // PURPOSE OF THIS SUBROUTINE:
        // To calculate the part-load ratio for the FCU with varying water flow rate

        Real64 const mDot = PLR * maxCoilFluidFlow;
        if (WaterControlNode > 0) {
            state.dataLoopNodes->Node(WaterControlNode).MassFlowRate = mDot;
        }
        state.dataLoopNodes->Node(AirInNode).MassFlowRate = AirMassFlowRate;

        Real64 QUnitOut;
        if (WaterControlNode == state.dataFanCoilUnits->FanCoil(FanCoilNum).CoolCoilFluidInletNode ||
            (WaterControlNode == state.dataFanCoilUnits->FanCoil(FanCoilNum).HeatCoilFluidInletNode &&
             state.dataFanCoilUnits->FanCoil(FanCoilNum).HCoilType_Num != HCoil::Electric)) {

            Calc4PipeFanCoil(state,
                             FanCoilNum,
                             ControlledZoneNum,
                             FirstHVACIteration,
                             QUnitOut,
                             0.0); // needs PLR=0 for electric heating coil, otherwise will run a full capacity

        } else {
            Calc4PipeFanCoil(state, FanCoilNum, ControlledZoneNum, FirstHVACIteration, QUnitOut, PLR);
        }

        // Calculate residual based on output magnitude
        if (std::abs(QZnReq) <= 100.0) {
            return (QUnitOut - QZnReq) / 100.0;
        } else {
            return (QUnitOut - QZnReq) / QZnReq;
        }
    }

    Real64 CalcFanCoilAirAndWaterFlowResidual(EnergyPlusData &state,
                                              Real64 const PLR, // water and air part load ratio
                                              int FanCoilNum,
                                              bool FirstHVACIteration,
                                              int ControlledZoneNum,
                                              Real64 QZnReq,
                                              int AirInNode,
                                              int WaterControlNode,
                                              Real64 MinWaterFlow)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad, FSEC
        //       DATE WRITTEN   December 2015

        // PURPOSE OF THIS SUBROUTINE:
        // To calculate the part-load ratio for the FCU with varying water flow rate

        // METHODOLOGY EMPLOYED:
        // Use SolveRoot to CALL this Function to converge on a solution

        // set air flow rate
        state.dataLoopNodes->Node(AirInNode).MassFlowRate =
            state.dataFanCoilUnits->FanCoil(FanCoilNum).MaxAirMassFlow *
            (state.dataFanCoilUnits->FanCoil(FanCoilNum).LowSpeedRatio + (PLR * (1.0 - state.dataFanCoilUnits->FanCoil(FanCoilNum).LowSpeedRatio)));
        // set water flow rate
        if (WaterControlNode == state.dataFanCoilUnits->FanCoil(FanCoilNum).CoolCoilFluidInletNode) {
            state.dataLoopNodes->Node(WaterControlNode).MassFlowRate =
                MinWaterFlow + (PLR * (state.dataFanCoilUnits->FanCoil(FanCoilNum).MaxCoolCoilFluidFlow - MinWaterFlow));
        } else if (WaterControlNode == state.dataFanCoilUnits->FanCoil(FanCoilNum).HeatCoilFluidInletNode) {
            state.dataLoopNodes->Node(WaterControlNode).MassFlowRate =
                MinWaterFlow + (PLR * (state.dataFanCoilUnits->FanCoil(FanCoilNum).MaxHeatCoilFluidFlow - MinWaterFlow));
        } else {
            // developer error
            ShowFatalError(state,
                           format("Developer Error - CalcFanCoilAirAndWaterFlowResidual: Water control node not found for {}",
                                  state.dataFanCoilUnits->FanCoil(FanCoilNum).Name));
        }
        Real64 QUnitOut; // delivered capacity [W]
        Calc4PipeFanCoil(state,
                         FanCoilNum,
                         ControlledZoneNum,
                         FirstHVACIteration,
                         QUnitOut,
                         PLR); // needs PLR for electric heating coil to output a specific capacity

        // Calculate residual based on output magnitude
        if (std::abs(QZnReq) <= 100.0) {
            return (QUnitOut - QZnReq) / 100.0;
        } else {
            return (QUnitOut - QZnReq) / QZnReq;
        }
    }

    int getEqIndex(EnergyPlusData &state, std::string_view CompName)
    {
        if (state.dataFanCoilUnits->GetFanCoilInputFlag) {
            GetFanCoilUnits(state);
            state.dataFanCoilUnits->GetFanCoilInputFlag = false;
        }

        for (int FanCoilIndex = 1; FanCoilIndex <= state.dataFanCoilUnits->NumFanCoils; ++FanCoilIndex) {
            auto &fanCoil = state.dataFanCoilUnits->FanCoil(FanCoilIndex);
            if (Util::SameString(fanCoil.Name, CompName)) {
                return FanCoilIndex;
            }
        }

        return 0;
    }

} // namespace FanCoilUnits

} // namespace EnergyPlus

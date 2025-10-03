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
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/CurveManager.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DesiccantDehumidifiers.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/HeatRecovery.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutAirNodeManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SteamCoils.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/VariableSpeedCoils.hh>
#include <EnergyPlus/WaterCoils.hh>

namespace EnergyPlus {

namespace DesiccantDehumidifiers {

    // Module containing the routines dealing with dehumidifiers

    // MODULE INFORMATION:
    //       AUTHOR         Michael J. Witte, GARD Analytics, Inc.
    //                      for Gas Research Institute
    //       DATE WRITTEN   March 2001
    //       MODIFIED       Jan 2005 M. J. Witte, GARD Analytics, Inc.
    //                        Add new control type option:
    //                          NODE LEAVING HUMRAT SETPOINT:BYPASS
    //                        Change existing control type to:
    //                          FIXED LEAVING HUMRAT SETPOINT:BYPASS
    //                        Work supported by ASHRAE research project 1254-RP
    //                      June 2007 R. Raustad, FSEC
    //                        Added new dehumidifier type -- DESICCANT DEHUMIDIFIER
    //                      Jan 2012  B. Nigusse, FSEC
    //                        Added steam and hot water heating coils

    //       RE-ENGINEERED  na

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and routines required to model desiccant dehumidifier
    // components in the EnergyPlus HVAC simulation

    // METHODOLOGY EMPLOYED:
    // The desiccant dehumidifier emcompasses not just the component but also its
    // control. The desiccant dehumidifier removes moisture from its air inlet to meet
    // the HumRatMax setpoint at its exit node. The HumRatMax is set by
    // an external setpoint manager or is a fixed user input.

    // REFERENCES: na

    // OTHER NOTES: This module is based substantially on the Humidifiers module.
    //              authored by Fred Buhl.
    //              Development of portions of this module was funded by the Gas Research Institute.
    //              (Please see copyright and disclaimer information at end of module)

    Real64 constexpr TempSteamIn = 100.0;

    void SimDesiccantDehumidifier(EnergyPlusData &state,
                                  std::string const &CompName,   // name of the dehumidifier unit
                                  bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                                  int &CompIndex)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Michael J. Witte, GARD Analytics, Inc.
        //                      for Gas Research Institute
        //       DATE WRITTEN   March 2001
        //       MODIFIED       June 2007, R. Raustad, Added new dehumidifier type -- DESICCANT DEHUMIDIFIER

        // PURPOSE OF THIS SUBROUTINE:
        // Manage the simulation of an air dehumidifier

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int DesicDehumNum;   // index of solid desiccant unit being simulated
        Real64 HumRatNeeded; // process air leaving humidity ratio set by controller [kg water/kg air]

        if (state.dataDesiccantDehumidifiers->GetInputDesiccantDehumidifier) {
            GetDesiccantDehumidifierInput(state);
            state.dataDesiccantDehumidifiers->GetInputDesiccantDehumidifier = false;
        }

        // Get the desiccant dehumidifier unit index
        if (CompIndex == 0) {
            DesicDehumNum = Util::FindItemInList(CompName, state.dataDesiccantDehumidifiers->DesicDehum);
            if (DesicDehumNum == 0) {
                ShowFatalError(state, format("SimDesiccantDehumidifier: Unit not found={}", CompName));
            }
            CompIndex = DesicDehumNum;
        } else {
            DesicDehumNum = CompIndex;
            if (DesicDehumNum > state.dataDesiccantDehumidifiers->NumDesicDehums || DesicDehumNum < 1) {
                ShowFatalError(state,
                               format("SimDesiccantDehumidifier:  Invalid CompIndex passed={}, Number of Units={}, Entered Unit name={}",
                                      DesicDehumNum,
                                      state.dataDesiccantDehumidifiers->NumDesicDehums,
                                      CompName));
            }
            if (CompName != state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).Name) {
                ShowFatalError(state,
                               format("SimDesiccantDehumidifier: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                      DesicDehumNum,
                                      CompName,
                                      state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).Name));
            }
        }

        InitDesiccantDehumidifier(state, DesicDehumNum, FirstHVACIteration);

        ControlDesiccantDehumidifier(state, DesicDehumNum, HumRatNeeded, FirstHVACIteration);

        // call the correct dehumidifier calculation routine
        switch (state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).DehumTypeCode) {
        case DesicDehumType::Solid: {
            CalcSolidDesiccantDehumidifier(state, DesicDehumNum, HumRatNeeded, FirstHVACIteration);
        } break;
        case DesicDehumType::Generic: {
            CalcGenericDesiccantDehumidifier(state, DesicDehumNum, HumRatNeeded, FirstHVACIteration);
        } break;
        default: {
            ShowFatalError(state,
                           format("Invalid type, Desiccant Dehumidifer={}", state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).DehumType));
        } break;
        }

        UpdateDesiccantDehumidifier(state, DesicDehumNum);

        ReportDesiccantDehumidifier(state, DesicDehumNum);
    }

    void GetDesiccantDehumidifierInput(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Michael J. Witte, GARD Analytics, Inc.
        //                      for Gas Research Institute
        //       DATE WRITTEN   March 2001
        //       MODIFIED       Jan 2005 M. J. Witte, GARD Analytics, Inc.
        //                        Add new control type option:
        //                          NODE LEAVING HUMRAT SETPOINT:BYPASS
        //                        Change existing control type to:
        //                          FIXED LEAVING HUMRAT SETPOINT:BYPASS
        //                        Work supported by ASHRAE research project 1254-RP
        //                      June 2007 R. Raustad, FSEC
        //                        Added new dehumidifier type -- DESICCANT DEHUMIDIFIER

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for humidifiers and stores it in dehumidifier data structures.

        // METHODOLOGY EMPLOYED:
        // Uses InputProcessor "Get" routines to obtain data.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("GetDesiccantDehumidifierInput: "); // include trailing blank space
        static constexpr std::string_view routineName = "GetDesiccantDehumidifierInput";
        static std::string const dehumidifierDesiccantNoFans("Dehumidifier:Desiccant:NoFans");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;                  // Number of Alphas for each GetObjectItem call
        int NumNumbers;                 // Number of Numbers for each GetObjectItem call
        int IOStatus;                   // Used in GetObjectItem
        bool ErrorsFound(false);        // Set to true if errors in input, fatal at end of routine
        bool ErrorsFound2(false);       // Set to true if errors in input, fatal at end of routine
        bool ErrorsFoundGeneric(false); // Set to true if errors in input, fatal at end of routine
        bool IsNotOK;                   // Flag to verify name
        bool OANodeError;               // Flag for check on outside air node
        std::string RegenCoilInlet;     // Desiccant system regeneration air heater inlet node
        std::string RegenCoilOutlet;    // Desiccant system regeneration air heater outlet node
        int DesuperHeaterIndex;         // Index of desuperheater heating coil
        int RegenCoilControlNodeNum;    // Control node number of regen heating coil
        Real64 CoilBypassedFlowFrac;    // Bypass air fraction for multimode DX coils
        Array1D_string Alphas;          // Alpha input items for object
        Array1D_string cAlphaFields;    // Alpha field names
        Array1D_string cNumericFields;  // Numeric field names
        Array1D<Real64> Numbers;        // Numeric input items for object
        Array1D_bool lAlphaBlanks;      // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;    // Logical array, numeric field input BLANK = .TRUE.
        bool errFlag;                   // local error flag
        std::string RegenCoilType;      // Regen heating coil type
        std::string RegenCoilName;      // Regen heating coil name
        bool RegairHeatingCoilFlag;     // local error flag

        int TotalArgs = 0;

        state.dataDesiccantDehumidifiers->NumSolidDesicDehums =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, dehumidifierDesiccantNoFans);
        state.dataDesiccantDehumidifiers->NumGenericDesicDehums =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "Dehumidifier:Desiccant:System");
        state.dataDesiccantDehumidifiers->NumDesicDehums =
            state.dataDesiccantDehumidifiers->NumSolidDesicDehums + state.dataDesiccantDehumidifiers->NumGenericDesicDehums;
        // allocate the data array
        state.dataDesiccantDehumidifiers->DesicDehum.allocate(state.dataDesiccantDehumidifiers->NumDesicDehums);
        state.dataDesiccantDehumidifiers->UniqueDesicDehumNames.reserve(state.dataDesiccantDehumidifiers->NumDesicDehums);
        state.dataDesiccantDehumidifiers->GetInputDesiccantDehumidifier = false;

        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, dehumidifierDesiccantNoFans, TotalArgs, NumAlphas, NumNumbers);
        int MaxNums = NumNumbers;
        int MaxAlphas = NumAlphas;
        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "Dehumidifier:Desiccant:System", TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);

        Alphas.allocate(MaxAlphas);
        cAlphaFields.allocate(MaxAlphas);
        cNumericFields.allocate(MaxNums);
        Numbers.dimension(MaxNums, 0.0);
        lAlphaBlanks.dimension(MaxAlphas, true);
        lNumericBlanks.dimension(MaxNums, true);

        // loop over solid desiccant dehumidifiers and load the input data
        std::string CurrentModuleObject = dehumidifierDesiccantNoFans;
        for (int DesicDehumIndex = 1; DesicDehumIndex <= state.dataDesiccantDehumidifiers->NumSolidDesicDehums; ++DesicDehumIndex) {
            auto &desicDehum = state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumIndex);

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     DesicDehumIndex,
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

            GlobalNames::VerifyUniqueInterObjectName(
                state, state.dataDesiccantDehumidifiers->UniqueDesicDehumNames, Alphas(1), CurrentModuleObject, cAlphaFields(1), ErrorsFound);
            desicDehum.Name = Alphas(1);
            desicDehum.DehumType = CurrentModuleObject;
            desicDehum.DehumTypeCode = DesicDehumType::Solid;
            desicDehum.Sched = Alphas(2);

            if (lAlphaBlanks(2)) {
                desicDehum.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((desicDehum.availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ErrorsFound = true;
            }
            // For node connections, this object is both a parent and a non-parent, because the
            // Desiccant wheel is not called out as a separate component, its nodes must be connected
            // as ObjectIsNotParent.  But for the Regen fan, the nodes are connected as ObjectIsParent
            desicDehum.ProcAirInNode = NodeInputManager::GetOnlySingleNode(state,
                                                                           Alphas(3),
                                                                           ErrorsFound,
                                                                           DataLoopNode::ConnectionObjectType::DehumidifierDesiccantNoFans,
                                                                           Alphas(1),
                                                                           DataLoopNode::NodeFluidType::Air,
                                                                           DataLoopNode::ConnectionType::Inlet,
                                                                           NodeInputManager::CompFluidStream::Primary,
                                                                           DataLoopNode::ObjectIsNotParent);

            desicDehum.ProcAirOutNode = NodeInputManager::GetOnlySingleNode(state,
                                                                            Alphas(4),
                                                                            ErrorsFound,
                                                                            DataLoopNode::ConnectionObjectType::DehumidifierDesiccantNoFans,
                                                                            Alphas(1),
                                                                            DataLoopNode::NodeFluidType::Air,
                                                                            DataLoopNode::ConnectionType::Outlet,
                                                                            NodeInputManager::CompFluidStream::Primary,
                                                                            DataLoopNode::ObjectIsNotParent);

            desicDehum.RegenAirInNode = NodeInputManager::GetOnlySingleNode(state,
                                                                            Alphas(5),
                                                                            ErrorsFound,
                                                                            DataLoopNode::ConnectionObjectType::DehumidifierDesiccantNoFans,
                                                                            Alphas(1),
                                                                            DataLoopNode::NodeFluidType::Air,
                                                                            DataLoopNode::ConnectionType::Inlet,
                                                                            NodeInputManager::CompFluidStream::Secondary,
                                                                            DataLoopNode::ObjectIsNotParent);

            desicDehum.RegenFanInNode = NodeInputManager::GetOnlySingleNode(state,
                                                                            Alphas(6),
                                                                            ErrorsFound,
                                                                            DataLoopNode::ConnectionObjectType::DehumidifierDesiccantNoFans,
                                                                            Alphas(1),
                                                                            DataLoopNode::NodeFluidType::Air,
                                                                            DataLoopNode::ConnectionType::Internal,
                                                                            NodeInputManager::CompFluidStream::Secondary,
                                                                            DataLoopNode::ObjectIsParent);

            if (Util::SameString(Alphas(7), "LEAVING HUMRAT:BYPASS")) {
                ShowWarningError(state, format("{}{} = {}", RoutineName, CurrentModuleObject, desicDehum.Name));
                ShowContinueError(state, format("Obsolete {} = {}", cAlphaFields(7), Alphas(7)));
                ShowContinueError(state, "setting to LeavingMaximumHumidityRatioSetpoint");
                desicDehum.controlType = DesicDehumCtrlType::FixedHumratBypass;
            }
            if (Util::SameString(Alphas(7), "LeavingMaximumHumidityRatioSetpoint")) {
                desicDehum.controlType = DesicDehumCtrlType::FixedHumratBypass;
            }
            if (Util::SameString(Alphas(7), "SystemNodeMaximumHumidityRatioSetpoint")) {
                desicDehum.controlType = DesicDehumCtrlType::NodeHumratBypass;
            }
            if (desicDehum.controlType == DesicDehumCtrlType::Invalid) {
                ShowWarningError(state, format("{}{} = {}", RoutineName, CurrentModuleObject, desicDehum.Name));
                ShowContinueError(state, format("Invalid {} = {}", cAlphaFields(7), Alphas(7)));
                ShowContinueError(state, "setting to LeavingMaximumHumidityRatioSetpoint");
                desicDehum.controlType = DesicDehumCtrlType::FixedHumratBypass;
            }
            desicDehum.HumRatSet = Numbers(1);
            desicDehum.NomProcAirVolFlow = Numbers(2);
            desicDehum.NomProcAirVel = Numbers(3);

            desicDehum.RegenCoilType = Alphas(8);
            desicDehum.RegenCoilName = Alphas(9);

            desicDehum.regenFanType = static_cast<HVAC::FanType>(getEnumValue(HVAC::fanTypeNamesUC, Alphas(10)));
            assert(desicDehum.regenFanType != HVAC::FanType::Invalid);

            RegenCoilType = Alphas(8);
            RegenCoilName = Alphas(9);

            if (Util::SameString(desicDehum.RegenCoilType, "Coil:Heating:Electric") ||
                Util::SameString(desicDehum.RegenCoilType, "Coil:Heating:Fuel")) {
                if (Util::SameString(desicDehum.RegenCoilType, "Coil:Heating:Electric")) {
                    desicDehum.RegenCoilType_Num = HVAC::Coil_HeatingElectric;
                }
                if (Util::SameString(desicDehum.RegenCoilType, "Coil:Heating:Fuel")) {
                    desicDehum.RegenCoilType_Num = HVAC::Coil_HeatingGasOrOtherFuel;
                }
                ValidateComponent(state, desicDehum.RegenCoilType, desicDehum.RegenCoilName, ErrorsFound2, CurrentModuleObject + '=' + Alphas(1));
                if (ErrorsFound2) {
                    ErrorsFound = true;
                }
                HeatingCoils::GetCoilIndex(state, desicDehum.RegenCoilName, desicDehum.RegenCoilIndex, ErrorsFound2);
                if (ErrorsFound2) {
                    ErrorsFound = true;
                }

            } else if (Util::SameString(desicDehum.RegenCoilType, "Coil:Heating:Water")) {
                desicDehum.RegenCoilType_Num = HVAC::Coil_HeatingWater;
                ValidateComponent(state, RegenCoilType, RegenCoilName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else { // mine data from heating coil object
                    errFlag = false;
                    desicDehum.RegenCoilIndex = WaterCoils::GetWaterCoilIndex(state, "COIL:HEATING:WATER", RegenCoilName, errFlag);
                    if (desicDehum.RegenCoilIndex == 0) {
                        ShowSevereError(state, format("{}{} illegal {} = {}", RoutineName, CurrentModuleObject, cAlphaFields(9), RegenCoilName));
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, desicDehum.Name));
                        ErrorsFound = true;
                    }

                    // Get the Heating Coil Hot water Inlet or control Node number
                    errFlag = false;
                    desicDehum.CoilControlNode = WaterCoils::GetCoilWaterInletNode(state, "Coil:Heating:Water", RegenCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, desicDehum.Name));
                        ErrorsFound = true;
                    }

                    // Get the Regeneration Heating Coil hot water max volume flow rate
                    errFlag = false;
                    desicDehum.MaxCoilFluidFlow = WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", RegenCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, desicDehum.Name));
                        ErrorsFound = true;
                    }

                    // Get the Regeneration Heating Coil Inlet Node
                    errFlag = false;
                    int RegenCoilAirInletNode = WaterCoils::GetCoilInletNode(state, "Coil:Heating:Water", RegenCoilName, errFlag);
                    desicDehum.RegenCoilInletNode = RegenCoilAirInletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, desicDehum.Name));
                        ErrorsFound = true;
                    }

                    // Get the Regeneration Heating Coil Outlet Node
                    errFlag = false;
                    int RegenCoilAirOutletNode = WaterCoils::GetCoilOutletNode(state, "Coil:Heating:Water", RegenCoilName, errFlag);
                    desicDehum.RegenCoilOutletNode = RegenCoilAirOutletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, desicDehum.Name));
                        ErrorsFound = true;
                    }
                }
            } else if (Util::SameString(desicDehum.RegenCoilType, "Coil:Heating:Steam")) {
                desicDehum.RegenCoilType_Num = HVAC::Coil_HeatingSteam;
                ValidateComponent(state, Alphas(8), RegenCoilName, IsNotOK, CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else { // mine data from the regeneration heating coil object

                    errFlag = false;
                    desicDehum.RegenCoilIndex = SteamCoils::GetSteamCoilIndex(state, "COIL:HEATING:STEAM", RegenCoilName, errFlag);
                    if (desicDehum.RegenCoilIndex == 0) {
                        ShowSevereError(state, format("{}{} illegal {} = {}", RoutineName, CurrentModuleObject, cAlphaFields(9), RegenCoilName));
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, desicDehum.Name));
                        ErrorsFound = true;
                    }

                    // Get the regeneration Heating Coil steam inlet node number
                    errFlag = false;
                    desicDehum.CoilControlNode = SteamCoils::GetCoilSteamInletNode(state, "Coil:Heating:Steam", RegenCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, desicDehum.Name));
                        ErrorsFound = true;
                    }

                    // Get the regeneration heating Coil steam max volume flow rate
                    desicDehum.MaxCoilFluidFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, desicDehum.RegenCoilIndex, errFlag);
                    if (desicDehum.MaxCoilFluidFlow > 0.0) {
                        Real64 SteamDensity = Fluid::GetSteam(state)->getSatDensity(state, TempSteamIn, 1.0, dehumidifierDesiccantNoFans);
                        desicDehum.MaxCoilFluidFlow *= SteamDensity;
                    }

                    // Get the regeneration heating Coil Inlet Node
                    errFlag = false;
                    int RegenCoilAirInletNode = SteamCoils::GetCoilAirInletNode(state, desicDehum.RegenCoilIndex, RegenCoilName, errFlag);
                    desicDehum.RegenCoilInletNode = RegenCoilAirInletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, desicDehum.Name));
                        ErrorsFound = true;
                    }

                    // Get the regeneration heating Coil Outlet Node
                    errFlag = false;
                    int RegenCoilAirOutletNode = SteamCoils::GetCoilAirOutletNode(state, desicDehum.RegenCoilIndex, RegenCoilName, errFlag);
                    desicDehum.RegenCoilOutletNode = RegenCoilAirOutletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, desicDehum.Name));
                        ErrorsFound = true;
                    }
                }
            } else {
                ShowSevereError(state, format("{}{} = {}", RoutineName, CurrentModuleObject, Alphas(1)));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(8), desicDehum.RegenCoilType));
                ErrorsFound = true;
            }

            desicDehum.NomRotorPower = Numbers(4);
            desicDehum.RegenFanName = Alphas(11);

            BranchNodeConnections::TestCompSet(state, desicDehum.DehumType, desicDehum.Name, Alphas(3), Alphas(4), "Process Air Nodes");

            // Set up component set for regen coil
            BranchNodeConnections::SetUpCompSets(state, desicDehum.DehumType, desicDehum.Name, Alphas(8), Alphas(9), "UNDEFINED", "UNDEFINED");

            // Set up component set for regen fan
            BranchNodeConnections::SetUpCompSets(state, desicDehum.DehumType, desicDehum.Name, Alphas(10), Alphas(11), Alphas(6), "UNDEFINED");

            if ((!Util::SameString(Alphas(12), "Default")) && (Util::SameString(Alphas(12), "UserCurves"))) {
                ShowWarningError(state, format("{}{}: Invalid{} = {}", RoutineName, CurrentModuleObject, cAlphaFields(12), Alphas(12)));
                ShowContinueError(state, "resetting to Default");
                desicDehum.PerformanceModel_Num = PerformanceModel::Default;
            }

            if (Util::SameString(Alphas(12), "UserCurves")) {
                desicDehum.PerformanceModel_Num = PerformanceModel::UserCurves;
                desicDehum.ProcDryBulbCurvefTW = Curve::GetCurveIndex(state, Alphas(13));
                if (desicDehum.ProcDryBulbCurvefTW == 0) {
                    ShowSevereError(state, format("{}Curve object={} not found.", RoutineName, Alphas(13)));
                    ErrorsFound2 = true;
                }
                desicDehum.ProcDryBulbCurvefV = Curve::GetCurveIndex(state, Alphas(14));
                if (desicDehum.ProcDryBulbCurvefV == 0) {
                    ShowSevereError(state, format("{}Curve object={} not found.", RoutineName, Alphas(14)));
                    ErrorsFound2 = true;
                }
                desicDehum.ProcHumRatCurvefTW = Curve::GetCurveIndex(state, Alphas(15));
                if (desicDehum.ProcHumRatCurvefTW == 0) {
                    ShowSevereError(state, format("{}Curve object={} not found.", RoutineName, Alphas(15)));
                    ErrorsFound2 = true;
                }
                desicDehum.ProcHumRatCurvefV = Curve::GetCurveIndex(state, Alphas(16));
                if (desicDehum.ProcHumRatCurvefV == 0) {
                    ShowSevereError(state, format("{}Curve object={} not found.", RoutineName, Alphas(16)));
                    ErrorsFound2 = true;
                }
                desicDehum.RegenEnergyCurvefTW = Curve::GetCurveIndex(state, Alphas(17));
                if (desicDehum.RegenEnergyCurvefTW == 0) {
                    ShowSevereError(state, format("{}Curve object={} not found.", RoutineName, Alphas(17)));
                    ErrorsFound2 = true;
                }
                desicDehum.RegenEnergyCurvefV = Curve::GetCurveIndex(state, Alphas(18));
                if (desicDehum.RegenEnergyCurvefV == 0) {
                    ShowSevereError(state, format("{}Curve object={} not found.", RoutineName, Alphas(18)));
                    ErrorsFound2 = true;
                }
                desicDehum.RegenVelCurvefTW = Curve::GetCurveIndex(state, Alphas(19));
                if (desicDehum.RegenVelCurvefTW == 0) {
                    ShowSevereError(state, format("{}Curve object={} not found.", RoutineName, Alphas(19)));
                    ErrorsFound2 = true;
                }
                desicDehum.RegenVelCurvefV = Curve::GetCurveIndex(state, Alphas(20));
                if (desicDehum.RegenVelCurvefV == 0) {
                    ShowSevereError(state, format("{}Curve object={} not found.", RoutineName, Alphas(20)));
                    ErrorsFound2 = true;
                }
                if (ErrorsFound2) {
                    ShowSevereError(state, format("{}{} = {}", RoutineName, CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, "Errors found in getting performance curves.");
                    ErrorsFound = true;
                }
                desicDehum.NomRegenTemp = Numbers(5);

                // Validate regen fan type, for user defined curves, can be constant or variable volume
                if (desicDehum.regenFanType == HVAC::FanType::Constant || desicDehum.regenFanType == HVAC::FanType::VAV ||
                    desicDehum.regenFanType == HVAC::FanType::SystemModel) {
                    ValidateComponent(state,
                                      HVAC::fanTypeNamesUC[(int)desicDehum.regenFanType],
                                      desicDehum.RegenFanName,
                                      ErrorsFound2,
                                      CurrentModuleObject + " = " + Alphas(1));
                    if (ErrorsFound2) {
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(10), Alphas(10)));
                    ErrorsFound = true;
                }
            } else {
                // If DEFAULT performance model, set operating limits curves.  Unit is off outside this range
                desicDehum.PerformanceModel_Num = PerformanceModel::Default;
                // this is wrong to initialize all dehumidifiers, it should be just this specific dehumidifier
                // this was likely tested with only 1 desiccant dehumidifier so this was never discovered
                // or maybe if there were more than 1 these data would not be used when Alphas(12) == "UserCurves" ???
                for (auto &e : state.dataDesiccantDehumidifiers->DesicDehum) {
                    e.MinProcAirInTemp = 1.67;       //  35 F
                    e.MaxProcAirInTemp = 48.89;      // 120 F
                    e.MinProcAirInHumRat = 0.002857; //  20 gr/lb
                    e.MaxProcAirInHumRat = 0.02857;  // 200 gr/lb
                }
                //  If DEFAULT performance model, warn if curve names and nominal regen temp have values
                if ((!lAlphaBlanks(13)) || (!lAlphaBlanks(14)) || (!lAlphaBlanks(15)) || (!lAlphaBlanks(16)) || (!lAlphaBlanks(17)) ||
                    (!lAlphaBlanks(18)) || (!lAlphaBlanks(19)) || (!lAlphaBlanks(20))) {
                    ShowWarningError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, "DEFAULT performance selected, curve names and nominal regen temp will be ignored.");
                }
                if (desicDehum.NomProcAirVel > 4.064) {
                    ShowWarningError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("{} > 4.064 m/s.; Value in input={:.3R}", cNumericFields(3), desicDehum.NomProcAirVel));
                    ShowContinueError(state, "DEFAULT performance curves not valid outside 2.032 to 4.064 m/s (400 to 800 fpm).");
                }
                if (desicDehum.NomProcAirVel < 2.032) {
                    ShowWarningError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("{} < 2.032 m/s.; Value in input={:.3R}", cNumericFields(3), desicDehum.NomProcAirVel));
                    ShowContinueError(state, "DEFAULT performance curves not valid outside 2.032 to 4.064 m/s (400 to 800 fpm).");
                }
                // Validate regen fan type, for default curves, can only variable volume
                if (desicDehum.regenFanType == HVAC::FanType::VAV || desicDehum.regenFanType == HVAC::FanType::SystemModel) {
                    ValidateComponent(state,
                                      HVAC::fanTypeNamesUC[(int)desicDehum.regenFanType],
                                      desicDehum.RegenFanName,
                                      ErrorsFound2,
                                      CurrentModuleObject + " = " + Alphas(1));
                    if (ErrorsFound2) {
                        ErrorsFound = true;
                    }
                } else {
                    ShowSevereError(state, format("{} = {}", CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(10), Alphas(10)));
                    ShowContinueError(state, "For DEFAULT performance model, the regen fan type must be Fan:VariableVolume");
                    ErrorsFound = true;
                }
            }
            // process regen fan
            ErrorsFound2 = false;

            desicDehum.RegenFanIndex = Fans::GetFanIndex(state, desicDehum.RegenFanName);
            if (desicDehum.RegenFanIndex == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(11), desicDehum.RegenFanName);
                ErrorsFoundGeneric = true;
            } else {
                auto *fan = state.dataFans->fans(desicDehum.RegenFanIndex);
                assert(desicDehum.regenFanType == fan->type);
                desicDehum.RegenFanInNode = fan->inletNodeNum;
                desicDehum.RegenFanOutNode = fan->outletNodeNum;
            }
        }

        for (int DesicDehumIndex = 1; DesicDehumIndex <= state.dataDesiccantDehumidifiers->NumGenericDesicDehums; ++DesicDehumIndex) {

            CurrentModuleObject = "Dehumidifier:Desiccant:System";

            int DesicDehumNum = DesicDehumIndex + state.dataDesiccantDehumidifiers->NumSolidDesicDehums;
            auto &desicDehum = state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum);

            desicDehum.DehumType = CurrentModuleObject;
            desicDehum.DehumTypeCode = DesicDehumType::Generic;
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     desicDehum.DehumType,
                                                                     DesicDehumIndex,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            ErrorObjectHeader eoh{routineName, desicDehum.DehumType, Alphas(1)};

            GlobalNames::VerifyUniqueInterObjectName(
                state, state.dataDesiccantDehumidifiers->UniqueDesicDehumNames, Alphas(1), CurrentModuleObject, cAlphaFields(1), ErrorsFoundGeneric);
            desicDehum.Name = Alphas(1);

            ErrorsFound2 = false;
            ValidateComponent(state, desicDehum.DehumType, desicDehum.Name, ErrorsFound2, desicDehum.DehumType + " = \"" + desicDehum.Name + "\"");
            if (ErrorsFound2) {
                ShowSevereError(state, format("{} \"{}\" is not unique", desicDehum.DehumType, desicDehum.Name));
                ErrorsFoundGeneric = true;
            }

            desicDehum.Sched = Alphas(2);
            if (lAlphaBlanks(2)) {
                desicDehum.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((desicDehum.availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ErrorsFound = true;
            }

            desicDehum.HXType = Alphas(3);
            desicDehum.HXName = Alphas(4);

            if (!Util::SameString(desicDehum.HXType, "HeatExchanger:Desiccant:BalancedFlow")) {
                ShowWarningError(state, format("{} = \"{}\"", desicDehum.DehumType, desicDehum.Name));
                ShowContinueError(state, format("Invalid {} = {}", cAlphaFields(3), desicDehum.HXType));
                ErrorsFoundGeneric = true;
            } else {
                desicDehum.HXTypeNum = BalancedHX;
            }

            ErrorsFound2 = false;
            ValidateComponent(state, desicDehum.HXType, desicDehum.HXName, ErrorsFound2, desicDehum.DehumType + " = \"" + desicDehum.Name + "\"");
            if (ErrorsFound2) {
                ErrorsFoundGeneric = true;
            }

            ErrorsFound2 = false;
            desicDehum.HXProcInNode = HeatRecovery::GetSecondaryInletNode(state, desicDehum.HXName, ErrorsFound2);
            if (ErrorsFound2) {
                ShowContinueError(state, format("...occurs in {} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                ErrorsFoundGeneric = true;
            }

            std::string ProcAirInlet = state.dataLoopNodes->NodeID(desicDehum.HXProcInNode);

            desicDehum.ProcAirInNode = NodeInputManager::GetOnlySingleNode(state,
                                                                           ProcAirInlet,
                                                                           ErrorsFound,
                                                                           DataLoopNode::ConnectionObjectType::DehumidifierDesiccantSystem,
                                                                           desicDehum.Name,
                                                                           DataLoopNode::NodeFluidType::Air,
                                                                           DataLoopNode::ConnectionType::Inlet,
                                                                           NodeInputManager::CompFluidStream::Primary,
                                                                           DataLoopNode::ObjectIsParent);

            ErrorsFound2 = false;
            desicDehum.HXProcOutNode = HeatRecovery::GetSecondaryOutletNode(state, desicDehum.HXName, ErrorsFound2);
            if (ErrorsFound2) {
                ShowContinueError(state, format("...occurs in {} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                ErrorsFoundGeneric = true;
            }

            std::string ProcAirOutlet = state.dataLoopNodes->NodeID(desicDehum.HXProcOutNode);

            desicDehum.ProcAirOutNode = NodeInputManager::GetOnlySingleNode(state,
                                                                            ProcAirOutlet,
                                                                            ErrorsFound,
                                                                            DataLoopNode::ConnectionObjectType::DehumidifierDesiccantSystem,
                                                                            desicDehum.Name,
                                                                            DataLoopNode::NodeFluidType::Air,
                                                                            DataLoopNode::ConnectionType::Outlet,
                                                                            NodeInputManager::CompFluidStream::Primary,
                                                                            DataLoopNode::ObjectIsParent);

            BranchNodeConnections::TestCompSet(state, desicDehum.DehumType, desicDehum.Name, ProcAirInlet, ProcAirOutlet, "Process Air Nodes");

            ErrorsFound2 = false;
            desicDehum.HXRegenInNode = HeatRecovery::GetSupplyInletNode(state, desicDehum.HXName, ErrorsFound2);
            if (ErrorsFound2) {
                ShowContinueError(state, format("...occurs in {} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                ErrorsFoundGeneric = true;
            }

            ErrorsFound2 = false;
            desicDehum.HXRegenOutNode = HeatRecovery::GetSupplyOutletNode(state, desicDehum.HXName, ErrorsFound2);
            if (ErrorsFound2) {
                ShowContinueError(state, format("...occurs in {} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                ErrorsFoundGeneric = true;
            }

            desicDehum.ControlNodeNum = NodeInputManager::GetOnlySingleNode(state,
                                                                            Alphas(5),
                                                                            ErrorsFound,
                                                                            DataLoopNode::ConnectionObjectType::DehumidifierDesiccantSystem,
                                                                            desicDehum.Name,
                                                                            DataLoopNode::NodeFluidType::Air,
                                                                            DataLoopNode::ConnectionType::Sensor,
                                                                            NodeInputManager::CompFluidStream::Primary,
                                                                            DataLoopNode::ObjectIsNotParent);

            if (desicDehum.ControlNodeNum == 0) {
                ShowSevereError(state, format("{} = \"{}\"", desicDehum.DehumType, desicDehum.Name));
                ShowContinueError(state, format("{} must be specified.", cAlphaFields(5)));
                ErrorsFoundGeneric = true;
            }

            desicDehum.regenFanType = static_cast<HVAC::FanType>(getEnumValue(HVAC::fanTypeNamesUC, Alphas(6)));
            assert(desicDehum.regenFanType != HVAC::FanType::Invalid);

            desicDehum.RegenFanName = Alphas(7);

            if (desicDehum.regenFanType == HVAC::FanType::OnOff || desicDehum.regenFanType == HVAC::FanType::Constant ||
                desicDehum.regenFanType == HVAC::FanType::SystemModel) {
                ErrorsFound2 = false;
                ValidateComponent(state,
                                  HVAC::fanTypeNamesUC[(int)desicDehum.regenFanType],
                                  desicDehum.RegenFanName,
                                  ErrorsFound2,
                                  desicDehum.DehumType + " \"" + desicDehum.Name + "\"");
                if (ErrorsFound2) {
                    ErrorsFoundGeneric = true;
                }
            } else {
                ShowSevereError(state, format("{} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(6), HVAC::fanTypeNamesUC[(int)desicDehum.regenFanType]));
                ErrorsFoundGeneric = true;
            }

            desicDehum.regenFanPlace = static_cast<HVAC::FanPlace>(getEnumValue(HVAC::fanPlaceNamesUC, Alphas(8)));
            if (desicDehum.regenFanPlace == HVAC::FanPlace::Invalid) {
                ShowWarningInvalidKey(state, eoh, cAlphaFields(8), Alphas(8), "DrawThrough");
                desicDehum.regenFanPlace = HVAC::FanPlace::DrawThru;
            }

            ErrorsFound2 = false;
            desicDehum.RegenFanIndex = Fans::GetFanIndex(state, desicDehum.RegenFanName);
            if (desicDehum.RegenFanIndex == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(7), desicDehum.RegenFanName);
                ErrorsFoundGeneric = true;
            } else {
                auto *fan = state.dataFans->fans(desicDehum.RegenFanIndex);
                assert(desicDehum.regenFanType == fan->type);
                desicDehum.RegenFanInNode = fan->inletNodeNum;
                desicDehum.RegenFanOutNode = fan->outletNodeNum;
            }

            desicDehum.RegenCoilType = Alphas(9);
            desicDehum.RegenCoilName = Alphas(10);
            RegenCoilType = Alphas(9);
            RegenCoilName = Alphas(10);
            desicDehum.RegenSetPointTemp = Numbers(1);

            if (!lAlphaBlanks(10)) {
                if (Util::SameString(desicDehum.RegenCoilType, "Coil:Heating:Electric") ||
                    Util::SameString(desicDehum.RegenCoilType, "Coil:Heating:Fuel")) {
                    if (Util::SameString(desicDehum.RegenCoilType, "Coil:Heating:Electric")) {
                        desicDehum.RegenCoilType_Num = HVAC::Coil_HeatingElectric;
                    }
                    if (Util::SameString(desicDehum.RegenCoilType, "Coil:Heating:Fuel")) {
                        desicDehum.RegenCoilType_Num = HVAC::Coil_HeatingGasOrOtherFuel;
                    }
                    ErrorsFound2 = false;
                    ValidateComponent(state, RegenCoilType, RegenCoilName, ErrorsFound2, desicDehum.DehumType + " \"" + desicDehum.Name + "\"");
                    if (ErrorsFound2) {
                        ErrorsFoundGeneric = true;
                    }

                    if (desicDehum.RegenSetPointTemp <= 0.0) {
                        ShowSevereError(state, format("{} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                        ShowContinueError(state, format("{} must be greater than 0.", cNumericFields(1)));
                        ErrorsFoundGeneric = true;
                    }

                    ErrorsFound2 = false;
                    desicDehum.RegenCoilInletNode = HeatingCoils::GetCoilInletNode(state, RegenCoilType, RegenCoilName, ErrorsFound2);
                    if (ErrorsFound2) {
                        ShowContinueError(state, format("...occurs in {} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                        ErrorsFoundGeneric = true;
                    }

                    ErrorsFound2 = false;
                    desicDehum.RegenCoilOutletNode = HeatingCoils::GetCoilOutletNode(state, RegenCoilType, RegenCoilName, ErrorsFound2);
                    if (ErrorsFound2) {
                        ShowContinueError(state, format("...occurs in {} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                        ErrorsFoundGeneric = true;
                    }

                    ErrorsFound2 = false;
                    HeatingCoils::GetCoilIndex(state, RegenCoilName, desicDehum.RegenCoilIndex, ErrorsFound2);
                    if (ErrorsFound2) {
                        ShowContinueError(state, format("...occurs in {} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                        ErrorsFoundGeneric = true;
                    }

                    ErrorsFound2 = false;
                    RegenCoilControlNodeNum = HeatingCoils::GetCoilControlNodeNum(state, RegenCoilType, RegenCoilName, ErrorsFound2);
                    if (ErrorsFound2) {
                        ShowContinueError(state, format("...occurs in {} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                        ErrorsFoundGeneric = true;
                    }

                    if (RegenCoilControlNodeNum > 0) {
                        ShowSevereError(state, format("{} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                        ShowContinueError(state,
                                          format("{} is specified as {:.3R} C in this object.", cNumericFields(1), desicDehum.RegenSetPointTemp));
                        ShowContinueError(state, " Do not specify a coil temperature setpoint node name in the regeneration air heater object.");
                        ShowContinueError(state, format("...{} = {}", cAlphaFields(9), desicDehum.RegenCoilType));
                        ShowContinueError(state, format("...{} = {}", cAlphaFields(10), desicDehum.RegenCoilName));
                        ShowContinueError(
                            state, format("...heating coil temperature setpoint node = {}", state.dataLoopNodes->NodeID(RegenCoilControlNodeNum)));
                        ShowContinueError(state, "...leave the heating coil temperature setpoint node name blank in the regen heater object.");
                        ErrorsFoundGeneric = true;
                    }

                    RegairHeatingCoilFlag = true;
                    HeatingCoils::SetHeatingCoilData(state, desicDehum.RegenCoilIndex, ErrorsFound2, RegairHeatingCoilFlag, DesicDehumNum);
                    if (ErrorsFound2) {
                        ShowContinueError(state, format("...occurs in {} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                        ErrorsFoundGeneric = true;
                    }

                } else if (Util::SameString(desicDehum.RegenCoilType, "Coil:Heating:Water")) {
                    desicDehum.RegenCoilType_Num = HVAC::Coil_HeatingWater;
                    ValidateComponent(state, RegenCoilType, RegenCoilName, IsNotOK, CurrentModuleObject);
                    if (IsNotOK) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    } else { // mine data from heating coil object
                        errFlag = false;
                        desicDehum.RegenCoilIndex = WaterCoils::GetWaterCoilIndex(state, "COIL:HEATING:WATER", RegenCoilName, errFlag);
                        if (desicDehum.RegenCoilIndex == 0) {
                            ShowSevereError(state, format("{} illegal {} = {}", CurrentModuleObject, cAlphaFields(9), RegenCoilName));
                            ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, desicDehum.Name));
                            ErrorsFound = true;
                        }

                        if (desicDehum.RegenSetPointTemp <= 0.0) {
                            ShowSevereError(state, format("{} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                            ShowContinueError(state, format("{} must be greater than 0.", cNumericFields(1)));
                            ErrorsFoundGeneric = true;
                        }

                        // Get the Heating Coil Hot water Inlet or control Node number
                        errFlag = false;
                        desicDehum.CoilControlNode = WaterCoils::GetCoilWaterInletNode(state, "Coil:Heating:Water", RegenCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, desicDehum.Name));
                            ErrorsFound = true;
                        }

                        // Get the Regeneration Heating Coil hot water max volume flow rate
                        errFlag = false;
                        desicDehum.MaxCoilFluidFlow = WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", RegenCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, desicDehum.Name));
                            ErrorsFound = true;
                        }

                        // Get the Regeneration Heating Coil Inlet Node
                        errFlag = false;
                        int RegenCoilAirInletNode = WaterCoils::GetCoilInletNode(state, "Coil:Heating:Water", RegenCoilName, errFlag);
                        desicDehum.RegenCoilInletNode = RegenCoilAirInletNode;
                        if (errFlag) {
                            ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, desicDehum.Name));
                            ErrorsFound = true;
                        }

                        // Get the Regeneration Heating Coil Outlet Node
                        errFlag = false;
                        int RegenCoilAirOutletNode = WaterCoils::GetCoilOutletNode(state, "Coil:Heating:Water", RegenCoilName, errFlag);
                        desicDehum.RegenCoilOutletNode = RegenCoilAirOutletNode;
                        if (errFlag) {
                            ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, desicDehum.Name));
                            ErrorsFound = true;
                        }

                        RegairHeatingCoilFlag = true;
                        WaterCoils::SetWaterCoilData(state, desicDehum.RegenCoilIndex, ErrorsFound2, RegairHeatingCoilFlag, DesicDehumNum);
                        if (ErrorsFound2) {
                            ShowContinueError(state, format("...occurs in {} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                            ErrorsFoundGeneric = true;
                        }
                    }
                } else if (Util::SameString(desicDehum.RegenCoilType, "Coil:Heating:Steam")) {
                    desicDehum.RegenCoilType_Num = HVAC::Coil_HeatingSteam;
                    ValidateComponent(state, RegenCoilType, RegenCoilName, IsNotOK, CurrentModuleObject);
                    if (IsNotOK) {
                        ShowContinueError(state, format("...occurs in {} = {}", CurrentModuleObject, Alphas(1)));
                        ErrorsFound = true;
                    } else { // mine data from the regeneration heating coil object
                        if (desicDehum.RegenSetPointTemp <= 0.0) {
                            ShowSevereError(state, format("{} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                            ShowContinueError(state, format("{} must be greater than 0.", cNumericFields(1)));
                            ErrorsFoundGeneric = true;
                        }

                        errFlag = false;
                        desicDehum.RegenCoilIndex = SteamCoils::GetSteamCoilIndex(state, "COIL:HEATING:STEAM", RegenCoilName, errFlag);
                        if (desicDehum.RegenCoilIndex == 0) {
                            ShowSevereError(state, format("{} illegal {} = {}", CurrentModuleObject, cAlphaFields(9), RegenCoilName));
                            ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, desicDehum.Name));
                            ErrorsFound = true;
                        }

                        // Get the regeneration Heating Coil steam inlet node number
                        errFlag = false;
                        desicDehum.CoilControlNode = SteamCoils::GetCoilSteamInletNode(state, "Coil:Heating:Steam", RegenCoilName, errFlag);
                        if (errFlag) {
                            ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, desicDehum.Name));
                            ErrorsFound = true;
                        }

                        // Get the regeneration heating Coil steam max volume flow rate
                        desicDehum.MaxCoilFluidFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, desicDehum.RegenCoilIndex, errFlag);
                        if (desicDehum.MaxCoilFluidFlow > 0.0) {
                            Real64 SteamDensity = Fluid::GetSteam(state)->getSatDensity(state, TempSteamIn, 1.0, dehumidifierDesiccantNoFans);
                            desicDehum.MaxCoilFluidFlow *= SteamDensity;
                        }

                        // Get the regeneration heating Coil Inlet Node
                        errFlag = false;
                        int RegenCoilAirInletNode = SteamCoils::GetCoilAirInletNode(state, desicDehum.RegenCoilIndex, RegenCoilName, errFlag);
                        desicDehum.RegenCoilInletNode = RegenCoilAirInletNode;
                        if (errFlag) {
                            ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, desicDehum.Name));
                            ErrorsFound = true;
                        }

                        // Get the regeneration heating Coil Outlet Node
                        errFlag = false;
                        int RegenCoilAirOutletNode = SteamCoils::GetCoilAirOutletNode(state, desicDehum.RegenCoilIndex, RegenCoilName, errFlag);
                        desicDehum.RegenCoilOutletNode = RegenCoilAirOutletNode;
                        if (errFlag) {
                            ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, desicDehum.Name));
                            ErrorsFound = true;
                        }
                    }

                    ErrorsFound2 = false;
                    RegenCoilControlNodeNum = SteamCoils::GetSteamCoilControlNodeNum(state, RegenCoilType, RegenCoilName, ErrorsFound2);

                    if (ErrorsFound2) {
                        ShowContinueError(state, format("...occurs in {} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                        ErrorsFoundGeneric = true;
                    }

                    if (RegenCoilControlNodeNum > 0) {
                        ShowSevereError(state, format("{} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                        ShowContinueError(state,
                                          format("{} is specified as {:.3R} C in this object.", cNumericFields(1), desicDehum.RegenSetPointTemp));
                        ShowContinueError(state, " Do not specify a coil temperature setpoint node name in the regeneration air heater object.");
                        ShowContinueError(state, format("...{} = {}", cAlphaFields(9), desicDehum.RegenCoilType));
                        ShowContinueError(state, format("...{} = {}", cAlphaFields(10), desicDehum.RegenCoilName));
                        ShowContinueError(
                            state, format("...heating coil temperature setpoint node = {}", state.dataLoopNodes->NodeID(RegenCoilControlNodeNum)));
                        ShowContinueError(state, "...leave the heating coil temperature setpoint node name blank in the regen heater object.");
                        ErrorsFoundGeneric = true;
                    }

                    RegairHeatingCoilFlag = true;
                    SteamCoils::SetSteamCoilData(state, desicDehum.RegenCoilIndex, ErrorsFound2, RegairHeatingCoilFlag, DesicDehumNum);
                    if (ErrorsFound2) {
                        ShowContinueError(state, format("...occurs in {} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                        ErrorsFoundGeneric = true;
                    }

                } else {
                    ShowSevereError(state, format("{} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                    ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(9), desicDehum.RegenCoilType));
                    ErrorsFoundGeneric = true;
                }
            }

            std::string RegenAirInlet = state.dataLoopNodes->NodeID(desicDehum.HXRegenInNode);

            std::string RegenAirOutlet = state.dataLoopNodes->NodeID(desicDehum.HXRegenOutNode);

            std::string RegenFanInlet = state.dataLoopNodes->NodeID(desicDehum.RegenFanInNode);

            std::string RegenFanOutlet = state.dataLoopNodes->NodeID(desicDehum.RegenFanOutNode);

            if (!lAlphaBlanks(10)) {
                RegenCoilInlet = state.dataLoopNodes->NodeID(desicDehum.RegenCoilInletNode);

                RegenCoilOutlet = state.dataLoopNodes->NodeID(desicDehum.RegenCoilOutletNode);
            }

            BranchNodeConnections::SetUpCompSets(
                state, desicDehum.DehumType, desicDehum.Name, desicDehum.HXType, desicDehum.HXName, ProcAirInlet, ProcAirOutlet);

            BranchNodeConnections::SetUpCompSets(state,
                                                 desicDehum.DehumType,
                                                 desicDehum.Name,
                                                 HVAC::fanTypeNamesUC[(int)desicDehum.regenFanType],
                                                 desicDehum.RegenFanName,
                                                 RegenFanInlet,
                                                 RegenFanOutlet);

            if (!lAlphaBlanks(10)) {
                BranchNodeConnections::SetUpCompSets(state,
                                                     desicDehum.DehumType,
                                                     desicDehum.Name,
                                                     desicDehum.RegenCoilType,
                                                     desicDehum.RegenCoilName,
                                                     RegenCoilInlet,
                                                     RegenCoilOutlet);
            }

            if (desicDehum.regenFanPlace == HVAC::FanPlace::BlowThru) {
                desicDehum.RegenAirInNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                RegenFanInlet,
                                                                                ErrorsFound,
                                                                                DataLoopNode::ConnectionObjectType::DehumidifierDesiccantSystem,
                                                                                desicDehum.Name,
                                                                                DataLoopNode::NodeFluidType::Air,
                                                                                DataLoopNode::ConnectionType::Inlet,
                                                                                NodeInputManager::CompFluidStream::Primary,
                                                                                DataLoopNode::ObjectIsParent);
                desicDehum.RegenAirOutNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                 RegenAirOutlet,
                                                                                 ErrorsFound,
                                                                                 DataLoopNode::ConnectionObjectType::DehumidifierDesiccantSystem,
                                                                                 desicDehum.Name,
                                                                                 DataLoopNode::NodeFluidType::Air,
                                                                                 DataLoopNode::ConnectionType::Outlet,
                                                                                 NodeInputManager::CompFluidStream::Primary,
                                                                                 DataLoopNode::ObjectIsParent);
                if (!lAlphaBlanks(10)) {
                    if (desicDehum.RegenFanOutNode != desicDehum.RegenCoilInletNode) {
                        ShowSevereError(state, format("{} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                        ShowContinueError(state,
                                          "Regen fan outlet node name and regen heater inlet node name do not match for fan placement: Blow Through");
                        ShowContinueError(state, format("...Regen fan outlet node   = {}", state.dataLoopNodes->NodeID(desicDehum.RegenFanOutNode)));
                        ShowContinueError(state,
                                          format("...Regen heater inlet node = {}", state.dataLoopNodes->NodeID(desicDehum.RegenCoilInletNode)));
                        ErrorsFoundGeneric = true;
                    }
                    if (desicDehum.RegenCoilOutletNode != desicDehum.HXRegenInNode) {
                        ShowSevereError(state, format("{} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                        ShowContinueError(state,
                                          "Regen heater outlet node name and desiccant heat exchanger regen inlet node name do not match for fan "
                                          "placement: Blow Through");
                        ShowContinueError(state,
                                          format("...Regen heater outlet node = {}", state.dataLoopNodes->NodeID(desicDehum.RegenCoilOutletNode)));
                        ShowContinueError(state, format("...HX regen inlet node      = {}", state.dataLoopNodes->NodeID(desicDehum.HXRegenInNode)));
                        ErrorsFoundGeneric = true;
                    }
                } else {
                    if (desicDehum.RegenFanOutNode != desicDehum.HXRegenInNode) {
                        ShowSevereError(state, format("{} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                        ShowContinueError(
                            state,
                            "Regen fan outlet node name and desiccant heat exchanger inlet node name do not match for fan placement: Blow Through");
                        ShowContinueError(state, format("...Regen fan outlet node   = {}", state.dataLoopNodes->NodeID(desicDehum.RegenFanOutNode)));
                        ShowContinueError(state, format("...Desiccant HX inlet node = {}", state.dataLoopNodes->NodeID(desicDehum.HXRegenInNode)));
                        ErrorsFoundGeneric = true;
                    }
                }
            } else { // ELSE for IF (desicDehum%RegenFanPlacement == HVAC::BlowThru)THEN
                desicDehum.RegenAirOutNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                 RegenFanOutlet,
                                                                                 ErrorsFound,
                                                                                 DataLoopNode::ConnectionObjectType::DehumidifierDesiccantSystem,
                                                                                 desicDehum.Name,
                                                                                 DataLoopNode::NodeFluidType::Air,
                                                                                 DataLoopNode::ConnectionType::Outlet,
                                                                                 NodeInputManager::CompFluidStream::Primary,
                                                                                 DataLoopNode::ObjectIsParent);
                if (!lAlphaBlanks(10)) {
                    desicDehum.RegenAirInNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                    RegenCoilInlet,
                                                                                    ErrorsFound,
                                                                                    DataLoopNode::ConnectionObjectType::DehumidifierDesiccantSystem,
                                                                                    desicDehum.Name,
                                                                                    DataLoopNode::NodeFluidType::Air,
                                                                                    DataLoopNode::ConnectionType::Inlet,
                                                                                    NodeInputManager::CompFluidStream::Primary,
                                                                                    DataLoopNode::ObjectIsParent);
                    if (desicDehum.RegenCoilOutletNode != desicDehum.HXRegenInNode) {
                        ShowSevereError(state, format("{} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                        ShowContinueError(state,
                                          "Regen heater outlet node name and desiccant heat exchanger regen inlet node name do not match for fan "
                                          "placement: Draw Through");
                        ShowContinueError(state,
                                          format("...Regen heater outlet node = {}", state.dataLoopNodes->NodeID(desicDehum.RegenCoilOutletNode)));
                        ShowContinueError(state, format("...HX regen inlet node      = {}", state.dataLoopNodes->NodeID(desicDehum.HXRegenInNode)));
                        ErrorsFoundGeneric = true;
                    }
                } else {
                    desicDehum.RegenAirInNode = NodeInputManager::GetOnlySingleNode(state,
                                                                                    RegenAirInlet,
                                                                                    ErrorsFound,
                                                                                    DataLoopNode::ConnectionObjectType::DehumidifierDesiccantSystem,
                                                                                    desicDehum.Name,
                                                                                    DataLoopNode::NodeFluidType::Air,
                                                                                    DataLoopNode::ConnectionType::Inlet,
                                                                                    NodeInputManager::CompFluidStream::Primary,
                                                                                    DataLoopNode::ObjectIsParent);
                }
                if (desicDehum.RegenFanInNode != desicDehum.HXRegenOutNode) {
                    ShowSevereError(state, format("{} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                    ShowContinueError(
                        state,
                        "Regen fan inlet node name and desiccant heat exchanger regen outlet node name do not match for fan placement: Draw Through");
                    ShowContinueError(state, format("...Regen fan inlet node = {}", state.dataLoopNodes->NodeID(desicDehum.RegenFanInNode)));
                    ShowContinueError(state, format("...HX regen outlet node = {}", state.dataLoopNodes->NodeID(desicDehum.HXRegenOutNode)));
                    ErrorsFoundGeneric = true;
                }
            }

            desicDehum.CoolingCoilType = Alphas(11);
            desicDehum.CoolingCoilName = Alphas(12);

            if (!lAlphaBlanks(12)) {
                if ((Util::SameString(desicDehum.CoolingCoilType, "COIL:COOLING:DX:SINGLESPEED")) ||
                    (Util::SameString(desicDehum.CoolingCoilType, "COIL:COOLING:DX:TWOSTAGEWITHHUMIDITYCONTROLMODE")) ||
                    (Util::SameString(desicDehum.CoolingCoilType, "COIL:COOLING:DX:VARIABLESPEED"))) {
                    ErrorsFound2 = false;
                    ValidateComponent(state,
                                      desicDehum.CoolingCoilType,
                                      desicDehum.CoolingCoilName,
                                      ErrorsFound2,
                                      desicDehum.DehumType + " \"" + desicDehum.Name + "\"");
                    if (ErrorsFound2) {
                        ErrorsFoundGeneric = true;
                    }

                    if ((Util::SameString(desicDehum.CoolingCoilType, "COIL:COOLING:DX:SINGLESPEED"))) {
                        desicDehum.coolingCoil_TypeNum = HVAC::CoilDX_CoolingSingleSpeed;
                    } else if ((Util::SameString(desicDehum.CoolingCoilType, "COIL:COOLING:DX:TWOSTAGEWITHHUMIDITYCONTROLMODE"))) {
                        desicDehum.coolingCoil_TypeNum = HVAC::CoilDX_CoolingTwoStageWHumControl;
                    } else if ((Util::SameString(desicDehum.CoolingCoilType, "COIL:COOLING:DX:VARIABLESPEED"))) {
                        desicDehum.coolingCoil_TypeNum = HVAC::Coil_CoolingAirToAirVariableSpeed;
                    }

                } else {
                    ShowSevereError(state, format("{}={}", desicDehum.DehumType, desicDehum.Name));
                    ShowContinueError(state, format("Illegal {} = {}", cAlphaFields(11), desicDehum.CoolingCoilType));
                    ErrorsFoundGeneric = true;
                }

                if ((desicDehum.coolingCoil_TypeNum == HVAC::CoilDX_CoolingSingleSpeed) ||
                    (desicDehum.coolingCoil_TypeNum == HVAC::CoilDX_CoolingTwoStageWHumControl)) {
                    ErrorsFound2 = false;
                    desicDehum.CoolingCoilOutletNode =
                        DXCoils::GetCoilOutletNode(state, desicDehum.CoolingCoilType, desicDehum.CoolingCoilName, ErrorsFound2);
                    desicDehum.CompanionCoilCapacity =
                        DXCoils::GetCoilCapacity(state, desicDehum.CoolingCoilType, desicDehum.CoolingCoilName, ErrorsFound2);
                    if (ErrorsFound2) {
                        ShowContinueError(state, format("...occurs in {} \"{}\"", desicDehum.DehumType, desicDehum.CoolingCoilName));
                    }

                    ErrorsFound2 = false;
                    DXCoils::GetDXCoilIndex(state, desicDehum.CoolingCoilName, desicDehum.DXCoilIndex, ErrorsFound2, desicDehum.CoolingCoilType);
                    if (ErrorsFound2) {
                        ShowContinueError(state, format("...occurs in {} \"{}\"", desicDehum.DehumType, desicDehum.CoolingCoilName));
                    }
                } else if (desicDehum.coolingCoil_TypeNum == HVAC::Coil_CoolingAirToAirVariableSpeed) {
                    ErrorsFound2 = false;
                    desicDehum.CoolingCoilOutletNode = VariableSpeedCoils::GetCoilOutletNodeVariableSpeed(
                        state, desicDehum.CoolingCoilType, desicDehum.CoolingCoilName, ErrorsFound2);
                    ErrorsFound2 = false;
                    desicDehum.CompanionCoilCapacity =
                        VariableSpeedCoils::GetCoilCapacityVariableSpeed(state, desicDehum.CoolingCoilType, desicDehum.CoolingCoilName, ErrorsFound2);
                    if (ErrorsFound2) {
                        ShowContinueError(state, format("...occurs in {} \"{}\"", desicDehum.DehumType, desicDehum.CoolingCoilName));
                    }
                    ErrorsFound2 = false;
                    desicDehum.DXCoilIndex =
                        VariableSpeedCoils::GetCoilIndexVariableSpeed(state, desicDehum.CoolingCoilType, desicDehum.CoolingCoilName, ErrorsFound2);
                    if (ErrorsFound2) {
                        ShowContinueError(state, format("...occurs in {} \"{}\"", desicDehum.DehumType, desicDehum.CoolingCoilName));
                    }
                }

            } //  (desicDehum%CoolingCoilName /= Blank)THEN

            if (Util::SameString(Alphas(13), "Yes")) {
                desicDehum.CoilUpstreamOfProcessSide = Selection::Yes;
            } else if (lAlphaBlanks(13)) {
                desicDehum.CoilUpstreamOfProcessSide = Selection::No;
            } else if (Util::SameString(Alphas(13), "No")) {
                desicDehum.CoilUpstreamOfProcessSide = Selection::No;
            } else {
                ShowWarningError(state, format("{} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                ShowContinueError(state, format("Invalid choice for {} = {}", cAlphaFields(13), Alphas(13)));
                ShowContinueError(state, "...resetting to the default value of No");
                desicDehum.CoilUpstreamOfProcessSide = Selection::No;
            }

            if (Util::SameString(Alphas(14), "Yes")) {
                desicDehum.Preheat = Selection::Yes;
            } else if (Util::SameString(Alphas(14), "No")) {
                desicDehum.Preheat = Selection::No;
            } else if (lAlphaBlanks(14)) {
                desicDehum.Preheat = Selection::No;
            } else {
                ShowWarningError(state, format("{} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                ShowContinueError(state, format("Invalid choice for {} = {}", cAlphaFields(14), Alphas(14)));
                ShowContinueError(state, "...resetting to the default value of NO");
                desicDehum.Preheat = Selection::No;
            }

            if (desicDehum.DXCoilIndex > 0) {

                if (desicDehum.Preheat == Selection::Yes) { // Companion coil waste heat used for regeneration of desiccant
                    ErrorsFound2 = false;
                    DesuperHeaterIndex =
                        HeatingCoils::GetHeatReclaimSourceIndex(state, desicDehum.CoolingCoilType, desicDehum.CoolingCoilName, ErrorsFound2);
                    if (ErrorsFound2) {
                        ShowContinueError(state, format("...occurs in {} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                        ErrorsFoundGeneric = true;
                    }

                    if (DesuperHeaterIndex > 0) {
                        ShowWarningError(state, format("{}={}", desicDehum.DehumType, desicDehum.Name));
                        ShowContinueError(state,
                                          "A Coil:Heating:Desuperheater object should not be used when condenser waste heat is reclaimed for "
                                          "desiccant regeneration.");
                        ShowContinueError(state,
                                          format("A Coil:Heating:Desuperheater object was found using waste heat from the {} \"{}\" object.",
                                                 desicDehum.CoolingCoilType,
                                                 desicDehum.CoolingCoilName));
                        //          ErrorsFoundGeneric = .TRUE.
                    }
                }
                if ((desicDehum.coolingCoil_TypeNum == HVAC::CoilDX_CoolingSingleSpeed) ||
                    (desicDehum.coolingCoil_TypeNum == HVAC::CoilDX_CoolingTwoStageWHumControl)) {
                    ErrorsFound2 = false;
                    desicDehum.CondenserInletNode =
                        DXCoils::GetCoilCondenserInletNode(state, desicDehum.CoolingCoilType, desicDehum.CoolingCoilName, ErrorsFound2);
                } else if (desicDehum.coolingCoil_TypeNum == HVAC::Coil_CoolingAirToAirVariableSpeed) {
                    ErrorsFound2 = false;
                    desicDehum.CondenserInletNode = VariableSpeedCoils::GetVSCoilCondenserInletNode(state, desicDehum.CoolingCoilName, ErrorsFound2);
                }
                if (desicDehum.CondenserInletNode == 0 && desicDehum.Preheat == Selection::Yes) {
                    desicDehum.CondenserInletNode =
                        NodeInputManager::GetOnlySingleNode(state,
                                                            desicDehum.CoolingCoilName + " Condenser Inlet Node",
                                                            ErrorsFound,
                                                            DataLoopNode::ConnectionObjectType::DehumidifierDesiccantSystem,
                                                            desicDehum.Name,
                                                            DataLoopNode::NodeFluidType::Air,
                                                            DataLoopNode::ConnectionType::OutsideAirReference,
                                                            NodeInputManager::CompFluidStream::Secondary,
                                                            DataLoopNode::ObjectIsNotParent);
                    OutAirNodeManager::CheckAndAddAirNodeNumber(state, desicDehum.CondenserInletNode, OANodeError);
                    if (!OANodeError) {
                        ShowWarningError(state, format("{} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                        ShowContinueError(state,
                                          format("The {} input is specified as Yes and a condenser air inlet node name was not specified for the "
                                                 "companion cooling coil.",
                                                 cAlphaFields(14)));
                        ShowContinueError(
                            state, format("Adding condenser inlet air node for {} \"{}\"", desicDehum.CoolingCoilType, desicDehum.CoolingCoilName));
                        ShowContinueError(
                            state, format("...condenser inlet air node name = {}", state.dataLoopNodes->NodeID(desicDehum.CondenserInletNode)));
                        ShowContinueError(state, "...this node name will be specified as an outdoor air node.");
                    }
                } else if (desicDehum.Preheat == Selection::Yes) {
                    if (!OutAirNodeManager::CheckOutAirNodeNumber(state, desicDehum.CondenserInletNode)) {
                        ShowSevereError(state, format("{} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                        ShowContinueError(
                            state,
                            format("The regeneration air inlet node must be specified as an outdoor air node when {} is specified as Yes.",
                                   cAlphaFields(14)));
                        ErrorsFoundGeneric = true;
                    }
                }
            }

            if (OutAirNodeManager::CheckOutAirNodeNumber(state, desicDehum.RegenAirInNode)) {
                desicDehum.RegenInletIsOutsideAirNode = true;
            }

            if (desicDehum.DXCoilIndex == 0 && desicDehum.Preheat == Selection::Yes) {
                ShowWarningError(state, format("{}={}", desicDehum.DehumType, desicDehum.Name));
                ShowContinueError(
                    state, format("A valid {} must be used when condenser waste heat is reclaimed for desiccant regeneration.", cAlphaFields(12)));
                ShowContinueError(state, format("... {} = {}", cAlphaFields(11), desicDehum.CoolingCoilType));
                ShowContinueError(state, format("... {} = {}", cAlphaFields(12), desicDehum.CoolingCoilName));
                ErrorsFoundGeneric = true;
            }

            if (desicDehum.DXCoilIndex > 0 && desicDehum.CoilUpstreamOfProcessSide == Selection::Yes) {
                if ((desicDehum.coolingCoil_TypeNum == HVAC::CoilDX_CoolingSingleSpeed) ||
                    (desicDehum.coolingCoil_TypeNum == HVAC::CoilDX_CoolingTwoStageWHumControl)) {
                    ErrorsFound2 = false;
                    CoilBypassedFlowFrac =
                        DXCoils::GetDXCoilBypassedFlowFrac(state, desicDehum.CoolingCoilType, desicDehum.CoolingCoilName, ErrorsFound2);
                } else if (desicDehum.coolingCoil_TypeNum == HVAC::Coil_CoolingAirToAirVariableSpeed) {
                    ErrorsFound2 = false;
                    CoilBypassedFlowFrac = 0.0; // bypass flow fraction not in VS coil model
                }
                if (ErrorsFound2) {
                    ShowContinueError(state, format("...occurs in {} \"{}\"", desicDehum.DehumType, desicDehum.CoolingCoilName));
                }
                if (CoilBypassedFlowFrac > 0.0) {
                    ShowWarningError(state, format("{}={}", desicDehum.DehumType, desicDehum.Name));
                    ShowContinueError(
                        state,
                        format("A DX coil bypassed air flow fraction greater than 0 may not be used when the input for {} is specified as Yes.",
                               cAlphaFields(13)));
                    ShowContinueError(state,
                                      format("A DX coil with a bypassed air flow fraction greater than 0 may be upstream of the process inlet "
                                             "however the input for {} must be specified as No.",
                                             cAlphaFields(13)));
                    ShowContinueError(state, format("... {} = {}", cAlphaFields(11), desicDehum.CoolingCoilType));
                    ShowContinueError(state, format("... {} = {}", cAlphaFields(12), desicDehum.CoolingCoilName));
                    ErrorsFoundGeneric = true;
                }
            } else if (desicDehum.DXCoilIndex == 0 && desicDehum.CoilUpstreamOfProcessSide == Selection::Yes) {
                ShowSevereError(state, format("{} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                ShowContinueError(state, format("A valid companion coil must be specified when {} is specified as Yes.", cAlphaFields(13)));
                ErrorsFoundGeneric = true;
            }

            if (!desicDehum.RegenInletIsOutsideAirNode && desicDehum.Preheat == Selection::Yes) {
                ShowWarningError(state, format("{}={}", desicDehum.DehumType, desicDehum.Name));
                ShowContinueError(
                    state,
                    format("The desiccant dehumidifier regeneration air inlet must be specified as an outdoor air node when {} is specified as Yes.",
                           cAlphaFields(14)));
                ShowContinueError(state,
                                  format("... desiccant dehumidifier regeneration air inlet node name = {}",
                                         state.dataLoopNodes->NodeID(desicDehum.RegenAirInNode)));
                ErrorsFoundGeneric = true;
            }

            if (desicDehum.CoilUpstreamOfProcessSide == Selection::Yes) {
                if (desicDehum.ProcAirInNode != desicDehum.CoolingCoilOutletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                    ShowContinueError(state, "Node names are inconsistent in companion cooling coil and desiccant heat exchanger objects.");
                    ShowContinueError(state,
                                      format("For companion cooling coil = {} \"{}\"", desicDehum.CoolingCoilType, desicDehum.CoolingCoilName));
                    ShowContinueError(
                        state, format("The outlet node name in cooling coil = {}", state.dataLoopNodes->NodeID(desicDehum.CoolingCoilOutletNode)));
                    ShowContinueError(state, format("For desiccant heat exchanger = {} \"{}\"", desicDehum.HXType, desicDehum.HXName));
                    ShowContinueError(state, format("The process air inlet node name = {}", state.dataLoopNodes->NodeID(desicDehum.ProcAirInNode)));
                    ShowFatalError(state, "...previous error causes program termination.");
                }
            }

            // Exhaust Fan input
            desicDehum.ExhaustFanMaxVolFlowRate = Numbers(2);
            desicDehum.ExhaustFanMaxPower = Numbers(3);
            desicDehum.ExhaustFanCurveIndex = Curve::GetCurveIndex(state, Alphas(15));

            if (desicDehum.ExhaustFanCurveIndex > 0) {
                ErrorsFoundGeneric |= EnergyPlus::Curve::CheckCurveDims(state,
                                                                        desicDehum.ExhaustFanCurveIndex, // Curve index
                                                                        {1},                             // Valid dimensions
                                                                        RoutineName,                     // Routine name
                                                                        CurrentModuleObject,             // Object Type
                                                                        desicDehum.Name,                 // Object Name
                                                                        cAlphaFields(15));               // Field Name
            }

            if (desicDehum.Preheat == Selection::Yes) {
                ErrorsFound2 = false;
                if (desicDehum.ExhaustFanMaxVolFlowRate <= 0) {
                    ErrorsFound2 = true;
                }
                if (desicDehum.ExhaustFanMaxPower <= 0) {
                    ErrorsFound2 = true;
                }
                if (ErrorsFound2) {
                    ShowSevereError(state, format("{} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                    ShowContinueError(
                        state, format("{} and {} must be defined if {} field is \"Yes\".", cNumericFields(2), cNumericFields(3), cAlphaFields(14)));
                }
            } else if (desicDehum.Preheat == Selection::No) {
                if (desicDehum.ExhaustFanMaxVolFlowRate > 0.0) {
                    ShowWarningError(state, format("{} \"{}\"", desicDehum.DehumType, desicDehum.Name));
                    ShowContinueError(state, format("{} should be 0 if {} field is \"No\".", cNumericFields(2), cAlphaFields(14)));
                    ShowContinueError(state, format("...{} will not be used and is reset to 0.", cNumericFields(2)));
                    desicDehum.ExhaustFanMaxVolFlowRate = 0.0;
                }
            }
        }

        // SET UP OUTPUTS
        for (int DesicDehumNum = 1; DesicDehumNum <= state.dataDesiccantDehumidifiers->NumSolidDesicDehums; ++DesicDehumNum) {
            auto &desicDehum = state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum);
            // Setup Report variables for the Desiccant Dehumidifiers
            SetupOutputVariable(state,
                                "Dehumidifier Removed Water Mass",
                                Constant::Units::kg,
                                desicDehum.WaterRemove,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                desicDehum.Name);
            SetupOutputVariable(state,
                                "Dehumidifier Removed Water Mass Flow Rate",
                                Constant::Units::kg_s,
                                desicDehum.WaterRemoveRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                desicDehum.Name);
            SetupOutputVariable(state,
                                "Dehumidifier Part Load Ratio",
                                Constant::Units::None,
                                desicDehum.PartLoad,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                desicDehum.Name);
            SetupOutputVariable(state,
                                "Dehumidifier Electricity Rate",
                                Constant::Units::W,
                                desicDehum.ElecUseRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                desicDehum.Name);
            SetupOutputVariable(state,
                                "Dehumidifier Electricity Energy",
                                Constant::Units::J,
                                desicDehum.ElecUseEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                desicDehum.Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Cooling);
            SetupOutputVariable(state,
                                "Dehumidifier Regeneration Specific Energy",
                                Constant::Units::J_kgWater,
                                desicDehum.SpecRegenEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                desicDehum.Name);
            SetupOutputVariable(state,
                                "Dehumidifier Regeneration Rate",
                                Constant::Units::W,
                                desicDehum.QRegen,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                desicDehum.Name);
            SetupOutputVariable(state,
                                "Dehumidifier Regeneration Energy",
                                Constant::Units::J,
                                desicDehum.RegenEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                desicDehum.Name);
            SetupOutputVariable(state,
                                "Dehumidifier Regeneration Air Speed",
                                Constant::Units::m_s,
                                desicDehum.RegenAirVel,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                desicDehum.Name);
            SetupOutputVariable(state,
                                "Dehumidifier Regeneration Air Mass Flow Rate",
                                Constant::Units::kg_s,
                                desicDehum.RegenAirInMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                desicDehum.Name);
            SetupOutputVariable(state,
                                "Dehumidifier Process Air Mass Flow Rate",
                                Constant::Units::kg_s,
                                desicDehum.ProcAirInMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                desicDehum.Name);
        }

        for (int DesicDehumIndex = 1; DesicDehumIndex <= state.dataDesiccantDehumidifiers->NumGenericDesicDehums; ++DesicDehumIndex) {
            // this is wrong, should be a loop from (state.dataDesiccantDehumidifiers->NumSolidDesicDehums + 1) to
            // (state.dataDesiccantDehumidifiers->NumDesicDehums = NumSolidDesicDehums + NumGenericDesicDehums)
            // DesicDehumNum = DesicDehumIndex + state.dataDesiccantDehumidifiers->NumSolidDesicDehums;
            auto &desicDehum = state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumIndex);
            // Setup Report variables for the Desiccant Dehumidifiers
            SetupOutputVariable(state,
                                "Dehumidifier Removed Water Mass",
                                Constant::Units::kg,
                                desicDehum.WaterRemove,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                desicDehum.Name);
            SetupOutputVariable(state,
                                "Dehumidifier Removed Water Mass Flow Rate",
                                Constant::Units::kg_s,
                                desicDehum.WaterRemoveRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                desicDehum.Name);
            SetupOutputVariable(state,
                                "Dehumidifier Part Load Ratio",
                                Constant::Units::None,
                                desicDehum.PartLoad,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                desicDehum.Name);
            if (desicDehum.ExhaustFanMaxVolFlowRate > 0) {
                SetupOutputVariable(state,
                                    "Dehumidifier Exhaust Fan Electricity Rate",
                                    Constant::Units::W,
                                    desicDehum.ExhaustFanPower,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    desicDehum.Name);
                SetupOutputVariable(state,
                                    "Dehumidifier Exhaust Fan Electricity Energy",
                                    Constant::Units::J,
                                    desicDehum.ExhaustFanElecConsumption,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    desicDehum.Name,
                                    Constant::eResource::Electricity,
                                    OutputProcessor::Group::HVAC,
                                    OutputProcessor::EndUseCat::Cooling);
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state, "Errors found in getting Dehumidifier:Desiccant:NoFans input");
        } else if (ErrorsFoundGeneric) {
            ShowFatalError(state, "Errors found in getting DESICCANT DEHUMIDIFIER input");
        }

        Alphas.deallocate();
        cAlphaFields.deallocate();
        cNumericFields.deallocate();
        Numbers.deallocate();
        lAlphaBlanks.deallocate();
        lNumericBlanks.deallocate();
    }

    void InitDesiccantDehumidifier(EnergyPlusData &state,
                                   int const DesicDehumNum,      // number of the current dehumidifier being simulated
                                   bool const FirstHVACIteration // TRUE if 1st HVAC simulation of system timestep
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Michael J. Witte, GARD Analytics, Inc.
        //                      for Gas Research Institute
        //       DATE WRITTEN   March 2001
        //       MODIFIED       Jan 2005 M. J. Witte, GARD Analytics, Inc.
        //                        Add setpoint validation for new control type option:
        //                          NODE LEAVING HUMRAT SETPOINT:BYPASS
        //                        Work supported by ASHRAE research project 1254-RP
        //                      June 2007 R. Raustad, FSEC
        //                        Added new dehumidifier type -- DESICCANT DEHUMIDIFIER
        //                      May 2009, B. Griffith, NREL. added EMS node setpoint checks

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the dehumidifier Components.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("InitDesiccantDehumidifier");
        static std::string const initCBVAV("InitCBVAV");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        // bool ErrorsFound(false);   // Set to true if errors in input, fatal at end of routine
        Real64 QCoilActual; // actual CBVAV steam heating coil load met (W)
        bool ErrorFlag;     // local error flag returned from data mining
        bool DoSetPointTest = state.dataHVACGlobal->DoSetPointTest;

        auto &desicDehum = state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum);

        if (state.dataDesiccantDehumidifiers->InitDesiccantDehumidifierOneTimeFlag) {

            // initialize the environment and sizing flags
            state.dataDesiccantDehumidifiers->MyEnvrnFlag.dimension(state.dataDesiccantDehumidifiers->NumDesicDehums, true);
            state.dataDesiccantDehumidifiers->MyPlantScanFlag.dimension(state.dataDesiccantDehumidifiers->NumDesicDehums, true);

            state.dataDesiccantDehumidifiers->InitDesiccantDehumidifierOneTimeFlag = false;
        }

        if (state.dataDesiccantDehumidifiers->MyPlantScanFlag(DesicDehumNum) && allocated(state.dataPlnt->PlantLoop)) {
            if ((desicDehum.RegenCoilType_Num == HVAC::Coil_HeatingWater) || (desicDehum.RegenCoilType_Num == HVAC::Coil_HeatingSteam)) {
                if (desicDehum.RegenCoilType_Num == HVAC::Coil_HeatingWater) {
                    ErrorFlag = false;
                    PlantUtilities::ScanPlantLoopsForObject(state,
                                                            desicDehum.RegenCoilName,
                                                            DataPlant::PlantEquipmentType::CoilWaterSimpleHeating,
                                                            desicDehum.plantLoc,
                                                            ErrorFlag,
                                                            _,
                                                            _,
                                                            _,
                                                            _,
                                                            _);
                    if (ErrorFlag) {
                        ShowFatalError(state, "InitDesiccantDehumidifier: Program terminated for previous conditions.");
                    }

                    ErrorFlag = false;
                    desicDehum.MaxCoilFluidFlow =
                        WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", desicDehum.RegenCoilName, ErrorFlag);
                    if (desicDehum.MaxCoilFluidFlow > 0.0) {
                        Real64 FluidDensity =
                            state.dataPlnt->PlantLoop(desicDehum.plantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, initCBVAV);
                        desicDehum.MaxCoilFluidFlow *= FluidDensity;
                    }

                } else if (desicDehum.RegenCoilType_Num == HVAC::Coil_HeatingSteam) {

                    ErrorFlag = false;
                    PlantUtilities::ScanPlantLoopsForObject(state,
                                                            desicDehum.RegenCoilName,
                                                            DataPlant::PlantEquipmentType::CoilSteamAirHeating,
                                                            desicDehum.plantLoc,
                                                            ErrorFlag,
                                                            _,
                                                            _,
                                                            _,
                                                            _,
                                                            _);

                    if (ErrorFlag) {
                        ShowFatalError(state, "InitDesiccantDehumidifier: Program terminated for previous conditions.");
                    }
                    ErrorFlag = false;
                    desicDehum.MaxCoilFluidFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, desicDehum.RegenCoilIndex, ErrorFlag);

                    if (desicDehum.MaxCoilFluidFlow > 0.0) {
                        Real64 FluidDensity = Fluid::GetSteam(state)->getSatDensity(state, TempSteamIn, 1.0, RoutineName);
                        desicDehum.MaxCoilFluidFlow *= FluidDensity;
                    }
                }

                // fill outlet node for regenartion hot water or steam heating coil
                desicDehum.CoilOutletNode = DataPlant::CompData::getPlantComponent(state, desicDehum.plantLoc).NodeNumOut;
                state.dataDesiccantDehumidifiers->MyPlantScanFlag(DesicDehumNum) = false;

            } else { // DesicDehum is not connected to plant
                state.dataDesiccantDehumidifiers->MyPlantScanFlag(DesicDehumNum) = false;
            }
        } else if (state.dataDesiccantDehumidifiers->MyPlantScanFlag(DesicDehumNum) && !state.dataGlobal->AnyPlantInModel) {
            state.dataDesiccantDehumidifiers->MyPlantScanFlag(DesicDehumNum) = false;
        }

        switch (desicDehum.DehumTypeCode) {
        case DesicDehumType::Solid: {
            if (!state.dataGlobal->SysSizingCalc && state.dataDesiccantDehumidifiers->MySetPointCheckFlag && DoSetPointTest) {
                if (desicDehum.controlType == DesicDehumCtrlType::NodeHumratBypass) {
                    int ControlNode = desicDehum.ProcAirOutNode;
                    if (ControlNode > 0) {
                        if (state.dataLoopNodes->Node(ControlNode).HumRatMax == DataLoopNode::SensedNodeFlagValue) {
                            if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                                ShowSevereError(state, "Missing humidity ratio setpoint (HumRatMax) for ");
                                ShowContinueError(state, format("Dehumidifier:Desiccant:NoFans: {}", desicDehum.Name));
                                ShowContinueError(state, format("Node Referenced={}", state.dataLoopNodes->NodeID(ControlNode)));
                                ShowContinueError(state, "use a Setpoint Manager to establish a setpoint at the process air outlet node.");
                                state.dataHVACGlobal->SetPointErrorFlag = true;
                            } else {
                                EMSManager::CheckIfNodeSetPointManagedByEMS(
                                    state, ControlNode, HVAC::CtrlVarType::MaxHumRat, state.dataHVACGlobal->SetPointErrorFlag);
                                if (state.dataHVACGlobal->SetPointErrorFlag) {
                                    ShowSevereError(state, "Missing humidity ratio setpoint (HumRatMax) for ");
                                    ShowContinueError(state, format("Dehumidifier:Desiccant:NoFans: {}", desicDehum.Name));
                                    ShowContinueError(state, format("Node Referenced={}", state.dataLoopNodes->NodeID(ControlNode)));
                                    ShowContinueError(state, "use a Setpoint Manager to establish a setpoint at the process air outlet node.");
                                    ShowContinueError(state, "Or use EMS Actuator to establish a setpoint at the process air outlet node.");
                                }
                            }
                        }
                    }
                }
                state.dataDesiccantDehumidifiers->MySetPointCheckFlag = false;
            }
            // always do these initializations every iteration
            int ProcInNode = desicDehum.ProcAirInNode;
            desicDehum.ProcAirInTemp = state.dataLoopNodes->Node(ProcInNode).Temp;
            desicDehum.ProcAirInHumRat = state.dataLoopNodes->Node(ProcInNode).HumRat;
            desicDehum.ProcAirInEnthalpy = state.dataLoopNodes->Node(ProcInNode).Enthalpy;
            desicDehum.ProcAirInMassFlowRate = state.dataLoopNodes->Node(ProcInNode).MassFlowRate;

            //  Determine heating coil inlet conditions by calling it with zero load
            //  Not sure if this is really a good way to do this, should revisit for next release.
            CalcNonDXHeatingCoils(state, DesicDehumNum, FirstHVACIteration, 0.0);

            int RegenInNode = desicDehum.RegenAirInNode;
            desicDehum.RegenAirInTemp = state.dataLoopNodes->Node(RegenInNode).Temp;
            desicDehum.RegenAirInHumRat = state.dataLoopNodes->Node(RegenInNode).HumRat;
            desicDehum.RegenAirInEnthalpy = state.dataLoopNodes->Node(RegenInNode).Enthalpy;

            desicDehum.WaterRemove = 0.0;
            desicDehum.ElecUseEnergy = 0.0;
            desicDehum.ElecUseRate = 0.0;

        } break;
        case DesicDehumType::Generic: {
            //      Do the Begin Environment initializations
            if (state.dataGlobal->BeginEnvrnFlag && state.dataDesiccantDehumidifiers->MyEnvrnFlag(DesicDehumNum)) {
                // Change the Volume Flow Rates to Mass Flow Rates
                desicDehum.ExhaustFanMaxMassFlowRate = desicDehum.ExhaustFanMaxVolFlowRate * state.dataEnvrn->StdRhoAir;

                //   set fluid-side hardware limits
                if (desicDehum.CoilControlNode > 0) {
                    //    If water coil max water flow rate is autosized, simulate once in order to mine max water flow rate
                    if (desicDehum.MaxCoilFluidFlow == DataSizing::AutoSize) {
                        if (desicDehum.RegenCoilType_Num == HVAC::Coil_HeatingWater) {
                            WaterCoils::SimulateWaterCoilComponents(state, desicDehum.RegenCoilName, FirstHVACIteration, desicDehum.RegenCoilIndex);
                            ErrorFlag = false;
                            Real64 CoilMaxVolFlowRate =
                                WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", desicDehum.RegenCoilName, ErrorFlag);
                            // if (ErrorFlag) {
                            //    ErrorsFound = true;
                            //}
                            if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                                Real64 FluidDensity = state.dataPlnt->PlantLoop(desicDehum.plantLoc.loopNum)
                                                          .glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                                desicDehum.MaxCoilFluidFlow = CoilMaxVolFlowRate * FluidDensity;
                            }
                        }
                        if (desicDehum.RegenCoilType_Num == HVAC::Coil_HeatingSteam) {
                            SteamCoils::SimulateSteamCoilComponents(state,
                                                                    desicDehum.RegenCoilName,
                                                                    FirstHVACIteration,
                                                                    desicDehum.RegenCoilIndex,
                                                                    1.0,
                                                                    QCoilActual); // simulate any load > 0 to get max capacity of steam coil
                            ErrorFlag = false;
                            Real64 CoilMaxVolFlowRate = SteamCoils::GetCoilMaxSteamFlowRate(state, desicDehum.RegenCoilIndex, ErrorFlag);
                            // if (ErrorFlag) {
                            //    ErrorsFound = true;
                            //}
                            if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                                Real64 FluidDensity = Fluid::GetSteam(state)->getSatDensity(state, TempSteamIn, 1.0, RoutineName);
                                desicDehum.MaxCoilFluidFlow = CoilMaxVolFlowRate * FluidDensity;
                            }
                        }
                    }
                    PlantUtilities::InitComponentNodes(
                        state, 0.0, desicDehum.MaxCoilFluidFlow, desicDehum.CoilControlNode, desicDehum.CoilOutletNode);
                }

                state.dataDesiccantDehumidifiers->MyEnvrnFlag(DesicDehumNum) = false;
            }

            if (!state.dataGlobal->SysSizingCalc && state.dataDesiccantDehumidifiers->MySetPointCheckFlag && DoSetPointTest) {
                int ControlNode = desicDehum.ControlNodeNum;
                if (ControlNode > 0) {
                    if (state.dataLoopNodes->Node(ControlNode).HumRatMax == DataLoopNode::SensedNodeFlagValue) {
                        if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                            ShowSevereError(state, "Missing maximum humidity ratio setpoint (MaxHumRat) for ");
                            ShowContinueError(state, format("{}: {}", desicDehum.DehumType, desicDehum.Name));
                            ShowContinueError(state, format("Node Referenced={}", state.dataLoopNodes->NodeID(ControlNode)));
                            ShowContinueError(state, "use a Setpoint Manager to establish a \"MaxHumRat\" setpoint at the process air control node.");
                            state.dataHVACGlobal->SetPointErrorFlag = true;
                        } else {
                            EMSManager::CheckIfNodeSetPointManagedByEMS(
                                state, ControlNode, HVAC::CtrlVarType::MaxHumRat, state.dataHVACGlobal->SetPointErrorFlag);
                            if (state.dataHVACGlobal->SetPointErrorFlag) {
                                ShowSevereError(state, "Missing maximum humidity ratio setpoint (MaxHumRat) for ");
                                ShowContinueError(state, format("{}: {}", desicDehum.DehumType, desicDehum.Name));
                                ShowContinueError(state, format("Node Referenced={}", state.dataLoopNodes->NodeID(ControlNode)));
                                ShowContinueError(state,
                                                  "use a Setpoint Manager to establish a \"MaxHumRat\" setpoint at the process air control node.");
                                ShowContinueError(state, "Or use EMS Actuator to establish a setpoint at the process air outlet node.");
                            }
                        }
                    }
                }
                state.dataDesiccantDehumidifiers->MySetPointCheckFlag = false;
            }
            int RegenInNode = desicDehum.RegenAirInNode;
            desicDehum.RegenAirInTemp = state.dataLoopNodes->Node(RegenInNode).Temp;
            desicDehum.RegenAirInMassFlowRate = state.dataLoopNodes->Node(RegenInNode).MassFlowRate;

            desicDehum.ExhaustFanPower = 0.0;
            desicDehum.WaterRemoveRate = 0.0;
        } break;
        default:
            break;
        }
    }

    void ControlDesiccantDehumidifier(EnergyPlusData &state,
                                      int const DesicDehumNum, // number of the current dehumidifier being simulated
                                      Real64 &HumRatNeeded,    // process air leaving humidity ratio set by controller [kg water/kg air]
                                      [[maybe_unused]] bool const FirstHVACIteration // TRUE if 1st HVAC simulation of system timestep !unused1208
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Michael J. Witte, GARD Analytics, Inc.
        //                      for Gas Research Institute
        //       DATE WRITTEN   March 2001
        //       MODIFIED       Jan 2005 M. J. Witte, GARD Analytics, Inc.
        //                        Add new control type option:
        //                          NODE LEAVING HUMRAT SETPOINT:BYPASS
        //                        Change existing control type to:
        //                          FIXED LEAVING HUMRAT SETPOINT:BYPASS
        //                        Work supported by ASHRAE research project 1254-RP
        //                      June 2007 R. Raustad, FSEC
        //                        Added new dehumidifier type -- DESICCANT DEHUMIDIFIER

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine sets the output required from the dehumidifier

        // METHODOLOGY EMPLOYED:
        // Uses a maximum humidity ratio setpoint to calculate required process
        // leaving humidity ratio

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 ProcAirMassFlowRate;  // process air mass flow rate [kg/s]
        Real64 RegenAirMassFlowRate; // regen air mass flow rate [kg/s]

        auto &desicDehum = state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum);

        ProcAirMassFlowRate = 0.0;
        RegenAirMassFlowRate = 0.0;
        bool UnitOn = true;

        switch (desicDehum.DehumTypeCode) {
        case DesicDehumType::Solid: {
            if (desicDehum.HumRatSet <= 0.0) {
                UnitOn = false;
            }
            ProcAirMassFlowRate = desicDehum.ProcAirInMassFlowRate;
            if (ProcAirMassFlowRate <= HVAC::SmallMassFlow) {
                UnitOn = false;
            }

            if (desicDehum.availSched->getCurrentVal() <= 0.0) {
                UnitOn = false;
            }

            // If incoming conditions are outside valid range for curve fits, then shut unit off, do not issue warnings

            if (UnitOn) {
                if ((desicDehum.ProcAirInTemp < desicDehum.MinProcAirInTemp) || (desicDehum.ProcAirInTemp > desicDehum.MaxProcAirInTemp)) {
                    UnitOn = false;
                }
                if ((desicDehum.ProcAirInHumRat < desicDehum.MinProcAirInHumRat) || (desicDehum.ProcAirInHumRat > desicDehum.MaxProcAirInHumRat)) {
                    UnitOn = false;
                }
            }

            if (UnitOn) {

                // perform the correct dehumidifier control strategy
                switch (desicDehum.controlType) {
                case DesicDehumCtrlType::FixedHumratBypass: {
                    HumRatNeeded = desicDehum.HumRatSet;
                    if (HumRatNeeded <= 0.0) {
                        ShowSevereError(state, format("Dehumidifier:Desiccant:NoFans: {}", desicDehum.Name));
                        ShowContinueError(state, format("Invalid Leaving Max Humidity Ratio Setpoint={:.8T}", HumRatNeeded));
                        ShowFatalError(state, "must be > 0.0");
                    }
                } break;
                case DesicDehumCtrlType::NodeHumratBypass: {
                    HumRatNeeded = state.dataLoopNodes->Node(desicDehum.ProcAirOutNode).HumRatMax;
                } break;
                default: {
                    ShowFatalError(state, format("Invalid control type in desiccant dehumidifier = {}", desicDehum.Name));
                } break;
                }

                // Setpoint of zero indicates no load from setpoint manager max hum
                if ((HumRatNeeded == 0.0) || (desicDehum.ProcAirInHumRat <= HumRatNeeded)) {
                    HumRatNeeded = desicDehum.ProcAirInHumRat;
                }
            } else {
                HumRatNeeded = desicDehum.ProcAirInHumRat;
            }

        } break;
        case DesicDehumType::Generic: {
            ProcAirMassFlowRate = state.dataLoopNodes->Node(desicDehum.ProcAirInNode).MassFlowRate;
            if (ProcAirMassFlowRate <= HVAC::SmallMassFlow) {
                UnitOn = false;
            }

            if (desicDehum.availSched->getCurrentVal() <= 0.0) {
                UnitOn = false;
            }

            if (UnitOn) {
                if (desicDehum.ControlNodeNum == desicDehum.ProcAirOutNode) {
                    HumRatNeeded = state.dataLoopNodes->Node(desicDehum.ControlNodeNum).HumRatMax;
                } else {
                    if (state.dataLoopNodes->Node(desicDehum.ControlNodeNum).HumRatMax > 0.0) {
                        HumRatNeeded = state.dataLoopNodes->Node(desicDehum.ControlNodeNum).HumRatMax -
                                       (state.dataLoopNodes->Node(desicDehum.ControlNodeNum).HumRat -
                                        state.dataLoopNodes->Node(desicDehum.ProcAirOutNode).HumRat);
                    } else {
                        HumRatNeeded = 0.0;
                    }
                }

                // Setpoint of zero indicates no load from setpoint manager max hum
                if ((HumRatNeeded == 0.0) || (state.dataLoopNodes->Node(desicDehum.ProcAirInNode).HumRat <= HumRatNeeded)) {
                    HumRatNeeded = state.dataLoopNodes->Node(desicDehum.ProcAirInNode).HumRat;
                }
            } else {
                HumRatNeeded = state.dataLoopNodes->Node(desicDehum.ProcAirInNode).HumRat;
            }

        } break;
        default:
            break;
        }
    }

    void CalcSolidDesiccantDehumidifier(EnergyPlusData &state,
                                        int const DesicDehumNum,      // number of the current dehumidifier being simulated
                                        Real64 const HumRatNeeded,    // process air leaving humidity ratio set by controller [kgWater/kgDryAir]
                                        bool const FirstHVACIteration // TRUE if 1st HVAC simulation of system timestep
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Michael J. Witte, GARD Analytics, Inc.
        //                      for Gas Research Institute
        //       DATE WRITTEN   March 2001

        // PURPOSE OF THIS SUBROUTINE:
        // Calculate the electricity consumption, regen heat requirements and the outlet
        // conditions for a solid desiccant dehumidifier, given the inlet conditions and
        // and the needed process leaving humidity ratio.

        // METHODOLOGY EMPLOYED:
        // Given the entering conditions, the full-load outlet conditions are calculated.
        // Adjust for part-load if required.
        // Caclulate required regen energy and call regen coil and regen fan.
        // Desiccant wheel leaving conditions and regen energy requirements are calculated
        // from empirical curve fits.  The user can select either default built-in
        // performance curves, or use custom user-defined curves.

        // REFERENCES:
        // The default performance curves represent a commerical-grade solid desiccant
        // wheel typical of HVAC applications in the early 1990's.  These curves were
        // developed for Gas Research Institute by William W. Worek, University of Illinois
        // at Chicago.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:

        Real64 ProcAirOutHumRat; // process outlet air humidity ratio [kgWater/kgDryAir]
        Real64 ProcAirOutTemp;   // process outlet air temperature [C]
        Real64 QRegen;           // regen heat input rate requested from regen coil [W]
        Real64 QDelivered;       // regen heat actually delivered by regen coil [W]
        // REAL(r64) :: RegenAirInHumRat        ! regen inlet air humidity ratio [kgWater/kgDryAir]
        Real64 RegenAirVel;          // regen air velocity [m/s]
        Real64 RegenAirMassFlowRate; // regen air mass flow rate [kg/s]
        Real64 SpecRegenEnergy;      // specific regen energy [J/kg of water removed]
        Real64 ElecUseRate;          // electricity consumption rate [W]

        // Variables for hardwired coefficients for default performance model

        Real64 TC0;
        Real64 TC1;
        Real64 TC2;
        Real64 TC3;
        Real64 TC4;
        Real64 TC5;
        Real64 TC6;
        Real64 TC7;
        Real64 TC8;
        Real64 TC9;
        Real64 TC10;
        Real64 TC11;
        Real64 TC12;
        Real64 TC13;
        Real64 TC14;
        Real64 TC15;

        Real64 WC0;
        Real64 WC1;
        Real64 WC2;
        Real64 WC3;
        Real64 WC4;
        Real64 WC5;
        Real64 WC6;
        Real64 WC7;
        Real64 WC8;
        Real64 WC9;
        Real64 WC10;
        Real64 WC11;
        Real64 WC12;
        Real64 WC13;
        Real64 WC14;
        Real64 WC15;

        Real64 QC0;
        Real64 QC1;
        Real64 QC2;
        Real64 QC3;
        Real64 QC4;
        Real64 QC5;
        Real64 QC6;
        Real64 QC7;
        Real64 QC8;
        Real64 QC9;
        Real64 QC10;
        Real64 QC11;
        Real64 QC12;
        Real64 QC13;
        Real64 QC14;
        Real64 QC15;

        Real64 RC0;
        Real64 RC1;
        Real64 RC2;
        Real64 RC3;
        Real64 RC4;
        Real64 RC5;
        Real64 RC6;
        Real64 RC7;
        Real64 RC8;
        Real64 RC9;
        Real64 RC10;
        Real64 RC11;
        Real64 RC12;
        Real64 RC13;
        Real64 RC14;
        Real64 RC15;

        auto &desicDehum = state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum);

        // Setup internal variables for calculations

        Real64 ProcAirInTemp = desicDehum.ProcAirInTemp;
        Real64 ProcAirInHumRat = desicDehum.ProcAirInHumRat;
        Real64 ProcAirMassFlowRate = desicDehum.ProcAirInMassFlowRate;
        Real64 ProcAirVel = desicDehum.NomProcAirVel;
        Real64 PartLoad = 0.0;

        Real64 RegenAirInTemp = desicDehum.RegenAirInTemp;
        Real64 NomRegenTemp = desicDehum.NomRegenTemp;

        // Calculate min available process out humrat
        bool UnitOn = false;
        Real64 MinProcAirOutHumRat = 0.0; // MAX(MinProcAirOutHumRat,0.000857)

        if (HumRatNeeded < ProcAirInHumRat) {

            UnitOn = true;

            switch (desicDehum.PerformanceModel_Num) { // Performance Model Part A
            case PerformanceModel::Default: {
                WC0 = 0.0148880824323806;
                WC1 = -0.000283393198398211;
                WC2 = -0.87802168940547;
                WC3 = -0.000713615831236411;
                WC4 = 0.0311261188874622;
                WC5 = 1.51738892142485e-06;
                WC6 = 0.0287250198281021;
                WC7 = 4.94796903231558e-06;
                WC8 = 24.0771139652826;
                WC9 = 0.000122270283927978;
                WC10 = -0.0151657189566474;
                WC11 = 3.91641393230322e-08;
                WC12 = 0.126032651553348;
                WC13 = 0.000391653854431574;
                WC14 = 0.002160537360507;
                WC15 = 0.00132732844211593;

                MinProcAirOutHumRat = WC0 + WC1 * ProcAirInTemp + WC2 * ProcAirInHumRat + WC3 * ProcAirVel + WC4 * ProcAirInTemp * ProcAirInHumRat +
                                      WC5 * ProcAirInTemp * ProcAirVel + WC6 * ProcAirInHumRat * ProcAirVel + WC7 * ProcAirInTemp * ProcAirInTemp +
                                      WC8 * ProcAirInHumRat * ProcAirInHumRat + WC9 * ProcAirVel * ProcAirVel +
                                      WC10 * ProcAirInTemp * ProcAirInTemp * ProcAirInHumRat * ProcAirInHumRat +
                                      WC11 * ProcAirInTemp * ProcAirInTemp * ProcAirVel * ProcAirVel +
                                      WC12 * ProcAirInHumRat * ProcAirInHumRat * ProcAirVel * ProcAirVel + WC13 * std::log(ProcAirInTemp) +
                                      WC14 * std::log(ProcAirInHumRat) + WC15 * std::log(ProcAirVel);

                // limit to 6 grains/lb (0.000857 kg/kg)

            } break;
            case PerformanceModel::UserCurves: {
                MinProcAirOutHumRat = Curve::CurveValue(state, desicDehum.ProcHumRatCurvefTW, ProcAirInTemp, ProcAirInHumRat) *
                                      Curve::CurveValue(state, desicDehum.ProcHumRatCurvefV, ProcAirVel);
            } break;

            default: {

                ShowFatalError(state, format("Invalid performance model in desiccant dehumidifier = {}", desicDehum.PerformanceModel_Num));
            } break;
            } // Performance Model Part A

            MinProcAirOutHumRat = max(MinProcAirOutHumRat, 0.000857);
        }

        if (MinProcAirOutHumRat >= ProcAirInHumRat) {
            UnitOn = false;
        }

        if (UnitOn) {

            // Calculate partload fraction of dehumidification capacity required to meet setpoint
            PartLoad = 1.0;
            if (MinProcAirOutHumRat < HumRatNeeded) {
                PartLoad = (ProcAirInHumRat - HumRatNeeded) / (ProcAirInHumRat - MinProcAirOutHumRat);
            }
            PartLoad = max(0.0, PartLoad);
            PartLoad = min(1.0, PartLoad);

            switch (desicDehum.PerformanceModel_Num) { // Performance Model Part B
            case PerformanceModel::Default: {
                // Calculate leaving conditions
                TC0 = -38.7782841989449;
                TC1 = 2.0127655837628;
                TC2 = 5212.49360216097;
                TC3 = 15.2362536782665;
                TC4 = -80.4910419759181;
                TC5 = -0.105014122001509;
                TC6 = -229.668673645144;
                TC7 = -0.015424703743461;
                TC8 = -69440.0689831847;
                TC9 = -1.6686064694322;
                TC10 = 38.5855718977592;
                TC11 = 0.000196395381206009;
                TC12 = 386.179386548324;
                TC13 = -0.801959614172614;
                TC14 = -3.33080986818745;
                TC15 = -15.2034386065714;

                ProcAirOutTemp = TC0 + TC1 * ProcAirInTemp + TC2 * ProcAirInHumRat + TC3 * ProcAirVel + TC4 * ProcAirInTemp * ProcAirInHumRat +
                                 TC5 * ProcAirInTemp * ProcAirVel + TC6 * ProcAirInHumRat * ProcAirVel + TC7 * ProcAirInTemp * ProcAirInTemp +
                                 TC8 * ProcAirInHumRat * ProcAirInHumRat + TC9 * ProcAirVel * ProcAirVel +
                                 TC10 * ProcAirInTemp * ProcAirInTemp * ProcAirInHumRat * ProcAirInHumRat +
                                 TC11 * ProcAirInTemp * ProcAirInTemp * ProcAirVel * ProcAirVel +
                                 TC12 * ProcAirInHumRat * ProcAirInHumRat * ProcAirVel * ProcAirVel + TC13 * std::log(ProcAirInTemp) +
                                 TC14 * std::log(ProcAirInHumRat) + TC15 * std::log(ProcAirVel);

                // Regen energy
                QC0 = -27794046.6291107;
                QC1 = -235725.171759615;
                QC2 = 975461343.331328;
                QC3 = -686069.373946731;
                QC4 = -17717307.3766266;
                QC5 = 31482.2539662489;
                QC6 = 55296552.8260743;
                QC7 = 6195.36070023868;
                QC8 = -8304781359.40435;
                QC9 = -188987.543809419;
                QC10 = 3933449.40965846;
                QC11 = -6.66122876558634;
                QC12 = -349102295.417547;
                QC13 = 83672.179730172;
                QC14 = -6059524.33170538;
                QC15 = 1220523.39525162;

                SpecRegenEnergy = QC0 + QC1 * ProcAirInTemp + QC2 * ProcAirInHumRat + QC3 * ProcAirVel + QC4 * ProcAirInTemp * ProcAirInHumRat +
                                  QC5 * ProcAirInTemp * ProcAirVel + QC6 * ProcAirInHumRat * ProcAirVel + QC7 * ProcAirInTemp * ProcAirInTemp +
                                  QC8 * ProcAirInHumRat * ProcAirInHumRat + QC9 * ProcAirVel * ProcAirVel +
                                  QC10 * ProcAirInTemp * ProcAirInTemp * ProcAirInHumRat * ProcAirInHumRat +
                                  QC11 * ProcAirInTemp * ProcAirInTemp * ProcAirVel * ProcAirVel +
                                  QC12 * ProcAirInHumRat * ProcAirInHumRat * ProcAirVel * ProcAirVel + QC13 * std::log(ProcAirInTemp) +
                                  QC14 * std::log(ProcAirInHumRat) + QC15 * std::log(ProcAirVel);

                // Regen face velocity
                RC0 = -4.67358908091488;
                RC1 = 0.0654323095468338;
                RC2 = 396.950518702316;
                RC3 = 1.52610165426736;
                RC4 = -11.3955868430328;
                RC5 = 0.00520693906104437;
                RC6 = 57.783645385621;
                RC7 = -0.000464800668311693;
                RC8 = -5958.78613212602;
                RC9 = -0.205375818291012;
                RC10 = 5.26762675442845;
                RC11 = -8.88452553055039e-05;
                RC12 = -182.382479369311;
                RC13 = -0.100289774002047;
                RC14 = -0.486980507964251;
                RC15 = -0.972715425435447;

                RegenAirVel = RC0 + RC1 * ProcAirInTemp + RC2 * ProcAirInHumRat + RC3 * ProcAirVel + RC4 * ProcAirInTemp * ProcAirInHumRat +
                              RC5 * ProcAirInTemp * ProcAirVel + RC6 * ProcAirInHumRat * ProcAirVel + RC7 * ProcAirInTemp * ProcAirInTemp +
                              RC8 * ProcAirInHumRat * ProcAirInHumRat + RC9 * ProcAirVel * ProcAirVel +
                              RC10 * ProcAirInTemp * ProcAirInTemp * ProcAirInHumRat * ProcAirInHumRat +
                              RC11 * ProcAirInTemp * ProcAirInTemp * ProcAirVel * ProcAirVel +
                              RC12 * ProcAirInHumRat * ProcAirInHumRat * ProcAirVel * ProcAirVel + RC13 * std::log(ProcAirInTemp) +
                              RC14 * std::log(ProcAirInHumRat) + RC15 * std::log(ProcAirVel);

            } break;
            case PerformanceModel::UserCurves: {

                ProcAirOutTemp = Curve::CurveValue(state, desicDehum.ProcDryBulbCurvefTW, ProcAirInTemp, ProcAirInHumRat) *
                                 Curve::CurveValue(state, desicDehum.ProcDryBulbCurvefV, ProcAirVel);

                SpecRegenEnergy = Curve::CurveValue(state, desicDehum.RegenEnergyCurvefTW, ProcAirInTemp, ProcAirInHumRat) *
                                  Curve::CurveValue(state, desicDehum.RegenEnergyCurvefV, ProcAirVel);

                RegenAirVel = Curve::CurveValue(state, desicDehum.RegenVelCurvefTW, ProcAirInTemp, ProcAirInHumRat) *
                              Curve::CurveValue(state, desicDehum.RegenVelCurvefV, ProcAirVel);

            } break;
            default: {

                ShowFatalError(state, format("Invalid performance model in desiccant dehumidifier = {}", desicDehum.PerformanceModel_Num));

                // Suppress uninitialized warnings
                ProcAirOutTemp = 0.0;
                SpecRegenEnergy = 0.0;
                RegenAirVel = 0.0;
            } break;
            } // Performance Model Part B

            ProcAirOutTemp = (1 - PartLoad) * ProcAirInTemp + (PartLoad)*ProcAirOutTemp;

            ProcAirOutHumRat = (1 - PartLoad) * ProcAirInHumRat + (PartLoad)*MinProcAirOutHumRat;

            // Calculate water removal
            desicDehum.WaterRemoveRate = ProcAirMassFlowRate * (ProcAirInHumRat - ProcAirOutHumRat);

            // Adjust for regen inlet temperature
            SpecRegenEnergy *= (NomRegenTemp - RegenAirInTemp) / (NomRegenTemp - ProcAirInTemp);
            SpecRegenEnergy = max(SpecRegenEnergy, 0.0);
            QRegen = SpecRegenEnergy * desicDehum.WaterRemoveRate;

            // Above curves are based on a 90deg regen angle and 245deg process air angle
            RegenAirMassFlowRate = ProcAirMassFlowRate * 90.0 / 245.0 * RegenAirVel / ProcAirVel;

            ElecUseRate = desicDehum.NomRotorPower;

        } else { // Unit is off

            ProcAirOutTemp = ProcAirInTemp;
            ProcAirOutHumRat = ProcAirInHumRat;
            SpecRegenEnergy = 0.0;
            QRegen = 0.0;
            ElecUseRate = 0.0;
            RegenAirVel = 0.0;
            RegenAirMassFlowRate = 0.0;
            desicDehum.WaterRemoveRate = 0.0;
            PartLoad = 0.0;

        } // UnitOn/Off

        // Set regen mass flow
        state.dataLoopNodes->Node(desicDehum.RegenFanInNode).MassFlowRate = RegenAirMassFlowRate;
        state.dataLoopNodes->Node(desicDehum.RegenFanInNode).MassFlowRateMaxAvail = RegenAirMassFlowRate;
        // Call regen fan
        state.dataFans->fans(desicDehum.RegenFanIndex)->simulate(state, FirstHVACIteration, _, _);

        // Call regen heating coil
        CalcNonDXHeatingCoils(state, DesicDehumNum, FirstHVACIteration, QRegen, QDelivered);

        // Verify is requestd flow was delivered (must do after heating coil has executed to pass flow to RegenAirInNode)
        if (state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRate != RegenAirMassFlowRate) {
            // Initialize standard air density
            ShowRecurringSevereErrorAtEnd(state,
                                          "Improper flow delivered by desiccant regen fan - RESULTS INVALID! Check regen fan capacity and schedule.",
                                          desicDehum.RegenFanErrorIndex1);
            ShowRecurringContinueErrorAtEnd(state, desicDehum.DehumType + '=' + desicDehum.Name, desicDehum.RegenFanErrorIndex2);
            ShowRecurringContinueErrorAtEnd(
                state,
                format("Flow requested [m3/s] from {} = {}", HVAC::fanTypeNames[(int)desicDehum.regenFanType], desicDehum.RegenFanName),
                desicDehum.RegenFanErrorIndex3,
                (RegenAirMassFlowRate / state.dataEnvrn->StdRhoAir));
            ShowRecurringContinueErrorAtEnd(
                state,
                "Flow request varied from delivered by [m3/s]",
                desicDehum.RegenFanErrorIndex4,
                ((RegenAirMassFlowRate - state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRate) / state.dataEnvrn->StdRhoAir),
                ((RegenAirMassFlowRate - state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRate) / state.dataEnvrn->StdRhoAir));
        }

        // Verify is requestd heating was delivered
        if (QDelivered < QRegen) {
            ShowRecurringSevereErrorAtEnd(
                state,
                "Inadequate heat delivered by desiccant regen coil - RESULTS INVALID! Check regen coil capacity and schedule.",
                desicDehum.RegenCapErrorIndex1);
            ShowRecurringContinueErrorAtEnd(state, desicDehum.DehumType + '=' + desicDehum.Name, desicDehum.RegenCapErrorIndex2);
            ShowRecurringContinueErrorAtEnd(state,
                                            format("Load requested [W] from {} = {}", desicDehum.RegenCoilType, desicDehum.RegenCoilName),
                                            desicDehum.RegenCapErrorIndex3,
                                            QRegen);
            ShowRecurringContinueErrorAtEnd(state, "Load request exceeded delivered by [W]", desicDehum.RegenCapErrorIndex4, (QRegen - QDelivered));
        }

        desicDehum.SpecRegenEnergy = SpecRegenEnergy;
        desicDehum.QRegen = QRegen;
        desicDehum.ElecUseRate = ElecUseRate;
        desicDehum.PartLoad = PartLoad;

        desicDehum.ProcAirOutMassFlowRate = ProcAirMassFlowRate;
        desicDehum.ProcAirOutTemp = ProcAirOutTemp;
        desicDehum.ProcAirOutHumRat = ProcAirOutHumRat;
        desicDehum.ProcAirOutEnthalpy = Psychrometrics::PsyHFnTdbW(ProcAirOutTemp, ProcAirOutHumRat);
        desicDehum.RegenAirInMassFlowRate = RegenAirMassFlowRate;
        desicDehum.RegenAirVel = RegenAirVel;
    }

    void CalcGenericDesiccantDehumidifier(EnergyPlusData &state,
                                          int const DesicDehumNum,      // number of the current dehumidifier being simulated
                                          Real64 const HumRatNeeded,    // process air leaving humidity ratio set by controller [kg water/kg air]
                                          bool const FirstHVACIteration // TRUE if 1st HVAC simulation of system timestep
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Mangesh Basarkar, FSEC
        //       DATE WRITTEN   May 2007

        // PURPOSE OF THIS SUBROUTINE:
        // Calculate the electricity consumption, regen heat requirements and the outlet
        // conditions for a desiccant dehumidifier, given the inlet conditions,
        // DX coil part-load ratio, and/or the needed process leaving humidity ratio.

        // METHODOLOGY EMPLOYED:
        // Given the entering conditions, the full-load outlet conditions are calculated.
        // Adjust for part-load if required.
        // Calculate the required regen energy and call the regen coil and the regen fan.

        // REFERENCES:
        // Kosar, D. 2006. Dehumidification Enhancements, ASHRAE Journal, Vol. 48, No. 2, February 2006.
        // Kosar, D. et al. 2006. Dehumidification Enhancement of Direct Expansion Systems Through Component
        //   Augmentation of the Cooling Coil. 15th Symposium on Improving Building Systems in Hot and Humid
        //   Climates, July 24-26, 2006.

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr MinVolFlowPerRatedTotQ(0.00002684); // m3/s per W = 200 cfm/ton,
        // min vol flow per rated evaporator capacity

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 DDPartLoadRatio;        // fraction of dehumidification capacity required to meet setpoint
        Real64 MassFlowRateNew;        // new required mass flow rate calculated to keep regen setpoint temperature (kg/s)
        Real64 CondenserWasteHeat;     // Condenser waste heat (W)
        Real64 CpAir;                  // Specific heat of air (J/kg-K)
        Real64 NewRegenInTemp;         // new temp calculated from condenser waste heat (C)
        Real64 ExhaustFanMassFlowRate; // exhaust fan mass flow rate (kg/s)
        Real64 ExhaustFanPLR;          // exhaust fan run time fraction calculated from new mass flow rate for regen side
        Real64 ExhaustFanPowerMod;     // used to calculate exhaust fan power from flow fraction
        Real64 VolFlowPerRatedTotQ;    // flow rate per rated total cooling capacity of the companion coil (m3/s/W)
        Real64 FanDeltaT;              // used to account for fan heat when calculating regeneration heater energy (C)
        Real64 OnOffFanPLF;            // save air loop fan part load fracton while calculating exhaust fan power
        Real64 RegenSetPointTemp;      // regeneration temperature setpoint (C)
        int RegenCoilIndex;            // index to regeneration heating coil, 0 when not used
        int CompanionCoilIndexNum;     // index for companion DX cooling coil, 0 when DX coil is not used
        bool UnitOn;                   // unit on flag
        //  LOGICAL       :: SimFlag                    ! used to turn off additional simulation if DX Coil is off
        Real64 QRegen_OASysFanAdjust; // temporary variable used to adjust regen heater load during iteration

        auto &desicDehum = state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum);
        auto &QRegen(state.dataDesiccantDehumidifiers->QRegen);

        UnitOn = false;
        DDPartLoadRatio = 0.0;
        RegenCoilIndex = desicDehum.RegenCoilIndex;
        FanDeltaT = 0.0;
        RegenSetPointTemp = desicDehum.RegenSetPointTemp;
        ExhaustFanMassFlowRate = 0.0;

        // Save OnOffFanPartLoadFraction while performing exhaust fan calculations
        OnOffFanPLF = state.dataHVACGlobal->OnOffFanPartLoadFraction;
        state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;

        if (desicDehum.CoilUpstreamOfProcessSide == Selection::Yes) {
            // Cooling coil directly upstream of desiccant dehumidifier, dehumidifier runs in tandem with DX coil

            CompanionCoilIndexNum = desicDehum.DXCoilIndex;
        } else {
            // desiccant dehumidifier determines its own PLR
            CompanionCoilIndexNum = 0;
        }

        if (HumRatNeeded < state.dataLoopNodes->Node(desicDehum.ProcAirInNode).HumRat) {
            UnitOn = true;
        }

        if (desicDehum.CoilUpstreamOfProcessSide == Selection::Yes) {
            if ((desicDehum.coolingCoil_TypeNum == HVAC::CoilDX_CoolingSingleSpeed) ||
                (desicDehum.coolingCoil_TypeNum == HVAC::CoilDX_CoolingTwoStageWHumControl)) {
                if (state.dataDXCoils->DXCoilPartLoadRatio(desicDehum.DXCoilIndex) == 0.0) {
                    UnitOn = false;
                }
            }
        }

        if (UnitOn) {

            if (desicDehum.RegenInletIsOutsideAirNode) {
                if (desicDehum.HXTypeNum == BalancedHX) {
                    state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRate =
                        state.dataLoopNodes->Node(desicDehum.ProcAirInNode).MassFlowRate;
                    state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRateMaxAvail =
                        state.dataLoopNodes->Node(desicDehum.ProcAirInNode).MassFlowRate;
                }
            }

            // Get conditions from DX Coil condenser if present (DXCoilIndex verified > 0 in GetInput)
            if (desicDehum.Preheat == Selection::Yes) {

                //     condenser waste heat is proportional to DX coil PLR
                if ((desicDehum.coolingCoil_TypeNum == HVAC::CoilDX_CoolingSingleSpeed) ||
                    (desicDehum.coolingCoil_TypeNum == HVAC::CoilDX_CoolingTwoStageWHumControl)) {
                    CondenserWasteHeat = state.dataHeatBal->HeatReclaimDXCoil(desicDehum.DXCoilIndex).AvailCapacity;
                    state.dataHeatBal->HeatReclaimDXCoil(desicDehum.DXCoilIndex).AvailCapacity = 0.0;
                } else if (desicDehum.coolingCoil_TypeNum == HVAC::Coil_CoolingAirToAirVariableSpeed) {
                    CondenserWasteHeat = state.dataHeatBal->HeatReclaimVS_Coil(desicDehum.DXCoilIndex).AvailCapacity;
                    state.dataHeatBal->HeatReclaimVS_Coil(desicDehum.DXCoilIndex).AvailCapacity = 0.0;
                }

                CpAir = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(desicDehum.CondenserInletNode).HumRat);

                if (desicDehum.regenFanPlace == HVAC::FanPlace::BlowThru) {
                    state.dataFans->fans(desicDehum.RegenFanIndex)->simulate(state, FirstHVACIteration, _, _);
                    FanDeltaT =
                        state.dataLoopNodes->Node(desicDehum.RegenFanOutNode).Temp - state.dataLoopNodes->Node(desicDehum.RegenFanInNode).Temp;
                    //       Adjust setpoint to account for fan heat
                    RegenSetPointTemp -= FanDeltaT;
                }

                //     CompanionCoilIndexNum .GT. 0 means the same thing as desicDehum%CoilUpstreamOfProcessSide == Yes
                if (CompanionCoilIndexNum > 0) {

                    //     calculate PLR and actual condenser outlet node (regen inlet node) temperature
                    if ((desicDehum.coolingCoil_TypeNum == HVAC::CoilDX_CoolingSingleSpeed) ||
                        (desicDehum.coolingCoil_TypeNum == HVAC::CoilDX_CoolingTwoStageWHumControl)) {
                        DDPartLoadRatio = state.dataDXCoils->DXCoilPartLoadRatio(desicDehum.DXCoilIndex);
                        if (state.dataDXCoils->DXCoilFanOp(desicDehum.DXCoilIndex) == HVAC::FanOp::Continuous) {
                            NewRegenInTemp =
                                state.dataLoopNodes->Node(desicDehum.CondenserInletNode).Temp +
                                CondenserWasteHeat / (CpAir * (state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRate) * DDPartLoadRatio);
                            CondenserWasteHeat /= DDPartLoadRatio;
                        } else {
                            NewRegenInTemp = state.dataLoopNodes->Node(desicDehum.CondenserInletNode).Temp +
                                             CondenserWasteHeat / (CpAir * (state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRate));
                        }
                    } else if (desicDehum.coolingCoil_TypeNum == HVAC::Coil_CoolingAirToAirVariableSpeed) {
                        DDPartLoadRatio = 1.0; // condenser waste heat already includes modulation down
                        NewRegenInTemp = state.dataLoopNodes->Node(desicDehum.CondenserInletNode).Temp +
                                         CondenserWasteHeat / (CpAir * (state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRate));
                    } else {
                        NewRegenInTemp = state.dataLoopNodes->Node(desicDehum.CondenserInletNode).Temp +
                                         CondenserWasteHeat / (CpAir * (state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRate));
                    }
                } else {
                    NewRegenInTemp = state.dataLoopNodes->Node(desicDehum.CondenserInletNode).Temp +
                                     CondenserWasteHeat / (CpAir * (state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRate));
                }

                state.dataLoopNodes->Node(desicDehum.RegenAirInNode).Temp = NewRegenInTemp;
                state.dataLoopNodes->Node(desicDehum.RegenAirInNode).Enthalpy = Psychrometrics::PsyHFnTdbW(
                    state.dataLoopNodes->Node(desicDehum.RegenAirInNode).Temp, state.dataLoopNodes->Node(desicDehum.RegenAirInNode).HumRat);
                MassFlowRateNew = 0.0;

                if (desicDehum.ExhaustFanMaxVolFlowRate > 0) {

                    //       calculate mass flow rate required to maintain regen inlet setpoint temp
                    if (NewRegenInTemp > RegenSetPointTemp) {
                        if (RegenSetPointTemp - state.dataLoopNodes->Node(desicDehum.CondenserInletNode).Temp != 0.0) {
                            MassFlowRateNew = max(0.0,
                                                  CondenserWasteHeat /
                                                      (CpAir * (RegenSetPointTemp - state.dataLoopNodes->Node(desicDehum.CondenserInletNode).Temp)));
                        } else {
                            MassFlowRateNew = state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRate;
                        }
                    }

                    //       calculate exhaust fan mass flow rate and new regen inlet temperature (may not be at setpoint)
                    if (MassFlowRateNew > state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRate) {
                        ExhaustFanMassFlowRate = MassFlowRateNew - state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRate;
                        ExhaustFanMassFlowRate = max(0.0, min(ExhaustFanMassFlowRate, desicDehum.ExhaustFanMaxMassFlowRate));

                        state.dataLoopNodes->Node(desicDehum.RegenAirInNode).Temp =
                            state.dataLoopNodes->Node(desicDehum.CondenserInletNode).Temp +
                            CondenserWasteHeat /
                                (CpAir * (state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRate + ExhaustFanMassFlowRate));
                        state.dataLoopNodes->Node(desicDehum.RegenAirInNode).HumRat = state.dataLoopNodes->Node(desicDehum.CondenserInletNode).HumRat;
                        state.dataLoopNodes->Node(desicDehum.RegenAirInNode).Enthalpy = Psychrometrics::PsyHFnTdbW(
                            state.dataLoopNodes->Node(desicDehum.RegenAirInNode).Temp, state.dataLoopNodes->Node(desicDehum.RegenAirInNode).HumRat);
                    }
                }

                if (RegenCoilIndex > 0) {
                    if (NewRegenInTemp < RegenSetPointTemp) {
                        CpAir = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(desicDehum.RegenAirInNode).HumRat);
                    }
                    QRegen = max(0.0,
                                 (CpAir * state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRate *
                                  (RegenSetPointTemp - state.dataLoopNodes->Node(desicDehum.RegenAirInNode).Temp)));
                    if (QRegen == 0.0) {
                        QRegen = -1.0;
                    }
                }

                //     CompanionCoilIndexNum .EQ. 0 means the same thing as desicDehum%CoilUpstreamOfProcessSide == No
                if (CompanionCoilIndexNum == 0) {

                    if (RegenCoilIndex > 0) {

                        QRegen_OASysFanAdjust = QRegen;
                        if (desicDehum.regenFanPlace == HVAC::FanPlace::BlowThru) {
                            if (state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRate > 0.0) {
                                //             For VAV systems, fan may restrict air flow during iteration. Adjust QRegen proportional to Mdot
                                //             reduction through fan
                                QRegen_OASysFanAdjust *= state.dataLoopNodes->Node(desicDehum.RegenFanOutNode).MassFlowRate /
                                                         state.dataLoopNodes->Node(desicDehum.RegenFanInNode).MassFlowRate;
                            }
                        }

                        CalcNonDXHeatingCoils(state, DesicDehumNum, FirstHVACIteration, QRegen_OASysFanAdjust);
                    }

                    HeatRecovery::SimHeatRecovery(state,
                                                  desicDehum.HXName,
                                                  FirstHVACIteration,
                                                  desicDehum.CompIndex,
                                                  HVAC::FanOp::Continuous,
                                                  1.0,
                                                  true,
                                                  CompanionCoilIndexNum,
                                                  desicDehum.RegenInletIsOutsideAirNode,
                                                  _,
                                                  _,
                                                  desicDehum.coolingCoil_TypeNum);

                    //       calculate desiccant part-load ratio
                    if (state.dataLoopNodes->Node(desicDehum.ProcAirInNode).HumRat != state.dataLoopNodes->Node(desicDehum.ProcAirOutNode).HumRat) {
                        DDPartLoadRatio = (state.dataLoopNodes->Node(desicDehum.ProcAirInNode).HumRat - HumRatNeeded) /
                                          (state.dataLoopNodes->Node(desicDehum.ProcAirInNode).HumRat -
                                           state.dataLoopNodes->Node(desicDehum.ProcAirOutNode).HumRat);
                        DDPartLoadRatio = max(0.0, min(1.0, DDPartLoadRatio));
                    } else {
                        DDPartLoadRatio = 1.0;
                    }
                }

                if (ExhaustFanMassFlowRate > 0.0) {

                    //       calculate exhaust fan mass flow rate due to desiccant system operation
                    ExhaustFanMassFlowRate *= DDPartLoadRatio;

                    //       calculate exhaust fan PLR due to desiccant system operation
                    ExhaustFanPLR = ExhaustFanMassFlowRate / desicDehum.ExhaustFanMaxMassFlowRate;

                    //       find exhaust fan power multiplier using exhaust fan part-load ratio
                    if (desicDehum.ExhaustFanCurveIndex > 0) {
                        ExhaustFanPowerMod = min(1.0, max(0.0, Curve::CurveValue(state, desicDehum.ExhaustFanCurveIndex, ExhaustFanPLR)));
                    } else {
                        ExhaustFanPowerMod = 1.0;
                    }

                    //       calculate exhaust fan power due to desiccant operation
                    desicDehum.ExhaustFanPower = desicDehum.ExhaustFanMaxPower * ExhaustFanPowerMod;
                }

            } else { // ELSE for IF(desicDehum%Preheat == Yes)THEN

                if (state.dataLoopNodes->Node(desicDehum.ProcAirInNode).HumRat > HumRatNeeded) {

                    //       Get Full load output of desiccant wheel
                    if (desicDehum.regenFanPlace == HVAC::FanPlace::BlowThru) {
                        state.dataFans->fans(desicDehum.RegenFanIndex)->simulate(state, FirstHVACIteration, _, _);

                        FanDeltaT =
                            state.dataLoopNodes->Node(desicDehum.RegenFanOutNode).Temp - state.dataLoopNodes->Node(desicDehum.RegenFanInNode).Temp;
                        RegenSetPointTemp -= FanDeltaT;
                    }

                    if (RegenCoilIndex > 0) {
                        CpAir = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(desicDehum.RegenAirInNode).HumRat);
                        QRegen = max(0.0,
                                     (CpAir * state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRate *
                                      (RegenSetPointTemp - state.dataLoopNodes->Node(desicDehum.RegenAirInNode).Temp)));

                        QRegen_OASysFanAdjust = QRegen;
                        if (desicDehum.regenFanPlace == HVAC::FanPlace::BlowThru) {
                            if (state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRate > 0.0) {
                                //             For VAV systems, fan may restrict air flow during iteration. Adjust QRegen proportional to Mdot
                                //             reduction through fan
                                QRegen_OASysFanAdjust *= state.dataLoopNodes->Node(desicDehum.RegenFanOutNode).MassFlowRate /
                                                         state.dataLoopNodes->Node(desicDehum.RegenFanInNode).MassFlowRate;
                            }
                        }

                        if (QRegen_OASysFanAdjust == 0.0) {
                            QRegen_OASysFanAdjust = -1.0;
                        }
                        CalcNonDXHeatingCoils(state, DesicDehumNum, FirstHVACIteration, QRegen_OASysFanAdjust);
                    }

                    //       CompanionCoilIndexNum .EQ. 0 means the same thing as desicDehum%CoilUpstreamOfProcessSide == No
                    if (CompanionCoilIndexNum == 0) {
                        HeatRecovery::SimHeatRecovery(state,
                                                      desicDehum.HXName,
                                                      FirstHVACIteration,
                                                      desicDehum.CompIndex,
                                                      HVAC::FanOp::Continuous,
                                                      1.0,
                                                      true,
                                                      CompanionCoilIndexNum,
                                                      desicDehum.RegenInletIsOutsideAirNode,
                                                      _,
                                                      _,
                                                      desicDehum.coolingCoil_TypeNum);

                        //         calculate desiccant part-load ratio
                        if (state.dataLoopNodes->Node(desicDehum.ProcAirInNode).HumRat !=
                            state.dataLoopNodes->Node(desicDehum.ProcAirOutNode).HumRat) {
                            DDPartLoadRatio = (state.dataLoopNodes->Node(desicDehum.ProcAirInNode).HumRat - HumRatNeeded) /
                                              (state.dataLoopNodes->Node(desicDehum.ProcAirInNode).HumRat -
                                               state.dataLoopNodes->Node(desicDehum.ProcAirOutNode).HumRat);
                            DDPartLoadRatio = max(0.0, min(1.0, DDPartLoadRatio));
                        } else {
                            DDPartLoadRatio = 1.0;
                        }
                    } else {
                        if ((desicDehum.coolingCoil_TypeNum == HVAC::CoilDX_CoolingSingleSpeed) ||
                            (desicDehum.coolingCoil_TypeNum == HVAC::CoilDX_CoolingTwoStageWHumControl)) {
                            DDPartLoadRatio = state.dataDXCoils->DXCoilPartLoadRatio(desicDehum.DXCoilIndex);
                        } else if (desicDehum.coolingCoil_TypeNum == HVAC::Coil_CoolingAirToAirVariableSpeed) {
                            DDPartLoadRatio = 1.0; // condenser waste heat already includes modulation down
                        }
                    }
                } else { // ELSE for IF(state.dataLoopNodes->Node(desicDehum%ProcAirInNode)%HumRat .GT. HumRatNeeded)THEN
                    DDPartLoadRatio = 0.0;
                } // END IF for IF(state.dataLoopNodes->Node(desicDehum%ProcAirInNode)%HumRat .GT. HumRatNeeded)THEN

            } // END IF for IF(desicDehum%Preheat == Yes)THEN

            desicDehum.PartLoad = DDPartLoadRatio;
            QRegen_OASysFanAdjust = QRegen;

            // set average regeneration air mass flow rate based on desiccant cycling ratio (DDPartLoadRatio)
            if (desicDehum.RegenInletIsOutsideAirNode) {
                state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRate *= DDPartLoadRatio;

                // **RR moved to here, only adjust regen heater load if mass flow rate is changed
                //   adjust regen heating coil capacity based on desiccant cycling ratio (PLR)
                QRegen_OASysFanAdjust *= DDPartLoadRatio;
            }

            // Call regen fan, balanced desiccant HX and heating coil
            if (desicDehum.regenFanPlace == HVAC::FanPlace::BlowThru) {
                state.dataFans->fans(desicDehum.RegenFanIndex)->simulate(state, FirstHVACIteration, _, _);
            }

            if (RegenCoilIndex > 0) {

                //!   adjust regen heating coil capacity based on desiccant cycling ratio (PLR)
                //    QRegen_OASysFanAdjust = QRegen * DDPartLoadRatio

                if (desicDehum.regenFanPlace == HVAC::FanPlace::BlowThru) {
                    if (state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRate > 0.0) {
                        //       For VAV systems, fan may restrict air flow during iteration. Adjust QRegen proportional to Mdot reduction through fan
                        QRegen_OASysFanAdjust *= state.dataLoopNodes->Node(desicDehum.RegenFanOutNode).MassFlowRate /
                                                 state.dataLoopNodes->Node(desicDehum.RegenFanInNode).MassFlowRate;
                    }
                }

                if (QRegen_OASysFanAdjust == 0.0) {
                    QRegen_OASysFanAdjust = -1.0;
                }
                CalcNonDXHeatingCoils(state, DesicDehumNum, FirstHVACIteration, QRegen_OASysFanAdjust);
            }

            HeatRecovery::SimHeatRecovery(state,
                                          desicDehum.HXName,
                                          FirstHVACIteration,
                                          desicDehum.CompIndex,
                                          HVAC::FanOp::Continuous,
                                          DDPartLoadRatio,
                                          true,
                                          CompanionCoilIndexNum,
                                          desicDehum.RegenInletIsOutsideAirNode,
                                          _,
                                          _,
                                          desicDehum.coolingCoil_TypeNum);

            if (desicDehum.regenFanPlace == HVAC::FanPlace::DrawThru) {
                state.dataFans->fans(desicDehum.RegenFanIndex)->simulate(state, FirstHVACIteration, _, _);
            }

            // Calculate water removal
            desicDehum.WaterRemoveRate =
                state.dataLoopNodes->Node(desicDehum.ProcAirInNode).MassFlowRate *
                (state.dataLoopNodes->Node(desicDehum.ProcAirInNode).HumRat - state.dataLoopNodes->Node(desicDehum.ProcAirOutNode).HumRat);

            // If preheat is Yes, exhaust fan is condenser fan, if CoilUpstreamOfProcessSide is No, DD runs an its own PLR
            if (desicDehum.Preheat == Selection::Yes && desicDehum.CoilUpstreamOfProcessSide == Selection::No) {
                //    should actually use DX coil RTF instead of PLR since fan power is being calculated
                if ((desicDehum.coolingCoil_TypeNum == HVAC::CoilDX_CoolingSingleSpeed) ||
                    (desicDehum.coolingCoil_TypeNum == HVAC::CoilDX_CoolingTwoStageWHumControl)) {
                    desicDehum.ExhaustFanPower += max(
                        0.0, (desicDehum.ExhaustFanMaxPower * (state.dataDXCoils->DXCoilPartLoadRatio(desicDehum.DXCoilIndex) - DDPartLoadRatio)));
                } else if (desicDehum.coolingCoil_TypeNum == HVAC::Coil_CoolingAirToAirVariableSpeed) {
                    desicDehum.ExhaustFanPower += max(0.0, (desicDehum.ExhaustFanMaxPower * (1.0 - DDPartLoadRatio)));
                }
            }

        } else { // unit must be off

            desicDehum.PartLoad = 0.0;

            if (desicDehum.RegenInletIsOutsideAirNode) {
                state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRate = 0.0;
                state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRateMaxAvail = 0.0;
            }

            if (desicDehum.regenFanPlace == HVAC::FanPlace::BlowThru) {
                state.dataFans->fans(desicDehum.RegenFanIndex)->simulate(state, FirstHVACIteration, _, _);
            }

            if (RegenCoilIndex > 0) {
                CalcNonDXHeatingCoils(state, DesicDehumNum, FirstHVACIteration, -1.0);
            }

            HeatRecovery::SimHeatRecovery(state,
                                          desicDehum.HXName,
                                          FirstHVACIteration,
                                          desicDehum.CompIndex,
                                          HVAC::FanOp::Continuous,
                                          0.0,
                                          false,
                                          CompanionCoilIndexNum,
                                          desicDehum.RegenInletIsOutsideAirNode,
                                          _,
                                          _,
                                          desicDehum.coolingCoil_TypeNum);

            if (desicDehum.regenFanPlace == HVAC::FanPlace::DrawThru) {
                state.dataFans->fans(desicDehum.RegenFanIndex)->simulate(state, FirstHVACIteration, _, _);
            }

            // Turn on exhaust fan if DX Coil is operating
            if (desicDehum.ExhaustFanMaxVolFlowRate > 0) {
                if (desicDehum.DXCoilIndex > 0) {
                    if ((desicDehum.coolingCoil_TypeNum == HVAC::CoilDX_CoolingSingleSpeed) ||
                        (desicDehum.coolingCoil_TypeNum == HVAC::CoilDX_CoolingTwoStageWHumControl)) {
                        DDPartLoadRatio = state.dataDXCoils->DXCoilPartLoadRatio(desicDehum.DXCoilIndex);
                    } else if (desicDehum.coolingCoil_TypeNum == HVAC::Coil_CoolingAirToAirVariableSpeed) {
                        DDPartLoadRatio = 1.0; // condenser waste heat already includes modulation down
                    }
                    desicDehum.ExhaustFanPower = desicDehum.ExhaustFanMaxPower * DDPartLoadRatio;
                    ExhaustFanMassFlowRate = desicDehum.ExhaustFanMaxMassFlowRate * DDPartLoadRatio;
                }
            }

        } // UnitOn/Off

        // check condenser minimum flow per rated total capacity
        if (DDPartLoadRatio > 0.0 && desicDehum.ExhaustFanMaxVolFlowRate > 0.0) {
            VolFlowPerRatedTotQ = (state.dataLoopNodes->Node(desicDehum.RegenAirInNode).MassFlowRate + ExhaustFanMassFlowRate) /
                                  max(0.00001, (desicDehum.CompanionCoilCapacity * DDPartLoadRatio * state.dataEnvrn->StdRhoAir));
            if (!state.dataGlobal->WarmupFlag && (VolFlowPerRatedTotQ < MinVolFlowPerRatedTotQ)) {
                ++desicDehum.ErrCount;
                if (desicDehum.ErrCount < 2) {
                    ShowWarningError(state,
                                     format("{} \"{}\" - Air volume flow rate per watt of total condenser waste heat is below the minimum "
                                            "recommended at {:G} m3/s/W.",
                                            desicDehum.DehumType,
                                            desicDehum.Name,
                                            VolFlowPerRatedTotQ));
                    ShowContinueErrorTimeStamp(state, "");
                    ShowContinueError(state,
                                      format("Expected minimum for VolumeFlowperRatedTotalCondenserWasteHeat = [{:G}]", MinVolFlowPerRatedTotQ));
                    ShowContinueError(state, "Possible causes include inconsistent air flow rates in system components ");
                    ShowContinueError(state, "on the regeneration side of the desiccant dehumidifier.");
                } else {
                    ShowRecurringWarningErrorAtEnd(
                        state,
                        desicDehum.DehumType + " \"" + desicDehum.Name +
                            "\" - Air volume flow rate per watt of rated total cooling capacity is out of range error continues...",
                        desicDehum.ErrIndex1,
                        VolFlowPerRatedTotQ,
                        VolFlowPerRatedTotQ);
                }
            } // flow per rated total capacity check ends
        }

        // Reset OnOffFanPartLoadFraction for process side fan calculations
        state.dataHVACGlobal->OnOffFanPartLoadFraction = OnOffFanPLF;
    }

    void UpdateDesiccantDehumidifier(EnergyPlusData &state, int const DesicDehumNum) // number of the current dehumidifier being simulated
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Michael J. Witte, GARD Analytics, Inc.
        //                      for Gas Research Institute
        //       DATE WRITTEN   March 2001

        // PURPOSE OF THIS SUBROUTINE:
        // Moves dehumidifier output to the outlet nodes.

        switch (state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).DehumTypeCode) {
        case DesicDehumType::Solid: {
            int ProcInNode = state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).ProcAirInNode;
            int ProcOutNode = state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).ProcAirOutNode;
            // Set the process outlet air node of the dehumidifier
            state.dataLoopNodes->Node(ProcOutNode).Temp = state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).ProcAirOutTemp;
            state.dataLoopNodes->Node(ProcOutNode).HumRat = state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).ProcAirOutHumRat;
            state.dataLoopNodes->Node(ProcOutNode).Enthalpy = state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).ProcAirOutEnthalpy;

            // Set the process outlet nodes for properties that just pass through & not used
            state.dataLoopNodes->Node(ProcOutNode).Quality = state.dataLoopNodes->Node(ProcInNode).Quality;
            state.dataLoopNodes->Node(ProcOutNode).Press = state.dataLoopNodes->Node(ProcInNode).Press;
            state.dataLoopNodes->Node(ProcOutNode).MassFlowRate = state.dataLoopNodes->Node(ProcInNode).MassFlowRate;
            state.dataLoopNodes->Node(ProcOutNode).MassFlowRateMin = state.dataLoopNodes->Node(ProcInNode).MassFlowRateMin;
            state.dataLoopNodes->Node(ProcOutNode).MassFlowRateMax = state.dataLoopNodes->Node(ProcInNode).MassFlowRateMax;
            state.dataLoopNodes->Node(ProcOutNode).MassFlowRateMinAvail = state.dataLoopNodes->Node(ProcInNode).MassFlowRateMinAvail;
            state.dataLoopNodes->Node(ProcOutNode).MassFlowRateMaxAvail = state.dataLoopNodes->Node(ProcInNode).MassFlowRateMaxAvail;

            //   RegenInNode =DesicDehum(DesicDehumNum)%RegenAirInNode
            //   RegenOutNode = DesicDehum(DesicDehumNum)%RegenAirOutNode
            // Set the regen outlet air node of the dehumidifier
            //   Node(RegenOutNode)%Temp         = DesicDehum(DesicDehumNum)%RegenAirOutTemp
            //   Node(RegenOutNode)%HumRat       = DesicDehum(DesicDehumNum)%RegenAirOutHumRat
            //   Node(RegenOutNode)%Enthalpy     = DesicDehum(DesicDehumNum)%RegenAirOutEnthalpy

            // Set the regen outlet nodes for properties that just pass through & not used
            //   Node(RegenOutNode)%Quality             = Node(RegenInNode)%Quality
            //   Node(RegenOutNode)%Press               = Node(RegenInNode)%Press
            //   Node(RegenOutNode)%MassFlowRate        = Node(RegenInNode)%MassFlowRate
            //   Node(RegenOutNode)%MassFlowRateMin     = Node(RegenInNode)%MassFlowRateMin
            //   Node(RegenOutNode)%MassFlowRateMax     = Node(RegenInNode)%MassFlowRateMax
            //   Node(RegenOutNode)%MassFlowRateMinAvail= Node(RegenInNode)%MassFlowRateMinAvail
            //   Node(RegenOutNode)%MassFlowRateMaxAvail= Node(RegenInNode)%MassFlowRateMaxAvail

        } break;
        case DesicDehumType::Generic: {
            return;
        } break;
        default:
            break;
        }
    }

    void ReportDesiccantDehumidifier(EnergyPlusData &state, int const DesicDehumNum) // number of the current dehumidifier being simulated
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Michael J. Witte, GARD Analytics, Inc.
        //                      for Gas Research Institute
        //       DATE WRITTEN   March 2001
        //       MODIFIED       June 2007, R. Raustad, Added new dehumidifier type -- DESICCANT DEHUMIDIFIER

        // PURPOSE OF THIS SUBROUTINE:
        // Fill remaining report variables

        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

        switch (state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).DehumTypeCode) {
        case DesicDehumType::Solid: {
            state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).WaterRemove =
                state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).WaterRemoveRate * TimeStepSysSec;
            state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).RegenEnergy =
                state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).QRegen * TimeStepSysSec;
            state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).ElecUseEnergy =
                state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).ElecUseRate * TimeStepSysSec;
        } break;
        case DesicDehumType::Generic: {
            state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).WaterRemove =
                state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).WaterRemoveRate * TimeStepSysSec;
            state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).ExhaustFanElecConsumption =
                state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).ExhaustFanPower * TimeStepSysSec;
        } break;
        default:
            break;
        }
    }

    void CalcNonDXHeatingCoils(EnergyPlusData &state,
                               int const DesicDehumNum,                     // Desiccant dehumidifier unit index
                               bool const FirstHVACIteration,               // flag for first HVAC iteration in the time step
                               Real64 const RegenCoilLoad,                  // heating coil load to be met (Watts)
                               ObjexxFCL::Optional<Real64> RegenCoilLoadmet // heating load met
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Bereket Nigusse, FSEC/UCF
        //       DATE WRITTEN   January 2012

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine simulates the four non dx heating coil types: Gas, Electric, hot water and steam.

        // METHODOLOGY EMPLOYED:
        // Simply calls the different heating coil component.  The hot water flow rate matching the coil load
        // is calculated iteratively.

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr ErrTolerance(0.001); // convergence limit for hotwater coil
        int constexpr SolveMaxIter(50);       // Max iteration for SolveRoot

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 RegenCoilActual; // actual heating load met
        Real64 mdot;            // heating coil steam or hot water mass flow rate
        Real64 MinWaterFlow;    // minimum hot water mass flow rate
        // unused  REAL(r64)      :: PartLoadFraction  ! heating or cooling part load fraction
        Real64 MaxHotWaterFlow; // maximum hot water mass flow rate, kg/s
        Real64 HotWaterMdot;    // actual hot water mass flow rate

        auto &desicDehum = state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum);

        RegenCoilActual = 0.0;
        if (RegenCoilLoad > HVAC::SmallLoad) {
            switch (desicDehum.RegenCoilType_Num) {
            case HVAC::Coil_HeatingGasOrOtherFuel:
            case HVAC::Coil_HeatingElectric: {
                HeatingCoils::SimulateHeatingCoilComponents(
                    state, desicDehum.RegenCoilName, FirstHVACIteration, RegenCoilLoad, desicDehum.RegenCoilIndex, RegenCoilActual);
            } break;
            case HVAC::Coil_HeatingWater: {
                MaxHotWaterFlow = desicDehum.MaxCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(
                    state, MaxHotWaterFlow, desicDehum.CoilControlNode, desicDehum.CoilOutletNode, desicDehum.plantLoc);
                RegenCoilActual = RegenCoilLoad;
                // simulate the regenerator hot water heating coil
                WaterCoils::SimulateWaterCoilComponents(
                    state, desicDehum.RegenCoilName, FirstHVACIteration, desicDehum.RegenCoilIndex, RegenCoilActual);

                if (RegenCoilActual > (RegenCoilLoad + HVAC::SmallLoad)) {
                    // control water flow to obtain output matching RegenCoilLoad
                    int SolFlag = 0;
                    MinWaterFlow = 0.0;
                    auto f = [&state, DesicDehumNum, FirstHVACIteration, RegenCoilLoad](Real64 HWFlow) {
                        Real64 RegenCoilHeatLoad = RegenCoilLoad;
                        Real64 RegenCoilActual = RegenCoilHeatLoad;
                        Real64 mdot = HWFlow;
                        PlantUtilities::SetComponentFlowRate(state,
                                                             mdot,
                                                             state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).CoilControlNode,
                                                             state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).CoilOutletNode,
                                                             state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).plantLoc);

                        // simulate the hot water regenerator heating coil
                        WaterCoils::SimulateWaterCoilComponents(state,
                                                                state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).RegenCoilName,
                                                                FirstHVACIteration,
                                                                state.dataDesiccantDehumidifiers->DesicDehum(DesicDehumNum).RegenCoilIndex,
                                                                RegenCoilActual);
                        if (RegenCoilHeatLoad != 0.0) {
                            return (RegenCoilActual - RegenCoilHeatLoad) / RegenCoilHeatLoad;
                        } else { // Autodesk:Return ELSE added to assure return value is set
                            return 0.0;
                        }
                    };
                    General::SolveRoot(state, ErrTolerance, SolveMaxIter, SolFlag, HotWaterMdot, f, MinWaterFlow, MaxHotWaterFlow);
                    if (SolFlag == -1) {
                        if (desicDehum.HotWaterCoilMaxIterIndex == 0) {
                            ShowWarningMessage(
                                state,
                                format("CalcNonDXHeatingCoils: Hot water coil control failed for {}=\"{}\"", desicDehum.DehumType, desicDehum.Name));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state,
                                              format("...Iteration limit [{}] exceeded in calculating hot water mass flow rate", SolveMaxIter));
                        }
                        ShowRecurringWarningErrorAtEnd(
                            state,
                            format("CalcNonDXHeatingCoils: Hot water coil control failed (iteration limit [{}]) for {}=\"{}\"",
                                   SolveMaxIter,
                                   desicDehum.DehumType,
                                   desicDehum.Name),
                            desicDehum.HotWaterCoilMaxIterIndex);
                    } else if (SolFlag == -2) {
                        if (desicDehum.HotWaterCoilMaxIterIndex2 == 0) {
                            ShowWarningMessage(state,
                                               format("CalcNonDXHeatingCoils: Hot water coil control failed (maximum flow limits) for {}=\"{}\"",
                                                      desicDehum.DehumType,
                                                      desicDehum.Name));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state, "...Bad hot water maximum flow rate limits");
                            ShowContinueError(state, format("...Given minimum water flow rate={:.3R} kg/s", MinWaterFlow));
                            ShowContinueError(state, format("...Given maximum water flow rate={:.3R} kg/s", MaxHotWaterFlow));
                        }
                        ShowRecurringWarningErrorAtEnd(state,
                                                       "CalcNonDXHeatingCoils: Hot water coil control failed (flow limits) for " +
                                                           desicDehum.DehumType + "=\"" + desicDehum.Name + "\"",
                                                       desicDehum.HotWaterCoilMaxIterIndex2,
                                                       MaxHotWaterFlow,
                                                       MinWaterFlow,
                                                       _,
                                                       "[kg/s]",
                                                       "[kg/s]");
                    }

                    RegenCoilActual = RegenCoilLoad;
                    // simulate the regenerator hot water heating coil
                    WaterCoils::SimulateWaterCoilComponents(
                        state, desicDehum.RegenCoilName, FirstHVACIteration, desicDehum.RegenCoilIndex, RegenCoilActual);
                }
            } break;
            case HVAC::Coil_HeatingSteam: {
                mdot = desicDehum.MaxCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(state, mdot, desicDehum.CoilControlNode, desicDehum.CoilOutletNode, desicDehum.plantLoc);
                // simulate the regenerator steam heating coil
                SteamCoils::SimulateSteamCoilComponents(
                    state, desicDehum.RegenCoilName, FirstHVACIteration, desicDehum.RegenCoilIndex, RegenCoilLoad, RegenCoilActual);
            } break;
            default:
                break;
            }
        } else {
            switch (desicDehum.RegenCoilType_Num) {
            case HVAC::Coil_HeatingGasOrOtherFuel:
            case HVAC::Coil_HeatingElectric: {
                HeatingCoils::SimulateHeatingCoilComponents(
                    state, desicDehum.RegenCoilName, FirstHVACIteration, RegenCoilLoad, desicDehum.RegenCoilIndex, RegenCoilActual);
            } break;
            case HVAC::Coil_HeatingWater: {
                mdot = 0.0;
                PlantUtilities::SetComponentFlowRate(state, mdot, desicDehum.CoilControlNode, desicDehum.CoilOutletNode, desicDehum.plantLoc);
                RegenCoilActual = RegenCoilLoad;
                // simulate the regenerator hot water heating coil
                WaterCoils::SimulateWaterCoilComponents(
                    state, desicDehum.RegenCoilName, FirstHVACIteration, desicDehum.RegenCoilIndex, RegenCoilActual);
            } break;
            case HVAC::Coil_HeatingSteam: {
                mdot = 0.0;
                PlantUtilities::SetComponentFlowRate(state, mdot, desicDehum.CoilControlNode, desicDehum.CoilOutletNode, desicDehum.plantLoc);
                // simulate the regenerator steam heating coil
                SteamCoils::SimulateSteamCoilComponents(
                    state, desicDehum.RegenCoilName, FirstHVACIteration, desicDehum.RegenCoilIndex, RegenCoilLoad, RegenCoilActual);
            } break;
            default:
                break;
            }
        }
        if (present(RegenCoilLoadmet)) {
            RegenCoilLoadmet = RegenCoilActual;
        }
    }

    int GetProcAirInletNodeNum(EnergyPlusData &state, std::string const &DesicDehumName, bool &ErrorsFound)
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   May 2019

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given Desiccant Dehumidifier and returns the process air inlet node number.
        // If incorrect Desiccant Dehumidifier name is given, ErrorsFound is returned as true and node number as zero.

        // Obtains and Allocates heat exchanger related parameters from input file
        if (state.dataDesiccantDehumidifiers->GetInputDesiccantDehumidifier) {
            GetDesiccantDehumidifierInput(state);
            state.dataDesiccantDehumidifiers->GetInputDesiccantDehumidifier = false;
        }

        int WhichDesicDehum = Util::FindItemInList(DesicDehumName, state.dataDesiccantDehumidifiers->DesicDehum);
        if (WhichDesicDehum != 0) {
            return state.dataDesiccantDehumidifiers->DesicDehum(WhichDesicDehum).ProcAirInNode;
        } else {
            ShowSevereError(state, format("GetProcAirInletNodeNum: Could not find Desciccant Dehumidifier = \"{}\"", DesicDehumName));
            ErrorsFound = true;
            return 0;
        }
    }

    int GetProcAirOutletNodeNum(EnergyPlusData &state, std::string const &DesicDehumName, bool &ErrorsFound)
    {
        // FUNCTION INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   May 2019

        // PURPOSE OF THIS FUNCTION:
        // This function looks up the given Desiccant Dehumidifier and returns the process air outlet node number.
        // If incorrect Desiccant Dehumidifier name is given, ErrorsFound is returned as true and node number as zero.

        // Obtains and Allocates heat exchanger related parameters from input file
        if (state.dataDesiccantDehumidifiers->GetInputDesiccantDehumidifier) {
            GetDesiccantDehumidifierInput(state);
            state.dataDesiccantDehumidifiers->GetInputDesiccantDehumidifier = false;
        }

        int WhichDesicDehum = Util::FindItemInList(DesicDehumName, state.dataDesiccantDehumidifiers->DesicDehum);
        if (WhichDesicDehum != 0) {
            return state.dataDesiccantDehumidifiers->DesicDehum(WhichDesicDehum).ProcAirOutNode;
        } else {
            ShowSevereError(state, format("GetProcAirInletNodeNum: Could not find Desciccant Dehumidifier = \"{}\"", DesicDehumName));
            ErrorsFound = true;
            return 0;
        }
    }

    //        End of Reporting subroutines for the SimAir Module
    // *****************************************************************************

    //                                 COPYRIGHT NOTICE

    //     Portions Copyright (c) Gas Research Institute 2001.  All rights reserved.

    //     GRI LEGAL NOTICE
    //     Neither GRI, members of GRI nor any person or organization acting on behalf
    //     of either:

    //     A. Makes any warranty of representation, express or implied with respect to
    //        the accuracy, completness, or usefulness of the information contained in
    //        in this program, including any warranty of merchantability or fitness of
    //        any purpose with respoect to the program, or that the use of any
    //        information disclosed in this program may not infringe privately-owned
    //        rights, or

    //     B.  Assumes any liability with respoct to the use of, or for any and all
    //         damages resulting from the use of the program or any portion thereof or
    //         any information disclosed therein.

} // namespace DesiccantDehumidifiers

} // namespace EnergyPlus

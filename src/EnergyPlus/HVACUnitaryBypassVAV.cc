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

// ObjexxFCL Headers
#include <ObjexxFCL/Array.functions.hh>
#include <ObjexxFCL/Fmath.hh>

// EnergyPlus Headers
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Coils/CoilCoolingDX.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirLoop.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/DataZoneControls.hh>
#include <EnergyPlus/DataZoneEnergyDemands.hh>
#include <EnergyPlus/DataZoneEquipment.hh>
#include <EnergyPlus/EMSManager.hh>
#include <EnergyPlus/Fans.hh>
#include <EnergyPlus/FluidProperties.hh>
#include <EnergyPlus/General.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/HVACDXHeatPumpSystem.hh>
#include <EnergyPlus/HVACHXAssistedCoolingCoil.hh>
#include <EnergyPlus/HVACUnitaryBypassVAV.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/MixedAir.hh>
#include <EnergyPlus/MixerComponent.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SetPointManager.hh>
#include <EnergyPlus/SteamCoils.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/VariableSpeedCoils.hh>
#include <EnergyPlus/WaterCoils.hh>
#include <EnergyPlus/ZonePlenum.hh>

namespace EnergyPlus {

namespace HVACUnitaryBypassVAV {

    // Module containing the routines for modeling changeover-bypass VAV systems

    // MODULE INFORMATION:
    //       AUTHOR         Richard Raustad
    //       DATE WRITTEN   July 2006
    //       MODIFIED       B. Nigusse, FSEC - January 2012 - Added steam and hot water heating coils

    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and algorithms needed to simulate changeover-bypass
    // variable-air-volume (CBVAV) systems, which are considered "Air Loop Equipment" in EnergyPlus

    // METHODOLOGY EMPLOYED:
    // Units are modeled as a collection of components: outside air mixer,
    // supply air fan, DX cooling coil, DX/gas/elec heating coil, and variable volume boxes.
    // Control is accomplished by calculating the load in all zones to determine a mode of operation.
    // The system will either cool, heat, or operate based on fan mode selection.

    // The CBVAV system is initialized with no load (coils off) to determine the outlet temperature.
    // A setpoint temperature is calculated on FirstHVACIteration = TRUE to force one VAV box fully open.
    // Once the setpoint is calculated, the inlet node mass flow rate on FirstHVACIteration = FALSE is used to
    // determine the bypass fraction. The simulation converges quickly on mass flow rate. If the zone
    // temperatures float in the deadband, additional iterations are required to converge on mass flow rate.

    // REFERENCES:
    // "Temp & VVT Commercial Comfort Systems," Engineering Training Manual, Technical Development Program, Carrier Corp., 1995.
    // "VariTrac Changeover Bypass VAV (Tracker System CB)," VAV-PRC003-EN, Trane Company, June 2004.
    // "Ventilation for Changeover-Bypass VAV Systems," D. Stanke, ASHRAE Journal Vol. 46, No. 11, November 2004.
    //  Lawrence Berkeley Laboratory. Nov. 1993. DOE-2 Supplement Version 2.1E, Winklemann et.al.

    void SimUnitaryBypassVAV(EnergyPlusData &state,
                             std::string_view CompName,     // Name of the CBVAV system
                             bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system time step
                             int const AirLoopNum,          // air loop index
                             int &CompIndex                 // Index to changeover-bypass VAV system
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2006

        // PURPOSE OF THIS SUBROUTINE:
        // Manages the simulation of a changeover-bypass VAV system. Called from SimAirServingZones.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int CBVAVNum = 0;      // Index of CBVAV system being simulated
        Real64 QUnitOut = 0.0; // Sensible capacity delivered by this air loop system

        // First time SimUnitaryBypassVAV is called, get the input for all the CBVAVs
        if (state.dataHVACUnitaryBypassVAV->GetInputFlag) {
            GetCBVAV(state);
            state.dataHVACUnitaryBypassVAV->GetInputFlag = false;
        }

        // Find the correct changeover-bypass VAV unit
        if (CompIndex == 0) {
            CBVAVNum = Util::FindItemInList(CompName, state.dataHVACUnitaryBypassVAV->CBVAVs);
            if (CBVAVNum == 0) {
                ShowFatalError(state, format("SimUnitaryBypassVAV: Unit not found={}", CompName));
            }
            CompIndex = CBVAVNum;
        } else {
            CBVAVNum = CompIndex;
            if (CBVAVNum > state.dataHVACUnitaryBypassVAV->NumCBVAV || CBVAVNum < 1) {
                ShowFatalError(state,
                               format("SimUnitaryBypassVAV:  Invalid CompIndex passed={}, Number of Units={}, Entered Unit name={}",
                                      CBVAVNum,
                                      state.dataHVACUnitaryBypassVAV->NumCBVAV,
                                      CompName));
            }
            if (state.dataHVACUnitaryBypassVAV->CheckEquipName(CBVAVNum)) {
                if (CompName != state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNum).Name) {
                    ShowFatalError(state,
                                   format("SimUnitaryBypassVAV: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                          CBVAVNum,
                                          CompName,
                                          state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNum).Name));
                }
                state.dataHVACUnitaryBypassVAV->CheckEquipName(CBVAVNum) = false;
            }
        }

        Real64 OnOffAirFlowRatio = 0.0; // Ratio of compressor ON airflow to average airflow over timestep
        bool HXUnitOn = true;           // flag to enable heat exchanger

        // Initialize the changeover-bypass VAV system
        InitCBVAV(state, CBVAVNum, FirstHVACIteration, AirLoopNum, OnOffAirFlowRatio, HXUnitOn);

        // Simulate the unit
        SimCBVAV(state, CBVAVNum, FirstHVACIteration, QUnitOut, OnOffAirFlowRatio, HXUnitOn);

        // Report the result of the simulation
        ReportCBVAV(state, CBVAVNum);
    }

    void SimCBVAV(EnergyPlusData &state,
                  int const CBVAVNum,            // Index of the current CBVAV system being simulated
                  bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                  Real64 &QSensUnitOut,          // Sensible delivered capacity [W]
                  Real64 &OnOffAirFlowRatio,     // Ratio of compressor ON airflow to AVERAGE airflow over timestep
                  bool const HXUnitOn            // flag to enable heat exchanger
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2006

        // PURPOSE OF THIS SUBROUTINE:
        // Simulate a changeover-bypass VAV system.

        // METHODOLOGY EMPLOYED:
        // Calls ControlCBVAVOutput to obtain the desired unit output

        QSensUnitOut = 0.0; // probably don't need this initialization

        auto &changeOverByPassVAV = state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNum);

        // zero the fan and DX coils electricity consumption
        state.dataHVACGlobal->DXElecCoolingPower = 0.0;
        state.dataHVACGlobal->DXElecHeatingPower = 0.0;
        state.dataHVACGlobal->ElecHeatingCoilPower = 0.0;
        state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = 0.0;
        state.dataHVACGlobal->DefrostElecPower = 0.0;

        // initialize local variables
        bool UnitOn = true;
        int OutletNode = changeOverByPassVAV.AirOutNode;
        int InletNode = changeOverByPassVAV.AirInNode;
        Real64 AirMassFlow = state.dataLoopNodes->Node(InletNode).MassFlowRate;
        Real64 PartLoadFrac = 0.0;

        // set the on/off flags
        if (changeOverByPassVAV.fanOp == HVAC::FanOp::Cycling) {
            // cycling unit only runs if there is a cooling or heating load.
            if (changeOverByPassVAV.HeatCoolMode == 0 || AirMassFlow < HVAC::SmallMassFlow) {
                UnitOn = false;
            }
        } else if (changeOverByPassVAV.fanOp == HVAC::FanOp::Continuous) {
            // continuous unit: fan runs if scheduled on; coil runs only if there is a cooling or heating load
            if (AirMassFlow < HVAC::SmallMassFlow) {
                UnitOn = false;
            }
        }

        state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;

        if (UnitOn) {
            ControlCBVAVOutput(state, CBVAVNum, FirstHVACIteration, PartLoadFrac, OnOffAirFlowRatio, HXUnitOn);
        } else {
            CalcCBVAV(state, CBVAVNum, FirstHVACIteration, PartLoadFrac, QSensUnitOut, OnOffAirFlowRatio, HXUnitOn);
        }
        if (changeOverByPassVAV.modeChanged) {
            // set outlet node SP for mixed air SP manager
            state.dataLoopNodes->Node(changeOverByPassVAV.AirOutNode).TempSetPoint = CalcSetPointTempTarget(state, CBVAVNum);
            if (changeOverByPassVAV.OutNodeSPMIndex > 0) {                                              // update mixed air SPM if exists
                state.dataSetPointManager->spms(changeOverByPassVAV.OutNodeSPMIndex)->calculate(state); // update mixed air SP based on new mode
                SetPointManager::UpdateMixedAirSetPoints(state); // need to know control node to fire off just one of these, do this later
            }
        }

        // calculate delivered capacity
        AirMassFlow = state.dataLoopNodes->Node(OutletNode).MassFlowRate;

        Real64 QTotUnitOut = AirMassFlow * (state.dataLoopNodes->Node(OutletNode).Enthalpy - state.dataLoopNodes->Node(InletNode).Enthalpy);

        Real64 MinOutletHumRat = min(state.dataLoopNodes->Node(InletNode).HumRat, state.dataLoopNodes->Node(OutletNode).HumRat);

        QSensUnitOut = AirMassFlow * (Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(OutletNode).Temp, MinOutletHumRat) -
                                      Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(InletNode).Temp, MinOutletHumRat));

        // report variables
        changeOverByPassVAV.CompPartLoadRatio = state.dataHVACUnitaryBypassVAV->SaveCompressorPLR;
        if (UnitOn) {
            changeOverByPassVAV.FanPartLoadRatio = 1.0;
        } else {
            changeOverByPassVAV.FanPartLoadRatio = 0.0;
        }

        changeOverByPassVAV.TotCoolEnergyRate = std::abs(min(0.0, QTotUnitOut));
        changeOverByPassVAV.TotHeatEnergyRate = std::abs(max(0.0, QTotUnitOut));
        changeOverByPassVAV.SensCoolEnergyRate = std::abs(min(0.0, QSensUnitOut));
        changeOverByPassVAV.SensHeatEnergyRate = std::abs(max(0.0, QSensUnitOut));
        changeOverByPassVAV.LatCoolEnergyRate = std::abs(min(0.0, (QTotUnitOut - QSensUnitOut)));
        changeOverByPassVAV.LatHeatEnergyRate = std::abs(max(0.0, (QTotUnitOut - QSensUnitOut)));

        Real64 HeatingPower = 0.0; // DX Htg coil Plus CrankCase electric power use or electric heating coil [W]
        Real64 locDefrostPower = 0.0;
        if (changeOverByPassVAV.heatCoilType == HVAC::CoilType::HeatingDXSingleSpeed) {
            HeatingPower = state.dataHVACGlobal->DXElecHeatingPower;
            locDefrostPower = state.dataHVACGlobal->DefrostElecPower;
        } else if (changeOverByPassVAV.heatCoilType == HVAC::CoilType::HeatingDXVariableSpeed) {
            HeatingPower = state.dataHVACGlobal->DXElecHeatingPower;
            locDefrostPower = state.dataHVACGlobal->DefrostElecPower;
        } else if (changeOverByPassVAV.heatCoilType == HVAC::CoilType::HeatingElectric) {
            HeatingPower = state.dataHVACGlobal->ElecHeatingCoilPower;
        } else {
            HeatingPower = 0.0;
        }

        Real64 locFanElecPower = state.dataFans->fans(changeOverByPassVAV.FanIndex)->totalPower;

        changeOverByPassVAV.ElecPower = locFanElecPower + state.dataHVACGlobal->DXElecCoolingPower + HeatingPower + locDefrostPower;
    }

    void GetCBVAV(EnergyPlusData &state)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2006
        //       MODIFIED       Bereket Nigusse, FSEC, April 2011: added OA Mixer object type

        // PURPOSE OF THIS SUBROUTINE:
        // Obtains input data for changeover-bypass VAV systems and stores it in CBVAV data structures

        // METHODOLOGY EMPLOYED:
        // Uses "Get" routines to read in data.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view routineName = "GetCBVAV";

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;                // Number of Alphas for each GetObjectItem call
        int NumNumbers;               // Number of Numbers for each GetObjectItem call
        int IOStatus;                 // Used in GetObjectItem
        std::string CompSetFanInlet;  // Used in SetUpCompSets call
        std::string CompSetFanOutlet; // Used in SetUpCompSets call
        bool ErrorsFound(false);      // Set to true if errors in input, fatal at end of routine
        Array1D_int OANodeNums(4);    // Node numbers of OA mixer (OA, EA, RA, MA)
        bool DXCoilErrFlag;           // used in warning messages

        Array1D_string Alphas(20, "");
        Array1D<Real64> Numbers(9, 0.0);
        Array1D_string cAlphaFields(20, "");
        Array1D_string cNumericFields(9, "");
        Array1D_bool lAlphaBlanks(20, true);
        Array1D_bool lNumericBlanks(9, true);

        // find the number of each type of CBVAV unit
        std::string CurrentModuleObject = "AirLoopHVAC:UnitaryHeatCool:VAVChangeoverBypass";

        auto &s_node = state.dataLoopNodes;
        
        // Update Num in state and make local convenience copy
        int NumCBVAV = state.dataHVACUnitaryBypassVAV->NumCBVAV =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);

        // allocate the data structures
        state.dataHVACUnitaryBypassVAV->CBVAVs.resize(NumCBVAV);
        state.dataHVACUnitaryBypassVAV->CheckEquipName.dimension(NumCBVAV, true);

        // loop over CBVAV units; get and load the input data
        for (int CBVAVNum = 1; CBVAVNum <= NumCBVAV; ++CBVAVNum) {
            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     CurrentModuleObject,
                                                                     CBVAVNum,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNum);

            cbvav.Name = Alphas(1);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, cbvav.Name};

            cbvav.UnitType = CurrentModuleObject;
            if (lAlphaBlanks(2)) {
                cbvav.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((cbvav.availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ErrorsFound = true;
            }

            cbvav.MaxCoolAirVolFlow = Numbers(1);
            if (cbvav.MaxCoolAirVolFlow <= 0.0 && cbvav.MaxCoolAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} illegal {} = {:.7T}", CurrentModuleObject, cNumericFields(1), Numbers(1)));
                ShowContinueError(state, format("{} must be greater than zero.", cNumericFields(1)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                ErrorsFound = true;
            }

            cbvav.MaxHeatAirVolFlow = Numbers(2);
            if (cbvav.MaxHeatAirVolFlow <= 0.0 && cbvav.MaxHeatAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} illegal {} = {:.7T}", CurrentModuleObject, cNumericFields(2), Numbers(2)));
                ShowContinueError(state, format("{} must be greater than zero.", cNumericFields(2)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                ErrorsFound = true;
            }

            cbvav.MaxNoCoolHeatAirVolFlow = Numbers(3);
            if (cbvav.MaxNoCoolHeatAirVolFlow < 0.0 && cbvav.MaxNoCoolHeatAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} illegal {} = {:.7T}", CurrentModuleObject, cNumericFields(3), Numbers(3)));
                ShowContinueError(state, format("{} must be greater than or equal to zero.", cNumericFields(3)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                ErrorsFound = true;
            }

            cbvav.CoolOutAirVolFlow = Numbers(4);
            if (cbvav.CoolOutAirVolFlow < 0.0 && cbvav.CoolOutAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} illegal {} = {:.7T}", CurrentModuleObject, cNumericFields(4), Numbers(4)));
                ShowContinueError(state, format("{} must be greater than or equal to zero.", cNumericFields(4)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                ErrorsFound = true;
            }

            cbvav.HeatOutAirVolFlow = Numbers(5);
            if (cbvav.HeatOutAirVolFlow < 0.0 && cbvav.HeatOutAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} illegal {} = {:.7T}", CurrentModuleObject, cNumericFields(5), Numbers(5)));
                ShowContinueError(state, format("{} must be greater than or equal to zero.", cNumericFields(5)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                ErrorsFound = true;
            }

            cbvav.NoCoolHeatOutAirVolFlow = Numbers(6);
            if (cbvav.NoCoolHeatOutAirVolFlow < 0.0 && cbvav.NoCoolHeatOutAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} illegal {} = {:.7T}", CurrentModuleObject, cNumericFields(6), Numbers(6)));
                ShowContinueError(state, format("{} must be greater than or equal to zero.", cNumericFields(6)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                ErrorsFound = true;
            }

            cbvav.outAirSched = Sched::GetSchedule(state, Alphas(3));
            if (cbvav.outAirSched != nullptr) {
                if (!cbvav.outAirSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 1.0)) {
                    Sched::ShowSevereBadMinMax(state, eoh, cAlphaFields(3), Alphas(3), Clusive::In, 0.0, Clusive::In, 1.0);
                    ErrorsFound = true;
                }
            }

            cbvav.AirInNode =
                NodeInputManager::GetOnlySingleNode(state,
                                                    Alphas(4),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatCoolVAVChangeoverBypass,
                                                    Alphas(1),
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Inlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);

            std::string MixerInletNodeName = Alphas(5);
            std::string SplitterOutletNodeName = Alphas(6);

            cbvav.AirOutNode =
                NodeInputManager::GetOnlySingleNode(state,
                                                    Alphas(7),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatCoolVAVChangeoverBypass,
                                                    Alphas(1),
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Outlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);

            cbvav.SplitterOutletAirNode =
                NodeInputManager::GetOnlySingleNode(state,
                                                    SplitterOutletNodeName,
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatCoolVAVChangeoverBypass,
                                                    Alphas(1),
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Internal,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);

            if (NumAlphas > 19 && !lAlphaBlanks(20)) {
                cbvav.PlenumMixerInletAirNode =
                    NodeInputManager::GetOnlySingleNode(state,
                                                        Alphas(20),
                                                        ErrorsFound,
                                                        DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatCoolVAVChangeoverBypass,
                                                        Alphas(1),
                                                        DataLoopNode::NodeFluidType::Air,
                                                        DataLoopNode::ConnectionType::Internal,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        DataLoopNode::ObjectIsParent);
                cbvav.PlenumMixerInletAirNode =
                    NodeInputManager::GetOnlySingleNode(state,
                                                        Alphas(20),
                                                        ErrorsFound,
                                                        DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatCoolVAVChangeoverBypass,
                                                        Alphas(1) + "_PlenumMixerInlet",
                                                        DataLoopNode::NodeFluidType::Air,
                                                        DataLoopNode::ConnectionType::Outlet,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        DataLoopNode::ObjectIsParent);
            }

            cbvav.plenumIndex = ZonePlenum::getReturnPlenumIndexFromInletNode(state, cbvav.PlenumMixerInletAirNode);
            cbvav.mixerIndex = MixerComponent::getZoneMixerIndexFromInletNode(state, cbvav.PlenumMixerInletAirNode);
            if (cbvav.plenumIndex > 0 && cbvav.mixerIndex > 0) {
                ShowSevereError(state, format("{}: {}", CurrentModuleObject, cbvav.Name));
                ShowContinueError(state, format("Illegal connection for {} = \"{}\".", cAlphaFields(20), Alphas(20)));
                ShowContinueError(
                    state, format("{} cannot be connected to both an AirloopHVAC:ReturnPlenum and an AirloopHVAC:ZoneMixer.", cAlphaFields(20)));
                ErrorsFound = true;
            } else if (cbvav.plenumIndex == 0 && cbvav.mixerIndex == 0 && cbvav.PlenumMixerInletAirNode > 0) {
                ShowSevereError(state, format("{}: {}", CurrentModuleObject, cbvav.Name));
                ShowContinueError(state, format("Illegal connection for {} = \"{}\".", cAlphaFields(20), Alphas(20)));
                ShowContinueError(
                    state,
                    format("{} must be connected to an AirloopHVAC:ReturnPlenum or AirloopHVAC:ZoneMixer. No connection found.", cAlphaFields(20)));
                ErrorsFound = true;
            }

            cbvav.MixerInletAirNode =
                NodeInputManager::GetOnlySingleNode(state,
                                                    MixerInletNodeName,
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatCoolVAVChangeoverBypass,
                                                    Alphas(1),
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Internal,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);

            cbvav.MixerInletAirNode =
                NodeInputManager::GetOnlySingleNode(state,
                                                    MixerInletNodeName,
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatCoolVAVChangeoverBypass,
                                                    Alphas(1) + "_Mixer",
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Outlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);

            cbvav.SplitterOutletAirNode =
                NodeInputManager::GetOnlySingleNode(state,
                                                    SplitterOutletNodeName,
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatCoolVAVChangeoverBypass,
                                                    Alphas(1) + "_Splitter",
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Inlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);

            cbvav.OAMixType = Alphas(8);
            cbvav.OAMixName = Alphas(9);

            bool errFlag = false;
            ValidateComponent(state, cbvav.OAMixType, cbvav.OAMixName, errFlag, CurrentModuleObject);
            if (errFlag) {
                ShowContinueError(state, format("specified in {} = \"{}\".", CurrentModuleObject, cbvav.Name));
                ErrorsFound = true;
            } else {
                // Get OA Mixer node numbers
                OANodeNums = MixedAir::GetOAMixerNodeNumbers(state, cbvav.OAMixName, errFlag);
                if (errFlag) {
                    ShowContinueError(state, format("that was specified in {} = {}", CurrentModuleObject, cbvav.Name));
                    ShowContinueError(state, "..OutdoorAir:Mixer is required. Enter an OutdoorAir:Mixer object with this name.");
                    ErrorsFound = true;
                } else {
                    cbvav.MixerOutsideAirNode = OANodeNums(1);
                    cbvav.MixerReliefAirNode = OANodeNums(2);
                    // cbvav%MixerInletAirNode  = OANodeNums(3)
                    cbvav.MixerMixedAirNode = OANodeNums(4);
                }
            }

            if (cbvav.MixerInletAirNode != OANodeNums(3)) {
                ShowSevereError(state, format("{}: {}", CurrentModuleObject, cbvav.Name));
                ShowContinueError(state, format("Illegal {} = {}.", cAlphaFields(5), MixerInletNodeName));
                ShowContinueError(
                    state, format("{} must be the same as the return air stream node specified in the OutdoorAir:Mixer object.", cAlphaFields(5)));
                ErrorsFound = true;
            }

            if (cbvav.MixerInletAirNode == cbvav.AirInNode) {
                ShowSevereError(state, format("{}: {}", CurrentModuleObject, cbvav.Name));
                ShowContinueError(state, format("Illegal {} = {}.", cAlphaFields(5), MixerInletNodeName));
                ShowContinueError(state, format("{} must be different than the {}.", cAlphaFields(5), cAlphaFields(4)));
                ErrorsFound = true;
            }

            if (cbvav.SplitterOutletAirNode == cbvav.AirOutNode) {
                ShowSevereError(state, format("{}: {}", CurrentModuleObject, cbvav.Name));
                ShowContinueError(state, format("Illegal {} = {}.", cAlphaFields(6), SplitterOutletNodeName));
                ShowContinueError(state, format("{} must be different than the {}.", cAlphaFields(6), cAlphaFields(7)));
                ErrorsFound = true;
            }

            // required field must be Key=Fan:ConstantVolume, Fan:OnOff or Fan:SystemModel and read in as upper case
            cbvav.fanType = static_cast<HVAC::FanType>(getEnumValue(HVAC::fanTypeNamesUC, Alphas(10)));
            assert(cbvav.fanType != HVAC::FanType::Invalid);

            cbvav.FanName = Alphas(11);
            int fanOutletNode(0);

            // check that the fan exists
            if ((cbvav.FanIndex = Fans::GetFanIndex(state, cbvav.FanName)) == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(11), cbvav.FanName);
                ErrorsFound = true;
                cbvav.FanVolFlow = 9999.0;
            } else {
                auto *fan = state.dataFans->fans(cbvav.FanIndex);
                cbvav.FanInletNodeNum = fan->inletNodeNum;
                fanOutletNode = fan->outletNodeNum;
                cbvav.FanVolFlow = fan->maxAirFlowRate;
            }

            // required field must be Key=BlowThrough or DrawThrough and read in as BLOWTHROUGH or DRAWTHROUGH
            cbvav.fanPlace = static_cast<HVAC::FanPlace>(getEnumValue(HVAC::fanPlaceNamesUC, Alphas(12)));

            if (cbvav.fanPlace == HVAC::FanPlace::DrawThru) {
                if (cbvav.SplitterOutletAirNode != fanOutletNode) {
                    ShowSevereError(state, format("{}: {}", CurrentModuleObject, cbvav.Name));
                    ShowContinueError(state, format("Illegal {} = {}.", cAlphaFields(6), SplitterOutletNodeName));
                    ShowContinueError(state,
                                      format("{} must be the same as the fan outlet node specified in {} = {}: {} when draw through {} is selected.",
                                             cAlphaFields(6),
                                             cAlphaFields(10),
                                             Alphas(10),
                                             cbvav.FanName,
                                             cAlphaFields(11)));
                    ErrorsFound = true;
                }
            }

            if (cbvav.FanVolFlow != DataSizing::AutoSize) {
                if (cbvav.FanVolFlow < cbvav.MaxCoolAirVolFlow && cbvav.MaxCoolAirVolFlow != DataSizing::AutoSize) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in {} = {} is less than the ",
                                            CurrentModuleObject,
                                            cbvav.FanVolFlow,
                                            cAlphaFields(11),
                                            cbvav.FanName) +
                                         cNumericFields(1));
                    ShowContinueError(state, format(" {} is reset to the fan flow rate and the simulation continues.", cNumericFields(1)));
                    ShowContinueError(state, format(" Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                    cbvav.MaxCoolAirVolFlow = cbvav.FanVolFlow;
                }
                if (cbvav.FanVolFlow < cbvav.MaxHeatAirVolFlow && cbvav.MaxHeatAirVolFlow != DataSizing::AutoSize) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in {} = {} is less than the ",
                                            CurrentModuleObject,
                                            cbvav.FanVolFlow,
                                            cAlphaFields(11),
                                            cbvav.FanName) +
                                         cNumericFields(2));
                    ShowContinueError(state, format(" {} is reset to the fan flow rate and the simulation continues.", cNumericFields(2)));
                    ShowContinueError(state, format(" Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                    cbvav.MaxHeatAirVolFlow = cbvav.FanVolFlow;
                }
            }

            //   only check that OA flow in cooling is >= SA flow in cooling when they are not autosized
            if (cbvav.CoolOutAirVolFlow > cbvav.MaxCoolAirVolFlow && cbvav.CoolOutAirVolFlow != DataSizing::AutoSize &&
                cbvav.MaxCoolAirVolFlow != DataSizing::AutoSize) {
                ShowWarningError(state, format("{}: {} cannot be greater than {}", CurrentModuleObject, cNumericFields(4), cNumericFields(1)));
                ShowContinueError(state, format(" {} is reset to the fan flow rate and the simulation continues.", cNumericFields(4)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                cbvav.CoolOutAirVolFlow = cbvav.FanVolFlow;
            }

            //   only check that SA flow in heating is >= OA flow in heating when they are not autosized
            if (cbvav.HeatOutAirVolFlow > cbvav.MaxHeatAirVolFlow && cbvav.HeatOutAirVolFlow != DataSizing::AutoSize &&
                cbvav.MaxHeatAirVolFlow != DataSizing::AutoSize) {
                ShowWarningError(state, format("{}: {} cannot be greater than {}", CurrentModuleObject, cNumericFields(5), cNumericFields(2)));
                ShowContinueError(state, format(" {} is reset to the fan flow rate and the simulation continues.", cNumericFields(5)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                cbvav.HeatOutAirVolFlow = cbvav.FanVolFlow;
            }

            cbvav.coolCoilType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Alphas(14)));
            cbvav.CoolCoilName = Alphas(15);

            if (cbvav.coolCoilType == HVAC::CoilType::CoolingDXSingleSpeed ||
                cbvav.coolCoilType == HVAC::CoilType::CoolingDXTwoStageWHumControl) {
                cbvav.CoolCoilNum = DXCoils::GetCoilIndex(state, cbvav.CoolCoilName);
                if (cbvav.CoolCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(15), cbvav.CoolCoilName);
                    ErrorsFound = true;
                } else {
                    auto const &dxCoil = state.dataDXCoils->DXCoil(cbvav.CoolCoilNum);
                    cbvav.CoolCoilAirInletNode = dxCoil.AirInNode;
                    cbvav.CoolCoilAirOutletNode = dxCoil.AirOutNode;
                    cbvav.CondenserNodeNum = dxCoil.CondenserInletNodeNum(1);
                }
                
            } else if (cbvav.coolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
                cbvav.CoolCoilNum = VariableSpeedCoils::GetCoilIndex(state, cbvav.CoolCoilName);
                if (cbvav.CoolCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(15), cbvav.CoolCoilName);
                    ErrorsFound = true;
                } else {
                    auto const &vsCoil = state.dataVariableSpeedCoils->VarSpeedCoil(cbvav.CoolCoilNum);
                    cbvav.CoolCoilAirInletNode = vsCoil.AirInletNodeNum;
                    cbvav.CoolCoilAirOutletNode = vsCoil.AirOutletNodeNum;
                    cbvav.CondenserNodeNum = vsCoil.CondenserInletNodeNum;
                }
                
            } else if (cbvav.coolCoilType == HVAC::CoilType::CoolingDXHXAssisted) {
              // For an HXAssisted coil, these variables get bumped
              // over to hxCoolCoil so that CoolCoil variables can
              // refer to the actual child DX coil
                cbvav.CoolCoilNum = HXAssistCoil::GetCoilIndex(state, cbvav.CoolCoilName);
                if (cbvav.CoolCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(15), cbvav.CoolCoilName);
                    ErrorsFound = true;
                } else {
                    // The HXAssisted coil has already mined all of these from the child coil
                    cbvav.childCoolCoilType = HXAssistCoil::GetCoilChildCoilType(state, cbvav.CoolCoilNum);
                    cbvav.childCoolCoilName = HXAssistCoil::GetCoilChildCoilName(state, cbvav.CoolCoilNum);
                    cbvav.childCoolCoilNum = HXAssistCoil::GetCoilChildCoilIndex(state, cbvav.CoolCoilNum);
                    // the HXAssisted coil should have already gathered all of these up properly
                    cbvav.CoolCoilAirInletNode = HXAssistCoil::GetCoilAirInletNode(state, cbvav.childCoolCoilNum);
                    cbvav.CoolCoilAirOutletNode = HXAssistCoil::GetCoilAirOutletNode(state, cbvav.childCoolCoilNum);
                    cbvav.CondenserNodeNum = HXAssistCoil::GetCoilCondenserInletNode(state, cbvav.childCoolCoilNum);
                }
            }

            cbvav.fanOpModeSched = Sched::GetSchedule(state, Alphas(13));
            if (cbvav.fanOpModeSched != nullptr) {
                if (!cbvav.fanOpModeSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 1.0)) {
                    Sched::ShowSevereBadMinMax(state, eoh, cAlphaFields(13), Alphas(13), Clusive::In, 0.0, Clusive::In, 1.0);
                    ShowContinueError(state, "A value of 0 represents cycling fan mode, any other value up to 1 represents constant fan mode.");
                    ErrorsFound = true;
                }

                //     Check supply air fan operating mode for cycling fan, if NOT cycling fan set AirFlowControl
                if (!cbvav.fanOpModeSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 0.0)) { // Autodesk:Note Range is 0 to 0?
                    //       set air flow control mode,
                    //       UseCompressorOnFlow  = operate at last cooling or heating air flow requested when compressor is off
                    //       UseCompressorOffFlow = operate at value specified by user (no input for this object type, UseCompONFlow)
                    //       AirFlowControl only valid if fan opmode = HVAC::FanOp::Continuous
                    cbvav.AirFlowControl =
                        (cbvav.MaxNoCoolHeatAirVolFlow == 0.0) ? AirFlowCtrlMode::UseCompressorOnFlow : AirFlowCtrlMode::UseCompressorOffFlow;
                }

            } else {
                if (!lAlphaBlanks(13)) {
                    ShowWarningError(state, format("{}: {}", CurrentModuleObject, cbvav.Name));
                    ShowContinueError(state,
                                      format("{} = {} not found. Supply air fan operating mode set to constant operation and simulation continues.",
                                             cAlphaFields(13),
                                             Alphas(13)));
                }
                cbvav.fanOp = HVAC::FanOp::Continuous;
                if (cbvav.MaxNoCoolHeatAirVolFlow == 0.0) {
                    cbvav.AirFlowControl = AirFlowCtrlMode::UseCompressorOnFlow;
                } else {
                    cbvav.AirFlowControl = AirFlowCtrlMode::UseCompressorOffFlow;
                }
            }

            //   Check FanVolFlow, must be >= CBVAV flow
            if (cbvav.FanVolFlow != DataSizing::AutoSize) {
                if (cbvav.FanVolFlow < cbvav.MaxNoCoolHeatAirVolFlow && cbvav.MaxNoCoolHeatAirVolFlow != DataSizing::AutoSize &&
                    cbvav.MaxNoCoolHeatAirVolFlow != 0.0) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in {} = {} is less than ",
                                            CurrentModuleObject,
                                            cbvav.FanVolFlow,
                                            cAlphaFields(11),
                                            cbvav.FanName) +
                                         cNumericFields(3));
                    ShowContinueError(state, format(" {} is reset to the fan flow rate and the simulation continues.", cNumericFields(3)));
                    ShowContinueError(state, format(" Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                    cbvav.MaxNoCoolHeatAirVolFlow = cbvav.FanVolFlow;
                }
            }
            //   only check that OA flow when compressor is OFF is >= SA flow when compressor is OFF when both are not autosized and
            //   that MaxNoCoolHeatAirVolFlow is /= 0 (trigger to use compressor ON flow, see AirFlowControl variable initialization above)
            if (cbvav.NoCoolHeatOutAirVolFlow > cbvav.MaxNoCoolHeatAirVolFlow && cbvav.NoCoolHeatOutAirVolFlow != DataSizing::AutoSize &&
                cbvav.MaxNoCoolHeatAirVolFlow != DataSizing::AutoSize && cbvav.MaxNoCoolHeatAirVolFlow != 0.0) {
                ShowWarningError(state, format("{}: {} cannot be greater than {}", CurrentModuleObject, cNumericFields(6), cNumericFields(3)));
                ShowContinueError(state, format(" {} is reset to the fan flow rate and the simulation continues.", cNumericFields(6)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                cbvav.NoCoolHeatOutAirVolFlow = cbvav.FanVolFlow;
            }

            std::string thisHeatCoilType = Alphas(16);
            cbvav.heatCoilType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, thisHeatCoilType));
            cbvav.HeatCoilName = Alphas(17);

            DXCoilErrFlag = false;
            if (cbvav.heatCoilType == HVAC::CoilType::HeatingDXSingleSpeed) {
                cbvav.HeatCoilNum = DXCoils::GetCoilIndex(state, cbvav.HeatCoilName);
                if (cbvav.HeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(17), cbvav.HeatCoilName);
                    ErrorsFound = true;
                } else {
                    auto const &dxCoil = state.dataDXCoils->DXCoil(cbvav.HeatCoilNum);
                    cbvav.MinOATCompressor = dxCoil.MinOATCompressor;
                    cbvav.HeatCoilAirInletNode = dxCoil.AirInNode;
                    cbvav.HeatCoilAirOutletNode = dxCoil.AirOutNode;
                }
                
            } else if (cbvav.heatCoilType == HVAC::CoilType::HeatingDXVariableSpeed) {
                cbvav.HeatCoilNum = VariableSpeedCoils::GetCoilIndex(state, cbvav.HeatCoilName);
                if (cbvav.HeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(17), cbvav.HeatCoilName);
                    ErrorsFound = true;
                } else {
                    auto &vsCoil = state.dataVariableSpeedCoils->VarSpeedCoil(cbvav.HeatCoilNum);
                    cbvav.MinOATCompressor = vsCoil.MinOATCompressor;
                    cbvav.HeatCoilAirInletNode = vsCoil.AirInletNodeNum;
                    cbvav.HeatCoilAirOutletNode = vsCoil.AirOutletNodeNum;
                }
            
            } else if (cbvav.heatCoilType == HVAC::CoilType::HeatingGasOrOtherFuel ||
                       cbvav.heatCoilType == HVAC::CoilType::HeatingElectric) {
                cbvav.HeatCoilNum = HeatingCoils::GetCoilIndex(state, cbvav.HeatCoilName);
                if (cbvav.HeatCoilNum == 0) { 
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(17), cbvav.HeatCoilName);
                    ErrorsFound = true;
                } else {
                    auto &heatCoil = state.dataHeatingCoils->HeatingCoil(cbvav.HeatCoilNum);
                    cbvav.MinOATCompressor = -999.9;
                    cbvav.HeatCoilAirInletNode = heatCoil.AirInletNodeNum;
                    cbvav.HeatCoilAirOutletNode = heatCoil.AirOutletNodeNum;
                }
                
            } else if (cbvav.heatCoilType == HVAC::CoilType::HeatingWater) {
                cbvav.HeatCoilNum = WaterCoils::GetCoilIndex(state, cbvav.HeatCoilName);
                if (cbvav.HeatCoilNum == 0) { 
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(17), cbvav.HeatCoilName);
                    ErrorsFound = true;
                } else {
                    auto const &waterCoil = state.dataWaterCoils->WaterCoil(cbvav.HeatCoilNum);
                    cbvav.HeatCoilControlNode = waterCoil.WaterInletNodeNum;
                    cbvav.MaxHeatCoilFluidFlow = waterCoil.MaxWaterVolFlowRate;
                    cbvav.HeatCoilAirInletNode = waterCoil.AirInletNodeNum;
                    cbvav.HeatCoilAirOutletNode = waterCoil.AirOutletNodeNum;
                }
                
            } else if (cbvav.heatCoilType == HVAC::CoilType::HeatingSteam) {
                cbvav.HeatCoilNum = SteamCoils::GetCoilIndex(state, cbvav.HeatCoilName);
                if (cbvav.HeatCoilNum == 0) { 
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(17), cbvav.HeatCoilName);
                    ErrorsFound = true;
                } else {
                    auto const &steamCoil = state.dataSteamCoils->SteamCoil(cbvav.HeatCoilNum);
                    cbvav.HeatCoilAirInletNode = steamCoil.AirInletNodeNum;
                    cbvav.HeatCoilAirOutletNode = steamCoil.AirOutletNodeNum;
                    cbvav.HeatCoilControlNode = steamCoil.SteamInletNodeNum;
                    cbvav.MaxHeatCoilFluidFlow = steamCoil.MaxSteamVolFlowRate;
                    if (cbvav.MaxHeatCoilFluidFlow > 0.0) {
                        cbvav.MaxHeatCoilFluidFlow *= Fluid::GetSteam(state)->getSatDensity(state, 100.0, 1.0, routineName);
                    }
                }
            }

            if (cbvav.CoolCoilAirOutletNode != cbvav.HeatCoilAirInletNode) {
                ShowSevereError(state, format("{} illegal coil placement. Cooling coil must be upstream of heating coil.", CurrentModuleObject));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                ErrorsFound = true;
            }

            if (cbvav.fanPlace == HVAC::FanPlace::BlowThru) {
                if (cbvav.SplitterOutletAirNode != cbvav.HeatCoilAirOutletNode) {
                    ShowSevereError(state, format("{}: {}", CurrentModuleObject, cbvav.Name));
                    ShowContinueError(state, format("Illegal {} = {}.", cAlphaFields(6), SplitterOutletNodeName));
                    ShowContinueError(
                        state,
                        format(
                            "{} must be the same as the outlet node specified in the heating coil object = {}: {} when blow through {} is selected.",
                            cAlphaFields(6),
                            HVAC::coilTypeNames[(int)cbvav.heatCoilType],
                            cbvav.HeatCoilName,
                            cAlphaFields(12)));
                    ErrorsFound = true;
                }
                if (cbvav.MixerMixedAirNode != cbvav.FanInletNodeNum) {
                    ShowSevereError(state, format("{}: {}", CurrentModuleObject, cbvav.Name));
                    ShowContinueError(state,
                                      format("Illegal {}. The fan inlet node name must be the same as the mixed air node specified in the {} = {} "
                                             "when blow through {} is selected.",
                                             cAlphaFields(11),
                                             cAlphaFields(9),
                                             cbvav.OAMixName,
                                             cAlphaFields(12)));
                    ErrorsFound = true;
                }
            }

            if (cbvav.fanPlace == HVAC::FanPlace::DrawThru) {
                if (cbvav.MixerMixedAirNode != cbvav.CoolCoilAirInletNode) {
                    ShowSevereError(state, format("{}: {}", CurrentModuleObject, cbvav.Name));
                    ShowContinueError(state,
                                      format("Illegal cooling coil placement. The cooling coil inlet node name must be the same as the mixed air "
                                             "node specified in the {} = {} when draw through {} is selected.",
                                             cAlphaFields(9),
                                             cbvav.OAMixName,
                                             cAlphaFields(12)));
                    ErrorsFound = true;
                }
            }

            if (Util::SameString(Alphas(18), "CoolingPriority")) {
                cbvav.PriorityControl = PriorityCtrlMode::CoolingPriority;
            } else if (Util::SameString(Alphas(18), "HeatingPriority")) {
                cbvav.PriorityControl = PriorityCtrlMode::HeatingPriority;
            } else if (Util::SameString(Alphas(18), "ZonePriority")) {
                cbvav.PriorityControl = PriorityCtrlMode::ZonePriority;
            } else if (Util::SameString(Alphas(18), "LoadPriority")) {
                cbvav.PriorityControl = PriorityCtrlMode::LoadPriority;
            } else {
                ShowSevereError(state, format("{} illegal {} = {}", CurrentModuleObject, cAlphaFields(18), Alphas(18)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                ShowContinueError(state, "Valid choices are CoolingPriority, HeatingPriority, ZonePriority or LoadPriority.");
                ErrorsFound = true;
            }

            if (Numbers(7) > 0.0) {
                cbvav.MinLATCooling = Numbers(7);
            } else {
                cbvav.MinLATCooling = 10.0;
            }

            if (Numbers(8) > 0.0) {
                cbvav.MaxLATHeating = Numbers(8);
            } else {
                cbvav.MaxLATHeating = 50.0;
            }

            if (cbvav.MinLATCooling > cbvav.MaxLATHeating) {
                ShowWarningError(state, format("{}: illegal leaving air temperature specified.", CurrentModuleObject));
                ShowContinueError(state, format("Resetting {} equal to {} and the simulation continues.", cNumericFields(7), cNumericFields(8)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                cbvav.MinLATCooling = cbvav.MaxLATHeating;
            }

            // Dehumidification control mode
            if (Util::SameString(Alphas(19), "None")) {
                cbvav.DehumidControlType = DehumidControl::None;
            } else if (Util::SameString(Alphas(19), "")) {
                cbvav.DehumidControlType = DehumidControl::None;
            } else if (Util::SameString(Alphas(19), "Multimode")) {
                if (cbvav.coolCoilType == HVAC::CoilType::CoolingDXTwoStageWHumControl) {
                    cbvav.DehumidControlType = DehumidControl::Multimode;
                } else {
                    ShowWarningError(state, format("Invalid {} = {}", cAlphaFields(19), Alphas(19)));
                    ShowContinueError(state, format("In {} \"{}\".", CurrentModuleObject, cbvav.Name));
                    ShowContinueError(state, format("Valid only with {} = Coil:Cooling:DX:TwoStageWithHumidityControlMode.", cAlphaFields(14)));
                    ShowContinueError(state, format("Setting {} to \"None\" and the simulation continues.", cAlphaFields(19)));
                    cbvav.DehumidControlType = DehumidControl::None;
                }
            } else if (Util::SameString(Alphas(19), "CoolReheat")) {
                if (cbvav.coolCoilType == HVAC::CoilType::CoolingDXTwoStageWHumControl) {
                    cbvav.DehumidControlType = DehumidControl::CoolReheat;
                } else {
                    ShowWarningError(state, format("Invalid {} = {}", cAlphaFields(19), Alphas(19)));
                    ShowContinueError(state, format("In {} \"{}\".", CurrentModuleObject, cbvav.Name));
                    ShowContinueError(state, format("Valid only with {} = Coil:Cooling:DX:TwoStageWithHumidityControlMode.", cAlphaFields(14)));
                    ShowContinueError(state, format("Setting {} to \"None\" and the simulation continues.", cAlphaFields(19)));
                    cbvav.DehumidControlType = DehumidControl::None;
                }
            } else {
                ShowSevereError(state, format("Invalid {} ={}", cAlphaFields(19), Alphas(19)));
                ShowContinueError(state, format("In {} \"{}\".", CurrentModuleObject, cbvav.Name));
            }

            if (NumNumbers > 8) {
                cbvav.minModeChangeTime = Numbers(9);
            }

            //   Initialize last mode of compressor operation
            cbvav.LastMode = HeatingMode;

            if (cbvav.fanType == HVAC::FanType::OnOff || cbvav.fanType == HVAC::FanType::Constant) {
                HVAC::FanType fanType2 = state.dataFans->fans(cbvav.FanIndex)->type;
                if (cbvav.fanType != fanType2) {
                    ShowWarningError(
                        state,
                        format("{} has {} = {} which is inconsistent with the fan object.", CurrentModuleObject, cAlphaFields(10), Alphas(10)));
                    ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                    ShowContinueError(state,
                                      format(" The fan object ({}) is actually a valid fan type and the simulation continues.", cbvav.FanName));
                    ShowContinueError(state, " Node connections errors may result due to the inconsistent fan type.");
                }
            }

            // Add fan to component sets array
            if (cbvav.fanPlace == HVAC::FanPlace::BlowThru) {
                CompSetFanInlet = s_node->NodeID(cbvav.MixerMixedAirNode);
                CompSetFanOutlet = s_node->NodeID(cbvav.CoolCoilAirInletNode);
            } else {
                CompSetFanInlet = s_node->NodeID(cbvav.HeatCoilAirOutletNode);
                CompSetFanOutlet = SplitterOutletNodeName;
            }
            std::string CompSetCoolInlet = s_node->NodeID(cbvav.CoolCoilAirInletNode);
            std::string CompSetCoolOutlet = s_node->NodeID(cbvav.CoolCoilAirOutletNode);

            // Add fan to component sets array
            BranchNodeConnections::SetUpCompSets(
                state, cbvav.UnitType, cbvav.Name, Alphas(10), cbvav.FanName, CompSetFanInlet, CompSetFanOutlet);

            // Add cooling coil to component sets array
            BranchNodeConnections::SetUpCompSets(state,
                                                 cbvav.UnitType,
                                                 cbvav.Name,
                                                 HVAC::coilTypeNames[(int)cbvav.coolCoilType],
                                                 cbvav.CoolCoilName,
                                                 CompSetCoolInlet,
                                                 CompSetCoolOutlet);

            // Add heating coil to component sets array
            BranchNodeConnections::SetUpCompSets(state,
                                                 cbvav.UnitType,
                                                 cbvav.Name,
                                                 HVAC::coilTypeNames[(int)cbvav.heatCoilType],
                                                 cbvav.HeatCoilName,
                                                 s_node->NodeID(cbvav.HeatCoilAirInletNode),
                                                 s_node->NodeID(cbvav.HeatCoilAirOutletNode));

            // Set up component set for OA mixer - use OA node and Mixed air node
            BranchNodeConnections::SetUpCompSets(state,
                                                 cbvav.UnitType,
                                                 cbvav.Name,
                                                 cbvav.OAMixType,
                                                 cbvav.OAMixName,
                                                 s_node->NodeID(cbvav.MixerOutsideAirNode),
                                                 s_node->NodeID(cbvav.MixerMixedAirNode));

            BranchNodeConnections::TestCompSet(state,
                                               cbvav.UnitType,
                                               cbvav.Name,
                                               s_node->NodeID(cbvav.AirInNode),
                                               s_node->NodeID(cbvav.AirOutNode),
                                               "Air Nodes");

            //   Find air loop associated with CBVAV system
            for (int AirLoopNum = 1; AirLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirLoopNum) {
                for (int BranchNum = 1; BranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).NumBranches; ++BranchNum) {
                    for (int CompNum = 1; CompNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).TotalComponents;
                         ++CompNum) {
                        if (!Util::SameString(state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).Name,
                                              cbvav.Name) ||
                            !Util::SameString(state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).TypeOf,
                                              cbvav.UnitType))
                            continue;
                        cbvav.AirLoopNumber = AirLoopNum;
                        //         Should EXIT here or do other checking?
                        break;
                    }
                }
            }

            if (cbvav.AirLoopNumber > 0) {
                cbvav.NumControlledZones = state.dataAirLoop->AirToZoneNodeInfo(cbvav.AirLoopNumber).NumZonesCooled;
                cbvav.ControlledZoneNum.allocate(cbvav.NumControlledZones);
                cbvav.ControlledZoneNodeNum.allocate(cbvav.NumControlledZones);
                cbvav.CBVAVBoxOutletNode.allocate(cbvav.NumControlledZones);
                cbvav.ZoneSequenceCoolingNum.allocate(cbvav.NumControlledZones);
                cbvav.ZoneSequenceHeatingNum.allocate(cbvav.NumControlledZones);

                cbvav.ControlledZoneNum = 0;
                for (int AirLoopZoneNum = 1; AirLoopZoneNum <= state.dataAirLoop->AirToZoneNodeInfo(cbvav.AirLoopNumber).NumZonesCooled;
                     ++AirLoopZoneNum) {
                    cbvav.ControlledZoneNum(AirLoopZoneNum) =
                        state.dataAirLoop->AirToZoneNodeInfo(cbvav.AirLoopNumber).CoolCtrlZoneNums(AirLoopZoneNum);
                    if (cbvav.ControlledZoneNum(AirLoopZoneNum) > 0) {
                        cbvav.ControlledZoneNodeNum(AirLoopZoneNum) =
                            state.dataZoneEquip->ZoneEquipConfig(cbvav.ControlledZoneNum(AirLoopZoneNum)).ZoneNode;
                        cbvav.CBVAVBoxOutletNode(AirLoopZoneNum) =
                            state.dataAirLoop->AirToZoneNodeInfo(cbvav.AirLoopNumber).CoolZoneInletNodes(AirLoopZoneNum);
                        // check for thermostat in controlled zone
                        bool FoundTstatZone = false;
                        for (int TstatZoneNum = 1; TstatZoneNum <= state.dataZoneCtrls->NumTempControlledZones; ++TstatZoneNum) {
                            if (state.dataZoneCtrls->TempControlledZone(TstatZoneNum).ActualZoneNum != cbvav.ControlledZoneNum(AirLoopZoneNum))
                                continue;
                            
                            FoundTstatZone = true;
                        }
                        if (!FoundTstatZone) {
                            ShowWarningError(state, format("{} \"{}\"", CurrentModuleObject, cbvav.Name));
                            ShowContinueError(state,
                                              format("Thermostat not found in zone = {} and the simulation continues.",
                                                     state.dataZoneEquip->ZoneEquipConfig(cbvav.ControlledZoneNum(AirLoopZoneNum)).ZoneName));
                            ShowContinueError(state, "This zone will not be controlled to a temperature setpoint.");
                        }
                        int zoneNum = cbvav.ControlledZoneNum(AirLoopZoneNum);
                        int zoneInlet = cbvav.CBVAVBoxOutletNode(AirLoopZoneNum);
                        // setup zone equipment sequence information based on finding matching air terminal
                        if (state.dataZoneEquip->ZoneEquipConfig(zoneNum).EquipListIndex > 0) {
                            int coolingPriority = 0;
                            int heatingPriority = 0;
                            state.dataZoneEquip->ZoneEquipList(state.dataZoneEquip->ZoneEquipConfig(zoneNum).EquipListIndex)
                                .getPrioritiesForInletNode(state, zoneInlet, coolingPriority, heatingPriority);
                            cbvav.ZoneSequenceCoolingNum(AirLoopZoneNum) = coolingPriority;
                            cbvav.ZoneSequenceHeatingNum(AirLoopZoneNum) = heatingPriority;
                        }
                        if (cbvav.ZoneSequenceCoolingNum(AirLoopZoneNum) == 0 || cbvav.ZoneSequenceHeatingNum(AirLoopZoneNum) == 0) {
                            ShowSevereError(
                                state,
                                format("AirLoopHVAC:UnitaryHeatCool:VAVChangeoverBypass, \"{}\": Airloop air terminal in the zone equipment list for "
                                       "zone = {} not found or is not allowed Zone Equipment Cooling or Heating Sequence = 0.",
                                       cbvav.Name,
                                       state.dataZoneEquip->ZoneEquipConfig(zoneNum).ZoneName));
                            ErrorsFound = true;
                        }
                    } else {
                        ShowSevereError(state, "Controlled Zone node not found.");
                        ErrorsFound = true;
                    }
                }
            } else {
            }

        } // CBVAVNum = 1,NumCBVAV

        if (ErrorsFound) {
            ShowFatalError(state, format("GetCBVAV: Errors found in getting {} input.", CurrentModuleObject));
        }

        for (int CBVAVNum = 1; CBVAVNum <= NumCBVAV; ++CBVAVNum) {
            // Setup Report variables
            auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNum);
            SetupOutputVariable(state,
                                "Unitary System Total Heating Rate",
                                Constant::Units::W,
                                cbvav.TotHeatEnergyRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                cbvav.Name);
            SetupOutputVariable(state,
                                "Unitary System Total Heating Energy",
                                Constant::Units::J,
                                cbvav.TotHeatEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                cbvav.Name);
            SetupOutputVariable(state,
                                "Unitary System Total Cooling Rate",
                                Constant::Units::W,
                                cbvav.TotCoolEnergyRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                cbvav.Name);
            SetupOutputVariable(state,
                                "Unitary System Total Cooling Energy",
                                Constant::Units::J,
                                cbvav.TotCoolEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                cbvav.Name);
            SetupOutputVariable(state,
                                "Unitary System Sensible Heating Rate",
                                Constant::Units::W,
                                cbvav.SensHeatEnergyRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                cbvav.Name);
            SetupOutputVariable(state,
                                "Unitary System Sensible Heating Energy",
                                Constant::Units::J,
                                cbvav.SensHeatEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                cbvav.Name);
            SetupOutputVariable(state,
                                "Unitary System Sensible Cooling Rate",
                                Constant::Units::W,
                                cbvav.SensCoolEnergyRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                cbvav.Name);
            SetupOutputVariable(state,
                                "Unitary System Sensible Cooling Energy",
                                Constant::Units::J,
                                cbvav.SensCoolEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                cbvav.Name);
            SetupOutputVariable(state,
                                "Unitary System Latent Heating Rate",
                                Constant::Units::W,
                                cbvav.LatHeatEnergyRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                cbvav.Name);
            SetupOutputVariable(state,
                                "Unitary System Latent Heating Energy",
                                Constant::Units::J,
                                cbvav.LatHeatEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                cbvav.Name);
            SetupOutputVariable(state,
                                "Unitary System Latent Cooling Rate",
                                Constant::Units::W,
                                cbvav.LatCoolEnergyRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                cbvav.Name);
            SetupOutputVariable(state,
                                "Unitary System Latent Cooling Energy",
                                Constant::Units::J,
                                cbvav.LatCoolEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                cbvav.Name);
            SetupOutputVariable(state,
                                "Unitary System Electricity Rate",
                                Constant::Units::W,
                                cbvav.ElecPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                cbvav.Name);
            SetupOutputVariable(state,
                                "Unitary System Electricity Energy",
                                Constant::Units::J,
                                cbvav.ElecConsumption,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                cbvav.Name);
            SetupOutputVariable(state,
                                "Unitary System Fan Part Load Ratio",
                                Constant::Units::None,
                                cbvav.FanPartLoadRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                cbvav.Name);
            SetupOutputVariable(state,
                                "Unitary System Compressor Part Load Ratio",
                                Constant::Units::None,
                                cbvav.CompPartLoadRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                cbvav.Name);
            SetupOutputVariable(state,
                                "Unitary System Bypass Air Mass Flow Rate",
                                Constant::Units::kg_s,
                                cbvav.BypassMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                cbvav.Name);
            SetupOutputVariable(state,
                                "Unitary System Air Outlet Setpoint Temperature",
                                Constant::Units::C,
                                cbvav.OutletTempSetPoint,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                cbvav.Name);
            SetupOutputVariable(state,
                                "Unitary System Operating Mode Index",
                                Constant::Units::None,
                                cbvav.HeatCoolMode,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                cbvav.Name);
        }
    }

    void InitCBVAV(EnergyPlusData &state,
                   int const CBVAVNum,            // Index of the current CBVAV unit being simulated
                   bool const FirstHVACIteration, // TRUE if first HVAC iteration
                   int const AirLoopNum,          // air loop index
                   Real64 &OnOffAirFlowRatio,     // Ratio of compressor ON airflow to average airflow over timestep
                   bool const HXUnitOn            // flag to enable heat exchanger
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2006
        //       MODIFIED       B. Griffith, May 2009, EMS setpoint check

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the changeover-bypass VAV system components.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations. The CBVAV system is simulated with no load (coils off) to
        // determine the outlet temperature. A setpoint temperature is calculated on FirstHVACIteration = TRUE.
        // Once the setpoint is calculated, the inlet mass flow rate on FirstHVACIteration = FALSE is used to
        // determine the bypass fraction. The simulation converges quickly on mass flow rate. If the zone
        // temperatures float in the deadband, additional iterations are required to converge on mass flow rate.

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("InitCBVAV");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 QSensUnitOut;         // Output of CBVAV system with coils off
        Real64 OutsideAirMultiplier; // Outside air multiplier schedule (= 1.0 if no schedule)
        Real64 QCoilActual;          // actual CBVAV steam heating coil load met (W)
        bool ErrorFlag;              // local error flag returned from data mining
        Real64 mdot;                 // heating coil fluid mass flow rate, kg/s

        auto &s_node = state.dataLoopNodes;
        
        auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNum);
        int NumCBVAV = state.dataHVACUnitaryBypassVAV->NumCBVAV;

        // Do the one time initializations
        if (state.dataHVACUnitaryBypassVAV->MyOneTimeFlag) {

            state.dataHVACUnitaryBypassVAV->MyEnvrnFlag.allocate(NumCBVAV);
            state.dataHVACUnitaryBypassVAV->MySizeFlag.allocate(NumCBVAV);
            state.dataHVACUnitaryBypassVAV->MyPlantScanFlag.allocate(NumCBVAV);
            state.dataHVACUnitaryBypassVAV->MyEnvrnFlag = true;
            state.dataHVACUnitaryBypassVAV->MySizeFlag = true;
            state.dataHVACUnitaryBypassVAV->MyPlantScanFlag = true;

            state.dataHVACUnitaryBypassVAV->MyOneTimeFlag = false;
            // speed up test based on code from 16 years ago to correct cycling fan economizer defect
            // see https://github.com/NatLabRockies/EnergyPlusArchive/commit/a2202f8a168fd0330bf3a45392833405e8bd08f2
            // This test sets simple flag so air loop doesn't iterate twice each pass (reverts above change)
            // AirLoopControlInfo(AirplantLoc.loopNum).Simple = true;
        }

        if (state.dataHVACUnitaryBypassVAV->MyPlantScanFlag(CBVAVNum) && allocated(state.dataPlnt->PlantLoop)) {
            if ((cbvav.heatCoilType == HVAC::CoilType::HeatingWater) ||
                (cbvav.heatCoilType == HVAC::CoilType::HeatingSteam)) {
                bool ErrorsFound = false; // Set to true if errors in input, fatal at end of routine
                if (cbvav.heatCoilType == HVAC::CoilType::HeatingWater) {

                    ErrorFlag = false;
                    PlantUtilities::ScanPlantLoopsForObject(
                        state, cbvav.HeatCoilName, DataPlant::PlantEquipmentType::CoilWaterSimpleHeating, cbvav.HeatCoilPlantLoc, ErrorFlag, _, _, _, _, _);
                    if (ErrorFlag) {
                        ShowFatalError(state, "InitCBVAV: Program terminated for previous conditions.");
                    }

                    cbvav.MaxHeatCoilFluidFlow = WaterCoils::GetCoilMaxWaterFlowRate(state, cbvav.HeatCoilNum);

                    if (cbvav.MaxHeatCoilFluidFlow > 0.0) {
                        Real64 FluidDensity =
                            state.dataPlnt->PlantLoop(cbvav.HeatCoilPlantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                        cbvav.MaxHeatCoilFluidFlow = WaterCoils::GetCoilMaxWaterFlowRate(state, cbvav.HeatCoilNum) * FluidDensity;
                    }

                } else if (cbvav.heatCoilType == HVAC::CoilType::HeatingSteam) {

                    ErrorFlag = false;
                    PlantUtilities::ScanPlantLoopsForObject(
                        state, cbvav.HeatCoilName, DataPlant::PlantEquipmentType::CoilSteamAirHeating, cbvav.HeatCoilPlantLoc, ErrorFlag, _, _, _, _, _);

                    if (ErrorFlag) {
                        ShowFatalError(state, "InitCBVAV: Program terminated for previous conditions.");
                    }

                    cbvav.MaxHeatCoilFluidFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, cbvav.HeatCoilNum);

                    if (cbvav.MaxHeatCoilFluidFlow > 0.0) {
                        // Why is TempSteamIn a state variable of the entire module?
                        Real64 FluidDensity =
                            Fluid::GetSteam(state)->getSatDensity(state, state.dataHVACUnitaryBypassVAV->TempSteamIn, 1.0, RoutineName);

                        cbvav.MaxHeatCoilFluidFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, cbvav.HeatCoilNum) * FluidDensity;
                    }
                }

                if (ErrorsFound) {
                    ShowContinueError(state, format("Occurs in {} = {}", "AirLoopHVAC:UnitaryHeatCool:VAVChangeoverBypass", cbvav.Name));
                }
                // fill outlet node for heating coil
                cbvav.HeatCoilFluidOutletNode = DataPlant::CompData::getPlantComponent(state, cbvav.HeatCoilPlantLoc).NodeNumOut;
                state.dataHVACUnitaryBypassVAV->MyPlantScanFlag(CBVAVNum) = false;

            } else { // CBVAV is not connected to plant
                state.dataHVACUnitaryBypassVAV->MyPlantScanFlag(CBVAVNum) = false;
            }
        } else if (state.dataHVACUnitaryBypassVAV->MyPlantScanFlag(CBVAVNum) && !state.dataGlobal->AnyPlantInModel) {
            state.dataHVACUnitaryBypassVAV->MyPlantScanFlag(CBVAVNum) = false;
        }

        if (!state.dataGlobal->SysSizingCalc && state.dataHVACUnitaryBypassVAV->MySizeFlag(CBVAVNum)) {
            SizeCBVAV(state, CBVAVNum);
            // Pass the fan cycling schedule index up to the air loop. Set the air loop unitary system flag.
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).cycFanSched = cbvav.fanOpModeSched;
            //   Set UnitarySys flag to FALSE and let the heating coil autosize independently of the cooling coil
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).UnitarySys = false;
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).fanOp = cbvav.fanOp;
            // check for set point manager on outlet node of CBVAV
            cbvav.OutNodeSPMIndex = SetPointManager::GetSetPointManagerIndexByNode(state,
                                                                                   cbvav.AirOutNode,
                                                                                   HVAC::CtrlVarType::Temp,
                                                                                   SetPointManager::SPMType::MixedAir,
                                                                                   true); // isRefNode
            state.dataHVACUnitaryBypassVAV->MySizeFlag(CBVAVNum) = false;
        }

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && state.dataHVACUnitaryBypassVAV->MyEnvrnFlag(CBVAVNum)) {
            Real64 RhoAir = state.dataEnvrn->StdRhoAir;
            // set the mass flow rates from the input volume flow rates
            cbvav.MaxCoolAirMassFlow = RhoAir * cbvav.MaxCoolAirVolFlow;
            cbvav.CoolOutAirMassFlow = RhoAir * cbvav.CoolOutAirVolFlow;
            cbvav.MaxHeatAirMassFlow = RhoAir * cbvav.MaxHeatAirVolFlow;
            cbvav.HeatOutAirMassFlow = RhoAir * cbvav.HeatOutAirVolFlow;
            cbvav.MaxNoCoolHeatAirMassFlow = RhoAir * cbvav.MaxNoCoolHeatAirVolFlow;
            cbvav.NoCoolHeatOutAirMassFlow = RhoAir * cbvav.NoCoolHeatOutAirVolFlow;
            // set the node max and min mass flow rates
            s_node->Node(cbvav.MixerOutsideAirNode).MassFlowRateMax = max(cbvav.CoolOutAirMassFlow, cbvav.HeatOutAirMassFlow);
            s_node->Node(cbvav.MixerOutsideAirNode).MassFlowRateMaxAvail = max(cbvav.CoolOutAirMassFlow, cbvav.HeatOutAirMassFlow);
            s_node->Node(cbvav.MixerOutsideAirNode).MassFlowRateMin = 0.0;
            s_node->Node(cbvav.MixerOutsideAirNode).MassFlowRateMinAvail = 0.0;
            s_node->Node(cbvav.AirInNode).MassFlowRateMax = max(cbvav.MaxCoolAirMassFlow, cbvav.MaxHeatAirMassFlow);
            s_node->Node(cbvav.AirInNode).MassFlowRateMaxAvail = max(cbvav.MaxCoolAirMassFlow, cbvav.MaxHeatAirMassFlow);
            s_node->Node(cbvav.AirInNode).MassFlowRateMin = 0.0;
            s_node->Node(cbvav.AirInNode).MassFlowRateMinAvail = 0.0;
            s_node->Node(cbvav.AirOutNode).Temp = s_node->Node(cbvav.AirInNode).Temp;
            s_node->Node(cbvav.AirOutNode).HumRat = s_node->Node(cbvav.AirInNode).HumRat;
            s_node->Node(cbvav.AirOutNode).Enthalpy = s_node->Node(cbvav.AirInNode).Enthalpy;
            s_node->Node(cbvav.MixerReliefAirNode) = s_node->Node(cbvav.MixerOutsideAirNode);
            state.dataHVACUnitaryBypassVAV->MyEnvrnFlag(CBVAVNum) = false;
            cbvav.LastMode = HeatingMode;
            cbvav.changeOverTimer = -1.0;
            //   set fluid-side hardware limits
            if (cbvav.HeatCoilControlNode > 0) {
                //    If water coil max water flow rate is autosized, simulate once in order to mine max water flow rate
                if (cbvav.MaxHeatCoilFluidFlow == DataSizing::AutoSize) {
                    if (cbvav.heatCoilType == HVAC::CoilType::HeatingWater) {
                        WaterCoils::SimulateWaterCoilComponents(state, cbvav.HeatCoilNum, FirstHVACIteration);
                        Real64 CoilMaxVolFlowRate = WaterCoils::GetCoilMaxWaterFlowRate(state, cbvav.HeatCoilNum);
                        if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                            Real64 FluidDensity =
                                state.dataPlnt->PlantLoop(cbvav.HeatCoilPlantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                            cbvav.MaxHeatCoilFluidFlow = CoilMaxVolFlowRate * FluidDensity;
                        }

                    } else if (cbvav.heatCoilType == HVAC::CoilType::HeatingSteam) {
                        SteamCoils::SimulateSteamCoilComponents(state,
                                                                cbvav.HeatCoilNum,
                                                                FirstHVACIteration,
                                                                1.0,
                                                                QCoilActual); // QCoilReq, simulate any load > 0 to get max capacity of steam coil
                        ErrorFlag = false;
                        Real64 CoilMaxVolFlowRate = SteamCoils::GetCoilMaxSteamFlowRate(state, cbvav.HeatCoilNum);
                        if (ErrorFlag) {
                            ShowContinueError(state, format("Occurs in {} = {}", "AirLoopHVAC:UnitaryHeatCool:VAVChangeoverBypass", cbvav.Name));
                        }
                        if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                            cbvav.MaxHeatCoilFluidFlow = CoilMaxVolFlowRate * Fluid::GetSteam(state)->getSatDensity(state, 100.0, 1.0, RoutineName);
                        }
                    }
                } // end of IF(cBVAV%MaxHeatCoilFluidFlow .EQ. DataSizing::AutoSize)THEN

                PlantUtilities::InitComponentNodes(state, 0.0, cbvav.MaxHeatCoilFluidFlow, cbvav.HeatCoilControlNode, cbvav.HeatCoilFluidOutletNode);

            } // end of IF(cBVAV%CoilControlNode .GT. 0)THEN
        } // end one time inits

        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataHVACUnitaryBypassVAV->MyEnvrnFlag(CBVAVNum) = true;
        }

        // IF CBVAV system was not autosized and the fan is autosized, check that fan volumetric flow rate is greater than CBVAV flow rates
        if (cbvav.CheckFanFlow) {

            if (!state.dataGlobal->DoingSizing && cbvav.FanVolFlow != DataSizing::AutoSize) {
                std::string CurrentModuleObject = "AirLoopHVAC:UnitaryHeatCool:VAVChangeoverBypass";
                //     Check fan versus system supply air flow rates
                if (cbvav.FanVolFlow < cbvav.MaxCoolAirVolFlow) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the maximum CBVAV system air flow rate when "
                                            "cooling is required ({:.7T}).",
                                            CurrentModuleObject,
                                            cbvav.FanVolFlow,
                                            cbvav.FanName,
                                            cbvav.MaxCoolAirVolFlow));
                    ShowContinueError(
                        state, " The CBVAV system flow rate when cooling is required is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state, format(" Occurs in Changeover-bypass VAV system = {}", cbvav.Name));
                    cbvav.MaxCoolAirVolFlow = cbvav.FanVolFlow;
                }
                if (cbvav.FanVolFlow < cbvav.MaxHeatAirVolFlow) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the maximum CBVAV system air flow rate when "
                                            "heating is required ({:.7T}).",
                                            CurrentModuleObject,
                                            cbvav.FanVolFlow,
                                            cbvav.FanName,
                                            cbvav.MaxHeatAirVolFlow));
                    ShowContinueError(
                        state, " The CBVAV system flow rate when heating is required is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state, format(" Occurs in Changeover-bypass VAV system = {}", cbvav.Name));
                    cbvav.MaxHeatAirVolFlow = cbvav.FanVolFlow;
                }
                if (cbvav.FanVolFlow < cbvav.MaxNoCoolHeatAirVolFlow && cbvav.MaxNoCoolHeatAirVolFlow != 0.0) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the maximum CBVAV system air flow rate when "
                                            "no heating or cooling is needed ({:.7T}).",
                                            CurrentModuleObject,
                                            cbvav.FanVolFlow,
                                            cbvav.FanName,
                                            cbvav.MaxNoCoolHeatAirVolFlow));
                    ShowContinueError(state,
                                      " The CBVAV system flow rate when no heating or cooling is needed is reset to the fan flow rate and the "
                                      "simulation continues.");
                    ShowContinueError(state, format(" Occurs in Changeover-bypass VAV system = {}", cbvav.Name));
                    cbvav.MaxNoCoolHeatAirVolFlow = cbvav.FanVolFlow;
                }
                //     Check fan versus outdoor air flow rates
                if (cbvav.FanVolFlow < cbvav.CoolOutAirVolFlow) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the maximum CBVAV outdoor air flow rate when "
                                            "cooling is required ({:.7T}).",
                                            CurrentModuleObject,
                                            cbvav.FanVolFlow,
                                            cbvav.FanName,
                                            cbvav.CoolOutAirVolFlow));
                    ShowContinueError(
                        state, " The CBVAV outdoor flow rate when cooling is required is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state, format(" Occurs in Changeover-bypass VAV system = {}", cbvav.Name));
                    cbvav.CoolOutAirVolFlow = cbvav.FanVolFlow;
                }
                if (cbvav.FanVolFlow < cbvav.HeatOutAirVolFlow) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the maximum CBVAV outdoor air flow rate when "
                                            "heating is required ({:.7T}).",
                                            CurrentModuleObject,
                                            cbvav.FanVolFlow,
                                            cbvav.FanName,
                                            cbvav.HeatOutAirVolFlow));
                    ShowContinueError(
                        state, " The CBVAV outdoor flow rate when heating is required is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state, format(" Occurs in Changeover-bypass VAV system = {}", cbvav.Name));
                    cbvav.HeatOutAirVolFlow = cbvav.FanVolFlow;
                }
                if (cbvav.FanVolFlow < cbvav.NoCoolHeatOutAirVolFlow) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the maximum CBVAV outdoor air flow rate when "
                                            "no heating or cooling is needed ({:.7T}).",
                                            CurrentModuleObject,
                                            cbvav.FanVolFlow,
                                            cbvav.FanName,
                                            cbvav.NoCoolHeatOutAirVolFlow));
                    ShowContinueError(state,
                                      " The CBVAV outdoor flow rate when no heating or cooling is needed is reset to the fan flow rate and the "
                                      "simulation continues.");
                    ShowContinueError(state, format(" Occurs in Changeover-bypass VAV system = {}", cbvav.Name));
                    cbvav.NoCoolHeatOutAirVolFlow = cbvav.FanVolFlow;
                }

                Real64 RhoAir = state.dataEnvrn->StdRhoAir;
                // set the mass flow rates from the reset volume flow rates
                cbvav.MaxCoolAirMassFlow = RhoAir * cbvav.MaxCoolAirVolFlow;
                cbvav.CoolOutAirMassFlow = RhoAir * cbvav.CoolOutAirVolFlow;
                cbvav.MaxHeatAirMassFlow = RhoAir * cbvav.MaxHeatAirVolFlow;
                cbvav.HeatOutAirMassFlow = RhoAir * cbvav.HeatOutAirVolFlow;
                cbvav.MaxNoCoolHeatAirMassFlow = RhoAir * cbvav.MaxNoCoolHeatAirVolFlow;
                cbvav.NoCoolHeatOutAirMassFlow = RhoAir * cbvav.NoCoolHeatOutAirVolFlow;
                // set the node max and min mass flow rates based on reset volume flow rates
                s_node->Node(cbvav.MixerOutsideAirNode).MassFlowRateMax = max(cbvav.CoolOutAirMassFlow, cbvav.HeatOutAirMassFlow);
                s_node->Node(cbvav.MixerOutsideAirNode).MassFlowRateMaxAvail = max(cbvav.CoolOutAirMassFlow, cbvav.HeatOutAirMassFlow);
                s_node->Node(cbvav.MixerOutsideAirNode).MassFlowRateMin = 0.0;
                s_node->Node(cbvav.MixerOutsideAirNode).MassFlowRateMinAvail = 0.0;
                s_node->Node(cbvav.AirInNode).MassFlowRateMax = max(cbvav.MaxCoolAirMassFlow, cbvav.MaxHeatAirMassFlow);
                s_node->Node(cbvav.AirInNode).MassFlowRateMaxAvail = max(cbvav.MaxCoolAirMassFlow, cbvav.MaxHeatAirMassFlow);
                s_node->Node(cbvav.AirInNode).MassFlowRateMin = 0.0;
                s_node->Node(cbvav.AirInNode).MassFlowRateMinAvail = 0.0;
                s_node->Node(cbvav.AirOutNode).Temp = s_node->Node(cbvav.AirInNode).Temp;
                s_node->Node(cbvav.AirOutNode).HumRat = s_node->Node(cbvav.AirInNode).HumRat;
                s_node->Node(cbvav.AirOutNode).Enthalpy = s_node->Node(cbvav.AirInNode).Enthalpy;
                s_node->Node(cbvav.MixerReliefAirNode) = s_node->Node(cbvav.MixerOutsideAirNode);
                cbvav.CheckFanFlow = false;
                if (cbvav.FanVolFlow > 0.0) {
                    cbvav.HeatingSpeedRatio = cbvav.MaxHeatAirVolFlow / cbvav.FanVolFlow;
                    cbvav.CoolingSpeedRatio = cbvav.MaxCoolAirVolFlow / cbvav.FanVolFlow;
                    cbvav.NoHeatCoolSpeedRatio = cbvav.MaxNoCoolHeatAirVolFlow / cbvav.FanVolFlow;
                }
            }
        }

        if (cbvav.fanOpModeSched != nullptr) {
            cbvav.fanOp = (cbvav.fanOpModeSched->getCurrentVal() == 0.0) ? HVAC::FanOp::Cycling : HVAC::FanOp::Continuous;
        }

        // Returns load only for zones requesting cooling (heating). If in deadband, Qzoneload = 0.
        if (FirstHVACIteration) cbvav.modeChanged = false;
        GetZoneLoads(state, CBVAVNum);

        OutsideAirMultiplier = (cbvav.outAirSched != nullptr) ? cbvav.outAirSched->getCurrentVal() : 1.0;

        // Set the inlet node mass flow rate
        if (cbvav.fanOp == HVAC::FanOp::Continuous) {
            // constant fan mode
            if (cbvav.HeatCoolMode == HeatingMode) {
                state.dataHVACUnitaryBypassVAV->CompOnMassFlow = cbvav.MaxHeatAirMassFlow;
                state.dataHVACUnitaryBypassVAV->CompOnFlowRatio = cbvav.HeatingSpeedRatio;
                state.dataHVACUnitaryBypassVAV->OACompOnMassFlow = cbvav.HeatOutAirMassFlow * OutsideAirMultiplier;
            } else if (cbvav.HeatCoolMode == CoolingMode) {
                state.dataHVACUnitaryBypassVAV->CompOnMassFlow = cbvav.MaxCoolAirMassFlow;
                state.dataHVACUnitaryBypassVAV->CompOnFlowRatio = cbvav.CoolingSpeedRatio;
                state.dataHVACUnitaryBypassVAV->OACompOnMassFlow = cbvav.CoolOutAirMassFlow * OutsideAirMultiplier;
            } else {
                state.dataHVACUnitaryBypassVAV->CompOnMassFlow = cbvav.MaxNoCoolHeatAirMassFlow;
                state.dataHVACUnitaryBypassVAV->CompOnFlowRatio = cbvav.NoHeatCoolSpeedRatio;
                state.dataHVACUnitaryBypassVAV->OACompOnMassFlow = cbvav.NoCoolHeatOutAirMassFlow * OutsideAirMultiplier;
            }

            if (cbvav.AirFlowControl == AirFlowCtrlMode::UseCompressorOnFlow) {
                if (cbvav.LastMode == HeatingMode) {
                    state.dataHVACUnitaryBypassVAV->CompOffMassFlow = cbvav.MaxHeatAirMassFlow;
                    state.dataHVACUnitaryBypassVAV->CompOffFlowRatio = cbvav.HeatingSpeedRatio;
                    state.dataHVACUnitaryBypassVAV->OACompOffMassFlow = cbvav.HeatOutAirMassFlow * OutsideAirMultiplier;
                } else {
                    state.dataHVACUnitaryBypassVAV->CompOffMassFlow = cbvav.MaxCoolAirMassFlow;
                    state.dataHVACUnitaryBypassVAV->CompOffFlowRatio = cbvav.CoolingSpeedRatio;
                    state.dataHVACUnitaryBypassVAV->OACompOffMassFlow = cbvav.CoolOutAirMassFlow * OutsideAirMultiplier;
                }
            } else {
                state.dataHVACUnitaryBypassVAV->CompOffMassFlow = cbvav.MaxNoCoolHeatAirMassFlow;
                state.dataHVACUnitaryBypassVAV->CompOffFlowRatio = cbvav.NoHeatCoolSpeedRatio;
                state.dataHVACUnitaryBypassVAV->OACompOffMassFlow = cbvav.NoCoolHeatOutAirMassFlow * OutsideAirMultiplier;
            }
        } else {
            // cycling fan mode
            if (cbvav.HeatCoolMode == HeatingMode) {
                state.dataHVACUnitaryBypassVAV->CompOnMassFlow = cbvav.MaxHeatAirMassFlow;
                state.dataHVACUnitaryBypassVAV->CompOnFlowRatio = cbvav.HeatingSpeedRatio;
                state.dataHVACUnitaryBypassVAV->OACompOnMassFlow = cbvav.HeatOutAirMassFlow * OutsideAirMultiplier;
            } else if (cbvav.HeatCoolMode == CoolingMode) {
                state.dataHVACUnitaryBypassVAV->CompOnMassFlow = cbvav.MaxCoolAirMassFlow;
                state.dataHVACUnitaryBypassVAV->CompOnFlowRatio = cbvav.CoolingSpeedRatio;
                state.dataHVACUnitaryBypassVAV->OACompOnMassFlow = cbvav.CoolOutAirMassFlow * OutsideAirMultiplier;
            } else {
                state.dataHVACUnitaryBypassVAV->CompOnMassFlow = cbvav.MaxCoolAirMassFlow;
                state.dataHVACUnitaryBypassVAV->CompOnFlowRatio = cbvav.CoolingSpeedRatio;
                state.dataHVACUnitaryBypassVAV->OACompOnMassFlow = cbvav.CoolOutAirMassFlow * OutsideAirMultiplier;
            }
            state.dataHVACUnitaryBypassVAV->CompOffMassFlow = 0.0;
            state.dataHVACUnitaryBypassVAV->CompOffFlowRatio = 0.0;
            state.dataHVACUnitaryBypassVAV->OACompOffMassFlow = 0.0;
        }

        // Check for correct control node at outlet of unit
        if (cbvav.HumRatMaxCheck) {
            if (cbvav.DehumidControlType != DehumidControl::None) {
                if (s_node->Node(cbvav.AirOutNode).HumRatMax == DataLoopNode::SensedNodeFlagValue) {
                    if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                        ShowWarningError(state, format("Unitary System:VAV:ChangeOverBypass = {}", cbvav.Name));
                        ShowContinueError(state,
                                          "Use SetpointManager:SingleZone:Humidity:Maximum to place a humidity setpoint at the air outlet node of "
                                          "the unitary system.");
                        ShowContinueError(state, "Setting Dehumidification Control Type to None and simulation continues.");
                        cbvav.DehumidControlType = DehumidControl::None;
                    } else {
                        // need call to EMS to check node
                        bool EMSSetPointCheck = false;
                        EMSManager::CheckIfNodeSetPointManagedByEMS(state, cbvav.AirOutNode, HVAC::CtrlVarType::MaxHumRat, EMSSetPointCheck);
                        s_node->NodeSetpointCheck(cbvav.AirOutNode).needsSetpointChecking = false;
                        if (EMSSetPointCheck) {
                            // There is no plugin anyways, so we now we have a bad condition.
                            ShowWarningError(state, format("Unitary System:VAV:ChangeOverBypass = {}", cbvav.Name));
                            ShowContinueError(state,
                                              "Use SetpointManager:SingleZone:Humidity:Maximum to place a humidity setpoint at the air outlet node "
                                              "of the unitary system.");
                            ShowContinueError(
                                state, "Or use an EMS Actuator to place a maximum humidity setpoint at the air outlet node of the unitary system.");
                            ShowContinueError(state, "Setting Dehumidification Control Type to None and simulation continues.");
                            cbvav.DehumidControlType = DehumidControl::None;
                        }
                    }
                }
                cbvav.HumRatMaxCheck = false;
            } else {
                cbvav.HumRatMaxCheck = false;
            }
        }

        // Set the inlet node mass flow rate
        if (cbvav.availSched->getCurrentVal() > 0.0 && state.dataHVACUnitaryBypassVAV->CompOnMassFlow != 0.0) {
            OnOffAirFlowRatio = 1.0;
            if (FirstHVACIteration) {
                s_node->Node(cbvav.AirInNode).MassFlowRate = state.dataHVACUnitaryBypassVAV->CompOnMassFlow;
                s_node->Node(cbvav.MixerInletAirNode).MassFlowRate = state.dataHVACUnitaryBypassVAV->CompOnMassFlow;
                s_node->Node(cbvav.MixerOutsideAirNode).MassFlowRate = state.dataHVACUnitaryBypassVAV->OACompOnMassFlow;
                s_node->Node(cbvav.MixerReliefAirNode).MassFlowRate = state.dataHVACUnitaryBypassVAV->OACompOnMassFlow;
                state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction = 0.0;
                state.dataHVACUnitaryBypassVAV->PartLoadFrac = 0.0;
            } else {
                if (cbvav.HeatCoolMode != 0) {
                    state.dataHVACUnitaryBypassVAV->PartLoadFrac = 1.0;
                } else {
                    state.dataHVACUnitaryBypassVAV->PartLoadFrac = 0.0;
                }
                if (cbvav.fanOp == HVAC::FanOp::Cycling) {
                    state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction = 0.0;
                } else {
                    if (cbvav.PlenumMixerInletAirNode == 0) {
                        state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction = max(
                            0.0, 1.0 - (s_node->Node(cbvav.AirInNode).MassFlowRate / state.dataHVACUnitaryBypassVAV->CompOnMassFlow));
                    }
                }
            }
        } else {
            state.dataHVACUnitaryBypassVAV->PartLoadFrac = 0.0;
            s_node->Node(cbvav.AirInNode).MassFlowRate = 0.0;
            s_node->Node(cbvav.AirOutNode).MassFlowRate = 0.0;
            s_node->Node(cbvav.AirOutNode).MassFlowRateMaxAvail = 0.0;

            s_node->Node(cbvav.MixerInletAirNode).MassFlowRate = 0.0;
            s_node->Node(cbvav.MixerOutsideAirNode).MassFlowRate = 0.0;
            s_node->Node(cbvav.MixerReliefAirNode).MassFlowRate = 0.0;

            OnOffAirFlowRatio = 1.0;
            state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction = 0.0;
        }

        CalcCBVAV(state, CBVAVNum, FirstHVACIteration, state.dataHVACUnitaryBypassVAV->PartLoadFrac, QSensUnitOut, OnOffAirFlowRatio, HXUnitOn);

        // If unit is scheduled OFF, setpoint is equal to inlet node temperature.
        if (cbvav.availSched->getCurrentVal() == 0.0) {
            cbvav.OutletTempSetPoint = s_node->Node(cbvav.AirInNode).Temp;
            return;
        }

        SetAverageAirFlow(state, CBVAVNum, OnOffAirFlowRatio);

        if (FirstHVACIteration) cbvav.OutletTempSetPoint = CalcSetPointTempTarget(state, CBVAVNum);

        // The setpoint is used to control the DX coils at their respective outlet nodes (not the unit outlet), correct
        // for fan heat for draw thru units only (fan heat is included at the outlet of each coil when blowthru is used)
        cbvav.CoilTempSetPoint = cbvav.OutletTempSetPoint;
        if (cbvav.fanPlace == HVAC::FanPlace::DrawThru) {
            cbvav.CoilTempSetPoint -= (s_node->Node(cbvav.AirOutNode).Temp - s_node->Node(cbvav.FanInletNodeNum).Temp);
        }

        if (FirstHVACIteration) {
            if (cbvav.heatCoilType == HVAC::CoilType::HeatingWater) {
                WaterCoils::SimulateWaterCoilComponents(state, cbvav.HeatCoilNum, FirstHVACIteration);

                //     set air-side and steam-side mass flow rates
                s_node->Node(cbvav.HeatCoilAirInletNode).MassFlowRate = state.dataHVACUnitaryBypassVAV->CompOnMassFlow;
                mdot = cbvav.MaxHeatCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(state, mdot, cbvav.HeatCoilControlNode, cbvav.HeatCoilFluidOutletNode, cbvav.HeatCoilPlantLoc);

                //     simulate water coil to find operating capacity
                WaterCoils::SimulateWaterCoilComponents(state, cbvav.HeatCoilNum, FirstHVACIteration, QCoilActual);
                cbvav.DesignSuppHeatingCapacity = QCoilActual;

            } else if (cbvav.heatCoilType == HVAC::CoilType::HeatingSteam) {

                //     set air-side and steam-side mass flow rates
                s_node->Node(cbvav.HeatCoilAirInletNode).MassFlowRate = state.dataHVACUnitaryBypassVAV->CompOnMassFlow;
                mdot = cbvav.MaxHeatCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(state, mdot, cbvav.HeatCoilControlNode, cbvav.HeatCoilFluidOutletNode, cbvav.HeatCoilPlantLoc);

                //     simulate steam coil to find operating capacity
                SteamCoils::SimulateSteamCoilComponents(state,
                                                        cbvav.HeatCoilNum,
                                                        FirstHVACIteration,
                                                        1.0,
                                                        QCoilActual); // QCoilReq, simulate any load > 0 to get max capacity of steam coil
                cbvav.DesignSuppHeatingCapacity = QCoilActual;

            } // from IF(cBVAV%HeatCoilType == HVAC::Coil_HeatingSteam) THEN
        } // from IF( FirstHVACIteration ) THEN

        if ((cbvav.HeatCoolMode == 0 && cbvav.fanOp == HVAC::FanOp::Cycling) || state.dataHVACUnitaryBypassVAV->CompOnMassFlow == 0.0) {
            state.dataHVACUnitaryBypassVAV->PartLoadFrac = 0.0;
            s_node->Node(cbvav.AirInNode).MassFlowRate = 0.0;
            s_node->Node(cbvav.AirOutNode).MassFlowRateMaxAvail = 0.0;
            s_node->Node(cbvav.MixerInletAirNode).MassFlowRate = 0.0;
            s_node->Node(cbvav.MixerOutsideAirNode).MassFlowRate = 0.0;
            s_node->Node(cbvav.MixerReliefAirNode).MassFlowRate = 0.0;
        }
    }

    void SizeCBVAV(EnergyPlusData &state, int const CBVAVNum) // Index to CBVAV system
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2006

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing changeover-bypass VAV components.

        // METHODOLOGY EMPLOYED:
        // Obtains flow rates from the zone sizing arrays.

        int curSysNum = state.dataSize->CurSysNum;
        int curOASysNum = state.dataSize->CurOASysNum;

        auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNum);

        if (curSysNum > 0 && curOASysNum == 0) {
            state.dataAirSystemsData->PrimaryAirSystems(curSysNum).supFanNum = cbvav.FanIndex;
            state.dataAirSystemsData->PrimaryAirSystems(curSysNum).supFanType = cbvav.fanType;
            state.dataAirSystemsData->PrimaryAirSystems(curSysNum).supFanPlace = cbvav.fanPlace;
        }

        if (cbvav.MaxCoolAirVolFlow == DataSizing::AutoSize) {

            if (curSysNum > 0) {

                CheckSysSizing(state, cbvav.UnitType, cbvav.Name);
                cbvav.MaxCoolAirVolFlow = state.dataSize->FinalSysSizing(curSysNum).DesMainVolFlow;
                if (cbvav.FanVolFlow < cbvav.MaxCoolAirVolFlow && cbvav.FanVolFlow != DataSizing::AutoSize) {
                    cbvav.MaxCoolAirVolFlow = cbvav.FanVolFlow;
                    ShowWarningError(state, format("{} \"{}\"", cbvav.UnitType, cbvav.Name));
                    ShowContinueError(state,
                                      "The CBVAV system supply air fan air flow rate is less than the autosized value for the maximum air flow rate "
                                      "in cooling mode. Consider autosizing the fan for this simulation.");
                    ShowContinueError(
                        state, "The maximum air flow rate in cooling mode is reset to the supply air fan flow rate and the simulation continues.");
                }
                if (cbvav.MaxCoolAirVolFlow < HVAC::SmallAirVolFlow) {
                    cbvav.MaxCoolAirVolFlow = 0.0;
                }
                BaseSizer::reportSizerOutput(state, cbvav.UnitType, cbvav.Name, "maximum cooling air flow rate [m3/s]", cbvav.MaxCoolAirVolFlow);
            }
        }

        if (cbvav.MaxHeatAirVolFlow == DataSizing::AutoSize) {

            if (curSysNum > 0) {

                CheckSysSizing(state, cbvav.UnitType, cbvav.Name);
                cbvav.MaxHeatAirVolFlow = state.dataSize->FinalSysSizing(curSysNum).DesMainVolFlow;
                if (cbvav.FanVolFlow < cbvav.MaxHeatAirVolFlow && cbvav.FanVolFlow != DataSizing::AutoSize) {
                    cbvav.MaxHeatAirVolFlow = cbvav.FanVolFlow;
                    ShowWarningError(state, format("{} \"{}\"", cbvav.UnitType, cbvav.Name));
                    ShowContinueError(state,
                                      "The CBVAV system supply air fan air flow rate is less than the autosized value for the maximum air flow rate "
                                      "in heating mode. Consider autosizing the fan for this simulation.");
                    ShowContinueError(
                        state, "The maximum air flow rate in heating mode is reset to the supply air fan flow rate and the simulation continues.");
                }
                if (cbvav.MaxHeatAirVolFlow < HVAC::SmallAirVolFlow) {
                    cbvav.MaxHeatAirVolFlow = 0.0;
                }
                BaseSizer::reportSizerOutput(state, cbvav.UnitType, cbvav.Name, "maximum heating air flow rate [m3/s]", cbvav.MaxHeatAirVolFlow);
            }
        }

        if (cbvav.MaxNoCoolHeatAirVolFlow == DataSizing::AutoSize) {

            if (curSysNum > 0) {

                CheckSysSizing(state, cbvav.UnitType, cbvav.Name);
                cbvav.MaxNoCoolHeatAirVolFlow = state.dataSize->FinalSysSizing(curSysNum).DesMainVolFlow;
                if (cbvav.FanVolFlow < cbvav.MaxNoCoolHeatAirVolFlow && cbvav.FanVolFlow != DataSizing::AutoSize) {
                    cbvav.MaxNoCoolHeatAirVolFlow = cbvav.FanVolFlow;
                    ShowWarningError(state, format("{} \"{}\"", cbvav.UnitType, cbvav.Name));
                    ShowContinueError(state,
                                      "The CBVAV system supply air fan air flow rate is less than the autosized value for the maximum air flow rate "
                                      "when no heating or cooling is needed. Consider autosizing the fan for this simulation.");
                    ShowContinueError(state,
                                      "The maximum air flow rate when no heating or cooling is needed is reset to the supply air fan flow rate and "
                                      "the simulation continues.");
                }
                if (cbvav.MaxNoCoolHeatAirVolFlow < HVAC::SmallAirVolFlow) {
                    cbvav.MaxNoCoolHeatAirVolFlow = 0.0;
                }

                BaseSizer::reportSizerOutput(
                    state, cbvav.UnitType, cbvav.Name, "maximum air flow rate when compressor/coil is off [m3/s]", cbvav.MaxNoCoolHeatAirVolFlow);
            }
        }

        if (cbvav.CoolOutAirVolFlow == DataSizing::AutoSize) {

            if (curSysNum > 0) {

                CheckSysSizing(state, cbvav.UnitType, cbvav.Name);
                cbvav.CoolOutAirVolFlow = state.dataSize->FinalSysSizing(curSysNum).DesOutAirVolFlow;
                if (cbvav.FanVolFlow < cbvav.CoolOutAirVolFlow && cbvav.FanVolFlow != DataSizing::AutoSize) {
                    cbvav.CoolOutAirVolFlow = cbvav.FanVolFlow;
                    ShowWarningError(state, format("{} \"{}\"", cbvav.UnitType, cbvav.Name));
                    ShowContinueError(state,
                                      "The CBVAV system supply air fan air flow rate is less than the autosized value for the outdoor air flow rate "
                                      "in cooling mode. Consider autosizing the fan for this simulation.");
                    ShowContinueError(
                        state, "The outdoor air flow rate in cooling mode is reset to the supply air fan flow rate and the simulation continues.");
                }
                if (cbvav.CoolOutAirVolFlow < HVAC::SmallAirVolFlow) {
                    cbvav.CoolOutAirVolFlow = 0.0;
                }
                BaseSizer::reportSizerOutput(
                    state, cbvav.UnitType, cbvav.Name, "maximum outside air flow rate in cooling [m3/s]", cbvav.CoolOutAirVolFlow);
            }
        }

        if (cbvav.HeatOutAirVolFlow == DataSizing::AutoSize) {

            if (curSysNum > 0) {

                CheckSysSizing(state, cbvav.UnitType, cbvav.Name);
                cbvav.HeatOutAirVolFlow = state.dataSize->FinalSysSizing(curSysNum).DesOutAirVolFlow;
                if (cbvav.FanVolFlow < cbvav.HeatOutAirVolFlow && cbvav.FanVolFlow != DataSizing::AutoSize) {
                    cbvav.HeatOutAirVolFlow = cbvav.FanVolFlow;
                    ShowContinueError(state,
                                      "The CBVAV system supply air fan air flow rate is less than the autosized value for the outdoor air flow rate "
                                      "in heating mode. Consider autosizing the fan for this simulation.");
                    ShowContinueError(
                        state, "The outdoor air flow rate in heating mode is reset to the supply air fan flow rate and the simulation continues.");
                }
                if (cbvav.HeatOutAirVolFlow < HVAC::SmallAirVolFlow) {
                    cbvav.HeatOutAirVolFlow = 0.0;
                }
                BaseSizer::reportSizerOutput(
                    state, cbvav.UnitType, cbvav.Name, "maximum outdoor air flow rate in heating [m3/s]", cbvav.CoolOutAirVolFlow);
            }
        }

        if (cbvav.NoCoolHeatOutAirVolFlow == DataSizing::AutoSize) {

            if (curSysNum > 0) {

                CheckSysSizing(state, cbvav.UnitType, cbvav.Name);
                cbvav.NoCoolHeatOutAirVolFlow = state.dataSize->FinalSysSizing(curSysNum).DesOutAirVolFlow;
                if (cbvav.FanVolFlow < cbvav.NoCoolHeatOutAirVolFlow && cbvav.FanVolFlow != DataSizing::AutoSize) {
                    cbvav.NoCoolHeatOutAirVolFlow = cbvav.FanVolFlow;
                    ShowContinueError(state,
                                      "The CBVAV system supply air fan air flow rate is less than the autosized value for the outdoor air flow rate "
                                      "when no heating or cooling is needed. Consider autosizing the fan for this simulation.");
                    ShowContinueError(state,
                                      "The outdoor air flow rate when no heating or cooling is needed is reset to the supply air fan flow rate and "
                                      "the simulation continues.");
                }
                if (cbvav.NoCoolHeatOutAirVolFlow < HVAC::SmallAirVolFlow) {
                    cbvav.NoCoolHeatOutAirVolFlow = 0.0;
                }
                BaseSizer::reportSizerOutput(
                    state, cbvav.UnitType, cbvav.Name, "maximum outdoor air flow rate when compressor is off [m3/s]", cbvav.NoCoolHeatOutAirVolFlow);
            }
        }
    }

    void ControlCBVAVOutput(EnergyPlusData &state,
                            int const CBVAVNum,            // Index to CBVAV system
                            bool const FirstHVACIteration, // Flag for 1st HVAC iteration
                            Real64 &PartLoadFrac,          // Unit part load fraction
                            Real64 &OnOffAirFlowRatio,     // Ratio of compressor ON airflow to AVERAGE airflow over timestep
                            bool const HXUnitOn            // flag to enable heat exchanger
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2006

        // PURPOSE OF THIS SUBROUTINE:
        // Determine the part load fraction of the CBVAV system for this time step.

        // METHODOLOGY EMPLOYED:
        // Use RegulaFalsi technique to iterate on part-load ratio until convergence is achieved.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 FullOutput = 0; // Unit full output when compressor is operating [W]
        PartLoadFrac = 0.0;

        auto &s_node = state.dataLoopNodes;
        auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNum);

        if (cbvav.availSched->getCurrentVal() == 0.0) return;

        // Get operating result
        PartLoadFrac = 1.0;
        CalcCBVAV(state, CBVAVNum, FirstHVACIteration, PartLoadFrac, FullOutput, OnOffAirFlowRatio, HXUnitOn);

        if ((s_node->Node(cbvav.AirOutNode).Temp - cbvav.OutletTempSetPoint) > HVAC::SmallTempDiff && cbvav.HeatCoolMode > 0 &&
            PartLoadFrac < 1.0) {
            CalcCBVAV(state, CBVAVNum, FirstHVACIteration, PartLoadFrac, FullOutput, OnOffAirFlowRatio, HXUnitOn);
        }
    }

    void CalcCBVAV(EnergyPlusData &state,
                   int const CBVAVNum,            // Unit index in fan coil array
                   bool const FirstHVACIteration, // Flag for 1st HVAC iteration
                   Real64 &PartLoadFrac,          // Compressor part load fraction
                   Real64 &LoadMet,               // Load met by unit (W)
                   Real64 &OnOffAirFlowRatio,     // Ratio of compressor ON airflow to AVERAGE airflow over timestep
                   bool const HXUnitOn            // flag to enable heat exchanger
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2006

        // PURPOSE OF THIS SUBROUTINE:
        // Simulate the components making up the changeover-bypass VAV system.

        // METHODOLOGY EMPLOYED:
        // Simulates the unit components sequentially in the air flow direction.

        // SUBROUTINE PARAMETER DEFINITIONS:
        int constexpr MaxIte(500); // Maximum number of iterations

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 MinHumRat;     // Minimum humidity ratio for sensible capacity calculation (kg/kg)
        int SolFla;           // Flag of RegulaFalsi solver
        Real64 QHeater;       // Load to be met by heater [W]
        Real64 QHeaterActual; // actual heating load met [W]
        Real64 CpAir;         // Specific heat of air [J/kg-K]
        Real64 ApproachTemp;
        Real64 DesiredDewPoint;
        Real64 OutdoorDryBulbTemp; // Dry-bulb temperature at outdoor condenser
        Real64 OutdoorBaroPress;   // Barometric pressure at outdoor condenser

        auto &s_node = state.dataLoopNodes;
        auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNum);

        int OutletNode = cbvav.AirOutNode;
        int InletNode = cbvav.AirInNode;
        if (cbvav.CondenserNodeNum > 0) {
            OutdoorDryBulbTemp = s_node->Node(cbvav.CondenserNodeNum).Temp;
            OutdoorBaroPress = s_node->Node(cbvav.CondenserNodeNum).Press;
        } else {
            OutdoorDryBulbTemp = state.dataEnvrn->OutDryBulbTemp;
            OutdoorBaroPress = state.dataEnvrn->OutBaroPress;
        }

        state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = 0.0;

        // Bypass excess system air through bypass duct and calculate new mixed air conditions at OA mixer inlet node
        if (cbvav.plenumIndex > 0 || cbvav.mixerIndex > 0) {
            Real64 saveMixerInletAirNodeFlow = s_node->Node(cbvav.MixerInletAirNode).MassFlowRate;
            s_node->Node(cbvav.MixerInletAirNode) = s_node->Node(InletNode);
            s_node->Node(cbvav.MixerInletAirNode).MassFlowRate = saveMixerInletAirNodeFlow;
        } else {
            s_node->Node(cbvav.MixerInletAirNode).Temp =
                (1.0 - state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction) * s_node->Node(InletNode).Temp +
                state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction * s_node->Node(OutletNode).Temp;
            s_node->Node(cbvav.MixerInletAirNode).HumRat =
                (1.0 - state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction) * s_node->Node(InletNode).HumRat +
                state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction * s_node->Node(OutletNode).HumRat;
            s_node->Node(cbvav.MixerInletAirNode).Enthalpy = Psychrometrics::PsyHFnTdbW(
                s_node->Node(cbvav.MixerInletAirNode).Temp, s_node->Node(cbvav.MixerInletAirNode).HumRat);
        }
        MixedAir::SimOAMixer(state, cbvav.OAMixName, cbvav.OAMixIndex);

        if (cbvav.fanPlace == HVAC::FanPlace::BlowThru) {
            state.dataFans->fans(cbvav.FanIndex)
                ->simulate(state, FirstHVACIteration, state.dataHVACUnitaryBypassVAV->FanSpeedRatio, _, 1.0 / OnOffAirFlowRatio);
        }
        // Simulate cooling coil if zone load is negative (cooling load)
        if (cbvav.HeatCoolMode == CoolingMode) {
            if (OutdoorDryBulbTemp >= cbvav.MinOATCompressor) {
                if (cbvav.coolCoilType == HVAC::CoilType::CoolingDXHXAssisted) {
                    HXAssistCoil::SimHXAssistedCoolingCoil(state,
                                                           cbvav.CoolCoilNum,
                                                           FirstHVACIteration,
                                                           HVAC::CompressorOp::On,
                                                           PartLoadFrac,
                                                           HVAC::FanOp::Continuous,
                                                           HXUnitOn);
                    if (s_node->Node(cbvav.CoolCoilAirInletNode).Temp <= cbvav.CoilTempSetPoint) {
                        //         If coil inlet temp is already below the setpoint, simulated with coil off
                        PartLoadFrac = 0.0;
                        HXAssistCoil::SimHXAssistedCoolingCoil(state,
                                                               cbvav.CoolCoilNum,
                                                               FirstHVACIteration,
                                                               HVAC::CompressorOp::Off,
                                                               PartLoadFrac,
                                                               HVAC::FanOp::Continuous,
                                                               HXUnitOn);

                    } else if (s_node->Node(cbvav.CoolCoilAirOutletNode).Temp < cbvav.CoilTempSetPoint) {
                        auto f = [&state, CBVAVNum, FirstHVACIteration, HXUnitOn](Real64 const PartLoadFrac) {
                            auto &s_node = state.dataLoopNodes;
                            auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNum);
                            HXAssistCoil::SimHXAssistedCoolingCoil(state,
                                                                   cbvav.CoolCoilNum,
                                                                   FirstHVACIteration,
                                                                   HVAC::CompressorOp::On,
                                                                   PartLoadFrac,
                                                                   HVAC::FanOp::Continuous,
                                                                   HXUnitOn);

                            Real64 OutletAirTemp = s_node->Node(cbvav.CoolCoilAirOutletNode).Temp;
                            return cbvav.CoilTempSetPoint - OutletAirTemp;
                        };
                        General::SolveRoot(state, HVAC::SmallTempDiff, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                        HXAssistCoil::SimHXAssistedCoolingCoil(state,
                                                               cbvav.CoolCoilNum,
                                                               FirstHVACIteration,
                                                               HVAC::CompressorOp::On,
                                                               PartLoadFrac,
                                                               HVAC::FanOp::Continuous,
                                                               HXUnitOn);
                        if (SolFla == -1 && !state.dataGlobal->WarmupFlag) {
                            if (cbvav.HXDXIterationExceeded < 1) {
                                ++cbvav.HXDXIterationExceeded;
                                ShowWarningError(state,
                                                 format("Iteration limit exceeded calculating HX assisted DX unit part-load ratio, for unit = {}",
                                                        cbvav.CoolCoilName));
                                ShowContinueError(state, format("Calculated part-load ratio = {:.3R}", PartLoadFrac));
                                ShowContinueErrorTimeStamp(
                                    state, "The calculated part-load ratio will be used and the simulation continues. Occurrence info:");
                            } else {
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    cbvav.Name + ", Iteration limit exceeded for HX assisted DX unit part-load ratio error continues.",
                                    cbvav.HXDXIterationExceededIndex,
                                    PartLoadFrac,
                                    PartLoadFrac);
                            }
                        } else if (SolFla == -2 && !state.dataGlobal->WarmupFlag) {
                            PartLoadFrac = max(0.0,
                                               min(1.0,
                                                   (s_node->Node(cbvav.CoolCoilAirInletNode).Temp - cbvav.CoilTempSetPoint) /
                                                       (s_node->Node(cbvav.CoolCoilAirInletNode).Temp -
                                                        s_node->Node(cbvav.CoolCoilAirOutletNode).Temp)));
                            if (cbvav.HXDXIterationFailed < 1) {
                                ++cbvav.HXDXIterationFailed;
                                ShowSevereError(
                                    state,
                                    format("HX assisted DX unit part-load ratio calculation failed: part-load ratio limits exceeded, for unit = {}",
                                           cbvav.CoolCoilName));
                                ShowContinueErrorTimeStamp(
                                    state,
                                    format("An estimated part-load ratio of {:.3R}will be used and the simulation continues. Occurrence info:",
                                           PartLoadFrac));
                            } else {
                                ShowRecurringWarningErrorAtEnd(state,
                                                               cbvav.Name +
                                                                   ", Part-load ratio calculation failed for HX assisted DX unit error continues.",
                                                               cbvav.HXDXIterationFailedIndex,
                                                               PartLoadFrac,
                                                               PartLoadFrac);
                            }
                        }
                    }
                    
                } else if (cbvav.coolCoilType == HVAC::CoilType::CoolingDXSingleSpeed) {
                    DXCoils::SimDXCoil(state,
                                       cbvav.CoolCoilNum,
                                       HVAC::CompressorOp::On,
                                       FirstHVACIteration,
                                       HVAC::FanOp::Continuous,
                                       PartLoadFrac,
                                       OnOffAirFlowRatio);
                    if (s_node->Node(cbvav.CoolCoilAirInletNode).Temp <= cbvav.CoilTempSetPoint) {
                        //         If coil inlet temp is already below the setpoint, simulated with coil off
                        PartLoadFrac = 0.0;
                        DXCoils::SimDXCoil(state,
                                           cbvav.CoolCoilNum,
                                           HVAC::CompressorOp::On,
                                           FirstHVACIteration,
                                           HVAC::FanOp::Continuous,
                                           PartLoadFrac,
                                           OnOffAirFlowRatio);
                    } else if (s_node->Node(cbvav.CoolCoilAirOutletNode).Temp < cbvav.CoilTempSetPoint) {
                        auto f = [&state, CBVAVNum, OnOffAirFlowRatio](Real64 const PartLoadFrac) {
                            auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNum);
                            DXCoils::CalcDoe2DXCoil(state,
                                                    cbvav.CoolCoilNum,
                                                    HVAC::CompressorOp::On,
                                                    false,
                                                    PartLoadFrac,
                                                    HVAC::FanOp::Continuous,
                                                    _,
                                                    OnOffAirFlowRatio);
                            Real64 OutletAirTemp = state.dataDXCoils->DXCoilOutletTemp(cbvav.CoolCoilNum);
                            return cbvav.CoilTempSetPoint - OutletAirTemp;
                        };
                        General::SolveRoot(state, HVAC::SmallTempDiff, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                        DXCoils::SimDXCoil(state,
                                           cbvav.CoolCoilNum,
                                           HVAC::CompressorOp::On,
                                           FirstHVACIteration,
                                           HVAC::FanOp::Continuous,
                                           PartLoadFrac,
                                           OnOffAirFlowRatio);
                        if (SolFla == -1 && !state.dataGlobal->WarmupFlag) {
                            if (cbvav.DXIterationExceeded < 1) {
                                ++cbvav.DXIterationExceeded;
                                ShowWarningError(
                                    state,
                                    format("Iteration limit exceeded calculating DX unit part-load ratio, for unit = {}", cbvav.CoolCoilName));
                                ShowContinueError(state, format("Calculated part-load ratio = {:.3R}", PartLoadFrac));
                                ShowContinueErrorTimeStamp(
                                    state, "The calculated part-load ratio will be used and the simulation continues. Occurrence info:");
                            } else {
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    cbvav.Name + ", Iteration limit exceeded for DX unit part-load ratio calculation error continues.",
                                    cbvav.DXIterationExceededIndex,
                                    PartLoadFrac,
                                    PartLoadFrac);
                            }
                        } else if (SolFla == -2 && !state.dataGlobal->WarmupFlag) {
                            PartLoadFrac = max(0.0,
                                               min(1.0,
                                                   (s_node->Node(cbvav.CoolCoilAirInletNode).Temp - cbvav.CoilTempSetPoint) /
                                                       (s_node->Node(cbvav.CoolCoilAirInletNode).Temp -
                                                        s_node->Node(cbvav.CoolCoilAirOutletNode).Temp)));
                            if (cbvav.DXIterationFailed < 1) {
                                ++cbvav.DXIterationFailed;
                                ShowSevereError(state,
                                                format("DX unit part-load ratio calculation failed: part-load ratio limits exceeded, for unit = {}",
                                                       cbvav.CoolCoilName));
                                ShowContinueErrorTimeStamp(
                                    state,
                                    format("An estimated part-load ratio of {:.3R}will be used and the simulation continues. Occurrence info:",
                                           PartLoadFrac));
                            } else {
                                ShowRecurringWarningErrorAtEnd(state,
                                                               cbvav.Name + ", Part-load ratio calculation failed for DX unit error continues.",
                                                               cbvav.DXIterationFailedIndex,
                                                               PartLoadFrac,
                                                               PartLoadFrac);
                            }
                        }
                    }
                    state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(cbvav.CoolCoilNum);

                } else if (cbvav.coolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
                    Real64 QZnReq(0.0);                 // Zone load (W), input to variable-speed DX coil
                    Real64 QLatReq(0.0);                // Zone latent load, input to variable-speed DX coil
                    Real64 LocalOnOffAirFlowRatio(1.0); // ratio of compressor on flow to average flow over time step
                    Real64 LocalPartLoadFrac(0.0);
                    Real64 SpeedRatio(0.0);
                    int SpeedNum(1);
                    int maxNumSpeeds = VariableSpeedCoils::GetCoilNumOfSpeeds(state, cbvav.CoolCoilNum);
                    Real64 DesOutTemp = cbvav.CoilTempSetPoint;
                    // Get no load result
                    VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                              cbvav.CoolCoilNum,
                                                              HVAC::FanOp::Continuous,
                                                              HVAC::CompressorOp::Off,
                                                              LocalPartLoadFrac,
                                                              SpeedNum,
                                                              SpeedRatio,
                                                              QZnReq,
                                                              QLatReq);

                    Real64 NoOutput = s_node->Node(cbvav.CoolCoilAirInletNode).MassFlowRate *
                                      (Psychrometrics::PsyHFnTdbW(s_node->Node(cbvav.CoolCoilAirOutletNode).Temp,
                                                                  s_node->Node(cbvav.CoolCoilAirOutletNode).HumRat) -
                                       Psychrometrics::PsyHFnTdbW(s_node->Node(cbvav.CoolCoilAirInletNode).Temp,
                                                                  s_node->Node(cbvav.CoolCoilAirOutletNode).HumRat));

                    // Get full load result
                    LocalPartLoadFrac = 1.0;
                    SpeedNum = maxNumSpeeds;
                    SpeedRatio = 1.0;
                    QZnReq = 0.001; // to indicate the coil is running
                    VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                              cbvav.CoolCoilNum,
                                                              HVAC::FanOp::Continuous,
                                                              HVAC::CompressorOp::On,
                                                              LocalPartLoadFrac,
                                                              SpeedNum,
                                                              SpeedRatio,
                                                              QZnReq,
                                                              QLatReq);

                    Real64 FullOutput = s_node->Node(cbvav.CoolCoilAirInletNode).MassFlowRate *
                                        (Psychrometrics::PsyHFnTdbW(s_node->Node(cbvav.CoolCoilAirOutletNode).Temp,
                                                                    s_node->Node(cbvav.CoolCoilAirOutletNode).HumRat) -
                                         Psychrometrics::PsyHFnTdbW(s_node->Node(cbvav.CoolCoilAirInletNode).Temp,
                                                                    s_node->Node(cbvav.CoolCoilAirOutletNode).HumRat));
                    Real64 ReqOutput = s_node->Node(cbvav.CoolCoilAirInletNode).MassFlowRate *
                                       (Psychrometrics::PsyHFnTdbW(DesOutTemp, s_node->Node(cbvav.CoolCoilAirOutletNode).HumRat) -
                                        Psychrometrics::PsyHFnTdbW(s_node->Node(cbvav.CoolCoilAirInletNode).Temp,
                                                                   s_node->Node(cbvav.CoolCoilAirOutletNode).HumRat));

                    Real64 loadAccuracy(0.001);                  // Watts, power
                    Real64 tempAccuracy(0.001);                  // delta C, temperature
                    if ((NoOutput - ReqOutput) < loadAccuracy) { //         IF NoOutput is lower than (more cooling than required) or very near
                                                                 //         the ReqOutput, do not run the compressor
                        LocalPartLoadFrac = 0.0;
                        SpeedNum = 1;
                        SpeedRatio = 0.0;
                        QZnReq = 0.0;
                        // Get no load result
                        VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                  cbvav.CoolCoilNum,
                                                                  HVAC::FanOp::Continuous,
                                                                  HVAC::CompressorOp::Off,
                                                                  LocalPartLoadFrac,
                                                                  SpeedNum,
                                                                  SpeedRatio,
                                                                  QZnReq,
                                                                  QLatReq);

                    } else if ((FullOutput - ReqOutput) > loadAccuracy) {
                        //         If the FullOutput is greater than (insufficient cooling) or very near the ReqOutput,
                        //         run the compressor at LocalPartLoadFrac = 1.
                        LocalPartLoadFrac = 1.0;
                        SpeedNum = maxNumSpeeds;
                        SpeedRatio = 1.0;
                        //         Else find the PLR to meet the load
                    } else {
                        //           OutletTempDXCoil is the full capacity outlet temperature at LocalPartLoadFrac = 1 from the CALL above. If this
                        //           temp is greater than the desired outlet temp, then run the compressor at LocalPartLoadFrac = 1, otherwise find
                        //           the operating PLR.
                        Real64 OutletTempDXCoil = state.dataVariableSpeedCoils->VarSpeedCoil(cbvav.CoolCoilNum).OutletAirDBTemp;
                        if (OutletTempDXCoil > DesOutTemp) {
                            LocalPartLoadFrac = 1.0;
                            SpeedNum = maxNumSpeeds;
                            SpeedRatio = 1.0;
                        } else {
                            // run at lowest speed
                            LocalPartLoadFrac = 1.0;
                            SpeedNum = 1;
                            SpeedRatio = 1.0;
                            QZnReq = 0.001; // to indicate the coil is running
                            VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                      cbvav.CoolCoilNum,
                                                                      HVAC::FanOp::Continuous,
                                                                      HVAC::CompressorOp::On,
                                                                      LocalPartLoadFrac,
                                                                      SpeedNum,
                                                                      SpeedRatio,
                                                                      QZnReq,
                                                                      QLatReq,
                                                                      LocalOnOffAirFlowRatio);

                            Real64 TempSpeedOut = s_node->Node(cbvav.CoolCoilAirInletNode).MassFlowRate *
                                                  (Psychrometrics::PsyHFnTdbW(s_node->Node(cbvav.CoolCoilAirOutletNode).Temp,
                                                                              s_node->Node(cbvav.CoolCoilAirOutletNode).HumRat) -
                                                   Psychrometrics::PsyHFnTdbW(s_node->Node(cbvav.CoolCoilAirInletNode).Temp,
                                                                              s_node->Node(cbvav.CoolCoilAirOutletNode).HumRat));
                            Real64 TempSpeedReqst =
                                s_node->Node(cbvav.CoolCoilAirInletNode).MassFlowRate *
                                (Psychrometrics::PsyHFnTdbW(DesOutTemp, s_node->Node(cbvav.CoolCoilAirOutletNode).HumRat) -
                                 Psychrometrics::PsyHFnTdbW(s_node->Node(cbvav.CoolCoilAirInletNode).Temp,
                                                            s_node->Node(cbvav.CoolCoilAirOutletNode).HumRat));

                            if ((TempSpeedOut - TempSpeedReqst) > tempAccuracy) {
                                // Check to see which speed to meet the load
                                LocalPartLoadFrac = 1.0;
                                SpeedRatio = 1.0;
                                for (int I = 2; I <= maxNumSpeeds; ++I) {
                                    SpeedNum = I;
                                    VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                              cbvav.CoolCoilNum,
                                                                              HVAC::FanOp::Continuous,
                                                                              HVAC::CompressorOp::On,
                                                                              LocalPartLoadFrac,
                                                                              SpeedNum,
                                                                              SpeedRatio,
                                                                              QZnReq,
                                                                              QLatReq,
                                                                              LocalOnOffAirFlowRatio);

                                    TempSpeedOut = s_node->Node(cbvav.CoolCoilAirInletNode).MassFlowRate *
                                                   (Psychrometrics::PsyHFnTdbW(s_node->Node(cbvav.CoolCoilAirOutletNode).Temp,
                                                                               s_node->Node(cbvav.CoolCoilAirOutletNode).HumRat) -
                                                    Psychrometrics::PsyHFnTdbW(s_node->Node(cbvav.CoolCoilAirInletNode).Temp,
                                                                               s_node->Node(cbvav.CoolCoilAirOutletNode).HumRat));
                                    TempSpeedReqst =
                                        s_node->Node(cbvav.CoolCoilAirInletNode).MassFlowRate *
                                        (Psychrometrics::PsyHFnTdbW(DesOutTemp, s_node->Node(cbvav.CoolCoilAirOutletNode).HumRat) -
                                         Psychrometrics::PsyHFnTdbW(s_node->Node(cbvav.CoolCoilAirInletNode).Temp,
                                                                    s_node->Node(cbvav.CoolCoilAirOutletNode).HumRat));

                                    if ((TempSpeedOut - TempSpeedReqst) < tempAccuracy) {
                                        SpeedNum = I;
                                        break;
                                    }
                                }
                                // now find the speed ratio for the found speednum
                                auto f = [&state, CBVAVNum, SpeedNum, DesOutTemp](Real64 const SpeedRatio) {
                                    auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNum);
                                    // FUNCTION LOCAL VARIABLE DECLARATIONS:
                                    Real64 OutletAirTemp; // outlet air temperature [C]
                                    Real64 QZnReqCycling = 0.001;
                                    Real64 QLatReqCycling = 0.0;
                                    Real64 OnOffAirFlowRatioCycling = 1.0;
                                    Real64 partLoadRatio = 1.0;
                                    HVAC::FanOp fanOp = HVAC::FanOp::Continuous;
                                    VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                              cbvav.CoolCoilNum,
                                                                              fanOp,
                                                                              HVAC::CompressorOp::On,
                                                                              partLoadRatio,
                                                                              SpeedNum,
                                                                              SpeedRatio,
                                                                              QZnReqCycling,
                                                                              QLatReqCycling,
                                                                              OnOffAirFlowRatioCycling);

                                    OutletAirTemp = state.dataVariableSpeedCoils->VarSpeedCoil(cbvav.CoolCoilNum).OutletAirDBTemp;
                                    return DesOutTemp - OutletAirTemp;
                                };
                                General::SolveRoot(state, tempAccuracy, MaxIte, SolFla, SpeedRatio, f, 1.0e-10, 1.0);

                                if (SolFla == -1) {
                                    if (!state.dataGlobal->WarmupFlag) {
                                        if (cbvav.DXIterationExceeded < 4) {
                                            ++cbvav.DXIterationExceeded;
                                            ShowWarningError(state,
                                                             format("{} - Iteration limit exceeded calculating VS DX coil speed ratio for coil named "
                                                                    "{}, in Unitary system named{}",
                                                                    HVAC::coilTypeNames[(int)cbvav.coolCoilType],
                                                                    cbvav.CoolCoilName,
                                                                    cbvav.Name));
                                            ShowContinueError(state, format("Calculated speed ratio = {:.4R}", SpeedRatio));
                                            ShowContinueErrorTimeStamp(
                                                state, "The calculated speed ratio will be used and the simulation continues. Occurrence info:");
                                        }
                                        ShowRecurringWarningErrorAtEnd(state,
                                                                       format("{} \"{}\" - Iteration limit exceeded calculating speed ratio error "
                                                                              "continues. Speed Ratio statistics follow.",
                                                                              HVAC::coilTypeNames[(int)cbvav.coolCoilType],
                                                                              cbvav.CoolCoilName),
                                                                       cbvav.DXIterationExceededIndex,
                                                                       LocalPartLoadFrac,
                                                                       LocalPartLoadFrac);
                                    }
                                } else if (SolFla == -2) {
                                    if (!state.dataGlobal->WarmupFlag) {
                                        if (cbvav.DXIterationFailed < 4) {
                                            ++cbvav.DXIterationFailed;
                                            ShowWarningError(state,
                                                             format("{} - DX unit speed ratio calculation failed: solver limits exceeded, for coil "
                                                                    "named {}, in Unitary system named{}",
                                                                    HVAC::coilTypeNames[(int)cbvav.coolCoilType],
                                                                    cbvav.CoolCoilName,
                                                                    cbvav.Name));
                                            ShowContinueError(state, format("Estimated speed ratio = {:.3R}", TempSpeedReqst / TempSpeedOut));
                                            ShowContinueErrorTimeStamp(
                                                state, "The estimated part-load ratio will be used and the simulation continues. Occurrence info:");
                                        }
                                        ShowRecurringWarningErrorAtEnd(
                                            state,
                                            format(
                                                "{} \"{}\" - DX unit speed ratio calculation failed error continues. speed ratio statistics follow.",
                                                HVAC::coilTypeNames[(int)cbvav.coolCoilType],
                                                cbvav.CoolCoilName),
                                            cbvav.DXIterationFailedIndex,
                                            SpeedRatio,
                                            SpeedRatio);
                                    }
                                    SpeedRatio = TempSpeedReqst / TempSpeedOut;
                                }
                            } else {
                                // cycling compressor at lowest speed number, find part load fraction
                                auto f = [&state, CBVAVNum, DesOutTemp](Real64 const PartLoadRatio) {
                                    auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNum);
                                    int speedNum = 1;
                                    Real64 speedRatio = 0.0;
                                    Real64 QZnReqCycling = 0.001;
                                    Real64 QLatReqCycling = 0.0;
                                    Real64 OnOffAirFlowRatioCycling = 1.0;
                                    VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                              cbvav.CoolCoilNum,
                                                                              HVAC::FanOp::Continuous,
                                                                              HVAC::CompressorOp::On,
                                                                              PartLoadRatio,
                                                                              speedNum,
                                                                              speedRatio,
                                                                              QZnReqCycling,
                                                                              QLatReqCycling,
                                                                              OnOffAirFlowRatioCycling);

                                    Real64 OutletAirTemp = state.dataVariableSpeedCoils->VarSpeedCoil(cbvav.CoolCoilNum).OutletAirDBTemp;
                                    return DesOutTemp - OutletAirTemp;
                                };
                                General::SolveRoot(state, tempAccuracy, MaxIte, SolFla, LocalPartLoadFrac, f, 1.0e-10, 1.0);
                                if (SolFla == -1) {
                                    if (!state.dataGlobal->WarmupFlag) {
                                        if (cbvav.DXCyclingIterationExceeded < 4) {
                                            ++cbvav.DXCyclingIterationExceeded;
                                            ShowWarningError(state,
                                                             format("{} - Iteration limit exceeded calculating VS DX unit low speed cycling ratio, "
                                                                    "for coil named {}, in Unitary system named{}",
                                                                    HVAC::coilTypeNames[(int)cbvav.coolCoilType],
                                                                    cbvav.CoolCoilName,
                                                                    cbvav.Name));
                                            ShowContinueError(state, format("Estimated cycling ratio  = {:.3R}", (TempSpeedReqst / TempSpeedOut)));
                                            ShowContinueError(state, format("Calculated cycling ratio = {:.3R}", LocalPartLoadFrac));
                                            ShowContinueErrorTimeStamp(
                                                state, "The calculated cycling ratio will be used and the simulation continues. Occurrence info:");
                                        }
                                        ShowRecurringWarningErrorAtEnd(
                                            state,
                                            format(" {} \"{}\" - Iteration limit exceeded calculating low speed cycling ratio "
                                                   "error continues. Sensible PLR statistics follow.",
                                                   HVAC::coilTypeNames[(int)cbvav.coolCoilType],
                                                   cbvav.CoolCoilName),
                                            cbvav.DXCyclingIterationExceededIndex,
                                            LocalPartLoadFrac,
                                            LocalPartLoadFrac);
                                    }
                                } else if (SolFla == -2) {

                                    if (!state.dataGlobal->WarmupFlag) {
                                        if (cbvav.DXCyclingIterationFailed < 4) {
                                            ++cbvav.DXCyclingIterationFailed;
                                            ShowWarningError(
                                                state,
                                                format("{} - DX unit low speed cycling ratio calculation failed: limits exceeded, for unit = {}",
                                                       HVAC::coilTypeNames[(int)cbvav.coolCoilType],
                                                       cbvav.Name));
                                            ShowContinueError(state,
                                                              format("Estimated low speed cycling ratio = {:.3R}", TempSpeedReqst / TempSpeedOut));
                                            ShowContinueErrorTimeStamp(state,
                                                                       "The estimated low speed cycling ratio will be used and the simulation "
                                                                       "continues. Occurrence info:");
                                        }
                                        ShowRecurringWarningErrorAtEnd(state,
                                                                       format("{} \"{}\" - DX unit low speed cycling ratio calculation failed error "
                                                                              "continues. cycling ratio statistics follow.",
                                                                              HVAC::coilTypeNames[(int)cbvav.coolCoilType],
                                                                              cbvav.CoolCoilName),
                                                                       cbvav.DXCyclingIterationFailedIndex,
                                                                       LocalPartLoadFrac,
                                                                       LocalPartLoadFrac);
                                    }
                                    LocalPartLoadFrac = TempSpeedReqst / TempSpeedOut;
                                }
                            }
                        }
                    }

                    if (LocalPartLoadFrac > 1.0) {
                        LocalPartLoadFrac = 1.0;
                    } else if (LocalPartLoadFrac < 0.0) {
                        LocalPartLoadFrac = 0.0;
                    }
                    state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = VariableSpeedCoils::getVarSpeedPartLoadRatio(state, cbvav.CoolCoilNum);
                    // variable-speed air-to-air cooling coil, end -------------------------

                } else if (cbvav.coolCoilType == HVAC::CoilType::CoolingDXTwoStageWHumControl) {
                    // Coil:Cooling:DX:TwoStageWithHumidityControlMode
                    // formerly (v3 and beyond) Coil:DX:MultiMode:CoolingEmpirical

                    // If DXCoolingSystem runs with a cooling load then set PartLoadFrac on Cooling System and the Mass Flow
                    // Multimode coil will switch to enhanced dehumidification if available and needed, but it
                    // still runs to meet the sensible load

                    // Determine required part load for normal mode

                    // Get full load result
                    HVAC::CoilMode DehumidMode = HVAC::CoilMode::Normal; // Dehumidification mode (0=normal, 1=enhanced)
                    cbvav.DehumidificationMode = DehumidMode;
                    DXCoils::SimDXCoilMultiMode(state,
                                                cbvav.CoolCoilNum,
                                                HVAC::CompressorOp::On,
                                                FirstHVACIteration,
                                                PartLoadFrac,
                                                DehumidMode,
                                                HVAC::FanOp::Continuous);
                    if (s_node->Node(cbvav.CoolCoilAirInletNode).Temp <= cbvav.CoilTempSetPoint) {
                        PartLoadFrac = 0.0;
                        DXCoils::SimDXCoilMultiMode(state,
                                                    cbvav.CoolCoilNum,
                                                    HVAC::CompressorOp::On,
                                                    FirstHVACIteration,
                                                    PartLoadFrac,
                                                    DehumidMode,
                                                    HVAC::FanOp::Continuous);
                    } else if (s_node->Node(cbvav.CoolCoilAirOutletNode).Temp > cbvav.CoilTempSetPoint) {
                        PartLoadFrac = 1.0;
                    } else {
                        auto f = [&state, CBVAVNum, DehumidMode](Real64 const PartLoadRatio) {
                            auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNum);
                            DXCoils::SimDXCoilMultiMode(state,
                                                        cbvav.CoolCoilName,
                                                        HVAC::CompressorOp::On,
                                                        false,
                                                        PartLoadRatio,
                                                        DehumidMode,
                                                        cbvav.CoolCoilNum,
                                                        HVAC::FanOp::Continuous);
                            return cbvav.CoilTempSetPoint - state.dataDXCoils->DXCoilOutletTemp(cbvav.CoolCoilNum);
                        };
                        General::SolveRoot(state, HVAC::SmallTempDiff, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                        if (SolFla == -1) {
                            if (cbvav.MMDXIterationExceeded < 1) {
                                ++cbvav.MMDXIterationExceeded;
                                ShowWarningError(state,
                                                 format("Iteration limit exceeded calculating DX unit part-load ratio, for unit={}", cbvav.Name));
                                ShowContinueErrorTimeStamp(state, format("Part-load ratio returned = {:.2R}", PartLoadFrac));
                                ShowContinueErrorTimeStamp(
                                    state, "The calculated part-load ratio will be used and the simulation continues. Occurrence info:");
                            } else {
                                ShowRecurringWarningErrorAtEnd(state,
                                                               cbvav.Name +
                                                                   ", Iteration limit exceeded calculating DX unit part-load ratio error continues.",
                                                               cbvav.MMDXIterationExceededIndex,
                                                               PartLoadFrac,
                                                               PartLoadFrac);
                            }
                        } else if (SolFla == -2) {
                            PartLoadFrac = max(0.0,
                                               min(1.0,
                                                   (s_node->Node(cbvav.CoolCoilAirInletNode).Temp - cbvav.CoilTempSetPoint) /
                                                       (s_node->Node(cbvav.CoolCoilAirInletNode).Temp -
                                                        s_node->Node(cbvav.CoolCoilAirOutletNode).Temp)));
                            if (cbvav.MMDXIterationFailed < 1) {
                                ++cbvav.MMDXIterationFailed;
                                ShowSevereError(
                                    state,
                                    format("DX unit part-load ratio calculation failed: part-load ratio limits exceeded, for unit={}", cbvav.Name));
                                ShowContinueError(state, format("Estimated part-load ratio = {:.3R}", PartLoadFrac));
                                ShowContinueErrorTimeStamp(
                                    state, "The estimated part-load ratio will be used and the simulation continues. Occurrence info:");
                            } else {
                                ShowRecurringWarningErrorAtEnd(state,
                                                               cbvav.Name + ", Part-load ratio calculation failed for DX unit error continues.",
                                                               cbvav.MMDXIterationFailedIndex,
                                                               PartLoadFrac,
                                                               PartLoadFrac);
                            }
                        }
                    }

                    // If humidity setpoint is not satisfied and humidity control type is Multimode,
                    // then turn on enhanced dehumidification mode 1

                    if ((s_node->Node(cbvav.CoolCoilAirOutletNode).HumRat > s_node->Node(cbvav.AirOutNode).HumRatMax) &&
                        (s_node->Node(cbvav.CoolCoilAirInletNode).HumRat > s_node->Node(cbvav.AirOutNode).HumRatMax) &&
                        (cbvav.DehumidControlType == DehumidControl::Multimode) && s_node->Node(cbvav.AirOutNode).HumRatMax > 0.0) {

                        // Determine required part load for enhanced dehumidification mode 1

                        // Get full load result
                        PartLoadFrac = 1.0;
                        DehumidMode = HVAC::CoilMode::Enhanced;
                        cbvav.DehumidificationMode = DehumidMode;
                        DXCoils::SimDXCoilMultiMode(state,
                                                    cbvav.CoolCoilNum,
                                                    HVAC::CompressorOp::On,
                                                    FirstHVACIteration,
                                                    PartLoadFrac,
                                                    DehumidMode,
                                                    HVAC::FanOp::Continuous);
                        if (s_node->Node(cbvav.CoolCoilAirInletNode).Temp <= cbvav.CoilTempSetPoint) {
                            PartLoadFrac = 0.0;
                        } else if (s_node->Node(cbvav.CoolCoilAirOutletNode).Temp > cbvav.CoilTempSetPoint) {
                            PartLoadFrac = 1.0;
                        } else {
                            auto f = [&state, CBVAVNum, DehumidMode](Real64 const PartLoadRatio) {
                                auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNum);
                                DXCoils::SimDXCoilMultiMode(state,
                                                            cbvav.CoolCoilNum,
                                                            HVAC::CompressorOp::On,
                                                            false,
                                                            PartLoadRatio,
                                                            DehumidMode,
                                                            HVAC::FanOp::Continuous);
                                return cbvav.CoilTempSetPoint - state.dataDXCoils->DXCoilOutletTemp(cbvav.CoolCoilNum);
                            };
                            General::SolveRoot(state, HVAC::SmallTempDiff, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                            if (SolFla == -1) {
                                if (cbvav.DMDXIterationExceeded < 1) {
                                    ++cbvav.DMDXIterationExceeded;
                                    ShowWarningError(
                                        state,
                                        format("Iteration limit exceeded calculating DX unit dehumidifying part-load ratio, for unit = {}",
                                               cbvav.Name));
                                    ShowContinueErrorTimeStamp(state, format("Part-load ratio returned={:.2R}", PartLoadFrac));
                                    ShowContinueErrorTimeStamp(
                                        state, "The calculated part-load ratio will be used and the simulation continues. Occurrence info:");
                                } else {
                                    ShowRecurringWarningErrorAtEnd(
                                        state,
                                        cbvav.Name + ", Iteration limit exceeded calculating DX unit dehumidifying part-load ratio error continues.",
                                        cbvav.DMDXIterationExceededIndex,
                                        PartLoadFrac,
                                        PartLoadFrac);
                                }
                            } else if (SolFla == -2) {
                                PartLoadFrac = max(0.0,
                                                   min(1.0,
                                                       (s_node->Node(cbvav.CoolCoilAirInletNode).Temp - cbvav.CoilTempSetPoint) /
                                                           (s_node->Node(cbvav.CoolCoilAirInletNode).Temp -
                                                            s_node->Node(cbvav.CoolCoilAirOutletNode).Temp)));
                                if (cbvav.DMDXIterationFailed < 1) {
                                    ++cbvav.DMDXIterationFailed;
                                    ShowSevereError(state,
                                                    format("DX unit dehumidifying part-load ratio calculation failed: part-load ratio limits "
                                                           "exceeded, for unit = {}",
                                                           cbvav.Name));
                                    ShowContinueError(state, format("Estimated part-load ratio = {:.3R}", PartLoadFrac));
                                    ShowContinueErrorTimeStamp(
                                        state, "The estimated part-load ratio will be used and the simulation continues. Occurrence info:");
                                } else {
                                    ShowRecurringWarningErrorAtEnd(
                                        state,
                                        cbvav.Name + ", Dehumidifying part-load ratio calculation failed for DX unit error continues.",
                                        cbvav.DMDXIterationFailedIndex,
                                        PartLoadFrac,
                                        PartLoadFrac);
                                }
                            }
                        }
                    } // End if humidity ratio setpoint not met - multimode humidity control

                    // If humidity setpoint is not satisfied and humidity control type is CoolReheat,
                    // then run to meet latent load

                    if ((s_node->Node(cbvav.CoolCoilAirOutletNode).HumRat > s_node->Node(cbvav.AirOutNode).HumRatMax) &&
                        (s_node->Node(cbvav.CoolCoilAirInletNode).HumRat > s_node->Node(cbvav.AirOutNode).HumRatMax) &&
                        (cbvav.DehumidControlType == DehumidControl::CoolReheat) && s_node->Node(cbvav.AirOutNode).HumRatMax > 0.0) {

                        // Determine revised desired outlet temperature  - use approach temperature control strategy
                        // based on CONTROLLER:SIMPLE TEMPANDHUMRAT control type.

                        // Calculate the approach temperature (difference between SA dry-bulb temp and SA dew point temp)
                        ApproachTemp = s_node->Node(cbvav.CoolCoilAirOutletNode).Temp -
                                       Psychrometrics::PsyTdpFnWPb(state, s_node->Node(cbvav.AirOutNode).HumRat, OutdoorBaroPress);
                        // Calculate the dew point temperature at the SA humidity ratio setpoint
                        DesiredDewPoint = Psychrometrics::PsyTdpFnWPb(state, s_node->Node(cbvav.AirOutNode).HumRatMax, OutdoorBaroPress);
                        // Adjust the calculated dew point temperature by the approach temp
                        cbvav.CoilTempSetPoint = min(cbvav.CoilTempSetPoint, (DesiredDewPoint + ApproachTemp));

                        // Determine required part load for cool reheat at adjusted DesiredOutletTemp

                        // Get full load result
                        PartLoadFrac = 1.0;
                        DehumidMode = HVAC::CoilMode::Normal;
                        cbvav.DehumidificationMode = DehumidMode;
                        DXCoils::SimDXCoilMultiMode(state,
                                                    cbvav.CoolCoilNum,
                                                    HVAC::CompressorOp::On,
                                                    FirstHVACIteration,
                                                    PartLoadFrac,
                                                    DehumidMode,
                                                    HVAC::FanOp::Continuous);
                        if (s_node->Node(cbvav.CoolCoilAirInletNode).Temp <= cbvav.CoilTempSetPoint) {
                            PartLoadFrac = 0.0;
                        } else if (s_node->Node(cbvav.CoolCoilAirOutletNode).Temp > cbvav.CoilTempSetPoint) {
                            PartLoadFrac = 1.0;
                        } else {
                            auto f = [&state, CBVAVNum, DehumidMode](Real64 const PartLoadRatio) {
                                auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNum);
                                DXCoils::SimDXCoilMultiMode(state,
                                                            cbvav.CoolCoilNum,
                                                            HVAC::CompressorOp::On,
                                                            false,
                                                            PartLoadRatio,
                                                            DehumidMode,
                                                            HVAC::FanOp::Continuous);
                                return cbvav.CoilTempSetPoint - state.dataDXCoils->DXCoilOutletTemp(cbvav.CoolCoilNum);
                            };
                            General::SolveRoot(state, HVAC::SmallTempDiff, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                            if (SolFla == -1) {
                                if (cbvav.CRDXIterationExceeded < 1) {
                                    ++cbvav.CRDXIterationExceeded;
                                    ShowWarningError(state,
                                                     format("Iteration limit exceeded calculating DX unit cool reheat part-load ratio, for unit = {}",
                                                            cbvav.Name));
                                    ShowContinueErrorTimeStamp(state, format("Part-load ratio returned = {:.2R}", PartLoadFrac));
                                    ShowContinueErrorTimeStamp(
                                        state, "The calculated part-load ratio will be used and the simulation continues. Occurrence info:");
                                } else {
                                    ShowRecurringWarningErrorAtEnd(
                                        state,
                                        cbvav.Name + ", Iteration limit exceeded calculating cool reheat part-load ratio DX unit error continues.",
                                        cbvav.CRDXIterationExceededIndex,
                                        PartLoadFrac,
                                        PartLoadFrac);
                                }
                            } else if (SolFla == -2) {
                                PartLoadFrac = max(0.0,
                                                   min(1.0,
                                                       (s_node->Node(cbvav.CoolCoilAirInletNode).Temp - cbvav.CoilTempSetPoint) /
                                                           (s_node->Node(cbvav.CoolCoilAirInletNode).Temp -
                                                            s_node->Node(cbvav.CoolCoilAirOutletNode).Temp)));
                                if (cbvav.CRDXIterationFailed < 1) {
                                    ++cbvav.CRDXIterationFailed;
                                    ShowSevereError(
                                        state,
                                        format(
                                            "DX unit cool reheat part-load ratio calculation failed: part-load ratio limits exceeded, for unit = {}",
                                            cbvav.Name));
                                    ShowContinueError(state, format("Estimated part-load ratio = {:.3R}", PartLoadFrac));
                                    ShowContinueErrorTimeStamp(
                                        state, "The estimated part-load ratio will be used and the simulation continues. Occurrence info:");
                                } else {
                                    ShowRecurringWarningErrorAtEnd(
                                        state,
                                        cbvav.Name + ", Dehumidifying part-load ratio calculation failed for DX unit error continues.",
                                        cbvav.DMDXIterationFailedIndex,
                                        PartLoadFrac,
                                        PartLoadFrac);
                                }
                            }
                        }
                    } // End if humidity ratio setpoint not met - CoolReheat humidity control

                    if (PartLoadFrac > 1.0) {
                        PartLoadFrac = 1.0;
                    } else if (PartLoadFrac < 0.0) {
                        PartLoadFrac = 0.0;
                    }
                    state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(cbvav.CoolCoilNum);

                } else {
                    ShowFatalError(state, format("SimCBVAV System: Invalid DX Cooling Coil={}", HVAC::coilTypeNames[(int)cbvav.coolCoilType]));
                }
                
            } else { // IF(OutdoorDryBulbTemp .GE. cBVAV%MinOATCompressor)THEN
                //     Simulate DX cooling coil with compressor off
                if (cbvav.coolCoilType == HVAC::CoilType::CoolingDXHXAssisted) {
                    HXAssistCoil::SimHXAssistedCoolingCoil(state,
                                                           cbvav.CoolCoilNum,
                                                           FirstHVACIteration,
                                                           HVAC::CompressorOp::Off,
                                                           0.0,
                                                           HVAC::FanOp::Continuous,
                                                           HXUnitOn);
                    state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(cbvav.CoolCoilNum);

                } else if (cbvav.coolCoilType == HVAC::CoilType::CoolingDXSingleSpeed) {
                    DXCoils::SimDXCoil(state,
                                       cbvav.CoolCoilNum,
                                       HVAC::CompressorOp::Off,
                                       FirstHVACIteration,
                                       HVAC::FanOp::Continuous,
                                       0.0,
                                       OnOffAirFlowRatio);
                    state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(cbvav.CoolCoilNum);
                    
                } else if (cbvav.coolCoilType == HVAC::CoilType::CoolingDXTwoStageWHumControl) {
                    DXCoils::SimDXCoilMultiMode(state,
                                                cbvav.CoolCoilNum,
                                                HVAC::CompressorOp::Off,
                                                FirstHVACIteration,
                                                0.0,
                                                HVAC::CoilMode::Normal,
                                                HVAC::FanOp::Continuous);
                    state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(cbvav.CoolCoilNum);
                    
                } else if (cbvav.coolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
                    // Real64 PartLoadFrac(0.0);
                    Real64 LocalPartLoadFrac = 0.0;
                    Real64 QZnReq = 0.0;  // Zone load (W), input to variable-speed DX coil
                    Real64 QLatReq = 0.0; // Zone latent load, input to variable-speed DX coil
                    Real64 SpeedRatio = 0.0;
                    int SpeedNum = 1;
                    // Get no load result
                    VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                              cbvav.CoolCoilNum,
                                                              HVAC::FanOp::Continuous,
                                                              HVAC::CompressorOp::Off,
                                                              LocalPartLoadFrac,
                                                              SpeedNum,
                                                              SpeedRatio,
                                                              QZnReq,
                                                              QLatReq);
                    state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = VariableSpeedCoils::getVarSpeedPartLoadRatio(state, cbvav.CoolCoilNum);
                }
            }

            // Simulate cooling coil with compressor off if zone requires heating
        } else { // HeatCoolMode == HeatingMode and no cooling is required, set PLR to 0
            if (cbvav.coolCoilType == HVAC::CoilType::CoolingDXHXAssisted) {
                HXAssistCoil::SimHXAssistedCoolingCoil(state,
                                                       cbvav.CoolCoilNum,
                                                       FirstHVACIteration,
                                                       HVAC::CompressorOp::Off,
                                                       0.0,
                                                       HVAC::FanOp::Continuous,
                                                       HXUnitOn);
                
            } else if (cbvav.coolCoilType == HVAC::CoilType::CoolingDXSingleSpeed) {
                DXCoils::SimDXCoil(state,
                                   cbvav.CoolCoilNum,
                                   HVAC::CompressorOp::Off,
                                   FirstHVACIteration,
                                   HVAC::FanOp::Continuous,
                                   0.0,
                                   OnOffAirFlowRatio);
                
            } else if (cbvav.coolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
                Real64 QZnReq = 0.0;  // Zone load (W), input to variable-speed DX coil
                Real64 QLatReq = 0.0; // Zone latent load, input to variable-speed DX coil
                Real64 LocalPartLoadFrac = 0.0;
                Real64 SpeedRatio = 0.0;
                int SpeedNum = 1;
                // run model with no load
                VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                          cbvav.CoolCoilNum,
                                                          HVAC::FanOp::Continuous,
                                                          HVAC::CompressorOp::Off,
                                                          LocalPartLoadFrac,
                                                          SpeedNum,
                                                          SpeedRatio,
                                                          QZnReq,
                                                          QLatReq);

            } else if (cbvav.coolCoilType == HVAC::CoilType::CoolingDXTwoStageWHumControl) {
                DXCoils::SimDXCoilMultiMode(state,
                                            cbvav.CoolCoilNum,
                                            HVAC::CompressorOp::Off,
                                            FirstHVACIteration,
                                            0.0,
                                            HVAC::CoilMode::Normal,
                                            HVAC::FanOp::Continuous);
            }
        }

        // Simulate the heating coil based on coil type
        switch (cbvav.heatCoilType) {
        case HVAC::CoilType::HeatingDXSingleSpeed: {
            //   Simulate DX heating coil if zone load is positive (heating load)
            if (cbvav.HeatCoolMode == HeatingMode) {
                if (OutdoorDryBulbTemp > cbvav.MinOATCompressor) {
                    //       simulate the DX heating coil
                    // vs coil issue

                    DXCoils::SimDXCoil(state,
                                       cbvav.HeatCoilNum,
                                       HVAC::CompressorOp::On,
                                       FirstHVACIteration,
                                       HVAC::FanOp::Continuous,
                                       PartLoadFrac,
                                       OnOffAirFlowRatio);
                    if (s_node->Node(cbvav.HeatCoilAirOutletNode).Temp > cbvav.CoilTempSetPoint &&
                        s_node->Node(cbvav.HeatCoilAirInletNode).Temp < cbvav.CoilTempSetPoint) {
                        // iterate to find PLR at CoilTempSetPoint
                        auto f = [&state, CBVAVNum, OnOffAirFlowRatio](Real64 const PartLoadFrac) {
                            auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNum);
                            DXCoils::CalcDXHeatingCoil(state, cbvav.HeatCoilNum, PartLoadFrac, HVAC::FanOp::Continuous, OnOffAirFlowRatio);
                            Real64 OutletAirTemp = state.dataDXCoils->DXCoilOutletTemp(cbvav.HeatCoilNum);
                            Real64 par2 = min(cbvav.CoilTempSetPoint, cbvav.MaxLATHeating);
                            return par2 - OutletAirTemp;
                        };
                        General::SolveRoot(state, HVAC::SmallTempDiff, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                        DXCoils::SimDXCoil(state,
                                           cbvav.HeatCoilNum,
                                           HVAC::CompressorOp::On,
                                           FirstHVACIteration,
                                           HVAC::FanOp::Continuous,
                                           PartLoadFrac,
                                           OnOffAirFlowRatio);
                        if (SolFla == -1 && !state.dataGlobal->WarmupFlag) {
                            ShowWarningError(
                                state, format("Iteration limit exceeded calculating DX unit part-load ratio, for unit = {}", cbvav.HeatCoilName));
                            ShowContinueError(state, format("Calculated part-load ratio = {:.3R}", PartLoadFrac));
                            ShowContinueErrorTimeStamp(state,
                                                       "The calculated part-load ratio will be used and the simulation continues. Occurrence info:");
                        } else if (SolFla == -2 && !state.dataGlobal->WarmupFlag) {
                            ShowSevereError(state,
                                            format("DX unit part-load ratio calculation failed: part-load ratio limits exceeded, for unit = {}",
                                                   cbvav.HeatCoilName));
                            ShowContinueErrorTimeStamp(
                                state,
                                format("A part-load ratio of {:.3R}will be used and the simulation continues. Occurrence info:", PartLoadFrac));
                            ShowContinueError(state, "Please send this information to the EnergyPlus support group.");
                        }
                    }
                } else { // OAT .LT. MinOATCompressor
                    //       simulate DX heating coil with compressor off
                    DXCoils::SimDXCoil(state,
                                       cbvav.HeatCoilNum,
                                       HVAC::CompressorOp::Off,
                                       FirstHVACIteration,
                                       HVAC::FanOp::Continuous,
                                       0.0,
                                       OnOffAirFlowRatio);
                }
                state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(cbvav.HeatCoilNum);
            } else { // HeatCoolMode = CoolingMode
                //     simulate DX heating coil with compressor off when cooling load is required
                DXCoils::SimDXCoil(state,
                                   cbvav.HeatCoilNum,
                                   HVAC::CompressorOp::Off,
                                   FirstHVACIteration,
                                   HVAC::FanOp::Continuous,
                                   0.0,
                                   OnOffAirFlowRatio);
            }
        } break;
          
        case HVAC::CoilType::HeatingDXVariableSpeed: {
            Real64 QZnReq = 0.0;                 // Zone load (W), input to variable-speed DX coil
            Real64 QLatReq = 0.0;                // Zone latent load, input to variable-speed DX coil
            Real64 LocalOnOffAirFlowRatio = 1.0; // ratio of compressor on flow to average flow over time step
            Real64 LocalPartLoadFrac = 0.0;
            Real64 SpeedRatio = 0.0;
            int SpeedNum = 1;
            int maxNumSpeeds = VariableSpeedCoils::GetCoilNumOfSpeeds(state, cbvav.HeatCoilNum);
            Real64 DesOutTemp = cbvav.CoilTempSetPoint;
            // Get no load result
            VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                      cbvav.HeatCoilNum,
                                                      HVAC::FanOp::Continuous,
                                                      HVAC::CompressorOp::Off,
                                                      LocalPartLoadFrac,
                                                      SpeedNum,
                                                      SpeedRatio,
                                                      QZnReq,
                                                      QLatReq);

            Real64 NoOutput = s_node->Node(cbvav.HeatCoilAirInletNode).MassFlowRate *
                              (Psychrometrics::PsyHFnTdbW(s_node->Node(cbvav.HeatCoilAirOutletNode).Temp,
                                                          s_node->Node(cbvav.HeatCoilAirInletNode).HumRat) -
                               Psychrometrics::PsyHFnTdbW(s_node->Node(cbvav.HeatCoilAirInletNode).Temp,
                                                          s_node->Node(cbvav.HeatCoilAirOutletNode).HumRat));
            Real64 TempNoOutput = s_node->Node(cbvav.HeatCoilAirOutletNode).Temp;
            // Real64 NoLoadHumRatOut = VariableSpeedCoils::VarSpeedCoil( CBVAV( CBVAVNum ).CoolCoilCompIndex ).OutletAirHumRat;

            // Get full load result
            LocalPartLoadFrac = 1.0;
            SpeedNum = maxNumSpeeds;
            SpeedRatio = 1.0;
            QZnReq = 0.001; // to indicate the coil is running
            VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                      cbvav.HeatCoilNum,
                                                      HVAC::FanOp::Continuous,
                                                      HVAC::CompressorOp::On,
                                                      LocalPartLoadFrac,
                                                      SpeedNum,
                                                      SpeedRatio,
                                                      QZnReq,
                                                      QLatReq);

            // Real64 FullLoadHumRatOut = VariableSpeedCoils::VarSpeedCoil( CBVAV( CBVAVNum ).CoolCoilCompIndex ).OutletAirHumRat;
            Real64 FullOutput = s_node->Node(cbvav.HeatCoilAirInletNode).MassFlowRate *
                                (Psychrometrics::PsyHFnTdbW(s_node->Node(cbvav.HeatCoilAirOutletNode).Temp,
                                                            s_node->Node(cbvav.HeatCoilAirOutletNode).HumRat) -
                                 Psychrometrics::PsyHFnTdbW(s_node->Node(cbvav.HeatCoilAirInletNode).Temp,
                                                            s_node->Node(cbvav.HeatCoilAirOutletNode).HumRat));
            Real64 ReqOutput = s_node->Node(cbvav.HeatCoilAirInletNode).MassFlowRate *
                               (Psychrometrics::PsyHFnTdbW(DesOutTemp, s_node->Node(cbvav.HeatCoilAirOutletNode).HumRat) -
                                Psychrometrics::PsyHFnTdbW(s_node->Node(cbvav.HeatCoilAirInletNode).Temp,
                                                           s_node->Node(cbvav.HeatCoilAirOutletNode).HumRat));

            Real64 loadAccuracy = 0.001;                  // Watts, power
            Real64 tempAccuracy = 0.001;                  // delta C, temperature
            if ((NoOutput - ReqOutput) > -loadAccuracy) { //         IF NoOutput is higher than (more heating than required) or very near the
                                                          //         ReqOutput, do not run the compressor
                LocalPartLoadFrac = 0.0;
                SpeedNum = 1;
                SpeedRatio = 0.0;
                QZnReq = 0.0;
                // call again with coil off
                VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                          cbvav.HeatCoilNum,
                                                          HVAC::FanOp::Continuous,
                                                          HVAC::CompressorOp::Off,
                                                          LocalPartLoadFrac,
                                                          SpeedNum,
                                                          SpeedRatio,
                                                          QZnReq,
                                                          QLatReq);

            } else if ((FullOutput - ReqOutput) < loadAccuracy) { //         If the FullOutput is less than (insufficient cooling) or very near
                                                                  //         the ReqOutput, run the compressor at LocalPartLoadFrac = 1.
                                                                  // which we just did so nothing to be done

            } else { //  Else find how the coil is modulating (speed level and speed ratio or part load between off and speed 1) to meet the load
                //           OutletTempDXCoil is the full capacity outlet temperature at LocalPartLoadFrac = 1 from the CALL above. If this temp is
                //           greater than the desired outlet temp, then run the compressor at LocalPartLoadFrac = 1, otherwise find the operating PLR.
                Real64 OutletTempDXCoil = state.dataVariableSpeedCoils->VarSpeedCoil(cbvav.HeatCoilNum).OutletAirDBTemp;
                if (OutletTempDXCoil < DesOutTemp) {
                    LocalPartLoadFrac = 1.0;
                    SpeedNum = maxNumSpeeds;
                    SpeedRatio = 1.0;
                    VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                              cbvav.HeatCoilNum,
                                                              HVAC::FanOp::Continuous,
                                                              HVAC::CompressorOp::On,
                                                              LocalPartLoadFrac,
                                                              SpeedNum,
                                                              SpeedRatio,
                                                              QZnReq,
                                                              QLatReq,
                                                              LocalOnOffAirFlowRatio);
                } else {
                    // run at lowest speed
                    LocalPartLoadFrac = 1.0;
                    SpeedNum = 1;
                    SpeedRatio = 1.0;
                    QZnReq = 0.001; // to indicate the coil is running
                    VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                              cbvav.HeatCoilNum,
                                                              HVAC::FanOp::Continuous,
                                                              HVAC::CompressorOp::On,
                                                              LocalPartLoadFrac,
                                                              SpeedNum,
                                                              SpeedRatio,
                                                              QZnReq,
                                                              QLatReq,
                                                              LocalOnOffAirFlowRatio);

                    Real64 TempSpeedOut = s_node->Node(cbvav.HeatCoilAirOutletNode).Temp;
                    Real64 TempSpeedOutSpeed1 = TempSpeedOut;

                    if ((TempSpeedOut - DesOutTemp) < tempAccuracy) {
                        // Check to see which speed to meet the load
                        LocalPartLoadFrac = 1.0;
                        SpeedRatio = 1.0;
                        for (int I = 2; I <= maxNumSpeeds; ++I) {
                            SpeedNum = I;
                            VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                      cbvav.HeatCoilNum,
                                                                      HVAC::FanOp::Continuous,
                                                                      HVAC::CompressorOp::On,
                                                                      LocalPartLoadFrac,
                                                                      SpeedNum,
                                                                      SpeedRatio,
                                                                      QZnReq,
                                                                      QLatReq,
                                                                      LocalOnOffAirFlowRatio);

                            TempSpeedOut = s_node->Node(cbvav.HeatCoilAirOutletNode).Temp;

                            if ((TempSpeedOut - DesOutTemp) > tempAccuracy) {
                                SpeedNum = I;
                                break;
                            }
                        }
                        // now find the speed ratio for the found speednum
                        int const vsCoilNum = cbvav.HeatCoilNum;
                        auto f = [&state, vsCoilNum, DesOutTemp, SpeedNum](Real64 const x) {
                            return HVACDXHeatPumpSystem::VSCoilSpeedResidual(state, x, vsCoilNum, DesOutTemp, SpeedNum, HVAC::FanOp::Continuous);
                        };
                        General::SolveRoot(state, tempAccuracy, MaxIte, SolFla, SpeedRatio, f, 1.0e-10, 1.0);

                        if (SolFla == -1) {
                            if (!state.dataGlobal->WarmupFlag) {
                                if (cbvav.DXHeatIterationExceeded < 4) {
                                    ++cbvav.DXHeatIterationExceeded;
                                    ShowWarningError(state,
                                                     format("{} - Iteration limit exceeded calculating VS DX coil speed ratio for coil named {}, in "
                                                            "Unitary system named{}",
                                                            HVAC::coilTypeNames[(int)cbvav.heatCoilType],
                                                            cbvav.HeatCoilName,
                                                            cbvav.Name));
                                    ShowContinueError(state, format("Calculated speed ratio = {:.4R}", SpeedRatio));
                                    ShowContinueErrorTimeStamp(
                                        state, "The calculated speed ratio will be used and the simulation continues. Occurrence info:");
                                }
                                ShowRecurringWarningErrorAtEnd(state,
                                                               format("{} \"{}\" - Iteration limit exceeded calculating speed ratio error continues. "
                                                                      "Speed Ratio statistics follow.",
                                                                      HVAC::coilTypeNames[(int)cbvav.heatCoilType],
                                                                      cbvav.HeatCoilName),
                                                               cbvav.DXHeatIterationExceededIndex,
                                                               LocalPartLoadFrac,
                                                               LocalPartLoadFrac);
                            }
                        } else if (SolFla == -2) {

                            if (!state.dataGlobal->WarmupFlag) {
                                if (cbvav.DXHeatIterationFailed < 4) {
                                    ++cbvav.DXHeatIterationFailed;
                                    ShowWarningError(state,
                                                     format("{} - DX unit speed ratio calculation failed: solver limits exceeded, for coil named {}, "
                                                            "in Unitary system named{}",
                                                            HVAC::coilTypeNames[(int)cbvav.heatCoilType],
                                                            cbvav.HeatCoilName,
                                                            cbvav.Name));
                                    ShowContinueErrorTimeStamp(state,
                                                               " Speed ratio will be set to 0.5, and the simulation continues. Occurrence info:");
                                }
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    format("{} \"{}\" - DX unit speed ratio calculation failed error continues. speed ratio statistics follow.",
                                           HVAC::coilTypeNames[(int)cbvav.heatCoilType],
                                           cbvav.HeatCoilName),
                                    cbvav.DXHeatIterationFailedIndex,
                                    SpeedRatio,
                                    SpeedRatio);
                            }
                            SpeedRatio = 0.5;
                        }
                        VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                  cbvav.HeatCoilNum,
                                                                  HVAC::FanOp::Continuous,
                                                                  HVAC::CompressorOp::On,
                                                                  LocalPartLoadFrac,
                                                                  SpeedNum,
                                                                  SpeedRatio,
                                                                  QZnReq,
                                                                  QLatReq,
                                                                  LocalOnOffAirFlowRatio);
                    } else {
                        // cycling compressor at lowest speed number, find part load fraction
                        int VSCoilNum = cbvav.HeatCoilNum;
                        auto f = [&state, VSCoilNum, DesOutTemp](Real64 const x) {
                            return HVACDXHeatPumpSystem::VSCoilCyclingResidual(state, x, VSCoilNum, DesOutTemp, HVAC::FanOp::Continuous);
                        };
                        General::SolveRoot(state, tempAccuracy, MaxIte, SolFla, LocalPartLoadFrac, f, 1.0e-10, 1.0);
                        if (SolFla == -1) {
                            if (!state.dataGlobal->WarmupFlag) {
                                if (cbvav.DXHeatCyclingIterationExceeded < 4) {
                                    ++cbvav.DXHeatCyclingIterationExceeded;
                                    ShowWarningError(state,
                                                     format("{} - Iteration limit exceeded calculating VS DX unit low speed cycling ratio, for coil "
                                                            "named {}, in Unitary system named{}",
                                                            HVAC::coilTypeNames[(int)cbvav.heatCoilType],
                                                            cbvav.HeatCoilName,
                                                            cbvav.Name));
                                    ShowContinueError(state, format("Estimated cycling ratio  = {:.3R}", (DesOutTemp / TempSpeedOut)));
                                    ShowContinueError(state, format("Calculated cycling ratio = {:.3R}", LocalPartLoadFrac));
                                    ShowContinueErrorTimeStamp(
                                        state, "The calculated cycling ratio will be used and the simulation continues. Occurrence info:");
                                }
                                ShowRecurringWarningErrorAtEnd(state,
                                                               format("{} \"{}\" - Iteration limit exceeded calculating low speed cycling ratio "
                                                                      "error continues. Sensible PLR statistics follow.",
                                                                      HVAC::coilTypeNames[(int)cbvav.heatCoilType],
                                                                      cbvav.HeatCoilName),
                                                               cbvav.DXHeatCyclingIterationExceededIndex,
                                                               LocalPartLoadFrac,
                                                               LocalPartLoadFrac);
                            }
                        } else if (SolFla == -2) {

                            if (!state.dataGlobal->WarmupFlag) {
                                if (cbvav.DXHeatCyclingIterationFailed < 4) {
                                    ++cbvav.DXHeatCyclingIterationFailed;
                                    ShowWarningError(state,
                                                     format("{} - DX unit low speed cycling ratio calculation failed: limits exceeded, for unit = {}",
                                                            HVAC::coilTypeNames[(int)cbvav.heatCoilType],
                                                            cbvav.Name));
                                    ShowContinueError(state,
                                                      format("Estimated low speed cycling ratio = {:.3R}",
                                                             (DesOutTemp - TempNoOutput) / (TempSpeedOutSpeed1 - TempNoOutput)));
                                    ShowContinueErrorTimeStamp(
                                        state, "The estimated low speed cycling ratio will be used and the simulation continues. Occurrence info:");
                                }
                                ShowRecurringWarningErrorAtEnd(state,
                                                               format("{} \"{}\" - DX unit low speed cycling ratio calculation failed error "
                                                                      "continues. cycling ratio statistics follow.",
                                                                      HVAC::coilTypeNames[(int)cbvav.heatCoilType],
                                                                      cbvav.HeatCoilName),
                                                               cbvav.DXHeatCyclingIterationFailedIndex,
                                                               LocalPartLoadFrac,
                                                               LocalPartLoadFrac);
                            }
                            LocalPartLoadFrac = (DesOutTemp - TempNoOutput) / (TempSpeedOutSpeed1 - TempNoOutput);
                        }
                        VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                  cbvav.HeatCoilNum,
                                                                  HVAC::FanOp::Continuous,
                                                                  HVAC::CompressorOp::On,
                                                                  LocalPartLoadFrac,
                                                                  SpeedNum,
                                                                  SpeedRatio,
                                                                  QZnReq,
                                                                  QLatReq,
                                                                  LocalOnOffAirFlowRatio);
                    }
                }
            }

            if (LocalPartLoadFrac > 1.0) {
                LocalPartLoadFrac = 1.0;
            } else if (LocalPartLoadFrac < 0.0) {
                LocalPartLoadFrac = 0.0;
            }
            state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = VariableSpeedCoils::getVarSpeedPartLoadRatio(state, cbvav.HeatCoilNum);
        } break;
        case HVAC::CoilType::HeatingGasOrOtherFuel:
        case HVAC::CoilType::HeatingElectric:
        case HVAC::CoilType::HeatingWater:
        case HVAC::CoilType::HeatingSteam: { // not a DX heating coil
            if (cbvav.HeatCoolMode == HeatingMode) {
                CpAir = Psychrometrics::PsyCpAirFnW(s_node->Node(cbvav.HeatCoilAirInletNode).HumRat);
                QHeater = s_node->Node(cbvav.HeatCoilAirInletNode).MassFlowRate * CpAir *
                          (cbvav.CoilTempSetPoint - s_node->Node(cbvav.HeatCoilAirInletNode).Temp);
            } else {
                QHeater = 0.0;
            }
            // Added None DX heating coils calling point
            s_node->Node(cbvav.HeatCoilAirOutletNode).TempSetPoint = cbvav.CoilTempSetPoint;
            CalcNonDXHeatingCoils(state, CBVAVNum, FirstHVACIteration, QHeater, cbvav.fanOp, QHeaterActual);
        } break;
        default: {
            ShowFatalError(state, format("SimCBVAV System: Invalid Heating Coil={}", HVAC::coilTypeNames[(int)cbvav.heatCoilType]));
        } break;
        }

        if (cbvav.fanPlace == HVAC::FanPlace::DrawThru) {
            state.dataFans->fans(cbvav.FanIndex)
                ->simulate(state, FirstHVACIteration, state.dataHVACUnitaryBypassVAV->FanSpeedRatio, _, 1.0 / OnOffAirFlowRatio, _);
        }
        int splitterOutNode = cbvav.SplitterOutletAirNode;
        s_node->Node(splitterOutNode).MassFlowRateSetPoint = s_node->Node(OutletNode).MassFlowRateSetPoint;
        s_node->Node(OutletNode) = s_node->Node(splitterOutNode);
        s_node->Node(OutletNode).TempSetPoint = cbvav.OutletTempSetPoint;
        s_node->Node(OutletNode).MassFlowRate =
            (1.0 - state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction) * s_node->Node(cbvav.MixerInletAirNode).MassFlowRate;
        // report variable
        cbvav.BypassMassFlowRate =
            state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction * s_node->Node(cbvav.MixerInletAirNode).MassFlowRate;
        // initialize bypass duct connected to mixer or plenum with flow rate and conditions
        if (cbvav.plenumIndex > 0 || cbvav.mixerIndex > 0) {
            int plenumOrMixerInletNode = cbvav.PlenumMixerInletAirNode;
            s_node->Node(plenumOrMixerInletNode) = s_node->Node(splitterOutNode);
            s_node->Node(plenumOrMixerInletNode).MassFlowRate =
                state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction * s_node->Node(cbvav.MixerInletAirNode).MassFlowRate;
            s_node->Node(plenumOrMixerInletNode).MassFlowRateMaxAvail = s_node->Node(plenumOrMixerInletNode).MassFlowRate;
            state.dataAirLoop->AirLoopFlow(cbvav.AirLoopNumber).BypassMassFlow = s_node->Node(plenumOrMixerInletNode).MassFlowRate;
        }

        // calculate sensible load met using delta enthalpy at a constant (minimum) humidity ratio)
        MinHumRat = min(s_node->Node(InletNode).HumRat, s_node->Node(OutletNode).HumRat);
        LoadMet =
            s_node->Node(OutletNode).MassFlowRate * (Psychrometrics::PsyHFnTdbW(s_node->Node(OutletNode).Temp, MinHumRat) -
                                                                  Psychrometrics::PsyHFnTdbW(s_node->Node(InletNode).Temp, MinHumRat));

        // calculate OA fraction used for zone OA volume flow rate calc
        state.dataAirLoop->AirLoopFlow(cbvav.AirLoopNumber).OAFrac = 0.0;
        if (s_node->Node(cbvav.AirOutNode).MassFlowRate > 0.0) {
            state.dataAirLoop->AirLoopFlow(cbvav.AirLoopNumber).OAFrac =
                s_node->Node(cbvav.MixerOutsideAirNode).MassFlowRate / s_node->Node(cbvav.AirOutNode).MassFlowRate;
        }
    }

    void GetZoneLoads(EnergyPlusData &state,
                      int const CBVAVNum // Index to CBVAV unit being simulated
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2006

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is used to poll the thermostats in each zone and determine the
        // mode of operation, either cooling, heating, or none.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 ZoneLoad = 0.0; // Total load in controlled zone [W]

        auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNum);

        int dayOfSim = state.dataGlobal->DayOfSim; // DayOfSim increments during Warmup when it actually simulates the same day
        if (state.dataGlobal->WarmupFlag) {
            // when warmupday increments then reset timer
            int lastDayOfSim(0); // used during warmup to reset changeOverTimer since need to do same thing next warmup day
            if (lastDayOfSim != dayOfSim) {
                cbvav.changeOverTimer = -1.0; // reset to default (thisTime always > -1)
            }

            lastDayOfSim = dayOfSim;
            dayOfSim = 1; // reset so that thisTime is <= 24 during warmup
        }
        Real64 thisTime = (dayOfSim - 1) * 24 + state.dataGlobal->HourOfDay - 1 + (state.dataGlobal->TimeStep - 1) * state.dataGlobal->TimeStepZone +
                          state.dataHVACGlobal->SysTimeElapsed;

        if (thisTime <= cbvav.changeOverTimer) {
            cbvav.modeChanged = true;
            return;
        }

        Real64 QZoneReqCool = 0.0; // Total cooling load in all controlled zones [W]
        Real64 QZoneReqHeat = 0.0; // Total heating load in all controlled zones [W]
        cbvav.NumZonesCooled = 0;
        cbvav.NumZonesHeated = 0;
        cbvav.HeatCoolMode = 0;

        for (int ZoneNum = 1; ZoneNum <= cbvav.NumControlledZones; ++ZoneNum) {
            int actualZoneNum = cbvav.ControlledZoneNum(ZoneNum);
            int coolSeqNum = cbvav.ZoneSequenceCoolingNum(ZoneNum);
            int heatSeqNum = cbvav.ZoneSequenceHeatingNum(ZoneNum);
            if (coolSeqNum > 0 && heatSeqNum > 0) {
                Real64 ZoneLoadToCoolSPSequenced =
                    state.dataZoneEnergyDemand->ZoneSysEnergyDemand(actualZoneNum).SequencedOutputRequiredToCoolingSP(coolSeqNum);
                Real64 ZoneLoadToHeatSPSequenced =
                    state.dataZoneEnergyDemand->ZoneSysEnergyDemand(actualZoneNum).SequencedOutputRequiredToHeatingSP(heatSeqNum);
                if (ZoneLoadToHeatSPSequenced > 0.0 && ZoneLoadToCoolSPSequenced > 0.0) {
                    ZoneLoad = ZoneLoadToHeatSPSequenced;
                } else if (ZoneLoadToHeatSPSequenced < 0.0 && ZoneLoadToCoolSPSequenced < 0.0) {
                    ZoneLoad = ZoneLoadToCoolSPSequenced;
                } else if (ZoneLoadToHeatSPSequenced <= 0.0 && ZoneLoadToCoolSPSequenced >= 0.0) {
                    ZoneLoad = 0.0;
                }
            } else {
                ZoneLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(actualZoneNum).RemainingOutputRequired;
            }

            if (!state.dataZoneEnergyDemand->CurDeadBandOrSetback(actualZoneNum)) {
                if (ZoneLoad > HVAC::SmallLoad) {
                    QZoneReqHeat += ZoneLoad;
                    ++cbvav.NumZonesHeated;
                } else if (ZoneLoad < -HVAC::SmallLoad) {
                    QZoneReqCool += ZoneLoad;
                    ++cbvav.NumZonesCooled;
                }
            }
        }

        switch (cbvav.PriorityControl) {
        case PriorityCtrlMode::CoolingPriority: {
            if (QZoneReqCool < 0.0) {
                cbvav.HeatCoolMode = CoolingMode;
            } else if (QZoneReqHeat > 0.0) {
                cbvav.HeatCoolMode = HeatingMode;
            }
        } break;
        case PriorityCtrlMode::HeatingPriority: {
            if (QZoneReqHeat > 0.0) {
                cbvav.HeatCoolMode = HeatingMode;
            } else if (QZoneReqCool < 0.0) {
                cbvav.HeatCoolMode = CoolingMode;
            }
        } break;
        case PriorityCtrlMode::ZonePriority: {
            if (cbvav.NumZonesHeated > cbvav.NumZonesCooled) {
                if (QZoneReqHeat > 0.0) {
                    cbvav.HeatCoolMode = HeatingMode;
                } else if (QZoneReqCool < 0.0) {
                    cbvav.HeatCoolMode = CoolingMode;
                }
            } else if (cbvav.NumZonesCooled > cbvav.NumZonesHeated) {
                if (QZoneReqCool < 0.0) {
                    cbvav.HeatCoolMode = CoolingMode;
                } else if (QZoneReqHeat > 0.0) {
                    cbvav.HeatCoolMode = HeatingMode;
                }
            } else {
                if (std::abs(QZoneReqCool) > std::abs(QZoneReqHeat) && QZoneReqCool != 0.0) {
                    cbvav.HeatCoolMode = CoolingMode;
                } else if (std::abs(QZoneReqCool) < std::abs(QZoneReqHeat) && QZoneReqHeat != 0.0) {
                    cbvav.HeatCoolMode = HeatingMode;
                } else if (std::abs(QZoneReqCool) == std::abs(QZoneReqHeat) && QZoneReqCool != 0.0) {
                    cbvav.HeatCoolMode = CoolingMode;
                }
            }
        } break;
        case PriorityCtrlMode::LoadPriority: {
            if (std::abs(QZoneReqCool) > std::abs(QZoneReqHeat) && QZoneReqCool != 0.0) {
                cbvav.HeatCoolMode = CoolingMode;
            } else if (std::abs(QZoneReqCool) < std::abs(QZoneReqHeat) && QZoneReqHeat != 0.0) {
                cbvav.HeatCoolMode = HeatingMode;
            } else if (cbvav.NumZonesHeated > cbvav.NumZonesCooled) {
                if (QZoneReqHeat > 0.0) {
                    cbvav.HeatCoolMode = HeatingMode;
                } else if (QZoneReqCool < 0.0) {
                    cbvav.HeatCoolMode = CoolingMode;
                }
            } else if (cbvav.NumZonesHeated < cbvav.NumZonesCooled) {
                if (QZoneReqCool < 0.0) {
                    cbvav.HeatCoolMode = CoolingMode;
                } else if (QZoneReqHeat > 0.0) {
                    cbvav.HeatCoolMode = HeatingMode;
                }
            } else {
                if (QZoneReqCool < 0.0) {
                    cbvav.HeatCoolMode = CoolingMode;
                } else if (QZoneReqHeat > 0.0) {
                    cbvav.HeatCoolMode = HeatingMode;
                }
            }
            break;
        default:
            break;
        }
        }

        if (cbvav.LastMode != cbvav.HeatCoolMode) {
            cbvav.changeOverTimer = thisTime + cbvav.minModeChangeTime;
            cbvav.LastMode = cbvav.HeatCoolMode;
            cbvav.modeChanged = true;
        }
    }

    Real64 CalcSetPointTempTarget(EnergyPlusData &state, int const CBVAVNumber) // Index to changeover-bypass VAV system
    {

        // FUNCTION INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   August 2006

        // PURPOSE OF THIS FUNCTION:
        //  Calculate outlet air node temperature setpoint

        // METHODOLOGY EMPLOYED:
        //  Calculate an outlet temperature to satisfy zone loads. This temperature is calculated
        //  based on 1 zone's VAV box fully opened. The other VAV boxes are partially open (modulated).

        // Return value
        Real64 CalcSetPointTempTarget;

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 ZoneLoad;                 // Zone load sensed by thermostat [W]
        Real64 QToCoolSetPt;             // Zone load to cooling setpoint [W]
        Real64 QToHeatSetPt;             // Zone load to heating setpoint [W]
        Real64 SupplyAirTemp;            // Supply air temperature required to meet load [C]
        Real64 SupplyAirTempToHeatSetPt; // Supply air temperature required to reach the heating setpoint [C]
        Real64 SupplyAirTempToCoolSetPt; // Supply air temperature required to reach the cooling setpoint [C]

        auto &s_node = state.dataLoopNodes;
        auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNumber);

        Real64 DXCoolCoilInletTemp = s_node->Node(cbvav.CoolCoilAirInletNode).Temp;
        Real64 OutAirTemp = s_node->Node(cbvav.AirOutNode).Temp;
        Real64 OutAirHumRat = s_node->Node(cbvav.AirOutNode).HumRat;

        if (cbvav.HeatCoolMode == CoolingMode) { // Cooling required
            CalcSetPointTempTarget = 99999.0;
        } else if (cbvav.HeatCoolMode == HeatingMode) { // Heating required
            CalcSetPointTempTarget = -99999.0;
        }
        Real64 TSupplyToHeatSetPtMax = -99999.0; // Maximum of the supply air temperatures required to reach the heating setpoint [C]
        Real64 TSupplyToCoolSetPtMin = 99999.0;  // Minimum of the supply air temperatures required to reach the cooling setpoint [C]

        for (int ZoneNum = 1; ZoneNum <= cbvav.NumControlledZones; ++ZoneNum) {
            int ZoneNodeNum = cbvav.ControlledZoneNodeNum(ZoneNum);
            int BoxOutletNodeNum = cbvav.CBVAVBoxOutletNode(ZoneNum);
            if ((cbvav.ZoneSequenceCoolingNum(ZoneNum) > 0) && (cbvav.ZoneSequenceHeatingNum(ZoneNum) > 0)) {
                QToCoolSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(cbvav.ControlledZoneNum(ZoneNum))
                                   .SequencedOutputRequiredToCoolingSP(cbvav.ZoneSequenceCoolingNum(ZoneNum));
                QToHeatSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(cbvav.ControlledZoneNum(ZoneNum))
                                   .SequencedOutputRequiredToHeatingSP(cbvav.ZoneSequenceHeatingNum(ZoneNum));
                if (QToHeatSetPt > 0.0 && QToCoolSetPt > 0.0) {
                    ZoneLoad = QToHeatSetPt;
                } else if (QToHeatSetPt < 0.0 && QToCoolSetPt < 0.0) {
                    ZoneLoad = QToCoolSetPt;
                } else if (QToHeatSetPt <= 0.0 && QToCoolSetPt >= 0.0) {
                    ZoneLoad = 0.0;
                }
            } else {
                ZoneLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(cbvav.ControlledZoneNum(ZoneNum)).RemainingOutputRequired;
                QToCoolSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(cbvav.ControlledZoneNum(ZoneNum)).OutputRequiredToCoolingSP;
                QToHeatSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(cbvav.ControlledZoneNum(ZoneNum)).OutputRequiredToHeatingSP;
            }

            Real64 CpSupplyAir = Psychrometrics::PsyCpAirFnW(OutAirHumRat);

            // Find the supply air temperature that will force the box to full flow
            if (BoxOutletNodeNum > 0) {
                if (s_node->Node(BoxOutletNodeNum).MassFlowRateMax == 0.0) {
                    SupplyAirTemp = s_node->Node(ZoneNodeNum).Temp;
                } else {
                    // The target supply air temperature is based on current zone temp and load and max box flow rate
                    SupplyAirTemp = s_node->Node(ZoneNodeNum).Temp +
                                    ZoneLoad / (CpSupplyAir * s_node->Node(BoxOutletNodeNum).MassFlowRateMax);
                }
            } else {
                SupplyAirTemp = s_node->Node(ZoneNodeNum).Temp;
            }

            //     Save the MIN (cooling) or MAX (heating) temperature for coil control
            //     One box will always operate at maximum damper position minimizing overall system energy use
            if (cbvav.HeatCoolMode == CoolingMode) {
                CalcSetPointTempTarget = min(SupplyAirTemp, CalcSetPointTempTarget);
            } else if (cbvav.HeatCoolMode == HeatingMode) {
                CalcSetPointTempTarget = max(SupplyAirTemp, CalcSetPointTempTarget);
            } else {
                //       Should use CpAirAtCoolSetPoint or CpAirAtHeatSetPoint here?
                //       If so, use ZoneThermostatSetPointLo(ZoneNum) and ZoneThermostatSetPointHi(ZoneNum)
                //       along with the zone humidity ratio
                if (s_node->Node(BoxOutletNodeNum).MassFlowRateMax == 0.0) {
                    SupplyAirTempToHeatSetPt = s_node->Node(ZoneNodeNum).Temp;
                    SupplyAirTempToCoolSetPt = s_node->Node(ZoneNodeNum).Temp;
                } else {
                    SupplyAirTempToHeatSetPt = s_node->Node(ZoneNodeNum).Temp +
                                               QToHeatSetPt / (CpSupplyAir * s_node->Node(BoxOutletNodeNum).MassFlowRateMax);
                    SupplyAirTempToCoolSetPt = s_node->Node(ZoneNodeNum).Temp +
                                               QToCoolSetPt / (CpSupplyAir * s_node->Node(BoxOutletNodeNum).MassFlowRateMax);
                }
                TSupplyToHeatSetPtMax = max(SupplyAirTempToHeatSetPt, TSupplyToHeatSetPtMax);
                TSupplyToCoolSetPtMin = min(SupplyAirTempToCoolSetPt, TSupplyToCoolSetPtMin);
            }
        }

        //   Account for floating condition where cooling/heating is required to avoid overshooting setpoint
        if (cbvav.HeatCoolMode == 0) {
            if (cbvav.fanOp == HVAC::FanOp::Continuous) {
                if (OutAirTemp > TSupplyToCoolSetPtMin) {
                    CalcSetPointTempTarget = TSupplyToCoolSetPtMin;
                } else if (OutAirTemp < TSupplyToHeatSetPtMax) {
                    CalcSetPointTempTarget = TSupplyToHeatSetPtMax;
                } else {
                    CalcSetPointTempTarget = OutAirTemp;
                }
            } else { // Reset setpoint to inlet air temp if unit is OFF and in cycling fan mode
                CalcSetPointTempTarget = s_node->Node(cbvav.AirInNode).Temp;
            }
            //   Reset cooling/heating mode to OFF if mixed air inlet temperature is below/above setpoint temperature.
            //   HeatCoolMode = 0 for OFF, 1 for cooling, 2 for heating
        } else if (cbvav.HeatCoolMode == CoolingMode) {
            if (DXCoolCoilInletTemp < CalcSetPointTempTarget) CalcSetPointTempTarget = DXCoolCoilInletTemp;
        } else if (cbvav.HeatCoolMode == HeatingMode) {
            if (DXCoolCoilInletTemp > CalcSetPointTempTarget) CalcSetPointTempTarget = DXCoolCoilInletTemp;
        }

        //   Limit outlet node temperature to MAX/MIN specified in input
        if (CalcSetPointTempTarget < cbvav.MinLATCooling) CalcSetPointTempTarget = cbvav.MinLATCooling;
        if (CalcSetPointTempTarget > cbvav.MaxLATHeating) CalcSetPointTempTarget = cbvav.MaxLATHeating;

        return CalcSetPointTempTarget;
    }

    void SetAverageAirFlow(EnergyPlusData &state,
                           int const CBVAVNum,       // Index to CBVAV system
                           Real64 &OnOffAirFlowRatio // Ratio of compressor ON airflow to average airflow over timestep
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2006

        // PURPOSE OF THIS SUBROUTINE:
        // Set the average air mass flow rates for this time step
        // Set OnOffAirFlowRatio to be used by DX coils

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 ZoneMassFlow; // Zone mass flow rate required to meet zone load [kg/s]
        Real64 ZoneLoad;     // Zone load calculated by ZoneTempPredictor [W]

        auto &s_node = state.dataLoopNodes;
        auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNum);

        int InletNode = cbvav.AirInNode;                     // Inlet node number for CBVAVNum
        int OutletNode = cbvav.AirOutNode;                   // Outlet node number for CBVAVNum
        int MixerMixedAirNode = cbvav.MixerMixedAirNode;     // Mixed air node number in OA mixer
        int MixerOutsideAirNode = cbvav.MixerOutsideAirNode; // Outside air node number in OA mixer
        int MixerReliefAirNode = cbvav.MixerReliefAirNode;   // Relief air node number in OA mixer
        int MixerInletAirNode = cbvav.MixerInletAirNode;     // Inlet air node number in OA mixer

        Real64 SystemMassFlow = 0.0; // System mass flow rate required for all zones [kg/s]
        Real64 CpSupplyAir = Psychrometrics::PsyCpAirFnW(s_node->Node(OutletNode).HumRat); // Specific heat of outlet air [J/kg-K]
        // Determine zone air flow
        for (int ZoneNum = 1; ZoneNum <= cbvav.NumControlledZones; ++ZoneNum) {
            int ZoneNodeNum = cbvav.ControlledZoneNodeNum(ZoneNum);
            int BoxOutletNodeNum = cbvav.CBVAVBoxOutletNode(ZoneNum); // Zone supply air inlet node number
            if ((cbvav.ZoneSequenceCoolingNum(ZoneNum) > 0) && (cbvav.ZoneSequenceHeatingNum(ZoneNum) > 0)) {
                Real64 QToCoolSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(cbvav.ControlledZoneNum(ZoneNum))
                                          .SequencedOutputRequiredToCoolingSP(cbvav.ZoneSequenceCoolingNum(ZoneNum));
                Real64 QToHeatSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(cbvav.ControlledZoneNum(ZoneNum))
                                          .SequencedOutputRequiredToHeatingSP(cbvav.ZoneSequenceHeatingNum(ZoneNum));
                if (QToHeatSetPt > 0.0 && QToCoolSetPt > 0.0) {
                    ZoneLoad = QToHeatSetPt;
                } else if (QToHeatSetPt < 0.0 && QToCoolSetPt < 0.0) {
                    ZoneLoad = QToCoolSetPt;
                } else if (QToHeatSetPt <= 0.0 && QToCoolSetPt >= 0.0) {
                    ZoneLoad = 0.0;
                }
            } else {
                ZoneLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(cbvav.ControlledZoneNum(ZoneNum)).RemainingOutputRequired;
            }
            Real64 CpZoneAir = Psychrometrics::PsyCpAirFnW(s_node->Node(ZoneNodeNum).HumRat);
            Real64 DeltaCpTemp = CpSupplyAir * s_node->Node(OutletNode).Temp - CpZoneAir * s_node->Node(ZoneNodeNum).Temp;

            // Need to check DeltaCpTemp and ensure that it is not zero
            if (DeltaCpTemp != 0.0) { // .AND. .NOT. CurDeadBandOrSetback(ZoneNum))THEN
                ZoneMassFlow = ZoneLoad / DeltaCpTemp;
            } else {
                //     reset to 0 so we don't add in the last zone's mass flow rate
                ZoneMassFlow = 0.0;
            }
            SystemMassFlow += max(s_node->Node(BoxOutletNodeNum).MassFlowRateMin,
                                  min(ZoneMassFlow, s_node->Node(BoxOutletNodeNum).MassFlowRateMax));
        }

        Real64 AverageUnitMassFlow = state.dataHVACUnitaryBypassVAV->CompOnMassFlow;
        Real64 AverageOAMassFlow = state.dataHVACUnitaryBypassVAV->OACompOnMassFlow;
        state.dataHVACUnitaryBypassVAV->FanSpeedRatio = state.dataHVACUnitaryBypassVAV->CompOnFlowRatio;

        s_node->Node(MixerInletAirNode) = s_node->Node(InletNode);

        s_node->Node(MixerMixedAirNode).MassFlowRateMin = 0.0;

        if (cbvav.availSched->getCurrentVal() == 0.0 || AverageUnitMassFlow == 0.0) {
            s_node->Node(InletNode).MassFlowRate = 0.0;
            s_node->Node(MixerOutsideAirNode).MassFlowRate = 0.0;
            s_node->Node(MixerReliefAirNode).MassFlowRate = 0.0;
            OnOffAirFlowRatio = 0.0;
            state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction = 0.0;
        } else {
            s_node->Node(MixerInletAirNode).MassFlowRate = AverageUnitMassFlow;
            s_node->Node(MixerOutsideAirNode).MassFlowRate = AverageOAMassFlow;
            s_node->Node(MixerReliefAirNode).MassFlowRate = AverageOAMassFlow;
            OnOffAirFlowRatio = 1.0;
            Real64 boxOutletNodeFlow = 0.0;
            for (int i = 1; i <= cbvav.NumControlledZones; ++i) {
                boxOutletNodeFlow += s_node->Node(cbvav.CBVAVBoxOutletNode(i)).MassFlowRate;
            }
            state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction = max(0.0, 1.0 - (boxOutletNodeFlow / AverageUnitMassFlow));
        }
    }

    void ReportCBVAV(EnergyPlusData &state, int const CBVAVNum) // Index of the current CBVAV unit being simulated
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Richard Raustad
        //       DATE WRITTEN   July 2006

        // PURPOSE OF THIS SUBROUTINE:
        // Fills some of the report variables for the changeover-bypass VAV system

        auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNum);

        Real64 ReportingConstant = state.dataHVACGlobal->TimeStepSysSec;

        cbvav.TotCoolEnergy = cbvav.TotCoolEnergyRate * ReportingConstant;
        cbvav.TotHeatEnergy = cbvav.TotHeatEnergyRate * ReportingConstant;
        cbvav.SensCoolEnergy = cbvav.SensCoolEnergyRate * ReportingConstant;
        cbvav.SensHeatEnergy = cbvav.SensHeatEnergyRate * ReportingConstant;
        cbvav.LatCoolEnergy = cbvav.LatCoolEnergyRate * ReportingConstant;
        cbvav.LatHeatEnergy = cbvav.LatHeatEnergyRate * ReportingConstant;
        cbvav.ElecConsumption = cbvav.ElecPower * ReportingConstant;

        if (cbvav.FirstPass) {
            if (!state.dataGlobal->SysSizingCalc) {
                DataSizing::resetHVACSizingGlobals(state, state.dataSize->CurZoneEqNum, state.dataSize->CurSysNum, cbvav.FirstPass);
            }
        }

        // reset to 1 in case blow through fan configuration (fan resets to 1, but for blow thru fans coil sets back down < 1)
        state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
    }

    void CalcNonDXHeatingCoils(EnergyPlusData &state,
                               int const CBVAVNum,            // Changeover bypass VAV unit index
                               bool const FirstHVACIteration, // flag for first HVAC iteration in the time step
                               Real64 &HeatCoilLoad,          // heating coil load to be met (Watts)
                               HVAC::FanOp const fanOp,       // fan operation mode
                               Real64 &HeatCoilLoadmet        // coil heating load met
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
        Real64 constexpr ErrTolerance = 0.001; // convergence limit for hotwater coil
        int constexpr SolveMaxIter = 50;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 mdot;            // heating coil steam or hot water mass flow rate
        Real64 MinWaterFlow;    // minimum water mass flow rate
        Real64 MaxHotWaterFlow; // maximum hot water mass flow rate, kg/s
        Real64 HotWaterMdot;    // actual hot water mass flow rate

        Real64 QCoilActual = 0.0; // actual heating load met

        auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNum);

        if (HeatCoilLoad > HVAC::SmallLoad) {
            switch (cbvav.heatCoilType) {
            case HVAC::CoilType::HeatingGasOrOtherFuel:
            case HVAC::CoilType::HeatingElectric: {
                HeatingCoils::SimulateHeatingCoilComponents(
                    state, cbvav.HeatCoilNum, FirstHVACIteration, HeatCoilLoad, QCoilActual, false, fanOp);
            } break;
            case HVAC::CoilType::HeatingWater: {
                // simulate the heating coil at maximum hot water flow rate
                MaxHotWaterFlow = cbvav.MaxHeatCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(state, MaxHotWaterFlow, cbvav.HeatCoilControlNode, cbvav.HeatCoilFluidOutletNode, cbvav.HeatCoilPlantLoc);
                WaterCoils::SimulateWaterCoilComponents(
                    state, cbvav.HeatCoilNum, FirstHVACIteration, QCoilActual, fanOp);
                if (QCoilActual > (HeatCoilLoad + HVAC::SmallLoad)) {
                    // control water flow to obtain output matching HeatCoilLoad
                    int SolFlag = 0;
                    MinWaterFlow = 0.0;
                    auto f = [&state, CBVAVNum, FirstHVACIteration, HeatCoilLoad](Real64 const HWFlow) {
                        auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAVs(CBVAVNum);
                        Real64 QCoilActual = HeatCoilLoad;
                        Real64 mdot = HWFlow;
                        PlantUtilities::SetComponentFlowRate(state, mdot, cbvav.HeatCoilControlNode, cbvav.HeatCoilFluidOutletNode, cbvav.HeatCoilPlantLoc);
                        // simulate the hot water supplemental heating coil
                        WaterCoils::SimulateWaterCoilComponents(
                            state, cbvav.HeatCoilNum, FirstHVACIteration, QCoilActual, cbvav.fanOp);
                        if (HeatCoilLoad != 0.0) {
                            return (QCoilActual - HeatCoilLoad) / HeatCoilLoad;
                        } // Autodesk:Return Condition added to assure return value is set
                        return 0.0;
                    };
                    General::SolveRoot(state, ErrTolerance, SolveMaxIter, SolFlag, HotWaterMdot, f, MinWaterFlow, MaxHotWaterFlow);
                    if (SolFlag == -1) {
                        if (cbvav.HotWaterCoilMaxIterIndex == 0) {
                            ShowWarningMessage(
                                state,
                                format("CalcNonDXHeatingCoils: Hot water coil control failed for {}=\"{}\"", cbvav.UnitType, cbvav.Name));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state, format("  Iteration limit [{}] exceeded in calculating hot water mass flow rate", SolveMaxIter));
                        }
                        ShowRecurringWarningErrorAtEnd(
                            state,
                            format("CalcNonDXHeatingCoils: Hot water coil control failed (iteration limit [{}]) for {}=\"{}",
                                   SolveMaxIter,
                                   cbvav.UnitType,
                                   cbvav.Name),
                            cbvav.HotWaterCoilMaxIterIndex);
                    } else if (SolFlag == -2) {
                        if (cbvav.HotWaterCoilMaxIterIndex2 == 0) {
                            ShowWarningMessage(state,
                                               format("CalcNonDXHeatingCoils: Hot water coil control failed (maximum flow limits) for {}=\"{}\"",
                                                      cbvav.UnitType,
                                                      cbvav.Name));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state, "...Bad hot water maximum flow rate limits");
                            ShowContinueError(state, format("...Given minimum water flow rate={:.3R} kg/s", MinWaterFlow));
                            ShowContinueError(state, format("...Given maximum water flow rate={:.3R} kg/s", MaxHotWaterFlow));
                        }
                        ShowRecurringWarningErrorAtEnd(state,
                                                       "CalcNonDXHeatingCoils: Hot water coil control failed (flow limits) for " +
                                                           cbvav.UnitType + "=\"" + cbvav.Name + "\"",
                                                       cbvav.HotWaterCoilMaxIterIndex2,
                                                       MaxHotWaterFlow,
                                                       MinWaterFlow,
                                                       _,
                                                       "[kg/s]",
                                                       "[kg/s]");
                    }
                    // simulate the hot water heating coil
                    QCoilActual = HeatCoilLoad;
                    // simulate the hot water heating coil
                    WaterCoils::SimulateWaterCoilComponents(
                        state, cbvav.HeatCoilNum, FirstHVACIteration, QCoilActual, fanOp);
                }
            } break;
            case HVAC::CoilType::HeatingSteam: {
                mdot = cbvav.MaxHeatCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(state, mdot, cbvav.HeatCoilControlNode, cbvav.HeatCoilFluidOutletNode, cbvav.HeatCoilPlantLoc);

                // simulate the steam heating coil
                SteamCoils::SimulateSteamCoilComponents(
                    state, cbvav.HeatCoilNum, FirstHVACIteration, HeatCoilLoad, QCoilActual, fanOp);
            } break;
            default:
                break;
            }
        } else {
            switch (cbvav.heatCoilType) {
            case HVAC::CoilType::HeatingGasOrOtherFuel:
            case HVAC::CoilType::HeatingElectric: {
                HeatingCoils::SimulateHeatingCoilComponents(
                    state, cbvav.HeatCoilNum, FirstHVACIteration, HeatCoilLoad, QCoilActual, false, fanOp);
            } break;
            case HVAC::CoilType::HeatingWater: {
                mdot = 0.0;
                PlantUtilities::SetComponentFlowRate(state, mdot, cbvav.HeatCoilControlNode, cbvav.HeatCoilFluidOutletNode, cbvav.HeatCoilPlantLoc);
                QCoilActual = HeatCoilLoad;
                // simulate the hot water heating coil
                WaterCoils::SimulateWaterCoilComponents(
                    state, cbvav.HeatCoilNum, FirstHVACIteration, QCoilActual, fanOp);
            } break;
            case HVAC::CoilType::HeatingSteam: {
                mdot = 0.0;
                PlantUtilities::SetComponentFlowRate(state, mdot, cbvav.HeatCoilControlNode, cbvav.HeatCoilFluidOutletNode, cbvav.HeatCoilPlantLoc);
                // simulate the steam heating coil
                SteamCoils::SimulateSteamCoilComponents(
                    state, cbvav.HeatCoilNum, FirstHVACIteration, HeatCoilLoad, QCoilActual, fanOp);
            } break;
            default:
                break;
            }
        }
        HeatCoilLoadmet = QCoilActual;
    }

} // namespace HVACUnitaryBypassVAV

} // namespace EnergyPlus

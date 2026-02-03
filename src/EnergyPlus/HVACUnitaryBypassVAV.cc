// EnergyPlus, Copyright (c) 1996-2026, The Board of Trustees of the University of Illinois,
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
            CBVAVNum = Util::FindItemInList(CompName, state.dataHVACUnitaryBypassVAV->CBVAV);
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
                if (CompName != state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum).Name) {
                    ShowFatalError(state,
                                   format("SimUnitaryBypassVAV: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
                                          CBVAVNum,
                                          CompName,
                                          state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum).Name));
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

        auto &changeOverByPassVAV = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);

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
        if (changeOverByPassVAV.HeatCoilType == HVAC::CoilType::DXHeatingEmpirical) {
            HeatingPower = state.dataHVACGlobal->DXElecHeatingPower;
            locDefrostPower = state.dataHVACGlobal->DefrostElecPower;
        } else if (changeOverByPassVAV.HeatCoilType == HVAC::CoilType::HeatingAirToAirVariableSpeed) {
            HeatingPower = state.dataHVACGlobal->DXElecHeatingPower;
            locDefrostPower = state.dataHVACGlobal->DefrostElecPower;
        } else if (changeOverByPassVAV.HeatCoilType == HVAC::CoilType::HeatingElectric) {
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
        static constexpr std::string_view getUnitaryHeatCoolVAVChangeoverBypass("GetUnitaryHeatCool:VAVChangeoverBypass");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int NumAlphas;                // Number of Alphas for each GetObjectItem call
        int NumNumbers;               // Number of Numbers for each GetObjectItem call
        int IOStatus;                 // Used in GetObjectItem
        std::string CompSetFanInlet;  // Used in SetUpCompSets call
        std::string CompSetFanOutlet; // Used in SetUpCompSets call
        bool ErrorsFound(false);      // Set to true if errors in input, fatal at end of routine
        bool DXErrorsFound(false);    // Set to true if errors in get coil input
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

        // Update Num in state and make local convenience copy
        int NumCBVAV = state.dataHVACUnitaryBypassVAV->NumCBVAV =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, CurrentModuleObject);

        // allocate the data structures
        state.dataHVACUnitaryBypassVAV->CBVAV.resize(NumCBVAV);
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

            auto &thisCBVAV = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);

            thisCBVAV.Name = Alphas(1);

            ErrorObjectHeader eoh{routineName, CurrentModuleObject, thisCBVAV.Name};

            thisCBVAV.UnitType = CurrentModuleObject;
            if (lAlphaBlanks(2)) {
                thisCBVAV.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((thisCBVAV.availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ErrorsFound = true;
            }

            thisCBVAV.MaxCoolAirVolFlow = Numbers(1);
            if (thisCBVAV.MaxCoolAirVolFlow <= 0.0 && thisCBVAV.MaxCoolAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} illegal {} = {:.7T}", CurrentModuleObject, cNumericFields(1), Numbers(1)));
                ShowContinueError(state, format("{} must be greater than zero.", cNumericFields(1)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, thisCBVAV.Name));
                ErrorsFound = true;
            }

            thisCBVAV.MaxHeatAirVolFlow = Numbers(2);
            if (thisCBVAV.MaxHeatAirVolFlow <= 0.0 && thisCBVAV.MaxHeatAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} illegal {} = {:.7T}", CurrentModuleObject, cNumericFields(2), Numbers(2)));
                ShowContinueError(state, format("{} must be greater than zero.", cNumericFields(2)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, thisCBVAV.Name));
                ErrorsFound = true;
            }

            thisCBVAV.MaxNoCoolHeatAirVolFlow = Numbers(3);
            if (thisCBVAV.MaxNoCoolHeatAirVolFlow < 0.0 && thisCBVAV.MaxNoCoolHeatAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} illegal {} = {:.7T}", CurrentModuleObject, cNumericFields(3), Numbers(3)));
                ShowContinueError(state, format("{} must be greater than or equal to zero.", cNumericFields(3)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, thisCBVAV.Name));
                ErrorsFound = true;
            }

            thisCBVAV.CoolOutAirVolFlow = Numbers(4);
            if (thisCBVAV.CoolOutAirVolFlow < 0.0 && thisCBVAV.CoolOutAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} illegal {} = {:.7T}", CurrentModuleObject, cNumericFields(4), Numbers(4)));
                ShowContinueError(state, format("{} must be greater than or equal to zero.", cNumericFields(4)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, thisCBVAV.Name));
                ErrorsFound = true;
            }

            thisCBVAV.HeatOutAirVolFlow = Numbers(5);
            if (thisCBVAV.HeatOutAirVolFlow < 0.0 && thisCBVAV.HeatOutAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} illegal {} = {:.7T}", CurrentModuleObject, cNumericFields(5), Numbers(5)));
                ShowContinueError(state, format("{} must be greater than or equal to zero.", cNumericFields(5)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, thisCBVAV.Name));
                ErrorsFound = true;
            }

            thisCBVAV.NoCoolHeatOutAirVolFlow = Numbers(6);
            if (thisCBVAV.NoCoolHeatOutAirVolFlow < 0.0 && thisCBVAV.NoCoolHeatOutAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, format("{} illegal {} = {:.7T}", CurrentModuleObject, cNumericFields(6), Numbers(6)));
                ShowContinueError(state, format("{} must be greater than or equal to zero.", cNumericFields(6)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, thisCBVAV.Name));
                ErrorsFound = true;
            }

            thisCBVAV.outAirSched = Sched::GetSchedule(state, Alphas(3));
            if (thisCBVAV.outAirSched != nullptr) {
                if (!thisCBVAV.outAirSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 1.0)) {
                    Sched::ShowSevereBadMinMax(state, eoh, cAlphaFields(3), Alphas(3), Clusive::In, 0.0, Clusive::In, 1.0);
                    ErrorsFound = true;
                }
            }

            thisCBVAV.AirInNode =
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

            thisCBVAV.AirOutNode =
                NodeInputManager::GetOnlySingleNode(state,
                                                    Alphas(7),
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatCoolVAVChangeoverBypass,
                                                    Alphas(1),
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Outlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);

            thisCBVAV.SplitterOutletAirNode =
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
                thisCBVAV.PlenumMixerInletAirNode =
                    NodeInputManager::GetOnlySingleNode(state,
                                                        Alphas(20),
                                                        ErrorsFound,
                                                        DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatCoolVAVChangeoverBypass,
                                                        Alphas(1),
                                                        DataLoopNode::NodeFluidType::Air,
                                                        DataLoopNode::ConnectionType::Internal,
                                                        NodeInputManager::CompFluidStream::Primary,
                                                        DataLoopNode::ObjectIsParent);
                thisCBVAV.PlenumMixerInletAirNode =
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

            thisCBVAV.plenumIndex = ZonePlenum::getReturnPlenumIndexFromInletNode(state, thisCBVAV.PlenumMixerInletAirNode);
            thisCBVAV.mixerIndex = MixerComponent::getZoneMixerIndexFromInletNode(state, thisCBVAV.PlenumMixerInletAirNode);
            if (thisCBVAV.plenumIndex > 0 && thisCBVAV.mixerIndex > 0) {
                ShowSevereError(state, format("{}: {}", CurrentModuleObject, thisCBVAV.Name));
                ShowContinueError(state, format("Illegal connection for {} = \"{}\".", cAlphaFields(20), Alphas(20)));
                ShowContinueError(
                    state, format("{} cannot be connected to both an AirloopHVAC:ReturnPlenum and an AirloopHVAC:ZoneMixer.", cAlphaFields(20)));
                ErrorsFound = true;
            } else if (thisCBVAV.plenumIndex == 0 && thisCBVAV.mixerIndex == 0 && thisCBVAV.PlenumMixerInletAirNode > 0) {
                ShowSevereError(state, format("{}: {}", CurrentModuleObject, thisCBVAV.Name));
                ShowContinueError(state, format("Illegal connection for {} = \"{}\".", cAlphaFields(20), Alphas(20)));
                ShowContinueError(
                    state,
                    format("{} must be connected to an AirloopHVAC:ReturnPlenum or AirloopHVAC:ZoneMixer. No connection found.", cAlphaFields(20)));
                ErrorsFound = true;
            }

            thisCBVAV.MixerInletAirNode =
                NodeInputManager::GetOnlySingleNode(state,
                                                    MixerInletNodeName,
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatCoolVAVChangeoverBypass,
                                                    Alphas(1),
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Internal,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);

            thisCBVAV.MixerInletAirNode =
                NodeInputManager::GetOnlySingleNode(state,
                                                    MixerInletNodeName,
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatCoolVAVChangeoverBypass,
                                                    Alphas(1) + "_Mixer",
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Outlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);

            thisCBVAV.SplitterOutletAirNode =
                NodeInputManager::GetOnlySingleNode(state,
                                                    SplitterOutletNodeName,
                                                    ErrorsFound,
                                                    DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatCoolVAVChangeoverBypass,
                                                    Alphas(1) + "_Splitter",
                                                    DataLoopNode::NodeFluidType::Air,
                                                    DataLoopNode::ConnectionType::Inlet,
                                                    NodeInputManager::CompFluidStream::Primary,
                                                    DataLoopNode::ObjectIsParent);

            thisCBVAV.OAMixType = Alphas(8);
            thisCBVAV.OAMixName = Alphas(9);

            bool errFlag = false;
            ValidateComponent(state, thisCBVAV.OAMixType, thisCBVAV.OAMixName, errFlag, CurrentModuleObject);
            if (errFlag) {
                ShowContinueError(state, format("specified in {} = \"{}\".", CurrentModuleObject, thisCBVAV.Name));
                ErrorsFound = true;
            } else {
                // Get OA Mixer node numbers
                OANodeNums = MixedAir::GetOAMixerNodeNumbers(state, thisCBVAV.OAMixName, errFlag);
                if (errFlag) {
                    ShowContinueError(state, format("that was specified in {} = {}", CurrentModuleObject, thisCBVAV.Name));
                    ShowContinueError(state, "..OutdoorAir:Mixer is required. Enter an OutdoorAir:Mixer object with this name.");
                    ErrorsFound = true;
                } else {
                    thisCBVAV.MixerOutsideAirNode = OANodeNums(1);
                    thisCBVAV.MixerReliefAirNode = OANodeNums(2);
                    // thisCBVAV%MixerInletAirNode  = OANodeNums(3)
                    thisCBVAV.MixerMixedAirNode = OANodeNums(4);
                }
            }

            if (thisCBVAV.MixerInletAirNode != OANodeNums(3)) {
                ShowSevereError(state, format("{}: {}", CurrentModuleObject, thisCBVAV.Name));
                ShowContinueError(state, format("Illegal {} = {}.", cAlphaFields(5), MixerInletNodeName));
                ShowContinueError(
                    state, format("{} must be the same as the return air stream node specified in the OutdoorAir:Mixer object.", cAlphaFields(5)));
                ErrorsFound = true;
            }

            if (thisCBVAV.MixerInletAirNode == thisCBVAV.AirInNode) {
                ShowSevereError(state, format("{}: {}", CurrentModuleObject, thisCBVAV.Name));
                ShowContinueError(state, format("Illegal {} = {}.", cAlphaFields(5), MixerInletNodeName));
                ShowContinueError(state, format("{} must be different than the {}.", cAlphaFields(5), cAlphaFields(4)));
                ErrorsFound = true;
            }

            if (thisCBVAV.SplitterOutletAirNode == thisCBVAV.AirOutNode) {
                ShowSevereError(state, format("{}: {}", CurrentModuleObject, thisCBVAV.Name));
                ShowContinueError(state, format("Illegal {} = {}.", cAlphaFields(6), SplitterOutletNodeName));
                ShowContinueError(state, format("{} must be different than the {}.", cAlphaFields(6), cAlphaFields(7)));
                ErrorsFound = true;
            }

            // required field must be Key=Fan:ConstantVolume, Fan:OnOff or Fan:SystemModel and read in as upper case
            thisCBVAV.fanType = static_cast<HVAC::FanType>(getEnumValue(HVAC::fanTypeNamesUC, Alphas(10)));
            assert(thisCBVAV.fanType != HVAC::FanType::Invalid);

            thisCBVAV.FanName = Alphas(11);
            int fanOutletNode(0);

            // check that the fan exists
            if ((thisCBVAV.FanIndex = Fans::GetFanIndex(state, thisCBVAV.FanName)) == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(11), thisCBVAV.FanName);
                ErrorsFound = true;
                thisCBVAV.FanVolFlow = 9999.0;
            } else {
                auto *fan = state.dataFans->fans(thisCBVAV.FanIndex);
                thisCBVAV.FanInletNodeNum = fan->inletNodeNum;
                fanOutletNode = fan->outletNodeNum;
                thisCBVAV.FanVolFlow = fan->maxAirFlowRate;
            }

            // required field must be Key=BlowThrough or DrawThrough and read in as BLOWTHROUGH or DRAWTHROUGH
            thisCBVAV.fanPlace = static_cast<HVAC::FanPlace>(getEnumValue(HVAC::fanPlaceNamesUC, Alphas(12)));

            if (thisCBVAV.fanPlace == HVAC::FanPlace::DrawThru) {
                if (thisCBVAV.SplitterOutletAirNode != fanOutletNode) {
                    ShowSevereError(state, format("{}: {}", CurrentModuleObject, thisCBVAV.Name));
                    ShowContinueError(state, format("Illegal {} = {}.", cAlphaFields(6), SplitterOutletNodeName));
                    ShowContinueError(state,
                                      format("{} must be the same as the fan outlet node specified in {} = {}: {} when draw through {} is selected.",
                                             cAlphaFields(6),
                                             cAlphaFields(10),
                                             Alphas(10),
                                             thisCBVAV.FanName,
                                             cAlphaFields(11)));
                    ErrorsFound = true;
                }
            }

            if (thisCBVAV.FanVolFlow != DataSizing::AutoSize) {
                if (thisCBVAV.FanVolFlow < thisCBVAV.MaxCoolAirVolFlow && thisCBVAV.MaxCoolAirVolFlow != DataSizing::AutoSize) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in {} = {} is less than the ",
                                            CurrentModuleObject,
                                            thisCBVAV.FanVolFlow,
                                            cAlphaFields(11),
                                            thisCBVAV.FanName) +
                                         cNumericFields(1));
                    ShowContinueError(state, format(" {} is reset to the fan flow rate and the simulation continues.", cNumericFields(1)));
                    ShowContinueError(state, format(" Occurs in {} = {}", CurrentModuleObject, thisCBVAV.Name));
                    thisCBVAV.MaxCoolAirVolFlow = thisCBVAV.FanVolFlow;
                }
                if (thisCBVAV.FanVolFlow < thisCBVAV.MaxHeatAirVolFlow && thisCBVAV.MaxHeatAirVolFlow != DataSizing::AutoSize) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in {} = {} is less than the ",
                                            CurrentModuleObject,
                                            thisCBVAV.FanVolFlow,
                                            cAlphaFields(11),
                                            thisCBVAV.FanName) +
                                         cNumericFields(2));
                    ShowContinueError(state, format(" {} is reset to the fan flow rate and the simulation continues.", cNumericFields(2)));
                    ShowContinueError(state, format(" Occurs in {} = {}", CurrentModuleObject, thisCBVAV.Name));
                    thisCBVAV.MaxHeatAirVolFlow = thisCBVAV.FanVolFlow;
                }
            }

            //   only check that OA flow in cooling is >= SA flow in cooling when they are not autosized
            if (thisCBVAV.CoolOutAirVolFlow > thisCBVAV.MaxCoolAirVolFlow && thisCBVAV.CoolOutAirVolFlow != DataSizing::AutoSize &&
                thisCBVAV.MaxCoolAirVolFlow != DataSizing::AutoSize) {
                ShowWarningError(state, format("{}: {} cannot be greater than {}", CurrentModuleObject, cNumericFields(4), cNumericFields(1)));
                ShowContinueError(state, format(" {} is reset to the fan flow rate and the simulation continues.", cNumericFields(4)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, thisCBVAV.Name));
                thisCBVAV.CoolOutAirVolFlow = thisCBVAV.FanVolFlow;
            }

            //   only check that SA flow in heating is >= OA flow in heating when they are not autosized
            if (thisCBVAV.HeatOutAirVolFlow > thisCBVAV.MaxHeatAirVolFlow && thisCBVAV.HeatOutAirVolFlow != DataSizing::AutoSize &&
                thisCBVAV.MaxHeatAirVolFlow != DataSizing::AutoSize) {
                ShowWarningError(state, format("{}: {} cannot be greater than {}", CurrentModuleObject, cNumericFields(5), cNumericFields(2)));
                ShowContinueError(state, format(" {} is reset to the fan flow rate and the simulation continues.", cNumericFields(5)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, thisCBVAV.Name));
                thisCBVAV.HeatOutAirVolFlow = thisCBVAV.FanVolFlow;
            }

            std::string thisCoolCoilType = Alphas(14);
            thisCBVAV.CoolCoilType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, thisCoolCoilType));
            thisCBVAV.DXCoolCoilName = Alphas(15);

            if (thisCBVAV.CoolCoilType == HVAC::CoilType::DXCoolingSingleSpeed) {
                DXCoilErrFlag = false;
                DXCoils::GetDXCoilIndex(state, thisCBVAV.DXCoolCoilName, thisCBVAV.DXCoolCoilIndexNum, DXCoilErrFlag, thisCoolCoilType);
                if (DXCoilErrFlag) {
                    ShowContinueError(state, format("...occurs in {} \"{}\"", thisCBVAV.UnitType, thisCBVAV.Name));
                    ErrorsFound = true;
                } else {
                    thisCBVAV.DXCoilInletNode = state.dataDXCoils->DXCoil(thisCBVAV.DXCoolCoilIndexNum).AirInNode;
                    thisCBVAV.DXCoilOutletNode = state.dataDXCoils->DXCoil(thisCBVAV.DXCoolCoilIndexNum).AirOutNode;
                    thisCBVAV.CondenserNodeNum = state.dataDXCoils->DXCoil(thisCBVAV.DXCoolCoilIndexNum).CondenserInletNodeNum(1);
                }
            } else if (thisCBVAV.CoolCoilType == HVAC::CoilType::CoolingAirToAirVariableSpeed) {
                DXCoilErrFlag = false;
                thisCBVAV.DXCoolCoilIndexNum =
                    VariableSpeedCoils::GetCoilIndexVariableSpeed(state, thisCoolCoilType, thisCBVAV.DXCoolCoilName, DXCoilErrFlag);
                if (DXCoilErrFlag) {
                    ShowContinueError(state, format("...occurs in {} \"{}\"", thisCBVAV.UnitType, thisCBVAV.Name));
                    ErrorsFound = true;
                } else {
                    thisCBVAV.DXCoilInletNode = state.dataVariableSpeedCoils->VarSpeedCoil(thisCBVAV.DXCoolCoilIndexNum).AirInletNodeNum;
                    thisCBVAV.DXCoilOutletNode = state.dataVariableSpeedCoils->VarSpeedCoil(thisCBVAV.DXCoolCoilIndexNum).AirOutletNodeNum;
                    thisCBVAV.CondenserNodeNum = state.dataVariableSpeedCoils->VarSpeedCoil(thisCBVAV.DXCoolCoilIndexNum).CondenserInletNodeNum;
                }
            } else if (thisCBVAV.CoolCoilType == HVAC::CoilType::DXCoolingHXAssisted) {
                DXCoilErrFlag = false;
                int ActualCoolCoilType =
                    HVACHXAssistedCoolingCoil::GetCoilObjectTypeNum(state, thisCoolCoilType, thisCBVAV.DXCoolCoilName, DXErrorsFound);
                if (DXErrorsFound) {
                    ShowSevereError(state, format("{}: {}", CurrentModuleObject, thisCBVAV.Name));
                    ShowContinueError(state, format("CoilSystem:Cooling:DX:HeatExchangerAssisted \"{}\" not found.", thisCBVAV.DXCoolCoilName));
                    ErrorsFound = true;
                } else {
                    if (ActualCoolCoilType == HVAC::CoilDX_CoolingSingleSpeed) {
                        DXCoils::GetDXCoilIndex(
                            state,
                            HVACHXAssistedCoolingCoil::GetHXDXCoilName(state, thisCoolCoilType, thisCBVAV.DXCoolCoilName, DXCoilErrFlag),
                            thisCBVAV.DXCoolCoilIndexNum,
                            DXCoilErrFlag,
                            "Coil:Cooling:DX:SingleSpeed");
                        if (DXCoilErrFlag) {
                            ShowContinueError(state, format("...occurs in {} \"{}\"", thisCBVAV.UnitType, thisCBVAV.Name));
                            ErrorsFound = true;
                        } else {
                            // the "coil" nodes are actually the parent nodes of the wrapped HX
                            thisCBVAV.DXCoilInletNode =
                                HVACHXAssistedCoolingCoil::GetCoilInletNode(state, thisCoolCoilType, thisCBVAV.DXCoolCoilName, DXErrorsFound);
                            thisCBVAV.DXCoilOutletNode =
                                HVACHXAssistedCoolingCoil::GetCoilOutletNode(state, thisCoolCoilType, thisCBVAV.DXCoolCoilName, DXErrorsFound);
                            // the DX coil holds the condenser inlet node number
                            thisCBVAV.CondenserNodeNum = state.dataDXCoils->DXCoil(thisCBVAV.DXCoolCoilIndexNum).CondenserInletNodeNum(1);
                        }
                    } else if (ActualCoolCoilType == HVAC::Coil_CoolingAirToAirVariableSpeed) {
                        thisCBVAV.DXCoolCoilIndexNum = VariableSpeedCoils::GetCoilIndexVariableSpeed(
                            state,
                            "Coil:Cooling:DX:VariableSpeed",
                            HVACHXAssistedCoolingCoil::GetHXDXCoilName(state, thisCoolCoilType, thisCBVAV.DXCoolCoilName, DXCoilErrFlag),
                            DXCoilErrFlag);
                        if (DXCoilErrFlag) {
                            ShowContinueError(state, format("...occurs in {} \"{}\"", thisCBVAV.UnitType, thisCBVAV.Name));
                            ErrorsFound = true;
                        } else {
                            thisCBVAV.DXCoilInletNode = state.dataVariableSpeedCoils->VarSpeedCoil(thisCBVAV.DXCoolCoilIndexNum).AirInletNodeNum;
                            thisCBVAV.DXCoilOutletNode = state.dataVariableSpeedCoils->VarSpeedCoil(thisCBVAV.DXCoolCoilIndexNum).AirOutletNodeNum;
                            thisCBVAV.CondenserNodeNum =
                                state.dataVariableSpeedCoils->VarSpeedCoil(thisCBVAV.DXCoolCoilIndexNum).CondenserInletNodeNum;
                        }
                    } else if (ActualCoolCoilType == HVAC::CoilDX_Cooling) {
                        thisCBVAV.DXCoolCoilIndexNum = CoilCoolingDX::factory(state, thisCBVAV.DXCoolCoilName);
                        if (thisCBVAV.DXCoolCoilIndexNum == -1) {
                            ShowContinueError(state, format("...occurs in {} \"{}\"", thisCBVAV.UnitType, thisCBVAV.Name));
                            ErrorsFound = true;
                        } else {
                            auto const &newCoil = state.dataCoilCoolingDX->coilCoolingDXs[thisCBVAV.DXCoolCoilIndexNum];
                            thisCBVAV.DXCoilInletNode = newCoil.evapInletNodeIndex;
                            thisCBVAV.DXCoilOutletNode = newCoil.evapOutletNodeIndex;
                            thisCBVAV.CondenserNodeNum = newCoil.condInletNodeIndex;
                        }
                    }
                }
            } else if (thisCBVAV.CoolCoilType == HVAC::CoilType::DXCoolingTwoStageWHumControl) {
                DXCoilErrFlag = false;
                DXCoils::GetDXCoilIndex(state, thisCBVAV.DXCoolCoilName, thisCBVAV.DXCoolCoilIndexNum, DXCoilErrFlag, thisCoolCoilType);
                if (DXCoilErrFlag) {
                    ShowContinueError(state, format("...occurs in {} \"{}\"", thisCBVAV.UnitType, thisCBVAV.Name));
                    ErrorsFound = true;
                } else {
                    thisCBVAV.DXCoilInletNode = state.dataDXCoils->DXCoil(thisCBVAV.DXCoolCoilIndexNum).AirInNode;
                    thisCBVAV.DXCoilOutletNode = state.dataDXCoils->DXCoil(thisCBVAV.DXCoolCoilIndexNum).AirOutNode;
                    thisCBVAV.CondenserNodeNum = state.dataDXCoils->DXCoil(thisCBVAV.DXCoolCoilIndexNum).CondenserInletNodeNum(1);
                }
            }

            thisCBVAV.fanOpModeSched = Sched::GetSchedule(state, Alphas(13));
            if (thisCBVAV.fanOpModeSched != nullptr) {
                if (!thisCBVAV.fanOpModeSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 1.0)) {
                    Sched::ShowSevereBadMinMax(state, eoh, cAlphaFields(13), Alphas(13), Clusive::In, 0.0, Clusive::In, 1.0);
                    ShowContinueError(state, "A value of 0 represents cycling fan mode, any other value up to 1 represents constant fan mode.");
                    ErrorsFound = true;
                }

                //     Check supply air fan operating mode for cycling fan, if NOT cycling fan set AirFlowControl
                if (!thisCBVAV.fanOpModeSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 0.0)) { // Autodesk:Note Range is 0 to 0?
                    //       set air flow control mode,
                    //       UseCompressorOnFlow  = operate at last cooling or heating air flow requested when compressor is off
                    //       UseCompressorOffFlow = operate at value specified by user (no input for this object type, UseCompONFlow)
                    //       AirFlowControl only valid if fan opmode = HVAC::FanOp::Continuous
                    thisCBVAV.AirFlowControl =
                        (thisCBVAV.MaxNoCoolHeatAirVolFlow == 0.0) ? AirFlowCtrlMode::UseCompressorOnFlow : AirFlowCtrlMode::UseCompressorOffFlow;
                }

            } else {
                if (!lAlphaBlanks(13)) {
                    ShowWarningError(state, format("{}: {}", CurrentModuleObject, thisCBVAV.Name));
                    ShowContinueError(state,
                                      format("{} = {} not found. Supply air fan operating mode set to constant operation and simulation continues.",
                                             cAlphaFields(13),
                                             Alphas(13)));
                }
                thisCBVAV.fanOp = HVAC::FanOp::Continuous;
                if (thisCBVAV.MaxNoCoolHeatAirVolFlow == 0.0) {
                    thisCBVAV.AirFlowControl = AirFlowCtrlMode::UseCompressorOnFlow;
                } else {
                    thisCBVAV.AirFlowControl = AirFlowCtrlMode::UseCompressorOffFlow;
                }
            }

            //   Check FanVolFlow, must be >= CBVAV flow
            if (thisCBVAV.FanVolFlow != DataSizing::AutoSize) {
                if (thisCBVAV.FanVolFlow < thisCBVAV.MaxNoCoolHeatAirVolFlow && thisCBVAV.MaxNoCoolHeatAirVolFlow != DataSizing::AutoSize &&
                    thisCBVAV.MaxNoCoolHeatAirVolFlow != 0.0) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in {} = {} is less than ",
                                            CurrentModuleObject,
                                            thisCBVAV.FanVolFlow,
                                            cAlphaFields(11),
                                            thisCBVAV.FanName) +
                                         cNumericFields(3));
                    ShowContinueError(state, format(" {} is reset to the fan flow rate and the simulation continues.", cNumericFields(3)));
                    ShowContinueError(state, format(" Occurs in {} = {}", CurrentModuleObject, thisCBVAV.Name));
                    thisCBVAV.MaxNoCoolHeatAirVolFlow = thisCBVAV.FanVolFlow;
                }
            }
            //   only check that OA flow when compressor is OFF is >= SA flow when compressor is OFF when both are not autosized and
            //   that MaxNoCoolHeatAirVolFlow is /= 0 (trigger to use compressor ON flow, see AirFlowControl variable initialization above)
            if (thisCBVAV.NoCoolHeatOutAirVolFlow > thisCBVAV.MaxNoCoolHeatAirVolFlow && thisCBVAV.NoCoolHeatOutAirVolFlow != DataSizing::AutoSize &&
                thisCBVAV.MaxNoCoolHeatAirVolFlow != DataSizing::AutoSize && thisCBVAV.MaxNoCoolHeatAirVolFlow != 0.0) {
                ShowWarningError(state, format("{}: {} cannot be greater than {}", CurrentModuleObject, cNumericFields(6), cNumericFields(3)));
                ShowContinueError(state, format(" {} is reset to the fan flow rate and the simulation continues.", cNumericFields(6)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, thisCBVAV.Name));
                thisCBVAV.NoCoolHeatOutAirVolFlow = thisCBVAV.FanVolFlow;
            }

            std::string thisHeatCoilType = Alphas(16);
            thisCBVAV.HeatCoilType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, thisHeatCoilType));
            thisCBVAV.HeatCoilName = Alphas(17);

            DXCoilErrFlag = false;
            if (thisCBVAV.HeatCoilType == HVAC::CoilType::DXHeatingEmpirical) {
                DXCoils::GetDXCoilIndex(state,
                                        thisCBVAV.HeatCoilName,
                                        thisCBVAV.DXHeatCoilIndexNum,
                                        DXCoilErrFlag,
                                        HVAC::coilTypeNamesUC[static_cast<int>(thisCBVAV.HeatCoilType)]);
                if (DXCoilErrFlag) {
                    ShowContinueError(state, format("...occurs in {} \"{}\"", thisCBVAV.UnitType, thisCBVAV.Name));
                    ErrorsFound = true;
                } else {
                    thisCBVAV.MinOATCompressor = state.dataDXCoils->DXCoil(thisCBVAV.DXHeatCoilIndexNum).MinOATCompressor;
                    thisCBVAV.HeatingCoilInletNode = state.dataDXCoils->DXCoil(thisCBVAV.DXHeatCoilIndexNum).AirInNode;
                    thisCBVAV.HeatingCoilOutletNode = state.dataDXCoils->DXCoil(thisCBVAV.DXHeatCoilIndexNum).AirOutNode;
                }
            } else if (thisCBVAV.HeatCoilType == HVAC::CoilType::HeatingAirToAirVariableSpeed) {
                thisCBVAV.DXHeatCoilIndexNum = VariableSpeedCoils::GetCoilIndexVariableSpeed(
                    state, HVAC::cAllCoilTypes(static_cast<int>(thisCBVAV.HeatCoilType) + 1), thisCBVAV.HeatCoilName, DXCoilErrFlag);
                if (DXCoilErrFlag) {
                    ShowContinueError(state, format("...occurs in {} \"{}\"", thisCBVAV.UnitType, thisCBVAV.Name));
                    ErrorsFound = true;
                } else {
                    thisCBVAV.MinOATCompressor = state.dataVariableSpeedCoils->VarSpeedCoil(thisCBVAV.DXHeatCoilIndexNum).MinOATCompressor;
                    thisCBVAV.HeatingCoilInletNode = state.dataVariableSpeedCoils->VarSpeedCoil(thisCBVAV.DXHeatCoilIndexNum).AirInletNodeNum;
                    thisCBVAV.HeatingCoilOutletNode = state.dataVariableSpeedCoils->VarSpeedCoil(thisCBVAV.DXHeatCoilIndexNum).AirOutletNodeNum;
                }
            } else if (thisCBVAV.HeatCoilType == HVAC::CoilType::HeatingGasOrOtherFuel || thisCBVAV.HeatCoilType == HVAC::CoilType::HeatingElectric) {
                HeatingCoils::GetCoilIndex(state, thisCBVAV.HeatCoilName, thisCBVAV.DXHeatCoilIndexNum, DXCoilErrFlag);
                if (DXCoilErrFlag) {
                    ShowContinueError(state, format("...occurs in {} \"{}\"", thisCBVAV.UnitType, thisCBVAV.Name));
                    ErrorsFound = true;
                } else {
                    thisCBVAV.MinOATCompressor = -999.9;
                    thisCBVAV.HeatingCoilInletNode = state.dataHeatingCoils->HeatingCoil(thisCBVAV.DXHeatCoilIndexNum).AirInletNodeNum;
                    thisCBVAV.HeatingCoilOutletNode = state.dataHeatingCoils->HeatingCoil(thisCBVAV.DXHeatCoilIndexNum).AirOutletNodeNum;
                }
            } else if (thisCBVAV.HeatCoilType == HVAC::CoilType::HeatingWater) {
                thisCBVAV.DXHeatCoilIndexNum = WaterCoils::GetWaterCoilIndex(state, "COIL:HEATING:WATER", thisCBVAV.HeatCoilName, DXCoilErrFlag);
                if (DXCoilErrFlag) {
                    ShowContinueError(state, format("...occurs in {} \"{}\"", thisCBVAV.UnitType, thisCBVAV.Name));
                    ErrorsFound = true;
                } else {
                    thisCBVAV.CoilControlNode = state.dataWaterCoils->WaterCoil(thisCBVAV.DXHeatCoilIndexNum).WaterInletNodeNum;
                    thisCBVAV.MaxHeatCoilFluidFlow = state.dataWaterCoils->WaterCoil(thisCBVAV.DXHeatCoilIndexNum).MaxWaterVolFlowRate;
                    thisCBVAV.HeatingCoilInletNode = state.dataWaterCoils->WaterCoil(thisCBVAV.DXHeatCoilIndexNum).AirInletNodeNum;
                    thisCBVAV.HeatingCoilOutletNode = state.dataWaterCoils->WaterCoil(thisCBVAV.DXHeatCoilIndexNum).AirOutletNodeNum;
                }
            } else if (thisCBVAV.HeatCoilType == HVAC::CoilType::HeatingSteam) {
                thisCBVAV.HeatCoilIndex = SteamCoils::GetSteamCoilIndex(state, "COIL:HEATING:STEAM", thisCBVAV.HeatCoilName, DXCoilErrFlag);
                if (DXCoilErrFlag) {
                    ShowContinueError(state, format("...occurs in {} \"{}\"", thisCBVAV.UnitType, thisCBVAV.Name));
                    ErrorsFound = true;
                } else {
                    thisCBVAV.HeatingCoilInletNode = state.dataSteamCoils->SteamCoil(thisCBVAV.HeatCoilIndex).AirInletNodeNum;
                    thisCBVAV.HeatingCoilOutletNode = state.dataSteamCoils->SteamCoil(thisCBVAV.HeatCoilIndex).AirOutletNodeNum;
                    thisCBVAV.CoilControlNode = state.dataSteamCoils->SteamCoil(thisCBVAV.HeatCoilIndex).SteamInletNodeNum;
                    thisCBVAV.MaxHeatCoilFluidFlow = state.dataSteamCoils->SteamCoil(thisCBVAV.HeatCoilIndex).MaxSteamVolFlowRate;
                    Real64 SteamDensity = Fluid::GetSteam(state)->getSatDensity(
                        state, state.dataHVACUnitaryBypassVAV->TempSteamIn, 1.0, getUnitaryHeatCoolVAVChangeoverBypass);
                    if (thisCBVAV.MaxHeatCoilFluidFlow > 0.0) {
                        thisCBVAV.MaxHeatCoilFluidFlow = thisCBVAV.MaxHeatCoilFluidFlow * SteamDensity;
                    }
                }
            }

            if (thisCBVAV.DXCoilOutletNode != thisCBVAV.HeatingCoilInletNode) {
                ShowSevereError(state, format("{} illegal coil placement. Cooling coil must be upstream of heating coil.", CurrentModuleObject));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, thisCBVAV.Name));
                ErrorsFound = true;
            }

            if (thisCBVAV.fanPlace == HVAC::FanPlace::BlowThru) {
                if (thisCBVAV.SplitterOutletAirNode != thisCBVAV.HeatingCoilOutletNode) {
                    ShowSevereError(state, format("{}: {}", CurrentModuleObject, thisCBVAV.Name));
                    ShowContinueError(state, format("Illegal {} = {}.", cAlphaFields(6), SplitterOutletNodeName));
                    ShowContinueError(
                        state,
                        format(
                            "{} must be the same as the outlet node specified in the heating coil object = {}: {} when blow through {} is selected.",
                            cAlphaFields(6),
                            HVAC::coilTypeNamesUC[static_cast<int>(thisCBVAV.HeatCoilType)],
                            thisCBVAV.HeatCoilName,
                            cAlphaFields(12)));
                    ErrorsFound = true;
                }
                if (thisCBVAV.MixerMixedAirNode != thisCBVAV.FanInletNodeNum) {
                    ShowSevereError(state, format("{}: {}", CurrentModuleObject, thisCBVAV.Name));
                    ShowContinueError(state,
                                      format("Illegal {}. The fan inlet node name must be the same as the mixed air node specified in the {} = {} "
                                             "when blow through {} is selected.",
                                             cAlphaFields(11),
                                             cAlphaFields(9),
                                             thisCBVAV.OAMixName,
                                             cAlphaFields(12)));
                    ErrorsFound = true;
                }
            }

            if (thisCBVAV.fanPlace == HVAC::FanPlace::DrawThru) {
                if (thisCBVAV.MixerMixedAirNode != thisCBVAV.DXCoilInletNode) {
                    ShowSevereError(state, format("{}: {}", CurrentModuleObject, thisCBVAV.Name));
                    ShowContinueError(state,
                                      format("Illegal cooling coil placement. The cooling coil inlet node name must be the same as the mixed air "
                                             "node specified in the {} = {} when draw through {} is selected.",
                                             cAlphaFields(9),
                                             thisCBVAV.OAMixName,
                                             cAlphaFields(12)));
                    ErrorsFound = true;
                }
            }

            if (Util::SameString(Alphas(18), "CoolingPriority")) {
                thisCBVAV.PriorityControl = PriorityCtrlMode::CoolingPriority;
            } else if (Util::SameString(Alphas(18), "HeatingPriority")) {
                thisCBVAV.PriorityControl = PriorityCtrlMode::HeatingPriority;
            } else if (Util::SameString(Alphas(18), "ZonePriority")) {
                thisCBVAV.PriorityControl = PriorityCtrlMode::ZonePriority;
            } else if (Util::SameString(Alphas(18), "LoadPriority")) {
                thisCBVAV.PriorityControl = PriorityCtrlMode::LoadPriority;
            } else {
                ShowSevereError(state, format("{} illegal {} = {}", CurrentModuleObject, cAlphaFields(18), Alphas(18)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, thisCBVAV.Name));
                ShowContinueError(state, "Valid choices are CoolingPriority, HeatingPriority, ZonePriority or LoadPriority.");
                ErrorsFound = true;
            }

            if (Numbers(7) > 0.0) {
                thisCBVAV.MinLATCooling = Numbers(7);
            } else {
                thisCBVAV.MinLATCooling = 10.0;
            }

            if (Numbers(8) > 0.0) {
                thisCBVAV.MaxLATHeating = Numbers(8);
            } else {
                thisCBVAV.MaxLATHeating = 50.0;
            }

            if (thisCBVAV.MinLATCooling > thisCBVAV.MaxLATHeating) {
                ShowWarningError(state, format("{}: illegal leaving air temperature specified.", CurrentModuleObject));
                ShowContinueError(state, format("Resetting {} equal to {} and the simulation continues.", cNumericFields(7), cNumericFields(8)));
                ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, thisCBVAV.Name));
                thisCBVAV.MinLATCooling = thisCBVAV.MaxLATHeating;
            }

            // Dehumidification control mode
            if (Util::SameString(Alphas(19), "None")) {
                thisCBVAV.DehumidControlType = DehumidControl::None;
            } else if (Util::SameString(Alphas(19), "")) {
                thisCBVAV.DehumidControlType = DehumidControl::None;
            } else if (Util::SameString(Alphas(19), "Multimode")) {
                if (thisCBVAV.CoolCoilType == HVAC::CoilType::DXCoolingTwoStageWHumControl) {
                    thisCBVAV.DehumidControlType = DehumidControl::Multimode;
                } else {
                    ShowWarningError(state, format("Invalid {} = {}", cAlphaFields(19), Alphas(19)));
                    ShowContinueError(state, format("In {} \"{}\".", CurrentModuleObject, thisCBVAV.Name));
                    ShowContinueError(state, format("Valid only with {} = Coil:Cooling:DX:TwoStageWithHumidityControlMode.", cAlphaFields(14)));
                    ShowContinueError(state, format("Setting {} to \"None\" and the simulation continues.", cAlphaFields(19)));
                    thisCBVAV.DehumidControlType = DehumidControl::None;
                }
            } else if (Util::SameString(Alphas(19), "CoolReheat")) {
                if (thisCBVAV.CoolCoilType == HVAC::CoilType::DXCoolingTwoStageWHumControl) {
                    thisCBVAV.DehumidControlType = DehumidControl::CoolReheat;
                } else {
                    ShowWarningError(state, format("Invalid {} = {}", cAlphaFields(19), Alphas(19)));
                    ShowContinueError(state, format("In {} \"{}\".", CurrentModuleObject, thisCBVAV.Name));
                    ShowContinueError(state, format("Valid only with {} = Coil:Cooling:DX:TwoStageWithHumidityControlMode.", cAlphaFields(14)));
                    ShowContinueError(state, format("Setting {} to \"None\" and the simulation continues.", cAlphaFields(19)));
                    thisCBVAV.DehumidControlType = DehumidControl::None;
                }
            } else {
                ShowSevereError(state, format("Invalid {} ={}", cAlphaFields(19), Alphas(19)));
                ShowContinueError(state, format("In {} \"{}\".", CurrentModuleObject, thisCBVAV.Name));
            }

            if (NumNumbers > 8) {
                thisCBVAV.minModeChangeTime = Numbers(9);
            }

            //   Initialize last mode of compressor operation
            thisCBVAV.LastMode = HeatingMode;

            if (thisCBVAV.fanType == HVAC::FanType::OnOff || thisCBVAV.fanType == HVAC::FanType::Constant) {
                HVAC::FanType fanType2 = state.dataFans->fans(thisCBVAV.FanIndex)->type;
                if (thisCBVAV.fanType != fanType2) {
                    ShowWarningError(
                        state,
                        format("{} has {} = {} which is inconsistent with the fan object.", CurrentModuleObject, cAlphaFields(10), Alphas(10)));
                    ShowContinueError(state, format("Occurs in {} = {}", CurrentModuleObject, thisCBVAV.Name));
                    ShowContinueError(state,
                                      format(" The fan object ({}) is actually a valid fan type and the simulation continues.", thisCBVAV.FanName));
                    ShowContinueError(state, " Node connections errors may result due to the inconsistent fan type.");
                }
            }

            // Add fan to component sets array
            if (thisCBVAV.fanPlace == HVAC::FanPlace::BlowThru) {
                CompSetFanInlet = state.dataLoopNodes->NodeID(thisCBVAV.MixerMixedAirNode);
                CompSetFanOutlet = state.dataLoopNodes->NodeID(thisCBVAV.DXCoilInletNode);
            } else {
                CompSetFanInlet = state.dataLoopNodes->NodeID(thisCBVAV.HeatingCoilOutletNode);
                CompSetFanOutlet = SplitterOutletNodeName;
            }
            std::string CompSetCoolInlet = state.dataLoopNodes->NodeID(thisCBVAV.DXCoilInletNode);
            std::string CompSetCoolOutlet = state.dataLoopNodes->NodeID(thisCBVAV.DXCoilOutletNode);

            // Add fan to component sets array
            BranchNodeConnections::SetUpCompSets(
                state, thisCBVAV.UnitType, thisCBVAV.Name, Alphas(10), thisCBVAV.FanName, CompSetFanInlet, CompSetFanOutlet);

            // Add cooling coil to component sets array
            BranchNodeConnections::SetUpCompSets(state,
                                                 thisCBVAV.UnitType,
                                                 thisCBVAV.Name,
                                                 HVAC::coilTypeNamesUC[static_cast<int>(thisCBVAV.CoolCoilType)],
                                                 thisCBVAV.DXCoolCoilName,
                                                 CompSetCoolInlet,
                                                 CompSetCoolOutlet);

            // Add heating coil to component sets array
            BranchNodeConnections::SetUpCompSets(state,
                                                 thisCBVAV.UnitType,
                                                 thisCBVAV.Name,
                                                 HVAC::coilTypeNamesUC[static_cast<int>(thisCBVAV.HeatCoilType)],
                                                 thisCBVAV.HeatCoilName,
                                                 state.dataLoopNodes->NodeID(thisCBVAV.HeatingCoilInletNode),
                                                 state.dataLoopNodes->NodeID(thisCBVAV.HeatingCoilOutletNode));

            // Set up component set for OA mixer - use OA node and Mixed air node
            BranchNodeConnections::SetUpCompSets(state,
                                                 thisCBVAV.UnitType,
                                                 thisCBVAV.Name,
                                                 thisCBVAV.OAMixType,
                                                 thisCBVAV.OAMixName,
                                                 state.dataLoopNodes->NodeID(thisCBVAV.MixerOutsideAirNode),
                                                 state.dataLoopNodes->NodeID(thisCBVAV.MixerMixedAirNode));

            BranchNodeConnections::TestCompSet(state,
                                               thisCBVAV.UnitType,
                                               thisCBVAV.Name,
                                               state.dataLoopNodes->NodeID(thisCBVAV.AirInNode),
                                               state.dataLoopNodes->NodeID(thisCBVAV.AirOutNode),
                                               "Air Nodes");

            //   Find air loop associated with CBVAV system
            for (int AirLoopNum = 1; AirLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirLoopNum) {
                for (int BranchNum = 1; BranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).NumBranches; ++BranchNum) {
                    for (int CompNum = 1; CompNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).TotalComponents;
                         ++CompNum) {
                        if (!Util::SameString(state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).Name,
                                              thisCBVAV.Name) ||
                            !Util::SameString(state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).TypeOf,
                                              thisCBVAV.UnitType)) {
                            continue;
                        }
                        thisCBVAV.AirLoopNumber = AirLoopNum;
                        //         Should EXIT here or do other checking?
                        break;
                    }
                }
            }

            if (thisCBVAV.AirLoopNumber > 0) {
                thisCBVAV.NumControlledZones = state.dataAirLoop->AirToZoneNodeInfo(thisCBVAV.AirLoopNumber).NumZonesCooled;
                thisCBVAV.ControlledZoneNum.allocate(thisCBVAV.NumControlledZones);
                thisCBVAV.ControlledZoneNodeNum.allocate(thisCBVAV.NumControlledZones);
                thisCBVAV.CBVAVBoxOutletNode.allocate(thisCBVAV.NumControlledZones);
                thisCBVAV.ZoneSequenceCoolingNum.allocate(thisCBVAV.NumControlledZones);
                thisCBVAV.ZoneSequenceHeatingNum.allocate(thisCBVAV.NumControlledZones);

                thisCBVAV.ControlledZoneNum = 0;
                for (int AirLoopZoneNum = 1; AirLoopZoneNum <= state.dataAirLoop->AirToZoneNodeInfo(thisCBVAV.AirLoopNumber).NumZonesCooled;
                     ++AirLoopZoneNum) {
                    thisCBVAV.ControlledZoneNum(AirLoopZoneNum) =
                        state.dataAirLoop->AirToZoneNodeInfo(thisCBVAV.AirLoopNumber).CoolCtrlZoneNums(AirLoopZoneNum);
                    if (thisCBVAV.ControlledZoneNum(AirLoopZoneNum) > 0) {
                        thisCBVAV.ControlledZoneNodeNum(AirLoopZoneNum) =
                            state.dataZoneEquip->ZoneEquipConfig(thisCBVAV.ControlledZoneNum(AirLoopZoneNum)).ZoneNode;
                        thisCBVAV.CBVAVBoxOutletNode(AirLoopZoneNum) =
                            state.dataAirLoop->AirToZoneNodeInfo(thisCBVAV.AirLoopNumber).CoolZoneInletNodes(AirLoopZoneNum);
                        // check for thermostat in controlled zone
                        bool FoundTstatZone = false;
                        for (int TstatZoneNum = 1; TstatZoneNum <= state.dataZoneCtrls->NumTempControlledZones; ++TstatZoneNum) {
                            if (state.dataZoneCtrls->TempControlledZone(TstatZoneNum).ActualZoneNum != thisCBVAV.ControlledZoneNum(AirLoopZoneNum)) {
                                continue;
                            }
                            FoundTstatZone = true;
                        }
                        if (!FoundTstatZone) {
                            ShowWarningError(state, format("{} \"{}\"", CurrentModuleObject, thisCBVAV.Name));
                            ShowContinueError(state,
                                              format("Thermostat not found in zone = {} and the simulation continues.",
                                                     state.dataZoneEquip->ZoneEquipConfig(thisCBVAV.ControlledZoneNum(AirLoopZoneNum)).ZoneName));
                            ShowContinueError(state, "This zone will not be controlled to a temperature setpoint.");
                        }
                        int zoneNum = thisCBVAV.ControlledZoneNum(AirLoopZoneNum);
                        int zoneInlet = thisCBVAV.CBVAVBoxOutletNode(AirLoopZoneNum);
                        // setup zone equipment sequence information based on finding matching air terminal
                        if (state.dataZoneEquip->ZoneEquipConfig(zoneNum).EquipListIndex > 0) {
                            int coolingPriority = 0;
                            int heatingPriority = 0;
                            state.dataZoneEquip->ZoneEquipList(state.dataZoneEquip->ZoneEquipConfig(zoneNum).EquipListIndex)
                                .getPrioritiesForInletNode(state, zoneInlet, coolingPriority, heatingPriority);
                            thisCBVAV.ZoneSequenceCoolingNum(AirLoopZoneNum) = coolingPriority;
                            thisCBVAV.ZoneSequenceHeatingNum(AirLoopZoneNum) = heatingPriority;
                        }
                        if (thisCBVAV.ZoneSequenceCoolingNum(AirLoopZoneNum) == 0 || thisCBVAV.ZoneSequenceHeatingNum(AirLoopZoneNum) == 0) {
                            ShowSevereError(
                                state,
                                format("AirLoopHVAC:UnitaryHeatCool:VAVChangeoverBypass, \"{}\": Airloop air terminal in the zone equipment list for "
                                       "zone = {} not found or is not allowed Zone Equipment Cooling or Heating Sequence = 0.",
                                       thisCBVAV.Name,
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
            auto &thisCBVAV = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);
            SetupOutputVariable(state,
                                "Unitary System Total Heating Rate",
                                Constant::Units::W,
                                thisCBVAV.TotHeatEnergyRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCBVAV.Name);
            SetupOutputVariable(state,
                                "Unitary System Total Heating Energy",
                                Constant::Units::J,
                                thisCBVAV.TotHeatEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisCBVAV.Name);
            SetupOutputVariable(state,
                                "Unitary System Total Cooling Rate",
                                Constant::Units::W,
                                thisCBVAV.TotCoolEnergyRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCBVAV.Name);
            SetupOutputVariable(state,
                                "Unitary System Total Cooling Energy",
                                Constant::Units::J,
                                thisCBVAV.TotCoolEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisCBVAV.Name);
            SetupOutputVariable(state,
                                "Unitary System Sensible Heating Rate",
                                Constant::Units::W,
                                thisCBVAV.SensHeatEnergyRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCBVAV.Name);
            SetupOutputVariable(state,
                                "Unitary System Sensible Heating Energy",
                                Constant::Units::J,
                                thisCBVAV.SensHeatEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisCBVAV.Name);
            SetupOutputVariable(state,
                                "Unitary System Sensible Cooling Rate",
                                Constant::Units::W,
                                thisCBVAV.SensCoolEnergyRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCBVAV.Name);
            SetupOutputVariable(state,
                                "Unitary System Sensible Cooling Energy",
                                Constant::Units::J,
                                thisCBVAV.SensCoolEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisCBVAV.Name);
            SetupOutputVariable(state,
                                "Unitary System Latent Heating Rate",
                                Constant::Units::W,
                                thisCBVAV.LatHeatEnergyRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCBVAV.Name);
            SetupOutputVariable(state,
                                "Unitary System Latent Heating Energy",
                                Constant::Units::J,
                                thisCBVAV.LatHeatEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisCBVAV.Name);
            SetupOutputVariable(state,
                                "Unitary System Latent Cooling Rate",
                                Constant::Units::W,
                                thisCBVAV.LatCoolEnergyRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCBVAV.Name);
            SetupOutputVariable(state,
                                "Unitary System Latent Cooling Energy",
                                Constant::Units::J,
                                thisCBVAV.LatCoolEnergy,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisCBVAV.Name);
            SetupOutputVariable(state,
                                "Unitary System Electricity Rate",
                                Constant::Units::W,
                                thisCBVAV.ElecPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCBVAV.Name);
            SetupOutputVariable(state,
                                "Unitary System Electricity Energy",
                                Constant::Units::J,
                                thisCBVAV.ElecConsumption,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisCBVAV.Name);
            SetupOutputVariable(state,
                                "Unitary System Fan Part Load Ratio",
                                Constant::Units::None,
                                thisCBVAV.FanPartLoadRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCBVAV.Name);
            SetupOutputVariable(state,
                                "Unitary System Compressor Part Load Ratio",
                                Constant::Units::None,
                                thisCBVAV.CompPartLoadRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCBVAV.Name);
            SetupOutputVariable(state,
                                "Unitary System Bypass Air Mass Flow Rate",
                                Constant::Units::kg_s,
                                thisCBVAV.BypassMassFlowRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCBVAV.Name);
            SetupOutputVariable(state,
                                "Unitary System Air Outlet Setpoint Temperature",
                                Constant::Units::C,
                                thisCBVAV.OutletTempSetPoint,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCBVAV.Name);
            SetupOutputVariable(state,
                                "Unitary System Operating Mode Index",
                                Constant::Units::None,
                                thisCBVAV.HeatCoolMode,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisCBVAV.Name);
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

        auto &cBVAV = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);
        int NumCBVAV = state.dataHVACUnitaryBypassVAV->NumCBVAV;

        int InNode = cBVAV.AirInNode;
        int OutNode = cBVAV.AirOutNode;

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
            if ((cBVAV.HeatCoilType == HVAC::CoilType::HeatingWater) || (cBVAV.HeatCoilType == HVAC::CoilType::HeatingSteam)) {
                bool ErrorsFound = false; // Set to true if errors in input, fatal at end of routine
                if (cBVAV.HeatCoilType == HVAC::CoilType::HeatingWater) {

                    ErrorFlag = false;
                    PlantUtilities::ScanPlantLoopsForObject(
                        state, cBVAV.HeatCoilName, DataPlant::PlantEquipmentType::CoilWaterSimpleHeating, cBVAV.plantLoc, ErrorFlag, _, _, _, _, _);
                    if (ErrorFlag) {
                        ShowFatalError(state, "InitCBVAV: Program terminated for previous conditions.");
                    }

                    cBVAV.MaxHeatCoilFluidFlow = WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", cBVAV.HeatCoilName, ErrorsFound);

                    if (cBVAV.MaxHeatCoilFluidFlow > 0.0) {
                        Real64 FluidDensity =
                            state.dataPlnt->PlantLoop(cBVAV.plantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                        cBVAV.MaxHeatCoilFluidFlow =
                            WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", cBVAV.HeatCoilName, ErrorsFound) * FluidDensity;
                    }

                } else if (cBVAV.HeatCoilType == HVAC::CoilType::HeatingSteam) {

                    ErrorFlag = false;
                    PlantUtilities::ScanPlantLoopsForObject(
                        state, cBVAV.HeatCoilName, DataPlant::PlantEquipmentType::CoilSteamAirHeating, cBVAV.plantLoc, ErrorFlag, _, _, _, _, _);

                    if (ErrorFlag) {
                        ShowFatalError(state, "InitCBVAV: Program terminated for previous conditions.");
                    }

                    cBVAV.MaxHeatCoilFluidFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, cBVAV.HeatCoilIndex, ErrorsFound);

                    if (cBVAV.MaxHeatCoilFluidFlow > 0.0) {
                        // Why is TempSteamIn a state variable of the entire module?
                        Real64 FluidDensity =
                            Fluid::GetSteam(state)->getSatDensity(state, state.dataHVACUnitaryBypassVAV->TempSteamIn, 1.0, RoutineName);

                        cBVAV.MaxHeatCoilFluidFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, cBVAV.HeatCoilIndex, ErrorsFound) * FluidDensity;
                    }
                }

                if (ErrorsFound) {
                    ShowContinueError(state, format("Occurs in {} = {}", "AirLoopHVAC:UnitaryHeatCool:VAVChangeoverBypass", cBVAV.Name));
                }
                // fill outlet node for heating coil
                cBVAV.CoilOutletNode = DataPlant::CompData::getPlantComponent(state, cBVAV.plantLoc).NodeNumOut;
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
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).cycFanSched = cBVAV.fanOpModeSched;
            //   Set UnitarySys flag to FALSE and let the heating coil autosize independently of the cooling coil
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).UnitarySys = false;
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).fanOp = cBVAV.fanOp;
            // check for set point manager on outlet node of CBVAV
            cBVAV.OutNodeSPMIndex = SetPointManager::GetSetPointManagerIndexByNode(state,
                                                                                   OutNode,
                                                                                   HVAC::CtrlVarType::Temp,
                                                                                   SetPointManager::SPMType::MixedAir,
                                                                                   true); // isRefNode
            state.dataHVACUnitaryBypassVAV->MySizeFlag(CBVAVNum) = false;
        }

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && state.dataHVACUnitaryBypassVAV->MyEnvrnFlag(CBVAVNum)) {
            int MixerOutsideAirNode = cBVAV.MixerOutsideAirNode;
            Real64 RhoAir = state.dataEnvrn->StdRhoAir;
            // set the mass flow rates from the input volume flow rates
            cBVAV.MaxCoolAirMassFlow = RhoAir * cBVAV.MaxCoolAirVolFlow;
            cBVAV.CoolOutAirMassFlow = RhoAir * cBVAV.CoolOutAirVolFlow;
            cBVAV.MaxHeatAirMassFlow = RhoAir * cBVAV.MaxHeatAirVolFlow;
            cBVAV.HeatOutAirMassFlow = RhoAir * cBVAV.HeatOutAirVolFlow;
            cBVAV.MaxNoCoolHeatAirMassFlow = RhoAir * cBVAV.MaxNoCoolHeatAirVolFlow;
            cBVAV.NoCoolHeatOutAirMassFlow = RhoAir * cBVAV.NoCoolHeatOutAirVolFlow;
            // set the node max and min mass flow rates
            state.dataLoopNodes->Node(MixerOutsideAirNode).MassFlowRateMax = max(cBVAV.CoolOutAirMassFlow, cBVAV.HeatOutAirMassFlow);
            state.dataLoopNodes->Node(MixerOutsideAirNode).MassFlowRateMaxAvail = max(cBVAV.CoolOutAirMassFlow, cBVAV.HeatOutAirMassFlow);
            state.dataLoopNodes->Node(MixerOutsideAirNode).MassFlowRateMin = 0.0;
            state.dataLoopNodes->Node(MixerOutsideAirNode).MassFlowRateMinAvail = 0.0;
            state.dataLoopNodes->Node(InNode).MassFlowRateMax = max(cBVAV.MaxCoolAirMassFlow, cBVAV.MaxHeatAirMassFlow);
            state.dataLoopNodes->Node(InNode).MassFlowRateMaxAvail = max(cBVAV.MaxCoolAirMassFlow, cBVAV.MaxHeatAirMassFlow);
            state.dataLoopNodes->Node(InNode).MassFlowRateMin = 0.0;
            state.dataLoopNodes->Node(InNode).MassFlowRateMinAvail = 0.0;
            state.dataLoopNodes->Node(OutNode).Temp = state.dataLoopNodes->Node(InNode).Temp;
            state.dataLoopNodes->Node(OutNode).HumRat = state.dataLoopNodes->Node(InNode).HumRat;
            state.dataLoopNodes->Node(OutNode).Enthalpy = state.dataLoopNodes->Node(InNode).Enthalpy;
            state.dataLoopNodes->Node(cBVAV.MixerReliefAirNode) = state.dataLoopNodes->Node(MixerOutsideAirNode);
            state.dataHVACUnitaryBypassVAV->MyEnvrnFlag(CBVAVNum) = false;
            cBVAV.LastMode = HeatingMode;
            cBVAV.changeOverTimer = -1.0;
            //   set fluid-side hardware limits
            if (cBVAV.CoilControlNode > 0) {
                //    If water coil max water flow rate is autosized, simulate once in order to mine max water flow rate
                if (cBVAV.MaxHeatCoilFluidFlow == DataSizing::AutoSize) {
                    if (cBVAV.HeatCoilType == HVAC::CoilType::HeatingWater) {
                        WaterCoils::SimulateWaterCoilComponents(state, cBVAV.HeatCoilName, FirstHVACIteration, cBVAV.HeatCoilIndex);
                        ErrorFlag = false;
                        Real64 CoilMaxVolFlowRate = WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", cBVAV.HeatCoilName, ErrorFlag);
                        if (ErrorFlag) {
                            ShowContinueError(state, format("Occurs in {} = {}", "AirLoopHVAC:UnitaryHeatCool:VAVChangeoverBypass", cBVAV.Name));
                        }
                        if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                            Real64 FluidDensity =
                                state.dataPlnt->PlantLoop(cBVAV.plantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                            cBVAV.MaxHeatCoilFluidFlow = CoilMaxVolFlowRate * FluidDensity;
                        }
                    }
                    if (cBVAV.HeatCoilType == HVAC::CoilType::HeatingSteam) {
                        SteamCoils::SimulateSteamCoilComponents(state,
                                                                cBVAV.HeatCoilName,
                                                                FirstHVACIteration,
                                                                cBVAV.HeatCoilIndex,
                                                                1.0,
                                                                QCoilActual); // QCoilReq, simulate any load > 0 to get max capacity of steam coil
                        ErrorFlag = false;
                        Real64 CoilMaxVolFlowRate = SteamCoils::GetCoilMaxSteamFlowRate(state, cBVAV.HeatCoilIndex, ErrorFlag);
                        if (ErrorFlag) {
                            ShowContinueError(state, format("Occurs in {} = {}", "AirLoopHVAC:UnitaryHeatCool:VAVChangeoverBypass", cBVAV.Name));
                        }
                        if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                            Real64 FluidDensity =
                                Fluid::GetSteam(state)->getSatDensity(state, state.dataHVACUnitaryBypassVAV->TempSteamIn, 1.0, RoutineName);
                            cBVAV.MaxHeatCoilFluidFlow = CoilMaxVolFlowRate * FluidDensity;
                        }
                    }
                } // end of IF(cBVAV%MaxHeatCoilFluidFlow .EQ. DataSizing::AutoSize)THEN

                PlantUtilities::InitComponentNodes(state, 0.0, cBVAV.MaxHeatCoilFluidFlow, cBVAV.CoilControlNode, cBVAV.CoilOutletNode);

            } // end of IF(cBVAV%CoilControlNode .GT. 0)THEN
        } // end one time inits

        if (!state.dataGlobal->BeginEnvrnFlag) {
            state.dataHVACUnitaryBypassVAV->MyEnvrnFlag(CBVAVNum) = true;
        }

        // IF CBVAV system was not autosized and the fan is autosized, check that fan volumetric flow rate is greater than CBVAV flow rates
        if (cBVAV.CheckFanFlow) {

            if (!state.dataGlobal->DoingSizing && cBVAV.FanVolFlow != DataSizing::AutoSize) {
                std::string CurrentModuleObject = "AirLoopHVAC:UnitaryHeatCool:VAVChangeoverBypass";
                //     Check fan versus system supply air flow rates
                if (cBVAV.FanVolFlow < cBVAV.MaxCoolAirVolFlow) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the maximum CBVAV system air flow rate when "
                                            "cooling is required ({:.7T}).",
                                            CurrentModuleObject,
                                            cBVAV.FanVolFlow,
                                            cBVAV.FanName,
                                            cBVAV.MaxCoolAirVolFlow));
                    ShowContinueError(
                        state, " The CBVAV system flow rate when cooling is required is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state, format(" Occurs in Changeover-bypass VAV system = {}", cBVAV.Name));
                    cBVAV.MaxCoolAirVolFlow = cBVAV.FanVolFlow;
                }
                if (cBVAV.FanVolFlow < cBVAV.MaxHeatAirVolFlow) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the maximum CBVAV system air flow rate when "
                                            "heating is required ({:.7T}).",
                                            CurrentModuleObject,
                                            cBVAV.FanVolFlow,
                                            cBVAV.FanName,
                                            cBVAV.MaxHeatAirVolFlow));
                    ShowContinueError(
                        state, " The CBVAV system flow rate when heating is required is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state, format(" Occurs in Changeover-bypass VAV system = {}", cBVAV.Name));
                    cBVAV.MaxHeatAirVolFlow = cBVAV.FanVolFlow;
                }
                if (cBVAV.FanVolFlow < cBVAV.MaxNoCoolHeatAirVolFlow && cBVAV.MaxNoCoolHeatAirVolFlow != 0.0) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the maximum CBVAV system air flow rate when "
                                            "no heating or cooling is needed ({:.7T}).",
                                            CurrentModuleObject,
                                            cBVAV.FanVolFlow,
                                            cBVAV.FanName,
                                            cBVAV.MaxNoCoolHeatAirVolFlow));
                    ShowContinueError(state,
                                      " The CBVAV system flow rate when no heating or cooling is needed is reset to the fan flow rate and the "
                                      "simulation continues.");
                    ShowContinueError(state, format(" Occurs in Changeover-bypass VAV system = {}", cBVAV.Name));
                    cBVAV.MaxNoCoolHeatAirVolFlow = cBVAV.FanVolFlow;
                }
                //     Check fan versus outdoor air flow rates
                if (cBVAV.FanVolFlow < cBVAV.CoolOutAirVolFlow) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the maximum CBVAV outdoor air flow rate when "
                                            "cooling is required ({:.7T}).",
                                            CurrentModuleObject,
                                            cBVAV.FanVolFlow,
                                            cBVAV.FanName,
                                            cBVAV.CoolOutAirVolFlow));
                    ShowContinueError(
                        state, " The CBVAV outdoor flow rate when cooling is required is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state, format(" Occurs in Changeover-bypass VAV system = {}", cBVAV.Name));
                    cBVAV.CoolOutAirVolFlow = cBVAV.FanVolFlow;
                }
                if (cBVAV.FanVolFlow < cBVAV.HeatOutAirVolFlow) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the maximum CBVAV outdoor air flow rate when "
                                            "heating is required ({:.7T}).",
                                            CurrentModuleObject,
                                            cBVAV.FanVolFlow,
                                            cBVAV.FanName,
                                            cBVAV.HeatOutAirVolFlow));
                    ShowContinueError(
                        state, " The CBVAV outdoor flow rate when heating is required is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state, format(" Occurs in Changeover-bypass VAV system = {}", cBVAV.Name));
                    cBVAV.HeatOutAirVolFlow = cBVAV.FanVolFlow;
                }
                if (cBVAV.FanVolFlow < cBVAV.NoCoolHeatOutAirVolFlow) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the maximum CBVAV outdoor air flow rate when "
                                            "no heating or cooling is needed ({:.7T}).",
                                            CurrentModuleObject,
                                            cBVAV.FanVolFlow,
                                            cBVAV.FanName,
                                            cBVAV.NoCoolHeatOutAirVolFlow));
                    ShowContinueError(state,
                                      " The CBVAV outdoor flow rate when no heating or cooling is needed is reset to the fan flow rate and the "
                                      "simulation continues.");
                    ShowContinueError(state, format(" Occurs in Changeover-bypass VAV system = {}", cBVAV.Name));
                    cBVAV.NoCoolHeatOutAirVolFlow = cBVAV.FanVolFlow;
                }
                int MixerOutsideAirNode = cBVAV.MixerOutsideAirNode;
                Real64 RhoAir = state.dataEnvrn->StdRhoAir;
                // set the mass flow rates from the reset volume flow rates
                cBVAV.MaxCoolAirMassFlow = RhoAir * cBVAV.MaxCoolAirVolFlow;
                cBVAV.CoolOutAirMassFlow = RhoAir * cBVAV.CoolOutAirVolFlow;
                cBVAV.MaxHeatAirMassFlow = RhoAir * cBVAV.MaxHeatAirVolFlow;
                cBVAV.HeatOutAirMassFlow = RhoAir * cBVAV.HeatOutAirVolFlow;
                cBVAV.MaxNoCoolHeatAirMassFlow = RhoAir * cBVAV.MaxNoCoolHeatAirVolFlow;
                cBVAV.NoCoolHeatOutAirMassFlow = RhoAir * cBVAV.NoCoolHeatOutAirVolFlow;
                // set the node max and min mass flow rates based on reset volume flow rates
                state.dataLoopNodes->Node(MixerOutsideAirNode).MassFlowRateMax = max(cBVAV.CoolOutAirMassFlow, cBVAV.HeatOutAirMassFlow);
                state.dataLoopNodes->Node(MixerOutsideAirNode).MassFlowRateMaxAvail = max(cBVAV.CoolOutAirMassFlow, cBVAV.HeatOutAirMassFlow);
                state.dataLoopNodes->Node(MixerOutsideAirNode).MassFlowRateMin = 0.0;
                state.dataLoopNodes->Node(MixerOutsideAirNode).MassFlowRateMinAvail = 0.0;
                state.dataLoopNodes->Node(InNode).MassFlowRateMax = max(cBVAV.MaxCoolAirMassFlow, cBVAV.MaxHeatAirMassFlow);
                state.dataLoopNodes->Node(InNode).MassFlowRateMaxAvail = max(cBVAV.MaxCoolAirMassFlow, cBVAV.MaxHeatAirMassFlow);
                state.dataLoopNodes->Node(InNode).MassFlowRateMin = 0.0;
                state.dataLoopNodes->Node(InNode).MassFlowRateMinAvail = 0.0;
                state.dataLoopNodes->Node(OutNode).Temp = state.dataLoopNodes->Node(InNode).Temp;
                state.dataLoopNodes->Node(OutNode).HumRat = state.dataLoopNodes->Node(InNode).HumRat;
                state.dataLoopNodes->Node(OutNode).Enthalpy = state.dataLoopNodes->Node(InNode).Enthalpy;
                state.dataLoopNodes->Node(cBVAV.MixerReliefAirNode) = state.dataLoopNodes->Node(MixerOutsideAirNode);
                cBVAV.CheckFanFlow = false;
                if (cBVAV.FanVolFlow > 0.0) {
                    cBVAV.HeatingSpeedRatio = cBVAV.MaxHeatAirVolFlow / cBVAV.FanVolFlow;
                    cBVAV.CoolingSpeedRatio = cBVAV.MaxCoolAirVolFlow / cBVAV.FanVolFlow;
                    cBVAV.NoHeatCoolSpeedRatio = cBVAV.MaxNoCoolHeatAirVolFlow / cBVAV.FanVolFlow;
                }
            }
        }

        if (cBVAV.fanOpModeSched != nullptr) {
            cBVAV.fanOp = (cBVAV.fanOpModeSched->getCurrentVal() == 0.0) ? HVAC::FanOp::Cycling : HVAC::FanOp::Continuous;
        }

        // Returns load only for zones requesting cooling (heating). If in deadband, Qzoneload = 0.
        if (FirstHVACIteration) {
            cBVAV.modeChanged = false;
        }
        GetZoneLoads(state, CBVAVNum);

        OutsideAirMultiplier = (cBVAV.outAirSched != nullptr) ? cBVAV.outAirSched->getCurrentVal() : 1.0;

        // Set the inlet node mass flow rate
        if (cBVAV.fanOp == HVAC::FanOp::Continuous) {
            // constant fan mode
            if (cBVAV.HeatCoolMode == HeatingMode) {
                state.dataHVACUnitaryBypassVAV->CompOnMassFlow = cBVAV.MaxHeatAirMassFlow;
                state.dataHVACUnitaryBypassVAV->CompOnFlowRatio = cBVAV.HeatingSpeedRatio;
                state.dataHVACUnitaryBypassVAV->OACompOnMassFlow = cBVAV.HeatOutAirMassFlow * OutsideAirMultiplier;
            } else if (cBVAV.HeatCoolMode == CoolingMode) {
                state.dataHVACUnitaryBypassVAV->CompOnMassFlow = cBVAV.MaxCoolAirMassFlow;
                state.dataHVACUnitaryBypassVAV->CompOnFlowRatio = cBVAV.CoolingSpeedRatio;
                state.dataHVACUnitaryBypassVAV->OACompOnMassFlow = cBVAV.CoolOutAirMassFlow * OutsideAirMultiplier;
            } else {
                state.dataHVACUnitaryBypassVAV->CompOnMassFlow = cBVAV.MaxNoCoolHeatAirMassFlow;
                state.dataHVACUnitaryBypassVAV->CompOnFlowRatio = cBVAV.NoHeatCoolSpeedRatio;
                state.dataHVACUnitaryBypassVAV->OACompOnMassFlow = cBVAV.NoCoolHeatOutAirMassFlow * OutsideAirMultiplier;
            }

            if (cBVAV.AirFlowControl == AirFlowCtrlMode::UseCompressorOnFlow) {
                if (cBVAV.LastMode == HeatingMode) {
                    state.dataHVACUnitaryBypassVAV->CompOffMassFlow = cBVAV.MaxHeatAirMassFlow;
                    state.dataHVACUnitaryBypassVAV->CompOffFlowRatio = cBVAV.HeatingSpeedRatio;
                    state.dataHVACUnitaryBypassVAV->OACompOffMassFlow = cBVAV.HeatOutAirMassFlow * OutsideAirMultiplier;
                } else {
                    state.dataHVACUnitaryBypassVAV->CompOffMassFlow = cBVAV.MaxCoolAirMassFlow;
                    state.dataHVACUnitaryBypassVAV->CompOffFlowRatio = cBVAV.CoolingSpeedRatio;
                    state.dataHVACUnitaryBypassVAV->OACompOffMassFlow = cBVAV.CoolOutAirMassFlow * OutsideAirMultiplier;
                }
            } else {
                state.dataHVACUnitaryBypassVAV->CompOffMassFlow = cBVAV.MaxNoCoolHeatAirMassFlow;
                state.dataHVACUnitaryBypassVAV->CompOffFlowRatio = cBVAV.NoHeatCoolSpeedRatio;
                state.dataHVACUnitaryBypassVAV->OACompOffMassFlow = cBVAV.NoCoolHeatOutAirMassFlow * OutsideAirMultiplier;
            }
        } else {
            // cycling fan mode
            if (cBVAV.HeatCoolMode == HeatingMode) {
                state.dataHVACUnitaryBypassVAV->CompOnMassFlow = cBVAV.MaxHeatAirMassFlow;
                state.dataHVACUnitaryBypassVAV->CompOnFlowRatio = cBVAV.HeatingSpeedRatio;
                state.dataHVACUnitaryBypassVAV->OACompOnMassFlow = cBVAV.HeatOutAirMassFlow * OutsideAirMultiplier;
            } else if (cBVAV.HeatCoolMode == CoolingMode) {
                state.dataHVACUnitaryBypassVAV->CompOnMassFlow = cBVAV.MaxCoolAirMassFlow;
                state.dataHVACUnitaryBypassVAV->CompOnFlowRatio = cBVAV.CoolingSpeedRatio;
                state.dataHVACUnitaryBypassVAV->OACompOnMassFlow = cBVAV.CoolOutAirMassFlow * OutsideAirMultiplier;
            } else {
                state.dataHVACUnitaryBypassVAV->CompOnMassFlow = cBVAV.MaxCoolAirMassFlow;
                state.dataHVACUnitaryBypassVAV->CompOnFlowRatio = cBVAV.CoolingSpeedRatio;
                state.dataHVACUnitaryBypassVAV->OACompOnMassFlow = cBVAV.CoolOutAirMassFlow * OutsideAirMultiplier;
            }
            state.dataHVACUnitaryBypassVAV->CompOffMassFlow = 0.0;
            state.dataHVACUnitaryBypassVAV->CompOffFlowRatio = 0.0;
            state.dataHVACUnitaryBypassVAV->OACompOffMassFlow = 0.0;
        }

        // Check for correct control node at outlet of unit
        if (cBVAV.HumRatMaxCheck) {
            if (cBVAV.DehumidControlType != DehumidControl::None) {
                if (state.dataLoopNodes->Node(OutNode).HumRatMax == DataLoopNode::SensedNodeFlagValue) {
                    if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                        ShowWarningError(state, format("Unitary System:VAV:ChangeOverBypass = {}", cBVAV.Name));
                        ShowContinueError(state,
                                          "Use SetpointManager:SingleZone:Humidity:Maximum to place a humidity setpoint at the air outlet node of "
                                          "the unitary system.");
                        ShowContinueError(state, "Setting Dehumidification Control Type to None and simulation continues.");
                        cBVAV.DehumidControlType = DehumidControl::None;
                    } else {
                        // need call to EMS to check node
                        bool EMSSetPointCheck = false;
                        EMSManager::CheckIfNodeSetPointManagedByEMS(state, OutNode, HVAC::CtrlVarType::MaxHumRat, EMSSetPointCheck);
                        state.dataLoopNodes->NodeSetpointCheck(OutNode).needsSetpointChecking = false;
                        if (EMSSetPointCheck) {
                            // There is no plugin anyways, so we now we have a bad condition.
                            ShowWarningError(state, format("Unitary System:VAV:ChangeOverBypass = {}", cBVAV.Name));
                            ShowContinueError(state,
                                              "Use SetpointManager:SingleZone:Humidity:Maximum to place a humidity setpoint at the air outlet node "
                                              "of the unitary system.");
                            ShowContinueError(
                                state, "Or use an EMS Actuator to place a maximum humidity setpoint at the air outlet node of the unitary system.");
                            ShowContinueError(state, "Setting Dehumidification Control Type to None and simulation continues.");
                            cBVAV.DehumidControlType = DehumidControl::None;
                        }
                    }
                }
                cBVAV.HumRatMaxCheck = false;
            } else {
                cBVAV.HumRatMaxCheck = false;
            }
        }

        // Set the inlet node mass flow rate
        if (cBVAV.availSched->getCurrentVal() > 0.0 && state.dataHVACUnitaryBypassVAV->CompOnMassFlow != 0.0) {
            OnOffAirFlowRatio = 1.0;
            if (FirstHVACIteration) {
                state.dataLoopNodes->Node(cBVAV.AirInNode).MassFlowRate = state.dataHVACUnitaryBypassVAV->CompOnMassFlow;
                state.dataLoopNodes->Node(cBVAV.MixerInletAirNode).MassFlowRate = state.dataHVACUnitaryBypassVAV->CompOnMassFlow;
                state.dataLoopNodes->Node(cBVAV.MixerOutsideAirNode).MassFlowRate = state.dataHVACUnitaryBypassVAV->OACompOnMassFlow;
                state.dataLoopNodes->Node(cBVAV.MixerReliefAirNode).MassFlowRate = state.dataHVACUnitaryBypassVAV->OACompOnMassFlow;
                state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction = 0.0;
                state.dataHVACUnitaryBypassVAV->PartLoadFrac = 0.0;
            } else {
                if (cBVAV.HeatCoolMode != 0) {
                    state.dataHVACUnitaryBypassVAV->PartLoadFrac = 1.0;
                } else {
                    state.dataHVACUnitaryBypassVAV->PartLoadFrac = 0.0;
                }
                if (cBVAV.fanOp == HVAC::FanOp::Cycling) {
                    state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction = 0.0;
                } else {
                    if (cBVAV.PlenumMixerInletAirNode == 0) {
                        state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction = max(
                            0.0, 1.0 - (state.dataLoopNodes->Node(cBVAV.AirInNode).MassFlowRate / state.dataHVACUnitaryBypassVAV->CompOnMassFlow));
                    }
                }
            }
        } else {
            state.dataHVACUnitaryBypassVAV->PartLoadFrac = 0.0;
            state.dataLoopNodes->Node(cBVAV.AirInNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(cBVAV.AirOutNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(cBVAV.AirOutNode).MassFlowRateMaxAvail = 0.0;

            state.dataLoopNodes->Node(cBVAV.MixerInletAirNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(cBVAV.MixerOutsideAirNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(cBVAV.MixerReliefAirNode).MassFlowRate = 0.0;

            OnOffAirFlowRatio = 1.0;
            state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction = 0.0;
        }

        CalcCBVAV(state, CBVAVNum, FirstHVACIteration, state.dataHVACUnitaryBypassVAV->PartLoadFrac, QSensUnitOut, OnOffAirFlowRatio, HXUnitOn);

        // If unit is scheduled OFF, setpoint is equal to inlet node temperature.
        if (cBVAV.availSched->getCurrentVal() == 0.0) {
            cBVAV.OutletTempSetPoint = state.dataLoopNodes->Node(InNode).Temp;
            return;
        }

        SetAverageAirFlow(state, CBVAVNum, OnOffAirFlowRatio);

        if (FirstHVACIteration) {
            cBVAV.OutletTempSetPoint = CalcSetPointTempTarget(state, CBVAVNum);
        }

        // The setpoint is used to control the DX coils at their respective outlet nodes (not the unit outlet), correct
        // for fan heat for draw thru units only (fan heat is included at the outlet of each coil when blowthru is used)
        cBVAV.CoilTempSetPoint = cBVAV.OutletTempSetPoint;
        if (cBVAV.fanPlace == HVAC::FanPlace::DrawThru) {
            cBVAV.CoilTempSetPoint -= (state.dataLoopNodes->Node(cBVAV.AirOutNode).Temp - state.dataLoopNodes->Node(cBVAV.FanInletNodeNum).Temp);
        }

        if (FirstHVACIteration) {
            if (cBVAV.HeatCoilType == HVAC::CoilType::HeatingWater) {
                WaterCoils::SimulateWaterCoilComponents(state, cBVAV.HeatCoilName, FirstHVACIteration, cBVAV.HeatCoilIndex);

                //     set air-side and steam-side mass flow rates
                state.dataLoopNodes->Node(cBVAV.HeatingCoilInletNode).MassFlowRate = state.dataHVACUnitaryBypassVAV->CompOnMassFlow;
                mdot = cBVAV.MaxHeatCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(state, mdot, cBVAV.CoilControlNode, cBVAV.CoilOutletNode, cBVAV.plantLoc);

                //     simulate water coil to find operating capacity
                WaterCoils::SimulateWaterCoilComponents(state, cBVAV.HeatCoilName, FirstHVACIteration, cBVAV.HeatCoilIndex, QCoilActual);
                cBVAV.DesignSuppHeatingCapacity = QCoilActual;

            } // from IF(MSHeatPump(MSHeatPumpNum)%SuppHeatCoilType == HVAC::Coil_HeatingWater) THEN

            if (cBVAV.HeatCoilType == HVAC::CoilType::HeatingSteam) {

                //     set air-side and steam-side mass flow rates
                state.dataLoopNodes->Node(cBVAV.HeatingCoilInletNode).MassFlowRate = state.dataHVACUnitaryBypassVAV->CompOnMassFlow;
                mdot = cBVAV.MaxHeatCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(state, mdot, cBVAV.CoilControlNode, cBVAV.CoilOutletNode, cBVAV.plantLoc);

                //     simulate steam coil to find operating capacity
                SteamCoils::SimulateSteamCoilComponents(state,
                                                        cBVAV.HeatCoilName,
                                                        FirstHVACIteration,
                                                        cBVAV.HeatCoilIndex,
                                                        1.0,
                                                        QCoilActual); // QCoilReq, simulate any load > 0 to get max capacity of steam coil
                cBVAV.DesignSuppHeatingCapacity = QCoilActual;

            } // from IF(cBVAV%HeatCoilType == HVAC::Coil_HeatingSteam) THEN
        } // from IF( FirstHVACIteration ) THEN

        if ((cBVAV.HeatCoolMode == 0 && cBVAV.fanOp == HVAC::FanOp::Cycling) || state.dataHVACUnitaryBypassVAV->CompOnMassFlow == 0.0) {
            state.dataHVACUnitaryBypassVAV->PartLoadFrac = 0.0;
            state.dataLoopNodes->Node(cBVAV.AirInNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(cBVAV.AirOutNode).MassFlowRateMaxAvail = 0.0;
            state.dataLoopNodes->Node(cBVAV.MixerInletAirNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(cBVAV.MixerOutsideAirNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(cBVAV.MixerReliefAirNode).MassFlowRate = 0.0;
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

        auto &cBVAV = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);

        if (curSysNum > 0 && curOASysNum == 0) {
            state.dataAirSystemsData->PrimaryAirSystems(curSysNum).supFanNum = cBVAV.FanIndex;
            state.dataAirSystemsData->PrimaryAirSystems(curSysNum).supFanType = cBVAV.fanType;
            state.dataAirSystemsData->PrimaryAirSystems(curSysNum).supFanPlace = cBVAV.fanPlace;
        }

        if (cBVAV.MaxCoolAirVolFlow == DataSizing::AutoSize) {

            if (curSysNum > 0) {

                CheckSysSizing(state, cBVAV.UnitType, cBVAV.Name);
                cBVAV.MaxCoolAirVolFlow = state.dataSize->FinalSysSizing(curSysNum).DesMainVolFlow;
                if (cBVAV.FanVolFlow < cBVAV.MaxCoolAirVolFlow && cBVAV.FanVolFlow != DataSizing::AutoSize) {
                    cBVAV.MaxCoolAirVolFlow = cBVAV.FanVolFlow;
                    ShowWarningError(state, format("{} \"{}\"", cBVAV.UnitType, cBVAV.Name));
                    ShowContinueError(state,
                                      "The CBVAV system supply air fan air flow rate is less than the autosized value for the maximum air flow rate "
                                      "in cooling mode. Consider autosizing the fan for this simulation.");
                    ShowContinueError(
                        state, "The maximum air flow rate in cooling mode is reset to the supply air fan flow rate and the simulation continues.");
                }
                if (cBVAV.MaxCoolAirVolFlow < HVAC::SmallAirVolFlow) {
                    cBVAV.MaxCoolAirVolFlow = 0.0;
                }
                BaseSizer::reportSizerOutput(state, cBVAV.UnitType, cBVAV.Name, "maximum cooling air flow rate [m3/s]", cBVAV.MaxCoolAirVolFlow);
            }
        }

        if (cBVAV.MaxHeatAirVolFlow == DataSizing::AutoSize) {

            if (curSysNum > 0) {

                CheckSysSizing(state, cBVAV.UnitType, cBVAV.Name);
                cBVAV.MaxHeatAirVolFlow = state.dataSize->FinalSysSizing(curSysNum).DesMainVolFlow;
                if (cBVAV.FanVolFlow < cBVAV.MaxHeatAirVolFlow && cBVAV.FanVolFlow != DataSizing::AutoSize) {
                    cBVAV.MaxHeatAirVolFlow = cBVAV.FanVolFlow;
                    ShowWarningError(state, format("{} \"{}\"", cBVAV.UnitType, cBVAV.Name));
                    ShowContinueError(state,
                                      "The CBVAV system supply air fan air flow rate is less than the autosized value for the maximum air flow rate "
                                      "in heating mode. Consider autosizing the fan for this simulation.");
                    ShowContinueError(
                        state, "The maximum air flow rate in heating mode is reset to the supply air fan flow rate and the simulation continues.");
                }
                if (cBVAV.MaxHeatAirVolFlow < HVAC::SmallAirVolFlow) {
                    cBVAV.MaxHeatAirVolFlow = 0.0;
                }
                BaseSizer::reportSizerOutput(state, cBVAV.UnitType, cBVAV.Name, "maximum heating air flow rate [m3/s]", cBVAV.MaxHeatAirVolFlow);
            }
        }

        if (cBVAV.MaxNoCoolHeatAirVolFlow == DataSizing::AutoSize) {

            if (curSysNum > 0) {

                CheckSysSizing(state, cBVAV.UnitType, cBVAV.Name);
                cBVAV.MaxNoCoolHeatAirVolFlow = state.dataSize->FinalSysSizing(curSysNum).DesMainVolFlow;
                if (cBVAV.FanVolFlow < cBVAV.MaxNoCoolHeatAirVolFlow && cBVAV.FanVolFlow != DataSizing::AutoSize) {
                    cBVAV.MaxNoCoolHeatAirVolFlow = cBVAV.FanVolFlow;
                    ShowWarningError(state, format("{} \"{}\"", cBVAV.UnitType, cBVAV.Name));
                    ShowContinueError(state,
                                      "The CBVAV system supply air fan air flow rate is less than the autosized value for the maximum air flow rate "
                                      "when no heating or cooling is needed. Consider autosizing the fan for this simulation.");
                    ShowContinueError(state,
                                      "The maximum air flow rate when no heating or cooling is needed is reset to the supply air fan flow rate and "
                                      "the simulation continues.");
                }
                if (cBVAV.MaxNoCoolHeatAirVolFlow < HVAC::SmallAirVolFlow) {
                    cBVAV.MaxNoCoolHeatAirVolFlow = 0.0;
                }

                BaseSizer::reportSizerOutput(
                    state, cBVAV.UnitType, cBVAV.Name, "maximum air flow rate when compressor/coil is off [m3/s]", cBVAV.MaxNoCoolHeatAirVolFlow);
            }
        }

        if (cBVAV.CoolOutAirVolFlow == DataSizing::AutoSize) {

            if (curSysNum > 0) {

                CheckSysSizing(state, cBVAV.UnitType, cBVAV.Name);
                cBVAV.CoolOutAirVolFlow = state.dataSize->FinalSysSizing(curSysNum).DesOutAirVolFlow;
                if (cBVAV.FanVolFlow < cBVAV.CoolOutAirVolFlow && cBVAV.FanVolFlow != DataSizing::AutoSize) {
                    cBVAV.CoolOutAirVolFlow = cBVAV.FanVolFlow;
                    ShowWarningError(state, format("{} \"{}\"", cBVAV.UnitType, cBVAV.Name));
                    ShowContinueError(state,
                                      "The CBVAV system supply air fan air flow rate is less than the autosized value for the outdoor air flow rate "
                                      "in cooling mode. Consider autosizing the fan for this simulation.");
                    ShowContinueError(
                        state, "The outdoor air flow rate in cooling mode is reset to the supply air fan flow rate and the simulation continues.");
                }
                if (cBVAV.CoolOutAirVolFlow < HVAC::SmallAirVolFlow) {
                    cBVAV.CoolOutAirVolFlow = 0.0;
                }
                BaseSizer::reportSizerOutput(
                    state, cBVAV.UnitType, cBVAV.Name, "maximum outside air flow rate in cooling [m3/s]", cBVAV.CoolOutAirVolFlow);
            }
        }

        if (cBVAV.HeatOutAirVolFlow == DataSizing::AutoSize) {

            if (curSysNum > 0) {

                CheckSysSizing(state, cBVAV.UnitType, cBVAV.Name);
                cBVAV.HeatOutAirVolFlow = state.dataSize->FinalSysSizing(curSysNum).DesOutAirVolFlow;
                if (cBVAV.FanVolFlow < cBVAV.HeatOutAirVolFlow && cBVAV.FanVolFlow != DataSizing::AutoSize) {
                    cBVAV.HeatOutAirVolFlow = cBVAV.FanVolFlow;
                    ShowContinueError(state,
                                      "The CBVAV system supply air fan air flow rate is less than the autosized value for the outdoor air flow rate "
                                      "in heating mode. Consider autosizing the fan for this simulation.");
                    ShowContinueError(
                        state, "The outdoor air flow rate in heating mode is reset to the supply air fan flow rate and the simulation continues.");
                }
                if (cBVAV.HeatOutAirVolFlow < HVAC::SmallAirVolFlow) {
                    cBVAV.HeatOutAirVolFlow = 0.0;
                }
                BaseSizer::reportSizerOutput(
                    state, cBVAV.UnitType, cBVAV.Name, "maximum outdoor air flow rate in heating [m3/s]", cBVAV.CoolOutAirVolFlow);
            }
        }

        if (cBVAV.NoCoolHeatOutAirVolFlow == DataSizing::AutoSize) {

            if (curSysNum > 0) {

                CheckSysSizing(state, cBVAV.UnitType, cBVAV.Name);
                cBVAV.NoCoolHeatOutAirVolFlow = state.dataSize->FinalSysSizing(curSysNum).DesOutAirVolFlow;
                if (cBVAV.FanVolFlow < cBVAV.NoCoolHeatOutAirVolFlow && cBVAV.FanVolFlow != DataSizing::AutoSize) {
                    cBVAV.NoCoolHeatOutAirVolFlow = cBVAV.FanVolFlow;
                    ShowContinueError(state,
                                      "The CBVAV system supply air fan air flow rate is less than the autosized value for the outdoor air flow rate "
                                      "when no heating or cooling is needed. Consider autosizing the fan for this simulation.");
                    ShowContinueError(state,
                                      "The outdoor air flow rate when no heating or cooling is needed is reset to the supply air fan flow rate and "
                                      "the simulation continues.");
                }
                if (cBVAV.NoCoolHeatOutAirVolFlow < HVAC::SmallAirVolFlow) {
                    cBVAV.NoCoolHeatOutAirVolFlow = 0.0;
                }
                BaseSizer::reportSizerOutput(
                    state, cBVAV.UnitType, cBVAV.Name, "maximum outdoor air flow rate when compressor is off [m3/s]", cBVAV.NoCoolHeatOutAirVolFlow);
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

        auto &cBVAV = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);

        if (cBVAV.availSched->getCurrentVal() == 0.0) {
            return;
        }

        // Get operating result
        PartLoadFrac = 1.0;
        CalcCBVAV(state, CBVAVNum, FirstHVACIteration, PartLoadFrac, FullOutput, OnOffAirFlowRatio, HXUnitOn);

        if ((state.dataLoopNodes->Node(cBVAV.AirOutNode).Temp - cBVAV.OutletTempSetPoint) > HVAC::SmallTempDiff && cBVAV.HeatCoolMode > 0 &&
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

        auto &cBVAV = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);

        int OutletNode = cBVAV.AirOutNode;
        int InletNode = cBVAV.AirInNode;
        if (cBVAV.CondenserNodeNum > 0) {
            OutdoorDryBulbTemp = state.dataLoopNodes->Node(cBVAV.CondenserNodeNum).Temp;
            OutdoorBaroPress = state.dataLoopNodes->Node(cBVAV.CondenserNodeNum).Press;
        } else {
            OutdoorDryBulbTemp = state.dataEnvrn->OutDryBulbTemp;
            OutdoorBaroPress = state.dataEnvrn->OutBaroPress;
        }

        state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = 0.0;

        // Bypass excess system air through bypass duct and calculate new mixed air conditions at OA mixer inlet node
        if (cBVAV.plenumIndex > 0 || cBVAV.mixerIndex > 0) {
            Real64 saveMixerInletAirNodeFlow = state.dataLoopNodes->Node(cBVAV.MixerInletAirNode).MassFlowRate;
            state.dataLoopNodes->Node(cBVAV.MixerInletAirNode) = state.dataLoopNodes->Node(InletNode);
            state.dataLoopNodes->Node(cBVAV.MixerInletAirNode).MassFlowRate = saveMixerInletAirNodeFlow;
        } else {
            state.dataLoopNodes->Node(cBVAV.MixerInletAirNode).Temp =
                (1.0 - state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction) * state.dataLoopNodes->Node(InletNode).Temp +
                state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction * state.dataLoopNodes->Node(OutletNode).Temp;
            state.dataLoopNodes->Node(cBVAV.MixerInletAirNode).HumRat =
                (1.0 - state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction) * state.dataLoopNodes->Node(InletNode).HumRat +
                state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction * state.dataLoopNodes->Node(OutletNode).HumRat;
            state.dataLoopNodes->Node(cBVAV.MixerInletAirNode).Enthalpy = Psychrometrics::PsyHFnTdbW(
                state.dataLoopNodes->Node(cBVAV.MixerInletAirNode).Temp, state.dataLoopNodes->Node(cBVAV.MixerInletAirNode).HumRat);
        }
        MixedAir::SimOAMixer(state, cBVAV.OAMixName, cBVAV.OAMixIndex);

        if (cBVAV.fanPlace == HVAC::FanPlace::BlowThru) {
            state.dataFans->fans(cBVAV.FanIndex)
                ->simulate(state, FirstHVACIteration, state.dataHVACUnitaryBypassVAV->FanSpeedRatio, _, 1.0 / OnOffAirFlowRatio);
        }
        // Simulate cooling coil if zone load is negative (cooling load)
        if (cBVAV.HeatCoolMode == CoolingMode) {
            if (OutdoorDryBulbTemp >= cBVAV.MinOATCompressor) {
                switch (cBVAV.CoolCoilType) {
                case HVAC::CoilType::DXCoolingHXAssisted: {
                    HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(state,
                                                                        cBVAV.DXCoolCoilName,
                                                                        FirstHVACIteration,
                                                                        HVAC::CompressorOp::On,
                                                                        PartLoadFrac,
                                                                        cBVAV.CoolCoilCompIndex,
                                                                        HVAC::FanOp::Continuous,
                                                                        HXUnitOn);
                    if (state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp <= cBVAV.CoilTempSetPoint) {
                        //         If coil inlet temp is already below the setpoint, simulated with coil off
                        PartLoadFrac = 0.0;
                        HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(state,
                                                                            cBVAV.DXCoolCoilName,
                                                                            FirstHVACIteration,
                                                                            HVAC::CompressorOp::Off,
                                                                            PartLoadFrac,
                                                                            cBVAV.CoolCoilCompIndex,
                                                                            HVAC::FanOp::Continuous,
                                                                            HXUnitOn);
                    } else if (state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).Temp < cBVAV.CoilTempSetPoint) {
                        auto f = [&state, CBVAVNum, FirstHVACIteration, HXUnitOn](Real64 const PartLoadFrac) {
                            auto &thisCBVAV = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);
                            HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(state,
                                                                                thisCBVAV.DXCoolCoilName,
                                                                                FirstHVACIteration,
                                                                                HVAC::CompressorOp::On,
                                                                                PartLoadFrac,
                                                                                thisCBVAV.CoolCoilCompIndex,
                                                                                HVAC::FanOp::Continuous,
                                                                                HXUnitOn);

                            Real64 OutletAirTemp = state.dataLoopNodes->Node(thisCBVAV.DXCoilOutletNode).Temp;
                            return thisCBVAV.CoilTempSetPoint - OutletAirTemp;
                        };
                        General::SolveRoot(state, HVAC::SmallTempDiff, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                        HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(state,
                                                                            cBVAV.DXCoolCoilName,
                                                                            FirstHVACIteration,
                                                                            HVAC::CompressorOp::On,
                                                                            PartLoadFrac,
                                                                            cBVAV.CoolCoilCompIndex,
                                                                            HVAC::FanOp::Continuous,
                                                                            HXUnitOn);
                        if (SolFla == -1 && !state.dataGlobal->WarmupFlag) {
                            if (cBVAV.HXDXIterationExceeded < 1) {
                                ++cBVAV.HXDXIterationExceeded;
                                ShowWarningError(state,
                                                 format("Iteration limit exceeded calculating HX assisted DX unit part-load ratio, for unit = {}",
                                                        cBVAV.DXCoolCoilName));
                                ShowContinueError(state, format("Calculated part-load ratio = {:.3R}", PartLoadFrac));
                                ShowContinueErrorTimeStamp(
                                    state, "The calculated part-load ratio will be used and the simulation continues. Occurrence info:");
                            } else {
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    cBVAV.Name + ", Iteration limit exceeded for HX assisted DX unit part-load ratio error continues.",
                                    cBVAV.HXDXIterationExceededIndex,
                                    PartLoadFrac,
                                    PartLoadFrac);
                            }
                        } else if (SolFla == -2 && !state.dataGlobal->WarmupFlag) {
                            PartLoadFrac = max(0.0,
                                               min(1.0,
                                                   (state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp - cBVAV.CoilTempSetPoint) /
                                                       (state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp -
                                                        state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).Temp)));
                            if (cBVAV.HXDXIterationFailed < 1) {
                                ++cBVAV.HXDXIterationFailed;
                                ShowSevereError(
                                    state,
                                    format("HX assisted DX unit part-load ratio calculation failed: part-load ratio limits exceeded, for unit = {}",
                                           cBVAV.DXCoolCoilName));
                                ShowContinueErrorTimeStamp(
                                    state,
                                    format("An estimated part-load ratio of {:.3R}will be used and the simulation continues. Occurrence info:",
                                           PartLoadFrac));
                            } else {
                                ShowRecurringWarningErrorAtEnd(state,
                                                               cBVAV.Name +
                                                                   ", Part-load ratio calculation failed for HX assisted DX unit error continues.",
                                                               cBVAV.HXDXIterationFailedIndex,
                                                               PartLoadFrac,
                                                               PartLoadFrac);
                            }
                        }
                    }
                } break;
                case HVAC::CoilType::DXCoolingSingleSpeed: {
                    DXCoils::SimDXCoil(state,
                                       cBVAV.DXCoolCoilName,
                                       HVAC::CompressorOp::On,
                                       FirstHVACIteration,
                                       cBVAV.CoolCoilCompIndex,
                                       HVAC::FanOp::Continuous,
                                       PartLoadFrac,
                                       OnOffAirFlowRatio);
                    if (state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp <= cBVAV.CoilTempSetPoint) {
                        //         If coil inlet temp is already below the setpoint, simulated with coil off
                        PartLoadFrac = 0.0;
                        DXCoils::SimDXCoil(state,
                                           cBVAV.DXCoolCoilName,
                                           HVAC::CompressorOp::On,
                                           FirstHVACIteration,
                                           cBVAV.CoolCoilCompIndex,
                                           HVAC::FanOp::Continuous,
                                           PartLoadFrac,
                                           OnOffAirFlowRatio);
                    } else if (state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).Temp < cBVAV.CoilTempSetPoint) {
                        auto f = [&state, CBVAVNum, OnOffAirFlowRatio](Real64 const PartLoadFrac) {
                            auto &thisCBVAV = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);
                            DXCoils::CalcDoe2DXCoil(state,
                                                    thisCBVAV.CoolCoilCompIndex,
                                                    HVAC::CompressorOp::On,
                                                    false,
                                                    PartLoadFrac,
                                                    HVAC::FanOp::Continuous,
                                                    _,
                                                    OnOffAirFlowRatio);
                            Real64 OutletAirTemp = state.dataDXCoils->DXCoilOutletTemp(thisCBVAV.CoolCoilCompIndex);
                            return thisCBVAV.CoilTempSetPoint - OutletAirTemp;
                        };
                        General::SolveRoot(state, HVAC::SmallTempDiff, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                        DXCoils::SimDXCoil(state,
                                           cBVAV.DXCoolCoilName,
                                           HVAC::CompressorOp::On,
                                           FirstHVACIteration,
                                           cBVAV.CoolCoilCompIndex,
                                           HVAC::FanOp::Continuous,
                                           PartLoadFrac,
                                           OnOffAirFlowRatio);
                        if (SolFla == -1 && !state.dataGlobal->WarmupFlag) {
                            if (cBVAV.DXIterationExceeded < 1) {
                                ++cBVAV.DXIterationExceeded;
                                ShowWarningError(
                                    state,
                                    format("Iteration limit exceeded calculating DX unit part-load ratio, for unit = {}", cBVAV.DXCoolCoilName));
                                ShowContinueError(state, format("Calculated part-load ratio = {:.3R}", PartLoadFrac));
                                ShowContinueErrorTimeStamp(
                                    state, "The calculated part-load ratio will be used and the simulation continues. Occurrence info:");
                            } else {
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    cBVAV.Name + ", Iteration limit exceeded for DX unit part-load ratio calculation error continues.",
                                    cBVAV.DXIterationExceededIndex,
                                    PartLoadFrac,
                                    PartLoadFrac);
                            }
                        } else if (SolFla == -2 && !state.dataGlobal->WarmupFlag) {
                            PartLoadFrac = max(0.0,
                                               min(1.0,
                                                   (state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp - cBVAV.CoilTempSetPoint) /
                                                       (state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp -
                                                        state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).Temp)));
                            if (cBVAV.DXIterationFailed < 1) {
                                ++cBVAV.DXIterationFailed;
                                ShowSevereError(state,
                                                format("DX unit part-load ratio calculation failed: part-load ratio limits exceeded, for unit = {}",
                                                       cBVAV.DXCoolCoilName));
                                ShowContinueErrorTimeStamp(
                                    state,
                                    format("An estimated part-load ratio of {:.3R}will be used and the simulation continues. Occurrence info:",
                                           PartLoadFrac));
                            } else {
                                ShowRecurringWarningErrorAtEnd(state,
                                                               cBVAV.Name + ", Part-load ratio calculation failed for DX unit error continues.",
                                                               cBVAV.DXIterationFailedIndex,
                                                               PartLoadFrac,
                                                               PartLoadFrac);
                            }
                        }
                    }
                    state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(cBVAV.DXCoolCoilIndexNum);
                } break;
                case HVAC::CoilType::CoolingAirToAirVariableSpeed: {
                    Real64 QZnReq(0.0);                 // Zone load (W), input to variable-speed DX coil
                    Real64 QLatReq(0.0);                // Zone latent load, input to variable-speed DX coil
                    Real64 LocalOnOffAirFlowRatio(1.0); // ratio of compressor on flow to average flow over time step
                    Real64 LocalPartLoadFrac(0.0);
                    Real64 SpeedRatio(0.0);
                    int SpeedNum(1);
                    bool errorFlag(false);
                    int maxNumSpeeds = VariableSpeedCoils::GetVSCoilNumOfSpeeds(state, cBVAV.DXCoolCoilName, errorFlag);
                    Real64 DesOutTemp = cBVAV.CoilTempSetPoint;
                    // Get no load result
                    VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                              cBVAV.DXCoolCoilName,
                                                              cBVAV.CoolCoilCompIndex,
                                                              HVAC::FanOp::Continuous,
                                                              HVAC::CompressorOp::Off,
                                                              LocalPartLoadFrac,
                                                              SpeedNum,
                                                              SpeedRatio,
                                                              QZnReq,
                                                              QLatReq);

                    Real64 NoOutput = state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).MassFlowRate *
                                      (Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).Temp,
                                                                  state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).HumRat) -
                                       Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp,
                                                                  state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).HumRat));

                    // Get full load result
                    LocalPartLoadFrac = 1.0;
                    SpeedNum = maxNumSpeeds;
                    SpeedRatio = 1.0;
                    QZnReq = 0.001; // to indicate the coil is running
                    VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                              cBVAV.DXCoolCoilName,
                                                              cBVAV.CoolCoilCompIndex,
                                                              HVAC::FanOp::Continuous,
                                                              HVAC::CompressorOp::On,
                                                              LocalPartLoadFrac,
                                                              SpeedNum,
                                                              SpeedRatio,
                                                              QZnReq,
                                                              QLatReq);

                    Real64 FullOutput = state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).MassFlowRate *
                                        (Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).Temp,
                                                                    state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).HumRat) -
                                         Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp,
                                                                    state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).HumRat));
                    Real64 ReqOutput = state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).MassFlowRate *
                                       (Psychrometrics::PsyHFnTdbW(DesOutTemp, state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).HumRat) -
                                        Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp,
                                                                   state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).HumRat));

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
                                                                  cBVAV.DXCoolCoilName,
                                                                  cBVAV.CoolCoilCompIndex,
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
                        Real64 OutletTempDXCoil = state.dataVariableSpeedCoils->VarSpeedCoil(cBVAV.CoolCoilCompIndex).OutletAirDBTemp;
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
                                                                      cBVAV.DXCoolCoilName,
                                                                      cBVAV.CoolCoilCompIndex,
                                                                      HVAC::FanOp::Continuous,
                                                                      HVAC::CompressorOp::On,
                                                                      LocalPartLoadFrac,
                                                                      SpeedNum,
                                                                      SpeedRatio,
                                                                      QZnReq,
                                                                      QLatReq,
                                                                      LocalOnOffAirFlowRatio);

                            Real64 TempSpeedOut = state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).MassFlowRate *
                                                  (Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).Temp,
                                                                              state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).HumRat) -
                                                   Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp,
                                                                              state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).HumRat));
                            Real64 TempSpeedReqst =
                                state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).MassFlowRate *
                                (Psychrometrics::PsyHFnTdbW(DesOutTemp, state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).HumRat) -
                                 Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp,
                                                            state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).HumRat));

                            if ((TempSpeedOut - TempSpeedReqst) > tempAccuracy) {
                                // Check to see which speed to meet the load
                                LocalPartLoadFrac = 1.0;
                                SpeedRatio = 1.0;
                                for (int I = 2; I <= maxNumSpeeds; ++I) {
                                    SpeedNum = I;
                                    VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                              cBVAV.DXCoolCoilName,
                                                                              cBVAV.CoolCoilCompIndex,
                                                                              HVAC::FanOp::Continuous,
                                                                              HVAC::CompressorOp::On,
                                                                              LocalPartLoadFrac,
                                                                              SpeedNum,
                                                                              SpeedRatio,
                                                                              QZnReq,
                                                                              QLatReq,
                                                                              LocalOnOffAirFlowRatio);

                                    TempSpeedOut = state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).MassFlowRate *
                                                   (Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).Temp,
                                                                               state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).HumRat) -
                                                    Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp,
                                                                               state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).HumRat));
                                    TempSpeedReqst =
                                        state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).MassFlowRate *
                                        (Psychrometrics::PsyHFnTdbW(DesOutTemp, state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).HumRat) -
                                         Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp,
                                                                    state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).HumRat));

                                    if ((TempSpeedOut - TempSpeedReqst) < tempAccuracy) {
                                        SpeedNum = I;
                                        break;
                                    }
                                }
                                // now find the speed ratio for the found speednum
                                auto f = [&state, CBVAVNum, SpeedNum, DesOutTemp](Real64 const SpeedRatio) {
                                    auto const &thisCBVAV = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);
                                    // FUNCTION LOCAL VARIABLE DECLARATIONS:
                                    Real64 OutletAirTemp; // outlet air temperature [C]
                                    Real64 QZnReqCycling = 0.001;
                                    Real64 QLatReqCycling = 0.0;
                                    Real64 OnOffAirFlowRatioCycling = 1.0;
                                    Real64 partLoadRatio = 1.0;
                                    int CoilIndex = thisCBVAV.CoolCoilCompIndex;
                                    HVAC::FanOp fanOp = HVAC::FanOp::Continuous;
                                    VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                              "",
                                                                              CoilIndex,
                                                                              fanOp,
                                                                              HVAC::CompressorOp::On,
                                                                              partLoadRatio,
                                                                              SpeedNum,
                                                                              SpeedRatio,
                                                                              QZnReqCycling,
                                                                              QLatReqCycling,
                                                                              OnOffAirFlowRatioCycling);

                                    OutletAirTemp = state.dataVariableSpeedCoils->VarSpeedCoil(CoilIndex).OutletAirDBTemp;
                                    return DesOutTemp - OutletAirTemp;
                                };
                                General::SolveRoot(state, tempAccuracy, MaxIte, SolFla, SpeedRatio, f, 1.0e-10, 1.0);

                                if (SolFla == -1) {
                                    if (!state.dataGlobal->WarmupFlag) {
                                        if (cBVAV.DXIterationExceeded < 4) {
                                            ++cBVAV.DXIterationExceeded;
                                            ShowWarningError(state,
                                                             format("{} - Iteration limit exceeded calculating VS DX coil speed ratio for coil named "
                                                                    "{}, in Unitary system named{}",
                                                                    HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.CoolCoilType)],
                                                                    cBVAV.DXCoolCoilName,
                                                                    cBVAV.Name));
                                            ShowContinueError(state, format("Calculated speed ratio = {:.4R}", SpeedRatio));
                                            ShowContinueErrorTimeStamp(
                                                state, "The calculated speed ratio will be used and the simulation continues. Occurrence info:");
                                        }
                                        ShowRecurringWarningErrorAtEnd(state,
                                                                       format("{} \"{}\" - Iteration limit exceeded calculating speed ratio error "
                                                                              "continues. Speed Ratio statistics follow.",
                                                                              HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.CoolCoilType)],
                                                                              cBVAV.DXCoolCoilName),
                                                                       cBVAV.DXIterationExceededIndex,
                                                                       LocalPartLoadFrac,
                                                                       LocalPartLoadFrac);
                                    }
                                } else if (SolFla == -2) {
                                    if (!state.dataGlobal->WarmupFlag) {
                                        if (cBVAV.DXIterationFailed < 4) {
                                            ++cBVAV.DXIterationFailed;
                                            ShowWarningError(state,
                                                             format("{} - DX unit speed ratio calculation failed: solver limits exceeded, for coil "
                                                                    "named {}, in Unitary system named{}",
                                                                    HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.CoolCoilType)],
                                                                    cBVAV.DXCoolCoilName,
                                                                    cBVAV.Name));
                                            ShowContinueError(state, format("Estimated speed ratio = {:.3R}", TempSpeedReqst / TempSpeedOut));
                                            ShowContinueErrorTimeStamp(
                                                state, "The estimated part-load ratio will be used and the simulation continues. Occurrence info:");
                                        }
                                        ShowRecurringWarningErrorAtEnd(
                                            state,
                                            format(
                                                "{} \"{}\" - DX unit speed ratio calculation failed error continues. speed ratio statistics follow.",
                                                HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.CoolCoilType)],
                                                cBVAV.DXCoolCoilName),
                                            cBVAV.DXIterationFailedIndex,
                                            SpeedRatio,
                                            SpeedRatio);
                                    }
                                    SpeedRatio = TempSpeedReqst / TempSpeedOut;
                                }
                            } else {
                                // cycling compressor at lowest speed number, find part load fraction
                                auto f = [&state, CBVAVNum, DesOutTemp](Real64 const PartLoadRatio) {
                                    auto &thisCBVAV = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);
                                    int speedNum = 1;
                                    Real64 speedRatio = 0.0;
                                    Real64 QZnReqCycling = 0.001;
                                    Real64 QLatReqCycling = 0.0;
                                    Real64 OnOffAirFlowRatioCycling = 1.0;
                                    VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                              "",
                                                                              thisCBVAV.CoolCoilCompIndex,
                                                                              HVAC::FanOp::Continuous,
                                                                              HVAC::CompressorOp::On,
                                                                              PartLoadRatio,
                                                                              speedNum,
                                                                              speedRatio,
                                                                              QZnReqCycling,
                                                                              QLatReqCycling,
                                                                              OnOffAirFlowRatioCycling);

                                    Real64 OutletAirTemp = state.dataVariableSpeedCoils->VarSpeedCoil(thisCBVAV.CoolCoilCompIndex).OutletAirDBTemp;
                                    return DesOutTemp - OutletAirTemp;
                                };
                                General::SolveRoot(state, tempAccuracy, MaxIte, SolFla, LocalPartLoadFrac, f, 1.0e-10, 1.0);
                                if (SolFla == -1) {
                                    if (!state.dataGlobal->WarmupFlag) {
                                        if (cBVAV.DXCyclingIterationExceeded < 4) {
                                            ++cBVAV.DXCyclingIterationExceeded;
                                            ShowWarningError(state,
                                                             format("{} - Iteration limit exceeded calculating VS DX unit low speed cycling ratio, "
                                                                    "for coil named {}, in Unitary system named{}",
                                                                    HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.CoolCoilType)],
                                                                    cBVAV.DXCoolCoilName,
                                                                    cBVAV.Name));
                                            ShowContinueError(state, format("Estimated cycling ratio  = {:.3R}", (TempSpeedReqst / TempSpeedOut)));
                                            ShowContinueError(state, format("Calculated cycling ratio = {:.3R}", LocalPartLoadFrac));
                                            ShowContinueErrorTimeStamp(
                                                state, "The calculated cycling ratio will be used and the simulation continues. Occurrence info:");
                                        }
                                        ShowRecurringWarningErrorAtEnd(
                                            state,
                                            format(" {} \"{}\" - Iteration limit exceeded calculating low speed cycling ratio "
                                                   "error continues. Sensible PLR statistics follow.",
                                                   HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.CoolCoilType)],
                                                   cBVAV.DXCoolCoilName),
                                            cBVAV.DXCyclingIterationExceededIndex,
                                            LocalPartLoadFrac,
                                            LocalPartLoadFrac);
                                    }
                                } else if (SolFla == -2) {

                                    if (!state.dataGlobal->WarmupFlag) {
                                        if (cBVAV.DXCyclingIterationFailed < 4) {
                                            ++cBVAV.DXCyclingIterationFailed;
                                            ShowWarningError(
                                                state,
                                                format("{} - DX unit low speed cycling ratio calculation failed: limits exceeded, for unit = {}",
                                                       HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.CoolCoilType)],
                                                       cBVAV.Name));
                                            ShowContinueError(state,
                                                              format("Estimated low speed cycling ratio = {:.3R}", TempSpeedReqst / TempSpeedOut));
                                            ShowContinueErrorTimeStamp(state,
                                                                       "The estimated low speed cycling ratio will be used and the simulation "
                                                                       "continues. Occurrence info:");
                                        }
                                        ShowRecurringWarningErrorAtEnd(state,
                                                                       format("{} \"{}\" - DX unit low speed cycling ratio calculation failed error "
                                                                              "continues. cycling ratio statistics follow.",
                                                                              HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.CoolCoilType)],
                                                                              cBVAV.DXCoolCoilName),
                                                                       cBVAV.DXCyclingIterationFailedIndex,
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
                    state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = VariableSpeedCoils::getVarSpeedPartLoadRatio(state, cBVAV.CoolCoilCompIndex);
                    // variable-speed air-to-air cooling coil, end -------------------------
                } break;
                case HVAC::CoilType::DXCoolingTwoStageWHumControl: {
                    // Coil:Cooling:DX:TwoStageWithHumidityControlMode
                    // formerly (v3 and beyond) Coil:DX:MultiMode:CoolingEmpirical

                    // If DXCoolingSystem runs with a cooling load then set PartLoadFrac on Cooling System and the Mass Flow
                    // Multimode coil will switch to enhanced dehumidification if available and needed, but it
                    // still runs to meet the sensible load

                    // Determine required part load for normal mode

                    // Get full load result
                    HVAC::CoilMode DehumidMode = HVAC::CoilMode::Normal; // Dehumidification mode (0=normal, 1=enhanced)
                    cBVAV.DehumidificationMode = DehumidMode;
                    DXCoils::SimDXCoilMultiMode(state,
                                                cBVAV.DXCoolCoilName,
                                                HVAC::CompressorOp::On,
                                                FirstHVACIteration,
                                                PartLoadFrac,
                                                DehumidMode,
                                                cBVAV.CoolCoilCompIndex,
                                                HVAC::FanOp::Continuous);
                    if (state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp <= cBVAV.CoilTempSetPoint) {
                        PartLoadFrac = 0.0;
                        DXCoils::SimDXCoilMultiMode(state,
                                                    cBVAV.DXCoolCoilName,
                                                    HVAC::CompressorOp::On,
                                                    FirstHVACIteration,
                                                    PartLoadFrac,
                                                    DehumidMode,
                                                    cBVAV.CoolCoilCompIndex,
                                                    HVAC::FanOp::Continuous);
                    } else if (state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).Temp > cBVAV.CoilTempSetPoint) {
                        PartLoadFrac = 1.0;
                    } else {
                        auto f = [&state, CBVAVNum, DehumidMode](Real64 const PartLoadRatio) {
                            auto &thisCBVAV = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);
                            DXCoils::SimDXCoilMultiMode(state,
                                                        "",
                                                        HVAC::CompressorOp::On,
                                                        false,
                                                        PartLoadRatio,
                                                        DehumidMode,
                                                        thisCBVAV.CoolCoilCompIndex,
                                                        HVAC::FanOp::Continuous);
                            return thisCBVAV.CoilTempSetPoint - state.dataDXCoils->DXCoilOutletTemp(thisCBVAV.CoolCoilCompIndex);
                        };
                        General::SolveRoot(state, HVAC::SmallTempDiff, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                        if (SolFla == -1) {
                            if (cBVAV.MMDXIterationExceeded < 1) {
                                ++cBVAV.MMDXIterationExceeded;
                                ShowWarningError(state,
                                                 format("Iteration limit exceeded calculating DX unit part-load ratio, for unit={}", cBVAV.Name));
                                ShowContinueErrorTimeStamp(state, format("Part-load ratio returned = {:.2R}", PartLoadFrac));
                                ShowContinueErrorTimeStamp(
                                    state, "The calculated part-load ratio will be used and the simulation continues. Occurrence info:");
                            } else {
                                ShowRecurringWarningErrorAtEnd(state,
                                                               cBVAV.Name +
                                                                   ", Iteration limit exceeded calculating DX unit part-load ratio error continues.",
                                                               cBVAV.MMDXIterationExceededIndex,
                                                               PartLoadFrac,
                                                               PartLoadFrac);
                            }
                        } else if (SolFla == -2) {
                            PartLoadFrac = max(0.0,
                                               min(1.0,
                                                   (state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp - cBVAV.CoilTempSetPoint) /
                                                       (state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp -
                                                        state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).Temp)));
                            if (cBVAV.MMDXIterationFailed < 1) {
                                ++cBVAV.MMDXIterationFailed;
                                ShowSevereError(
                                    state,
                                    format("DX unit part-load ratio calculation failed: part-load ratio limits exceeded, for unit={}", cBVAV.Name));
                                ShowContinueError(state, format("Estimated part-load ratio = {:.3R}", PartLoadFrac));
                                ShowContinueErrorTimeStamp(
                                    state, "The estimated part-load ratio will be used and the simulation continues. Occurrence info:");
                            } else {
                                ShowRecurringWarningErrorAtEnd(state,
                                                               cBVAV.Name + ", Part-load ratio calculation failed for DX unit error continues.",
                                                               cBVAV.MMDXIterationFailedIndex,
                                                               PartLoadFrac,
                                                               PartLoadFrac);
                            }
                        }
                    }

                    // If humidity setpoint is not satisfied and humidity control type is Multimode,
                    // then turn on enhanced dehumidification mode 1

                    if ((state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).HumRat > state.dataLoopNodes->Node(OutletNode).HumRatMax) &&
                        (state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).HumRat > state.dataLoopNodes->Node(OutletNode).HumRatMax) &&
                        (cBVAV.DehumidControlType == DehumidControl::Multimode) && state.dataLoopNodes->Node(OutletNode).HumRatMax > 0.0) {

                        // Determine required part load for enhanced dehumidification mode 1

                        // Get full load result
                        PartLoadFrac = 1.0;
                        DehumidMode = HVAC::CoilMode::Enhanced;
                        cBVAV.DehumidificationMode = DehumidMode;
                        DXCoils::SimDXCoilMultiMode(state,
                                                    cBVAV.DXCoolCoilName,
                                                    HVAC::CompressorOp::On,
                                                    FirstHVACIteration,
                                                    PartLoadFrac,
                                                    DehumidMode,
                                                    cBVAV.CoolCoilCompIndex,
                                                    HVAC::FanOp::Continuous);
                        if (state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp <= cBVAV.CoilTempSetPoint) {
                            PartLoadFrac = 0.0;
                        } else if (state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).Temp > cBVAV.CoilTempSetPoint) {
                            PartLoadFrac = 1.0;
                        } else {
                            auto f = [&state, CBVAVNum, DehumidMode](Real64 const PartLoadRatio) {
                                auto &thisCBVAV = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);
                                DXCoils::SimDXCoilMultiMode(state,
                                                            "",
                                                            HVAC::CompressorOp::On,
                                                            false,
                                                            PartLoadRatio,
                                                            DehumidMode,
                                                            thisCBVAV.CoolCoilCompIndex,
                                                            HVAC::FanOp::Continuous);
                                return thisCBVAV.CoilTempSetPoint - state.dataDXCoils->DXCoilOutletTemp(thisCBVAV.CoolCoilCompIndex);
                            };
                            General::SolveRoot(state, HVAC::SmallTempDiff, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                            if (SolFla == -1) {
                                if (cBVAV.DMDXIterationExceeded < 1) {
                                    ++cBVAV.DMDXIterationExceeded;
                                    ShowWarningError(
                                        state,
                                        format("Iteration limit exceeded calculating DX unit dehumidifying part-load ratio, for unit = {}",
                                               cBVAV.Name));
                                    ShowContinueErrorTimeStamp(state, format("Part-load ratio returned={:.2R}", PartLoadFrac));
                                    ShowContinueErrorTimeStamp(
                                        state, "The calculated part-load ratio will be used and the simulation continues. Occurrence info:");
                                } else {
                                    ShowRecurringWarningErrorAtEnd(
                                        state,
                                        cBVAV.Name + ", Iteration limit exceeded calculating DX unit dehumidifying part-load ratio error continues.",
                                        cBVAV.DMDXIterationExceededIndex,
                                        PartLoadFrac,
                                        PartLoadFrac);
                                }
                            } else if (SolFla == -2) {
                                PartLoadFrac = max(0.0,
                                                   min(1.0,
                                                       (state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp - cBVAV.CoilTempSetPoint) /
                                                           (state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp -
                                                            state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).Temp)));
                                if (cBVAV.DMDXIterationFailed < 1) {
                                    ++cBVAV.DMDXIterationFailed;
                                    ShowSevereError(state,
                                                    format("DX unit dehumidifying part-load ratio calculation failed: part-load ratio limits "
                                                           "exceeded, for unit = {}",
                                                           cBVAV.Name));
                                    ShowContinueError(state, format("Estimated part-load ratio = {:.3R}", PartLoadFrac));
                                    ShowContinueErrorTimeStamp(
                                        state, "The estimated part-load ratio will be used and the simulation continues. Occurrence info:");
                                } else {
                                    ShowRecurringWarningErrorAtEnd(
                                        state,
                                        cBVAV.Name + ", Dehumidifying part-load ratio calculation failed for DX unit error continues.",
                                        cBVAV.DMDXIterationFailedIndex,
                                        PartLoadFrac,
                                        PartLoadFrac);
                                }
                            }
                        }
                    } // End if humidity ratio setpoint not met - multimode humidity control

                    // If humidity setpoint is not satisfied and humidity control type is CoolReheat,
                    // then run to meet latent load

                    if ((state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).HumRat > state.dataLoopNodes->Node(OutletNode).HumRatMax) &&
                        (state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).HumRat > state.dataLoopNodes->Node(OutletNode).HumRatMax) &&
                        (cBVAV.DehumidControlType == DehumidControl::CoolReheat) && state.dataLoopNodes->Node(OutletNode).HumRatMax > 0.0) {

                        // Determine revised desired outlet temperature  - use approach temperature control strategy
                        // based on CONTROLLER:SIMPLE TEMPANDHUMRAT control type.

                        // Calculate the approach temperature (difference between SA dry-bulb temp and SA dew point temp)
                        ApproachTemp = state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).Temp -
                                       Psychrometrics::PsyTdpFnWPb(state, state.dataLoopNodes->Node(OutletNode).HumRat, OutdoorBaroPress);
                        // Calculate the dew point temperature at the SA humidity ratio setpoint
                        DesiredDewPoint = Psychrometrics::PsyTdpFnWPb(state, state.dataLoopNodes->Node(OutletNode).HumRatMax, OutdoorBaroPress);
                        // Adjust the calculated dew point temperature by the approach temp
                        cBVAV.CoilTempSetPoint = min(cBVAV.CoilTempSetPoint, (DesiredDewPoint + ApproachTemp));

                        // Determine required part load for cool reheat at adjusted DesiredOutletTemp

                        // Get full load result
                        PartLoadFrac = 1.0;
                        DehumidMode = HVAC::CoilMode::Normal;
                        cBVAV.DehumidificationMode = DehumidMode;
                        DXCoils::SimDXCoilMultiMode(state,
                                                    cBVAV.DXCoolCoilName,
                                                    HVAC::CompressorOp::On,
                                                    FirstHVACIteration,
                                                    PartLoadFrac,
                                                    DehumidMode,
                                                    cBVAV.CoolCoilCompIndex,
                                                    HVAC::FanOp::Continuous);
                        if (state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp <= cBVAV.CoilTempSetPoint) {
                            PartLoadFrac = 0.0;
                        } else if (state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).Temp > cBVAV.CoilTempSetPoint) {
                            PartLoadFrac = 1.0;
                        } else {
                            auto f = [&state, CBVAVNum, DehumidMode](Real64 const PartLoadRatio) {
                                auto &thisCBVAV = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);
                                DXCoils::SimDXCoilMultiMode(state,
                                                            "",
                                                            HVAC::CompressorOp::On,
                                                            false,
                                                            PartLoadRatio,
                                                            DehumidMode,
                                                            thisCBVAV.CoolCoilCompIndex,
                                                            HVAC::FanOp::Continuous);
                                return thisCBVAV.CoilTempSetPoint - state.dataDXCoils->DXCoilOutletTemp(thisCBVAV.CoolCoilCompIndex);
                            };
                            General::SolveRoot(state, HVAC::SmallTempDiff, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                            if (SolFla == -1) {
                                if (cBVAV.CRDXIterationExceeded < 1) {
                                    ++cBVAV.CRDXIterationExceeded;
                                    ShowWarningError(state,
                                                     format("Iteration limit exceeded calculating DX unit cool reheat part-load ratio, for unit = {}",
                                                            cBVAV.Name));
                                    ShowContinueErrorTimeStamp(state, format("Part-load ratio returned = {:.2R}", PartLoadFrac));
                                    ShowContinueErrorTimeStamp(
                                        state, "The calculated part-load ratio will be used and the simulation continues. Occurrence info:");
                                } else {
                                    ShowRecurringWarningErrorAtEnd(
                                        state,
                                        cBVAV.Name + ", Iteration limit exceeded calculating cool reheat part-load ratio DX unit error continues.",
                                        cBVAV.CRDXIterationExceededIndex,
                                        PartLoadFrac,
                                        PartLoadFrac);
                                }
                            } else if (SolFla == -2) {
                                PartLoadFrac = max(0.0,
                                                   min(1.0,
                                                       (state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp - cBVAV.CoilTempSetPoint) /
                                                           (state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp -
                                                            state.dataLoopNodes->Node(cBVAV.DXCoilOutletNode).Temp)));
                                if (cBVAV.CRDXIterationFailed < 1) {
                                    ++cBVAV.CRDXIterationFailed;
                                    ShowSevereError(
                                        state,
                                        format(
                                            "DX unit cool reheat part-load ratio calculation failed: part-load ratio limits exceeded, for unit = {}",
                                            cBVAV.Name));
                                    ShowContinueError(state, format("Estimated part-load ratio = {:.3R}", PartLoadFrac));
                                    ShowContinueErrorTimeStamp(
                                        state, "The estimated part-load ratio will be used and the simulation continues. Occurrence info:");
                                } else {
                                    ShowRecurringWarningErrorAtEnd(
                                        state,
                                        cBVAV.Name + ", Dehumidifying part-load ratio calculation failed for DX unit error continues.",
                                        cBVAV.DMDXIterationFailedIndex,
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
                    state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(cBVAV.DXCoolCoilIndexNum);
                } break;
                default: {
                    ShowFatalError(
                        state, format("SimCBVAV System: Invalid DX Cooling Coil={}", HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.CoolCoilType)]));
                } break;
                }
            } else { // IF(OutdoorDryBulbTemp .GE. cBVAV%MinOATCompressor)THEN
                //     Simulate DX cooling coil with compressor off
                if (cBVAV.CoolCoilType == HVAC::CoilType::DXCoolingHXAssisted) {
                    HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(state,
                                                                        cBVAV.DXCoolCoilName,
                                                                        FirstHVACIteration,
                                                                        HVAC::CompressorOp::Off,
                                                                        0.0,
                                                                        cBVAV.CoolCoilCompIndex,
                                                                        HVAC::FanOp::Continuous,
                                                                        HXUnitOn);
                    state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(cBVAV.DXCoolCoilIndexNum);
                } else if (cBVAV.CoolCoilType == HVAC::CoilType::DXCoolingSingleSpeed) {
                    DXCoils::SimDXCoil(state,
                                       cBVAV.DXCoolCoilName,
                                       HVAC::CompressorOp::Off,
                                       FirstHVACIteration,
                                       cBVAV.CoolCoilCompIndex,
                                       HVAC::FanOp::Continuous,
                                       0.0,
                                       OnOffAirFlowRatio);
                    state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(cBVAV.DXCoolCoilIndexNum);
                } else if (cBVAV.CoolCoilType == HVAC::CoilType::DXCoolingTwoStageWHumControl) {
                    DXCoils::SimDXCoilMultiMode(state,
                                                cBVAV.DXCoolCoilName,
                                                HVAC::CompressorOp::Off,
                                                FirstHVACIteration,
                                                0.0,
                                                HVAC::CoilMode::Normal,
                                                cBVAV.CoolCoilCompIndex,
                                                HVAC::FanOp::Continuous);
                    state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(cBVAV.DXCoolCoilIndexNum);
                } else if (cBVAV.CoolCoilType == HVAC::CoilType::CoolingAirToAirVariableSpeed) {
                    // Real64 PartLoadFrac(0.0);
                    Real64 LocalPartLoadFrac = 0.0;
                    Real64 QZnReq = 0.0;  // Zone load (W), input to variable-speed DX coil
                    Real64 QLatReq = 0.0; // Zone latent load, input to variable-speed DX coil
                    Real64 SpeedRatio = 0.0;
                    int SpeedNum = 1;
                    // Get no load result
                    VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                              cBVAV.DXCoolCoilName,
                                                              cBVAV.CoolCoilCompIndex,
                                                              HVAC::FanOp::Continuous,
                                                              HVAC::CompressorOp::Off,
                                                              LocalPartLoadFrac,
                                                              SpeedNum,
                                                              SpeedRatio,
                                                              QZnReq,
                                                              QLatReq);
                    state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = VariableSpeedCoils::getVarSpeedPartLoadRatio(state, cBVAV.CoolCoilCompIndex);
                }
            }

            // Simulate cooling coil with compressor off if zone requires heating
        } else { // HeatCoolMode == HeatingMode and no cooling is required, set PLR to 0
            if (cBVAV.CoolCoilType == HVAC::CoilType::DXCoolingHXAssisted) {
                HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(state,
                                                                    cBVAV.DXCoolCoilName,
                                                                    FirstHVACIteration,
                                                                    HVAC::CompressorOp::Off,
                                                                    0.0,
                                                                    cBVAV.CoolCoilCompIndex,
                                                                    HVAC::FanOp::Continuous,
                                                                    HXUnitOn);
            } else if (cBVAV.CoolCoilType == HVAC::CoilType::DXCoolingSingleSpeed) {
                DXCoils::SimDXCoil(state,
                                   cBVAV.DXCoolCoilName,
                                   HVAC::CompressorOp::Off,
                                   FirstHVACIteration,
                                   cBVAV.CoolCoilCompIndex,
                                   HVAC::FanOp::Continuous,
                                   0.0,
                                   OnOffAirFlowRatio);
            } else if (cBVAV.CoolCoilType == HVAC::CoilType::CoolingAirToAirVariableSpeed) {
                Real64 QZnReq = 0.0;  // Zone load (W), input to variable-speed DX coil
                Real64 QLatReq = 0.0; // Zone latent load, input to variable-speed DX coil
                Real64 LocalPartLoadFrac = 0.0;
                Real64 SpeedRatio = 0.0;
                int SpeedNum = 1;
                // run model with no load
                VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                          cBVAV.DXCoolCoilName,
                                                          cBVAV.CoolCoilCompIndex,
                                                          HVAC::FanOp::Continuous,
                                                          HVAC::CompressorOp::Off,
                                                          LocalPartLoadFrac,
                                                          SpeedNum,
                                                          SpeedRatio,
                                                          QZnReq,
                                                          QLatReq);

            } else if (cBVAV.CoolCoilType == HVAC::CoilType::DXCoolingTwoStageWHumControl) {
                DXCoils::SimDXCoilMultiMode(state,
                                            cBVAV.DXCoolCoilName,
                                            HVAC::CompressorOp::Off,
                                            FirstHVACIteration,
                                            0.0,
                                            HVAC::CoilMode::Normal,
                                            cBVAV.CoolCoilCompIndex,
                                            HVAC::FanOp::Continuous);
            }
        }

        // Simulate the heating coil based on coil type
        switch (cBVAV.HeatCoilType) {
        case HVAC::CoilType::DXHeatingEmpirical: {
            //   Simulate DX heating coil if zone load is positive (heating load)
            if (cBVAV.HeatCoolMode == HeatingMode) {
                if (OutdoorDryBulbTemp > cBVAV.MinOATCompressor) {
                    //       simulate the DX heating coil
                    // vs coil issue

                    DXCoils::SimDXCoil(state,
                                       cBVAV.HeatCoilName,
                                       HVAC::CompressorOp::On,
                                       FirstHVACIteration,
                                       cBVAV.HeatCoilIndex,
                                       HVAC::FanOp::Continuous,
                                       PartLoadFrac,
                                       OnOffAirFlowRatio);
                    if (state.dataLoopNodes->Node(cBVAV.HeatingCoilOutletNode).Temp > cBVAV.CoilTempSetPoint &&
                        state.dataLoopNodes->Node(cBVAV.HeatingCoilInletNode).Temp < cBVAV.CoilTempSetPoint) {
                        // iterate to find PLR at CoilTempSetPoint
                        auto f = [&state, CBVAVNum, OnOffAirFlowRatio](Real64 const PartLoadFrac) {
                            auto &thisCBVAV = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);
                            DXCoils::CalcDXHeatingCoil(state, thisCBVAV.HeatCoilIndex, PartLoadFrac, HVAC::FanOp::Continuous, OnOffAirFlowRatio);
                            Real64 OutletAirTemp = state.dataDXCoils->DXCoilOutletTemp(thisCBVAV.HeatCoilIndex);
                            Real64 par2 = min(thisCBVAV.CoilTempSetPoint, thisCBVAV.MaxLATHeating);
                            return par2 - OutletAirTemp;
                        };
                        General::SolveRoot(state, HVAC::SmallTempDiff, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                        DXCoils::SimDXCoil(state,
                                           cBVAV.HeatCoilName,
                                           HVAC::CompressorOp::On,
                                           FirstHVACIteration,
                                           cBVAV.HeatCoilIndex,
                                           HVAC::FanOp::Continuous,
                                           PartLoadFrac,
                                           OnOffAirFlowRatio);
                        if (SolFla == -1 && !state.dataGlobal->WarmupFlag) {
                            ShowWarningError(
                                state, format("Iteration limit exceeded calculating DX unit part-load ratio, for unit = {}", cBVAV.HeatCoilName));
                            ShowContinueError(state, format("Calculated part-load ratio = {:.3R}", PartLoadFrac));
                            ShowContinueErrorTimeStamp(state,
                                                       "The calculated part-load ratio will be used and the simulation continues. Occurrence info:");
                        } else if (SolFla == -2 && !state.dataGlobal->WarmupFlag) {
                            ShowSevereError(state,
                                            format("DX unit part-load ratio calculation failed: part-load ratio limits exceeded, for unit = {}",
                                                   cBVAV.HeatCoilName));
                            ShowContinueErrorTimeStamp(
                                state,
                                format("A part-load ratio of {:.3R}will be used and the simulation continues. Occurrence info:", PartLoadFrac));
                            ShowContinueError(state, "Please send this information to the EnergyPlus support group.");
                        }
                    }
                } else { // OAT .LT. MinOATCompressor
                    //       simulate DX heating coil with compressor off
                    DXCoils::SimDXCoil(state,
                                       cBVAV.HeatCoilName,
                                       HVAC::CompressorOp::Off,
                                       FirstHVACIteration,
                                       cBVAV.HeatCoilIndex,
                                       HVAC::FanOp::Continuous,
                                       0.0,
                                       OnOffAirFlowRatio);
                }
                state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(cBVAV.DXHeatCoilIndexNum);
            } else { // HeatCoolMode = CoolingMode
                //     simulate DX heating coil with compressor off when cooling load is required
                DXCoils::SimDXCoil(state,
                                   cBVAV.HeatCoilName,
                                   HVAC::CompressorOp::Off,
                                   FirstHVACIteration,
                                   cBVAV.HeatCoilIndex,
                                   HVAC::FanOp::Continuous,
                                   0.0,
                                   OnOffAirFlowRatio);
            }
        } break;
        case HVAC::CoilType::HeatingAirToAirVariableSpeed: {
            Real64 QZnReq = 0.0;                 // Zone load (W), input to variable-speed DX coil
            Real64 QLatReq = 0.0;                // Zone latent load, input to variable-speed DX coil
            Real64 LocalOnOffAirFlowRatio = 1.0; // ratio of compressor on flow to average flow over time step
            Real64 LocalPartLoadFrac = 0.0;
            Real64 SpeedRatio = 0.0;
            int SpeedNum = 1;
            bool errorFlag = false;
            int maxNumSpeeds = VariableSpeedCoils::GetVSCoilNumOfSpeeds(state, cBVAV.HeatCoilName, errorFlag);
            Real64 DesOutTemp = cBVAV.CoilTempSetPoint;
            // Get no load result
            VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                      cBVAV.HeatCoilName,
                                                      cBVAV.DXHeatCoilIndexNum,
                                                      HVAC::FanOp::Continuous,
                                                      HVAC::CompressorOp::Off,
                                                      LocalPartLoadFrac,
                                                      SpeedNum,
                                                      SpeedRatio,
                                                      QZnReq,
                                                      QLatReq);

            Real64 NoOutput = state.dataLoopNodes->Node(cBVAV.HeatingCoilInletNode).MassFlowRate *
                              (Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(cBVAV.HeatingCoilOutletNode).Temp,
                                                          state.dataLoopNodes->Node(cBVAV.HeatingCoilInletNode).HumRat) -
                               Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(cBVAV.HeatingCoilInletNode).Temp,
                                                          state.dataLoopNodes->Node(cBVAV.HeatingCoilOutletNode).HumRat));
            Real64 TempNoOutput = state.dataLoopNodes->Node(cBVAV.HeatingCoilOutletNode).Temp;
            // Real64 NoLoadHumRatOut = VariableSpeedCoils::VarSpeedCoil( CBVAV( CBVAVNum ).CoolCoilCompIndex ).OutletAirHumRat;

            // Get full load result
            LocalPartLoadFrac = 1.0;
            SpeedNum = maxNumSpeeds;
            SpeedRatio = 1.0;
            QZnReq = 0.001; // to indicate the coil is running
            VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                      cBVAV.HeatCoilName,
                                                      cBVAV.DXHeatCoilIndexNum,
                                                      HVAC::FanOp::Continuous,
                                                      HVAC::CompressorOp::On,
                                                      LocalPartLoadFrac,
                                                      SpeedNum,
                                                      SpeedRatio,
                                                      QZnReq,
                                                      QLatReq);

            // Real64 FullLoadHumRatOut = VariableSpeedCoils::VarSpeedCoil( CBVAV( CBVAVNum ).CoolCoilCompIndex ).OutletAirHumRat;
            Real64 FullOutput = state.dataLoopNodes->Node(cBVAV.HeatingCoilInletNode).MassFlowRate *
                                (Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(cBVAV.HeatingCoilOutletNode).Temp,
                                                            state.dataLoopNodes->Node(cBVAV.HeatingCoilOutletNode).HumRat) -
                                 Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(cBVAV.HeatingCoilInletNode).Temp,
                                                            state.dataLoopNodes->Node(cBVAV.HeatingCoilOutletNode).HumRat));
            Real64 ReqOutput = state.dataLoopNodes->Node(cBVAV.HeatingCoilInletNode).MassFlowRate *
                               (Psychrometrics::PsyHFnTdbW(DesOutTemp, state.dataLoopNodes->Node(cBVAV.HeatingCoilOutletNode).HumRat) -
                                Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(cBVAV.HeatingCoilInletNode).Temp,
                                                           state.dataLoopNodes->Node(cBVAV.HeatingCoilOutletNode).HumRat));

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
                                                          cBVAV.HeatCoilName,
                                                          cBVAV.DXHeatCoilIndexNum,
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
                Real64 OutletTempDXCoil = state.dataVariableSpeedCoils->VarSpeedCoil(cBVAV.DXHeatCoilIndexNum).OutletAirDBTemp;
                if (OutletTempDXCoil < DesOutTemp) {
                    LocalPartLoadFrac = 1.0;
                    SpeedNum = maxNumSpeeds;
                    SpeedRatio = 1.0;
                    VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                              cBVAV.HeatCoilName,
                                                              cBVAV.DXHeatCoilIndexNum,
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
                                                              cBVAV.HeatCoilName,
                                                              cBVAV.DXHeatCoilIndexNum,
                                                              HVAC::FanOp::Continuous,
                                                              HVAC::CompressorOp::On,
                                                              LocalPartLoadFrac,
                                                              SpeedNum,
                                                              SpeedRatio,
                                                              QZnReq,
                                                              QLatReq,
                                                              LocalOnOffAirFlowRatio);

                    Real64 TempSpeedOut = state.dataLoopNodes->Node(cBVAV.HeatingCoilOutletNode).Temp;
                    Real64 TempSpeedOutSpeed1 = TempSpeedOut;

                    if ((TempSpeedOut - DesOutTemp) < tempAccuracy) {
                        // Check to see which speed to meet the load
                        LocalPartLoadFrac = 1.0;
                        SpeedRatio = 1.0;
                        for (int I = 2; I <= maxNumSpeeds; ++I) {
                            SpeedNum = I;
                            VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                      cBVAV.HeatCoilName,
                                                                      cBVAV.DXHeatCoilIndexNum,
                                                                      HVAC::FanOp::Continuous,
                                                                      HVAC::CompressorOp::On,
                                                                      LocalPartLoadFrac,
                                                                      SpeedNum,
                                                                      SpeedRatio,
                                                                      QZnReq,
                                                                      QLatReq,
                                                                      LocalOnOffAirFlowRatio);

                            TempSpeedOut = state.dataLoopNodes->Node(cBVAV.HeatingCoilOutletNode).Temp;

                            if ((TempSpeedOut - DesOutTemp) > tempAccuracy) {
                                SpeedNum = I;
                                break;
                            }
                        }
                        // now find the speed ratio for the found speednum
                        int const vsCoilIndex = cBVAV.DXHeatCoilIndexNum;
                        auto f = [&state, vsCoilIndex, DesOutTemp, SpeedNum](Real64 const x) {
                            return HVACDXHeatPumpSystem::VSCoilSpeedResidual(state, x, vsCoilIndex, DesOutTemp, SpeedNum, HVAC::FanOp::Continuous);
                        };
                        General::SolveRoot(state, tempAccuracy, MaxIte, SolFla, SpeedRatio, f, 1.0e-10, 1.0);

                        if (SolFla == -1) {
                            if (!state.dataGlobal->WarmupFlag) {
                                if (cBVAV.DXHeatIterationExceeded < 4) {
                                    ++cBVAV.DXHeatIterationExceeded;
                                    ShowWarningError(state,
                                                     format("{} - Iteration limit exceeded calculating VS DX coil speed ratio for coil named {}, in "
                                                            "Unitary system named{}",
                                                            HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.HeatCoilType)],
                                                            cBVAV.HeatCoilName,
                                                            cBVAV.Name));
                                    ShowContinueError(state, format("Calculated speed ratio = {:.4R}", SpeedRatio));
                                    ShowContinueErrorTimeStamp(
                                        state, "The calculated speed ratio will be used and the simulation continues. Occurrence info:");
                                }
                                ShowRecurringWarningErrorAtEnd(state,
                                                               format("{} \"{}\" - Iteration limit exceeded calculating speed ratio error continues. "
                                                                      "Speed Ratio statistics follow.",
                                                                      HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.HeatCoilType)],
                                                                      cBVAV.HeatCoilName),
                                                               cBVAV.DXHeatIterationExceededIndex,
                                                               LocalPartLoadFrac,
                                                               LocalPartLoadFrac);
                            }
                        } else if (SolFla == -2) {

                            if (!state.dataGlobal->WarmupFlag) {
                                if (cBVAV.DXHeatIterationFailed < 4) {
                                    ++cBVAV.DXHeatIterationFailed;
                                    ShowWarningError(state,
                                                     format("{} - DX unit speed ratio calculation failed: solver limits exceeded, for coil named {}, "
                                                            "in Unitary system named{}",
                                                            HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.HeatCoilType)],
                                                            cBVAV.HeatCoilName,
                                                            cBVAV.Name));
                                    ShowContinueErrorTimeStamp(state,
                                                               " Speed ratio will be set to 0.5, and the simulation continues. Occurrence info:");
                                }
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    format("{} \"{}\" - DX unit speed ratio calculation failed error continues. speed ratio statistics follow.",
                                           HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.HeatCoilType)],
                                           cBVAV.HeatCoilName),
                                    cBVAV.DXHeatIterationFailedIndex,
                                    SpeedRatio,
                                    SpeedRatio);
                            }
                            SpeedRatio = 0.5;
                        }
                        VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                  cBVAV.HeatCoilName,
                                                                  cBVAV.DXHeatCoilIndexNum,
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
                        int VSCoilIndex = cBVAV.DXHeatCoilIndexNum;
                        auto f = [&state, VSCoilIndex, DesOutTemp](Real64 const x) {
                            return HVACDXHeatPumpSystem::VSCoilCyclingResidual(state, x, VSCoilIndex, DesOutTemp, HVAC::FanOp::Continuous);
                        };
                        General::SolveRoot(state, tempAccuracy, MaxIte, SolFla, LocalPartLoadFrac, f, 1.0e-10, 1.0);
                        if (SolFla == -1) {
                            if (!state.dataGlobal->WarmupFlag) {
                                if (cBVAV.DXHeatCyclingIterationExceeded < 4) {
                                    ++cBVAV.DXHeatCyclingIterationExceeded;
                                    ShowWarningError(state,
                                                     format("{} - Iteration limit exceeded calculating VS DX unit low speed cycling ratio, for coil "
                                                            "named {}, in Unitary system named{}",
                                                            HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.HeatCoilType)],
                                                            cBVAV.HeatCoilName,
                                                            cBVAV.Name));
                                    ShowContinueError(state, format("Estimated cycling ratio  = {:.3R}", (DesOutTemp / TempSpeedOut)));
                                    ShowContinueError(state, format("Calculated cycling ratio = {:.3R}", LocalPartLoadFrac));
                                    ShowContinueErrorTimeStamp(
                                        state, "The calculated cycling ratio will be used and the simulation continues. Occurrence info:");
                                }
                                ShowRecurringWarningErrorAtEnd(state,
                                                               format("{} \"{}\" - Iteration limit exceeded calculating low speed cycling ratio "
                                                                      "error continues. Sensible PLR statistics follow.",
                                                                      HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.HeatCoilType)],
                                                                      cBVAV.HeatCoilName),
                                                               cBVAV.DXHeatCyclingIterationExceededIndex,
                                                               LocalPartLoadFrac,
                                                               LocalPartLoadFrac);
                            }
                        } else if (SolFla == -2) {

                            if (!state.dataGlobal->WarmupFlag) {
                                if (cBVAV.DXHeatCyclingIterationFailed < 4) {
                                    ++cBVAV.DXHeatCyclingIterationFailed;
                                    ShowWarningError(state,
                                                     format("{} - DX unit low speed cycling ratio calculation failed: limits exceeded, for unit = {}",
                                                            HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.HeatCoilType)],
                                                            cBVAV.Name));
                                    ShowContinueError(state,
                                                      format("Estimated low speed cycling ratio = {:.3R}",
                                                             (DesOutTemp - TempNoOutput) / (TempSpeedOutSpeed1 - TempNoOutput)));
                                    ShowContinueErrorTimeStamp(
                                        state, "The estimated low speed cycling ratio will be used and the simulation continues. Occurrence info:");
                                }
                                ShowRecurringWarningErrorAtEnd(state,
                                                               format("{} \"{}\" - DX unit low speed cycling ratio calculation failed error "
                                                                      "continues. cycling ratio statistics follow.",
                                                                      HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.HeatCoilType)],
                                                                      cBVAV.HeatCoilName),
                                                               cBVAV.DXHeatCyclingIterationFailedIndex,
                                                               LocalPartLoadFrac,
                                                               LocalPartLoadFrac);
                            }
                            LocalPartLoadFrac = (DesOutTemp - TempNoOutput) / (TempSpeedOutSpeed1 - TempNoOutput);
                        }
                        VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                  cBVAV.HeatCoilName,
                                                                  cBVAV.DXHeatCoilIndexNum,
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
            state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = VariableSpeedCoils::getVarSpeedPartLoadRatio(state, cBVAV.DXHeatCoilIndexNum);
        } break;
        case HVAC::CoilType::HeatingGasOrOtherFuel:
        case HVAC::CoilType::HeatingElectric:
        case HVAC::CoilType::HeatingWater:
        case HVAC::CoilType::HeatingSteam: { // not a DX heating coil
            if (cBVAV.HeatCoolMode == HeatingMode) {
                CpAir = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(cBVAV.HeatingCoilInletNode).HumRat);
                QHeater = state.dataLoopNodes->Node(cBVAV.HeatingCoilInletNode).MassFlowRate * CpAir *
                          (cBVAV.CoilTempSetPoint - state.dataLoopNodes->Node(cBVAV.HeatingCoilInletNode).Temp);
            } else {
                QHeater = 0.0;
            }
            // Added None DX heating coils calling point
            state.dataLoopNodes->Node(cBVAV.HeatingCoilOutletNode).TempSetPoint = cBVAV.CoilTempSetPoint;
            CalcNonDXHeatingCoils(state, CBVAVNum, FirstHVACIteration, QHeater, cBVAV.fanOp, QHeaterActual);
        } break;
        default: {
            ShowFatalError(state, format("SimCBVAV System: Invalid Heating Coil={}", HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.HeatCoilType)]));
        } break;
        }

        if (cBVAV.fanPlace == HVAC::FanPlace::DrawThru) {
            state.dataFans->fans(cBVAV.FanIndex)
                ->simulate(state, FirstHVACIteration, state.dataHVACUnitaryBypassVAV->FanSpeedRatio, _, 1.0 / OnOffAirFlowRatio, _);
        }
        int splitterOutNode = cBVAV.SplitterOutletAirNode;
        state.dataLoopNodes->Node(splitterOutNode).MassFlowRateSetPoint = state.dataLoopNodes->Node(OutletNode).MassFlowRateSetPoint;
        state.dataLoopNodes->Node(OutletNode) = state.dataLoopNodes->Node(splitterOutNode);
        state.dataLoopNodes->Node(OutletNode).TempSetPoint = cBVAV.OutletTempSetPoint;
        state.dataLoopNodes->Node(OutletNode).MassFlowRate =
            (1.0 - state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction) * state.dataLoopNodes->Node(cBVAV.MixerInletAirNode).MassFlowRate;
        // report variable
        cBVAV.BypassMassFlowRate =
            state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction * state.dataLoopNodes->Node(cBVAV.MixerInletAirNode).MassFlowRate;
        // initialize bypass duct connected to mixer or plenum with flow rate and conditions
        if (cBVAV.plenumIndex > 0 || cBVAV.mixerIndex > 0) {
            int plenumOrMixerInletNode = cBVAV.PlenumMixerInletAirNode;
            state.dataLoopNodes->Node(plenumOrMixerInletNode) = state.dataLoopNodes->Node(splitterOutNode);
            state.dataLoopNodes->Node(plenumOrMixerInletNode).MassFlowRate =
                state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction * state.dataLoopNodes->Node(cBVAV.MixerInletAirNode).MassFlowRate;
            state.dataLoopNodes->Node(plenumOrMixerInletNode).MassFlowRateMaxAvail = state.dataLoopNodes->Node(plenumOrMixerInletNode).MassFlowRate;
            state.dataAirLoop->AirLoopFlow(cBVAV.AirLoopNumber).BypassMassFlow = state.dataLoopNodes->Node(plenumOrMixerInletNode).MassFlowRate;
        }

        // calculate sensible load met using delta enthalpy at a constant (minimum) humidity ratio)
        MinHumRat = min(state.dataLoopNodes->Node(InletNode).HumRat, state.dataLoopNodes->Node(OutletNode).HumRat);
        LoadMet =
            state.dataLoopNodes->Node(OutletNode).MassFlowRate * (Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(OutletNode).Temp, MinHumRat) -
                                                                  Psychrometrics::PsyHFnTdbW(state.dataLoopNodes->Node(InletNode).Temp, MinHumRat));

        // calculate OA fraction used for zone OA volume flow rate calc
        state.dataAirLoop->AirLoopFlow(cBVAV.AirLoopNumber).OAFrac = 0.0;
        if (state.dataLoopNodes->Node(cBVAV.AirOutNode).MassFlowRate > 0.0) {
            state.dataAirLoop->AirLoopFlow(cBVAV.AirLoopNumber).OAFrac =
                state.dataLoopNodes->Node(cBVAV.MixerOutsideAirNode).MassFlowRate / state.dataLoopNodes->Node(cBVAV.AirOutNode).MassFlowRate;
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

        auto &cBVAV = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);

        int dayOfSim = state.dataGlobal->DayOfSim; // DayOfSim increments during Warmup when it actually simulates the same day
        if (state.dataGlobal->WarmupFlag) {
            // when warmupday increments then reset timer
            int lastDayOfSim(0); // used during warmup to reset changeOverTimer since need to do same thing next warmup day
            if (lastDayOfSim != dayOfSim) {
                cBVAV.changeOverTimer = -1.0; // reset to default (thisTime always > -1)
            }
            lastDayOfSim = dayOfSim;
            dayOfSim = 1; // reset so that thisTime is <= 24 during warmup
        }
        Real64 thisTime = (dayOfSim - 1) * 24 + state.dataGlobal->HourOfDay - 1 + (state.dataGlobal->TimeStep - 1) * state.dataGlobal->TimeStepZone +
                          state.dataHVACGlobal->SysTimeElapsed;

        if (thisTime <= cBVAV.changeOverTimer) {
            cBVAV.modeChanged = true;
            return;
        }

        Real64 QZoneReqCool = 0.0; // Total cooling load in all controlled zones [W]
        Real64 QZoneReqHeat = 0.0; // Total heating load in all controlled zones [W]
        cBVAV.NumZonesCooled = 0;
        cBVAV.NumZonesHeated = 0;
        cBVAV.HeatCoolMode = 0;

        for (int ZoneNum = 1; ZoneNum <= cBVAV.NumControlledZones; ++ZoneNum) {
            int actualZoneNum = cBVAV.ControlledZoneNum(ZoneNum);
            int coolSeqNum = cBVAV.ZoneSequenceCoolingNum(ZoneNum);
            int heatSeqNum = cBVAV.ZoneSequenceHeatingNum(ZoneNum);
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
                    ++cBVAV.NumZonesHeated;
                } else if (ZoneLoad < -HVAC::SmallLoad) {
                    QZoneReqCool += ZoneLoad;
                    ++cBVAV.NumZonesCooled;
                }
            }
        }

        switch (cBVAV.PriorityControl) {
        case PriorityCtrlMode::CoolingPriority: {
            if (QZoneReqCool < 0.0) {
                cBVAV.HeatCoolMode = CoolingMode;
            } else if (QZoneReqHeat > 0.0) {
                cBVAV.HeatCoolMode = HeatingMode;
            }
        } break;
        case PriorityCtrlMode::HeatingPriority: {
            if (QZoneReqHeat > 0.0) {
                cBVAV.HeatCoolMode = HeatingMode;
            } else if (QZoneReqCool < 0.0) {
                cBVAV.HeatCoolMode = CoolingMode;
            }
        } break;
        case PriorityCtrlMode::ZonePriority: {
            if (cBVAV.NumZonesHeated > cBVAV.NumZonesCooled) {
                if (QZoneReqHeat > 0.0) {
                    cBVAV.HeatCoolMode = HeatingMode;
                } else if (QZoneReqCool < 0.0) {
                    cBVAV.HeatCoolMode = CoolingMode;
                }
            } else if (cBVAV.NumZonesCooled > cBVAV.NumZonesHeated) {
                if (QZoneReqCool < 0.0) {
                    cBVAV.HeatCoolMode = CoolingMode;
                } else if (QZoneReqHeat > 0.0) {
                    cBVAV.HeatCoolMode = HeatingMode;
                }
            } else {
                if (std::abs(QZoneReqCool) > std::abs(QZoneReqHeat) && QZoneReqCool != 0.0) {
                    cBVAV.HeatCoolMode = CoolingMode;
                } else if (std::abs(QZoneReqCool) < std::abs(QZoneReqHeat) && QZoneReqHeat != 0.0) {
                    cBVAV.HeatCoolMode = HeatingMode;
                } else if (std::abs(QZoneReqCool) == std::abs(QZoneReqHeat) && QZoneReqCool != 0.0) {
                    cBVAV.HeatCoolMode = CoolingMode;
                }
            }
        } break;
        case PriorityCtrlMode::LoadPriority: {
            if (std::abs(QZoneReqCool) > std::abs(QZoneReqHeat) && QZoneReqCool != 0.0) {
                cBVAV.HeatCoolMode = CoolingMode;
            } else if (std::abs(QZoneReqCool) < std::abs(QZoneReqHeat) && QZoneReqHeat != 0.0) {
                cBVAV.HeatCoolMode = HeatingMode;
            } else if (cBVAV.NumZonesHeated > cBVAV.NumZonesCooled) {
                if (QZoneReqHeat > 0.0) {
                    cBVAV.HeatCoolMode = HeatingMode;
                } else if (QZoneReqCool < 0.0) {
                    cBVAV.HeatCoolMode = CoolingMode;
                }
            } else if (cBVAV.NumZonesHeated < cBVAV.NumZonesCooled) {
                if (QZoneReqCool < 0.0) {
                    cBVAV.HeatCoolMode = CoolingMode;
                } else if (QZoneReqHeat > 0.0) {
                    cBVAV.HeatCoolMode = HeatingMode;
                }
            } else {
                if (QZoneReqCool < 0.0) {
                    cBVAV.HeatCoolMode = CoolingMode;
                } else if (QZoneReqHeat > 0.0) {
                    cBVAV.HeatCoolMode = HeatingMode;
                }
            }
            break;
        default:
            break;
        }
        }

        if (cBVAV.LastMode != cBVAV.HeatCoolMode) {
            cBVAV.changeOverTimer = thisTime + cBVAV.minModeChangeTime;
            cBVAV.LastMode = cBVAV.HeatCoolMode;
            cBVAV.modeChanged = true;
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

        auto &cBVAV = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNumber);

        Real64 DXCoolCoilInletTemp = state.dataLoopNodes->Node(cBVAV.DXCoilInletNode).Temp;
        Real64 OutAirTemp = state.dataLoopNodes->Node(cBVAV.AirOutNode).Temp;
        Real64 OutAirHumRat = state.dataLoopNodes->Node(cBVAV.AirOutNode).HumRat;

        if (cBVAV.HeatCoolMode == CoolingMode) { // Cooling required
            CalcSetPointTempTarget = 99999.0;
        } else if (cBVAV.HeatCoolMode == HeatingMode) { // Heating required
            CalcSetPointTempTarget = -99999.0;
        }
        Real64 TSupplyToHeatSetPtMax = -99999.0; // Maximum of the supply air temperatures required to reach the heating setpoint [C]
        Real64 TSupplyToCoolSetPtMin = 99999.0;  // Minimum of the supply air temperatures required to reach the cooling setpoint [C]

        for (int ZoneNum = 1; ZoneNum <= cBVAV.NumControlledZones; ++ZoneNum) {
            int ZoneNodeNum = cBVAV.ControlledZoneNodeNum(ZoneNum);
            int BoxOutletNodeNum = cBVAV.CBVAVBoxOutletNode(ZoneNum);
            if ((cBVAV.ZoneSequenceCoolingNum(ZoneNum) > 0) && (cBVAV.ZoneSequenceHeatingNum(ZoneNum) > 0)) {
                QToCoolSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(cBVAV.ControlledZoneNum(ZoneNum))
                                   .SequencedOutputRequiredToCoolingSP(cBVAV.ZoneSequenceCoolingNum(ZoneNum));
                QToHeatSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(cBVAV.ControlledZoneNum(ZoneNum))
                                   .SequencedOutputRequiredToHeatingSP(cBVAV.ZoneSequenceHeatingNum(ZoneNum));
                if (QToHeatSetPt > 0.0 && QToCoolSetPt > 0.0) {
                    ZoneLoad = QToHeatSetPt;
                } else if (QToHeatSetPt < 0.0 && QToCoolSetPt < 0.0) {
                    ZoneLoad = QToCoolSetPt;
                } else if (QToHeatSetPt <= 0.0 && QToCoolSetPt >= 0.0) {
                    ZoneLoad = 0.0;
                }
            } else {
                ZoneLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(cBVAV.ControlledZoneNum(ZoneNum)).RemainingOutputRequired;
                QToCoolSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(cBVAV.ControlledZoneNum(ZoneNum)).OutputRequiredToCoolingSP;
                QToHeatSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(cBVAV.ControlledZoneNum(ZoneNum)).OutputRequiredToHeatingSP;
            }

            Real64 CpSupplyAir = Psychrometrics::PsyCpAirFnW(OutAirHumRat);

            // Find the supply air temperature that will force the box to full flow
            if (BoxOutletNodeNum > 0) {
                if (state.dataLoopNodes->Node(BoxOutletNodeNum).MassFlowRateMax == 0.0) {
                    SupplyAirTemp = state.dataLoopNodes->Node(ZoneNodeNum).Temp;
                } else {
                    // The target supply air temperature is based on current zone temp and load and max box flow rate
                    SupplyAirTemp = state.dataLoopNodes->Node(ZoneNodeNum).Temp +
                                    ZoneLoad / (CpSupplyAir * state.dataLoopNodes->Node(BoxOutletNodeNum).MassFlowRateMax);
                }
            } else {
                SupplyAirTemp = state.dataLoopNodes->Node(ZoneNodeNum).Temp;
            }

            //     Save the MIN (cooling) or MAX (heating) temperature for coil control
            //     One box will always operate at maximum damper position minimizing overall system energy use
            if (cBVAV.HeatCoolMode == CoolingMode) {
                CalcSetPointTempTarget = min(SupplyAirTemp, CalcSetPointTempTarget);
            } else if (cBVAV.HeatCoolMode == HeatingMode) {
                CalcSetPointTempTarget = max(SupplyAirTemp, CalcSetPointTempTarget);
            } else {
                //       Should use CpAirAtCoolSetPoint or CpAirAtHeatSetPoint here?
                //       If so, use ZoneThermostatSetPointLo(ZoneNum) and ZoneThermostatSetPointHi(ZoneNum)
                //       along with the zone humidity ratio
                if (state.dataLoopNodes->Node(BoxOutletNodeNum).MassFlowRateMax == 0.0) {
                    SupplyAirTempToHeatSetPt = state.dataLoopNodes->Node(ZoneNodeNum).Temp;
                    SupplyAirTempToCoolSetPt = state.dataLoopNodes->Node(ZoneNodeNum).Temp;
                } else {
                    SupplyAirTempToHeatSetPt = state.dataLoopNodes->Node(ZoneNodeNum).Temp +
                                               QToHeatSetPt / (CpSupplyAir * state.dataLoopNodes->Node(BoxOutletNodeNum).MassFlowRateMax);
                    SupplyAirTempToCoolSetPt = state.dataLoopNodes->Node(ZoneNodeNum).Temp +
                                               QToCoolSetPt / (CpSupplyAir * state.dataLoopNodes->Node(BoxOutletNodeNum).MassFlowRateMax);
                }
                TSupplyToHeatSetPtMax = max(SupplyAirTempToHeatSetPt, TSupplyToHeatSetPtMax);
                TSupplyToCoolSetPtMin = min(SupplyAirTempToCoolSetPt, TSupplyToCoolSetPtMin);
            }
        }

        //   Account for floating condition where cooling/heating is required to avoid overshooting setpoint
        if (cBVAV.HeatCoolMode == 0) {
            if (cBVAV.fanOp == HVAC::FanOp::Continuous) {
                if (OutAirTemp > TSupplyToCoolSetPtMin) {
                    CalcSetPointTempTarget = TSupplyToCoolSetPtMin;
                } else if (OutAirTemp < TSupplyToHeatSetPtMax) {
                    CalcSetPointTempTarget = TSupplyToHeatSetPtMax;
                } else {
                    CalcSetPointTempTarget = OutAirTemp;
                }
            } else { // Reset setpoint to inlet air temp if unit is OFF and in cycling fan mode
                CalcSetPointTempTarget = state.dataLoopNodes->Node(cBVAV.AirInNode).Temp;
            }
            //   Reset cooling/heating mode to OFF if mixed air inlet temperature is below/above setpoint temperature.
            //   HeatCoolMode = 0 for OFF, 1 for cooling, 2 for heating
        } else if (cBVAV.HeatCoolMode == CoolingMode) {
            if (DXCoolCoilInletTemp < CalcSetPointTempTarget) {
                CalcSetPointTempTarget = DXCoolCoilInletTemp;
            }
        } else if (cBVAV.HeatCoolMode == HeatingMode) {
            if (DXCoolCoilInletTemp > CalcSetPointTempTarget) {
                CalcSetPointTempTarget = DXCoolCoilInletTemp;
            }
        }

        //   Limit outlet node temperature to MAX/MIN specified in input
        if (CalcSetPointTempTarget < cBVAV.MinLATCooling) {
            CalcSetPointTempTarget = cBVAV.MinLATCooling;
        }
        if (CalcSetPointTempTarget > cBVAV.MaxLATHeating) {
            CalcSetPointTempTarget = cBVAV.MaxLATHeating;
        }

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

        auto &cBVAV = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);

        int InletNode = cBVAV.AirInNode;                     // Inlet node number for CBVAVNum
        int OutletNode = cBVAV.AirOutNode;                   // Outlet node number for CBVAVNum
        int MixerMixedAirNode = cBVAV.MixerMixedAirNode;     // Mixed air node number in OA mixer
        int MixerOutsideAirNode = cBVAV.MixerOutsideAirNode; // Outside air node number in OA mixer
        int MixerReliefAirNode = cBVAV.MixerReliefAirNode;   // Relief air node number in OA mixer
        int MixerInletAirNode = cBVAV.MixerInletAirNode;     // Inlet air node number in OA mixer

        Real64 SystemMassFlow = 0.0; // System mass flow rate required for all zones [kg/s]
        Real64 CpSupplyAir = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(OutletNode).HumRat); // Specific heat of outlet air [J/kg-K]
        // Determine zone air flow
        for (int ZoneNum = 1; ZoneNum <= cBVAV.NumControlledZones; ++ZoneNum) {
            int ZoneNodeNum = cBVAV.ControlledZoneNodeNum(ZoneNum);
            int BoxOutletNodeNum = cBVAV.CBVAVBoxOutletNode(ZoneNum); // Zone supply air inlet node number
            if ((cBVAV.ZoneSequenceCoolingNum(ZoneNum) > 0) && (cBVAV.ZoneSequenceHeatingNum(ZoneNum) > 0)) {
                Real64 QToCoolSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(cBVAV.ControlledZoneNum(ZoneNum))
                                          .SequencedOutputRequiredToCoolingSP(cBVAV.ZoneSequenceCoolingNum(ZoneNum));
                Real64 QToHeatSetPt = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(cBVAV.ControlledZoneNum(ZoneNum))
                                          .SequencedOutputRequiredToHeatingSP(cBVAV.ZoneSequenceHeatingNum(ZoneNum));
                if (QToHeatSetPt > 0.0 && QToCoolSetPt > 0.0) {
                    ZoneLoad = QToHeatSetPt;
                } else if (QToHeatSetPt < 0.0 && QToCoolSetPt < 0.0) {
                    ZoneLoad = QToCoolSetPt;
                } else if (QToHeatSetPt <= 0.0 && QToCoolSetPt >= 0.0) {
                    ZoneLoad = 0.0;
                }
            } else {
                ZoneLoad = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(cBVAV.ControlledZoneNum(ZoneNum)).RemainingOutputRequired;
            }
            Real64 CpZoneAir = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(ZoneNodeNum).HumRat);
            Real64 DeltaCpTemp = CpSupplyAir * state.dataLoopNodes->Node(OutletNode).Temp - CpZoneAir * state.dataLoopNodes->Node(ZoneNodeNum).Temp;

            // Need to check DeltaCpTemp and ensure that it is not zero
            if (DeltaCpTemp != 0.0) { // .AND. .NOT. CurDeadBandOrSetback(ZoneNum))THEN
                ZoneMassFlow = ZoneLoad / DeltaCpTemp;
            } else {
                //     reset to 0 so we don't add in the last zone's mass flow rate
                ZoneMassFlow = 0.0;
            }
            SystemMassFlow += max(state.dataLoopNodes->Node(BoxOutletNodeNum).MassFlowRateMin,
                                  min(ZoneMassFlow, state.dataLoopNodes->Node(BoxOutletNodeNum).MassFlowRateMax));
        }

        Real64 AverageUnitMassFlow = state.dataHVACUnitaryBypassVAV->CompOnMassFlow;
        Real64 AverageOAMassFlow = state.dataHVACUnitaryBypassVAV->OACompOnMassFlow;
        state.dataHVACUnitaryBypassVAV->FanSpeedRatio = state.dataHVACUnitaryBypassVAV->CompOnFlowRatio;

        state.dataLoopNodes->Node(MixerInletAirNode) = state.dataLoopNodes->Node(InletNode);

        state.dataLoopNodes->Node(MixerMixedAirNode).MassFlowRateMin = 0.0;

        if (cBVAV.availSched->getCurrentVal() == 0.0 || AverageUnitMassFlow == 0.0) {
            state.dataLoopNodes->Node(InletNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(MixerOutsideAirNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(MixerReliefAirNode).MassFlowRate = 0.0;
            OnOffAirFlowRatio = 0.0;
            state.dataHVACUnitaryBypassVAV->BypassDuctFlowFraction = 0.0;
        } else {
            state.dataLoopNodes->Node(MixerInletAirNode).MassFlowRate = AverageUnitMassFlow;
            state.dataLoopNodes->Node(MixerOutsideAirNode).MassFlowRate = AverageOAMassFlow;
            state.dataLoopNodes->Node(MixerReliefAirNode).MassFlowRate = AverageOAMassFlow;
            OnOffAirFlowRatio = 1.0;
            Real64 boxOutletNodeFlow = 0.0;
            for (int i = 1; i <= cBVAV.NumControlledZones; ++i) {
                boxOutletNodeFlow += state.dataLoopNodes->Node(cBVAV.CBVAVBoxOutletNode(i)).MassFlowRate;
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

        auto &thisCBVAV = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);

        Real64 ReportingConstant = state.dataHVACGlobal->TimeStepSysSec;

        thisCBVAV.TotCoolEnergy = thisCBVAV.TotCoolEnergyRate * ReportingConstant;
        thisCBVAV.TotHeatEnergy = thisCBVAV.TotHeatEnergyRate * ReportingConstant;
        thisCBVAV.SensCoolEnergy = thisCBVAV.SensCoolEnergyRate * ReportingConstant;
        thisCBVAV.SensHeatEnergy = thisCBVAV.SensHeatEnergyRate * ReportingConstant;
        thisCBVAV.LatCoolEnergy = thisCBVAV.LatCoolEnergyRate * ReportingConstant;
        thisCBVAV.LatHeatEnergy = thisCBVAV.LatHeatEnergyRate * ReportingConstant;
        thisCBVAV.ElecConsumption = thisCBVAV.ElecPower * ReportingConstant;

        if (thisCBVAV.FirstPass) {
            if (!state.dataGlobal->SysSizingCalc) {
                DataSizing::resetHVACSizingGlobals(state, state.dataSize->CurZoneEqNum, state.dataSize->CurSysNum, thisCBVAV.FirstPass);
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

        auto &thisCBVAV = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);

        if (HeatCoilLoad > HVAC::SmallLoad) {
            switch (thisCBVAV.HeatCoilType) {
            case HVAC::CoilType::HeatingGasOrOtherFuel:
            case HVAC::CoilType::HeatingElectric: {
                HeatingCoils::SimulateHeatingCoilComponents(
                    state, thisCBVAV.HeatCoilName, FirstHVACIteration, HeatCoilLoad, thisCBVAV.HeatCoilIndex, QCoilActual, false, fanOp);
            } break;
            case HVAC::CoilType::HeatingWater: {
                // simulate the heating coil at maximum hot water flow rate
                MaxHotWaterFlow = thisCBVAV.MaxHeatCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(state, MaxHotWaterFlow, thisCBVAV.CoilControlNode, thisCBVAV.CoilOutletNode, thisCBVAV.plantLoc);
                WaterCoils::SimulateWaterCoilComponents(
                    state, thisCBVAV.HeatCoilName, FirstHVACIteration, thisCBVAV.HeatCoilIndex, QCoilActual, fanOp);
                if (QCoilActual > (HeatCoilLoad + HVAC::SmallLoad)) {
                    // control water flow to obtain output matching HeatCoilLoad
                    int SolFlag = 0;
                    MinWaterFlow = 0.0;
                    auto f = [&state, CBVAVNum, FirstHVACIteration, HeatCoilLoad](Real64 const HWFlow) {
                        auto &thiscBVAV = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);
                        Real64 QCoilActual = HeatCoilLoad;
                        Real64 mdot = HWFlow;
                        PlantUtilities::SetComponentFlowRate(state, mdot, thiscBVAV.CoilControlNode, thiscBVAV.CoilOutletNode, thiscBVAV.plantLoc);
                        // simulate the hot water supplemental heating coil
                        WaterCoils::SimulateWaterCoilComponents(
                            state, thiscBVAV.HeatCoilName, FirstHVACIteration, thiscBVAV.HeatCoilIndex, QCoilActual, thiscBVAV.fanOp);
                        if (HeatCoilLoad != 0.0) {
                            return (QCoilActual - HeatCoilLoad) / HeatCoilLoad;
                        } // Autodesk:Return Condition added to assure return value is set
                        return 0.0;
                    };
                    General::SolveRoot(state, ErrTolerance, SolveMaxIter, SolFlag, HotWaterMdot, f, MinWaterFlow, MaxHotWaterFlow);
                    if (SolFlag == -1) {
                        if (thisCBVAV.HotWaterCoilMaxIterIndex == 0) {
                            ShowWarningMessage(
                                state,
                                format("CalcNonDXHeatingCoils: Hot water coil control failed for {}=\"{}\"", thisCBVAV.UnitType, thisCBVAV.Name));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state, format("  Iteration limit [{}] exceeded in calculating hot water mass flow rate", SolveMaxIter));
                        }
                        ShowRecurringWarningErrorAtEnd(
                            state,
                            format("CalcNonDXHeatingCoils: Hot water coil control failed (iteration limit [{}]) for {}=\"{}",
                                   SolveMaxIter,
                                   thisCBVAV.UnitType,
                                   thisCBVAV.Name),
                            thisCBVAV.HotWaterCoilMaxIterIndex);
                    } else if (SolFlag == -2) {
                        if (thisCBVAV.HotWaterCoilMaxIterIndex2 == 0) {
                            ShowWarningMessage(state,
                                               format("CalcNonDXHeatingCoils: Hot water coil control failed (maximum flow limits) for {}=\"{}\"",
                                                      thisCBVAV.UnitType,
                                                      thisCBVAV.Name));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state, "...Bad hot water maximum flow rate limits");
                            ShowContinueError(state, format("...Given minimum water flow rate={:.3R} kg/s", MinWaterFlow));
                            ShowContinueError(state, format("...Given maximum water flow rate={:.3R} kg/s", MaxHotWaterFlow));
                        }
                        ShowRecurringWarningErrorAtEnd(state,
                                                       "CalcNonDXHeatingCoils: Hot water coil control failed (flow limits) for " +
                                                           thisCBVAV.UnitType + "=\"" + thisCBVAV.Name + "\"",
                                                       thisCBVAV.HotWaterCoilMaxIterIndex2,
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
                        state, thisCBVAV.HeatCoilName, FirstHVACIteration, thisCBVAV.HeatCoilIndex, QCoilActual, fanOp);
                }
            } break;
            case HVAC::CoilType::HeatingSteam: {
                mdot = thisCBVAV.MaxHeatCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(state, mdot, thisCBVAV.CoilControlNode, thisCBVAV.CoilOutletNode, thisCBVAV.plantLoc);

                // simulate the steam heating coil
                SteamCoils::SimulateSteamCoilComponents(
                    state, thisCBVAV.HeatCoilName, FirstHVACIteration, thisCBVAV.HeatCoilIndex, HeatCoilLoad, QCoilActual, fanOp);
            } break;
            default:
                break;
            }
        } else {
            switch (thisCBVAV.HeatCoilType) {
            case HVAC::CoilType::HeatingGasOrOtherFuel:
            case HVAC::CoilType::HeatingElectric: {
                HeatingCoils::SimulateHeatingCoilComponents(
                    state, thisCBVAV.HeatCoilName, FirstHVACIteration, HeatCoilLoad, thisCBVAV.HeatCoilIndex, QCoilActual, false, fanOp);
            } break;
            case HVAC::CoilType::HeatingWater: {
                mdot = 0.0;
                PlantUtilities::SetComponentFlowRate(state, mdot, thisCBVAV.CoilControlNode, thisCBVAV.CoilOutletNode, thisCBVAV.plantLoc);
                QCoilActual = HeatCoilLoad;
                // simulate the hot water heating coil
                WaterCoils::SimulateWaterCoilComponents(
                    state, thisCBVAV.HeatCoilName, FirstHVACIteration, thisCBVAV.HeatCoilIndex, QCoilActual, fanOp);
            } break;
            case HVAC::CoilType::HeatingSteam: {
                mdot = 0.0;
                PlantUtilities::SetComponentFlowRate(state, mdot, thisCBVAV.CoilControlNode, thisCBVAV.CoilOutletNode, thisCBVAV.plantLoc);
                // simulate the steam heating coil
                SteamCoils::SimulateSteamCoilComponents(
                    state, thisCBVAV.HeatCoilName, FirstHVACIteration, thisCBVAV.HeatCoilIndex, HeatCoilLoad, QCoilActual, fanOp);
            } break;
            default:
                break;
            }
        }
        HeatCoilLoadmet = QCoilActual;
    }

} // namespace HVACUnitaryBypassVAV

} // namespace EnergyPlus

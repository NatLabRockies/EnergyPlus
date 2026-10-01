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
#include <format>

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
                ShowFatalError(state, std::format("SimUnitaryBypassVAV: Unit not found={}", CompName));
            }
            CompIndex = CBVAVNum;
        } else {
            CBVAVNum = CompIndex;
            if (CBVAVNum > state.dataHVACUnitaryBypassVAV->NumCBVAV || CBVAVNum < 1) {
                ShowFatalError(state,
                               std::format("SimUnitaryBypassVAV:  Invalid CompIndex passed={}, Number of Units={}, Entered Unit name={}",
                                           CBVAVNum,
                                           state.dataHVACUnitaryBypassVAV->NumCBVAV,
                                           CompName));
            }
            if (state.dataHVACUnitaryBypassVAV->CheckEquipName(CBVAVNum)) {
                if (CompName != state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum).Name) {
                    ShowFatalError(state,
                                   std::format("SimUnitaryBypassVAV: Invalid CompIndex passed={}, Unit name={}, stored Unit Name for that index={}",
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

            auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);

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
                ShowSevereError(state, std::format("{} illegal {} = {:.7f}", CurrentModuleObject, cNumericFields(1), Numbers(1)));
                ShowContinueError(state, std::format("{} must be greater than zero.", cNumericFields(1)));
                ShowContinueError(state, std::format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                ErrorsFound = true;
            }

            cbvav.MaxHeatAirVolFlow = Numbers(2);
            if (cbvav.MaxHeatAirVolFlow <= 0.0 && cbvav.MaxHeatAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, std::format("{} illegal {} = {:.7f}", CurrentModuleObject, cNumericFields(2), Numbers(2)));
                ShowContinueError(state, std::format("{} must be greater than zero.", cNumericFields(2)));
                ShowContinueError(state, std::format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                ErrorsFound = true;
            }

            cbvav.MaxNoCoolHeatAirVolFlow = Numbers(3);
            if (cbvav.MaxNoCoolHeatAirVolFlow < 0.0 && cbvav.MaxNoCoolHeatAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, std::format("{} illegal {} = {:.7f}", CurrentModuleObject, cNumericFields(3), Numbers(3)));
                ShowContinueError(state, std::format("{} must be greater than or equal to zero.", cNumericFields(3)));
                ShowContinueError(state, std::format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                ErrorsFound = true;
            }

            cbvav.CoolOutAirVolFlow = Numbers(4);
            if (cbvav.CoolOutAirVolFlow < 0.0 && cbvav.CoolOutAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, std::format("{} illegal {} = {:.7f}", CurrentModuleObject, cNumericFields(4), Numbers(4)));
                ShowContinueError(state, std::format("{} must be greater than or equal to zero.", cNumericFields(4)));
                ShowContinueError(state, std::format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                ErrorsFound = true;
            }

            cbvav.HeatOutAirVolFlow = Numbers(5);
            if (cbvav.HeatOutAirVolFlow < 0.0 && cbvav.HeatOutAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, std::format("{} illegal {} = {:.7f}", CurrentModuleObject, cNumericFields(5), Numbers(5)));
                ShowContinueError(state, std::format("{} must be greater than or equal to zero.", cNumericFields(5)));
                ShowContinueError(state, std::format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                ErrorsFound = true;
            }

            cbvav.NoCoolHeatOutAirVolFlow = Numbers(6);
            if (cbvav.NoCoolHeatOutAirVolFlow < 0.0 && cbvav.NoCoolHeatOutAirVolFlow != DataSizing::AutoSize) {
                ShowSevereError(state, std::format("{} illegal {} = {:.7f}", CurrentModuleObject, cNumericFields(6), Numbers(6)));
                ShowContinueError(state, std::format("{} must be greater than or equal to zero.", cNumericFields(6)));
                ShowContinueError(state, std::format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                ErrorsFound = true;
            }

            cbvav.outAirSched = Sched::GetSchedule(state, Alphas(3));
            if (cbvav.outAirSched != nullptr) {
                if (!cbvav.outAirSched->checkMinMaxVals(state, Clusive::In, 0.0, Clusive::In, 1.0)) {
                    Sched::ShowSevereBadMinMax(state, eoh, cAlphaFields(3), Alphas(3), Clusive::In, 0.0, Clusive::In, 1.0);
                    ErrorsFound = true;
                }
            }

            cbvav.AirInNode = Node::GetOnlySingleNode(state,
                                                      Alphas(4),
                                                      ErrorsFound,
                                                      Node::ConnectionObjectType::AirLoopHVACUnitaryHeatCoolVAVChangeoverBypass,
                                                      Alphas(1),
                                                      Node::FluidType::Air,
                                                      Node::ConnectionType::Inlet,
                                                      Node::CompFluidStream::Primary,
                                                      Node::ObjectIsParent);

            std::string MixerInletNodeName = Alphas(5);
            std::string SplitterOutletNodeName = Alphas(6);

            cbvav.AirOutNode = Node::GetOnlySingleNode(state,
                                                       Alphas(7),
                                                       ErrorsFound,
                                                       Node::ConnectionObjectType::AirLoopHVACUnitaryHeatCoolVAVChangeoverBypass,
                                                       Alphas(1),
                                                       Node::FluidType::Air,
                                                       Node::ConnectionType::Outlet,
                                                       Node::CompFluidStream::Primary,
                                                       Node::ObjectIsParent);

            cbvav.SplitterOutletAirNode = Node::GetOnlySingleNode(state,
                                                                  SplitterOutletNodeName,
                                                                  ErrorsFound,
                                                                  Node::ConnectionObjectType::AirLoopHVACUnitaryHeatCoolVAVChangeoverBypass,
                                                                  Alphas(1),
                                                                  Node::FluidType::Air,
                                                                  Node::ConnectionType::Internal,
                                                                  Node::CompFluidStream::Primary,
                                                                  Node::ObjectIsParent);

            if (NumAlphas > 19 && !lAlphaBlanks(20)) {
                cbvav.PlenumMixerInletAirNode = Node::GetOnlySingleNode(state,
                                                                        Alphas(20),
                                                                        ErrorsFound,
                                                                        Node::ConnectionObjectType::AirLoopHVACUnitaryHeatCoolVAVChangeoverBypass,
                                                                        Alphas(1),
                                                                        Node::FluidType::Air,
                                                                        Node::ConnectionType::Internal,
                                                                        Node::CompFluidStream::Primary,
                                                                        Node::ObjectIsParent);
                cbvav.PlenumMixerInletAirNode = Node::GetOnlySingleNode(state,
                                                                        Alphas(20),
                                                                        ErrorsFound,
                                                                        Node::ConnectionObjectType::AirLoopHVACUnitaryHeatCoolVAVChangeoverBypass,
                                                                        Alphas(1) + "_PlenumMixerInlet",
                                                                        Node::FluidType::Air,
                                                                        Node::ConnectionType::Outlet,
                                                                        Node::CompFluidStream::Primary,
                                                                        Node::ObjectIsParent);
            }

            cbvav.plenumIndex = ZonePlenum::getReturnPlenumIndexFromInletNode(state, cbvav.PlenumMixerInletAirNode);
            cbvav.mixerIndex = MixerComponent::getZoneMixerIndexFromInletNode(state, cbvav.PlenumMixerInletAirNode);
            if (cbvav.plenumIndex > 0 && cbvav.mixerIndex > 0) {
                ShowSevereError(state, std::format("{}: {}", CurrentModuleObject, cbvav.Name));
                ShowContinueError(state, std::format("Illegal connection for {} = \"{}\".", cAlphaFields(20), Alphas(20)));
                ShowContinueError(
                    state, std::format("{} cannot be connected to both an AirloopHVAC:ReturnPlenum and an AirloopHVAC:ZoneMixer.", cAlphaFields(20)));
                ErrorsFound = true;
            } else if (cbvav.plenumIndex == 0 && cbvav.mixerIndex == 0 && cbvav.PlenumMixerInletAirNode > 0) {
                ShowSevereError(state, std::format("{}: {}", CurrentModuleObject, cbvav.Name));
                ShowContinueError(state, std::format("Illegal connection for {} = \"{}\".", cAlphaFields(20), Alphas(20)));
                ShowContinueError(state,
                                  std::format("{} must be connected to an AirloopHVAC:ReturnPlenum or AirloopHVAC:ZoneMixer. No connection found.",
                                              cAlphaFields(20)));
                ErrorsFound = true;
            }

            cbvav.MixerInletAirNode = Node::GetOnlySingleNode(state,
                                                              MixerInletNodeName,
                                                              ErrorsFound,
                                                              Node::ConnectionObjectType::AirLoopHVACUnitaryHeatCoolVAVChangeoverBypass,
                                                              Alphas(1),
                                                              Node::FluidType::Air,
                                                              Node::ConnectionType::Internal,
                                                              Node::CompFluidStream::Primary,
                                                              Node::ObjectIsParent);

            cbvav.MixerInletAirNode = Node::GetOnlySingleNode(state,
                                                              MixerInletNodeName,
                                                              ErrorsFound,
                                                              Node::ConnectionObjectType::AirLoopHVACUnitaryHeatCoolVAVChangeoverBypass,
                                                              Alphas(1) + "_Mixer",
                                                              Node::FluidType::Air,
                                                              Node::ConnectionType::Outlet,
                                                              Node::CompFluidStream::Primary,
                                                              Node::ObjectIsParent);

            cbvav.SplitterOutletAirNode = Node::GetOnlySingleNode(state,
                                                                  SplitterOutletNodeName,
                                                                  ErrorsFound,
                                                                  Node::ConnectionObjectType::AirLoopHVACUnitaryHeatCoolVAVChangeoverBypass,
                                                                  Alphas(1) + "_Splitter",
                                                                  Node::FluidType::Air,
                                                                  Node::ConnectionType::Inlet,
                                                                  Node::CompFluidStream::Primary,
                                                                  Node::ObjectIsParent);

            cbvav.OAMixType = Alphas(8);
            cbvav.OAMixName = Alphas(9);

            bool errFlag = false;
            ValidateComponent(state, cbvav.OAMixType, cbvav.OAMixName, errFlag, CurrentModuleObject);
            if (errFlag) {
                ShowContinueError(state, std::format("specified in {} = \"{}\".", CurrentModuleObject, cbvav.Name));
                ErrorsFound = true;
            } else {
                // Get OA Mixer node numbers
                OANodeNums = MixedAir::GetOAMixerNodeNumbers(state, cbvav.OAMixName, errFlag);
                if (errFlag) {
                    ShowContinueError(state, std::format("that was specified in {} = {}", CurrentModuleObject, cbvav.Name));
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
                ShowSevereError(state, std::format("{}: {}", CurrentModuleObject, cbvav.Name));
                ShowContinueError(state, std::format("Illegal {} = {}.", cAlphaFields(5), MixerInletNodeName));
                ShowContinueError(
                    state,
                    std::format("{} must be the same as the return air stream node specified in the OutdoorAir:Mixer object.", cAlphaFields(5)));
                ErrorsFound = true;
            }

            if (cbvav.MixerInletAirNode == cbvav.AirInNode) {
                ShowSevereError(state, std::format("{}: {}", CurrentModuleObject, cbvav.Name));
                ShowContinueError(state, std::format("Illegal {} = {}.", cAlphaFields(5), MixerInletNodeName));
                ShowContinueError(state, std::format("{} must be different than the {}.", cAlphaFields(5), cAlphaFields(4)));
                ErrorsFound = true;
            }

            if (cbvav.SplitterOutletAirNode == cbvav.AirOutNode) {
                ShowSevereError(state, std::format("{}: {}", CurrentModuleObject, cbvav.Name));
                ShowContinueError(state, std::format("Illegal {} = {}.", cAlphaFields(6), SplitterOutletNodeName));
                ShowContinueError(state, std::format("{} must be different than the {}.", cAlphaFields(6), cAlphaFields(7)));
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
                    ShowSevereError(state, std::format("{}: {}", CurrentModuleObject, cbvav.Name));
                    ShowContinueError(state, std::format("Illegal {} = {}.", cAlphaFields(6), SplitterOutletNodeName));
                    ShowContinueError(
                        state,
                        std::format("{} must be the same as the fan outlet node specified in {} = {}: {} when draw through {} is selected.",
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
                                     std::format("{} - air flow rate = {:.7f} in {} = {} is less than the ",
                                                 CurrentModuleObject,
                                                 cbvav.FanVolFlow,
                                                 cAlphaFields(11),
                                                 cbvav.FanName) +
                                         cNumericFields(1));
                    ShowContinueError(state, std::format(" {} is reset to the fan flow rate and the simulation continues.", cNumericFields(1)));
                    ShowContinueError(state, std::format(" Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                    cbvav.MaxCoolAirVolFlow = cbvav.FanVolFlow;
                }
                if (cbvav.FanVolFlow < cbvav.MaxHeatAirVolFlow && cbvav.MaxHeatAirVolFlow != DataSizing::AutoSize) {
                    ShowWarningError(state,
                                     std::format("{} - air flow rate = {:.7f} in {} = {} is less than the ",
                                                 CurrentModuleObject,
                                                 cbvav.FanVolFlow,
                                                 cAlphaFields(11),
                                                 cbvav.FanName) +
                                         cNumericFields(2));
                    ShowContinueError(state, std::format(" {} is reset to the fan flow rate and the simulation continues.", cNumericFields(2)));
                    ShowContinueError(state, std::format(" Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                    cbvav.MaxHeatAirVolFlow = cbvav.FanVolFlow;
                }
            }

            //   only check that OA flow in cooling is >= SA flow in cooling when they are not autosized
            if (cbvav.CoolOutAirVolFlow > cbvav.MaxCoolAirVolFlow && cbvav.CoolOutAirVolFlow != DataSizing::AutoSize &&
                cbvav.MaxCoolAirVolFlow != DataSizing::AutoSize) {
                ShowWarningError(state, std::format("{}: {} cannot be greater than {}", CurrentModuleObject, cNumericFields(4), cNumericFields(1)));
                ShowContinueError(state, std::format(" {} is reset to the fan flow rate and the simulation continues.", cNumericFields(4)));
                ShowContinueError(state, std::format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                cbvav.CoolOutAirVolFlow = cbvav.FanVolFlow;
            }

            //   only check that SA flow in heating is >= OA flow in heating when they are not autosized
            if (cbvav.HeatOutAirVolFlow > cbvav.MaxHeatAirVolFlow && cbvav.HeatOutAirVolFlow != DataSizing::AutoSize &&
                cbvav.MaxHeatAirVolFlow != DataSizing::AutoSize) {
                ShowWarningError(state, std::format("{}: {} cannot be greater than {}", CurrentModuleObject, cNumericFields(5), cNumericFields(2)));
                ShowContinueError(state, std::format(" {} is reset to the fan flow rate and the simulation continues.", cNumericFields(5)));
                ShowContinueError(state, std::format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                cbvav.HeatOutAirVolFlow = cbvav.FanVolFlow;
            }

            std::string thisCoolCoilType = Alphas(14);
            cbvav.coolCoilType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, thisCoolCoilType));
            cbvav.DXCoolCoilName = Alphas(15);

            if (cbvav.coolCoilType == HVAC::CoilType::CoolingDXSingleSpeed) {
                DXCoilErrFlag = false;
                DXCoils::GetDXCoilIndex(state, cbvav.DXCoolCoilName, cbvav.DXCoolCoilIndexNum, DXCoilErrFlag, thisCoolCoilType);
                if (DXCoilErrFlag) {
                    ShowContinueError(state, std::format("...occurs in {} \"{}\"", cbvav.UnitType, cbvav.Name));
                    ErrorsFound = true;
                } else {
                    cbvav.DXCoilInletNode = state.dataDXCoils->DXCoil(cbvav.DXCoolCoilIndexNum).AirInNode;
                    cbvav.DXCoilOutletNode = state.dataDXCoils->DXCoil(cbvav.DXCoolCoilIndexNum).AirOutNode;
                    cbvav.CondenserNodeNum = state.dataDXCoils->DXCoil(cbvav.DXCoolCoilIndexNum).CondenserInletNodeNum(1);
                }
            } else if (cbvav.coolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
                DXCoilErrFlag = false;
                cbvav.DXCoolCoilIndexNum =
                    VariableSpeedCoils::GetCoilIndexVariableSpeed(state, thisCoolCoilType, cbvav.DXCoolCoilName, DXCoilErrFlag);
                if (DXCoilErrFlag) {
                    ShowContinueError(state, std::format("...occurs in {} \"{}\"", cbvav.UnitType, cbvav.Name));
                    ErrorsFound = true;
                } else {
                    cbvav.DXCoilInletNode = state.dataVariableSpeedCoils->VarSpeedCoil(cbvav.DXCoolCoilIndexNum).AirInletNodeNum;
                    cbvav.DXCoilOutletNode = state.dataVariableSpeedCoils->VarSpeedCoil(cbvav.DXCoolCoilIndexNum).AirOutletNodeNum;
                    cbvav.CondenserNodeNum = state.dataVariableSpeedCoils->VarSpeedCoil(cbvav.DXCoolCoilIndexNum).CondenserInletNodeNum;
                }
            } else if (cbvav.coolCoilType == HVAC::CoilType::CoolingDXHXAssisted) {
                DXCoilErrFlag = false;
                HVAC::CoilType ActualCoolCoilType =
                    HVACHXAssistedCoolingCoil::GetCoilObjectTypeNum(state, thisCoolCoilType, cbvav.DXCoolCoilName, DXErrorsFound);
                if (DXErrorsFound) {
                    ShowSevereError(state, std::format("{}: {}", CurrentModuleObject, cbvav.Name));
                    ShowContinueError(state, std::format("CoilSystem:Cooling:DX:HeatExchangerAssisted \"{}\" not found.", cbvav.DXCoolCoilName));
                    ErrorsFound = true;
                } else {
                    if (ActualCoolCoilType == HVAC::CoilType::CoolingDXSingleSpeed) {
                        DXCoils::GetDXCoilIndex(
                            state,
                            HVACHXAssistedCoolingCoil::GetHXDXCoilName(state, thisCoolCoilType, cbvav.DXCoolCoilName, DXCoilErrFlag),
                            cbvav.DXCoolCoilIndexNum,
                            DXCoilErrFlag,
                            "Coil:Cooling:DX:SingleSpeed");
                        if (DXCoilErrFlag) {
                            ShowContinueError(state, std::format("...occurs in {} \"{}\"", cbvav.UnitType, cbvav.Name));
                            ErrorsFound = true;
                        } else {
                            // the "coil" nodes are actually the parent nodes of the wrapped HX
                            cbvav.DXCoilInletNode =
                                HVACHXAssistedCoolingCoil::GetCoilInletNode(state, thisCoolCoilType, cbvav.DXCoolCoilName, DXErrorsFound);
                            cbvav.DXCoilOutletNode =
                                HVACHXAssistedCoolingCoil::GetCoilOutletNode(state, thisCoolCoilType, cbvav.DXCoolCoilName, DXErrorsFound);
                            // the DX coil holds the condenser inlet node number
                            cbvav.CondenserNodeNum = state.dataDXCoils->DXCoil(cbvav.DXCoolCoilIndexNum).CondenserInletNodeNum(1);
                        }
                    } else if (ActualCoolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
                        cbvav.DXCoolCoilIndexNum = VariableSpeedCoils::GetCoilIndexVariableSpeed(
                            state,
                            "Coil:Cooling:DX:VariableSpeed",
                            HVACHXAssistedCoolingCoil::GetHXDXCoilName(state, thisCoolCoilType, cbvav.DXCoolCoilName, DXCoilErrFlag),
                            DXCoilErrFlag);
                        if (DXCoilErrFlag) {
                            ShowContinueError(state, std::format("...occurs in {} \"{}\"", cbvav.UnitType, cbvav.Name));
                            ErrorsFound = true;
                        } else {
                            cbvav.DXCoilInletNode = state.dataVariableSpeedCoils->VarSpeedCoil(cbvav.DXCoolCoilIndexNum).AirInletNodeNum;
                            cbvav.DXCoilOutletNode = state.dataVariableSpeedCoils->VarSpeedCoil(cbvav.DXCoolCoilIndexNum).AirOutletNodeNum;
                            cbvav.CondenserNodeNum = state.dataVariableSpeedCoils->VarSpeedCoil(cbvav.DXCoolCoilIndexNum).CondenserInletNodeNum;
                        }
                    } else if (ActualCoolCoilType == HVAC::CoilType::CoolingDX) {
                        cbvav.DXCoolCoilIndexNum = CoilCoolingDX::factory(state, cbvav.DXCoolCoilName);
                        if (cbvav.DXCoolCoilIndexNum == -1) {
                            ShowContinueError(state, std::format("...occurs in {} \"{}\"", cbvav.UnitType, cbvav.Name));
                            ErrorsFound = true;
                        } else {
                            auto const &newCoil = state.dataCoilCoolingDX->coilCoolingDXs[cbvav.DXCoolCoilIndexNum];
                            cbvav.DXCoilInletNode = newCoil.evapInletNodeIndex;
                            cbvav.DXCoilOutletNode = newCoil.evapOutletNodeIndex;
                            cbvav.CondenserNodeNum = newCoil.condInletNodeIndex;
                        }
                    }
                }
            } else if (cbvav.coolCoilType == HVAC::CoilType::CoolingDXTwoStageWHumControl) {
                DXCoilErrFlag = false;
                DXCoils::GetDXCoilIndex(state, cbvav.DXCoolCoilName, cbvav.DXCoolCoilIndexNum, DXCoilErrFlag, thisCoolCoilType);
                if (DXCoilErrFlag) {
                    ShowContinueError(state, std::format("...occurs in {} \"{}\"", cbvav.UnitType, cbvav.Name));
                    ErrorsFound = true;
                } else {
                    cbvav.DXCoilInletNode = state.dataDXCoils->DXCoil(cbvav.DXCoolCoilIndexNum).AirInNode;
                    cbvav.DXCoilOutletNode = state.dataDXCoils->DXCoil(cbvav.DXCoolCoilIndexNum).AirOutNode;
                    cbvav.CondenserNodeNum = state.dataDXCoils->DXCoil(cbvav.DXCoolCoilIndexNum).CondenserInletNodeNum(1);
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
                    ShowWarningError(state, std::format("{}: {}", CurrentModuleObject, cbvav.Name));
                    ShowContinueError(
                        state,
                        std::format("{} = {} not found. Supply air fan operating mode set to constant operation and simulation continues.",
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
                                     std::format("{} - air flow rate = {:.7f} in {} = {} is less than ",
                                                 CurrentModuleObject,
                                                 cbvav.FanVolFlow,
                                                 cAlphaFields(11),
                                                 cbvav.FanName) +
                                         cNumericFields(3));
                    ShowContinueError(state, std::format(" {} is reset to the fan flow rate and the simulation continues.", cNumericFields(3)));
                    ShowContinueError(state, std::format(" Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                    cbvav.MaxNoCoolHeatAirVolFlow = cbvav.FanVolFlow;
                }
            }
            //   only check that OA flow when compressor is OFF is >= SA flow when compressor is OFF when both are not autosized and
            //   that MaxNoCoolHeatAirVolFlow is /= 0 (trigger to use compressor ON flow, see AirFlowControl variable initialization above)
            if (cbvav.NoCoolHeatOutAirVolFlow > cbvav.MaxNoCoolHeatAirVolFlow && cbvav.NoCoolHeatOutAirVolFlow != DataSizing::AutoSize &&
                cbvav.MaxNoCoolHeatAirVolFlow != DataSizing::AutoSize && cbvav.MaxNoCoolHeatAirVolFlow != 0.0) {
                ShowWarningError(state, std::format("{}: {} cannot be greater than {}", CurrentModuleObject, cNumericFields(6), cNumericFields(3)));
                ShowContinueError(state, std::format(" {} is reset to the fan flow rate and the simulation continues.", cNumericFields(6)));
                ShowContinueError(state, std::format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                cbvav.NoCoolHeatOutAirVolFlow = cbvav.FanVolFlow;
            }

            std::string thisHeatCoilType = Alphas(16);

            cbvav.heatCoilType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, thisHeatCoilType));
            cbvav.HeatCoilName = Alphas(17);

            DXCoilErrFlag = false;
            if (cbvav.heatCoilType == HVAC::CoilType::HeatingDXSingleSpeed) {
                DXCoils::GetDXCoilIndex(
                    state, cbvav.HeatCoilName, cbvav.DXHeatCoilIndexNum, DXCoilErrFlag, HVAC::coilTypeNamesUC[static_cast<int>(cbvav.heatCoilType)]);
                if (DXCoilErrFlag) {
                    ShowContinueError(state, std::format("...occurs in {} \"{}\"", cbvav.UnitType, cbvav.Name));
                    ErrorsFound = true;
                } else {
                    cbvav.MinOATCompressor = state.dataDXCoils->DXCoil(cbvav.DXHeatCoilIndexNum).MinOATCompressor;
                    cbvav.HeatingCoilInletNode = state.dataDXCoils->DXCoil(cbvav.DXHeatCoilIndexNum).AirInNode;
                    cbvav.HeatingCoilOutletNode = state.dataDXCoils->DXCoil(cbvav.DXHeatCoilIndexNum).AirOutNode;
                }
            } else if (cbvav.heatCoilType == HVAC::CoilType::HeatingDXVariableSpeed) {
                cbvav.DXHeatCoilIndexNum = VariableSpeedCoils::GetCoilIndexVariableSpeed(
                    state, HVAC::coilTypeNames[(int)cbvav.heatCoilType], cbvav.HeatCoilName, DXCoilErrFlag);
                if (DXCoilErrFlag) {
                    ShowContinueError(state, std::format("...occurs in {} \"{}\"", cbvav.UnitType, cbvav.Name));
                    ErrorsFound = true;
                } else {
                    cbvav.MinOATCompressor = state.dataVariableSpeedCoils->VarSpeedCoil(cbvav.DXHeatCoilIndexNum).MinOATCompressor;
                    cbvav.HeatingCoilInletNode = state.dataVariableSpeedCoils->VarSpeedCoil(cbvav.DXHeatCoilIndexNum).AirInletNodeNum;
                    cbvav.HeatingCoilOutletNode = state.dataVariableSpeedCoils->VarSpeedCoil(cbvav.DXHeatCoilIndexNum).AirOutletNodeNum;
                }
            } else if (cbvav.heatCoilType == HVAC::CoilType::HeatingGasOrOtherFuel || cbvav.heatCoilType == HVAC::CoilType::HeatingElectric) {
                HeatingCoils::GetCoilIndex(state, cbvav.HeatCoilName, cbvav.DXHeatCoilIndexNum, DXCoilErrFlag);
                if (DXCoilErrFlag) {
                    ShowContinueError(state, std::format("...occurs in {} \"{}\"", cbvav.UnitType, cbvav.Name));
                    ErrorsFound = true;
                } else {
                    cbvav.MinOATCompressor = -999.9;
                    cbvav.HeatingCoilInletNode = state.dataHeatingCoils->HeatingCoil(cbvav.DXHeatCoilIndexNum).AirInletNodeNum;
                    cbvav.HeatingCoilOutletNode = state.dataHeatingCoils->HeatingCoil(cbvav.DXHeatCoilIndexNum).AirOutletNodeNum;
                }
            } else if (cbvav.heatCoilType == HVAC::CoilType::HeatingWater) {
                cbvav.DXHeatCoilIndexNum = WaterCoils::GetWaterCoilIndex(state, "COIL:HEATING:WATER", cbvav.HeatCoilName, DXCoilErrFlag);
                if (DXCoilErrFlag) {
                    ShowContinueError(state, std::format("...occurs in {} \"{}\"", cbvav.UnitType, cbvav.Name));
                    ErrorsFound = true;
                } else {
                    cbvav.CoilControlNode = state.dataWaterCoils->WaterCoil(cbvav.DXHeatCoilIndexNum).WaterInletNodeNum;
                    cbvav.MaxHeatCoilFluidFlow = state.dataWaterCoils->WaterCoil(cbvav.DXHeatCoilIndexNum).MaxWaterVolFlowRate;
                    cbvav.HeatingCoilInletNode = state.dataWaterCoils->WaterCoil(cbvav.DXHeatCoilIndexNum).AirInletNodeNum;
                    cbvav.HeatingCoilOutletNode = state.dataWaterCoils->WaterCoil(cbvav.DXHeatCoilIndexNum).AirOutletNodeNum;
                }
            } else if (cbvav.heatCoilType == HVAC::CoilType::HeatingSteam) {
                cbvav.HeatCoilIndex = SteamCoils::GetSteamCoilIndex(state, "COIL:HEATING:STEAM", cbvav.HeatCoilName, DXCoilErrFlag);
                if (DXCoilErrFlag) {
                    ShowContinueError(state, std::format("...occurs in {} \"{}\"", cbvav.UnitType, cbvav.Name));
                    ErrorsFound = true;
                } else {
                    cbvav.HeatingCoilInletNode = state.dataSteamCoils->SteamCoil(cbvav.HeatCoilIndex).AirInletNodeNum;
                    cbvav.HeatingCoilOutletNode = state.dataSteamCoils->SteamCoil(cbvav.HeatCoilIndex).AirOutletNodeNum;
                    cbvav.CoilControlNode = state.dataSteamCoils->SteamCoil(cbvav.HeatCoilIndex).SteamInletNodeNum;
                    cbvav.MaxHeatCoilFluidFlow = state.dataSteamCoils->SteamCoil(cbvav.HeatCoilIndex).MaxSteamVolFlowRate;
                    Real64 SteamDensity = Fluid::GetSteam(state)->getSatDensity(
                        state, state.dataHVACUnitaryBypassVAV->TempSteamIn, 1.0, getUnitaryHeatCoolVAVChangeoverBypass);
                    if (cbvav.MaxHeatCoilFluidFlow > 0.0) {
                        cbvav.MaxHeatCoilFluidFlow = cbvav.MaxHeatCoilFluidFlow * SteamDensity;
                    }
                }
            }

            if (cbvav.DXCoilOutletNode != cbvav.HeatingCoilInletNode) {
                ShowSevereError(state, std::format("{} illegal coil placement. Cooling coil must be upstream of heating coil.", CurrentModuleObject));
                ShowContinueError(state, std::format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                ErrorsFound = true;
            }

            if (cbvav.fanPlace == HVAC::FanPlace::BlowThru) {
                if (cbvav.SplitterOutletAirNode != cbvav.HeatingCoilOutletNode) {
                    ShowSevereError(state, std::format("{}: {}", CurrentModuleObject, cbvav.Name));
                    ShowContinueError(state, std::format("Illegal {} = {}.", cAlphaFields(6), SplitterOutletNodeName));
                    ShowContinueError(
                        state,
                        std::format(
                            "{} must be the same as the outlet node specified in the heating coil object = {}: {} when blow through {} is selected.",
                            cAlphaFields(6),
                            HVAC::coilTypeNamesUC[static_cast<int>(cbvav.heatCoilType)],
                            cbvav.HeatCoilName,
                            cAlphaFields(12)));
                    ErrorsFound = true;
                }
                if (cbvav.MixerMixedAirNode != cbvav.FanInletNodeNum) {
                    ShowSevereError(state, std::format("{}: {}", CurrentModuleObject, cbvav.Name));
                    ShowContinueError(
                        state,
                        std::format("Illegal {}. The fan inlet node name must be the same as the mixed air node specified in the {} = {} "
                                    "when blow through {} is selected.",
                                    cAlphaFields(11),
                                    cAlphaFields(9),
                                    cbvav.OAMixName,
                                    cAlphaFields(12)));
                    ErrorsFound = true;
                }
            }

            if (cbvav.fanPlace == HVAC::FanPlace::DrawThru) {
                if (cbvav.MixerMixedAirNode != cbvav.DXCoilInletNode) {
                    ShowSevereError(state, std::format("{}: {}", CurrentModuleObject, cbvav.Name));
                    ShowContinueError(
                        state,
                        std::format("Illegal cooling coil placement. The cooling coil inlet node name must be the same as the mixed air "
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
                ShowSevereError(state, std::format("{} illegal {} = {}", CurrentModuleObject, cAlphaFields(18), Alphas(18)));
                ShowContinueError(state, std::format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
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
                ShowWarningError(state, std::format("{}: illegal leaving air temperature specified.", CurrentModuleObject));
                ShowContinueError(state, std::format("Resetting {} equal to {} and the simulation continues.", cNumericFields(7), cNumericFields(8)));
                ShowContinueError(state, std::format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
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
                    ShowWarningError(state, std::format("Invalid {} = {}", cAlphaFields(19), Alphas(19)));
                    ShowContinueError(state, std::format("In {} \"{}\".", CurrentModuleObject, cbvav.Name));
                    ShowContinueError(state, std::format("Valid only with {} = Coil:Cooling:DX:TwoStageWithHumidityControlMode.", cAlphaFields(14)));
                    ShowContinueError(state, std::format("Setting {} to \"None\" and the simulation continues.", cAlphaFields(19)));
                    cbvav.DehumidControlType = DehumidControl::None;
                }
            } else if (Util::SameString(Alphas(19), "CoolReheat")) {
                if (cbvav.coolCoilType == HVAC::CoilType::CoolingDXTwoStageWHumControl) {
                    cbvav.DehumidControlType = DehumidControl::CoolReheat;
                } else {
                    ShowWarningError(state, std::format("Invalid {} = {}", cAlphaFields(19), Alphas(19)));
                    ShowContinueError(state, std::format("In {} \"{}\".", CurrentModuleObject, cbvav.Name));
                    ShowContinueError(state, std::format("Valid only with {} = Coil:Cooling:DX:TwoStageWithHumidityControlMode.", cAlphaFields(14)));
                    ShowContinueError(state, std::format("Setting {} to \"None\" and the simulation continues.", cAlphaFields(19)));
                    cbvav.DehumidControlType = DehumidControl::None;
                }
            } else {
                ShowSevereError(state, std::format("Invalid {} ={}", cAlphaFields(19), Alphas(19)));
                ShowContinueError(state, std::format("In {} \"{}\".", CurrentModuleObject, cbvav.Name));
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
                        std::format("{} has {} = {} which is inconsistent with the fan object.", CurrentModuleObject, cAlphaFields(10), Alphas(10)));
                    ShowContinueError(state, std::format("Occurs in {} = {}", CurrentModuleObject, cbvav.Name));
                    ShowContinueError(state,
                                      std::format(" The fan object ({}) is actually a valid fan type and the simulation continues.", cbvav.FanName));
                    ShowContinueError(state, " Node connections errors may result due to the inconsistent fan type.");
                }
            }

            // Add fan to component sets array
            if (cbvav.fanPlace == HVAC::FanPlace::BlowThru) {
                CompSetFanInlet = state.dataLoopNodes->NodeID(cbvav.MixerMixedAirNode);
                CompSetFanOutlet = state.dataLoopNodes->NodeID(cbvav.DXCoilInletNode);
            } else {
                CompSetFanInlet = state.dataLoopNodes->NodeID(cbvav.HeatingCoilOutletNode);
                CompSetFanOutlet = SplitterOutletNodeName;
            }
            std::string CompSetCoolInlet = state.dataLoopNodes->NodeID(cbvav.DXCoilInletNode);
            std::string CompSetCoolOutlet = state.dataLoopNodes->NodeID(cbvav.DXCoilOutletNode);

            // Add fan to component sets array
            Node::SetUpCompSets(state, cbvav.UnitType, cbvav.Name, Alphas(10), cbvav.FanName, CompSetFanInlet, CompSetFanOutlet);

            // Add cooling coil to component sets array
            Node::SetUpCompSets(state,
                                cbvav.UnitType,
                                cbvav.Name,
                                HVAC::coilTypeNamesUC[static_cast<int>(cbvav.coolCoilType)],
                                cbvav.DXCoolCoilName,
                                CompSetCoolInlet,
                                CompSetCoolOutlet);

            // Add heating coil to component sets array
            Node::SetUpCompSets(state,
                                cbvav.UnitType,
                                cbvav.Name,
                                HVAC::coilTypeNamesUC[static_cast<int>(cbvav.heatCoilType)],
                                cbvav.HeatCoilName,
                                state.dataLoopNodes->NodeID(cbvav.HeatingCoilInletNode),
                                state.dataLoopNodes->NodeID(cbvav.HeatingCoilOutletNode));

            // Set up component set for OA mixer - use OA node and Mixed air node
            Node::SetUpCompSets(state,
                                cbvav.UnitType,
                                cbvav.Name,
                                cbvav.OAMixType,
                                cbvav.OAMixName,
                                state.dataLoopNodes->NodeID(cbvav.MixerOutsideAirNode),
                                state.dataLoopNodes->NodeID(cbvav.MixerMixedAirNode));

            Node::TestCompSet(state,
                              cbvav.UnitType,
                              cbvav.Name,
                              state.dataLoopNodes->NodeID(cbvav.AirInNode),
                              state.dataLoopNodes->NodeID(cbvav.AirOutNode),
                              "Air Nodes");

            //   Find air loop associated with CBVAV system
            for (int AirLoopNum = 1; AirLoopNum <= state.dataHVACGlobal->NumPrimaryAirSys; ++AirLoopNum) {
                for (int BranchNum = 1; BranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).NumBranches; ++BranchNum) {
                    for (int CompNum = 1; CompNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).TotalComponents;
                         ++CompNum) {
                        if (!Util::SameString(state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).Name,
                                              cbvav.Name) ||
                            !Util::SameString(state.dataAirSystemsData->PrimaryAirSystems(AirLoopNum).Branch(BranchNum).Comp(CompNum).TypeOf,
                                              cbvav.UnitType)) {
                            continue;
                        }
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
                            if (state.dataZoneCtrls->TempControlledZone(TstatZoneNum).ActualZoneNum != cbvav.ControlledZoneNum(AirLoopZoneNum)) {
                                continue;
                            }
                            FoundTstatZone = true;
                        }
                        if (!FoundTstatZone) {
                            ShowWarningError(state, std::format("{} \"{}\"", CurrentModuleObject, cbvav.Name));
                            ShowContinueError(state,
                                              std::format("Thermostat not found in zone = {} and the simulation continues.",
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
                                std::format(
                                    "AirLoopHVAC:UnitaryHeatCool:VAVChangeoverBypass, \"{}\": Airloop air terminal in the zone equipment list for "
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
            ShowFatalError(state, std::format("GetCBVAV: Errors found in getting {} input.", CurrentModuleObject));
        }

        for (int CBVAVNum = 1; CBVAVNum <= NumCBVAV; ++CBVAVNum) {
            // Setup Report variables
            auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);
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
            if ((cBVAV.heatCoilType == HVAC::CoilType::HeatingWater) || (cBVAV.heatCoilType == HVAC::CoilType::HeatingSteam)) {
                bool ErrorsFound = false; // Set to true if errors in input, fatal at end of routine
                if (cBVAV.heatCoilType == HVAC::CoilType::HeatingWater) {

                    ErrorFlag = false;
                    PlantUtilities::ScanPlantLoopsForObject(
                        state, cBVAV.HeatCoilName, DataPlant::PlantEquipmentType::CoilWaterSimpleHeating, cBVAV.plantLoc, ErrorFlag, _, _, _, _, _);
                    if (ErrorFlag) {
                        ShowFatalError(state, "InitCBVAV: Program terminated for previous conditions.");
                    }

                    cBVAV.MaxHeatCoilFluidFlow = WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", cBVAV.HeatCoilName, ErrorsFound);

                    if (cBVAV.MaxHeatCoilFluidFlow > 0.0) {
                        Real64 FluidDensity = cBVAV.plantLoc.loop->glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                        cBVAV.MaxHeatCoilFluidFlow =
                            WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", cBVAV.HeatCoilName, ErrorsFound) * FluidDensity;
                    }

                } else if (cBVAV.heatCoilType == HVAC::CoilType::HeatingSteam) {

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
                    ShowContinueError(state, std::format("Occurs in {} = {}", "AirLoopHVAC:UnitaryHeatCool:VAVChangeoverBypass", cBVAV.Name));
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
                    if (cBVAV.heatCoilType == HVAC::CoilType::HeatingWater) {
                        WaterCoils::SimulateWaterCoilComponents(state, cBVAV.HeatCoilName, FirstHVACIteration, cBVAV.HeatCoilIndex);
                        ErrorFlag = false;
                        Real64 CoilMaxVolFlowRate = WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", cBVAV.HeatCoilName, ErrorFlag);
                        if (ErrorFlag) {
                            ShowContinueError(state, std::format("Occurs in {} = {}", "AirLoopHVAC:UnitaryHeatCool:VAVChangeoverBypass", cBVAV.Name));
                        }
                        if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                            Real64 FluidDensity = cBVAV.plantLoc.loop->glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                            cBVAV.MaxHeatCoilFluidFlow = CoilMaxVolFlowRate * FluidDensity;
                        }
                    }
                    if (cBVAV.heatCoilType == HVAC::CoilType::HeatingSteam) {
                        SteamCoils::SimulateSteamCoilComponents(state,
                                                                cBVAV.HeatCoilName,
                                                                FirstHVACIteration,
                                                                cBVAV.HeatCoilIndex,
                                                                1.0,
                                                                QCoilActual); // QCoilReq, simulate any load > 0 to get max capacity of steam coil
                        ErrorFlag = false;
                        Real64 CoilMaxVolFlowRate = SteamCoils::GetCoilMaxSteamFlowRate(state, cBVAV.HeatCoilIndex, ErrorFlag);
                        if (ErrorFlag) {
                            ShowContinueError(state, std::format("Occurs in {} = {}", "AirLoopHVAC:UnitaryHeatCool:VAVChangeoverBypass", cBVAV.Name));
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
                    ShowWarningError(
                        state,
                        std::format("{} - air flow rate = {:.7f} in fan object {} is less than the maximum CBVAV system air flow rate when "
                                    "cooling is required ({:.7f}).",
                                    CurrentModuleObject,
                                    cBVAV.FanVolFlow,
                                    cBVAV.FanName,
                                    cBVAV.MaxCoolAirVolFlow));
                    ShowContinueError(
                        state, " The CBVAV system flow rate when cooling is required is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state, std::format(" Occurs in Changeover-bypass VAV system = {}", cBVAV.Name));
                    cBVAV.MaxCoolAirVolFlow = cBVAV.FanVolFlow;
                }
                if (cBVAV.FanVolFlow < cBVAV.MaxHeatAirVolFlow) {
                    ShowWarningError(
                        state,
                        std::format("{} - air flow rate = {:.7f} in fan object {} is less than the maximum CBVAV system air flow rate when "
                                    "heating is required ({:.7f}).",
                                    CurrentModuleObject,
                                    cBVAV.FanVolFlow,
                                    cBVAV.FanName,
                                    cBVAV.MaxHeatAirVolFlow));
                    ShowContinueError(
                        state, " The CBVAV system flow rate when heating is required is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state, std::format(" Occurs in Changeover-bypass VAV system = {}", cBVAV.Name));
                    cBVAV.MaxHeatAirVolFlow = cBVAV.FanVolFlow;
                }
                if (cBVAV.FanVolFlow < cBVAV.MaxNoCoolHeatAirVolFlow && cBVAV.MaxNoCoolHeatAirVolFlow != 0.0) {
                    ShowWarningError(
                        state,
                        std::format("{} - air flow rate = {:.7f} in fan object {} is less than the maximum CBVAV system air flow rate when "
                                    "no heating or cooling is needed ({:.7f}).",
                                    CurrentModuleObject,
                                    cBVAV.FanVolFlow,
                                    cBVAV.FanName,
                                    cBVAV.MaxNoCoolHeatAirVolFlow));
                    ShowContinueError(state,
                                      " The CBVAV system flow rate when no heating or cooling is needed is reset to the fan flow rate and the "
                                      "simulation continues.");
                    ShowContinueError(state, std::format(" Occurs in Changeover-bypass VAV system = {}", cBVAV.Name));
                    cBVAV.MaxNoCoolHeatAirVolFlow = cBVAV.FanVolFlow;
                }
                //     Check fan versus outdoor air flow rates
                if (cBVAV.FanVolFlow < cBVAV.CoolOutAirVolFlow) {
                    ShowWarningError(
                        state,
                        std::format("{} - air flow rate = {:.7f} in fan object {} is less than the maximum CBVAV outdoor air flow rate when "
                                    "cooling is required ({:.7f}).",
                                    CurrentModuleObject,
                                    cBVAV.FanVolFlow,
                                    cBVAV.FanName,
                                    cBVAV.CoolOutAirVolFlow));
                    ShowContinueError(
                        state, " The CBVAV outdoor flow rate when cooling is required is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state, std::format(" Occurs in Changeover-bypass VAV system = {}", cBVAV.Name));
                    cBVAV.CoolOutAirVolFlow = cBVAV.FanVolFlow;
                }
                if (cBVAV.FanVolFlow < cBVAV.HeatOutAirVolFlow) {
                    ShowWarningError(
                        state,
                        std::format("{} - air flow rate = {:.7f} in fan object {} is less than the maximum CBVAV outdoor air flow rate when "
                                    "heating is required ({:.7f}).",
                                    CurrentModuleObject,
                                    cBVAV.FanVolFlow,
                                    cBVAV.FanName,
                                    cBVAV.HeatOutAirVolFlow));
                    ShowContinueError(
                        state, " The CBVAV outdoor flow rate when heating is required is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state, std::format(" Occurs in Changeover-bypass VAV system = {}", cBVAV.Name));
                    cBVAV.HeatOutAirVolFlow = cBVAV.FanVolFlow;
                }
                if (cBVAV.FanVolFlow < cBVAV.NoCoolHeatOutAirVolFlow) {
                    ShowWarningError(
                        state,
                        std::format("{} - air flow rate = {:.7f} in fan object {} is less than the maximum CBVAV outdoor air flow rate when "
                                    "no heating or cooling is needed ({:.7f}).",
                                    CurrentModuleObject,
                                    cBVAV.FanVolFlow,
                                    cBVAV.FanName,
                                    cBVAV.NoCoolHeatOutAirVolFlow));
                    ShowContinueError(state,
                                      " The CBVAV outdoor flow rate when no heating or cooling is needed is reset to the fan flow rate and the "
                                      "simulation continues.");
                    ShowContinueError(state, std::format(" Occurs in Changeover-bypass VAV system = {}", cBVAV.Name));
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
                if (state.dataLoopNodes->Node(OutNode).HumRatMax == Node::SensedNodeFlagValue) {
                    if (!state.dataGlobal->AnyEnergyManagementSystemInModel) {
                        ShowWarningError(state, std::format("Unitary System:VAV:ChangeOverBypass = {}", cBVAV.Name));
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
                            ShowWarningError(state, std::format("Unitary System:VAV:ChangeOverBypass = {}", cBVAV.Name));
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
            if (cBVAV.heatCoilType == HVAC::CoilType::HeatingWater) {
                WaterCoils::SimulateWaterCoilComponents(state, cBVAV.HeatCoilName, FirstHVACIteration, cBVAV.HeatCoilIndex);

                //     set air-side and steam-side mass flow rates
                state.dataLoopNodes->Node(cBVAV.HeatingCoilInletNode).MassFlowRate = state.dataHVACUnitaryBypassVAV->CompOnMassFlow;
                mdot = cBVAV.MaxHeatCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(state, mdot, cBVAV.CoilControlNode, cBVAV.CoilOutletNode, cBVAV.plantLoc);

                //     simulate water coil to find operating capacity
                WaterCoils::SimulateWaterCoilComponents(state, cBVAV.HeatCoilName, FirstHVACIteration, cBVAV.HeatCoilIndex, QCoilActual);
                cBVAV.DesignSuppHeatingCapacity = QCoilActual;

            } // from IF(MSHeatPump(MSHeatPumpNum)%SuppHeatCoilType == HVAC::Coil_HeatingWater) THEN

            if (cBVAV.heatCoilType == HVAC::CoilType::HeatingSteam) {

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
                    ShowWarningError(state, std::format("{} \"{}\"", cBVAV.UnitType, cBVAV.Name));
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
                    ShowWarningError(state, std::format("{} \"{}\"", cBVAV.UnitType, cBVAV.Name));
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
                    ShowWarningError(state, std::format("{} \"{}\"", cBVAV.UnitType, cBVAV.Name));
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
                    ShowWarningError(state, std::format("{} \"{}\"", cBVAV.UnitType, cBVAV.Name));
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
                switch (cBVAV.coolCoilType) {
                case HVAC::CoilType::CoolingDXHXAssisted: {
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
                            auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);
                            HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(state,
                                                                                cbvav.DXCoolCoilName,
                                                                                FirstHVACIteration,
                                                                                HVAC::CompressorOp::On,
                                                                                PartLoadFrac,
                                                                                cbvav.CoolCoilCompIndex,
                                                                                HVAC::FanOp::Continuous,
                                                                                HXUnitOn);

                            Real64 OutletAirTemp = state.dataLoopNodes->Node(cbvav.DXCoilOutletNode).Temp;
                            return cbvav.CoilTempSetPoint - OutletAirTemp;
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
                                ShowWarningError(
                                    state,
                                    std::format("Iteration limit exceeded calculating HX assisted DX unit part-load ratio, for unit = {}",
                                                cBVAV.DXCoolCoilName));
                                ShowContinueError(state, std::format("Calculated part-load ratio = {:.3f}", PartLoadFrac));
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
                                    std::format(
                                        "HX assisted DX unit part-load ratio calculation failed: part-load ratio limits exceeded, for unit = {}",
                                        cBVAV.DXCoolCoilName));
                                ShowContinueErrorTimeStamp(
                                    state,
                                    std::format("An estimated part-load ratio of {:.3f} will be used and the simulation continues. Occurrence info:",
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
                case HVAC::CoilType::CoolingDXSingleSpeed: {
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
                            auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);
                            DXCoils::CalcDoe2DXCoil(state,
                                                    cbvav.CoolCoilCompIndex,
                                                    HVAC::CompressorOp::On,
                                                    false,
                                                    PartLoadFrac,
                                                    HVAC::FanOp::Continuous,
                                                    _,
                                                    OnOffAirFlowRatio);
                            Real64 OutletAirTemp = state.dataDXCoils->DXCoilOutletTemp(cbvav.CoolCoilCompIndex);
                            return cbvav.CoilTempSetPoint - OutletAirTemp;
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
                                    std::format("Iteration limit exceeded calculating DX unit part-load ratio, for unit = {}", cBVAV.DXCoolCoilName));
                                ShowContinueError(state, std::format("Calculated part-load ratio = {:.3f}", PartLoadFrac));
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
                                ShowSevereError(
                                    state,
                                    std::format("DX unit part-load ratio calculation failed: part-load ratio limits exceeded, for unit = {}",
                                                cBVAV.DXCoolCoilName));
                                ShowContinueErrorTimeStamp(
                                    state,
                                    std::format("An estimated part-load ratio of {:.3f} will be used and the simulation continues. Occurrence info:",
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
                case HVAC::CoilType::CoolingDXVariableSpeed: {
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
                                    auto const &cbvav = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);
                                    // FUNCTION LOCAL VARIABLE DECLARATIONS:
                                    Real64 OutletAirTemp; // outlet air temperature [C]
                                    Real64 QZnReqCycling = 0.001;
                                    Real64 QLatReqCycling = 0.0;
                                    Real64 OnOffAirFlowRatioCycling = 1.0;
                                    Real64 partLoadRatio = 1.0;
                                    int CoilIndex = cbvav.CoolCoilCompIndex;
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
                                            ShowWarningError(
                                                state,
                                                std::format("{} - Iteration limit exceeded calculating VS DX coil speed ratio for coil named "
                                                            "{}, in Unitary system named{}",
                                                            HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.coolCoilType)],
                                                            cBVAV.DXCoolCoilName,
                                                            cBVAV.Name));
                                            ShowContinueError(state, std::format("Calculated speed ratio = {:.4f}", SpeedRatio));
                                            ShowContinueErrorTimeStamp(
                                                state, "The calculated speed ratio will be used and the simulation continues. Occurrence info:");
                                        }
                                        ShowRecurringWarningErrorAtEnd(
                                            state,
                                            std::format("{} \"{}\" - Iteration limit exceeded calculating speed ratio error "
                                                        "continues. Speed Ratio statistics follow.",
                                                        HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.coolCoilType)],
                                                        cBVAV.DXCoolCoilName),
                                            cBVAV.DXIterationExceededIndex,
                                            LocalPartLoadFrac,
                                            LocalPartLoadFrac);
                                    }
                                } else if (SolFla == -2) {
                                    if (!state.dataGlobal->WarmupFlag) {
                                        if (cBVAV.DXIterationFailed < 4) {
                                            ++cBVAV.DXIterationFailed;
                                            ShowWarningError(
                                                state,
                                                std::format("{} - DX unit speed ratio calculation failed: solver limits exceeded, for coil "
                                                            "named {}, in Unitary system named{}",
                                                            HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.coolCoilType)],
                                                            cBVAV.DXCoolCoilName,
                                                            cBVAV.Name));
                                            ShowContinueError(state, std::format("Estimated speed ratio = {:.3f}", TempSpeedReqst / TempSpeedOut));
                                            ShowContinueErrorTimeStamp(
                                                state, "The estimated part-load ratio will be used and the simulation continues. Occurrence info:");
                                        }
                                        ShowRecurringWarningErrorAtEnd(
                                            state,
                                            std::format(
                                                "{} \"{}\" - DX unit speed ratio calculation failed error continues. speed ratio statistics follow.",
                                                HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.coolCoilType)],
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
                                    auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);
                                    int speedNum = 1;
                                    Real64 speedRatio = 0.0;
                                    Real64 QZnReqCycling = 0.001;
                                    Real64 QLatReqCycling = 0.0;
                                    Real64 OnOffAirFlowRatioCycling = 1.0;
                                    VariableSpeedCoils::SimVariableSpeedCoils(state,
                                                                              "",
                                                                              cbvav.CoolCoilCompIndex,
                                                                              HVAC::FanOp::Continuous,
                                                                              HVAC::CompressorOp::On,
                                                                              PartLoadRatio,
                                                                              speedNum,
                                                                              speedRatio,
                                                                              QZnReqCycling,
                                                                              QLatReqCycling,
                                                                              OnOffAirFlowRatioCycling);

                                    Real64 OutletAirTemp = state.dataVariableSpeedCoils->VarSpeedCoil(cbvav.CoolCoilCompIndex).OutletAirDBTemp;
                                    return DesOutTemp - OutletAirTemp;
                                };
                                General::SolveRoot(state, tempAccuracy, MaxIte, SolFla, LocalPartLoadFrac, f, 1.0e-10, 1.0);
                                if (SolFla == -1) {
                                    if (!state.dataGlobal->WarmupFlag) {
                                        if (cBVAV.DXCyclingIterationExceeded < 4) {
                                            ++cBVAV.DXCyclingIterationExceeded;
                                            ShowWarningError(
                                                state,
                                                std::format("{} - Iteration limit exceeded calculating VS DX unit low speed cycling ratio, "
                                                            "for coil named {}, in Unitary system named{}",
                                                            HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.coolCoilType)],
                                                            cBVAV.DXCoolCoilName,
                                                            cBVAV.Name));
                                            ShowContinueError(state,
                                                              std::format("Estimated cycling ratio  = {:.3f}", (TempSpeedReqst / TempSpeedOut)));
                                            ShowContinueError(state, std::format("Calculated cycling ratio = {:.3f}", LocalPartLoadFrac));
                                            ShowContinueErrorTimeStamp(
                                                state, "The calculated cycling ratio will be used and the simulation continues. Occurrence info:");
                                        }
                                        ShowRecurringWarningErrorAtEnd(
                                            state,
                                            std::format(" {} \"{}\" - Iteration limit exceeded calculating low speed cycling ratio "
                                                        "error continues. Sensible PLR statistics follow.",
                                                        HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.coolCoilType)],
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
                                                std::format("{} - DX unit low speed cycling ratio calculation failed: limits exceeded, for unit = {}",
                                                            HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.coolCoilType)],
                                                            cBVAV.Name));
                                            ShowContinueError(
                                                state, std::format("Estimated low speed cycling ratio = {:.3f}", TempSpeedReqst / TempSpeedOut));
                                            ShowContinueErrorTimeStamp(state,
                                                                       "The estimated low speed cycling ratio will be used and the simulation "
                                                                       "continues. Occurrence info:");
                                        }
                                        ShowRecurringWarningErrorAtEnd(
                                            state,
                                            std::format("{} \"{}\" - DX unit low speed cycling ratio calculation failed error "
                                                        "continues. cycling ratio statistics follow.",
                                                        HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.coolCoilType)],
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
                case HVAC::CoilType::CoolingDXTwoStageWHumControl: {
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
                            auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);
                            DXCoils::SimDXCoilMultiMode(state,
                                                        "",
                                                        HVAC::CompressorOp::On,
                                                        false,
                                                        PartLoadRatio,
                                                        DehumidMode,
                                                        cbvav.CoolCoilCompIndex,
                                                        HVAC::FanOp::Continuous);
                            return cbvav.CoilTempSetPoint - state.dataDXCoils->DXCoilOutletTemp(cbvav.CoolCoilCompIndex);
                        };
                        General::SolveRoot(state, HVAC::SmallTempDiff, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                        if (SolFla == -1) {
                            if (cBVAV.MMDXIterationExceeded < 1) {
                                ++cBVAV.MMDXIterationExceeded;
                                ShowWarningError(
                                    state, std::format("Iteration limit exceeded calculating DX unit part-load ratio, for unit={}", cBVAV.Name));
                                ShowContinueErrorTimeStamp(state, std::format("Part-load ratio returned = {:.2f}", PartLoadFrac));
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
                                    std::format("DX unit part-load ratio calculation failed: part-load ratio limits exceeded, for unit={}",
                                                cBVAV.Name));
                                ShowContinueError(state, std::format("Estimated part-load ratio = {:.3f}", PartLoadFrac));
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
                                auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);
                                DXCoils::SimDXCoilMultiMode(state,
                                                            "",
                                                            HVAC::CompressorOp::On,
                                                            false,
                                                            PartLoadRatio,
                                                            DehumidMode,
                                                            cbvav.CoolCoilCompIndex,
                                                            HVAC::FanOp::Continuous);
                                return cbvav.CoilTempSetPoint - state.dataDXCoils->DXCoilOutletTemp(cbvav.CoolCoilCompIndex);
                            };
                            General::SolveRoot(state, HVAC::SmallTempDiff, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                            if (SolFla == -1) {
                                if (cBVAV.DMDXIterationExceeded < 1) {
                                    ++cBVAV.DMDXIterationExceeded;
                                    ShowWarningError(
                                        state,
                                        std::format("Iteration limit exceeded calculating DX unit dehumidifying part-load ratio, for unit = {}",
                                                    cBVAV.Name));
                                    ShowContinueErrorTimeStamp(state, std::format("Part-load ratio returned={:.2f}", PartLoadFrac));
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
                                                    std::format("DX unit dehumidifying part-load ratio calculation failed: part-load ratio limits "
                                                                "exceeded, for unit = {}",
                                                                cBVAV.Name));
                                    ShowContinueError(state, std::format("Estimated part-load ratio = {:.3f}", PartLoadFrac));
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
                                auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);
                                DXCoils::SimDXCoilMultiMode(state,
                                                            "",
                                                            HVAC::CompressorOp::On,
                                                            false,
                                                            PartLoadRatio,
                                                            DehumidMode,
                                                            cbvav.CoolCoilCompIndex,
                                                            HVAC::FanOp::Continuous);
                                return cbvav.CoilTempSetPoint - state.dataDXCoils->DXCoilOutletTemp(cbvav.CoolCoilCompIndex);
                            };
                            General::SolveRoot(state, HVAC::SmallTempDiff, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                            if (SolFla == -1) {
                                if (cBVAV.CRDXIterationExceeded < 1) {
                                    ++cBVAV.CRDXIterationExceeded;
                                    ShowWarningError(
                                        state,
                                        std::format("Iteration limit exceeded calculating DX unit cool reheat part-load ratio, for unit = {}",
                                                    cBVAV.Name));
                                    ShowContinueErrorTimeStamp(state, std::format("Part-load ratio returned = {:.2f}", PartLoadFrac));
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
                                        std::format(
                                            "DX unit cool reheat part-load ratio calculation failed: part-load ratio limits exceeded, for unit = {}",
                                            cBVAV.Name));
                                    ShowContinueError(state, std::format("Estimated part-load ratio = {:.3f}", PartLoadFrac));
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
                        state,
                        std::format("SimCBVAV System: Invalid DX Cooling Coil={}", HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.coolCoilType)]));
                } break;
                }
            } else { // IF(OutdoorDryBulbTemp .GE. cBVAV%MinOATCompressor)THEN
                //     Simulate DX cooling coil with compressor off
                if (cBVAV.coolCoilType == HVAC::CoilType::CoolingDXHXAssisted) {
                    HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(state,
                                                                        cBVAV.DXCoolCoilName,
                                                                        FirstHVACIteration,
                                                                        HVAC::CompressorOp::Off,
                                                                        0.0,
                                                                        cBVAV.CoolCoilCompIndex,
                                                                        HVAC::FanOp::Continuous,
                                                                        HXUnitOn);
                    state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(cBVAV.DXCoolCoilIndexNum);
                } else if (cBVAV.coolCoilType == HVAC::CoilType::CoolingDXSingleSpeed) {
                    DXCoils::SimDXCoil(state,
                                       cBVAV.DXCoolCoilName,
                                       HVAC::CompressorOp::Off,
                                       FirstHVACIteration,
                                       cBVAV.CoolCoilCompIndex,
                                       HVAC::FanOp::Continuous,
                                       0.0,
                                       OnOffAirFlowRatio);
                    state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(cBVAV.DXCoolCoilIndexNum);
                } else if (cBVAV.coolCoilType == HVAC::CoilType::CoolingDXTwoStageWHumControl) {
                    DXCoils::SimDXCoilMultiMode(state,
                                                cBVAV.DXCoolCoilName,
                                                HVAC::CompressorOp::Off,
                                                FirstHVACIteration,
                                                0.0,
                                                HVAC::CoilMode::Normal,
                                                cBVAV.CoolCoilCompIndex,
                                                HVAC::FanOp::Continuous);
                    state.dataHVACUnitaryBypassVAV->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(cBVAV.DXCoolCoilIndexNum);
                } else if (cBVAV.coolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
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
            if (cBVAV.coolCoilType == HVAC::CoilType::CoolingDXHXAssisted) {
                HVACHXAssistedCoolingCoil::SimHXAssistedCoolingCoil(state,
                                                                    cBVAV.DXCoolCoilName,
                                                                    FirstHVACIteration,
                                                                    HVAC::CompressorOp::Off,
                                                                    0.0,
                                                                    cBVAV.CoolCoilCompIndex,
                                                                    HVAC::FanOp::Continuous,
                                                                    HXUnitOn);
            } else if (cBVAV.coolCoilType == HVAC::CoilType::CoolingDXSingleSpeed) {
                DXCoils::SimDXCoil(state,
                                   cBVAV.DXCoolCoilName,
                                   HVAC::CompressorOp::Off,
                                   FirstHVACIteration,
                                   cBVAV.CoolCoilCompIndex,
                                   HVAC::FanOp::Continuous,
                                   0.0,
                                   OnOffAirFlowRatio);
            } else if (cBVAV.coolCoilType == HVAC::CoilType::CoolingDXVariableSpeed) {
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

            } else if (cBVAV.coolCoilType == HVAC::CoilType::CoolingDXTwoStageWHumControl) {
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
        switch (cBVAV.heatCoilType) {
        case HVAC::CoilType::HeatingDXSingleSpeed: {
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
                            auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);
                            DXCoils::CalcDXHeatingCoil(state, cbvav.HeatCoilIndex, PartLoadFrac, HVAC::FanOp::Continuous, OnOffAirFlowRatio);
                            Real64 OutletAirTemp = state.dataDXCoils->DXCoilOutletTemp(cbvav.HeatCoilIndex);
                            Real64 par2 = min(cbvav.CoilTempSetPoint, cbvav.MaxLATHeating);
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
                                state,
                                std::format("Iteration limit exceeded calculating DX unit part-load ratio, for unit = {}", cBVAV.HeatCoilName));
                            ShowContinueError(state, std::format("Calculated part-load ratio = {:.3f}", PartLoadFrac));
                            ShowContinueErrorTimeStamp(state,
                                                       "The calculated part-load ratio will be used and the simulation continues. Occurrence info:");
                        } else if (SolFla == -2 && !state.dataGlobal->WarmupFlag) {
                            ShowSevereError(state,
                                            std::format("DX unit part-load ratio calculation failed: part-load ratio limits exceeded, for unit = {}",
                                                        cBVAV.HeatCoilName));
                            ShowContinueErrorTimeStamp(
                                state,
                                std::format("A part-load ratio of {:.3f} will be used and the simulation continues. Occurrence info:", PartLoadFrac));
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
        case HVAC::CoilType::HeatingDXVariableSpeed: {
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
                                    ShowWarningError(
                                        state,
                                        std::format("{} - Iteration limit exceeded calculating VS DX coil speed ratio for coil named {}, in "
                                                    "Unitary system named{}",
                                                    HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.heatCoilType)],
                                                    cBVAV.HeatCoilName,
                                                    cBVAV.Name));
                                    ShowContinueError(state, std::format("Calculated speed ratio = {:.4f}", SpeedRatio));
                                    ShowContinueErrorTimeStamp(
                                        state, "The calculated speed ratio will be used and the simulation continues. Occurrence info:");
                                }
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    std::format("{} \"{}\" - Iteration limit exceeded calculating speed ratio error continues. "
                                                "Speed Ratio statistics follow.",
                                                HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.heatCoilType)],
                                                cBVAV.HeatCoilName),
                                    cBVAV.DXHeatIterationExceededIndex,
                                    LocalPartLoadFrac,
                                    LocalPartLoadFrac);
                            }
                        } else if (SolFla == -2) {

                            if (!state.dataGlobal->WarmupFlag) {
                                if (cBVAV.DXHeatIterationFailed < 4) {
                                    ++cBVAV.DXHeatIterationFailed;
                                    ShowWarningError(
                                        state,
                                        std::format("{} - DX unit speed ratio calculation failed: solver limits exceeded, for coil named {}, "
                                                    "in Unitary system named{}",
                                                    HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.heatCoilType)],
                                                    cBVAV.HeatCoilName,
                                                    cBVAV.Name));
                                    ShowContinueErrorTimeStamp(state,
                                                               " Speed ratio will be set to 0.5, and the simulation continues. Occurrence info:");
                                }
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    std::format("{} \"{}\" - DX unit speed ratio calculation failed error continues. speed ratio statistics follow.",
                                                HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.heatCoilType)],
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
                                    ShowWarningError(
                                        state,
                                        std::format("{} - Iteration limit exceeded calculating VS DX unit low speed cycling ratio, for coil "
                                                    "named {}, in Unitary system named{}",
                                                    HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.heatCoilType)],
                                                    cBVAV.HeatCoilName,
                                                    cBVAV.Name));
                                    ShowContinueError(state, std::format("Estimated cycling ratio  = {:.3f}", (DesOutTemp / TempSpeedOut)));
                                    ShowContinueError(state, std::format("Calculated cycling ratio = {:.3f}", LocalPartLoadFrac));
                                    ShowContinueErrorTimeStamp(
                                        state, "The calculated cycling ratio will be used and the simulation continues. Occurrence info:");
                                }
                                ShowRecurringWarningErrorAtEnd(state,
                                                               std::format("{} \"{}\" - Iteration limit exceeded calculating low speed cycling ratio "
                                                                           "error continues. Sensible PLR statistics follow.",
                                                                           HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.heatCoilType)],
                                                                           cBVAV.HeatCoilName),
                                                               cBVAV.DXHeatCyclingIterationExceededIndex,
                                                               LocalPartLoadFrac,
                                                               LocalPartLoadFrac);
                            }
                        } else if (SolFla == -2) {

                            if (!state.dataGlobal->WarmupFlag) {
                                if (cBVAV.DXHeatCyclingIterationFailed < 4) {
                                    ++cBVAV.DXHeatCyclingIterationFailed;
                                    ShowWarningError(
                                        state,
                                        std::format("{} - DX unit low speed cycling ratio calculation failed: limits exceeded, for unit = {}",
                                                    HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.heatCoilType)],
                                                    cBVAV.Name));
                                    ShowContinueError(state,
                                                      std::format("Estimated low speed cycling ratio = {:.3f}",
                                                                  (DesOutTemp - TempNoOutput) / (TempSpeedOutSpeed1 - TempNoOutput)));
                                    ShowContinueErrorTimeStamp(
                                        state, "The estimated low speed cycling ratio will be used and the simulation continues. Occurrence info:");
                                }
                                ShowRecurringWarningErrorAtEnd(state,
                                                               std::format("{} \"{}\" - DX unit low speed cycling ratio calculation failed error "
                                                                           "continues. cycling ratio statistics follow.",
                                                                           HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.heatCoilType)],
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
            ShowFatalError(state,
                           std::format("SimCBVAV System: Invalid Heating Coil={}", HVAC::coilTypeNamesUC[static_cast<int>(cBVAV.heatCoilType)]));
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
        Real64 CalcSetPointTempTarget = 0.0;

        // FUNCTION LOCAL VARIABLE DECLARATIONS:
        Real64 ZoneLoad = 0.0;           // Zone load sensed by thermostat [W]
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
        Real64 ZoneMassFlow;   // Zone mass flow rate required to meet zone load [kg/s]
        Real64 ZoneLoad = 0.0; // Zone load calculated by ZoneTempPredictor [W]

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

        auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);

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

        auto &cbvav = state.dataHVACUnitaryBypassVAV->CBVAV(CBVAVNum);

        if (HeatCoilLoad > HVAC::SmallLoad) {
            switch (cbvav.heatCoilType) {
            case HVAC::CoilType::HeatingGasOrOtherFuel:
            case HVAC::CoilType::HeatingElectric: {
                HeatingCoils::SimulateHeatingCoilComponents(
                    state, cbvav.HeatCoilName, FirstHVACIteration, HeatCoilLoad, cbvav.HeatCoilIndex, QCoilActual, false, fanOp);
            } break;
            case HVAC::CoilType::HeatingWater: {
                // simulate the heating coil at maximum hot water flow rate
                MaxHotWaterFlow = cbvav.MaxHeatCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(state, MaxHotWaterFlow, cbvav.CoilControlNode, cbvav.CoilOutletNode, cbvav.plantLoc);
                WaterCoils::SimulateWaterCoilComponents(state, cbvav.HeatCoilName, FirstHVACIteration, cbvav.HeatCoilIndex, QCoilActual, fanOp);
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
                        if (cbvav.HotWaterCoilMaxIterIndex == 0) {
                            ShowWarningMessage(
                                state, std::format("CalcNonDXHeatingCoils: Hot water coil control failed for {}=\"{}\"", cbvav.UnitType, cbvav.Name));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state,
                                              std::format("  Iteration limit [{}] exceeded in calculating hot water mass flow rate", SolveMaxIter));
                        }
                        ShowRecurringWarningErrorAtEnd(
                            state,
                            std::format("CalcNonDXHeatingCoils: Hot water coil control failed (iteration limit [{}]) for {}=\"{}",
                                        SolveMaxIter,
                                        cbvav.UnitType,
                                        cbvav.Name),
                            cbvav.HotWaterCoilMaxIterIndex);
                    } else if (SolFlag == -2) {
                        if (cbvav.HotWaterCoilMaxIterIndex2 == 0) {
                            ShowWarningMessage(state,
                                               std::format("CalcNonDXHeatingCoils: Hot water coil control failed (maximum flow limits) for {}=\"{}\"",
                                                           cbvav.UnitType,
                                                           cbvav.Name));
                            ShowContinueErrorTimeStamp(state, "");
                            ShowContinueError(state, "...Bad hot water maximum flow rate limits");
                            ShowContinueError(state, std::format("...Given minimum water flow rate={:.3f} kg/s", MinWaterFlow));
                            ShowContinueError(state, std::format("...Given maximum water flow rate={:.3f} kg/s", MaxHotWaterFlow));
                        }
                        ShowRecurringWarningErrorAtEnd(state,
                                                       "CalcNonDXHeatingCoils: Hot water coil control failed (flow limits) for " + cbvav.UnitType +
                                                           "=\"" + cbvav.Name + "\"",
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
                    WaterCoils::SimulateWaterCoilComponents(state, cbvav.HeatCoilName, FirstHVACIteration, cbvav.HeatCoilIndex, QCoilActual, fanOp);
                }
            } break;
            case HVAC::CoilType::HeatingSteam: {
                mdot = cbvav.MaxHeatCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(state, mdot, cbvav.CoilControlNode, cbvav.CoilOutletNode, cbvav.plantLoc);

                // simulate the steam heating coil
                SteamCoils::SimulateSteamCoilComponents(
                    state, cbvav.HeatCoilName, FirstHVACIteration, cbvav.HeatCoilIndex, HeatCoilLoad, QCoilActual, fanOp);
            } break;
            default:
                break;
            }
        } else {
            switch (cbvav.heatCoilType) {
            case HVAC::CoilType::HeatingGasOrOtherFuel:
            case HVAC::CoilType::HeatingElectric: {
                HeatingCoils::SimulateHeatingCoilComponents(
                    state, cbvav.HeatCoilName, FirstHVACIteration, HeatCoilLoad, cbvav.HeatCoilIndex, QCoilActual, false, fanOp);
            } break;
            case HVAC::CoilType::HeatingWater: {
                mdot = 0.0;
                PlantUtilities::SetComponentFlowRate(state, mdot, cbvav.CoilControlNode, cbvav.CoilOutletNode, cbvav.plantLoc);
                QCoilActual = HeatCoilLoad;
                // simulate the hot water heating coil
                WaterCoils::SimulateWaterCoilComponents(state, cbvav.HeatCoilName, FirstHVACIteration, cbvav.HeatCoilIndex, QCoilActual, fanOp);
            } break;
            case HVAC::CoilType::HeatingSteam: {
                mdot = 0.0;
                PlantUtilities::SetComponentFlowRate(state, mdot, cbvav.CoilControlNode, cbvav.CoilOutletNode, cbvav.plantLoc);
                // simulate the steam heating coil
                SteamCoils::SimulateSteamCoilComponents(
                    state, cbvav.HeatCoilName, FirstHVACIteration, cbvav.HeatCoilIndex, HeatCoilLoad, QCoilActual, fanOp);
            } break;
            default:
                break;
            }
        }
        HeatCoilLoadmet = QCoilActual;
    }

} // namespace HVACUnitaryBypassVAV

} // namespace EnergyPlus

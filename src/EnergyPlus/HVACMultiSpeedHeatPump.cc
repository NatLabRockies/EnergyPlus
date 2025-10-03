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
#include <AirflowNetwork/Solver.hpp>
#include <EnergyPlus/Autosizing/Base.hh>
#include <EnergyPlus/Autosizing/CoolingCapacitySizing.hh>
#include <EnergyPlus/Autosizing/HeatingCapacitySizing.hh>
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/DXCoils.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataAirSystems.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
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
#include <EnergyPlus/HVACMultiSpeedHeatPump.hh>
#include <EnergyPlus/HeatingCoils.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/OutputReportPredefined.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/PlantUtilities.hh>
#include <EnergyPlus/Psychrometrics.hh>
#include <EnergyPlus/ScheduleManager.hh>
#include <EnergyPlus/SteamCoils.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/WaterCoils.hh>
#include <EnergyPlus/ZoneTempPredictorCorrector.hh>

namespace EnergyPlus {

namespace HVACMultiSpeedHeatPump {

    // Module containing the Multi Speed Heat Pump simulation routines

    // MODULE INFORMATION:
    //       AUTHOR         Lixing Gu, Florida Solar Energy Center
    //       DATE WRITTEN   June 2007
    //       MODIFIED       Bereket Nigusse, FSEC, June 2010 - deprecated supply air flow fraction through controlled
    //                      zone from the furnace object input field. Now, the flow fraction is calculated internally
    //                      Brent Griffith, NREL, Dec 2010 -- upgrade to new plant for heat recovery, general fluid props.
    //                      Bereket Nigusse, FSEC, Jan. 2012 -- added hot water and steam heating coil
    // PURPOSE OF THIS MODULE:
    // To encapsulate the data and algorithms required to simulate Multi Speed Heat Pump in
    // EnergyPlus.

    // Module currently models air-cooled or evap-cooled direct expansion systems
    // (split or packaged) with multiple speeds. Air-side performance is modeled to determine
    // coil discharge air conditions. The module also determines the DX unit's energy
    // usage. Neither the air-side performance nor the energy usage includes the effect
    // of supply air fan heat/energy usage. The supply air fan is modeled by other modules.

    // Curve Types
    enum class CurveType
    {
        Invalid = -1,
        Linear,      // Linear curve type
        BiLinear,    // Bi-linear curve type
        Quadratic,   // Quadratic curve type
        BiQuadratic, // Bi-quadratic curve type
        Cubic,       // Cubic curve type
        Num
    };

    void SimMSHeatPump(EnergyPlusData &state,
                       std::string_view CompName,     // Name of the unitary engine driven heat pump system
                       bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system time step
                       int const AirLoopNum,          // air loop index
                       int &CompIndex                 // Index to changeover-bypass VAV system
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu, Florida Solar Energy Center
        //       DATE WRITTEN   June. 2007

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int MSHeatPumpNum;        // index of fan coil unit being simulated
        Real64 OnOffAirFlowRatio; // Ratio of compressor ON airflow to average airflow over timestep
        Real64 QZnLoad;           // Zone load required by all zones served by this air loop system
        Real64 QSensUnitOut;      // MSHP sensible capacity output [W]

        // First time SimMSHeatPump is called, get the input
        if (state.dataHVACMultiSpdHP->GetInputFlag) {
            GetMSHeatPumpInput(state);
            state.dataHVACMultiSpdHP->GetInputFlag = false; // Set GetInputFlag false so you don't get coil inputs again
        }

        if (CompIndex == 0) {
            MSHeatPumpNum = Util::FindItemInList(CompName, state.dataHVACMultiSpdHP->MSHeatPump);
            if (MSHeatPumpNum == 0) {
                ShowFatalError(state, format("MultiSpeed Heat Pump is not found={}", CompName));
            }
            CompIndex = MSHeatPumpNum;
        } else {
            MSHeatPumpNum = CompIndex;
            if (MSHeatPumpNum > state.dataHVACMultiSpdHP->NumMSHeatPumps || MSHeatPumpNum < 1) {
                ShowFatalError(state,
                               format("SimMSHeatPump: Invalid CompIndex passed={}, Number of MultiSpeed Heat Pumps={}, Heat Pump name={}",
                                      MSHeatPumpNum,
                                      state.dataHVACMultiSpdHP->NumMSHeatPumps,
                                      CompName));
            }
            if (state.dataHVACMultiSpdHP->CheckEquipName(MSHeatPumpNum)) {
                if (CompName != state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).Name) {
                    ShowFatalError(state,
                                   format("SimMSHeatPump: Invalid CompIndex passed={}, Heat Pump name={}{}",
                                          MSHeatPumpNum,
                                          CompName,
                                          state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).Name));
                }
                state.dataHVACMultiSpdHP->CheckEquipName(MSHeatPumpNum) = false;
            }
        }

        OnOffAirFlowRatio = 0.0;

        // Initialize the engine driven heat pump
        InitMSHeatPump(state, MSHeatPumpNum, FirstHVACIteration, AirLoopNum, QZnLoad, OnOffAirFlowRatio);

        SimMSHP(state, MSHeatPumpNum, FirstHVACIteration, AirLoopNum, QSensUnitOut, QZnLoad, OnOffAirFlowRatio);

        // Update the unit outlet nodes
        UpdateMSHeatPump(state, MSHeatPumpNum);

        // Report the result of the simulation
        ReportMSHeatPump(state, MSHeatPumpNum);
    }

    //******************************************************************************

    void SimMSHP(EnergyPlusData &state,
                 int const MSHeatPumpNum,       // number of the current engine driven Heat Pump being simulated
                 bool const FirstHVACIteration, // TRUE if 1st HVAC simulation of system timestep
                 int const AirLoopNum,          // air loop index
                 Real64 &QSensUnitOut,          // cooling/heating delivered to zones [W]
                 Real64 const QZnReq,           // required zone load
                 Real64 &OnOffAirFlowRatio      // ratio of compressor ON airflow to AVERAGE airflow over timestep
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   June 2007
        //       RE-ENGINEERED  Revised based on SimPTHP

        // PURPOSE OF THIS SUBROUTINE:
        // Simulate a multispeed heat pump; adjust its output to match the
        // required system load.

        // METHODOLOGY EMPLOYED:
        // Calls ControlMSHPOutput to obtain the desired unit output

        // Locals
        Real64 SupHeaterLoad;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 PartLoadFrac; // compressor part load fraction
        Real64 SpeedRatio;   // compressor speed ratio
        bool UnitOn;         // TRUE if unit is on
        int OutletNode;      // MSHP air outlet node
        int InletNode;       // MSHP air inlet node
        Real64 AirMassFlow;  // air mass flow rate [kg/s]
        HVAC::FanOp fanOp;   // operating mode (fan cycling or continuous; DX coil always cycles)
        int ZoneNum;         // Controlled zone number
        Real64 QTotUnitOut;
        int SpeedNum;                    // Speed number
        HVAC::CompressorOp compressorOp; // compressor operation; 1=on, 0=off
        Real64 SaveMassFlowRate;         // saved inlet air mass flow rate [kg/s]

        // zero the fan, DX coils, and supplemental electric heater electricity consumption
        state.dataHVACGlobal->DXElecHeatingPower = 0.0;
        state.dataHVACGlobal->DXElecCoolingPower = 0.0;
        state.dataHVACMultiSpdHP->SaveCompressorPLR = 0.0;
        state.dataHVACGlobal->ElecHeatingCoilPower = 0.0;
        state.dataHVACGlobal->SuppHeatingCoilPower = 0.0;
        state.dataHVACGlobal->DefrostElecPower = 0.0;

        auto &multiSpeedHeatPump = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);

        // initialize local variables
        UnitOn = true;
        OutletNode = multiSpeedHeatPump.AirOutletNodeNum;
        InletNode = multiSpeedHeatPump.AirInletNodeNum;
        AirMassFlow = state.dataLoopNodes->Node(InletNode).MassFlowRate;
        fanOp = multiSpeedHeatPump.fanOp;
        ZoneNum = multiSpeedHeatPump.ControlZoneNum;
        compressorOp = HVAC::CompressorOp::On;

        // set the on/off flags
        if (multiSpeedHeatPump.fanOp == HVAC::FanOp::Cycling) {
            // cycling unit only runs if there is a cooling or heating load.
            if (std::abs(QZnReq) < HVAC::SmallLoad || AirMassFlow < HVAC::SmallMassFlow ||
                state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) {
                UnitOn = false;
            }
        } else if (multiSpeedHeatPump.fanOp == HVAC::FanOp::Continuous) {
            // continuous unit: fan runs if scheduled on; coil runs only if there is a cooling or heating load
            if (AirMassFlow < HVAC::SmallMassFlow) {
                UnitOn = false;
            }
        }

        state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;

        SaveMassFlowRate = state.dataLoopNodes->Node(InletNode).MassFlowRate;
        if (multiSpeedHeatPump.EMSOverrideCoilSpeedNumOn) {
            Real64 SpeedVal = multiSpeedHeatPump.EMSOverrideCoilSpeedNumValue;

            if (!FirstHVACIteration && multiSpeedHeatPump.fanOp == HVAC::FanOp::Cycling && QZnReq < 0.0 &&
                state.dataAirLoop->AirLoopControlInfo(AirLoopNum).EconoActive) {
                compressorOp = HVAC::CompressorOp::Off;
                ControlMSHPOutputEMS(state,
                                     MSHeatPumpNum,
                                     FirstHVACIteration,
                                     compressorOp,
                                     fanOp,
                                     QZnReq,
                                     SpeedVal,
                                     SpeedNum,
                                     SpeedRatio,
                                     PartLoadFrac,
                                     OnOffAirFlowRatio,
                                     SupHeaterLoad);
                if (ceil(SpeedVal) == multiSpeedHeatPump.NumOfSpeedCooling && SpeedRatio == 1.0) {
                    state.dataLoopNodes->Node(InletNode).MassFlowRate = SaveMassFlowRate;
                    compressorOp = HVAC::CompressorOp::On;
                    ControlMSHPOutputEMS(state,
                                         MSHeatPumpNum,
                                         FirstHVACIteration,
                                         compressorOp,
                                         fanOp,
                                         QZnReq,
                                         SpeedVal,
                                         SpeedNum,
                                         SpeedRatio,
                                         PartLoadFrac,
                                         OnOffAirFlowRatio,
                                         SupHeaterLoad);
                }
            } else {
                ControlMSHPOutputEMS(state,
                                     MSHeatPumpNum,
                                     FirstHVACIteration,
                                     compressorOp,
                                     fanOp,
                                     QZnReq,
                                     SpeedVal,
                                     SpeedNum,
                                     SpeedRatio,
                                     PartLoadFrac,
                                     OnOffAirFlowRatio,
                                     SupHeaterLoad);
            }
        } else {
            if (!FirstHVACIteration && multiSpeedHeatPump.fanOp == HVAC::FanOp::Cycling && QZnReq < 0.0 &&
                state.dataAirLoop->AirLoopControlInfo(AirLoopNum).EconoActive) {
                // for cycling fan, cooling load, check whether furnace can meet load with compressor off
                compressorOp = HVAC::CompressorOp::Off;
                ControlMSHPOutput(state,
                                  MSHeatPumpNum,
                                  FirstHVACIteration,
                                  compressorOp,
                                  fanOp,
                                  QZnReq,
                                  ZoneNum,
                                  SpeedNum,
                                  SpeedRatio,
                                  PartLoadFrac,
                                  OnOffAirFlowRatio,
                                  SupHeaterLoad);
                if (SpeedNum == multiSpeedHeatPump.NumOfSpeedCooling && SpeedRatio == 1.0) {
                    // compressor on (reset inlet air mass flow rate to starting value)
                    state.dataLoopNodes->Node(InletNode).MassFlowRate = SaveMassFlowRate;
                    compressorOp = HVAC::CompressorOp::On;
                    ControlMSHPOutput(state,
                                      MSHeatPumpNum,
                                      FirstHVACIteration,
                                      compressorOp,
                                      fanOp,
                                      QZnReq,
                                      ZoneNum,
                                      SpeedNum,
                                      SpeedRatio,
                                      PartLoadFrac,
                                      OnOffAirFlowRatio,
                                      SupHeaterLoad);
                }
            } else {
                // compressor on
                ControlMSHPOutput(state,
                                  MSHeatPumpNum,
                                  FirstHVACIteration,
                                  compressorOp,
                                  fanOp,
                                  QZnReq,
                                  ZoneNum,
                                  SpeedNum,
                                  SpeedRatio,
                                  PartLoadFrac,
                                  OnOffAirFlowRatio,
                                  SupHeaterLoad);
            }
        }

        if (multiSpeedHeatPump.HeatCoilType != HVAC::CoilDX_MultiSpeedHeating) {
            state.dataHVACMultiSpdHP->SaveCompressorPLR = PartLoadFrac;
        } else {
            if (SpeedNum > 1) {
                state.dataHVACMultiSpdHP->SaveCompressorPLR = 1.0;
            }

            if (PartLoadFrac == 1.0 && state.dataHVACMultiSpdHP->SaveCompressorPLR < 1.0 && (!multiSpeedHeatPump.Staged)) {
                PartLoadFrac = state.dataHVACMultiSpdHP->SaveCompressorPLR;
            }
        }

        CalcMSHeatPump(state,
                       MSHeatPumpNum,
                       FirstHVACIteration,
                       compressorOp,
                       SpeedNum,
                       SpeedRatio,
                       PartLoadFrac,
                       QSensUnitOut,
                       QZnReq,
                       OnOffAirFlowRatio,
                       SupHeaterLoad);

        // calculate delivered capacity
        AirMassFlow = state.dataLoopNodes->Node(InletNode).MassFlowRate;

        QTotUnitOut = AirMassFlow * (state.dataLoopNodes->Node(OutletNode).Enthalpy -
                                     state.dataLoopNodes->Node(multiSpeedHeatPump.NodeNumOfControlledZone).Enthalpy);

        // report variables
        multiSpeedHeatPump.CompPartLoadRatio = state.dataHVACMultiSpdHP->SaveCompressorPLR;
        if (multiSpeedHeatPump.fanOp == HVAC::FanOp::Cycling) {
            if (SupHeaterLoad > 0.0) {
                multiSpeedHeatPump.FanPartLoadRatio = 1.0;
            } else {
                if (SpeedNum < 2) {
                    multiSpeedHeatPump.FanPartLoadRatio = PartLoadFrac;
                } else {
                    multiSpeedHeatPump.FanPartLoadRatio = 1.0;
                }
            }
        } else {
            if (UnitOn) {
                multiSpeedHeatPump.FanPartLoadRatio = 1.0;
            } else {
                if (SpeedNum < 2) {
                    multiSpeedHeatPump.FanPartLoadRatio = PartLoadFrac;
                } else {
                    multiSpeedHeatPump.FanPartLoadRatio = 1.0;
                }
            }
        }

        if (multiSpeedHeatPump.HeatCoolMode == ModeOfOperation::HeatingMode) {
            multiSpeedHeatPump.TotHeatEnergyRate = std::abs(max(0.0, QTotUnitOut));
            multiSpeedHeatPump.SensHeatEnergyRate = std::abs(max(0.0, QSensUnitOut));
            multiSpeedHeatPump.LatHeatEnergyRate = std::abs(max(0.0, (QTotUnitOut - QSensUnitOut)));
            multiSpeedHeatPump.TotCoolEnergyRate = 0.0;
            multiSpeedHeatPump.SensCoolEnergyRate = 0.0;
            multiSpeedHeatPump.LatCoolEnergyRate = 0.0;
        }
        if (multiSpeedHeatPump.HeatCoolMode == ModeOfOperation::CoolingMode) {
            multiSpeedHeatPump.TotCoolEnergyRate = std::abs(min(0.0, QTotUnitOut));
            multiSpeedHeatPump.SensCoolEnergyRate = std::abs(min(0.0, QSensUnitOut));
            multiSpeedHeatPump.LatCoolEnergyRate = std::abs(min(0.0, (QTotUnitOut - QSensUnitOut)));
            multiSpeedHeatPump.TotHeatEnergyRate = 0.0;
            multiSpeedHeatPump.SensHeatEnergyRate = 0.0;
            multiSpeedHeatPump.LatHeatEnergyRate = 0.0;
        }

        multiSpeedHeatPump.AuxElecPower = multiSpeedHeatPump.AuxOnCyclePower * state.dataHVACMultiSpdHP->SaveCompressorPLR +
                                          multiSpeedHeatPump.AuxOffCyclePower * (1.0 - state.dataHVACMultiSpdHP->SaveCompressorPLR);
        Real64 locFanElecPower = 0.0;
        locFanElecPower = state.dataFans->fans(multiSpeedHeatPump.FanNum)->totalPower;
        multiSpeedHeatPump.ElecPower = locFanElecPower + state.dataHVACGlobal->DXElecCoolingPower + state.dataHVACGlobal->DXElecHeatingPower +
                                       state.dataHVACGlobal->ElecHeatingCoilPower + state.dataHVACGlobal->SuppHeatingCoilPower +
                                       state.dataHVACGlobal->DefrostElecPower + multiSpeedHeatPump.AuxElecPower;
    }

    //******************************************************************************

    void GetMSHeatPumpInput(EnergyPlusData &state)
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Lixing Gu, FSEC
        //       DATE WRITTEN:    July 2007

        // PURPOSE OF THIS SUBROUTINE:
        //  This routine will get the input required by the multispeed heat pump model

        using namespace OutputReportPredefined;

        // PARAMETERS
        static constexpr std::string_view RoutineName("GetMSHeatPumpInput: "); // include trailing blank space
        static constexpr std::string_view routineName = "GetMSHeatPumpInput";

        // LOCAL VARIABLES
        int NumAlphas;                 // Number of elements in the alpha array
        int NumNumbers;                // Number of Numbers for each GetObjectItem call
        int IOStatus;                  // Used in GetObjectItem
        bool ErrorsFound(false);       // True when input errors found
        bool IsNotOK;                  // Flag to verify name
        bool AirNodeFound;             // True when an air node is found
        bool AirLoopFound;             // True when an air loop is found
        int i;                         // Index to speeds
        int j;                         // Index to speeds
        bool Found;                    // Flag to find autosize
        bool LocalError;               // Local error flag
        Array1D_string Alphas;         // Alpha input items for object
        Array1D_string cAlphaFields;   // Alpha field names
        Array1D_string cNumericFields; // Numeric field names
        Array1D<Real64> Numbers;       // Numeric input items for object
        Array1D_bool lAlphaBlanks;     // Logical array, alpha field input BLANK = .TRUE.
        Array1D_bool lNumericBlanks;   // Logical array, numeric field input BLANK = .TRUE.
        int MaxNums(0);                // Maximum number of numeric input fields
        int MaxAlphas(0);              // Maximum number of alpha input fields
        int TotalArgs(0);              // Total number of alpha and numeric arguments (max) for a
        //  certain object in the input file
        bool errFlag;
        Real64 SteamDensity; // density of steam at 100C

        auto &MSHeatPump(state.dataHVACMultiSpdHP->MSHeatPump);

        if (MSHeatPump.allocated()) {
            return;
        }

        state.dataHVACMultiSpdHP->CurrentModuleObject =
            "AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed"; // Object type for getting and error messages

        state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(
            state, state.dataHVACMultiSpdHP->CurrentModuleObject, TotalArgs, NumAlphas, NumNumbers);
        MaxNums = max(MaxNums, NumNumbers);
        MaxAlphas = max(MaxAlphas, NumAlphas);

        Alphas.allocate(MaxAlphas);
        cAlphaFields.allocate(MaxAlphas);
        Numbers.dimension(MaxNums, 0.0);
        cNumericFields.allocate(MaxNums);
        lAlphaBlanks.dimension(MaxAlphas, true);
        lNumericBlanks.dimension(MaxNums, true);

        state.dataHVACMultiSpdHP->NumMSHeatPumps =
            state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, state.dataHVACMultiSpdHP->CurrentModuleObject);

        if (state.dataHVACMultiSpdHP->NumMSHeatPumps <= 0) {
            ShowSevereError(state, format("No {} objects specified in input file.", state.dataHVACMultiSpdHP->CurrentModuleObject));
            ErrorsFound = true;
        }

        // ALLOCATE ARRAYS
        MSHeatPump.allocate(state.dataHVACMultiSpdHP->NumMSHeatPumps);
        state.dataHVACMultiSpdHP->MSHeatPumpReport.allocate(state.dataHVACMultiSpdHP->NumMSHeatPumps);
        state.dataHVACMultiSpdHP->CheckEquipName.dimension(state.dataHVACMultiSpdHP->NumMSHeatPumps, true);

        // Load arrays with reformulated electric EIR chiller data
        for (int MSHPNum = 1; MSHPNum <= state.dataHVACMultiSpdHP->NumMSHeatPumps; ++MSHPNum) {
            auto &thisMSHP = MSHeatPump(MSHPNum);
            int HeatingCoilInletNode = 0;
            int HeatingCoilOutletNode = 0;
            int CoolingCoilInletNode = 0;
            int CoolingCoilOutletNode = 0;
            int SuppHeatCoilInletNode = 0;
            int SuppHeatCoilOutletNode = 0;

            state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                     state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                                     MSHPNum,
                                                                     Alphas,
                                                                     NumAlphas,
                                                                     Numbers,
                                                                     NumNumbers,
                                                                     IOStatus,
                                                                     lNumericBlanks,
                                                                     lAlphaBlanks,
                                                                     cAlphaFields,
                                                                     cNumericFields);

            thisMSHP.Name = Alphas(1);

            ErrorObjectHeader eoh{routineName, state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name};

            if (lAlphaBlanks(2)) {
                thisMSHP.availSched = Sched::GetScheduleAlwaysOn(state);
            } else if ((thisMSHP.availSched = Sched::GetSchedule(state, Alphas(2))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(2), Alphas(2));
                ErrorsFound = true;
            }

            thisMSHP.AirInletNodeName = Alphas(3);
            thisMSHP.AirOutletNodeName = Alphas(4);
            thisMSHP.AirInletNodeNum = GetOnlySingleNode(state,
                                                         Alphas(3),
                                                         ErrorsFound,
                                                         DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatPumpAirToAirMultiSpeed,
                                                         Alphas(1),
                                                         DataLoopNode::NodeFluidType::Air,
                                                         DataLoopNode::ConnectionType::Inlet,
                                                         NodeInputManager::CompFluidStream::Primary,
                                                         DataLoopNode::ObjectIsParent);

            thisMSHP.AirOutletNodeNum = GetOnlySingleNode(state,
                                                          Alphas(4),
                                                          ErrorsFound,
                                                          DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatPumpAirToAirMultiSpeed,
                                                          Alphas(1),
                                                          DataLoopNode::NodeFluidType::Air,
                                                          DataLoopNode::ConnectionType::Outlet,
                                                          NodeInputManager::CompFluidStream::Primary,
                                                          DataLoopNode::ObjectIsParent);

            BranchNodeConnections::TestCompSet(state, state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1), Alphas(3), Alphas(4), "Air Nodes");

            // Get the Controlling Zone or Location of the engine driven heat pump Thermostat
            thisMSHP.ControlZoneNum = Util::FindItemInList(Alphas(5), state.dataHeatBal->Zone);
            thisMSHP.ControlZoneName = Alphas(5);
            if (thisMSHP.ControlZoneNum == 0) {
                ShowSevereError(state,
                                format("{}, \"{}\" {} not found: {}",
                                       state.dataHVACMultiSpdHP->CurrentModuleObject,
                                       thisMSHP.Name,
                                       cAlphaFields(5),
                                       thisMSHP.ControlZoneName));
                ErrorsFound = true;
            }

            // Get the node number for the zone with the thermostat
            if (thisMSHP.ControlZoneNum > 0) {
                AirNodeFound = false;
                AirLoopFound = false;
                int ControlledZoneNum = thisMSHP.ControlZoneNum;
                // Find the controlled zone number for the specified thermostat location
                thisMSHP.NodeNumOfControlledZone = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).ZoneNode;
                // Determine if furnace is on air loop served by the thermostat location specified
                for (int zoneInNode = 1; zoneInNode <= state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).NumInletNodes; ++zoneInNode) {
                    int AirLoopNumber = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNodeAirLoopNum(zoneInNode);
                    if (AirLoopNumber > 0) {
                        for (int BranchNum = 1; BranchNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).NumBranches; ++BranchNum) {
                            for (int CompNum = 1;
                                 CompNum <= state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).TotalComponents;
                                 ++CompNum) {
                                if (!Util::SameString(state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).Comp(CompNum).Name,
                                                      thisMSHP.Name) ||
                                    !Util::SameString(
                                        state.dataAirSystemsData->PrimaryAirSystems(AirLoopNumber).Branch(BranchNum).Comp(CompNum).TypeOf,
                                        state.dataHVACMultiSpdHP->CurrentModuleObject)) {
                                    continue;
                                }
                                AirLoopFound = true;
                                thisMSHP.AirLoopNumber = AirLoopNumber;
                                break;
                            }
                            thisMSHP.ZoneInletNode = state.dataZoneEquip->ZoneEquipConfig(ControlledZoneNum).InletNode(zoneInNode);
                            if (AirLoopFound) {
                                break;
                            }
                        }
                        for (int TstatZoneNum = 1; TstatZoneNum <= state.dataZoneCtrls->NumTempControlledZones; ++TstatZoneNum) {
                            if (state.dataZoneCtrls->TempControlledZone(TstatZoneNum).ActualZoneNum != thisMSHP.ControlZoneNum) {
                                continue;
                            }
                            AirNodeFound = true;
                        }
                        for (int TstatZoneNum = 1; TstatZoneNum <= state.dataZoneCtrls->NumComfortControlledZones; ++TstatZoneNum) {
                            if (state.dataZoneCtrls->ComfortControlledZone(TstatZoneNum).ActualZoneNum != thisMSHP.ControlZoneNum) {
                                continue;
                            }
                            AirNodeFound = true;
                        }
                        for (int TstatZoneNum = 1; TstatZoneNum <= state.dataZoneTempPredictorCorrector->NumStageCtrZone; ++TstatZoneNum) {
                            if (state.dataZoneCtrls->StageControlledZone(TstatZoneNum).ActualZoneNum != thisMSHP.ControlZoneNum) {
                                continue;
                            }
                            AirNodeFound = true;
                        }
                    }
                    if (AirLoopFound) {
                        break;
                    }
                }
                if (!AirNodeFound) {
                    ShowSevereError(state,
                                    format("Did not find Air Node ({}), {} = \"\"{}",
                                           cAlphaFields(5),
                                           state.dataHVACMultiSpdHP->CurrentModuleObject,
                                           thisMSHP.Name));
                    ShowContinueError(state, format("Specified {} = {}", cAlphaFields(5), Alphas(5)));
                    ErrorsFound = true;
                }
                if (!AirLoopFound) {
                    ShowSevereError(
                        state, format("Did not find correct AirLoopHVAC for {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(state, format("The {} = {} is not served by this Primary Air Loop equipment.", cAlphaFields(5), Alphas(5)));
                    ErrorsFound = true;
                }
            }

            // Get supply fan data
            thisMSHP.FanNum = Fans::GetFanIndex(state, Alphas(7));
            if (thisMSHP.FanNum == 0) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(7), Alphas(7));
                ErrorsFound = true;
            } else {
                auto *fan = state.dataFans->fans(thisMSHP.FanNum);
                thisMSHP.FanName = fan->Name;
                thisMSHP.fanType = fan->type;
                thisMSHP.FanInletNode = fan->inletNodeNum;
                thisMSHP.FanOutletNode = fan->outletNodeNum;
                BranchNodeConnections::SetUpCompSets(state,
                                                     state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                     thisMSHP.Name,
                                                     HVAC::fanTypeNames[(int)thisMSHP.fanType],
                                                     thisMSHP.FanName,
                                                     "UNDEFINED",
                                                     "UNDEFINED");
            }

            // Get supply fan placement data
            thisMSHP.fanPlace = static_cast<HVAC::FanPlace>(getEnumValue(HVAC::fanPlaceNamesUC, Alphas(8)));
            assert(thisMSHP.fanPlace != HVAC::FanPlace::Invalid);

            if ((thisMSHP.fanOpModeSched = Sched::GetSchedule(state, Alphas(9))) == nullptr) {
                ShowSevereItemNotFound(state, eoh, cAlphaFields(9), Alphas(9));
                ErrorsFound = true;
            }

            if (thisMSHP.fanOpModeSched != nullptr && thisMSHP.fanType == HVAC::FanType::Constant) {
                if (!thisMSHP.fanOpModeSched->checkMinMaxVals(state, Clusive::Ex, 0.0, Clusive::In, 1.0)) {
                    Sched::ShowSevereBadMinMax(state,
                                               eoh,
                                               cAlphaFields(9),
                                               Alphas(9),
                                               Clusive::Ex,
                                               0.0,
                                               Clusive::In,
                                               1.0,
                                               "Fan mode must be continuous (schedule values > 0) for Fan:ConstantVolume.");
                    ErrorsFound = true;
                }
            }

            if (Util::SameString(Alphas(10), "Coil:Heating:DX:MultiSpeed")) {
                thisMSHP.HeatCoilType = HVAC::CoilDX_MultiSpeedHeating;
                thisMSHP.HeatCoilNum = state.dataInputProcessing->inputProcessor->getObjectItemNum(state, "Coil:Heating:DX:MultiSpeed", Alphas(11));
                thisMSHP.DXHeatCoilName = Alphas(11);
                if (thisMSHP.HeatCoilNum <= 0) {
                    ShowSevereError(state, format("Configuration error in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("{} \"{}\" not found.", cAlphaFields(11), Alphas(11)));
                    ShowContinueError(state, format("{} must be Coil:Heating:DX:MultiSpeed ", cAlphaFields(10)));
                    ShowFatalError(state,
                                   format("{}Errors found in getting {} input. Preceding condition(s) causes termination.",
                                          RoutineName,
                                          state.dataHVACMultiSpdHP->CurrentModuleObject));
                    ErrorsFound = true;
                }
                LocalError = false;
                DXCoils::GetDXCoilIndex(state, thisMSHP.DXHeatCoilName, thisMSHP.DXHeatCoilIndex, LocalError, "Coil:Heating:DX:MultiSpeed");
                if (LocalError) {
                    ShowSevereError(state, format("The index of {} is not found \"{}\"", cAlphaFields(11), Alphas(11)));
                    ShowContinueError(state, format("...occurs in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                    LocalError = false;
                }
                HeatingCoilInletNode = DXCoils::GetCoilInletNode(state, Alphas(10), Alphas(11), LocalError);
                if (LocalError) {
                    ShowSevereError(state, format("The inlet node number of {} is not found \"{}\"", cAlphaFields(11), Alphas(11)));
                    ShowContinueError(state, format("...occurs in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                    LocalError = false;
                }
                HeatingCoilOutletNode = DXCoils::GetCoilOutletNode(state, Alphas(10), Alphas(11), LocalError);
                if (LocalError) {
                    ShowSevereError(state, format("The outlet node number of {} is not found \"{}\"", cAlphaFields(11), Alphas(11)));
                    ShowContinueError(state, format("...occurs in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                    LocalError = false;
                }
                thisMSHP.MinOATCompressorHeating = DXCoils::GetMinOATCompressor(state, thisMSHP.DXHeatCoilIndex, LocalError);
                if (LocalError) {
                    ShowContinueError(state,
                                      format("...for heating coil. Occurs in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    LocalError = false;
                }
                BranchNodeConnections::SetUpCompSets(state,
                                                     state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                     thisMSHP.Name,
                                                     "Coil:Heating:DX:MultiSpeed",
                                                     thisMSHP.DXHeatCoilName,
                                                     "UNDEFINED",
                                                     "UNDEFINED");
            } else if (Util::SameString(Alphas(10), "Coil:Heating:Electric:MultiStage") ||
                       Util::SameString(Alphas(10), "Coil:Heating:Gas:MultiStage")) {

                if (Util::SameString(Alphas(10), "Coil:Heating:Electric:MultiStage")) {
                    thisMSHP.HeatCoilType = HVAC::Coil_HeatingElectric_MultiStage;
                    thisMSHP.HeatCoilNum =
                        state.dataInputProcessing->inputProcessor->getObjectItemNum(state, "Coil:Heating:Electric:MultiStage", Alphas(11));
                    if (thisMSHP.HeatCoilNum <= 0) {
                        ShowSevereError(state, format("Configuration error in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state, format("{} \"{}\" not found.", cAlphaFields(11), Alphas(11)));
                        ShowContinueError(state, format("{} must be Coil:Heating:Electric:MultiStage ", cAlphaFields(10)));
                        ShowFatalError(state,
                                       format("{}Errors found in getting {} input. Preceding condition(s) causes termination.",
                                              RoutineName,
                                              state.dataHVACMultiSpdHP->CurrentModuleObject));
                        ErrorsFound = true;
                    }
                } else {
                    thisMSHP.HeatCoilType = HVAC::Coil_HeatingGas_MultiStage;
                    thisMSHP.HeatCoilNum =
                        state.dataInputProcessing->inputProcessor->getObjectItemNum(state, "Coil:Heating:Gas:MultiStage", Alphas(11));
                    if (thisMSHP.HeatCoilNum <= 0) {
                        ShowSevereError(state, format("Configuration error in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                        ShowContinueError(state, format("{} \"{}\" not found.", cAlphaFields(11), Alphas(11)));
                        ShowContinueError(state, format("{} must be Coil:Heating:Gas:MultiStage ", cAlphaFields(10)));
                        ShowFatalError(state,
                                       format("{}Errors found in getting {} input. Preceding condition(s) causes termination.",
                                              RoutineName,
                                              state.dataHVACMultiSpdHP->CurrentModuleObject));
                        ErrorsFound = true;
                    }
                }
                thisMSHP.HeatCoilName = Alphas(11);
                LocalError = false;
                if (Util::SameString(Alphas(10), "Coil:Heating:Electric:MultiStage")) {
                    HeatingCoils::GetCoilIndex(state, thisMSHP.HeatCoilName, thisMSHP.HeatCoilIndex, LocalError);
                } else {
                    HeatingCoils::GetCoilIndex(state, thisMSHP.HeatCoilName, thisMSHP.HeatCoilIndex, LocalError);
                }
                if (LocalError) {
                    ShowSevereError(state, format("The index of {} is not found \"{}\"", cAlphaFields(11), Alphas(11)));
                    ShowContinueError(state, format("...occurs in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                    LocalError = false;
                }
                HeatingCoilInletNode = HeatingCoils::GetCoilInletNode(state, Alphas(10), Alphas(11), LocalError);
                if (LocalError) {
                    ShowSevereError(state, format("The inlet node number of {} is not found \"{}\"", cAlphaFields(11), Alphas(11)));
                    ShowContinueError(state, format("...occurs in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                    LocalError = false;
                }
                HeatingCoilOutletNode = HeatingCoils::GetCoilOutletNode(state, Alphas(10), Alphas(11), LocalError);
                if (LocalError) {
                    ShowSevereError(state, format("The outlet node number of {} is not found \"{}\"", cAlphaFields(11), Alphas(11)));
                    ShowContinueError(state, format("...occurs in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                    LocalError = false;
                }
                if (Util::SameString(Alphas(10), "Coil:Heating:Electric:MultiStage")) {
                    BranchNodeConnections::SetUpCompSets(state,
                                                         state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                         thisMSHP.Name,
                                                         "Coil:Heating:Electric:MultiStage",
                                                         thisMSHP.HeatCoilName,
                                                         "UNDEFINED",
                                                         "UNDEFINED");
                } else {
                    BranchNodeConnections::SetUpCompSets(state,
                                                         state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                         thisMSHP.Name,
                                                         "Coil:Heating:Gas:MultiStage",
                                                         thisMSHP.HeatCoilName,
                                                         "UNDEFINED",
                                                         "UNDEFINED");
                }
            } else if (Util::SameString(Alphas(10), "Coil:Heating:Water")) {
                thisMSHP.HeatCoilType = HVAC::Coil_HeatingWater;
                ValidateComponent(state, Alphas(10), Alphas(11), IsNotOK, state.dataHVACMultiSpdHP->CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else { // mine data from heating coil object

                    thisMSHP.HeatCoilName = Alphas(11);
                    // Get the Heating Coil water Inlet or control Node number
                    errFlag = false;
                    thisMSHP.CoilControlNode = WaterCoils::GetCoilWaterInletNode(state, "Coil:Heating:Water", thisMSHP.HeatCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                        ErrorsFound = true;
                    }

                    // Get the ReHeat Coil hot water max volume flow rate
                    errFlag = false;
                    thisMSHP.MaxCoilFluidFlow = WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", thisMSHP.HeatCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                        ErrorsFound = true;
                    }

                    // Get the supplemental Heating Coil Inlet Node
                    errFlag = false;
                    HeatingCoilInletNode = WaterCoils::GetCoilInletNode(state, "Coil:Heating:Water", thisMSHP.HeatCoilName, errFlag);
                    thisMSHP.CoilAirInletNode = HeatingCoilInletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                        ErrorsFound = true;
                    }

                    // Get the supplemental Heating Coil Outlet Node
                    errFlag = false;
                    HeatingCoilOutletNode = WaterCoils::GetCoilOutletNode(state, "Coil:Heating:Water", thisMSHP.HeatCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                        ErrorsFound = true;
                    }
                    BranchNodeConnections::SetUpCompSets(state,
                                                         state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                         thisMSHP.Name,
                                                         "Coil:Heating:Water",
                                                         thisMSHP.HeatCoilName,
                                                         state.dataLoopNodes->NodeID(HeatingCoilInletNode),
                                                         state.dataLoopNodes->NodeID(HeatingCoilOutletNode));
                }
            } else if (Util::SameString(Alphas(10), "Coil:Heating:Steam")) {
                thisMSHP.HeatCoilType = HVAC::Coil_HeatingSteam;
                ValidateComponent(state, Alphas(10), Alphas(11), IsNotOK, state.dataHVACMultiSpdHP->CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ErrorsFound = true;
                } else { // mine data from heating coil object

                    thisMSHP.HeatCoilName = Alphas(11);
                    errFlag = false;
                    thisMSHP.HeatCoilNum = SteamCoils::GetSteamCoilIndex(state, Alphas(10), thisMSHP.HeatCoilName, errFlag);
                    if (thisMSHP.HeatCoilNum == 0) {
                        ShowSevereError(
                            state,
                            format("{} illegal {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, cAlphaFields(10), thisMSHP.HeatCoilName));
                        ShowContinueError(state, format("Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                        ErrorsFound = true;
                    }

                    // Get the supplemental Heating Coil steam inlet node number
                    errFlag = false;
                    thisMSHP.CoilControlNode = SteamCoils::GetCoilAirOutletNode(state, "Coil:Heating:Steam", thisMSHP.HeatCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                        ErrorsFound = true;
                    }

                    // Get the supplemental Heating Coil steam max volume flow rate
                    thisMSHP.MaxCoilFluidFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, thisMSHP.HeatCoilNum, errFlag);
                    if (thisMSHP.MaxCoilFluidFlow > 0.0) {
                        SteamDensity = Fluid::GetSteam(state)->getSatDensity(state, state.dataHVACMultiSpdHP->TempSteamIn, 1.0, routineName);
                        thisMSHP.MaxCoilFluidFlow *= SteamDensity;
                    }

                    // Get the supplemental Heating Coil Inlet Node
                    errFlag = false;
                    HeatingCoilInletNode = SteamCoils::GetCoilAirInletNode(state, thisMSHP.HeatCoilNum, thisMSHP.HeatCoilName, errFlag);
                    thisMSHP.CoilAirInletNode = HeatingCoilInletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                        ErrorsFound = true;
                    }

                    // Get the supplemental Heating Coil Outlet Node
                    errFlag = false;
                    HeatingCoilOutletNode = SteamCoils::GetCoilAirOutletNode(state, thisMSHP.HeatCoilNum, thisMSHP.HeatCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                        ErrorsFound = true;
                    }

                    BranchNodeConnections::SetUpCompSets(state,
                                                         state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                         thisMSHP.Name,
                                                         "Coil:Heating:Steam",
                                                         thisMSHP.HeatCoilName,
                                                         state.dataLoopNodes->NodeID(HeatingCoilInletNode),
                                                         state.dataLoopNodes->NodeID(HeatingCoilOutletNode));
                }
            } else {
                ShowSevereError(state,
                                format("The allowed {} are Coil:Heating:DX:MultiSpeed, Coil:Heating:Electric:MultiStage, and "
                                       "Coil:Heating:Gas:MultiStage  in {} \"{}\"",
                                       cAlphaFields(10),
                                       state.dataHVACMultiSpdHP->CurrentModuleObject,
                                       Alphas(1)));
                ShowContinueError(state, format("The entered {} = \"{}\".", cAlphaFields(10), Alphas(10)));
                ErrorsFound = true;
            }

            if (Util::SameString(Alphas(12), "Coil:Cooling:DX:MultiSpeed")) {
                thisMSHP.CoolCoilType = HVAC::CoilDX_MultiSpeedCooling;
                thisMSHP.DXCoolCoilName = Alphas(13);
                if (state.dataInputProcessing->inputProcessor->getObjectItemNum(state, "Coil:Cooling:DX:MultiSpeed", Alphas(13)) <= 0) {
                    ShowSevereError(state, format("Configuration error in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("{} \"{}\" not found.", cAlphaFields(13), Alphas(13)));
                    ShowContinueError(state, format("{} must be Coil:Cooling:DX:MultiSpeed ", cAlphaFields(12)));
                    ShowFatalError(state,
                                   format("{}Errors found in getting {} input. Preceding condition(s) causes termination.",
                                          RoutineName,
                                          state.dataHVACMultiSpdHP->CurrentModuleObject));
                    ErrorsFound = true;
                }
                LocalError = false;
                DXCoils::GetDXCoilIndex(state, thisMSHP.DXCoolCoilName, thisMSHP.DXCoolCoilIndex, LocalError, "Coil:Cooling:DX:MultiSpeed");
                if (LocalError) {
                    ShowSevereError(state, format("The index of {} is not found \"{}\"", cAlphaFields(13), Alphas(13)));
                    ShowContinueError(state, format("...occurs in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                    LocalError = false;
                }
                CoolingCoilInletNode = DXCoils::GetCoilInletNode(state, Alphas(12), Alphas(13), LocalError);
                if (LocalError) {
                    ShowSevereError(state, format("The inlet node number of {} is not found \"{}\"", cAlphaFields(13), Alphas(13)));
                    ShowContinueError(state, format("...occurs in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                    LocalError = false;
                }
                CoolingCoilOutletNode = DXCoils::GetCoilOutletNode(state, Alphas(12), Alphas(13), LocalError);
                if (LocalError) {
                    ShowSevereError(state, format("The outlet node number of {} is not found \"{}\"", cAlphaFields(13), Alphas(13)));
                    ShowContinueError(state, format("...occurs in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                    LocalError = false;
                }
                thisMSHP.MinOATCompressorCooling = DXCoils::GetMinOATCompressor(state, thisMSHP.DXCoolCoilIndex, LocalError);
                if (LocalError) {
                    ShowContinueError(state,
                                      format("...for cooling coil. Occurs in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    LocalError = false;
                }
            } else {
                ShowSevereError(state,
                                format("The allowed {} is Coil:Cooling:DX:MultiSpeed in {} \"{}\"",
                                       cAlphaFields(12),
                                       state.dataHVACMultiSpdHP->CurrentModuleObject,
                                       Alphas(1)));
                ShowContinueError(state, format("The entered {} = \"{}\".", cAlphaFields(12), Alphas(12)));
                ErrorsFound = true;
            }
            BranchNodeConnections::SetUpCompSets(state,
                                                 state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                 thisMSHP.Name,
                                                 "Coil:Cooling:DX:MultiSpeed",
                                                 thisMSHP.DXCoolCoilName,
                                                 "UNDEFINED",
                                                 "UNDEFINED");

            // Get supplemental heating coil data
            thisMSHP.SuppHeatCoilName = Alphas(15);
            if (Util::SameString(Alphas(14), "Coil:Heating:Fuel")) {
                thisMSHP.SuppHeatCoilType = HVAC::Coil_HeatingGasOrOtherFuel;
                errFlag = false;
                thisMSHP.SuppHeatCoilNum = HeatingCoils::GetHeatingCoilIndex(state, "Coil:Heating:Fuel", Alphas(15), errFlag);
                if (thisMSHP.SuppHeatCoilNum <= 0 || errFlag) {
                    ShowContinueError(state, format("Configuration error in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("{} of type Coil:Heating:Fuel \"{}\" not found.", cAlphaFields(15), Alphas(15)));
                    ErrorsFound = true;
                }

                // Get the Supplemental Heating Coil Node Numbers
                LocalError = false;
                SuppHeatCoilInletNode = HeatingCoils::GetCoilInletNode(state, Alphas(14), Alphas(15), LocalError);
                if (LocalError) {
                    ShowSevereError(state, format("The inlet node number of {} is not found \"{}\"", cAlphaFields(15), Alphas(15)));
                    ShowContinueError(state, format("...occurs in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                    LocalError = false;
                }
                SuppHeatCoilOutletNode = HeatingCoils::GetCoilOutletNode(state, Alphas(14), Alphas(15), LocalError);
                if (LocalError) {
                    ShowSevereError(state, format("The outlet node number of {} is not found \"{}\"", cAlphaFields(15), Alphas(15)));
                    ShowContinueError(state, format("...occurs in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                    LocalError = false;
                }

                // Get supplemental heating coil capacity to see if it is autosize
                thisMSHP.DesignSuppHeatingCapacity = HeatingCoils::GetCoilCapacity(state, Alphas(14), Alphas(15), LocalError);
                if (LocalError) {
                    ShowSevereError(state, format("The capacity {} is not found \"{}\"", cAlphaFields(15), Alphas(15)));
                    ShowContinueError(state, format("...occurs in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                    LocalError = false;
                }
                BranchNodeConnections::SetUpCompSets(state,
                                                     state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                     thisMSHP.Name,
                                                     "Coil:Heating:Fuel",
                                                     thisMSHP.SuppHeatCoilName,
                                                     "UNDEFINED",
                                                     "UNDEFINED");
            }
            if (Util::SameString(Alphas(14), "Coil:Heating:Electric")) {
                thisMSHP.SuppHeatCoilType = HVAC::Coil_HeatingElectric;
                errFlag = false;
                thisMSHP.SuppHeatCoilNum = HeatingCoils::GetHeatingCoilIndex(state, "Coil:Heating:Electric", Alphas(15), errFlag);
                if (thisMSHP.SuppHeatCoilNum <= 0 || errFlag) {
                    ShowContinueError(state, format("Configuration error in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    ShowContinueError(state, format("{} of type Coil:Heating:Electric \"{}\" not found.", cAlphaFields(15), Alphas(15)));
                    ErrorsFound = true;
                }

                // Get the Supplemental Heating Coil Node Numbers
                LocalError = false;
                SuppHeatCoilInletNode = HeatingCoils::GetCoilInletNode(state, Alphas(14), Alphas(15), LocalError);
                if (LocalError) {
                    ShowSevereError(state, format("The inlet node number of {} is not found \"{}\"", cAlphaFields(15), Alphas(15)));
                    ShowContinueError(state, format("...occurs in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                    LocalError = false;
                }
                SuppHeatCoilOutletNode = HeatingCoils::GetCoilOutletNode(state, Alphas(14), Alphas(15), LocalError);
                if (LocalError) {
                    ShowSevereError(state, format("The outlet node number of {} is not found \"{}\"", cAlphaFields(15), Alphas(15)));
                    ShowContinueError(state, format("...occurs in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                    LocalError = false;
                }

                // Get supplemental heating coil capacity to see if it is autosize
                thisMSHP.DesignSuppHeatingCapacity = HeatingCoils::GetCoilCapacity(state, Alphas(14), Alphas(15), LocalError);
                if (LocalError) {
                    ShowSevereError(state, format("The capacity {} is not found \"{}\"", cAlphaFields(15), Alphas(15)));
                    ShowContinueError(state, format("...occurs in {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                    LocalError = false;
                }

                BranchNodeConnections::SetUpCompSets(state,
                                                     state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                     thisMSHP.Name,
                                                     "Coil:Heating:Electric",
                                                     thisMSHP.SuppHeatCoilName,
                                                     "UNDEFINED",
                                                     "UNDEFINED");
            }

            if (Util::SameString(Alphas(14), "Coil:Heating:Water")) {
                thisMSHP.SuppHeatCoilType = HVAC::Coil_HeatingWater;
                ValidateComponent(state, Alphas(14), thisMSHP.SuppHeatCoilName, IsNotOK, state.dataHVACMultiSpdHP->CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1)));
                    ErrorsFound = true;
                } else { // mine data from heating coil object

                    // Get the Heating Coil water Inlet or control Node number
                    errFlag = false;
                    thisMSHP.SuppCoilControlNode = WaterCoils::GetCoilWaterInletNode(state, "Coil:Heating:Water", thisMSHP.SuppHeatCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                        ErrorsFound = true;
                    }

                    // Get the ReHeat Coil hot water max volume flow rate
                    errFlag = false;
                    thisMSHP.MaxSuppCoilFluidFlow =
                        WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", thisMSHP.SuppHeatCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                        ErrorsFound = true;
                    }

                    // Get the Supplemental Heating Coil Inlet Node
                    errFlag = false;
                    SuppHeatCoilInletNode = WaterCoils::GetCoilInletNode(state, "Coil:Heating:Water", thisMSHP.SuppHeatCoilName, errFlag);
                    thisMSHP.SuppCoilAirInletNode = SuppHeatCoilInletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                        ErrorsFound = true;
                    }

                    // Get the Supplemental Heating Coil Outlet Node
                    errFlag = false;
                    SuppHeatCoilOutletNode = WaterCoils::GetCoilOutletNode(state, "Coil:Heating:Water", thisMSHP.SuppHeatCoilName, errFlag);
                    thisMSHP.SuppCoilAirOutletNode = SuppHeatCoilOutletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                        ErrorsFound = true;
                    }
                    BranchNodeConnections::SetUpCompSets(state,
                                                         state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                         thisMSHP.Name,
                                                         "Coil:Heating:Water",
                                                         thisMSHP.SuppHeatCoilName,
                                                         state.dataLoopNodes->NodeID(SuppHeatCoilInletNode),
                                                         state.dataLoopNodes->NodeID(SuppHeatCoilOutletNode));
                }
            }
            if (Util::SameString(Alphas(14), "Coil:Heating:Steam")) {
                thisMSHP.SuppHeatCoilType = HVAC::Coil_HeatingSteam;
                ValidateComponent(state, Alphas(14), thisMSHP.SuppHeatCoilName, IsNotOK, state.dataHVACMultiSpdHP->CurrentModuleObject);
                if (IsNotOK) {
                    ShowContinueError(state, format("...occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ErrorsFound = true;
                } else { // mine data from heating coil object

                    errFlag = false;
                    thisMSHP.SuppHeatCoilNum = SteamCoils::GetSteamCoilIndex(state, Alphas(14), thisMSHP.SuppHeatCoilName, errFlag);
                    if (thisMSHP.SuppHeatCoilNum == 0) {
                        ShowSevereError(
                            state,
                            format("{} illegal {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, cAlphaFields(14), thisMSHP.SuppHeatCoilName));
                        ShowContinueError(state, format("Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                        ErrorsFound = true;
                    }

                    // Get the Supplemental Heating Coil steam inlet node number
                    errFlag = false;
                    thisMSHP.SuppCoilControlNode = SteamCoils::GetCoilAirOutletNode(state, "Coil:Heating:Steam", thisMSHP.SuppHeatCoilName, errFlag);
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                        ErrorsFound = true;
                    }

                    // Get the Supplemental Heating Coil steam max volume flow rate
                    thisMSHP.MaxSuppCoilFluidFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, thisMSHP.SuppHeatCoilNum, errFlag);
                    if (thisMSHP.MaxSuppCoilFluidFlow > 0.0) {
                        SteamDensity = Fluid::GetSteam(state)->getSatDensity(state, state.dataHVACMultiSpdHP->TempSteamIn, 1.0, routineName);
                        thisMSHP.MaxSuppCoilFluidFlow *= SteamDensity;
                    }

                    // Get the Supplemental Heating Coil Inlet Node
                    errFlag = false;
                    SuppHeatCoilInletNode = SteamCoils::GetCoilAirInletNode(state, thisMSHP.SuppHeatCoilNum, thisMSHP.SuppHeatCoilName, errFlag);
                    thisMSHP.SuppCoilAirInletNode = SuppHeatCoilInletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                        ErrorsFound = true;
                    }

                    // Get the Supplemental Heating Coil Outlet Node
                    errFlag = false;
                    SuppHeatCoilOutletNode = SteamCoils::GetCoilAirOutletNode(state, thisMSHP.SuppHeatCoilNum, thisMSHP.SuppHeatCoilName, errFlag);
                    thisMSHP.SuppCoilAirOutletNode = SuppHeatCoilOutletNode;
                    if (errFlag) {
                        ShowContinueError(state, format("Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                        ErrorsFound = true;
                    }

                    BranchNodeConnections::SetUpCompSets(state,
                                                         state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                         thisMSHP.Name,
                                                         "Coil:Heating:Steam",
                                                         thisMSHP.SuppHeatCoilName,
                                                         state.dataLoopNodes->NodeID(SuppHeatCoilInletNode),
                                                         state.dataLoopNodes->NodeID(SuppHeatCoilOutletNode));
                }
            }

            if (thisMSHP.SuppHeatCoilType == 0) {
                ShowSevereError(state,
                                format("{}, \"{}\", {} is not allowed = {}",
                                       state.dataHVACMultiSpdHP->CurrentModuleObject,
                                       thisMSHP.Name,
                                       cAlphaFields(14),
                                       Alphas(14)));
                ShowContinueError(state, "Valid choices are Coil:Heating:Fuel,Coil:Heating:Electric,Coil:Heating:Steam,or Coil:Heating:Water");
                ErrorsFound = true;
            }

            thisMSHP.HeatingSizingRatio = Numbers(1);
            if (thisMSHP.HeatCoilType == HVAC::CoilDX_MultiSpeedHeating) {
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchDXHeatCoilSizingRatio, thisMSHP.DXHeatCoilName, thisMSHP.HeatingSizingRatio);
            }
            thisMSHP.SuppMaxAirTemp = Numbers(2);
            thisMSHP.SuppMaxOATemp = Numbers(3);
            if (thisMSHP.SuppMaxOATemp > 21.0) {
                ShowSevereError(
                    state,
                    format("{}, \"{}\", {} is greater than 21.0", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name, cNumericFields(3)));
                ShowContinueError(state, format("The input value is {:.2R}", Numbers(3)));
                ErrorsFound = true;
            }
            OutputReportPredefined::PreDefTableEntry(
                state, state.dataOutRptPredefined->pdchDXHeatCoilSuppHiT, thisMSHP.DXHeatCoilName, thisMSHP.SuppMaxOATemp);

            thisMSHP.AuxOnCyclePower = Numbers(4);
            thisMSHP.AuxOffCyclePower = Numbers(5);
            if (thisMSHP.AuxOnCyclePower < 0.0) {
                ShowSevereError(state,
                                format("{}, \"{}\", A negative value for {} is not allowed ",
                                       state.dataHVACMultiSpdHP->CurrentModuleObject,
                                       thisMSHP.Name,
                                       cNumericFields(4)));
                ErrorsFound = true;
            }
            if (thisMSHP.AuxOffCyclePower < 0.0) {
                ShowSevereError(state,
                                format("{}, \"{}\", A negative value for {} is not allowed ",
                                       state.dataHVACMultiSpdHP->CurrentModuleObject,
                                       thisMSHP.Name,
                                       cNumericFields(5)));
                ErrorsFound = true;
            }

            // Heat recovery
            thisMSHP.DesignHeatRecFlowRate = Numbers(6);
            if (thisMSHP.DesignHeatRecFlowRate > 0.0) {
                thisMSHP.HeatRecActive = true;
                thisMSHP.DesignHeatRecMassFlowRate = Psychrometrics::RhoH2O(Constant::HWInitConvTemp) * thisMSHP.DesignHeatRecFlowRate;
                thisMSHP.HeatRecInletNodeNum = GetOnlySingleNode(state,
                                                                 Alphas(16),
                                                                 ErrorsFound,
                                                                 DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatPumpAirToAirMultiSpeed,
                                                                 Alphas(1),
                                                                 DataLoopNode::NodeFluidType::Water,
                                                                 DataLoopNode::ConnectionType::Inlet,
                                                                 NodeInputManager::CompFluidStream::Tertiary,
                                                                 DataLoopNode::ObjectIsNotParent);
                if (thisMSHP.HeatRecInletNodeNum == 0) {
                    ShowSevereError(
                        state, format("{}, \"{}\", Missing {}.", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name, cAlphaFields(16)));
                    ErrorsFound = true;
                }
                thisMSHP.HeatRecOutletNodeNum = GetOnlySingleNode(state,
                                                                  Alphas(17),
                                                                  ErrorsFound,
                                                                  DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatPumpAirToAirMultiSpeed,
                                                                  Alphas(1),
                                                                  DataLoopNode::NodeFluidType::Water,
                                                                  DataLoopNode::ConnectionType::Outlet,
                                                                  NodeInputManager::CompFluidStream::Tertiary,
                                                                  DataLoopNode::ObjectIsNotParent);
                if (thisMSHP.HeatRecOutletNodeNum == 0) {
                    ShowSevereError(
                        state, format("{}, \"{}\", Missing {}.", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name, cAlphaFields(17)));
                    ErrorsFound = true;
                }
                BranchNodeConnections::TestCompSet(
                    state, state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1), Alphas(16), Alphas(17), "MSHP Heat Recovery Nodes");
                DXCoils::SetMSHPDXCoilHeatRecoveryFlag(state, thisMSHP.DXCoolCoilIndex);
                if (thisMSHP.DXHeatCoilIndex > 0) {
                    DXCoils::SetMSHPDXCoilHeatRecoveryFlag(state, thisMSHP.DXHeatCoilIndex);
                }
            } else {
                thisMSHP.HeatRecActive = false;
                thisMSHP.DesignHeatRecMassFlowRate = 0.0;
                thisMSHP.HeatRecInletNodeNum = 0;
                thisMSHP.HeatRecOutletNodeNum = 0;
                if (!lAlphaBlanks(16) || !lAlphaBlanks(17)) {
                    ShowWarningError(state,
                                     format("Since {} = 0.0, heat recovery is inactive for {} = {}",
                                            cNumericFields(6),
                                            state.dataHVACMultiSpdHP->CurrentModuleObject,
                                            Alphas(1)));
                    ShowContinueError(state, format("However, {} or {} was specified.", cAlphaFields(16), cAlphaFields(17)));
                }
            }
            thisMSHP.MaxHeatRecOutletTemp = Numbers(7);
            if (thisMSHP.MaxHeatRecOutletTemp < 0.0) {
                ShowSevereError(state,
                                format("{}, \"{}\", The value for {} is below 0.0",
                                       state.dataHVACMultiSpdHP->CurrentModuleObject,
                                       thisMSHP.Name,
                                       cNumericFields(7)));
                ErrorsFound = true;
            }
            if (thisMSHP.MaxHeatRecOutletTemp > 100.0) {
                ShowSevereError(state,
                                format("{}, \"{}\", The value for {} is above 100.0",
                                       state.dataHVACMultiSpdHP->CurrentModuleObject,
                                       thisMSHP.Name,
                                       cNumericFields(7)));
                ErrorsFound = true;
            }

            thisMSHP.IdleVolumeAirRate = Numbers(8);
            if (thisMSHP.IdleVolumeAirRate < 0.0 && thisMSHP.IdleVolumeAirRate != DataSizing::AutoSize) {
                ShowSevereError(
                    state,
                    format(
                        "{}, \"{}\", {} cannot be less than zero.", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name, cNumericFields(8)));
                ErrorsFound = true;
            }

            //     AirFlowControl only valid if fan opmode = FanOp::Continuous
            if (thisMSHP.IdleVolumeAirRate == 0.0) {
                thisMSHP.AirFlowControl = AirflowControl::UseCompressorOnFlow;
            } else {
                thisMSHP.AirFlowControl = AirflowControl::UseCompressorOffFlow;
            }

            //   Initialize last mode of compressor operation
            thisMSHP.LastMode = ModeOfOperation::HeatingMode;

            thisMSHP.NumOfSpeedHeating = Numbers(9);
            if (thisMSHP.NumOfSpeedHeating < 2 || thisMSHP.NumOfSpeedHeating > 4) {
                if (thisMSHP.HeatCoilType == HVAC::CoilDX_MultiSpeedHeating) {
                    ShowSevereError(state,
                                    format("{}, The maximum {} is 4, and the minimum number is 2",
                                           state.dataHVACMultiSpdHP->CurrentModuleObject,
                                           cNumericFields(9)));
                    ShowContinueError(state, format("The input value is {:.0R}", Numbers(9)));
                    ErrorsFound = true;
                }
            }
            thisMSHP.NumOfSpeedCooling = Numbers(10);
            if (thisMSHP.NumOfSpeedCooling < 2 || thisMSHP.NumOfSpeedCooling > 4) {
                ShowSevereError(state,
                                format("{}, The maximum {} is 4, and the minimum number is 2",
                                       state.dataHVACMultiSpdHP->CurrentModuleObject,
                                       cNumericFields(10)));
                ShowContinueError(state, format("The input value is {:.0R}", Numbers(10)));
                ErrorsFound = true;
            }

            // Generate a dynamic array for heating
            if (thisMSHP.NumOfSpeedHeating > 0) {
                thisMSHP.HeatMassFlowRate.allocate(thisMSHP.NumOfSpeedHeating);
                thisMSHP.HeatVolumeFlowRate.allocate(thisMSHP.NumOfSpeedHeating);
                thisMSHP.HeatingSpeedRatio.allocate(thisMSHP.NumOfSpeedHeating);
                thisMSHP.HeatingSpeedRatio = 1.0;
                for (i = 1; i <= thisMSHP.NumOfSpeedHeating; ++i) {
                    thisMSHP.HeatVolumeFlowRate(i) = Numbers(10 + i);
                    if (thisMSHP.HeatCoilType == HVAC::CoilDX_MultiSpeedHeating) {
                        if (thisMSHP.HeatVolumeFlowRate(i) <= 0.0 && thisMSHP.HeatVolumeFlowRate(i) != DataSizing::AutoSize) {
                            ShowSevereError(state,
                                            format("{}, \"{}\", {} must be greater than zero.",
                                                   state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                   thisMSHP.Name,
                                                   cNumericFields(10 + i)));
                            ErrorsFound = true;
                        }
                    }
                }
                // Ensure flow rate at high speed should be greater or equal to the flow rate at low speed
                for (i = 2; i <= thisMSHP.NumOfSpeedHeating; ++i) {
                    if (thisMSHP.HeatVolumeFlowRate(i) == DataSizing::AutoSize) {
                        continue;
                    }
                    Found = false;
                    for (j = i - 1; j >= 1; --j) {
                        if (thisMSHP.HeatVolumeFlowRate(i) != DataSizing::AutoSize) {
                            Found = true;
                            break;
                        }
                    }
                    if (Found) {
                        if (thisMSHP.HeatVolumeFlowRate(i) < thisMSHP.HeatVolumeFlowRate(j)) {
                            ShowSevereError(
                                state,
                                format("{}, \"{}\", {}", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name, cNumericFields(10 + i)));
                            ShowContinueError(state, format(" cannot be less than {}", cNumericFields(10 + j)));
                            ErrorsFound = true;
                        }
                    }
                }
            }

            if (state.dataGlobal->DoCoilDirectSolutions) {
                int MaxNumber = std::max(thisMSHP.NumOfSpeedCooling, thisMSHP.NumOfSpeedHeating);
                thisMSHP.FullOutput.allocate(MaxNumber);
                DXCoils::DisableLatentDegradation(state, thisMSHP.DXCoolCoilIndex);
            }
            // Generate a dynamic array for cooling
            if (thisMSHP.NumOfSpeedCooling > 0) {
                thisMSHP.CoolMassFlowRate.allocate(thisMSHP.NumOfSpeedCooling);
                thisMSHP.CoolVolumeFlowRate.allocate(thisMSHP.NumOfSpeedCooling);
                thisMSHP.CoolingSpeedRatio.allocate(thisMSHP.NumOfSpeedCooling);
                thisMSHP.CoolingSpeedRatio = 1.0;
                for (i = 1; i <= thisMSHP.NumOfSpeedCooling; ++i) {
                    thisMSHP.CoolVolumeFlowRate(i) = Numbers(14 + i);
                    if (thisMSHP.CoolVolumeFlowRate(i) <= 0.0 && thisMSHP.CoolVolumeFlowRate(i) != DataSizing::AutoSize) {
                        ShowSevereError(state,
                                        format("{}, \"{}\", {} must be greater than zero.",
                                               state.dataHVACMultiSpdHP->CurrentModuleObject,
                                               thisMSHP.Name,
                                               cNumericFields(14 + i)));
                        ErrorsFound = true;
                    }
                }
                // Ensure flow rate at high speed should be greater or equal to the flow rate at low speed
                for (i = 2; i <= thisMSHP.NumOfSpeedCooling; ++i) {
                    if (thisMSHP.CoolVolumeFlowRate(i) == DataSizing::AutoSize) {
                        continue;
                    }
                    Found = false;
                    for (j = i - 1; j >= 1; --j) {
                        if (thisMSHP.CoolVolumeFlowRate(i) != DataSizing::AutoSize) {
                            Found = true;
                            break;
                        }
                    }
                    if (Found) {
                        if (thisMSHP.CoolVolumeFlowRate(i) < thisMSHP.CoolVolumeFlowRate(j)) {
                            ShowSevereError(
                                state,
                                format("{}, \"{}\", {}", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name, cNumericFields(14 + i)));
                            ShowContinueError(state, format(" cannot be less than {}", cNumericFields(14 + j)));
                            ErrorsFound = true;
                        }
                    }
                }
            }

            // Check node integrity
            if (thisMSHP.fanPlace == HVAC::FanPlace::BlowThru) {
                if (thisMSHP.FanInletNode != thisMSHP.AirInletNodeNum) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(
                        state, format("When a blow through fan is specified, the fan inlet node name must be the same as the {}", cAlphaFields(3)));
                    ShowContinueError(state, format("...Fan inlet node name           = {}", state.dataLoopNodes->NodeID(thisMSHP.FanInletNode)));
                    ShowContinueError(state, format("...{} = {}", cAlphaFields(3), state.dataLoopNodes->NodeID(thisMSHP.AirInletNodeNum)));
                    ErrorsFound = true;
                }
                if (thisMSHP.FanOutletNode != CoolingCoilInletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(
                        state,
                        "When a blow through fan is specified, the fan outlet node name must be the same as the cooling coil inlet node name.");
                    ShowContinueError(state, format("...Fan outlet node name         = {}", state.dataLoopNodes->NodeID(thisMSHP.FanOutletNode)));
                    ShowContinueError(state, format("...Cooling coil inlet node name = {}", state.dataLoopNodes->NodeID(CoolingCoilInletNode)));
                    ErrorsFound = true;
                }
                if (CoolingCoilOutletNode != HeatingCoilInletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(state, "The cooling coil outlet node name must be the same as the heating coil inlet node name.");
                    ShowContinueError(state, format("...Cooling coil outlet node name = {}", state.dataLoopNodes->NodeID(CoolingCoilOutletNode)));
                    ShowContinueError(state, format("...Heating coil inlet node name  = {}", state.dataLoopNodes->NodeID(HeatingCoilInletNode)));
                    ErrorsFound = true;
                }
                if (HeatingCoilOutletNode != SuppHeatCoilInletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(state,
                                      "When a blow through fan is specified, the heating coil outlet node name must be the same as the reheat coil "
                                      "inlet node name.");
                    ShowContinueError(state, format("...Heating coil outlet node name = {}", state.dataLoopNodes->NodeID(HeatingCoilOutletNode)));
                    ShowContinueError(state, format("...Reheat coil inlet node name   = {}", state.dataLoopNodes->NodeID(SuppHeatCoilInletNode)));
                    ErrorsFound = true;
                }
                if (SuppHeatCoilOutletNode != thisMSHP.AirOutletNodeNum) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(state, format("The supplemental heating coil outlet node name must be the same as the {}", cAlphaFields(4)));
                    ShowContinueError(
                        state, format("...Supplemental heating coil outlet node name   = {}", state.dataLoopNodes->NodeID(SuppHeatCoilOutletNode)));
                    ShowContinueError(state, format("...{} = {}", cAlphaFields(4), state.dataLoopNodes->NodeID(thisMSHP.AirOutletNodeNum)));
                    ErrorsFound = true;
                }
            } else {
                if (CoolingCoilInletNode != thisMSHP.AirInletNodeNum) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(
                        state,
                        format("When a draw through fan is specified, the cooling coil inlet node name must be the same as the {}", cAlphaFields(3)));
                    ShowContinueError(state, format("...Cooling coil inlet node name  = {}", state.dataLoopNodes->NodeID(CoolingCoilInletNode)));
                    ShowContinueError(state, format("...{} = {}", cAlphaFields(3), state.dataLoopNodes->NodeID(thisMSHP.AirInletNodeNum)));
                    ErrorsFound = true;
                }
                if (CoolingCoilOutletNode != HeatingCoilInletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(state, "The cooling coil outlet node name must be the same as the heating coil inlet node name.");
                    ShowContinueError(state, format("...Cooling coil outlet node name = {}", state.dataLoopNodes->NodeID(CoolingCoilOutletNode)));
                    ShowContinueError(state, format("...Heating coil inlet node name  = {}", state.dataLoopNodes->NodeID(HeatingCoilInletNode)));
                    ErrorsFound = true;
                }
                if (HeatingCoilOutletNode != thisMSHP.FanInletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(
                        state,
                        "When a draw through fan is specified, the heating coil outlet node name must be the same as the fan inlet node name.");
                    ShowContinueError(state, format("...Heating coil outlet node name = {}", state.dataLoopNodes->NodeID(HeatingCoilOutletNode)));
                    ShowContinueError(state, format("...Fan inlet node name           = {}", state.dataLoopNodes->NodeID(thisMSHP.FanInletNode)));
                    ErrorsFound = true;
                }
                if (thisMSHP.FanOutletNode != SuppHeatCoilInletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(
                        state, "When a draw through fan is specified, the fan outlet node name must be the same as the reheat coil inlet node name.");
                    ShowContinueError(state, format("...Fan outlet node name        = {}", state.dataLoopNodes->NodeID(thisMSHP.FanOutletNode)));
                    ShowContinueError(state, format("...Reheat coil inlet node name = {}", state.dataLoopNodes->NodeID(SuppHeatCoilInletNode)));
                    ErrorsFound = true;
                }
                if (SuppHeatCoilOutletNode != thisMSHP.AirOutletNodeNum) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(state, format("The reheat coil outlet node name must be the same as the {}", cAlphaFields(4)));
                    ShowContinueError(state, format("...Reheat coil outlet node name   = {}", state.dataLoopNodes->NodeID(SuppHeatCoilOutletNode)));
                    ShowContinueError(state, format("...{} = {}", cAlphaFields(4), state.dataLoopNodes->NodeID(thisMSHP.AirOutletNodeNum)));
                    ErrorsFound = true;
                }
            }

            // Ensure the numbers of speeds defined in the parent object are equal to the numbers defined in coil objects
            if (thisMSHP.HeatCoilType == HVAC::CoilDX_MultiSpeedHeating) {
                i = DXCoils::GetDXCoilNumberOfSpeeds(state, Alphas(10), Alphas(11), ErrorsFound);
                if (thisMSHP.NumOfSpeedHeating != i) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(
                        state, format("The {} is not equal to the number defined in {} = {}", cNumericFields(9), cAlphaFields(11), Alphas(11)));
                    ErrorsFound = true;
                }
            } else if (thisMSHP.HeatCoilType == HVAC::Coil_HeatingElectric_MultiStage || thisMSHP.HeatCoilType == HVAC::Coil_HeatingGas_MultiStage) {
                i = HeatingCoils::GetHeatingCoilNumberOfStages(state, Alphas(10), Alphas(11), ErrorsFound);
                if (thisMSHP.NumOfSpeedHeating != i) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(
                        state, format("The {} is not equal to the number defined in {} = {}", cNumericFields(9), cAlphaFields(11), Alphas(11)));
                    ErrorsFound = true;
                }
            }
            i = DXCoils::GetDXCoilNumberOfSpeeds(state, Alphas(12), Alphas(13), ErrorsFound);
            if (thisMSHP.NumOfSpeedCooling != i) {
                ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                ShowContinueError(state,
                                  format("The {} is not equal to the number defined in {} = {}", cNumericFields(10), cAlphaFields(13), Alphas(13)));
                ErrorsFound = true;
            }
        }

        if (ErrorsFound) {
            ShowFatalError(state,
                           format("{}Errors found in getting {} input.  Preceding condition(s) causes termination.",
                                  RoutineName,
                                  state.dataHVACMultiSpdHP->CurrentModuleObject));
        }
        // End of multispeed heat pump

        int MSHPNum = 0;
        for (auto &thisMSHeatPump : state.dataHVACMultiSpdHP->MSHeatPump) {
            auto &thisMSHPReport = state.dataHVACMultiSpdHP->MSHeatPumpReport(++MSHPNum);
            // Setup Report Variables for MSHP Equipment
            SetupOutputVariable(state,
                                "Unitary System Ancillary Electricity Rate",
                                Constant::Units::W,
                                thisMSHeatPump.AuxElecPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisMSHeatPump.Name);
            SetupOutputVariable(state,
                                "Unitary System Cooling Ancillary Electricity Energy",
                                Constant::Units::J,
                                thisMSHPReport.AuxElecCoolConsumption,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisMSHeatPump.Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Cooling);
            SetupOutputVariable(state,
                                "Unitary System Heating Ancillary Electricity Energy",
                                Constant::Units::J,
                                thisMSHPReport.AuxElecHeatConsumption,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisMSHeatPump.Name,
                                Constant::eResource::Electricity,
                                OutputProcessor::Group::HVAC,
                                OutputProcessor::EndUseCat::Heating);
            SetupOutputVariable(state,
                                "Unitary System Fan Part Load Ratio",
                                Constant::Units::None,
                                thisMSHeatPump.FanPartLoadRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisMSHeatPump.Name);
            SetupOutputVariable(state,
                                "Unitary System Compressor Part Load Ratio",
                                Constant::Units::None,
                                thisMSHeatPump.CompPartLoadRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisMSHeatPump.Name);
            SetupOutputVariable(state,
                                "Unitary System Electricity Rate",
                                Constant::Units::W,
                                thisMSHeatPump.ElecPower,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisMSHeatPump.Name);
            SetupOutputVariable(state,
                                "Unitary System Electricity Energy",
                                Constant::Units::J,
                                thisMSHPReport.ElecPowerConsumption,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Sum,
                                thisMSHeatPump.Name);
            SetupOutputVariable(state,
                                "Unitary System DX Coil Cycling Ratio",
                                Constant::Units::None,
                                thisMSHPReport.CycRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisMSHeatPump.Name);
            SetupOutputVariable(state,
                                "Unitary System DX Coil Speed Ratio",
                                Constant::Units::None,
                                thisMSHPReport.SpeedRatio,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisMSHeatPump.Name);
            SetupOutputVariable(state,
                                "Unitary System DX Coil Speed Level",
                                Constant::Units::None,
                                thisMSHPReport.SpeedNum,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisMSHeatPump.Name);
            SetupOutputVariable(state,
                                "Unitary System Total Cooling Rate",
                                Constant::Units::W,
                                thisMSHeatPump.TotCoolEnergyRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisMSHeatPump.Name);
            SetupOutputVariable(state,
                                "Unitary System Total Heating Rate",
                                Constant::Units::W,
                                thisMSHeatPump.TotHeatEnergyRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisMSHeatPump.Name);
            SetupOutputVariable(state,
                                "Unitary System Sensible Cooling Rate",
                                Constant::Units::W,
                                thisMSHeatPump.SensCoolEnergyRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisMSHeatPump.Name);
            SetupOutputVariable(state,
                                "Unitary System Sensible Heating Rate",
                                Constant::Units::W,
                                thisMSHeatPump.SensHeatEnergyRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisMSHeatPump.Name);
            SetupOutputVariable(state,
                                "Unitary System Latent Cooling Rate",
                                Constant::Units::W,
                                thisMSHeatPump.LatCoolEnergyRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisMSHeatPump.Name);
            SetupOutputVariable(state,
                                "Unitary System Latent Heating Rate",
                                Constant::Units::W,
                                thisMSHeatPump.LatHeatEnergyRate,
                                OutputProcessor::TimeStepType::System,
                                OutputProcessor::StoreType::Average,
                                thisMSHeatPump.Name);
            if (thisMSHeatPump.HeatRecActive) {
                SetupOutputVariable(state,
                                    "Unitary System Heat Recovery Rate",
                                    Constant::Units::W,
                                    thisMSHeatPump.HeatRecoveryRate,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    thisMSHeatPump.Name);
                SetupOutputVariable(state,
                                    "Unitary System Heat Recovery Inlet Temperature",
                                    Constant::Units::C,
                                    thisMSHeatPump.HeatRecoveryInletTemp,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    thisMSHeatPump.Name);
                SetupOutputVariable(state,
                                    "Unitary System Heat Recovery Outlet Temperature",
                                    Constant::Units::C,
                                    thisMSHeatPump.HeatRecoveryOutletTemp,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    thisMSHeatPump.Name);
                SetupOutputVariable(state,
                                    "Unitary System Heat Recovery Fluid Mass Flow Rate",
                                    Constant::Units::kg_s,
                                    thisMSHeatPump.HeatRecoveryMassFlowRate,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Average,
                                    thisMSHeatPump.Name);
                SetupOutputVariable(state,
                                    "Unitary System Heat Recovery Energy",
                                    Constant::Units::J,
                                    thisMSHPReport.HeatRecoveryEnergy,
                                    OutputProcessor::TimeStepType::System,
                                    OutputProcessor::StoreType::Sum,
                                    thisMSHeatPump.Name);
            }
        }
        if (state.dataGlobal->AnyEnergyManagementSystemInModel) {
            for (auto &thisCoil : state.dataHVACMultiSpdHP->MSHeatPump) {
                SetupEMSActuator(state,
                                 "Coil Speed Control",
                                 thisCoil.Name,
                                 "Unitary System DX Coil Speed Value",
                                 "[]",
                                 thisCoil.EMSOverrideCoilSpeedNumOn,
                                 thisCoil.EMSOverrideCoilSpeedNumValue);
            }
        }
    }

    //******************************************************************************

    void InitMSHeatPump(EnergyPlusData &state,
                        int const MSHeatPumpNum,       // Engine driven heat pump number
                        bool const FirstHVACIteration, // TRUE if first HVAC iteration
                        int const AirLoopNum,          // air loop index
                        Real64 &QZnReq,                // Heating/Cooling load for all served zones
                        Real64 &OnOffAirFlowRatio      // Ratio of compressor ON airflow to average airflow over timestep
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Lixing Gu, FSEC
        //       DATE WRITTEN:    July 2007
        //       MODIFIED         Bereket Nigusse, June 2010 - added a procedure to calculate supply air flow fraction
        //                        through controlled zone

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for initializations of the multispeed heat pump (MSHP) components.

        // METHODOLOGY EMPLOYED:
        // Uses the status flags to trigger initializations. The MSHP system is simulated with no load (coils off) to
        // determine the outlet temperature. A setpoint temperature is calculated on FirstHVACIteration = TRUE.
        // Once the setpoint is calculated, the inlet mass flow rate on FirstHVACIteration = FALSE is used to
        // determine the bypass fraction. The simulation converges quickly on mass flow rate. If the zone
        // temperatures float in the deadband, additional iterations are required to converge on mass flow rate.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        static constexpr std::string_view RoutineName("InitMSHeatPump");
        Real64 RhoAir; // Air density at InNode

        Real64 QSensUnitOut; // Output of MSHP system with coils off
        Real64 PartLoadFrac; // Part-load ratio
        int ZoneNum;
        int i;                // Index to speed
        Real64 DeltaMassRate; // Difference of mass flow rate between inlet node and system outlet node

        int NumAirLoopZones(0);                           // number of zone inlet nodes in an air loop
        Real64 SumOfMassFlowRateMax(0.0);                 // the sum of mass flow rates at inlet to zones in an airloop
        Real64 CntrlZoneTerminalUnitMassFlowRateMax(0.0); // Maximum mass flow rate through controlled zone terminal unit
        Real64 rho;                                       // local fluid density
        Real64 MdotHR;                                    // local temporary for heat recovery fluid mass flow rate (kg/s)
        Real64 ZoneLoadToCoolSPSequenced;
        Real64 ZoneLoadToHeatSPSequenced;

        bool ErrorsFound(false);        // flag returned from mining call
        Real64 mdot(0.0);               // local temporary for mass flow rate (kg/s)
        Real64 SteamDensity(0.0);       // density of steam at 100C, used for steam heating coils
        Real64 CoilMaxVolFlowRate(0.0); // coil fluid maximum volume flow rate
        Real64 QActual(0.0);            // coil actual capacity

        auto &MSHeatPump = state.dataHVACMultiSpdHP->MSHeatPump;

        int InNode = MSHeatPump(MSHeatPumpNum).AirInletNodeNum;
        int OutNode = MSHeatPump(MSHeatPumpNum).AirOutletNodeNum;
        int NumOfSpeedCooling = MSHeatPump(MSHeatPumpNum).NumOfSpeedCooling;
        int NumOfSpeedHeating = MSHeatPump(MSHeatPumpNum).NumOfSpeedHeating;

        ++state.dataHVACMultiSpdHP->AirLoopPass;
        if (state.dataHVACMultiSpdHP->AirLoopPass > 2) {
            state.dataHVACMultiSpdHP->AirLoopPass = 1;
        }

        if (MSHeatPump(MSHeatPumpNum).MyPlantScantFlag && allocated(state.dataPlnt->PlantLoop)) {
            bool errFlag;
            if (MSHeatPump(MSHeatPumpNum).HeatRecActive) {
                errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(state,
                                                        MSHeatPump(MSHeatPumpNum).Name,
                                                        DataPlant::PlantEquipmentType::MultiSpeedHeatPumpRecovery,
                                                        MSHeatPump(MSHeatPumpNum).HRPlantLoc,
                                                        errFlag,
                                                        _,
                                                        _,
                                                        _,
                                                        _,
                                                        _);
                if (errFlag) {
                    ShowFatalError(state, "InitMSHeatPump: Program terminated for previous conditions.");
                }

                MSHeatPump(MSHeatPumpNum).MyPlantScantFlag = false;
            } else {
                MSHeatPump(MSHeatPumpNum).MyPlantScantFlag = false;
            }
            if (MSHeatPump(MSHeatPumpNum).HeatCoilType == HVAC::Coil_HeatingWater) {
                errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(state,
                                                        MSHeatPump(MSHeatPumpNum).HeatCoilName,
                                                        DataPlant::PlantEquipmentType::CoilWaterSimpleHeating,
                                                        MSHeatPump(MSHeatPumpNum).plantLoc,
                                                        errFlag,
                                                        _,
                                                        _,
                                                        _,
                                                        _,
                                                        _);
                if (errFlag) {
                    ShowFatalError(state, "InitMSHeatPump: Program terminated for previous conditions.");
                }
                MSHeatPump(MSHeatPumpNum).MaxCoilFluidFlow =
                    WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", MSHeatPump(MSHeatPumpNum).HeatCoilName, ErrorsFound);

                if (MSHeatPump(MSHeatPumpNum).MaxCoilFluidFlow > 0.0) {
                    rho = state.dataPlnt->PlantLoop(MSHeatPump(MSHeatPumpNum).plantLoc.loopNum)
                              .glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                    MSHeatPump(MSHeatPumpNum).MaxCoilFluidFlow =
                        WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", MSHeatPump(MSHeatPumpNum).HeatCoilName, ErrorsFound) * rho;
                }
                // fill outlet node for coil
                MSHeatPump(MSHeatPumpNum).CoilOutletNode =
                    DataPlant::CompData::getPlantComponent(state, MSHeatPump(MSHeatPumpNum).plantLoc).NodeNumOut;
                MSHeatPump(MSHeatPumpNum).MyPlantScantFlag = false;

            } else if (MSHeatPump(MSHeatPumpNum).HeatCoilType == HVAC::Coil_HeatingSteam) {
                errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(state,
                                                        MSHeatPump(MSHeatPumpNum).HeatCoilName,
                                                        DataPlant::PlantEquipmentType::CoilSteamAirHeating,
                                                        MSHeatPump(MSHeatPumpNum).plantLoc,
                                                        errFlag,
                                                        _,
                                                        _,
                                                        _,
                                                        _,
                                                        _);
                if (errFlag) {
                    ShowFatalError(state, "InitMSHeatPump: Program terminated for previous conditions.");
                }
                MSHeatPump(MSHeatPumpNum).MaxCoilFluidFlow =
                    SteamCoils::GetCoilMaxSteamFlowRate(state, MSHeatPump(MSHeatPumpNum).HeatCoilNum, ErrorsFound);
                if (MSHeatPump(MSHeatPumpNum).MaxCoilFluidFlow > 0.0) {
                    SteamDensity = Fluid::GetSteam(state)->getSatDensity(state, state.dataHVACMultiSpdHP->TempSteamIn, 1.0, RoutineName);
                    MSHeatPump(MSHeatPumpNum).MaxCoilFluidFlow *= SteamDensity;
                }

                // fill outlet node for coil
                MSHeatPump(MSHeatPumpNum).CoilOutletNode =
                    DataPlant::CompData::getPlantComponent(state, MSHeatPump(MSHeatPumpNum).plantLoc).NodeNumOut;
                MSHeatPump(MSHeatPumpNum).MyPlantScantFlag = false;
            }
            if (MSHeatPump(MSHeatPumpNum).SuppHeatCoilType == HVAC::Coil_HeatingWater) {
                errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(state,
                                                        MSHeatPump(MSHeatPumpNum).SuppHeatCoilName,
                                                        DataPlant::PlantEquipmentType::CoilWaterSimpleHeating,
                                                        MSHeatPump(MSHeatPumpNum).SuppPlantLoc,
                                                        errFlag,
                                                        _,
                                                        _,
                                                        _,
                                                        _,
                                                        _);
                if (errFlag) {
                    ShowFatalError(state, "InitMSHeatPump: Program terminated for previous conditions.");
                }
                MSHeatPump(MSHeatPumpNum).MaxSuppCoilFluidFlow =
                    WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", MSHeatPump(MSHeatPumpNum).SuppHeatCoilName, ErrorsFound);

                if (MSHeatPump(MSHeatPumpNum).MaxSuppCoilFluidFlow > 0.0) {
                    rho = state.dataPlnt->PlantLoop(MSHeatPump(MSHeatPumpNum).SuppPlantLoc.loopNum)
                              .glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                    MSHeatPump(MSHeatPumpNum).MaxSuppCoilFluidFlow =
                        WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", MSHeatPump(MSHeatPumpNum).SuppHeatCoilName, ErrorsFound) *
                        rho;
                }
                // fill outlet node for coil
                MSHeatPump(MSHeatPumpNum).SuppCoilOutletNode =
                    DataPlant::CompData::getPlantComponent(state, MSHeatPump(MSHeatPumpNum).SuppPlantLoc).NodeNumOut;
                MSHeatPump(MSHeatPumpNum).MyPlantScantFlag = false;

            } else if (MSHeatPump(MSHeatPumpNum).SuppHeatCoilType == HVAC::Coil_HeatingSteam) {
                errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(state,
                                                        MSHeatPump(MSHeatPumpNum).SuppHeatCoilName,
                                                        DataPlant::PlantEquipmentType::CoilSteamAirHeating,
                                                        MSHeatPump(MSHeatPumpNum).SuppPlantLoc,
                                                        errFlag,
                                                        _,
                                                        _,
                                                        _,
                                                        _,
                                                        _);
                if (errFlag) {
                    ShowFatalError(state, "InitMSHeatPump: Program terminated for previous conditions.");
                }
                MSHeatPump(MSHeatPumpNum).MaxSuppCoilFluidFlow =
                    SteamCoils::GetCoilMaxSteamFlowRate(state, MSHeatPump(MSHeatPumpNum).SuppHeatCoilNum, ErrorsFound);
                if (MSHeatPump(MSHeatPumpNum).MaxSuppCoilFluidFlow > 0.0) {
                    SteamDensity = Fluid::GetSteam(state)->getSatDensity(state, state.dataHVACMultiSpdHP->TempSteamIn, 1.0, RoutineName);
                    MSHeatPump(MSHeatPumpNum).MaxSuppCoilFluidFlow *= SteamDensity;
                }

                // fill outlet node for coil
                MSHeatPump(MSHeatPumpNum).SuppCoilOutletNode =
                    DataPlant::CompData::getPlantComponent(state, MSHeatPump(MSHeatPumpNum).SuppPlantLoc).NodeNumOut;
                MSHeatPump(MSHeatPumpNum).MyPlantScantFlag = false;
            }
        } else if (MSHeatPump(MSHeatPumpNum).MyPlantScantFlag && !state.dataGlobal->AnyPlantInModel) {
            MSHeatPump(MSHeatPumpNum).MyPlantScantFlag = false;
        }

        if (!state.dataGlobal->SysSizingCalc && MSHeatPump(MSHeatPumpNum).MySizeFlag) {
            MSHeatPump(MSHeatPumpNum).FanVolFlow = state.dataFans->fans(MSHeatPump(MSHeatPumpNum).FanNum)->maxAirFlowRate;
            SizeMSHeatPump(state, MSHeatPumpNum);
            MSHeatPump(MSHeatPumpNum).FlowFraction = 1.0;
            MSHeatPump(MSHeatPumpNum).MySizeFlag = false;
            // Pass the fan cycling schedule index up to the air loop. Set the air loop unitary system flag.
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).cycFanSched = MSHeatPump(MSHeatPumpNum).fanOpModeSched;
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).UnitarySys = true;
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).UnitarySysSimulating =
                false; // affects child coil sizing by allowing coil to size itself instead of parent telling coil what size to use
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).fanOp = MSHeatPump(MSHeatPumpNum).fanOp;
        }
        if (MSHeatPump(MSHeatPumpNum).reportACCAManualS && !state.dataGlobal->DoingSizing && !state.dataGlobal->WarmupFlag) {
            MSHeatPump(MSHeatPumpNum).reportACCAManualS = false;
            // ACCA Manual S reporting
            Real64 SysSensCoolingLoad = 0.0;
            Real64 SysTotCoolingLoad = 0.0;
            Real64 SysLatCoolingLoad = 0.0;
            Real64 SysHeatingLoad = 0.0;

            if (MSHeatPump(MSHeatPumpNum).isHeatPump &&
                (state.dataSize->CurSysNum > 0 && !state.dataSize->FinalSysSizing.empty() &&
                 state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).heatCoilSizingMethod != DataSizing::HeatCoilSizMethod::None)) {

                if (state.dataSize->CurSysNum > 0) {
                    SysTotCoolingLoad = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).TotCoolCap;
                    SysSensCoolingLoad = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).SensCoolCap;
                    SysLatCoolingLoad = SysTotCoolingLoad - SysSensCoolingLoad;
                    SysHeatingLoad = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).HeatCap;
                }

                Real64 coilSHR = 1.0;
                auto const &thisCoolCoil = state.dataDXCoils->DXCoil(MSHeatPump(MSHeatPumpNum).DXCoolCoilIndex);
                std::string_view cCoilName = thisCoolCoil.Name;
                Real64 ratedCoolingCap = thisCoolCoil.MSRatedTotCap(MSHeatPump(MSHeatPumpNum).NumOfSpeedCooling);
                coilSHR = thisCoolCoil.MSRatedSHR(MSHeatPump(MSHeatPumpNum).NumOfSpeedCooling);
                auto const &thisHeatCoil = state.dataDXCoils->DXCoil(MSHeatPump(MSHeatPumpNum).DXHeatCoilIndex);
                std::string_view hCoilName = thisHeatCoil.Name;
                Real64 ratedHeatingCap = thisCoolCoil.MSRatedTotCap(MSHeatPump(MSHeatPumpNum).NumOfSpeedHeating);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchMSHPType, cCoilName, "AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed");
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSHPName, cCoilName, MSHeatPump(MSHeatPumpNum).Name);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchMSCoilType, cCoilName, HVAC::cAllCoilTypes(int(MSHeatPump(MSHeatPumpNum).CoolCoilType)));
                if (state.dataSize->CurSysNum > 0) {
                    OutputReportPredefined::PreDefTableEntry(state,
                                                             state.dataOutRptPredefined->pdchMSSizMethod,
                                                             cCoilName,
                                                             DataSizing::HeatCoilSizMethodNamesUC[static_cast<int>(
                                                                 state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).heatCoilSizingMethod)]);
                    OutputReportPredefined::PreDefTableEntry(state,
                                                             state.dataOutRptPredefined->pdchMSSizMethod,
                                                             hCoilName,
                                                             DataSizing::HeatCoilSizMethodNamesUC[static_cast<int>(
                                                                 state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).heatCoilSizingMethod)]);
                } else {
                    OutputReportPredefined::PreDefTableEntry(
                        state,
                        state.dataOutRptPredefined->pdchMSSizMethod,
                        cCoilName,
                        DataSizing::HeatCoilSizMethodNamesUC[static_cast<int>(
                            state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).heatCoilSizingMethod)]);
                    OutputReportPredefined::PreDefTableEntry(
                        state,
                        state.dataOutRptPredefined->pdchMSSizMethod,
                        hCoilName,
                        DataSizing::HeatCoilSizMethodNamesUC[static_cast<int>(
                            state.dataSize->FinalZoneSizing(state.dataSize->CurZoneEqNum).heatCoilSizingMethod)]);
                }
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSTotLoad, cCoilName, SysTotCoolingLoad);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSSensLoad, cCoilName, SysSensCoolingLoad);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSTotCapacity, cCoilName, ratedCoolingCap);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSSensCapacity, cCoilName, ratedCoolingCap * coilSHR);
                Real64 coilTotFactor = (SysTotCoolingLoad > 0.0) ? ratedCoolingCap / SysTotCoolingLoad : 1.0;
                Real64 coilSensFactor = (SysSensCoolingLoad > 0.0) ? ratedCoolingCap * coilSHR / SysSensCoolingLoad : 1.0;
                Real64 coilLatFactor = (SysLatCoolingLoad > 0.0) ? ratedCoolingCap * (1.0 - coilSHR) / SysLatCoolingLoad : 1.0;
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSTotRatio, cCoilName, coilTotFactor);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSSensRatio, cCoilName, coilSensFactor);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSLatRatio, cCoilName, coilLatFactor);

                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchMSHPType, hCoilName, "AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed");
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSHPName, hCoilName, MSHeatPump(MSHeatPumpNum).Name);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchMSCoilType, hCoilName, HVAC::cAllCoilTypes(int(MSHeatPump(MSHeatPumpNum).HeatCoilType)));
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSTotLoad, hCoilName, SysHeatingLoad);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSSensLoad, hCoilName, SysHeatingLoad);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSTotCapacity, hCoilName, ratedHeatingCap);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSSensCapacity, hCoilName, ratedHeatingCap);
                coilTotFactor = (SysHeatingLoad > 0.0) ? ratedHeatingCap / SysHeatingLoad : 1.0;
                coilLatFactor = 0.0;
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSTotRatio, hCoilName, coilTotFactor);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSSensRatio, hCoilName, coilTotFactor);
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSLatRatio, hCoilName, coilLatFactor);
            }
        }

        if (allocated(state.dataZoneEquip->ZoneEquipConfig) && MSHeatPump(MSHeatPumpNum).MyCheckFlag) {
            int zoneNum = MSHeatPump(MSHeatPumpNum).ControlZoneNum;
            int zoneInlet = MSHeatPump(MSHeatPumpNum).ZoneInletNode;
            // setup furnace zone equipment sequence information based on finding matching air terminal
            if (state.dataZoneEquip->ZoneEquipConfig(zoneNum).EquipListIndex > 0) {
                int coolingPriority = 0;
                int heatingPriority = 0;
                state.dataZoneEquip->ZoneEquipList(state.dataZoneEquip->ZoneEquipConfig(zoneNum).EquipListIndex)
                    .getPrioritiesForInletNode(state, zoneInlet, coolingPriority, heatingPriority);
                MSHeatPump(MSHeatPumpNum).ZoneSequenceCoolingNum = coolingPriority;
                MSHeatPump(MSHeatPumpNum).ZoneSequenceHeatingNum = heatingPriority;
            }
            MSHeatPump(MSHeatPumpNum).MyCheckFlag = false;
            if (MSHeatPump(MSHeatPumpNum).ZoneSequenceCoolingNum == 0 || MSHeatPump(MSHeatPumpNum).ZoneSequenceHeatingNum == 0) {
                ShowSevereError(state,
                                format("AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed, \"{}\": Airloop air terminal in the zone equipment list for "
                                       "zone = {} not found or is not allowed Zone Equipment Cooling or Heating Sequence = 0.",
                                       MSHeatPump(MSHeatPumpNum).Name,
                                       MSHeatPump(MSHeatPumpNum).ControlZoneName));
                ShowFatalError(state,
                               "Subroutine InitMSHeatPump: Errors found in getting AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed input.  Preceding "
                               "condition(s) causes termination.");
            }
        }

        // Find the number of zones (zone Inlet Nodes) attached to an air loop from the air loop number
        NumAirLoopZones =
            state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesCooled + state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesHeated;
        if (allocated(state.dataAirLoop->AirToZoneNodeInfo) && MSHeatPump(MSHeatPumpNum).MyFlowFracFlag) {
            state.dataHVACMultiSpdHP->FlowFracFlagReady = true;
            for (int ZoneInSysIndex = 1; ZoneInSysIndex <= NumAirLoopZones; ++ZoneInSysIndex) {
                // zone inlet nodes for cooling
                if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesCooled > 0) {
                    if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolInletNodes(ZoneInSysIndex) == -999) {
                        // the data structure for the zones inlet nodes has not been filled
                        state.dataHVACMultiSpdHP->FlowFracFlagReady = false;
                    }
                }
                // zone inlet nodes for heating
                if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesHeated > 0) {
                    if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitHeatInletNodes(ZoneInSysIndex) == -999) {
                        // the data structure for the zones inlet nodes has not been filled
                        state.dataHVACMultiSpdHP->FlowFracFlagReady = false;
                    }
                }
            }
        }
        if (allocated(state.dataAirLoop->AirToZoneNodeInfo) && state.dataHVACMultiSpdHP->FlowFracFlagReady) {
            SumOfMassFlowRateMax = 0.0; // initialize the sum of the maximum flows
            for (int ZoneInSysIndex = 1; ZoneInSysIndex <= NumAirLoopZones; ++ZoneInSysIndex) {
                int ZoneInletNodeNum = state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).TermUnitCoolInletNodes(ZoneInSysIndex);
                SumOfMassFlowRateMax += state.dataLoopNodes->Node(ZoneInletNodeNum).MassFlowRateMax;
                if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).CoolCtrlZoneNums(ZoneInSysIndex) == MSHeatPump(MSHeatPumpNum).ControlZoneNum) {
                    CntrlZoneTerminalUnitMassFlowRateMax = state.dataLoopNodes->Node(ZoneInletNodeNum).MassFlowRateMax;
                }
            }
            if (SumOfMassFlowRateMax != 0.0 && MSHeatPump(MSHeatPumpNum).MyFlowFracFlag) {
                if (CntrlZoneTerminalUnitMassFlowRateMax >= HVAC::SmallAirVolFlow) {
                    MSHeatPump(MSHeatPumpNum).FlowFraction = CntrlZoneTerminalUnitMassFlowRateMax / SumOfMassFlowRateMax;
                } else {
                    ShowSevereError(state, format("{} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, MSHeatPump(MSHeatPumpNum).Name));
                    ShowContinueError(state, " The Fraction of Supply Air Flow That Goes Through the Controlling Zone is set to 1.");
                }
                BaseSizer::reportSizerOutput(state,
                                             state.dataHVACMultiSpdHP->CurrentModuleObject,
                                             MSHeatPump(MSHeatPumpNum).Name,
                                             "Fraction of Supply Air Flow That Goes Through the Controlling Zone",
                                             MSHeatPump(MSHeatPumpNum).FlowFraction);
                MSHeatPump(MSHeatPumpNum).MyFlowFracFlag = false;
            }
        }

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && MSHeatPump(MSHeatPumpNum).MyEnvrnFlag) {
            RhoAir = state.dataEnvrn->StdRhoAir;
            // set the mass flow rates from the input volume flow rates
            for (i = 1; i <= NumOfSpeedCooling; ++i) {
                MSHeatPump(MSHeatPumpNum).CoolMassFlowRate(i) = RhoAir * MSHeatPump(MSHeatPumpNum).CoolVolumeFlowRate(i);
            }
            for (i = 1; i <= NumOfSpeedHeating; ++i) {
                MSHeatPump(MSHeatPumpNum).HeatMassFlowRate(i) = RhoAir * MSHeatPump(MSHeatPumpNum).HeatVolumeFlowRate(i);
            }
            MSHeatPump(MSHeatPumpNum).IdleMassFlowRate = RhoAir * MSHeatPump(MSHeatPumpNum).IdleVolumeAirRate;
            // set the node max and min mass flow rates
            state.dataLoopNodes->Node(InNode).MassFlowRateMax =
                max(MSHeatPump(MSHeatPumpNum).CoolMassFlowRate(NumOfSpeedCooling), MSHeatPump(MSHeatPumpNum).HeatMassFlowRate(NumOfSpeedHeating));
            state.dataLoopNodes->Node(InNode).MassFlowRateMaxAvail =
                max(MSHeatPump(MSHeatPumpNum).CoolMassFlowRate(NumOfSpeedCooling), MSHeatPump(MSHeatPumpNum).HeatMassFlowRate(NumOfSpeedHeating));
            state.dataLoopNodes->Node(InNode).MassFlowRateMin = 0.0;
            state.dataLoopNodes->Node(InNode).MassFlowRateMinAvail = 0.0;
            state.dataLoopNodes->Node(OutNode) = state.dataLoopNodes->Node(InNode);
            MSHeatPump(MSHeatPumpNum).LoadLoss = 0.0;

            if ((MSHeatPump(MSHeatPumpNum).HeatRecActive) && (!MSHeatPump(MSHeatPumpNum).MyPlantScantFlag)) {

                rho = state.dataPlnt->PlantLoop(MSHeatPump(MSHeatPumpNum).HRPlantLoc.loopNum)
                          .glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);

                MSHeatPump(MSHeatPumpNum).DesignHeatRecMassFlowRate = MSHeatPump(MSHeatPumpNum).DesignHeatRecFlowRate * rho;

                PlantUtilities::InitComponentNodes(state,
                                                   0.0,
                                                   MSHeatPump(MSHeatPumpNum).DesignHeatRecMassFlowRate,
                                                   MSHeatPump(MSHeatPumpNum).HeatRecInletNodeNum,
                                                   MSHeatPump(MSHeatPumpNum).HeatRecOutletNodeNum);
            }
            if (MSHeatPump(MSHeatPumpNum).CoilControlNode > 0) {
                if (MSHeatPump(MSHeatPumpNum).MaxCoilFluidFlow == DataSizing::AutoSize) {
                    if (MSHeatPump(MSHeatPumpNum).HeatCoilType == HVAC::Coil_HeatingWater) {
                        WaterCoils::SimulateWaterCoilComponents(
                            state, MSHeatPump(MSHeatPumpNum).HeatCoilName, FirstHVACIteration, MSHeatPump(MSHeatPumpNum).HeatCoilNum);

                        CoilMaxVolFlowRate =
                            WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", MSHeatPump(MSHeatPumpNum).HeatCoilName, ErrorsFound);
                        if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                            rho = state.dataPlnt->PlantLoop(MSHeatPump(MSHeatPumpNum).plantLoc.loopNum)
                                      .glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                            MSHeatPump(MSHeatPumpNum).MaxCoilFluidFlow = CoilMaxVolFlowRate * rho;
                        }
                        PlantUtilities::InitComponentNodes(state,
                                                           0.0,
                                                           MSHeatPump(MSHeatPumpNum).MaxCoilFluidFlow,
                                                           MSHeatPump(MSHeatPumpNum).CoilControlNode,
                                                           MSHeatPump(MSHeatPumpNum).CoilOutletNode);
                    }
                    if (MSHeatPump(MSHeatPumpNum).HeatCoilType == HVAC::Coil_HeatingSteam) {

                        SteamCoils::SimulateSteamCoilComponents(state,
                                                                MSHeatPump(MSHeatPumpNum).HeatCoilName,
                                                                FirstHVACIteration,
                                                                MSHeatPump(MSHeatPumpNum).HeatCoilNum,
                                                                1.0,
                                                                QActual); // QCoilReq, simulate any load > 0 to get max capacity of steam coil
                        CoilMaxVolFlowRate = SteamCoils::GetCoilMaxSteamFlowRate(state, MSHeatPump(MSHeatPumpNum).HeatCoilNum, ErrorsFound);

                        if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                            SteamDensity = Fluid::GetSteam(state)->getSatDensity(state, state.dataHVACMultiSpdHP->TempSteamIn, 1.0, RoutineName);
                            MSHeatPump(MSHeatPumpNum).MaxCoilFluidFlow = CoilMaxVolFlowRate * SteamDensity;
                        }
                        PlantUtilities::InitComponentNodes(state,
                                                           0.0,
                                                           MSHeatPump(MSHeatPumpNum).MaxCoilFluidFlow,
                                                           MSHeatPump(MSHeatPumpNum).CoilControlNode,
                                                           MSHeatPump(MSHeatPumpNum).CoilOutletNode);
                    }
                }
            }
            if (MSHeatPump(MSHeatPumpNum).SuppCoilControlNode > 0) {
                if (MSHeatPump(MSHeatPumpNum).MaxSuppCoilFluidFlow == DataSizing::AutoSize) {
                    if (MSHeatPump(MSHeatPumpNum).SuppHeatCoilType == HVAC::Coil_HeatingWater) {
                        WaterCoils::SimulateWaterCoilComponents(
                            state, MSHeatPump(MSHeatPumpNum).SuppHeatCoilName, FirstHVACIteration, MSHeatPump(MSHeatPumpNum).SuppHeatCoilNum);

                        CoilMaxVolFlowRate =
                            WaterCoils::GetCoilMaxWaterFlowRate(state, "Coil:Heating:Water", MSHeatPump(MSHeatPumpNum).SuppHeatCoilName, ErrorsFound);
                        if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                            rho = state.dataPlnt->PlantLoop(MSHeatPump(MSHeatPumpNum).SuppPlantLoc.loopNum)
                                      .glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                            MSHeatPump(MSHeatPumpNum).MaxSuppCoilFluidFlow = CoilMaxVolFlowRate * rho;
                        }
                        PlantUtilities::InitComponentNodes(state,
                                                           0.0,
                                                           MSHeatPump(MSHeatPumpNum).MaxSuppCoilFluidFlow,
                                                           MSHeatPump(MSHeatPumpNum).SuppCoilControlNode,
                                                           MSHeatPump(MSHeatPumpNum).SuppCoilOutletNode);
                    }
                    if (MSHeatPump(MSHeatPumpNum).SuppHeatCoilType == HVAC::Coil_HeatingSteam) {

                        SteamCoils::SimulateSteamCoilComponents(state,
                                                                MSHeatPump(MSHeatPumpNum).SuppHeatCoilName,
                                                                FirstHVACIteration,
                                                                MSHeatPump(MSHeatPumpNum).SuppHeatCoilNum,
                                                                1.0,
                                                                QActual); // QCoilReq, simulate any load > 0 to get max capacity of steam coil
                        CoilMaxVolFlowRate = SteamCoils::GetCoilMaxSteamFlowRate(state, MSHeatPump(MSHeatPumpNum).SuppHeatCoilNum, ErrorsFound);

                        if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                            SteamDensity = Fluid::GetSteam(state)->getSatDensity(state, state.dataHVACMultiSpdHP->TempSteamIn, 1.0, RoutineName);
                            MSHeatPump(MSHeatPumpNum).MaxSuppCoilFluidFlow = CoilMaxVolFlowRate * SteamDensity;
                        }
                        PlantUtilities::InitComponentNodes(state,
                                                           0.0,
                                                           MSHeatPump(MSHeatPumpNum).MaxSuppCoilFluidFlow,
                                                           MSHeatPump(MSHeatPumpNum).SuppCoilControlNode,
                                                           MSHeatPump(MSHeatPumpNum).SuppCoilOutletNode);
                    }
                }
            }
            MSHeatPump(MSHeatPumpNum).MyEnvrnFlag = false;
        } // end one time inits

        if (!state.dataGlobal->BeginEnvrnFlag) {
            MSHeatPump(MSHeatPumpNum).MyEnvrnFlag = true;
        }

        // IF MSHP system was not autosized and the fan is autosized, check that fan volumetric flow rate is greater than MSHP flow rates
        if (!state.dataGlobal->DoingSizing && MSHeatPump(MSHeatPumpNum).CheckFanFlow) {
            state.dataHVACMultiSpdHP->CurrentModuleObject = "AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed";
            MSHeatPump(MSHeatPumpNum).FanVolFlow = state.dataFans->fans(MSHeatPump(MSHeatPumpNum).FanNum)->maxAirFlowRate;
            if (MSHeatPump(MSHeatPumpNum).FanVolFlow != DataSizing::AutoSize) {
                //     Check fan versus system supply air flow rates
                if (MSHeatPump(MSHeatPumpNum).FanVolFlow < MSHeatPump(MSHeatPumpNum).CoolVolumeFlowRate(NumOfSpeedCooling)) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the MSHP system air flow rate when cooling is "
                                            "required ({:.7T}).",
                                            state.dataHVACMultiSpdHP->CurrentModuleObject,
                                            MSHeatPump(MSHeatPumpNum).FanVolFlow,
                                            MSHeatPump(MSHeatPumpNum).FanName,
                                            MSHeatPump(MSHeatPumpNum).CoolVolumeFlowRate(NumOfSpeedCooling)));
                    ShowContinueError(
                        state, " The MSHP system flow rate when cooling is required is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state,
                                      format(" Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, MSHeatPump(MSHeatPumpNum).Name));
                    MSHeatPump(MSHeatPumpNum).CoolVolumeFlowRate(NumOfSpeedCooling) = MSHeatPump(MSHeatPumpNum).FanVolFlow;
                    // Check flow rates in other speeds and ensure flow rates are not above the max flow rate
                    for (i = NumOfSpeedCooling - 1; i >= 1; --i) {
                        if (MSHeatPump(MSHeatPumpNum).CoolVolumeFlowRate(i) > MSHeatPump(MSHeatPumpNum).CoolVolumeFlowRate(i + 1)) {
                            ShowContinueError(state,
                                              format(" The MSHP system flow rate when cooling is required is reset to the flow rate at higher speed "
                                                     "and the simulation continues at Speed{}.",
                                                     i));
                            ShowContinueError(
                                state, format(" Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, MSHeatPump(MSHeatPumpNum).Name));
                            MSHeatPump(MSHeatPumpNum).CoolVolumeFlowRate(i) = MSHeatPump(MSHeatPumpNum).CoolVolumeFlowRate(i + 1);
                        }
                    }
                }
                if (MSHeatPump(MSHeatPumpNum).FanVolFlow < MSHeatPump(MSHeatPumpNum).HeatVolumeFlowRate(NumOfSpeedHeating)) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the MSHP system air flow rate when heating is "
                                            "required ({:.7T}).",
                                            state.dataHVACMultiSpdHP->CurrentModuleObject,
                                            MSHeatPump(MSHeatPumpNum).FanVolFlow,
                                            MSHeatPump(MSHeatPumpNum).FanName,
                                            MSHeatPump(MSHeatPumpNum).HeatVolumeFlowRate(NumOfSpeedHeating)));
                    ShowContinueError(
                        state, " The MSHP system flow rate when heating is required is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state,
                                      format(" Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, MSHeatPump(MSHeatPumpNum).Name));
                    MSHeatPump(MSHeatPumpNum).HeatVolumeFlowRate(NumOfSpeedHeating) = MSHeatPump(MSHeatPumpNum).FanVolFlow;
                    for (i = NumOfSpeedHeating - 1; i >= 1; --i) {
                        if (MSHeatPump(MSHeatPumpNum).HeatVolumeFlowRate(i) > MSHeatPump(MSHeatPumpNum).HeatVolumeFlowRate(i + 1)) {
                            ShowContinueError(state,
                                              format(" The MSHP system flow rate when heating is required is reset to the flow rate at higher speed "
                                                     "and the simulation continues at Speed{}.",
                                                     i));
                            ShowContinueError(
                                state,
                                format(" Occurs in {} system = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, MSHeatPump(MSHeatPumpNum).Name));
                            MSHeatPump(MSHeatPumpNum).HeatVolumeFlowRate(i) = MSHeatPump(MSHeatPumpNum).HeatVolumeFlowRate(i + 1);
                        }
                    }
                }
                if (MSHeatPump(MSHeatPumpNum).FanVolFlow < MSHeatPump(MSHeatPumpNum).IdleVolumeAirRate &&
                    MSHeatPump(MSHeatPumpNum).IdleVolumeAirRate != 0.0) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the MSHP system air flow rate when no heating "
                                            "or cooling is needed ({:.7T}).",
                                            state.dataHVACMultiSpdHP->CurrentModuleObject,
                                            MSHeatPump(MSHeatPumpNum).FanVolFlow,
                                            MSHeatPump(MSHeatPumpNum).FanName,
                                            MSHeatPump(MSHeatPumpNum).IdleVolumeAirRate));
                    ShowContinueError(state,
                                      " The MSHP system flow rate when no heating or cooling is needed is reset to the fan flow rate and the "
                                      "simulation continues.");
                    ShowContinueError(state,
                                      format(" Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, MSHeatPump(MSHeatPumpNum).Name));
                    MSHeatPump(MSHeatPumpNum).IdleVolumeAirRate = MSHeatPump(MSHeatPumpNum).FanVolFlow;
                }
                RhoAir = state.dataEnvrn->StdRhoAir;
                // set the mass flow rates from the reset volume flow rates
                for (i = 1; i <= NumOfSpeedCooling; ++i) {
                    MSHeatPump(MSHeatPumpNum).CoolMassFlowRate(i) = RhoAir * MSHeatPump(MSHeatPumpNum).CoolVolumeFlowRate(i);
                    if (MSHeatPump(MSHeatPumpNum).FanVolFlow > 0.0) {
                        MSHeatPump(MSHeatPumpNum).CoolingSpeedRatio(i) =
                            MSHeatPump(MSHeatPumpNum).CoolVolumeFlowRate(i) / MSHeatPump(MSHeatPumpNum).FanVolFlow;
                    }
                }
                for (i = 1; i <= NumOfSpeedHeating; ++i) {
                    MSHeatPump(MSHeatPumpNum).HeatMassFlowRate(i) = RhoAir * MSHeatPump(MSHeatPumpNum).HeatVolumeFlowRate(i);
                    if (MSHeatPump(MSHeatPumpNum).FanVolFlow > 0.0) {
                        MSHeatPump(MSHeatPumpNum).HeatingSpeedRatio(i) =
                            MSHeatPump(MSHeatPumpNum).HeatVolumeFlowRate(i) / MSHeatPump(MSHeatPumpNum).FanVolFlow;
                    }
                }
                MSHeatPump(MSHeatPumpNum).IdleMassFlowRate = RhoAir * MSHeatPump(MSHeatPumpNum).IdleVolumeAirRate;
                if (MSHeatPump(MSHeatPumpNum).FanVolFlow > 0.0) {
                    MSHeatPump(MSHeatPumpNum).IdleSpeedRatio = MSHeatPump(MSHeatPumpNum).IdleVolumeAirRate / MSHeatPump(MSHeatPumpNum).FanVolFlow;
                }
                // set the node max and min mass flow rates based on reset volume flow rates
                state.dataLoopNodes->Node(InNode).MassFlowRateMax =
                    max(MSHeatPump(MSHeatPumpNum).CoolMassFlowRate(NumOfSpeedCooling), MSHeatPump(MSHeatPumpNum).HeatMassFlowRate(NumOfSpeedHeating));
                state.dataLoopNodes->Node(InNode).MassFlowRateMaxAvail =
                    max(MSHeatPump(MSHeatPumpNum).CoolMassFlowRate(NumOfSpeedCooling), MSHeatPump(MSHeatPumpNum).HeatMassFlowRate(NumOfSpeedHeating));
                state.dataLoopNodes->Node(InNode).MassFlowRateMin = 0.0;
                state.dataLoopNodes->Node(InNode).MassFlowRateMinAvail = 0.0;
                state.dataLoopNodes->Node(OutNode) = state.dataLoopNodes->Node(InNode);
                MSHeatPump(MSHeatPumpNum).CheckFanFlow = false;
            }
        }

        if (MSHeatPump(MSHeatPumpNum).fanOpModeSched != nullptr) {
            MSHeatPump(MSHeatPumpNum).fanOp =
                (MSHeatPump(MSHeatPumpNum).fanOpModeSched->getCurrentVal() == 0.0) ? HVAC::FanOp::Cycling : HVAC::FanOp::Continuous;
        }

        // Calculate air distribution losses
        if (!FirstHVACIteration && state.dataHVACMultiSpdHP->AirLoopPass == 1) {
            int ZoneInNode = MSHeatPump(MSHeatPumpNum).ZoneInletNode;
            DeltaMassRate = state.dataLoopNodes->Node(OutNode).MassFlowRate -
                            state.dataLoopNodes->Node(ZoneInNode).MassFlowRate / MSHeatPump(MSHeatPumpNum).FlowFraction;
            if (DeltaMassRate < 0.0) {
                DeltaMassRate = 0.0;
            }
            Real64 MassFlowRate(0.0);        // parent mass flow rate
            Real64 LatentOutput(0.0);        // latent output rate
            Real64 TotalOutput(0.0);         // total output rate
            Real64 SensibleOutputDelta(0.0); // delta sensible output rate
            Real64 LatentOutputDelta(0.0);   // delta latent output rate
            Real64 TotalOutputDelta(0.0);    // delta total output rate
            MassFlowRate = state.dataLoopNodes->Node(ZoneInNode).MassFlowRate / MSHeatPump(MSHeatPumpNum).FlowFraction;
            Real64 MinHumRat = state.dataLoopNodes->Node(ZoneInNode).HumRat;
            if (state.dataLoopNodes->Node(OutNode).Temp < state.dataLoopNodes->Node(MSHeatPump(MSHeatPumpNum).NodeNumOfControlledZone).Temp) {
                MinHumRat = state.dataLoopNodes->Node(OutNode).HumRat;
            }
            CalcZoneSensibleLatentOutput(MassFlowRate,
                                         state.dataLoopNodes->Node(OutNode).Temp,
                                         MinHumRat,
                                         state.dataLoopNodes->Node(ZoneInNode).Temp,
                                         MinHumRat,
                                         MSHeatPump(MSHeatPumpNum).LoadLoss,
                                         LatentOutput,
                                         TotalOutput);
            CalcZoneSensibleLatentOutput(DeltaMassRate,
                                         state.dataLoopNodes->Node(OutNode).Temp,
                                         MinHumRat,
                                         state.dataLoopNodes->Node(MSHeatPump(MSHeatPumpNum).NodeNumOfControlledZone).Temp,
                                         MinHumRat,
                                         SensibleOutputDelta,
                                         LatentOutputDelta,
                                         TotalOutputDelta);
            MSHeatPump(MSHeatPumpNum).LoadLoss = MSHeatPump(MSHeatPumpNum).LoadLoss + SensibleOutputDelta;
            if (std::abs(MSHeatPump(MSHeatPumpNum).LoadLoss) < 1.0e-6) {
                MSHeatPump(MSHeatPumpNum).LoadLoss = 0.0;
            }
        }

        // Returns load only for zones requesting cooling (heating). If in deadband, Qzoneload = 0.
        ZoneNum = MSHeatPump(MSHeatPumpNum).ControlZoneNum;
        if ((MSHeatPump(MSHeatPumpNum).ZoneSequenceCoolingNum > 0) && (MSHeatPump(MSHeatPumpNum).ZoneSequenceHeatingNum > 0)) {
            ZoneLoadToCoolSPSequenced = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(MSHeatPump(MSHeatPumpNum).ControlZoneNum)
                                            .SequencedOutputRequiredToCoolingSP(MSHeatPump(MSHeatPumpNum).ZoneSequenceCoolingNum);
            ZoneLoadToHeatSPSequenced = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(MSHeatPump(MSHeatPumpNum).ControlZoneNum)
                                            .SequencedOutputRequiredToHeatingSP(MSHeatPump(MSHeatPumpNum).ZoneSequenceHeatingNum);
            if (ZoneLoadToHeatSPSequenced > HVAC::SmallLoad && ZoneLoadToCoolSPSequenced > HVAC::SmallLoad) {
                QZnReq = ZoneLoadToHeatSPSequenced;
            } else if (ZoneLoadToHeatSPSequenced < (-1.0 * HVAC::SmallLoad) && ZoneLoadToCoolSPSequenced < (-1.0 * HVAC::SmallLoad)) {
                QZnReq = ZoneLoadToCoolSPSequenced;
            } else if (ZoneLoadToHeatSPSequenced <= (-1.0 * HVAC::SmallLoad) && ZoneLoadToCoolSPSequenced >= HVAC::SmallLoad) {
                QZnReq = 0.0;
            } else {
                QZnReq = 0.0; // Autodesk:Init Case added to prevent use of uninitialized value (occurred in MultiSpeedACFurnace example)
            }
            QZnReq /= MSHeatPump(MSHeatPumpNum).FlowFraction;
        } else {
            QZnReq = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputRequired / MSHeatPump(MSHeatPumpNum).FlowFraction;
        }
        if (state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) {
            QZnReq = 0.0;
        }

        if (QZnReq > HVAC::SmallLoad) {
            MSHeatPump(MSHeatPumpNum).HeatCoolMode = ModeOfOperation::HeatingMode;
        } else if (QZnReq < (-1.0 * HVAC::SmallLoad)) {
            MSHeatPump(MSHeatPumpNum).HeatCoolMode = ModeOfOperation::CoolingMode;
        } else {
            MSHeatPump(MSHeatPumpNum).HeatCoolMode = ModeOfOperation::Invalid;
        }

        // Determine the staged status
        if (allocated(state.dataZoneCtrls->StageZoneLogic)) {
            if (state.dataZoneCtrls->StageZoneLogic(ZoneNum)) {
                MSHeatPump(MSHeatPumpNum).Staged = true;
                MSHeatPump(MSHeatPumpNum).StageNum = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).StageNum;
            } else {
                if (MSHeatPump(MSHeatPumpNum).MyStagedFlag) {
                    ShowWarningError(state,
                                     "ZoneControl:Thermostat:StagedDualSetpoint is found, but is not applied to this "
                                     "AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed object = ");
                    ShowContinueError(state, format("{}. Please make correction. Simulation continues...", MSHeatPump(MSHeatPumpNum).Name));
                    MSHeatPump(MSHeatPumpNum).MyStagedFlag = false;
                }
            }
        }
        // Set the inlet node mass flow rate
        if (MSHeatPump(MSHeatPumpNum).fanOp == HVAC::FanOp::Continuous) {
            // constant fan mode
            if (QZnReq > HVAC::SmallLoad && !state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) {
                state.dataHVACMultiSpdHP->CompOnMassFlow = MSHeatPump(MSHeatPumpNum).HeatMassFlowRate(1);
                state.dataHVACMultiSpdHP->CompOnFlowRatio = MSHeatPump(MSHeatPumpNum).HeatingSpeedRatio(1);
                MSHeatPump(MSHeatPumpNum).LastMode = ModeOfOperation::HeatingMode;
            } else if (QZnReq < (-1.0 * HVAC::SmallLoad) && !state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) {
                state.dataHVACMultiSpdHP->CompOnMassFlow = MSHeatPump(MSHeatPumpNum).CoolMassFlowRate(1);
                state.dataHVACMultiSpdHP->CompOnFlowRatio = MSHeatPump(MSHeatPumpNum).CoolingSpeedRatio(1);
                MSHeatPump(MSHeatPumpNum).LastMode = ModeOfOperation::CoolingMode;
            } else {
                state.dataHVACMultiSpdHP->CompOnMassFlow = MSHeatPump(MSHeatPumpNum).IdleMassFlowRate;
                state.dataHVACMultiSpdHP->CompOnFlowRatio = MSHeatPump(MSHeatPumpNum).IdleSpeedRatio;
            }
            state.dataHVACMultiSpdHP->CompOffMassFlow = MSHeatPump(MSHeatPumpNum).IdleMassFlowRate;
            state.dataHVACMultiSpdHP->CompOffFlowRatio = MSHeatPump(MSHeatPumpNum).IdleSpeedRatio;
        } else {
            // cycling fan mode
            if (QZnReq > HVAC::SmallLoad && !state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) {
                state.dataHVACMultiSpdHP->CompOnMassFlow = MSHeatPump(MSHeatPumpNum).HeatMassFlowRate(1);
                state.dataHVACMultiSpdHP->CompOnFlowRatio = MSHeatPump(MSHeatPumpNum).HeatingSpeedRatio(1);
            } else if (QZnReq < (-1.0 * HVAC::SmallLoad) && !state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) {
                state.dataHVACMultiSpdHP->CompOnMassFlow = MSHeatPump(MSHeatPumpNum).CoolMassFlowRate(1);
                state.dataHVACMultiSpdHP->CompOnFlowRatio = MSHeatPump(MSHeatPumpNum).CoolingSpeedRatio(1);
            } else {
                state.dataHVACMultiSpdHP->CompOnMassFlow = 0.0;
                state.dataHVACMultiSpdHP->CompOnFlowRatio = 0.0;
            }
            state.dataHVACMultiSpdHP->CompOffMassFlow = 0.0;
            state.dataHVACMultiSpdHP->CompOffFlowRatio = 0.0;
        }

        // Set the inlet node mass flow rate
        if (MSHeatPump(MSHeatPumpNum).availSched->getCurrentVal() > 0.0 && state.dataHVACMultiSpdHP->CompOnMassFlow != 0.0) {
            OnOffAirFlowRatio = 1.0;
            if (FirstHVACIteration) {
                state.dataLoopNodes->Node(MSHeatPump(MSHeatPumpNum).AirInletNodeNum).MassFlowRate = state.dataHVACMultiSpdHP->CompOnMassFlow;
                PartLoadFrac = 0.0;
            } else {
                if (MSHeatPump(MSHeatPumpNum).HeatCoolMode != ModeOfOperation::Invalid) {
                    PartLoadFrac = 1.0;
                } else {
                    PartLoadFrac = 0.0;
                }
            }
        } else {
            PartLoadFrac = 0.0;
            state.dataLoopNodes->Node(InNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(OutNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(OutNode).MassFlowRateMaxAvail = 0.0;
            OnOffAirFlowRatio = 1.0;
        }

        // Check availability of DX coils
        if (MSHeatPump(MSHeatPumpNum).availSched->getCurrentVal() > 0.0) {
            if (MSHeatPump(MSHeatPumpNum).HeatCoolMode == ModeOfOperation::CoolingMode) {
                auto *coilAvailSched = DXCoils::GetDXCoilAvailSched( // TODO: Why isn't this stored on the struct?
                    state,
                    "Coil:Cooling:DX:MultiSpeed",
                    MSHeatPump(MSHeatPumpNum).DXCoolCoilName,
                    ErrorsFound,
                    MSHeatPump(MSHeatPumpNum).DXCoolCoilIndex);
                if (ErrorsFound) {
                    ShowFatalError(state, "InitMSHeatPump, The previous error causes termination.");
                }
                if (coilAvailSched->getCurrentVal() == 0.0) {
                    if (MSHeatPump(MSHeatPumpNum).CoolCountAvail == 0) {
                        ++MSHeatPump(MSHeatPumpNum).CoolCountAvail;
                        ShowWarningError(
                            state,
                            format("{} is ready to perform cooling, but its DX cooling coil = {} is not available at Available Schedule = {}.",
                                   MSHeatPump(MSHeatPumpNum).Name,
                                   MSHeatPump(MSHeatPumpNum).DXCoolCoilName,
                                   coilAvailSched->Name));
                        ShowContinueErrorTimeStamp(state, format("Availability schedule returned={:.1R}", coilAvailSched->getCurrentVal()));
                    } else {
                        ++MSHeatPump(MSHeatPumpNum).CoolCountAvail;
                        ShowRecurringWarningErrorAtEnd(state,
                                                       MSHeatPump(MSHeatPumpNum).Name + ": Cooling coil is still not available ...",
                                                       MSHeatPump(MSHeatPumpNum).CoolIndexAvail,
                                                       coilAvailSched->getCurrentVal(),
                                                       coilAvailSched->getCurrentVal());
                    }
                }
            }
            if (MSHeatPump(MSHeatPumpNum).HeatCoolMode == ModeOfOperation::HeatingMode &&
                MSHeatPump(MSHeatPumpNum).HeatCoilType == HVAC::CoilDX_MultiSpeedHeating) {
                auto *coilAvailSched = DXCoils::GetDXCoilAvailSched(state,
                                                                    "Coil:Heating:DX:MultiSpeed",
                                                                    MSHeatPump(MSHeatPumpNum).DXHeatCoilName,
                                                                    ErrorsFound,
                                                                    MSHeatPump(MSHeatPumpNum).DXHeatCoilIndex);
                if (ErrorsFound) {
                    ShowFatalError(state, "InitMSHeatPump, The previous error causes termination.");
                }
                if (coilAvailSched->getCurrentVal() == 0.0) {
                    if (MSHeatPump(MSHeatPumpNum).HeatCountAvail == 0) {
                        ++MSHeatPump(MSHeatPumpNum).HeatCountAvail;
                        ShowWarningError(
                            state,
                            format("{} is ready to perform heating, but its DX heating coil = {} is not available at Available Schedule = {}.",
                                   MSHeatPump(MSHeatPumpNum).Name,
                                   MSHeatPump(MSHeatPumpNum).DXCoolCoilName,
                                   coilAvailSched->Name));
                        ShowContinueErrorTimeStamp(state, format("Availability schedule returned={:.1R}", coilAvailSched->getCurrentVal()));
                    } else {
                        ++MSHeatPump(MSHeatPumpNum).HeatCountAvail;
                        ShowRecurringWarningErrorAtEnd(state,
                                                       MSHeatPump(MSHeatPumpNum).Name + ": Heating coil is still not available ...",
                                                       MSHeatPump(MSHeatPumpNum).HeatIndexAvail,
                                                       coilAvailSched->getCurrentVal(),
                                                       coilAvailSched->getCurrentVal());
                    }
                }
            }
        }

        state.dataHVACMultiSpdHP->MSHeatPumpReport(MSHeatPumpNum).CycRatio = 0.0;
        state.dataHVACMultiSpdHP->MSHeatPumpReport(MSHeatPumpNum).SpeedRatio = 0.0;
        state.dataHVACMultiSpdHP->MSHeatPumpReport(MSHeatPumpNum).SpeedNum = 0;

        CalcMSHeatPump(state,
                       MSHeatPumpNum,
                       FirstHVACIteration,
                       HVAC::CompressorOp::On,
                       1,
                       0.0,
                       PartLoadFrac,
                       QSensUnitOut,
                       QZnReq,
                       OnOffAirFlowRatio,
                       state.dataHVACMultiSpdHP->SupHeaterLoad);

        auto &e = MSHeatPump(MSHeatPumpNum);
        {
            e.TotHeatEnergyRate = 0.0;
            e.SensHeatEnergyRate = 0.0;
            e.LatHeatEnergyRate = 0.0;
            e.TotCoolEnergyRate = 0.0;
            e.SensCoolEnergyRate = 0.0;
            e.LatCoolEnergyRate = 0.0;
        }
        // If unit is scheduled OFF, setpoint is equal to inlet node temperature.
        //!!LKL Discrepancy with < 0
        if (MSHeatPump(MSHeatPumpNum).availSched->getCurrentVal() == 0.0) {
            state.dataLoopNodes->Node(OutNode).Temp = state.dataLoopNodes->Node(InNode).Temp;
            return;
        }

        if ((MSHeatPump(MSHeatPumpNum).HeatCoolMode == ModeOfOperation::Invalid && MSHeatPump(MSHeatPumpNum).fanOp == HVAC::FanOp::Cycling) ||
            state.dataHVACMultiSpdHP->CompOnMassFlow == 0.0) {
            QZnReq = 0.0;
            PartLoadFrac = 0.0;
            state.dataLoopNodes->Node(InNode).MassFlowRate = 0.0;
            state.dataLoopNodes->Node(OutNode).MassFlowRateMaxAvail = 0.0;
        }
        MSHeatPump(MSHeatPumpNum).LoadMet = 0.0;
        SetAverageAirFlow(state, MSHeatPumpNum, PartLoadFrac, OnOffAirFlowRatio);

        // Init maximum available Heat Recovery flow rate
        if ((MSHeatPump(MSHeatPumpNum).HeatRecActive) && (!MSHeatPump(MSHeatPumpNum).MyPlantScantFlag)) {
            if (PartLoadFrac > 0.0) {
                if (FirstHVACIteration) {
                    MdotHR = MSHeatPump(MSHeatPumpNum).DesignHeatRecMassFlowRate;
                } else {
                    if (MSHeatPump(MSHeatPumpNum).HeatRecoveryMassFlowRate > 0.0) {
                        MdotHR = MSHeatPump(MSHeatPumpNum).HeatRecoveryMassFlowRate;
                    } else {
                        MdotHR = MSHeatPump(MSHeatPumpNum).DesignHeatRecMassFlowRate;
                    }
                }
            } else {
                MdotHR = 0.0;
            }

            PlantUtilities::SetComponentFlowRate(state,
                                                 MdotHR,
                                                 MSHeatPump(MSHeatPumpNum).HeatRecInletNodeNum,
                                                 MSHeatPump(MSHeatPumpNum).HeatRecOutletNodeNum,
                                                 MSHeatPump(MSHeatPumpNum).HRPlantLoc);
        }

        // get operating capacity of water and steam coil
        if (FirstHVACIteration) {
            if (MSHeatPump(MSHeatPumpNum).HeatCoilType == HVAC::Coil_HeatingWater) {
                //     set air-side and steam-side mass flow rates
                state.dataLoopNodes->Node(MSHeatPump(MSHeatPumpNum).CoilAirInletNode).MassFlowRate = state.dataHVACMultiSpdHP->CompOnMassFlow;
                mdot = MSHeatPump(MSHeatPumpNum).MaxCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(state,
                                                     mdot,
                                                     MSHeatPump(MSHeatPumpNum).CoilControlNode,
                                                     MSHeatPump(MSHeatPumpNum).CoilOutletNode,
                                                     MSHeatPump(MSHeatPumpNum).plantLoc);
                //     simulate water coil to find operating capacity
                WaterCoils::SimulateWaterCoilComponents(
                    state, MSHeatPump(MSHeatPumpNum).HeatCoilName, FirstHVACIteration, MSHeatPump(MSHeatPumpNum).HeatCoilNum, QActual);
            } // from IF(MSHeatPump(MSHeatPumpNum)%HeatCoilType == Coil_HeatingWater) THEN

            if (MSHeatPump(MSHeatPumpNum).HeatCoilType == HVAC::Coil_HeatingSteam) {

                //     set air-side and steam-side mass flow rates
                state.dataLoopNodes->Node(MSHeatPump(MSHeatPumpNum).CoilAirInletNode).MassFlowRate = state.dataHVACMultiSpdHP->CompOnMassFlow;
                mdot = MSHeatPump(MSHeatPumpNum).MaxCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(state,
                                                     mdot,
                                                     MSHeatPump(MSHeatPumpNum).CoilControlNode,
                                                     MSHeatPump(MSHeatPumpNum).CoilOutletNode,
                                                     MSHeatPump(MSHeatPumpNum).plantLoc);

                //     simulate steam coil to find operating capacity
                SteamCoils::SimulateSteamCoilComponents(state,
                                                        MSHeatPump(MSHeatPumpNum).HeatCoilName,
                                                        FirstHVACIteration,
                                                        MSHeatPump(MSHeatPumpNum).HeatCoilNum,
                                                        1.0,
                                                        QActual); // QCoilReq, simulate any load > 0 to get max capacity of steam coil

            } // from IF(MSHeatPump(MSHeatPumpNum)%HeatCoilType == Coil_HeatingSteam) THEN
            if (MSHeatPump(MSHeatPumpNum).SuppHeatCoilType == HVAC::Coil_HeatingWater) {
                //     set air-side and steam-side mass flow rates
                state.dataLoopNodes->Node(MSHeatPump(MSHeatPumpNum).SuppCoilAirInletNode).MassFlowRate = state.dataHVACMultiSpdHP->CompOnMassFlow;
                mdot = MSHeatPump(MSHeatPumpNum).MaxSuppCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(state,
                                                     mdot,
                                                     MSHeatPump(MSHeatPumpNum).SuppCoilControlNode,
                                                     MSHeatPump(MSHeatPumpNum).SuppCoilOutletNode,
                                                     MSHeatPump(MSHeatPumpNum).SuppPlantLoc);
                //     simulate water coil to find operating capacity
                WaterCoils::SimulateWaterCoilComponents(
                    state, MSHeatPump(MSHeatPumpNum).SuppHeatCoilName, FirstHVACIteration, MSHeatPump(MSHeatPumpNum).SuppHeatCoilNum, QActual);
                MSHeatPump(MSHeatPumpNum).DesignSuppHeatingCapacity = QActual;

            } // from IF(MSHeatPump(MSHeatPumpNum)%SuppHeatCoilType == Coil_HeatingWater) THEN

            if (MSHeatPump(MSHeatPumpNum).SuppHeatCoilType == HVAC::Coil_HeatingSteam) {

                //     set air-side and steam-side mass flow rates
                state.dataLoopNodes->Node(MSHeatPump(MSHeatPumpNum).SuppCoilAirInletNode).MassFlowRate = state.dataHVACMultiSpdHP->CompOnMassFlow;
                mdot = MSHeatPump(MSHeatPumpNum).MaxSuppCoilFluidFlow;
                PlantUtilities::SetComponentFlowRate(state,
                                                     mdot,
                                                     MSHeatPump(MSHeatPumpNum).SuppCoilControlNode,
                                                     MSHeatPump(MSHeatPumpNum).SuppCoilOutletNode,
                                                     MSHeatPump(MSHeatPumpNum).SuppPlantLoc);

                //     simulate steam coil to find operating capacity
                SteamCoils::SimulateSteamCoilComponents(state,
                                                        MSHeatPump(MSHeatPumpNum).SuppHeatCoilName,
                                                        FirstHVACIteration,
                                                        MSHeatPump(MSHeatPumpNum).SuppHeatCoilNum,
                                                        1.0,
                                                        QActual); // QCoilReq, simulate any load > 0 to get max capacity of steam coil
                MSHeatPump(MSHeatPumpNum).DesignSuppHeatingCapacity =
                    SteamCoils::GetCoilCapacity(state, "Coil:Heating:Steam", MSHeatPump(MSHeatPumpNum).SuppHeatCoilName, ErrorsFound);

            } // from IF(MSHeatPump(MSHeatPumpNum)%SuppHeatCoilType == Coil_HeatingSteam) THEN
        } // from IF( FirstHVACIteration ) THEN
    }

    //******************************************************************************

    void SizeMSHeatPump(EnergyPlusData &state, int const MSHeatPumpNum) // Engine driven heat pump number
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Lixing Gu, FSEC
        //       DATE WRITTEN:    June 2007

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine is for sizing multispeed heat pump airflow rates and flow fraction.

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        std::string_view constexpr RoutineName = "SizeMSHeatPump";
        int NumOfSpeedCooling; // Number of speeds for cooling
        int NumOfSpeedHeating; // Number of speeds for heating
        int i;                 // Index to speed

        auto &MSHeatPump = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);
        if (state.dataSize->CurSysNum > 0 && state.dataSize->CurOASysNum == 0) {
            state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).supFanNum = MSHeatPump.FanNum;
            state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).supFanType = MSHeatPump.fanType;
            state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).supFanPlace = MSHeatPump.fanPlace;
        }

        NumOfSpeedCooling = MSHeatPump.NumOfSpeedCooling;
        NumOfSpeedHeating = MSHeatPump.NumOfSpeedHeating;

        MSHeatPump.isHeatPump =
            MSHeatPump.CoolCoilType == HVAC::CoilDX_MultiSpeedCooling && MSHeatPump.HeatCoilType == HVAC::CoilDX_MultiSpeedHeating;
        Real64 SysTotCoolingLoad;
        Real64 SysCoolingLoad;
        Real64 SysCoolingCapacity;
        Real64 SysLatCoolingLoad;
        Real64 SysHeatingCapacity;
        Real64 SysHeatingLoad;
        if (MSHeatPump.isHeatPump && !state.dataSize->FinalSysSizing.empty() &&
            state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).heatCoilSizingMethod != DataSizing::HeatCoilSizMethod::None) {
            bool const PrintFlag = false;
            bool errorsFound = false;
            std::string_view CompType;
            std::string_view CompName;
            auto const &finalSysSizing = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum);
            if (finalSysSizing.SizingOption == DataSizing::SizingConcurrence::NonCoincident) {
                state.dataSize->DataFlowUsedForSizing = finalSysSizing.NonCoinCoolMassFlow / state.dataEnvrn->StdRhoAir;
            } else {
                state.dataSize->DataFlowUsedForSizing = finalSysSizing.CoinCoolMassFlow / state.dataEnvrn->StdRhoAir;
            }

            auto const &thisCoolingCoil = state.dataDXCoils->DXCoil(MSHeatPump.DXCoolCoilIndex);
            Real64 TempSize = thisCoolingCoil.MSRatedTotCap(MSHeatPump.NumOfSpeedCooling);
            CoolingCapacitySizer sizingCoolingCapacity;
            sizingCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            SysCoolingCapacity = sizingCoolingCapacity.size(state, TempSize, errorsFound);
            SysCoolingLoad = finalSysSizing.SensCoolCap;
            SysTotCoolingLoad = finalSysSizing.TotCoolCap;
            SysLatCoolingLoad = SysTotCoolingLoad - SysCoolingLoad;

            if (finalSysSizing.SizingOption == DataSizing::SizingConcurrence::NonCoincident) {
                state.dataSize->DataFlowUsedForSizing = finalSysSizing.NonCoinHeatMassFlow / state.dataEnvrn->StdRhoAir;
            } else {
                state.dataSize->DataFlowUsedForSizing = finalSysSizing.CoinHeatMassFlow / state.dataEnvrn->StdRhoAir;
            }

            auto const &thisHeatingCoil = state.dataDXCoils->DXCoil(MSHeatPump.DXHeatCoilIndex);
            TempSize = thisHeatingCoil.MSRatedTotCap(MSHeatPump.NumOfSpeedHeating);
            HeatingCapacitySizer sizingHeatingCapacity;
            sizingHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, RoutineName);
            SysHeatingCapacity = sizingHeatingCapacity.size(state, TempSize, errorsFound) / MSHeatPump.HeatingSizingRatio;
            SysHeatingLoad = finalSysSizing.HeatCap;

            DataSizing::setHeatPumpSize(state, SysCoolingCapacity, SysHeatingCapacity, MSHeatPump.HeatingSizingRatio);
            auto &EqSizing = state.dataSize->UnitarySysEqSizing(state.dataSize->CurSysNum);
            EqSizing.CoolingCapacity = true;
            EqSizing.DesCoolingLoad = SysCoolingCapacity;
            EqSizing.HeatingCapacity = true;
            EqSizing.DesHeatingLoad = SysHeatingCapacity;
            state.dataSize->DataFlowUsedForSizing = 0.0;
        }

        for (i = NumOfSpeedCooling; i >= 1; --i) {

            if (MSHeatPump.CoolVolumeFlowRate(i) == DataSizing::AutoSize) {
                if (state.dataSize->CurSysNum > 0) {
                    if (i == NumOfSpeedCooling) {
                        CheckSysSizing(state, state.dataHVACMultiSpdHP->CurrentModuleObject, MSHeatPump.Name);
                        MSHeatPump.CoolVolumeFlowRate(i) = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow;
                        if (MSHeatPump.FanVolFlow < MSHeatPump.CoolVolumeFlowRate(i) && MSHeatPump.FanVolFlow != DataSizing::AutoSize) {
                            MSHeatPump.CoolVolumeFlowRate(i) = MSHeatPump.FanVolFlow;
                            ShowWarningError(state, format("{} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, MSHeatPump.Name));
                            ShowContinueError(state,
                                              "The supply air flow rate at high speed is less than the autosized value for the supply air flow rate "
                                              "in cooling mode. Consider autosizing the fan for this simulation.");
                            ShowContinueError(
                                state,
                                "The air flow rate at high speed in cooling mode is reset to the supply air flow rate and the simulation continues.");
                        }
                    } else {
                        MSHeatPump.CoolVolumeFlowRate(i) = MSHeatPump.CoolVolumeFlowRate(NumOfSpeedCooling) * i / NumOfSpeedCooling;
                    }
                    if (MSHeatPump.CoolVolumeFlowRate(i) < HVAC::SmallAirVolFlow) {
                        MSHeatPump.CoolVolumeFlowRate = 0.0;
                    }
                    // Ensure the flow rate at lower speed has to be less or equal to the flow rate at higher speed
                    if (i != NumOfSpeedCooling) {
                        if (MSHeatPump.CoolVolumeFlowRate(i) > MSHeatPump.CoolVolumeFlowRate(i + 1)) {
                            MSHeatPump.CoolVolumeFlowRate(i) = MSHeatPump.CoolVolumeFlowRate(i + 1);
                        }
                    }
                    BaseSizer::reportSizerOutput(state,
                                                 state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                 MSHeatPump.Name,
                                                 format("Speed {} Supply Air Flow Rate During Cooling Operation [m3/s]", i),
                                                 MSHeatPump.CoolVolumeFlowRate(i));
                }
            }
        }

        for (i = NumOfSpeedHeating; i >= 1; --i) {
            if (MSHeatPump.HeatVolumeFlowRate(i) == DataSizing::AutoSize) {
                if (state.dataSize->CurSysNum > 0) {
                    if (i == NumOfSpeedHeating) {
                        CheckSysSizing(state, state.dataHVACMultiSpdHP->CurrentModuleObject, MSHeatPump.Name);
                        MSHeatPump.HeatVolumeFlowRate(i) = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow;
                        if (MSHeatPump.FanVolFlow < MSHeatPump.HeatVolumeFlowRate(i) && MSHeatPump.FanVolFlow != DataSizing::AutoSize) {
                            MSHeatPump.HeatVolumeFlowRate(i) = MSHeatPump.FanVolFlow;
                            ShowWarningError(state, format("{} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, MSHeatPump.Name));
                            ShowContinueError(state,
                                              "The supply air flow rate at high speed is less than the autosized value for the maximum air flow rate "
                                              "in heating mode. Consider autosizing the fan for this simulation.");
                            ShowContinueError(state,
                                              "The maximum air flow rate at high speed in heating mode is reset to the supply air flow rate and the "
                                              "simulation continues.");
                        }
                    } else {
                        MSHeatPump.HeatVolumeFlowRate(i) = MSHeatPump.HeatVolumeFlowRate(NumOfSpeedHeating) * i / NumOfSpeedHeating;
                    }
                    if (MSHeatPump.HeatVolumeFlowRate(i) < HVAC::SmallAirVolFlow) {
                        MSHeatPump.HeatVolumeFlowRate(i) = 0.0;
                    }
                    // Ensure the flow rate at lower speed has to be less or equal to the flow rate at higher speed
                    if (i != NumOfSpeedHeating) {
                        if (MSHeatPump.HeatVolumeFlowRate(i) > MSHeatPump.HeatVolumeFlowRate(i + 1)) {
                            MSHeatPump.HeatVolumeFlowRate(i) = MSHeatPump.HeatVolumeFlowRate(i + 1);
                        }
                    }
                    BaseSizer::reportSizerOutput(state,
                                                 state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                 MSHeatPump.Name,
                                                 format("Speed{}Supply Air Flow Rate During Heating Operation [m3/s]", i),
                                                 MSHeatPump.HeatVolumeFlowRate(i));
                }
            }
        }

        if (MSHeatPump.IdleVolumeAirRate == DataSizing::AutoSize) {
            if (state.dataSize->CurSysNum > 0) {
                CheckSysSizing(state, state.dataHVACMultiSpdHP->CurrentModuleObject, MSHeatPump.Name);
                MSHeatPump.IdleVolumeAirRate = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow;
                if (MSHeatPump.FanVolFlow < MSHeatPump.IdleVolumeAirRate && MSHeatPump.FanVolFlow != DataSizing::AutoSize) {
                    MSHeatPump.IdleVolumeAirRate = MSHeatPump.FanVolFlow;
                    ShowWarningError(state, format("{} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, MSHeatPump.Name));
                    ShowContinueError(state,
                                      "The supply air flow rate is less than the autosized value for the maximum air flow rate when no heating or "
                                      "cooling is needed. Consider autosizing the fan for this simulation.");
                    ShowContinueError(state,
                                      "The maximum air flow rate when no heating or cooling is needed is reset to the supply air flow rate and the "
                                      "simulation continues.");
                }
                if (MSHeatPump.IdleVolumeAirRate < HVAC::SmallAirVolFlow) {
                    MSHeatPump.IdleVolumeAirRate = 0.0;
                }

                BaseSizer::reportSizerOutput(state,
                                             state.dataHVACMultiSpdHP->CurrentModuleObject,
                                             MSHeatPump.Name,
                                             "Supply Air Flow Rate When No Cooling or Heating is Needed [m3/s]",
                                             MSHeatPump.IdleVolumeAirRate);
            }
        }

        if (MSHeatPump.SuppMaxAirTemp == DataSizing::AutoSize) {
            if (state.dataSize->CurSysNum > 0) {
                if (MSHeatPump.SuppHeatCoilType == HVAC::Coil_HeatingGasOrOtherFuel) { // Gas
                    CheckZoneSizing(state, "Coil:Heating:Fuel", MSHeatPump.Name);
                } else {
                    CheckZoneSizing(state, "Coil:Heating:Electric", MSHeatPump.Name);
                }
                MSHeatPump.SuppMaxAirTemp = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).HeatSupTemp;
                BaseSizer::reportSizerOutput(state,
                                             state.dataHVACMultiSpdHP->CurrentModuleObject,
                                             MSHeatPump.Name,
                                             "Maximum Supply Air Temperature from Supplemental Heater [C]",
                                             MSHeatPump.SuppMaxAirTemp);
            }
        }

        if (MSHeatPump.DesignSuppHeatingCapacity == DataSizing::AutoSize) {
            if (state.dataSize->CurSysNum > 0) {
                if (MSHeatPump.SuppHeatCoilType == HVAC::Coil_HeatingGasOrOtherFuel) { // Gas
                    CheckSysSizing(state, "Coil:Heating:Fuel", MSHeatPump.Name);
                } else {
                    CheckSysSizing(state, "Coil:Heating:Electric", MSHeatPump.Name);
                }
                MSHeatPump.DesignSuppHeatingCapacity = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).HeatCap;
            } else {
                MSHeatPump.DesignSuppHeatingCapacity = 0.0;
            }
            BaseSizer::reportSizerOutput(state,
                                         state.dataHVACMultiSpdHP->CurrentModuleObject,
                                         MSHeatPump.Name,
                                         "Supplemental Heating Coil Nominal Capacity [W]",
                                         MSHeatPump.DesignSuppHeatingCapacity);
        }
        state.dataSize->SuppHeatCap = MSHeatPump.DesignSuppHeatingCapacity;

        if (MSHeatPump.HeatRecActive) {
            PlantUtilities::RegisterPlantCompDesignFlow(state, MSHeatPump.HeatRecInletNodeNum, MSHeatPump.DesignHeatRecFlowRate);
        }
    }

    //******************************************************************************

    void ControlMSHPOutputEMS(EnergyPlusData &state,
                              int const MSHeatPumpNum,               // Unit index of engine driven heat pump
                              bool const FirstHVACIteration,         // flag for 1st HVAC iteration in the time step
                              HVAC::CompressorOp const compressorOp, // compressor operation; 1=on, 0=off
                              HVAC::FanOp const fanOp,               // operating mode: FanOp::Cycling | FanOp::Continuous
                              Real64 const QZnReq,                   // cooling or heating output needed by zone [W]
                              Real64 const SpeedVal,                 // continuous speed value
                              int &SpeedNum,                         // discrete speed level
                              Real64 &SpeedRatio,                    // unit speed ratio for DX coils
                              Real64 &PartLoadFrac,                  // unit part load fraction
                              Real64 &OnOffAirFlowRatio,             // ratio of compressor ON airflow to AVERAGE airflow over timestep
                              Real64 &SupHeaterLoad                  // Supplemental heater load [W]

    )
    {
        OnOffAirFlowRatio = 0.0;
        SupHeaterLoad = 0.0;

        auto &MSHeatPump = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);

        // Get EMS output
        SpeedNum = ceil(SpeedVal);
        bool useMaxedSpeed = false;
        std::string useMaxedSpeedCoilName;
        if (MSHeatPump.HeatCoolMode == ModeOfOperation::HeatingMode) {
            if (SpeedNum > MSHeatPump.NumOfSpeedHeating) {
                SpeedNum = MSHeatPump.NumOfSpeedHeating;
                useMaxedSpeed = true;
                useMaxedSpeedCoilName = MSHeatPump.DXHeatCoilName;
            }
        } else if (MSHeatPump.HeatCoolMode == ModeOfOperation::CoolingMode) {
            if (SpeedNum > MSHeatPump.NumOfSpeedCooling) {
                SpeedNum = MSHeatPump.NumOfSpeedCooling;
                useMaxedSpeed = true;
                useMaxedSpeedCoilName = MSHeatPump.DXCoolCoilName;
            }
        }
        if (useMaxedSpeed) {
            MSHeatPump.CoilSpeedErrIndex++;
            ShowRecurringWarningErrorAtEnd(state,
                                           "Wrong coil speed EMS override value, for unit=\"" + useMaxedSpeedCoilName +
                                               "\". Exceeding maximum coil speed level. Speed level is set to the maximum coil speed level allowed.",
                                           MSHeatPump.CoilSpeedErrIndex,
                                           SpeedVal,
                                           SpeedVal,
                                           _,
                                           "",
                                           "");
        }
        // Calculate TempOutput
        Real64 TempOutput = 0.0; // unit output when iteration limit exceeded [W]

        if (SpeedNum == 1) {
            SpeedRatio = 0.0;
            if (useMaxedSpeed || floor(SpeedVal) == SpeedVal) {
                PartLoadFrac = 1;
            } else {
                PartLoadFrac = SpeedVal - floor(SpeedVal);
            }
            CalcMSHeatPump(state,
                           MSHeatPumpNum,
                           FirstHVACIteration,
                           compressorOp,
                           SpeedNum,
                           SpeedRatio,
                           PartLoadFrac,
                           TempOutput,
                           QZnReq,
                           OnOffAirFlowRatio,
                           SupHeaterLoad);
        } else {
            PartLoadFrac = 0.0;
            if (useMaxedSpeed || floor(SpeedVal) == SpeedVal) {
                SpeedRatio = 1;
            } else {
                SpeedRatio = SpeedVal - floor(SpeedVal);
            }
            CalcMSHeatPump(state,
                           MSHeatPumpNum,
                           FirstHVACIteration,
                           compressorOp,
                           SpeedNum,
                           SpeedRatio,
                           PartLoadFrac,
                           TempOutput,
                           QZnReq,
                           OnOffAirFlowRatio,
                           SupHeaterLoad);
        }

        ControlMSHPSupHeater(state,
                             MSHeatPumpNum,
                             FirstHVACIteration,
                             compressorOp,
                             fanOp,
                             QZnReq,
                             TempOutput,
                             SpeedNum,
                             SpeedRatio,
                             PartLoadFrac,
                             OnOffAirFlowRatio,
                             SupHeaterLoad);
        state.dataHVACMultiSpdHP->MSHeatPumpReport(MSHeatPumpNum).CycRatio = PartLoadFrac;
        state.dataHVACMultiSpdHP->MSHeatPumpReport(MSHeatPumpNum).SpeedRatio = SpeedRatio;
        state.dataHVACMultiSpdHP->MSHeatPumpReport(MSHeatPumpNum).SpeedNum = SpeedNum;
    }

    void ControlMSHPSupHeater(EnergyPlusData &state,
                              int const MSHeatPumpNum,               // Unit index of engine driven heat pump
                              bool const FirstHVACIteration,         // flag for 1st HVAC iteration in the time step
                              HVAC::CompressorOp const compressorOp, // compressor operation; 1=on, 0=off
                              HVAC::FanOp const fanOp,               // operating mode: FanOp::Cycling | FanOp::Continuous
                              Real64 const QZnReq,                   // cooling or heating output needed by zone [W]
                              int const EMSOutput,                   // unit full output when compressor is operating [W]
                              int const SpeedNum,                    // Speed number
                              Real64 SpeedRatio,                     // unit speed ratio for DX coils
                              Real64 PartLoadFrac,                   // unit part load fraction
                              Real64 OnOffAirFlowRatio,              // ratio of compressor ON airflow to AVERAGE airflow over timestep
                              Real64 &SupHeaterLoad                  // Supplemental heater load [W]

    )
    {
        // if the DX heating coil cannot meet the load, trim with supplemental heater
        // occurs with constant fan mode when compressor is on or off
        // occurs with cycling fan mode when compressor PLR is equal to 1
        auto &MSHeatPump = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);

        if ((QZnReq > HVAC::SmallLoad && QZnReq > EMSOutput)) {
            Real64 TempOutput;
            if (state.dataEnvrn->OutDryBulbTemp <= MSHeatPump.SuppMaxAirTemp) {
                SupHeaterLoad = QZnReq - EMSOutput;
            } else {
                SupHeaterLoad = 0.0;
            }
            CalcMSHeatPump(state,
                           MSHeatPumpNum,
                           FirstHVACIteration,
                           compressorOp,
                           SpeedNum,
                           SpeedRatio,
                           PartLoadFrac,
                           TempOutput,
                           QZnReq,
                           OnOffAirFlowRatio,
                           SupHeaterLoad);
        }

        // check the outlet of the supplemental heater to be lower than the maximum supplemental heater supply air temperature
        if (state.dataLoopNodes->Node(MSHeatPump.AirOutletNodeNum).Temp > MSHeatPump.SuppMaxAirTemp && SupHeaterLoad > 0.0) {

            //   If the supply air temperature is to high, turn off the supplemental heater to recalculate the outlet temperature
            SupHeaterLoad = 0.0;
            Real64 QCoilActual; // coil load actually delivered returned to calling component
            CalcNonDXHeatingCoils(state, MSHeatPumpNum, FirstHVACIteration, SupHeaterLoad, fanOp, QCoilActual);

            //   If the outlet temperature is below the maximum supplemental heater supply air temperature, reduce the load passed to
            //   the supplemental heater, otherwise leave the supplemental heater off. If the supplemental heater is to be turned on,
            //   use the outlet conditions when the supplemental heater was off (CALL above) as the inlet conditions for the calculation
            //   of supplemental heater load to just meet the maximum supply air temperature from the supplemental heater.
            if (state.dataLoopNodes->Node(MSHeatPump.AirOutletNodeNum).Temp < MSHeatPump.SuppMaxAirTemp) {
                Real64 CpAir = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(MSHeatPump.AirOutletNodeNum).HumRat);
                SupHeaterLoad = state.dataLoopNodes->Node(MSHeatPump.AirInletNodeNum).MassFlowRate * CpAir *
                                (MSHeatPump.SuppMaxAirTemp - state.dataLoopNodes->Node(MSHeatPump.AirOutletNodeNum).Temp);

            } else {
                SupHeaterLoad = 0.0;
            }
        }
    }

    void ControlMSHPOutput(EnergyPlusData &state,
                           int const MSHeatPumpNum,               // Unit index of engine driven heat pump
                           bool const FirstHVACIteration,         // flag for 1st HVAC iteration in the time step
                           HVAC::CompressorOp const compressorOp, // compressor operation; 1=on, 0=off
                           HVAC::FanOp const fanOp,               // operating mode: FanOp::Cycling | FanOp::Continuous
                           Real64 const QZnReq,                   // cooling or heating output needed by zone [W]
                           int const ZoneNum [[maybe_unused]],    // Index to zone number
                           int &SpeedNum,                         // Speed number
                           Real64 &SpeedRatio,                    // unit speed ratio for DX coils
                           Real64 &PartLoadFrac,                  // unit part load fraction
                           Real64 &OnOffAirFlowRatio,             // ratio of compressor ON airflow to AVERAGE airflow over timestep
                           Real64 &SupHeaterLoad                  // Supplemental heater load [W]
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing Gu
        //       DATE WRITTEN   June 2007
        //       RE-ENGINEERED  Revised for multispeed heat pump use based on ControlPTHPOutput

        // PURPOSE OF THIS SUBROUTINE:
        // Determine the part load fraction at low speed, and speed ratio at high speed for this time step.

        // METHODOLOGY EMPLOYED:
        // Use RegulaFalsi technique to iterate on part-load ratio until convergence is achieved.

        // SUBROUTINE PARAMETER DEFINITIONS:
        int constexpr MaxIte(500); // maximum number of iterations

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 FullOutput;         // unit full output when compressor is operating [W]
        Real64 LowOutput;          // unit full output at low speed [W]
        Real64 TempOutput;         // unit output when iteration limit exceeded [W]
        Real64 NoCompOutput;       // output when no active compressor [W]
        Real64 ErrorToler;         // error tolerance
        int SolFla;                // Flag of RegulaFalsi solver
        Real64 CpAir;              // air specific heat
        Real64 OutsideDryBulbTemp; // Outside air temperature at external node height
        Real64 QCoilActual;        // coil load actually delivered returned to calling component
        int i;                     // Speed index

        SupHeaterLoad = 0.0;
        PartLoadFrac = 0.0;
        SpeedRatio = 0.0;
        SpeedNum = 1;

        OutsideDryBulbTemp = state.dataEnvrn->OutDryBulbTemp;

        auto &MSHeatPump = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);

        //!!LKL Discrepancy with < 0
        if (MSHeatPump.availSched->getCurrentVal() == 0.0) {
            return;
        }

        // Get result when DX coil is off
        CalcMSHeatPump(state,
                       MSHeatPumpNum,
                       FirstHVACIteration,
                       compressorOp,
                       SpeedNum,
                       SpeedRatio,
                       PartLoadFrac,
                       NoCompOutput,
                       QZnReq,
                       OnOffAirFlowRatio,
                       SupHeaterLoad);

        // If cooling and NoCompOutput < QZnReq, the coil needs to be off
        // If heating and NoCompOutput > QZnReq, the coil needs to be off
        if ((QZnReq < (-1.0 * HVAC::SmallLoad) && NoCompOutput < QZnReq) || (QZnReq > HVAC::SmallLoad && NoCompOutput > QZnReq) ||
            std::abs(QZnReq) <= HVAC::SmallLoad) {
            return;
        }

        // Get full load result
        PartLoadFrac = 1.0;
        SpeedRatio = 1.0;
        if (MSHeatPump.HeatCoolMode == ModeOfOperation::HeatingMode) {
            SpeedNum = MSHeatPump.NumOfSpeedHeating;
            if (MSHeatPump.Staged && std::abs(MSHeatPump.StageNum) < SpeedNum) {
                SpeedNum = std::abs(MSHeatPump.StageNum);
                if (SpeedNum == 1) {
                    SpeedRatio = 0.0;
                }
            }
        }
        if (MSHeatPump.HeatCoolMode == ModeOfOperation::CoolingMode) {
            SpeedNum = MSHeatPump.NumOfSpeedCooling;
            if (MSHeatPump.Staged && std::abs(MSHeatPump.StageNum) < SpeedNum) {
                SpeedNum = std::abs(MSHeatPump.StageNum);
                if (SpeedNum == 1) {
                    SpeedRatio = 0.0;
                }
            }
        }

        CalcMSHeatPump(state,
                       MSHeatPumpNum,
                       FirstHVACIteration,
                       compressorOp,
                       SpeedNum,
                       SpeedRatio,
                       PartLoadFrac,
                       FullOutput,
                       QZnReq,
                       OnOffAirFlowRatio,
                       SupHeaterLoad);

        if (QZnReq < (-1.0 * HVAC::SmallLoad)) {
            // Since we are cooling, we expect FullOutput to be < 0 and FullOutput < NoCompOutput
            // Check that this is the case; if not set PartLoadFrac = 0.0 (off) and return
            if (FullOutput >= 0.0 || FullOutput >= NoCompOutput) {
                PartLoadFrac = 0.0;
                SpeedRatio = 0.0;
                SpeedNum = 0;
                return;
            }
            //  ! If the QZnReq <= FullOutput the unit needs to run full out
            if (QZnReq <= FullOutput) {
                PartLoadFrac = 1.0;
                SpeedRatio = 1.0;
                if (MSHeatPump.Staged && SpeedNum == 1) {
                    SpeedRatio = 0.0;
                }
                state.dataHVACMultiSpdHP->MSHeatPumpReport(MSHeatPumpNum).CycRatio = PartLoadFrac;
                state.dataHVACMultiSpdHP->MSHeatPumpReport(MSHeatPumpNum).SpeedRatio = SpeedRatio;
                state.dataHVACMultiSpdHP->MSHeatPumpReport(MSHeatPumpNum).SpeedNum = SpeedNum;
                return;
            }
            ErrorToler = 0.001; // Error tolerance for convergence from input deck
        } else {
            // Since we are heating, we expect FullOutput to be > 0 and FullOutput > NoCompOutput
            // Check that this is the case; if not set PartLoadFrac = 0.0 (off)
            if (FullOutput <= 0.0 || FullOutput <= NoCompOutput) {
                PartLoadFrac = 0.0;
                SpeedRatio = 0.0;
                // may need supplemental heating so don't return in heating mode
            }
            if (QZnReq >= FullOutput) {
                PartLoadFrac = 1.0;
                SpeedRatio = 1.0;
                // may need supplemental heating so don't return in heating mode
            }
            ErrorToler = 0.001; // Error tolerance for convergence from input deck
        }

        // Direct solution
        if (state.dataGlobal->DoCoilDirectSolutions && !MSHeatPump.Staged) {
            Real64 TempOutput0 = 0.0;
            MSHeatPump.FullOutput = 0.0;

            // heating
            if (QZnReq > HVAC::SmallLoad && QZnReq < FullOutput) {
                CalcMSHeatPump(
                    state, MSHeatPumpNum, FirstHVACIteration, compressorOp, 1, 0.0, 0.0, TempOutput0, QZnReq, OnOffAirFlowRatio, SupHeaterLoad);

                for (int k = 1; k <= MSHeatPump.NumOfSpeedHeating; ++k) {
                    if (k == 1) {
                        CalcMSHeatPump(state,
                                       MSHeatPumpNum,
                                       FirstHVACIteration,
                                       compressorOp,
                                       k,
                                       0.0,
                                       1.0,
                                       MSHeatPump.FullOutput(k),
                                       QZnReq,
                                       OnOffAirFlowRatio,
                                       SupHeaterLoad);
                        if (QZnReq <= MSHeatPump.FullOutput(k)) {
                            SpeedNum = k;
                            PartLoadFrac = (QZnReq - TempOutput0) / (MSHeatPump.FullOutput(k) - TempOutput0);
                            CalcMSHeatPump(state,
                                           MSHeatPumpNum,
                                           FirstHVACIteration,
                                           compressorOp,
                                           k,
                                           0.0,
                                           PartLoadFrac,
                                           TempOutput,
                                           QZnReq,
                                           OnOffAirFlowRatio,
                                           SupHeaterLoad);
                            break;
                        }
                    } else {
                        CalcMSHeatPump(state,
                                       MSHeatPumpNum,
                                       FirstHVACIteration,
                                       compressorOp,
                                       k,
                                       1.0,
                                       1.0,
                                       MSHeatPump.FullOutput(k),
                                       QZnReq,
                                       OnOffAirFlowRatio,
                                       SupHeaterLoad);
                        if (QZnReq <= MSHeatPump.FullOutput(k)) {
                            SpeedNum = k;
                            PartLoadFrac = 1.0;
                            SpeedRatio = (QZnReq - MSHeatPump.FullOutput(k - 1)) / (MSHeatPump.FullOutput(k) - MSHeatPump.FullOutput(k - 1));
                            CalcMSHeatPump(state,
                                           MSHeatPumpNum,
                                           FirstHVACIteration,
                                           compressorOp,
                                           k,
                                           SpeedRatio,
                                           1.0,
                                           TempOutput,
                                           QZnReq,
                                           OnOffAirFlowRatio,
                                           SupHeaterLoad);
                            break;
                        }
                    }
                }
            }

            // Cooling
            if (QZnReq < (-1.0 * HVAC::SmallLoad) && QZnReq > FullOutput) {
                CalcMSHeatPump(
                    state, MSHeatPumpNum, FirstHVACIteration, compressorOp, 1, 0.0, 0.0, TempOutput0, QZnReq, OnOffAirFlowRatio, SupHeaterLoad);
                for (int k = 1; k <= MSHeatPump.NumOfSpeedCooling; ++k) {
                    if (k == 1) {
                        CalcMSHeatPump(state,
                                       MSHeatPumpNum,
                                       FirstHVACIteration,
                                       compressorOp,
                                       k,
                                       0.0,
                                       1.0,
                                       MSHeatPump.FullOutput(k),
                                       QZnReq,
                                       OnOffAirFlowRatio,
                                       SupHeaterLoad);
                        if (QZnReq >= MSHeatPump.FullOutput(k)) {
                            SpeedNum = k;
                            PartLoadFrac = (QZnReq - TempOutput0) / (MSHeatPump.FullOutput(k) - TempOutput0);
                            CalcMSHeatPump(state,
                                           MSHeatPumpNum,
                                           FirstHVACIteration,
                                           compressorOp,
                                           k,
                                           0.0,
                                           PartLoadFrac,
                                           TempOutput,
                                           QZnReq,
                                           OnOffAirFlowRatio,
                                           SupHeaterLoad);
                            break;
                        }
                    } else {
                        CalcMSHeatPump(state,
                                       MSHeatPumpNum,
                                       FirstHVACIteration,
                                       compressorOp,
                                       k,
                                       1.0,
                                       1.0,
                                       MSHeatPump.FullOutput(k),
                                       QZnReq,
                                       OnOffAirFlowRatio,
                                       SupHeaterLoad);
                        if (QZnReq >= MSHeatPump.FullOutput(k)) {
                            SpeedNum = k;
                            PartLoadFrac = 1.0;
                            SpeedRatio = (QZnReq - MSHeatPump.FullOutput(k - 1)) / (MSHeatPump.FullOutput(k) - MSHeatPump.FullOutput(k - 1));
                            CalcMSHeatPump(state,
                                           MSHeatPumpNum,
                                           FirstHVACIteration,
                                           compressorOp,
                                           k,
                                           SpeedRatio,
                                           1.0,
                                           TempOutput,
                                           QZnReq,
                                           OnOffAirFlowRatio,
                                           SupHeaterLoad);
                            break;
                        }
                    }
                }
            }
        } else {
            // Calculate the part load fraction
            if (((QZnReq > HVAC::SmallLoad && QZnReq < FullOutput) || (QZnReq < (-1.0 * HVAC::SmallLoad) && QZnReq > FullOutput)) &&
                (!MSHeatPump.Staged)) {
                // Check whether the low speed coil can meet the load or not
                CalcMSHeatPump(
                    state, MSHeatPumpNum, FirstHVACIteration, compressorOp, 1, 0.0, 1.0, LowOutput, QZnReq, OnOffAirFlowRatio, SupHeaterLoad);
                if ((QZnReq > 0.0 && QZnReq <= LowOutput) || (QZnReq < 0.0 && QZnReq >= LowOutput)) {
                    SpeedRatio = 0.0;
                    SpeedNum = 1;
                    auto f = [&state, MSHeatPumpNum, FirstHVACIteration, QZnReq, OnOffAirFlowRatio, SupHeaterLoad, compressorOp](
                                 Real64 const PartLoadFrac) {
                        //  Calculates residual function ((ActualOutput - QZnReq)/QZnReq); MSHP output depends on PLR which is being varied to zero
                        //  the residual.
                        Real64 ActualOutput; // delivered capacity of MSHP
                        Real64 tmpAirFlowRatio = OnOffAirFlowRatio;
                        Real64 tmpHeaterLoad = SupHeaterLoad;
                        CalcMSHeatPump(state,
                                       MSHeatPumpNum,
                                       FirstHVACIteration,
                                       compressorOp,
                                       1,
                                       0.0,
                                       PartLoadFrac,
                                       ActualOutput,
                                       QZnReq,
                                       tmpAirFlowRatio,
                                       tmpHeaterLoad);
                        return (ActualOutput - QZnReq) / QZnReq;
                    };
                    General::SolveRoot(state, ErrorToler, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                    if (SolFla == -1) {
                        if (!state.dataGlobal->WarmupFlag) {
                            if (state.dataHVACMultiSpdHP->ErrCountCyc == 0) {
                                ++state.dataHVACMultiSpdHP->ErrCountCyc; // TODO: Why is the error count shared among all heat pump units?
                                ShowWarningError(state,
                                                 format("Iteration limit exceeded calculating DX unit cycling ratio, for unit={}", MSHeatPump.Name));
                                ShowContinueErrorTimeStamp(state, format("Cycling ratio returned={:.2R}", PartLoadFrac));
                            } else {
                                ++state.dataHVACMultiSpdHP->ErrCountCyc;
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    MSHeatPump.Name + "\": Iteration limit warning exceeding calculating DX unit cycling ratio  continues...",
                                    MSHeatPump.ErrIndexCyc,
                                    PartLoadFrac,
                                    PartLoadFrac);
                            }
                        }
                    } else if (SolFla == -2) {
                        ShowFatalError(
                            state,
                            format("DX unit cycling ratio calculation failed: cycling limits exceeded, for unit={}", MSHeatPump.DXCoolCoilName));
                    }
                } else {
                    // Check to see which speed to meet the load
                    PartLoadFrac = 1.0;
                    SpeedRatio = 1.0;
                    if (QZnReq < (-1.0 * HVAC::SmallLoad)) { // Cooling
                        for (i = 2; i <= MSHeatPump.NumOfSpeedCooling; ++i) {
                            CalcMSHeatPump(state,
                                           MSHeatPumpNum,
                                           FirstHVACIteration,
                                           compressorOp,
                                           i,
                                           SpeedRatio,
                                           PartLoadFrac,
                                           TempOutput,
                                           QZnReq,
                                           OnOffAirFlowRatio,
                                           SupHeaterLoad);
                            if (QZnReq >= TempOutput) {
                                SpeedNum = i;
                                break;
                            }
                        }
                    } else {
                        for (i = 2; i <= MSHeatPump.NumOfSpeedHeating; ++i) {
                            CalcMSHeatPump(state,
                                           MSHeatPumpNum,
                                           FirstHVACIteration,
                                           compressorOp,
                                           i,
                                           SpeedRatio,
                                           PartLoadFrac,
                                           TempOutput,
                                           QZnReq,
                                           OnOffAirFlowRatio,
                                           SupHeaterLoad);
                            if (QZnReq <= TempOutput) {
                                SpeedNum = i;
                                break;
                            }
                        }
                    }
                    auto f = [&state, OnOffAirFlowRatio, SupHeaterLoad, MSHeatPumpNum, FirstHVACIteration, compressorOp, SpeedNum, QZnReq](
                                 Real64 const SpeedRatio) {
                        //  Calculates residual function ((ActualOutput - QZnReq)/QZnReq) MSHP output depends on PLR which is being varied to zero the
                        //  residual.
                        Real64 localAirFlowRatio = OnOffAirFlowRatio;
                        Real64 localHeaterLoad = SupHeaterLoad;
                        Real64 ActualOutput;
                        CalcMSHeatPump(state,
                                       MSHeatPumpNum,
                                       FirstHVACIteration,
                                       compressorOp,
                                       SpeedNum,
                                       SpeedRatio,
                                       1.0,
                                       ActualOutput,
                                       QZnReq,
                                       localAirFlowRatio,
                                       localHeaterLoad);
                        return (ActualOutput - QZnReq) / QZnReq;
                    };
                    General::SolveRoot(state, ErrorToler, MaxIte, SolFla, SpeedRatio, f, 0.0, 1.0);
                    if (SolFla == -1) {
                        if (!state.dataGlobal->WarmupFlag) {
                            if (state.dataHVACMultiSpdHP->ErrCountVar == 0) {
                                ++state.dataHVACMultiSpdHP->ErrCountVar;
                                ShowWarningError(state,
                                                 format("Iteration limit exceeded calculating DX unit speed ratio, for unit={}", MSHeatPump.Name));
                                ShowContinueErrorTimeStamp(state, format("Speed ratio returned=[{:.2R}], Speed number ={}", SpeedRatio, SpeedNum));
                            } else {
                                ++state.dataHVACMultiSpdHP->ErrCountVar;
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    MSHeatPump.Name + "\": Iteration limit warning exceeding calculating DX unit speed ratio continues...",
                                    MSHeatPump.ErrIndexVar,
                                    SpeedRatio,
                                    SpeedRatio);
                            }
                        }
                    } else if (SolFla == -2) {
                        ShowFatalError(
                            state,
                            format("DX unit compressor speed calculation failed: speed limits exceeded, for unit={}", MSHeatPump.DXCoolCoilName));
                    }
                }
            } else {
                // Staged thermostat performance
                if (MSHeatPump.StageNum != 0) {
                    SpeedNum = std::abs(MSHeatPump.StageNum);
                    if (SpeedNum == 1) {
                        CalcMSHeatPump(
                            state, MSHeatPumpNum, FirstHVACIteration, compressorOp, 1, 0.0, 1.0, LowOutput, QZnReq, OnOffAirFlowRatio, SupHeaterLoad);
                        SpeedRatio = 0.0;
                        if ((QZnReq > 0.0 && QZnReq <= LowOutput) || (QZnReq < 0.0 && QZnReq >= LowOutput)) {
                            auto f = [&state, MSHeatPumpNum, FirstHVACIteration, QZnReq, OnOffAirFlowRatio, SupHeaterLoad, compressorOp](
                                         Real64 const PartLoadFrac) {
                                //  Calculates residual function ((ActualOutput - QZnReq)/QZnReq); MSHP output depends on PLR which is being varied to
                                //  zero the residual.
                                Real64 ActualOutput; // delivered capacity of MSHP
                                Real64 tmpAirFlowRatio = OnOffAirFlowRatio;
                                Real64 tmpHeaterLoad = SupHeaterLoad;
                                CalcMSHeatPump(state,
                                               MSHeatPumpNum,
                                               FirstHVACIteration,
                                               compressorOp,
                                               1,
                                               0.0,
                                               PartLoadFrac,
                                               ActualOutput,
                                               QZnReq,
                                               tmpAirFlowRatio,
                                               tmpHeaterLoad);
                                return (ActualOutput - QZnReq) / QZnReq;
                            };
                            General::SolveRoot(state, ErrorToler, MaxIte, SolFla, PartLoadFrac, f, 0.0, 1.0);
                            if (SolFla == -1) {
                                if (!state.dataGlobal->WarmupFlag) {
                                    if (state.dataHVACMultiSpdHP->ErrCountCyc == 0) {
                                        ++state.dataHVACMultiSpdHP->ErrCountCyc;
                                        ShowWarningError(
                                            state,
                                            format("Iteration limit exceeded calculating DX unit cycling ratio, for unit={}", MSHeatPump.Name));
                                        ShowContinueErrorTimeStamp(state, format("Cycling ratio returned={:.2R}", PartLoadFrac));
                                    } else {
                                        ++state.dataHVACMultiSpdHP->ErrCountCyc;
                                        ShowRecurringWarningErrorAtEnd(
                                            state,
                                            MSHeatPump.Name + "\": Iteration limit warning exceeding calculating DX unit cycling ratio  continues...",
                                            MSHeatPump.ErrIndexCyc,
                                            PartLoadFrac,
                                            PartLoadFrac);
                                    }
                                }
                            } else if (SolFla == -2) {
                                ShowFatalError(state,
                                               format("DX unit cycling ratio calculation failed: cycling limits exceeded, for unit={}",
                                                      MSHeatPump.DXCoolCoilName));
                            }
                        } else {
                            FullOutput = LowOutput;
                            PartLoadFrac = 1.0;
                        }
                    } else {
                        if (MSHeatPump.StageNum < 0) {
                            SpeedNum = min(MSHeatPump.NumOfSpeedCooling, std::abs(MSHeatPump.StageNum));
                        } else {
                            SpeedNum = min(MSHeatPump.NumOfSpeedHeating, std::abs(MSHeatPump.StageNum));
                        }
                        CalcMSHeatPump(state,
                                       MSHeatPumpNum,
                                       FirstHVACIteration,
                                       compressorOp,
                                       SpeedNum,
                                       0.0,
                                       1.0,
                                       LowOutput,
                                       QZnReq,
                                       OnOffAirFlowRatio,
                                       SupHeaterLoad);
                        if ((QZnReq > 0.0 && QZnReq >= LowOutput) || (QZnReq < 0.0 && QZnReq <= LowOutput)) {
                            CalcMSHeatPump(state,
                                           MSHeatPumpNum,
                                           FirstHVACIteration,
                                           compressorOp,
                                           SpeedNum,
                                           1.0,
                                           1.0,
                                           FullOutput,
                                           QZnReq,
                                           OnOffAirFlowRatio,
                                           SupHeaterLoad);
                            if ((QZnReq > 0.0 && QZnReq <= FullOutput) || (QZnReq < 0.0 && QZnReq >= FullOutput)) {
                                auto f = // (AUTO_OK_LAMBDA)
                                    [&state, OnOffAirFlowRatio, SupHeaterLoad, MSHeatPumpNum, FirstHVACIteration, compressorOp, SpeedNum, QZnReq](
                                        Real64 const SpeedRatio) {
                                        //  Calculates residual function ((ActualOutput - QZnReq)/QZnReq) MSHP output depends on PLR which is being
                                        //  varied to zero the residual.
                                        Real64 localAirFlowRatio = OnOffAirFlowRatio;
                                        Real64 localHeaterLoad = SupHeaterLoad;
                                        Real64 ActualOutput;
                                        CalcMSHeatPump(state,
                                                       MSHeatPumpNum,
                                                       FirstHVACIteration,
                                                       compressorOp,
                                                       SpeedNum,
                                                       SpeedRatio,
                                                       1.0,
                                                       ActualOutput,
                                                       QZnReq,
                                                       localAirFlowRatio,
                                                       localHeaterLoad);
                                        return (ActualOutput - QZnReq) / QZnReq;
                                    };
                                General::SolveRoot(state, ErrorToler, MaxIte, SolFla, SpeedRatio, f, 0.0, 1.0);
                                if (SolFla == -1) {
                                    if (!state.dataGlobal->WarmupFlag) {
                                        if (state.dataHVACMultiSpdHP->ErrCountVar == 0) {
                                            ++state.dataHVACMultiSpdHP->ErrCountVar;
                                            ShowWarningError(
                                                state,
                                                format("Iteration limit exceeded calculating DX unit speed ratio, for unit={}", MSHeatPump.Name));
                                            ShowContinueErrorTimeStamp(
                                                state, format("Speed ratio returned=[{:.2R}], Speed number ={}", SpeedRatio, SpeedNum));
                                        } else {
                                            ++state.dataHVACMultiSpdHP->ErrCountVar;
                                            ShowRecurringWarningErrorAtEnd(
                                                state,
                                                MSHeatPump.Name +
                                                    "\": Iteration limit warning exceeding calculating DX unit speed ratio continues...",
                                                MSHeatPump.ErrIndexVar,
                                                SpeedRatio,
                                                SpeedRatio);
                                        }
                                    }
                                } else if (SolFla == -2) {
                                    ShowFatalError(state,
                                                   format("DX unit compressor speed calculation failed: speed limits exceeded, for unit={}",
                                                          MSHeatPump.DXCoolCoilName));
                                }
                            } else {
                                SpeedRatio = 1.0;
                            }
                        } else { // lowOutput provides a larger capacity than needed
                            SpeedRatio = 0.0;
                        }
                    }
                }
            }
        }

        // if the DX heating coil cannot meet the load, trim with supplemental heater
        // occurs with constant fan mode when compressor is on or off
        // occurs with cycling fan mode when compressor PLR is equal to 1
        if ((QZnReq > HVAC::SmallLoad && QZnReq > FullOutput)) {
            PartLoadFrac = 1.0;
            SpeedRatio = 1.0;
            if (MSHeatPump.Staged && SpeedNum == 1) {
                SpeedRatio = 0.0;
            }
            if (OutsideDryBulbTemp <= MSHeatPump.SuppMaxAirTemp) {
                SupHeaterLoad = QZnReq - FullOutput;
            } else {
                SupHeaterLoad = 0.0;
            }
            CalcMSHeatPump(state,
                           MSHeatPumpNum,
                           FirstHVACIteration,
                           compressorOp,
                           SpeedNum,
                           SpeedRatio,
                           PartLoadFrac,
                           TempOutput,
                           QZnReq,
                           OnOffAirFlowRatio,
                           SupHeaterLoad);
        }

        // check the outlet of the supplemental heater to be lower than the maximum supplemental heater supply air temperature
        if (state.dataLoopNodes->Node(MSHeatPump.AirOutletNodeNum).Temp > MSHeatPump.SuppMaxAirTemp && SupHeaterLoad > 0.0) {

            //   If the supply air temperature is to high, turn off the supplemental heater to recalculate the outlet temperature
            SupHeaterLoad = 0.0;
            CalcNonDXHeatingCoils(state, MSHeatPumpNum, FirstHVACIteration, SupHeaterLoad, fanOp, QCoilActual);

            //   If the outlet temperature is below the maximum supplemental heater supply air temperature, reduce the load passed to
            //   the supplemental heater, otherwise leave the supplemental heater off. If the supplemental heater is to be turned on,
            //   use the outlet conditions when the supplemental heater was off (CALL above) as the inlet conditions for the calculation
            //   of supplemental heater load to just meet the maximum supply air temperature from the supplemental heater.
            if (state.dataLoopNodes->Node(MSHeatPump.AirOutletNodeNum).Temp < MSHeatPump.SuppMaxAirTemp) {
                CpAir = Psychrometrics::PsyCpAirFnW(state.dataLoopNodes->Node(MSHeatPump.AirOutletNodeNum).HumRat);
                SupHeaterLoad = state.dataLoopNodes->Node(MSHeatPump.AirInletNodeNum).MassFlowRate * CpAir *
                                (MSHeatPump.SuppMaxAirTemp - state.dataLoopNodes->Node(MSHeatPump.AirOutletNodeNum).Temp);

            } else {
                SupHeaterLoad = 0.0;
            }
        }

        state.dataHVACMultiSpdHP->MSHeatPumpReport(MSHeatPumpNum).CycRatio = PartLoadFrac;
        state.dataHVACMultiSpdHP->MSHeatPumpReport(MSHeatPumpNum).SpeedRatio = SpeedRatio;
        state.dataHVACMultiSpdHP->MSHeatPumpReport(MSHeatPumpNum).SpeedNum = SpeedNum;
    }

    //******************************************************************************

    void CalcMSHeatPump(EnergyPlusData &state,
                        int const MSHeatPumpNum,               // Engine driven heat pump number
                        bool const FirstHVACIteration,         // Flag for 1st HVAC iteration
                        HVAC::CompressorOp const compressorOp, // Compressor on/off; 1=on, 0=off
                        int const SpeedNum,                    // Speed number
                        Real64 const SpeedRatio,               // Compressor speed ratio
                        Real64 const PartLoadFrac,             // Compressor part load fraction
                        Real64 &LoadMet,                       // Load met by unit (W)
                        Real64 const QZnReq,                   // Zone load (W)
                        Real64 &OnOffAirFlowRatio,             // Ratio of compressor ON airflow to AVERAGE airflow over timestep
                        Real64 &SupHeaterLoad                  // supplemental heater load (W)
    )
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Lixing Gu, FSEC
        //       DATE WRITTEN:    June 2007

        // PURPOSE OF THIS SUBROUTINE:
        //  This routine will calculates MSHP performance based on given system load

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int OutletNode;            // MSHP air outlet node
        int InletNode;             // MSHP air inlet node
        Real64 OutsideDryBulbTemp; // Outdoor dry bulb temperature [C]
        Real64 AirMassFlow;        // Air mass flow rate [kg/s]
        Real64 SavePartloadRatio;
        Real64 SaveSpeedRatio;
        Real64 QCoilActual;  // coil load actually delivered returned to calling component
        Real64 MinWaterFlow; // minimum water flow rate
        Real64 ErrorToler;   // supplemental heating coil convergence tolerance

        auto &MSHeatPump = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);

        OutletNode = MSHeatPump.AirOutletNodeNum;
        InletNode = MSHeatPump.AirInletNodeNum;
        if (MSHeatPump.DXHeatCoilIndex > 0) {
            if (state.dataDXCoils->DXCoil(MSHeatPump.DXHeatCoilIndex).IsSecondaryDXCoilInZone) {
                OutsideDryBulbTemp =
                    state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataDXCoils->DXCoil(MSHeatPump.DXHeatCoilIndex).SecZonePtr).ZT;
            } else {
                OutsideDryBulbTemp = state.dataEnvrn->OutDryBulbTemp;
            }
        } else if (MSHeatPump.DXCoolCoilIndex > 0) {
            if (state.dataDXCoils->DXCoil(MSHeatPump.DXCoolCoilIndex).IsSecondaryDXCoilInZone) {
                OutsideDryBulbTemp =
                    state.dataZoneTempPredictorCorrector->zoneHeatBalance(state.dataDXCoils->DXCoil(MSHeatPump.DXCoolCoilIndex).SecZonePtr).ZT;
            } else {
                OutsideDryBulbTemp = state.dataEnvrn->OutDryBulbTemp;
            }
        } else {
            OutsideDryBulbTemp = state.dataEnvrn->OutDryBulbTemp;
        }

        state.dataHVACMultiSpdHP->SaveCompressorPLR = 0.0;
        SavePartloadRatio = 0.0;
        MinWaterFlow = 0.0;
        ErrorToler = 0.001;
        // Set inlet air mass flow rate based on PLR and compressor on/off air flow rates
        SetAverageAirFlow(state, MSHeatPumpNum, PartLoadFrac, OnOffAirFlowRatio, SpeedNum, SpeedRatio);

        AirMassFlow = state.dataLoopNodes->Node(InletNode).MassFlowRate;
        // if blow through, simulate fan then coils
        if (MSHeatPump.fanPlace == HVAC::FanPlace::BlowThru) {
            state.dataFans->fans(MSHeatPump.FanNum)->simulate(state, FirstHVACIteration, state.dataHVACMultiSpdHP->FanSpeedRatio);
            if (QZnReq < (-1.0 * HVAC::SmallLoad)) {
                if (OutsideDryBulbTemp > MSHeatPump.MinOATCompressorCooling) {
                    DXCoils::SimDXCoilMultiSpeed(state,
                                                 MSHeatPump.DXCoolCoilName,
                                                 SpeedRatio,
                                                 PartLoadFrac,
                                                 MSHeatPump.DXCoolCoilIndex,
                                                 SpeedNum,
                                                 MSHeatPump.fanOp,
                                                 compressorOp);
                    SavePartloadRatio = PartLoadFrac;
                    SaveSpeedRatio = SpeedRatio;
                } else {
                    DXCoils::SimDXCoilMultiSpeed(
                        state, MSHeatPump.DXCoolCoilName, 0.0, 0.0, MSHeatPump.DXCoolCoilIndex, SpeedNum, MSHeatPump.fanOp, compressorOp);
                }
                state.dataHVACMultiSpdHP->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(MSHeatPump.DXCoolCoilIndex);
            } else {
                DXCoils::SimDXCoilMultiSpeed(
                    state, MSHeatPump.DXCoolCoilName, 0.0, 0.0, MSHeatPump.DXCoolCoilIndex, SpeedNum, MSHeatPump.fanOp, compressorOp);
            }
            if (MSHeatPump.HeatCoilType == HVAC::CoilDX_MultiSpeedHeating) {
                if (QZnReq > HVAC::SmallLoad) {
                    if (OutsideDryBulbTemp > MSHeatPump.MinOATCompressorHeating) {
                        DXCoils::SimDXCoilMultiSpeed(state,
                                                     MSHeatPump.DXHeatCoilName,
                                                     SpeedRatio,
                                                     PartLoadFrac,
                                                     MSHeatPump.DXHeatCoilIndex,
                                                     SpeedNum,
                                                     MSHeatPump.fanOp,
                                                     compressorOp);
                        SavePartloadRatio = PartLoadFrac;
                        SaveSpeedRatio = SpeedRatio;
                    } else {
                        DXCoils::SimDXCoilMultiSpeed(
                            state, MSHeatPump.DXHeatCoilName, 0.0, 0.0, MSHeatPump.DXHeatCoilIndex, SpeedNum, MSHeatPump.fanOp, compressorOp);
                    }
                    state.dataHVACMultiSpdHP->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(MSHeatPump.DXHeatCoilIndex);
                } else {
                    DXCoils::SimDXCoilMultiSpeed(
                        state, MSHeatPump.DXHeatCoilName, 0.0, 0.0, MSHeatPump.DXHeatCoilIndex, SpeedNum, MSHeatPump.fanOp, compressorOp);
                }
            } else if (MSHeatPump.HeatCoilType == HVAC::Coil_HeatingElectric_MultiStage ||
                       MSHeatPump.HeatCoilType == HVAC::Coil_HeatingGas_MultiStage) {
                if (QZnReq > HVAC::SmallLoad) {
                    HeatingCoils::SimulateHeatingCoilComponents(
                        state, MSHeatPump.HeatCoilName, FirstHVACIteration, _, 0, _, _, MSHeatPump.fanOp, PartLoadFrac, SpeedNum, SpeedRatio);
                } else {
                    HeatingCoils::SimulateHeatingCoilComponents(
                        state, MSHeatPump.HeatCoilName, FirstHVACIteration, _, 0, _, _, MSHeatPump.fanOp, 0.0, SpeedNum, 0.0);
                }
            } else {
                CalcNonDXHeatingCoils(state, MSHeatPumpNum, FirstHVACIteration, QZnReq, MSHeatPump.fanOp, QCoilActual, PartLoadFrac);
            }
            // Call twice to ensure the fan outlet conditions are updated
            state.dataFans->fans(MSHeatPump.FanNum)->simulate(state, FirstHVACIteration, state.dataHVACMultiSpdHP->FanSpeedRatio);
            if (QZnReq < (-1.0 * HVAC::SmallLoad)) {
                if (OutsideDryBulbTemp > MSHeatPump.MinOATCompressorCooling) {
                    DXCoils::SimDXCoilMultiSpeed(state,
                                                 MSHeatPump.DXCoolCoilName,
                                                 SpeedRatio,
                                                 PartLoadFrac,
                                                 MSHeatPump.DXCoolCoilIndex,
                                                 SpeedNum,
                                                 MSHeatPump.fanOp,
                                                 compressorOp);
                    SavePartloadRatio = PartLoadFrac;
                    SaveSpeedRatio = SpeedRatio;
                } else {
                    DXCoils::SimDXCoilMultiSpeed(
                        state, MSHeatPump.DXCoolCoilName, 0.0, 0.0, MSHeatPump.DXCoolCoilIndex, SpeedNum, MSHeatPump.fanOp, compressorOp);
                }
                state.dataHVACMultiSpdHP->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(MSHeatPump.DXCoolCoilIndex);
            } else {
                DXCoils::SimDXCoilMultiSpeed(
                    state, MSHeatPump.DXCoolCoilName, 0.0, 0.0, MSHeatPump.DXCoolCoilIndex, SpeedNum, MSHeatPump.fanOp, compressorOp);
            }
            if (MSHeatPump.HeatCoilType == HVAC::CoilDX_MultiSpeedHeating) {
                if (QZnReq > HVAC::SmallLoad) {
                    if (OutsideDryBulbTemp > MSHeatPump.MinOATCompressorHeating) {
                        DXCoils::SimDXCoilMultiSpeed(state,
                                                     MSHeatPump.DXHeatCoilName,
                                                     SpeedRatio,
                                                     PartLoadFrac,
                                                     MSHeatPump.DXHeatCoilIndex,
                                                     SpeedNum,
                                                     MSHeatPump.fanOp,
                                                     compressorOp);
                        SavePartloadRatio = PartLoadFrac;
                        SaveSpeedRatio = SpeedRatio;
                    } else {
                        DXCoils::SimDXCoilMultiSpeed(
                            state, MSHeatPump.DXHeatCoilName, 0.0, 0.0, MSHeatPump.DXHeatCoilIndex, SpeedNum, MSHeatPump.fanOp, compressorOp);
                    }
                    state.dataHVACMultiSpdHP->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(MSHeatPump.DXHeatCoilIndex);
                } else {
                    DXCoils::SimDXCoilMultiSpeed(
                        state, MSHeatPump.DXHeatCoilName, 0.0, 0.0, MSHeatPump.DXHeatCoilIndex, SpeedNum, MSHeatPump.fanOp, compressorOp);
                }
            } else if (MSHeatPump.HeatCoilType == HVAC::Coil_HeatingElectric_MultiStage ||
                       MSHeatPump.HeatCoilType == HVAC::Coil_HeatingGas_MultiStage) {
                if (QZnReq > HVAC::SmallLoad) {
                    HeatingCoils::SimulateHeatingCoilComponents(
                        state, MSHeatPump.HeatCoilName, FirstHVACIteration, _, 0, _, _, MSHeatPump.fanOp, PartLoadFrac, SpeedNum, SpeedRatio);
                } else {
                    HeatingCoils::SimulateHeatingCoilComponents(
                        state, MSHeatPump.HeatCoilName, FirstHVACIteration, _, 0, _, _, MSHeatPump.fanOp, 0.0, SpeedNum, 0.0);
                }
            } else {
                CalcNonDXHeatingCoils(state, MSHeatPumpNum, FirstHVACIteration, QZnReq, MSHeatPump.fanOp, QCoilActual, PartLoadFrac);
            }
            //  Simulate supplemental heating coil for blow through fan
            if (MSHeatPump.SuppHeatCoilNum > 0) {
                CalcNonDXHeatingCoils(state, MSHeatPumpNum, FirstHVACIteration, SupHeaterLoad, MSHeatPump.fanOp, QCoilActual);
            }
        } else { // otherwise simulate DX coils then fan then supplemental heater
            if (QZnReq < (-1.0 * HVAC::SmallLoad)) {
                if (OutsideDryBulbTemp > MSHeatPump.MinOATCompressorCooling) {
                    DXCoils::SimDXCoilMultiSpeed(state,
                                                 MSHeatPump.DXCoolCoilName,
                                                 SpeedRatio,
                                                 PartLoadFrac,
                                                 MSHeatPump.DXCoolCoilIndex,
                                                 SpeedNum,
                                                 MSHeatPump.fanOp,
                                                 compressorOp);
                    SavePartloadRatio = PartLoadFrac;
                    SaveSpeedRatio = SpeedRatio;
                } else {
                    DXCoils::SimDXCoilMultiSpeed(
                        state, MSHeatPump.DXCoolCoilName, 0.0, 0.0, MSHeatPump.DXCoolCoilIndex, SpeedNum, MSHeatPump.fanOp, compressorOp);
                }
                state.dataHVACMultiSpdHP->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(MSHeatPump.DXCoolCoilIndex);
            } else {
                DXCoils::SimDXCoilMultiSpeed(
                    state, MSHeatPump.DXCoolCoilName, 0.0, 0.0, MSHeatPump.DXCoolCoilIndex, SpeedNum, MSHeatPump.fanOp, compressorOp);
            }
            if (MSHeatPump.HeatCoilType == HVAC::CoilDX_MultiSpeedHeating) {
                if (QZnReq > HVAC::SmallLoad) {
                    if (OutsideDryBulbTemp > MSHeatPump.MinOATCompressorHeating) {
                        DXCoils::SimDXCoilMultiSpeed(state,
                                                     MSHeatPump.DXHeatCoilName,
                                                     SpeedRatio,
                                                     PartLoadFrac,
                                                     MSHeatPump.DXHeatCoilIndex,
                                                     SpeedNum,
                                                     MSHeatPump.fanOp,
                                                     compressorOp);
                        SavePartloadRatio = PartLoadFrac;
                        SaveSpeedRatio = SpeedRatio;
                    } else {
                        DXCoils::SimDXCoilMultiSpeed(
                            state, MSHeatPump.DXHeatCoilName, 0.0, 0.0, MSHeatPump.DXHeatCoilIndex, SpeedNum, MSHeatPump.fanOp, compressorOp);
                    }
                    state.dataHVACMultiSpdHP->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(MSHeatPump.DXHeatCoilIndex);
                } else {
                    DXCoils::SimDXCoilMultiSpeed(
                        state, MSHeatPump.DXHeatCoilName, 0.0, 0.0, MSHeatPump.DXHeatCoilIndex, SpeedNum, MSHeatPump.fanOp, compressorOp);
                }
            } else if (MSHeatPump.HeatCoilType == HVAC::Coil_HeatingElectric_MultiStage ||
                       MSHeatPump.HeatCoilType == HVAC::Coil_HeatingGas_MultiStage) {
                if (QZnReq > HVAC::SmallLoad) {
                    HeatingCoils::SimulateHeatingCoilComponents(
                        state, MSHeatPump.HeatCoilName, FirstHVACIteration, _, 0, _, _, MSHeatPump.fanOp, PartLoadFrac, SpeedNum, SpeedRatio);
                } else {
                    HeatingCoils::SimulateHeatingCoilComponents(
                        state, MSHeatPump.HeatCoilName, FirstHVACIteration, _, 0, _, _, MSHeatPump.fanOp, 0.0, SpeedNum, 0.0);
                }
            } else {
                CalcNonDXHeatingCoils(state, MSHeatPumpNum, FirstHVACIteration, QZnReq, MSHeatPump.fanOp, QCoilActual, PartLoadFrac);
            }
            state.dataFans->fans(MSHeatPump.FanNum)->simulate(state, FirstHVACIteration, state.dataHVACMultiSpdHP->FanSpeedRatio);
            //  Simulate supplemental heating coil for draw through fan
            if (MSHeatPump.SuppHeatCoilNum > 0) {
                CalcNonDXHeatingCoils(state, MSHeatPumpNum, FirstHVACIteration, SupHeaterLoad, MSHeatPump.fanOp, QCoilActual);
            }
        }

        // calculate sensible load met
        Real64 SensibleOutput(0.0); // sensible output rate
        // calculate sensible load met using delta enthalpy at a constant (minimum) humidity ratio)
        Real64 MinHumRat = state.dataLoopNodes->Node(MSHeatPump.NodeNumOfControlledZone).HumRat;
        if (state.dataLoopNodes->Node(OutletNode).Temp < state.dataLoopNodes->Node(MSHeatPump.NodeNumOfControlledZone).Temp) {
            MinHumRat = state.dataLoopNodes->Node(OutletNode).HumRat;
        }
        SensibleOutput = AirMassFlow * Psychrometrics::PsyDeltaHSenFnTdb2W2Tdb1W1(state.dataLoopNodes->Node(OutletNode).Temp,
                                                                                  MinHumRat,
                                                                                  state.dataLoopNodes->Node(MSHeatPump.NodeNumOfControlledZone).Temp,
                                                                                  MinHumRat);
        LoadMet = SensibleOutput - MSHeatPump.LoadLoss;

        MSHeatPump.LoadMet = LoadMet;
    }

    void UpdateMSHeatPump(EnergyPlusData &state, int const MSHeatPumpNum) // Engine driven heat pump number
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Lixing Gu, FSEC
        //       DATE WRITTEN:    June 2007

        // PURPOSE OF THIS SUBROUTINE:
        //  This routine will update MSHP performance and calculate heat recovery rate and crankcase heater power

        // Calculate heat recovery
        if (state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).HeatRecActive) {
            MSHPHeatRecovery(state, MSHeatPumpNum);
        }

        if (state.afn->distribution_simulated) {
            state.dataAirLoop->AirLoopAFNInfo(state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).AirLoopNumber).LoopSystemOnMassFlowrate =
                state.dataHVACMultiSpdHP->CompOnMassFlow;
            state.dataAirLoop->AirLoopAFNInfo(state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).AirLoopNumber).LoopSystemOffMassFlowrate =
                state.dataHVACMultiSpdHP->CompOffMassFlow;
            state.dataAirLoop->AirLoopAFNInfo(state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).AirLoopNumber).LoopFanOperationMode =
                state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).fanOp;
            state.dataAirLoop->AirLoopAFNInfo(state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).AirLoopNumber).LoopOnOffFanPartLoadRatio =
                state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).FanPartLoadRatio;
            state.dataAirLoop->AirLoopAFNInfo(state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).AirLoopNumber).LoopCompCycRatio =
                state.dataHVACMultiSpdHP->MSHeatPumpReport(MSHeatPumpNum).CycRatio;
        }
    }

    //******************************************************************************

    void ReportMSHeatPump(EnergyPlusData &state, int const MSHeatPumpNum) // Engine driven heat pump number
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Lixing Gu, FSEC
        //       DATE WRITTEN:    June 2007

        // PURPOSE OF THIS SUBROUTINE:
        //  This routine will write values to output variables in MSHP

        Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

        auto &MSHeatPump = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);
        auto &MSHeatPumpReport = state.dataHVACMultiSpdHP->MSHeatPumpReport(MSHeatPumpNum);

        MSHeatPumpReport.ElecPowerConsumption = MSHeatPump.ElecPower * TimeStepSysSec; // + &
        MSHeatPumpReport.HeatRecoveryEnergy = MSHeatPump.HeatRecoveryRate * TimeStepSysSec;

        MSHeatPumpReport.AuxElecHeatConsumption = 0.0;
        MSHeatPumpReport.AuxElecCoolConsumption = 0.0;

        MSHeatPump.AuxElecPower = MSHeatPump.AuxOnCyclePower * state.dataHVACMultiSpdHP->SaveCompressorPLR +
                                  MSHeatPump.AuxOffCyclePower * (1.0 - state.dataHVACMultiSpdHP->SaveCompressorPLR);
        if (MSHeatPump.HeatCoolMode == ModeOfOperation::CoolingMode) {
            MSHeatPumpReport.AuxElecCoolConsumption = MSHeatPump.AuxOnCyclePower * state.dataHVACMultiSpdHP->SaveCompressorPLR * TimeStepSysSec;
        }
        if (MSHeatPump.HeatCoolMode == ModeOfOperation::HeatingMode) {
            MSHeatPumpReport.AuxElecHeatConsumption = MSHeatPump.AuxOnCyclePower * state.dataHVACMultiSpdHP->SaveCompressorPLR * TimeStepSysSec;
        }
        if (MSHeatPump.LastMode == ModeOfOperation::HeatingMode) {
            MSHeatPumpReport.AuxElecHeatConsumption +=
                MSHeatPump.AuxOffCyclePower * (1.0 - state.dataHVACMultiSpdHP->SaveCompressorPLR) * TimeStepSysSec;
        } else {
            MSHeatPumpReport.AuxElecCoolConsumption +=
                MSHeatPump.AuxOffCyclePower * (1.0 - state.dataHVACMultiSpdHP->SaveCompressorPLR) * TimeStepSysSec;
        }

        if (MSHeatPump.FirstPass) {
            if (!state.dataGlobal->SysSizingCalc) {
                DataSizing::resetHVACSizingGlobals(state, state.dataSize->CurZoneEqNum, state.dataSize->CurSysNum, MSHeatPump.FirstPass);
            }
        }

        // reset to 1 in case blow through fan configuration (fan resets to 1, but for blow thru fans coil sets back down < 1)
        state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;
    }

    void MSHPHeatRecovery(EnergyPlusData &state, int const MSHeatPumpNum) // Number of the current electric MSHP being simulated
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Lixing Gu
        //       DATE WRITTEN:    June 2007
        //       RE-ENGINEERED    Revised to calculate MSHP heat recovery rate based on EIR Chiller heat recovery subroutine
        // PURPOSE OF THIS SUBROUTINE:
        //  Calculate the heat recovered from MSHP

        // SUBROUTINE PARAMETER DEFINITIONS:
        static constexpr std::string_view RoutineName("MSHPHeatRecovery");

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 HeatRecOutletTemp; // Heat reclaim outlet temp [C]

        auto &mshp = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);
        // Begin routine
        int HeatRecInNode = mshp.HeatRecInletNodeNum;
        int HeatRecOutNode = mshp.HeatRecOutletNodeNum;

        // Inlet node to the heat recovery heat exchanger
        Real64 HeatRecInletTemp = state.dataLoopNodes->Node(HeatRecInNode).Temp;

        // Set heat recovery mass flow rates
        Real64 HeatRecMassFlowRate = state.dataLoopNodes->Node(HeatRecInNode).MassFlowRate;

        Real64 QHeatRec = state.dataHVACGlobal->MSHPWasteHeat;

        if (HeatRecMassFlowRate > 0.0) {
            // Heat reclaim water inlet specific heat [J/kg-K]
            Real64 CpHeatRec = state.dataPlnt->PlantLoop(mshp.HRPlantLoc.loopNum).glycol->getSpecificHeat(state, HeatRecInletTemp, RoutineName);

            HeatRecOutletTemp = QHeatRec / (HeatRecMassFlowRate * CpHeatRec) + HeatRecInletTemp;
            if (HeatRecOutletTemp > mshp.MaxHeatRecOutletTemp) {
                HeatRecOutletTemp = max(HeatRecInletTemp, mshp.MaxHeatRecOutletTemp);
                QHeatRec = HeatRecMassFlowRate * CpHeatRec * (HeatRecOutletTemp - HeatRecInletTemp);
            }
        } else {
            HeatRecOutletTemp = HeatRecInletTemp;
            QHeatRec = 0.0;
        }

        PlantUtilities::SafeCopyPlantNode(state, HeatRecInNode, HeatRecOutNode);
        // changed outputs
        state.dataLoopNodes->Node(HeatRecOutNode).Temp = HeatRecOutletTemp;

        mshp.HeatRecoveryRate = QHeatRec;
        mshp.HeatRecoveryInletTemp = HeatRecInletTemp;
        mshp.HeatRecoveryOutletTemp = HeatRecOutletTemp;
        mshp.HeatRecoveryMassFlowRate = HeatRecMassFlowRate;
    }

    void SetAverageAirFlow(EnergyPlusData &state,
                           int const MSHeatPumpNum,                     // Unit index
                           Real64 const PartLoadRatio,                  // unit part load ratio
                           Real64 &OnOffAirFlowRatio,                   // ratio of compressor ON airflow to average airflow over timestep
                           ObjexxFCL::Optional_int_const SpeedNum,      // Speed number
                           ObjexxFCL::Optional<Real64 const> SpeedRatio // Speed ratio
    )
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Lixing
        //       DATE WRITTEN   June 2007
        //       RE-ENGINEERED  Resived to meet requirements of multispeed heat pump based on the same subroutine
        //                      in PTHP module

        // PURPOSE OF THIS SUBROUTINE:
        // Set the average air mass flow rates using the part load fraction of the heat pump for this time step
        // Set OnOffAirFlowRatio to be used by DX coils

        // Using/Aliasing
        auto &MSHPMassFlowRateHigh = state.dataHVACGlobal->MSHPMassFlowRateHigh;
        auto &MSHPMassFlowRateLow = state.dataHVACGlobal->MSHPMassFlowRateLow;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        int InletNode;              // inlet node number for PTHPNum
        Real64 AverageUnitMassFlow; // average supply air mass flow rate over time step

        MSHPMassFlowRateLow = 0.0;  // Mass flow rate at low speed
        MSHPMassFlowRateHigh = 0.0; // Mass flow rate at high speed

        if (!state.dataZoneEnergyDemand->CurDeadBandOrSetback(state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).ControlZoneNum) &&
            present(SpeedNum)) {
            if (state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).HeatCoolMode == ModeOfOperation::HeatingMode) {
                if (SpeedNum == 1) {
                    state.dataHVACMultiSpdHP->CompOnMassFlow = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).HeatMassFlowRate(SpeedNum);
                    state.dataHVACMultiSpdHP->CompOnFlowRatio = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).HeatingSpeedRatio(SpeedNum);
                    MSHPMassFlowRateLow = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).HeatMassFlowRate(1);
                    MSHPMassFlowRateHigh = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).HeatMassFlowRate(1);
                } else if (SpeedNum > 1) {
                    state.dataHVACMultiSpdHP->CompOnMassFlow =
                        SpeedRatio * state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).HeatMassFlowRate(SpeedNum) +
                        (1.0 - SpeedRatio) * state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).HeatMassFlowRate(SpeedNum - 1);
                    state.dataHVACMultiSpdHP->CompOnFlowRatio =
                        SpeedRatio * state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).HeatingSpeedRatio(SpeedNum) +
                        (1.0 - SpeedRatio) * state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).HeatingSpeedRatio(SpeedNum - 1);
                    MSHPMassFlowRateLow = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).HeatMassFlowRate(SpeedNum - 1);
                    MSHPMassFlowRateHigh = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).HeatMassFlowRate(SpeedNum);
                }
            } else if (state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).HeatCoolMode == ModeOfOperation::CoolingMode) {
                if (SpeedNum == 1) {
                    state.dataHVACMultiSpdHP->CompOnMassFlow = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).CoolMassFlowRate(SpeedNum);
                    state.dataHVACMultiSpdHP->CompOnFlowRatio = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).CoolingSpeedRatio(SpeedNum);
                    MSHPMassFlowRateLow = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).CoolMassFlowRate(1);
                    MSHPMassFlowRateHigh = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).CoolMassFlowRate(1);
                } else if (SpeedNum > 1) {
                    state.dataHVACMultiSpdHP->CompOnMassFlow =
                        SpeedRatio * state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).CoolMassFlowRate(SpeedNum) +
                        (1.0 - SpeedRatio) * state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).CoolMassFlowRate(SpeedNum - 1);
                    state.dataHVACMultiSpdHP->CompOnFlowRatio =
                        SpeedRatio * state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).CoolingSpeedRatio(SpeedNum) +
                        (1.0 - SpeedRatio) * state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).CoolingSpeedRatio(SpeedNum - 1);
                    MSHPMassFlowRateLow = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).CoolMassFlowRate(SpeedNum - 1);
                    MSHPMassFlowRateHigh = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).CoolMassFlowRate(SpeedNum);
                }
            }
        }
        InletNode = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).AirInletNodeNum;

        // Set up fan flow rate during compressor off time
        if (state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).fanOp == HVAC::FanOp::Continuous && present(SpeedNum)) {
            if (state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).AirFlowControl == AirflowControl::UseCompressorOnFlow &&
                state.dataHVACMultiSpdHP->CompOnMassFlow > 0.0) {
                if (state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).LastMode == ModeOfOperation::HeatingMode) {
                    state.dataHVACMultiSpdHP->CompOffMassFlow = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).HeatMassFlowRate(SpeedNum);
                    state.dataHVACMultiSpdHP->CompOffFlowRatio = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).HeatingSpeedRatio(SpeedNum);
                } else {
                    state.dataHVACMultiSpdHP->CompOffMassFlow = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).CoolMassFlowRate(SpeedNum);
                    state.dataHVACMultiSpdHP->CompOffFlowRatio = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).CoolingSpeedRatio(SpeedNum);
                }
            }
        }

        if (present(SpeedNum)) {
            if (SpeedNum > 1) {
                AverageUnitMassFlow = state.dataHVACMultiSpdHP->CompOnMassFlow;
                state.dataHVACMultiSpdHP->FanSpeedRatio = state.dataHVACMultiSpdHP->CompOnFlowRatio;
            } else {
                AverageUnitMassFlow =
                    (PartLoadRatio * state.dataHVACMultiSpdHP->CompOnMassFlow) + ((1 - PartLoadRatio) * state.dataHVACMultiSpdHP->CompOffMassFlow);
                if (state.dataHVACMultiSpdHP->CompOffFlowRatio > 0.0) {
                    state.dataHVACMultiSpdHP->FanSpeedRatio = (PartLoadRatio * state.dataHVACMultiSpdHP->CompOnFlowRatio) +
                                                              ((1 - PartLoadRatio) * state.dataHVACMultiSpdHP->CompOffFlowRatio);
                } else {
                    state.dataHVACMultiSpdHP->FanSpeedRatio = state.dataHVACMultiSpdHP->CompOnFlowRatio;
                }
            }
        } else {
            AverageUnitMassFlow =
                (PartLoadRatio * state.dataHVACMultiSpdHP->CompOnMassFlow) + ((1 - PartLoadRatio) * state.dataHVACMultiSpdHP->CompOffMassFlow);
            if (state.dataHVACMultiSpdHP->CompOffFlowRatio > 0.0) {
                state.dataHVACMultiSpdHP->FanSpeedRatio =
                    (PartLoadRatio * state.dataHVACMultiSpdHP->CompOnFlowRatio) + ((1 - PartLoadRatio) * state.dataHVACMultiSpdHP->CompOffFlowRatio);
            } else {
                state.dataHVACMultiSpdHP->FanSpeedRatio = state.dataHVACMultiSpdHP->CompOnFlowRatio;
            }
        }

        //!!LKL Discrepancy with > 0
        if (state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum).availSched->getCurrentVal() == 0.0) {
            state.dataLoopNodes->Node(InletNode).MassFlowRate = 0.0;
            OnOffAirFlowRatio = 0.0;
        } else {
            state.dataLoopNodes->Node(InletNode).MassFlowRate = AverageUnitMassFlow;
            state.dataLoopNodes->Node(InletNode).MassFlowRateMaxAvail = AverageUnitMassFlow;
            if (AverageUnitMassFlow > 0.0) {
                OnOffAirFlowRatio = state.dataHVACMultiSpdHP->CompOnMassFlow / AverageUnitMassFlow;
            } else {
                OnOffAirFlowRatio = 0.0;
            }
        }
    }

    void CalcNonDXHeatingCoils(EnergyPlusData &state,
                               int const MSHeatPumpNum,       // multispeed heatpump index
                               bool const FirstHVACIteration, // flag for first HVAC iteration in the time step
                               Real64 const HeatingLoad,      // supplemental coil load to be met by unit (watts)
                               HVAC::FanOp const fanOp,       // fan operation mode
                               Real64 &HeatCoilLoadmet,       // Heating Load Met
                               ObjexxFCL::Optional<Real64 const> PartLoadFrac)
    {

        // SUBROUTINE INFORMATION:
        //       AUTHOR         Bereket Nigusse, FSEC/UCF
        //       DATE WRITTEN   January 2012

        // PURPOSE OF THIS SUBROUTINE:
        // This subroutine simulates the four non dx heating coil types: Gas, Electric, hot water and steam.

        // METHODOLOGY EMPLOYED:
        // Simply calls the different heating coil component.  The hot water flow rate matching the coil load
        // is calculated iteratively.

        // Locals
        static constexpr std::string_view CurrentModuleObject("AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed");

        // SUBROUTINE PARAMETER DEFINITIONS:
        Real64 constexpr ErrTolerance(0.001); // convergence limit for hotwater coil
        int constexpr SolveMaxIter(50);

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 QCoilActual;     // actual heating load met
        Real64 mdot;            // heating coil steam or hot water mass flow rate
        Real64 MinWaterFlow;    // coil minimum hot water mass flow rate, kg/s
        Real64 MaxHotWaterFlow; // coil maximum hot water mass flow rate, kg/s
        Real64 HotWaterMdot;    // actual hot water mass flow rate

        int HeatCoilType;
        int HeatCoilNum;
        Real64 MaxCoilFluidFlow;
        Real64 SteamCoilHeatingLoad;
        int CoilControlNode;
        int CoilOutletNode;
        PlantLocation plantLoc{};

        QCoilActual = 0.0;

        auto &MSHeatPump = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);

        if (present(PartLoadFrac)) {
            HeatCoilType = MSHeatPump.HeatCoilType;
            state.dataHVACMultiSpdHP->HeatCoilName = MSHeatPump.HeatCoilName;
            HeatCoilNum = MSHeatPump.HeatCoilNum;
            MaxCoilFluidFlow = MSHeatPump.MaxCoilFluidFlow;
            CoilControlNode = MSHeatPump.CoilControlNode;
            CoilOutletNode = MSHeatPump.CoilOutletNode;
            plantLoc = MSHeatPump.plantLoc;
        } else {
            HeatCoilType = MSHeatPump.SuppHeatCoilType;
            state.dataHVACMultiSpdHP->HeatCoilName = MSHeatPump.SuppHeatCoilName;
            HeatCoilNum = MSHeatPump.SuppHeatCoilNum;
            MaxCoilFluidFlow = MSHeatPump.MaxSuppCoilFluidFlow;
            CoilControlNode = MSHeatPump.SuppCoilControlNode;
            CoilOutletNode = MSHeatPump.SuppCoilOutletNode;
            plantLoc = MSHeatPump.SuppPlantLoc;
        }

        MSHeatPump.HotWaterPlantLoc = plantLoc;
        MSHeatPump.HotWaterCoilControlNode = CoilControlNode;
        MSHeatPump.HotWaterCoilOutletNode = CoilOutletNode;
        MSHeatPump.HotWaterCoilName = state.dataHVACMultiSpdHP->HeatCoilName;
        MSHeatPump.HotWaterCoilNum = HeatCoilNum;

        if (HeatingLoad > HVAC::SmallLoad) {

            switch (HeatCoilType) {
            case HVAC::Coil_HeatingGasOrOtherFuel:
            case HVAC::Coil_HeatingElectric: {
                HeatingCoils::SimulateHeatingCoilComponents(
                    state, state.dataHVACMultiSpdHP->HeatCoilName, FirstHVACIteration, HeatingLoad, HeatCoilNum, QCoilActual, true, fanOp);
            } break;
            case HVAC::Coil_HeatingWater: {
                if (present(PartLoadFrac)) {
                    MaxHotWaterFlow = MaxCoilFluidFlow * PartLoadFrac;
                    PlantUtilities::SetComponentFlowRate(state, MaxHotWaterFlow, CoilControlNode, CoilOutletNode, plantLoc);
                    WaterCoils::SimulateWaterCoilComponents(
                        state, state.dataHVACMultiSpdHP->HeatCoilName, FirstHVACIteration, HeatCoilNum, QCoilActual, fanOp);
                } else {
                    MaxHotWaterFlow = MaxCoilFluidFlow;
                    PlantUtilities::SetComponentFlowRate(state, MaxHotWaterFlow, CoilControlNode, CoilOutletNode, plantLoc);
                    WaterCoils::SimulateWaterCoilComponents(
                        state, state.dataHVACMultiSpdHP->HeatCoilName, FirstHVACIteration, HeatCoilNum, QCoilActual, fanOp);
                    if (QCoilActual > (HeatingLoad + HVAC::SmallLoad)) {
                        // control water flow to obtain output matching HeatingLoad
                        int SolFlag = 0;
                        MinWaterFlow = 0.0;
                        auto f = [&state, MSHeatPumpNum, FirstHVACIteration, HeatingLoad](Real64 const HWFlow) {
                            // Calculates residual function (QCoilActual - SupHeatCoilLoad) / SupHeatCoilLoad
                            // coil actual output depends on the hot water flow rate which is varied to minimize the residual.
                            Real64 targetHeatingCoilLoad = HeatingLoad;
                            Real64 calcHeatingCoilLoad = targetHeatingCoilLoad;
                            Real64 mdot = HWFlow;
                            auto &hp = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);
                            PlantUtilities::SetComponentFlowRate(
                                state, mdot, hp.HotWaterCoilControlNode, hp.HotWaterCoilOutletNode, hp.HotWaterPlantLoc);
                            // simulate the hot water supplemental heating coil
                            WaterCoils::SimulateWaterCoilComponents(
                                state, hp.HotWaterCoilName, FirstHVACIteration, hp.HotWaterCoilNum, calcHeatingCoilLoad, hp.fanOp);
                            if (targetHeatingCoilLoad != 0.0) {
                                return (calcHeatingCoilLoad - targetHeatingCoilLoad) / targetHeatingCoilLoad;
                            } else { // Autodesk:Return Condition added to assure return value is set
                                return 0.0;
                            }
                        };
                        General::SolveRoot(state, ErrTolerance, SolveMaxIter, SolFlag, HotWaterMdot, f, MinWaterFlow, MaxHotWaterFlow);
                        if (SolFlag == -1) {
                            if (MSHeatPump.HotWaterCoilMaxIterIndex == 0) {
                                ShowWarningMessage(state,
                                                   format("CalcNonDXHeatingCoils: Hot water coil control failed for {}=\"{}\"",
                                                          CurrentModuleObject,
                                                          MSHeatPump.Name));
                                ShowContinueErrorTimeStamp(state, "");
                                ShowContinueError(state,
                                                  format("  Iteration limit [{}] exceeded in calculating hot water mass flow rate", SolveMaxIter));
                            }
                            ShowRecurringWarningErrorAtEnd(
                                state,
                                format("CalcNonDXHeatingCoils: Hot water coil control failed (iteration limit [{}]) for {}=\"{}",
                                       SolveMaxIter,
                                       CurrentModuleObject,
                                       MSHeatPump.Name),
                                MSHeatPump.HotWaterCoilMaxIterIndex);
                        } else if (SolFlag == -2) {
                            if (MSHeatPump.HotWaterCoilMaxIterIndex2 == 0) {
                                ShowWarningMessage(state,
                                                   format("CalcNonDXHeatingCoils: Hot water coil control failed (maximum flow limits) for {}=\"{}\"",
                                                          CurrentModuleObject,
                                                          MSHeatPump.Name));
                                ShowContinueErrorTimeStamp(state, "");
                                ShowContinueError(state, "...Bad hot water maximum flow rate limits");
                                ShowContinueError(state, format("...Given minimum water flow rate={:.3R} kg/s", MinWaterFlow));
                                ShowContinueError(state, format("...Given maximum water flow rate={:.3R} kg/s", MaxHotWaterFlow));
                            }
                            ShowRecurringWarningErrorAtEnd(state,
                                                           "CalcNonDXHeatingCoils: Hot water coil control failed (flow limits) for " +
                                                               std::string{CurrentModuleObject} + "=\"" + MSHeatPump.Name + "\"",
                                                           MSHeatPump.HotWaterCoilMaxIterIndex2,
                                                           MaxHotWaterFlow,
                                                           MinWaterFlow,
                                                           _,
                                                           "[kg/s]",
                                                           "[kg/s]");
                        }
                        // simulate hot water supplemental heating coil
                        WaterCoils::SimulateWaterCoilComponents(
                            state, state.dataHVACMultiSpdHP->HeatCoilName, FirstHVACIteration, HeatCoilNum, QCoilActual, fanOp);
                    }
                }
            } break;
            case HVAC::Coil_HeatingSteam: {
                if (present(PartLoadFrac)) {
                    mdot = MSHeatPump.MaxCoilFluidFlow * PartLoadFrac;
                    SteamCoilHeatingLoad = HeatingLoad * PartLoadFrac;
                } else {
                    mdot = MSHeatPump.MaxCoilFluidFlow;
                    SteamCoilHeatingLoad = HeatingLoad;
                }
                PlantUtilities::SetComponentFlowRate(state, mdot, CoilControlNode, CoilOutletNode, plantLoc);
                // simulate steam supplemental heating coil
                SteamCoils::SimulateSteamCoilComponents(
                    state, state.dataHVACMultiSpdHP->HeatCoilName, FirstHVACIteration, HeatCoilNum, SteamCoilHeatingLoad, QCoilActual, fanOp);
            } break;
            default:
                break;
            }

        } else { // end of IF (HeatingLoad > SmallLoad) THEN

            switch (HeatCoilType) {
            case HVAC::Coil_HeatingGasOrOtherFuel:
            case HVAC::Coil_HeatingElectric: {
                HeatingCoils::SimulateHeatingCoilComponents(
                    state, state.dataHVACMultiSpdHP->HeatCoilName, FirstHVACIteration, HeatingLoad, HeatCoilNum, QCoilActual, true, fanOp);
            } break;
            case HVAC::Coil_HeatingWater: {
                mdot = 0.0;
                PlantUtilities::SetComponentFlowRate(state, mdot, CoilControlNode, CoilOutletNode, plantLoc);
                WaterCoils::SimulateWaterCoilComponents(
                    state, state.dataHVACMultiSpdHP->HeatCoilName, FirstHVACIteration, HeatCoilNum, QCoilActual, fanOp);
            } break;
            case HVAC::Coil_HeatingSteam: {
                mdot = 0.0;
                PlantUtilities::SetComponentFlowRate(state, mdot, CoilControlNode, CoilOutletNode, plantLoc);
                // simulate the steam supplemental heating coil
                SteamCoils::SimulateSteamCoilComponents(
                    state, state.dataHVACMultiSpdHP->HeatCoilName, FirstHVACIteration, HeatCoilNum, HeatingLoad, QCoilActual, fanOp);
            } break;
            default:
                break;
            }
        }
        HeatCoilLoadmet = QCoilActual;
    }

} // namespace HVACMultiSpeedHeatPump

} // namespace EnergyPlus

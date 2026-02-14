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
        Real64 AirMassFlow;  // air mass flow rate [kg/s]
        HVAC::FanOp fanOp;   // operating mode (fan cycling or continuous; DX coil always cycles)
        int ZoneNum;         // Controlled zone number
        Real64 QTotUnitOut;
        int SpeedNum;                    // Speed number
        HVAC::CompressorOp compressorOp; // compressor operation; 1=on, 0=off
        Real64 SaveMassFlowRate;         // saved inlet air mass flow rate [kg/s]

        // zero the fan, DX coils, and supplemental electric heater electricity consumption

        // Why are these state variables?
        state.dataHVACGlobal->DXElecHeatingPower = 0.0; 
        state.dataHVACGlobal->DXElecCoolingPower = 0.0;
        state.dataHVACMultiSpdHP->SaveCompressorPLR = 0.0;
        state.dataHVACGlobal->ElecHeatingCoilPower = 0.0;
        state.dataHVACGlobal->SuppHeatingCoilPower = 0.0;
        state.dataHVACGlobal->DefrostElecPower = 0.0;

        auto &mshp = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);

        // initialize local variables
        UnitOn = true;
        AirMassFlow = state.dataLoopNodes->Node(mshp.AirInletNode).MassFlowRate;
        fanOp = mshp.fanOp;
        ZoneNum = mshp.ControlZoneNum;
        compressorOp = HVAC::CompressorOp::On;

        // set the on/off flags
        if (mshp.fanOp == HVAC::FanOp::Cycling) {
            // cycling unit only runs if there is a cooling or heating load.
            if (std::abs(QZnReq) < HVAC::SmallLoad || AirMassFlow < HVAC::SmallMassFlow ||
                state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) {
                UnitOn = false;
            }
        } else if (mshp.fanOp == HVAC::FanOp::Continuous) {
            // continuous unit: fan runs if scheduled on; coil runs only if there is a cooling or heating load
            if (AirMassFlow < HVAC::SmallMassFlow) {
                UnitOn = false;
            }
        }

        state.dataHVACGlobal->OnOffFanPartLoadFraction = 1.0;

        SaveMassFlowRate = state.dataLoopNodes->Node(mshp.AirInletNode).MassFlowRate;
        if (mshp.EMSOverrideCoilSpeedNumOn) {
            Real64 SpeedVal = mshp.EMSOverrideCoilSpeedNumValue;

            if (!FirstHVACIteration && mshp.fanOp == HVAC::FanOp::Cycling && QZnReq < 0.0 &&
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
                if (ceil(SpeedVal) == mshp.NumOfSpeedCooling && SpeedRatio == 1.0) {
                    state.dataLoopNodes->Node(mshp.AirInletNode).MassFlowRate = SaveMassFlowRate;
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
            if (!FirstHVACIteration && mshp.fanOp == HVAC::FanOp::Cycling && QZnReq < 0.0 &&
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
                if (SpeedNum == mshp.NumOfSpeedCooling && SpeedRatio == 1.0) {
                    // compressor on (reset inlet air mass flow rate to starting value)
                    state.dataLoopNodes->Node(mshp.AirInletNode).MassFlowRate = SaveMassFlowRate;
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

        if (mshp.heatCoilType != HVAC::CoilType::HeatingDXMultiSpeed) {
            state.dataHVACMultiSpdHP->SaveCompressorPLR = PartLoadFrac;
        } else {
            if (SpeedNum > 1) {
                state.dataHVACMultiSpdHP->SaveCompressorPLR = 1.0;
            }

            if (PartLoadFrac == 1.0 && state.dataHVACMultiSpdHP->SaveCompressorPLR < 1.0 && (!mshp.Staged)) {
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
        AirMassFlow = state.dataLoopNodes->Node(mshp.AirInletNode).MassFlowRate;

        QTotUnitOut = AirMassFlow * (state.dataLoopNodes->Node(mshp.AirOutletNode).Enthalpy -
                                     state.dataLoopNodes->Node(mshp.NodeNumOfControlledZone).Enthalpy);

        // report variables
        mshp.CompPartLoadRatio = state.dataHVACMultiSpdHP->SaveCompressorPLR;
        if (mshp.fanOp == HVAC::FanOp::Cycling) {
            if (SupHeaterLoad > 0.0) {
                mshp.FanPartLoadRatio = 1.0;
            } else {
                if (SpeedNum < 2) {
                    mshp.FanPartLoadRatio = PartLoadFrac;
                } else {
                    mshp.FanPartLoadRatio = 1.0;
                }
            }
        } else {
            if (UnitOn) {
                mshp.FanPartLoadRatio = 1.0;
            } else {
                if (SpeedNum < 2) {
                    mshp.FanPartLoadRatio = PartLoadFrac;
                } else {
                    mshp.FanPartLoadRatio = 1.0;
                }
            }
        }

        if (mshp.HeatCoolMode == ModeOfOperation::HeatingMode) {
            mshp.TotHeatEnergyRate = std::abs(max(0.0, QTotUnitOut));
            mshp.SensHeatEnergyRate = std::abs(max(0.0, QSensUnitOut));
            mshp.LatHeatEnergyRate = std::abs(max(0.0, (QTotUnitOut - QSensUnitOut)));
            mshp.TotCoolEnergyRate = 0.0;
            mshp.SensCoolEnergyRate = 0.0;
            mshp.LatCoolEnergyRate = 0.0;
        }
        if (mshp.HeatCoolMode == ModeOfOperation::CoolingMode) {
            mshp.TotCoolEnergyRate = std::abs(min(0.0, QTotUnitOut));
            mshp.SensCoolEnergyRate = std::abs(min(0.0, QSensUnitOut));
            mshp.LatCoolEnergyRate = std::abs(min(0.0, (QTotUnitOut - QSensUnitOut)));
            mshp.TotHeatEnergyRate = 0.0;
            mshp.SensHeatEnergyRate = 0.0;
            mshp.LatHeatEnergyRate = 0.0;
        }

        mshp.AuxElecPower = mshp.AuxOnCyclePower * state.dataHVACMultiSpdHP->SaveCompressorPLR +
                                          mshp.AuxOffCyclePower * (1.0 - state.dataHVACMultiSpdHP->SaveCompressorPLR);
        Real64 locFanElecPower = 0.0;
        locFanElecPower = state.dataFans->fans(mshp.FanNum)->totalPower;
        mshp.ElecPower = locFanElecPower + state.dataHVACGlobal->DXElecCoolingPower + state.dataHVACGlobal->DXElecHeatingPower +
                                       state.dataHVACGlobal->ElecHeatingCoilPower + state.dataHVACGlobal->SuppHeatingCoilPower +
                                       state.dataHVACGlobal->DefrostElecPower + mshp.AuxElecPower;
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
        bool AirNodeFound;             // True when an air node is found
        bool AirLoopFound;             // True when an air loop is found
        int i;                         // Index to speeds
        int j;                         // Index to speeds
        bool Found;                    // Flag to find autosize
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

        auto &s_node = state.dataLoopNodes;

        if (state.dataHVACMultiSpdHP->MSHeatPump.allocated()) return;

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
        state.dataHVACMultiSpdHP->MSHeatPump.allocate(state.dataHVACMultiSpdHP->NumMSHeatPumps);
        state.dataHVACMultiSpdHP->MSHeatPumpReport.allocate(state.dataHVACMultiSpdHP->NumMSHeatPumps);
        state.dataHVACMultiSpdHP->CheckEquipName.dimension(state.dataHVACMultiSpdHP->NumMSHeatPumps, true);

        // Load arrays with reformulated electric EIR chiller data
        for (int MSHPNum = 1; MSHPNum <= state.dataHVACMultiSpdHP->NumMSHeatPumps; ++MSHPNum) {
            auto &thisMSHP = state.dataHVACMultiSpdHP->MSHeatPump(MSHPNum);
            
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

            thisMSHP.AirInletNode = GetOnlySingleNode(state,
                                                         Alphas(3),
                                                         ErrorsFound,
                                                         DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatPumpAirToAirMultiSpeed,
                                                         Alphas(1),
                                                         DataLoopNode::NodeFluidType::Air,
                                                         DataLoopNode::ConnectionType::Inlet,
                                                         NodeInputManager::CompFluidStream::Primary,
                                                         DataLoopNode::ObjectIsParent);

            thisMSHP.AirOutletNode = GetOnlySingleNode(state,
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

            thisMSHP.heatCoilType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Alphas(10)));
            thisMSHP.HeatCoilName = Alphas(11);

            if (thisMSHP.heatCoilType == HVAC::CoilType::HeatingDXMultiSpeed) {
                thisMSHP.HeatCoilNum = DXCoils::GetCoilIndex(state, thisMSHP.HeatCoilName);
                if (thisMSHP.HeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(11), thisMSHP.HeatCoilName);
                    ErrorsFound = true;
                } else {
                    thisMSHP.HeatCoilAirInletNode = DXCoils::GetCoilAirInletNode(state, thisMSHP.HeatCoilNum); 
                    thisMSHP.HeatCoilAirOutletNode = DXCoils::GetCoilAirOutletNode(state, thisMSHP.HeatCoilNum);

                    thisMSHP.MinOATCompressorHeating = DXCoils::GetCoilMinOATCompressor(state, thisMSHP.HeatCoilNum);
                    BranchNodeConnections::SetUpCompSets(state,
                                                         state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                         thisMSHP.Name,
                                                         "Coil:Heating:DX:MultiSpeed",
                                                         thisMSHP.HeatCoilName,
                                                         "UNDEFINED",
                                                         "UNDEFINED");
                }
                
            } else if (thisMSHP.heatCoilType == HVAC::CoilType::HeatingElectricMultiStage || 
                       thisMSHP.heatCoilType == HVAC::CoilType::HeatingGasMultiStage) { 

                thisMSHP.HeatCoilNum = HeatingCoils::GetCoilIndex(state, thisMSHP.HeatCoilName);
                if (thisMSHP.HeatCoilNum == 0) { 
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(11), thisMSHP.HeatCoilName);
                    ErrorsFound = true;
                } else {
                    thisMSHP.HeatCoilAirInletNode = HeatingCoils::GetCoilAirInletNode(state, thisMSHP.HeatCoilNum);
                    thisMSHP.HeatCoilAirOutletNode = HeatingCoils::GetCoilAirOutletNode(state, thisMSHP.HeatCoilNum);

                    BranchNodeConnections::SetUpCompSets(state,
                                                         state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                         thisMSHP.Name,
                                                         HVAC::coilTypeNames[(int)thisMSHP.heatCoilType], 
                                                         thisMSHP.HeatCoilName,
                                                         "UNDEFINED",
                                                         "UNDEFINED");
                }
                
            } else if (thisMSHP.heatCoilType == HVAC::CoilType::HeatingWater) {
                // Get the Heating Coil water Inlet or control Node number
                thisMSHP.HeatCoilNum = WaterCoils::GetCoilIndex(state, thisMSHP.HeatCoilName);
                if (thisMSHP.HeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(11), thisMSHP.HeatCoilName);
                    ErrorsFound = true;
                } else {
                    thisMSHP.HeatCoilControlNode = WaterCoils::GetCoilWaterInletNode(state, thisMSHP.HeatCoilNum);
                    thisMSHP.HeatCoilMaxFluidFlow = WaterCoils::GetCoilMaxWaterFlowRate(state, thisMSHP.HeatCoilNum);
                    thisMSHP.HeatCoilAirInletNode = WaterCoils::GetCoilAirInletNode(state, thisMSHP.HeatCoilNum);
                    thisMSHP.HeatCoilAirOutletNode = WaterCoils::GetCoilAirOutletNode(state, thisMSHP.HeatCoilNum);

                    BranchNodeConnections::SetUpCompSets(state,
                                                         state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                         thisMSHP.Name,
                                                         HVAC::coilTypeNames[(int)thisMSHP.heatCoilType],
                                                         thisMSHP.HeatCoilName,
                                                         s_node->NodeID(thisMSHP.HeatCoilAirInletNode),
                                                         s_node->NodeID(thisMSHP.HeatCoilAirOutletNode));
                }
                
            } else if (thisMSHP.heatCoilType == HVAC::CoilType::HeatingSteam) {
                thisMSHP.HeatCoilNum = SteamCoils::GetCoilIndex(state, thisMSHP.HeatCoilName);
                if (thisMSHP.HeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(11), thisMSHP.HeatCoilName);
                    ErrorsFound = true;
                } else {
                    thisMSHP.HeatCoilControlNode = SteamCoils::GetCoilAirOutletNode(state, thisMSHP.HeatCoilNum);
                    thisMSHP.HeatCoilMaxFluidFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, thisMSHP.HeatCoilNum);
                    if (thisMSHP.HeatCoilMaxFluidFlow > 0.0) {
                        thisMSHP.HeatCoilMaxFluidFlow *= Fluid::GetSteam(state)->getSatDensity(state, 100.0, 1.0, routineName);
                    }

                    // Get the supplemental Heating Coil Inlet Node
                    thisMSHP.HeatCoilAirInletNode = SteamCoils::GetCoilAirInletNode(state, thisMSHP.HeatCoilNum);
                    thisMSHP.HeatCoilAirOutletNode = SteamCoils::GetCoilAirOutletNode(state, thisMSHP.HeatCoilNum);
                    BranchNodeConnections::SetUpCompSets(state,
                                                         state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                         thisMSHP.Name,
                                                         HVAC::coilTypeNames[(int)thisMSHP.heatCoilType],
                                                         thisMSHP.HeatCoilName,
                                                         s_node->NodeID(thisMSHP.HeatCoilAirInletNode),
                                                         s_node->NodeID(thisMSHP.HeatCoilAirOutletNode));
                }
            } else {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(10), Alphas(10));
                ErrorsFound = true;
            }

            // thisMSHP.MinOATCompressor = Numbers(1); // deprecated, now uses coil MinOAT inputs

            thisMSHP.coolCoilType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Alphas(12)));
            thisMSHP.CoolCoilName = Alphas(13);

            if (thisMSHP.coolCoilType == HVAC::CoilType::CoolingDXMultiSpeed) {
              
                thisMSHP.CoolCoilNum = DXCoils::GetCoilIndex(state, thisMSHP.CoolCoilName);
                if (thisMSHP.CoolCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(13), thisMSHP.CoolCoilName);
                    ErrorsFound = true;
                } else {
                    thisMSHP.CoolCoilAirInletNode = DXCoils::GetCoilAirInletNode(state, thisMSHP.CoolCoilNum);
                    thisMSHP.CoolCoilAirOutletNode = DXCoils::GetCoilAirOutletNode(state, thisMSHP.CoolCoilNum);
                    thisMSHP.MinOATCompressorCooling = DXCoils::GetCoilMinOATCompressor(state, thisMSHP.CoolCoilNum);
            
                    BranchNodeConnections::SetUpCompSets(state,
                                                         state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                         thisMSHP.Name,
                                                         HVAC::coilTypeNames[(int)thisMSHP.coolCoilType],
                                                         thisMSHP.CoolCoilName,
                                                         "UNDEFINED",
                                                         "UNDEFINED");
                }
            } else {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(12), Alphas(12));
                ErrorsFound = true;
            }

            // Get supplemental heating coil data
            thisMSHP.suppHeatCoilType = static_cast<HVAC::CoilType>(getEnumValue(HVAC::coilTypeNamesUC, Alphas(14)));
            thisMSHP.SuppHeatCoilName = Alphas(15);
            
            if (thisMSHP.suppHeatCoilType == HVAC::CoilType::HeatingElectric ||
                thisMSHP.suppHeatCoilType == HVAC::CoilType::HeatingGasOrOtherFuel) {
              
                thisMSHP.SuppHeatCoilNum = HeatingCoils::GetCoilIndex(state, thisMSHP.SuppHeatCoilName);
                if (thisMSHP.SuppHeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(15), thisMSHP.SuppHeatCoilName);
                    ErrorsFound = true;
                } else {
                    thisMSHP.SuppHeatCoilAirInletNode = HeatingCoils::GetCoilAirInletNode(state, thisMSHP.SuppHeatCoilNum);
                    thisMSHP.SuppHeatCoilAirOutletNode = HeatingCoils::GetCoilAirOutletNode(state, thisMSHP.SuppHeatCoilNum);
                    thisMSHP.DesignSuppHeatingCapacity = HeatingCoils::GetCoilCapacity(state, thisMSHP.SuppHeatCoilNum);
                    BranchNodeConnections::SetUpCompSets(state,
                                                         state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                         thisMSHP.Name,
                                                         HVAC::coilTypeNames[(int)thisMSHP.suppHeatCoilType],
                                                         thisMSHP.SuppHeatCoilName,
                                                         "UNDEFINED",
                                                         "UNDEFINED");
                }
                
            } else if (thisMSHP.suppHeatCoilType == HVAC::CoilType::HeatingWater) {
                thisMSHP.SuppHeatCoilNum = WaterCoils::GetCoilIndex(state, thisMSHP.SuppHeatCoilName);
                if (thisMSHP.SuppHeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(15), thisMSHP.SuppHeatCoilName);
                    ErrorsFound = true;
                } else {
                    thisMSHP.SuppHeatCoilControlNode = WaterCoils::GetCoilWaterInletNode(state, thisMSHP.SuppHeatCoilNum);
                    thisMSHP.SuppHeatCoilMaxFluidFlow = WaterCoils::GetCoilMaxWaterFlowRate(state, thisMSHP.SuppHeatCoilNum);
                    thisMSHP.SuppHeatCoilAirInletNode = WaterCoils::GetCoilAirInletNode(state, thisMSHP.SuppHeatCoilNum);
                    thisMSHP.SuppHeatCoilAirOutletNode = WaterCoils::GetCoilAirOutletNode(state, thisMSHP.SuppHeatCoilNum);

                    BranchNodeConnections::SetUpCompSets(state,
                                                         state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                         thisMSHP.Name,
                                                         "Coil:Heating:Water",
                                                         thisMSHP.SuppHeatCoilName,
                                                         s_node->NodeID(thisMSHP.SuppHeatCoilAirInletNode),
                                                         s_node->NodeID(thisMSHP.SuppHeatCoilAirOutletNode));
                }

            } else if (thisMSHP.suppHeatCoilType == HVAC::CoilType::HeatingSteam) {
                thisMSHP.SuppHeatCoilNum = SteamCoils::GetCoilIndex(state, thisMSHP.SuppHeatCoilName);
                if (thisMSHP.SuppHeatCoilNum == 0) {
                    ShowSevereItemNotFound(state, eoh, cAlphaFields(15), thisMSHP.SuppHeatCoilName);
                    ErrorsFound = true;
                } else {
                    // This is the control node?  Everywhere else the control node is a fluid node.
                    thisMSHP.SuppHeatCoilControlNode = SteamCoils::GetCoilAirOutletNode(state, thisMSHP.SuppHeatCoilNum);
                    thisMSHP.SuppHeatCoilMaxFluidFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, thisMSHP.SuppHeatCoilNum);
                    if (thisMSHP.SuppHeatCoilMaxFluidFlow > 0.0) {
                        thisMSHP.SuppHeatCoilMaxFluidFlow *= Fluid::GetSteam(state)->getSatDensity(state, 100.0, 1.0, routineName);
                    }

                    thisMSHP.SuppHeatCoilAirInletNode = SteamCoils::GetCoilAirInletNode(state, thisMSHP.SuppHeatCoilNum);
                    thisMSHP.SuppHeatCoilAirOutletNode = SteamCoils::GetCoilAirOutletNode(state, thisMSHP.SuppHeatCoilNum);
                    BranchNodeConnections::SetUpCompSets(state,
                                                         state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                         thisMSHP.Name,
                                                         HVAC::coilTypeNames[(int)thisMSHP.suppHeatCoilType],
                                                         thisMSHP.SuppHeatCoilName,
                                                         s_node->NodeID(thisMSHP.SuppHeatCoilAirInletNode),
                                                         s_node->NodeID(thisMSHP.SuppHeatCoilAirOutletNode));
                }
                
            } else {
                ShowSevereInvalidKey(state, eoh, cAlphaFields(14), Alphas(14));
                ErrorsFound = true;
            }

            thisMSHP.HeatingSizingRatio = Numbers(1);
            if (thisMSHP.heatCoilType == HVAC::CoilType::HeatingDXMultiSpeed) {
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchDXHeatCoilSizingRatio, thisMSHP.HeatCoilName, thisMSHP.HeatingSizingRatio);
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
                state, state.dataOutRptPredefined->pdchDXHeatCoilSuppHiT, thisMSHP.HeatCoilName, thisMSHP.SuppMaxOATemp);

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
                thisMSHP.HeatRecFluidInletNode = GetOnlySingleNode(state,
                                                                 Alphas(16),
                                                                 ErrorsFound,
                                                                 DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatPumpAirToAirMultiSpeed,
                                                                 Alphas(1),
                                                                 DataLoopNode::NodeFluidType::Water,
                                                                 DataLoopNode::ConnectionType::Inlet,
                                                                 NodeInputManager::CompFluidStream::Tertiary,
                                                                 DataLoopNode::ObjectIsNotParent);
                if (thisMSHP.HeatRecFluidInletNode == 0) {
                    ShowSevereError(
                        state, format("{}, \"{}\", Missing {}.", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name, cAlphaFields(16)));
                    ErrorsFound = true;
                }
                thisMSHP.HeatRecFluidOutletNode = GetOnlySingleNode(state,
                                                                  Alphas(17),
                                                                  ErrorsFound,
                                                                  DataLoopNode::ConnectionObjectType::AirLoopHVACUnitaryHeatPumpAirToAirMultiSpeed,
                                                                  Alphas(1),
                                                                  DataLoopNode::NodeFluidType::Water,
                                                                  DataLoopNode::ConnectionType::Outlet,
                                                                  NodeInputManager::CompFluidStream::Tertiary,
                                                                  DataLoopNode::ObjectIsNotParent);
                if (thisMSHP.HeatRecFluidOutletNode == 0) {
                    ShowSevereError(
                        state, format("{}, \"{}\", Missing {}.", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name, cAlphaFields(17)));
                    ErrorsFound = true;
                }
                BranchNodeConnections::TestCompSet(
                    state, state.dataHVACMultiSpdHP->CurrentModuleObject, Alphas(1), Alphas(16), Alphas(17), "MSHP Heat Recovery Nodes");
                DXCoils::SetMSHPDXCoilHeatRecoveryFlag(state, thisMSHP.CoolCoilNum);
                if (thisMSHP.HeatCoilNum > 0) {
                    DXCoils::SetMSHPDXCoilHeatRecoveryFlag(state, thisMSHP.HeatCoilNum);
                }
            } else {
                thisMSHP.HeatRecActive = false;
                thisMSHP.DesignHeatRecMassFlowRate = 0.0;
                thisMSHP.HeatRecFluidInletNode = 0;
                thisMSHP.HeatRecFluidOutletNode = 0;
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
                if (thisMSHP.heatCoilType == HVAC::CoilType::HeatingDXMultiSpeed) {
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
                for (int i = 1; i <= thisMSHP.NumOfSpeedHeating; ++i) {
                    thisMSHP.HeatVolumeFlowRate(i) = Numbers(10 + i);
                    if (thisMSHP.heatCoilType == HVAC::CoilType::HeatingDXMultiSpeed) {
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
                DXCoils::DisableLatentDegradation(state, thisMSHP.CoolCoilNum);
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
                if (thisMSHP.FanInletNode != thisMSHP.AirInletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(
                        state, format("When a blow through fan is specified, the fan inlet node name must be the same as the {}", cAlphaFields(3)));
                    ShowContinueError(state, format("...Fan inlet node name           = {}", s_node->NodeID(thisMSHP.FanInletNode)));
                    ShowContinueError(state, format("...{} = {}", cAlphaFields(3), s_node->NodeID(thisMSHP.AirInletNode)));
                    ErrorsFound = true;
                }
                if (thisMSHP.FanOutletNode != thisMSHP.CoolCoilAirInletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(
                        state,
                        "When a blow through fan is specified, the fan outlet node name must be the same as the cooling coil inlet node name.");
                    ShowContinueError(state, format("...Fan outlet node name         = {}", s_node->NodeID(thisMSHP.FanOutletNode)));
                    ShowContinueError(state, format("...Cooling coil inlet node name = {}", s_node->NodeID(thisMSHP.CoolCoilAirInletNode)));
                    ErrorsFound = true;
                }
                if (thisMSHP.CoolCoilAirOutletNode != thisMSHP.HeatCoilAirInletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(state, "The cooling coil outlet node name must be the same as the heating coil inlet node name.");
                    ShowContinueError(state, format("...Cooling coil outlet node name = {}", s_node->NodeID(thisMSHP.CoolCoilAirOutletNode)));
                    ShowContinueError(state, format("...Heating coil inlet node name  = {}", s_node->NodeID(thisMSHP.HeatCoilAirInletNode)));
                    ErrorsFound = true;
                }
                if (thisMSHP.HeatCoilAirOutletNode != thisMSHP.SuppHeatCoilAirInletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(state,
                                      "When a blow through fan is specified, the heating coil outlet node name must be the same as the reheat coil "
                                      "inlet node name.");
                    ShowContinueError(state, format("...Heating coil outlet node name = {}", s_node->NodeID(thisMSHP.HeatCoilAirOutletNode)));
                    ShowContinueError(state, format("...Reheat coil inlet node name   = {}", s_node->NodeID(thisMSHP.SuppHeatCoilAirInletNode)));
                    ErrorsFound = true;
                }
                if (thisMSHP.SuppHeatCoilAirOutletNode != thisMSHP.AirOutletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(state, format("The supplemental heating coil outlet node name must be the same as the {}", cAlphaFields(4)));
                    ShowContinueError(
                        state, format("...Supplemental heating coil outlet node name   = {}", s_node->NodeID(thisMSHP.SuppHeatCoilAirOutletNode)));
                    ShowContinueError(state, format("...{} = {}", cAlphaFields(4), s_node->NodeID(thisMSHP.AirOutletNode)));
                    ErrorsFound = true;
                }
            } else {
                if (thisMSHP.CoolCoilAirInletNode != thisMSHP.AirInletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(
                        state,
                        format("When a draw through fan is specified, the cooling coil inlet node name must be the same as the {}", cAlphaFields(3)));
                    ShowContinueError(state, format("...Cooling coil inlet node name  = {}", s_node->NodeID(thisMSHP.CoolCoilAirInletNode)));
                    ShowContinueError(state, format("...{} = {}", cAlphaFields(3), s_node->NodeID(thisMSHP.AirInletNode)));
                    ErrorsFound = true;
                }
                if (thisMSHP.CoolCoilAirOutletNode != thisMSHP.HeatCoilAirInletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(state, "The cooling coil outlet node name must be the same as the heating coil inlet node name.");
                    ShowContinueError(state, format("...Cooling coil outlet node name = {}", s_node->NodeID(thisMSHP.CoolCoilAirOutletNode)));
                    ShowContinueError(state, format("...Heating coil inlet node name  = {}", s_node->NodeID(thisMSHP.HeatCoilAirInletNode)));
                    ErrorsFound = true;
                }
                if (thisMSHP.HeatCoilAirOutletNode != thisMSHP.FanInletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(
                        state,
                        "When a draw through fan is specified, the heating coil outlet node name must be the same as the fan inlet node name.");
                    ShowContinueError(state, format("...Heating coil outlet node name = {}", s_node->NodeID(thisMSHP.HeatCoilAirOutletNode)));
                    ShowContinueError(state, format("...Fan inlet node name           = {}", s_node->NodeID(thisMSHP.FanInletNode)));
                    ErrorsFound = true;
                }
                if (thisMSHP.FanOutletNode != thisMSHP.SuppHeatCoilAirInletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(
                        state, "When a draw through fan is specified, the fan outlet node name must be the same as the reheat coil inlet node name.");
                    ShowContinueError(state, format("...Fan outlet node name        = {}", s_node->NodeID(thisMSHP.FanOutletNode)));
                    ShowContinueError(state, format("...Reheat coil inlet node name = {}", s_node->NodeID(thisMSHP.SuppHeatCoilAirInletNode)));
                    ErrorsFound = true;
                }
                if (thisMSHP.SuppHeatCoilAirOutletNode != thisMSHP.AirOutletNode) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(state, format("The reheat coil outlet node name must be the same as the {}", cAlphaFields(4)));
                    ShowContinueError(state, format("...Reheat coil outlet node name   = {}", s_node->NodeID(thisMSHP.SuppHeatCoilAirOutletNode)));
                    ShowContinueError(state, format("...{} = {}", cAlphaFields(4), s_node->NodeID(thisMSHP.AirOutletNode)));
                    ErrorsFound = true;
                }
            }

            // Ensure the numbers of speeds defined in the parent object are equal to the numbers defined in coil objects
            if (thisMSHP.heatCoilType == HVAC::CoilType::HeatingDXMultiSpeed) {
                int numSpeeds = DXCoils::GetCoilNumberOfSpeeds(state, thisMSHP.HeatCoilNum);
                if (thisMSHP.NumOfSpeedHeating != numSpeeds) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(
                        state, format("The {} is not equal to the number defined in {} = {}", cNumericFields(9), cAlphaFields(11), Alphas(11)));
                    ErrorsFound = true;
                }
            } else if (thisMSHP.heatCoilType == HVAC::CoilType::HeatingElectricMultiStage ||
                       thisMSHP.heatCoilType == HVAC::CoilType::HeatingGasMultiStage) {
                int numSpeeds = HeatingCoils::GetCoilNumberOfStages(state, thisMSHP.HeatCoilNum);
                if (thisMSHP.NumOfSpeedHeating != numSpeeds) {
                    ShowSevereError(state, format("For {} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, thisMSHP.Name));
                    ShowContinueError(
                        state, format("The {} is not equal to the number defined in {} = {}", cNumericFields(9), cAlphaFields(11), Alphas(11)));
                    ErrorsFound = true;
                }
            }

            int numSpeeds = DXCoils::GetCoilNumberOfSpeeds(state, thisMSHP.CoolCoilNum);
            if (thisMSHP.NumOfSpeedCooling != numSpeeds) {
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

        Real64 mdot(0.0);               // local temporary for mass flow rate (kg/s)
        Real64 CoilMaxVolFlowRate(0.0); // coil fluid maximum volume flow rate
        Real64 QActual(0.0);            // coil actual capacity

        auto &s_node = state.dataLoopNodes;
        auto &mshp = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);

        ++state.dataHVACMultiSpdHP->AirLoopPass;
        if (state.dataHVACMultiSpdHP->AirLoopPass > 2) {
            state.dataHVACMultiSpdHP->AirLoopPass = 1;
        }

        if (mshp.MyPlantScantFlag && allocated(state.dataPlnt->PlantLoop)) {
            bool errFlag;
            if (mshp.HeatRecActive) {
                errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(state,
                                                        mshp.Name,
                                                        DataPlant::PlantEquipmentType::MultiSpeedHeatPumpRecovery,
                                                        mshp.HRPlantLoc,
                                                        errFlag,
                                                        _,
                                                        _,
                                                        _,
                                                        _,
                                                        _);
                if (errFlag) {
                    ShowFatalError(state, "InitMSHeatPump: Program terminated for previous conditions.");
                }

                mshp.MyPlantScantFlag = false;
            } else {
                mshp.MyPlantScantFlag = false;
            }
            
            if (mshp.heatCoilType == HVAC::CoilType::HeatingWater) {
                errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(state,
                                                        mshp.HeatCoilName,
                                                        DataPlant::PlantEquipmentType::CoilWaterSimpleHeating,
                                                        mshp.HeatCoilPlantLoc,
                                                        errFlag,
                                                        _,
                                                        _,
                                                        _,
                                                        _,
                                                        _);
                if (errFlag) {
                    ShowFatalError(state, "InitMSHeatPump: Program terminated for previous conditions.");
                }
                mshp.HeatCoilMaxFluidFlow = WaterCoils::GetCoilMaxWaterFlowRate(state, mshp.HeatCoilNum);

                if (mshp.HeatCoilMaxFluidFlow > 0.0) {
                    mshp.HeatCoilMaxFluidFlow *=
                      state.dataPlnt->PlantLoop(mshp.HeatCoilPlantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                }

                // fill outlet node for coil
                mshp.HeatCoilFluidOutletNode = DataPlant::CompData::getPlantComponent(state, mshp.HeatCoilPlantLoc).NodeNumOut;
                mshp.MyPlantScantFlag = false;

            } else if (mshp.heatCoilType == HVAC::CoilType::HeatingSteam) {
                errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(state,
                                                        mshp.HeatCoilName,
                                                        DataPlant::PlantEquipmentType::CoilSteamAirHeating,
                                                        mshp.HeatCoilPlantLoc,
                                                        errFlag,
                                                        _,
                                                        _,
                                                        _,
                                                        _,
                                                        _);
                if (errFlag) {
                    ShowFatalError(state, "InitMSHeatPump: Program terminated for previous conditions.");
                }
                mshp.HeatCoilMaxFluidFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, mshp.HeatCoilNum);
                if (mshp.HeatCoilMaxFluidFlow > 0.0) {
                    mshp.HeatCoilMaxFluidFlow *= Fluid::GetSteam(state)->getSatDensity(state, 100.0, 1.0, RoutineName);
                }

                // fill outlet node for coil
                mshp.HeatCoilFluidOutletNode = DataPlant::CompData::getPlantComponent(state, mshp.HeatCoilPlantLoc).NodeNumOut;
                mshp.MyPlantScantFlag = false;
            }
            
            if (mshp.suppHeatCoilType == HVAC::CoilType::HeatingWater) {
                errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(state,
                                                        mshp.SuppHeatCoilName,
                                                        DataPlant::PlantEquipmentType::CoilWaterSimpleHeating,
                                                        mshp.SuppHeatCoilPlantLoc,
                                                        errFlag,
                                                        _,
                                                        _,
                                                        _,
                                                        _,
                                                        _);
                if (errFlag) {
                    ShowFatalError(state, "InitMSHeatPump: Program terminated for previous conditions.");
                }
                mshp.SuppHeatCoilMaxFluidFlow = WaterCoils::GetCoilMaxWaterFlowRate(state, mshp.SuppHeatCoilNum);

                if (mshp.SuppHeatCoilMaxFluidFlow > 0.0) {
                    mshp.SuppHeatCoilMaxFluidFlow *=
                      state.dataPlnt->PlantLoop(mshp.SuppHeatCoilPlantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                }
                
                // fill outlet node for coil
                mshp.SuppHeatCoilFluidOutletNode = DataPlant::CompData::getPlantComponent(state, mshp.SuppHeatCoilPlantLoc).NodeNumOut;
                mshp.MyPlantScantFlag = false;

            } else if (mshp.suppHeatCoilType == HVAC::CoilType::HeatingSteam) {
                errFlag = false;
                PlantUtilities::ScanPlantLoopsForObject(state,
                                                        mshp.SuppHeatCoilName,
                                                        DataPlant::PlantEquipmentType::CoilSteamAirHeating,
                                                        mshp.SuppHeatCoilPlantLoc,
                                                        errFlag,
                                                        _,
                                                        _,
                                                        _,
                                                        _,
                                                        _);
                if (errFlag) {
                    ShowFatalError(state, "InitMSHeatPump: Program terminated for previous conditions.");
                }

                mshp.SuppHeatCoilMaxFluidFlow = SteamCoils::GetCoilMaxSteamFlowRate(state, mshp.SuppHeatCoilNum);
                if (mshp.SuppHeatCoilMaxFluidFlow > 0.0) {
                    mshp.SuppHeatCoilMaxFluidFlow *= Fluid::GetSteam(state)->getSatDensity(state, 100.0, 1.0, RoutineName);
                }

                // fill outlet node for coil
                mshp.SuppHeatCoilFluidOutletNode = DataPlant::CompData::getPlantComponent(state, mshp.SuppHeatCoilPlantLoc).NodeNumOut;
                mshp.MyPlantScantFlag = false;
            }
        } else if (mshp.MyPlantScantFlag && !state.dataGlobal->AnyPlantInModel) {
            mshp.MyPlantScantFlag = false;
        }

        if (!state.dataGlobal->SysSizingCalc && mshp.MySizeFlag) {
            mshp.FanVolFlow = state.dataFans->fans(mshp.FanNum)->maxAirFlowRate;
            SizeMSHeatPump(state, MSHeatPumpNum);
            mshp.FlowFraction = 1.0;
            mshp.MySizeFlag = false;
            // Pass the fan cycling schedule index up to the air loop. Set the air loop unitary system flag.
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).cycFanSched = mshp.fanOpModeSched;
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).UnitarySys = true;
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).UnitarySysSimulating =
                false; // affects child coil sizing by allowing coil to size itself instead of parent telling coil what size to use
            state.dataAirLoop->AirLoopControlInfo(AirLoopNum).fanOp = mshp.fanOp;
        }
        if (mshp.reportACCAManualS && !state.dataGlobal->DoingSizing && !state.dataGlobal->WarmupFlag) {
            mshp.reportACCAManualS = false;
            // ACCA Manual S reporting
            Real64 SysSensCoolingLoad = 0.0;
            Real64 SysTotCoolingLoad = 0.0;
            Real64 SysLatCoolingLoad = 0.0;
            Real64 SysHeatingLoad = 0.0;

            if (mshp.isHeatPump &&
                (state.dataSize->CurSysNum > 0 && !state.dataSize->FinalSysSizing.empty() &&
                 state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).heatCoilSizingMethod != DataSizing::HeatCoilSizMethod::None)) {

                if (state.dataSize->CurSysNum > 0) {
                    SysTotCoolingLoad = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).TotCoolCap;
                    SysSensCoolingLoad = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).SensCoolCap;
                    SysLatCoolingLoad = SysTotCoolingLoad - SysSensCoolingLoad;
                    SysHeatingLoad = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).HeatCap;
                }

                Real64 coilSHR = 1.0;
                auto const &thisCoolCoil = state.dataDXCoils->DXCoil(mshp.CoolCoilNum);
                std::string_view cCoilName = thisCoolCoil.Name;
                Real64 ratedCoolingCap = thisCoolCoil.MSRatedTotCap(mshp.NumOfSpeedCooling);
                coilSHR = thisCoolCoil.MSRatedSHR(mshp.NumOfSpeedCooling);
                auto const &thisHeatCoil = state.dataDXCoils->DXCoil(mshp.HeatCoilNum);
                std::string_view hCoilName = thisHeatCoil.Name;
                Real64 ratedHeatingCap = thisCoolCoil.MSRatedTotCap(mshp.NumOfSpeedHeating);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchMSHPType, cCoilName, "AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed");
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSHPName, cCoilName, mshp.Name);
                OutputReportPredefined::PreDefTableEntry(
                   state, state.dataOutRptPredefined->pdchMSCoilType, cCoilName, HVAC::coilTypeNames[(int)mshp.coolCoilType]);
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
                OutputReportPredefined::PreDefTableEntry(state, state.dataOutRptPredefined->pdchMSHPName, hCoilName, mshp.Name);
                OutputReportPredefined::PreDefTableEntry(
                    state, state.dataOutRptPredefined->pdchMSCoilType, hCoilName, HVAC::coilTypeNames[(int)mshp.heatCoilType]);
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

        if (allocated(state.dataZoneEquip->ZoneEquipConfig) && mshp.MyCheckFlag) {
            int zoneNum = mshp.ControlZoneNum;
            int zoneInlet = mshp.ZoneInletNode;
            // setup furnace zone equipment sequence information based on finding matching air terminal
            if (state.dataZoneEquip->ZoneEquipConfig(zoneNum).EquipListIndex > 0) {
                int coolingPriority = 0;
                int heatingPriority = 0;
                state.dataZoneEquip->ZoneEquipList(state.dataZoneEquip->ZoneEquipConfig(zoneNum).EquipListIndex)
                    .getPrioritiesForInletNode(state, zoneInlet, coolingPriority, heatingPriority);
                mshp.ZoneSequenceCoolingNum = coolingPriority;
                mshp.ZoneSequenceHeatingNum = heatingPriority;
            }
            mshp.MyCheckFlag = false;
            if (mshp.ZoneSequenceCoolingNum == 0 || mshp.ZoneSequenceHeatingNum == 0) {
                ShowSevereError(state,
                                format("AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed, \"{}\": Airloop air terminal in the zone equipment list for "
                                       "zone = {} not found or is not allowed Zone Equipment Cooling or Heating Sequence = 0.",
                                       mshp.Name,
                                       mshp.ControlZoneName));
                ShowFatalError(state,
                               "Subroutine InitMSHeatPump: Errors found in getting AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed input.  Preceding "
                               "condition(s) causes termination.");
            }
        }

        // Find the number of zones (zone Inlet Nodes) attached to an air loop from the air loop number
        NumAirLoopZones =
            state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesCooled + state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).NumZonesHeated;
        if (allocated(state.dataAirLoop->AirToZoneNodeInfo) && mshp.MyFlowFracFlag) {
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
                SumOfMassFlowRateMax += s_node->Node(ZoneInletNodeNum).MassFlowRateMax;
                if (state.dataAirLoop->AirToZoneNodeInfo(AirLoopNum).CoolCtrlZoneNums(ZoneInSysIndex) == mshp.ControlZoneNum) {
                    CntrlZoneTerminalUnitMassFlowRateMax = s_node->Node(ZoneInletNodeNum).MassFlowRateMax;
                }
            }
            if (SumOfMassFlowRateMax != 0.0 && mshp.MyFlowFracFlag) {
                if (CntrlZoneTerminalUnitMassFlowRateMax >= HVAC::SmallAirVolFlow) {
                    mshp.FlowFraction = CntrlZoneTerminalUnitMassFlowRateMax / SumOfMassFlowRateMax;
                } else {
                    ShowSevereError(state, format("{} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, mshp.Name));
                    ShowContinueError(state, " The Fraction of Supply Air Flow That Goes Through the Controlling Zone is set to 1.");
                }
                BaseSizer::reportSizerOutput(state,
                                             state.dataHVACMultiSpdHP->CurrentModuleObject,
                                             mshp.Name,
                                             "Fraction of Supply Air Flow That Goes Through the Controlling Zone",
                                             mshp.FlowFraction);
                mshp.MyFlowFracFlag = false;
            }
        }

        // Do the Begin Environment initializations
        if (state.dataGlobal->BeginEnvrnFlag && mshp.MyEnvrnFlag) {
            RhoAir = state.dataEnvrn->StdRhoAir;
            // set the mass flow rates from the input volume flow rates
            for (i = 1; i <= mshp.NumOfSpeedCooling; ++i) {
                mshp.CoolMassFlowRate(i) = RhoAir * mshp.CoolVolumeFlowRate(i);
            }
            for (i = 1; i <= mshp.NumOfSpeedHeating; ++i) {
                mshp.HeatMassFlowRate(i) = RhoAir * mshp.HeatVolumeFlowRate(i);
            }
            mshp.IdleMassFlowRate = RhoAir * mshp.IdleVolumeAirRate;
            // set the node max and min mass flow rates
            s_node->Node(mshp.AirInletNode).MassFlowRateMax =
                max(mshp.CoolMassFlowRate(mshp.NumOfSpeedCooling), mshp.HeatMassFlowRate(mshp.NumOfSpeedHeating));
            s_node->Node(mshp.AirInletNode).MassFlowRateMaxAvail =
                max(mshp.CoolMassFlowRate(mshp.NumOfSpeedCooling), mshp.HeatMassFlowRate(mshp.NumOfSpeedHeating));
            s_node->Node(mshp.AirInletNode).MassFlowRateMin = 0.0;
            s_node->Node(mshp.AirInletNode).MassFlowRateMinAvail = 0.0;
            s_node->Node(mshp.AirOutletNode) = s_node->Node(mshp.AirInletNode);
            mshp.LoadLoss = 0.0;

            if ((mshp.HeatRecActive) && (!mshp.MyPlantScantFlag)) {

                rho = state.dataPlnt->PlantLoop(mshp.HRPlantLoc.loopNum)
                          .glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);

                mshp.DesignHeatRecMassFlowRate = mshp.DesignHeatRecFlowRate * rho;

                PlantUtilities::InitComponentNodes(state,
                                                   0.0,
                                                   mshp.DesignHeatRecMassFlowRate,
                                                   mshp.HeatRecFluidInletNode,
                                                   mshp.HeatRecFluidOutletNode);
            }
            
            if (mshp.HeatCoilControlNode > 0) {
                if (mshp.HeatCoilMaxFluidFlow == DataSizing::AutoSize) {
                  if (mshp.heatCoilType == HVAC::CoilType::HeatingWater) {
                        WaterCoils::SimulateWaterCoilComponents(state, mshp.HeatCoilName, FirstHVACIteration, mshp.HeatCoilNum);

                        mshp.HeatCoilMaxVolFlowRate = WaterCoils::GetCoilMaxWaterFlowRate(state, mshp.HeatCoilNum);
                        if (CoilMaxVolFlowRate != DataSizing::AutoSize) {
                            mshp.HeatCoilMaxFluidFlow = mshp.HeatCoilMaxVolFlowRate *
                                state.dataPlnt->PlantLoop(mshp.HeatCoilPlantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                        }
                        PlantUtilities::InitComponentNodes(state,
                                                           0.0,
                                                           mshp.HeatCoilMaxFluidFlow,
                                                           mshp.HeatCoilControlNode,
                                                           mshp.HeatCoilFluidOutletNode);

                  } else if (mshp.heatCoilType == HVAC::CoilType::HeatingSteam) {

                        SteamCoils::SimulateSteamCoilComponents(state,
                                                                mshp.HeatCoilName,
                                                                FirstHVACIteration,
                                                                mshp.HeatCoilNum,
                                                                1.0,
                                                                QActual); // QCoilReq, simulate any load > 0 to get max capacity of steam coil

                        mshp.HeatCoilMaxVolFlowRate = SteamCoils::GetCoilMaxSteamFlowRate(state, mshp.HeatCoilNum);

                        if (mshp.HeatCoilMaxVolFlowRate != DataSizing::AutoSize) {
                            mshp.HeatCoilMaxFluidFlow = mshp.HeatCoilMaxVolFlowRate *
                                Fluid::GetSteam(state)->getSatDensity(state, 100.0, 1.0, RoutineName);
                        }
                        PlantUtilities::InitComponentNodes(state,
                                                           0.0,
                                                           mshp.HeatCoilMaxFluidFlow,
                                                           mshp.HeatCoilControlNode,
                                                           mshp.HeatCoilFluidOutletNode);
                    }
                }
            }
            
            if (mshp.SuppHeatCoilControlNode > 0) {
                if (mshp.SuppHeatCoilMaxFluidFlow == DataSizing::AutoSize) {
                    if (mshp.suppHeatCoilType == HVAC::CoilType::HeatingWater) {
                        WaterCoils::SimulateWaterCoilComponents(state, mshp.SuppHeatCoilName, FirstHVACIteration, mshp.SuppHeatCoilNum);

                        mshp.SuppHeatCoilMaxVolFlowRate = WaterCoils::GetCoilMaxWaterFlowRate(state, mshp.SuppHeatCoilNum);
                        if (mshp.SuppHeatCoilMaxVolFlowRate != DataSizing::AutoSize) {
                            mshp.SuppHeatCoilMaxFluidFlow = mshp.SuppHeatCoilMaxVolFlowRate *
                              state.dataPlnt->PlantLoop(mshp.SuppHeatCoilPlantLoc.loopNum).glycol->getDensity(state, Constant::HWInitConvTemp, RoutineName);
                        }
                        PlantUtilities::InitComponentNodes(state,
                                                           0.0,
                                                           mshp.SuppHeatCoilMaxFluidFlow,
                                                           mshp.SuppHeatCoilControlNode,
                                                           mshp.SuppHeatCoilFluidOutletNode);

                    } else if (mshp.suppHeatCoilType == HVAC::CoilType::HeatingSteam) {

                        SteamCoils::SimulateSteamCoilComponents(state,
                                                                mshp.SuppHeatCoilName,
                                                                FirstHVACIteration,
                                                                mshp.SuppHeatCoilNum,
                                                                1.0,
                                                                QActual); // QCoilReq, simulate any load > 0 to get max capacity of steam coil
                        mshp.SuppHeatCoilMaxVolFlowRate = SteamCoils::GetCoilMaxSteamFlowRate(state, mshp.SuppHeatCoilNum);
                        if (mshp.SuppHeatCoilMaxVolFlowRate != DataSizing::AutoSize) {
                            mshp.SuppHeatCoilMaxFluidFlow = mshp.SuppHeatCoilMaxVolFlowRate *
                              Fluid::GetSteam(state)->getSatDensity(state, 100.0, 1.0, RoutineName);
                              
                        }
                        PlantUtilities::InitComponentNodes(state,
                                                           0.0,
                                                           mshp.SuppHeatCoilMaxFluidFlow,
                                                           mshp.SuppHeatCoilControlNode,
                                                           mshp.SuppHeatCoilFluidOutletNode);
                    }
                }
            }
            mshp.MyEnvrnFlag = false;
        } // end one time inits

        if (!state.dataGlobal->BeginEnvrnFlag) {
            mshp.MyEnvrnFlag = true;
        }

        // IF MSHP system was not autosized and the fan is autosized, check that fan volumetric flow rate is greater than MSHP flow rates
        if (!state.dataGlobal->DoingSizing && mshp.CheckFanFlow) {
            state.dataHVACMultiSpdHP->CurrentModuleObject = "AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed";
            mshp.FanVolFlow = state.dataFans->fans(mshp.FanNum)->maxAirFlowRate;
            if (mshp.FanVolFlow != DataSizing::AutoSize) {
                //     Check fan versus system supply air flow rates
                if (mshp.FanVolFlow < mshp.CoolVolumeFlowRate(mshp.NumOfSpeedCooling)) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the MSHP system air flow rate when cooling is "
                                            "required ({:.7T}).",
                                            state.dataHVACMultiSpdHP->CurrentModuleObject,
                                            mshp.FanVolFlow,
                                            mshp.FanName,
                                            mshp.CoolVolumeFlowRate(mshp.NumOfSpeedCooling)));
                    ShowContinueError(
                        state, " The MSHP system flow rate when cooling is required is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state,
                                      format(" Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, mshp.Name));
                    mshp.CoolVolumeFlowRate(mshp.NumOfSpeedCooling) = mshp.FanVolFlow;
                    // Check flow rates in other speeds and ensure flow rates are not above the max flow rate
                    for (i = mshp.NumOfSpeedCooling - 1; i >= 1; --i) {
                        if (mshp.CoolVolumeFlowRate(i) > mshp.CoolVolumeFlowRate(i + 1)) {
                            ShowContinueError(state,
                                              format(" The MSHP system flow rate when cooling is required is reset to the flow rate at higher speed "
                                                     "and the simulation continues at Speed{}.",
                                                     i));
                            ShowContinueError(
                                state, format(" Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, mshp.Name));
                            mshp.CoolVolumeFlowRate(i) = mshp.CoolVolumeFlowRate(i + 1);
                        }
                    }
                }
                if (mshp.FanVolFlow < mshp.HeatVolumeFlowRate(mshp.NumOfSpeedHeating)) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the MSHP system air flow rate when heating is "
                                            "required ({:.7T}).",
                                            state.dataHVACMultiSpdHP->CurrentModuleObject,
                                            mshp.FanVolFlow,
                                            mshp.FanName,
                                            mshp.HeatVolumeFlowRate(mshp.NumOfSpeedHeating)));
                    ShowContinueError(
                        state, " The MSHP system flow rate when heating is required is reset to the fan flow rate and the simulation continues.");
                    ShowContinueError(state,
                                      format(" Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, mshp.Name));
                    mshp.HeatVolumeFlowRate(mshp.NumOfSpeedHeating) = mshp.FanVolFlow;
                    for (i = mshp.NumOfSpeedHeating - 1; i >= 1; --i) {
                        if (mshp.HeatVolumeFlowRate(i) > mshp.HeatVolumeFlowRate(i + 1)) {
                            ShowContinueError(state,
                                              format(" The MSHP system flow rate when heating is required is reset to the flow rate at higher speed "
                                                     "and the simulation continues at Speed{}.",
                                                     i));
                            ShowContinueError(
                                state,
                                format(" Occurs in {} system = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, mshp.Name));
                            mshp.HeatVolumeFlowRate(i) = mshp.HeatVolumeFlowRate(i + 1);
                        }
                    }
                }
                if (mshp.FanVolFlow < mshp.IdleVolumeAirRate &&
                    mshp.IdleVolumeAirRate != 0.0) {
                    ShowWarningError(state,
                                     format("{} - air flow rate = {:.7T} in fan object {} is less than the MSHP system air flow rate when no heating "
                                            "or cooling is needed ({:.7T}).",
                                            state.dataHVACMultiSpdHP->CurrentModuleObject,
                                            mshp.FanVolFlow,
                                            mshp.FanName,
                                            mshp.IdleVolumeAirRate));
                    ShowContinueError(state,
                                      " The MSHP system flow rate when no heating or cooling is needed is reset to the fan flow rate and the "
                                      "simulation continues.");
                    ShowContinueError(state,
                                      format(" Occurs in {} = {}", state.dataHVACMultiSpdHP->CurrentModuleObject, mshp.Name));
                    mshp.IdleVolumeAirRate = mshp.FanVolFlow;
                }
                RhoAir = state.dataEnvrn->StdRhoAir;
                // set the mass flow rates from the reset volume flow rates
                for (i = 1; i <= mshp.NumOfSpeedCooling; ++i) {
                    mshp.CoolMassFlowRate(i) = RhoAir * mshp.CoolVolumeFlowRate(i);
                    if (mshp.FanVolFlow > 0.0) {
                        mshp.CoolingSpeedRatio(i) =
                            mshp.CoolVolumeFlowRate(i) / mshp.FanVolFlow;
                    }
                }
                for (i = 1; i <= mshp.NumOfSpeedHeating; ++i) {
                    mshp.HeatMassFlowRate(i) = RhoAir * mshp.HeatVolumeFlowRate(i);
                    if (mshp.FanVolFlow > 0.0) {
                        mshp.HeatingSpeedRatio(i) =
                            mshp.HeatVolumeFlowRate(i) / mshp.FanVolFlow;
                    }
                }
                mshp.IdleMassFlowRate = RhoAir * mshp.IdleVolumeAirRate;
                if (mshp.FanVolFlow > 0.0) {
                    mshp.IdleSpeedRatio = mshp.IdleVolumeAirRate / mshp.FanVolFlow;
                }
                // set the node max and min mass flow rates based on reset volume flow rates
                s_node->Node(mshp.AirInletNode).MassFlowRateMax =
                    max(mshp.CoolMassFlowRate(mshp.NumOfSpeedCooling), mshp.HeatMassFlowRate(mshp.NumOfSpeedHeating));
                s_node->Node(mshp.AirInletNode).MassFlowRateMaxAvail =
                    max(mshp.CoolMassFlowRate(mshp.NumOfSpeedCooling), mshp.HeatMassFlowRate(mshp.NumOfSpeedHeating));
                s_node->Node(mshp.AirInletNode).MassFlowRateMin = 0.0;
                s_node->Node(mshp.AirInletNode).MassFlowRateMinAvail = 0.0;
                s_node->Node(mshp.AirOutletNode) = s_node->Node(mshp.AirInletNode);
                mshp.CheckFanFlow = false;
            }
        }

        if (mshp.fanOpModeSched != nullptr) {
            mshp.fanOp = (mshp.fanOpModeSched->getCurrentVal() == 0.0) ? HVAC::FanOp::Cycling : HVAC::FanOp::Continuous;
        }

        // Calculate air distribution losses
        if (!FirstHVACIteration && state.dataHVACMultiSpdHP->AirLoopPass == 1) {
            int ZoneInNode = mshp.ZoneInletNode;
            DeltaMassRate = s_node->Node(mshp.AirOutletNode).MassFlowRate -
                            s_node->Node(ZoneInNode).MassFlowRate / mshp.FlowFraction;
            if (DeltaMassRate < 0.0) DeltaMassRate = 0.0;
            Real64 MassFlowRate(0.0);        // parent mass flow rate
            Real64 LatentOutput(0.0);        // latent output rate
            Real64 TotalOutput(0.0);         // total output rate
            Real64 SensibleOutputDelta(0.0); // delta sensible output rate
            Real64 LatentOutputDelta(0.0);   // delta latent output rate
            Real64 TotalOutputDelta(0.0);    // delta total output rate
            MassFlowRate = s_node->Node(ZoneInNode).MassFlowRate / mshp.FlowFraction;
            Real64 MinHumRat = s_node->Node(ZoneInNode).HumRat;
            if (s_node->Node(mshp.AirOutletNode).Temp < s_node->Node(mshp.NodeNumOfControlledZone).Temp)
                MinHumRat = s_node->Node(mshp.AirOutletNode).HumRat;
            CalcZoneSensibleLatentOutput(MassFlowRate,
                                         s_node->Node(mshp.AirOutletNode).Temp,
                                         MinHumRat,
                                         s_node->Node(ZoneInNode).Temp,
                                         MinHumRat,
                                         mshp.LoadLoss,
                                         LatentOutput,
                                         TotalOutput);
            CalcZoneSensibleLatentOutput(DeltaMassRate,
                                         s_node->Node(mshp.AirOutletNode).Temp,
                                         MinHumRat,
                                         s_node->Node(mshp.NodeNumOfControlledZone).Temp,
                                         MinHumRat,
                                         SensibleOutputDelta,
                                         LatentOutputDelta,
                                         TotalOutputDelta);
            mshp.LoadLoss = mshp.LoadLoss + SensibleOutputDelta;
            if (std::abs(mshp.LoadLoss) < 1.0e-6) mshp.LoadLoss = 0.0;
        }

        // Returns load only for zones requesting cooling (heating). If in deadband, Qzoneload = 0.
        ZoneNum = mshp.ControlZoneNum;
        if ((mshp.ZoneSequenceCoolingNum > 0) && (mshp.ZoneSequenceHeatingNum > 0)) {
            ZoneLoadToCoolSPSequenced = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(mshp.ControlZoneNum)
                                            .SequencedOutputRequiredToCoolingSP(mshp.ZoneSequenceCoolingNum);
            ZoneLoadToHeatSPSequenced = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(mshp.ControlZoneNum)
                                            .SequencedOutputRequiredToHeatingSP(mshp.ZoneSequenceHeatingNum);
            if (ZoneLoadToHeatSPSequenced > HVAC::SmallLoad && ZoneLoadToCoolSPSequenced > HVAC::SmallLoad) {
                QZnReq = ZoneLoadToHeatSPSequenced;
            } else if (ZoneLoadToHeatSPSequenced < (-1.0 * HVAC::SmallLoad) && ZoneLoadToCoolSPSequenced < (-1.0 * HVAC::SmallLoad)) {
                QZnReq = ZoneLoadToCoolSPSequenced;
            } else if (ZoneLoadToHeatSPSequenced <= (-1.0 * HVAC::SmallLoad) && ZoneLoadToCoolSPSequenced >= HVAC::SmallLoad) {
                QZnReq = 0.0;
            } else {
                QZnReq = 0.0; // Autodesk:Init Case added to prevent use of uninitialized value (occurred in MultiSpeedACFurnace example)
            }
            QZnReq /= mshp.FlowFraction;
        } else {
            QZnReq = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).RemainingOutputRequired / mshp.FlowFraction;
        }
        if (state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) {
            QZnReq = 0.0;
        }

        if (QZnReq > HVAC::SmallLoad) {
            mshp.HeatCoolMode = ModeOfOperation::HeatingMode;
        } else if (QZnReq < (-1.0 * HVAC::SmallLoad)) {
            mshp.HeatCoolMode = ModeOfOperation::CoolingMode;
        } else {
            mshp.HeatCoolMode = ModeOfOperation::Invalid;
        }

        // Determine the staged status
        if (allocated(state.dataZoneCtrls->StageZoneLogic)) {
            if (state.dataZoneCtrls->StageZoneLogic(ZoneNum)) {
                mshp.Staged = true;
                mshp.StageNum = state.dataZoneEnergyDemand->ZoneSysEnergyDemand(ZoneNum).StageNum;
            } else {
                if (mshp.MyStagedFlag) {
                    ShowWarningError(state,
                                     "ZoneControl:Thermostat:StagedDualSetpoint is found, but is not applied to this "
                                     "AirLoopHVAC:UnitaryHeatPump:AirToAir:MultiSpeed object = ");
                    ShowContinueError(state, format("{}. Please make correction. Simulation continues...", mshp.Name));
                    mshp.MyStagedFlag = false;
                }
            }
        }
        // Set the inlet node mass flow rate
        if (mshp.fanOp == HVAC::FanOp::Continuous) {
            // constant fan mode
            if (QZnReq > HVAC::SmallLoad && !state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) {
                state.dataHVACMultiSpdHP->CompOnMassFlow = mshp.HeatMassFlowRate(1);
                state.dataHVACMultiSpdHP->CompOnFlowRatio = mshp.HeatingSpeedRatio(1);
                mshp.LastMode = ModeOfOperation::HeatingMode;
            } else if (QZnReq < (-1.0 * HVAC::SmallLoad) && !state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) {
                state.dataHVACMultiSpdHP->CompOnMassFlow = mshp.CoolMassFlowRate(1);
                state.dataHVACMultiSpdHP->CompOnFlowRatio = mshp.CoolingSpeedRatio(1);
                mshp.LastMode = ModeOfOperation::CoolingMode;
            } else {
                state.dataHVACMultiSpdHP->CompOnMassFlow = mshp.IdleMassFlowRate;
                state.dataHVACMultiSpdHP->CompOnFlowRatio = mshp.IdleSpeedRatio;
            }
            state.dataHVACMultiSpdHP->CompOffMassFlow = mshp.IdleMassFlowRate;
            state.dataHVACMultiSpdHP->CompOffFlowRatio = mshp.IdleSpeedRatio;
        } else {
            // cycling fan mode
            if (QZnReq > HVAC::SmallLoad && !state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) {
                state.dataHVACMultiSpdHP->CompOnMassFlow = mshp.HeatMassFlowRate(1);
                state.dataHVACMultiSpdHP->CompOnFlowRatio = mshp.HeatingSpeedRatio(1);
            } else if (QZnReq < (-1.0 * HVAC::SmallLoad) && !state.dataZoneEnergyDemand->CurDeadBandOrSetback(ZoneNum)) {
                state.dataHVACMultiSpdHP->CompOnMassFlow = mshp.CoolMassFlowRate(1);
                state.dataHVACMultiSpdHP->CompOnFlowRatio = mshp.CoolingSpeedRatio(1);
            } else {
                state.dataHVACMultiSpdHP->CompOnMassFlow = 0.0;
                state.dataHVACMultiSpdHP->CompOnFlowRatio = 0.0;
            }
            state.dataHVACMultiSpdHP->CompOffMassFlow = 0.0;
            state.dataHVACMultiSpdHP->CompOffFlowRatio = 0.0;
        }

        // Set the inlet node mass flow rate
        if (mshp.availSched->getCurrentVal() > 0.0 && state.dataHVACMultiSpdHP->CompOnMassFlow != 0.0) {
            OnOffAirFlowRatio = 1.0;
            if (FirstHVACIteration) {
                s_node->Node(mshp.AirInletNode).MassFlowRate = state.dataHVACMultiSpdHP->CompOnMassFlow;
                PartLoadFrac = 0.0;
            } else {
                if (mshp.HeatCoolMode != ModeOfOperation::Invalid) {
                    PartLoadFrac = 1.0;
                } else {
                    PartLoadFrac = 0.0;
                }
            }
        } else {
            PartLoadFrac = 0.0;
            s_node->Node(mshp.AirInletNode).MassFlowRate = 0.0;
            s_node->Node(mshp.AirOutletNode).MassFlowRate = 0.0;
            s_node->Node(mshp.AirOutletNode).MassFlowRateMaxAvail = 0.0;
            OnOffAirFlowRatio = 1.0;
        }

        // Check availability of DX coils
        if (mshp.availSched->getCurrentVal() > 0.0) {
            if (mshp.HeatCoolMode == ModeOfOperation::CoolingMode) {
                auto *coilAvailSched = DXCoils::GetCoilAvailSched(state, mshp.CoolCoilNum); // TODO: Why isn't this stored on the struct?
                if (coilAvailSched->getCurrentVal() == 0.0) {
                    if (mshp.CoolCountAvail == 0) {
                        ++mshp.CoolCountAvail;
                        ShowWarningError(
                            state,
                            format("{} is ready to perform cooling, but its DX cooling coil = {} is not available at Available Schedule = {}.",
                                   mshp.Name,
                                   mshp.CoolCoilName,
                                   coilAvailSched->Name));
                        ShowContinueErrorTimeStamp(state, format("Availability schedule returned={:.1R}", coilAvailSched->getCurrentVal()));
                    } else {
                        ++mshp.CoolCountAvail;
                        ShowRecurringWarningErrorAtEnd(state,
                                                       mshp.Name + ": Cooling coil is still not available ...",
                                                       mshp.CoolIndexAvail,
                                                       coilAvailSched->getCurrentVal(),
                                                       coilAvailSched->getCurrentVal());
                    }
                }
            }

            if (mshp.HeatCoolMode == ModeOfOperation::HeatingMode &&
                mshp.heatCoilType == HVAC::CoilType::HeatingDXMultiSpeed) {
                auto *coilAvailSched = DXCoils::GetCoilAvailSched(state, mshp.HeatCoilNum);
                if (coilAvailSched->getCurrentVal() == 0.0) {
                    if (mshp.HeatCountAvail == 0) {
                        ++mshp.HeatCountAvail;
                        ShowWarningError(
                            state,
                            format("{} is ready to perform heating, but its DX heating coil = {} is not available at Available Schedule = {}.",
                                   mshp.Name,
                                   mshp.CoolCoilName,
                                   coilAvailSched->Name));
                        ShowContinueErrorTimeStamp(state, format("Availability schedule returned={:.1R}", coilAvailSched->getCurrentVal()));
                    } else {
                        ++mshp.HeatCountAvail;
                        ShowRecurringWarningErrorAtEnd(state,
                                                       mshp.Name + ": Heating coil is still not available ...",
                                                       mshp.HeatIndexAvail,
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

        mshp.TotHeatEnergyRate = 0.0;
        mshp.SensHeatEnergyRate = 0.0;
        mshp.LatHeatEnergyRate = 0.0;
        mshp.TotCoolEnergyRate = 0.0;
        mshp.SensCoolEnergyRate = 0.0;
        mshp.LatCoolEnergyRate = 0.0;

        // If unit is scheduled OFF, setpoint is equal to inlet node temperature.
        //!!LKL Discrepancy with < 0
        if (mshp.availSched->getCurrentVal() == 0.0) {
            s_node->Node(mshp.AirOutletNode).Temp = s_node->Node(mshp.AirInletNode).Temp;
            return;
        }

        if ((mshp.HeatCoolMode == ModeOfOperation::Invalid && mshp.fanOp == HVAC::FanOp::Cycling) ||
            state.dataHVACMultiSpdHP->CompOnMassFlow == 0.0) {
            QZnReq = 0.0;
            PartLoadFrac = 0.0;
            s_node->Node(mshp.AirInletNode).MassFlowRate = 0.0;
            s_node->Node(mshp.AirOutletNode).MassFlowRateMaxAvail = 0.0;
        }
        mshp.LoadMet = 0.0;
        SetAverageAirFlow(state, MSHeatPumpNum, PartLoadFrac, OnOffAirFlowRatio);

        // Init maximum available Heat Recovery flow rate
        if ((mshp.HeatRecActive) && (!mshp.MyPlantScantFlag)) {
            if (PartLoadFrac > 0.0) {
                if (FirstHVACIteration) {
                    MdotHR = mshp.DesignHeatRecMassFlowRate;
                } else {
                    if (mshp.HeatRecoveryMassFlowRate > 0.0) {
                        MdotHR = mshp.HeatRecoveryMassFlowRate;
                    } else {
                        MdotHR = mshp.DesignHeatRecMassFlowRate;
                    }
                }
            } else {
                MdotHR = 0.0;
            }

            PlantUtilities::SetComponentFlowRate(state,
                                                 MdotHR,
                                                 mshp.HeatRecFluidInletNode,
                                                 mshp.HeatRecFluidOutletNode,
                                                 mshp.HRPlantLoc);
        }

        // get operating capacity of water and steam coil
        if (FirstHVACIteration) {
            if (mshp.heatCoilType == HVAC::CoilType::HeatingWater) {
                //     set air-side and steam-side mass flow rates
                s_node->Node(mshp.HeatCoilAirInletNode).MassFlowRate = state.dataHVACMultiSpdHP->CompOnMassFlow;
                mdot = mshp.HeatCoilMaxFluidFlow;
                PlantUtilities::SetComponentFlowRate(state,
                                                     mdot,
                                                     mshp.HeatCoilControlNode,
                                                     mshp.HeatCoilFluidOutletNode,
                                                     mshp.HeatCoilPlantLoc);
                //     simulate water coil to find operating capacity
                WaterCoils::SimulateWaterCoilComponents(state, mshp.HeatCoilName, FirstHVACIteration, mshp.HeatCoilNum, QActual);

            } else if (mshp.heatCoilType == HVAC::CoilType::HeatingSteam) {

                //     set air-side and steam-side mass flow rates
                s_node->Node(mshp.HeatCoilAirInletNode).MassFlowRate = state.dataHVACMultiSpdHP->CompOnMassFlow;
                mdot = mshp.HeatCoilMaxFluidFlow;
                PlantUtilities::SetComponentFlowRate(state,
                                                     mdot,
                                                     mshp.HeatCoilControlNode,
                                                     mshp.HeatCoilFluidOutletNode,
                                                     mshp.HeatCoilPlantLoc);

                //     simulate steam coil to find operating capacity
                SteamCoils::SimulateSteamCoilComponents(state,
                                                        mshp.HeatCoilName,
                                                        FirstHVACIteration,
                                                        mshp.HeatCoilNum,
                                                        1.0,
                                                        QActual); // QCoilReq, simulate any load > 0 to get max capacity of steam coil

            }
            
            if (mshp.suppHeatCoilType == HVAC::CoilType::HeatingWater) {
                //     set air-side and steam-side mass flow rates
                s_node->Node(mshp.SuppHeatCoilAirInletNode).MassFlowRate = state.dataHVACMultiSpdHP->CompOnMassFlow;
                mdot = mshp.SuppHeatCoilMaxFluidFlow;
                PlantUtilities::SetComponentFlowRate(state,
                                                     mdot,
                                                     mshp.SuppHeatCoilControlNode,
                                                     mshp.SuppHeatCoilFluidOutletNode,
                                                     mshp.SuppHeatCoilPlantLoc);
                //     simulate water coil to find operating capacity
                WaterCoils::SimulateWaterCoilComponents(
                    state, mshp.SuppHeatCoilName, FirstHVACIteration, mshp.SuppHeatCoilNum, QActual);
                mshp.DesignSuppHeatingCapacity = QActual;

            } else if (mshp.suppHeatCoilType == HVAC::CoilType::HeatingSteam) {

                //     set air-side and steam-side mass flow rates
                s_node->Node(mshp.SuppHeatCoilAirInletNode).MassFlowRate = state.dataHVACMultiSpdHP->CompOnMassFlow;
                mdot = mshp.SuppHeatCoilMaxFluidFlow;
                PlantUtilities::SetComponentFlowRate(state,
                                                     mdot,
                                                     mshp.SuppHeatCoilControlNode,
                                                     mshp.SuppHeatCoilFluidOutletNode,
                                                     mshp.SuppHeatCoilPlantLoc);

                //     simulate steam coil to find operating capacity
                SteamCoils::SimulateSteamCoilComponents(state,
                                                        mshp.SuppHeatCoilNum,
                                                        FirstHVACIteration,
                                                        1.0,
                                                        QActual); // QCoilReq, simulate any load > 0 to get max capacity of steam coil
                mshp.DesignSuppHeatingCapacity = SteamCoils::GetCoilCapacity(state, mshp.SuppHeatCoilNum);

            } // from IF(mshp%SuppHeatCoilType == Coil_HeatingSteam) THEN
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
        constexpr std::string_view routineName = "SizeMSHeatPump";
      
        auto &mshp = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);

        if (state.dataSize->CurSysNum > 0 && state.dataSize->CurOASysNum == 0) {
            state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).supFanNum = mshp.FanNum;
            state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).supFanType = mshp.fanType;
            state.dataAirSystemsData->PrimaryAirSystems(state.dataSize->CurSysNum).supFanPlace = mshp.fanPlace;
        }

        mshp.isHeatPump =
            mshp.coolCoilType == HVAC::CoilType::CoolingDXMultiSpeed && mshp.heatCoilType == HVAC::CoilType::HeatingDXMultiSpeed;
        Real64 SysTotCoolingLoad;
        Real64 SysCoolingLoad;
        Real64 SysCoolingCapacity;
        Real64 SysLatCoolingLoad;
        Real64 SysHeatingCapacity;
        Real64 SysHeatingLoad;
        if (mshp.isHeatPump && !state.dataSize->FinalSysSizing.empty() &&
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

            auto const &thisCoolingCoil = state.dataDXCoils->DXCoil(mshp.CoolCoilNum);
            Real64 TempSize = thisCoolingCoil.MSRatedTotCap(mshp.NumOfSpeedCooling);
            CoolingCapacitySizer sizingCoolingCapacity;
            sizingCoolingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, routineName);
            SysCoolingCapacity = sizingCoolingCapacity.size(state, TempSize, errorsFound);
            SysCoolingLoad = finalSysSizing.SensCoolCap;
            SysTotCoolingLoad = finalSysSizing.TotCoolCap;
            SysLatCoolingLoad = SysTotCoolingLoad - SysCoolingLoad;

            if (finalSysSizing.SizingOption == DataSizing::SizingConcurrence::NonCoincident) {
                state.dataSize->DataFlowUsedForSizing = finalSysSizing.NonCoinHeatMassFlow / state.dataEnvrn->StdRhoAir;
            } else {
                state.dataSize->DataFlowUsedForSizing = finalSysSizing.CoinHeatMassFlow / state.dataEnvrn->StdRhoAir;
            }

            auto const &thisHeatingCoil = state.dataDXCoils->DXCoil(mshp.HeatCoilNum);
            TempSize = thisHeatingCoil.MSRatedTotCap(mshp.NumOfSpeedHeating);
            HeatingCapacitySizer sizingHeatingCapacity;
            sizingHeatingCapacity.initializeWithinEP(state, CompType, CompName, PrintFlag, routineName);
            SysHeatingCapacity = sizingHeatingCapacity.size(state, TempSize, errorsFound) / mshp.HeatingSizingRatio;
            SysHeatingLoad = finalSysSizing.HeatCap;

            DataSizing::setHeatPumpSize(state, SysCoolingCapacity, SysHeatingCapacity, mshp.HeatingSizingRatio);
            auto &EqSizing = state.dataSize->UnitarySysEqSizing(state.dataSize->CurSysNum);
            EqSizing.CoolingCapacity = true;
            EqSizing.DesCoolingLoad = SysCoolingCapacity;
            EqSizing.HeatingCapacity = true;
            EqSizing.DesHeatingLoad = SysHeatingCapacity;
            state.dataSize->DataFlowUsedForSizing = 0.0;
        }

        for (int i = mshp.NumOfSpeedCooling; i >= 1; --i) {
            if (mshp.CoolVolumeFlowRate(i) == DataSizing::AutoSize) {
                if (state.dataSize->CurSysNum > 0) {
                    if (i == mshp.NumOfSpeedCooling) {
                        CheckSysSizing(state, state.dataHVACMultiSpdHP->CurrentModuleObject, mshp.Name);
                        mshp.CoolVolumeFlowRate(i) = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow;
                        if (mshp.FanVolFlow < mshp.CoolVolumeFlowRate(i) && mshp.FanVolFlow != DataSizing::AutoSize) {
                            mshp.CoolVolumeFlowRate(i) = mshp.FanVolFlow;
                            ShowWarningError(state, format("{} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, mshp.Name));
                            ShowContinueError(state,
                                              "The supply air flow rate at high speed is less than the autosized value for the supply air flow rate "
                                              "in cooling mode. Consider autosizing the fan for this simulation.");
                            ShowContinueError(
                                state,
                                "The air flow rate at high speed in cooling mode is reset to the supply air flow rate and the simulation continues.");
                        }
                    } else {
                        mshp.CoolVolumeFlowRate(i) = mshp.CoolVolumeFlowRate(mshp.NumOfSpeedCooling) * i / mshp.NumOfSpeedCooling;
                    }
                    if (mshp.CoolVolumeFlowRate(i) < HVAC::SmallAirVolFlow) {
                        mshp.CoolVolumeFlowRate = 0.0;
                    }
                    // Ensure the flow rate at lower speed has to be less or equal to the flow rate at higher speed
                    if (i != mshp.NumOfSpeedCooling) {
                        if (mshp.CoolVolumeFlowRate(i) > mshp.CoolVolumeFlowRate(i + 1)) {
                            mshp.CoolVolumeFlowRate(i) = mshp.CoolVolumeFlowRate(i + 1);
                        }
                    }
                    BaseSizer::reportSizerOutput(state,
                                                 state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                 mshp.Name,
                                                 format("Speed {} Supply Air Flow Rate During Cooling Operation [m3/s]", i),
                                                 mshp.CoolVolumeFlowRate(i));
                }
            }
        }

        for (int i = mshp.NumOfSpeedHeating; i >= 1; --i) {
            if (mshp.HeatVolumeFlowRate(i) == DataSizing::AutoSize) {
                if (state.dataSize->CurSysNum > 0) {
                    if (i == mshp.NumOfSpeedHeating) {
                        CheckSysSizing(state, state.dataHVACMultiSpdHP->CurrentModuleObject, mshp.Name);
                        mshp.HeatVolumeFlowRate(i) = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow;
                        if (mshp.FanVolFlow < mshp.HeatVolumeFlowRate(i) && mshp.FanVolFlow != DataSizing::AutoSize) {
                            mshp.HeatVolumeFlowRate(i) = mshp.FanVolFlow;
                            ShowWarningError(state, format("{} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, mshp.Name));
                            ShowContinueError(state,
                                              "The supply air flow rate at high speed is less than the autosized value for the maximum air flow rate "
                                              "in heating mode. Consider autosizing the fan for this simulation.");
                            ShowContinueError(state,
                                              "The maximum air flow rate at high speed in heating mode is reset to the supply air flow rate and the "
                                              "simulation continues.");
                        }
                    } else {
                        mshp.HeatVolumeFlowRate(i) = mshp.HeatVolumeFlowRate(mshp.NumOfSpeedHeating) * i / mshp.NumOfSpeedHeating;
                    }
                    if (mshp.HeatVolumeFlowRate(i) < HVAC::SmallAirVolFlow) {
                        mshp.HeatVolumeFlowRate(i) = 0.0;
                    }
                    // Ensure the flow rate at lower speed has to be less or equal to the flow rate at higher speed
                    if (i != mshp.NumOfSpeedHeating) {
                        if (mshp.HeatVolumeFlowRate(i) > mshp.HeatVolumeFlowRate(i + 1)) {
                            mshp.HeatVolumeFlowRate(i) = mshp.HeatVolumeFlowRate(i + 1);
                        }
                    }
                    BaseSizer::reportSizerOutput(state,
                                                 state.dataHVACMultiSpdHP->CurrentModuleObject,
                                                 mshp.Name,
                                                 format("Speed{}Supply Air Flow Rate During Heating Operation [m3/s]", i),
                                                 mshp.HeatVolumeFlowRate(i));
                }
            }
        }

        if (mshp.IdleVolumeAirRate == DataSizing::AutoSize) {
            if (state.dataSize->CurSysNum > 0) {
                CheckSysSizing(state, state.dataHVACMultiSpdHP->CurrentModuleObject, mshp.Name);
                mshp.IdleVolumeAirRate = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).DesMainVolFlow;
                if (mshp.FanVolFlow < mshp.IdleVolumeAirRate && mshp.FanVolFlow != DataSizing::AutoSize) {
                    mshp.IdleVolumeAirRate = mshp.FanVolFlow;
                    ShowWarningError(state, format("{} \"{}\"", state.dataHVACMultiSpdHP->CurrentModuleObject, mshp.Name));
                    ShowContinueError(state,
                                      "The supply air flow rate is less than the autosized value for the maximum air flow rate when no heating or "
                                      "cooling is needed. Consider autosizing the fan for this simulation.");
                    ShowContinueError(state,
                                      "The maximum air flow rate when no heating or cooling is needed is reset to the supply air flow rate and the "
                                      "simulation continues.");
                }
                if (mshp.IdleVolumeAirRate < HVAC::SmallAirVolFlow) {
                    mshp.IdleVolumeAirRate = 0.0;
                }

                BaseSizer::reportSizerOutput(state,
                                             state.dataHVACMultiSpdHP->CurrentModuleObject,
                                             mshp.Name,
                                             "Supply Air Flow Rate When No Cooling or Heating is Needed [m3/s]",
                                             mshp.IdleVolumeAirRate);
            }
        }

        if (mshp.SuppMaxAirTemp == DataSizing::AutoSize) {
            if (state.dataSize->CurSysNum > 0) {
                CheckZoneSizing(state, HVAC::coilTypeNames[(int)mshp.suppHeatCoilType], mshp.Name);
                mshp.SuppMaxAirTemp = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).HeatSupTemp;
                BaseSizer::reportSizerOutput(state,
                                             state.dataHVACMultiSpdHP->CurrentModuleObject,
                                             mshp.Name,
                                             "Maximum Supply Air Temperature from Supplemental Heater [C]",
                                             mshp.SuppMaxAirTemp);
            }
        }

        if (mshp.DesignSuppHeatingCapacity == DataSizing::AutoSize) {
            if (state.dataSize->CurSysNum > 0) {
                CheckSysSizing(state, HVAC::coilTypeNames[(int)mshp.suppHeatCoilType], mshp.Name);
                mshp.DesignSuppHeatingCapacity = state.dataSize->FinalSysSizing(state.dataSize->CurSysNum).HeatCap;
            } else {
                mshp.DesignSuppHeatingCapacity = 0.0;
            }
            BaseSizer::reportSizerOutput(state,
                                         state.dataHVACMultiSpdHP->CurrentModuleObject,
                                         mshp.Name,
                                         "Supplemental Heating Coil Nominal Capacity [W]",
                                         mshp.DesignSuppHeatingCapacity);
        }
        state.dataSize->SuppHeatCap = mshp.DesignSuppHeatingCapacity;

        if (mshp.HeatRecActive) {
            PlantUtilities::RegisterPlantCompDesignFlow(state, mshp.HeatRecFluidInletNode, mshp.DesignHeatRecFlowRate);
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

        auto &mshp = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);

        // Get EMS output
        SpeedNum = ceil(SpeedVal);
        bool useMaxedSpeed = false;
        std::string useMaxedSpeedCoilName;
        if (mshp.HeatCoolMode == ModeOfOperation::HeatingMode) {
            if (SpeedNum > mshp.NumOfSpeedHeating) {
                SpeedNum = mshp.NumOfSpeedHeating;
                useMaxedSpeed = true;
                useMaxedSpeedCoilName = mshp.HeatCoilName;
            }
        } else if (mshp.HeatCoolMode == ModeOfOperation::CoolingMode) {
            if (SpeedNum > mshp.NumOfSpeedCooling) {
                SpeedNum = mshp.NumOfSpeedCooling;
                useMaxedSpeed = true;
                useMaxedSpeedCoilName = mshp.CoolCoilName;
            }
        }
        if (useMaxedSpeed) {
            mshp.CoilSpeedErrIndex++;
            ShowRecurringWarningErrorAtEnd(state,
                                           "Wrong coil speed EMS override value, for unit=\"" + useMaxedSpeedCoilName +
                                               "\". Exceeding maximum coil speed level. Speed level is set to the maximum coil speed level allowed.",
                                           mshp.CoilSpeedErrIndex,
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
        auto &s_node = state.dataLoopNodes;
        auto &mshp = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);

        if ((QZnReq > HVAC::SmallLoad && QZnReq > EMSOutput)) {
            Real64 TempOutput;
            if (state.dataEnvrn->OutDryBulbTemp <= mshp.SuppMaxAirTemp) {
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
        if (s_node->Node(mshp.AirOutletNode).Temp > mshp.SuppMaxAirTemp && SupHeaterLoad > 0.0) {

            //   If the supply air temperature is to high, turn off the supplemental heater to recalculate the outlet temperature
            SupHeaterLoad = 0.0;
            Real64 QCoilActual; // coil load actually delivered returned to calling component
            CalcNonDXHeatingCoils(state, MSHeatPumpNum, FirstHVACIteration, SupHeaterLoad, fanOp, QCoilActual);

            //   If the outlet temperature is below the maximum supplemental heater supply air temperature, reduce the load passed to
            //   the supplemental heater, otherwise leave the supplemental heater off. If the supplemental heater is to be turned on,
            //   use the outlet conditions when the supplemental heater was off (CALL above) as the inlet conditions for the calculation
            //   of supplemental heater load to just meet the maximum supply air temperature from the supplemental heater.
            if (s_node->Node(mshp.AirOutletNode).Temp < mshp.SuppMaxAirTemp) {
                Real64 CpAir = Psychrometrics::PsyCpAirFnW(s_node->Node(mshp.AirOutletNode).HumRat);
                SupHeaterLoad = s_node->Node(mshp.AirInletNode).MassFlowRate * CpAir *
                                (mshp.SuppMaxAirTemp - s_node->Node(mshp.AirOutletNode).Temp);

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

        auto &s_node = state.dataLoopNodes;
        
        auto &mshp = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);

        //!!LKL Discrepancy with < 0
        if (mshp.availSched->getCurrentVal() == 0.0) return;

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
        if (mshp.HeatCoolMode == ModeOfOperation::HeatingMode) {
            SpeedNum = mshp.NumOfSpeedHeating;
            if (mshp.Staged && std::abs(mshp.StageNum) < SpeedNum) {
                SpeedNum = std::abs(mshp.StageNum);
                if (SpeedNum == 1) SpeedRatio = 0.0;
            }
        } else if (mshp.HeatCoolMode == ModeOfOperation::CoolingMode) {
            SpeedNum = mshp.NumOfSpeedCooling;
            if (mshp.Staged && std::abs(mshp.StageNum) < SpeedNum) {
                SpeedNum = std::abs(mshp.StageNum);
                if (SpeedNum == 1) SpeedRatio = 0.0;
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
                if (mshp.Staged && SpeedNum == 1) SpeedRatio = 0.0;
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
        if (state.dataGlobal->DoCoilDirectSolutions && !mshp.Staged) {
            Real64 TempOutput0 = 0.0;
            mshp.FullOutput = 0.0;

            // heating
            if (QZnReq > HVAC::SmallLoad && QZnReq < FullOutput) {
                CalcMSHeatPump(
                    state, MSHeatPumpNum, FirstHVACIteration, compressorOp, 1, 0.0, 0.0, TempOutput0, QZnReq, OnOffAirFlowRatio, SupHeaterLoad);

                for (int k = 1; k <= mshp.NumOfSpeedHeating; ++k) {
                    if (k == 1) {
                        CalcMSHeatPump(state,
                                       MSHeatPumpNum,
                                       FirstHVACIteration,
                                       compressorOp,
                                       k,
                                       0.0,
                                       1.0,
                                       mshp.FullOutput(k),
                                       QZnReq,
                                       OnOffAirFlowRatio,
                                       SupHeaterLoad);
                        if (QZnReq <= mshp.FullOutput(k)) {
                            SpeedNum = k;
                            PartLoadFrac = (QZnReq - TempOutput0) / (mshp.FullOutput(k) - TempOutput0);
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
                                       mshp.FullOutput(k),
                                       QZnReq,
                                       OnOffAirFlowRatio,
                                       SupHeaterLoad);
                        if (QZnReq <= mshp.FullOutput(k)) {
                            SpeedNum = k;
                            PartLoadFrac = 1.0;
                            SpeedRatio = (QZnReq - mshp.FullOutput(k - 1)) / (mshp.FullOutput(k) - mshp.FullOutput(k - 1));
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
                for (int k = 1; k <= mshp.NumOfSpeedCooling; ++k) {
                    if (k == 1) {
                        CalcMSHeatPump(state,
                                       MSHeatPumpNum,
                                       FirstHVACIteration,
                                       compressorOp,
                                       k,
                                       0.0,
                                       1.0,
                                       mshp.FullOutput(k),
                                       QZnReq,
                                       OnOffAirFlowRatio,
                                       SupHeaterLoad);
                        if (QZnReq >= mshp.FullOutput(k)) {
                            SpeedNum = k;
                            PartLoadFrac = (QZnReq - TempOutput0) / (mshp.FullOutput(k) - TempOutput0);
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
                                       mshp.FullOutput(k),
                                       QZnReq,
                                       OnOffAirFlowRatio,
                                       SupHeaterLoad);
                        if (QZnReq >= mshp.FullOutput(k)) {
                            SpeedNum = k;
                            PartLoadFrac = 1.0;
                            SpeedRatio = (QZnReq - mshp.FullOutput(k - 1)) / (mshp.FullOutput(k) - mshp.FullOutput(k - 1));
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
                (!mshp.Staged)) {
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
                                                 format("Iteration limit exceeded calculating DX unit cycling ratio, for unit={}", mshp.Name));
                                ShowContinueErrorTimeStamp(state, format("Cycling ratio returned={:.2R}", PartLoadFrac));
                            } else {
                                ++state.dataHVACMultiSpdHP->ErrCountCyc;
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    mshp.Name + "\": Iteration limit warning exceeding calculating DX unit cycling ratio  continues...",
                                    mshp.ErrIndexCyc,
                                    PartLoadFrac,
                                    PartLoadFrac);
                            }
                        }
                    } else if (SolFla == -2) {
                        ShowFatalError(
                            state,
                            format("DX unit cycling ratio calculation failed: cycling limits exceeded, for unit={}", mshp.CoolCoilName));
                    }
                } else {
                    // Check to see which speed to meet the load
                    PartLoadFrac = 1.0;
                    SpeedRatio = 1.0;
                    if (QZnReq < (-1.0 * HVAC::SmallLoad)) { // Cooling
                        for (i = 2; i <= mshp.NumOfSpeedCooling; ++i) {
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
                        for (i = 2; i <= mshp.NumOfSpeedHeating; ++i) {
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
                                                 format("Iteration limit exceeded calculating DX unit speed ratio, for unit={}", mshp.Name));
                                ShowContinueErrorTimeStamp(state, format("Speed ratio returned=[{:.2R}], Speed number ={}", SpeedRatio, SpeedNum));
                            } else {
                                ++state.dataHVACMultiSpdHP->ErrCountVar;
                                ShowRecurringWarningErrorAtEnd(
                                    state,
                                    mshp.Name + "\": Iteration limit warning exceeding calculating DX unit speed ratio continues...",
                                    mshp.ErrIndexVar,
                                    SpeedRatio,
                                    SpeedRatio);
                            }
                        }
                    } else if (SolFla == -2) {
                        ShowFatalError(
                            state,
                            format("DX unit compressor speed calculation failed: speed limits exceeded, for unit={}", mshp.CoolCoilName));
                    }
                }
            } else {
                // Staged thermostat performance
                if (mshp.StageNum != 0) {
                    SpeedNum = std::abs(mshp.StageNum);
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
                                            format("Iteration limit exceeded calculating DX unit cycling ratio, for unit={}", mshp.Name));
                                        ShowContinueErrorTimeStamp(state, format("Cycling ratio returned={:.2R}", PartLoadFrac));
                                    } else {
                                        ++state.dataHVACMultiSpdHP->ErrCountCyc;
                                        ShowRecurringWarningErrorAtEnd(
                                            state,
                                            mshp.Name + "\": Iteration limit warning exceeding calculating DX unit cycling ratio  continues...",
                                            mshp.ErrIndexCyc,
                                            PartLoadFrac,
                                            PartLoadFrac);
                                    }
                                }
                            } else if (SolFla == -2) {
                                ShowFatalError(state,
                                               format("DX unit cycling ratio calculation failed: cycling limits exceeded, for unit={}",
                                                      mshp.CoolCoilName));
                            }
                        } else {
                            FullOutput = LowOutput;
                            PartLoadFrac = 1.0;
                        }
                    } else {
                        if (mshp.StageNum < 0) {
                            SpeedNum = min(mshp.NumOfSpeedCooling, std::abs(mshp.StageNum));
                        } else {
                            SpeedNum = min(mshp.NumOfSpeedHeating, std::abs(mshp.StageNum));
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
                                                format("Iteration limit exceeded calculating DX unit speed ratio, for unit={}", mshp.Name));
                                            ShowContinueErrorTimeStamp(
                                                state, format("Speed ratio returned=[{:.2R}], Speed number ={}", SpeedRatio, SpeedNum));
                                        } else {
                                            ++state.dataHVACMultiSpdHP->ErrCountVar;
                                            ShowRecurringWarningErrorAtEnd(
                                                state,
                                                mshp.Name +
                                                    "\": Iteration limit warning exceeding calculating DX unit speed ratio continues...",
                                                mshp.ErrIndexVar,
                                                SpeedRatio,
                                                SpeedRatio);
                                        }
                                    }
                                } else if (SolFla == -2) {
                                    ShowFatalError(state,
                                                   format("DX unit compressor speed calculation failed: speed limits exceeded, for unit={}",
                                                          mshp.CoolCoilName));
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
            if (mshp.Staged && SpeedNum == 1) SpeedRatio = 0.0;
            if (OutsideDryBulbTemp <= mshp.SuppMaxAirTemp) {
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
        if (s_node->Node(mshp.AirOutletNode).Temp > mshp.SuppMaxAirTemp && SupHeaterLoad > 0.0) {

            //   If the supply air temperature is to high, turn off the supplemental heater to recalculate the outlet temperature
            SupHeaterLoad = 0.0;
            CalcNonDXHeatingCoils(state, MSHeatPumpNum, FirstHVACIteration, SupHeaterLoad, fanOp, QCoilActual);

            //   If the outlet temperature is below the maximum supplemental heater supply air temperature, reduce the load passed to
            //   the supplemental heater, otherwise leave the supplemental heater off. If the supplemental heater is to be turned on,
            //   use the outlet conditions when the supplemental heater was off (CALL above) as the inlet conditions for the calculation
            //   of supplemental heater load to just meet the maximum supply air temperature from the supplemental heater.
            if (s_node->Node(mshp.AirOutletNode).Temp < mshp.SuppMaxAirTemp) {
                CpAir = Psychrometrics::PsyCpAirFnW(s_node->Node(mshp.AirOutletNode).HumRat);
                SupHeaterLoad = s_node->Node(mshp.AirInletNode).MassFlowRate * CpAir *
                                (mshp.SuppMaxAirTemp - s_node->Node(mshp.AirOutletNode).Temp);

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
        Real64 OutsideDryBulbTemp; // Outdoor dry bulb temperature [C]
        Real64 AirMassFlow;        // Air mass flow rate [kg/s]
        Real64 SavePartloadRatio;
        Real64 SaveSpeedRatio;
        Real64 QCoilActual;  // coil load actually delivered returned to calling component
        Real64 MinWaterFlow; // minimum water flow rate
        Real64 ErrorToler;   // supplemental heating coil convergence tolerance

        auto &s_node = state.dataLoopNodes;
        auto &mshp = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);

        if (mshp.heatCoilType == HVAC::CoilType::HeatingDXMultiSpeed && mshp.HeatCoilNum > 0) {
            auto const &dxCoil = state.dataDXCoils->DXCoil(mshp.HeatCoilNum);
            if (dxCoil.IsSecondaryDXCoilInZone) {
                OutsideDryBulbTemp = state.dataZoneTempPredictorCorrector->zoneHeatBalance(dxCoil.SecZonePtr).ZT;
            } else {
                OutsideDryBulbTemp = state.dataEnvrn->OutDryBulbTemp;
            }
        } else if (mshp.coolCoilType == HVAC::CoilType::CoolingDXMultiSpeed && mshp.CoolCoilNum > 0) {
            auto const &dxCoil = state.dataDXCoils->DXCoil(mshp.CoolCoilNum);
            if (dxCoil.IsSecondaryDXCoilInZone) {
                OutsideDryBulbTemp = state.dataZoneTempPredictorCorrector->zoneHeatBalance(dxCoil.SecZonePtr).ZT;
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

        AirMassFlow = s_node->Node(mshp.AirInletNode).MassFlowRate;
        // if blow through, simulate fan then coils
        if (mshp.fanPlace == HVAC::FanPlace::BlowThru) {
            state.dataFans->fans(mshp.FanNum)->simulate(state, FirstHVACIteration, state.dataHVACMultiSpdHP->FanSpeedRatio);
            if (QZnReq < (-1.0 * HVAC::SmallLoad)) {
                if (OutsideDryBulbTemp > mshp.MinOATCompressorCooling) {
                    DXCoils::SimDXCoilMultiSpeed(state,
                                                 mshp.CoolCoilName,
                                                 SpeedRatio,
                                                 PartLoadFrac,
                                                 mshp.CoolCoilNum,
                                                 SpeedNum,
                                                 mshp.fanOp,
                                                 compressorOp);
                    SavePartloadRatio = PartLoadFrac;
                    SaveSpeedRatio = SpeedRatio;
                } else {
                    DXCoils::SimDXCoilMultiSpeed(
                        state, mshp.CoolCoilName, 0.0, 0.0, mshp.CoolCoilNum, SpeedNum, mshp.fanOp, compressorOp);
                }
                state.dataHVACMultiSpdHP->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(mshp.CoolCoilNum);
            } else {
                DXCoils::SimDXCoilMultiSpeed(
                    state, mshp.CoolCoilName, 0.0, 0.0, mshp.CoolCoilNum, SpeedNum, mshp.fanOp, compressorOp);
            }
            
            if (mshp.heatCoilType == HVAC::CoilType::HeatingDXMultiSpeed) {
                if (QZnReq > HVAC::SmallLoad) {
                    if (OutsideDryBulbTemp > mshp.MinOATCompressorHeating) {
                        DXCoils::SimDXCoilMultiSpeed(state,
                                                     mshp.HeatCoilName,
                                                     SpeedRatio,
                                                     PartLoadFrac,
                                                     mshp.HeatCoilNum,
                                                     SpeedNum,
                                                     mshp.fanOp,
                                                     compressorOp);
                        SavePartloadRatio = PartLoadFrac;
                        SaveSpeedRatio = SpeedRatio;
                    } else {
                        DXCoils::SimDXCoilMultiSpeed(
                            state, mshp.HeatCoilName, 0.0, 0.0, mshp.HeatCoilNum, SpeedNum, mshp.fanOp, compressorOp);
                    }
                    state.dataHVACMultiSpdHP->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(mshp.HeatCoilNum);
                } else {
                    DXCoils::SimDXCoilMultiSpeed(
                        state, mshp.HeatCoilName, 0.0, 0.0, mshp.HeatCoilNum, SpeedNum, mshp.fanOp, compressorOp);
                }
                
            } else if (mshp.heatCoilType == HVAC::CoilType::HeatingElectricMultiStage ||
                       mshp.heatCoilType == HVAC::CoilType::HeatingGasMultiStage) {
                if (QZnReq > HVAC::SmallLoad) {
                    HeatingCoils::SimulateHeatingCoilComponents(
                        state, mshp.HeatCoilName, FirstHVACIteration, _, 0, _, _, mshp.fanOp, PartLoadFrac, SpeedNum, SpeedRatio);
                } else {
                    HeatingCoils::SimulateHeatingCoilComponents(
                        state, mshp.HeatCoilName, FirstHVACIteration, _, 0, _, _, mshp.fanOp, 0.0, SpeedNum, 0.0);
                }
            } else {
                CalcNonDXHeatingCoils(state, MSHeatPumpNum, FirstHVACIteration, QZnReq, mshp.fanOp, QCoilActual, PartLoadFrac);
            }
            // Call twice to ensure the fan outlet conditions are updated
            state.dataFans->fans(mshp.FanNum)->simulate(state, FirstHVACIteration, state.dataHVACMultiSpdHP->FanSpeedRatio);
            if (QZnReq < (-1.0 * HVAC::SmallLoad)) {
                if (OutsideDryBulbTemp > mshp.MinOATCompressorCooling) {
                    DXCoils::SimDXCoilMultiSpeed(state,
                                                 mshp.CoolCoilName,
                                                 SpeedRatio,
                                                 PartLoadFrac,
                                                 mshp.CoolCoilNum,
                                                 SpeedNum,
                                                 mshp.fanOp,
                                                 compressorOp);
                    SavePartloadRatio = PartLoadFrac;
                    SaveSpeedRatio = SpeedRatio;
                } else {
                    DXCoils::SimDXCoilMultiSpeed(
                        state, mshp.CoolCoilName, 0.0, 0.0, mshp.CoolCoilNum, SpeedNum, mshp.fanOp, compressorOp);
                }
                state.dataHVACMultiSpdHP->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(mshp.CoolCoilNum);
            } else {
                DXCoils::SimDXCoilMultiSpeed(
                    state, mshp.CoolCoilName, 0.0, 0.0, mshp.CoolCoilNum, SpeedNum, mshp.fanOp, compressorOp);
            }
            
            if (mshp.heatCoilType == HVAC::CoilType::HeatingDXMultiSpeed) {
                if (QZnReq > HVAC::SmallLoad) {
                    if (OutsideDryBulbTemp > mshp.MinOATCompressorHeating) {
                        DXCoils::SimDXCoilMultiSpeed(state,
                                                     mshp.HeatCoilName,
                                                     SpeedRatio,
                                                     PartLoadFrac,
                                                     mshp.HeatCoilNum,
                                                     SpeedNum,
                                                     mshp.fanOp,
                                                     compressorOp);
                        SavePartloadRatio = PartLoadFrac;
                        SaveSpeedRatio = SpeedRatio;
                    } else {
                        DXCoils::SimDXCoilMultiSpeed(
                            state, mshp.HeatCoilName, 0.0, 0.0, mshp.HeatCoilNum, SpeedNum, mshp.fanOp, compressorOp);
                    }
                    state.dataHVACMultiSpdHP->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(mshp.HeatCoilNum);
                } else {
                    DXCoils::SimDXCoilMultiSpeed(
                        state, mshp.HeatCoilName, 0.0, 0.0, mshp.HeatCoilNum, SpeedNum, mshp.fanOp, compressorOp);
                }
                
            } else if (mshp.heatCoilType == HVAC::CoilType::HeatingElectricMultiStage ||
                       mshp.heatCoilType == HVAC::CoilType::HeatingGasMultiStage) {
                if (QZnReq > HVAC::SmallLoad) {
                    HeatingCoils::SimulateHeatingCoilComponents(
                        state, mshp.HeatCoilName, FirstHVACIteration, _, 0, _, _, mshp.fanOp, PartLoadFrac, SpeedNum, SpeedRatio);
                } else {
                    HeatingCoils::SimulateHeatingCoilComponents(
                        state, mshp.HeatCoilName, FirstHVACIteration, _, 0, _, _, mshp.fanOp, 0.0, SpeedNum, 0.0);
                }
            } else {
                CalcNonDXHeatingCoils(state, MSHeatPumpNum, FirstHVACIteration, QZnReq, mshp.fanOp, QCoilActual, PartLoadFrac);
            }
            //  Simulate supplemental heating coil for blow through fan

            if (mshp.SuppHeatCoilNum > 0) {
                CalcNonDXHeatingCoils(state, MSHeatPumpNum, FirstHVACIteration, SupHeaterLoad, mshp.fanOp, QCoilActual);
            }
        } else { // otherwise simulate DX coils then fan then supplemental heater
            if (QZnReq < (-1.0 * HVAC::SmallLoad)) {
                if (OutsideDryBulbTemp > mshp.MinOATCompressorCooling) {
                    DXCoils::SimDXCoilMultiSpeed(state,
                                                 mshp.CoolCoilName,
                                                 SpeedRatio,
                                                 PartLoadFrac,
                                                 mshp.CoolCoilNum,
                                                 SpeedNum,
                                                 mshp.fanOp,
                                                 compressorOp);
                    SavePartloadRatio = PartLoadFrac;
                    SaveSpeedRatio = SpeedRatio;
                } else {
                    DXCoils::SimDXCoilMultiSpeed(
                        state, mshp.CoolCoilName, 0.0, 0.0, mshp.CoolCoilNum, SpeedNum, mshp.fanOp, compressorOp);
                }
                state.dataHVACMultiSpdHP->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(mshp.CoolCoilNum);
            } else {
                DXCoils::SimDXCoilMultiSpeed(
                    state, mshp.CoolCoilName, 0.0, 0.0, mshp.CoolCoilNum, SpeedNum, mshp.fanOp, compressorOp);
            }

            if (mshp.heatCoilType == HVAC::CoilType::HeatingDXMultiSpeed) {
                if (QZnReq > HVAC::SmallLoad) {
                    if (OutsideDryBulbTemp > mshp.MinOATCompressorHeating) {
                        DXCoils::SimDXCoilMultiSpeed(state,
                                                     mshp.HeatCoilName,
                                                     SpeedRatio,
                                                     PartLoadFrac,
                                                     mshp.HeatCoilNum,
                                                     SpeedNum,
                                                     mshp.fanOp,
                                                     compressorOp);
                        SavePartloadRatio = PartLoadFrac;
                        SaveSpeedRatio = SpeedRatio;
                    } else {
                        DXCoils::SimDXCoilMultiSpeed(
                            state, mshp.HeatCoilName, 0.0, 0.0, mshp.HeatCoilNum, SpeedNum, mshp.fanOp, compressorOp);
                    }
                    state.dataHVACMultiSpdHP->SaveCompressorPLR = state.dataDXCoils->DXCoilPartLoadRatio(mshp.HeatCoilNum);
                } else {
                    DXCoils::SimDXCoilMultiSpeed(
                        state, mshp.HeatCoilName, 0.0, 0.0, mshp.HeatCoilNum, SpeedNum, mshp.fanOp, compressorOp);
                }
            } else if (mshp.heatCoilType == HVAC::CoilType::HeatingElectricMultiStage ||
                       mshp.heatCoilType == HVAC::CoilType::HeatingGasMultiStage) {
                if (QZnReq > HVAC::SmallLoad) {
                    HeatingCoils::SimulateHeatingCoilComponents(
                        state, mshp.HeatCoilName, FirstHVACIteration, _, 0, _, _, mshp.fanOp, PartLoadFrac, SpeedNum, SpeedRatio);
                } else {
                    HeatingCoils::SimulateHeatingCoilComponents(
                        state, mshp.HeatCoilName, FirstHVACIteration, _, 0, _, _, mshp.fanOp, 0.0, SpeedNum, 0.0);
                }
            } else {
                CalcNonDXHeatingCoils(state, MSHeatPumpNum, FirstHVACIteration, QZnReq, mshp.fanOp, QCoilActual, PartLoadFrac);
            }
            state.dataFans->fans(mshp.FanNum)->simulate(state, FirstHVACIteration, state.dataHVACMultiSpdHP->FanSpeedRatio);
            //  Simulate supplemental heating coil for draw through fan
            if (mshp.SuppHeatCoilNum > 0) {
                CalcNonDXHeatingCoils(state, MSHeatPumpNum, FirstHVACIteration, SupHeaterLoad, mshp.fanOp, QCoilActual);
            }
        }

        // calculate sensible load met
        Real64 SensibleOutput(0.0); // sensible output rate
        // calculate sensible load met using delta enthalpy at a constant (minimum) humidity ratio)
        Real64 MinHumRat = s_node->Node(mshp.NodeNumOfControlledZone).HumRat;
        if (s_node->Node(mshp.AirOutletNode).Temp < s_node->Node(mshp.NodeNumOfControlledZone).Temp)
            MinHumRat = s_node->Node(mshp.AirOutletNode).HumRat;
        SensibleOutput = AirMassFlow * Psychrometrics::PsyDeltaHSenFnTdb2W2Tdb1W1(s_node->Node(mshp.AirOutletNode).Temp,
                                                                                  MinHumRat,
                                                                                  s_node->Node(mshp.NodeNumOfControlledZone).Temp,
                                                                                  MinHumRat);
        LoadMet = SensibleOutput - mshp.LoadLoss;

        mshp.LoadMet = LoadMet;
    }

    void UpdateMSHeatPump(EnergyPlusData &state, int const MSHeatPumpNum) // Engine driven heat pump number
    {
        // SUBROUTINE INFORMATION:
        //       AUTHOR:          Lixing Gu, FSEC
        //       DATE WRITTEN:    June 2007

        // PURPOSE OF THIS SUBROUTINE:
        //  This routine will update MSHP performance and calculate heat recovery rate and crankcase heater power

        auto &mshp = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);
      
        // Calculate heat recovery
        if (mshp.HeatRecActive) {
            MSHPHeatRecovery(state, MSHeatPumpNum);
        }

        if (state.afn->distribution_simulated) {
            auto &airLoopAFN = state.dataAirLoop->AirLoopAFNInfo(mshp.AirLoopNumber);
            airLoopAFN.LoopSystemOnMassFlowrate = state.dataHVACMultiSpdHP->CompOnMassFlow;
            airLoopAFN.LoopSystemOffMassFlowrate = state.dataHVACMultiSpdHP->CompOffMassFlow;
            airLoopAFN.LoopFanOperationMode = mshp.fanOp;
            airLoopAFN.LoopOnOffFanPartLoadRatio = mshp.FanPartLoadRatio;
            airLoopAFN.LoopCompCycRatio = state.dataHVACMultiSpdHP->MSHeatPumpReport(MSHeatPumpNum).CycRatio;
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

        auto &mshp = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);
        auto &MSHeatPumpReport = state.dataHVACMultiSpdHP->MSHeatPumpReport(MSHeatPumpNum);

        MSHeatPumpReport.ElecPowerConsumption = mshp.ElecPower * TimeStepSysSec; // + &
        MSHeatPumpReport.HeatRecoveryEnergy = mshp.HeatRecoveryRate * TimeStepSysSec;

        MSHeatPumpReport.AuxElecHeatConsumption = 0.0;
        MSHeatPumpReport.AuxElecCoolConsumption = 0.0;

        mshp.AuxElecPower = mshp.AuxOnCyclePower * state.dataHVACMultiSpdHP->SaveCompressorPLR +
                                  mshp.AuxOffCyclePower * (1.0 - state.dataHVACMultiSpdHP->SaveCompressorPLR);
        if (mshp.HeatCoolMode == ModeOfOperation::CoolingMode) {
            MSHeatPumpReport.AuxElecCoolConsumption = mshp.AuxOnCyclePower * state.dataHVACMultiSpdHP->SaveCompressorPLR * TimeStepSysSec;
        }
        if (mshp.HeatCoolMode == ModeOfOperation::HeatingMode) {
            MSHeatPumpReport.AuxElecHeatConsumption = mshp.AuxOnCyclePower * state.dataHVACMultiSpdHP->SaveCompressorPLR * TimeStepSysSec;
        }
        if (mshp.LastMode == ModeOfOperation::HeatingMode) {
            MSHeatPumpReport.AuxElecHeatConsumption +=
                mshp.AuxOffCyclePower * (1.0 - state.dataHVACMultiSpdHP->SaveCompressorPLR) * TimeStepSysSec;
        } else {
            MSHeatPumpReport.AuxElecCoolConsumption +=
                mshp.AuxOffCyclePower * (1.0 - state.dataHVACMultiSpdHP->SaveCompressorPLR) * TimeStepSysSec;
        }

        if (mshp.FirstPass) {
            if (!state.dataGlobal->SysSizingCalc) {
                DataSizing::resetHVACSizingGlobals(state, state.dataSize->CurZoneEqNum, state.dataSize->CurSysNum, mshp.FirstPass);
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

        auto &s_node = state.dataLoopNodes;
        auto &mshp = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);
        // Begin routine // LOL
        
        // Inlet node to the heat recovery heat exchanger
        Real64 HeatRecInletTemp = s_node->Node(mshp.HeatRecFluidInletNode).Temp;

        // Set heat recovery mass flow rates
        Real64 HeatRecMassFlowRate = s_node->Node(mshp.HeatRecFluidInletNode).MassFlowRate;

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

        PlantUtilities::SafeCopyPlantNode(state, mshp.HeatRecFluidInletNode, mshp.HeatRecFluidOutletNode);
        // changed outputs
        s_node->Node(mshp.HeatRecFluidOutletNode).Temp = HeatRecOutletTemp;

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

        auto &s_node = state.dataLoopNodes;
      
        auto &mshp = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);
        // Why? Why are these global variables?
        auto &MSHPMassFlowRateHigh = state.dataHVACGlobal->MSHPMassFlowRateHigh;
        auto &MSHPMassFlowRateLow = state.dataHVACGlobal->MSHPMassFlowRateLow;

        // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
        Real64 AverageUnitMassFlow; // average supply air mass flow rate over time step

        MSHPMassFlowRateLow = 0.0;  // Mass flow rate at low speed
        MSHPMassFlowRateHigh = 0.0; // Mass flow rate at high speed

        if (!state.dataZoneEnergyDemand->CurDeadBandOrSetback(mshp.ControlZoneNum) &&
            present(SpeedNum)) {
            if (mshp.HeatCoolMode == ModeOfOperation::HeatingMode) {
                if (SpeedNum == 1) {
                    state.dataHVACMultiSpdHP->CompOnMassFlow = mshp.HeatMassFlowRate(SpeedNum);
                    state.dataHVACMultiSpdHP->CompOnFlowRatio = mshp.HeatingSpeedRatio(SpeedNum);
                    MSHPMassFlowRateLow = mshp.HeatMassFlowRate(1);
                    MSHPMassFlowRateHigh = mshp.HeatMassFlowRate(1);
                } else if (SpeedNum > 1) {
                    state.dataHVACMultiSpdHP->CompOnMassFlow =
                        SpeedRatio * mshp.HeatMassFlowRate(SpeedNum) + (1.0 - SpeedRatio) * mshp.HeatMassFlowRate(SpeedNum - 1);
                    state.dataHVACMultiSpdHP->CompOnFlowRatio =
                        SpeedRatio * mshp.HeatingSpeedRatio(SpeedNum) + (1.0 - SpeedRatio) * mshp.HeatingSpeedRatio(SpeedNum - 1);
                    MSHPMassFlowRateLow = mshp.HeatMassFlowRate(SpeedNum - 1);
                    MSHPMassFlowRateHigh = mshp.HeatMassFlowRate(SpeedNum);
                }
                
            } else if (mshp.HeatCoolMode == ModeOfOperation::CoolingMode) {
                if (SpeedNum == 1) {
                    state.dataHVACMultiSpdHP->CompOnMassFlow = mshp.CoolMassFlowRate(SpeedNum);
                    state.dataHVACMultiSpdHP->CompOnFlowRatio = mshp.CoolingSpeedRatio(SpeedNum);
                    MSHPMassFlowRateLow = mshp.CoolMassFlowRate(1);
                    MSHPMassFlowRateHigh = mshp.CoolMassFlowRate(1);
                } else if (SpeedNum > 1) {
                    state.dataHVACMultiSpdHP->CompOnMassFlow =
                        SpeedRatio * mshp.CoolMassFlowRate(SpeedNum) + (1.0 - SpeedRatio) * mshp.CoolMassFlowRate(SpeedNum - 1);
                    state.dataHVACMultiSpdHP->CompOnFlowRatio =
                        SpeedRatio * mshp.CoolingSpeedRatio(SpeedNum) + (1.0 - SpeedRatio) * mshp.CoolingSpeedRatio(SpeedNum - 1);
                    MSHPMassFlowRateLow = mshp.CoolMassFlowRate(SpeedNum - 1);
                    MSHPMassFlowRateHigh = mshp.CoolMassFlowRate(SpeedNum);
                }
            }
        }
        
        // Set up fan flow rate during compressor off time
        if (mshp.fanOp == HVAC::FanOp::Continuous && present(SpeedNum)) {
            if (mshp.AirFlowControl == AirflowControl::UseCompressorOnFlow &&
                state.dataHVACMultiSpdHP->CompOnMassFlow > 0.0) {
                if (mshp.LastMode == ModeOfOperation::HeatingMode) {
                    state.dataHVACMultiSpdHP->CompOffMassFlow = mshp.HeatMassFlowRate(SpeedNum);
                    state.dataHVACMultiSpdHP->CompOffFlowRatio = mshp.HeatingSpeedRatio(SpeedNum);
                } else {
                    state.dataHVACMultiSpdHP->CompOffMassFlow = mshp.CoolMassFlowRate(SpeedNum);
                    state.dataHVACMultiSpdHP->CompOffFlowRatio = mshp.CoolingSpeedRatio(SpeedNum);
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
        if (mshp.availSched->getCurrentVal() == 0.0) {
            s_node->Node(mshp.AirInletNode).MassFlowRate = 0.0;
            OnOffAirFlowRatio = 0.0;
        } else {
            s_node->Node(mshp.AirInletNode).MassFlowRate = AverageUnitMassFlow;
            s_node->Node(mshp.AirInletNode).MassFlowRateMaxAvail = AverageUnitMassFlow;
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

        Real64 QCoilActual = 0.0; // actual heating load met

        auto &mshp = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);

        if (present(PartLoadFrac)) {
        
            if (HeatingLoad > HVAC::SmallLoad) {

                switch (mshp.heatCoilType) {
                case HVAC::CoilType::HeatingGasOrOtherFuel:
                case HVAC::CoilType::HeatingElectric: {
                  HeatingCoils::SimulateHeatingCoilComponents(
                      state, mshp.HeatCoilName, FirstHVACIteration, HeatingLoad, mshp.HeatCoilNum, QCoilActual, true, fanOp);
                } break;
                  
                case HVAC::CoilType::HeatingWater: {
                    Real64 MaxHotWaterFlow = mshp.HeatCoilMaxFluidFlow * PartLoadFrac;
                    PlantUtilities::SetComponentFlowRate(state, MaxHotWaterFlow, mshp.HeatCoilControlNode, mshp.HeatCoilFluidOutletNode, mshp.HeatCoilPlantLoc);
                    WaterCoils::SimulateWaterCoilComponents(state, mshp.HeatCoilName, FirstHVACIteration, mshp.HeatCoilNum, QCoilActual, fanOp);
                } break;

                case HVAC::CoilType::HeatingSteam: {
                    Real64 mdot = mshp.HeatCoilMaxFluidFlow * PartLoadFrac;
                    Real64 CoilHeatingLoad = HeatingLoad * PartLoadFrac;
                    PlantUtilities::SetComponentFlowRate(state, mdot, mshp.HeatCoilControlNode, mshp.HeatCoilFluidOutletNode, mshp.HeatCoilPlantLoc);
                    // simulate steam supplemental heating coil
                    SteamCoils::SimulateSteamCoilComponents(
                        state, mshp.HeatCoilName, FirstHVACIteration, mshp.HeatCoilNum, CoilHeatingLoad, QCoilActual, fanOp);
                } break;
                default:
                  break;
                }
                
            } else { // end of IF (HeatingLoad > SmallLoad) THEN
              
                switch (mshp.heatCoilType) {
                case HVAC::CoilType::HeatingGasOrOtherFuel:
                case HVAC::CoilType::HeatingElectric: {
                    HeatingCoils::SimulateHeatingCoilComponents(
                       state, mshp.HeatCoilName, FirstHVACIteration, HeatingLoad, mshp.HeatCoilNum, QCoilActual, true, fanOp);
                } break;
                  
                case HVAC::CoilType::HeatingWater: {
                    Real64 mdot = 0.0;
                    PlantUtilities::SetComponentFlowRate(state, mdot, mshp.HeatCoilControlNode, mshp.HeatCoilFluidOutletNode, mshp.HeatCoilPlantLoc);
                    WaterCoils::SimulateWaterCoilComponents(state, mshp.HeatCoilName, FirstHVACIteration, mshp.HeatCoilNum, QCoilActual, fanOp);
                } break;
                  
                case HVAC::CoilType::HeatingSteam: {
                    Real64 mdot = 0.0;
                    PlantUtilities::SetComponentFlowRate(state, mdot, mshp.HeatCoilControlNode, mshp.HeatCoilFluidOutletNode, mshp.HeatCoilPlantLoc);
                    // simulate the steam supplemental heating coil
                    SteamCoils::SimulateSteamCoilComponents(
                        state, mshp.HeatCoilName, FirstHVACIteration, mshp.HeatCoilNum, HeatingLoad, QCoilActual, fanOp);
                } break;
                default:
                  break;
                }
            }
            
        } else { // !present(PartLoad)
            if (HeatingLoad > HVAC::SmallLoad) {

                switch (mshp.suppHeatCoilType) {
                case HVAC::CoilType::HeatingGasOrOtherFuel:
                case HVAC::CoilType::HeatingElectric: {
                    HeatingCoils::SimulateHeatingCoilComponents(
                        state, mshp.SuppHeatCoilName, FirstHVACIteration, HeatingLoad, mshp.SuppHeatCoilNum, QCoilActual, true, fanOp);
                } break;
                  
                case HVAC::CoilType::HeatingWater: {
                    Real64 MaxHotWaterFlow = mshp.SuppHeatCoilMaxFluidFlow;
                    PlantUtilities::SetComponentFlowRate(
                        state, MaxHotWaterFlow, mshp.SuppHeatCoilControlNode, mshp.SuppHeatCoilFluidOutletNode, mshp.SuppHeatCoilPlantLoc);
                    WaterCoils::SimulateWaterCoilComponents(
                        state, mshp.SuppHeatCoilName, FirstHVACIteration, mshp.SuppHeatCoilNum, QCoilActual, fanOp);

                    if (QCoilActual > (HeatingLoad + HVAC::SmallLoad)) {
                        // control water flow to obtain output matching HeatingLoad
                        int SolFlag = 0;
                        Real64 MinWaterFlow = 0.0;
                        auto f = [&state, MSHeatPumpNum, FirstHVACIteration, HeatingLoad](Real64 const HWFlow) {
                            // Calculates residual function (QCoilActual - SupHeatCoilLoad) / SupHeatCoilLoad
                            // coil actual output depends on the hot water flow rate which is varied to minimize the residual.
                            Real64 targetHeatingCoilLoad = HeatingLoad;
                            Real64 calcHeatingCoilLoad = targetHeatingCoilLoad;
                            Real64 mdot = HWFlow;
                            auto &hp = state.dataHVACMultiSpdHP->MSHeatPump(MSHeatPumpNum);
                            PlantUtilities::SetComponentFlowRate(
                                state, mdot, hp.SuppHeatCoilControlNode, hp.SuppHeatCoilFluidOutletNode, hp.SuppHeatCoilPlantLoc);
                            // simulate the hot water supplemental heating coil
                            WaterCoils::SimulateWaterCoilComponents(
                                 state, hp.SuppHeatCoilName, FirstHVACIteration, hp.SuppHeatCoilNum, calcHeatingCoilLoad, hp.fanOp);
                            if (targetHeatingCoilLoad != 0.0) {
                                return (calcHeatingCoilLoad - targetHeatingCoilLoad) / targetHeatingCoilLoad;
                            } // Autodesk:Return Condition added to assure return value is set
                            return 0.0;
                        };

                        Real64 HotWaterMdot;
                        General::SolveRoot(state, ErrTolerance, SolveMaxIter, SolFlag, HotWaterMdot, f, MinWaterFlow, MaxHotWaterFlow);
                        if (SolFlag == -1) {
                            if (mshp.HotWaterCoilMaxIterIndex == 0) {
                                ShowWarningMessage(state,
                                                   format("CalcNonDXHeatingCoils: Hot water coil control failed for {}=\"{}\"",
                                                          CurrentModuleObject,
                                                          mshp.Name));
                                ShowContinueErrorTimeStamp(state, "");
                                ShowContinueError(state,
                                                  format("  Iteration limit [{}] exceeded in calculating hot water mass flow rate", SolveMaxIter));
                                 }
                            ShowRecurringWarningErrorAtEnd(
                                                           state,
                                                           format("CalcNonDXHeatingCoils: Hot water coil control failed (iteration limit [{}]) for {}=\"{}",
                                                                  SolveMaxIter,
                                                                  CurrentModuleObject,
                                                                  mshp.Name),
                                                           mshp.HotWaterCoilMaxIterIndex);
                        } else if (SolFlag == -2) {
                            if (mshp.HotWaterCoilMaxIterIndex2 == 0) {
                              ShowWarningMessage(state,
                                                 format("CalcNonDXHeatingCoils: Hot water coil control failed (maximum flow limits) for {}=\"{}\"",
                                                        CurrentModuleObject,
                                                        mshp.Name));
                              ShowContinueErrorTimeStamp(state, "");
                              ShowContinueError(state, "...Bad hot water maximum flow rate limits");
                              ShowContinueError(state, format("...Given minimum water flow rate={:.3R} kg/s", MinWaterFlow));
                              ShowContinueError(state, format("...Given maximum water flow rate={:.3R} kg/s", MaxHotWaterFlow));
                            }
                            ShowRecurringWarningErrorAtEnd(state,
                                                           "CalcNonDXHeatingCoils: Hot water coil control failed (flow limits) for " +
                                                           std::string{CurrentModuleObject} + "=\"" + mshp.Name + "\"",
                                                           mshp.HotWaterCoilMaxIterIndex2,
                                                           MaxHotWaterFlow,
                                                           MinWaterFlow,
                                                           _,
                                                           "[kg/s]",
                                                           "[kg/s]");
                             }
                        // simulate hot water supplemental heating coil
                        WaterCoils::SimulateWaterCoilComponents(
                            state, mshp.SuppHeatCoilName, FirstHVACIteration, mshp.SuppHeatCoilNum, QCoilActual, fanOp);
                    }
                } break;
                
                case HVAC::CoilType::HeatingSteam: {
                    Real64 mdot = mshp.SuppHeatCoilMaxFluidFlow;
                    Real64 SteamCoilHeatingLoad = HeatingLoad;
                    PlantUtilities::SetComponentFlowRate(
                        state, mdot, mshp.SuppHeatCoilControlNode, mshp.SuppHeatCoilFluidOutletNode, mshp.SuppHeatCoilPlantLoc);
                    // simulate steam supplemental heating coil
                    SteamCoils::SimulateSteamCoilComponents(
                        state, mshp.SuppHeatCoilName, FirstHVACIteration, mshp.SuppHeatCoilNum, SteamCoilHeatingLoad, QCoilActual, fanOp);
                } break;
                default:
                  break;
                }
                
            } else { // end of IF (HeatingLoad > SmallLoad) THEN
              
                switch (mshp.suppHeatCoilType) {
                case HVAC::CoilType::HeatingGasOrOtherFuel:
                case HVAC::CoilType::HeatingElectric: {
                    HeatingCoils::SimulateHeatingCoilComponents(
                        state, mshp.SuppHeatCoilName, FirstHVACIteration, HeatingLoad, mshp.SuppHeatCoilNum, QCoilActual, true, fanOp);
                } break;
                  
                case HVAC::CoilType::HeatingWater: {
                    Real64 mdot = 0.0;
                    PlantUtilities::SetComponentFlowRate(
                        state, mdot, mshp.SuppHeatCoilControlNode, mshp.SuppHeatCoilFluidOutletNode, mshp.SuppHeatCoilPlantLoc);
                    WaterCoils::SimulateWaterCoilComponents(state, mshp.SuppHeatCoilName, FirstHVACIteration, mshp.HeatCoilNum, QCoilActual, fanOp);
                } break;
                  
                case HVAC::CoilType::HeatingSteam: {
                    Real64 mdot = 0.0;
                    PlantUtilities::SetComponentFlowRate(
                        state, mdot, mshp.SuppHeatCoilControlNode, mshp.SuppHeatCoilFluidOutletNode, mshp.SuppHeatCoilPlantLoc);
                    // simulate the steam supplemental heating coil
                    SteamCoils::SimulateSteamCoilComponents(
                        state, mshp.SuppHeatCoilName, FirstHVACIteration, mshp.SuppHeatCoilNum, HeatingLoad, QCoilActual, fanOp);
                } break;
                default:
                  break;
                } // switch (mshp.heatCoilType)
            }
        }
        HeatCoilLoadmet = QCoilActual;
    }
  
} // namespace HVACMultiSpeedHeatPump

} // namespace EnergyPlus

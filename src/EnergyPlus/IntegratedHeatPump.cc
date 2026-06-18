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
#include <format>

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/BranchNodeConnections.hh>
#include <EnergyPlus/Data/EnergyPlusData.hh>
#include <EnergyPlus/DataEnvironment.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataSizing.hh>
#include <EnergyPlus/GeneralRoutines.hh>
#include <EnergyPlus/GlobalNames.hh>
#include <EnergyPlus/InputProcessing/InputProcessor.hh>
#include <EnergyPlus/IntegratedHeatPump.hh>
#include <EnergyPlus/NodeInputManager.hh>
#include <EnergyPlus/OutputProcessor.hh>
#include <EnergyPlus/Plant/DataPlant.hh>
#include <EnergyPlus/UtilityRoutines.hh>
#include <EnergyPlus/VariableSpeedCoils.hh>
#include <EnergyPlus/WaterThermalTanks.hh>

namespace EnergyPlus::IntegratedHeatPump {

Real64 constexpr WaterDensity(986.0); // standard water density at 60 C

void SimIHP(EnergyPlusData &state,
            std::string_view CompName,             // Coil Name
            int &CompIndex,                        // Index for Component name
            HVAC::FanOp const fanOp,               // Continuous fan OR cycling compressor
            HVAC::CompressorOp const compressorOp, // compressor on/off. 0 = off; 1= on
            Real64 const PartLoadFrac,             // part load fraction
            int const SpeedNum,                    // compressor speed number
            Real64 const SpeedRatio,               // compressor speed ratio
            Real64 const SensLoad,                 // Sensible demand load [W]
            Real64 const LatentLoad,               // Latent demand load [W]
            bool const IsCallbyWH,                 // whether the call from the water heating loop or air loop, true = from water heating loop
            [[maybe_unused]] bool const FirstHVACIteration,   // TRUE if First iteration of simulation
            ObjexxFCL::Optional<Real64 const> OnOffAirFlowRat // ratio of comp on to comp off air flow rate
)
{

    //       AUTHOR         Bo Shen, ORNL
    //       DATE WRITTEN   March 2016
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine manages variable-speed integrated Air source heat pump simulation.

    // Using/Aliasing
    using VariableSpeedCoils::InitVarSpeedCoil;
    using VariableSpeedCoils::SimVariableSpeedCoils;
    using VariableSpeedCoils::UpdateVarSpeedCoil;

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int DXCoilNum(0); // The IHP No that you are currently dealing with

    // Obtains and Allocates ASIHP related parameters from input file
    if (state.dataIntegratedHP->GetCoilsInputFlag) { // First time subroutine has been entered
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    }

    if (CompIndex == 0) {
        DXCoilNum = Util::FindItemInList(CompName, state.dataIntegratedHP->IntegratedHeatPumps);
        if (DXCoilNum == 0) {
            ShowFatalError(state, std::format("Integrated Heat Pump not found={}", CompName));
        }
        CompIndex = DXCoilNum;
    } else {
        DXCoilNum = CompIndex;
        if (DXCoilNum > static_cast<int>(state.dataIntegratedHP->IntegratedHeatPumps.size()) || DXCoilNum < 1) {
            ShowFatalError(state,
                           std::format("SimIHP: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name={}",
                                       DXCoilNum,
                                       state.dataIntegratedHP->IntegratedHeatPumps.size(),
                                       CompName));
        }
        if (!CompName.empty() && CompName != state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum).Name) {
            ShowFatalError(state,
                           std::format("SimIHP: Invalid CompIndex passed={}, Integrated HP name={}, stored Integrated HP Name for that index={}",
                                       DXCoilNum,
                                       CompName,
                                       state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum).Name));
        }
    };

    auto &ihp = state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum);

    if (!ihp.IHPCoilsSized) {
        SizeIHP(state, DXCoilNum);
    }

    InitializeIHP(state, DXCoilNum);

    Real64 airMassFlowRate = state.dataLoopNodes->Node(ihp.AirCoolInletNodeNum).MassFlowRate;
    ihp.AirLoopFlowRate = airMassFlowRate;

    switch (ihp.CurMode) {
    case IHPOperationMode::SpaceClg:
        if (!IsCallbyWH) // process when called from air loop
        {
            SimVariableSpeedCoils(state, std::string(), ihp.SCDWHCoolCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SCDWHWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SHDWHHeatCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SHDWHWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SCWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.DWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);

            SimVariableSpeedCoils(state,
                                  std::string(),
                                  ihp.SCCoilIndex,
                                  fanOp,
                                  compressorOp,
                                  PartLoadFrac,
                                  SpeedNum,
                                  SpeedRatio,
                                  SensLoad,
                                  LatentLoad,
                                  OnOffAirFlowRat);

            SimVariableSpeedCoils(state, std::string(), ihp.SHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);

            ihp.AirFlowSavInAirLoop = airMassFlowRate;
        }

        ihp.TankSourceWaterMassFlowRate = 0.0;
        break;
    case IHPOperationMode::SpaceHtg:
        if (!IsCallbyWH) // process when called from air loop
        {
            SimVariableSpeedCoils(state, std::string(), ihp.SCDWHCoolCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SCDWHWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SHDWHHeatCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SHDWHWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SCWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SCCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.DWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);

            SimVariableSpeedCoils(state,
                                  std::string(),
                                  ihp.SHCoilIndex,
                                  fanOp,
                                  compressorOp,
                                  PartLoadFrac,
                                  SpeedNum,
                                  SpeedRatio,
                                  SensLoad,
                                  LatentLoad,
                                  OnOffAirFlowRat);

            ihp.AirFlowSavInAirLoop = airMassFlowRate;
        }
        ihp.TankSourceWaterMassFlowRate = 0.0;
        break;
    case IHPOperationMode::DedicatedWaterHtg:
        if (IsCallbyWH) // process when called from water loop
        {
            SimVariableSpeedCoils(state, std::string(), ihp.SCDWHCoolCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SCDWHWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SHDWHHeatCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SHDWHWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SCWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SCCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);

            SimVariableSpeedCoils(state,
                                  std::string(),
                                  ihp.DWHCoilIndex,
                                  fanOp,
                                  compressorOp,
                                  PartLoadFrac,
                                  SpeedNum,
                                  SpeedRatio,
                                  SensLoad,
                                  LatentLoad,
                                  OnOffAirFlowRat);
            // IntegratedHeatPumps(DXCoilNum).TotalHeatingEnergyRate =
            // VarSpeedCoil(IntegratedHeatPumps(DXCoilNum).DWHCoilIndex).TotalHeatingEnergyRate;
        }

        ihp.TankSourceWaterMassFlowRate = state.dataLoopNodes->Node(ihp.WaterInletNodeNum).MassFlowRate;
        break;
    case IHPOperationMode::SCWHMatchSC:
        if (!IsCallbyWH) // process when called from air loop
        {
            SimVariableSpeedCoils(state, std::string(), ihp.SCDWHCoolCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SCDWHWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SHDWHHeatCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SHDWHWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SCCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.DWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);

            SimVariableSpeedCoils(state,
                                  std::string(),
                                  ihp.SCWHCoilIndex,
                                  fanOp,
                                  compressorOp,
                                  PartLoadFrac,
                                  SpeedNum,
                                  SpeedRatio,
                                  SensLoad,
                                  LatentLoad,
                                  OnOffAirFlowRat);

            SimVariableSpeedCoils(state, std::string(), ihp.SHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);

            ihp.AirFlowSavInAirLoop = airMassFlowRate;
        }

        ihp.TankSourceWaterMassFlowRate = state.dataLoopNodes->Node(ihp.WaterInletNodeNum).MassFlowRate;

        break;
    case IHPOperationMode::SCWHMatchWH:
        if (IsCallbyWH) // process when called from water loop
        {
            SimVariableSpeedCoils(state, std::string(), ihp.SCDWHCoolCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SCDWHWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SHDWHHeatCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SHDWHWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SCCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.DWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);

            SimVariableSpeedCoils(state,
                                  std::string(),
                                  ihp.SCWHCoilIndex,
                                  fanOp,
                                  compressorOp,
                                  PartLoadFrac,
                                  SpeedNum,
                                  SpeedRatio,
                                  SensLoad,
                                  LatentLoad,
                                  OnOffAirFlowRat);

            SimVariableSpeedCoils(state, std::string(), ihp.SHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);

            ihp.AirFlowSavInWaterLoop = airMassFlowRate;
        }

        ihp.TankSourceWaterMassFlowRate = state.dataLoopNodes->Node(ihp.WaterInletNodeNum).MassFlowRate;
        break;
    case IHPOperationMode::SpaceClgDedicatedWaterHtg:
        if (!IsCallbyWH) // process when called from air loop
        {
            SimVariableSpeedCoils(state, std::string(), ihp.SHDWHHeatCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SHDWHWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SCWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SCCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.DWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);

            SimVariableSpeedCoils(state,
                                  std::string(),
                                  ihp.SCDWHWHCoilIndex,
                                  fanOp,
                                  compressorOp,
                                  PartLoadFrac,
                                  SpeedNum,
                                  SpeedRatio,
                                  SensLoad,
                                  LatentLoad,
                                  OnOffAirFlowRat);
            SimVariableSpeedCoils(state,
                                  std::string(),
                                  ihp.SCDWHCoolCoilIndex,
                                  fanOp,
                                  compressorOp,
                                  PartLoadFrac,
                                  SpeedNum,
                                  SpeedRatio,
                                  SensLoad,
                                  LatentLoad,
                                  OnOffAirFlowRat);

            SimVariableSpeedCoils(state, std::string(), ihp.SHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);

            ihp.AirFlowSavInAirLoop = airMassFlowRate;
        }

        ihp.TankSourceWaterMassFlowRate = state.dataLoopNodes->Node(ihp.WaterInletNodeNum).MassFlowRate;
        break;
    case IHPOperationMode::SHDWHElecHeatOff:
    case IHPOperationMode::SHDWHElecHeatOn:
        if (!IsCallbyWH) // process when called from air loop
        {
            SimVariableSpeedCoils(state, std::string(), ihp.SCDWHCoolCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SCDWHWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SCWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SCCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.SHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            SimVariableSpeedCoils(state, std::string(), ihp.DWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);

            SimVariableSpeedCoils(state,
                                  std::string(),
                                  ihp.SHDWHWHCoilIndex,
                                  fanOp,
                                  compressorOp,
                                  PartLoadFrac,
                                  SpeedNum,
                                  SpeedRatio,
                                  SensLoad,
                                  LatentLoad,
                                  OnOffAirFlowRat);
            SimVariableSpeedCoils(state,
                                  std::string(),
                                  ihp.SHDWHHeatCoilIndex,
                                  fanOp,
                                  compressorOp,
                                  PartLoadFrac,
                                  SpeedNum,
                                  SpeedRatio,
                                  SensLoad,
                                  LatentLoad,
                                  OnOffAirFlowRat);

            ihp.AirFlowSavInAirLoop = airMassFlowRate;
        }

        ihp.TankSourceWaterMassFlowRate = state.dataLoopNodes->Node(ihp.WaterInletNodeNum).MassFlowRate;
        break;
    case IHPOperationMode::Idle:
    default: // clear up
        SimVariableSpeedCoils(state, std::string(), ihp.SCDWHCoolCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
        SimVariableSpeedCoils(state, std::string(), ihp.SCDWHWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
        SimVariableSpeedCoils(state, std::string(), ihp.SHDWHHeatCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
        SimVariableSpeedCoils(state, std::string(), ihp.SHDWHWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
        SimVariableSpeedCoils(state, std::string(), ihp.SCWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
        SimVariableSpeedCoils(state, std::string(), ihp.SCCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
        SimVariableSpeedCoils(state, std::string(), ihp.SHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
        SimVariableSpeedCoils(state, std::string(), ihp.DWHCoilIndex, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
        ihp.TankSourceWaterMassFlowRate = 0.0;
        ihp.AirFlowSavInAirLoop = 0.0;
        ihp.AirFlowSavInWaterLoop = 0.0;
        break;
    }

    UpdateIHP(state, DXCoilNum);
}

void GetIHPInput(EnergyPlusData &state)
{

    // SUBROUTINE INFORMATION:
    //       AUTHOR         Bo Shen
    //       DATE WRITTEN   December, 2015
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // Obtains input data for Integrated HPs and stores it in IHP data structures

    // METHODOLOGY EMPLOYED:
    // Uses "Get" routines to read in data.

    // Using/Aliasing
    using GlobalNames::VerifyUniqueCoilName;
    using Node::OverrideNodeConnectionType;
    using Node::RegisterNodeConnection;
    using Node::SetUpCompSets;
    using Node::TestCompSet;
    using VariableSpeedCoils::GetCoilIndexVariableSpeed;

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("GetIHPInput: "); // include trailing blank space

    // SUBROUTINE LOCAL VARIABLE DECLARATIONS:
    int NumAlphas;                   // Number of variables in String format
    int NumNums;                     // Number of variables in Numeric format
    int NumParams;                   // Total number of input fields
    int MaxNums(0);                  // Maximum number of numeric input fields
    int MaxAlphas(0);                // Maximum number of alpha input fields
    std::string CurrentModuleObject; // for ease in getting objects
    Array1D_string AlphArray;        // Alpha input items for object
    Array1D_string cAlphaFields;     // Alpha field names
    Array1D_string cNumericFields;   // Numeric field names
    Array1D<Real64> NumArray;        // Numeric input items for object
    Array1D_bool lAlphaBlanks;       // Logical array, alpha field input BLANK = .TRUE.
    Array1D_bool lNumericBlanks;     // Logical array, numeric field input BLANK = .TRUE.

    bool ErrorsFound(false); // If errors detected in input
    bool IsNotOK;            // Flag to verify name
    bool errFlag;
    int IOStat;

    int NumASIHPs = state.dataInputProcessing->inputProcessor->getNumObjectsFound(state, "COILSYSTEM:INTEGRATEDHEATPUMP:AIRSOURCE");

    if (NumASIHPs <= 0) {
        return;
    }

    // Allocate Arrays
    state.dataIntegratedHP->IntegratedHeatPumps.allocate(NumASIHPs);

    // air-source integrated heat pump
    state.dataInputProcessing->inputProcessor->getObjectDefMaxArgs(state, "COILSYSTEM:INTEGRATEDHEATPUMP:AIRSOURCE", NumParams, NumAlphas, NumNums);
    MaxNums = max(MaxNums, NumNums);
    MaxAlphas = max(MaxAlphas, NumAlphas);

    AlphArray.allocate(MaxAlphas);
    cAlphaFields.allocate(MaxAlphas);
    lAlphaBlanks.dimension(MaxAlphas, true);
    cNumericFields.allocate(MaxNums);
    lNumericBlanks.dimension(MaxNums, true);
    NumArray.dimension(MaxNums, 0.0);

    // Get the data for air-source IHPs
    CurrentModuleObject = "COILSYSTEM:INTEGRATEDHEATPUMP:AIRSOURCE"; // for reporting

    for (int CoilCounter = 1; CoilCounter <= NumASIHPs; ++CoilCounter) {

        state.dataInputProcessing->inputProcessor->getObjectItem(state,
                                                                 CurrentModuleObject,
                                                                 CoilCounter,
                                                                 AlphArray,
                                                                 NumAlphas,
                                                                 NumArray,
                                                                 NumNums,
                                                                 IOStat,
                                                                 lNumericBlanks,
                                                                 lAlphaBlanks,
                                                                 cAlphaFields,
                                                                 cNumericFields);

        // ErrorsFound will be set to True if problem was found, left untouched otherwise
        VerifyUniqueCoilName(state, CurrentModuleObject, AlphArray(1), ErrorsFound, CurrentModuleObject + " Name");

        auto &ihp = state.dataIntegratedHP->IntegratedHeatPumps(CoilCounter);

        ihp.Name = AlphArray(1);
        ihp.IHPtype = "AIRSOURCE_IHP";

        // AlphArray( 2 ) is the water sensor node

        ihp.SCCoilType = "COIL:COOLING:DX:VARIABLESPEED";
        ihp.SCCoilName = AlphArray(3);
        ihp.SCCoilTypeNum = Node::ConnectionObjectType::CoilCoolingDXVariableSpeed;

        ValidateComponent(state, ihp.SCCoilType, ihp.SCCoilName, IsNotOK, CurrentModuleObject);
        if (IsNotOK) {
            ShowContinueError(state, std::format("...specified in {}=\"{}\".", CurrentModuleObject, AlphArray(1)));
            ErrorsFound = true;
        } else {
            errFlag = false;
            ihp.SCCoilIndex = GetCoilIndexVariableSpeed(state, ihp.SCCoilType, ihp.SCCoilName, errFlag);
            if (errFlag) {
                ShowContinueError(state, std::format("...specified in {}=\"{}\".", CurrentModuleObject, AlphArray(1)));
                ErrorsFound = true;
            }
        }

        ihp.SHCoilType = "COIL:HEATING:DX:VARIABLESPEED";
        ihp.SHCoilName = AlphArray(4);
        ihp.SHCoilTypeNum = Node::ConnectionObjectType::CoilHeatingDXVariableSpeed;

        ValidateComponent(state, ihp.SHCoilType, ihp.SHCoilName, IsNotOK, CurrentModuleObject);
        if (IsNotOK) {
            ShowContinueError(state, std::format("...specified in {}=\"{}\".", CurrentModuleObject, AlphArray(1)));
            ErrorsFound = true;
        } else {
            errFlag = false;
            ihp.SHCoilIndex = GetCoilIndexVariableSpeed(state, ihp.SHCoilType, ihp.SHCoilName, errFlag);
            if (errFlag) {
                ShowContinueError(state, std::format("...specified in {}=\"{}\".", CurrentModuleObject, AlphArray(1)));
                ErrorsFound = true;
            }
        }

        ihp.DWHCoilType = "COIL:WATERHEATING:AIRTOWATERHEATPUMP:VARIABLESPEED";
        ihp.DWHCoilName = AlphArray(5);
        ihp.DWHCoilTypeNum = Node::ConnectionObjectType::CoilWaterHeatingAirToWaterHeatPumpVariableSpeed;

        ValidateComponent(state, ihp.DWHCoilType, ihp.DWHCoilName, IsNotOK, CurrentModuleObject);
        if (IsNotOK) {
            ShowContinueError(state, std::format("...specified in {}=\"{}\".", CurrentModuleObject, AlphArray(1)));
            ErrorsFound = true;
        } else {
            errFlag = false;
            ihp.DWHCoilIndex = GetCoilIndexVariableSpeed(state, ihp.DWHCoilType, ihp.DWHCoilName, errFlag);
            if (errFlag) {
                ShowContinueError(state, std::format("...specified in {}=\"{}\".", CurrentModuleObject, AlphArray(1)));
                ErrorsFound = true;
            }
        }

        ihp.SCWHCoilType = "COIL:WATERHEATING:AIRTOWATERHEATPUMP:VARIABLESPEED";
        ihp.SCWHCoilName = AlphArray(6);
        ihp.SCWHCoilTypeNum = Node::ConnectionObjectType::CoilWaterHeatingAirToWaterHeatPumpVariableSpeed;

        ValidateComponent(state, ihp.SCWHCoilType, ihp.SCWHCoilName, IsNotOK, CurrentModuleObject);
        if (IsNotOK) {
            ShowContinueError(state, std::format("...specified in {}=\"{}\".", CurrentModuleObject, AlphArray(1)));
            ErrorsFound = true;
        } else {
            errFlag = false;
            ihp.SCWHCoilIndex = GetCoilIndexVariableSpeed(state, ihp.SCWHCoilType, ihp.SCWHCoilName, errFlag);
            if (errFlag) {
                ShowContinueError(state, std::format("...specified in {}=\"{}\".", CurrentModuleObject, AlphArray(1)));
                ErrorsFound = true;
            }
        }

        ihp.SCDWHCoolCoilType = "COIL:COOLING:DX:VARIABLESPEED";
        ihp.SCDWHCoolCoilName = AlphArray(7);
        ihp.SCDWHCoolCoilTypeNum = Node::ConnectionObjectType::CoilCoolingDXVariableSpeed;

        ValidateComponent(state, ihp.SCDWHCoolCoilType, ihp.SCDWHCoolCoilName, IsNotOK, CurrentModuleObject);
        if (IsNotOK) {
            ShowContinueError(state, std::format("...specified in {}=\"{}\".", CurrentModuleObject, AlphArray(1)));
            ErrorsFound = true;
        } else {
            errFlag = false;
            ihp.SCDWHCoolCoilIndex = GetCoilIndexVariableSpeed(state, ihp.SCDWHCoolCoilType, ihp.SCDWHCoolCoilName, errFlag);
            if (errFlag) {
                ShowContinueError(state, std::format("...specified in {}=\"{}\".", CurrentModuleObject, AlphArray(1)));
                ErrorsFound = true;
            }
        }

        ihp.SCDWHWHCoilType = "COIL:WATERHEATING:AIRTOWATERHEATPUMP:VARIABLESPEED";
        ihp.SCDWHWHCoilName = AlphArray(8);
        ihp.SCDWHWHCoilTypeNum = Node::ConnectionObjectType::CoilWaterHeatingAirToWaterHeatPumpVariableSpeed;

        ValidateComponent(state, ihp.SCDWHWHCoilType, ihp.SCDWHWHCoilName, IsNotOK, CurrentModuleObject);
        if (IsNotOK) {
            ShowContinueError(state, std::format("...specified in {}=\"{}\".", CurrentModuleObject, AlphArray(1)));
            ErrorsFound = true;
        } else {
            errFlag = false;
            ihp.SCDWHWHCoilIndex = GetCoilIndexVariableSpeed(state, ihp.SCDWHWHCoilType, ihp.SCDWHWHCoilName, errFlag);
            if (errFlag) {
                ShowContinueError(state, std::format("...specified in {}=\"{}\".", CurrentModuleObject, AlphArray(1)));
                ErrorsFound = true;
            } else {
                state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHWHCoilIndex).bIsDesuperheater = true;
            }
        }

        ihp.SHDWHHeatCoilType = "COIL:HEATING:DX:VARIABLESPEED";
        ihp.SHDWHHeatCoilName = AlphArray(9);
        ihp.SHDWHHeatCoilTypeNum = Node::ConnectionObjectType::CoilHeatingDXVariableSpeed;

        ValidateComponent(state, ihp.SHDWHHeatCoilType, ihp.SHDWHHeatCoilName, IsNotOK, CurrentModuleObject);
        if (IsNotOK) {
            ShowContinueError(state, std::format("...specified in {}=\"{}\".", CurrentModuleObject, AlphArray(1)));
            ErrorsFound = true;
        } else {
            errFlag = false;
            ihp.SHDWHHeatCoilIndex = GetCoilIndexVariableSpeed(state, ihp.SHDWHHeatCoilType, ihp.SHDWHHeatCoilName, errFlag);
            if (errFlag) {
                ShowContinueError(state, std::format("...specified in {}=\"{}\".", CurrentModuleObject, AlphArray(1)));
                ErrorsFound = true;
            }
        }

        ihp.SHDWHWHCoilType = "COIL:WATERHEATING:AIRTOWATERHEATPUMP:VARIABLESPEED";
        ihp.SHDWHWHCoilName = AlphArray(10);
        ihp.SHDWHWHCoilTypeNum = Node::ConnectionObjectType::CoilWaterHeatingAirToWaterHeatPumpVariableSpeed;

        ValidateComponent(state, ihp.SHDWHWHCoilType, ihp.SHDWHWHCoilName, IsNotOK, CurrentModuleObject);
        if (IsNotOK) {
            ShowContinueError(state, std::format("...specified in {}=\"{}\".", CurrentModuleObject, AlphArray(1)));
            ErrorsFound = true;
        } else {
            errFlag = false;
            ihp.SHDWHWHCoilIndex = GetCoilIndexVariableSpeed(state, ihp.SHDWHWHCoilType, ihp.SHDWHWHCoilName, errFlag);
            if (errFlag) {
                ShowContinueError(state, std::format("...specified in {}=\"{}\".", CurrentModuleObject, AlphArray(1)));
                ErrorsFound = true;
            } else {
                state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHWHCoilIndex).bIsDesuperheater = true;
            }
        }

        ihp.TindoorOverCoolAllow = NumArray(1);
        ihp.TambientOverCoolAllow = NumArray(2);
        ihp.TindoorWHHighPriority = NumArray(3);
        ihp.TambientWHHighPriority = NumArray(4);
        ihp.ModeMatchSCWH = int(NumArray(5));
        ihp.MinSpedSCWH = int(NumArray(6));
        ihp.WaterVolSCDWH = NumArray(7);
        ihp.MinSpedSCDWH = int(NumArray(8));
        ihp.TimeLimitSHDWH = NumArray(9);
        ihp.MinSpedSHDWH = int(NumArray(10));

        // Due to the overlapping coil objects, compsets and node registrations are handled as follows:
        //  1. The ASIHP coil object is registered as four different coils, Name+" Cooling Coil", Name+" Heating Coil",
        //     Name+" Outdoor Coil", and Name+" Water Coil"
        //  2. For each of these four coils, TestCompSet is called once to register it as a child object
        //  3. For each of these four coils, RegisterNodeConnection is called twice to register the inlet and outlet nodes
        //     RegisterNodeConnection is used instead of GetOnlySingleNode because the node names are not inputs here
        //  4. The parent objects that reference the ASIHP coil must use the appropriate name suffixes when calling SetUpCompSets
        //  5. The ASIHP calls SetUpCompSets to register the various child coils.  This is important so that the system energy
        //     use is collected in SystemReports::CalcSystemEnergyUse
        //  6. The child coil inlet/outlet node connections are reset to connection type "Internal" to avoid duplicate node problems
        //     using OverrideNodeConnectionType

        // cooling coil air node connections
        int ChildCoilIndex = ihp.SCCoilIndex;
        int InNode = state.dataVariableSpeedCoils->VarSpeedCoil(ChildCoilIndex).AirInletNodeNum;
        int OutNode = state.dataVariableSpeedCoils->VarSpeedCoil(ChildCoilIndex).AirOutletNodeNum;
        std::string InNodeName = state.dataLoopNodes->NodeID(InNode);
        std::string OutNodeName = state.dataLoopNodes->NodeID(OutNode);

        ihp.AirCoolInletNodeNum = InNode;
        ihp.AirHeatInletNodeNum = OutNode;

        TestCompSet(state, CurrentModuleObject, ihp.Name + " Cooling Coil", InNodeName, OutNodeName, "Cooling Air Nodes");
        RegisterNodeConnection(state,
                               InNode,
                               state.dataLoopNodes->NodeID(InNode),
                               Node::ConnectionObjectType::CoilSystemIntegratedHeatPumpAirSource,
                               ihp.Name + " Cooling Coil",
                               Node::ConnectionType::Inlet,
                               Node::CompFluidStream::Primary,
                               Node::ObjectIsNotParent,
                               ErrorsFound);
        RegisterNodeConnection(state,
                               OutNode,
                               state.dataLoopNodes->NodeID(OutNode),
                               Node::ConnectionObjectType::CoilSystemIntegratedHeatPumpAirSource,
                               ihp.Name + " Cooling Coil",
                               Node::ConnectionType::Outlet,
                               Node::CompFluidStream::Primary,
                               Node::ObjectIsNotParent,
                               ErrorsFound);

        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Cooling Coil", ihp.SCCoilType, ihp.SCCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.SCCoilTypeNum,
                                   ihp.SCCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Primary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.SCCoilTypeNum,
                                   ihp.SCCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Primary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);

        if ((state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilIndex).AirInletNodeNum != InNode) ||
            (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilIndex).AirOutletNodeNum != OutNode)) {
            ShowContinueError(state, std::format("Mistaken air node connection: {}{}-wrong coil node names.", CurrentModuleObject, ihp.SCWHCoilName));
            ErrorsFound = true;
        }
        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Cooling Coil", ihp.SCWHCoilType, ihp.SCWHCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.SCWHCoilTypeNum,
                                   ihp.SCWHCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Primary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.SCWHCoilTypeNum,
                                   ihp.SCWHCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Primary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);

        if ((state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHCoolCoilIndex).AirInletNodeNum != InNode) ||
            (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHCoolCoilIndex).AirOutletNodeNum != OutNode)) {
            ShowContinueError(state,
                              std::format("Mistaken air node connection: {}{}-wrong coil node names.", CurrentModuleObject, ihp.SCDWHCoolCoilName));
            ErrorsFound = true;
        }
        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Cooling Coil", ihp.SCDWHCoolCoilType, ihp.SCDWHCoolCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.SCDWHCoolCoilTypeNum,
                                   ihp.SCDWHCoolCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Primary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.SCDWHCoolCoilTypeNum,
                                   ihp.SCDWHCoolCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Primary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);

        // heating coil air node connections
        ChildCoilIndex = ihp.SHCoilIndex;

        InNode = ihp.AirHeatInletNodeNum;
        OutNode = state.dataVariableSpeedCoils->VarSpeedCoil(ChildCoilIndex).AirOutletNodeNum;
        ihp.AirOutletNodeNum = OutNode;
        InNodeName = state.dataLoopNodes->NodeID(InNode);
        OutNodeName = state.dataLoopNodes->NodeID(OutNode);
        if (state.dataVariableSpeedCoils->VarSpeedCoil(ChildCoilIndex).AirInletNodeNum != InNode) {
            ShowContinueError(
                state, std::format("Mistaken air node connection: {}- cooling coil outlet mismatches heating coil inlet.", CurrentModuleObject));
            ErrorsFound = true;
        }
        TestCompSet(state, CurrentModuleObject, ihp.Name + " Heating Coil", InNodeName, OutNodeName, "Heating Air Nodes");
        RegisterNodeConnection(state,
                               InNode,
                               state.dataLoopNodes->NodeID(InNode),
                               Node::ConnectionObjectType::CoilSystemIntegratedHeatPumpAirSource,
                               ihp.Name + " Heating Coil",
                               Node::ConnectionType::Inlet,
                               Node::CompFluidStream::Primary,
                               Node::ObjectIsNotParent,
                               ErrorsFound);
        RegisterNodeConnection(state,
                               OutNode,
                               state.dataLoopNodes->NodeID(OutNode),
                               Node::ConnectionObjectType::CoilSystemIntegratedHeatPumpAirSource,
                               ihp.Name + " Heating Coil",
                               Node::ConnectionType::Outlet,
                               Node::CompFluidStream::Primary,
                               Node::ObjectIsNotParent,
                               ErrorsFound);

        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Heating Coil", ihp.SHCoilType, ihp.SHCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.SHCoilTypeNum,
                                   ihp.SHCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Primary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.SHCoilTypeNum,
                                   ihp.SHCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Primary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);

        if ((state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHHeatCoilIndex).AirInletNodeNum != InNode) ||
            (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHHeatCoilIndex).AirOutletNodeNum != OutNode)) {
            ShowContinueError(state,
                              std::format("Mistaken air node connection: {}:{}-wrong coil node names.", CurrentModuleObject, ihp.SHDWHHeatCoilName));
            ErrorsFound = true;
        }
        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Heating Coil", ihp.SHDWHHeatCoilType, ihp.SHDWHHeatCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.SHDWHHeatCoilTypeNum,
                                   ihp.SHDWHHeatCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Primary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.SHDWHHeatCoilTypeNum,
                                   ihp.SHDWHHeatCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Primary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);

        // water node connections
        ChildCoilIndex = ihp.SCWHCoilIndex;

        InNode = state.dataVariableSpeedCoils->VarSpeedCoil(ChildCoilIndex).WaterInletNodeNum;
        OutNode = state.dataVariableSpeedCoils->VarSpeedCoil(ChildCoilIndex).WaterOutletNodeNum;
        InNodeName = state.dataLoopNodes->NodeID(InNode);
        OutNodeName = state.dataLoopNodes->NodeID(OutNode);
        ihp.WaterInletNodeNum = InNode;
        ihp.WaterOutletNodeNum = OutNode;
        if ((state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHWHCoilIndex).WaterInletNodeNum != InNode) ||
            (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHWHCoilIndex).WaterOutletNodeNum != OutNode)) {
            ShowContinueError(state,
                              std::format("Mistaken air node connection: {}:{}-wrong coil node names.", CurrentModuleObject, ihp.SCDWHWHCoilName));
            ErrorsFound = true;
        }

        TestCompSet(state, CurrentModuleObject, ihp.Name + " Water Coil", InNodeName, OutNodeName, "Water Nodes");
        RegisterNodeConnection(state,
                               InNode,
                               state.dataLoopNodes->NodeID(InNode),
                               Node::ConnectionObjectType::CoilSystemIntegratedHeatPumpAirSource,
                               ihp.Name + " Water Coil",
                               Node::ConnectionType::Inlet,
                               Node::CompFluidStream::Primary,
                               Node::ObjectIsNotParent,
                               ErrorsFound);
        RegisterNodeConnection(state,
                               OutNode,
                               state.dataLoopNodes->NodeID(InNode),
                               Node::ConnectionObjectType::CoilSystemIntegratedHeatPumpAirSource,
                               ihp.Name + " Water Coil",
                               Node::ConnectionType::Outlet,
                               Node::CompFluidStream::Primary,
                               Node::ObjectIsNotParent,
                               ErrorsFound);

        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Water Coil", ihp.SCWHCoilType, ihp.SCWHCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.SCWHCoilTypeNum,
                                   ihp.SCWHCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Secondary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.SCWHCoilTypeNum,
                                   ihp.SCWHCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Secondary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);

        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Water Coil", ihp.SCDWHWHCoilType, ihp.SCDWHWHCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.SCDWHWHCoilTypeNum,
                                   ihp.SCDWHWHCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Secondary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.SCDWHWHCoilTypeNum,
                                   ihp.SCDWHWHCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Secondary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);

        if ((state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHWHCoilIndex).WaterInletNodeNum != InNode) ||
            (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHWHCoilIndex).WaterOutletNodeNum != OutNode)) {
            ShowContinueError(state,
                              std::format("Mistaken air node connection: {}:{}-wrong coil node names.", CurrentModuleObject, ihp.SHDWHWHCoilName));
            ErrorsFound = true;
        }
        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Water Coil", ihp.SHDWHWHCoilType, ihp.SHDWHWHCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.SHDWHWHCoilTypeNum,
                                   ihp.SHDWHWHCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Secondary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.SHDWHWHCoilTypeNum,
                                   ihp.SHDWHWHCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Secondary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);

        if ((state.dataVariableSpeedCoils->VarSpeedCoil(ihp.DWHCoilIndex).WaterInletNodeNum != InNode) ||
            (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.DWHCoilIndex).WaterOutletNodeNum != OutNode)) {
            ShowContinueError(state, std::format("Mistaken air node connection: {}:{}-wrong coil node names.", CurrentModuleObject, ihp.DWHCoilName));
            ErrorsFound = true;
        }
        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Water Coil", ihp.DWHCoilType, ihp.DWHCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.DWHCoilTypeNum,
                                   ihp.DWHCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Secondary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.DWHCoilTypeNum,
                                   ihp.DWHCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Secondary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);

        ihp.WaterTankoutNod = GetOnlySingleNode(state,
                                                AlphArray(2),
                                                ErrorsFound,
                                                Node::ConnectionObjectType::CoilSystemIntegratedHeatPumpAirSource,
                                                AlphArray(1),
                                                Node::FluidType::Water,
                                                Node::ConnectionType::Sensor,
                                                Node::CompFluidStream::Secondary,
                                                Node::ObjectIsNotParent);

        // outdoor air node connections for water heating coils
        // DWH, SCDWH, SHDWH coils have the same outdoor air nodes
        ChildCoilIndex = ihp.DWHCoilIndex;
        InNode = state.dataVariableSpeedCoils->VarSpeedCoil(ChildCoilIndex).AirInletNodeNum;
        OutNode = state.dataVariableSpeedCoils->VarSpeedCoil(ChildCoilIndex).AirOutletNodeNum;
        InNodeName = state.dataLoopNodes->NodeID(InNode);
        OutNodeName = state.dataLoopNodes->NodeID(OutNode);
        ihp.ODAirInletNodeNum = InNode;
        ihp.ODAirOutletNodeNum = OutNode;
        if ((state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHWHCoilIndex).AirInletNodeNum != InNode) ||
            (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHWHCoilIndex).AirOutletNodeNum != OutNode)) {
            ShowContinueError(state,
                              std::format("Mistaken air node connection: {}:{}-wrong coil node names.", CurrentModuleObject, ihp.SCDWHWHCoilName));
            ErrorsFound = true;
        }

        TestCompSet(state, CurrentModuleObject, ihp.Name + " Outdoor Coil", InNodeName, OutNodeName, "Outdoor Air Nodes");
        RegisterNodeConnection(state,
                               InNode,
                               state.dataLoopNodes->NodeID(InNode),
                               Node::ConnectionObjectType::CoilSystemIntegratedHeatPumpAirSource,
                               ihp.Name + " Outdoor Coil",
                               Node::ConnectionType::Inlet,
                               Node::CompFluidStream::Primary,
                               Node::ObjectIsNotParent,
                               ErrorsFound);
        RegisterNodeConnection(state,
                               OutNode,
                               state.dataLoopNodes->NodeID(InNode),
                               Node::ConnectionObjectType::CoilSystemIntegratedHeatPumpAirSource,
                               ihp.Name + " Outdoor Coil",
                               Node::ConnectionType::Outlet,
                               Node::CompFluidStream::Primary,
                               Node::ObjectIsNotParent,
                               ErrorsFound);

        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Outdoor Coil", ihp.DWHCoilType, ihp.DWHCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.DWHCoilTypeNum,
                                   ihp.DWHCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Primary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.DWHCoilTypeNum,
                                   ihp.DWHCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Primary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);

        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Outdoor Coil", ihp.SCDWHWHCoilType, ihp.SCDWHWHCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.SCDWHWHCoilTypeNum,
                                   ihp.SCDWHWHCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Primary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.SCDWHWHCoilTypeNum,
                                   ihp.SCDWHWHCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Primary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);

        // why was this here
        //        state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHWHCoilIndex).AirInletNodeNum = InNode;
        //        state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHWHCoilIndex).AirOutletNodeNum = OutNode;

        if ((state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHWHCoilIndex).AirInletNodeNum != InNode) ||
            (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHWHCoilIndex).AirOutletNodeNum != OutNode)) {
            ShowContinueError(state,
                              std::format("Mistaken air node connection: {}:{}-wrong coil node names.", CurrentModuleObject, ihp.SHDWHWHCoilName));
            ErrorsFound = true;
        }
        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Outdoor Coil", ihp.SHDWHWHCoilType, ihp.SHDWHWHCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.SHDWHWHCoilTypeNum,
                                   ihp.SHDWHWHCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Primary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.SHDWHWHCoilTypeNum,
                                   ihp.SHDWHWHCoilName,
                                   Node::ConnectionType::Internal,
                                   Node::CompFluidStream::Primary,
                                   Node::ObjectIsNotParent,
                                   ErrorsFound);

        ihp.IHPCoilsSized = false;
        ihp.CoolVolFlowScale = 1.0; // scale coil flow rates to match the parent fan object
        ihp.HeatVolFlowScale = 1.0; // scale coil flow rates to match the parent fan object
        ihp.CurMode = IHPOperationMode::Idle;
        ihp.MaxHeatAirMassFlow = 1e10;
        ihp.MaxHeatAirVolFlow = 1e10;
        ihp.MaxCoolAirMassFlow = 1e10;
        ihp.MaxCoolAirVolFlow = 1e10;

        VariableSpeedCoils::SetCoilSystemHeatingDXFlag(state, ihp.SHCoilType, ihp.SHCoilName);
        VariableSpeedCoils::SetCoilSystemHeatingDXFlag(state, ihp.SHDWHHeatCoilType, ihp.SHDWHHeatCoilName);
    }

    if (ErrorsFound) {
        ShowFatalError(
            state, std::format("{} Errors found in getting {} input. Preceding condition(s) causes termination.", RoutineName, CurrentModuleObject));
    }

    for (int CoilCounter = 1; CoilCounter <= NumASIHPs; ++CoilCounter) {

        auto &ihp = state.dataIntegratedHP->IntegratedHeatPumps(CoilCounter);

        // set up output variables, not reported in the individual coil models
        SetupOutputVariable(state,
                            "Integrated Heat Pump Air Loop Mass Flow Rate",
                            Constant::Units::kg_s,
                            ihp.AirLoopFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            ihp.Name);
        SetupOutputVariable(state,
                            "Integrated Heat Pump Condenser Water Mass Flow Rate",
                            Constant::Units::kg_s,
                            ihp.TankSourceWaterMassFlowRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            ihp.Name);
        SetupOutputVariable(state,
                            "Integrated Heat Pump Air Total Cooling Rate",
                            Constant::Units::W,
                            ihp.TotalCoolingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            ihp.Name);
        SetupOutputVariable(state,
                            "Integrated Heat Pump Air Heating Rate",
                            Constant::Units::W,
                            ihp.TotalSpaceHeatingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            ihp.Name);
        SetupOutputVariable(state,
                            "Integrated Heat Pump Water Heating Rate",
                            Constant::Units::W,
                            ihp.TotalWaterHeatingRate,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            ihp.Name);
        SetupOutputVariable(state,
                            "Integrated Heat Pump Electricity Rate",
                            Constant::Units::W,
                            ihp.TotalPower,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            ihp.Name);
        SetupOutputVariable(state,
                            "Integrated Heat Pump Air Latent Cooling Rate",
                            Constant::Units::W,
                            ihp.TotalLatentLoad,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            ihp.Name);
        SetupOutputVariable(state,
                            "Integrated Heat Pump Source Heat Transfer Rate",
                            Constant::Units::W,
                            ihp.Qsource,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            ihp.Name);
        SetupOutputVariable(state,
                            "Integrated Heat Pump COP",
                            Constant::Units::None,
                            ihp.TotalCOP,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Average,
                            ihp.Name);
        SetupOutputVariable(state,
                            "Integrated Heat Pump Electricity Energy",
                            Constant::Units::J,
                            ihp.Energy,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            ihp.Name);
        SetupOutputVariable(state,
                            "Integrated Heat Pump Air Total Cooling Energy",
                            Constant::Units::J,
                            ihp.EnergyLoadTotalCooling,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            ihp.Name);
        SetupOutputVariable(state,
                            "Integrated Heat Pump Air Heating Energy",
                            Constant::Units::J,
                            ihp.EnergyLoadTotalHeating,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            ihp.Name);
        SetupOutputVariable(state,
                            "Integrated Heat Pump Water Heating Energy",
                            Constant::Units::J,
                            ihp.EnergyLoadTotalWaterHeating,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            ihp.Name);
        SetupOutputVariable(state,
                            "Integrated Heat Pump Air Latent Cooling Energy",
                            Constant::Units::J,
                            ihp.EnergyLatent,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            ihp.Name);
        SetupOutputVariable(state,
                            "Integrated Heat Pump Source Heat Transfer Energy",
                            Constant::Units::J,
                            ihp.EnergySource,
                            OutputProcessor::TimeStepType::System,
                            OutputProcessor::StoreType::Sum,
                            ihp.Name);
    }
}

void SizeIHP(EnergyPlusData &state, int const DXCoilNum)
{
    using DataSizing::AutoSize;
    using VariableSpeedCoils::SetVarSpeedCoilData;
    using VariableSpeedCoils::SimVariableSpeedCoils;
    using VariableSpeedCoils::SizeVarSpeedCoil;

    bool ErrorsFound = false;
    Real64 RatedCapacity(0.0); // rated building cooling load

    // Obtains and Allocates AS-IHP related parameters from input file
    if (state.dataIntegratedHP->GetCoilsInputFlag) { // First time subroutine has been entered
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    };

    if (DXCoilNum > static_cast<int>(state.dataIntegratedHP->IntegratedHeatPumps.size()) || DXCoilNum < 1) {
        ShowFatalError(state,
                       std::format("SizeIHP: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name=AS-IHP",
                                   DXCoilNum,
                                   state.dataIntegratedHP->IntegratedHeatPumps.size()));
    }

    if (state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum).IHPCoilsSized) {
        return;
    }

    auto &ihp = state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum);

    // associate SC coil with SH coil
    bool errFlag = false;
    SetVarSpeedCoilData(state, ihp.SCCoilIndex, errFlag, _, ihp.SHCoilIndex);
    if (errFlag) {
        ShowSevereError(state, std::format(R"(SizeIHP: Could not match cooling coil"{}" with heating coil="{}")", ihp.SCCoilName, ihp.SHCoilName));
        ErrorsFound = true;
    };

    errFlag = false;
    SizeVarSpeedCoil(state, ihp.SCCoilIndex, errFlag); // size cooling coil
    if (errFlag) {
        ShowSevereError(state, std::format("SizeIHP: failed to size SC coil\"{}\"", ihp.SCCoilName));
        ErrorsFound = true;
    } else {
        RatedCapacity = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCCoilIndex).RatedCapCoolTotal;
    };

    errFlag = false;
    SizeVarSpeedCoil(state, ihp.SHCoilIndex, errFlag); // size heating coil
    if (errFlag) {
        ShowSevereError(state, std::format("SizeIHP: failed to size SH coil\"{}\"", ihp.SHCoilName));
        ErrorsFound = true;
    };

    // pass SC coil capacity to SCDWH cool coil
    if (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHCoolCoilIndex).RatedCapCoolTotal == AutoSize) {
        state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHCoolCoilIndex).RatedCapCoolTotal = RatedCapacity;
    };

    // associate SCDWH air coil to SHDWH air coil
    errFlag = false;
    SetVarSpeedCoilData(state, ihp.SCDWHCoolCoilIndex, errFlag, _, ihp.SHDWHHeatCoilIndex);
    // size SCDWH air coil
    SizeVarSpeedCoil(state, ihp.SCDWHCoolCoilIndex, errFlag);
    if (errFlag) {
        ShowSevereError(state, std::format("SizeIHP: failed to size SCDWH cooling coil\"{}\"", ihp.SCDWHCoolCoilName));
        ErrorsFound = true;
    };

    // size SHDWH air coil
    errFlag = false;
    SizeVarSpeedCoil(state, ihp.SHDWHHeatCoilIndex, errFlag);
    if (errFlag) {
        ShowSevereError(state, std::format("SizeIHP: failed to size SHDWH heating coil\"{}\"", ihp.SHDWHHeatCoilName));
        ErrorsFound = true;
    };

    // size the water coils below
    // size SCWH water coil
    if (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilIndex).RatedCapWH == AutoSize) {
        state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilIndex).RatedCapWH =
            RatedCapacity / (1.0 - 1.0 / state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilIndex).RatedCOPHeat);
    }

    errFlag = false;
    SizeVarSpeedCoil(state, ihp.SCWHCoilIndex, errFlag);
    if (errFlag) {
        ShowSevereError(state, std::format("SizeIHP: failed to size SCWH coil\"{}\"", ihp.SCWHCoilName));
        ErrorsFound = true;
    };

    // size DWH water coil
    if (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.DWHCoilIndex).RatedCapWH == AutoSize) {
        state.dataVariableSpeedCoils->VarSpeedCoil(ihp.DWHCoilIndex).RatedCapWH = RatedCapacity;
    }

    errFlag = false;
    SizeVarSpeedCoil(state, ihp.DWHCoilIndex, errFlag);
    if (errFlag) {
        ShowSevereError(state, std::format("SizeIHP: failed to size DWH coil\"{}\"", ihp.DWHCoilName));
        ErrorsFound = true;
    };

    // size SCDWH water coil
    if (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHWHCoilIndex).RatedCapWH == AutoSize) {
        state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHWHCoilIndex).RatedCapWH = RatedCapacity * 0.13;
    }

    errFlag = false;
    SizeVarSpeedCoil(state, ihp.SCDWHWHCoilIndex, errFlag);
    if (errFlag) {
        ShowSevereError(state, std::format("SizeIHP: failed to size SCDWH water heating coil\"{}\"", ihp.SCDWHWHCoilName));
        ErrorsFound = true;
    };

    // size SHDWH water coil
    if (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHWHCoilIndex).RatedCapWH == AutoSize) {
        state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHWHCoilIndex).RatedCapWH = RatedCapacity * 0.1;
    }

    errFlag = false;
    SizeVarSpeedCoil(state, ihp.SHDWHWHCoilIndex, errFlag);
    if (errFlag) {
        ShowSevereError(state, std::format("SizeIHP: failed to size SHDWH water heating coil\"{}\"", ihp.SHDWHWHCoilName));
        ErrorsFound = true;
    };

    if (ErrorsFound) {
        ShowFatalError(state, "Program terminates due to preceding condition(s).");
    }

    ihp.IHPCoilsSized = true;
}

void InitializeIHP(EnergyPlusData &state, int const DXCoilNum)
{
    // Obtains and Allocates AS-IHP related parameters from input file
    if (state.dataIntegratedHP->GetCoilsInputFlag) { // First time subroutine has been entered
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    }

    if (DXCoilNum > static_cast<int>(state.dataIntegratedHP->IntegratedHeatPumps.size()) || DXCoilNum < 1) {
        ShowFatalError(state,
                       std::format("InitializeIHP: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name=AS-IHP",
                                   DXCoilNum,
                                   state.dataIntegratedHP->IntegratedHeatPumps.size()));
    }

    auto &ihp = state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum);

    ihp.AirLoopFlowRate = 0.0;             // air loop mass flow rate [kg/s]
    ihp.TankSourceWaterMassFlowRate = 0.0; // water loop mass flow rate [kg/s]
    ihp.TotalCoolingRate = 0.0;            // total cooling rate [w]
    ihp.TotalWaterHeatingRate = 0.0;       // total water heating rate [w]
    ihp.TotalSpaceHeatingRate = 0.0;       // total space heating rate [w]
    ihp.TotalPower = 0.0;                  // total power consumption  [w]
    ihp.TotalLatentLoad = 0.0;             // total latent cooling rate [w]
    ihp.Qsource = 0.0;                     // source energy rate, [w]
    ihp.Energy = 0.0;                      // total electric energy consumption [J]
    ihp.EnergyLoadTotalCooling = 0.0;      // total cooling energy [J]
    ihp.EnergyLoadTotalHeating = 0.0;      // total heating energy [J]
    ihp.EnergyLoadTotalWaterHeating = 0.0; // total heating energy [J]
    ihp.EnergyLatent = 0.0;                // total latent energy [J]
    ihp.EnergySource = 0.0;                // total source energy
    ihp.TotalCOP = 0.0;
}

void UpdateIHP(EnergyPlusData &state, int const DXCoilNum)
{
    Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    // Obtains and Allocates AS-IHP related parameters from input file
    if (state.dataIntegratedHP->GetCoilsInputFlag) { // First time subroutine has been entered
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    }

    if (DXCoilNum > static_cast<int>(state.dataIntegratedHP->IntegratedHeatPumps.size()) || DXCoilNum < 1) {
        ShowFatalError(state,
                       std::format("UpdateIHP: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name=AS-IHP",
                                   DXCoilNum,
                                   state.dataIntegratedHP->IntegratedHeatPumps.size()));
    }

    auto &ihp = state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum);

    switch (ihp.CurMode) {
    case IHPOperationMode::SpaceClg:
        ihp.TotalCoolingRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCCoilIndex).QLoadTotal; // total cooling rate [w]
        ihp.TotalWaterHeatingRate = 0.0;                                                               // total water heating rate [w]
        ihp.TotalSpaceHeatingRate = 0.0;                                                               // total space heating rate [w]
        ihp.TotalPower = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCCoilIndex).Power;            // total power consumption  [w]
        ihp.TotalLatentLoad = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCCoilIndex).QLatent;     // total latent cooling rate [w]
        ihp.Qsource = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCCoilIndex).QSource;             // source energy rate, [w]
        break;
    case IHPOperationMode::SpaceHtg:
        ihp.TotalCoolingRate = 0.0;                                                                         // total cooling rate [w]
        ihp.TotalWaterHeatingRate = 0.0;                                                                    // total water heating rate [w]
        ihp.TotalSpaceHeatingRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHCoilIndex).QLoadTotal; // total space heating rate [w]
        ihp.TotalPower = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHCoilIndex).Power;                 // total power consumption  [w]
        ihp.TotalLatentLoad = 0.0;                                                                          // total latent cooling rate [w]
        ihp.Qsource = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHCoilIndex).QSource;                  // source energy rate, [w]
        break;
    case IHPOperationMode::DedicatedWaterHtg:
        ihp.TotalCoolingRate = 0.0;                                                                       // total cooling rate [w]
        ihp.TotalWaterHeatingRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.DWHCoilIndex).QSource; // total water heating rate [w]
        ihp.TotalSpaceHeatingRate = 0.0;                                                                  // total space heating rate [w]
        ihp.TotalPower = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.DWHCoilIndex).Power;              // total power consumption  [w]
        ihp.TotalLatentLoad = 0.0;                                                                        // total latent cooling rate [w]
        ihp.Qsource = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.DWHCoilIndex).QLoadTotal;            // source energy rate, [w]
        break;
    case IHPOperationMode::SCWHMatchSC:
    case IHPOperationMode::SCWHMatchWH:
        ihp.TotalCoolingRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilIndex).QLoadTotal;   // total cooling rate [w]
        ihp.TotalWaterHeatingRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilIndex).QSource; // total water heating rate [w]
        ihp.TotalSpaceHeatingRate = 0.0;                                                                   // total space heating rate [w]
        ihp.TotalPower = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilIndex).Power;              // total power consumption  [w]
        ihp.TotalLatentLoad = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilIndex).QLatent;       // total latent cooling rate [w]
        ihp.Qsource = 0.0;                                                                                 // source energy rate, [w]
        break;
    case IHPOperationMode::SpaceClgDedicatedWaterHtg:
        ihp.TotalCoolingRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHCoolCoilIndex).QLoadTotal; // total cooling rate [w]
        ihp.TotalSpaceHeatingRate = 0.0;                                                                      // total space heating rate [w]
        ihp.TotalPower = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHCoolCoilIndex).Power;            // total power consumption  [w]
        ihp.TotalLatentLoad = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHCoolCoilIndex).QLatent;     // total latent cooling rate [w]
        ihp.Qsource = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHCoolCoilIndex).QSource;             // source energy rate, [w]

        ihp.TotalWaterHeatingRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHWHCoilIndex).QSource; // total water heating rate [w]

        break;
    case IHPOperationMode::SHDWHElecHeatOff:
    case IHPOperationMode::SHDWHElecHeatOn:
        ihp.TotalCoolingRate = 0.0;                                                                                // total cooling rate [w]
        ihp.TotalSpaceHeatingRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHHeatCoilIndex).QLoadTotal; // total space heating rate [w]
        ihp.TotalPower = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHHeatCoilIndex).Power;                 // total power consumption  [w]
        ihp.TotalLatentLoad = 0.0;                                                                                 // total latent cooling rate [w]
        ihp.Qsource = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHHeatCoilIndex).QSource;                  // source energy rate, [w]

        ihp.TotalWaterHeatingRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHWHCoilIndex).QSource; // total water heating rate [w]

        break;
    case IHPOperationMode::Idle:
    default:
        break;
    }

    ihp.Energy = ihp.TotalPower * TimeStepSysSec;                                 // total electric energy consumption [J]
    ihp.EnergyLoadTotalCooling = ihp.TotalCoolingRate * TimeStepSysSec;           // total cooling energy [J]
    ihp.EnergyLoadTotalHeating = ihp.TotalSpaceHeatingRate * TimeStepSysSec;      // total heating energy [J]
    ihp.EnergyLoadTotalWaterHeating = ihp.TotalWaterHeatingRate * TimeStepSysSec; // total heating energy [J]
    ihp.EnergyLatent = ihp.TotalLatentLoad * TimeStepSysSec;                      // total latent energy [J]
    ihp.EnergySource = ihp.Qsource * TimeStepSysSec;                              // total source energy

    if (ihp.TotalPower > 0.0) {
        Real64 TotalDelivery = ihp.TotalCoolingRate + ihp.TotalSpaceHeatingRate + ihp.TotalWaterHeatingRate;
        ihp.TotalCOP = TotalDelivery / ihp.TotalPower;
    }
}

void DecideWorkMode(EnergyPlusData &state,
                    int const DXCoilNum,
                    Real64 const SensLoad,  // Sensible demand load [W]
                    Real64 const LatentLoad // Latent demand load [W]
                    )                       // shall be called from a air loop parent
{
    //       AUTHOR         Bo Shen, ORNL
    //       DATE WRITTEN   March 2016
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS SUBROUTINE:
    // This subroutine determine the IHP working mode in the next time step,
    // it should be called by an air loop parent object, when FirstHVACIteration == true

    // Using/Aliasing
    using HVAC::SmallLoad;
    using WaterThermalTanks::GetWaterThermalTankInput;

    Real64 TimeStepSysSec = state.dataHVACGlobal->TimeStepSysSec;

    Real64 MyLoad(0.0);
    Real64 WHHeatTimeSav(0.0); // time accumulation for water heating
    Real64 WHHeatVolSave(0.0); // volume accumulation for water heating

    // Obtains and Allocates AS-IHP related parameters from input file
    if (state.dataIntegratedHP->GetCoilsInputFlag) { // First time subroutine has been entered
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    }

    if (DXCoilNum > static_cast<int>(state.dataIntegratedHP->IntegratedHeatPumps.size()) || DXCoilNum < 1) {
        ShowFatalError(state,
                       std::format("DecideWorkMode: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name=AS-IHP",
                                   DXCoilNum,
                                   state.dataIntegratedHP->IntegratedHeatPumps.size()));
    }

    auto &ihp = state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum);

    if (ihp.IHPCoilsSized == false) {
        SizeIHP(state, DXCoilNum);
    }

    // decide working mode at the first moment
    // check if there is a water heating call
    ihp.IsWHCallAvail = false;
    ihp.CheckWHCall = true; // set checking flag
    if (ihp.WHtankID == 0)  // not initialized yet
    {
        ihp.IsWHCallAvail = false;
    } else {
        state.dataLoopNodes->Node(ihp.WaterInletNodeNum).MassFlowRate = GetWaterVolFlowRateIHP(state, DXCoilNum, 1.0, 1.0) * WaterDensity;
        state.dataLoopNodes->Node(ihp.WaterOutletNodeNum).Temp = state.dataLoopNodes->Node(ihp.WaterInletNodeNum).Temp;

        DataPlant::PlantEquipmentType tankType = ihp.WHtankType;

        switch (tankType) {
        case DataPlant::PlantEquipmentType::WtrHeaterMixed:
        case DataPlant::PlantEquipmentType::WtrHeaterStratified:
        case DataPlant::PlantEquipmentType::ChilledWaterTankMixed:
        case DataPlant::PlantEquipmentType::ChilledWaterTankStratified:

        {
            int tankIDX = WaterThermalTanks::getTankIDX(state, ihp.WHtankName, ihp.WHtankID);
            auto &tank = state.dataWaterThermalTanks->WaterThermalTank(tankIDX);
            tank.callerLoopNum = ihp.LoopNum;
            PlantLocation A(0, DataPlant::LoopSideLocation::Invalid, 0, 0);
            tank.simulate(state, A, true, MyLoad, true);
            tank.callerLoopNum = 0;

            break;
        }
        case DataPlant::PlantEquipmentType::HeatPumpWtrHeaterPumped:
        case DataPlant::PlantEquipmentType::HeatPumpWtrHeaterWrapped:

        {
            int hpIDX = WaterThermalTanks::getHPTankIDX(state, ihp.WHtankName, ihp.WHtankID);
            auto &HPWH = state.dataWaterThermalTanks->HPWaterHeater(hpIDX);
            int tankIDX = HPWH.WaterHeaterTankNum;
            auto &tank = state.dataWaterThermalTanks->WaterThermalTank(tankIDX);
            tank.callerLoopNum = ihp.LoopNum;
            ihp.WHtankType = tankType;
            PlantLocation A(0, DataPlant::LoopSideLocation::Invalid, 0, 0);
            HPWH.simulate(state, A, true, MyLoad, true);
            tank.callerLoopNum = 0;
            break;
        }
        default:
            break;
        }
    }
    ihp.CheckWHCall = false; // clear checking flag

    // keep the water heating time and volume history
    WHHeatTimeSav = ihp.SHDWHRunTime;
    if (IHPOperationMode::SpaceClgDedicatedWaterHtg == ihp.CurMode) {
        WHHeatVolSave = ihp.WaterFlowAccumVol +
                        state.dataLoopNodes->Node(ihp.WaterTankoutNod).MassFlowRate / 983.0 * TimeStepSysSec; // 983 - water density at 60 C
    } else {
        WHHeatVolSave = 0.0;
    }

    // clear the accumulation amount for other modes
    ihp.SHDWHRunTime = 0.0;
    ihp.WaterFlowAccumVol = 0.0;

    if (!ihp.IsWHCallAvail) // no water heating call
    {
        if ((SensLoad < (-1.0 * SmallLoad)) || (LatentLoad < (-1.0 * SmallLoad))) // space cooling mode
        {
            ihp.CurMode = IHPOperationMode::SpaceClg;
        } else if (SensLoad > SmallLoad) {
            if ((ihp.ControlledZoneTemp > ihp.TindoorOverCoolAllow) &&
                (state.dataEnvrn->OutDryBulbTemp > ihp.TambientOverCoolAllow)) { // used for cooling season, avoid heating after SCWH mode
                ihp.CurMode = IHPOperationMode::Idle;
            } else {
                ihp.CurMode = IHPOperationMode::SpaceHtg;
            }
        } else {
            ihp.CurMode = IHPOperationMode::Idle;
        }
    }
    // below has water heating calls
    else if ((SensLoad < (-1.0 * SmallLoad)) || (LatentLoad < (-1.0 * SmallLoad))) // simultaneous SC and WH calls
    {
        if (WHHeatVolSave < ihp.WaterVolSCDWH) // small water heating amount
        {
            ihp.CurMode = IHPOperationMode::SpaceClgDedicatedWaterHtg;
            ihp.WaterFlowAccumVol = WHHeatVolSave;
        } else {
            if (1 == ihp.ModeMatchSCWH) { // water heating priority
                ihp.CurMode = IHPOperationMode::SCWHMatchWH;
            } else { // space cooling priority
                ihp.CurMode = IHPOperationMode::SCWHMatchSC;
            }
        };

    } else if ((ihp.ControlledZoneTemp > ihp.TindoorOverCoolAllow) &&
               (state.dataEnvrn->OutDryBulbTemp > ihp.TambientOverCoolAllow)) // over-cooling allowed, water heating priority
    {
        ihp.CurMode = IHPOperationMode::SCWHMatchWH;
    } else if ((ihp.ControlledZoneTemp > ihp.TindoorWHHighPriority) &&
               (state.dataEnvrn->OutDryBulbTemp > ihp.TambientWHHighPriority)) // ignore space heating request
    {
        ihp.CurMode = IHPOperationMode::DedicatedWaterHtg;
    } else if (SensLoad > SmallLoad) {
        ihp.SHDWHRunTime = WHHeatTimeSav + TimeStepSysSec;

        if (WHHeatTimeSav > ihp.TimeLimitSHDWH) {
            ihp.CurMode = IHPOperationMode::SHDWHElecHeatOn;
        } else {
            ihp.CurMode = IHPOperationMode::SHDWHElecHeatOff;
        };
    } else {
        ihp.CurMode = IHPOperationMode::DedicatedWaterHtg;
    }

    // clear up, important
    ClearCoils(state, DXCoilNum);
}

void ClearCoils(EnergyPlusData &state, int const DXCoilNum)
{
    using VariableSpeedCoils::SimVariableSpeedCoils;

    // Obtains and Allocates WatertoAirHP related parameters from input file
    if (state.dataIntegratedHP->GetCoilsInputFlag) { // First time subroutine has been entered
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    }

    if (DXCoilNum > static_cast<int>(state.dataIntegratedHP->IntegratedHeatPumps.size()) || DXCoilNum < 1) {
        ShowFatalError(state,
                       std::format("ClearCoils: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name=AS-IHP",
                                   DXCoilNum,
                                   state.dataIntegratedHP->IntegratedHeatPumps.size()));
    }

    auto &ihp = state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum);

    // clear up
    SimVariableSpeedCoils(state, std::string(), ihp.SCDWHCoolCoilIndex, HVAC::FanOp::Cycling, HVAC::CompressorOp::On, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    SimVariableSpeedCoils(state, std::string(), ihp.SCDWHWHCoilIndex, HVAC::FanOp::Cycling, HVAC::CompressorOp::On, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    SimVariableSpeedCoils(state, std::string(), ihp.SHDWHHeatCoilIndex, HVAC::FanOp::Cycling, HVAC::CompressorOp::On, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    SimVariableSpeedCoils(state, std::string(), ihp.SHDWHWHCoilIndex, HVAC::FanOp::Cycling, HVAC::CompressorOp::On, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    SimVariableSpeedCoils(state, std::string(), ihp.SCWHCoilIndex, HVAC::FanOp::Cycling, HVAC::CompressorOp::On, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    SimVariableSpeedCoils(state, std::string(), ihp.SCCoilIndex, HVAC::FanOp::Cycling, HVAC::CompressorOp::On, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    SimVariableSpeedCoils(state, std::string(), ihp.SHCoilIndex, HVAC::FanOp::Cycling, HVAC::CompressorOp::On, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    SimVariableSpeedCoils(state, std::string(), ihp.DWHCoilIndex, HVAC::FanOp::Cycling, HVAC::CompressorOp::On, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
}

IHPOperationMode GetCurWorkMode(EnergyPlusData &state, int const DXCoilNum)
{
    // Obtains and Allocates WatertoAirHP related parameters from input file
    if (state.dataIntegratedHP->GetCoilsInputFlag) { // First time subroutine has been entered
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    }

    if (DXCoilNum > static_cast<int>(state.dataIntegratedHP->IntegratedHeatPumps.size()) || DXCoilNum < 1) {
        ShowFatalError(state,
                       std::format("GetCurWorkMode: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name=AS-IHP",
                                   DXCoilNum,
                                   state.dataIntegratedHP->IntegratedHeatPumps.size()));
    }

    if (state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum).IHPCoilsSized == false) {
        SizeIHP(state, DXCoilNum);
    }

    return (state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum).CurMode);
}

bool IHPInModel(EnergyPlusData &state)
{
    if (state.dataIntegratedHP->GetCoilsInputFlag) {
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    }
    return !state.dataIntegratedHP->IntegratedHeatPumps.empty();
}

int GetCoilIndexIHP(EnergyPlusData &state,
                    std::string const &CoilType, // must match coil types in this module
                    std::string const &CoilName, // must match coil names for the coil type
                    bool &ErrorsFound            // set to true if problem
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Bo Shen
    //       DATE WRITTEN   March 2016
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function looks up the coil index for the given coil and returns it.  If
    // incorrect coil type or name is given, ErrorsFound is returned as true and index is returned
    // as zero.

    // Return value
    int IndexNum; // returned index of matched coil

    // Obtains and Allocates WatertoAirHP related parameters from input file
    if (state.dataIntegratedHP->GetCoilsInputFlag) { // First time subroutine has been entered
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    }

    IndexNum = Util::FindItemInList(CoilName, state.dataIntegratedHP->IntegratedHeatPumps);

    if (IndexNum == 0) {
        ShowSevereError(state, std::format(R"(GetCoilIndexIHP: Could not find CoilType="{}" with Name="{}")", CoilType, CoilName));
        ErrorsFound = true;
    }

    return IndexNum;
}

int GetCoilInletNodeIHP(EnergyPlusData &state,
                        std::string const &CoilType, // must match coil types in this module
                        std::string const &CoilName, // must match coil names for the coil type
                        bool &ErrorsFound            // set to true if problem
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Bo Shen
    //       DATE WRITTEN   March 2016
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function looks up the given coil and returns the inlet node.  If
    // incorrect coil type or name is given, ErrorsFound is returned as true and value is returned
    // as zero.

    // Return value
    int NodeNumber(0); // returned outlet node of matched coil

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    int WhichCoil;

    // Obtains and Allocates WatertoAirHP related parameters from input file
    if (state.dataIntegratedHP->GetCoilsInputFlag) { // First time subroutine has been entered
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    }

    WhichCoil = Util::FindItemInList(CoilName, state.dataIntegratedHP->IntegratedHeatPumps);
    if (WhichCoil != 0) {
        NodeNumber = state.dataIntegratedHP->IntegratedHeatPumps(WhichCoil).AirCoolInletNodeNum;
    }

    if (WhichCoil == 0) {
        ShowSevereError(state, std::format(R"(GetCoilInletNodeIHP: Could not find CoilType="{}" with Name="{}")", CoilType, CoilName));
        ErrorsFound = true;
        NodeNumber = 0;
    }

    return NodeNumber;
}

int GetDWHCoilInletNodeIHP(EnergyPlusData &state,
                           std::string const &CoilType, // must match coil types in this module
                           std::string const &CoilName, // must match coil names for the coil type
                           bool &ErrorsFound            // set to true if problem
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Bo Shen
    //       DATE WRITTEN   July 2016
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function looks up the given coil and returns the inlet node.  If
    // incorrect coil type or name is given, ErrorsFound is returned as true and value is returned
    // as zero.

    // Return value
    int NodeNumber(0); // returned outlet node of matched coil

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    int WhichCoil;

    // Obtains and Allocates WatertoAirHP related parameters from input file
    if (state.dataIntegratedHP->GetCoilsInputFlag) { // First time subroutine has been entered
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    }

    WhichCoil = Util::FindItemInList(CoilName, state.dataIntegratedHP->IntegratedHeatPumps);
    if (WhichCoil != 0) {
        NodeNumber = state.dataIntegratedHP->IntegratedHeatPumps(WhichCoil).ODAirInletNodeNum;
    }

    if (WhichCoil == 0) {
        ShowSevereError(state, std::format(R"(GetCoilInletNodeIHP: Could not find CoilType="{}" with Name="{}")", CoilType, CoilName));
        ErrorsFound = true;
        NodeNumber = 0;
    }

    return NodeNumber;
}

int GetDWHCoilOutletNodeIHP(EnergyPlusData &state,
                            std::string const &CoilType, // must match coil types in this module
                            std::string const &CoilName, // must match coil names for the coil type
                            bool &ErrorsFound            // set to true if problem
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Bo Shen
    //       DATE WRITTEN   July 2016
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function looks up the given coil and returns the outlet node.  If
    // incorrect coil type or name is given, ErrorsFound is returned as true and value is returned
    // as zero.

    // Return value
    int NodeNumber(0); // returned outlet node of matched coil

    // FUNCTION LOCAL VARIABLE DECLARATIONS:
    int WhichCoil;

    // Obtains and Allocates WatertoAirHP related parameters from input file
    if (state.dataIntegratedHP->GetCoilsInputFlag) { // First time subroutine has been entered
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    }

    WhichCoil = Util::FindItemInList(CoilName, state.dataIntegratedHP->IntegratedHeatPumps);
    if (WhichCoil != 0) {
        NodeNumber = state.dataIntegratedHP->IntegratedHeatPumps(WhichCoil).ODAirOutletNodeNum;
    }

    if (WhichCoil == 0) {
        ShowSevereError(state, std::format(R"(GetCoilInletNodeIHP: Could not find CoilType="{}" with Name="{}")", CoilType, CoilName));
        ErrorsFound = true;
        NodeNumber = 0;
    }

    return NodeNumber;
}

int GetIHPDWHCoilPLFFPLR(EnergyPlusData &state,
                         std::string const &CoilType,                  // must match coil types in this module
                         std::string const &CoilName,                  // must match coil names for the coil type
                         [[maybe_unused]] IHPOperationMode const Mode, // mode coil type
                         bool &ErrorsFound                             // set to true if problem
)
{
    // FUNCTION INFORMATION:
    //       AUTHOR         Bo Shen
    //       DATE WRITTEN   March, 2016
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function looks up the given coil and returns PLR curve index.  If
    // incorrect coil type or name is given, ErrorsFound is returned as true and value is returned
    // as zero.

    // Using/Aliasing
    using VariableSpeedCoils::GetVSCoilPLFFPLR;

    // Return value
    int PLRNumber(0); // returned outlet node of matched coil

    // Obtains and Allocates WatertoAirHP related parameters from input file
    if (state.dataIntegratedHP->GetCoilsInputFlag) { // First time subroutine has been entered
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    }

    int WhichCoil = Util::FindItemInList(CoilName, state.dataIntegratedHP->IntegratedHeatPumps);
    if (WhichCoil != 0) {

        auto &ihp = state.dataIntegratedHP->IntegratedHeatPumps(WhichCoil);

        // this will be called by HPWH parent
        if (ihp.DWHCoilIndex > 0) {
            PLRNumber = GetVSCoilPLFFPLR(state, ihp.DWHCoilType, ihp.DWHCoilName, ErrorsFound);
        } else {
            PLRNumber = GetVSCoilPLFFPLR(state, ihp.SCWHCoilType, ihp.SCWHCoilName, ErrorsFound);
        }
    } else {
        WhichCoil = 0;
    }

    if (WhichCoil == 0) {
        ShowSevereError(state, std::format(R"(GetIHPDWHCoilPLFFPLR: Could not find CoilType="{}" with Name="{}")", CoilType, CoilName));
        ErrorsFound = true;
        PLRNumber = 0;
    }

    return PLRNumber;
}

Real64 GetDWHCoilCapacityIHP(EnergyPlusData &state,
                             std::string const &CoilType,                  // must match coil types in this module
                             std::string const &CoilName,                  // must match coil names for the coil type
                             [[maybe_unused]] IHPOperationMode const Mode, // mode coil type
                             bool &ErrorsFound                             // set to true if problem
)
{

    // FUNCTION INFORMATION:
    //       AUTHOR         Bo Shen
    //       DATE WRITTEN   Jan 2016
    //       MODIFIED       na
    //       RE-ENGINEERED  na

    // PURPOSE OF THIS FUNCTION:
    // This function looks up the rated coil capacity at the nominal speed level for the given coil and returns it.  If
    // incorrect coil type or name is given, ErrorsFound is returned as true and capacity is returned
    // as negative.

    // Using/Aliasing
    using VariableSpeedCoils::GetCoilCapacityVariableSpeed;

    // Return value
    Real64 CoilCapacity; // returned capacity of matched coil

    // Obtains and Allocates WatertoAirHP related parameters from input file
    if (state.dataIntegratedHP->GetCoilsInputFlag) { // First time subroutine has been entered
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    }

    int WhichCoil = Util::FindItemInList(CoilName, state.dataIntegratedHP->IntegratedHeatPumps);
    if (WhichCoil != 0) {

        auto &ihp = state.dataIntegratedHP->IntegratedHeatPumps(WhichCoil);

        if (ihp.IHPCoilsSized == false) {
            SizeIHP(state, WhichCoil);
        }

        if (ihp.DWHCoilIndex > 0) {
            CoilCapacity = GetCoilCapacityVariableSpeed(state, ihp.DWHCoilType, ihp.DWHCoilName, ErrorsFound);
        } else {
            CoilCapacity = GetCoilCapacityVariableSpeed(state, ihp.SCWHCoilType, ihp.SCWHCoilName, ErrorsFound);
        }
    } else {
        WhichCoil = 0;
    }

    if (WhichCoil == 0) {
        ShowSevereError(state, std::format(R"(GetCoilCapacityVariableSpeed: Could not find CoilType="{}" with Name="{}")", CoilType, CoilName));
        ErrorsFound = true;
        CoilCapacity = -1000.0;
    }

    return CoilCapacity;
}

int GetLowSpeedNumIHP(EnergyPlusData &state, int const DXCoilNum)
{
    int SpeedNum(0);

    // Obtains and Allocates WatertoAirHP related parameters from input file
    if (state.dataIntegratedHP->GetCoilsInputFlag) { // First time subroutine has been entered
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    }

    if (DXCoilNum > static_cast<int>(state.dataIntegratedHP->IntegratedHeatPumps.size()) || DXCoilNum < 1) {
        ShowFatalError(state,
                       std::format("GetLowSpeedNumIHP: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name=AS-IHP",
                                   DXCoilNum,
                                   state.dataIntegratedHP->IntegratedHeatPumps.size()));
    }

    auto const &ihp = state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum);

    switch (ihp.CurMode) {
    case IHPOperationMode::Idle:
    case IHPOperationMode::SpaceClg:
    case IHPOperationMode::SpaceHtg:
    case IHPOperationMode::DedicatedWaterHtg:
        SpeedNum = 1;
        break;
    case IHPOperationMode::SCWHMatchSC:
    case IHPOperationMode::SCWHMatchWH:
        SpeedNum = ihp.MinSpedSCWH;
        break;
    case IHPOperationMode::SpaceClgDedicatedWaterHtg:
        SpeedNum = ihp.MinSpedSCDWH;
        break;
    case IHPOperationMode::SHDWHElecHeatOff:
    case IHPOperationMode::SHDWHElecHeatOn:
        SpeedNum = ihp.MinSpedSHDWH;
        break;
    default:
        SpeedNum = 1;
        break;
    }

    return (SpeedNum);
}

int GetMaxSpeedNumIHP(EnergyPlusData &state, int const DXCoilNum)
{
    // Obtains and Allocates WatertoAirHP related parameters from input file
    if (state.dataIntegratedHP->GetCoilsInputFlag) { // First time subroutine has been entered
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    }

    if (DXCoilNum > static_cast<int>(state.dataIntegratedHP->IntegratedHeatPumps.size()) || DXCoilNum < 1) {
        ShowFatalError(state,
                       std::format("GetMaxSpeedNumIHP: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name=AS-IHP",
                                   DXCoilNum,
                                   state.dataIntegratedHP->IntegratedHeatPumps.size()));
    }

    int SpeedNum(0);
    auto &ihp = state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum);

    switch (ihp.CurMode) {
    case IHPOperationMode::Idle:
    case IHPOperationMode::SpaceClg:
        SpeedNum = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCCoilIndex).NumOfSpeeds;
        break;
    case IHPOperationMode::SpaceHtg:
        SpeedNum = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHCoilIndex).NumOfSpeeds;
        break;
    case IHPOperationMode::DedicatedWaterHtg:
        SpeedNum = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.DWHCoilIndex).NumOfSpeeds;
        break;
    case IHPOperationMode::SCWHMatchSC:
    case IHPOperationMode::SCWHMatchWH:
        SpeedNum = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilIndex).NumOfSpeeds;
        break;
    case IHPOperationMode::SpaceClgDedicatedWaterHtg:
        SpeedNum = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHCoolCoilIndex).NumOfSpeeds;
        break;
    case IHPOperationMode::SHDWHElecHeatOff:
    case IHPOperationMode::SHDWHElecHeatOn:
        SpeedNum = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHHeatCoilIndex).NumOfSpeeds;
        break;
    default:
        SpeedNum = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCCoilIndex).NumOfSpeeds;
        break;
    }

    return (SpeedNum);
}

Real64 GetAirVolFlowRateIHP(EnergyPlusData &state,
                            int const DXCoilNum,
                            int const SpeedNum,
                            Real64 const SpeedRatio,
                            bool const IsCallbyWH // whether the call from the water heating loop or air loop, true = from water heating loop
)
{
    int IHPCoilIndex(0);
    Real64 AirVolFlowRate(0.0);
    Real64 FlowScale(1.0);
    bool IsResultFlow(false); // IsResultFlow = true, the air flow rate will be from a simultaneous mode, won't be re-calculated

    // Obtains and Allocates WatertoAirHP related parameters from input file
    if (state.dataIntegratedHP->GetCoilsInputFlag) { // First time subroutine has been entered
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    }

    if (DXCoilNum > static_cast<int>(state.dataIntegratedHP->IntegratedHeatPumps.size()) || DXCoilNum < 1) {
        ShowFatalError(state,
                       std::format("GetAirVolFlowRateIHP: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name=AS-IHP",
                                   DXCoilNum,
                                   state.dataIntegratedHP->IntegratedHeatPumps.size()));
    }

    auto &ihp = state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum);

    if (!ihp.IHPCoilsSized) {
        SizeIHP(state, DXCoilNum);
    }

    FlowScale = 0.0;
    switch (ihp.CurMode) {
    case IHPOperationMode::Idle:
        IHPCoilIndex = ihp.SCCoilIndex;
        break;
    case IHPOperationMode::SpaceClg:
        IHPCoilIndex = ihp.SCCoilIndex;
        if (!IsCallbyWH) // call from air loop
        {
            FlowScale = ihp.CoolVolFlowScale;
        }

        break;
    case IHPOperationMode::SpaceHtg:
        IHPCoilIndex = ihp.SHCoilIndex;
        if (!IsCallbyWH) // call from air loop
        {
            FlowScale = ihp.HeatVolFlowScale;
        }
        break;
    case IHPOperationMode::DedicatedWaterHtg:
        IHPCoilIndex = ihp.DWHCoilIndex;
        FlowScale = 1.0;
        break;
    case IHPOperationMode::SCWHMatchSC:
        IHPCoilIndex = ihp.SCWHCoilIndex;
        FlowScale = ihp.CoolVolFlowScale;
        if (IsCallbyWH) // call from water loop
        {
            IsResultFlow = true;
            AirVolFlowRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilIndex).AirVolFlowRate;
        }
        break;
    case IHPOperationMode::SCWHMatchWH:
        IHPCoilIndex = ihp.SCWHCoilIndex;
        FlowScale = ihp.CoolVolFlowScale;
        if (!IsCallbyWH) {
            IsResultFlow = true;
            AirVolFlowRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilIndex).AirVolFlowRate;
        }
        break;
    case IHPOperationMode::SpaceClgDedicatedWaterHtg:
        IHPCoilIndex = ihp.SCDWHCoolCoilIndex;
        FlowScale = ihp.CoolVolFlowScale;
        if (IsCallbyWH) {
            IsResultFlow = true;
            AirVolFlowRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHCoolCoilIndex).AirVolFlowRate;
        }
        break;
    case IHPOperationMode::SHDWHElecHeatOff:
    case IHPOperationMode::SHDWHElecHeatOn:
        IHPCoilIndex = ihp.SHDWHHeatCoilIndex;
        FlowScale = ihp.HeatVolFlowScale;
        if (IsCallbyWH) {
            IsResultFlow = true;
            AirVolFlowRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHHeatCoilIndex).AirVolFlowRate;
        }
        break;
    default:
        IHPCoilIndex = ihp.SCCoilIndex;
        FlowScale = 0.0;
        break;
    }

    if (!IsResultFlow) {
        if (1 == SpeedNum) {
            AirVolFlowRate = state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedAirVolFlowRate(SpeedNum);
        } else {
            AirVolFlowRate = SpeedRatio * state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedAirVolFlowRate(SpeedNum) +
                             (1.0 - SpeedRatio) * state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedAirVolFlowRate(SpeedNum - 1);
        }

        AirVolFlowRate = AirVolFlowRate * FlowScale;
    }

    if (AirVolFlowRate > ihp.MaxCoolAirVolFlow) {
        AirVolFlowRate = ihp.MaxCoolAirVolFlow;
    }
    if (AirVolFlowRate > ihp.MaxHeatAirVolFlow) {
        AirVolFlowRate = ihp.MaxHeatAirVolFlow;
    }

    return (AirVolFlowRate);
}

Real64 GetWaterVolFlowRateIHP(EnergyPlusData &state, int const DXCoilNum, int const SpeedNum, Real64 const SpeedRatio)
{
    int IHPCoilIndex(0);
    Real64 WaterVolFlowRate(0.0);

    // Obtains and Allocates WatertoAirHP related parameters from input file
    if (state.dataIntegratedHP->GetCoilsInputFlag) { // First time subroutine has been entered
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    }

    if (DXCoilNum > static_cast<int>(state.dataIntegratedHP->IntegratedHeatPumps.size()) || DXCoilNum < 1) {
        ShowFatalError(state,
                       std::format("GetWaterVolFlowRateIHP: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name=AS-IHP",
                                   DXCoilNum,
                                   state.dataIntegratedHP->IntegratedHeatPumps.size()));
    }

    auto const &ihp = state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum);

    if (!ihp.IHPCoilsSized) {
        SizeIHP(state, DXCoilNum);
    }

    switch (ihp.CurMode) {
    case IHPOperationMode::Idle:
    case IHPOperationMode::SpaceClg:
    case IHPOperationMode::SpaceHtg:
        WaterVolFlowRate = 0.0;
        break;
    case IHPOperationMode::DedicatedWaterHtg:
        IHPCoilIndex = ihp.DWHCoilIndex;
        if (1 == SpeedNum) {
            WaterVolFlowRate = state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum);
        } else {
            WaterVolFlowRate = SpeedRatio * state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum) +
                               (1.0 - SpeedRatio) * state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum - 1);
        }
        break;
    case IHPOperationMode::SCWHMatchSC:
    case IHPOperationMode::SCWHMatchWH:
        IHPCoilIndex = ihp.SCWHCoilIndex;
        if (1 == SpeedNum) {
            WaterVolFlowRate = state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum);
        } else {
            WaterVolFlowRate = SpeedRatio * state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum) +
                               (1.0 - SpeedRatio) * state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum - 1);
        }
        break;
    case IHPOperationMode::SpaceClgDedicatedWaterHtg:
        IHPCoilIndex = ihp.SCDWHWHCoilIndex;
        if (1 == SpeedNum) {
            WaterVolFlowRate = state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum);
        } else {
            WaterVolFlowRate = SpeedRatio * state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum) +
                               (1.0 - SpeedRatio) * state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum - 1);
        }
        break;
    case IHPOperationMode::SHDWHElecHeatOff:
    case IHPOperationMode::SHDWHElecHeatOn:
        IHPCoilIndex = ihp.SHDWHWHCoilIndex;
        if (1 == SpeedNum) {
            WaterVolFlowRate = state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum);
        } else {
            WaterVolFlowRate = SpeedRatio * state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum) +
                               (1.0 - SpeedRatio) * state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum - 1);
        }
        break;
    default:
        WaterVolFlowRate = 0.0;
        break;
    }

    return (WaterVolFlowRate);
}

Real64 GetAirMassFlowRateIHP(EnergyPlusData &state,
                             int const DXCoilNum,
                             int const SpeedNum,
                             Real64 const SpeedRatio,
                             bool const IsCallbyWH // whether the call from the water heating loop or air loop, true = from water heating loop
)
{
    int IHPCoilIndex(0);
    Real64 AirMassFlowRate(0.0);
    Real64 FlowScale(1.0);
    bool IsResultFlow(false); // IsResultFlow = true, the air flow rate will be from a simultaneous mode, won't be re-calculated

    // Obtains and Allocates WatertoAirHP related parameters from input file
    if (state.dataIntegratedHP->GetCoilsInputFlag) { // First time subroutine has been entered
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    }

    if (DXCoilNum > static_cast<int>(state.dataIntegratedHP->IntegratedHeatPumps.size()) || DXCoilNum < 1) {
        ShowFatalError(state,
                       std::format("GetAirMassFlowRateIHP: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name=AS-IHP",
                                   DXCoilNum,
                                   state.dataIntegratedHP->IntegratedHeatPumps.size()));
    }

    auto &ihp = state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum);

    if (!ihp.IHPCoilsSized) {
        SizeIHP(state, DXCoilNum);
    }

    FlowScale = 0.0;
    switch (ihp.CurMode) {
    case IHPOperationMode::Idle:
        IHPCoilIndex = ihp.SCCoilIndex;
        AirMassFlowRate = 0.0;
        break;
    case IHPOperationMode::SpaceClg:
        IHPCoilIndex = ihp.SCCoilIndex;
        if (!IsCallbyWH) {
            FlowScale = ihp.CoolVolFlowScale;
        } else {
            IsResultFlow = true;
            AirMassFlowRate = ihp.AirFlowSavInAirLoop;
        }
        break;
    case IHPOperationMode::SpaceHtg:
        IHPCoilIndex = ihp.SHCoilIndex;
        if (!IsCallbyWH) {
            FlowScale = ihp.HeatVolFlowScale;
        } else {
            IsResultFlow = true;
            AirMassFlowRate = ihp.AirFlowSavInAirLoop;
        }
        break;
    case IHPOperationMode::DedicatedWaterHtg:
        IHPCoilIndex = ihp.DWHCoilIndex;
        FlowScale = 1.0;
        break;
    case IHPOperationMode::SCWHMatchSC:
        IHPCoilIndex = ihp.SCWHCoilIndex;
        FlowScale = ihp.CoolVolFlowScale;
        state.dataLoopNodes->Node(ihp.WaterInletNodeNum).MassFlowRate = GetWaterVolFlowRateIHP(state, DXCoilNum, SpeedNum, SpeedRatio) * WaterDensity;
        if (IsCallbyWH) {
            IsResultFlow = true;
            AirMassFlowRate = ihp.AirFlowSavInAirLoop;
        }
        break;
    case IHPOperationMode::SCWHMatchWH:
        IHPCoilIndex = ihp.SCWHCoilIndex;
        FlowScale = ihp.CoolVolFlowScale;
        if (!IsCallbyWH) {
            IsResultFlow = true;
            AirMassFlowRate = ihp.AirFlowSavInWaterLoop;
        }
        break;
    case IHPOperationMode::SpaceClgDedicatedWaterHtg:
        IHPCoilIndex = ihp.SCDWHCoolCoilIndex;
        FlowScale = ihp.CoolVolFlowScale;
        state.dataLoopNodes->Node(ihp.WaterInletNodeNum).MassFlowRate = GetWaterVolFlowRateIHP(state, DXCoilNum, SpeedNum, SpeedRatio) * WaterDensity;
        if (IsCallbyWH) {
            IsResultFlow = true;
            AirMassFlowRate = ihp.AirFlowSavInAirLoop;
        }
        break;
    case IHPOperationMode::SHDWHElecHeatOff:
    case IHPOperationMode::SHDWHElecHeatOn:
        IHPCoilIndex = ihp.SHDWHHeatCoilIndex;
        FlowScale = ihp.HeatVolFlowScale;
        state.dataLoopNodes->Node(ihp.WaterInletNodeNum).MassFlowRate = GetWaterVolFlowRateIHP(state, DXCoilNum, SpeedNum, SpeedRatio) * WaterDensity;
        if (IsCallbyWH) {
            IsResultFlow = true;
            AirMassFlowRate = ihp.AirFlowSavInAirLoop;
        }
        break;
    default:
        IHPCoilIndex = ihp.SCCoilIndex;
        FlowScale = 0.0;
        break;
    }

    if (!IsResultFlow) {
        if (SpeedNum == 1) {
            AirMassFlowRate = state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedAirMassFlowRate(SpeedNum);
        } else {
            AirMassFlowRate = SpeedRatio * state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedAirMassFlowRate(SpeedNum) +
                              (1.0 - SpeedRatio) * state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedAirMassFlowRate(SpeedNum - 1);
        }

        AirMassFlowRate = AirMassFlowRate * FlowScale;
    }

    if (AirMassFlowRate > ihp.MaxCoolAirMassFlow) {
        AirMassFlowRate = ihp.MaxCoolAirMassFlow;
    }
    if (AirMassFlowRate > ihp.MaxHeatAirMassFlow) {
        AirMassFlowRate = ihp.MaxHeatAirMassFlow;
    }

    // set max air flow rate
    state.dataLoopNodes->Node(ihp.AirCoolInletNodeNum).MassFlowRateMax = AirMassFlowRate;
    state.dataLoopNodes->Node(ihp.AirHeatInletNodeNum).MassFlowRateMax = AirMassFlowRate;
    state.dataLoopNodes->Node(ihp.AirOutletNodeNum).MassFlowRateMax = AirMassFlowRate;

    return AirMassFlowRate;
}

} // namespace EnergyPlus::IntegratedHeatPump

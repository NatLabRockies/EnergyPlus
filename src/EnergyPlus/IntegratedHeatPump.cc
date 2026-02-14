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

// Using/Aliasing
using namespace DataLoopNode;

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
            ShowFatalError(state, format("Integrated Heat Pump not found={}", CompName));
        }
        CompIndex = DXCoilNum;
    } else {
        DXCoilNum = CompIndex;
        if (DXCoilNum > static_cast<int>(state.dataIntegratedHP->IntegratedHeatPumps.size()) || DXCoilNum < 1) {
            ShowFatalError(state,
                           format("SimIHP: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name={}",
                                  DXCoilNum,
                                  state.dataIntegratedHP->IntegratedHeatPumps.size(),
                                  CompName));
        }
        if (!CompName.empty() && CompName != state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum).Name) {
            ShowFatalError(state,
                           format("SimIHP: Invalid CompIndex passed={}, Integrated HP name={}, stored Integrated HP Name for that index={}",
                                  DXCoilNum,
                                  CompName,
                                  state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum).Name));
        }
    }

    SimIHP(state,
           DXCoilNum,
           fanOp,
           compressorOp, 
           PartLoadFrac,
           SpeedNum,
           SpeedRatio,
           SensLoad,
           LatentLoad,
           IsCallbyWH,
           FirstHVACIteration,
           OnOffAirFlowRat);

}
  
void SimIHP(EnergyPlusData &state,
            int const ihpNum,
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
    assert(ihpNum >= 0 && ihpNum <= (int)state.dataIntegratedHP->IntegratedHeatPumps.size());
    auto &ihp = state.dataIntegratedHP->IntegratedHeatPumps(ihpNum);

    if (!ihp.IHPCoilsSized) SizeIHP(state, ihpNum);

    InitializeIHP(state, ihpNum);

    Real64 airMassFlowRate = state.dataLoopNodes->Node(ihp.AirCoolInletNodeNum).MassFlowRate;
    ihp.AirLoopFlowRate = airMassFlowRate;

    switch (ihp.CurMode) {
    case IHPOperationMode::SpaceClg: {
        if (!IsCallbyWH) // process when called from air loop
        {
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCDWHCoolCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCDWHWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SHDWHHeatCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SHDWHWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.DWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);

            VariableSpeedCoils::SimVariableSpeedCoils(state,
                                  ihp.SCCoilNum,
                                  fanOp,
                                  compressorOp,
                                  PartLoadFrac,
                                  SpeedNum,
                                  SpeedRatio,
                                  SensLoad,
                                  LatentLoad,
                                  OnOffAirFlowRat);

            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);

            ihp.AirFlowSavInAirLoop = airMassFlowRate;
        }

        ihp.TankSourceWaterMassFlowRate = 0.0;
    } break;

    case IHPOperationMode::SpaceHtg: {
        if (!IsCallbyWH) // process when called from air loop
        {
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCDWHCoolCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCDWHWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SHDWHHeatCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SHDWHWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.DWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);

            VariableSpeedCoils::SimVariableSpeedCoils(state,
                                  ihp.SHCoilNum,
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
    } break;

    case IHPOperationMode::DedicatedWaterHtg: {
        if (IsCallbyWH) // process when called from water loop
        {
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCDWHCoolCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCDWHWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SHDWHHeatCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SHDWHWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);

            VariableSpeedCoils::SimVariableSpeedCoils(state,
                                  ihp.DWHCoilNum,
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
    } break;

    case IHPOperationMode::SCWHMatchSC: {
        if (!IsCallbyWH) // process when called from air loop
        {
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCDWHCoolCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCDWHWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SHDWHHeatCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SHDWHWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.DWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);

            VariableSpeedCoils::SimVariableSpeedCoils(state,
                                  ihp.SCWHCoilNum,
                                  fanOp,
                                  compressorOp,
                                  PartLoadFrac,
                                  SpeedNum,
                                  SpeedRatio,
                                  SensLoad,
                                  LatentLoad,
                                  OnOffAirFlowRat);

            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);

            ihp.AirFlowSavInAirLoop = airMassFlowRate;
        }

        ihp.TankSourceWaterMassFlowRate = state.dataLoopNodes->Node(ihp.WaterInletNodeNum).MassFlowRate;

    } break;

    case IHPOperationMode::SCWHMatchWH: {
        if (IsCallbyWH) // process when called from water loop
        {
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCDWHCoolCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCDWHWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SHDWHHeatCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SHDWHWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.DWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);

            VariableSpeedCoils::SimVariableSpeedCoils(state,
                                  ihp.SCWHCoilNum,
                                  fanOp,
                                  compressorOp,
                                  PartLoadFrac,
                                  SpeedNum,
                                  SpeedRatio,
                                  SensLoad,
                                  LatentLoad,
                                  OnOffAirFlowRat);

            VariableSpeedCoils::SimVariableSpeedCoils(state,  ihp.SHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);

            ihp.AirFlowSavInWaterLoop = airMassFlowRate;
        }

        ihp.TankSourceWaterMassFlowRate = state.dataLoopNodes->Node(ihp.WaterInletNodeNum).MassFlowRate;
    } break;

    case IHPOperationMode::SpaceClgDedicatedWaterHtg: {
        if (!IsCallbyWH) // process when called from air loop
        {
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SHDWHHeatCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SHDWHWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.DWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);

            VariableSpeedCoils::SimVariableSpeedCoils(state,
                                  ihp.SCDWHWHCoilNum,
                                  fanOp,
                                  compressorOp,
                                  PartLoadFrac,
                                  SpeedNum,
                                  SpeedRatio,
                                  SensLoad,
                                  LatentLoad,
                                  OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state,
                                  ihp.SCDWHCoolCoilNum,
                                  fanOp,
                                  compressorOp,
                                  PartLoadFrac,
                                  SpeedNum,
                                  SpeedRatio,
                                  SensLoad,
                                  LatentLoad,
                                  OnOffAirFlowRat);

            VariableSpeedCoils::SimVariableSpeedCoils(state,  ihp.SHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);

            ihp.AirFlowSavInAirLoop = airMassFlowRate;
        }

        ihp.TankSourceWaterMassFlowRate = state.dataLoopNodes->Node(ihp.WaterInletNodeNum).MassFlowRate;
    } break;

    case IHPOperationMode::SHDWHElecHeatOff:
    case IHPOperationMode::SHDWHElecHeatOn: {
        if (!IsCallbyWH) // process when called from air loop
        {
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCDWHCoolCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCDWHWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.DWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);

            VariableSpeedCoils::SimVariableSpeedCoils(state,
                                  ihp.SHDWHWHCoilNum,
                                  fanOp,
                                  compressorOp,
                                  PartLoadFrac,
                                  SpeedNum,
                                  SpeedRatio,
                                  SensLoad,
                                  LatentLoad,
                                  OnOffAirFlowRat);
            VariableSpeedCoils::SimVariableSpeedCoils(state,
                                  ihp.SHDWHHeatCoilNum,
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
    } break;

    case IHPOperationMode::Idle:
    default: { // clear up
        VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCDWHCoolCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
        VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCDWHWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
        VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SHDWHHeatCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
        VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SHDWHWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
        VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
        VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SCCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
        VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.SHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
        VariableSpeedCoils::SimVariableSpeedCoils(state, ihp.DWHCoilNum, fanOp, compressorOp, 0.0, 1, 0.0, 0.0, 0.0, OnOffAirFlowRat);
        ihp.TankSourceWaterMassFlowRate = 0.0;
        ihp.AirFlowSavInAirLoop = 0.0;
        ihp.AirFlowSavInWaterLoop = 0.0;
    } break;
    } // 
    UpdateIHP(state, ihpNum);
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
    using namespace NodeInputManager;
    using BranchNodeConnections::OverrideNodeConnectionType;
    using BranchNodeConnections::RegisterNodeConnection;
    using BranchNodeConnections::SetUpCompSets;
    using BranchNodeConnections::TestCompSet;
    using GlobalNames::VerifyUniqueCoilName;

    // SUBROUTINE PARAMETER DEFINITIONS:
    static constexpr std::string_view RoutineName("GetIHPInput: "); // include trailing blank space
    static constexpr std::string_view routineName = "GetIHPInput"; 

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

        ErrorObjectHeader eoh{routineName, CurrentModuleObject, AlphArray(1)};
        
        // ErrorsFound will be set to True if problem was found, left untouched otherwise
        VerifyUniqueCoilName(state, CurrentModuleObject, AlphArray(1), ErrorsFound, CurrentModuleObject + " Name");

        auto &ihp = state.dataIntegratedHP->IntegratedHeatPumps(CoilCounter);

        ihp.Name = AlphArray(1);
        ihp.IHPtype = "AIRSOURCE_IHP";

        // AlphArray( 2 ) is the water sensor node

        ihp.SCCoilType = HVAC::CoilType::CoolingDXVariableSpeed;
        ihp.SCCoilName = AlphArray(3);
        ihp.SCCoilTypeNum = DataLoopNode::ConnectionObjectType::CoilCoolingDXVariableSpeed;

        ihp.SCCoilNum = VariableSpeedCoils::GetCoilIndex(state, ihp.SCCoilName);
        if (ihp.SCCoilNum == 0) {
            ShowSevereItemNotFound(state, eoh, cAlphaFields(3), ihp.SCCoilName);
            ErrorsFound = true;
        }

        ihp.SHCoilType = HVAC::CoilType::HeatingDXVariableSpeed;
        ihp.SHCoilName = AlphArray(4);
        ihp.SHCoilTypeNum = DataLoopNode::ConnectionObjectType::CoilHeatingDXVariableSpeed;

        ihp.SHCoilNum = VariableSpeedCoils::GetCoilIndex(state, ihp.SHCoilName);
        if (ihp.SHCoilNum == 0) {
            ShowSevereItemNotFound(state, eoh, cAlphaFields(4), ihp.SHCoilName);
            ErrorsFound = true;
        }

        ihp.DWHCoilType = HVAC::CoilType::WaterHeatingAWHPVariableSpeed;
        ihp.DWHCoilName = AlphArray(5);
        ihp.DWHCoilTypeNum = DataLoopNode::ConnectionObjectType::CoilWaterHeatingAirToWaterHeatPumpVariableSpeed;

        ihp.DWHCoilNum = VariableSpeedCoils::GetCoilIndex(state, ihp.DWHCoilName);
        if (ihp.DWHCoilNum == 0) {
            ShowSevereItemNotFound(state, eoh, cAlphaFields(5), ihp.DWHCoilName);
            ErrorsFound = true;
        }

        ihp.SCWHCoilType = HVAC::CoilType::WaterHeatingAWHPVariableSpeed;
        ihp.SCWHCoilName = AlphArray(6);
        ihp.SCWHCoilTypeNum = DataLoopNode::ConnectionObjectType::CoilWaterHeatingAirToWaterHeatPumpVariableSpeed;

        ihp.SCWHCoilNum = VariableSpeedCoils::GetCoilIndex(state, ihp.SCWHCoilName);
        if (ihp.SCWHCoilNum == 0) {
            ShowSevereItemNotFound(state, eoh, cAlphaFields(6), ihp.SCWHCoilName);
            ErrorsFound = true;
        }

        ihp.SCDWHCoolCoilType = HVAC::CoilType::CoolingDXVariableSpeed;
        ihp.SCDWHCoolCoilName = AlphArray(7);
        ihp.SCDWHCoolCoilTypeNum = DataLoopNode::ConnectionObjectType::CoilCoolingDXVariableSpeed;

        ihp.SCDWHCoolCoilNum = VariableSpeedCoils::GetCoilIndex(state, ihp.SCDWHCoolCoilName);
        if (ihp.SCDWHCoolCoilNum == 0) {
            ShowSevereItemNotFound(state, eoh, cAlphaFields(7), ihp.SCDWHCoolCoilName);
            ErrorsFound = true;
        }

        ihp.SCDWHWHCoilType = HVAC::CoilType::WaterHeatingAWHPVariableSpeed;
        ihp.SCDWHWHCoilName = AlphArray(8);
        ihp.SCDWHWHCoilTypeNum = DataLoopNode::ConnectionObjectType::CoilWaterHeatingAirToWaterHeatPumpVariableSpeed;

        ihp.SCDWHWHCoilNum = VariableSpeedCoils::GetCoilIndex(state, ihp.SCDWHWHCoilName);
        if (ihp.SCDWHWHCoilNum == 0) {
            ShowSevereItemNotFound(state, eoh, cAlphaFields(8), ihp.SCDWHWHCoilName);
            ErrorsFound = true;
        } else {
            state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHWHCoilNum).bIsDesuperheater = true;
        }

        ihp.SHDWHHeatCoilType = HVAC::CoilType::HeatingDXVariableSpeed;
        ihp.SHDWHHeatCoilName = AlphArray(9);
        ihp.SHDWHHeatCoilTypeNum = DataLoopNode::ConnectionObjectType::CoilHeatingDXVariableSpeed;

        ihp.SHDWHHeatCoilNum = VariableSpeedCoils::GetCoilIndex(state, ihp.SHDWHHeatCoilName);
        if (ihp.SHDWHHeatCoilNum == 0) {
            ShowSevereItemNotFound(state, eoh, cAlphaFields(9), ihp.SHDWHHeatCoilName);
            ErrorsFound = true;
        }

        ihp.SHDWHWHCoilType = HVAC::CoilType::WaterHeatingAWHPVariableSpeed;
        ihp.SHDWHWHCoilName = AlphArray(10);
        ihp.SHDWHWHCoilTypeNum = DataLoopNode::ConnectionObjectType::CoilWaterHeatingAirToWaterHeatPumpVariableSpeed;

        ihp.SHDWHWHCoilNum = VariableSpeedCoils::GetCoilIndex(state, ihp.SHDWHWHCoilName);
        if (ihp.SHDWHWHCoilNum == 0) {
            ShowSevereItemNotFound(state, eoh, cAlphaFields(10), ihp.SHDWHWHCoilName);
            ErrorsFound = true;
        } else {
            state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHWHCoilNum).bIsDesuperheater = true;
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
        int ChildCoilIndex = ihp.SCCoilNum;
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
                               DataLoopNode::ConnectionObjectType::CoilSystemIntegratedHeatPumpAirSource,
                               ihp.Name + " Cooling Coil",
                               DataLoopNode::ConnectionType::Inlet,
                               NodeInputManager::CompFluidStream::Primary,
                               ObjectIsNotParent,
                               ErrorsFound);
        RegisterNodeConnection(state,
                               OutNode,
                               state.dataLoopNodes->NodeID(OutNode),
                               DataLoopNode::ConnectionObjectType::CoilSystemIntegratedHeatPumpAirSource,
                               ihp.Name + " Cooling Coil",
                               DataLoopNode::ConnectionType::Outlet,
                               NodeInputManager::CompFluidStream::Primary,
                               ObjectIsNotParent,
                               ErrorsFound);

        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Cooling Coil", HVAC::coilTypeNames[(int)ihp.SCCoilType], ihp.SCCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.SCCoilTypeNum,
                                   ihp.SCCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Primary,
                                   ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.SCCoilTypeNum,
                                   ihp.SCCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Primary,
                                   ObjectIsNotParent,
                                   ErrorsFound);

        if ((state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilNum).AirInletNodeNum != InNode) ||
            (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilNum).AirOutletNodeNum != OutNode)) {
            ShowContinueError(state, format("Mistaken air node connection: {}{}-wrong coil node names.", CurrentModuleObject, ihp.SCWHCoilName));
            ErrorsFound = true;
        }
        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Cooling Coil", HVAC::coilTypeNames[(int)ihp.SCWHCoilType], ihp.SCWHCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.SCWHCoilTypeNum,
                                   ihp.SCWHCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Primary,
                                   ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.SCWHCoilTypeNum,
                                   ihp.SCWHCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Primary,
                                   ObjectIsNotParent,
                                   ErrorsFound);

        if ((state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHCoolCoilNum).AirInletNodeNum != InNode) ||
            (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHCoolCoilNum).AirOutletNodeNum != OutNode)) {
            ShowContinueError(state, format("Mistaken air node connection: {}{}-wrong coil node names.", CurrentModuleObject, ihp.SCDWHCoolCoilName));
            ErrorsFound = true;
        }
        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Cooling Coil", HVAC::coilTypeNames[(int)ihp.SCDWHCoolCoilType], ihp.SCDWHCoolCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.SCDWHCoolCoilTypeNum,
                                   ihp.SCDWHCoolCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Primary,
                                   ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.SCDWHCoolCoilTypeNum,
                                   ihp.SCDWHCoolCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Primary,
                                   ObjectIsNotParent,
                                   ErrorsFound);

        // heating coil air node connections
        ChildCoilIndex = ihp.SHCoilNum;

        InNode = ihp.AirHeatInletNodeNum;
        OutNode = state.dataVariableSpeedCoils->VarSpeedCoil(ChildCoilIndex).AirOutletNodeNum;
        ihp.AirOutletNodeNum = OutNode;
        InNodeName = state.dataLoopNodes->NodeID(InNode);
        OutNodeName = state.dataLoopNodes->NodeID(OutNode);
        if (state.dataVariableSpeedCoils->VarSpeedCoil(ChildCoilIndex).AirInletNodeNum != InNode) {
            ShowContinueError(state,
                              format("Mistaken air node connection: {}- cooling coil outlet mismatches heating coil inlet.", CurrentModuleObject));
            ErrorsFound = true;
        }
        TestCompSet(state, CurrentModuleObject, ihp.Name + " Heating Coil", InNodeName, OutNodeName, "Heating Air Nodes");
        RegisterNodeConnection(state,
                               InNode,
                               state.dataLoopNodes->NodeID(InNode),
                               DataLoopNode::ConnectionObjectType::CoilSystemIntegratedHeatPumpAirSource,
                               ihp.Name + " Heating Coil",
                               DataLoopNode::ConnectionType::Inlet,
                               NodeInputManager::CompFluidStream::Primary,
                               ObjectIsNotParent,
                               ErrorsFound);
        RegisterNodeConnection(state,
                               OutNode,
                               state.dataLoopNodes->NodeID(OutNode),
                               DataLoopNode::ConnectionObjectType::CoilSystemIntegratedHeatPumpAirSource,
                               ihp.Name + " Heating Coil",
                               DataLoopNode::ConnectionType::Outlet,
                               NodeInputManager::CompFluidStream::Primary,
                               ObjectIsNotParent,
                               ErrorsFound);

        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Heating Coil", HVAC::coilTypeNames[(int)ihp.SHCoilType], ihp.SHCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.SHCoilTypeNum,
                                   ihp.SHCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Primary,
                                   ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.SHCoilTypeNum,
                                   ihp.SHCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Primary,
                                   ObjectIsNotParent,
                                   ErrorsFound);

        if ((state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHHeatCoilNum).AirInletNodeNum != InNode) ||
            (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHHeatCoilNum).AirOutletNodeNum != OutNode)) {
            ShowContinueError(state,
                              format("Mistaken air node connection: {}:{}-wrong coil node names.", CurrentModuleObject, ihp.SHDWHHeatCoilName));
            ErrorsFound = true;
        }
        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Heating Coil", HVAC::coilTypeNames[(int)ihp.SHDWHHeatCoilType], ihp.SHDWHHeatCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.SHDWHHeatCoilTypeNum,
                                   ihp.SHDWHHeatCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Primary,
                                   ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.SHDWHHeatCoilTypeNum,
                                   ihp.SHDWHHeatCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Primary,
                                   ObjectIsNotParent,
                                   ErrorsFound);

        // water node connections
        ChildCoilIndex = ihp.SCWHCoilNum;

        InNode = state.dataVariableSpeedCoils->VarSpeedCoil(ChildCoilIndex).WaterInletNodeNum;
        OutNode = state.dataVariableSpeedCoils->VarSpeedCoil(ChildCoilIndex).WaterOutletNodeNum;
        InNodeName = state.dataLoopNodes->NodeID(InNode);
        OutNodeName = state.dataLoopNodes->NodeID(OutNode);
        ihp.WaterInletNodeNum = InNode;
        ihp.WaterOutletNodeNum = OutNode;
        if ((state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHWHCoilNum).WaterInletNodeNum != InNode) ||
            (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHWHCoilNum).WaterOutletNodeNum != OutNode)) {
            ShowContinueError(state, format("Mistaken air node connection: {}:{}-wrong coil node names.", CurrentModuleObject, ihp.SCDWHWHCoilName));
            ErrorsFound = true;
        }

        TestCompSet(state, CurrentModuleObject, ihp.Name + " Water Coil", InNodeName, OutNodeName, "Water Nodes");
        RegisterNodeConnection(state,
                               InNode,
                               state.dataLoopNodes->NodeID(InNode),
                               DataLoopNode::ConnectionObjectType::CoilSystemIntegratedHeatPumpAirSource,
                               ihp.Name + " Water Coil",
                               DataLoopNode::ConnectionType::Inlet,
                               NodeInputManager::CompFluidStream::Primary,
                               ObjectIsNotParent,
                               ErrorsFound);
        RegisterNodeConnection(state,
                               OutNode,
                               state.dataLoopNodes->NodeID(InNode),
                               DataLoopNode::ConnectionObjectType::CoilSystemIntegratedHeatPumpAirSource,
                               ihp.Name + " Water Coil",
                               DataLoopNode::ConnectionType::Outlet,
                               NodeInputManager::CompFluidStream::Primary,
                               ObjectIsNotParent,
                               ErrorsFound);

        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Water Coil", HVAC::coilTypeNames[(int)ihp.SCWHCoilType], ihp.SCWHCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.SCWHCoilTypeNum,
                                   ihp.SCWHCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Secondary,
                                   ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.SCWHCoilTypeNum,
                                   ihp.SCWHCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Secondary,
                                   ObjectIsNotParent,
                                   ErrorsFound);

        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Water Coil", HVAC::coilTypeNames[(int)ihp.SCDWHWHCoilType], ihp.SCDWHWHCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.SCDWHWHCoilTypeNum,
                                   ihp.SCDWHWHCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Secondary,
                                   ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.SCDWHWHCoilTypeNum,
                                   ihp.SCDWHWHCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Secondary,
                                   ObjectIsNotParent,
                                   ErrorsFound);

        if ((state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHWHCoilNum).WaterInletNodeNum != InNode) ||
            (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHWHCoilNum).WaterOutletNodeNum != OutNode)) {
            ShowContinueError(state, format("Mistaken air node connection: {}:{}-wrong coil node names.", CurrentModuleObject, ihp.SHDWHWHCoilName));
            ErrorsFound = true;
        }
        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Water Coil", HVAC::coilTypeNames[(int)ihp.SHDWHWHCoilType], ihp.SHDWHWHCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.SHDWHWHCoilTypeNum,
                                   ihp.SHDWHWHCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Secondary,
                                   ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.SHDWHWHCoilTypeNum,
                                   ihp.SHDWHWHCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Secondary,
                                   ObjectIsNotParent,
                                   ErrorsFound);

        if ((state.dataVariableSpeedCoils->VarSpeedCoil(ihp.DWHCoilNum).WaterInletNodeNum != InNode) ||
            (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.DWHCoilNum).WaterOutletNodeNum != OutNode)) {
            ShowContinueError(state, format("Mistaken air node connection: {}:{}-wrong coil node names.", CurrentModuleObject, ihp.DWHCoilName));
            ErrorsFound = true;
        }
        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Water Coil", HVAC::coilTypeNames[(int)ihp.DWHCoilType], ihp.DWHCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.DWHCoilTypeNum,
                                   ihp.DWHCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Secondary,
                                   ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.DWHCoilTypeNum,
                                   ihp.DWHCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Secondary,
                                   ObjectIsNotParent,
                                   ErrorsFound);

        ihp.WaterTankoutNod = GetOnlySingleNode(state,
                                                AlphArray(2),
                                                ErrorsFound,
                                                DataLoopNode::ConnectionObjectType::CoilSystemIntegratedHeatPumpAirSource,
                                                AlphArray(1),
                                                DataLoopNode::NodeFluidType::Water,
                                                DataLoopNode::ConnectionType::Sensor,
                                                NodeInputManager::CompFluidStream::Secondary,
                                                ObjectIsNotParent);

        // outdoor air node connections for water heating coils
        // DWH, SCDWH, SHDWH coils have the same outdoor air nodes
        ChildCoilIndex = ihp.DWHCoilNum;
        InNode = state.dataVariableSpeedCoils->VarSpeedCoil(ChildCoilIndex).AirInletNodeNum;
        OutNode = state.dataVariableSpeedCoils->VarSpeedCoil(ChildCoilIndex).AirOutletNodeNum;
        InNodeName = state.dataLoopNodes->NodeID(InNode);
        OutNodeName = state.dataLoopNodes->NodeID(OutNode);
        ihp.ODAirInletNodeNum = InNode;
        ihp.ODAirOutletNodeNum = OutNode;
        if ((state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHWHCoilNum).AirInletNodeNum != InNode) ||
            (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHWHCoilNum).AirOutletNodeNum != OutNode)) {
            ShowContinueError(state, format("Mistaken air node connection: {}:{}-wrong coil node names.", CurrentModuleObject, ihp.SCDWHWHCoilName));
            ErrorsFound = true;
        }

        TestCompSet(state, CurrentModuleObject, ihp.Name + " Outdoor Coil", InNodeName, OutNodeName, "Outdoor Air Nodes");
        RegisterNodeConnection(state,
                               InNode,
                               state.dataLoopNodes->NodeID(InNode),
                               DataLoopNode::ConnectionObjectType::CoilSystemIntegratedHeatPumpAirSource,
                               ihp.Name + " Outdoor Coil",
                               DataLoopNode::ConnectionType::Inlet,
                               NodeInputManager::CompFluidStream::Primary,
                               ObjectIsNotParent,
                               ErrorsFound);
        RegisterNodeConnection(state,
                               OutNode,
                               state.dataLoopNodes->NodeID(InNode),
                               DataLoopNode::ConnectionObjectType::CoilSystemIntegratedHeatPumpAirSource,
                               ihp.Name + " Outdoor Coil",
                               DataLoopNode::ConnectionType::Outlet,
                               NodeInputManager::CompFluidStream::Primary,
                               ObjectIsNotParent,
                               ErrorsFound);

        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Outdoor Coil", HVAC::coilTypeNames[(int)ihp.DWHCoilType], ihp.DWHCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.DWHCoilTypeNum,
                                   ihp.DWHCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Primary,
                                   ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.DWHCoilTypeNum,
                                   ihp.DWHCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Primary,
                                   ObjectIsNotParent,
                                   ErrorsFound);

        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Outdoor Coil", HVAC::coilTypeNames[(int)ihp.SCDWHWHCoilType], ihp.SCDWHWHCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.SCDWHWHCoilTypeNum,
                                   ihp.SCDWHWHCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Primary,
                                   ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.SCDWHWHCoilTypeNum,
                                   ihp.SCDWHWHCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Primary,
                                   ObjectIsNotParent,
                                   ErrorsFound);

        // why was this here
        //        state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHWHCoilIndex).AirInletNodeNum = InNode;
        //        state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHWHCoilIndex).AirOutletNodeNum = OutNode;

        if ((state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHWHCoilNum).AirInletNodeNum != InNode) ||
            (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHWHCoilNum).AirOutletNodeNum != OutNode)) {
            ShowContinueError(state, format("Mistaken air node connection: {}:{}-wrong coil node names.", CurrentModuleObject, ihp.SHDWHWHCoilName));
            ErrorsFound = true;
        }
        SetUpCompSets(state, CurrentModuleObject, ihp.Name + " Outdoor Coil", HVAC::coilTypeNames[(int)ihp.SHDWHWHCoilType], ihp.SHDWHWHCoilName, InNodeName, OutNodeName);
        OverrideNodeConnectionType(state,
                                   InNode,
                                   InNodeName,
                                   ihp.SHDWHWHCoilTypeNum,
                                   ihp.SHDWHWHCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Primary,
                                   ObjectIsNotParent,
                                   ErrorsFound);
        OverrideNodeConnectionType(state,
                                   OutNode,
                                   OutNodeName,
                                   ihp.SHDWHWHCoilTypeNum,
                                   ihp.SHDWHWHCoilName,
                                   DataLoopNode::ConnectionType::Internal,
                                   NodeInputManager::CompFluidStream::Primary,
                                   ObjectIsNotParent,
                                   ErrorsFound);

        ihp.IHPCoilsSized = false;
        ihp.CoolVolFlowScale = 1.0; // scale coil flow rates to match the parent fan object
        ihp.HeatVolFlowScale = 1.0; // scale coil flow rates to match the parent fan object
        ihp.CurMode = IHPOperationMode::Idle;
        ihp.MaxHeatAirMassFlow = 1e10;
        ihp.MaxHeatAirVolFlow = 1e10;
        ihp.MaxCoolAirMassFlow = 1e10;
        ihp.MaxCoolAirVolFlow = 1e10;
    }

    if (ErrorsFound) {
        ShowFatalError(state,
                       format("{} Errors found in getting {} input. Preceding condition(s) causes termination.", RoutineName, CurrentModuleObject));
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

    bool ErrorsFound = false;
    Real64 RatedCapacity(0.0); // rated building cooling load

    // Obtains and Allocates AS-IHP related parameters from input file
    if (state.dataIntegratedHP->GetCoilsInputFlag) { // First time subroutine has been entered
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    };

    if (DXCoilNum > static_cast<int>(state.dataIntegratedHP->IntegratedHeatPumps.size()) || DXCoilNum < 1) {
        ShowFatalError(state,
                       format("SizeIHP: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name=AS-IHP",
                              DXCoilNum,
                              state.dataIntegratedHP->IntegratedHeatPumps.size()));
    }

    if (state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum).IHPCoilsSized) {
        return;
    }

    auto &ihp = state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum);

    // associate SC coil with SH coil
    bool errFlag = false;
    VariableSpeedCoils::SetCoilData(state, ihp.SCCoilNum, _, ihp.SHCoilNum);
    if (errFlag) {
        ShowSevereError(state, format(R"(SizeIHP: Could not match cooling coil"{}" with heating coil="{}")", ihp.SCCoilName, ihp.SHCoilName));
        ErrorsFound = true;
    };

    errFlag = false;
    VariableSpeedCoils::SizeVarSpeedCoil(state, ihp.SCCoilNum, errFlag); // size cooling coil
    if (errFlag) {
        ShowSevereError(state, format("SizeIHP: failed to size SC coil\"{}\"", ihp.SCCoilName));
        ErrorsFound = true;
    } else {
        RatedCapacity = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCCoilNum).RatedCapCoolTotal;
    };

    errFlag = false;
    VariableSpeedCoils::SizeVarSpeedCoil(state, ihp.SHCoilNum, errFlag); // size heating coil
    if (errFlag) {
        ShowSevereError(state, format("SizeIHP: failed to size SH coil\"{}\"", ihp.SHCoilName));
        ErrorsFound = true;
    };

    // pass SC coil capacity to SCDWH cool coil
    if (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHCoolCoilNum).RatedCapCoolTotal == AutoSize) {
        state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHCoolCoilNum).RatedCapCoolTotal = RatedCapacity;
    };

    // associate SCDWH air coil to SHDWH air coil
    errFlag = false;
    VariableSpeedCoils::SetCoilData(state, ihp.SCDWHCoolCoilNum, _, ihp.SHDWHHeatCoilNum);
    // size SCDWH air coil
    VariableSpeedCoils::SizeVarSpeedCoil(state, ihp.SCDWHCoolCoilNum, errFlag);
    if (errFlag) {
        ShowSevereError(state, format("SizeIHP: failed to size SCDWH cooling coil\"{}\"", ihp.SCDWHCoolCoilName));
        ErrorsFound = true;
    };

    // size SHDWH air coil
    errFlag = false;
    VariableSpeedCoils::SizeVarSpeedCoil(state, ihp.SHDWHHeatCoilNum, errFlag);
    if (errFlag) {
        ShowSevereError(state, format("SizeIHP: failed to size SHDWH heating coil\"{}\"", ihp.SHDWHHeatCoilName));
        ErrorsFound = true;
    };

    // size the water coils below
    // size SCWH water coil
    if (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilNum).RatedCapWH == AutoSize) {
        state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilNum).RatedCapWH =
            RatedCapacity / (1.0 - 1.0 / state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilNum).RatedCOPHeat);
    }

    errFlag = false;
    VariableSpeedCoils::SizeVarSpeedCoil(state, ihp.SCWHCoilNum, errFlag);
    if (errFlag) {
        ShowSevereError(state, format("SizeIHP: failed to size SCWH coil\"{}\"", ihp.SCWHCoilName));
        ErrorsFound = true;
    };

    // size DWH water coil
    if (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.DWHCoilNum).RatedCapWH == AutoSize) {
        state.dataVariableSpeedCoils->VarSpeedCoil(ihp.DWHCoilNum).RatedCapWH = RatedCapacity;
    }

    errFlag = false;
    VariableSpeedCoils::SizeVarSpeedCoil(state, ihp.DWHCoilNum, errFlag);
    if (errFlag) {
        ShowSevereError(state, format("SizeIHP: failed to size DWH coil\"{}\"", ihp.DWHCoilName));
        ErrorsFound = true;
    };

    // size SCDWH water coil
    if (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHWHCoilNum).RatedCapWH == AutoSize) {
        state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHWHCoilNum).RatedCapWH = RatedCapacity * 0.13;
    }

    errFlag = false;
    VariableSpeedCoils::SizeVarSpeedCoil(state, ihp.SCDWHWHCoilNum, errFlag);
    if (errFlag) {
        ShowSevereError(state, format("SizeIHP: failed to size SCDWH water heating coil\"{}\"", ihp.SCDWHWHCoilName));
        ErrorsFound = true;
    };

    // size SHDWH water coil
    if (state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHWHCoilNum).RatedCapWH == AutoSize) {
        state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHWHCoilNum).RatedCapWH = RatedCapacity * 0.1;
    }

    errFlag = false;
    VariableSpeedCoils::SizeVarSpeedCoil(state, ihp.SHDWHWHCoilNum, errFlag);
    if (errFlag) {
        ShowSevereError(state, format("SizeIHP: failed to size SHDWH water heating coil\"{}\"", ihp.SHDWHWHCoilName));
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
                       format("InitializeIHP: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name=AS-IHP",
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
                       format("UpdateIHP: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name=AS-IHP",
                              DXCoilNum,
                              state.dataIntegratedHP->IntegratedHeatPumps.size()));
    }

    auto &ihp = state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum);

    switch (ihp.CurMode) {
    case IHPOperationMode::SpaceClg:
        ihp.TotalCoolingRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCCoilNum).QLoadTotal; // total cooling rate [w]
        ihp.TotalWaterHeatingRate = 0.0;                                                               // total water heating rate [w]
        ihp.TotalSpaceHeatingRate = 0.0;                                                               // total space heating rate [w]
        ihp.TotalPower = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCCoilNum).Power;            // total power consumption  [w]
        ihp.TotalLatentLoad = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCCoilNum).QLatent;     // total latent cooling rate [w]
        ihp.Qsource = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCCoilNum).QSource;             // source energy rate, [w]
        break;
    case IHPOperationMode::SpaceHtg:
        ihp.TotalCoolingRate = 0.0;                                                                         // total cooling rate [w]
        ihp.TotalWaterHeatingRate = 0.0;                                                                    // total water heating rate [w]
        ihp.TotalSpaceHeatingRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHCoilNum).QLoadTotal; // total space heating rate [w]
        ihp.TotalPower = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHCoilNum).Power;                 // total power consumption  [w]
        ihp.TotalLatentLoad = 0.0;                                                                          // total latent cooling rate [w]
        ihp.Qsource = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHCoilNum).QSource;                  // source energy rate, [w]
        break;
    case IHPOperationMode::DedicatedWaterHtg:
        ihp.TotalCoolingRate = 0.0;                                                                       // total cooling rate [w]
        ihp.TotalWaterHeatingRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.DWHCoilNum).QSource; // total water heating rate [w]
        ihp.TotalSpaceHeatingRate = 0.0;                                                                  // total space heating rate [w]
        ihp.TotalPower = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.DWHCoilNum).Power;              // total power consumption  [w]
        ihp.TotalLatentLoad = 0.0;                                                                        // total latent cooling rate [w]
        ihp.Qsource = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.DWHCoilNum).QLoadTotal;            // source energy rate, [w]
        break;
    case IHPOperationMode::SCWHMatchSC:
    case IHPOperationMode::SCWHMatchWH:
        ihp.TotalCoolingRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilNum).QLoadTotal;   // total cooling rate [w]
        ihp.TotalWaterHeatingRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilNum).QSource; // total water heating rate [w]
        ihp.TotalSpaceHeatingRate = 0.0;                                                                   // total space heating rate [w]
        ihp.TotalPower = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilNum).Power;              // total power consumption  [w]
        ihp.TotalLatentLoad = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilNum).QLatent;       // total latent cooling rate [w]
        ihp.Qsource = 0.0;                                                                                 // source energy rate, [w]
        break;
    case IHPOperationMode::SpaceClgDedicatedWaterHtg:
        ihp.TotalCoolingRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHCoolCoilNum).QLoadTotal; // total cooling rate [w]
        ihp.TotalSpaceHeatingRate = 0.0;                                                                      // total space heating rate [w]
        ihp.TotalPower = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHCoolCoilNum).Power;            // total power consumption  [w]
        ihp.TotalLatentLoad = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHCoolCoilNum).QLatent;     // total latent cooling rate [w]
        ihp.Qsource = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHCoolCoilNum).QSource;             // source energy rate, [w]

        ihp.TotalWaterHeatingRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHWHCoilNum).QSource; // total water heating rate [w]

        break;
    case IHPOperationMode::SHDWHElecHeatOff:
    case IHPOperationMode::SHDWHElecHeatOn:
        ihp.TotalCoolingRate = 0.0;                                                                                // total cooling rate [w]
        ihp.TotalSpaceHeatingRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHHeatCoilNum).QLoadTotal; // total space heating rate [w]
        ihp.TotalPower = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHHeatCoilNum).Power;                 // total power consumption  [w]
        ihp.TotalLatentLoad = 0.0;                                                                                 // total latent cooling rate [w]
        ihp.Qsource = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHHeatCoilNum).QSource;                  // source energy rate, [w]

        ihp.TotalWaterHeatingRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHWHCoilNum).QSource; // total water heating rate [w]

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
                       format("DecideWorkMode: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name=AS-IHP",
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
    // Obtains and Allocates WatertoAirHP related parameters from input file
    if (state.dataIntegratedHP->GetCoilsInputFlag) { // First time subroutine has been entered
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    }

    if (DXCoilNum > static_cast<int>(state.dataIntegratedHP->IntegratedHeatPumps.size()) || DXCoilNum < 1) {
        ShowFatalError(state,
                       format("ClearCoils: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name=AS-IHP",
                              DXCoilNum,
                              state.dataIntegratedHP->IntegratedHeatPumps.size()));
    }

    auto &ihp = state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum);

    // clear up
    VariableSpeedCoils::SimVariableSpeedCoils(state,  ihp.SCDWHCoolCoilNum, HVAC::FanOp::Cycling, HVAC::CompressorOp::On, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    VariableSpeedCoils::SimVariableSpeedCoils(state,  ihp.SCDWHWHCoilNum, HVAC::FanOp::Cycling, HVAC::CompressorOp::On, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    VariableSpeedCoils::SimVariableSpeedCoils(state,  ihp.SHDWHHeatCoilNum, HVAC::FanOp::Cycling, HVAC::CompressorOp::On, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    VariableSpeedCoils::SimVariableSpeedCoils(state,  ihp.SHDWHWHCoilNum, HVAC::FanOp::Cycling, HVAC::CompressorOp::On, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    VariableSpeedCoils::SimVariableSpeedCoils(state,  ihp.SCWHCoilNum, HVAC::FanOp::Cycling, HVAC::CompressorOp::On, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    VariableSpeedCoils::SimVariableSpeedCoils(state,  ihp.SCCoilNum, HVAC::FanOp::Cycling, HVAC::CompressorOp::On, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    VariableSpeedCoils::SimVariableSpeedCoils(state,  ihp.SHCoilNum, HVAC::FanOp::Cycling, HVAC::CompressorOp::On, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
    VariableSpeedCoils::SimVariableSpeedCoils(state,  ihp.DWHCoilNum, HVAC::FanOp::Cycling, HVAC::CompressorOp::On, 0.0, 1.0, 0.0, 0.0, 0.0, 1.0);
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
                       format("GetCurWorkMode: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name=AS-IHP",
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
        ShowSevereError(state, format(R"(GetCoilIndexIHP: Could not find CoilType="{}" with Name="{}")", CoilType, CoilName));
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
        ShowSevereError(state, format(R"(GetCoilInletNodeIHP: Could not find CoilType="{}" with Name="{}")", CoilType, CoilName));
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
        ShowSevereError(state, format(R"(GetCoilInletNodeIHP: Could not find CoilType="{}" with Name="{}")", CoilType, CoilName));
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
        ShowSevereError(state, format(R"(GetCoilInletNodeIHP: Could not find CoilType="{}" with Name="{}")", CoilType, CoilName));
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

    // Obtains and Allocates WatertoAirHP related parameters from input file
    if (state.dataIntegratedHP->GetCoilsInputFlag) { // First time subroutine has been entered
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    }

    int WhichCoil = Util::FindItemInList(CoilName, state.dataIntegratedHP->IntegratedHeatPumps);
    if (WhichCoil == 0) {
        ShowSevereError(state, format(R"(GetIHPDWHCoilPLFFPLR: Could not find CoilType="{}" with Name="{}")", CoilType, CoilName));
        ErrorsFound = true;
        return 0;
    }
    
    auto &ihp = state.dataIntegratedHP->IntegratedHeatPumps(WhichCoil);

    // this will be called by HPWH parent
    return (ihp.DWHCoilNum > 0) ? 
            VariableSpeedCoils::GetCoilPLFFPLR(state, ihp.DWHCoilNum) : VariableSpeedCoils::GetCoilPLFFPLR(state, ihp.SCWHCoilNum);
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

        if (ihp.DWHCoilNum > 0) {
            CoilCapacity = VariableSpeedCoils::GetCoilCapacity(state, ihp.DWHCoilNum);
        } else {
            CoilCapacity = VariableSpeedCoils::GetCoilCapacity(state, ihp.SCWHCoilNum);
        }
    } else {
        WhichCoil = 0;
    }

    if (WhichCoil == 0) {
        ShowSevereError(state, format(R"(GetCoilCapacityVariableSpeed: Could not find CoilType="{}" with Name="{}")", CoilType, CoilName));
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
                       format("GetLowSpeedNumIHP: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name=AS-IHP",
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
                       format("GetMaxSpeedNumIHP: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name=AS-IHP",
                              DXCoilNum,
                              state.dataIntegratedHP->IntegratedHeatPumps.size()));
    }

    int SpeedNum(0);
    auto &ihp = state.dataIntegratedHP->IntegratedHeatPumps(DXCoilNum);

    switch (ihp.CurMode) {
    case IHPOperationMode::Idle:
    case IHPOperationMode::SpaceClg:
        SpeedNum = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCCoilNum).NumOfSpeeds;
        break;
    case IHPOperationMode::SpaceHtg:
        SpeedNum = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHCoilNum).NumOfSpeeds;
        break;
    case IHPOperationMode::DedicatedWaterHtg:
        SpeedNum = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.DWHCoilNum).NumOfSpeeds;
        break;
    case IHPOperationMode::SCWHMatchSC:
    case IHPOperationMode::SCWHMatchWH:
        SpeedNum = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilNum).NumOfSpeeds;
        break;
    case IHPOperationMode::SpaceClgDedicatedWaterHtg:
        SpeedNum = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHCoolCoilNum).NumOfSpeeds;
        break;
    case IHPOperationMode::SHDWHElecHeatOff:
    case IHPOperationMode::SHDWHElecHeatOn:
        SpeedNum = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHHeatCoilNum).NumOfSpeeds;
        break;
    default:
        SpeedNum = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCCoilNum).NumOfSpeeds;
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
                       format("GetAirVolFlowRateIHP: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name=AS-IHP",
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
        IHPCoilIndex = ihp.SCCoilNum;
        break;
    case IHPOperationMode::SpaceClg:
        IHPCoilIndex = ihp.SCCoilNum;
        if (!IsCallbyWH) // call from air loop
        {
            FlowScale = ihp.CoolVolFlowScale;
        }

        break;
    case IHPOperationMode::SpaceHtg:
        IHPCoilIndex = ihp.SHCoilNum;
        if (!IsCallbyWH) // call from air loop
        {
            FlowScale = ihp.HeatVolFlowScale;
        }
        break;
    case IHPOperationMode::DedicatedWaterHtg:
        IHPCoilIndex = ihp.DWHCoilNum;
        FlowScale = 1.0;
        break;
    case IHPOperationMode::SCWHMatchSC:
        IHPCoilIndex = ihp.SCWHCoilNum;
        FlowScale = ihp.CoolVolFlowScale;
        if (IsCallbyWH) // call from water loop
        {
            IsResultFlow = true;
            AirVolFlowRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilNum).AirVolFlowRate;
        }
        break;
    case IHPOperationMode::SCWHMatchWH:
        IHPCoilIndex = ihp.SCWHCoilNum;
        FlowScale = ihp.CoolVolFlowScale;
        if (!IsCallbyWH) {
            IsResultFlow = true;
            AirVolFlowRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCWHCoilNum).AirVolFlowRate;
        }
        break;
    case IHPOperationMode::SpaceClgDedicatedWaterHtg:
        IHPCoilIndex = ihp.SCDWHCoolCoilNum;
        FlowScale = ihp.CoolVolFlowScale;
        if (IsCallbyWH) {
            IsResultFlow = true;
            AirVolFlowRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SCDWHCoolCoilNum).AirVolFlowRate;
        }
        break;
    case IHPOperationMode::SHDWHElecHeatOff:
    case IHPOperationMode::SHDWHElecHeatOn:
        IHPCoilIndex = ihp.SHDWHHeatCoilNum;
        FlowScale = ihp.HeatVolFlowScale;
        if (IsCallbyWH) {
            IsResultFlow = true;
            AirVolFlowRate = state.dataVariableSpeedCoils->VarSpeedCoil(ihp.SHDWHHeatCoilNum).AirVolFlowRate;
        }
        break;
    default:
        IHPCoilIndex = ihp.SCCoilNum;
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
                       format("GetWaterVolFlowRateIHP: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name=AS-IHP",
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
    case IHPOperationMode::SpaceHtg: {
        WaterVolFlowRate = 0.0;
    } break;

    case IHPOperationMode::DedicatedWaterHtg: {
        IHPCoilIndex = ihp.DWHCoilNum;
        if (1 == SpeedNum) {
            WaterVolFlowRate = state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum);
        } else {
            WaterVolFlowRate = SpeedRatio * state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum) +
                               (1.0 - SpeedRatio) * state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum - 1);
        }
    } break;

    case IHPOperationMode::SCWHMatchSC:
    case IHPOperationMode::SCWHMatchWH: {
        IHPCoilIndex = ihp.SCWHCoilNum;
        if (1 == SpeedNum) {
            WaterVolFlowRate = state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum);
        } else {
            WaterVolFlowRate = SpeedRatio * state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum) +
                               (1.0 - SpeedRatio) * state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum - 1);
        }
    } break;
    
    case IHPOperationMode::SpaceClgDedicatedWaterHtg: {
        IHPCoilIndex = ihp.SCDWHWHCoilNum;
        if (1 == SpeedNum) {
            WaterVolFlowRate = state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum);
        } else {
            WaterVolFlowRate = SpeedRatio * state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum) +
                               (1.0 - SpeedRatio) * state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum - 1);
        }
    } break;
    case IHPOperationMode::SHDWHElecHeatOff:
    case IHPOperationMode::SHDWHElecHeatOn: {
        IHPCoilIndex = ihp.SHDWHWHCoilNum;
        if (1 == SpeedNum) {
            WaterVolFlowRate = state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum);
        } else {
            WaterVolFlowRate = SpeedRatio * state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum) +
                               (1.0 - SpeedRatio) * state.dataVariableSpeedCoils->VarSpeedCoil(IHPCoilIndex).MSRatedWaterVolFlowRate(SpeedNum - 1);
        }
    } break;
    default: {
        WaterVolFlowRate = 0.0;
    } break;
    } // end switch

    return WaterVolFlowRate;
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
                       format("GetAirMassFlowRateIHP: Invalid CompIndex passed={}, Number of Integrated HPs={}, IHP name=AS-IHP",
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
        IHPCoilIndex = ihp.SCCoilNum;
        AirMassFlowRate = 0.0;
        break;
    case IHPOperationMode::SpaceClg:
        IHPCoilIndex = ihp.SCCoilNum;
        if (!IsCallbyWH) {
            FlowScale = ihp.CoolVolFlowScale;
        } else {
            IsResultFlow = true;
            AirMassFlowRate = ihp.AirFlowSavInAirLoop;
        }
        break;
    case IHPOperationMode::SpaceHtg:
        IHPCoilIndex = ihp.SHCoilNum;
        if (!IsCallbyWH) {
            FlowScale = ihp.HeatVolFlowScale;
        } else {
            IsResultFlow = true;
            AirMassFlowRate = ihp.AirFlowSavInAirLoop;
        }
        break;
    case IHPOperationMode::DedicatedWaterHtg:
        IHPCoilIndex = ihp.DWHCoilNum;
        FlowScale = 1.0;
        break;
    case IHPOperationMode::SCWHMatchSC:
        IHPCoilIndex = ihp.SCWHCoilNum;
        FlowScale = ihp.CoolVolFlowScale;
        state.dataLoopNodes->Node(ihp.WaterInletNodeNum).MassFlowRate = GetWaterVolFlowRateIHP(state, DXCoilNum, SpeedNum, SpeedRatio) * WaterDensity;
        if (IsCallbyWH) {
            IsResultFlow = true;
            AirMassFlowRate = ihp.AirFlowSavInAirLoop;
        }
        break;
    case IHPOperationMode::SCWHMatchWH:
        IHPCoilIndex = ihp.SCWHCoilNum;
        FlowScale = ihp.CoolVolFlowScale;
        if (!IsCallbyWH) {
            IsResultFlow = true;
            AirMassFlowRate = ihp.AirFlowSavInWaterLoop;
        }
        break;
    case IHPOperationMode::SpaceClgDedicatedWaterHtg:
        IHPCoilIndex = ihp.SCDWHCoolCoilNum;
        FlowScale = ihp.CoolVolFlowScale;
        state.dataLoopNodes->Node(ihp.WaterInletNodeNum).MassFlowRate = GetWaterVolFlowRateIHP(state, DXCoilNum, SpeedNum, SpeedRatio) * WaterDensity;
        if (IsCallbyWH) {
            IsResultFlow = true;
            AirMassFlowRate = ihp.AirFlowSavInAirLoop;
        }
        break;
    case IHPOperationMode::SHDWHElecHeatOff:
    case IHPOperationMode::SHDWHElecHeatOn:
        IHPCoilIndex = ihp.SHDWHHeatCoilNum;
        FlowScale = ihp.HeatVolFlowScale;
        state.dataLoopNodes->Node(ihp.WaterInletNodeNum).MassFlowRate = GetWaterVolFlowRateIHP(state, DXCoilNum, SpeedNum, SpeedRatio) * WaterDensity;
        if (IsCallbyWH) {
            IsResultFlow = true;
            AirMassFlowRate = ihp.AirFlowSavInAirLoop;
        }
        break;
    default:
        IHPCoilIndex = ihp.SCCoilNum;
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

int GetIHPIndex(EnergyPlusData &state, std::string const &ihpName)
{
    if (state.dataIntegratedHP->GetCoilsInputFlag) { // First time subroutine has been entered
        GetIHPInput(state);
        state.dataIntegratedHP->GetCoilsInputFlag = false;
    }

    return Util::FindItemInList(ihpName, state.dataIntegratedHP->IntegratedHeatPumps);
}
  
int GetIHPDWHCoilPLFFPLR(EnergyPlusData &state, int const ihpNum)
{
    assert(ihpNum > 0 && ihpNum <= (int)state.dataIntegratedHP->IntegratedHeatPumps.size());
    auto &ihp = state.dataIntegratedHP->IntegratedHeatPumps(ihpNum);
    return (ihp.DWHCoilNum > 0) ?
        VariableSpeedCoils::GetCoilPLFFPLR(state, ihp.DWHCoilNum) : VariableSpeedCoils::GetCoilPLFFPLR(state, ihp.SCWHCoilNum);
}

Real64 GetIHPDWHCoilCapacity(EnergyPlusData &state, int const ihpNum)
{
    assert(ihpNum > 0 && ihpNum <= (int)state.dataIntegratedHP->IntegratedHeatPumps.size());
    auto &ihp = state.dataIntegratedHP->IntegratedHeatPumps(ihpNum);
    if (ihp.IHPCoilsSized == false) SizeIHP(state, ihpNum); // WHY?
    return (ihp.DWHCoilNum > 0) ?
        VariableSpeedCoils::GetCoilCapacity(state, ihp.DWHCoilNum) : VariableSpeedCoils::GetCoilCapacity(state, ihp.SCWHCoilNum);
}

int GetIHPCoilAirInletNode(EnergyPlusData &state, int const ihpNum)
{
    assert(ihpNum > 0 && ihpNum <= (int)state.dataIntegratedHP->IntegratedHeatPumps.size());
    return state.dataIntegratedHP->IntegratedHeatPumps(ihpNum).AirCoolInletNodeNum;
}

int GetIHPDWHCoilAirInletNode(EnergyPlusData &state, int const ihpNum)
{
    assert(ihpNum > 0 && ihpNum <= (int)state.dataIntegratedHP->IntegratedHeatPumps.size());
    return state.dataIntegratedHP->IntegratedHeatPumps(ihpNum).ODAirInletNodeNum;
}

int GetIHPDWHCoilAirOutletNode(EnergyPlusData &state, int const ihpNum)
{
    assert(ihpNum > 0 && ihpNum <= (int)state.dataIntegratedHP->IntegratedHeatPumps.size());
    return state.dataIntegratedHP->IntegratedHeatPumps(ihpNum).ODAirOutletNodeNum;
}

} // namespace EnergyPlus::IntegratedHeatPump

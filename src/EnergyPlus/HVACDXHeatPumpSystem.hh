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

#ifndef HVACDXHeatPumpSystem_hh_INCLUDED
#define HVACDXHeatPumpSystem_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace HVACDXHeatPumpSystem {

    // MODULE PARAMETER DEFINITIONS
    constexpr Real64 MinAirMassFlow(0.001);

    // Types
    struct DXHeatPumpSystemStruct
    {
        // Members
        std::string DXHeatPumpSystemType; // Type of DXHeatingSystem
        std::string Name;                 // Name of the DXHeatingSystem
        Sched::Schedule *availSched = nullptr;
        HVAC::CoilType coilType = HVAC::CoilType::Invalid;
        std::string HeatPumpCoilName;
        int HeatPumpCoilIndex;
        int DXHeatPumpCoilInletNodeNum;
        int DXHeatPumpCoilOutletNodeNum;
        int DXSystemControlNodeNum; // the node number of the node with the set point
        Real64 DesiredOutletTemp;   // the temperature at the unit outlet node needed
        // to meet the supply air set point.
        Real64 PartLoadFrac;                      // part load fraction for current time step (single speed)
        Real64 SpeedRatio;                        // current compressor speed ratio (variable speed)
        Real64 CycRatio;                          // cycling part load ratio (variable speed)
        HVAC::FanOp fanOp = HVAC::FanOp::Invalid; // Fan operating mode (see parameter above)
        // Warning message variables
        int DXCoilSensPLRIter;      // used in DXCoil calculations
        int DXCoilSensPLRIterIndex; // used in DXCoil calculations
        int DXCoilSensPLRFail;      // used in DXCoil calculations
        int DXCoilSensPLRFailIndex; // used in DXCoil calculations
        // When the Dx system is a part of Outdoor Air Unit
        Real64 OAUnitSetTemp; // set
        // variable-speed coil
        int SpeedNum; // select speed number for variable-speed coil
        // Fault model of coil SAT sensor
        bool FaultyCoilSATFlag;     // True if the coil has SAT sensor fault
        int FaultyCoilSATIndex;     // Index of the fault object corresponding to the coil
        Real64 FaultyCoilSATOffset; // Coil SAT sensor offset

        // Default Constructor
        DXHeatPumpSystemStruct()
            : HeatPumpCoilIndex(0), DXHeatPumpCoilInletNodeNum(0), DXHeatPumpCoilOutletNodeNum(0), DXSystemControlNodeNum(0), DesiredOutletTemp(0.0),
              PartLoadFrac(0.0), SpeedRatio(0.0), CycRatio(0.0), DXCoilSensPLRIter(0), DXCoilSensPLRIterIndex(0), DXCoilSensPLRFail(0),
              DXCoilSensPLRFailIndex(0), OAUnitSetTemp(0.0), SpeedNum(0), FaultyCoilSATFlag(false), FaultyCoilSATIndex(0), FaultyCoilSATOffset(0.0)
        {
        }
    };

    void SimDXHeatPumpSystem(EnergyPlusData &state,
                             std::string_view DXHeatPumpSystemName,                // Name of DXSystem:Airloop object
                             bool const FirstHVACIteration,                        // True when first HVAC iteration
                             int const AirLoopNum,                                 // Primary air loop number
                             int &CompIndex,                                       // Index to CoilSystem:Heating:DX object
                             ObjexxFCL::Optional_int_const OAUnitNum = _,          // If the system is an equipment of OutdoorAirUnit
                             ObjexxFCL::Optional<Real64 const> OAUCoilOutTemp = _, // the coil inlet temperature of OutdoorAirUnit
                             ObjexxFCL::Optional<Real64> QTotOut = _               // the total cooling output of unit
    );

    // Get Input Section of the Module
    //******************************************************************************

    void GetDXHeatPumpSystemInput(EnergyPlusData &state);

    // End of Get Input subroutines for the Module
    //******************************************************************************

    // Beginning of Initialization subroutines for the Module
    // *****************************************************************************

    void InitDXHeatPumpSystem(EnergyPlusData &state,
                              int const DXSystemNum,                               // number of the current DX Sys being simulated
                              int const AirLoopNum,                                // number of the current air loop being simulated
                              ObjexxFCL::Optional_int_const OAUnitNum = _,         // number of the current outdoor air unit being simulated
                              ObjexxFCL::Optional<Real64 const> OAUCoilOutTemp = _ // the coil inlet temperature of OutdoorAirUnit
    );

    // End of Initialization subroutines for the Module
    // *****************************************************************************

    // Beginning of Calculation subroutines for the DXCoolingSystem Module
    // *****************************************************************************

    void ControlDXHeatingSystem(EnergyPlusData &state,
                                int const DXSystemNum,        // index to DXSystem
                                bool const FirstHVACIteration // First HVAC iteration flag
    );

    Real64 VSCoilCyclingResidual(EnergyPlusData &state,
                                 Real64 PartLoadRatio, // compressor cycling ratio (1.0 is continuous, 0.0 is off)
                                 int CoilIndex,
                                 Real64 desiredTemp,
                                 HVAC::FanOp const fanOp);

    //******************************************************************************

    Real64 VSCoilSpeedResidual(EnergyPlusData &state,
                               Real64 SpeedRatio, // compressor cycling ratio (1.0 is continuous, 0.0 is off)
                               int CoilIndex,
                               Real64 desiredTemp,
                               int speedNumber,
                               HVAC::FanOp const fanOp);

    int GetHeatingCoilInletNodeNum(EnergyPlusData &state, std::string const &DXCoilSysName, bool &InletNodeErrFlag);

    int GetHeatingCoilOutletNodeNum(EnergyPlusData &state, std::string const &DXCoilSysName, bool &OutletNodeErrFlag);

} // namespace HVACDXHeatPumpSystem

struct HVACDXHeatPumpSystemData : BaseGlobalStruct
{

    int NumDXHeatPumpSystems = 0; // The Number of DXHeatPumpSystems found in the Input
    bool EconomizerFlag = false;  // holds air loop economizer status
    bool GetInputFlag = true;     // Flag to get input only once
    Array1D_bool CheckEquipName;
    Array1D<HVACDXHeatPumpSystem::DXHeatPumpSystemStruct> DXHeatPumpSystem;

    Real64 QZnReq = 0.001;          // Zone load (W), input to variable-speed DX coil
    Real64 QLatReq = 0.0;           // Zone latent load, input to variable-speed DX coil
    Real64 OnOffAirFlowRatio = 1.0; // ratio of compressor on flow to average flow over time step
    bool ErrorsFound = false;       // If errors detected in input
    int TotalArgs = 0;              // Total number of alpha and numeric arguments (max) for a certain object in the input file
    bool MySetPointCheckFlag = true;
    int SpeedNum = 1;                  // speed number of variable speed DX cooling coil
    Real64 QZnReqr = 0.001;            // Zone load (W), input to variable-speed DX coil
    Real64 QLatReqr = 0.0;             // Zone latent load, input to variable-speed DX coil
    Real64 OnandOffAirFlowRatio = 1.0; // ratio of compressor on flow to average flow over time step
    Real64 SpeedRatio = 0.0;           // SpeedRatio varies between 1.0 (higher speed) and 0.0 (lower speed)
    int SpeedNumber = 1;               // speed number of variable speed DX cooling coil
    Real64 QZoneReq = 0.001;           // Zone load (W), input to variable-speed DX coil
    Real64 QLatentReq = 0.0;           // Zone latent load, input to variable-speed DX coil
    Real64 AirFlowOnOffRatio = 1.0;    // ratio of compressor on flow to average flow over time step
    Real64 SpeedPartLoadRatio = 1.0;   // SpeedRatio varies between 1.0 (higher speed) and 0.0 (lower speed)

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->GetInputFlag = true;
        this->NumDXHeatPumpSystems = 0;
        this->EconomizerFlag = false;
        this->CheckEquipName.deallocate();
        this->DXHeatPumpSystem.deallocate();
        this->QZnReq = 0.001;
        this->QLatReq = 0.0;
        this->OnOffAirFlowRatio = 1.0;
        this->ErrorsFound = false;
        this->TotalArgs = 0;
        this->MySetPointCheckFlag = true;
        this->SpeedNum = 1;
        this->QZnReq = 0.001;
        this->QLatReq = 0.0;
        this->OnOffAirFlowRatio = 1.0;
        this->SpeedRatio = 0.0;
        this->SpeedNumber = 1;
        this->QZoneReq = 0.001;
        this->QLatentReq = 0.0;
        this->AirFlowOnOffRatio = 1.0;
        this->SpeedPartLoadRatio = 1.0;
    }
};

} // namespace EnergyPlus

#endif

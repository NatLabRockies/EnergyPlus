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

#ifndef IntegratedHeatPump_hh_INCLUDED
#define IntegratedHeatPump_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.fwd.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/DataLoopNode.hh>
#include <EnergyPlus/EPVector.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/Enums.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace IntegratedHeatPump {

    // operation mode
    enum class IHPOperationMode : int
    {
        Invalid = -1,
        Idle,
        SpaceClg,          // Space Cooling
        SpaceHtg,          // Space Heating
        DedicatedWaterHtg, // Dedicated Water Heating
        SCWHMatchSC,       // Space Cooling Water Heating
        SCWHMatchWH,
        SpaceClgDedicatedWaterHtg,
        SHDWHElecHeatOff,
        SHDWHElecHeatOn,
        Num
    };

    struct IntegratedHeatPumpData // variable speed coil
    {
        // Members
        std::string Name;    // Name of the  Coil
        std::string IHPtype; // type of coil

        std::string SCCoilType; // Numeric Equivalent for SC Coil Type
        std::string SCCoilName;
        int SCCoilIndex; // Index to SC coil
        DataLoopNode::ConnectionObjectType SCCoilTypeNum = DataLoopNode::ConnectionObjectType::Invalid;

        std::string SHCoilType; // Numeric Equivalent for SH Coil Type
        std::string SHCoilName;
        int SHCoilIndex; // Index to SH coil
        DataLoopNode::ConnectionObjectType SHCoilTypeNum = DataLoopNode::ConnectionObjectType::Invalid;

        std::string SCWHCoilType; // Numeric Equivalent for SCWH Coil Type
        std::string SCWHCoilName;
        int SCWHCoilIndex; // Index to SCWH coil
        DataLoopNode::ConnectionObjectType SCWHCoilTypeNum = DataLoopNode::ConnectionObjectType::Invalid;

        std::string DWHCoilType; // Numeric Equivalent for DWH Coil Type
        std::string DWHCoilName;
        int DWHCoilIndex; // Index to DWH coil
        DataLoopNode::ConnectionObjectType DWHCoilTypeNum = DataLoopNode::ConnectionObjectType::Invalid;

        std::string SCDWHCoolCoilType; // Numeric Equivalent for SCDWH Coil Type, cooling part
        std::string SCDWHCoolCoilName;
        int SCDWHCoolCoilIndex; // Index to SCDWH coil, cooling part
        DataLoopNode::ConnectionObjectType SCDWHCoolCoilTypeNum = DataLoopNode::ConnectionObjectType::Invalid;

        std::string SCDWHWHCoilType; // Numeric Equivalent for SCDWH Coil Type, water heating part
        std::string SCDWHWHCoilName;
        int SCDWHWHCoilIndex; // Index to SCDWH coil, water heating part
        DataLoopNode::ConnectionObjectType SCDWHWHCoilTypeNum = DataLoopNode::ConnectionObjectType::Invalid;

        std::string SHDWHHeatCoilType; // Numeric Equivalent for SHDWH Coil Type, heating part
        std::string SHDWHHeatCoilName;
        int SHDWHHeatCoilIndex; // Index to SHDWH coil, heating part
        DataLoopNode::ConnectionObjectType SHDWHHeatCoilTypeNum = DataLoopNode::ConnectionObjectType::Invalid;

        std::string SHDWHWHCoilType; // Numeric Equivalent for SHDWH Coil Type, water heating part
        std::string SHDWHWHCoilName;
        int SHDWHWHCoilIndex; // Index to SHDWH coil, water heating part
        DataLoopNode::ConnectionObjectType SHDWHWHCoilTypeNum = DataLoopNode::ConnectionObjectType::Invalid;

        int AirCoolInletNodeNum; // Node Number of the Air cooling coil Inlet
        int AirHeatInletNodeNum; // Node Number of the Air cooling coil Inlet
        int AirOutletNodeNum;    // Node Number of the Air Outlet
        int WaterInletNodeNum;   // Node Number of the Water Onlet
        int WaterOutletNodeNum;  // Node Number of the Water Outlet
        int WaterTankoutNod;     // water node to monitor the supply water flow amount

        int ModeMatchSCWH;
        //- 0: match cooling load, 1 : match water heating load in SCWH mode
        int MinSpedSCWH;  //-minimum speed level for SCWH mode
        int MinSpedSCDWH; //- minimum speed level for SCDWH mode
        int MinSpedSHDWH;
        //- minimum speed level for SHDWH mode
        Real64 TindoorOverCoolAllow;   //- [C], indoor temperature above which indoor overcooling is allowed
        Real64 TambientOverCoolAllow;  //- [C], ambient temperature above which indoor overcooling is allowed
        Real64 TindoorWHHighPriority;  //- [C], indoor temperature above which water heating has the higher priority
        Real64 TambientWHHighPriority; // ambient temperature above which water heating has the higher priority

        Real64 WaterVolSCDWH;
        // limit of water volume before switching from SCDWH to SCWH
        Real64 TimeLimitSHDWH; // time limit before turning from SHDWH to electric heating

        DataPlant::PlantEquipmentType WHtankType;
        std::string WHtankName;
        int WHtankID;
        int LoopNum;
        int LoopSideNum;
        bool IsWHCallAvail;
        // whether water heating call available
        bool CheckWHCall;
        IHPOperationMode CurMode; // current working mode
        Real64 ControlledZoneTemp;
        Real64 WaterFlowAccumVol;
        // water flow accumulated volume
        Real64 SHDWHRunTime;
        Real64 CoolVolFlowScale;
        // max fan cooling volumetric flow rate
        Real64 HeatVolFlowScale;
        // max fan heating volumetric flow rate
        Real64 MaxHeatAirMassFlow;
        // maximum air mass flow rate for heating mode
        Real64 MaxHeatAirVolFlow;
        // maximum air volume flow rate for heating mode
        Real64 MaxCoolAirMassFlow;
        // maximum air mass flow rate for heating mode
        Real64 MaxCoolAirVolFlow;
        // maximum air volume flow rate for heating mode
        bool IHPCoilsSized; // whether IHP coils have been sized

        std::string IDFanName;
        // IHP indoor fan name
        int IDFanID;
        // IHP indoor fan index
        HVAC::FanPlace fanPlace; // indoor fan placement

        int ODAirInletNodeNum;  // oudoor coil inlet Nod
        int ODAirOutletNodeNum; // oudoor coil outlet Nod
        Real64 TankSourceWaterMassFlowRate;
        // tank source water flow rate
        Real64 AirFlowSavInWaterLoop; // air flow saving for SCWH mode
        Real64 AirFlowSavInAirLoop;   // air flow saving for SCWH mode

        // new output variables
        Real64 AirLoopFlowRate;
        // air loop mass flow rate [kg/s]
        Real64 TotalCoolingRate;
        // total cooling rate [w]
        Real64 TotalWaterHeatingRate;
        // total water heating rate [w]
        Real64 TotalSpaceHeatingRate;
        // total space heating rate [w]
        Real64 TotalPower;
        // total power consumption  [w]
        Real64 TotalLatentLoad;
        // total latent cooling rate [w]
        Real64 Qsource;
        // source energy rate, [w]
        Real64 Energy;
        // total electric energy consumption [J]
        Real64 EnergyLoadTotalCooling;
        // total cooling energy [J]
        Real64 EnergyLoadTotalHeating;
        // total heating energy [J]
        Real64 EnergyLoadTotalWaterHeating;
        // total heating energy [J]
        Real64 EnergyLatent; // total latent energy [J]
        Real64 EnergySource;
        // total source energy
        Real64 TotalCOP; // total COP

        // Default Constructor
        IntegratedHeatPumpData()
            : SCCoilIndex(0), SHCoilIndex(0), SCWHCoilIndex(0), DWHCoilIndex(0), SCDWHCoolCoilIndex(0), SCDWHWHCoilIndex(0), SHDWHHeatCoilIndex(0),
              SHDWHWHCoilIndex(0), AirCoolInletNodeNum(0), AirHeatInletNodeNum(0), AirOutletNodeNum(0), WaterInletNodeNum(0), WaterOutletNodeNum(0),
              WaterTankoutNod(0), ModeMatchSCWH(0), MinSpedSCWH(1), MinSpedSCDWH(1), MinSpedSHDWH(1), TindoorOverCoolAllow(0.0),
              TambientOverCoolAllow(0.0), TindoorWHHighPriority(0.0), TambientWHHighPriority(0.0), WaterVolSCDWH(0.0), TimeLimitSHDWH(0.0),
              WHtankType(DataPlant::PlantEquipmentType::Invalid), WHtankID(0), LoopNum(0), LoopSideNum(0), IsWHCallAvail(false), CheckWHCall(false),
              CurMode(IHPOperationMode::Idle), ControlledZoneTemp(0), WaterFlowAccumVol(0), SHDWHRunTime(0), CoolVolFlowScale(0), HeatVolFlowScale(0),
              MaxHeatAirMassFlow(0), MaxHeatAirVolFlow(0), MaxCoolAirMassFlow(0), MaxCoolAirVolFlow(0), IHPCoilsSized(false), IDFanID(0),
              fanPlace(HVAC::FanPlace::Invalid), ODAirInletNodeNum(0),                                                // oudoor coil inlet Nod
              ODAirOutletNodeNum(0),                                                                                  // oudoor coil outlet Nod
              TankSourceWaterMassFlowRate(0), AirFlowSavInWaterLoop(0), AirFlowSavInAirLoop(0), AirLoopFlowRate(0.0), // air loop mass flow rate
              TotalCoolingRate(0.0),                                                                                  // total cooling rate [w]
              TotalWaterHeatingRate(0.0),                                                                             // total water heating rate [w]
              TotalSpaceHeatingRate(0.0),                                                                             // total space heating rate [w]
              TotalPower(0.0),                                                                                        // total power consumption  [w]
              TotalLatentLoad(0),                                                                                     // total latent cooling rate [w]
              Qsource(0.0),                                                                                           // source energy rate, [w]
              Energy(0.0),                      // total electric energy consumption [J]
              EnergyLoadTotalCooling(0.0),      // total cooling energy [J]
              EnergyLoadTotalHeating(0.0),      // total heating energy [J]
              EnergyLoadTotalWaterHeating(0.0), // total heating energy [J]
              EnergyLatent(0.0),                // total latent energy [J]
              EnergySource(0.0),                // total source energy
              TotalCOP(0.0)                     // total COP
        {
        }
    };

    void SimIHP(EnergyPlusData &state,
                std::string_view CompName,       // Coil Name
                int &CompIndex,                  // Index for Component name
                HVAC::FanOp const fanOp,         // Continuous fan OR cycling compressor
                HVAC::CompressorOp compressorOp, // compressor on/off. 0 = off; 1= on
                Real64 const PartLoadFrac,
                int const SpeedNum,            // compressor speed number
                Real64 const SpeedRatio,       // compressor speed ratio
                Real64 const SensLoad,         // Sensible demand load [W]
                Real64 const LatentLoad,       // Latent demand load [W]
                bool const IsCallbyWH,         // whether the call from the water heating loop or air loop, true = from water heating loop
                bool const FirstHVACIteration, // TRUE if First iteration of simulation
                ObjexxFCL::Optional<Real64 const> OnOffAirFlowRat = _ // ratio of comp on to comp off air flow rate
    );

    void GetIHPInput(EnergyPlusData &state);

    void SizeIHP(EnergyPlusData &state, int const CoilNum);

    void InitializeIHP(EnergyPlusData &state, int const DXCoilNum);

    void UpdateIHP(EnergyPlusData &state, int const DXCoilNum);

    void DecideWorkMode(EnergyPlusData &state,
                        int const DXCoilNum,
                        Real64 const SensLoad,  // Sensible demand load [W]
                        Real64 const LatentLoad // Latent demand load [W]
    );

    IHPOperationMode GetCurWorkMode(EnergyPlusData &state, int const DXCoilNum);

    int GetLowSpeedNumIHP(EnergyPlusData &state, int const DXCoilNum);

    int GetMaxSpeedNumIHP(EnergyPlusData &state, int const DXCoilNum);

    Real64 GetAirVolFlowRateIHP(EnergyPlusData &state,
                                int const DXCoilNum,
                                int const SpeedNum,
                                Real64 const SpeedRatio,
                                bool const IsCallbyWH // whether the call from the water heating loop or air loop, true = from water heating loop
    );

    Real64 GetWaterVolFlowRateIHP(EnergyPlusData &state, int const DXCoilNum, int const SpeedNum, Real64 const SpeedRatio);

    Real64 GetAirMassFlowRateIHP(EnergyPlusData &state,
                                 int const DXCoilNum,
                                 int const SpeedNum,
                                 Real64 const SpeedRatio,
                                 bool const IsCallbyWH // whether the call from the water heating loop or air loop, true = from water heating loop
    );

    bool IHPInModel(EnergyPlusData &state);

    int GetCoilIndexIHP(EnergyPlusData &state,
                        std::string const &CoilType, // must match coil types in this module
                        std::string const &CoilName, // must match coil names for the coil type
                        bool &ErrorsFound            // set to true if problem
    );

    int GetCoilInletNodeIHP(EnergyPlusData &state,
                            std::string const &CoilType, // must match coil types in this module
                            std::string const &CoilName, // must match coil names for the coil type
                            bool &ErrorsFound            // set to true if problem
    );

    int GetDWHCoilInletNodeIHP(EnergyPlusData &state,
                               std::string const &CoilType, // must match coil types in this module
                               std::string const &CoilName, // must match coil names for the coil type
                               bool &ErrorsFound            // set to true if problem
    );

    int GetDWHCoilOutletNodeIHP(EnergyPlusData &state,
                                std::string const &CoilType, // must match coil types in this module
                                std::string const &CoilName, // must match coil names for the coil type
                                bool &ErrorsFound            // set to true if problem
    );

    Real64 GetDWHCoilCapacityIHP(EnergyPlusData &state,
                                 std::string const &CoilType, // must match coil types in this module
                                 std::string const &CoilName, // must match coil names for the coil type
                                 IHPOperationMode const Mode, // mode coil type
                                 bool &ErrorsFound            // set to true if problem
    );

    int GetIHPDWHCoilPLFFPLR(EnergyPlusData &state,
                             std::string const &CoilType, // must match coil types in this module
                             std::string const &CoilName, // must match coil names for the coil type
                             IHPOperationMode const Mode, // mode coil type
                             bool &ErrorsFound            // set to true if problem
    );

    void ClearCoils(EnergyPlusData &state, int const DXCoilNum // coil ID
    );

} // namespace IntegratedHeatPump

struct IntegratedHeatPumpGlobalData : BaseGlobalStruct
{

    bool GetCoilsInputFlag = true;
    EPVector<IntegratedHeatPump::IntegratedHeatPumpData> IntegratedHeatPumps;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->GetCoilsInputFlag = true;
        this->IntegratedHeatPumps.deallocate();
    }
};

} // namespace EnergyPlus

#endif

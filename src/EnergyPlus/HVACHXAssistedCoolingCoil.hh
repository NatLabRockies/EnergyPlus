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

#ifndef HVACHXAssistedCoolingCoil_hh_INCLUDED
#define HVACHXAssistedCoolingCoil_hh_INCLUDED

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

namespace HXAssistCoil {

    struct HXAssistedCoilParameters
    {
        // Members
        std::string Name;               // Name of the HXAssistedCoolingCoil
        HVAC::CoilType coilType = HVAC::CoilType::Invalid; // coilType of the parent object
        int HXCoilInNodeNum = 0;              // Inlet node to HXAssistedCoolingCoil compound object
        int HXCoilOutNodeNum = 0;             // Outlet node to HXAssistedCoolingCoil compound object
      
        HVAC::CoilType coolCoilType = HVAC::CoilType::Invalid; // Must be DetailedFlatCooling Coil:DX:CoolingBypassFactorEmpirical
        std::string CoolCoilName; // Cooling coil name
        int CoolCoilNum = 0;

        int CoolCoilCondenserInNodeNum = 0;
        int CoolCoilWaterInNodeNum = 0;
        int CoolCoilWaterOutNodeNum = 0;
        int CoolCoilInNodeNum = 0;
        int CoolCoilOutNodeNum = 0;
      
        int DXCoilNumOfSpeeds; // number of speed levels for variable speed DX coil
        // Heat Exchanger type must be HeatExchanger:AirToAir:FlatPlate,
        // HeatExchanger:AirToAir:SensibleAndLatent or
        // HeatExchanger:Desiccant:BalancedFlow

        Real64 Capacity = 0.0;
      
        HVAC::HXType hxType = HVAC::HXType::Invalid; // Numeric Equivalent for heat exchanger
        std::string HeatExchangerName;               // Heat Exchanger name
        int HeatExchangerNum = 0;                      // Heat Exchanger index

        int SupplyAirInNodeNum = 0;
        int SupplyAirOutNodeNum = 0;
        int SecAirInNodeNum = 0;
        int SecAirOutNodeNum = 0;
      
        int HXCoilExhaustAirInNodeNum = 0;                // Inlet node number for air-to-air heat exchanger

        Real64 MassFlowRate = 0.0;                         // Mass flow rate through HXAssistedCoolingCoil compound object
        int MaxIterCounter = 0;                          // used in warning messages
        int MaxIterIndex = 0;                            // used in warning messages
        int ControllerIndex = 0;                         // index to water coil controller
        std::string ControllerName;                      // name of water coil controller

        Real64 MaxWaterFlowRate = 0.0;
        Real64 MaxAirFlowRate = 0.0;
    };

    void
    SimHXAssistedCoolingCoil(EnergyPlusData &state,
                             std::string_view HXAssistedCoilName, // Name of HXAssistedCoolingCoil
                             bool const FirstHVACIteration,       // FirstHVACIteration flag
                             HVAC::CompressorOp compressorOp,     // compressor operation; 1=on, 0=off
                             Real64 const PartLoadRatio,          // Part load ratio of Coil:DX:CoolingBypassFactorEmpirical
                             int &CompIndex,
                             HVAC::FanOp const fanOp,                           // Allows the parent object to control fan operation
                             ObjexxFCL::Optional_bool_const HXUnitEnable = _,   // flag to enable heat exchanger heat recovery
                             ObjexxFCL::Optional<Real64 const> OnOffAFR = _,    // Ratio of compressor ON air mass flow rate to AVERAGE over time step
                             ObjexxFCL::Optional_bool_const EconomizerFlag = _, // OA sys or air loop economizer status
                             ObjexxFCL::Optional<Real64> QTotOut = _,           // the total cooling output of unit
                             ObjexxFCL::Optional<HVAC::CoilMode const> DehumidificationMode = _, // Optional dehumbidication mode
                             ObjexxFCL::Optional<Real64 const> LoadSHR = _                       // Optional coil SHR pass over
    );

    void
    SimHXAssistedCoolingCoil(EnergyPlusData &state,
                             int const coilNum,
                             bool const FirstHVACIteration,       // FirstHVACIteration flag
                             HVAC::CompressorOp compressorOp,     // compressor operation; 1=on, 0=off
                             Real64 const PartLoadRatio,          // Part load ratio of Coil:DX:CoolingBypassFactorEmpirical
                             HVAC::FanOp const fanOp,                           // Allows the parent object to control fan operation
                             ObjexxFCL::Optional_bool_const HXUnitEnable = _,   // flag to enable heat exchanger heat recovery
                             ObjexxFCL::Optional<Real64 const> OnOffAFR = _,    // Ratio of compressor ON air mass flow rate to AVERAGE over time step
                             ObjexxFCL::Optional_bool_const EconomizerFlag = _, // OA sys or air loop economizer status
                             ObjexxFCL::Optional<Real64> QTotOut = _,           // the total cooling output of unit
                             ObjexxFCL::Optional<HVAC::CoilMode const> DehumidificationMode = _, // Optional dehumbidication mode
                             ObjexxFCL::Optional<Real64 const> LoadSHR = _                       // Optional coil SHR pass over
    );
  
    void GetHXAssistedCoolingCoilInput(EnergyPlusData &state);

    void InitHXAssistedCoolingCoil(EnergyPlusData &state, int const HXAssistedCoilNum); // index for HXAssistedCoolingCoil

    void
    CalcHXAssistedCoolingCoil(EnergyPlusData &state,
                              int const HXAssistedCoilNum,                        // Index number for HXAssistedCoolingCoil
                              bool const FirstHVACIteration,                      // FirstHVACIteration flag
                              HVAC::CompressorOp compressorOp,                    // compressor operation; 1=on, 0=off
                              Real64 const PartLoadRatio,                         // Cooling coil part load ratio
                              bool const HXUnitOn,                                // Flag to enable heat exchanger
                              HVAC::FanOp const fanOp,                            // Allows parent object to control fan operation
                              ObjexxFCL::Optional<Real64 const> OnOffAirFlow = _, // Ratio of compressor ON air mass flow to AVERAGE over time step
                              ObjexxFCL::Optional_bool_const EconomizerFlag = _,  // OA (or airloop) econommizer status
                              ObjexxFCL::Optional<HVAC::CoilMode const> DehumidificationMode = _, // Optional dehumbidication mode
                              [[maybe_unused]] ObjexxFCL::Optional<Real64 const> LoadSHR = _      // Optional coil SHR pass over
    );

    int GetCoilIndex(EnergyPlusData &state,
                     std::string_view const coilType,
                     std::string const &coilName,
                     bool &ErrorsFound);

#ifdef OLD_API
    Real64 GetCoilCapacity(EnergyPlusData &state,
                             std::string_view const coilType,
                             std::string const &coilName,
                             bool &ErrorsFound);

    HVAC::CoilType GetCoilChildCoilType(EnergyPlusData &state,
                                         std::string_view const coilType,
                                         std::string const &coilName, 
                                         bool &ErrorsFound);

    std::string GetCoilChildCoilName(EnergyPlusData &state,
                                      std::string_view const coilType,
                                      std::string const &coilName, 
                                      bool &ErrorsFound);

    int GetCoilChildCoilIndex(EnergyPlusData &state,
                              std::string_view const coilType,
                              std::string const &coilName, 
                              bool &ErrorsFound);
  
    int GetCoilAirInletNode(EnergyPlusData &state, std::string_view const coilType, std::string const &coilName, bool &ErrorsFound);

    int GetCoilAirOutletNode(EnergyPlusData &state, std::string_view const coilType, std::string const &coilName, bool &ErrorsFound);

    int GetCoilCondenserInletNode(EnergyPlusData &state, std::string_view const coilType, std::string const &coilName, bool &ErrorsFound);
  
    Real64 GetCoilMaxWaterFlowRate(EnergyPlusData &state, std::string_view const coilType, std::string const &coilName, bool &ErrorsFound);

    Real64 GetCoilMaxAirFlowRate(EnergyPlusData &state, std::string_view const coilType, std::string const &coilName, bool &ErrorsFound);

    Real64 GetCoilScheduleValue(EnergyPlusData &state, std::string_view const compType, std::string const &coilName, bool &ErrorsFound);
#endif // OLD_API
  
    // New API
    int GetCoilIndex(EnergyPlusData &state, std::string const &coilName);

    Real64 GetCoilCapacity(EnergyPlusData &state, int const coilNum);

    Real64 GetCoilMaxAirFlowRate(EnergyPlusData &state, int const coilNum);

    Real64 GetCoilMaxWaterFlowRate(EnergyPlusData &state, int const coilNum);

    int GetCoilAirInletNode(EnergyPlusData &state, int const coilNum);
  
    int GetCoilAirOutletNode(EnergyPlusData &state, int const coilNum);

    int GetCoilCondenserInletNode(EnergyPlusData &state, int const coilNum);
  
    int GetCoilWaterInletNode(EnergyPlusData &state, int const coilNum);

    int GetCoilWaterOutletNode(EnergyPlusData &state, int const coilNum);

    HVAC::CoilType GetCoilChildCoilType(EnergyPlusData &state, int const coilNum);

    std::string GetCoilChildCoilName(EnergyPlusData &state, int const coilNum);

    int GetCoilChildCoilIndex(EnergyPlusData &state, int const coilType);  
  
    bool VerifyHeatExchangerParent(EnergyPlusData &state,
                                   std::string const &HXType, // must match coil types in this module
                                   std::string const &HXName  // must match coil names for the coil type
    );

    Real64 GetCoilScheduleValue(EnergyPlusData &state, int const coilNum);
} // namespace HXAssistCoil

struct HVACHXAssistedCoolingCoilData : BaseGlobalStruct
{
    int TotalNumHXAssistedCoils = 0;            // The total number of HXAssistedCoolingCoil compound objects
    Array1D<Real64> HXAssistedCoilOutletTemp;   // Outlet temperature from this compound object
    Array1D<Real64> HXAssistedCoilOutletHumRat; // Outlet humidity ratio from this compound object
    bool GetCoilsInputFlag = true;              // Flag to allow input data to be retrieved from idf on first call to this subroutine
    Array1D_bool CheckEquipName;
    Array1D<HXAssistCoil::HXAssistedCoilParameters> HXAssistedCoils;
    std::unordered_map<std::string, std::string> UniqueHXAssistedCoilNames;
    Real64 CoilOutputTempLast; // Exiting cooling coil temperature from last iteration
    int ErrCount = 0;
    int ErrCount2 = 0;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->TotalNumHXAssistedCoils = 0;
        this->HXAssistedCoilOutletTemp.clear();
        this->HXAssistedCoilOutletHumRat.clear();
        this->GetCoilsInputFlag = true;
        this->CheckEquipName.clear();
        this->HXAssistedCoils.clear();
        this->UniqueHXAssistedCoilNames.clear();
        this->ErrCount = 0;
        this->ErrCount2 = 0;
    }
};

} // namespace EnergyPlus

#endif

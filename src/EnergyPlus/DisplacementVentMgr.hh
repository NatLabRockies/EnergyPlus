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

#ifndef DisplacementVentMgr_hh_INCLUDED
#define DisplacementVentMgr_hh_INCLUDED

// EnergyPlus Headers
#include <EnergyPlus/ConvectionCoefficients.hh>
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace RoomAir {

    void ManageDispVent3Node(EnergyPlusData &state, int ZoneNum); // index number for the specified zone

    void InitDispVent3Node(EnergyPlusData &state, int ZoneNum);

    void HcDispVent3Node(EnergyPlusData &state, int ZoneNum, Real64 FractionHeight);

    Real64 calculateThirdOrderFloorTemperature(Real64 temperatureHistoryTerm,
                                               Real64 HAT_floor,
                                               Real64 HA_floor,
                                               Real64 MCpT_Total,
                                               Real64 MCp_Total,
                                               Real64 occupiedTemp,
                                               Real64 nonAirSystemResponse,
                                               Real64 zoneMultiplier,
                                               Real64 airCap);

    void CalcDispVent3Node(EnergyPlusData &state, int ZoneNum); // Which Zonenum

    static constexpr std::array<DataHeatBalance::IntGainType, 52> IntGainTypesOccupied = {
        DataHeatBalance::IntGainType::People,
        DataHeatBalance::IntGainType::WaterHeaterMixed,
        DataHeatBalance::IntGainType::WaterHeaterStratified,
        DataHeatBalance::IntGainType::ThermalStorageChilledWaterMixed,
        DataHeatBalance::IntGainType::ThermalStorageChilledWaterStratified,
        DataHeatBalance::IntGainType::ThermalStorageHotWaterStratified,
        DataHeatBalance::IntGainType::ElectricEquipment,
        DataHeatBalance::IntGainType::ElectricEquipmentITEAirCooled,
        DataHeatBalance::IntGainType::GasEquipment,
        DataHeatBalance::IntGainType::HotWaterEquipment,
        DataHeatBalance::IntGainType::SteamEquipment,
        DataHeatBalance::IntGainType::OtherEquipment,
        DataHeatBalance::IntGainType::IndoorGreen,
        DataHeatBalance::IntGainType::ZoneBaseboardOutdoorTemperatureControlled,
        DataHeatBalance::IntGainType::GeneratorFuelCell,
        DataHeatBalance::IntGainType::WaterUseEquipment,
        DataHeatBalance::IntGainType::GeneratorMicroCHP,
        DataHeatBalance::IntGainType::ElectricLoadCenterTransformer,
        DataHeatBalance::IntGainType::ElectricLoadCenterInverterSimple,
        DataHeatBalance::IntGainType::ElectricLoadCenterInverterFunctionOfPower,
        DataHeatBalance::IntGainType::ElectricLoadCenterInverterLookUpTable,
        DataHeatBalance::IntGainType::ElectricLoadCenterStorageLiIonNmcBattery,
        DataHeatBalance::IntGainType::ElectricLoadCenterStorageBattery,
        DataHeatBalance::IntGainType::ElectricLoadCenterStorageSimple,
        DataHeatBalance::IntGainType::PipeIndoor,
        DataHeatBalance::IntGainType::RefrigerationCase,
        DataHeatBalance::IntGainType::RefrigerationCompressorRack,
        DataHeatBalance::IntGainType::RefrigerationSystemAirCooledCondenser,
        DataHeatBalance::IntGainType::RefrigerationSystemSuctionPipe,
        DataHeatBalance::IntGainType::RefrigerationSecondaryReceiver,
        DataHeatBalance::IntGainType::RefrigerationSecondaryPipe,
        DataHeatBalance::IntGainType::RefrigerationWalkIn,
        DataHeatBalance::IntGainType::RefrigerationTransSysAirCooledGasCooler,
        DataHeatBalance::IntGainType::RefrigerationTransSysSuctionPipeMT,
        DataHeatBalance::IntGainType::RefrigerationTransSysSuctionPipeLT,
        DataHeatBalance::IntGainType::Pump_VarSpeed,
        DataHeatBalance::IntGainType::Pump_ConSpeed,
        DataHeatBalance::IntGainType::Pump_Cond,
        DataHeatBalance::IntGainType::PumpBank_VarSpeed,
        DataHeatBalance::IntGainType::PumpBank_ConSpeed,
        DataHeatBalance::IntGainType::PlantComponentUserDefined,
        DataHeatBalance::IntGainType::CoilUserDefined,
        DataHeatBalance::IntGainType::ZoneHVACForcedAirUserDefined,
        DataHeatBalance::IntGainType::AirTerminalUserDefined,
        DataHeatBalance::IntGainType::PackagedTESCoilTank,
        DataHeatBalance::IntGainType::SecCoolingDXCoilSingleSpeed,
        DataHeatBalance::IntGainType::SecHeatingDXCoilSingleSpeed,
        DataHeatBalance::IntGainType::SecCoolingDXCoilTwoSpeed,
        DataHeatBalance::IntGainType::SecCoolingDXCoilMultiSpeed,
        DataHeatBalance::IntGainType::SecHeatingDXCoilMultiSpeed,
        DataHeatBalance::IntGainType::ElectricLoadCenterConverter,
        DataHeatBalance::IntGainType::FanSystemModel};

    static constexpr std::array<DataHeatBalance::IntGainType, 2> IntGainTypesMixedSubzone = {DataHeatBalance::IntGainType::DaylightingDeviceTubular,
                                                                                             DataHeatBalance::IntGainType::Lights};

    // Explicitly list internal gains not applicable for Displacement Vent
    static constexpr std::array<DataHeatBalance::IntGainType, 2> ExcludedIntGainTypes = {
        DataHeatBalance::IntGainType::ZoneContaminantSourceAndSinkCarbonDioxide,
        DataHeatBalance::IntGainType::ZoneContaminantSourceAndSinkGenericContam};

} // namespace RoomAir

struct DisplacementVentMgrData : BaseGlobalStruct
{
    Real64 HAT_MX = 0.0;                  // HAT_MX Convection Coefficient times Area times Temperature for the upper subzone
    Real64 HA_MX = 0.0;                   // HA_MX Convection Coefficient times Area for the upper subzone
    Real64 HAT_OC = 0.0;                  // HAT_OC Convection Coefficient times Area times Temperature for the lower subzone
    Real64 HA_OC = 0.0;                   // HA_OC Convection Coefficient times Area for the lower subzone
    Real64 HAT_FLOOR = 0.0;               // HAT_FLOOR Convection Coefficient times Area times Temperature for the floor(?) subzone
    Real64 HA_FLOOR = 0.0;                // HA_FLOOR Convection Coefficient times Area for the floor(?) subzone
    Real64 HeightFloorSubzoneTop = 0.2;   // Assumed thickness of floor subzone
    Real64 ThickOccupiedSubzoneMin = 0.2; // Minimum thickness of occupied subzone
    Real64 HeightIntMass = 0.0;           // Height of internal mass surfaces, assumed vertical, cannot exceed ceiling height
    Real64 HeightIntMassDefault = 2.0;    // Default height of internal mass surfaces
    bool InitUCSDDVMyOneTimeFlag = true;
    Array1D_bool MyEnvrnFlag;
    Real64 TempDepCoef = 0.0; // Formerly CoefSumha, coef in zone temp equation with dimensions of h*A
    Real64 TempIndCoef = 0.0; // Formerly CoefSumhat, coef in zone temp equation with dimensions of h*A(T1

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->HAT_MX = 0.0;
        this->HA_MX = 0.0;
        this->HAT_OC = 0.0;
        this->HA_OC = 0.0;
        this->HAT_FLOOR = 0.0;
        this->HA_FLOOR = 0.0;
        this->HeightFloorSubzoneTop = 0.2;
        this->ThickOccupiedSubzoneMin = 0.2;
        this->HeightIntMass = 0.0;
        this->HeightIntMassDefault = 2.0;
        this->InitUCSDDVMyOneTimeFlag = true;
        this->MyEnvrnFlag.clear();
        this->TempDepCoef = 0.0;
        this->TempIndCoef = 0.0;
    }
};

} // namespace EnergyPlus

#endif

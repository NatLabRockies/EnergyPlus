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

#ifndef WaterToAirHeatPumpSimple_hh_INCLUDED
#define WaterToAirHeatPumpSimple_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>
#include <ObjexxFCL/Optional.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/DataHVACGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace Curve {
    struct Curve;
}

namespace WaterToAirHeatPumpSimple {

    enum class WatertoAirHP
    {
        Invalid = -1,
        Heating,
        Cooling,
        Num
    };

    struct SimpleWatertoAirHPConditions
    {
        // Members
        std::string Name; // Name of the Water to Air Heat pump

        HVAC::CoilType coilType = HVAC::CoilType::Invalid;
        int coilReportNum = -1;

        Sched::Schedule *availSched = nullptr;                                                // availability schedule
        WatertoAirHP WAHPType = WatertoAirHP::Invalid;                                        // Type of WatertoAirHP ie. Heating or Cooling
        DataPlant::PlantEquipmentType WAHPPlantType = DataPlant::PlantEquipmentType::Invalid; // type of component in plant
        bool SimFlag = false;                                                                 // Heat Pump Simulation Flag
        Real64 AirVolFlowRate = 0.0;                                                          // Air Volumetric Flow Rate[m3/s]
        Real64 AirMassFlowRate = 0.0;                                                         // Air Mass Flow Rate[kg/s]
        Real64 InletAirDBTemp = 0.0;                                                          // Inlet Air Dry Bulb Temperature [C]
        Real64 InletAirHumRat = 0.0;                                                          // Inlet Air Humidity Ratio [kg/kg]
        Real64 InletAirEnthalpy = 0.0;                                                        // Inlet Air Enthalpy [J/kg]
        Real64 OutletAirDBTemp = 0.0;                                                         // Outlet Air Dry Bulb Temperature [C]
        Real64 OutletAirHumRat = 0.0;                                                         // Outlet Air Humidity Ratio [kg/kg]
        Real64 OutletAirEnthalpy = 0.0;                                                       // Outlet Air Enthalpy [J/kg]
        Real64 WaterVolFlowRate = 0.0;                                                        // Water Volumetric Flow Rate [m3/s]
        Real64 WaterMassFlowRate = 0.0;                                                       // Water Mass Flow Rate [kg/s]
        Real64 DesignWaterMassFlowRate = 0.0;
        Real64 InletWaterTemp = 0.0;                 // Inlet Water Temperature [C]
        Real64 InletWaterEnthalpy = 0.0;             // Inlet Water Enthalpy [J/kg]
        Real64 OutletWaterTemp = 0.0;                // Outlet Water Temperature [C]
        Real64 OutletWaterEnthalpy = 0.0;            // Outlet Water Enthalpy [J/kg]
        Real64 Power = 0.0;                          // Power Consumption [W]
        Real64 QLoadTotal = 0.0;                     // Load Side Total Heat Transfer Rate [W]
        Real64 QLoadTotalReport = 0.0;               // Load side total heat transfer rate for reporting[W]
        Real64 QSensible = 0.0;                      // Sensible Load Side Heat Transfer Rate [W]
        Real64 QLatent = 0.0;                        // Latent Load Side Heat Transfer Rate [W]
        Real64 QSource = 0.0;                        // Source Side Heat Transfer Rate [W]
        Real64 Energy = 0.0;                         // Energy Consumption [J]
        Real64 EnergyLoadTotal = 0.0;                // Load Side Total Heat Transferred [J]
        Real64 EnergySensible = 0.0;                 // Sensible Load Side Heat Transferred [J]
        Real64 EnergyLatent = 0.0;                   // Latent Load Side Heat Transferred [J]
        Real64 EnergySource = 0.0;                   // Source Side Heat Transferred [J]
        Real64 COP = 0.0;                            // Heat Pump Coefficient of Performance [-]
        Real64 RunFrac = 0.0;                        // Duty Factor
        Real64 PartLoadRatio = 0.0;                  // Part Load Ratio
        Real64 RatedWaterVolFlowRate = 0.0;          // Rated Water Volumetric Flow Rate [m3/s]
        Real64 RatedAirVolFlowRate = 0.0;            // Rated Air Volumetric Flow Rate [m3/s]
        Real64 RatedCapHeat = 0.0;                   // Rated Heating Capacity [W]
        Real64 RatedCapHeatAtRatedCdts = 0.0;        // Rated Heating Capacity at Rated Conditions [W]
        Real64 RatedCapCoolAtRatedCdts = 0.0;        // Rated Cooling Capacity at Rated Conditions [W]
        Real64 RatedCapCoolSensDesAtRatedCdts = 0.0; // Rated Sensible Capacity at Rated Conditions [W]
        Real64 RatedPowerHeat = 0.0;                 // Rated Heating Power Consumption [W]
        Real64 RatedPowerHeatAtRatedCdts = 0.0;      // Rated Heating Power Consumption at Rated Conditions[W]
        Real64 RatedCOPHeatAtRatedCdts = 0.0;        // Rated Heating COP at Rated Conditions [W/w]
        Real64 RatedCapCoolTotal = 0.0;              // Rated Total Cooling Capacity [W]
        Real64 RatedCapCoolSens = 0.0;               // Rated Sensible Cooling Capacity [W]
        Real64 RatedPowerCool = 0.0;                 // Rated Cooling Power Consumption[W]
        Real64 RatedPowerCoolAtRatedCdts = 0.0;      // Rated Cooling Power Consumption at Rated Conditions [W]
        Real64 RatedCOPCoolAtRatedCdts = 0.0;        // Rated Cooling COP at Rated Conditions [W/W]
        Real64 RatedEntWaterTemp = 0.0;              // Rated Entering Water Temperature [C]
        Real64 RatedEntAirWetbulbTemp = 0.0;         // Rated Entering Air Wetbulb Temperature [C]
        Real64 RatedEntAirDrybulbTemp = 0.0;         // Rated Entering Air Drybulb Temperature [C]
        Real64 RatioRatedHeatRatedTotCoolCap = 0.0;  // Ratio of Rated Heating Capacity to Rated Cooling Capacity [-]

        Curve::Curve *HeatCapCurve = nullptr;      // Index of the heating capacity performance curve
        Curve::Curve *HeatPowCurve = nullptr;      // Index of the heating power consumption curve
        Curve::Curve *TotalCoolCapCurve = nullptr; // Index of the Total Cooling capacity performance curve
        Curve::Curve *SensCoolCapCurve = nullptr;  // Index of the Sensible Cooling capacity performance curve
        Curve::Curve *CoolPowCurve = nullptr;      // Index of the Cooling power consumption curve
        Curve::Curve *PLFCurve = nullptr;          // Index of the Part Load Factor curve

        int AirInletNodeNum = 0;    // Node Number of the Air Inlet
        int AirOutletNodeNum = 0;   // Node Number of the Air Outlet
        int WaterInletNodeNum = 0;  // Node Number of the Water Onlet
        int WaterOutletNodeNum = 0; // Node Number of the Water Outlet

        PlantLocation plantLoc;
        HVAC::WaterFlow WaterCyclingMode = HVAC::WaterFlow::Invalid; // Heat Pump Coil water flow mode; See definitions in DataHVACGlobals,
        // 1=water cycling, 2=water constant, 3=water constant on demand (old mode)
        int LastOperatingMode = 0; // type of coil calling for water flow, either heating or cooling,
        // start it at 1 so there will be water flow from the start,
        // even if there is no load.
        // Gets updated only during the first iteration of each timestep
        bool WaterFlowMode = false; // whether the water flow through the coil is called
        // because there is a load on the coil, or not.
        // Gets updated each iteration
        // set by parent object and "pushed" to this structure in SetSimpleWSHPData subroutine
        int CompanionCoolingCoilNum = 0; // Heating coil companion cooling coil index
        int CompanionHeatingCoilNum = 0; // Cooling coil companion heating coil index
        Real64 Twet_Rated = 0.0;         // Nominal Time for Condensate Removal to Begin [s]
        Real64 Gamma_Rated = 0.0;        // Ratio of Initial Moisture Evaporation Rate
        // and Steady-state Latent Capacity
        Real64 MaxONOFFCyclesperHour = 0.0;      // Maximum cycling rate of heat pump [cycles/hr]
        Real64 LatentCapacityTimeConstant = 0.0; // Latent capcacity time constant [s]
        Real64 FanDelayTime = 0.0;               // Fan delay time, time delay for the HP's fan to
        bool reportCoilFinalSizes = true;        // one time report of sizes to coil report
        bool LowFlowFlag = true;                 // one time low flow warning for coil in cycling fan mode
    };

    void SimWatertoAirHPSimple(EnergyPlusData &state,
                               std::string_view CompName, // Coil Name
                               int &CompIndex,            // Index for Component name
                               Real64 const SensLoad,     // Sensible demand load [W]
                               Real64 const LatentLoad,   // Latent demand load [W]
                               HVAC::FanOp const fanOp,   // Continuous fan OR cycling compressor
                               HVAC::CompressorOp compressorOp,
                               Real64 const PartLoadRatio,
                               bool const FirstHVACIteration,
                               Real64 const OnOffAirFlowRat = 1.0 // ratio of comp on to comp off air flow rate
    );

    // MODULE SUBROUTINES:
    //*************************************************************************

    void GetSimpleWatertoAirHPInput(EnergyPlusData &state);

    // Beginning Initialization Section of the Module
    //******************************************************************************

    void InitSimpleWatertoAirHP(EnergyPlusData &state,
                                int const HPNum,                // Current HPNum under simulation
                                Real64 const SensLoad,          // Control zone sensible load[W]
                                Real64 const LatentLoad,        // Control zone latent load[W]
                                HVAC::FanOp const fanOp,        // fan operating mode
                                Real64 const OnOffAirFlowRatio, // ratio of compressor on flow to average flow over time step
                                bool const FirstHVACIteration,  // Iteration flag
                                Real64 const PartLoadRatio      // compressor part load ratio
    );

    void SizeHVACWaterToAir(EnergyPlusData &state, int const HPNum);

    void CalcHPCoolingSimple(EnergyPlusData &state,
                             int const HPNum,                 // Heat Pump Number
                             HVAC::FanOp const fanOp,         // Fan/Compressor cycling scheme indicator
                             Real64 const SensDemand,         // Cooling Sensible Demand [W] !unused1208
                             Real64 const LatentDemand,       // Cooling Latent Demand [W]
                             HVAC::CompressorOp compressorOp, // compressor operation flag
                             Real64 const PartLoadRatio,      // compressor part load ratio
                             Real64 const OnOffAirFlowRatio   // ratio of compressor on flow to average flow over time step
    );

    void CalcHPHeatingSimple(EnergyPlusData &state,
                             int const HPNum,                 // Heat Pump Number
                             HVAC::FanOp const fanOp,         // Fan/Compressor cycling scheme indicator
                             Real64 const SensDemand,         // Cooling Sensible Demand [W] !unused1208
                             HVAC::CompressorOp compressorOp, // compressor operation flag
                             Real64 const PartLoadRatio,      // compressor part load ratio
                             Real64 const OnOffAirFlowRatio   // ratio of compressor on flow to average flow over time step
    );

    void UpdateSimpleWatertoAirHP(EnergyPlusData &state, int const HPNum);

    //        End of Update subroutines for the WatertoAirHP Module
    // *****************************************************************************

    Real64 CalcEffectiveSHR(EnergyPlusData &state,
                            int const HPNum,         // Index number for cooling coil
                            Real64 const SHRss,      // Steady-state sensible heat ratio
                            HVAC::FanOp const fanOp, // Fan/compressor cycling scheme indicator
                            Real64 const RTF,        // Compressor run-time fraction
                            Real64 const QLatRated,  // Rated latent capacity
                            Real64 const QLatActual, // Actual latent capacity
                            Real64 const EnteringDB, // Entering air dry-bulb temperature
                            Real64 const EnteringWB  // Entering air wet-bulb temperature
    );

    int GetCoilIndex(EnergyPlusData &state,
                     std::string const &CoilType, // must match coil types in this module
                     std::string const &CoilName, // must match coil names for the coil type
                     bool &ErrorsFound            // set to true if problem
    );

    Real64 GetCoilCapacity(EnergyPlusData &state,
                           std::string_view const coilType, // must match coil types in this module
                           std::string const &CoilName,     // must match coil names for the coil type
                           bool &ErrorsFound                // set to true if problem
    );

    Real64 GetCoilAirFlowRate(EnergyPlusData &state,
                              std::string_view const coilType, // must match coil types in this module
                              std::string const &CoilName,     // must match coil names for the coil type
                              bool &ErrorsFound                // set to true if problem
    );

    int GetCoilInletNode(EnergyPlusData &state,
                         std::string const &CoilType, // must match coil types in this module
                         std::string const &CoilName, // must match coil names for the coil type
                         bool &ErrorsFound            // set to true if problem
    );

    int GetCoilOutletNode(EnergyPlusData &state,
                          std::string const &CoilType, // must match coil types in this module
                          std::string const &CoilName, // must match coil names for the coil type
                          bool &ErrorsFound            // set to true if problem
    );

    void SetSimpleWSHPData(EnergyPlusData &state,
                           int const SimpleWSHPNum,                             // Number of OA Controller
                           bool &ErrorsFound,                                   // Set to true if certain errors found
                           HVAC::WaterFlow const waterCyclingMode,              // the coil water flow mode (cycling, constant or constantondemand)
                           ObjexxFCL::Optional_int CompanionCoolingCoilNum = _, // Index to cooling coil for heating coil = SimpleWSHPNum
                           ObjexxFCL::Optional_int CompanionHeatingCoilNum = _  // Index to heating coil for cooling coil = SimpleWSHPNum
    );

    void CheckSimpleWAHPRatedCurvesOutputs(EnergyPlusData &state,
                                           std::string const &CoilName // must match coil names for the coil type
    );

} // namespace WaterToAirHeatPumpSimple

struct WaterToAirHeatPumpSimpleData : BaseGlobalStruct
{
    int NumWatertoAirHPs; // The Number of Water to Air Heat Pumps found in the Input
                          // INTEGER        :: WaterIndex = 0                   ! Water index
                          // INTEGER        :: Count = 0
    int AirflowErrPointer;
    bool GetCoilsInputFlag; // Flag set to make sure you get input once
    Array1D_bool MySizeFlag;
    Array1D_bool SimpleHPTimeStepFlag; // determines whether the previous operating mode for the coil and it's partner has been initialized

    Real64 QLatRated;          // Latent Capacity [W] rated at entering air conditions [Tdb=26.7C Twb=19.4C]
    Real64 QLatActual;         // Actual Latent Capacity [W]
    Real64 Winput;             // Power Consumption [W]
    bool MyOneTimeFlag = true; // one time allocation flag

    Array1D<WaterToAirHeatPumpSimple::SimpleWatertoAirHPConditions> SimpleWatertoAirHP;

    Array1D_bool MyEnvrnFlag; // used for initializations each begin environment flag
    Array1D_bool MyPlantScanFlag;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumWatertoAirHPs = 0;
        this->AirflowErrPointer = 0;
        this->MyOneTimeFlag = true;
        this->GetCoilsInputFlag = true;
        this->MySizeFlag.clear();
        this->SimpleHPTimeStepFlag.clear();
        this->SimpleWatertoAirHP.deallocate();
        this->MyEnvrnFlag.deallocate();
        this->MyPlantScanFlag.deallocate();
    }

    // Default Constructor
    WaterToAirHeatPumpSimpleData()
        : NumWatertoAirHPs(0), AirflowErrPointer(0), GetCoilsInputFlag(true), QLatRated(0.0), QLatActual(0.0), Winput(0.0), MyOneTimeFlag(true)
    {
    }
};

} // namespace EnergyPlus

#endif

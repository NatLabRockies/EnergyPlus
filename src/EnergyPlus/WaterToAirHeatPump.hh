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

#ifndef WaterToAirHeatPump_hh_INCLUDED
#define WaterToAirHeatPump_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/FluidProperties.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace WaterToAirHeatPump {

    enum class CompressorType
    {
        Invalid = -1,
        Reciprocating,
        Rotary,
        Scroll,
        Num
    };

    struct WatertoAirHPEquipConditions
    {
        // Members
        std::string Name;                       // Name of the Water to Air Heat pump
        Sched::Schedule *availSched = nullptr;  // availability schedule
        std::string WatertoAirHPType;           // Type of WatertoAirHP ie. Heating or Cooling
        DataPlant::PlantEquipmentType WAHPType; // type of component in plant
        std::string Refrigerant;                // Refrigerant name
        Fluid::RefrigProps *refrig = nullptr;
        bool SimFlag;
        Real64 InletAirMassFlowRate;       // Inlet Air Mass Flow through the Water to Air Heat Pump being Simulated [kg/s]
        Real64 OutletAirMassFlowRate;      // Outlet Air Mass Flow through the Water to Air Heat Pump being Simulated [kg/s]
        Real64 InletAirDBTemp;             // Inlet Air Dry Bulb Temperature [C]
        Real64 InletAirHumRat;             // Inlet Air Humidity Ratio [kg/kg]
        Real64 OutletAirDBTemp;            // Outlet Air Dry Bulb Temperature [C]
        Real64 OutletAirHumRat;            // Outlet Air Humidity Ratio [kg/kg]
        Real64 InletAirEnthalpy;           // Inlet Air Enthalpy [J/kg]
        Real64 OutletAirEnthalpy;          // Outlet Air Enthalpy [J/kg]
        Real64 InletWaterTemp;             // Inlet Water Temperature [C]
        Real64 OutletWaterTemp;            // Outlet Water Temperature [C]
        Real64 InletWaterMassFlowRate;     // Inlet Water Mass Flow Rate [kg/s]
        Real64 OutletWaterMassFlowRate;    // Outlet Water Mass Flow Rate [kg/s]
        Real64 DesignWaterMassFlowRate;    // Design Water Mass Flow Rate [kg/s]
        Real64 DesignWaterVolFlowRate;     // Design Water Volumetric Flow Rate [m3/s]
        Real64 InletWaterEnthalpy;         // Inlet Water Enthalpy [J/kg]
        Real64 OutletWaterEnthalpy;        // Outlet Water Enthalpy [J/kg]
        Real64 Power;                      // Power Consumption [W]
        Real64 Energy;                     // Energy Consumption [J]
        Real64 QSensible;                  // Sensible Load Side Heat Transfer Rate [W]
        Real64 QLatent;                    // Latent Load Side Heat Transfer Rate [W]
        Real64 QSource;                    // Source Side Heat Transfer Rate [W]
        Real64 EnergySensible;             // Sensible Load Side Heat Transferred [J]
        Real64 EnergyLatent;               // Latent Load Side Heat Transferred [J]
        Real64 EnergySource;               // Source Side Heat Transferred [J]
        Real64 RunFrac;                    // Duty Factor
        Real64 PartLoadRatio;              // Part Load Ratio
        Real64 HeatingCapacity;            // Nominal Heating Capacity
        Real64 CoolingCapacity;            // Nominal Cooling Capacity
        Real64 QLoadTotal;                 // Load Side Total Heat Transfer Rate [W]
        Real64 EnergyLoadTotal;            // Load Side Total Heat Transferred [J]
        Real64 Twet_Rated;                 // Nominal Time for Condensate Removal to Begin [s]
        Real64 Gamma_Rated;                // Ratio of Initial Moisture Evaporation Rate and Steady-state Latent Capacity
        Real64 MaxONOFFCyclesperHour;      // Maximum cycling rate of heat pump [cycles/hr]
        Real64 LatentCapacityTimeConstant; // Latent capacity time constant [s]
        Real64 FanDelayTime;               // Fan delay time, time delay for the HP's fan to
        // shut off after compressor cycle off [s]
        Real64 SourceSideUACoeff;      // Source Side Heat Transfer coefficient [W/C]
        Real64 LoadSideTotalUACoeff;   // Load Side Total Heat Transfer coefficient [W/C]
        Real64 LoadSideOutsideUACoeff; // Load Side Outside Heat Transfer coefficient [W/C]
        Real64 CompPistonDisp;         // Compressor Piston Displacement [m3/s]
        Real64 CompClearanceFactor;    // Compressor Clearance Factor
        Real64 CompSucPressDrop;       // Suction Pressure Drop [Pa]
        Real64 SuperheatTemp;          // Superheat Temperature [C]
        Real64 PowerLosses;            // Constant Part of the Compressor Power Losses [W]
        Real64 LossFactor;             // Compressor Power Loss Factor
        Real64 RefVolFlowRate;         // Refrigerant Volume Flow rate at the beginning
        // of the Compression [m3/s]
        Real64 VolumeRatio;   // Built-in-volume ratio [~]
        Real64 LeakRateCoeff; // Coefficient for the relationship between
        // Pressure Ratio and Leakage Rate [~]
        Real64 SourceSideHTR1;         // Source Side Heat Transfer Resistance coefficient 1 [~]
        Real64 SourceSideHTR2;         // Source Side Heat Transfer Resistance coefficient 2 [k/kW]
        int PLFCurveIndex = 0;         // Index of the Part Load Factor curve
        Real64 HighPressCutoff;        // High Pressure Cut-off [Pa]
        Real64 LowPressCutoff;         // Low Pressure Cut-off [Pa]
        CompressorType compressorType; // Type of Compressor ie. Reciprocating,Rotary or Scroll
        int AirInletNodeNum;           // air side coil inlet node number
        int AirOutletNodeNum;          // air side coil outlet node number
        int WaterInletNodeNum;         // water side coil inlet node number
        int WaterOutletNodeNum;        // water side coil outlet node number
        int LowPressClgError;          // count for low pressure errors (cooling)
        int HighPressClgError;         // count for high pressure errors (cooling)
        int LowPressHtgError;          // count for low pressure errors (heating)
        int HighPressHtgError;         // count for high pressure errors (heating)
        PlantLocation plantLoc;

        // Default Constructor
        WatertoAirHPEquipConditions()
            : WAHPType(DataPlant::PlantEquipmentType::Invalid), SimFlag(false), InletAirMassFlowRate(0.0), OutletAirMassFlowRate(0.0),
              InletAirDBTemp(0.0), InletAirHumRat(0.0), OutletAirDBTemp(0.0), OutletAirHumRat(0.0), InletAirEnthalpy(0.0), OutletAirEnthalpy(0.0),
              InletWaterTemp(0.0), OutletWaterTemp(0.0), InletWaterMassFlowRate(0.0), OutletWaterMassFlowRate(0.0), DesignWaterMassFlowRate(0.0),
              DesignWaterVolFlowRate(0.0), InletWaterEnthalpy(0.0), OutletWaterEnthalpy(0.0), Power(0.0), Energy(0.0), QSensible(0.0), QLatent(0.0),
              QSource(0.0), EnergySensible(0.0), EnergyLatent(0.0), EnergySource(0.0), RunFrac(0.0), PartLoadRatio(0.0), HeatingCapacity(0.0),
              CoolingCapacity(0.0), QLoadTotal(0.0), EnergyLoadTotal(0.0), Twet_Rated(0.0), Gamma_Rated(0.0), MaxONOFFCyclesperHour(0.0),
              LatentCapacityTimeConstant(0.0), FanDelayTime(0.0), SourceSideUACoeff(0.0), LoadSideTotalUACoeff(0.0), LoadSideOutsideUACoeff(0.0),
              CompPistonDisp(0.0), CompClearanceFactor(0.0), CompSucPressDrop(0.0), SuperheatTemp(0.0), PowerLosses(0.0), LossFactor(0.0),
              RefVolFlowRate(0.0), VolumeRatio(0.0), LeakRateCoeff(0.0), SourceSideHTR1(0.0), SourceSideHTR2(0.0), HighPressCutoff(0.0),
              LowPressCutoff(0.0), compressorType(CompressorType::Invalid), AirInletNodeNum(0), AirOutletNodeNum(0), WaterInletNodeNum(0),
              WaterOutletNodeNum(0), LowPressClgError(0), HighPressClgError(0), LowPressHtgError(0), HighPressHtgError(0), plantLoc{}
        {
        }
    };

    // Functions

    void SimWatertoAirHP(EnergyPlusData &state,
                         std::string_view CompName,     // component name
                         int &CompIndex,                // Index for Component name
                         Real64 const DesignAirflow,    // design air flow rate
                         HVAC::FanOp const fanOp,       // cycling scheme--either continuous fan/cycling compressor or
                         bool const FirstHVACIteration, // first iteration flag
                         bool const InitFlag,           // initialization flag used to suppress property routine errors
                         Real64 const SensLoad,         // sensible load
                         Real64 const LatentLoad,       // latent load
                         HVAC::CompressorOp compressorOp,
                         Real64 const PartLoadRatio);

    void GetWatertoAirHPInput(EnergyPlusData &state);

    void InitWatertoAirHP(EnergyPlusData &state,
                          int const HPNum, // index to main heat pump data structure
                          bool const InitFlag,
                          Real64 const SensLoad,
                          Real64 const LatentLoad,
                          Real64 const DesignAirFlow,
                          Real64 const PartLoadRatio);

    void CalcWatertoAirHPCooling(EnergyPlusData &state,
                                 int const HPNum,               // heat pump number
                                 HVAC::FanOp const fanOp,       // fan/compressor cycling scheme indicator
                                 bool const FirstHVACIteration, // first iteration flag
                                 bool const InitFlag,           // suppress property errors if true
                                 Real64 const SensDemand,
                                 HVAC::CompressorOp compressorOp,
                                 Real64 const PartLoadRatio);

    void CalcWatertoAirHPHeating(EnergyPlusData &state,
                                 int const HPNum,               // heat pump number
                                 HVAC::FanOp const fanOp,       // fan/compressor cycling scheme indicator
                                 bool const FirstHVACIteration, // first iteration flag
                                 bool const InitFlag,           // first iteration flag
                                 Real64 const SensDemand,
                                 HVAC::CompressorOp compressorOp,
                                 Real64 const PartLoadRatio);

    void UpdateWatertoAirHP(EnergyPlusData &state, int const HPNum);

    Real64 CalcEffectiveSHR(EnergyPlusData &state,
                            int const HPNum,         // Index number for cooling coil
                            Real64 const SHRss,      // Steady-state sensible heat ratio
                            HVAC::FanOp const fanOp, // fan/compressor cycling scheme indicator
                            Real64 const RTF,        // Compressor run-time fraction
                            Real64 const QLatRated,  // Rated latent capacity
                            Real64 const QLatActual, // Actual latent capacity
                            Real64 const EnteringDB, // Entering air dry-bulb temperature
                            Real64 const EnteringWB  // Entering air wet-bulb temperature
    );

    Real64 DegradF(EnergyPlusData &state,
                   Fluid::GlycolProps *glycol,
                   Real64 &Temp // Temperature of the fluid
    );

    int GetCoilIndex(EnergyPlusData &state,
                     std::string const &CoilType, // must match coil types in this module
                     std::string const &CoilName, // must match coil names for the coil type
                     bool &ErrorsFound            // set to true if problem
    );

    Real64 GetCoilCapacity(EnergyPlusData &state,
                           std::string const &CoilType, // must match coil types in this module
                           std::string const &CoilName, // must match coil names for the coil type
                           bool &ErrorsFound            // set to true if problem
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

} // namespace WaterToAirHeatPump

struct WaterToAirHeatPumpData : BaseGlobalStruct
{

    int NumWatertoAirHPs; // The Number of Water to Air Heat Pumps found in the Input
    Array1D_bool CheckEquipName;

    bool GetCoilsInputFlag; // Flag set to make sure you get input once
    bool MyOneTimeFlag;
    bool firstTime;

    Array1D<WaterToAirHeatPump::WatertoAirHPEquipConditions> WatertoAirHP;

    Real64 initialQSource = 0.0; // Guess Source Side Heat Transfer Rate [W]
    Real64 initialQLoad = 0.0;   // Guess Load Side Heat Transfer rate [W]

    Array1D_bool MyPlantScanFlag;
    Array1D_bool MyEnvrnFlag;

    Real64 initialQSource_calc = 0.0;    // Guess Source Side Heat Transfer Rate [W]
    Real64 initialQLoadTotal_calc = 0.0; // Guess Load Side Heat Transfer rate [W]

    Real64 CompSuctionTemp = 0.0; // Temperature of the Refrigerant Entering the Compressor [C]

    Real64 LoadSideInletDBTemp_Init = 0.0;  // rated conditions
    Real64 LoadSideInletHumRat_Init = 0.0;  // rated conditions
    Real64 LoadSideAirInletEnth_Init = 0.0; // rated conditions

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        this->NumWatertoAirHPs = 0;
        this->CheckEquipName.clear();
        this->GetCoilsInputFlag = true;
        this->MyOneTimeFlag = true;
        this->firstTime = true;
        this->WatertoAirHP.clear();
        this->initialQSource = 0.0;
        this->initialQLoad = 0.0;
        this->MyPlantScanFlag.deallocate();
        this->MyEnvrnFlag.deallocate();
        this->initialQSource_calc = 0.0;
        this->initialQLoadTotal_calc = 0.0;
        this->CompSuctionTemp = 0.0;
        this->LoadSideInletDBTemp_Init = 0.0;
        this->LoadSideInletHumRat_Init = 0.0;
        this->LoadSideAirInletEnth_Init = 0.0;
    }

    // Default Constructor
    WaterToAirHeatPumpData() : NumWatertoAirHPs(0), GetCoilsInputFlag(true), MyOneTimeFlag(true), firstTime(true)
    {
    }
};

} // namespace EnergyPlus

#endif

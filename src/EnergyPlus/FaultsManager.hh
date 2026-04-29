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

#ifndef FaultsManager_hh_INCLUDED
#define FaultsManager_hh_INCLUDED

// ObjexxFCL Headers
#include <ObjexxFCL/Array1D.hh>

// EnergyPlus Headers
#include <EnergyPlus/Data/BaseData.hh>
#include <EnergyPlus/DataGlobals.hh>
#include <EnergyPlus/EnergyPlus.hh>
#include <EnergyPlus/Plant/DataPlant.hh>

namespace EnergyPlus {

// Forward declarations
struct EnergyPlusData;

namespace FaultsManager {

    // Using/Aliasing

    // Data
    // MODULE PARAMETER DEFINITIONS

    // DERIVED TYPE DEFINITIONS:

    // MODULE VARIABLE TYPE DECLARATIONS:

    // ControllerTypeEnum
    int constexpr iController_AirEconomizer = 1001;

    // Input methods for fouling coils
    enum class FouledCoil
    {
        Invalid = -1,
        UARated,
        FoulingFactor,
        Num
    };

    // FaultTypeEnum
    enum class FaultType
    {
        Invalid = -1,
        TemperatureSensorOffset_OutdoorAir,
        HumiditySensorOffset_OutdoorAir,
        EnthalpySensorOffset_OutdoorAir,
        TemperatureSensorOffset_ReturnAir,
        EnthalpySensorOffset_ReturnAir,
        Fouling_Coil,
        ThermostatOffset,
        HumidistatOffset,
        Fouling_AirFilter,
        TemperatureSensorOffset_ChillerSupplyWater,
        TemperatureSensorOffset_CondenserSupplyWater,
        Fouling_Tower,
        TemperatureSensorOffset_CoilSupplyAir,
        Fouling_Boiler,
        Fouling_Chiller,
        Fouling_EvapCooler,
        Num
    };

    // Types of faults under Group Operational Faults in IDD
    //  1. Temperature sensor offset (FY14)
    //  2. Humidity sensor offset (FY14)
    //  3. Enthalpy sensor offset (FY14)
    //  4. Fouling coils (FY14)
    //  5. Thermostat offset (FY15)
    //  6. Humidistat offset (FY15)
    //  7. Fouling air filter (FY15)
    //  8. Chiller Supply Water Temperature Sensor Offset (FY16)
    //  9. Condenser Supply Water Temperature Sensor Offset (FY16)
    //  10. Cooling Tower Scaling (FY16)
    //  11. Coil Supply Air Temperature Sensor Offset (FY16)
    // coming ...
    //  Fouling: chillers, boilers, cooling towers
    //  Damper leakage: return air, outdoor air
    //  Blockage: pipe
    //  Meter: air flow, water flow
    //  CO2 sensor
    //  Pressure sensor offset
    //  more

    //      'FaultModel:PressureSensorOffset:OutdoorAir   ', &
    //      'FaultModel:TemperatureSensorOffset:SupplyAir ', &
    //      'FaultModel:TemperatureSensorOffset:ZoneAir   ', &
    //      'FaultModel:Blockage:Branch                   ', &
    //      'FaultModel:Dirty:AirFilter                   ', &
    //      'FaultModel:Fouling:Chiller                   ', &
    //      'FaultModel:Fouling:Boiler                    ', &
    //      'FaultModel:Fouling:CoolingTower              ', &
    //      'FaultModel:DamperLeakage:ReturnAir           ', &
    //      'FaultModel:DamperLeakage:OutdoorAir          ' /)

    // SUBROUTINE SPECIFICATIONS:

    // Types

    struct FaultProperties // Base Class for operational faults
    {
        // Members
        std::string Name;
        FaultType type = FaultType::Invalid;
        Sched::Schedule *availSched = nullptr;
        Sched::Schedule *severitySched = nullptr;
        Real64 Offset = 0.0; // offset, + means sensor reading is higher than actual value
        bool Status = false; // for future use

        // Virtual Destructor
        virtual ~FaultProperties() = default;

    public:
        Real64 CalFaultOffsetAct(EnergyPlusData &state);
    };

    struct FaultPropertiesEconomizer : public FaultProperties // Class for fault models related with economizer
    {
        // Members
        int ControllerTypeEnum;
        int ControllerID;           // Point to a controller associated with the fault
        std::string ControllerType; // Controller type
        std::string ControllerName; // Controller name

        // Default Constructor
        FaultPropertiesEconomizer() : ControllerTypeEnum(0), ControllerID(0)
        {
        }

        // Destructor
        virtual ~FaultPropertiesEconomizer() = default;
    };

    struct FaultPropertiesThermostat : public FaultProperties // Class for FaultModel:ThermostatOffset
    {
        // Members
        std::string FaultyThermostatName; // The faulty thermostat name

        // Default Constructor
        FaultPropertiesThermostat()
        {
        }

        // Destructor
        virtual ~FaultPropertiesThermostat() = default;
    };

    struct FaultPropertiesHumidistat : public FaultProperties // Class for FaultModel:HumidistatOffset
    {
        // Members
        std::string FaultyThermostatName; // The faulty thermostat name
        std::string FaultyHumidistatName; // The faulty humidistat name
        std::string FaultyHumidistatType; // The faulty humidistat type

        // Default Constructor
        FaultPropertiesHumidistat()
        {
        }

        // Destructor
        virtual ~FaultPropertiesHumidistat() = default;
    };

    struct FaultPropertiesFoulingCoil : public FaultProperties // Class for FaultModel:Fouling:Coil
    {
        // Members
        std::string FouledCoilName;                   // The fouled coil name
        DataPlant::PlantEquipmentType FouledCoilType; // Type of coil that's fouled
        int FouledCoilNum;             // The "FouledUARated" implies having to use the Coil's UA, which could be autosized, so have to use this index
        FouledCoil FoulingInputMethod; // Coil fouling input method
        Real64 UAFouled;               // Fouling coil UA under rating conditions
        Real64 Rfw;                    // Water side fouling factor
        Real64 Rfa;                    // Air side fouling factor
        Real64 Aout;                   // Coil outside surface area
        Real64 Aratio;                 // Inside to outside surface area ratio

        // Default Constructor
        FaultPropertiesFoulingCoil()
            : FouledCoilType(DataPlant::PlantEquipmentType::Invalid), FouledCoilNum(0), FoulingInputMethod(FouledCoil::Invalid), UAFouled(0.0),
              Rfw(0.0), Rfa(0.0), Aout(0.0), Aratio(0.0)
        {
        }

        // Destructor
        virtual ~FaultPropertiesFoulingCoil() = default;

    public:
        // Calculate the fouling thermal insulance factor (the reciprocal of a heat transfer coefficient) due to fouling in a coil
        // Real64 CalFaultyCoilFoulingFactor();

        // Calculate the Fault Fraction based on Availability and Severity Schedules
        Real64 FaultFraction(EnergyPlusData &state);
    };

    struct FaultPropertiesAirFilter : public FaultProperties // Class for FaultModel:Fouling:AirFilter, derived from FaultProperties
    {
        // Members
        std::string fanName; // The name of the fan corresponding to the fouled air filter
        int fanNum = 0;
        HVAC::FanType fanType = HVAC::FanType::Invalid; // The type of the fan corresponding to the fouled air filter
        int fanCurveNum = 0;                            // The index to the curve
        Sched::Schedule *pressFracSched = nullptr;      // The pointer to the schedule
        Real64 fanPressInc = 0.0;                       // The increase of the fan pressure due to fouled air filter
        Real64 fanFlowDec = 0.0;                        // The decrease of the fan airflow rate due to fouled air filter

        // Destructor
        virtual ~FaultPropertiesAirFilter() = default;

    public:
        bool CheckFaultyAirFilterFanCurve(EnergyPlusData &state);
    };

    struct FaultPropertiesCoilSAT : public FaultProperties // Class for FaultModel:TemperatureSensorOffset:CoilSupplyAir
    {
        // Members
        std::string CoilType;                // Coil type
        std::string CoilName;                // Coil name
        std::string WaterCoilControllerName; // Water coil controller name

        // Default Constructor
        FaultPropertiesCoilSAT()
        {
        }
    };

    struct FaultPropertiesChillerSWT : public FaultProperties // Class for FaultModel:TemperatureSensorOffset:ChillerSupplyWater
    {
        // Members
        std::string ChillerType; // Chiller type
        std::string ChillerName; // Chiller name

        // Default Constructor
        FaultPropertiesChillerSWT()
        {
        }

        // Destructor
        virtual ~FaultPropertiesChillerSWT() = default;

    public:
        void CalFaultChillerSWT(bool FlagConstantFlowChiller,  // True if chiller is constant flow and false if it is variable flow
                                Real64 FaultyChillerSWTOffset, // Faulty chiller SWT sensor offset
                                Real64 Cp,                     // Local fluid specific heat
                                Real64 EvapInletTemp,          // Chiller evaporator inlet water temperature
                                Real64 &EvapOutletTemp,        // Chiller evaporator outlet water temperature
                                Real64 &EvapMassFlowRate,      // Chiller mass flow rate
                                Real64 &QEvaporator            // Chiller evaporator heat transfer rate
        );
    };

    struct FaultPropertiesCondenserSWT : public FaultProperties // Class for FaultModel:TemperatureSensorOffset:CondenserSupplyWater
    {
        // Members
        std::string TowerType; // Tower type
        std::string TowerName; // Tower name

        // Default Constructor
        FaultPropertiesCondenserSWT()
        {
        }
    };

    struct FaultPropertiesTowerFouling : public FaultProperties // Class for FaultModel:Fouling:CoolingTower
    {
        // Members
        std::string TowerType;    // Tower type
        std::string TowerName;    // Tower name
        Real64 UAReductionFactor; // UA Reduction Factor

        // Default Constructor
        FaultPropertiesTowerFouling() : UAReductionFactor(1.0)
        {
        }

    public:
        Real64 CalFaultyTowerFoulingFactor(EnergyPlusData &state);
    };

    struct FaultPropertiesFouling : public FaultProperties // Class for FaultModel:Fouling
    {
        // Members
        Real64 FoulingFactor; // Fouling Factor

        // Default Constructor
        FaultPropertiesFouling() : FoulingFactor(1.0)
        {
        }

    public:
        Real64 CalFoulingFactor(EnergyPlusData &state); // To calculate the dynamic fouling factor
    };

    struct FaultPropertiesBoilerFouling : public FaultPropertiesFouling // Class for FaultModel:Fouling:Boiler
    {
        // Members
        std::string BoilerType; // Boiler type
        std::string BoilerName; // Boiler name

        // Default Constructor
        FaultPropertiesBoilerFouling()
        {
        }
    };

    struct FaultPropertiesChillerFouling : public FaultPropertiesFouling // Class for FaultModel:Fouling:Chiller
    {
        // Members
        std::string ChillerType; // Chiller type
        std::string ChillerName; // Chiller name

        // Default Constructor
        FaultPropertiesChillerFouling()
        {
        }
    };

    struct FaultPropertiesEvapCoolerFouling : public FaultPropertiesFouling // Class for FaultModel:Fouling:EvaporativeCooler
    {
        // Members
        std::string EvapCoolerType; // Evaporative Cooler type
        std::string EvapCoolerName; // Evaporative Cooler name

        // Default Constructor
        FaultPropertiesEvapCoolerFouling()
        {
        }
    };

    // Functions

    void CheckAndReadFaults(EnergyPlusData &state);

    void SetFaultyCoilSATSensor(
        EnergyPlusData &state, std::string const &CompType, std::string_view CompName, bool &FaultyCoilSATFlag, int &FaultyCoilSATIndex);

} // namespace FaultsManager

struct FaultsManagerData : BaseGlobalStruct
{

    bool RunFaultMgrOnceFlag = false; // True if CheckAndReadFaults is already done
    bool ErrorsFound = false;         // True if errors detected in input

    bool AnyFaultsInModel = false;       // True if there are operational faults in the model
    int NumFaults = 0;                   // Number of faults (include multiple faults of same type) in the model
    int NumFaultyEconomizer = 0;         // Total number of faults related with the economizer
    int NumFouledCoil = 0;               // Total number of fouled coils
    int NumFaultyThermostat = 0;         // Total number of faulty thermostat with offset
    int NumFaultyHumidistat = 0;         // Total number of faulty humidistat with offset
    int NumFaultyAirFilter = 0;          // Total number of fouled air filters
    int NumFaultyChillerSWTSensor = 0;   // Total number of faulty Chillers Supply Water Temperature Sensor
    int NumFaultyCondenserSWTSensor = 0; // Total number of faulty Condenser Supply Water Temperature Sensor
    int NumFaultyTowerFouling = 0;       // Total number of faulty Towers with Scaling
    int NumFaultyCoilSATSensor = 0;      // Total number of faulty Coil Supply Air Temperature Sensor
    int NumFaultyBoilerFouling = 0;      // Total number of faulty Boilers with Fouling
    int NumFaultyChillerFouling = 0;     // Total number of faulty Chillers with Fouling
    int NumFaultyEvapCoolerFouling = 0;  // Total number of faulty Evaporative Coolers with Fouling

    // Object Data
    Array1D<FaultsManager::FaultPropertiesEconomizer> FaultsEconomizer;
    Array1D<FaultsManager::FaultPropertiesFoulingCoil> FouledCoils;
    Array1D<FaultsManager::FaultPropertiesThermostat> FaultsThermostatOffset;
    Array1D<FaultsManager::FaultPropertiesHumidistat> FaultsHumidistatOffset;
    Array1D<FaultsManager::FaultPropertiesAirFilter> FaultsFouledAirFilters;
    Array1D<FaultsManager::FaultPropertiesChillerSWT> FaultsChillerSWTSensor;
    Array1D<FaultsManager::FaultPropertiesCondenserSWT> FaultsCondenserSWTSensor;
    Array1D<FaultsManager::FaultPropertiesTowerFouling> FaultsTowerFouling;
    Array1D<FaultsManager::FaultPropertiesCoilSAT> FaultsCoilSATSensor;
    Array1D<FaultsManager::FaultPropertiesBoilerFouling> FaultsBoilerFouling;
    Array1D<FaultsManager::FaultPropertiesChillerFouling> FaultsChillerFouling;
    Array1D<FaultsManager::FaultPropertiesEvapCoolerFouling> FaultsEvapCoolerFouling;

    void init_constant_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void init_state([[maybe_unused]] EnergyPlusData &state) override
    {
    }

    void clear_state() override
    {
        RunFaultMgrOnceFlag = false;
        ErrorsFound = false;
        AnyFaultsInModel = false;
        NumFaults = 0;
        NumFaultyEconomizer = 0;
        NumFouledCoil = 0;
        NumFaultyThermostat = 0;
        NumFaultyHumidistat = 0;
        NumFaultyAirFilter = 0;
        NumFaultyChillerSWTSensor = 0;
        NumFaultyCondenserSWTSensor = 0;
        NumFaultyTowerFouling = 0;
        NumFaultyCoilSATSensor = 0;

        FaultsEconomizer.deallocate();
        FouledCoils.deallocate();
        FaultsThermostatOffset.deallocate();
        FaultsHumidistatOffset.deallocate();
        FaultsFouledAirFilters.deallocate();
        FaultsChillerSWTSensor.deallocate();
        FaultsCondenserSWTSensor.deallocate();
        FaultsTowerFouling.deallocate();
        FaultsCoilSATSensor.deallocate();
    }
};

} // namespace EnergyPlus

#endif
